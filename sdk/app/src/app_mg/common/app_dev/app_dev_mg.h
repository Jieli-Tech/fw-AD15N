#ifndef _APP_DEV_MG_H_
#define _APP_DEV_MG_H_

enum {
    APP_DEV_INNER_FLASH = 0,
#if (EXT_FLASH_EN)
    APP_DEV_EXT_FLASH,
#endif

    APP_DEV_MAX,
};

enum {
    APP_DEV_CMD_SET,
    APP_DEV_CMD_GET,
    APP_DEV_CMD_NEXT,
    APP_DEV_CMD_GET_HDL,
    APP_DEV_CMD_GET_FS_TYPE,
};

u32 app_device_ioctl(u32 cmd, int arg);
void device_upgrate_try(void);
#endif
