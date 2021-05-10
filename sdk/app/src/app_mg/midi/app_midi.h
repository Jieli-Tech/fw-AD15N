#ifndef _APP_MIDI_H
#define _APP_MIDI_H

extern const u16 iokey_msg_midi_table[3][IO_KEY_MAX_NUM];
extern const u16 adkey_msg_midi_table[3][AD_KEY_MAX_NUM];
int app_midi(void *param);
#endif
