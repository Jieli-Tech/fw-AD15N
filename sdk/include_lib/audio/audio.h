#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "typedef.h"


void audio_init(void);
void audio_off(void);
void audio_lookup(void);
void dac_power_off();
void dac_power_on(u32 sr, bool delay_flag);

bool ladc_capless_init(u32 delay);
#endif
