#ifndef _PORT_WKUP_H_
#define _PORT_WKUP_H_
#include "typedef.h"

//sh57:
/*JL_WAKEUP->CON0:
 *                17个事件：PA0~PA12,PB0~PB3
 * */

/*
 * @brief 使能IO口[唤醒/外部中断]
 * @parm port 端口 such as:IO_PORTA_00
 * @parm edge 检测边缘，1 下降沿，0 上升沿
 * @parm cbfun 中断回调函数
 * @return 0 成功，< 0 失败
 */
int port_wkup_enable(u8 port, u8 edge, void (*cbfun)(void));
/*
 * @brief 失能IO口[唤醒/外部中断]
 * @parm port 端口 such as:IO_PORTA_00
 * @return null
 */
void port_wkup_disable(u8 port);
#endif

