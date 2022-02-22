#include "app_power_mg.h"
#include "adc_drv.h"
#include "key.h"
#include "msg.h"
#include "asm/power/p33.h"
#include "efuse.h"
#include "clock.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"

#define LOW_POWER_VOL		3300//3.3v

static volatile u8 powerup_kick_start_cnt = 0;
static void app_power_powerup_kick_start(void)
{
    if (powerup_kick_start_cnt++ < 10) {
        adc_kick_start(app_power_powerup_kick_start);
        return;
    }
    log_info("vbat value:%d \n", app_power_get_vbat());
}

void app_power_init(void)
{
    adc_add_sample_ch(ADC_CH_PMU_VBAT);
    adc_kick_start(app_power_powerup_kick_start);
    while (powerup_kick_start_cnt < 10);
}

u32 app_power_get_vbat(void)
{
    return adc_get_voltage(ADC_CH_PMU_VBAT) * 4;
}

void app_power_scan(void)
{
    static u16 low_power_cnt = 0;
    u32 vol = adc_get_voltage(ADC_CH_PMU_VBAT) * 4;

#if (CPU_SH55==0)
    //AD14电压低于2v需切换到LRC时钟源
    if (vol <= 2000) {
        pll_ref_sel_init(PLL_REF_SEL_LRC);
    } else {
        pll_ref_sel_init(PLL_REF_SEL_HTC);
    }
#endif

    if (vol <= LOW_POWER_VOL) {
        low_power_cnt++;
        if (low_power_cnt == 10) {
            log_info("low power \n");
            post_msg(1, MSG_LOW_POWER);
        }
    } else {
        low_power_cnt = 0;
    }

#if (!(KEY_AD_EN || KEY_MATRIX_EN))
    adc_kick_start(NULL);
#endif
}
