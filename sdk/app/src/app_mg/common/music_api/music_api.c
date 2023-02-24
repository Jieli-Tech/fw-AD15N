#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "hwi.h"
#include "decoder_api.h"
#include "dev_manage.h"
#include "vfs.h"
#include "circular_buf.h"
#include "audio.h"
#include "dac.h"
#include "ump3_api.h"
#include "midi_api.h"
#include "f1a_api.h"
#include "a_api.h"
#include "msg.h"
#include "speed_api.h"
#include "src_api.h"
#include "music_api.h"
#include "music_file_op/music_file_op.h"
#include "speed_api.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#define MUSIC_PLAY_MAX_CHL		3
static decode_succ_callback_t decode_succ_cb = NULL;

static music_play_obj *music_hdl_alloc(void)
{
    static music_play_obj hdl_table[MUSIC_PLAY_MAX_CHL];

    for (int index = 0; index < sizeof(hdl_table) / sizeof(hdl_table[0]); index++) {
        if (hdl_table[index].hdl_active_flg == 0) {
            memset((u8 *)(&hdl_table[index]), 0x00, sizeof(music_play_obj));
            hdl_table[index].hdl_active_flg = 1;
            return &hdl_table[index];
        }
    }

    return NULL;
}

static void music_hdl_free(music_play_obj *hdl)
{
    if (hdl == NULL) {
        return;
    }

    hdl->hdl_active_flg = 0;
}

/*----------------------------------------------------------------------------*/
/**@brief 音乐解码成功回调接口
   @param cb:callback function
   @return null
   @note
*/
/*----------------------------------------------------------------------------*/
void music_decode_succ_cb_regist(decode_succ_callback_t cb)
{
    decode_succ_cb = cb;
}

music_play_obj *music_play_create(music_play_obj *hdl, const char *dir, u32 index, u32 decoder_type, \
                                  void *device, PLAY_MODE play_mode, char *type, void *bpbuff)
{

    void *_bpbuff = bpbuff;
    hdl->play_mode = play_mode;

    //DECODER TYPE初始化
    hdl->decoder_type = decoder_type;

    //MIDI琴不需要打开文件,decode_io()的pfile直接传入NULL
    if (decoder_type == BIT_MIDI_CTRL) {
        log_info("midi ctrl init!\n");
        dec_obj *obj = decoder_io(NULL, hdl->decoder_type, NULL, 0);
        if (obj == 0) {
            return NULL;
        }
        hdl->decode_api.p_dec = obj;
        return hdl;
    }

    //文件夹内容初始化
    hdl->dir.dir = dir;

    //文件查找
    void **ppvfs = &hdl->file.pvfs;
    void **ppvfile = &hdl->file.pvfile;
    hdl->file.cur_file_index = index;
    if (musci_file_open_bydirindex(ppvfs, ppvfile, hdl->dir.dir, &hdl->file.cur_file_index, device, type) != 0) {
        music_err_printf("music open file error !!!, try first file \n");
        hdl->file.cur_file_index = 1;
        _bpbuff = NULL;
        if (musci_file_open_bydirindex(ppvfs, ppvfile, hdl->dir.dir, &hdl->file.cur_file_index, device, type) != 0) {
            music_err_printf("music open file failed !!! \n");
            return NULL;
        }
    }
    hdl->file.total_file_num = music_file_get_total(hdl->file.pvfile);
    /* u8 file_name[VFS_FILE_NAME_LEN] = {0}; */
    if (music_file_name(hdl->file.pvfile, g_file_sname, sizeof(g_file_sname))) {
        music_printf("cur play file name:%s \n", g_file_sname);
    }

    dec_obj *obj = decoder_io(hdl->file.pvfile, hdl->decoder_type, _bpbuff, hdl->loop);
    if (obj == 0) {
        music_file_close(&hdl->file.pvfs, &hdl->file.pvfile);
        return NULL;
    }

    hdl->decode_api.p_dec = obj;

    return hdl;
}

/*----------------------------------------------------------------------------*/
/**@brief  音乐播放接口
   @param  _hdl:音乐播放句柄
   @param  dir:文件夹名称
   @param  index:文件号，如果播放文件号找不到就去播放第一首.如果传0就是按照文件路径播放，dir传一个完整路径即可。
   @param  decoder_type:解码的类型，和是否支持变速变调。可选参数如下：
			BIT_F1A1, BIT_F1A2, BIT_MP3, BIT_A, BIT_MIDI, BIT_SPEED
   @param  device:设备句柄,通过dev_open获取到，内置flash使用NULL即可,device = dev_open(__SFC_NANE, 0);
   @param  play_mode:播放模式
   @param  fs_type:文件系统类型,支持类型为：FS_TYPE_SYD,FS_TYPE_NORFS,FS_TYPE_FAT
   @param  bpbuff:断点信息。有断点时候按照断点播放，没有传NULL。
   @return 0:ok 非0：fail
   @note
*/
/*----------------------------------------------------------------------------*/
int music_play(music_play_obj **_hdl, const char *dir,  u32 index, u32 decoder_type, \
               void *device, PLAY_MODE play_mode, void *fs_type, void *bpbuff)
{
    if (*_hdl) {
        music_play_destroy(_hdl);
        *_hdl = NULL;
    }

    //申请句柄
    music_play_obj *hdl = music_hdl_alloc();
    if (hdl == NULL) {
        music_err_printf("music error !!! hdl null \n");
        return -1;
    }

    hdl->en = MUSIC_PLAY_INIT_NONE;
    dp_buff *pdbuff = NULL;
#if (MUSIC_BREAK_POINT_EN || MUSIC_LOOP_EN)
    pdbuff = &hdl->dbuff;
    clear_dp(pdbuff);
    if (bpbuff) {
        memcpy(pdbuff, bpbuff, sizeof(dp_buff));
    }
#if MUSIC_LOOP_EN
    hdl->loop = 0;/*设置循环播放次数，0~255,255为无限循环*/
#endif
#endif
    if (music_play_create(hdl, dir, index, decoder_type, device, play_mode, fs_type, pdbuff) == NULL) {
        music_err_printf("music_play_create failed !!! \n");
        music_hdl_free(hdl);
        return -1;
    }

    hdl->en = MUSIC_PLAY_INIT_OK;
    *_hdl = hdl;

    if (decode_succ_cb) {
        decode_succ_cb(hdl);
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief  音乐播放接口
   @param  _hdl:音乐播放句柄
   @return 0:ok 非0：fail
   @note
*/
/*----------------------------------------------------------------------------*/
int music_midi_ctrl_play(music_play_obj **_hdl)
{
    if (*_hdl) {
        music_play_destroy(_hdl);
        *_hdl = NULL;
    }

    //申请句柄
    music_play_obj *hdl = music_hdl_alloc();
    if (hdl == NULL) {
        music_err_printf("music error !!! hdl null \n");
        return -1;
    }

    hdl->en = MUSIC_PLAY_INIT_NONE;
    hdl->decoder_type = BIT_MIDI_CTRL;
    dec_obj *obj = decoder_io(NULL, hdl->decoder_type, NULL, hdl->loop);
    if (obj == 0) {
        music_hdl_free(hdl);
        return -1;
    }

    hdl->decode_api.p_dec = obj;
    hdl->en = MUSIC_PLAY_INIT_OK;
    *_hdl = hdl;

    if (decode_succ_cb) {
        decode_succ_cb(hdl);
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief  音乐播放器结束，下一步动作
   @param  hdl:音乐播放句柄
   @return 0:ok -1:fail
   @note
*/
/*----------------------------------------------------------------------------*/
int music_play_end_operation(music_play_obj **_hdl)
{
    music_play_obj *hdl = *_hdl;
    if (_hdl == NULL || hdl == NULL || hdl->en != MUSIC_PLAY_INIT_OK) {
        return -1;
    }

    if (hdl->play_mode == MUSIC_MODE_PLAY_ONE) {
        music_play_destroy(_hdl);
        return 0;
    }

    if (hdl->decode_api.p_dec) {
        decoder_stop(hdl->decode_api.p_dec, NEED_WAIT);
    }

    switch (hdl->play_mode) {
    case MUSIC_MODE_CIRCUL_ONE:	//循环播放第一首
        break;
    case MUSIC_MODE_AUTO_NEXT: 	//自动下一曲
        hdl->file.cur_file_index = (++hdl->file.cur_file_index <= hdl->file.total_file_num) ? \
                                   hdl->file.cur_file_index : 1;

        if (music_file_reopen_byindex(&hdl->file.pvfs, &hdl->file.pvfile, &hdl->file.cur_file_index, 0) != 0) {
            music_play_destroy(_hdl);
            return -1;
        }
        break;
    case MUSIC_MODE_AUTO_PREV:	//自动上一曲
        hdl->file.cur_file_index = (--hdl->file.cur_file_index >= 1) ? \
                                   hdl->file.cur_file_index : hdl->file.total_file_num;

        if (music_file_reopen_byindex(&hdl->file.pvfs, &hdl->file.pvfile, &hdl->file.cur_file_index, 1) != 0) {
            music_play_destroy(_hdl);
            return -1;
        }
        break;

    default:
        break;
    }


    dp_buff *pdbuff = NULL;
#if (MUSIC_LOOP_EN)
    pdbuff = &hdl->dbuff;
    clear_dp(pdbuff);
#endif
    dec_obj *obj = decoder_io(hdl->file.pvfile, hdl->decoder_type, pdbuff, hdl->loop);
    if (obj == 0) {
        music_file_close(&hdl->file.pvfs, &hdl->file.pvfile);
        return -1;
    }

    hdl->decode_api.p_dec = obj;

    if (decode_succ_cb) {
        decode_succ_cb(hdl);
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief  音乐播放器下一曲
   @param  hdl:音乐播放句柄
   @return 0:ok -1:fail
   @note
*/
/*----------------------------------------------------------------------------*/
int music_play_next_file(music_play_obj **_hdl)
{
    music_play_obj *hdl = *_hdl;
    if (_hdl == NULL || hdl == NULL || hdl->en != MUSIC_PLAY_INIT_OK) {
        music_printf("hdl uninit !!!! \n");
        return -1;
    }

    if (hdl->play_mode == MUSIC_MODE_PLAY_ONE) {
        music_printf("play mode  MUSIC_MODE_PLAY_ONE!!!! \n");
        return 0;
    }

    if (hdl->decode_api.p_dec) {
        decoder_stop(hdl->decode_api.p_dec, NO_WAIT);
    }

    switch (hdl->play_mode) {
    case MUSIC_MODE_CIRCUL_ONE:	//循环播放第一首
        break;
    case MUSIC_MODE_AUTO_NEXT: 	//自动下一曲
        hdl->file.cur_file_index = (++hdl->file.cur_file_index <= hdl->file.total_file_num) ? \
                                   hdl->file.cur_file_index : 1;
        break;
    case MUSIC_MODE_AUTO_PREV:	//自动上一曲
        hdl->file.cur_file_index = (--hdl->file.cur_file_index >= 1) ? \
                                   hdl->file.cur_file_index : hdl->file.total_file_num;
        break;

    default:
        break;
    }

    if (music_file_reopen_byindex(&hdl->file.pvfs, &hdl->file.pvfile, &hdl->file.cur_file_index, 0) != 0) {
        music_err_printf("open file error !!! \n");
        music_play_destroy(_hdl);
        return -1;
    }

    dp_buff *pdbuff = NULL;
#if (MUSIC_LOOP_EN)
    pdbuff = &hdl->dbuff;
    clear_dp(pdbuff);
#endif
    dec_obj *obj = decoder_io(hdl->file.pvfile, hdl->decoder_type, pdbuff, hdl->loop);
    if (obj == 0) {
        music_err_printf("decoder error !!!\n");
        music_file_close(&hdl->file.pvfs, &hdl->file.pvfile);
        return -1;
    }

    hdl->decode_api.p_dec = obj;

    if (decode_succ_cb) {
        decode_succ_cb(hdl);
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief  音乐播放器上一曲
   @param  hdl:音乐播放句柄
   @return 0:ok -1:fail
   @note
*/
/*----------------------------------------------------------------------------*/
int music_play_priv_file(music_play_obj **_hdl)
{
    music_play_obj *hdl = *_hdl;
    if (_hdl == NULL || hdl == NULL || hdl->en != MUSIC_PLAY_INIT_OK) {
        music_printf("hdl uninit !!!! \n");
        return -1;
    }

    if (hdl->play_mode == MUSIC_MODE_PLAY_ONE) {
        music_printf("play mode  MUSIC_MODE_PLAY_ONE!!!! \n");
        return 0;
    }

    if (hdl->decode_api.p_dec) {
        decoder_stop(hdl->decode_api.p_dec, NO_WAIT);
    }

    switch (hdl->play_mode) {
    case MUSIC_MODE_CIRCUL_ONE:	//循环播放第一首
        break;
    case MUSIC_MODE_AUTO_NEXT: 	//自动下一曲
        hdl->file.cur_file_index = (--hdl->file.cur_file_index >= 1) ? \
                                   hdl->file.cur_file_index : hdl->file.total_file_num;
        break;
    case MUSIC_MODE_AUTO_PREV:	//自动上一曲
        hdl->file.cur_file_index = (++hdl->file.cur_file_index <= hdl->file.total_file_num) ? \
                                   hdl->file.cur_file_index : 1;
        break;

    default:
        break;
    }

    if (music_file_reopen_byindex(&hdl->file.pvfs, &hdl->file.pvfile, &hdl->file.cur_file_index, 1) != 0) {
        music_err_printf("open file error !!! \n");
        music_play_destroy(_hdl);
        return -1;
    }

    dp_buff *pdbuff = NULL;
#if (MUSIC_LOOP_EN)
    pdbuff = &hdl->dbuff;
    clear_dp(pdbuff);
#endif
    dec_obj *obj = decoder_io(hdl->file.pvfile, hdl->decoder_type, pdbuff, hdl->loop);
    if (obj == 0) {
        music_err_printf("decoder error !!!\n");
        music_file_close(&hdl->file.pvfs, &hdl->file.pvfile);
        return -1;
    }

    hdl->decode_api.p_dec = obj;

    if (decode_succ_cb) {
        decode_succ_cb(hdl);
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief  暂停音乐播放器
   @param  hdl:音乐播放句柄
   @return 音乐播放器状态,音乐播放器包含的状态如下:
    MUSIC_PAUSE,
    MUSIC_PLAY,
    MUSIC_STOP,
   @note
*/
/*----------------------------------------------------------------------------*/
int music_play_pp(music_play_obj *hdl)
{
    if (hdl == NULL || hdl->en != MUSIC_PLAY_INIT_OK) {
        return MUSIC_STOP;
    }

    dec_obj *obj = hdl->decode_api.p_dec;
    if (NULL == obj) {
        return MUSIC_STOP;
    }

    decoder_pause(obj);

    return (obj->sound.enable & B_DEC_PAUSE) ? MUSIC_PAUSE : MUSIC_PLAY;
}

/*----------------------------------------------------------------------------*/
/**@brief  获取乐播放器状态
   @param  hdl:音乐播放句柄
   @return 音乐播放器状态,音乐播放器包含的状态如下:
    MUSIC_PAUSE,
    MUSIC_PLAY,
    MUSIC_STOP,
   @note
*/
/*----------------------------------------------------------------------------*/
int music_play_get_status(music_play_obj *hdl)
{
    if (hdl == NULL || hdl->en != MUSIC_PLAY_INIT_OK) {
        return MUSIC_STOP;
    }

    dec_obj *obj = hdl->decode_api.p_dec;
    if (NULL == obj) {
        return MUSIC_STOP;
    }

    return (obj->sound.enable & B_DEC_PAUSE) ? MUSIC_PAUSE : MUSIC_PLAY;
}

/*----------------------------------------------------------------------------*/
/**@brief  获取音乐播放器类型
   @param  _hdl:音乐播放句柄
   @return -1:fail type：音乐类型,类型包括如下：
    D_TYPE_F1A_1
    D_TYPE_F1A_2
    D_TYPE_MP3
    D_TYPE_A
    D_TYPE_MIDI
   @note
*/
/*----------------------------------------------------------------------------*/
int music_play_get_decoder_type(music_play_obj *hdl)
{
    if (hdl == NULL || hdl->en != MUSIC_PLAY_INIT_OK) {
        return -1;
    }

    dec_obj *obj = hdl->decode_api.p_dec;
    if (NULL == obj) {
        return -1;
    }


    return obj->type;
}

/*----------------------------------------------------------------------------*/
/**@brief  音乐播放io api
   @param  _hdl:音乐播放句柄
   @param  cmd:命令名称，详细看MUSIC_IO_CTL_CMD定义
   @param  arg:参数，需要获取的数据通过arg传出来，注意很多时候会指针
   @return 0:ok 非0：fail
   @note
*/
/*----------------------------------------------------------------------------*/
int music_play_io_ctl(music_play_obj *hdl, MUSIC_IO_CTL_CMD cmd, int arg)
{
    if (hdl == NULL || hdl->en != MUSIC_PLAY_INIT_OK) {
        return -1;
    }

    dec_obj *obj = hdl->decode_api.p_dec;
    if (NULL == obj) {
        return -1;
    }

    if (arg == 0) {
        return -1;
    }

    audio_decoder_ops *ops = obj->dec_ops;

    switch (cmd) {
    case MUSIC_CMD_GET_PLAYTIME:
        (*(int *)arg) = ops->get_playtime(obj->p_dbuf);
        break;
    case MUSIC_CMD_GET_INFO:
        (*(dec_inf_t *)arg) = *(ops->get_dec_inf(obj->p_dbuf));
        break;
    case MUSIC_CMD_GET_CUR_FILE_INDEX:
        (*(int *)arg) = hdl->file.cur_file_index;
        break;
    case MUSIC_CMD_GET_TOTAL_NUM:
        (*(int *)arg) = hdl->file.total_file_num;
        break;
    case MUSIC_CMD_GET_LOOP_NUM:
        (*(int *)arg) = hdl->loop;
        break;
    case MUSIC_CMD_SET_LOOP_NUM:
        hdl->loop = (u8)arg;
        break;
    default:
        return -1;
        break;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief 获取断点 ,获取断点时候会把解码停止,不要没必要的播放中途获取一下断点
   @param  _hdl:音乐播放句柄
   @param  dbuff:断点信息
   @return 0:ok 非0：fail
   @note
*/
/*----------------------------------------------------------------------------*/
int music_play_get_bp(music_play_obj *hdl, dp_buff *dbuff)
{
    int err = -1;
    if (hdl == NULL || dbuff == NULL) {
        return err;
    }

    if (hdl->en != MUSIC_PLAY_INIT_OK) {
        return err;
    }

    music_printf("music play destroy \n");
    hdl->en = MUSIC_PLAY_INIT_NONE;

    if (hdl->decode_api.p_dec) {
        decoder_stop(hdl->decode_api.p_dec, NO_WAIT);
        dbuff->findex = hdl->file.cur_file_index;
        if (get_dp(hdl->decode_api.p_dec, dbuff) == true) {
            err = 0;
        }
    }

    music_file_close(&hdl->file.pvfs, &hdl->file.pvfile);
    music_hdl_free(hdl);

    return err;
}

/*----------------------------------------------------------------------------*/
/**@brief 支持动态设置音乐播放变速变调参数
   @param  _hdl:音乐播放句柄
   @param  sp_parm:变速变调参数
   @return 0:ok 非0：fail
   @note
*/
/*----------------------------------------------------------------------------*/
int music_play_speed_set_para(music_play_obj *hdl, SPEED_PITCH_PARA_STRUCT *sp_parm)
{
    if (hdl == NULL || hdl->en != MUSIC_PLAY_INIT_OK || sp_parm == NULL) {
        log_info("no init \n");
        return -1;
    }

    dec_obj *p_dec = hdl->decode_api.p_dec;
    if (NULL == p_dec) {
        log_info("no init \n");
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
    SPEEDPITCH_STUCT_API *ops = get_sppitch_context();           //获取变采样函数接口
    //初始化：rs_buf：运算Buf; rs_parm：参数指针，传完可以释放的，里面不会记录这个指针的。sppitch_io:output接口，说明如下
    OS_ENTER_CRITICAL();
    ops->open(((sound_in_obj *)effect->p_si)->p_dbuf, sp_parm, (void *)NULL);
    OS_EXIT_CRITICAL();

    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief  关闭音乐播放器
   @param  _hdl:音乐播放句柄
   @return 0:ok 非0：fail
   @note
*/
/*----------------------------------------------------------------------------*/
int music_play_destroy(music_play_obj **music_hdl)
{
    music_play_obj *hdl = *music_hdl;
    if (hdl == NULL) {
        return -1;
    }

    if (hdl->en != MUSIC_PLAY_INIT_OK) {
        return -1;
    }

    music_printf("music play destroy \n");
    hdl->en = MUSIC_PLAY_INIT_NONE;

    if (hdl->decode_api.p_dec) {
        decoder_stop(hdl->decode_api.p_dec, NO_WAIT);
    }

    music_file_close(&hdl->file.pvfs, &hdl->file.pvfile);
    music_hdl_free(hdl);

    *music_hdl = NULL;
    return 0;
}
