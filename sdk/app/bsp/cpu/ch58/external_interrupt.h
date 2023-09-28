#ifndef _IO_EXTERNAL_INTERRUPT_H
#define _IO_EXTERNAL_INTERRUPT_H

#include "typedef.h"
/**
 * 注意：JL_WAKEUP 是io外部中断，区别于PMU管理的唤醒。在低功耗的情况下，中断无效。
 */

//cd09
/* IO外部中断共8个，共用输入通道，驱动会查找空闲通道并占用，无空闲通道会失败
 *    中断0~7: GP_ICH0~GP_ICH7 (每个ich可选任意io且只有单边沿,两个ich映射到同一io组成双边沿)
 * */

#define EXTI_IRQ_PRIORITY  3//中断优先级，范围:0~7(低~高)

//无 int exti_io_init(u8 port, u8 edge, void (*cbfun)(u32, u32, u32));
//无 int exti_io_close(u8 port);
int exti_ich_init(u8 port, u8 edge, void (*cbfun)(u32, u32, u32));
int exti_ich_close(u8 port);


void exti_irq_change_callback(u32 port, void (*cbfun)(u32, u32, u32));
void exti_irq_change_en_state(u32 port, u8 exti_en);//exti_en:1:en,0:disable
void exti_irq_change_edge_state(u32 port, u8 edge);//只适用于单边沿io
u8 exti_irq_get_edge_state(u32 port);


#endif

