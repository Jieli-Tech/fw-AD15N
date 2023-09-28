#include "simple_play_file.h"
#include "device.h"
#include "device_mge.h"
#include "vfs.h"
#include "errno-base.h"
#include "sound_effect_api.h"
#include "vm_api.h"

#include "decoder_api.h"
#include "decoder_msg_tab.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[midi_dec]"
#include "log.h"

u32 get_decoder_status(dec_obj *obj)
{
    if (NULL == obj) {
        return MUSIC_STOP;
    }

    u32 enable = obj->sound.enable;
    if (0 == (enable & B_DEC_RUN_EN)) {
        return MUSIC_STOP;
    }

    return (enable & B_DEC_PAUSE) ? MUSIC_STOP : MUSIC_PLAY;
}

u32 simple_dev_fs_close(play_control *ppctl)
{
    vfs_file_close(&ppctl->pfile);
    vfs_fs_close(&ppctl->pfs);
    device_close(ppctl->dev_index);
    return 0;
}

u32 simple_dev_fs_mount(play_control *ppctl)
{
    ppctl->device = device_open(ppctl->dev_index);
    if ((NULL == ppctl->device) && (INNER_FLASH_RO != ppctl->dev_index)) {
        log_info("device open null!\n");
        return E_DEV_NULL;
    }
    u32 err = vfs_mount(&ppctl->pfs, ppctl->device, "sydfile");
    if (err) {
        log_info("vfs_mount err:0x%x\n", err);
        return E_MOUNT;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief   获取指定音乐文件
   @param   pvfs 文件系统句柄
   @param   ppvfile 文件句柄指针
   @param   path 文件路径，当findex为0时，按路径打开文件
   @param   findex 文件序号，当findex非0时，按序号打开文件
   @return  错误值，详见errno-base.h
   @note     static u32 music_file_open(void *pvfs, void **ppvfile, const char *path, u32 findex)
*/
/*----------------------------------------------------------------------------*/
static u32 music_file_open(void *pvfs, void **ppvfile, const char *path, u32 findex)
{
    u32 err = 0;

    /* findex等于0时，按路径播放 */
    err = vfs_openbypath(pvfs, ppvfile, path);
    if (err) {
        log_error("vfs_openbypath err:0x%x\n", err);
        return err;
    }

    /* findex非0时，按序号播放 */
    if (0 != findex) {
        err = vfs_openbyindex(pvfs, ppvfile, findex);
        if (err) {
            log_error("vfs_openbyindex err:0x%x idx:%d\n", err, findex);
            return err;
        }
    }

    return 0;
}

u32 simple_play_file_bypath(play_control *ppctl, const char *path)
{
    u32 err;
    decoder_stop(ppctl->p_dec_obj, NO_WAIT);

    vm_pre_erase();

    err = music_file_open(ppctl->pfs,   \
                          &ppctl->pfile, \
                          path,         \
                          0);
    if (err) {
        return err;
    }

    /* if (0 != ppctl->loop) { */
    /*     #<{(| 循环播放前清除断点 |)}># */
    /*     clear_dp(ppctl->pdp); */
    /* } */

    ppctl->p_dec_obj = decoder_io(ppctl->pfile,   \
                                  ppctl->dec_type, \
                                  ppctl->pdp,     \
                                  ppctl->loop);
    if (NULL == ppctl->p_dec_obj) {
        return E_DECODER;
    }

    log_info("dev:0x%x fpath:%s loop:%d\n", ppctl->dev_index, path, ppctl->loop);
    return 0;
}

u32 simple_play_file_byindex(play_control *ppctl, u32 index)
{
    u32 err;
    decoder_stop(ppctl->p_dec_obj, NO_WAIT);

    vm_pre_erase();

    err = music_file_open(\
                          ppctl->pfs,                                       \
                          &ppctl->pfile,                                    \
                          ((const char **)ppctl->pdir)[ppctl->dir_index], \
                          index                                               \
                         );
    if (err) {
        return err;
    }

    /* if (0 != ppctl->loop) { */
    /*     #<{(| 循环播放前清除断点 |)}># */
    /*     clear_dp(ppctl->pdp); */
    /* } */

    ppctl->p_dec_obj = decoder_io(ppctl->pfile,   \
                                  ppctl->dec_type, \
                                  ppctl->pdp,     \
                                  ppctl->loop);
    if (NULL == ppctl->p_dec_obj) {
        return E_DECODER;
    }

    vfs_ioctl(ppctl->pfile, FS_IOCTL_DIR_FILE_TOTAL, (int)&ppctl->ftotal);
    log_info("dev:0x%x findex:%d ftotal:%d loop:%d\n", ppctl->dev_index, ppctl->findex, ppctl->ftotal, ppctl->loop);

    return 0;
}

u32 simple_play_file_operate(play_control *ppctl, char c)
{
    u32 err;
    u32 retry = 10;
    while (--retry) {
        if (c == '-') {
            /* 上一曲 */
            ppctl->findex--;
            if (ppctl->findex == 0) {
                ppctl->findex = ppctl->ftotal;
            }
        } else if (c == '+') {
            /* 下一曲 */
            ppctl->findex++;
            if (ppctl->findex > ppctl->ftotal) {
                ppctl->findex = 1;
            }
        }
        err = play_one_file(ppctl);
        if (0 == err) {
            break;
        }
        log_info("play_next_file FAIL!, err:0x%x retry:%d\n", err, retry);
    }
    return err;
}

u32 play_one_file(play_control *ppctl)
{
    u32 err = 0;
    err = simple_play_file_byindex(ppctl, ppctl->findex);
    if (0 == err) {
        dp_buff *dp = (dp_buff *)ppctl->pdp;
        if (NULL != dp) {
            dp->findex = ppctl->findex;
#if SIMPLE_DEC_BP_ENABLE
            /* if (0 == ppctl->loop) { */
            /*     clear_dp(dp); */
            /* } */
            if (NULL != ppctl->p_vm_tab) {
                vm_write(\
                         ppctl->p_vm_tab[ppctl->dir_index],  \
                         ppctl->pdp,                         \
                         sizeof(dp_buff));
            }
#endif
        }
    }
    return err;
}
u32 play_prev_file(play_control *ppctl)
{
    return simple_play_file_operate(ppctl, '-');
}
u32 play_next_file(play_control *ppctl)
{
    return simple_play_file_operate(ppctl, '+');
}

#if defined(AUDIO_SPEED_EN) && (AUDIO_SPEED_EN)
int simple_play_speed_set_para(play_control *ppctl, SPEED_PITCH_PARA_STRUCT *sp_parm)
{
    dec_obj *p_dec = ppctl->p_dec_obj;
    if (NULL == p_dec) {
        log_info("no init \n");
        return -1;
    }

    if (NULL == sp_parm) {
        log_info("parm null\n");
        return -1;
    }
    sp_parm->insample = p_dec->sr;
    sound_out_obj *p_curr_sound = 0;
    p_curr_sound = &p_dec->sound;
    if (!(p_curr_sound->enable & B_DEC_EFFECT)) {
        log_info("no effect init \n");
        return -1;
    }
    EFFECT_OBJ *effect = p_curr_sound->effect;
    SPEEDPITCH_STUCT_API *ops = get_sppitch_context();           //获取变速变调函数接口
    OS_ENTER_CRITICAL();
    ops->open(((sound_in_obj *)effect->p_si)->p_dbuf, sp_parm, (void *)NULL);
    OS_EXIT_CRITICAL();

    return 0;
}
#endif

