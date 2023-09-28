#ifndef  __ADC_DRV_H__
#define  __ADC_DRV_H__

#include "typedef.h"
#include "asm/power_interface.h"

#define DMA_CONTI_BUF_LEN	(600) //DMA单通道模式采集x个点起一次中断

//ADC采样数据位宽
#define ADC_SEL_6_BIT   (0x0)
#define ADC_SEL_8_BIT   (0x1)
#define ADC_SEL_10_BIT  (0x2)
#define ADC_SEL_12_BIT  (0x3)
#define ADC_BIT_SEL		ADC_SEL_10_BIT //adc工作采样宽度

#if (ADC_BIT_SEL == ADC_SEL_12_BIT)
#define AD_MAX_VALUE	(0xfffL)
#elif (ADC_BIT_SEL == ADC_SEL_10_BIT)
#define AD_MAX_VALUE	(0x3ffL)
#elif (ADC_BIT_SEL == ADC_SEL_8_BIT)
#define AD_MAX_VALUE	(0xffL)
#elif (ADC_BIT_SEL == ADC_SEL_6_BIT)
#define AD_MAX_VALUE	(0x3fL)
#endif

#define	ADC_VBG_CENTER  (800) //VBG基准值

#define ADC_CH_MASK_TYPE_SEL	0xf0000
#define ADC_CH_MASK_CH_SEL	    0x0000ffff
#define ADC_REUSE_IO	        (0x1<<20)
#define ADC_BUSY_IO	            (0x2<<20)

#define ADC_IO_CH_PA0     (0x0) //ADC_IO_0
#define ADC_IO_CH_PA1     (0x8) //ADC_IO_8
#define ADC_IO_CH_PA2     (0x4) //ADC_IO_4
#define ADC_IO_CH_PA3     (0xc) //ADC_IO_12
#define ADC_IO_CH_PA4     (0x2) //ADC_IO_2
#define ADC_IO_CH_PA5     (0xa) //ADC_IO_10
#define ADC_IO_CH_PA6     (0x6) //ADC_IO_6
#define ADC_IO_CH_PA7     (0xe) //ADC_IO_14
#define ADC_IO_CH_PA8     (0x1) //ADC_IO_1
#define ADC_IO_CH_PA9     (0x9) //ADC_IO_9
#define ADC_IO_CH_PA10    (0x5) //ADC_IO_5
#define ADC_IO_CH_PA11    (0xd) //ADC_IO_13
#define ADC_IO_CH_PA12    (0x3) //ADC_IO_3
#define ADC_IO_CH_DP      (0xb) //ADC_IO_11
#define ADC_IO_CH_DM      (0x7) //ADC_IO_7

#define ADC_PMU_CH_VBG          (0b0000)
#define ADC_PMU_CH_VP17         (0b0001)
#define ADC_PMU_CH_VLCD_DIV_4   (0b0010)
#define ADC_PMU_CH_AVDDR_DIV_2  (0b0011)
#define ADC_PMU_CH_VTEMP        (0b0100)
#define ADC_PMU_CH_VPWR_DIV_4   (0b0101)
#define ADC_PMU_CH_AVDDCP_DIV_4 (0b0110)
#define ADC_PMU_CH_AVDDCP_DIV_2 (0b0111)
#define ADC_PMU_CH_AVDDCP_VB17  (0b1000)
#define ADC_PMU_CH_DCVDD        (0b1010)
#define ADC_PMU_CH_DVDD         (0b1011)
#define ADC_PMU_CH_WVDD         (0b1101)

#define ADC_DIFF_CH_SP_PA2_SN_PA3	(0x01) //ADC_DIFF_1
#define ADC_DIFF_CH_SP_PA2_SN_PA4	(0x02) //ADC_DIFF_2
#define ADC_DIFF_CH_SP_PA2_SN_PA5	(0x03) //ADC_DIFF_3
#define ADC_DIFF_CH_SP_PA3_SN_PA2	(0x10) //ADC_DIFF_4
#define ADC_DIFF_CH_SP_PA3_SN_PA4	(0x12) //ADC_DIFF_6
#define ADC_DIFF_CH_SP_PA3_SN_PA5	(0x13) //ADC_DIFF_7
#define ADC_DIFF_CH_SP_PA4_SN_PA2	(0x20) //ADC_DIFF_8
#define ADC_DIFF_CH_SP_PA4_SN_PA3	(0x21) //ADC_DIFF_9
#define ADC_DIFF_CH_SP_PA4_SN_PA5	(0x23) //ADC_DIFF_11
#define ADC_DIFF_CH_SP_PA5_SN_PA2	(0x30) //ADC_DIFF_12
#define ADC_DIFF_CH_SP_PA5_SN_PA3	(0x31) //ADC_DIFF_13
#define ADC_DIFF_CH_SP_PA5_SN_PA4	(0x32) //ADC_DIFF_14
#define ADC_DIFF_CH_VB				(0x44)

#define AD_HADC_TEST     (0x0<<16)
#define AD_AUDIO_TEST    (0x1<<16)
#define AD_PMU_TEST      (0x2<<16)
#define AD_X32K_TEST     (0x3<<16)
#define AD_PLL_TEST      (0x5<<16)
#define AD_IO_TEST       (0xe<<16)
#define AD_DIFF_TEST     (0xf<<16)



//ANA通道（模拟通道只支持CPU采集，不支持DMA采集）
#define AD_ANA_PMU_CH_VBG          (AD_PMU_TEST | ADC_PMU_CH_VBG)
#define AD_ANA_PMU_CH_VP17         (AD_PMU_TEST | ADC_PMU_CH_VP17)
#define AD_ANA_PMU_CH_VLCD_DIV_4   (AD_PMU_TEST | ADC_PMU_CH_VLCD_DIV_4)
#define AD_ANA_PMU_CH_AVDDR_DIV_2  (AD_PMU_TEST | ADC_PMU_CH_AVDDR_DIV_2)
#define AD_ANA_PMU_CH_VTEMP        (AD_PMU_TEST | ADC_PMU_CH_VTEMP)
#define AD_ANA_PMU_CH_VPWR_DIV_4   (AD_PMU_TEST | ADC_PMU_CH_VPWR_DIV_4)
#define AD_ANA_PMU_CH_AVDDCP_DIV_4 (AD_PMU_TEST | ADC_PMU_CH_AVDDCP_DIV_4)
#define AD_ANA_PMU_CH_AVDDCP_DIV_2 (AD_PMU_TEST | ADC_PMU_CH_AVDDCP_DIV_2)
#define AD_ANA_PMU_CH_AVDDCP_VB17  (AD_PMU_TEST | ADC_PMU_CH_AVDDCP_VB17)
#define AD_ANA_PMU_CH_DCVDD        (AD_PMU_TEST | ADC_PMU_CH_DCVDD)
#define AD_ANA_PMU_CH_DVDD         (AD_PMU_TEST | ADC_PMU_CH_DVDD)
#define AD_ANA_PMU_CH_WVDD         (AD_PMU_TEST | ADC_PMU_CH_WVDD)

//IO通道（支持CPU采集和DMA采集）
#define ADC_CH_PA0  (AD_IO_TEST | ADC_IO_CH_PA0) //0xe0000
#define ADC_CH_PA8  (AD_IO_TEST | ADC_IO_CH_PA8)
#define ADC_CH_PA4  (AD_IO_TEST | ADC_IO_CH_PA4)
#define ADC_CH_PA12 (AD_IO_TEST | ADC_IO_CH_PA12)
#define ADC_CH_PA2  (AD_IO_TEST | ADC_IO_CH_PA2)
#define ADC_CH_PA10 (AD_IO_TEST | ADC_IO_CH_PA10)
#define ADC_CH_PA6  (AD_IO_TEST | ADC_IO_CH_PA6)
#define ADC_CH_DM   (AD_IO_TEST | ADC_IO_CH_DM)
#define ADC_CH_PA1  (AD_IO_TEST | ADC_IO_CH_PA1)
#define ADC_CH_PA9  (AD_IO_TEST | ADC_IO_CH_PA9)
#define ADC_CH_PA5  (AD_IO_TEST | ADC_IO_CH_PA5)
#define ADC_CH_DP   (AD_IO_TEST | ADC_IO_CH_DP)
#define ADC_CH_PA3  (AD_IO_TEST | ADC_IO_CH_PA3)
#define ADC_CH_PA11 (AD_IO_TEST | ADC_IO_CH_PA11)
#define ADC_CH_PA7  (AD_IO_TEST | ADC_IO_CH_PA7)

//DIFF通道（支持CPU采集和DMA采集）最后采集得到的电压值为iovdd/2 - (N端电压 - P端电压)/2
#define AD_DIFF_CH_SP_PA2_SN_PA3  (AD_DIFF_TEST | ADC_DIFF_CH_SP_PA2_SN_PA3) //0xf0001
#define AD_DIFF_CH_SP_PA2_SN_PA4  (AD_DIFF_TEST | ADC_DIFF_CH_SP_PA2_SN_PA4)
#define AD_DIFF_CH_SP_PA2_SN_PA5  (AD_DIFF_TEST | ADC_DIFF_CH_SP_PA2_SN_PA5)
#define AD_DIFF_CH_SP_PA3_SN_PA2  (AD_DIFF_TEST | ADC_DIFF_CH_SP_PA3_SN_PA2)
#define AD_DIFF_CH_SP_PA3_SN_PA4  (AD_DIFF_TEST | ADC_DIFF_CH_SP_PA3_SN_PA4)
#define AD_DIFF_CH_SP_PA3_SN_PA5  (AD_DIFF_TEST | ADC_DIFF_CH_SP_PA3_SN_PA5)
#define AD_DIFF_CH_SP_PA4_SN_PA2  (AD_DIFF_TEST | ADC_DIFF_CH_SP_PA4_SN_PA2)
#define AD_DIFF_CH_SP_PA4_SN_PA3  (AD_DIFF_TEST | ADC_DIFF_CH_SP_PA4_SN_PA3)
#define AD_DIFF_CH_SP_PA4_SN_PA5  (AD_DIFF_TEST | ADC_DIFF_CH_SP_PA4_SN_PA5)
#define AD_DIFF_CH_SP_PA5_SN_PA2  (AD_DIFF_TEST | ADC_DIFF_CH_SP_PA5_SN_PA2)
#define AD_DIFF_CH_SP_PA5_SN_PA3  (AD_DIFF_TEST | ADC_DIFF_CH_SP_PA5_SN_PA3)
#define AD_DIFF_CH_SP_PA5_SN_PA4  (AD_DIFF_TEST | ADC_DIFF_CH_SP_PA5_SN_PA4)
#define AD_DIFF_CH_VB             (AD_DIFF_TEST | ADC_DIFF_CH_VB)

#define ADC_CH_PMU_VBAT  AD_ANA_PMU_CH_VPWR_DIV_4

/*
 * PMU相关宏定义
 */
enum {
    VBG_TEST_SEL_WBG04,
    VBG_TEST_SEL_MBG08,
    VBG_TEST_SEL_LVDBG,
    VBG_TEST_SEL_MVBG,
};
#define VBG_TEST_SEL(sel)			P33_CON_SET(P3_PMU_ADC0, 6, 2, sel)
#define VBG_TEST_EN(en)				p33_fast_access(P3_PMU_ADC0, BIT(5), en)
#define ADC_CHANNEL_SEL(ch)         P33_CON_SET(P3_PMU_ADC0, 0, 4, ch)
#define VBG_BUFFER_EN(en)			p33_fast_access(P3_PMU_ADC0, BIT(4), en)
#define PMU_TOADC_EN(en)			p33_fast_access(P3_PMU_ADC1, BIT(1), en)
#define PMU_DET_OE(en)				p33_fast_access(P3_PMU_ADC1, BIT(0), en)


/*
 * @brief SARADC INIT
 */
void adc_init(void);

/*
 * @brief SARADC CLK INIT
 */
void adc_clk_init(int clk, u32 ch);

/*
 * @brief 获取对应通道的ADC值
 * @ch ADC通道
 * @return 返回通道ADC值
 * @note 需确保通道有存在于CPU或DMA的采集队列,否则需先通过adc_add_sample_ch/adc_dma_add_sample_ch注册通道
 */
u16 adc_get_value(u32 ch);

/*
 * @brief 获取对应通道的电压
 * @ch ADC通道
 * @return 返回通道的电压/mV
 * @note 需确保通道有存在于CPU或DMA的采集队列,否则需先通过adc_add_sample_ch/adc_dma_add_sample_ch注册通道
 */
u32 adc_get_voltage(u32 ch);

/*
 * @brief 将ADC数据转换为实际电压
 * @adc_vbg MVBG通道ADC值
 * @adc_raw_value 转换通道的ADC值
 * @return 返回转换后的电压/mV
 */
u32 adc_value_to_voltage(u16 adc_vbg, u16 adc_raw_value);

/*
 * @brief 添加ch到CPU模式ADC采集队列
 * @ch ADC通道（ANA通道，IO通道和DIFF通道）
 */
void adc_add_sample_ch(u32 ch);

/*
 * @brief 删除CPU模式ADC采集队列里的ch
 * @ch ADC通道（ANA通道，IO通道和DIFF通道）
 */
void adc_remove_sample_ch(u32 ch);

void adc_cal_test(void);
u16 adc_sample_vbg();
void adc_scan();
int adc_kick_start(void (*adc_scan_over)(void));
u8 adc_ch2port(u32 real_ch);
u8 adc_add_ch_reuse(u32 ch, u8 busy);
u8 adc_remove_ch_reuse(u32 ch);
void adc_uninit(void);
/*
 * @brief 添加ch到DMA模式ADC采集队列
 * @ch ADC通道（仅支持IO通道和DIFF通道）
 */
void adc_dma_add_sample_ch(u32 ch);

/*
 * @brief 删除DMA模式ADC采集队列里的ch
 * @ch ADC通道（仅支持IO通道和DIFF通道）
 */
void adc_dma_remove_sample_ch(u32 ch);

/*
 * @brief ADC进入DMA单通道连续采集模式（该模式下CPU模式采样无法进行）
 * @ch ADC通道（仅支持IO通道和DIFF通道）gpadc_dma_irq_callback: DMA单通道中断回调函数
 * @note 注意需要与adc_dma_exit_single_ch_sample()成对使用
 * @note 回调函数16bits的buf，有效数据为bit16~4，低4位无效，读取时需>>4
 */
void adc_dma_enter_single_ch_sample(u32 ch, void (*gpadc_dma_irq_callback)(u16 *buf, u32 len));

/*
 * @brief ADC退出DMA单通道连续采集模式（恢复DMA多通道模式采集和CPU模式采集）
 * @note 注意需要与adc_dma_enter_single_ch_sample()成对使用
 */
void adc_dma_exit_single_ch_sample(u32 ch);

/*
 * @brief DMA模式配置，在增减DMA采样通道之后都需重新配置
 */
void adc_dma_config(void);
/*
 * @brief 关闭DMA模式
 */
void adc_dma_close(void);

#endif  /*GPADC_H*/


