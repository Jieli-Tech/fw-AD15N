#ifndef __APA_H__
#define __APA_H__

#include "typedef.h"

//=============================================================================
//=                                                                           =
//=                       Audio APA Physics Definition                        =
//=                                                                           =
//=============================================================================
//APA 模式
#define APA_PLL320M          1
#define APA_PLL240M_LOW      2
#define APA_PLL240M_HIGH     3
// #define APA_HSBCLK           3

#define APA_MODE_BTL         (0 << 11)
#define APA_MODE_PBTL        (1 << 11)

#define APA_MODE1_NOR         (0 << 9)
#define APA_MODE2_SNR         (1 << 9)
#define APA_MODE3_THD         (2 << 9)//暂不支持
#define APA_MODE_BITS         (3 << 9)

#define APA_MUTE             (1 << 8)

#define APA_PND              (1 << 7)
#define APA_CPND             (1 << 6)
#define APA_IE_EN            (1 << 5)
#define APA_PWM_EN           (1 << 4)
#define APA_DSM_EN           (1 << 3)

//apa 输出采样率
#define APA_SR48K    (0<<0)
#define APA_SR44K1   (1<<0)
#define APA_SR32K    (2<<0)
#define APA_SRBITS   (7<<0)


#define APA_CLK_DEFAULT      APA_PLL240M_LOW
#define APA_MODE_DEFAULT     APA_MODE2_SNR
#define APA_DEFAULT          (APA_IE_EN | APA_MODE_DEFAULT | APA_MODE_BTL)

#define apa_sp_handle(n)  do {                              \
                            static signed char ii = 1;             \
                            if (ii > 0) {                   \
                                ii = -1;                    \
                            } else {                        \
                                ii = 1;                     \
                            }                               \
                            (n) += ii;                      \
                        } while(0);

//=============================================================================
//=                                                                           =
//=                       Audio APA Management                                =
//=                                                                           =
//=============================================================================

typedef struct _APA_CTRL_HDL {
    void *buf0;
    void *buf1;
    u16 sp_len;
    u16 con0;
} APA_CTRL_HDL;

void apa_mode_init(void);
void apa_init(u32 sr, bool delay_flag);
void apa_sr_api(u32 sr);
void apa_off_api(void);
void test_audio_apa(void);

// apa库接口
void apa_isr_t(void);
void apa_resource_init(u8 *buff, u32 len, u32 con0, u32 con1);
void apa_phy_init(u32 sr_sel);
void apa_phy_off(void);
void apa_analog_pre_open(void);
u32 apa_sr_lookup(u32 sr);
u32 apa_sr_read(void);
u32 apa_sr_set(u32 sr);
#endif
