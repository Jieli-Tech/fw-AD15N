#include "msg.h"
#include "config.h"
#include "key.h"
#include "common.h"
#include "string.h"
#include "circular_buf.h"
#include "vm_api.h"
#include "dac_api.h"
#include "toy_main.h"
#include "app_power_mg.h"
#include "adc_drv.h"
#include "audio.h"
#include "device_mge.h"
#if HAS_USB_EN
#include "usb/host/usb_host.h"
#include "usb/device/usb_stack.h"
#endif

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[comm_msg]"
#include "log.h"

void music_vol_update(void)
{
    u8 vol = 0;
    vm_read(VM_INDEX_VOL, &vol, sizeof(vol));
    if (vol != dac_vol('r', 0)) {
        vol = dac_vol('r', 0);
        vm_write(VM_INDEX_VOL, &vol, sizeof(vol));
        /* log_info("vol update to vm : %d\n", vol); */
    }
}

int common_msg_deal(int *msg)
{
    switch (*msg) {
    case MSG_500MS:
        wdt_clear();
        music_vol_update();
        adc_sample_vbg();
        app_power_scan();
        audio_lookup();
        break;
    case MSG_VOL_UP:
        dac_vol('+', 255);
        log_info("vol:%d \n", dac_vol('r', 0));
        break;
    case MSG_VOL_DOWN:
        dac_vol('-', 255);
        log_info("vol:%d \n", dac_vol('r', 0));
        break;
    case MSG_NEXT_WORKMODE:
        log_info("MSG_NEXT_WORKMODE\n");
        app_next_mode();
        post_msg(1, MSG_CHANGE_WORK_MODE);
        break;
    case MSG_LOW_POWER:
    case MSG_POWER_OFF:
        log_info("MSG_POWER_OFF\n");
        work_mode = TOY_SOFTOFF;
        post_msg(1, MSG_CHANGE_WORK_MODE);
        break;

        /* 设备升级 */
#if defined(TCFG_UDISK_ENABLE) && (TCFG_UDISK_ENABLE)
    case MSG_USB_DISK_IN:
        log_info("udisk in\n");
        usb_host_mount(0, 3, 20, 200);
#endif
#if defined(TFG_SD_EN) && (TFG_SD_EN)
    case MSG_SDMMCA_IN:
#endif
#if defined(TFG_DEV_UPGRADE_SUPPORT) && (1 == TFG_DEV_UPGRADE_SUPPORT)
        u8 dev_idx = *msg - MSG_USB_DISK_IN;
        device_update(dev_idx);
#endif
        break;

#if defined(TCFG_UDISK_ENABLE) && (TCFG_UDISK_ENABLE)
    case MSG_USB_DISK_OUT:
        log_info("udisk out\n");
        usb_host_unmount(0);
        break;
#endif
#if defined(TFG_SD_EN) && (TFG_SD_EN)
    case MSG_SDMMCA_OUT:
        break;
#endif
#if TCFG_PC_ENABLE
    case MSG_USB_PC_IN:
    case MSG_PC_IN:
        log_info("pc in\n");
        work_mode = TOY_USB_SLAVE;
        post_msg(1, MSG_CHANGE_WORK_MODE);
        break;
    case MSG_USB_PC_OUT:
    case MSG_PC_OUT:
        log_info("pc out\n");
        usb_stop();
        break;
#endif

#if KEY_IR_EN
    case MSG_0:
    case MSG_1:
    case MSG_2:
    case MSG_3:
    case MSG_4:
    case MSG_5:
    case MSG_6:
    case MSG_7:
    case MSG_8:
    case MSG_9:
        if (!Sys_IRInput) {
            break;
        }
        if (Input_Number > 999) {
            Input_Number = 0;
        }
        Input_Number = Input_Number * 10 + *msg;
        break;
#endif


    default:
        break;
    }

    return -1;
}
