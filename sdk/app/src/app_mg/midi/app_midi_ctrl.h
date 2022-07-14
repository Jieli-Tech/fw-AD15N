#ifndef __APP_MIDI_CTRL_H__
#define __APP_MIDI_CTRL_H__
#include "typedef.h"
#include "key.h"

extern const u16 iokey_msg_midi_keyboard_table[][AD_KEY_MAX_NUM];
extern const u16 adkey_msg_midi_keyboard_table[][AD_KEY_MAX_NUM];
int app_midi_keyboard(void *param);
#endif
