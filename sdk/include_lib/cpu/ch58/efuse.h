#ifndef __EFUSE_H__
#define __EFUSE_H__

#include "boot.h"

#if 0
u16 get_chip_id();
u16 get_vbat_trim();
u16 get_vbg_trim();
u32 get_chip_version();
u32 p33_rd_page(u8 page);
#endif
void boot_info_init(void *_info);
extern BOOT_INFO boot_info;

u32 efuse_get_wvdd_trim(void);

u32 efuse_get_sfc_fast_boot(void);
#endif
