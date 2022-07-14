#include "config.h"
#include "clock.h"
#include "uart.h"
#include "app_config.h"
#include "gpio.h"

#define     UART  JL_UT0

#if UART_LOG

void uart_init(u32 fre)
{
    if (FALSE == libs_debug) {
        return;
    }
#if (UART_TX_OUTPUT_CH_EN == 1)
    uart_clk_sel();//JL_CLK->CON0 &= ~(0b11 << 8);//pll48
    UART->BAUD = clk_get("uart") / fre / 4 - 1;

    //PA口用OUTCHANNEL0/1 PB口用OUTCHANNEL2/3
    if (UART_OUTPUT_CH_PORT >= IO_PORTB_00) {
        gpio_output_channle(UART_OUTPUT_CH_PORT, CH3_UT0_TX);//io选择--->设置输出通道
    } else {
        gpio_output_channle(UART_OUTPUT_CH_PORT, CH1_UT0_TX);//io选择--->设置输出通道
    }

    UART->CON |= BIT(13) | BIT(12)  | BIT(0);
    UART->BUF = ' ';
#else
    uart_clk_sel();//JL_CLK->CON0 &= ~(0b11 << 8);//pll48
#if 1
    JL_IOMC->IOMC0 &= ~BIT(6);
    JL_IOMC->IOMC0 &= ~BIT(5);
    JL_IOMC->IOMC0 |=  BIT(4);

    JL_PORTA->DIR &= ~BIT(6);
    JL_PORTA->DIR |= BIT(7);
#else
    JL_IOMC->IOMC0 &= ~BIT(6);
    JL_IOMC->IOMC0 |= BIT(5);
    JL_IOMC->IOMC0 |=  BIT(4);

    JL_PORTA->DIR &= ~BIT(0);
    JL_PORTA->DIR |= BIT(1);
#endif

    UART->BAUD = (clk_get("uart") / fre) / 4 - 1;
    UART->CON = BIT(13) | BIT(12) | BIT(0);
    UART->CON |= BIT(13);  //清Tx pending
    UART->BUF = ' ';

#endif
}

void uart_uninit(void)
{
    if (FALSE == libs_debug) {
        return;
    }
#if (UART_TX_OUTPUT_CH_EN == 1)
    UART->CON = BIT(13) | BIT(12);
    gpio_set_direction(UART_OUTPUT_CH_PORT, 1);
    gpio_set_pull_down(UART_OUTPUT_CH_PORT, 0);
    gpio_set_pull_up(UART_OUTPUT_CH_PORT, 0);
#else
    UART->CON = BIT(13) | BIT(12);
#endif
}
AT(..log_ut.text.cache.L2)
int putchar(int a)
{
    if (FALSE == libs_debug) {
        return 0;
    }
    u32 i = 0;
    if (UART->CON & BIT(0)) {
        __asm__ volatile("csync");
        while ((UART->CON & BIT(15)) == 0) {
            i++;
            if (i > 0xc300) {
                break;
            }
        }    //TX IDLE
        UART->CON |= BIT(13);  //清Tx pending
        UART->BUF = a;
    }
    return a;
}
AT(..log_ut.text.cache.L2)
char get_byte(void)
{
    char c;
    c = 0;
    if (UART->CON & BIT(14)) {
        c = UART->BUF;
        UART->CON |= BIT(12);
    }
    return c;
}


AT(..log_ut.text.cache.L2)
void put_u4hex(u8 dat)
{
    dat = 0xf & dat;

    if (dat > 9) {
        putchar(dat - 10 + 'A');
    } else {
        putchar(dat + '0');
    }
}
AT(..log_ut.text.cache.L2)
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
#if 0

AT(..log_ut.text.cache.L2)
void put_u32hex0(u32 dat)
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
    putchar(' ');
}
AT(..log_ut.text.cache.L2)
u32 u32d_tab[] = {
    1000000000,
    100000000,
    10000000,
    1000000,
    100000,
    10000,
    1000,
    100,
    10,
    1,
};
AT(..log_ut.text.cache.L2)
void put_u32d(u32 dat)
{
    u32 i, tmp;
    u32 zero = 0;
    putchar(' ');
    //putchar('d');
    for (i = 0; i < (sizeof(u32d_tab) / 4); i++) {
        tmp = dat / u32d_tab[i];
        //put_u32hex(dat);
        //put_u32hex(tmp);
        //putchar('\n');
        if (tmp) {
            zero++;
            dat -= tmp * u32d_tab[i];
        }
        if ((0 != tmp) || (0 != zero)) {
            put_u4hex(tmp);
        }

    }
    if (0 == zero) {
        put_u4hex(tmp);
    }
    putchar('\n');
}
void put_u16hex0(u16 dat)
{
    put_u4hex(dat >> 12);
    put_u4hex(dat >> 8);

    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putchar(' ');
}
void put_u16hex(u16 dat)
{
    put_u4hex(dat >> 12);
    put_u4hex(dat >> 8);

    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putchar('\n');
}
#endif



#if 1
AT(..log_ut.text.cache.L2)
void put_u8hex(u8 dat)
{
    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putchar(' ');
}
AT(..log_ut.text.cache.L2)
void printf_buf(u8 *buf, u32 len)
{

    u32 i ;
    //putchar('\n') ;
    xput_u32hex((u32)buf) ;
    // put_u32hex(len);

    for (i = 0 ; i < len ; i++) {
        if ((i % 16) == 0) {
            if (0 != i) {
                // put_u32hex0(len);
                // put_u32hex0(i);
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
#else

AT(..log_ut.text.cache.L2)
char get_byte(void)
{
    char c;
    c = 0;
    return c;
}
AT(..log_ut.text.cache.L2)
int putchar(int a)
{
    return 0;
}

#endif

#endif	/*	UART_LOG	*/
