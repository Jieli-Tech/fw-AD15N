/*********************************************************************************************
    *   Filename        : power_api.c

    *   Description     : 低功耗所用接口及流程

    *   Author          : Sunlicheng

    *   Email           : Sunlicheng@zh-jieli.com

    *   Last modifiled  : 2021-07-13 11:50

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/

#include "power_api.h"
#include "asm/power_interface.h"
#include "asm/power/p33.h"
#include "gpio.h"
#include "tick_timer_driver.h"
#include "wdt.h"
#include "app_config.h"
#include "maskrom.h"

#define ENABLE								1
#define DISABLE								0

#define LOG_TAG_CONST       PWRA
#define LOG_TAG             "[pwra]"
#include "debug.h"

/************************************************ power_param *******************************************************/
#define TCFG_LOWPOWER_POWER_SEL				PWR_LDO15                    //电源模式设置，可选DCDC和LDO
#define TCFG_LOWPOWER_BTOSC_DISABLE			0                            //低功耗模式下BTOSC是否保持
#define TCFG_LOWPOWER_LOWPOWER_SEL			SLEEP_EN                  //SNIFF状态下芯片是否进入powerdown
/*mvddiom: 2.0~3.4*/
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_32V
/*wvddiom: 2.0~3.4*/
#define TCFG_LOWPOWER_VDDIOW_LEVEL			VDDIOW_VOL_28V               //弱VDDIO等级配置
#define TCFG_KEEP_FLASH_POWER_GATE          0							 //flash power由用户控制
#define TCFG_LOWPOWER_DAC_OPEN				1							 //出低功耗是否开dac电源，对于频繁进出低功耗应用建议在dac使用前开dac电源
#define TCFG_NORFLASH_4BYTE_MODE			0							 //出低功耗是否重新初始化4byte模式flash，大于16M的flash不打开此定义vm会报错



const struct low_power_param power_param = {
    .config         = TCFG_LOWPOWER_LOWPOWER_SEL,          //0：sniff时芯片不进入低功耗  1：sniff时芯片进入powerdown
    .btosc_hz         = 16000000,                   	   //外接晶振频率
    .delay_us       = 16000000 / 1000000L,        		   //提供给低功耗模块的延时(不需要需修改)
    .btosc_disable  = TCFG_LOWPOWER_BTOSC_DISABLE,         //进入低功耗时BTOSC是否保持
    .vddiom_lev     = TCFG_LOWPOWER_VDDIOM_LEVEL,          //强VDDIO等级,可选：2.0V  2.2V  2.4V  2.6V  2.8V  3.0V  3.2V  3.6V
    .vddiow_lev     = TCFG_LOWPOWER_VDDIOW_LEVEL,          //弱VDDIO等级,可选：2.1V  2.4V  2.8V  3.2V
    .osc_type       = OSC_TYPE_LRC,
    .flash_pg       = TCFG_KEEP_FLASH_POWER_GATE,
    .vdc13_cap_en   = 0,									//根据vdc13引脚是否有外部电容来配置, 1.外挂电容 0.无外挂电容
};

/**************************************************** wk_param *********************************************************/

const struct port_wakeup port0 = {
    .pullup_down_enable = ENABLE,                          //配置I/O 内部上下拉是否使能
    .edge       = FALLING_EDGE,                            //唤醒方式选择,可选：上升沿\下降沿
    .attribute  = BLUETOOTH_RESUME,                        //保留参数
    .iomap      = POWER_WAKEUP_IO,                             //唤醒口选择
};

const struct sub_wakeup sub_wkup = {
    .attribute  = BLUETOOTH_RESUME,
};

const struct charge_wakeup charge_wkup = {
    .attribute  = BLUETOOTH_RESUME,
};

const struct wakeup_param wk_param = {
    .port[1] = &port0,
    .sub = &sub_wkup,
    .charge = &charge_wkup,
};

/******************************************************* rs_param ***************************************************/
const struct reset_param rs_param = {
    .en = 0,
    .mode = 1,										//系统释放方式 0：等复位源翻转再释放 1：立刻释放
    .level = 0,										//有效电平选择 0：低电平有效		 1：高电平有效
    .iomap = IO_PORTA_01,
    .hold_time = LONG_4S_RESET,
};

/*#include "port_init.c"*/
void mask_io_cfg();
void close_gpio(u8 soft_off);

/**
 * @brief power_wakeup_reason
 * @note 1、该接口需要在系统初始化sys_power_init()前调用，否则获取的唤醒源不准确
 *		 2、可在sys_power_init()前用全局变量保存
 * @return 唤醒端口(对应wakeup_param中所设置的port)
 * 		   非边沿唤醒时返回-1
 **/
int power_wakeup_reason(void)
{
    u8 wkup_src = p33_rx_1byte(P3_WKUP_SRC);
    int wkup_port = -1;

    if (wkup_src & BIT(P3_WKUP_SRC_EDGE)) {
        u16 wkup_pnd = p33_rx_1byte(P3_WKUP_PND0) | (p33_rx_1byte(P3_WKUP_PND1) << 8);
        for (int i = 0; i < MAX_WAKEUP_PORT; i++) {
            if (wkup_pnd & BIT(i)) {
                wkup_port = i;
            }
        }
    }
    return wkup_port;
}

static u8 dac_state = 1;
void dac_power_off();
void dac_power_on();
void norflash_check_4byte_mode(void);
__attribute__((weak))
void dac_power_off()
{

}

__attribute__((weak))
void dac_power_on()
{
    //audio_init();
    //dac_init_api(32000);
}

void sleep_exit_callback(u32 usec)
{
    putchar('>');
}

void sleep_enter_callback(u8  step)
{
    /* 此函数禁止添加打印 */
    if (step == 1) {
        putchar('<');
        dac_power_off();
        dac_state = 0;
    } else {
        close_gpio(0);
    }
}

void board_set_soft_poweroff(void)
{
    close_gpio(1);

    dac_power_off();
}

extern u8 sys_low_power_request;
extern u32 lowpower_usec;

/*----------------------------------------------------------------------------*/
/**@brief 进入powerdown 模式
   @param usec: -2:静态睡眠，需要等待按键唤醒  非-2：睡眠时间，单位us，例如1000000为睡眠1S后自动唤醒
   @return null
   @note
*/
void sys_power_down(u32 usec)
{
    u8 temp_wdt_con = 0;
    u8 ret = 0;
    OS_ENTER_CRITICAL();
    if (!sys_low_power_request) {
        temp_wdt_con = wdt_rx_con();
        if (usec == (u32) - 2) {
            wdt_close();
        }
        lowpower_usec = usec;
        ret = low_power_sys_request(NULL);
        wdt_tx_con(temp_wdt_con);
        wdt_clear();
    }

    if (ret == 0) {
        OS_EXIT_CRITICAL();
        return;
    }

#if TCFG_NORFLASH_4BYTE_MODE
    norflash_check_4byte_mode();
#endif
    OS_EXIT_CRITICAL();

#if TCFG_LOWPOWER_DAC_OPEN
    if (dac_state == 0) {
        dac_power_on();
    }
#endif

}

void sys_softoff()
{
    power_set_soft_poweroff();
}

__attribute__((weak))
void tick_timer_sleep_init(void)
{

}

AT_VOLATILE_RAM_CODE
void __lvd_irq_handler(void)
{
    VLVD_PND_CLR(1);
    putchar('$');

#if 0
    //Garentee Flash power drop below 0.4V
    spi_flash_port_unmount();

    spi_flash_power_release();

    chip_reset();
#endif


}

void p33_vlvd(u8 vlvd)
{
    u8 reg;

    reg = p33_rx_1byte(P3_VLVD_CON);
    reg &= ~(BIT(3) | BIT(4) | BIT(5));
    reg |= vlvd << 3;

    p33_tx_1byte(P3_VLVD_CON, reg);
}

AT_VOLATILE_RAM_CODE
void powerdown_io_reinit()
{

}

AT_VOLATILE_RAM_CODE
void softoff_io_reinit()
{

}

void sys_power_init()
{
    log_info("Power init : %s", __FILE__);

    power_init(&power_param);

    if (power_param.config & SLEEP_EN) {
        tick_timer_sleep_init();
    }

    power_set_callback(TCFG_LOWPOWER_LOWPOWER_SEL, sleep_enter_callback, sleep_exit_callback, board_set_soft_poweroff);

    power_keep_dacvdd_en(0);

    power_wakeup_init(&wk_param);
}




