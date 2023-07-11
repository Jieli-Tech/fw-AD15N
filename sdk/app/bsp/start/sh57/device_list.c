#include "config.h"
#include "common.h"
#include "app_config.h"
#include "device.h"
#include "norflash.h"
#include "spi1.h"
/* #include "sdmmc/sd_host_api.h" */
#include "app_config.h"

#define LOG_TAG_CONST       MAIN
#define LOG_TAG             "[dev list]"
#include "log.h"

// *INDENT-OFF*
//sh57: SPI1:  CLK,  DO,  DI  crossbar
#if (EXT_FLASH_EN)
const struct spi_platform_data spi1_p_data = {
    .port = {
        IO_PORTA_01,    //CLK
        IO_PORTA_02,    //DO
        IO_PORTA_03,    //DI
    },
    .mode = HW_SPI_WORK_MODE,
    .clk = 12000000,
    .role = SPI_ROLE_MASTER,
};

//norflash
NORFLASH_DEV_PLATFORM_DATA_BEGIN(norflash_data)
.spi_hw_num = SPI_HW_NUM,
.spi_cs_port = SPI_CS_PORT_SEL,
.spi_read_width = SPI_READ_DATA_WIDTH,
.spi_pdata = &spi1_p_data,
.start_addr = 0,
.size = 2 * 1024 * 1024,
NORFLASH_DEV_PLATFORM_DATA_END()
#endif


/************************** otg data****************************/
extern const struct device_operations mass_storage_ops;
REGISTER_DEVICES(device_table) = {
#if (EXT_FLASH_EN)
    {.name = __EXT_FLASH_NANE, .ops = &norfs_dev_ops, .priv_data = (void *) &norflash_data},
#endif
    {.name = __SFC_NANE, .ops = &sfc_dev_ops, .priv_data = (void *)NULL},
};

