#ifndef __CPU_H__
#define __CPU_H__

//DSM 模式


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
#define DAC_SRBITS   APA_SRBITS


#define APA_CLK_DEFAULT      APA_PLL240M_LOW
#define APA_MODE_DEFAULT     APA_MODE2_SNR
#define DAC_DEFAULT          (APA_IE_EN | APA_MODE_DEFAULT | APA_MODE_BTL)


#define audio_clk_init()  SFR(JL_CLOCK->CLK_CON2,  0,  2,  APA_CLK_DEFAULT)

#define SR_DEFAULT  32000


#define dac_sp_handle(n)  do {                              \
                            static signed char ii = 1;             \
                            if (ii > 0) {                   \
                                ii = -1;                    \
                            } else {                        \
                                ii = 1;                     \
                            }                               \
                            (n) += ii;                      \
                        } while(0);

#endif
