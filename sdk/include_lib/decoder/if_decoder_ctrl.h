#ifndef __if_decoder_ctrl_h
#define __if_decoder_ctrl_h




#include "string.h"

#include "typedef.h"
//#include "dec_com.h"
//#define DECODE_NORMAL  0x00
//#define DECODE_FF      0x01
//#define DECODE_FR      0x02
//#define DECODE_STOP    0x03

#define SET_DECODE_MODE   0x80


typedef struct _PARM_RECOVERMODE_ {
    u32  RECOVER_MODE_value;
} PARM_RECOVERMODE;

typedef struct _AUDIO_DECODE_PARA {
    u32 mode;
} AUDIO_DECODE_PARA;

typedef struct _PARM_DECODE_STEPV_ {
    u32  ff_fr_step;
} PARM_DECODE_STEPV;

typedef struct _AUDIO_FLEN_PARA {
    u32 flen;
} AUDIO_FLEN_PARA;

typedef struct _AUDIO_FADE_PARA {
    u32 mode;//0表示不进行任何处理,1代表要fade到0
} AUDIO_FADE_PARA;

#define PLAY_MOD_NORMAL   	0x00
#define PLAY_MOD_FF   		0x01
#define PLAY_MOD_FB   		0x02


#define REAPT_MOD_STARTA   	0x01
#define REAPT_MOD_STARTB   	0x02
#define REAPT_MOD_STARTN   	0x03
#define REAPT_MOD_FREPT    	0x04

#define SET_DECODE_MODE   	0x80
#define SET_FILE_TOTAL_LEN	0x84
#define SET_FF_FR_STEP_CMD	0x94

enum {
    SET_BREAKPOINT_A = 0x08,
    SET_BREAKPOINT_B,
    SET_RECOVER_MODE
};

#define CMD_SET_CONTINUE_BK	0x90
#define CMD_SET_PLAY_FILE	0x91
#define CMD_SET_FADEOUT		0x93

//play control
#define PLAY_FILE       0x80000000
#define PLAY_CONTINUE   0x80000001
#define PLAY_NEXT       0x80000002

#define AUDIO_BK_EN

typedef struct _EX_PlayFile_STRUCT_ {
    void *priv;
    u32(*set_play_file)(void *priv, u32 *startpos, u32 *endpos);
} EX_PlayFile_STRUCT;

struct if_decoder_io {
    void *priv;
    int(*input)(void *priv, u32 addr, void *buf, int len);
    int(*output)(void *priv, void *data, int len);
};

typedef struct if_decoder_io IF_DECODER_IO;
typedef struct decoder_inf {
    u16 sr;            ///< sample rate
    u16 br;            ///< bit rate
    u32 nch;           ///<声道
    u32 total_time;     ///<总时间
} dec_inf_t;


typedef struct __audio_decoder_ops {
    char *name;                                                            ///< 解码器名称
    u32(*open)(void *work_buf, const struct if_decoder_io *decoder_io, u8 *bk_point_ptr);  ///<打开解码器
    u32(*format_check)(void *work_buf);					///<格式检查
    u32(*run)(void *work_buf, u32 type);					///<主循环
    dec_inf_t *(*get_dec_inf)(void *work_buf);				///<获取解码信息
    u32(*get_playtime)(void *work_buf);					///<获取播放时间
    u32(*get_bp_inf)(void *work_buf);						///<获取断点信息															//u32 (*need_workbuf_size)() ;							///<获取整个解码所需的buffer
    u32(*need_dcbuf_size)();		                       ///<获取解码需要的buffer
    u32(*need_bpbuf_size)();				                ///<获取保存断点信息需要的buffer
    u32(*dec_confing)(void *work_buf, u32 cmd, void *parm);
} audio_decoder_ops, decoder_ops_t;
extern audio_decoder_ops *get_ump3_ops();
extern audio_decoder_ops *get_f1a_ops();
extern audio_decoder_ops *get_ima_ops();
extern audio_decoder_ops *get_mp3_ops();
extern audio_decoder_ops *get_wav_ops();


///------------------
// #include "sound_effect_api.h"

typedef struct _dec_buff {
    u32 start;
    u32 end;
} dec_buf;



#endif



