#include "music/music_play.h"
#include "vfs.h"
#include "music/music_play.h"
#include "msg.h"
#include "music/device.h"
#include "music/get_music_file.h"
#include "common/ui/ui_api.h"
#include "common/mbox_common.h"
#include "mbox_main.h"
#include "vm.h"
#include "line_in/line_in_mode.h"
#include "fm/fm_radio.h"
#include "usb_device/usb_device_mode.h"
#include "common/hot_msg.h"
#include "usb/usr/usb_audio_interface.h"
#include "common/pa_mute.h"
#include "clock.h"
#include "rec/rec_mode.h"
#include "decoder_api.h"
#include "efuse.h"
#include "audio.h"
#if KEY_VOICE_EN
#include "sine_play.h"
#endif
#include "loudspeaker/loudspeaker_mode.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"


ENUM_WORK_MODE work_mode;
bool Sys_Volume;
bool Sys_IRInput;
bool Sys_HalfSec = 0;

static void mbox_sys_init(void)
{
    vm_read(VM_INDEX_SYSMODE, (u8 *)&work_mode, sizeof(work_mode));
    if (work_mode >= MAX_WORK_MODE) {
        work_mode = MUSIC_MODE;
        vm_write(VM_INDEX_SYSMODE, (u8 *)&work_mode, sizeof(work_mode));
    }
    playfile.given_device = NO_DEVICE;
}

void work_mode_save(void)
{
    vm_write(VM_INDEX_SYSMODE, (u8 *)&work_mode, sizeof(work_mode));
}

AT(.ram_led)
void tick_timer_ram_loop(void)
{
#if LED_5X7
    LED5X7_scan();
#endif
}

void app_timer_loop(void)
{
    static u16 cnt = 0;
    cnt ++;

    if (0 == (cnt % 25)) { //50ms
#ifdef LINEIN_EN
        line_in_det();
#endif
    }

    if (0 == (cnt % 250)) { //500ms
        Sys_HalfSec = !Sys_HalfSec;
    }

#ifdef USB_DEVICE_EN
    if (0 == (cnt % 10)) {
        uac_inc_sync();
    }

    if (0 == (cnt % 500)) {
        uac_1s_sync();
    }
#endif

    if (cnt >= 1000) {
        cnt = 0;
    }

}

u32 sdx_dev_deal_suspend_event_cbfun(u32 event)
{
    if (event & BIT(SDX_SUSPEND_EVENT_LINEIN)) {
        void line_in_det1(void);
        line_in_det1();
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/** @brief: 系统主循环
    @param: void
    @return:void
    @author:
    @note:  void main(void)
*/
/*----------------------------------------------------------------------------*/
void mbox_main(void)
{
#if TCFG_USB_EXFLASH_UDISK_ENABLE
    post_event(EVENT_EXTFLSH_IN);
#endif

#if ENCODER_EN
    extern const char MIC_CAPLESS_EN;
    if (MIC_CAPLESS_EN) {
        ladc_capless_init(30);
    }
    delay_10ms(20);//等待系统稳定。
#else
    delay_10ms(50);//等待系统稳定。
#endif

    vm_isr_response_list_register(BIT(IRQ_TICKTMR_IDX) | BIT(IRQ_AUDIO_IDX));
    pa_unmute();
#if LED_5X7
    LED5X7_init();
#endif
    mbox_sys_init();
    decoder_init();
    if (get_up_suc_flag()) {
        log_info("----- device update end ---- \n");
        wdt_close();
#if KEY_VOICE_EN
        d_key_voice_kick();
#endif
        while (1);
    }

    while (1) {
        __builtin_pi32_idle();
        flush_all_msg();
        switch (work_mode) {
        case MUSIC_MODE:
            log_info("-Music Mode\n");
            music_app();
            break;

#ifdef USB_DEVICE_EN
        case USB_DEVICE_MODE:
            log_info("-PC Mode\n");
            USB_device_mode();
            break;
#endif

#ifdef FM_ENABLE
        case FM_RADIO_MODE:
            log_info("-FM Mode\n");
            fm_mode();
            break;
#endif

#ifdef REC_ENABLE
        case REC_MODE:
            log_info("-REC Mode\n");
            rec_mode();
            break;
#endif

#ifdef LINEIN_EN
        case AUX_MODE:
            log_info("-AUX Mode\n");
            Line_in_mode();
            break;
#endif

#ifdef LOUDSPEAKER_EN
        case LOUDSPEAKER_MODE:
            log_info("-LOUDSPEAKER Mode\n");
            loudspeaker_mode();
            break;
#endif

        default:
            work_mode = MUSIC_MODE;
            break;
        }
    }
}

void app(void)
{
    mbox_main();
}
