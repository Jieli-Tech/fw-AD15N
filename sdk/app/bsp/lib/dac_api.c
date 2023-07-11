/***********************************Jieli tech************************************************
  File : dac_api.c
  By   : liujie
  Email: liujie@zh-jieli.com
  date : 2019-1-14
********************************************************************************************/
#include "dac_api.h"
#include "dac.h"
#include "circular_buf.h"
#include "string.h"
#include "uart.h"
#include "config.h"
#include "audio.h"
/* #include "decoder_api.h" */
#include "decoder_mge.h"
#include "sound_effect_api.h"
#include "audio_analog.h"
#include "mio_api.h"
#include "sine_play.h"
#include "app_config.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"



typedef struct _DAC_MANAGE {
    // void *obuf[DAC_CHANNEL_NUMBER];
    sound_out_obj *sound[DAC_CHANNEL_NUMBER];
    void (*kick[DAC_CHANNEL_NUMBER])(void *);
    /* u8 vol[DAC_CHANNEL_NUMBER]; */
    u8 ch;
    u16 vol_phy;
    u8 vol;
    u8 flag;
} DAC_MANAGE;

#define B_DAC_MUTE		BIT(0)
#define B_DAC_FADE_EN   BIT(1)
#define B_DAC_FADE_OUT  BIT(2)


const u16 vol_tab[] = {
    0	,
    164	,
    191	,
    223	,
    260	,
    303	,
    353	,
    412	,
    480	,
    559	,
    652	,
    760	,
    887	,
    1034	,
    1205	,
    1405	,
    1638	,
    1910	,
    2227	,
    2597	,
    3028	,
    3530	,
    4115	,
    4798	,
    5594	,
    6523	,
    7605	,
    8867	,
    10338	,
    12053	,
    14052	,
    16383
};
#define MAX_VOL_LEVEL ((sizeof(vol_tab)/2) - 1)
#define MAX_PHY_VOL   vol_tab[MAX_VOL_LEVEL]

DAC_MANAGE dac_mge AT(.DAC_BUFFER);
u16 double_dac_buf[DAC_PACKET_SIZE * 2] AT(.DAC_BUFFER);
/* s16 sp_dac_buf[DAC_PACKET_SIZE] AT(.DAC_BUFFER); */
/*----------------------------------------------------------------------------*/
/**@brief   dac模块初始化
   @param   无
   @return  无
   @author  liujie
   @note    void dac_mode_init(u16 vol)
*/
/*----------------------------------------------------------------------------*/
void dac_mode_init(u16 vol)
{
    memset(&dac_mge, 0, sizeof(dac_mge));
#if DAC_FADE_ENABLE
    dac_mge.vol = vol;
    if (dac_mge.vol > MAX_VOL_LEVEL) {
        dac_mge.vol = MAX_VOL_LEVEL;
    }
#else
    dac_vol(0, vol);
#endif

    memset(&double_dac_buf[0], 0, sizeof(double_dac_buf));
    u32 con = dac_mode_check(DAC_DEFAULT);
    dac_resource_init((u8 *)&double_dac_buf[0], sizeof(double_dac_buf), con, 0);
}

//1:有延时 0:没有延时, 上电开机调用需要延时至少1.2ms
void dac_init_api(u32 sr, bool delay_flag)
{
    dac_phy_init(dac_sr_lookup(sr));
    /* delay_10ms(2); */
    /* delay(5000); */
    if (delay_flag) {
        udelay(1000);//约1.2ms
    }
    dac_cpu_mode();
}

void dac_sr_api(u32 sr)
{
    /* u32 dac_sr_set(u32 sr) */
    dac_sr_set(dac_sr_lookup(sr));
    /* dac_analog_init(); */
}

void dac_off_api(void)
{
    rdac_analog_close();
    apa_analog_close();
    dac_phy_off();
}
/* void dac_sr_api(u32 sr) */
/* { */
/* dac_sr_set(dac_sr_lookup(sr)); */
/* } */
SEC(.audio_d.text.cache.L2)
bool dac_cbuff_active(void *sound_hld)
{
    sound_out_obj *psound = sound_hld;
    if (psound->enable & (B_DEC_PAUSE | B_DEC_FIRST)) {
        if (cbuf_get_data_size(psound->p_obuf) >= (cbuf_get_space(psound->p_obuf) / 2)) {
            psound->enable &= ~B_DEC_FIRST;
        }
        return false;
    } else {
        return true;
    }
}

AT(.audio_d.text.cache.L2)
void fill_dac_fill_phy(u8 *buf, u32 len)
{
    u32 i, sp_cnt, active_flag;
    s32 t_sp;
    s16 r_sp;
    s16 *sp_buf;
    s16 *rptr[DAC_CHANNEL_NUMBER];
    u32 olen[DAC_CHANNEL_NUMBER];
    u32 p_cnt[DAC_CHANNEL_NUMBER];
    u32 sp_number = len / 2;
    memset(p_cnt, 0, sizeof(p_cnt));
    memset(rptr,  0, sizeof(rptr));
    memset(olen,  0, sizeof(olen));
    memset(buf,   0, len);

    d_key_voice_read(buf, len);

    sp_buf = (void *)buf;
    /* log_info("a %d\n",sp_number); */

    active_flag = 0;
    for (i = 0; i < DAC_CHANNEL_NUMBER; i++) {
        if (0 == (dac_mge.ch & BIT(i))) {
            continue;
        }
        if (false == dac_cbuff_active(dac_mge.sound[i])) {
            continue;
        }
        rptr[i] = cbuf_read_alloc(dac_mge.sound[i]->p_obuf, &olen[i]);
        if (0 == olen[i]) {
            log_char('z');
            rptr[i] = 0;
        }
        d_mio_kick(dac_mge.sound[i]->mio, DAC_PACKET_SIZE);
    }


    for (sp_cnt = 0; sp_cnt < sp_number; sp_cnt++) {
        t_sp = 0;
        for (i = 0; i < DAC_CHANNEL_NUMBER; i++) {
            if (0 == rptr[i]) {
                continue;
            }
            if (0 == olen[i]) {
                rptr[i] = cbuf_read_alloc(dac_mge.sound[i]->p_obuf, &olen[i]);
                if (0 == olen[i]) {
                    log_char('x');
                    rptr[i] = 0;
                }
                continue;
            }
            /* log_info("b\n"); */
            /* active_flag++; */
            p_cnt[i]++;
            /* if (i == 0) */
            t_sp += *rptr[i];
            rptr[i]++;
            if ((p_cnt[i] * 2) >= olen[i]) {
                cbuf_read_updata(dac_mge.sound[i]->p_obuf, p_cnt[i] * 2);
                /* rptr[i] = 0; */
                olen[i] = 0;
                p_cnt[i] = 0;
                rptr[i] = cbuf_read_alloc(dac_mge.sound[i]->p_obuf, &olen[i]);
                if (0 == olen[i]) {
                    log_char('y');
                    rptr[i] = 0;
                }

            }
        }
        if (MAX_PHY_VOL != dac_mge.vol_phy) {
            /* t_sp = (t_sp / (MAX_PHY_VOL + 1)) * dac_mge.vol_phy; */
            t_sp = (t_sp * dac_mge.vol_phy) / (MAX_PHY_VOL + 1);
        }
        t_sp += sp_buf[sp_cnt];
        dac_sp_handle(t_sp);
        if (t_sp > 32767) {
            t_sp = 32767;
        } else if (t_sp < -32768) {
            t_sp = -32768;
        }
        sp_buf[sp_cnt] = t_sp;

        /* if (0 == active_flag) { */
        /* break; */
        /* } */
    }
    /* log_info("b"); */

    for (i = 0; i < DAC_CHANNEL_NUMBER; i++) {
        if (0 == (dac_mge.ch & BIT(i))) {
            continue;
        }
        if (0 != rptr[i]) {
            cbuf_read_updata(dac_mge.sound[i]->p_obuf, p_cnt[i] * 2);
        }
        dac_kick_decoder(dac_mge.sound[i], dac_mge.kick[i]);
    }
}

AT(.audio_d.text.cache.L2)
void fill_dac_fill(u8 *buf, u32 len, AUDIO_TYPE type)
{
    fill_dac_fill_phy(buf, len);
}

u8 dac_vol(char set, u8 vol)
{
    if ('+' == set) {
        dac_mge.vol++;
    } else if ('-' == set) {
        if (0 != dac_mge.vol) {
            dac_mge.vol--;
        }
    } else if ('r' == set) {
        goto __dac_vol_end;
    } else {
        dac_mge.vol = vol;
    }
    if (dac_mge.vol > MAX_VOL_LEVEL) {
        dac_mge.vol = MAX_VOL_LEVEL;
    }
    if (0 == (dac_mge.flag & B_DAC_MUTE)) {
#if DAC_FADE_ENABLE
        if (0 == (dac_mge.flag & B_DAC_FADE_EN))
#endif
        {
            dac_mge.vol_phy = vol_tab[dac_mge.vol];
        }
        dac_mge.flag &= ~B_DAC_MUTE;
    }
    log_info(" dac vol %d, 0x%x\n", dac_mge.vol, dac_mge.vol_phy);
__dac_vol_end:
    return dac_mge.vol;
}

bool dac_mute(bool mute)
{
    if (mute) {
        dac_mge.flag |= B_DAC_MUTE;
        dac_mge.vol_phy = 0;
    } else {
        dac_mge.flag &= ~B_DAC_MUTE;
        dac_mge.vol_phy = vol_tab[dac_mge.vol];
    }
    return true;
}

bool regist_dac_channel(void *psound, void *kick)
{
    u8 i;
    for (i = 0; i < DAC_CHANNEL_NUMBER; i++) {
        if (dac_mge.ch & BIT(i)) {
            continue;
        }
        /* dac_mge.obuf[i] = obuf; */
        dac_mge.kick[i] = kick;
        dac_mge.sound[i] = psound;
        dac_mge.ch |= BIT(i);
        /* log_info("dac_channel :0x%x 0x%x\n", i, dac_mge.ch); */
        return true;
    }
    return false;
}

bool unregist_dac_channel(void *psound)
{
    u8 i;
    sound_out_obj *ps = psound;

    for (i = 0; i < DAC_CHANNEL_NUMBER; i++) {
        if (0 == (dac_mge.ch & BIT(i))) {
            continue;
        }

        if (dac_mge.sound[i] == psound) {
            local_irq_disable();
            dac_mge.ch &= ~BIT(i);
            /* dac_mge.obuf[i] = 0; */
            dac_mge.sound[i] = 0;
            dac_mge.kick[i] = NULL;
            ps->enable &= ~B_DEC_OBUF_EN;
            local_irq_enable();
            break;
        }
    }
    return true;
}



#if 0
#include "circular_buf.h"
cbuffer_t t_cbuf;
u32 t_hld;
static u8 t_obuf[2 * 32 * 20];

const u16 sine_wav_t[8] = {
    0x0000,
    0xa57e,
    0x8000,
    0xa57e,
    0x0000,
    0x5a82,
    0x7fff,
    0x5a82
};

void dac_api_test_demo(void)
{
    audio_init();
    dac_mode_init();
    dac_init_api(8000);

    cbuf_init(&t_cbuf, t_obuf, sizeof(t_obuf));

    regist_dac_channel(&t_hld, NULL);
    while (1) {
        delay(10);
        /* cbuf_write( &t_cbuf, (void *)&sine_wav_t[0], sizeof(sina_wav_t)); */
        cbuf_write(&t_cbuf, (void *)&sine_wav_t[0], 8 * 2);
        wdt_clear();
    }
}
#endif

/* FPGA DAC IO:
 * dacck    --> PJ4
 * dac_pout --> PJ5
 * dac_pout --> PJ6
 * dac_o    --> PJ7
 * */
void  dac_usb_vol(u8 vol_r, u8 vol_l)
{
    u8 vol = (vol_r + vol_l) / 2;
    dac_vol(0, vol);
}



/* #define DAC_FADE_STEP (16384 / 1638) */
void dac_fade(void)
{
    static u16 cnt = 0;
    if (0 == (dac_mge.flag & B_DAC_FADE_EN)) {
        return;
    }
    if (dac_mge.flag & B_DAC_MUTE) {
        return;
    }
    u16 target_vol = 0;
    u16 curr_vol = dac_mge.vol_phy;
    u16 t_fade_step;
    cnt++;
    if (curr_vol > 100) {
        t_fade_step = (curr_vol + 1000) / 60;
    } else {
        t_fade_step =  10;
    }

    if (dac_mge.flag & B_DAC_FADE_OUT) {
        if (curr_vol > t_fade_step) {
            curr_vol -= t_fade_step;
        } else {
            curr_vol = 0;
        }
    } else {
        target_vol = vol_tab[dac_mge.vol];
        if (target_vol > curr_vol) {
            if (curr_vol < (target_vol - t_fade_step)) {
                curr_vol += t_fade_step;
            } else {
                curr_vol = target_vol;
            }

        } else {
            if (curr_vol > (target_vol + t_fade_step)) {
                curr_vol -= t_fade_step;
            } else {
                curr_vol = target_vol;
            }
        }

    }
    dac_mge.vol_phy = curr_vol;
    /* log_info(" fade  %d %d \n",dac_mge.vol_phy, cnt); */
    if (target_vol == dac_mge.vol_phy) {
        dac_mge.flag &= ~B_DAC_FADE_EN;
        cnt = 0;
    }
}
void dac_fade_in(void)
{
    CPU_INT_DIS();
    /* log_info(" fade in  %d \n",dac_mge.vol_phy); */
    dac_mge.flag &= ~B_DAC_FADE_OUT;
    dac_mge.flag |= B_DAC_FADE_EN;
    CPU_INT_EN();
}

void dac_fade_out(u32 delay)
{
    CPU_INT_DIS();
    /* log_info(" fade out  %d \n",dac_mge.vol_phy); */
    dac_mge.flag |= B_DAC_FADE_OUT;
    dac_mge.flag |= B_DAC_FADE_EN;
    CPU_INT_EN();
    u32 to_cnt = 0;
    while (dac_mge.flag & B_DAC_FADE_EN) {
        wdt_clear();
        delay_10ms(1);
        to_cnt++;
        if (to_cnt > delay) {
            break;
        }
    }
}




