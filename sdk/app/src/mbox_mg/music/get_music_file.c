#include "config.h"
#include "music_play.h"
#include "get_music_file.h"
#include "device.h"
#include "play_file.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

SELECT_PLAY_FILE playfile;

/*----------------------------------------------------------------------------*/
/**@brief   获取一个随机数
   @param   无
   @return  获取到的随机数
   @author:
   @note    u16 get_u16_random()
*/
/*----------------------------------------------------------------------------*/
u16 get_u16_random(void)
{
    extern volatile u32 jiffies;
    u16 *ptr;
    u16 res;

    ptr = (u16 *)0x200;
    res = *ptr++;
    res = *ptr + jiffies;

    return res;
}

/*----------------------------------------------------------------------------*/
/**@brief   获取文件号
   @param   playmode   循环播放模式
   @param   searchMode  0：上一个文件；1：下一个文件；2：在强行REPEAT_ONE模式時在REPEAT_ALL模式下查找文件
   @return  0:fail 1:success
   @author:
   @note    u8 fs_get_filenum(ENUM_PLAY_MODE playmode, ENUM_FILE_SELECT_MODE searchMode)
*/
/*----------------------------------------------------------------------------*/
u8 fs_get_filenum(ENUM_PLAY_MODE playmode, ENUM_FILE_SELECT_MODE searchMode)
{
    int start_num, end_num;
    u16 fileTotal = music_device_info[device_active].wfileTotal;

    if (searchMode == PLAY_FIRST_FILE) {			//播放第一个文件
        playfile.given_file_number = 1;
        return GET_FILE_OK;
    } else if (searchMode == PLAY_LAST_FILE) {			//播放最后一个文件
        playfile.given_file_number = fileTotal;
        return GET_FILE_OK;
    } else if (searchMode == PLAY_BREAK_POINT) {			//播放记忆的文件序号和断点
#ifdef LAST_MEM_FILE_PLAY_EN
        if (find_break_point_file_flag) {               /*记忆文件有效*/
            find_break_point_file_flag = 0;
            playfile.given_file_number = 0;
        } else {
            playfile.given_file_number = music_device_info[device_active].wBreak_point_filenum;
            if (playfile.given_file_number == 0) {
                playfile.given_file_number = 1;
            }
        }
#else
        playfile.given_file_number = 1;
#endif
        return GET_FILE_OK;
    }

    if ((playmode == REPEAT_ONE) && (searchMode != PLAY_AUTO_NEXT)) {
        playmode = REPEAT_ALL;					//在单曲循环模式下，转换成全循环模式
    }
    switch (playmode) {
#ifdef RANDOM_PLAY_EN
    case REPEAT_RANDOM:
        playfile.given_file_number = get_u16_random();
        playfile.given_file_number = (playfile.given_file_number % fileTotal) + 1;
        break;
#endif

    case REPEAT_ALL:
        if (searchMode == PLAY_PREV_FILE) {				//prev file
            playfile.given_file_number--;
            if (playfile.given_file_number == 0) {
                return GET_DEVICE_HEAD;
            }
        } else {				              //next file
            playfile.given_file_number++;
            if (playfile.given_file_number > fileTotal) {
                playfile.given_file_number = 1;
                return GET_DEVICE_END;
            }
        }
        break;
#ifdef FOLDER_PLAY_EN
    case REPEAT_FOLDER: {
        fs_get_folferinfo(&start_num, &end_num);
        //deg("X-playfile.given_file_number : %04x\n", playfile.given_file_number);
        if (searchMode == PLAY_PREV_FILE) {
            playfile.given_file_number--;
            if (playfile.given_file_number <= start_num) {
                playfile.given_file_number = end_num;
            }
            //deg("2-playfile.given_file_number : %04x\n", playfile.given_file_number);
        } else {
            playfile.given_file_number++;
            if (playfile.given_file_number > end_num) {
                playfile.given_file_number = start_num;
            }
            //deg("3-playfile.given_file_number : %04x\n", playfile.given_file_number);
        }
    }
    break;
#endif
    default:
        break;
    }

    return GET_FILE_OK;
}




