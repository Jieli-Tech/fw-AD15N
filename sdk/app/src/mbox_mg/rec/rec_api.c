#include "typedef.h"
#include "rec_mode.h"
#include "msg.h"
#include "errno-base.h"
#include "sdmmc/sd_host_api.h"
#include "rec_api.h"
#include "vfs.h"
#include "music/device.h"
#include "music/get_music_file.h"
#include "vm_api.h"
#include "music/break_point.h"
#include "vfs_fat.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"
#define REC_MAX_FILE_NUM			9999

static char path_buff[25];
static enc_file_info_t enc_file_info;
extern void vfs_info_disable(u8 device_index);

static char *path_create(const char *folder, const char *filename)
{
    if ((strlen(folder) + strlen(filename) + strlen("/") + 1) > sizeof(path_buff)) {
        log_info("path too tiny ,error !!! \n");
        return NULL;
    }

    memset(path_buff, 0x00, sizeof(path_buff));
    strcat(path_buff, folder);
    strcat(path_buff, "/");
    strcat(path_buff, filename);

    log_info("path_buff:%s \n", path_buff);
    return path_buff;
}

static void numtostr(char *str, u32 num, u8 bitnum)
{
    while (bitnum --) {
        *str-- = (num % 10) + '0';
        num /= 10;
    }
}

void *enc_create_file(void *device, const char *folder, const char *filename)
{
    char *path = NULL;
    if (!device || !folder || !filename) {
        log_info("parametre error !!! \n");
        return NULL;
    }

    path = path_create(folder, filename);
    if (path == NULL) {
        return NULL;
    }

    void *pfs = NULL, *pfile = NULL;
    u32 res = vfs_mount(&pfs, device, "fat");
    if (res != 0) {
        log_info("dev mount error !!! \n");
        goto _exit;
    }
    //search available file num
    u32 last_num = 0;
    u32 file_total_num = 0;
    vfs_get_encfolder_info(pfs, (char *)folder, "MP3", &last_num, &file_total_num);
    last_num ++;
    if (last_num >= REC_MAX_FILE_NUM && file_total_num >= REC_MAX_FILE_NUM) {
        log_info("file num overflow !!!! \n");
        goto _exit;
    }
    if (file_total_num == 0) {
        //create folder
        vfs_mk_dir(pfs, (char *)folder, 0);
    }

    char *file_num_ptr = strchr(path, '.') - 1;
    int err_cnt = 0;

_get_file:
    if (last_num > REC_MAX_FILE_NUM) {
        last_num = 0;
    }
    numtostr(file_num_ptr, last_num, 4);

    res = vfs_openbypath(pfs, &pfile, path);
    if (res == 0) {
        //file exist,open next
        vfs_file_close(&pfile);
        pfile = NULL;
        err_cnt ++;
        if (err_cnt >= REC_MAX_FILE_NUM) {
            log_info("err cnt full \n");
            goto _exit;
        }
        last_num ++;
        goto _get_file;
    } else {
        //create file
        log_info("REC FILE NAME:%s \n", path);
        void *temp_path = path;//clean warning
        res = vfs_createfile(pfs, &pfile, (u32 *)temp_path);

        if (res != 0) {
            log_info("open file error !!! \n");
            goto _exit;
        }
    }

    enc_file_info.pfs = pfs;
    enc_file_info.pfile = pfile;
    enc_file_info.device_hdl = device;
    return &enc_file_info;

_exit:
    if (pfile) {
        vfs_file_close(&pfile);
        pfile = NULL;
    }
    if (pfs) {
        vfs_fs_close(&pfs);
        pfs = NULL;
    }
    return NULL;
}

void enc_file_close(void *enc_hdl, u8 enc_bp_save_flg)
{
    if (NULL == enc_hdl) {
        return;
    }

    enc_file_info_t *info = (enc_file_info_t *)enc_hdl;
    if (info->pfile) {
        vfs_file_close(&(info->pfile));
    }
    if (info->pfs) {
        if (enc_bp_save_flg) {
            u32 index = 0;
            void *pfile = NULL;
            u32 res = vfs_openbypath(info->pfs, &pfile, path_buff);
            if (res == 0) {
                struct vfs_attr attr = {0};
                vfs_get_attrs(pfile, &attr);
                vfs_file_close(&pfile);
                save_rec_break_point(attr.sclust, index);
            }

        }

        vfs_fs_close(&(info->pfs));
    }
}

void *enc_file_open(const char *folder, const char *filename)
{
    void *device = rec_find_device();
    if (device == NULL) {
        log_info("device null !!! \n");
        return NULL;
    }
#if TCFG_USB_EXFLASH_UDISK_ENABLE
    if (dev_online(__EXT_FLASH_NANE)) {
        dev_ioctl(device, IOCTL_SET_READ_USE_CACHE, 1);
    }
#endif

    enc_file_info_t *info = enc_create_file(device, folder, filename);

    if (info == NULL) {
        device_close(device);
    } else {
        vfs_info_disable(device_active);	//清除设备扫描信息，提示该设备内文件有改动
    }

    return info;
}

