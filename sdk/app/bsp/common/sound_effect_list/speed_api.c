/* #include "resample_api.h" */
#include "speed_api.h"
/* #include "resample.h" */
#include "typedef.h"
#include "decoder_api.h"
#include "config.h"
#include "sound_effect_api.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"

#if AUDIO_SPEED_EN

SPEED_PITCH_PARA_STRUCT sp_parm AT(.sp_data);

void *speed_api(void *obuf, u32 insample, void **ppsound)
{
    log_info("speed_api\n");


    sp_parm.insample = insample;             //输入音频采样率
    sp_parm.pitchrate = 100;           //变调比例，128:为不变调，<128:音调变高，>128:音调变低，声音更沉
    //speedout/speedin 的结果大于0.6小于1.8.变快变慢最好都不要超过2倍
    sp_parm.speedin = 80;
    sp_parm.speedout = 144;          //sp_parm.speedin：sp_parm.speedout为变速比例，例如speedin=1;speedout=2;则为变慢1倍
    sp_parm.quality = 4;            //变调运算的运算质量，跟速度成正比，配置范围3-8
    sp_parm.pitchflag = 1;     //如果pitchflag=1,就是输入输出速度只由speedin：speedout决定,调节音调的时候，速度比例保持不变。例如设置speedout：speedin=1:2 ，这时候，去调节pitchrate，都是只变调，速度一直是保持放慢1倍。
    //如果pitchflag=0，就是变速变调比例分开控制 ，这样实际变速的比例是 （speedout/speedin)*pitchrate了。

    return speed_phy(obuf, &sp_parm, ppsound);
}
#endif




