#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "audio.h"
#include "dac.h"
#include "audio_adc.h"
#include "msg.h"
#include "bsp_loop.h"
#include "circular_buf.h"
#include "dac_api.h"
#include "sound_effect_api.h"
#include "errno-base.h"
#include "sine_play.h"

#if SPEAKER_EN
#include "speak_api.h"
#endif

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"

cbuffer_t cbuf_aux_o AT(.aux_data);
u8 obuf_aux_o[1024] AT(.aux_data) ;

sound_out_obj aux_sound;


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
void aux_init(void)
{
    u32 err;
    memset(&aux_sound, 0, sizeof(aux_sound));
    cbuf_init(&cbuf_aux_o, &obuf_aux_o[0], sizeof(obuf_aux_o));
    aux_sound.p_obuf = &cbuf_aux_o;

    err = audio_adc_init_api(24000, ADC_MIC, BIT(1));
    /* dac_sr_api(24000); */
    /* dac_init_api(24000); */
    aux_test_audio();
    regist_dac_channel(&aux_sound, NULL);//注册到DAC;
    regist_audio_adc_channel(&aux_sound, NULL); //注册到DAC;
    audio_adc_enable(MIC_PGA_G);
    aux_sound.enable |= B_DEC_RUN_EN;
}
void aux_off(void)
{
    aux_sound.enable = 0;
    audio_adc_off_api();
    unregist_audio_adc_channel(&aux_sound);
    unregist_dac_channel(&aux_sound);
    /* dac_sr_api(32000); */
}


#define D_MIC_PGA_G(g)    SFR(JL_ADDA->ADA_CON2, 19, 5, gain)

extern void simple_next(void);

void aux_demo(void)
{

    dac_vol(0, 31);
    dac_sr_api(24000);
    /* dac_init_api(24000); */
    /* dac_vol(0, 31); */
    /* aux_init(); */
    dac_fade_in_api();
    audio_adc_speaker_start();
    int msg[2];
    char c;
    while (1) {

        get_msg(2, &msg[0]);

        if (NO_MSG == msg[0]) {
            c = get_byte();

            if (0 != c) {
                d_key_voice_kick();
                if ('N' == c) {
                    aux_off();
                    simple_next();
                    delay_10ms(10);
                    return ;
                } else if ('n' == c) {
                    aux_off();
                } else if ('+' == c) {
                    msg[0] = MSG_VOL_UP;
                } else if ('-' == c) {
                    msg[0] = MSG_VOL_DOWN;
                }


                u32 gain = 0xff;
                if ((c >= '0') && (c <= '9')) {
                    gain = c - '0';
                }
                if ((c >= 'a') && (c <= 'e')) {
                    gain = c - 'a' + 10;
                }
                if ((c > 'A') && (c <= 'E')) {
                    gain = c - 'A' + 10;
                }
#if SPEAKER_EN
                else if ('o' == c) {
                    audio_adc_speaker_start();
                } else if ('c' == c) {
                    audio_adc_speaker_reless();
                }
#endif
                if (gain <= 14) {
                    log_info(" gain : %d", gain);
                    D_MIC_PGA_G(gain);
                }
                if ('x' == c) {
                    log_info(" 6db\n");
                    SFR(JL_ADDA->ADA_CON2, 2, 1, 0);      //MIC_0db_11V   0:6db   1:0db 增益需要放出去
                } else if ('y' == c) {
                    log_info(" 0db\n");
                    SFR(JL_ADDA->ADA_CON2, 2, 1, 1);      //MIC_0db_11V   0:6db   1:0db 增益需要放出去
                }
            }
        }
        bsp_loop();
        switch (msg[0]) {
        case MSG_500MS:
            wdt_clear();
            /* log_char('5'); */
            break;
        case MSG_VOL_UP:
            dac_vol('+', 255);
            break;
        case MSG_VOL_DOWN:
            dac_vol('-', 255);
            break;
        default:
            break;
        }
        ;
    }

}




