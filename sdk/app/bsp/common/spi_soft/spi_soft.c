#include "gpio.h"
#include "clock.h"
#include "spi_soft.h"
#include "spi1.h"
#include "includes.h"
#include "typedef.h"
#include "cpu.h"
#include "errno-base.h"
#include "config.h"

/* #define LOG_TAG_CONST NORM */
#define LOG_TAG     "[spi_soft]"
//#define LOG_ERROR_ENABLE
//#define LOG_DEBUG_ENABLE
#include "debug.h"

const struct spi_soft_platform_data spi_soft_p_data[] = {
    {
        //soft spi0
        .clk_pin = IO_PORTA_11,
        .do_pin = IO_PORTA_12,
        .di_pin = IO_PORTA_10,
        .d2_pin = 0xff,
        .d3_pin = 0xff,
        .data_wide_mode = SPI_MODE_BIDIR_1BIT,
        .work_mode = SPI_CPOL1_CPHA1,
        .clk = 10000000,      //软件spi延时参数，影响通讯时钟频率
        .role = SPI_ROLE_MASTER,
    },
#if 0
    {
        //soft spi1
        .clk_pin = IO_PORTA_00,
        .do_pin = IO_PORTA_01,
        .di_pin = IO_PORTA_02,
        .d2_pin = 0xff,
        .d3_pin = 0xff,
        .data_wide_mode = SPI_MODE_BIDIR_1BIT,
        .work_mode = SPI_CPOL1_CPHA1,
        .clk = 10000000,      //软件spi延时参数，影响通讯时钟频率
        .role = SPI_ROLE_MASTER,
    },
#endif
};
#define soft_spi_get_info_id(spi) (spi)

static void soft_spi_io_port_init(u8 port, u8 dir)
{
    if (port != (u8) - 1) {
        if (dir == 1) {
            gpio_set_direction(port, 1);
        } else {
            gpio_write(port, 1);
            gpio_set_direction(port, 0);
        }
        gpio_set_die(port, 1);
        gpio_set_pull_up(port, 0);
        gpio_set_pull_down(port, 0);
    }
}
static void soft_spi_io_port_uninit(u8 port)
{
    if (port != (u8) - 1) {
        gpio_set_direction(port, 1);
        gpio_set_die(port, 0);
        gpio_set_pull_up(port, 0);
        gpio_set_pull_down(port, 0);
    }
}

void soft_spi_set_bit_mode(spi_soft_dev spi, int data_wide_mode)
{
    u8 id = soft_spi_get_info_id(spi);
    switch (data_wide_mode) {
    case SPI_MODE_BIDIR_1BIT:
        soft_spi_io_port_init(spi_soft_p_data[id].di_pin, 1);
        break;
    case SPI_MODE_UNIDIR_1BIT:
        break;
    case SPI_MODE_UNIDIR_2BIT:
        soft_spi_io_port_init(spi_soft_p_data[id].di_pin, 0);
        break;
    case SPI_MODE_UNIDIR_4BIT:
        soft_spi_io_port_init(spi_soft_p_data[id].di_pin, 0);
        soft_spi_io_port_init(spi_soft_p_data[id].d2_pin, 0);
        soft_spi_io_port_init(spi_soft_p_data[id].d3_pin, 0);
        break;
    }
    soft_spi_io_port_init(spi_soft_p_data[id].clk_pin, spi_soft_p_data[id].role == SPI_ROLE_MASTER ? 0 : 1);
    soft_spi_io_port_init(spi_soft_p_data[id].do_pin, 0);
}
int soft_spi_set_baud(spi_soft_dev spi, u32 baud)
{
    /* u32 get_spiclk; */
    /* //SPICK = get_spiclk / (SPIx_BAUD + 1) */
    /* //=> SPIx_BAUD = get_spiclk / SPICK - 1 */
    /* u8 id = soft_spi_get_info_id(spi); */
    /* get_spiclk = clk_get("lsb"); */
    /* log_info("spi clock source freq %lu", get_spiclk); */
    /* if (get_spiclk < baud) { */
    /*     spi_w_reg_baud(spi_regs[id], 0); */
    /*     return -EINVAL; */
    /* } */
    /* spi_w_reg_baud(spi_regs[id], get_spiclk / baud - 1); */
    return 0;
}
u32 soft_spi_get_baud(spi_soft_dev spi)
{
    u8 id = soft_spi_get_info_id(spi);
    return spi_soft_p_data[id].clk;
}
int soft_spi_open(spi_soft_dev spi)
{
    int err = 0;
    u8 id = soft_spi_get_info_id(spi);
    soft_spi_set_bit_mode(spi, spi_soft_p_data[id].data_wide_mode);
    /* spi_smp_edge_rise(spi_regs[id]); */
    /* spi_ud_edge_fall(spi_regs[id]); */
    /* spi_cs_idle_h(spi_regs[id]); */
    /* spi_clk_idle_h(spi_regs[id]); */
    /* err = soft_spi_set_baud(spi, clock); */
    if (err) {
        log_error("invalid spi baudrate");
        return -1;
    }
    return 0;
}

void soft_spi_close(spi_soft_dev spi)
{
    u8 id = soft_spi_get_info_id(spi);
    switch (spi_soft_p_data[id].data_wide_mode) {
    case SPI_MODE_BIDIR_1BIT:
        soft_spi_io_port_uninit(spi_soft_p_data[id].di_pin);
        break;
    case SPI_MODE_UNIDIR_1BIT:
        break;
    case SPI_MODE_UNIDIR_2BIT:
        soft_spi_io_port_uninit(spi_soft_p_data[id].di_pin);
        break;
    case SPI_MODE_UNIDIR_4BIT:
        soft_spi_io_port_uninit(spi_soft_p_data[id].di_pin);
        soft_spi_io_port_uninit(spi_soft_p_data[id].d2_pin);
        soft_spi_io_port_uninit(spi_soft_p_data[id].d3_pin);
        break;
    }
    soft_spi_io_port_uninit(spi_soft_p_data[id].clk_pin);
    soft_spi_io_port_uninit(spi_soft_p_data[id].do_pin);
}
void soft_spi_suspend(spi_soft_dev spi)
{
    soft_spi_close(spi);
}
void soft_spi_resume(spi_soft_dev spi)
{
    soft_spi_open(spi);
}
/* CPOL = 0, CPHA = 0, MSB first, clk_l, smp_edge_rise*/
static u8 soft_spi_rw_mode0(spi_soft_dev spi, u8 write_dat)
{
    u8 i, read_dat = 0;
    u8 id = soft_spi_get_info_id(spi);
    gpio_write(spi_soft_p_data[id].clk_pin, 0);
    for (i = 0; i < 8; i++) {
        if (write_dat & 0x80) {
            gpio_write(spi_soft_p_data[id].do_pin, 1);
        } else {
            gpio_write(spi_soft_p_data[id].do_pin, 0);
        }
        write_dat <<= 1;
        delay(1);
        gpio_write(spi_soft_p_data[id].clk_pin, 1);
        read_dat <<= 1;
        if (gpio_read(spi_soft_p_data[id].di_pin)) {
            read_dat++;
        }
        delay(1);
        gpio_write(spi_soft_p_data[id].clk_pin, 0);
    }

    return read_dat;
}

/* CPOL=0，CPHA=1, MSB first, clk_l, smp_edge_fall*/
static u8 soft_spi_rw_mode1(spi_soft_dev spi, u8 byte)
{
    u8 i, Temp = 0;
    u8 id = soft_spi_get_info_id(spi);

    gpio_write(spi_soft_p_data[id].clk_pin, 0);
    for (i = 0; i < 8; i++) { // 循环8次
        gpio_write(spi_soft_p_data[id].clk_pin, 1);
        if (byte & 0x80) {
            gpio_write(spi_soft_p_data[id].do_pin, 1);
        } else {
            gpio_write(spi_soft_p_data[id].do_pin, 0);
        }
        byte <<= 1;     // 低一位移位到最高位
        delay(1);
        gpio_write(spi_soft_p_data[id].clk_pin, 0);
        Temp <<= 1;     //数据左移

        if (gpio_read(spi_soft_p_data[id].di_pin)) {
            Temp++;    //若从从机接收到高电平，数据自加一
        }
        delay(1);

    }
    return (Temp);     //返回数据
}

/* CPOL=1，CPHA=0, MSB first, clk_h, smp_edge_fall*/
static u8 soft_spi_rw_mode2(spi_soft_dev spi, u8 byte)
{
    u8 i, Temp = 0;
    u8 id = soft_spi_get_info_id(spi);

    gpio_write(spi_soft_p_data[id].clk_pin, 1);
    for (i = 0; i < 8; i++) { // 循环8次
        if (byte & 0x80) {
            gpio_write(spi_soft_p_data[id].do_pin, 1);
        } else {
            gpio_write(spi_soft_p_data[id].do_pin, 0);
        }
        byte <<= 1;     // 低一位移位到最高位
        delay(1);
        gpio_write(spi_soft_p_data[id].clk_pin, 0);
        Temp <<= 1;     //数据左移

        if (gpio_read(spi_soft_p_data[id].di_pin)) {
            Temp++;    //若从从机接收到高电平，数据自加一
        }
        delay(1);
        gpio_write(spi_soft_p_data[id].clk_pin, 1);

    }
    return (Temp);     //返回数据
}

/* CPOL = 1, CPHA = 1, MSB first, clk_h, smp_edge_rise */
static u8 soft_spi_rw_mode3(spi_soft_dev spi, u8 write_dat)
{
    u8 i, read_dat = 0;
    u8 id = soft_spi_get_info_id(spi);
    gpio_write(spi_soft_p_data[id].clk_pin, 1);
    for (i = 0; i < 8; i++) {
        gpio_write(spi_soft_p_data[id].clk_pin, 0);
        if (write_dat & 0x80) {
            gpio_write(spi_soft_p_data[id].do_pin, 1);
        } else {
            gpio_write(spi_soft_p_data[id].do_pin, 0);
        }
        write_dat <<= 1;
        delay(1);
        gpio_write(spi_soft_p_data[id].clk_pin, 1);
        read_dat <<= 1;
        if (gpio_read(spi_soft_p_data[id].di_pin)) {
            read_dat++;
        }
        delay(1);
    }
    return read_dat;
}

int soft_spi_send_byte(spi_soft_dev spi, u8 byte)
{
    u8 id = soft_spi_get_info_id(spi);
    switch (spi_soft_p_data[id].work_mode) {
    case SPI_CPOL0_CPHA0:
        soft_spi_rw_mode0(spi, byte);
        break;
    case SPI_CPOL0_CPHA1:
        soft_spi_rw_mode1(spi, byte);
        break;
    case SPI_CPOL1_CPHA0:
        soft_spi_rw_mode2(spi, byte);
        break;
    case SPI_CPOL1_CPHA1:
        soft_spi_rw_mode3(spi, byte);
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

u8 soft_spi_recv_byte(spi_soft_dev spi, int *err)
{
    u8 id = soft_spi_get_info_id(spi);
    u8 rx_data;
    switch (spi_soft_p_data[id].work_mode) {
    case SPI_CPOL0_CPHA0:
        rx_data = soft_spi_rw_mode0(spi, 0xff);
        break;
    case SPI_CPOL0_CPHA1:
        rx_data = soft_spi_rw_mode1(spi, 0xff);
        break;
    case SPI_CPOL1_CPHA0:
        rx_data = soft_spi_rw_mode2(spi, 0xff);
        break;
    case SPI_CPOL1_CPHA1:
        rx_data = soft_spi_rw_mode3(spi, 0xff);
        break;
    default:
        err != NULL ? *err = -EINVAL : 0;
        return 0;
    }
    err != NULL ? *err = 0 : 0;
    return rx_data;
}

/*
 * @brief 18 SPI_MODE_BIDIR_1BIT
 * @parm spi  spi
 * @parm byte
 * @parm err  err0 < 0
 * @return
 */
u8 soft_spi_send_recv_byte(spi_soft_dev spi, u8 byte, int *err)
{
    u8 id = soft_spi_get_info_id(spi);
    u8 rx_data;
    switch (spi_soft_p_data[id].work_mode) {
    case SPI_CPOL0_CPHA0:
        rx_data = soft_spi_rw_mode0(spi, byte);
        break;
    case SPI_CPOL0_CPHA1:
        rx_data = soft_spi_rw_mode1(spi, byte);
        break;
    case SPI_CPOL1_CPHA0:
        rx_data = soft_spi_rw_mode2(spi, byte);
        break;
    case SPI_CPOL1_CPHA1:
        rx_data = soft_spi_rw_mode3(spi, byte);
        break;
    default:
        err != NULL ? *err = -EINVAL : 0;
        return 0;
    }
    err != NULL ? *err = 0 : 0;
    return rx_data;
}

/*
 * @brief spi dma
 * @parm spi  spi
 * @parm buf
 * @parm len
 * @return < 0
 */
int soft_spi_dma_recv(spi_soft_dev spi, void *buf, u32 len)
{
    u8 id = soft_spi_get_info_id(spi);
    u8 *buf_temp = (u8 *)buf;
    /* ASSERT((u32)buf % 4 == 0, "spi dma addr need 4-aligned"); */
    switch (spi_soft_p_data[id].work_mode) {
    case SPI_CPOL0_CPHA0:
        while (len--) {
            *buf_temp = soft_spi_rw_mode0(spi, 0xff);
            buf_temp++;
        }
        break;
    case SPI_CPOL0_CPHA1:
        while (len--) {
            *buf_temp = soft_spi_rw_mode1(spi, 0xff);
            buf_temp++;
        }
        break;
    case SPI_CPOL1_CPHA0:
        while (len--) {
            *buf_temp = soft_spi_rw_mode2(spi, 0xff);
            buf_temp++;
        }
        break;
    case SPI_CPOL1_CPHA1:
        while (len--) {
            *buf_temp = soft_spi_rw_mode3(spi, 0xff);
            buf_temp++;
        }
        break;
    default:
        return -EFAULT;
    }
    return len;
}

int soft_spi_dma_send(spi_soft_dev spi, const void *buf, u32 len)
{
    u8 id = soft_spi_get_info_id(spi);
    u8 *buf_temp = (u8 *)buf;
    /* ASSERT((u32)buf % 4 == 0, "spi dma addr need 4-aligned"); */
    switch (spi_soft_p_data[id].work_mode) {
    case SPI_CPOL0_CPHA0:
        while (len--) {
            soft_spi_rw_mode0(spi, *buf_temp);
            buf_temp++;
        }
        break;
    case SPI_CPOL0_CPHA1:
        while (len--) {
            soft_spi_rw_mode1(spi, *buf_temp);
            buf_temp++;
        }
        break;
    case SPI_CPOL1_CPHA0:
        while (len--) {
            soft_spi_rw_mode2(spi, *buf_temp);
            buf_temp++;
        }
        break;
    case SPI_CPOL1_CPHA1:
        while (len--) {
            soft_spi_rw_mode3(spi, *buf_temp);
            buf_temp++;
        }
        break;
    default:
        return -EFAULT;
    }
    return len;
}


