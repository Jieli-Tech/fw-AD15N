#include "common.h"
#include "uart.h"
#include "errno-base.h"
#include "vfs.h"
#include "boot.h"
#include "ioctl_cmds.h"
#include "device.h"
/* #include "mbr.h" */
#include "my_malloc.h"
#include "simple_fat/simple_fat.h"

extern u32 __dev_read(void *p, u8 *buf, u32 addr);
extern u32 __dev_write(void *p, u8 *buf, u32 addr);
#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

extern FIL pfi;
/****************api*****************/
u32 smpl_fat_monut_api(void **ppfs, void *p_device)
{
    if ((u8 *)p_device  == NULL) {
        return -1;
    }

    return fat_init((u8 *)p_device);
}


u32 smpl_fat_openR_api(void *pfs, void **ppfile, const char *path)
{
    return f_open(&pfi, (char *)path, FA_OPEN_EXISTING);
}

u32 smpl_fat_read_api(void *pfile, void *buff, u32 len)
{
    u32 rlen = f_read(&pfi, buff, len);
    if (rlen != len) {
        return 0;
    }
    return rlen;
}

u32 smpl_fat_seek_api(void *pfile, u32 offset, u32 fromwhere)
{
    return f_seek(&pfi, offset);
}

u32 smpl_fat_file_close_api(void **ppfile)
{
    return (u32) * ppfile;
}

u32 smpl_fat_fs_close_api(void **ppfs)
{
    return (u32) * ppfs;
}

const struct vfs_operations smpl_fat_vfs_ops sec_used(.vfs_operations) = {
    .fs_type = "fat",
    /* .init        = fat_init_api, */
    .mount       = smpl_fat_monut_api,
    .openbypath  = smpl_fat_openR_api,
    .createfile  = NULL,
    .read        = smpl_fat_read_api,
    .write       = NULL,
    .seek        = smpl_fat_seek_api,
    .close_fs 	 = smpl_fat_fs_close_api,
    .close_file  = smpl_fat_file_close_api,
    .fget_attr   = NULL,
    .name        = NULL,
    .flen        = NULL,
    .ftell       = NULL,
    .openbyindex = NULL,
    .openbyclust = NULL,
    .openbyfile  = NULL,
    /* .fscan       = NULL, */
    .ioctl       = NULL,
};

