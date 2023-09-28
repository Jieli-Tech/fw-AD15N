#include "config.h"
#include "common.h"
#include "boot.h"
#include "msg.h"
#include "vm_api.h"
#include "vm_sfc.h"
#include "init.h"
#include "clock.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

extern void exception_irq_handler(void);
void all_init_isr(void)
{
    u32 i;
    unsigned int *israddr = (unsigned int *)IRQ_MEM_ADDR;
    for (i = 0; i < MAX_IRQ_ENTRY_NUM; i++) {
        israddr[i] = (u32)exception_irq_handler;
    }
}

sec_used(.version)
u8 const lib_update_version[] = "\x7c\x4f\x94\x0aUPDATE-@20210816-$9c89ae0";

void test_audio_dac(void);
void clock_2_64m(void);
void system_init(void)
{
    emu_init();
}




