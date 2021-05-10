#ifndef _MUSIC_MIDI_API_H
#define _MUSIC_MIDI_API_H
#include "MIDI_CTRL_API.h"
#include "MIDI_DEC_API.h"

int music_midi_dec_config(music_play_obj *hdl, u32 cmd, void *parm);
void midi_okon_goon(music_play_obj *music_obj);
void midi_okon_mode_set(music_play_obj *music_obj, u8 mode);

#endif
