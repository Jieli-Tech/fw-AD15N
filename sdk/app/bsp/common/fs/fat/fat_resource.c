#include "common.h"
#include "uart.h"
#include "errno-base.h"
#include "vfs.h"
#include "boot.h"
/* #include "fat/ff_api.h" */
#include "fat/tff.h"
#include "fat/fat_io_new.h"
#include "ioctl_cmds.h"
#include "device.h"
/* #include "mbr.h" */
#include "fat/fs_file_name.h"
#include "fat/mbr.h"
#include "my_malloc.h"
#include "vfs_fat.h"

extern u32 __dev_read(void *p, u8 *buf, u32 addr);
extern u32 __dev_write(void *p, u8 *buf, u32 addr);
#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

/****************resource manage*****************/
/* static FATFS g_fat_fs; */
/* static FIL g_fat_f; */
/* static char tmp_buf[644]; */
// FIL g_fat_f_lrc ;
/* static SWIN_BUF g_sector_buffer[MAX_DEEPTH]; */

/**************************************************
//       以下宏定义需要对应库修改
**************************************************/
#define FOPEN_LONG 0 //长文件名打开方式
#define RENAME_ENABLE 0 //重命名使能
#define W_WOL_ENABLE 0 //写卷标使能
//////////////////////////////////////////////////

#if FOPEN_LONG
static char lfn_buffer[LFN_MAX_SIZE] AT(.fat_buf);
#endif
static FAT_SCANDEV scan_buffer AT(.fat_buf);
/* static char ff_apis_buffer[sizeof(FF_APIS)]; */
#if FOPEN_LONG || RENAME_ENABLE || W_WOL_ENABLE
static char tmp_buf[512 + 260 + 256 + 6] AT(.fat_tmp_buf);
#else
static char tmp_buf[512] AT(.fat_tmp_buf);
#endif

FATFS *fat_fshdl_alloc(void)
{
    return  my_malloc(sizeof(FATFS), MM_FATFS);
    /* memset((u8 *)&g_fat_fs, 0x00, sizeof(FATFS)); */
    /* return &g_fat_fs; */
}

FATFS *fat_fshdl_free(FATFS *fshdl)
{
    return my_free(fshdl);
}

FIL *fat_fhdl_alloc(void)
{
    return  my_malloc(sizeof(FIL), MM_FATFF);
    /* return &g_fat_f; */
}

FIL *fat_fhdl_free(FIL *pfile)
{
    return my_free(pfile);
}

void *fat_lfn_alloc(void)
{
#if FOPEN_LONG
    memset((u8 *)&lfn_buffer, 0x00, LFN_MAX_SIZE);
    return &lfn_buffer;
#else
    return NULL;
#endif
}

void fat_lfn_free(void *lfn_buffer)
{

}

void *fat_tmp_alloc(void)
{
    /* return  my_malloc(sizeof(FIL) + 1024, MM_FAT_TMP); */
    memset((u8 *)&tmp_buf, 0x00, sizeof(tmp_buf));
    return &tmp_buf;
}

void *fat_tmp_free(void *tmp_buf)
{
    /* return my_free(tmp_buf); */
    return 0;
}

#if 0
FF_APIS *fat_ff_apis_alloc(void)
{
    memset((u8 *)&ff_apis_buffer, 0x00, sizeof(ff_apis_buffer));
    return (FF_APIS *)&ff_apis_buffer;
    /* return  my_malloc(sizeof(FF_APIS), MM_FF_APIS_BUF); */
}

FF_APIS *fat_ff_apis_free(FF_APIS *ff_apis_buf)
{
    /* return my_free(ff_apis_buf); */
    return 0;
}
#endif

SWIN_BUF *fat_wbuf_alloc(void)
{
    return  my_malloc(sizeof(SWIN_BUF), MM_SWIN_BUF);
}

SWIN_BUF *fat_wbuf_free(SWIN_BUF *tmp_wbuf)
{
    return my_free(tmp_wbuf);

}

VFSCAN *fat_vfscan_alloc(void)
{
    return  my_malloc(sizeof(VFSCAN), MM_VFSCAN_BUF);
}

VFSCAN *fat_vfscan_free(VFSCAN *fsn_wbuf)
{
    return my_free(fsn_wbuf);

}

FAT_SCANDEV *fat_scan_alloc(void)
{
    memset((u8 *)&scan_buffer, 0x00, sizeof(FAT_SCANDEV));
    return (FAT_SCANDEV *)&scan_buffer;
    /* return  my_malloc(sizeof(FAT_SCANDEV), MM_SCAN_BUF); */
}

FAT_SCANDEV *fat_scan_free(FAT_SCANDEV *scan_buf)
{
    /* return my_free(scan_buf); */
    return 0;
}

/****************api*****************/
u32 fat_monut_api(void **ppfs, void *p_device)
{
    _FS_DEV_INFO dev_info;

    if (p_device == NULL) {
        return -1;
    }
    {
        //get mbr
        MBR_DRIVE_INFO mbr_info;
        int res = mbr_scan_parition(&mbr_info, p_device);
        if (res != MBR_OK && mbr_info.drive_cnt) {
            log_info("mbr error !!!! \n");
            return -1;
        }

        dev_info.drive_base = mbr_info.drive_boot_sect[0];
    }

    dev_ioctl(p_device, IOCTL_GET_BLOCK_SIZE, (u32)&dev_info.block_size);
    dev_info.block_size /= 512;
    dev_info.hdl = p_device;
    /* dev_info.read_p = dev_lba_read; */
    /* dev_info.write_p = dev_lba_write; */
    dev_info.read_p = __dev_read;
    dev_info.write_p = __dev_write;

    s32 res = fat_drive_open(ppfs, &dev_info);
    if (res != 0) {
        log_info("fat_mount error:%d !!! \n", res);
        return -1;
    }

    return 0;
}


u32 fat_openR_api(void *pfs, void **ppfile, const char *path)
{
    return fat_open(pfs, ppfile, (char *)path, "r");
}

u32 fat_openW_api(void *pfs, void **ppfile, u32 *pindex)
{
    return fat_open(pfs, ppfile, (char *)pindex, "w+");
}

/* u32 fat_openbyfilenum_api(void *pfs, void **ppfile, u32 filenum) */
/* { */
/*     s32 res = fat_get_file_byindex(pfs, ppfile, filenum); */
/*     if (res != 0) { */
/*         log_info("file open error !!! \n"); */
/*         return -1; */
/*     } */
/*  */
/*     return 0; */
/* } */

u32 fat_read_api(void *pfile, void *buff, u32 len)
{
    int rlen = fat_read(pfile, buff, len);
    if (rlen < 0) {
        return 0;
    }
    return rlen;
}

u32 fat_seek_api(void *pfile, u32 offset, u32 fromwhere)
{
    return fat_seek(pfile, fromwhere, offset);
}

u32 fat_write_api(void *pfile, void *buff, u32 len)
{
    u32 wlen = fat_write(pfile, buff, len);
    if (wlen != len) {
        return 0;
    }
    return wlen;
}

u32 fat_file_close_api(void **ppfile)
{
    s32 res = fat_close(ppfile);
    return (u32) * ppfile;
//    if (res != 0) {
//        return -1;
//    }
//
//    return 0;
}

u32 fat_fs_close_api(void **ppfs)
{
    s32 res = fat_drive_close(ppfs);
    return (u32) * ppfs;
//    if (res != 0) {
//        return -1;
//    }
//
//    return 0;
}

u32 fat_delete_file_api(void *pfile)
{
    s32 res = fat_delete(pfile);
    return res;
}

int fat_attrs_api(void *pfile, void *attr)
{
    return fat_get_attrs(pfile, (struct vfs_attr *)attr);
}

u32 fat_file_name_api(void *pfile, void *name, u32 len)
{
    return fat_get_name(pfile, (char *)name, len);
}

int fat_get_fsize_api(void *pfile, u32 *parm)
{
    return fat_get_fsize(pfile, parm);
}

int fat_get_pos_api(void *pfile, u32 *parm)
{
    return fat_get_ftell(pfile, parm);
}

#if 0
u32 fat_openbyindex_api(void *pfs, void **ppfile, u32 index)
{
    return fat_get_file_byindex(pfs, ppfile, index);
}

u32 fat_openbyclust_api(void *pfs, void **ppfile, u32 clust)
{
    return fat_get_file_bysclust(pfs, ppfile, clust);
}
#endif

u32 fat_openbyfile_api(void *pcvfile, void **ppfile, void *ext_name)
{
    return fat_get_file_byfile(pcvfile, ppfile, ext_name);
}

int fat_fscan_interrupt_api(struct vfscan *fsn, void *pfs, const char *path, u8 max_deepth, int (*callback)(void))
{
    return fat_fscan(fsn, pfs, path, max_deepth, callback);
    /* return fat_get_file_total(pfs, file_type, max_deepth, clust, ret_p); */
}

void fat_fscan_release_api(struct vfscan *fsn)
{
    fat_fscan_release(fsn);
}

int fat_sel_api(struct vfscan *fsn, void *pfs, int sel_mode, void **ppfile, int arg)
{
    return fat_fsel(fsn, pfs, sel_mode, ppfile, arg);
}

int fat_ioctl_api(void *pfile, int cmd, int arg)
{
    switch (cmd) {
    case FS_IOCTL_FILE_INDEX:
        int *inparam = (int *)arg;
        *inparam = scan_buffer.scn.cur_file_number;
        return 0 ;
        break;
    default:
        return fat_io_ctrl(pfile, cmd, arg);
        break;
    }
    return -1;
}

const struct vfs_operations fat_vfs_ops sec_used(.vfs_operations) = {
    .fs_type = "fat",
    /* .init        = fat_init_api, */
    .mount       = fat_monut_api,
    .openbypath  = fat_openR_api,
    .createfile  = fat_openW_api,
    .read        = fat_read_api,
    .write       = fat_write_api,
    .seek        = fat_seek_api,
    .close_fs 	 = fat_fs_close_api,
    .close_file  = fat_file_close_api,
    .fget_attr   = fat_attrs_api,
    .name        = fat_file_name_api,
    .flen        = fat_get_fsize_api,
    .ftell       = fat_get_pos_api,
    .fdelete     = fat_delete_file_api,
    /* .openbyindex = fat_openbyindex_api, */
    /* .openbyclust = fat_openbyclust_api, */
    .openbyfile  = fat_openbyfile_api,
    .fscan_interrupt  = fat_fscan_interrupt_api,
    .fscan_release  = fat_fscan_release_api,
    .fsel        = fat_sel_api,
    .ioctl       = fat_ioctl_api,
};

#if 0
#define SET_BP_TEST  0
#define CREATE_DIR_TEST 0
#define CREATE_FILE_TEST 0
#define LONG_NAME_CREATE_FILE_TEST 0
#define VFS_OPEN_BY_PATH 0
#define VFS_OPEN_BY_NUMBER 0
#define VFS_DELETE_FILE 1
static u8 buf_test[512];
extern void wdt_clear(void);
static const char scan_parm_test[] = "-t"
                                     "MP1MP2MP3WAVTXT"
                                     " -sn -r"
                                     ;

struct vfscan			*fsn_test;//设备扫描句柄
void fat_demo(void)
{
    /* void *device = device_get_dev_hdl("sd0");; */
    void *device = dev_open("sd0", NULL);
    /* void *device = dev_open("udisk0", NULL); */
    if (device == NULL) {
        log_info("dev null !!!! \n");
        return;
    }
    static void *pfs = NULL, *pfile = NULL;
    /* u32 res = fat_monut_api(&pfs, device); */
    u32 res = vfs_mount(&pfs, device, "fat");
    if (res != 0) {
        log_info("dev mount error !!! \n");
        return;
    }

#if VFS_DELETE_FILE
    int err = -1;
    vfs_openbypath(pfs, &pfile, "/123.txt");
    if (pfile) {
        err = vfs_file_delete(pfile);
        if (err) {
            r_printf(">>>[test]:err delete!!!!!!!\n");
        }
    } else {
        r_printf(">>>[test]:err open!!!!!!!\n");
    }
#endif

#if LONG_NAME_CREATE_FILE_TEST
    /* char file_path[128] = {'/', '\\', 'U', 0x31, 0x0, 0x32, 0x0, 0x33, 0x0, 0x34, 0x0, 0x35, 0x0, 0x36, 0x0, 0X37, 0x0, 0x38, 0x0, 0x39, 0x0, 0x31, 0, 0x32, 0, 0x33, 0, 0x2E, 0x0, 0x6D, 0x0, 0x70, 0x0, 0x33, 0}; */
    char file_path[128] = "/abcd.txt";
    int err = vfs_createfile(pfs, &pfile, (u32 *)file_path);
    if (err) {
        log_info("err open w+");
        return;
    }
    vfs_file_close(&pfile);
    vfs_openbypath(pfs, &pfile, file_path);
    vfs_file_close(&pfile);
#endif

#if CREATE_FILE_TEST
    u8 write_err_lag = 0;
    for (int i = 0; i < 512; i++) {
        buf_test[i] = i & 0xff;
    }
    int fs_size = 1 * 1024 * 1024;
    /* char path[64] = {"storage/sd0/C#<{(|.txt"}; */
    char path[16] = {"/**.txt"};
    /* char *change = strchr(path, '*'); */
    char *change = &path[1];
    for (int d = 0; d < 100; d++) {
        change[0] = d % 100 / 10 + '0';
        change[1] = d % 10 + '0';
        y_printf(">>>[test]:path = %s\n", path);
        int err = vfs_createfile(pfs, &pfile, (u32 *)path);
        if (err) {
            log_info("err open w+");
            return;
        }
        for (int i = 0; i < fs_size;) {
            wdt_clear();
            int wlen = vfs_write(pfile, buf_test, sizeof(buf_test));
            if (wlen != sizeof(buf_test)) {
                log_info("write errr wlen = %d", wlen);
                /* write_err_lag = 1; */
                /* goto __delete; */
                return ;
            }
            i += wlen;
            if (i == fs_size) {
                break;
            }
            if (i + wlen > fs_size) {
                wlen = fs_size - i;
            }
        }
        int parm = 0;
        vfs_ftell(pfile, &parm);
        log_info("write ok !!!!!!!!!! len = %d", parm);
        vfs_file_close(&pfile);

        r_printf(">>>[test]:goto read !!!!!!!!\n");
        vfs_openbypath(pfs, &pfile, path);
        for (int i = 0; i < fs_size;) {
            wdt_clear();
            memset(buf_test, 0, sizeof(buf_test));
            int rlen = vfs_read(pfile, buf_test, sizeof(buf_test));
            if (rlen != sizeof(buf_test)) {
                log_info("read errr rlen = %d", rlen);
                break ;
            }
            for (int j = 0; j < sizeof(buf_test); j++) {
                if (buf_test[j] != (j & 0xff)) {
                    log_info("j = %d, buf_test[j] = %d", j, buf_test[j]);
                    put_buf(buf_test, sizeof(buf_test));
                    return;
                }
            }
            i += rlen;
            if (i == fs_size) {
                break;
            }
            if (i + rlen > fs_size) {
                rlen = fs_size - i;
            }
        }
        y_printf(">>>[test]:read OK !!!!!!!!\n");
        vfs_file_close(&pfile);
    }
    /* __delete: */
    /*     if (write_err_lag) { */
    /*     } */

    return;
#endif
#if CREATE_DIR_TEST
    char folder[] = "/JL_REC";
    vfs_mk_dir(pfs, folder, 0);
    u32 last_num = 0;
    u32 total = 0;
    vfs_get_encfolder_info(pfs, folder, "MP3", &last_num, &total);
    r_printf(">>>[test]:last_num = %d, total = %d\n", last_num, total);
    return;
#endif

#if SET_BP_TEST
    /* u32 total_num = fat_get_file_total(pfs, "MP3WAV", 0, NULL); */
    u32 *find = 0;
    extern u32 *fat_set_bp_info(u32 clust, u32 fsize);
    find = fat_set_bp_info(28406, 46116908);
    r_printf(">>>[test]:*find = 0x%x\n", find);
    /* u32 total_num = vfs_fscan(pfs, "MP3WAV", 9, 0, NULL); */
    fsn_test = vfs_fscan(pfs, "/", scan_parm_test, 9, NULL);
    u32 total_num = fsn_test->file_number;
    r_printf(">>>[test]:*find = 0x%x, find = %d\n", find, *find);
    log_info("total_num:%d \n", total_num);
    /* vfs_set_lfn_buf(); ///长文件名buf设置 */
    return;
#endif
#if VFS_OPEN_BY_PATH
////////////////////////////write/////////////////////////////////////////
    /* fs_ext_setting("MP3WAVTXT"); */
    char path[64] = "/*/4*.?";
    u8 buf[128];
    fsn_test = vfs_fscan(pfs, "/", scan_parm_test, 9, NULL);
    /* vfs_openbypath(pfs, &pfile, path); */
    vfs_select(pfs, &pfile, fsn_test, FSEL_BY_PATH, (int)path);
    int name_len = vfs_file_name(pfile, buf, sizeof(buf));
    r_printf(">>>[test]:name = %s\n", buf);
    put_buf(buf, name_len);
    memset(buf_test, 0, 512);
    int rlen = vfs_read(pfile, buf_test, 512);
    r_printf(">>>[test]:rlen = %d\n", rlen);
    put_buf(buf_test, rlen);
///////////////////////////////////////////////////////////////////
#endif

#if VFS_OPEN_BY_NUMBER
    fs_ext_setting("MP3WAV");
    /* res = vfs_openbyindex(pfs, &pfile, 3); */
    res = vfs_select(pfs, &pfile, fsn_test, FSEL_BY_NUMBER, 3);
    if (res != 0) {
        log_info("file open error !!! \n");
        /* fat_file_close_api(&pfile); */
        vfs_file_close(&pfile);
        return;
    }
    log_info("file open succ \n");
    u8 file_name[VFS_FILE_NAME_LEN];
    int name_len = vfs_file_name(pfile, file_name, sizeof(file_name));
    /* fat_get_file_name(pfile, &file_name); */
    log_info(">>>>filename:%s , name_len = %d\n", file_name, name_len);
    put_buf(file_name, name_len);
    ///////////////////////////////////////////////////////////////////////
    /* res = vfs_openbyindex(pfs, &pfile, 5); */
    res = vfs_select(pfs, &pfile, fsn_test, FSEL_BY_NUMBER, 5);
    if (res != 0) {
        log_info("file open error !!! \n");
        /* fat_file_close_api(&pfile); */
        vfs_file_close(&pfile);
        return;
    }
    log_info("file open succ \n");

    //////////////////////////////////////////////////////////////////////
    struct vfs_attr attr = {0};
    vfs_get_attrs(pfile, (void *)&attr);
    r_printf(">>>[test]:sclust = %d, fsize = %d, attr =0x%x\n", attr.sclust, attr.fsize, attr.attr);
    u32 fsize = 0;
    vfs_get_fsize(pfile, (void *)&fsize);
    r_printf(">>>[test]:fsize = %d\n", fsize);
    int start = 0, end = 0;
    vfs_get_folderinfo(pfile, fsn_test, &start, &end);
    r_printf(">>>[test]:s= %d, e= %d\n", start, end);

    return;

    int pos = 0;
    static u8 buff[512];
    /* fat_read_api(pfile, buff, 100); */
    vfs_read(pfile, buff, 100);
    put_buf(buff, 100);
    /* fat_read_api(pfile, buff, 100); */
    vfs_read(pfile, buff, 100);
    put_buf(buff, 100);
    r_printf(">>>[test]:seek 0 :\n");
    vfs_seek(pfile, 0, SEEK_SET);
    vfs_read(pfile, buff, 100);
    put_buf(buff, 100);
    vfs_ftell(pfile, (void *)&pos);
    y_printf(">>>[test]:pos = %d\n", pos);

    /////////////////////////////////////////////////////////////////////
    void *file = NULL;
    /* res = vfs_openbyclust(pfs, &file, 5); */
    res = vfs_select(pfs, &file, fsn_test, FSEL_BY_SCLUST, 5);
    if (res != 0) {
        log_info("file open error !!! \n");
        vfs_file_close(&file);
        return;
    }
    log_info("file open succ \n");
    /* char file_name[12]; */
    memset(file_name, 0, 12);
    name_len = vfs_file_name(file, file_name, sizeof(file_name));
    log_info(">>>>filename:%s \n", file_name);
    /* struct vfs_attr attr = {0}; */
    /* memset(&attr, 0, sizeof(vfs_attr)); */
    vfs_get_attrs(file, (void *)&attr);
    r_printf(">>>[test]:sclust = %d, fsize = %d, attr =0x%x\n", attr.sclust, attr.fsize, attr.attr);
    vfs_file_close(&file);

#endif
    vfs_fscan_release(pfs, fsn_test);

    /* fat_file_close_api(&pfile); */
    /* fat_fs_close_api(&pfs); */
    vfs_file_close(&pfile);
    vfs_fs_close(&pfs);
}
#endif

