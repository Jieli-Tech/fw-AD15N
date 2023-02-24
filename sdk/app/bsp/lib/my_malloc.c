
#include "my_malloc.h"


#define LOG_TAG_CONST       HEAP
#define LOG_TAG             "[malloc]"
#include "log.h"

/* void *pvPortMalloc( size_t xWantedSize ) */
/* u32 example(u32 dat) */
/* { */
/* if (dat & 0x80) { */
/* dat += 1; */
/* } else { */
/* dat <<= 1; */
/* } */
/* return dat; */
/* } */

const u16 configHEAP_BEST_SIZE = 100;

void *my_malloc(u32 size, mm_type xType)
{
    void *res;
    /* return pvPortMalloc(size); */
    res =  pvPortMalloc(size, xType);
    if (NULL != res) {
        log_info(" malloc res 0x%x, type 0x%x\n", (u32)res, xType);
        memset(res, 0, size);
    } else {
        log_info(" err malloc empty\n");
    }
    return res;
}

void *my_free(void *pv)
{
    if (NULL != pv) {
        vPortFree(pv);
    }
    return NULL;
}


extern const u8 _free_start[];
extern const u8 _free_end[];

void my_malloc_init(void)
{
    u32 len = (u32)&_free_end[0] - (u32)&_free_start[0]  + 1;
    log_info(" HEAP----: 0x%x; 0x%x\n", (u32)&_free_end[0], (u32)&_free_start[0]);
    memset((void *)&_free_start[0], 0, len);
    vPortInit((void *)&_free_start[0], len);
}
void xTaskResumeAll(void)
{
    ;
}
