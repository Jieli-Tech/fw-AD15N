#ifndef __PLAY_FILE_H__
#define __PLAY_FILE_H__
#include "common/mbox_common.h"
#include "config.h"

enum {						//config command
    MAD_STOP = 0x00,
    MAD_PLAY = 0x01,
    MAD_PLAY_IDLE = 0x11,
    MAD_PAUSE = 0x02,
    MAD_FAST_FORWARD = 0x03,
    MAD_FAST_REVERASE = 0x04,
    MAD_INIT = 0x05,
    MAD_FF_FR_END = 0x06,
    MAD_VOICE_STOP = 0x07,

};

bool decode_get_bp(void *bp_info);
void fs_file_close(void);
u8 fs_mount(void);
u8 fs_get_total_files(void) ;
bool fs_get_file_sclust(u32 *clust);
int fs_get_folferinfo(int *start_num, int *end_num);
u8 decode_pp(void);
bool start_decode(bool break_point);
void stop_decode(void);
bool decode_init(bool break_point);
u16 get_music_play_time(void);
void decode_ff(u8 step);
void decode_fr(u8 step);
int eq_set_mode(u8 mode);
u8 decode_is_enable(void);
void vfs_info_clear(void);
void vfs_info_disable(u8 device_index);

#endif

