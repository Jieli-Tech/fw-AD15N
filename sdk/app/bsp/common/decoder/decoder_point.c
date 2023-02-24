#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "decoder_api.h"
#include "crc16.h"
#include "errno-base.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

u32 dp_buff_len(void)
{
    dp_buff *p_dbuf = (void *)0;
    return sizeof(dp_buff) - ((u32)&p_dbuf->buff[0] - (u32)p_dbuf);
}

bool get_dp(dec_obj *obj, dp_buff *dbuff)
{
    decoder_ops_t *ops;

    if ((0 == obj) || (0 == dbuff)) {
        log_info(" get decoder point buf null\n");
        return false;
    }
    u32 t_size;
    u32 size = (u32)&dbuff->buff[0] - (u32)dbuff;
    size = sizeof(dp_buff) - size;

    ops = obj->dec_ops;

    bool bres;
    local_irq_disable();
    t_size = ops->need_bpbuf_size();
    if (t_size <= size) {
        memcpy(&dbuff->buff[0], (void *)ops->get_bp_inf(obj->p_dbuf), t_size);
        dbuff->len = t_size;
        dbuff->crc = CRC16(&dbuff->len, dbuff->len + 2);
        /* log_info("get docoder_point, crc: 0x%x\n", dbuff->crc); */
        /* log_info_hexdump(&dbuff->buff[0], t_size); */
        bres = true;
    } else {
        log_info(" err dp size %d %d\n", t_size, size);
        bres = false;
    }
    local_irq_enable();

    return bres;
}
void *check_dp(dp_buff *dbuff)
{
    u16 crc;
    if (NULL == dbuff) {
        log_info("  check decoder point buf null\n");
        return 0;
    }
    u32 len = dbuff->len;
    if ((0 == len) || (len > dp_buff_len())) {
        return 0;
    }
    crc = CRC16(&dbuff->len, dbuff->len + 2);
    if ((0 != crc) && (crc == dbuff->crc)) {
        /* log_info(" decoder point check succ\n"); */
        /* log_info_hexdump(&dbuff->buff[0], dbuff->len); */
        return &dbuff->buff[0];
    } else {
        log_info(" docoder_point err, crc: 0x%x 0x%x\n", crc, dbuff->crc);
        /* log_info_hexdump(&dbuff->buff[0], dbuff->len); */
        return 0;
    }
}
void clear_dp(dp_buff *dbuff)
{
    u16 crc;
    if (0 == dbuff) {
        return;
    }
    memset(&dbuff->crc, 0, sizeof(dp_buff) - 4);
}
void clear_dp_buff(void *buff)
{
    u16 crc;
    if (0 == buff) {
        return;
    }
    memset(buff, 0, sizeof(dp_buff) - 8);
}
