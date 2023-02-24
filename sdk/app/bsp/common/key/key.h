#ifndef _KEY_H_
#define _KEY_H_

#include "typedef.h"
#include "sdk_cfg.h"
#include "app_config.h"
//#define KEY_UART_DEBUG

#ifdef KEY_UART_DEBUG
#define key_puts           log_info
#define key_printf         log_info
#define key_buf            log_info_hexdump
#else
#define key_puts(...)
#define key_printf(...)
#define key_buf(...)
#endif/*KEY_UART_DEBUG*/

/*按键输出注册接口*/
typedef int (*key_emit_t)(u8 key_status, u8 key_num, u8 key_type);

#ifndef CPU_SH57
#define KEY_IO_EN         	    0   ///<IO按键使能
#define KEY_AD_EN				1	///<AD按键使能
#define KEY_MATRIX_EN			0	///<矩阵按键使能
#define KEY_IR_EN				0   ///<IR按键使能
#define KEY_TOUCH_EN			0   ///<触摸按键使能
#else
#define KEY_IO_EN         	    1   ///<IO按键使能
#define KEY_AD_EN				0	///<AD按键使能
#define KEY_MATRIX_EN			0	///<矩阵按键使能
#define KEY_IR_EN				0   ///<IR按键使能
#define KEY_TOUCH_EN			0   ///<触摸按键使能
#endif

/*双击按键使能*/
#define KEY_DOUBLE_CLICK_EN		0


/*按键类型*/
typedef enum {
    KEY_TYPE_IO,
    KEY_TYPE_AD,
    KEY_TYPE_MATRIX,
    KEY_TYPE_IR,
    KEY_TYPE_TOUCH,
} KEY_TYPE;

/*按键门槛值*/
#define KEY_BASE_CNT  2
#define KEY_LONG_CNT  75
#define KEY_HOLD_CNT  15
#define KEY_SHORT_CNT 3

/*按键状态*/
enum {
    KEY_SHORT    =   0x0,
    KEY_SHORT_UP =   0x1,
    KEY_LONG     =   0x2,
    KEY_HOLD     =   0x3,
    KEY_LONG_UP  =   0x4,
    KEY_DOUBLE	 =	 0x5,

    KEY_MAX_STATUS,
};

/*按键最大个数*/
#define IO_KEY_MAX_NUM		10
#define AD_KEY_MAX_NUM		10
#define MATRIX_KEY_MAX_NUM  10
#define IR_KEY_MAX_NUM 		21
#define TOUCH_KEY_MAX_NUM	10

#define KEY_DOUBLE_CLICK_CNT    35 //35*10ms

#define NO_KEY          0xff

typedef struct {
    KEY_TYPE key_type;
    void (*key_init)(void);
    u8(*key_get_value)(void);
} key_interface_t;

typedef struct {
    u8 key_type;
    u8 key_num;
} key_io_t;

void key_init(void);
void key_scan();
void key_table_sel(void *msg_filter);
#endif
