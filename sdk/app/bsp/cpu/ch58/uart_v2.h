#ifndef DEVICE_UART_H
#define DEVICE_UART_H

#include "typedef.h"
// #include "device/device.h"
#include "ioctl.h"
#include "jiffies.h"
// #include "system/task.h"
// #include "cpu/includes.h"
#include "uart_types.h"

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

#if CONFIG_ENABLE_UART_OS_SEM
typedef OS_SEM UT_Semaphore ;
static inline int UT_OSSemCreate(UT_Semaphore *sem, u32 count)
{
    return os_sem_create(sem, count);
}
static inline int UT_OSSemPost(UT_Semaphore *sem)
{
    return os_sem_post(sem);
}
static inline int UT_OSSemPend(UT_Semaphore *sem, u32 timeout)
{
    return os_sem_pend(sem, timeout);
}
static inline int UT_OSSemSet(UT_Semaphore *sem, u32 count)
{
    return os_sem_set(sem, count);
}
static inline int UT_OSSemClose(UT_Semaphore *sem, u32 block)
{
    return os_sem_del(sem, block);
}
static inline void ut_sleep()
{
    os_time_dly(1);
}

typedef OS_MUTEX UT_mutex;
static inline int UT_OSMutexCreate(UT_mutex *mutex)
{
    return os_mutex_create(mutex);
}
static inline int UT_OSMutexPost(UT_mutex *mutex)
{
    return os_mutex_post(mutex);
}
static inline int UT_OSMutexPend(UT_mutex *mutex, u32 timeout)
{
    return os_mutex_pend(mutex, timeout);
}
static inline int UT_OSMutexClose(UT_mutex *mutex, u32 block)
{
    return os_mutex_del(mutex, block);
}

#else

typedef volatile u32 UT_Semaphore;
extern void wdt_clear();
static inline int UT_OSSemCreate(UT_Semaphore *sem, u32 count)
{
    *sem = count;
    return 0;
}
static inline int UT_OSSemPost(UT_Semaphore *sem)
{
    (*sem)++;
    return 0;
}
static inline int UT_OSSemPend(UT_Semaphore *sem, u32 timeout)
{
    timeout = ut_msecs_to_jiffies(timeout);
    u32 _timeout = timeout + ut_get_jiffies();
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
static inline int UT_OSSemSet(UT_Semaphore *sem, u32 count)
{
    *sem = count;
    return 0;
}
static inline int UT_OSSemClose(UT_Semaphore *sem, u32 block)
{

    return 0;
}
static inline void ut_sleep()
{
    wdt_clear();
}

typedef volatile u32 UT_mutex;
static inline int UT_OSMutexCreate(UT_mutex *mutex)
{
    *mutex = 1;
    return 0;
}
static inline int UT_OSMutexPost(UT_mutex *mutex)
{
    (*mutex) = 1;
    return 0;
}
static inline int UT_OSMutexPend(UT_mutex *mutex, u32 timeout)
{
    timeout = ut_msecs_to_jiffies(timeout);
    u32 _timeout = timeout + ut_get_jiffies();
    while (1) {
        if (*mutex) {
            (*mutex) = 0;
            break;
        }
        if ((timeout != 0) && time_before(_timeout, ut_get_jiffies())) {
            return -1;
        }
        wdt_clear();
    }
    return 0;
}
static inline int UT_OSMutexClose(UT_mutex *mutex, u32 block)
{
    return 0;
}
#endif



extern void putbyte(char a);
typedef const int uart_dev;

enum uart_parity {
    UART_PARITY_DISABLE  = 0x0,
    UART_PARITY_all_0,
    UART_PARITY_all_1,
    UART_PARITY_EVEN,
    UART_PARITY_ODD
};

enum uart_event {
    UART_EVENT_RX_DATA = 1,
    UART_EVENT_RX_FIFO_OVF = 2,       //接收缓冲区溢出
    UART_EVENT_RX_TIMEOUT = 4,        //数据帧接收完成
    UART_EVENT_PARITY_ERR = 8,        //奇偶校验错误
    UART_EVENT_TX_DONE = 16,          //数据发送完成
};

struct uart_config {
    u32 baud_rate;
    u16 tx_pin;
    u16 rx_pin;      //当rx_pin == tx_pin 的时候为单线半双工通讯模式
    enum uart_parity parity;
};


struct uart_dma_config {
    u32 rx_timeout_thresh;//us
    u32 frame_size;
    u32 event_mask;
    void (*irq_callback)(uart_dev uart_num, enum uart_event);      //推送到调用者的线程执行

    void *rx_cbuffer;
    u32 rx_cbuffer_size;
};


struct uart_flow_ctrl {
    u16 cts_pin;
    u16 rts_pin;
    u8 cts_idle_level;     //0:cts空闲电平为低; 1:cts空闲电平为高
    u8 rts_idle_level;     //0:rts空闲电平为低; 1:rts空闲电平为高
    u32 rx_thresh;         // 0~100 %
};


//uart_num = -1则自动分配串口号，返回值为负数，则初始化失败
s32 uartx_init(uart_dev uart_num, const struct uart_config *config);
s32 uart_deinit(uart_dev uart_num);

s32 uart_dma_init(uart_dev uart_num, const struct uart_dma_config *dma_config);

//return 实际波特率,0:error; >0:ok
s32 uart_set_baudrate(uart_dev uart_num, u32 baud_rate);

s32 uart_set_rx_timeout_thresh(uart_dev uart_num, u32 timeout_thresh);

//发送数据按位取反后发出
//tx_inv_en:1:取反,0:不取反
void uart_tx_data_inv(uart_dev uart_num, u8 tx_inv_en);
//接收数据按位取反后接收
//rx_inv_en:1:取反,0:不取反
void uart_rx_data_inv(uart_dev uart_num, u8 rx_inv_en);

//启动dma发送，要求buffer指向的地址是允许dma的，
//函数启动dma后，立即返回，不会等待数据发送完成
//要求buffer指向的空间在发送结束前是有效的
s32 uart_send_bytes(uart_dev uart_num, const void *buffer, u32 tx_size);

// 等待串口发送完成，timeout_ms = 0 就一直等
s32 uart_wait_tx_idle(uart_dev uart_num, u32 timeout_ms);

//阻塞式dma发送数据 要求src指向的地址是允许dma的，
//函数会等到数据发送完成，再返回
//timeout_ms = 0就一直等直到发送完成
s32 uart_send_blocking(uart_dev uart_num, const void *buffer, u32 tx_size, u32 timeout_ms);

//从缓冲区读取数据，返回实际读取的长度
s32 uart_recv_bytes(uart_dev uart_num, void *buffer, u32 len);

//从缓冲区读取数据，直到读取到有效长度的数据，或者超时
//返回实际读取的长度
s32 uart_recv_blocking(uart_dev uart_num, void *buffer, u32 len, u32 timeout_ms);

//dma接收错乱时重置dma及buf，重新接收
void uart_dma_rx_reset(uart_dev uart_num);

//流控配置，对于没有硬件流控的uart，则使用软件实现，会有比较大的滞后，设置的阈值余量要足够
s32 uart_flow_ctrl_init(uart_dev uart_num, const struct uart_flow_ctrl *flow_ctrl);
s32 uart_flow_ctrl_deinit(uart_dev uart_num);

s32 uart_putbyte(uart_dev uart_num, u32 a);
s32 uart_getbyte(uart_dev uart_num);
#endif

