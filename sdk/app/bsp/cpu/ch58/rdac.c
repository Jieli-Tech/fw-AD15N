/***********************************Jieli tech************************************************
  File : dac_api.c
  By   : liujie
  Email: liujie@zh-jieli.com
  date : 2019-1-14
********************************************************************************************/
#include "rdac.h"
#include "dac_api.h"
#include "dac.h"
#include "config.h"
#include "audio.h"
#include "audio_analog.h"
#include "asm/power_interface.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[dac cpu]"
#include "log.h"

const u32 audio_dac_con1 = AUDAC_FADE_SLOW(0) | AUDAC_FADE_STEP(1) | DAC_CON1_DEFAULT;

static u16 audio_dac_buf[DAC_PACKET_SIZE * 2];

#define DAC_MAX_SP  (sizeof(audio_dac_buf) / (DAC_TRACK_NUMBER * (AUDAC_BIT_WIDE / 8)))
#define DAC_PNS     (DAC_MAX_SP / 3)

RDAC_CTRL_HDL audio_dac_ops = {
    .buf         = audio_dac_buf,
    .sp_total    = DAC_MAX_SP,
    .sp_max_free = (DAC_MAX_SP - DAC_PNS) / 2,
    .sp_size     = DAC_TRACK_NUMBER * (AUDAC_BIT_WIDE / 8),
    .con         = DAC_CON0_DEFAULT,
    .pns         = DAC_PNS,
};

void rdac_mode_init(void)
{
    memset(&audio_dac_buf[0], 0, sizeof(audio_dac_buf));
    rdac_resource_init((void *)&audio_dac_ops);
}

void rdac_init(u32 sr, bool delay_flag)
{
    rdac_analog_open();
    delay(1000);
    rdac_phy_init(rdac_sr_lookup(sr));
    rdac_trim_api();
}

void rdac_sr_api(u32 sr)
{
    rdac_sr_set(rdac_sr_lookup(sr));
}

void rdac_off_api(void)
{
    rdac_phy_off();
}

void rdac_trim_api(void)
{
    u32 dac_trim_val = 0;
    JL_AUDIO->DAC_TM0 = dac_trim_val;
    delay(2000);
}

void audio_config_hexdump(void)
{
    log_info("AUDIO CLOCK");
    log_info("JL_ASS->CLK_CON   : 0x%x", JL_ASS->CLK_CON);
    log_info("JL_CLOCK->PRP_CON0: 0x%x", JL_CLOCK->PRP_CON0);
    log_info("JL_CLOCK->PRP_TCON: 0x%x\n", JL_CLOCK->PRP_TCON);

    log_info("AUDIO POWER");
    log_info("JL_ADDA->AVDD_CON0: 0x%x", JL_ADDA->AVDD_CON0);
    log_info("JL_ADDA->AVDD_CON1: 0x%x", JL_ADDA->AVDD_CON1);
    log_info("P3_CHG_PUMP       : 0x%x",   P33_CON_GET(P3_CHG_PUMP));
    log_info("P3_ANA_FLOW1      : 0x%x\n", P33_CON_GET(P3_ANA_FLOW1));

    log_info("AUDIO DIGITAL");
    log_info("JL_AUDIO->DAC_CON : 0x%x", JL_AUDIO->DAC_CON);
    log_info("JL_AUDIO->DAC_CON1: 0x%x", JL_AUDIO->DAC_CON1);
    log_info("JL_AUDIO->DAC_CON2: 0x%x", JL_AUDIO->DAC_CON2);
    log_info("JL_AUDIO->DAC_CON3: 0x%x", JL_AUDIO->DAC_CON3);
    log_info("JL_AUDIO->DAC_CON4: 0x%x", JL_AUDIO->DAC_CON4);
    log_info("JL_AUDIO->AUD_CON : 0x%x", JL_AUDIO->AUD_CON);
    log_info("JL_AUDIO->DAC_VL0 : 0x%x", JL_AUDIO->DAC_VL0);
    log_info("JL_AUDIO->DAC_DIT : 0x%x", JL_AUDIO->DAC_DIT);
    log_info("JL_AUDIO->DAC_TM0 : 0x%x", JL_AUDIO->DAC_TM0);
    log_info("JL_AUDIO->DAC_COP : 0x%x\n", JL_AUDIO->DAC_COP);

    log_info("AUDIO ANALOG");
    log_info("JL_ADDA->DAA_CON0 : 0x%x", JL_ADDA->DAA_CON0);
    log_info("JL_ADDA->DAA_CON7 : 0x%x", JL_ADDA->DAA_CON7);
    log_info("JL_ADDA->HADA_CON0: 0x%x", JL_ADDA->HADA_CON0);
    log_info("JL_ADDA->HADA_CON1: 0x%x", JL_ADDA->HADA_CON1);
    log_info("JL_ADDA->HADA_CON2: 0x%x", JL_ADDA->HADA_CON2);
    log_info("JL_ADDA->ADDA_CON0: 0x%x", JL_ADDA->ADDA_CON0);
    log_info("JL_ADDA->AVDD_CON0: 0x%x", JL_ADDA->AVDD_CON0);
    log_info("JL_ADDA->AVDD_CON1: 0x%x\n", JL_ADDA->AVDD_CON1);
}
