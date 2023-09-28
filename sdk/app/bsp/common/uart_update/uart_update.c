#include "uart_update.h"
#include "app_config.h"
#include "app_modules.h"
#include "includes.h"
#include "asm/power_interface.h"
#include "asm/power/p33.h"
#include "cpu.h"
#include "uart_dev.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[uart_update]"
#include "log.h"

#if HAS_UART_UPDATE_EN

static u8 tmp_buf[DMA_BUF_LEN] = {0};
static u8 uart_buf[DMA_BUF_LEN] __attribute__((aligned(4)));
static const uart_bus_t *ut1 = NULL;
extern const uart_bus_t *uart_dev_open(const struct uart_platform_data_t *arg);

/*--------------------uart1设备模块--------------------*/
static void uart_update_isr_hook(void *arg, u32 status)
{
    const uart_bus_t *ubus = arg;

    if (status == UT_RX_OT) {
        u32 rx_len = JL_UT1->HRXCNT;
        u32 len = ubus->read(tmp_buf, rx_len, 0);
        if (len != 0) {
            /* log_info("uart_rx_ot len : %d", len); */
            /* log_info_hexdump(tmp_buf, len); */
            uart_update_data_deal(tmp_buf, len);
        }
    }
}

static int uart_dev_init(const uart_bus_t *ut)
{
    memset((void *)uart_buf, 0, sizeof(uart_buf));
    struct uart_platform_data_t arg;
    arg.tx_pin = TCFG_UART_UPDATE_PORT;
    arg.rx_pin = TCFG_UART_UPDATE_PORT;
    arg.rx_cbuf = uart_buf;
    arg.rx_cbuf_size = sizeof(uart_buf);
    arg.frame_length = sizeof(uart_buf);
    arg.rx_timeout = 2;
    arg.isr_cbfun = uart_update_isr_hook;
    arg.argv = JL_UT1;
    arg.is_9bit = 0;
    arg.baud = 9600;
    ut = uart_dev_open(&arg);
    if (NULL != ut) {
        return 0;
    } else {
        return -1;
    }
}

/*--------------------初始化接口--------------------*/
void uart_update_init(void)
{
    if (0 != uart_dev_init(ut1)) {	//默认使用uart1进行串口不掉电升级
        log_info("uart_update init fail!\n");
        return;
    }
    uart_update_data_init(ut1);
}

#else

void uart_update_init(void)
{

}

#endif
