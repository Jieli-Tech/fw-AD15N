#include "cpu.h"
#include "typedef.h"
#include "config.h"
#include "sound_effect_api.h"
#include "circular_buf.h"
#include "decoder_api.h"
#include "vfs.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[mp_io]"
#include "log.h"

//解码数据流io接口：
//读数接口，priv为传入的dec_hld，返回值是读到多少个byte
int mp_input(void *priv, u32 addr, void *buf, int len)
{
    dec_obj *obj = priv;
    vfs_seek(obj->p_file, addr, SEEK_SET);//addr为相对文件起始位置的偏移，len为多少个byte
    int rlen = vfs_read(obj->p_file, buf, len);
    return rlen;
}

//出数接口，data是出数的起始地址，len是长度（byte），priv为传入的
int mp_output(void *priv, void *data, int len)
{
    dec_obj *obj = priv;
    return sound_output(&obj->sound, data, len);
}
