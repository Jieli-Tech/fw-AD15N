#include "asm/power_interface.h"
#include "app_config.h"
#include "gpio.h"

void port_hd_init(void)
{
    u16 porta_hd0 = 0xffff;
    u16 porta_hd1 = 0;
    JL_PORTA->HD0 = porta_hd0;
    JL_PORTA->HD1 = porta_hd1;

    u16 portb_hd0 = 0xffff;
    u16 portb_hd1 = 0;
    JL_PORTB->HD0 = portb_hd0;
    JL_PORTB->HD1 = portb_hd1;

    u16 portd_hd0 = 0xffff;
    u16 portd_hd1 = 0x10;
    JL_PORTD->HD0 = portd_hd0;
    JL_PORTD->HD1 = portd_hd1;
}



