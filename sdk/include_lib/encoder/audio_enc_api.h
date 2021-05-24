#ifndef audio_enc_api_h__
#define audio_enc_api_h__

#ifndef u8
#define u8  unsigned char
#endif

#ifndef u16
#define u16 unsigned short
#endif

#ifndef u32
#define u32 unsigned int
#endif

#ifndef s16
#define s16 signed short
#endif


typedef struct _EN_FILE_IO_ {
    void *priv;
    u16(*input_data)(void *priv, s16 *buf, u16 len);
    u32(*output_data)(void *priv, u8 *buf, u16 len);
} EN_FILE_IO;



typedef  struct   _ENC_DATA_INFO_ {
    u16 sr;            ///<sample rate
    u16 br;            ///<mp2的时候它是bitrate，但是adpcm的时候，它是blockSize,一般配成256/512/1024/2048，超过2048会被限制成2048
} ENC_DATA_INFO;


typedef struct _ENC_OPS {
    u32(*need_buf)();
    void (*open)(u8 *ptr, ENC_DATA_INFO *data_info, EN_FILE_IO *audioIO);
    u32(*run)(u8 *ptr);
} ENC_OPS;

ENC_OPS *get_ima_code_ops(void);
ENC_OPS *get_mp2_ops(void);
ENC_OPS *get_mp2standard_ops(void);

#endif // audio_enc_api_h__
