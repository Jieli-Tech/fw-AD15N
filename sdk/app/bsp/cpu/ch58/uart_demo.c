#include "includes.h"
#include "uart_v2.h"
#include "malloc.h"

/* #define LOG_TAG_CONST   UART */
#define LOG_TAG         "[uart_demo]"
#include "log.h"

static void uart_irq_func(int uart_num, enum uart_event event)
{
    if (event & UART_EVENT_TX_DONE) {
        log_info("uart[%d] tx done", uart_num);
    }

    if (event & UART_EVENT_RX_DATA) {
        log_info("uart[%d] rx data", uart_num);
    }

    if (event & UART_EVENT_RX_FIFO_OVF) {
        log_info("uart[%d] rx fifo ovf", uart_num);
    }
}

u8 uart_rx_ptr[128];
u8 frame_ptr[64];

#define UART_DEMO_NUM 1

void uart_sync_demo_start(void)
{
    struct uart_config config = {
        .baud_rate = 1000000,
        .tx_pin = IO_PORTC_08,
        .rx_pin = IO_PORTC_09,
        .parity = UART_PARITY_DISABLE,
    };

    log_info("uart_rx_ptr:%d", uart_rx_ptr);
    log_info("frame_ptr:%d", frame_ptr);

    struct uart_dma_config dma = {
        .rx_timeout_thresh = 100,
        .frame_size = 32,
        .event_mask = UART_EVENT_TX_DONE | UART_EVENT_RX_DATA | UART_EVENT_RX_FIFO_OVF,
        .irq_callback = uart_irq_func,
        .rx_cbuffer = uart_rx_ptr,
        .rx_cbuffer_size = 128,
    };

    int r = uartx_init(UART_DEMO_NUM, &config);
    if (r < 0) {
        log_error("init error %d", r);
    }
    uart_dma_init(UART_DEMO_NUM, &dma);
}

void uart_sync_demo_run(void)
{
    int r;
    if (frame_ptr) {
        r = uart_recv_blocking(UART_DEMO_NUM, frame_ptr, 64, 10);
        r = uart_send_blocking(UART_DEMO_NUM, frame_ptr, r, 20);
        if (r) {
            printf_buf((u8 *)frame_ptr, r);
        }
    }
}

void uart_sync_demo_stop(void)
{
    uart_deinit(UART_DEMO_NUM);
}

