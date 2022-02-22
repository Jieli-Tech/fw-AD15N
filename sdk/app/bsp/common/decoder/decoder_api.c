#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "hwi.h"
#include "decoder_api.h"
#include "vfs.h"
#include "circular_buf.h"
#include "audio.h"
#include "dac.h"
#include "ump3_api.h"
#include "midi_api.h"
#include "list/midi_ctrl_api.h"
#include "f1a_api.h"
#include "a_api.h"
#include "mp3_standard_api.h"
#include "wav_api.h"
#include "msg.h"
#include "speed_api.h"
#include "src_api.h"
#include "errno-base.h"
#include "decoder_msg_tab.h"
#include "eq.h"
#include "mio_api.h"
#include "app_config.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"

extern u32 ump3_buff_api(dec_buf *p_dec_buf);;

AT(.dac_oput_code)
u32 dec_hld_tab[] = {
    F1A1_LST
    F1A2_LST
    UMP3_LST
    A_LST
    MIDI_LST
    MIDI_CTRL_LST
    WAV_LST
    MP3_ST_LST
};


const u32 decoder_tab[] = {
    F1A1_API
    F1A2_API
    UMP3_API
    A_API
    MIDI_API
    MIDI_CTRL_API
    WAV_API
    MP3_ST_API
};


const u32 decoder_mutual[] = {
    F1A1_MUT_TAB
    F1A2_MUT_TAB
    UMP3_MUT_TAB
    A_MUT_TAB
    MIDI_MUT_TAB
    MIDI_CTRL_MUT_TAB
    WAV_MUT_TAB
    MP3_ST_MUT_TAB
};

void decoder_init(void)
{
    u32 i;
    dec_obj *obj;
    u8 dc;
    dc = sizeof(dec_hld_tab) / 4;

    for (i = 0; i < dc; i++) {
        obj = (void *)dec_hld_tab[i];
        memset(obj, 0, sizeof(dec_obj));
    }
    decoder_channel_set(dc);
    HWI_Install(IRQ_SOFT0_IDX, (u32)decoder_soft0_isr, IRQ_DECODER_IP) ;
}

void decoder_mutex(u32 index)
{
    dec_buf cw;
    dec_buf cl;
    u32 max_loop = sizeof(decoder_mutual) / sizeof(decoder_mutual[0]);
    if (index >= max_loop) {
        return;
    }
    u32(*fun)(dec_buf * p) = (void *)decoder_mutual[index];
    fun(&cw);
    for (u32 i = 0; i < max_loop; i++) {
        if (i == index) {
            continue;
        }
        fun = (void *)decoder_mutual[i];
        fun(&cl);
        if ((cl.start >= cw.end) || (cw.start >= cl.end)) {
            continue;
        }
        /* log_info("start 0x%x; end  0x%x", cl.start, cl.end); */
        /* log_info("start 0x%x; end  0x%x", cw.start, cw.end); */
        /*  */
        /* log_info("decoder mutex : %d %d\n", index, i); */
        decoder_stop_now((void *)dec_hld_tab[i]);
    }
}
dec_obj *decoder_io(void *pfile, u32 dec_ctl, dp_buff *dbuff, u8 loop)
{
    u32(*fun)(void *, void **, void *);
    u32 res, dec_i, j;


    int file_len = vfs_file_name(pfile, (void *)g_file_sname, sizeof(g_file_sname));
    if (check_ext_api(g_file_sname, ".mio", 4)) {
        return NULL;
    }

    /* log_info("\n************\ndecoder_fun"); */


#if HAS_MIO_EN
    u32 mio_res = -1;
    void *mio_pfile = NULL;
    if (pfile) {
        mio_res = vfs_openbyfile(pfile, &mio_pfile, "mio");
    }
#endif

    dec_obj *p_dec = 0;
    sound_out_obj *p_curr_sound = 0;
    sound_out_obj *p_next_sound = 0;
    res = E_DECODER;
    for (dec_i = 0; dec_i < (sizeof(decoder_tab) / 4); dec_i++) {
        if (0 == (dec_ctl & BIT(dec_i))) {
            continue;
        }
        //启动解码时，将其他与之互斥的解码停止
        decoder_mutex(dec_i);

        vfs_seek(pfile, 0, SEEK_SET);
        fun = (void *)decoder_tab[dec_i];
        p_dec = 0;
        res = fun(pfile, (void **)(&p_dec), check_dp(dbuff));
        if (0 == res) {
            break;
        }
    }

    if (0 == res) {
        // 设置解码参数
        u32 flen = 0;
        vfs_get_fsize(pfile, &flen);
        /* log_info("flen:%d \n", flen); */
        if (flen) {
            decoder_set_file_size(p_dec, flen);
        }
        p_curr_sound = &p_dec->sound;
        p_curr_sound->enable = 0;
        sound_out_obj *first_sound = p_curr_sound;
        void *cbuff_o = p_dec->sound.p_obuf;
#if AUDIO_SPEED_EN
        //变速变调
        if (dec_ctl & BIT_SPEED) {
            p_curr_sound->effect = speed_api(cbuff_o, p_dec->sr, (void **) &p_next_sound);
            if (NULL != p_curr_sound->effect) {
                p_curr_sound->enable |= B_DEC_EFFECT;
                p_curr_sound = p_next_sound;
                p_curr_sound->p_obuf = cbuff_o;
                p_next_sound = 0;
                /* log_info("src init succ\n"); */
            }
        }
#endif

        //硬件src
        p_curr_sound->enable = 0;
        if (32000 != p_dec->sr) {
            p_curr_sound = link_src_sound(p_curr_sound, cbuff_o, (void **) &p_dec->src_effect, p_dec->sr, 32000);
        } else {
            void *src_tmp = src_hld_malloc();
            src_reless((void **)&src_tmp);
            //log_info("do't need src\n");
        }
#if HAS_MIO_EN
        if (0 == mio_res) {
            d_mio_open(&first_sound->mio, mio_pfile, (void *)mio_a_hook_init);
        }
#endif

        p_curr_sound->mio = p_dec->sound.mio;

        clear_dp(dbuff);
        if (0 != loop) { // (dec_ctl & BIT_LOOP)
            p_dec->loop = loop;
            //log_info("get loop dp\n");
            if (true == get_dp(p_dec, dbuff)) {
                //log_info(" -loop save succ!\n");
                p_dec->p_dp_buf = check_dp(dbuff);
            } else {
                log_info(" -loop save fail!\n");
            }
        }

        p_dec->sound.enable |= B_DEC_ENABLE | B_DEC_KICK;
        kick_decoder();
        log_info("decode succ \n");
    } else {
        log_info("decode err : 0x%x\n", res);
#if HAS_MIO_EN
        if (0 == mio_res) {
            vfs_file_close(&mio_pfile);
        }
#endif
    }
    dac_fade_in_api();
    //while(1)clear_wdt();
    return p_dec;
}

__attribute__((weak))
void midi_error_play_end_cb(dec_obj *obj, u32 ret)
{

}

void irq_decoder_ret(dec_obj *obj, u32 ret)
{
    if (MAD_ERROR_PLAY_END == ret) {
        midi_error_play_end_cb(obj, ret);
        return;
    }
    if (0 != ret) {
        log_info("decoder ret : 0x%x\n", ret);
        if (MAD_ERROR_F1X_START_ADDR == ret) {
            /* ret = MAD_ERROR_PLAY_END;  */
            post_event(obj->event_tab[MAD_ERROR_PLAY_END & 0x0f]);
        } else {
            post_event(obj->event_tab[ret & 0x0f]);
        }
    }
    switch (ret) {
    case MAD_ERROR_FILE_END:
    case MAD_ERROR_SYNC_LIMIT:
    case MAD_ERROR_F1X_START_ADDR:
        obj->sound.enable |= B_DEC_ERR;
        log_info("file end\n");
        break;

    }
}

void decoder_soft_hook(void)
{
    d_mio_run();
}

void decoder_pause(dec_obj *obj)
{
    obj->sound.enable ^= B_DEC_PAUSE;
    if (0 == (obj->sound.enable & B_DEC_PAUSE)) {
        obj->sound.enable |= B_DEC_KICK;
        kick_decoder();
    }
}

void decoder_stop_phy(dec_obj *obj, DEC_STOP_WAIT wait, bool fade)
{
    if (NULL == obj) {
        return;
    }
    /* log_info("decode stop --\n"); */
    if (obj->sound.enable & B_DEC_RUN_EN) {
        clear_dp_buff(obj->p_dp_buf);
    }
    /* log_info("decode stop\n"); */
    obj->sound.enable &= ~B_DEC_RUN_EN;
    dac_fade_out_api(200);
    if (NO_WAIT != wait) {
        log_info("decode stop wait!\n");
        while (obj->sound.enable & B_DEC_OBUF_EN) {
            if (false == dac_cbuff_active(&obj->sound)) {
                break;
            }
        }
        /* log_info("decode stop wait ok!\n"); */
    } else {
        /* log_info("decode stop no wait!\n"); */
    }

    d_mio_close(&obj->sound.mio);
    unregist_dac_channel(&obj->sound);
    if (NULL != obj->src_effect) {
        src_reless(&obj->src_effect);
    }
}
void decoder_stop(dec_obj *obj, DEC_STOP_WAIT wait)
{
    decoder_stop_phy(obj, wait, 1);
}

void decoder_stop_now(dec_obj *obj)
{
    decoder_stop_phy(obj, NO_WAIT, 1);
}

void decoder_ff(dec_obj *obj, u8 step)
{
    if (!(obj->function & DEC_FUNCTION_FF_FR)) {
        // 不支持快进快退
        return ;
    }
    if (obj->sound.enable & B_DEC_PAUSE) {
        // 如果是暂停状态，改为播放
        decoder_pause(obj);
    }
    // 设置步长
    obj->ff_fr_step = step;
}

void decoder_fr(dec_obj *obj, u8 step)
{
    if (!(obj->function & DEC_FUNCTION_FF_FR)) {
        // 不支持快进快退
        return ;
    }
    if (obj->sound.enable & B_DEC_PAUSE) {
        // 如果是暂停状态，改为播放
        decoder_pause(obj);
    }
    // 设置步长
    obj->ff_fr_step = 0 - step;
}

void decoder_set_file_size(dec_obj *obj, u32 size)
{
    decoder_ops_t *ops;
    ops = obj->dec_ops;
    AUDIO_FLEN_PARA file_parm;
    file_parm.flen = size;
    ops->dec_confing(obj->p_dbuf, SET_FILE_TOTAL_LEN, &file_parm);
}


