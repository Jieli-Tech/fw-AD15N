#ifndef NOTCH_HOWLING_API
#define NOTCH_HOWLING_API

#include "typedef.h"

typedef struct _NotchHowlingParam {
    int gain;
    int Q;
    int fade_time;
    int threshold;
    int SampleRate;
} NotchHowlingParam;

typedef struct _NH_IO_CONTEXT_ {
    void *priv;
    int(*output)(void *priv, void *data, int len);
} NH_IO_CONTEXT;

typedef struct _NH_STRUCT_API_ {
    int(*need_buf)(NotchHowlingParam *param);
    void(*open)(void *workBuf, NotchHowlingParam *para, NH_IO_CONTEXT *io);
    void(*update)(void *workBuf, NotchHowlingParam *para);
    int(*run)(void *workBuf, short *indata, int len); //len 为indata 字节数，返回值为实际消耗indata的字节数
} NH_STRUCT_API;

NH_STRUCT_API *get_notchHowling_ops();


void *link_notch_howling_sound(void *p_sound_out, void *p_dac_cbuf, void **pp_effect, u32 sr);


#endif // !NOTCH_HOWLING_API
