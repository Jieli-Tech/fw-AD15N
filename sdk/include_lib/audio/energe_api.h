
#ifndef ENERGE_API_H
#define ENERGE_API_H

#include "sound_effect_api.h"

struct energe_param {
    int sample_rate;   //采样率
    int channel;	  // 通道
    int detect_time;  //检测时间ms  当attFactor 与 relFactor为0 时有效
    int attFactor;	  //启动因子 (int)(exp(-1 / (time * sample_rate)) * (1 << 30));   time 为 0.001 ~ 0.1；默认0.001   time越小db值上升越快
    int relFactor;	  //释放因子 (int)(exp(-1 / (time * sample_rate)) * (1 << 30));   time 为 0.3 ~ 0.9；  默认0.5        time越小db值下降越快
};

int get_energe_buf(struct energe_param *param);
void energe_init(void *work_buf, struct energe_param *param);
int energe_run(void *work_buf, short *indata, int per_channel_npoint);  //运算buf, indata 输入数据； per_channel_npoint 每个通道的输入数据点数,返回值为0
int *get_energe_db(void *work_buf); //获取mdb值  例如 int *db_value = get_energe_db(work_buf);

// 注意：
// 1、db值计算库里分两种模式：
// (1)、attFactor 与 relFactor 等于0时。实时计算detect_time这一段时间 pcm数据最大值的db值
// (2)、当attFactor 或者 relFactor 不等0时，detect_time参数无效。库里有做特殊处理，db值上升快，下降慢。由于定点运算db值大小会比实际db值小1 ~2db

// 2、获取mdb值函数，获取到的mdb值指针，包含两个通道的mdb值，若只有一个通道则取第一个值，如db_value[0];
// 获取到的是mdb值，转化为db值的关系是  db = mdb / 1000;

#define ENERGE_INIT     BIT(0)

void energe_detect_init(u32 sr);
void energe_run_api(short *inbuf, int len);
void energe_dt_api(u32 sr);
#endif
