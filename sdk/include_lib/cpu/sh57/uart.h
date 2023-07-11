#ifndef _UART_
#define _UART_
#include "typedef.h"
// #include "wdt.h"

#define UART_LOG    1

#if UART_LOG

#define UART0_EN  0
#define UART1_EN  1
typedef enum {
    UART0_PA11_PA10,
    UART0_PA6_PA3,
    UART0_PA11_IOMUX_SEL,
    UART0_PA6_IOMUX_SEL,

    UART1_PB15_PB14,
    UART1_PA7_PA8,
    UART1_PB15_IOMUX_SEL,
    UART1_PA6_IOMUX_SEL,
} UART_MODULE;
void uart_init(u32 fre);
void uart_uninit(void);
int putchar(int a);
int puts(const char *out);
char get_byte(void);

void put_u32hex(u32 dat);
void put_u32hex0(u32 dat);
void put_u32d(u32 dat);
void put_u16hex0(u16 dat);
void put_u16hex(u16 dat);
void put_u8hex(u8 dat);
void printf_buf(u8 *buf, u32 len);
void put_buf(u8 *buf, u32 len);
#define my_puts(a) puts(a)

#else

#define uart_init(...)
#define debug_puts(...)
#define printf_buf(...)
#define put_u32hex(...)
#define put_u16hex(...)
#define put_u8hex(...)
#define put_buf(...)
// #define putchar(...)
int putchar(int a);
char get_byte(void);
#define my_puts(...)
#endif

#endif

