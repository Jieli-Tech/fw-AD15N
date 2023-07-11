#ifndef __MIDI_CONFIG_H__
#define __MIDI_CONFIG_H__

#include "config.h"
#include "decoder_api.h"
#include "MIDI_DEC_API.h"

int midi_dec_config(dec_obj *p_dec_obj, u32 cmd, void *parm);
void midi_mode_set(dec_obj *p_dec_obj, u8 mode);
void midi_okon_goon(dec_obj *p_dec_obj);
void midi_init_info(MIDI_INIT_STRUCT *init_info);
#endif
