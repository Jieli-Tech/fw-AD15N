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
#include "vm_sfc.h"
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
    for (i = 0; i < MAX_IRQ_ENTRY_NUM; i++) {
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
    u32 flash_id = 0;

    err = vfs_mount(&pvfs, (void *)NULL, (void *) NULL);
    ASSERT(!err, "fii vfs mount : 0x%x\n", err)
    err = vfs_openbypath(pvfs, &pvfile, "/app_area_head/VM");
    ASSERT(!err, "fii vfs openbypath : 0x%x\n", err)
    err = vfs_ioctl(pvfile, FS_IOCTL_FILE_ATTR, (int)&vm_attr);
    ASSERT(!err, "fii vfs ioctl : 0x%x\n", err)
    log_info("file size : 0x%x\nfile sclust : 0x%x\n", vm_attr.fsize, vm_attr.sclust);
    /* log_info("boot info 0x%x\n", boot_info.flash_size); */
    vfs_file_close(&pvfile);
    vfs_fs_close(&pvfs);

    boot_info.vm.vm_saddr = vm_attr.sclust;
    boot_info.vm.vm_size = vm_attr.fsize;

    /* void sdk_sfc_init(void); */
    /* sdk_sfc_init(); */

    device = dev_open(__SFC_NANE, 0);
    dev_ioctl(device, IOCTL_GET_CAPACITY, (u32)&capacity);
    dev_ioctl(device, IOCTL_GET_ID, (u32)&flash_id);
    boot_info.flash_size = capacity;
    log_info("flash id : 0x%x capacity : 0x%x\n", flash_id, boot_info.flash_size);
    dev_ioctl(device, IOCTL_SET_VM_INFO, (u32)&boot_info);
    dev_ioctl(device, IOCTL_SET_PROTECT_INFO, (u32)flash_code_protect_callback);
    dev_close(device);

    //该函数位置必须放在flash_info_init()之后，且isd_config.ini文件中VM区域大小不可为0
    norflash_set_write_protect(1);
    return 0;
}
struct vfs_attr *get_vm_attr_p(void)
{
    return &vm_attr;
}

AT_SPI_CODE/*该函数放置段不可更改*/
u32 flash_code_protect_callback(u32 offset, u32 len)
{
    u32 limit_addr = vm_attr.sclust;
    /* log_info("0x%x 0x%x", limit_addr, offset); */
    if ((offset < limit_addr) || ((offset + len) > boot_info.flash_size)) {
        /* 超过正常擦写区域，不进行擦写操作 */
        return 1;
    } else {
        /* 进行擦写操作 */
        return 0;
    }
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
    /* u8 data_buf[10]; */

    /* for (int i = 0; i < sizeof(data_buf); i++) { */
    /* data_buf[i] = JL_RAND->R64L; */
    /* } */

    /* put_buf(data_buf, 10); */
    /* vm_write(VM_INDEX_DEMO, data_buf, 10); */

    /* memset(data_buf, 0x00, 10); */

    /* vm_read(VM_INDEX_DEMO, data_buf, 10); */

    /* put_buf(data_buf, 10); */

//   while (1);
}
struct vfs_attr *get_eeprom_attr_p(void)
{
    return &eeprom_attr;
}

sec_used(.version)
u8 const lib_update_version[] = "\x7c\x4f\x94\x0aUPDATE-@20210816-$9c89ae0";

void test_fs(void);
void test_audio_dac(void);
void clock_2_64m(void);
void system_init(void)
{
    /* clock_2_64m(); */
    my_malloc_init();
    message_init();

    adc_init();
    app_power_init();
    key_init();
    tick_timer_init();

    vfs_resource_init();
    vfs_init();

    devices_init();
    flash_info_init();
    vm_init_api();
#if defined(NORFS_EN) && (NORFS_EN)
    norfs_init_api();
#endif

    d_key_voice_init();

    audio_init();
    dac_mode_init(16, D_PHY_VOL_SET_FUNC);
    dac_init_api(SR_DEFAULT, 1);

    d_mio_module_init();
}
#if 0
static void *t_pvfs = 0;
static void *t_pfile = 0;
static u8 t_buf[32];
void test_fs(void)
{
    u32 err;
    log_info("****************************************\n");
    err = vfs_mount(&t_pvfs, (void *)NULL, (void *)NULL);
    if (0 != err) {
        log_info(" err vfs mount 0x%x\n", err);
    }
    log_info("A 0x%x\n", (u32)t_pvfs);
    err = vfs_openbypath(t_pvfs, &t_pfile, "/dir_bin_01");
    if (0 != err) {
        log_info(" err openbypath 0x%x\n", err);
    }
    log_info("B\n");
    err = vfs_openbyindex(t_pvfs, &t_pfile, 1);//SLEEP.lrc
    if (0 != err) {
        log_info(" err openbyindex 0x%x\n", err);
    }
    log_info("C\n");
    u32 len;
    len = vfs_read(t_pfile, t_buf, sizeof(t_buf));
    if (0 == len) {
        log_info("no data\n");
    }
    log_info_hexdump(t_buf, sizeof(t_buf));

}
#endif



