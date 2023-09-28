#ifndef __RDAC_H__
#define __RDAC_H__
#include "typedef.h"

//=============================================================================
//=                                                                           =
//=                       Audio DAC Physics Definition                        =
//=                                                                           =
//=============================================================================
/**********DAC_CON0***************/
#define A_DAC_DEM_EN    BIT(31)
#define A_DAC_PST_OK    BIT(30)

#define A_DAC_D_LS_2    (0b00<<28)
#define A_DAC_D_LS_3    (0b01<<28)
#define A_DAC_D_LS_7    (0b10<<28)
#define A_DAC_D_LS_11   (0b11<<28)
#define A_DAC_D_LSBIT   (0b11<<28)

#define A_DAC_ACDIT_187 (0<<27)
#define A_DAC_ACDIT_375 (1<<27)
#define A_DAC_ACDIT_S   (0<<26)
#define A_DAC_ACDIT_T   (1<<26)
#define A_DAC_ACDIT_EN  (0<<26)
#define A_DAC_ACDIT_DIS (1<<26)
#define A_DAC_ACDIT_6M  (0<<26)
#define A_DAC_ACDIT_12M (1<<26)

#define A_DAC_CH0_EN    BIT(16)
#define A_DAC_SRC_EN    BIT(11)

#define A_DAC_DFIFOR    BIT(8)
#define A_DAC_PND       BIT(7)
#define A_DAC_CLR_PND   BIT(6)
#define A_DAC_IE        BIT(5)
#define A_DAC_EN        BIT(4)

//dac 输出采样率
#define DAC_SR8000      (0b1110<<0)
#define DAC_SR11025     (0b1101<<0)
#define DAC_SR12000     (0b1100<<0)
#define DAC_SR16000     (0b1010<<0)
#define DAC_SR22050     (0b1001<<0)
#define DAC_SR24000     (0b1000<<0)
#define DAC_SR32000     (0b0110<<0)
#define DAC_SR44100     (0b0101<<0)
#define DAC_SR48000     (0b0100<<0)
#define DAC_SR64000     (0b0010<<0)
#define DAC_SR88200     (0b0001<<0)
#define DAC_SR96000     (0b0000<<0)
#define DAC_SRBITS      (0b1111<<0)

#define DAC_TRACK_NUMBER      1
#define AUDAC_BIT_WIDE        16

#define DAC_CON0_DEFAULT  ( A_DAC_DEM_EN | \
                            A_DAC_D_LS_7 | \
                            A_DAC_CH0_EN | \
                            A_DAC_SRC_EN | \
                            A_DAC_IE     | \
                            A_DAC_CLR_PND )

/********* define for DAC_CON1**********************************/
#define AUDAC_2M_MODE       BIT(31)
#define AUDAC_FIFO_PND      BIT(30)
#define AUDAC_FIFO_CPND     BIT(29)
#define AUDAC_FIFO_IE       BIT(28)
#define AUDAC_RDAC_RSMP     (5<<16)
#define AUDAC_FADE_EN       BIT(15)
#define AUDAC_VOL_INV       BIT(13)
#define AUDAC_FADE_SLOW(n) ((n & 0xf) << 4)
#define AUDAC_FADE_STEP(n) ((n & 0xf) << 0)

#define DAC_CON1_DEFAULT  ( AUDAC_RDAC_RSMP )

//=============================================================================
//=                                                                           =
//=                       Audio DAC Management                                =
//=                                                                           =
//=============================================================================
#define AUDAC_SHIFT_BITS    0

typedef struct _RDAC_CTRL_HDL {
    void *buf;
    u32  con;
    u32  pns;     //dac中断门槛
    u16  sp_total;
    u16  sp_max_free;//填充数据后允许的最大Free空间，不够填零
    u8   sp_size;
} RDAC_CTRL_HDL;

void rdac_mode_init(void);
void rdac_init(u32 sr, bool delay_flag);
void rdac_sr_api(u32 sr);
void rdac_off_api(void);
void rdac_clk_init(void);
void rdac_clk_close(void);
void rdac_trim_api(void);
void audio_config_hexdump(void);
void audio_isr(void);

//libs接口
void rdac_resource_init(const RDAC_CTRL_HDL *ops);
void rdac_phy_init(u32 sr_sel);
void rdac_phy_off(void);
void rdac_phy_vol(u16 mono_vol);
u32 rdac_sr_set(u32 sr);
u32 rdac_sr_read(void);
u32 rdac_sr_lookup(u32 sr);
void audio_common_analog_open(void);
void audio_common_analog_close(void);

#endif
