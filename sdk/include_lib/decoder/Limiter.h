#ifndef Limiter_new_h__
#define Limiter_new_h__

//ReleaseTime and AttackTime is in ms
//TargetLvl is in mdB 0~-20
//SampleRate in Hz
//nChannel 1~2
// lookAheadTime is in ms and generate delay
typedef struct _LimiterParam {
    int attackTime;     //限幅器启动时间，输入信号超出阈值开始启动  默认5，范围1 ~100 越小启动越快
    int releaseTime;    //限幅器释放时间，输入信号低于阈值限幅恢复原始信号时间  默认300   范围200 ~ 15000
    int TargerLvl;      //限幅器阈值   单位mdb  比如 -5db 需填-5000。db到mdb 进制1000
    int lookAheadTime;  //限幅器提前检测时间。 范围 0~ 20 单位ms 默认0 ，不为0，压制效果好些，但会造成时延
    int SampleRate;     //采样率
    int nChannel;       //声道数，默认填1,单声道
} LimiterParam;


/**@brief  获取运行buf 大小(lookAheadTime  影响buf大小)
  * @param[in]  无
  * @return
  */
int Limiter_QueryBufSize(LimiterParam *param);

/**@brief  限幅器初始化
  * @param[in]  workbuf  运行buf
  * @return
  */
void Limiter_Init(void *workbuf, LimiterParam *param);

/**@brief  限幅器更新参数
  * @param[in]  lookAheadTime 不能调用此函数更新，lookAheadTime 发生变化需重新申请buf 走初始化
  * @return
  */
void Limiter_SetVariable(void *workbuf, LimiterParam *param);

/**@brief  限幅器运行
  * @param[in]  workbuf  运行buf；  in输入数据  out 输出数据   npoint 每个通道的点数
  * @return
  */
void Limiter_Run(void *workbuf, int *in, short *out, int npoint);

#endif // Limiter_new_h__


