#ifndef POWER_API_H
#define POWER_API_H
#include "typedef.h"

enum {
    P3_WKUP_SRC_PCNT_OVF = 0,
    P3_WKUP_SRC_EDGE = 1,
    P3_WKUP_SRC_VDD50_LVD = 4,
    P3_WKUP_SRC_WDT_INT = 7,
};

enum {
    LVLD_SEL_18V = 0,
    LVLD_SEL_19V,
    LVLD_SEL_20V,
    LVLD_SEL_21V,
    LVLD_SEL_22V,
    LVLD_SEL_23V,
    LVLD_SEL_24V,
    LVLD_SEL_25V = 7,
};

void sys_power_init();

void sys_power_down(u32 usec);

void sys_softoff();

void p33_vlvd(u8 vlvd);

#endif
