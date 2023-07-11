#include "vfs.h"
#include "msg.h"
#include "errno-base.h"
#include "device.h"
#include "device_mge.h"
#include "update.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[dev_app]"
#include "log.h"

const char *device_name[MAX_DEVICE] = {
    NULL,               //只读内置flash，用于读取资源文件
    __SFC_NANE,         //可读写内置flash，用于挂载norfs或访问虚拟设备
#if EXT_FLASH_EN
    __EXT_FLASH_NANE,   //可读写外挂flash
#endif
};

typedef struct __device_mge_t {
    void *p_device;
    u8 device_used_cnt;
} device_mge_t;
static device_mge_t device_mge[MAX_DEVICE];

/*----------------------------------------------------------------------------*/
/**@brief  设备文件文件句柄申请接口
   @param  device_id:需要打开的设备号,如 INNER_FLASH_RO
   @return 失败: NULL
           成功: 返回设备挂载的文件系统句柄
   @note
*/
/*----------------------------------------------------------------------------*/
void *device_open(u8 device_id)
{
    if (device_id >= MAX_DEVICE) {
        return NULL;
    }

    /* log_info("001"); */

    if (INNER_FLASH_RO == device_id) {
        return NULL;
    }

    device_mge[device_id].p_device = dev_open((void *)device_name[device_id], 0);
    if (NULL == device_mge[device_id].p_device) {
        return NULL;
    }
    device_mge[device_id].device_used_cnt++;

    /* log_info("dev:%d open cnt:%d\n",  device_id, device_mge[device_id].device_used_cnt); */

    return device_mge[device_id].p_device;
}

/*----------------------------------------------------------------------------*/
/**@brief  设备文件文件句柄释放接口
   @param  device_id:需要打开的设备号,如 INNER_FLASH_RO
   @return 设备已被关闭: 0
           设备关闭成功: 1
   @note
*/
/*----------------------------------------------------------------------------*/
u32 device_close(u8 device_id)
{
    if ((device_id >= MAX_DEVICE) || (0 == device_mge[device_id].device_used_cnt)) {
        return 0;
    }

    /* log_info("00b"); */
    if (INNER_FLASH_RO == device_id) {
        return 0;
    }

    u32 res = 0;

    device_mge[device_id].device_used_cnt--;
    if (0 == device_mge[device_id].device_used_cnt) {
        u32 retry = 20;
        do {
            res = dev_close(device_mge[device_id].p_device);
            if (0 == res) {
                device_mge[device_id].p_device = NULL;
            }
            retry--;
        } while (res && (0 != retry));
    }
    if (0 != res) {
        log_info("close_device FAIL\n");
        return 0;
    }

    /* log_info("dev:%d close cnt:%d\n",  device_id, device_mge[device_id].device_used_cnt); */

    return 1;
}

void *device_obj(u32 index)
{
    if ((index > MAX_DEVICE) || (INNER_FLASH_RO == index)) {
        return 0;
    }

    return device_mge[index].p_device;
}

u32 device_status(u32 index, bool mode)
{
    if (index > MAX_DEVICE) {
        return E_IDEV_ILL;
    }
    if (INNER_FLASH_RO == index) {
        return 0;
    }
    bool lost = 0;
    if (!dev_online((void *)&device_name[index][0])) {
        log_info("Ask device:%d is't online ", index);
        if (0 != mode) {
            device_close(index);
            lost = 1;
        }
    } else {
        log_info("device:%d status is ok\n", index);
        return 0;
    }
    bool bres = dev_online(&device_name[index][0]);
    if (bres) {
        if (lost) {
            return E_DEV_LOST;
        } else {
            return 0;
        }
    } else {
        return E_DEV_OFFLINE;
    }
}

u32 device_online(void)
{
    u32 online = 0;
    for (u32 i = 0; i < MAX_DEVICE; i++) {
        if (INNER_FLASH_RO == i) {
            online |= BIT(i);
            continue;
        }
        if (E_DEV_OFFLINE != device_status(i, 1)) {
            online |= BIT(i);
        }
    }
    return online;
}

#if defined(TFG_DEV_UPGRADE_SUPPORT) && (1 == TFG_DEV_UPGRADE_SUPPORT)
void device_update(u8 dev_idx)
{
    const char *dev_name[] = {
        __UDISK0,
        __SD0_NANE,
    };
    log_info("dev name %s\n", dev_name[dev_idx]);
    try_to_upgrade((char *)dev_name[dev_idx], TFG_UPGRADE_FILE_NAME);
}
#endif

#if defined(TFG_SD_EN) && (TFG_SD_EN)
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
