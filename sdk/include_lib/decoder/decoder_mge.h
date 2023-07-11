#ifndef __DECODER_MGE_H__
#define __DECODER_MGE_H__
#include "if_decoder_ctrl.h"
#include "sound_effect_api.h"



typedef enum {
    MAD_ERROR_FILE_END         = 0x40,
    MAD_ERROR_FILESYSTEM_ERR   = 0x41,              // NO USED
    MAD_ERROR_DISK_ERR         = 0x42,              // NO USED
    MAD_ERROR_SYNC_LIMIT       = 0x43,              // 文件错误
    MAD_ERROR_FF_FR_FILE_END   = 0x44,              //快进结束
    MAD_ERROR_FF_FR_END        = 0x45,              // NO USED
    MAD_ERROR_FF_FR_FILE_START = 0x46,              //快退到头
    MAD_ERROR_LIMIT            = 0x47,              // NO USED
    MAD_ERROR_NODATA           = 0x48,              // NO USED
    MAD_ERROR_PLAY_END		   = 0x50,				//MIDI CTRL DATA OUTPUT END
    MAD_ERROR_F1X_START_ADDR   = 0x51,				//F1X起始位置错误
} MAD_INFO ;


typedef struct _dec_obj {
    void *p_file;
    void *dec_ops;
    void *p_dbuf;
    void *p_dp_buf;
    u8 *event_tab;
    u32 sr;
    sound_out_obj sound;
    void *src_effect;
    u32(*eq)(u32);
    u8 loop;
    u8 type; // DECOER_TYPE
    u8 function;		// 解码器支持的功能
    char ff_fr_step;	// 快进快退步长。正数-快进；负数-快退。单位-秒
} dec_obj;


extern u32 dec_hld_tab[];

void dac_kick_decoder(void *sound_hld, void *pkick);
void kick_decoder(void);
void decoder_soft_hook(void);
void irq_decoder_ret(dec_obj *obj, u32 ret);



#endif


