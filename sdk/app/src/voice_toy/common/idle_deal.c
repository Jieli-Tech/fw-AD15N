#include "msg.h"
#include "config.h"
#include "asm/power_interface.h"
#include "power_api.h"
#include "audio.h"
#include "dac_cpu.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[idle]"
#include "log.h"

void enter_idle_deal(void)
{
    dac_power_off();
}

void exit_idle_deal(void)
{
    dac_power_on(SR_DEFAULT, 0);
}

/*----------------------------------------------------------------------------*/
/**@brief   进入power down模式
   @param   usec : -2:静态睡眠，需要等待按键唤醒
                 非-2:睡眠时间，单位us，如1000000即1s后唤醒
   @note    睡眠时间不可超过看门狗唤醒时间的一半
**/
/*----------------------------------------------------------------------------*/
void sys_idle_deal(u32 usec)
{
    enter_idle_deal();
    sys_power_down(usec);
    exit_idle_deal();
}
