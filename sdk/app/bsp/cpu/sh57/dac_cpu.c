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

#define apasr_con1(n10, n0, n8)   ((n10 << 10)| (n8 << 8) | (n0 << 0))

void dac_cpu_mode(void)
{
    apa_analog_open();
}

/* int dac_low_power(void) */
/* { */
/* return 0; */
/* } */

u32 dac_mode_check(u32 con)
{
    return con;
}
#define APA_HALF_PARA 1
/* #define APA_HALF_PARA 3 */

/************START apa_para_define**********************************************/
#if (APA_PLL240M_LOW == APA_CLK_DEFAULT)
/*---------------START apa_240M_low_para_define--------------------*/

#define  CON0_48K   (( 624 << 22) | (APA_HALF_PARA<<18) | (3<<16))
#define  CON0_44K1  (( 679 << 22) | (APA_HALF_PARA<<18) | (3<<16))
#define  CON0_32K   (( 624 << 22) | (2<<18) | (3<<16))

#if (APA_MODE1_NOR == APA_MODE_DEFAULT)
#define CON1_48K    apasr_con1(156, 69, 2)
#define CON1_44K1   apasr_con1(170, 75, 2)
#define CON1_32K    apasr_con1(156, 69, 2)
#elif (APA_MODE2_SNR == APA_MODE_DEFAULT)
#define CON1_48K    apasr_con1(312, 69, 3)
#define CON1_44K1   apasr_con1(340, 75, 3)
#define CON1_32K    apasr_con1(312, 69, 3)
#elif (APA_MODE3_THD == APA_MODE_DEFAULT)
#define CON1_48K    apasr_con1(156, 69, 2)
#define CON1_44K1   apasr_con1(170, 75, 2)
#define CON1_32K    apasr_con1(156, 69, 2)
#endif
/*---------------END apa_240M_low_para_define--------------------*/

#elif (APA_PLL240M_HIGH == APA_CLK_DEFAULT)
/*---------------START apa_240M_high_para_define--------------------*/

#define  CON0_48K   (( 312 << 22) | (3<<18) | (3<<16))
#define  CON0_44K1  (( 339 << 22) | (3<<18) | (3<<16))
#define  CON0_32K   (( 468 << 22) | (3<<18) | (3<<16))

#if (APA_MODE1_NOR == APA_MODE_DEFAULT)
#define CON1_48K    apasr_con1(78,  69, 1)
#define CON1_44K1   apasr_con1(85,  75, 1)
#define CON1_32K    apasr_con1(117 104, 1)
#elif (APA_MODE2_SNR == APA_MODE_DEFAULT)
#define CON1_48K    apasr_con1(156, 69, 2)
#define CON1_44K1   apasr_con1(170, 75, 2)
#define CON1_32K    apasr_con1(234,104, 2)
#elif (APA_MODE3_THD == APA_MODE_DEFAULT)
#define CON1_48K    apasr_con1(78,  69, 1)
#define CON1_44K1   apasr_con1(85,  75, 1)
#define CON1_32K    apasr_con1(117,104, 1)
#endif
/*---------------END apa_240M_high_para_define--------------------*/

#elif (APA_PLL320M == APA_CLK_DEFAULT)
/*---------------START apa_320M_para_define--------------------*/

#define  CON0_48K   (( 416 << 22) | (3<<18) | (3<<16))
#define  CON0_44K1  (( 453 << 22) | (3<<18) | (3<<16))
#define  CON0_32K   (( 624 << 22) | (3<<18) | (3<<16))

#if (APA_MODE1_NOR == APA_MODE_DEFAULT)
#define CON1_48K    apasr_con1(104,  92, 1)
#define CON1_44K1   apasr_con1(113, 100, 1)
#define CON1_32K    apasr_con1(156,  69, 2)
#elif (APA_MODE2_SNR == APA_MODE_DEFAULT)
#define CON1_48K    apasr_con1(208,  92, 2)
#define CON1_44K1   apasr_con1(227, 101, 2)
#define CON1_32K    apasr_con1(312,  69, 3)
#elif (APA_MODE3_THD == APA_MODE_DEFAULT)
#define CON1_48K    apasr_con1(104,  92, 1)
#define CON1_44K1   apasr_con1(113, 100, 1)
#define CON1_32K    apasr_con1(156,  69, 2)
#endif
/*---------------END apa_320M_para_define--------------------*/

#endif/* END #if (APA_PLL240M == APA_CLK_DEFAULT)*/

enum {
    DCC_238HZ4 = 8,
    DCC_119HZ4,
    DCC_59HZ7,
    DCC_29HZ7,
    DCC_14HZ8,
    DCC_7HZ5,
    DCC_3HZ6,
    DCC_1HZ8,
} ;
#define CON2_APA ( (0<<18) | (0 << 17) | (1 << 16) | (0 << 0) )

/************END apa_para_define**********************************************/
const u32 sr48k_para[3] = {
    CON0_48K,  CON1_48K | (DCC_3HZ6 << 24), CON2_APA
};

const u32 sr44k1_para[3] = {
    CON0_44K1, CON1_44K1 | (DCC_3HZ6 << 24), CON2_APA
};

const u32 sr32k_para[3] = {
    CON0_32K,  CON1_32K | (DCC_3HZ6 << 24), CON2_APA
};

const u32 *dac_get_para(u32 con)
{
    u32 sr_mode = con & APA_SRBITS;
    switch (sr_mode) {
    case APA_SR32K:
        return &sr32k_para[0];
    case APA_SR44K1:
        return &sr44k1_para[0];
    case APA_SR48K:
        return &sr48k_para[0];
    default:
        return &sr44k1_para[0];
    }
}

void test_audio_dac(void)
{
    log_info("JL_CLOCK->CLK_CON2 : 0x%x", JL_CLOCK->CLK_CON2);
    log_info("JL_APA->APA_CON0   : 0x%x", JL_APA->APA_CON0);
    log_info("JL_APA->APA_CON1   : 0x%x", JL_APA->APA_CON1);
    log_info("JL_APA->APA_CON2   : 0x%x", JL_APA->APA_CON2);
    log_info("JL_APA->APA_CON3   : 0x%x", JL_APA->APA_CON3);
    log_info("JL_APA->APA_CON4   : 0x%x", JL_APA->APA_CON4);
}

#define APA_NORMAL_OUT      0
#define APAP_OUT_APAN_MUTE  1
#define APAN_OUT_APAP_MUTE  2
#if (0 == (DAC_DEFAULT & APA_MODE2_SNR))
//AD17N mode1模式单端推功放功能配置
//0：APAP / APAN均输出信号，差分推功放
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

/* audio库调用，初始化APA时MUTE住防止po声 */
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
