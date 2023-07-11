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
#include "app_config.h"
#include "init.h"
#include "asm/power_interface.h"
#include "power_api.h"

#define LOG_TAG_CONST       MAIN
#define LOG_TAG             "[main]"
#include "log.h"

extern void port_hd_init(void);
extern void app(void);
__attribute__((noreturn))
int c_main(int cfg_addr)
{
    port_hd_init();
    log_init(1000000);

    struct maskrom_argv argv = {0};
    argv.exp_hook = exception_analyze;
    argv.enter_critical_hook = enter_critical_hook;
    argv.exit_critical_hook = exit_critical_hook;
    register_handle_printf_putchar(putchar);
    mask_init(&argv);

    log_info("---------sh57 apps------------- \n");

    u8 reg = p33_rx_1byte(P3_PR_PWR);
    reg &= ~BIT(3);
    p33_tx_1byte(P3_PR_PWR, reg);   //PB2关闭MCLR复位

    emu_init();

    wdt_init(WDT_8S);

    //记录及打印复位源
    reset_source_dump();

    sys_power_init();

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

