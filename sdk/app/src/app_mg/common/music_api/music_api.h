#ifndef _MUSIC_API_H
#define _MUSIC_API_H
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "decoder_api.h"
#include "audio.h"
#include "dac.h"
#include "ump3_api.h"
#include "midi_api.h"
#include "f1a_api.h"
#include "a_api.h"
#include "music_api.h"
#include "music_file_op/music_file_op.h"
#include "speed_api.h"

#define MUSIC_UART_DEBUG

#ifdef MUSIC_UART_DEBUG
#define music_puts           log_info
#define music_printf         log_info
#define music_buf            log_info_hexdump
#define music_err_printf	 log_error
#else
#define music_puts(...)
#define music_printf(...)
#define music_buf(...)
#define music_err_printf
#endif

#define MUSIC_BREAK_POINT_EN 			1
#define MUSIC_LOOP_EN					0

#define FS_TYPE_SYD		"sydfile"
#define FS_TYPE_NORFS	"norfs"
#define FS_TYPE_FAT		"fatfs"

typedef u32(*decode_api_t)(void *p_file, void **ppdec);
typedef void(*decode_stop_t)(dec_obj *obj);
typedef void(*decode_pause_t)(dec_obj *obj);

typedef enum {
    MUSIC_MODE_PLAY_ONE,	//播放一首歌曲结束
    MUSIC_MODE_CIRCUL_ONE, 	//循环播放第一首
    MUSIC_MODE_AUTO_NEXT, 	//自动下一曲
    MUSIC_MODE_AUTO_PREV,	//自动上一曲
} PLAY_MODE;

typedef enum {
    MUSIC_PAUSE = 0x1122,
    MUSIC_PLAY,
    MUSIC_STOP,
} PLAY_STATUS;

typedef enum {
    MUSIC_PLAY_INIT_NONE = 0,
    MUSIC_PLAY_INIT_OK,
} INIT_STAGE;

typedef enum {
    /*获取当前播放时间*/
    MUSIC_CMD_GET_PLAYTIME,
    /*获取播放信息,看结构体dec_inf_t*/
    MUSIC_CMD_GET_INFO,
    /*获取当前播放文件号*/
    MUSIC_CMD_GET_CUR_FILE_INDEX,
    /*获取当前文件夹总文件数*/
    MUSIC_CMD_GET_TOTAL_NUM,
    /*获取当前LOOP次数*/
    MUSIC_CMD_GET_LOOP_NUM,
    /*设置LOOP次数*/
    MUSIC_CMD_SET_LOOP_NUM,
} MUSIC_IO_CTL_CMD;

typedef struct _decode_api_ext {
    dec_obj *p_dec;
} music_decode_api_t;

typedef struct _music_play_obj {
    u8 hdl_active_flg;
    INIT_STAGE en;
    dec_obj *pdec;
    music_decode_api_t decode_api;
    PLAY_MODE play_mode;
    u32 decoder_type;
    u8 loop;/*0~255,255为无限循环*/
#if (MUSIC_BREAK_POINT_EN || MUSIC_LOOP_EN)
    dp_buff dbuff;
#endif
    struct {
        void *pvfs;
        void *pvfile;
        u32 total_file_num;
        u32 cur_file_index;
    } file;
    struct {
        const char *dir;
    } dir;
} music_play_obj;
typedef void(*decode_succ_callback_t)(music_play_obj *hdl);

int music_play_destroy(music_play_obj **hdl);
int music_play(music_play_obj **hdl, const char *dir,  u32 index, u32 decoder_type, \
               void *device, PLAY_MODE play_mode, void *fs_type, void *bpbuff);
int music_midi_ctrl_play(music_play_obj **_hdl);
int music_play_end_operation(music_play_obj **hdl);
int music_play_next_file(music_play_obj **hdl);
int music_play_priv_file(music_play_obj **hdl);
int music_play_pp(music_play_obj *hdl);
int music_play_get_status(music_play_obj *hdl);
int music_play_get_decoder_type(music_play_obj *hdl);
int music_play_io_ctl(music_play_obj *hdl, MUSIC_IO_CTL_CMD cmd, int arg);
int music_play_get_bp(music_play_obj *hdl, dp_buff *dbuff);
void music_decode_succ_cb_regist(decode_succ_callback_t cb);
int music_play_speed_set_para(music_play_obj *hdl, SPEED_PITCH_PARA_STRUCT *sp_parm);
#endif
