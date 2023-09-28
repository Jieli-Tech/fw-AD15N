#ifndef __P33_API_H__
#define __P33_API_H__

void adc_pmu_ch_enable();
void adc_pmu_ch_disable();
void adc_pmu_ch_select(u16 ch);


//
//
//					vol
//
//
//
/****************************************************************/
enum {
    DVDD_VOL_SEL_084V = 0,
    DVDD_VOL_SEL_087V,
    DVDD_VOL_SEL_090V,
    DVDD_VOL_SEL_093V,
    DVDD_VOL_SEL_096V,
    DVDD_VOL_SEL_099V,
    DVDD_VOL_SEL_102V,
    DVDD_VOL_SEL_105V,
    DVDD_VOL_SEL_108V,
    DVDD_VOL_SEL_111V,
    DVDD_VOL_SEL_114V,
    DVDD_VOL_SEL_117V,
    DVDD_VOL_SEL_120V,
    DVDD_VOL_SEL_123V,
    DVDD_VOL_SEL_126V,
    DVDD_VOL_SEL_129V,
};

enum {
    DCVDD_VOL_SEL_100V = 0,
    DCVDD_VOL_SEL_105V,
    DCVDD_VOL_SEL_110V,
    DCVDD_VOL_SEL_115V,
    DCVDD_VOL_SEL_120V,
    DCVDD_VOL_SEL_125V,
    DCVDD_VOL_SEL_130V,
    DCVDD_VOL_SEL_135V,
    DCVDD_VOL_SEL_140V,
    DCVDD_VOL_SEL_145V,
    DCVDD_VOL_SEL_150V,
    DCVDD_VOL_SEL_155V,
    DCVDD_VOL_SEL_160V,
};

enum {
    VDDIOM_VOL_21V = 0,
    VDDIOM_VOL_22V,
    VDDIOM_VOL_23V,
    VDDIOM_VOL_24V,
    VDDIOM_VOL_25V,
    VDDIOM_VOL_26V,
    VDDIOM_VOL_27V,
    VDDIOM_VOL_28V,
    VDDIOM_VOL_29V,
    VDDIOM_VOL_30V,
    VDDIOM_VOL_31V,
    VDDIOM_VOL_32V,
    VDDIOM_VOL_33V,
    VDDIOM_VOL_34V,
    VDDIOM_VOL_35V,
    VDDIOM_VOL_36V,
};
enum {
    VDDIOW_VOL_21V = 0,
    VDDIOW_VOL_22V,
    VDDIOW_VOL_23V,
    VDDIOW_VOL_24V,
    VDDIOW_VOL_25V,
    VDDIOW_VOL_26V,
    VDDIOW_VOL_27V,
    VDDIOW_VOL_28V,
    VDDIOW_VOL_29V,
    VDDIOW_VOL_30V,
    VDDIOW_VOL_31V,
    VDDIOW_VOL_32V,
    VDDIOW_VOL_33V,
    VDDIOW_VOL_34V,
    VDDIOW_VOL_35V,
    VDDIOW_VOL_36V,
};

enum {
    WLDO_LEVEL_050V = 0,
    WLDO_LEVEL_054V,
    WLDO_LEVEL_058V,
    WLDO_LEVEL_062V,
    WLDO_LEVEL_066V,
    WLDO_LEVEL_070V,
    WLDO_LEVEL_085V,
    WLDO_LEVEL_120V,
};

void dvdd_vol_sel(u8 vol);
u8 get_dvdd_vol_sel();
void dcvdd_vol_sel(u8 vol);
u8 get_dcvdd_vol_sel();
void vddiom_vol_sel(u8 vol);
u8 get_vddiom_vol_sel();
void vddiow_vol_sel(u8 vol);
u8 get_vddiow_vol_sel();

//
//
//				 p33_io
//
//
//
/****************************************************************/
bool is_pinr_en();
u8 get_pinr_port();
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
bool is_mclr_en();

//
//
//					lvd
//
//
//
/****************************************************************/
typedef enum {
    LVD_RESET_MODE,		//复位模式
    LVD_EXCEPTION_MODE, //异常模式，进入异常中断
    LVD_WAKUP_MODE,     //唤醒模式，进入唤醒中断，callback参数为回调函数
} LVD_MODE;

typedef enum {
    VLVD_SEL_18V = 0,
    VLVD_SEL_19V,
    VLVD_SEL_20V,
    VLVD_SEL_21V,
    VLVD_SEL_22V,
    VLVD_SEL_23V,
    VLVD_SEL_24V,
    VLVD_SEL_25V,
} LVD_VOL;

void lvd_en(u8 en);
void lvd_config(LVD_VOL vol, u8 expin_en, LVD_MODE mode, void (*callback));



#endif
