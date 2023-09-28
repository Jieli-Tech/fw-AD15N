#include "external_interrupt.h"
#include "irq.h"
#include "gpio.h"

/* #define LOG_TAG_CONST       EXTI */
#define LOG_TAG             "[exti]"
#include "log.h"

/**
 * 注意：JL_WAKEUP 是io外部中断，区别于PMU管理的唤醒。在低功耗的情况下，中断无效。
 */


//ch58
/* IO外部中断共8个，共用输入通道，驱动会查找空闲通道并占用，无空闲通道会失败
 *    中断0~7: GP_ICH0~GP_ICH7 (每个ich可选任意io且只有单边沿,两个ich映射到同一io组成双边沿)
 * */


static void (*exti_irq_cbfun[8])(u32, u32, u32);
static u8 user_port[8];
/**
 * @brief 引脚中断函数
 */
___interrupt
void exti_irq(void)
{
    u8 exti_index = 0;
    /* log_info("png:0x%x\n", JL_WAKEUP->CON3); */
    for (; exti_index < 8; exti_index++) {
        if ((JL_WAKEUP->CON0 & BIT(exti_index)) && (JL_WAKEUP->CON3 & BIT(exti_index))) {
            JL_WAKEUP->CON2 |= BIT(exti_index);
            if (exti_irq_cbfun[exti_index]) {
                /* log_info(">"); */
                exti_irq_cbfun[exti_index](user_port[exti_index] / IO_GROUP_NUM, user_port[exti_index] % IO_GROUP_NUM, (!!(JL_WAKEUP->CON1 & BIT(exti_index))) + 1);
            }
        }
    }
}

/**
 * @brief 初始化
 */
static void exti_info_init()
{
    static u8 init_flag = 1;
    if (init_flag) {
        memset((u8 *)user_port, 0xff, sizeof(user_port));
        memset((u8 *)exti_irq_cbfun, 0, sizeof(exti_irq_cbfun));
        init_flag = 0;
    }
}

#if 1
static int gpio_exti_ich_ios(u32 gpio)
{
    u8 idle_ich;
_restart:
    idle_ich = gpio_get_unoccupied_gp_ich();
    if (idle_ich == 0xff) { //error
        log_error("Input Channel(ich0~ich7) Use Full");
        return -3;
    } else if (idle_ich > 7) { //exti:ich0~ich7
        gpio_release_gp_ich(idle_ich);
        log_error("Input Channel(ich0~ich7) Use Full");
        return -4;
    }

    /* gpio_set_fun_input_port(gpio, (PFI_GP_ICH0 + (idle_ich << 2))); */
    int ret = gpio_mux_in(gpio, (idle_ich << 8) | 0x01);
    if (ret < 0) {
        goto _restart;
    }
    return idle_ich;
}
static int gpio_exti_ich_disable(u32 index)
{
    if (index <= 7) {
        /* gpio_disable_fun_input_port(PFI_GP_ICH0 + (index << 2)); */
        gpio_mux_in_close((index << 8) | 0x01);
        gpio_release_gp_ich(index);
    } else {
        //err
    }
    return 0;
}
#endif
/*
 * @brief 使能ICH0~7口[唤醒/外部中断]
 * @parm port 端口 such as:IO_PORTA_00
 * @parm edge 检测边缘，1 下降沿，0 上升沿
 * @parm cbfun 中断回调函数
 * @return 0 成功，< 0 失败
 */
int exti_ich_init(u8 port, u8 edge, void (*cbfun)(u32, u32, u32))
{
    exti_info_init();
    if (port >= IO_PORT_MAX) {
        log_error("unsupported port\n");
        return -1;
    }
    int exti_s = gpio_exti_ich_ios(port);
    if (exti_s < 0) {
        return -1;
    }
    if (JL_WAKEUP->CON0 & BIT(exti_s)) {
        log_error("EXTI(%d) has been used\n", exti_s);
        gpio_exti_ich_disable(exti_s);
        return -1;
    }
    JL_WAKEUP->CON2 = 0xffffffff;  //clear pending
    if (edge) {
        JL_WAKEUP->CON1 |= BIT(exti_s);  //detect falling edge
        gpio_set_die(port, 1);
        gpio_set_direction(port, 1);
        gpio_set_pull_up(port, GPIO_PULLUP_10K);
        gpio_set_pull_down(port, GPIO_PULLDOWN_DISABLE);
    } else {
        JL_WAKEUP->CON1 &= ~BIT(exti_s);  //detect rising edge
        gpio_set_die(port, 1);
        gpio_set_direction(port, 1);
        gpio_set_pull_up(port, GPIO_PULLUP_DISABLE);
        gpio_set_pull_down(port, GPIO_PULLDOWN_10K);
    }

    if (cbfun) {
        exti_irq_cbfun[exti_s] = cbfun;
    }
    request_irq(IRQ_PORT_IDX, EXTI_IRQ_PRIORITY, exti_irq, 0);//中断优先级3
    /* JL_WAKEUP->CON2 |= BIT(exti_s);  //clear pending */
    JL_WAKEUP->CON0 |= BIT(exti_s);  //wakeup enable
    JL_WAKEUP->CON2 |= BIT(exti_s);  //clear pending
    user_port[exti_s] = port;
    log_info("exti_s(ich):%d, port:%d init ok\n", exti_s, port);
    return 0;
}
/*
 * @brief 失能IO口[唤醒/外部中断]
 * @parm port 端口 such as:IO_PORTA_00
 * @return null
 */
int exti_ich_close(u8 port)
{
    if (port >= IO_PORT_MAX) {
        log_error("%s parameter:port error!", __func__);
        return -1;
    }

    u8 exti_s = 0xff, exti_s1 = 0xff, i;
    for (i = 0; i < 8; i++) {
        if (port == user_port[i]) {
            if (exti_s == 0xff) {
                exti_s = i;
            } else {
                exti_s1 = i;
            }
            /* break; */
        }
    }
    if (exti_s == 0xff) {
        log_error("%s parameter:port error!", __func__);
        return -2;
    }


    if (JL_WAKEUP->CON0 & BIT(exti_s)) {
        JL_WAKEUP->CON0 &= ~BIT(exti_s);
        JL_WAKEUP->CON2 |= BIT(exti_s);
        gpio_exti_ich_disable(exti_s);
        user_port[exti_s] = 0xff;
        exti_irq_cbfun[exti_s] = NULL;
        if (JL_WAKEUP->CON0 & BIT(exti_s1)) {
            JL_WAKEUP->CON0 &= ~BIT(exti_s1);
            JL_WAKEUP->CON2 |= BIT(exti_s1);
            gpio_exti_ich_disable(exti_s1);
            user_port[exti_s1] = 0xff;
            exti_irq_cbfun[exti_s1] = NULL;
            log_info("exti_s:%d, port:%d close\n", exti_s1, port);
        }
        gpio_set_die(port, 0);
        gpio_set_direction(port, 1);
        gpio_set_pull_up(port, GPIO_PULLUP_DISABLE);
        gpio_set_pull_down(port, GPIO_PULLDOWN_DISABLE);
        log_info("exti_s:%d, port:%d close\n", exti_s, port);
        return 0;
    } else {
        log_error("exti source:%d has been closed!", exti_s);
        return -1;
    }
}




/*******************************************************/
void exti_irq_change_callback(u32 port, void (*cbfun)(u32, u32, u32))
{
    u8 change_cnt = 0;
    for (u8 i = 0; i < 8; i++) {
        if (port == user_port[i]) {
            change_cnt++;
            exti_irq_cbfun[i] = cbfun;
            log_info("change_callback:port:%d,exti_s:%d", port, i);
        }
    }
    if (!change_cnt) {
        log_error("%s parameter:port error!\n", __func__);
    }
}
void exti_irq_change_en_state(u32 port, u8 exti_en)//exti_en:1:en,0:disable
{
    u8 change_cnt = 0;
    for (u8 i = 0; i < 8; i++) {
        if (port == user_port[i]) {
            change_cnt++;
            if (exti_en) {
                JL_WAKEUP->CON2 |= BIT(i);//清一次pnd
                JL_WAKEUP->CON0 |= BIT(i);//引脚中断使能
                JL_WAKEUP->CON2 |= BIT(i);//清一次pnd
            } else {
                JL_WAKEUP->CON0 &= ~ BIT(i); //引脚中断失能
                JL_WAKEUP->CON2 |= BIT(i);//清一次pnd
            }
            log_info("change_en_state:port:%d,exti_s:%d", port, i);
        }
    }
    if (!change_cnt) {
        log_error("%s parameter:port error!\n", __func__);
    }
}

//只适用于单边沿io
void exti_irq_change_edge_state(u32 port, u8 edge)
{
    u8 change_cnt = 0;
    for (u8 i = 0; i < 8; i++) {
        if (port == user_port[i]) {
            change_cnt++;
            if (edge != (!!(JL_WAKEUP->CON1 & BIT(i)))) {
                if (edge) {
                    JL_WAKEUP->CON1 |= BIT(i);
                } else {
                    JL_WAKEUP->CON1 &= ~BIT(i);//0:上升沿
                }
            }
            log_info("change_edge_state:port:%d,exti_s:%d", port, i);
        }
    }
    if (!change_cnt) {
        log_error("%s parameter:port error!\n", __func__);
    }
}
u8 exti_irq_get_edge_state(u32 port)
{
    u8 change_cnt = 0;
    u8 edge_res = 0;
    for (u8 i = 0; i < 8; i++) {
        if (port == user_port[i]) {
            change_cnt++;
            if (JL_WAKEUP->CON1 & BIT(i)) { //fall
                edge_res |= 0x02;
            }
            if (!(JL_WAKEUP->CON1 & BIT(i))) { //rise
                edge_res |= 0x01;
            }
            /* log_info("get_edge_state:%d,exti_s:%d", port, i); */
        }
    }
    if (!change_cnt) {
        log_error("%s parameter:port error!\n", __func__);
    }
    return edge_res;
}



/*********************************************************************************************************
 * ******************************           使用举例如下           ***************************************
 * ******************************************************************************************************/
#if 0
#define TEST_IO_P JL_PORTD
#define TEST_IO_N 4
void _pa2(u32 port, u32 pin, u32 edge)
{
    TEST_IO_P->OUT ^= BIT(TEST_IO_N);
    log_info("%s", __func__);
}
void _pa3(u32 port, u32 pin, u32 edge)
{
    TEST_IO_P->OUT ^= BIT(TEST_IO_N);
    log_info("%s", __func__);
}

void _dp(u32 port, u32 pin, u32 edge)
{
    TEST_IO_P->OUT ^= BIT(TEST_IO_N);
    log_info("%s", __func__);
}
void _ich0(u32 port, u32 pin, u32 edge)
{
    TEST_IO_P->OUT ^= BIT(TEST_IO_N);
    log_info("%s", __func__);
}
#define _EDGE_1  0 //0:rise 1:fall
#define _EDGE_2  1
void mdelay(u32 ms);
void wdt_clear();
void exti_test()
{
    log_info("-------------------exti isr test---------------------------\n");
    log_info("wkupcon0:%x,con1:%x,con3:%x\n", JL_WAKEUP->CON0, JL_WAKEUP->CON1, JL_WAKEUP->CON3);
    TEST_IO_P->OUT &= ~BIT(TEST_IO_N);
    TEST_IO_P->DIR &= ~BIT(TEST_IO_N);
    exti_ich_init(IO_PORTA_02, _EDGE_1, _pa2);//上升沿触发
    exti_ich_init(IO_PORTA_03, _EDGE_1, _pa3);//上升沿触发

    exti_ich_init(IO_PORT_DP, _EDGE_2, _dp);//下升沿触发
    /* exti_ich_init(IO_PORT_DM, _EDGE_2, _dm);//下升沿触发 */
    exti_ich_init(IO_PORTA_02, _EDGE_2, _ich0);//下升沿触发
    /* exti_ich_close(IO_PORTA_02); */
    while (1) {
        putchar('-');
        mdelay(100);
        wdt_clear();
    }
}

#endif


