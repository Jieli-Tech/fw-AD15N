#include "mbox_common.h"

void pa_unmute(void)
{
    JL_PORTA->OUT &= ~BIT(8);
    JL_PORTA->PU &= ~BIT(8);
    JL_PORTA->PD &= ~BIT(8);
    JL_PORTA->DIR &= ~BIT(8);
}

void pa_mute(void)
{
    JL_PORTA->OUT |= BIT(8);
    JL_PORTA->PU &= ~BIT(8);
    JL_PORTA->PD &= ~BIT(8);
    JL_PORTA->DIR &= ~BIT(8);
}

