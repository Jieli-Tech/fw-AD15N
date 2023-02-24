#include "common/mbox_common.h"
#include "device.h"
#include "play_file.h"
#include "get_music_file.h"
#include "music_play.h"
#include "break_point.h"
#include "dev_mg/device.h"
#include "msg.h"
#include "update.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

u8 _data device_online;			    ///<设备在线标记
u8 _data device_active;                ///<当前在线活动设备
bool find_break_point_file_flag;

#ifndef NDEBUG
u8 const file_type[] = "WAV";  ///<解码文件格式
#else
u8 const file_type[] = "WAVMP?";  ///<解码文件格式
#endif

int device_status_emit(const char *device_name, const u8 status)
{
    int event = 0;
    log_info("device_name:%s status:%d \n", device_name, status);

    if (!strcmp(device_name, "sd0")) {
        if (status) {
            post_event(EVENT_SD0_IN);
            log_info(">>>>>>>sd online \n");
        } else {
            post_event(EVENT_SD0_OUT);
            log_info(">>>>>>>sd offline \n");
        }
    }
    return 0;
}

void device_upgrate_try(void)
{
#if TFG_DEV_UPGRADE_SUPPORT
    const char *dev_name = NULL;
#ifdef SDMMCA_EN
    if (device_active == DEVICE_SDMMCA) {
        dev_name = __SD0_NANE;
    }
#endif

#if TCFG_USB_EXFLASH_UDISK_ENABLE
    if (device_active == DEVICE_EXTFLSH) {
        dev_name = __EXT_FLASH_NANE;
    }
#endif

    if (device_active == DEVICE_UDISK) {
        dev_name = __UDISK0;
    }

    if (dev_name) {
        try_to_upgrade((char *)dev_name, TFG_UPGRADE_FILE_NAM);
    }

#endif
}

void *device_get_active_hdl(void)
{
    void *hdl = NULL;

#ifdef SDMMCA_EN
    if (device_active == DEVICE_SDMMCA) {
        hdl = dev_open(__SD0_NANE, 0);
    }
#endif

#if TCFG_USB_EXFLASH_UDISK_ENABLE
    if (device_active == DEVICE_EXTFLSH) {
        hdl = dev_open(__EXT_FLASH_NANE, 0);
    }
#endif

    if (device_active == DEVICE_UDISK) {
        hdl = dev_open(__UDISK0, 0);
    }

    return hdl;
}
void device_close(void *device)
{
    if (device) {
        dev_close(device);
    }
}
/*----------------------------------------------------------------------------*/
/**@brief   检测当前活动的设备
   @param   void
   @return  0：当前活动的设备不在线：其他：当前活动的设备
   @author:
   @note    u8 device_check(void)
*/
/*----------------------------------------------------------------------------*/
u8 device_check(void)
{
#ifdef SDMMCA_EN
    if (dev_online("sd0") == 0) {
        device_online &= ~BIT(DEVICE_SDMMCA);
    } else {
        device_online |= BIT(DEVICE_SDMMCA);
    }
#endif

#ifdef SDMMCB_EN
    if (dev_online("sd1") == 0) {
        device_online &= ~BIT(DEVICE_SDMMCB);
    } else {
        device_online |= BIT(DEVICE_SDMMCB);
    }
#endif

#ifdef USB_DISK_EN
    if (dev_online("udisk0")) { //Host online
        log_info("udisk0 online \n");
        device_online |= BIT(DEVICE_UDISK);
    } else {
        log_info("udisk0 offline \n");
        device_online &= ~BIT(DEVICE_UDISK);
    }
#endif

#if TCFG_USB_EXFLASH_UDISK_ENABLE
    if (dev_online(__EXT_FLASH_NANE)) {
        log_info("extflash online \n");
        device_online |= BIT(DEVICE_EXTFLSH);
    } else {
        log_info("extflash offline \n");
        device_online &= ~BIT(DEVICE_EXTFLSH);
    }
#endif

    return (BIT(device_active)) & device_online;
}

u8 device_active_is_online(void)
{
#ifdef SDMMCA_EN
    if (device_active == DEVICE_SDMMCA) {
        if (dev_online("sd0")) {
            return true;
        }
    }
#endif

#ifdef SDMMCB_EN
    if (device_active == DEVICE_SDMMCB) {
        if (dev_online("sd1")) {
            return true;
        }
    }
#endif

#ifdef USB_DISK_EN
    if (device_active == DEVICE_UDISK) {
        if (dev_online("udisk0")) {
            return true;
        }
    }
#endif

#if TCFG_USB_EXFLASH_UDISK_ENABLE
    if (device_active == DEVICE_EXTFLSH) {
        if (dev_online(__EXT_FLASH_NANE)) {
            return true;
        }
    }
#endif

    return false;
}

u8 device_any_online(void)
{
#ifdef SDMMCA_EN
    if (dev_online("sd0")) {
        return true;
    }
#endif

#ifdef SDMMCB_EN
    if (dev_online("sd1")) {
        return true;
    }
#endif

#ifdef USB_DISK_EN
    if (dev_online("udisk0")) {
        return true;
    }
#endif

#if TCFG_USB_EXFLASH_UDISK_ENABLE
    if (dev_online(__EXT_FLASH_NANE)) {
        return true;
    }
#endif

    return false;
}

/*----------------------------------------------------------------------------*/
/**@brief   设备初始化
   @param   无
   @return  0：找到有效设备；其他：没有找到有效设备
   @author:
   @note    u8 device_init(void)
*/
/*----------------------------------------------------------------------------*/
u8 device_init(u8 dev)
{
#if 0
    fs_ext_setting(file_type);
#endif

    //if (device_check())  ///<当前需要活动的设备在线
    if (fs_mount()) {
        return 0xff;
    }
    return 0;
}
/*----------------------------------------------------------------------------*/
/**@brief   查找指定的设备或自动查找下一个有效设备
   @param   select : 1：查找SD设备；2：查找USB mass storage设备; 0x81:查找上一个设备；0x82:查找下一个设备;其它：无效
   @return  0：成功查找到一个设备；1:指定的设备没有找到；2：只有一个设备，查找上一个设备或下一个设备无效;3:找到设备，但初始化不成功;4：无在线设备
   @author:
   @note    u8 find_device(u8 select)
*/
/*----------------------------------------------------------------------------*/
u8 find_device(ENUM_DEVICE_SELECT_MODE select)
{
    u8 i;

    /*Maskrom File by number*/
    //fs_getfile_bynumber(&ff_api,ff_api.fs_msg.fileNumber);

    device_check();

    i = mbox_get_one_count(device_online & 0x07);

    if (i == 0) {			//无设备
        device_active = NO_DEVICE;
        return NO_DEV_ONLINE;
    }

    if (select == DEVICE_AUTO_PREV) {	//查找上一个设备
        for (i = 0; i <= MAX_DEVICE; i++) {
            device_active--;

            if (device_active > MAX_DEVICE) {
#if TCFG_USB_EXFLASH_UDISK_ENABLE
                device_active = DEVICE_EXTFLSH;
#elif defined (SDMMCB_EN)
                device_active = DEVICE_SDMMCB;
#elif defined (SDMMCA_EN)
                device_active = DEVICE_SDMMCA;
#else
                device_active = DEVICE_UDISK;
#endif
            }
            if (device_check() == 0) {
                continue;
            }

            if (device_init(device_active)) {           //找到有效设备
                continue;
            }

            if (!fs_get_total_files()) {
                return FIND_DEV_OK;
            }
        }
        device_active = NO_DEVICE;
        return NO_EFFECTIVE_DEV;				//无有效可以使用的设备
    } else if (select == DEVICE_AUTO_NEXT) { //查找下一个设备
        for (i = 0; i <= MAX_DEVICE; i++) {
            device_active++;
            if (device_active > MAX_DEVICE) {
                device_active = DEVICE_UDISK;
            }

            if (device_check() == 0) {
                continue;
            }

            if (device_init(device_active)) {           //找到有效设备
                continue;
            }

            if (!fs_get_total_files()) {
                return FIND_DEV_OK;
            }
        }

        device_active = NO_DEVICE;
        return NO_EFFECTIVE_DEV;							//无有效可以使用的设备
    } else {            //选择指定的设备
        device_active = select;
        if (BIT(select) & device_online) {
            if (!device_init(device_active)) {
                if (!fs_get_total_files()) {
                    return FIND_DEV_OK;
                }
            }
            {
                return DEV_INIT_ERR;
            }
        } else {
            return NO_DEFINE_DEV;
        }
    }
}

