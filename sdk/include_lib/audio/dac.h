#ifndef __DAC_H__
#define __DAC_H__

#include "typedef.h"
// #include "sound_effect_api.h"

/**********DAC_CON0***************/
#define DAC_CLR_PND  BIT(14)
#define DAC_IE       BIT(13)
#define DAC_FLAG     BIT(12)
// #define DAC_DSMR     BIT(11)
#define DAC_CIC_CLR  BIT(11)
#define DAC_NINV     0//BIT(10) //DAC 反向输出
#define DAC_DEM_EN   BIT(9)
//DAC 扰动，用于DSM模式！！
#define DAC_DT150K   0
#define DAC_DT300K   BIT(8)
//
#define DAC_DSM_EN   BIT(7)
//DSM 模式
#include "dac_cpu.h"
//DAC 调制方式
#define DAC_DSM      DAC_DSM_EN
#define DAC_PWM      BIT(4)

//dac 输出采样率
#define DAC_SR8K     (0<<1)
#define DAC_SR16K    (1<<1)
#define DAC_SR32K    (2<<1)
#define DAC_SR12K    (5<<1)
#define DAC_SR24K    (6<<1)
#define DAC_SRBITS   (7<<1)

#define DAC_ENABLE   BIT(0)


#define DAC_MODE_1_A   ( DAC_PWM )
// 以下DSM模式暂时皆不可用
#define DAC_MODE_3_C   ( DAC_DSM | DAC_DSM2 )
#define DAC_MODE_3_D   ( DAC_DSM | DAC_DSM3 )
#define DAC_MODE_1_B   ( DAC_DSM | DAC_DSM11 )

#define DAC_MODE_5_A   ( DAC_DSM | DAC_DSM7)    //AD150不支持

#ifdef D_APP_MBOX
#define DAC_CURR_MODE  DAC_MODE_5_A
#else
#define DAC_CURR_MODE  DAC_MODE_1_A
#endif



//#define DAC_DEFAULT  ( DAC_IE | DAC_DEM_EN | DAC_DSM_EN | DAC_DSM11 | DAC_PWM | DAC_8K | DAC_ENABLE)
//#define DAC_DEFAULT  ( DAC_IE | DAC_DEM_EN | DAC_DSM_EN | DAC_DSM11 | DAC_PWM | DAC_8K )
// #define DAC_DEFAULT    ( DAC_IE | DAC_DEM_EN | (DAC_CURR_MODE) | DAC_8K )
#define DAC_DEFAULT  ( DAC_IE | DAC_DEM_EN | (DAC_CURR_MODE))
// #define DAC_CON1

/************************************************************************/
#define DAC_CHANNEL_NUMBER 3


/************************************************************************/
typedef enum {
    DAC_SOURCE = 0,
    ALINE_SOURCE,
} AUDIO_TYPE;


void dac_resource_init(u8 *buff, u32 len, u32 con0, u32 con1);
void dac_phy_init(u32 sr_sel);
void dac_phy_off(void);
u32 dac_sr_lookup(u32 sr);
u32 dac_sr_set(u32 sr);
u32 dac_sr_read(void);
void dac_cpu_mode(void);
int dac_low_power(void);
u32 dac_mode_check(u32 con);


#endif
