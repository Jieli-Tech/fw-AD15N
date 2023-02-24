#include "app_midi_ctrl.h"
#include "MIDI_CTRL_API.h"
#include "music_midi_ctrl_api.h"
#include "midi_prog.h"
#include "music_api.h"
#include "config.h"
#include "msg.h"
#include "key.h"
#include "app_mg/app_mg.h"
#include "music_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#define MIDI_VIBRATO_ENABLE 0   //亲键颤音功能
#define MAX_MIDI_CHN        16
static music_play_obj *music_midi_keyboard_obj = NULL;
static music_play_obj *music_a_obj = NULL;
static u16 midi_keyboard_idle_cnt = 0;
/* demo示例音色库只有钢琴可以弹奏全key音色，其余乐器key范围只有[60, 71] */
const u8 Channal_Prog_Tab[MAX_MIDI_CHN] = {
    /* 标准乐器0~8 */
    MIDI_PROG_ACOUSTIC_GRAND_PIANO,
    MIDI_PROG_MUSIC_BOX_CHROMATIC_PERCUSSION,
    MIDI_PROG_HARMONICA_ORGAN,
    MIDI_PROG_NYLON_ACOUSTIC_GUITAR,
    MIDI_PROG_ACOUSTIC_BASS,
    MIDI_PROG_VIOLIN_SOLO_STRINGS,
    MIDI_PROG_TRUMPET_BRASS,
    MIDI_PROG_SOPRANO_SAX_REED,
    MIDI_PROG_TAIKO_DRUM_PERCUSSIVE,
    /* 乐器号>=128的打击乐器,只能用通道9 */
    MIDI_PROG_PERCUSSION_STANDARD,
    /* 标准乐器10~15 */
    MIDI_PROG_TUBULAR_BELLS_CHROMATIC_PERCUSSION,
    MIDI_PROG_ORCHESTRAL_HARP_SOLO_STRINGS,
    MIDI_PROG_FLUTE_PIPE,
    MIDI_PROG_BANJO_ETHNIC,
    MIDI_PROG_SHANAI_ETHNIC,
    MIDI_PROG_TELEPHONE_RING_SOUND_EFFECTS,
};

static const char *const dir_a = {
    "dir_a",
};

static u16 midi_ctrl_msg_filter(u8 key_status, u8 key_num, u8 key_type)
{
    u16 msg = NO_MSG;
    switch (key_type) {
#if KEY_IO_EN
    case KEY_TYPE_IO:
        msg = iokey_msg_midi_keyboard_table[key_status][key_num];
        break;
#endif
#if KEY_AD_EN
    case KEY_TYPE_AD:
        msg = adkey_msg_midi_keyboard_table[key_status][key_num];
        break;
#endif
    default:
        break;
    }
    return msg;
}

/* 音符开始回调 */
u32 midi_ctrl_melody_trigger(void *priv, u8 key, u8 vel)
{
    log_info("ON %d %d\n", key, vel);
    midi_keyboard_idle_cnt++;
    return 0;
}
/* 音符结束回调 */
u32 midi_ctrl_melody_stop_trigger(void *priv, u8 key)
{
    log_info("OFF %d\n", key);
    midi_keyboard_idle_cnt--;
    if (0 == midi_keyboard_idle_cnt) {
        midi_ctrl_play_end_deal(music_midi_keyboard_obj);
    }
    return 0;
}
static int app_midi_keyboard_init(void *param)
{
    key_table_sel((void *)midi_ctrl_msg_filter);
    decoder_init();
    if (music_play(&music_midi_keyboard_obj, NULL, 0, BIT_MIDI_CTRL, NULL, 0, 0, NULL) == -1) {
        log_error("music_play_midi_ctrl failed !!!! \n");
    } else {
        log_info("music_play_midi_ctrl succeed \n");
    }
    for (int chn_num = 0; chn_num < MAX_MIDI_CHN; chn_num++) {
        if (9 == chn_num) {
            /* 乐器号>=128的打击乐器使用通道9, 传入的乐器号需要减128 */
            music_midi_ctrl_set_prog(music_midi_keyboard_obj, Channal_Prog_Tab[chn_num] - 128, chn_num);
        } else {
            music_midi_ctrl_set_prog(music_midi_keyboard_obj, Channal_Prog_Tab[chn_num], chn_num);
        }
    }
    midi_ctrl_callback_init(music_midi_keyboard_obj, midi_ctrl_melody_trigger, midi_ctrl_melody_stop_trigger);
    music_midi_ctrl_note_on(music_midi_keyboard_obj, 60, 127, 0);
    return 0;

}

int app_midi_keyboard(void *param)
{
    int msg[2];
    u8 scale = 5;
    u8 key_offset = 0;
    u8 cur_channal = 0;
    u16 pitch_val = 256;

    log_info("app_midi_keyboard \n");
    if (app_midi_keyboard_init(param) != 0) {
        log_error("app_midi_keyboard init error !!! \n");
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
        /* NOTE ON 按下琴键 */
        case MSG_MIDICTRL_NOTE_ON_DO:
            log_info("Do 1");
            key_offset = 0;
            goto __midikey_note_on;
        case MSG_MIDICTRL_NOTE_ON_RE:
            log_info("Re 2");
            key_offset = 2;
            goto __midikey_note_on;
        case MSG_MIDICTRL_NOTE_ON_MI:
            log_info("Mi 3");
            key_offset = 4;
            goto __midikey_note_on;
        case MSG_MIDICTRL_NOTE_ON_FA:
            log_info("Fa 4");
            key_offset = 5;
__midikey_note_on:
            music_midi_ctrl_note_on(music_midi_keyboard_obj, key_offset + scale * 12, 127, cur_channal);
#if MIDI_VIBRATO_ENABLE
            music_midi_ctrl_vel_vibrate(music_midi_keyboard_obj, key_offset + scale * 12, 2, 9, cur_channal);
#endif
            break;

        /* NOTE OFF 松开琴键，部分乐器需要NOTE_OFF才会停止发声 */
        case MSG_MIDICTRL_NOTE_OFF_DO:
            key_offset = 0;
            goto __midikey_note_off;
        case MSG_MIDICTRL_NOTE_OFF_RE:
            key_offset = 2;
            goto __midikey_note_off;
        case MSG_MIDICTRL_NOTE_OFF_MI:
            key_offset = 4;
            goto __midikey_note_off;
        case MSG_MIDICTRL_NOTE_OFF_FA:
            key_offset = 5;
__midikey_note_off:
            music_midi_ctrl_note_off(music_midi_keyboard_obj, key_offset + scale * 12, cur_channal, 0);
            log_info("NOTE OFF");
            break;

        /* PITCH BEND 弯音轮 */
        case MSG_MIDICTRL_PITCH_BEND_DOWN:
            pitch_val -= 16;
            goto __midikeypitch_bend;
        case MSG_MIDICTRL_PITCH_BEND_UP:
            pitch_val += 16;
__midikeypitch_bend:
            music_midi_ctrl_pitch_bend(music_midi_keyboard_obj, pitch_val, cur_channal);
            log_info("Pitch Bend : %d\n", pitch_val);
            break;

        /* PROG CHANNAL SEL 乐器通道选择 */
        case MSG_MIDICTRL_CHANNAL_PREV:
            cur_channal--;
            if (cur_channal >= MAX_MIDI_CHN) {
                cur_channal = MAX_MIDI_CHN - 1;
            }
            goto __midikey_channal_sel;
        case MSG_MIDICTRL_CHANNAL_NEXT:
            cur_channal++;
            if (cur_channal >= MAX_MIDI_CHN) {
                cur_channal = 0;
            }
__midikey_channal_sel:
            log_info("Channal:%d, Prog:%d\n", cur_channal, Channal_Prog_Tab[cur_channal]);
            break;

        /* COMMON */
        case MSG_A_PLAY:
            log_info("a play \n");
            if (music_play(&music_a_obj, dir_a, 1, BIT_A, NULL, MUSIC_MODE_PLAY_ONE, FS_TYPE_SYD, NULL) == -1) {
                log_error("music_play failed !!! \n");
            }
            break;
        case MSG_A_FILE_END:
            log_info("decode end :%d \n", music_play_get_decoder_type(music_a_obj));
            music_play_end_operation(&music_a_obj);
            break;
        case MSG_500MS:
            if (music_play_get_status(music_midi_keyboard_obj) == MUSIC_PLAY || music_play_get_status(music_a_obj) == MUSIC_PLAY) {
                //busy
                idle_check_deal(1);
            } else {
                idle_check_deal(0);
            }
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
    music_play_destroy(&music_midi_keyboard_obj);
    music_play_destroy(&music_a_obj);
    return 0;
}
