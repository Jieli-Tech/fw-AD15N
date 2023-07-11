#include "toy_softoff.h"
#include "toy_main.h"
#include "common.h"
#include "msg.h"
#include "circular_buf.h"
#include "jiffies.h"
#include "device.h"
#include "vfs.h"
#include "toy_main.h"
#include "power_api.h"
#include "decoder_api.h"
#include "decoder_msg_tab.h"
#include "audio.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[low_pwr]"
#include "log.h"

void toy_softoff(void)
{
    int msg[2], err;
    void *pfs = 0;
    void *pfile = 0;
    dec_obj *obj = 0;
    decoder_init();

    err = vfs_mount(&pfs, (void *)NULL, (void *)NULL);
    if (err) {
        log_info("vfs_mount err : 0x%x\n", err);
        goto __softoff_app_deal;
    }
    err = vfs_openbypath(pfs, &pfile, "dir_notice/power_off.f1a");
    if (err) {
        log_info("fs_openbypath err : 0x%x\n", err);
        vfs_fs_close(&pfs);
        goto __softoff_app_deal;
    }
    obj = decoder_io(pfile, BIT_F1A1 | BIT_A, NULL, 0);
    if (NULL == obj) {
        log_info("fs_openbypath err : 0x%x\n", err);
        vfs_file_close(&pfile);
        vfs_fs_close(&pfs);
        goto __softoff_app_deal;
    }

    while (1) {
        err = get_msg(2, &msg[0]);
        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
            log_info("get msg err 0x%x\n", err);
        }

        switch (msg[0]) {
        case MSG_F1A1_FILE_END:
        case MSG_MP3_FILE_END:
        case MSG_WAV_FILE_END:
            decoder_stop(obj, NO_WAIT);
            vfs_file_close(&pfile);
            vfs_fs_close(&pfs);
__softoff_app_deal:
            sys_softoff();
            break;
        case MSG_500MS:
            wdt_clear();
            break;
        }
    }
}
