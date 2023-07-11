#ifndef  __VIRTUAL_RTC_H__
#define  __VIRTUAL_RTC_H__
#include "typedef.h"

struct sys_time {
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 min;
    u8 sec;
};

struct vir_rtc_data {
    struct sys_time *default_sys_time;	//初始化默认时间
    struct sys_time *default_alarm;		//初始化默认闹钟
    void (*cbfun)(void);				//闹钟响应函数
};

s32 rtc_mem_read(u32 index, u8 *data_buf, u16 len);
s32 rtc_mem_save(u32 index, u8 *data_buf, u16 len);




void read_sys_time(struct sys_time *curr_time);		//读取当前时间
void write_sys_time(struct sys_time *curr_time);	//修改当前时间
void read_alarm(struct sys_time *alarm_time);		//读取闹钟
void write_alarm(struct sys_time *alarm_time);		//修改闹钟
void rtc_time_dump(struct sys_time *temp_time);		//打印时间
void vir_set_alarm_ctrl(u8 set_alarm);				//设置闹钟使能标志
u8 vir_get_alarm_ctrl(void);						//获取闹钟使能标志
u8 is_the_alm_wakeup();
int vir_rtc_ioctl(u32 cmd, u32 arg);
int rtc_simulate_init(const struct vir_rtc_data *arg);//虚拟rtc初始化


#endif
