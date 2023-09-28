#include "midi_config.h"
#include "midi_api.h"
#include "MIDI_DEC_API.h"
#include "decoder_api.h"
#include "if_decoder_ctrl.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

/*----------------------------------------------------------------------------*/
/**@brief   配置midi解码参数
   @param   p_dec_obj   midi解码器句柄
   @param   cmd         midi支持的命令，参考MIDI_DEC_API.h
   @param   parm        cmd对应配置参数指针，参考MIDI_DEC_API.h
   @return  0：成功 -1：失败
   @note     int midi_dec_config(dec_obj *p_dec_obj, u32 cmd, void *parm)
*/
/*----------------------------------------------------------------------------*/
int midi_dec_config(dec_obj *p_dec_obj, u32 cmd, void *parm)
{
    if (NULL == p_dec_obj) {
        return -1;
    }

    audio_decoder_ops *ops = (audio_decoder_ops *)p_dec_obj->dec_ops;
    void *work_buf = p_dec_obj->p_dbuf;
    if (ops == NULL || work_buf == NULL) {
        return -1;
    }

    ops->dec_confing(work_buf, cmd, parm);
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief   设置midi解码模式
   @param   p_dec_obj   midi解码器句柄
   @param   mode        CMD_MIDI_CTRL_MODE_0：正常解码
                        CMD_MIDI_CTRL_MODE_1：OKON模式
                        CMD_MIDI_CTRL_MODE_2：只推消息不出声
                        CMD_MIDI_CTRL_MODE_3：外部音源模式
   @return  无
   @note     void midi_mode_set(dec_obj *p_dec_obj, u8 mode)
*/
/*----------------------------------------------------------------------------*/
void midi_mode_set(dec_obj *p_dec_obj, u8 mode)
{
    MIDI_PLAY_CTRL_MODE *mode_info = get_midi_mode();
    mode_info->mode = mode;
    midi_dec_config(p_dec_obj, CMD_MIDI_CTRL_MODE, (void *)mode_info);
}

/*----------------------------------------------------------------------------*/
/**@brief   midi解码okon模式播放下一音符
   @param   p_dec_obj   midi解码器句柄
   @return  无
   @note     void midi_okon_goon(dec_obj *p_dec_obj)
*/
/*----------------------------------------------------------------------------*/
void midi_okon_goon(dec_obj *p_dec_obj)
{
    midi_dec_config(p_dec_obj, CMD_MIDI_GOON, NULL);
}

u32 midi_timeDiv_trigger(void *priv)
{
    return 0;
}
u32 midi_melody_trigger(void *priv, u8 key, u8 vel)
{
    return 0;
}
u32 midi_melody_stop_trigger(void *priv, u8 key, u8 chn)
{
    return 0;
}
u32 midi_beat_trigger(void *priv, u8 val1, u8 val2)
{
    return 0;
}
u32 midi_mark_trigger(void *priv, u8 *val, u8 len)
{
    return 0;
}
u32 wdt_clear_trigger(void *priv)
{
    wdt_clear();
    return 64;
}
void midi_init_info(MIDI_INIT_STRUCT *init_info)
{
    /* log_info(">>>>>>>>>midi information init<<<<<<<<<< \n"); */

    ///模式设定
    init_info->mode_info.mode = CMD_MIDI_CTRL_MODE_0;

    ///midi节奏初始化
    init_info->tempo_info.tempo_val = 1042;
    for (int i = 0; i < CTRL_CHANNEL_NUM; i++) {
        init_info->tempo_info.decay_val[i] = ((u16)31 << 11) | 1024;
    }
    init_info->tempo_info.mute_threshold = (u16)1L << 29;

    ///midi主轨道初始化
    init_info->mainTrack_info.chn = 17;

    ///midi外部音量初始化
    for (int i = 0; i < CTRL_CHANNEL_NUM; i++) {
        init_info->vol_info.cc_vol[i] = 4096;
    }

    ///okon info
    init_info->okon_info.Melody_Key_Mode = CMD_MIDI_MELODY_KEY_0;
    init_info->okon_info.OKON_Mode = CMD_MIDI_OKON_MODE_0;

    ///midi主轨道乐器设置
    init_info->prog_info.prog = 0;
    init_info->prog_info.ex_vol = 1024;
    init_info->prog_info.replace_mode = 0;

    ///midi音符移半音设置
    for (int i = 0; i < CTRL_CHANNEL_NUM; i++) {
        init_info->semitone_info.key_diff[i] = 12;
    }

    ///OKON 解码副旋空转回调注册
    init_info->wdt_clear.priv = NULL;
    init_info->wdt_clear.count = 68;
    init_info->wdt_clear.wdt_clear_trigger = wdt_clear_trigger;

    ///midi 控制初始化
    init_info->mark_info.priv = NULL;
    init_info->mark_info.mark_trigger = midi_mark_trigger;

    init_info->moledy_info.priv = NULL;
    init_info->moledy_info.melody_trigger = midi_melody_trigger;

    init_info->moledy_stop_info.priv = NULL;
    init_info->moledy_stop_info.main_chn_enable = 1;
    init_info->moledy_stop_info.melody_stop_trigger = midi_melody_stop_trigger;

    init_info->beat_info.priv = NULL;
    init_info->beat_info.beat_trigger = midi_beat_trigger;

    init_info->tmDiv_info.priv = NULL;
    init_info->tmDiv_info.timeDiv_trigger = midi_timeDiv_trigger;

    ///使能控制
    init_info->switch_info = MELODY_STOP_ENABLE | MARK_ENABLE | MELODY_ENABLE | TIM_DIV_ENABLE | BEAT_TRIG_ENABLE | MELODY_PLAY_ENABLE | EX_VOL_ENABLE;// | SEMITONE_ENABLE;
}
