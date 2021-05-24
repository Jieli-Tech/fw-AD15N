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

#if DECODER_MIDI_EN
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
int music_midi_ctrl_set_prog(music_play_obj *hdl, u8 prog, u8 trk_num)
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

    midi_ctrl_set_prog(work_buf, prog, trk_num);
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

#endif
