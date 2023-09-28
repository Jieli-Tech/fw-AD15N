
/* #pragma bss_seg(".uart.data.bss") */
/* #pragma data_seg(".uart.data") */
/* #pragma const_seg(".uart.text.const") */
/* #pragma code_seg(".uart.text") */
/* #pragma str_literal_override(".uart.text.const") */

#include "config.h"
#include "clock.h"
#include "uart.h"
#include "uart_v2.h"
#include "app_config.h"

#if UART_LOG


#if 1
//old uart driver
#define     DEBUG_UART          JL_UART0
#define     DEBUG_UART_OUTMAP   FO_UART0_TX
#define     DEBUG_UART_INMAP    PFI_UART0_RX
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

void uart_init(u32 fre)
{

    SFR(JL_CLOCK->PRP_CON0, 16, 3, 0b001);
    if (UART_OUTPUT_CH_PORT < IO_PORT_MAX) {
        //crossbar
        gpio_direction_output(UART_OUTPUT_CH_PORT, 1);
        gpio_set_fun_output_port(UART_OUTPUT_CH_PORT, DEBUG_UART_OUTMAP, 1, 1);
    }
#ifdef TCFG_UART_RX_PORT
    if (TCFG_UART_RX_PORT < IO_PORT_MAX) {
        //crossbar
        gpio_direction_input(TCFG_UART_RX_PORT);
        gpio_set_pull_up(TCFG_UART_RX_PORT, 1);
        gpio_set_die(TCFG_UART_RX_PORT, 1);
        gpio_set_fun_input_port(TCFG_UART_RX_PORT, DEBUG_UART_INMAP);
        /* #<{(| request_irq(IRQ_UART0_IDX, 0, uart_irq_handler, 0); |)}># */
    }
#endif
    DEBUG_UART->BAUD = (clk_get("uart") / fre) / 4 - 1;
    DEBUG_UART->CON0 = BIT(13) | BIT(12) | BIT(10) | BIT(1) | BIT(0);
    /* DEBUG_UART->CON0 = BIT(13) | BIT(12) | BIT(0); */
    DEBUG_UART->CON1 = 0;  //清Tx pending
    DEBUG_UART->CON0 |= BIT(13) | BIT(12);  //清Tx pending
}

#else

//new uart driver
#define     DEBUG_UART_NUM  0
#define     DEBUG_UART_DMA_EN   0

/* #define     TCFG_UART_BAUDRATE  1000000 */
#define     TCFG_UART_TX_PORT   UART_OUTPUT_CH_PORT
#define     TCFG_UART_RX_PORT   -1

#if DEBUG_UART_DMA_EN
#define     MAX_DEBUG_FIFO  256
static u8 debug_uart_buf[2][MAX_DEBUG_FIFO];
static u32 tx_jiffies = 0;
static u16 pos = 0;
static u8 uart_buffer_index = 0;

static void uart_irq(uart_dev uart_num, enum uart_event event)
{
    tx_jiffies = jiffies;
    uart_send_bytes(DEBUG_UART_NUM, debug_uart_buf[uart_buffer_index], pos);
    uart_buffer_index = !uart_buffer_index;
    pos = 0;
}
#endif
void uart_init(u32 freq)
{
    struct uart_config debug_uart_config = {
        .baud_rate = freq/*TCFG_UART_BAUDRATE*/,
        .tx_pin = TCFG_UART_TX_PORT,
        .rx_pin = TCFG_UART_RX_PORT,
    };
    uartx_init(DEBUG_UART_NUM, &debug_uart_config);

#if DEBUG_UART_DMA_EN
    struct uart_dma_config dma_config = {
        .event_mask = UART_EVENT_TX_DONE,
        .irq_callback = uart_irq,
    };
    uart_dma_init(DEBUG_UART_NUM, &dma_config);
#endif
}

AT(..log_ut.text.cache.L2)
static void ut_putchar(char a)
{
#if DEBUG_UART_DMA_EN

    debug_uart_buf[uart_buffer_index][pos] = a;
    pos++;
    if ((jiffies - tx_jiffies > 10) || (pos == MAX_DEBUG_FIFO)) {
        tx_jiffies = jiffies;
        uart_wait_tx_idle(DEBUG_UART_NUM, 10);
        uart_send_bytes(DEBUG_UART_NUM, debug_uart_buf[uart_buffer_index], pos);
        uart_buffer_index = !uart_buffer_index;
        pos = 0;
    }
#else
    /* if(a == '\n'){                          */
    /*     uart_putbyte(DEBUG_UART_NUM, '\r'); */
    /* }                                       */
    uart_putbyte(DEBUG_UART_NUM, a);

#endif
}

static char ut_getchar(void)
{
    return uart_getbyte(DEBUG_UART_NUM);
}
#endif







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
