#ifndef __CPU_H__
#define __CPU_H__

#include "typedef.h"

#define AUOUT_USE_RDAC      0

#if (0 == AUOUT_USE_RDAC)
//apa
#include "apa.h"
#define auout_mode_init     apa_mode_init
#define auout_init(n,m)     apa_init(n,m)
#define auout_sr_api(sr)    apa_sr_api(sr)
#define auout_off_api		apa_off_api
#define dac_sr_read         apa_sr_read
#define dac_sr_set          apa_sr_set

#define dac_sp_handle(n)    apa_sp_handle(n)
#define D_PHY_VOL_SET_FUNC  0

#define audio_analog_open   apa_analog_pre_open
#define audio_analog_close  apa_analog_close
#define audio_isr_init()    HWI_Install(IRQ_APA_IDX, (u32)apa_isr_t, IRQ_AUDIO_IP)
#define audio_clk_init()    SFR(JL_CLOCK->PRP_CON0, 12,  2,  APA_CLK_DEFAULT)
#define audio_clk_close(...)
#define IRQ_AUDIO_IDX       IRQ_APA_IDX

#else
// 模拟audio_dac
#include "rdac.h"
#define auout_mode_init     rdac_mode_init
#define auout_init(n,m)     rdac_init(n,m)
#define auout_sr_api(sr)    rdac_sr_api(sr)
#define auout_off_api		rdac_off_api
#define dac_sr_read         rdac_sr_read
#define dac_sr_set          rdac_sr_set

#define dac_sp_handle(...)
#define D_PHY_VOL_SET_FUNC  rdac_phy_vol

#define audio_analog_open   audio_common_analog_open
#define audio_analog_close  audio_common_analog_close
#define audio_isr_init()    HWI_Install(IRQ_RDAC_IDX, (u32)audio_isr, IRQ_AUDIO_IP)
#define audio_clk_init      rdac_clk_init
#define audio_clk_close     rdac_clk_close
#define IRQ_AUDIO_IDX       IRQ_RDAC_IDX

#endif

#define SR_DEFAULT  32000

#endif
