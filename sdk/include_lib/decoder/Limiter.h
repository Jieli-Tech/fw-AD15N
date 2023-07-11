#ifndef Limiter_new_h__
#define Limiter_new_h__

//ReleaseTime and AttackTime is in ms
//TargetLvl is in mdB 0~-20
//SampleRate in Hz
//nChannel 1~2
void Limiter_Init(void *workbuf, int AttackTime, int ReleaseTime, int TargetLvl, int SampleRate, int nChannel);
void Limiter_SetVariable(void *workbuf, int AttackTime, int ReleaseTime, int TargerLvl, int SampleRate);
int Limiter_QueryBufSize();
void Limiter_Run(void *workbuf, int *in, short *out, int npoint);
#endif // Limiter_new_h__

