/*--------------------------------------------------------------------------*/
/**@file    break_point.c
   @brief   断点播放功能接口
   @details
   @author
   @date
   @note
*/
/*----------------------------------------------------------------------------*/
#include "common/mbox_common.h"
#include "break_point.h"
#include "play_file.h"
#include "music_play.h"
#include "device.h"
#include "get_music_file.h"
#include "mbox_main.h"
#include "device.h"
#include "vm_api.h"
#include "decoder_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

const dev_bp_vm_idx_t break_point_table[MAX_DEVICE + 1] = {
    {VM_INDEX_UDISK_INDEX, VM_INDEX_UDISK_BP},

#ifdef SDMMCA_EN
    {VM_INDEX_SDMMCA_INDEX, VM_INDEX_SDMMCA_BP},
#endif

#ifdef SDMMCB_EN
    {VM_INDEX_SDMMCB_INDEX, VM_INDEX_SDMMCB_BP},
#endif

#if TCFG_USB_EXFLASH_UDISK_ENABLE
    {VM_INDEX_EXTFLSH_INDEX, VM_INDEX_EXTFLSH_BP},
#endif
};

/*----------------------------------------------------------------------------*/
/** @brief: 断点信息清除函数
    @param: dev：需要清除信息的设备
    @return:void
    @author:
    @note:  void clear_music_break_point(u8 dev)
*/
/*----------------------------------------------------------------------------*/
void clear_music_break_point(u8 dev)
{
    dev_vm_index_t dev_info;
    dev_info.file_index = 1;
    dev_info.sclust = 0;
    dev_info.bp_flag = 0;
    vm_write(break_point_table[dev].index, (u8 *) & (dev_info), sizeof(dev_vm_index_t));
}

/*----------------------------------------------------------------------------*/
/**@brief   记忆设备断点信息到存储器eerprm
   @param   dev：需要记忆信息的设备
   @param   mode:1 index+bp 0:only index
   @return  void
   @author:
   @note    void save_music_break_point(u8 dev)
*/
/*----------------------------------------------------------------------------*/
void save_music_break_point(u8 dev, u8 mode)
{
    if (Music_Play_var.bPlayStatus == MAD_STOP) {
        return;
    }

    u32 sclust = 0;
    if (fs_get_file_sclust(&sclust) == false) {
        return;
    }

    if (!mode) {
        dev_vm_index_t dev_info;
        dev_info.file_index = playfile.given_file_number;
        dev_info.sclust = sclust;
        dev_info.bp_flag = 0;
        vm_write(break_point_table[dev].index, (u8 *) & (dev_info), sizeof(dev_vm_index_t));
    } else {
        dev_vm_index_t dev_info;
        dev_info.file_index = playfile.given_file_number;
        dev_info.sclust = sclust;
        dev_info.bp_flag = 1;
        vm_write(break_point_table[dev].index, (u8 *) & (dev_info), sizeof(dev_vm_index_t));

        dp_buff dbuff;
        dbuff.findex = playfile.given_file_number;
        if (decode_get_bp(&dbuff) == true) {
            vm_write(break_point_table[dev].bp, (u8 *)&dbuff, sizeof(dbuff));
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   记忆设备断点信息到存储器eerprm
   @param   dev：需要记忆信息的设备
   @param   slcust:簇号   file_index:文件号
   @return  void
   @author:
   @note    void save_rec_break_point(u32 sclust,u32 file_index)
*/
/*----------------------------------------------------------------------------*/
void save_rec_break_point(u32 sclust, u32 file_index)
{
    u8 dev = device_active;
    dev_vm_index_t dev_info;
    dev_info.file_index = playfile.given_file_number;
    dev_info.sclust = sclust;
    dev_info.bp_flag = 0;
    vm_write(break_point_table[dev].index, (u8 *) & (dev_info), sizeof(dev_vm_index_t));
}

/*----------------------------------------------------------------------------*/
/**@brief   从存储器读取设备断点信息
   @param   dev：需要读取信息的设备
   @return  读取断点信息成功标志
   @author:
   @note    _bool load_music_break_point(u8 dev,void *bp_info)
*/
/*----------------------------------------------------------------------------*/
bool load_music_break_point(u8 dev, void *bp_info, u32 *sclust)
{
    dev_vm_index_t dev_info;
    vm_read(break_point_table[dev].index, (u8 *) & (dev_info), sizeof(dev_vm_index_t));

    if (dev_info.bp_flag == 0xff || dev_info.file_index == 0xffffffff) {
        ///illegal data
        memset(bp_info, 0x00, sizeof(dp_buff));
        ((dp_buff *)bp_info)->findex = 1;
        return false;
    }

    *sclust = dev_info.sclust;
    if (dev_info.bp_flag == 0) {
        //no decode break point
        memset(bp_info, 0x00, sizeof(dp_buff));
        ((dp_buff *)bp_info)->findex = dev_info.file_index;
        return true;
    }

    vm_read(break_point_table[dev].bp, bp_info, sizeof(dp_buff));
    return true;
}

