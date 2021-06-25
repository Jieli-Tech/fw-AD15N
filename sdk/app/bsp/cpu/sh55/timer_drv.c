#include "sfr.h"
#include "cpu.h"
#include "config.h"
#include "gpio.h"
#include "clock.h"
#include "mcpwm.h"
#include "timer_drv.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"

#define TIMER_SFR(ch) JL_TMR##ch

/*
 *timer
 * */
#define _timer_init(ch,us)  \
    HWI_Install(IRQ_TIME##ch##_IDX, (u32)timer##ch##_isr, 0); 	\
	TIMER_SFR(ch)->PRD = clk_get("lsb")/1000000 * us;			\
	TIMER_SFR(ch)->CON = BIT(0)|BIT(6);

SET(interrupt(""))
static void timer0_isr(void)
{
    TIMER_SFR(0)->CON |= BIT(6);
    log_char('0');
    JL_PORTA->DIR &= ~BIT(4);
    JL_PORTA->DIE |= BIT(4);
    JL_PORTA->OUT ^= BIT(4);
}
SET(interrupt(""))
static void timer1_isr(void)
{
    TIMER_SFR(1)->CON |= BIT(6);
    log_char('1');
}
SET(interrupt(""))
static void timer2_isr(void)
{
    TIMER_SFR(2)->CON |= BIT(6);
    log_char('2');
}

void timer_init(u8 timer_ch, u32 us)
{
    switch (timer_ch) {
    case 0:
        _timer_init(0, us);
        break;
    case 1:
        _timer_init(1, us);
        break;
    case 2:
        _timer_init(2, us);
        break;
    default:
        break;
    }
}

/*
 *timer pwm
 * */
const u32 timer2_pwm_tab[] = {
    IO_PORTA_03,
    IO_PORTA_02,
};

#define _timer2_pwm_init(ch,fre,duty)  							\
	TIMER_SFR(2)->PRD = clk_get("lsb")/fre;						\
	TIMER_SFR(2)->PWM##ch = TIMER_SFR(2)->PRD*duty/100;			\
	TIMER_SFR(2)->CON |= BIT(8 + 4*ch);							\
    gpio_set_direction(timer2_pwm_tab[ch], 0);      			\
    gpio_set_die(timer2_pwm_tab[ch], 0);      			\
	JL_IOMC->IOMC1 |= BIT(15 + ch);							\
	TIMER_SFR(2)->CON |= BIT(0)|BIT(6);

void timer2_pwm_init(u8 ch, u32 fre, u8 duty)
{
    switch (ch) {
    case 0:
        _timer2_pwm_init(0, fre, duty);
        break;
    case 1:
        _timer2_pwm_init(1, fre, duty);
        break;
    default:
        break;
    }
}

