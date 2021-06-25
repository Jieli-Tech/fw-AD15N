#ifndef NOTCH_HOWLING_API
#define NOTCH_HOWLING_API

#include "typedef.h"
typedef struct _NH_IO_CONTEXT_ {
    void *priv;
    int(*output)(void *priv, void *data, int len);
} NH_IO_CONTEXT;

typedef struct _NH_PARA_STRUCT_ {
    int depth;
    int bandwidth;
    int sampleRate;
} NH_PARA_STRUCT;

typedef struct _NH_STRUCT_API_ {
    int(*need_buf)();
    int(*open)(void *workBuf, NH_PARA_STRUCT *para, NH_IO_CONTEXT *io);
    int(*update)(void *workBuf, NH_PARA_STRUCT *para);
    int(*run)(void *workBuf, short *inbuf, int len);
} NH_STRUCT_API;

NH_STRUCT_API *get_notchHowling_ops();


void *link_notch_howling_sound(void *p_sound_out, void *p_dac_cbuf, void **pp_effect, u32 sr);


#endif // !NOTCH_HOWLING_API
