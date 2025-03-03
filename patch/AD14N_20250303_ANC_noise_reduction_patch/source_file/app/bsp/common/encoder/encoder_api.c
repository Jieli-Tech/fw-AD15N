#include "encoder_mge.h"
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "hwi.h"
#include "dev_manage.h"
/* #include "fs_io.h" */
#include "vfs.h"
#include "circular_buf.h"
#include "a_encoder.h"
#include "mp3_encoder.h"
#include "app_modules.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#if (defined(ANS_EN) && (ANS_EN))
#include "ans_api.h"
cbuffer_t cbuf_ans AT(.ans_data);
u32 ans_buff[512 * 5 / 4] AT(.ans_data);
#endif

cbuffer_t cbuf_adc AT(.rec_data);
u32 adc_buff[512 * 10 / 4] AT(.rec_data) ;

#if (0 == FPGA)
sound_out_obj rec_sound;
sound_out_obj enc_in_sound;

#define START_ADC_RUN  rec_sound.enable |= (B_DEC_RUN_EN | B_REC_RUN)
#define STOP_ADC_RUN  rec_sound.enable &= ~(B_DEC_RUN_EN | B_REC_RUN)


void rec_phy_init(void)
{
    memset(&rec_sound, 0, sizeof(rec_sound));
    memset(&enc_in_sound, 0, sizeof(enc_in_sound));
    cbuf_init(&cbuf_adc, &adc_buff[0], sizeof(adc_buff));
    enc_in_sound.p_obuf = &cbuf_adc;
    rec_sound.p_obuf = &cbuf_adc;

#if (defined(ANS_EN) && (ANS_EN))
    cbuf_init(&cbuf_ans, &ans_buff[0], sizeof(ans_buff));
    enc_in_sound.p_obuf = &cbuf_ans;

    ans_init(&cbuf_adc, &cbuf_ans, kick_encode_api);
    regist_audio_adc_channel(&rec_sound, (void *) ans_check_kick_start); //注册到ADC;
#else
    regist_audio_adc_channel(&rec_sound, (void *) kick_encode_api); //注册到ADC;

#endif
}
void rec_phy_suspend(void)
{

#if (defined(ANS_EN) && (ANS_EN))
    ans_deinit();
#endif
    unregist_audio_adc_channel(&rec_sound);
}

AT(.audio_a.text.cache.L2)
void kick_encode_api(void *obj)
{
    kick_encode_isr();
}
#endif



enc_obj *enc_hdl;

extern const char MIC_PGA_G;
void start_encode(void)
{
    START_ADC_RUN;
    audio_adc_enable(MIC_PGA_G);
}
void stop_encode(void *pfile, u32 dlen)
{
    enc_obj *obj = enc_hdl;
    u32 err;
    audio_adc_disable();
    STOP_ADC_RUN;
    log_info("stop encode\n");
    if (NULL == enc_hdl) {
        rec_phy_suspend();
        return;
    }
    obj->enable |= B_ENC_STOP;

    log_info("stop encode A\n");
    while (0 != cbuf_get_data_size(obj->p_ibuf)) {
        if (obj->enable & B_ENC_FULL) {
            break;
        }
        kick_encode_isr();
        delay(100);
    }

    log_info("stop encode C\n");
    while (0 != cbuf_get_data_size(obj->p_obuf)) {
        if (obj->enable & B_ENC_FULL) {
            break;
        }
        kick_wfile_isr();
        delay(100);
    }
    log_info("stop encode D\n");
    obj->enable &= ~B_ENC_ENABLE;
    HWI_Uninstall(IRQ_SOFT1_IDX);
    HWI_Uninstall(IRQ_SOFT2_IDX);

    u32 flen = dlen;
    err = vfs_ioctl(pfile, FS_IOCTL_FILE_SYNC, (int)&flen);
    rec_phy_suspend();
    enc_hdl = 0;
}

/* static u32 recfil_index; */
void encoder_io(u32(*fun)(void *), void *pfile)
{
    s32 err;
    rec_phy_init();

    enc_hdl = (void *)fun(pfile);
    if (0 != enc_hdl) {
        enc_phy_init();
        enc_hdl->enable = B_ENC_ENABLE;
        start_encode();//adc_enable();
        log_info("encode succ: \n");
    } else {
        log_info("encode fail \n");
    }
    //while(1)clear_wdt();
}

#if 0
/*----------------------------------------------------------------------------*/
/**@brief   encode写中断hook函数
   @param   *hdl  : 录音设备句柄
   @return
   @author  chenzhuohao
   @note    若设备写入速度较慢导致看门狗复位，可通过该函数在写入前喂狗
   			注意：写入前喂狗可能使系统无法回到主循环,导致无法响应其他事件！
**/
/*----------------------------------------------------------------------------*/
void wfil_soft2_isr_hook(enc_obj *hdl)
{

}
#endif
