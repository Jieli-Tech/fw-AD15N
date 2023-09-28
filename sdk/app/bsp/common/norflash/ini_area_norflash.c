//========================================================================================================//
/*  (1)ini_area_norflash.c应用于将内置flash中用户自行开辟的空间视为一个独立的设备;
    (2)注册了设备后,可以通过设备接口读取空间内的数据;

    使用示例
    1.在sdk下载目录的isd_config.ini文件中开辟独立分区TEST;
    TEST1_ADR=AUTO;
    TEST1_LEN=24K;
    TEST1_OPT=1;

    TEST2_ADR=AUTO;
    TEST2_LEN=24K;
    TEST2_OPT=1;

    2.在device_list.c文件中，将虚拟设备加入device_table链表，并将分区名传入私有参数；
    {.name = "test1_area", .ops = &ini_area_norflash_ops, .priv_data = (void *)"TEST1"},
    {.name = "test2_area", .ops = &ini_area_norflash_ops, .priv_data = (void *)"TEST2"},

    3.初始化设备后,即可按照正常设备流程,打开设备读取对应分区数据；
*/
//========================================================================================================//
#include "errno-base.h"
#include "device.h"
#include "vfs.h"
#include "boot.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[ini_area]"
#include "log.h"

#define MAX_INI_AREA_NUM    1   //ini开辟的空间个数,根据需要调整
#define INI_AREA_BASE_PATH  "/app_area_head/"

extern const struct device_operations sfc_dev_ops;
struct ini_area_norflash_partition {
    const char *name;
    u32 start_addr;
    u32 size;
    struct device device;
};
static struct ini_area_norflash_partition ini_nor_info[MAX_INI_AREA_NUM];

static struct ini_area_norflash_partition *ini_area_info_find(const char *name)
{
    u32 index;
    struct ini_area_norflash_partition *info = NULL;
    for (index = 0; index < MAX_INI_AREA_NUM; index++) {
        info = &ini_nor_info[index];
        if (NULL == info->name) {
            continue;
        }
        if (0 == strcmp(info->name, name)) {
            return info;
        }
    }
    return NULL;
}

static struct ini_area_norflash_partition *ini_area_info_install(const char *name, char *area_name)
{
    u32 err, index;
    struct ini_area_norflash_partition *info;

    for (index = 0; index < MAX_INI_AREA_NUM; index++) {
        info = &ini_nor_info[index];
        if (NULL == info->name) {
            break;
        }
    }
    if (NULL != info->name) {
        log_error("ini_area info install fail\n");
        return NULL;
    }

    void *pvfs = 0;
    void *pvfile = 0;
    struct vfs_attr tmp_attr = {0};;
    char path_buf[32] = {0};

    strcat(path_buf, INI_AREA_BASE_PATH);
    strcat(path_buf, area_name);
    /* log_info("%s_AREA path:%s\n", area_name, path_buf); */

    err = vfs_mount(&pvfs, (void *)NULL, (void *)NULL);
    ASSERT(!err, "fii vfs mount : 0x%x\n", err)
    err = vfs_openbypath(pvfs, &pvfile, path_buf);
    ASSERT(!err, "fii vfs openbypath : 0x%x\n", err)
    err = vfs_ioctl(pvfile, FS_IOCTL_FILE_ATTR, (int)&tmp_attr);
    ASSERT(!err, "fii vfs ioctl : 0x%x\n", err)
    /* log_info("%s_AREA size : 0x%x sclust : 0x%x\n", tmp_attr.fsize, tmp_attr.sclust); */
    vfs_file_close(&pvfile);
    vfs_fs_close(&pvfs);

    info->name         = name;
    info->start_addr   = tmp_attr.sclust;
    info->size         = tmp_attr.fsize;

    log_info("area_info:%s 0x%x 0x%x\n", info->name, info->start_addr, info->size);
    return info;
}

int ini_area_norflash_dev_init(const struct dev_node *node, void *arg)
{
    char *area_name = (char *)arg;
    struct ini_area_norflash_partition *info = ini_area_info_find(node->name);
    if (NULL == info) {
        ini_area_info_install(node->name, area_name);
    } else {
        log_error("Ini_area:%s init fail\n", node->name);
        return 1;
    }

    return sfc_dev_ops.init(node, NULL);
}

int ini_area_norflash_dev_open(const char *name, struct device **device, void *arg)
{
    u32 err;
    struct ini_area_norflash_partition *info = ini_area_info_find(name);
    log_info("info->name:%s name:%s\n", info->name, name);
    if ((info->name == NULL) || (0 != strcmp(info->name, name))) {
        return 1;
    }

    err = sfc_dev_ops.open(name, device, arg);
    *device = &info->device;
    (*device)->private_data = (void *)info;

    return err;
}

int ini_area_norflash_byte_read(struct device *device, void *buf, u32 len, u32 offset)
{
    u32 real_offset;
    struct ini_area_norflash_partition *info = (struct ini_area_norflash_partition *)device->private_data;

    if (offset >= info->size) {
        return 0;
    }
    if ((len + offset) > info->size) {
        len = info->size - offset;
    }
    real_offset = offset + info->start_addr;
    return sfc_dev_ops.read(device, buf, len, real_offset);
}

int ini_area_norflash_byte_write(struct device *device, void *buf, u32 len, u32 offset)
{
    u32 real_offset;
    struct ini_area_norflash_partition *info = (struct ini_area_norflash_partition *)device->private_data;

    if (offset >= info->size) {
        return 0;
    }
    if ((len + offset) > info->size) {
        len = info->size - offset;
    }
    real_offset = offset + info->start_addr;
    return sfc_dev_ops.write(device, buf, len, real_offset);
}

int ini_area_norflash_ioctl(struct device *device, u32 cmd, u32 arg)
{
    struct ini_area_norflash_partition *info = (struct ini_area_norflash_partition *)device->private_data;

    int ret = 0;
    u32 erase_size = 0;
    u32 align_addr = 0;

    switch (cmd) {
    case IOCTL_ERASE_PAGE:
        erase_size = 256;
        align_addr = arg / erase_size * erase_size;
        goto __ini_area_earse;
    case IOCTL_ERASE_SECTOR:
        erase_size = 4 * 1024;
        align_addr = arg / erase_size * erase_size;
        goto __ini_area_earse;
    case IOCTL_ERASE_BLOCK:
        erase_size = 64 * 1024;
        align_addr = arg / erase_size * erase_size;
__ini_area_earse:
        if ((align_addr + erase_size) > info->size) {
            return -EINVAL;
        }
        ret = sfc_dev_ops.ioctl(device, cmd, align_addr + info->start_addr);
        break;
    case IOCTL_GET_CAPACITY:
        *((int *)arg) = info->size;
        break;
    default :
        ret = -ENOTTY;
        break;
    }

    return ret;
}

bool ini_area_norflash_dev_online(const struct dev_node *node)
{
    return 1;
}

int ini_area_norflash_dev_close(struct device *device)
{
    return 0;
}

const struct device_operations ini_area_norflash_ops = {
    .init   = ini_area_norflash_dev_init,
    .online = ini_area_norflash_dev_online,
    .open   = ini_area_norflash_dev_open,
    .read   = ini_area_norflash_byte_read,
    .write  = ini_area_norflash_byte_write,
    .bulk_read   = NULL,
    .bulk_write  = NULL,
    .ioctl  = ini_area_norflash_ioctl,
    .close  = ini_area_norflash_dev_close,
};
