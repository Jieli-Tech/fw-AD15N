#include "config.h"
#include "hwi.h"
#include "clock.h"
#include "math.h"
#include "common.h"
#include "efuse.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[clock]"
#include "debug.h"


const u32 pll_clock_tab0[]  = {
    0,
    96000000L,
    320000000L,
    480000000L,
};
const u32 pll_clock_tab1[]  = {
    0,
    107000000L,
    137000000L,
    192000000L,
};
const u8 div_taba[] = {
    1, 3, 5, 7
};

const u8 div_tabb[] = {
    1, 2, 4, 8
};

enum {
    UART_CLK_IN_PLL48M = 0,
    UART_CLK_IN_OSC,
    UART_CLK_IN_LSB,
    UART_CLK_IN_DISABLE,
};
#define UART_CLOCK_IN(x)        SFR(JL_CLK->CON0,  8,  2,  x)
#define UART_SRC_CLK    UART_CLK_IN_PLL48M//UART_CLK_IN_OSC
enum {
    OSC_CLK_IN_ICH3 = 0,
    OSC_CLK_IN_PAT,
    OSC_CLK_IN_LRC,
    OSC_CLK_IN_PLL12M,
};
#define OSC_CLOCK_IN(x)        SFR(JL_CLK->CON0,  19,  2,  x)
#define OSC_SRC_CLK    OSC_CLK_IN_PLL12M//OSC_CLK_IN_LRC

u32 sys_clock_peration(void)
{
    u32 t_sel;
    u32 clock = 0;
    t_sel = (JL_CLK->CON1 >> 20) & 0x3;
    /* log_info("clk 0x%x\n", JL_CLK->CON1); */
    if (0 != t_sel) {
        clock = pll_clock_tab0[t_sel];
    } else {
        t_sel = (JL_CLK->CON1 >> 22) & 0x3;
        if (0 != t_sel) {
            clock = pll_clock_tab1[t_sel];
        }
    }
    if (0 == clock) {
        log_info(" sys clock info err\n");
        return 0;
    }
    u32 t_diva = div_taba[(JL_CLK->CON1 >> 16) & 0x3];
    u32 t_divb = div_tabb[(JL_CLK->CON1 >> 18) & 0x3];
    u32 t_divc = ((JL_CLK->CON1 >> 5) & 0x7) + 1;
    clock = clock / (t_diva * t_divb * t_divc);

    /* log_info(" sys clock %ld\n", clock); */
    return clock;
}
void sfc_resume(u32 disable_spi);

AT(.ram_code)
void sfc_suspend(u32 enable_spi)
{
    //wait cache idle
    while (!(JL_CACHE->CON & BIT(5)));
    //wait sfc idle
    while (JL_SFC->CON & BIT(31));

    //disable sfc
    JL_PORTD->PU |= BIT(2);
    JL_PORTD->DIR |= BIT(2);

    JL_SFC->CON &= ~BIT(0);

    JL_PORTD->DIR &= ~BIT(2);

    if (enable_spi) {
        JL_SPI0->CON |= BIT(0);
    }
}
static u32 sfc_clk;
#define     SPI_TSHSL   40
AT(.ram_code)
void sfc_baud_set(u32 baud)
{
    local_irq_disable();
    sfc_suspend(0);
    const u32 tshsl = SPI_TSHSL * (sfc_clk / 1000000) / 1000 + 1;
    // see https://gitee.com/Jieli-Tech/fw-AD15N/issues/I41WDD
    /* const u32 tshsl = 0x7; */
    SFR(JL_SFC->CON, 20, 4, tshsl);
    JL_SFC->BAUD = baud;
    sfc_resume(0);
    local_irq_enable();
}

AT(.ram_code)
static u32 sfc_max_baud(u32 pll_clock, _PLL_DIV pll_div)
{
    u32 t_pll_clk = 0;
    /* log_info("pll set, pll_clock %d, pll_div %d \n", pll_clock, pll_div); */
    if (0 == (3 & pll_clock)) {
        if (0b0100 == (0x0c & pll_clock)) {
            t_pll_clk = 107;
        } else if (0b1000 == (0x0c & pll_clock)) {
            t_pll_clk = 137;
        } else if (0b1100 == (0x0c & pll_clock)) {
            t_pll_clk = 192;
        }
    } else if (1 == (0x03 & pll_clock)) {
        t_pll_clk = 96;
    } else if (2 == (0x03 & pll_clock)) {
        t_pll_clk = 320;
    } else if (3 == (0x03 & pll_clock)) {
        t_pll_clk = 480;
    }
    u32 a_div = pll_div & 0x3;
    a_div = a_div * 2 + 1;
    u32 b_div = (pll_div >> 2) & 0x3;
    if (0 == b_div) {
        b_div = 1;
    } else if (1 == b_div) {
        b_div = 2;
    } else if (2 == b_div) {
        b_div = 4;
    } else if (3 == b_div) {
        b_div = 8;
    }
    u32 div = a_div * b_div;
    sfc_clk = t_pll_clk / div;
    u32 baud = 0;
    while ((sfc_clk / (baud + 1)) > 60) {
        baud++;
    }
    sfc_clk *= 1000000;
    /* log_info(" PLL_SYS_CLK %d;  SFC BAUD %d\n",sfc_clk, baud); */
    return baud;

}

/* void pll_sel(u32 pll_clock, _PLL_DIV pll_div) */
static u32 sys_clock;
__attribute__((always_inline))
u32 sys_clock_get(void)
{
    return sys_clock;
}
void pll_sel(u32 pll_clock, _PLL_DIV pll_div, _PLL_B_DIV pll_b_div)
{
    u32 clock;
    local_irq_disable();
    JL_CLK->CON0 &= ~BIT(5);          // select rc
    delay(1);

    SFR(JL_CLK->CON1, 20, 4, pll_clock);      //pll sys clk sel 96m  3
    SFR(JL_CLK->CON1, 16, 4, pll_div);      //pll sys clk div 1   4

    SFR(JL_CLK->CON1, 5, 3, pll_b_div);       //hsb div 2

    sys_clock = sys_clock_peration();
    clock = sys_clock;

    for (u32 i = 0; i < 8; i++) {
        clock = sys_clock / (i + 1);
        if (clock <= 80000000L) {
            SFR(JL_CLK->CON1, 2, 3, i);       //lsb div
            break;
        }
    }
    SFR(JL_CLK->CON0, 3, 2, 3);       // select pll
    JL_CLK->CON0 |=  BIT(5);          // select mux
    delay(10);

    u32 baud = sfc_max_baud(pll_clock, pll_div);
    sfc_baud_set(baud);

    local_irq_enable();

    lrc_trace_init();
    lrc_trace_trim();

    log_info("---SFC CLK : %d", sfc_clk);
    log_info("---SPI CLK : %d", sfc_clk / (baud + 1));
    log_info("---HSB CLK : %d", sys_clock);
    log_info("---LSB CLK : %d", clk_get("lsb"));
}

u32 lsb_clk_get(void)
{
    /* u32 div57 = ((JL_CLK->CON1 >> 5) & 0x7) + 1; */
    u32 div42 = ((JL_CLK->CON1 >> 2) & 0x7) + 1;
    u32 lsb_clk = sys_clock_get() / (div42);

    /* log_info("lsb_clk %d\n", lsb_clk); */
    return lsb_clk;
}

void uart_clk_sel()
{
    UART_CLOCK_IN(UART_SRC_CLK);
    if (UART_SRC_CLK == UART_CLK_IN_OSC) {
        OSC_CLOCK_IN(OSC_SRC_CLK);
    }
}
u32 uart_clk_get()
{
    switch (UART_SRC_CLK) {
    case UART_CLK_IN_PLL48M:
        /* log_info("---uart clk:48m!\n"); */
        return (48 * 1000000);
    case UART_CLK_IN_OSC:
        if (OSC_SRC_CLK == OSC_CLK_IN_ICH3) {
            log_info("uart_clk:input channel 3!\n");
        } else if (OSC_SRC_CLK == OSC_CLK_IN_LRC) {
            return (32 * 1000);
        } else if (OSC_SRC_CLK == OSC_CLK_IN_PLL12M) {
            /* log_info("---uart clk:12m!\n"); */
            return (12 * 1000000);
        }
        break;
    case UART_CLK_IN_LSB:
        /* log_info("---uart clk:lsb!\n"); */
        return lsb_clk_get();
    }
    return 0;
}
__attribute__((always_inline_when_const_args))
int clk_get(const char *name)
{
    if (!strcmp(name, "sys")) {
        return sys_clock_get();
    }
    if (!strcmp(name, "uart")) {
        return uart_clk_get();
    }
    if (!strcmp(name, "timer")) {
        return (48 * 1000000);
    }
    if (!strcmp(name, "lsb")) {
        return lsb_clk_get();
    }
    if (!strcmp(name, "sd")) {
        return lsb_clk_get();
    }
    if (!strcmp(name, "spi")) {
        return lsb_clk_get();
    }
    if (!strcmp(name, "adc")) {
        return lsb_clk_get();
    }
    if (!strcmp(name, "mcpwm")) {
        return lsb_clk_get();
    }
    if (!strcmp(name, "lrc")) {
        return lrc_clk_get();
    }
    return 0;
}

void clk_output_sel(u8 clk)
{
    JL_PORTA->DIR &= ~BIT(3);
    SFR(JL_CLK->CON0, 10, 5, 0);
    SFR(JL_CLK->CON0, 10, 4, clk);
    SFR(JL_CLK->CON0, 14, 1, 1);
}

static void set_timer_captrue_dp(u32 io_config)
{
    JL_IOMC->USB_IO_CON0 |= BIT(11);//io mode
    JL_USB->CON0 |= BIT(0);//phy on
    if (io_config) {
        JL_IOMC->USB_IO_CON0 |= BIT(9) | BIT(10); // DP & DM die en
        JL_IOMC->USB_IO_CON0 &= ~(BIT(0) | BIT(1)); // DP & DM do dis
        JL_IOMC->USB_IO_CON0 |= BIT(2) | BIT(3); // DP & DM di en
        JL_IOMC->USB_IO_CON0 &= ~(BIT(4) | BIT(5)); //DP & DM pull down disable
        JL_IOMC->USB_IO_CON0 &= ~(BIT(6) | BIT(7)); //DP & DM pull up disable
        JL_IOMC->USB_IO_CON0 |= BIT(6); //DP pull up enable
    }

    JL_IOMC->IOMC2 &= ~(0x1f << 0);
    JL_IOMC->IOMC2 |= ((24) << 0); //dp--->input ch0
    JL_IOMC->IOMC1 |= 0b11 << 4; //input ch0 -->t2 cap
    JL_TMR2->CON = BIT(6);
}

#include "stdlib.h"

struct usb_trim_t {
    u16 last_cnt;
    u16 last_prd;
    u32 trim_osc_freq;
    u8 sof_cnt;
    u32 pll_ds;
    volatile u8 g_pll_trimok;
};
static u32 timer_div()
{
    u32 timer_src_clk = clk_get("lsb");
    if (timer_src_clk > 64 * 1000000) {
        return 4;
    }
    return 1;
}
static struct usb_trim_t usb_trim ;//SEC(.usb_config_var);
SET_INTERRUPT
void sof_trim_isr()
{
    udelay(140);
    JL_TMR2->CON |= BIT(6);      //clear tmr pnd

    u16 cur_prd = (u16)JL_TMR2->PRD - usb_trim.last_cnt;
    usb_trim.last_cnt = JL_TMR2->PRD;

    if (abs(usb_trim.last_prd - cur_prd) < 260) {
        usb_trim.trim_osc_freq += cur_prd;

        usb_trim.sof_cnt++;

        if (usb_trim.sof_cnt == (4)) {
            JL_TMR2->CON = BIT(6);
            u32 timer_src_clk = clk_get("lsb") / 1000;

            usb_trim.trim_osc_freq = usb_trim.trim_osc_freq * timer_div() / 4 ;

            usb_trim.pll_ds = (((u32)(timer_src_clk) << 1) * usb_trim.pll_ds) / usb_trim.trim_osc_freq;
            usb_trim.pll_ds = (usb_trim.pll_ds >> 1) + (usb_trim.pll_ds & BIT(0));
            usb_trim.g_pll_trimok = 1;
        }
    } else {
        usb_trim.sof_cnt = 0;
        usb_trim.trim_osc_freq = 0;
    }

    usb_trim.last_prd = cur_prd;
}
AT_RAM
void pll_config(u32 pll_ds)
{
    pll_ds -= 2;

    log_info("JL_PLL->CON1 %x", JL_PLL->CON1);

    local_irq_disable();
    JL_CLK->CON0 |= BIT(1);
    JL_CLK->CON0 &= ~BIT(5);          // select hrc
    delay(10);

    SFR(JL_PLL->CON1, 0, 12, pll_ds);

    JL_PLL->CON0 |= BIT(0);//EN
    //udelay(20);
    delay(40);
    JL_PLL->CON0 |= BIT(1);//RST
    //udelay(10);
    delay(20);
    JL_CLK->CON0 |=  BIT(5);          // select mux

    local_irq_enable();

    log_info("JL_PLL->CON1 %x", JL_PLL->CON1);
}
void usb_sof_trim(void (*trim_enter)(void), void (*trim_exit)(void))
{
    memset(&usb_trim, 0, sizeof(usb_trim));

    usb_trim.pll_ds = (JL_PLL->CON1 & 0xfff) + 2;


    log_info("old pll_ds %d", usb_trim.pll_ds);

    if (trim_enter) {
        trim_enter();
    }
    request_irq(IRQ_TIME2_IDX, 7, sof_trim_isr, cpu_id);
    set_timer_captrue_dp(1);

    JL_TMR2->CNT = 0;
    JL_TMR2->PRD = 0;//100000;
    if (timer_div() == 1) {
        JL_TMR2->CON = BIT(1) | (0b00 << 2) | (0b00 << 4); //选择lsb时钟,div1
    } else {
        JL_TMR2->CON = BIT(1) | (0b00 << 2) | (0b01 << 4); //选择lsb时钟,div4
    }

    u32 timeout = 100;
    timeout = jiffies + timeout;

    while (1) {
        if (time_after(jiffies, timeout)) {
            log_info("trim error");
            break;
        }

        if (usb_trim.g_pll_trimok) {
            log_info("trim succ");
            break;
        }
    }


    bit_clr_ie(IRQ_TIME2_IDX);

    if (usb_trim.g_pll_trimok) {
        log_info("new pll_ds %d", usb_trim.pll_ds);

        pll_config(usb_trim.pll_ds);
    }
    if (trim_exit) {
        trim_exit();
    }

}

extern const char LRC_TRIM_DISABLE;  //LRC trim 主时钟
extern const char libs_debug;   //打印总开关

#define DS_CENTER   1873
static u32 lrc_pll_ds = DS_CENTER;
void LRC_CLK_EN(void);
void lrc_trace_init(void)
{
    if (LRC_TRIM_DISABLE) {
        return;
    }
    //lrc init
    LRC_CLK_EN();

    JL_LRCT->CON = 0;
    JL_LRCT->CON |= (3 << 1);//32 * 2^3 = 256，单位LRC周期

    u32 efuse_page0 = p33_rd_page(0);
    if (!(efuse_page0 & BIT(15))) {
        log_error("efuse_page0 BIT(15) = 0, No lrc pll ds trim !\n");
        while (1);
    }
    u32 efuse_page1 = p33_rd_page(1);
    if ((libs_debug) && (efuse_page1 == 0xffffffff)) {
        log_error("No efuse trim !\n");
        while (1);
    }
    u32 ds_diff = (efuse_page0 >> 16) & 0xff;
    if (efuse_page1 & BIT(25)) {
        ds_diff |= BIT(8);
    }
    if (efuse_page1 & BIT(26)) {
        lrc_pll_ds = DS_CENTER - ds_diff + 2;
    } else {
        lrc_pll_ds = DS_CENTER + ds_diff + 2;
    }
    log_info("ds_diff = 0x%x\n", ds_diff);
    log_info("lrc_pll_ds = 0x%x\n", lrc_pll_ds);
}
void lrc_trace_trim(void)
{
    if (LRC_TRIM_DISABLE) {
        return;
    }
    JL_LRCT->CON |= BIT(6) | BIT(0);
    while (!(JL_LRCT->CON & BIT(7)));
    u32 num = JL_LRCT->NUM;
    JL_LRCT->CON |= BIT(6);
    JL_LRCT->CON &= ~BIT(0);
    log_info("jl_lrct->num = %d\n", num);
    u32 old_pll_ds = JL_PLL->CON1 & 0xfff;
    log_info("old_pll_ds = %d\n", old_pll_ds + 2);
    u32 new_pll_ds = (old_pll_ds + 2) * 256 * lrc_pll_ds / (num >> 3);
    log_info("new_pll_ds = %d\n", new_pll_ds - 2);
    pll_config(new_pll_ds);
}

u32 lrc_clk_get(void)
{
    if (LRC_TRIM_DISABLE) {
        return 32000;
    }

    return 480000000 / lrc_pll_ds / 8;
}

#if 0
u32 clock_get_uart(void)
{
    return (48 * 1000000);
}

void clk_out(enum CLK_OUT_SOURCE clk)
{
    gpio_set_direction(IO_PORTA_03, 0);

    JL_CLK->CON0 |= ((clk | BIT(4)) << 10);
}
#endif

