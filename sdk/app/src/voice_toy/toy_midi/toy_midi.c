#include "toy_midi.h"
#include "app_modules.h"
#include "simple_play_file.h"
#include "toy_main.h"
#include "midi_config.h"
#include "midi_api.h"
#include "MIDI_DEC_API.h"
#include "device_mge.h"
#include "audio.h"
#include "msg.h"
#include "key.h"
#include "bsp_loop.h"
#include "vm_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#if defined(DECODER_MIDI_EN) && (DECODER_MIDI_EN)

static play_control midi_pctl[2]    AT(.midi_buf);
static u8 app_midi_mode             AT(.midi_buf);
static const char *const dir_midi_tab[] = {
    "/dir_midi",
};
static const char *const dir_tab_a[] = {
    "/dir_a",
};

void toy_midi_app(void)
{
    u32 err;
    int msg[2];

    log_info("toy midi mode!\n");
    decoder_init();
    midi_decode_init();
    key_table_sel((void *)midi_msg_filter);

    memset(&midi_pctl[0], 0, sizeof(midi_pctl));
    midi_pctl[0].dev_index   = INNER_FLASH_RO;
    midi_pctl[0].findex      = 1;
    midi_pctl[0].dec_type    = BIT_MIDI;
    midi_pctl[0].pdir        = (void *)&dir_midi_tab[0];
    midi_pctl[0].dir_total   = sizeof(dir_midi_tab) / 4;
    simple_dev_fs_mount(&midi_pctl[0]);
    post_msg(1, MSG_PLAY_FILE1);

    midi_pctl[1].dev_index   = INNER_FLASH_RO;
    midi_pctl[1].findex      = 1;
    midi_pctl[1].loop        = 255;
    midi_pctl[1].dec_type    = BIT_A;
    midi_pctl[1].pdir        = (void *)&dir_tab_a[0];
    midi_pctl[1].dir_total   = sizeof(dir_tab_a) / 4;
    simple_dev_fs_mount(&midi_pctl[1]);
    /* post_msg(1, MSG_A_PLAY); */

    while (1) {
        err = get_msg(2, &msg[0]);
        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
            log_info("get msg err 0x%x\n", err);
        }
        bsp_loop();

        if (NO_MSG == msg[0]) {
            continue;
        }

        switch (msg[0]) {
        case MSG_MIDI_MODE_SWITCH:
            log_info("MSG_MIDI_MODE_SWITCH\n");
            MIDI_PLAY_CTRL_MODE *mode_info = get_midi_mode();
            if (mode_info->mode == CMD_MIDI_CTRL_MODE_0) {
                mode_info->mode = CMD_MIDI_CTRL_MODE_1;
            } else {
                mode_info->mode = CMD_MIDI_CTRL_MODE_0;
            }
            midi_dec_config(midi_pctl[0].p_dec_obj, CMD_MIDI_CTRL_MODE, (void *)mode_info);
            break;
        case MSG_MIDI_OKON_GOON:
            if (get_midi_mode()->mode == CMD_MIDI_CTRL_MODE_1) {
                log_info("a");
                midi_okon_goon(midi_pctl[0].p_dec_obj);
            }
            break;

        case MSG_PLAY_FILE1:
            log_info("MSG_PLAY_FILE1\n");
            err = play_one_file(&midi_pctl[0]);
            if (err) {
                play_next_file(&midi_pctl[0]);
            }
            break;

        case MSG_PP:
            log_info("MSG_PP\n");
            decoder_pause(midi_pctl[0].p_dec_obj);
            break;
        case MSG_PREV_FILE:
            log_info("MSG_PREV_FILE\n");
            play_prev_file(&midi_pctl[0]);
            break;
        case MSG_NEXT_FILE:
            log_info("MSG_NEXT_FILE\n");
            play_next_file(&midi_pctl[0]);
            break;

        case MSG_MIDI_FILE_END:
        case MSG_MIDI_FILE_ERR:
            log_info("FILE END OR ERR\n");
            decoder_stop(midi_pctl[0].p_dec_obj, NEED_WAIT);
            play_next_file(&midi_pctl[0]);
            break;

        case MSG_A_PLAY:
            log_info("MSG_A_PLAY\n");
            play_one_file(&midi_pctl[1]);
            break;
        case MSG_A_FILE_END:
        case MSG_A_FILE_ERR:
            log_info("A FILE END OR ERR\n");
            decoder_stop(midi_pctl[1].p_dec_obj, NEED_WAIT);
            break;
        case MSG_A_LOOP:
            log_info("-loop\n");
            break;

        case MSG_CHANGE_WORK_MODE:
            goto __toy_midi_exit;
        case MSG_500MS:
            if ((MUSIC_PLAY != get_decoder_status(midi_pctl[0].p_dec_obj)) && \
                (MUSIC_PLAY != get_decoder_status(midi_pctl[1].p_dec_obj))) {
                vm_pre_erase();
                sys_idle_deal(-2);
            }
        default:
            common_msg_deal(&msg[0]);
            break;
        }
    }
__toy_midi_exit:
    decoder_stop(midi_pctl[0].p_dec_obj, NEED_WAIT);
    simple_dev_fs_close(&midi_pctl[0]);
    decoder_stop(midi_pctl[1].p_dec_obj, NEED_WAIT);
    simple_dev_fs_close(&midi_pctl[1]);
    key_table_sel(NULL);
}
#endif
