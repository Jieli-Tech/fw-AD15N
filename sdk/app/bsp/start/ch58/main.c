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
#include "clock.h"
#include "asm/power_interface.h"
#include "asm/power/power_app.h"
#include "key_drv_ad.h"
#include "adc_drv.h"
/* #include "asm/power_interface.h" */
/* #include "vm_api.h" */
/* #include "power_api.h" */

#define LOG_TAG_CONST       MAIN
#define LOG_TAG             "[main]"
#include "log.h"

extern void emu_init(void);
extern void exception_analyze(unsigned int *sp);
extern void app(void);

void maskrom_init(void)
{
    struct maskrom_argv argv;
    memset((void *)&argv, 0, sizeof(struct maskrom_argv));
    argv.pchar = (void *)putchar;
    argv.exp_hook = exception_analyze;
#ifndef CONFIG_PRINT_FLOAT_ENABLE
    argv.flt = NULL;
#else
    argv.flt = flt;
#endif
    argv.local_irq_enable = local_irq_enable;
    argv.local_irq_disable = local_irq_disable;
    mask_init(&argv);
}

int c_main(int cfg_addr)
{
    all_init_isr();
    register_handle_printf_putchar(putchar);
    maskrom_init();

    mclr_reset_sw(0);
    port_init();
    log_init(1000000);

    clk_voltage_init(CLOCK_MODE_ADAPTIVE, DVDD_VOL_SEL_123V);
    clk_early_init(CLOCK_PLL_REF_LRC200K, 200000, 480 * 1000000);
    clk_set("sys", 80 * 1000000);
    /* clock_dump(); */
    /* clk_out(IO_PORTA_05, HSB_CLK_OUT); */

    log_info("-------ch58-apps--------");

    wdt_init(WDT_8S);

    emu_init();

    power_early_init();
    sys_power_init();

    system_init();

    volatage_trim_init();

    app();

    while (1) {
        wdt_clear();
    }
    return 0;
}




void cpu_assert_debug(void)
{
    while (1) {
        asm("nop");
    }
}

