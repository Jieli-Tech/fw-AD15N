#ifndef _VM_H_
#define _VM_H_
#include "typedef.h"
#include "ioctl.h"
#include "dev_mg/device.h"

#define IOCTL_SET_VM_INFO               _IOW('V', 1, 1)
#define IOCTL_GET_VM_INFO               _IOW('V', 2, 1)

typedef enum {
    VM_INDEX_DEMO = 0,//用户可以使用
    VM_INDEX_SONG,
    VM_INDEX_ENG,
    VM_INDEX_POETRY,
    VM_INDEX_STORY,
    VM_INDEX_F1X,
    VM_INDEX_EXT_SONG,
    VM_INDEX_VOL,

#if 1
    VM_INDEX_SONG_BP,
    VM_INDEX_ENG_BP,
    VM_INDEX_POETRY_BP,
    VM_INDEX_STORY_BP,
    VM_INDEX_F1X_BP,
    VM_INDEX_EXT_SONG_BP,
#endif
    VM_INDEX_LP_SUM_CNT,
    VM_INDEX_LP_LAST_CNT,
    VM_INDEX_RTC_TIME,
    VM_INDEX_ALM_TIME,

    /*mbox*/
    VM_INDEX_SYSMODE,
    VM_INDEX_UDISK_BP,
    VM_INDEX_UDISK_INDEX,
    VM_INDEX_SDMMCA_BP,
    VM_INDEX_SDMMCA_INDEX,
    VM_INDEX_SDMMCB_BP,
    VM_INDEX_SDMMCB_INDEX,
    VM_INDEX_EXTFLSH_BP,
    VM_INDEX_EXTFLSH_INDEX,
    VM_INDEX_ACTIVE_DEV,
    VM_INDEX_FM_FRE,
    VM_INDEX_FM_CHAN,
    VM_INDEX_FM_CHANNL,//VM_INDEX_FM_CHANNL和VM_INDEX_FM_CHANNL_END之间不能插入其它INDEX
    VM_INDEX_FM_CHANNL_END = VM_INDEX_FM_CHANNL + 28,
    /*mbox*/
    VM_INDEX_FLASH_UPDATE,
    VM_INDEX_AUTO_BP,
    VM_INDEX_MAX = 128,
} VM_INDEX;

// vm_sfc api
typedef u32(*flash_code_protect_cb_t)(u32 offset, u32 len);
u32 flash_code_protect_callback(u32 offset, u32 len);
extern volatile u8 vm_busy;
void spi_cache_way_switch(u8 way_num);
// vm擦写时可放出多个中断
void vm_isr_response_list_register(u32 bit_list);
// 兼容旧程序,旧程序vm擦写时只能放出一个中断!
#define vm_isr_response_index_register(index) vm_isr_response_list_register(BIT(index) | BIT(IRQ_AUDIO_IDX))
int vm_read(u32 id, u8 *data_buf, u16 len);
int vm_write(u32 id, u8 *data_buf, u16 len);
#endif  //_VM_H_

