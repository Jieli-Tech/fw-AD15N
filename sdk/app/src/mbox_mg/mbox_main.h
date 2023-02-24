#ifndef _MBOX_MAIN_H
#define _MBOX_MAIN_H

#include "config.h"

#if 1//def __SMART_CPU__
/*
 *SDK引脚分布：
SD:
	PA1~PA3
FM:
	PA1，PA3
LINEIN:
	PA14(AUX)，PA2(DETECT)
USB:
	DP,DM
LED7：
	PA0,PA4,PA5,PA6,PA10,PA11,PA12
UART:
	PA7
MIC:
	PB1,PA13(BIAS)
AUDIO:
	PB0
ADKEY:
	PA15
IRKEY:
	PA9
 * */

typedef enum {
    MUSIC_MODE = 0,
#ifdef USB_DEVICE_EN
    USB_DEVICE_MODE,
#endif
#ifdef FM_ENABLE
    FM_RADIO_MODE,
#endif
#ifdef LINEIN_EN
    AUX_MODE,
#endif
#ifdef LOUDSPEAKER_EN
    LOUDSPEAKER_MODE,
#endif
#ifdef REC_ENABLE
    REC_MODE,
#endif
    MAX_WORK_MODE,
} ENUM_WORK_MODE;

typedef enum {
    SDX_SUSPEND_EVENT_LINEIN = 0,

} SDX_SUSPEND_EVENT_TYPE;


extern bool Sys_Volume;
extern bool Sys_IRInput;
extern bool Sys_HalfSec;


extern ENUM_WORK_MODE work_mode;
extern u8 main_vol_L, main_vol_R;
void work_mode_save(void);
//extern void (_near_func * _pdata int_enter_pro[16])(void);

#endif

#endif
