//*********************************************************************************//
// Module name : HWI                                                               //
// Description : hardware interrupt subroutine                                     //
// By Designer : zequan_liu                                                        //
// Dat changed :                                                                   //
//*********************************************************************************//

#include "includes.h"
#include "uart.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"


AT_POWER
void error_isr_handler(void)
{
    //log_info(__func__);
    while (1);
}

static unsigned int icfg_ptr_bakup[MAX_IRQ_ENTRY_NUM / 8];
AT_RAM
void irq_save(void)
{
    volatile unsigned int *icfg_ptr = &(j32CPU(core_num())->ICFG00);
    for (u8 i = 0; i < MAX_IRQ_ENTRY_NUM / 8; i++) {
        icfg_ptr_bakup[i] = icfg_ptr[i];
        icfg_ptr[i] &= ~(BIT(0) | BIT(4) | BIT(8) | BIT(12) | BIT(16) | BIT(20) | BIT(24) | BIT(28));
    }
}
AT_RAM
void irq_resume(void)
{
    volatile unsigned int *icfg_ptr = &(j32CPU(core_num())->ICFG00);
    for (u8 i = 0; i < MAX_IRQ_ENTRY_NUM / 8; i++) {
        icfg_ptr[i] = icfg_ptr_bakup[i];
    }
}
AT_RAM
void irq_enable(u8 index)
{
    unsigned int icfg_num =  index / 8;
    unsigned int icfg_bit = (index % 8) * 4;
    volatile unsigned int *icfg_ptr = &(j32CPU(core_num())->ICFG00);
    icfg_ptr[icfg_num] |= BIT(icfg_bit);
}

//---------------------------------------------//
// HWI_Install
//---------------------------------------------//

void HWI_Install(unsigned char index, unsigned int isr, unsigned char priority)
{
    unsigned int icfg_num =  index / 8;
    unsigned int icfg_bit = (index % 8) * 4;
    unsigned int *israddr = (unsigned int *)IRQ_MEM_ADDR;
    israddr[index] = isr;

    local_irq_disable();

    unsigned char icfg_wdat = ((priority & 0x7) << 1) | BIT(0);
    unsigned int icfg_clr = ~(0xf << icfg_bit);
    unsigned int icfg_set = icfg_wdat << icfg_bit;

    volatile unsigned int *icfg_ptr = &(j32CPU(core_num())->ICFG00);
    icfg_ptr[icfg_num] = (icfg_ptr[icfg_num] & icfg_clr) | icfg_set;

    local_irq_enable();
}

//---------------------------------------------//
// HWI_Install
//---------------------------------------------//

void HWI_Uninstall(unsigned char index)
{
    unsigned int icfg_num =  index / 8;
    unsigned int icfg_bit = (index % 8) * 4;
    unsigned int *israddr = (unsigned int *)IRQ_MEM_ADDR;
    israddr[index] = (u32)error_isr_handler;

    local_irq_disable();

    unsigned char icfg_wdat = ((0 & 0x7) << 1) | BIT(0);
    unsigned int icfg_clr = ~(0xf << icfg_bit);
    unsigned int icfg_set = 0;

    volatile unsigned int *icfg_ptr = &(j32CPU(core_num())->ICFG00);
    icfg_ptr[icfg_num] = (icfg_ptr[icfg_num] & icfg_clr) | icfg_set;

    local_irq_enable();
}

//------------------------------------------------------//
// interrupt ip
//------------------------------------------------------//

void reg_set_ip(unsigned char index, unsigned char priority)
{

    unsigned int icfg_num =  index / 8;
    unsigned int icfg_bit = (index % 8) * 4;

    local_irq_disable();

    unsigned char icfg_wdat = (priority & 0x7) << 1;
    unsigned int icfg_clr = ~(0xe << icfg_bit);
    unsigned int icfg_set = icfg_wdat << icfg_bit;

    volatile unsigned int *icfg_ptr = &(j32CPU(core_num())->ICFG00);
    icfg_ptr[icfg_num] = (icfg_ptr[icfg_num] & icfg_clr) | icfg_set;

    local_irq_enable();
}

//------------------------------------------------------//
// interrupt ie
//------------------------------------------------------//

void bit_set_ie(unsigned char index)
{
    unsigned int icfg_num =  index / 8;
    unsigned int icfg_bit = (index % 8) * 4;

    local_irq_disable();
    volatile unsigned int *icfg_ptr = &(j32CPU(core_num())->ICFG00);
    icfg_ptr[icfg_num] |= BIT(icfg_bit);
    local_irq_enable();
}

void bit_clr_ie(unsigned char index)
{
    unsigned int icfg_num =  index / 8;
    unsigned int icfg_bit = (index % 8) * 4;

    local_irq_disable();
    volatile unsigned int *icfg_ptr = &(j32CPU(core_num())->ICFG00);
    icfg_ptr[icfg_num] &= ~BIT(icfg_bit);
    local_irq_enable();
}

AT_RAM
void _OS_ENTER_CRITICAL(u8 no_close_isr_index)
{
    OS_ENTER_CRITICAL();
    irq_save();
    irq_enable(no_close_isr_index);  //open your isr
    isr_tick_timer_close = 1;

    OS_EXIT_CRITICAL();
}

AT_RAM
void _OS_EXIT_CRITICAL(void)
{
    OS_ENTER_CRITICAL();
    irq_resume();
    OS_EXIT_CRITICAL();
    isr_tick_timer_close = 0;

}

//------------------------------------------------------//
// soft interrupt
//------------------------------------------------------//
#if 0
void bit_set_swi(unsigned char index)
{
    j32CPU(core_num())->ILAT_SET = BIT(index);
}

void bit_clr_swi(unsigned char index)
{
    j32CPU(core_num())->ILAT_CLR = BIT(index);
}
#endif
//*********************************************************************************//
//                                                                                 //
//                               end of this module                                //
//                                                                                 //
//*********************************************************************************//

