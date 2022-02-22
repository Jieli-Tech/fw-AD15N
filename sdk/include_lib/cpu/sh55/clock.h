#ifndef _CLOCK_
#define _CLOCK_

// enum CLK_OUT_SOURCE {
// NONE = 0,
// RCLK,
// ALNK_CLK,
// LSB_CLK,
// HSB_CLK,
// SFC_CLK,
// LRC_CLK_8DIV1,
// LRC_CLK,
// PLL_48M,
// P11_24M,
// };

// extern u32 clock_get_uart(void);

extern int clk_get(const char *name);

// void clk_out(enum CLK_OUT_SOURCE clk);

#define PLL_192M    0b1100
#define PLL_137M    0b1000
#define PLL_107M    0b0100
#define PLL_96M     0b0001
#define PLL_320M    0b0010
#define PLL_480M    0b0011

typedef enum {
    PLL_DIV1 = 0x0,
    PLL_DIV3,
    PLL_DIV5,
    PLL_DIV7,
    PLL_DIV2 = 0x4,
    PLL_DIV6,
    PLL_DIV10,
    PLL_DIV14,

    PLL_DIV4 = 0x8,
    PLL_DIV12,
    PLL_DIV20,
    PLL_DIV28,

    PLL_DIV8 = 0xc,
    PLL_DIV24,
    PLL_DIV40,
    PLL_DIV56,

} _PLL_DIV;

typedef enum {
    PLL_B_DIV1 = 0x0,
    PLL_B_DIV2,
    PLL_B_DIV3,
    PLL_B_DIV4,
    PLL_B_DIV5,
    PLL_B_DIV6,
    PLL_B_DIV7,
    PLL_B_DIV8,

} _PLL_B_DIV;

typedef enum {
    CLK_OUTPUT_NONE = 0,
    CLK_OUTPUT_RCLK = 1,
    CLK_OUTPUT_OSC_CLK = 2,
    CLK_OUTPUT_PLL_TDM_CLK = 3,
    CLK_OUTPUT_LSB_CLK = 4,
    CLK_OUTPUT_HSB_CLK = 5,
    CLK_OUTPUT_SFC_CLK = 6,
    CLK_OUTPUT_IRC_CLKX8 = 7,
    CLK_OUTPUT_IRC_CLK = 8,
    CLK_OUTPUT_PLL_48M = 9,
    CLK_OUTPUT_PLL_24M = 10,
} _CLK_OUTPUT_TYPE;

void pll_sel(u32 pll_clock, _PLL_DIV pll_div, _PLL_B_DIV pll_b_div);
u32 sys_clock_get(void);
u32 sys_clock_peration(void);
void uart_clk_sel();
void clk_output_sel(u8 clk);

#endif
