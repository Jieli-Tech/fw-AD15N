#include "config.h"
#include "hwi.h"
#include "clock.h"
#include "math.h"
#include "common.h"
#include "gpio.h"
#include "asm/power_interface.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[clock]"
#include "log.h"
void sfc_suspend(u32 enable_spi);
void sfc_resume(u32 disable_spi);

static u32 sys_clock = 48000000;
static  u32 sfc_clk;

#define     SPI_TSHSL   40
#define     LSB_MAX_CLK     (80 * MHz_UNIT)
#define     SPI_MAX_CLK     (50 * MHz_UNIT)

#define     RANGE_MAX       3


extern u8 div0_1357[] ;
extern u8 div1_1248[] ;

#define      SYS_PLL_USING_FRAC_MODE   0

enum {
    LIMIT_HSB_CLK,
    LIMIT_LSB_CLK,
    LIMIT_MAX,
};

static u32 sys_clock_limit[RANGE_MAX][LIMIT_MAX] = {
    // HSB       | LSB
    { 96 * MHz_UNIT,  48 * MHz_UNIT},
    {120 * MHz_UNIT,  60 * MHz_UNIT},
    {160 * MHz_UNIT,  80 * MHz_UNIT},
};

static  u8 sys_dvdd_tbl[RANGE_MAX][2] = {
    {DVDD_VOL_SEL_102V, DCVDD_VOL_SEL_125V},
    {DVDD_VOL_SEL_114V, DCVDD_VOL_SEL_125V},
    {DVDD_VOL_SEL_123V, DCVDD_VOL_SEL_125V},
};

static void clock_voltage_scaling(u32 range)
{
    dvdd_vol_sel(sys_dvdd_tbl[range][0]);
    dcvdd_vol_sel(sys_dvdd_tbl[range][1]);
}

static u32 clock_adaptive_voltage_scaling(u32 hsb_freq, u32 lsb_freq)
{
    u32 range = 0;
    int i;
    for (i = 0; i < RANGE_MAX; i++) {
        if (hsb_freq <= sys_clock_limit[i][LIMIT_HSB_CLK]) {
            break;
        }
    }
    int j;
    for (j = 0; j < RANGE_MAX; j++) {
        if (lsb_freq <= sys_clock_limit[j][LIMIT_LSB_CLK]) {
            break;
        }
    }
    i = i > j ? i : j;

    range = (i == RANGE_MAX) ? RANGE_MAX - 1 : i;
    clock_voltage_scaling(range);
    return range;
}
__attribute__((always_inline))
u32 sys_clock_get(void)
{
    return sys_clock;
}

void clk_out0(u8 gpio, enum CLK_OUT_SOURCE clk)
{
    SFR(JL_CLOCK->CLK_CON0, 9, 4, clk);
    SFR(JL_IOMC->OCH_CON0, 0, 4, 3);
    gpio_set_direction(gpio, 0);
    gpio_och_sel_output_signal(gpio, OUTPUT_CH_SIGNAL_CLOCK_OUT0);
}
void clk_out1(u8 gpio, enum CLK_OUT_SOURCE clk)
{
    SFR(JL_CLOCK->CLK_CON0, 13, 4, clk);
    SFR(JL_IOMC->OCH_CON0, 0, 4, 3);
    gpio_set_direction(gpio, 0);
    gpio_och_sel_output_signal(gpio, OUTPUT_CH_SIGNAL_CLOCK_OUT1);
}
void clk_out2(u8 gpio, enum CLK_OUT2_SOURCE clk, u32 div)
{
    SFR(JL_CLOCK->CLK_CON0, 17, 4, clk);
    SFR(JL_CLOCK->CLK_CON0, 21, 6, div);
    SFR(JL_IOMC->OCH_CON0, 0, 4, 3);
    gpio_set_direction(gpio, 0);

    gpio_och_sel_output_signal(gpio, OUTPUT_CH_SIGNAL_CLOCK_OUT2);
}

const u32 pll_clock_tab0[]  = {
    0,
    137000000L,
    192000000L,
    240000000L,
    320000000L,
    480000000L,
};

const u8 div_taba[] = {
    1, 3, 5, 7
};

const u8 div_tabb[] = {
    1, 2, 4, 8
};

__attribute__((always_inline))
void main_clock_sel(u32 sys_clk_src)
{
    SFR(JL_CLOCK->SYS_SEL, 0, 3, sys_clk_src);
    asm("csync");
    asm("nop");
}

__attribute__((always_inline))
int get_sys_pll_sel()
{
    return JL_CLOCK->CLK_CON1 & 0x7;
}

__attribute__((always_inline))
u32 get_sys_pll_clk()
{
    u32 t_sel;
    u32 clock = 0;
    t_sel = get_sys_pll_sel();
    clock = pll_clock_tab0[t_sel];
    if (0 == clock) {
        log_info(" sys clock info err\n");
        return 0;
    }
    u32 t_diva = div_taba[(JL_CLOCK->CLK_CON1 >> 3) & 0x3];
    u32 t_divb = div_tabb[(JL_CLOCK->CLK_CON1 >> 5) & 0x3];
    return clock / (t_diva * t_divb);
}

__attribute__((always_inline))
u32 lsb_clk_get(void)
{
    u32 div42 = ((JL_CLOCK->SYS_DIV >> 8) & 0xf) + 1;
    u32 lsb_clk = sys_clock_get() / (div42);

    return lsb_clk;
}

__attribute__((always_inline))
u32 sys_clock_peration(void)
{
    u32 hsb_div = (JL_CLOCK->SYS_DIV & 0xff) + 1;
    u32 pll_sys_clk = get_sys_pll_clk();
    u32 hsb_clk = pll_sys_clk / (hsb_div);

    /* log_info(" sys clock %ld\n", hsb_clk); */
    return hsb_clk;
}

__attribute__((always_inline))
void hsb_clk_div(u32 div)
{
    SFR(JL_CLOCK->SYS_DIV, 0, 8, div);
}
__attribute__((always_inline))
void lsb_clk_div(u32 div)
{
    SFR(JL_CLOCK->SYS_DIV, 8, 4, div);
}

AT(.clock.text.cache.L1)
__attribute__((noinline))
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

AT(.clock.text.cache.L1)
__attribute__((noinline))
void spi_baud_set(u32 baud)
{
    JL_SPI0->BAUD = baud;
}

static u32 sfc_max_baud(u32 pll_clock, _PLL_DIV pll_div)
{
    sfc_clk = get_sys_pll_clk();
    u32 baud = 0;
    while ((sfc_clk / (baud + 1)) > SPI_MAX_CLK) {
        baud++;
    }
    return baud;
}
/* AT(.clock.text.cache.L1) */
void pll_sel(u32 pll_clock, _PLL_DIV pll_div, _HSB_CLK_DIV hsb_div)
{
    u32 clock;
    local_irq_disable();
    main_clock_sel(SYS_SRC_RC16M);

    SFR(JL_CLOCK->CLK_CON1, 0, 3, pll_clock);
    SFR(JL_CLOCK->CLK_CON1, 3, 4, pll_div);

    hsb_clk_div(hsb_div);
    lsb_clk_div(0);

    sys_clock = sys_clock_peration();
    clock = sys_clock;

    for (u32 i = 0; i < 8; i++) {
        clock = sys_clock / (i + 1);
        if (clock <= LSB_MAX_CLK) {
            lsb_clk_div(i);
            break;
        }
    }

    u32 lsb_clk = clk_get("lsb");
    u32 range = clock_adaptive_voltage_scaling(sys_clock, lsb_clk);

    u32 baud = sfc_max_baud(pll_clock, pll_div);
    sfc_baud_set(baud);
    u32 tmp_baud;
    tmp_baud = baud < 3 ? 3 : baud;
    spi_baud_set(tmp_baud);
    main_clock_sel(SYS_SRC_PLL_SYS_CLK);
    delay(10);

    local_irq_enable();

    log_info("---SFC CLK : %d", sfc_clk);
    log_info("---SPI CLK : %d", sfc_clk / (baud + 1));
    log_info("---HSB CLK : %d", sys_clock);
    log_info("---LSB CLK : %d", clk_get("lsb"));
    log_info("---RANGE   : %d", range);

    if (sys_clock > sys_clock_limit[RANGE_MAX - 1][0]) {
        log_error("sys_clock over limt %d > %d",
                  sys_clock, sys_clock_limit[RANGE_MAX - 1][0]);
        while (1);
    }
    if (sfc_clk > sys_clock_limit[RANGE_MAX - 1][0]) {
        log_error("sfc_clk over limt %d > %d",
                  sfc_clk, sys_clock_limit[RANGE_MAX - 1][0]);
        while (1);
    }
    /* clk_out0(IO_PORTA_07,CLK_OUT_HSB); */
    /* clk_out0(IO_PORTA_07,CLK_OUT_LSB); */
    /* clk_out0(IO_PORTA_07,CLK_OUT_LRC_200K); */
    /* clk_out1(IO_PORTA_07, CLK_OUT_STD_48M); */
    /* clk_out2(IO_PORTA_07, CLK_OUT2_SYSPLL_D1P0,8); */
}
__attribute__((always_inline_when_const_args))
int clk_get(const char *name)
{
    if (!strcmp(name, "sys")) {
        return sys_clock_get();
    }
    if (!strcmp(name, "uart")) {
        return (48 * 1000000);
    }
    if (!strcmp(name, "timer")) {
        return (48 * 1000000);
    }
    if (!strcmp(name, "lsb")) {
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
    /* if (!strcmp(name, "lrc")) { */
    /*     return lrc_clk_get();   */
    /* }                           */
    return 0;
}
