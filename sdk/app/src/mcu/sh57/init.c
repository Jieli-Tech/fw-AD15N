
#include "config.h"
#include "common.h"
#include "boot.h"
#include "msg.h"
#include "init.h"
#include "clock.h"
#include "efuse.h"
/* #include "efuse_trim_value.h" */

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

extern void exception_irq_handler(void);
void all_init_isr(void)
{
    u32 i;
    unsigned int *israddr = (unsigned int *)IRQ_MEM_ADDR;
    for (i = 0; i < 32; i++) {
        israddr[i] = (u32)exception_irq_handler;
    }
}



void system_init(void)
{
    pll_sel(PLL_320M, PLL_DIV2, PLL_B_DIV2);
    /* efuse_trim_value_init(); */
    /* sys_clock_get(); */

    //------
    emu_init();
}

