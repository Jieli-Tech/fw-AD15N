#include "config.h"
#include "common.h"
#include "app_config.h"
#include "device.h"
#include "norflash.h"
#include "spi1.h"
#include "sdmmc/sd_host_api.h"
#include "usb/otg.h"
#include "app_config.h"

#define LOG_TAG_CONST       MAIN
#define LOG_TAG             "[dev list]"
#include "debug.h"

// *INDENT-OFF*
//sh54: SPI1:  CLK       ,  DO        ,  DI
//SPI1: A组IO: IO_PORT_DP,  IO_PORT_DM,  IO_PORTA_03,
//SPI1: B组IO: IO_PORTA_11, IO_PORTA_12, IO_PORTA_10,
//SPI1: C组IO: IO_PORTA_04, IO_PORTA_05, IO_PORTA_06
//sh55: SPI1:  CLK       ,  DO        ,  DI
//SPI1: A组IO: IO_PORTB_00, IO_PORTB_01, IO_PORTB_02,
//SPI1: B组IO: IO_PORTA_14, IO_PORTA_15, IO_PORTA_13,
//SPI1: C组IO: IO_PORTA_06, IO_PORTA_07, IO_PORTA_08,
//SPI1: D组IO: IO_PORTB_08, IO_PORTB_09, IO_PORTB_07
#if (EXT_FLASH_EN)
const struct spi_platform_data spi1_p_data = {
    .port = {
         SPI1_GROUPB_IO
    },
    .mode = SPI_MODE_BIDIR_1BIT,
    .clk = 10000000,
    .role = SPI_ROLE_MASTER,
};
//norflash
NORFLASH_DEV_PLATFORM_DATA_BEGIN(norflash_data)
.spi_hw_num = 1,
.spi_cs_port = IO_PORTA_05,
.spi_read_width = SPI_MODE_BIDIR_1BIT,
.spi_pdata = &spi1_p_data,
.start_addr = 0,
.size = 2 * 1024 * 1024,
NORFLASH_DEV_PLATFORM_DATA_END()
#endif

//SH54:
//SD A组IO:   CMD:PA02    CLK:PA01    DAT:PA03
//SD B组IO:   CMD:PA12    CLK:PA11    DAT:PA10
//SD C组IO:   CMD:PA05    CLK:PA04    DAT:USBDM
//SD D组IO:   CMD:PA05    CLK:PA04    DAT:PA06
//SH55:
//SD A组IO:   CMD:PA02    CLK:PA01    DAT:PA03
//SD B组IO:   CMD:PB01    CLK:PB00    DAT:PB02
//SD C组IO:   CMD:PB05    CLK:PB04    DAT:PB06
//SD D组IO:   CMD:PA07    CLK:PA06    DAT:PA08
#if TFG_SD_EN
SD0_PLATFORM_DATA_BEGIN(sd0_data)
  .port                   = 'A',
  .data_width             = 1,
  .speed                  = 12000000,
#if 0 //CMD检测
  .detect_mode            = SD_CMD_DECT,
  .detect_func            = sdmmc_0_cmd_detect,
#endif
#if 1 //CLK检测
  .detect_mode            = SD_CLK_DECT,
  .detect_func            = sdmmc_0_clk_detect,
  .detect_io_level        = 0,//0:低电平检测到卡  1:高电平检测到卡
#endif
#if 0 //IO检测
  .detect_mode            = SD_IO_DECT,
  .detect_func            = sdmmc_0_io_detect,
  .detect_io              = IO_PORTx_xx,//用于检测的引脚
  .detect_io_level        = x,//0:低电平检测到卡  1:高电平检测到卡
#endif
  .power                  = NULL,
  .priority               = 3,
SD0_PLATFORM_DATA_END()

#endif

/************************** otg data****************************/
#if TCFG_OTG_MODE
struct otg_dev_data otg_data = {
    .usb_dev_en = TCFG_OTG_USB_DEV_EN,
	.slave_online_cnt = TCFG_OTG_SLAVE_ONLINE_CNT,
	.slave_offline_cnt = TCFG_OTG_SLAVE_OFFLINE_CNT,
	.host_online_cnt = TCFG_OTG_HOST_ONLINE_CNT,
	.host_offline_cnt = TCFG_OTG_HOST_OFFLINE_CNT,
	.detect_mode = TCFG_OTG_MODE,
	.detect_time_interval = TCFG_OTG_DET_INTERVAL,
};
#endif
extern const struct device_operations mass_storage_ops;
REGISTER_DEVICES(device_table) = {
#if (EXT_FLASH_EN)
#if TCFG_USB_EXFLASH_UDISK_ENABLE
    {.name = __EXT_FLASH_NANE, .ops = &norflash_dev_ops, .priv_data = (void *) &norflash_data},
#else
    {.name = __EXT_FLASH_NANE, .ops = &norfs_dev_ops, .priv_data = (void *) &norflash_data},
#endif
#endif
    {.name = __SFC_NANE, .ops = &sfc_dev_ops, .priv_data = (void *)NULL},
#if TFG_SD_EN
    {.name = __SD0_NANE, .ops = &sd_dev_ops, .priv_data = (void *)&sd0_data},
#endif
#if TCFG_OTG_MODE
    { .name = __OTG,     .ops=&usb_dev_ops, .priv_data = (void *) &otg_data},
#endif
#if TCFG_UDISK_ENABLE
    { .name = __UDISK0,     .ops=&mass_storage_ops, .priv_data = (void *)NULL},
#endif
};

