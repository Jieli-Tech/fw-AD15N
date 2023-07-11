
/* #pragma bss_seg(".uart.data.bss") */
/* #pragma data_seg(".uart.data") */
/* #pragma const_seg(".uart.text.const") */
/* #pragma code_seg(".uart.text") */
/* #pragma str_literal_override(".uart.text.const") */

#include "config.h"
#include "clock.h"
#include "uart.h"
#include "app_config.h"

#if UART_LOG

#define     DEBUG_UART  JL_UART0

AT(..log_ut.text.cache.L2)
void ut_putchar(char a)
{
    u32 i = 0x10000;
    if (!(DEBUG_UART->CON0 & BIT(0))) {
        return;
    }
    while (((DEBUG_UART->CON0 & BIT(15)) == 0) && (0 != i)) {  //TX IDLE
        i--;
    }
    DEBUG_UART->CON0 |= BIT(13);  //清Tx pending

    DEBUG_UART->BUF = a;
    __asm__ volatile("csync");
}

char ut_getchar(void)
{
    char c;
    c = 0;
    if (DEBUG_UART->CON0 & BIT(14)) {
        c = DEBUG_UART->BUF;
        DEBUG_UART->CON0 |=  BIT(12);

    }
    return c;
}


void uart_init(u32 fre)
{

    SFR(JL_CLOCK->CLK_CON3, 3, 2, 0b01);
#if (UART_TX_OUTPUT_CH_EN == 1)
    if (UART_OUTPUT_CH_PORT < IO_PORT_MAX) {
        gpio_direction_output(UART_OUTPUT_CH_PORT, 1);
        gpio_set_fun_output_port(UART_OUTPUT_CH_PORT, FO_UART0_TX, 1, 1);
    }
#else
    JL_PORTA->DIR &= ~BIT(4);
    JL_OMAP->PA4_OUT = FO_UART0_TX ;
#endif
    //JL_PORTA->DIR |= BIT(5);
    //JL_PORTA->DIE |= BIT(5);
    //JL_IMAP->FI_UART0_RX = PA5_IN;

    /* DEBUG_UART->BAUD = (sys_clock_get() / 1000000) / 4 - 1; */
    DEBUG_UART->BAUD = (48000000 / fre) / 4 - 1;
    DEBUG_UART->CON0 = BIT(13) | BIT(12) | BIT(1) | BIT(0);
    DEBUG_UART->CON1 = 0;//
    DEBUG_UART->CON0 |= BIT(13);  //清Tx pending

}

char get_byte(void)
{
    return ut_getchar();
}


AT(..log_ut.text.cache.L2)
int putchar(int a)
{
    ut_putchar(a);
    return a;
}

char getchar(void)
{
    return ut_getchar();
}

void put_u4hex(u8 dat)
{
    dat = 0xf & dat;

    if (dat > 9) {
        putchar(dat - 10 + 'A');
    } else {
        putchar(dat + '0');
    }
}

static void xput_u32hex(u32 dat)
{
    putchar('0');
    putchar('x');
    put_u4hex(dat >> 28);
    put_u4hex(dat >> 24);

    put_u4hex(dat >> 20);
    put_u4hex(dat >> 16);

    put_u4hex(dat >> 12);
    put_u4hex(dat >> 8);

    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putchar('\n');
}

void put_u8hex(u8 dat)
{
    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putchar(' ');
}

void printf_buf(u8 *buf, u32 len)
{

    u32 i ;
    xput_u32hex((u32)buf) ;

    for (i = 0 ; i < len ; i++) {
        if ((i % 16) == 0) {
            if (0 != i) {
                putchar('\n') ;
            }
        }

        put_u8hex(buf[i]) ;
    }
    putchar('\n') ;
}

void put_buf(u8 *buf, u32 len)
{
    return printf_buf(buf, len);
}

#endif
