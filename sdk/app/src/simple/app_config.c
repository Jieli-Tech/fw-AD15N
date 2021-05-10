#include "includes.h"
//中断优先级
//系统使用到的
const int IRQ_IRTMR_IP   = 5;
const int IRQ_AUDIO_IP   = 3;
const int IRQ_DECODER_IP = 2;
const int IRQ_WFILE_IP   = 1;
const int IRQ_ADC_IP     = 1;
const int IRQ_ENCODER_IP = 0;
const int IRQ_TICKTMR_IP = 0;
const int IRQ_USB_IP	 = 3;
const int IRQ_SD_IP		 = 3;
//系统还未使用到的
const int IRQ_UART0_IP   = 3;
const int IRQ_UART1_IP   = 3;
const int IRQ_ALINK0_IP  = 3;

//时钟
const char LRC_TRIM_DISABLE = FALSE;  //LRC trim 主时钟

//内存管理
const char MM_ASSERT     = TRUE;  //malloc内部断言
//
const char MIC_CAPLESS_EN   = FALSE; // MIC省电容的使能
const char MIC_RES_BUILT_IN = TRUE;  // MIC使用内置偏置电阻的使能
//00000:NA;     00001:2;   00010:5;   00011:1.425; 00100:7;   00101:1.555; 00110:2.916; 00111:1.186;
//01000:NA;     01001:3;   01010:6;   01011:2.428; 01100:8;   01101:2.555; 01110:3.916; 01111:2.186;
//10000:NA;     10001:3.5; 10010:6.5; 10011:2.926; 10100:8.5; 10101:3.055; 10110:4.416; 10111:2.686;
//11000:NA;     11001:2.6; 11010:5.6; 11011:2.208; 11100:7.6; 11101:2.4;   11110:3.73;  11111:1.99;
//000001:0.85k; 0000001:0.35k
const char MIC_RES_IN       = 7;    //内置MIC的电阻,低3bit不能为0
const char MIC_0DB          = 1;    //MIC_0db_11V   0:6db   1:0db
//MIC_PGA_G_11V[4:0] x0000: 0dB x0001:2dB x0010:4db   .... x1110: 28db
const char MIC_PGA_G        = 14;    //0 ~ 14


const char AUDIO_ADC_SR_COR = FALSE; // 采样率自动纠错开关
const u32  AUDIO_ADC_SR     = 12000; // 8000 12000 16000 24000
const u8   AUDIO_VCMCAP     = FALSE;	 //TRUE:VCM有电容   FALSE:VCM没有电容

//
const char ADC_VBG_TRIM_EN	= TRUE; // VBG TRIM

//升级使用的区域，0：VM区， 1：eeprom区
const u8 dev_update_use_eeprom = 0;

//内核异常打印
const u8 config_asser = 1;
/**
 * @brief Bluetooth Controller Log
 */
/*-----------------------------------------------------------*/

const char libs_debug AT(.LOG_TAG_CONST) = TRUE; //打印总开关

#define  CONFIG_DEBUG_LIBS(X)   (X & libs_debug)

const char log_tag_const_i_MAIN AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_d_MAIN AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_MAIN AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);

const char log_tag_const_i_KEYM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_d_KEYM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_KEYM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);

const char log_tag_const_i_MUGRD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_d_MUGRD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_MUGRD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);

const char log_tag_const_i_PWRA AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_d_PWRA AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_PWRA AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);

const char log_tag_const_i_LP_TIMER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_d_LP_TIMER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);

const char log_tag_const_e_LP_TIMER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);


const char log_tag_const_i_P33 AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_d_P33 AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_P33 AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);


const char log_tag_const_i_LRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_d_LRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_LRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);


const char log_tag_const_i_WKUP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_d_WKUP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_WKUP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);


const char log_tag_const_i_PMU AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_d_PMU AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_PMU AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);


const char log_tag_const_i_SOFI AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_d_SOFI AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_SOFI AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);


const char log_tag_const_i_UTD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_d_UTD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_UTD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_c_UTD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);


const char log_tag_const_i_LBUF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_d_LBUF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_LBUF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);


const char log_tag_const_i_NORM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_d_NORM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_e_NORM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_c_NORM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);


const char log_tag_const_i_DEBUG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_d_DEBUG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_e_DEBUG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_c_DEBUG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);

const char log_tag_const_i_FLASH AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_d_FLASH AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_FLASH AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_c_FLASH AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);

const char log_tag_const_i_SPI1 AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_d_SPI1 AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_SPI1 AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_c_SPI1 AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);

const char log_tag_const_i_SPI1_TEST AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_d_SPI1_TEST AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_SPI1_TEST AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);

const char log_tag_const_i_EEPROM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_d_EEPROM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_EEPROM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_c_EEPROM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);

const char log_tag_const_i_IIC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_d_IIC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_IIC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_c_IIC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);

const char log_tag_const_i_USB AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_d_USB AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_USB AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_c_USB AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);


const char log_tag_const_i_HEAP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_d_HEAP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_HEAP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_c_HEAP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);

const char log_tag_const_i_VM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_d_VM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
const char log_tag_const_e_VM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(1);
const char log_tag_const_c_VM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIBS(0);
