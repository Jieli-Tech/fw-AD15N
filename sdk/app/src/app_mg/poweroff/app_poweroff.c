#include "config.h"
#include "msg.h"
#include "app_mg/app_mg.h"
#include "app_mg/poweroff/app_poweroff.h"
#include "asm/power_interface.h"
#include "app_mg/music/app_music.h"
#include "music_api.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"
static music_play_obj *music_obj = NULL;

static u16 poweroff_msg_filter(u8 key_status, u8 key_num, u8 key_type)
{
    u16 msg = NO_MSG;
    switch (key_type) {
#if KEY_IO_EN
    case KEY_TYPE_IO:
        msg = iokey_msg_poweroff_table[key_status][key_num];
        break;
#endif
#if KEY_AD_EN
    case KEY_TYPE_AD:
        msg = adkey_msg_poweroff_table[key_status][key_num];
        break;
#endif

    default:
        break;
    }

    return msg;
}

static int app_poweroff_init(void *param)
{
    if (param == NULL) {
        return -1;
    }

    decoder_init();
    dac_vol(0, 16);
    key_table_sel((void *)poweroff_msg_filter);
    if (music_play(&music_obj, "dir_notice/power_off.f1a", 0, BIT_UMP3 | BIT_F1A1, NULL, MUSIC_MODE_PLAY_ONE, FS_TYPE_SYD, NULL) == -1) {
        log_error("music_play failed !!! \n");
    } else {
        log_info("music_play succeed !!! \n");
    }

    return 0;
}


static int app_poweroff_close(void)
{
    log_info("app_poweroff_close \n");
    music_play_destroy(&music_obj);
    return 0;
}

int app_poweroff(void *param)
{
    int msg[2];

    log_info("app_poweroff \n");
    if (app_poweroff_init(param) != 0) {
        log_info("app_poweroff init error !!! \n");
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

        case MSG_WAV_FILE_END:
        case MSG_MP3_FILE_END:
        case MSG_F1A1_FILE_END:
        case MSG_F1A2_FILE_END:
        case MSG_MIDI_FILE_END:
            log_info("decode end :%d \n", music_play_get_decoder_type(music_obj));
            music_play_destroy(&music_obj);
            power_set_soft_poweroff();
            break;

        case MSG_NEXT_MODE:
            app_switch_next();
            break;
        case MSG_APP_SWITCH_ACTIVE:
            if (app_switch_en()) {
                goto _app_switch;
            }
            break;

        default:
            break;
        }
    }

_app_switch:
    app_poweroff_close();
    return 0;
}


