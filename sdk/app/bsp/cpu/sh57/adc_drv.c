#include "asm/power_interface.h"
#include "gpio.h"
#include "adc_drv.h"
#include "cpu.h"
#include "hwi.h"
#include "clock.h"
#include "errno-base.h"

#define LOG_TAG_CONST         NORM
#define LOG_TAG               "[saradc]"
#include "log.h"

#define PMU_DET_EN(en)          p33_fast_access(P3_ANA_CON4, BIT(0), en)
#define MVBG_TO_ADC_EN(en)      p33_fast_access(P3_ANA_CON4, BIT(5), en)
#define WVBG_TO_ADC_EN(en)      p33_fast_access(P3_ANA_CON4, BIT(6), en)
#define VBG_BUFFER_EN(en)      	p33_fast_access(P3_ANA_CON4, BIT(4), en)
#define ADC_CHANNEL_SEL(ch)     P33_CON_SET(P3_ANA_CON4, 1, 3, ch)

#define B_SADC_INIT             BIT(0)
#define B_SADC_BUSY             BIT(1)
#define B_SADC_WAIT             BIT(2)
void udelay(u32 us);

typedef struct _adc_info {
    u8 ch;
    u16 value;
} adc_info_t;

void (*adc_scan_over_cb)(void) = NULL;
static adc_info_t adc_info[ADC_MAX_CH_NUM];
static volatile u8 cur_ch = 0;
volatile u8 adc_flag = 0;
static u16 vbg_value = 0;

const u8 adc_clk_div_t[] = {
    1,     /*000*/
    6,     /*001*/
    12,    /*010*/
    24,    /*011*/
    48,    /*100*/
    72,    /*101*/
    96,    /*110*/
    128,    /*111*/
    160    /*111*/
};
#define ADC_MAX_CLK    1000000L

u8 adc_ch2port(u16 real_ch)
{
    if (real_ch > ADC_CH_PD3) {
        return IO_PORT_MAX;
    }
    u32 ch = real_ch & 0xf;
    const u8 io_adcch_map_table[] = {
        IO_PORTA_00,
        IO_PORTA_01,
        IO_PORTA_02,
        IO_PORTA_03,
        IO_PORTA_04,
        IO_PORTA_05,
        IO_PORTA_06,
        IO_PORTA_07,
        IO_PORTA_08,
        IO_PORTA_09,
        IO_PORTA_10,
        IO_PORTA_11,
        IO_PORTA_12,
        IO_PORTD_01,
        IO_PORTD_03,
    };

    return io_adcch_map_table[ch];
}

static u32 adc_is_wait(void)
{
    local_irq_disable();
    if (adc_flag & B_SADC_WAIT) {
        local_irq_enable();
        return E_ADC_WAIT;
    }
    adc_flag |= B_SADC_WAIT;
    local_irq_enable();
    return 0;
}

static u32 adc_is_busy(u32 retry)
{
    while (1) {
        if (!(adc_flag & B_SADC_BUSY)) {
            break;
        }
        if (0 == (retry--)) {
            return E_ADC_BUF;
        }
    }
    return 0;
}

static u32 adc_idle(u32 retry)
{
    u32 err = adc_is_wait();
    if (0 != err) {
        /* putchar('w'); */
        return err;
    }
    err = adc_is_busy(retry);
    if (0 != err) {
        adc_flag &= ~B_SADC_WAIT;
        /* putchar('b'); */
        return err;
    }
    adc_flag |= B_SADC_BUSY;
    adc_flag &= ~B_SADC_WAIT;
    return 0;
}

void adc_add_sample_ch(u16 real_ch)
{
    local_irq_disable();
    for (u8 i = 0; i < ADC_MAX_CH_NUM; i++) {
        if ((adc_info[i].ch & 0xf) == real_ch) {
            break;
        } else if (adc_info[i].ch == 0xff) {
            adc_info[i].ch = real_ch;
            adc_info[i].value = ADC_VALUE_NONE;
            log_info("add sample ch %x, chidx %d\n", real_ch, i);
            break;
        }
    }
    local_irq_enable();
}

void adc_remove_sample_ch(u16 real_ch)
{
    local_irq_disable();
    for (u8 i = 0; i < ADC_MAX_CH_NUM; i++) {
        if ((adc_info[i].ch & 0xf) == real_ch) {
            adc_info[i].ch = 0xff;
            adc_info[i].value = ADC_VALUE_NONE;
            break;
        }
    }
    local_irq_enable();
}

u8 adc_add_ch_reuse(u16 real_ch, u8 busy)
{
    for (u8 i = 0; i < ADC_MAX_CH_NUM; i++) {
        if ((adc_info[i].ch & 0xf) == real_ch) {
            adc_info[i].ch |= ADC_REUSE_IO;
            if (busy) {
                adc_info[i].ch |= ADC_BUSY_IO;
                adc_info[i].value = ADC_VALUE_NONE;
            } else {
                adc_info[i].ch &= ~ADC_BUSY_IO;
            }
        }
    }
    return -1;
}

u8 adc_remove_ch_reuse(u16 real_ch)
{
    for (u8 i = 0; i < ADC_MAX_CH_NUM; i++) {
        if ((adc_info[i].ch & 0xf) == real_ch) {
            adc_info[i].ch &= ~ADC_REUSE_IO;
            adc_info[i].ch &= ~ADC_BUSY_IO;
        }
    }
    return -1;
}

static u8 adc_get_next_ch(u8 now_ch)
{
    for (u8 i = now_ch + 1; i < ADC_MAX_CH_NUM; i++) {
        if (adc_info[i].ch != 0xff) {
            return i;
        }
    }
    return 0xff;
}

SET(interrupt(""))
void adc_isr()
{
    JL_ADC->CON = BIT(6);
    adc_info[cur_ch].value = JL_ADC->RES;

    PMU_DET_EN(0);

    /* putchar('t'); */
    adc_scan();


}


static void adc_sample(u16 real_ch, bool isr_flag)
{
    u8 time_div = 0;
    JL_ADC->CON = 0;
    int clk = clk_get("lsb");
    for (int j = 0; j < sizeof(adc_clk_div_t) / sizeof(adc_clk_div_t[0]); j++) {
        if (clk / adc_clk_div_t[j] <= ADC_MAX_CLK) {
            time_div = adc_clk_div_t[j];
            time_div = time_div / 2;
            /* log_info("time_div 0x%x\n",time_div); */
            break;
        }
    }
    JL_ADC->CON |= time_div << 8;
    JL_ADC->CON |= 0 << 15; //延时启动，实际启动延时为N*8个CLK
    /* 通道选择 */
    if (real_ch & ADC_MUX_IO) {
        /* log_info("IO\n"); */
        ADC_CHL(real_ch & 0xf);
        IO_CHL_EN(1);
    } else if (real_ch & ADC_MUX_AN) {
        ADC_CHANNEL_SEL(real_ch & 0xf);//CHANNEL
        if (real_ch == ADC_CH_PMU_VBG) {
            /* log_info("VBG\n"); */
            MVBG_TO_ADC_EN(1);
        }
        PMU_DET_EN(1);
        ADC_TEST_SEL(ADC_CH_PMU);             //SFR(JL_ADC->CON, 23, 3, ch)
        TEST_CHL_EN(1);						  //SFR(JL_ADC->CON, 3, 1, en)
    } else {
        log_error("channle error\n");
        return;
    }
    if (isr_flag) {
        JL_ADC->CON |= BIT(1);  //IE
    } else {
        JL_ADC->CON &= ~BIT(1);
    }
    ADC_DEN(1);
    CPND(1);
}

int adc_kick_start(void (*adc_scan_over)(void))
{
    if (!(adc_flag & B_SADC_INIT)) {
        return 0xff;
    }
    u32 err = adc_idle(0);
    if (0 != err) {
        /* putchar('e'); */
        return err;
    }
    u8 ch = adc_get_first_available_ch();
    if (ch == 0xff) {
        log_error("kick start find channle error\n");
        return -1;
    }
    adc_scan_over_cb = adc_scan_over;
    /* putchar('k'); */
    adc_sample(adc_info[cur_ch].ch, 1);
    return 0;
}


u16 adc_get_first_available_ch()
{
    if (!(adc_flag & B_SADC_INIT)) {
        return 0xff;
    }
    for (u8 i = 0; i < ARRAY_SIZE(adc_info); i++) {
        if (adc_info[i].ch != 0xff) {
            cur_ch = i;
            return cur_ch;
        }
    }
    return 0xff;
}

void adc_scan(void)
{
    cur_ch = adc_get_next_ch(cur_ch);
    if (cur_ch == 0xff) {
        /* ADC_DEN(0); */
        /* IO_CHL_EN(0); */
        /* TEST_CHL_EN(0); */
        /* PMU_DET_EN(0); */
        /* putchar('o'); */
        adc_flag &= ~B_SADC_BUSY;
        if (adc_scan_over_cb) {
            adc_scan_over_cb();
        }
        return;
    }
    adc_sample(adc_info[cur_ch].ch, 1);
}

u16 adc_get_value(u16 real_ch)
{
    for (u8 i = 0; i < ARRAY_SIZE(adc_info); i++) {
        if (adc_info[i].ch == real_ch) {
            return adc_info[i].value;
        }
    }

    return ((u16) - 1);
}

void adc_init(void)
{
    log_info("adc_init\n");
    JL_ADC->CON = 0;
    int clk = clk_get("lsb");

    for (int i = 0; i < ADC_MAX_CH_NUM; i++) {
        adc_info[i].ch = 0xff;
        adc_info[i].value = ADC_VALUE_NONE;
    }
    u8 time_div = 0;
    for (int j = 0; j < sizeof(adc_clk_div_t) / sizeof(adc_clk_div_t[0]); j++) {
        if (clk / adc_clk_div_t[j] <= ADC_MAX_CLK) {
            time_div = adc_clk_div_t[j];
            time_div = time_div / 2;
            /* log_info("time_div 0x%x\n",time_div); */
            break;
        }
    }
    u32 adc_con = 0;
    adc_con |= time_div << 8;
    JL_ADC->CON = adc_con;

    VBG_BUFFER_EN(1);
    udelay(100);
    adc_sample(ADC_CH_PMU_VBG, 0);
    for (u8 i = 0; i < 10; i++) {
        while (!(JL_ADC->CON & BIT(7)));
        vbg_value += JL_ADC->RES;
        JL_ADC->CON |= BIT(6);
    }
    vbg_value /= 10;

    ADC_DEN(0);
    TEST_CHL_EN(0);						  //SFR(JL_ADC->CON, 3, 1, en)
    PMU_DET_EN(0);
    MVBG_TO_ADC_EN(0);					  //P33_CON_SET(P3_ANA_CON4, 5, 1, 1)
    udelay(100);
    VBG_BUFFER_EN(0);//关闭BUFFER
    log_info("vbg_value %d\n", vbg_value);
    /* adc_add_sample_ch(ADC_CH_PMU_IOVDD); */
    log_info("P3_ANA_CON3 %x",    p33_rx_1byte(P3_ANA_CON3));
    HWI_Install(IRQ_GPADC_IDX, (u32)adc_isr, IRQ_ADC_IP) ;
    JL_ADC->CON |= BIT(1); //IE
    adc_flag |= B_SADC_INIT;
}

u16 adc_sample_vbg()
{
    if (!(adc_flag & B_SADC_INIT)) {
        return 0;
    }
    u32 err = adc_idle(100);
    if (0 != err) {
        return err;
    }
    VBG_BUFFER_EN(1);
    udelay(100);
    adc_sample(ADC_CH_PMU_VBG, 0);
    while (!(JL_ADC->CON & BIT(7)));
    vbg_value = JL_ADC->RES;
    ADC_DEN(0);
    TEST_CHL_EN(0);
    PMU_DET_EN(0);
    MVBG_TO_ADC_EN(0);
    udelay(100);
    VBG_BUFFER_EN(0);//关闭BUFFER

    adc_flag &= ~B_SADC_BUSY;
    return vbg_value;
}

u32 adc_value2voltage(u32 adc_vbg, u32 adc_ch_val)
{
    u32 adc_res = adc_ch_val;
    u32 vbg_center = 800;
    adc_res = adc_res * vbg_center / adc_vbg;
    return adc_res;
}

u32 adc_get_voltage(u16 real_ch)
{
    u32 adc_res = adc_get_value(real_ch);
    u32 adc_vbg = vbg_value;
    if (adc_vbg == 0) {
        return 0;
    }
    return adc_value2voltage(adc_vbg, adc_res);
}

u32 adc_get_vbat_voltage(void)
{
    u32 vbat_voltage = 4 * adc_get_voltage(ADC_CH_PMU_1_4VBAT);
    return vbat_voltage;
}



