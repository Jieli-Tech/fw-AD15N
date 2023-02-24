#include "app_config.h"
#include "printf.h"
#if TCFG_PC_ENABLE
#include "usb/usb_config.h"
#include "usb/device/usb_stack.h"
#include "usb/device/uac_audio.h"
#include "usb/usr/usb_audio_interface.h"
#include "uac_stream.h"
#include "usb_config.h"
#include "circular_buf.h"
#include "sound_effect_api.h"

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[UAC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "log.h"
#include "uart.h"
#include "msg.h"
#include "dac_api.h"

#define     UAC_DEBUG_ECHO_MODE 0

sound_out_obj usb_spk_sound;

static volatile u8 speaker_stream_is_open = 0;
struct uac_speaker_handle {
    cbuffer_t cbuf;
    sound_out_obj *sound;
    /* volatile u8 need_resume; */
    /* u8 channel; */
    void *buffer;
    u16 sr;
    u16 in_cnt;
    u8 not_first;
    u8 ch;
    /* void *audio_track; */
    /* void (*rx_handler)(int, void *, int); */
};
#define UAC_BUFFER_SIZE     (2 * 1024)
#define UAC_BUFFER_MAX		(UAC_BUFFER_SIZE * 50 / 100)

static struct uac_speaker_handle *uac_speaker = NULL;
#if USB_MALLOC_ENABLE

#else
static struct uac_speaker_handle uac_speaker_handle SEC(.uac_var);
static u8 uac_rx_buffer[UAC_BUFFER_SIZE] ALIGNED(4) SEC(.uac_rx);
#endif
u32 uac_speaker_stream_length()
{
    return UAC_BUFFER_SIZE;
}
u32 uac_speaker_stream_size()
{
    if (!speaker_stream_is_open) {
        return 0;
    }

    if (uac_speaker) {
        return cbuf_get_data_size(&uac_speaker->cbuf);
    }

    return 0;
}

void uac_speaker_stream_buf_clear(void)
{
    if (speaker_stream_is_open) {
        cbuf_clear(&uac_speaker->cbuf);
    }
}

void set_uac_speaker_rx_handler(void *priv, void (*rx_handler)(int, void *, int))
{
    if (uac_speaker) {
        /* uac_speaker->rx_handler = rx_handler; */
    }
}

void uac_speaker_stream_write(const u8 *obuf, u32 len)
{
    if (speaker_stream_is_open) {
        uac_speaker->in_cnt += len / 2;
        int wlen = sound_output(&usb_spk_sound, (void *)obuf, len);
        if (wlen != len) {
            putchar('W');
        }
        /* if (uac_speaker->rx_handler) { */
        /* if (uac_speaker->cbuf.data_len >= UAC_BUFFER_MAX) { */
        /* // 马上就要满了，赶紧取走 */
        /* [>uac_speaker->need_resume = 1; //2020-12-22注:无需唤醒<] */
        /* } */
        /* if (uac_speaker->need_resume) { */
        /* uac_speaker->need_resume = 0; */
        /* uac_speaker->rx_handler(0, (void *)obuf, len); */
        /* } */
        /* } */
    }
}

u32 uac_speaker_sound(void)
{

    /* *pp_sound = &usb_spk_sound; */
    if (speaker_stream_is_open) {
        return uac_speaker->sr;
    }
    return 0;
}
void uac_speaker_stream_open(u32 samplerate, u32 ch)
{

    log_info(" uac_speaker_stream_open!\n");
    if (speaker_stream_is_open) {
        return;
    }
    log_info("%s", __func__);

    if (!uac_speaker) {

        memset(&uac_speaker_handle, 0, sizeof(struct uac_speaker_handle));
        uac_speaker = &uac_speaker_handle;
        uac_speaker->buffer = uac_rx_buffer;

    }

    cbuf_init(&uac_speaker->cbuf, uac_speaker->buffer, UAC_BUFFER_SIZE);

    memset(&usb_spk_sound, 0, sizeof(usb_spk_sound));
    uac_speaker->sound = &usb_spk_sound;
    uac_speaker->sound->p_obuf = &uac_speaker->cbuf;
    uac_speaker->sr =  samplerate;
    uac_speaker->ch =  ch;

    /* log_info(" event post pc spk!!\n"); */
    usb_slave_sound_open(&usb_spk_sound, uac_speaker->sr);
    post_event(EVENT_PC_SPK);
    speaker_stream_is_open = 1;

    /* struct sys_event event;                             */
    /* event.type = SYS_DEVICE_EVENT;                      */
    /* event.arg = (void *)DEVICE_EVENT_FROM_UAC;          */
    /* event.u.dev.event = USB_AUDIO_PLAY_OPEN;            */
    /* event.u.dev.value = (int)((ch << 24) | samplerate); */

    /* sys_event_notify(&event);                           */

    return;

__err:
    return;
}

void uac_speaker_stream_close()
{
    if (speaker_stream_is_open == 0) {
        return;
    }

    log_info("%s", __func__);
    speaker_stream_is_open = 0;

    if (uac_speaker) {
#if USB_MALLOC_ENABLE
        if (uac_speaker->buffer) {
            free(uac_speaker->buffer);
        }
        free(uac_speaker);
#endif
        usb_slave_sound_close(&usb_spk_sound);
        uac_speaker = NULL;
    }
    /* struct sys_event event;                    */
    /* event.type = SYS_DEVICE_EVENT;             */
    /* event.arg = (void *)DEVICE_EVENT_FROM_UAC; */
    /* event.u.dev.event = USB_AUDIO_PLAY_CLOSE;  */
    /* event.u.dev.value = (int)0;                */

    /* sys_event_notify(&event);                  */
}

u32 uac_speaker_stream_1s(void)
{
    if (0 != speaker_stream_is_open) {
        if (0 != uac_speaker->in_cnt) {
            if (0 == uac_speaker->not_first) {
                uac_speaker->in_cnt = 0;
                uac_speaker->not_first = 1;
                log_info("uac_1s first\n");
            } else {
                u32 res = uac_speaker->in_cnt;
                uac_speaker->in_cnt = 0;
                return res;
            }
        }
    }
    return 0;
}

int __attribute__((weak)) uac_vol_switch(int vol)
{

    u16 valsum = vol * 32 / 100;

    if (valsum > 31) {
        valsum = 31;
    }
    return valsum;
}

int uac_get_spk_vol()
{
    return 0;
}
static u32 mic_stream_is_open;
void uac_mute_volume(u32 type, u32 l_vol, u32 r_vol)
{
    /* struct sys_event event;                    */
    /* event.type = SYS_DEVICE_EVENT;             */
    /* event.arg = (void *)DEVICE_EVENT_FROM_UAC; */

    int l_valsum, r_valsum;
    static u32 last_spk_l_vol = (u32) - 1, last_spk_r_vol = (u32) - 1;
    static u32 last_mic_vol = (u32) - 1;

    switch (type) {
    case MIC_FEATURE_UNIT_ID: //MIC
        /* if (mic_stream_is_open == 0) { */
        /* return ; */
        /* } */
        if (l_vol == last_mic_vol) {
            return;
        }
        last_mic_vol = l_vol;
        l_vol /= 2;
        l_vol += 6;
        /* event.u.dev.event = USB_AUDIO_SET_MIC_VOL; */
        break;
    case SPK_FEATURE_UNIT_ID: //SPK
        /* if (speaker_stream_is_open == 0) { */
        /* return; */
        /* } */
        if (l_vol == last_spk_l_vol && r_vol == last_spk_r_vol) {
            return;
        }
        last_spk_l_vol = l_vol;
        last_spk_r_vol = r_vol;
        /* event.u.dev.event = USB_AUDIO_SET_PLAY_VOL; */
        break;
    default:
        break;
    }
    l_valsum = uac_vol_switch(l_vol);
    r_valsum = uac_vol_switch(r_vol);

#if (SPK_CHANNEL == 1)
    dac_usb_vol(l_valsum, l_valsum);
#elif  (SPK_CHANNEL == 2)
    dac_usb_vol(r_valsum, l_valsum);
#endif

    log_info("type :%d; lvol:%d; rvol:%d;", type, l_valsum, r_valsum);
    /* event.u.dev.value = (int)(r_valsum << 16 | l_valsum); */
    /* sys_event_notify(&event);                             */
}


static const s16 sin_48k[] = {
    0, 2139, 4240, 6270, 8192, 9974, 11585, 12998,
    14189, 15137, 15826, 16244, 16384, 16244, 15826, 15137,
    14189, 12998, 11585, 9974, 8192, 6270, 4240, 2139,
    0, -2139, -4240, -6270, -8192, -9974, -11585, -12998,
    -14189, -15137, -15826, -16244, -16384, -16244, -15826, -15137,
    -14189, -12998, -11585, -9974, -8192, -6270, -4240, -2139
};
static int (*mic_tx_handler)(int, void *, int) SEC(.uac_rx);
int uac_mic_stream_read(u8 *buf, u32 len)
{
    if (mic_stream_is_open == 0) {
        return 0;
    }

#if USB_DEVICE_CLASS_CONFIG & MIC_CLASS
    int rlen = cbuf_read(&cbuf_usb_mic_o, buf, len);
    if (rlen != len) {
        log_info("buf null\n");
    }
    return rlen;
#endif

#if 0//48K 1ksin
#if MIC_CHANNEL == 2
    u16 *l_ch = (u16 *)buf;
    u16 *r_ch = (u16 *)buf;
    r_ch++;
    for (int i = 0; i < len / 4; i++) {
        *l_ch = sin_48k[i];
        *r_ch = sin_48k[i];
        l_ch += 2;
        r_ch += 2;
    }
#else
    memcpy(buf, sin_48k, len);
#endif
    return len;
#elif   UAC_DEBUG_ECHO_MODE
    len = uac_speaker_read(NULL, buf, len);
#if MIC_CHANNEL == 2
    u16 *r_ch = (u16 *)buf;
    r_ch++;
    for (int i = 0; i < len / 4; i++) {
        *r_ch = sin_48k[i];
        r_ch += 2;
    }
#endif
    return len;
#else
    if (mic_tx_handler) {
#if 1
        return mic_tx_handler(0, buf, len);
#else
        //16bit ---> 24bit
        int rlen = mic_tx_handler(0, tmp_buf, len / 3 * 2);
        rlen /= 2; //sampe point
        for (int i = 0 ; i < rlen ; i++) {
            buf[i * 3] = 0;
            buf[i * 3 + 1] = tmp_buf[i * 2];
            buf[i * 3 + 2] = tmp_buf[i * 2 + 1];
        }
#endif
    }
    return 0;
#endif
    return 0;
}

void set_uac_mic_tx_handler(void *priv, int (*tx_handler)(int, void *, int))
{
    mic_tx_handler = tx_handler;
}

u32 uac_mic_stream_open(u32 samplerate, u32 frame_len, u32 ch)
{
    if (mic_stream_is_open) {
        return 0;
    }

    mic_tx_handler = NULL;
    log_info("%s", __func__);

    /* struct sys_event event;                             */
    /* event.type = SYS_DEVICE_EVENT;                      */
    /* event.arg = (void *)DEVICE_EVENT_FROM_UAC;          */
    /* event.u.dev.event = USB_AUDIO_MIC_OPEN;             */
    /* event.u.dev.value = (int)((ch << 24) | samplerate); */
    /* sys_event_notify(&event);                           */
#if USB_DEVICE_CLASS_CONFIG & MIC_CLASS
    usb_mic_init();
#endif
    mic_stream_is_open = 1;
    return 0;
}
void uac_mic_stream_close()
{
    if (mic_stream_is_open == 0) {
        return ;
    }
    log_info("%s", __func__);
    /* struct sys_event event; */
    /* event.type = SYS_DEVICE_EVENT; */
    /* event.arg = (void *)DEVICE_EVENT_FROM_UAC; */
    /* event.u.dev.event = USB_AUDIO_MIC_CLOSE; */
    /* event.u.dev.value = (int)0; */
    /* sys_event_notify(&event); */
    mic_stream_is_open = 0;
#if USB_DEVICE_CLASS_CONFIG & MIC_CLASS
    usb_mic_uninit();
#endif
}
#endif

