#ifndef _APP_POWEROFF_H
#define _APP_POWEROFF_H

extern const u16 iokey_msg_poweroff_table[3][IO_KEY_MAX_NUM];
extern const u16 adkey_msg_poweroff_table[3][AD_KEY_MAX_NUM];
int app_poweroff(void *param);
#endif

