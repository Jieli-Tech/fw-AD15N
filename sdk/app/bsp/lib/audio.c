/***********************************Jieli tech************************************************
  File : dac.c
  By   : liujie
  Email: liujie@zh-jieli.com
  date : 2019-1-14
********************************************************************************************/
#include "string.h"
#include "dac.h"
#include "audio_adc.h"
#include "hwi.h"
#include "sfr.h"
#include "cpu.h"
#include "config.h"
#include "audio.h"
#include "audio_analog.h"
#include "dac_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[audio]"
#include "debug.h"


extern void dac_isr_t(void);

SET(interrupt(""))
AT(.dac_oput_code)
void audio_isr(void)
{
    /* dac_isr(); */
    dac_isr_t();
    audio_adc_isr();

}

void audio_init(void)
{
    HWI_Install(IRQ_AUDIO_IDX, (u32)audio_isr, IRQ_AUDIO_IP) ;
    JL_CLK->CON1 &= ~AUDIO_CLKDIV_BITS;
    JL_CLK->CON1 |= AUDIO_CLK_PLL48M;
    audio_analog_open();
}
void audio_off(void)
{
    dac_off_api();
    audio_adc_off_api();
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

void dac_power_on()
{
    audio_init();
    dac_init_api(32000);
}







