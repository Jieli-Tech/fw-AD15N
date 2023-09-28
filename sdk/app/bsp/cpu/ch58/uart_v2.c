#include "uart_v2.h"
/* #include "cpu/includes.h" */
#include "gpio.h"
#include "clock.h"
#include "circular_buf.h"

#define SPIN_LOCK_EN 0
#if SPIN_LOCK_EN
#include "spinlock.h"
#endif

#define UART_MALLOC_EN 0
#if UART_MALLOC_EN
#include "malloc.h"
#endif

#ifdef SUPPORT_MS_EXTENSIONS
#pragma data_seg(".uart.data")
#pragma bss_seg(".uart.data.bss")
#pragma code_seg(".uart.text")
#pragma const_seg(".uart.text.const")
#endif

/* #define LOG_TAG_CONST   UART */
#define LOG_TAG         "[UART]"
#include "log.h"

#define UART_CLK  clk_get("uart")
#define UART_OT_CLK  clk_get("uart")


struct uart_context {
    JL_UART_TypeDef *uart;
    cbuffer_t cbuf;
    volatile u32 tx_idle;

    struct uart_config uart_config;
    struct uart_flow_ctrl flow_ctrl;
    struct uart_dma_config dma;

#if SPIN_LOCK_EN
    spinlock_t lock;
#endif
    UT_mutex mutex;
    /* UT_Semaphore uart_tx; */
    UT_Semaphore uart_rx;
};


#if SPIN_LOCK_EN
#define UART_ENTER_CRITICAL() \
	spin_lock(&uart_context[uart_num]->lock)

#define UART_EXIT_CRITICAL() \
	spin_unlock(&uart_context[uart_num]->lock)
#else
#define UART_ENTER_CRITICAL()
#define UART_EXIT_CRITICAL()
#endif

#if UART_MALLOC_EN==0
static struct uart_context uart_context_buf[HW_UART_NUM];
#endif
static struct uart_context *uart_context[HW_UART_NUM];

static JL_UART_TypeDef *uart_table[HW_UART_NUM] = {
    JL_UART0,
    JL_UART1,
    JL_UART2,
};

static u8 uart_irq_idx[HW_UART_NUM] = {
    IRQ_UART0_IDX,
    IRQ_UART1_IDX,
    IRQ_UART2_IDX,
};

static u32 uart_tx_func_table[HW_UART_NUM] = {
    FO_UART0_TX,
    FO_UART1_TX,
    FO_UART2_TX,
};

static u32 uart_rx_func_table[HW_UART_NUM] = {
    PFI_UART0_RX,
    PFI_UART1_RX,
    PFI_UART2_RX,
};
static void uartx_flow_ctl_rts_suspend(uart_dev uart_num);
static void uartx_flow_ctl_rts_resume(uart_dev uart_num);
s32 uart_flow_ctrl_deinit(uart_dev uart_num);
static void uart0_isr();
static void uart1_isr();
static void uart2_isr();
typedef void (*irq_handler)(void);
static irq_handler uart_irq_handler[HW_UART_NUM] = {
    uart0_isr,
    uart1_isr,
    uart2_isr,
};

static u8 uart_is_single_wire(uart_dev uart_num)
{
    return uart_context[uart_num]->uart_config.tx_pin == uart_context[uart_num]->uart_config.rx_pin;
}

static void uart_set_dma_dir(uart_dev uart_num, u32 dir)
{
    if (uart_is_single_wire(uart_num)) {
        if (dir) {
            gpio_set_die(uart_context[uart_num]->uart_config.tx_pin, 1);
            gpio_set_direction(uart_context[uart_num]->uart_config.tx_pin, 1);
            gpio_set_pull_up(uart_context[uart_num]->uart_config.tx_pin, GPIO_PULLUP_10K);
            gpio_set_pull_down(uart_context[uart_num]->uart_config.tx_pin, GPIO_PULLDOWN_DISABLE);
            UART_ENTER_CRITICAL();
            uart_context[uart_num]->uart->CON1 &= ~BIT(4);
            UART_EXIT_CRITICAL();
        } else {
            UART_ENTER_CRITICAL();
            uart_context[uart_num]->uart->CON1 |= BIT(4);
            UART_EXIT_CRITICAL();
            gpio_write(uart_context[uart_num]->uart_config.tx_pin, 1);
            gpio_set_direction(uart_context[uart_num]->uart_config.tx_pin, 0);
            gpio_set_die(uart_context[uart_num]->uart_config.tx_pin, 0);
            gpio_set_pull_up(uart_context[uart_num]->uart_config.tx_pin, GPIO_PULLUP_DISABLE);
        }
    }
}

static void uart_fifo_reset(uart_dev uart_num)
{
    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;
    uart->CON1 |= UART_RX_DISABLE;
    uart->RXSADR = (u32)uart_context[uart_num]->dma.rx_cbuffer;
    uart->RXEADR = (u32)(uart_context[uart_num]->dma.rx_cbuffer + uart_context[uart_num]->dma.rx_cbuffer_size);
    cbuf_clear(&uart_context[uart_num]->cbuf);
    uart->CON1 &= ~(UART_RX_DISABLE);
}

void uart_dma_rx_reset(uart_dev uart_num)
{
    UT_OSMutexPend(&uart_context[uart_num]->mutex, 0);
    UART_ENTER_CRITICAL();
    uart_fifo_reset(uart_num);
    UT_OSSemSet(&uart_context[uart_num]->uart_rx, 0);
    UART_EXIT_CRITICAL();
    UT_OSMutexPost(&uart_context[uart_num]->mutex);
}

static void uart_isr(uart_dev uart_num)
{
    u32 rx_len;

    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;

    UART_ENTER_CRITICAL();
    u32 uart_con0 = uart->CON0 ;
    u32 uart_con1 = uart->CON1 ;
    u32 uart_con2 = uart->CON2 ;
    UART_EXIT_CRITICAL();

    if ((uart_con0 & UART_TX_IE) && (uart_con0 & UART_TX_PND)) {

        UART_ENTER_CRITICAL();
        uart->CON0 |= UART_CLR_TX_PND;
        uart_context[uart_num]->tx_idle = 1;
        UART_EXIT_CRITICAL();
        uart_set_dma_dir(uart_num, 1);

        /* int r = UT_OSSemPost(&uart_context[uart_num]->uart_tx); */
        if ((uart_context[uart_num]->dma.event_mask & UART_EVENT_TX_DONE) &&
            (uart_context[uart_num]->dma.irq_callback)) {
            uart_context[uart_num]->dma.irq_callback(uart_num, UART_EVENT_TX_DONE);
        }
    }

    u32 rx_data = 0;
    if ((uart_con0 & UART_OT_IE) && (uart_con0 & UART_OT_PND)) {
        rx_data = BIT(0);
    }

    if ((uart_con0 & UART_RX_IE) && (uart_con0 & UART_RX_PND)) {
        rx_data |= BIT(1);
    }

    if (rx_data) {

        UART_ENTER_CRITICAL();
        uart->CON0 |= UART_RX_FLUSH;
        if (rx_data & BIT(0)) {
            uart->CON0 |= UART_CLR_OT_PND;
        }
        uart->CON0 |= UART_CLR_RX_PND;
        asm volatile("csync;");
        while (!(uart->CON0 & UART_RX_FLUSH_OVER)) {
            __asm__ volatile("nop");
        }

        rx_len = uart->HRXCNT;

        UART_EXIT_CRITICAL();
        u32 fifo_ovf = !cbuf_is_write_able(&uart_context[uart_num]->cbuf, rx_len);
        if (fifo_ovf) {
            uart_fifo_reset(uart_num);
            UT_OSSemSet(&uart_context[uart_num]->uart_rx, 0);
        } else {
            cbuf_write_updata(&uart_context[uart_num]->cbuf, rx_len);
        }

        uartx_flow_ctl_rts_suspend(uart_num);

        if (rx_len) {

            int r = UT_OSSemPost(&uart_context[uart_num]->uart_rx);

            if (fifo_ovf) {
                if ((uart_context[uart_num]->dma.event_mask & UART_EVENT_RX_FIFO_OVF) &&
                    (uart_context[uart_num]->dma.irq_callback)) {
                    uart_context[uart_num]->dma.irq_callback(uart_num, UART_EVENT_RX_FIFO_OVF);
                }
            } else {
                if ((uart_context[uart_num]->dma.event_mask & UART_EVENT_RX_DATA) &&
                    (uart_context[uart_num]->dma.irq_callback)) {
                    uart_context[uart_num]->dma.irq_callback(uart_num, UART_EVENT_RX_DATA);
                }
            }
        }
    }
//校验中断 parity
    if ((uart_con2 & UART_CHK_IE) && (uart_con2 & UART_CHK_PND)) {
        UART_ENTER_CRITICAL();
        u32 rx_err_len = uart->RX_ERR_CNT;
        uart->CON2 |= UART_CLR_CHK_PND;             //清CHK PND
        UART_EXIT_CRITICAL();
        log_error("uart%d parity fail! RB8:%d,rx num:%d\n", uart_num, !!(uart_con2 & UART_RB8), rx_err_len);
        if ((uart_context[uart_num]->dma.event_mask & UART_EVENT_PARITY_ERR) &&
            (uart_context[uart_num]->dma.irq_callback)) {
            uart_context[uart_num]->dma.irq_callback(uart_num, UART_EVENT_PARITY_ERR);
        }
    }
    asm("csync");
}

___interrupt
static void uart0_isr()
{
    uart_isr(0);
}

___interrupt
static void uart1_isr()
{
    uart_isr(1);
}

___interrupt
static void uart2_isr()
{
    uart_isr(2);
}

//return 实际波特率,0:error; >0:ok
s32 uart_set_baudrate(uart_dev uart_num, u32 baud_rate)
{
    ASSERT(uart_num < HW_UART_NUM, "uart num %d < %d", uart_num, HW_UART_NUM);

    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;

    u32 b3 = ((UART_CLK + baud_rate / 2) / baud_rate) / 3 - 1;
    u32 b4 = ((UART_CLK + baud_rate / 2) / baud_rate) / 4 - 1;
    if ((b3 == 0xffffffff) && (b4 == 0xffffffff)) {
        return 0;//error
    }
    b3 &= 0xffff;
    b4 &= 0xffff;
    u32 b3_baud = UART_CLK / ((b3 + 1) * 3);
    u32 b4_baud = UART_CLK / ((b4 + 1) * 4);
    s32 ret = 0;

    uart->CON0 &= ~(UART_TX_EN | UART_RX_EN);
    uart->CON0 |= UART_CLR_ALL_PND;

    if ((__builtin_abs((int)(b3_baud - baud_rate))) > (__builtin_abs((int)(b4_baud - baud_rate)))) { //ABS_FUN
        uart->CON0 &= ~ BIT(4);
        uart->BAUD = b4;
        ret = b4_baud;
    } else {
        uart->CON0 |= BIT(4);
        uart->BAUD = b3;
        ret = b3_baud;
    }

    if (uart->CON0 & UART_RX_DMA) {
        uart->OTCNT = uart_context[uart_num]->dma.rx_timeout_thresh * (UART_OT_CLK / 1000000);
    }
    uart->CON0 |= (UART_TX_EN | UART_RX_EN);
    return ret;
}

s32 uart_set_rx_timeout_thresh(uart_dev uart_num, u32 timeout_thresh)
{
    ASSERT(uart_num < HW_UART_NUM, "uart num %d < %d", uart_num, HW_UART_NUM);

    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;

    uart->CON0 &= ~(UART_TX_EN | UART_RX_EN);
    uart->CON0 |= UART_CLR_ALL_PND;

    if (uart->CON0 & UART_RX_DMA) {
        uart_context[uart_num]->dma.rx_timeout_thresh = timeout_thresh;
        uart->OTCNT = uart_context[uart_num]->dma.rx_timeout_thresh * (UART_OT_CLK / 1000000);
    }
    uart->CON0 |= (UART_TX_EN | UART_RX_EN);
    return 0;
}

static u32 uart_is_idle(uart_dev ut_num)
{
    switch (ut_num) {
    case 0 :
        return !(JL_UART0->CON0 & UART_TX_EN);
    case 1 :
        return !(JL_UART1->CON0 & UART_TX_EN);
    case 2 :
        return !(JL_UART2->CON0 & UART_TX_EN);
    default :
        break;
    }
    return 0;
}
extern void *zalloc(size_t size);
s32 uartx_init(uart_dev _uart_num, const struct uart_config *config)
{
    /* ASSERT(uart_num < HW_UART_NUM, "uart num %d < %d", uart_num, HW_UART_NUM); */
    int uart_num = _uart_num;
    if (uart_num == -1) {
        for (int i = 0; i < HW_UART_NUM; i++) {
            if (uart_is_idle(i)) {
                uart_num = i;
                break;
            }
        }
    }
    if (uart_num == -1) {
        return -1;
    }

#if UART_MALLOC_EN
    uart_context[uart_num] = (struct uart_context *) zalloc(sizeof(struct uart_context));
    if (uart_context[uart_num] == NULL) {
        return -1;
    }
#else
    uart_context[uart_num] = &uart_context_buf[uart_num];
#endif

    memcpy(&uart_context[uart_num]->uart_config, config, sizeof(struct uart_config));

    uart_context[uart_num]->uart = uart_table[uart_num];
    memset(&uart_context[uart_num]->flow_ctrl, 0xff, sizeof(struct uart_flow_ctrl));
    memset(&uart_context[uart_num]->dma, 0, sizeof(struct uart_dma_config));

    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;

    uart->CON0 = UART_CLR_ALL_PND;
    uart->CON1 = 0;
    uart->CON2 = UART_CLR_CHK_PND;

    u32 tx_pin = config->tx_pin ;
    u32 rx_pin = config->rx_pin ;

    if (tx_pin != -1) {
        gpio_write(tx_pin, 1);
        gpio_set_direction(tx_pin, 0);
        gpio_set_fun_output_port(tx_pin, uart_tx_func_table[uart_num], 1, 1);
    }

    if (rx_pin != -1) {
        gpio_set_die(rx_pin, 1);
        gpio_set_direction(rx_pin, 1);
        gpio_set_pull_up(rx_pin, GPIO_PULLUP_10K);
        gpio_set_pull_down(rx_pin, GPIO_PULLDOWN_DISABLE);
        gpio_set_fun_input_port(rx_pin, uart_rx_func_table[uart_num]);
    }

    if (rx_pin == tx_pin) {
        uart_set_dma_dir(uart_num, 1);
    } else {
    }

    if (config->parity != UART_PARITY_DISABLE) {
        uart->CON2 |= ((config->parity - 1) << 4);
        uart->CON2 |= UART_CHK_IE;//dma
        uart->CON2 |= UART_CHK_EN;
        uart->CON2 |= UART_M9_EN;
        uart->CON2 |= UART_CLR_CHK_PND;
    }

    uart_context[uart_num]->tx_idle = 1;
    uart_set_baudrate(uart_num, uart_context[uart_num]->uart_config.baud_rate);

    int r = UT_OSMutexCreate(&uart_context[uart_num]->mutex);
    if (r) {
        goto __exit_error;
    }
    /* r = UT_OSSemCreate(&uart_context[uart_num]->uart_tx, 1); */
    /* if (r) { */
    /*     goto __exit_error; */
    /* } */
    r = UT_OSSemCreate(&uart_context[uart_num]->uart_rx, 0);
    if (r) {
        goto __exit_error;
    }

    /* printf("uart_v2:%d open ok!\n", uart_num); */
    return uart_num;
__exit_error:
#if UART_MALLOC_EN
    free(uart_context[uart_num]);
#else
#endif
    uart_context[uart_num] = NULL;
    return -1;
}

s32 uart_deinit(uart_dev uart_num)
{
    ASSERT(uart_num < HW_UART_NUM, "uart num %d < %d", uart_num, HW_UART_NUM);

    if (uart_context[uart_num] == NULL) {
        return 0;
    }

    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;

    unrequest_irq(uart_irq_idx[uart_num]);
    uart->CON0 = UART_CLR_ALL_PND;
    uart->CON1 = 0;
    uart->CON2 = 0;
    uart_flow_ctrl_deinit(uart_num);

    //关闭io
    u32 tx_pin = uart_context[uart_num]->uart_config.tx_pin ;
    u32 rx_pin = uart_context[uart_num]->uart_config.rx_pin;
    if (tx_pin != -1) {
        gpio_disable_fun_output_port(tx_pin);
    }
    if (rx_pin != -1) {
        gpio_disable_fun_input_port(uart_rx_func_table[uart_num]);
    }

    UT_OSMutexClose(&uart_context[uart_num]->mutex, 1);
    /* UT_OSSemClose(&uart_context[uart_num]->uart_tx, 1); */
    UT_OSSemClose(&uart_context[uart_num]->uart_rx, 1);

#if UART_MALLOC_EN
    free(uart_context[uart_num]);
#else
#endif
    uart_context[uart_num] = NULL;
    return 0;
}

#if defined(UART_TX_INV)
void uart_tx_data_inv(uart_dev uart_num, u8 tx_inv_en)
{
    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;
    UART_ENTER_CRITICAL();
    if (tx_inv_en) {
        uart->CON1 |= UART_TX_INV;
    } else {
        uart->CON1 &= ~UART_TX_INV;
    }
    UART_EXIT_CRITICAL();
}
#endif

#if defined(UART_RX_INV)
void uart_rx_data_inv(uart_dev uart_num, u8 rx_inv_en)
{
    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;
    UART_ENTER_CRITICAL();
    if (rx_inv_en) {
        uart->CON1 |= UART_RX_INV;
    } else {
        uart->CON1 &= ~UART_RX_INV;
    }
    UART_EXIT_CRITICAL();
}
#endif

s32 uart_dma_init(uart_dev uart_num, const struct uart_dma_config *dma_config)
{
    ASSERT(uart_num < HW_UART_NUM, "uart num %d < %d", uart_num, HW_UART_NUM);

    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;
    memcpy(&uart_context[uart_num]->dma, dma_config, sizeof(struct uart_dma_config));



    if (dma_config->rx_cbuffer) {
        /* if (memory_in_phy(dma_config->rx_cbuffer) == 0) { */
        /*     log_error("uart dma buffer must used dma_malloc()"); */
        /*     return -1; */
        /* } */

        if (dma_config->rx_cbuffer_size % 4) {
            log_error("uart rx buffer size align 4");
            return -1;
        }

        if ((u32)dma_config->rx_cbuffer % 4) {
            log_error("uart rx buffer addr align 4");
            return -1;
        }
        cbuf_init(&uart_context[uart_num]->cbuf, dma_config->rx_cbuffer, dma_config->rx_cbuffer_size);
        uart->RXSADR = (u32)dma_config->rx_cbuffer;
        uart->RXEADR = (u32)(dma_config->rx_cbuffer + dma_config->rx_cbuffer_size);
        uart->RXCNT = dma_config->frame_size;
        uart->CON0 |= UART_RX_IE | UART_OT_IE | UART_RX_DMA ;
    }

    uart->CON0 |= UART_TX_IE;
    request_irq(uart_irq_idx[uart_num], 3, uart_irq_handler[uart_num], 0);
    uart_set_baudrate(uart_num, uart_context[uart_num]->uart_config.baud_rate);

    return 0;
}
s32 uart_putbyte(uart_dev uart_num, u32 a)
{
    if (uart_context[uart_num] == NULL) {
        return 0;
    }
    /* UT_OSMutexPend(&uart_context[uart_num]->mutex, 0); */
    u32 i = 0x10000;
    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;

    if (uart_is_idle(uart_num)) {
        return 0;
    }
    uart_set_dma_dir(uart_num, 0);
    if (uart_is_single_wire(uart_num) == 0) {
        while (((uart->CON0 & UART_TX_PND) == 0) && (0 != i)) {  //TX IDLE
            i--;
        }
    }
    /* UART_ENTER_CRITICAL(); */
    uart->CON0 |= UART_CLR_TX_PND;  //清Tx pending
    uart->BUF = a;
    if (uart_is_single_wire(uart_num)) {
        while (((uart->CON0 & UART_TX_PND) == 0) && (0 != i)) {  //TX IDLE
            i--;
        }
        uart->CON0 |= UART_CLR_TX_PND;  //清Tx pending
        uart_set_dma_dir(uart_num, 1);
    }
    /* UART_EXIT_CRITICAL(); */
    /* UT_OSMutexPost(&uart_context[uart_num]->mutex); */
    return a;
}

// timeout_us = 0 就一直等
s32 uart_wait_tx_idle(uart_dev uart_num, u32 timeout_ms)
{
    if (uart_context[uart_num] == NULL) {
        return 0;
    }
    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;

    u32 tx_ie = uart->CON0 & UART_TX_IE;
    timeout_ms = ut_msecs_to_jiffies(timeout_ms);
    u32 _timeout = timeout_ms + ut_get_jiffies();
    /* u32 curr_msec = jiffies_msec(); */
    /* u32 end_msec = jiffies_offset2msec(curr_msec, timeout_ms); */

    /* while (cpu_in_irq() || cpu_irq_disabled() || (tx_ie == 0)) { */
    while (1) {
        if (uart_context[uart_num]->tx_idle) {
            uart_set_dma_dir(uart_num, 1);
            return 0;
        }

        if (uart->CON0 & UART_TX_PND) {
            if (tx_ie) {
                uart->CON0 |= UART_CLR_TX_PND;
            }
            uart_context[uart_num]->tx_idle = 1;
        }

        if ((timeout_ms != 0) && time_before(_timeout, ut_get_jiffies())) {
            return -1;
        }
        wdt_clear();
        /* curr_msec = jiffies_msec(); */
        /* int offset = jiffies_msec2offset(curr_msec, end_msec); */
        /* if (offset < 0) { */
        /*     return 0; */
        /* } */
    }
    /* int timeout_tick = msecs_to_jiffies(timeout_ms); */
    /* int r = UT_OSSemPend(&uart_context[uart_num]->uart_tx, timeout_tick); */
    /* return r; */
}
static s32 uart_dma_tx(uart_dev uart_num, const void *src, s32 size)
{
    if (uart_context[uart_num] == NULL) {
        return -1;
    }
    /* if (memory_in_phy(src) == 0) { */
    /*     log_error("uart dma buffer must used dma_malloc()"); */
    /*     return -1; */
    /* } */

    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;
    /* UT_OSSemSet(&uart_context[uart_num]->uart_tx, 0); */
    uart_set_dma_dir(uart_num, 0);

    UART_ENTER_CRITICAL();
    uart_context[uart_num]->tx_idle = 0;
    uart->CON0 |= UART_CLR_TX_PND;
    UART_EXIT_CRITICAL();
    asm("csync");
    uart->TXADR = (u32)src;
    uart->TXCNT = size;

    return size;
}
s32 uart_send_bytes(uart_dev uart_num, const void *src, u32 size)
{
    if (size == 0) {
        return 0;
    }
    ASSERT(uart_num < HW_UART_NUM, "uart num %d < %d", uart_num, HW_UART_NUM);

    if (uart_context[uart_num] == NULL) {
        return 0;
    }

    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;

    int os_err = UT_OSMutexPend(&uart_context[uart_num]->mutex, 0);

    size = uart_dma_tx(uart_num, src, size);

    os_err = UT_OSMutexPost(&uart_context[uart_num]->mutex);
    return size;
}


//阻塞式dma发送数据
s32 uart_send_blocking(uart_dev uart_num, const void *src, u32 size, u32 timeout_ms)
{
    if (size == 0) {
        return 0;
    }
    ASSERT(uart_num < HW_UART_NUM, "uart num %d < %d", uart_num, HW_UART_NUM);

    if (uart_context[uart_num] == NULL) {
        return 0;
    }

    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;

    int os_err = UT_OSMutexPend(&uart_context[uart_num]->mutex, 0);


    size = uart_dma_tx(uart_num, src, size);
    uart_wait_tx_idle(uart_num, timeout_ms);

    os_err = UT_OSMutexPost(&uart_context[uart_num]->mutex);
    return size;
}

s32 uart_getbyte(uart_dev uart_num)
{
    UT_OSMutexPend(&uart_context[uart_num]->mutex, 0);
    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;
    uart_set_dma_dir(uart_num, 1);
    while (!(uart->CON0 & UART_RX_PND)) {
    }
    int byte = uart->BUF;
    UART_ENTER_CRITICAL();
    uart->CON0 |= UART_CLR_RX_PND;
    UART_EXIT_CRITICAL();
    __asm__ volatile("csync");
    UT_OSMutexPost(&uart_context[uart_num]->mutex);
    return byte;
}

s32 uart_recv_bytes(uart_dev uart_num, void *buf, u32 len)
{
    int os_err = UT_OSMutexPend(&uart_context[uart_num]->mutex, 0);

    u32 cbuf_cur_len = uart_context[uart_num]->cbuf.data_len;//cbuf_get_data_size(&uart_context[uart_num]->cbuf);
    if (cbuf_cur_len) {
        len = MIN(len, cbuf_cur_len);
        len = cbuf_read(&uart_context[uart_num]->cbuf, buf, len);
    } else {
        len = 0;
    }

    uartx_flow_ctl_rts_resume(uart_num);

    os_err = UT_OSMutexPost(&uart_context[uart_num]->mutex);
    return len;
}

s32 uart_recv_blocking(uart_dev uart_num, void *buf, u32 len, u32 timeout_ms)
{
    /* int os_err = UT_OSMutexPend (&uart_context[uart_num]->mutex, 0); */
    u32 timeout_tick = timeout_ms / 2; //msecs_to_jiffies(timeout_ms);

    if (timeout_tick == 0) {
        timeout_tick = -1;
    }

    u32 offset = 0;
    while ((offset < len) && timeout_tick) {
        int r = uart_recv_bytes(uart_num, buf, len - offset);
        buf += r;
        offset += r;
        r  = UT_OSSemPend(&uart_context[uart_num]->uart_rx, 1);
        timeout_tick --;
    }
    /* os_err = UT_OSMutexPost(&uart_context[uart_num]->mutex); */
    return offset;
}


static u32 uart_rts_func_table[HW_UART_NUM] = {
#ifdef CONFIG_CPU_BR27
    PORT_FUNC_UART0_RTS,
    PORT_FUNC_UART1_RTS,
    PORT_FUNC_UART2_RTS,
#else
    (u32) - 1,
    FO_UART2_RTS,
    (u32) - 1,
#endif
};
static u32 uart_cts_func_table[HW_UART_NUM] = {
#ifdef CONFIG_CPU_BR27
    PORT_FUNC_UART0_CTS,
    PORT_FUNC_UART1_CTS,
    PORT_FUNC_UART2_CTS,
#else
    (u32) - 1,
    PFI_UART2_CTS,
    (u32) - 1,
#endif
};
#define     UART_FLOW_CTRL_V1 1 //br27,br28,br36
#define     UART_FLOW_CTRL_V2 2 //br50
#if (defined CONFIG_CPU_BR50 || defined CONFIG_CPU_CD09)
#define     UART_FLOW_CTRL_V UART_FLOW_CTRL_V2
#else
#define     UART_FLOW_CTRL_V UART_FLOW_CTRL_V1
#endif

#if (UART_FLOW_CTRL_V == UART_FLOW_CTRL_V1)
//UART CON1
#define     UART_CTS_PND          (1<<15)
#define     UART_CLR_CTS_PND      (1<<14)
#define     UART_CLR_RTS_PND      (1<<13)
#define     UART_CTS_INV          (1<<14)
#define     UART_RTS_INV          (1<<13)
/* #define     UART_RX_DISABLE       (1<<4) */
#define     UART_CTS_IE           (1<<3)
#define     UART_CTS_EN           (1<<2)
#define     UART_RTS_DMAEN        (1<<1)
#define     UART_RTS_EN           (1<<0)
#define     UART_FLOW_INIT        (UART_CLR_CTS_PND | UART_CLR_RTS_PND)
#elif (UART_FLOW_CTRL_V == UART_FLOW_CTRL_V2)
//UART CON1
#define     UART_CTS_PND          (1<<15)
#define     UART_RTS_PND          (1<<14)
#define     UART_CLR_CTS_PND      (1<<13)
#define     UART_CLR_RTS_PND      (1<<12)
#define     UART_CTS_INV          (1<<7)//cts有效电平
#define     UART_RTS_INV          (1<<6)//rts有效电平
#define     UART_RX_BYPASS        (1<<5)//直通 or 滤波
/* #define     UART_RX_DISABLE       (1<<4) */
#define     UART_CTS_IE           (1<<3)
#define     UART_CTS_EN           (1<<2)
#define     UART_RTS_EN           (1<<0)
#define     UART_FLOW_INIT        (UART_CLR_CTS_PND | UART_CLR_RTS_PND | UART_RX_BYPASS)
#endif
s32 uart_flow_ctrl_init(uart_dev uart_num, const struct uart_flow_ctrl *flow_ctrl)
{
    if (flow_ctrl == NULL) {
        return -1;
    }

#ifdef CONFIG_CPU_BR27
    ASSERT(uart_num < HW_UART_NUM, "uart num %d < %d", uart_num, HW_UART_NUM);
#else
#if defined CONFIG_CPU_BR50
    ASSERT(uart_num == 1, "uart num %d no rts-cts", uart_num);
#endif
#if defined CONFIG_CPU_CD09
    ASSERT(uart_num == 2, "uart num %d no rts-cts", uart_num);
#endif
#endif

    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;
    memcpy(&uart_context[uart_num]->flow_ctrl, flow_ctrl, sizeof(struct uart_flow_ctrl));

    uart->CON1 = UART_FLOW_INIT;

    u32 rts_io = flow_ctrl->rts_pin;
    u32 cts_io = flow_ctrl->cts_pin;
    //RTS
    if (rts_io < IO_PORT_MAX) {
#if 0//硬件rts
        if (flow_ctrl->rts_idle_level) {
            uart->CON1 &= ~ UART_RTS_INV;
#if (UART_FLOW_CTRL_V == UART_FLOW_CTRL_V2)
            gpio_write(rts_io, 1);
            gpio_set_direction(rts_io, 0);
#else
            gpio_write(rts_io, 0);
            gpio_set_direction(rts_io, 0);
#endif
        } else {
            uart->CON1 |= UART_RTS_INV;
            gpio_write(rts_io, 0);
            gpio_set_direction(rts_io, 0);
        }

        gpio_set_fun_output_port(rts_io, uart_rts_func_table[uart_num], 1, 1);
        uart->CON1 |= (UART_CLR_RTS_PND | UART_RTS_EN);
#else //软件rts

        if (flow_ctrl->rts_idle_level) {
            gpio_write(rts_io, 1);
            gpio_set_direction(rts_io, 0);
        } else {
            gpio_write(rts_io, 0);
            gpio_set_direction(rts_io, 0);
        }
#endif
    }
    //CTS
    if (cts_io < IO_PORT_MAX) {
        if (flow_ctrl->cts_idle_level) {
#if (UART_FLOW_CTRL_V == UART_FLOW_CTRL_V2)
            gpio_set_die(cts_io, 1);
            gpio_set_direction(cts_io, 1);
            gpio_set_pull_up(cts_io, GPIO_PULLUP_10K);
            gpio_set_pull_down(cts_io, GPIO_PULLDOWN_DISABLE);
#else
            gpio_set_die(cts_io, 1);
            gpio_set_direction(cts_io, 1);
            gpio_set_pull_up(cts_io, GPIO_PULLUP_DISABLE);
            gpio_set_pull_down(cts_io, GPIO_PULLDOWN_10K);
#endif
            uart->CON1 &= ~ UART_CTS_INV;
        } else {
            gpio_set_die(cts_io, 1);
            gpio_set_direction(cts_io, 1);
            gpio_set_pull_up(cts_io, GPIO_PULLUP_DISABLE);
            gpio_set_pull_down(cts_io, GPIO_PULLDOWN_10K);
            uart->CON1 |= UART_CTS_INV;
        }
        gpio_set_fun_input_port(cts_io, uart_cts_func_table[uart_num]);

        /* request_irq(uart_irq_idx[uart_num], 3, uart_irq_handler[uart_num], 0); */
        /* uart->CON1 |= UART_CTS_IE;//CTS 中断 en */
        uart->CON1 |= (UART_CLR_CTS_PND | UART_CTS_EN);
    }
    log_info("cts rts con1:0x%x", uart->CON1);

    return 0;//ok
}

s32 uart_flow_ctrl_deinit(uart_dev uart_num)
{
    ASSERT(uart_num < HW_UART_NUM, "uart num %d < %d", uart_num, HW_UART_NUM);

    if (((uart_context[uart_num]->flow_ctrl.rts_pin) == 0xffff) &&
        ((uart_context[uart_num]->flow_ctrl.cts_pin) == 0xffff)) {
        return -1;
    }

    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;
    memset(&uart_context[uart_num]->flow_ctrl, 0, sizeof(struct uart_flow_ctrl));


    uart->CON1 &= ~(UART_CTS_IE | UART_CTS_EN | UART_RTS_EN);
    uart->CON1 |= (UART_CLR_CTS_PND | UART_CLR_RTS_PND);
    return 0;//ok
}

static void uartx_flow_ctl_rts_suspend(uart_dev uart_num)
{
    ASSERT(uart_num < HW_UART_NUM, "uart num %d < %d", uart_num, HW_UART_NUM);

    if ((uart_context[uart_num]->flow_ctrl.rts_pin) == 0xffff) {
        return;
    }
    u32 cbuf_cur_len = uart_context[uart_num]->cbuf.data_len;//cbuf_get_data_size(&uart_context[uart_num]->cbuf);
    if (cbuf_cur_len < uart_context[uart_num]->flow_ctrl.rx_thresh * uart_context[uart_num]->dma.rx_cbuffer_size) {
        return;
    }
    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;
    if (!(uart->CON1 & UART_RTS_EN)) {
        gpio_write(uart_context[uart_num]->flow_ctrl.rts_pin, !(uart_context[uart_num]->flow_ctrl.rts_idle_level));
    }
    UART_ENTER_CRITICAL();
    uart->CON1 |= UART_RX_DISABLE;
    UART_EXIT_CRITICAL();
}

static void uartx_flow_ctl_rts_resume(uart_dev uart_num)
{
    ASSERT(uart_num < HW_UART_NUM, "uart num %d < %d", uart_num, HW_UART_NUM);

    if ((uart_context[uart_num]->flow_ctrl.rts_pin) == 0xffff) {
        return;
    }
    u32 cbuf_cur_len = uart_context[uart_num]->cbuf.data_len;//cbuf_get_data_size(&uart_context[uart_num]->cbuf);
    if (cbuf_cur_len > uart_context[uart_num]->flow_ctrl.rx_thresh * uart_context[uart_num]->dma.rx_cbuffer_size) {
        return;
    }

    JL_UART_TypeDef *uart = uart_context[uart_num]->uart;
    UART_ENTER_CRITICAL();
    uart->CON1 &= ~UART_RX_DISABLE;
    uart->CON1 |= UART_CLR_RTS_PND;
    UART_EXIT_CRITICAL();
    if (uart->CON1 & UART_RTS_EN) {
    } else {
        gpio_write(uart_context[uart_num]->flow_ctrl.rts_pin, uart_context[uart_num]->flow_ctrl.rts_idle_level);
    }
}



/*-----------------------------------------------------------*/
#include "asm/power_interface.h"

static JL_UART_TypeDef UART0_POWEROFF;

static u8 uart_enter_deepsleep(void)
{
    UART0_POWEROFF.CON0 =  JL_UART0->CON0;
    UART0_POWEROFF.CON1 =  JL_UART0->CON1;
    UART0_POWEROFF.BAUD =  JL_UART0->BAUD;
    UART0_POWEROFF.OTCNT =  JL_UART0->OTCNT;
    UART0_POWEROFF.TXADR =  JL_UART0->TXADR;
    UART0_POWEROFF.RXSADR =  JL_UART0->RXSADR;
    UART0_POWEROFF.RXEADR =  JL_UART0->RXEADR;
    UART0_POWEROFF.CON2 =  JL_UART0->CON2;

    return 0;
}

static u8 uart_exit_deepsleep(void)
{
    JL_UART0->CON1	 =  UART0_POWEROFF.CON1;
    JL_UART0->BAUD	 =  UART0_POWEROFF.BAUD;
    JL_UART0->OTCNT	 =  UART0_POWEROFF.OTCNT;
    JL_UART0->TXADR	 =  UART0_POWEROFF.TXADR;
    JL_UART0->RXSADR =	UART0_POWEROFF.RXSADR;
    JL_UART0->RXEADR =	UART0_POWEROFF.RXEADR;
    JL_UART0->CON2	 =  UART0_POWEROFF.CON2;

    JL_UART0->CON0   =  UART0_POWEROFF.CON0;
    //JL_UART0->CON0 |= UART_CLR_ALL_PND;

    return 0;
}

/* DEEPSLEEP_TARGET_REGISTER(uart) = { */
/*     .name   = "uart", */
/*     .enter  = uart_enter_deepsleep, */
/*     .exit   = uart_exit_deepsleep, */
/* }; */

/*-----------------------------------------------------------*/

