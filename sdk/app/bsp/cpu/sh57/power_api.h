#ifndef ___POWER_API_H___
#define ___POWER_API_H___

#include "typedef.h"

void sys_power_init();

void sys_power_down(u32 usec);

void sys_softoff();

#endif
