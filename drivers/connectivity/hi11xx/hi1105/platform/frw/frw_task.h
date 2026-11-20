

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

/* STRUCT 定义 */
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
typedef struct {
    oal_task_stru *pst_event_kthread; /*  task变量 */
    oal_wait_queue_head_stru frw_wq;  /* waitqueue */
    uint8_t uc_task_state;          /* 线程状态，0暂停，1运行 */
    uint8_t auc_resv[3]; /* 3 */
    uint32_t ul_total_loop_cnt;
    uint32_t ul_total_event_cnt; /* 统计线程空转次数 */
    uint32_t ul_max_empty_count;
    void (*p_event_handler_func)(void *_pst_bind_cpu); /* kthread主程序 */
} frw_task_stru;

#elif (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_TASKLET)
typedef struct {
    oal_tasklet_stru st_ipi_tasklet;
    oal_tasklet_stru st_event_tasklet;               /* tasklet变量 */
    void (*p_event_handler_func)(oal_uint); /* tasklet处理程序 */
} frw_task_stru;
#endif

/* 全局变量声明 */
extern frw_task_stru g_st_event_task[];

/* 函数声明 */
uint32_t frw_task_init(void);
void frw_task_exit(void);
void frw_task_event_handler_register (void (*p_func)(oal_uint));
void frw_task_set_state(uint32_t ul_core_id, uint8_t uc_task_state);
uint8_t frw_task_get_state(uint32_t ul_core_id);
void frw_task_sched(uint32_t ul_core_id);

#endif /* end of frw_task.h */
