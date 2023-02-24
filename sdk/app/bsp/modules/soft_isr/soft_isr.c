#include  "soft_isr.h"

#if 0
#define LOG_TAG_CONST       SOFI
#define LOG_TAG             "[soft isr]"
#include "log.h"

#define LOG_INFO
#ifdef  LOG_INFO
#define    si_printf      log_info
#define    si_putchar
#define    si_printf_buf  log_info_hexdump
#else
#define    si_printf
#define    si_putchar
#define    si_printf_buf
#endif

SET(interrupt(""))
void demo_soft0_isr()
{
    bit_clr_swi(0);

    si_putchar('0');
}

SET(interrupt(""))
void demo_soft1_isr()
{
    bit_clr_swi(1);

    si_putchar('1');
}

SET(interrupt(""))
void demo_soft2_isr()
{
    bit_clr_swi(2);

    si_putchar('2');
}

SET(interrupt(""))
void demo_soft3_isr()
{
    bit_clr_swi(3);

    si_putchar('3');
}

void demo_soft_init(void)
{
    HWI_Install(IRQ_SOFT0_IDX, (u32)demo_soft0_isr, 0) ;
    HWI_Install(IRQ_SOFT1_IDX, (u32)demo_soft1_isr, 0) ;
    HWI_Install(IRQ_SOFT2_IDX, (u32)demo_soft2_isr, 0) ;
    HWI_Install(IRQ_SOFT3_IDX, (u32)demo_soft3_isr, 0) ;

    bit_set_swi(0);
    bit_set_swi(1);
    bit_set_swi(2);
    bit_set_swi(3);
}
#endif
/* AT_RAM */
AT(.audio_d.text.cache.L2)
void bit_set_swi0(void)
{
    bit_set_swi(0);
}

