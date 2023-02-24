/*--------------------------------------------------------------------------*/
/**@file    Line_in_mode.c
   @brief   Line-In 模式主循环
   @details
   @author
   @date
   @note
*/
/*----------------------------------------------------------------------------*/
#include "typedef.h"
#include "line_in_mode.h"
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

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#ifdef LINEIN_EN

static cbuffer_t cbuf_aux_o AT(.aux_data);
static u8 obuf_aux_o[1024] AT(.aux_data) ;

static sound_out_obj aux_sound;

static volatile u8 linein_online_flg = 0;
void line_in_det(void)
{
    if (!sdx_dev_send_suspend_event(SDX_SUSPEND_EVENT_LINEIN)) {
        void line_in_det1(void);
        line_in_det1();
    }
}

void line_in_det1(void)
{
#define LINEIN_THRESHOLD		930
    static u8 status_keep_cnt = 0;

#if (LINEIN_DET_MULT_SD && TFG_SD_EN)
    if (sd_io_suspend(0, 0) == 0)
#endif
    {
        gpio_set_die(adc_ch2port(ADC_CH_PA2), 0);
        gpio_set_direction(adc_ch2port(ADC_CH_PA2), 1);
        gpio_set_pull_down(adc_ch2port(ADC_CH_PA2), 0);
        gpio_set_pull_up(adc_ch2port(ADC_CH_PA2), 1);
        int value = adc_scan_once(ADC_CH_PA2);
        gpio_set_die(adc_ch2port(ADC_CH_PA2), 1);
        if (value == -1) {
#if (LINEIN_DET_MULT_SD && TFG_SD_EN)
            sd_io_resume(0, 0);
#endif
            return;
        }
        //log_info("line value:%d \n",value);

        if ((value < LINEIN_THRESHOLD) && !linein_online_flg) {
            //online
            status_keep_cnt++;
            if (status_keep_cnt >= 2) {
                linein_online_flg = 1;
                log_info("linein online \n");
                post_event(EVENT_AUX_IN);
            }
        } else if ((value >= LINEIN_THRESHOLD) && linein_online_flg) {
            status_keep_cnt++;
            if (status_keep_cnt >= 2) {
                linein_online_flg = 0;
                log_info("linein off \n");
                post_event(EVENT_AUX_OUT);
            }
        } else {
            status_keep_cnt = 0;
        }

#if (LINEIN_DET_MULT_SD && TFG_SD_EN)
        sd_io_resume(0, 0);
#endif
    }
}

u8 line_in_online(void)
{
    return linein_online_flg;
}

static u16 line_in_msg_filter(u8 key_status, u8 key_num, u8 key_type)
{
    u16 msg = NO_MSG;
    switch (key_type) {
#if KEY_IO_EN
    case KEY_TYPE_IO:
        msg = iokey_msg_mbox_line_in_table[key_status][key_num];
        break;
#endif
#if KEY_AD_EN
    case KEY_TYPE_AD:
        msg = adkey_msg_mbox_line_in_table[key_status][key_num];
        break;
#endif

#if KEY_IR_EN
    case KEY_TYPE_IR:
        msg = irff00_msg_line_in_table[key_status][key_num];
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

/*----------------------------------------------------------------------------*/
/**@brief   Line-In 模式信息初始化
   @param   void
   @return  void
   @author
   @note    void Line_in_info_init(void)
*/
/*----------------------------------------------------------------------------*/
void Line_in_info_init(void)
{
    /*------------System configuration-----------*/
    Sys_IRInput = 0;
    Sys_Volume = 1;

    /*---------AUX MAIN UI--------------*/
    SET_UI_MAIN(MENU_AUX_MAIN);
    UI_menu(MENU_AUX_MAIN);
    key_table_sel(line_in_msg_filter);
}


/*----------------------------------------------------------------------------*/
/**@brief   Line-In 模式主循环
   @param   void
   @return  void
   @author
   @note    void Line_in_loop(void)
*/
/*----------------------------------------------------------------------------*/
void Line_in_mode_loop(void)
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
        case MSG_AUX_IN:
            break;
        case MSG_AUX_OUT :
            work_mode++;
            return;
        case MSG_MUSIC_NEW_DEVICE_IN:
            if (work_mode == AUX_MODE) {
                work_mode = MUSIC_MODE;
            }
        case MSG_CHANGE_WORK_MODE:
            //deg_puts("Exit RTC mode\n");
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
/**@brief   Line-In 模式
   @param   void
   @return  void
   @author
   @note    void Line_in_mode(void)
*/
/*----------------------------------------------------------------------------*/
void Line_in_mode(void)
{
    work_mode_save();
    Line_in_info_init();
    linein_init();
    Line_in_mode_loop();
    linein_off();
}

#endif
