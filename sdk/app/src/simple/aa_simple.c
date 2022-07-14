#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "errno-base.h"
#include "app_config.h"
#include "audio.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"

#include "simple_encode.h"
#include "simple_decode.h"
#include "simple_aux.h"
#include "simple_usb_slave.h"
#include "usb/usr/usb_audio_interface.h"


void uac_1s_sync(void);
void app_timer_loop(void)
{
#ifdef USB_DEVICE_EN
    static u16 cnt = 0;
    cnt++;
    if (0 == (cnt % 10)) {
        uac_inc_sync();
    }
    if (cnt >= 500) {
        uac_1s_sync();
        cnt = 0;
    }
#endif
}
enum {
    SIM_DEC = 0,
    SIM_AUX,
    SIM_ENC,
    SIM_USB_S,
};

static u8 sys_mode;
void simple_next(void)
{
    if (SIM_DEC == sys_mode) {
        sys_mode = SIM_AUX;
    } else {
        sys_mode = SIM_DEC;
    }
    return;
    sys_mode++;
    if (sys_mode > SIM_AUX) {
        sys_mode = 0;
    }
}

/* void test_openfile_by_file(void); */
void app(void)
{
    log_init(1000000);
#if ENCODER_EN
    ladc_capless_init(30);
#endif
    /* sys_mode = SIM_AUX; */
    sys_mode = SIM_DEC;
    log_info("app run\n");

    /* test_openfile_by_file(); */

    while (1) {
        switch (sys_mode) {
        case SIM_DEC:
            log_info("simple decoder_init\n");
            decoder_init();
            log_info("simple decoder_demo\n");
            decoder_demo();
            break;
#if ENCODER_EN
        case SIM_ENC:
            encode_demo();
            break;
#endif
#ifdef USB_DEVICE_EN
        case SIM_USB_S:
            usb_slave_demo();
            break;
#endif
#if AUX_EN
        case SIM_AUX:
            aux_demo();
            break;
#endif
        default:
            sys_mode++;
            if (sys_mode > SIM_AUX) {
                sys_mode = 0;
            }
            break;
        }
    }
}

#if 0
#include "vfs.h"
char t_fbf_buf[32];
void test_openfile_by_file(void)
{
    void *pvfs = NULL;
    void *pvfile = NULL;
    void *pdevice = NULL;
    u32 err;
    log_info(" %s %d\n", __func__, __LINE__);
    err = vfs_mount(&pvfs, pdevice, (void *)NULL);
    if (0 != err) {
        goto __tofbf_end;
    }
    log_info("vfs_mount succ\n");
    err = vfs_openbypath(pvfs, &pvfile, "/sfile_dir/Layer3.mp3");
    if (0 != err) {
        goto __tofbf_end;
    }
    log_info("vfs_openbypath succ\n");
    void *pvfile_ext = NULL;
    err = vfs_openbyfile(pvfile, &pvfile_ext, "mio");
    if (0 != err) {
        goto __tofbf_end;
    }
    log_info("vfs_ioctl succ\n");

    memset(&t_fbf_buf[0], 0, sizeof(t_fbf_buf));

    u32 rlen = vfs_read(pvfile_ext, &t_fbf_buf[0], 16);
    log_info("%d %s", rlen, &t_fbf_buf[0]);
    log_info_hexdump((u8 *)&t_fbf_buf[0], 32);
__tofbf_end:
    log_info(" err 0x%x\n", err);
    while (1) {
        wdt_clear();

    }
}
#endif



