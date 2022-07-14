#ifndef _VFS_FAT_H_
#define _VFS_FAT_H_

#include "fat/ff_opr.h"
#include "fat/fat_resource.h"
#include "typedef.h"

// typedef struct ffscan {
//     u32 magic;
//     u8 order;
//     // u8 sort_by;
//     u8 subpath;
//     u8 store_mode;
//     u8 cycle_mode;
//     u16 cur_index;
//     u16 prev_fnum;
//     u16 cur_file_number;
//     u16 first_file_number;
//     u16 last_file_number;
//     u32 first_CrtTime;
//     u32 last_CrtTime;
//     u32 cur_time;
//     FF_APIS ff_api;
// }SCAN;
//

typedef struct vfscan_reset_info {
    // struct vfscan *fsn; //扫描句柄
    u16 file_total; // 当前设备文件总数
    u16 dir_total;// 当前设备文件夹总数
    u8 active; //当前设备是否有效，无效需要扫描。
    u8 scan_over; //当前设备之前是否扫描过，没有需要扫描。
} VFSCAN_RESET_INFO;

u32 vfs_openbyclust(void *pvfs, void **ppvfile, u32 clust);
int vfs_get_fsize(void *pvfile, void *parm);
int vfs_ftell(void *pvfile, void *parm);
u32 vfs_file_delete(void *pvfile);
// int vfs_fscan(void *pvfs, const char *file_type, u8 max_deepth, u32 clust, u32 *ret_p);
struct vfscan *vfs_fscan(void *pvfs, const char *path, const char *arg, u8 max_deepth, int (*callback)(void));
void vfs_fscan_release(void *pvfs, struct vfscan *fs);
int vfs_select(void *pvfs, void **ppvfile, struct vfscan *fs, int sel_mode, int arg);

// int vfs_get_folderinfo(void *pvfile, int *start_num, int *end_num);
int vfs_get_folderinfo(void *pvfile, struct vfscan *fs, int *start_num, int *end_num);
int vfs_mk_dir(void *pvfs, char *folder, u8 mode);
int vfs_get_encfolder_info(void *pvfs, char *folder, char *ext, u32 *last_num, u32 *total_num);

// int vfs_fscan_new(void *pvfs, const char *path, const char *arg, u8 max_deepth, int (*callback)(void), struct vfscan *fsn, struct vfscan_reset_info *info);
struct vfscan *vfs_fscan_new(void *pvfs, const char *path, const char *arg, u8 max_deepth, int (*callback)(void), struct vfscan *fsn, struct vfscan_reset_info *info);

#endif
