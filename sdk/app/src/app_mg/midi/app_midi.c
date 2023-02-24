#include "config.h"
#include "msg.h"
#include "app_mg/app_mg.h"
#include "app_mg/midi/app_midi.h"
#include "music_api.h"
#include "music_midi_api.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#define DIR_MEMS(ppdir) (sizeof(ppdir)/sizeof(ppdir[0]))
static const char *const dir_table = {
    "dir_midi",
};
static const char *const dir_a = {
    "dir_a",
};
static music_play_obj *music_midi_obj = NULL;
static music_play_obj *music_a_obj = NULL;
u8 midi_mode = CMD_MIDI_CTRL_MODE_0;

static u16 midi_msg_filter(u8 key_status, u8 key_num, u8 key_type)
{
    u16 msg = NO_MSG;
    switch (key_type) {
#if KEY_IO_EN
    case KEY_TYPE_IO:
        msg = iokey_msg_midi_table[key_status][key_num];
        break;
#endif
#if KEY_AD_EN
    case KEY_TYPE_AD:
        msg = adkey_msg_midi_table[key_status][key_num];
        break;
#endif

    default:
        break;
    }

    return msg;
}

void midi_decode_succ_func(music_play_obj *hdl)
{
    midi_okon_mode_set(hdl, midi_mode);
}
static int app_midi_init(void *param)
{
    midi_mode = CMD_MIDI_CTRL_MODE_0;
    key_table_sel((void *)midi_msg_filter);
    music_decode_succ_cb_regist(midi_decode_succ_func);
    decoder_init();
    if (music_play(&music_midi_obj, dir_table, 1, BIT_MIDI, NULL, MUSIC_MODE_AUTO_PREV, FS_TYPE_SYD, NULL) == -1) {
        log_error("music_play_midi failed !!!! \n");
    } else {
        log_info("music_play_midi succeed \n");
#if 0
        midi_okon_mode_set(music_midi_obj, CMD_MIDI_CTRL_MODE_1);///one key one note mode
        midi_okon_mode_set(music_midi_obj, CMD_MIDI_CTRL_MODE_0);///正常模式
        midi_okon_goon(music_midi_obj);///one key one note mode 下一个音节
#endif
    }
    return 0;
}

static int app_midi_close(void)
{
    log_info("app_midi_close \n");
    music_play_destroy(&music_midi_obj);
    music_play_destroy(&music_a_obj);
    music_decode_succ_cb_regist(NULL);
    return 0;
}

int app_midi(void *param)
{
    int msg[2];
    log_info("app_midi \n");
    if (app_midi_init(param) != 0) {
        log_error("app_midi init error !!! \n");
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
        case MSG_NEXT_FILE:
            log_info("music next file\n");
            music_play_next_file(&music_midi_obj);
            break;
        case MSG_PRIV_FILE:
            log_info("music priv file\n");
            music_play_priv_file(&music_midi_obj);
            break;
        case MSG_PP:
            log_info("music pp\n");
            music_play_pp(music_midi_obj);
            break;
        case MSG_MIDI_MODE_SWITCH:
            log_info("midi mode switch \n");
            if (midi_mode == CMD_MIDI_CTRL_MODE_0) {
                midi_mode = CMD_MIDI_CTRL_MODE_1;
            } else {
                midi_mode = CMD_MIDI_CTRL_MODE_0;
            }
            midi_okon_mode_set(music_midi_obj, midi_mode);
            break;
        case MSG_MIDI_OKON_GOON:
            if (midi_mode == CMD_MIDI_CTRL_MODE_1) {
                log_info("a ");
                midi_okon_goon(music_midi_obj);
            }
            break;
        case MSG_A_PLAY:
            log_info("a play \n");
            if (music_play(&music_a_obj, dir_a, 1, BIT_A, NULL, MUSIC_MODE_PLAY_ONE, FS_TYPE_SYD, NULL) == -1) {
                log_error("music_play failed !!! \n");
            }
            break;

        case MSG_500MS:
            if (music_play_get_status(music_midi_obj) == MUSIC_PLAY || music_play_get_status(music_a_obj) == MUSIC_PLAY) {
                //busy
                idle_check_deal(1);
            } else {
                idle_check_deal(0);
            }
            break;

        case MSG_A_FILE_END:
            log_info("decode end :%d \n", music_play_get_decoder_type(music_a_obj));
            music_play_end_operation(&music_a_obj);
            break;
        case MSG_MIDI_FILE_END:
            log_info("midi end\n");
            music_play_end_operation(&music_midi_obj);
            break;
        case MSG_MIDI_FILE_ERR:
            log_info("midi err\n");
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
    app_midi_close();
    return 0;
}

