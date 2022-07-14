#ifndef _FS_H_
#define _FS_H_

#include "config.h"
#include "fat/ff_opr.h"
#include "fat/fat_resource.h"

#define SEEK_SET	0	/* Seek from beginning of file.  */
#define SEEK_CUR	1	/* Seek from current position.  */
#define SEEK_END	2	/* Seek from end of file.  */

#define F_ATTR_RO       0x01
#define F_ATTR_ARC      0x02
#define F_ATTR_DIR      0x04
#define F_ATTR_VOL      0x08


struct vfs_attr {
    u8 attr;		//属性
    u32 fsize;		//文件大小
    u32 sclust;		//地址
};


enum {
    FS_IOCTL_GET_FILE_NUM,
    FS_IOCTL_FILE_CHECK,
    FS_IOCTL_GET_ERR_CODE,   //暂不支持
    FS_IOCTL_FREE_CACHE,
    FS_IOCTL_SET_NAME_FILTER,  //设置文件过滤
    FS_IOCTL_GET_FOLDER_INFO,  //获取文件夹序号和文件夹内文件数目
    FS_IOCTL_SET_LFN_BUF,   // 512
    FS_IOCTL_SET_LDN_BUF,   // 512

    FS_IOCTL_SET_EXT_TYPE,  //设置后缀类型
    FS_IOCTL_OPEN_DIR,    //打开目录
    FS_IOCTL_ENTER_DIR,   //进入目录
    FS_IOCTL_EXIT_DIR,    //退出
    FS_IOCTL_GET_DIR_INFO,  //获取目录信息

    FS_IOCTL_GETFILE_BYNAME_INDIR, //由歌曲名称获得歌词

    FS_IOCTL_GET_DISP_INFO, //用于长文件名获取

    FS_IOCTL_MK_DIR, //创建文件夹
    FS_IOCTL_GET_ENCFOLDER_INFO, //获取录音文件信息i
    FS_IOCTL_DIR_FILE_TOTAL,
    FS_IOCTL_FILE_TOTAL,
    FS_IOCTL_FS_TOTAL,
    FS_IOCTL_FILE_ATTR,
    FS_IOCTL_FILE_SYNC,
    FS_IOCTL_FILE_INDEX,
    FS_IOCTL_FS_INDEX,
    FS_IOCTL_RESET_VFSCAN,
};



struct vfs_operations {
    const char *fs_type;
    void (*init)(void);
    u32(*mount)(void **ppfs, void *p_device);
    u32(*openbypath)(void *pfs, void **ppfile, const char *path);
    u32(*openbyindex)(void *pfs, void **ppfile, u32 index);
    u32(*openbyfile)(void *pcvfile, void **ppfile, void *ext_name);
    u32(*openbyclust)(void *pfs, void **ppfile, u32 clust);
    u32(*createfile)(void *pfs, void **ppfile, u32 *pindex);
    u32(*read)(void *pfile, void *buff, u32 len);
    u32(*write)(void *pfile, void *buff, u32 len);
    u32(*seek)(void *pfile, u32 offset, u32 mode);
    u32(*close_fs)(void **ppfs);
    u32(*close_file)(void **ppfile);
    u32(*fdelete)(void *);
    int (*fget_attr)(void *, void *attr);
    int (*flen)(void *, u32 *parm);
    int (*ftell)(void *, u32 *parm);
    u32(*name)(void *, void *name, u32 len);
    int (*ioctl)(void *, int cmd, int arg);
    int (*fscan_interrupt)(struct vfscan *, void *, const char *path, u8 max_deepth, int (*callback)(void));
    // int (*fscan)(struct vfscan *, const char *path, u8 max_deepth);
    // int (*fscan)(void *pfs, const char *file_type, u8 max_deepth, u32 clust, u32 *ret_p);
    void (*fscan_release)(struct vfscan *);
    int (*fsel)(struct vfscan *, void *, int sel_mode, void **, int);



    // int (*mount)(struct imount *, int);
    // int (*unmount)(struct imount *);
    // int (*format)(struct vfs_devinfo *, struct vfs_partition *);
    // int (*fset_vol)(struct vfs_partition *, const char *name);
    // int (*fget_free_space)(struct vfs_devinfo *, struct vfs_partition *, u32 *space);
    // int (*fopen)(FILE *, const char *path, const char *mode);
    // int (*fread)(FILE *, void *buf, u32 len);
    // int (*fwrite)(FILE *, void *buf, u32 len);
    // int (*fseek)(FILE *, int offset, int);
    // int (*flen)(FILE *);
    // int (*fpos)(FILE *);
    // int (*fcopy)(FILE *, FILE *);
    // int (*fget_name)(FILE *, u8 *name, int len);
    // int (*frename)(FILE *, const char *path);
    // int (*fclose)(FILE *);
    // int (*fdelete)(FILE *);
    // int (*fscan)(struct vfscan *, const char *path, u8 max_deepth);
    // int (*fscan_interrupt)(struct vfscan *, const char *path, u8 max_deepth, int (*callback)(void));
    // void (*fscan_release)(struct vfscan *);
    // int (*fsel)(struct vfscan *, int sel_mode, FILE *, int);
    // int (*fget_attr)(FILE *, int *attr);
    // int (*fset_attr)(FILE *, int attr);
    // int (*fget_attrs)(FILE *, struct vfs_attr *);
    // int (*fmove)(FILE *file, const char *path_dst, FILE *, int clr_attr);
    // int (*ioctl)(void *, int cmd, int arg);
};

struct imount {
    struct vfs_operations *ops;
    union {
        void *pfs;
        void *pfile;
    };
};

#define REGISTER_VFS_OPERATIONS(ops) \
	const struct vfs_operations ops SEC(.vfs_operations)



struct imount *vfs_hdl_malloc(void);
struct imount *vfs_fhdl_free(struct imount *pvfs);

void vfs_resource_init(void);

void vfs_init(void);
u32 vfs_mount(void **ppvfs, void *device, void *type);
u32 vfs_openbypath(void *pvfs, void **ppvfile, const char *path);
u32 vfs_openbyindex(void *pvfs, void **ppvfile, u32 index);
u32 vfs_openbyfile(void *pcvfile, void **ppvfile, void *ext_name);
u32 vfs_createfile(void *pvfs, void **ppvfile, u32 *pindex);
u32 vfs_read(void *pvfile, void *buf, u32 len);
u32 vfs_write(void *pvfile, void *buf, u32 len);
u32 vfs_seek(void *pvfile, u32 offset, u32 mode);
u32 vfs_file_close(void **ppvfile);
u32 vfs_fs_close(void **ppvfs);
u32 vfs_file_name(void *pvfile, void *name, u32 len);
int vfs_get_attrs(void *pvfile, void *pvfs_attr);
int vfs_ioctl(void *pvfile, int cmd, int arg);

#define VFS_FILE_NAME_LEN			16
extern char g_file_sname[VFS_FILE_NAME_LEN];

#endif
