#include "asm/power_interface.h"
#include "power_api.h"
#include "app_config.h"
#include "gpio.h"
#include "wdt.h"

#define LOG_TAG_CONST       PMU
#define LOG_TAG             "[PMU]"
#include "log.h"

#define P33_IO_WKUP_IOMAP           POWER_WAKEUP_IO
#define P33_IO_WKUP_EDGE            FALLING_EDGE
#define P33_IO_LONG_PRESS_RESET     IO_PORTB_01
#define VDDIOM_VOL_CONFIG           VDDIOM_VOL_32V
#define FLASH_POWER_GATE_VDDIO  	0                   //flash power接vddio时配置为1，内封flash配置为0


/************************************************ power_param *******************************************************/
const struct low_power_param power_param = {
    .config         = SLEEP_EN,                         //0：sniff时芯片不进入低功耗  1：sniff时芯片进入powerdown
    .btosc_hz       = 16000000,                         //外接晶振频率
    .vddiom_lev     = VDDIOM_VOL_CONFIG,                //强VDDIO等级,可选：2.0V  2.2V  2.4V  2.6V  2.8V  3.0V  3.2V  3.6V
    .osc_type       = OSC_TYPE_LRC,                     //低功耗使用时钟
};

/**************************************************** wk_param *********************************************************/
const struct p33_io_wkup_config port0 = {
    .pullup_down_enable = ENABLE,                          //配置I/O 内部上下拉是否使能
    .edge       = P33_IO_WKUP_EDGE,                            //唤醒方式选择,可选：上升沿\下降沿
    .iomap      = P33_IO_WKUP_IOMAP,                         //唤醒口选择
};

const struct p33_io_wkup_param wk_param = {
    .port[1] = &port0,
};

void sleep_exit_callback()
{
    putchar('>');
}

void sleep_enter_callback()
{
    putchar('<');
}

void soff_latch_release_prepare()
{
    port_init();

    if ((P33_CON_GET(P3_RST_FLAG) & BIT(3)) == 0) {
        //wkup gpio init
        gpio_set_direction(POWER_WAKEUP_IO, 1);
        gpio_set_dieh(POWER_WAKEUP_IO, 1);
        if (port0.pullup_down_enable) {
            if (port0.edge == FALLING_EDGE) {
                gpio_set_pull_up(POWER_WAKEUP_IO, 1);
                gpio_set_pull_down(POWER_WAKEUP_IO, 0);
            } else {
                gpio_set_pull_up(POWER_WAKEUP_IO, 0);
                gpio_set_pull_down(POWER_WAKEUP_IO, 1);
            }

        }
    }
}



static void __mask_io_cfg()
{
    struct boot_soft_flag_t boot_soft_flag = {0};

    boot_soft_flag.flag0.boot_ctrl.sfc_fast_boot = 0;

    mask_softflag_config(&boot_soft_flag);
}

void board_set_soft_poweroff()
{
    u32 gpio_config[3] = {0xffff, 0xffff, 0xffff};
#define PORT_PROTECT(gpio)	gpio_config[gpio/IO_GROUP_NUM] &= ~BIT(gpio%IO_GROUP_NUM)

    PORT_PROTECT(POWER_WAKEUP_IO);

    //MCLR
    if (is_mclr_en()) {
        PORT_PROTECT(MCLR_PORT);
    }

    //长按复位
    if (is_pinr_en()) {
        u8 port_sel = get_pinr_port();
        if ((port_sel >= PA0_IN) && (port_sel <= PA12_IN)) {
            PORT_PROTECT(IO_PORTA_00 + (port_sel - PA0_IN));
        } else if ((port_sel >= PB0_IN) && (port_sel <= PB3_IN)) {
            PORT_PROTECT(IO_PORTB_00 + (port_sel - PB0_IN));
        }
    }

    //FLASH
    if (GET_SFC_PORT() == 0) {
        PORT_PROTECT(SPI0_CS_A);
        PORT_PROTECT(SPI0_CLK_A);
        PORT_PROTECT(SPI0_DO_D0_A);
        if (get_sfc_bit_mode() != 0) {
            PORT_PROTECT(SPI0_DI_D1_A);
        }
    }

    __mask_io_cfg();

    gpio_close(JL_PORTA, gpio_config[0]);
    gpio_close(JL_PORTB, gpio_config[1]);
    gpio_close(JL_PORTD, gpio_config[2]);
}

void sys_power_init()
{
    power_config_vdc13_cap(0);
    power_config_flash_pg_vddio(FLASH_POWER_GATE_VDDIO);
    power_config_wvdd_lev(WLDO_LEVEL_085V);
    power_config_sf_vddio_keep(VDDIO_KEEP_TYPE_NORMAL);
    power_config_pd_vddio_keep(VDDIO_KEEP_TYPE_NORMAL);

    power_init(&power_param);

    power_wakeup_init(&wk_param);

    gpio_longpress_pin0_reset_config(P33_IO_LONG_PRESS_RESET, 0, 0, 1, 0);
}


