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
        case DAC_MODE_3_C:
            break;
        case DAC_MODE_1_B:
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

#define APA_NORMAL_OUT      0
#define APAP_OUT_APAN_MUTE  1
#define APAN_OUT_APAP_MUTE  2
#if (DAC_CURR_MODE == DAC_MODE_3_C)
//AD15N DSM2模式单端推功放功能配置
//0：APAP / APAN均输出信号
//1：APAP输出信号，APAN作普通IO控制功放MUTE
//2：APAN输出信号，APAP作普通IO控制功放MUTE
#define SINGLE_APA_ENABLE       APA_NORMAL_OUT
//APA单端输出时，开机MUTE脚状态配置，需根据功放MUTE电平配置：
//0：MUTE脚开机输出低电平
//1：MUTE脚开机输出高电平
#define SINGLE_APA_MUTE_VALUE   1
#else
#define SINGLE_APA_ENABLE       APA_NORMAL_OUT
#define SINGLE_APA_MUTE_STATUS  0
#endif

/* audio库调用，初始化DSM2时MUTE住防止po声 */
void single_apa_startup_mute_cb(void)
{
#if (SINGLE_APA_ENABLE == APAP_OUT_APAN_MUTE)
    set_apan_output_status(SINGLE_APA_MUTE_VALUE);
#elif (SINGLE_APA_ENABLE == APAN_OUT_APAP_MUTE)
    set_apap_output_status(SINGLE_APA_MUTE_VALUE);
#endif
}

void single_apa_mute(u8 mute)
{
#if (SINGLE_APA_ENABLE == APAP_OUT_APAN_MUTE)
    if (mute) {
        set_apan_output_status(SINGLE_APA_MUTE_VALUE);
    } else {
        set_apan_output_status(!SINGLE_APA_MUTE_VALUE);
    }
#elif (SINGLE_APA_ENABLE == APAN_OUT_APAP_MUTE)
    if (mute) {
        set_apap_output_status(SINGLE_APA_MUTE_VALUE);
    } else {
        set_apap_output_status(!SINGLE_APA_MUTE_VALUE);
    }
#endif
}
