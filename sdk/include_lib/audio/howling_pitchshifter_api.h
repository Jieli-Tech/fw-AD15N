
#ifndef HOWLING_pitchshifer_api_h__
#define HOWLING_pitchshifer_api_h__

#ifndef u8
#define u8  unsigned char
#endif

#ifndef u16
#define u16 unsigned short
#endif

#ifndef s16
#define s16 short
#endif


#ifndef u32
#define u32 unsigned int
#endif


#ifndef s32
#define s32 int
#endif

#ifndef s16
#define s16 signed short
#endif

/*#define  EFFECT_OLD_RECORD          0x01
#define  EFFECT_MOYIN               0x0*/
//#define  EFFECT_ROBORT_FLAG          0X04

enum {
    EFFECT_HOWLING_PS        = 0x01,              //1.5《=》12 ms
    EFFECT_HOWLING_HE       = 0x02
};

typedef struct HOWLING_PITCHSHIFT_PARM_ {
    u32 sr;                          //input  audio samplerate
    s16 ps_parm;
    s16 he_parm;
    s16 fe_parm;
    u32 effect_v;
} HOWLING_PITCHSHIFT_PARM;


typedef struct _HOWL_PS_IO_CONTEXT_ {
    void *priv;
    int(*output)(void *priv, void *data, int len);
} HOWL_PS_IO_CONTEXT;



typedef struct _HOWLING_PITCHSHIFT_FUNC_API_ {
    u32(*need_buf)(int flag);
    void (*open)(void *ptr, HOWLING_PITCHSHIFT_PARM *pitchshift_obj, HOWL_PS_IO_CONTEXT *how_ps_io);        //中途改变参数，可以调init
    void (*init)(void *ptr, HOWLING_PITCHSHIFT_PARM *pitchshift_obj);
    int (*run)(void *ptr, short *indata,  int len);   //len是多少个byte
} HOWLING_PITCHSHIFT_FUNC_API;

extern HOWLING_PITCHSHIFT_FUNC_API *get_howling_ps_func_api();

#endif // reverb_api_h__

