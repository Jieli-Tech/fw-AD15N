#include "clock.h"
#include "adc_drv.h"
#include "jiffies.h"
#include "gpio.h"
#include "my_malloc.h"
#include "errno-base.h"

#include "log.h"
#define LOG_TAG_CONST     NORM
#define LOG_TAG		      "[SARADC]"

#define ADC_MAX_CH  				(10)  //cpu模式采集队列支持的最大通道数
#define DMA_BUF_CH_UNIT				(5)   //单个dma_buf内存放单个通道数据组数
#define GET_DMA_CUR_USE_BUF_FLAG() 	((JL_ADC->DMA_CON0&BIT(13))>>13) //0:BUF0 1:BUF1

#define ADC_PMU_CLK					(500000L)
#define ADC_CLK						(1000000L)
#define ADC_DMA_CONTI_CLK			(10000000L)

void (*adc_scan_over_cb)(void) = NULL;
//cpu模式adc采集队列信息结构体
struct adc_info_t {
    u32 ch;
    u16 adc_value;
    u16 voltage;
};
struct adc_info_t adc_queue[ADC_MAX_CH];

//dma模式adc采集队列信息结构体
struct adc_dma_info_t {
    u32 ch;
    u16 adc_value;
    u16 voltage;
};
struct adc_dma_info_t adc_dma_queue[16];//dma_chl[0~15]
/*************gpadc_info.flag****************/
#define B_SADC_INIT             BIT(0)
#define B_SADC_BUSY             BIT(1)
#define B_SADC_WAIT             BIT(2)

struct gpadc_info_t {
    u8 flag;			  //adc flag
    u8 cur_ch;			  //cpu_mode adc采集队列当前编号
    u8 dma_conti_mode;	  //dma 单通道采集模式标志
    u16 adc_vbg;		  //vbg ad_value
    u16 dma_ch_en;		  //dma_mode 16个通道的使能标志
    u16 dma_ch_num;		  //dma_mode 当前采集通道总数
    u16 *dma_badr_buf;	  //dma_mode dma_buf
    u16 *dma_conti_buf;   //dma_conti_mode dma_buf
    void (*dma_irq_callback)(u16 *buf, u32 len);
};
static struct gpadc_info_t gpadc_info = {
    .cur_ch = ADC_MAX_CH,
    .dma_badr_buf = NULL,
    .dma_conti_buf = NULL,
    .dma_irq_callback = NULL,
};
#define __this (&gpadc_info)

const u32 dma_ch_table[] = {
    ADC_CH_PA0,  0xffff, 				   0xffff, 			 		 ADC_CH_PA1, 	//chl0
    ADC_CH_PA8,  AD_DIFF_CH_SP_PA2_SN_PA3, 0xffff, 			 		 ADC_CH_PA9, 	//chl1
    ADC_CH_PA4,  AD_DIFF_CH_SP_PA2_SN_PA4, 0xffff, 			 		 ADC_CH_PA5, 	//chl2
    ADC_CH_PA12, AD_DIFF_CH_SP_PA2_SN_PA5, 0xffff, 			 		 ADC_CH_DP,  	//chl3
    ADC_CH_PA2,  AD_DIFF_CH_SP_PA3_SN_PA2, 0xffff, 					 ADC_CH_PA3, 	//chl4
    ADC_CH_PA10, 0xffff, 				   0xffff, 			 		 ADC_CH_PA11,	//chl5
    ADC_CH_PA6,  AD_DIFF_CH_SP_PA3_SN_PA4, 0xffff, 					 ADC_CH_PA7, 	//chl6
    ADC_CH_DM, 	 AD_DIFF_CH_SP_PA3_SN_PA5, 0xffff, 				     0xffff,		//chl7
    ADC_CH_PA1,  AD_DIFF_CH_SP_PA4_SN_PA2, 0xffff, 					 AD_DIFF_CH_VB, //chl8
    ADC_CH_PA9,  AD_DIFF_CH_SP_PA4_SN_PA3, AD_DIFF_CH_SP_PA2_SN_PA3, AD_DIFF_CH_VB, //chl9
    ADC_CH_PA5,  0xffff, 				   AD_DIFF_CH_SP_PA2_SN_PA4, AD_DIFF_CH_VB, //chl10
    ADC_CH_DP, 	 AD_DIFF_CH_SP_PA4_SN_PA5, AD_DIFF_CH_SP_PA2_SN_PA5, AD_DIFF_CH_VB, //chl11
    ADC_CH_PA3,  AD_DIFF_CH_SP_PA5_SN_PA2, AD_DIFF_CH_SP_PA3_SN_PA2, AD_DIFF_CH_VB, //chl12
    ADC_CH_PA11, AD_DIFF_CH_SP_PA5_SN_PA3, 0xffff, 					 AD_DIFF_CH_VB, //chl13
    ADC_CH_PA7,  AD_DIFF_CH_SP_PA5_SN_PA4, AD_DIFF_CH_SP_PA3_SN_PA4, AD_DIFF_CH_VB, //chl14
    0xffff, 	 0xffff, 				   AD_DIFF_CH_SP_PA3_SN_PA5, AD_DIFF_CH_VB, //chl15

};

/*
 * @brief 把ADC IO通道换成IO名,仅支持普通IO
 * @ch ADC通道（IO通道）
 * @return 返回IO名
 */
u8 adc_ch2port(u32 real_ch)
{
    if ((real_ch < ADC_CH_PA0) && (real_ch > ADC_CH_PA7)) {
        return IO_PORT_MAX;
    }
    u32 ch = real_ch & 0xf;
    const u8 io_adcch_map_table[] = {
        IO_PORTA_00,
        IO_PORTA_08,
        IO_PORTA_04,
        IO_PORTA_12,
        IO_PORTA_02,
        IO_PORTA_10,
        IO_PORTA_06,
        IO_PORT_DM,
        IO_PORTA_01,
        IO_PORTA_09,
        IO_PORTA_05,
        IO_PORT_DP,
        IO_PORTA_03,
        IO_PORTA_11,
        IO_PORTA_07,
    };
    return io_adcch_map_table[ch];
}

/*
 * @brief ADC资源占用配置
 * @retry 重试次数,和循环执行速度相关
 * @return 错误对应值
 */
static u32 adc_is_wait(void)
{
    local_irq_disable();
    if (__this->flag & B_SADC_WAIT) {
        local_irq_enable();
        return E_ADC_WAIT;
    }
    __this->flag |= B_SADC_WAIT;
    local_irq_enable();
    return 0;
}

static u32 adc_is_busy(u32 retry)
{
    while (1) {
        if (!(__this->flag & B_SADC_BUSY)) {
            break;
        }
        if (0 == (retry--)) {
            return E_ADC_BUF;
        }
    }
    return 0;
}

static u32 adc_idle_check(u32 retry)
{
    u32 err = adc_is_wait();
    if (0 != err) {
        /* putchar('w'); */
        return err;
    }
    err = adc_is_busy(retry);
    if (0 != err) {
        __this->flag &= ~B_SADC_WAIT;
        /* putchar('b'); */
        return err;
    }
    __this->flag |= B_SADC_BUSY;
    __this->flag &= ~B_SADC_WAIT;
    return 0;
}



/*
 * @brief 获取对应通道的ADC值
 * @ch ADC通道
 * @return 返回通道ADC值
 */
u16 adc_get_value(u32 ch)
{
    for (u8 i = 0; i < ADC_MAX_CH; i++) {
        if ((adc_queue[i].ch & (ADC_CH_MASK_TYPE_SEL | ADC_CH_MASK_CH_SEL)) == ch) {
            return adc_queue[i].adc_value;
        }
        if ((adc_dma_queue[i].ch & (ADC_CH_MASK_TYPE_SEL | ADC_CH_MASK_CH_SEL)) == ch) {
            return adc_dma_queue[i].adc_value;
        }
    }
    return 0;
}

/*
 * @brief 获取对应通道的电压
 * @ch ADC通道
 * @return 返回通道的电压/mV
 */
u32 adc_get_voltage(u32 ch)
{
    for (u8 i = 0; i < ADC_MAX_CH; i++) {
        if ((adc_queue[i].ch & (ADC_CH_MASK_TYPE_SEL | ADC_CH_MASK_CH_SEL)) == ch) {
            return adc_queue[i].voltage;
        }
        if ((adc_dma_queue[i].ch & (ADC_CH_MASK_TYPE_SEL | ADC_CH_MASK_CH_SEL)) == ch) {
            return adc_dma_queue[i].voltage;
        }
    }
    return 0;
}

/*
 * @brief 将ADC数据转换为实际电压
 * @adc_vbg MVBG通道ADC值
 * @adc_raw_value 转换通道的ADC值
 * @return 返回转换后的电压/mV
 */
u32 adc_value_to_voltage(u16 adc_vbg, u16 adc_raw_value)
{
    u32 vbg_volt = ADC_VBG_CENTER;
    u32 ch_volt;
    if (adc_vbg == 0) {
        adc_vbg = 1; //防止div0异常
    }
    ch_volt = adc_raw_value * vbg_volt / adc_vbg;
    return ch_volt;
}

/*
 * @brief CPU模式下，获取ADC采样数据
 * @return 返回ADC采样数据
 */
u16 adc_get_res(void)
{
    u16 adc_res = JL_ADC->RES;
    switch (ADC_BIT_SEL) {
    case ADC_SEL_6_BIT:
        adc_res >>= 6;
        break;
    case ADC_SEL_8_BIT:
        adc_res >>= 4;
        break;
    case ADC_SEL_10_BIT:
        adc_res >>= 2;
        break;
    case ADC_SEL_12_BIT:
        break;
    }
    return adc_res;
}

/*
 * @brief CPU模式下，获取采集队列中下一个通道的队列编号
 * @return 返回下一个采集通道编号
 */
u32 adc_get_next_ch(void)
{
    u8 i;
    if (__this->cur_ch == ADC_MAX_CH) {
        i = 0;
    } else {
        i = __this->cur_ch + 1;
    }
    for (; i < ADC_MAX_CH; i++) {
        if (adc_queue[i].ch) {
            if ((adc_queue[i].ch & (ADC_REUSE_IO | ADC_BUSY_IO)) == (ADC_REUSE_IO | ADC_BUSY_IO)) {
                continue;
            } else if ((adc_queue[i].ch & ADC_REUSE_IO) == ADC_REUSE_IO) { //通道复用但未被占用

            }
            return i;
        }
    }
    return ADC_MAX_CH;
}

/*
 * @brief CPU模式下，ADC切换采集通道
 * @ch ADC通道（ANA通道，IO通道和DIFF通道）
 */
static void adc_channel_select(u32 ch)
{
    u16 adc_ch_sel = ch & ADC_CH_MASK_CH_SEL;
    switch (ch & ADC_CH_MASK_TYPE_SEL) {
    case AD_HADC_TEST:
        SFR(JL_ADC->ADC_CON, 3, 3, 0b001);
        SFR(JL_ADC->ADC_CON, 10, 3, AD_HADC_TEST >> 16);
        break;
    case AD_AUDIO_TEST:
        SFR(JL_ADC->ADC_CON, 3, 3, 0b001);
        SFR(JL_ADC->ADC_CON, 10, 3, AD_AUDIO_TEST >> 16);
        break;
    case AD_PMU_TEST:
        SFR(JL_ADC->ADC_CON, 3, 3, 0b001);
        SFR(JL_ADC->ADC_CON, 10, 3, AD_PMU_TEST >> 16);
        ADC_CHANNEL_SEL(adc_ch_sel);
        PMU_TOADC_EN(1);
        PMU_DET_OE(1);
        break;
    case AD_X32K_TEST:
        SFR(JL_ADC->ADC_CON, 3, 3, 0b001);
        SFR(JL_ADC->ADC_CON, 10, 3, AD_X32K_TEST >> 16);
        break;
    case AD_PLL_TEST:
        SFR(JL_ADC->ADC_CON, 3, 3, 0b001);
        SFR(JL_ADC->ADC_CON, 10, 3, AD_PLL_TEST >> 16);
        break;
    case AD_IO_TEST:
        SFR(JL_ADC->ADC_CON, 3, 3, 0b010);
        SFR(JL_ADC->ADC_CON, 6, 4, adc_ch_sel);
        break;
    case AD_DIFF_TEST:
        SFR(JL_ADC->ADC_CON, 3, 3, 0b100);
        if (adc_ch_sel == ADC_DIFF_CH_VB) {
            SFR(JL_ADC->ANA_CON, 0, 2, 0); //sp sn disable
            SFR(JL_ADC->ANA_CON, 6, 2, 3); //vp vn enable
        } else {
            SFR(JL_ADC->ANA_CON, 6, 2, 0); //vp vn disable
            SFR(JL_ADC->ANA_CON, 0, 2, 3); //sp sn enable
            SFR(JL_ADC->ANA_CON, 2, 2, (ch & 0xf0) >> 4); //sp sel
            SFR(JL_ADC->ANA_CON, 4, 2, (ch & 0x0f)); //sn sel
        }
        break;
    }
}
static void adc_sample(u32 ch, bool isr_flag)
{
    SFR(JL_ADC->ADC_CON, 31, 1, 0);
    adc_clk_init(ADC_CLK, ch);
    /* SFR(JL_ADC->ADC_CON, 23, 2, ADC_BIT_SEL); */
    /* log_info("samp ch 0x%x\n", ch); */
    adc_channel_select(ch);
    if (isr_flag) {
        SFR(JL_ADC->ADC_CON, 13, 1, 1); //ie
    } else {
        SFR(JL_ADC->ADC_CON, 13, 1, 0); //ie
    }
    SFR(JL_ADC->ADC_CON, 31, 1, 1);
    SFR(JL_ADC->ADC_CON, 14, 1, 1); //clr pending
}

/*
 * @brief SARADC 中断处理函数
 */
___interrupt
static void adc_isr(void)
{
    /* log_info("cur_ch %d\n", __this->cur_ch); */
    u16 buf_len = __this->dma_ch_num * DMA_BUF_CH_UNIT;
    u32 dma_adc_value = 0;
    u8 dma_ch = 0;

    //dma mode
    if (JL_ADC->DMA_CON0 & BIT(15)) {
        SFR(JL_ADC->DMA_CON0, 14, 1, 1); //clr pending
        SFR(JL_ADC->DMA_CON0, 0, 1, 0); //ie
        if (__this->dma_conti_mode && __this->dma_irq_callback) {
            __this->dma_irq_callback(__this->dma_conti_buf + (!GET_DMA_CUR_USE_BUF_FLAG() * DMA_CONTI_BUF_LEN), DMA_CONTI_BUF_LEN);
        } else {
            for (u8 i = 0; i < 16; i++) {
                if ((__this->dma_ch_en & BIT(i))) {
                    for (u8 j = 0; j < buf_len; j++) {
                        if (j % __this->dma_ch_num == dma_ch) {
                            dma_adc_value += (__this->dma_badr_buf[(!GET_DMA_CUR_USE_BUF_FLAG()) * buf_len + j] >> 4);
                        }
                    }
                    adc_dma_queue[i].adc_value = dma_adc_value / DMA_BUF_CH_UNIT;
                    adc_dma_queue[i].voltage = adc_value_to_voltage(__this->adc_vbg, adc_dma_queue[i].adc_value);
                    dma_adc_value = 0;
                    dma_ch++;
                }
            }
        }
        SFR(JL_ADC->DMA_CON0, 0, 1, 1); //ie
    }

    //cpu mode
    if (JL_ADC->ADC_CON & BIT(15)) {
        PMU_TOADC_EN(0);
        PMU_DET_OE(0);
        SFR(JL_ADC->ADC_CON, 13, 1, 0); //ie
        if (__this->cur_ch < ADC_MAX_CH) {
            adc_queue[__this->cur_ch].adc_value = adc_get_res();
            adc_queue[__this->cur_ch].voltage = adc_value_to_voltage(__this->adc_vbg, adc_queue[__this->cur_ch].adc_value);
        }

        __this->cur_ch = adc_get_next_ch();
        /* log_info("next_ch %d\n", __this->cur_ch); */
        if (__this->cur_ch < ADC_MAX_CH) {
            adc_sample(adc_queue[__this->cur_ch].ch, 1);
        } else { //通道轮询结束
            __this->flag &= ~B_SADC_BUSY;
            if (adc_scan_over_cb) {
                adc_scan_over_cb();
            }
        }
    }
    asm("csync");
}

/*
 * @brief 添加ch到CPU模式ADC采集队列
 * @ch ADC通道（ANA通道，IO通道和DIFF通道）
 */
void adc_add_sample_ch(u32 ch)
{
    u8 i;
    for (i = 0; i < ADC_MAX_CH; i++) {
        if ((adc_queue[i].ch & (ADC_CH_MASK_TYPE_SEL | ADC_CH_MASK_CH_SEL)) == ch) {
            break;
        } else if (!adc_queue[i].ch) {
            switch (ch & ADC_CH_MASK_TYPE_SEL) {
            case AD_HADC_TEST:
            case AD_AUDIO_TEST:
            case AD_PMU_TEST:
            case AD_X32K_TEST:
            case AD_PLL_TEST:
            case AD_IO_TEST:
            case AD_DIFF_TEST:
                adc_queue[i].ch = ch;
                //en usb 2 saradc
                if (ch == ADC_CH_DP) {
                    SFR(JL_PORTUSB->CON, 1, 1, 1);
                } else if (ch == ADC_CH_DM) {
                    SFR(JL_PORTUSB->CON, 3, 1, 1);
                }
                break;
            default:
                log_error("adc add sample ch err!!!!\n");
                break;
            }
            break;
        }
    }
    if (i == (ADC_MAX_CH - 1) && (adc_queue[i].ch & (ADC_CH_MASK_TYPE_SEL | ADC_CH_MASK_CH_SEL)) != ch) {
        log_error("adc add err, queue full!!!!\n");
    }
}


/*
 * @brief 删除CPU模式ADC采集队列里的ch
 * @ch ADC通道（ANA通道，IO通道和DIFF通道）
 */
void adc_remove_sample_ch(u32 ch)
{
    for (u8 i = 0; i < ADC_MAX_CH; i++) {
        if ((adc_queue[i].ch & (ADC_CH_MASK_TYPE_SEL | ADC_CH_MASK_CH_SEL)) == ch) {
            adc_queue[i].ch = 0;
            if (ch == ADC_CH_DP) {
                SFR(JL_PORTUSB->CON, 1, 1, 0);
            } else if (ch == ADC_CH_DM) {
                SFR(JL_PORTUSB->CON, 3, 1, 0);
            }
            break;
        }
    }
}

u8 adc_add_ch_reuse(u32 ch, u8 busy)
{
    for (u8 i = 0; i < ADC_MAX_CH; i++) {
        if ((adc_queue[i].ch & (ADC_CH_MASK_TYPE_SEL | ADC_CH_MASK_CH_SEL)) == ch) {
            adc_queue[i].ch |= ADC_REUSE_IO;
            if (busy) {
                adc_queue[i].ch |= ADC_BUSY_IO;
                adc_queue[i].adc_value = 0xffff;
                adc_queue[i].voltage = 0xffff;
            } else {
                adc_queue[i].ch &= ~ADC_BUSY_IO;
            }
            return 0;
        }
    }
    return -1;
}

u8 adc_remove_ch_reuse(u32 ch)
{
    for (u8 i = 0; i < ADC_MAX_CH; i++) {
        if ((adc_queue[i].ch & (ADC_CH_MASK_TYPE_SEL | ADC_CH_MASK_CH_SEL)) == ch) {
            adc_queue[i].ch &= ~ADC_REUSE_IO;
            adc_queue[i].ch &= ~ADC_BUSY_IO;
        }
    }
    return -1;
}

/*
 * @brief CPU模式下，ADC定时通道扫描采集函数，使用AD按键和电源电压采集功能时，驱动中会调用
 */
int adc_kick_start(void (*adc_scan_over)(void))
{
    if (!(__this->flag & B_SADC_INIT)) {
        return -1;
    }
    u32 err = adc_idle_check(0);
    if (0 != err) {
        return err;
    }
    if (__this->cur_ch != ADC_MAX_CH) {
        /* log_info("sampling\n"); */
        return -1;
    }
    __this->cur_ch = adc_get_next_ch();
    if (__this->cur_ch == ADC_MAX_CH) {
        return -1;
    }
    adc_scan_over_cb = adc_scan_over;
    adc_sample(adc_queue[__this->cur_ch].ch, 1);
    return 0;
}


/*
 * @brief CPU模式下，主循环更新VBG AD值
 */
u16 adc_sample_vbg()
{
    return 0;
    if (!(__this->flag & B_SADC_INIT)) {
        return -1;
    }
    u32 err = adc_idle_check(100);
    if (0 != err) {
        log_error("ADC is busy\n");
        return err;
    }
    VBG_TEST_SEL(VBG_TEST_SEL_MBG08);
    VBG_BUFFER_EN(1);
    udelay(100);
    VBG_TEST_EN(1);
    PMU_TOADC_EN(1);
    PMU_DET_OE(1);

    adc_sample(AD_ANA_PMU_CH_VBG, 0);
    u8 sample_cnt = 8; //连续采集8次
    u32 adc_v = 0;
    while (sample_cnt--) {
        while (!(JL_ADC->ADC_CON & BIT(15)));
        adc_v += adc_get_res();
        SFR(JL_ADC->ADC_CON, 14, 1, 1); //clr pending
    }

    PMU_DET_OE(0);
    PMU_TOADC_EN(0);
    VBG_TEST_EN(0);
    udelay(100);
    VBG_BUFFER_EN(0);
    __this->adc_vbg = adc_v / 8;
    __this->flag &= ~B_SADC_BUSY;
    return __this->adc_vbg;
}

/*
 * @brief SARADC CLK INIT
 */
void adc_clk_init(int clk, u32 ch)
{
    u32 clk_limit = clk;
    u8 i;
    if ((ch & ADC_CH_MASK_TYPE_SEL) == AD_PMU_TEST) {
        i = 0x7f; //max div
        goto __adc_baud_set;
    }
    int lsb_clk;
    lsb_clk = clk_get("lsb");
    for (i = 1; i < 0x80; i++) {
        if ((lsb_clk / (i * 2)) <= clk_limit) {
            /* log_info("i %d\n", i); */
            break;
        }
    }
__adc_baud_set:
    SFR(JL_ADC->ADC_CON, 16, 7, i);
}

/*
 * @brief 校准SARADC模拟部分基准电压
 */
void adc_cal_test(void)
{
    for (int i = 0; i < 20; i++) {
        if (i < 10) {
            SFR(JL_ADC->ADC_CON, 25, 1, 0);
        } else {
            SFR(JL_ADC->ADC_CON, 25, 1, 1);
        }
        SFR(JL_ADC->ADC_CON, 16, 7, i % 4);
        SFR(JL_ADC->ADC_CON, 31, 1, 0);
        SFR(JL_ADC->ADC_CON, 31, 1, 1);
        JL_ADC->ADC_CON |= BIT(2);
        delay(100);
    }
}

void adc_uninit(void)
{
    JL_ADC->ADC_CON = 0;
    JL_ADC->ANA_CON = 0;
    JL_ADC->DMA_CON0 = 0;
    JL_ADC->DMA_CON1 = 0;
    JL_ADC->DMA_CON2 = 0;
    JL_ADC->DMA_BADR = 0;
    __this->flag &= ~B_SADC_INIT;
    return;
}

/*
 * @brief SARADC INIT
 */
void adc_init(void)
{
    if (__this->flag & B_SADC_INIT) {
        return;
    }
    JL_ADC->ADC_CON = 0x2;
    JL_ADC->ANA_CON = 0;
    JL_ADC->DMA_CON0 = 0;
    JL_ADC->DMA_CON1 = 0;
    JL_ADC->DMA_CON2 = 0;
    JL_ADC->DMA_BADR = 0;

    adc_cal_test();

    SFR(JL_ADC->ANA_CON, 8, 1, 1); //sense en
    SFR(JL_ADC->ANA_CON, 9, 2, 2);
    SFR(JL_ADC->ANA_CON, 11, 2, 0); //sense gain
    SFR(JL_ADC->ANA_CON, 13, 1, 1); //vcm en
    SFR(JL_ADC->ANA_CON, 14, 1, 1);
    SFR(JL_ADC->ANA_CON, 15, 2, 2);
    SFR(JL_ADC->ANA_CON, 17, 2, 2);
    SFR(JL_ADC->ANA_CON, 19, 2, 3);
    SFR(JL_ADC->ANA_CON, 21, 2, 3);
    SFR(JL_ADC->ADC_CON, 31, 1, 1);

    adc_clk_init(ADC_CLK, AD_ANA_PMU_CH_VBG);
    SFR(JL_ADC->ADC_CON, 23, 2, ADC_BIT_SEL);

    VBG_TEST_SEL(VBG_TEST_SEL_MBG08);
    VBG_BUFFER_EN(1);
    udelay(100);
    VBG_TEST_EN(1);
    PMU_TOADC_EN(1);
    PMU_DET_OE(1);

    adc_sample(AD_ANA_PMU_CH_VBG, 0);
    u8 sample_cnt = 8; //连续采集8次
    u32 adc_v = 0;
    while (sample_cnt--) {
        while (!(JL_ADC->ADC_CON & BIT(15)));
        adc_v += adc_get_res();
        SFR(JL_ADC->ADC_CON, 14, 1, 1); //clr pending
    }
    __this->adc_vbg = adc_v / 8;
    log_info("vbg_value 0x%x\n", __this->adc_vbg);
    PMU_DET_OE(0);
    PMU_TOADC_EN(0);
    VBG_TEST_EN(0);
    udelay(100);
    VBG_BUFFER_EN(0);
    SFR(JL_ADC->ADC_CON, 31, 1, 0);
    SFR(JL_ADC->ADC_CON, 13, 1, 0); //ie
    __this->cur_ch = ADC_MAX_CH;//等kick
    request_irq(IRQ_SARADC_IDX, IRQ_ADC_IP, adc_isr, 0);
    __this->flag |= B_SADC_INIT;
}

/*********************************DMA mode*********************************/
/*
 * @brief 把IO通道和DIFF通道转换成DMA通道序号
 * @ch ADC通道（IO通道和DIFF通道）
 * @return 返回DMA通道序号
 */
u8 cpu_2_dma_ch(u32 ch)
{
    for (u8 i = 0; i < ARRAY_SIZE(dma_ch_table); i++) {
        if (dma_ch_table[i] == ch) {
            if (!(__this->dma_ch_en & BIT(i / 4))) {
                return i;
            }
        }
    }
    return ARRAY_SIZE(dma_ch_table);
}
/*
 * @brief 添加ch到DMA模式ADC采集队列
 * @ch ADC通道（IO通道和DIFF通道）
 */
void adc_dma_add_sample_ch(u32 ch)
{
    u8 dma_ch_sel = cpu_2_dma_ch(ch);

    if (dma_ch_sel == ARRAY_SIZE(dma_ch_table)) {
        log_error("dma adc add err\n");
        return;
    } else {
        __this->dma_ch_en |= BIT(dma_ch_sel / 4);
        SFR(JL_ADC->DMA_CON2, dma_ch_sel / 4 * 2, 2, dma_ch_sel % 4);
    }

    //en usb 2 saradc
    if (ch == ADC_CH_DP) {
        SFR(JL_PORTUSB->CON, 1, 1, 1);
    } else if (ch == ADC_CH_DM) {
        SFR(JL_PORTUSB->CON, 3, 1, 1);
    }

    adc_dma_queue[dma_ch_sel / 4].ch = ch;
    adc_dma_config();
}
/*
 * @brief 删除DMA模式ADC采集队列里的ch
 * @ch ADC通道（IO通道和DIFF通道）
 */
void adc_dma_remove_sample_ch(u32 ch)
{
    for (u8 i = 0; i < 16; i++) {
        if (adc_dma_queue[i].ch == ch) {
            adc_dma_queue[i].ch = 0;
            if (ch == ADC_CH_DP) {
                SFR(JL_PORTUSB->CON, 1, 1, 0);
            } else if (ch == ADC_CH_DM) {
                SFR(JL_PORTUSB->CON, 3, 1, 0);
            }
            __this->dma_ch_en &= ~BIT(i);
            adc_dma_config();
            break;
        }
    }
}

/*
 * @brief ADC进入DMA单通道连续采集模式（该模式下CPU模式采样无法进行）
 * @ch ADC通道（仅支持IO通道和DIFF通道）gpadc_dma_irq_callback: DMA单通道中断回调函数
 * @note 注意需要与adc_dma_exit_single_ch_sample()成对使用
 */
void adc_dma_enter_single_ch_sample(u32 ch, void (*gpadc_dma_irq_callback)(u16 *buf, u32 len))
{
    u16 sp_rate;
    u8 dma_ch_sel = cpu_2_dma_ch(ch);

    if (dma_ch_sel == ARRAY_SIZE(dma_ch_table)) {
        log_error("dma adc add err\n");
        return;
    } else {
        SFR(JL_ADC->DMA_CON0, 0, 1, 0); //dma ie
        SFR(JL_ADC->DMA_CON2, dma_ch_sel / 4 * 2, 2, dma_ch_sel % 4);
    }

    //en usb 2 saradc
    if (ch == ADC_CH_DP) {
        SFR(JL_PORTUSB->CON, 1, 1, 1);
    } else if (ch == ADC_CH_DM) {
        SFR(JL_PORTUSB->CON, 3, 1, 1);
    }

    __this->dma_conti_mode = 1;
    __this->dma_irq_callback = gpadc_dma_irq_callback;
    SFR(JL_ADC->DMA_CON1, 0, 16, 0);

    if (__this->dma_conti_buf) {
        my_free(__this->dma_conti_buf);
        __this->dma_conti_buf = NULL;
    }
    __this->dma_conti_buf = (u16 *)my_malloc(DMA_CONTI_BUF_LEN * 2 * 2, MM_SARADC_BUF);
    if (__this->dma_conti_buf) {
        SFR(JL_ADC->DMA_CON0, 16, 16, DMA_CONTI_BUF_LEN);
        JL_ADC->DMA_BADR = (u32)__this->dma_conti_buf;
    } else {
        log_error("dma malloc err!!!\n");
        adc_dma_close();
        return;
    }
    adc_clk_init(ADC_DMA_CONTI_CLK, 0);
    sp_rate = (1 + 1) * (10 + (6 + ADC_BIT_SEL * 2) * 2) + (19 - 1);
    SFR(JL_ADC->DMA_CON1, 16, 16, sp_rate);

    SFR(JL_ADC->DMA_CON0, 1, 1, 1);
    SFR(JL_ADC->DMA_CON0, 12, 1, 1); //buf clr
    SFR(JL_ADC->DMA_CON0, 14, 1, 1); //clr pend
    SFR(JL_ADC->DMA_CON0, 0, 1, 1); //dma ie
    SFR(JL_ADC->DMA_CON1, 0, 16, BIT(dma_ch_sel / 4)); //kst
}

/*
 * @brief ADC退出DMA单通道连续采集模式（恢复DMA多通道模式采集和CPU模式采集）
 * @note 注意需要与adc_dma_enter_single_ch_sample()成对使用
 */
void adc_dma_exit_single_ch_sample(u32 ch)
{
    adc_clk_init(ADC_CLK, 0);
    SFR(JL_ADC->DMA_CON0, 0, 1, 0); //dma ie
    for (u8 i = 0; i < 16; i++) {
        if (adc_dma_queue[i].ch == ch) {
            adc_dma_queue[i].ch = 0;
            if (ch == ADC_CH_DP) {
                SFR(JL_PORTUSB->CON, 1, 1, 0);
            } else if (ch == ADC_CH_DM) {
                SFR(JL_PORTUSB->CON, 3, 1, 0);
            }
            break;
        }
    }
    __this->dma_conti_mode = 0;

    if (__this->dma_conti_buf) {
        my_free(__this->dma_conti_buf);
        __this->dma_conti_buf = NULL;
    }
    __this->dma_irq_callback = NULL;

    adc_dma_config();

    SFR(JL_ADC->DMA_CON0, 1, 1, 0);
    SFR(JL_ADC->DMA_CON0, 12, 1, 1); //buf clr
    SFR(JL_ADC->DMA_CON0, 14, 1, 1); //clr pend
    SFR(JL_ADC->DMA_CON0, 0, 1, 1); //dma ie
    SFR(JL_ADC->DMA_CON1, 0, 16, __this->dma_ch_en); //kst
}

/*
 * @brief 关闭DMA模式
 */
void adc_dma_close(void)
{
    __this->dma_ch_en = 0;
    JL_ADC->DMA_CON0 = 0;
    JL_ADC->DMA_CON1 = 0;
    JL_ADC->DMA_CON2 = 0;
    JL_ADC->DMA_BADR = 0;
}

/*
 * @brief 获取DMA模式下采集通道总数
 * @return 返回DMA采集总通道
 */
u8 get_adc_dma_sample_ch_num(void)
{
    u8 ch_num = 0;
    for (u8 i = 0; i < 16; i++) {
        if (__this->dma_ch_en & BIT(i)) {
            ch_num++;
        }
    }
    return ch_num;
}

/*
 * @brief DMA模式配置，在增减DMA采样通道之后都需重新配置
 */
void adc_dma_config(void)
{
    u16 sp_rate;
    SFR(JL_ADC->DMA_CON1, 0, 16, 0); //kst
    __this->dma_ch_num = get_adc_dma_sample_ch_num();

    if (!__this->dma_ch_num) {
        return;
    }

    //dma_buf init
    if (__this->dma_badr_buf) {
        my_free(__this->dma_badr_buf);
        __this->dma_badr_buf = NULL;
    }
    __this->dma_badr_buf = (u16 *)my_malloc(__this->dma_ch_num * DMA_BUF_CH_UNIT * 2 * 2, MM_SARADC_BUF);
    if (__this->dma_badr_buf) {
        SFR(JL_ADC->DMA_CON0, 16, 16, __this->dma_ch_num * DMA_BUF_CH_UNIT);
        JL_ADC->DMA_BADR = (u32)__this->dma_badr_buf;
    } else {
        log_error("dma malloc err!!!\n");
        adc_dma_close();
        return;
    }

    //sp_rate config
    sp_rate = (__this->dma_ch_num + 1) * (10 + (6 + ADC_BIT_SEL * 2) * 2) + (19 - __this->dma_ch_num);
    SFR(JL_ADC->DMA_CON1, 16, 16, sp_rate);

    SFR(JL_ADC->DMA_CON0, 2, 1, 0);  //chl log en
    SFR(JL_ADC->DMA_CON0, 26, 1, 0); //dma priority
    SFR(JL_ADC->DMA_CON0, 12, 1, 1); //buf clr
    SFR(JL_ADC->DMA_CON0, 14, 1, 1); //clr pend
    SFR(JL_ADC->DMA_CON0, 0, 1, 1); //dma ie
    SFR(JL_ADC->ADC_CON, 31, 1, 0);
    SFR(JL_ADC->ADC_CON, 31, 1, 1);
    SFR(JL_ADC->DMA_CON1, 0, 16, __this->dma_ch_en); //kst
}


