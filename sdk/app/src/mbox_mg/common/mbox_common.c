#include "mbox_common.h"
#include "msg.h"
#include "vm.h"
#include "power_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

void put_msg_lifo(u16 msg)
{
    int err = post_msg(1, msg);
    if (err != MSG_NO_ERROR) {
        log_info("post msg error !!! \n");
    }
}

u16 app_get_msg(void)
{
    int msg[2];
    get_msg(2, &msg[0]);

    return (u16)msg[0];
}

void dac_mute_toggle(void)
{

}

void flush_all_msg()
{
    clear_all_message();
}

static const u8 one_table[] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
u8 mbox_get_one_count(u8 tt)
{
    return one_table[tt & 0x0f] + one_table[(tt >> 4)];
}

void idle_check_deal(u8 is_busy)
{
    if (is_busy) {
        return;
    }
#if (!POWERDOWN_UDISK_MODE_EN)
    if (dev_online("udisk0")) {
        return;
    }
#endif
    sys_power_down(-2);
}

