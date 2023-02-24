
/* #include "cpu.h" */
/* #include "config.h" */
/* #include "typedef.h" */
/* #include "audio.h" */
/* #include "dac.h" */
/* #include "audio_adc.h" */
/* #include "msg.h" */
/* #include "circular_buf.h" */
/* #include "dac_api.h" */
/* #include "errno-base.h" */
/* #include "app_config.h" */
#include "usb/device/msd.h"
#include "usb/device/usb_stack.h"
#include "usb/device/uac_audio.h"
#include "usb/usr/usb_audio_interface.h"

#if  TCFG_PC_ENABLE

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[UAC]"
#include "log.h"
#include "uart.h"

EFFECT_OBJ *usb_src_obj = NULL;

static u32 sr_curr = 0;
static u8 pe_cnt  = 0;
static u8 pe5_cnt  = 0;
static u8 pe5_step  = 2;
static u16 pe_inc_data = 0;
static u16 pe_sub_data = 0;
static u8 last_pe = 0xff;
static u16 last_sr = 0;
static s16 x_step = 5;
static u32 sync_cnt = 0;
static void uac_sync_init(s32 sr)
{
    sr_curr = 0;
    pe_cnt  = 0;
    pe5_cnt  = 0;
    pe5_step  = 2;
    pe_inc_data = 0;
    pe_sub_data = 0;
    last_pe = 0xff;
    last_sr = 0;
    x_step = sr * 5 / 10000;

}

static u32 uac_sr_milli(u32 sr)
{
    u32 tmp;
    if (sr > sr_curr) {
        tmp = sr - sr_curr;
    } else {
        tmp = sr_curr - sr ;
    }
    u32 milli = tmp * 1000 / sr;
    return milli;
}

static bool uac_is_need_sync(void)
{
    // u32 in_sr;
    /* log_info("O\n"); */
    if (NULL == usb_src_obj) {
        return 0;
    }
    /*
    in_sr = uac_speaker_sound();
    u32 milli = uac_sr_milli(in_sr);
    if (milli > 50) {
        return 0;
    }
    */


    /* log_info("A\n"); */
    sound_in_obj *p_src_si = usb_src_obj->p_si;
    if (NULL == p_src_si) {
        return 0;
    }
    /* log_info("B\n"); */
    SRC_STUCT_API *p_ops =  p_src_si->ops;
    if (NULL == p_ops) {
        return 0;
    }
    return 1;
}



void uac_1s_sync(void)
{
    u32 sr_1s = uac_speaker_stream_1s();
    sr_curr = sr_1s;
    if (0 == sr_1s) {
        return;
    }
    if (0 == uac_is_need_sync()) {
        return;
    }
    sound_in_obj *p_src_si = usb_src_obj->p_si;
    SRC_STUCT_API *p_ops =  p_src_si->ops;
    if (0 != last_sr) {
        u32 milli = uac_sr_milli(last_sr);
        if (milli < 4) {
            return;
        }
    }
    log_info(" 1S SR %d\n", sr_1s);
    /* p_ops->config(p_src_si->p_dbuf, SRC_CMD_INSR_SET, (void *)sr_1s); */
    last_sr = sr_1s;
}


static void uac_inc_sync_pe_reset(void)
{
    pe5_step += 4;
    pe5_cnt = pe5_step;
    pe_inc_data = 0;
    pe_sub_data = 0;
    pe_cnt = 0;
    last_pe = 0xff;
}
void uac_inc_sync(void)
{
    if (0 == uac_is_need_sync()) {
        return;
    }
    sound_in_obj *p_src_si = usb_src_obj->p_si;
    SRC_STUCT_API *p_ops =  p_src_si->ops;

    u32 uac_spk_data = uac_speaker_stream_size();
    u32 uac_spk_size = uac_speaker_stream_length();
    u32 percent = (uac_spk_data * 100) / uac_spk_size;
    char c = 0;
    s32 step = 0;
    if (percent > 60) {
        if (0 == pe5_cnt) {
            c = '+';
            step = x_step;
            uac_inc_sync_pe_reset();
        }
        sync_cnt = 0;
    } else if (percent < 40) {
        if (0 == pe5_cnt) {
            c = '-';
            step = 0 - x_step;
            uac_inc_sync_pe_reset();
        }
        sync_cnt = 0;
    } else {
        if (sync_cnt > 60) {
            sync_cnt = 0;
            if (x_step > 2) {
                /* u32 tmp  = x_step / 6; */
                /* x_step -= tmp ; */
            }
            /* c = '='; */
        }
        sync_cnt++;
        pe5_step = 2;
    }
    if (0 != pe5_cnt) {
        pe5_cnt--;
    }
#if 1
    /* pe_cnt++; */
    /* if (pe_cnt >= 100) { */
    /* pe_cnt = 0; */
    /* pe_inc_data = 0; */
    /* pe_sub_data = 0; */
    /* } */

    if (last_pe <= 100) {
        if (percent < last_pe) {
            pe_sub_data += (last_pe - percent);
        } else if (percent > last_pe) {
            pe_inc_data += (percent - last_pe);
        }

        /* if (0 == (pe_cnt % 30)) { */
        /* pe_inc_data = 0; */
        /* pe_sub_data = 0; */

        /* } */
        if (pe_sub_data > pe_inc_data) {
            pe_sub_data -= pe_inc_data;
            pe_inc_data = 0;
            if (pe_sub_data > 14) {
                c = 's';
                step =  0 - x_step;
                pe_sub_data = 0;
            }
        } else {
            pe_inc_data -= pe_sub_data;
            pe_sub_data = 0;
            if (pe_inc_data > 14) {
                c = 'p';
                step = x_step;
                pe_inc_data = 0;
            }
        }

    }
#endif
    if (c != 0) {
        log_char(c);
        /* log_char( (u8)0xff & percent ); */
    }
    if (0 != step) {
        p_ops->config(p_src_si->p_dbuf, SRC_CMD_INSR_INC_SET, (void *)step);
    }

    last_pe = percent;
}

void usb_slave_sound_close(sound_out_obj *p_sound)
{
    log_info("usb slave sound off\n");
    p_sound->enable &= ~B_DEC_RUN_EN;
    unregist_dac_channel(p_sound);
    usb_src_obj = NULL;
    if (NULL != p_sound->effect) {
        src_reless(&p_sound->effect);
    } else {
        log_info("usb slave sound effect null\n");
    }
}

void usb_slave_sound_open(sound_out_obj *p_sound, u32 sr)
{
    sound_out_obj *p_curr_sound = 0;
    sound_out_obj *p_next_sound = 0;
    uac_sync_init(sr);
    if (0 != sr) {
        p_curr_sound = p_sound;
        void *cbuf_o = p_curr_sound->p_obuf;
        p_curr_sound = link_src_sound(p_curr_sound, cbuf_o, (void **)&usb_src_obj, sr, 32000);
#if 0
        p_curr_sound->effect = src_api(p_sound->p_obuf, sr, 32000, (void **)&p_next_sound);
        if (NULL != p_curr_sound->effect) {
            /* p_dec->src_effect = p_curr_sound->effect; */
            p_curr_sound->enable |= B_DEC_EFFECT;
            p_curr_sound = p_next_sound;
            /* log_info("src init succ\n"); */
        } else {
            log_info("src init fail\n");
        }
        usb_src_obj = p_sound->effect;
#endif
        regist_dac_channel(p_sound, NULL);//注册到DAC;
        p_sound->enable |=  B_DEC_RUN_EN;
    }
}


#if USB_DEVICE_CLASS_CONFIG & MIC_CLASS
sound_out_obj usb_mic_sound;
u8 obuf_usb_mic_o[1024] ;
cbuffer_t cbuf_usb_mic_o;
void usb_mic_init(void)
{
    log_info("usb mic init\n");
    u32 err;
    memset(&usb_mic_sound, 0, sizeof(usb_mic_sound));

    cbuf_init(&cbuf_usb_mic_o, &obuf_usb_mic_o[0], sizeof(obuf_usb_mic_o));
    usb_mic_sound.p_obuf = &cbuf_usb_mic_o;

    //16k 采样,单声道
    err = audio_adc_init_api(MIC_AUDIO_RATE, ADC_MIC, BIT(0)); //PA13 -> mic
    regist_audio_adc_channel(&usb_mic_sound, NULL); //注册到ADC;
    audio_adc_enable(14);
    usb_mic_sound.enable |= B_DEC_RUN_EN;
}
void usb_mic_uninit()
{
    log_info("usb slave mic off \n");
    usb_mic_sound.enable &= ~B_DEC_RUN_EN;
    audio_adc_off_api();
    unregist_audio_adc_channel(&usb_mic_sound); //卸载ADC
    audio_adc_disable();
}
#endif

#endif
