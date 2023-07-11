/* #include "resample_api.h" */
/* #include "resample.h" */
#include "typedef.h"
/* #include "decoder_api.h" */
#include "config.h"
#include "sound_effect_api.h"
#include "vo_pitch_api.h"
#include "midi_file0_h.h"
#include "dac.h"
#include "audio_adc.h"
#include "app_modules.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[vo_pitch]"
#include "log.h"

#if VO_PITCH_EN

const int VC_ENABLE_FLAG        = 1;
const int EXTRA_DATA_SIZE       = 2000; //rap模式所可以包含的音源长度
const int VO_RAP_LOOPEN         = 0;    //rap模式是否repeat音源
const int VO_RAP_COMPRESS_RATE  = 460;  //0到460：越大音源时间压缩越多
const int VP_DECAY_VAL          = 70;   //ECHO模式的decay速度
const int VP_HIS_LEN            = 2000; //ECHO模式的delay:复用其他模式的Buf的

VOICE_PITCH_PARA_STRUCT vp_parm AT(.vp_data);


void rap_callback(void *priv, int pos);
bool vp_cmd_case(VP_CMD cmd, VOICE_PITCH_PARA_STRUCT *p_vc_parm);


EFFECT_OBJ vp_obj AT(.vp_data);
/* u32  VP_BUFLEN[(5 * 1024 + 512) / 4] AT(.vp_data);         //处理32k音频变速变调需要的空间 */

static sound_in_obj vp_si AT(.vp_data);




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
        log_info("voice pitch init succ\n");
    } else {
        log_info("voice pitch init fail\n");
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

/* rap结束后调用该函数重新开始rap模式 */
u32 VP_BUFLEN[];
void vp_cmd_rap(VOICE_PITCH_PARA_STRUCT *p_vc_parm);
void rap_reopen(void)
{
    log_info("rap reopen!\n");
    VOICE_PITCH_PARA_STRUCT p_vc_parm;
    vp_cmd_rap(&p_vc_parm);

    VOICEPITCH_STUCT_API *ops;
    ops = get_vopitch_context();
    ops->open(&VP_BUFLEN[0], &p_vc_parm, NULL);
}


/*************  RAP 模式参数[所有参数都有效]********************/
void vp_cmd_rap(VOICE_PITCH_PARA_STRUCT *p_vc_parm)
{
    p_vc_parm->do_flag = HARMO_RAP;
    p_vc_parm->samplerate = read_audio_adc_sr();
    p_vc_parm->noise_dc = 2048;
    p_vc_parm->hamorrate = 128;
    p_vc_parm->pitchrate = 100;
    p_vc_parm->midi_file = midifile_file0_tab;
    p_vc_parm->midifile_len = sizeof(midifile_file0_tab);
    p_vc_parm->callback = rap_callback;
    p_vc_parm->priv = NULL;
}

/************  平调机器音模式 [红色部分参数有效]***********************************/
void vp_cmd_robot(VOICE_PITCH_PARA_STRUCT *p_vc_parm)
{
    p_vc_parm->do_flag = HARMO_ROBORT;
    p_vc_parm->samplerate = read_audio_adc_sr();
    p_vc_parm->noise_dc = 2048;
    p_vc_parm->hamorrate = 100; //机器音音色，50到250
    p_vc_parm->pitchrate = 80;  //机器音音高，50到250，不同的组合改变输出的机器音的音色
    //无效参数：
    p_vc_parm->midi_file = midifile_file0_tab;
    p_vc_parm->midifile_len = sizeof(midifile_file0_tab);
    p_vc_parm->callback = rap_callback;
    p_vc_parm->priv = NULL;
}

/************  变调机器音模式 [红色部分参数有效]***********************************/
void vp_cmd_robot2(VOICE_PITCH_PARA_STRUCT *p_vc_parm)
{
    p_vc_parm->do_flag = HARMO_ROBORT2;
    p_vc_parm->samplerate = read_audio_adc_sr();
    p_vc_parm->noise_dc = 2048;
    p_vc_parm->pitchrate = 180; //调出不同的声音
    //无效参数：
    p_vc_parm->hamorrate = 128;
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
    p_vc_parm->samplerate = read_audio_adc_sr();
    //改变pitchrate可以跳变出女声(80)，娃娃音(60)，怪兽音(200)，男声(150)等
    p_vc_parm->pitchrate = 80;  //<128音调升高，>128音调变低
    //无效参数：
    p_vc_parm->hamorrate = 128;
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
    p_vc_parm->samplerate = read_audio_adc_sr();
    p_vc_parm->hamorrate = 90;  //50到250
    p_vc_parm->pitchrate = 60;  //50到250，不同的组合，出来的声音效果会变,这组默认参数比较接近男变女的效果
    //无效参数：
    p_vc_parm->midi_file = midifile_file0_tab;
    p_vc_parm->midifile_len = sizeof(midifile_file0_tab);
    p_vc_parm->callback = rap_callback;
    p_vc_parm->priv = NULL;
}

/********* 实时RAP模式 [红色部分参数有效]***********************************/
void vp_cmd_rap_realtime(VOICE_PITCH_PARA_STRUCT *p_vc_parm)
{
    p_vc_parm->do_flag = HARMO_RAP_REALTIME ;
    p_vc_parm->noise_dc = 2048;
    p_vc_parm->samplerate = read_audio_adc_sr();
    p_vc_parm->hamorrate = 128; //改变音色，50到250
    p_vc_parm->pitchrate = 128; //改变音高，50到250
    p_vc_parm->midi_file = midifile2_file1_tab; //改变旋律
    p_vc_parm->midifile_len = sizeof(midifile2_file1_tab);
    //无效参数：
    p_vc_parm->callback = rap_callback;
    p_vc_parm->priv = NULL;

}

/********* 卡通变声模式 [红色部分参数有效]***********************************/
void vp_cmd_cartoon(VOICE_PITCH_PARA_STRUCT *p_vc_parm)
{
    p_vc_parm->do_flag = HARMO_CARTOON;
    p_vc_parm->noise_dc = 2048;
    p_vc_parm->samplerate = read_audio_adc_sr();
    p_vc_parm->hamorrate = 400; //-800到800
    p_vc_parm->pitchrate = 70;  //50到250
    //无效参数：
    p_vc_parm->midi_file = NULL;
    p_vc_parm->midifile_len = 0;
    p_vc_parm->callback = NULL;
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
    case VP_CMD_CARTOON:
        vp_cmd_cartoon(p_vc_parm);
        break;
    /* case VP_CMD_ECHO: */
    /* break; */
    default:
        return false;
    }
    return true;
}





/********************************VP_PHY************************/
u32  VP_BUFLEN[(8 * 1024) / 4] AT(.vp_data);       //处理32k音频变速变调需要的空间

const struct _VP_IO_CONTEXT_ vp_pitch_io = {
    &vp_obj.sound,      //input跟output函数的第一个参数，解码器不做处理，直接回传，可以为NULL
    sound_output,
};

static int vp_run(void *hld, short *inbuf, int len)
{
    VOICEPITCH_STUCT_API *ops;
    int res = 0;
    sound_in_obj *p_si = hld;
    ops = p_si->ops;
    res = ops->run(p_si->p_dbuf, inbuf, len);
    return res;
}
void *vp_phy(void *obuf, VOICE_PITCH_PARA_STRUCT *pvp_parm, void **ppsound)
{
    u32 buff_len, i;
    VOICEPITCH_STUCT_API *ops;
    log_info("voice pitch api\n");

    ops = get_vopitch_context();           //获取变采样函数接口
    buff_len = ops->need_buf();                          //运算空间获取
    log_info("vo pitch buff need len: %d\n", buff_len);
    if (buff_len > sizeof(VP_BUFLEN)) {
        log_error("vo pitch buff is not enough big!\n");
        return 0;
    }
    /******************************************/
    //初始化：rs_buf：运算Buf; rs_parm：参数指针，传完可以释放的，里面不会记录这个指针的。vp_pitch_io:output接口，说明如下
    ops->open(&VP_BUFLEN[0], pvp_parm, (void *)&vp_pitch_io);
    /*************************************************/
    memset(&vp_obj, 0, sizeof(vp_obj));
    vp_si.ops = ops;
    vp_si.p_dbuf = &VP_BUFLEN[0];
    /*************************************************/
    vp_obj.p_si = &vp_si;
    vp_obj.run = vp_run;
    vp_obj.sound.p_obuf = obuf;
    *ppsound = &vp_obj.sound;
    log_info("vp_succ\n");
    return &vp_obj;
}

#endif


