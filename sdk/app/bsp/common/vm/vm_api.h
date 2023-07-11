#ifndef __VM_API_H__
#define __VM_API_H__
#include "typedef.h"

#define NO_VM			    0
#define USE_NEW_VM          1
#define USE_OLD_VM          2
#define SYS_MEMORY_SELECT   USE_NEW_VM

#if (SYS_MEMORY_SELECT == USE_NEW_VM)
//新版vm，支持预擦除
#include "new_vm.h"
#define syscfg_vm_init_api(addr, size)         	nvm_init_api(addr, size)
#define vm_read_api(id, buf, len)       		nvm_read_api(id, buf, len)
#define vm_write_api(id, buf, len)      		nvm_write_api(id, buf, len)
#define vm_pre_erase_api()              		nvm_erasure_next_api()

#elif (SYS_MEMORY_SELECT == USE_OLD_VM)
//旧版vm
#include "old_vm.h"
#define syscfg_vm_init_api(addr, size)			syscfg_old_vm_init(addr, size)
#define vm_read_api(id, buf, len)				old_vm_read(id, buf, len)
#define vm_write_api(id, buf, len)				old_vm_write(id, buf, len)
#define vm_pre_erase_api()

#else

#define syscfg_vm_init_api(addr, size)			-1
#define vm_read_api(id, buf, len)				-1
#define vm_write_api(id, buf, len)				-1
#define vm_pre_erase_api()
#endif

int syscfg_vm_init(u32 mem_addr, u32 mem_size);
int vm_read(u32 id, u8 *data_buf, u16 len);
int vm_write(u32 id, u8 *data_buf, u16 len);
void vm_pre_erase(void);
#endif
