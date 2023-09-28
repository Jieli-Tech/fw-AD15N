#include "mcpwm.h"
#include "clock.h"
#include "gpio.h"
#include "app_modules.h"

#define LOG_TAG_CONST       MAIN
#define LOG_TAG             "[main]"
#include "log.h"

#define MCPWM_CLK   clk_get("mcpwm")


PWM_TIMER_REG *get_pwm_timer_reg(pwm_ch_num_type index)
{
    PWM_TIMER_REG *reg = NULL;
    switch (index) {
    case pwm_ch0:
        reg = (PWM_TIMER_REG *)(&(JL_MCPWM->TMR0_CON));
        break;
    case pwm_ch1:
        reg = (PWM_TIMER_REG *)(&(JL_MCPWM->TMR1_CON));
        break;
    default:
        break;
    }
    return reg;
}

PWM_CH_REG *get_pwm_ch_reg(pwm_ch_num_type index)
{
    PWM_CH_REG *reg = NULL;
    switch (index) {
    case pwm_ch0:
        reg = (PWM_CH_REG *)(&(JL_MCPWM->CH0_CON0));
        break;
    case pwm_ch1:
        reg = (PWM_CH_REG *)(&(JL_MCPWM->CH1_CON0));
        break;
    default:
        break;
    }
    return reg;
}

/*
 * @brief 更改MCPWM的频率
 * @parm frequency 频率
 */
void mcpwm_set_frequency(pwm_ch_num_type ch, pwm_aligned_mode_type align, u32 frequency)
{
    PWM_TIMER_REG *reg = get_pwm_timer_reg(ch);
    if (reg == NULL) {
        return;
    }

    reg->tmr_con = 0;
    reg->tmr_cnt = 0;
    reg->tmr_pr = 0;

    u32 i = 0;
    u32 mcpwm_div_clk = 0;
    u32 mcpwm_tmr_pr = 0;
    u32 mcpwm_fre_min = 0;
    u32 clk = MCPWM_CLK;
    for (i = 0; i < 16; i++) {
        mcpwm_fre_min = clk / (65536 * (1 << i));
        if ((frequency >= mcpwm_fre_min) || (i == 15)) {
            break;
        }
    }
    /* i = 15; */
    reg->tmr_con |= (i << 3); //div 2^i
    mcpwm_div_clk = clk / (1 << i);
    if (frequency == 0) {
        mcpwm_tmr_pr = 0;
    } else {
        if (align == pwm_center_aligned) { //中心对齐
            mcpwm_tmr_pr = mcpwm_div_clk / (frequency * 2) - 1;
        } else {
            mcpwm_tmr_pr = mcpwm_div_clk / frequency - 1;
        }
    }
    reg->tmr_pr = mcpwm_tmr_pr;
    //timer mode
    if (align == pwm_center_aligned) { //中心对齐
        reg->tmr_con |= 0b10;
    } else {
        reg->tmr_con |= 0b01;
    }
}

/*
 * @brief 设置两个引脚的占空比
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1
 * @parm duty 同时设置H和L引脚的占空比：0 ~ 10000 对应 0% ~ 100%
 */
void mcpwm_set_duty(pwm_ch_num_type pwm_ch, u16 duty)
{
    PWM_TIMER_REG *timer_reg = get_pwm_timer_reg(pwm_ch);
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);
    if (pwm_reg && timer_reg) {
        pwm_reg->ch_cmph = (timer_reg->tmr_pr + 1) * duty / 10000;
        pwm_reg->ch_cmpl = pwm_reg->ch_cmph;
    }
}
/*
 * @brief 设置H引脚的占空比
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1
 * @parm h_duty H引脚的占空比：0 ~ 10000 对应 0% ~ 100%，如果没有使能引脚，则设置的占空比无效
 */
void mcpwm_set_h_duty(pwm_ch_num_type pwm_ch, u16 h_duty)
{
    PWM_TIMER_REG *timer_reg = get_pwm_timer_reg(pwm_ch);
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);
    if (pwm_reg && timer_reg) {
        pwm_reg->ch_cmph = (timer_reg->tmr_pr + 1) * h_duty / 10000;
    }
}
/*
 * @brief 设置L引脚的占空比
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1
 * @parm l_duty L引脚的占空比：0 ~ 10000 对应 0% ~ 100%，如果没有使能引脚，则设置的占空比无效
 */
void mcpwm_set_l_duty(pwm_ch_num_type pwm_ch, u16 l_duty)
{
    PWM_TIMER_REG *timer_reg = get_pwm_timer_reg(pwm_ch);
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);
    if (pwm_reg && timer_reg) {
        pwm_reg->ch_cmpl = (timer_reg->tmr_pr + 1) * l_duty / 10000;
    }
}

/*
 * @brief 打开或者关闭一个时基
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 * @parm enable 1：打开  0：关闭
 */
void mctimer_ch_open_or_close(pwm_ch_num_type pwm_ch, u8 enable)
{
    if (pwm_ch > pwm_ch_max) {
        return;
    }
    if (enable) {
        JL_MCPWM->MCPWM_CON0 |=  BIT(pwm_ch + 8); //TnEN
    } else {
        JL_MCPWM->MCPWM_CON0 &= ~BIT(pwm_ch + 8); //TnDIS
    }
}


/*
 * @brief 打开或者关闭一个通道
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 * @parm enable 1：打开  0：关闭
 */
void mcpwm_ch_open_or_close(pwm_ch_num_type pwm_ch, u8 enable)
{
    if (pwm_ch >= pwm_ch_max) {
        return;
    }
    if (enable) {
        JL_MCPWM->MCPWM_CON0 |=  BIT(pwm_ch); //PWMnEN
    } else {
        JL_MCPWM->MCPWM_CON0 &= ~BIT(pwm_ch); //PWMnDIS
    }
}

/*
 * @brief 关闭MCPWM模块
 */
void mcpwm_open(pwm_ch_num_type pwm_ch)
{
    if (pwm_ch >= pwm_ch_max) {
        return;
    }
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);
    pwm_reg->ch_con1 &= ~(0b111 << 8);
    pwm_reg->ch_con1 |= (pwm_ch << 8); //sel mctmr
    mcpwm_ch_open_or_close(pwm_ch, 1);
    mctimer_ch_open_or_close(pwm_ch, 1);
}


/*
 * @brief 关闭MCPWM模块
 */
void mcpwm_close(pwm_ch_num_type pwm_ch)
{
    mctimer_ch_open_or_close(pwm_ch, 0);
    mcpwm_ch_open_or_close(pwm_ch, 0);
}


void log_pwm_info(pwm_ch_num_type pwm_ch)
{
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);
    PWM_TIMER_REG *timer_reg = get_pwm_timer_reg(pwm_ch);
    log_info("tmr%d con0 = 0x%x", pwm_ch, timer_reg->tmr_con);
    log_info("tmr%d cnt = %d", pwm_ch, timer_reg->tmr_cnt);
    log_info("tmr%d pr = %d", pwm_ch, timer_reg->tmr_pr);
    log_info("pwm ch%d_con0 = 0x%x", pwm_ch, pwm_reg->ch_con0);
    log_info("pwm ch%d_con1 = 0x%x", pwm_ch, pwm_reg->ch_con1);
    log_info("pwm ch%d_cmph = %d, pwm ch%d_cmpl = %d", pwm_ch, pwm_reg->ch_cmph, pwm_ch, pwm_reg->ch_cmpl);
    log_info("MCPWM_CON0 = 0x%x", JL_MCPWM->MCPWM_CON0);
    log_info("mcpwm clk = %d", MCPWM_CLK);
}

void mcpwm_init(struct pwm_platform_data *arg)
{
    //set output IO
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(arg->pwm_ch_num);
    if (pwm_reg == NULL) {
        return;
    }
    //set mctimer frequency
    mcpwm_set_frequency(arg->pwm_ch_num, arg->pwm_aligned_mode, arg->frequency);

    pwm_reg->ch_con0 = 1;

    if (arg->h_inv_en) {            //每个周期，是否先输出低电平
        pwm_reg->ch_con0 |=  BIT(4);
    } else {
        pwm_reg->ch_con0 &= ~BIT(4);
    }
    if (arg->l_inv_en) {            //每个周期，是否先输出低电平
        pwm_reg->ch_con0 |=  BIT(5);
    } else {
        pwm_reg->ch_con0 &= ~BIT(5);
    }

    /* mcpwm_open(arg->pwm_ch_num); 	 //mcpwm enable */
    //set duty
    mcpwm_set_h_duty(arg->pwm_ch_num, arg->h_duty);
    mcpwm_set_l_duty(arg->pwm_ch_num, arg->l_duty);
    //H:
    if (arg->h_pin < IO_MAX_NUM) {
        gpio_mux_out(arg->h_pin, GPIO_OUT_COMP0_MC_PWM0_H + 0x202 * arg->pwm_ch_num);
    }
    //L:
    if (arg->l_pin < IO_MAX_NUM) {
        gpio_mux_out(arg->l_pin, GPIO_OUT_COMP1_MC_PWM0_L + 0x202 * arg->pwm_ch_num);
    }
    mcpwm_open(arg->pwm_ch_num); 	 //mcpwm enable

    log_pwm_info(arg->pwm_ch_num);
}


///////////// for test code //////////////////
void mcpwm_test(void)
{
#define PWM_CH0_ENABLE 		1
#define PWM_CH1_ENABLE 		1

    struct pwm_platform_data pwm_p_data;

#if PWM_CH0_ENABLE
    memset((u8 *)&pwm_p_data, 0, sizeof(struct pwm_platform_data));
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned;         //边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch0;                        //通道号
    pwm_p_data.frequency = 10000;                           //10KHz
    pwm_p_data.h_pin = -1;                                  //任意IO，不需要则填-1
    pwm_p_data.h_duty = 5000;                               //占空比，选的引脚有值才有效
    pwm_p_data.l_pin = IO_PORTC_08;                         //任意IO，不需要就填-1
    pwm_p_data.l_duty = 5000;                               //占空比，选的引脚有值才有效
    mcpwm_init(&pwm_p_data);
#endif

#if PWM_CH1_ENABLE
    memset((u8 *)&pwm_p_data, 0, sizeof(struct pwm_platform_data));
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned;         //边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch1;                        //通道号
    pwm_p_data.frequency = 1000;                            //1KHz
    pwm_p_data.h_pin = IO_PORTC_09;                         //任意IO，不需要则填-1
    pwm_p_data.h_duty = 5000;                               //占空比，选的引脚有值才有效
    pwm_p_data.l_pin = IO_PORTC_10;                         //任意IO，不需要就填-1
    pwm_p_data.l_duty = 7000;                               //占空比，选的引脚有值才有效
    pwm_p_data.l_inv_en = 1;                                //l_pin的每个周期先输出低电平，即占空比体现在低电平
    mcpwm_init(&pwm_p_data);
#endif
    extern void wdt_clear();
    while (1) {
        wdt_clear();
    }
}

#if defined(HAS_MIO_EN) && (HAS_MIO_EN)

struct PWM_MIO_DATA {
    pwm_aligned_mode_type pwm_aligned_mode;           ///< PWM对齐方式选择
    u32 frequency;                               	  ///< 初始化该通道的频率,即该通道的两个引脚H&L都是同一个频率。
    u16 duty;                                         ///< H引脚的初始占空比，0~10000 对应 0%~100% 。每个引脚可以有不同的占空比。
    u8 ch;	   				                          ///< 逻辑通道号，0~1选择ch0,2~3选ch1
    u8 pin;                                           ///< 引脚，不需要则填-1
    u8 inv_en;                                        ///< 引脚输出的波形，每个周期内，0: 先输出高电平， 1: 先输出低电平，此时占空比为低电平的占空比
};

struct PWM_MIO_DATA pwm_mio_data[4]; //硬件上只有两组一共4路PWM,使用时0和1、2和3，也就是同一组的PWM通道频率需要一致

void mio_a_pwm_cpu_init(u32 chl, u32 gpio, u32 frequency)
{
    if (chl > 3) {
        return;
    }
    memset((u8 *)&pwm_mio_data[chl], 0, sizeof(struct PWM_MIO_DATA));
    pwm_mio_data[chl].pwm_aligned_mode = pwm_edge_aligned;         //边沿对齐
    pwm_mio_data[chl].ch = chl;                                    //通道号
    pwm_mio_data[chl].frequency = frequency;                       //10KHz
    pwm_mio_data[chl].pin = gpio;                                  //任意IO
    pwm_mio_data[chl].duty = 7500;                                 //占空比，选的引脚有值才有效
    pwm_mio_data[chl].inv_en = 0;

    u32 using_chn; //PWM硬件通道
    PWM_CH_REG *pwm_reg;
    //set output IO
    if ((0 == chl) || (1 == chl)) {
        using_chn = 0;
        pwm_reg = get_pwm_ch_reg(using_chn);
    } else {
        using_chn = 1;
        pwm_reg = get_pwm_ch_reg(using_chn);
    }
    if (pwm_reg == NULL) {
        return;
    }
    //set mctimer frequency
    mcpwm_set_frequency(using_chn, pwm_mio_data[chl].pwm_aligned_mode, pwm_mio_data[chl].frequency);

    pwm_reg->ch_con0 = 1;
    log_info("using_chn %d\n", using_chn);
    if ((0 == chl) || (2 == chl)) {     		   //配置h口
        if (pwm_mio_data[chl].inv_en) { 		   //h口每个周期，是否先输出低电平
            pwm_reg->ch_con0 |=  BIT(4);
        } else {
            pwm_reg->ch_con0 &= ~BIT(4);
        }
        mcpwm_set_h_duty(using_chn, pwm_mio_data[chl].duty);
        //H:
        if (pwm_mio_data[chl].pin < IO_MAX_NUM) {
            /* 出现类似comp_out[0] busy通道繁忙的断言可以先调用gpio_mux_out_close */
            /* gpio_mux_out_close(pwm_mio_data[chl].pin, GPIO_OUT_MC_PWM0_H + 0x202 * using_chn); */
            gpio_mux_out(pwm_mio_data[chl].pin, GPIO_OUT_MC_PWM0_H + 0x202 * using_chn);
        }
    } else {
        if (pwm_mio_data[chl].inv_en) {            //l口每个周期，是否先输出低电平
            pwm_reg->ch_con0 |=  BIT(5);
        } else {
            pwm_reg->ch_con0 &= ~BIT(5);
        }
        mcpwm_set_l_duty(using_chn, pwm_mio_data[chl].duty);
        //L:
        if (pwm_mio_data[chl].pin < IO_MAX_NUM) {
            /* 出现类似comp_out[0] busy通道繁忙的断言可以先调用gpio_mux_out_close */
            /* gpio_mux_out_close(pwm_mio_data[chl].pin, GPIO_OUT_MC_PWM0_L + 0x202 * using_chn); */
            gpio_mux_out(pwm_mio_data[chl].pin, GPIO_OUT_MC_PWM0_L + 0x202 * using_chn);
        }
    }
    mcpwm_open(using_chn); 	 //mcpwm enable

    log_pwm_info(using_chn);
}
void mio_a_pwm_cpu_run(u32 chl, u32 pwm_var)
{
    if (chl > 3) {
        return;
    }
    u8 using_chn = 0;
    if (chl < 2) {
        using_chn = 0;
    } else {
        using_chn = 1;
    }
    u32 duty = pwm_var * 10000 / 255;
    /* log_info("duty is %d\n", duty); */
    if ((0 == chl) || (2 == chl)) {
        //H:
        mcpwm_set_h_duty(using_chn, duty);
    } else {
        //L:
        mcpwm_set_l_duty(using_chn, duty);
    }
}

void mio_a_io_cpu_init(u32 mask, JL_PORT_TypeDef *port, u32 offset)
{
    log_info("mio io init -> mask : 0x%x\n", mask);
    port->PU0 &= ~(mask << offset);
    port->PU1 &= ~(mask << offset);
    port->PD0 |= (mask << offset);
    port->PD1 |= (mask << offset);
    port->DIR &= ~(mask << offset);
    port->OUT &= ~(mask << offset);
    /* log_info("PA12 0x%x\n", JL_OMAP->PA12_OUT); */
}
#endif
