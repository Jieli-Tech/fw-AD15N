#include "vfs_fat.h"
#include "vfs.h"
#include "errno-base.h"
#include "device.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[vffat]"
#include "log.h"

int vfs_get_fsize(void *pvfile, void *parm)
{
    struct imount *p_vfile = pvfile;
    struct vfs_operations *ops;
    if ((void *)NULL == p_vfile) {
        return 0;
    }
    ops = p_vfile->ops;
    if (((void *)NULL != ops)  && ((void *)NULL !=  ops->flen)) {
        u32 res;
        return ops->flen(p_vfile->pfile, (u32 *)parm);
    }
    return 0;
}

int vfs_ftell(void *pvfile, void *parm)
{
    struct imount *p_vfile = pvfile;
    struct vfs_operations *ops;
    if ((void *)NULL == p_vfile) {
        return 0;
    }
    ops = p_vfile->ops;
    if (((void *)NULL != ops)  && ((void *)NULL !=  ops->ftell)) {
        u32 res;
        return ops->ftell(p_vfile->pfile, (u32 *)parm);
    }
    return 0;
}
/* --------------------------------------------------------------------------*/
/**
 * @brief 文件删除
 *
 * @param pvfile  文件句柄
 * @note SH系列文件关闭在外面应用，删除接口里面不处理
 *
 * @return 0成功
 */
/* ----------------------------------------------------------------------------*/
u32 vfs_file_delete(void *pvfile)
{
    struct imount *p_vfile = pvfile;
    if ((void *)NULL == p_vfile) {
        return E_FS_PFILE;
    }
    struct vfs_operations *ops;
    ops = p_vfile->ops;
    if (((void *)NULL != ops)  && ((void *)NULL !=  ops->fdelete)) {
        u32 res = E_VFS_OPS;
        if (NULL != (p_vfile->pfile)) {
            res = ops->fdelete(p_vfile->pfile);
            if (0 != res) {
                return E_FS_PFILE;
            }
        }
        return res;
    }
    return  E_VFS_OPS;
}

/*
 * -t  文件类型
 * -r  包含子目录
 * -d  扫描文件夹
 * -a  文件属性 r: 读， /: 非
 * -s  排序方式， t:按时间排序， n:按文件号排序
 */
static void __fscan_arg_handler(struct vfscan *fs, const char *arg)
{
    int step = 0;
    char *p;

    /*
     * fs->attr = F_ATTR_RO: 搜索只读文件
     * fs->attr = F_ATTR_ARC: 搜索非读文件
     * fs->attr = F_ATTR_ARC | F_ATTR_RO: 搜索所有文件
     */

    fs->attr = F_ATTR_ARC | F_ATTR_RO;

    while (*arg) {
        switch (step) {
        case 0:
            if (*arg == '-') {
                step = 1;
            }
            break;
        case 1:
            if (*arg == 't') {
                step = 2;
                p = fs->ftype;
                fs->scan_file = 1;
            } else if (*arg == 'r') {
                step = 0;
                fs->subpath = 1;
            } else if (*arg == 'd') {
                step = 0;
                fs->attr |= F_ATTR_DIR;
            } else if (*arg == 's') {
                step = 3;
            } else if (*arg == 'a') {
                step = 4;
            } else if (*arg == 'm') {
                step = 5;
                p = fs->filt_dir;
            }

            break;
        case 2:
            if (*arg == '-') {
                step = 1;
            } else if (*arg != ' ') {
                *p++ = *arg;
            }
            break;
        case 3:
            if (*arg == '-') {
                step = 1;
            } else if (*arg != ' ') {
                fs->sort = *arg;
            }
            break;
        case 4:
            if (*arg == '-') {
                step = 1;
            } else if (*arg == '/') {
                if (*++arg == 'r') {
                    fs->attr &= ~F_ATTR_RO;
                }
            } else if (*arg == 'r') {
                fs->attr &= ~F_ATTR_ARC;
            }
            break;
        case 5:
            if (*arg == '-') {
                step = 1;
            } else if (*arg != ' ') {
                *p++ = *arg;
            }
            break;
        default:
            break;
        }

        arg++;
    }
}

static int vfs_reset_vfscan(void *pvfs, struct vfscan *fs, u16 file_total, u16 dir_total, const char *path, const char *pram, u8 max_deepth, int (*callback)(void))
{
    int err = -1;
    struct imount *p_vfs = pvfs;
    struct vfs_operations *ops;
    if ((void *)NULL == p_vfs) {
        return -1;
    }
    ops = p_vfs->ops;

    __fscan_arg_handler(fs, pram);
    int arg[6] = {0};
    arg[0] = (int)p_vfs->pfs;
    arg[1] = (int)path;
    arg[2] = (int)file_total;
    arg[3] = (int)dir_total;
    arg[4] = (int)max_deepth;
    arg[5] = (int)callback;
    if (ops->ioctl) {
        err = ops->ioctl(fs, FS_IOCTL_RESET_VFSCAN, (int)arg);
    }
    if (err) {
        fat_vfscan_free(fs);
        fs = NULL;
    }
    return err;
}

struct vfscan *vfs_fscan_new(void *pvfs, const char *path, const char *arg, u8 max_deepth, int (*callback)(void), struct vfscan *fsn, struct vfscan_reset_info *info)
{
    int err = -1;
    struct imount *p_vfs = pvfs;
    struct vfs_operations *ops;
    if ((void *)NULL == p_vfs) {
        return NULL;
    }
    ops = p_vfs->ops;
    if (info->scan_over && info->active && fsn) {
        y_printf("\n >>>[test]:func = %s,line= %d\n", __FUNCTION__, __LINE__);
        err = vfs_reset_vfscan(pvfs, fsn, info->file_total, info->dir_total, path, arg, max_deepth, callback);
        if (err) {
            return NULL;
        }
        return fsn;
    }
    if (fsn) {
        fat_vfscan_free(fsn);
        fsn = NULL;
    }
    struct vfscan *fs = (struct vfscan *)fat_vfscan_alloc();
    ASSERT(fs);

    __fscan_arg_handler(fs, arg);
    if (ops->fscan_interrupt) {
        err = ops->fscan_interrupt(fs, p_vfs->pfs, path, max_deepth, callback);
    }
    if (err) {
        fat_vfscan_free(fs);
        fs = NULL;
    }
    info->file_total = fs->file_number;
    info->dir_total = fs->dir_totalnumber;
    info->scan_over = 1;

    return fs;
}


struct vfscan *vfs_fscan(void *pvfs, const char *path, const char *arg, u8 max_deepth, int (*callback)(void))
{
    int err = -1;
    struct imount *p_vfs = pvfs;
    struct vfs_operations *ops;
    if ((void *)NULL == p_vfs) {
        return NULL;
    }
    ops = p_vfs->ops;
    struct vfscan *fs = (struct vfscan *)fat_vfscan_alloc();
    ASSERT(fs);

    __fscan_arg_handler(fs, arg);
    if (ops->fscan_interrupt) {
        err = ops->fscan_interrupt(fs, p_vfs->pfs, path, max_deepth, callback);
    }
    /*  else { */
    /*     err = mt->ops->fscan(fs, dir, max_deepth); */
    /* } */
    if (err) {
        fat_vfscan_free(fs);
        fs = NULL;
    }

    return fs;
}

void vfs_fscan_release(void *pvfs, struct vfscan *fs)
{
    struct imount *p_vfs = pvfs;
    struct vfs_operations *ops;
    ASSERT(p_vfs);
    ops = p_vfs->ops;

    ops->fscan_release(fs);
    fat_vfscan_free(fs);
}


int vfs_select(void *pvfs, void **ppvfile, struct vfscan *fs, int sel_mode, int arg)
{

    if ((void *)NULL == *ppvfile) {
        *ppvfile = vfs_hdl_malloc();
        if ((void *)NULL == *ppvfile) {
            return E_NO_VFS;
        }
    }

    int err;
    struct vfs_operations *ops;
    struct imount *p_vfs = pvfs;
    struct imount *p_vfile = *ppvfile;
    if ((void *)NULL == p_vfs) {
        err =  E_VFS_HDL;
    }
    p_vfile->ops = p_vfs->ops;
    ops = p_vfs->ops;

    if (ops->fsel) {
        err = ops->fsel(fs, p_vfs->pfs, sel_mode, &p_vfile->pfile, arg);
    } else {
        err = E_VFS_OPS;
    }

    if (0 != err) {
        if (((void *)NULL != ops)  && ((void *)NULL !=  ops->close_file)) {
            if (NULL != (p_vfile->pfile)) {
                ops->close_file(&p_vfile->pfile);
            }

        }
        *ppvfile = vfs_fhdl_free(*ppvfile);

        /* log_info("func : %s, line : %d, p_vfile->ops : 0x%x\n", __func__, __LINE__, (u32)p_vfile->ops); */
        /* log_info("func : %s, line : %d\n", __func__, __LINE__); */
        /* vfs_file_close(ppvfile); */
    }
    return err;
}
/* int vfs_fscan(void *pvfs, const char *file_type, u8 max_deepth, u32 clust, u32 *ret_p) */
/* { */
/*     struct imount *p_vfs = pvfs; */
/*     struct vfs_operations *ops; */
/*     if ((void *)NULL == p_vfs) { */
/*         return E_VFILE_HDL; */
/*     } */
/*     ops = p_vfs->ops; */
/*     if (((void *)NULL != ops)  && ((void *)NULL !=  ops->fscan)) { */
/*         return ops->fscan(p_vfs->pfs, file_type, max_deepth, clust, ret_p); */
/*     } */
/*     return  E_VFS_OPS; */
/* } */


int vfs_mk_dir(void *pvfs, char *folder, u8 mode)
{
    struct imount *p_vfs = pvfs;
    struct vfs_operations *ops;
    if ((void *)NULL == p_vfs) {
        return 0;
    }
    ops = p_vfs->ops;
    int arg[2] = {0};
    arg[0] = (int)folder;
    arg[1] = (int)mode;
    if (((void *)NULL != ops)  && ((void *)NULL !=  ops->ioctl)) {
        u32 res;
        return ops->ioctl(p_vfs->pfs, FS_IOCTL_MK_DIR, (int)arg);
    }
    return 0;
}

int vfs_get_encfolder_info(void *pvfs, char *folder, char *ext, u32 *last_num, u32 *total_num)
{
    struct imount *p_vfs = pvfs;
    struct vfs_operations *ops;
    if ((void *)NULL == p_vfs) {
        return 0;
    }
    ops = p_vfs->ops;
    int arg[4] = {0};
    arg[0] = (int)folder;
    arg[1] = (int)ext;
    arg[2] = (int)last_num;
    arg[3] = (int)total_num;
    if (((void *)NULL != ops)  && ((void *)NULL !=  ops->ioctl)) {
        u32 res;
        return ops->ioctl(p_vfs->pfs, FS_IOCTL_GET_ENCFOLDER_INFO, (int)arg);
    }
    return 0;
}

int vfs_get_folderinfo(void *pvfile, struct vfscan *fs, int *start_num, int *end_num)
{
    struct imount *p_vfile = pvfile;
    struct vfs_operations *ops;
    if ((void *)NULL == p_vfile) {
        return 0;
    }
    ops = p_vfile->ops;
    int arg[2] = {0};
    arg[0] = (int)start_num;
    arg[1] = (int)end_num;
    if (((void *)NULL != ops)  && ((void *)NULL !=  ops->ioctl)) {
        u32 res;
        return ops->ioctl(fs, FS_IOCTL_GET_FOLDER_INFO, (int)arg);
    }
    return 0;
}

#if 0
int vfs_set_vol(void *pvfs, u8 *name)
{
    struct imount *p_vfs = pvfs;
    struct vfs_operations *ops;
    if ((void *)NULL == p_vfs) {
        return 0;
    }
    ops = p_vfs->ops;
    if (((void *)NULL != ops)  && ((void *)NULL !=  ops->ioctl)) {
        return ops->ioctl(p_vfs->pfs, FS_IOCTL_SET_VOL, (int)arg);
    }
    return 0;
}
#endif


/* --------------------------------------------------------------------------*/
/**
 * @brief    文件删除统一处理
 *
 * @param pvfs 文件系统挂载后的句柄
 * @param path 扫描路径名
 * @param param 配置参数
 * @param dir_flag 是否删除文件夹标志
 * @note 加速处理:(删除文件的时候使用)从前往后依次删除. 文件夹必须从后往前删。
 *
 * @return  0成功， 其他失败
 */
/* ----------------------------------------------------------------------------*/
static int vfs_delete_deal(void *pvfs, char *path, char *param, u8 dir_flag)
{
    u16 folder_total_file = 0;
    int d_err = 0;
    struct vfscan *fsn = NULL;
    void *pvfile = NULL;

    fsn = vfs_fscan(pvfs, path, param, 9, NULL);
    if (fsn == NULL) {
        r_printf(">>>[test]:err!!!!!! fsacn fsn fail\n");
        return 1;
    }
    folder_total_file = fsn->file_number;
    y_printf(">>>[test]:total = %d\n", folder_total_file);
    for (int i = folder_total_file; i >= 1; i--) {
        if (!dir_flag) {
            d_err = vfs_select(pvfs, &pvfile, fsn, FSEL_BY_NUMBER, folder_total_file - i + 1); //加速处理，不用找到最后一个文件。
        } else {
            d_err = vfs_select(pvfs, &pvfile, fsn, FSEL_BY_NUMBER, i);
        }
        if (pvfile == NULL) {
            r_printf(">>>[test]:err!! select file err\n");
            return 1;
        }
        putchar('D');
        d_err = vfs_file_delete(pvfile);
        if (d_err || pvfile == NULL) {
            r_printf(">>>[test]:err!! delete file err\n");
            return 1;
        }
        vfs_file_close(&pvfile);
        pvfile = NULL;
    }
    return 0;
}


/* --------------------------------------------------------------------------*/
/** @brief:文件夹删除处理
 *
 * @param pvfs 文件系统挂载后的句柄
 * @param path 需要删除的文件夹的路径
 * @author:phewlee
 * @note:
 * @date: 2024-09-05,10:16
 * @return
 */
/* ----------------------------------------------------------------------------*/
int vfs_delete_dir(void *pvfs, char *path)
{
    int err = 0;
    struct __dev *dev;
    void *pvfile = NULL;
    /* char path[128] = {0}; */

    static const u8 delete_file_param[] = "-t"
                                          "ALL"
                                          " -sn -r";

    static const u8 delete_folder_param[] = "-t"
                                            "ALL"
                                            " -sn -d -r";

#if 0
    dev = dev_manager_find_spec(dev_logo, 0);
    if (dev == NULL) {
        r_printf(">>>[test]:errr!!!!!!!!! not find dev\n");
        return 1;
    }
    char *root_path = dev_manager_get_root_path(dev);
    memcpy(path, root_path, strlen(root_path));
    memcpy(path + strlen(root_path), folder, folder_len);
#endif
    r_printf(">>>[test]:path = %s\n", path);
    err = vfs_delete_deal(pvfs, path, (char *)delete_file_param, 0);
    if (err) {
        r_printf(">>>[test]:errr!!!!!!!!! delete file deal fail\n");
        return 1;
    }
    err = vfs_delete_deal(pvfs, path, (char *)delete_folder_param, 1);
    if (err) {
        r_printf(">>>[test]:errr!!!!!!!!! delete folder  deal fail\n");
        return 1;
    }
    err = vfs_openbypath(pvfs, &pvfile, path);
    if (pvfile == NULL) {
        r_printf(">>>[test]:err open folder\n");
        return 1;
    }
    err = vfs_file_delete(pvfile);
    vfs_file_close(&pvfile);
    return err;
}

int vfs_format(void **ppvfs, const char *dev_name, const char *type, u32 clust_size, u8 create_new)
{
    if ((void *)NULL == *ppvfs) {
        if (clust_size == 0) {
            return E_NO_VFS;
        }
        *ppvfs = vfs_hdl_malloc();
        if ((void *)NULL == *ppvfs) {
            return E_NO_VFS;
        }
    }
    int err;
    void *device = dev_open(dev_name, NULL);
    if (device == NULL) {
        log_info("dev null !!!! \n");
        return E_DEV_NULL;
    }

    struct vfs_operations *ops;
    struct imount *pvfs = *ppvfs;
    list_for_each_vfs_operation(ops) {
        //log_info("%s, %s", ops->fs_type, type);
        if (0 == strcmp(ops->fs_type, type)) {
            pvfs->ops = ops;
            break;
        }
    }
    if (pvfs->ops == NULL) {
        return E_NO_FS;
    }

    ops = pvfs->ops;
    if (NULL != ops->format) {
        err = ops->format(&(pvfs->pfs), device, clust_size, create_new);
    } else {
        err = E_VFS_OPS;
    }
    if (err) {
        log_info("f_format: err = %x\n", err);
    }

    if (device) {
        dev_ioctl(device, IOCTL_FLUSH, 0);
    }

    //全部释放，format之后需要使用mount
    if (pvfs->pfs) {
        pvfs->pfs = fat_fshdl_free(pvfs->pfs);
    }
    if (pvfs->pfile) {
        pvfs->pfile = fat_fhdl_free(pvfs->pfile);
    }
    *ppvfs = vfs_fhdl_free(*ppvfs);
    dev_close(device);
    return err;
}

