#include "tab_read.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[sine]"
#include "log.h"

void tab_init(rtab_obj *stab, void *tab, u32 size)
{
    memset(stab, 0, sizeof(rtab_obj));
    stab->tab = tab;
    stab->size = size;

    //log_info("size %d\n", size);
    //log_info_hexdump(stab->tab, size * 2);

}
#ifndef CPU_SH57
AT(.audio_d.text.cache.L2)
u32 tab_read(void *buff, rtab_obj *stab, u32 len)
{
    u8 *rbuff = buff;
    u8 *rtab = stab->tab;
    u32 max_len = stab->size;
    u32 rlen = 0;
    u32 offset = stab->offset;
    while (len && stab->cnt) {
        rlen = max_len - offset;
        rlen = rlen > len ? len : rlen;
        memcpy(rbuff, &rtab[offset], rlen);
        len    -= rlen;
        offset += rlen;
        rbuff  += rlen;
        if (offset >= max_len) {
            offset = 0;
            if (((u16) - 1) != stab->cnt) {
                if (0 != stab->cnt) {
                    /* log_info("stab->cnt  %d\n", stab->cnt); */
                    stab->cnt--;
                    /* log_char('k'); */
                }
            }
        }
    }
    stab->offset = offset;
    /* log_info("remain len : %d\n",len); */
    return len;
}
#endif
