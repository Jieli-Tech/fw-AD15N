#ifndef __EFUSE_H__
#define __EFUSE_H__

#include "boot.h"


void efuse_init();
u32 get_efuse_page1();
u8 get_wvdd_level_trim();



#if 0
u16 get_chip_id();
u16 get_vbat_trim();
u16 get_vbg_trim();
u32 get_chip_version();
u32 p33_rd_page(u8 page);
#endif
void boot_info_init(void *_info);
extern BOOT_INFO boot_info;
#endif
