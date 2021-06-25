#ifndef howling_api_h__
#define howling_api_h__

#include "howling_pitchshifter_api.h"
#include "notch_howling_api.h"

void *howling_phy(void *obuf, HOWLING_PITCHSHIFT_PARM *parm, void **ppsound);
void howing_parm_update(s16 ps_parm);

void *notch_howling_phy(void *obuf, NH_PARA_STRUCT *parm, void **ppsound);
void notch_howing_parm_update(NH_PARA_STRUCT *parm);



#endif // reverb_api_h__
