#ifndef _UI_COMMON_H_
#define _UI_COMMON_H_
#include "typedef.h"

void itoa1(u8 i);
void itoa2(u8 i);
void itoa3(u16 i);
void itoa4(u16 i);

extern const u8 asc_number[];

extern _no_init u8 _data bcd_number[5];

#endif

