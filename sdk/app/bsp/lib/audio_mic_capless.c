/*
 ****************************************************************************
 *							Audio Mic_Capless Module
 *省电容mic模块
 *即直接将mic的信号输出接到芯片的MICIN引脚(PA1/PB8)
 ****************************************************************************
 */

#include "audio.h"
#include "audio_adc.h"
#include "sfr.h"
#include "cpu.h"
#include "config.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[cles]"
#include "debug.h"

volatile s16 g_dtb;
extern const char MIC_CAPLESS_EN;
extern const char MIC_PGA_G;
bool ladc_capless_init(u32 delay)
{
    if (TRUE == MIC_CAPLESS_EN) {
        g_dtb = 13000;
        audio_adc_init_api(24000, ADC_MIC, 0);
        audio_adc_enable(MIC_PGA_G);
        delay_10ms(delay);
        audio_adc_off_api();
    }
    return MIC_CAPLESS_EN;
}

s16 mic_capless_dtb_read(void)
{
    return g_dtb;
}

AT(.audio_a.text.cache.L2)
u16 get_ladc_capless_bud(void)
{
    return 200;
}
AT(.audio_a.text.cache.L2)
bool ladc_capless_adjust_abandon()
{
    static u8 abandon = 0;
    if (abandon < 50) {
        abandon++;
        return true;
    }
    return false;
}

AT(.audio_a.text.cache.L2)
s16 ladc_capless_adjust_step(s16 curr_dtb, s16 dac32_dtb)
{
    static u8 cnt = 0;
    s16 stmp  = dac32_dtb - curr_dtb;
    if ((100 > stmp) && (stmp > -100)) {
        if (cnt < 255) {
            cnt++;
        }
    } else {
        if (cnt > 5) {
            cnt--;
        } else {
            cnt = 0;
        }
    }
    s16 stmp_dtb;
    if (cnt < 10) {
        stmp_dtb = dac32_dtb;
        /* log_info("[ + ] DAC_DTB %d %d\n", curr_dtb, stmp_dtb); */
    } else {
        if (stmp > 10) {
            stmp = 10;
        } else if (stmp < -10) {
            stmp = -10;
        }
        stmp_dtb = curr_dtb + stmp;
        /* log_info("[   ] DAC_DTB %d %d %d\n", curr_dtb, stmp_dtb, dac32_dtb); */
    }
    g_dtb = stmp_dtb;
    return stmp_dtb;
}


