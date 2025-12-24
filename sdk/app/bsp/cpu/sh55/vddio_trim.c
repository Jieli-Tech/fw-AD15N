#include "typedef.h"
#include "adc_drv.h"
#include "asm/power/p33.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

void mdelay(u32 us);
void set_wvddio_lev(u8 lev);
void P33_CON_SET(u16 addr, u8 start, u8 len, u8 data);
void adc_sample(u8 ch);
u32 adc_value_to_voltage(u32 adc_vbg, u32 adc_vbat);


AT(.volatile_ram_code)
static u32 adc_vddio_voltage()
{
    u32 vddio_max = 0;
    u32 vddio_min = -1;
    u32 vddio_voltage = 0;
    u32 vddio_voltage_sum = 0;

    adc_sample(ADC_CH_PMU_VBG08);
    for (int i = 0; i < 16; i++) {
        while (!(JL_GPADC->CON & BIT(7)));
        u16 adc_vbg = JL_GPADC->DAT;
        JL_GPADC->CON |= BIT(6);//clear pnd

        u32 vddio_voltage  = adc_value_to_voltage(adc_vbg, 1023);
        if (vddio_voltage > vddio_max) {
            vddio_max = vddio_voltage;
        }
        if (vddio_voltage < vddio_min) {
            vddio_min = vddio_voltage;
        }
        vddio_voltage_sum += vddio_voltage;
    }

    vddio_voltage = (vddio_voltage_sum - vddio_max - vddio_min) / (16 - 2);

    return vddio_voltage;
}


AT(.volatile_ram_code)
int mvddio_trim(const u32 ms, u8 mvddio_lev)
{
    P33_CON_SET(P3_ANA_CON5, 3, 3, 0);

    u16 mvddio_vol[3];
    int lev_offsets[3] = {mvddio_lev - 1, mvddio_lev, mvddio_lev + 1};

    for (int i = 0; i < 3; i++) {
        P33_CON_SET(P3_ANA_CON5, 0, 3, lev_offsets[i]);
        mdelay(ms);
        mvddio_vol[i] = adc_vddio_voltage();
    }

    int target_mvddio = 2000 + 200 * mvddio_lev;
    u32 min_diff = -1;
    int best_lev = mvddio_lev;

    for (int i = 0; i < 3; i++) {
        u32 diff = __builtin_abs(mvddio_vol[i] - target_mvddio);
        if (diff < min_diff) {
            min_diff = diff;
            best_lev = lev_offsets[i];
        }
    }

    P33_CON_SET(P3_ANA_CON5, 0, 3, best_lev);

    return best_lev;
}

AT(.volatile_ram_code)
int wvddio_trim(const u32 ms, u8 wvddio_lev)
{
    P33_CON_SET(P3_ANA_CON5, 0, 3, 0);

    u16 wvddio_vol[3];
    int lev_offsets[3] = {wvddio_lev - 1, wvddio_lev, wvddio_lev + 1};

    for (int i = 0; i < 3; i++) {
        P33_CON_SET(P3_ANA_CON5, 3, 3, lev_offsets[i]);
        mdelay(ms);
        wvddio_vol[i] = adc_vddio_voltage();
    }

    int target_wvddio = 2000 + 200 * wvddio_lev;
    u32 min_diff = -1;
    int best_lev = wvddio_lev;

    for (int i = 0; i < 3; i++) {
        u32 diff = __builtin_abs(wvddio_vol[i] - target_wvddio);
        if (diff < min_diff) {
            min_diff = diff;
            best_lev = lev_offsets[i];
        }
    }

    P33_CON_SET(P3_ANA_CON5, 3, 3, best_lev);

    return best_lev;
}

AT(.volatile_ram_code)
void vddio_trim(u32 ms, u8 wvddio_lev, u8 mvddio_lev)
{
    local_irq_disable();
    const u8 p3_vlvd_con = p33_rx_1byte(P3_VLVD_CON);
    P33_CON_SET(P3_VLVD_CON, 3, 3, 0);
    wvddio_lev = wvddio_trim(ms, wvddio_lev);
    mvddio_lev =  mvddio_trim(ms, mvddio_lev);
    P33_CON_SET(P3_ANA_CON5, 0, 3, mvddio_lev);
    set_wvddio_lev(wvddio_lev);
    p33_tx_1byte(P3_VLVD_CON, p3_vlvd_con);
    local_irq_enable();
    log_info("wvddio_lev %x,mvddio_lev %x\n", wvddio_lev, mvddio_lev);
}

