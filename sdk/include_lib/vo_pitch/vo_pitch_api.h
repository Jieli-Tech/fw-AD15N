#ifndef __VOPITCH_API_H__
#define __VOPITCH_API_H__

#include "sound_effect_api.h"
#include "vo_pitch_phy.h"


typedef enum {
    VP_CMD_ROBOT = 0,    //机器人音效
    VP_CMD_ROBOT2,       //机器人音效2
    VP_CMD_PITCHSHIFT,   //变速变调
    VP_CMD_RAP,          //RAP音效
    VP_CMD_PITCHSHIFT2,  //变速变调2
    VP_CMD_RAP_REALTIME, //实时RAP音效
    VP_CMD_CARTOON,      //卡通音效
} VP_CMD ;

//----fireware io
void *vp_phy(void *obuf, VOICE_PITCH_PARA_STRUCT *pvp_parm, void **ppsound);
sound_out_obj *link_voice_pitch_sound(sound_out_obj *p_curr_sound, void *p_dac_cbuf, void **pp_effect, u32 cmd);


#endif
