#ifndef CPU_IRQ_H
#define CPU_IRQ_H


#include "hwi.h"
#include "typedef.h"

extern void enter_critical_hook();
extern void exit_critical_hook();

#define     CPU_INT_DIS local_irq_disable
#define     CPU_INT_EN  local_irq_enable

#define     OS_ENTER_CRITICAL   CPU_INT_DIS
#define     OS_EXIT_CRITICAL    CPU_INT_EN

extern __attribute__((weak)) void tick_timer_set(bool);
extern __attribute__((weak)) bool tick_timer_close(void);

#define tick_timer_set_api(n)        if(tick_timer_set)   { tick_timer_set(n);   }
#define tick_timer_close_api()      if(tick_timer_close) { tick_timer_close(); }

void _OS_EXIT_CRITICAL(void);
void _OS_ENTER_CRITICAL(u32 bit_list_l, u32 bit_list_h);

#define NEW_OS_ENTER_CRITICAL(l, h) _OS_ENTER_CRITICAL(l, h)
#define NEW_OS_EXIT_CRITICAL()  	_OS_EXIT_CRITICAL()
// #define ___interrupt 	__attribute__((interrupt("")))

#endif

