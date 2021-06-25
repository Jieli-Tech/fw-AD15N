#include "config.h"
#include "app_mg.h"
#include "msg.h"
#include "app_mg/music/app_music.h"
#include "app_mg/poweroff/app_poweroff.h"
#include "usb/device/usb_stack.h"
#include "usb/device/msd.h"
#include "audio.h"
#include "efuse.h"
#if APP_MODE_MIDI_EN
#include "app_mg/midi/app_midi.h"
#endif
#if APP_MODE_RECORD_EN
#include "app_mg/record/app_record.h"
#endif
#if APP_MODE_LINEIN_EN
#include "app_mg/linein/app_linein.h"
#endif

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"

static volatile u8 cur_app = APP_FIRST_RESERVE;
static void *app_param = NULL;
static volatile u8 app_switch_flg = 0;

void app_timer_loop(void)
{

}

int app_mg_init(APP_T app, void *param)
{
    cur_app = app;
    app_param = param;
    app_switch_flg = 0;
    return 0;
}

int app_switch(APP_T app, void *param)
{
    if (cur_app == app) {
        return (-1);
    }

    cur_app = app;
    app_param = param;
    app_switch_flg = 1;
    post_event(EVENT_APP_SW_ACTIVE);
    return 0;
}

u8 app_get(void)
{
    return cur_app;
}

bool app_switch_en(void)
{
    return app_switch_flg ? true : false;
}

int app_switch_next(void)
{
    cur_app++;
    if (cur_app >= APP_MAX) {
        cur_app = APP_FIRST_RESERVE + 1;
    }

    app_switch_flg = 1;
    post_event(EVENT_APP_SW_ACTIVE);
    return 0;
}

int app_switch_priv(void)
{
    cur_app--;
    if (cur_app <= APP_FIRST_RESERVE) {
        cur_app = APP_MAX - 1;
    }

    app_switch_flg = 1;
    post_event(EVENT_APP_SW_ACTIVE);
    return 0;
}
#if APP_MODE_USB_EN
int app_switch_to_usb(void)
{
    cur_app = APP_USB;

    app_switch_flg = 1;
    post_event(EVENT_APP_SW_ACTIVE);
    return 0;
}
#endif

#if APP_MODE_USB_EN
void app_usb_loop(void *parm)
{
    int msg[2];
    log_info("usb_app_loop \n");
    usb_start();
    while (1) {
        get_msg(2, &msg[0]);
        bsp_loop();
        if (common_msg_deal(msg) != (-1)) {
            continue;
        }
        USB_MassStorage(NULL);
        switch (msg[0]) {
        case MSG_PC_OUT:
            log_info(" pc out");
            usb_stop();
            app_switch_next();
            return;

        default:
            break;
        }
    }
}
#endif

__attribute__((weak))
u32 get_up_suc_flag(void)
{
    return 0;
}

/* __attribute__((weak)) */
void app(void)
{
#if ENCODER_EN
    extern const char MIC_CAPLESS_EN;
    if (MIC_CAPLESS_EN) {
        ladc_capless_init(30);
    }
#endif
    if (get_up_suc_flag()) {
        log_info("----- device update end ---- \n");
        wdt_close();
#if KEY_VOICE_EN
        d_key_voice_kick();
#endif
        while (1);
    }

    ///init
    app_mg_init(APP_MUSIC, NULL);

    while (1) {
        switch (cur_app) {
#if APP_MODE_MUSIC_EN
        case APP_MUSIC:
            app_music(app_param);
            break;
#endif
#if APP_MODE_RECORD_EN
        case APP_REC:
            app_record(app_param);
            break;
#endif
#if APP_MODE_MIDI_EN
        case APP_MIDI:
            app_midi(app_param);
            break;
#endif
#if APP_MODE_POWEROFF_EN
        case APP_POWEROFF:
            app_poweroff(app_param);
            break;
#endif
#if APP_MODE_LINEIN_EN
        case APP_LINEIN:
            app_linein(app_param);
            break;
#endif
#if APP_MODE_USB_EN
        case APP_USB:
            app_usb_loop(app_param);
            break;
#endif

        default:
            app_printf("app error !!!! unknow app:%d \n", cur_app);
            break;
        }

        app_switch_flg = 0;
    }
}
