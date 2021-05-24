#ifndef __KEY_DRV_IO_H__
#define __KEY_DRV_IO_H__

#include "gpio.h"
#include "key.h"

#define IS_KEY0_DOWN()    	(!(JL_PORTA->IN & BIT(7)))
#define IS_KEY1_DOWN()    	(!(JL_PORTA->IN & BIT(8)))
#define IS_KEY2_DOWN()    	(!(JL_PORTA->IN & BIT(9)))

#define KEY_INIT()        do{\
							/**key0 init*/\
							JL_PORTA->PU |= BIT(7),\
							JL_PORTA->PD &= ~BIT(7),\
							JL_PORTA->DIE |= BIT(7),\
							JL_PORTA->DIR |= BIT(7);\
							/**key1 init*/\
							JL_PORTA->PU |= BIT(8),\
							JL_PORTA->PD &= ~BIT(8),\
							JL_PORTA->DIE |= BIT(8),\
							JL_PORTA->DIR |= BIT(8);\
							/**key2 init*/\
							JL_PORTA->PU |= BIT(9),\
							JL_PORTA->PD &= ~BIT(9),\
							JL_PORTA->DIE |= BIT(9),\
							JL_PORTA->DIR |= BIT(9);\
					    	}while(0)

extern const key_interface_t key_io_info;

#endif/*__KEY_DRV_IO_H__*/
