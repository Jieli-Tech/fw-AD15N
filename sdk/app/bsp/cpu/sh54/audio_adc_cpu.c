#include "config.h"
#include "audio.h"
#include "audio_adc.h"
#include "audio_analog.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[auadc_cpu]"
#include "log.h"

#define ADC_PACKET_SIZE 32
u8 double_audio_adc_buf[ADC_PACKET_SIZE * 2 * 2] AT(.AUDIO_ADC_BUFFER);

void auin_mode_init(void)
{
    audio_adc_resource_init(double_audio_adc_buf, sizeof(double_audio_adc_buf), 25);
}

u32 auin_init(u32 sr, ADC_MODE mode, u32 ch)
{
    u32 res = 0;

    if (ADC_MIC == mode) {
        JL_PORTB->DIR |= BIT(1);
        JL_PORTB->DIE &= ~BIT(1);
        JL_PORTB->DIEH &= ~BIT(1);
        JL_PORTB->PU &= ~BIT(1);
        JL_PORTB->PD &= ~BIT(1);
    } else if (ADC_LINE_IN == mode) {
        if (BIT(0) & ch) {
            JL_PORTA->DIR |= BIT(13);
            JL_PORTA->DIE &= ~BIT(13);
            JL_PORTA->DIEH &= ~BIT(13);
            JL_PORTA->PU &= ~BIT(13);
            JL_PORTA->PD &= ~BIT(13);

        }
        if (BIT(1) & ch) {
            JL_PORTA->DIR |= BIT(14);
            JL_PORTA->DIE &= ~BIT(14);
            JL_PORTA->DIEH &= ~BIT(14);
            JL_PORTA->PU &= ~BIT(14);
            JL_PORTA->PD &= ~BIT(14);

        }
    }
    res = adc_analog_open(mode, ch);
    /* void dac_analog_init_t(); */
    /* dac_analog_init_t(); */
    if (0 == res) {
        res = audio_adc_phy_init(sr);
    }
    return res;
}

void auin_off_api(void)
{
    audio_adc_phy_off();
    adc_analog_close();
}
