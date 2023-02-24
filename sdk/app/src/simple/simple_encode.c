#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "dev_manage.h"
#include "vfs.h"
#include "msg.h"
#include "bsp_loop.h"
//----
#include "boot.h"
#include "vm.h"
#include "device.h"
#include "encoder_mge.h"
#include "a_encoder.h"
#include "mp3_encoder.h"
/* #include "decoder_api.h" */
#include "simple_encode.h"
#include "simple_decode.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"


#define debug_demo_show(ptr,err)            \
    if(0 != err){                           \
        log_info("Demo 0x%x : %s",err,ptr);   \
        return ;                            \
    }


u8 e_status;// ENC_STATUS e_status;



extern void norfs_demo(void);

void encode_stop_demo(void **ppfile)
{
    if (ENC_ING == e_status) {
        stop_encode(*ppfile, 0);
        vfs_file_close(ppfile);
        e_status = ENC_NULL;

    }
}

extern void simple_next(void);
play_control epctl;
void encode_demo(void)
{
    char c;
    u32 err;
    void *device = 0;
    void *pfs = 0;
    void *pfile = 0;
    u32 file_total = 0;
    dec_obj *d_obj = 0;

    e_status = ENC_NULL;
    /* norfs_demo(); */
    /* return; */
    dac_vol(0, 31);


    log_info("encode_demo\n");
    device = dev_open(__SFC_NANE, 0);

    if (NULL == device) {
        log_info("device null\n");
        return;
    }
    vfs_mount(&pfs, device, "norfs");
    err = vfs_ioctl(pfs, FS_IOCTL_FS_TOTAL, (int)&file_total);

    u32 findex  = 12;
    u32 flen = 0;

    int msg[2];
    while (1) {
        c = get_byte();
        if (0 ==  c) {
            get_msg(2, &msg[0]);
        } else {
            if ('N' == c) {
                audio_adc_off_api();
                encode_stop_demo(&pfile);
                simple_next();
                return ;
            }
            if ('s' == c) {
                msg[0] = MSG_RECODE_START;
            } else if ('e' == c) {
                msg[0] = MSG_RECODE_END;
            } else if ('p' == c) {
                msg[0] = MSG_PP;
            }
        }
        bsp_loop();
        switch (msg[0]) {
        case MSG_500MS:
            wdt_clear();
            /* log_char('5'); */
            break;
        case MSG_RECODE_START:

            err = audio_adc_init_api(24000, ADC_MIC, 1);
            if (0 != err) {
                log_info(" audio adc init fail : 0x%x\n");
                break;
            }

            if (ENC_ING == e_status) {
                encode_stop_demo(&pfile);
                post_msg(1, MSG_PP);
            } else {
                err = vfs_createfile(pfs, &pfile, &findex);
                if (0 != err) {
                    log_info("vfs create 0x%x!\n", err);
                    break;
                }
                log_info(" file index : %d\n", findex);
                encoder_io(ump3_encode_api, pfile);
                e_status = ENC_ING;
            }
            break;
        case MSG_WFILE_FULL:
        case MSG_RECODE_END:

            audio_adc_off_api();

            encode_stop_demo(&pfile);
            log_info(" recode end\n");
            log_info(" file index : %d\n", findex);
            break;
        case MSG_PP:
            encode_stop_demo(&pfile);
            memset(&epctl, 0, sizeof(epctl));
            epctl.type = BIT_UMP3 | BIT_A | BIT_SPEED;
            epctl.pfile = pfile;
            epctl.findex = findex;
            /* epctl.loop = 255;   //0 ~ 254无缝循环次数；255一直无缝循环播放 */
            d_obj = decoder_by_index(pfs, &epctl);
            /* d_obj = decoder_by_index(pfs, &pfile, 0, findex, BIT_A); */
            if (0 == d_obj) {
                log_info("err recode open file : 0x%x\n", err);
                break;
            }
            break;
        case MSG_WAV_FILE_END:
        case MSG_MP3_FILE_END:
        case MSG_A_FILE_END:
            decoder_stop(d_obj, NEED_WAIT);
            break;

        case MSG_WAV_FILE_ERR:
        case MSG_MP3_FILE_ERR:
        case MSG_A_FILE_ERR:
            decoder_stop(d_obj, NO_WAIT);
            break;
        }

        ;
    }
    return;
}


#if 0
u8 rt_buff[512];
/* extern void read_flash_id(void); */
void norfs_demo(void)
{
    char c;
    u32 resi, err;
    s32 decoder_index;
    struct vfs_attr attr;
    u32 capacity;
    void *device = 0;
    void *pfs = 0;
    void *pfile = 0;
    u32 file_total = 0;

    device = dev_open(__SFC_NANE, 0);

    if (NULL == device) {
        log_info("device null\n");
        return;
    } else {
        log_info("sfc dev ok!\n");
    }
#if 0
    u32 addr = boot_info.vm.vm_saddr + 1024 * 4;
    log_info("device 001\n");
    JL_PORTA->OUT |= BIT(1);
    dev_ioctl(device, IOCTL_ERASE_SECTOR, addr);
    JL_PORTA->OUT &= ~BIT(1);
    log_info("device 002 0\n");
    dev_bulk_read(device, &rt_buff[0], addr, 512);
    log_info_hexdump(&rt_buff[0], 512);
    /* log_info_hexdump(&rt_buff[0], 512); */
    memset(&rt_buff[0], 0xBD, 512);
    JL_PORTA->OUT |= BIT(1);
    dev_byte_write(device, &rt_buff[0], addr, 512);

    JL_PORTA->OUT &= ~BIT(1);
    memset(&rt_buff[0], 0xff, 512);
    log_info_hexdump(&rt_buff[0], 512);
    JL_PORTA->OUT |= BIT(1);
    dev_byte_read(device, &rt_buff[0], addr, 512);
    JL_PORTA->OUT &= ~BIT(1);
    log_info_hexdump(&rt_buff[0], 512);
    /* wdt_clear(); */
#else
    vfs_mount(&pfs, device, "norfs");
    err = vfs_ioctl(pfs, FS_IOCTL_FS_TOTAL, (int)&file_total);
    ASSERT(!err, "ERR vfs fs total : 0x%x\n", err);
    log_info("norfs file_total %d\n", file_total);
    /* return; */
    for (u32 i = 0; i < 512; i++) {
        rt_buff[i] = i;
    }

    log_info_hexdump(rt_buff, 512);
    u32 findex  = 12345678;
    u32 flen = 0;
    /* err = vfs_createfile(pfs, &pfile, &findex); */
    /* ASSERT(!err, "ERR vfs createfile : 0x%x\n", err); */
    /* log_info(" file index : 0x%x\n", findex); */
    /* err = vfs_write(pfile, rt_buff, 512); */
    /* ASSERT((512 == err), " ERR vfs createfile : 0x%x\n", err); */
    /* log_info(" vfs write 512 : %d\n", err); */
    /* err = vfs_ioctl(pfile, FS_IOCTL_FILE_SYNC, (int)&flen); */
    /* ASSERT(!err, "ERR vfs FS_IOCTL_FILE_SYNC  : 0x%x\n", err); */
    /* log_info(" vfs file size : %d\n", flen); */
    /* err = vfs_file_close(&pfile); */
    /* ASSERT(!err, "ERR vfs close file w : 0x%x\n", err); */
    /* log_info("close file : 0x%x\n", (int)pfile); */

    /* ASSERT(!err,"ERR vfs  : 0x%x\n",err); */
    findex = 7;
    err = vfs_openbyindex(pfs, &pfile, findex);
    ASSERT(!err, "ERR vfs openbyindex  : 0x%x\n", err);
    do {
        memset(&rt_buff[0], 0xff, 512);
        /* log_info_hexdump(rt_buff, 512); */
        err = vfs_read(pfile, rt_buff, 512);
        /* ASSERT((512 != err), "ERR vfs read : 0x%x\n", err); */
        log_info_hexdump(rt_buff, err);
    } while (err  == 512);

    /* err = vfs_file_close(&pfile); */
    /* ASSERT(!err, "ERR vfs close file r : 0x%x\n", err); */

#endif
}
#endif



