/*--------------------------------------------------------------------------*/
/**@file    REC_mode.c
   @brief   REC 模式主循环
   @details
   @author
   @date
   @note
*/
/*----------------------------------------------------------------------------*/
#include "typedef.h"
#include "rec_mode.h"
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
#include "encoder_mge.h"
#include "mp3_encoder.h"
#include "rec_api.h"
#include "vfs.h"
#include "rec_mode.h"
#include "common/ui/ui_api.h"
#include "device.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#ifdef REC_ENABLE
typedef enum {
    ENC_NULL = 0,
    ENC_ING,

} ENC_STATUS;

volatile u8 e_status = 0;
static enc_file_info_t *enc_info = NULL;
static void *pfile = 0;
static volatile u8 enc_bp_save_flg = 0;//0:退出rec时候不保存文件断点到音乐播放。1：保存当前录音文件断点去music播放。

static const char *rec_folder = {"/JL_REC"};
static const char *rec_filename = "AD14****.mp3";

static u16 rec_msg_filter(u8 key_status, u8 key_num, u8 key_type)
{
    u16 msg = NO_MSG;
    switch (key_type) {
#if KEY_IO_EN
    case KEY_TYPE_IO:
        msg = iokey_msg_mbox_rec_table[key_status][key_num];
        break;
#endif
#if KEY_AD_EN
    case KEY_TYPE_AD:
        msg = adkey_msg_mbox_rec_table[key_status][key_num];
        break;
#endif

#if KEY_IR_EN
    case KEY_TYPE_IR:
        msg = irff00_msg_rec_table[key_status][key_num];
        break;
#endif

    default:
        break;
    }

    return msg;
}

static void encode_stop(void)
{
    if (e_status == ENC_ING) {
        log_info("ing stop_encode\n");
        stop_encode(pfile, 0);
        enc_file_close(enc_info, enc_bp_save_flg);
        if (enc_info && enc_info->device_hdl) {
            dev_close(enc_info->device_hdl);
        }
        e_status = ENC_NULL;
    }

    audio_adc_off_api();
}

static int encode_start(void)
{
    char c;
    u32 err;
    void *device = 0;
    e_status = 0;
    enc_bp_save_flg = 0;

    enc_info = enc_file_open(rec_folder, rec_filename);
    if (NULL == enc_info) {
        log_info("enc file open failed !!! \n");
        return -1;
    }

    //采样率支持8k,12k,16k,24k
    err = audio_adc_init_api(24000, ADC_MIC, 0);
    if (0 != err) {
        log_info(" audio adc init fail : 0x%x\n");
        enc_file_close(enc_info, 0);
        return -1;
    }

    encoder_io(mp3_encode_api, enc_info->pfile);	//mp2 格式

    e_status = 1;

    return 0;
}

static int record_init(void)
{
    /* key_table_sel((void *)record_msg_filter); */

    key_table_sel(rec_msg_filter);
    return encode_start();
}

static int record_close(void)
{
    log_info("app_record_close \n");
    encode_stop();
    return 0;
}

void rec_mode_loop(void)
{
    u16 msg;
    dac_mute(0);
    SET_UI_MAIN(MENU_REC);
    UI_menu(MENU_REC);

    while (1) {
        msg = app_get_msg();
        bsp_loop();

        switch (msg) {
        case MSG_REC_MODE_SWITCH:
            work_mode = MUSIC_MODE;
            enc_bp_save_flg = 1;
            return;
        case MSG_WFILE_FULL:
            log_info("record write full !!! \n");
            work_mode = MUSIC_MODE;
            return;
        case MSG_MUSIC_NEW_DEVICE_IN:
            if (work_mode == REC_MODE) {
                work_mode = MUSIC_MODE;
            }
        case MSG_CHANGE_WORK_MODE:
            //deg_puts("Exit RTC mode\n");
            return;

        case MSG_500MS:
            log_char('H');
            LED_FADE_OFF();
            UI_menu(MENU_MAIN);
        default:
            ap_handle_hotkey(msg);
            break;
        }
    }

}

/*----------------------------------------------------------------------------*/
/**@brief   rec-In 模式
   @param   void
   @return  void
   @author
   @note    void rec_in_mode(void)
*/
/*----------------------------------------------------------------------------*/
void rec_mode(void)
{
    if (record_init() == 0) {
        work_mode_save();
        rec_mode_loop();
        record_close();
    } else {
        work_mode++;
    }
}
#endif


