#ifndef __RTC_H__
#define __RTC_H__

#include "typedef.h"

enum KEEP_RTC_CLK {
    KEEP_CLK_LRC = 1,
    KEEP_CLK_32K,
};


enum RTC_CLK {
    CLK_SEL_LRC = 1,
    CLK_SEL_32K,
    CLK_SEL_BTOSC,
};

enum RTC_SEL {
    HW_RTC,
    VIR_RTC,
};


struct rtc_config {
    enum RTC_CLK rtc_clk;
    /* u8 rtc_div; */
    enum RTC_SEL rtc_sel;
    u8 alm_en;
    u8 timer_wkup_en;
    void (*cbfun)(void);
};

struct sys_time {
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 min;
    u8 sec;
};

//API
void set_time_wakeup_soff(enum RTC_CLK clk_sel, u32 sec);
void time_waekup_soff_disable(void);
u32 rtc_get_alm_wkup(void);

struct rtc_dev_ops {
    void (*rtc_init)(const struct rtc_config *rtc);
    void (*rtc_get_time)(struct sys_time *time);
    void (*rtc_set_time)(const struct sys_time *time);
    void (*rtc_get_alarm)(struct sys_time *time);
    void (*rtc_set_alarm)(const struct sys_time *time);
    void (*rtc_time_dump)(void);
    void (*rtc_wkup_en)(u32 en);
    u32(*get_rtc_wkup_en)(void);

};


void rtc_dev_init(const struct rtc_config *rtc);
void rtc_read_time(struct sys_time *time);
void rtc_write_time(const struct sys_time *time);
void rtc_read_alarm(struct sys_time *time);
void rtc_write_alarm(const struct sys_time *time);
void rtc_debug_dump(void);
void rtc_alarm_en(u32 en);
u32 rtc_get_alarm_en(void);


bool leapyear(u32 year); //判断是否为闰年
u32 year_to_day(u32 year);
u32 month_to_day(u32 year, u32 month);
void day_to_ymd(u32 day, struct sys_time *sys_time);
u32 ymd_to_day(struct sys_time *time);
u32 caculate_weekday_by_time(struct sys_time *r_time); //计算当天为星期几
u32 get_day_of_month(u32 year, u32 month); //返回每月的天数


#endif // __RTC_API_H__
