#include "msg.h"
#include "common.h"
#include "string.h"
#include "circular_buf.h"
#include "uart.h"
#include <stdarg.h>
#include "config.h"
#include "device.h"
#include "asm/power_interface.h"
#include "power_api.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#define IDLE_CHECK_EN		0

#define IDLE_SLEEP_TIME		(30 * 2 * 60)//half second
static u16 idle_cnt = 0;

void idle_check_deal(u8 is_busy)
{
#if IDLE_CHECK_EN
    if (is_busy) {
        idle_cnt = 0;
        return;
    }

    idle_cnt ++;
#if 0
    if (IDLE_SLEEP_TIME == idle_cnt) {
        log_info("idle \n");
        post_msg(1, MSG_ENTER_IDLE);
    }
#else

    sys_power_down(-2);

#endif

#endif
}

