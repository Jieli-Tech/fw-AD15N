#include "config.h"
#include "common.h"




AT_RAM
void delay(volatile u32 t)
{
    while (t--) {
        asm("nop");
    }
}

void system_enter_critical(void)
{
}

void system_exit_critical(void)
{
}




