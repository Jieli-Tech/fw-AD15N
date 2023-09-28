#include "cpu.h"
/* #include "vm.h" */
/* #include "asm/power/virtual_rtc.h" */
#if defined(VIRTUAL_RTC_EN) && (VIRTUAL_RTC_EN)
#include "asm/power/p33.h"
#include "asm/power/virtual_rtc.h"
#endif

#define LOG_TAG     "[rtc]"
#include "log.h"

#if SYS_VM_EN
#include "vm_api.h"
#define rtc_save_api(p0, p1, p2)    vm_write(p0, p1, p2)
#define rtc_read_api(p0, p1, p2)    vm_read(p0, p1, p2)
#else

#define rtc_save_api(...)   -1
#define rtc_read_api(...)   -1
#endif

s32 rtc_mem_save(u32 index, u8 *data_buf, u16 len)
{
    return rtc_save_api(index, data_buf, len);
}
s32 rtc_mem_read(u32 index, u8 *data_buf, u16 len)
{
    return rtc_read_api(index, data_buf, len);

}

#if 0

void alm_test_cbfun(void)
{
    log_info("alm_ring!!!!!");
    log_info("alm_ring!!!!!");
    log_info("alm_ring!!!!!");
}

static struct sys_time def_sys_time = {  //初始一下当前时间
    .year = 2050,
    .month = 1,
    .day = 1,
    .hour = 1,
    .min = 1,
    .sec = 1,
};

static struct sys_time test_sys_time = {  //初始一下目标时间
    .year = 2051,
    .month = 2,
    .day = 2,
    .hour = 2,
    .min = 2,
    .sec = 2,
};

static struct sys_time def_alarm = {     //初始一下闹钟时间
    .year = 2050,
    .month = 1,
    .day = 1,
    .hour = 1,
    .min = 1,
    .sec = 10,
};

static struct sys_time test_alarm = {     //初始一下目标闹钟时间
    .year = 2051,
    .month = 2,
    .day = 2,
    .hour = 2,
    .min = 2,
    .sec = 20,
};

const struct vir_rtc_data rtc_demo = {
    .default_sys_time = &def_sys_time,
    .default_alarm = &def_alarm,
    .cbfun = alm_test_cbfun,
};

void rtc_test(void)
{
    extern void wdt_close();
    extern void sys_power_down(u32 usec);
    extern void delay(volatile u32 t);
    wdt_close();

    struct sys_time tmp_time;
    rtc_simulate_init(&rtc_demo);

    /* =================================== */
    /* 读写时间和闹钟 */
    /* =================================== */
    read_sys_time(&tmp_time);						//读当前rtc时间
    log_info("rtc_rtc_read_time_before: %d-%d-%d %d:%d:%d\n", tmp_time.year, tmp_time.month, tmp_time.day, tmp_time.hour, tmp_time.min, tmp_time.sec);	//打印读取时间值

    write_sys_time(&test_sys_time); 		//修改rtc时间
    read_sys_time(&tmp_time); 				//读修改后rtc时间
    log_info("rtc_rtc_read_time_after: %d-%d-%d %d:%d:%d\n", tmp_time.year, tmp_time.month, tmp_time.day, tmp_time.hour, tmp_time.min, tmp_time.sec);		//打印修改后时间值

    read_alarm(&tmp_time); 					//读当前alarm时间
    log_info("rtc_read_alarm_before: %d-%d-%d %d:%d:%d\n", tmp_time.year, tmp_time.month, tmp_time.day, tmp_time.hour, tmp_time.min, tmp_time.sec);	//打印读取闹钟时间值

    write_alarm(&test_alarm); 	//修改alarm时间
    read_alarm(&tmp_time); 		//读修改后alarm时间
    log_info("rtc_read_alarm_after: %d-%d-%d %d:%d:%d\n", tmp_time.year, tmp_time.month, tmp_time.day, tmp_time.hour, tmp_time.min, tmp_time.sec);		//打印修改后闹钟时间值

    /* ============================================== */
    /* 走时打印测试 */
    /* ============================================== */
    while (1) {
        delay(50000);
        read_sys_time(&tmp_time);
        rtc_time_dump(&tmp_time);
        /* sys_power_down(6000000); */
        /* power_set_soft_poweroff(); */
    }
}

#endif
