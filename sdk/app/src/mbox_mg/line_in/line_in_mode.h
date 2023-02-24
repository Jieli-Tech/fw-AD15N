#ifndef _LINE_IN_MODE_H
#define _LINE_IN_MODE_H
#include "key.h"

void Line_in_mode(void);
void line_in_det(void);
u8 line_in_online(void);
extern const u16 iokey_msg_mbox_line_in_table[3][IO_KEY_MAX_NUM];
extern const u16 adkey_msg_mbox_line_in_table[3][AD_KEY_MAX_NUM];
extern const u16 irff00_msg_line_in_table[3][IR_KEY_MAX_NUM];

#endif
