#include "config.h"
#include "msg.h"
#include "app_mg/app_mg.h"
#include "music_api.h"
#include "audio.h"
#include "dac.h"
#include "audio_adc.h"
#include "msg.h"
#include "circular_buf.h"
#include "dac_api.h"
#include "sound_effect_api.h"
#include "errno-base.h"
#include "app_linein.h"

#if AUX_EN

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

cbuffer_t cbuf_aux_o AT(.aux_data);
u8 obuf_aux_o[1024] AT(.aux_data) ;

sound_out_obj aux_sound;

static u16 linein_msg_filter(u8 key_status, u8 key_num, u8 key_type)
{
    u16 msg = NO_MSG;
    switch (key_type) {
#if KEY_IO_EN
    case KEY_TYPE_IO:
        msg = iokey_msg_linein_table[key_status][key_num];
        break;
#endif
#if KEY_AD_EN
    case KEY_TYPE_AD:
        msg = adkey_msg_linein_table[key_status][key_num];
        break;
#endif

    default:
        break;
    }

    return msg;
}

void aux_test_audio(void)
{
    log_info(" JL_ADDA->DAC_CON0    0x%x;",   JL_ADDA->DAC_CON0);
    log_info(" JL_ADDA->DAC_CON1    0x%x;",   JL_ADDA->DAC_CON1);
    log_info(" JL_ADDA->DAC_TRM     0x%x;",   JL_ADDA->DAC_TRM);
    log_info(" JL_ADDA->DAC_ADR     0x%x;",   JL_ADDA->DAC_ADR);
    log_info(" JL_ADDA->DAC_LEN     0x%x;",   JL_ADDA->DAC_LEN);
    log_info(" JL_ADDA->DAC_COP     0x%x;",   JL_ADDA->DAC_COP);
    log_info(" JL_ADDA->DAC_DTB     0x%x;",   JL_ADDA->DAC_DTB);
    log_info(" JL_ADDA->ADC_CON     0x%x;",   JL_ADDA->ADC_CON);
    log_info(" JL_ADDA->ADC_ADR     0x%x;",   JL_ADDA->ADC_ADR);
    log_info(" JL_ADDA->ADC_LEN     0x%x;",   JL_ADDA->ADC_LEN);
    log_info(" JL_ADDA->RAM_SPD     0x%x;",   JL_ADDA->RAM_SPD);
    log_info(" JL_ADDA->DAA_CON0    0x%x;",   JL_ADDA->DAA_CON0);
    log_info(" JL_ADDA->DAA_CON1    0x%x;",   JL_ADDA->DAA_CON1);
    log_info(" JL_ADDA->ADA_CON0    0x%x;",   JL_ADDA->ADA_CON0);
    log_info(" JL_ADDA->ADA_CON1    0x%x;",   JL_ADDA->ADA_CON1);
    log_info(" JL_ADDA->ADA_CON2    0x%x;\n",   JL_ADDA->ADA_CON2);
}

extern const char MIC_PGA_G;
int linein_init(void)
{
    u32 err;
    memset(&aux_sound, 0, sizeof(aux_sound));
    cbuf_init(&cbuf_aux_o, &obuf_aux_o[0], sizeof(obuf_aux_o));
    aux_sound.p_obuf = &cbuf_aux_o;

    ///BIT(0):ch0 PA13; BIT(1):ch1 PA14
    err = audio_adc_init_api(24000, ADC_LINE_IN, BIT(1));
    dac_sr_api(24000);
    /* dac_init_api(24000); */
    aux_test_audio();
    regist_dac_channel(&aux_sound, NULL); //注册到DAC;
    regist_audio_adc_channel(&aux_sound, NULL); //注册到DAC;
    audio_adc_enable(MIC_PGA_G);
    aux_sound.enable |= B_DEC_RUN_EN;
    return 0;
}
void linein_off(void)
{
    aux_sound.enable = 0;
    audio_adc_off_api();
    unregist_audio_adc_channel(&aux_sound);
    unregist_dac_channel(&aux_sound);
    dac_sr_api(SR_DEFAULT);
}


static int app_linein_init(void *param)
{
    key_table_sel((void *)linein_msg_filter);

    return linein_init();
}

static int app_linein_close(void)
{
    log_info("app_linein_close \n");
    linein_off();
    return 0;
}

int app_linein(void *param)
{
    int msg[2];

    log_info("app_linein \n");
    if (app_linein_init(param) != 0) {
        log_error("app_linein init error !!! \n");
        app_switch_next();
        return 0;
    }

    while (1) {
        get_msg(2, &msg[0]);
        bsp_loop();

        if (common_msg_deal(msg) != (-1)) {
            continue;
        }

        switch (msg[0]) {

        case MSG_500MS:
            //busy
            idle_check_deal(1);
            break;

        case MSG_NEXT_MODE:
            app_switch_next();
            break;
        case MSG_APP_SWITCH_ACTIVE:
            if (app_switch_en()) {
                goto _app_switch;
            }
            break;
#ifdef USB_DEVICE_EN
        case MSG_USB_PC_IN:
        case MSG_PC_IN:
            app_switch_to_usb();
            break;

#endif

        default:
            break;
        }
    }

_app_switch:
    app_linein_close();
    return 0;
}

#endif
