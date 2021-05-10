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
//uart0_rxas: IO_PORTA_07, IO_PORTA_01,
//uart1_rxas: IO_PORTA_05, IO_PORTB_09,
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
 *  * @brief 使能IO口[唤醒/外部中断]
 *   * @parm port 端口
 *    * @parm edge 检测边缘，1 下降沿，0 上升沿
 *     * @return 0 成功，< 0 失败
 *      */
int port_wkup_enable(u8 port, u8 edge)
{
    if (JL_WAKEUP->CON0 & BIT(0)) {
        log_error("WKUP0 has been used\n");
        return -1;
    }
    if (port < IO_PORT_MAX) {
        gpio_set_direction(port, 1);
        gpio_set_pull_up(port, 0);
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
 *  * @brief 失能IO口[唤醒/外部中断]
 *   * @parm port 端口
 *    * @return null
 *     */
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

void port_wkup_test_main()
{
    port_wkup_enable(IO_PORTA_08, 0);//PA7-IO中断，0:上升沿触发
    /* port_wkup_disable(IO_PORTA_08); */
    HWI_Install(IRQ_PORT_IDX, (u32)port_wkup_isr, 3);//中断优先级3
}
