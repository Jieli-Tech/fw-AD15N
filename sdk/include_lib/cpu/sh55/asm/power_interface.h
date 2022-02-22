#ifndef POWER_INTERFACE_H
#define POWER_INTERFACE_H

// #include "asm/hwi.h"
//
#include "typedef.h"
#define NEW_BASEBAND_COMPENSATION       0

#define AT_VOLATILE_RAM             AT(.volatile_ram)
#define AT_VOLATILE_RAM_CODE        AT(.volatile_ram_code)
#define AT_NON_VOLATILE_RAM         AT(.non_volatile_ram)
#define AT_NON_VOLATILE_RAM_CODE    AT(.non_volatile_ram_code)

/*复位原因*/
enum {
    /*主系统*/
    MSYS_P33_RST = 0,
    MSYS_DVDD_POR_RST = 1,
    MSYS_SOFT_RST = 2,
    MSYS_PLPM_RST = 3,
    /*P33*/
    P33_VDDIO_POR_RST = 8,
    P33_VDDIO_LVD_RST = 9,
    P33_WDT_RST = 10,
    P33_VCM_RST = 11,
    P33_MCLR_RST = 12,
    P33_PINR_RST = 13,
    P33_SYS_RST = 14,
    P33_SOFT_RST = 15,
    P33_POWER_RETURN = 16,
    /*SUB*/
    P33_EXCEPTION_SOFT_RST = 17,
};

extern u32 nvbss_begin;
extern u32 nvbss_length;
extern u32 nvdata_begin;
extern u32 nvdata_size;
extern u32 nvdata_addr;

#define NV_RAM_START                &nvbss_begin
#define NV_RAM_SIZE                 &nvbss_length
#define NV_RAM_END                  (NV_RAM_START + NV_RAM_SIZE)

enum {
    MAGIC_ADDR = 2,
    ENTRY_ADDR = 3,
};
#define RAM1_MAGIC_ADDR (NV_RAM_END - MAGIC_ADDR*4)
#define RAM1_ENTRY_ADDR (NV_RAM_END - ENTRY_ADDR*4)

#define SLEEP_EN                            BIT(2)
#define SLEEP_SAVE_TIME_US                  1L
#define BT_SLEEP_RECOVER_TIME_US            1500L
#define LRC_SLEEP_RECOVER_TIME_US           3500L

#define DEEP_SLEEP_EN                       BIT(1)
#define DSLEEP_SAVE_BEFORE_ENTER_MS         1
#define DSLEEP_RECOVER_AFTER_EXIT_MS        10
#define DEEP_SLEEP_TIMEOUT_MIN_US           (60*625L)  //间隔至少要60slot以上才进入power off

#define SLEEP_TICKS_UNIT                    (10*1000L) //
#define DEEP_SLEEP_TICKS_UNIT               (20*1000L) //


enum {
    OSC_TYPE_LRC = 0,
    OSC_TYPE_RTC,
    OSC_TYPE_BT_OSC,
};

enum {
    PWR_NO_CHANGE = 0,
    PWR_LDO33,
    PWR_LDO15,
    PWR_DCDC15,
};

enum {
    LONG_1S_RESET = 0,
    LONG_2S_RESET,
    LONG_4S_RESET,
    LONG_8S_RESET,
    LONG_16S_RESET = 4,
};

//Macro for VDDIOM_VOL_SEL
enum {
    VDDIOM_VOL_20V = 0,
    VDDIOM_VOL_22V,
    VDDIOM_VOL_24V,
    VDDIOM_VOL_26V,
    VDDIOM_VOL_28V,
    VDDIOM_VOL_30V,
    VDDIOM_VOL_32V,
    VDDIOM_VOL_34V,
};

//Macro for VDDIOW_VOL_SEL
enum {
    VDDIOW_VOL_20V = 0,
    VDDIOW_VOL_22V,
    VDDIOW_VOL_24V,
    VDDIOW_VOL_26V,
    VDDIOW_VOL_28V,
    VDDIOW_VOL_30V,
    VDDIOW_VOL_32V,
    VDDIOW_VOL_34V,
};

struct low_power_param {
    u8 osc_type;
    u32 btosc_hz;
    u8  delay_us;
    u8  config;
    u8  btosc_disable;
    u8 dcdc_port;

    u8 vddiom_lev;
    u8 vddiow_lev;
    u8 pd_wdvdd_lev;
    u8 vddio_keep;
    u8 vdc13_keep; //如果进入低功耗时哒哒声，可以将该标志置1。该位置1时，会同时keep住vddio，单耳功耗会大100ua左右，对耳会大200ua左右。(哒哒声是由于电容的效应导致的)

    u32 osc_delay_us;
    u8  fake_rtc;

    u8 flash_pg;

    u8 vdc13_cap_en;
};

struct soft_flag0_t {
    u8 wdt_dis: 1;
    u8 lvd_en: 1;
    u8 usbdp: 2;
    u8 usbdm: 2;
    u8 fast_boot: 1;
    u8 res: 1;
};

struct boot_soft_flag_t {
    union {
        struct soft_flag0_t boot_ctrl;
        u8 value;
    } flag0;
    u32 poweron;
};

#define BLUETOOTH_RESUME    BIT(1)

#define RISING_EDGE         0
#define FALLING_EDGE        1

typedef enum {
    PORT_FLT_NULL = 0,
    PORT_FLT_32us,
    PORT_FLT_64us,
    PORT_FLT_128us,
    PORT_FLT_256us,
    PORT_FLT_512us,
    PORT_FLT_1ms,
    PORT_FLT_2ms,
} PORT_FLT;

struct port_wakeup {
    u8 pullup_down_enable;        //
    u8 edge;        //[0]: Rising / [1]: Falling
    u8 attribute;   //Relate operation bitmap OS_RESUME | BLUETOOTH_RESUME
    u8 iomap;       //Port Group-Port Index
    u8 filter_enable;
};

struct charge_wakeup {
    u8 attribute;   //Relate operation bitmap OS_RESUME | BLUETOOTH_RESUME
};

struct alarm_wakeup {
    u8 attribute;   //Relate operation bitmap OS_RESUME | BLUETOOTH_RESUME
};

struct lvd_wakeup {
    u8 attribute;   //Relate operation bitmap OS_RESUME | BLUETOOTH_RESUME
};

struct sub_wakeup {
    u8 attribute;   //Relate operation bitmap OS_RESUME | BLUETOOTH_RESUME
};

//<Max hardware wakeup port
#define MAX_WAKEUP_PORT    12

struct wakeup_param {
    const PORT_FLT filter;
    const struct port_wakeup *port[MAX_WAKEUP_PORT];
    const struct charge_wakeup *charge;
    const struct alarm_wakeup *alram;
    const struct lvd_wakeup *lvd;
    const struct sub_wakeup *sub;
};

struct reset_param {
    u8 en;
    u8 mode;
    u8 level;
    u8 iomap;   //Port Group, Port Index
    int hold_time;
};

struct low_power_operation {

    const char *name;

    u32(*get_timeout)(void *priv);

    void (*suspend_probe)(void *priv);

    void (*suspend_post)(void *priv, u32 usec);

    void (*resume)(void *priv, u32 usec);

    void (*resume_post)(void *priv, u32 usec);

    void (*off_probe)(void *priv);

    void (*off_post)(void *priv, u32 usec);

    void (*on)(void *priv);
};

u32 __tus_carry(u32 x);

u8 __power_is_poweroff(void);

void poweroff_recover(void);

void power_init(const struct low_power_param *param);

u8 power_is_low_power_probe(void);

u8 power_is_low_power_post(void);

void set_softoff_wakeup_time_ms(u32 ums);

void set_softoff_wakeup_time_sec(u32 sec);

void power_set_soft_poweroff(void);

void power_set_mode(u8 mode);

void power_keep_dacvdd_en(u8 en);

void power_set_callback(u8 mode, void (*powerdown_enter)(u8 step), void (*powerdown_exit)(u32), void (*soft_poweroff_enter)(void));

u8 power_is_poweroff_post(void);
// #define  power_is_poweroff_post()   0

void power_set_proweroff(void);

void reset_source_dump(void);

u8 power_reset_source_dump(void);
/*-----------------------------------------------------------*/

void low_power_on(void);

void low_power_request(void);

void low_power_exit_request(void);

void low_power_lock(void);

void low_power_unlock(void);

void *low_power_get(void *priv, const struct low_power_operation *ops);

void low_power_put(void *priv);

u8 low_power_sys_request(void *priv);

void *low_power_sys_get(void *priv, const struct low_power_operation *ops);

void low_power_sys_put(void *priv);

u8 low_power_sys_is_idle(void);

s32 low_power_trace_drift(u32 usec);

void low_power_reset_osc_type(u8 type);

u8 low_power_get_default_osc_type(void);

u8 low_power_get_osc_type(void);
/*-----------------------------------------------------------*/

void power_wakeup_index_enable(u8 index);

void power_wakeup_index_disable(u8 index);

void power_wakeup_init(const struct wakeup_param *param);

void power_wakeup_port_set(u8 idx, struct port_wakeup *port);

void power_wakeup_init_test();

u8 get_wakeup_source(void);

u8 is_ldo5v_wakeup(void);
// void power_wakeup_callback(JL_SignalEvent_t cb_event);

void p33_soft_reset(void);
/*-----------------------------------------------------------*/
void power_reset_init(const struct reset_param *rs_param);		//长按复位接口

void power_reset_close();

void lrc_debug(u8 a, u8 b);

void sdpg_config(int enable);

void power_set_wvdd(u8 level);

int cpu_reset_by_soft();
/*-----------------------------------------------------------*/
void reset_vddiom_lev(u8 lev);

/*-----------------------------------------------------------*/
void lrc_scan();

/*-----------------------------------------------------------*/
void mask_softflag_config(const struct boot_soft_flag_t *softflag);

/*-----------------------------------------------------------*/
void lvd_reset_en();

void lvd_wkup_en();

void __lvd_irq_handler(void);

void spi_flash_port_unmount(void);

void spi_flash_power_release(void);

void softoff_io_reinit();

void powerdown_io_reinit();


typedef u8(*idle_handler_t)(void);

struct lp_target {
    char *name;
    idle_handler_t is_idle;
};

#define REGISTER_LP_TARGET(target) \
        const struct lp_target target sec_used(.lp_target)


extern const struct lp_target lp_target_begin[];
extern const struct lp_target lp_target_end[];

#define list_for_each_lp_target(p) \
    for (p = lp_target_begin; p < lp_target_end; p++)
/*-----------------------------------------------------------*/

#endif
