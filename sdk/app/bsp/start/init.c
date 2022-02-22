
#include "config.h"
#include "common.h"
#include "boot.h"
#include "tick_timer_driver.h"
#include "audio.h"
#include "msg.h"
#include "dac.h"
#include "dac_api.h"
#include "audio_adc.h"
#include "vfs.h"
#include "midi_api.h"
#include "src_api.h"
#include "device.h"
#include "ioctl_cmds.h"
#include "vm.h"
#include "nor_fs/nor_fs.h"
#include "key.h"
#include "init.h"
#include "adc_drv.h"
#include "clock.h"
#include "app_power_mg.h"
#include "efuse.h"
#include "my_malloc.h"
#include "mio_api.h"
#include "sine_play.h"
#include "list/midi_ctrl_api.h"
#include "efuse_trim_value.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"



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

    device = dev_open(__SFC_NANE, 0);
    dev_ioctl(device, IOCTL_GET_CAPACITY, (u32)&capacity);
    dev_ioctl(device, IOCTL_SET_VM_INFO, (u32)&boot_info);
    dev_close(device);

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
    /*
     *demo
    u8 data_buf[10];
    memset(data_buf,0xaa,10);
    vm_write(VM_INDEX_DEMO, data_buf, 10);
    memset(data_buf,0x00,10);
    vm_read(VM_INDEX_DEMO, data_buf, 10);
    put_buf(data_buf,10);
    */
}
struct vfs_attr *get_eeprom_attr_p(void)
{
    return &eeprom_attr;
}

sec_used(.version)
u8 const lib_update_version[] = "177491836UPDATE-@20210816-$9c89ae0";

void system_init(void)
{
    my_malloc_init();
    pll_sel(PLL_320M, PLL_DIV2, PLL_B_DIV2);
    efuse_trim_value_init();
    /* sys_clock_get(); */
    //--------------------------
    d_key_voice_init();
    message_init();
    adc_init();
    app_power_init();
    key_init();

    //------
    tick_timer_init();
    emu_init();

    //-------------------------------
    vfs_resource_init();
    vfs_init();
#if DECODER_MIDI_EN
    midi_decode_init();
    midi_ctrl_decode_init();
#endif

    dac_mode_init(16);
    audio_adc_mode_init();
    audio_init();

    //--src----------
    src_mode_init();
    //----------------------------
    dac_init_api(32000);
    /* device */
    devices_init();
    flash_info_init();
    vm_init_api();
    norfs_init_api();
    //----------------------
    d_mio_module_init();
    /* dac_vol(0, 16); */
}

