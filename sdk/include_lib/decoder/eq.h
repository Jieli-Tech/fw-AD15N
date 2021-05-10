#ifndef __EQ_H__
#define __EQ_H__

#include "typedef.h"
typedef enum {
    EQ_ZERO = 0x00,
    EQ_POP,
    EQ_ROCK,
    EQ_JAZZ,
    EQ_CLASSIC,
    EQ_MODEMAX
} EQ_MODE;

extern u8 g_eq_mode;

#endif
