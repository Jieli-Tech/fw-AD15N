#ifndef	_FM_IIC_DRI_H_
#define _FM_IIC_DRI_H_

#include "typedef.h"
#include "tick_timer_driver.h"
#define delay_n10ms(x) delay_10ms(x)

void fm_i2c_init();
void fm_iic_write(u8 chip_id, u8 reg_addr, u8 *iic_dat, u8 len);
void fm_iic_readn(u8 chip_id, u8 reg_addr, u8 *iic_dat, u8 len);
u8 QND_ReadReg(u8 reg_addr);
#endif

