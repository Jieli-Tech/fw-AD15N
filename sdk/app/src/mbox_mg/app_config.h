#ifndef _MBOX_CONFIG_H
#define _MBOX_CONFIG_H
#include "app_modules.h"
#include "common/ui/ui_common.h"

#define ENABLE								1
#define DISABLE								0

/*---------UART Configuration---------------*/
#define UART_TX_OUTPUT_CH_EN        ENABLE
#define UART_OUTPUT_CH_PORT			IO_PORTA_07

/*---------KEY Configuration---------------*/
//AD KEY
#define AD_KEY_CH_SEL				ADC_CH_PA15
//IR KEY
#define IR_KEY_IO_SEL				IO_PORTA_09
#define TCFG_ADKEY_IR_IO_REUSE		DISABLE//ADKEY 和 红外IO复用
//MATRIX KEY
///X轴 io 要求是AD口，详细AD口看adc_drv.h
#define X_ADC_CH_SEL 				{ADC_CH_PA2,ADC_CH_PA3,ADC_CH_PA5}
///Y轴 io 要求是普通IO口
#define Y_PORT_SEL 					{IO_PORTA_10,IO_PORTA_11}
#define MATRIX_KEY_THRESHOLD_SET	300
//TOUCH KEY
#define TOUCH_KEY_SEL				{IO_PORTA_09,IO_PORTA_10,IO_PORTA_11}

/*---------KEY VOICE Configuration---------------*/
#define KEY_VOICE_EN   				1

/*---------power wakeup io---------------*/
#define POWER_WAKEUP_IO				IO_PORTA_15

/*---------udisk powerdown ctrl---------------*/
#define POWERDOWN_UDISK_MODE_EN		0



/*---------UI Configuration---------------*/
#define LED_FADE_ON()
#define LED_FADE_OFF()
#define LED_5X7					1//与外挂flash io冲突

#if LED_5X7
#define UI_ENABLE				1
#else
#define UI_ENABLE				0
#endif

/*---------REC Configuration---------------*/
#define REC_ENABLE

/*---------LINEIN Configuration---------------*/
#define LINEIN_EN
#define LINEIN_DET_MULT_SD		1

/*---------FM Configuration---------------*/
#if FM_EN
#define RDA5807					1
#define BK1080					0
#define KT0830EG				0
#define QN8035					0
#define AR1019					0
#if RDA5807 || BK1080 || KT0830EG || QN8035 || AR1019
#define FM_ENABLE           //FM 模式开关
#endif
#define FM_SD_IO_REUSE 1
#define TCFG_FM_RDA5807_ENABLE ENABLE
#define TCFG_FM_QN8035_ENABLE  ENABLE
#define TCFG_FM_BK1080_ENABLE  ENABLE

#else

#define TCFG_FM_RDA5807_ENABLE DISABLE
#define TCFG_FM_QN8035_ENABLE  DISABLE
#define TCFG_FM_BK1080_ENABLE  DISABLE
#endif

/*---------System Charateristic Configuration v100---------------*/
///<音乐播放功能选择
#define    LAST_MEM_FILE_PLAY_EN        //是否允许记忆文件序号播放功能
#ifdef     LAST_MEM_FILE_PLAY_EN
#define    BREAK_POINT_PLAY_EN			//是否允许断点播放功能
/*
 *功能描述：自动实时断点保存会根据EEPROM大小和vm的使用状况自动算出来一个合适的断点保存间隔。
 适当放大EEPROM和关闭不使用的解码格式（例如F1A,UMP3）可以让程序的自动保存间隔更短。
 ******************注意事项**************************
注意：实时断点保存功能需要频繁写flash，很多flash频繁写会导致flash坏掉。这个功能应用需要自己和
flash厂家联系评估写的频率是否是产品安全范围。
 * */
#define    BREAK_POINT_AUTO_SAVE		0
#endif

//#define     GET_MUSIC_TOTAL_TIME         //是否获取音乐文件总时间
#define     RANDOM_PLAY_EN               //是否支持随机播放功能
#define 	FOLDER_PLAY_EN               //是否支持文件夹切换和播放功能

/*---------dev upgrade Configuration---------------*/
#if (HAS_UPDATE_EN)
#define TFG_DEV_UPGRADE_SUPPORT 		1
#define TFG_UPGRADE_FILE_NAM			"/update.ufw"
#else
#define TFG_DEV_UPGRADE_SUPPORT 		0
#endif

/*---------FLASH Configuration---------------*/
#define TCFG_FLASH_SPI_TYPE_SELECT 		1//1:flash 选择硬件spi; 0:flash use soft_spi
#define HW_SPI_WORK_MODE				SPI_MODE_BIDIR_1BIT
#define SPI_READ_DATA_WIDTH				SPI_MODE_BIDIR_1BIT
#define SPI_CS_PORT_SEL					IO_PORTA_05
//port select for soft spi
#define A_CLK_BIT           			BIT(12)// set clk
#define A_CLK_PORT(x)       			JL_PORTA->x
#define A_D0_BIT            			BIT(11)// set d0
#define A_D0_PORT(x)        			JL_PORTA->x
#define A_D1_BIT            			BIT(11)// set d1
#define A_D1_PORT(x)        			JL_PORTA->x
#define SOFT_SPI_WORK_MODE				SPI_MODE_UNIDIR_1BIT//只支持双向或单线
#if TCFG_FLASH_SPI_TYPE_SELECT
#define SPI_HW_NUM						1
#else
#define SPI_HW_NUM						0
#endif

/*sd 和 flash复用使能*/
#define SPI_SD_IO_REUSE					0//SPI_FLASH与SD卡模块IO复用使能
/*---------SD Configuration---------------*/
///<SD卡接口选择
#if defined(TFG_SD_EN) && (TFG_SD_EN)
#define SDMMCA_EN
#endif

/*---------USB Configuration---------------*/
#define TCFG_PC_ENABLE						ENABLE  //PC模块使能
#define TCFG_USB_MSD_CDROM_ENABLE           DISABLE
#define TCFG_USB_EXFLASH_UDISK_ENABLE       DISABLE  //外掛FLASH UDISK
#define TCFG_UDISK_ENABLE					ENABLE //U盘模块使能
#define TCFG_HID_HOST_ENABLE				DISABLE
#define TCFG_ADB_ENABLE      				DISABLE
#define TCFG_AOA_ENABLE      				DISABLE
#define TCFG_PUSH_CODE_ENABLE               DISABLE  //该功能需要关闭OTG使能

#define TCFG_USB_PORT_CHARGE                DISABLE
#define TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0  DISABLE

#if TCFG_PC_ENABLE
#define USB_DEVICE_EN       //Enable USB SLAVE MODE
#endif
#if TCFG_UDISK_ENABLE
#define	USB_DISK_EN        //是否可以读U盘
#endif

#if TCFG_PC_ENABLE || TCFG_UDISK_ENABLE
#include "usb_std_class_def.h"
#include "usb_common_def.h"

#undef USB_DEVICE_CLASS_CONFIG
#define  USB_DEVICE_CLASS_CONFIG             (MASSSTORAGE_CLASS|SPEAKER_CLASS|MIC_CLASS|HID_CLASS)  //配置usb从机模式支持的class

#undef TCFG_OTG_MODE
#define TCFG_OTG_MODE                       (TCFG_OTG_MODE_HOST|TCFG_OTG_MODE_SLAVE|TCFG_OTG_MODE_CHARGE)
#else
#define  USB_DEVICE_CLASS_CONFIG            0
#define TCFG_OTG_MODE                       0
#endif

#if TCFG_PUSH_CODE_ENABLE
#include "usb_std_class_def.h"
#include "usb_common_def.h"
#undef TCFG_OTG_MODE
#define TCFG_OTG_MODE                       0
#endif


#endif
