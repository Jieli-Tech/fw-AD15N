#ifndef  __GPIO_H__
#define  __GPIO_H__

#include "typedef.h"

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

    PFI_SPI0_CLK = ((u32)(&(JL_IMAP->FI_SPI0_CLK))),
    PFI_SPI0_DA0 = ((u32)(&(JL_IMAP->FI_SPI0_DA0))),
    PFI_SPI0_DA1 = ((u32)(&(JL_IMAP->FI_SPI0_DA1))),
    PFI_SPI0_DA2 = ((u32)(&(JL_IMAP->FI_SPI0_DA2))),
    PFI_SPI0_DA3 = ((u32)(&(JL_IMAP->FI_SPI0_DA3))),
    PFI_SPI1_CLK = ((u32)(&(JL_IMAP->FI_SPI1_CLK))),
    PFI_SPI1_DA0 = ((u32)(&(JL_IMAP->FI_SPI1_DA0))),
    PFI_SPI1_DA1 = ((u32)(&(JL_IMAP->FI_SPI1_DA1))),

    PFI_IIC0_SCL = ((u32)(&(JL_IMAP->FI_I2C0_SCL))),
    PFI_IIC0_SDA = ((u32)(&(JL_IMAP->FI_I2C0_SDA))),

    PFI_UART0_RX = ((u32)(&(JL_IMAP->FI_UART0_RX))),
    PFI_UART1_RX = ((u32)(&(JL_IMAP->FI_UART1_RX))),

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

#define IO_PORTB_00 				(IO_GROUP_NUM * 1 + 0)
#define IO_PORTB_01 				(IO_GROUP_NUM * 1 + 1)
#define IO_PORTB_02 				(IO_GROUP_NUM * 1 + 2)
#define IO_PORTB_03 				(IO_GROUP_NUM * 1 + 3)
#define IO_PORTD_00 				(IO_GROUP_NUM * 2 + 0)
#define IO_PORTD_01 				(IO_GROUP_NUM * 2 + 1)
#define IO_PORTD_02 				(IO_GROUP_NUM * 2 + 2)
#define IO_PORTD_03 				(IO_GROUP_NUM * 2 + 3)
#define IO_PORTD_04 				(IO_GROUP_NUM * 2 + 4)


#define IO_MAX_NUM 					(IO_PORTD_04 + 1)

// #define P33_IO_OFFSET               2
// #define IO_CHGFL_DET                (IO_MAX_NUM + P33_IO_OFFSET + 0)
// #define IO_VBTDT_DET                (IO_MAX_NUM + P33_IO_OFFSET + 1)
// #define IO_VBTCH_DET                (IO_MAX_NUM + P33_IO_OFFSET + 2)
// #define IO_LDOIN_DET                (IO_MAX_NUM + P33_IO_OFFSET + 3)

#define IO_PORT_MAX					(IO_PORTD_04 + 1)

#define GPIOA                       (IO_GROUP_NUM * 0)
#define GPIOB                       (IO_GROUP_NUM * 1)
#define GPIOD                       (IO_GROUP_NUM * 2)
// #define GPIOP33                     (IO_MAX_NUM + P33_IO_OFFSET)


enum CLK_OUT_SOURCE {
    CLK_OUT_DISABLE,//1
    CLK_OUT_LRC_200K,
    CLK_OUT_RC250K,
    CLK_OUT_RC16M,
    CLK_OUT_PLL_96M,
    CLK_OUT_STD_48M,
    CLK_OUT_STD_24M,
    CLK_OUT_STD_12M,
    CLK_OUT_SRC_CLK,
    CLK_OUT_HSB,
    CLK_OUT_LSB,
    CLK_OUT_USB_CLK,
    CLK_OUT_UART_CLK,
    CLK_OUT_EXT_CLK,
    CLK_OUT_PLL_SYNC_CLK,
};
enum CLK_OUT2_SOURCE {
    CLK_OUT2_DISABLE,
    CLK_OUT2_STD_12M,
    CLK_OUT2_STD_48M,
    CLK_OUT2_SYSPLL_D3P5,
    CLK_OUT2_SYSPLL_D2P5,
    CLK_OUT2_SYSPLL_D2P0,
    CLK_OUT2_SYSPLL_D1P5,
    CLK_OUT2_SYSPLL_D1P0,
};

enum OUTPUT_CH_SIGNAL {
    OUTPUT_CH_SIGNAL_TIMER1_PWM,//och0
    OUTPUT_CH_SIGNAL_TIMER2_PWM,//och1
    OUTPUT_CH_SIGNAL_UART1_RTS,
    OUTPUT_CH_SIGNAL_CLOCK_OUT0,
    OUTPUT_CH_SIGNAL_CLOCK_OUT1,
    OUTPUT_CH_SIGNAL_CLOCK_OUT2,
    OUTPUT_CH_SIGNAL_GP_ICH4,
    OUTPUT_CH_SIGNAL_GP_ICH5,
};
enum INPUT_CH_SIGNAL {
//ICH_CON0:
    INPUT_CH_SIGNAL_TIMER1_CIN, //bit0~3
    INPUT_CH_SIGNAL_TIMER1_CAP, //bit4~7
    INPUT_CH_SIGNAL_TIMER2_CIN,
    INPUT_CH_SIGNAL_TIMER2_CAP,
    INPUT_CH_SIGNAL_UART1_CTS,
    INPUT_CH_SIGNAL_CLK_PIN,
    INPUT_CH_SIGNAL_CAP,
    INPUT_CH_SIGNAL_IRFLT,
//ICH_CON1:
    INPUT_CH_SIGNAL_CLK_MUX = 8,
    INPUT_CH_SIGNAL_MC_PWM_FP0,
    INPUT_CH_SIGNAL_MC_PWM_FP1,
};

enum INPUT_CH_TYPE {
    INPUT_CH_TYPE_GP_ICH = 0,
    INPUT_CH_TYPE_TMR1_PWM = 8,
    INPUT_CH_TYPE_TMR2_PWM,
};
#define  INPUT_GP_ICH_MAX  8
#define  OUTPUT_GP_OCH_MAX 8

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

struct gpio_reg {
    volatile unsigned int in;
    volatile unsigned int out;
    volatile unsigned int dir;
    volatile unsigned int die;
    volatile unsigned int dieh;
    volatile unsigned int pu;
    volatile unsigned int pd;
    volatile unsigned int hd0;
    volatile unsigned int hd1;
    volatile unsigned int spl;
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
 * @param value 1，上拉；0，不上拉
 *
 * @return 0：成功  非0：失败
 */
u32 gpio_set_pull_up(u32 gpio, u32 value);

/**
 * @brief gpio_set_pull_down，设置引脚的下拉，下拉电阻10K，当引脚为输入模式时才有效
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param value 1，下拉；0，不下拉
 *
 * @return 0：成功  非0：失败
 */
u32 gpio_set_pull_down(u32 gpio, u32 value);


enum GPIO_HDRIVE {
    GPIO_HDRIVE_2_4_MA = 0, //2.4mA
    GPIO_HDRIVE_8_MA,    //8mA
    GPIO_HDRIVE_26_4_MA, //26.4mA
    GPIO_HDRIVE_40_MA,   //60mA
};
/**
 * @brief gpio_set_hd, 设置引脚的输出电流，当引脚为输出模式时才有效
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param value see enum GPIO_HDRIVE
 *        hd0: 1，关闭内阻增强输出  0，存在内阻，芯片默认的
 *        hd1: 1，电流变大增强输出； 0，默认电流
 *
 * @return 0：成功  非0：失败
 */
u32 gpio_set_hd(u32 gpio, u32 value);

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

u32 gpio_set_spl(u32 gpio, u32 value);
/**
 * @brief get_gpio, 解析字符串，获取引脚对应的序号
 *
 * @param p 引脚字符串，如“PA00”, "PA01"，，，，
 *
 * @return 引脚对应的序号. 如传入“PB01”，返回17
 */
u32 get_gpio(const char *p);


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

//return:och 号
int gpio_och_sel_output_signal(u32 gpio, enum OUTPUT_CH_SIGNAL signal);
//return:无效
int gpio_och_disable_output_signal(u32 gpio, enum OUTPUT_CH_SIGNAL signal);
//return:ich 号
int gpio_ich_sel_input_signal(u32 gpio, enum INPUT_CH_SIGNAL signal, enum INPUT_CH_TYPE type);
//return:ich 号
int gpio_ich_disable_input_signal(u32 gpio, enum INPUT_CH_SIGNAL signal, enum INPUT_CH_TYPE type);

void gpio_clk_out0(u8 gpio, enum CLK_OUT_SOURCE clk);
void gpio_clk_out1(u8 gpio, enum CLK_OUT_SOURCE clk);
void gpio_clk_out2(u8 gpio, enum CLK_OUT2_SOURCE clk, u8 div);//div:0~63(div1~div64)

u32 gpio_die(u32 gpio, u32 start, u32 len, u32 dat, enum gpio_op_mode op);

u32 gpio_dieh(u32 gpio, u32 start, u32 len, u32 dat, enum gpio_op_mode op);

u32 gpio_dir(u32 gpio, u32 start, u32 len, u32 dat, enum gpio_op_mode op);

u32 gpio_set_pu(u32 gpio, u32 start, u32 len, u32 dat, enum gpio_op_mode op);

u32 gpio_set_pd(u32 gpio, u32 start, u32 len, u32 dat, enum gpio_op_mode op);

#endif  /*GPIO_H*/


