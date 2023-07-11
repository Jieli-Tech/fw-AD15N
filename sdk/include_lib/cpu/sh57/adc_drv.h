#ifndef __SIMPLE_SARADC_H_
#define __SIMPLE_SARADC_H_

#include "typedef.h"
#include "app_config.h"
#define ADC_MUX_IO           (0x1 << 4)  //普通IO
#define ADC_MUX_AN           (0x1 << 5)  //内部模拟电源通道
#define ADC_BUSY_IO	         (0x1 << 6)  //繁忙通道
#define ADC_REUSE_IO         (0x1 << 7)  //复用通道

#define ADC_CH_PMU_VBG       (ADC_MUX_AN | 0x0)
#define ADC_CH_PMU_VDC12     (ADC_MUX_AN | 0x1)
#define ADC_CH_PMU_DVDD      (ADC_MUX_AN | 0x2)
#define ADC_CH_PMU_VTEMP     (ADC_MUX_AN | 0x3)
#define ADC_CH_PMU_WVDD      (ADC_MUX_AN | 0x4)
#define ADC_CH_PMU_IOVDD     (ADC_MUX_AN | 0x5)
#define ADC_CH_PMU_1_4VBAT   (ADC_MUX_AN | 0x6)

#define ADC_CH_PMU           (0x01)
#define ADC_CH_CLASSD        (0x02)
#define ADC_CH_PLL1          (0x05)

#define ADC_DEN(en)     	 SFR(JL_ADC->CON, 31, 1, en)
#define ADC_TEST_SEL(ch)     SFR(JL_ADC->CON, 23, 3, ch)
#define ADC_CHL(ch)     	 SFR(JL_ADC->CON, 19, 4, ch)
#define CPND(en)     	     SFR(JL_ADC->CON, 6, 1, en)
#define TEST_CHL_EN(en)      SFR(JL_ADC->CON, 3, 1, en)
#define IO_CHL_EN(en)      	 SFR(JL_ADC->CON, 2, 1, en)
#define ADC_AEN(en)     	 SFR(JL_ADC->CON, 1, 1, en)

//AD channel define
#define ADC_CH_PA0           (ADC_MUX_IO | 0x0)
#define ADC_CH_PA1           (ADC_MUX_IO | 0x1)
#define ADC_CH_PA2           (ADC_MUX_IO | 0x2)
#define ADC_CH_PA3           (ADC_MUX_IO | 0x3)
#define ADC_CH_PA4           (ADC_MUX_IO | 0x4)
#define ADC_CH_PA5           (ADC_MUX_IO | 0x5)
#define ADC_CH_PA6           (ADC_MUX_IO | 0x6)
#define ADC_CH_PA7           (ADC_MUX_IO | 0x7)
#define ADC_CH_PA8           (ADC_MUX_IO | 0x8)
#define ADC_CH_PA9           (ADC_MUX_IO | 0x9)
#define ADC_CH_PA10          (ADC_MUX_IO | 0xa)
#define ADC_CH_PA11          (ADC_MUX_IO | 0xb)
#define ADC_CH_PA12          (ADC_MUX_IO | 0xc)
#define ADC_CH_PD1           (ADC_MUX_IO | 0xd)
#define ADC_CH_PD3           (ADC_MUX_IO | 0xe)
#define ADC_CH_FSP           (ADC_MUX_IO | 0xf)

#define ADC_MAX_CH_NUM       (10)
#define ADC_VALUE_NONE		 0xffff
#define ADC_CH_NONE		 	 0xff

void adc_add_sample_ch(u16 real_ch);
void adc_remove_sample_ch(u16 real_ch);
u8 adc_ch2port(u16 real_ch);
void adc_scan(void);
u16 adc_get_value(u16 real_ch);
u16 adc_get_pmu_value(u16 real_ch);
u16 adc_get_io_value(u16 real_ch);
void adc_init(void);
u32 adc_value2voltage(u32 adc_vbg, u32 adc_ch_val);
u32 adc_get_voltage(u16 real_ch);
u32 adc_get_vbat_voltage(void);
u16 adc_get_first_available_ch();
u16 adc_sample_vbg();
int adc_kick_start(void (*adc_scan_over)(void));
u8 adc_add_ch_reuse(u16 real_ch, u8 busy);
u8 adc_remove_ch_reuse(u16 real_ch);

#define ADC_CH_PMU_VBAT			ADC_CH_PMU_1_4VBAT
#endif



