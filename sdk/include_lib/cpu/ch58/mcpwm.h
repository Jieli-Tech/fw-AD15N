#ifndef _MCPWM_H_
#define _MCPWM_H_


#include "typedef.h"


/* 对齐方式选择 */
typedef enum {
    pwm_edge_aligned,                                   ///< 边沿对齐模式
    pwm_center_aligned,                                 ///< 中心对齐模式
} pwm_aligned_mode_type;

/* pwm通道选择 */
typedef enum {
    pwm_ch0,
    pwm_ch1,
    pwm_ch_max,
} pwm_ch_num_type;

/* MCPWM TIMER寄存器 */
typedef struct _pwm_timer_reg {
    volatile u32 tmr_con;
    volatile u32 tmr_cnt;
    volatile u32 tmr_pr;
} PWM_TIMER_REG;

/* MCPWM通道寄存器 */
typedef struct _pwm_ch_reg {
    volatile u32 ch_con0;
    volatile u32 ch_con1;
    volatile u32 ch_cmph;
    volatile u32 ch_cmpl;
} PWM_CH_REG;

/* 初始化要用的参数结构体 */
struct pwm_platform_data {
    pwm_aligned_mode_type pwm_aligned_mode;             ///< PWM对齐方式选择
    pwm_ch_num_type pwm_ch_num;                         ///< 选择pwm通道号
    u32 frequency;                               		///< 初始化该通道的频率,即该通道的两个引脚H&L都是同一个频率。
    u16 h_duty;                                         ///< H引脚的初始占空比，0~10000 对应 0%~100% 。每个引脚可以有不同的占空比。
    u16 l_duty;                                         ///< L引脚的初始占空比，0~10000 对应 0%~100% 。每个引脚可以有不同的占空比。
    u8 h_pin;                                           ///< 该通道的H引脚，不需要则填-1
    u8 l_pin;                                           ///< 该通道的L引脚，不需要则填-1
    u8 h_inv_en;                                        ///< 该通道的H引脚输出的波形，每个周期内，0: 先输出高电平， 1: 先输出低电平，此时占空比为低电平的占空比
    u8 l_inv_en;                                        ///< 该通道的L引脚输出的波形，每个周期内，0: 先输出高电平， 1: 先输出低电平，此时占空比为低电平的占空比
};


void mcpwm_set_frequency(pwm_ch_num_type ch, pwm_aligned_mode_type align, u32 frequency);
void mcpwm_set_duty(pwm_ch_num_type pwm_ch, u16 duty);
void mcpwm_set_h_duty(pwm_ch_num_type pwm_ch, u16 h_duty);
void mcpwm_set_l_duty(pwm_ch_num_type pwm_ch, u16 l_duty);
void mctimer_ch_open_or_close(pwm_ch_num_type pwm_ch, u8 enable);
void mcpwm_ch_open_or_close(pwm_ch_num_type pwm_ch, u8 enable);
void mcpwm_open(pwm_ch_num_type pwm_ch);
void mcpwm_close(pwm_ch_num_type pwm_ch);
void mcpwm_init(struct pwm_platform_data *arg);
void mcpwm_test(void);


#endif

