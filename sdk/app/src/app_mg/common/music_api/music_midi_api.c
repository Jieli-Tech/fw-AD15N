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


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#if DECODER_MIDI_EN
/*----------------------------------------------------------------------------*/
/**@brief  midi cmd 配置接口
   @param  hdl:音乐播放句柄
   @param  cmd:命令，详细看MIDI_CMD_API.H
   @param  param:参数，根据cmd来定
   @return 0:ok -1:fail
   @note
*/
/*----------------------------------------------------------------------------*/
int music_midi_dec_config(music_play_obj *hdl, u32 cmd, void *parm)
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

/*----------------------------------------------------------------------------*/
/**@brief   midi mode set
   @param  music_obj:音乐播放句柄
   @param  mode:
        CMD_MIDI_CTRL_MODE_0        	= 0X00,		//正常播放模式
    	CMD_MIDI_CTRL_MODE_1          	= 0X01,		//one key one note的模式，就是默认外面要按一下键（调CMD_MIDI_GOON）才弹奏下一个
    	CMD_MIDI_CTRL_MODE_2,						//CMD_MIDI_CTRL_MODE_2是为了外面可以设置不要主音轨的声音
    	CMD_MIDI_CTRL_MODE_W2S                   	//外部音源
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void midi_okon_mode_set(music_play_obj *music_obj, u8 mode)
{
    MIDI_PLAY_CTRL_MODE mode_info;
    mode_info.mode = mode;
    music_midi_dec_config(music_obj, CMD_MIDI_CTRL_MODE, (void *)&mode_info);
}

/*----------------------------------------------------------------------------*/
/**@brief  midi在one key one note 模式下面启动下一个音节
   @param  music_obj:音乐播放句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void midi_okon_goon(music_play_obj *music_obj)
{
    music_midi_dec_config(music_obj, CMD_MIDI_GOON, NULL);
}

u32 midi_timeDiv_trigger(void *priv)
{
    return 0;
}
u32 midi_melody_trigger(void *priv, u8 key, u8 vel)
{
    return 0;
}
u32 midi_melody_stop_trigger(void *priv, u8 key)
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
    /* wdt_clear(); */
    /* log_char('w'); */
    /* log_info("wdt_clear_trigger \n"); */
    return 64;
}

/*----------------------------------------------------------------------------*/
/**@brief  midi 初始化接口，会给midi解码器主动调用
   @param  init_info:初始化的参数
   @return NULL
   @note
*/
/*----------------------------------------------------------------------------*/
void midi_init_info(MIDI_INIT_STRUCT *init_info)
{
    log_info(">>>>>>>>>midi information init<<<<<<<<<< \n");

    ///模式设定
    init_info->mode_info.mode = CMD_MIDI_CTRL_MODE_1;

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
    init_info->moledy_stop_info.melody_stop_trigger = midi_melody_stop_trigger;

    init_info->beat_info.priv = NULL;
    init_info->beat_info.beat_trigger = midi_beat_trigger;

    init_info->tmDiv_info.priv = NULL;
    init_info->tmDiv_info.timeDiv_trigger = midi_timeDiv_trigger;

    ///使能控制
    init_info->switch_info = MELODY_STOP_ENABLE | MARK_ENABLE | MELODY_ENABLE | TIM_DIV_ENABLE | BEAT_TRIG_ENABLE | MELODY_PLAY_ENABLE | EX_VOL_ENABLE;// | SEMITONE_ENABLE;
}

#endif
