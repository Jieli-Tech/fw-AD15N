#ifndef __A_ADC_H__
#define __A_ADC_H__

#include "typedef.h"
#include "audio_analog.h"

#if AUDIO_ADC_EN
void audio_adc_mode_init(void);
void audio_adc_isr(void);
u32 audio_adc_init_api(u32 sr, ADC_MODE mode, u32 ch);
void audio_adc_off_api(void);
#else
#define audio_adc_mode_init()
#define audio_adc_isr()
#define audio_adc_init_api(...)
#define audio_adc_off_api()
#endif

void fill_audio_adc_fill(u8 *buf, u32 len);
bool regist_audio_adc_channel(void *psound, void *kick);
bool unregist_audio_adc_channel(void *psound);
u32 read_audio_adc_sr(void);
void set_audio_adc_sr(u32 sr);
void audio_adc_resource_init(u8 *buf, u32 buf_len, u32 cnt);
u32 audio_adc_phy_init(u32 sr);
void audio_adc_phy_off(void);

void audio_adc_enable(u32 gain);
void audio_adc_disable(void);


#endif
