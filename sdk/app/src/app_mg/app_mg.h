#ifndef _APP_MG_H
#define _APP_MG_H
#include "key.h"
#include "bsp_loop.h"

#define APP_UART_DEBUG

#ifdef APP_UART_DEBUG
#define app_puts           log_info
#define app_printf         log_info
#define app_buf            log_info_hexdump
#define app_err_printf	   log_info
#else
#define app_puts(...)
#define app_printf(...)
#define app_buf(...)
#define app_err_printf
#endif

#if DECODER_MIDI_EN
#define APP_MODE_MIDI_EN			1
#else
#define APP_MODE_MIDI_EN			0
#endif

#if DECODER_MIDI_KEYBOARD_EN
#define APP_MODE_MIDI_KEYBOARD_EN   1
#else
#define APP_MODE_MIDI_KEYBOARD_EN   0
#endif

#define APP_MODE_MUSIC_EN			1
#if ENCODER_EN
#define APP_MODE_RECORD_EN			1
#else
#define APP_MODE_RECORD_EN			0
#endif
#define APP_MODE_POWEROFF_EN		1
#if AUX_EN
#define APP_MODE_LINEIN_EN			1
#else
#define APP_MODE_LINEIN_EN			0
#endif
#ifdef USB_DEVICE_EN
#define APP_MODE_USB_EN  			1
#else
#define APP_MODE_USB_EN			0
#endif

typedef enum {
    APP_FIRST_RESERVE = 0,

#if APP_MODE_MUSIC_EN
    APP_MUSIC,
#endif
#if APP_MODE_RECORD_EN
    APP_REC,
#endif
#if APP_MODE_MIDI_EN
    APP_MIDI,
#endif
#if APP_MODE_MIDI_KEYBOARD_EN
    APP_MIDI_KEYBOARD,
#endif
#if APP_MODE_POWEROFF_EN
    APP_POWEROFF,
#endif
#if APP_MODE_LINEIN_EN
    APP_LINEIN,
#endif
#if APP_MODE_USB_EN
    APP_USB,
#endif

    APP_MAX,
} APP_T;

int common_msg_deal(int *msg);
int app_switch_priv(void);
int app_switch_next(void);
int app_switch(APP_T app, void *param);
u8 app_get(void);
bool app_switch_en(void);
void app(void);
void idle_check_tick(void);
void idle_check_deal(u8 is_busy);
int app_switch_to_usb(void);
#endif
