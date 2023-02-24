#ifndef _MUSIC_MIDI_CTRL_API_H
#define _MUSIC_MIDI_CTRL_API_H
#include "music_api.h"

int music_midi_ctrl_pitch_bend(music_play_obj *hdl, u16 pitch_val, u8 chn);
int music_midi_ctrl_note_off(music_play_obj *hdl, u8 nkey, u8 chn, u16 decay_time);
int music_midi_ctrl_note_on(music_play_obj *hdl, u8 nkey, u8 nvel, u8 chn);
int music_midi_ctrl_vel_vibrate(music_play_obj *hdl, u8 nkey, u8 vel_step, u8 vel_rate, u8 chn);
int music_midi_ctrl_set_prog(music_play_obj *hdl, u8 prog, u8 trk_num);
int music_midi_ctrl_config(music_play_obj *hdl, u32 cmd, void *parm);
void midi_ctrl_play_end_deal(music_play_obj *hdl);
void midi_ctrl_callback_init(music_play_obj *hdl, u32(*melody_callback)(void *, u8, u8), u32(*melody_stop_callback)(void *, u8));
void get_midi_ctrl_play_key(music_play_obj *hdl, u8 chn);

#endif
