#include "config.h"
#include "msg.h"
#include "app_mg/app_mg.h"
#include "app_mg/record/app_record.h"
#include "encoder_mge.h"
#include "a_encoder.h"
#include "mp3_encoder.h"
#include "boot.h"
#include "vm.h"
#include "device.h"
#include "vfs.h"
#include "music_api.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#if ENCODER_EN
typedef enum {
    ENC_NULL = 0,
    ENC_ING,

} ENC_STATUS;

static u8 e_status = 0;
static music_play_obj *music_obj = NULL;
static u32 findex  = 1;
static void *pfs = 0;
static void *pfile = 0;


static u16 record_msg_filter(u8 key_status, u8 key_num, u8 key_type)
{
    u16 msg = NO_MSG;
    switch (key_type) {
#if KEY_IO_EN
    case KEY_TYPE_IO:
        msg = iokey_msg_record_table[key_status][key_num];
        break;
#endif
#if KEY_AD_EN
    case KEY_TYPE_AD:
        msg = adkey_msg_record_table[key_status][key_num];
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
        vfs_file_close(&pfile);
        vfs_fs_close(&pfs);
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

    //采样率支持8k,12k,16k,24k
    err = audio_adc_init_api(24000, ADC_MIC, 0);
    if (0 != err) {
        log_info(" audio adc init fail : 0x%x\n");
        return -1;
    }

    device = dev_open(__SFC_NANE, 0);

    if (NULL == device) {
        log_info("device null\n");
        return -1;
    }

    if (vfs_mount(&pfs, device, "norfs") != 0) {
        log_error("vfs mount error !!! \n");
        return -1;
    }

    err = vfs_createfile(pfs, &pfile, &findex);
    if (0 != err) {
        log_error("vfs create 0x%x!\n", err);
        vfs_fs_close(&pfs);
        return -1;
    }
    log_info(" file index : %d\n", findex);
    encoder_io(ump3_encode_api, pfile);	//mp2 格式
//	encoder_io(a_encode_api, pfile);	//a 格式

    e_status = 1;

    return 0;
}

static int app_record_init(void *param)
{
    key_table_sel((void *)record_msg_filter);

    return encode_start();
}

static int app_record_close(void)
{
    log_info("app_record_close \n");
    encode_stop();
    return 0;
}

int app_record(void *param)
{
    int msg[2];

    u32 decoder_type = BIT_A | BIT_UMP3 | BIT_SPEED;
    log_info("app_record \n");
    if (app_record_init(param) != 0) {
        log_error("app_record init error !!! \n");
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
        case MSG_REC_MODE_SWITCH:
            if (e_status == ENC_ING) {
                ///开始播放录音
                encode_stop();
                decoder_init();
                void *device = dev_open(__SFC_NANE, 0);
                if (music_play(&music_obj, NULL, findex, decoder_type, device, MUSIC_MODE_PLAY_ONE, FS_TYPE_NORFS, NULL) == -1) {
                    log_info("music play succ \n");
                }
            } else {
                ///开始录音
                music_play_destroy(&music_obj);
                encode_start();
            }
            break;
        case MSG_REC_SPEED_EN:
            if (decoder_type & BIT_SPEED) {
                log_info("record normal mode \n");
                decoder_type = BIT_A | BIT_UMP3;
            } else {
                log_info("record speed mode \n");
                decoder_type = BIT_A | BIT_UMP3 | BIT_SPEED;
            }
            break;

        case MSG_500MS:
            if (music_play_get_status(music_obj) == MUSIC_PLAY || e_status == ENC_ING) {
                //busy
                idle_check_deal(1);
            } else {
                idle_check_deal(0);
            }
            break;
        case MSG_WFILE_FULL:
            log_info("record write full !!! \n");
            if (e_status == ENC_ING) {
                encode_stop();
            }
            break;
        case MSG_WAV_FILE_END:
        case MSG_MP3_FILE_END:
        case MSG_F1A1_FILE_END:
        case MSG_F1A2_FILE_END:
        case MSG_MIDI_FILE_END:
            log_info("decode end :%d \n", music_play_get_decoder_type(music_obj));
            music_play_end_operation(&music_obj);
            break;
        case MSG_WAV_FILE_ERR:
        case MSG_F1A1_FILE_ERR:
        case MSG_F1A2_FILE_ERR:
        case MSG_MP3_FILE_ERR:
        case MSG_MIDI_FILE_ERR:
            music_play_destroy(&music_obj);
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
    app_record_close();
    return 0;
}
#endif


