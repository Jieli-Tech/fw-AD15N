/* #include "resample_api.h" */
#include "notch_howling_api.h"
/* #include "resample.h" */
#include "decoder_api.h"
#include "config.h"
#include "sound_effect_api.h"
#include "howling_api.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"


void *notch_howling_api(void *obuf, u32 sr, void **ppsound)
{
    NH_PARA_STRUCT nhparm = {0};
    nhparm.depth          = 10;  //深度
    nhparm.bandwidth      = 45; //带宽
    nhparm.sampleRate     = sr;//采样率

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
    NH_PARA_STRUCT parm;
    u32 update;
    cbuffer_t cbuf;
} NH_HOWLING_HDL;


u32 notch_howling_work_buf[(9340 + 3) / 4] AT(.notch_howling_data);
NH_HOWLING_HDL notch_howling_hdl_save AT(.notch_howling_data);
u8 notch_howling_in_buf[160 * 2] AT(.notch_howling_data); //陷波器啸叫抑制，固定160点运算一次


void notch_howing_parm_update(NH_PARA_STRUCT *parm)
{
    NH_HOWLING_HDL *howling_hdl = &notch_howling_hdl_save;
    if (howling_hdl) {
        if (parm) {
            memcpy(&howling_hdl->parm, parm, sizeof(NH_PARA_STRUCT));
            howling_hdl->update = 1;
        }
    }
}

static int notch_howing_run(void *hld, short *inbuf, int len)
{
    int wlen = 0;
    NH_HOWLING_HDL *howling_hdl = &notch_howling_hdl_save;
    if (!howling_hdl) {
        return 0;
    }
    u32 tlen = len;
    u8 *p_inbuf = (void *)inbuf;
    while (0 != tlen) {
        wlen += cbuf_write(&howling_hdl->cbuf, &p_inbuf[wlen], tlen);
        tlen -= wlen;
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
}


void notch_howling_parm_debug(NH_HOWLING_HDL *howling_hdl)
{
    log_info("howling->parm.depth %d\n", howling_hdl->parm.depth);
    log_info("howling->parm.bandwidth %d\n", howling_hdl->parm.bandwidth);
    log_info("howling->parm.sampleRate %d\n", howling_hdl->parm.sampleRate);
}


void *notch_howling_phy(void *obuf, NH_PARA_STRUCT *parm, void **ppsound)
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
    buf_len = ops->need_buf();           //运算空间获取
    log_info("notch_howling work_buf_len %d\n", buf_len);
    memset(&notch_howling_hdl_save, 0x0, sizeof(NH_HOWLING_HDL));
    NH_HOWLING_HDL *howling_hdl = &notch_howling_hdl_save;
    if (howling_hdl && parm) {
        memcpy(&howling_hdl->parm, parm, sizeof(NH_PARA_STRUCT));
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

    cbuf_init(&howling_hdl->cbuf, &notch_howling_in_buf[0], sizeof(notch_howling_in_buf));
    return howling_obj;
}
