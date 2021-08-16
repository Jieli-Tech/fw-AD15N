/***********************************Jieli tech************************************************
  File : speak_api.c
  By   : liujie
  Email: liujie@zh-jieli.com
  date : 2021年6月15日
********************************************************************************************/
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "audio.h"
#include "dac.h"
#include "audio_adc.h"
#include "circular_buf.h"
#include "dac_api.h"
#include "sound_effect_api.h"
#include "errno-base.h"
#include "sine_play.h"
#include "src_api.h"
#if VO_PITCH_EN
#include "vo_pitch_api.h"
#endif
#if PCM_EQ_EN
#include "pcm_eq_api.h"
#endif
#include "echo_api.h"
#include "notch_howling_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[speak_api]"
#include "debug.h"

extern const char MIC_PGA_G;

cbuffer_t cbuf_aas_o AT(.speaker_data);
u8 obuf_aas_o[512]  AT(.speaker_data);

cbuffer_t cbuf_ads_o AT(.speaker_data);
u8 obuf_ads_o[1024]  AT(.speaker_data);
/* sound_out_obj audio_dac_sound; */

typedef struct __audio_adc_speaker {
    sound_out_obj  sound;
    sound_out_obj  *p_dac_sound;
    EFFECT_OBJ    *p_src;            // p_curr_sound->effect;
    EFFECT_OBJ    *p_vp;            // p_curr_sound->effect;
} __aa_speaker;

__aa_speaker aa_speaker ;//AT(.aux_data);


AT(.adc_oput_code)
void kick_sound(void *_sound)
{
    sound_out_obj *sound = (sound_out_obj *)_sound;
    if (sound && sound->p_obuf) {
        u32 size = cbuf_get_data_size(sound->p_obuf);
        if (size >= 64) {
            sound->enable |= B_DEC_KICK;
            bit_set_swi(1);
        }
    }
}

SET(interrupt(""))
void speaker_soft1_isr()
{
    bit_clr_swi(1);

    sound_out_obj *sound = &aa_speaker.sound;
    if (NULL == sound) {
        return;
    }

    if (0 == (sound->enable & B_DEC_RUN_EN)) {
        return;
    }

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

void audio_adc_speaker_start(void)
{
    if (aa_speaker.sound.enable & B_DEC_RUN_EN) {
        log_info("SPEAKER START ERR !!! \n");
        return;
    }

    log_info("SPEAKER START !!! \n");

    HWI_Uninstall(IRQ_SOFT1_IDX);
    HWI_Install(IRQ_SOFT1_IDX, (u32)speaker_soft1_isr, IRQ_DECODER_IP);

    memset(&aa_speaker.sound, 0, sizeof(aa_speaker.sound));
    cbuf_init(&cbuf_aas_o, &obuf_aas_o[0], sizeof(obuf_aas_o));
    aa_speaker.sound.p_obuf = &cbuf_aas_o;
    audio_adc_init_api(24000, ADC_MIC, BIT(1));

    u32 adc_sr = read_audio_adc_sr();
    u32 dac_sr = dac_sr_read();


    sound_out_obj *p_curr_sound = 0;
    p_curr_sound = &aa_speaker.sound;

    cbuf_init(&cbuf_ads_o, &obuf_ads_o[0], sizeof(obuf_ads_o));


#if  HOWLING_EN     //防止啸叫
    /* p_curr_sound = link_notch_howling_sound(p_curr_sound, &cbuf_ads_o, 0, adc_sr); */
#endif

#if ECHO_EN         //混响
    /* p_curr_sound = link_echo_sound(p_curr_sound, &cbuf_ads_o, 0, adc_sr); */
#endif
#if VO_PITCH_EN     //多种音效集合库：机器人、rap、两种变速变调；
    /* p_curr_sound = link_voice_pitch_sound(p_curr_sound, &cbuf_ads_o, (void **)&aa_speaker.p_vp, VP_CMD_ROBOT); */
#endif

#if PCM_EQ_EN
    p_curr_sound = link_pcm_eq_sound(p_curr_sound, &cbuf_ads_o, 0, adc_sr, 1);
#endif


    if (adc_sr != dac_sr) {
        p_curr_sound = link_src_sound(p_curr_sound, &cbuf_ads_o, (void **)&aa_speaker.p_src, adc_sr, dac_sr);
    };
    if (&aa_speaker.sound == p_curr_sound) {
        regist_audio_adc_channel(&aa_speaker.sound, (void *) NULL);
    } else {
        regist_audio_adc_channel(&aa_speaker.sound, (void *) kick_sound);

    }
    regist_dac_channel(p_curr_sound, NULL);//注册到DAC;
    aa_speaker.p_dac_sound = p_curr_sound;

    /* aa_speaker.p_sound = &aa_speaker.sound; */


    audio_adc_enable(MIC_PGA_G);
    aa_speaker.sound.enable |= B_DEC_RUN_EN;
}

void audio_adc_speaker_reless(void)
{
    if (aa_speaker.sound.enable & B_DEC_RUN_EN) {
        log_info("SPEAKER RELESS !!! \n");
        aa_speaker.sound.enable &= ~B_DEC_RUN_EN;
        unregist_audio_adc_channel(&aa_speaker.sound);

        unregist_dac_channel(aa_speaker.p_dac_sound);

        if (NULL != aa_speaker.p_src) {
            src_reless((void **)&aa_speaker.p_src);
        }

        memset(&aa_speaker.sound, 0, sizeof(aa_speaker.sound));
    }

}













