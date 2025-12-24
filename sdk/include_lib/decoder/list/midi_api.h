#ifndef __MIDI_API_H__
#define __MIDI_API_H__
#include "if_decoder_ctrl.h"
#include "MIDI_DEC_API.h"

#ifndef MIDI_DEC_MAX_KEY
#define MIDI_DEC_MAX_KEY    8
#define MIDI_DEC_BUF_SIZE   (4436 + 3)
#endif

MIDI_PLAY_CTRL_MODE *get_midi_mode(void);
u32 *get_midi_switch_info(void);
u32 midi_decode_api(void *p_file, void **ppdec, void *p_dp_buf);
void midi_decode_init(void);
u32 midi_buff_api(dec_buf *p_dec_buf);

#endif


