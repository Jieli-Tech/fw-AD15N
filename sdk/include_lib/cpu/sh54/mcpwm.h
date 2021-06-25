#ifndef __MCPWM_H__
#define __MCPWM_H__

#include "typedef.h"


void mcpwm_set(u8 channel, u32 fre, u32 duty);
void pwm_frq_duty(u8 channel, u32 fre, u32 duty);

#endif

