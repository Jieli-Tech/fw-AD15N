#ifndef __MUSIC_DEVICE_H__
#define __MUSIC_DEVICE_H__


#include "common/mbox_common.h"
#include "config.h"


typedef enum {
    DEVICE_UDISK,               //必须占住0位置，不管是否有读U盘的功能

#ifdef SDMMCA_EN
    DEVICE_SDMMCA,
#endif

#ifdef SDMMCB_EN
    DEVICE_SDMMCB,
#endif

#if TCFG_USB_EXFLASH_UDISK_ENABLE
    DEVICE_EXTFLSH,
#endif

#if TCFG_USB_EXFLASH_UDISK_ENABLE
    MAX_DEVICE = DEVICE_EXTFLSH,
#elif  defined (SDMMCB_EN)
    MAX_DEVICE = DEVICE_SDMMCB,
#elif  defined (SDMMCA_EN)
    MAX_DEVICE = DEVICE_SDMMCA,
#else
    MAX_DEVICE = DEVICE_UDISK,
#endif

    DEVICE_AUTO_NEXT = 0x81,
    DEVICE_AUTO_PREV = 0x82,
    NO_DEVICE = 0xff,
} ENUM_DEVICE_SELECT_MODE;

enum {
    FIND_DEV_OK      = 0,				//成功找到设备
    NO_DEFINE_DEV,    					//指定设备未找到
    DEV_INIT_ERR,     					//找到设备，但初始化失败
    NO_EFFECTIVE_DEV,					//无有效可以使用的设备
    NO_DEV_ONLINE,    					//无在线设备
};

typedef struct _DEVICE_INFO {
    u16 wBreak_point_filenum;
    u16 wfileTotal;
} DEVICE_INFO;

u8 find_device(ENUM_DEVICE_SELECT_MODE);
u8 device_init(u8 dev);
u8 device_check(void);
void *device_get_active_hdl(void);
void device_close(void *device);
u8 device_active_is_online(void);
u8 device_any_online(void);
void device_upgrate_try(void);

extern u8 _data device_active;
extern u8 _data device_online;
extern _no_init bool find_break_point_file_flag;
extern _no_init DEVICE_INFO music_device_info[MAX_DEVICE + 1];

#endif

