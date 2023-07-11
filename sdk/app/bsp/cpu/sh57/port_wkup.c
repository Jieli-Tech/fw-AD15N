#include "gpio.h"
#include "port_wkup.h"
#include "jiffies.h"
#include "hwi.h"

/* #define LOG_TAG_CONST       WKUP */
#define LOG_TAG             "[PORT-WKUP]"
#include "log.h"

/**
 * 注意：JL_WAKEUP 区别于PMU管理的唤醒。可理解为一个独立的模块使用。但在低功耗的情况下，中断无效。
 */

//sh57:
/*JL_WAKEUP->CON0:
 *                17个事件：PA0~PA12,PB0~PB3
 * */

#define PORT_WKUP_IRQ_PRIORITY  3//中断优先级，范围:0~7(低~高)

static void (*port_wkup_irq_cbfun[17])(void) = {NULL};
__attribute__((interrupt("")))
static void port_wkup_isr()
{
    u8 wakeup_index = 0;
    /* log_info("png:0x%x",JL_WAKEUP->CON3); */
    for (; wakeup_index < 17; wakeup_index++) {
        if ((JL_WAKEUP->CON0 & BIT(wakeup_index)) && (JL_WAKEUP->CON3 & BIT(wakeup_index))) {
            JL_WAKEUP->CON2 |= BIT(wakeup_index);
            if (port_wkup_irq_cbfun[wakeup_index]) {
                putchar('>');
                port_wkup_irq_cbfun[wakeup_index]();
            }
        }
    }
}


/*
 * @brief 使能IO口[唤醒/外部中断]
 * @parm port 端口 such as:IO_PORTA_00
 * @parm edge 检测边缘，1 下降沿，0 上升沿
 * @parm cbfun 中断回调函数
 * @return 0 成功，< 0 失败
 */
int port_wkup_enable(u8 port, u8 edge, void (*cbfun)(void))
{
    u8 wkup_s = port;
    if (port >= IO_MAX_NUM) {
        log_error("unsupported port\n");
        return -1;
    }
    if (port > 15) {
        wkup_s = port % 16 + 13;
    }
    if (JL_WAKEUP->CON0 & BIT(wkup_s)) {
        log_error("PORT WKUP(%d) has been used\n", port);
        return -1;
    }
    JL_WAKEUP->CON2 = 0xffffffff;  //clear pending
    gpio_set_direction(port, 1);
    gpio_set_die(port, 1);
    if (edge) {
        JL_WAKEUP->CON1 |= BIT(wkup_s);  //detect falling edge
        gpio_set_pull_up(port, 1);
        gpio_set_pull_down(port, 0);
    } else {
        JL_WAKEUP->CON1 &= ~BIT(wkup_s);  //detect rising edge
        gpio_set_pull_up(port, 0);
        gpio_set_pull_down(port, 1);
    }

    if (cbfun) {
        port_wkup_irq_cbfun[wkup_s] = cbfun;
    }
    HWI_Install(IRQ_PORT_IDX, (u32)port_wkup_isr, PORT_WKUP_IRQ_PRIORITY);//中断优先级3
    JL_WAKEUP->CON2 |= BIT(wkup_s);  //clear pending
    JL_WAKEUP->CON0 |= BIT(wkup_s);  //wakeup enable
    /* log_info("wkup_s:%d, port:%d [en%x,png%x](ok)",wkup_s,port,JL_WAKEUP->CON0,JL_WAKEUP->CON3); */
    return 0;
}

/*
 * @brief 失能IO口[唤醒/外部中断]
 * @parm port 端口 such as:IO_PORTA_00
 * @return null
 */
void port_wkup_disable(u8 port)
{
    u8 wkup_s = port;
    if (port >= IO_MAX_NUM) {
        log_error("%s parameter:port error!", __func__);
        return ;
    }
    if (port > 15) {
        wkup_s = port % 16 + 13;
    }

    if (JL_WAKEUP->CON0 & BIT(wkup_s)) {
        JL_WAKEUP->CON0 &= ~BIT(wkup_s);
        JL_WAKEUP->CON2 |= BIT(wkup_s);
        gpio_set_die(port, 0);
        gpio_set_direction(port, 1);
        gpio_set_pull_up(port, 0);
        gpio_set_pull_down(port, 0);
        port_wkup_irq_cbfun[wkup_s] = NULL;
        /* log_info("wkup_s:%d, port:%d (close)",wkup_s,port); */
    } else {
        log_error("port wkup source:%d has been closed!", wkup_s);
    }
}




/******************************wkup test******************************/
#if 0

#define __port_num 3
void _port_wkup_pa1(void)//
{
    JL_PORTA->OUT ^= BIT(__port_num);
    log_info("%s", __func__);
}
void _port_wkup_pa2(void)//
{
    JL_PORTA->OUT ^= BIT(__port_num);
    log_info("%s", __func__);
}
void _port_wkup_pb0(void)
{
    JL_PORTA->OUT ^= BIT(__port_num);
    log_info("%s", __func__);
}


void port_wkup_test()
{
    log_info("-------------------port wkup isr---------------------------");
    log_info("wkupcon0:%x,con1:%x,con3:%x", JL_WAKEUP->CON0, JL_WAKEUP->CON1, JL_WAKEUP->CON3);
    gpio_write(IO_PORTA_03, 0);
    gpio_set_direction(IO_PORTA_03, 0);
    /* port_wkup_enable(IO_PORTA_00, 0, _port_wkup_pa0);//上升沿触发 */
    port_wkup_enable(IO_PORTA_01, 1, _port_wkup_pa1);//下升沿触发
    port_wkup_enable(IO_PORTA_02, 1, _port_wkup_pa2);//下升沿触发
    /* port_wkup_enable(IO_PORTA_03, 1, _port_wkup_pa3);//下升沿触发 */
    /* port_wkup_enable(IO_PORTA_04, 1, _port_wkup_pa4);//下升沿触发 */
    /* port_wkup_enable(IO_PORTA_05, 1, _port_wkup_pa5);//下升沿触发 */
    /* port_wkup_enable(IO_PORTA_06, 1, _port_wkup_pa6);//下升沿触发 */
    /* port_wkup_enable(IO_PORTA_07, 1, _port_wkup_pa7);//下升沿触发 */
    /* port_wkup_enable(IO_PORTA_08, 1, _port_wkup_pa8);//下升沿触发 */
    /* port_wkup_enable(IO_PORTA_09, 1, _port_wkup_pa9);//下升沿触发 */
    /* port_wkup_enable(IO_PORTA_10, 1, _port_wkup_pa10);//下升沿触发 */
    /* port_wkup_enable(IO_PORTA_11, 1, _port_wkup_pa11);//下升沿触发 */
    /* port_wkup_enable(IO_PORTA_12, 1, _port_wkup_pa12);//下升沿触发 */
    port_wkup_enable(IO_PORTB_00, 1, _port_wkup_pb0);//下升沿触发
    /* port_wkup_enable(IO_PORTB_01, 1, _port_wkup_pb1);//下升沿触发 */
    /* port_wkup_enable(IO_PORTB_02, 1, _port_wkup_pb2);//下升沿触发 */
    /* port_wkup_enable(IO_PORTB_03, 1, _port_wkup_pb3);//下升沿触发 */

    port_wkup_disable(IO_PORTB_03);
    /* port_wkup_disable(IO_PORTA_04); */

    log_info("wkupcon0:%x,con1:%x,con3:%x", JL_WAKEUP->CON0, JL_WAKEUP->CON1, JL_WAKEUP->CON3);
    while (1) {
        delay(999999);
        log_info("poll png:0x%x", JL_WAKEUP->CON3);
        wdt_clear();
    }
}

#endif

