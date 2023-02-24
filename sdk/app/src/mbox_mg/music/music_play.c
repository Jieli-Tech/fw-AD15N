#include "config.h"
#include "msg.h"
#include "common/mbox_common.h"
#include "music_play.h"
#include "play_file.h"
#include "device.h"
#include "common/hot_msg.h"
#include "get_music_file.h"
#include "break_point.h"
#include "common/ui/ui_api.h"
#include "common/hot_msg.h"
#include "mbox_main.h"
#include "decoder_api.h"
#include "vm.h"
#include "key.h"
#include "eq.h"
#include "music_auto_save_bp.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"



MUSIC_PLAY_VAR Music_Play_var;
DEVICE_INFO music_device_info[MAX_DEVICE + 1];
const u16 music_noflush_msg_table[] = {
    MSG_MUSIC_NEW_DEVICE_IN,
    MSG_CHANGE_WORK_MODE,
    MSG_NEXT_WORKMODE,
    MSG_MUSIC_SELECT_NEW_DEVICE,
};

static u16 music_msg_filter(u8 key_status, u8 key_num, u8 key_type)
{
    u16 msg = NO_MSG;
    switch (key_type) {
#if KEY_IO_EN
    case KEY_TYPE_IO:
        msg = iokey_msg_mbox_music_table[key_status][key_num];
        break;
#endif
#if KEY_AD_EN
    case KEY_TYPE_AD:
        msg = adkey_msg_mbox_music_table[key_status][key_num];
        break;
#endif

#if KEY_IR_EN
    case KEY_TYPE_IR:
        msg = irff00_msg_music_table[key_status][key_num];
        break;
#endif

    default:
        break;
    }

    return msg;
}

static bool is_noflush_msg(u16 msg)
{
    for (int i = 0; i < sizeof(music_noflush_msg_table) / sizeof(music_noflush_msg_table[0]); i++) {
        if (msg == music_noflush_msg_table[i]) {
            return true;
        }
    }

    return false;
}

void music_dev_file_total_clear(void)
{
    for (int i = 0; i <= MAX_DEVICE; i++) {
        music_device_info[i].wfileTotal = 0;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief    MUSIC 清除非music_noflush_msg_table中包含的所有消息
   @param    无
   @return   无
   @note     void music_flush_all_msg(void)
*/
/*----------------------------------------------------------------------------*/
static void music_flush_all_msg(void)
{
    int msg[2];
    int ret = MSG_NO_ERROR;

    do {
        ret = get_msg(2, &msg[0]);
        if (is_noflush_msg((u16)msg[0])) {
            post_msg(1, msg[0]);
            break;
        }
    } while (ret == MSG_NO_ERROR && msg[0] != NO_MSG);

}

/*----------------------------------------------------------------------------*/
/** @brief: Music 模式信息初始化
    @param: void
    @return:void
    @author:
    @note:  void music_app_init(void)
*/
/*----------------------------------------------------------------------------*/
static void music_info_init(void)
{
    /*------------System configuration-----------*/
    Sys_IRInput = 1;
    Sys_Volume = 1;

    device_active = NO_DEVICE;

    music_dev_file_total_clear();
    fs_file_close();
    if (playfile.given_device == NO_DEVICE) {
        u8 mem_dev = 0;
        vm_read(VM_INDEX_ACTIVE_DEV, &mem_dev, sizeof(mem_dev));
        playfile.given_device = mem_dev;
        playfile.given_device = (playfile.given_device > MAX_DEVICE) ? DEVICE_UDISK : playfile.given_device;
    }

    if (playfile.given_device == DEVICE_UDISK) {
        playfile.given_file_method = PLAY_BREAK_POINT;
        playfile.given_device = DEVICE_UDISK;
        put_msg_lifo(MSG_MUSIC_SELECT_NEW_DEVICE);
    }
#if TCFG_USB_EXFLASH_UDISK_ENABLE
    else if (playfile.given_device == DEVICE_EXTFLSH) {
        playfile.given_file_method = PLAY_BREAK_POINT;
        playfile.given_device = DEVICE_EXTFLSH;
        put_msg_lifo(MSG_MUSIC_SELECT_NEW_DEVICE);
    }
#endif
#ifdef SDMMCB_EN
    else if (playfile.given_device == DEVICE_SDMMCB) {
        playfile.given_file_method = PLAY_BREAK_POINT;
        playfile.given_device = DEVICE_SDMMCB;
        put_msg_lifo(MSG_MUSIC_SELECT_NEW_DEVICE);
    }
#endif
#ifdef SDMMCA_EN
    else if (playfile.given_device == DEVICE_SDMMCA) {
        playfile.given_file_method = PLAY_BREAK_POINT;
        playfile.given_device = DEVICE_SDMMCA;
        put_msg_lifo(MSG_MUSIC_SELECT_NEW_DEVICE);
    }
#endif
    else {
        playfile.given_device = DEVICE_UDISK;
        playfile.given_file_method = PLAY_FIRST_FILE;
        put_msg_lifo(MSG_MUSIC_SELECT_NEW_DEVICE);
    }

    /*--------Music UI*/
    SET_UI_MAIN(MENU_MUSIC_MAIN);
    //UI_menu(MENU_WAIT);
    key_table_sel(music_msg_filter);
}

/*----------------------------------------------------------------------------*/
/** @brief: 文件播放函数
    @param: file_break_point：断点记忆控制位
    @return:播放成功标志位
    @author:
    @note:  bool play_device_file(bool file_break_point)
*/
/*----------------------------------------------------------------------------*/
bool play_device_file(bool file_break_point)
{
    bool bres;

    bres = decode_init(file_break_point);
    /*Main UI recovery*/
    SET_UI_MAIN(MENU_MUSIC_MAIN);

    return bres;
}


/*----------------------------------------------------------------------------*/
/** @brief: Music 模式主循环
    @param: void
    @return:void
    @author:
    @note:  void music_app_loop(void)
*/
/*----------------------------------------------------------------------------*/
void music_app_loop(void)
{
    u8 res;
    bool bres;
    u32 temp_jiffies = jiffies;;
    dac_mute(0);

    while (1) {
        u16 key;
        key = app_get_msg();
        bsp_loop();

        switch (key) {
        case MSG_CHANGE_WORK_MODE:
#if defined BREAK_POINT_PLAY_EN
            save_music_break_point(device_active, 1);
#endif
            return;

        case MSG_MUSIC_NEW_DEVICE_IN:							//有新设备接入
            log_info(">>>>>func:%s line:%d \n", __func__, __LINE__);
            if (decode_is_enable() && (playfile.given_device == device_active)) {
                log_info(">>>>>func:%s line:%d \n", __func__, __LINE__);
                break;
            }
#if defined BREAK_POINT_PLAY_EN
            save_music_break_point(device_active, 1);
#endif
        case MSG_MUSIC_SELECT_NEW_DEVICE:					    //重新选择设备
            log_info(">>>>>func:%s line:%d \n", __func__, __LINE__);
            //deg("0-MSG_MUSIC_SELECT_NEW_DEVICE %02x\n", playfile.given_device);
            if (!device_any_online()) {
                put_msg_lifo(MSG_NEXT_WORKMODE);
                break;
            }

            UI_menu(MENU_WAIT);
            stop_decode();
            fs_file_close();
            res = find_device(playfile.given_device);
            if ((res == DEV_INIT_ERR) ||
                (res == NO_DEFINE_DEV)) {                  //指定的设备不在线，或初始化失败
                if (!device_active_is_online()) {
                    //break;
                }
                playfile.given_device = DEVICE_AUTO_NEXT;			   //自动选择下一个设备
                put_msg_lifo(MSG_MUSIC_SELECT_NEW_DEVICE);
                break;
            } else if ((res == NO_EFFECTIVE_DEV) ||
                       (res == NO_DEV_ONLINE)) {                  //无可播放的设备
                put_msg_lifo(MSG_NEXT_WORKMODE);
                break;
            }
        //找到可用设备
        case MSG_MUSIC_SELECT_NEW_FILE:
            log_info(">>>>>func:%s line:%d \n", __func__, __LINE__);
            if (device_check() == 0) {                              //当前播放设备已经不在线,则自动选取下一个设备
                playfile.given_device = DEVICE_AUTO_NEXT;
                put_msg_lifo(MSG_MUSIC_SELECT_NEW_DEVICE);
                break;
            }

            res = fs_get_filenum(playfile.play_mode, playfile.given_file_method);

            //deg("1-MSG_MUSIC_SELECT_NEW_FILE : %02x\n", res);
            if (res == GET_DEVICE_END) {							//下一曲到设备的结束，查找下一个设备的第一首
                playfile.given_file_method = PLAY_FIRST_FILE;
                playfile.given_device = DEVICE_AUTO_NEXT;
                put_msg_lifo(MSG_MUSIC_SELECT_NEW_DEVICE);
                //music_device_info[device_active].wBreak_point_filenum = 1;
#ifdef BREAK_POINT_PLAY_EN
                clear_music_break_point(device_active);
#endif
                music_flush_all_msg();
                break;
            } else if (res == GET_DEVICE_HEAD) {							//上一曲到设备的结束，查找下一个设备的最后一首
                playfile.given_file_method = PLAY_LAST_FILE;
                playfile.given_device = DEVICE_AUTO_PREV;
                put_msg_lifo(MSG_MUSIC_SELECT_NEW_DEVICE);
                music_flush_all_msg();
                break;
            }
#if defined BREAK_POINT_PLAY_EN
        case MSG_MUSIC_PLAY_NEW_FILE0:
            save_music_break_point(device_active, 0);
#endif
        case MSG_MUSIC_PLAY_NEW_FILE:
            log_info(">>>>>func:%s line:%d \n", __func__, __LINE__);
            stop_decode();
            if (playfile.given_file_method == PLAY_BREAK_POINT) {
                bres = play_device_file(1);
            } else {
                bres = play_device_file(0);
            }

            UI_menu(MENU_FILENUM);
            if (!bres) {
                if (playfile.given_file_method <= PLAY_BREAK_POINT) {	//重新找下一首
                    playfile.given_file_method = PLAY_NEXT_FILE;
                    put_msg_lifo(MSG_MUSIC_SELECT_NEW_FILE);
                } else {										          //重新找上一首
                    playfile.given_file_method = PLAY_PREV_FILE;
                    put_msg_lifo(MSG_MUSIC_SELECT_NEW_FILE);
                }
                break;
            } else {
                vm_write(VM_INDEX_ACTIVE_DEV, &device_active, sizeof(device_active));
                //deg("0-Get Device %02x\n", get_memory(MEM_ACTIVE_DEV));
                music_device_info[device_active].wBreak_point_filenum = playfile.given_file_number;
#ifdef LAST_MEM_FILE_PLAY_EN
                if (playfile.given_file_method != PLAY_BREAK_POINT) {
                    save_music_break_point(device_active, 0);
                }
#endif
            }
            break;
        case MSG_MP3_FILE_ERR:
        case MSG_WAV_FILE_ERR:
            log_info("decode error !! \n");
            playfile.given_file_method = PLAY_AUTO_NEXT;
            if (device_active_is_online()) {
                put_msg_lifo(MSG_MUSIC_SELECT_NEW_FILE);
            }
            break;

        case MSG_WAV_FILE_END:
        case MSG_MP3_FILE_END:
        case MSG_F1A1_FILE_END:
        case MSG_F1A2_FILE_END:
        case MSG_MIDI_FILE_END:
        case MSG_DECODE_FILE_END:                               //解码自动至下一首
            log_info(">>>>>func:%s line:%d \n", __func__, __LINE__);
            log_info("file play end \n");
            if (device_active_is_online()) {
                playfile.given_file_method = PLAY_AUTO_NEXT;
                put_msg_lifo(MSG_MUSIC_SELECT_NEW_FILE);
            } else {
                log_info("************************ device offline \n");
            }
            break;

        case MSG_MUSIC_PREV_FILE:												//选择上一个文件进行播放;
            playfile.given_file_method = PLAY_PREV_FILE;
            put_msg_lifo(MSG_MUSIC_SELECT_NEW_FILE);
            break;

        case MSG_MUSIC_NEXT_FILE:												//选择下一个文件进行播放
            playfile.given_file_method = PLAY_NEXT_FILE;
            put_msg_lifo(MSG_MUSIC_SELECT_NEW_FILE);
            break;

        case MSG_MUSIC_PP:	                                                    //暂停和播放
#if UI_ENABLE
            if (UI_var.bCurMenu == MENU_INPUT_NUMBER) {
                put_msg_lifo(MSG_INPUT_TIMEOUT);
                break;
            }
#endif
            decode_pp();
            if (Music_Play_var.bPlayStatus == MAD_PLAY) {
                SET_UI_MAIN(MENU_MUSIC_MAIN);
                UI_menu(MENU_MUSIC_MAIN);
            } else if (Music_Play_var.bPlayStatus == MAD_PAUSE) {
                SET_UI_MAIN(MENU_PAUSE);
                UI_menu(MENU_PAUSE);
            }
            break;

        case MSG_MUSIC_FF:
            log_info("ff");
            decode_ff(2);
            UI_menu(MENU_HALF_SEC_REFRESH);
            SET_UI_MAIN(MENU_MUSIC_MAIN);
            UI_menu(MENU_MUSIC_MAIN);
            break;

        case MSG_MUSIC_FR:
            log_info("fr");
            decode_fr(2);
            UI_menu(MENU_HALF_SEC_REFRESH);
            SET_UI_MAIN(MENU_MUSIC_MAIN);
            UI_menu(MENU_MUSIC_MAIN);
            break;

        case MSG_MUSIC_NEXT_EQ:
            if (Music_Play_var.bPlayStatus == MAD_PAUSE) {
                break;
            }

            Music_Play_var.bEQ++;
            if (Music_Play_var.bEQ >= EQ_MODEMAX) {
                Music_Play_var.bEQ = EQ_ZERO;
            }
            eq_set_mode(Music_Play_var.bEQ);
            UI_menu(MENU_EQ);
            break;

        case MSG_NEXT_PLAYMODE:
            playfile.play_mode++;
            if (playfile.play_mode >= MAX_PLAY_MODE) {
                playfile.play_mode = REPEAT_ALL;
            }
            log_info("playmode:%d \n", playfile.play_mode);
            UI_menu(MENU_PLAYMODE);
            break;

        case MSG_INPUT_TIMEOUT:
            /*由红外界面返回*/
            if ((input_number <= music_device_info[device_active].wfileTotal) && (input_number != 0)) {
                playfile.given_file_number = input_number;
                playfile.given_file_method = PLAY_FIRST_FILE;
                put_msg_lifo(MSG_MUSIC_PLAY_NEW_FILE);
            } else {
                UI_menu(MENU_MUSIC_MAIN);
            }

            input_number = 0;
            break;

        case MSG_500MS:
#if BREAK_POINT_AUTO_SAVE
            masb_bp_save_scan();
#endif
            if ((Music_Play_var.bPlayStatus != MAD_PLAY) && (0 !=  decode_is_enable())) {
                idle_check_deal(0);
            }

            if (jiffies - temp_jiffies > 25) {
                temp_jiffies = jiffies;
                LED_FADE_OFF();
                UI_menu(MENU_MAIN);
                if (Music_Play_var.bPlayStatus != MAD_PLAY) {
                    music_energy = 0;
                }
                UI_menu(MENU_HALF_SEC_REFRESH);
            } else {
                log_char('M');
            }
        default :
            ap_handle_hotkey(key);
            break;
        }
    }
}

void music_app(void)
{
    if (device_any_online()) {
        log_info("mbox music \n");
        /* dac_init_api(32000); */
        work_mode_save();
        decoder_init();
        music_info_init();
        music_app_loop();
        stop_decode();
        fs_file_close();
        /* playfile.given_device = NO_DEVICE; */
    } else {
        work_mode++;
    }
}
