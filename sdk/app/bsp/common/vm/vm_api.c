#include "vm_api.h"
#include "config.h"
#include "app_config.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[vm_api]"
#include "log.h"

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

int syscfg_vm_init(u32 mem_addr, u32 mem_size)
{
    return syscfg_vm_init_api(mem_addr, mem_size);
}

int vm_read(u32 id, u8 *data_buf, u16 len)
{
    return vm_read_api(id, data_buf, len);
}

int vm_write(u32 id, u8 *data_buf, u16 len)
{
    return vm_write_api(id, data_buf, len);
}

void vm_pre_erase(void)
{
    vm_pre_erase_api();
}
