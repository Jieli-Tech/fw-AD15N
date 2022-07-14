#ifndef __VOPITCH_PHY_H__
#define __VOPITCH_PHY_H__

typedef struct _VP_IO_CONTEXT_ {
    void *priv;
    int(*output)(void *priv, void *data, int len);
} VP_IO_CONTEXT;

enum {
    HARMO_ROBORT = 0x1,
    HARMO_PITCHSHIFT = 0x2,
    HARMO_RAP = 0x4,
    HARMO_PITCHSHIFT2 = 0x8,
    HARMO_ECHO = 0x10,
    HARMO_RAP_REALTIME = 0x20,
    HARMO_ROBORT2 = 0x40,
    HARMO_CARTOON = 0x80,
    HARMO_NULL = 0x80,
};

enum {
    FILT_NORH = 1,
    FILT_LOWSELF = 2,
    FILT_HIGHSELF = 3,
    FILT_BAND
};

enum {
    RAP_PREPARE = 0x0,
    RAP_START = 0x01,
    RAP_END
};

typedef struct _VOICE_PITCH_PARA_STRUCT_ {
    unsigned short do_flag;         //选择什么效果
    unsigned short samplerate;      //输入音频采样率，支持16k/24k
    unsigned short noise_dc;        //能量阈值，默认值2048，低于该值的声音mute住
    unsigned short pitchrate;       //音调高低，128代表原音调，<128音调升高，>128音调降低，建议范围 40-256
    unsigned short hamorrate;       //音色调整，128代表不变，<128音色变变， >128音色变沉
    const unsigned char *midi_file; //RAP模式的音源输入
    unsigned short midifile_len;    //RAP模式音源大小
    void (*callback)(void *priv, int pos);//RAP模式的状态回调：因为rap模式输入输出长度不等，所以加了个回调，表示状态
    void *priv;
} VOICE_PITCH_PARA_STRUCT;

typedef struct  _VOICEPITCH_STUCT_API_ {
    unsigned int(*need_buf)();
    int(*open)(unsigned int *ptr, VOICE_PITCH_PARA_STRUCT *speedpitch_para, VP_IO_CONTEXT *rs_io);
    int(*run)(unsigned int *ptr, short *inbuf, int len);
} VOICEPITCH_STUCT_API;

VOICEPITCH_STUCT_API *get_vopitch_context();

extern void no_filt_init(void *ptr, int sr, VP_IO_CONTEXT *vp_io);
extern int  no_filt_do(void *ptr, void *data, int len);
extern int  no_filt_needbuf();

#endif
