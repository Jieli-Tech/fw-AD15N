
#ifndef __ANS_API_H__
#define __ANS_API_H__
#include "typedef.h"

// ..note ::
// ANS降噪算法最低系统时钟需要跑96M以上
// 只支持16k 和 8k采样率
//
int ans_check_kick_start(void *priv);
s32 ans_init(cbuffer_t *in_cbuf, cbuffer_t *out_cbuf, void *kick);
s32 ans_deinit(void);

#endif
