#ifndef __EFUSE_H__
#define __EFUSE_H__

#include "boot.h"
u16 get_chip_id();
u16 get_vbat_trim();
u16 get_vbg_trim();
u32 get_chip_version();
u32 p33_rd_page(u8 page);
u32 get_up_suc_flag(void);
void boot_info_init(void *_info);
extern BOOT_INFO boot_info;
#endif
