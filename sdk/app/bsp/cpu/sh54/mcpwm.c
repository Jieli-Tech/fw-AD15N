#include "sfr.h"
#include "cpu.h"
#include "config.h"
#include "gpio.h"
#include "clock.h"
#include "mcpwm.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#define pwm_frq_cnt(f)  (clk_get("lsb")/f)

const u32 mcpwm_tab[4] = {
    IO_PORTA_04,
    IO_PORTA_05,
    IO_PORTA_08,
    IO_PORTA_09,
};

#define _pwm_frq_duty(channel,frq,duty) \
    JL_PWM->TMR##channel##_PR = pwm_frq_cnt(frq); \
    JL_PWM->CH##channel##_CMP = pwm_frq_cnt(frq) * duty / 100;


#define _mcpwm_set(channel,frq,duty)                 \
    _pwm_frq_duty(channel,frq,duty);                 \
    /*sel pwm use mcpwmtmr0*/                       \
    SFR(JL_PWM->PWMCON1, (1+channel*4), 2, channel);\
    SFR(JL_PWM->PWMCON1, (channel*4), 1, 0);		\
    /* mcpwm tmr0 en */                             \
    JL_PWM->TMR##channel##_CON = BIT(5);            \
    JL_PWM->PWMCON0 |= BIT(8+channel);              \
    /*mcpwm ch0 en  */                              \
    JL_PWM->PWMCON0 |= BIT(channel);                \
    gpio_set_direction(mcpwm_tab[channel], 0);      \
	JL_IOMC->IOMC0 |= BIT(16+channel);

void mcpwm_set(u8 channel, u32 fre, u32 duty)
{
    switch (channel) {
    case 0:
        _mcpwm_set(0, fre, duty);
        break;
    case 1:
        _mcpwm_set(1, fre, duty);
        break;
    case 2:
        _mcpwm_set(2, fre, duty);
        break;
    case 3:
        _mcpwm_set(3, fre, duty);
        break;
    default:
        break;
    }
}

void pwm_frq_duty(u8 channel, u32 fre, u32 duty)
{
    switch (channel) {
    case 0:
        _pwm_frq_duty(0, fre, duty);
        break;
    case 1:
        _pwm_frq_duty(1, fre, duty);
        break;
    case 2:
        _pwm_frq_duty(2, fre, duty);
        break;
    case 3:
        _pwm_frq_duty(3, fre, duty);
        break;
    default:
        break;
    }
}

void test_mcpwm(void)
{
    log_info("*****test mcpwm****\n");
    mcpwm_set(0, 10000, 50); //fre 1000,50%占空比
    mcpwm_set(1, 10000, 40); //fre 1000,40%占空比
    mcpwm_set(2, 10000, 30); //fre 1000,30%占空比
    mcpwm_set(3, 10000, 20); //fre 1000,20%占空比
}

