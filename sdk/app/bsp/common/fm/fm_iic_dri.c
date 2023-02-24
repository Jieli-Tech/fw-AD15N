#include "includes.h"
#include "iic_hw.h"
#include "iic_soft.h"
#include "fm_iic_dri.h"
#include "app_config.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[fm-iic-dri]"
#include "log.h"

#if FM_EN

#if(FM_SD_IO_REUSE)  //SD FM IO复用
static u8 iic_busy = 0;
#include "sdmmc/sd_host_api.h"
#endif

/*******************************FM 操作函数******************************/
#define FM_IIC_SELECT 0
#if(FM_IIC_SELECT)
#define fm_iic_dev                IIC_0 //硬件IIC设备选择
#define fm_iic_init(iic)                       hw_iic_init(iic)
#define fm_iic_uninit(iic)                     hw_iic_uninit(iic)
#define fm_iic_start(iic)                      hw_iic_start(iic)
#define fm_iic_stop(iic)                       hw_iic_stop(iic)
#define fm_iic_tx_byte(iic, byte)              hw_iic_tx_byte(iic, byte)
#define fm_iic_rx_byte(iic, ack)               hw_iic_rx_byte(iic, ack)
#define fm_iic_read_buf(iic, buf, len)         hw_iic_read_buf(iic, buf, len)
#define fm_iic_write_buf(iic, buf, len)        hw_iic_write_buf(iic, buf, len)
#define fm_iic_suspend(iic)                    hw_iic_suspend(iic)
#define fm_iic_resume(iic)                     hw_iic_resume(iic)
#else
#define fm_iic_dev                0 //软件IIC设备选择
#define fm_iic_init(iic)                       soft_iic_init(iic)
#define fm_iic_uninit(iic)                     soft_iic_uninit(iic)
#define fm_iic_start(iic)                      soft_iic_start(iic)
#define fm_iic_stop(iic)                       soft_iic_stop(iic)
#define fm_iic_tx_byte(iic, byte)              soft_iic_tx_byte(iic, byte)
#define fm_iic_rx_byte(iic, ack)               soft_iic_rx_byte(iic, ack)
#define fm_iic_read_buf(iic, buf, len)         soft_iic_read_buf(iic, buf, len)
#define fm_iic_write_buf(iic, buf, len)        soft_iic_write_buf(iic, buf, len)
#define fm_iic_suspend(iic)                    soft_iic_suspend(iic)
#define fm_iic_resume(iic)                     soft_iic_resume(iic)
#endif

void fm_i2c_init()
{
#if(FM_IIC_SELECT)
    log_info("-------hw_iic-------\n");
#else
    log_info("-------soft_iic-------\n");
#endif
#if(FM_SD_IO_REUSE)
    iic_busy = 1;
    if (sd_io_suspend(0, 1) == 0) {
#endif
        fm_iic_init(fm_iic_dev);
#if(FM_SD_IO_REUSE)
        fm_iic_suspend(fm_iic_dev);
    } else {
        log_error("sd io suspend fail!");
    }
    iic_busy = 0;
    sd_io_resume(0, 1);
#endif
}
/*----------------------------------------------------------------------------*/
/**@brief   IIC写函数
   @param   chip_id ：目标IC的ID号
   @param   reg_addr: 目标IC的目标寄存器的地址
   @param   *iic_dat: 写望目标IC的数据的指针
   @param   len:需要写的数据的数目
   @return  无
   @note    void  iic_write(u8 chip_id,u8 reg_addr,u8 *iic_dat,u8 len)
*/
/*----------------------------------------------------------------------------*/
void _fm_iic_write(u8 chip_id, u8 reg_addr, u8 *iic_dat, u8 len)
{
    /* g_iic_busy  = 1; */
    fm_iic_start(fm_iic_dev);                //I2C启动
    fm_iic_tx_byte(fm_iic_dev, chip_id);         //写命令

    if (0xff != reg_addr) {
        fm_iic_tx_byte(fm_iic_dev, reg_addr);   //写地址
    }
    for (; len > 1; len--) {
        fm_iic_tx_byte(fm_iic_dev, *iic_dat++);      //写数据
    }
#if(FM_IIC_SELECT)
    fm_iic_stop(fm_iic_dev);                 //I2C停止时序
    fm_iic_tx_byte(fm_iic_dev, *iic_dat++);      //写数据
#else
    fm_iic_tx_byte(fm_iic_dev, *iic_dat++);      //写数据
    fm_iic_stop(fm_iic_dev);                 //I2C停止时序
#endif
    /* g_iic_busy = 0; */
}

void fm_iic_write(u8 chip_id, u8 reg_addr, u8 *iic_dat, u8 len)
{
#if(FM_SD_IO_REUSE)
    iic_busy = 1;
    if (sd_io_suspend(0, 1) == 0) {
        fm_iic_resume(fm_iic_dev);
#endif
        _fm_iic_write(chip_id, reg_addr, iic_dat, len);
#if(FM_SD_IO_REUSE)
        fm_iic_suspend(fm_iic_dev);
    } else {
        log_error("sd io suspend fail!");
    }
    iic_busy = 0;
    sd_io_resume(0, 1);
#endif
}
/*----------------------------------------------------------------------------*/
/**@brief   IIC总线向一个目标ID读取几个数据
   @param   chip_id ：目标IC的ID号
   @param   reg_addr: 目标IC的目标寄存器的地址
   @param   *iic_dat     :  存档读取到的数据的buffer指针
   @param   len :  需要读取的数据的个数
   @return  无
   @note    void iic_readn(u8 chip_id, u8 reg_addr, u8 *iic_dat, u8 len)
*/
/*----------------------------------------------------------------------------*/
void _fm_iic_readn(u8 chip_id, u8 reg_addr, u8 *iic_dat, u8 len)
{
    /* g_iic_busy = 1; */
    fm_iic_start(fm_iic_dev);                //I2C启动
    fm_iic_tx_byte(fm_iic_dev, chip_id);         //读命令
    if (0xff != reg_addr) {
        fm_iic_tx_byte(fm_iic_dev, reg_addr);   //写地址
    }
    for (; len > 1; len--) {
        *iic_dat++ = fm_iic_rx_byte(fm_iic_dev, 1);      //读数据
    }
#if(FM_IIC_SELECT)
    fm_iic_stop(fm_iic_dev);                 //I2C停止时序
    *iic_dat++ = fm_iic_rx_byte(fm_iic_dev, 0);
#else
    *iic_dat++ = fm_iic_rx_byte(fm_iic_dev, 0);
    fm_iic_stop(fm_iic_dev);                 //I2C停止时序
#endif
    /* g_iic_busy = 0; */
}

void fm_iic_readn(u8 chip_id, u8 reg_addr, u8 *iic_dat, u8 len)
{
#if(FM_SD_IO_REUSE)
    iic_busy = 1;
    if (sd_io_suspend(0, 1) == 0) {
        fm_iic_resume(fm_iic_dev);
#endif
        _fm_iic_readn(chip_id, reg_addr, iic_dat, len);
#if(FM_SD_IO_REUSE)
        fm_iic_suspend(fm_iic_dev);
    } else {
        log_error("sd io suspend fail!");
    }
    iic_busy = 0;
    sd_io_resume(0, 1);
#endif
}

u8 _QND_ReadReg(u8 reg_addr)
{
    u8  byte;
    /* g_iic_busy = 1; */
    fm_iic_start(fm_iic_dev);                    //I2C启动
    fm_iic_tx_byte(fm_iic_dev, 0x20);             //写命令
    fm_iic_tx_byte(fm_iic_dev, reg_addr);         //写地址
    fm_iic_start(fm_iic_dev);                    //写转为读命令，需要再次启动I2C
    fm_iic_tx_byte(fm_iic_dev, 0x21);             //读命令
#if(FM_IIC_SELECT)
    fm_iic_stop(fm_iic_dev);                 //I2C停止时序
    byte = fm_iic_rx_byte(fm_iic_dev, 0);
#else
    byte = fm_iic_rx_byte(fm_iic_dev, 0);
    fm_iic_stop(fm_iic_dev);                 //I2C停止时序
#endif
    fm_iic_stop(fm_iic_dev);                     //I2C停止
    /* g_iic_busy = 0; */
    return  byte;
}

u8 QND_ReadReg(u8 reg_addr)
{
    u8  byte = 0;
#if(FM_SD_IO_REUSE)
    iic_busy = 1;
    if (sd_io_suspend(0, 1) == 0) {
        fm_iic_resume(fm_iic_dev);
#endif
        byte = _QND_ReadReg(reg_addr);
#if(FM_SD_IO_REUSE)
        fm_iic_suspend(fm_iic_dev);
    } else {
        log_error("sd io suspend fail!");
    }
    iic_busy = 0;
    sd_io_resume(0, 1);
#endif
    return  byte;
}
#endif
