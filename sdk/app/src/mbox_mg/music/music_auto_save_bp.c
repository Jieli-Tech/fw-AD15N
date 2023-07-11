#include "common/mbox_common.h"
#include "play_file.h"
#include "msg.h"
#include "break_point.h"
#include "music_play.h"
#include "device.h"
#include "mbox_main.h"
#include "get_music_file.h"
#include "decoder_api.h"
#include "vfs.h"
#include "dev_mg/device.h"
#include "eq.h"
#include "music_auto_save_bp.h"
#include "vfs.h"
#include "vm_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#define VM_MAX_SECTOR_SIZE (16 * (4*1024) * 2)
#define VM_REFRAG_LINE     80
#define VM_BP_SIZE		   (sizeof(dev_vm_index_t) + 4 + sizeof(dp_buff) + 4)///4:vm data head
#define MIN_SAVE_FRE		5//断点最小保存的频率，单位S

typedef struct {
    u32 save_fre;//单位s
    u32 time_cnt;
} masb_ctl_t;

static masb_ctl_t masb_ctl;

static void masb_vm_refrag(void)
{
#if (USE_OLD_VM == SYS_MEMORY_SELECT)
    u8 data = 0;
    vm_defrag_line_set(10);//设置vm空间整理的阀值,按照百分比计算，取值范围0~100
    vm_read(VM_INDEX_AUTO_BP, &data, 1);
    data ++;
    vm_write(VM_INDEX_AUTO_BP, &data, 1);
    vm_defrag_line_set(VM_REFRAG_LINE);//设置vm空间整理的阀值,按照百分比计算，取值范围0~100
#endif
}

void masb_run_init(u32 music_play_total_time)
{
#if (USE_OLD_VM == SYS_MEMORY_SELECT)
    masb_ctl.time_cnt = 0;
    if (music_play_total_time == 0) {
        masb_ctl.save_fre = 0;
        log_info("music_play_total_time error !!! \n");
        return;
    }

    u32 vm_area_len, vm_used_len, vm_available_len = 0;
    u8 need_refrag = 0;
    vm_get_area_using_info(&vm_area_len, &vm_used_len);
    if ((vm_area_len * VM_REFRAG_LINE / 100) > vm_used_len) {
        vm_available_len = (vm_area_len * VM_REFRAG_LINE / 100) - vm_used_len;
    }

    u32 bp_max_save_times = vm_available_len / VM_BP_SIZE;
    if (!bp_max_save_times || (music_play_total_time / bp_max_save_times + 1) > MIN_SAVE_FRE) {
        //need vm refrag
        masb_vm_refrag();

        vm_get_area_using_info(&vm_area_len, &vm_used_len);
        if ((vm_area_len * VM_REFRAG_LINE / 100) > vm_used_len) {
            vm_available_len = (vm_area_len * VM_REFRAG_LINE / 100) - vm_used_len;
        }

        bp_max_save_times = vm_available_len / VM_BP_SIZE;
    }

    u32 min_save_fre = (music_play_total_time / bp_max_save_times + 1);
    masb_ctl.save_fre = (min_save_fre > MIN_SAVE_FRE) ? min_save_fre : MIN_SAVE_FRE;
    log_info("music save fre:%d \n", masb_ctl.save_fre);
#endif
}

void masb_bp_save_scan(void)
{
#if (USE_OLD_VM == SYS_MEMORY_SELECT)
#define SCAN_FRE		500//ms
    if (masb_ctl.save_fre == 0) {
        return;
    }

    masb_ctl.time_cnt ++;
    if (masb_ctl.time_cnt * SCAN_FRE >= masb_ctl.save_fre * 1000) {
        log_info("music save fre:%d \n", masb_ctl.save_fre);
        masb_ctl.time_cnt = 0;
        save_music_break_point(device_active, 1);
    }
#endif
}


