#ifndef _CONFIG_
#define _CONFIG_

#include "includes.h"
#include "common.h"
#include "errno-base.h"



#if UART_DEBUG
#define  DEBUG_EN
#endif

extern const char libs_debug;

#ifdef DEBUG_EN
#define debug_init(x)           uart_init(x)
#define debug_uninit()          uart_uninit()
// #define debug_putchar(x)        putchar(x)
// #define debug_puts(x)           puts(x)
// #define debug_u32d(x)           put_u32d(x)
// #define debug_u32hex(x)         put_u32hex(x)
// #define debug_u16hex0(x)        put_u16hex0(x)
#define debug_u8hex(x)          put_u8hex(x)
// #define debug_buf(x,y)          printf_buf(x,y)
// #define debug                   printf
#else
#define debug_init(...)
#define debug_uninit()
// #define debug_putchar(...)
// #define debug_puts(...)
// #define debug_u32d(...)
// #define debug_u32hex(...)
// #define debug_u16hex0(...)
#define debug_u8hex(...)
// #define debug_buf(...)
#endif
#define debug(...)


//< huayue add
#define     IO_DEBUG_0(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}
#define     IO_DEBUG_1(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}
#define     IO_DEBUG_TOGGLE(i,x)  {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT ^= BIT(x);}

#define set_ie              bit_set_ie
#define clear_ie            bit_clr_ie

#define TRIGGER()           __asm__ volatile ("trigger")


#define SYS_CLOCK 48000000L


#endif
