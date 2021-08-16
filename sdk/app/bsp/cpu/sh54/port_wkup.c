#include "asm/power/p33.h"
#include "hwi.h"
#include "gpio.h"
#include "port_wkup.h"
/* #include "typedef.h" */

/* #define LOG_TAG_CONST       WKUP */
#define LOG_TAG             "[PORT-WKUP]"
#include "debug.h"

/*JL_WAKEUP->CON0:
 *                0：ich2 IO-wkup
 *                1：ich1 IRFLT-wkup
 *                2：uart0_rxas
 *                3：uart1_rxas
 *                4：sd0_dat_i
 * */
//sh54:
//uart0_rxas: IO_PORTA_05, IO_PORTA_01,ich2
//uart1_rxas: IO_PORT_DP , IO_PORTA_09,ich0
//sd0_dat_i: IO_PORTA_03, IO_PORTA_10, IO_PORT_DM , IO_PORTA_06,

static void (*port_wkup_irq_cbfun)(void) = NULL;
static void (*irflt_wkup_irq_cbfun)(void) = NULL;
static void (*uart0_rx_wkup_irq_cbfun)(void) = NULL;
static void (*uart1_rx_wkup_irq_cbfun)(void) = NULL;
static void (*sd0_dat_wkup_irq_cbfun)(void) = NULL;
#define PORT_WKUP_IRQ_PRIORITY  3//中断优先级，范围:0~7(低~高)
__attribute__((interrupt("")))
static void port_wkup_isr()
{
    if ((JL_WAKEUP->CON0 & BIT(0)) && (JL_WAKEUP->CON3 & BIT(0))) {
        JL_WAKEUP->CON2 |= BIT(0);
        log_info("port %d wakeup", (JL_IOMC->IOMC2 >> 10) & 0x1f);
        if (port_wkup_irq_cbfun) {
            port_wkup_irq_cbfun();
        }
    }
    if ((JL_WAKEUP->CON0 & BIT(1)) && (JL_WAKEUP->CON3 & BIT(1))) {
        JL_WAKEUP->CON2 |= BIT(1);
        log_info("IRFLT %d wakeup", (JL_IOMC->IOMC2 >> 5) & 0x1f);
        if (irflt_wkup_irq_cbfun) {
            irflt_wkup_irq_cbfun();
        }
    }
    if ((JL_WAKEUP->CON0 & BIT(2)) && (JL_WAKEUP->CON3 & BIT(2))) {
        JL_WAKEUP->CON2 |= BIT(2);
        log_info("uart0_rx wakeup");
        if (uart0_rx_wkup_irq_cbfun) {
            uart0_rx_wkup_irq_cbfun();
        }
    }
    if ((JL_WAKEUP->CON0 & BIT(3)) && (JL_WAKEUP->CON3 & BIT(3))) {
        JL_WAKEUP->CON2 |= BIT(3);
        log_info("uart1_rx wakeup");
        if (uart1_rx_wkup_irq_cbfun) {
            uart1_rx_wkup_irq_cbfun();
        }
    }
    if ((JL_WAKEUP->CON0 & BIT(4)) && (JL_WAKEUP->CON3 & BIT(4))) {
        JL_WAKEUP->CON2 |= BIT(4);
        log_info("sd0_dat_i wakeup");
        if (sd0_dat_wkup_irq_cbfun) {
            sd0_dat_wkup_irq_cbfun();
        }
    }
}

/*
 * @brief irflt 普通IO中断[唤醒/外部中断](与红外冲突)
 * @parm port 端口 ich1
 * @parm edge 检测边缘，1 下降沿，0 上升沿
 * @parm cbfun 中断回调函数
 * @return 0 成功，< 0 失败
 */
int irflt_wkup_enable(u8 port, u8 edge, void (*cbfun)(void))
{
    if (JL_WAKEUP->CON0 & BIT(1)) {
        log_error("IRFLT WKUP has been used\n");
        return -1;
    }
    if (port < IO_PORT_MAX) {
        gpio_set_direction(port, 1);
        gpio_set_die(port, 1);
        JL_IOMC->IOMC2 &= ~(0x1ful << 5);  //wakeup0 input select
        if (port >= IO_PORTD_00) {
            port = port - IO_PORTD_00 + 18;
        }
        JL_IOMC->IOMC2 |= (port << 5);
    } else {
        log_error("unsupported port\n");
        return -1;
    }
    if (cbfun) {
        irflt_wkup_irq_cbfun = cbfun;
    }
    HWI_Install(IRQ_PORT_IDX, (u32)port_wkup_isr, PORT_WKUP_IRQ_PRIORITY);//中断优先级3
    JL_WAKEUP->CON2 |= BIT(1);  //clear pending
    if (edge) {
        JL_WAKEUP->CON1 |= BIT(1);  //detect falling edge
        gpio_set_pull_up(port, 1);
        gpio_set_pull_down(port, 0);
    } else {
        JL_WAKEUP->CON1 &= ~BIT(1);  //detect rising edge
        gpio_set_pull_up(port, 0);
        gpio_set_pull_down(port, 1);
    }
    JL_WAKEUP->CON0 |= BIT(1);  //wakeup enable
    return 0;
}
/*
 * @brief 失能IO口[唤醒/外部中断]
 * @parm port 端口
 * @return null
 */
void irflt_wkup_disable(u8 port)
{
    u8 temp;
    temp = (JL_IOMC->IOMC2 >> 5) & 0x1f;
    if (temp >= 18) {
        temp = temp - 18 + IO_PORTD_00;
    }
    if ((port < IO_PORT_MAX) && (temp == port)) {
        if (JL_WAKEUP->CON0 & BIT(1)) {
            JL_WAKEUP->CON0 &= ~BIT(1);
            JL_WAKEUP->CON2 |= BIT(1);
            if (irflt_wkup_irq_cbfun) {
                irflt_wkup_irq_cbfun = NULL;
            }
            gpio_set_die(port, 0);
        }
    }
}
/*
 * @brief 使能IO口[唤醒/外部中断]
 * @parm port 端口 ich2
 * @parm edge 检测边缘，1 下降沿，0 上升沿
 * @parm cbfun 中断回调函数
 * @return 0 成功，< 0 失败
 */
int port_wkup_enable(u8 port, u8 edge, void (*cbfun)(void))
{
    if (JL_WAKEUP->CON0 & BIT(0)) {
        log_error("PORT WKUP0 has been used\n");
        return -1;
    }
    if (port < IO_PORT_MAX) {
        gpio_set_direction(port, 1);
        gpio_set_die(port, 1);
        JL_IOMC->IOMC2 &= ~(0x1ful << 10);  //wakeup0 input select
        if (port >= IO_PORTD_00) {
            port = port - IO_PORTD_00 + 18;
        }
        JL_IOMC->IOMC2 |= (port << 10);
    } else {
        log_error("unsupported port\n");
        return -1;
    }
    if (cbfun) {
        port_wkup_irq_cbfun = cbfun;
    }
    HWI_Install(IRQ_PORT_IDX, (u32)port_wkup_isr, PORT_WKUP_IRQ_PRIORITY);//中断优先级3
    JL_WAKEUP->CON2 |= BIT(0);  //clear pending
    if (edge) {
        JL_WAKEUP->CON1 |= BIT(0);  //detect falling edge
        gpio_set_pull_up(port, 1);
        gpio_set_pull_down(port, 0);
    } else {
        JL_WAKEUP->CON1 &= ~BIT(0);  //detect rising edge
        gpio_set_pull_up(port, 0);
        gpio_set_pull_down(port, 1);
    }
    JL_WAKEUP->CON0 |= BIT(0);  //wakeup enable
    return 0;
}

/*
 * @brief 失能IO口[唤醒/外部中断]
 * @parm port 端口
 * @return null
 */
void port_wkup_disable(u8 port)
{
    u8 temp;
    temp = (JL_IOMC->IOMC2 >> 10) & 0x1f;
    if (temp >= 18) {
        temp = temp - 18 + IO_PORTD_00;
    }
    if ((port < IO_PORT_MAX) && (temp == port)) {
        if (JL_WAKEUP->CON0 & BIT(0)) {
            JL_WAKEUP->CON0 &= ~BIT(0);
            JL_WAKEUP->CON2 |= BIT(0);
            if (port_wkup_irq_cbfun) {
                port_wkup_irq_cbfun = NULL;
            }
            gpio_set_die(port, 0);
        }
    }
}
/*
 * @brief uart rx做普通IO中断[外部中断](会与uart rx功能冲突)
 * @parm uart_dev :uart0 or uart1
 * @parm port port io端口
 *      uartx_wkup_enable与uartx_init需选择同一组port，否则出错
 *      port参数即为uartx_init中rx_pin(如果uartx已打开只有选择相
 *      应rx才会成功初始化wkup,若uartx未开，则根据port完成初始化)
 *       sh54:
 *            uart0_rx: IO_PORTA_05, IO_PORTA_01,ich2(与IO_WKUP共用)
 *            uart1_rx: IO_PORT_DP, IO_PORTA_09,ich0(与timer012 capin共用)*
 *            uart_rx与sd0的CMD/CLK也不能重复
 * @parm edge 检测边缘，1 下降沿，0 上升沿
 * @parm cbfun 中断回调函数
 * @return 0 成功，< 0 失败
 */
int uartx_wkup_enable(u8 uart_dev, u8 port, u8 edge, void (*cbfun)(void))
{
    u8 uart_en = uart_dev + 2;
    u8 uart_io_port = (JL_IOMC->IOMC0 >> (uart_dev * 3 + 5)) & (0x03);
    u8 uart_ich_en = 0, uart_ich_bit = 0;
    if (uart_dev > 1) {
        log_error("There is no uart_dev:%d!\n", uart_dev);
        return -1;
    }
    if (JL_WAKEUP->CON0 & BIT(uart_en)) {
        log_error("UART%d RX WKUP has been used\n", uart_dev);
        return -1;
    }
    if (port < IO_PORT_MAX) {
        switch (uart_io_port + uart_dev * 4) {
        case 0:
            if (JL_UT0->CON & 0x0001) {
                if (port != IO_PORTA_05) {
                    log_error("uart0 rx wkup port sel error\n");
                    return -1;
                }
                break;
            } else {
                if (port == IO_PORTA_05) {
                    SFR(JL_IOMC->IOMC0, 4, 3, 1);
                    break;
                }
            }
        case 1:
            if (JL_UT0->CON & 0x0001) {
                if (port != IO_PORTA_01) {
                    log_error("uart0 rx wkup port sel error\n");
                    return -1;
                }
                break;
            } else {
                if (port == IO_PORTA_01) {
                    SFR(JL_IOMC->IOMC0, 4, 3, 3);
                    break;
                }
            }
        case 2:
            uart_ich_en = 1;
            uart_ich_bit = 10;
            SFR(JL_IOMC->IOMC0, 5, 2, 2);
            break;
        case 3:
            uart_ich_en = 1;
            uart_ich_bit = 10;
            break;
        case 4:
            if ((JL_UT1->CON0 & 0x0003) != 0) {
                if (port != IO_PORT_DP) {
                    log_error("uart1 rx wkup port sel error\n");
                    return -1;
                }
                break;
            } else {
                if (port == IO_PORT_DP) {
                    SFR(JL_IOMC->IOMC0, 7, 3, 1);
                    break;
                }
            }
        case 5:
            if ((JL_UT1->CON0 & 0x0003) != 0) {
                if (port != IO_PORTA_09) {
                    log_error("uart1 rx wkup port sel error\n");
                    return -1;
                }
                break;
            } else {
                if (port == IO_PORTA_09) {
                    SFR(JL_IOMC->IOMC0, 7, 3, 3);
                    break;
                }
            }
        case 6:
            uart_ich_en = 1;
            SFR(JL_IOMC->IOMC0, 8, 2, 2);
            break;
        case 7:
            uart_ich_en = 1;
            break;
        }
        gpio_set_direction(port, 1);
        gpio_set_die(port, 1);
        if (uart_ich_en) {
            JL_IOMC->IOMC2 &= ~(0x1ful << uart_ich_bit);  //wakeup0 input select
            if (port >= IO_PORTD_00) {
                port = port - IO_PORTD_00 + 18;
            }
            JL_IOMC->IOMC2 |= (port << uart_ich_bit);
        }
    } else {
        log_error("unsupported port\n");
        return -1;
    }
    if (cbfun) {
        if (uart_dev) {
            uart1_rx_wkup_irq_cbfun = cbfun;
        } else {
            uart0_rx_wkup_irq_cbfun = cbfun;
        }
    }
    HWI_Install(IRQ_PORT_IDX, (u32)port_wkup_isr, PORT_WKUP_IRQ_PRIORITY);//中断优先级3
    JL_WAKEUP->CON2 |= BIT(uart_en);  //clear pending
    if (edge) {
        JL_WAKEUP->CON1 |= BIT(uart_en);  //detect falling edge
        gpio_set_pull_up(port, 1);
        gpio_set_pull_down(port, 0);
    } else {
        JL_WAKEUP->CON1 &= ~BIT(uart_en);  //detect rising edge
        gpio_set_pull_up(port, 0);
        gpio_set_pull_down(port, 1);
    }
    JL_WAKEUP->CON0 |= BIT(uart_en);  //wakeup enable
    return 0;
}

/*
 * @brief 失能IO口[唤醒/外部中断]
 * @parm uart_dev uart0 or uart1
 * @parm port 端口
 * @return null
 */
void uartx_wkup_disable(u8 uart_dev, u8 port)
{
    u8 uart_en = uart_dev + 2;
    u8 uart_io_port = (JL_IOMC->IOMC0 >> (uart_dev * 3 + 5)) & (0x03);

    u8 temp;
    if (uart_io_port > 1) {
        temp = (JL_IOMC->IOMC2 >> 10) & 0x1f;
        if (temp >= 18) {
            temp = temp - 18 + IO_PORTD_00;
        }
    } else {
        temp = port;
    }
    if ((port < IO_PORT_MAX) && (temp == port)) {
        if (JL_WAKEUP->CON0 & BIT(uart_en)) {
            JL_WAKEUP->CON0 &= ~BIT(uart_en);
            JL_WAKEUP->CON2 |= BIT(uart_en);
            if (uart_dev) {
                if (uart1_rx_wkup_irq_cbfun) {
                    uart1_rx_wkup_irq_cbfun = NULL;
                }
            } else {
                if (uart0_rx_wkup_irq_cbfun) {
                    uart0_rx_wkup_irq_cbfun = NULL;
                }
            }
            gpio_set_die(port, 0);
        }
    }
}

/*
 * @brief sd0_dat_i 做普通IO中断[外部中断](会与sd dat功能冲突)
 * @parm port port io端口
 *      sd0_dat_wkup_enable与sd0_dat_init需选择同一组port，否则出错
 *      port参数即为sd0_dat_init中rx_pin(如果sd0已打开只有选择相
 *      应dat_io才会成功初始化wkup,若sd0未开，则根据port完成初始化)
 *       sh54:
 *            sd0_dat_i: IO_PORTA_03, IO_PORTA_10, IO_PORT_DM , IO_PORTA_06
 *            uart_rx与sd0的CMD/CLK也不能重复
 * @parm edge 检测边缘，1 下降沿，0 上升沿
 * @parm cbfun 中断回调函数
 * @return 0 成功，< 0 失败
 */
int sd0_dat_wkup_enable(u8 port, u8 edge, void (*cbfun)(void))
{
    u8 i = 0;
    u8 sd0_dat_io[4] = {IO_PORTA_03, IO_PORTA_10, IO_PORT_DM, IO_PORTA_06};
    u8 sd0_io_port = (JL_IOMC->IOMC1 >> 10) & (0x03);
    if (JL_WAKEUP->CON0 & BIT(4)) {
        log_error("SD DAT IO WKUP has been used\n");
        return -1;
    }
    if (port < IO_PORT_MAX) {
        if (JL_SD0->CON1 & 0x0001) {
            if (port != sd0_dat_io[sd0_io_port]) {
                log_error("sd data io wkup port sel error\n");
                return -1;
            }
        } else {
            for (i = 0; i < 4; i++) {
                if (port == sd0_dat_io[i]) {
                    SFR(JL_IOMC->IOMC1, 10, 2, i);
                    SFR(JL_IOMC->IOMC1, 13, 1, 1);
                    JL_SD0->CON1 |= BIT(0);
                    break;
                }
            }
            if (i == 4) {
                log_error("sd data io wkup port sel error\n");
                return -1;
            }
        }
        gpio_set_direction(port, 1);
        gpio_set_die(port, 1);
    } else {
        log_error("unsupported port\n");
        return -1;
    }
    if (cbfun) {
        sd0_dat_wkup_irq_cbfun = cbfun;
    }
    HWI_Install(IRQ_PORT_IDX, (u32)port_wkup_isr, PORT_WKUP_IRQ_PRIORITY);//中断优先级3
    JL_WAKEUP->CON2 |= BIT(4);  //clear pending
    if (edge) {
        JL_WAKEUP->CON1 |= BIT(4);  //detect falling edge
        gpio_set_pull_up(port, 1);
        gpio_set_pull_down(port, 0);
    } else {
        JL_WAKEUP->CON1 &= ~BIT(4);  //detect rising edge
        gpio_set_pull_up(port, 0);
        gpio_set_pull_down(port, 1);
    }
    JL_WAKEUP->CON0 |= BIT(4);  //wakeup enable
    return 0;
}

/*
 * @brief 失能IO口[唤醒/外部中断]
 * @parm port 端口
 * @return null
 */
void sd0_dat_wkup_disable(u8 port)
{
    if (port < IO_PORT_MAX) {
        if (JL_WAKEUP->CON0 & BIT(4)) {
            JL_WAKEUP->CON0 &= ~BIT(4);
            JL_WAKEUP->CON2 |= BIT(4);
            if (sd0_dat_wkup_irq_cbfun) {
                sd0_dat_wkup_irq_cbfun = NULL;
            }
            gpio_set_die(port, 0);
        }
    }
}




/******************************wkup test******************************/
#if 0
void port_wkup_irq_cbfun_test()
{
    log_info("port wkup cbfun!\n");
}
void irflt_wkup_irq_cbfun_test()
{
    log_info("irflt wkup cbfun!\n");
}
void uart0_rx_wkup_irq_cbfun_test()
{
    log_info("uart0 wkup cbfun!\n");
}
void uart1_rx_wkup_irq_cbfun_test()
{
    log_info("uart1 wkup cbfun!\n");
}
void sd0_dat_wkup_irq_cbfun_test()
{
    log_info("sd0 wkup cbfun!\n");
}

void port_wkup_test_main()
{
    log_info("-------------------port wkup isr---------------------------");
    port_wkup_enable(IO_PORTA_11, 1, port_wkup_irq_cbfun_test); //PA11-IO中断，1:下降沿触发，回调函数port_wkup_irq_cbfun_test
    /* port_wkup_disable(IO_PORTA_11); */
    uartx_wkup_enable(0, IO_PORTA_05, 1, uart0_rx_wkup_irq_cbfun_test); //uart0 rx PA5-IO中断，1:下降沿触发，回调函数uart0_rx_wkup_irq_cbfun_test
    /* uartx_wkup_disable(0,IO_PORTA_05); */
    uartx_wkup_enable(1, IO_PORTA_09, 1, NULL); //uart1 rx PA9-IO中断，1:下降沿触发，回调函数:NULL无
    /* uartx_wkup_disable(1,IO_PORTA_09); */
    irflt_wkup_enable(IO_PORTA_07, 0, irflt_wkup_irq_cbfun_test); //PA7-IO中断，0:上升沿触发，回调函数irflt_wkup_irq_cbfun_test
    /* irflt_wkup_disable(IO_PORTA_07); */
    sd0_dat_wkup_enable(IO_PORTA_03, 1, NULL); //PA3-IO中断，1:下降沿触发，回调函数:NULL无
    /* sd0_dat_wkup_disable(IO_PORTA_03); */
    log_info("wkupcon0:%x,con1:%x,con3:%x", JL_WAKEUP->CON0, JL_WAKEUP->CON1, JL_WAKEUP->CON3);
    log_info("iomc0:%x,iomc2:%x", JL_IOMC->IOMC0, JL_IOMC->IOMC2);
}
#endif

