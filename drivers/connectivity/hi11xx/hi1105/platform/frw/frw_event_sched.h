

#ifndef __FRW_EVENT_SCHED_H__
#define __FRW_EVENT_SCHED_H__

/* 其他头文件包含 */
#include "oal_ext_if.h"
#include "frw_main.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FRW_EVENT_SCHED_H

/* 宏定义 */
/* 获取事件队列中已经缓存的事件个数 */
#define frw_event_queue_get_pending_events_num(_pst_event_queue) oal_queue_get_length(&(_pst_event_queue)->st_queue)

/*
 * 枚举说明: 事件队列对应的调度策略
 */
typedef enum {
    FRW_SCHED_POLICY_HI = 0,     /* 高优先级队列 */
    FRW_SCHED_POLICY_NORMAL = 1, /* 普通优先级队列 */

    FRW_SCHED_POLICY_BUTT
} frw_sched_policy_enum;

/*
 * 枚举说明: 事件队列状态
 */
typedef enum {
    FRW_EVENT_QUEUE_STATE_INACTIVE = 0, /* 事件队列不活跃(不可被调度) */
    FRW_EVENT_QUEUE_STATE_ACTIVE,       /* 事件队列活跃(可被调度) */

    FRW_EVENT_QUEUE_STATE_BUTT
} frw_event_queue_state_enum;

/*
 * 枚举说明: 事件部署类型
 */
typedef enum {
    FRW_VAP_STATE_RESUME = 0, /* VAP恢复 */
    FRW_VAP_STATE_PAUSE,      /* VAP暂停 */

    FRW_VAP_STATE_BUTT
} frw_vap_state_enum;

/*
 * 结构说明: 可调度队列
 */
typedef struct {
    oal_spin_lock_stru st_lock;
    uint32_t ul_total_weight_cnt; /* 可调度队列上的总权重计数器 */
    oal_dlist_head_stru st_head;    /* 可调度队列的事件链表头 */
} frw_event_sched_queue_stru;

/*
 * 结构说明: 事件队列结构体
 */
typedef struct tag_frw_event_queue_stru {
    oal_spin_lock_stru st_lock;
    oal_queue_stru st_queue;                   /* 队列 */
    uint8_t en_state; /* 队列状态 */
    uint8_t en_policy;     /* 队列调度策略(高优先级、普通优先级) */

    uint8_t uc_weight;     /* WRR权重配置值 */
    uint8_t uc_weight_cnt; /* WRR权重计数器 */

    uint8_t en_vap_state; /* VAP的状态值，0为恢复，1为暂停 */
    uint8_t auc_resv[3]; /* 定义oal_uint8数组，长度为3 */

    oal_dlist_head_stru st_list;
} frw_event_queue_stru;

/* 函数声明 */
uint32_t frw_event_sched_init(frw_event_sched_queue_stru *pst_sched_queue);
uint32_t frw_event_queue_init(frw_event_queue_stru *pst_event_queue, uint8_t uc_weight, uint8_t en_policy,
                              uint8_t en_state, uint8_t uc_max_events);
void frw_event_queue_destroy(frw_event_queue_stru *pst_event_queue);
void *frw_event_sched_pick_next_event_queue_wrr(frw_event_sched_queue_stru *pst_sched_queue);
uint32_t frw_event_sched_deactivate_queue(frw_event_sched_queue_stru *pst_sched_queue,
                                          frw_event_queue_stru *pst_event_queue);

uint32_t frw_event_sched_activate_queue(frw_event_sched_queue_stru *pst_sched_queue,
                                        frw_event_queue_stru *pst_event_queue);

void frw_event_sched_pause_queue(frw_event_sched_queue_stru *pst_sched_queue, frw_event_queue_stru *pst_event_queue);
void frw_event_sched_resume_queue(frw_event_sched_queue_stru *pst_sched_queue, frw_event_queue_stru *pst_event_queue);

/*
 * 函 数 名  : frw_event_sched_reset_weight
 * 功能描述  : 重置调度队列上各个事件队列的权重计数器
 * 输入参数  : pst_sched_queue: 调度队列指针
 */
OAL_STATIC OAL_INLINE void frw_event_sched_reset_weight(frw_event_sched_queue_stru *pst_sched_queue)
{
    oal_dlist_head_stru *pst_list = NULL;
    frw_event_queue_stru *pst_event_queue = NULL;

    if (oal_unlikely(pst_sched_queue == NULL)) {
        oal_warn_on(1);
        return;
    }

    /* 遍历整个调度链表 */
    oal_dlist_search_for_each(pst_list, &pst_sched_queue->st_head)
    {
        /* 获取调度链表中的一个事件队列 */
        pst_event_queue = oal_dlist_get_entry(pst_list, frw_event_queue_stru, st_list);
        /* 只是重置恢复状态VAP的权重值 */
        if (pst_event_queue->en_vap_state == FRW_VAP_STATE_RESUME) {
            /* 重置事件队列的权重计数器 */
            pst_event_queue->uc_weight_cnt = pst_event_queue->uc_weight;

            /* 更新调度队列上的总权重计数器 */
            pst_sched_queue->ul_total_weight_cnt += pst_event_queue->uc_weight;
        }
    }
}

/*
 * 函 数 名  : frw_event_schedule
 * 功能描述  : 事件调度入口函数
 * 输入参数  : pst_sched_queue: 调度队列指针
 */
OAL_STATIC OAL_INLINE void *frw_event_schedule(frw_event_sched_queue_stru *pst_sched_queue)
{
    void *p_event = NULL;
    frw_event_sched_queue_stru *pst_queue = NULL;

    if (oal_unlikely(pst_sched_queue == NULL)) {
        oal_warn_on(1);
        return NULL;
    }

    /* 遍历全部调度类 */
    pst_queue = &pst_sched_queue[FRW_SCHED_POLICY_HI];

    /* 如果高优先级调度队列为空，则取下一个调度队列 */
    if (!oal_dlist_is_empty(&pst_queue->st_head)) {
        /* 从调度类中挑选下一个待处理的事件 */
        p_event = frw_event_sched_pick_next_event_queue_wrr(pst_queue);
        if (p_event != NULL) {
            return p_event;
        }
    }

    pst_queue = &pst_sched_queue[FRW_SCHED_POLICY_NORMAL];

    /* 如果普通优先级调度队列为空，则取下一个调度队列 */
    if (!oal_dlist_is_empty(&pst_queue->st_head)) {
        /* 从调度类中挑选下一个待处理的事件 */
        p_event = frw_event_sched_pick_next_event_queue_wrr(pst_queue);
        if (p_event != NULL) {
            return p_event;
        }
    }

    return NULL;
}

#endif /* end of frw_event_sched.h */
