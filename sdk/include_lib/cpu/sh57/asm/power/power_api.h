#ifndef __POWER_API_H__
#define __POWER_API_H__

#define AT_VOLATILE_RAM_CODE_POWER        AT(.power_driver.text.cache.L1)

//=========================电源参数配置==================================
struct low_power_param {
    //---------------power_config
    u8 vddiom_lev;			//vddiom
    u8 vddiow_lev;			//vddiow

    //--------------lowpower
    u8 config;				//低功耗使能，蓝牙&&系统空闲可进入低功耗
    u8 osc_type;			//低功耗晶振类型，btosc/lrc
    u32 btosc_hz;			//蓝牙晶振频率
    u32 osc_delay_us;		//低功耗晶振起振延时，为预留配置。

    u8 lptmr_flow;			//低功耗参数由用户配置
    u32 t1;
    u32 t2;
    u32 t3_lrc;
    u32 t4_lrc;
    u32 t3_btosc;
    u32 t4_btosc;
};

//config
#define SLEEP_EN                            BIT(0)
#define DEEP_SLEEP_EN                       BIT(1)

//osc_type
enum {
    OSC_TYPE_LRC = 0,
    OSC_TYPE_BT_OSC,
    OSC_TYPE_NULL,
};

//电源模式
enum {
    PWR_LDO15,
    PWR_DCDC15,
};

//==============================电源接口============================
void power_init(const struct low_power_param *param);

//==========================lowpower===============================
void power_config_vdc13_cap(u8 en);
void power_config_flash_pg_vddio(u8 en);
void power_config_wvdd_lev(u8 lev);
void power_config_sf_vddio_keep(u8 type);
void power_config_pd_vddio_keep(u8 type);

enum {
    VDDIO_KEEP_TYPE_NORMAL = 0,
    VDDIO_KEEP_TYPE_TRIM,
    VDDIO_KEEP_TYPE_PG,
};


//==============================sleep接口============================
//slee模式介绍
//1.所有数字模块停止，包括cpu、periph、audio、rf等
//2.所有模拟模块停止，包括pll、btosc、rc等
//3.只保留pmu模块

//light_sleep: 不切电源域
//normal_sleep: dvdd低电
//deepsleep：dvdd掉电
struct low_power_operation {

    const char *name;

    u32(*get_timeout)(void *priv);

    void (*suspend_probe)(void *priv);

    void (*suspend_post)(void *priv, u32 usec);

    void (*resume)(void *priv, u32 usec);

    void (*resume_post)(void *priv, u32 usec);
};

enum LOW_POWER_LEVEL {
    LOW_POWER_MODE_LIGHT_SLEEP = 1,
    LOW_POWER_MODE_SLEEP,
    LOW_POWER_MODE_DEEP_SLEEP,
};

typedef u8(*idle_handler_t)(void);
typedef enum LOW_POWER_LEVEL(*level_handler_t)(void);

typedef u8(*idle_handler_t)(void);

struct lp_target {
    char *name;
    level_handler_t level;
    idle_handler_t is_idle;
};

#define REGISTER_LP_TARGET(target) \
        const struct lp_target target SEC_USED(.lp_target)


extern const struct lp_target lp_target_begin[];
extern const struct lp_target lp_target_end[];

#define list_for_each_lp_target(p) \
    for (p = lp_target_begin; p < lp_target_end; p++)

struct deepsleep_target {
    char *name;
    u8(*enter)(void);
    u8(*exit)(void);
};

#define DEEPSLEEP_TARGET_REGISTER(target) \
        const struct deepsleep_target target sec(.deepsleep_target)


extern const struct deepsleep_target deepsleep_target_begin[];
extern const struct deepsleep_target deepsleep_target_end[];

#define list_for_each_deepsleep_target(p) \
    for (p = deepsleep_target_begin; p < deepsleep_target_end; p++)


void *low_power_get(void *priv, const struct low_power_operation *ops);

void low_power_put(void *priv);

void *low_power_sys_get(void *priv, const struct low_power_operation *ops);

void low_power_sys_put(void *priv);

void low_power_sys_request(void *priv);

u8 is_low_power_mode(enum LOW_POWER_LEVEL level);

u8 low_power_sys_is_idle(void);

s32 low_power_trace_drift(u32 usec);

void low_power_reset_osc_type(u8 type);

u8 low_power_get_default_osc_type(void);

u8 low_power_get_osc_type(void);

void low_power_on(void);

void low_power_request(void);

void sys_power_down(u32 usec);

//==============================soff接口============================
struct soft_flag0_t {
    u8 wdt_dis: 1;
    u8 lvd_en: 1;
    u8 uart_key_port: 1;
    u8 flash_power_keep: 1;
    u8 skip_flash_reset: 1;
    u8 sfc_fast_boot: 1;
    u8 flash_stable_delay_sel: 2;
};

struct boot_soft_flag_t {
    /*
    u8 soff_wkup;
    union {
         struct efuse_flag_t eboot_ctrl;
         u8 value;
     } eflag;
    */
    union {
        struct soft_flag0_t boot_ctrl;
        u8 value;
    } flag0;
};

extern struct boot_soft_flag_t g_boot_soft_flag ;

enum soft_flag_io_stage {
    SOFTFLAG_HIGH_RESISTANCE,
    SOFTFLAG_PU,
    SOFTFLAG_PD,

    SOFTFLAG_OUT0,
    SOFTFLAG_OUT0_HD0,
    SOFTFLAG_OUT0_HD,
    SOFTFLAG_OUT0_HD0_HD,

    SOFTFLAG_OUT1,
    SOFTFLAG_OUT1_HD0,
    SOFTFLAG_OUT1_HD,
    SOFTFLAG_OUT1_HD0_HD,
};

void power_set_soft_poweroff(void);
void mask_softflag_config(struct boot_soft_flag_t *softflag);

void soff_latch_release();



#endif
