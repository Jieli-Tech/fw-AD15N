/*--------------------------------------------------------------------------*/
/**@file     bk1080.h
   @brief    BK1080收音
   @details
   @author
   @date   2011-3-30
   @note
*/
/*----------------------------------------------------------------------------*/
#ifndef _BK_1080_H_
#define _BK_1080_H_

#include "app_config.h"

#if(TCFG_FM_BK1080_ENABLE == ENABLE)


#define XTAL_CLOCK			1
#define CHIP_DEV_ID 		0x80


void bk1080_init(void);
void bk1080_setfreq(u16 curFreq);
bool bk1080_set_fre(u16 freq);
void bk1080_powerdown(void);
void bk1080_mute(u8 flag);
bool bk1080_read_id(void);
void bk1080_setch(u8 db);


#define MIN_FRE 875
#endif

#endif		/*  _BK_1080_H_ */
