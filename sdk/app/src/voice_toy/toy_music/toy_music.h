#ifndef __TOY_MUSIC_H__
#define __TOY_MUSIC_H__

#include "decoder_api.h"
#include "includes.h"
#include "simple_play_file.h"

extern u16 music_msg_filter(u8 key_status, u8 key_num, u8 key_type);
static u32 simple_next_dir(play_control *ppctl);
static u32 simple_switch_device(play_control *ppctl);
extern void sys_idle_deal(u32 usec);
void toy_music_app(void);
#endif
