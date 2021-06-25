#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "audio.h"
#include "dac.h"
#include "audio_adc.h"
#include "msg.h"
#include "bsp_loop.h"
#include "circular_buf.h"
#include "dac_api.h"
#include "sound_effect_api.h"
#include "errno-base.h"
#include "app_config.h"
#include "usb/device/msd.h"
#include "usb/device/usb_stack.h"
#include "usb/device/uac_audio.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"

#ifdef USB_DEVICE_EN
extern void simple_next(void);
void usb_slave(void)
{
    dac_init_api(32000);
    /* dac_vol(0, 31); */
    dac_fade_in_api();
    int msg[2];
    char c;
    while (1) {

        c = get_byte();

        if (0 != c) {
        }

        get_msg(2, &msg[0]);
        bsp_loop();
        USB_MassStorage(NULL);
        switch (msg[0]) {
        case MSG_PC_IN:
            log_info("pc in\n");
            usb_start();
            break;
        case MSG_PC_OUT:
            log_info("pc out\n");
            usb_stop();
            simple_next();
            return ;
        /* break; */
        case MSG_PC_SPK:
            log_info("pc spk!!!!!\n");
            break;
        case MSG_500MS:
            wdt_clear();
            /* log_char('5'); */
            break;
        }
    }
}

void usb_slave_demo(void)
{
    /* while(usb_otg_online(usb_id) != SLAVE_MODE) */
    /* { */
    /* wdt_clear(); */
    /* }  */
#if 1
    /*********debug code********/
    /* JL_PLL->CON1 = 0XA403730; //me */
    /* JL_PLL->CON1 = 0XA403751; //卓豪 */
    /* JL_PLL->CON1 = 0XA403724; //测试部 */
    JL_PLL->CON1 =    0xa40371e;
    JL_PLL->CON0 |= BIT(0);//EN
    volatile int i = 0;
    for (i = 0; i < 0xfffff; i++);
    JL_PLL->CON0 |= BIT(1);//RST
    for (i = 0; i < 0xfffff; i++);
    /*********debug code********/
#endif
    /* dac_vol(0, 31); */
    log_info("usb slave\n");
    usb_slave();

}


#endif











