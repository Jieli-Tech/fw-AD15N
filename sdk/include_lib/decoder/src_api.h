#ifndef __SRC_API_H__
#define __SRC_API_H__
#include "typedef.h"
// #include "sound_effect_api.h"
// void *speed_api(void *obuf, u32 insample);
//void *speed_api(void *obuf,u32 insample, u32 outsample);
void src_mode_init(void);
void src_reless(void **pp_eobj);
void *src_api(void *obuf, u32 insample, void **ppsound);

#endif

