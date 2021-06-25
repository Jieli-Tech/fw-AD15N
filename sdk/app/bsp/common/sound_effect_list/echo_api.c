/* #include "resample_api.h" */
#include "echo_api.h"
/* #include "resample.h" */
#include "decoder_api.h"
#include "config.h"
#include "sound_effect_api.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"

#define MAX_RATE 24000

#if (MAX_RATE == 16000)
#define echo_work_buf_len   3692
#elif (MAX_RATE == 24000)
#define echo_work_buf_len   5292
#endif

/*1代表下采，0代表不下采。下采的话，申请的Buf会减少，但是回声部分的高频也会丢失*/
const  int DOWN_S_FLAG = 0;


u32 echo_work_buf[(echo_work_buf_len + 3) / 4] AT(.echo_data);//echo混响buf长度，采样率改变，buf长度会变

void *echo_api(void *obuf, u32 sr, void **ppsound)
{
    ECHO_PARM eparm;
    memset(&eparm, 0, sizeof(ECHO_PARM));
    log_info("echo_api\n");
    eparm.echo_parm_obj.decayval          	= 60;  //decay(0~70)回声衰减比
    eparm.echo_parm_obj.delay             	= 200; //回声延时0~300ms :范围0到max_ms，超过max_ms的话，会当成max_ms的
    eparm.echo_parm_obj.energy_vad_threshold = 512; //mute阈值
    eparm.echo_parm_obj.direct_sound_enable  = 1;   //如果运算结果需要叠加干声，则这个置成1，否则置0
    eparm.echo_fix_parm.max_ms               = 100; //最大延时100ms
    eparm.echo_fix_parm.sr                   = sr;  //采样率
    eparm.echo_fix_parm.wetgain              = 3000;//湿声增益
    eparm.echo_fix_parm.drygain              = 4096;//干声增益，如果direct_sound_enable是0，则这个参数无效，因为不叠加干声了
    eparm.ptr = echo_work_buf;
    eparm.ptr_len = sizeof(echo_work_buf);

    return echo_phy(obuf, &eparm, ppsound);
}

void *link_echo_sound(void *p_sound_out, void *p_dac_cbuf, void **pp_effect, u32 sr)
{
    sound_out_obj *p_next_sound = 0;
    sound_out_obj *p_curr_sound = p_sound_out;
    p_curr_sound->effect = echo_api(p_curr_sound->p_obuf, sr, (void **)&p_next_sound);
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



