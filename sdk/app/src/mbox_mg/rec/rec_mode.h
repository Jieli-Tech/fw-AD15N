#ifndef _REC_MODE_H
#define _REC_MODE_H
#include "key.h"

extern const u16 iokey_msg_mbox_rec_table[3][IO_KEY_MAX_NUM];
extern const u16 adkey_msg_mbox_rec_table[3][AD_KEY_MAX_NUM];
extern const u16 irff00_msg_rec_table[3][IR_KEY_MAX_NUM];


bool enc_file_play(const char *folder);
void enc_play_close(void);
void *rec_find_device(void);
void rec_mode(void);
#endif
