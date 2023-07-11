#include "toy_record.h"
#include "device.h"
#include "vfs.h"
#include "msg.h"
#include "encoder_mge.h"
#include "device_mge.h"

#include "decoder_api.h"
#include "decoder_msg_tab.h"
#include "simple_play_file.h"
#include "audio.h"
#include "dac_api.h"
#include "audio_adc.h"
#include "toy_main.h"
#include "bsp_loop.h"
#include "mp3_encoder.h"
#include "a_encoder.h"

#if defined(RECORD_MODE_EN) && (RECORD_MODE_EN)
#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rec]"
#include "log.h"

#define RECORD_AUDIO_ADC_SR     RECORD_ADC_SR_24K
static Encode_Control record_obj;

void toy_record_app(void)
{
    int msg[2];
    u32 err = 0;
    dec_obj *p_dec_obj = 0;
    u16 norfs_decode_type = BIT_A | BIT_UMP3 | BIT_SPEED;
    key_table_sel(record_key_msg_filter);
    decoder_init();

    memset(&record_obj, 0, sizeof(record_obj));

    while (1) {
        err = get_msg(2, &msg[0]);
        bsp_loop();
        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
            log_info("get msg err 0x%x\n", err);
        }

        switch (msg[0]) {
        case MSG_RECODE_START:
            if (ENC_ING == record_obj.enc_status) {
                /* 结束录音并播放录音 */
                encode_stop(&record_obj);
                post_msg(1, MSG_PP);
            } else {
                /* 开始录音 */
                decoder_stop(p_dec_obj, NEED_WAIT);
                encode_file_fs_close(&record_obj);
                encode_start(&record_obj);
                log_info("dev:%d fs_name:%s\n", record_obj.dev_index, record_obj.fs_name);
            }
            break;
        case MSG_PP:
            encode_stop(&record_obj);
            decoder_stop(p_dec_obj, NEED_WAIT);
            if (0 == (strcmp(record_obj.fs_name, "norfs"))) {
                p_dec_obj = norfs_enc_file_decode(&record_obj, norfs_decode_type);
            } else {
                log_info("record hasn't been started!\n");
                break;
            }
            if (NULL == p_dec_obj) {
                log_info("record file decode fail!\n");
                encode_file_fs_close(&record_obj);
            }
            break;

        case MSG_REC_SPEED_EN:
            if (norfs_decode_type & BIT_SPEED) {
                log_info("record normal mode \n");
                norfs_decode_type &= ~BIT_SPEED;
            } else {
                log_info("record speed mode \n");
                norfs_decode_type |= BIT_SPEED;
            }
            break;
        case MSG_WFILE_FULL:
            log_info("MSG_WFILE_FULL\n");
            encode_stop(&record_obj);
            break;
        case MSG_WAV_FILE_END:
        case MSG_MP3_FILE_END:
        case MSG_A_FILE_END:
            decoder_stop(p_dec_obj, NEED_WAIT);
            encode_file_fs_close(&record_obj);
            break;

        case MSG_CHANGE_WORK_MODE:
            goto __record_app_exit;
        case MSG_500MS:
            if (record_obj.enc_status == ENC_ING) {
                log_char('R');
            }
            if ((MUSIC_PLAY != get_decoder_status(p_dec_obj)) && \
                (record_obj.enc_status == ENC_NULL)) {
                sys_idle_deal(-2);
            } else {
            }
        default:
            common_msg_deal(&msg[0]);
            break;
        }
    }

__record_app_exit:
    key_table_sel(NULL);
    if (ENC_ING == record_obj.enc_status) {
        encode_stop(&record_obj);
    } else {
        decoder_stop(p_dec_obj, NEED_WAIT);
        encode_file_fs_close(&record_obj);
    }
    return;
}

void encode_file_fs_close(Encode_Control *obj)
{
    vfs_file_close(&obj->pfile);
    vfs_fs_close(&obj->pfs);
    device_close(obj->dev_index);
}

static void encode_stop(Encode_Control *obj)
{
    if (ENC_ING == obj->enc_status) {
        stop_encode(obj->pfile, 0);
        encode_file_fs_close(obj);
        obj->enc_status = ENC_NULL;
    }
    audio_adc_off_api();
}

static int encode_start(Encode_Control *obj)
{
    u32 sr = RECORD_AUDIO_ADC_SR;
    /* u32 sr = dac_sr_read(); */
    /* log_info("adc sr:%d\n", sr); */
    int err = audio_adc_init_api(sr, ADC_MIC, BIT(1));
    if (0 != err) {
        log_info(" audio adc init fail : 0x%x\n");
        return err;
    }

#if ENCODER_UMP3_EN
    obj->dev_index = INNER_FLASH_RW;//内置flash录音
    strcpy(obj->fs_name, "norfs");
    err = norfs_enc_file_create(obj);
    if (0 != err) {
        log_info("vfs create 0x%x!\n", err);
        return err;
    }
    encoder_io(ump3_encode_api, obj->pfile);
#elif ENCODER_A_EN
    obj->dev_index = INNER_FLASH_RW;//内置flash录音
    strcpy(obj->fs_name, "norfs");
    err = norfs_enc_file_create(obj);
    if (0 != err) {
        log_info("vfs create 0x%x!\n", err);
        return err;
    }
    encoder_io(a_encode_api, obj->pfile);
#else
    log_info("no ump3 & a format encoder!\n");
    return -1;
#endif

    obj->enc_status = ENC_ING;
    return 0;
}

#endif
