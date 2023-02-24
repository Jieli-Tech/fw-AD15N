/* #include "system/includes.h" */
#include "usb/device/msd.h"
#include "usb/scsi.h"
#include "usb_config.h"
#include "cpu.h"

#define LOG_TAG_CONST       MUGRD
#define LOG_TAG             "[mugrd]"
#include "log.h"

#if TCFG_PC_ENABLE

#if TCFG_PC_UPDATE

typedef enum {
    UPGRADE_NULL = 0,
    UPGRADE_USB_HARD_KEY,
    UPGRADE_USB_SOFT_KEY,
    UPGRADE_UART_KEY,
} UPGRADE_STATE;
void chip_reset();
void nvram_set_boot_state(u32 state);
void hw_mmu_disable(void);
extern u32 msd_usb2mcu(const struct usb_device_t *usb_device, u8 *buffer, u32 len);
extern u32 msd_mcu2usb(const struct usb_device_t *usb_device, const u8 *buffer, u32 len);
extern u32 cdrom_read_data(u8 *buf, u32 addr, u32 cnt);
AT(.volatile_ram_code)
static void go_mask_usb_updata()
{
    local_irq_disable();
    /* ram_protect_close(); */
    /* hw_mmu_disable(); */
    nvram_set_boot_state(UPGRADE_USB_SOFT_KEY);

    /* JL_CLOCK->PWR_CON |= (1 << 4); */
    chip_reset();
    /* cpu_reset(); */
    while (1);
}


s32 scsi_flash_operations(const struct usb_device_t *usb_device, const struct usb_scsi_cbw *cbw);
/* 私有scsi命令接口  */
u32 private_scsi_cmd(const struct usb_device_t *usb_device, struct usb_scsi_cbw *cbw)
{
    void *fd = NULL;
    u32 lba;
    u8 ack[16];
    switch (cbw->operationCode) {

#if TCFG_USB_MSD_CDROM_ENABLE
    // FLASH操作接口
    case 0xF4:
        //log_info("res_upgrade\n");
        scsi_flash_operations(usb_device, cbw);
        break;
#endif
    case 0xFC:
        go_mask_usb_updata();
        break;

    default:
        return FALSE;
    }

    return TRUE;
}
#else
u32 private_scsi_cmd(const struct usb_device_t *usb_device, struct usb_scsi_cbw *cbw)
{
    return FALSE;
}
#endif //PC_UPDATE_ENABLE

#endif

