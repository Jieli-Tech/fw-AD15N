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

u16 double_dac_buf[DAC_PACKET_SIZE * 2] AT(.DAC_BUFFER);

void auout_mode_init(void)
{
    memset(&double_dac_buf[0], 0, sizeof(double_dac_buf));
    u32 con = dac_mode_check(DAC_DEFAULT);
    dac_resource_init((u8 *)&double_dac_buf[0], sizeof(double_dac_buf), con, 0);
}

void auout_init(u32 sr, bool delay_flag)
{
    dac_phy_init(dac_sr_lookup(sr));
    /* delay_10ms(2); */
    /* delay(5000); */
    if (delay_flag) {
        udelay(1000);//约1.2ms
    }
    dac_cpu_mode();
}

void auout_sr_api(u32 sr)
{
    /* u32 dac_sr_set(u32 sr) */
    dac_sr_set(dac_sr_lookup(sr));
    /* dac_analog_init(); */
}

void auout_off_api(void)
{
    rdac_analog_close();
    apa_analog_close();
    dac_phy_off();
}

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


