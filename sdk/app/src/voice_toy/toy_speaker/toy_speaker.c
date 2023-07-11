#include "toy_speaker.h"
#include "common.h"
#include "msg.h"
/* #include "circular_buf.h" */

#include "dac_api.h"
#include "audio_adc.h"
#include "toy_main.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[toy_spk]"
#include "log.h"
#if defined(LOUDSPEAKER_EN) && (LOUDSPEAKER_EN)
#define LOUDSPK_SR  24000

static void pa_unmute(void)
{
    JL_PORTA->OUT &= ~BIT(8);
    JL_PORTA->PU &= ~BIT(8);
    JL_PORTA->PD &= ~BIT(8);
    JL_PORTA->DIR &= ~BIT(8);
}

static void pa_mute(void)
{
    JL_PORTA->OUT |= BIT(8);
    JL_PORTA->PU &= ~BIT(8);
    JL_PORTA->PD &= ~BIT(8);
    JL_PORTA->DIR &= ~BIT(8);
}
void toy_speaker_app(void)
{
    key_table_sel(toy_speaker_key_msg_filter);

    u32 dac_sr = dac_sr_read();
    dac_sr_api(LOUDSPK_SR);
    audio_adc_speaker_start();

    int msg[2];
    u32 err;
    u8 mute = 0;
    while (1) {
        err = get_msg(2, &msg[0]);
        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
            log_info("get msg err 0x%x\n", err);
        }

        switch (msg[0]) {
        case MSG_PP:
            if (mute == 0) {
                log_info("MUTE\n");
                mute = 1;
                dac_mute(1);
                /* pa_mute(); */
            } else {
                log_info("UNMUTE\n");
                mute = 0;
                dac_mute(0);
                /* pa_unmute(); */
            }
            break;
        case MSG_CHANGE_WORK_MODE:
            goto __loudspk_app_exit;
        case MSG_500MS:
        default:
            common_msg_deal(&msg[0]);
            break;
        }
    }
__loudspk_app_exit:
    if (0 != mute) {
        dac_mute(0);
        /* pa_mute(); */
    }
    audio_adc_speaker_reless();
    dac_sr_api(dac_sr);
    key_table_sel(NULL);
}
#endif

