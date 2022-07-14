#ifndef _UART_UPDATE_
#define _UART_UPDATE_
#include "typedef.h"
#include "uart_dev.h"

#define DMA_BUF_LEN			32

#ifdef CPU_SH54
#define TCFG_UART_UPDATE_PORT		IO_PORTA_09//SH54
#endif

#ifdef CPU_SH55
#define TCFG_UART_UPDATE_PORT		IO_PORTB_09//SH55
#endif

void uart_update_data_deal(u8 *data, u8 len);
void uart_update_data_init(const uart_bus_t *ut);

void uart1_tx_rx_switch(u8 port, u8 status);
void uart_update_init(void);

#endif
