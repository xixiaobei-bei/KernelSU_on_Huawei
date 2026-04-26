

#ifndef __FRW_TIMER_H__
#define __FRW_TIMER_H__

/* 其他头文件包含 */
#include "frw_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FRW_TIMER_H

/* 宏定义 */
#define FRW_TIME_UL_MAX       0xFFFFFFFF
#define FRW_TIMER_MAX_TIMEOUT (FRW_TIME_UL_MAX >> 1) /* 32位最大值的1/2 */

/* 全局变量声明 */
#if defined(_PRE_FRW_TIMER_BIND_CPU) && defined(CONFIG_NR_CPUS)
extern uint32_t g_frw_timer_cpu_count[];
#endif

/* 函数声明 */
uint32_t frw_timer_timeout_proc(frw_event_mem_stru *pst_timeout_event);
void frw_timer_init(uint32_t delay, oal_timer_func p_func, uintptr_t arg);
void frw_timer_exit(void);
void frw_timer_timeout_proc_event(uintptr_t arg);
void frw_timer_restart(void);
void frw_timer_stop(void);

/* return true if the time a is after time b,in case of overflow and wrap around to zero */
OAL_STATIC OAL_INLINE int32_t frw_time_after(uint32_t a, uint32_t b)
{
    return ((int32_t)((b) - (a)) <= 0);
}

#endif /* end of frw_timer.h */
