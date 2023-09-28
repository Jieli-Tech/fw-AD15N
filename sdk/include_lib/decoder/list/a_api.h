#ifndef __A_API_H__
#define __A_API_H__
#include "if_decoder_ctrl.h"
#include "app_modules.h"


u32 a_decode_api(void *p_file, void **p_dec, void *p_dp_buf);
u32 a_buff_api(dec_buf *p_dec_buf);
u32 get_a_dp_buff_size(void);
bool clear_a_dp_buff(void *a_obj);

#if defined(DECODER_A_EN) && (DECODER_A_EN)
#define check_and_clr_a_dp_buff(n)      clear_a_dp_buff(n)
#else
#define check_and_clr_a_dp_buff(...)    false
#endif

#endif
