#include "config.h"
#include "common.h"
#include "tick_timer_driver.h"
#include "boot.h"
#include "msg.h"
#include "audio.h"
#include "dac.h"
#include "dac_api.h"
/* #include "audio_adc.h" */
#include "vfs.h"
/* #include "midi_api.h" */
#include "src_api.h"
#include "device.h"
#include "ioctl_cmds.h"
#include "vm_api.h"
#include "nor_fs/nor_fs.h"
#include "key.h"
#include "init.h"
#include "adc_drv.h"
#include "clock.h"
#include "app_power_mg.h"
/* #include "efuse.h" */
#include "my_malloc.h"
#include "mio_api.h"
#include "sine_play.h"
#include "flash_wp.h"
/* #include "list/midi_ctrl_api.h" */
/* #include "efuse_trim_value.h" */

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"



extern void exception_irq_handler(void);
void all_init_isr(void)
{
    u32 i;
    unsigned int *israddr = (unsigned int *)IRQ_MEM_ADDR;
    for (i = 0; i < 32; i++) {
        israddr[i] = (u32)exception_irq_handler;
    }
}


static struct vfs_attr vm_attr;
int flash_info_init(void)
{
    u32 err;
    void *pvfs = 0;
    void *pvfile = 0;
    void *device = 0;
    u32 capacity = 0;

    err = vfs_mount(&pvfs, (void *)NULL, (void *) NULL);
    ASSERT(!err, "fii vfs mount : 0x%x\n", err)
    err = vfs_openbypath(pvfs, &pvfile, "/app_area_head/VM");
    ASSERT(!err, "fii vfs openbypath : 0x%x\n", err)
    err = vfs_ioctl(pvfile, FS_IOCTL_FILE_ATTR, (int)&vm_attr);
    ASSERT(!err, "fii vfs ioctl : 0x%x\n", err)
    log_info("file size : 0x%x\nfile sclust : 0x%x\n", vm_attr.fsize, vm_attr.sclust);
    log_info("boot info 0x%x\n", boot_info.flash_size);
    vfs_file_close(&pvfile);
    vfs_fs_close(&pvfs);

    boot_info.vm.vm_saddr = vm_attr.sclust;
    boot_info.vm.vm_size = vm_attr.fsize;

    void sdk_sfc_init(void);
    sdk_sfc_init();

    device = dev_open(__SFC_NANE, 0);
    dev_ioctl(device, IOCTL_GET_CAPACITY, (u32)&capacity);
    boot_info.flash_size = capacity;
    dev_close(device);

    norflash_set_write_protect(1);
    return 0;
}
struct vfs_attr *get_vm_attr_p(void)
{
    return &vm_attr;
}


static struct vfs_attr eeprom_attr;
void vm_init_api(void)
{
    u32 err;
    void *pvfs = 0;
    void *pvfile = 0;
    void *device = 0;
    u32 capacity = 0;


    err = vfs_mount(&pvfs, (void *)NULL, (void *)NULL);
    ASSERT(!err, "fii vfs mount : 0x%x\n", err)
    err = vfs_openbypath(pvfs, &pvfile, "/app_area_head/EEPROM");
    ASSERT(!err, "fii vfs openbypath : 0x%x\n", err)
    err = vfs_ioctl(pvfile, FS_IOCTL_FILE_ATTR, (int)&eeprom_attr);
    ASSERT(!err, "fii vfs ioctl : 0x%x\n", err)
    log_info("EEPROM size : 0x%x\nEEPROM sclust : 0x%x\n", eeprom_attr.fsize, eeprom_attr.sclust);
    vfs_file_close(&pvfile);
    vfs_fs_close(&pvfs);

    syscfg_vm_init(eeprom_attr.sclust, eeprom_attr.fsize);
    /* *demo */
#if 0
    u8 data_buf[10];
    for (int i = 0; i < sizeof(data_buf); i++) {
        data_buf[i] = JL_RAND->R64L;
    }
    put_buf(data_buf, 10);
    vm_write(VM_INDEX_DEMO, data_buf, 10);
    memset(data_buf, 0x00, 10);
    vm_read(VM_INDEX_DEMO, data_buf, 10);
    put_buf(data_buf, 10);
#endif
}
struct vfs_attr *get_eeprom_attr_p(void)
{
    return &eeprom_attr;
}

void test_audio_dac(void);
void system_init(void)
{
    my_malloc_init();

    pll_sel(PLL_192M, PLL_DIV2, PLL_B_DIV1);
    d_key_voice_init();
    d_mio_module_init();
    message_init();

    adc_init();
    app_power_init();
    key_init();

    tick_timer_init();
    devices_init();
    vfs_resource_init();
    vfs_init();

    flash_info_init();
    vm_init_api();

    dac_mode_init(16);
    dac_power_on(SR_DEFAULT, 1);
    test_audio_dac();
}



