#include "vm_api.h"
#include "config.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[vm_api]"
#include "log.h"

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
