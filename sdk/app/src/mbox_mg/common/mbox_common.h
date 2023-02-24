#ifndef _MBOX_COMMON_H
#define _MBOX_COMMON_H
#include "typedef.h"
#include "ui/ui_api.h"
#include "app_config.h"
#include "bsp_loop.h"


void put_msg_lifo(u16 msg);
void dac_mute_toggle(void);
void set_memory(u8 addr, u8 dat);
u8 get_memory(u8 addr);
void flush_all_msg();
u16 app_get_msg(void);
u8 mbox_get_one_count(u8 tt);
void vm_refrag_check(u8 start_line, u8 end_line);
void idle_check_deal(u8 is_busy);

enum {
    EEPROM_SIGN_0 = 0,		 ///<用作EEPROM识别位,2个bytes，建议使用0x55AA
    EEPROM_SIGN_1,
    DEVICE_MEM,
    MEM_SYSMODE,
    MEM_ALARM_0_HOUR,
    MEM_ALARM_0_MIN,
    MEM_ALARM_0_WEEKDAY,
    MEM_VOL_L,
    MEM_VOL_R,
    MEM_VOL_PCL,
    MEM_VOL_PCR,

    MEM_EQ_MODE,
    MEM_PLAY_MODE,
    MEM_ACTIVE_DEV,

    MEM_FRE,
    MEM_CHAN,
    MEM_CHANNL,
};

#endif
