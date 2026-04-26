

#include "hmac_tid.h"
#include "hmac_tx_msdu_dscr.h"
#include "hmac_host_tx_data.h"
#if(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif
#include "host_hal_device.h"
#include "host_hal_ring.h"
#include "oal_net.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TID_C

/* tid调度管理结构体, 主要用于保存待调度的user tid链表, 在调度线程中处理 */
hmac_tid_schedule_stru g_tid_schedule_mgmt;


void hmac_tid_schedule_init(void)
{
    oal_dlist_init_head(&g_tid_schedule_mgmt.tid_schedule_list);
    oal_spin_lock_init(&g_tid_schedule_mgmt.lock);
    g_tid_schedule_mgmt.enabled = OAL_TRUE;
    oal_atomic_set(&g_tid_schedule_mgmt.ring_mpdu_num, 0);
}


OAL_STATIC hmac_tid_info_stru *hmac_tid_schedule_dequeue(void)
{
    oal_dlist_head_stru *entry = NULL;
    hmac_tid_info_stru *tid_info = NULL;

    oal_spin_lock_bh(&g_tid_schedule_mgmt.lock);

    if (oal_dlist_is_empty(&g_tid_schedule_mgmt.tid_schedule_list)) {
        oal_spin_unlock_bh(&g_tid_schedule_mgmt.lock);
        return NULL;
    }

    entry = oal_dlist_delete_head(&g_tid_schedule_mgmt.tid_schedule_list);
    tid_info = oal_dlist_get_entry(entry, hmac_tid_info_stru, tid_entry);

    oal_spin_unlock_bh(&g_tid_schedule_mgmt.lock);

    return tid_info;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_tid_push_ring_allowed(hmac_tid_info_stru *tid_info)
{
    return oal_atomic_read(&tid_info->tx_ring.enabled) && !hmac_tid_ring_full(&tid_info->tx_ring);
}

OAL_STATIC OAL_INLINE void hmac_tid_get_netbuf_list(oal_netbuf_head_stru *tid_queue,
    oal_netbuf_head_stru *tid_sch_head_t)
{
    oal_netbuf_head_init(tid_sch_head_t);

    /* 调用内核队列锁，关闭软中断 */
    oal_spin_lock_head_bh(tid_queue);
    oal_netbuf_queue_splice_tail_init(tid_queue, tid_sch_head_t);
    oal_spin_unlock_head_bh(tid_queue);
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (!defined _PRE_WINDOWS_SUPPORT)
static void hmac_tx_set_cb_csum_info(mac_tx_ctl_stru *tx_ctl, oal_netbuf_stru *netbuf)
{
    mac_ether_header_stru *ether_header = (mac_ether_header_stru *)oal_netbuf_data(netbuf);
    if (ether_header->us_ether_type == oal_host2net_short(ETHER_TYPE_IP)) {
        struct iphdr *iph = (struct iphdr *)(netbuf->head + netbuf->network_header);
        if (iph->protocol == OAL_IPPROTO_TCP) {
            tx_ctl->csum_type = CSUM_TYPE_IPV4_TCP;
        } else if (iph->protocol == OAL_IPPROTO_UDP) {
            tx_ctl->csum_type = CSUM_TYPE_IPV4_UDP;
        }
    } else if (ether_header->us_ether_type == oal_host2net_short(ETHER_TYPE_IPV6)) {
        struct ipv6hdr *ipv6h = (struct ipv6hdr *)(netbuf->head + netbuf->network_header);
        if (ipv6h->nexthdr == OAL_IPPROTO_TCP) {
            tx_ctl->csum_type = CSUM_TYPE_IPV6_TCP;
        } else if (ipv6h->nexthdr == OAL_IPPROTO_UDP) {
            tx_ctl->csum_type = CSUM_TYPE_IPV6_UDP;
        }
    }
}
#else
static void hmac_tx_set_cb_csum_info(mac_tx_ctl_stru *tx_ctl, oal_netbuf_stru *netbuf)
{
}
#endif


OAL_STATIC uint32_t hmac_tid_push_netbuf_to_device_ring(
    hmac_tid_info_stru *tid_info, mac_vap_stru *mac_vap, hmac_user_stru *hmac_user)
{
    uint32_t ret;
    oal_netbuf_head_stru tid_sch_head_t;
    oal_netbuf_stru *netbuf = NULL;
    frw_event_stru *frw_event = NULL; /* 事件结构体 */
    frw_event_mem_stru *frw_event_mem = NULL;
    dmac_tx_event_stru *dtx_event = NULL;
    mac_tx_ctl_stru *tx_ctl = NULL;

    hmac_tid_get_netbuf_list(&tid_info->tid_queue, &tid_sch_head_t);
    if (oal_unlikely(oal_netbuf_list_empty(&tid_sch_head_t))) {
        return OAL_SUCC;
    }

    /* 优化成netbuf链, 直接在一个事件中赋值dtx_event->pst_netbuf, 不需要循环抛多个事件 */
    while ((netbuf = oal_netbuf_delist_nolock(&tid_sch_head_t)) != NULL) {
        frw_event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
        if (oal_unlikely(frw_event_mem == NULL)) {
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_TX, "{hmac_tx_lan_to_wlan::frw_event_alloc_m failed.}");
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        frw_event = frw_get_event_stru(frw_event_mem);
        frw_event_hdr_init(&(frw_event->st_event_hdr), FRW_EVENT_TYPE_HOST_DRX,
                           DMAC_TX_HOST_DTX, sizeof(dmac_tx_event_stru),
                           FRW_EVENT_PIPELINE_STAGE_1, mac_vap->uc_chip_id,
                           mac_vap->uc_device_id, mac_vap->uc_vap_id);

        dtx_event = (dmac_tx_event_stru *)frw_event->auc_event_data;
        dtx_event->pst_netbuf = netbuf;

        tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);

        /* 设置8023帧长, 用于设置device ring msdu info的msdu_len字段 */
        MAC_GET_CB_MPDU_LEN(tx_ctl) = oal_netbuf_len(netbuf);
#ifndef _PRE_WINDOWS_SUPPORT
        if (netbuf->ip_summed == CHECKSUM_PARTIAL) {
            hmac_tx_set_cb_csum_info(tx_ctl, netbuf);
        }
#endif
        ret = frw_event_dispatch_event(frw_event_mem);
        if (oal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_TX,
                             "{hmac_tid_trans_netbuf_to_device::frw_event_dispatch_event failed[%d]}", ret);
            oam_stat_vap_incr(mac_vap->uc_vap_id, tx_abnormal_msdu_dropped, 1);
        }

        frw_event_free_m(frw_event_mem);
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_tid_push_netbuf_to_host_ring(
    hmac_tid_info_stru *tid_info, mac_vap_stru *mac_vap, hmac_user_stru *hmac_user)
{
    oal_netbuf_stru *netbuf = NULL;
    mac_user_stru *mac_user = NULL;
    oal_netbuf_head_stru tid_sch_head_t;

    host_cnt_init_record_performance(TX_RING_PROC);
    host_start_record_performance(TX_RING_PROC);

    if (oal_unlikely(!hmac_tid_push_ring_allowed(tid_info))) {
        return OAL_FAIL;
    }

    mac_user = mac_res_get_mac_user(tid_info->user_index);
    if (mac_user == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id,
            OAM_SF_TX, "{hmac_tid_push_netbuf_to_host_ring::mac_user is null.}");
        return OAL_FAIL;
    }

    hmac_tid_get_netbuf_list(&tid_info->tid_queue, &tid_sch_head_t);
    if (oal_unlikely(oal_netbuf_list_empty(&tid_sch_head_t))) {
        hmac_tx_sync_ring_info(mac_user, tid_info, TID_CMDID_ENQUE);
        return OAL_SUCC;
    }

    while ((netbuf = oal_netbuf_delist_nolock(&tid_sch_head_t)) != NULL) {
        if (oal_unlikely(hmac_host_ring_tx(mac_vap, hmac_user, netbuf)) != OAL_SUCC) {

            hmac_tid_enqueue(&tid_info->tid_queue, netbuf, OPERATE_TYPE_HEAD);
            host_cnt_inc_record_performance(TX_RING_PROC);
            break;
        }
    }

    if (oal_likely(hmac_get_tx_update_mode_switch() == OAL_FALSE)) {
        hmac_tx_reg_write_ring_info(hmac_user, tid_info, TID_CMDID_ENQUE);
    }
    hmac_tx_sync_ring_info(mac_user, tid_info, TID_CMDID_ENQUE);

    host_end_record_performance(host_cnt_get_record_performance(TX_RING_PROC), TX_RING_PROC);
    return OAL_SUCC;
}

typedef uint32_t (* hmac_tid_push_ring)(hmac_tid_info_stru *, mac_vap_stru *, hmac_user_stru *);
OAL_STATIC hmac_tid_push_ring hmac_get_tid_push_ring_func(hmac_tid_info_stru *tid_info)
{
    hmac_tid_push_ring func = NULL;
    uint8_t ring_tx_mode = (uint8_t)oal_atomic_read(&tid_info->ring_tx_mode);

    if (oal_likely(ring_tx_mode == HOST_RING_TX_MODE)) {
        func = hmac_tid_push_netbuf_to_host_ring;
    } else if (ring_tx_mode == DEVICE_RING_TX_MODE) {
        func = hmac_tid_push_netbuf_to_device_ring;
    } else {
        oam_warning_log3(0, OAM_SF_TX, "{hmac_get_tid_push_ring_func::user[%d] tid[%d] push ring[%d] not allowed!}",
            tid_info->user_index, tid_info->tid_no, ring_tx_mode);
    }

    return func;
}


int32_t hmac_tid_schedule_thread(void *p_data)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hmac_tid_info_stru *tid_info = NULL;
    hmac_user_stru *hmac_user = NULL;
    mac_vap_stru *mac_vap = NULL;
    hmac_tid_push_ring push_ring_func;

    struct sched_param param;

    param.sched_priority = 97; /* 调度优先级97 */
    oal_set_thread_property(current, SCHED_FIFO, &param, -10); /* set nice -10 */

    allow_signal(SIGTERM);

    while (oal_likely(!down_interruptible(&g_tid_schedule_mgmt.tid_schedule_sema))) {
#ifdef _PRE_WINDOWS_SUPPORT
        if (oal_kthread_should_stop((PRT_THREAD)p_data)) {
#else
        if (oal_kthread_should_stop()) {
#endif
            break;
        }

        /* 调用平台接口唤醒device */
        wlan_pm_wakeup_dev_lock();

        tid_info = hmac_tid_schedule_dequeue();
        if (oal_unlikely(tid_info == NULL)) {
            continue;
        }

        hmac_user = mac_res_get_hmac_user(tid_info->user_index);
        if (oal_unlikely(hmac_user == NULL ||
            hmac_user->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_ASSOC)) {
            continue;
        }

        mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(hmac_user->st_user_base_info.uc_vap_id);
        if (oal_unlikely(mac_vap == NULL || mac_vap->en_vap_state != MAC_VAP_STATE_UP)) {
            continue;
        }

        push_ring_func = hmac_get_tid_push_ring_func(tid_info);
        if (push_ring_func == NULL) {
            continue;
        }
        g_pm_wlan_pkt_statis.ring_tx_pkt += (uint32_t)oal_atomic_read(&g_tid_schedule_mgmt.ring_mpdu_num);

        push_ring_func(tid_info, mac_vap, hmac_user);
    }
#endif

    return OAL_SUCC;
}

uint8_t hmac_is_tid_empty(void)
{
    oal_dlist_head_stru *entry = NULL;
    hmac_tid_info_stru *tid_info = NULL;
    oal_netbuf_head_stru *tid_queue = NULL;
    uint8_t ret = OAL_TRUE;;

    oal_spin_lock_bh(&hmac_get_tid_schedule_mgmt()->lock);
    oal_dlist_search_for_each(entry, &hmac_get_tid_schedule_mgmt()->tid_schedule_list) {
        tid_info = oal_dlist_get_entry(entry, hmac_tid_info_stru, tid_entry);
        tid_queue = &tid_info->tid_queue;
        if (oal_netbuf_list_len(tid_queue) != 0) {
            ret = OAL_FALSE;
            break;
        }
    }
    oal_spin_unlock_bh(&hmac_get_tid_schedule_mgmt()->lock);
    return ret;
}

uint8_t hmac_ba_ring_is_empty(void)
{
    hal_host_device_stru *hal_dev = hal_get_host_device(0);
    hal_host_ring_ctl_stru *ba_info_ring = &hal_dev->st_host_tx_ba_info_ring;
    if (hal_ring_is_empty(ba_info_ring) != OAL_TRUE) {
        return OAL_FALSE;
    }
    return OAL_TRUE;
}
