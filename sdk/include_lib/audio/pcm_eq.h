#ifndef __PCM_EQ_H__
#define __PCM_EQ_H__




typedef struct _PCM_EQ_PARM_ {
    const int *LCoeff_OnChip;
    const int *RCoeff_OnChip;
    int LGain;
    int RGain;
    int SHI;
    int SHO;
    int nSection;
    int channel;
} PCM_EQ_PARM;



int getEQBuf(int nSection, int channel);
void EQInit(void *workBuf, const int *LCoeff_OnChip, const int *RCoeff_OnChip, int LGain, int RGain, int SHI, int SHO, int nSection, int channel);
void EQUpdate(void *workBuf, const int *LCoeff_OnChip, const int *RCoeff_OnChip, int LGain, int RGain, int SHI, int SHO, int nSection, int channel);
void clearEQSOSMem(void *workBuf);
int EQRun(void *workBuf, short *in, short *out, int npoint);



////-----------------
int pcm_eq_buf_len(int nSection, int channel);
void *pcm_eq_phy(void *obuf, void *dbuf, PCM_EQ_PARM *parm, void **ppsound);
void pcm_eq_updata(void *dbuf, PCM_EQ_PARM *p_parm);

#endif
