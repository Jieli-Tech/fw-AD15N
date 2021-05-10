#ifndef __SIMPLE_DECODER_H__
#define __SIMPLE_DECODER_H__
#include "config.h"
#include "typedef.h"
#include "decoder_api.h"

typedef struct __play_control {
    void *pfile;
    void *pdir;
    u32  findex;
    u32  ftotal;
    void *pdp;  //断点
    dec_obj *p_dec_obj;
    u16  type;
    u8   loop;
    u8   dir_index;
} play_control;


void app(void);
dec_obj *decoder_by_index(void *pvfs, play_control *ppctl);
void decoder_demo(void);
#endif
