#include "device.h"
#include "cpu.h"
#include "boot.h"
#include "crc16.h"
#include "reserved_area.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#define FLASH_ADDR2CPU_ADDR(flash_addr) \
	(boot_info.sfc.app_addr + flash_addr - boot_info.sfc.sfc_base_addr)

static u32 rsv_flash_addr_get(void)
{
    void *device = NULL;
    u32 capacity = 0;
    device = dev_open(__SFC_NANE, 0);
    dev_ioctl(device, IOCTL_GET_CAPACITY, (u32)&capacity);
    return (capacity - RESERVED_AREA_SIZE);
}

/*****************auth*******************/
/*
 *start addr:reserved area last 256 byte
  data start:start addr + 80 byte
 * */

void rsv_auth_analysis(void)
{
    auth_header_t *auth = (auth_header_t *)FLASH_ADDR2CPU_ADDR(rsv_flash_addr_get() + RESERVED_AREA_SIZE - 256 + 80);
    u8 *crc_data = (u8 *)(&auth->data_len);
    u32 crc_len = auth->data_len + sizeof(auth->data_len);
    u16 crc16 = CRC16(crc_data, crc_len);
    if (crc16 != auth->crc16) {
        log_error("no auth data !!!\n ");
        return;
    }

    log_info("auth data:");
    log_info_hexdump((u8 *)auth->data_p, auth->data_len);
}
