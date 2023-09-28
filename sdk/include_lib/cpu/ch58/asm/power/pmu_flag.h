//*********************************************************************************//
// Module name : pmu_flag.h                                                            //
// Description : soft flag and efuse head file                                     //
// By Designer : longshusheng                                                      //
// Dat changed :                                                                   //
//*********************************************************************************//

#ifndef __PMU_FLAG__
#define __PMU_FLAG__


enum soft_flag_io_stage {
    SOFTFLAG_HIGH_RESISTANCE,
    SOFTFLAG_PU10K,
    SOFTFLAG_PD10K,

    SOFTFLAG_OUT0,
    SOFTFLAG_OUT0_HD0,
    SOFTFLAG_OUT0_HD,
    SOFTFLAG_OUT0_HD0_HD,

    SOFTFLAG_OUT1,
    SOFTFLAG_OUT1_HD0,
    SOFTFLAG_OUT1_HD,
    SOFTFLAG_OUT1_HD0_HD,

    SOFTFLAG_PU100K,
    SOFTFLAG_PU1M,
    SOFTFLAG_PD100K,
    SOFTFLAG_PD1M,
};

//p33 soft flag
//==============================软关机参数配置============================
//软关机会复位寄存器，该参数为传给rom配置的参数。
struct soft_flag0_t {
    u8 wdt_dis: 1;
    u8 lvd_en: 1;
    u8 efuse_page1_2_read_en: 1;
    u8 flash_power_keep: 1;
    u8 skip_flash_reset: 1;
    u8 lrc_init_en: 1;
    u8 flash_stable_delay_sel: 1;       //0: 0mS;   1: 4mS
    u8 sfc_flash_stable_delay_sel: 1;   //0: 0.5mS; 1: 1mS
};

struct soft_flag1_t {
    u8 usbdp: 4;
    u8 usbdm: 4;
};

struct soft_flag2_t {
    u8 avddcp_short_en: 1;  //0:always short; 1:short every wakeup
    u8 avddcp_vol_sel: 3;
    u8 avddcp_delay_sel: 2; //for avddcp. 0:0.5mS;  1:0.75mS; 2:1.00mS; 3:1.25mS;
    //for avddr.  0:0.2mS;  1:0.3mS;  2:0.4mS;  3:0.5mS;
    u8 fast_adc: 1;         //0: slow power up; 1: fast adc powerup
    u8 avddcp_hadc_sel: 1;  //0: avdd charge pump enable; 1: avddcp short to VPWR; HADC enable;
};

struct soft_flag3_t {
    u8 avddcp_clktune_sel: 4;
    u8 avddcp_clkdiv_sel: 1;//0: div1; 1: div2;     //use in VPWR AVDDCP short delay. 0: 50uS; 1: 1mS
    u8 avddr_vol_sel: 3;
};

struct soft_flag7_4_t {
    /* 04-00*/  u32 avddr_ref_sel: 4;
    /* 15-04*/  u32 lrc_pll_ds_cfg: 12;
    /* 31-16*/  u32 res: 16;
};


struct soft_flag11_8_t {
    u32 chip_key : 32;
};

struct boot_soft_flag_t {
    u8 soff_wkup;
    union {
        struct soft_flag0_t boot_ctrl;
        u8 value;
    } flag0;
    union {
        struct soft_flag1_t boot_ctrl;
        u8 value;
    } flag1;
    union {
        struct soft_flag2_t boot_ctrl;
        u8 value;
    } flag2;
    union {
        struct soft_flag3_t boot_ctrl;
        u8 value;
    } flag3;
    union {
        struct soft_flag7_4_t boot_ctrl;
        u32 value;
    } flag7_4;
    union {
        struct soft_flag11_8_t boot_ctrl;
        u32 value;
    } flag11_8;
};




//*********************************************************************************//
//                                                                                 //
//                               end of this module                                //
//                                                                                 //
//*********************************************************************************//
#endif
