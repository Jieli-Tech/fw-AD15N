#ifndef __IRQ_API_H__
#define __IRQ_API_H__
#include "hwi.h"
#include "typedef.h"




//#define IRQ_REGISTER(idx, hdl) \
//    SET_INTERRUPT void irq_##hdl() \
//    {\
//        hdl();\
//        /*irq_common_handler(idx); */\
//    }
//
//#define IRQ_REQUEST(idx, hdl) \
//	HWI_Install(idx, (u32)irq_##hdl, irq_index_to_prio(idx))

void HWI_Uninstall(unsigned char index);

#define     CPU_INT_DIS local_irq_disable
#define     CPU_INT_EN  local_irq_enable

#define     OS_ENTER_CRITICAL   CPU_INT_DIS
#define     OS_EXIT_CRITICAL    CPU_INT_EN

extern __attribute__((weak)) void tick_timer_set(bool);
extern __attribute__((weak)) bool tick_timer_close(void);

#define tick_timer_set_api(n)        if(tick_timer_set)   { tick_timer_set(n);   }
#define tick_timer_close_api()      if(tick_timer_close) { tick_timer_close(); }


void _OS_EXIT_CRITICAL(void);
void _OS_ENTER_CRITICAL(u32 bit_list);
void irq_index_tab_reg(void *tab, u32 max_cnt);
u8 irq_index_to_prio(u8 idx);

#endif	/*	_AD100_H	*/

