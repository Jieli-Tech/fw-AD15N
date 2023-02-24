/*--------------------------------------------------------------------------*/
/**@file    FM_API.c
   @brief   FM 模式功能接口函数
   @details
   @author  bingquan Cai
   @date    2012-8-30
   @note    AC109N
*/
/*----------------------------------------------------------------------------*/
#include "app_config.h"
#include "fm_api.h"
#include "fm_radio.h"

#ifdef FM_ENABLE
#include "common/hot_msg.h"
#include "msg.h"
#include "key.h"
#include "mbox_main.h"
#include "common/ui/ui_api.h"
#include "clock.h"
#include "vm.h"
#include "common/mbox_common.h"

#if	BK1080
#include "BK1080.h"
#endif

#if	QN8035
#include "QN8035.h"
#endif

#if RDA5807
#include "RDA5807.h"
#endif

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

void (* const fm_init[])(void) = {
#if	RDA5807
    rda5807_init,
#endif

#if	BK1080
    bk1080_init,
#endif

#if	KT0830EG
    init_KT0830,
#endif

#if	QN8035
    qn8035_init,
#endif

#if	AR1019
    init_AR1019,
#endif

};

bool (* const fm_set_fre[])(u16) = {
#if	RDA5807
    rda5807_set_fre,
#endif

#if BK1080
    bk1080_set_fre,
#endif

#if KT0830EG
    set_fre_KT0830,
#endif

#if QN8035
    qn8035_set_fre,
#endif

#if	AR1019
    set_freq_AR1019,
#endif
};

void (* const fm_power_off[])(void) = {
#if	RDA5807
    rda5807_powerdown,
#endif

#if BK1080
    bk1080_powerdown,
#endif

#if KT0830EG
    KT0830_PowerDown,
#endif

#if QN8035
    qn8035_powerdown,
#endif

#if	AR1019
    AR1019_powerdown,
#endif
};

bool (* const fm_read_id[])(void) = {
#if	RDA5807
    rda5807_read_id,
#endif

#if BK1080
    bk1080_read_id,
#endif

#if KT0830EG
    KT0830_Read_ID,
#endif

#if QN8035
    qn8035_read_id,
#endif

#if	AR1019
    AR1019_Read_ID,
#endif
};

void (* const fm_mute[])(u8) = {
#if	RDA5807
    rda5807_mute,
#endif

#if	BK1080
    bk1080_mute,
#endif

#if	KT0830EG
    KT0830_mute,
#endif

#if QN8035
    qn8035_mute,
#endif

#if	AR1019
    AR1019_mute,
#endif
};

/*----------------------------------------------------------------------------*/
/**@brief   FM模块初始化接口函数
   @param
   @return
   @note    void init_fm_rev(void)
*/
/*----------------------------------------------------------------------------*/
bool init_fm_rev(void)
{
    for (fm_mode_var.bAddr = 0; fm_mode_var.bAddr < (sizeof(fm_read_id) / 2); fm_mode_var.bAddr++) {
        if ((*fm_read_id[fm_mode_var.bAddr])()) {
            (* fm_init[fm_mode_var.bAddr])();
            return TRUE;
        }
    }

    return FALSE;
}


/*----------------------------------------------------------------------------*/
/**@brief   关闭FM模块电源
   @param
   @return
   @note    void fm_rev_powerdown(void
*/
/*----------------------------------------------------------------------------*/
void fm_rev_powerdown(void)
{
    (* fm_power_off[fm_mode_var.bAddr])();
}


/*----------------------------------------------------------------------------*/
/**@brief   设置一个FM频点的接口函数
   @param   mode = 0: 使用frequency中的值，= 1:频点加1， = 2:频点减1
   @return  1：有台；0：无台
   @note    bool set_fre(u16 fre, u8 mode)
*/
/*----------------------------------------------------------------------------*/
bool set_fre(u8 mode)
{
    fm_module_mute(1);

    if (mode == FM_FRE_INC) {
        fm_mode_var.wFreq++;
    } else if (mode == FM_FRE_DEC) {
        fm_mode_var.wFreq--;
    }
    if (fm_mode_var.wFreq > MAX_FRE) {
        fm_mode_var.wFreq = MIN_FRE;
    }
    if (fm_mode_var.wFreq < MIN_FRE) {
        fm_mode_var.wFreq = MAX_FRE;
    }

    return (* fm_set_fre[fm_mode_var.bAddr])(fm_mode_var.wFreq);
}


/*----------------------------------------------------------------------------*/
/**@brief   FM模块Mute开关
   @param   flag：Mute使能位
   @return  无
   @note    void fm_module_mute(u8 flag)
*/
/*----------------------------------------------------------------------------*/
void fm_module_mute(u8 flag)
{
    (* fm_mute[fm_mode_var.bAddr])(flag);
}

/*----------------------------------------------------------------------------*/
/**@brief 获取全部记录的频道
   @param 	无
   @return  频道总数
   @note  u8 get_total_mem_channel(void)
*/
/*----------------------------------------------------------------------------*/
u8 get_total_mem_channel(void)
{
    u8 i, j;
    u8 total;
    u8 fm_channl = 0;

    total = 0;
    for (i = 0; i < MEM_FM_LEN; i++) {
        vm_read(VM_INDEX_FM_CHANNL + i, &fm_channl, sizeof(fm_channl));
        j = fm_channl;
        total += mbox_get_one_count(j);
    }

    if (total > MAX_CHANNL) {
        total = MAX_CHANNL;
    }

    return total;
}
/*----------------------------------------------------------------------------*/
/**@brief 通过频道获取频点
   @param 	channel：频道
   @return  有效的频点偏移量
   @note  u8 get_fre_via_channle(u8 channel)
*/
/*----------------------------------------------------------------------------*/
u8 get_fre_via_channle(u8 channel)
{
    u8 i, j, k;
    u8 total;
    u8 fm_channl = 0;

    total = 0;
    for (i = 0; i < MEM_FM_LEN; i++) {
        vm_read(VM_INDEX_FM_CHANNL + i, &fm_channl, sizeof(fm_channl));
        j = fm_channl;

        for (k = 0; k < 8; k++) {
            if (j & (BIT(k))) {
                total++;
                if (total == channel) {
                    return i * 8 + k;		 //fre = MIN_FRE + return val
                }
            }
        }

    }
    return 0xff;							//find none
}
/*----------------------------------------------------------------------------*/
/**@brief 根据频点偏移量获取频道
   @param 	channel：频道
   @return  频道
   @note  u8 get_channel_via_fre(u8 fre)
*/
/*----------------------------------------------------------------------------*/
u8 get_channel_via_fre(u8 fre)
{
    u8 i, j, k;
    u8 total;
    u8 fm_channl = 0;

    total = 0;
    for (i = 0; i < MEM_FM_LEN; i++) {
        vm_read(VM_INDEX_FM_CHANNL + i, &fm_channl, sizeof(fm_channl));
        j = fm_channl;
        for (k = 0; k < 8; k++) {
            if (j & (BIT(k))) {
                total++;
                if (fre == (i * 8 + k)) {
                    return total;		 //return fre index
                }
            }
        }
    }
    return fm_mode_var.bFreChannel;						    //find none
}
/*----------------------------------------------------------------------------*/
/**@brief 根据频点偏移量保存到相应的频点位变量到EEPROM
   @param 	fre：频点偏移量
   @return  无
   @note  void save_fm_point(u8 fre)
*/
/*----------------------------------------------------------------------------*/
void save_fm_point(u8 fre)
{
    u8 i, j, k;
    u8 fm_channl = 0;

    i = fre / 8;
    k = fre % 8;

    vm_read(VM_INDEX_FM_CHANNL + i, &fm_channl, sizeof(fm_channl));
    j = fm_channl | BIT(k);
    vm_write(VM_INDEX_FM_CHANNL + i, &j, sizeof(fm_channl));
}
/*----------------------------------------------------------------------------*/
/**@brief 从EEPROM清除所有频点信息
   @param 	无
   @return  无
   @note  void clear_all_fm_point(void)
*/
/*----------------------------------------------------------------------------*/
void clear_all_fm_point(void)
{
    u8 i;
    u8 fm_channl = 0;

    for (i = VM_INDEX_FM_CHANNL; i <= (VM_INDEX_FM_CHANNL + MEM_FM_LEN); i++) {
        vm_write(i, &fm_channl, sizeof(fm_channl));
    }
}
/*----------------------------------------------------------------------------*/
/**@brief 保存频道
   @param 	无
   @return  无
   @note  u8 ch_save(void)
*/
/*----------------------------------------------------------------------------*/
void ch_save(void)
{
    save_fm_point(fm_mode_var.wFreq - MIN_FRE);
    fm_mode_var.bFreChannel = get_channel_via_fre(fm_mode_var.wFreq - MIN_FRE);
    fm_mode_var.bTotalChannel = get_total_mem_channel();
}

/*----------------------------------------------------------------------------*/
/**@brief  fm_scan 状态标志
   @param  flag=1 标志了进入搜台状态   flag=0；跳出搜台状态
   @return 无
   @note   void enter_fm_rev(void)
*/
/*----------------------------------------------------------------------------*/
void fm_scan_flag(bool flag)
{
    u8 fm_chan = 0;
    if (flag) {
        fm_chan = fm_mode_var.bFreChannel | 0x80;
        vm_write(VM_INDEX_FM_CHAN, &fm_chan, sizeof(fm_chan));
    } else {
        fm_chan = fm_mode_var.bFreChannel & 0x7f;
        vm_write(VM_INDEX_FM_CHAN, &fm_chan, sizeof(fm_chan));
    }
}

/*----------------------------------------------------------------------------*/
/**@brief  FM 模式信息初始化
   @param  无
   @return 无
   @note   void fm_info_init(void)
*/
/*----------------------------------------------------------------------------*/
void fm_info_init(void)
{
    /*------------System configuration-----------*/
    Sys_IRInput = 1;
    Sys_Volume = 1;


    vm_read(VM_INDEX_FM_FRE, (u8 *)&fm_mode_var.wFreq, sizeof(fm_mode_var.wFreq));

    if (fm_mode_var.wFreq > (MAX_FRE - MIN_FRE)) {
        fm_mode_var.wFreq = MIN_FRE;
    } else {
        fm_mode_var.wFreq += MIN_FRE;
    }

    fm_mode_var.bTotalChannel = get_total_mem_channel();

    if (!fm_mode_var.bTotalChannel) {
        fm_mode_var.bTotalChannel = 1;
    }

    vm_read(VM_INDEX_FM_CHAN, &fm_mode_var.bFreChannel, sizeof(fm_mode_var.bFreChannel));

    if (fm_mode_var.bFreChannel > MAX_CHANNL) {				//台号为1;总台数为1
        fm_mode_var.bFreChannel = 1;
        fm_mode_var.bTotalChannel = 1;
    } else if (!fm_mode_var.bFreChannel) {
        fm_mode_var.bFreChannel = 1;
    }

    fm_mode_var.bFreChannel = get_channel_via_fre(fm_mode_var.wFreq - MIN_FRE);

    if (0xff == fm_mode_var.bFreChannel) {
        fm_mode_var.bFreChannel = 1;
    }

    set_fre(FM_CUR_FRE);
    fm_module_mute(0);
    scan_mode = FM_SCAN_STOP;
    /*---------FM MAIN UI--------------*/
    SET_UI_MAIN(MENU_FM_MAIN);
    UI_menu(MENU_FM_MAIN);
}


/*----------------------------------------------------------------------------*/
/**@brief    全频段搜索
   @param    mode: 0全频段, 1:上一个有效频点， 2:下一个有效频点
   @return   0:未找到频点 1:搜索完成 2:退出FM模式 3：闹钟响起退出
   @note     u8 fm_scan(u8 mode)
*/
/*----------------------------------------------------------------------------*/
bool fm_scan(u8 mode)
{
    bool res;
    u16 fm_fre = 0;

#if QN8035
    if (qn8035_online) {
        qn8035_setch(4);
    }
#endif

    if (mode == FM_SCAN_PREV) {
        res = set_fre(FM_FRE_DEC);
    } else {
        res = set_fre(FM_FRE_INC);
    }

    UI_menu(MENU_FM_DISP_FRE);

    if (res) {					//找到一个台
        fm_module_mute(0);
        fm_fre = fm_mode_var.wFreq - MIN_FRE;
        vm_write(VM_INDEX_FM_FRE, (u8 *)&fm_fre, sizeof(fm_fre));
        save_fm_point(fm_mode_var.wFreq - MIN_FRE);
        fm_mode_var.bFreChannel = get_channel_via_fre(fm_mode_var.wFreq - MIN_FRE);
        fm_mode_var.bTotalChannel = get_total_mem_channel();
        UI_menu(MENU_FM_FIND_STATION);
        return true;
    }

    return false;
}
#endif

