#include "sfr.h"
#include "cpu.h"
#include "config.h"
#include "gpio.h"
#include "clock.h"
#include "mcpwm.h"
#include "timer_drv.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#define TIMER_SFR(ch) JL_TIMER##ch

/*
 *timer
 * */
#define _timer_init(ch,us)  \
    HWI_Install(IRQ_TIME##ch##_IDX, (u32)timer##ch##_isr, 6);\
	TIMER_SFR(ch)->PRD = clk_get("timer") / 16 / 1000000 * us;/*需注意是否溢出*/\
	TIMER_SFR(ch)->CON = BIT(14)/*clear pnd*/ | \
                            (4 << 10)/*STD_48M时钟源*/ | \
                            (2 << 4)/*16分频*/ | \
                            (1 << 0)/*cnt start*/;\
    log_info("timer%d init, con : 0x%x", ch, TIMER_SFR(ch)->CON);

SET(interrupt(""))
static void timer0_isr(void)
{
    TIMER_SFR(0)->CON |= BIT(14);
    JL_PORTA->DIR &= ~BIT(1);
    JL_PORTA->OUT ^= BIT(1);
    /* log_char('0'); */
}
SET(interrupt(""))
static void timer1_isr(void)
{
    TIMER_SFR(1)->CON |= BIT(14);
    log_char('1');
}
SET(interrupt(""))
static void timer2_isr(void)
{
    TIMER_SFR(2)->CON |= BIT(14);
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
