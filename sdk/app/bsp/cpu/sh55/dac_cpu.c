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
    apa_analog_open();
    SFR(JL_ADDA->DAA_CON1, 13, 2, 1);	    //APA_LSDTS_11V		00: 5U		01: 10U	       10: 15U	      11: 20U
}

