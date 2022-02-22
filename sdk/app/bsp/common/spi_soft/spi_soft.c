#include "gpio.h"
#include "clock.h"
#include "spi_soft.h"
#include "spi1.h"
#include "includes.h"
#include "typedef.h"
#include "cpu.h"
#include "errno-base.h"
#include "config.h"
#include "app_config.h"

/* #define LOG_TAG_CONST NORM */
#define LOG_TAG     "[spi_soft]"
//#define LOG_ERROR_ENABLE
//#define LOG_DEBUG_ENABLE
#include "debug.h"

// | func\port |  A   |  B   |
// |-----------|------|------|
// | CLK       | PA11  | PA1  |
// | DO(D0)    | PA12  | PA2  |
// | DI(D1)    | PA10  | PA3 |
// | WP(D2)    | PA8   | PA8 |
// | HOLD(D3)  | PA9   | PA9 |

/********************soft spi0********************/
//-----------TEAM A spi io set
#define   A_DATA_WIDE_MODE SOFT_SPI_WORK_MODE//只支持双向或单线
#define   A_WORK_MODE      SPI_CPOL1_CPHA1//enum spi_soft_work_mode中的枚举常量
#define   A_SPI_DELAY      1 //软件spi延时参数，影响通讯时钟频率,无效
#define   A_SPI_ROLE       SPI_ROLE_MASTER//只支持主机


#define SOFT_SPI_B_GROUP_EN 0
#if(SOFT_SPI_B_GROUP_EN)
/********************soft spi1********************/
//-----------TEAM B spi io set
// set clk
#define B_CLK_BIT           BIT(11)
#define B_CLK_PORT(x)       JL_PORTA->x
// set d0
#define B_D0_BIT            BIT(12)
#define B_D0_PORT(x)        JL_PORTA->x
// set d1
#define B_D1_BIT            BIT(10)
#define B_D1_PORT(x)        JL_PORTA->x
// set d2
/* #define B_D2_BIT            BIT(8) */
/* #define B_D2_PORT(x)        JL_PORTA->x */
/* // set d3 */
/* #define B_D3_BIT            BIT(9) */
/* #define B_D3_PORT(x)        JL_PORTA->x */
#define   B_DATA_WIDE_MODE SPI_MODE_BIDIR_1BIT//SPI_MODE_UNIDIR_1BIT//
#define   B_WORK_MODE      SPI_CPOL1_CPHA1
#define   B_SPI_DELAY      1 //软件spi延时参数，影响通讯时钟频率,无效
#define   B_SPI_ROLE       SPI_ROLE_MASTER
#endif

struct _dat_bit {
    u8 bit0 : 1;
    u8 bit1 : 1;
    u8 bit2 : 1;
    u8 bit3 : 1;
    u8 bit4 : 1;
    u8 bit5 : 1;
    u8 bit6 : 1;
    u8 bit7 : 1;
};


#define bit_delay_r()  asm("nop");asm("nop");asm("nop");asm("nop")
#define bit_delay_w()  asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop")
static void soft_spi_io_port_init(spi_soft_dev spi)
{
    if (spi == 0) {
        A_CLK_PORT(DIE) |= A_CLK_BIT;
        A_CLK_PORT(PU) |= A_CLK_BIT;
        A_CLK_PORT(PD) &= ~A_CLK_BIT;
        A_D0_PORT(OUT) |= A_D0_BIT;
        A_D0_PORT(DIR) &= ~A_D0_BIT;
        A_D0_PORT(DIE) |= A_D0_BIT;
        A_D0_PORT(PU) |= A_D0_BIT;
        A_D0_PORT(PD) &= ~A_D0_BIT;
#if(SOFT_SPI_B_GROUP_EN)
    } else {
        B_CLK_PORT(DIE) |= B_CLK_BIT;
        B_CLK_PORT(PU) |= B_CLK_BIT;
        B_CLK_PORT(PD) &= ~B_CLK_BIT;
        B_D0_PORT(OUT) |= B_D0_BIT;
        B_D0_PORT(DIR) &= ~B_D0_BIT;
        B_D0_PORT(DIE) |= B_D0_BIT;
        B_D0_PORT(PU) |= B_D0_BIT;
        B_D0_PORT(PD) &= ~B_D0_BIT;
#endif
    }
}
static void soft_spi_io_port_uninit(spi_soft_dev spi)
{
    if (spi == 0) {
        A_CLK_PORT(DIR) |= A_CLK_BIT;
        A_CLK_PORT(DIE) &= ~A_CLK_BIT;
        A_CLK_PORT(PU) &= ~A_CLK_BIT;
        A_CLK_PORT(PD) &= ~A_CLK_BIT;
        A_D0_PORT(DIR) |= A_D0_BIT;
        A_D0_PORT(DIE) &= ~A_D0_BIT;
        A_D0_PORT(PU) &= ~A_D0_BIT;
        A_D0_PORT(PD) &= ~A_D0_BIT;
#if(SOFT_SPI_B_GROUP_EN)
    } else {
        B_CLK_PORT(DIR) |= B_CLK_BIT;
        B_CLK_PORT(DIE) &= ~B_CLK_BIT;
        B_CLK_PORT(PU) &= ~B_CLK_BIT;
        B_CLK_PORT(PD) &= ~B_CLK_BIT;
        B_D0_PORT(DIR) |= B_D0_BIT;
        B_D0_PORT(DIE) &= ~B_D0_BIT;
        B_D0_PORT(PU) &= ~B_D0_BIT;
        B_D0_PORT(PD) &= ~B_D0_BIT;
#endif
    }
}

void soft_spi_set_bit_mode(spi_soft_dev spi, int data_wide)
{
    u8 data_wide_mode = 0, role = 0, work_mode = 3;
    if (spi == 0) {
        data_wide_mode = A_DATA_WIDE_MODE;
        role = A_SPI_ROLE;
        work_mode = A_WORK_MODE;
        if (role == SPI_ROLE_MASTER) {
            if (work_mode <= SPI_CPOL0_CPHA1) {
                A_CLK_PORT(OUT) &= ~A_CLK_BIT;
            } else {
                A_CLK_PORT(OUT) |= A_CLK_BIT;
            }
            A_CLK_PORT(DIR) &= ~A_CLK_BIT;
        } else { //不支持从机
            A_CLK_PORT(DIR) |= A_CLK_BIT;
        }
#if(SOFT_SPI_B_GROUP_EN)
    } else {
        data_wide_mode = B_DATA_WIDE_MODE;
        role = B_SPI_ROLE;
        work_mode = B_WORK_MODE;
        if (role == SPI_ROLE_MASTER) {
            if (work_mode <= SPI_CPOL0_CPHA1) {
                B_CLK_PORT(OUT) &= ~B_CLK_BIT;
            } else {
                B_CLK_PORT(OUT) |= B_CLK_BIT;
            }
            B_CLK_PORT(DIR) &= ~B_CLK_BIT;
            /* log_info("b clk:%d",B_CLK_BIT); */
        } else { //不支持从机
            B_CLK_PORT(DIR) |= B_CLK_BIT;
        }
#endif
    }
    switch (data_wide_mode) {
    case SPI_MODE_BIDIR_1BIT:
        if (spi == 0) {
            A_D1_PORT(DIR) |= A_D1_BIT;
            A_D1_PORT(DIE) |= A_D1_BIT;
            A_D1_PORT(PU) &= ~A_D1_BIT;
            A_D1_PORT(PD) &= ~A_D1_BIT;
#if(SOFT_SPI_B_GROUP_EN)
        } else {
            B_D1_PORT(DIR) |= B_D1_BIT;
            B_D1_PORT(DIE) |= B_D1_BIT;
            B_D1_PORT(PU) &= ~B_D1_BIT;
            B_D1_PORT(PD) &= ~B_D1_BIT;
#endif
        }
        break;
    case SPI_MODE_UNIDIR_1BIT:
        break;
    case SPI_MODE_UNIDIR_2BIT://不支持
        if (spi == 0) {
            A_D1_PORT(DIR) &= ~A_D1_BIT;
            A_D1_PORT(DIE) |= A_D1_BIT;
            A_D1_PORT(PU) &= ~A_D1_BIT;
            A_D1_PORT(PD) &= ~A_D1_BIT;
#if(SOFT_SPI_B_GROUP_EN)
        } else {
            B_D1_PORT(DIR) &= ~B_D1_BIT;
            B_D1_PORT(DIE) |= B_D1_BIT;
            B_D1_PORT(PU) &= ~B_D1_BIT;
            B_D1_PORT(PD) &= ~B_D1_BIT;
#endif
        }
        break;
    case SPI_MODE_UNIDIR_4BIT://不支持
        break;
    }
    soft_spi_io_port_init(spi);
}
int soft_spi_open(spi_soft_dev spi)
{
    soft_spi_set_bit_mode(spi, 0);
    /* log_info("soft spi open"); */
    return 0;
}

void soft_spi_close(spi_soft_dev spi)
{
    u8 data_wide_mode = 0;
    if (spi == 0) {
        data_wide_mode = A_DATA_WIDE_MODE;
#if(SOFT_SPI_B_GROUP_EN)
    } else {
        data_wide_mode = B_DATA_WIDE_MODE;
#endif
    }
    switch (data_wide_mode) {
    case SPI_MODE_BIDIR_1BIT:
    case SPI_MODE_UNIDIR_2BIT://不支持
        if (spi == 0) {
            A_D1_PORT(DIR) |= A_D1_BIT;
            A_D1_PORT(DIE) &= ~A_D1_BIT;
            A_D1_PORT(PU) &= ~A_D1_BIT;
            A_D1_PORT(PD) &= ~A_D1_BIT;
#if(SOFT_SPI_B_GROUP_EN)
        } else {
            B_D1_PORT(DIR) |= B_D1_BIT;
            B_D1_PORT(DIE) &= ~B_D1_BIT;
            B_D1_PORT(PU) &= ~B_D1_BIT;
            B_D1_PORT(PD) &= ~B_D1_BIT;
#endif
        }
        break;
    case SPI_MODE_UNIDIR_1BIT:
        break;
    case SPI_MODE_UNIDIR_4BIT://不支持
        break;
    }
    soft_spi_io_port_uninit(spi);
}
void soft_spi_suspend(spi_soft_dev spi)
{
    soft_spi_close(spi);
}
void soft_spi_resume(spi_soft_dev spi)
{
    soft_spi_open(spi);
}

/* mode1: CPOL=0，CPHA=1, MSB first, clk_l, smp_edge_fall*/
/* mode2: CPOL=1，CPHA=0, MSB first, clk_h, smp_edge_fall*/
static void soft_spi_a_w_mode12(spi_soft_dev spi, u8 write_dat)
{
    struct _dat_bit *_dat = (struct _dat_bit *)&write_dat;
    A_D0_PORT(DIR) &= ~A_D0_BIT;
    local_irq_disable();
    if (A_WORK_MODE == SPI_CPOL0_CPHA1) {
        A_CLK_PORT(OUT) |=  A_CLK_BIT;
    }
    _dat->bit7 ? A_D0_PORT(OUT) |= A_D0_BIT : (A_D0_PORT(OUT) &= ~A_D0_BIT);
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    bit_delay_w();
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    _dat->bit6 ? A_D0_PORT(OUT) |= A_D0_BIT : (A_D0_PORT(OUT) &= ~A_D0_BIT);
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    bit_delay_w();
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    _dat->bit5 ? A_D0_PORT(OUT) |= A_D0_BIT : (A_D0_PORT(OUT) &= ~A_D0_BIT);
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    bit_delay_w();
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    _dat->bit4 ? A_D0_PORT(OUT) |= A_D0_BIT : (A_D0_PORT(OUT) &= ~A_D0_BIT);
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    bit_delay_w();
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    _dat->bit3 ? A_D0_PORT(OUT) |= A_D0_BIT : (A_D0_PORT(OUT) &= ~A_D0_BIT);
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    bit_delay_w();
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    _dat->bit2 ? A_D0_PORT(OUT) |= A_D0_BIT : (A_D0_PORT(OUT) &= ~A_D0_BIT);
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    bit_delay_w();
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    _dat->bit1 ? A_D0_PORT(OUT) |= A_D0_BIT : (A_D0_PORT(OUT) &= ~A_D0_BIT);
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    bit_delay_w();
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    _dat->bit0 ? A_D0_PORT(OUT) |= A_D0_BIT : (A_D0_PORT(OUT) &= ~A_D0_BIT);
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    if (A_WORK_MODE == SPI_CPOL1_CPHA0) {
        bit_delay_w();
        A_CLK_PORT(OUT) |=  A_CLK_BIT;
    }
    local_irq_enable();
    A_D0_PORT(OUT) &= ~A_D0_BIT;
}
/* mode1: CPOL=0，CPHA=1, MSB first, clk_l, smp_edge_fall*/
/* mode2: CPOL=1，CPHA=0, MSB first, clk_h, smp_edge_fall*/
static u8 soft_spi_a_r_mode12(spi_soft_dev spi)
{
    u8 read_dat = 0;
    struct _dat_bit *_dat = (struct _dat_bit *)&read_dat;
    A_D1_PORT(DIR) |= A_D1_BIT;
    local_irq_disable();
    if (A_WORK_MODE == SPI_CPOL0_CPHA1) {
        A_CLK_PORT(OUT) |=  A_CLK_BIT;
        bit_delay_r();
    }
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    (A_D1_PORT(IN)& A_D1_BIT) ? _dat->bit7 = 1 : (_dat->bit7 = 0);
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    bit_delay_r();
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    (A_D1_PORT(IN)& A_D1_BIT) ? _dat->bit6 = 1 : (_dat->bit6 = 0);
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    bit_delay_r();
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    (A_D1_PORT(IN)& A_D1_BIT) ? _dat->bit5 = 1 : (_dat->bit5 = 0);
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    bit_delay_r();
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    (A_D1_PORT(IN)& A_D1_BIT) ? _dat->bit4 = 1 : (_dat->bit4 = 0);
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    bit_delay_r();
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    (A_D1_PORT(IN)& A_D1_BIT) ? _dat->bit3 = 1 : (_dat->bit3 = 0);
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    bit_delay_r();
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    (A_D1_PORT(IN)& A_D1_BIT) ? _dat->bit2 = 1 : (_dat->bit2 = 0);
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    bit_delay_r();
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    (A_D1_PORT(IN)& A_D1_BIT) ? _dat->bit1 = 1 : (_dat->bit1 = 0);
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    bit_delay_r();
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    (A_D1_PORT(IN)& A_D1_BIT) ? _dat->bit0 = 1 : (_dat->bit0 = 0);
    if (A_WORK_MODE == SPI_CPOL1_CPHA0) {
        A_CLK_PORT(OUT) |=  A_CLK_BIT;
    }
    local_irq_enable();
    return read_dat;
}

/* mode0: CPOL = 0, CPHA = 0, MSB first, clk_l, smp_edge_rise*/
/* mode3: CPOL = 1, CPHA = 1, MSB first, clk_h, smp_edge_rise */
static void soft_spi_a_w_mode03(spi_soft_dev spi, u8 write_dat)
{
    struct _dat_bit *_dat = (struct _dat_bit *)&write_dat;
    A_D0_PORT(DIR) &= ~A_D0_BIT;
    local_irq_disable();
    if (A_WORK_MODE == SPI_CPOL1_CPHA1) {
        A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    }
    _dat->bit7 ? A_D0_PORT(OUT) |= A_D0_BIT : (A_D0_PORT(OUT) &= ~A_D0_BIT);
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    bit_delay_w();
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    _dat->bit6 ? A_D0_PORT(OUT) |= A_D0_BIT : (A_D0_PORT(OUT) &= ~A_D0_BIT);
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    bit_delay_w();
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    _dat->bit5 ? A_D0_PORT(OUT) |= A_D0_BIT : (A_D0_PORT(OUT) &= ~A_D0_BIT);
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    bit_delay_w();
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    _dat->bit4 ? A_D0_PORT(OUT) |= A_D0_BIT : (A_D0_PORT(OUT) &= ~A_D0_BIT);
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    bit_delay_w();
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    _dat->bit3 ? A_D0_PORT(OUT) |= A_D0_BIT : (A_D0_PORT(OUT) &= ~A_D0_BIT);
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    bit_delay_w();
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    _dat->bit2 ? A_D0_PORT(OUT) |= A_D0_BIT : (A_D0_PORT(OUT) &= ~A_D0_BIT);
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    bit_delay_w();
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    _dat->bit1 ? A_D0_PORT(OUT) |= A_D0_BIT : (A_D0_PORT(OUT) &= ~A_D0_BIT);
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    bit_delay_w();
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    _dat->bit0 ? A_D0_PORT(OUT) |= A_D0_BIT : (A_D0_PORT(OUT) &= ~A_D0_BIT);
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    if (A_WORK_MODE == SPI_CPOL0_CPHA0) {
        bit_delay_w();
        A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    }
    local_irq_enable();
    A_D0_PORT(OUT) &= ~A_D0_BIT;
}
/* mode0: CPOL = 0, CPHA = 0, MSB first, clk_l, smp_edge_rise*/
/* mode3: CPOL = 1, CPHA = 1, MSB first, clk_h, smp_edge_rise */
static u8 soft_spi_a_r_mode03(spi_soft_dev spi)
{
    u8 read_dat = 0;
    struct _dat_bit *_dat = (struct _dat_bit *)&read_dat;
    A_D1_PORT(DIR) |= A_D1_BIT;
    local_irq_disable();
    if (A_WORK_MODE == SPI_CPOL1_CPHA1) {
        A_CLK_PORT(OUT) &= ~A_CLK_BIT;
        bit_delay_r();
    }
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    (A_D1_PORT(IN)& A_D1_BIT) ? _dat->bit7 = 1 : (_dat->bit7 = 0);
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    bit_delay_r();
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    (A_D1_PORT(IN)& A_D1_BIT) ? _dat->bit6 = 1 : (_dat->bit6 = 0);
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    bit_delay_r();
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    (A_D1_PORT(IN)& A_D1_BIT) ? _dat->bit5 = 1 : (_dat->bit5 = 0);
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    bit_delay_r();
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    (A_D1_PORT(IN)& A_D1_BIT) ? _dat->bit4 = 1 : (_dat->bit4 = 0);
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    bit_delay_r();
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    (A_D1_PORT(IN)& A_D1_BIT) ? _dat->bit3 = 1 : (_dat->bit3 = 0);
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    bit_delay_r();
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    (A_D1_PORT(IN)& A_D1_BIT) ? _dat->bit2 = 1 : (_dat->bit2 = 0);
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    bit_delay_r();
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    (A_D1_PORT(IN)& A_D1_BIT) ? _dat->bit1 = 1 : (_dat->bit1 = 0);
    A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    bit_delay_r();
    A_CLK_PORT(OUT) |=  A_CLK_BIT;
    (A_D1_PORT(IN)& A_D1_BIT) ? _dat->bit0 = 1 : (_dat->bit0 = 0);
    if (A_WORK_MODE == SPI_CPOL0_CPHA0) {
        A_CLK_PORT(OUT) &= ~A_CLK_BIT;
    }
    local_irq_enable();
    return read_dat;
}

#if(SOFT_SPI_B_GROUP_EN)
/* mode1: CPOL=0，CPHA=1, MSB first, clk_l, smp_edge_fall*/
/* mode2: CPOL=1，CPHA=0, MSB first, clk_h, smp_edge_fall*/
static void soft_spi_b_w_mode12(spi_soft_dev spi, u8 write_dat)
{
    struct _dat_bit *_dat = (struct _dat_bit *)&write_dat;
    B_D0_PORT(DIR) &= ~B_D0_BIT;
    local_irq_disable();
    if (B_WORK_MODE == SPI_CPOL0_CPHA1) {
        B_CLK_PORT(OUT) |=  B_CLK_BIT;
    }
    _dat->bit7 ? B_D0_PORT(OUT) |= B_D0_BIT : (B_D0_PORT(OUT) &= ~B_D0_BIT);
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    bit_delay_w();
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    _dat->bit6 ? B_D0_PORT(OUT) |= B_D0_BIT : (B_D0_PORT(OUT) &= ~B_D0_BIT);
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    bit_delay_w();
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    _dat->bit5 ? B_D0_PORT(OUT) |= B_D0_BIT : (B_D0_PORT(OUT) &= ~B_D0_BIT);
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    bit_delay_w();
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    _dat->bit4 ? B_D0_PORT(OUT) |= B_D0_BIT : (B_D0_PORT(OUT) &= ~B_D0_BIT);
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    bit_delay_w();
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    _dat->bit3 ? B_D0_PORT(OUT) |= B_D0_BIT : (B_D0_PORT(OUT) &= ~B_D0_BIT);
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    bit_delay_w();
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    _dat->bit2 ? B_D0_PORT(OUT) |= B_D0_BIT : (B_D0_PORT(OUT) &= ~B_D0_BIT);
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    bit_delay_w();
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    _dat->bit1 ? B_D0_PORT(OUT) |= B_D0_BIT : (B_D0_PORT(OUT) &= ~B_D0_BIT);
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    bit_delay_w();
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    _dat->bit0 ? B_D0_PORT(OUT) |= B_D0_BIT : (B_D0_PORT(OUT) &= ~B_D0_BIT);
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    if (B_WORK_MODE == SPI_CPOL1_CPHA0) {
        bit_delay_w();
        B_CLK_PORT(OUT) |=  B_CLK_BIT;
    }
    local_irq_enable();
    B_D0_PORT(OUT) &= ~B_D0_BIT;
}
/* mode1: CPOL=0，CPHA=1, MSB first, clk_l, smp_edge_fall*/
/* mode2: CPOL=1，CPHA=0, MSB first, clk_h, smp_edge_fall*/
static u8 soft_spi_b_r_mode12(spi_soft_dev spi)
{
    u8 read_dat = 0;
    struct _dat_bit *_dat = (struct _dat_bit *)&read_dat;
    B_D1_PORT(DIR) |= B_D1_BIT;
    local_irq_disable();
    if (B_WORK_MODE == SPI_CPOL0_CPHA1) {
        B_CLK_PORT(OUT) |=  B_CLK_BIT;
        bit_delay_r();
    }
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    (B_D1_PORT(IN)& B_D1_BIT) ? _dat->bit7 = 1 : (_dat->bit7 = 0);
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    bit_delay_r();
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    (B_D1_PORT(IN)& B_D1_BIT) ? _dat->bit6 = 1 : (_dat->bit6 = 0);
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    bit_delay_r();
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    (B_D1_PORT(IN)& B_D1_BIT) ? _dat->bit5 = 1 : (_dat->bit5 = 0);
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    bit_delay_r();
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    (B_D1_PORT(IN)& B_D1_BIT) ? _dat->bit4 = 1 : (_dat->bit4 = 0);
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    bit_delay_r();
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    (B_D1_PORT(IN)& B_D1_BIT) ? _dat->bit3 = 1 : (_dat->bit3 = 0);
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    bit_delay_r();
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    (B_D1_PORT(IN)& B_D1_BIT) ? _dat->bit2 = 1 : (_dat->bit2 = 0);
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    bit_delay_r();
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    (B_D1_PORT(IN)& B_D1_BIT) ? _dat->bit1 = 1 : (_dat->bit1 = 0);
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    bit_delay_r();
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    (B_D1_PORT(IN)& B_D1_BIT) ? _dat->bit0 = 1 : (_dat->bit0 = 0);
    if (B_WORK_MODE == SPI_CPOL1_CPHA0) {
        B_CLK_PORT(OUT) |=  B_CLK_BIT;
    }
    local_irq_enable();
    return read_dat;
}

/* mode0: CPOL = 0, CPHA = 0, MSB first, clk_l, smp_edge_rise*/
/* mode3: CPOL = 1, CPHA = 1, MSB first, clk_h, smp_edge_rise */
AT_RAM
static void soft_spi_b_w_mode03(spi_soft_dev spi, u8 write_dat)
{
    struct _dat_bit *_dat = (struct _dat_bit *)&write_dat;
    B_D0_PORT(DIR) &= ~B_D0_BIT;
    local_irq_disable();
    /* u32 D0_PORT = B_D0_PORT(OUT) & (~B_D0_BIT); */
    if (B_WORK_MODE == SPI_CPOL1_CPHA1) {
        B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    }
    /* B_D0_PORT(OUT) =  D0_PORT | (_dat->bit7 * B_D0_BIT); */
    _dat->bit7 ? B_D0_PORT(OUT) |= B_D0_BIT : (B_D0_PORT(OUT) &= ~B_D0_BIT);
    /* delay(10);#<{(| asm("nop"); |)}>#	#<{(| asm("csync"); |)}># */
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    bit_delay_w();
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    /* B_D0_PORT(OUT) =  D0_PORT | (_dat->bit6 * B_D0_BIT); */
    _dat->bit6 ? B_D0_PORT(OUT) |= B_D0_BIT : (B_D0_PORT(OUT) &= ~B_D0_BIT);
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    bit_delay_w();
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    /* B_D0_PORT(OUT) =  D0_PORT | (_dat->bit5 * B_D0_BIT); */
    _dat->bit5 ? B_D0_PORT(OUT) |= B_D0_BIT : (B_D0_PORT(OUT) &= ~B_D0_BIT);
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    bit_delay_w();
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    /* B_D0_PORT(OUT) =  D0_PORT | (_dat->bit4 * B_D0_BIT); */
    _dat->bit4 ? B_D0_PORT(OUT) |= B_D0_BIT : (B_D0_PORT(OUT) &= ~B_D0_BIT);
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    bit_delay_w();
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    /* B_D0_PORT(OUT) =  D0_PORT | (_dat->bit3 * B_D0_BIT); */
    _dat->bit3 ? B_D0_PORT(OUT) |= B_D0_BIT : (B_D0_PORT(OUT) &= ~B_D0_BIT);
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    bit_delay_w();
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    /* B_D0_PORT(OUT) =  D0_PORT | (_dat->bit2 * B_D0_BIT); */
    _dat->bit2 ? B_D0_PORT(OUT) |= B_D0_BIT : (B_D0_PORT(OUT) &= ~B_D0_BIT);
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    bit_delay_w();
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    /* B_D0_PORT(OUT) =  D0_PORT | (_dat->bit1 * B_D0_BIT); */
    _dat->bit1 ? B_D0_PORT(OUT) |= B_D0_BIT : (B_D0_PORT(OUT) &= ~B_D0_BIT);
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    bit_delay_w();
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    /* B_D0_PORT(OUT) =  D0_PORT | (_dat->bit0 * B_D0_BIT); */
    _dat->bit0 ? B_D0_PORT(OUT) |= B_D0_BIT : (B_D0_PORT(OUT) &= ~B_D0_BIT);
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    if (B_WORK_MODE == SPI_CPOL0_CPHA0) {
        bit_delay_w();
        B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    }
    local_irq_enable();
    B_D0_PORT(OUT) &= ~B_D0_BIT;
}
/* mode0: CPOL = 0, CPHA = 0, MSB first, clk_l, smp_edge_rise*/
/* mode3: CPOL = 1, CPHA = 1, MSB first, clk_h, smp_edge_rise */
AT_RAM
static u8 soft_spi_b_r_mode03(spi_soft_dev spi)
{
    u8 read_dat = 0;
    struct _dat_bit *_dat = (struct _dat_bit *)&read_dat;
    B_D1_PORT(DIR) |= B_D1_BIT;
    local_irq_disable();
    if (B_WORK_MODE == SPI_CPOL1_CPHA1) {
        B_CLK_PORT(OUT) &= ~B_CLK_BIT;
        bit_delay_r();
    }
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    /* read_dat |= (!!(B_D1_PORT(IN)& B_D1_BIT)*BIT(7)); */
    (B_D1_PORT(IN)& B_D1_BIT) ? _dat->bit7 = 1 : (_dat->bit7 = 0);
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    bit_delay_r();
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    /* read_dat |= (!!(B_D1_PORT(IN)& B_D1_BIT)*BIT(6)); */
    (B_D1_PORT(IN)& B_D1_BIT) ? _dat->bit6 = 1 : (_dat->bit6 = 0);
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    bit_delay_r();
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    /* read_dat |= (!!(B_D1_PORT(IN)& B_D1_BIT)*BIT(5)); */
    (B_D1_PORT(IN)& B_D1_BIT) ? _dat->bit5 = 1 : (_dat->bit5 = 0);
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    bit_delay_r();
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    /* read_dat |= (!!(B_D1_PORT(IN)& B_D1_BIT)*BIT(4)); */
    (B_D1_PORT(IN)& B_D1_BIT) ? _dat->bit4 = 1 : (_dat->bit4 = 0);
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    bit_delay_r();
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    /* read_dat |= (!!(B_D1_PORT(IN)& B_D1_BIT)*BIT(3)); */
    (B_D1_PORT(IN)& B_D1_BIT) ? _dat->bit3 = 1 : (_dat->bit3 = 0);
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    bit_delay_r();
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    /* read_dat |= (!!(B_D1_PORT(IN)& B_D1_BIT)*BIT(2)); */
    (B_D1_PORT(IN)& B_D1_BIT) ? _dat->bit2 = 1 : (_dat->bit2 = 0);
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    bit_delay_r();
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    /* read_dat |= (!!(B_D1_PORT(IN)& B_D1_BIT)*BIT(1)); */
    (B_D1_PORT(IN)& B_D1_BIT) ? _dat->bit1 = 1 : (_dat->bit1 = 0);
    B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    bit_delay_r();
    B_CLK_PORT(OUT) |=  B_CLK_BIT;
    /* read_dat |= (!!(B_D1_PORT(IN)& B_D1_BIT)*BIT(0)); */
    (B_D1_PORT(IN)& B_D1_BIT) ? _dat->bit0 = 1 : (_dat->bit0 = 0);
    if (B_WORK_MODE == SPI_CPOL0_CPHA0) {
        B_CLK_PORT(OUT) &= ~B_CLK_BIT;
    }
    local_irq_enable();
    return read_dat;
}
#endif

int soft_spi_send_byte(spi_soft_dev spi, u8 byte)
{
    u8 work_mode = 3;
    if (spi == 0) {
        work_mode = A_WORK_MODE;
#if(SOFT_SPI_B_GROUP_EN)
    } else {
        work_mode = B_WORK_MODE;
#endif
    }
    switch (work_mode) {
    case SPI_CPOL0_CPHA0:
    case SPI_CPOL1_CPHA1:
        if (spi == 0) {
            soft_spi_a_w_mode03(spi, byte);
#if(SOFT_SPI_B_GROUP_EN)
        } else {
            soft_spi_b_w_mode03(spi, byte);
#endif
        }
        break;
    case SPI_CPOL0_CPHA1:
    case SPI_CPOL1_CPHA0:
        if (spi == 0) {
            soft_spi_a_w_mode12(spi, byte);
#if(SOFT_SPI_B_GROUP_EN)
        } else {
            soft_spi_b_w_mode12(spi, byte);
#endif
        }
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

u8 soft_spi_recv_byte(spi_soft_dev spi, int *err)
{
    u8 rx_data = 0;
    u8 work_mode = 3;
    if (spi == 0) {
        work_mode = A_WORK_MODE;
#if(SOFT_SPI_B_GROUP_EN)
    } else {
        work_mode = B_WORK_MODE;
#endif
    }
    switch (work_mode) {
    case SPI_CPOL0_CPHA0:
    case SPI_CPOL1_CPHA1:
        if (spi == 0) {
            rx_data = soft_spi_a_r_mode03(spi);
#if(SOFT_SPI_B_GROUP_EN)
        } else {
            rx_data = soft_spi_b_r_mode03(spi);
#endif
        }
        break;
    case SPI_CPOL0_CPHA1:
    case SPI_CPOL1_CPHA0:
        if (spi == 0) {
            rx_data = soft_spi_a_r_mode12(spi);
#if(SOFT_SPI_B_GROUP_EN)
        } else {
            rx_data = soft_spi_b_r_mode12(spi);
#endif
        }
        break;
    default:
        err != NULL ? *err = -EINVAL : 0;
        return 0;
    }
    err != NULL ? *err = 0 : 0;
    return rx_data;
}

/*
 * @brief 18 SPI_MODE_BIDIR_1BIT
 * @parm spi  spi
 * @parm byte
 * @parm err  err0 < 0
 * @return
 */
u8 soft_spi_send_recv_byte(spi_soft_dev spi, u8 byte, int *err)
{
    u8 rx_data = 0;
    /* u8 work_mode; */
    /* if(spi==0){ */
    /* 	work_mode=A_WORK_MODE; */
    /* }else{ */
    /* 	work_mode=B_WORK_MODE; */
    /* } */
    /* switch (work_mode) { */
    /* case SPI_CPOL0_CPHA0: */
    /*     rx_data = soft_spi_rw_mode0(spi, byte); */
    /*     break; */
    /* case SPI_CPOL0_CPHA1: */
    /*     rx_data = soft_spi_rw_mode1(spi, byte); */
    /*     break; */
    /* case SPI_CPOL1_CPHA0: */
    /*     rx_data = soft_spi_rw_mode2(spi, byte); */
    /*     break; */
    /* case SPI_CPOL1_CPHA1: */
    /*     rx_data = soft_spi_rw_mode3(spi, byte); */
    /*     break; */
    /* default: */
    /*     err != NULL ? *err = -EINVAL : 0; */
    /*     return 0; */
    /* } */
    /* err != NULL ? *err = 0 : 0; */
    return rx_data;
}

/*
 * @brief spi dma
 * @parm spi  spi
 * @parm buf
 * @parm len
 * @return < 0
 */
int soft_spi_dma_recv(spi_soft_dev spi, void *buf, u32 len)
{
    u8 *buf_temp = (u8 *)buf;
    u8 work_mode = 3;
    if (spi == 0) {
        work_mode = A_WORK_MODE;
#if(SOFT_SPI_B_GROUP_EN)
    } else {
        work_mode = B_WORK_MODE;
#endif
    }
    /* ASSERT((u32)buf % 4 == 0, "spi dma addr need 4-aligned"); */
    switch (work_mode) {
    case SPI_CPOL0_CPHA0:
    case SPI_CPOL1_CPHA1:
        if (spi == 0) {
            while (len--) {
                *buf_temp = soft_spi_a_r_mode03(spi);
                buf_temp++;
            }
#if(SOFT_SPI_B_GROUP_EN)
        } else {
            while (len--) {
                *buf_temp = soft_spi_b_r_mode03(spi);
                buf_temp++;
            }
#endif
        }
        break;
    case SPI_CPOL0_CPHA1:
    case SPI_CPOL1_CPHA0:
        if (spi == 0) {
            while (len--) {
                *buf_temp = soft_spi_a_r_mode12(spi);
                buf_temp++;
            }
#if(SOFT_SPI_B_GROUP_EN)
        } else {
            while (len--) {
                *buf_temp = soft_spi_b_r_mode12(spi);
                buf_temp++;
            }
#endif
        }
        break;
    default:
        return -EFAULT;
    }
    return len;
}

int soft_spi_dma_send(spi_soft_dev spi, const void *buf, u32 len)
{
    u8 *buf_temp = (u8 *)buf;
    u8 work_mode = 3;
    if (spi == 0) {
        work_mode = A_WORK_MODE;
#if(SOFT_SPI_B_GROUP_EN)
    } else {
        work_mode = B_WORK_MODE;
#endif
    }
    /* ASSERT((u32)buf % 4 == 0, "spi dma addr need 4-aligned"); */
    switch (work_mode) {
    case SPI_CPOL0_CPHA0:
    case SPI_CPOL1_CPHA1:
        if (spi == 0) {
            while (len--) {
                soft_spi_a_w_mode03(spi, *buf_temp);
                buf_temp++;
            }
#if(SOFT_SPI_B_GROUP_EN)
        } else {
            while (len--) {
                soft_spi_b_w_mode03(spi, *buf_temp);
                buf_temp++;
            }
#endif
        }
        break;
    case SPI_CPOL0_CPHA1:
    case SPI_CPOL1_CPHA0:
        if (spi == 0) {
            while (len--) {
                soft_spi_a_w_mode12(spi, *buf_temp);
                buf_temp++;
            }
#if(SOFT_SPI_B_GROUP_EN)
        } else {
            while (len--) {
                soft_spi_b_w_mode12(spi, *buf_temp);
                buf_temp++;
            }
#endif
        }
        break;
    default:
        return -EFAULT;
    }
    return len;
}


/********************************soft spi test*****************************/
#if 0
void soft_spi_init_test(spi_soft_dev spi)
{
    local_irq_disable();
    soft_spi_open(spi);
    JL_PORTA->OUT |= BIT(5);//cs
    JL_PORTA->DIR &= ~BIT(5);
    local_irq_enable();
}
void soft_spi_buf_write_test(spi_soft_dev spi)
{
    u32 len = 100;
    u8 buf_temp_w[len];
    local_irq_disable();
    JL_PORTA->OUT &= ~BIT(5);
    for (u32 i = len; i > 0; i--) {
        if (i > 50) {
            buf_temp_w[i - 1] = 0x55;
        } else {
            buf_temp_w[i - 1] = 0xaa;
        }
    }
    soft_spi_dma_send(spi, buf_temp_w, len);
    JL_PORTA->OUT |= BIT(5);
    local_irq_enable();
    log_info("soft spi buf write");
}
void soft_spi_byte_write_test(spi_soft_dev spi)
{
    u8 len = 200;
    u8 byte = 0x55;
    local_irq_disable();
    JL_PORTA->OUT &= ~BIT(5);
    while (len--) {
        soft_spi_send_byte(spi, byte);
    }
    JL_PORTA->OUT |= BIT(5);
    local_irq_enable();
    log_info("soft spi byte write");
}
void soft_spi__test()
{
    soft_spi_init_test(1);
    soft_spi_byte_write_test(1);
    soft_spi_buf_write_test(1);
}

#endif

