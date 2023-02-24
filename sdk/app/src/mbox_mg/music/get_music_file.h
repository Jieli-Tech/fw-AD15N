#ifndef  _GET_MUSIC_FILE_
#define  _GET_MUSIC_FILE_


#include "config.h"
#include "music/device.h"
//#include "tff.h"

#if 1//def __SMART_CPU__

typedef enum {						//播放文件的方式定义
    PLAY_NEXT_FILE = 0,
    PLAY_AUTO_NEXT,
    PLAY_FIRST_FILE,  		    //从第一个文件播放
    PLAY_BREAK_POINT,           //从记忆文件和位置开始播放
    PLAY_LAST_FILE,				//从最后一个文件播放
    PLAY_PREV_FILE,
    PLAY_SPEC_FILE,             //IR 输入指定文件
    PLAY_REC_FILE,
} ENUM_FILE_SELECT_MODE;

typedef enum {
    REPEAT_ALL = 0,

#ifdef FOLDER_PLAY_EN
    REPEAT_FOLDER,
#endif

    REPEAT_ONE,

#ifdef RANDOM_PLAY_EN
    REPEAT_RANDOM,
#endif

    MAX_PLAY_MODE,
} ENUM_PLAY_MODE;


enum {
    GET_FILE_OK = 0,			        //找到文件序号
    GET_DEVICE_END,						//已经播放到设备结束
    GET_DEVICE_HEAD,					//已经播放到设备开头
    //GET_SPEC_FILE_ERR
};

typedef struct _SELECT_PLAY_FILE {
    ENUM_PLAY_MODE play_mode;
    ENUM_FILE_SELECT_MODE given_file_method;
    ENUM_DEVICE_SELECT_MODE given_device;
    u16 given_file_number;
    u16 break_point_filenum;
} SELECT_PLAY_FILE;

u8 fs_get_filenum(ENUM_PLAY_MODE playmode, ENUM_FILE_SELECT_MODE searchMode);
void get_dir_file_total(void);


extern bool play_break_point;
extern SELECT_PLAY_FILE playfile;
#endif


#endif

