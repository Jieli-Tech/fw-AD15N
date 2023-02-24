#include "common/mbox_common.h"
#include "common/hot_msg.h"
#include "msg.h"
#include "music/device.h"
#include "music/get_music_file.h"
#include "mbox_main.h"
#include "music/music_play.h"
#include "common/ui/ui_api.h"
#include "music/play_file.h"
#include "music/break_point.h"
#include "dac_api.h"
#include "vm.h"
#include "app_power_mg.h"
#include "audio.h"
#include "usb/host/usb_host.h"
#include "usb/device/usb_stack.h"
#include "sdmmc/sd_host_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

extern ENUM_WORK_MODE _data work_mode;
extern u8 _data main_vol_L, main_vol_R;

_no_init u16 _data input_number;	    ///<输入的数字的实际值

extern volatile u32 jiffies;
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

/*----------------------------------------------------------------------------*/
/** @brief:
    @param:
    @return:
    @author:Juntham
    @note:
*/
/*----------------------------------------------------------------------------*/
void new_device_in(u8 dev)
{
    LED_FADE_ON();
    dev -= MSG_USB_DISK_IN;
    music_device_info[dev].wBreak_point_filenum = 0;
    music_device_info[dev].wfileTotal = 0;
    vfs_info_disable(dev);
    playfile.given_file_method = PLAY_BREAK_POINT;
    playfile.given_device = (ENUM_DEVICE_SELECT_MODE)dev;
    put_msg_lifo(MSG_MUSIC_NEW_DEVICE_IN);
}
/*----------------------------------------------------------------------------*/
/**@brief   公共消息集中处理函数
   @param   key： 需要处理的消息
   @return  void
   @author: Juntham
   @note    void ap_handle_hotkey(u8 key)
*/
/*----------------------------------------------------------------------------*/
void ap_handle_hotkey(u16 key)
{
    CPU_SR_ALLOC();
    switch (key) {
    case MSG_500MS:
        music_vol_update();
        wdt_clear();
        music_vol_update();
        app_power_scan();
        audio_lookup();
        break;
    case MSG_MUTE:												//静音
        if (!Sys_Volume) {
            break;
        }
        dac_mute_toggle();
        break;

#ifdef USB_DISK_EN
    case MSG_USB_DISK_OUT:
        log_info("udisk out");
        usb_host_unmount(0);
#endif
    case MSG_SDMMCA_OUT:
#ifdef SDMMCB_EN
    case MSG_SDMMCB_OUT:
#endif
        LED_FADE_ON();
        /*当前活动设备不在线*/
        if ((device_check() == 0x0) && (work_mode == MUSIC_MODE)) {
            playfile.given_file_method = PLAY_BREAK_POINT;
            playfile.given_device = DEVICE_AUTO_NEXT;
            put_msg_lifo(MSG_MUSIC_SELECT_NEW_DEVICE);
            /*记忆断点信息*/
//#if defined BREAK_POINT_PLAY_EN && defined USE_EEPROM_MEMORY
#if defined BREAK_POINT_PLAY_EN && !defined USE_IRTC_MEMORY
            save_music_break_point(device_active, 1);
#endif
        }
        break;

#ifdef USB_DEVICE_EN
    case MSG_USB_PC_IN:
    case MSG_PC_IN:
        log_info(" pc in");
#ifdef SDMMCA_EN
        music_device_info[DEVICE_SDMMCA].wBreak_point_filenum = 0;
        music_device_info[DEVICE_SDMMCA].wfileTotal = 0;
        vfs_info_disable(DEVICE_SDMMCA);
#endif
#ifdef SDMMCB_EN
        music_device_info[DEVICE_SDMMCB].wBreak_point_filenum = 0;
        music_device_info[DEVICE_SDMMCB].wfileTotal = 0;
        vfs_info_disable(DEVICE_SDMMCB);
#endif
#if TCFG_USB_EXFLASH_UDISK_ENABLE
        music_device_info[DEVICE_EXTFLSH].wBreak_point_filenum = 0;
        music_device_info[DEVICE_EXTFLSH].wfileTotal = 0;
        vfs_info_disable(DEVICE_EXTFLSH);
#endif
        LED_FADE_ON();
        work_mode = USB_DEVICE_MODE;
        put_msg_lifo(MSG_CHANGE_WORK_MODE);
        break;

#endif
#ifdef USB_DEVICE_EN
    case MSG_USB_PC_OUT:
    case MSG_PC_OUT:
        log_info(" pc out1");
        usb_stop();
        LED_FADE_ON();
        break;
#endif

#ifdef LINEIN_EN
    case MSG_AUX_IN :
        if (jiffies < 100) {
            //上电前1S设备插入事件不响应。
            break;
        }

        LED_FADE_ON();
#ifdef USB_DEVICE_EN
        if (work_mode != USB_DEVICE_MODE)
#endif
        {
            work_mode = AUX_MODE;
            put_msg_lifo(MSG_CHANGE_WORK_MODE);
        }
        break;

    case MSG_AUX_OUT :
        LED_FADE_ON();
        break;
#endif

#ifdef USB_DISK_EN
    case MSG_USB_DISK_IN  :
        log_info("udisk in");
        usb_host_mount(0, 3, 20, 200); //
#endif
#if TCFG_USB_EXFLASH_UDISK_ENABLE
    case MSG_EXTFLSH_IN:
#endif
#ifdef SDMMCA_EN
    case MSG_SDMMCA_IN :
#endif

#ifdef SDMMCB_EN
    case MSG_SDMMCB_IN :
#endif
        if (jiffies < 100) {
            //上电前1S设备插入事件不响应。
            break;
        }

        new_device_in(key);
        break;

    case MSG_NEXT_WORKMODE:
        work_mode++;
        put_msg_lifo(MSG_CHANGE_WORK_MODE);
        break;

    case MSG_VOL_UP:
        flush_all_msg();
        if (!Sys_Volume) {
            break;
        }

        dac_vol('+', 255);
        log_info("vol:%d \n", dac_vol('r', 0));
        UI_menu(MENU_MAIN_VOL);
        break;
    case MSG_VOL_DOWN:
        flush_all_msg();
        if (!Sys_Volume) {
            break;
        }

        dac_vol('-', 255);
        log_info("vol:%d \n", dac_vol('r', 0));
        UI_menu(MENU_MAIN_VOL);
        break;

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

        if (input_number > 999) {
            input_number = 0;
        }

        input_number = input_number * 10 + key;
        UI_menu(MENU_INPUT_NUMBER);
        break;
    }
}


