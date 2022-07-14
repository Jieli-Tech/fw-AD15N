#ifndef howling_api_h__
#define howling_api_h__

#include "howling_pitchshifter_api.h"
#include "notch_howling_api.h"

void *howling_phy(void *obuf, void *dbuf, HOWLING_PITCHSHIFT_PARM *parm, u32 sr, void **ppsound);
void howing_parm_update(s16 ps_parm);

void *notch_howling_phy(void *obuf, NotchHowlingParam *parm, void **ppsound);
void notch_howing_parm_update(NotchHowlingParam *parm);



#endif // reverb_api_h__
