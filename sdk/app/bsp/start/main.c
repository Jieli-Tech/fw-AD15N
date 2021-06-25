/*********************************************************************************************
    *   Filename        : main.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  :

    *   Copyright:(c)JIELI  2011-2017  @ , All Rights Reserved.
*********************************************************************************************/
#include "config.h"
#include "common.h"
#include "maskrom.h"
#include "asm/power/p33.h"
#include "app_config.h"
#include "init.h"
#include "vm.h"
#include "asm/power_interface.h"
#include "power_api.h"

#define LOG_TAG_CONST       MAIN
#define LOG_TAG             "[main]"
#include "debug.h"

__attribute__((weak))
u32 get_reset_source_value(void)
{
    return 0;
}
void reset_info_dump(void)
{
    u32 reset_value = get_reset_source_value();
    u8 JL_RST_SRC = (u8)reset_value;
    u8 _P3_RST_SRC = (reset_value >> 8) & 0xff;

    u8 sfr = JL_RST_SRC;
    /*P33复位，造成主系统复位*/
    if (sfr & BIT(0)) {
        log_info("P33 RESET");
    }
    /*电源上电复位*/
    if (sfr & BIT(1)) {
        log_info("DVDD POR");
    }
    /*软件复位, JL_CLOCK->POWER_CON: BIT(4)*/
    if (sfr & BIT(2)) {
        log_info("SOFTR");
    }
    /*soft_off复位*/
    if (sfr & BIT(3)) {
        log_info("PLPM");
    }

    if (reset_value & BIT(16)) {
        log_info("power return");
        return;
    }

    sfr =  _P3_RST_SRC;
    /*VDDIO上电复位*/
    if (sfr & BIT(0)) {
        log_info("VDDIO POR");
    }
    /*VDDIO低电复位*/
    if (sfr & BIT(1)) {
        log_info("VDDIO LVD");
    }
    /*看门狗复位*/
    if (sfr & BIT(2)) {
        log_info("WDT");
    }
    /*短按复位: 按键按了vcom引脚, P3_ANA_CON2: BIT(3)*/
    if (sfr & BIT(3)) {
        log_info("VCM");
    }
    /*短按复位: 按键按了pa7, P3_PR_WWR: BIT(3), mclr引脚需要开dieh*/
    if (sfr & BIT(4)) {
        log_info("MCLR");
    }
    /*长按复位*/
    if (sfr & BIT(5)) {
        log_info("PINR");
    }
    /*主系统传到P33的复位，默认是屏蔽的，P3_IVS: BIT(5)*/
    if (sfr & BIT(6)) {
        log_info("SYS RESET");
    }
    /*软件复位, P3_PR_PWR: BIT(4)*/
    if (sfr & BIT(7)) {
        log_info("SOFT RESET");
    }
}

extern void app(void);
int c_main(int cfg_addr)
{

#if 1
    ///暂时调低p33跑的波特率
    JL_P33->SPI_CON |= (BIT(2) | BIT(3));
#endif

    /* spi_cache_way_switch(1); */
    /* request_irq(1, 7, exception_irq_handler, 0); */
    mask_init(exception_analyze, putchar);
    all_init_isr();

    reset_source_dump();
    power_reset_source_dump();

    //soft_off会Lat io, 唤醒之后电源初始化才会释放io，所以在电源初始化之后才能翻io/打印(注: soft_off不需要查复位源)
    sys_power_init();

    debug_init(1000000);
    log_info("--------- sh5x-apps -------------\n");
    reset_info_dump();

    p33_tx_1byte(P3_PINR_CON, 0);

    //--- OSC CL  12M
    SFR(JL_CLK->CON0, 19, 2, 3);

    system_init();
    app();
    while (1) {
        wdt_clear();
    }
}


int __attribute__((weak)) eSystemConfirmStopStatus(void)
{
    /* 系统进入在未来时间里，无任务超时唤醒，可根据用户选择系统停止，或者系统定时唤醒(100ms)，或自己指定唤醒时间 */
    //1:Endless Sleep
    //0:100 ms wakeup
    //other: x ms wakeup
    if (0) {
        return 1;
    } else {
        return 0;
    }
}

