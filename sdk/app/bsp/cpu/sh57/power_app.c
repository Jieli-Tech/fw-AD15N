#include "asm/power_interface.h"
#include "wdt.h"
#include "app_config.h"
#include "gpio.h"
#include "audio/audio.h"
#include "vm_api.h"

#define LOG_TAG_CONST       PMU
#define LOG_TAG             "[PMU]"
#include "log.h"

//-------------------------------------------------------------------
/*调试pdown进不去的场景，影响低功耗流程
 * 打印蓝牙和系统分别可进入低功耗的时间(msec)
 * 打印当前哪些模块处于busy,用于蓝牙已经进入sniff但系统无法进入低功耗的情况，如果usr_timer处于busy则会打印对应的func地址
 */
const char debug_is_idle = 0;

//-------------------------------------------------------------------
/* 调试快速起振信息，不影响低功耗流程
 */
//const bool pdebug_xosc_resume = 0;

//-------------------------------------------------------------------
/* 调试低功耗流程
 */
//出pdown打印信息，不影响低功耗流程
const bool pdebug_pdown_info = 0;

//使能串口调试低功耗，在pdown、soff模式保持串口，配合打开log_debug\pdebug_uart_pdown
const u32 pdebug_uart_lowpower = 0;
const u32 pdebug_uart_port = IO_PORTA_04;

//使能串口putbyte调试pdown流程(不影响pdown流程)
const bool pdebug_putbyte_pdown = 0;

void sys_softoff()
{
    /* 睡眠前vm预擦除 */
    vm_pre_erase();
    dac_power_off();//软关机前需要关闭audio
    power_set_soft_poweroff();
}

extern volatile u8 sys_low_power_request;
extern volatile u32 lowpower_usec;
void sys_power_down(u32 usec)
{
    /* 关机前vm预擦除 */
    vm_pre_erase();
    if (sys_low_power_request ==  0) {
        lowpower_usec = usec;
        low_power_sys_request(NULL);
    }
}

AT_VOLATILE_RAM_CODE_POWER
void __lvd_irq_handler(void)
{
    p33_fast_access(P3_VLVD_CON, BIT(6), 1);
    putchar('$');
    sys_softoff();
}

static enum LOW_POWER_LEVEL power_app_level(void)
{
    return LOW_POWER_MODE_SLEEP;
}

static u8 power_app_idle(void)
{
    return 1;
}

REGISTER_LP_TARGET(power_app_lp_target) = {
    .name = "power_app",
    .level = power_app_level,
    .is_idle = power_app_idle,
};
