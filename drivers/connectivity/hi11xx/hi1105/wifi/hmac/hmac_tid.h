

#ifndef __HMAC_TID_H__
#define __HMAC_TID_H__

#include "mac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TID_H

typedef enum {
    OPERATE_TYPE_HEAD,
    OPERATE_TYPE_TAIL,

    OPERATE_TYPE_BUTT
} hmac_tid_operate_type_enum;

typedef struct {
    oal_dlist_head_stru tid_schedule_list;
    struct semaphore tid_schedule_sema;
    oal_spin_lock_stru lock;
    oal_task_stru *tid_schedule_thread;
    oal_bool_enum_uint8 enabled;
    oal_atomic  ring_mpdu_num;
} hmac_tid_schedule_stru;

extern hmac_tid_schedule_stru g_tid_schedule_mgmt;

OAL_STATIC OAL_INLINE hmac_tid_schedule_stru *hmac_get_tid_schedule_mgmt(void)
{
    return &g_tid_schedule_mgmt;
}


OAL_STATIC OAL_INLINE void hmac_tid_scheduled(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    up(&g_tid_schedule_mgmt.tid_schedule_sema);
#endif
}


OAL_STATIC OAL_INLINE void hmac_tid_enqueue(
    oal_netbuf_head_stru *tid_queue, oal_netbuf_stru *netbuf, hmac_tid_operate_type_enum type)
{
    if (type == OPERATE_TYPE_HEAD) {
        oal_netbuf_addlist(tid_queue, netbuf);
    } else {
        oal_netbuf_list_tail(tid_queue, netbuf);
    }
}


OAL_STATIC OAL_INLINE oal_netbuf_stru *hmac_tid_dequeue(
    oal_netbuf_head_stru *tid_queue, hmac_tid_operate_type_enum type)
{
    if (oal_netbuf_list_empty(tid_queue)) {
        return NULL;
    }

    return type == OPERATE_TYPE_HEAD ? oal_netbuf_delist(tid_queue) : oal_netbuf_delist_tail(tid_queue);
}

int32_t hmac_tid_schedule_thread(void *p_data);
void hmac_tid_schedule_init(void);
uint8_t hmac_is_tid_empty(void);
uint8_t hmac_ba_ring_is_empty(void);
#endif
