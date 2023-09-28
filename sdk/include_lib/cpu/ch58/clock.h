#ifndef _CPU_CLOCK_
#define _CPU_CLOCK_

#include "typedef.h"

#include "clock_define.h"

/*
 * system enter critical and exit critical handle
 * */
struct clock_critical_handler {
    void (*enter)();
    void (*exit)();
};

#define HSB_CRITICAL_HANDLE_REG(name, enter, exit) \
	const struct clock_critical_handler hsb_##name \
		 SEC(.hsb_critical_txt) = {enter, exit};

extern struct clock_critical_handler hsb_critical_handler_begin[];
extern struct clock_critical_handler hsb_critical_handler_end[];

#define list_for_each_loop_hsb_critical(h) \
	for (h=hsb_critical_handler_begin; h<hsb_critical_handler_end; h++)

#define LSB_CRITICAL_HANDLE_REG(name, enter, exit) \
	const struct clock_critical_handler lsb_##name \
		 SEC(.lsb_critical_txt) = {enter, exit};

extern struct clock_critical_handler lsb_critical_handler_begin[];
extern struct clock_critical_handler lsb_critical_handler_end[];

#define list_for_each_loop_lsb_critical(h) \
	for (h=lsb_critical_handler_begin; h<lsb_critical_handler_end; h++)

void clock_set_sfc_max_freq(u32 max_freq);

void clk_voltage_init(u8 mode, u8 sys_dvdd);

int clk_early_init(u8 sys_in, u32 input_freq, u32 pll_freq);

int clk_get(const char *name);

int clk_set(const char *name, int clk);

int clk_set_sys_lock(int clk, int lock_en);

void fast_boot_mode_clk(void);

enum CLK_OUT_SOURCE {
    NONE_CLK_OUT,
    RTC_OSC_CLK_OUT,
    LRC_CLK_OUT,
    STD_12M_CLK_OUT,
    STD_24M_CLK_OUT,
    STD_48M_CLK_OUT,
    HSB_CLK_OUT,
    LSB_CLK_OUT,
    PLL_96M_CLK_OUT,
    RC_250K_CLK_OUT,
    RC_16M_CLK_OUT,
    USB_CLK_OUT,
};

enum CLK_OUT_SOURCE_2 {
    NONE_CLK_OUT_2,
    RTC_OSC_CLK_OUT_2,
    LRC_CLK_OUT_2,
    SYS_PLL_D3P5_CLK_OUT_2,
    SYS_PLL_D2P5_CLK_OUT_2,
    SYS_PLL_D2P0_CLK_OUT_2,
    SYS_PLL_D1P5_CLK_OUT_2,
    SYS_PLL_D1P0_CLK_OUT_2,
};

void clk_out(u8 gpio, enum CLK_OUT_SOURCE clk);
void clk_out1(u8 gpio, enum CLK_OUT_SOURCE clk);
void clk_out2(u8 gpio, enum CLK_OUT_SOURCE_2 clk, u8 div);
void clk_out_close(u8 gpio);
void clk_out1_close(u8 gpio);
void clk_out2_close(u8 gpio);

void clock_dump(void);

#define MHz_UNIT    (1000000L)
#define KHz_UNIT    (1000L)
#define MHz	        (1000000L)


enum clk_mode {
    CLOCK_MODE_ADAPTIVE = 0,
    CLOCK_MODE_USR,
};

void udelay(u32 us);
void mdelay(u32 ms);

enum {
    USB_TRIM_HAND,  //手动校准模式
    USB_TRIM_AUTO,  //full_speed自动校准模式
};

#define FUSB_TRIM_CON0      JL_PLL0->TRIM_CON0
#define FUSB_TRIM_CON1      JL_PLL0->TRIM_CON1
#define FUSB_TRIM_PND       JL_PLL0->TRIM_PND
#define FUSB_FRQ_CNT        JL_PLL0->FRQ_CNT
#define FUSB_FRC_SCA        JL_PLL0->FRC_SCA
#define FUSB_PLL_CON0       JL_PLL0->CON0
#define FUSB_PLL_CON1       JL_PLL0->CON1
#define FUSB_PLL_NR         JL_PLL0->NR

u8 fusb_pll_trim(u8 mode, u16 trim_prd);
u32 sys_clock_get(void);

#endif

