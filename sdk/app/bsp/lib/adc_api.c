/***********************************Jieli tech************************************************
  File : adc_api.c
  By   : liujie
  Email: liujie@zh-jieli.com
  date : 2019-1-14
********************************************************************************************/
/* #include "adc_api.h" */
/* #include "adc.h" */
#include "circular_buf.h"
#include "string.h"
#include "uart.h"
#include "config.h"
#include "audio.h"
#include "audio_adc.h"
#include "audio_analog.h"
#include "sound_effect_api.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"


#define ADC_CHANNEL_NUMBER 2

typedef struct _AUDIO_ADC_MANAGE {
    // void *obuf[ADC_CHANNEL_NUMBER];
    sound_out_obj *sound[ADC_CHANNEL_NUMBER];
    void (*kick[ADC_CHANNEL_NUMBER])(void *);
    /* void *cbuff; */
    /* u8 vol[ADC_CHANNEL_NUMBER]; */
    u8 flag;
} AUDIO_ADC_MANAGE;

AUDIO_ADC_MANAGE audio_adc_mge AT(.AUDIO_ADC_BUFFER);


#define ADC_PACKET_SIZE 32
u8 double_audio_adc_buf[ADC_PACKET_SIZE * 2 * 2] AT(.AUDIO_ADC_BUFFER);
/*----------------------------------------------------------------------------*/
/**@brief   adc模块初始化
   @param   无
   @return  无
   @author  liujie
   @note    void adc_init_api(void)
*/
/*----------------------------------------------------------------------------*/
void audio_adc_mode_init(void)
{
    memset(&audio_adc_mge, 0, sizeof(audio_adc_mge));
    audio_adc_resource_init(double_audio_adc_buf, sizeof(double_audio_adc_buf), 25);
}

u32 audio_adc_init_api(u32 sr, ADC_MODE mode, u32 ch)
{
    u32 res = 0;

    if (ADC_MIC == mode) {
        JL_PORTB->DIR |= BIT(1);
        JL_PORTB->DIE &= ~BIT(1);
        JL_PORTB->DIEH &= ~BIT(1);
        JL_PORTB->PU &= ~BIT(1);
        JL_PORTB->PD &= ~BIT(1);
    } else if (ADC_LINE_IN == mode) {
        if (BIT(0) & ch) {
            JL_PORTA->DIR |= BIT(13);
            JL_PORTA->DIE &= ~BIT(13);
            JL_PORTA->DIEH &= ~BIT(13);
            JL_PORTA->PU &= ~BIT(13);
            JL_PORTA->PD &= ~BIT(13);

        }
        if (BIT(1) & ch) {
            JL_PORTA->DIR |= BIT(14);
            JL_PORTA->DIE &= ~BIT(14);
            JL_PORTA->DIEH &= ~BIT(14);
            JL_PORTA->PU &= ~BIT(14);
            JL_PORTA->PD &= ~BIT(14);

        }
    }
    res = adc_analog_open(mode, ch);
    /* void dac_analog_init_t(); */
    /* dac_analog_init_t(); */
    if (0 == res) {
        res = audio_adc_phy_init(sr);
    }
    return res;

}
void audio_adc_off_api(void)
{
    audio_adc_phy_off();
    adc_analog_close();
}

/* extern void kick_encode_api(void); */
/* //弱定义函数，定义不可修改，由扩音模式重新实现 */
/* __attribute__((weak))void kick_loudsperaker(void *_sound)AT(.audio_a.text.cache.L2) */
/* { */

/* } */
AT(.audio_a.text.cache.L2)
void fill_audio_adc_fill(u8 *buf, u32 len)
{
    u32 i;
    for (i = 0; i < ADC_CHANNEL_NUMBER; i++) {
        if (0 == (audio_adc_mge.flag & BIT(i))) {
            continue;
        }
        if (0 == (audio_adc_mge.sound[i]->enable & B_DEC_RUN_EN)) {
            continue;
        }
        cbuf_write(audio_adc_mge.sound[i]->p_obuf, buf, len);
        /* if (audio_adc_mge.sound[i]->enable & B_REC_RUN) { */
        if (NULL !=  audio_adc_mge.kick[i]) {
            audio_adc_mge.kick[i](audio_adc_mge.sound[i]);
        }
        /* kick_encode_api(); */
        /* } */

        /* if (audio_adc_mge.sound[i]->enable & B_LOUDSPEAKER) { */
        /* kick_loudsperaker((void *)audio_adc_mge.sound[i]); */
        /* } */
    }
}

bool regist_audio_adc_channel(void *psound, void *kick)
{
    u8 i;
    for (i = 0; i < ADC_CHANNEL_NUMBER; i++) {
        if (audio_adc_mge.flag & BIT(i)) {
            continue;
        }
        audio_adc_mge.sound[i] = psound;
        audio_adc_mge.kick[i] = kick;
        audio_adc_mge.flag |= BIT(i);
        return true;
    }
    return false;
}

bool unregist_audio_adc_channel(void *psound)
{
    u8 i;
    sound_out_obj *ps = psound;

    for (i = 0; i < ADC_CHANNEL_NUMBER; i++) {
        if (0 == (audio_adc_mge.flag & BIT(i))) {
            continue;
        }

        if (audio_adc_mge.sound[i] == psound) {
            local_irq_disable();
            audio_adc_mge.flag &= ~BIT(i);
            /* audio_adc_mge.obuf[i] = 0; */
            audio_adc_mge.sound[i] = 0;
            audio_adc_mge.kick[i] = NULL;
            ps->enable &= ~B_DEC_OBUF_EN;
            local_irq_enable();
            break;
        }
    }
    return true;
}




