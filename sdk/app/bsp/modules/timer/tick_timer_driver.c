#include "app_config.h"
#include "tick_timer_driver.h"
#include "clock.h"
#include "sdmmc/sd_host_api.h"
#include "key.h"
#include "adc_drv.h"
#include "asm/power_interface.h"
#include "irflt.h"
#include "msg.h"
#include "bsp_loop.h"
#include "app_power_mg.h"
#include "vm.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"


#define    tt_printf      log_info
#define    tt_putchar
#define    tt_printf_buf  log_info_hexdump

volatile u32 jiffies = 0;
volatile u8 sys_low_power_request = 0;
volatile u8 isr_tick_timer_close = 0;


void app_timer_loop(void);
void tick_timer_loop()
{
    static u8 cnt = 0;

    cnt ++;

    app_timer_loop();
    if (0 == (cnt % 5)) {

        jiffies ++;
        ir_timeout();
        key_scan();
    }
    if (0 == (cnt % 250)) { //500ms
        post_msg(1, MSG_500MS);
        cnt = 0;
    }
    if (0 == (cnt % 50)) { //100ms
        bsp_post_event(B_EVENT_100MS);
    }

    if (0 == (cnt % 100)) { //200ms
#if TFG_SD_EN
        sd0_dev_detect(NULL);
#endif
    }

    if (0 == (cnt % 125)) { //250ms
#if TCFG_PC_ENABLE || TCFG_UDISK_ENABLE
        void usb_hotplug_detect(void *argv);
        usb_hotplug_detect(NULL);
#endif
    }

    lrc_scan();
    if (!sys_low_power_request) {
        low_power_sys_request(NULL);
    }

}

__attribute__((weak))
AT(.tick_timer_code)
void tick_timer_ram_loop(void)
{

}

SET(interrupt(""))
AT(.tick_timer_code)
void tick_timer_isr()
{
    /*
     *用户的timer函数不能加入到这里,加到tick_timer_loop
     * */
    //bit_clr_swi(TIME0_INIT);
    j32CPU(core_num())->TTMR_CON |= BIT(6);

    tick_timer_ram_loop();
    if (isr_tick_timer_close) {
        return;
    }

    tick_timer_loop();
}

void tick_timer_init(void)
{
    tt_printf("tick timer init \n");
    HWI_Install(IRQ_TICKTMR_IDX, (u32)tick_timer_isr, IRQ_TICKTMR_IP) ; //timer0_isr
    j32CPU(core_num())->TTMR_CNT = 0X00;
    j32CPU(core_num())->TTMR_PRD = (sys_clock_get() / 1000) * 2;
    j32CPU(core_num())->TTMR_CON = BIT(0);
    tt_printf("tick timer init end \n");
}

void os_time_dly(u32 tick)
{
    tick = tick + jiffies;

    while (1) {
        if (time_after(jiffies, tick)) {
            break;
        }
    }
}

void delay_10ms(u32 tick)
{
    tick = tick + jiffies;
    while (1) {
        if (time_after(jiffies, tick)) {
            break;
        }
    }
}

/***********************************sleep api*****************************************************/


extern int eSystemConfirmStopStatus(void);

#define     APP_IO_DEBUG_0(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}
#define     APP_IO_DEBUG_1(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}

#define configTICK_RATE_HZ						( 100 ) /* In this non-real time simulated environment the tick frequency has to be at least a multiple of the Win32 tick frequency, and therefore very slow. */
#ifndef configSYSTICK_CLOCK_HZ
#define configSYSTICK_CLOCK_HZ  clk_get("sys")
#endif

static u32 compensation;
static u32 tick_cnt;
static void *power_ctrl;

extern int eSystemConfirmStopStatus(void);

#define     APP_IO_DEBUG_0(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}
#define     APP_IO_DEBUG_1(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}

static void vPortStopTimerInterrupt(void)
{
    j32CPU(core_num())->TTMR_CON = BIT(6);
}

static void vPortRecoverTimerInterrupt(void)
{
    /*u32 configRate = (configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ) - 1UL;*/
    u32 configRate = (configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ);

    if (tick_cnt + compensation <= configRate) {
        j32CPU(core_num())->TTMR_CNT = tick_cnt + compensation;
    }

    j32CPU(core_num())->TTMR_PRD = configRate;

    /* log_debug("TICK_PRD %d / %d", TICK_PRD, configRate); */
    j32CPU(core_num())->TTMR_CON = BIT(6) | BIT(0);
}

static u32 __power_get_timeout(void *priv)
{
    u32 us;

    int eStopStatus;

    if (low_power_sys_is_idle() == 0) {
        /*log_error("low_power_sys_is_idle"); */
        return 0;
    }


    us = 100 * 1000L;
    /*log_info("eNoTasksWaitingTimeout"); */

    eStopStatus = eSystemConfirmStopStatus();
    switch (eStopStatus) {
    case 0:
        break;
    case 1:
        us = -2;
        wdt_close();
        break;
    default:
        us = eStopStatus * 1000L;
        break;
    }

    //tick to time
    tick_cnt = j32CPU(core_num())->TTMR_CNT;

    compensation = (u64)tick_cnt  * 1000000L / configSYSTICK_CLOCK_HZ ;

    us -= compensation;

    /* APP_IO_DEBUG_1(A, 8); */
    /* log_debug("Sys timeout : %d", (eSleepStatus == eAbortSleep) ? 0 : us); */
    /* int mask; */

    /* __asm__ volatile("mov %0,icfg" : "=r"(mask)); */
    /* log_debug("Sys timeout : %d(ms) %d", (eSleepStatus == eAbortSleep) ? 0 : xGetExpectedIdleTime() * (1000L / configTICK_RATE_HZ), xTaskGetTickCount()); */
    return  us;
}

static void __power_suspend_post(void *priv, u32 usec)
{
    sys_low_power_request = 1;
    vPortStopTimerInterrupt();
}

static void __power_resume(void *priv, u32 usec)
{
    /* int mask; */
    if (usec != -2) {
        /* __asm__ volatile("mov %0,icfg" : "=r"(mask)); */
        /* log_debug("Sys resume : %d / %d", usec, usec / (1000000L / configTICK_RATE_HZ)); */
        /* Correct the kernels tick count to account for the time the
        microcontroller spent in its low power state. */
        /*vTaskStepTick(usec / (1000000L / configTICK_RATE_HZ));*/

        jiffies += usec / (1000000L / configTICK_RATE_HZ);

        //reset tick
        compensation = usec % (1000000L / configTICK_RATE_HZ);

        /* log_i("0-compensation %d(us)", compensation); */
        //time to tick
        compensation = (u64)compensation * configSYSTICK_CLOCK_HZ / 1000000L;

        /* log_i("1-compensation 0x%x(tick)", compensation); */
    };

    /* Restart the timer that is generating the tick interrupt. */
    vPortRecoverTimerInterrupt();

    sys_low_power_request = 0;
}

const struct low_power_operation sys_power_ops  = {
    .get_timeout 	= __power_get_timeout,

    .suspend_probe 	= NULL,
    .suspend_post 	= __power_suspend_post,
    .resume 		= __power_resume,
};

void tick_timer_sleep_init()
{
    power_ctrl = low_power_sys_get(NULL, &sys_power_ops);
}




