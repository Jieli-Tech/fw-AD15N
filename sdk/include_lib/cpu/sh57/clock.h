#ifndef _CLOCK_
#define _CLOCK_

u32 sys_clock_get(void);
int clk_get(const char *name);

#define PLL_137M    0b001  //d3p5
#define PLL_192M    0b010  //d2p5
#define PLL_240M    0b011  //d2p0
#define PLL_320M    0b100  //d1p5
#define PLL_480M    0b101  //d1p0

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

} _HSB_CLK_DIV;

#define     SYS_SRC_RC16M           0
#define     SYS_SRC_EXT_CLK         1
#define     SYS_SRC_RC250K          2
#define     SYS_SRC_LCR_CLK         3
#define     SYS_SRC_PLL_SYS_CLK         5

#define MHz_UNIT	1000000L
#define KHz_UNIT	1000L
void pll_sel(u32 pll_clock, _PLL_DIV pll_div, _HSB_CLK_DIV pll_b_div);
#endif
