#ifndef _TOY_LINEINE_H_
#define _TOY_LINEINE_H_

#include "typedef.h"
#include "key.h"
#include "decoder_mge.h"

void toy_linein_app(void);
void aux_test_audio(void);
extern u16 linein_key_msg_filter(u8 key_status, u8 key_num, u8 key_type);
#endif
