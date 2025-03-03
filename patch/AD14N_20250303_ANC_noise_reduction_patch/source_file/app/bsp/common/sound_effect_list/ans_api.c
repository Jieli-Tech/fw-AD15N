#include "circular_buf.h"
#include "errno-base.h"
#include "NoiseSuppressLib.h"
#include "app_modules.h"

#define LOG_TAG_CONST       NORM
/* #define LOG_TAG_CONST       OFF */
#define LOG_TAG             "[ans]"
#include "log.h"

#if defined(ANS_EN) && (ANS_EN)

/*
 * ANS STATE BITMAP
 */
#define ANS_STATE_INIT          BIT(0)  /*ANS task ready*/
#define ANS_STATE_ADC           BIT(1)  /*ANS adc ready*/
#define ANS_STATE_REF           BIT(2)  /*ANS dac ready*/
#define ANS_STATE_CLOSE         BIT(3)  /*ANS task ready*/
#define ANS_STATE_PEND          BIT(4)  /*ANS task delete*/
#define ANS_STATE_SUSPEND       BIT(5)  /*ANS task suspend*/
#define ANS_STATE_RESET         BIT(6)  /*ANS reseting*/
#define ANS_STATE_NULL          0
#define ANS_STATE_OK            (ANS_STATE_INIT | ANS_STATE_ADC)

#define NS_MODE                 0
#define NS_IS_WIDEBAND          1//0:8K 1:16K
#define NS_D                    (64 << NS_IS_WIDEBAND)

#define READSIZE                128
#define ANS_RUN_BUFFSIZE        5400
#define ANS_TMP_BUFSIZE         3604
#define ANS_NEAR_SIZE           (READSIZE * 2)

int ans_runbuf[ANS_RUN_BUFFSIZE / 4] AT(.ans_data);
int ans_tmpbuf[ANS_TMP_BUFSIZE / 4] AT(.ans_data);

typedef struct {
    volatile unsigned char state;
    short output[READSIZE];
    short ans_near[ANS_NEAR_SIZE / 2];
    cbuffer_t *in_cbuf;
    cbuffer_t *out_cbuf;
    void (*kick)(void *);
} ANS_VAR;

static ANS_VAR ans_var AT(.ans_data);
static ANS_VAR *ans;

static void ans_output(s16 *dat, u16 len)
{
    //降噪后输出接口
    if (cbuf_get_available_space(ans->out_cbuf) < len) {
        log_char('N');
        return;
    }
    cbuf_write(ans->out_cbuf, dat, len);
    ans->kick(NULL);
}

___interrupt
static void soft3_isr(void)
{
    //平台相关
    bit_clr_swi(3);
    if ((ans == NULL) && (ans->state != ANS_STATE_OK)) {
        return;
    }
    //从cbuf读取近端录音数据
    cbuf_read(ans->in_cbuf, ans->ans_near, ANS_NEAR_SIZE);

    //执行降噪算法
    NoiseSuppress_Process(ans_runbuf, ans_tmpbuf, ans->ans_near, ans->output, NULL, NULL, READSIZE);

    ans_output(ans->output, READSIZE * sizeof(short));
}

//mic 中断填数据
AT(.audio_a.text.cache.L2)
int ans_check_kick_start(void *priv)
{
    if (ans == NULL) {
        return 0;
    }
    ans->state |= ANS_STATE_ADC;
    if (ans->state != ANS_STATE_OK) {
        return 0;
    }
    //查看buffer数据是否够运行一次ans算法,不够则退出
    if (cbuf_get_data_size(ans->in_cbuf) < ANS_NEAR_SIZE) {
        return 0;
    }

    //启动软中断
    bit_set_swi(3);
    return 0;
}

s32 ans_init(cbuffer_t *in_cbuf, cbuffer_t *out_cbuf, void *kick)
{
    ans = &ans_var;
    memset(ans, 0, sizeof(ANS_VAR));

    ans->in_cbuf = in_cbuf;
    ans->out_cbuf = out_cbuf;
    ans->kick = kick;

    int tolbufsize = NoiseSuppress_QueryBufSize(NS_MODE, NS_IS_WIDEBAND);;
    log_info("tolbufsize %d %d\n", tolbufsize, sizeof(ans_runbuf));
    ASSERT(ANS_RUN_BUFFSIZE >= tolbufsize);
    int maxtmpbufsize = NoiseSuppress_QueryTempBufSize(NS_MODE, NS_IS_WIDEBAND);
    log_info("maxtmpbufsize %d %d\n", maxtmpbufsize, sizeof(ans_tmpbuf));
    ASSERT(ANS_TMP_BUFSIZE >= maxtmpbufsize);

    int ANS_AggressFactor = (int)(125 * 65536 / 100);/*范围：1~2,动态调整,越大越强(1.25f)*/
    int ANS_MinSuppress = (int)(10 * 65536 / 100);	/*范围：0~1,静态定死最小调整,越小越强(0.1f)*/
    int ANS_NoiseLevel = (int)(1429 * 1024);      /*范围：-100dB ~ -40dB (-75dB) (1429 = (10^(-75/20))*2^23)*/
    NoiseSuppress_Init(ans_runbuf, ANS_AggressFactor, ANS_MinSuppress, NS_MODE, NS_IS_WIDEBAND, ANS_NoiseLevel);

    //注册软中断 -- 平台相关
    HWI_Install(IRQ_SOFT3_IDX, (u32)soft3_isr, 0);

    ans->state = ANS_STATE_INIT;
    return 0;
}

s32 ans_deinit(void)
{
    memset(ans, 0, sizeof(ANS_VAR));
    HWI_Uninstall(IRQ_SOFT3_IDX);
    ans = NULL;
    return 0;
}

//算法库使用的表
const short STFT_Win_FixHalf_M256_D128[] = {
    0,   402,   804,  1206,  1608,  2009,  2411,  2811,  3212,  3612,  4011,  4410,  4808,  5205,  5602,  5998, //
    6393,  6787,  7180,  7571,  7962,  8351,  8740,  9127,  9512,  9896, 10279, 10660, 11039, 11417, 11793, 12167, //
    12540, 12910, 13279, 13646, 14010, 14373, 14733, 15091, 15447, 15800, 16151, 16500, 16846, 17190, 17531, 17869, //
    18205, 18538, 18868, 19195, 19520, 19841, 20160, 20475, 20788, 21097, 21403, 21706, 22006, 22302, 22595, 22884, //
    23170, 23453, 23732, 24008, 24279, 24548, 24812, 25073, 25330, 25583, 25833, 26078, 26320, 26557, 26791, 27020, //
    27246, 27467, 27684, 27897, 28106, 28311, 28511, 28707, 28899, 29086, 29269, 29448, 29622, 29792, 29957, 30118, //
    30274, 30425, 30572, 30715, 30853, 30986, 31114, 31238, 31357, 31471, 31581, 31686, 31786, 31881, 31972, 32058, //
    32138, 32214, 32286, 32352, 32413, 32470, 32522, 32568, 32610, 32647, 32679, 32706, 32729, 32746, 32758, 32766, //
    32767, 32766, 32758, 32746, 32729, 32706, 32679, 32647, 32610, 32568, 32522, 32470, 32413, 32352, 32286, 32214, //
    32138, 32058, 31972, 31881, 31786, 31686, 31581, 31471, 31357, 31238, 31114, 30986, 30853, 30715, 30572, 30425, //
    30274, 30118, 29957, 29792, 29622, 29448, 29269, 29086, 28899, 28707, 28511, 28311, 28106, 27897, 27684, 27467, //
    27246, 27020, 26791, 26557, 26320, 26078, 25833, 25583, 25330, 25073, 24812, 24548, 24279, 24008, 23732, 23453, //
    23170, 22884, 22595, 22302, 22006, 21706, 21403, 21097, 20788, 20475, 20160, 19841, 19520, 19195, 18868, 18538, //
    18205, 17869, 17531, 17190, 16846, 16500, 16151, 15800, 15447, 15091, 14733, 14373, 14010, 13646, 13279, 12910, //
    12540, 12167, 11793, 11417, 11039, 10660, 10279,  9896,  9512,  9127,  8740,  8351,  7962,  7571,  7180,  6787, //
    6393,  5998,  5602,  5205,  4808,  4410,  4011,  3612,  3212,  2811,  2411,  2009,  1608,  1206,   804,   402, //
};

#endif

