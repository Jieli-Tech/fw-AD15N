#include "sfr.h"
#include "cpu.h"
#include "config.h"
#include "clock.h"
#include "mcpwm.h"
#include "app_modules.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"


#define pwm_frq_cnt(f)  (clk_get("lsb")/f)




#define _pwm_frq_duty(channel,frq,duty) \
    JL_PWM->TMR##channel##_PR = pwm_frq_cnt(frq); \
    JL_PWM->CH##channel##_CMP = pwm_frq_cnt(frq) * duty / 100;

const u32 mcpwm_tab[4] = {
    IO_PORTA_06,
    IO_PORTA_07,
    IO_PORTA_11,
    IO_PORTA_12,
};


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
    log_info("JL_PWM->PWMCON1:%x \n", JL_PWM->PWMCON1);
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


/*
 *
 * mio功能驱动
 */

#if defined(HAS_MIO_EN) && (HAS_MIO_EN)


void mio_a_pwm_cpu_init(u32 chl, u32 gpio, u32 frequency)
{
    gpio_set_pull_up(gpio, 1);
    gpio_set_pull_down(gpio, 1);
    SFR(JL_IOMC->IOMC3, 0, 3, 0);
    gpio_set_direction(gpio, 0);
    gpio_set_die(gpio, 0);
    JL_PWM->TMR0_PR = sys_clock_get() / frequency;
    JL_PWM->CH0_CMP = 0;
    SFR(JL_PWM->PWMCON1, (1 + chl * 4), 3, chl);
    JL_PWM->TMR0_CON = BIT(5);
    JL_PWM->PWMCON0 |= BIT(8 + chl);
    JL_PWM->PWMCON0 |= BIT(chl);
}
void mio_a_pwm_cpu_run(u32 chl, u32 pwm_var)
{
    local_irq_disable();
    u16 tmr_pr = JL_PWM->TMR0_PR;
    JL_PWM->CH0_CMP = (tmr_pr + 1) * pwm_var / 255;	//pwm_var范围在0-255
    local_irq_enable();
}
void mio_a_io_cpu_init(u32 mask, JL_PORT_TypeDef *port, u32 offset)
{
    log_info("mio io init -> mask : 0x%x\n", mask);
    port->PU &= ~(mask << offset);
    port->PD |= (mask << offset);
    port->DIR &= ~(mask << offset);
    port->OUT &= ~(mask << offset);
}

#endif
