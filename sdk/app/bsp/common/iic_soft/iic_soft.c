//#include "system/init.h"
#include "iic_soft.h"
#include "gpio.h"
extern void delay(unsigned int cnt);

//软件IIC设备数据初始化
const struct soft_iic_config soft_iic_cfg[] = {
    {
        //soft_iic0 data
        .scl = IO_PORTA_01,   //IIC CLK脚
        .sda = IO_PORTA_03,   //IIC DAT脚
        .delay = 60,          //软件IIC延时参数，影响通讯时钟频率
        .io_pu = 1,           //是否打开上拉电阻，如果外部电路没有焊接上拉电阻需要置1
    },
#if 0
    {
        //soft_iic1 data
        .scl = IO_PORTA_05,
        .sda = IO_PORTA_06,
        .delay = 60,
        .io_pu = 1,
    },
#endif
};


#define IIC_SCL_DIR(scl, val) \
    gpio_set_direction(scl, val)

#define IIC_SCL_SET_PU(scl, on) \
    gpio_set_pull_up(scl, on)

#define IIC_SCL_SET_PD(scl, on) \
    gpio_set_pull_down(scl, on)

#define IIC_SCL_SET_DIE(scl, on) \
    gpio_set_die(scl, on)

#define IIC_SCL_H(scl)    \
    gpio_write(scl, 1)

#define IIC_SCL_L(scl)    \
    gpio_write(scl, 0)

#define IIC_SDA_DIR(sda, val) \
    gpio_set_direction(sda, val)

#define IIC_SDA_SET_PU(sda, on) \
    gpio_set_pull_up(sda, on)

#define IIC_SDA_SET_PD(sda, on) \
    gpio_set_pull_down(sda, on)

#define IIC_SDA_SET_DIE(sda, on) \
    gpio_set_die(sda, on)

#define IIC_SDA_H(sda)    \
    gpio_write(sda, 1)

#define IIC_SDA_L(sda)    \
    gpio_write(sda, 0)

#define IIC_SDA_READ(sda) \
    gpio_read(sda)

#define iic_get_id(iic)     (iic)

static inline u32 iic_get_scl(soft_iic_dev iic)
{
    u8 id = iic_get_id(iic);
    return soft_iic_cfg[id].scl;
}

static inline u32 iic_get_sda(soft_iic_dev iic)
{
    u8 id = iic_get_id(iic);
    return soft_iic_cfg[id].sda;
}

static inline u32 iic_get_delay(soft_iic_dev iic)
{
    u8 id = iic_get_id(iic);
    return soft_iic_cfg[id].delay;
}

static inline u32 iic_get_io_pu(soft_iic_dev iic)
{
    u8 id = iic_get_id(iic);
    return soft_iic_cfg[id].io_pu;
}


int soft_iic_init(soft_iic_dev iic)
{
    u32 scl, sda;

    scl = iic_get_scl(iic);
    sda = iic_get_sda(iic);

    IIC_SCL_DIR(scl, 0);
    IIC_SCL_SET_PD(scl, 0);
    IIC_SCL_SET_DIE(scl, 1);
    IIC_SCL_L(scl);
    IIC_SDA_DIR(sda, 0);
    IIC_SDA_SET_PD(sda, 0);
    IIC_SDA_SET_DIE(sda, 1);
    IIC_SDA_H(sda);
    IIC_SCL_H(scl);
    if (iic_get_io_pu(iic)) {
        IIC_SCL_SET_PU(scl, 1);
        IIC_SDA_SET_PU(sda, 1);
    } else {
        IIC_SCL_SET_PU(scl, 0);
        IIC_SDA_SET_PU(sda, 0);
    }
    return 0;
}

void soft_iic_uninit(soft_iic_dev iic)
{
    u32 scl, sda;

    scl = iic_get_scl(iic);
    sda = iic_get_sda(iic);

    IIC_SCL_DIR(scl, 1);
    IIC_SCL_SET_PU(scl, 0);
    IIC_SCL_SET_PD(scl, 0);
    IIC_SCL_SET_DIE(scl, 0);
    IIC_SDA_DIR(sda, 1);
    IIC_SDA_SET_PU(sda, 0);
    IIC_SDA_SET_PD(sda, 0);
    IIC_SDA_SET_DIE(sda, 0);
}

void soft_iic_suspend(soft_iic_dev iic)
{
    soft_iic_uninit(iic);
}

void soft_iic_resume(soft_iic_dev iic)
{
    soft_iic_init(iic);
}

void soft_iic_start(soft_iic_dev iic)
{
    u32 scl, sda, dly_t;

    scl = iic_get_scl(iic);
    sda = iic_get_sda(iic);
    dly_t = iic_get_delay(iic);

    IIC_SDA_H(sda);
    delay(dly_t);

    IIC_SCL_H(scl);
    delay(dly_t * 2);

    IIC_SDA_L(sda);
    delay(dly_t);

    IIC_SCL_L(scl);
    delay(dly_t);
}

void soft_iic_stop(soft_iic_dev iic)
{
    u32 scl, sda, dly_t;

    scl = iic_get_scl(iic);
    sda = iic_get_sda(iic);
    dly_t = iic_get_delay(iic);

    IIC_SDA_L(sda);
    delay(dly_t);

    IIC_SCL_H(scl);
    delay(dly_t * 2);

    IIC_SDA_H(sda);
    delay(dly_t);
}

static u8 soft_iic_check_ack(soft_iic_dev iic)
{
    u8 ack;
    u32 scl, sda, dly_t;

    scl = iic_get_scl(iic);
    sda = iic_get_sda(iic);
    dly_t = iic_get_delay(iic);

    IIC_SDA_DIR(sda, 1);
    IIC_SCL_L(scl);
    delay(dly_t);

    IIC_SCL_H(scl);
    delay(dly_t);

    if (IIC_SDA_READ(sda) == 0) {
        ack = 1;
    } else {
        ack = 0;
    }
    delay(dly_t);

    IIC_SCL_L(scl);
    delay(dly_t);
    IIC_SDA_DIR(sda, 0);
    IIC_SDA_L(sda);

    return ack;
}

static void soft_iic_rx_ack(soft_iic_dev iic)
{
    u32 scl, sda, dly_t;

    scl = iic_get_scl(iic);
    sda = iic_get_sda(iic);
    dly_t = iic_get_delay(iic);

    IIC_SDA_L(sda);
    delay(dly_t);

    IIC_SCL_H(scl);
    delay(dly_t * 2);

    IIC_SCL_L(scl);
    delay(dly_t);
}

static void soft_iic_rx_nack(soft_iic_dev iic)
{
    u32 scl, sda, dly_t;

    scl = iic_get_scl(iic);
    sda = iic_get_sda(iic);
    dly_t = iic_get_delay(iic);

    IIC_SDA_H(sda);
    delay(dly_t);

    IIC_SCL_H(scl);
    delay(dly_t * 2);

    IIC_SCL_L(scl);
    delay(dly_t);
}

u8 soft_iic_tx_byte(soft_iic_dev iic, u8 byte)
{
    u8 i, ret;
    u32 scl, sda, dly_t;

    scl = iic_get_scl(iic);
    sda = iic_get_sda(iic);
    dly_t = iic_get_delay(iic);

    IIC_SCL_L(scl);
    for (i = 0; i < 8; i++) {  //MSB FIRST
        if ((byte << i) & 0x80) {
            IIC_SDA_H(sda);
        } else {
            IIC_SDA_L(sda);
        }
        delay(dly_t);

        IIC_SCL_H(scl);
        delay(dly_t * 2);

        IIC_SCL_L(scl);
        delay(dly_t);
    }
    return soft_iic_check_ack(iic);
}

u8 soft_iic_rx_byte(soft_iic_dev iic, u8 ack)
{
    u8 byte = 0, i;
    u32 scl, sda, dly_t;

    scl = iic_get_scl(iic);
    sda = iic_get_sda(iic);
    dly_t = iic_get_delay(iic);

    IIC_SDA_DIR(sda, 1);

    for (i = 0; i < 8; i++) {
        delay(dly_t);

        IIC_SCL_H(scl);
        delay(dly_t);

        byte = byte << 1;
        if (IIC_SDA_READ(sda)) {
            byte |= 1;
        }
        delay(dly_t);

        IIC_SCL_L(scl);
        delay(dly_t);
    }

    IIC_SDA_DIR(sda, 0);
    if (ack) {
        soft_iic_rx_ack(iic);
    } else {
        soft_iic_rx_nack(iic);
    }

    return byte;
}


int soft_iic_read_buf(soft_iic_dev iic, void *buf, int len)
{
    int i = 0;

    if (!buf || !len) {
        return -1;
    }
    for (i = 0; i < len - 1; i++) {
        ((u8 *)buf)[i] = soft_iic_rx_byte(iic, 1);
    }
    ((u8 *)buf)[len - 1] = soft_iic_rx_byte(iic, 0);
    return len;
}

int soft_iic_write_buf(soft_iic_dev iic, const void *buf, int len)
{
    int i;
    u8 ack;

    if (!buf || !len) {
        return -1;
    }
    for (i = 0; i < len; i++) {
        ack = soft_iic_tx_byte(iic, ((u8 *)buf)[i]);
        if (ack == 0) {
            break;
        }
    }
    return i;
}

