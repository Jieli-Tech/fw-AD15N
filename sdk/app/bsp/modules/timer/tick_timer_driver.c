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
#include "dac_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"


#define    tt_printf      log_info
#define    tt_putchar
#define    tt_printf_buf  log_info_hexdump

volatile u32 jiffies = 0;
volatile u8 isr_tick_timer_close = 0;


/*
 * 警告！！！！！！！！！！！！
 * 以下两个函数一定要内联
 * */
__attribute__((always_inline))
void tick_timer_set(bool on_off)
{
    isr_tick_timer_close = on_off;
}
__attribute__((always_inline))
bool tick_timer_close(void)
{
    return isr_tick_timer_close;
}

#if (CPU_SH55==0)
#if EXT_FLASH_EN//sh54
extern u8 get_flash_cache_timer(void);
#endif
#endif

void app_timer_loop(void);
static u8 cnt = 0;
void tick_timer_loop()
{

    cnt ++;

    app_timer_loop();
    if (0 == (cnt % 5)) {
        dac_fade_api();
        jiffies ++;
        ir_timeout();
        key_scan();
    }
#if (CPU_SH55==0)
#if EXT_FLASH_EN
    if (get_flash_cache_timer()) {
        if (0 == (cnt % 50)) {//100ms
            bsp_post_event(B_EVENT_100MS);
            /* putchar('z'); */
        }
    }
#endif
#endif
    if (0 == (cnt % 250)) { //500ms
        post_msg(1, MSG_500MS);
        cnt = 0;
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
volatile u8 sys_low_power_request = 0;
volatile u32 lowpower_usec = -2;
static void *power_ctrl;

static u32 __power_get_timeout(void *priv)
{
    //-2 , endless sleep mode
    return lowpower_usec;
}

static void __power_suspend_post(void *priv, u32 usec)
{
    sys_low_power_request = 1;
}

static void __power_resume(void *priv, u32 usec)
{
    sys_low_power_request = 0;
}

const struct low_power_operation sys_power_ops  = {
    .get_timeout 	= __power_get_timeout,

    .suspend_probe 	= NULL,
    .suspend_post 	= __power_suspend_post,
    .resume 		= __power_resume,
};

void tick_timer_sleep_init(void)
{
    power_ctrl = low_power_sys_get(NULL, &sys_power_ops);
}
