#ifndef _MIDI_CTRL_API_H
#define _MIDI_CTRL_API_H

#include "decoder_api.h"

u32 midi_ctrl_decode_api(void *p_file, void **ppdec, void *p_dp_buf);
void midi_ctrl_decode_init(void);
u32 midi_ctrl_buff_api(dec_buf *p_dec_buf);

void midi_error_play_end_cb(dec_obj *obj, u32 ret);
u32 midi_ctrl_set_prog(void *work_buf, u8 prog, u8 trk_num);
u32 midi_ctrl_note_on(void *work_buf, u8 nkey, u8 nvel, u8 chn);
u32 midi_ctrl_note_off(void *work_buf, u8 nkey, u8 chn, u16 decay_time);
u32 midi_ctrl_vel_vibrate(void *work_buf, u8 nkey, u8 vel_step, u8 vel_rate, u8 chn);
u32 midi_ctrl_pitch_bend(void *work_buf, u16 pitch_val, u8 chn);
u8 *midi_ctrl_query_play_key(void *work_buf, u8 chn);

#endif
