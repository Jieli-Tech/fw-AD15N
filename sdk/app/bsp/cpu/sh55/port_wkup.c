#include "asm/power/p33.h"
#include "hwi.h"
#include "gpio.h"
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
//sh55:
//uart0_rxas: IO_PORTA_07, IO_PORTA_01,ich2
//uart1_rxas: IO_PORTA_05, IO_PORTB_09,ich0
//sd0_dat_i: IO_PORTA_03, IO_PORTB_02, IO_PORTB_06 , IO_PORTA_08,

__attribute__((interrupt("")))
static void port_wkup_isr()
{
    if ((JL_WAKEUP->CON0 & BIT(0)) && (JL_WAKEUP->CON3 & BIT(0))) {
        JL_WAKEUP->CON2 |= BIT(0);
        log_info("port %d wakeup\n", (JL_IOMC->IOMC2 >> 12) & 0x3f);
    }
    if ((JL_WAKEUP->CON0 & BIT(1)) && (JL_WAKEUP->CON3 & BIT(1))) {
        JL_WAKEUP->CON2 |= BIT(1);
        log_info("IRFLT %d wakeup\n", (JL_IOMC->IOMC2 >> 6) & 0x3f);
    }
    if ((JL_WAKEUP->CON0 & BIT(2)) && (JL_WAKEUP->CON3 & BIT(2))) {
        JL_WAKEUP->CON2 |= BIT(2);
        log_info("uart0_rx wakeup\n");
    }
    if ((JL_WAKEUP->CON0 & BIT(3)) && (JL_WAKEUP->CON3 & BIT(3))) {
        JL_WAKEUP->CON2 |= BIT(3);
        log_info("uart1_rx wakeup\n");
    }
    if ((JL_WAKEUP->CON0 & BIT(4)) && (JL_WAKEUP->CON3 & BIT(4))) {
        JL_WAKEUP->CON2 |= BIT(4);
        log_info("sd0_dat_i wakeup\n");
    }

}

/*
 * @brief irflt 普通IO中断[唤醒/外部中断](与红外冲突)
 * @parm port 端口
 * @parm edge 检测边缘，1 下降沿，0 上升沿
 * @return 0 成功，< 0 失败
 */
int irflt_wkup_enable(u8 port, u8 edge)
{
    if (JL_WAKEUP->CON0 & BIT(1)) {
        log_error("IRFLT WKUP has been used\n");
        return -1;
    }
    if (port < IO_PORT_MAX) {
        gpio_set_direction(port, 1);
        gpio_set_pull_up(port, 1);
        gpio_set_pull_down(port, 0);
        gpio_set_die(port, 1);
        JL_IOMC->IOMC2 &= ~(0x3ful << 6);  //wakeup0 input select
        if (port >= IO_PORTD_00) {
            port = port - IO_PORTD_00 + 28;
        }
        JL_IOMC->IOMC2 |= (port << 6);
    } else {
        log_error("unsupported port\n");
        return -1;
    }
    JL_WAKEUP->CON2 |= BIT(1);  //clear pending
    if (edge) {
        JL_WAKEUP->CON1 |= BIT(1);  //detect falling edge
    } else {
        JL_WAKEUP->CON1 &= ~BIT(1);  //detect rising edge
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
    temp = (JL_IOMC->IOMC2 >> 6) & 0x3f;
    if (temp >= 28) {
        temp = temp - 28 + IO_PORTD_00;
    }
    if ((port < IO_PORT_MAX) && (temp == port)) {
        if (JL_WAKEUP->CON0 & BIT(1)) {
            JL_WAKEUP->CON0 &= ~BIT(1);
            JL_WAKEUP->CON2 |= BIT(1);
            gpio_set_die(port, 0);
        }
    }
}
/*
 * @brief 使能IO口[唤醒/外部中断]
 * @parm port 端口
 * @parm edge 检测边缘，1 下降沿，0 上升沿
 * @return 0 成功，< 0 失败
 */
int port_wkup_enable(u8 port, u8 edge)
{
    if (JL_WAKEUP->CON0 & BIT(0)) {
        log_error("PORT WKUP0 has been used\n");
        return -1;
    }
    if (port < IO_PORT_MAX) {
        gpio_set_direction(port, 1);
        gpio_set_pull_up(port, 1);
        gpio_set_pull_down(port, 0);
        gpio_set_die(port, 1);
        JL_IOMC->IOMC2 &= ~(0x3ful << 12);  //wakeup0 input select
        if (port >= IO_PORTD_00) {
            port = port - IO_PORTD_00 + 28;
        }
        JL_IOMC->IOMC2 |= (port << 12);
    } else {
        log_error("unsupported port\n");
        return -1;
    }
    JL_WAKEUP->CON2 |= BIT(0);  //clear pending
    if (edge) {
        JL_WAKEUP->CON1 |= BIT(0);  //detect falling edge
    } else {
        JL_WAKEUP->CON1 &= ~BIT(0);  //detect rising edge
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
    temp = (JL_IOMC->IOMC2 >> 12) & 0x3f;
    if (temp >= 28) {
        temp = temp - 28 + IO_PORTD_00;
    }
    if ((port < IO_PORT_MAX) && (temp == port)) {
        if (JL_WAKEUP->CON0 & BIT(0)) {
            JL_WAKEUP->CON0 &= ~BIT(0);
            JL_WAKEUP->CON2 |= BIT(0);
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
 *       sh55:
 *            uart0_rx: IO_PORTA_07, IO_PORTA_01,ich2(与IO_WKUP共用)
 *            uart1_rx: IO_PORTA_05, IO_PORTB_09,ich0(与timer012 capin共用)*
 * @parm edge 检测边缘，1 下降沿，0 上升沿
 * @return 0 成功，< 0 失败
 */
int uartx_wkup_enable(u8 uart_dev, u8 port, u8 edge)
{
    u8 uart_en = uart_dev + 2;
    u8 uart_io_port = (JL_IOMC->IOMC0 >> (uart_dev * 3 + 5)) & (0x03);
    u8 uart_ich_en = 0, uart_ich_bit = 0;
    if (uart_dev > 1) {
        log_error("There is no uart_dev:%d!\n", uart_dev);
    }
    if (JL_WAKEUP->CON0 & BIT(uart_en)) {
        log_error("UART%d RX WKUP has been used\n", uart_dev);
        return -1;
    }
    if (port < IO_PORT_MAX) {
        switch (uart_io_port + uart_dev * 4) {
        case 0:
            if (JL_UT0->CON & 0x0001) {
                if (port != IO_PORTA_07) {
                    log_error("uart0 rx wkup port sel error\n");
                    return -1;
                }
                break;
            } else {
                if (port == IO_PORTA_07) {
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
            uart_ich_bit = 12;
            SFR(JL_IOMC->IOMC0, 5, 2, 2);
            break;
        case 3:
            uart_ich_en = 1;
            uart_ich_bit = 12;
            break;
        case 4:
            if ((JL_UT1->CON0 & 0x0003) != 0) {
                if (port != IO_PORTA_05) {
                    log_error("uart1 rx wkup port sel error\n");
                    return -1;
                }
                break;
            } else {
                if (port == IO_PORTA_05) {
                    SFR(JL_IOMC->IOMC0, 7, 3, 1);
                    break;
                }
            }
        case 5:
            if ((JL_UT1->CON0 & 0x0003) != 0) {
                if (port != IO_PORTB_09) {
                    log_error("uart1 rx wkup port sel error\n");
                    return -1;
                }
                break;
            } else {
                if (port == IO_PORTB_09) {
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
        gpio_set_pull_up(port, 1);
        gpio_set_pull_down(port, 0);
        gpio_set_die(port, 1);
        if (uart_ich_en) {
            JL_IOMC->IOMC2 &= ~(0x3ful << uart_ich_bit);  //wakeup0 input select
            if (port >= IO_PORTD_00) {
                port = port - IO_PORTD_00 + 28;
            }
            JL_IOMC->IOMC2 |= (port << uart_ich_bit);
        }
    } else {
        log_error("unsupported port\n");
        return -1;
    }
    JL_WAKEUP->CON2 |= BIT(uart_en);  //clear pending
    if (edge) {
        JL_WAKEUP->CON1 |= BIT(uart_en);  //detect falling edge
    } else {
        JL_WAKEUP->CON1 &= ~BIT(uart_en);  //detect rising edge
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
        temp = (JL_IOMC->IOMC2 >> 12) & 0x3f;
        if (temp >= 28) {
            temp = temp - 28 + IO_PORTD_00;
        }
    } else {
        temp = port;
    }
    if ((port < IO_PORT_MAX) && (temp == port)) {
        if (JL_WAKEUP->CON0 & BIT(uart_en)) {
            JL_WAKEUP->CON0 &= ~BIT(uart_en);
            JL_WAKEUP->CON2 |= BIT(uart_en);
            gpio_set_die(port, 0);
        }
    }
}

void port_wkup_test_main()
{
    log_info("-------------------port wkup isr---------------------------");
    port_wkup_enable(IO_PORTA_08, 0);//PA8-IO中断，0:上升沿触发
    /* port_wkup_disable(IO_PORTA_08); */
    uartx_wkup_enable(0, IO_PORTA_07, 0); //uart0 rx PA7-IO中断，0:上升沿触发
    /* uartx_wkup_disable(0,IO_PORTA_07); */
    uartx_wkup_enable(1, IO_PORTB_09, 0); //uart1 rx PA5-IO中断，0:上升沿触发
    /* uartx_wkup_disable(1,IO_PORTB_09); */
    irflt_wkup_enable(IO_PORTA_09, 0);//PA9-IO中断，0:上升沿触发
    /* irflt_wkup_disable(IO_PORTA_09); */
    log_info("wkupcon0:%x,con1:%x,con3:%x", JL_WAKEUP->CON0, JL_WAKEUP->CON1, JL_WAKEUP->CON3);
    log_info("iomc0:%x,iomc2:%x", JL_IOMC->IOMC0, JL_IOMC->IOMC2);
    /* irflt_wkup_disable(u8 port); */
    HWI_Install(IRQ_PORT_IDX, (u32)port_wkup_isr, 3);//中断优先级3
}
