#include "typedef.h"
#include "rec_mode.h"
#include "msg.h"
#include "errno-base.h"
#include "sdmmc/sd_host_api.h"
#include "rec_api.h"
#include "vfs.h"
#include "music/device.h"
#include "music/get_music_file.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

void *rec_find_device(void)
{
    u8 i;
    void *hdl = NULL;

    /*Maskrom File by number*/
    //fs_getfile_bynumber(&ff_api,ff_api.fs_msg.fileNumber);

    device_check();

    i = mbox_get_one_count(device_online & 0x07);

    if (i == 0) {			//无设备
        device_active = NO_DEVICE;
        return NULL;
    }

    if (device_active != NO_DEVICE) {
        playfile.given_device = device_active;
        if (BIT(device_active) & device_online) {
            hdl = device_get_active_hdl();
            if (hdl) {
                return hdl;
            }
        }
    }

    for (i = 0; i <= MAX_DEVICE; i++) {
        device_active++;
        if (device_active > MAX_DEVICE) {
            device_active = DEVICE_UDISK;
        }

        hdl = device_get_active_hdl();
        if (hdl) {
            playfile.given_device = device_active;
            return hdl;
        }
    }

    device_active = NO_DEVICE;
    return NULL;							//无有效可以使用的设备
}

