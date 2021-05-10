//#undef LOG_TAG_CONST
//#define LOG_TAG     "[SPI]"
#define LOG_TAG_CONST SPI1
#define LOG_TAG     "[spi1]"
//#define LOG_ERROR_ENABLE
//#define LOG_DEBUG_ENABLE
#include "debug.h"
#include "gpio.h"
#include "clock.h"
#include "spi1.h"
#include "includes.h"
#include "typedef.h"
#include "cpu.h"
#include "errno-base.h"
/* #include "jiffies.h" */
/* #include "irq.h" */
/* #include "wdt.h" */
#include "config.h"
#define spi_enable(reg)                     ((reg)->CON |= BIT(0))
#define spi_disable(reg)                    ((reg)->CON &= ~BIT(0))
#define spi_role_slave(reg)                 ((reg)->CON |= BIT(1))
#define spi_role_master(reg)                ((reg)->CON &= ~BIT(1))
#define spi_cs_en(reg)                      ((reg)->CON |= BIT(2))
#define spi_cs_dis(reg)                     ((reg)->CON &= ~BIT(2))
#define spi_bidir(reg)                      ((reg)->CON |= BIT(3))
#define spi_unidir(reg)                     ((reg)->CON &= ~BIT(3))
#define spi_smp_edge_rise(reg)              ((reg)->CON &= ~BIT(4))
#define spi_smp_edge_fall(reg)              ((reg)->CON |= BIT(4))
#define spi_ud_edge_rise(reg)               ((reg)->CON &= ~BIT(5))
#define spi_ud_edge_fall(reg)               ((reg)->CON |= BIT(5))
#define spi_clk_idle_h(reg)                 ((reg)->CON |= BIT(6))
#define spi_clk_idle_l(reg)                 ((reg)->CON &= ~BIT(6))
#define spi_cs_idle_h(reg)                  ((reg)->CON |= BIT(7))
#define spi_cs_idle_l(reg)                  ((reg)->CON &= ~BIT(7))
#define spi_data_width(reg, x)              ((reg)->CON = (reg->CON&~(3<<10))|((x&0x3)<<10))
#define spi_dir_in(reg)                     ((reg)->CON |= BIT(12))
#define spi_dir_out(reg)                    ((reg)->CON &= ~BIT(12))
#define spi_ie_en(reg)                      ((reg)->CON |= BIT(13))
#define spi_ie_dis(reg)                     ((reg)->CON &= ~BIT(13))
#define spi_clr_pnd(reg)                    ((reg)->CON |= BIT(14))
#define spi_pnd(reg)                        ((reg)->CON & BIT(15))
#define spi_w_reg_con(reg, val)             ((reg)->CON = (val))
#define spi_r_reg_con(reg)                  ((reg)->CON)
#define spi_w_reg_buf(reg, val)             ((reg)->BUF = (val))
#define spi_r_reg_buf(reg)                  ((reg)->BUF)
#define spi_w_reg_baud(reg, baud)           ((reg)->BAUD = (baud))
#define spi_r_reg_baud(reg)                 ((reg)->BAUD)
#define spi_w_reg_dma_addr(reg, addr)       ((reg)->ADR  = (addr))
#define spi_w_reg_dma_cnt(reg, cnt)         ((reg)->CNT = (cnt))

#define irq_disable(x)  bit_clr_ie(x)

#define SPI0_ENABLE   0   //SPI1spi0_p_data
#define SPI1_ENABLE   1   //SPI1spi1_p_data

static JL_SPI_TypeDef *const spi_regs[SPI_MAX_HW_NUM] = {
    JL_SPI0,
    JL_SPI1
};
static u8 spi_get_info_id(spi_dev spi)
{
    ASSERT(spi < SPI_MAX_HW_NUM);
    return spi;
}
static u8 *spi_get_info_port(spi_dev spi)
{
    u8 *port = NULL;
    u8 id = spi_get_info_id(spi);
    switch (id) {
#if SPI0_ENABLE
    case SPI0:
        port = (u8 *)spi0_p_data.port;
        break;
#endif
#if SPI1_ENABLE
    case SPI1:
        port = (u8 *)spi1_p_data.port;
        break;
#endif
    default:
        break;
    }
    return port;
}
static u8 spi_get_info_mode(spi_dev spi)
{
    u8 mode = (u8) - 1;
    u8 id = spi_get_info_id(spi);
    switch (id) {
#if SPI0_ENABLE
    case SPI0:
        mode = spi0_p_data.mode;
        break;
#endif
#if SPI1_ENABLE
    case SPI1:
        mode = spi1_p_data.mode;
        break;
#endif
    default:
        break;
    }
    return mode;
}
static u8 spi_get_info_role(spi_dev spi)
{
    u8 role = (u8) - 1;
    u8 id = spi_get_info_id(spi);
    switch (id) {
#if SPI0_ENABLE
    case SPI0:
        role = spi0_p_data.role;
        break;
#endif
#if SPI1_ENABLE
    case SPI1:
        role = spi1_p_data.role;
        break;
#endif
    default:
        break;
    }
    return role;
}
static u32 spi_get_info_clock(spi_dev spi)
{
    u32 clock = (u8) - 1;
    u8 id = spi_get_info_id(spi);
    switch (id) {
#if SPI0_ENABLE
    case SPI0:
        clock = spi0_p_data.clk;
        break;
#endif
#if SPI1_ENABLE
    case SPI1:
        clock = spi1_p_data.clk;
        break;
#endif
    default:
        break;
    }
    return clock;
}
static void spi0_iomc_config(spi_dev spi)
{
}
static void spi1_iomc_config(spi_dev spi)
{
    u8 i = 0;
    u8 id = spi_get_info_id(spi);
    u8 *port = spi_get_info_port(spi);
    if (port[0] == IO_PORTB_00) {
        i = 0;
    } else if (port[0] == IO_PORTA_14) {
        i = 1;
    } else if (port[0] == IO_PORTA_06) {
        i = 2;
    } else if (port[0] == IO_PORTB_08) {
        i = 3;
    }
    JL_IOMC->IOMC0 |= BIT(12);
    SFR(JL_IOMC->IOMC0, 10, 2, i);
}
static void (*pSPI_IOMC_CONFIG[])(spi_dev spi) = {
    spi0_iomc_config,
    spi1_iomc_config,
};
static void spi_io_port_init(u8 port, u8 dir)
{
    if (port != (u8) - 1) {
        if (dir == 1) {
            gpio_set_direction(port, 1);
        } else {
            gpio_set_direction(port, 0);
        }
        gpio_set_die(port, 1);
        gpio_set_pull_up(port, 0);
        gpio_set_pull_down(port, 0);
    }
}
static void spi_io_port_uninit(u8 port)
{
    if (port != (u8) - 1) {
        gpio_set_direction(port, 1);
        gpio_set_die(port, 0);
        gpio_set_pull_up(port, 0);
        gpio_set_pull_down(port, 0);
    }
}
/*
 * @brief
 * @parm spi  spi
 * @parm baud
 * @return 0 < 0
 */
int spi_set_baud(spi_dev spi, u32 baud)
{
    u32 get_spiclk;
    //SPICK = get_spiclk / (SPIx_BAUD + 1)
    //=> SPIx_BAUD = get_spiclk / SPICK - 1
    u8 id = spi_get_info_id(spi);
    get_spiclk = clk_get("lsb");
    log_info("spi clock source freq %lu", get_spiclk);
    if (get_spiclk < baud) {
        spi_w_reg_baud(spi_regs[id], 0);
        return -EINVAL;
    }
    spi_w_reg_baud(spi_regs[id], get_spiclk / baud - 1);
    return 0;
}
/*
 * @brief
 * @parm spi  spi
 * @return
 */
u32 spi_get_baud(spi_dev spi)
{
    u8 id = spi_get_info_id(spi);
    return spi_get_info_clock(spi);
}
/*
 * @brief spi[/]
 * @parm spi  spi
 * @parm mode
 * @return null
 */
void spi_set_bit_mode(spi_dev spi, int mode)
{
    u8 id = spi_get_info_id(spi);
    u8 *port = spi_get_info_port(spi);
    u8 role = spi_get_info_role(spi);
    switch (mode) {
    case SPI_MODE_BIDIR_1BIT:
        spi_bidir(spi_regs[id]);
        spi_data_width(spi_regs[id], 0);
        break;
    case SPI_MODE_UNIDIR_1BIT:
        spi_unidir(spi_regs[id]);
        spi_data_width(spi_regs[id], 0);
        break;
    case SPI_MODE_UNIDIR_2BIT:
        spi_unidir(spi_regs[id]);
        spi_data_width(spi_regs[id], 1);
        break;
    }
    spi_io_port_init(port[0], role == SPI_ROLE_MASTER ? 0 : 1);
    spi_io_port_init(port[1], 0);
    if (mode == SPI_MODE_BIDIR_1BIT) {
        spi_io_port_init(port[2], 1);
    } else if (mode == SPI_MODE_UNIDIR_2BIT) {
        spi_io_port_init(port[2], 0);
    }
}
/*
 * @brief spi
 * @parm spi  spi
 * @return 0 < 0
 */
int spi_open(spi_dev spi)
{
    int err;
    u8 id = spi_get_info_id(spi);
    u8 mode = spi_get_info_mode(spi);
    u8 role = spi_get_info_role(spi);
    u32 clock = spi_get_info_clock(spi);
    pSPI_IOMC_CONFIG[id](spi);
    spi_set_bit_mode(spi, mode);
    spi_cs_dis(spi_regs[id]);
    if (role == SPI_ROLE_MASTER) {
        spi_role_master(spi_regs[id]);
        spi_dir_out(spi_regs[id]);
    } else if (role == SPI_ROLE_SLAVE) {
        spi_role_slave(spi_regs[id]);
        spi_dir_in(spi_regs[id]);
    }
    spi_smp_edge_rise(spi_regs[id]);
    spi_ud_edge_fall(spi_regs[id]);
    spi_cs_idle_h(spi_regs[id]);
    spi_clk_idle_h(spi_regs[id]);
    spi_clr_pnd(spi_regs[id]);
    err = spi_set_baud(spi, clock);
    if (err) {
        log_error("invalid spi baudrate");
        /* return 0; */
    }
    spi_w_reg_buf(spi_regs[id], 0);//spiDO
    spi_enable(spi_regs[id]);
    //无中断bit13，无传送方向bit12

#if 0
    log_info("spi%d clk     = %d\n", id, clock);
    log_info("spi%d mode    = %d\n", id, mode);
    log_info("spi%d role    = %d\n", id, role);
    log_info("spi%d clk_pin = %d\n", id, port[0]);
    log_info("spi%d do_pin  = %d\n", id, port[1]);
    log_info("spi%d di_pin  = %d\n", id, port[2]);
    log_info("spi%d CON     = %04x\n", id, spi_r_reg_con(spi_regs[id]));
    log_info("spi%d IOMC1   = %08x\n", id, JL_IOMAP->CON1);
#endif
    return 0;
}

static int __spi_wait_ok(spi_dev spi, u32 len)
{
    u8 id = spi_get_info_id(spi);
    u32 baud = spi_get_info_clock(spi);
    baud = clk_get("lsb") / baud - 1;
    u32 retry = baud * (clk_get("sys") / clk_get("lsb")) * 8 * len * 5; //500% spi baudate
    while (!spi_pnd(spi_regs[id])) {
        __asm__ volatile("nop");
        if (--retry == 0) {
            log_error("spi wait pnd timeout");
            return -EFAULT;
        }
    }
    spi_clr_pnd(spi_regs[id]);
    return 0;
}
/*
 * @brief 1
 * @parm spi  spi
 * @parm byte
 * @return 0 < 0
 */
int spi_send_byte(spi_dev spi, u8 byte)
{
    u8 id = spi_get_info_id(spi);
    spi_dir_out(spi_regs[id]);
    spi_w_reg_buf(spi_regs[id], byte);
    return __spi_wait_ok(spi, 1);
}
/*
 * @brief 1pnd
 * @parm spi  spi
 * @parm byte
 * @return null
 */
void spi_send_byte_for_isr(spi_dev spi, u8 byte)
{
    u8 id = spi_get_info_id(spi);
    spi_dir_out(spi_regs[id]);
    spi_w_reg_buf(spi_regs[id], byte);
}
/*
 * @brief 1
 * @parm spi  spi
 * @parm err  err0 < 0
 * @return
 */
u8 spi_recv_byte(spi_dev spi, int *err)
{
    u8 id = spi_get_info_id(spi);
    int ret;
    spi_dir_in(spi_regs[id]);
    spi_w_reg_buf(spi_regs[id], 0xff);
    ret = __spi_wait_ok(spi, 1);
    if (ret) {
        err != NULL ? *err = ret : 0;
        return 0;
    }
    err != NULL ? *err = 0 : 0;
    return spi_r_reg_buf(spi_regs[id]);
}
/*
 * @brief 1pnd
 * @parm spi  spi
 * @return
 */
u8 spi_recv_byte_for_isr(spi_dev spi)
{
    u8 id = spi_get_info_id(spi);
    spi_dir_in(spi_regs[id]);
    return spi_r_reg_buf(spi_regs[id]);
}
/*
 * @brief 18 SPI_MODE_BIDIR_1BIT
 * @parm spi  spi
 * @parm byte
 * @parm err  err0 < 0
 * @return
 */
u8 spi_send_recv_byte(spi_dev spi, u8 byte, int *err)
{
    u8 id = spi_get_info_id(spi);
    int ret;
    spi_w_reg_buf(spi_regs[id], byte);
    ret = __spi_wait_ok(spi, 1);
    if (ret) {
        err != NULL ? *err = ret : 0;
        return 0;
    }
    err != NULL ? *err = 0 : 0;
    return spi_r_reg_buf(spi_regs[id]);
}

/*
 * @brief spi dma
 * @parm spi  spi
 * @parm buf
 * @parm len
 * @return < 0
 */
int spi_dma_recv(spi_dev spi, void *buf, u32 len)
{
    u8 id = spi_get_info_id(spi);
    /* ASSERT((u32)buf % 4 == 0, "spi dma addr need 4-aligned"); */
    spi_dir_in(spi_regs[id]);
    spi_w_reg_dma_addr(spi_regs[id], (u32)buf);
    spi_w_reg_dma_cnt(spi_regs[id], len);
    asm("csync");
    if (__spi_wait_ok(spi, len)) {
        return -EFAULT;
    }
    return len;
}
/*
 * @brief spi dma
 * @parm spi  spi
 * @parm buf
 * @parm len
 * @return < 0
 */
int spi_dma_send(spi_dev spi, const void *buf, u32 len)
{
    u8 id = spi_get_info_id(spi);
    /* ASSERT((u32)buf % 4 == 0, "spi dma addr need 4-aligned"); */
    spi_dir_out(spi_regs[id]);
    spi_w_reg_dma_addr(spi_regs[id], (u32)buf);
    spi_w_reg_dma_cnt(spi_regs[id], len);
    asm("csync");
    if (__spi_wait_ok(spi, len)) {
        return -EFAULT;
    }
    return len;
}
/*
 * @brief spi dmapnd
 * @parm spi  spi
 * @parm buf
 * @parm len
 * @parm rw  1  / 0
 * @return null
 */
void spi_dma_set_addr_for_isr(spi_dev spi, void *buf, u32 len, u8 rw)
{
    u8 id = spi_get_info_id(spi);
    /* ASSERT((u32)buf % 4 == 0, "spi dma addr need 4-aligned"); */
    rw ? spi_dir_in(spi_regs[id]) : spi_dir_out(spi_regs[id]);
    spi_w_reg_dma_addr(spi_regs[id], (u32)buf);
    spi_w_reg_dma_cnt(spi_regs[id], len);
}
///*中断函数*/
//__attribute__((interrupt("")))
//static void spi1_isr()
//{
//    if(spi_pnd(spi_regs[SPI1]))//发送，接收，DMA收发
//    {
//        spi_clr_pnd(spi_regs[SPI1]);
//    }
//}
/*
 * @brief
 * @parm spi  spi
 * @parm en  1 0
 * @return null
 */
void spi_set_ie(spi_dev spi, u8 en)//还需注册中断服务函数*************************************
{
    u8 id = spi_get_info_id(spi);
    /* if(en) HWI_Install(IRQ_SPI1_IDX,(u32)spi1_isr,3); */
    /* else irq_disable(IRQ_SPI1_IDX); */
    en ? spi_ie_en(spi_regs[id]) : spi_ie_dis(spi_regs[id]);
}
/*
 * @brief
 * @parm spi  spi
 * @return 0 / 1
 */
u8 spi_get_pending(spi_dev spi)
{
    u8 id = spi_get_info_id(spi);
    return spi_pnd(spi_regs[id]) ? 1 : 0;
}
/*
 * @brief
 * @parm spi  spi
 * @return null
 */
void spi_clear_pending(spi_dev spi)
{
    u8 id = spi_get_info_id(spi);
    spi_clr_pnd(spi_regs[id]);
}
/*
 * @brief spi
 * @parm spi  spi
 * @return null
 */
void spi_close(spi_dev spi)
{
    u8 id = spi_get_info_id(spi);
    u8 *port = spi_get_info_port(spi);
    spi_io_port_uninit(port[0]);
    spi_io_port_uninit(port[1]);
    spi_io_port_uninit(port[2]);
    spi_disable(spi_regs[id]);
}

