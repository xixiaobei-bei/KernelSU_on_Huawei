

#ifndef __FRW_TASK_H__
#define __FRW_TASK_H__

/* 其他头文件包含 */
#include "oal_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FRW_TASK_H

/* 线程的运行状态 */
typedef enum {
    FRW_TASK_STATE_IRQ_UNBIND = 0, /* 线程处于暂停状态 */
    FRW_TASK_STATE_IRQ_BIND,       /* 线程处于运行状态 */

    FRW_TASK_BUTT
} frw_task_state_enum;

typedef oal_uint8 frw_task_state_enum_uint8;

/* STRUCT 定义 */
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
typedef struct {
    oal_task_stru *pst_event_kthread; /*  task变量 */
    oal_wait_queue_head_stru frw_wq;  /* waitqueue */
    oal_uint8 uc_task_state;          /* 线程状态，0暂停，1运行 */
    oal_uint8 auc_resv[3];
    oal_uint32 ul_total_loop_cnt;
    oal_uint32 ul_total_event_cnt; /* 统计线程空转次数 */
    oal_uint32 ul_max_empty_count;
    oal_void (*p_event_handler_func)(oal_void *_pst_bind_cpu); /* kthread主程序 */
} frw_task_stru;

#elif (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_TASKLET)
typedef struct {
    oal_tasklet_stru st_ipi_tasklet;
    oal_tasklet_stru st_event_tasklet;               /* tasklet变量 */
    oal_void (*p_event_handler_func)(oal_uint); /* tasklet处理程序 */
} frw_task_stru;
#endif

/* 全局变量声明 */
extern frw_task_stru event_task_etc[];

/* 函数声明 */
extern oal_uint32 frw_task_init_etc(oal_void);
extern oal_void frw_task_exit_etc(oal_void);
extern oal_void frw_task_event_handler_register_etc (oal_void (*p_func)(oal_uint));
extern oal_void frw_task_set_state_etc(oal_uint32 ul_core_id, oal_uint8 uc_task_state);
extern oal_uint8 frw_task_get_state_etc(oal_uint32 ul_core_id);
extern oal_void frw_task_sched_etc(oal_uint32 ul_core_id);

#endif /* end of frw_task.h */
