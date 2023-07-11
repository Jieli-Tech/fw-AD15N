#ifndef __TOY_IDLE_H__
#define __TOY_IDLE_H__
#include "typedef.h"
#include "key.h"

extern const u16 iokey_msg_idle_table[5][IO_KEY_MAX_NUM];
extern const u16 irkey_msg_idle_table[5][IO_KEY_MAX_NUM];
extern const u16 adkey_msg_idle_table[5][AD_KEY_MAX_NUM];
extern u16 idle_msg_filter(u8 key_status, u8 key_num, u8 key_type);
extern void app_next_mode(void);
void toy_idle_app(void);
#endif
