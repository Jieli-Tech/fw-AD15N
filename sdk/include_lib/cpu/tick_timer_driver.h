#ifndef TICK_TIMER_H
#define TICK_TIMER_H
#include "config.h"

extern volatile u32 jiffies;
extern void tick_timer_init(void);
void tick_timer_sleep_init();

#endif
