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
#include "asm/power_interface.h"
#include "power_api.h"

#define LOG_TAG_CONST       MAIN
#define LOG_TAG             "[main]"
#include "log.h"

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

    log_init(1000000);

    log_info("--------- sh5x-apps -------------\n");

    reset_source_dump();
    power_reset_source_dump();

    //注:soft_off会Latch io, 唤醒之后电源初始化才会释放io，所以在电源初始化之后才能翻io/打印
    sys_power_init();

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

