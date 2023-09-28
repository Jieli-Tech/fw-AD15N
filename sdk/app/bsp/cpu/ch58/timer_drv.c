#include "gpio.h"
#include "clock.h"
#include "timer_drv.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#define TIMER_SFR(ch) JL_TIMER##ch

/*
 *timer
 * */
#define _timer_init(ch,us)  \
    HWI_Install(IRQ_TIME##ch##_IDX, (u32)timer##ch##_isr, 0); 	\
	SFR(TIMER_SFR(ch)->CON, 10, 4, 5);	\
	SFR(TIMER_SFR(ch)->CON, 4, 4, 8);	\
	TIMER_SFR(ch)->CNT = 0;	\
	TIMER_SFR(ch)->PRD = 12000000 / 1000000 * us / 256;			\
	TIMER_SFR(ch)->CON |= BIT(0);	\
	TIMER_SFR(ch)->CON |= BIT(14);

SET(interrupt(""))
static void timer0_isr(void)
{
    TIMER_SFR(0)->CON |= BIT(14);
    putchar('A');
    JL_PORTA->DIR &= ~BIT(4);
    JL_PORTA->DIE |= BIT(4);
    JL_PORTA->OUT ^= BIT(4);
}
SET(interrupt(""))
static void timer1_isr(void)
{
    TIMER_SFR(1)->CON |= BIT(14);
    putchar('B');
}
SET(interrupt(""))
static void timer2_isr(void)
{
    TIMER_SFR(2)->CON |= BIT(14);
    putchar('C');
}

SET(interrupt(""))
static void timer3_isr(void)
{
    TIMER_SFR(3)->CON |= BIT(14);
    putchar('D');
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
    case 3:
        _timer_init(3, us);
        break;
    default:
        break;
    }
}

/*
 *timer pwm
 * */
const u32 timer_pwm_tab[] = {
    IO_PORTA_01,
    IO_PORTA_02,
    IO_PORTA_03,
    IO_PORTC_08,
};

const u32 timer_pwm_gpio_out_tab [] = {
    GPIO_OUT_COMP4_TIMER0_PWM,
    GPIO_OUT_COMP5_TIMER1_PWM,
    GPIO_OUT_COMP6_TIMER2_PWM,
    GPIO_OUT_COMP7_TIMER3_PWM,
};

#define _timer_pwm_init(ch,fre,duty)  							\
	TIMER_SFR(ch)->PRD = clk_get("lsb") / (4 * fre);						\
	TIMER_SFR(ch)->PWM = TIMER_SFR(ch)->PRD * duty / 100;			\
	TIMER_SFR(ch)->CNT = 0;	\
	SFR(TIMER_SFR(ch)->CON, 10, 4, 1);	\
	SFR(TIMER_SFR(ch)->CON, 4, 4, 1);	\
    gpio_set_direction(timer_pwm_tab[ch], 0);      			\
    gpio_set_die(timer_pwm_tab[ch], 0);      			\
	gpio_mux_out(timer_pwm_tab[ch], timer_pwm_gpio_out_tab[ch]);	\
	TIMER_SFR(ch)->CON |= BIT(0) | BIT(8) | BIT(14);

void timer_pwm_init(u8 ch, u32 fre, u8 duty)
{
    switch (ch) {
    case 0:
        _timer_pwm_init(0, fre, duty);
        break;
    case 1:
        _timer_pwm_init(1, fre, duty);
        break;
    case 2:
        _timer_pwm_init(2, fre, duty);
        break;
    case 3:
        _timer_pwm_init(3, fre, duty);
        break;
    default:
        break;
    }
}


