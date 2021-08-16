/***********************************Jieli tech************************************************
  File : cpu.c
  By   : liujie
  Email: liujie@zh-jieli.com
  date : 2021-8-2
********************************************************************************************/
#include "config.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[cpu.c]"
#include "debug.h"



char chip_versions(void)
{
    char c = 'A';
    u32 chip_id = JL_CHIP->ID;
    switch (chip_id & 0xf) {
    case 0xb1100:
    case 0xb1101:
    case 0xb1110:
    case 0xb1111:
        c = 'A';
        break;
    default:
        c = 'A' + (chip_id & 0xf);
        break;
    }
    /* log_info("chip_id 0x%x; CHIP VERSIONS : %c\n", chip_id, c); */
    return c;
}


