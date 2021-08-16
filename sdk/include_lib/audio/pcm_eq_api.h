#ifndef pcm_eq_api_h__
#define pcm_eq_api_h__

#include "typedef.h"
#include "pcm_eq.h"


void *link_pcm_eq_sound(void *p_sound_out, void *p_dac_cbuf, void **pp_effect, u32 sr, u32 channel);

#endif // pcm_eq_api_h__
