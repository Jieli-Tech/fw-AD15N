
#include "cpu.h"
#include "config.h"
#include "typedef.h"
/* #include "decoder_api.h" */
/* #include "dev_manage.h" */
#include "audio.h"
#include "dac.h"
#include "sound_effect_api.h"
#include "circular_buf.h"
#include "mio_api.h"
/* #include "energe_api.h" */


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

void sound_output_hook(void *priv, void *data, int len)
{
    sound_out_obj *psound = priv;
    d_mio_start(psound->mio);

    /* energe_run_api(data, len); */
}

//出数接口,data是出数的起始地址，len是长度（byte），priv为传入的
int sound_output(void *priv, void *data, int len)
{
    sound_out_obj *psound = priv;
    void *obuf;
    u32 dlen = 0;
    if (B_DEC_EFFECT & psound->enable) {
        /* debug_puts("A"); */
        EFFECT_OBJ *e_obj;
        e_obj = (void *)psound->effect;
        if (NULL != e_obj) {
            /* debug_puts("B"); */
            dlen = e_obj->run(e_obj->p_si, data, len);
            goto __mp_output_end;
        }
        /* debug_puts("C"); */
    }
    /* debug_puts("D"); */
    obuf = psound->p_obuf;
    dlen = cbuf_write(obuf, data, len);

    sound_output_hook(psound, data, len);
__mp_output_end:
    return dlen; //返回输出了多少个bye，没输完的下次继续输出
}

int sound_input(void *priv, void *data, int len)
{
    sound_out_obj *psound = priv;
    void *obuf;
    u32 dlen = 0;
    if (B_DEC_RUN_EN & psound->enable) {
        /* debug_puts("D"); */
        obuf = psound->p_obuf;
        dlen = cbuf_read(obuf, data, len);
    }
    return dlen; //返回输出了多少个bye，没输完的下次继续输出
}



