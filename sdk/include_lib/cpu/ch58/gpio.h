#ifndef  __GPIO_H__
#define  __GPIO_H__

#include "typedef.h"
#include "clock.h"

//===================================================//
// uc03 Crossbar API
//===================================================//
enum PFI_TABLE {
    PFI_GP_ICH0 = ((u32)(&(JL_IMAP->FI_GP_ICH0))),
    PFI_GP_ICH1 = ((u32)(&(JL_IMAP->FI_GP_ICH1))),
    PFI_GP_ICH2 = ((u32)(&(JL_IMAP->FI_GP_ICH2))),
    PFI_GP_ICH3 = ((u32)(&(JL_IMAP->FI_GP_ICH3))),
    PFI_GP_ICH4 = ((u32)(&(JL_IMAP->FI_GP_ICH4))),
    PFI_GP_ICH5 = ((u32)(&(JL_IMAP->FI_GP_ICH5))),
    PFI_GP_ICH6 = ((u32)(&(JL_IMAP->FI_GP_ICH6))),
    PFI_GP_ICH7 = ((u32)(&(JL_IMAP->FI_GP_ICH7))),

    PFI_SPI1_CLK = ((u32)(&(JL_IMAP->FI_SPI1_CLK))),
    PFI_SPI1_DA0 = ((u32)(&(JL_IMAP->FI_SPI1_DA0))),
    PFI_SPI1_DA1 = ((u32)(&(JL_IMAP->FI_SPI1_DA1))),
    PFI_SPI1_DA2 = ((u32)(&(JL_IMAP->FI_SPI1_DA2))),
    PFI_SPI1_DA3 = ((u32)(&(JL_IMAP->FI_SPI1_DA3))),

    PFI_SPI2_CLK = ((u32)(&(JL_IMAP->FI_SPI2_CLK))),
    PFI_SPI2_DA0 = ((u32)(&(JL_IMAP->FI_SPI2_DA0))),
    PFI_SPI2_DA1 = ((u32)(&(JL_IMAP->FI_SPI2_DA1))),

    PFI_IIC0_SCL = ((u32)(&(JL_IMAP->FI_IIC0_SCL))),
    PFI_IIC0_SDA = ((u32)(&(JL_IMAP->FI_IIC0_SDA))),
    PFI_IIC1_SCL = ((u32)(&(JL_IMAP->FI_IIC1_SCL))),
    PFI_IIC1_SDA = ((u32)(&(JL_IMAP->FI_IIC1_SDA))),

    PFI_UART0_RX = ((u32)(&(JL_IMAP->FI_UART0_RX))),
    PFI_UART1_RX = ((u32)(&(JL_IMAP->FI_UART1_RX))),
    PFI_UART2_RX = ((u32)(&(JL_IMAP->FI_UART2_RX))),
    PFI_UART2_CTS = ((u32)(&(JL_IMAP->FI_UART2_CTS))),

    PFI_TOTAL = ((u32)(&(JL_IMAP->FI_TOTAL))),
};



#define IO_GROUP_NUM 		16

#define IO_PORTA_00 				(IO_GROUP_NUM * 0 + 0)
#define IO_PORTA_01 				(IO_GROUP_NUM * 0 + 1)
#define IO_PORTA_02 				(IO_GROUP_NUM * 0 + 2)
#define IO_PORTA_03 				(IO_GROUP_NUM * 0 + 3)
#define IO_PORTA_04 				(IO_GROUP_NUM * 0 + 4)
#define IO_PORTA_05 				(IO_GROUP_NUM * 0 + 5)
#define IO_PORTA_06 				(IO_GROUP_NUM * 0 + 6)
#define IO_PORTA_07 				(IO_GROUP_NUM * 0 + 7)
#define IO_PORTA_08 				(IO_GROUP_NUM * 0 + 8)
#define IO_PORTA_09 				(IO_GROUP_NUM * 0 + 9)
#define IO_PORTA_10 				(IO_GROUP_NUM * 0 + 10)
#define IO_PORTA_11 				(IO_GROUP_NUM * 0 + 11)
#define IO_PORTA_12 				(IO_GROUP_NUM * 0 + 12)

// #define IO_PORTB_00 				(IO_GROUP_NUM * 1 + 0)
// #define IO_PORTB_01 				(IO_GROUP_NUM * 1 + 1)
// #define IO_PORTB_02 				(IO_GROUP_NUM * 1 + 2)
// #define IO_PORTB_03 				(IO_GROUP_NUM * 1 + 3)
// #define IO_PORTB_04 				(IO_GROUP_NUM * 1 + 4)
// #define IO_PORTB_05 				(IO_GROUP_NUM * 1 + 5)
// #define IO_PORTB_06 				(IO_GROUP_NUM * 1 + 6)
// #define IO_PORTB_07 				(IO_GROUP_NUM * 1 + 7)
// #define IO_PORTB_08 				(IO_GROUP_NUM * 1 + 8)
// #define IO_PORTB_09 				(IO_GROUP_NUM * 1 + 9)
// #define IO_PORTB_10 				(IO_GROUP_NUM * 1 + 10)

#define IO_PORTC_00 				(IO_GROUP_NUM * 2 + 0)
#define IO_PORTC_01 				(IO_GROUP_NUM * 2 + 1)
#define IO_PORTC_02 				(IO_GROUP_NUM * 2 + 2)
#define IO_PORTC_03 				(IO_GROUP_NUM * 2 + 3)
#define IO_PORTC_04 				(IO_GROUP_NUM * 2 + 4)
#define IO_PORTC_05 				(IO_GROUP_NUM * 2 + 5)
#define IO_PORTC_06 				(IO_GROUP_NUM * 2 + 6)
#define IO_PORTC_07 				(IO_GROUP_NUM * 2 + 7)
#define IO_PORTC_08 				(IO_GROUP_NUM * 2 + 8)
#define IO_PORTC_09 				(IO_GROUP_NUM * 2 + 9)
#define IO_PORTC_10 				(IO_GROUP_NUM * 2 + 10)
#define IO_PORTC_11 				(IO_GROUP_NUM * 2 + 11)
#define IO_PORTC_12 				(IO_GROUP_NUM * 2 + 12)
#define IO_PORTC_13 				(IO_GROUP_NUM * 2 + 13)
#define IO_PORTC_14 				(IO_GROUP_NUM * 2 + 14)
#define IO_PORTC_15 				(IO_GROUP_NUM * 2 + 15)

#define IO_PORTD_00 				(IO_GROUP_NUM * 3 + 0)
#define IO_PORTD_01 				(IO_GROUP_NUM * 3 + 1)
#define IO_PORTD_02 				(IO_GROUP_NUM * 3 + 2)
#define IO_PORTD_03 				(IO_GROUP_NUM * 3 + 3)
#define IO_PORTD_04 				(IO_GROUP_NUM * 3 + 4)
#define IO_PORTD_05 				(IO_GROUP_NUM * 3 + 5)
#define IO_PORTD_06 				(IO_GROUP_NUM * 3 + 6)
#define IO_PORTD_07 				(IO_GROUP_NUM * 3 + 7)
#define IO_PORTD_08 				(IO_GROUP_NUM * 3 + 8)
#define IO_PORTD_09 				(IO_GROUP_NUM * 3 + 9)
#define IO_PORTD_10 				(IO_GROUP_NUM * 3 + 10)
#define IO_PORTD_11 				(IO_GROUP_NUM * 3 + 11)
#define IO_PORTD_12 				(IO_GROUP_NUM * 3 + 12)
#define IO_PORTD_13 				(IO_GROUP_NUM * 3 + 13)
#define IO_PORTD_14 				(IO_GROUP_NUM * 3 + 14)
#define IO_PORTD_15 				(IO_GROUP_NUM * 3 + 15)

#define IO_PORTF_00 				(IO_GROUP_NUM * 4 + 0)
#define IO_PORTF_01 				(IO_GROUP_NUM * 4 + 1)
#define IO_PORTF_02 				(IO_GROUP_NUM * 4 + 2)
#define IO_PORTF_03 				(IO_GROUP_NUM * 4 + 3)

// #define IO_PORTP_00 				(IO_GROUP_NUM * 4 + 0)

#define IO_MAX_NUM 					(IO_PORTF_03 + 1)

#define IO_PORT_DP                  (IO_GROUP_NUM * 5 + 0)
#define IO_PORT_DM                  (IO_GROUP_NUM * 5 + 1)

#define P33_IO_OFFSET               0
// #define IO_CHGFL_DET                (IO_MAX_NUM + P33_IO_OFFSET + 0)
// #define IO_VBTDT_DET                (IO_MAX_NUM + P33_IO_OFFSET + 1)
// #define IO_VBTCH_DET                (IO_MAX_NUM + P33_IO_OFFSET + 2)
// #define IO_LDOIN_DET                (IO_MAX_NUM + P33_IO_OFFSET + 3)

// #define IO_PORT_MAX					(IO_LDOIN_DET + 1)
#define IO_PORT_MAX					(IO_PORT_DM + 1)


#define GPIOA                       (IO_GROUP_NUM * 0)
// #define GPIOB                       (IO_GROUP_NUM * 1)
#define GPIOC                       (IO_GROUP_NUM * 2)
#define GPIOD                       (IO_GROUP_NUM * 3)
#define GPIOF                       (IO_GROUP_NUM * 4)
#define GPIOUSB                     (IO_GROUP_NUM * 5)
#define GPIOP33                     (IO_MAX_NUM + P33_IO_OFFSET)

// enum CLK_OUT_SOURCE {
// NONE_CLK_OUT,
// RTC_OSC_CLK_OUT,
// LRC_CLK_OUT,
// STD_12M_CLK_OUT,
// STD_24M_CLK_OUT,
// STD_48M_CLK_OUT,
// HSB_CLK_OUT,
// LSB_CLK_OUT,
// PLL_96M_CLK_OUT,
// RC_250K_CLK_OUT,
// RC_16M_CLK_OUT,
// USB_CLK_OUT,
// };

// enum CLK_OUT_SOURCE_2 {
// NONE_CLK_OUT_2,
// RTC_OSC_CLK_OUT_2,
// LRC_CLK_OUT_2,
// SYS_PLL_D3P5_CLK_OUT_2,
// SYS_PLL_D2P5_CLK_OUT_2,
// SYS_PLL_D2P0_CLK_OUT_2,
// SYS_PLL_D1P5_CLK_OUT_2,
// SYS_PLL_D1P0_CLK_OUT_2,
// };

#if 0
enum OUTPUT_CH_SIGNAL {
    OUTPUT_CH_SIGNAL_TIMER0_PWM,
    OUTPUT_CH_SIGNAL_TIMER1_PWM,
    OUTPUT_CH_SIGNAL_TIMER2_PWM,
    OUTPUT_CH_SIGNAL_CLOCK_OUT0,
    OUTPUT_CH_SIGNAL_CLOCK_OUT1,
    OUTPUT_CH_SIGNAL_CLOCK_OUT2,
    OUTPUT_CH_SIGNAL_CLOCK_OUT3,
    OUTPUT_CH_SIGNAL_GP_ICH6,
    OUTPUT_CH_SIGNAL_GP_ICH7,
    OUTPUT_CH_SIGNAL_GP_LEDC,
    OUTPUT_CH_SIGNAL_P11_DBG_OUT,
    OUTPUT_CH_SIGNAL_P33_DBG_OUT,
};
enum INPUT_CH_SIGNAL {
//ICH_CON0:
    INPUT_CH_SIGNAL_TIMER0_CIN = 0, //bit0~3
    INPUT_CH_SIGNAL_TIMER0_CAP,  //bit4~7
    INPUT_CH_SIGNAL_TIMER1_CIN,
    INPUT_CH_SIGNAL_TIMER1_CAP,
    INPUT_CH_SIGNAL_TIMER2_CIN,
    INPUT_CH_SIGNAL_TIMER2_CAP,
    INPUT_CH_SIGNAL_WKUP,
    INPUT_CH_SIGNAL_IRFLT,
//ICH_CON1:
    INPUT_CH_SIGNAL_CAP = 8,
    INPUT_CH_SIGNAL_CLK_PIN,
};

enum INPUT_CH_TYPE {
    INPUT_CH_TYPE_GP_ICH = 0,
    INPUT_CH_TYPE_TMR0_PWM = 8,
    INPUT_CH_TYPE_TMR1_PWM,
};
#else
#define  INPUT_GP_ICH_MAX  8
#define  OUTPUT_GP_OCH_MAX 8

enum GPIO_INPUT_FUN {
    // ICH0 同时输入到4个信号
    GPIO_INPUT_ICH0_TIMER0_CLK = 0X0000,
    GPIO_INPUT_ICH0_WAKEUP0 = 0X0001,
    GPIO_INPUT_ICH0_PLL_EXT_CLK = 0X0002,
    GPIO_INPUT_ICH0_QDEC_PHASE_A = 0X0003,

    GPIO_INPUT_ICH1_TIMER0_CAP = 0X0100,
    GPIO_INPUT_ICH1_WAKEUP1 = 0X0101,

    GPIO_INPUT_ICH2_TIMER1_CLK = 0X0200,
    GPIO_INPUT_ICH2_WAKEUP2 = 0X0201,
    GPIO_INPUT_ICH2_IRFLT = 0X0202,
    GPIO_INPUT_ICH2_ASS_DBG_DI0 = 0X0203,

    GPIO_INPUT_ICH3_TIMER1_CAP = 0X0300,
    GPIO_INPUT_ICH3_WAKEUP3 = 0X0301,
    GPIO_INPUT_ICH3_USB_DBG = 0X0302,

    GPIO_INPUT_ICH4_TIMER2_CLK = 0X0400,
    GPIO_INPUT_ICH4_WAKEUP4 = 0X0401,
    GPIO_INPUT_ICH4_CLK_MUX = 0X0402,
    GPIO_INPUT_ICH4_QDEC_PHASE_B = 0X0403,

    GPIO_INPUT_ICH5_TIMER2_CAP = 0X0500,
    GPIO_INPUT_ICH5_WAKEUP5 = 0X0501,

    GPIO_INPUT_ICH6_TIMER3_CLK = 0X0600,
    GPIO_INPUT_ICH6_WAKEUP6 = 0X0601,
    GPIO_INPUT_ICH6_TOUCH_CAP = 0X0602,
    GPIO_INPUT_ICH6_ASS_DBG_DI1 = 0X0603,

    GPIO_INPUT_ICH7_TIMER3_CAP = 0X0700,
    GPIO_INPUT_ICH7_WAKEUP7 = 0X0701,
};

enum GPIO_OUTPUT_FUN {
    //COMPx 只能输出一个信号
    GPIO_OUT_COMP0_MC_PWM0_H = 0X0000,
    GPIO_OUT_COMP0_PAP_WE = 0X0004,
    GPIO_OUT_COMP0_CLK_OUT0 = 0X0005,
    // GPIO_OUT_COMP0_CLK_OUT1 = 0X0006,
    // GPIO_OUT_COMP0_CLK_OUT2 = 0X0007,

    GPIO_OUT_COMP1_MC_PWM0_L = 0X0101,
    GPIO_OUT_COMP1_PAP_RE = 0X0104,
    // GPIO_OUT_COMP1_CLK_OUT0 = 0X0105,
    GPIO_OUT_COMP1_CLK_OUT1 = 0X0106,
    // GPIO_OUT_COMP1_CLK_OUT2 = 0X0107,

    GPIO_OUT_COMP2_MC_PWM1_H = 0X0202,
    GPIO_OUT_COMP2_CLK_OUT2 = 0X0205,
    GPIO_OUT_COMP2_P33_SIG_DBG0 = 0X0206,
    // GPIO_OUT_COMP2_APA_POUT = 0X0207,


    GPIO_OUT_COMP3_MC_PWM1_L = 0X0303,
    GPIO_OUT_COMP3_P33_SIG_DBG1 = 0X0306,
    // GPIO_OUT_COMP3_APA_NOUT = 0X0307,

    GPIO_OUT_COMP4_TIMER0_PWM = 0x0400,
    GPIO_OUT_COMP4_MC_PWM2_H = 0X0404,

    GPIO_OUT_COMP5_TIMER1_PWM = 0x0501,
    GPIO_OUT_COMP5_MC_PWM2_L = 0X0505,

    GPIO_OUT_COMP6_TIMER2_PWM = 0x0602,
    GPIO_OUT_COMP6_ICH0 = 0X0604,
    GPIO_OUT_COMP6_USB_DBG_OUT = 0X0605,

    GPIO_OUT_COMP7_TIMER3_PWM = 0x0703,
    GPIO_OUT_COMP7_ICH1 = 0X0704,
    GPIO_OUT_COMP7_P33_CLK_DBG = 0X0705,
};

int gpio_mux_in(u32 gpio, enum GPIO_INPUT_FUN fun);
u32 gpio_mux_in_close(u32 fd);
u32 gpio_mux_out(u32 gpio, enum GPIO_OUTPUT_FUN fun);
u32 gpio_mux_out_close(u32 gpio, u32 fd);

u32 gpio_get_ich_use_flag();
//获取空闲的gp_ich
//return: 0xff:error
u8 gpio_get_unoccupied_gp_ich();
//value:gp_ich序号
void gpio_release_gp_ich(u8 value);

#endif

enum gpio_op_mode {
    GPIO_SET = 1,
    GPIO_AND,
    GPIO_OR,
    GPIO_XOR,
};

enum gpio_direction {
    GPIO_OUT = 0,
    GPIO_IN = 1,
};

enum gpio_pullup_mode {
    GPIO_PULLUP_DISABLE,
    GPIO_PULLUP_10K,
    GPIO_PULLUP_100K,
    GPIO_PULLUP_1M,
};

enum gpio_pulldown_mode {
    GPIO_PULLDOWN_DISABLE,
    GPIO_PULLDOWN_10K,
    GPIO_PULLDOWN_100K,
    GPIO_PULLDOWN_1M,
};

enum gpio_hd_mode {
    GPIO_HD_2p4mA,		/* 最大驱动电流  2.4mA */
    GPIO_HD_8p0mA,		/* 最大驱动电流  8.0mA */
    GPIO_HD_24p0mA,		/* 最大驱动电流  24mA  */
    GPIO_HD_64p0mA,		/* 最大驱动电流 64.0mA */
};

struct gpio_reg {
    volatile unsigned int in;
    volatile unsigned int out;
    volatile unsigned int dir;
    volatile unsigned int die;
    volatile unsigned int dieh;
    volatile unsigned int pu0;
    volatile unsigned int pu1;
    volatile unsigned int pd0;
    volatile unsigned int pd1;
    volatile unsigned int hd0;
    volatile unsigned int hd1;
    volatile unsigned int spl;
    volatile unsigned int con; // usb phy only
};

/**
 * @brief usb_iomode, usb引脚设为普通IO
 *
 * @param enable 1，使能；0，关闭
 */
void usb_iomode(u32 enable);

/**
 * @brief gpio2reg，获取引脚序号对应的端口寄存器地址
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 *
 * @return 端口寄存器地址，如JL_PORTA,JL_PORTB,,,
 */
struct gpio_reg *gpio2reg(u32 gpio);

/**
 * @brief gpio_set_direction，设置引脚方向
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param value 1，输入；0，输出
 *
 * @return 0：成功  非0：失败
 */
u32 gpio_set_direction(u32 gpio, u32 value);

/**
 * @brief gpio_read  &  gpio_read_input_value，读取引脚的输入电平，引脚为输入模式时才有效
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 *
 * @return 电平值
 */
int gpio_read(u32 gpio);
int gpio_read_input_value(u32 gpio);

/**
 * @brief gpio_direction_input，将引脚直接设为输入模式
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 *
 * @return 0：成功  非0：失败
 */
u32 gpio_direction_input(u32 gpio);

/**
 * @brief gpio_write  &  gpio_set_output_value，设置引脚输出的电平，引脚为输出模式时才有效
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param value 1，输出1,  0，输出0
 *
 * @return 0：成功  非0：失败
 */
u32 gpio_write(u32 gpio, u32 value);
u32 gpio_set_output_value(u32 gpio, u32 value);

/**
 * @brief gpio_direction_output，设置引脚的方向为输出，并设置一下电平
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param value 1，输出1；0，输出0
 *
 * @return 0：成功  非0：失败
 */
u32 gpio_direction_output(u32 gpio, u32 value);

/**
 * @brief gpio_set_pull_up，设置引脚的上拉，上拉电阻10K，当引脚为输入模式时才有效
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param mode见枚举
 *
 * @return 0：成功  非0：失败
 */
u32 gpio_set_pull_up(u32 gpio, enum gpio_pullup_mode mode);

/**
 * @brief gpio_set_pull_down，设置引脚的下拉，下拉电阻10K，当引脚为输入模式时才有效
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param mode 见枚举
 *
 * @return 0：成功  非0：失败
 */
u32 gpio_set_pull_down(u32 gpio, enum gpio_pulldown_mode mode);

/**
 * @brief gpio_set_hd0, 设置引脚的内阻，当引脚为输出模式时才有效
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param value 1，关闭内阻增强输出  0，存在内阻，芯片默认的
 *
 * @return 0：成功  非0：失败
 */
// u32 gpio_set_hd0(u32 gpio, u32 value);

/**
 * @brief gpio_set_hd, 设置引脚的输出电流，当引脚为输出模式时才有效
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param mode 见枚举
 *
 * @return 0：成功  非0：失败
 */
u32 gpio_set_hd(u32 gpio, enum gpio_hd_mode mode);

/**
 * @brief gpio_set_die，设置引脚为数字功能还是模拟功能，比如引脚作为ADC的模拟输入，则die要置0
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param value 1，数字功能，即01信号；  0，跟电压先关的模拟功能
 *
 * @return 0：成功  非0：失败
 */
u32 gpio_set_die(u32 gpio, u32 value);

/**
 * @brief gpio_set_dieh, 设置引脚为数字功能还是模拟功能，但模拟功能时，跟die的不一样而已
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param value 1，数字功能，即01信号；  0，跟电压先关的模拟功能
 *
 * @return 0：成功  非0：失败
 */
u32 gpio_set_dieh(u32 gpio, u32 value);

//用于硬件iic
u32 gpio_set_spl(u32 gpio, u32 value);

/**
 * @brief get_gpio, 解析字符串，获取引脚对应的序号
 *
 * @param p 引脚字符串，如“PA00”, "PA01"，，，，
 *
 * @return 引脚对应的序号. 如传入“PB01”，返回17
 */
u32 get_gpio(const char *p);

const char *gpio_get_name(u32 gpio);


//=================================================================================//
//@brief: CrossBar 获取某IO的输出映射寄存器
//@input:
// 		gpio: 需要输出外设信号的IO口; 如IO_PORTA_00
//@return:
// 		输出映射寄存器地址; 如&(JL_OMAP->PA0_OUT)
//=================================================================================//
u32 *gpio2crossbar_outreg(u32 gpio);

//=================================================================================//
//@brief: CrossBar 获取某IO的输入映射序号
//@input:
// 		gpio: 需要输出外设信号的IO口; 如IO_PORTA_00
//@return:
// 		输出映射序号; 如PA0_IN
//=================================================================================//
u32 gpio2crossbar_inport(u32 gpio);

//=================================================================================//
//@brief: CrossBar 输出设置 API, 将指定IO口设置为某个外设的输出
//@input:
// 		gpio: 需要输出外设信号的IO口;
// 		fun_index: 需要输出到指定IO口的外设信号, 可以输出外设信号列表请查看io_omap.h文件;
// 		dir_ctl: IO口方向由外设控制使能, 常设为1;
// 		data_ctl: IO口电平状态由外设控制使能, 常设为1;
//@return:
// 		1)0: 执行正确;
//		2)-EINVAL: 传参出错;
//@note: 所映射的IO需要在设置IO状态为输出配置;
//@example: 将UART0的Tx信号输出到IO_PORTA_05口:
// 			gpio_direction_output(IO_PORTA_05, 1); //设置IO为输出状态
//			gpio_set_fun_output_port(IO_PORTA_05, FO_UART0_TX, 1, 1); //将UART0的Tx信号输出到IO_PORTA_05口
//=================================================================================//
u32 gpio_set_fun_output_port(u32 gpio, u32 fun_index, u8 dir_ctl, u8 data_ctl);

//=================================================================================//
//@brief: CrossBar 输出设置 API, 将指定IO释放外设控制, 变为普通IO;
//@input:
// 		gpio: 需要释放外设控制IO口, 释放后变为普通IO模式;
//@return:
// 		1)0: 执行正确;
//		2)-EINVAL: 传参出错;
//@note:
//@example: 将IO_PORTA_05口被某一外设控制状态释放:
// 			gpio_disable_fun_output_port(IO_PORTA_05);
//=================================================================================//
u32 gpio_disable_fun_output_port(u32 gpio);

//=================================================================================//
//@brief: CrossBar 输入设置 API, 将某个外设的输入设置为从某个IO输入
//@input:
// 		gpio: 需要输入外设信号的IO口;
// 		pfun: 需要从指定IO输入的外设信号, 可以输入的外设信号列表请查看gpio.h文件enum PFI_TABLE枚举项;
//@return:
// 		1)0: 执行正确;
//		2)-EINVAL: 传参出错;
//@note: 所映射的IO需要在设置IO状态为输入配置;
//@example: 将UART0的Rx信号设置为IO_PORTA_05口输入:
//			gpio_set_die(data->rx_pin, 1); 		//数字输入使能
//			gpio_set_pull_up(data->rx_pin, 1);  //上拉输入使能
//			gpio_direction_input(IO_PORTA_05);  //设置IO为输入状态
//			gpio_set_fun_input_port(IO_PORTA_05, PFI_UART0_RX); //将UART0的Rx信号设置为IO_PORTA_05口输入
//=================================================================================//
u32 gpio_set_fun_input_port(u32 gpio, enum PFI_TABLE pfun);

//=================================================================================//
//@brief: CrossBar 输入设置 API, 将某个外设信号释放IO口控制, 变为普通IO;
//@input:
// 		pfun: 需要释放由某个IO口输入的外设信号, 外设信号列表请查看gpio.h文件enum PFI_TABLE枚举项;
//@return:  默认为0, 无出错处理;
//@note:
//@example: 将外设信号PFI_UART0_RX释放由某个IO输入:
// 			gpio_disable_fun_input_port(PFI_UART0_RX);
//=================================================================================//
u32 gpio_disable_fun_input_port(enum PFI_TABLE pfun);

// //return:och 号
// int gpio_och_sel_output_signal(u32 gpio, enum OUTPUT_CH_SIGNAL signal);
// //return:无效
// int gpio_och_disable_output_signal(u32 gpio, enum OUTPUT_CH_SIGNAL signal);
// //return:ich 号
// int gpio_ich_sel_input_signal(u32 gpio, enum INPUT_CH_SIGNAL signal, enum INPUT_CH_TYPE type);
// //return:ich 号
// int gpio_ich_disable_input_signal(u32 gpio, enum INPUT_CH_SIGNAL signal, enum INPUT_CH_TYPE type);

void gpio_clk_out0(u8 gpio, enum CLK_OUT_SOURCE clk);
void gpio_clk_out1(u8 gpio, enum CLK_OUT_SOURCE clk);
void gpio_clk_out2(u8 gpio, enum CLK_OUT_SOURCE_2 clk, u8 div);//pa1

u32 gpio_die(u32 gpio, u32 start, u32 len, u32 dat, enum gpio_op_mode op);

u32 gpio_dieh(u32 gpio, u32 start, u32 len, u32 dat, enum gpio_op_mode op);

#endif  /*GPIO_H*/

