

/* 1 头文件包含 */
#include "hmac_host_rx.h"
#include "mac_data.h"
#include "hmac_frag.h"
#include "hmac_11i.h"
#include "hmac_blockack.h"
#include "host_hal_ext_if.h"
#include "hmac_stat.h"
#include "hmac_wapi.h"
#ifdef _PRE_WLAN_FEATURE_MONITOR
#include "hmac_monitor.h"
#endif


#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_HOST_RX_C

OAL_STATIC uint32_t hmac_rx_proc_80211_to_eth(oal_netbuf_stru *netbuf,
    mac_rx_ctl_stru *rx_ctrl, mac_ieee80211_frame_stru *frame_hdr)
{
    dmac_msdu_stru rx_msdu;
    uint8_t *frame_addr = NULL;

    memset_s(&rx_msdu, sizeof(dmac_msdu_stru), 0, sizeof(dmac_msdu_stru));
    mac_get_transmit_addr(frame_hdr, &frame_addr);
    oal_set_mac_addr(rx_msdu.auc_ta, frame_addr);

    /* 对当前的msdu进行赋值 */
    rx_msdu.pst_netbuf = netbuf;

    /* 获取源地址和目的地址 */
    mac_rx_get_sa(frame_hdr, &frame_addr);
    oal_set_mac_addr(rx_msdu.auc_sa, frame_addr);
    mac_rx_get_da(frame_hdr, &frame_addr);
    oal_set_mac_addr(rx_msdu.auc_da, frame_addr);

    /* 将netbuf的data指针指向mac frame的payload处 */
    oal_netbuf_pull(netbuf, rx_ctrl->uc_mac_header_len);
    return hmac_rx_frame_80211_to_eth(netbuf, rx_msdu.auc_da, rx_msdu.auc_sa);
}


OAL_STATIC void hmac_rx_statistics(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf)
{
    hmac_stat_vap_rx_netbuf(hmac_vap, netbuf);
    hmac_stat_device_rx_netbuf(oal_netbuf_len(netbuf));
    return;
}

OAL_STATIC oal_bool_enum_uint8 hmac_rx_qos_data_need_drop(hmac_host_rx_context_stru *rx_context)
{
    mac_rx_ctl_stru     *rx_ctl = rx_context->cb;
    hmac_user_stru      *hmac_user = rx_context->hmac_user;
    hmac_ba_rx_stru     *ba_rx_hdl = NULL;
    uint16_t             seq_frag_num;
    uint8_t              rx_tid;

    if (!hmac_rx_is_qos_data_frame(rx_ctl)) {
        return OAL_FALSE;
    }
    rx_tid = MAC_GET_RX_CB_TID(rx_ctl);
    if (!mac_tid_is_valid(rx_tid)) {
        oam_warning_log1(0, OAM_SF_RX, "{hmac_rx_qos_data_need_drop:: tid invalid[%d]}", rx_tid);
        return OAL_FALSE;
    }

    /* qos 已建ba返回 */
    ba_rx_hdl = hmac_user->ast_tid_info[rx_tid].pst_ba_rx_info;
    if (ba_rx_hdl != NULL && (ba_rx_hdl->en_ba_status == DMAC_BA_COMPLETE)) {
        return OAL_FALSE;
    }

    if (rx_ctl->bit_is_fragmented) {
        seq_frag_num = ((uint16_t)(rx_ctl->us_seq_num) << 4) | rx_ctl->bit_fragment_num; /* 合并sn放高4位 */
    } else {
        seq_frag_num = rx_ctl->us_seq_num;
    }

    /* no ba qos data，retry 且seq num相同则过滤，否则更新 */
    if (((mac_header_frame_control_stru *)&rx_ctl->us_frame_control)->bit_retry == OAL_TRUE &&
        (seq_frag_num == hmac_user->qos_last_seq_frag_num[rx_tid])) {
        oam_warning_log2(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_RX,
            "{hmac_rx_qos_data_need_drop::tid is[%d],qos duplicate frame[%d].}", rx_tid, seq_frag_num);
        return OAL_TRUE;
    }

    hmac_user->qos_last_seq_frag_num[rx_tid] = seq_frag_num;
    return OAL_FALSE;
}
/* 功能描述 : 检查上报帧的vap是否有效 */
static uint32_t hmac_is_hal_vap_valid(hal_host_device_stru *hal_device, mac_rx_ctl_stru *rx_ctl)
{
    uint8_t   hal_vap_id;

    if (MAC_RXCB_IS_AMSDU_SUB_MSDU(rx_ctl)) {
        return OAL_SUCC;
    }

    hal_vap_id = MAC_GET_RX_CB_HAL_VAP_IDX(rx_ctl);
    if (oal_unlikely(hal_vap_id >= HAL_MAX_VAP_NUM) && (hal_vap_id != g_wlan_spec_cfg->other_bss_id)) {
        oam_error_log1(0, OAM_SF_RX, "{hmac_is_hal_vap_valid::invalid hal vap id[%d].}", hal_vap_id);
        return OAL_FAIL;
    }

    if ((hal_vap_id < HAL_MAX_VAP_NUM) && (!hal_device->hal_vap_sts_info[hal_vap_id].hal_vap_valid)) {
        oam_warning_log2(0, OAM_SF_RX, "{hmac_is_hal_vap_valid::haldev[%d] invalid hal vap[%d]. }",
            hal_device->device_id, hal_vap_id);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}
/* 功能描述 : 检查上报帧的tid是否有效 */
OAL_STATIC uint32_t hmac_is_tid_valid(hal_host_device_stru *hal_device, mac_rx_ctl_stru *rx_ctl)
{
    uint8_t frame_type;

    if (MAC_RXCB_IS_AMSDU_SUB_MSDU(rx_ctl)) {
        return OAL_SUCC;
    }

    /* 判断该帧是不是qos帧 */
    frame_type = mac_get_frame_type_and_subtype((uint8_t *)&rx_ctl->us_frame_control);
    if (frame_type != (WLAN_FC0_SUBTYPE_QOS | WLAN_FC0_TYPE_DATA)) {
        return OAL_SUCC;
    }

    if (!mac_tid_is_valid(MAC_GET_RX_CB_TID(rx_ctl))) {
        oam_error_log1(0, OAM_SF_RX, "{hmac_is_tid_valid::invalid tid[%d].}", MAC_GET_RX_CB_TID(rx_ctl));
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
static void hmac_alrx_process(hal_host_device_stru *hal_device, mac_rx_ctl_stru *rx_ctl)
{
    if (rx_ctl->rx_status != HAL_RX_SUCCESS) {
        hal_device->st_alrx.rx_ppdu_fail_num++;
    } else if (rx_ctl->bit_is_ampdu == OAL_TRUE) {
        hal_device->st_alrx.rx_ampdu_succ_num++;
    } else {
        hal_device->st_alrx.rx_normal_mdpu_succ_num++;
    }
    return;
}
static uint8_t hmac_rx_da_is_in_same_bss(oal_netbuf_stru *netbuf, mac_rx_ctl_stru *rx_ctl,
    hmac_vap_stru *hmac_vap)
{
    hmac_user_stru *hmac_da_user = NULL;
    uint16_t user_dix = 0;
    uint8_t *puc_da = hmac_rx_get_eth_da(netbuf);

    if (mac_vap_find_user_by_macaddr(&hmac_vap->st_vap_base_info, puc_da, &user_dix) != OAL_SUCC) {
        oam_info_log0(0, 0, "{hmac_rx_da_is_in_same_bss::find da fail}");
        return OAL_FALSE;
    }

    hmac_da_user = (hmac_user_stru *)mac_res_get_hmac_user(user_dix);
    if (hmac_da_user == NULL) {
        oam_error_log1(0, 0, "{hmac_rx_da_is_in_same_bss::find da fail usr idx:%d}", user_dix);
        return OAL_FALSE;
    }

    if (hmac_da_user->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_ASSOC) {
        hmac_mgmt_send_deauth_frame(&hmac_vap->st_vap_base_info,
            hmac_da_user->st_user_base_info.auc_user_mac_addr, MAC_NOT_AUTHED, OAL_FALSE);
        return OAL_FALSE;
    }

    MAC_GET_RX_CB_DA_USER_IDX(rx_ctl) = hmac_da_user->st_user_base_info.us_assoc_id;
    return OAL_TRUE;
}


void hmac_rx_ap_w2w_proc(hmac_host_rx_context_stru *rx_context,
    oal_netbuf_head_stru *release_list, oal_netbuf_head_stru *w2w_list)
{
    oal_netbuf_stru          *netbuf = NULL;
    mac_rx_ctl_stru          *rx_ctl = NULL;
    mac_ieee80211_frame_stru *copy_frame_hdr = NULL;
    oal_netbuf_stru          *netbuf_copy = NULL;
    hmac_vap_stru            *hmac_vap = rx_context->hmac_vap;
    oal_netbuf_stru          *next_netbuf = NULL;
    uint8_t                  *frame_ctl = NULL;

    oal_netbuf_search_for_each_safe(netbuf, next_netbuf, release_list) {
        rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        frame_ctl = (uint8_t *)&rx_ctl->us_frame_control;
        if (mac_is_4addr(frame_ctl) && rx_ctl->bit_amsdu_enable) {
            continue;
        }
        if (ether_is_multicast(hmac_rx_get_eth_da(netbuf)) == OAL_TRUE) {
            /* copy_frame_hdr复用03接口，06不用 */
            if (hmac_rx_copy_netbuff(&netbuf_copy, netbuf, rx_ctl->uc_mac_vap_id, &copy_frame_hdr) != OAL_SUCC) {
                continue;
            }
            hmac_rx_prepare_to_wlan(w2w_list, netbuf_copy);
        } else if (hmac_rx_da_is_in_same_bss(netbuf, rx_ctl, hmac_vap) == OAL_TRUE) {
            /* 目的用户已在AP的用户表中，进行WLAN_TO_WLAN转发 */
            oal_netbuf_delete(netbuf, release_list);
            hmac_rx_prepare_to_wlan(w2w_list, netbuf);
        } else {
            /* 该帧呆在链表里面 等待to lan */
            oam_info_log1(0, OAM_SF_RX, "{hmac_rx_host_ring_ap_proc::dst_is_valid[%d]}", rx_ctl->bit_dst_is_valid);
            oam_info_log4(0, OAM_SF_RX, "{hmac_rx_host_ring_ap_proc::band[%d] dst[%d] vap[%d]dst[%d]}",
                rx_ctl->bit_band_id, rx_ctl->bit_dst_band_id, rx_ctl->bit_vap_id, rx_ctl->dst_hal_vap_id);
        }
    }
}


uint32_t hmac_rx_proc_reorder(hmac_host_rx_context_stru *rx_context,
    oal_netbuf_head_stru *rpt_list, uint8_t *buff_is_valid)
{
    hmac_vap_stru   *hmac_vap = rx_context->hmac_vap;
    hmac_user_stru  *hmac_user = rx_context->hmac_user;
    mac_rx_ctl_stru *rx_ctl = rx_context->cb;
    uint32_t         ret;
    hmac_ba_rx_stru *ba_rx_hdl = NULL;

    if ((hmac_user == NULL) || (buff_is_valid == OAL_FALSE)) {
        return OAL_SUCC;
    }
    if (hmac_ba_check_rx_aggrate(hmac_vap, rx_ctl) != OAL_SUCC) {
        return OAL_SUCC;
    }
    ba_rx_hdl = hmac_user->ast_tid_info[MAC_GET_RX_CB_TID(rx_ctl)].pst_ba_rx_info;
    if (ba_rx_hdl == NULL) {
        return OAL_SUCC;
    }
    hmac_ba_rx_hdl_hold(ba_rx_hdl);
    if (ba_rx_hdl->en_ba_status != DMAC_BA_COMPLETE) {
        hmac_ba_rx_hdl_put(ba_rx_hdl);
        return OAL_SUCC;
    }
    ret = hmac_rx_data_wnd_proc(rx_context, ba_rx_hdl, rpt_list, buff_is_valid);
    if (ret != OAL_SUCC) {
        hmac_ba_rx_hdl_put(ba_rx_hdl);
        return ret;
    }
    hmac_ba_rx_hdl_put(ba_rx_hdl);
    return OAL_SUCC;
}


oal_bool_enum_uint8 hmac_rx_eapol_while_sta_just_up(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    mac_ether_header_stru *ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(netbuf);

    if (hmac_vap->st_vap_base_info.en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return OAL_FALSE;
    }

    if (hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_UP &&
        hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_STA_WAIT_ASOC) {
        oam_warning_log0(0, OAM_SF_RX, "{hmac_rx_ring_entry_proc:hmac_vap is not up}");
        return OAL_FALSE;
    }

    if (oal_byteorder_host_to_net_uint16(ETHER_TYPE_PAE) != ether_hdr->us_ether_type)  {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}
/* Rxcb中amsdu非首帧的信息填充 */
void hmac_rx_update_submsdu_rxctl(mac_rx_ctl_stru *dst_rxcb, mac_rx_ctl_stru *src_rxcb)
{
    MAC_GET_RX_CB_TID(dst_rxcb) = MAC_GET_RX_CB_TID(src_rxcb);
    MAC_GET_RX_CB_HAL_VAP_IDX(dst_rxcb) = MAC_GET_RX_CB_HAL_VAP_IDX(src_rxcb);
    MAC_RXCB_IS_AMSDU(dst_rxcb) = MAC_RXCB_IS_AMSDU(src_rxcb);
}

static uint32_t hmac_host_rx_mpdu_build_cb(hal_host_device_stru *hal_dev, oal_netbuf_stru *netbuf)
{
    mac_rx_ctl_stru *rxctl = NULL;
    uint32_t         amsdu_flag = OAL_FALSE;
    mac_rx_ctl_stru  st_rx_ctl;
    oal_netbuf_stru *next = NULL;

    memset_s(&st_rx_ctl, sizeof(mac_rx_ctl_stru), 0, sizeof(mac_rx_ctl_stru));
    while (netbuf != NULL) {
        next = oal_netbuf_next(netbuf);
        rxctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        if (hal_host_rx_get_msdu_info_dscr(hal_dev, netbuf, rxctl) != OAL_SUCC) {
            return OAL_FAIL;
        }
        /* AMSDU专有处理流程 */
        if (MAC_RXCB_IS_FIRST_AMSDU(rxctl) == OAL_TRUE) {
            memcpy_s(&st_rx_ctl, sizeof(mac_rx_ctl_stru), rxctl, sizeof(mac_rx_ctl_stru));
            amsdu_flag = OAL_TRUE;
        } else if (amsdu_flag == OAL_TRUE) {
            hmac_rx_update_submsdu_rxctl(rxctl, &st_rx_ctl);
        }
        netbuf = next;
    }
    return OAL_SUCC;
}


oal_netbuf_stru *hmac_rx_defrag(hmac_host_rx_context_stru *rx_context)
{
    hmac_user_stru *hmac_user = rx_context->hmac_user;
    oal_netbuf_stru *netbuf = rx_context->netbuf;
    mac_rx_ctl_stru *rx_ctl = rx_context->cb;

    if ((rx_ctl->bit_amsdu_enable == OAL_TRUE) ||
        (rx_ctl->bit_frame_format != MAC_FRAME_TYPE_80211) ||
        rx_context->hmac_user == NULL) {
        return netbuf;
    }
    return hmac_defrag_process(hmac_user, netbuf, rx_ctl->uc_mac_header_len);
}


static hmac_rx_frame_ctrl_enum hmac_host_rx_filter_encrypt(hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf, mac_rx_ctl_stru *rx_ctl)
{
    mac_header_frame_control_stru *frame_ctl = NULL;

    if (hmac_user == NULL) {
        return HMAC_RX_FRAME_CTRL_GOON;
    }

    frame_ctl = (mac_header_frame_control_stru *)(&rx_ctl->us_frame_control);
    /* 加密场景接收到非加密的数据帧，则过滤；null帧、QOS null帧、eapol帧无需过滤 */
    if ((hmac_user->st_user_base_info.st_key_info.en_cipher_type != WLAN_80211_CIPHER_SUITE_NO_ENCRYP) &&
        (rx_ctl->bit_cipher_type == WLAN_80211_CIPHER_SUITE_NO_ENCRYP) &&
        (frame_ctl->bit_type == WLAN_DATA_BASICTYPE)) {
        uint8_t uc_data_type = mac_get_data_type_from_8023(oal_netbuf_data(netbuf), MAC_NETBUFF_PAYLOAD_ETH);
        if ((frame_ctl->bit_sub_type != WLAN_NULL_FRAME) &&
            (frame_ctl->bit_sub_type != WLAN_QOS_NULL_FRAME) &&
            (uc_data_type != MAC_DATA_EAPOL) && (uc_data_type != MAC_DATA_DHCP) &&
            (uc_data_type != MAC_DATA_DHCPV6)) {
            return HMAC_RX_FRAME_CTRL_DROP;
        }
    }
    return HMAC_RX_FRAME_CTRL_GOON;
}


hmac_rx_frame_ctrl_enum hmac_host_rx_pre_abnormal_filter(hmac_host_rx_context_stru *rx_context)
{
    uint8_t                  rx_status;
    hal_host_device_stru    *hal_dev = rx_context->hal_dev;
    oal_netbuf_stru         *netbuf = rx_context->netbuf;
    mac_rx_ctl_stru         *rx_ctl = rx_context->cb;
    hmac_vap_stru           *hmac_vap = rx_context->hmac_vap;
    hmac_vap_stat_stru      *hmac_vap_stat = NULL;

    if (hmac_vap == NULL) {
        return HMAC_RX_FRAME_CTRL_DROP;
    }

    hmac_vap_stat = hmac_stat_get_vap_stat(hmac_vap);

    if (hal_dev->st_alrx.en_al_rx_flag) {
#ifdef _PRE_WLAN_FEATURE_MONITOR
        if (hmac_host_rx_sniffer_is_on(hal_dev) != OAL_TRUE)
#endif
        {
            hmac_alrx_process(hal_dev, rx_ctl);
            HMAC_VAP_DFT_STATS_PKT_INCR(hmac_vap_stat->rx_dropped_misc, 1);
            return HMAC_RX_FRAME_CTRL_DROP;
        }
    }
    rx_status = rx_ctl->rx_status;
    if (MAC_RX_DSCR_NEED_DROP(rx_status)) {
        if (rx_status == HAL_RX_NEW) {
            oam_error_log1(0, OAM_SF_RX, "hmac_host_rx_pre_abnormal_filter::rx_status:[%d]", rx_status);
            oam_report_dscr(BROADCAST_MACADDR, (uint8_t *)oal_netbuf_data(netbuf), HAL_RX_DSCR_LEN,
                OAM_OTA_TYPE_RX_MSDU_DSCR);
        }
        HMAC_VAP_DFT_STATS_PKT_INCR(hmac_vap_stat->rx_dropped_misc, 1);
        return HMAC_RX_FRAME_CTRL_DROP;
    }
    if ((hmac_is_hal_vap_valid(hal_dev, rx_ctl) != OAL_SUCC) ||
        (hmac_is_tid_valid(hal_dev, rx_ctl)) != OAL_SUCC) {
        HMAC_VAP_DFT_STATS_PKT_INCR(hmac_vap_stat->rx_dropped_misc, 1);
        return HMAC_RX_FRAME_CTRL_DROP;
    }
    /* 加密收到非加密的数据帧需要过滤处理,只处理首帧 */
    if (hmac_host_rx_filter_encrypt(rx_context->hmac_user, netbuf, rx_ctl) == HMAC_RX_FRAME_CTRL_DROP) {
        return HMAC_RX_FRAME_CTRL_DROP;
    }

    return HMAC_RX_FRAME_CTRL_GOON;
}


OAL_STATIC hmac_user_stru *hmac_ap_rx_recheck_user_idx(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf, mac_rx_ctl_stru *rx_ctl)
{
    mac_ieee80211_frame_stru *frmhdr = NULL;
    uint8_t *transmit_addr = NULL;
    hmac_user_stru *hmac_user = NULL;
    uint8_t usr_lut = rx_ctl->bit_ta_user_idx;
    uint16_t user_idx;

    /* lut为无效值且为80211帧时，才进行查找,此为异常保护流程 */
    if ((usr_lut < g_wlan_spec_cfg->invalid_user_id) ||
        (rx_ctl->bit_frame_format != MAC_FRAME_TYPE_80211)) {
        return NULL;
    }
    frmhdr = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf);
    transmit_addr = frmhdr->auc_address2;
    if (mac_vap_find_user_by_macaddr(&(hmac_vap->st_vap_base_info), transmit_addr, &user_idx) == OAL_SUCC) {
        hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(user_idx);
    }
    return hmac_user;
}


OAL_INLINE hmac_user_stru *hmac_host_rx_lut_get_user(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
                                                     hal_host_device_stru *hal_dev, uint8_t usr_lut)
{
    hmac_user_stru *hmac_user = NULL;

    if ((usr_lut < HAL_MAX_LUT) && (hal_dev->user_sts_info[usr_lut].user_valid)) {
        hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(hal_dev->user_sts_info[usr_lut].user_id);
    }
    if (oal_unlikely(hmac_user == NULL)) {
        oam_warning_log1(hal_dev->device_id, OAM_SF_ANY,
                         "hmac_host_rx_lut_get_user:get user fail, lut idx[%d]", usr_lut);
        if (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_ROAMING) {
            hmac_rx_report_eth_frame(&hmac_vap->st_vap_base_info, netbuf);
        }
    }
    return hmac_user;
}


OAL_INLINE void hmac_host_rx_update_cb_user_idx(hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf)
{
    mac_rx_ctl_stru *rx_ctl = NULL;
    uint16_t user_idx;

    if (hmac_user == NULL) {
        return;
    }

    user_idx = hmac_user->st_user_base_info.us_assoc_id;

    for (; netbuf; netbuf = oal_netbuf_next(netbuf)) {
        rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        rx_ctl->bit_ta_user_idx = user_idx;
    }
}


OAL_INLINE void hmac_host_build_rx_context(hal_host_device_stru *hal_dev,
    oal_netbuf_stru *netbuf, hmac_host_rx_context_stru *rx_context)
{
    mac_rx_ctl_stru *cb = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(cb->uc_mac_vap_id);
    uint8_t usr_lut = cb->bit_ta_user_idx;
    hmac_user_stru *hmac_user = NULL;
    if (hmac_vap == NULL) {
        oam_error_log0(cb->uc_mac_vap_id, OAM_SF_ANY, "hmac_host_build_rx_context:hmac_vap null");
        return;
    }
    rx_context->hal_dev = hal_dev;
    rx_context->hmac_vap = hmac_vap;
    hmac_user = hmac_host_rx_lut_get_user(hmac_vap, netbuf, hal_dev, usr_lut);
    if ((hmac_user == NULL) && (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP)) {
        hmac_user = hmac_ap_rx_recheck_user_idx(hmac_vap, netbuf, cb);
    }
    hmac_host_rx_update_cb_user_idx(hmac_user, netbuf);
    rx_context->hmac_user = hmac_user;
    rx_context->netbuf = netbuf;
    rx_context->cb = cb;
}


void hmac_host_rx_pre_static_proc(hmac_host_rx_context_stru *rx_context)
{
}


uint32_t hmac_host_rx_pre_proc_feature(hmac_host_rx_context_stru *rx_context)
{
    return OAL_SUCC;
}


uint32_t hmac_host_rx_proc_dev_routine(hal_host_device_stru *hal_dev,
    oal_netbuf_stru *netbuf, hmac_host_rx_context_stru *rx_context)
{
    hal_host_rx_amsdu_list_build(hal_dev, netbuf);
    if (hal_host_rx_proc_msdu_dscr(hal_dev, netbuf) != OAL_SUCC) {
        hmac_rx_netbuf_list_free(netbuf);
        return OAL_FAIL;
    }
    if (hmac_host_rx_mpdu_build_cb(hal_dev, netbuf) != OAL_SUCC) {
        hmac_rx_netbuf_list_free(netbuf);
        return OAL_FAIL;
    }
    hmac_host_build_rx_context(hal_dev, netbuf, rx_context);
    if (hmac_host_rx_pre_abnormal_filter(rx_context) != HMAC_RX_FRAME_CTRL_GOON) {
        hmac_rx_netbuf_list_free(netbuf);
        return OAL_FAIL;
    }
    hmac_host_rx_pre_static_proc(rx_context);
    if (hmac_host_rx_pre_proc_feature(rx_context) != OAL_SUCC) {
        hmac_rx_netbuf_list_free(netbuf);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


void hmac_host_rx_static_proc(hmac_host_rx_context_stru *rx_context)
{
    hmac_vap_stru *hmac_vap = rx_context->hmac_vap;
    oal_netbuf_stru *netbuf = rx_context->netbuf;
    mac_rx_ctl_stru *rx_ctl = rx_context->cb;

    for (;netbuf != NULL; netbuf = oal_netbuf_next(netbuf)) {
        /* 此统计位置需要调整 */
        hmac_rx_statistics(hmac_vap, rx_context->hmac_user, netbuf);
    }

    if (rx_ctl->bit_mcast_bcast == OAL_FALSE) {
        oal_atomic_inc(&(hmac_vap->st_hmac_arp_probe.rx_unicast_pkt_to_lan));
    }
}


#define ETHER_HEADER_ETH_TYPE_OFFSET 12
#define SNAP_HEADER_ETH_TYPE_OFFSET 6
OAL_INLINE uint16_t hmac_rx_netbuf_get_ether_type(oal_netbuf_stru *netbuf,
    mac_rx_ctl_stru *mac_rx_ctl)
{
    uint16_t frame_formate;
    uint8_t *header = NULL;
    uint16_t ether_type = 0;

    frame_formate = mac_rx_ctl->bit_frame_format;
    header = oal_netbuf_data(netbuf);
    if (frame_formate == MAC_FRAME_TYPE_80211) {
        ether_type = *(uint16_t *)(header + SNAP_HEADER_ETH_TYPE_OFFSET);
    } else if (frame_formate == MAC_FRAME_TYPE_RTH) {
        ether_type = *(uint16_t *)(header + ETHER_HEADER_ETH_TYPE_OFFSET);
    }
    return ether_type;
}


OAL_INLINE hmac_rx_frame_ctrl_enum hmac_host_rx_feature_filter(hmac_host_rx_context_stru *rx_context)
{
    hmac_vap_stru *hmac_vap = rx_context->hmac_vap;
    hmac_user_stru *hmac_user = rx_context->hmac_user;
    oal_netbuf_stru *netbuf = rx_context->netbuf;
    mac_rx_ctl_stru *mac_rx_ctl = NULL;
    uint16_t ether_type;

    mac_rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    if (hmac_user == NULL) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                         "{hmac_host_rx_feature_filter::user[%d] null!}",
                         MAC_GET_RX_CB_TA_USER_IDX(mac_rx_ctl));
        return HMAC_RX_FRAME_CTRL_DROP;
    }

    if (hmac_rx_qos_data_need_drop(rx_context)) {
        return HMAC_RX_FRAME_CTRL_DROP;
    }

    ether_type = hmac_rx_netbuf_get_ether_type(netbuf, mac_rx_ctl);
    if (hmac_11i_ether_type_filter(hmac_vap, hmac_user, ether_type) != OAL_SUCC) {
        return HMAC_RX_FRAME_CTRL_DROP;
    }
    return HMAC_RX_FRAME_CTRL_GOON;
}


OAL_INLINE uint32_t hmac_rx_proc_misc_feature(hmac_host_rx_context_stru *rx_context)
{
    return OAL_SUCC;
}


OAL_INLINE void hmac_rx_netbuf_join_to_list(oal_netbuf_stru *netbuf, oal_netbuf_head_stru *list)
{
    oal_netbuf_stru *next = NULL;

    while (netbuf != NULL) {
        next = oal_netbuf_next(netbuf);
        oal_netbuf_add_to_list_tail(netbuf, list);
        netbuf = next;
    }
}


OAL_INLINE void hmac_rx_context_netbuf_add_to_list(hmac_host_rx_context_stru *rx_context,
    oal_netbuf_head_stru *list, uint8_t buff_is_valid)
{
    oal_netbuf_stru *netbuf = rx_context->netbuf;

    if (buff_is_valid) {
        hmac_rx_netbuf_join_to_list(netbuf, list);
    }
    rx_context->netbuf = NULL;
}


static uint32_t hmac_rx_context_prepare_msdu_list_to_eth(hmac_host_rx_context_stru *rx_context)
{
    mac_ieee80211_frame_stru *frame_hdr = NULL;
    mac_rx_ctl_stru *rx_ctrl = rx_context->cb;
    oal_netbuf_stru *netbuf = rx_context->netbuf;
    oal_netbuf_stru *next_netbuf = NULL;
    oal_netbuf_head_stru netbuf_head;
    uint8_t datatype;

    frame_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf);

    oal_netbuf_head_init(&netbuf_head);
    while (netbuf != NULL) {
        next_netbuf = netbuf->next;
        if (rx_ctrl->bit_frame_format == MAC_FRAME_TYPE_80211) {
            /* 80211转换成8023 */
            if (hmac_rx_proc_80211_to_eth(netbuf, rx_ctrl, frame_hdr) != OAL_SUCC) {
                oal_netbuf_free(netbuf);
                continue;
            }
        }

        /* 获取datatype，进行关键帧维测 */
        datatype = mac_get_data_type_from_8023((uint8_t *)oal_netbuf_payload(netbuf), MAC_NETBUFF_PAYLOAD_ETH);
        if (datatype <= MAC_DATA_ARP_RSP) {
            hmac_rx_vip_info(&(rx_context->hmac_vap->st_vap_base_info), datatype, netbuf, rx_ctrl);
        }

#ifdef _PRE_WLAN_FEATURE_SPECIAL_PKT_LOG
        hmac_parse_special_ipv4_packet(oal_netbuf_data(netbuf), oal_netbuf_get_len(netbuf), HMAC_PKT_DIRECTION_RX);
#endif
        oal_netbuf_add_to_list_tail(netbuf, &netbuf_head);

        netbuf = next_netbuf;
    }

    if (oal_netbuf_list_empty(&netbuf_head)) {
        rx_context->netbuf = NULL;
        return OAL_FAIL;
    }

    /* 将最后一个NETBUF置空 */
    oal_set_netbuf_next((oal_netbuf_tail(&netbuf_head)), NULL);
    rx_context->netbuf = oal_netbuf_peek(&netbuf_head);

    return OAL_SUCC;
}

uint32_t hmac_rx_proc_feature(hmac_host_rx_context_stru *rx_context,
    oal_netbuf_head_stru *rpt_list, oal_netbuf_head_stru *w2w_list)
{
    oal_netbuf_stru *netbuf = rx_context->netbuf;
    uint8_t buff_is_valid = OAL_TRUE;
    oal_netbuf_head_stru release_list;

    if (hmac_host_rx_feature_filter(rx_context) == HMAC_RX_FRAME_CTRL_DROP) {
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_WAPI
    if (hmac_wapi_netbuf_rx_decrypt_1106(rx_context) != OAL_SUCC) {
        /* 解密失败，已经释放netbuf，此处返回成功，不需要释放netbuf */
        return OAL_SUCC;
    }
#endif /* _PRE_WLAN_FEATURE_WAPI */

    netbuf = hmac_rx_defrag(rx_context);
    if (netbuf == NULL) {
        return OAL_SUCC;
    }
    /* defrag后更新参数 */
    rx_context->netbuf = netbuf;

    if (hmac_rx_context_prepare_msdu_list_to_eth(rx_context) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 所有非关键功能放此接口内 */
    if (hmac_rx_proc_misc_feature(rx_context) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* release list包括需要转发和上报的netbuf */
    oal_netbuf_head_init(&release_list);
    if (hmac_rx_proc_reorder(rx_context, &release_list, &buff_is_valid) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 此函数执行后rx context中不包含netbuf,全部挂在release list中 */
    hmac_rx_context_netbuf_add_to_list(rx_context, &release_list, buff_is_valid);

    /* APUT转发链更新 */
    if (rx_context->hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_rx_ap_w2w_proc(rx_context, &release_list, w2w_list);
    }
    /* 剩下的报文全部上报协议栈 */
    oal_netbuf_queue_splice_tail_init(&release_list, rpt_list);
    return OAL_SUCC;
}


void hmac_rx_rpt_netbuf(hmac_vap_stru *hmac_vap, oal_netbuf_head_stru *list)
{
    oal_netbuf_stru *netbuf = NULL;
    oal_netbuf_stru *next = NULL;
    uint32_t need_wake_up;

    need_wake_up = (oal_netbuf_list_len(list) > 0);
    oal_netbuf_search_for_each_safe(netbuf, next, list) {
        hmac_rx_enqueue(netbuf, hmac_vap->pst_net_device);
    }
    if (need_wake_up && (hmac_get_rxthread_enable() == OAL_TRUE)) {
        hmac_rxdata_sched();
    }
}


void hmac_rx_common_proc(hmac_host_rx_context_stru *rx_context, oal_netbuf_head_stru *rpt_list)
{
    oal_netbuf_head_stru w2w_list;
    hmac_vap_stru *hmac_vap = rx_context->hmac_vap;

    oal_netbuf_list_head_init(&w2w_list);
    hmac_host_rx_static_proc(rx_context);
    if (hmac_rx_proc_feature(rx_context, rpt_list, &w2w_list) != OAL_SUCC) {
        hmac_rx_netbuf_list_free(rx_context->netbuf);
        return;
    }

    g_pm_wlan_pkt_statis.host_rx_pkt += oal_netbuf_list_len(rpt_list) + oal_netbuf_list_len(&w2w_list);
    hmac_rx_wlan_to_wlan_proc(hmac_vap, &w2w_list);
}


uint32_t hmac_host_rx_ring_data_event(frw_event_mem_stru *event_mem)
{
    hal_host_device_stru        *hal_device = NULL;
    hal_host_rx_event           *wlan_rx_event = NULL;
    oal_netbuf_head_stru         list;
    oal_netbuf_stru             *netbuf = NULL;
    hmac_host_rx_context_stru    rx_context;
    oal_netbuf_head_stru rpt_list;

    host_cnt_init_record_performance(RX_RING_PROC);
    host_start_record_performance(RX_RING_PROC);

    if (oal_unlikely(event_mem == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    wlan_rx_event = (hal_host_rx_event *)frw_get_event_stru(event_mem)->auc_event_data;
    hal_device    = wlan_rx_event->hal_dev;
    if (oal_unlikely(hal_device == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(&rx_context, sizeof(hmac_host_rx_context_stru), 0, sizeof(hmac_host_rx_context_stru));
    /* 优先补充netbuf资源 */
    hal_host_rx_add_buff(hal_device, HAL_RX_DSCR_NORMAL_PRI_QUEUE);
    /* 处理乒乓链表 */
    oal_netbuf_list_head_init(&list);
    hal_host_rx_mpdu_que_pop(hal_device, &list);
    /* MPDU级别处理每个netbuf */
    netbuf = oal_netbuf_delist(&list);
    oal_netbuf_list_head_init(&rpt_list);
    for (; netbuf; netbuf = oal_netbuf_delist(&list)) {
        host_cnt_inc_record_performance(RX_RING_PROC);

        if (hmac_host_rx_proc_dev_routine(hal_device, netbuf, &rx_context) != OAL_SUCC) {
            continue;
        }

#ifdef _PRE_WLAN_FEATURE_MONITOR
        if (hmac_host_rx_sniffer_filter_check(hal_device, rx_context.netbuf) == OAL_TRUE) {
            continue;
        }
#endif
        hmac_rx_common_proc(&rx_context, &rpt_list);
    }
    if (oal_netbuf_list_len(&rpt_list)) {
        hmac_rx_rpt_netbuf(rx_context.hmac_vap, &rpt_list);
    }

    host_end_record_performance(host_cnt_get_record_performance(RX_RING_PROC), RX_RING_PROC);
    return OAL_SUCC;
}
