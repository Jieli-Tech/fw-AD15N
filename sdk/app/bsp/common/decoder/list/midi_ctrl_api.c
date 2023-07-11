#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "decoder_api.h"
#include "vfs.h"
#include "circular_buf.h"
#include "dac.h"
/* #include "resample.h" */
#include "msg.h"
#include "errno-base.h"
#include "midi_api.h"
#include "MIDI_CTRL_API.h"
#include "MIDI_DEC_API.h"
#include "boot.h"
#include "decoder_msg_tab.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

/* midi琴最大同时发声的key数,该值影响音符的叠加,值越大需要的解码buffer越大,需要的buffer大小由need_dcbuf_size()获取 */
extern const int MAX_CTR_PLAYER_CNT;//该值在app_config.c中定义

/* param */
static u32 midi_ctrl_tone_tab = 0;
MIDI_CONFIG_PARM midi_ctrl_t_parm               AT(.midi_ctrl_buf);
MIDI_CTRL_PARM midi_ctrl_parmt                  AT(.midi_ctrl_buf);
/* decode */
dec_obj dec_midi_ctrl_hld;
cbuffer_t cbuf_midi_ctrl                        AT(.midi_ctrl_buf);
u16 obuf_midi_ctrl[DAC_DECODER_BUF_SIZE / 2]    AT(.midi_ctrl_buf);
u32 midi_ctrl_decode_buff[(3944) / 4]       AT(.midi_ctrl_buf);
#define MIDI_CTRL_CAL_BUF ((void *)&midi_ctrl_decode_buff[0])


void midi_error_play_end_cb(dec_obj *obj, u32 ret)
{
    if (MAD_ERROR_PLAY_END == ret) {
        obj->sound.enable |= B_DEC_PAUSE;
    }
}

u32 midi_ctrl_decode_api(void *p_file, void **ppdec, void *p_dp_buf)
{
    if (p_file != NULL) {
        return E_MIDI_FILEHDL;
    }

    dec_obj **p_dec = (dec_obj **)ppdec;
    u32 buff_len, sr;

    extern const decoder_ops_t midi_ctrl_ops;
    decoder_ops_t *ops = (decoder_ops_t *)&midi_ctrl_ops;
    if (!midi_ctrl_tone_tab) {
        return E_MIDI_NO_CFG;
    }

    local_irq_disable();
    memset(&dec_midi_ctrl_hld, 0, sizeof(dec_obj));
    local_irq_enable();

    dec_midi_ctrl_hld.type = D_TYPE_MIDI_CTRL;

    buff_len = ops->need_dcbuf_size();
    log_info("MIDI_CTRL Need Buff Len:%d\n", buff_len);//buff大小会随MAX_CTR_PLAYER_CNT改变
    if (buff_len > sizeof(midi_ctrl_decode_buff)) {
        return E_MIDI_DBUF;
    }
    /******************************************/
    cbuf_init(&cbuf_midi_ctrl, &obuf_midi_ctrl[0], sizeof(obuf_midi_ctrl));

    dec_midi_ctrl_hld.sr = 32000;
    dec_midi_ctrl_hld.p_file = p_file;
    dec_midi_ctrl_hld.sound.p_obuf = &cbuf_midi_ctrl;
    dec_midi_ctrl_hld.p_dbuf = MIDI_CTRL_CAL_BUF;
    dec_midi_ctrl_hld.dec_ops = ops;
    dec_midi_ctrl_hld.event_tab = (u8 *)&midi_evt[0];
    //
    midi_ctrl_t_parm.player_t = MAX_CTR_PLAYER_CNT;                                //设置需要合成的最多按键个数，8到32可配
    midi_ctrl_t_parm.sample_rate = 2;//0:48k,1:44.1k,2:32k,3:24k,4:22.050k,5:16k,6:12k,7:11.025k,8:8k
    midi_ctrl_t_parm.spi_pos = (u8 *)midi_ctrl_tone_tab;                    //spi_memory为音色文件数据起始地址

    midi_ctrl_parmt.output = mp_output;          //这个是最后的输出函数接口，
    midi_ctrl_parmt.tempo = 1000;
    midi_ctrl_parmt.track_num = 1;
    midi_ctrl_parmt.priv = &dec_midi_ctrl_hld;


    /******************************************/
    ops->open(MIDI_CTRL_CAL_BUF, (const struct if_decoder_io *)&midi_ctrl_parmt, (u8 *)&midi_ctrl_t_parm);        //传入io接口，说明如下

    /**输出dec handle*/
    *p_dec = &dec_midi_ctrl_hld;

    regist_dac_channel(&dec_midi_ctrl_hld.sound, kick_decoder); //注册到DAC;

    return 0;
}

void midi_ctrl_decode_init(void)
{
    void *pvfs = 0;
    void *pvfile = 0;
    u32 err = 0;

    err = vfs_mount(&pvfs, (void *)NULL, (void *)NULL);
    if (err != 0) {
        return;
    }

    err = vfs_openbypath(pvfs, &pvfile, "/midi_ctrl_prog/MIDI_CTRL.mda");
    if (err != 0) {
        log_info("midi ctrl mda open fail, try old midi_cfg.bin!\n");
        err = vfs_openbypath(pvfs, &pvfile, "/midi_cfg/00_MIDI.mda");
        if (err != 0) {
            vfs_fs_close(&pvfs);
            return;
        }
    }

    ///获取midi音色库的cache地址
    struct vfs_attr attr;
    vfs_get_attrs(pvfile, &attr);
    midi_ctrl_tone_tab = boot_info.sfc.app_addr + attr.sclust;

    vfs_file_close(&pvfile);
    vfs_fs_close(&pvfs);
}

typedef u32(*dec_open_t)(void *work_buf, const struct if_decoder_io *decoder_io, u8 *bk_point_ptr);
static u32 midi_ctrl_open(void *work_buf, void *dec_parm, void *parm)
{
    MIDI_CTRL_CONTEXT *ops = get_midi_ctrl_ops();
    return ops->open(work_buf, dec_parm, parm);
}
static u32 midi_ctrl_format_check(void *work_buf)
{
    return 0;
}
static u32 midi_ctrl_run(void *work_buf, u32 type)
{
    MIDI_CTRL_CONTEXT *ops = get_midi_ctrl_ops();
    return ops->run(work_buf);
}
static dec_inf_t *midi_ctrl_get_dec_inf(void *work_buf)
{
    return NULL;
}
static u32 midi_ctrl_get_playtime(void *work_buf)
{
    return 0;
}
static u32 midi_ctrl_get_bp_inf(void *work_buf)
{
    return 0;
}
static u32 midi_ctrl_need_dcbuf_size()
{
    MIDI_CTRL_CONTEXT *ops = get_midi_ctrl_ops();
    return ops->need_workbuf_size();
}
static u32 midi_ctrl_need_bpbuf_size()
{
    return 0xffffffff;
}
static u32 midi_ctrl_dec_confing(void *work_buf, u32 cmd, void *parm)
{
    MIDI_CTRL_CONTEXT *ops = get_midi_ctrl_ops();
    u32 ret = ops->ctl_confing(work_buf, cmd, parm);
    return ret;
}
u32 midi_ctrl_set_prog(void *work_buf, u8 prog, u8 chn)
{
    MIDI_CTRL_CONTEXT *ops = get_midi_ctrl_ops();
    dec_midi_ctrl_hld.sound.enable &= ~B_DEC_PAUSE;
    dec_midi_ctrl_hld.sound.enable |= B_DEC_KICK;
    u32 ret = ops->set_prog(work_buf, prog, chn);
    kick_decoder();
    return ret;
}
u32 midi_ctrl_note_on(void *work_buf, u8 nkey, u8 nvel, u8 chn)
{
    MIDI_CTRL_CONTEXT *ops = get_midi_ctrl_ops();
    dec_midi_ctrl_hld.sound.enable &= ~B_DEC_PAUSE;
    dec_midi_ctrl_hld.sound.enable |= B_DEC_KICK;
    u32 ret = ops->note_on(work_buf, nkey, nvel, chn);
    kick_decoder();
    return ret;
}
u32 midi_ctrl_note_off(void *work_buf, u8 nkey, u8 chn, u16 decay_time)
{
    MIDI_CTRL_CONTEXT *ops = get_midi_ctrl_ops();
    dec_midi_ctrl_hld.sound.enable &= ~B_DEC_PAUSE;
    dec_midi_ctrl_hld.sound.enable |= B_DEC_KICK;
    u32 ret = ops->note_off(work_buf, nkey, chn, decay_time);
    kick_decoder();
    return ret;
}
u32 midi_ctrl_vel_vibrate(void *work_buf, u8 nkey, u8 vel_step, u8 vel_rate, u8 chn)
{
    MIDI_CTRL_CONTEXT *ops = get_midi_ctrl_ops();
    dec_midi_ctrl_hld.sound.enable &= ~B_DEC_PAUSE;
    dec_midi_ctrl_hld.sound.enable |= B_DEC_KICK;
    u32 ret = ops->vel_vibrate(work_buf, nkey, vel_step, vel_rate, chn);
    kick_decoder();
    return ret;
}
u32 midi_ctrl_pitch_bend(void *work_buf, u16 pitch_val, u8 chn)
{
    MIDI_CTRL_CONTEXT *ops = get_midi_ctrl_ops();
    dec_midi_ctrl_hld.sound.enable &= ~B_DEC_PAUSE;
    dec_midi_ctrl_hld.sound.enable |= B_DEC_KICK;
    u32 ret = ops->pitch_bend(work_buf, pitch_val, chn);
    kick_decoder();
    return ret;
}

u8 *midi_ctrl_query_play_key(void *work_buf, u8 chn)
{
    MIDI_CTRL_CONTEXT *ops = get_midi_ctrl_ops();
    return ops->query_play_key(work_buf, chn);
}

static const decoder_ops_t midi_ctrl_ops = {
    .name = "midi_ctrl",
    .open = (dec_open_t)midi_ctrl_open,
    .format_check = midi_ctrl_format_check,
    .run = midi_ctrl_run,
    .get_dec_inf = midi_ctrl_get_dec_inf,
    .get_playtime = midi_ctrl_get_playtime,
    .get_bp_inf = midi_ctrl_get_bp_inf,
    .need_dcbuf_size = midi_ctrl_need_dcbuf_size,
    .need_bpbuf_size = midi_ctrl_need_bpbuf_size,
    .dec_confing = midi_ctrl_dec_confing,
};

extern const u8 midi_ctrl_buf_start[];
extern const u8 midi_ctrl_buf_end[];
u32 midi_ctrl_buff_api(dec_buf *p_dec_buf)
{
    p_dec_buf->start = (u32)&midi_ctrl_buf_start[0];
    p_dec_buf->end   = (u32)&midi_ctrl_buf_end[0];
    return 0;
}



