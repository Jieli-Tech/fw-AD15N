#ifndef __SRC_API_H__
#define __SRC_API_H__
#include "typedef.h"
#include "app_modules.h"
#include "resample_api.h"
#include "src.h"

#ifdef HAS_HW_SRC_EN
//获取硬件src ops句柄
extern const SRC_STUCT_API *get_src_context(void);
#define GET_SRC_OPS()     get_src_context()

#else
//获取软件src ops句柄
extern const SRC_STUCT_API *get_soft_src_context(void);
#define GET_SRC_OPS()     get_soft_src_context()

#endif

// #include "sound_effect_api.h"
// void *speed_api(void *obuf, u32 insample);
//void *speed_api(void *obuf,u32 insample, u32 outsample);
void src_mode_init(void);
void src_reless(void **pp_eobj);
void *src_api(void *obuf, u32 insample, u32 outsample, void **ppsound, void *p_src_ops);
void *link_src_sound(void *p_sound_out, void *p_dac_cbuf, void **pp_effect, u32 insample, u32 outsample, void *p_src_ops);
void *src_hld_malloc();

#endif

