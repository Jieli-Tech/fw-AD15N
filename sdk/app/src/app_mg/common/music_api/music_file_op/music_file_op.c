#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "hwi.h"
#include "dev_manage.h"
#include "vfs.h"
#include "music_file_op/music_file_op.h"
#include "errno-base.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

u32 musci_file_open_bypath(void **ppvfs, void **ppvfile, const char *path, void *device, void *type)
{
    u32 err = 0;
    err = vfs_mount(ppvfs, device, (void *)type);
    if (err != 0) {
        return err;
    }
    err = vfs_openbypath(*ppvfs, ppvfile, path);
    if (err !=  0) {
        vfs_fs_close(ppvfs);
        return err;
    }
    return err;
}

u32 musci_file_open_bydirindex(void **ppvfs, void **ppvfile, const char *path, u32 *index, void *device, void *type)
{
    u32 err = 0;
    err = vfs_mount(ppvfs, device, (void *)type);
    if (err != 0) {
        log_error("vfs mount error !!!\n");
        return err;
    }
    if (path != NULL) {
        err = vfs_openbypath(*ppvfs, ppvfile, path);
        if (err !=  0) {
            vfs_fs_close(ppvfs);
            return err;
        }
    }

_find_next_file:
    if (*index) {
        err = vfs_openbyindex(*ppvfs, ppvfile, *index);
        if (err !=  0) {
            log_error("vfs open file error !!!\n");
            vfs_file_close(ppvfile);
            vfs_fs_close(ppvfs);
            return err;
        }

        int total = 0;
        char name[VFS_FILE_NAME_LEN] = {0};
        vfs_file_name(*ppvfile, (void *)name, sizeof(name));
        if (strstr(name, ".mio") != NULL) {
            vfs_ioctl(*ppvfile, FS_IOCTL_DIR_FILE_TOTAL, (int)(&total));
            (++(*index) > total) ? ((*index) = 1) : *index;
            goto _find_next_file;
        }
    }

    return err;
}

u32 music_file_reopen_byindex(void **ppvfs, void **ppvfile, u32 *index, u8 dir)
{
    if ((*ppvfs == NULL) || (*ppvfile == NULL)) {
        return E_NO_FS;
    }

_find_next_file:
    u32 err = vfs_openbyindex(*ppvfs, ppvfile, *index);

    int total = 0;
    char name[VFS_FILE_NAME_LEN] = {0};
    vfs_file_name(*ppvfile, (void *)name, sizeof(name));
    if (strstr(name, ".mio") != NULL) {
        vfs_ioctl(*ppvfile, FS_IOCTL_DIR_FILE_TOTAL, (int)(&total));
        if (dir == 0) {
            (++(*index) > total) ? (*index = 1) : *index;
        } else {
            if (*index > 1) {
                (*index)--;
            } else {
                (*index) = total;
            }
        }
        goto _find_next_file;
    }

    return err;
}

u32 music_file_reopen_bypath(void **ppvfs, void **ppvfile, const char *path)
{
    if ((*ppvfs == NULL) || (*ppvfile == NULL)) {
        return E_NO_FS;
    }

    u32 err = vfs_openbypath(*ppvfs, ppvfile, path);

    return err;
}

u32 music_file_get_total(void *pvfile)
{
    int total = 0;
    int ret = vfs_ioctl(pvfile, FS_IOCTL_DIR_FILE_TOTAL, (int)(&total));
    if (ret < 0) {
        return 0;
    }

    return (u32)total ;
}

u32 music_file_name(void *pvfile, void *name, u32 len)
{
    if (pvfile == NULL) {
        return 0;
    }

    return vfs_file_name(pvfile, (void *)name, len);
}

void music_file_close(void **ppvfs, void **ppvfile)
{
    if (ppvfile !=  NULL) {
        vfs_file_close(ppvfile);
    }

    if (ppvfs != NULL) {
        vfs_fs_close(ppvfs);
    }
}

