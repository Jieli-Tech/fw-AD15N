#include "vfs.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

char g_file_sname[VFS_FILE_NAME_LEN];

#if 0
#define  VFS_MAX_HDL   4 //0~8

typedef struct _vfs_hdl {
    struct imount hdl[VFS_MAX_HDL];
    u8 flag;
} _VFS_HDL;

_VFS_HDL vfs_hdl;

void vfs_resource_init(void)
{
    memset(&vfs_hdl, 0, sizeof(_VFS_HDL));
}

struct imount *vfs_hdl_malloc(void)
{
    for (u8 i = 0; i < VFS_MAX_HDL; i++) {
        if (0 == (BIT(i) & vfs_hdl.flag)) {
            vfs_hdl.flag |= BIT(i);
            memset(&vfs_hdl.hdl[i], 0, sizeof(struct imount));
            return &vfs_hdl.hdl[i];
        }
    }
    log_info("no null vfs hdl\n");
    return (void *) NULL;
}

struct imount *vfs_fhdl_free(struct imount *phdl)
{
    for (u8 i = 0; i < VFS_MAX_HDL; i++) {
        if (phdl == &vfs_hdl.hdl[i]) {
            vfs_hdl.flag &= ~BIT(i);
            return (void *) NULL;
        }
    }
    return phdl;
}
#else
#include "my_malloc.h"

void vfs_resource_init(void)
{
}

struct imount *vfs_hdl_malloc(void)
{
    return my_malloc(sizeof(struct imount), MM_VFS);
}

struct imount *vfs_fhdl_free(struct imount *phdl)
{
    return my_free(phdl);
}

#endif
