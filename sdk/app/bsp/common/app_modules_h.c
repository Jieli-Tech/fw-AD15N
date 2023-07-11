// *INDENT-OFF*
ifndef __APP_MODULES_H__
define __APP_MODULES_H__

** 此文件在在服务器编译库时会自动生成，源文件位于app/bsp/common/app_modules_h.c

** 作者: 刘杰
** 日期:2022年11月22日
** 设计目的: 用于在应用代码中控制各种算法模块的使用
** 注意事项：不要在库文件中包含

-
#ifdef HAS_A_DECODER
-
** A格式解码
define DECODER_A_EN  1
#endif


#ifdef HAS_MP3_ST_DECODER
-
** 标准MP3格式解码
define DECODER_MP3_ST_EN 1
#endif


#ifdef HAS_WAV_DECODER
-
** WAV格式解码
define DECODER_WAV_EN 1
#endif


#ifdef HAS_F1A_DECODER
-
** F1A格式解码
define DECODER_F1A_EN  1
define MAX_F1A_CHANNEL HAS_MAX_F1A_NUMBER
#endif

#ifdef HAS_UMP3_DECODER
-
** UMP3格式解码
define DECODER_UMP3_EN 1
#endif

#ifdef HAS_MIDI_DECODER
-
** MIDI格式解码
define DECODER_MIDI_EN 1
#else
define DECODER_MIDI_EN 0
#endif

#ifdef HAS_MIDI_KEYBOARD_DECODER
-
** MIDI琴格式解码
define DECODER_MIDI_KEYBOARD_EN 1
#else
define DECODER_MIDI_KEYBOARD_EN 0
#endif

#ifdef HAS_OPUS_DECODER
-
** OPUS格式解码
define DECODER_OPUS_EN 1
#endif

/* - */
/* ** 无缝循环使能 */
/* #ifdef HAS_DECODER_LOOP_EN */
/* define DECODER_LOOP_EN 1 */
/* #else */
/* define DECODER_LOOP_EN 0 */
/* #endif */

/* - */
/* ** mp3格式压缩 */
/* #ifdef HAS_MP3_ENCODER */
/* define ENCODER_MP3_EN     0 */
/* #else */
/* define ENCODER_MP3_EN     0 */
/* #endif */


#ifdef HAS_UMP3_ENCODER
-
** ump3格式压缩
define ENCODER_UMP3_EN  1
#endif

#ifdef HAS_A_ENCODER
-
** a格式压缩
define ENCODER_A_EN  1
#endif

/* - */
/* ** opus格式压缩 */
/* #ifdef HAS_OPUS_ENCODER */
/* define ENCODER_OPUS_EN  1 */
/* #else */
/* define ENCODER_OPUS_EN  0 */
/* #endif */

-
** MIO功能使能
#ifdef HAS_MIO_PLAYER
define HAS_MIO_EN  0
#else
define HAS_MIO_EN  0
#endif

#ifdef HAS_SPEED_EN
-
** 解码SPEED功能使能
define AUDIO_SPEED_EN  1
#endif

#ifdef HOWLING_EN
-
** 陷波/移频啸叫抑制使能
define NOTCH_HOWLING_EN  1
define PITCHSHIFT_HOWLING_EN 2
#endif

#ifdef HAS_VOICE_PITCH_EN
-
** 变声功能使能
define VO_PITCH_EN  0
#endif

#ifdef HAS_VOICE_CHANGER_EN
-
** 动物变声功能使能
define VO_CHANGER_EN  0
#endif

#ifdef HAS_ECHO_EN
-
** ECHO混响功能使能
define ECHO_EN   1
#endif

#ifdef HAS_PCM_EQ_EN
-
** EQ功能使能
define PCM_EQ_EN   0
#endif

/* - */
/* ** A/F1A格式的解码文件需要同时判断后缀名确认采样率 */
/* if ( DECODER_A_EN || DECODER_F1A_EN  ) */
/* define DECODE_SR_IS_NEED_JUDIGMENT 1 */
/* else */
/* define DECODE_SR_IS_NEED_JUDIGMENT 0 */
/* endif */

/* - */
/* ** 定时任务注册功能使能 */
/* #ifdef HAS_SYS_TIMER_EN */
/* define SYS_TIMER_EN   1 */
/* #else */
/* define SYS_TIMER_EN   0 */
/* #endif */



-
** APP应用使能
/* #ifdef HAS_MUSIC_MODE */
/* define MUSIC_MODE_EN  1  -- mbox音乐应用模式 */
/* #else */
/* define MUSIC_MODE_EN  0  -- mbox音乐应用模式 */
/* #endif */

#ifdef ENCODER_EN
#if ENCODER_EN
define RECORD_MODE_EN  1  -- 录音应用模式
#else
define RECORD_MODE_EN  0  -- 录音应用模式
#endif
#endif

#ifdef AUX_EN
#if AUX_EN
define LINEIN_MODE_EN  1 -- Linein应用模式
#else
define LINEIN_MODE_EN  0 -- Linein应用模式
#endif
#endif

#ifdef HAS_SIMPLE_DEC_MODE
define SIMPLE_DEC_EN  1  -- 多路解码应用模式
#else
define SIMPLE_DEC_EN  0  -- 多路解码应用模式
#endif

#ifdef SPEAKER_EN
#if SPEAKER_EN
define LOUDSPEAKER_EN  1  -- 扩音应用模式
#else
define LOUDSPEAKER_EN  0  -- 扩音应用模式
#endif
#endif

#ifdef HAS_FM_EN
#if HAS_FM_EN
define FM_EN  1  -- FM应用模式
#else
define FM_EN  0  -- FM应用模式
#endif
#endif

/* #ifdef HAS_RTC_MODE */
/* define RTC_EN  0  -- RTC模式 */
/* #else */
/* define RTC_EN  0  -- RTC模式 */
/* #endif */

-
** 驱动使能
#ifdef HAS_EEPROM_EN
define EEPROM_EN	1  -- IIC_EEPROM驱动使能
#endif

#ifdef HAS_UART_UPDATE_EN
define UART_UPDATE_EN	0  -- 串口带电升级驱动使能
#endif

-

#ifdef HAS_VIRTUAL_RTC_EN
define VIRTUAL_RTC_EN	0  -- 虚拟RTC
#endif
-
-
endif
// *INDENT-ON*
