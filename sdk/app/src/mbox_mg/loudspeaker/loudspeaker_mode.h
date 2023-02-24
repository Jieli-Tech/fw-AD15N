#ifndef _LOUDSPEAKER_MODE_H
#define _LOUDSPEAKER_MODE_H
#include "key.h"


void loudspeaker_mode(void);
extern const u16 iokey_msg_mbox_LOUDSPEAKER_table[3][IO_KEY_MAX_NUM];
extern const u16 adkey_msg_mbox_LOUDSPEAKER_table[3][AD_KEY_MAX_NUM];
extern const u16 irff00_msg_LOUDSPEAKER_table[3][IR_KEY_MAX_NUM];
extern const char MIC_PGA_G;;
#endif
