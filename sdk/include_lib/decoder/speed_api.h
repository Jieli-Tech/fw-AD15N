#ifndef __SPEED_API_H__
#define __SPEED_API_H__
#include "typedef.h"

typedef struct _SPEEDPITCH_IO_CONTEXT_ {
    void *priv;
    int(*output)(void *priv, void *data, int len);
} SPEEDPITCH_IO_CONTEXT;

typedef struct _SPEED_PITCH_PARA_STRUCT_ {
    unsigned short insample;
    unsigned short pitchrate;        //128<=>1
    unsigned char  quality;
    unsigned char speedin;
    unsigned char speedout;
    unsigned char pitchflag;
} SPEED_PITCH_PARA_STRUCT;

typedef struct  _SPEEDPITCH_STUCT_API_ {
    unsigned int(*need_buf)(int srv);
    int(*open)(unsigned int *ptr, SPEED_PITCH_PARA_STRUCT *speedpitch_para, SPEEDPITCH_IO_CONTEXT *rs_io);
    int(*run)(unsigned int *ptr, short *inbuf, int len);
} SPEEDPITCH_STUCT_API;

SPEEDPITCH_STUCT_API *get_sppitch_context();






// void *speed_api(void *obuf, u32 insample);
//void *speed_api(void *obuf,u32 insample, u32 outsample);
void *speed_api(void *obuf, u32 insample, void **ppsound);
void *speed_phy(void *obuf, SPEED_PITCH_PARA_STRUCT *psp_parm, void **ppsound);


#endif

