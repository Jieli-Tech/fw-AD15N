#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "app_dev_mg.h"
#include "music_api.h"
#include "device.h"
#include "app_config.h"
#include "msg.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"

static u8 app_active_dev = APP_DEV_INNER_FLASH;

#if (TFG_DEV_UPGRADE_SUPPORT)
#include "update.h"
void device_upgrate_try(void)
{
    const char *dev_name_buf[] = {
        __SD0_NANE,
    };
    for (int i = 0; i < sizeof(dev_name_buf) / sizeof(dev_name_buf[0]); i++) {
        try_to_upgrade((char *)dev_name_buf[i], TFG_UPGRADE_FILE_NAM);
    }
}
#endif


#if TFG_SD_EN
static u8 sd_buffer[512];
void *sdx_dev_get_cache_buf(void)
{
    return sd_buffer;
}

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
#endif

u32 app_device_ioctl(u32 cmd, int arg)
{
    u8 dev = (u8)arg;
    switch (cmd) {
    case APP_DEV_CMD_GET:
        return app_active_dev;
        break;
    case APP_DEV_CMD_SET:
        app_active_dev = dev;
        break;
    case APP_DEV_CMD_NEXT:
        app_active_dev++;
        if (app_active_dev >= APP_DEV_MAX) {
            app_active_dev = APP_DEV_INNER_FLASH;
        }
        break;
    case APP_DEV_CMD_GET_HDL:
        if (app_active_dev == APP_DEV_INNER_FLASH) {
            return (u32)NULL;//dev_open(__SFC_NANE, 0);
        }
#if (EXT_FLASH_EN)
        else if (app_active_dev == APP_DEV_EXT_FLASH) {
            return (u32)dev_open(__EXT_FLASH_NANE, 0);
        }
#endif
        break;
    case APP_DEV_CMD_GET_FS_TYPE:
        if (app_active_dev == APP_DEV_INNER_FLASH) {
            return (u32)FS_TYPE_SYD;
        }
#if (EXT_FLASH_EN)
        else if (app_active_dev == APP_DEV_EXT_FLASH) {
            return (u32)FS_TYPE_SYD;
        }
#endif
        break;
    default:
        break;
    }

    return 0;
}

