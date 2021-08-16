#ifndef _SD_HOST_API_H_
#define _SD_HOST_API_H_

#include "typedef.h"
#include "sfr.h"

#define     SD_CMD_DECT     0
#define     SD_CLK_DECT     1
#define     SD_IO_DECT      2


struct sdmmc_platform_data {
    char port;
    u8 irq;
    u8 data_width;
    u8 priority;
    u8 detect_mode;
    u8 detect_io;
    u8 detect_io_level;
    u8 detect_time_interval;
    u32 detect_timeout;
    u32 speed;
    JL_SD_TypeDef *sfr;
    int (*detect_func)(const struct sdmmc_platform_data *);
    void (*port_init)(const struct sdmmc_platform_data *, int mode);
    void (*power)(int on);
};

extern const struct device_operations sd_dev_ops;
extern void sd0_dev_detect(void *p);

extern void sd_set_power(u8 enable);
extern u8 sd_io_suspend(u8 sdx, u8 sd_io);
extern u8 sd_io_resume(u8 sdx, u8 sd_io);
extern u32 sdx_dev_send_suspend_event(u8 event_idx);

extern void sdmmc_0_port_init(const struct sdmmc_platform_data *, int mode);

extern int sdmmc_0_clk_detect(const struct sdmmc_platform_data *);
extern int sdmmc_0_io_detect(const struct sdmmc_platform_data *);
extern int sdmmc_0_cmd_detect(const struct sdmmc_platform_data *);

#define SD0_PLATFORM_DATA_BEGIN(data) \
	static const struct sdmmc_platform_data data = {

#define SD0_PLATFORM_DATA_END() \
	.irq 					= IRQ_SD0_IDX, \
    .sfr                    = JL_SD0, \
	.port_init 				= sdmmc_0_port_init, \
	.detect_time_interval 	= 250, \
	.detect_timeout     	= 2000, \
};

#endif

