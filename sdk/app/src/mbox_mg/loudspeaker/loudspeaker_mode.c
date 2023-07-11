/*--------------------------------------------------------------------------*/
/**@file    loudspeaker_mode.c
   @brief   loudspeaker 模式主循环
   @details
   @author
   @date
   @note
*/
/*----------------------------------------------------------------------------*/
#include "typedef.h"
#include "loudspeaker_mode.h"
#include "msg.h"
#include "common/hot_msg.h"
#include "key.h"
#include "mbox_main.h"
#include "common/mbox_common.h"
#include "audio.h"
#include "dac.h"
#include "audio_adc.h"
#include "dac_api.h"
#include "circular_buf.h"
#include "sound_effect_api.h"
#include "errno-base.h"
#include "sdmmc/sd_host_api.h"
#include "adc_drv.h"
#include "decoder_api.h"
#include "app_modules.h"
#include "src_api.h"
#include "echo_api.h"
#include "howling_api.h"
#include "vo_pitch_api.h"
#include "pcm_eq_api.h"
#if VO_CHANGER_EN
#include "voiceChanger_av_api.h"
#endif

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#ifdef LOUDSPEAKER_EN
#define  LOUDSPEAKER_EFFECT ((HOWLING_EN) || (ECHO_EN))


#if (PITCHSHIFT_HOWLING_EN) && (NOTCH_HOWLING_EN)
#if defined(PITCHSHIFT_HOWLING_EN) && (PITCHSHIFT_HOWLING_EN)
#define  SHIFT_FREQ_HOWLING  0                   //移频啸叫抑制
#endif

#if defined(NOTCH_HOWLING_EN) && (NOTCH_HOWLING_EN)
#define  NOTCH_HOWLING       1                   //陷波啸叫抑制
#endif
#define  HOWLING_SEL        NOTCH_HOWLING//SHIFT_FREQ_HOWLING//
#endif

static cbuffer_t cbuf_mic AT(.loudspeaker_data);
static u8 obuf_mic[512]   AT(.loudspeaker_data);  //mic输出缓冲buf

#if LOUDSPEAKER_EFFECT
static cbuffer_t cbuf_dac AT(.loudspeaker_data);
#if defined(NOTCH_HOWLING) && (HOWLING_SEL == NOTCH_HOWLING)
static u8 obuf_dac[1024]   AT(.loudspeaker_data); //dac接收缓冲buf
#else
static u8 obuf_dac[512]   AT(.loudspeaker_data);  //dac接收缓冲buf
#endif
#endif

/* #define LOUDSPEAKER_RATE (16000) */
#define LOUDSPEAKER_RATE (24000)      //扩音模式采样率


/* #if (ECHO_EN && USER_ECHO_CONFIG) */
/* #if (LOUDSPEAKER_RATE == 16000) */
/* #define echo_work_buf_len   3692 */
/* #elif (LOUDSPEAKER_RATE == 24000) */
/* #define echo_work_buf_len   5292 */
/* #endif */
/* u32 echo_work_buf[(echo_work_buf_len + 3) / 4] AT(.echo_data);//echo混响buf长度，采样率改变，buf长度会变 */
/* #endif */

/*sdk需要添加这个定义，代表是否下采回声。*/
/*1代表下采，0代表不下采。下采的话，申请的Buf会减少，但是回声部分的高频也会丢失*/
/* const  int DOWN_S_FLAG = 0; */
/* sdk需要添加这个定义，代表滤波器阶数参数: 配置范围为2<=>8，它的大小跟运算量成正相关 */
/* const  int FRESHIFT_SPEED_MODE_QUALITY = 2; */


typedef struct _effect_obj {
    sound_out_obj first_sound;
    sound_out_obj *last_sound;
} effect_obj;
effect_obj e_obj AT(.loudspeaker_data);

static u16 loudspeaker_msg_filter(u8 key_status, u8 key_num, u8 key_type)
{
    u16 msg = NO_MSG;
    switch (key_type) {
#if KEY_IO_EN
    case KEY_TYPE_IO:
        msg = iokey_msg_mbox_LOUDSPEAKER_table[key_status][key_num];
        break;
#endif
#if KEY_AD_EN
    case KEY_TYPE_AD:
        msg = adkey_msg_mbox_LOUDSPEAKER_table[key_status][key_num];
        break;
#endif

#if KEY_IR_EN
    case KEY_TYPE_IR:
        msg = irff00_msg_LOUDSPEAKER_table[key_status][key_num];
        break;
#endif

    default:
        break;
    }

    return msg;
}

//该函数在adc中断调用，需要放ram
//函数定义不可修改,函数体内不可添加非ram区的接口(例如：打印函数)
AT(.audio_a.text.cache.L2)
void kick_loudsperaker(void *_sound)
{
#if LOUDSPEAKER_EFFECT
    sound_out_obj *sound = (sound_out_obj *)_sound;
    if (sound && sound->p_obuf) {
        u32 size = cbuf_get_data_size(sound->p_obuf);
        if (size >= (64)) {
            sound->enable |= B_DEC_KICK;
            kick_decoder();
        }
    }
#endif
}


// *INDENT-ON*
SET(interrupt(""))
void loudspeaker_soft0_isr()
{
    bit_clr_swi(0);

    sound_out_obj *sound = &e_obj.first_sound;
    if (sound) {
        if (sound->enable & B_DEC_KICK) {
            sound->enable &= ~B_DEC_KICK;
            u32 rlen = 0;
            u32 wlen = 0;
            s16 *data = cbuf_read_alloc(sound->p_obuf, &rlen);
            if (rlen) {
                wlen = sound_output(sound, data, rlen);
            }
            cbuf_read_updata(sound->p_obuf, wlen);
        }
    }
}


#define loudspeaker_sof_isr (IRQ_SOFT0_IDX)
void loudspeaker_obj_init()
{
    /*------------System configuration-----------*/
    Sys_IRInput = 1;
    Sys_Volume = 1;

    /*---------AUX MAIN UI--------------*/
    SET_UI_MAIN(MENU_LSP);
    UI_menu(MENU_LSP);

    memset(&e_obj, 0x0, sizeof(effect_obj));

#if LOUDSPEAKER_EFFECT
    HWI_Uninstall(loudspeaker_sof_isr);
    HWI_Install(loudspeaker_sof_isr, (u32)loudspeaker_soft0_isr, IRQ_DECODER_IP);
#endif
}

void loudspeaker_obj_uninit()
{
#if LOUDSPEAKER_EFFECT
    HWI_Uninstall(loudspeaker_sof_isr);
#endif
}


void *loudspeaker_add_effect(sound_out_obj *p_sound, u32 sr)
{
    sound_out_obj *p_curr_sound = 0;
    sound_out_obj *p_next_sound = 0;
    p_curr_sound = p_sound;

#if HOWLING_EN
#if defined(PITCHSHIFT_HOWLING_EN) && (PITCHSHIFT_HOWLING_EN)
#if (HOWLING_SEL == SHIFT_FREQ_HOWLING)
//移频啸叫抑制
    p_curr_sound = link_pitchshift_howling_sound(p_curr_sound, &cbuf_dac, 0, sr);
#if 0
    HOWLING_PITCHSHIFT_PARM hparm = {0};
    /* hparm.sr       =  sr;  //配置采样率：支持8到48k */
    hparm.ps_parm  = -200; //配置移频系数:建议范围 -300到300
    hparm.effect_v = EFFECT_HOWLING_PS | EFFECT_HOWLING_HE;

    p_curr_sound->enable = 0;
    p_curr_sound->effect = howling_phy(ip_sound->p_obuf, &hparm, sr, (void **)&p_next_sound);
    if (NULL != p_curr_sound->effect) {
        p_curr_sound->enable |= B_DEC_EFFECT;
        p_curr_sound = p_next_sound;
        p_curr_sound->p_obuf = &cbuf_dac;
        p_next_sound = 0;
    } else {
        log_info("shift freq howling init fail\n");
    }
#endif
#endif
#endif

#if defined(NOTCH_HOWLING_EN) && (NOTCH_HOWLING_EN)
#if (HOWLING_SEL == NOTCH_HOWLING)
//陷波啸叫抑制
    p_curr_sound = link_notch_howling_sound(p_curr_sound, &cbuf_dac, 0, sr);
#if 0
//深度是指 陷波器衰减程度，带宽是指 陷波器的频率带宽，（调大调小会影响防啸叫效果以及音质）
//深度范围是0 - 15
//带宽没有限制，一般是越宽防啸叫效果越好，但音质会损伤一点
//算法效果有限，要配合调mic增益以及 模具上做好隔音处理
    NH_PARA_STRUCT nhparm = {0};
    nhparm.depth          = 10;  //深度
    nhparm.bandwidth      = 45; //带宽
    nhparm.sampleRate     = sr;//采样率

    p_curr_sound->enable = 0;
    p_curr_sound->effect = notch_howling_phy(p_sound->p_obuf, &nhparm, (void **)&p_next_sound);
    if (NULL != p_curr_sound->effect) {
        p_curr_sound->enable |= B_DEC_EFFECT;
        p_curr_sound = p_next_sound;
        p_curr_sound->p_obuf = &cbuf_dac;
        p_next_sound = 0;
    } else {
        log_info("notch howling init fail\n");
    }
#endif
#endif
#endif
#endif

#if ECHO_EN
    //echo混响
    p_curr_sound = link_echo_sound(p_curr_sound, &cbuf_dac, 0, sr);
#if 0
    /* ECHO_PARM eparm = {0}; */
    /* eparm.echo_parm_obj.decayval          	= 60;  //decay(0~70)回声衰减比 */
    /* eparm.echo_parm_obj.delay             	= 200; //回声延时0~300ms :范围0到max_ms，超过max_ms的话，会当成max_ms的 */
    /* eparm.echo_parm_obj.energy_vad_threshold = 512; //mute阈值 */
    /* eparm.echo_parm_obj.direct_sound_enable  = 1;   //如果运算结果需要叠加干声，则这个置成1，否则置0 */
    /* eparm.echo_fix_parm.max_ms               = 100; //最大延时100ms */
    /* eparm.echo_fix_parm.sr                   = sr;  //采样率 */
    /* eparm.echo_fix_parm.wetgain              = 3000;//湿声增益 */
    /* eparm.echo_fix_parm.drygain              = 4096;//干声增益，如果direct_sound_enable是0，则这个参数无效，因为不叠加干声了 */
    /* eparm.ptr = echo_work_buf; */
    /* eparm.ptr_len = sizeof(echo_work_buf); */
    /* p_curr_sound->enable = 0; */
    /* p_curr_sound->effect = echo_phy(ip_sound->p_obuf, &eparm, (void **)&p_next_sound); */
    /* if (NULL != p_curr_sound->effect) { */
    /* p_curr_sound->enable |= B_DEC_EFFECT; */
    /* p_curr_sound = p_next_sound; */
    /* p_curr_sound->p_obuf = &cbuf_dac; */
    /* p_next_sound = 0; */
    /* } else { */
    /* log_info("echo init fail\n"); */
    /* } */
#endif
#endif

    /* 变音、echo与pcm_eq互斥，三者同时只可打开一个 */
#if VO_PITCH_EN
    /* p_curr_sound = link_voice_pitch_sound(p_curr_sound, &cbuf_dac, 0, VP_CMD_ROBOT); */
#endif

#if VO_CHANGER_EN //测试用
    /* p_curr_sound = link_voice_changer_sound(p_curr_sound, &cbuf_dac, 0, sr); */
#endif

#if PCM_EQ_EN
    /* p_curr_sound = link_pcm_eq_sound(p_curr_sound, &cbuf_dac, 0, sr, 1); */
#endif


    return p_curr_sound;
}


int loudspeaker_init(void)
{
    u32 err;
    sound_out_obj *fist_sound = &e_obj.first_sound;
    /*****************ADC init******************/
    cbuf_init(&cbuf_mic, &obuf_mic[0], sizeof(obuf_mic));
#if LOUDSPEAKER_EFFECT
    cbuf_init(&cbuf_dac, &obuf_dac[0], sizeof(obuf_dac));
#endif
    fist_sound->p_obuf = &cbuf_mic;
    err = audio_adc_init_api(LOUDSPEAKER_RATE, ADC_MIC, BIT(1));
    regist_audio_adc_channel(fist_sound, (void *)kick_loudsperaker); //注册到ADC;

    /**************** effect init ***************/
    e_obj.last_sound = loudspeaker_add_effect(fist_sound, LOUDSPEAKER_RATE);

    /*****************DAC init******************/
    dac_sr_api(LOUDSPEAKER_RATE);
    /* dac_init_api(LOUDSPEAKER_RATE); */
    regist_dac_channel(e_obj.last_sound, kick_decoder); //注册到DAC

    /*****************ADC en******************/
    audio_adc_enable(MIC_PGA_G);
    u8 mic_gain = 7;
    SFR(JL_ADDA->ADA_CON2, 19, 5, mic_gain);//此处临时设置mic增益,后续等接口完善，再替换
    fist_sound->enable |= B_DEC_RUN_EN;
    return 0;
}

void loudspeaker_off(void)
{
    e_obj.first_sound.enable = 0;
    e_obj.last_sound->enable = 0;
    audio_adc_off_api();
    unregist_audio_adc_channel(&e_obj.first_sound);
    unregist_dac_channel(e_obj.last_sound);
    dac_sr_api(SR_DEFAULT);
}




/*----------------------------------------------------------------------------*/
/**@brief   loudspeaker 模式主循环
   @param   void
   @return  void
   @author
   @note    void loudspeaker_loop(void)
*/
/*----------------------------------------------------------------------------*/
void loudspeaker_loop(void)
{
    u16 msg;
    u8 mute = 0;
    dac_mute(0);

    while (1) {
        msg = app_get_msg();
        bsp_loop();
        switch (msg) {
        case MSG_MUTE:
            if (mute) {
                mute = 0;
                dac_mute(0);
            } else {
                mute = 1;
                dac_mute(1);
            }
            break;
            /* case MSG_AUX_IN: */
            /* break; */
            /* case MSG_AUX_OUT : */
            /* work_mode++; */
            /* log_info("mode ++ %d\n", work_mode); */
            /* return; */
#if ECHO_EN
        case MSG_ECHO_EFF: {
            ECHO_PARM_SET parm = {0};
            parm.decayval = 60;              //decay(0~70)回声衰减比
            parm.delay = 100;                //回声延时0~300msms
            parm.energy_vad_threshold =  512;//mute阈值
            parm.direct_sound_enable =  1;   //如果运算结果需要叠加干声，则这个置成1，否则置0
            echo_parm_update(&parm);
        }
        break;
#endif
        case MSG_MUSIC_NEW_DEVICE_IN:
            if (work_mode == LOUDSPEAKER_MODE) {
                work_mode = MUSIC_MODE;
            }
        case MSG_CHANGE_WORK_MODE:
            log_info("Exit ====LOUDSPERKER==== mode\n");
            return;

        case MSG_500MS:
            LED_FADE_OFF();
            UI_menu(MENU_MAIN);
        default:
            ap_handle_hotkey(msg);
            break;
        }
    }

}


/*----------------------------------------------------------------------------*/
/**@brief   loudspeaker 模式
   @param   void
   @return  void
   @author
   @note    void loudspeaker_mode(void)
*/
/*----------------------------------------------------------------------------*/
void loudspeaker_mode(void)
{
    work_mode_save();

    key_table_sel(loudspeaker_msg_filter);
    SET_UI_MAIN(MENU_AUX_MAIN);
    UI_menu(MENU_AUX_MAIN);
    loudspeaker_obj_init();
    loudspeaker_init();
    loudspeaker_loop();
    loudspeaker_off();
    loudspeaker_obj_uninit();
}
#endif
