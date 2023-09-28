#include "encoder_mge.h"
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "hwi.h"
#include "app_modules.h"
#include "circular_buf.h"

#if defined(ENCODER_UMP3_EN) && (ENCODER_UMP3_EN)

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"




static cbuffer_t cbuf_emp3_o AT(.enc_mp3_data);
static u8 obuf_emp3_o[1024] AT(.enc_mp3_data) ;

static u32 mp3_encode_buff[830] AT(.enc_mp3_data) ;

static enc_obj enc_mp3_hdl;

static const EN_FILE_IO mp3_enc_io = {
    &enc_mp3_hdl,      //input跟output函数的第一个参数，解码器不做处理，直接回传，可以为NULL
    enc_input,
    enc_output,
};

u32 ump3_encode_api(void *p_file)
{
    u32 buff_len, i;
    ENC_OPS *ops;
    log_info("ump3_encode_api\n");
    ops = get_mp2_ops();
    buff_len = ops->need_buf();
    if (buff_len > sizeof(mp3_encode_buff)) {
        return 0;
    }
    /******************************************/
    cbuf_init(&cbuf_emp3_o, &obuf_emp3_o[0], sizeof(obuf_emp3_o));
    log_info("A\n");
    // log_info("B\n");
    enc_mp3_hdl.p_file = p_file;
    enc_mp3_hdl.p_ibuf = REC_ADC_CBUF;//adc_hdl.p_adc_cbuf;//&cbuf_emp3_i;
    enc_mp3_hdl.p_obuf = &cbuf_emp3_o;
    enc_mp3_hdl.p_dbuf = &mp3_encode_buff[0];
    enc_mp3_hdl.enc_ops = ops;
    enc_mp3_hdl.info.sr = read_audio_adc_sr();
    /*br的范围是：sr*16/压缩比，压缩比的范围是3~8
     * 例如sr = 24k，那么br的范围是 (24*16/8) ~ (24*16/3)*/
    enc_mp3_hdl.info.br = 80;


    log_info("D\n");
    /******************************************/
    ops->open((void *)&mp3_encode_buff[0], &enc_mp3_hdl.info, (void *)&mp3_enc_io);           //传入io接口，说明如下
    /* enc_mp3_hdl.enable = B_ENC_ENABLE; */
    //debug_u32hex(enc_mp3_hdl.enable);
    return (u32)&enc_mp3_hdl;
}
#endif
