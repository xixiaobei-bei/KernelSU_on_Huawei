

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 头文件包含 */
#include "wlan_spec.h"
#include "mac_vap.h"
#include "hmac_blockack.h"
#include "hmac_main.h"
#include "hmac_rx_data.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_user.h"
#include "hmac_auto_adjust_freq.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_BLOCKACK_C

/* 2 全局变量定义 */
/* 3 函数实现 */

oal_void hmac_reorder_ba_timer_start_etc(hmac_vap_stru *pst_hmac_vap,
                                         hmac_user_stru *pst_hmac_user,
                                         oal_uint8 uc_tid)
{
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    hmac_ba_rx_stru *pst_ba_rx_stru = OAL_PTR_NULL;
    mac_device_stru *pst_device = OAL_PTR_NULL;
    oal_uint16 us_timeout;

    /* 如果超时定时器已经被注册则返回 */
    if (pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer.en_is_registerd == OAL_TRUE) {
        return;
    }

    pst_mac_vap = &pst_hmac_vap->st_vap_base_info;

    pst_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (pst_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                       "{hmac_reorder_ba_timer_start_etc::pst_device[%d] null.}",
                       pst_mac_vap->uc_device_id);
        return;
    }

    /* 业务量较小时,使用小周期的重排序定时器,保证及时上报至协议栈;
       业务量较大时,使用大周期的重排序定时器,保证尽量不丢包 */
    if (OAL_FALSE == hmac_wifi_rx_is_busy()) {
        us_timeout = pst_hmac_vap->us_rx_timeout_min[WLAN_WME_TID_TO_AC(uc_tid)];
    } else {
        us_timeout = pst_hmac_vap->us_rx_timeout[WLAN_WME_TID_TO_AC(uc_tid)];
    }

    pst_ba_rx_stru = pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info;

    oal_spin_lock(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer_lock));

    FRW_TIMER_CREATE_TIMER(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer),
                           hmac_ba_timeout_fn_etc,
                           us_timeout,
                           &(pst_ba_rx_stru->st_alarm_data),
                           OAL_FALSE,
                           OAM_MODULE_ID_HMAC,
                           pst_device->ul_core_id);

    oal_spin_unlock(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer_lock));
}


oal_void hmac_ba_rx_hdl_baw_init_etc(hmac_user_stru *pst_hmac_user,
    hmac_ba_rx_stru *pst_ba_rx_stru, oal_uint8 uc_tid)
{
    /* 保存ba size实际大小,用于反馈addba rsp */
    pst_hmac_user->aus_tid_baw_size[uc_tid] = pst_ba_rx_stru->us_baw_size;

    if (pst_ba_rx_stru->us_baw_size <= 1) {
        pst_ba_rx_stru->us_baw_size = WLAN_AMPDU_RX_BUFFER_SIZE;
        /* 异常更新反馈值 */
        pst_hmac_user->aus_tid_baw_size[uc_tid] = WLAN_AMPDU_RX_BUFFER_SIZE;
    }

#if defined(_PRE_WLAN_FEATURE_11AX)
    /* HE设备并且BA窗大于64时, */
    if ((pst_ba_rx_stru->us_baw_size > WLAN_AMPDU_RX_BUFFER_SIZE) &&
         MAC_USER_IS_HE_USER(&pst_hmac_user->st_user_base_info)) {
        /* resp反馈值与addba req保持一致,但是BA BAW需要更新,避免兼容性问题,因此不更新user */
        pst_ba_rx_stru->us_baw_size = WLAN_AMPDU_RX_HE_BUFFER_SIZE;

        /* 定制化设定BA BAW, 则取小 */
        if (g_st_rx_buffer_size_stru.en_rx_ampdu_bitmap_ini == OAL_TRUE) {
            pst_ba_rx_stru->us_baw_size =
                    OAL_MIN(g_st_rx_buffer_size_stru.us_rx_buffer_size, WLAN_AMPDU_RX_HE_BUFFER_SIZE);
        }

        /* 兼容性问题:如果决策的BAW size 小于对端BAW size,那么更新resp帧BAW size,否则按照对端的BAW size反馈 */
        pst_hmac_user->aus_tid_baw_size[uc_tid] =
                    OAL_MIN(pst_hmac_user->aus_tid_baw_size[uc_tid], pst_ba_rx_stru->us_baw_size);
    } else
#endif
    {
        /* BA窗小于64时协商BA窗SIZE */
        pst_ba_rx_stru->us_baw_size = OAL_MIN(pst_ba_rx_stru->us_baw_size, WLAN_AMPDU_RX_BUFFER_SIZE);
        /* 非HE设备更新,反馈值与baw同步更新 */
        pst_hmac_user->aus_tid_baw_size[uc_tid] = pst_ba_rx_stru->us_baw_size;
    }

    /* 配置优先级最高，手动配置接收聚合个数 */
    if (g_st_rx_buffer_size_stru.en_rx_ampdu_bitmap_cmd == OAL_TRUE) {
        pst_ba_rx_stru->us_baw_size = g_st_rx_buffer_size_stru.us_rx_buffer_size;
        pst_hmac_user->aus_tid_baw_size[uc_tid] = pst_ba_rx_stru->us_baw_size;
    }
}


oal_void hmac_ba_rx_hdl_init_etc(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user, oal_uint8 uc_tid)
{
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    hmac_ba_rx_stru *pst_ba_rx_stru = OAL_PTR_NULL;
    hmac_rx_buf_stru *pst_rx_buff = OAL_PTR_NULL;
    oal_uint16 us_reorder_index;

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);

    pst_ba_rx_stru = pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info;

    /* 初始化reorder队列 */
    for (us_reorder_index = 0; us_reorder_index < WLAN_AMPDU_RX_HE_BUFFER_SIZE; us_reorder_index++) {
        pst_rx_buff = HMAC_GET_BA_RX_DHL(pst_ba_rx_stru, us_reorder_index);
        pst_rx_buff->in_use = 0;
        pst_rx_buff->us_seq_num = 0;
        pst_rx_buff->uc_num_buf = 0;
        oal_netbuf_list_head_init(&(pst_rx_buff->st_netbuf_head));
    }

    /* 初始化BAW */
    hmac_ba_rx_hdl_baw_init_etc(pst_hmac_user, pst_ba_rx_stru, uc_tid);

    /* Ba会话参数初始化 */
    pst_ba_rx_stru->us_baw_end = DMAC_BA_SEQ_ADD(pst_ba_rx_stru->us_baw_start, (pst_ba_rx_stru->us_baw_size - 1));
    pst_ba_rx_stru->us_baw_tail = DMAC_BA_SEQNO_SUB(pst_ba_rx_stru->us_baw_start, 1);
    pst_ba_rx_stru->us_baw_head = DMAC_BA_SEQNO_SUB(pst_ba_rx_stru->us_baw_start, HMAC_BA_BMP_SIZE);
    pst_ba_rx_stru->uc_mpdu_cnt = 0;
    pst_ba_rx_stru->en_is_ba = OAL_TRUE;  // Ba session is processing
    pst_ba_rx_stru->en_amsdu_supp = ((pst_hmac_vap->bit_rx_ampduplusamsdu_active & pst_hmac_vap->en_ps_rx_amsdu) ?
                                      OAL_TRUE : OAL_FALSE);
    pst_ba_rx_stru->en_back_var = MAC_BACK_COMPRESSED;
    pst_ba_rx_stru->puc_transmit_addr = pst_hmac_user->st_user_base_info.auc_user_mac_addr;

    /* 初始化定时器资源 */
    pst_ba_rx_stru->st_alarm_data.pst_ba = pst_ba_rx_stru;
    pst_ba_rx_stru->st_alarm_data.us_mac_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;
    pst_ba_rx_stru->st_alarm_data.uc_vap_id = pst_mac_vap->uc_vap_id;
    pst_ba_rx_stru->st_alarm_data.uc_tid = uc_tid;
    pst_ba_rx_stru->st_alarm_data.us_timeout_times = 0;
    pst_ba_rx_stru->st_alarm_data.en_direction = MAC_RECIPIENT_DELBA;
    pst_ba_rx_stru->en_timer_triggered = OAL_FALSE;

    oal_spin_lock_init(&pst_ba_rx_stru->st_ba_lock);
}


OAL_STATIC hmac_rx_buf_stru *hmac_ba_buffer_frame_in_reorder(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                             oal_uint16 us_seq_num,
                                                             mac_rx_ctl_stru *pst_cb_ctrl)
{
    oal_uint16 us_buf_index;
    hmac_rx_buf_stru *pst_rx_buf;

    us_buf_index = (us_seq_num & (WLAN_AMPDU_RX_HE_BUFFER_SIZE - 1));
    pst_rx_buf = HMAC_GET_BA_RX_DHL(pst_ba_rx_hdl, us_buf_index);

    if (pst_rx_buf->in_use == 1) {
        hmac_rx_free_netbuf_list_etc(&pst_rx_buf->st_netbuf_head, pst_rx_buf->uc_num_buf);
        pst_ba_rx_hdl->uc_mpdu_cnt--;
        pst_rx_buf->in_use = 0;
        pst_rx_buf->uc_num_buf = 0;
        OAM_INFO_LOG1(0, OAM_SF_BA, "{hmac_ba_buffer_frame_in_reorder::slot already used, seq[%d].}", us_seq_num);
    }

    if (pst_cb_ctrl->bit_amsdu_enable == OAL_TRUE) {
        if (pst_cb_ctrl->bit_is_first_buffer == OAL_TRUE) {
            if (oal_netbuf_list_len(&pst_rx_buf->st_netbuf_head) != 0) {
                hmac_rx_free_netbuf_list_etc(&pst_rx_buf->st_netbuf_head,
                                             oal_netbuf_list_len(&pst_rx_buf->st_netbuf_head));
                OAM_INFO_LOG1(0, OAM_SF_BA,
                    "{hmac_ba_buffer_frame_in_reorder::seq[%d] amsdu first buffer, need clear st_netbuf_head first}",
                    us_seq_num);
            }
            pst_rx_buf->uc_num_buf = 0;
        }

        /* offload下,amsdu帧拆成单帧分别上报 */
        pst_rx_buf->uc_num_buf += pst_cb_ctrl->bit_buff_nums;

        /* 遇到最后一个amsdu buffer 才标记in use 为 1 */
        if (pst_cb_ctrl->bit_is_last_buffer == OAL_TRUE) {
            pst_ba_rx_hdl->uc_mpdu_cnt++;
            pst_rx_buf->in_use = 1;
        } else {
            pst_rx_buf->in_use = 0;
        }
    } else {
        pst_rx_buf->uc_num_buf = pst_cb_ctrl->bit_buff_nums;
        pst_ba_rx_hdl->uc_mpdu_cnt++;
        pst_rx_buf->in_use = 1;
    }
    /* Update the buffered receive packet details */
    pst_rx_buf->us_seq_num = us_seq_num;
    pst_rx_buf->ul_rx_time = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    return pst_rx_buf;
}


OAL_STATIC oal_uint32 hmac_ba_send_frames_with_gap(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                   oal_netbuf_head_stru *pst_netbuf_header,
                                                   oal_uint16 us_last_seqnum,
                                                   mac_vap_stru *pst_vap)
{
    oal_uint8 uc_num_frms = 0;
    oal_uint16 us_seq_num;
    hmac_rx_buf_stru *pst_rx_buf = OAL_PTR_NULL;
    oal_uint8 uc_loop_index;
    oal_netbuf_stru *pst_netbuf = OAL_PTR_NULL;

    us_seq_num = pst_ba_rx_hdl->us_baw_start;

    OAM_INFO_LOG1(pst_vap->uc_vap_id, OAM_SF_BA, "{hmac_ba_send_frames_with_gap::to seq[%d].}", us_last_seqnum);

    while (us_seq_num != us_last_seqnum) {
        pst_rx_buf = hmac_remove_frame_from_reorder_q(pst_ba_rx_hdl, us_seq_num);
        if (pst_rx_buf != OAL_PTR_NULL) {
            pst_ba_rx_hdl->uc_mpdu_cnt--;
            pst_netbuf = oal_netbuf_peek(&pst_rx_buf->st_netbuf_head);
            if (OAL_UNLIKELY(pst_netbuf == OAL_PTR_NULL)) {
                OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_BA,
                                 "{hmac_ba_send_frames_with_gap::gap[%d].\r\n}",
                                 us_seq_num);

                us_seq_num = DMAC_BA_SEQNO_ADD(us_seq_num, 1);
                pst_rx_buf->uc_num_buf = 0;

                continue;
            }

            for (uc_loop_index = 0; uc_loop_index < pst_rx_buf->uc_num_buf; uc_loop_index++) {
                pst_netbuf = oal_netbuf_delist(&pst_rx_buf->st_netbuf_head);
                OAL_MEM_NETBUF_TRACE(pst_netbuf, OAL_FALSE);
                if (pst_netbuf != OAL_PTR_NULL) {
                    oal_netbuf_list_tail_nolock(pst_netbuf_header, pst_netbuf);
                }
            }
            pst_rx_buf->uc_num_buf = 0;
            uc_num_frms++;
        }

        us_seq_num = DMAC_BA_SEQNO_ADD(us_seq_num, 1);
    }

    OAM_WARNING_LOG4(pst_vap->uc_vap_id, OAM_SF_BA,
        "{hmac_ba_send_frames_with_gap::old_baw_start[%d], new_baw_start[%d], uc_num_frms[%d], uc_mpdu_cnt=%d.}",
        pst_ba_rx_hdl->us_baw_start, us_last_seqnum, uc_num_frms, pst_ba_rx_hdl->uc_mpdu_cnt);

    return uc_num_frms;
}


OAL_STATIC oal_uint16 hmac_ba_send_frames_in_order(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                   oal_netbuf_head_stru *pst_netbuf_header,
                                                   mac_vap_stru *pst_vap)
{
    oal_uint16 us_seq_num;
    hmac_rx_buf_stru *pst_rx_buf = OAL_PTR_NULL;
    oal_uint8 uc_loop_index;
    oal_netbuf_stru *pst_netbuf = OAL_PTR_NULL;

    us_seq_num = pst_ba_rx_hdl->us_baw_start;

    while ((pst_rx_buf = hmac_remove_frame_from_reorder_q(pst_ba_rx_hdl, us_seq_num)) != OAL_PTR_NULL) {
        pst_ba_rx_hdl->uc_mpdu_cnt--;
        us_seq_num = HMAC_BA_SEQNO_ADD(us_seq_num, 1);
        pst_netbuf = oal_netbuf_peek(&pst_rx_buf->st_netbuf_head);
        if (pst_netbuf == OAL_PTR_NULL) {
            OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_BA,
                             "{hmac_ba_send_frames_in_order::[%d] slot error.}",
                             us_seq_num);
            pst_rx_buf->uc_num_buf = 0;
            continue;
        }

        for (uc_loop_index = 0; uc_loop_index < pst_rx_buf->uc_num_buf; uc_loop_index++) {
            pst_netbuf = oal_netbuf_delist(&pst_rx_buf->st_netbuf_head);
            OAL_MEM_NETBUF_TRACE(pst_netbuf, OAL_FALSE);
            if (pst_netbuf != OAL_PTR_NULL) {
                oal_netbuf_list_tail_nolock(pst_netbuf_header, pst_netbuf);
            }
        }

        pst_rx_buf->uc_num_buf = 0;
    }

    return us_seq_num;
}


OAL_STATIC OAL_INLINE oal_void hmac_ba_buffer_rx_frame(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                       mac_rx_ctl_stru *pst_cb_ctrl,
                                                       oal_netbuf_head_stru *pst_netbuf_header,
                                                       oal_uint16 us_seq_num)
{
    hmac_rx_buf_stru *pst_rx_netbuf = OAL_PTR_NULL;
    oal_netbuf_stru *pst_netbuf = OAL_PTR_NULL;
    oal_uint8 uc_netbuf_index;
#ifdef _PRE_DEBUG_MODE
    oal_uint32 ul_netbuf_num;
#endif

    /* Get the pointer to the buffered packet */
    pst_rx_netbuf = hmac_ba_buffer_frame_in_reorder(pst_ba_rx_hdl, us_seq_num, pst_cb_ctrl);

#ifdef _PRE_DEBUG_MODE
    ul_netbuf_num = oal_netbuf_get_buf_num(&pst_rx_netbuf->st_netbuf_head);
    if (ul_netbuf_num != 0) {
        OAM_INFO_LOG1(pst_cb_ctrl->uc_mac_vap_id, OAM_SF_BA,
                      "{hmac_ba_buffer_rx_frame::%d netbuf miss here.}",
                      ul_netbuf_num);
    }

    if (pst_rx_netbuf->uc_num_buf != 1) {
        OAM_INFO_LOG1(pst_cb_ctrl->uc_mac_vap_id, OAM_SF_BA, "{hmac_ba_buffer_rx_frame:find amsdu netbuff cnt %d.}",
                      pst_rx_netbuf->uc_num_buf);
    }
#endif

    /* all buffers of this frame must be deleted from the buf list */
    for (uc_netbuf_index = pst_cb_ctrl->bit_buff_nums; uc_netbuf_index > 0; uc_netbuf_index--) {
        pst_netbuf = oal_netbuf_delist_nolock(pst_netbuf_header);

        OAL_MEM_NETBUF_TRACE(pst_netbuf, OAL_TRUE);
        if (OAL_UNLIKELY(pst_netbuf != OAL_PTR_NULL)) {
            /* enqueue reorder queue */
            oal_netbuf_add_to_list_tail(pst_netbuf, &pst_rx_netbuf->st_netbuf_head);
        } else {
            OAM_ERROR_LOG0(pst_cb_ctrl->uc_mac_vap_id, OAM_SF_BA, "{hmac_ba_buffer_rx_frame:netbuff error in amsdu.}");
        }
    }

    if (oal_netbuf_list_len(&pst_rx_netbuf->st_netbuf_head) != pst_rx_netbuf->uc_num_buf) {
        OAM_WARNING_LOG2(pst_cb_ctrl->uc_mac_vap_id, OAM_SF_BA, "{hmac_ba_buffer_rx_frame: list_len=%d numbuf=%d}",
                         oal_netbuf_list_len(&pst_rx_netbuf->st_netbuf_head), pst_rx_netbuf->uc_num_buf);
        pst_rx_netbuf->uc_num_buf = oal_netbuf_list_len(&pst_rx_netbuf->st_netbuf_head);
    }
}


OAL_STATIC OAL_INLINE oal_void hmac_ba_reorder_rx_data(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                       oal_netbuf_head_stru *pst_netbuf_header,
                                                       mac_vap_stru *pst_vap,
                                                       oal_uint16 us_seq_num)
{
    oal_uint8 uc_seqnum_pos;
    oal_uint16 us_temp_winstart;

    uc_seqnum_pos = hmac_ba_seqno_bound_chk(pst_ba_rx_hdl->us_baw_start, pst_ba_rx_hdl->us_baw_end, us_seq_num);

    if (uc_seqnum_pos == DMAC_BA_BETWEEN_SEQLO_SEQHI) {
        pst_ba_rx_hdl->us_baw_start = hmac_ba_send_frames_in_order(pst_ba_rx_hdl, pst_netbuf_header, pst_vap);
        pst_ba_rx_hdl->us_baw_end = DMAC_BA_SEQNO_ADD(pst_ba_rx_hdl->us_baw_start, (pst_ba_rx_hdl->us_baw_size - 1));
    } else if (uc_seqnum_pos == DMAC_BA_GREATER_THAN_SEQHI) {
        us_temp_winstart = HMAC_BA_SEQNO_SUB(us_seq_num, (pst_ba_rx_hdl->us_baw_size - 1));

        hmac_ba_send_frames_with_gap(pst_ba_rx_hdl, pst_netbuf_header, us_temp_winstart, pst_vap);
        pst_ba_rx_hdl->us_baw_start = us_temp_winstart;
        pst_ba_rx_hdl->us_baw_start = hmac_ba_send_frames_in_order(pst_ba_rx_hdl, pst_netbuf_header, pst_vap);
        pst_ba_rx_hdl->us_baw_end = HMAC_BA_SEQNO_ADD(pst_ba_rx_hdl->us_baw_start, (pst_ba_rx_hdl->us_baw_size - 1));
    } else {
        OAM_WARNING_LOG3(pst_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_ba_reorder_rx_data::else branch seqno[%d] ws[%d] we[%d].}",
            us_seq_num, pst_ba_rx_hdl->us_baw_start, pst_ba_rx_hdl->us_baw_end);
    }
}


OAL_STATIC oal_void hmac_ba_flush_reorder_q(hmac_ba_rx_stru *pst_rx_ba)
{
    hmac_rx_buf_stru *pst_rx_buf = OAL_PTR_NULL;
    oal_uint16 us_index;

    for (us_index = 0; us_index < WLAN_AMPDU_RX_HE_BUFFER_SIZE; us_index++) {
        pst_rx_buf = HMAC_GET_BA_RX_DHL(pst_rx_ba, us_index);

        if (pst_rx_buf->in_use == OAL_TRUE) {
            hmac_rx_free_netbuf_list_etc(&pst_rx_buf->st_netbuf_head, pst_rx_buf->uc_num_buf);

            pst_rx_buf->in_use = OAL_FALSE;
            pst_rx_buf->uc_num_buf = 0;
            pst_rx_ba->uc_mpdu_cnt--;
        }
    }

    if (pst_rx_ba->uc_mpdu_cnt != 0) {
        OAM_WARNING_LOG1(0, OAM_SF_BA, "{hmac_ba_flush_reorder_q:: %d mpdu cnt left.}", pst_rx_ba->uc_mpdu_cnt);
    }
}

OAL_STATIC OAL_INLINE oal_uint32 hmac_ba_check_rx_aggr(mac_vap_stru *pst_vap,
                                                       mac_ieee80211_frame_stru *pst_frame_hdr)
{
    /* 该vap是否是ht */
    if (OAL_FALSE == mac_mib_get_HighThroughputOptionImplemented(pst_vap)) {
        OAM_INFO_LOG0(pst_vap->uc_vap_id, OAM_SF_BA, "{hmac_ba_check_rx_aggr::ht not supported by this vap.}");
        return OAL_FAIL;
    }

    /* 判断该帧是不是qos帧 */
    if ((WLAN_FC0_SUBTYPE_QOS | WLAN_FC0_TYPE_DATA) != ((oal_uint8 *)pst_frame_hdr)[0]) {
        OAM_INFO_LOG0(pst_vap->uc_vap_id, OAM_SF_BA, "{hmac_ba_check_rx_aggr::not qos data.}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_ba_need_update_hw_baw(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                                     oal_uint16 us_seq_num)
{
    if ((OAL_TRUE == hmac_ba_seqno_lt(us_seq_num, pst_ba_rx_hdl->us_baw_start))
        && (OAL_FALSE == hmac_ba_rx_seqno_lt(us_seq_num, pst_ba_rx_hdl->us_baw_start))) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


oal_uint32 hmac_ba_filter_serv_etc(hmac_user_stru *pst_hmac_user,
                                   mac_rx_ctl_stru *pst_cb_ctrl,
                                   oal_netbuf_head_stru *pst_netbuf_header,
                                   oal_bool_enum_uint8 *pen_is_ba_buf)
{
    hmac_ba_rx_stru *pst_ba_rx_hdl = OAL_PTR_NULL;
    oal_uint16 us_seq_num;
    oal_bool_enum_uint8 en_is_4addr;
    oal_uint8 uc_is_tods;
    oal_uint8 uc_is_from_ds;
    oal_uint8 uc_tid;
    oal_uint16 us_baw_start_temp;
    oal_uint32 ul_ret;
    mac_ieee80211_frame_stru *pst_frame_hdr = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;

    if (OAL_ANY_NULL_PTR3(pst_netbuf_header, pst_cb_ctrl, pen_is_ba_buf)) {
        OAM_ERROR_LOG0(0, OAM_SF_BA, "{hmac_ba_filter_serv_etc::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_frame_hdr = (mac_ieee80211_frame_stru *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_cb_ctrl);
    if (OAL_UNLIKELY(pst_frame_hdr == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_BA, "{hmac_ba_filter_serv_etc::pst_frame_hdr null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_cb_ctrl->uc_mac_vap_id);
    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_BA, "{hmac_ba_filter_serv_etc::pst_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &pst_hmac_vap->st_vap_base_info;

    ul_ret = hmac_ba_check_rx_aggr(pst_mac_vap, pst_frame_hdr);
    if (ul_ret != OAL_SUCC) {
        return OAL_SUCC;
    }

    /* 考虑四地址情况获取报文的tid */
    uc_is_tods = mac_hdr_get_to_ds((oal_uint8 *)pst_frame_hdr);
    uc_is_from_ds = mac_hdr_get_from_ds((oal_uint8 *)pst_frame_hdr);
    en_is_4addr = uc_is_tods && uc_is_from_ds;
    uc_tid = mac_get_tid_value((oal_uint8 *)pst_frame_hdr, en_is_4addr);

    pst_ba_rx_hdl = pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info;
    if (pst_ba_rx_hdl == OAL_PTR_NULL) {
        return OAL_SUCC;
    }
    if (pst_ba_rx_hdl->en_ba_status != DMAC_BA_COMPLETE) {
        OAM_WARNING_LOG1(pst_cb_ctrl->uc_mac_vap_id, OAM_SF_BA,
                         "{hmac_ba_filter_serv_etc::ba_status = %d.",
                         pst_ba_rx_hdl->en_ba_status);
        return OAL_SUCC;
    }

    /* 暂时保存BA窗口的序列号，用于鉴别是否有帧上报 */
    us_baw_start_temp = pst_ba_rx_hdl->us_baw_start;

    us_seq_num = mac_get_seq_num((oal_uint8 *)pst_frame_hdr);

    if ((oal_bool_enum_uint8)pst_frame_hdr->st_frame_control.bit_more_frag == OAL_TRUE) {
        OAM_WARNING_LOG1(pst_cb_ctrl->uc_mac_vap_id, OAM_SF_BA,
                         "{hmac_ba_filter_serv_etc::We get a frag_frame[seq_num=%d] When BA_session is set UP!",
                         us_seq_num);
        return OAL_SUCC;
    }

    /* duplicate frame判断 */
    if (OAL_TRUE == hmac_ba_rx_seqno_lt(us_seq_num, pst_ba_rx_hdl->us_baw_start)) {
        /* 上次非定时器上报，直接删除duplicate frame帧，否则，直接上报 */
        if (pst_ba_rx_hdl->en_timer_triggered == OAL_FALSE) {
            /* 确实已经收到该帧 */
            if (hmac_ba_isset(pst_ba_rx_hdl, us_seq_num)) {
                HMAC_USER_STATS_PKT_INCR(pst_hmac_user->ul_rx_pkt_drop, 1);
                return OAL_FAIL;
            }
        }

        return OAL_SUCC;
    }

    /* 更新tail */
    if (OAL_TRUE == hmac_ba_seqno_lt(pst_ba_rx_hdl->us_baw_tail, us_seq_num)) {
        pst_ba_rx_hdl->us_baw_tail = us_seq_num;
    }

    /* 接收到的帧的序列号等于BAW_START，并且缓存队列帧个数为0，则直接上报给HMAC */
    if ((us_seq_num == pst_ba_rx_hdl->us_baw_start) && (pst_ba_rx_hdl->uc_mpdu_cnt == 0)
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        /* offload 下amsdu帧由于可能多个buffer组成，一律走重排序 */
        && (pst_cb_ctrl->bit_amsdu_enable == OAL_FALSE)
#endif
    ) {
        pst_ba_rx_hdl->us_baw_start = DMAC_BA_SEQNO_ADD(pst_ba_rx_hdl->us_baw_start, 1);
        pst_ba_rx_hdl->us_baw_end = DMAC_BA_SEQNO_ADD(pst_ba_rx_hdl->us_baw_end, 1);
    } else {
        /* Buffer the new MSDU */
        *pen_is_ba_buf = OAL_TRUE;

        /* buffer frame to reorder */
        hmac_ba_buffer_rx_frame(pst_ba_rx_hdl, pst_cb_ctrl, pst_netbuf_header, us_seq_num);

        /* put the reordered netbufs to the end of the list */
        hmac_ba_reorder_rx_data(pst_ba_rx_hdl, pst_netbuf_header, pst_mac_vap, us_seq_num);

        /* Check for Sync loss and flush the reorder queue when one is detected */
        if ((pst_ba_rx_hdl->us_baw_tail == DMAC_BA_SEQNO_SUB(pst_ba_rx_hdl->us_baw_start, 1)) &&
            (pst_ba_rx_hdl->uc_mpdu_cnt > 0)) {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                             "{hmac_ba_filter_serv_etc::Sync loss and flush the reorder queue.}");
            hmac_ba_flush_reorder_q(pst_ba_rx_hdl);
        }

        /* 重排序队列刷新后,如果队列中有帧那么启动定时器 */
        if (pst_ba_rx_hdl->uc_mpdu_cnt > 0) {
            hmac_reorder_ba_timer_start_etc(pst_hmac_vap, pst_hmac_user, uc_tid);
        }
    }

    if (us_baw_start_temp != pst_ba_rx_hdl->us_baw_start) {
        pst_ba_rx_hdl->en_timer_triggered = OAL_FALSE;
    }

    return OAL_SUCC;
}


oal_void hmac_reorder_ba_rx_buffer_bar_etc(hmac_ba_rx_stru *pst_rx_ba,
                                           oal_uint16 us_start_seq_num,
                                           mac_vap_stru *pst_vap)
{
    oal_netbuf_head_stru st_netbuf_head;
    oal_uint8 uc_seqnum_pos;

    if (pst_rx_ba == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_BA,
                         "{hmac_reorder_ba_rx_buffer_bar_etc::receive a bar, but ba session doesnot set up.}");
        return;
    }

    /* 针对 BAR 的SSN和窗口的start_num相等时，不需要移窗 */
    if (pst_rx_ba->us_baw_start == us_start_seq_num) {
        OAM_INFO_LOG0(0, OAM_SF_BA, "{hmac_reorder_ba_rx_buffer_bar_etc::seq is equal to start num.}");
        return;
    }

    oal_netbuf_list_head_init(&st_netbuf_head);

    uc_seqnum_pos = hmac_ba_seqno_bound_chk(pst_rx_ba->us_baw_start, pst_rx_ba->us_baw_end, us_start_seq_num);
    /* 针对BAR的的SSN在窗口内才移窗 */
    if (uc_seqnum_pos == DMAC_BA_BETWEEN_SEQLO_SEQHI) {
        hmac_ba_send_frames_with_gap(pst_rx_ba, &st_netbuf_head, us_start_seq_num, pst_vap);
        pst_rx_ba->us_baw_start = us_start_seq_num;
        pst_rx_ba->us_baw_start = hmac_ba_send_frames_in_order(pst_rx_ba, &st_netbuf_head, pst_vap);
        pst_rx_ba->us_baw_end = HMAC_BA_SEQNO_ADD(pst_rx_ba->us_baw_start, (pst_rx_ba->us_baw_size - 1));

        OAM_WARNING_LOG3(pst_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_reorder_ba_rx_buffer_bar_etc::receive a bar, us_baw_start=%d us_baw_end=%d. us_seq_num=%d.}",
            pst_rx_ba->us_baw_start, pst_rx_ba->us_baw_end, us_start_seq_num);
        hmac_rx_lan_frame_etc(&st_netbuf_head);
    } else if (uc_seqnum_pos == DMAC_BA_GREATER_THAN_SEQHI) {
        /* 异常 */
        OAM_WARNING_LOG3(pst_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_reorder_ba_rx_buffer_bar_etc::recv a bar and ssn out of winsize,baw_start=%d baw_end=%d seq_num=%d}",
            pst_rx_ba->us_baw_start, pst_rx_ba->us_baw_end, us_start_seq_num);
    } else {
        OAM_WARNING_LOG3(pst_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_reorder_ba_rx_buffer_bar_etc::recv a bar ssn less than baw_start,baw_start=%d end=%d seq_num=%d}",
            pst_rx_ba->us_baw_start, pst_rx_ba->us_baw_end, us_start_seq_num);
    }
}


OAL_STATIC oal_uint32 hmac_ba_rx_prepare_bufflist(hmac_vap_stru *pst_hmac_vap,
                                                  hmac_rx_buf_stru *pst_rx_buf,
                                                  oal_netbuf_head_stru *pst_netbuf_head)
{
    oal_netbuf_stru *pst_netbuf;
    oal_uint8 uc_loop_index;

    pst_netbuf = oal_netbuf_peek(&pst_rx_buf->st_netbuf_head);
    if (pst_netbuf == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_ba_rx_prepare_bufflist::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (uc_loop_index = 0; uc_loop_index < pst_rx_buf->uc_num_buf; uc_loop_index++) {
        pst_netbuf = oal_netbuf_delist(&pst_rx_buf->st_netbuf_head);
        if (pst_netbuf != OAL_PTR_NULL) {
            oal_netbuf_add_to_list_tail(pst_netbuf, pst_netbuf_head);
        } else {
            OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                             "{hmac_ba_rx_prepare_bufflist::uc_num_buf in reorder list is error.}");
        }
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_ba_send_reorder_timeout(hmac_ba_rx_stru *pst_rx_ba,
                                                   hmac_vap_stru *pst_hmac_vap,
                                                   hmac_ba_alarm_stru *pst_alarm_data,
                                                   oal_uint16 *pus_timeout)
{
    oal_uint32 ul_time_diff, ul_rx_timeout, ul_ret;
    oal_netbuf_head_stru st_netbuf_head;
    oal_uint16 us_baw_head, us_baw_end, us_baw_start; /* 保存最初的窗口起始序列号 */
    hmac_rx_buf_stru *pst_rx_buf = OAL_PTR_NULL;
    oal_uint8 uc_buff_count = 0;
    oal_uint16 uc_send_count = 0;

    oal_netbuf_list_head_init(&st_netbuf_head);
    us_baw_head = pst_rx_ba->us_baw_start;
    us_baw_start = pst_rx_ba->us_baw_start;
    us_baw_end = HMAC_BA_SEQNO_ADD(pst_rx_ba->us_baw_tail, 1);
    ul_rx_timeout = (oal_uint32)(*pus_timeout);

    oal_spin_lock(&pst_rx_ba->st_ba_lock);

    while (us_baw_head != us_baw_end) {
        pst_rx_buf = hmac_get_frame_from_reorder_q(pst_rx_ba, us_baw_head);
        if (pst_rx_buf == OAL_PTR_NULL) {
            uc_buff_count++;
            us_baw_head = HMAC_BA_SEQNO_ADD(us_baw_head, 1);
            continue;
        }

        /* 如果冲排序队列中的帧滞留时间小于定时器超时时间,那么暂时不强制flush */
        ul_time_diff = (oal_uint32)OAL_TIME_GET_STAMP_MS() - pst_rx_buf->ul_rx_time;
        if (ul_time_diff < ul_rx_timeout) {
            *pus_timeout = (oal_uint16)(ul_rx_timeout - ul_time_diff);
            break;
        }

        pst_rx_ba->uc_mpdu_cnt--;
        pst_rx_buf->in_use = 0;

        ul_ret = hmac_ba_rx_prepare_bufflist(pst_hmac_vap, pst_rx_buf, &st_netbuf_head);
        if (ul_ret != OAL_SUCC) {
            uc_buff_count++;
            us_baw_head = HMAC_BA_SEQNO_ADD(us_baw_head, 1);
            continue;
        }

        uc_send_count++;
        uc_buff_count++;
        us_baw_head = HMAC_BA_SEQNO_ADD(us_baw_head, 1);
        pst_rx_ba->us_baw_start = HMAC_BA_SEQNO_ADD(pst_rx_ba->us_baw_start, uc_buff_count);
        pst_rx_ba->us_baw_end = HMAC_BA_SEQNO_ADD(pst_rx_ba->us_baw_start, (pst_rx_ba->us_baw_size - 1));

        uc_buff_count = 0;
    }

    oal_spin_unlock(&pst_rx_ba->st_ba_lock);

    /* 判断本次定时器超时是否有帧上报 */
    if (us_baw_start != pst_rx_ba->us_baw_start) {
        pst_rx_ba->en_timer_triggered = OAL_TRUE;
        OAM_WARNING_LOG4(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
            "{hmac_ba_send_reorder_timeout::new baw_start:%d, old baw_start:%d, ul_rx_timeout:%d, send mpdu cnt:%d.}",
            pst_rx_ba->us_baw_start, us_baw_start, ul_rx_timeout, uc_send_count);
    }

    hmac_rx_lan_frame_etc(&st_netbuf_head);

    return OAL_SUCC;
}


oal_uint32 hmac_ba_timeout_fn_etc(oal_void *p_arg)
{
    hmac_ba_rx_stru *pst_rx_ba = OAL_PTR_NULL;
    hmac_vap_stru *pst_vap = OAL_PTR_NULL;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    hmac_ba_alarm_stru *pst_alarm_data;
    mac_delba_initiator_enum_uint8 en_direction;
    oal_uint8 uc_tid;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    oal_uint16 us_timeout = 0;

    pst_alarm_data = (hmac_ba_alarm_stru *)p_arg;

    en_direction = pst_alarm_data->en_direction;

    uc_tid = pst_alarm_data->uc_tid;
    if (uc_tid >= WLAN_TID_MAX_NUM) {
        OAM_ERROR_LOG1(0, OAM_SF_BA, "{hmac_ba_timeout_fn_etc::tid %d overflow.}", uc_tid);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_alarm_data->uc_vap_id);
    if (pst_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_BA, "{hmac_ba_timeout_fn_etc::pst_vap null. vap id %d.}", pst_alarm_data->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(pst_alarm_data->us_mac_user_idx);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_BA, "{hmac_ba_timeout_fn_etc::pst_hmac_user null. user idx %d.}",
                         pst_alarm_data->us_mac_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev_etc(pst_vap->st_vap_base_info.uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_BA, "{hmac_ba_timeout_fn_etc::pst_mac_device null. uc_device_id id %d.}",
                       pst_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_device->ul_core_id >= WLAN_FRW_MAX_NUM_CORES) {
        OAM_ERROR_LOG1(0, OAM_SF_BA, "{hmac_ba_timeout_fn_etc::core id %d overflow.}", pst_mac_device->ul_core_id);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    if (en_direction == MAC_RECIPIENT_DELBA) {
        pst_rx_ba = (hmac_ba_rx_stru *)pst_alarm_data->pst_ba;

        if (pst_rx_ba == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(0, OAM_SF_BA, "{hmac_ba_timeout_fn_etc::pst_rx_ba is null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 接收业务量较少时只能靠超时定时器冲刷重排序队列,为改善游戏帧延时,需要将超时时间设小 */
        if (OAL_FALSE == hmac_wifi_rx_is_busy()) {
            us_timeout = pst_vap->us_rx_timeout_min[WLAN_WME_TID_TO_AC(uc_tid)];
        } else {
            us_timeout = pst_vap->us_rx_timeout[WLAN_WME_TID_TO_AC(uc_tid)];
        }

        if (pst_rx_ba->uc_mpdu_cnt > 0) {
            hmac_ba_send_reorder_timeout(pst_rx_ba, pst_vap, pst_alarm_data, &us_timeout);
        }

        /* 若重排序队列刷新后,依然有缓存帧则需要重启定时器;
           若重排序队列无帧则为了节省功耗不启动定时器,在有帧入队时重启 */
        if (pst_rx_ba->uc_mpdu_cnt > 0) {
            oal_spin_lock(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer_lock));
            /* 此处不需要判断定时器是否已经启动,如果未启动则启动定时器;
               如果此定时器已经启动 */
            FRW_TIMER_CREATE_TIMER(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer),
                                   hmac_ba_timeout_fn_etc,
                                   us_timeout,
                                   pst_alarm_data,
                                   OAL_FALSE,
                                   OAM_MODULE_ID_HMAC,
                                   pst_mac_device->ul_core_id);

            oal_spin_unlock(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer_lock));
        }

    } else {
        /* tx ba不删除 */
        FRW_TIMER_CREATE_TIMER(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer),
                               hmac_ba_timeout_fn_etc,
                               pst_vap->us_rx_timeout[WLAN_WME_TID_TO_AC(uc_tid)],
                               pst_alarm_data,
                               OAL_FALSE,
                               OAM_MODULE_ID_HMAC,
                               pst_mac_device->ul_core_id);
    }

    return OAL_SUCC;
}


oal_uint32 hmac_ba_reset_rx_handle_etc(mac_device_stru *pst_mac_device,
                                       hmac_user_stru *pst_hmac_user, oal_uint8 uc_tid,
                                       oal_bool_enum_uint8 en_is_aging)
{
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    mac_chip_stru *pst_mac_chip = OAL_PTR_NULL;
    oal_bool_enum en_need_del_lut = OAL_TRUE;
    hmac_ba_rx_stru *pst_ba_rx_info = pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info;

    if (OAL_UNLIKELY((pst_ba_rx_info == OAL_PTR_NULL) || (pst_ba_rx_info->en_is_ba != OAL_TRUE))) {
        OAM_WARNING_LOG0(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle_etc::rx ba not set yet.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (uc_tid >= WLAN_TID_MAX_NUM) {
        OAM_ERROR_LOG1(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle_etc::tid %d overflow.}", uc_tid);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_ba_rx_info->st_alarm_data.uc_vap_id);
    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle_etc::pst_hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* Step1: disable the flag of ba session */
    pst_ba_rx_info->en_is_ba = OAL_FALSE;

    /* Step2: flush reorder q */
    hmac_ba_flush_reorder_q(pst_ba_rx_info);

    if (pst_ba_rx_info->uc_lut_index == MAC_INVALID_RX_BA_LUT_INDEX) {
        en_need_del_lut = OAL_FALSE;
        OAM_WARNING_LOG1(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle_etc::no need to del lut index, lut index[%d]}\n",
                         pst_ba_rx_info->uc_lut_index);
    }

    /* Step3: if lut index is valid, del lut index alloc before */
    if ((pst_ba_rx_info->uc_ba_policy == MAC_BA_POLICY_IMMEDIATE) && (en_need_del_lut == OAL_TRUE)) {
        pst_mac_chip = hmac_res_get_mac_chip(pst_mac_device->uc_chip_id);
        if (pst_mac_chip == OAL_PTR_NULL) {
            return OAL_ERR_CODE_PTR_NULL;
        }
        hmac_ba_del_lut_index(pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_table, pst_ba_rx_info->uc_lut_index);
    }

    /* Step4: dec the ba session cnt maitence in device struc */
    hmac_rx_ba_session_decr_etc(pst_hmac_vap, uc_tid);

    oal_spin_lock(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer_lock));
    /* Step5: Del Timer */
    if (pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer.en_is_registerd == OAL_TRUE) {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer));
    }
    oal_spin_unlock(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer_lock));

    /* Step6: Free rx handle */
    OAL_MEM_FREE(pst_ba_rx_info, OAL_TRUE);

    pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint8 hmac_mgmt_check_set_rx_ba_ok_etc(hmac_vap_stru *pst_hmac_vap,
                                           hmac_user_stru *pst_hmac_user,
                                           hmac_ba_rx_stru *pst_ba_rx_info,
                                           mac_device_stru *pst_device,
                                           hmac_tid_stru *pst_tid_info)
{
    mac_chip_stru *pst_mac_chip = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_RX_AGGR_EXTEND
    oal_uint8 uc_max_rx_ba_size;
#endif

    pst_ba_rx_info->uc_lut_index = MAC_INVALID_RX_BA_LUT_INDEX;

    /* 立即块确认判断 */
    if (pst_ba_rx_info->uc_ba_policy == MAC_BA_POLICY_IMMEDIATE) {
        if (OAL_FALSE == mac_mib_get_dot11ImmediateBlockAckOptionImplemented(&pst_hmac_vap->st_vap_base_info)) {
            /* 不支持立即块确认 */
            OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                             "{hmac_mgmt_check_set_rx_ba_ok_etc::not support immediate Block Ack.}");
            return MAC_INVALID_REQ_PARAMS;
        } else {
            if (pst_ba_rx_info->en_back_var != MAC_BACK_COMPRESSED) {
                /* 不支持非压缩块确认 */
                OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                                 "{hmac_mgmt_check_set_rx_ba_ok_etc::not support non-Compressed Block Ack.}");
                return MAC_REQ_DECLINED;
            }
        }
    } else if (pst_ba_rx_info->uc_ba_policy == MAC_BA_POLICY_DELAYED) {
        /* 延迟块确认不支持 */
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_check_set_rx_ba_ok_etc::not support delayed Block Ack.}");
        return MAC_INVALID_REQ_PARAMS;
    }
    pst_mac_chip = hmac_res_get_mac_chip(pst_device->uc_chip_id);
    if (pst_mac_chip == OAL_PTR_NULL) {
        return MAC_REQ_DECLINED;
    }
    if (mac_mib_get_RxBASessionNumber(&pst_hmac_vap->st_vap_base_info) > WLAN_MAX_RX_BA) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_check_set_rx_ba_ok_etc::uc_rx_ba_session_num[%d] is up to max.}\r\n",
                         mac_mib_get_RxBASessionNumber(&pst_hmac_vap->st_vap_base_info));
        return MAC_REQ_DECLINED;
    }

    /* 获取BA LUT INDEX */
    pst_ba_rx_info->uc_lut_index = hmac_ba_get_lut_index(pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_table,
                                                         0, HAL_MAX_RX_BA_LUT_SIZE);
#ifdef _PRE_WLAN_FEATURE_RX_AGGR_EXTEND
    oal_reset_lut_index_status(pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_status_table,
                               HAL_MAX_RX_BA_LUT_SIZE, pst_ba_rx_info->uc_lut_index);
#endif
    /* LUT index表已满 */
    if (pst_ba_rx_info->uc_lut_index == MAC_INVALID_RX_BA_LUT_INDEX) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                       "{hmac_mgmt_check_set_rx_ba_ok_etc::ba lut index table full.");
        return MAC_REQ_DECLINED;
    }

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* 共存黑名单用户，不建立聚合，直到对应业务将标记清除 */
    if (OAL_TRUE == MAC_BTCOEX_CHECK_VALID_STA(&(pst_hmac_vap->st_vap_base_info))) {
        if (pst_hmac_user->st_hmac_user_btcoex.st_hmac_btcoex_addba_req.en_ba_handle_allow == OAL_FALSE) {
            OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                             "{hmac_mgmt_check_set_rx_ba_ok_etc::btcoex blacklist user, not addba!");
            return MAC_REQ_DECLINED;
        }
    }
#endif

    /* 该tid下不允许建BA，配置命令需求 */
    if (pst_tid_info->en_ba_handle_rx_enable == OAL_FALSE) {
        OAM_WARNING_LOG2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_check_set_rx_ba_ok_etc::uc_tid_no[%d] user_idx[%d] is not available.}",
                         pst_tid_info->uc_tid_no, pst_tid_info->us_hmac_user_idx);
        return MAC_REQ_DECLINED;
    }

    return MAC_SUCCESSFUL_STATUSCODE;
}


oal_void hmac_up_rx_bar_etc(hmac_vap_stru *pst_hmac_vap, dmac_rx_ctl_stru *pst_rx_ctl, oal_netbuf_stru *pst_netbuf)
{
    oal_uint8 *puc_payload = OAL_PTR_NULL;
    mac_ieee80211_frame_stru *pst_frame_hdr = OAL_PTR_NULL;
    oal_uint8 *puc_sa_addr = OAL_PTR_NULL;
    oal_uint8 uc_tidno;
    hmac_user_stru *pst_ta_user = OAL_PTR_NULL;
    oal_uint16 us_start_seqnum;
    hmac_ba_rx_stru *pst_ba_rx_info = OAL_PTR_NULL;
    mac_rx_ctl_stru *pst_rx_info = OAL_PTR_NULL;
    oal_uint16 us_frame_len;

    pst_frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(pst_rx_ctl->st_rx_info));
    puc_sa_addr = pst_frame_hdr->auc_address2;
    pst_rx_info = (mac_rx_ctl_stru *)(&(pst_rx_ctl->st_rx_info));
    us_frame_len = pst_rx_info->us_frame_len - pst_rx_info->uc_mac_header_len;

    /*  获取用户指针 */
    pst_ta_user = mac_vap_get_hmac_user_by_addr_etc(&(pst_hmac_vap->st_vap_base_info), puc_sa_addr);
    if (pst_ta_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_up_rx_bar_etc::pst_ta_user  is null.}");
        return;
    }

    /* BAR Control(2) +  BlockAck Starting seq num)(2) */
    if (us_frame_len < 4) {
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{hmac_up_rx_bar_etc:frame len err. frame len[%d], machdr len[%d].}",
            pst_rx_info->us_frame_len, pst_rx_info->uc_mac_header_len);
        return;
    }

    /* 获取帧头和payload指针 */
    puc_payload = MAC_GET_RX_PAYLOAD_ADDR(&(pst_rx_ctl->st_rx_info), pst_netbuf);

    /*************************************************************************/
    /*                     BlockAck Request Frame Format                     */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BAR Control|BlockAck Starting    |FCS|  */
    /* |             |        |  |  |           |Sequence number      |   |  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |2          |2                    |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    uc_tidno = (puc_payload[1] & 0xF0) >> 4;
    if (uc_tidno >= WLAN_TID_MAX_NUM) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_up_rx_bar_etc::uc_tidno[%d] invalid.}", uc_tidno);
        return;
    }
    us_start_seqnum = mac_get_bar_start_seq_num(puc_payload);
    pst_ba_rx_info = pst_ta_user->ast_tid_info[uc_tidno].pst_ba_rx_info;

    hmac_reorder_ba_rx_buffer_bar_etc(pst_ba_rx_info, us_start_seqnum, &(pst_hmac_vap->st_vap_base_info));
}

oal_bool_enum_uint8 hmac_is_device_ba_setup_etc(void)
{
    oal_uint8 uc_vap_id;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;

    for (uc_vap_id = 0; uc_vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_vap_id++) {
        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
        if (pst_hmac_vap == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_is_device_ba_setup_etc pst_mac_vap is null.}");
            continue;
        }
        if ((pst_hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_UP) &&
            (pst_hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_PAUSE)) {
            continue;
        }
        if ((mac_mib_get_TxBASessionNumber(&pst_hmac_vap->st_vap_base_info) != 0) ||
            (mac_mib_get_RxBASessionNumber(&pst_hmac_vap->st_vap_base_info) != 0)) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}


oal_uint32 hmac_ps_rx_delba_etc(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    mac_vap_rom_stru *pst_mac_vap_rom  = OAL_PTR_NULL;
    mac_cfg_delba_req_param_stru st_mac_cfg_delba_param;
    dmac_to_hmac_ps_rx_delba_event_stru *pst_dmac_to_hmac_ps_rx_delba = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    pst_mac_vap_rom = (mac_vap_rom_stru *)(pst_mac_vap->_rom);
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                       "{hmac_ps_rx_delba_etc::pst_hmac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_to_hmac_ps_rx_delba = (dmac_to_hmac_ps_rx_delba_event_stru *)puc_param;

    pst_hmac_user = mac_res_get_hmac_user_etc(pst_dmac_to_hmac_ps_rx_delba->us_user_id);
    if (OAL_UNLIKELY(pst_hmac_user == OAL_PTR_NULL)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                         "{hmac_ps_rx_delba_etc::pst_hmac_user is null! user_id is %d.}",
                         pst_dmac_to_hmac_ps_rx_delba->us_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 刷新HMAC VAP下PS业务时RX AMSDU使能标志 */
    pst_mac_vap_rom->en_ps_rx_amsdu = pst_dmac_to_hmac_ps_rx_delba->en_rx_amsdu;
    pst_mac_vap_rom->uc_ps_type |= pst_dmac_to_hmac_ps_rx_delba->uc_ps_type;
    /* 保证命令优先级最高 */
    if (pst_mac_vap_rom->uc_ps_type & MAC_PS_TYPE_CMD) {
        return OAL_SUCC;
    }
    pst_hmac_vap->en_ps_rx_amsdu = pst_mac_vap_rom->en_ps_rx_amsdu;
    memset_s((oal_uint8 *)&st_mac_cfg_delba_param, OAL_SIZEOF(st_mac_cfg_delba_param),
             0, OAL_SIZEOF(st_mac_cfg_delba_param));
    oal_set_mac_addr(st_mac_cfg_delba_param.auc_mac_addr, pst_hmac_user->st_user_base_info.auc_user_mac_addr);
    st_mac_cfg_delba_param.en_direction = MAC_RECIPIENT_DELBA;
    st_mac_cfg_delba_param.en_trigger = MAC_DELBA_TRIGGER_PS;

    for (st_mac_cfg_delba_param.uc_tidno = 0;
         st_mac_cfg_delba_param.uc_tidno < WLAN_TID_MAX_NUM; st_mac_cfg_delba_param.uc_tidno++) {
        ul_ret = hmac_config_delba_req_etc(pst_mac_vap, 0, (oal_uint8 *)&st_mac_cfg_delba_param);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                "{hmac_ps_rx_delba_etc::delba send failed, ul_ret: %d, tid: %d}",
                ul_ret, st_mac_cfg_delba_param.uc_tidno);
            return ul_ret;
        }
    }
    return OAL_SUCC;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

