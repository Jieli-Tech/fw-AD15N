#ifndef _KEY_DRV_AD_H
#define _KEY_DRV_AD_H
#include "adc_drv.h"
#include "gpio.h"
#include "key.h"


#define AD_KEY_CH 		AD_KEY_CH_SEL
#define EXTERN_R_UP		100//220 -> 22k,外挂上拉电阻,0使用内部上拉,内部上拉为10k

extern const key_interface_t key_ad_info;

#endif
