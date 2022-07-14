#ifndef __DAC_H__
#define __DAC_H__

#include "typedef.h"
#include "dac_cpu.h"
// #include "sound_effect_api.h"





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
