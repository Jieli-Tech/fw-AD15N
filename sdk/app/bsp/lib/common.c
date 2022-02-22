#include "config.h"
#include "common.h"
#include "circular_buf.h"
#include "clock.h"




AT_RAM
void delay(volatile u32 t)
{
    while (t--) {
        asm("nop");
    }
}

AT_RAM
void udelay(u32 us)
{
    u32 mips = (sys_clock_get() / 1000000 / 8);

    while (us--) {
        for (int i = 0; i < mips; i++) {
            asm("nop");
        }
    }
}

__attribute__((always_inline))
void enter_critical_hook()
{
    tick_timer_set_api(1);
}
__attribute__((always_inline))
void exit_critical_hook()
{
    tick_timer_set_api(0);
}

void system_enter_critical(void)
{
}

void system_exit_critical(void)
{
}

AT_RAM
u32 cbuf_write(cbuffer_t *cbuffer, void *buf, u32 len)
{
    extern u32 _cbuf_write(cbuffer_t *cbuffer, void *buf, u32 len);
    OS_ENTER_CRITICAL();
    u32 res = _cbuf_write(cbuffer, buf, len);
    OS_EXIT_CRITICAL();
    return res;
}


