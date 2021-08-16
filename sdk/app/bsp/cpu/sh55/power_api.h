#ifndef POWER_API_H
#define POWER_API_H
#include "typedef.h"

enum {
    P3_WKUP_SRC_PCNT_OVF = 0,
    P3_WKUP_SRC_EDGE = 1,
    P3_WKUP_SRC_VDD50_LVD = 4,
    P3_WKUP_SRC_WDT_INT = 7,
};

void sys_power_init();

void sys_power_down(u32 usec);

void sys_softoff();

#endif
