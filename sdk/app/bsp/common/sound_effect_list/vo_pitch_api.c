/* #include "resample_api.h" */
/* #include "resample.h" */
#include "typedef.h"
/* #include "decoder_api.h" */
#include "config.h"
#include "sound_effect_api.h"
#include "vo_pitch_api.h"
#include "midi_file0_h.h"
#include "dac.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"

#if VO_PITCH_EN

const int  VC_ENABLE_FLAG = 1;
const  int  EXTRA_DATA_SIZE = 500;  //影响rap的音频最大允许长度
const  int  VP_DECAY_VAL = 70;      //ECHO模式的decay速度
const  int  VP_HIS_LEN = 2000;       //ECHO模式的delay:复用其他模式的Buf的

VOICE_PITCH_PARA_STRUCT vp_parm AT(.vp_data);
void rap_callback(void *priv, int pos);
bool vp_cmd_case(VP_CMD cmd, VOICE_PITCH_PARA_STRUCT *p_vc_parm);

void *voice_pitch_api(void *obuf, u32 cmd, void **ppsound)
{
    VOICE_PITCH_PARA_STRUCT vc_parm;
    log_info("voice pitch api\n");

    vp_cmd_case(cmd, &vc_parm);

    return vp_phy(obuf, &vc_parm, ppsound);
}

sound_out_obj *link_voice_pitch_sound(sound_out_obj *p_curr_sound, void *p_dac_cbuf, void **pp_effect, u32 cmd)
{
    sound_out_obj *p_next_sound = 0;
    p_curr_sound->effect = voice_pitch_api(p_curr_sound->p_obuf, cmd, (void **)&p_next_sound);
    if (NULL != p_curr_sound->effect) {
        if (NULL != pp_effect) {
            *pp_effect = p_curr_sound->effect;
        }
        p_curr_sound->enable |= B_DEC_EFFECT;
        p_curr_sound = p_next_sound;
        p_curr_sound->p_obuf = p_dac_cbuf;
        log_info("src init succ\n");
    } else {
        log_info("src init fail\n");
    }
    return p_curr_sound;
}





//整体接口跟之前的lib_speed_pitch.a 的io接口是一样的，除了RAP 模式参数, RAP模式的进出数据是不等的， 先录一段音，然后计算完在开始播放。 所有在rap模式加了个callback。其中的参数pos代表当前状态：
u8 startindexflag;
void rap_callback(void *priv, int pos)
{
    if (pos == RAP_PREPARE) {
        startindexflag = 0;                //进入不会出数阶段，只会消耗来数,生成音源
    } else if (pos == RAP_START) {
        startindexflag = 1;                //进入run会出数，但是跟输入的数据无关，跟进midi表来生成的阶段
    } else if (pos == RAP_END) {
        startindexflag = 0;                //输出完毕了,结束了
    }
}




/*************  RAP 模式参数[所有参数都有效]********************/
void vp_cmd_rap(VOICE_PITCH_PARA_STRUCT *p_vc_parm)
{
    u32 sr = dac_sr_read();
    p_vc_parm->do_flag = HARMO_RAP;
    p_vc_parm->samplerate = sr;              //支持16k/24k
    p_vc_parm->noise_dc = 2048;
    p_vc_parm->hamorrate = 128;
    p_vc_parm->pitchrate = 100;
    p_vc_parm->midi_file = midifile_file0_tab;
    p_vc_parm->midifile_len = sizeof(midifile_file0_tab);
    p_vc_parm->callback = rap_callback;
    p_vc_parm->priv = NULL;
}

/************  机器音模式 [红色部分参数有效]***********************************/
void vp_cmd_robot(VOICE_PITCH_PARA_STRUCT *p_vc_parm)
{
    p_vc_parm->do_flag = HARMO_ROBORT;
    p_vc_parm->noise_dc = 2048;
    p_vc_parm->hamorrate = 100;
    p_vc_parm->pitchrate = 80;                          //不同的组合改变输出的机器音的音色
    p_vc_parm->midi_file = midifile_file0_tab;
    p_vc_parm->midifile_len = sizeof(midifile_file0_tab);
    p_vc_parm->callback = rap_callback;
    p_vc_parm->priv = NULL;
}

/************  机器音模式 [红色部分参数有效]***********************************/
void vp_cmd_robot2(VOICE_PITCH_PARA_STRUCT *p_vc_parm)
{
    p_vc_parm->noise_dc = 2048;
    p_vc_parm->samplerate = 16000;
    p_vc_parm->do_flag = HARMO_ROBORT2;
    p_vc_parm->hamorrate = 128;
    p_vc_parm->pitchrate = 180;
    p_vc_parm->midi_file = midifile2_file1_tab;
    p_vc_parm->midifile_len = sizeof(midifile2_file1_tab);
    p_vc_parm->callback = rap_callback;
    p_vc_parm->priv = NULL;
}

/************  变调模式 [红色部分参数有效]***********************************/
void vp_cmd_pitchshift(VOICE_PITCH_PARA_STRUCT *p_vc_parm)
{
    p_vc_parm->do_flag = HARMO_PITCHSHIFT;
    p_vc_parm->noise_dc = 2048;
    p_vc_parm->hamorrate = 128;
    p_vc_parm->pitchrate = 80;                 //<128，音调升高，  > 128 音调变低
    p_vc_parm->midi_file = midifile_file0_tab;
    p_vc_parm->midifile_len = sizeof(midifile_file0_tab);
    p_vc_parm->callback = rap_callback;
    p_vc_parm->priv = NULL;
}
/* test_ops->open(rs_buf, &vc_parm, NULL); */



/********* 变声模式 [红色部分参数有效]***********************************/
void vp_cmd_pitchshift2(VOICE_PITCH_PARA_STRUCT *p_vc_parm)
{
    p_vc_parm->do_flag = HARMO_PITCHSHIFT2;
    p_vc_parm->noise_dc = 2048;
    p_vc_parm->hamorrate = 90;
    p_vc_parm->pitchrate = 60;                                //不同的组合，出来的声音效果会变,这组默认参数比较接近男变女的效果
    p_vc_parm->midi_file = midifile_file0_tab;
    p_vc_parm->midifile_len = sizeof(midifile_file0_tab);
    p_vc_parm->callback = rap_callback;
    p_vc_parm->priv = NULL;
}

void vp_cmd_rap_realtime(VOICE_PITCH_PARA_STRUCT *p_vc_parm)
{
    p_vc_parm->noise_dc = 2048;
    p_vc_parm->samplerate = 16000;
    p_vc_parm->do_flag = HARMO_RAP_REALTIME ;
    p_vc_parm->hamorrate = 128;
    p_vc_parm->pitchrate = 128;
    p_vc_parm->midi_file = midifile2_file1_tab;
    p_vc_parm->midifile_len = sizeof(midifile2_file1_tab);
    p_vc_parm->callback = rap_callback;
    p_vc_parm->priv = NULL;

}

bool vp_cmd_case(VP_CMD cmd, VOICE_PITCH_PARA_STRUCT *p_vc_parm)
{
    switch (cmd) {
    case VP_CMD_ROBOT:
        vp_cmd_robot(p_vc_parm);
        break;
    case VP_CMD_ROBOT2:
        vp_cmd_robot2(p_vc_parm);
        break;
    case VP_CMD_PITCHSHIFT:
        vp_cmd_pitchshift(p_vc_parm);
        break;
    case VP_CMD_RAP:
        vp_cmd_rap(p_vc_parm);
        break;
    case VP_CMD_PITCHSHIFT2:
        vp_cmd_pitchshift2(p_vc_parm);
        break;
    case VP_CMD_RAP_REALTIME:
        vp_cmd_rap_realtime(p_vc_parm);
        break;
    /* case VP_CMD_ECHO: */
    /* break; */
    default:
        return false;
    }
    return true;
}


#endif


