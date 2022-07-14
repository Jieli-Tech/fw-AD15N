/***********************************Jieli tech************************************************
  File : dac.c
  By   : liujie
  Email: liujie@zh-jieli.com
  date : 2019-1-14
********************************************************************************************/
#include "string.h"
#include "dac.h"
#include "hwi.h"
#include "sfr.h"
#include "cpu.h"
#include "config.h"
#include "audio.h"
#include "audio_analog.h"
#include "dac_api.h"

#if AUDIO_ADC_EN
#include "audio_adc.h"
#endif

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[audio]"
#include "debug.h"


extern void dac_isr_t(void);

SET(interrupt(""))
AT(.audio_d.text.cache.L2)
void audio_isr(void)
{

    /* dac_isr(); */
    dac_isr_t();
#if AUDIO_ADC_EN
    audio_adc_isr();
#endif


}

void audio_init(void)
{
    /* log_info("audio_init"); */
    HWI_Install(IRQ_AUDIO_IDX, (u32)audio_isr, IRQ_AUDIO_IP) ;
    /* log_info("audio_clk_init"); */
    audio_clk_init();
    /* log_info("audio_analog_open"); */
    audio_analog_open();
}
void audio_off(void)
{
    dac_off_api();
#if AUDIO_ADC_EN
    audio_adc_off_api();
#endif
    audio_analog_close();
}

void audio_lookup(void)
{
    audio_analog_lookup(audio_analog, 1);
}

void dac_power_off()
{
    audio_off();
}

void dac_power_on(u32 sr, bool delay_flag)
{
    audio_init();
    dac_init_api(sr, delay_flag);
}







