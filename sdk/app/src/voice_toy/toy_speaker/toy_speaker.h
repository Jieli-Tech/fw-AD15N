#ifndef _TOY_SPEAKER_H_
#define _TOY_SPEAKER_H_

#include "typedef.h"
#include "key.h"
#include "decoder_mge.h"

void pa_unmute(void);
void pa_mute(void);
void toy_speaker_app(void);
void audio_adc_speaker_start(void);
void audio_adc_speaker_reless(void);
extern u16 toy_speaker_key_msg_filter(u8 key_status, u8 key_num, u8 key_type);

#endif
