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
#include "wdt.h"

/* #include "vm_api.h" */
/* #include "power_api.h" */

#define LOG_TAG_CONST       MAIN
#define LOG_TAG             "[main]"
#include "log.h"

extern void app(void);
extern void emu_init(void);
extern void exception_analyze(unsigned int *sp);
extern void port_hd_init(void);
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

    log_info("---------sh57-apps------------- \n");

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


