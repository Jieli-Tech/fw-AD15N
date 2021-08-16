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
#include "debug.h"


void dac_cpu_mode(void)
{
    if (DAC_MODE_5_A == (JL_ADDA->DAC_CON0 & (7 << 5))) {
        JL_PORTB->DIR |= BIT(0);
        JL_PORTB->DIE &= ~BIT(0);
        JL_PORTB->DIEH &= ~BIT(0);
        JL_PORTB->PU &= ~BIT(0);
        JL_PORTB->PD &= ~BIT(0);
        rdac_analog_open();
    } else {
        apa_analog_open();
    }
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

    if (cpu_mode <  'C') {
        u32 dsm_pwm = con & DAC_MODE_BITS;
        switch (dsm_pwm) {
        case DAC_MODE_1_A:
        case DAC_MODE_5_A:
            break;
        case DAC_MODE_1_B:
        case DAC_MODE_3_C:
        case DAC_MODE_3_D:
            /* default: */
            log_error("err dac Work mode!!!\n");
            con = (con & ~(DAC_MODE_BITS)) | DAC_MODE_1_A;
            /* while(1); */
            break;
        default:
            break;
        }
    }
    return con;
}


