#ifndef __CPU_CRC16_H__
#define __CPU_CRC16_H__
#include "typedef.h"

#define CRC16 	chip_crc16

u16 chip_crc16(void *ptr, u32  len); //CRC校验
void CrcDecode(void  *buf, u16 len);

#endif


