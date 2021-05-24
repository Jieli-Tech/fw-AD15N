#ifndef _UART_DEV_H_
#define _UART_DEV_H_
/*#include "typedef.h"*/
/*#include "os/os_api.h"*/
#include "jiffies.h"
#include "irq.h"
/* #include "jtime.h" */
#include "typedef.h"
#include "wdt.h"
#include "includes.h"
#include "gpio.h"



#define	CONFIG_ENABLE_UART_SEM	0
#ifndef SET_INTERRUPT
#define     SET_INTERRUPT   ___interrupt
#endif
#define irq_disable(x)  bit_clr_ie(x)
#define irq_enable(x)  bit_set_ie(x)
#ifndef time_after
#define time_after(a,b)  (((long)(b) - (long)(a)) < 0)
#endif
#ifndef time_before
#define time_before(a,b) time_after(b,a)
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
static inline u32 ut_get_jiffies(void)
{
#if 1
    return jiffies;
#endif
#if 0
    return Jtime_updata_jiffies();
#endif
}
static inline u32 ut_msecs_to_jiffies(u32 msecs)
{
    if (msecs >= 10) {
        msecs /= 10;
    } else if (msecs) {
        msecs = 1;
    }
    return msecs;
}
#if CONFIG_ENABLE_UART_SEM
typedef OS_SEM UT_Semaphore ;
static inline void UT_OSSemCreate(UT_Semaphore *sem, u32 count)
{
    os_sem_create(sem, count);
}
static inline void UT_OSSemPost(UT_Semaphore *sem)
{
    os_sem_post(sem);
}
static inline u32 UT_OSSemPend(UT_Semaphore *sem, u32 timeout)
{
    return os_sem_pend(sem, timeout);
}
static inline void UT_OSSemSet(UT_Semaphore *sem, u32 count)
{
    os_sem_set(sem, count);
}
static inline void UT_OSSemClose(UT_Semaphore *sem)
{
}
static inline void ut_sleep()
{
    os_time_dly(1);
}
#else
typedef volatile u32 UT_Semaphore;
static inline void UT_OSSemCreate(UT_Semaphore *sem, u32 count)//初始化，打开
{
    *sem = count;
}
static inline void UT_OSSemPost(UT_Semaphore *sem)//中断处理
{
    (*sem)++;
}
static inline u32 UT_OSSemPend(UT_Semaphore *sem, u32 timeout)//接收；dma发   当为timeout=0时，死等
{
    u32 _timeout = timeout + ut_get_jiffies();
    extern void wdt_clear();
    while (1) {
        if (*sem) {
            (*sem) --;
            break;
        }
        if ((timeout != 0) && time_before(_timeout, ut_get_jiffies())) {
            return -1;
        }
        wdt_clear();
    }
    return 0;
}
static inline void UT_OSSemSet(UT_Semaphore *sem, u32 count)//接收；dma发
{
    *sem = count;
}
static inline void UT_OSSemClose(UT_Semaphore *sem)
{
}
static inline void ut_sleep()//接收等待
{
    extern void wdt_clear();
    wdt_clear();
}
#endif
typedef void (*ut_isr_cbfun)(void *ut_bus, u32 status);
struct uart_platform_data_t {
    u8 tx_pin;                                          ///< gpio.h -1
    u8 rx_pin;                                          ///< gpio.h -1
    void *rx_cbuf;                                      ///< DMAbufutNULL
    u32 rx_cbuf_size;                                   ///< buf,2bufNULL
    u32 frame_length;                                   ///< RT
    u32 rx_timeout;                                     ///< OTms
    ut_isr_cbfun isr_cbfun; 			                ///< utNULL
    void *argv;                                         ///< ut
    u32 is_9bit: 1;                                     ///< ut01
    u32 baud: 24;                                       ///< ut
};
/**
 * @brief buf
 */
typedef struct {
    u8 *buffer;                                         ///<buf
    u32 buf_size;                                       ///<buf
    u32 buf_in;                                         ///<buf
    u32 buf_out;                                        ///<buf
} KFIFO;
enum {
    UT_TX = 1,
    UT_RX,
    UT_RX_OT
};
/**
 * @brief ut
 */
typedef struct {
    ut_isr_cbfun isr_cbfun;                             ///< utNULL
    void *argv;                                         ///< ut,
    void (*putbyte)(char a);                            ///< utbyte
    u8(*getbyte)(u8 *buf, u32 timeout);                 ///< utbytebuftimeoutms01
    u32(*read)(u8 *inbuf, u32 len, u32 timeout);        ///< utinbuflentimeoutms
    void (*write)(const u8 *outbuf, u32 len);           ///< utoutbuflen
    void (*set_baud)(u32 baud);                         ///< utbaud
    u32 frame_length;
    u32 rx_timeout;
    KFIFO kfifo;                                        ///< utbuf
    UT_Semaphore  sem_rx;
    UT_Semaphore  sem_tx;
} uart_bus_t;
const uart_bus_t *uart_dev_open(const struct uart_platform_data_t *arg);
u32 uart_dev_close(uart_bus_t *ut);
////////////////////////////////////////////////////////////////////////////////
#endif

