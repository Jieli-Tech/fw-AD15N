#ifndef __TOY_MAIN_H__
#define __TOY_MAIN_H__
#include "typedef.h"

enum {
    TOY_MUSIC = 0,
    TOY_MIDI,
    TOY_MIDI_KEYBOARD,
    TOY_SPEAKER,
    TOY_LINEIN,
    TOY_RECORD,
    TOY_USB_SLAVE,
    TOY_IDLE,
    MAX_MODE,

    TOY_SOFTOFF = 255,
};

extern volatile u8 work_mode;
extern u16 g_tick_cnt;
extern void vm_isr_response_list_register(u32 bit_list);
extern bool Sys_IRInput;
extern u16 Input_Number;
int common_msg_deal(int *msg);
void app_next_mode(void);
#endif
