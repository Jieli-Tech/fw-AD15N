#ifndef _APP_POWER_MG_H
#define _APP_POWER_MG_H
#include "typedef.h"

#define LVD_WARNING_FOR_LOW_POWER   1//1:使用LVD警告电压作低电提醒 0:使用固定值作低电提醒

#if LVD_WARNING_FOR_LOW_POWER
#define LOW_POWER_VOL		    lvd_warning_voltage
#define LOW_POWER_LOG           "Warning!!! Vbat is near to lvd!\n"
#define low_power_warning_init  lvd_warning_init
#else
#define LOW_POWER_VOL		    3300//3.3v
#define LOW_POWER_LOG           "low power\n"
#define low_power_warning_init
#endif

void app_power_init(void);
void app_power_scan(void);
u32 app_power_get_vbat(void);
#endif
