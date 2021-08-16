#ifndef _PORT_WKUP_H_
#define _PORT_WKUP_H_
#include "typedef.h"

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

/*
 * @brief irflt 普通IO中断[唤醒/外部中断](与红外冲突)
 * @parm port 端口 ich1
 * @parm edge 检测边缘，1 下降沿，0 上升沿
 * @parm cbfun 中断回调函数
 * @return 0 成功，< 0 失败
 */
int irflt_wkup_enable(u8 port, u8 edge, void (*cbfun)(void));
/*
 * @brief 失能IO口[唤醒/外部中断]
 * @parm port 端口
 * @return null
 */
void irflt_wkup_disable(u8 port);
/*
 * @brief 使能IO口[唤醒/外部中断]
 * @parm port 端口 ich2
 * @parm edge 检测边缘，1 下降沿，0 上升沿
 * @parm cbfun 中断回调函数
 * @return 0 成功，< 0 失败
 */
int port_wkup_enable(u8 port, u8 edge, void (*cbfun)(void));
/*
 * @brief 失能IO口[唤醒/外部中断]
 * @parm port 端口
 * @return null
 */
void port_wkup_disable(u8 port);
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
int uartx_wkup_enable(u8 uart_dev, u8 port, u8 edge, void (*cbfun)(void));
/*
 * @brief 失能IO口[唤醒/外部中断]
 * @parm uart_dev uart0 or uart1
 * @parm port 端口
 * @return null
 */
void uartx_wkup_disable(u8 uart_dev, u8 port);
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
int sd0_dat_wkup_enable(u8 port, u8 edge, void (*cbfun)(void));
/*
 * @brief 失能IO口[唤醒/外部中断]
 * @parm port 端口
 * @return null
 */
void sd0_dat_wkup_disable(u8 port);

#endif

