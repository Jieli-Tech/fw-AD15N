#ifndef __DECODER_API_H__
#define __DECODER_API_H__
#include "if_decoder_ctrl.h"
#include "dac_api.h"
#include "sound_effect_api.h"
#include "decoder_msg_tab.h"


// #define is_halfway_effect(obj)    (obj->enable & B_HALFWAY_EFFECT)
// #define clr_halfway_effect(obj)    obj->enable &= ~B_HALFWAY_EFFECT
// #define set_halfway_effect(obj)    obj->enable |= B_HALFWAY_EFFECT

// #define kick_decoder()   bit_set_swi(0)
#define kick_decoder   bit_set_swi0

typedef enum {
    MAD_ERROR_FILE_END         = 0x40,
    MAD_ERROR_FILESYSTEM_ERR   = 0x41,              // NO USED
    MAD_ERROR_DISK_ERR         = 0x42,              // NO USED
    MAD_ERROR_SYNC_LIMIT       = 0x43,              // 文件错误
    MAD_ERROR_FF_FR_FILE_END   = 0x44,              //快进结束
    MAD_ERROR_FF_FR_END        = 0x45,              // NO USED
    MAD_ERROR_FF_FR_FILE_START = 0x46,              //快退到头
    MAD_ERROR_LIMIT            = 0x47,              // NO USED
    MAD_ERROR_NODATA           = 0x48,              // NO USED
    MAD_ERROR_PLAY_END		   = 0x50,				//MIDI CTRL DATA OUTPUT END
    MAD_ERROR_F1X_START_ADDR   = 0x51,				//F1X起始位置错误
} MAD_INFO ;

typedef enum {
    D_TYPE_F1A_1 = 0,
#if (MAX_F1A_CHANNEL > 1)
    D_TYPE_F1A_2 = 1,
#endif
    D_TYPE_UMP3 = 2,
    D_TYPE_A,
    D_TYPE_MIDI,
    D_TYPE_MIDI_CTRL,
    D_TYPE_WAV,
    D_TYPE_MP3_ST,
} DECOER_TYPE ;

#define DEC_FUNCTION_FF_FR		BIT(0)	// 快进快退功能

typedef struct _dec_obj {
    void *p_file;
    void *dec_ops;
    void *p_dbuf;
    void *p_dp_buf;
    u8 *event_tab;
    u32 sr;
    sound_out_obj sound;
    void *src_effect;
    u32(*eq)(u32);
    u8 loop;
    u8 type; // DECOER_TYPE
    u8 function;		// 解码器支持的功能
    char ff_fr_step;	// 快进快退步长。正数-快进；负数-快退。单位-秒
} dec_obj;

typedef struct _dp_buff {

    u32 findex;
    u16 crc;
    u16 len;
    union {
        u8 buff[1];
#if DECODER_UMP3_EN
        u8 mp3[20];
#endif
#if DECODER_F1A_EN
        u8 f1a[60];
#endif
#if DECODER_WAV_EN
        u8 wav[12];
#endif
    };
} dp_buff;


extern dec_obj dec_ump3_hld;
extern dec_obj dec_midi_hld;
extern dec_obj dec_midi_ctrl_hld;
extern dec_obj dec_f1a_hld[];
extern dec_obj dec_a_hld;
extern dec_obj dec_mp3_st_hld;
extern dec_obj dec_wav_hld;

//断点函数
bool get_dp(dec_obj *obj, dp_buff *dbuff);
void *check_dp(dp_buff *dbuff);
void clear_dp(dp_buff *dbuff);
void clear_dp_buff(void *buff);


int mp_input(void *priv, u32 addr, void *buf, int len);
int mp_output(void *priv, void *data, int len);
int effect_output(void *priv, void *data, int len);
void decoder_init(void);
u32 decoder_set_sr(dec_obj *d_obj);

typedef enum  {//停止解码时，是否需要将DAC中剩余的样点消耗完
    NO_WAIT = 0,
    NEED_WAIT = 1,
} DEC_STOP_WAIT;




extern u32 dec_hld_tab[];
extern const u32 decoder_tab[];


bool dac_cbuff_active(void *sound_hld);
void dac_kick_decoder(void *sound_hld, void *pkick);
void decoder_test_fun(void);
u32 mp_read_2_dac(void);
bool check_ext_api(char _xdata *fname, char const *ext, u32 len);


void irq_decoder_ret(dec_obj *obj, u32 ret);

void decoder_soft0_isr();
void decoder_channel_set(u8 dc);

void decoder_pause(dec_obj *obj);
void decoder_stop(dec_obj *obj, DEC_STOP_WAIT wait);
void decoder_stop_now(dec_obj *obj);
int decoder_fun(void *pfile, u32 dec_ctl, s32 *dec_index);
// dec_obj *decoder_io(void *pfile, u32 dec_ctl, dp_buff * dbuff);
dec_obj *decoder_io(void *pfile, u32 dec_ctl, dp_buff *dbuff, u8 loop);
// void decoder_fun(u32(*fun)(void *, void **), void *pfs, void **ppfile, u32 file_index);

void decoder_ff(dec_obj *obj, u8 step);	// 快进。step单位-秒
void decoder_fr(dec_obj *obj, u8 step);	// 快退。step单位-秒

void decoder_set_file_size(dec_obj *obj, u32 size);	// 设置解码文件长度
void decoder_soft_hook(void);

#endif


