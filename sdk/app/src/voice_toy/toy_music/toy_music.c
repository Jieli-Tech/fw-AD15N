#include "toy_music.h"
#include "app_modules.h"
#include "simple_play_file.h"
#include "common.h"
#include "msg.h"
#include "key.h"
#include "toy_main.h"
#include "vfs.h"
#include "vm_api.h"
#include "circular_buf.h"
#include "jiffies.h"
#include "tick_timer_driver.h"
#include "device_mge.h"
#include "bsp_loop.h"

#include "decoder_api.h"
#include "decoder_msg_tab.h"
#include "dac_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[toy_music]"
#include "log.h"

#define TFG_EXT_FLASH_EN        0

#if SIMPLE_DEC_EN

static play_control dec_pctl[2] AT(.toy_music_data);

#define INR_DIR_NUM   5
static dp_buff inr_dec_dp[INR_DIR_NUM] AT(.toy_music_data);
static const char *const dir_inr_tab[INR_DIR_NUM] = {
    "/dir_song",
    "/dir_eng",
    "/dir_poetry",
    "/dir_story",
    "/dir_bin_f1x",
};
static const u8 dir_inr_vm_tab[INR_DIR_NUM] = {
    VM_INDEX_SONG,
    VM_INDEX_ENG,
    VM_INDEX_POETRY,
    VM_INDEX_STORY,
    VM_INDEX_F1X,
};
static const char *const dir_tab_a[] = {
    "/dir_a",
};

#if TFG_EXT_FLASH_EN
#define EXT_DIR_NUM   1
static dp_buff ext_dec_dp[EXT_DIR_NUM] AT(.toy_music_data);
static const char *const dir_ext_tab[EXT_DIR_NUM] = {
    "/",
};
static const char dir_ext_vm_tab[EXT_DIR_NUM] = {
    VM_INDEX_EXT_SONG,
};
#endif

void toy_music_app(void)
{
#if KEY_IR_EN
    Sys_IRInput = 1;
#endif
    int msg[2], err;
    key_table_sel(music_msg_filter);
    decoder_init();

    memset(&dec_pctl[0], 0, sizeof(dec_pctl));      //初始化dec_pctl[0]和dec_pctl[1]
    memset(&inr_dec_dp[0], 0, sizeof(inr_dec_dp));  //初始化dec_dp[0]和dec_dp[1]
#if TFG_EXT_FLASH_EN
    memset(&ext_dec_dp[0], 0, sizeof(ext_dec_dp));
#endif

    dec_pctl[0].dev_index   = INNER_FLASH_RO;
    dec_pctl[0].findex      = 1;
    dec_pctl[0].loop        = 0;
    dec_pctl[0].dec_type    = BIT_F1A1 | BIT_UMP3;
    dec_pctl[0].pdp         = &inr_dec_dp[0];
    dec_pctl[0].p_vm_tab    = (void *)&dir_inr_vm_tab[0];
    dec_pctl[0].pdir        = (void *)&dir_inr_tab[0];
    dec_pctl[0].dir_total   = sizeof(dir_inr_tab) / 4;
    simple_dev_fs_mount(&dec_pctl[0]);
#if SIMPLE_DEC_BP_ENABLE
    /* 读取断点信息 */
    if (NULL != dec_pctl[0].pdp) {
        if (sizeof(dp_buff) == vm_read(\
                                       dec_pctl[0].p_vm_tab[dec_pctl[0].dir_index], \
                                       dec_pctl[0].pdp,                            \
                                       sizeof(dp_buff))) {
            dp_buff *pdp = (dp_buff *)dec_pctl[0].pdp;
            dec_pctl[0].findex = pdp->findex;
        }
    }
#endif

    dec_pctl[1].dev_index   = INNER_FLASH_RO;
    dec_pctl[1].findex      = 1;
    dec_pctl[1].loop        = 0;//A格式无缝循环不需要断点buff
    dec_pctl[1].dec_type    = BIT_A;
    dec_pctl[1].pdir        = (void *)&dir_tab_a[0];
    dec_pctl[1].dir_total   = sizeof(dir_tab_a) / 4;
    simple_dev_fs_mount(&dec_pctl[1]);

    /* dec_pctl[2].dev_index   = INNER_FLASH_RO; */
    /* dec_pctl[2].findex      = 1; */
    /* dec_pctl[2].dec_type    = BIT_F1A2; */
    /* dec_pctl[2].pdir        = (void *)&dir_inr_tab[0]; */
    /* dec_pctl[2].dir_total   = sizeof(dir_inr_tab) / 4; */
    /* simple_dev_fs_mount(&dec_pctl[2]); */

    post_msg(1, MSG_PLAY_FILE1);
    /* post_msg(1, MSG_PLAY_FILE2); */
    /* post_msg(1, MSG_A_PLAY); */
    /* simple_play_file_bypath(&dec_pctl[0], "/dir_song/so002.f1b"); */

    while (1) {
        err = get_msg(2, &msg[0]);
        bsp_loop();
        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
            log_info("get msg err 0x%x\n", err);
        }

        switch (msg[0]) {
        case MSG_PLAY_FILE1:
            log_info("MSG_PLAY_FILE1\n");
            err = play_one_file(&dec_pctl[0]);
            if (err) {
                play_next_file(&dec_pctl[0]);
            }
            break;

        case MSG_PP:
            log_info("MSG_PP\n");
            decoder_pause(dec_pctl[0].p_dec_obj);
            break;
        case MSG_PREV_FILE:
            log_info("MSG_PREV_FILE\n");
            play_prev_file(&dec_pctl[0]);
            break;
        case MSG_NEXT_FILE:
            log_info("MSG_NEXT_FILE\n");
            play_next_file(&dec_pctl[0]);
            break;

        case MSG_NEXT_DIR:
            log_info("MSG_NEXT_DIR\n");
            simple_next_dir(&dec_pctl[0]);
            break;

#if TFG_EXT_FLASH_EN
        case MSG_NEXT_DEVICE:
            simple_switch_device(&dec_pctl[0]);
            break;
#endif
        case MSG_F1A1_FILE_ERR:
        case MSG_MP3_FILE_ERR:
        case MSG_WAV_FILE_ERR:
        case MSG_F1A1_FILE_END:
        case MSG_MP3_FILE_END:
        case MSG_WAV_FILE_END:
            log_info("FILE END OR ERR\n");
            decoder_stop(dec_pctl[0].p_dec_obj, NEED_WAIT);
            play_next_file(&dec_pctl[0]);
            break;

        case MSG_A_PLAY:
            log_info("MSG_A_PLAY\n");
            play_one_file(&dec_pctl[1]);
            break;
        case MSG_A_FILE_END:
        case MSG_A_FILE_ERR:
            log_info("A FILE END OR ERR\n");
            decoder_stop(dec_pctl[1].p_dec_obj, NEED_WAIT);
            break;

        /* case MSG_PLAY_FILE2: */
        /*     log_info("MSG_PLAY_F1A2\n"); */
        /*     play_one_file(&dec_pctl[2]); */
        /*     break; */
        /* case MSG_F1A2_FILE_END: */
        /* case MSG_F1A2_FILE_ERR: */
        /*     log_info("F1A2 FILE END OR ERR\n"); */
        /*     decoder_stop(dec_pctl[2].p_dec_obj, NEED_WAIT); */
        /*     break; */

        case MSG_F1A1_LOOP:
        case MSG_F1A2_LOOP:
        case MSG_MP3_LOOP:
        case MSG_WAV_LOOP:
        case MSG_A_LOOP:
            log_info("-loop\n");
            break;

        case MSG_CHANGE_WORK_MODE:
            goto __toy_music_exit;
        case MSG_500MS:
            if ((MUSIC_PLAY != get_decoder_status(dec_pctl[0].p_dec_obj)) && \
                (MUSIC_PLAY != get_decoder_status(dec_pctl[1].p_dec_obj))) {
                vm_pre_erase();
                sys_idle_deal(-2);
            }
        default:
            common_msg_deal(&msg[0]);
            break;
        }
    }
__toy_music_exit:
    decoder_stop(dec_pctl[0].p_dec_obj, NEED_WAIT);
#if SIMPLE_DEC_BP_ENABLE
    if (true == get_dp(dec_pctl[0].p_dec_obj, dec_pctl[0].pdp)) {
        vm_write(\
                 dec_pctl[0].p_vm_tab[dec_pctl[0].dir_index], \
                 dec_pctl[0].pdp,                            \
                 sizeof(dp_buff));
    }
#endif
    simple_dev_fs_close(&dec_pctl[0]);
    decoder_stop(dec_pctl[1].p_dec_obj, NEED_WAIT);
    simple_dev_fs_close(&dec_pctl[1]);
#if KEY_IR_EN
    Sys_IRInput = 0;
#endif
    key_table_sel(NULL);
}



#if TFG_EXT_FLASH_EN
static u32 simple_switch_device(play_control *ppctl)
{
    decoder_stop(ppctl->p_dec_obj, NEED_WAIT);//记录音乐断点
    if (NULL != ppctl->pdp) {
        if (true == get_dp(ppctl->p_dec_obj, ppctl->pdp)) {
#if SIMPLE_DEC_BP_ENABLE
            vm_write(ppctl->p_vm_tab[ppctl->dir_index], ppctl->pdp, sizeof(dp_buff));
#endif
        }
    }
    simple_dev_fs_close(ppctl);
    if (EXT_FLASH_RW == ppctl->dev_index) {
        log_info("SWITCH TO INNER_FLASH\n");
        memset(ppctl, 0, sizeof(play_control));
        ppctl->dev_index   = INNER_FLASH_RO;
        ppctl->findex      = 1;
        ppctl->dec_type    = BIT_F1A1 | BIT_UMP3;
        ppctl->pdp         = &inr_dec_dp[ppctl->dir_index];
        ppctl->p_vm_tab    = (void *)&dir_inr_vm_tab[0];
        ppctl->pdir        = (void *)&dir_inr_tab[0];
        ppctl->dir_total   = sizeof(dir_inr_tab) / 4;
    } else {
        log_info("SWITCH TO EXT_FLASH\n");
        memset(ppctl, 0, sizeof(play_control));
        ppctl->dev_index   = EXT_FLASH_RW;
        ppctl->findex      = 1;
        ppctl->dec_type    = BIT_F1A1 | BIT_UMP3;
        ppctl->pdp         = &ext_dec_dp[ppctl->dir_index];
        ppctl->p_vm_tab    = (void *)&dir_ext_vm_tab[0];
        ppctl->pdir        = (void *)&dir_ext_tab[0];
        ppctl->dir_total   = sizeof(dir_ext_tab) / 4;
    }
    if (simple_dev_fs_mount(ppctl)) {
        log_error("play next dev err!\n");
        return false;
    }
#if SIMPLE_DEC_BP_ENABLE
    if (NULL != ppctl->pdp) {
        u32 ret = vm_read(ppctl->p_vm_tab[ppctl->dir_index], \
                          ppctl->pdp, \
                          sizeof(dp_buff));
        if (sizeof(dp_buff) == ret) {
            dp_buff *dp = (dp_buff *)ppctl->pdp;
            ppctl->findex = dp->findex;
        }
    }
    log_info("next dev findex : %d\n", ppctl->findex);
#endif
    return play_one_file(ppctl);
}
#endif

/*----------------------------------------------------------------------------*/
/**@brief   播放下一个文件夹
   @param   ppctl 播放器句柄
   @return  成功：解码器句柄  失败：NULL
   @note     static dec_obj *play_one_file(music_player *p_music)
*/
/*----------------------------------------------------------------------------*/
static u32 simple_next_dir(play_control *ppctl)
{
    decoder_stop(ppctl->p_dec_obj, NEED_WAIT);//记录音乐断点
    if (NULL != ppctl->pdp) {
        if (true == get_dp(ppctl->p_dec_obj, ppctl->pdp)) {
#if SIMPLE_DEC_BP_ENABLE
            vm_write(ppctl->p_vm_tab[ppctl->dir_index], ppctl->pdp, sizeof(dp_buff));
#endif
        }
    }
    ppctl->dir_index++;
    if (ppctl->dir_index >= ppctl->dir_total) {
        ppctl->dir_index = 0;
    }
#if TFG_EXT_FLASH_EN
    if (EXT_FLASH_RW == ppctl->dev_index) {
        ppctl->pdp = &ext_dec_dp[ppctl->dir_index];
    } else
#endif
    {
        ppctl->pdp = &inr_dec_dp[ppctl->dir_index];
    }
    if (NULL != ppctl->pdp) {
#if SIMPLE_DEC_BP_ENABLE
        vm_read(ppctl->p_vm_tab[ppctl->dir_index], ppctl->pdp, sizeof(dp_buff));
#endif
        dp_buff *dp = (dp_buff *)ppctl->pdp;
        /* log_info("dp->findex : %d\n", dp->findex); */
        if (0 != dp->findex) {
            ppctl->findex = dp->findex;
        } else {
            ppctl->findex = 1;
        }
    } else {
        ppctl->findex = 1;
    }
    return play_one_file(ppctl);
}
#endif
