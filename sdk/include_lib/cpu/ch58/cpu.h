
#ifndef ASM_CPU_H
#define ASM_CPU_H


#include "ch58.h"
#include "csfr.h"

#ifndef __ASSEMBLY__

typedef unsigned char   		u8, bool, BOOL;
typedef char            		s8;
typedef unsigned short  		u16;
typedef signed short    		s16;
typedef unsigned int    		u32;
typedef signed int      		s32;
typedef unsigned long long 		u64;
typedef u32						FOURCC;
typedef long long               s64;
typedef unsigned long long      u64;

#endif


#define ___trig         __asm__ volatile ("trigger")

#define ___idle         __asm__ volatile ("idle")

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 			0x3021
#endif
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 		0x4576
#endif
#define CPU_ENDIAN 			LITTLE_ENDIAN


#define CPU_CORE_NUM     1
#if CPU_CORE_NUM > 1
static inline int current_cpu_id()
{
    unsigned id;
    asm volatile("%0 = cnum" : "=r"(id) ::);
    return id ;
}
#else
static inline int current_cpu_id()
{
    return 0;
}
#endif


#define  CPU_INT_NESTING 	2


#ifndef __ASSEMBLY__


static inline int cpu_in_irq()
{
    int flag;
    __asm__ volatile("%0 = icfg" : "=r"(flag));
    return flag & 0xff;
}

static inline int cpu_irq_disabled()
{
    int flag;
    __asm__ volatile("%0 = icfg" : "=r"(flag));
    return (flag & 0x300) != 0x300;
}


static inline int data_sat_s16(int ind)
{
    __asm__ volatile(
        " %0 = sat16(%0)(s)  \t\n"
        : "=&r"(ind)
        : "0"(ind)
        :);
    return ind;
}

static inline u32 reverse_u32(u32 data32)
{
    __asm__ volatile("%0 = rev8(%0) \t\n" : "=&r"(data32) : "0"(data32) :);
    return data32;
}

static inline u32 reverse_u16(u16 data16)
{
    u32 retv;
    retv = ((u32)data16) << 16;
    __asm__ volatile("%0 = rev8(%0) \t\n" : "=&r"(retv) : "0"(retv) :);
    return retv;
}

static inline u32 rand32()
{
    return JL_RAND->R64L;
}

static inline u32 rand32_2()
{
    return JL_RAND->R64H;
}

#define __asm_sine(s64, precision) \
    ({ \
        u64 ret; \
        u8 sel = 0; \
        __asm__ volatile ("%0 = copex(%1) (%2)" : "=r"(ret) : "r"(s64), "i"(sel)); \
        ret = ret>>32; \
        ret;\
    })

void p33_soft_reset(void);
void latch_reset(void);
static inline void cpu_reset(void)
{
    latch_reset();
}

#define __asm_csync() \
    do { \
		asm volatile("csync;"); \
    } while (0)

#define __asm_btbclr() \
    do { \
        asm volatile("btbclr"); \
    } while (0)

#include "irq.h"
// #include "generic/printf.h"


#define arch_atomic_read(v)  \
	({ \
        __asm_csync(); \
		(*(volatile int *)&(v)->counter); \
	 })

#if 0
extern volatile int irq_lock_cnt[];

static inline void local_irq_disable()
{
    __builtin_pi32_cli();
    irq_lock_cnt[0]++;
}

static inline void local_irq_enable()
{
    if (--irq_lock_cnt[0] == 0) {
        __builtin_pi32_sti();
    }
}
#else
extern void __local_irq_disable();
extern void __local_irq_enable();
#define local_irq_disable __local_irq_disable
#define local_irq_enable __local_irq_enable
#endif


#define arch_spin_trylock(lock) \
	do { \
        __asm_csync(); \
		while ((lock)->rwlock); \
		(lock)->rwlock = 1; \
	}while(0)

#define arch_spin_unlock(lock) \
	do { \
		(lock)->rwlock = 0; \
	}while(0)


#define	CPU_SR_ALLOC() 	\
//	int flags

#define CPU_CRITICAL_ENTER()  \
	do { \
		local_irq_disable(); \
        __asm_csync(); \
	}while(0)


#define CPU_CRITICAL_EXIT() \
	do { \
		local_irq_enable(); \
	}while(0)


#define OS_SR_ALLOC()

// #define OS_ENTER_CRITICAL()  \
// 		CPU_CRITICAL_ENTER(); \

// #define OS_EXIT_CRITICAL()  \
// 		CPU_CRITICAL_EXIT()

extern void cpu_assert_debug();
extern void chip_reset();
extern const u8 config_asser;

#define ASSERT(a,...)   \
		do { \
			if(config_asser){\
				if(!(a)){ \
					xprintf("file:%s, line:%d\n", __FILE__, __LINE__); \
					xprintf("ASSERT-FAILD: "__VA_ARGS__); \
					cpu_assert_debug(); \
				} \
			}else {\
				if(!(a)){ \
					chip_reset();\
				}\
			}\
		}while(0);

#endif //__ASSEMBLY__


#endif


