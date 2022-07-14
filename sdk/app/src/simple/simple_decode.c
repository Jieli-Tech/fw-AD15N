#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "decoder_api.h"
#include "dev_manage.h"
#include "vfs.h"
#include "audio.h"
#include "dac.h"
#include "ump3_api.h"
#include "midi_api.h"
#include "f1a_api.h"
#include "a_api.h"
#include "msg.h"
#include "bsp_loop.h"
#include "simple_decode.h"
#include "errno-base.h"
#include "sine_play.h"

#if SPEAKER_EN
#include "speak_api.h"
#endif

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"


void *g_pvfs = 0;

dp_buff dp_ctl[4];
static const char *const dir_tab[] = {
    /* "/dir_bin_f1x/", */
    "/dir_song/",
    "/dir_story/",
    "/dir_poetry/",
    "/dir_eng/",
};

static const char *const dir_tab_a[] = {
    "/dir_a/",
};
dec_obj *decoder_by_index(void *pvfs, play_control *ppctl)
{
    u32 err;
    const char *dir;
    decoder_stop(ppctl->p_dec_obj, NO_WAIT);
    if (NULL != ppctl->pdir) {
        dir = ((const char **)ppctl->pdir)[ppctl->dir_index];
        log_info(" %s\n", dir);
        err = vfs_openbypath(pvfs, &ppctl->pfile, dir);
        if (0 != err) {
            return 0;
        }
    }
    err = vfs_openbyindex(pvfs, &ppctl->pfile, ppctl->findex);//SLEEP.lrc
    if (0 != err) {
        /* if(E_FILEINDEX == err) */
        /* { */
        /* post_msg(1, MSG_NEXT_FILE); */
        /* } */
        log_info(" err openbyindex 0x%x,%d\n", err, ppctl->findex);
        return 0;
    }
    return decoder_io(ppctl->pfile, ppctl->type, &dp_ctl[ppctl->dir_index], ppctl->loop);
}

play_control pctl[2];
#define ALL_DECODER_CHANNEL (sizeof(pctl)/sizeof(play_control));
extern void simple_next(void);

void print_audio_sfr(void);

void print_audio_sfr(void)
{
    /* log_info(" JL_ADDA->DAC_CON0    0x%x;",   JL_ADDA->DAC_CON0); */
    /* log_info(" JL_ADDA->DAC_CON1    0x%x;",   JL_ADDA->DAC_CON1); */
    /* log_info(" JL_ADDA->DAC_TRM     0x%x;",   JL_ADDA->DAC_TRM); */
    /* log_info(" JL_ADDA->DAC_ADR     0x%x;",   JL_ADDA->DAC_ADR); */
    /* log_info(" JL_ADDA->DAC_LEN     0x%x;",   JL_ADDA->DAC_LEN); */
    /* log_info(" JL_ADDA->DAC_COP     0x%x;",   JL_ADDA->DAC_COP); */
    /* log_info(" JL_ADDA->DAC_DTB     0x%x;",   JL_ADDA->DAC_DTB); */
    /* log_info(" JL_ADDA->ADC_CON     0x%x;",   JL_ADDA->ADC_CON); */
    /* log_info(" JL_ADDA->ADC_ADR     0x%x;",   JL_ADDA->ADC_ADR); */
    /* log_info(" JL_ADDA->ADC_LEN     0x%x;",   JL_ADDA->ADC_LEN); */
    /* log_info(" JL_ADDA->RAM_SPD     0x%x;",   JL_ADDA->RAM_SPD); */
    /* log_info(" JL_ADDA->DAA_CON0    0x%x;",   JL_ADDA->DAA_CON0); */
    /* log_info(" JL_ADDA->DAA_CON1    0x%x;",   JL_ADDA->DAA_CON1); */
    /* log_info(" JL_ADDA->ADA_CON0    0x%x;",   JL_ADDA->ADA_CON0); */
    /* log_info(" JL_ADDA->ADA_CON1    0x%x;",   JL_ADDA->ADA_CON1); */
    /* log_info(" JL_ADDA->ADA_CON2    0x%x;\n",   JL_ADDA->ADA_CON2); */
}

void decoder_demo(void)
{
    log_info("decoder_demo\n");
    dac_sr_api(SR_DEFAULT);
    /* dac_init_api(32000); */
    u32 i, err;
    u32 dir_index;
    log_info("decoder_demo 001\n");
    memset(&pctl[0], 0, sizeof(pctl));

    pctl[0].findex = 1;                 //需要播放的文件号
    pctl[0].dir_index = 0;
    /* pctl[0].type = BIT_UMP3 | BIT_F1A1;  //播放需要使用的解码器 */
    pctl[0].type = BIT_F1A1;  //播放需要使用的解码器
    pctl[0].pdir = (void *)&dir_tab[0]; //播放的路径
    pctl[0].loop = 0;                   //0 ~ 254无缝循环次数；255一直无缝循环播放

    pctl[1].findex = 1;
    pctl[1].type = BIT_A ;
    pctl[1].pdir = (void *)&dir_tab_a[0];

    err = vfs_mount(&g_pvfs, (void *)NULL, (void *)NULL);
    if (0 != err) {
        log_info(" err 0x%x\n", err);
    }
    char c = 0;
    /* while ('s' != c) { */
    /* c = get_byte(); */
    /* wdt_clear(); */
    /* } */
    /* c = 0; */

    log_info("decoder_demo 002\n");
    err = post_msg(1, MSG_PLAY_FILE1);
    post_msg(1, MSG_PLAY_FILE2);
    log_info("decoder_demo 003\n");

    int msg[2];
    /* msg[0] : (len << 12) |(msg& 0x0fff) */
    /* msg[1] : para */
    while (1) {
        /* log_char('a'); */
        err = get_msg(2, &msg[0]);
        bsp_loop();

        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
            log_info("get msg err 0x%x\n", err);
        }

        if (NO_MSG == msg[0]) {
            c = get_byte();
            if (0 != c) {
                log_char(c);
                d_key_voice_kick();
            }
            if ('N' == c) {
                decoder_stop(pctl[0].p_dec_obj, NO_WAIT);
                decoder_stop(pctl[1].p_dec_obj, NO_WAIT);
                simple_next();
                return ;
            }
            if ('p' == c) {
                msg[0] = MSG_PP;
            } else if ('n' == c) {
                msg[0] = MSG_NEXT_FILE;
            } else if ('P' == c) {
                msg[0] = MSG_PP_2;
            } else if ('D' == c) {
                msg[0] = MSG_NEXT_DIR;
            } else if ('+' == c) {
                msg[0] = MSG_VOL_UP;
            } else if ('-' == c) {
                msg[0] = MSG_VOL_DOWN;
            } else if ('s' == c) {
                msg[0] = MSG_PLAY_FILE1;
            } else if ('t' == c) {
                /* aux_test_audio(); */
            }
#if SPEAKER_EN
            else if ('o' == c) {
                audio_adc_speaker_start();
            } else if ('c' == c) {
                audio_adc_speaker_reless();
            }
#endif
        } else {
            /* log_info(" msg :0x%x\n",msg[0]); */
        }
        switch (msg[0]) {
        case MSG_500MS:
            wdt_clear();
            /* print_audio_sfr(); */
            //模拟量的调节,电压不要急升
            audio_lookup();
            /* log_char('5'); */
            break;
        case MSG_VOL_UP:
            dac_vol('+', 255);
            break;
        case MSG_VOL_DOWN:
            dac_vol('-', 255);
            break;
        case MSG_PP:
            log_info("\n msg pp\n");
            decoder_pause(pctl[0].p_dec_obj);
            break;

        case MSG_PLAY_FILE1:
            log_info("\n/************************************************/\n");
            pctl[0].p_dec_obj = decoder_by_index(g_pvfs, &pctl[0]);
            if (0 == pctl[0].p_dec_obj) {
                post_msg(1, MSG_NEXT_FILE);
                break;
            }
            dp_ctl[pctl[0].dir_index].findex = pctl[0].findex;
            vfs_ioctl(pctl[0].pfile, FS_IOCTL_DIR_FILE_TOTAL, (int)&pctl[0].ftotal);
            log_info(" file_total : %d\n", pctl[0].ftotal);
            break;

        case MSG_F1A1_FILE_END:
        //case MSG_F1A2_FILE_END:
        case MSG_MP3_FILE_END:
        case MSG_WAV_FILE_END:
            /* log_info("file err end!!!\n"); */
            decoder_stop(pctl[0].p_dec_obj, NEED_WAIT);
            goto __1_next_file;

        case MSG_F1A1_FILE_ERR:
        //case MSG_F1A2_FILE_ERR:
        case MSG_MP3_FILE_ERR:
        case MSG_WAV_FILE_ERR:
        case MSG_NEXT_FILE:
            log_info("\n next file\n");
__1_next_file:
            pctl[0].findex++;
            if (pctl[0].findex > pctl[0].ftotal) {
                pctl[0].findex = 1;
            }
            log_info(" file_index : %d\n", pctl[0].findex);
            post_msg(1, MSG_PLAY_FILE1);
            break;
        case MSG_NEXT_DIR:
            log_info("\n next dir\n");

            /* decoder_stop(pctl[0].p_dec_obj, NO_WAIT); */
            get_dp(pctl[0].p_dec_obj, &dp_ctl[pctl[0].dir_index]);
            pctl[0].p_dec_obj->p_dp_buf = 0 ;
            /* decoder_stop(pctl[0].p_dec_obj, NO_WAIT); */
            pctl[0].dir_index++;
            if (pctl[0].dir_index >= (sizeof(dir_tab) / 4)) {
                pctl[0].dir_index = 0;
            }
            pctl[0].findex = dp_ctl[pctl[0].dir_index].findex;
            post_msg(1, MSG_PLAY_FILE1);
            break;
        case MSG_WAV_LOOP:
        case MSG_F1A1_LOOP:
        case MSG_F1A2_LOOP:
        case MSG_MP3_LOOP:
        case MSG_A_LOOP:
            log_info(" -msg loop!!!,%d\n", pctl[0].p_dec_obj->loop);
            break;
        /********************************************************************/
        case MSG_PLAY_FILE2:
            log_info("/************************************************/\n");
            pctl[1].p_dec_obj = decoder_by_index(g_pvfs, &pctl[1]);
            break;
        case MSG_PP_2:
            log_info(" msg pp\n");
            decoder_pause(pctl[1].p_dec_obj);
            break;
        case MSG_A_FILE_END:
        case MSG_A_FILE_ERR:
            log_info(" A err or end\n");
            decoder_stop(pctl[1].p_dec_obj, NEED_WAIT);
            post_msg(1, MSG_PLAY_FILE2);
            break;
        case MSG_PC_IN:
            /* log_info("simple_decode pc in\n"); */
            break;
        }
    }
}










#if 0

bool check_dec_msg(dec_obj *pd_obj, MAD_INFO m_info, int msg)
{
    u32 event, t_msg;
    if ((NULL == pd_obj) || (NULL == pd_obj->event_tab)) {
        return false;
    }
    event = pd_obj->event_tab[m_info & 0x0f];
    t_msg = event2msg_api(event);
    if (msg == t_msg) {
        return true;
    }
    return false;
}

case MSG_F1A1_FILE_END:
case MSG_F1A2_FILE_END:
case MSG_MIDI_FILE_END:
case MSG_A_FILE_END:
case MSG_MP3_FILE_END:
case MSG_WAV_FILE_END:
for (i = 0 ; i < ALL_DECODER_CHANNEL; i++)
{
    if (check_dec_msg(pctl[i].p_dec_obj, MAD_ERROR_FILE_END, msg[0])) {
        decoder_stop(pctl[i].p_dec_obj, NEED_WAIT);
    }
}
break;
case MSG_F1A1_FILE_ERR:
case MSG_F1A2_FILE_ERR:
case MSG_MIDI_FILE_ERR:
case MSG_A_FILE_ERR:
case MSG_MP3_FILE_ERR:
case MSG_WAV_FILE_ERR:
for (i = 0 ; i < ALL_DECODER_CHANNEL; i++)
{
    if (check_dec_msg(pctl[i].p_dec_obj, MAD_ERROR_SYNC_LIMIT, msg[0])) {
        decoder_stop(pctl[i].p_dec_obj, NEED_WAIT);
    }
}
break;
#endif


