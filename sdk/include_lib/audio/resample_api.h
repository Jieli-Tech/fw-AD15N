#ifndef __RESAMPLE_API_H__
#define __RESAMPLE_API_H__
#include "typedef.h"

typedef struct _RS_IO_CONTEXT_ {
    void *priv;
    int(*output)(void *priv, void *data, int len);
} RS_IO_CONTEXT;

typedef struct _RS_PARA_STRUCT_ {
    unsigned short insample;
    unsigned short outsample;
    unsigned char  quality;
} RS_PARA_STRUCT;

typedef struct _RS_STUCT_API_ {
    unsigned int(*need_buf)();
    int (*open)(unsigned int *ptr, RS_PARA_STRUCT *rs_para, RS_IO_CONTEXT *rs_io);
    int (*run)(unsigned int *ptr, short *inbuf, int len);
} RS_STUCT_API;

#endif
