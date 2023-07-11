#include "iic_hw.h"
#include "clock.h"

/* #define LOG_TAG_CONST       IIC */
#define LOG_TAG             "[iic]"
#include "log.h"


#ifndef EINVAL
#define EINVAL      22  /* Invalid argument */
#endif

const struct hw_iic_config hw_iic_cfg[] = {
    //iic0 data
    {
        .port = {IO_PORTA_01, IO_PORTA_02}, //
        .baudrate = 100000,      //IIC通讯波特率
        .hdrive = 0,             //是否打开IO口强驱
        .io_filter = 3,          //是否打开滤波器（去纹波）//0:close; 1:<1*Tiic_baud_clk, 2:<2*Tiic_baud_clk, 3:<3*Tiic_baud_clk
        .io_pu = 1,              //0:no, 1:10k.是否打开上拉电阻，如果外部电路没有焊接上拉电阻需要置1
        .role = IIC_MASTER,
        /* .role = IIC_SLAVE, */
        .slave_addr = 0x54,//bit7~bit1
        .ie_en = 0,
    },
};
static JL_IIC_TypeDef *const iic_regs[IIC_HW_NUM] = {
    JL_IIC0,
};

#define iic_get_id(iic)         (iic)

#define iic_info_port(iic, x)      (hw_iic_cfg[iic_get_id(iic)].port[x])
#define iic_info_baud(iic)         (hw_iic_cfg[iic_get_id(iic)].baudrate)
#define iic_info_hdrive(iic)       (hw_iic_cfg[iic_get_id(iic)].hdrive)
#define iic_info_io_filt(iic)      (hw_iic_cfg[iic_get_id(iic)].io_filter)
#define iic_info_io_pu(iic)        (hw_iic_cfg[iic_get_id(iic)].io_pu)
#define iic_info_role(iic)         (hw_iic_cfg[iic_get_id(iic)].role)
#define iic_info_slave_addr(iic)   (hw_iic_cfg[iic_get_id(iic)].slave_addr)
#define iic_info_ie_en(iic)        (hw_iic_cfg[iic_get_id(iic)].ie_en)

static inline u32 iic_get_scl(hw_iic_dev iic)
{
    u8 port = iic_info_port(iic, 0);
    return port;
}

static inline u32 iic_get_sda(hw_iic_dev iic)
{
    u8 port = iic_info_port(iic, 1);
    return port;
}

static int iic_port_init(hw_iic_dev iic)
{
    u32 reg;
    int ret = 0;
    u8 id = iic_get_id(iic);
    u32 scl, sda;
    scl = iic_get_scl(iic);
    sda = iic_get_sda(iic);
    if (id == 0) {
        gpio_set_fun_output_port(scl, FO_I2C0_SCL, 1, 1);
        gpio_set_fun_output_port(sda, FO_I2C0_SDA, 1, 1);
        gpio_set_fun_input_port(scl, PFI_IIC0_SCL);
        gpio_set_fun_input_port(sda, PFI_IIC0_SDA);

        if (iic_info_hdrive(iic) < 4) {
            gpio_set_hd(scl, iic_info_hdrive(iic));
            gpio_set_hd(sda, iic_info_hdrive(iic));
        } else {
            gpio_set_hd(scl, 0);
            gpio_set_hd(sda, 0);
        }
        if (iic_info_io_pu(iic) < 2) {
            gpio_set_pull_up(sda, iic_info_io_pu(iic));
            gpio_set_pull_up(scl, iic_info_io_pu(iic));
            gpio_set_pull_down(scl, 0);
            gpio_set_pull_down(sda, 0);
        } else {//error
            gpio_set_pull_up(scl, 0);
            gpio_set_pull_down(scl, 0);
            gpio_set_pull_up(sda, 0);
            gpio_set_pull_down(sda, 0);
        }
        gpio_set_spl(scl, 1);
        gpio_set_spl(sda, 1);
    } else {
        ret = -EINVAL;
    }
    return ret;
}

int hw_iic_set_baud(hw_iic_dev iic, u32 baud)
{
    //fiic_bus_scl =  fiic_clk /(2 * BUAD_CNT) + T电阻上拉
    //=> BUAD_CNT = fiic_clk / (2 * fiic_bus_scl)
    u32 sysclk;
    u16 baud_cnt = 0;
    u8 id = iic_get_id(iic);
    //set iic clk =uart clk
    //get iic clk
    sysclk = 48000000;//clk_get("uart");
    if (sysclk < 2 * baud) {
        return -EINVAL;
    }
    baud_cnt = sysclk / (2 * baud);
    if (baud_cnt > 4095) {
        baud_cnt = 4095;
    } else if (baud_cnt < 4) {
        baud_cnt = 4;
    }
    iic_baud_reg(iic_regs[id]) = baud_cnt;
    if (baud_cnt > 3) {
        iic_tsu_reg(iic_regs[id]) = 2; //<=127 sda信号保持时间
        iic_thd_reg(iic_regs[id]) = 2; //<=127 sda信号建立时间
    }
    /* log_info("sysclk:%d,cal-baud:0x%x\n", sysclk, baud_cnt); */
    /* log_info("iic_tsu:%d,iic_thd:%d\n", iic_tsu_reg(iic_regs[id]), iic_thd_reg(iic_regs[id])); */
    return 0;
}

static void hw_iic_set_die(hw_iic_dev iic, u8 en)
{
    u8 id = iic_get_id(iic);
    u32 scl, sda;
    scl = iic_get_scl(iic);
    sda = iic_get_sda(iic);
    if (id == 0) {
        gpio_set_die(scl, en);
        gpio_set_die(sda, en);
    } else {
        //undefined
    }
}

void hw_iic_slave_isr();
int hw_iic_init(hw_iic_dev iic)
{
    int ret;
    u8 id = iic_get_id(iic);

    if ((ret = iic_port_init(iic))) {
        log_error("invalid hardware iic port\n");
        return ret;
    }
    hw_iic_set_die(iic, 1);//
    iic_regs[id]->CON = 1;
    iic_png_reg(iic_regs[id]) = 0x1ff << 10; //clr png
    if (iic_info_role(iic) == IIC_MASTER) {
        iic_role_host(iic_regs[id]);
        if ((ret = hw_iic_set_baud(iic, iic_info_baud(iic)))) {
            log_error("iic baudrate is invalid\n");
            return ret ;
        }
    } else {
        iic_role_slave(iic_regs[id]);
        hw_iic_slave_set_addr(iic, iic_info_slave_addr(iic), 1);
        /* iic_baddr_resp_en(iic_regs[id]);//广播地址 */
        iic_baddr_resp_dis(iic_regs[id]);//广播地址
        iic_slv_rx_manu(iic_regs[id]);
        iic_slv_tx_manu(iic_regs[id]);
        iic_tsu_reg(iic_regs[id]) = 3; //<=127
        iic_thd_reg(iic_regs[id]) = 3; //<=127
    }
    iic_flt_sel(iic_regs[id], iic_info_io_filt(iic));
    iic_stretch_en(iic_regs[id]);//时钟延展
    /* iic_stretch_dis(iic_regs[id]); */
#if MASTER_IIC_WRITE_MODE_FAST_RESP
    iic_ignore_nack_dis(iic_regs[id]);//iic
#else
    iic_ignore_nack_dis(iic_regs[id]);//iic
    /* iic_ignore_nack_en(iic_regs[id]);//特殊用途 */
#endif

    if (iic_info_ie_en(iic)) {
        HWI_Install(IRQ_IIC_IDX, (u32)hw_iic_slave_isr, 3);//3: 中断优先级
#if SLAVE_NO_STRETCH_AUTO_TASK
        iic_stretch_dis(iic_regs[0]);
        iic_slv_rx_auto(iic_regs[0]);//rx
        iic_slv_tx_auto(iic_regs[0]);//tx
        hw_iic_set_ie(iic, I2C_PND_TXTASK_LOAD, 1);
        hw_iic_set_ie(iic, I2C_PND_RXDATA_DONE, 1);
        hw_iic_set_ie(iic, I2C_PND_STOP, 1);
#else
        hw_iic_set_ie(iic, I2C_PND_ADR_MATCH, 1);
        hw_iic_set_ie(iic, I2C_PND_TASK_DONE, 1);
        hw_iic_set_ie(iic, I2C_PND_STOP, 1);
#endif
    } else {
        SFR(iic_png_reg(iic_regs[id]), 0, 9, 0); //ie dis
    }

    SFR(iic_png_reg(iic_regs[id]), 10, 9, 0x1ff); //clr png
    iic_rst_release(iic_regs[id]);//run

#if SLAVE_NO_STRETCH_AUTO_TASK
    if (iic_info_role(iic) == IIC_SLAVE) {
        iic_tx_buf_reg(iic_regs[0]) = 0;//first tx data
    }
#endif

#if 0
    log_info("info->scl = %d", iic_get_scl(iic));
    log_info("info->sda = %d", iic_get_sda(iic));
    log_info("info->baudrate = %d", iic_info_baud(iic));
    log_info("info->hdrive = %d", iic_info_hdrive(iic));
    log_info("info->io_filter = %d", iic_info_io_filt(iic));
    log_info("info->io_pu = %d", iic_info_io_pu(iic));
    log_info("info->role = %d", iic_info_role(iic));
    log_info("IIC_CON 0x%x", iic_regs[id]->CON);
    log_info("IIC_TASK 0x%x", iic_regs[id]->TASK);
    log_info("IIC_PND 0x%x", iic_regs[id]->PND);
    log_info("IIC_ADDR 0x%x", iic_regs[id]->ADDR);
    log_info("IIC_BAUD 0x%x", iic_regs[id]->BAUD);
    log_info("IIC_TSU 0x%x", iic_regs[id]->TSU);
    log_info("IIC_THD 0x%x", iic_regs[id]->THD);
    log_info("IIC_DBG 0x%x", iic_regs[id]->DBG);
#endif
    return 0;
}

void hw_iic_uninit(hw_iic_dev iic)
{
    u8 id = iic_get_id(iic);
    u32 scl, sda;

    scl = iic_get_scl(iic);
    sda = iic_get_sda(iic);
    hw_iic_set_die(iic, 0);
    if (id == 0) {
        gpio_set_hd(scl, 0);
        gpio_set_hd(sda, 0);
        gpio_set_pull_up(scl, 0);
        gpio_set_pull_up(sda, 0);
    }
    iic_disable(iic_regs[id]);
    // iic_rst_rst(iic_regs[id]);
}


void hw_iic_suspend(hw_iic_dev iic)
{
    hw_iic_uninit(iic);
}

//return:0:ok, <0:error
int hw_iic_resume(hw_iic_dev iic)
{
    u8 id = iic_get_id(iic);
    if ((iic_port_init(iic))) {
        log_error("invalid hardware iic port\n");
        return -EINVAL;
    }
    hw_iic_set_die(iic, 1);
    SFR(iic_png_reg(iic_regs[id]), 10, 9, 0x1ff); //clr png
    iic_enable(iic_regs[id]);
    iic_rst_release(iic_regs[id]);//run
#if SLAVE_NO_STRETCH_AUTO_TASK
    if (iic_info_role(iic) == IIC_SLAVE) {
        iic_tx_buf_reg(iic_regs[0]) = 0;//first tx data
    }
#endif
    return 0;
}

void hw_i2c_put_task(hw_iic_dev iic, I2C_TASK_Typedef task)
{
    u8 id = iic_get_id(iic);
    iic_task_sel(iic_regs[id], task);
}

void hw_iic_set_ie(hw_iic_dev iic, I2C_PND_Typedef png, u8 en)
{
    u8 id = iic_get_id(iic);

    if (en) {
        iic_png_reg(iic_regs[id]) |= (png >> 20);
    } else {
        iic_png_reg(iic_regs[id]) &= ~(png >> 20);
    }
}
u8 hw_iic_get_pnd(hw_iic_dev iic, I2C_PND_Typedef png)
{
    u8 id = iic_get_id(iic);
    if (iic_png_reg(iic_regs[id])&png) {
        return 1;
    }
    return 0;
}
void hw_iic_clr_pnd(hw_iic_dev iic, I2C_PND_Typedef png)
{
    u8 id = iic_get_id(iic);
    iic_png_reg(iic_regs[id]) |= (png >> 10);
}

static u8 start_signal = 0;
void hw_iic_start(hw_iic_dev iic)
{
    start_signal >>= 1;
    start_signal |= BIT(7);
}


#if MASTER_IIC_WRITE_MODE_FAST_RESP
static u8 addr_cnt = 0;
#endif
//1:ack
u8 hw_iic_start_addr(hw_iic_dev iic, u8 addr)
{
    u8 id = iic_get_id(iic);
    iic_addr_reg(iic_regs[id]) = addr;
    hw_i2c_put_task(iic, I2C_TASK_SEND_ADDR);
#if MASTER_IIC_WRITE_MODE_FAST_RESP
    if (addr_cnt == 1) {
        /* while(!hw_iic_get_pnd(iic,I2C_PND_START));//主机不起start_png */
        while (!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)); //主机不起start_png
        if (hw_iic_get_pnd(iic, I2C_PND_RXNACK)) {
            log_error("iic tx reg ack fail!");
            while (!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE));
            SFR(iic_png_reg(iic_regs[id]), 10, 9, 0x1ff); //clr all png
            addr_cnt = 0;
            start_signal = 0;
            return 0;
        }
        hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
        hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);
        hw_iic_clr_pnd(iic, I2C_PND_RXACK);
    }
    addr_cnt++;
#endif
    while (!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)); //wait addr done
    hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
    hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);
    hw_iic_clr_pnd(iic, I2C_PND_START);
    if (hw_iic_get_pnd(iic, I2C_PND_RXACK)) {
        hw_iic_clr_pnd(iic, I2C_PND_RXACK);
        hw_iic_clr_pnd(iic, I2C_PND_RXNACK);
        return 1;
    }
    log_error("iic tx addr ack fail!");
    hw_iic_clr_pnd(iic, I2C_PND_RXNACK);
    start_signal = 0;
#if MASTER_IIC_WRITE_MODE_FAST_RESP
    addr_cnt = 0;
#endif
    return 0;
}

void hw_iic_stop(hw_iic_dev iic)
{
    u8 id = iic_get_id(iic);
    hw_i2c_put_task(iic, I2C_TASK_SEND_STOP);
    /* hw_i2c_put_task(iic, I2C_TASK_SEND_NACK_STOP); */
#if MASTER_IIC_WRITE_MODE_FAST_RESP
    if (addr_cnt == 1) {
        /* while(!hw_iic_get_pnd(iic,I2C_PND_STOP));//主机不起stop_png */
        while (!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE));
        hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
    }
#if MASTER_IIC_READ_MODE_FAST_RESP
    else if (addr_cnt == 2) {
        while (!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE));
        hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
        //read iic_rx_buf_reg(iic_regs[id]);
    }
#endif
    addr_cnt = 0;
#endif
    while (!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE));
    start_signal = 0;
    /* log_info("stop IIC_PND 0x%x", iic_regs[id]->PND); */
    SFR(iic_png_reg(iic_regs[id]), 10, 9, 0x1ff); //clr all png
}

//与其他从机验证ignore_nack模式
//1:ack
u8 hw_iic_tx_byte(hw_iic_dev iic, u8 byte)
{
    if (start_signal & BIT(7)) {
        start_signal >>= 1;
        /* log_info("st/"); */
        return hw_iic_start_addr(iic, byte);//addr + w/r
    }

    u8 id = iic_get_id(iic);
    iic_tx_buf_reg(iic_regs[id]) = byte;
    hw_i2c_put_task(iic, I2C_TASK_SEND_DATA);
    /* putchar('a'); */
#if MASTER_IIC_WRITE_MODE_FAST_RESP
    while (!hw_iic_get_pnd(iic, I2C_PND_TXTASK_LOAD));
    hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);
    hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
    if ((hw_iic_get_pnd(iic, I2C_PND_RXACK) == 0) && (hw_iic_get_pnd(iic, I2C_PND_RXNACK) == 0)) {
        return 1;//MASTER_IIC_WRITE_MODE_FAST_RESP标记有应答
    }

#else
    while (!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE));
    hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
    hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);
#endif
    /* putchar('b'); */
    if (hw_iic_get_pnd(iic, I2C_PND_RXACK)) {
        hw_iic_clr_pnd(iic, I2C_PND_RXACK);
        hw_iic_clr_pnd(iic, I2C_PND_RXNACK);
        return 1;
    }
    log_error("iic tx data ack fail!");
    hw_iic_clr_pnd(iic, I2C_PND_RXNACK);
    return 0;
}

void hw_iic_set_ack(hw_iic_dev iic, u8 ack_en)
{
    if (ack_en) {
        hw_i2c_put_task(iic, I2C_TASK_SEND_ACK);
    } else {
        hw_i2c_put_task(iic, I2C_TASK_SEND_NACK);
    }
    while (!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE));
    hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
}
//ack:0:nack, 1:ack, 2:only rx data
u8 hw_iic_rx_byte(hw_iic_dev iic, u8 ack)
{
    u8 id = iic_get_id(iic);

    if (ack == 1) {
        hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_ACK);
    } else if (ack == 0) {
        hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_NACK);
    } else if (ack == 2) { //特殊
        hw_i2c_put_task(iic, I2C_TASK_RECV_DATA);
    }

#if MASTER_IIC_READ_MODE_FAST_RESP
    while (!hw_iic_get_pnd(iic, I2C_PND_RXTASK_LOAD));
    hw_iic_clr_pnd(iic, I2C_PND_RXTASK_LOAD);
    hw_iic_clr_pnd(iic, I2C_PND_RXDATA_DONE);
    hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
#else
    while (!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE));
    /* xput_u32hex(iic_regs[id]->PND); */
    hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
    hw_iic_clr_pnd(iic, I2C_PND_RXDATA_DONE);
#endif
    return iic_rx_buf_reg(iic_regs[id]);
}

int hw_iic_read_buf(hw_iic_dev iic, void *buf, int len)
{
    u8 id = iic_get_id(iic);
    int i;

    if (!buf || !len) {
        return -1;
    }
    hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_ACK);
    for (i = 0; i < len; i++) {
        if (i == len - 1) {
            hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_NACK);
        } else {
            hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_ACK);
        }
        while (!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE));
        hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
        hw_iic_clr_pnd(iic, I2C_PND_RXDATA_DONE);
        ((u8 *)buf)[i] = iic_rx_buf_reg(iic_regs[id]);
    }
    return len;
}

int hw_iic_write_buf(hw_iic_dev iic, const void *buf, int len)
{
    u8 id = iic_get_id(iic);
    int i = 0;

    if (!buf || !len) {
        return -1;
    }

    for (i = 0; i < len; i++) {
        iic_tx_buf_reg(iic_regs[id]) = ((u8 *)buf)[i];
        hw_i2c_put_task(iic, I2C_TASK_SEND_DATA);
        while (!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE));
        hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
        hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);

        if (hw_iic_get_pnd(iic, I2C_PND_RXNACK)) {
            hw_iic_clr_pnd(iic, I2C_PND_RXACK);
            hw_iic_clr_pnd(iic, I2C_PND_RXNACK);
            break;
        }
    }
    return i;
}



void hw_iic_slave_set_addr(hw_iic_dev iic, u8 addr, u8 addr_ack)
{
    u8 id = iic_get_id(iic);

    if (addr_ack) {
        iic_addr_resp_auto(iic_regs[id]);
    } else {
        iic_addr_resp_manu(iic_regs[id]);
    }
    iic_addr_reg(iic_regs[id]) = addr; //高7为有效
}





//ack:0:nack, 1:ack, 2:only rx data
void hw_iic_slave_rx_prepare(hw_iic_dev iic, u8 ack)
{
    u8 id = iic_get_id(iic);

    if (ack == 1) {
        hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_ACK);
    } else if (ack == 0) {
        hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_NACK);
    } else if (ack == 2) {
        hw_i2c_put_task(iic, I2C_TASK_RECV_DATA);
    }
}

u8 hw_iic_slave_rx_byte(hw_iic_dev iic, bool *is_start_addr)
{
    u8 id = iic_get_id(iic);

    if (hw_iic_get_pnd(iic, I2C_PND_START)) {
        hw_iic_clr_pnd(iic, I2C_PND_START);
        is_start_addr ? (*is_start_addr = 1) : 0;
    } else {
        is_start_addr ? (*is_start_addr = 0) : 0;
    }
    return iic_rx_buf_reg(iic_regs[id]);
}

void hw_iic_slave_tx_byte(hw_iic_dev iic, u8 byte)
{
    u8 id = iic_get_id(iic);
    iic_tx_buf_reg(iic_regs[id]) = byte;
    hw_i2c_put_task(iic, I2C_TASK_SEND_DATA);
}

//1:ack
u8 hw_iic_slave_tx_check_ack(hw_iic_dev iic)
{
    u8 id = iic_get_id(iic);

    if (hw_iic_get_pnd(iic, I2C_PND_RXACK)) {
        hw_iic_clr_pnd(iic, I2C_PND_RXACK);
        hw_iic_clr_pnd(iic, I2C_PND_RXNACK);
        return 1;
    }
    log_error("iic tx data ack fail!");
    hw_iic_clr_pnd(iic, I2C_PND_RXNACK);
    return 0;
}


u32 _err_cnt = 0;
u32 _run_cnt = 0;
u8 slave_tx_data[256];
u8 slave_rx_data[256] = {0};


#if SLAVE_NO_STRETCH_AUTO_TASK
//中断法: hw_iic_cfg结构体配置从机模式,地址,使能iic中断即可
//从机中断接收不可被其它打断(如高优先级中断)，否则可能丢失结束信号导致出错
AT_RAM
___interrupt
void hw_iic_slave_isr()//slave  no_stretch
{
    u8 iic = 0, id = 0;
    static u8 rx_cnt = 0;
    static u8 tx_cnt = 0;
    static u8 mode_cnt = 0;
    if (hw_iic_get_pnd(iic, I2C_PND_TXTASK_LOAD)) {
        //发送准备(1个byte时间)

        iic_tx_buf_reg(iic_regs[id]) = slave_tx_data[++tx_cnt];
        /* hw_i2c_put_task(iic, I2C_TASK_SEND_DATA); */
        mode_cnt = 2;
        SFR(iic_png_reg(iic_regs[id]), 10, 9, 0x1ff); //clr all png
        return;
    }
    if (hw_iic_get_pnd(iic, I2C_PND_RXDATA_DONE)) {
        //接收准备(1个byte时间)

        slave_rx_data[rx_cnt] = iic_rx_buf_reg(iic_regs[0]);//rx reg
        SFR(iic_png_reg(iic_regs[id]), 10, 9, 0x1ff); //clr all png
        hw_i2c_put_task(iic, I2C_TASK_SEND_ACK);// rx data1
        mode_cnt = 1;
        rx_cnt++;
    }

    if (hw_iic_get_pnd(iic, I2C_PND_STOP)) {

        /* while(!hw_iic_get_pnd(iic,I2C_PND_TASK_DONE));//*/
        SFR(iic_png_reg(iic_regs[id]), 10, 9, 0x1ff); //clr all png
        /* log_info("no_stretch end PND 0x%x", iic_regs[id]->PND); */
#if 0//check rx data
        _run_cnt++;
        if (mode_cnt == 1) {
            log_info_hexdump(slave_rx_data, rx_cnt);
            u8 flag = 0;
            for (u8 i = 0; i < rx_cnt; i++) {
                if (i != slave_rx_data[i]) {
                    flag = 1;
                    log_error("num:0x%x,data:0x%x", i, slave_rx_data[i]);
                    break;
                }
            }
            if (flag == 0) {
                puts("slave no_stretch read test pass\n");
            } else {
                puts("slave no_stretch read test fail\n");
                _err_cnt++;
            }
            memset(slave_rx_data, 0, rx_cnt);
        }
        log_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~_err_cnt:%d, _run_cnt:%d", _err_cnt, _run_cnt);
#endif
        mode_cnt = 0;
        rx_cnt = 0;
        tx_cnt = 0;
        iic_tx_buf_reg(iic_regs[id]) = slave_tx_data[0];
        return;
    }
}
#else
AT_RAM
___interrupt
void hw_iic_slave_isr()//slave  stretch
{
    u8 iic = 0, id = 0;
    static u8 rx_cnt = 0;
    static u8 tx_cnt = 0;
    static u8 mode_cnt = 0;
    if (hw_iic_get_pnd(iic, I2C_PND_ADR_MATCH)) {
        while (!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)); //0x1230
        SFR(iic_png_reg(iic_regs[id]), 10, 9, 0x1ff); //clr all png
        if (iic_rx_buf_reg(iic_regs[id]) == (hw_iic_cfg[0].slave_addr + 1)) {
            //tx data,,,,,,,,,,,,,,,,,,
            iic_tx_buf_reg(iic_regs[id]) = slave_tx_data[tx_cnt];
            hw_i2c_put_task(iic, I2C_TASK_SEND_DATA);
            mode_cnt = 2; //tx
        } else {
            hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_ACK);//rx reg
            mode_cnt = 1; //rx
        }
        return;
    }
    if (hw_iic_get_pnd(iic, I2C_PND_STOP)) {
        /* while(!hw_iic_get_pnd(iic,I2C_PND_TASK_DONE));//0x1230 */
        SFR(iic_png_reg(iic_regs[id]), 10, 9, 0x1ff); //clr all png
        /* log_info("stretch end PND 0x%x", iic_regs[id]->PND); */
#if 0//check rx data
        _run_cnt++;
        if (mode_cnt == 1) {
            log_info_hexdump(slave_rx_data, rx_cnt);
            u8 flag = 0;
            for (u8 i = 0; i < rx_cnt; i++) {
                if (i != slave_rx_data[i]) {
                    flag = 1;
                    log_error("num:0x%x,data:0x%x", i, slave_rx_data[i]);
                    break;
                }
            }
            if (flag == 0) {
                puts("slave stretch read test pass\n");
            } else {
                puts("slave stretch read test fail\n");
                _err_cnt++;
            }
            memset(slave_rx_data, 0, rx_cnt);
        }
        log_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~_err_cnt:%d, _run_cnt:%d", _err_cnt, _run_cnt);
#endif
        mode_cnt = 0;
        rx_cnt = 0;
        tx_cnt = 0;
        return;
    }
    if (hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)) {
        if ((mode_cnt == 1) && (hw_iic_get_pnd(iic, I2C_PND_RXDATA_DONE))) {
            slave_rx_data[rx_cnt] = iic_rx_buf_reg(iic_regs[0]);//rx reg
            hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
            hw_iic_clr_pnd(iic, I2C_PND_RXDATA_DONE);
            hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_ACK);//rx restart or rx data1
            rx_cnt++;
        } else if (mode_cnt == 2) {
            if (hw_iic_get_pnd(iic, I2C_PND_RXNACK)) {
                hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
                hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);
                // putchar('n');
                mode_cnt = 3;
                return;
            }
            hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
            hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);
            iic_tx_buf_reg(iic_regs[id]) = slave_tx_data[++tx_cnt];
            hw_i2c_put_task(iic, I2C_TASK_SEND_DATA);
        }
    }
}

#endif

