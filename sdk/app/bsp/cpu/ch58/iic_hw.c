#include "iic_hw.h"
#include "wdt.h"
/* #include "asm/clock.h" */


/* #define LOG_TAG_CONST       IIC */
#define LOG_TAG             "[iic_hw]"
#include "log.h"

const struct iic_master_config hw_iic_cfg_const[MAX_HW_IIC_NUM] = {
    //iic0
    {
        .role = IIC_MASTER,
        .scl_io = IO_PORTA_01,//crossbar
        .sda_io = IO_PORTA_03,//crossbar
        .io_mode = GPIO_PULLUP_10K,
        .hdrive = GPIO_HD_2p4mA,
        .master_frequency = 100000,
        .io_filter = 3,
        .ie_en = 0,
    },
    //iic1
    {
        .role = IIC_MASTER,
        .scl_io = IO_PORTA_01,//crossbar
        .sda_io = IO_PORTA_03,//crossbar
        .io_mode = GPIO_PULLUP_10K,
        .hdrive = GPIO_HD_2p4mA,
        .master_frequency = 100000,
        .io_filter = 3,
        .ie_en = 0,
    }
};

static u8 hw_iic_state[MAX_HW_IIC_NUM] = {0, 0};//bit7:init ok, bit6:suspend, bit5:role:1:slave
static struct hw_iic_slave_config hw_iic_cfg_cache[MAX_HW_IIC_NUM];
static void (*iic_slave_irq_cbfun[MAX_HW_IIC_NUM])(void) = {NULL, NULL};

static JL_IIC_TypeDef *const iic_regs[MAX_HW_IIC_NUM] = {
    JL_IIC0,
    JL_IIC1,
};


#define SPIN_LOCK_EN 0
#if SPIN_LOCK_EN
#include "spinlock.h"
spinlock_t iic_lock[MAX_HW_IIC_NUM];
#define IIC_HW_ENTER_CRITICAL() \
	spin_lock(&iic_lock[iic])

#define IIC_HW_EXIT_CRITICAL() \
	spin_unlock(&iic_lock[iic])
#else
#define IIC_HW_ENTER_CRITICAL()
#define IIC_HW_EXIT_CRITICAL()
#endif

#if IIC_WHILE_TIMEOUT_EN
#define WHILE_TIMEOUT(x,y) \
    volatile u32 cnt = y; \
    while((x)&&(cnt!=0)){ \
        cnt--; \
    }
#else
#define WHILE_TIMEOUT(x,y) \
    while(x);
#endif

static int iic_port_init(hw_iic_dev iic)
{
    u32 fo_iicx_scl = 0, pfi_iicx_scl = 0;
    u32 fo_iicx_sda = 0, pfi_iicx_sda = 0;
    int ret = 0;
    if (iic == HW_IIC_0) {
        fo_iicx_scl = FO_IIC0_SCL;
        pfi_iicx_scl = PFI_IIC0_SCL;
        fo_iicx_sda = FO_IIC0_SDA;
        pfi_iicx_sda = PFI_IIC0_SDA;
    } else if (iic == HW_IIC_1) {
        fo_iicx_scl = FO_IIC1_SCL;
        pfi_iicx_scl = PFI_IIC1_SCL;
        fo_iicx_sda = FO_IIC1_SDA;
        pfi_iicx_sda = PFI_IIC1_SDA;
    } else {
        return IIC_ERROR_INDEX_ERROR;
    }

    gpio_set_direction(hw_iic_cfg_cache[iic].config.scl_io, 1);
    gpio_set_die(hw_iic_cfg_cache[iic].config.scl_io, 1);
    gpio_set_pull_down(hw_iic_cfg_cache[iic].config.scl_io, GPIO_PULLDOWN_DISABLE);
    gpio_set_pull_up(hw_iic_cfg_cache[iic].config.scl_io, hw_iic_cfg_cache[iic].config.io_mode);
    gpio_set_hd(hw_iic_cfg_cache[iic].config.scl_io, hw_iic_cfg_cache[iic].config.hdrive);
    gpio_set_spl(hw_iic_cfg_cache[iic].config.scl_io, 1);

    gpio_set_fun_output_port(hw_iic_cfg_cache[iic].config.scl_io, fo_iicx_scl, 1, 1);
    gpio_set_fun_input_port(hw_iic_cfg_cache[iic].config.scl_io, pfi_iicx_scl);

    gpio_set_direction(hw_iic_cfg_cache[iic].config.sda_io, 1);
    gpio_set_die(hw_iic_cfg_cache[iic].config.sda_io, 1);
    gpio_set_pull_down(hw_iic_cfg_cache[iic].config.sda_io, GPIO_PULLDOWN_DISABLE);
    gpio_set_pull_up(hw_iic_cfg_cache[iic].config.sda_io, hw_iic_cfg_cache[iic].config.io_mode);
    gpio_set_hd(hw_iic_cfg_cache[iic].config.sda_io, hw_iic_cfg_cache[iic].config.hdrive);
    gpio_set_spl(hw_iic_cfg_cache[iic].config.sda_io, 1);

    gpio_set_fun_output_port(hw_iic_cfg_cache[iic].config.sda_io, fo_iicx_sda, 1, 1);
    gpio_set_fun_input_port(hw_iic_cfg_cache[iic].config.sda_io, pfi_iicx_sda);

    return ret;
}

int hw_iic_set_baud(hw_iic_dev iic, u32 baud)
{
    //fiic_bus_scl =  fiic_clk /(2 * BUAD_CNT) + T电阻上拉
    //=> BUAD_CNT = fiic_clk / (2 * fiic_bus_scl)
    u32 sysclk;
    u16 baud_cnt = 0;
    //set iic clk =uart clk
    //get iic clk
    sysclk = clk_get("lsb");//sh57:uart br29,br50:lsb
    if (sysclk < 2 * baud) {
        return IIC_ERROR_FREQUENCY_ERROR;
    }
    baud_cnt = sysclk / (2 * baud);
    if (baud_cnt > 4095) {
        baud_cnt = 4095;
    } else if (baud_cnt < 4) {
        baud_cnt = 4;
    }
    iic_baud_reg(iic_regs[iic]) = baud_cnt;
    if (baud_cnt > 3) {
        iic_tsu_reg(iic_regs[iic]) = 2; //<=127 sda信号保持时间
        iic_thd_reg(iic_regs[iic]) = 2; //<=127 sda信号建立时间
    }
    /* log_info("iic%d sysclk:%d,cal-baud:0x%x\n", iic, sysclk, baud_cnt); */
    /* log_info("iic%d iic_tsu:%d,iic_thd:%d\n", iic, iic_tsu_reg(iic_regs[iic]), iic_thd_reg(iic_regs[iic])); */
    return 0;
}

struct iic_master_config *get_hw_iic_config(hw_iic_dev iic)
{
    return (struct iic_master_config *)&hw_iic_cfg_const[iic];
}

void hw_iic_isr0();
void hw_iic_isr1();
enum iic_state_enum hw_iic_master_init(hw_iic_dev iic, struct iic_master_config *i2c_config)
{
    if (i2c_config == NULL) {
        log_error("hw iic%d param error!", iic);
        return IIC_ERROR_PARAM_ERROR;
    }

    if (iic >= MAX_HW_IIC_NUM) {
        log_error("hw iic index:%d error!", iic);
        return IIC_ERROR_INDEX_ERROR;
    }
    if ((hw_iic_state[iic]&BIT(7)) != 0) {
        log_error("hw iic%d has been occupied!", iic);
        return IIC_ERROR_INIT_FAIL;
    }
    memcpy(&hw_iic_cfg_cache[iic], i2c_config, sizeof(struct iic_master_config));

    int ret = iic_port_init(iic);
    if (ret) {
        log_error("iic%d invalid hardware iic port\n", iic);
        return ret;
    }
    iic_init_prepare(iic_regs[iic]);
    iic_reset_pnd(iic_regs[iic]); //clr png
    if (i2c_config->role == IIC_MASTER) {
        iic_role_host(iic_regs[iic]);
        ret = hw_iic_set_baud(iic, i2c_config->master_frequency);
        if (ret) {
            log_error("iic%d baudrate is invalid\n", iic);
            return ret ;
        }
        /* } else { */
        /*     iic_role_slave(iic_regs[iic]); */
        /*     hw_iic_slave_set_addr(iic, i2c_config->slave_addr, 1); */
        /*     #<{(| iic_baddr_resp_en(iic_regs[iic]);//广播地址 |)}># */
        /*     iic_baddr_resp_dis(iic_regs[iic]);//广播地址 */
        /*     iic_slv_rx_manu(iic_regs[iic]); */
        /*     iic_slv_tx_manu(iic_regs[iic]); */
        /*     iic_tsu_reg(iic_regs[iic]) = 3; //<=127 */
        /*     iic_thd_reg(iic_regs[iic]) = 3; //<=127 */
    }
    iic_flt_sel(iic_regs[iic], i2c_config->io_filter);
    iic_stretch_en(iic_regs[iic]);//时钟延展
    /* iic_stretch_dis(iic_regs[iic]); */
#if MASTER_IIC_WRITE_MODE_FAST_RESP
    iic_ignore_nack_dis(iic_regs[iic]);//iic
#else
    iic_ignore_nack_dis(iic_regs[iic]);//iic
    /* iic_ignore_nack_en(iic_regs[iic]);//特殊用途 */
#endif

    if (i2c_config->ie_en) {
        if (iic == HW_IIC_1) {
            request_irq(IRQ_IIC1_IDX, HW_IIC1_IRQ_PRIORITY, hw_iic_isr1, 0);//3: 中断优先级
        } else {
            request_irq(IRQ_IIC0_IDX, HW_IIC0_IRQ_PRIORITY, hw_iic_isr0, 0);//3: 中断优先级
        }
        /* if (i2c_config->iic_slave_irq_callback) {//slave */
        /*     iic_slave_irq_cbfun[iic] = i2c_config->iic_slave_irq_callback; */
        /* } else { */
        /*     log_error("iic slave irq_callback is invalid\n"); */
        /* } */
#if SLAVE_NO_STRETCH_AUTO_TASK
        iic_stretch_dis(iic_regs[iic]);
        iic_slv_rx_auto(iic_regs[iic]);//rx
        iic_slv_tx_auto(iic_regs[iic]);//tx
        hw_iic_set_ie(iic, I2C_PND_TXTASK_LOAD, 1);
        hw_iic_set_ie(iic, I2C_PND_RXDATA_DONE, 1);
        hw_iic_set_ie(iic, I2C_PND_STOP, 1);
#else
        hw_iic_set_ie(iic, I2C_PND_ADR_MATCH, 1);
        hw_iic_set_ie(iic, I2C_PND_TASK_DONE, 1);
        hw_iic_set_ie(iic, I2C_PND_STOP, 1);
#endif
    } else {
        SFR(iic_png_reg(iic_regs[iic]), 0, 9, 0); //ie dis
    }

    SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1ff); //clr png
    hw_iic_state[iic] = BIT(7);//bit7:init ok, bit5:role:0:master
    iic_rst_release(iic_regs[iic]);//run

    /* #if SLAVE_NO_STRETCH_AUTO_TASK */
    /*     if (iic_info_role(iic) == IIC_SLAVE) { */
    /*         iic_tx_buf_reg(iic_regs[iic]) = 0;//first tx data */
    /*     } */
    /* #endif */
    /* log_info("hw iic%d init ok!", iic); */

#if 0
    log_info("iic%d info->scl      = %d", iic, i2c_config->scl_io);
    log_info("iic%d info->sda      = %d", iic, i2c_config->sda_io);
    log_info("iic%d info->baudrate = %d", iic, i2c_config->master_frequency);
    log_info("iic%d info->hdrive   = %d", iic, i2c_config->hdrive);
    log_info("iic%d info->io_filter = %d", iic, i2c_config->io_filter);
    log_info("iic%d info->io_mode    = %d", iic, i2c_config->io_mode);
    log_info("iic%d info->role     = %d", iic, i2c_config->role);
    log_info("iic%d IIC_CON  0x%x", iic, iic_regs[iic]->CON);
    log_info("iic%d IIC_TASK 0x%x", iic, iic_regs[iic]->TASK);
    log_info("iic%d IIC_PND  0x%x", iic, iic_regs[iic]->PND);
    log_info("iic%d IIC_ADDR 0x%x", iic, iic_regs[iic]->ADDR);
    log_info("iic%d IIC_BAUD 0x%x", iic, iic_regs[iic]->BAUD);
    log_info("iic%d IIC_TSU  0x%x", iic, iic_regs[iic]->TSU);
    log_info("iic%d IIC_THD  0x%x", iic, iic_regs[iic]->THD);
    log_info("iic%d IIC_DBG  0x%x", iic, iic_regs[iic]->DBG);
#endif
    return IIC_OK;
}
enum iic_state_enum hw_iic_init(hw_iic_dev iic)
{
    return hw_iic_master_init(iic, get_hw_iic_config(iic));
}

static enum iic_state_enum hw_iic_io_uninit(hw_iic_dev iic)
{
    enum iic_state_enum ret = IIC_OK;
    u32 pfi_iicx_scl = 0;
    u32 pfi_iicx_sda = 0;
    if (iic == HW_IIC_0) {
        pfi_iicx_scl = PFI_IIC0_SCL;
        pfi_iicx_sda = PFI_IIC0_SDA;
    } else if (iic == HW_IIC_1) {
        pfi_iicx_scl = PFI_IIC1_SCL;
        pfi_iicx_sda = PFI_IIC1_SDA;
    } else {
        return IIC_ERROR_INDEX_ERROR;
    }

    gpio_disable_fun_output_port(hw_iic_cfg_cache[iic].config.scl_io);
    gpio_disable_fun_input_port(pfi_iicx_scl);
    gpio_disable_fun_output_port(hw_iic_cfg_cache[iic].config.sda_io);
    gpio_disable_fun_input_port(pfi_iicx_sda);

    gpio_set_direction(hw_iic_cfg_cache[iic].config.scl_io, 1);
    gpio_set_die(hw_iic_cfg_cache[iic].config.scl_io, 0);
    gpio_set_pull_down(hw_iic_cfg_cache[iic].config.scl_io, GPIO_PULLDOWN_DISABLE);
    gpio_set_pull_up(hw_iic_cfg_cache[iic].config.scl_io, GPIO_PULLUP_DISABLE);
    gpio_set_hd(hw_iic_cfg_cache[iic].config.scl_io, GPIO_HD_2p4mA);
    gpio_set_spl(hw_iic_cfg_cache[iic].config.scl_io, 0);

    gpio_set_direction(hw_iic_cfg_cache[iic].config.sda_io, 1);
    gpio_set_die(hw_iic_cfg_cache[iic].config.sda_io, 0);
    gpio_set_pull_down(hw_iic_cfg_cache[iic].config.sda_io, GPIO_PULLDOWN_DISABLE);
    gpio_set_pull_up(hw_iic_cfg_cache[iic].config.sda_io, GPIO_PULLUP_DISABLE);
    gpio_set_hd(hw_iic_cfg_cache[iic].config.sda_io, GPIO_HD_2p4mA);
    gpio_set_spl(hw_iic_cfg_cache[iic].config.sda_io, 0);
    return ret;
}

enum iic_state_enum hw_iic_uninit(hw_iic_dev iic)
{
    if (hw_iic_state[iic] == 0) {
        log_error("hw iic%d has been no init!", iic);
        return IIC_ERROR_NO_INIT;
    }

    enum iic_state_enum iic_state = hw_iic_io_uninit(iic);
    if (iic_state != IIC_OK) {
        return iic_state;
    }

    hw_iic_state[iic] = 0;//no init
    iic_disable(iic_regs[iic]);
    /* log_info("hw iic%d close!", iic); */
    // iic_rst_rst(iic_regs[iic]);
    return IIC_OK;
}


enum iic_state_enum hw_iic_suspend(hw_iic_dev iic)
{
    if ((hw_iic_state[iic] & 0xc0) != 0x80) {
        log_error("hw iic%d is no init or suspend!", iic);
        return IIC_ERROR_SUSPEND_FAIL;
    }
    if ((hw_iic_state[iic] & 0x0f) != 0) {
        log_error("hw iic%d is busy!", iic);
        return IIC_ERROR_BUSY;
    }

    enum iic_state_enum iic_state = hw_iic_io_uninit(iic);
    if (iic_state != IIC_OK) {
        return iic_state;
    }

    IIC_HW_ENTER_CRITICAL();
    hw_iic_state[iic] |= BIT(6);//bit6:suspend ok
    iic_disable(iic_regs[iic]);
    IIC_HW_EXIT_CRITICAL();
    return IIC_OK;
}

//return:0:ok, <0:error
enum iic_state_enum hw_iic_resume(hw_iic_dev iic)
{
    if ((hw_iic_state[iic] & 0xc0) != 0xc0) {
        log_error("hw iic%d is no init or no suspend!", iic);
        return IIC_ERROR_RESUME_FAIL;
    }

    int ret = iic_port_init(iic);
    if (ret) {
        log_error("iic%d invalid hardware iic port\n", iic);
        return ret;
    }
    IIC_HW_ENTER_CRITICAL();
    hw_iic_state[iic] &= ~ BIT(6); //resume ok
    SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1ff); //clr png
    iic_enable(iic_regs[iic]);
    iic_rst_release(iic_regs[iic]);//run
#if SLAVE_NO_STRETCH_AUTO_TASK
    if (hw_iic_cfg_cache[iic].config.role == IIC_SLAVE) {
        iic_tx_buf_reg(iic_regs[iic]) = 0;//first tx data
    }
#endif
    IIC_HW_EXIT_CRITICAL();
    return IIC_OK;
}

//return:0:error, 1:ok
enum iic_state_enum hw_iic_check_busy(hw_iic_dev iic)
{
    if (hw_iic_state[iic] & 0x0f) {
        return IIC_ERROR_BUSY;//error
    }
    IIC_HW_ENTER_CRITICAL();
    hw_iic_state[iic]++;//busy
    IIC_HW_EXIT_CRITICAL();
    return IIC_OK;//ok
}
void hw_iic_idle(hw_iic_dev iic)
{
    hw_iic_state[iic] &= 0xf0;//idle
}



void hw_i2c_put_task(hw_iic_dev iic, i2c_task_typedef task)
{
    iic_task_sel(iic_regs[iic], task);
}

void hw_iic_set_ie(hw_iic_dev iic, i2c_pnd_typedef png, u8 en)
{
    if (en) {
        iic_png_reg(iic_regs[iic]) |= (png >> 20);
    } else {
        iic_png_reg(iic_regs[iic]) &= ~(png >> 20);
    }
}
u8 hw_iic_get_pnd(hw_iic_dev iic, i2c_pnd_typedef png)
{
    if (iic_png_reg(iic_regs[iic])&png) {
        return 1;
    }
    return 0;
}
void hw_iic_clr_pnd(hw_iic_dev iic, i2c_pnd_typedef png)
{
    iic_png_reg(iic_regs[iic]) |= (png >> 10);
}

void hw_iic_clr_all_pnd(hw_iic_dev iic)
{
    IIC_HW_ENTER_CRITICAL();
    SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1ff); //clr all png
    IIC_HW_EXIT_CRITICAL();
}







static u8 start_signal[MAX_HW_IIC_NUM] = {0, 0};
void hw_iic_start(hw_iic_dev iic)
{
    IIC_HW_ENTER_CRITICAL();
    start_signal[iic] >>= 1;
    start_signal[iic] |= BIT(7);
    IIC_HW_EXIT_CRITICAL();
}


#if MASTER_IIC_WRITE_MODE_FAST_RESP
static u8 addr_cnt[MAX_HW_IIC_NUM] = {0, 0};
#endif
//1:ack
u8 hw_iic_start_addr(hw_iic_dev iic, u8 addr)
{
    iic_addr_reg(iic_regs[iic]) = addr;
    hw_i2c_put_task(iic, I2C_TASK_SEND_ADDR);
#if MASTER_IIC_WRITE_MODE_FAST_RESP
    if (addr_cnt[iic] == 1) {
        /* while(!hw_iic_get_pnd(iic,I2C_PND_START));//主机不起start_png */
        WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)), IIC_WHILE_TIMEOUT_CNT_); //主机不起start_png
        if (hw_iic_get_pnd(iic, I2C_PND_RXNACK)) {
            log_error("iic%d tx reg ack fail!", iic);
            WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)), IIC_WHILE_TIMEOUT_CNT_);
            SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1ff); //clr all png
            addr_cnt[iic] = 0;
            start_signal[iic] = 0;
            return 0;
        }
        hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
        hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);
        hw_iic_clr_pnd(iic, I2C_PND_RXACK);
    }
    addr_cnt[iic]++;
#endif
    WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)), IIC_WHILE_TIMEOUT_CNT_); //wait addr done
    hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
    hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);
    hw_iic_clr_pnd(iic, I2C_PND_START);
    if (hw_iic_get_pnd(iic, I2C_PND_RXACK)) {
        hw_iic_clr_pnd(iic, I2C_PND_RXACK);
        hw_iic_clr_pnd(iic, I2C_PND_RXNACK);
        return 1;
    }
    /* log_error("iic%d tx addr ack fail!", iic); */
    hw_iic_clr_pnd(iic, I2C_PND_RXNACK);
    start_signal[iic] = 0;
#if MASTER_IIC_WRITE_MODE_FAST_RESP
    addr_cnt[iic] = 0;
#endif
    return 0;
}

void hw_iic_stop(hw_iic_dev iic)
{
    IIC_HW_ENTER_CRITICAL();
    hw_i2c_put_task(iic, I2C_TASK_SEND_STOP);
    /* hw_i2c_put_task(iic, I2C_TASK_SEND_NACK_STOP); */
#if MASTER_IIC_WRITE_MODE_FAST_RESP
    if (addr_cnt[iic] == 1) {
        /* while(!hw_iic_get_pnd(iic,I2C_PND_STOP));//主机不起stop_png */
        WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)), IIC_WHILE_TIMEOUT_CNT_);
        hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
    }
#if MASTER_IIC_READ_MODE_FAST_RESP
    else if (addr_cnt[iic] == 2) {
        WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)), IIC_WHILE_TIMEOUT_CNT_);
        hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
        //read iic_rx_buf_reg(iic_regs[iic]);
    }
#endif
    addr_cnt[iic] = 0;
#endif
    WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)), IIC_WHILE_TIMEOUT_CNT_);
    start_signal[iic] = 0;
    hw_iic_idle(iic);
    /* log_info("iic%d stop IIC_PND 0x%x", iic, iic_regs[iic]->PND); */
    SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1ff); //clr all png
    IIC_HW_EXIT_CRITICAL();
}

void hw_iic_reset(hw_iic_dev iic)
{
    IIC_HW_ENTER_CRITICAL();
    hw_i2c_put_task(iic, I2C_TASK_SEND_RESET);
    WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)), IIC_WHILE_TIMEOUT_CNT_); //wait addr done
    start_signal[iic] = 0;
    hw_iic_idle(iic);
    /* log_info("iic%d stop IIC_PND 0x%x", iic, iic_regs[iic]->PND); */
    SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1ff); //clr all png
    IIC_HW_EXIT_CRITICAL();
}

//与其他从机验证ignore_nack模式
//1:ack
u8 hw_iic_tx_byte(hw_iic_dev iic, u8 byte)
{
    u8 ack = 0;
    IIC_HW_ENTER_CRITICAL();
    if (start_signal[iic] & BIT(7)) {
        start_signal[iic] >>= 1;
        /* log_info("st/"); */
        ack = hw_iic_start_addr(iic, byte);//addr + w/r
        goto __iic_tx_byte_end;
    }

    iic_tx_buf_reg(iic_regs[iic]) = byte;
    hw_i2c_put_task(iic, I2C_TASK_SEND_DATA);
#if MASTER_IIC_WRITE_MODE_FAST_RESP
    WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TXTASK_LOAD)), IIC_WHILE_TIMEOUT_CNT_);
    hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);
    hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
    if ((hw_iic_get_pnd(iic, I2C_PND_RXACK) == 0) && (hw_iic_get_pnd(iic, I2C_PND_RXNACK) == 0)) {
        ack = 1;//MASTER_IIC_WRITE_MODE_FAST_RESP标记有应答
        goto __iic_tx_byte_end;
    }

#else
    WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)), IIC_WHILE_TIMEOUT_CNT_);
    hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
    hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);
#endif
    if (hw_iic_get_pnd(iic, I2C_PND_RXACK)) {
        hw_iic_clr_pnd(iic, I2C_PND_RXACK);
        hw_iic_clr_pnd(iic, I2C_PND_RXNACK);
        ack = 1;
        goto __iic_tx_byte_end;
    }
    /* log_error("iic%d tx data ack fail!", iic); */
    hw_iic_clr_pnd(iic, I2C_PND_RXNACK);
    ack = 0;
__iic_tx_byte_end:
    IIC_HW_EXIT_CRITICAL();
    return ack;
}

void hw_iic_set_ack(hw_iic_dev iic, u8 ack_en)
{
    IIC_HW_ENTER_CRITICAL();
    if (ack_en) {
        hw_i2c_put_task(iic, I2C_TASK_SEND_ACK);
    } else {
        hw_i2c_put_task(iic, I2C_TASK_SEND_NACK);
    }
    WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)), IIC_WHILE_TIMEOUT_CNT_);
    hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
    IIC_HW_EXIT_CRITICAL();
}
//ack:0:nack, 1:ack, 2:only rx data
u8 hw_iic_rx_byte(hw_iic_dev iic, u8 ack)
{
    IIC_HW_ENTER_CRITICAL();
    if (ack == 1) {
        hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_ACK);
    } else if (ack == 0) {
        hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_NACK);
    } else if (ack == 2) { //特殊
        hw_i2c_put_task(iic, I2C_TASK_RECV_DATA);
    }

#if MASTER_IIC_READ_MODE_FAST_RESP
    WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_RXTASK_LOAD)), IIC_WHILE_TIMEOUT_CNT_);
    hw_iic_clr_pnd(iic, I2C_PND_RXTASK_LOAD);
    hw_iic_clr_pnd(iic, I2C_PND_RXDATA_DONE);
    hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
#else
    WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)), IIC_WHILE_TIMEOUT_CNT_);
    hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
    hw_iic_clr_pnd(iic, I2C_PND_RXDATA_DONE);
#endif
    u8 rx_byte = iic_rx_buf_reg(iic_regs[iic]);
    IIC_HW_EXIT_CRITICAL();
    return rx_byte;
}

//return: =len:ok
int hw_iic_read_buf(hw_iic_dev iic, void *buf, int len)
{
    int i;

    if (!buf || !len) {
        return IIC_ERROR_PARAM_ERROR;
    }
    IIC_HW_ENTER_CRITICAL();
    for (i = 0; i < len; i++) {
        if (i == len - 1) {
            hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_NACK);
        } else {
            hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_ACK);
        }
        WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)), IIC_WHILE_TIMEOUT_CNT_);
        hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
        hw_iic_clr_pnd(iic, I2C_PND_RXDATA_DONE);
        ((u8 *)buf)[i] = iic_rx_buf_reg(iic_regs[iic]);
    }
    IIC_HW_EXIT_CRITICAL();
    return len;
}

//return: =len:ok
int hw_iic_write_buf(hw_iic_dev iic, const void *buf, int len)
{
    int i = 0;

    if (!buf || !len) {
        return IIC_ERROR_PARAM_ERROR;
    }

    IIC_HW_ENTER_CRITICAL();
    for (i = 0; i < len; i++) {
        iic_tx_buf_reg(iic_regs[iic]) = ((u8 *)buf)[i];
        hw_i2c_put_task(iic, I2C_TASK_SEND_DATA);
        WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)), IIC_WHILE_TIMEOUT_CNT_);
        hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
        hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);

        if (hw_iic_get_pnd(iic, I2C_PND_RXNACK)) {
            hw_iic_clr_pnd(iic, I2C_PND_RXACK);
            hw_iic_clr_pnd(iic, I2C_PND_RXNACK);
            break;
        }
    }
    IIC_HW_EXIT_CRITICAL();
    return i;
}






/******************************hw iic slave*****************************/
enum iic_state_enum hw_iic_slave_init(hw_iic_dev iic, struct hw_iic_slave_config *i2c_config)
{
    if (i2c_config == NULL) {
        log_error("hw iic%d param error!", iic);
        return IIC_ERROR_PARAM_ERROR;
    }

    if (iic >= MAX_HW_IIC_NUM) {
        log_error("hw iic index:%d error!", iic);
        return IIC_ERROR_INDEX_ERROR;
    }
    if ((hw_iic_state[iic]&BIT(7)) != 0) {
        log_error("hw iic%d has been occupied!", iic);
        return IIC_ERROR_INIT_FAIL;
    }
    memcpy(&hw_iic_cfg_cache[iic], i2c_config, sizeof(struct hw_iic_slave_config));

    int ret = iic_port_init(iic);
    if (ret) {
        log_error("iic%d invalid hardware iic port\n", iic);
        return ret;
    }
    iic_init_prepare(iic_regs[iic]);
    iic_reset_pnd(iic_regs[iic]); //clr png
    if (i2c_config->config.role == IIC_MASTER) {
        /* iic_role_host(iic_regs[iic]); */
        /* ret = hw_iic_set_baud(iic, i2c_config->master_frequency); */
        /* if (ret) { */
        /*     log_error("iic%d baudrate is invalid\n", iic); */
        /*     return ret ; */
        /* } */
    } else {
        iic_role_slave(iic_regs[iic]);
        hw_iic_slave_set_addr(iic, i2c_config->slave_addr, 1);
        /* iic_baddr_resp_en(iic_regs[iic]);//广播地址 */
        iic_baddr_resp_dis(iic_regs[iic]);//广播地址
        iic_slv_rx_manu(iic_regs[iic]);
        iic_slv_tx_manu(iic_regs[iic]);
        iic_tsu_reg(iic_regs[iic]) = 3; //<=127
        iic_thd_reg(iic_regs[iic]) = 3; //<=127
    }
    iic_flt_sel(iic_regs[iic], i2c_config->config.io_filter);
    iic_stretch_en(iic_regs[iic]);//时钟延展
    /* iic_stretch_dis(iic_regs[iic]); */
#if MASTER_IIC_WRITE_MODE_FAST_RESP
    iic_ignore_nack_dis(iic_regs[iic]);//iic
#else
    iic_ignore_nack_dis(iic_regs[iic]);//iic
    /* iic_ignore_nack_en(iic_regs[iic]);//特殊用途 */
#endif

    if (i2c_config->config.ie_en) {
        if (iic == 1) {
            request_irq(IRQ_IIC1_IDX, HW_IIC1_IRQ_PRIORITY, hw_iic_isr1, 0);//3: 中断优先级
        } else {
            request_irq(IRQ_IIC0_IDX, HW_IIC0_IRQ_PRIORITY, hw_iic_isr0, 0);//3: 中断优先级
        }
        if (i2c_config->iic_slave_irq_callback) {//slave
            iic_slave_irq_cbfun[iic] = i2c_config->iic_slave_irq_callback;
        } else {
            log_error("iic slave irq_callback is invalid\n");
        }
#if SLAVE_NO_STRETCH_AUTO_TASK
        iic_stretch_dis(iic_regs[iic]);
        iic_slv_rx_auto(iic_regs[iic]);//rx
        iic_slv_tx_auto(iic_regs[iic]);//tx
        hw_iic_set_ie(iic, I2C_PND_TXTASK_LOAD, 1);
        hw_iic_set_ie(iic, I2C_PND_RXDATA_DONE, 1);
        hw_iic_set_ie(iic, I2C_PND_STOP, 1);
#else
        hw_iic_set_ie(iic, I2C_PND_ADR_MATCH, 1);
        hw_iic_set_ie(iic, I2C_PND_TASK_DONE, 1);
        hw_iic_set_ie(iic, I2C_PND_STOP, 1);
#endif
    } else {
        SFR(iic_png_reg(iic_regs[iic]), 0, 9, 0); //ie dis
    }

    SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1ff); //clr png
    hw_iic_state[iic] = BIT(7) | BIT(5);//bit7:init ok, bit5:role:1:slave
    iic_rst_release(iic_regs[iic]);//run

#if SLAVE_NO_STRETCH_AUTO_TASK
    if (i2c_config->config.role == IIC_SLAVE) {
        iic_tx_buf_reg(iic_regs[iic]) = 0;//first tx data
    }
#endif

#if 0
    log_info("iic%d info->scl      = %d", iic, i2c_config->scl_io);
    log_info("iic%d info->sda      = %d", iic, i2c_config->sda_io);
    /* log_info("iic%d info->baudrate = %d", iic, i2c_config->master_frequency); */
    log_info("iic%d info->hdrive   = %d", iic, i2c_config->hdrive);
    log_info("iic%d info->io_filter = %d", iic, i2c_config->io_filter);
    log_info("iic%d info->io_mode    = %d", iic, i2c_config->io_mode);
    log_info("iic%d info->role     = %d", iic, i2c_config->role);
    log_info("iic%d IIC_CON  0x%x", iic, iic_regs[iic]->CON);
    log_info("iic%d IIC_TASK 0x%x", iic, iic_regs[iic]->TASK);
    log_info("iic%d IIC_PND  0x%x", iic, iic_regs[iic]->PND);
    log_info("iic%d IIC_ADDR 0x%x", iic, iic_regs[iic]->ADDR);
    log_info("iic%d IIC_BAUD 0x%x", iic, iic_regs[iic]->BAUD);
    log_info("iic%d IIC_TSU  0x%x", iic, iic_regs[iic]->TSU);
    log_info("iic%d IIC_THD  0x%x", iic, iic_regs[iic]->THD);
    log_info("iic%d IIC_DBG  0x%x", iic, iic_regs[iic]->DBG);
#endif
    return IIC_OK;
}

void hw_iic_slave_set_addr(hw_iic_dev iic, u8 addr, u8 addr_ack)
{
    IIC_HW_ENTER_CRITICAL();
    if (addr_ack) {
        iic_addr_resp_auto(iic_regs[iic]);
    } else {
        iic_addr_resp_manu(iic_regs[iic]);
    }
    iic_addr_reg(iic_regs[iic]) = addr; //高7为有效
    IIC_HW_EXIT_CRITICAL();
}
u8 hw_iic_slave_get_addr(hw_iic_dev iic)
{
    return iic_baud_reg(iic_regs[iic]) & 0xfe;
}

void hw_iic_slave_set_callback(hw_iic_dev iic, void (*iic_slave_irq_callback)(void))
{
    IIC_HW_ENTER_CRITICAL();
    if (iic_slave_irq_callback) {
        iic_slave_irq_cbfun[iic] = iic_slave_irq_callback;
    }
    IIC_HW_EXIT_CRITICAL();
}

enum iic_slave_rx_state hw_iic_slave_rx_prepare(hw_iic_dev iic, u8 ack, u32 wait_time)
{
    enum iic_slave_rx_state ret;
    IIC_HW_ENTER_CRITICAL();
    while (!hw_iic_get_pnd(iic, I2C_PND_STOP)) {
        if (hw_iic_get_pnd(iic, I2C_PND_RXDATA_DONE)) {//收到数据
            ret = IIC_SLAVE_RX_PREPARE_OK;//rx data ok
            goto __iic_s_rx_p_end;
        }
        if ((hw_iic_get_pnd(iic, I2C_PND_START)) || (hw_iic_get_pnd(iic, I2C_PND_ADR_MATCH))) {
            while (hw_iic_get_pnd(iic, I2C_PND_ADR_MATCH)) { //收到地址且匹配
                ret = IIC_SLAVE_RX_PREPARE_OK;//rx data ok
                goto __iic_s_rx_p_end;
            }
        }
        if (wait_time == 0) {
            SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1ff); //clr all png
            ret = IIC_SLAVE_RX_PREPARE_TIMEOUT;//error
            goto __iic_s_rx_p_end;
        }
        wait_time--;
        wdt_clear();
    }
    SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1ff); //clr all png
    ret = IIC_SLAVE_RX_PREPARE_END_OK;//rx end ok
__iic_s_rx_p_end:
    IIC_HW_EXIT_CRITICAL();
    return ret;
}

enum iic_slave_rx_state hw_iic_slave_rx_byte(hw_iic_dev iic, u8 *rx_byte)
{
    enum iic_slave_rx_state ret;
    IIC_HW_ENTER_CRITICAL();
    *rx_byte = iic_rx_buf_reg(iic_regs[iic]);
    if (hw_iic_get_pnd(iic, I2C_PND_ADR_MATCH)) {
        if (*rx_byte != (hw_iic_cfg_cache[iic].slave_addr)) {
            if (*rx_byte != (hw_iic_cfg_cache[iic].slave_addr + 1)) {
                //iic_buf_reg(iic_regs[iic]) = 0xff;
                SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1ff); //clr all png
                put_u8hex(*rx_byte);
                /* putchar('1'); */
                ret = IIC_SLAVE_RX_ADDR_NO_MATCH;//err:start_ok,addr_err
                goto __iic_s_rx_b_end;
            }
            hw_iic_clr_pnd(iic, I2C_PND_RXDATA_DONE);
            hw_iic_clr_pnd(iic, I2C_PND_ADR_MATCH);
            hw_iic_clr_pnd(iic, I2C_PND_START);
            WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)), IIC_WHILE_TIMEOUT_CNT_);
            hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
            ret = IIC_SLAVE_RX_ADDR_TX;//tx
            goto __iic_s_rx_b_end;
        }
        //rx addr
        hw_iic_clr_pnd(iic, I2C_PND_RXDATA_DONE);
        hw_iic_clr_pnd(iic, I2C_PND_ADR_MATCH);
        hw_iic_clr_pnd(iic, I2C_PND_START);
        WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)), IIC_WHILE_TIMEOUT_CNT_);
        hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
        hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_ACK);//rx
        ret = IIC_SLAVE_RX_ADDR_RX;//rx
        goto __iic_s_rx_b_end;
    }
    //rx data
    hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
    hw_iic_clr_pnd(iic, I2C_PND_RXDATA_DONE);
    hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_ACK);//rx
    /* put_u8hex(*rx_byte); */
    ret = IIC_SLAVE_RX_DATA;
__iic_s_rx_b_end:
    IIC_HW_EXIT_CRITICAL();
    return ret;
}
int hw_iic_slave_rx_nbyte(hw_iic_dev iic, u8 *rx_buf)//含结束位 轮询
{
    int rx_cnt = 0;
    IIC_HW_ENTER_CRITICAL();
    while (!hw_iic_get_pnd(iic, I2C_PND_STOP)) {
        while (hw_iic_get_pnd(iic, I2C_PND_RXDATA_DONE)) {
            rx_buf[rx_cnt++] = iic_rx_buf_reg(iic_regs[iic]);//
            hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
            hw_iic_clr_pnd(iic, I2C_PND_RXDATA_DONE);
            //prepare
            hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_ACK);//rx
        }
    }
    SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1ff); //clr all png
    IIC_HW_EXIT_CRITICAL();
    return rx_cnt;
}
void hw_iic_slave_tx_byte(hw_iic_dev iic, u8 byte)
{
    IIC_HW_ENTER_CRITICAL();
    hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
    hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);
    iic_tx_buf_reg(iic_regs[iic]) = byte;
    hw_i2c_put_task(iic, I2C_TASK_SEND_DATA);
    IIC_HW_EXIT_CRITICAL();
}
int hw_iic_slave_tx_nbyte(hw_iic_dev iic, u8 *tx_buf)
{
    int tx_cnt = 0;
    IIC_HW_ENTER_CRITICAL();
    while (1) {
        WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)), IIC_WHILE_TIMEOUT_CNT_);
        if (hw_iic_get_pnd(iic, I2C_PND_RXNACK)) {
            hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
            hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);
            break;
        }
        hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
        hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);
        iic_tx_buf_reg(iic_regs[iic]) = tx_buf[tx_cnt++];
        hw_i2c_put_task(iic, I2C_TASK_SEND_DATA);
    }
    WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_STOP)), IIC_WHILE_TIMEOUT_CNT_);
    SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1ff); //clr all png
    IIC_HW_EXIT_CRITICAL();
    return tx_cnt;
}
u8 hw_iic_slave_tx_check_ack(hw_iic_dev iic)
{
    if (hw_iic_get_pnd(iic, I2C_PND_RXNACK)) {
        IIC_HW_ENTER_CRITICAL();
        hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
        hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);
        IIC_HW_EXIT_CRITICAL();
        return 0;
    }
    return 1;
}





static u8 *slave_tx_data[MAX_HW_IIC_NUM] = {NULL, NULL};
static u8 *slave_rx_data[MAX_HW_IIC_NUM] = {NULL, NULL};
static u8 mode_cnt[MAX_HW_IIC_NUM] = {0, 0};
static u8 iic_data_len = 0;
void iic_slave_isr_set_buf(hw_iic_dev iic, u8 *tx_buf, u8 *rx_buf)
{
    if (tx_buf) {
        slave_tx_data[iic] = tx_buf;
    }
    if (tx_buf) {
        slave_rx_data[iic] = rx_buf;
    }
}
//return:0:err,1:rx; 2:tx
u8 get_iic_slave_isr_end_flag(hw_iic_dev iic, u8 *len)
{
    u8 ret = 0;
    *len = iic_data_len;
    if (mode_cnt[iic] == 0x10) {//rx
        mode_cnt[iic] = 0;
        ret = 1;
    } else if (mode_cnt[iic] == 0x20) {//tx
        mode_cnt[iic] = 0;
        ret = 2;
    }
    return ret;
}
#if SLAVE_NO_STRETCH_AUTO_TASK
//中断法: hw_iic_slave_config结构体配置从机模式,地址,使能iic中断即可
//从机中断接收不可被其它打断(如高优先级中断)，否则可能丢失结束信号导致出错
AT(.iic.text.cache.L1)
void hw_iic_slave_isr(u8 iic)//slave  no_stretch
{
    static u8 rx_cnt[MAX_HW_IIC_NUM] = {0, 0};
    static u8 tx_cnt[MAX_HW_IIC_NUM] = {0, 0};
    if (hw_iic_get_pnd(iic, I2C_PND_TXTASK_LOAD)) {
        //发送准备(1个byte时间)

        iic_tx_buf_reg(iic_regs[iic]) = slave_tx_data[iic][++tx_cnt[iic]];
        /* hw_i2c_put_task(iic, I2C_TASK_SEND_DATA); */
        mode_cnt[iic] = 2;
        SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1fb); //clr all png
        return;
    }
    if (hw_iic_get_pnd(iic, I2C_PND_RXDATA_DONE)) {
        //接收准备(1个byte时间)

        slave_rx_data[iic][rx_cnt[iic]] = iic_rx_buf_reg(iic_regs[iic]);//rx reg
        SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1fb); //clr all png
        if (hw_iic_get_pnd(iic, I2C_PND_STOP) == 0) {
            hw_i2c_put_task(iic, I2C_TASK_SEND_ACK);// rx data1
        }
        mode_cnt[iic] = 1;
        rx_cnt[iic]++;
    }

    if (hw_iic_get_pnd(iic, I2C_PND_STOP)) {

        /* while(!hw_iic_get_pnd(iic,I2C_PND_TASK_DONE));//0x1230 */
        SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1ff); //clr all png
        /* log_info("iic%d no_stretch end PND 0x%x", iic, iic_regs[iic]->PND); */
#if 0//check rx data
        _run_cnt[iic]++;
        if (mode_cnt[iic] == 1) {
            log_info_hexdump(slave_rx_data[iic], rx_cnt[iic]);
            memset(slave_rx_data[iic], 0, rx_cnt[iic]);
        }
        /* log_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~_err_cnt[%d]:%d, _run_cnt[%d]:%d", iic, _err_cnt[iic], iic, _run_cnt[iic]); */
#endif
        if (mode_cnt[iic] == 1) {//rx
            iic_data_len = rx_cnt[iic];
            mode_cnt[iic] == 0x10;
        } else { //tx
            iic_data_len = tx_cnt[iic];
            mode_cnt[iic] == 0x20;
        }
        rx_cnt[iic] = 0;
        tx_cnt[iic] = 0;
        iic_tx_buf_reg(iic_regs[iic]) = slave_tx_data[iic][0];
        return;
    }
}
#else
AT(.iic.text.cache.L1)
void hw_iic_slave_isr(u8 iic)//slave  stretch
{
    static u8 rx_cnt[MAX_HW_IIC_NUM] = {0, 0};
    static u8 tx_cnt[MAX_HW_IIC_NUM] = {0, 0};
    if (hw_iic_get_pnd(iic, I2C_PND_ADR_MATCH)) {
        WHILE_TIMEOUT((!hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)), IIC_WHILE_TIMEOUT_CNT_); //0x1230
        SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1ff); //clr all png
        if (iic_rx_buf_reg(iic_regs[iic]) == (hw_iic_cfg_cache[iic].slave_addr + 1)) {
            //tx data,,,,,,,,,,,,,,,,,,
            iic_tx_buf_reg(iic_regs[iic]) = slave_tx_data[iic][tx_cnt[iic]];
            hw_i2c_put_task(iic, I2C_TASK_SEND_DATA);
            mode_cnt[iic] = 2; //tx
        } else {
            hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_ACK);//rx reg
            // put_u32hex(iic_regs[iic]->PND);
            mode_cnt[iic] = 1; //rx
        }
        return;
    }
    if (hw_iic_get_pnd(iic, I2C_PND_STOP)) {
        /* while(!hw_iic_get_pnd(iic,I2C_PND_TASK_DONE));//0x1230 */
        SFR(iic_png_reg(iic_regs[iic]), 10, 9, 0x1ff); //clr all png
        /* log_info("iic%d stretch end PND 0x%x", iic, iic_regs[iic]->PND); */
#if 0//check rx data
        _run_cnt[iic]++;
        if (mode_cnt[iic] == 1) {
            log_info_hexdump(slave_rx_data[iic], rx_cnt[iic]);
            memset(slave_rx_data[iic], 0, rx_cnt[iic]);
        }
        /* log_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~_err_cnt[%d]:%d, _run_cnt[%d]:%d", iic, _err_cnt[iic], iic, _run_cnt[iic]); */
#endif
        if (mode_cnt[iic] == 1) {//rx
            iic_data_len = rx_cnt[iic];
            mode_cnt[iic] = 0x10;
        } else { //tx
            iic_data_len = tx_cnt[iic];
            mode_cnt[iic] = 0x20;
        }
        rx_cnt[iic] = 0;
        tx_cnt[iic] = 0;
        return;
    }
    if (hw_iic_get_pnd(iic, I2C_PND_TASK_DONE)) {
        if ((mode_cnt[iic] == 1) && (hw_iic_get_pnd(iic, I2C_PND_RXDATA_DONE))) {
            slave_rx_data[iic][rx_cnt[iic]] = iic_rx_buf_reg(iic_regs[iic]);//rx reg
            hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
            hw_iic_clr_pnd(iic, I2C_PND_RXDATA_DONE);
            hw_i2c_put_task(iic, I2C_TASK_RECV_DATA_ACK);//rx restart or rx data1
            rx_cnt[iic]++;
        } else if (mode_cnt[iic] == 2) {
            if (hw_iic_get_pnd(iic, I2C_PND_RXNACK)) {
                // put_u32hex(iic_regs[iic]->PND);
                hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
                hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);
                // putchar('n');
                mode_cnt[iic] = 3;
                return;
            }
            hw_iic_clr_pnd(iic, I2C_PND_TASK_DONE);
            hw_iic_clr_pnd(iic, I2C_PND_TXTASK_LOAD);
            iic_tx_buf_reg(iic_regs[iic]) = slave_tx_data[iic][++tx_cnt[iic]];
            hw_i2c_put_task(iic, I2C_TASK_SEND_DATA);
        }
    }
}

#endif

AT(.iic.text.cache.L1)
___interrupt
void hw_iic_isr0()//slave  stretch
{
    if (iic_slave_irq_cbfun[0]) {
        iic_slave_irq_cbfun[0]();
    } else {
        hw_iic_slave_isr(0);
    }
}

AT(.iic.text.cache.L1)
___interrupt
void hw_iic_isr1()//slave  stretch
{
    if (iic_slave_irq_cbfun[1]) {
        iic_slave_irq_cbfun[1]();
    } else {
        hw_iic_slave_isr(1);
    }
}

