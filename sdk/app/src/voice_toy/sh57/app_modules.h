#ifndef __APP_MODULES_H__
#define __APP_MODULES_H__
//  此文件在在服务器编译库时会自动生成，源文件位于app/bsp/common/app_modules_h.c
//  作者: 刘杰
//  日期:2022年11月22日
//  设计目的: 用于在应用代码中控制各种算法模块的使用
//  注意事项：不要在库文件中包含


//  A格式解码
#define DECODER_A_EN 1

//  F1A格式解码
#define DECODER_F1A_EN 1
#define MAX_F1A_CHANNEL 1

//  MIDI格式解码
#define DECODER_MIDI_EN 1

//  MIDI琴格式解码
#define DECODER_MIDI_KEYBOARD_EN 1

//  MIO功能使能
#define HAS_MIO_EN 0

//  APP应用使能
#define SIMPLE_DEC_EN 1 //  多路解码应用模式

//  驱动使能



#endif


// ****************************************************************
//  此文件用于管理不同CPU间有差异的宏，源文件为app_modules_cpu_h.c
// ****************************************************************
#ifndef __APP_MODULES_CPU_H__
#define __APP_MODULES_CPU_H__

//  驱动使能
#define EXT_FLASH_EN 0 //  外挂资源flash使能
#endif
