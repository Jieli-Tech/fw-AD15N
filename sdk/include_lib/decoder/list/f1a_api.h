#ifndef __F1A_API_H__
#define __F1A_API_H__
#include "if_decoder_ctrl.h"

u32 f1a_decode_api_1(void *p_file, void **p_pdec, void *p_dp_buf);
u32 f1a_decode_api_2(void *p_file, void **p_pdec, void *p_dp_buf);

u32 f1a_1_buff_api(dec_buf *p_dec_buf);
u32 f1a_2_buff_api(dec_buf *p_dec_buf);

#endif


