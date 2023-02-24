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

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[dac cpu]"
#include "log.h"

void dac_cpu_mode(void)
{
    apa_analog_open();
    SFR(JL_ADDA->DAA_CON1, 13, 2, 1);	    //APA_LSDTS_11V		00: 5U		01: 10U	       10: 15U	      11: 20U
}

int dac_low_power(void)
{
    char cpu_mode = 'A';
    cpu_mode = chip_versions();
    if (cpu_mode <  'C') {
        return 3600; //3.6V
    } else {
        return 3000; //3.0V
    }
}
u32 dac_mode_check(u32 con)
{
    char cpu_mode = 'A';
    cpu_mode = chip_versions();

    u32 dsm_pwm = con & DAC_MODE_BITS;
    if (cpu_mode <  'C') {
        switch (dsm_pwm) {
        case DAC_MODE_1_A:
            break;
        case DAC_MODE_1_B:
        case DAC_MODE_3_C:
        case DAC_MODE_3_D:
            log_error("err dac Work mode!!!\n");
            con = (con & ~(DAC_MODE_BITS)) | DAC_MODE_1_A;
            /* con = (con & ~DAC_MODE_BITS) | DAC_MODE_1_A; */
            break;
        default:
            break;
        }
    }
    return con;
}
