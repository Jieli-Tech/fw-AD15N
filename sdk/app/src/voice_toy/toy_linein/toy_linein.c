#include "toy_linein.h"
#include "common.h"
#include "msg.h"
#include "circular_buf.h"
#include "bsp_loop.h"
#include "dac_api.h"
#include "audio_adc.h"
#include "app_modules.h"
#include "toy_main.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[aux]"
#include "log.h"

#if defined(LINEIN_MODE_EN) && (LINEIN_MODE_EN)
extern const char MIC_PGA_G;
cbuffer_t cbuf_digital_linein        AT(.aux_data);
u16 obuf_digital_linein[1024 / 2]    AT(.aux_data);
sound_out_obj digital_linein_sound   AT(.aux_data);


void toy_linein_app(void)
{
    key_table_sel(linein_key_msg_filter);
    log_info("digital linein init!\n");
    memset(&digital_linein_sound, 0, sizeof(digital_linein_sound));
    cbuf_init(&cbuf_digital_linein, &obuf_digital_linein[0], sizeof(obuf_digital_linein));
    digital_linein_sound.p_obuf = &cbuf_digital_linein;

    u32 sr = dac_sr_read();
    /* log_info("dac_sr %d\n", sr); */
    dac_sr_api(24000);
    audio_adc_init_api(24000, ADC_LINE_IN, BIT(1));//24k采样率
    regist_dac_channel(&digital_linein_sound, NULL);
    regist_audio_adc_channel(&digital_linein_sound, NULL);

    audio_adc_enable(MIC_PGA_G);
    digital_linein_sound.enable |= B_DEC_RUN_EN | B_DEC_FIRST;

    aux_test_audio();

    int msg[2];
    u32 err;
    u8 mute = 0;
    while (1) {
        err = get_msg(2, &msg[0]);
        bsp_loop();

        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
            log_info("get msg err 0x%x\n", err);
        }
        switch (msg[0]) {
        case MSG_PP:
            if (mute == 0) {
                log_info("MUTE\n");
                mute = 1;
                dac_mute(1);
                /* pa_mute(1); */
            } else {
                log_info("UNMUTE\n");
                mute = 0;
                dac_mute(0);
                /* pa_mute(0); */
            }
            break;
        case MSG_CHANGE_WORK_MODE:
            goto __linein_app_exit;
        case MSG_500MS:
        default:
            common_msg_deal(&msg[0]);
            break;
        }
    }
__linein_app_exit:
    if (0 != mute) {
        dac_mute(0);
        /* pa_mute(0); */
    }
    digital_linein_sound.enable &= ~B_DEC_RUN_EN;
    audio_adc_disable();
    unregist_audio_adc_channel(&digital_linein_sound);
    unregist_dac_channel(&digital_linein_sound);
    audio_adc_off_api();
    dac_sr_api(sr);
    key_table_sel(NULL);
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

#endif

