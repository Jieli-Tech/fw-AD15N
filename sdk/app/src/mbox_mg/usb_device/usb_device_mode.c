#include "typedef.h"
#include "usb_device_mode.h"
#include "msg.h"
#include "common/hot_msg.h"
#include "key.h"
#include "mbox_main.h"
#include "common/mbox_common.h"
#include "dev_mg/device.h"
#include "dac_api.h"
#include "decoder_api.h"
#include "clock.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"
#include "app_config.h"

#ifdef USB_DEVICE_EN
static u16 usb_msg_filter(u8 key_status, u8 key_num, u8 key_type)
{
    u16 msg = NO_MSG;
    switch (key_type) {
#if KEY_IO_EN
    case KEY_TYPE_IO:
        msg = iokey_msg_mbox_usb_table[key_status][key_num];
        break;
#endif
#if KEY_AD_EN
    case KEY_TYPE_AD:
        msg = adkey_msg_mbox_usb_table[key_status][key_num];
        break;
#endif

#if KEY_IR_EN
    case KEY_TYPE_IR:
        msg = irff00_msg_usb_table[key_status][key_num];
        break;
#endif

    default:
        break;
    }

    return msg;
}

void USB_MassStorage(const struct usb_device_t *usb_device);
void USB_devie_mode_loop(void)
{
    u16 msg;
    log_info("USB_devie_mode_loop \n");
    SET_UI_MAIN(MENU_PC_MAIN);
    UI_menu(MENU_PC_MAIN);

    while (1) {
        msg = app_get_msg();
        bsp_loop();

        USB_MassStorage(NULL);
        switch (msg) {
        case MSG_CHANGE_WORK_MODE:
            log_info(" MSG_CHANGE_WORK_MODE\n");
            return;
        case MSG_PC_OUT:
            log_info(" pc out");
            LED_FADE_ON();
            work_mode = MUSIC_MODE;
            usb_stop();
            return;

#if (USB_DEVICE_CLASS_CONFIG&HID_CLASS)
        case MSG_MUSIC_PREV_FILE:
            log_info("MSG_MUSIC_PREV_FILE\n");
            usb_hid_control(USB_AUDIO_PREFILE); //按下
            usb_hid_control(0); //抬起
            break;
        case MSG_MUSIC_NEXT_FILE:
            log_info("MSG_MUSIC_NEXT_FILE\n");
            usb_hid_control(USB_AUDIO_NEXTFILE); //按下
            usb_hid_control(0); //抬起
            break;
        case MSG_MUSIC_PP:
            log_info("MSG_MUSIC_PP_FILE\n");
            usb_hid_control(USB_AUDIO_PP); //按下
            usb_hid_control(0); //抬起
            break;
        case MSG_VOL_DOWN:
            log_info("MSG_VOL_DOWN\n");
            usb_hid_control(USB_AUDIO_VOLDOWN);//按下
            usb_hid_control(0); //抬起
            break;
        case MSG_VOL_UP:
            log_info("MSG_VOL_UP\n");
            usb_hid_control(USB_AUDIO_VOLUP); //按下
            usb_hid_control(0); //抬起
            break;
        case MSG_MUTE:
            log_info("MSG_MUTE\n");
            usb_hid_control(USB_AUDIO_MUTE); //按下
            usb_hid_control(0);//抬起
            break;
#else
        case MSG_VOL_DOWN:
            break;
        case MSG_VOL_UP:
            break;
        case MSG_MUTE:
            break;
#endif

        case MSG_500MS:
            LED_FADE_OFF();
            UI_menu(MENU_MAIN);
            UI_menu(MENU_HALF_SEC_REFRESH);

        default:
            ap_handle_hotkey(msg);
            break;
        }
    }

}

void USB_device_mode(void)
{
    if (dev_online(__OTG)) {
        u8 vol = dac_vol('r', 0);
        Sys_IRInput = 0;
        work_mode_save();
        key_table_sel(usb_msg_filter);
        /* dac_init_api(32000); */
        decoder_init();
#if KEY_IR_EN
        void irflt_restore(void);
        usb_sof_trim(NULL, (void *)irflt_restore);
        usb_device_mode(0, 0);
#else
        usb_sof_trim(NULL, NULL);
        usb_device_mode(0, 0);
#endif
#if TCFG_USB_EXFLASH_UDISK_ENABLE
        void *device = dev_open(__EXT_FLASH_NANE, 0);
        if (device != NULL) {
            dev_ioctl(device, IOCTL_SET_READ_USE_CACHE, 1);
            dev_ioctl(device, IOCTL_SET_CACHE_SYNC_ISR_EN, 0);
        }
#endif
        usb_start();
        USB_devie_mode_loop();
        usb_stop();
#if TCFG_USB_EXFLASH_UDISK_ENABLE
        if (device != NULL) {
            dev_close(device);
        }
#endif
        dac_vol(0, vol);
    } else {
        work_mode++;
    }
}
#endif

