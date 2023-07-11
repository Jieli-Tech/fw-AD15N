// *INDENT-OFF*
-
-
******************************************************************
** 此文件用于管理不同CPU间有差异的宏，源文件为app_modules_cpu_h.c
******************************************************************
ifndef __APP_MODULES_CPU_H__
define __APP_MODULES_CPU_H__

-
** 驱动使能
#ifdef HAS_EXT_FLASH_EN
define EXT_FLASH_EN  0  -- 外挂资源flash使能
#else
define EXT_FLASH_EN  0  -- 外挂资源flash使能
#endif

#ifdef HAS_SDMMC_EN
define TFG_SD_EN	1  -- SDMMC驱动使能
#else
define TFG_SD_EN	0  -- SDMMC驱动使能
#endif
endif
// *INDENT-ON*
