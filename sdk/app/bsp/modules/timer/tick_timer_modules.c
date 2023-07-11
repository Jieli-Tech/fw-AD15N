#include "app_config.h"
#include "tick_timer_driver.h"
/* #include "clock.h" */
/* #include "asm/power_interface.h" */
#include "msg.h"
/* #include "bsp_loop.h" */
/* #include "app_power_mg.h" */
#include "vm_api.h"
#include "dac_api.h"
/* #include "adc_drv.h" */
#include "key.h"
#if KEY_IR_EN
#include "irflt.h"
#endif
#if defined(TFG_SD_EN) && (TFG_SD_EN)
#include "sdmmc/sd_host_api.h"
#endif

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

void modules_tick_timer(u32 cnt)
{
    if (0 == (cnt % 5)) {
#if KEY_IR_EN
        ir_timeout();
#endif
        key_scan();
    }

    if (0 == (cnt % 100)) { //200ms
#if defined(TFG_SD_EN) && (TFG_SD_EN)
        sd0_dev_detect(NULL);
#endif
    }

    if (0 == (cnt % 125)) { //250ms
#if TCFG_PC_ENABLE || TCFG_UDISK_ENABLE
        void usb_hotplug_detect(void *argv);
        usb_hotplug_detect(NULL);
#endif
    }

}

