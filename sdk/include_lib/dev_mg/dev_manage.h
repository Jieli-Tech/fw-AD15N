#ifndef _DEV_MANAGE_H
#define _DEV_MANAGE_H

void *device_get_dev_hdl(const char *device_name);

int device_status_emit(const char *device_name, const u8 status);
int dev_online_mount(const char *device_name);
int dev_offline_unmount(const char *device_name);
u32 dev_lba_read(void *device, void *buf, u32 lba);
u32 dev_lba_write(void *devive, void *buf, u32 lba);

#endif
