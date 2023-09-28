
#include "vfs.h"
#include "my_malloc.h"
#include "device.h"
#include "update.h"
#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#define USE_SD_SIMP_FAT 1
u16 chip_crc16_with_init(void *ptr, u32  len, u32 init);
u32 jlfs_get_idle_bank_info(u32 *bank_addr, u32 *bank_size);
u32 get_flash_alignsize(void);
u32 jlfs_updata_dual_bank_info(u32 bank_addr, u16 data_crc);
u32 jlfs_check_dual_bank_info(u32 update_bank_addr);
/* 升级流程中不可运行其他操作flash的流程 */
void dual_bank_test()
{
    log_info("----------------- app 1------------------");

    u32 res = 0;
    void *flash_dev = dev_open("sfc", NULL);
    if (flash_dev == NULL) {
        log_error("flash_dev null !!!! \n");
        res = DEVIVE_OPEN_ERROR;
        goto __close_flash_dev;
    }
    const char *dev_name = __SD0_NANE;
    log_info("dev name %s\n", dev_name);
    struct device *device = dev_open((char *)dev_name, NULL);
    if (device == NULL) {
        log_error("device null !!!! \n");
        res = DEVIVE_OPEN_ERROR;
        goto __close_dev;
    }
    void *pfs = NULL, *pfile = NULL;
    res = vfs_mount(&pfs, (void *)device, "fat");
    if (res) {
        log_error("fat mount error !!! \n");
        res = FAT_MOUNT_ERROR;
        goto __close_fs;
    }

    const char *file_path = "/db_data.bin";

    log_info("file_path %s\n", file_path);
    res = vfs_openbypath(pfs, &pfile, file_path);
    if (res) {
        log_error("opend file error !!! 0x%x\n", res);
        res = FILE_OPEN_ERROR;
        goto __close_fs;
    }

    struct vfs_attr file_attr;

#if USE_SD_SIMP_FAT//simple_fat没有获取文件长度接口
    u32 len_cnt = 0;
    u32 f_offset = 0;
    u8 *fr_buf = (u8 *)my_malloc(512, 0);
    while (1) {
        u32 rlen = 0;
        vfs_seek(pfile, f_offset, SEEK_SET);
        rlen = vfs_read(pfile, fr_buf, 512);
        len_cnt += rlen;
        if (rlen == 0) {
            break;
        }
        f_offset += rlen;
        wdt_clear();
    }
    file_attr.fsize = len_cnt;
    my_free((void *)fr_buf);
#else
    vfs_ioctl(pfile, FS_IOCTL_FILE_ATTR, (int)&file_attr);
#endif
    log_info("file_attr.fsize 0x%x\n", file_attr.fsize);

    file_attr.fsize += 4096;

    int file_size = file_attr.fsize;



    u32 upgrade_start_addr = 0;
    u32 bank_size;
    res = jlfs_get_idle_bank_info(&upgrade_start_addr, &bank_size);
    log_info("addr %x,size %x\n", upgrade_start_addr, bank_size);
    if (res) {
        log_error("opend bank file error !!! 0x%x\n", res);
        res = FILE_OPEN_ERROR;
        goto __write_end;
    }

    if (bank_size < file_size) {
        log_error("bank size too samll !!! 0x%x < \n", bank_size, file_size);
        res = FILE_OPEN_ERROR;
        goto __write_end;
    }


    u8 flash_erase_cmd = IOCTL_ERASE_SECTOR;

    u32 flash_alignsize = get_flash_alignsize();

    if (flash_alignsize == 256) {
        flash_erase_cmd = IOCTL_ERASE_PAGE;
    }


    log_info("erase unit %d", file_size / flash_alignsize);
    int erase_err = 0;
    for (u32 i = 0; i < (file_size / flash_alignsize); i ++) {
        erase_err = dev_ioctl(flash_dev, flash_erase_cmd, upgrade_start_addr + i * flash_alignsize);
        if (erase_err) {
            log_error("erase %x", upgrade_start_addr + i * flash_alignsize);
            break;
        }
        wdt_clear();
        log_info("erase %x", upgrade_start_addr + i * flash_alignsize);
    }

    u8 *tmp_buf = (u8 *)my_malloc(512, 0);

    //write data --> flash
    file_size = file_attr.fsize;

    u32 offset = 0;
    u32 data_crc = 0;
    while (file_size) {
        vfs_seek(pfile, offset, 0);
        u32 cnt = file_size > 512 ? 512 : file_size;
        vfs_read(pfile, tmp_buf, cnt);
        data_crc = chip_crc16_with_init(tmp_buf, cnt, data_crc);
        dev_byte_write(flash_dev, tmp_buf, upgrade_start_addr + offset, cnt);
        log_info("write %x", upgrade_start_addr + offset);
        offset += cnt;
        file_size -= cnt;
        wdt_clear();
    }

    log_info("data crc %x\n", data_crc);
    dev_ioctl(flash_dev, IOCTL_SET_SFC_READ, 1);

    //verify data
    file_size = file_attr.fsize - 4096;
    log_info("upgrade_start_addr 0x%x\n", upgrade_start_addr);
    offset = 0;
    u8 *f_tmp_buf = (u8 *)my_malloc(512, 0);
    while (file_size) {
        vfs_seek(pfile, offset, 0);
        u32 cnt = file_size > 512 ? 512 : file_size;
        memset(tmp_buf, 0, 512);
        memset(f_tmp_buf, 0, 512);
        vfs_read(pfile, tmp_buf, cnt);
        dev_byte_read(flash_dev, f_tmp_buf, upgrade_start_addr + offset, cnt);
        /* log_info("\n \n"); */
        /* log_info("file data"); */
        /* put_buf((u8 *)tmp_buf, 512); */
        /* log_info("flash data"); */
        /* put_buf((u8 *)f_tmp_buf, 512); */
        for (u16 i = 0; i < 512; i++) {
            if (tmp_buf[i] != f_tmp_buf[i]) {
                log_error("verify data err  i %d\n", i);
                goto __verify_fail;
            }
        }
        offset += cnt;
        file_size -= cnt;
        wdt_clear();
    }
    dev_ioctl(flash_dev, IOCTL_SET_SFC_READ, 0);
    my_free((void *)f_tmp_buf);
    my_free((void *)tmp_buf);

    jlfs_updata_dual_bank_info(upgrade_start_addr, data_crc);

    dev_ioctl(flash_dev, IOCTL_SET_SFC_READ, 1);
    u32 head_check_res = jlfs_check_dual_bank_info(upgrade_start_addr);
    if (head_check_res) {
        log_error("head verify fail\n");
        goto __verify_fail;
    }

    dev_ioctl(flash_dev, IOCTL_SET_SFC_READ, 0);
    vfs_file_close(&pfile);
    vfs_fs_close(&pfs);
    dev_close(device);
    dev_close(flash_dev);

    log_info("dual bank update success, waiting for reset\n");
    while (1);


__verify_fail:
    dev_ioctl(flash_dev, IOCTL_SET_SFC_READ, 0);
    dev_ioctl(flash_dev, IOCTL_ERASE_SECTOR, upgrade_start_addr);
    my_free((void *)f_tmp_buf);
    my_free((void *)tmp_buf);
__write_end:
__close_file:
    vfs_file_close(&pfile);
__close_fs:
    vfs_fs_close(&pfs);
__close_dev:
    dev_close(device);
__close_flash_dev:
    dev_close(flash_dev);
    log_error("dual bank update fail\n");
    return;
}


