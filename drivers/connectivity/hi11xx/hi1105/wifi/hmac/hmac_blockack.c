

/* 1 头文件包含 */
#include "wlan_spec.h"
#include "mac_vap.h"
#include "hmac_blockack.h"
#include "hmac_main.h"
#include "hmac_rx_data.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_user.h"
#include "hmac_auto_adjust_freq.h"
#include "wlan_chip_i.h"
#include "mac_mib.h"
#include "hmac_config.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_BLOCKACK_C

const uint16_t g_ba_rx_timeout_len[WLAN_WME_AC_PSM + 2] = { // 2 enum num
    HMAC_BA_RX_BE_TIMEOUT,      /* WLAN_WME_AC_BE */
    HMAC_BA_RX_BK_TIMEOUT,      /* WLAN_WME_AC_BK */
    HMAC_BA_RX_VI_TIMEOUT,      /* WLAN_WME_AC_VI */
    HMAC_BA_RX_VO_TIMEOUT,      /* WLAN_WME_AC_VO */
    HMAC_BA_RX_DEFAULT_TIMEOUT, /* WLAN_WME_AC_SOUNDING */
    HMAC_BA_RX_DEFAULT_TIMEOUT, /* WLAN_WME_AC_MGMT */
    HMAC_BA_RX_DEFAULT_TIMEOUT, /* WLAN_WME_AC_MC */
};

OAL_STATIC uint8_t hmac_ba_timeout_param_is_valid(hmac_ba_alarm_stru *alarm_data,
    hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, uint8_t rx_tid)
{
    if (rx_tid >= WLAN_TID_MAX_NUM) {
        oam_error_log1(0, OAM_SF_BA, "{hmac_ba_timeout_fn::tid %d overflow.}", rx_tid);
        return OAL_FALSE;
    }

    if (hmac_vap == NULL) {
        oam_error_log1(0, 0, "{hmac_ba_timeout_fn::vap:%d}", alarm_data->uc_vap_id);
        return OAL_FALSE;
    }

    if (hmac_user == NULL) {
        oam_error_log2(0, 0, "{hmac_ba_timeout_fn::user:%d,user[%p]}", alarm_data->us_mac_user_idx, hmac_user);
        return OAL_FALSE;
    }

    return OAL_TRUE;
}


/* 2 全局变量定义 */
/* 3 函数实现 */

void hmac_reorder_ba_timer_start(hmac_vap_stru *pst_hmac_vap,
                                 hmac_user_stru *pst_hmac_user,
                                 uint8_t uc_tid)
{
    mac_vap_stru *pst_mac_vap = NULL;
    hmac_ba_rx_stru *pst_ba_rx_stru = NULL;
    mac_device_stru *pst_device = NULL;
    uint16_t us_timeout;

    /* 如果超时定时器已经被注册则返回 */
    if (pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer.en_is_registerd == OAL_TRUE) {
        return;
    }

    pst_mac_vap = &pst_hmac_vap->st_vap_base_info;

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_device == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                       "{hmac_reorder_ba_timer_start::pst_device[%d] null.}",
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

    frw_timer_create_timer_m(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer),
                             hmac_ba_timeout_fn,
                             us_timeout,
                             &(pst_ba_rx_stru->st_alarm_data),
                             OAL_FALSE,
                             OAM_MODULE_ID_HMAC,
                             pst_device->core_id);

    oal_spin_unlock(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer_lock));
}


void hmac_ba_rx_hdl_baw_init(hmac_user_stru *pst_hmac_user,
    hmac_ba_rx_stru *pst_ba_rx_stru, uint8_t uc_tid)
{
    mac_rx_buffer_size_stru *rx_buffer_size = NULL;
    rx_buffer_size = mac_vap_get_rx_buffer_size();

    /* 保存ba size实际大小,用于反馈addba rsp */
    pst_hmac_user->aus_tid_baw_size[uc_tid] = pst_ba_rx_stru->us_baw_size;

    if (pst_ba_rx_stru->us_baw_size <= 1) {
        pst_ba_rx_stru->us_baw_size = WLAN_AMPDU_RX_BUFFER_SIZE;
        /* 异常更新反馈值 */
        pst_hmac_user->aus_tid_baw_size[uc_tid] = WLAN_AMPDU_RX_BUFFER_SIZE;
    }

#if defined(_PRE_WLAN_FEATURE_11AX)
    /* HE设备并且BA窗大于64时, */
    if (g_wlan_spec_cfg->feature_11ax_is_open &&
        ((pst_ba_rx_stru->us_baw_size > WLAN_AMPDU_RX_BUFFER_SIZE) &&
        MAC_USER_IS_HE_USER(&pst_hmac_user->st_user_base_info))) {
        /* resp反馈值与addba req保持一致,但是BA BAW需要更新,避免兼容性问题,因此不更新user */
        pst_ba_rx_stru->us_baw_size = WLAN_AMPDU_RX_HE_BUFFER_SIZE;

        /* 定制化设定BA BAW, 则取小 */
        if (rx_buffer_size->en_rx_ampdu_bitmap_ini == OAL_TRUE) {
            pst_ba_rx_stru->us_baw_size =
                    oal_min(rx_buffer_size->us_rx_buffer_size, WLAN_AMPDU_RX_HE_BUFFER_SIZE);
        }

        /* 兼容性问题:如果决策的BAW size 小于对端BAW size,那么更新resp帧BAW size,否则按照对端的BAW size反馈 */
        pst_hmac_user->aus_tid_baw_size[uc_tid] =
                    oal_min(pst_hmac_user->aus_tid_baw_size[uc_tid], pst_ba_rx_stru->us_baw_size);
    } else {
#endif
        /* BA窗小于64时协商BA窗SIZE */
        pst_ba_rx_stru->us_baw_size = oal_min(pst_ba_rx_stru->us_baw_size, WLAN_AMPDU_RX_BUFFER_SIZE);
        /* 非HE设备更新,反馈值与baw同步更新 */
        pst_hmac_user->aus_tid_baw_size[uc_tid] = pst_ba_rx_stru->us_baw_size;
#if defined(_PRE_WLAN_FEATURE_11AX)
    }
#endif
    /* 配置优先级最高，手动配置接收聚合个数 */
    if (rx_buffer_size->en_rx_ampdu_bitmap_cmd == OAL_TRUE) {
        pst_ba_rx_stru->us_baw_size = rx_buffer_size->us_rx_buffer_size;
        pst_hmac_user->aus_tid_baw_size[uc_tid] = pst_ba_rx_stru->us_baw_size;
    }
}


void hmac_ba_rx_hdl_init(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user, uint8_t uc_tid)
{
    mac_vap_stru *pst_mac_vap = NULL;
    hmac_ba_rx_stru *pst_ba_rx_stru = NULL;
    hmac_rx_buf_stru *pst_rx_buff = NULL;
    uint16_t us_reorder_index;

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
    hmac_ba_rx_hdl_baw_init(pst_hmac_user, pst_ba_rx_stru, uc_tid);

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

/* hw reorder window init */
void hmac_ba_rx_win_init(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, uint8_t rx_tid)
{
    mac_vap_stru *mac_vap = NULL;
    hmac_ba_rx_stru *ba_rx_win = NULL;
    uint16_t index;

    mac_vap = &(hmac_vap->st_vap_base_info);
    ba_rx_win = hmac_user->ast_tid_info[rx_tid].pst_ba_rx_info;

    /* 初始化BAW */
    hmac_ba_rx_hdl_baw_init(hmac_user, ba_rx_win, rx_tid);

    /* 1. 初始化接收窗口， */
    /* 1.1 BA接收窗口参数(SSN与窗口大小)由管理面收到BA请求帧后获取并同步到数据面 */
    ba_rx_win->us_last_relseq = DMAC_BA_SEQNO_SUB(ba_rx_win->us_baw_start, 1);

    /* 1.2 其它窗口参数通过计算获取 */
    ba_rx_win->us_baw_end = DMAC_BA_SEQ_ADD(ba_rx_win->us_baw_start, (ba_rx_win->us_baw_size - 1));
    ba_rx_win->us_baw_tail = DMAC_BA_SEQNO_SUB(ba_rx_win->us_baw_start, 1);
    ba_rx_win->us_baw_head = DMAC_BA_SEQNO_SUB(ba_rx_win->us_baw_start, HMAC_BA_BMP_SIZE);

    /* 1.3 其它辅助参数进行初始化 */
    ba_rx_win->uc_mpdu_cnt = 0;

    oal_spin_lock_init(&ba_rx_win->st_ba_lock);
    oal_atomic_set(&ba_rx_win->ref_cnt, 1);
    ba_rx_win->en_timer_triggered = OAL_FALSE;
    ba_rx_win->en_ba_status = DMAC_BA_INIT;

    for (index = 0; index < WLAN_AMPDU_RX_HE_BUFFER_SIZE; index++) {
        ba_rx_win->ast_re_order_list[index].in_use = 0;
        ba_rx_win->ast_re_order_list[index].us_seq_num = 0;
        ba_rx_win->ast_re_order_list[index].uc_num_buf  = 0; //  可以删除  ?
        oal_netbuf_list_head_init(&(ba_rx_win->ast_re_order_list[index].st_netbuf_head));
    }

    ba_rx_win->en_back_var = MAC_BACK_COMPRESSED;
    ba_rx_win->puc_transmit_addr = hmac_user->st_user_base_info.auc_user_mac_addr;
    ba_rx_win->uc_mpdu_cnt = 0;
    ba_rx_win->en_is_ba = OAL_TRUE;  // Ba session is processing
    ba_rx_win->en_amsdu_supp = ((hmac_vap->bit_rx_ampduplusamsdu_active & hmac_vap->en_ps_rx_amsdu) ?
                                      OAL_TRUE : OAL_FALSE);
    /* 初始化定时器资源 */
    ba_rx_win->st_alarm_data.pst_ba = ba_rx_win;
    ba_rx_win->st_alarm_data.us_mac_user_idx = hmac_user->st_user_base_info.us_assoc_id;
    ba_rx_win->st_alarm_data.uc_vap_id = mac_vap->uc_vap_id;
    ba_rx_win->st_alarm_data.uc_tid = rx_tid;
    ba_rx_win->st_alarm_data.us_timeout_times = 0;
    ba_rx_win->st_alarm_data.en_direction = MAC_RECIPIENT_DELBA;
    ba_rx_win->en_timer_triggered = OAL_FALSE;

    memset_s(ba_rx_win->aul_rx_buf_bitmap, sizeof(ba_rx_win->aul_rx_buf_bitmap),
        0, sizeof(ba_rx_win->aul_rx_buf_bitmap));

    /* 1.4 状态置为初始状态 */
    ba_rx_win->en_ba_status = DMAC_BA_INIT;
}



OAL_STATIC hmac_rx_buf_stru *hmac_ba_buffer_frame_in_reorder(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                             uint16_t us_seq_num,
                                                             mac_rx_ctl_stru *pst_cb_ctrl)
{
    uint16_t us_buf_index;
    hmac_rx_buf_stru *pst_rx_buf;

    us_buf_index = (us_seq_num & (WLAN_AMPDU_RX_HE_BUFFER_SIZE - 1));
    pst_rx_buf = HMAC_GET_BA_RX_DHL(pst_ba_rx_hdl, us_buf_index);
    if (pst_rx_buf->in_use == 1) {
        hmac_rx_free_netbuf_list(&pst_rx_buf->st_netbuf_head, pst_rx_buf->uc_num_buf);
        pst_ba_rx_hdl->uc_mpdu_cnt--;
        pst_rx_buf->in_use = 0;
        pst_rx_buf->uc_num_buf = 0;
        oam_info_log1(0, OAM_SF_BA, "{hmac_ba_buffer_frame_in_reorder::slot already used, seq[%d].}", us_seq_num);
    }

    if (pst_cb_ctrl->bit_amsdu_enable == OAL_TRUE) {
        if (pst_cb_ctrl->bit_is_first_buffer == OAL_TRUE) {
            if (oal_netbuf_list_len(&pst_rx_buf->st_netbuf_head) != 0) {
                hmac_rx_free_netbuf_list(&pst_rx_buf->st_netbuf_head,
                                         oal_netbuf_list_len(&pst_rx_buf->st_netbuf_head));
                oam_info_log1(0, OAM_SF_BA,
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
    pst_rx_buf->rx_time = (uint32_t)oal_time_get_stamp_ms();

    return pst_rx_buf;
}

OAL_STATIC void hmac_ba_send_frames_add_list(hmac_rx_buf_stru *pst_rx_buf, oal_netbuf_stru *pst_netbuf,
    oal_netbuf_head_stru *pst_netbuf_header)
{
    uint8_t uc_loop_index;

    for (uc_loop_index = 0; uc_loop_index < pst_rx_buf->uc_num_buf; uc_loop_index++) {
        pst_netbuf = oal_netbuf_delist(&pst_rx_buf->st_netbuf_head);
        oal_mem_netbuf_trace(pst_netbuf, OAL_FALSE);
        if (pst_netbuf != NULL) {
            oal_netbuf_list_tail_nolock(pst_netbuf_header, pst_netbuf);
        }
    }
}


OAL_STATIC uint32_t hmac_ba_send_frames_with_gap(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                 oal_netbuf_head_stru *pst_netbuf_header,
                                                 uint16_t us_last_seqnum,
                                                 mac_vap_stru *pst_vap)
{
    uint8_t uc_num_frms = 0;
    uint16_t us_seq_num;
    hmac_rx_buf_stru *pst_rx_buf = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;

    us_seq_num = pst_ba_rx_hdl->us_baw_start;

    oam_info_log1(pst_vap->uc_vap_id, OAM_SF_BA, "{hmac_ba_send_frames_with_gap::to seq[%d].}", us_last_seqnum);

    while (us_seq_num != us_last_seqnum) {
        pst_rx_buf = hmac_remove_frame_from_reorder_q(pst_ba_rx_hdl, us_seq_num);
        if (pst_rx_buf != NULL) {
            pst_netbuf = oal_netbuf_peek(&pst_rx_buf->st_netbuf_head);
            if (oal_unlikely(pst_netbuf == NULL)) {
                oam_warning_log1(pst_vap->uc_vap_id, OAM_SF_BA,
                                 "{hmac_ba_send_frames_with_gap::gap[%d].\r\n}",
                                 us_seq_num);

                us_seq_num = DMAC_BA_SEQNO_ADD(us_seq_num, 1);
                pst_rx_buf->uc_num_buf = 0;

                continue;
            }
            hmac_ba_send_frames_add_list(pst_rx_buf, pst_netbuf, pst_netbuf_header);
            pst_rx_buf->uc_num_buf = 0;
            uc_num_frms++;
        }

        us_seq_num = DMAC_BA_SEQNO_ADD(us_seq_num, 1);
    }

    oam_warning_log4(pst_vap->uc_vap_id, OAM_SF_BA,
        "{hmac_ba_send_frames_with_gap::old_baw_start[%d], new_baw_start[%d], uc_num_frms[%d], uc_mpdu_cnt=%d.}",
        pst_ba_rx_hdl->us_baw_start, us_last_seqnum, uc_num_frms, pst_ba_rx_hdl->uc_mpdu_cnt);

    return uc_num_frms;
}


OAL_STATIC uint16_t hmac_ba_send_frames_in_order(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                 oal_netbuf_head_stru *pst_netbuf_header,
                                                 mac_vap_stru *pst_vap)
{
    uint16_t us_seq_num;
    hmac_rx_buf_stru *pst_rx_buf = NULL;
    uint8_t uc_loop_index;
    oal_netbuf_stru *pst_netbuf = NULL;

    us_seq_num = pst_ba_rx_hdl->us_baw_start;

    while ((pst_rx_buf = hmac_remove_frame_from_reorder_q(pst_ba_rx_hdl, us_seq_num)) != NULL) {
        us_seq_num = hmac_ba_seqno_add(us_seq_num, 1);
        pst_netbuf = oal_netbuf_peek(&pst_rx_buf->st_netbuf_head);
        if (pst_netbuf == NULL) {
            oam_warning_log1(pst_vap->uc_vap_id, OAM_SF_BA,
                             "{hmac_ba_send_frames_in_order::[%d] slot error.}",
                             us_seq_num);
            pst_rx_buf->uc_num_buf = 0;
            continue;
        }

        for (uc_loop_index = 0; uc_loop_index < pst_rx_buf->uc_num_buf; uc_loop_index++) {
            pst_netbuf = oal_netbuf_delist(&pst_rx_buf->st_netbuf_head);
            oal_mem_netbuf_trace(pst_netbuf, OAL_FALSE);
            if (pst_netbuf != NULL) {
                oal_netbuf_list_tail_nolock(pst_netbuf_header, pst_netbuf);
            }
        }

        pst_rx_buf->uc_num_buf = 0;
    }

    return us_seq_num;
}


OAL_STATIC OAL_INLINE void hmac_ba_buffer_rx_frame(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                   mac_rx_ctl_stru *pst_cb_ctrl,
                                                   oal_netbuf_head_stru *pst_netbuf_header,
                                                   uint16_t us_seq_num)
{
    hmac_rx_buf_stru *pst_rx_netbuf = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;
    uint8_t uc_netbuf_index;

    /* Get the pointer to the buffered packet */
    pst_rx_netbuf = hmac_ba_buffer_frame_in_reorder(pst_ba_rx_hdl, us_seq_num, pst_cb_ctrl);

    /* all buffers of this frame must be deleted from the buf list */
    for (uc_netbuf_index = pst_cb_ctrl->bit_buff_nums; uc_netbuf_index > 0; uc_netbuf_index--) {
        pst_netbuf = oal_netbuf_delist_nolock(pst_netbuf_header);

        oal_mem_netbuf_trace(pst_netbuf, OAL_TRUE);
        if (oal_unlikely(pst_netbuf != NULL)) {
            /* enqueue reorder queue */
            oal_netbuf_add_to_list_tail(pst_netbuf, &pst_rx_netbuf->st_netbuf_head);
        } else {
            oam_error_log0(pst_cb_ctrl->uc_mac_vap_id, OAM_SF_BA, "{hmac_ba_buffer_rx_frame:netbuff error in amsdu.}");
        }
    }

    if (oal_netbuf_list_len(&pst_rx_netbuf->st_netbuf_head) != pst_rx_netbuf->uc_num_buf) {
        oam_warning_log2(pst_cb_ctrl->uc_mac_vap_id, OAM_SF_BA, "{hmac_ba_buffer_rx_frame: list_len=%d numbuf=%d}",
                         oal_netbuf_list_len(&pst_rx_netbuf->st_netbuf_head), pst_rx_netbuf->uc_num_buf);
        pst_rx_netbuf->uc_num_buf = oal_netbuf_list_len(&pst_rx_netbuf->st_netbuf_head);
    }
}


OAL_STATIC OAL_INLINE void hmac_ba_reorder_rx_data(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                   oal_netbuf_head_stru *pst_netbuf_header,
                                                   mac_vap_stru *pst_vap,
                                                   uint16_t us_seq_num)
{
    uint8_t uc_seqnum_pos;
    uint16_t us_temp_winstart;

    uc_seqnum_pos = hmac_ba_seqno_bound_chk(pst_ba_rx_hdl->us_baw_start, pst_ba_rx_hdl->us_baw_end, us_seq_num);
    if (uc_seqnum_pos == DMAC_BA_BETWEEN_SEQLO_SEQHI) {
        pst_ba_rx_hdl->us_baw_start = hmac_ba_send_frames_in_order(pst_ba_rx_hdl, pst_netbuf_header, pst_vap);
        pst_ba_rx_hdl->us_baw_end = DMAC_BA_SEQNO_ADD(pst_ba_rx_hdl->us_baw_start, (pst_ba_rx_hdl->us_baw_size - 1));
    } else if (uc_seqnum_pos == DMAC_BA_GREATER_THAN_SEQHI) {
        us_temp_winstart = hmac_ba_seqno_sub(us_seq_num, (pst_ba_rx_hdl->us_baw_size - 1));

        hmac_ba_send_frames_with_gap(pst_ba_rx_hdl, pst_netbuf_header, us_temp_winstart, pst_vap);
        pst_ba_rx_hdl->us_baw_start = us_temp_winstart;
        pst_ba_rx_hdl->us_baw_start = hmac_ba_send_frames_in_order(pst_ba_rx_hdl, pst_netbuf_header, pst_vap);
        pst_ba_rx_hdl->us_baw_end = hmac_ba_seqno_add(pst_ba_rx_hdl->us_baw_start, (pst_ba_rx_hdl->us_baw_size - 1));
    } else {
        oam_warning_log3(pst_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_ba_reorder_rx_data::else branch seqno[%d] ws[%d] we[%d].}",
            us_seq_num, pst_ba_rx_hdl->us_baw_start, pst_ba_rx_hdl->us_baw_end);
    }
}


OAL_STATIC void hmac_ba_flush_reorder_q(hmac_ba_rx_stru *pst_rx_ba)
{
    hmac_rx_buf_stru *pst_rx_buf = NULL;
    uint16_t us_index;

    for (us_index = 0; us_index < WLAN_AMPDU_RX_HE_BUFFER_SIZE; us_index++) {
        pst_rx_buf = HMAC_GET_BA_RX_DHL(pst_rx_ba, us_index);
        if (pst_rx_buf->in_use == OAL_TRUE) {
            hmac_rx_free_netbuf_list(&pst_rx_buf->st_netbuf_head, pst_rx_buf->uc_num_buf);
            pst_rx_buf->in_use = OAL_FALSE;
            pst_rx_buf->uc_num_buf = 0;
            pst_rx_ba->uc_mpdu_cnt--;
        }
    }

    if (pst_rx_ba->uc_mpdu_cnt != 0) {
        oam_warning_log1(0, OAM_SF_BA, "{hmac_ba_flush_reorder_q:: %d mpdu cnt left.}", pst_rx_ba->uc_mpdu_cnt);
    }
}

OAL_STATIC OAL_INLINE uint32_t hmac_ba_check_rx_aggr(mac_vap_stru *pst_vap,
                                                     mac_ieee80211_frame_stru *pst_frame_hdr)
{
    /* 该vap是否是ht */
    if (OAL_FALSE == mac_mib_get_HighThroughputOptionImplemented(pst_vap)) {
        oam_info_log0(pst_vap->uc_vap_id, OAM_SF_BA, "{hmac_ba_check_rx_aggr::ht not supported by this vap.}");
        return OAL_FAIL;
    }

    /* 判断该帧是不是qos帧 */
    if ((WLAN_FC0_SUBTYPE_QOS | WLAN_FC0_TYPE_DATA) != ((uint8_t *)pst_frame_hdr)[0]) {
        oam_info_log0(pst_vap->uc_vap_id, OAM_SF_BA, "{hmac_ba_check_rx_aggr::not qos data.}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_ba_need_update_hw_baw(hmac_ba_rx_stru *pst_ba_rx_hdl,
                                                                     uint16_t us_seq_num)
{
    if ((OAL_TRUE == hmac_ba_seqno_lt(us_seq_num, pst_ba_rx_hdl->us_baw_start)) &&
        (OAL_FALSE == hmac_ba_rx_seqno_lt(us_seq_num, pst_ba_rx_hdl->us_baw_start))) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


uint32_t hmac_ba_filter_serv(hmac_user_stru *pst_hmac_user,
                             mac_rx_ctl_stru *pst_cb_ctrl,
                             oal_netbuf_head_stru *pst_netbuf_header,
                             oal_bool_enum_uint8 *pen_is_ba_buf)
{
    hmac_ba_rx_stru *pst_ba_rx_hdl = NULL;
    uint16_t us_seq_num;
    oal_bool_enum_uint8 en_is_4addr;
    uint8_t uc_is_tods;
    uint8_t uc_is_from_ds;
    uint8_t uc_tid;
    uint16_t us_baw_start_temp;
    uint32_t ret;
    mac_ieee80211_frame_stru *pst_frame_hdr = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    mac_vap_stru *pst_mac_vap = NULL;

    if (oal_any_null_ptr3(pst_netbuf_header, pst_cb_ctrl, pen_is_ba_buf)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_ba_filter_serv::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_frame_hdr = (mac_ieee80211_frame_stru *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_cb_ctrl);
    if (oal_unlikely(pst_frame_hdr == NULL)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_ba_filter_serv::pst_frame_hdr null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_cb_ctrl->uc_mac_vap_id);
    if (oal_unlikely(pst_hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_ba_filter_serv::pst_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &pst_hmac_vap->st_vap_base_info;

    ret = hmac_ba_check_rx_aggr(pst_mac_vap, pst_frame_hdr);
    if (ret != OAL_SUCC) {
        return OAL_SUCC;
    }

    /* 考虑四地址情况获取报文的tid */
    uc_is_tods = mac_hdr_get_to_ds((uint8_t *)pst_frame_hdr);
    uc_is_from_ds = mac_hdr_get_from_ds((uint8_t *)pst_frame_hdr);
    en_is_4addr = uc_is_tods && uc_is_from_ds;
    uc_tid = mac_get_tid_value((uint8_t *)pst_frame_hdr, en_is_4addr);

    pst_ba_rx_hdl = pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info;
    if (pst_ba_rx_hdl == NULL) {
        return OAL_SUCC;
    }
    if (pst_ba_rx_hdl->en_ba_status != DMAC_BA_COMPLETE) {
        oam_warning_log1(pst_cb_ctrl->uc_mac_vap_id, OAM_SF_BA,
                         "{hmac_ba_filter_serv::ba_status = %d.",
                         pst_ba_rx_hdl->en_ba_status);
        return OAL_SUCC;
    }

    /* 暂时保存BA窗口的序列号，用于鉴别是否有帧上报 */
    us_baw_start_temp = pst_ba_rx_hdl->us_baw_start;

    us_seq_num = mac_get_seq_num((uint8_t *)pst_frame_hdr);

    if ((oal_bool_enum_uint8)pst_frame_hdr->st_frame_control.bit_more_frag == OAL_TRUE) {
        oam_warning_log1(pst_cb_ctrl->uc_mac_vap_id, OAM_SF_BA,
                         "{hmac_ba_filter_serv::We get a frag_frame[seq_num=%d] When BA_session is set UP!",
                         us_seq_num);
        return OAL_SUCC;
    }

    /* duplicate frame判断 */
    if (OAL_TRUE == hmac_ba_rx_seqno_lt(us_seq_num, pst_ba_rx_hdl->us_baw_start)) {
        /* 上次非定时器上报，直接删除duplicate frame帧，否则，直接上报 */
        if (pst_ba_rx_hdl->en_timer_triggered == OAL_FALSE) {
            /* 确实已经收到该帧 */
            if (hmac_ba_isset(pst_ba_rx_hdl, us_seq_num)) {
                HMAC_USER_STATS_PKT_INCR(pst_hmac_user->rx_pkt_drop, 1);
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
        /* offload 下amsdu帧由于可能多个buffer组成，一律走重排序 */
        && (pst_cb_ctrl->bit_amsdu_enable == OAL_FALSE)
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
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                             "{hmac_ba_filter_serv::Sync loss and flush the reorder queue.}");
            hmac_ba_flush_reorder_q(pst_ba_rx_hdl);
        }

        /* 重排序队列刷新后,如果队列中有帧那么启动定时器 */
        if (pst_ba_rx_hdl->uc_mpdu_cnt > 0) {
            hmac_reorder_ba_timer_start(pst_hmac_vap, pst_hmac_user, uc_tid);
        }
    }

    if (us_baw_start_temp != pst_ba_rx_hdl->us_baw_start) {
        pst_ba_rx_hdl->en_timer_triggered = OAL_FALSE;
    }

    return OAL_SUCC;
}


void hmac_reorder_ba_rx_buffer_bar(hmac_ba_rx_stru *pst_rx_ba, uint16_t us_start_seq_num,
    mac_vap_stru *pst_vap)
{
    oal_netbuf_head_stru st_netbuf_head;
    uint8_t uc_seqnum_pos;

    if (pst_rx_ba == NULL) {
        oam_warning_log0(0, OAM_SF_BA, "{hmac_reorder_ba_rx_buffer_bar::rcv a bar, but ba not set up.}");
        return;
    }

    /* 针对 BAR 的SSN和窗口的start_num相等时，不需要移窗 */
    if (pst_rx_ba->us_baw_start == us_start_seq_num) {
        oam_info_log0(0, OAM_SF_BA, "{hmac_reorder_ba_rx_buffer_bar::seq is equal to start num.}");
        return;
    }

    oal_netbuf_list_head_init(&st_netbuf_head);

    uc_seqnum_pos = hmac_ba_seqno_bound_chk(pst_rx_ba->us_baw_start, pst_rx_ba->us_baw_end, us_start_seq_num);
    /* 针对BAR的的SSN在窗口内才移窗 */
    if (uc_seqnum_pos == DMAC_BA_BETWEEN_SEQLO_SEQHI) {
        hmac_ba_send_frames_with_gap(pst_rx_ba, &st_netbuf_head, us_start_seq_num, pst_vap);
        pst_rx_ba->us_baw_start = us_start_seq_num;
        pst_rx_ba->us_baw_start = hmac_ba_send_frames_in_order(pst_rx_ba, &st_netbuf_head, pst_vap);
        pst_rx_ba->us_baw_end = hmac_ba_seqno_add(pst_rx_ba->us_baw_start, (pst_rx_ba->us_baw_size - 1));

        oam_warning_log3(pst_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_reorder_ba_rx_buffer_bar::receive a bar, us_baw_start=%d us_baw_end=%d. us_seq_num=%d.}",
            pst_rx_ba->us_baw_start, pst_rx_ba->us_baw_end, us_start_seq_num);
        hmac_rx_lan_frame(&st_netbuf_head);
    } else if (uc_seqnum_pos == DMAC_BA_GREATER_THAN_SEQHI) {
        /* 异常 */
        oam_warning_log3(pst_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_reorder_ba_rx_buffer_bar::recv a bar and ssn out of winsize,baw_start=%d baw_end=%d seq_num=%d}",
            pst_rx_ba->us_baw_start, pst_rx_ba->us_baw_end, us_start_seq_num);
    } else {
        oam_warning_log3(pst_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_reorder_ba_rx_buffer_bar::recv a bar ssn less than baw_start,baw_start=%d end=%d seq_num=%d}",
            pst_rx_ba->us_baw_start, pst_rx_ba->us_baw_end, us_start_seq_num);
    }
}


OAL_STATIC uint32_t hmac_ba_rx_prepare_bufflist(hmac_vap_stru *pst_hmac_vap,
                                                hmac_rx_buf_stru *pst_rx_buf,
                                                oal_netbuf_head_stru *pst_netbuf_head)
{
    oal_netbuf_stru *pst_netbuf;
    uint8_t uc_loop_index;

    pst_netbuf = oal_netbuf_peek(&pst_rx_buf->st_netbuf_head);
    if (pst_netbuf == NULL) {
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_ba_rx_prepare_bufflist::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (uc_loop_index = 0; uc_loop_index < pst_rx_buf->uc_num_buf; uc_loop_index++) {
        pst_netbuf = oal_netbuf_delist(&pst_rx_buf->st_netbuf_head);
        if (pst_netbuf != NULL) {
            oal_netbuf_add_to_list_tail(pst_netbuf, pst_netbuf_head);
        } else {
            oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                             "{hmac_ba_rx_prepare_bufflist::uc_num_buf in reorder list is error.}");
        }
    }

    return OAL_SUCC;
}


uint32_t hmac_ba_send_reorder_timeout(hmac_ba_rx_stru *pst_rx_ba, hmac_vap_stru *pst_hmac_vap,
    hmac_ba_alarm_stru *pst_alarm_data, uint32_t *timeout)
{
    uint32_t time_diff, rx_timeout, ret;
    oal_netbuf_head_stru st_netbuf_head;
    uint16_t us_baw_head, us_baw_end, us_baw_start; /* 保存最初的窗口起始序列号 */
    hmac_rx_buf_stru *pst_rx_buf = NULL;
    uint8_t uc_buff_count = 0;
    uint16_t uc_send_count = 0;

    oal_netbuf_list_head_init(&st_netbuf_head);
    us_baw_head = pst_rx_ba->us_baw_start;
    us_baw_start = pst_rx_ba->us_baw_start;
    us_baw_end = hmac_ba_seqno_add(pst_rx_ba->us_baw_tail, 1);
    rx_timeout = (*timeout);

    oal_spin_lock(&pst_rx_ba->st_ba_lock);

    while (us_baw_head != us_baw_end) {
        pst_rx_buf = hmac_get_frame_from_reorder_q(pst_rx_ba, us_baw_head);
        if (pst_rx_buf == NULL) {
            uc_buff_count++;
            us_baw_head = hmac_ba_seqno_add(us_baw_head, 1);
            continue;
        }

        /* 如果冲排序队列中的帧滞留时间小于定时器超时时间,那么暂时不强制flush */
        time_diff = (uint32_t)oal_time_get_stamp_ms() - pst_rx_buf->rx_time;
        if (time_diff < rx_timeout) {
            *timeout = (rx_timeout - time_diff);
            break;
        }

        pst_rx_ba->uc_mpdu_cnt--;
        pst_rx_buf->in_use = 0;

        ret = hmac_ba_rx_prepare_bufflist(pst_hmac_vap, pst_rx_buf, &st_netbuf_head);
        if (ret != OAL_SUCC) {
            uc_buff_count++;
            us_baw_head = hmac_ba_seqno_add(us_baw_head, 1);
            continue;
        }

        uc_send_count++;
        uc_buff_count++;
        us_baw_head = hmac_ba_seqno_add(us_baw_head, 1);
        pst_rx_ba->us_baw_start = hmac_ba_seqno_add(pst_rx_ba->us_baw_start, uc_buff_count);
        pst_rx_ba->us_baw_end = hmac_ba_seqno_add(pst_rx_ba->us_baw_start, (pst_rx_ba->us_baw_size - 1));

        uc_buff_count = 0;
    }

    oal_spin_unlock(&pst_rx_ba->st_ba_lock);

    /* 判断本次定时器超时是否有帧上报 */
    if (us_baw_start != pst_rx_ba->us_baw_start) {
        pst_rx_ba->en_timer_triggered = OAL_TRUE;
        oam_warning_log4(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
            "{hmac_ba_send_reorder_timeout::new baw_start:%d, old baw_start:%d, rx_timeout:%d, send mpdu cnt:%d.}",
            pst_rx_ba->us_baw_start, us_baw_start, rx_timeout, uc_send_count);
    }

    hmac_rx_lan_frame(&st_netbuf_head);

    return OAL_SUCC;
}


uint32_t hmac_ba_timeout_fn(void *p_arg)
{
    hmac_ba_rx_stru *pst_rx_ba = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = NULL;
    hmac_ba_alarm_stru *alarm_data = NULL;
    mac_delba_initiator_enum_uint8 en_direction;
    uint8_t rx_tid;
    mac_device_stru *mac_device = NULL;
    uint32_t timeout;

    alarm_data = (hmac_ba_alarm_stru *)p_arg;
    en_direction = alarm_data->en_direction;
    rx_tid = alarm_data->uc_tid;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(alarm_data->uc_vap_id);
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(alarm_data->us_mac_user_idx);
    if (OAL_TRUE != hmac_ba_timeout_param_is_valid(alarm_data, hmac_vap, hmac_user, rx_tid)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (mac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (en_direction == MAC_RECIPIENT_DELBA) {
        pst_rx_ba = (hmac_ba_rx_stru *)alarm_data->pst_ba;
        if (pst_rx_ba == NULL) {
            oam_error_log0(0, OAM_SF_BA, "{hmac_ba_timeout_fn::pst_rx_ba is null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 接收业务量较少时只能靠超时定时器冲刷重排序队列,为改善游戏帧延时,需要将超时时间设小 */
        if (OAL_FALSE == hmac_wifi_rx_is_busy()) {
            timeout = hmac_vap->us_rx_timeout_min[WLAN_WME_TID_TO_AC(rx_tid)];
        } else {
            timeout = hmac_vap->us_rx_timeout[WLAN_WME_TID_TO_AC(rx_tid)];
        }

        if (pst_rx_ba->uc_mpdu_cnt > 0) {
            wlan_chip_ba_send_reorder_timeout(pst_rx_ba, hmac_vap, alarm_data, &timeout);
        }

        /* 若重排序队列刷新后,依然有缓存帧则需要重启定时器;
           若重排序队列无帧则为了节省功耗不启动定时器,在有帧入队时重启 */
        if (pst_rx_ba->uc_mpdu_cnt > 0) {
            oal_spin_lock(&(hmac_user->ast_tid_info[rx_tid].st_ba_timer_lock));
            /* 此处不需要判断定时器是否已经启动,如果未启动则启动定时器;
               如果此定时器已经启动 */
            frw_timer_create_timer_m(&(hmac_user->ast_tid_info[rx_tid].st_ba_timer),
                hmac_ba_timeout_fn, timeout, alarm_data,
                OAL_FALSE, OAM_MODULE_ID_HMAC, mac_device->core_id);
            oal_spin_unlock(&(hmac_user->ast_tid_info[rx_tid].st_ba_timer_lock));
        }
    } else {
        /* tx ba不删除 */
        frw_timer_create_timer_m(&(hmac_user->ast_tid_info[rx_tid].st_ba_timer),
            hmac_ba_timeout_fn, hmac_vap->us_rx_timeout[WLAN_WME_TID_TO_AC(rx_tid)],
            alarm_data, OAL_FALSE, OAM_MODULE_ID_HMAC, mac_device->core_id);
    }

    return OAL_SUCC;
}


uint32_t hmac_ba_reset_rx_handle(mac_device_stru *pst_mac_device,
                                 hmac_user_stru *pst_hmac_user, uint8_t uc_tid,
                                 oal_bool_enum_uint8 en_is_aging)
{
    hmac_vap_stru *pst_hmac_vap = NULL;
    mac_chip_stru *pst_mac_chip = NULL;
    oal_bool_enum en_need_del_lut = OAL_TRUE;
    hmac_ba_rx_stru *pst_ba_rx_info = pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info;

    if (oal_unlikely((pst_ba_rx_info == NULL) || (pst_ba_rx_info->en_is_ba != OAL_TRUE))) {
        oam_warning_log0(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle::rx ba not set yet.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (uc_tid >= WLAN_TID_MAX_NUM) {
        oam_error_log1(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle::tid %d overflow.}", uc_tid);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_ba_rx_info->st_alarm_data.uc_vap_id);
    if (oal_unlikely(pst_hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle::pst_hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* Step1: disable the flag of ba session */
    pst_ba_rx_info->en_is_ba = OAL_FALSE;

    /* Step2: flush reorder q */
    hmac_ba_flush_reorder_q(pst_ba_rx_info);

    if (pst_ba_rx_info->uc_lut_index == MAC_INVALID_RX_BA_LUT_INDEX) {
        en_need_del_lut = OAL_FALSE;
        oam_warning_log1(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle::no need to del lut index, lut index[%d]}\n",
                         pst_ba_rx_info->uc_lut_index);
    }

    /* Step3: if lut index is valid, del lut index alloc before */
    if ((pst_ba_rx_info->uc_ba_policy == MAC_BA_POLICY_IMMEDIATE) && (en_need_del_lut == OAL_TRUE)) {
        pst_mac_chip = hmac_res_get_mac_chip(pst_mac_device->uc_chip_id);
        if (pst_mac_chip == NULL) {
            return OAL_ERR_CODE_PTR_NULL;
        }
        hmac_ba_del_lut_index(pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_table, pst_ba_rx_info->uc_lut_index);
    }

    /* Step4: dec the ba session cnt maitence in device struc */
    hmac_rx_ba_session_decr(pst_hmac_vap, uc_tid);

    oal_spin_lock(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer_lock));
    /* Step5: Del Timer */
    if (pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer));
    }
    oal_spin_unlock(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer_lock));

    /* Step6: Free rx handle */
    oal_mem_free_m(pst_ba_rx_info, OAL_TRUE);

    pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info = NULL;

    return OAL_SUCC;
}

OAL_STATIC uint8_t hmac_mgmt_check_rx_ba_policy(hmac_vap_stru *pst_hmac_vap, hmac_ba_rx_stru *pst_ba_rx_info)
{
    /* 立即块确认判断 */
    if (pst_ba_rx_info->uc_ba_policy == MAC_BA_POLICY_IMMEDIATE) {
        if (OAL_FALSE == mac_mib_get_dot11ImmediateBlockAckOptionImplemented(&pst_hmac_vap->st_vap_base_info)) {
            /* 不支持立即块确认 */
            oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                             "{hmac_mgmt_check_set_rx_ba_ok::not support immediate Block Ack.}");
            return MAC_INVALID_REQ_PARAMS;
        } else {
            if (pst_ba_rx_info->en_back_var != MAC_BACK_COMPRESSED) {
                /* 不支持非压缩块确认 */
                oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                                 "{hmac_mgmt_check_set_rx_ba_ok::not support non-Compressed Block Ack.}");
                return MAC_REQ_DECLINED;
            }
        }
    } else if (pst_ba_rx_info->uc_ba_policy == MAC_BA_POLICY_DELAYED) {
        /* 延迟块确认不支持 */
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_check_set_rx_ba_ok::not support delayed Block Ack.}");
        return MAC_INVALID_REQ_PARAMS;
    }
    return MAC_SUCCESSFUL_STATUSCODE;
}


uint8_t hmac_mgmt_check_set_rx_ba_ok(hmac_vap_stru *pst_hmac_vap,
                                     hmac_user_stru *pst_hmac_user,
                                     hmac_ba_rx_stru *pst_ba_rx_info,
                                     mac_device_stru *pst_device,
                                     hmac_tid_stru *pst_tid_info)
{
    mac_chip_stru *pst_mac_chip = NULL;
    uint8_t ba_status;
    uint8_t ret;

    pst_ba_rx_info->uc_lut_index = MAC_INVALID_RX_BA_LUT_INDEX;
    ret = hmac_mgmt_check_rx_ba_policy(pst_hmac_vap, pst_ba_rx_info);
    if (ret != MAC_SUCCESSFUL_STATUSCODE) {
        return ret;
    }

    pst_mac_chip = hmac_res_get_mac_chip(pst_device->uc_chip_id);
    if (pst_mac_chip == NULL) {
        return MAC_REQ_DECLINED;
    }
    if (mac_mib_get_RxBASessionNumber(&pst_hmac_vap->st_vap_base_info) > g_wlan_spec_cfg->max_rx_ba) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_check_set_rx_ba_ok::uc_rx_ba_session_num[%d] is up to max.}\r\n",
                         mac_mib_get_RxBASessionNumber(&pst_hmac_vap->st_vap_base_info));
        return MAC_REQ_DECLINED;
    }

    /* 获取BA LUT INDEX */
    pst_ba_rx_info->uc_lut_index = hmac_ba_get_lut_index(pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_table,
                                                         0, HAL_MAX_RX_BA_LUT_SIZE);

    if ((pst_ba_rx_info->uc_lut_index == MAC_INVALID_RX_BA_LUT_INDEX) &&
        (wlan_chip_ba_need_check_lut_idx() == OAL_TRUE)) {
        oam_error_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, 0, "{hmac_mgmt_check_set_rx_ba_ok:ba lut idx tb full");
        return MAC_REQ_DECLINED;
    }
    ba_status = hmac_btcoex_check_addba_req(pst_hmac_vap, pst_hmac_user);
    if (ba_status != MAC_SUCCESSFUL_STATUSCODE) {
        return ba_status;
    }
    /* 该tid下不允许建BA，配置命令需求 */
    if (pst_tid_info->en_ba_handle_rx_enable == OAL_FALSE) {
        oam_warning_log2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_check_set_rx_ba_ok::uc_tid_no[%d] user_idx[%d] is not available.}",
                         pst_tid_info->uc_tid_no, pst_tid_info->us_hmac_user_idx);
        return MAC_REQ_DECLINED;
    }
    return MAC_SUCCESSFUL_STATUSCODE;
}


void hmac_up_rx_bar(hmac_vap_stru *pst_hmac_vap, dmac_rx_ctl_stru *pst_rx_ctl, oal_netbuf_stru *pst_netbuf)
{
    uint8_t *puc_payload = NULL;
    mac_ieee80211_frame_stru *pst_frame_hdr = NULL;
    uint8_t *puc_sa_addr = NULL;
    uint8_t uc_tidno;
    hmac_user_stru *pst_ta_user = NULL;
    uint16_t us_start_seqnum;
    hmac_ba_rx_stru *pst_ba_rx_info = NULL;
    mac_rx_ctl_stru *pst_rx_info = NULL;
    uint16_t us_frame_len;

    if (wlan_chip_check_need_process_bar() == OAL_FALSE) {
        oam_info_log0(0, OAM_SF_BA, "{hmac_up_rx_bar: not need process bar, return.}");
        return;
    }

    pst_frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(pst_rx_ctl->st_rx_info));
    puc_sa_addr = pst_frame_hdr->auc_address2;
    pst_rx_info = (mac_rx_ctl_stru *)(&(pst_rx_ctl->st_rx_info));
    us_frame_len = pst_rx_info->us_frame_len - pst_rx_info->uc_mac_header_len;

    /*  获取用户指针 */
    pst_ta_user = mac_vap_get_hmac_user_by_addr(&(pst_hmac_vap->st_vap_base_info), puc_sa_addr);
    if (pst_ta_user == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_up_rx_bar::pst_ta_user  is null.}");
        return;
    }

    /* BAR Control(2) +  BlockAck Starting seq num)(2) */
    if (us_frame_len < 4) {  /* mac header帧头长度小于4，帧格式错误 */
        oam_warning_log2(0, OAM_SF_ANY, "{hmac_up_rx_bar:frame len err. frame len[%d], machdr len[%d].}",
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
    uc_tidno = (puc_payload[1] & 0xF0) >> 4; /* 取4~7bit位为TID编号类别 */
    if (uc_tidno >= WLAN_TID_MAX_NUM) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_up_rx_bar::uc_tidno[%d] invalid.}", uc_tidno);
        return;
    }
    us_start_seqnum = mac_get_bar_start_seq_num(puc_payload);
    pst_ba_rx_info = pst_ta_user->ast_tid_info[uc_tidno].pst_ba_rx_info;

    hmac_reorder_ba_rx_buffer_bar(pst_ba_rx_info, us_start_seqnum, &(pst_hmac_vap->st_vap_base_info));
}

oal_bool_enum_uint8 hmac_is_device_ba_setup(void)
{
    uint8_t uc_vap_id;
    hmac_vap_stru *pst_hmac_vap = NULL;

    for (uc_vap_id = 0; uc_vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_vap_id++) {
        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
        if (pst_hmac_vap == NULL) {
            oam_error_log0(0, OAM_SF_ANY, "{hmac_is_device_ba_setup pst_mac_vap is null.}");
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


uint32_t hmac_ps_rx_delba(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    mac_cfg_delba_req_param_stru st_mac_cfg_delba_param;
    dmac_to_hmac_ps_rx_delba_event_stru *pst_dmac_to_hmac_ps_rx_delba = NULL;
    uint32_t ret;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (oal_unlikely(pst_hmac_vap == NULL)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                       "{hmac_ps_rx_delba::pst_hmac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_to_hmac_ps_rx_delba = (dmac_to_hmac_ps_rx_delba_event_stru *)puc_param;

    pst_hmac_user = mac_res_get_hmac_user(pst_dmac_to_hmac_ps_rx_delba->us_user_id);
    if (oal_unlikely(pst_hmac_user == NULL)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                         "{hmac_ps_rx_delba::pst_hmac_user is null! user_id is %d.}",
                         pst_dmac_to_hmac_ps_rx_delba->us_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 刷新HMAC VAP下PS业务时RX AMSDU使能标志 */
    pst_mac_vap->en_ps_rx_amsdu = pst_dmac_to_hmac_ps_rx_delba->en_rx_amsdu;
    pst_mac_vap->uc_ps_type |= pst_dmac_to_hmac_ps_rx_delba->uc_ps_type;
    /* 保证命令优先级最高 */
    if (pst_mac_vap->uc_ps_type & MAC_PS_TYPE_CMD) {
        return OAL_SUCC;
    }
    pst_hmac_vap->en_ps_rx_amsdu = pst_mac_vap->en_ps_rx_amsdu;
    memset_s((uint8_t *)&st_mac_cfg_delba_param, sizeof(st_mac_cfg_delba_param),
             0, sizeof(st_mac_cfg_delba_param));
    oal_set_mac_addr(st_mac_cfg_delba_param.auc_mac_addr, pst_hmac_user->st_user_base_info.auc_user_mac_addr);
    st_mac_cfg_delba_param.en_direction = MAC_RECIPIENT_DELBA;
    st_mac_cfg_delba_param.en_trigger = MAC_DELBA_TRIGGER_PS;

    for (st_mac_cfg_delba_param.uc_tidno = 0;
         st_mac_cfg_delba_param.uc_tidno < WLAN_TID_MAX_NUM; st_mac_cfg_delba_param.uc_tidno++) {
        ret = hmac_config_delba_req(pst_mac_vap, 0, (uint8_t *)&st_mac_cfg_delba_param);
        if (ret != OAL_SUCC) {
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                "{hmac_ps_rx_delba::delba send failed, ret: %d, tid: %d}",
                ret, st_mac_cfg_delba_param.uc_tidno);
            return ret;
        }
    }
    return OAL_SUCC;
}


void hmac_ba_rx_update_release_seqnum(hmac_ba_rx_stru *ba_rx_hdl, uint16_t release_sn)
{
    if (oal_unlikely((ba_rx_hdl == NULL) || (release_sn > MAX_BA_SN))) {
        oam_error_log1(0, OAM_SF_RX, "{hmac_ba_rx_update_release_seqnum::release_sn[%d].}", release_sn);
        return;
    }

    if ((hmac_baw_rightside(ba_rx_hdl->us_last_relseq, release_sn)) || (ba_rx_hdl->us_last_relseq == release_sn)) {
        ba_rx_hdl->us_last_relseq = release_sn;
        return;
    } else {
        oam_warning_log0(0, OAM_SF_RX, "{hmac_ba_rx_update_release_seqnum::sw req update wrong release sn.}");
        return;
    }
}


OAL_STATIC uint32_t hmac_ba_rx_prepare_buffer(hmac_vap_stru *hmac_vap,
    hmac_rx_buf_stru *rx_entry, oal_netbuf_head_stru *pnetbuf_head)
{
    oal_netbuf_stru    *netbuf = NULL;
    mac_rx_ctl_stru    *rx_ctl = NULL;

    if (oal_any_null_ptr2(hmac_vap, pnetbuf_head)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_ba_rx_prepare_buffer::input params null.}");
        return OAL_FAIL;
    }

    if (rx_entry == NULL) {
        oam_warning_log0(0, OAM_SF_BA, "{hmac_ba_rx_prepare_buffer::rx_reo_entry null.}");
        return OAL_FAIL;
    }

    if (oal_netbuf_list_empty(&rx_entry->st_netbuf_head)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_ba_rx_prepare_buffer::netbuf_head is empty.}");
        return OAL_FAIL;
    }

    /* 按顺序串链缓存帧 */
    while (!oal_netbuf_list_empty(&(rx_entry->st_netbuf_head))) {
        netbuf = oal_netbuf_delist(&(rx_entry->st_netbuf_head));
        if (netbuf == NULL) {
            oam_error_log0(0, OAM_SF_RX, "{hmac_ba_rx_prepare_bufflist::netbuf null.}");
            return OAL_FAIL;
        }

        rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        rx_ctl->bit_is_reo_timeout = OAL_TRUE;
        oal_netbuf_add_to_list_tail(netbuf, pnetbuf_head);
    }

    return OAL_SUCC;
}


uint32_t  hmac_ba_check_rx_aggrate(hmac_vap_stru *hmac_vap, mac_rx_ctl_stru *rx_ctl)
{
    uint8_t    frame_type;

    if (oal_unlikely(oal_any_null_ptr2(hmac_vap, rx_ctl))) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_ba_check_rx_aggr::input params null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    frame_type = mac_get_frame_type_and_subtype((uint8_t *)&rx_ctl->us_frame_control);
    /* 判断该帧是不是qos帧 */
    if (frame_type != (WLAN_FC0_SUBTYPE_QOS | WLAN_FC0_TYPE_DATA)) {
        oam_info_log0(0, OAM_SF_BA, "{hmac_ba_check_rx_aggr::not qos data.}");
        return OAL_FAIL;
    }
    /* 对于STA还需要判断是否是广播帧，考虑兼容某些设备广播帧带qos域 */
    if ((hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (rx_ctl->bit_mcast_bcast == OAL_TRUE)) {
        oam_info_log0(0, OAM_SF_BA, "{hmac_ba_check_rx_aggr::This is a multicast data frame.}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


void hmac_ba_rx_hdl_hold(hmac_ba_rx_stru *ba_rx_hdl)
{
    oal_atomic_inc(&ba_rx_hdl->ref_cnt);
}


void hmac_ba_rx_hdl_put(hmac_ba_rx_stru *ba_rx_hdl)
{
    oal_atomic_dec(&ba_rx_hdl->ref_cnt);
}

OAL_STATIC uint32_t hmac_rx_frm_diff_time(hmac_rx_buf_stru *rx_buf, uint32_t curr_time)
{
    uint32_t time_diff;

    if (curr_time >= rx_buf->rx_time) {
        time_diff = curr_time - rx_buf->rx_time;
    } else {
        time_diff = (HMAC_U32_MAX - rx_buf->rx_time) + curr_time;
    }

    return time_diff;
}

OAL_STATIC uint16_t hmac_rx_ba_get_baw_head(hmac_ba_rx_stru *rx_ba, uint16_t baw_head)
{
    return (hmac_baw_rightside(baw_head, hmac_ba_seqno_add(rx_ba->us_last_relseq, 1))) ? baw_head :
        hmac_ba_seqno_add(rx_ba->us_last_relseq, 1);
}


uint32_t hmac_ba_send_ring_reorder_timeout(hmac_ba_rx_stru *rx_ba,
    hmac_vap_stru *hmac_vap, hmac_ba_alarm_stru *alarm_data, uint32_t *p_timeout)
{
    uint32_t             curr_time, time_diff, rx_timeout;
    uint16_t             baw_head, baw_start, baw_end;
    uint32_t             ret;
    oal_netbuf_head_stru netbuf_head;
    hmac_rx_buf_stru    *rx_buf = NULL;

    oal_netbuf_list_head_init(&netbuf_head);
    baw_head = rx_ba->us_baw_start;
    baw_start = rx_ba->us_baw_start;
    rx_timeout = g_ba_rx_timeout_len[WLAN_WME_TID_TO_AC(alarm_data->uc_tid)];
    baw_end = hmac_ba_seqno_add(rx_ba->us_baw_tail, 1);

    if (baw_head != hmac_ba_seqno_add(rx_ba->us_last_relseq, 1)) {
        /* baw_start eq. us_last_relseq + 1 从较靠左侧的开始判断超时 */
        oam_warning_log2(0, 0, "{hmac_ba_send_reo:baw_start[%d]!=last_relseq[%d]+1}", baw_head, rx_ba->us_last_relseq);
        baw_head = hmac_rx_ba_get_baw_head(rx_ba, baw_head);
    }

    curr_time = (uint32_t)oal_time_get_stamp_ms();
    while (baw_head != baw_end) {
        rx_buf = hmac_get_frame_from_reorder_q(rx_ba, baw_head);
        if (rx_buf == NULL) {
            baw_head = hmac_ba_seqno_add(baw_head, 1);
            continue;
        }

        /* 获取当前时间和收到报文时的差值，差值如果大于定时器timer，则release报文 */
        time_diff = hmac_rx_frm_diff_time(rx_buf, curr_time);
        if (time_diff < rx_timeout) {
            *p_timeout = rx_timeout - time_diff;
            break;
        }
        rx_buf = hmac_remove_frame_from_reorder_q(rx_ba, baw_head);
        ret = hmac_ba_rx_prepare_buffer(hmac_vap, rx_buf, &netbuf_head);

        /* timeout上报中无需check软件是否需要主动上报缓存帧，因为对于rx_ba->us_baw_start之前的缓存帧一定会在此 */
        /* baw_start handler的地方检查了，此处只需更新last release sequence number */
        hmac_ba_rx_update_release_seqnum(rx_ba, baw_head);
        if (ret != OAL_SUCC) {
            baw_head = hmac_ba_seqno_add(baw_head, 1);
            continue;
        }
        baw_head = hmac_ba_seqno_add(baw_head, 1);
    }
    rx_ba->us_baw_start = baw_head;

    if (baw_start != rx_ba->us_baw_start) {
        rx_ba->en_timer_triggered = OAL_TRUE;
    }
    /* To kernel or to wlan */
    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_rx_data_sta_proc(&netbuf_head);
    }
    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_rx_data_ap_proc(hmac_vap, &netbuf_head);
    }
    return OAL_SUCC;
}


OAL_STATIC hmac_rx_buf_stru *hmac_ba_buffer_frame_in_reodr(hmac_ba_rx_stru *ba_rx_hdl,
    uint16_t sn, oal_netbuf_head_stru *pnetbuf_head)
{
    uint16_t        buf_index;
    hmac_rx_buf_stru *rx_buf = NULL;

    buf_index = (sn & (WLAN_AMPDU_RX_HE_BUFFER_SIZE - 1));
    rx_buf = &(ba_rx_hdl->ast_re_order_list[buf_index]);
    if (rx_buf == NULL) {
        return NULL;
    }

    if (rx_buf->in_use) {
        hmac_rx_free_netbuf_list(&rx_buf->st_netbuf_head, rx_buf->uc_num_buf);
        ba_rx_hdl->uc_mpdu_cnt--;
        rx_buf->in_use = 0;
        rx_buf->uc_num_buf = 0;
        oam_info_log1(0, OAM_SF_BA, "{hmac_ba_buffer_frame_in_reorder::slot already used, seq[%d].}", sn);
    }

    ba_rx_hdl->uc_mpdu_cnt++;
    rx_buf->in_use = OAL_TRUE;

    return rx_buf;
}


OAL_STATIC void hmac_ba_add_netbuf_to_reo_list(hmac_ba_rx_stru *ba_rx_hdl,
    oal_netbuf_stru *netbuf_mpdu, oal_netbuf_head_stru *netbuf_head)
{
    mac_rx_ctl_stru  *rx_ctl = NULL;
    hmac_rx_buf_stru *rx_reo_entry = NULL;

    if (oal_any_null_ptr2(ba_rx_hdl, netbuf_mpdu)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_ba_add_netbuf_to_reo_list::input params null.}");
        return;
    }

    rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf_mpdu);
    rx_reo_entry = hmac_ba_buffer_frame_in_reodr(ba_rx_hdl, MAC_GET_RX_CB_SEQ_NUM(rx_ctl), netbuf_head);
    if (rx_reo_entry == NULL) {
        return;
    }

    /*
    * 在报文缓存到接收缓存队列的时候，队列中每个元素包含的是skb结构；
    * 如果是amsdu，则会把所有msdu以skb的形式串链
    **/
    rx_reo_entry->us_seq_num = MAC_GET_RX_CB_SEQ_NUM(rx_ctl);
    rx_reo_entry->rx_time = (uint32_t)oal_time_get_stamp_ms();
    oal_netbuf_list_head_init(&(rx_reo_entry->st_netbuf_head));
    hmac_rx_mpdu_to_netbuf_list(&(rx_reo_entry->st_netbuf_head), netbuf_mpdu);
    rx_reo_entry->uc_num_buf = oal_netbuf_list_len(&(rx_reo_entry->st_netbuf_head));
    if (OAL_TRUE == hmac_ba_seqno_lt(ba_rx_hdl->us_baw_tail, MAC_GET_RX_CB_SEQ_NUM(rx_ctl))) {
        ba_rx_hdl->us_baw_tail = MAC_GET_RX_CB_SEQ_NUM(rx_ctl);
    }
    return;
}


uint32_t hmac_ba_rx_reo_timer_restart(hmac_user_stru *hmac_user,
    hmac_ba_rx_stru *ba_rx_hdl, oal_netbuf_stru *netbuf_mpdu)
{
    uint8_t          rx_tid;
    uint32_t         rx_timeout;
    hmac_vap_stru   *hmac_vap = NULL;
    mac_rx_ctl_stru *rx_ctl = NULL;
    mac_device_stru *mac_device = NULL;

    if (oal_any_null_ptr3(hmac_user, ba_rx_hdl, netbuf_mpdu)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_ba_rx_reo_timer_restart::invalid param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf_mpdu);
    rx_tid = MAC_GET_RX_CB_TID(rx_ctl);

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(rx_ctl->uc_mac_vap_id);
    if (oal_unlikely(oal_any_null_ptr1(hmac_vap))) {
        oam_error_log1(0, OAM_SF_RX, "{hmac_ba_rx_reo_timer_restart::hmac_vap NULL.}", rx_ctl->uc_mac_vap_id);
        return OAL_FAIL;
    }

    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (mac_device == NULL) {
        oam_error_log1(0, OAM_SF_BA, "{hmac_reorder_ba_timer_start::mac dev[%d] null.}",
            hmac_vap->st_vap_base_info.uc_device_id);
        return OAL_FAIL;
    }

    rx_timeout = g_ba_rx_timeout_len[WLAN_WME_TID_TO_AC(rx_tid)];
    if (hmac_user->ast_tid_info[rx_tid].st_ba_timer.en_is_registerd == OAL_TRUE) {
        return OAL_SUCC;
    } else {
        /* 获取device结构 */
        oal_spin_lock(&(hmac_user->ast_tid_info[rx_tid].st_ba_timer_lock));
        frw_timer_create_timer_m(&(hmac_user->ast_tid_info[rx_tid].st_ba_timer),
                                 hmac_ba_timeout_fn,
                                 g_ba_rx_timeout_len[WLAN_WME_TID_TO_AC(rx_tid)],
                                 &(ba_rx_hdl->st_alarm_data),
                                 OAL_FALSE,
                                 OAM_MODULE_ID_HMAC,
                                 mac_device->core_id);
        oal_spin_unlock(&(hmac_user->ast_tid_info[rx_tid].st_ba_timer_lock));
    }
    return OAL_SUCC;
}
void hmac_ba_rx_buffered_data_add_list(hmac_rx_buf_stru *rx_reo_entry, oal_netbuf_stru *netbuf,
    oal_netbuf_head_stru *netbuf_head)
{
    netbuf = oal_netbuf_delist(&(rx_reo_entry->st_netbuf_head));
    if (netbuf != NULL) {
        oal_netbuf_add_to_list_tail(netbuf, netbuf_head);
    }
}


void hmac_rx_reorder_release_to_sn(hmac_ba_rx_stru *ba_rx_hdl,
    uint16_t sn, oal_netbuf_head_stru *rpt_list)
{
    uint16_t          start_sn;
    hmac_rx_buf_stru *rx_reo_entry = NULL;

    /* 从us_last_relseq + 1 一直上报到check_sequence_number -1，因为check_sequence_number走自己的逻辑上报 */
    start_sn = DMAC_BA_SEQNO_ADD(ba_rx_hdl->us_last_relseq, 1);
    while (start_sn != sn) {
        rx_reo_entry = hmac_remove_frame_from_reorder_q(ba_rx_hdl, start_sn);
        if (rx_reo_entry != NULL) {
            oal_netbuf_queue_splice_tail_init(&rx_reo_entry->st_netbuf_head, rpt_list);
            rx_reo_entry->uc_num_buf = 0;
        }
        start_sn = DMAC_BA_SEQNO_ADD(start_sn, 1);
    }
    hmac_ba_rx_update_release_seqnum(ba_rx_hdl, (uint16_t)DMAC_BA_SEQNO_SUB(start_sn, 1));
}


uint32_t hmac_rx_ba_release_check_sn(hmac_ba_rx_stru *ba_rx_hdl,
    uint16_t sn, mac_rx_ctl_stru *rx_ctl, oal_netbuf_head_stru *rpt_list)
{
    if (!(hmac_baw_rightside(ba_rx_hdl->us_last_relseq, sn))) {
        oam_info_log2(0, OAM_SF_RX, "{hmac_ba_rx_buffered_data_check::input err check sn lrelease[%d] check[%d]}",
            ba_rx_hdl->us_last_relseq, sn);
        oam_info_log2(0, OAM_SF_RX, "{hmac_ba_rx_buffered_data_check::RELEASE_VALID[%d] PROCESS_FLAG[%d]}",
            MAC_GET_RX_CB_REL_IS_VALID(rx_ctl), MAC_GET_RX_CB_PROC_FLAG(rx_ctl));
        oam_info_log4(0, OAM_SF_RX, "{hmac_ba_rx_buffered_data_check::SSN[%d] SN[%d] REL_START[%d] REL_END[%d].}",
            MAC_GET_RX_CB_SSN(rx_ctl), MAC_GET_RX_CB_SEQ_NUM(rx_ctl),
            MAC_GET_RX_CB_REL_START_SEQNUM(rx_ctl), MAC_GET_RX_CB_REL_END_SEQNUM(rx_ctl));
        return OAL_FAIL;
    }

    hmac_rx_reorder_release_to_sn(ba_rx_hdl, sn, rpt_list);
    return OAL_SUCC;
}


OAL_INLINE uint32_t hmac_rx_release_valid_check(hmac_ba_rx_stru *ba_rx_hdl,
    uint16_t release_start_sn, uint16_t release_end_sn)
{
    if (oal_unlikely(release_start_sn > MAX_BA_SN)) {
        oam_error_log1(0, OAM_SF_RX, "{hmac_rx_release_valid_check::invalid param release_start_sn[%p].}",
            release_start_sn);
        return OAL_FAIL;
    }
    if (!(hmac_baw_rightside(release_start_sn, release_end_sn)) &&
        (release_start_sn != release_end_sn)) {
        oam_warning_log2(0, OAM_SF_RX, "{hmac_rx_release_valid_check:report wrong realese sn,start[%d] end[%d]}",
            release_start_sn, release_end_sn);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


void hmac_rx_ba_handle_release_flag(hmac_host_rx_context_stru *rx_context,
    hmac_ba_rx_stru *ba_rx_hdl, oal_netbuf_head_stru *rpt_list)
{
    uint8_t           tid;
    uint16_t          release_end_sn;
    uint16_t          release_start_sn;
    uint32_t          rel_valid_check;
    mac_rx_ctl_stru  *rx_ctl = rx_context->cb;
    uint16_t          start_sn;
    hmac_rx_buf_stru *rx_reo_entry = NULL;

    if (MAC_GET_RX_CB_REL_IS_VALID(rx_ctl) == 0) {
        return;
    }
    /* 如果缓存帧需要处理，则根据描述符中的信息把缓存帧放入pst_netbuf_head链表中 */
    tid = MAC_GET_RX_CB_TID(rx_ctl);
    release_start_sn = MAC_GET_RX_CB_REL_START_SEQNUM(rx_ctl);
    release_end_sn = MAC_GET_RX_CB_REL_END_SEQNUM(rx_ctl);
    rel_valid_check = hmac_rx_release_valid_check(ba_rx_hdl, release_start_sn, release_end_sn);
    if (rel_valid_check != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_RX, "{hmac_rx_ba_handle_release_flag::release valid check failed.}");
        /* 直接返回，不做后续的释放操作 */
        return;
    }

    if (!(hmac_baw_rightside(ba_rx_hdl->us_last_relseq, release_end_sn))) {
        oam_info_log2(0, OAM_SF_RX, "{hmac_rx_ba_handle_release_flag::input err check sn lrelease[%d] check[%d]}",
            ba_rx_hdl->us_last_relseq, release_end_sn);
        oam_info_log2(0, OAM_SF_RX, "{hmac_rx_ba_handle_release_flag::RELEASE_VALID[%d] PROCESS_FLAG[%d]}",
            MAC_GET_RX_CB_REL_IS_VALID(rx_ctl), MAC_GET_RX_CB_PROC_FLAG(rx_ctl));
        oam_info_log4(0, OAM_SF_RX, "{hmac_rx_ba_handle_release_flag::SSN[%d] SN[%d] REL_START[%d] REL_END[%d].}",
            MAC_GET_RX_CB_SSN(rx_ctl), MAC_GET_RX_CB_SEQ_NUM(rx_ctl),
            MAC_GET_RX_CB_REL_START_SEQNUM(rx_ctl), MAC_GET_RX_CB_REL_END_SEQNUM(rx_ctl));
        return;
    }

    start_sn = DMAC_BA_SEQNO_ADD(ba_rx_hdl->us_last_relseq, 1);
    do {
        rx_reo_entry = hmac_remove_frame_from_reorder_q(ba_rx_hdl, start_sn);
        if (rx_reo_entry != NULL) {
            oal_netbuf_queue_splice_tail_init(&rx_reo_entry->st_netbuf_head, rpt_list);
            rx_reo_entry->uc_num_buf = 0;
        }
        start_sn = DMAC_BA_SEQNO_ADD(start_sn, 1);
    } while (start_sn != release_end_sn);

    hmac_ba_rx_update_release_seqnum(ba_rx_hdl, (uint16_t)release_end_sn);
    return;
}


uint32_t hmac_ba_rx_data_release(hmac_host_rx_context_stru *rx_context,
    hmac_ba_rx_stru *ba_rx_hdl, oal_netbuf_head_stru *rpt_list, uint8_t *buff_is_valid)
{
    oal_netbuf_stru *netbuf = rx_context->netbuf;
    mac_rx_ctl_stru *rx_ctl = rx_context->cb;

    *buff_is_valid = OAL_FALSE;
    if (hmac_rx_ba_release_check_sn(ba_rx_hdl, MAC_GET_RX_CB_SEQ_NUM(rx_ctl), rx_ctl, rpt_list) != OAL_SUCC) {
        return OAL_FAIL;
    }
    hmac_rx_mpdu_to_netbuf_list(rpt_list, netbuf);
    hmac_ba_rx_update_release_seqnum(ba_rx_hdl, (uint16_t)MAC_GET_RX_CB_SEQ_NUM(rx_ctl));
    return OAL_SUCC;
}


uint32_t hmac_ba_rx_data_buffer(hmac_host_rx_context_stru *rx_context,
    hmac_ba_rx_stru *ba_rx_hdl, oal_netbuf_head_stru *rpt_list, uint8_t *buff_is_valid)
{
    oal_netbuf_stru *netbuf = rx_context->netbuf;

    *buff_is_valid = OAL_FALSE;
    hmac_ba_add_netbuf_to_reo_list(ba_rx_hdl, netbuf, rpt_list);
    return OAL_SUCC;
}
