

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
extern oal_uint32 frw_timer_cpu_count_etc[];
#endif

/* 函数声明 */
extern oal_uint32 frw_timer_timeout_proc_etc(frw_event_mem_stru *pst_timeout_event);
extern oal_void frw_timer_init_etc(oal_uint32 ul_delay, oal_timer_func p_func, oal_ulong arg);
extern oal_void frw_timer_exit_etc(oal_void);
extern oal_void frw_timer_timeout_proc_event_etc(oal_ulong arg);
extern oal_void frw_timer_restart_etc(oal_void);
extern oal_void frw_timer_stop_etc(oal_void);

/* return true if the time a is after time b,in case of overflow and wrap around to zero */
OAL_STATIC OAL_INLINE oal_int32 frw_time_after(oal_uint32 a, oal_uint32 b)
{
    return ((oal_int32)((b) - (a)) <= 0);
}

#endif /* end of frw_timer.h */
