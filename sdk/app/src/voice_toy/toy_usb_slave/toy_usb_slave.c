#include "toy_usb_slave.h"
#include "common.h"
#include "msg.h"
#if HAS_USB_EN
#include "usb/usb_config.h"
#include "usb/device/hid.h"
#include "usb/device/msd.h"
#include "usb/usr/usb_audio_interface.h"
#endif
#include "clock.h"
#include "dac_api.h"
#include "app_config.h"
#include "decoder_api.h"
#include "toy_main.h"
#include "bsp_loop.h"
#if TCFG_PC_ENABLE

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[pc]"
#include "log.h"

#if (USB_DEVICE_CLASS_CONFIG & HID_CLASS)
static void hid_key_api(u32 key, u8 flag)
{
    usb_hid_control(key);
    if (flag) {
        usb_hid_control(0);
    }
}
#endif

extern void USB_MassStorage(const struct usb_device_t *usb_device);
void toy_usb_slave_app(void)
{
    u32 dac_sr = dac_sr_read();
    u8 temp_vol = dac_vol('r', 0);
    key_table_sel(usb_slave_key_msg_filter);

    /* decoder_init(); */

    usb_sof_trim(NULL, NULL);
    usb_device_mode(0, 0);
#if TCFG_USB_EXFLASH_UDISK_ENABLE
    void *device = dev_open("ext_flsh", 0);
    if (device != NULL) {
        dev_ioctl(device, IOCTL_SET_READ_USE_CACHE, 1);
        dev_ioctl(device, IOCTL_SET_CACHE_SYNC_ISR_EN, 0);
    } else {
        log_info("dev open err\n");
    }
#endif
    usb_start();

    int msg[2];
    u32 err;
    while (1) {
#if (USB_DEVICE_CLASS_CONFIG & MASSSTORAGE_CLASS)
        USB_MassStorage(NULL);
#endif
        err = get_msg(2, &msg[0]);
        bsp_loop();
        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
            log_info("get msg err 0x%x\n", err);
        }

        switch (msg[0]) {
#if (USB_DEVICE_CLASS_CONFIG & MASSSTORAGE_CLASS)
        case MSG_SDMMCA_IN:
            log_info("MSG_SDMMCA_IN");
            break;
        case MSG_SDMMCA_OUT:
            log_info("MSG_SDMMCA_OUT");
            break;
#endif

            /* #if TCFG_CFG_TOOL_ENABLE */
#if (USB_DEVICE_CLASS_CONFIG & HID_CLASS)
        case MSG_PREV_FILE:
            log_info("PR\n");
            hid_key_api(USB_AUDIO_PREFILE, 1);
            break;
        case MSG_NEXT_FILE:
            log_info("NE\n");
            hid_key_api(USB_AUDIO_NEXTFILE, 1);
            break;
        case MSG_PP:
            log_info("PP\n");
            hid_key_api(USB_AUDIO_PP, 1);
            break;
        case MSG_VOL_DOWN:
            log_info("DO\n");
            hid_key_api(USB_AUDIO_VOLDOWN, 1);
            break;
        case MSG_VOL_UP:
            log_info("UP\n");
            hid_key_api(USB_AUDIO_VOLUP, 1);
            break;
#else
        case MSG_VOL_DOWN:
        case MSG_VOL_UP:
            break;
#endif
        case MSG_PC_OUT:
            log_info("MSG_PC_OUT\n");
            usb_stop();
            work_mode++;
        case MSG_CHANGE_WORK_MODE:
            usb_pause();
            goto __out_t_usb_slave;
        case MSG_500MS:
        default:
            common_msg_deal(&msg[0]);
            break;
        }
    }
__out_t_usb_slave:
#if TCFG_USB_EXFLASH_UDISK_ENABLE
    if (device != NULL) {
        dev_close(device);
    }
#endif
    dac_vol(0, temp_vol);
    dac_sr_api(dac_sr);
    key_table_sel(NULL);
}
#endif


