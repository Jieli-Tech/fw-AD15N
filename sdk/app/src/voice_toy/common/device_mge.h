#ifndef __TOY_DEV_MG__
#define __TOY_DEV_MG__
#include "app_config.h"

enum {
    INNER_FLASH_RO = 0, //只读内置flash，用于读取资源文件
    INNER_FLASH_RW,     //可读写内置flash，用于挂载norfs或访问虚拟设备
#if EXT_FLASH_EN
    EXT_FLASH_RW,       //可读写外挂flash
#endif
    MAX_DEVICE,
};

void *device_open(u8 device_id);
u32 device_close(u8 device_id);
void *device_obj(u32 index);
u32 device_online(void);
void device_update(u8 dev_idx);

#endif
