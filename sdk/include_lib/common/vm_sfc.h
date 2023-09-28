#ifndef _VM_SFC_H_
#define _VM_SFC_H_

#include "typedef.h"
#include "ioctl.h"
#include "dev_mg/device.h"


#define IOCTL_SET_VM_INFO               _IOW('V', 1, 1)
#define IOCTL_GET_VM_INFO               _IOW('V', 2, 1)

// vm_sfc api
typedef u32(*flash_code_protect_cb_t)(u32 offset, u32 len);
u32 flash_code_protect_callback(u32 offset, u32 len);
extern volatile u8 vm_busy;
void spi_cache_way_switch(u8 way_num);
// vm擦写时可放出多个中断
void vm_isr_response_list_register(u32 bit_list);
// 兼容旧程序,旧程序vm擦写时只能放出一个中断!
// #define vm_isr_response_index_register(index) vm_isr_response_list_register(BIT(index) | BIT(IRQ_AUDIO_IDX))
void vm_isr_response_index_register(u8 index);
void vm_isr_response_index_unregister(u8 index);
u32 get_vm_isr_response_index_h(void);//获取放出中断的高32位(index 32-63)
u32 get_vm_isr_response_index_l(void);//获取放出中断的低32位(index 0-31)  AD14/15/17只有低32位

#endif
