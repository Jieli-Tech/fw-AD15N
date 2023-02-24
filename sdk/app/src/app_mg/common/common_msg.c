#include "msg.h"
#include "common.h"
#include "string.h"
#include "circular_buf.h"
#include "uart.h"
#include <stdarg.h>
#include "config.h"
#include "device.h"
#include "app_power_mg.h"
#include "vm.h"
#include "dac_api.h"
#include "app_mg/app_mg.h"
#include "audio.h"
#include "sdmmc/sd_host_api.h"
#include "app_dev_mg.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

void music_vol_update(void)
{
    u8 vol = 0;
    vm_read(VM_INDEX_VOL, &vol, sizeof(vol));
    if (vol != dac_vol('r', 0)) {
        vol = dac_vol('r', 0);
        vm_write(VM_INDEX_VOL, &vol, sizeof(vol));
        //log_info("vol update to vm \n");
    }
}
int common_msg_deal(int *msg)
{
    switch (*msg) {
    case MSG_SDMMCA_IN:
#if TFG_DEV_UPGRADE_SUPPORT
#ifdef CPU_SH54
        device_upgrate_try();
#endif
#endif
        break;
    case MSG_SDMMCA_OUT:
        break;

    case MSG_500MS:
        wdt_clear();
        music_vol_update();
        app_power_scan();
        audio_lookup();
        break;
    case MSG_POWER_OFF:
        app_switch(APP_POWEROFF, (void *)MSG_POWER_OFF);
        break;
    case MSG_LOW_POWER:
        app_switch(APP_POWEROFF, (void *)MSG_LOW_POWER);
        break;
    case MSG_ENTER_IDLE:
        app_switch(APP_POWEROFF, (void *)MSG_ENTER_IDLE);
        break;

    case MSG_VOL_UP:
        dac_vol('+', 255);
        log_info("vol:%d \n", dac_vol('r', 0));
        break;
    case MSG_VOL_DOWN:
        dac_vol('-', 255);
        log_info("vol:%d \n", dac_vol('r', 0));
        break;

    default:
        break;
    }

    return -1;
}

