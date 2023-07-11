#include "toy_idle.h"
#include "toy_main.h"
#include "includes.h"
#include "msg.h"
#include "key.h"
#include "bsp_loop.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

void toy_idle_app(void)
{
    u32 err;
    int msg[2];

    log_info("toy idle mode!\n");
    key_table_sel((void *)idle_msg_filter);

    while (1) {
        err = get_msg(2, &msg[0]);
        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
            log_info("get msg err 0x%x\n", err);
        }
        bsp_loop();

        if (NO_MSG == msg[0]) {
            continue;
        }

        switch (msg[0]) {
        case MSG_CHANGE_WORK_MODE:
            goto __toy_idle_exit;
        default:
            common_msg_deal(&msg[0]);
            break;
        }
    }
__toy_idle_exit:
    key_table_sel(NULL);
}
