/***********************************Jieli tech************************************************
  File : dac_api.c
  By   : liujie
  Email: liujie@zh-jieli.com
  date : 2019-1-14
********************************************************************************************/
#include "dac_api.h"
#include "dac.h"
#include "config.h"
#include "audio.h"
#include "audio_analog.h"

void dac_cpu_mode(void)
{
    if (DAC_MODE_5_A == (JL_ADDA->DAC_CON0 & (7 << 5))) {
        rdac_analog_open();
    } else {
        apa_analog_open();
    }
}

