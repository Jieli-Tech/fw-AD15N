/* #include "resample_api.h" */
#include "notch_howling_api.h"
/* #include "resample.h" */
#include "decoder_api.h"
#include "config.h"
#include "sound_effect_api.h"
#include "howling_api.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"


void *notch_howling_api(void *obuf, u32 sr, void **ppsound)
{
    NH_PARA_STRUCT nhparm = {0};
    nhparm.depth          = 10;  //深度
    nhparm.bandwidth      = 45; //带宽
    nhparm.sampleRate     = sr;//采样率

    return notch_howling_phy(obuf, &nhparm, ppsound);
}

void *link_notch_howling_sound(void *p_sound_out, void *p_dac_cbuf, void **pp_effect, u32 sr)
{
    sound_out_obj *p_next_sound = 0;
    sound_out_obj *p_curr_sound = p_sound_out;
    p_curr_sound->effect = notch_howling_api(p_curr_sound->p_obuf, sr, (void **)&p_next_sound);
    if (NULL != p_curr_sound->effect) {
        if (NULL != pp_effect) {
            *pp_effect = p_curr_sound->effect;
        }
        p_curr_sound->enable |= B_DEC_EFFECT;
        p_curr_sound = p_next_sound;
        p_curr_sound->p_obuf = p_dac_cbuf;
    } else {
        log_info("echo init fail\n");
    }
    return p_curr_sound;
}



