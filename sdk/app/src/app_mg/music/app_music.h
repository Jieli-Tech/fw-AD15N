#ifndef _APP_MUSIC_H
#define _APP_MUSIC_H

#include "key.h"
#include "app_mg/app_mg.h"

extern const u16 iokey_msg_music_table[3][IO_KEY_MAX_NUM];
extern const u16 adkey_msg_music_table[3][AD_KEY_MAX_NUM];
int app_music(void *param);

#endif
