/*********************************************************************************************
    *   Filename        : port_init.c

    *   Description     : 对上电和低功耗所有GPIO的处理

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
#include "usb/otg.h"
#include "usb/host/usb_host.h"

void port_init(void)
{
    u32 sdtap_ch = -1;


    u16 porta_die = 0;
    u16 porta_dieh = 0;
    u16 portb_die = 0;
    u16 portb_dieh = 0;
    u16 portd_die = 0;
    u16 portd_dieh = 0;
    u8 usb_dp_die = 0;
    u8 usb_dp_dieh = 0;
    u8 usb_dm_die = 0;
    u8 usb_dm_dieh = 0;

    //uart_port
#if UART_TX_OUTPUT_CH_EN
    if (UART_OUTPUT_CH_PORT < GPIOB) {
        porta_die |= BIT(UART_OUTPUT_CH_PORT % IO_GROUP_NUM);
    } else if (UART_OUTPUT_CH_PORT >= GPIOB && UART_OUTPUT_CH_PORT < GPIOD) {
        portb_die |= BIT(UART_OUTPUT_CH_PORT % IO_GROUP_NUM);
    } else if (UART_OUTPUT_CH_PORT >= GPIOD && UART_OUTPUT_CH_PORT < IO_MAX_NUM) {
        portd_die |= BIT(UART_OUTPUT_CH_PORT % IO_GROUP_NUM);
    } else if (UART_OUTPUT_CH_PORT == IO_PORT_DP) {
        usb_dp_die = 1;
    } else if (UART_OUTPUT_CH_PORT == IO_PORT_DM) {
        usb_dm_die = 1;
    }
#endif

    //lvd(数字)、mclr需要使能dieh，模拟lvd不需要
    portb_dieh |= BIT(1);

    //flash port
    portd_die |= BIT(1) | BIT(3);

    JL_PORTA->DIE = porta_die;
    JL_PORTA->DIEH = porta_dieh;

    JL_PORTB->DIE = portb_die;
    JL_PORTB->DIEH = portb_dieh;

    //For Flash Boot data port
    JL_PORTD->DIE = portd_die;
    JL_PORTD->DIEH = portd_dieh;

    //USB
    JL_USB->CON0 = 0;

    usb_iomode(1);

    gpio_set_die(IO_PORT_DP, usb_dp_die);
    gpio_set_dieh(IO_PORT_DP, usb_dp_dieh);
    gpio_set_die(IO_PORT_DM, usb_dm_die);
    gpio_set_dieh(IO_PORT_DM, usb_dm_dieh);
}

void mask_io_cfg()
{
    struct boot_soft_flag_t boot_soft_flag = {0};

    boot_soft_flag.flag0.boot_ctrl.wdt_dis = 0;
    boot_soft_flag.flag0.boot_ctrl.lvd_en = GET_P33_VLVD_EN();
    boot_soft_flag.flag0.boot_ctrl.usbdp = SOFTFLAG_HIGH_RESISTANCE;
    boot_soft_flag.flag0.boot_ctrl.usbdm = SOFTFLAG_HIGH_RESISTANCE;

    mask_softflag_config(&boot_soft_flag);
}

extern u32 spi_get_port(void);
extern const struct low_power_param power_param;
void close_gpio(u8 soft_off)
{
    u32 porta_value = 0xffff & ~(BIT(0));
    u32 portb_value = 0xffff;
    u32 portd_value = 0x1f;

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

#if TCFG_UDISK_ENABLE
    if (soft_off == 0 && usb_otg_online(0) == HOST_MODE) {
        usb_host_suspend(0);
    } else {
        usb_iomode(1);

        gpio_set_pull_up(IO_PORT_DP, 0);
        gpio_set_pull_down(IO_PORT_DP, 0);
        gpio_set_direction(IO_PORT_DP, 1);
        gpio_set_die(IO_PORT_DP, 0);
        gpio_set_dieh(IO_PORT_DP, 0);

        gpio_set_pull_up(IO_PORT_DM, 0);
        gpio_set_pull_down(IO_PORT_DM, 0);
        gpio_set_direction(IO_PORT_DM, 1);
        gpio_set_die(IO_PORT_DM, 0);
        gpio_set_dieh(IO_PORT_DM, 0);
    }
#else
    usb_iomode(1);

    gpio_set_pull_up(IO_PORT_DP, 0);
    gpio_set_pull_down(IO_PORT_DP, 0);
    gpio_set_direction(IO_PORT_DP, 1);
    gpio_set_die(IO_PORT_DP, 0);
    gpio_set_dieh(IO_PORT_DP, 0);

    gpio_set_pull_up(IO_PORT_DM, 0);
    gpio_set_pull_down(IO_PORT_DM, 0);
    gpio_set_direction(IO_PORT_DM, 1);
    gpio_set_die(IO_PORT_DM, 0);
    gpio_set_dieh(IO_PORT_DM, 0);
#endif
}

