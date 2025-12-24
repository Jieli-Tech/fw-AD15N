#include "asm/power_interface.h"
#include "wdt.h"
#include "app_config.h"
#include "gpio.h"
#include "audio/audio.h"
#include "vm_api.h"

#define LOG_TAG_CONST       PMU
#define LOG_TAG             "[PMU]"
#include "log.h"

void sys_softoff()
{
    lowpower_init();

    /* 睡眠前vm预擦除 */
    vm_pre_erase();
    dac_power_off();//软关机前需要关闭audio
    power_set_soft_poweroff();
}

extern volatile u8 sys_low_power_request;
extern volatile u32 lowpower_usec;
void sys_power_down(u32 usec)
{
    lowpower_init();

    /* 关机前vm预擦除 */
    vm_pre_erase();
    if (sys_low_power_request ==  0) {
        lowpower_usec = usec;
        low_power_sys_request(NULL);
    }

    lowpower_uninit();
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
