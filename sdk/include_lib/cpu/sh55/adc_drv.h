#ifndef __ADC_API_H__
#define __ADC_API_H__
#include "typedef.h"

//AD channel define
#define ADC_CH_PA0  		0X00
#define ADC_CH_PA1			0X01
#define ADC_CH_PA2			0X02
#define ADC_CH_PA3			0X03
#define ADC_CH_PA4			0X04
#define ADC_CH_PA5			0X05
#define ADC_CH_PA6			0X06
#define ADC_CH_PA7			0X07
#define ADC_CH_PA14			0X08
#define ADC_CH_PA15			0X09
#define ADC_CH_PB0			0X0a
#define ADC_CH_PB1			0X0b
#define ADC_CH_PB4			0X0c
#define ADC_CH_PB5			0X0d
#define ADC_CH_ANA_TEST		0X0e
#define ADC_CH_P33_TEST		0X0f

#define ADC_CH_PMU_VBG08	((0) + ADC_CH_P33_TEST)
#define ADC_CH_PMU_VDC12	((1) + ADC_CH_P33_TEST)
#define ADC_CH_PMU_DVDD		((2) + ADC_CH_P33_TEST)
#define ADC_CH_PMU_WVDD		((4) + ADC_CH_P33_TEST)
#define ADC_CH_PMU_VDDIO	((5) + ADC_CH_P33_TEST)
#define ADC_CH_PMU_VBAT		((6) + ADC_CH_P33_TEST)

#define ADC_CH_NONE			0Xff
#define ADC_VALUE_NONE 		0XFFFF

void adc_init();

void adc_sample(u8 ch);

u16 adc_get_value(u32 ch);

int adc_add_sample_ch(u32 ch);

int adc_remove_sample_ch(u32 ch);

u8 adc_add_ch_reuse(u32 ch, u8 busy);
u8 adc_remove_ch_reuse(u32 ch);

int adc_kick_start(void (*adc_scan_over)(void));

u8 adc_ch2port(u8 ch);

int adc_scan_once(u8 ch);

u32 adc_value_to_voltage(u32 adc_vbg, u32 adc_vbat);

u32 adc_get_voltage(u32 ch);
#define	adc_sample_vbg()  (-1)
#endif
