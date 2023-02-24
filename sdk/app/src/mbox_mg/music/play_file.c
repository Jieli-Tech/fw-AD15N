#include "common/mbox_common.h"
#include "play_file.h"
#include "msg.h"
#include "break_point.h"
#include "music_play.h"
#include "device.h"
#include "mbox_main.h"
#include "get_music_file.h"
#include "decoder_api.h"
#include "vfs.h"
#include "dev_mg/device.h"
#include "eq.h"
#include "music_auto_save_bp.h"
#include "vfs_fat.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"


u32 music_energy;

extern MUSIC_PLAY_VAR Music_Play_var;

static struct vfscan_reset_info vfs_info[MAX_DEVICE + 1];
static dec_obj *p_dec_obj = NULL;
void *pvfile = NULL;
static void *pvfs = NULL;
static void *device = NULL;
#ifdef BREAK_POINT_PLAY_EN
static dp_buff dbuff;
#endif

struct vfscan			*fsn;//设备扫描句柄


/*----------------------------------------------------------------------------*/
/**@brief   文件系统mount
   @param   void
   @return  0：成功：其他：失败
   @author:
   @note    u8 fs_mount(void)
*/
/*----------------------------------------------------------------------------*/
u8 fs_mount(void)
{
    u8 res, err;
    //device_active
    device = device_get_active_hdl();
    if (device == NULL) {
        log_info("open device error !!! \n");
        return 0xfe;
    }

    device_upgrate_try();

    res = vfs_mount(&pvfs, (void *)device, (void *)"fat");
    if (0 != res) {
        log_info("vfs mount error !!! \n");
        device_close(device);
        device = NULL;
        return 0xfe;
    }

    return 0;
}
/*----------------------------------------------------------------------------*/
/**@brief   文件系统close
   @param   void
   @return  0：成功：其他：失败
   @author:
   @note    u8 fs_mount(void)
*/
/*----------------------------------------------------------------------------*/
void fs_file_close(void)
{
    if (pvfile) {
        vfs_file_close(&pvfile);
        //pvfile = NULL;
    }
    if (pvfs) {
        vfs_fs_close(&pvfs);
        //pvfs = NULL;
    }

    if (device) {
        device_close(device);
        device = NULL;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   文件信息reset
   @param   void
   @return  void
   @author:
   @note    void vfs_info_clear(void)
*/
/*----------------------------------------------------------------------------*/
void vfs_info_clear(void)
{
    if (fsn) {
        fat_vfscan_free(fsn);
        fsn = NULL;
    }
    for (int i = 0; i < sizeof(vfs_info) / sizeof(vfs_info[0]); i++) {
        memset((u8 *)&vfs_info[i], 0x00, sizeof(vfs_info[0]));
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   清除指定设备的文件扫描信息
   @param   void
   @return  void
   @author:
   @note    void vfs_info_disable(u8 device_index)
*/
/*----------------------------------------------------------------------------*/
void vfs_info_disable(u8 device_index)
{
    vfs_info[device_index].active = 0;
}

/*----------------------------------------------------------------------------*/
/**@brief   文件扫描
   @param   void
   @return  0：扫描成功：其他：失败
   @author:
   @note    u8 fs_get_total_files(void)
*/
/*----------------------------------------------------------------------------*/
static const char scan_parm[] = "-t"
                                "MP3WAV"
                                " -sn -r"
                                ;
/* 后缀名扫描只匹配3个字符,如UMP3只需添加“UMP”,对于A格式文件,添加"A??" */

u8 fs_get_total_files(void)
{
    int total = 0;
    u32 ret_p;
    fsn = vfs_fscan_new(pvfs, "/", scan_parm, 9, NULL, fsn, &vfs_info[device_active]);
    total = fsn->file_number;


    if (total == 0) {
        return 0xfe;
    }
    if (!device_active_is_online()) {
        log_info(">>>>>>>>>>>device offline \n");
        return 0xfe;
    }
    log_info("device file total:%d \n", total);
    vfs_info[device_active].active = 1;
    music_device_info[device_active].wfileTotal = total;
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief   获取文件夹信息
   @param   start_num:文件夹第一个文件文件号。	end_num：文件夹最后一个文件文件号
   @return  0：扫描成功：其他：失败
   @author:
   @note    int fs_get_folferinfo(int *start_num, int *end_num)
*/
/*----------------------------------------------------------------------------*/
int fs_get_folferinfo(int *start_num, int *end_num)
{
    if (pvfile == NULL) {
        return -1;
    }

    vfs_get_folderinfo(pvfile, fsn, start_num, end_num);
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief   获取文件簇号
   @param   clust:当前文件的簇号
   @return  false：获取失败：true：成功获取
   @author:
   @note    bool fs_get_file_sclust(u32 *clust)
*/
/*----------------------------------------------------------------------------*/
bool fs_get_file_sclust(u32 *clust)
{
    if (pvfile == NULL) {
        return false;
    }

    struct vfs_attr attr = {0};
    vfs_get_attrs(pvfile, (void *)&attr);
    *clust = attr.sclust;
    log_info("start clust:%x \n", *clust);

    return true;
}

/*----------------------------------------------------------------------------*/
/** @brief: 是否有解码线程
    @param: void
    @return:void
    @author:
    @note:  u8 decode_is_enable(void)
*/
/*----------------------------------------------------------------------------*/
u8 decode_is_enable(void)
{
    return (p_dec_obj != NULL);
}

/*----------------------------------------------------------------------------*/
/** @brief: 暂停解码
    @param: void
    @return:void
    @author:
    @note:  u8 decode_pp(void)
*/
/*----------------------------------------------------------------------------*/
u8 decode_pp(void)
{
    u8 res;
    res = Music_Play_var.bPlayStatus;

    if (p_dec_obj == NULL) {
        return res;
    }

    decoder_pause(p_dec_obj);
    if (p_dec_obj->sound.enable & B_DEC_PAUSE) {
        Music_Play_var.bPlayStatus = MAD_PAUSE;
    } else {
        Music_Play_var.bPlayStatus = MAD_PLAY;
    }

    res = Music_Play_var.bPlayStatus;
    return res;
}

/*----------------------------------------------------------------------------*/
/** @brief: 快进
    @param: void
    @return:void
    @author:
    @note:  void decode_ff(void)
*/
/*----------------------------------------------------------------------------*/
void decode_ff(u8 step)
{
    if (p_dec_obj == NULL) {
        return ;
    }

    Music_Play_var.bPlayStatus = MAD_PLAY;
    decoder_ff(p_dec_obj, step);
}

/*----------------------------------------------------------------------------*/
/** @brief: 快退
    @param: void
    @return:void
    @author:
    @note:  void decode_fr(void)
*/
/*----------------------------------------------------------------------------*/
void decode_fr(u8 step)
{
    if (p_dec_obj == NULL) {
        return ;
    }

    Music_Play_var.bPlayStatus = MAD_PLAY;
    decoder_fr(p_dec_obj, step);
}

/*----------------------------------------------------------------------------*/
/** @brief: 获取播放时间
    @param: void
    @return:void
    @author:
    @note:  u16 get_music_play_time(void)
*/
/*----------------------------------------------------------------------------*/
u16 get_music_play_time(void)
{
    if (p_dec_obj == NULL) {
        return 0;
    }

    audio_decoder_ops *ops = p_dec_obj->dec_ops;
    u16 time = (u16)ops->get_playtime(p_dec_obj->p_dbuf);

    return time;
}

/*----------------------------------------------------------------------------*/
/** @brief:设置EQ的模式
 *详细的eq模式看const s8 eqtab[EQ_MODEMAX][EQ_FRE_NUM]表.
    @param: void
    @return:void
    @author:
    @note:  int eq_set_mode(u8 mode)
*/
/*----------------------------------------------------------------------------*/
int eq_set_mode(u8 mode)
{
    if ((p_dec_obj == NULL) || (p_dec_obj->eq == NULL)) {
        return -1;
    }

    g_eq_mode = p_dec_obj->eq(mode);
    log_info("eq mode:%d \n", g_eq_mode);

    return g_eq_mode;
}

/*----------------------------------------------------------------------------*/
/** @brief: 停止解码
    @param: void
    @return:void
    @author:
    @note:  void stop_decode(void)
*/
/*----------------------------------------------------------------------------*/
void stop_decode(void)
{
    if (p_dec_obj == NULL) {
        return;
    }

    decoder_stop(p_dec_obj, NO_WAIT);
    p_dec_obj = NULL;
    Music_Play_var.bPlayStatus = MAD_STOP;
}

/*----------------------------------------------------------------------------*/
/** @brief: 开始解码
    @param: void
    @return:void
    @author:
    @note:  void start_decode(void)
*/
/*----------------------------------------------------------------------------*/
bool start_decode(bool break_point)
{
    void *bp_info = NULL;
#ifdef BREAK_POINT_PLAY_EN
    if (break_point) {
        bp_info = &dbuff;
    }
#endif
    log_info(">>pvfile:%x \n", (u32)pvfile);
    p_dec_obj = decoder_io(pvfile, BIT_WAV  | BIT_UMP3 /*|BIT_UMP3 | BIT_F1A1 | BIT_MIDI */ | BIT_MP3_ST, bp_info, 0);
    if (!p_dec_obj) {
        Music_Play_var.bPlayStatus = MAD_STOP;
        log_info("start decode error !!!\n");
        return false;
    }

#if BREAK_POINT_AUTO_SAVE
    audio_decoder_ops *ops = p_dec_obj->dec_ops;
    dec_inf_t *info = ops->get_dec_inf(p_dec_obj->p_dbuf);
    masb_run_init(info->total_time);
#endif

    Music_Play_var.bPlayStatus = MAD_PLAY;
    return true;
}
/*----------------------------------------------------------------------------*/
/** @brief: 获取断点
    @param: void
    @return:true:成功  false：失败
    @author:
    @note:  void start_decode(void)
*/
/*----------------------------------------------------------------------------*/
bool decode_get_bp(void *bp_info)
{
    if (p_dec_obj == NULL) {
        return false;
    }
    if (get_dp(p_dec_obj, bp_info) == true) {
        return true;
    }

    return false;
}

/*----------------------------------------------------------------------------*/
/** @brief: 解码模块初始化函数
    @param: break_point：断点信息控制位
    @return:解码成功标志位
    @author:
    @note:  bool decode_init(bool break_point)
*/
/*----------------------------------------------------------------------------*/
bool decode_init(bool break_point)
{
    u8 res, err;
    //device_active

#ifdef BREAK_POINT_PLAY_EN
    u32 bp_sclust = 0;
    if (break_point) {
        load_music_break_point(device_active, &dbuff, &bp_sclust);
        if (bp_sclust) {
            /* err = vfs_openbyclust(pvfs, &pvfile, bp_sclust);///play last file */
            err = vfs_select(pvfs, &pvfile, fsn, FSEL_BY_SCLUST, bp_sclust);
            if (0 != err) {
                log_info("clust open file error ,try first \n");
                playfile.given_file_number = 1;
                break_point = 0;
                goto _open_file;
            }
            u32 index = 0;
            vfs_ioctl(pvfile, FS_IOCTL_FILE_INDEX, (int)&index);
            playfile.given_file_number = index;

            log_info("cur play fileindex:%d \n", playfile.given_file_number);
            if (start_decode(break_point) == false) {
                return false;
            }

            return true;
        }
    }
#endif
    log_info("cur play fileindex:%d \n", playfile.given_file_number);

_open_file:
    /* err = vfs_openbyindex(pvfs, &pvfile, playfile.given_file_number);//SLEEP.lrc */
    err = vfs_select(pvfs, &pvfile, fsn, FSEL_BY_NUMBER, playfile.given_file_number);
    if (0 != err) {
        log_info("open file error !!! \n");
        return false;
    }

    if (start_decode(break_point) == false) {
        return false;
    }

    return true;
}

