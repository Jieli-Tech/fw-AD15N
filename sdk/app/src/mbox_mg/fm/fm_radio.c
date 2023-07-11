/*--------------------------------------------------------------------------*/
/**@file    FM_radio.c
   @brief   FM 模式主循环
   @details
   @author  bingquan Cai
   @date    2012-8-30
   @note    AC109N
*/
/*----------------------------------------------------------------------------*/
#include "app_config.h"
#include "fm_radio.h"
#include "fm_api.h"
#include "common/hot_msg.h"
#include "msg.h"
#include "key.h"
#include "mbox_main.h"
#include "common/ui/ui_api.h"
#include "music/music_play.h"
#include "fm/fm_iic_dri.h"
#include "vm_api.h"
#include "fm_radio.h"
#include "audio.h"
#include "dac.h"
#include "audio_adc.h"
#include "dac_api.h"
#include "circular_buf.h"
#include "sound_effect_api.h"
#include "errno-base.h"
#ifdef LINEIN_EN
#include "line_in/line_in_mode.h"
#endif

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#ifdef FM_ENABLE
static cbuffer_t cbuf_aux_o AT(.aux_data);
static u8 obuf_aux_o[1024] AT(.aux_data) ;
static sound_out_obj aux_sound;

extern ENUM_WORK_MODE _data work_mode;
_no_init FM_MODE_VAR _data fm_mode_var;
_no_init u8 _data scan_mode;

const u16 fm_noflush_msg_table[] = {
    MSG_MUSIC_NEW_DEVICE_IN,
    MSG_CHANGE_WORK_MODE,
    MSG_NEXT_WORKMODE,
};

static u16 fm_msg_filter(u8 key_status, u8 key_num, u8 key_type)
{
    u16 msg = NO_MSG;
    switch (key_type) {
#if KEY_IO_EN
    case KEY_TYPE_IO:
        msg = iokey_msg_mbox_fm_table[key_status][key_num];
        break;
#endif
#if KEY_AD_EN
    case KEY_TYPE_AD:
        msg = adkey_msg_mbox_fm_table[key_status][key_num];
        break;
#endif

#if KEY_IR_EN
    case KEY_TYPE_IR:
        msg = irff00_msg_fm_table[key_status][key_num];
        break;
#endif

    default:
        break;
    }

    return msg;
}

extern const char MIC_PGA_G;
int fm_linein_init(void)
{
    u32 err;
    memset(&aux_sound, 0, sizeof(aux_sound));
    cbuf_init(&cbuf_aux_o, &obuf_aux_o[0], sizeof(obuf_aux_o));
    aux_sound.p_obuf = &cbuf_aux_o;

    ///BIT(0):ch0 PA13; BIT(1):ch1 PA14
    err = audio_adc_init_api(24000, ADC_LINE_IN, BIT(1));
    dac_sr_api(24000);
    /* dac_init_api(24000); */
    /* aux_test_audio(); */
    regist_dac_channel(&aux_sound, NULL); //注册到DAC;
    regist_audio_adc_channel(&aux_sound, NULL); //注册到DAC;
    audio_adc_enable(MIC_PGA_G);
    aux_sound.enable |= B_DEC_RUN_EN;
    return 0;
}
void fm_linein_off(void)
{
    aux_sound.enable = 0;
    audio_adc_off_api();
    unregist_audio_adc_channel(&aux_sound);
    unregist_dac_channel(&aux_sound);
    dac_sr_api(SR_DEFAULT);
}

static bool is_noflush_msg(u16 msg)
{
    for (int i = 0; i < sizeof(fm_noflush_msg_table) / sizeof(fm_noflush_msg_table[0]); i++) {
        if (msg == fm_noflush_msg_table[i]) {
            return true;
        }
    }

    return false;
}

/*----------------------------------------------------------------------------*/
/**@brief    FM 清除非fm_noflush_msg_table中包含的所有消息
   @param    无
   @return   无
   @note     void fm_flush_all_msg(void)
*/
/*----------------------------------------------------------------------------*/
static void fm_flush_all_msg(void)
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
/**@brief    FM 模式主循环
   @param    无
   @return   无
   @note     void fm_play(void)
*/
/*----------------------------------------------------------------------------*/
void fm_play(void)
{
    u8 scan_counter;
    u8 fm_chan = 0;
    u8 fm_fre = 0;
    u8 mute = 0;
    dac_mute(0);

    while (1) {
        u16 key = app_get_msg();
        bsp_loop();

        switch (key) {
        case MSG_MUSIC_NEW_DEVICE_IN:
            if (work_mode == FM_RADIO_MODE) {
                work_mode = MUSIC_MODE;
            }
        case MSG_CHANGE_WORK_MODE:
            return;

//        case MSG_STOP_SCAN:
//            scan_counter = 1;                   //再搜索一个频点就停止
//            break;
        case MSG_MUTE:
            if (mute) {
                mute = 0;
                dac_mute(0);
            } else {
                mute = 1;
                dac_mute(1);
            }
            break;

        case MSG_FM_SCAN_ALL_INIT:
            if (scan_mode == FM_SCAN_STOP) {
                fm_chan = 0;
                fm_fre = 0;
                vm_write(VM_INDEX_FM_CHAN, &fm_chan, sizeof(fm_chan));
                vm_write(VM_INDEX_FM_FRE, &fm_fre, sizeof(fm_fre));
                clear_all_fm_point();
                fm_mode_var.bTotalChannel = 0;
                fm_mode_var.bFreChannel = 0;
                fm_mode_var.wFreq = MIN_FRE - 1;					//自动搜索从最低的频点开始
                scan_counter = MAX_CHANNL;
                scan_mode = FM_SCAN_ALL;
            } else {
                scan_counter = 1;                   //再搜索一个频点就停止
            }

        case MSG_FM_SCAN_ALL:
            fm_flush_all_msg();
            if (fm_scan(scan_mode)) {
                if (scan_mode == FM_SCAN_ALL) {
                    delay_n10ms(100);    //播放1S
                } else {
                    scan_mode = FM_SCAN_STOP;
                    break;
                }
            }

            scan_counter--;
            if (scan_counter == 0) {
                if (scan_mode == FM_SCAN_ALL) {
                    //全频点搜索结束，播放第一个台
                    put_msg_lifo(MSG_FM_NEXT_STATION);
                    scan_mode = FM_SCAN_STOP;
                } else {                        //半自动搜索，播放当前频点
                    scan_mode = FM_SCAN_STOP;
                    fm_module_mute(0);
                    break;
                }
            } else {                                                //搜索过程中只响应事件
                put_msg_lifo(MSG_FM_SCAN_ALL);
            }
            break;

        case MSG_FM_SCAN_ALL_DOWN:
            scan_mode = FM_SCAN_PREV;
            put_msg_lifo(MSG_FM_SCAN_ALL);
            break;

        case MSG_FM_SCAN_ALL_UP:
            scan_mode = FM_SCAN_NEXT;
            put_msg_lifo(MSG_FM_SCAN_ALL);
            break;

        case MSG_FM_PREV_STEP:
            fm_flush_all_msg();
            set_fre(FM_FRE_DEC);
            fm_mode_var.bFreChannel = get_channel_via_fre(fm_mode_var.wFreq - MIN_FRE);						//查找该频点是否有记忆过
            fm_chan = fm_mode_var.bFreChannel;
            fm_fre = fm_mode_var.wFreq - MIN_FRE;
            vm_write(VM_INDEX_FM_CHAN, &fm_chan, sizeof(fm_chan));
            vm_write(VM_INDEX_FM_FRE, &fm_fre, sizeof(fm_fre));
            fm_module_mute(0);
            UI_menu(MENU_FM_MAIN);
            break;

        case MSG_FM_NEXT_STEP:
            fm_flush_all_msg();
            set_fre(FM_FRE_INC);
            fm_mode_var.bFreChannel = get_channel_via_fre(fm_mode_var.wFreq - MIN_FRE);						//查找该频点是否有记忆过
            fm_chan = fm_mode_var.bFreChannel;
            fm_fre = fm_mode_var.wFreq - MIN_FRE;
            vm_write(VM_INDEX_FM_CHAN, &fm_chan, sizeof(fm_chan));
            vm_write(VM_INDEX_FM_FRE, &fm_fre, sizeof(fm_fre));
            fm_module_mute(0);
            UI_menu(MENU_FM_MAIN);
            break;

        case MSG_FM_PREV_STATION:
            fm_flush_all_msg();
            if (fm_mode_var.bTotalChannel == 0) {
                break;
            }
            fm_mode_var.bFreChannel -= 2;
        case MSG_FM_NEXT_STATION:
            if (fm_mode_var.bTotalChannel == 0) {
                break;
            }
            fm_mode_var.bFreChannel++;

            if ((fm_mode_var.bFreChannel == 0) || (fm_mode_var.bFreChannel == 0xff)) {
                fm_mode_var.bFreChannel = fm_mode_var.bTotalChannel;
            } else if (fm_mode_var.bFreChannel > fm_mode_var.bTotalChannel) {
                fm_mode_var.bFreChannel = 1;
            }
            fm_mode_var.wFreq = get_fre_via_channle(fm_mode_var.bFreChannel) + MIN_FRE;				//根据台号找频点
            set_fre(FM_CUR_FRE);
            fm_chan = fm_mode_var.bFreChannel;
            fm_fre = fm_mode_var.wFreq - MIN_FRE;
            vm_write(VM_INDEX_FM_CHAN, &fm_chan, sizeof(fm_chan));
            vm_write(VM_INDEX_FM_FRE, &fm_fre, sizeof(fm_fre));
            fm_module_mute(0);
            UI_menu(MENU_FM_CHANNEL);
            break;

        case MSG_CH_SAVE:
            ch_save();
            break;

        case MSG_MUSIC_PP:
#if UI_ENABLE
            if (UI_var.bCurMenu == MENU_INPUT_NUMBER) { //暂停和播放
                put_msg_lifo(MSG_INPUT_TIMEOUT);
                break;
            }
#else
            break;
#endif

        case MSG_INPUT_TIMEOUT:
            /*由红外界面返回*/
            if (input_number <= MAX_CHANNL) {						//输入的是台号
                key = get_fre_via_channle(input_number);
                if (key != 0xff) {
                    fm_mode_var.wFreq = key + MIN_FRE;
                    fm_mode_var.bFreChannel = input_number;
                    set_fre(FM_CUR_FRE);
                    fm_module_mute(0);
                    UI_menu(MENU_FM_DISP_FRE);
                }
            } else if ((input_number >= MIN_FRE) && (input_number <= MAX_FRE)) { //输入的是频点
                fm_mode_var.wFreq = input_number;
                fm_mode_var.bFreChannel = get_channel_via_fre(fm_mode_var.wFreq - MIN_FRE);
                set_fre(FM_CUR_FRE);
                fm_module_mute(0);
            }
            input_number = 0;
            fm_chan = fm_mode_var.bFreChannel;
            fm_fre = fm_mode_var.wFreq - MIN_FRE;
            vm_write(VM_INDEX_FM_CHAN, &fm_chan, sizeof(fm_chan));
            vm_write(VM_INDEX_FM_FRE, &fm_fre, sizeof(fm_fre));
            UI_menu(MENU_FM_DISP_FRE);
            break;

        case MSG_500MS:
            LED_FADE_OFF();
            UI_menu(MENU_MAIN);

        default:
            ap_handle_hotkey(key);
            break;
        }

    }
}

/*----------------------------------------------------------------------------*/
/**@brief    FM 模式
   @param    无
   @return   无
   @note     void fm_mode(void)
*/
/*----------------------------------------------------------------------------*/
void fm_mode(void)
{
#ifdef LINEIN_EN
    if (line_in_online()) {
        work_mode++;
        log_info("linein online,switch to linein mode \n");
        return;
    }
#endif

    fm_i2c_init();
    if (init_fm_rev()) {
        work_mode_save();
        key_table_sel(fm_msg_filter);
        dac_mute(1);///关闭dac，消除fm init出现的突击声
        fm_linein_init();
        fm_info_init();
        dac_mute(0);
        fm_play();
        fm_rev_powerdown();
        fm_linein_off();
    } else {				// no fm module
        work_mode++;
    }

}
#endif
