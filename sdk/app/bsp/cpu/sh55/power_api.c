#include "power_api.h"
#include "asm/power_interface.h"
#include "asm/power/p33.h"
#include "gpio.h"
#include "tick_timer_driver.h"
/* #include "audio.h" */

#define ENABLE								1
#define DISABLE								0

#define LOG_TAG_CONST       PWRA
#define LOG_TAG             "[pwra]"
#include "debug.h"

//*********************************************************************************//
//                                  低功耗配置                                     //
//*********************************************************************************//
#define TCFG_LOWPOWER_POWER_SEL				PWR_LDO15                    //电源模式设置，可选DCDC和LDO
#define TCFG_LOWPOWER_BTOSC_DISABLE			0                            //低功耗模式下BTOSC是否保持
#define TCFG_LOWPOWER_LOWPOWER_SEL			SLEEP_EN                  //SNIFF状态下芯片是否进入powerdown
/*mvddiom: 2.0~3.4*/
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_32V
/*wvddiom: 2.0~3.4*/
#define TCFG_LOWPOWER_VDDIOW_LEVEL			VDDIOW_VOL_28V               //弱VDDIO等级配置
#define TCFG_KEEP_FLASH_POWER_GATE          0							 //flash power由软件控制


/************************** LOW POWER config ****************************/
const struct low_power_param power_param = {
    .config         = TCFG_LOWPOWER_LOWPOWER_SEL,          //0：sniff时芯片不进入低功耗  1：sniff时芯片进入powerdown
    .btosc_hz         = 16000000,                   	   //外接晶振频率
    .delay_us       = 16000000 / 1000000L,        		   //提供给低功耗模块的延时(不需要需修改)
    .btosc_disable  = TCFG_LOWPOWER_BTOSC_DISABLE,         //进入低功耗时BTOSC是否保持
    .vddiom_lev     = TCFG_LOWPOWER_VDDIOM_LEVEL,          //强VDDIO等级,可选：2.0V  2.2V  2.4V  2.6V  2.8V  3.0V  3.2V  3.6V
    .vddiow_lev     = TCFG_LOWPOWER_VDDIOW_LEVEL,          //弱VDDIO等级,可选：2.1V  2.4V  2.8V  3.2V
    .osc_type       = OSC_TYPE_LRC,
    .flash_pg       = TCFG_KEEP_FLASH_POWER_GATE,
    .vdc13_cap_en   = 1,									//根据vdc13引脚是否有外部电容来配置, 1.外挂电容 0.无外挂电容
};

/************************** PWR config ****************************/

const struct port_wakeup port0 = {
    .pullup_down_enable = ENABLE,                          //配置I/O 内部上下拉是否使能
    .edge       = FALLING_EDGE,                            //唤醒方式选择,可选：上升沿\下降沿
    .attribute  = BLUETOOTH_RESUME,                        //保留参数
    .iomap      = IO_PORTA_00,                             //唤醒口选择
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

/************************** PWR long hold reset config ****************************/
const struct reset_param rs_param = {
    .en = 0,
    .mode = 1,										//系统释放方式 0：等复位源翻转再释放 1：立刻释放
    .level = 0,										//有效电平选择 0：低电平有效		 1：高电平有效
    .iomap = IO_PORTA_01,
    .hold_time = LONG_4S_RESET,
};

__attribute__((weak))
void dac_power_off()
{

}

__attribute__((weak))
void tick_timer_sleep_init(void)
{
}

static void mask_io_cfg()
{
    struct boot_soft_flag_t boot_soft_flag = {0};

    boot_soft_flag.flag0.boot_ctrl.wdt_dis = 0;
    boot_soft_flag.flag0.boot_ctrl.lvd_en = GET_P33_VLVD_EN();

    mask_softflag_config(&boot_soft_flag);
}

/*进软关机之前默认将IO口都设置成高阻状态，需要保留原来状态的请修改该函数*/
extern void dac_power_off(void);
extern void dac_sniff_power_off(void);
extern u32 spi_get_port(void);
void board_set_soft_poweroff(void)
{
    u32 porta_value = 0xffff & ~(BIT(0));
    u32 portb_value = 0xffff & ~(BIT(10));

    mask_io_cfg();

    /*gpio_write(MIC_HW_IO, 0);*/

    JL_PORTA->DIR |= porta_value;
    JL_PORTA->PU &= ~(porta_value);
    JL_PORTA->PD &= ~(porta_value);
    JL_PORTA->DIE &= ~(porta_value);
    JL_PORTA->DIEH &= ~(porta_value);

    //PB1:长按复位
    JL_PORTB->DIR |= portb_value;
    JL_PORTB->PU &= ~(portb_value);
    JL_PORTB->PD &= ~(portb_value);
    JL_PORTB->DIE &= ~(portb_value);
    JL_PORTB->DIEH &= ~(portb_value);

    VDDIOW_VOL_SEL(power_param.vddiow_lev);

    dac_power_off();
}


#define     APP_IO_DEBUG_0(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}
#define     APP_IO_DEBUG_1(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}

void sleep_exit_callback(u32 usec)
{
    putchar('>');
    APP_IO_DEBUG_1(A, 6);
}

void sleep_enter_callback(u8  step)
{
    /* 此函数禁止添加打印 */
    if (step == 1) {
        putchar('<');
        APP_IO_DEBUG_0(A, 6);
        /*dac_sniff_power_off();*/
        dac_power_off();
    } else {

        u32 porta_value = 0xffff & ~(BIT(0));
        u32 portb_value = 0xffff & ~BIT(10);
        u32 portd_value = 0x1f;

        if (spi_get_port() == 0) {
            portd_value = 0;
        }

        /*gpio_write(MIC_HW_IO, 0);*/

        JL_PORTA->DIR |= porta_value;
        JL_PORTA->PU &= ~(porta_value);
        JL_PORTA->PD &= ~(porta_value);
        JL_PORTA->DIE &= ~(porta_value);
        JL_PORTA->DIEH &= ~(porta_value);

        //PB1:长按复位
        JL_PORTB->DIR |= portb_value;
        JL_PORTB->PU &= ~(portb_value);
        JL_PORTB->PD &= ~(portb_value);
        JL_PORTB->DIE &= ~(portb_value);
        JL_PORTB->DIEH &= ~(portb_value);

        JL_PORTD->DIR |= portd_value;
        JL_PORTD->PU &= ~(portd_value);
        JL_PORTD->PD &= ~(portd_value);
        JL_PORTD->DIE &= ~(portd_value);
        JL_PORTD->DIEH &= ~(portd_value);
    }
}

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

extern u8 sys_low_power_request;
extern u32 lowpower_usec;

void sys_power_down(u32 usec)
{
    if (sys_low_power_request) {
        lowpower_usec = usec;
        low_power_sys_request(NULL);
        wdt_clear();
    }
}

void sys_softoff()
{
    power_set_soft_poweroff();
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




