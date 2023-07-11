#ifndef __SIMPLE_PLAY_FILE_H__
#define __SIMPLE_PLAY_FILE_H__

#include "decoder_api.h"
#include "typedef.h"
#if defined(AUDIO_SPEED_EN) && (AUDIO_SPEED_EN)
#include "speed_api.h"
#endif
#define SIMPLE_DEC_BP_ENABLE    1

typedef enum {
    MUSIC_PAUSE = 0x1122,
    MUSIC_PLAY,
    MUSIC_STOP,
} PLAY_STATUS;

typedef struct __play_control {
    void *pfs;          //文件系统句柄
    void *pfile;        //文件句柄
    dec_obj *p_dec_obj; //解码器句柄
    void *pdp;          //断点指针
    u32  findex;        //当前播放的文件序号
    u32  ftotal;        //当前文件夹中的文件总数
    u16  dec_type;      //解码器支持的解码类型，如BIT_F1A1
    u8   dev_index;     //当前挂载设备的序号，如INNER_FLASH_RO
    u8   play_mode;     //播放模式(Music模式使用)
    void *device;       //当前挂载设备的句柄
    void *pdir;         //文件夹指针,用于切换文件夹
    u8   *p_vm_tab;     //存放vm项指针,用于记忆不同文件夹的掉电数据
    u8   dir_total;     //文件夹总数
    u8   dir_index;     //当前播放的文件夹序号
    u8   loop;          //播放循环次数
    // u8   flag;          //标志位(Music模式使用)
} play_control;

u32 get_decoder_status(dec_obj *obj);
u32 simple_dev_fs_mount(play_control *ppctl);
u32 simple_dev_fs_close(play_control *ppctl);
u32 simple_play_file_byindex(play_control *p_music, u32 index);
u32 simple_play_file_operate(play_control *ppctl, char c);
u32 simple_play_file_bypath(play_control *p_music, const char *path);
// ply_file api
u32 play_one_file(play_control *ppctl);
u32 play_prev_file(play_control *ppctl);
u32 play_next_file(play_control *ppctl);
#if defined(AUDIO_SPEED_EN) && (AUDIO_SPEED_EN)
int simple_play_speed_set_para(play_control *ppctl, SPEED_PITCH_PARA_STRUCT *sp_parm);
#endif
#endif
