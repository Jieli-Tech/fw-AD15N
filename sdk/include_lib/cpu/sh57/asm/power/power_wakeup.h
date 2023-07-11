#ifndef __POWER_WAKEUP_H__
#define __POWER_WAKEUP_H__

//=========================唤醒原因==================================
enum WAKEUP_REASON {
    PWR_WK_REASON_PLUSE_CNT_OVERFLOW,   	    //pcnt唤醒复位
    PWR_WK_REASON_P11,   					    //P11唤醒复位
    PWR_WK_REASON_LPCTMU,   				    //触摸唤醒复位
    PWR_WK_REASON_PORT_EDGE,				    //数字io输入边沿唤醒复位
    PWR_WK_REASON_ANA_EDGE,					    //模拟io输入边沿唤醒复位
    PWR_WK_REASON_VDDIO_LVD,				    //vddio lvd唤醒复位
    PWR_WK_REASON_WDT,				    		//vddio lvd看门狗唤醒复位
    PWR_WK_REASON_EDGE_INDEX0,				    //p33 index0  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX1,				    //p33 index1  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX2,			    	//p33 index2  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX3,				    //p33 index3  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX4,     	            //p33 index4  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX5,          	    //p33 index5  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX6,          	    //p33 index6  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX7,          	    //p33 index7  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX8,          	    //p33 index8  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX9,          	    //p33 index9  io唤醒复位
    PWR_WK_REASON_EDGE_INDEX10,         	    //p33 index10 io唤醒复位
    PWR_WK_REASON_EDGE_INDEX11,         	    //p33 index11 io唤醒复位
    PWR_ANA_WK_REASON_FALLINIG_EDGE_LDOIN, 		//LDO5V上升沿唤醒复位
    PWR_ANA_WK_REASON_RISING_EDGE_LDOIN,        //LDO5V下降沿唤醒复位
    PWR_ANA_WK_REASON_FALLING_EDGE_VBATCH,      //VBATCH上升降沿唤醒复位
    PWR_ANA_WK_REASON_RISING_EDGE_VBATCH,       //VBATCH下降沿唤醒复位
    PWR_ANA_WK_REASON_RISING_EDGE_VPWRDT,     	//p33 vpwr detect上升沿唤醒复位
    PWR_ANA_WK_REASON_FALLINIG_EDGE_VPWRDT,     //p33 vpwr detec下降沿t唤醒复位

    PWR_RTC_WK_REASON_ALM, 					    //RTC闹钟唤醒复位
    PWR_RTC_WK_REASON_256HZ, 					//RTC 256Hz时基唤醒复位
    PWR_RTC_WK_REASON_64HZ, 					//RTC 64Hz时基唤醒复位
    PWR_RTC_WK_REASON_2HZ, 						//RTC 2Hz时基唤醒复位
    PWR_RTC_WK_REASON_1HZ, 						//RTC 1Hz时基唤醒复位

    PWR_WKUP_REASON_RESERVE = 63,

};



//=========================唤醒参数配置==================================
#define MAX_WAKEUP_PORT     8  //最大同时支持数字io输入个数

typedef enum {
    RISING_EDGE = 0,
    FALLING_EDGE,
} P33_IO_WKUP_EDGE;

typedef enum {
    PORT_FLT_NULL = 0,
    PORT_FLT_32us,
    PORT_FLT_64us,
    PORT_FLT_128us,
    PORT_FLT_256us,
    PORT_FLT_512us,
    PORT_FLT_1ms,
    PORT_FLT_2ms,
} P33_IO_WKUP_FLT;

struct p33_io_wkup_config {
    u8 iomap;      			  //唤醒io
    u8 pullup_down_enable;    //上下拉是否使能
    u8 filter_enable;		  //滤波使能，所有通道共用滤波参数
    P33_IO_WKUP_EDGE edge;     //唤醒边沿条件
};

struct p33_io_wkup_param {
    P33_IO_WKUP_FLT filter;	  //滤波参数，数字
    const struct p33_io_wkup_config *port[MAX_WAKEUP_PORT];
};

//=========================唤醒接口==================================
void power_wakeup_init(const struct p33_io_wkup_param *param);

void p33_io_wakeup_init(const struct p33_io_wkup_param *param);

void p33_io_wkup_index_init(u8 index, const struct p33_io_wkup_config *port);

/**
  * @brief gpio_longpress_pin0_reset_config
  *
  * @param pin 任意GPIO
  * @param level 0(下降沿触发) 1(上升沿触发)
  * @param time 0(disable) 1 2 4 8 16单位为秒
  * @param relase 0(等待电平翻转后复位) 1(立即复位)
  * @param protect 0(关闭写 保护) 1(打开写保护)
  */
void gpio_longpress_pin0_reset_config(u32 pin, u32 level, u32 time, u8 release, u8 protect);


void p33_io_wakeup_set_callback(void (*callback)(u8 index));

void lvd_wakeup_set_callback(void (*callback)());


#endif
