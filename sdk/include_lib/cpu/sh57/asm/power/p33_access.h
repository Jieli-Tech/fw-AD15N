#ifndef __P33_ACCESS_H__
#define __P33_ACCESS_H__

//
//
//					for p33_operation
//
//
//
/***************************************************************/

#define p33_cs_h(x)        do{local_irq_disable();\
							if(x&BIT(15)){JL_PMU->SPI_CON |= BIT(0)|BIT(8);}\
							else{JL_PMU->SPI_CON  &= ~BIT(8);JL_PMU->SPI_CON  |= BIT(0);}}while(0)

#define p33_cs_l           do{JL_PMU->SPI_CON  &= ~(BIT(0)|BIT(8));local_irq_enable();}while(0)

#define P33_OR              0b001
#define P33_AND             0b010
#define P33_XOR             0b011

u8 p33_buf(u8 buf);

void p33_xor_1byte(u16 addr, u8 data0);

void p33_and_1byte(u16 addr, u8 data0);

void p33_or_1byte(u16 addr, u8 data0);

void p33_tx_1byte(u16 addr, u8 data0);

u8 p33_rx_1byte(u16 addr);

_INLINE_
static u8 P33_CON_GET(u8 addr)
{
    u8 reg = 0;
    reg = p33_rx_1byte(addr);
    return reg;
}

void P33_CON_SET(u16 addr, u8 start, u8 len, u8 data);

#define p33_fast_access(reg, data, en)           \
{ 												 \
    if (en) {                                    \
		p33_or_1byte(reg, (data));               \
    } else {                                     \
		p33_and_1byte(reg, (u8)~(data));             \
    }                                            \
}




#endif
