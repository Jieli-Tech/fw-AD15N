#ifndef __MP3_ST_API_H__
#define __MP3_ST_API_H__
#include "if_decoder_ctrl.h"

u32 mp3_st_decode_api(void *p_file, void **p_dec, void *p_dp_buf);
u32 mp3_st_buff_api(dec_buf *p_dec_buf);

#endif


