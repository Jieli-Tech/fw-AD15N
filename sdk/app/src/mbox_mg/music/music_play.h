#ifndef _MUSIC_PLAY_H
#define _MUSIC_PLAY_H
void music_app(void);

#ifndef __MUSIC_PLAY_H__
#define __MUSIC_PLAY_H__
#include "common/mbox_common.h"
#include "config.h"
#include "key.h"

//OBUF复位及buffer大小设置
#define OBUF_REST       0x0
#define OBUF_0          0x1
#define OBUF_1          0x2
#define OBUF_2          0x3
#define OBUF_3          0x4

#if 1//def __SMART_CPU__


enum {
    NORMAL = 0,
    POP,
    ROCK,
    JAZZ,
    CLASSIC,
};


typedef struct _MUSIC_PLAY_VAR {
    u8  bPlayStatus;    //<播放状态
    u8  bEQ;            //<EQ 音效
} MUSIC_PLAY_VAR;


void music_app(void);
void set_eq(bool eq_en, u8 eq);
bool play_device_file(bool file_break_point);
extern u32 count_energy(u8 *addr, u8 len);

extern _no_init bool mem_update;
extern _no_init MUSIC_PLAY_VAR Music_Play_var;
extern _no_init u32 _xdata music_energy;
extern const u16 iokey_msg_mbox_music_table[3][IO_KEY_MAX_NUM];
extern const u16 adkey_msg_mbox_music_table[3][AD_KEY_MAX_NUM];
extern const u16 irff00_msg_music_table[3][IR_KEY_MAX_NUM];
#endif

#endif

#endif
