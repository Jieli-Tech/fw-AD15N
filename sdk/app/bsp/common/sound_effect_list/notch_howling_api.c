/* #include "resample_api.h" */
#include "notch_howling_api.h"
/* #include "resample.h" */
#include "decoder_api.h"
#include "config.h"
#include "sound_effect_api.h"
#include "howling_api.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"


void *notch_howling_api(void *obuf, u32 sr, void **ppsound)
{
    /* 调试顺序：gain -> Q -> threshold。gain压制越多，Q越小会容易出现说话过程中声音断续的问题 */
    NotchHowlingParam nhparm    = {0};
    nhparm.gain                 = (int)(-20.0 * (1 << 20)); //陷波器压制程度，越大放啸叫越好，但发声啸叫频点误检时音质会更差
    nhparm.Q                    = (int)(0.3 * (1 << 24));   //陷波器带宽，越小放啸叫越好，但发声啸叫频点误检时音质会更差
    nhparm.fade_time            = 1000;                     //启动时间与施放时间，越小启动与释放越快，可能导致杂音出现切音质变差
    nhparm.threshold            = (int)(25.0 * (1 << 15));  //频点啸叫判定阈值，越小越容易判定啸叫频点，但可能误检导致音质变差
    nhparm.SampleRate           = sr;                       //采样率

    return notch_howling_phy(obuf, &nhparm, ppsound);
}

void *link_notch_howling_sound(void *p_sound_out, void *p_dac_cbuf, void **pp_effect, u32 sr)
{
    sound_out_obj *p_next_sound = 0;
    sound_out_obj *p_curr_sound = p_sound_out;
    p_curr_sound->effect = notch_howling_api(p_curr_sound->p_obuf, sr, (void **)&p_next_sound);
    if (NULL != p_curr_sound->effect) {
        if (NULL != pp_effect) {
            *pp_effect = p_curr_sound->effect;
        }
        p_curr_sound->enable |= B_DEC_EFFECT;
        p_curr_sound = p_next_sound;
        p_curr_sound->p_obuf = p_dac_cbuf;
    } else {
        log_info("echo init fail\n");
    }
    return p_curr_sound;
}


/****************************phy********************************************************************/
#include "circular_buf.h"
typedef struct _NH_HOWLING_HDL {
    EFFECT_OBJ obj;//必须在第一个
    sound_in_obj si;
    NH_IO_CONTEXT io;
    NotchHowlingParam parm;
    u32 update;
    cbuffer_t cbuf;
} NH_HOWLING_HDL;


u32 notch_howling_work_buf[(3152 + 3) / 4] AT(.notch_howling_data);
NH_HOWLING_HDL notch_howling_hdl_save AT(.notch_howling_data);
/* u8 notch_howling_in_buf[160 * 2] AT(.notch_howling_data); //陷波器啸叫抑制，固定160点运算一次 */


void notch_howing_parm_update(NotchHowlingParam *parm)
{
    NH_HOWLING_HDL *howling_hdl = &notch_howling_hdl_save;
    if (howling_hdl) {
        if (parm) {
            memcpy(&howling_hdl->parm, parm, sizeof(NotchHowlingParam));
            howling_hdl->update = 1;
        }
    }
}

static int notch_howing_run(void *hld, short *inbuf, int len)
{
    NH_HOWLING_HDL *howling_hdl = &notch_howling_hdl_save;
    if (!howling_hdl) {
        return 0;
    }

    NH_STRUCT_API *ops;
    int res = 0;
    sound_in_obj *p_si = hld;
    ops = (NH_STRUCT_API *)p_si->ops;
    res = ops->run(p_si->p_dbuf, inbuf, len);//len 为indata 字节数，返回值为实际消耗indata的字节数

    if (howling_hdl->update) {
        howling_hdl->update = 0;
        ops->update(p_si->p_dbuf, &howling_hdl->parm);
    }

    return res;

#if 0
    int wlen = 0;
    NH_HOWLING_HDL *howling_hdl = &notch_howling_hdl_save;
    if (!howling_hdl) {
        return 0;
    }
    u32 tlen = len;
    u8 *p_inbuf = (void *)inbuf;
    while (0 != tlen) {
        u32 cbuf_wlen = cbuf_write(&howling_hdl->cbuf, &p_inbuf[wlen], tlen);
        wlen += cbuf_wlen;
        tlen -= cbuf_wlen;
        NH_STRUCT_API *ops;
        sound_in_obj *p_si = hld;
        ops = (NH_STRUCT_API *)p_si->ops;
        int res = 0;
        u32 rlen = 0;
        s16 *data = cbuf_read_alloc(&howling_hdl->cbuf, &rlen);
        if (rlen >= 320) {
            res = ops->run(p_si->p_dbuf, data, 320);//陷波啸叫抑制每次处理长度固定160点
            //log_info("res %d\n", res);
        }
        cbuf_read_updata(&howling_hdl->cbuf, res);

        if (howling_hdl->update) {
            howling_hdl->update = 0;
            ops->update(p_si->p_dbuf, &howling_hdl->parm);
        }
    }

    return wlen;
#endif
}


void notch_howling_parm_debug(NH_HOWLING_HDL *howling_hdl)
{
    log_info("howling->parm.gain %d", howling_hdl->parm.gain);
    log_info("howling->parm.Q %d", howling_hdl->parm.Q);
    log_info("howling->parm.fade_time %d", howling_hdl->parm.fade_time);
    log_info("howling->parm.threshold %d", howling_hdl->parm.threshold);
    log_info("howling->parm.SampleRate %d\n", howling_hdl->parm.SampleRate);
}


void *notch_howling_phy(void *obuf, NotchHowlingParam *parm, void **ppsound)
{
    u32 buf_len, i;
    NH_STRUCT_API *ops;
    EFFECT_OBJ *howling_obj;
    sound_in_obj *howling_si;
    if (!parm) {
        log_error("notch howling parm NULL\n");
        return NULL;
    }
    ops = (NH_STRUCT_API *)get_notchHowling_ops(); //接口获取
    buf_len = ops->need_buf(parm);           //运算空间获取
    log_info("notch_howling work_buf_len %d\n", buf_len);
    memset(&notch_howling_hdl_save, 0x0, sizeof(NH_HOWLING_HDL));
    NH_HOWLING_HDL *howling_hdl = &notch_howling_hdl_save;
    if (howling_hdl && parm) {
        memcpy(&howling_hdl->parm, parm, sizeof(NotchHowlingParam));
        notch_howling_parm_debug(howling_hdl);
    }
    unsigned int *howling_hdl_ptr = (unsigned int *)notch_howling_work_buf;
    log_info("notch howling_hdl_ptr %x\n", howling_hdl_ptr);
    if (sizeof(notch_howling_work_buf) < buf_len) {
        log_error("notch howing work buf less %d, need len %d", sizeof(notch_howling_work_buf), buf_len);
        return NULL;
    }
    howling_obj = &howling_hdl->obj;
    howling_hdl->io.priv = &howling_obj->sound;
    howling_hdl->io.output = sound_output;

    /*************************************************/
    howling_si =  &howling_hdl->si;
    howling_si->ops = ops;
    howling_si->p_dbuf = howling_hdl_ptr;
    /*************************************************/
    howling_obj->p_si = howling_si;
    howling_obj->run = notch_howing_run;
    howling_obj->sound.p_obuf = obuf;
    *ppsound = &howling_obj->sound;

    ops->open(howling_hdl_ptr, &howling_hdl->parm, &howling_hdl->io);
    /* cbuf_init(&howling_hdl->cbuf, &notch_howling_in_buf[0], sizeof(notch_howling_in_buf)); */
    return howling_obj;
}
