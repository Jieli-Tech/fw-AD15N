/***********************************Jieli tech************************************************
  File : audio_adc.c
  By   : liujie
  Email: liujie@zh-jieli.com
  date : 2020-9-2
********************************************************************************************/

#include "sfr.h"
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "clock.h"
#include "adc_drv.h"
#include "asm/power/p33.h"
#include "efuse.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"


#define ADC_MAX_CH_NUM		(ADC_CH_PMU_VBAT + 1)
#define ADC_MAX_CLK			1000000L

typedef struct _adc_info {
    u8 ch;
    u16 value;
} adc_info_t;

void (*adc_scan_over_cb)(void) = NULL;
static volatile u8 kick_start_flg = 0;
static adc_info_t adc_info[ADC_MAX_CH_NUM];
static u8 clk_div = 0;
static u8 cur_active_ch = 0;
static u8 init_flg = 0;
const u8 adc_clk_div[] = {
    1,			/*000*/
    6,			/*001*/
    12,			/*010*/
    24,			/*011*/
    48,			/*100*/
    72,			/*101*/
    96,			/*110*/
    128,		/*111*/
};

void adc_suspend()
{
    JL_GPADC->CON &= ~BIT(4);
}

void adc_resume()
{
    JL_GPADC->CON |= BIT(4);
}

void adc_close()
{
    JL_GPADC->CON = 0;
}

u8 adc_ch2port(u8 ch)
{
    if (ch > ADC_CH_PB0) {
        return IO_PORT_MAX;
    }

    const u8 io_adcch_map_table[] = {
        IO_PORTA_00,
        IO_PORTA_01,
        IO_PORTA_02,
        IO_PORTA_03,
        IO_PORT_DP,
        IO_PORT_DM,
        IO_PORTA_04,
        IO_PORTA_05,
        IO_PORTA_10,
        IO_PORTA_11,
        IO_PORTA_13,
        IO_PORTA_14,
        IO_PORTA_15,
        IO_PORTB_00,
    };

    return io_adcch_map_table[ch];
}

int adc_add_sample_ch(u32 ch)
{
    if (ch > (ADC_MAX_CH_NUM)) {
        return -1;
    }

    adc_info[ch].ch++;
    return 0;
}

int adc_remove_sample_ch(u32 ch)
{
    if (ch > (ADC_MAX_CH_NUM)) {
        return -1;
    }

    if (adc_info[ch].ch) {
        adc_info[ch].ch--;
        adc_info[ch].ch &= ~BIT(6);
        adc_info[ch].ch &= ~BIT(7);
    }

    if (adc_info[ch].ch == 0) {
        adc_info[ch].value = ADC_VALUE_NONE;
    }
    return 0;
}

u8 adc_add_ch_reuse(u32 ch, u8 busy)
{
    if (ch > (ADC_MAX_CH_NUM)) {
        return -1;
    }

    if (adc_info[ch].ch == 0) {
        adc_info[ch].value = ADC_VALUE_NONE;
        return -1;
    } else {
        adc_info[ch].ch |= BIT(7);
        if (busy) {
            adc_info[ch].ch |= BIT(6);
            adc_info[ch].value = ADC_VALUE_NONE;
        } else {
            adc_info[ch].ch &= ~BIT(6);
        }
    }
    return 0;
}
u8 adc_remove_ch_reuse(u32 ch)
{
    if (ch > (ADC_MAX_CH_NUM)) {
        return -1;
    }

    if (adc_info[ch].ch == 0) {
        adc_info[ch].value = ADC_VALUE_NONE;
        return -1;
    } else {
        adc_info[ch].ch &= ~BIT(7);
        adc_info[ch].ch &= ~BIT(6);
    }
    return 0;
}

u16 adc_get_value(u32 ch)
{
    if (!init_flg) {
        return ADC_VALUE_NONE;
    }
    if (ch > (ADC_MAX_CH_NUM)) {
        return ADC_VALUE_NONE;
    }

    if (adc_info[ch].ch) {
        return adc_info[ch].value;
    }

    return ADC_VALUE_NONE;
}

void adc_save_value(u32 ch, u16 value)
{
    if (ch > (ADC_MAX_CH_NUM)) {
        return;
    }

    adc_info[ch].value = value;
}

u8 adc_get_next_ch(u8 cur_ch)
{
    if ((adc_info[cur_active_ch].ch >> 6) == 0b10) {
        /* gpio_set_die(adc_ch2port(cur_active_ch), 1); */
    }
    for (int j = cur_ch + 1; j < ADC_MAX_CH_NUM; j++) {
        if (adc_info[j].ch != 0) {
            if ((adc_info[j].ch >> 6) == 0b11) {
                continue;
            } else if ((adc_info[j].ch >> 6) == 0b10) { //通道复用但未被占用
                /* gpio_set_die(adc_ch2port(j), 0); */
            }
            return j;
        }
    }
    return ADC_CH_NONE;
}

u8 adc_get_first_ch(void)
{
    int i = 0;
    for (; i < ADC_MAX_CH_NUM; i++) {
        if (adc_info[i].ch != 0) {
            if ((adc_info[i].ch >> 6) == 0b11) {
                continue;
            } else if ((adc_info[i].ch >> 6) == 0b10) {
                /* gpio_set_die(adc_ch2port(i), 0); */
            }
            return i;
        }
    }
    return ADC_CH_NONE;
}

static void adc_pmu_ch_select(u32 ch)
{
    P33_CON_SET(P3_ANA_CON4, 1, 3, ch);
}

__attribute__((weak))
void _adc_pmu_detect_en(u32 ch)
{
    P33_CON_SET(P3_ANA_CON4, 0, 1, 0);
    adc_pmu_ch_select(ch);
    P33_CON_SET(P3_ANA_CON4, 4, 3, 0);
    if (ch == (ADC_CH_PMU_VBG08 >> 4)) {
        P33_CON_SET(P3_ANA_CON4, 4, 1, 1);
        P33_CON_SET(P3_ANA_CON4, 5, 1, 1);
    }
    P33_CON_SET(P3_ANA_CON4, 0, 1, 1);
}

void adc_sample(u8 ch)
{
    u8 adc_ch = (ch >= ADC_CH_P33_TEST) ? (ADC_CH_P33_TEST) : ch;
    cur_active_ch = ch;
    u32 adc_con = 0;
    adc_con |= (clk_div/*时钟分频*/\
                | (0xf << 12)/*启动延时控制，实际启动延时为此数值*8个ADC时钟*/\
                | ((adc_ch & 0xf) << 8)/*ch选择*/\
                | BIT(3) | BIT(5)/*ie*/ | BIT(6));

    if (ch >= ADC_CH_P33_TEST) {
        _adc_pmu_detect_en(ch - ADC_CH_P33_TEST);
    }

    JL_GPADC->CON = adc_con;
    JL_GPADC->CON |= BIT(4);//en
    JL_GPADC->CON |= BIT(6);//clear pnd
}

int adc_scan_once(u8 ch)
{
    if (kick_start_flg) {
        return -1;
    }
    if ((adc_info[ch].ch >> 6) == 0b11) {
        return -1;    //ch reuse & busy
    }
    kick_start_flg = 1;
    u8 adc_ch = (ch >= ADC_CH_P33_TEST) ? (ADC_CH_P33_TEST) : ch;
    cur_active_ch = ch;

    u32 adc_con = 0;
    adc_con |= (clk_div/*时钟分频*/\
                | (0x0 << 12)/*启动延时控制，实际启动延时为此数值*8个ADC时钟*/\
                | ((adc_ch & 0xf) << 8)/*ch选择*/\
                | BIT(3) | BIT(6));

    if (ch >= ADC_CH_P33_TEST) {
        _adc_pmu_detect_en(ch - ADC_CH_P33_TEST);
    }

    JL_GPADC->CON = adc_con;
    JL_GPADC->CON |= BIT(4);//en
    JL_GPADC->CON |= BIT(6);//clear pnd
    while (!(JL_GPADC->CON & BIT(7)));
    u16 data = JL_GPADC->DAT;
    JL_GPADC->CON |= BIT(6);//clear pnd
    kick_start_flg = 0;
    return data;
}

int adc_kick_start(void (*adc_scan_over)(void))
{
    if (!init_flg) {
        return -1;
    }
    OS_ENTER_CRITICAL();
    if (kick_start_flg) {
        OS_EXIT_CRITICAL();
        return -1;
    }
    kick_start_flg = 1;

    u8 ch = adc_get_first_ch();
    if (ch == ADC_CH_NONE) {
        kick_start_flg = 0;
        OS_EXIT_CRITICAL();
        return -1;
    }

    adc_scan_over_cb = adc_scan_over;
    adc_sample(ch);
    OS_EXIT_CRITICAL();

    return 0;
}

SET(interrupt(""))
void adc_isr()
{
    JL_GPADC->CON |= BIT(6);//clear pnd
    adc_suspend();

    //save data
    adc_save_value(cur_active_ch, JL_GPADC->DAT);

    u8 ch = adc_get_next_ch(cur_active_ch);
    if (ch == ADC_CH_NONE) {
        //scan over
        kick_start_flg = 0;
        if (adc_scan_over_cb) {
            adc_scan_over_cb();
        }
        return;
    }

    adc_sample(ch);
}

void adc_init(void)
{
    u32 con = 0;
    int clk =  clk_get("lsb");

    //init value
    for (int j = 0; j < ADC_MAX_CH_NUM; j++) {
        adc_info[j].ch = 0;
        adc_info[j].value = ADC_VALUE_NONE;
    }

    JL_GPADC->CON = 0;

    for (int i = 0; i < sizeof(adc_clk_div) / sizeof(adc_clk_div[0]); i++) {
        if (clk / adc_clk_div[i] <= ADC_MAX_CLK) {
            clk_div = i;
            con |= clk_div;
            break;
        }
    }

    adc_add_sample_ch(ADC_CH_PMU_VBG08);

    HWI_Install(IRQ_ADC_IDX, (u32)adc_isr, IRQ_ADC_IP) ; //timer0_isr
    JL_GPADC->CON = con;
    init_flg = 1;
}

u32 adc_value_to_voltage(u32 adc_vbg, u32 adc_vbat)
{
    int vbg_center = 800;
    int trim = 0;
    extern const char ADC_VBG_TRIM_EN;
    if (ADC_VBG_TRIM_EN) {
        trim = get_vbg_trim();
    }
    if (trim & BIT(4)) {
        vbg_center -= (trim & 0x0f) * 3;
    } else {
        vbg_center += (trim & 0x0f) * 3;
    }
    u32 voltage = (vbg_center) * adc_vbat / adc_vbg;
    return voltage;
}

u32 adc_get_voltage(u32 ch)
{
    u32 adc_vbat = adc_get_value(ch);
    u32 adc_vbg = adc_get_value(ADC_CH_PMU_VBG08);
    if (adc_vbg == 0) {
        return 0;
    }
    return adc_value_to_voltage(adc_vbg, adc_vbat);
}

