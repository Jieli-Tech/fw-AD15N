#include "app_config.h"
#include "tick_timer_driver.h"
#include "clock.h"
#include "asm/power_interface.h"
#include "msg.h"
#include "bsp_loop.h"
#include "app_power_mg.h"
#include "vm.h"
#include "dac_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"


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

#if (FLASH_CACHE_ENABLE==1)
#if EXT_FLASH_EN//sh54
extern u8 get_flash_cache_timer(void);
#endif
#endif

void app_timer_loop(void);
u16 g_tick_cnt = 0;
void sys_tick_timer(u32 cnt)
{
    if (0 == (cnt % 5)) {
        dac_fade_api();
        jiffies ++;
    }
#if (FLASH_CACHE_ENABLE==1)
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
        /* cnt = 0; */
    }

}

void tick_timer_loop()
{

    g_tick_cnt ++;

    sys_tick_timer(g_tick_cnt);
    app_timer_loop();
    modules_tick_timer(g_tick_cnt);

    if (0 == (g_tick_cnt % 250)) { //500ms
        g_tick_cnt = 0;
    }
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


__attribute__((weak))
void modules_tick_timer(u32 cnt)
{

}

