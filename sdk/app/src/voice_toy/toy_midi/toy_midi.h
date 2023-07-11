#ifndef __TOY_MIDI_H__
#define __TOY_MIDI_H__

#include "config.h"
#include "typedef.h"

extern u16 midi_msg_filter(u8 key_status, u8 key_num, u8 key_type);
extern void sys_idle_deal(u32 usec);
void toy_midi_app(void);
#endif
