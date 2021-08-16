#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "decoder_api.h"
#include "vfs.h"
#include "circular_buf.h"
#include "dac.h"
/* #include "resample.h" */
#include "msg.h"
#include "errno-base.h"
#include "midi_api.h"
#include "MIDI_CTRL_API.h"
#include "MIDI_DEC_API.h"
#include "boot.h"
#include "decoder_msg_tab.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"



static u32 midi_tone_tab = 0;
cbuffer_t cbuf_midi AT(.midi_buf);
dec_obj dec_midi_hld;
u16 obuf_midi[DAC_DECODER_BUF_SIZE / 2] AT(.midi_buf) ;
u32 midi_decode_buff[(4984 + 3) / 4] AT(.midi_buf) ;
#define MIDI_CAL_BUF ((void *)&midi_decode_buff[0])
static MIDI_CONFIG_PARM midi_t_parm AT(.midi_buf);
static MIDI_INIT_STRUCT init_info AT(.midi_buf);


const struct if_decoder_io midi_dec_io0 = {
    &dec_midi_hld,      //input跟output函数的第一个参数，解码器不做处理，直接回传，可以为NULL
    mp_input,
    mp_output,
};


static u8 midi_musicsr_to_cfgsr(u32 sr)
{
    const int midi_sr_tab[] = {
        48000,
        44100,
        32000,
        24000,
        22050,
        16000,
        12000,
        11025,
        8000,
    };

    for (int i = 0; i < sizeof(midi_sr_tab) / sizeof(midi_sr_tab[0]); i++) {
        if (sr == midi_sr_tab[i]) {
            return i;
        }
    }

    ASSERT(0, "midi error sr !!!:%d \n", sr);
    return 0;
}

__attribute__((weak))
void midi_init_info(MIDI_INIT_STRUCT *init_info)
{

}

u32 midi_decode_api(void *p_file, void **ppdec, void *p_dp_buf)
{
    dec_obj **p_dec = (dec_obj **)ppdec;
    u32 buff_len, sr;
    decoder_ops_t *ops;
    log_info("midi_decode_api\n");
    if (!midi_tone_tab) {
        return E_MIDI_NO_CFG;
    }

    local_irq_disable();
    memset(&dec_midi_hld, 0, sizeof(dec_obj));
    local_irq_enable();

    dec_midi_hld.type = D_TYPE_MIDI;

    ops = get_midi_ops();
    buff_len = ops->need_dcbuf_size();
    log_info("buff_len:%d \n", buff_len);
    if (buff_len > sizeof(midi_decode_buff)) {
        return E_MIDI_DBUF;
    }
    /******************************************/
    cbuf_init(&cbuf_midi, &obuf_midi[0], sizeof(obuf_midi));
    /* char name[VFS_FILE_NAME_LEN] = {0}; */
    int file_len = vfs_file_name(p_file, (void *) g_file_sname, sizeof(g_file_sname));
    /* u8 *name = vfs_file_name(p_file); */
    log_info("file name : %s\n", g_file_sname);

    dec_midi_hld.p_file = p_file;
    dec_midi_hld.sound.p_obuf = &cbuf_midi;
    dec_midi_hld.p_dbuf = MIDI_CAL_BUF;
    dec_midi_hld.dec_ops = ops;
    dec_midi_hld.event_tab = (u8 *)&midi_evt[0];
    //
    /******************************************/
    ops->open(MIDI_CAL_BUF, &midi_dec_io0, NULL);         //传入io接口，说明如下
    if (ops->format_check(MIDI_CAL_BUF)) {                  //格式检查
        return E_MIDIFORMAT;
    }
    /******************************************************************************/
    sr = ops->get_dec_inf(MIDI_CAL_BUF)->sr;                //获取采样率
    dec_midi_hld.sr = sr;
    //dac_sr_api(sr);
    log_info(">>>>>>>sr:%d \n", sr);
    /**************相对MP3的调用流程多了这个，其他一致。这个一定要配置***************/
    midi_t_parm.player_t = 8;                                //设置需要合成的最多按键个数，8到32可配
    midi_t_parm.sample_rate = midi_musicsr_to_cfgsr(sr);                            //采样率设为16k
    midi_t_parm.spi_pos = (u8 *)midi_tone_tab;                    //spi_memory为音色文件数据起始地址
    log_info_hexdump(midi_t_parm.spi_pos, 16);
    memset((u8 *)&init_info, 0x00, sizeof(init_info));
    init_info.init_info = midi_t_parm;
    midi_init_info(&init_info);
    ops->dec_confing(MIDI_CAL_BUF, CMD_INIT_CONFIG, &init_info);

    /**输出dec handle*/
    *p_dec = &dec_midi_hld;

    regist_dac_channel(&dec_midi_hld.sound, kick_decoder); //注册到DAC;

    return 0;
}


void midi_decode_init(void)
{
    void *pvfs = 0;
    void *pvfile = 0;
    u32 err = 0;

    err = vfs_mount(&pvfs, (void *)NULL, (void *)NULL);
    if (err != 0) {
        return;
    }

    err = vfs_openbypath(pvfs, &pvfile, "/midi_cfg/midi_cfg.bin");
    if (err != 0) {
        vfs_fs_close(&pvfs);
        return;
    }

    ///获取midi音色库的cache地址
    struct vfs_attr attr;
    vfs_get_attrs(pvfile, &attr);
    midi_tone_tab = boot_info.sfc.app_addr + attr.sclust;

    vfs_file_close(&pvfile);
    vfs_fs_close(&pvfs);
}

extern const u8 midi_buf_start[];
extern const u8 midi_buf_end[];
u32 midi_buff_api(dec_buf *p_dec_buf)
{
    p_dec_buf->start = (u32)&midi_buf_start[0];
    p_dec_buf->end   = (u32)&midi_buf_end[0];
    return 0;
}








