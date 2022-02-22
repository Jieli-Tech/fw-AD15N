#include "vfs.h"
#include "errno-base.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"



extern struct vfs_operations vfs_ops_begin[];
extern struct vfs_operations vfs_ops_end[];

#define list_for_each_vfs_operation(ops) \
	for (ops=vfs_ops_begin; ops<vfs_ops_end; ops++)



void vfs_init(void)
{
    struct vfs_operations *ops;
    list_for_each_vfs_operation(ops) {
        if (NULL != ops->init) {
            log_info("ops_init!!!\n");
            ops->init();
        }
    }
}
u32 vfs_mount(void **ppvfs, void *device, void *type)
{
    if ((void *)NULL == *ppvfs) {
        *ppvfs = vfs_hdl_malloc();
        if ((void *)NULL == *ppvfs) {
            return E_NO_VFS;
        }
    }

    struct imount *pvfs = *ppvfs;
    struct vfs_operations *ops;
    list_for_each_vfs_operation(ops) {
        if (NULL != ops->mount) {
            if (NULL != type) {
                if (0 != strcmp(ops->fs_type, type)) {
                    continue;
                }
            } else {
                if (0 == strcmp(ops->fs_type, "norfs")) {
                    continue;
                }

            }
            if (0 == ops->mount(&(pvfs->pfs), device)) {
                pvfs->ops = ops;
                return 0;
            } else {
                if (NULL == ops->close_fs) {
                    ops->close_fs(pvfs->pfs);
                }
            }
        }
    }
    *ppvfs = vfs_fhdl_free(*ppvfs);
    return E_NO_FS;
}

u32 vfs_openbypath(void *pvfs, void **ppvfile, const char *path)
{

    if ((void *)NULL == *ppvfile) {
        *ppvfile = vfs_hdl_malloc();
        if ((void *)NULL == *ppvfile) {
            return E_NO_VFS;
        }
    }

    u32 err;

    struct vfs_operations *ops;
    struct imount *p_vfs = pvfs;
    struct imount *p_vfile = *ppvfile;
    if ((void *)NULL == p_vfs) {
        err =  E_VFS_HDL;
        goto __vfs_openbypath;
    }
    p_vfile->ops = p_vfs->ops;
    ops = p_vfs->ops;

    if (NULL != ops->openbypath) {
        err = ops->openbypath(p_vfs->pfs, &p_vfile->pfile, path);
        /* if (0 != err) { */
        /* *ppvfile = vfs_fhdl_free(*ppvfile); */
        /* } */
        /* return err; */
    } else {
        err = E_VFS_OPS;
    }
__vfs_openbypath:
    if (0 != err) {
        vfs_file_close(ppvfile);
    }
    return err;
}

u32 vfs_openbyindex(void *pvfs, void **ppvfile, u32 index)
{

    if ((void *)NULL == *ppvfile) {
        *ppvfile = vfs_hdl_malloc();
        if ((void *)NULL == *ppvfile) {
            return E_NO_VFS;
        }
    }

    u32 err;
    struct vfs_operations *ops;
    struct imount *p_vfs = pvfs;
    struct imount *p_vfile = *ppvfile;
    if ((void *)NULL == p_vfs) {
        err =  E_VFS_HDL;
    }
    p_vfile->ops = p_vfs->ops;
    ops = p_vfs->ops;

    if (NULL != ops->openbyindex) {
        err = ops->openbyindex(p_vfs->pfs, &p_vfile->pfile, index);
    } else {
        err = E_VFS_OPS;
    }
__vfs_openbyindex:
    if (0 != err) {
        vfs_file_close(ppvfile);
    }
    return err;
}

u32 vfs_openbyfile(void *pcvfile, void **ppvfile, void *ext_name)
{

    if ((void *)NULL == *ppvfile) {
        *ppvfile = vfs_hdl_malloc();
        if ((void *)NULL == *ppvfile) {
            return E_NO_VFS;
        }
    }

    u32 err;
    struct vfs_operations *ops;

    struct imount *p_cvfile = pcvfile;
    struct imount *p_vfile = *ppvfile;
    if ((void *)NULL == p_cvfile) {
        err =  E_VFS_HDL;
    }
    p_vfile->ops = p_cvfile->ops;

    ops = p_vfile->ops;

    if (NULL != ops->openbyfile) {
        err = ops->openbyfile(p_cvfile->pfile, &p_vfile->pfile, ext_name);
    } else {
        err = E_VFS_OPS;
    }
__vfs_openbyindex:
    if (0 != err) {
        log_info("openbyfile : 0x%x\n", err);
        vfs_file_close(ppvfile);
    }
    return err;
}

u32 vfs_openbyclust(void *pvfs, void **ppvfile, u32 clust)
{

    if ((void *)NULL == *ppvfile) {
        *ppvfile = vfs_hdl_malloc();
        if ((void *)NULL == *ppvfile) {
            return E_NO_VFS;
        }
    }

    u32 err;
    struct vfs_operations *ops;
    struct imount *p_vfs = pvfs;
    struct imount *p_vfile = *ppvfile;
    if ((void *)NULL == p_vfs) {
        err =  E_VFS_HDL;
    }
    p_vfile->ops = p_vfs->ops;
    ops = p_vfs->ops;

    if (NULL != ops->openbyclust) {
        err = ops->openbyclust(p_vfs->pfs, &p_vfile->pfile, clust);
    } else {
        err = E_VFS_OPS;
    }
__vfs_openbyclust:
    if (0 != err) {
        vfs_file_close(ppvfile);
    }
    return err;
}

u32 vfs_createfile(void *pvfs, void **ppvfile, u32 *pindex)
{

    if ((void *)NULL == *ppvfile) {
        *ppvfile = vfs_hdl_malloc();
        if ((void *)NULL == *ppvfile) {
            return E_NO_VFS;
        }
    }

    u32 err;
    struct vfs_operations *ops;
    struct imount *p_vfs = pvfs;
    struct imount *p_vfile = *ppvfile;
    if ((void *)NULL == p_vfs) {
        err =  E_VFS_HDL;
    }
    p_vfile->ops = p_vfs->ops;
    ops = p_vfs->ops;

    if (NULL != ops->createfile) {
        err = ops->createfile(p_vfs->pfs, &p_vfile->pfile, pindex);
    } else {
        err = E_VFS_OPS;
    }
__vfs_openbyindex:
    if (0 != err) {
        vfs_file_close(ppvfile);
    }
    return err;
}

u32 vfs_read(void *pvfile, void *buf, u32 len)
{
    struct imount *p_vfile = pvfile;
    struct vfs_operations *ops;
    if ((void *)NULL == p_vfile) {
        return 0;
    }
    ops = p_vfile->ops;
    if (((void *)NULL != ops)  && ((void *)NULL !=  ops->read)) {
        u32 res;
        res = ops->read(p_vfile->pfile, buf, len);
        return res;
    }
    return  0;
}

u32 vfs_write(void *pvfile, void *buf, u32 len)
{
    struct imount *p_vfile = pvfile;
    struct vfs_operations *ops;
    if ((void *)NULL == p_vfile) {
        return 0;
    }
    ops = p_vfile->ops;
    if (((void *)NULL != ops)  && ((void *)NULL !=  ops->write)) {
        u32 res;
        res = ops->write(p_vfile->pfile, buf, len);
        return res;
    }
    return  0;
}

u32 vfs_seek(void *pvfile, u32 offset, u32 mode)
{
    struct imount *p_vfile = pvfile;
    struct vfs_operations *ops;
    if ((void *)NULL == p_vfile) {
        return E_VFILE_HDL;
    }
    ops = p_vfile->ops;
    if (((void *)NULL != ops)  && ((void *)NULL !=  ops->seek)) {
        u32 res;
        res = ops->seek(p_vfile->pfile, offset, mode);
        return res;
    }
    return  E_VFS_OPS;
}
u32 vfs_file_close(void **ppvfile)
{
    if ((void *)NULL == *ppvfile) {
        return E_VFILE_HDL;
    }
    struct imount *p_vfile = *ppvfile;
    struct vfs_operations *ops;
    ops = p_vfile->ops;
    if (((void *)NULL != ops)  && ((void *)NULL !=  ops->close_file)) {
        u32 res = E_VFS_OPS;
        if (NULL != (p_vfile->pfile)) {
            res = ops->close_file(&p_vfile->pfile);
            if (0 != res) {
                return E_FS_PFILE;
            }
        }
        *ppvfile = vfs_fhdl_free(*ppvfile);
        return res;
    }
    return  E_VFS_OPS;
}

u32 vfs_fs_close(void **ppvfs)
{
    if ((void *)NULL == *ppvfs) {
        return E_VFS_HDL;
    }
    struct imount *p_vfs = *ppvfs;
    struct vfs_operations *ops;
    u32 res  = E_VFS_OPS;
    ops = p_vfs->ops;
    if (((void *)NULL != ops)  && ((void *)NULL !=  ops->close_fs)) {
        if (NULL != p_vfs->pfs) {
            res = ops->close_fs(&p_vfs->pfs);
        }
        /* if (0 != res) { */
        /* return E_FS_PFILE; */
        /* } */
        /* return res; */
    }

    *ppvfs = vfs_fhdl_free(*ppvfs);
    return  res;
}

u32 vfs_file_name(void *pvfile, void *name, u32 len)
{
    struct imount *p_vfile = pvfile;
    struct vfs_operations *ops;
    if ((void *)NULL == p_vfile) {
        return 0;
    }
    ops = p_vfile->ops;
    if (((void *)NULL != ops)  && ((void *)NULL !=  ops->name)) {
        u32 res;
        return ops->name(p_vfile->pfile, name, len);
    }
    return 0;
}

int vfs_get_attrs(void *pvfile, void *pvfs_attr)
{
    struct imount *p_vfile = pvfile;
    struct vfs_operations *ops;
    if ((void *)NULL == p_vfile) {
        return 0;
    }
    ops = p_vfile->ops;
    if (((void *)NULL != ops)  && ((void *)NULL !=  ops->fget_attr)) {
        u32 res;
        return ops->fget_attr(p_vfile->pfile, pvfs_attr);
    }
    return 0;
}

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

int vfs_ioctl(void *pvfile, int cmd, int arg)
{
    struct imount *p_vfile = pvfile;
    struct vfs_operations *ops;
    if ((void *)NULL == p_vfile) {
        return E_VFILE_HDL;
    }
    ops = p_vfile->ops;
    /* log_info("ops->ioctl : 0x%x  0x%lx!!\n",ops->ioctl, (u32)&ops->ioctl); */
    if (((void *)NULL != ops)  && ((void *)NULL !=  ops->ioctl)) {
        return ops->ioctl(p_vfile->pfile, cmd, arg);
    }
    return  E_VFS_OPS;
}

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

#if 0
#define vfs_demo_show(ptr,err)              \
    log_info("Demo 0x%x : %s",err,ptr);       \
    if(0 != err){                           \
        return ;                            \
    }


static u8 demo_buff[512];
void vfs_demo(void)
{
    void *pvfs = 0;
    void *pvfile = 0;
    u32 err;
    log_info("vfs init !!!\n");
    vfs_init();
    log_info("vfs init finish !!!\n");
    err = vfs_mount(&pvfs, (void *)NULL, (void *)NULL);
    vfs_demo_show("vfs_mount\n", err);
    err = vfs_openbypath(pvfs, &pvfile, "/dir_bin_01/file1.txt");
    vfs_demo_show("vfs_openbypath\n", err);
    err = vfs_read(pvfile, demo_buff, 512);
    log_info("vfs read len : %d\n", err);
    log_info_hexdump(demo_buff, 512);
}
#endif


