/*********************************************************************************************
    *   Filename        : port_init.c

    *   Description     : 对上电和低功耗所有GPIO的处理，低功耗flash io由库进行处理。

    *   Author          : Sunlicheng

    *   Email           : Sunlicheng@zh-jieli.com

    *   Last modifiled  : 2021-07-13 11:50

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/
#include "asm/power_interface.h"
#include "asm/power/p33.h"
#include <stdlib.h>
#include "app_config.h"
#include "typedef.h"
#include "gpio.h"

void port_hd_init(void)
{
    u16 porta_hd0 = 0xffff;
    u16 porta_hd1 = 0;
    JL_PORTA->HD0 = porta_hd0;
    JL_PORTA->HD1 = porta_hd1;

    u16 portb_hd0 = 0xffff;
    u16 portb_hd1 = 0;
    JL_PORTB->HD0 = portb_hd0;
    JL_PORTB->HD1 = portb_hd1;

    u16 portd_hd0 = 0xffff;
    u16 portd_hd1 = 0x10;
    JL_PORTD->HD0 = portd_hd0;
    JL_PORTD->HD1 = portd_hd1;
}

void port_init(void)
{
    u32 sdtap_ch = -1;

    u16 porta_die = 0;
    u16 porta_dieh = 0;
    u16 portb_die = 0;
    u16 portb_dieh = 0;
    u16 portd_die = 0;
    u16 portd_dieh = 0;

    //uart_port
#if UART_TX_OUTPUT_CH_EN
    if (UART_OUTPUT_CH_PORT >= GPIOA && UART_OUTPUT_CH_PORT < GPIOB) {
        porta_die |= BIT(UART_OUTPUT_CH_PORT % IO_GROUP_NUM);
    } else if (UART_OUTPUT_CH_PORT >= GPIOB && UART_OUTPUT_CH_PORT < GPIOD) {
        portb_die |= BIT(UART_OUTPUT_CH_PORT % IO_GROUP_NUM);
    } else if (UART_OUTPUT_CH_PORT >= GPIOD && UART_OUTPUT_CH_PORT < IO_MAX_NUM) {
        portd_die |= BIT(UART_OUTPUT_CH_PORT % IO_GROUP_NUM);
    }
#endif

    //lvd(数字)、mclr(pb10)需要使能dieh，模拟lvd不需要
    portb_dieh |= BIT(10);

    //flash port
    portd_die |= BIT(1) | BIT(3);

    JL_PORTA->DIE = porta_die;
    JL_PORTA->DIEH = porta_dieh;

    JL_PORTB->DIE = portb_die;
    JL_PORTB->DIEH = portb_dieh;

    //For Flash Boot data port
    JL_PORTD->DIE = portd_die;
    JL_PORTD->DIEH = portd_dieh;

    port_hd_init();
}

void mask_io_cfg()
{
    struct boot_soft_flag_t boot_soft_flag = {0};

    boot_soft_flag.flag0.boot_ctrl.wdt_dis = 0;
    boot_soft_flag.flag0.boot_ctrl.lvd_en = GET_P33_VLVD_EN();
    /* boot_soft_flag.flag0.boot_ctrl.fast_boot = 1; //关闭软关机唤醒串口升级，优化启动时间 */

    mask_softflag_config(&boot_soft_flag);
}

extern u32 spi_get_port(void);
extern const struct low_power_param power_param;
void close_gpio(u8 soft_off)
{
    u32 porta_value = 0xffff;
    u32 portb_value = 0xffff & ~(BIT(10));
    u32 portd_value = 0x1f;

    if (!(POWER_WAKEUP_IO & 0x10)) {
        porta_value &= ~BIT(POWER_WAKEUP_IO);
    } else {
        portb_value &= ~BIT(POWER_WAKEUP_IO & (~0xfff0));
    }

    if (soft_off) {
        mask_io_cfg();
    }

    if (spi_get_port() == 0) {
        portd_value &= ~0x1f;
        if (power_param.flash_pg) {
            portd_value |= BIT(4);
        }
    } else {

    }

    JL_PORTA->DIR |= porta_value;
    JL_PORTA->PU &= ~(porta_value);
    JL_PORTA->PD &= ~(porta_value);
    JL_PORTA->DIE &= ~(porta_value);
    JL_PORTA->DIEH &= ~(porta_value);

    //PB1:长按复位
    JL_PORTB->DIR |= portb_value;
    JL_PORTB->PU &= ~(portb_value);
    JL_PORTB->PD &= ~(portb_value);
    JL_PORTB->DIE &= ~(portb_value);
    JL_PORTB->DIEH &= ~(portb_value);

    JL_PORTD->DIR |= portd_value;
    JL_PORTD->PU &= ~(portd_value);
    JL_PORTD->PD &= ~(portd_value);
    JL_PORTD->DIE &= ~(portd_value);
    JL_PORTD->DIEH &= ~(portd_value);
}
