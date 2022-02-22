#ifndef	_SPI_SOFT_H_
#define _SPI_SOFT_H_
#include "typedef.h"
#include "ioctl.h"
#include "config.h"
#include "clock.h"

// enum spi_soft_mode {
//     SPI_MODE_BIDIR_1BIT,    //支持SPIx，全双工，di接收，do发送
//     SPI_MODE_UNIDIR_1BIT,   //支持SPIx，半双工，do分时发送/接收
//     SPI_MODE_UNIDIR_2BIT,   //支持SPIx，半双工，di & do共2bit分时发送/接收
//     SPI_MODE_UNIDIR_4BIT,   //支持SPIx，半双工，di & do & wp & hoid共4bit分时发送/接收
// };
enum spi_soft_work_mode {
    SPI_CPOL0_CPHA0,   /* CPOL=0, CPHA=0, MSB first, clk_l, smp_edge_rise*/
    SPI_CPOL0_CPHA1,   /* CPOL=0，CPHA=1, MSB first, clk_l, smp_edge_fall*/
    SPI_CPOL1_CPHA0,   /* CPOL=1，CPHA=0, MSB first, clk_h, smp_edge_fall*/
    SPI_CPOL1_CPHA1,   /* CPOL=1, CPHA=1, MSB first, clk_h, smp_edge_rise */
};


typedef const int spi_soft_dev;


int soft_spi_open(spi_soft_dev spi);
void soft_spi_close(spi_soft_dev spi);
void soft_spi_set_bit_mode(spi_soft_dev spi, int data_wide);
int soft_spi_set_baud(spi_soft_dev spi, u32 baud);
u32 soft_spi_get_baud(spi_soft_dev spi);
void soft_spi_suspend(spi_soft_dev spi);
void soft_spi_resume(spi_soft_dev spi);
int soft_spi_send_byte(spi_soft_dev spi, u8 byte);
u8 soft_spi_recv_byte(spi_soft_dev spi, int *err);
u8 soft_spi_send_recv_byte(spi_soft_dev spi, u8 byte, int *err);
int soft_spi_dma_recv(spi_soft_dev spi, void *buf, u32 len);
int soft_spi_dma_send(spi_soft_dev spi, const void *buf, u32 len);

#endif

