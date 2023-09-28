#ifndef __P33_SFR_H__
#define __P33_SFR_H__

#include "typedef.h"
//===============================================================================//
//
//      p33 vddio
//
//===============================================================================//
//............. 0X0000 - 0X000F............ for analog control
#define P3_ANA_FLOW0                  0x00
#define P3_ANA_FLOW1                  0x01
#define P3_ANA_FLOW2                  0x02
#define P3_VBG_CON0                   0x03
#define P3_PMU_ADC0                   0x04
#define P3_PMU_ADC1                   0x05
#define P3_IOV_CON0                   0x06
#define P3_IOV_CON1                   0x07
#define P3_DCV_CON0                   0x08
#define P3_DCV_CON1                   0x09
#define P3_DVD_CON0                   0x0a
#define P3_DVD_CON1                   0x0b
#define P3_ANA_MFIX                   0x0c
#define P3_CHG_PUMP                   0x0d

#define P3_ANA_KEEP0                  0x0e
#define P3_ANA_KEEP1                  0x0f

//............. 0X0010 - 0X001F............ for analog others
#define P3_VLVD_CON                   0x10
#define P3_VLVD_TRIM                  0x11
#define P3_RST_SRC                    0x12
#define P3_LRC_CON0                   0x13
#define P3_LRC_CON1                   0x14
#define P3_RST_CON0                   0x15
#define P3_VLVD_FLT                   0x16
#define P3_VLD_KEEP                   0x17
#define P3_WDT_CON                    0x18

//............. 0x0020 - 0x002f............ for analog others
#define P3_PGDR_CON0                  0x20
#define P3_PGDR_CON1                  0x21
#define P3_PGSD_CON                   0x22
#define P3_PGFS_CON                   0x23
//  #define P3_ANA_READ                   0x24
#define P3_DBG_CON0                   0x25

#define P3_LCDC_CON0                  0x28
#define P3_LCDC_CON1                  0x29
#define P3_LCDC_CON2                  0x2a
#define P3_LCDC_CON3                  0x2b
#define P3_LCDC_CON4                  0x2c
#define P3_LCDC_CON5                  0x2d
#define P3_LCDC_CON6                  0x2e
#define P3_LCDC_CON7                  0x2f

//............. 0X0030 - 0X003F............ for PMU manager
#define P3_PMU_CON0                   0x30
#define P3_PMU_CON1                   0x31
#define P3_PMU_CON4                   0x34
#define P3_PMU_CON5                   0x35
#define P3_LP_PRP0                    0x36
#define P3_LP_PRP1                    0x37
#define P3_LP_STB0_STB1               0x38
#define P3_LP_STB2_STB3               0x39
#define P3_LP_STB4_STB5               0x3a
#define P3_LP_STB6                    0x3b
#define P3_RST_FLAG                   0x3c
#define P3_EFU_FLAG                   0x3d

//............. 0X0040 - 0X005F............ for PMU timer
#define P3_LP_RSC00                   0x40
#define P3_LP_RSC01                   0x41
#define P3_LP_RSC02                   0x42
#define P3_LP_RSC03                   0x43
#define P3_LP_PRD00                   0x44
#define P3_LP_PRD01                   0x45
#define P3_LP_PRD02                   0x46
#define P3_LP_PRD03                   0x47
#define P3_LP_RSC10                   0x48
#define P3_LP_RSC11                   0x49
#define P3_LP_RSC12                   0x4a
#define P3_LP_RSC13                   0x4b
#define P3_LP_PRD10                   0x4c
#define P3_LP_PRD11                   0x4d
#define P3_LP_PRD12                   0x4e
#define P3_LP_PRD13                   0x4f
//
#define P3_LP_TMR0_CLK                0x54
#define P3_LP_TMR1_CLK                0x55

#define P3_LP_TMR0_CON                0x58
#define P3_LP_TMR1_CON                0x59

//............. 0X0060 - 0X006F............ for PMU counter
#define P3_LP_CNTRD0                  0x60
#define P3_LP_CNTRD1                  0x61
#define P3_LP_CNT0                    0x62
#define P3_LP_CNT1                    0x63
#define P3_LP_CNT2                    0x64
#define P3_LP_CNT3                    0x65

#define P3_MSTM_RD                    0x68
#define P3_IVS_RD                     0x69
#define P3_IVS_SET                    0x6a
#define P3_IVS_CLR                    0x6b
#define P3_WVDD_AUTO0                 0x6c
#define P3_WVDD_AUTO1                 0x6d

//............. 0X0070 - 0X007F............ for PMU others
#define P3_SFLAG0                     0x70
#define P3_SFLAG1                     0x71
#define P3_SFLAG2                     0x72
#define P3_SFLAG3                     0x73
#define P3_SFLAG4                     0x74
#define P3_SFLAG5                     0x75
#define P3_SFLAG6                     0x76
#define P3_SFLAG7                     0x77
#define P3_SFLAG8                     0x78
#define P3_SFLAG9                     0x79
#define P3_SFLAGA                     0x7a
#define P3_SFLAGB                     0x7b

//............. 0X0080 - 0X008F............ for LCD
#define P3_SEG0_DAT0                  0x80
#define P3_SEG0_DAT1                  0x81
#define P3_SEG0_DAT2                  0x82
#define P3_SEG0_DAT3                  0x83
#define P3_SEG1_DAT0                  0x84
#define P3_SEG1_DAT1                  0x85
#define P3_SEG1_DAT2                  0x86
#define P3_SEG1_DAT3                  0x87
#define P3_SEG2_DAT0                  0x88
#define P3_SEG2_DAT1                  0x89
#define P3_SEG2_DAT2                  0x8a
#define P3_SEG2_DAT3                  0x8b
#define P3_SEG3_DAT0                  0x8c
#define P3_SEG3_DAT1                  0x8d
#define P3_SEG3_DAT2                  0x8e
#define P3_SEG3_DAT3                  0x8f

//............. 0X0090 - 0X009F............ for PORT and clock reset power control
#define P3_PINR_CON                   0x90
#define P3_PINR_SAFE                  0x91
#define P3_PCNT_CON                   0x92
#define P3_PCNT_VLUE                  0x93
#define P3_PR_IN                      0x94
#define P3_PR_OUT                     0x95
#define P3_PR_DIR                     0x96
#define P3_PR_DIE                     0x97
#define P3_PR_PU                      0x98
#define P3_PR_PD                      0x99
#define P3_PR_HD                      0x9a

//............. 0X00A0 - 0X00AF............
#define P3_PR_PWR                     0xa0

#define P3_OSL_CON                    0xa5
#define P3_CLK_CON0                   0xa6

#define P3_WKUP_SRC                   0xa8

#define P3_LS_EN                      0xae
#define P3_ANA_LAT                    0xaf

//............. 0X00B0 - 0X00BF............ for EFUSE
#define P3_EFUSE_CON0                 0xb0
#define P3_EFUSE_CON1                 0xb1
#define P3_EFUSE_RDAT                 0xb2
#define P3_EFUSE_PU_DAT0              0xb3
#define P3_EFUSE_PU_DAT1              0xb4
#define P3_EFUSE_PU_DAT2              0xb5
#define P3_EFUSE_PU_DAT3              0xb6

//............. 0X00C0 - 0X00CF............ for port input select
#define P3_PORT_SEL0                  0xc0
#define P3_PORT_SEL1                  0xc1
#define P3_PORT_SEL2                  0xc2
#define P3_PORT_SEL3                  0xc3
#define P3_PORT_SEL4                  0xc4
#define P3_PORT_SEL5                  0xc5
#define P3_PORT_SEL6                  0xc6
#define P3_PORT_SEL7                  0xc7
#define P3_PORT_SEL8                  0xc8
#define P3_PORT_SEL9                  0xc9
#define P3_PORT_SEL10                 0xca
#define P3_PORT_SEL11                 0xcb

//............. 0X00D0 - 0X00DF............ analog latch
#define P3_SEG4_DAT0                  0xd0
#define P3_SEG4_DAT1                  0xd1
#define P3_SEG4_DAT2                  0xd2
#define P3_SEG4_DAT3                  0xd3
#define P3_SEG5_DAT0                  0xd4
#define P3_SEG5_DAT1                  0xd5
#define P3_SEG5_DAT2                  0xd6
#define P3_SEG5_DAT3                  0xd7
#define P3_SEG_IO_EN0                 0xd8
#define P3_SEG_IO_EN1                 0xd9
#define P3_SEG_IO_EN2                 0xda
#define P3_SEG_IO_EN3                 0xdb
#define P3_LCD_CHG_CON0               0xdc
#define P3_LCD_CHG_CON1               0xdd
#define P3_LCD_CHG_CON2               0xde

//............. 0X00E0 - 0X00EF............ wkup
#define P3_WKUP_FLT_EN0               0xe0
#define P3_WKUP_P_IE0                 0xe1
#define P3_WKUP_N_IE0                 0xe2
#define P3_WKUP_LEVEL0                0xe3
#define P3_WKUP_P_CPND0               0xe4
#define P3_WKUP_N_CPND0               0xe5
#define P3_WKUP_P_PND0                0xe6
#define P3_WKUP_N_PND0                0xe7
#define P3_WKUP_FLT_EN1               0xe8
#define P3_WKUP_P_IE1                 0xe9
#define P3_WKUP_N_IE1                 0xea
#define P3_WKUP_LEVEL1                0xeb
#define P3_WKUP_P_CPND1               0xec
#define P3_WKUP_N_CPND1               0xed
#define P3_WKUP_P_PND1                0xee
#define P3_WKUP_N_PND1                0xef

//............. 0x00f0 - 0x00ff............
#define P3_PORTC_PD00                 0xf0
#define P3_PORTC_PD01                 0xf1
#define P3_PORTC_PD10                 0xf2
#define P3_PORTC_PD11                 0xf3
#define P3_PORTD_PD00                 0xf4
#define P3_PORTD_PD01                 0xf5
#define P3_PORTD_PD10                 0xf6
#define P3_PORTD_PD11                 0xf7

#define P3_WKUP_CLK_SEL               0xfe
//  #define P3_AWKUP_CLK_SEL              0xff


//===============================================================================//
//
//      p33 rtcvdd
//
//===============================================================================//

#define RTC_SFR_BEGIN                     0x8000

//............. 0X0080 - 0X008F............ for RTC
#define R3_ALM_CON                    (0x80 | RTC_SFR_BEGIN)

#define R3_RTC_CON0                   (0x84 | RTC_SFR_BEGIN)
#define R3_RTC_CON1                   (0x85 | RTC_SFR_BEGIN)
#define R3_RTC_DAT0                   (0x86 | RTC_SFR_BEGIN)
#define R3_RTC_DAT1                   (0x87 | RTC_SFR_BEGIN)
#define R3_RTC_DAT2                   (0x88 | RTC_SFR_BEGIN)
#define R3_RTC_DAT3                   (0x89 | RTC_SFR_BEGIN)
#define R3_RTC_DAT4                   (0x8a | RTC_SFR_BEGIN)

//............. 0X0090 - 0X009F............ for PORT control
#define R3_WKUP_EN                    (0x90 | RTC_SFR_BEGIN)
#define R3_WKUP_EDGE                  (0x91 | RTC_SFR_BEGIN)
#define R3_WKUP_CPND                  (0x92 | RTC_SFR_BEGIN)
#define R3_WKUP_PND                   (0x93 | RTC_SFR_BEGIN)
#define R3_WKUP_FLEN                  (0x94 | RTC_SFR_BEGIN)
#define R3_PORT_FLT                   (0x95 | RTC_SFR_BEGIN)

#define R3_PR_IN                      (0x98 | RTC_SFR_BEGIN)
#define R3_PR_OUT                     (0x99 | RTC_SFR_BEGIN)
#define R3_PR_DIR                     (0x9a | RTC_SFR_BEGIN)
#define R3_PR_DIE                     (0x9b | RTC_SFR_BEGIN)
#define R3_PR_PU                      (0x9c | RTC_SFR_BEGIN)
#define R3_PR_PD                      (0x9d | RTC_SFR_BEGIN)
#define R3_PR_HD                      (0x9e | RTC_SFR_BEGIN)

//............. 0X00A0 - 0X00AF............ for system
#define R3_TIME_CON                   (0xa0 | RTC_SFR_BEGIN)
#define R3_TIME_CPND                  (0xa1 | RTC_SFR_BEGIN)
#define R3_TIME_PND                   (0xa2 | RTC_SFR_BEGIN)

#define R3_ADC_CON                    (0xa4 | RTC_SFR_BEGIN)
#define R3_OSL_CON                    (0xa5 | RTC_SFR_BEGIN)

#define R3_WKUP_SRC                   (0xa8 | RTC_SFR_BEGIN)
#define R3_RST_SRC                    (0xa9 | RTC_SFR_BEGIN)

#define R3_RST_CON                    (0xab | RTC_SFR_BEGIN)
#define R3_CLK_CON                    (0xac | RTC_SFR_BEGIN)


/* extern u8 p33_buf(u8 buf); */
/* extern void p33_xor_1byte(u16 addr, u32 data0); */
/* extern void p33_and_1byte(u16 addr, u32 data0); */
/* extern void p33_or_1byte(u16 addr, u32 data0); */
/* extern void p33_tx_1byte(u16 addr, u32 data0); */
/* extern u8 p33_rx_1byte(u16 addr); */
/* extern void P33_CON_SET(u16 addr, u8 start, u8 len, u8 data); */
/* #define P33_CON_GET     p33_rx_1byte */
/* extern void p33_rx_nbyte(u16 addr, u8 *buf_ptr, u8 len); */
/* extern void p33_tx_nbyte(u16 addr, u8 *buf_ptr, u8 len); */

#endif
//===============================================================================//
//
//      end of this file
//
//===============================================================================//


