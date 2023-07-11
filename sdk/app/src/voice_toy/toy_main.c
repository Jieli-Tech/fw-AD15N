#include "app_config.h"
#include "toy_main.h"
#include "includes.h"
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "errno-base.h"
#include "dac_api.h"
#include "msg.h"
#include "toy_music.h"
#include "toy_idle.h"
#include "toy_midi.h"
#include "toy_midi_ctrl.h"
#include "toy_speaker.h"
#include "toy_linein.h"
#include "toy_record.h"
#include "toy_usb_slave.h"
#include "toy_softoff.h"
#include "vm_api.h"
#include "usb/usr/usb_audio_interface.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

volatile u8 work_mode;

AT(.tick_timer.text.cache.L2)
void tick_timer_ram_loop(void)
{
    /* 该函数2ms回调一次，该函数中所有内容需在ram运行 */
}
void app_timer_loop(void)
{
    static u16 cnt = 0;
    cnt++;
#if TCFG_PC_ENABLE
    if (0 == (cnt % 10)) {
        uac_inc_sync();
    }
    if (0 == (cnt % 500)) {
        /* uac_1s_sync(); */
    }
#endif
    if (cnt >= 1000) {
        cnt = 0;
    }
}

void app_next_mode(void)
{
    work_mode++;
    if (work_mode >= MAX_MODE) {
        work_mode = TOY_MUSIC;
    }
}

__attribute__((weak))
u32 get_up_suc_flag(void)
{
    return 0;
}
#if KEY_IR_EN
bool Sys_IRInput;
u16 Input_Number;
#endif

void app(void)
{
    log_info("voice toy app run!\n");
    if (get_up_suc_flag()) {
        log_info("----device update end----\n");
        wdt_close();
        while (1);
    }

    vm_isr_response_list_register(BIT(IRQ_AUDIO_IDX) | BIT(IRQ_TICKTMR_IP));

    u8 vol = 0;
    u32 res = vm_read(VM_INDEX_VOL, &vol, sizeof(vol));
    if ((vol <= 31) && (res == sizeof(vol))) {
        dac_vol(0, vol);
        log_info("powerup set vol : %d\n", vol);
    }

    work_mode = TOY_MUSIC;
    /* work_mode = TOY_MIDI; */
    /* work_mode = TOY_MIDI_KEYBOARD; */
    /* work_mode = TOY_SPEAKER; */
    /* work_mode = TOY_LINEIN; */
    /* work_mode = TOY_RECORD; */
    /* work_mode = TOY_IDLE; */

    while (1) {
        clear_all_message();
        switch (work_mode) {
#if SIMPLE_DEC_EN
        case TOY_MUSIC:
            toy_music_app();
            break;
#endif
#if defined(DECODER_MIDI_EN) && (DECODER_MIDI_EN)
        case TOY_MIDI:
            toy_midi_app();
            break;
#endif
#if defined(DECODER_MIDI_KEYBOARD_EN) && (DECODER_MIDI_KEYBOARD_EN)
        case TOY_MIDI_KEYBOARD:
            toy_midi_keyboard_app();
            break;
#endif
#if defined(LOUDSPEAKER_EN) && (LOUDSPEAKER_EN)
        case TOY_SPEAKER:
            toy_speaker_app();
            break;
#endif
#if defined(LINEIN_MODE_EN) && (LINEIN_MODE_EN)
        case TOY_LINEIN:
            toy_linein_app();
            break;
#endif
#if defined(RECORD_MODE_EN) && (RECORD_MODE_EN)
        case TOY_RECORD:
            toy_record_app();
            break;
#endif
#if TCFG_PC_ENABLE
        case TOY_USB_SLAVE:
            if (SLAVE_MODE == usb_otg_online(0)) {
                toy_usb_slave_app();
            } else {
                work_mode++;
            }
            break;
#endif
        case TOY_IDLE:
            toy_idle_app();
            break;
        case TOY_SOFTOFF:
            toy_softoff();
            break;
        default:
            work_mode++;
            if (work_mode >= MAX_MODE) {
                work_mode = TOY_MUSIC;
            }
            break;
        }
    }
}

