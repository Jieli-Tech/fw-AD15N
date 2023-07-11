
#ifndef __SH57__
#define __SH57__

//===============================================================================//
//
//      sfr define
//
//===============================================================================//

#define hs_base            0x3ee0000
#define ls_base            0x3ed0000

#define __RW               volatile       // read write
#define __RO               volatile const // only read
#define __WO               volatile       // only write

#define __u8               unsigned int   // u8  to u32 special for struct
#define __u16              unsigned int   // u16 to u32 special for struct
#define __u32              unsigned int

#define __s8(x)            char(x); char(reserved_1_##x); char(reserved_2_##x); char(reserved_3_##x)
#define __s16(x)           short(x); short(reserved_1_##x)
#define __s32(x)           int(x)

#define map_adr(grp, adr)  ((64 * grp + adr) * 4)     // grp(0x0-0xff), adr(0x0-0x3f)

//===============================================================================//
//
//      high speed sfr address define
//
//===============================================================================//

//............. 0x0000 - 0x00ff............ for hemu
typedef struct {
    __RW __u32 WREN;
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
    __RW __u32 MSG0;
    __RW __u32 MSG1;
    __RW __u32 MSG2;
    __RW __u32 MSG3;
    __RO __u32 ID;
} JL_HEMU_TypeDef;

#define JL_HEMU_BASE                    (hs_base + map_adr(0x00, 0x00))
#define JL_HEMU                         ((JL_HEMU_TypeDef    *)JL_HEMU_BASE)

//............. 0x0100 - 0x02ff............ for sfc
typedef struct {
    __RW __u32 CON;
    __RW __u32 BAUD;
    __RW __u32 REDU_BAUD;
    __RW __u32 CODE;
    __RW __u32 BASE_ADR;
    __RW __u32 QUCNT;
    __RW __u8  ENC_CON;
    __RW __u16 ENC_KEY;
    __WO __u32 UNENC_ADRH;
    __WO __u32 UNENC_ADRL;
} JL_SFC_TypeDef;

#define JL_SFC_BASE                     (hs_base + map_adr(0x02, 0x00))
#define JL_SFC                          ((JL_SFC_TypeDef    *)JL_SFC_BASE)

//............. 0x0300 - 0x06ff............

//............. 0x0700 - 0x07ff............
//typedef struct {
//    __RW __u32 CON;
//    __RW __u32 SEL;
//    __RW __u32 BEG;
//    __RW __u32 END;
//    __RW __u32 DAT_VLD0;
//    __RW __u32 DAT_VLD1;
//    __RW __u32 DAT_VLD2;
//    __RW __u32 DAT_VLD3;
//} JL_HMBIST_TypeDef;

//#define JL_HMBIST_BASE                  (hs_base + map_adr(0x07, 0x00))
//#define JL_HMBIST                       ((JL_HMBIST_TypeDef *)JL_HMBIST_BASE)

//............. 0x0800 - 0x08ff............ for audio_top
typedef struct {
    __RW __u32(APA_LEN);
    __RW __u32(APA_ADR);
    __RW __u32(APA_CON0);
    __RW __u32(APA_CON1);
    __RW __u32(APA_CON2);
    __RW __u32(APA_CON3);
    __RO __u32(APA_CON4);
} JL_APA_TypeDef;

#define JL_APA_BASE                   (hs_base + map_adr(0x08, 0x00))
#define JL_APA                        ((JL_APA_TypeDef   *)JL_APA_BASE)

//............. 0x0900 - 0x09ff............ for ass_clkgen

//............. 0x0a00 - 0x0aff............ for audio_ana

//............. 0x0b00 - 0x0bff............ for ass_mbist

//............. 0x0c00 - 0x0cff............ for eq
//typedef struct {
//    __RW __u32 CON0;
//    __RW __u32 CON1;
//    __RW __u32 CON2;
//    __RW __u32 CON3;
//    __RW __u32 DATAI_ADR;
//    __RW __u32 DATAO_ADR;
//    __RW __u32 DATA_LEN;
//    __RW __u32 FLT_ADR;
//} JL_EQ_TypeDef;

//#define JL_EQ_BASE                      (hs_base + map_adr(0x0c, 0x00))
//#define JL_EQ                           ((JL_EQ_TypeDef			*)JL_EQ_BASE)

//............. 0x0d00 - 0x0dff............ for src
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
    __RW __u32 IDAT_ADR;
    __RW __u32 IDAT_LEN;
    __RW __u32 ODAT_ADR;
    __RW __u32 ODAT_LEN;
    __RW __u32 FLTB_ADR;
    __WO __u32 ODAT_ADR_START;
    __WO __u32 ODAT_ADR_END;
    __WO __u32 STOP_FLAG;
    __RW __u32 INSR;
    __RW __u32 OUTSR;
    __RW __u32 PHASE;
    __RW __u32 COEF;
} JL_SRC_TypeDef;

#define JL_SRC_BASE                     (hs_base + map_adr(0x0d, 0x00))
#define JL_SRC                          ((JL_SRC_TypeDef			*)JL_SRC_BASE)

//===============================================================================//
//
//      low speed sfr address define
//
//===============================================================================//

//............. 0x0000 - 0x00ff............
typedef struct {
    __RW __u32 WREN;
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
    __RW __u32 MSG0;
    __RW __u32 MSG1;
    __RW __u32 MSG2;
    __RW __u32 MSG3;
    __RO __u32 ID;
} JL_LEMU_TypeDef;

#define JL_LEMU_BASE                    (ls_base + map_adr(0x00, 0x00))
#define JL_LEMU                         ((JL_LEMU_TypeDef    *)JL_LEMU_BASE)

//............. 0x0100 - 0x01ff............ for pmu_ctl
typedef struct {
    __RW __u32 PMU_CON;
    __RW __u32 SPI_CON;
    __RW __u32 SPI_DAT;
} JL_PMU_TypeDef;

#define JL_PMU_BASE                     (ls_base + map_adr(0x01, 0x00))
#define JL_PMU                          ((JL_PMU_TypeDef        *)JL_PMU_BASE)

//............. 0x0200 - 0x02ff............
typedef struct {
    __RW __u32 MODE_CON;
} JL_MODE_TypeDef;

#define JL_MODE_BASE                    (ls_base + map_adr(0x02, 0x00))
#define JL_MODE                         ((JL_MODE_TypeDef     *)JL_MODE_BASE)

//............. 0x0300 - 0x03ff............ for syscfg
typedef struct {
    __RW __u32 RST_SRC;
} JL_RST_TypeDef;

#define JL_RST_BASE                     (ls_base + map_adr(0x03, 0x00))
#define JL_RST                          ((JL_RST_TypeDef        *)JL_RST_BASE)

typedef struct {
    __RW __u32 PWR_CON;
    __RW __u32 SYS_SEL;
    __RW __u32 SYS_DIV;
    __RW __u32 CLK_CON0;
    __RW __u32 CLK_CON1;
    __RW __u32 CLK_CON2;
    __RW __u32 CLK_CON3;
} JL_CLOCK_TypeDef;

#define JL_CLOCK_BASE                   (ls_base + map_adr(0x03, 0x01))
#define JL_CLOCK                        ((JL_CLOCK_TypeDef      *)JL_CLOCK_BASE)

//............. 0x0400 - 0x04ff............ for pll0_ctl
typedef struct {
    __RW __u32 PLL_CON0;
    __RW __u32 PLL_CON1;
    __RW __u32 PLL_NR;
    __RW __u32 TRIM_CON0;
    __RW __u32 TRIM_CON1;
    __RW __u32 TRIM_PND;
    __RW __u32 FRQ_CNT;
    __RW __u32 FRC_SCA;
} JL_PLL0_TypeDef;

#define JL_PLL0_BASE                    (ls_base + map_adr(0x04, 0x00))
#define JL_PLL0                         ((JL_PLL0_TypeDef      *)JL_PLL0_BASE)

//............. 0x0500 - 0x05ff............
typedef struct {
    __RW __u32 CON;
    __RW __u32 CNT;
    __RW __u32 PRD;
    __RW __u32 PWM;
} JL_TIMER_TypeDef;

#define JL_TIMER0_BASE                  (ls_base + map_adr(0x05, 4*0))
#define JL_TIMER1_BASE                  (ls_base + map_adr(0x05, 4*1))
#define JL_TIMER2_BASE                  (ls_base + map_adr(0x05, 4*2))

#define JL_TIMER0                       ((JL_TIMER_TypeDef     *)JL_TIMER0_BASE)
#define JL_TIMER1                       ((JL_TIMER_TypeDef     *)JL_TIMER1_BASE)
#define JL_TIMER2                       ((JL_TIMER_TypeDef     *)JL_TIMER2_BASE)

//............. 0x0600 - 0x06ff............

//............. 0x0700 - 0x07ff............

//............. 0x0800 - 0x09ff............
typedef struct {
    __RW __u16 CON0;
    __RW __u16 CON1;
    __RW __u16 CON2;
    __RW __u16 BAUD;
    __RW __u8  BUF;
} JL_UART_LITE_TypeDef;

#define JL_UART0_BASE                   (ls_base + map_adr(0x08, 0x00))
#define JL_UART0                        ((JL_UART_LITE_TypeDef       *)JL_UART0_BASE)

typedef struct {
    __RW __u16 CON0;
    __RW __u16 CON1;
    __RW __u16 CON2;
    __RW __u16 BAUD;
    __RW __u8  BUF;
    __RW __u32 OTCNT;
    __RW __u32 TXADR;
    __WO __u16 TXCNT;
    __RW __u32 RXSADR;
    __RW __u32 RXEADR;
    __RW __u32 RXCNT;
    __RO __u16 HRXCNT;
    __RO __u16 RX_ERR_CNT;
} JL_UART_TypeDef;

#define JL_UART1_BASE                   (ls_base + map_adr(0x09, 0x00))
#define JL_UART1                        ((JL_UART_TypeDef       *)JL_UART1_BASE)

//............. 0x0a00 - 0x0bff............
typedef struct {
    __RW __u32 CON;
    __RW __u32 BAUD;
    __RW __u32 BUF;
    __WO __u32 ADR;
    __RW __u32 CNT;
    __RW __u32 CON1;
} JL_SPI_TypeDef;

#define JL_SPI0_BASE                    (ls_base + map_adr(0x0a, 0x00))
#define JL_SPI0                         ((JL_SPI_TypeDef      *)JL_SPI0_BASE)

#define JL_SPI1_BASE                    (ls_base + map_adr(0x0b, 0x00))
#define JL_SPI1                         ((JL_SPI_TypeDef      *)JL_SPI1_BASE)

//............. 0x0c00 - 0x0cff............
typedef struct {
    __RW __u32 CON    ;
    __RW __u32 TASK   ;
    __RW __u32 PND    ;
    __RW __u32 TX_BUF ;
    __RO __u32 RX_BUF ;
    __RW __u32 ADDR   ;
    __RW __u32 BAUD   ;
    __RW __u32 TSU    ;
    __RW __u32 THD    ;
    __RO __u32 DBG    ;
} JL_IIC_TypeDef;

#define JL_IIC0_BASE                     (ls_base + map_adr(0x0c, 0x00))
#define JL_IIC0                          ((JL_IIC_TypeDef       *)JL_IIC0_BASE)

//............. 0x0d00 - 0x0dff............
//typedef struct {
//    __RW __u32 CON0;
//    __RW __u32 CON1;
//    __RW __u32 CON2;
//    __WO __u32 CPTR;
//    __WO __u32 DPTR;
//    __RW __u32 CTU_CON;
//    __WO __u32 CTU_CNT;
//} JL_SD_TypeDef;

//#define JL_SD0_BASE                     (ls_base + map_adr(0x0d, 0x00))
//#define JL_SD0                          ((JL_SD_TypeDef        *)JL_SD0_BASE)

//............. 0x0e00 - 0x0eff............
typedef struct {
    __RW __u32 CON;
    __RO __u32 RES;
    __RW __u32 DMA_CON0;
    __RW __u32 DMA_CON1;
    __RW __u32 DMA_CON2;
    __RW __u32 DMA_BADR;
} JL_ADC_TypeDef;

#define JL_ADC_BASE                     (ls_base + map_adr(0x0e, 0x00))
#define JL_ADC                          ((JL_ADC_TypeDef       *)JL_ADC_BASE)

//............. 0x0f00 - 0x0fff............
typedef struct {
    __RW __u32 RFLT_CON;
} JL_IR_TypeDef;

#define JL_IR_BASE                      (ls_base + map_adr(0x0f, 0x00))
#define JL_IR                           ((JL_IR_TypeDef         *)JL_IR_BASE)

//............. 0x1000 - 0x10ff............
typedef struct {
    __RW __u32 CON;
    __RW __u32 VAL;
} JL_PCNT_TypeDef;

#define JL_PCNT_BASE                    (ls_base + map_adr(0x10, 0x00))
#define JL_PCNT                         ((JL_PCNT_TypeDef       *)JL_PCNT_BASE)

//............. 0x1100 - 0x11ff............
typedef struct {
    __RW __u32 CON;
    __RO __u32 NUM;
} JL_GPCNT_TypeDef;

#define JL_GPCNT_BASE                   (ls_base + map_adr(0x11, 0x00))
#define JL_GPCNT                        ((JL_GPCNT_TypeDef     *)JL_GPCNT_BASE)

//............. 0x1200 - 0x12ff............
typedef struct {
    __WO __u32 CON;
    __RW __u32 NUM;
} JL_LRCT_TypeDef;

#define JL_LRCT_BASE                    (ls_base + map_adr(0x12, 0x00))
#define JL_LRCT                         ((JL_LRCT_TypeDef     *)JL_LRCT_BASE)

//............. 0x1300 - 0x13ff............
typedef struct {
    __RW __u32 CON;
} JL_OSA_TypeDef;

#define JL_OSA_BASE                     (ls_base + map_adr(0x13, 0x00))
#define JL_OSA                          ((JL_OSA_TypeDef          *)JL_OSA_BASE)

//............. 0x1400 - 0x14ff............
typedef struct {
    __WO __u32 FIFO;
    __RW __u32 REG;
} JL_CRC_TypeDef;

#define JL_CRC_BASE                     (ls_base + map_adr(0x14, 0x00))
#define JL_CRC                          ((JL_CRC_TypeDef       *)JL_CRC_BASE)

//............. 0x1500 - 0x15ff............ for lsb peri(spi0/sd0)
typedef struct {
    __RW __u8  ENCCON ;
    __WO __u16 ENCKEY ;
    __WO __u16 ENCADR ;
} JL_PERIENC_TypeDef;

#define JL_PERIENC_BASE                 (ls_base + map_adr(0x15, 0x00))
#define JL_PERIENC                      ((JL_PERIENC_TypeDef *)JL_PERIENC_BASE)

//............. 0x1600 - 0x16ff............
typedef struct {
    __RO __u32 R64L;
    __RO __u32 R64H;
} JL_RAND_TypeDef;

#define JL_RAND_BASE                    (ls_base + map_adr(0x16, 0x00))
#define JL_RAND                         ((JL_RAND_TypeDef   *)JL_RAND_BASE)

//............. 0x1700 - 0x17ff............
typedef struct {
    __RW __u32 TMR0_CON;
    __RW __u32 TMR0_CNT;
    __RW __u32 TMR0_PR;
    __RW __u32 TMR1_CON;
    __RW __u32 TMR1_CNT;
    __RW __u32 TMR1_PR;
    __RW __u32 TMR2_CON;
    __RW __u32 TMR2_CNT;
    __RW __u32 TMR2_PR;
    __RW __u32 TMR3_CON;
    __RW __u32 TMR3_CNT;
    __RW __u32 TMR3_PR;
    __RW __u32 TMR4_CON;
    __RW __u32 TMR4_CNT;
    __RW __u32 TMR4_PR;
    __RW __u32 TMR5_CON;
    __RW __u32 TMR5_CNT;
    __RW __u32 TMR5_PR;
    __RW __u32 TMR6_CON;
    __RW __u32 TMR6_CNT;
    __RW __u32 TMR6_PR;
    __RW __u32 TMR7_CON;
    __RW __u32 TMR7_CNT;
    __RW __u32 TMR7_PR;
    __RW __u32 FPIN_CON;
    __RW __u32 CH0_CON0;
    __RW __u32 CH0_CON1;
    __RW __u32 CH0_CMPH;
    __RW __u32 CH0_CMPL;
    __RW __u32 CH1_CON0;
    __RW __u32 CH1_CON1;
    __RW __u32 CH1_CMPH;
    __RW __u32 CH1_CMPL;
    __RW __u32 CH2_CON0;
    __RW __u32 CH2_CON1;
    __RW __u32 CH2_CMPH;
    __RW __u32 CH2_CMPL;
    __RW __u32 CH3_CON0;
    __RW __u32 CH3_CON1;
    __RW __u32 CH3_CMPH;
    __RW __u32 CH3_CMPL;
    __RW __u32 CH4_CON0;
    __RW __u32 CH4_CON1;
    __RW __u32 CH4_CMPH;
    __RW __u32 CH4_CMPL;
    __RW __u32 CH5_CON0;
    __RW __u32 CH5_CON1;
    __RW __u32 CH5_CMPH;
    __RW __u32 CH5_CMPL;
    __RW __u32 CH6_CON0;
    __RW __u32 CH6_CON1;
    __RW __u32 CH6_CMPH;
    __RW __u32 CH6_CMPL;
    __RW __u32 CH7_CON0;
    __RW __u32 CH7_CON1;
    __RW __u32 CH7_CMPH;
    __RW __u32 CH7_CMPL;
    __RW __u32 MCPWM_CON0;
} JL_MCPWM_TypeDef;

#define JL_MCPWM_BASE                   (ls_base + map_adr(0x17, 0x00))
#define JL_MCPWM                        ((JL_MCPWM_TypeDef     *)JL_MCPWM_BASE)

//............. 0x1800 - 0x18ff............
//typedef struct {
//    __RW __u32 CLK;
//} JL_LEDCK_TypeDef;
//#define JL_LEDCK_BASE                   (ls_base + map_adr(0x18,0x00))
//#define JL_LEDCK                        ((JL_LEDCK_TypeDef     *)JL_LEDCK_BASE)

//typedef struct {
//    __RW __u32 CON;
//    __RW __u32 FD;
//    __RW __u32 LP;
//    __RW __u32 TIX;
//    __RW __u32 RSTX;
//    __WO __u32 ADR;
//} JL_LEDC_TypeDef;
//#define JL_LEDC_BASE                    (ls_base + map_adr(0x18, 0x01))
//#define JL_LEDC                         ((JL_LEDC_TypeDef      *)JL_LEDC_BASE)

//............. 0x1900 - 0x19ff............
//typedef struct {
//    __RW __u8  CON;
//    __RW __u8  DAT;
//    __RW __u8  SMP;
//} JL_RDEC_TypeDef;

//#define JL_RDEC0_BASE                   (ls_base + map_adr(0x19, 0x00))
//#define JL_RDEC0                        ((JL_RDEC_TypeDef       *)JL_RDEC0_BASE)

//............. 0x1a00 - 0x1aff............ for led
//typedef struct {
//    __RW __u32 CON0;
//    __RW __u32 CON1;
//    __RW __u32 CON2;
//    __RW __u32 CON3;
//    __RW __u32 BRI_PRDL;
//    __RW __u32 BRI_PRDH;
//    __RW __u32 BRI_DUTY0L;
//    __RW __u32 BRI_DUTY0H;
//    __RW __u32 BRI_DUTY1L;
//    __RW __u32 BRI_DUTY1H;
//    __RW __u32 PRD_DIVL;
//    __RW __u32 DUTY0;
//    __RW __u32 DUTY1;
//    __RW __u32 DUTY2;
//    __RW __u32 DUTY3;
//    __RO __u32 CNT_RD;
//    __RW __u32 CON4;
//    __WO __u32 CNT_SYNC;
//    __RW __u32 CNT_DEC;
//} JL_PLED_TypeDef;

//#define JL_PLED_BASE                    (ls_base + map_adr(0x1a, 0x00))
//#define JL_PLED                         ((JL_PLED_TypeDef      *)JL_PLED_BASE)

//............. 0x1b00 - 0x1bff............ for lcd
//typedef struct {
//    __RW __u32 CON0;
//    __RW __u32 CON1;
//    __RW __u32 SEG_IO_EN0;
//    __RW __u32 SEG_IO_EN1;
//    __RW __u32 SEG0_DAT;
//    __RW __u32 SEG1_DAT;
//    __RW __u32 SEG2_DAT;
//    __RW __u32 SEG3_DAT;
//    __RW __u32 SEG4_DAT;
//    __RW __u32 SEG5_DAT;
//} JL_LCDC_TypeDef;

//#define JL_LCDC_BASE                    (ls_base + map_adr(0x1b, 0x00))
//#define JL_LCDC                         ((JL_LCDC_TypeDef      *)JL_LCDC_BASE)

//............. 0x1c00 - 0x1cff............ for pps
//typedef struct {
//    __RW __u32 CON;
//    __RW __u32 CNT;
//    __RW __u32 CFG;
//    __RW __u32 VTRIM;
//    __RW __u32 TX_BASE;
//    __RW __u32 RX_BASE;
//} JL_PPS_TypeDef;
//
//#define JL_PPS_BASE                     (ls_base + map_adr(0x1c, 0x00))
//#define JL_PPS                          ((JL_PPS_TypeDef       *)JL_PPS_BASE)

//............. 0x1d00 - 0x1dff............ for ppm
//typedef struct {
//    __RW __u32 CON;
//    __RW __u32 STRAN_DAT;
//    __RW __u32 STRAN_SET;
//    __RW __u32 DMA_ADR;
//    __RW __u32 PPM_RD;
// } JL_PPM_TypeDef;

// #define JL_PPM_BASE                    (ls_base + map_adr(0x1d, 0x00))
// #define JL_PPM                         ((JL_PPM_TypeDef       *)JL_PPM_BASE)

//............. 0x2000 - 0x24ff............ for sie
//typedef struct {
//    __RW __u32 FADDR;
//    __RW __u32 POWER;
//    __RO __u32 INTRTX1;
//    __RO __u32 INTRTX2;
//    __RO __u32 INTRRX1;
//    __RO __u32 INTRRX2;
//    __RO __u32 INTRUSB;
//    __RW __u32 INTRTX1E;
//    __RW __u32 INTRTX2E;
//    __RW __u32 INTRRX1E;
//    __RW __u32 INTRRX2E;
//    __RW __u32 INTRUSBE;
//    __RO __u32 FRAME1;
//    __RO __u32 FRAME2;
//    __RO __u32 RESERVED14;
//    __RW __u32 DEVCTL;
//    __RO __u32 RESERVED10_0x16[0x16-0x10+1];
//} JL_USB_SIE_TypeDef;

//#define JL_USB_SIE_BASE                 (ls_base + map_adr(0x20, 0x00))
//#define JL_USB_SIE                      ((JL_USB_SIE_TypeDef *)JL_USB_SIE_BASE)

//typedef struct {
//    __RO __u32 RESERVED0;
//    __RW __u32 CSR0;
//    __RO __u32 RESERVED2_5[5-1];
//    __RO __u32 COUNT0;
//
//} JL_USB_EP0_TypeDef;

//#define JL_USB_EP0_BASE                 (ls_base + map_adr(0x20, 0x10))
//#define JL_USB_EP0                      ((JL_USB_EP0_TypeDef *)JL_USB_EP0_BASE)

//typedef struct {
//    __RW __u32 TXMAXP;
//    __RW __u32 TXCSR1;
//    __RW __u32 TXCSR2;
//    __RW __u32 RXMAXP;
//    __RW __u32 RXCSR1;
//    __RW __u32 RXCSR2;
//    __RO __u32 RXCOUNT1;
//    __RO __u32 RXCOUNT2;
//    __RW __u32 TXTYPE;
//    __RO __u32 TXINTERVAL;
//    __RW __u32 RXTYPE;
//    __RO __u32 RXINTERVAL;

//} JL_USB_EP_TypeDef;

//#define JL_USB_EP1_BASE                 (ls_base + map_adr(0x21, 0x10))
//#define JL_USB_EP1                      ((JL_USB_EP_TypeDef *)JL_USB_EP1_BASE)
//
//#define JL_USB_EP2_BASE                 (ls_base + map_adr(0x22, 0x10))
//#define JL_USB_EP2                      ((JL_USB_EP_TypeDef *)JL_USB_EP2_BASE)
//
//#define JL_USB_EP3_BASE                 (ls_base + map_adr(0x23, 0x10))
//#define JL_USB_EP3                      ((JL_USB_EP_TypeDef *)JL_USB_EP3_BASE)

//#define JL_USB_EP4_BASE                 (ls_base + map_adr(0x24, 0x10))
//#define JL_USB_EP4                      ((JL_USB_EP_TypeDef *)JL_USB_EP4_BASE)

//............. 0x2500 - 0x25ff............
//typedef struct {
//    __RW __u32 CON0;
//    __RW __u32 CON1;
//    __WO __u32 EP0_CNT;
//    __WO __u32 EP1_CNT;
//    __WO __u32 EP2_CNT;
//    __WO __u32 EP3_CNT;
//    __WO __u32 EP4_CNT;
//    __WO __u32 EP0_ADR;
//    __WO __u32 EP1_TADR;
//    __WO __u32 EP1_RADR;
//    __WO __u32 EP2_TADR;
//    __WO __u32 EP2_RADR;
//    __WO __u32 EP3_TADR;
//    __WO __u32 EP3_RADR;
//    __WO __u32 EP4_TADR;
//    __WO __u32 EP4_RADR;
//    __RW __u32 TXDLY_CON;
//    __RW __u32 EP1_RLEN;
//    __RW __u32 EP2_RLEN;
//    __RW __u32 EP3_RLEN;
//    __RW __u32 EP4_RLEN;
//    __RW __u32 EP1_MTX_PRD;
//    __RW __u32 EP1_MRX_PRD;
//    __RO __u32 EP1_MTX_NUM;
//    __RO __u32 EP1_MRX_NUM;
//} JL_USB_TypeDef;

//#define JL_USB_BASE                     (ls_base + map_adr(0x25, 0x00))
//#define JL_USB                          ((JL_USB_TypeDef       *)JL_USB_BASE)

//............. 0x3000 - 0x34ff............ for port
typedef struct {
    __RO __u32 IN;
    __RW __u32 OUT;
    __RW __u32 DIR;
    __RW __u32 DIE;
    __RW __u32 DIEH;
    __RW __u32 PU0;
    __RW __u32 PD0;
    __RW __u32 HD0;
    __RW __u32 HD1;
    __RW __u32 SPL;
} JL_PORT_TypeDef;

#define JL_PORTA_BASE                   (ls_base + map_adr(0x30, 0x00))
#define JL_PORTA                        ((JL_PORT_TypeDef *)JL_PORTA_BASE)

#define JL_PORTB_BASE                   (ls_base + map_adr(0x31, 0x00))
#define JL_PORTB                        ((JL_PORT_TypeDef *)JL_PORTB_BASE)

#define JL_PORTD_BASE                   (ls_base + map_adr(0x33, 0x00))
#define JL_PORTD                        ((JL_PORT_TypeDef *)JL_PORTD_BASE)

//............. 0x3d00 - 0x3dff............ for port others
//typedef struct {
//    __RW __u32 CON0;
//} JL_USB_IO_TypeDef;

//#define JL_USB_IO_BASE                  (ls_base + map_adr(0x3d, 0x00))
//#define JL_USB_IO                       ((JL_USB_IO_TypeDef   *)JL_USB_IO_BASE)

typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
} JL_WAKEUP_TypeDef;

#define JL_WAKEUP_BASE                  (ls_base + map_adr(0x3d, 0x01))
#define JL_WAKEUP                       ((JL_WAKEUP_TypeDef   *)JL_WAKEUP_BASE)

typedef struct {
    __RW __u32 FSPG_CON;
} JL_FSPG_TypeDef;

#define JL_FSPG_BASE                    (ls_base + map_adr(0x3d, 0x05))
#define JL_FSPG                         ((JL_FSPG_TypeDef     *)JL_FSPG_BASE)

typedef struct {
    __RW __u32 IOMC0;
    __RW __u32 OCH_CON0;
    __RW __u32 ICH_CON0;
    __RW __u32 ICH_CON1;
} JL_IOMC_TypeDef;

#define JL_IOMC_BASE                    (ls_base + map_adr(0x3d, 0x06))
#define JL_IOMC                         ((JL_IOMC_TypeDef     *)JL_IOMC_BASE)

//............. 0xxx00 - 0xxxff............ TODO
//typedef struct {
//    __WO __u32 CHIP_ID;
//} JL_SYSTEM_TypeDef;
//
//#define JL_SYSTEM_BASE                  (ls_base + map_adr(0xxx, 0x00))
//#define JL_SYSTEM                       ((JL_SYSTEM_TypeDef   *)JL_SYSTEM_BASE)

#include "io_omap.h"
#include "io_imap.h"

#endif

