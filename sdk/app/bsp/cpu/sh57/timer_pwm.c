#include "includes.h"
#include "gpio.h"
#include "timer_pwm.h"
#include "log.h"

#define LOG_TAG_CONST       NORM//TIMER_PWM
#define LOG_TAG             "[timer_pwm]"

static const u32 TIMERx_table[3] = {
    (u32)JL_TIMER1,
    (u32)JL_TIMER2,
    (u32)JL_TIMER0,
};

/**
 * @param JL_TIMERx : JL_TIMER0/1/2
 * @param pwm_io : JL_PORTA_01, JL_PORTB_02,,,等等，支持任意普通IO
 * @param fre : 频率，单位Hz
 * @param duty : 初始占空比，0~10000对应0~100%
 */
void timer_pwm_init(JL_TIMER_TypeDef *JL_TIMERx, u32 pwm_io, u32 fre, u32 duty)
{
    u8 tmr_num;
    for (tmr_num = 0; tmr_num < 3; tmr_num ++) {
        if ((u32)JL_TIMERx == TIMERx_table[tmr_num]) {
            break;
        }
        if (tmr_num == 2) {
            return;
        }
    }
    if ((u32)JL_TIMERx == (u32)JL_TIMER0) {
        gpio_set_die(IO_PORTA_03, 1);
        gpio_set_pull_down(IO_PORTA_03, 0);
        gpio_set_pull_up(IO_PORTA_03, 0);
        gpio_set_direction(IO_PORTA_03, 0);
    } else {
        gpio_och_sel_output_signal(pwm_io, OUTPUT_CH_SIGNAL_TIMER1_PWM + tmr_num);
        gpio_set_die(pwm_io, 1);
        gpio_set_pull_up(pwm_io, 0);
        gpio_set_pull_down(pwm_io, 0);
        gpio_set_direction(pwm_io, 0);
    }

    //初始化timer
    JL_TIMERx->CON = 0;
    SFR(JL_TIMERx->CON, 10, 4, TIMER_SRC_STD_48M);
    u32 timer_clk = 48000000;
    SFR(JL_TIMERx->CON, 4, 4, TIMER_PRESCALE_64);
    JL_TIMERx->CNT = 0;								//清计数值
    JL_TIMERx->PRD = timer_clk / (64 * fre);			//设置周期

    //设置初始占空比
    JL_TIMERx->PWM = (JL_TIMERx->PRD * duty) / 10000;	//0~10000对应0~100%
    JL_TIMERx->CON |= BIT(8) | (0b01 << 0); 			//计数模式
}

/**
 * @param JL_TIMERx : JL_TIMER0/1/2/3/4/5
 * @param duty : 占空比，0~10000对应0~100%
 */
void set_timer_pwm_duty(JL_TIMER_TypeDef *JL_TIMERx, u32 duty)
{
    JL_TIMERx->PWM = (JL_TIMERx->PRD * duty) / 10000;	//0~10000对应0~100%
}


/********************************* 以下SDK的参考示例 ****************************/
#if 0
void timer_pwm_test(void)
{
    log_info("*********** timer pwm test *************\n");
    timer_pwm_init(JL_TIMER0, IO_PORTA_03, 5000, 2000); //1KHz 50%
    timer_pwm_init(JL_TIMER1, IO_PORTA_10, 1000, 5000); //1KHz 50%
    timer_pwm_init(JL_TIMER2, IO_PORTA_07, 10000, 7000);
    log_info("JL_TIMER1->CON = 0x%x\n", JL_TIMER1->CON);
    log_info("JL_TIMER2->CON = 0x%x\n", JL_TIMER2->CON);
}
#endif
