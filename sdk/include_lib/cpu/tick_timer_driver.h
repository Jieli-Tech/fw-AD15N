#ifndef TICK_TIMER_H
#define TICK_TIMER_H
#include "config.h"

extern volatile u32 jiffies;
extern void tick_timer_init(void);
void tick_timer_sleep_init();
void modules_tick_timer(u32 cnt);
// extern void lrc_scan(void);
#endif
