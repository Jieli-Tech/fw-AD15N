#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "typedef.h"
#include "atomic.h"
#include "ioctl_cmds.h"
#include "dev_manage.h"

#define __SFC_NANE  		"sfc"
#define __SD0_NANE  		"sd0"
#define __EXT_FLASH_NANE  	"ext_flsh"
#define __OTG				"otg"
#define __UDISK0			"udisk0"

struct dev_node;
struct device;

struct device_operations {
    bool (*online)(const struct dev_node *node);
    int (*init)(const struct dev_node *node, void *);
    int (*open)(const char *name, struct device **device, void *arg);
    int (*read)(struct device *device, void *buf, u32 len, u32);
    int (*write)(struct device *device, void *buf, u32 len, u32);
    int (*bulk_read)(struct device *device, void *buf, u32 len, u32);
    int (*bulk_write)(struct device *device, void *buf, u32 len, u32);
    int (*seek)(struct device *device, u32 offset, int orig);
    int (*ioctl)(struct device *device, u32 cmd, u32 arg);
    int (*close)(struct device *device);
};

struct dev_node {
    const char *name;
    const struct device_operations *ops;
    void *priv_data;
};

struct device {
    atomic_t ref;
    void *private_data;
    const struct device_operations *ops;
    void *platform_data;
    void *driver_data;
};

#define REGISTER_DEVICE(node) \
    const struct dev_node node sec_used(.device)

#define REGISTER_DEVICES(node) \
    const struct dev_node node[] sec_used(.device)

int devices_init();
bool dev_online(const char *name);
void *dev_open(const char *name, void *arg);
// int dev_read(void *device, void *buf, u32 len);
// int dev_write(void *device, void *buf, u32 len);
// int dev_seek(void *device, u32 offset, int orig);
int dev_ioctl(void *device, int cmd, u32 arg);
int dev_close(void *device);
int dev_byte_read(void *_device, void *buf, u32 offset, u32 len);
int dev_byte_write(void *_device, void *buf, u32 offset, u32 len);
int dev_bulk_read(void *_device, void *buf, u32 sector, u32 sector_num);
int dev_bulk_write(void *_device, void *buf, u32 sector, u32 sector_num);

extern const struct device_operations sfc_dev_ops;
#endif


