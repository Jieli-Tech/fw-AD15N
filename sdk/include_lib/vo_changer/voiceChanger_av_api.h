
#ifndef _voiceChanger_av_api_h__
#define _voiceChanger_av_api_h__
#include "typedef.h"

enum {
    EFFECT_VC_AV_PITCHSHIFT = 0x00,
    EFFECT_VC_AV_F0_TD,
    EFFECT_VC_AV_F0_TD_AUTOFIT,
    EFFECT_VC_AV_PITCHSHIFT_AUTOFIT,
    EFFECT_VC_AV_BIRD,
    EFFECT_VC_AV_BIRD1,
    EFFECT_VC_AV_BIRD2,
    EFFECT_VC_AV_BIRD3,
    EFFECT_VC_AV_BIRD4,
    EFFECT_VC_AV_BIRD5,
    EFFECT_VC_AV_BIRD6
};


typedef  struct  _VOICESYN_AV_PARM_ {
    u32  vibrate_rate_u;     //0-100
    u32  vibrate_rate_d;     //0-100 影响抖音最大幅度
    u32  vibrate_lenCtrol;   //0-100 抖音衰减速度，影响抖音持续时长，100代表最短，0代表最长
    u32  randpercent;        //0-100
} VOICESYN_AV_PARM;


typedef struct _VOICECHANGER_AV_PARM {
    u32 effect_v;                    //
    u32 shiftv;                      //pitch rate:  30-250
    u32 formant_shift;               // 30-250
    u32 speedv;                      // 50-200
} VOICECHANGER_AV_PARM;


typedef struct _VCAD_IO_CONTEXT_ {
    void *priv;
    int(*output)(void *priv, void *data, int len);
} VCAD_IO_CONTEXT;

typedef struct _VOICECHANGER_A_FUNC_API_ {
    u32(*need_buf)(u32 sr, VOICECHANGER_AV_PARM *vc_parm);
    int (*open)(void *ptr, u32 sr, VOICECHANGER_AV_PARM *vc_parm, VOICESYN_AV_PARM *vsyn_ctrol, VCAD_IO_CONTEXT *rs_io);       //中途改变参数，可以调init
    int (*run)(void *ptr, short *indata, int len);    //len是多少个byte
} VOICECHANGER_A_FUNC_API;

extern VOICECHANGER_A_FUNC_API *get_voiceChangerA_func_api();

void *link_voice_changer_sound(void *p_sound_out, void *p_dac_cbuf, void **pp_effect, u32 in_sr);
void *voice_changer_api(void *obuf, u32 sr, void **ppsound);
int voice_changer_run(void *hld, short *inbuf, int len);
void *voice_changer_phy(void *obuf, u32 sr, VOICECHANGER_AV_PARM *pvc_parm, VOICESYN_AV_PARM *vsyn_ctrol, void **ppsound);
#endif // reverb_api_h__

