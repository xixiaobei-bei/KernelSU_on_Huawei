

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
#include "hmac_statistic_data_flow.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_BLOCKACK_C

/* 2 全局变量定义 */
/* 3 函数实现 */

OAL_STATIC hmac_rx_buf_stru *hmac_ba_buffer_frame_in_reorder(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                             oal_uint16 us_seq_num,
                                                             mac_rx_ctl_stru *pst_cb_ctrl,
                                                             oal_netbuf_head_stru *pst_netbuf_header)
{
    oal_uint16           us_idx;
    oal_uint8            uc_bitmap_index;
    oal_uint8            uc_reorder_index;
    hmac_rx_buf_stru    *pst_rx_buf;
    oal_netbuf_stru     *pst_netbuf;

    uc_bitmap_index = hmac_tcp_ack_bitmap_index_get(us_seq_num);
    uc_reorder_index = hmac_tcp_ack_reorder_index_get(us_seq_num);

    pst_rx_buf = &(pst_ba_rx_hdl->ast_re_order_list[uc_reorder_index]);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    if (pst_rx_buf->in_use == 1) {
        hmac_rx_free_netbuf_list(&pst_rx_buf->st_netbuf_head, pst_rx_buf->uc_num_buf);
        OAM_INFO_LOG1(0, OAM_SF_BA, "{hmac_ba_buffer_frame_in_reorder::slot already used, seq[%d].}", us_seq_num);
    } else {
        pst_ba_rx_hdl->uc_mpdu_cnt++;
    }

    pst_rx_buf->uc_num_buf = pst_cb_ctrl->bit_buff_nums;  // 标识该MPDU占用的netbuff个数，一般用于AMSDU

    pst_rx_buf->in_use = 1;
#else
    if (pst_rx_buf->in_use == 1) {
        /*
         * tcp ack过滤机制下, 若某个netbuf被释放的信息丢失(如上报使用的bitmap不够大)
         * 可能造成reorder队列中seq_num对应的netbuf还未上报，seq_num + 64的netbuf就已到来，覆盖掉原netbuf
         * 出现该情况时，需要上报reorder队列中原来的netbuf
         */
        for (us_idx = pst_rx_buf->uc_num_buf; us_idx > 0; us_idx--) {
            pst_netbuf = oal_netbuf_delist(&pst_rx_buf->st_netbuf_head);
            if (pst_netbuf != OAL_PTR_NULL) {
                oal_netbuf_add_to_list_tail(pst_netbuf, pst_netbuf_header);
            }
        }

        pst_ba_rx_hdl->uc_mpdu_cnt--;
        pst_rx_buf->in_use = 0;
        pst_rx_buf->uc_num_buf = 0;
        OAM_INFO_LOG1(0, OAM_SF_BA, "{hmac_ba_buffer_frame_in_reorder::slot already used, seq[%d].}", us_seq_num);
    }

    if (pst_rx_buf->en_tcp_ack_filtered[uc_bitmap_index]) {
        OAM_WARNING_LOG0(0, OAM_SF_BA, "{hmac_ba_buffer_frame_in_reorder::en_tcp_ack_filtered!}");
        pst_rx_buf->en_tcp_ack_filtered[uc_bitmap_index] = OAL_FALSE;
    }

    if (pst_cb_ctrl->bit_amsdu_enable == OAL_TRUE) {
        if (pst_cb_ctrl->bit_is_first_buffer == OAL_TRUE) {
            /* 当收到当前seq第一帧，如果当前seq对应链表中已有数据，则释放并且计数清零 */
            if (oal_netbuf_list_len(&pst_rx_buf->st_netbuf_head) != 0) {
                hmac_rx_free_netbuf_list(&pst_rx_buf->st_netbuf_head, oal_netbuf_list_len(&pst_rx_buf->st_netbuf_head));
                OAM_INFO_LOG1(0, OAM_SF_BA,
                              "{hmac_ba_buffer_frame_in_reorder::seq[%d] amsdu first buffer, need clear st_netbuf_head first}", us_seq_num);
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
#endif
    return pst_rx_buf;
}


OAL_STATIC oal_uint32 hmac_ba_send_frames_with_gap(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                   oal_netbuf_head_stru *pst_netbuf_header,
                                                   oal_uint16 us_last_seqnum,
                                                   mac_vap_stru *pst_vap)
{
    oal_uint16               us_seq_num;
    oal_uint8                uc_num_frms = 0;
    oal_uint8                uc_loop_index;
    oal_bool_enum_uint8      en_tcp_ack_filtered;
    oal_netbuf_stru         *pst_netbuf;
    hmac_rx_buf_stru        *pst_rx_buf = OAL_PTR_NULL;

    us_seq_num = pst_ba_rx_hdl->us_baw_start;

    OAM_INFO_LOG1(pst_vap->uc_vap_id, OAM_SF_BA, "{hmac_ba_send_frames_with_gap::to seq[%d].}", us_last_seqnum);

    while (us_seq_num != us_last_seqnum) {
        if ((pst_rx_buf = hmac_remove_frame_from_reorder_q(pst_ba_rx_hdl, us_seq_num, &en_tcp_ack_filtered)) != OAL_PTR_NULL) {
            pst_ba_rx_hdl->uc_mpdu_cnt--;
            pst_netbuf = oal_netbuf_peek(&pst_rx_buf->st_netbuf_head);
            if (OAL_UNLIKELY(pst_netbuf == OAL_PTR_NULL)) {
                OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_BA,
                                 "{hmac_ba_send_frames_with_gap::gap[%d].\r\n}",
                                 us_seq_num);

                us_seq_num = HMAC_BA_SEQNO_ADD(us_seq_num, 1);
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

        us_seq_num = HMAC_BA_SEQNO_ADD(us_seq_num, 1);
    }

    if (pst_ba_rx_hdl->uc_mpdu_cnt != 0) {
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_BA,
                         "{hmac_ba_send_frames_with_gap::uc_mpdu_cnt=%d.}",
                         pst_ba_rx_hdl->uc_mpdu_cnt);
    }

    return uc_num_frms;
}


OAL_STATIC oal_uint16 hmac_ba_send_frames_in_order(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                   oal_netbuf_head_stru *pst_netbuf_header,
                                                   mac_vap_stru *pst_vap)
{
    oal_uint16               us_seq_num;
    oal_uint8                uc_loop_index;
    oal_bool_enum_uint8      en_tcp_ack_filtered;
    oal_netbuf_stru         *pst_netbuf;
    hmac_rx_buf_stru        *pst_rx_buf = OAL_PTR_NULL;

    us_seq_num = pst_ba_rx_hdl->us_baw_start;
    pst_rx_buf = hmac_remove_frame_from_reorder_q(pst_ba_rx_hdl, us_seq_num, &en_tcp_ack_filtered);
    while (pst_rx_buf != OAL_PTR_NULL || en_tcp_ack_filtered) {
        if (pst_rx_buf != OAL_PTR_NULL) {
            pst_ba_rx_hdl->uc_mpdu_cnt--;
            pst_netbuf = oal_netbuf_peek(&pst_rx_buf->st_netbuf_head);

            if (pst_netbuf == OAL_PTR_NULL) {
                OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_BA,
                                 "{hmac_ba_send_frames_in_order::[%d] slot error.}", us_seq_num);
            } else {
                for (uc_loop_index = 0; uc_loop_index < pst_rx_buf->uc_num_buf; uc_loop_index++) {
                    pst_netbuf = oal_netbuf_delist(&pst_rx_buf->st_netbuf_head);
                    OAL_MEM_NETBUF_TRACE(pst_netbuf, OAL_FALSE);
                    if (pst_netbuf != OAL_PTR_NULL) {
                        oal_netbuf_list_tail_nolock(pst_netbuf_header, pst_netbuf);
                    }
                }
            }

            pst_rx_buf->uc_num_buf = 0;
        }

        us_seq_num = HMAC_BA_SEQNO_ADD(us_seq_num, 1);
        pst_rx_buf = hmac_remove_frame_from_reorder_q(pst_ba_rx_hdl, us_seq_num, &en_tcp_ack_filtered);
    }

    return us_seq_num;
}


OAL_STATIC OAL_INLINE oal_void hmac_ba_buffer_rx_frame(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                       hmac_rx_ctl_stru *pst_cb_ctrl,
                                                       oal_netbuf_head_stru *pst_netbuf_header,
                                                       oal_uint16 us_seq_num,
                                                       mac_vap_stru *pst_vap)
{
    hmac_rx_buf_stru *pst_rx_netbuf = OAL_PTR_NULL;
    oal_netbuf_stru  *pst_netbuf;
    oal_uint8         uc_netbuf_index;
#ifdef _PRE_DEBUG_MODE
    oal_uint32        ul_netbuf_num;
#endif

    /* Get the pointer to the buffered packet */
    pst_rx_netbuf = hmac_ba_buffer_frame_in_reorder(pst_ba_rx_hdl,
                                                    us_seq_num,
                                                    &(pst_cb_ctrl->st_rx_info),
                                                    pst_netbuf_header);

    /* Update the buffered receive packet details */
    pst_rx_netbuf->us_seq_num = us_seq_num;
    pst_rx_netbuf->ul_rx_time = (oal_uint32)OAL_TIME_GET_STAMP_MS();

#ifdef _PRE_DEBUG_MODE
    ul_netbuf_num = oal_netbuf_get_buf_num(&pst_rx_netbuf->st_netbuf_head);
    if (ul_netbuf_num != 0) {
        OAM_INFO_LOG1(pst_vap->uc_vap_id, OAM_SF_BA, "{hmac_ba_buffer_rx_frame::%d netbuf miss here.}", ul_netbuf_num);
    }

    if (pst_rx_netbuf->uc_num_buf != 1) {
        OAM_INFO_LOG1(pst_vap->uc_vap_id, OAM_SF_BA,
                      "{hmac_ba_buffer_rx_frame:find amsdu netbuff cnt %d.}",
                      pst_rx_netbuf->uc_num_buf);
    }
#endif

    /* all buffers of this frame must be deleted from the buf list */
    for (uc_netbuf_index = pst_cb_ctrl->st_rx_info.bit_buff_nums; uc_netbuf_index > 0; uc_netbuf_index--) {
        pst_netbuf = oal_netbuf_delist_nolock(pst_netbuf_header);

        OAL_MEM_NETBUF_TRACE(pst_netbuf, OAL_TRUE);
        if (OAL_UNLIKELY(pst_netbuf != OAL_PTR_NULL)) {
            oal_netbuf_add_to_list_tail(pst_netbuf, &pst_rx_netbuf->st_netbuf_head);
        } else {
            OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_BA, "{hmac_ba_buffer_rx_frame:netbuff error in amsdu.}");
        }
    }
    /* 当链表长度和统计个数不一致时，以链表长度为准。异常处理。 */
    if (oal_netbuf_list_len(&pst_rx_netbuf->st_netbuf_head) != pst_rx_netbuf->uc_num_buf) {
        OAM_WARNING_LOG2(pst_vap->uc_vap_id, OAM_SF_BA, "{hmac_ba_buffer_rx_frame: list_len=%d numbuf=%d}",
                         oal_netbuf_list_len(&pst_rx_netbuf->st_netbuf_head), pst_rx_netbuf->uc_num_buf);
        pst_rx_netbuf->uc_num_buf = oal_netbuf_list_len(&pst_rx_netbuf->st_netbuf_head);
    }
}


OAL_STATIC OAL_INLINE oal_void hmac_ba_reorder_rx_data(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                       oal_netbuf_head_stru *pst_netbuf_header,
                                                       mac_vap_stru *pst_vap,
                                                       oal_uint16 us_seq_num)
{
    oal_uint8  uc_seqnum_pos;
    oal_uint16 us_temp_winend;
    oal_uint16 us_temp_winstart;

    uc_seqnum_pos = hmac_ba_seqno_bound_chk(pst_ba_rx_hdl->us_baw_start, pst_ba_rx_hdl->us_baw_end, us_seq_num);

    if (uc_seqnum_pos == DMAC_BA_BETWEEN_SEQLO_SEQHI) {
        pst_ba_rx_hdl->us_baw_start = hmac_ba_send_frames_in_order(pst_ba_rx_hdl, pst_netbuf_header, pst_vap);
        pst_ba_rx_hdl->us_baw_end = DMAC_BA_SEQNO_ADD(pst_ba_rx_hdl->us_baw_start, (pst_ba_rx_hdl->us_baw_size - 1));
    } else if (uc_seqnum_pos == DMAC_BA_GREATER_THAN_SEQHI) {
        us_temp_winend = us_seq_num;
        us_temp_winstart = HMAC_BA_SEQNO_SUB(us_temp_winend, (pst_ba_rx_hdl->us_baw_size - 1));

        hmac_ba_send_frames_with_gap(pst_ba_rx_hdl, pst_netbuf_header, us_temp_winstart, pst_vap);
        pst_ba_rx_hdl->us_baw_start = us_temp_winstart;
        pst_ba_rx_hdl->us_baw_start = hmac_ba_send_frames_in_order(pst_ba_rx_hdl, pst_netbuf_header, pst_vap);
        pst_ba_rx_hdl->us_baw_end = HMAC_BA_SEQNO_ADD(pst_ba_rx_hdl->us_baw_start, (pst_ba_rx_hdl->us_baw_size - 1));
    } else {
        OAM_INFO_LOG3(pst_vap->uc_vap_id, OAM_SF_BA, "{hmac_ba_reorder_rx_data::else branch seqno[%d] ws[%d] we[%d].}",
                      us_seq_num, pst_ba_rx_hdl->us_baw_start, pst_ba_rx_hdl->us_baw_end);
    }
}


OAL_STATIC oal_void hmac_ba_flush_reorder_q(hmac_ba_rx_stru *pst_rx_ba)
{
    hmac_rx_buf_stru *pst_rx_buf = OAL_PTR_NULL;
    oal_uint16        us_index;

    for (us_index = 0; us_index < WLAN_AMPDU_RX_BUFFER_SIZE; us_index++) {
        pst_rx_buf = &(pst_rx_ba->ast_re_order_list[us_index]);

        if (pst_rx_buf->in_use == OAL_TRUE) {
            hmac_rx_free_netbuf_list(&pst_rx_buf->st_netbuf_head, pst_rx_buf->uc_num_buf);

            pst_rx_buf->in_use = OAL_FALSE;
            pst_rx_buf->uc_num_buf = 0;
            pst_rx_ba->uc_mpdu_cnt--;
        }

        pst_rx_buf->en_tcp_ack_filtered[0] = OAL_FALSE;
        pst_rx_buf->en_tcp_ack_filtered[1] = OAL_FALSE;
    }

    if (pst_rx_ba->uc_mpdu_cnt != 0) {
        OAM_WARNING_LOG1(0, OAM_SF_BA, "{hmac_ba_flush_reorder_q:: %d mpdu cnt left.}", pst_rx_ba->uc_mpdu_cnt);
    }
}

OAL_STATIC OAL_INLINE oal_uint32 hmac_ba_check_rx_aggr(mac_vap_stru *pst_vap,
                                                       mac_ieee80211_frame_stru *pst_frame_hdr)
{
    /* 该vap是否是ht */
    if (pst_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11HighThroughputOptionImplemented == OAL_FALSE) {
        OAM_INFO_LOG0(pst_vap->uc_vap_id, OAM_SF_BA, "{hmac_ba_check_rx_aggr::ht not supported by this vap.}");
        return OAL_FAIL;
    }

    /* 判断该帧是不是qos帧 */
    if (((oal_uint8 *)pst_frame_hdr)[0] != (WLAN_FC0_SUBTYPE_QOS | WLAN_FC0_TYPE_DATA)) {
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


oal_uint32 hmac_ba_filter_serv(mac_vap_stru *pst_vap,
                               hmac_user_stru *pst_hmac_user,
                               hmac_rx_ctl_stru *pst_cb_ctrl,
                               mac_ieee80211_frame_stru *pst_frame_hdr,
                               oal_netbuf_head_stru *pst_netbuf_header,
                               oal_bool_enum_uint8 *pen_is_ba_buf)
{
    oal_uint8                uc_tid;
    hmac_ba_rx_stru         *pst_ba_rx_hdl;
    oal_uint16               us_seq_num;
    oal_bool_enum_uint8      en_is_4addr;
    oal_uint8                uc_is_tods;
    oal_uint8                uc_is_from_ds;
    oal_uint16               us_baw_start_temp;
    oal_uint32               ul_ret;

    if (OAL_UNLIKELY(pst_netbuf_header == OAL_PTR_NULL ||
                     pst_cb_ctrl == OAL_PTR_NULL ||
                     pst_frame_hdr == OAL_PTR_NULL ||
                     pen_is_ba_buf == OAL_PTR_NULL ||
                     pst_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_BA, "{hmac_ba_filter_serv::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_ba_check_rx_aggr(pst_vap, pst_frame_hdr);
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
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_BA,
                         "{hmac_ba_filter_serv::ba_status = %d.",
                         pst_ba_rx_hdl->en_ba_status);
        return OAL_SUCC;
    }

    /* 暂时保存BA窗口的序列号，用于鉴别是否有帧上报 */
    us_baw_start_temp = pst_ba_rx_hdl->us_baw_start;

    us_seq_num = mac_get_seq_num((oal_uint8 *)pst_frame_hdr);

    if ((oal_bool_enum_uint8)pst_frame_hdr->st_frame_control.bit_more_frag == OAL_TRUE) {
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_BA,
                         "{hmac_ba_filter_serv::We get a frag_frame[seq_num=%d] When BA_session is set UP!", us_seq_num);
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
    /* restart ba timer */
    if (OAL_TRUE == hmac_ba_seqno_lt(pst_ba_rx_hdl->us_baw_tail, us_seq_num)) {
        pst_ba_rx_hdl->us_baw_tail = us_seq_num;
    }

    /* 接收到的帧的序列号等于BAW_START，并且缓存队列帧个数为0，则直接上报给HMAC */
    if ((pst_ba_rx_hdl->us_baw_start == us_seq_num) && (pst_ba_rx_hdl->uc_mpdu_cnt == 0)
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        /* offload 下amsdu帧由于可能多个buffer组成，一律走重排序 */
        && (pst_cb_ctrl->st_rx_info.bit_amsdu_enable == OAL_FALSE)
#endif
       ) {
        pst_ba_rx_hdl->us_baw_start = DMAC_BA_SEQNO_ADD(pst_ba_rx_hdl->us_baw_start, 1);
        pst_ba_rx_hdl->us_baw_end = DMAC_BA_SEQNO_ADD(pst_ba_rx_hdl->us_baw_end, 1);
    } else {
        /* Buffer the new MSDU */
        *pen_is_ba_buf = OAL_TRUE;

        hmac_ba_buffer_rx_frame(pst_ba_rx_hdl, pst_cb_ctrl, pst_netbuf_header, us_seq_num, pst_vap);

        /* put the reordered netbufs to the end of the list */
        hmac_ba_reorder_rx_data(pst_ba_rx_hdl, pst_netbuf_header, pst_vap, us_seq_num);

        /* Check for Sync loss and flush the reorder queue when one is detected */
        if ((pst_ba_rx_hdl->us_baw_tail == DMAC_BA_SEQNO_SUB(pst_ba_rx_hdl->us_baw_start, 1)) &&
            (pst_ba_rx_hdl->uc_mpdu_cnt > 0)) {
            OAM_WARNING_LOG0(pst_vap->uc_vap_id, OAM_SF_BA,
                             "{hmac_ba_filter_serv::Sync loss and flush the reorder queue.}");
            hmac_ba_flush_reorder_q(pst_ba_rx_hdl);
        }
    }

    if (us_baw_start_temp != pst_ba_rx_hdl->us_baw_start) {
        pst_ba_rx_hdl->en_timer_triggered = OAL_FALSE;
    }

    return OAL_SUCC;
}


oal_void hmac_reorder_ba_rx_buffer_bar(hmac_ba_rx_stru *pst_rx_ba, oal_uint16 us_start_seq_num, mac_vap_stru *pst_vap)
{
    oal_netbuf_head_stru st_netbuf_head;
    oal_uint8            uc_seqnum_pos;

    if (pst_rx_ba == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_BA,
                         "{hmac_reorder_ba_rx_buffer_bar::receive a bar, but ba session doesnot set up.}");
        return;
    }

    /* 针对 BAR 的SSN和窗口的start_num相等时，不需要移窗 */
    if (pst_rx_ba->us_baw_start == us_start_seq_num) {
        OAM_INFO_LOG0(0, OAM_SF_BA, "{hmac_reorder_ba_rx_buffer_bar::seq is equal to start num.}");
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

        OAM_INFO_LOG3(pst_vap->uc_vap_id, OAM_SF_BA,
                      "{hmac_reorder_ba_rx_buffer_bar::receive a bar, us_baw_start=%d us_baw_end=%d. us_seq_num=%d.}",
                      pst_rx_ba->us_baw_start,
                      pst_rx_ba->us_baw_end,
                      us_start_seq_num);

        hmac_rx_lan_frame(&st_netbuf_head);
    } else if (uc_seqnum_pos == DMAC_BA_GREATER_THAN_SEQHI) {
        /* 异常 */
        OAM_WARNING_LOG3(pst_vap->uc_vap_id, OAM_SF_BA,
                         "{hmac_reorder_ba_rx_buffer_bar::receive a bar and ssn is out of winsize, us_baw_start=%d us_baw_end=%d, us_seq_num=%d.}",
                         pst_rx_ba->us_baw_start,
                         pst_rx_ba->us_baw_end,
                         us_start_seq_num);
    }
}


OAL_STATIC oal_uint32 hmac_ba_rx_prepare_bufflist(hmac_vap_stru *pst_hmac_vap,
                                                  hmac_rx_buf_stru *pst_rx_buf,
                                                  oal_netbuf_head_stru *pst_netbuf_head)
{
    oal_netbuf_stru *pst_netbuf;
    oal_uint8        uc_loop_index;

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
    oal_uint32              ul_time_diff;
    oal_uint32              ul_rx_timeout;
    oal_netbuf_head_stru    st_netbuf_head;
    oal_uint16              us_baw_head;
    oal_uint16              us_baw_start; /* 保存最初的窗口起始序列号 */
    hmac_rx_buf_stru       *pst_rx_buf;
    oal_uint8               uc_buff_count = 0;
    oal_uint32              ul_ret;
    oal_uint16              us_baw_end;

    oal_netbuf_list_head_init(&st_netbuf_head);
    us_baw_head = pst_rx_ba->us_baw_start;
    us_baw_start = pst_rx_ba->us_baw_start;
    ul_rx_timeout = (oal_uint32)pst_hmac_vap->us_rx_timeout[WLAN_WME_TID_TO_AC(pst_alarm_data->uc_tid)];
    us_baw_end = HMAC_BA_SEQNO_ADD(pst_rx_ba->us_baw_tail, 1);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_spin_lock(&pst_rx_ba->st_ba_lock);
#endif

    while (us_baw_head != us_baw_end) {
        pst_rx_buf = hmac_get_frame_from_reorder_q(pst_rx_ba, us_baw_head);
        if (pst_rx_buf == OAL_PTR_NULL) {
            uc_buff_count++;
            us_baw_head = HMAC_BA_SEQNO_ADD(us_baw_head, 1);
            continue;
        }

        ul_time_diff = (oal_uint32)OAL_TIME_GET_STAMP_MS() - pst_rx_buf->ul_rx_time;
        if (ul_time_diff < ul_rx_timeout) {
            *pus_timeout = (oal_uint16)(ul_rx_timeout - ul_time_diff);
            break;
        }

        pst_rx_ba->uc_mpdu_cnt--;
        pst_rx_buf->in_use = 0;

        pst_rx_buf->en_tcp_ack_filtered[0] = OAL_FALSE;
        pst_rx_buf->en_tcp_ack_filtered[1] = OAL_FALSE;

        ul_ret = hmac_ba_rx_prepare_bufflist(pst_hmac_vap, pst_rx_buf, &st_netbuf_head);
        if (ul_ret != OAL_SUCC) {
            uc_buff_count++;
            us_baw_head = HMAC_BA_SEQNO_ADD(us_baw_head, 1);
            continue;
        }

        uc_buff_count++;
        us_baw_head = HMAC_BA_SEQNO_ADD(us_baw_head, 1);
        pst_rx_ba->us_baw_start = HMAC_BA_SEQNO_ADD(pst_rx_ba->us_baw_start, uc_buff_count);
        pst_rx_ba->us_baw_end = HMAC_BA_SEQNO_ADD(pst_rx_ba->us_baw_start, (pst_rx_ba->us_baw_size - 1));

        uc_buff_count = 0;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_spin_unlock(&pst_rx_ba->st_ba_lock);
#endif

    /* 判断本次定时器超时是否有帧上报 */
    if (us_baw_start != pst_rx_ba->us_baw_start) {
        pst_rx_ba->en_timer_triggered = OAL_TRUE;
    }

    hmac_rx_lan_frame(&st_netbuf_head);

    return OAL_SUCC;
}


oal_uint32 hmac_ba_timeout_fn(oal_void *p_arg)
{
    hmac_ba_rx_stru                 *pst_rx_ba;
    hmac_vap_stru                   *pst_vap;
    hmac_user_stru                  *pst_hmac_user;
    hmac_ba_alarm_stru              *pst_alarm_data;
    mac_delba_initiator_enum_uint8   en_direction;
    oal_uint8                        uc_tid;
    mac_device_stru                 *pst_mac_device;
    oal_uint16                       us_timeout = 0;

    pst_alarm_data = (hmac_ba_alarm_stru *)p_arg;

    en_direction = pst_alarm_data->en_direction;

    uc_tid = pst_alarm_data->uc_tid;
    if (uc_tid >= WLAN_TID_MAX_NUM) {
        OAM_ERROR_LOG1(0, OAM_SF_BA, "{hmac_ba_timeout_fn::tid %d overflow.}", uc_tid);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_alarm_data->uc_vap_id);
    if (pst_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_BA, "{hmac_ba_timeout_fn::pst_vap null. vap id %d.}", pst_alarm_data->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(pst_alarm_data->us_mac_user_idx);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_BA,
                         "{hmac_ba_timeout_fn::pst_hmac_user null. user idx %d.}",
                         pst_alarm_data->us_mac_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_vap->st_vap_base_info.uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_BA,
                       "{hmac_ba_timeout_fn::pst_mac_device null. uc_device_id id %d.}",
                       pst_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_device->ul_core_id >= WLAN_FRW_MAX_NUM_CORES) {
        OAM_ERROR_LOG1(0, OAM_SF_BA, "{hmac_ba_timeout_fn::core id %d overflow.}", pst_mac_device->ul_core_id);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    if (en_direction == MAC_RECIPIENT_DELBA) {
        pst_rx_ba = (hmac_ba_rx_stru *)pst_alarm_data->pst_ba;

        if (pst_rx_ba == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(0, OAM_SF_BA, "{hmac_ba_timeout_fn::pst_rx_ba is null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 接收业务量较少时只能靠超时定时器冲刷重排序队列，为改善游戏帧延时，需要将超时时间设小 */
        if (OAL_FALSE == hmac_wifi_rx_is_busy()) {
            us_timeout = pst_vap->us_rx_timeout_min[WLAN_WME_TID_TO_AC(uc_tid)];
        } else {
            us_timeout = pst_vap->us_rx_timeout[WLAN_WME_TID_TO_AC(uc_tid)];
        }

        if (pst_rx_ba->uc_mpdu_cnt > 0) {
            hmac_ba_send_reorder_timeout(pst_rx_ba, pst_vap, pst_alarm_data, &us_timeout);
        }

        FRW_TIMER_CREATE_TIMER(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer),
                               hmac_ba_timeout_fn,
                               us_timeout,
                               pst_alarm_data,
                               OAL_FALSE,
                               OAM_MODULE_ID_HMAC,
                               pst_mac_device->ul_core_id);
    } else {
        /* tx ba不删除 */
        FRW_TIMER_CREATE_TIMER(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer),
                               hmac_ba_timeout_fn,
                               pst_vap->us_rx_timeout[WLAN_WME_TID_TO_AC(uc_tid)],
                               pst_alarm_data,
                               OAL_FALSE,
                               OAM_MODULE_ID_HMAC,
                               pst_mac_device->ul_core_id);
    }

    return OAL_SUCC;
}


oal_uint32 hmac_ba_reset_rx_handle(mac_device_stru *pst_mac_device,
                                   hmac_ba_rx_stru **ppst_rx_ba,
                                   oal_uint8 uc_tid,
                                   oal_bool_enum_uint8 en_is_aging)
{
    hmac_vap_stru  *pst_hmac_vap;
    hmac_user_stru *pst_hmac_user;
    oal_bool_enum   en_need_del_lut = OAL_TRUE;

    if (OAL_UNLIKELY((*ppst_rx_ba == OAL_PTR_NULL) || ((*ppst_rx_ba)->en_is_ba != OAL_TRUE))) {
        OAM_WARNING_LOG0(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle::rx ba not set yet.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (uc_tid >= WLAN_TID_MAX_NUM) {
        OAM_ERROR_LOG1(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle::tid %d overflow.}", uc_tid);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap((*ppst_rx_ba)->st_alarm_data.uc_vap_id);
    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle::pst_hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* Step1: disable the flag of ba session */
    (*ppst_rx_ba)->en_is_ba = OAL_FALSE;

    /* Step2: flush reorder q */
    hmac_ba_flush_reorder_q(*ppst_rx_ba);

    if ((*ppst_rx_ba)->uc_lut_index == DMAC_INVALID_BA_LUT_INDEX) {
        en_need_del_lut = OAL_FALSE;
        OAM_WARNING_LOG1(0, OAM_SF_BA,
                         "{hmac_ba_reset_rx_handle::no need to del lut index, lut index[%d]}\n",
                         (*ppst_rx_ba)->uc_lut_index);
    }

    /* Step3: if lut index is valid, del lut index alloc before */
    if (((*ppst_rx_ba)->uc_ba_policy) == MAC_BA_POLICY_IMMEDIATE && (en_need_del_lut == OAL_TRUE)) {
        hmac_ba_del_lut_index(pst_mac_device->auc_rx_ba_lut_idx_table, (*ppst_rx_ba)->uc_lut_index);
    }

    /* Step4: dec the ba session cnt maitence in device struc */
#ifdef _PRE_WLAN_FEATURE_AMPDU_VAP
    hmac_rx_ba_session_decr(pst_hmac_vap, uc_tid);
#else
    hmac_rx_ba_session_decr(pst_mac_device, uc_tid);
#endif
    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user((*ppst_rx_ba)->st_alarm_data.us_mac_user_idx);
    if (OAL_UNLIKELY(pst_hmac_user == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* Step5: Del Timer */
    if (pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer.en_is_registerd == OAL_TRUE) {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer));
    }

    /* Step6: Free rx handle */
    OAL_MEM_FREE(*ppst_rx_ba, OAL_TRUE);
    *ppst_rx_ba = OAL_PTR_NULL;

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_BTCOEX
extern oal_bool_enum_uint8 g_en_btcoex_reject_addba;
#endif


oal_uint8 hmac_mgmt_check_set_rx_ba_ok(hmac_vap_stru *pst_hmac_vap,
                                       hmac_user_stru *pst_hmac_user,
                                       hmac_ba_rx_stru *pst_ba_rx_info,
                                       mac_device_stru *pst_device)
{
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    hmac_device_stru *pst_hmac_device;
#endif

    pst_ba_rx_info->uc_lut_index = DMAC_INVALID_BA_LUT_INDEX;

    /* 立即块确认判断 */
    if (pst_ba_rx_info->uc_ba_policy == MAC_BA_POLICY_IMMEDIATE) {
        if (pst_hmac_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.en_dot11ImmediateBlockAckOptionImplemented == OAL_FALSE) {
            /* 不支持立即块确认 */
            OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                             "{hmac_mgmt_check_set_rx_ba_ok::not support immediate Block Ack.}");
            return MAC_INVALID_REQ_PARAMS;
        } else {
            if (pst_ba_rx_info->en_back_var != MAC_BACK_COMPRESSED) {
                /* 不支持非压缩块确认 */
                OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                                 "{hmac_mgmt_check_set_rx_ba_ok::not support non-Compressed Block Ack.}");
                return MAC_REQ_DECLINED;
            }
        }
    } else if (pst_ba_rx_info->uc_ba_policy == MAC_BA_POLICY_DELAYED) {
        /* 延迟块确认不支持 */
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_check_set_rx_ba_ok::not support delayed Block Ack.}");
        return MAC_INVALID_REQ_PARAMS;
    }
#ifdef _PRE_WLAN_FEATURE_AMPDU_VAP
    if (pst_hmac_vap->uc_rx_ba_session_num > WLAN_MAX_RX_BA) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_check_set_rx_ba_ok::uc_rx_ba_session_num[%d] is up to max.}\r\n",
                         pst_hmac_vap->uc_rx_ba_session_num);
        return MAC_REQ_DECLINED;
    }
#else
    if (pst_device->uc_rx_ba_session_num > WLAN_MAX_RX_BA) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_check_set_rx_ba_ok::uc_rx_ba_session_num[%d] is up to max.}\r\n",
                         pst_device->uc_rx_ba_session_num);
        return MAC_REQ_DECLINED;
    }
#endif

    /* 获取BA LUT INDEX */
    pst_ba_rx_info->uc_lut_index = hmac_ba_get_lut_index(pst_device->auc_rx_ba_lut_idx_table, 0, HAL_MAX_BA_LUT_SIZE);

    /* LUT index表已满 */
    if (pst_ba_rx_info->uc_lut_index == DMAC_INVALID_BA_LUT_INDEX) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                       "{hmac_mgmt_check_set_rx_ba_ok::ba lut index table full.");
        return MAC_REQ_DECLINED;
    }

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    if (pst_hmac_user->st_hmac_user_btcoex.st_hmac_btcoex_addba_req.en_ba_handle_allow == OAL_FALSE) {
        return MAC_REQ_DECLINED;
    }

    if ((pst_hmac_user->st_hmac_user_btcoex.en_ba_type == BTCOEX_BA_TYPE_REJECT) ||
        (g_en_btcoex_reject_addba == OAL_TRUE)) {
        pst_hmac_device = hmac_res_get_mac_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
        if (pst_hmac_device == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                           "hmac_mgmt_check_set_rx_ba_ok:pst_hmac_device is null");
            return MAC_INVALID_REQ_PARAMS;
        }
        if (pst_hmac_device->st_hmac_device_btcoex.st_btble_status.un_bt_status.st_bt_status.bit_bt_on == OAL_TRUE) {
            return MAC_REQ_DECLINED;
        }
    }
#endif

    return MAC_SUCCESSFUL_STATUSCODE;
}


oal_void hmac_up_rx_bar(hmac_vap_stru *pst_hmac_vap, dmac_rx_ctl_stru *pst_rx_ctl, oal_netbuf_stru *pst_netbuf)
{
    oal_uint8                   *puc_payload;
    mac_ieee80211_frame_stru    *pst_frame_hdr;
    oal_uint8                   *puc_sa_addr;
    oal_uint8                    uc_tidno;
    hmac_user_stru              *pst_ta_user;
    oal_uint16                   us_start_seqnum;
    hmac_ba_rx_stru             *pst_ba_rx_info;

    pst_frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(pst_rx_ctl->st_rx_info));
    puc_sa_addr = pst_frame_hdr->auc_address2;

    /*  获取用户指针 */
    pst_ta_user = mac_vap_get_hmac_user_by_addr(&(pst_hmac_vap->st_vap_base_info), puc_sa_addr);
    if (pst_ta_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_up_rx_bar::pst_ta_user  is null.}");
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
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_up_rx_bar::uc_tidno is wrong! uc_tidno:%u}", uc_tidno);
        return;
    }

    us_start_seqnum = mac_get_bar_start_seq_num(puc_payload);

    pst_ba_rx_info = pst_ta_user->ast_tid_info[uc_tidno].pst_ba_rx_info;

    hmac_reorder_ba_rx_buffer_bar(pst_ba_rx_info, us_start_seqnum, &(pst_hmac_vap->st_vap_base_info));
}
#ifdef _PRE_WLAN_FEATURE_AMPDU_VAP
oal_bool_enum_uint8 hmac_is_device_ba_setup(void)
{
    oal_uint8 uc_vap_id;
    hmac_vap_stru *pst_hmac_vap;

    for (uc_vap_id = 0; uc_vap_id < WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT; uc_vap_id++) {
        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
        if (pst_hmac_vap == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_is_device_ba_setup pst_mac_vap is null.}");
            continue;
        }
        if ((pst_hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_UP) &&
            (pst_hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_PAUSE)) {
            continue;
        }
        if ((pst_hmac_vap->uc_tx_ba_session_num != 0) ||
            (pst_hmac_vap->uc_rx_ba_session_num != 0)) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}
#else
oal_bool_enum_uint8 hmac_is_device_ba_setup(void)
{
    mac_device_stru *pst_mac_device;

    pst_mac_device = mac_res_get_dev(0);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_is_device_ba_setup pst_mac_device is null.}");
        return OAL_FALSE;
    }
    if ((pst_mac_device->uc_tx_ba_session_num != 0) ||
        (pst_mac_device->uc_rx_ba_session_num != 0)) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}
#endif


oal_void hmac_ba_update_reorder_queue(hmac_ba_rx_stru *pst_ba_rx_hdl, mac_tcp_ack_record_stru *pst_tcp_ack_info)
{
    oal_uint64          ull_tmp_bit;
    oal_uint16          us_end_seq;
    oal_uint16          us_seq;
    oal_uint8           uc_bitmap_index;
    oal_uint8           uc_reorder_index;
    hmac_rx_buf_stru   *pst_reorder_buf;

    us_seq = pst_tcp_ack_info->us_start_seq_num;
    us_end_seq = HMAC_BA_SEQNO_ADD(pst_tcp_ack_info->us_end_seq_num, 1);

    while (us_seq != us_end_seq) {
        uc_bitmap_index = hmac_tcp_ack_bitmap_index_get(us_seq);
        uc_reorder_index = hmac_tcp_ack_reorder_index_get(us_seq);

        ull_tmp_bit = ((oal_uint64)1) << uc_reorder_index;
        pst_reorder_buf = &pst_ba_rx_hdl->ast_re_order_list[uc_reorder_index];

        /*
         * 对于重传帧, 可能上次上报的bitmap过滤, 这次不过滤
         * 若直接根据bitmap置0置1, 可能导致重传帧之前的过滤信息丢失
         * 因此直接仅在bit为1时才置位
         */
        if ((pst_tcp_ack_info->aull_ba_bitmap[uc_bitmap_index] & ull_tmp_bit) != 0) {
            pst_reorder_buf->en_tcp_ack_filtered[uc_bitmap_index] = OAL_TRUE;
        }

        us_seq = HMAC_BA_SEQNO_ADD(us_seq, 1);
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

