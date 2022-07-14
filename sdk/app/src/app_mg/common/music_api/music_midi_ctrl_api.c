#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "hwi.h"
#include "decoder_api.h"
#include "dev_manage.h"
#include "vfs.h"
#include "circular_buf.h"
#include "audio.h"
#include "dac.h"
#include "ump3_api.h"
#include "midi_api.h"
#include "f1a_api.h"
#include "a_api.h"
#include "msg.h"
#include "speed_api.h"
#include "src_api.h"
#include "music_api.h"
#include "music_file_op/music_file_op.h"
#include "music_midi_api.h"
#include "list/midi_ctrl_api.h"
#include "music_midi_ctrl_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"

#if DECODER_MIDI_KEYBOARD_EN
int music_midi_ctrl_config(music_play_obj *hdl, u32 cmd, void *parm)
{
    if (hdl == NULL || hdl->en != MUSIC_PLAY_INIT_OK) {
        return MUSIC_STOP;
    }

    dec_obj *obj = hdl->decode_api.p_dec;
    if (NULL == obj) {
        return -1;
    }

    audio_decoder_ops *ops = (audio_decoder_ops *)obj->dec_ops;
    void *work_buf = obj->p_dbuf;
    if (ops == NULL || work_buf == NULL) {
        return -1;
    }

    ops->dec_confing(work_buf, cmd, parm);
    return 0;
}
int music_midi_ctrl_set_prog(music_play_obj *hdl, u8 prog, u8 chn)
{
    if (hdl == NULL || hdl->en != MUSIC_PLAY_INIT_OK) {
        return MUSIC_STOP;
    }

    dec_obj *obj = hdl->decode_api.p_dec;
    if (NULL == obj) {
        return -1;
    }

    audio_decoder_ops *ops = (audio_decoder_ops *)obj->dec_ops;
    void *work_buf = obj->p_dbuf;
    if (ops == NULL || work_buf == NULL) {
        return -1;
    }

    midi_ctrl_set_prog(work_buf, prog, chn);
    return 0;
}
int music_midi_ctrl_note_on(music_play_obj *hdl, u8 nkey, u8 nvel, u8 chn)
{
    if (hdl == NULL || hdl->en != MUSIC_PLAY_INIT_OK) {
        return MUSIC_STOP;
    }

    dec_obj *obj = hdl->decode_api.p_dec;
    if (NULL == obj) {
        return -1;
    }

    audio_decoder_ops *ops = (audio_decoder_ops *)obj->dec_ops;
    void *work_buf = obj->p_dbuf;
    if (ops == NULL || work_buf == NULL) {
        return -1;
    }

    midi_ctrl_note_on(work_buf, nkey, nvel, chn);
    return 0;
}
int music_midi_ctrl_note_off(music_play_obj *hdl, u8 nkey, u8 chn)
{
    if (hdl == NULL || hdl->en != MUSIC_PLAY_INIT_OK) {
        return MUSIC_STOP;
    }

    dec_obj *obj = hdl->decode_api.p_dec;
    if (NULL == obj) {
        return -1;
    }

    audio_decoder_ops *ops = (audio_decoder_ops *)obj->dec_ops;
    void *work_buf = obj->p_dbuf;
    if (ops == NULL || work_buf == NULL) {
        return -1;
    }

    midi_ctrl_note_off(work_buf, nkey, chn);
    return 0;
}
int music_midi_ctrl_vel_vibrate(music_play_obj *hdl, u8 nkey, u8 vel_step, u8 vel_rate, u8 chn)
{
    if (hdl == NULL || hdl->en != MUSIC_PLAY_INIT_OK) {
        return MUSIC_STOP;
    }

    dec_obj *obj = hdl->decode_api.p_dec;
    if (NULL == obj) {
        return -1;
    }

    audio_decoder_ops *ops = (audio_decoder_ops *)obj->dec_ops;
    void *work_buf = obj->p_dbuf;
    if (ops == NULL || work_buf == NULL) {
        return -1;
    }

    midi_ctrl_vel_vibrate(work_buf, nkey, vel_step, vel_rate, chn);
    return 0;
}
int music_midi_ctrl_pitch_bend(music_play_obj *hdl, u16 pitch_val, u8 chn)
{
    if (hdl == NULL || hdl->en != MUSIC_PLAY_INIT_OK) {
        return MUSIC_STOP;
    }

    dec_obj *obj = hdl->decode_api.p_dec;
    if (NULL == obj) {
        return -1;
    }

    audio_decoder_ops *ops = (audio_decoder_ops *)obj->dec_ops;
    void *work_buf = obj->p_dbuf;
    if (ops == NULL || work_buf == NULL) {
        return -1;
    }

    midi_ctrl_pitch_bend(work_buf, pitch_val, chn);
    return 0;
}

void midi_ctrl_play_end_deal(music_play_obj *hdl)
{
    dec_obj *obj = hdl->decode_api.p_dec;
    midi_error_play_end_cb(obj, MAD_ERROR_PLAY_END);
}

void midi_ctrl_callback_init(music_play_obj *hdl, u32(*melody_callback)(void *, u8, u8), u32(*melody_stop_callback)(void *, u8))
{
    EX_MELODY_STRUCT melody_parm;
    melody_parm.priv = NULL;
    melody_parm.melody_trigger = melody_callback;
    music_midi_ctrl_config(hdl, CMD_MIDI_MELODY_TRIGGER, &melody_parm);

    EX_MELODY_STOP_STRUCT melody_stop_parm;
    melody_stop_parm.priv = NULL;
    melody_stop_parm.melody_stop_trigger = melody_stop_callback;
    music_midi_ctrl_config(hdl, CMD_MIDI_STOP_MELODY_TRIGGER, &melody_stop_parm);

    int switch_enable = MELODY_ENABLE | MELODY_STOP_ENABLE;
    music_midi_ctrl_config(hdl, CMD_MIDI_SET_SWITCH, &switch_enable);
}
#endif
