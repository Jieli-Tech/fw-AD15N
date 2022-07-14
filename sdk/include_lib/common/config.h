#ifndef _CONFIG_
#define _CONFIG_

#include "includes.h"
#include "common.h"
#include "errno-base.h"



#if UART_LOG
#define  UT_LOG_EN
#endif

extern const char libs_debug;

#ifdef UT_LOG_EN
#define log_init(x)           uart_init(x)
#define log_uninit()          uart_uninit()
// #define log_putchar(x)        putchar(x)
// #define log_puts(x)           puts(x)
// #define log_u32d(x)           put_u32d(x)
// #define log_u32hex(x)         put_u32hex(x)
// #define log_u16hex0(x)        put_u16hex0(x)
#define log_u8hex(x)          put_u8hex(x)
// #define log_buf(x,y)          printf_buf(x,y)
// #define log                   printf
#else
#define log_init(...)
#define log_uninit()
// #define log_putchar(...)
// #define log_puts(...)
// #define log_u32d(...)
// #define log_u32hex(...)
// #define log_u16hex0(...)
#define log_u8hex(...)
// #define log_buf(...)
#endif
#define log(...)


//< huayue add
#define     IO_DEBUG_0(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}
#define     IO_DEBUG_1(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}
#define     IO_DEBUG_TOGGLE(i,x)  {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT ^= BIT(x);}

#define set_ie              bit_set_ie
#define clear_ie            bit_clr_ie

#define TRIGGER()           __asm__ volatile ("trigger")


#define SYS_CLOCK 48000000L


#endif
