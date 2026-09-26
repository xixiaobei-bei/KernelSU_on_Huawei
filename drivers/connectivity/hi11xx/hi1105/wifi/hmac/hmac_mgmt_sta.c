
#include "wlan_spec.h"
#include "wlan_mib.h"

#include "mac_frame.h"
#include "mac_ie.h"
#include "mac_regdomain.h"
#include "mac_user.h"
#include "mac_vap.h"

#include "mac_device.h"
#include "hmac_device.h"
#include "hmac_user.h"
#include "hmac_mgmt_sta_up.h"
#include "hmac_mgmt_sta.h"
#include "hmac_fsm.h"
#include "hmac_rx_data.h"
#include "hmac_chan_mgmt.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_sme_sta.h"
#include "hmac_scan.h"
#include "hmac_degradation.h"

#include "hmac_tx_amsdu.h"

#include "hmac_11i.h"

#include "hmac_protection.h"

#include "hmac_config.h"
#include "hmac_ext_if.h"
#include "hmac_p2p.h"

#include "hmac_mgmt_ap.h"

#include "hmac_blockack.h"
#include "frw_ext_if.h"

#include "hmac_roam_main.h"

#ifdef _PRE_WLAN_FEATURE_WAPI
#include "hmac_wapi.h"
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS
#include "hmac_smps.h"
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#include "hmac_opmode.h"
#endif

#ifdef _PRE_WLAN_FEATURE_WMMAC
#include "hmac_wmmac.h"
#endif

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
#include "hmac_11v.h"
#endif

#include "hmac_btcoex.h"

#ifdef _PRE_WLAN_FEATURE_SNIFFER
#ifdef CONFIG_HW_SNIFFER
#include <hwnet/ipv4/sysctl_sniffer.h>
#endif
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif

#ifdef _PRE_WLAN_FEATURE_HIEX
#include "hmac_hiex.h"
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
#include "hmac_wifi6_self_cure.h"
#include "hmac_11ax.h"
#endif
#include "hmac_ht_self_cure.h"
#include "securec.h"
#ifdef _PRE_WLAN_FEATURE_TWT
#include "hmac_twt.h"
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
#include "hmac_ftm.h"
#endif
#include "hmac_sae.h"
#include "mac_mib.h"
#include "hmac_11r.h"
#include "hmac_ota_report.h"
#include "hmac_11w.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID       OAM_FILE_ID_HMAC_MGMT_STA_C
#define MAX_AID_NUM         2007
#define MAC_TYPE_OFFSET     5

/* _puc_ie是指向ht cap字段的指针，故偏移5,6,7,8字节分别对应MCS四条空间流所支持的速率 */
#define IS_INVALID_HT_RATE_HP(_puc_ie) \
    ((0x02 == (_puc_ie)[5]) && (0x00 == (_puc_ie)[6]) && (0x05 == (_puc_ie)[7]) && (0x00 == (_puc_ie)[8]))

hmac_data_rate_stru g_st_data_11b_rates[WLAN_11B_BUTT] = {
    { 0x82, 0x02, WLAN_11B_1_M_BPS },       /* 1 Mbps   */
    { 0x84, 0x04, WLAN_11B_2_M_BPS },       /* 2 Mbps   */
    { 0x8B, 0x0B, WLAN_11B_5_HALF_M_BPS }, /* 5.5 Mbps */
    { 0x96, 0x16, WLAN_11B_11_M_BPS }       /* 11 Mbps  */
};

/* 11ag 协议速率 */
hmac_data_rate_stru g_st_data_legacy_ofdm_rates[WLAN_11AG_BUTT] = {
    { 0x8C, 0x0C, WLAN_11AG_6M_BPS },  /* 6 Mbps   */
    { 0x92, 0x12, WLAN_11AG_9M_BPS },  /* 9 Mbps   */
    { 0x98, 0x18, WLAN_11AG_12M_BPS }, /* 12 Mbps  */
    { 0xA4, 0x24, WLAN_11AG_18M_BPS }, /* 18 Mbps  */
    { 0xB0, 0x30, WLAN_11AG_24M_BPS }, /* 24 Mbps  */
    { 0xC8, 0x48, WLAN_11AG_36M_BPS }, /* 36 Mbps  */
    { 0xE0, 0x60, WLAN_11AG_48M_BPS }, /* 48 Mbps  */
    { 0xEC, 0x6C, WLAN_11AG_54M_BPS }  /* 54 Mbps  */
};


OAL_STATIC uint32_t hmac_sta_not_up_rx_wnm_action(mac_wnm_action_type_enum_uint8 en_wnm_action_field,
    hmac_vap_stru *pst_hmac_vap_sta, hmac_user_stru *pst_hmac_user, oal_netbuf_stru *pst_netbuf)
{
    switch (en_wnm_action_field) {
#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
        /* bss transition request 帧处理入口 */
        case MAC_WNM_ACTION_BSS_TRANSITION_MGMT_REQUEST:
            if (pst_hmac_user != NULL) {    /* BSS Transtion Request hmac user exist */
                hmac_rx_bsst_req_action(pst_hmac_vap_sta, pst_hmac_user, pst_netbuf);
            } else {
                oam_warning_log0(pst_hmac_vap_sta->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                    "{hmac_sta_not_up_rx_mgmt:: bss transition pst_hmac_user don't exist.}");
            }
            break;
#endif
        default:
            hmac_rx_mgmt_send_to_host(pst_hmac_vap_sta, pst_netbuf);
            break;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_FTM

OAL_STATIC uint32_t hmac_sta_not_up_rx_public_action(mac_public_action_type_enum_uint8 en_public_action_field,
    hmac_vap_stru *pst_hmac_vap_sta, oal_netbuf_stru *pst_netbuf)
{
    uint32_t ret;

    switch (en_public_action_field) {
        case MAC_PUB_GAS_INIT_RESP:
            oam_warning_log0(0, OAM_SF_RX, "{hmac_sta_not_up_rx_public_action::receive gas initial response frame.}");
            ret = hmac_ftm_rx_gas_initial_response_frame(pst_hmac_vap_sta, pst_netbuf);
            if (ret == OAL_FAIL) {
                hmac_rx_mgmt_send_to_host(pst_hmac_vap_sta, pst_netbuf);
            }
            break;

        default:
            oam_warning_log0(0, OAM_SF_RX, "{hmac_sta_not_up_rx_public_action::rx other public action frame }");
            hmac_rx_mgmt_send_to_host(pst_hmac_vap_sta, pst_netbuf);
            break;
    }

    return OAL_SUCC;
}
#endif

OAL_STATIC void hmac_sta_not_up_rx_mgmt_action_proc(uint8_t *puc_data,
    hmac_vap_stru *pst_hmac_vap_sta, hmac_user_stru *pst_hmac_user, dmac_wlan_crx_event_stru *pst_mgmt_rx_event)
{
    switch (puc_data[MAC_ACTION_OFFSET_CATEGORY]) {
        case MAC_ACTION_CATEGORY_WNM:
            hmac_sta_not_up_rx_wnm_action(puc_data[MAC_ACTION_OFFSET_ACTION], pst_hmac_vap_sta,
                                          pst_hmac_user, pst_mgmt_rx_event->pst_netbuf);
            break;
        case MAC_ACTION_CATEGORY_RADIO_MEASURMENT:
            if (puc_data[MAC_ACTION_OFFSET_ACTION] == MAC_RM_ACTION_RADIO_MEASUREMENT_REQUEST) {
                wlan_chip_rrm_proc_rm_request(pst_hmac_vap_sta, pst_mgmt_rx_event->pst_netbuf);
            }
            hmac_rx_mgmt_send_to_host(pst_hmac_vap_sta, pst_mgmt_rx_event->pst_netbuf);
        break;
#ifdef _PRE_WLAN_FEATURE_FTM // FTM认证
        case MAC_ACTION_CATEGORY_PUBLIC:
            if (g_wlan_spec_cfg->feature_ftm_is_open) {
                hmac_sta_not_up_rx_public_action(puc_data[MAC_ACTION_OFFSET_ACTION], pst_hmac_vap_sta,
                                                 pst_mgmt_rx_event->pst_netbuf);
                break;
            }
            hmac_rx_mgmt_send_to_host(pst_hmac_vap_sta, pst_mgmt_rx_event->pst_netbuf);
            break;
#endif
        default:
            hmac_rx_mgmt_send_to_host(pst_hmac_vap_sta, pst_mgmt_rx_event->pst_netbuf);
            break;
    }
}

OAL_STATIC void hmac_rx_mgmt_probe_req(hmac_vap_stru *hmac_vap, dmac_wlan_crx_event_stru *mgmt_rx_event)
{
    mac_vap_stru *mac_vap = NULL;

    mac_vap = &(hmac_vap->st_vap_base_info);
    /* 判断为P2P设备,则上报probe req帧到wpa_supplicant */
    if (!IS_LEGACY_VAP(mac_vap)) {
        hmac_rx_mgmt_send_to_host(hmac_vap, mgmt_rx_event->pst_netbuf);
    }
    return;
}

OAL_STATIC void hmac_rx_mgmt_action(uint8_t *data, hmac_vap_stru *hmac_vap, dmac_wlan_crx_event_stru *mgmt_rx_event,
    uint8_t *mac_hdr)
{
    mac_ieee80211_frame_stru *frame_hdr = NULL; /* 保存mac帧的指针 */
    hmac_user_stru *hmac_user = NULL;

#if defined(_PRE_WLAN_FEATURE_LOCATION) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
    if (oal_memcmp(data + BYTE_OFFSET_2, g_auc_huawei_oui, MAC_OUI_LEN) == 0) {
        hmac_huawei_action_process(hmac_vap, mgmt_rx_event->pst_netbuf, data[MAC_TYPE_OFFSET]);
    } else {
        frame_hdr = (mac_ieee80211_frame_stru *)mac_hdr;
        hmac_user = mac_vap_get_hmac_user_by_addr(&hmac_vap->st_vap_base_info, frame_hdr->auc_address2);
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
            "{hmac_sta_not_up_rx_mgmt::category=[%d].}", data[MAC_ACTION_OFFSET_CATEGORY]);
        hmac_sta_not_up_rx_mgmt_action_proc(data, hmac_vap, hmac_user, mgmt_rx_event);
    }
#else
    /* 如果是Action 帧，则直接上报wpa_supplicant */
    frame_hdr = (mac_ieee80211_frame_stru *)mac_hdr;
    hmac_user = mac_vap_get_hmac_user_by_addr(&hmac_vap->st_vap_base_info, frame_hdr->auc_address2);
    oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
        "{hmac_sta_not_up_rx_mgmt::category=[%d].}", data[MAC_ACTION_OFFSET_CATEGORY]);
    hmac_sta_not_up_rx_mgmt_action_proc(data, hmac_vap, hmac_user, mgmt_rx_event);
#endif
    return;
}


uint32_t hmac_sta_not_up_rx_mgmt(hmac_vap_stru *hmac_vap, void *param)
{
    dmac_wlan_crx_event_stru *mgmt_rx_event = NULL;
    mac_rx_ctl_stru *rx_info = NULL;
    uint8_t *mac_hdr = NULL;
    uint8_t *data = NULL;
    uint8_t mgmt_frm_type;

    if (oal_any_null_ptr2(hmac_vap, param)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_sta_not_up_rx_mgmt::PTR_NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mgmt_rx_event = (dmac_wlan_crx_event_stru *)param;
    rx_info = (mac_rx_ctl_stru *)oal_netbuf_cb(mgmt_rx_event->pst_netbuf);
    mac_hdr = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(rx_info);
    if (mac_hdr == NULL) {
        oam_error_log3(0, OAM_SF_RX, "{hmac_sta_not_up_rx_mgmt::mac_hdr null, vap_id %d, frame_len %d, mac_hdr_len %d}",
            rx_info->bit_vap_id, rx_info->us_frame_len, rx_info->uc_mac_header_len);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获取帧体指针 */
    data = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(rx_info) + rx_info->uc_mac_header_len;
    /* STA在NOT UP状态下接收到各种管理帧处理 */
    mgmt_frm_type = mac_get_frame_type_and_subtype(mac_hdr);

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    if (wlan_pm_wkup_src_debug_get() == OAL_TRUE) {
        wlan_pm_wkup_src_debug_set(OAL_FALSE);
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
            "{wifi_wake_src:hmac_sta_not_up_rx_mgmt::wakeup mgmt type[0x%x]}", mgmt_frm_type);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_SNIFFER
#ifdef CONFIG_HW_SNIFFER
    proc_sniffer_write_file(NULL, 0, mac_hdr, rx_info->us_frame_len, 0);
#endif
#endif

    switch (mgmt_frm_type) {
        /* 判断接收到的管理帧类型 */
        case WLAN_FC0_SUBTYPE_PROBE_REQ | WLAN_FC0_TYPE_MGT:
            hmac_rx_mgmt_probe_req(hmac_vap, mgmt_rx_event);
            break;
        case WLAN_FC0_SUBTYPE_ACTION | WLAN_FC0_TYPE_MGT:
            hmac_rx_mgmt_action(data, hmac_vap, mgmt_rx_event, mac_hdr);
            break;
        case WLAN_FC0_SUBTYPE_PROBE_RSP | WLAN_FC0_TYPE_MGT:
            /* 如果是probe response帧，则直接上报wpa_supplicant */
            hmac_rx_mgmt_send_to_host(hmac_vap, mgmt_rx_event->pst_netbuf);
            break;
        default:
            break;
    }
    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_update_vht_opern_ie_sta(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user,
    uint8_t *puc_payload)
{
    if (oal_any_null_ptr3(pst_mac_vap, pst_hmac_user, puc_payload)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_update_vht_opern_ie_sta::mac_vap or hmac_user or payload is null.}");
        return MAC_NO_CHANGE;
    }

    /* 支持11ac，才进行后续的处理 */
    if (OAL_FALSE == mac_mib_get_VHTOptionImplemented(pst_mac_vap)) {
        return MAC_NO_CHANGE;
    }

    return mac_ie_proc_vht_opern_ie(pst_mac_vap, puc_payload, &(pst_hmac_user->st_user_base_info));
}


void hmac_sta_set_txopps_partial_aid(mac_vap_stru *pst_mac_vap)
{
    uint16_t us_temp_aid;
    uint8_t uc_temp_bssid;
    uint32_t ret;
    mac_cfg_txop_sta_stru st_txop_info = { 0 };

    /* 此处需要注意:按照协议规定(802.11ac-2013.pdf,9.17a)，ap分配给sta的aid，不可以
       使计算出来的partial aid为0，后续如果ap支持的最大关联用户数目超过512，必须要
       对aid做这个合法性检查
    */
    if (pst_mac_vap->en_protocol != WLAN_VHT_MODE && pst_mac_vap->en_protocol != WLAN_VHT_ONLY_MODE &&
        pst_mac_vap->en_protocol != WLAN_HE_MODE) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_TXOP, "{hmac_sta_set_txopps_partial_aid::VAP unsupport}.");
        return;
    }

    /* 计算partial aid */
    us_temp_aid = pst_mac_vap->us_sta_aid & 0x1FF;
    uc_temp_bssid = (pst_mac_vap->auc_bssid[BYTE_OFFSET_5] & 0x0F) ^
        ((pst_mac_vap->auc_bssid[BYTE_OFFSET_5] & 0xF0) >> BIT_OFFSET_4);
    st_txop_info.us_partial_aid = (us_temp_aid + (uc_temp_bssid << BIT_OFFSET_5)) & ((1 << BIT_OFFSET_9) - 1);
    if (st_txop_info.us_partial_aid == 0) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_TXOP,
            "{hmac_sta_set_txopps_partial_aid::partial aid calc zero! aid[%d].}", pst_mac_vap->us_sta_aid);
    }

    /***************************************************************************
    抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_STA_TXOP_AID, sizeof(mac_cfg_txop_sta_stru),
        (uint8_t *)&st_txop_info);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_TXOP,
                         "{hmac_sta_set_txopps_partial_aid::hmac_config_send_event failed[%d].}", ret);
    }

    return;
}


void hmac_sta_update_mac_user_info(hmac_user_stru *pst_hmac_user_ap, uint16_t us_user_idx)
{
    mac_vap_stru *pst_mac_vap = NULL;
    mac_user_stru *pst_mac_user_ap = NULL;
    uint32_t ret;

    if (pst_hmac_user_ap == NULL) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_sta_update_mac_user_info::param null.}");
        return;
    }

    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_hmac_user_ap->st_user_base_info.uc_vap_id);
    if (oal_unlikely(pst_mac_vap == NULL)) {
        oam_error_log1(0, OAM_SF_RX, "{hmac_sta_update_mac_user_info::get mac_vap [vap_id:%d] null.}",
                       pst_hmac_user_ap->st_user_base_info.uc_vap_id);
        return;
    }

    pst_mac_user_ap = &(pst_hmac_user_ap->st_user_base_info);

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_RX,
                     "{hmac_sta_update_mac_user_info::us_user_idx:%d,en_avail_bandwidth:%d,en_cur_bandwidth:%d}",
                     us_user_idx,
                     pst_mac_user_ap->en_avail_bandwidth,
                     pst_mac_user_ap->en_cur_bandwidth);

    ret = hmac_config_user_info_syn(pst_mac_vap, pst_mac_user_ap);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_RX,
                       "{hmac_sta_update_mac_user_info::hmac_config_user_info_syn failed[%d].}", ret);
    }

    ret = hmac_config_user_rate_info_syn(pst_mac_vap, pst_mac_user_ap);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_RX,
                       "{hmac_sta_wait_asoc_rx::hmac_syn_rate_info failed[%d].}", ret);
    }
    return;
}


uint8_t *hmac_sta_find_ie_in_probe_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_eid, uint16_t *pus_index)
{
    mac_bss_dscr_stru *pst_bss_dscr = NULL;
    uint8_t *puc_ie = NULL;
    uint8_t *puc_payload = NULL;
    uint8_t us_offset;

    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_SCAN, "{find ie fail, pst_mac_vap is null.}");
        return NULL;
    }

    pst_bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(pst_mac_vap, pst_mac_vap->auc_bssid);
    if (pst_bss_dscr == NULL) {
        oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_sta_find_ie_in_probe_rsp::find the bss failed by bssid:%02X:XX:XX:%02X:%02X:%02X",
                         pst_mac_vap->auc_bssid[0],
                         pst_mac_vap->auc_bssid[BYTE_OFFSET_3],
                         pst_mac_vap->auc_bssid[BYTE_OFFSET_4],
                         pst_mac_vap->auc_bssid[BYTE_OFFSET_5]);

        return NULL;
    }

    /* 以IE开头的payload，返回供调用者使用 */
    us_offset = MAC_80211_FRAME_LEN + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    /*lint -e416*/
    puc_payload = (uint8_t *)(pst_bss_dscr->auc_mgmt_buff + us_offset);
    /*lint +e416*/
    if (pst_bss_dscr->mgmt_len < us_offset) {
        return NULL;
    }

    puc_ie = mac_find_ie(uc_eid, puc_payload, (int32_t)(pst_bss_dscr->mgmt_len - us_offset));
    if (puc_ie == NULL) {
        return NULL;
    }

    /* IE长度初步校验 */
    if (*(puc_ie + 1) == 0) {
        oam_warning_log1(0, OAM_SF_ANY, "{IE[%d] len in probe rsp is 0, find ie fail.}", uc_eid);
        return NULL;
    }

    *pus_index = (uint16_t)(puc_ie - puc_payload);

    oam_warning_log1(0, OAM_SF_ANY, "{found ie[%d] in probe rsp.}", uc_eid);

    return puc_payload;
}


void hmac_sta_check_ht_cap_ie(mac_vap_stru *pst_mac_sta, uint8_t *puc_payload, mac_user_stru *pst_mac_user_ap,
    uint16_t *pus_amsdu_maxsize, uint16_t us_payload_len)
{
    uint8_t *puc_ie = NULL;
    uint8_t *puc_payload_for_ht_cap_chk = NULL;
    uint16_t us_ht_cap_index;

    if (oal_any_null_ptr3(pst_mac_sta, puc_payload, pst_mac_user_ap)) {
        return;
    }

    puc_ie = mac_find_ie(MAC_EID_HT_CAP, puc_payload, us_payload_len);
    if (puc_ie == NULL || puc_ie[1] < MAC_HT_CAP_LEN) {
        puc_payload_for_ht_cap_chk = hmac_sta_find_ie_in_probe_rsp(pst_mac_sta, MAC_EID_HT_CAP, &us_ht_cap_index);
        if (puc_payload_for_ht_cap_chk == NULL) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_sta_check_ht_cap_ie::puc_payload_for_ht_cap_chk is null.}");
            return;
        }

        /*lint -e413*/
        if (puc_payload_for_ht_cap_chk[us_ht_cap_index + 1] < MAC_HT_CAP_LEN) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_sta_check_ht_cap_ie::invalid ht cap len[%d].}",
                             puc_payload_for_ht_cap_chk[us_ht_cap_index + 1]);
            return;
        }
        /*lint +e413*/
        puc_ie = puc_payload_for_ht_cap_chk + us_ht_cap_index; /* 赋值HT CAP IE */
    } else {
        if (puc_ie < puc_payload) {
            return;
        }
        us_ht_cap_index = (uint16_t)(puc_ie - puc_payload);
        puc_payload_for_ht_cap_chk = puc_payload;
    }

    if ((WLAN_BAND_2G == pst_mac_sta->st_channel.en_band) && IS_INVALID_HT_RATE_HP(puc_ie)) {
        oam_warning_log0(pst_mac_sta->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_sta_update_ht_cap:: invalid mcs set, disable HT.}");
        mac_user_set_ht_capable(pst_mac_user_ap, OAL_FALSE);
        return;
    }

    /* 根据协议值设置特性，必须在hmac_amsdu_init_user后面调用 */
    mac_ie_proc_ht_sta(pst_mac_sta, puc_payload_for_ht_cap_chk, us_ht_cap_index, pst_mac_user_ap, pus_amsdu_maxsize);
}


void hmac_sta_check_ext_cap_ie(mac_vap_stru *pst_mac_sta, mac_user_stru *pst_mac_user_ap, uint8_t *puc_payload,
    uint16_t us_rx_len)
{
    uint8_t *puc_ie;
    uint8_t *puc_payload_proc = NULL;
    uint16_t us_index;

    puc_ie = mac_find_ie(MAC_EID_EXT_CAPS, puc_payload, us_rx_len);
    if (puc_ie == NULL || puc_ie[1] < MAC_MIN_XCAPS_LEN) {
        puc_payload_proc = hmac_sta_find_ie_in_probe_rsp(pst_mac_sta, MAC_EID_EXT_CAPS, &us_index);
        if (puc_payload_proc == NULL) {
            return;
        }

        /*lint -e413*/
        if (puc_payload_proc[us_index + 1] < MAC_MIN_XCAPS_LEN) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_sta_check_ext_cap_ie::invalid ext cap len[%d].}",
                             puc_payload_proc[us_index + 1]);
            return;
        }
        /*lint +e413*/
    } else {
        puc_payload_proc = puc_payload;
        if (puc_ie < puc_payload) {
            return;
        }

        us_index = (uint16_t)(puc_ie - puc_payload);
    }

    /* 处理 Extended Capabilities IE */
    /*lint -e613*/
    mac_ie_proc_ext_cap_ie(pst_mac_user_ap, &puc_payload_proc[us_index]);
    /*lint +e613*/
}


uint32_t hmac_sta_check_ht_opern_ie(mac_vap_stru *pst_mac_sta, mac_user_stru *pst_mac_user_ap, uint8_t *puc_payload,
    uint16_t us_rx_len)
{
    uint8_t *puc_ie;
    uint8_t *puc_payload_proc = NULL;
    uint16_t us_index;
    uint32_t change = MAC_NO_CHANGE;

    puc_ie = mac_find_ie(MAC_EID_HT_OPERATION, puc_payload, us_rx_len);
    if (puc_ie == NULL || puc_ie[1] < MAC_HT_OPERN_LEN) {
        puc_payload_proc = hmac_sta_find_ie_in_probe_rsp(pst_mac_sta, MAC_EID_HT_OPERATION, &us_index);
        if (puc_payload_proc == NULL) {
            return change;
        }

        /*lint -e413*/
        if (puc_payload_proc[us_index + 1] < MAC_HT_OPERN_LEN) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_sta_check_ht_opern_ie::invalid ht cap len[%d].}",
                             puc_payload_proc[us_index + 1]);
            return change;
        }
        /*lint +e413*/
    } else {
        puc_payload_proc = puc_payload;
        if (puc_ie < puc_payload) {
            return change;
        }

        us_index = (uint16_t)(puc_ie - puc_payload);
    }

    change |= mac_proc_ht_opern_ie(pst_mac_sta, &puc_payload_proc[us_index], pst_mac_user_ap);

    return change;
}


uint32_t hmac_mgmt_assoc_rsp_check_ht_sta(mac_vap_stru *pst_mac_sta, uint8_t *puc_payload, uint16_t us_rx_len,
    hmac_user_stru *p_hmac_user_ap, uint16_t *pus_amsdu_maxsize)
{
    uint32_t change = MAC_NO_CHANGE;

    if (oal_any_null_ptr3(pst_mac_sta, puc_payload, p_hmac_user_ap)) {
        return change;
    }

    hmac_ht_self_cure_event_set(pst_mac_sta, p_hmac_user_ap->st_user_base_info.auc_user_mac_addr,
        HMAC_HT_SELF_CURE_EVENT_ASSOC);
    hmac_user_set_close_ht_flag(pst_mac_sta, p_hmac_user_ap);

    /* 初始化HT cap为FALSE，入网时会把本地能力跟随AP能力 */
    mac_user_set_ht_capable(&p_hmac_user_ap->st_user_base_info, OAL_FALSE);

    /* 至少支持11n才进行后续的处理 */
    if (OAL_FALSE == mac_mib_get_HighThroughputOptionImplemented(pst_mac_sta)) {
        return change;
    }

    hmac_sta_check_ht_cap_ie(pst_mac_sta, puc_payload, &p_hmac_user_ap->st_user_base_info,
        pus_amsdu_maxsize, us_rx_len);
    /* sta处理AP的 Extended Capability */
    hmac_sta_check_ext_cap_ie(pst_mac_sta, &p_hmac_user_ap->st_user_base_info, puc_payload, us_rx_len);

    change = hmac_sta_check_ht_opern_ie(pst_mac_sta, &p_hmac_user_ap->st_user_base_info, puc_payload, us_rx_len);

    if (p_hmac_user_ap->st_user_base_info.st_vht_hdl.en_vht_capable == OAL_FALSE) {
        hmac_user_set_amsdu_level(p_hmac_user_ap, WLAN_TX_AMSDU_BY_2);
    }

    return change;
}



OAL_STATIC void hmac_1024qam_judge(hmac_user_stru *pst_hmac_user, uint8_t *puc_payload, uint16_t us_msg_len)
{
    uint8_t *puc_tmp_ie = NULL;
    puc_tmp_ie = mac_find_vendor_ie(MAC_HUAWEI_VENDER_IE, MAC_HISI_1024QAM_IE, puc_payload, us_msg_len);
    if (puc_tmp_ie != NULL) {
        pst_hmac_user->st_user_base_info.st_cap_info.bit_1024qam_cap = OAL_TRUE;
        pst_hmac_user->st_user_base_info.st_cap_info.bit_20mmcs11_compatible_1103 = OAL_TRUE;
        pst_hmac_user->user_hisi = OAL_TRUE; /* 支持1024qam且可获得MAC_HUAWEI_VENDER_IE的为hisi芯片 */
        return;
    }
    pst_hmac_user->user_hisi = OAL_FALSE;

    if (get_hi110x_subchip_type() != BOARD_VERSION_HI1103) {
        /* BCM私有IE的1024QAM */
        puc_tmp_ie = mac_find_vendor_ie(MAC_WLAN_OUI_BROADCOM_EPIGRAM, MAC_WLAN_OUI_VENDOR_VHT_TYPE,
            puc_payload, us_msg_len);
        if ((puc_tmp_ie != NULL) && (puc_tmp_ie[1] >= MAC_WLAN_OUI_VENDOR_VHT_HEADER)) {
            if ((puc_tmp_ie[BYTE_OFFSET_6] == MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE2)
                || (puc_tmp_ie[BYTE_OFFSET_6] == MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE4)) {
                pst_hmac_user->st_user_base_info.st_cap_info.bit_1024qam_cap = OAL_TRUE;
                pst_hmac_user->st_user_base_info.st_cap_info.bit_20mmcs11_compatible_1103 = OAL_FALSE;
            }
        }
    }
}

OAL_STATIC void hmac_proc_asoc_rsp_set_sta_ampdu_active(mac_vap_stru *pst_mac_vap, hmac_vap_stru *pst_hmac_sta,
    hmac_user_stru *pst_hmac_user)
{
    if (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) {
#ifdef _PRE_WLAN_FEATURE_11AX
        if (pst_hmac_user->st_user_base_info.en_protocol_mode == WLAN_HE_MODE) {
            pst_hmac_sta->bit_rx_ampduplusamsdu_active = !(g_uc_host_rx_ampdu_amsdu & (BIT2));
        } else {
            pst_hmac_sta->bit_rx_ampduplusamsdu_active = g_uc_host_rx_ampdu_amsdu & BIT1;
        }
#else
        pst_hmac_sta->bit_rx_ampduplusamsdu_active = g_uc_host_rx_ampdu_amsdu & BIT1;
#endif
    } else {
        pst_hmac_sta->bit_rx_ampduplusamsdu_active = g_uc_host_rx_ampdu_amsdu & BIT0;
    }
}

OAL_STATIC void hmac_process_assoc_rsp_bw_change_info(mac_vap_stru *mac_vap,
    hmac_vap_stru *pst_hmac_sta, hmac_user_stru *pst_hmac_user, uint32_t change)
{
    if (MAC_BW_CHANGE & change) {
        /* 获取用户与VAP带宽能力交集,通知硬件切带宽 */
        hmac_chan_sync(mac_vap, mac_vap->st_channel.uc_chan_number, mac_vap->st_channel.en_bandwidth,
            OAL_TRUE);
        oam_warning_log4(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_process_assoc_rsp:change BW. change[0x%x], vap band[%d] chnl[%d], bandwidth[%d].}",
                         change, mac_vap->st_channel.en_band, mac_vap->st_channel.uc_chan_number,
                         mac_vap->st_channel.en_bandwidth);
        oam_warning_log4(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_process_assoc_rsp:mac usr id[%d] bw cap[%d]avil[%d]cur[%d].}",
                         pst_hmac_user->st_user_base_info.us_assoc_id,
                         pst_hmac_user->st_user_base_info.en_bandwidth_cap,
                         pst_hmac_user->st_user_base_info.en_avail_bandwidth,
                         pst_hmac_user->st_user_base_info.en_cur_bandwidth);
    }
}

void hmac_ie_check_priv_mcs_judge(hmac_user_stru *pst_hmac_user, uint8_t *puc_payload, uint16_t us_msg_len)
{
#ifdef _PRE_WLAN_FEATURE_1024QAM
    hmac_1024qam_judge(pst_hmac_user, puc_payload, us_msg_len);
#endif
}

static void hmac_process_assoc_rsp_update_vht_cap(uint8_t *payload,
    uint32_t msg_len, hmac_user_stru *hmac_user, hmac_vap_stru *hmac_sta, uint32_t *change)
{
    mac_vap_stru *mac_vap = &(hmac_sta->st_vap_base_info);
    uint8_t *tmp_ie = NULL;

    /* 评估是否需要进行带宽切换 */
    /* 根据BRCM VENDOR OUI 适配2G 11AC */
    if (hmac_user->st_user_base_info.st_vht_hdl.en_vht_capable == OAL_FALSE) {
        uint8_t *puc_vendor_vht_ie;
        uint32_t vendor_vht_ie_offset = MAC_WLAN_OUI_VENDOR_VHT_HEADER + MAC_IE_HDR_LEN;
        puc_vendor_vht_ie = mac_find_vendor_ie(MAC_WLAN_OUI_BROADCOM_EPIGRAM, MAC_WLAN_OUI_VENDOR_VHT_TYPE,
            payload, msg_len);
        if ((puc_vendor_vht_ie != NULL) && (puc_vendor_vht_ie[1] >= vendor_vht_ie_offset)) {
            oam_warning_log0(hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                "{hmac_process_assoc_rsp::find broadcom/epigram vendor ie, enable hidden bit_11ac2g}");

            /* 进入此函数代表user支持2G 11ac */
            tmp_ie = mac_find_ie(MAC_EID_VHT_CAP, puc_vendor_vht_ie + vendor_vht_ie_offset,
                (int32_t)(puc_vendor_vht_ie[1] - MAC_WLAN_OUI_VENDOR_VHT_HEADER));
            if (tmp_ie != NULL) {
                hmac_proc_vht_cap_ie(mac_vap, hmac_user, tmp_ie);
            }
            /* 更新11ac VHT operation ie */
            tmp_ie = mac_find_ie(MAC_EID_VHT_OPERN, puc_vendor_vht_ie + vendor_vht_ie_offset,
                (int32_t)(puc_vendor_vht_ie[1] - MAC_WLAN_OUI_VENDOR_VHT_HEADER));
            if (tmp_ie != NULL) {
                *change |= hmac_update_vht_opern_ie_sta(mac_vap, hmac_user, tmp_ie);
            }
        }
    }
}

static void hmac_process_assoc_rsp_update_cap_ie(uint8_t *payload,
    uint32_t msg_len, hmac_user_stru *hmac_user, hmac_vap_stru *hmac_sta, uint32_t *change)
{
    mac_vap_stru *mac_vap = &(hmac_sta->st_vap_base_info);
    uint8_t *tmp_ie = NULL;

    /* 更新关联用户的 QoS protocol table */
    hmac_mgmt_update_assoc_user_qos_table(payload, msg_len, hmac_user);

    /* 更新关联用户的legacy速率集合 */
    hmac_user_init_rates(hmac_user);
    hmac_ie_proc_assoc_user_legacy_rate(payload, msg_len, hmac_user);

    hmac_user_cap_update_by_hisi_cap_ie(hmac_user, payload, msg_len);
#ifdef _PRE_WLAN_FEATURE_11AX
    /* 更新 11ax HE Capabilities ie */
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        *change |= hmac_ie_check_he_sta(hmac_sta, hmac_user, payload, msg_len);
    }
#endif

    /* 更新 HT 参数  以及 EXTEND CAPABILITY */
    memset_s(&(hmac_user->st_user_base_info.st_ht_hdl), sizeof(mac_user_ht_hdl_stru),
        0, sizeof(mac_user_ht_hdl_stru));
    *change |= hmac_mgmt_assoc_rsp_check_ht_sta(&hmac_sta->st_vap_base_info, payload, msg_len,
        hmac_user, &hmac_user->amsdu_maxsize);

    /* 更新11ac VHT capabilities ie */
    memset_s(&(hmac_user->st_user_base_info.st_vht_hdl), sizeof(mac_vht_hdl_stru),
             0, sizeof(mac_vht_hdl_stru));
    tmp_ie = mac_find_ie(MAC_EID_VHT_CAP, payload, msg_len);
    if (tmp_ie != NULL) {
        hmac_proc_vht_cap_ie(mac_vap, hmac_user, tmp_ie);
        hmac_ie_check_priv_mcs_judge(hmac_user, payload, msg_len);
    }
    /* 更新11ac VHT operation ie */
    tmp_ie = mac_find_ie(MAC_EID_VHT_OPERN, payload, msg_len);
    if (tmp_ie != NULL) {
        *change |= hmac_update_vht_opern_ie_sta(mac_vap, hmac_user, tmp_ie);
    }

    if (OAL_TRUE == hmac_user_ht_support(hmac_user)) {
        hmac_process_assoc_rsp_update_vht_cap(payload, msg_len, hmac_user, hmac_sta, change);
    }

#ifdef _PRE_WLAN_FEATURE_HIEX
    if (g_wlan_spec_cfg->feature_hiex_is_open) {
        hmac_rx_assoc_rsp_parse_tb_frame_gain(payload, msg_len);
        *change |= hmac_hiex_rx_assoc_rsp(hmac_sta, hmac_user, payload, msg_len);
    }
#endif
}

static void hmac_process_assoc_rsp_set_user_para(hmac_user_stru *hmac_user,
    hmac_vap_stru *hmac_sta, uint32_t *change)
{
    mac_vap_stru *mac_vap = &(hmac_sta->st_vap_base_info);
    mac_user_stru *mac_user = &(hmac_user->st_user_base_info);
    wlan_channel_bandwidth_enum_uint8 sta_new_bandwidth;
    uint32_t ret;
    uint8_t avail_mode;
#ifdef _PRE_WLAN_FEATURE_11AC2G
    oal_bool_enum_uint8 avail_mode_is_vht;
#endif

    avail_mode =
        g_auc_avail_protocol_mode[mac_vap->en_protocol][hmac_user->st_user_base_info.en_protocol_mode];
#ifdef _PRE_WLAN_FEATURE_11AC2G
    avail_mode_is_vht = ((mac_vap->en_protocol == WLAN_HT_MODE) &&
        (hmac_user->st_user_base_info.en_protocol_mode == WLAN_VHT_MODE) &&
        (mac_vap->st_cap_flag.bit_11ac2g == OAL_TRUE) && (mac_vap->st_channel.en_band == WLAN_BAND_2G));
    if (avail_mode_is_vht) {
        avail_mode = WLAN_VHT_MODE;
    }
#endif

    hmac_proc_asoc_rsp_set_sta_ampdu_active(mac_vap, hmac_sta, hmac_user);

    /* 获取用户与VAP协议模式交集 */
    mac_user_set_avail_protocol_mode(&hmac_user->st_user_base_info, avail_mode);
    mac_user_set_cur_protocol_mode(&hmac_user->st_user_base_info,
        hmac_user->st_user_base_info.en_avail_protocol_mode);

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        if (hmac_user->st_user_base_info.en_cur_protocol_mode == WLAN_HE_MODE) {
            hmac_user->uc_tx_ba_limit = DMAC_UCAST_FRAME_TX_COMP_TIMES_HE;
        }
    }
#endif

    /* 获取用户和VAP 可支持的11a/b/g 速率交集 */
    hmac_vap_set_user_avail_rates(&(hmac_sta->st_vap_base_info), hmac_user);

    /* 获取用户与VAP空间流交集 */
    ret = hmac_user_set_avail_num_space_stream(mac_user, mac_vap->en_vap_rx_nss);
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_process_assoc_rsp:ap max nss is [%d]}",
                     hmac_user->st_user_base_info.en_user_max_cap_nss);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_process_assoc_rsp::mac_user_set_avail_num_space_stream failed[%d].}", ret);
    }

    /* 获取用户和VAP带宽，并判断是否有带宽变化需要通知硬件切带宽 */
    sta_new_bandwidth = mac_vap_get_ap_usr_opern_bandwidth(mac_vap, &(hmac_user->st_user_base_info));
    *change |= mac_vap_set_bw_check(mac_vap, sta_new_bandwidth);

    /* 同步用户的带宽能力 */
    mac_user_set_bandwidth_info(mac_user, mac_vap_bw_mode_to_bw(sta_new_bandwidth),
        mac_vap_bw_mode_to_bw(sta_new_bandwidth));

    /* 获取用户160M带宽下的空间流 */
    hmac_user_set_num_spatial_stream_160M(mac_user);

#ifdef _PRE_WLAN_FEATURE_SMPS
    /* 根据smps更新空间流能力 */
    if (!IS_VAP_SINGLE_NSS(mac_vap) && !IS_USER_SINGLE_NSS(mac_user)) {
        hmac_smps_update_user_status(mac_vap, mac_user);
    }
#endif
}

static void hmac_process_assoc_rsp_user_sync_cap(hmac_vap_stru *hmac_sta,
    hmac_user_stru *hmac_user, uint8_t *mac_hdr, uint8_t *payload, uint16_t msg_len)
{
    mac_vap_stru *mac_vap = &(hmac_sta->st_vap_base_info);
    mac_user_stru *mac_user = &(hmac_user->st_user_base_info);
    uint32_t ret;

    /* 同步空间流信息 */
    ret = hmac_config_user_num_spatial_stream_cap_syn(mac_vap, mac_user);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_process_assoc_rsp::hmac_config_user_num_spatial_stream_cap_syn failed[%d].}", ret);
    }

    /* dmac offload架构下，同步STA USR信息到dmac */
    ret = hmac_config_user_cap_syn(&(hmac_sta->st_vap_base_info), &hmac_user->st_user_base_info);

    ret = hmac_config_user_info_syn(&(hmac_sta->st_vap_base_info), &hmac_user->st_user_base_info);

    ret = hmac_config_user_rate_info_syn(&(hmac_sta->st_vap_base_info), &hmac_user->st_user_base_info);

    /* dmac offload架构下，同步STA USR信息到dmac */
    ret = hmac_config_sta_vap_info_syn(&(hmac_sta->st_vap_base_info));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_process_assoc_rsp_user_sync_cap::ret not succ[%d].}", ret);
    }

    /* edac 设置放在关联状态设置完成之后，便于知道需要设置的队列ID,sta更新自身的edca parameters, assoc rsp帧是管理帧 */
    hmac_sta_up_update_edca_params(payload, msg_len, hmac_sta, mac_get_frame_sub_type(mac_hdr),
        hmac_user);

    hmac_arp_probe_type_set(mac_vap, OAL_TRUE, HMAC_VAP_ARP_PROBE_TYPE_HTC);
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        hmac_sta_up_update_he_edca_params(payload, msg_len, hmac_sta,
                                          mac_get_frame_sub_type(mac_hdr), hmac_user);
    }
#endif
}

#ifdef _PRE_WLAN_FEATURE_11AX
static void hmac_process_assoc_rsp_update_sr_nfrp_para(hmac_vap_stru *hmac_sta,
    hmac_user_stru *hmac_user, uint8_t *mac_hdr, uint8_t *payload, uint16_t msg_len)
{
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        hmac_sta_up_update_assoc_rsp_sr_params(payload, msg_len, hmac_sta,
                                               mac_get_frame_sub_type(mac_hdr), hmac_user);
        hmac_sta_up_update_he_nfrp_params(payload, msg_len, hmac_sta,
            mac_get_frame_sub_type(mac_hdr), hmac_user);
    }
}
#endif


uint32_t hmac_process_assoc_rsp(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    uint8_t *mac_hdr, uint16_t hdr_len, uint8_t *payload, uint16_t msg_len)
{
    uint32_t rslt, ret;
    uint16_t aid;
    mac_vap_stru *mac_vap = NULL;
    uint32_t change = MAC_NO_CHANGE;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_process_assoc_rsp::pst_hmac_sta null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap = &(hmac_vap->st_vap_base_info);

    /* 更新关联ID */
    aid = mac_get_asoc_id(payload);
    if ((aid > 0) && (aid <= MAX_AID_NUM)) {
        mac_vap_set_aid(mac_vap, aid);
    } else {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_process_assoc_rsp::invalid us_sta_aid[%d].}", aid);
    }

    payload += (MAC_CAP_INFO_LEN + MAC_STATUS_CODE_LEN + MAC_AID_LEN);
    msg_len -= (MAC_CAP_INFO_LEN + MAC_STATUS_CODE_LEN + MAC_AID_LEN);

    /* 初始化安全端口过滤参数 */
    rslt = hmac_init_user_security_port(&(hmac_vap->st_vap_base_info), &(hmac_user->st_user_base_info));
    if (rslt != OAL_SUCC) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_process_assoc_rsp::hmac_init_user_security_port failed[%d].}", rslt);
    }

#ifdef _PRE_WLAN_FEATURE_PMF
    /* STA模式下的pmf能力来源于WPA_supplicant，只有启动pmf和不启动pmf两种类型 */
    mac_user_set_pmf_active(&hmac_user->st_user_base_info, mac_vap->en_user_pmf_cap);
    oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                     "{hmac_process_assoc_rsp::set user pmf[%d].}", mac_vap->en_user_pmf_cap);
#endif /* #if(_PRE_WLAN_FEATURE_PMF) */

#ifdef _PRE_WLAN_FEATURE_11AX
    hmac_process_assoc_rsp_update_sr_nfrp_para(hmac_vap, hmac_user, mac_hdr, payload, msg_len);
#endif
    hmac_process_assoc_rsp_update_cap_ie(payload, msg_len, hmac_user, hmac_vap, &change);
    /* 获取用户的协议模式 */
    hmac_set_user_protocol_mode(mac_vap, hmac_user);

    hmac_process_assoc_rsp_set_user_para(hmac_user, hmac_vap, &change);

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    /* 处理Operating Mode Notification 信息元素 */
    ret = hmac_check_opmode_notify(hmac_vap, mac_hdr, payload, msg_len, hmac_user);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_process_assoc_rsp::hmac_check_opmode_notify failed[%d].}", ret);
    }
#endif

    mac_user_set_asoc_state(&hmac_user->st_user_base_info, MAC_USER_STATE_ASSOC);
    hmac_process_assoc_rsp_user_sync_cap(hmac_vap, hmac_user, mac_hdr, payload, msg_len);

    /* sta计算自身的partial aid，写入到mac寄存器 */
    hmac_sta_set_txopps_partial_aid(mac_vap);

    
    hmac_process_assoc_rsp_bw_change_info(mac_vap, hmac_vap, hmac_user, change);

    return OAL_SUCC;
}

static void hmac_sta_wait_asoc_rx_set_ba_allowed(hmac_vap_stru *hmac_sta,
    hmac_user_stru *hmac_user_ap, uint8_t *addr_sa)
{
    if (hmac_btcoex_check_exception_in_list(hmac_sta, addr_sa) == OAL_TRUE) {
        if (HMAC_BTCOEX_GET_BLACKLIST_TYPE(hmac_user_ap) == BTCOEX_BLACKLIST_TPYE_FIX_BASIZE) {
            oam_warning_log0(hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_COEX,
                             "{hmac_sta_wait_asoc_rx_set_ba_allowed::mac_addr in blacklist.}");
            hmac_user_ap->st_hmac_user_btcoex.st_hmac_btcoex_addba_req.en_ba_handle_allow = OAL_FALSE;
        } else {
            hmac_user_ap->st_hmac_user_btcoex.st_hmac_btcoex_addba_req.en_ba_handle_allow = OAL_TRUE;
        }
    } else {
        /* 初始允许建立聚合，两个方案保持对齐 */
        hmac_user_ap->st_hmac_user_btcoex.st_hmac_btcoex_addba_req.en_ba_handle_allow = OAL_TRUE;
    }
}

static uint32_t hmac_sta_wait_asoc_rx_judge_frame_subtype(hmac_vap_stru *hmac_sta,
    uint8_t *mac_hdr, dmac_wlan_crx_event_stru *mgmt_rx_event)
{
    dmac_rx_ctl_stru *rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(mgmt_rx_event->pst_netbuf);
    uint8_t frame_sub_type = mac_get_frame_type_and_subtype(mac_hdr);

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    if (OAL_TRUE == wlan_pm_wkup_src_debug_get()) {
        wlan_pm_wkup_src_debug_set(OAL_FALSE);
        oam_warning_log1(hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                         "{wifi_wake_src:hmac_sta_wait_asoc_rx::wakeup mgmt type[0x%x]}",
                         frame_sub_type);
    }
#endif

    if ((WLAN_FC0_SUBTYPE_ASSOC_RSP | WLAN_FC0_TYPE_MGT) != frame_sub_type &&
        (WLAN_FC0_SUBTYPE_REASSOC_RSP | WLAN_FC0_TYPE_MGT) != frame_sub_type) {
        oam_warning_log1(hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_sta_wait_asoc_rx:: uc_frame_sub_type=0x%02x.}", frame_sub_type);
        /* 打印此netbuf相关信息 */
        mac_rx_report_80211_frame((uint8_t *)&(hmac_sta->st_vap_base_info), (uint8_t *)rx_ctrl,
            mgmt_rx_event->pst_netbuf, OAM_OTA_TYPE_RX_HMAC_CB);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

static uint32_t hmac_sta_wait_asoc_rx_frame_para_check(hmac_vap_stru *hmac_sta, uint8_t *payload, uint16_t msg_len)
{
    mac_status_code_enum_uint16 asoc_status;
    if (msg_len < OAL_ASSOC_RSP_FIXED_OFFSET) {
        oam_error_log1(hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_sta_wait_asoc_rx::asoc_rsp_body is too short(%d) to going on!}", msg_len);
        return OAL_FAIL;
    }
    asoc_status = mac_get_asoc_status(payload);
    if (asoc_status != MAC_SUCCESSFUL_STATUSCODE) {
        oam_warning_log1(hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_sta_wait_asoc_rx_frame_para_check:: AP refuse STA assoc reason=%d.}", asoc_status);
        if (asoc_status != MAC_AP_FULL) {
            chr_exception(chr_wifi_drv(CHR_WIFI_DRV_EVENT_CONNECT, CHR_WIFI_DRV_ERROR_ASSOC_REJECTED));
        }

        hmac_sta->st_mgmt_timetout_param.en_status_code = asoc_status;

        return OAL_FAIL;
    }
    return OAL_SUCC;
}

static void hmac_sta_wait_asoc_rx_cfg_rsp_para(hmac_asoc_rsp_stru *asoc_rsp, uint8_t *mac_hdr,
    uint16_t msg_len, hmac_user_stru *hmac_user_ap)
{
    asoc_rsp->en_result_code = HMAC_MGMT_SUCCESS;
    asoc_rsp->en_status_code = MAC_SUCCESSFUL_STATUSCODE;

    /* 记录关联响应帧的部分内容，用于上报给内核 */
#ifdef _PRE_WINDOWS_SUPPORT
    asoc_rsp->asoc_rsp_ie_len = msg_len; /* windows产品不偏移fixed paramenters的6字节 */
#else
    asoc_rsp->asoc_rsp_ie_len = msg_len - OAL_ASSOC_RSP_FIXED_OFFSET; /* 除去MAC帧头24字节和FIXED部分6字节 */
#endif
    asoc_rsp->puc_asoc_rsp_ie_buff = mac_hdr + OAL_ASSOC_RSP_IE_OFFSET;

    /* 获取AP的mac地址 */
    mac_get_bssid(mac_hdr, asoc_rsp->auc_addr_ap, WLAN_MAC_ADDR_LEN);

    /* 获取关联请求帧信息 */
    asoc_rsp->puc_asoc_req_ie_buff = hmac_user_ap->puc_assoc_req_ie_buff;
    asoc_rsp->asoc_req_ie_len = hmac_user_ap->assoc_req_ie_len;
}

static void hmac_sta_wait_asoc_rx_rpt_user_chan_check(mac_vap_stru *mac_vap, uint32_t *ret,
    hmac_vap_stru *hmac_sta, uint16_t user_idx)
{
    uint32_t rslt;
    mac_cfg_user_info_param_stru st_hmac_user_info_event;
    uint8_t uc_ap_follow_channel = 0;

    st_hmac_user_info_event.us_user_idx = user_idx;

    hmac_config_vap_info(mac_vap, sizeof(uint32_t), (uint8_t *)ret);
    hmac_config_user_info(mac_vap, sizeof(mac_cfg_user_info_param_stru), (uint8_t *)&st_hmac_user_info_event);

    if (OAL_SUCC != oal_notice_sta_join_result(hmac_sta->st_vap_base_info.uc_chip_id, OAL_TRUE)) {
        oam_warning_log0(hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_COEX,
                         "{hmac_sta_wait_asoc_rx_rpt_user_chan_check::oal_notice_sta_join_result fail.}");
    }

    /* 信道跟随检查 */
    rslt = hmac_check_ap_channel_follow_sta(mac_vap, &mac_vap->st_channel, &uc_ap_follow_channel);
    if (rslt == OAL_SUCC) {
        oam_warning_log1(hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
            "{hmac_sta_wait_asoc_rx_rpt_user_chan_check::after hmac_check_ap_channel_follow_sta. ap chan change to %d}",
            uc_ap_follow_channel);
    }
}

static uint32_t hmac_sta_wait_asoc_rx_get_user(hmac_vap_stru *hmac_sta, uint8_t *mac_hdr,
    uint8_t *addr_sa, uint16_t *user_idx, hmac_user_stru **hmac_user_ap)
{
    uint32_t rslt;

    /* 获取SA 地址 */
    mac_get_address2(mac_hdr, addr_sa, WLAN_MAC_ADDR_LEN);

    /* 根据SA 找到对应AP USER结构 */
    rslt = mac_vap_find_user_by_macaddr(&(hmac_sta->st_vap_base_info), addr_sa, user_idx);
    if (rslt != OAL_SUCC) {
        oam_warning_log1(hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_sta_wait_asoc_rx:: mac_vap_find_user_by_macaddr failed[%d].}", rslt);
        return rslt;
    }

    /* 获取STA关联的AP的用户指针 */
    *hmac_user_ap = mac_res_get_hmac_user(*user_idx);
    if (*hmac_user_ap == NULL) {
        oam_warning_log1(hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                         "{hmac_sta_wait_asoc_rx::pst_hmac_user_ap[%d] null.}", *user_idx);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

static void hmac_sta_wait_asoc_rx_succ_handle(hmac_user_stru *hmac_user_ap,
    hmac_vap_stru *hmac_sta, uint8_t *addr_sa, uint16_t user_idx)
{
    /* 关联上用户之后，初始化黑名单方案 */
    hmac_btcoex_blacklist_handle_init(hmac_user_ap);
    hmac_sta_wait_asoc_rx_set_ba_allowed(hmac_sta, hmac_user_ap, addr_sa);

    /* STA切换到UP状态 */
    hmac_fsm_change_state(hmac_sta, MAC_VAP_STATE_UP);

    /* user已经关联上，抛事件给DMAC，在DMAC层挂用户算法钩子 */
    hmac_user_add_notify_alg(&(hmac_sta->st_vap_base_info), user_idx);

    hmac_roam_info_init(hmac_sta);
}

uint32_t hmac_sta_wait_asoc_rx(hmac_vap_stru *pst_hmac_sta, void *pst_msg)
{
    dmac_wlan_crx_event_stru *pst_mgmt_rx_event = NULL;
    dmac_rx_ctl_stru *pst_rx_ctrl = NULL;
    mac_rx_ctl_stru *pst_rx_info = NULL;
    uint8_t *puc_mac_hdr = NULL;
    uint8_t *puc_payload = NULL;
    uint16_t us_msg_len, us_hdr_len, us_user_idx;
    hmac_asoc_rsp_stru st_asoc_rsp;
    uint8_t auc_addr_sa[WLAN_MAC_ADDR_LEN];
    uint32_t rslt, ret;
    hmac_user_stru *pst_hmac_user_ap = NULL;
    mac_vap_stru *pst_mac_vap = NULL;

    if (oal_any_null_ptr2(pst_hmac_sta, pst_msg)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_sta_wait_asoc_rx::hmac_sta or msg is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &(pst_hmac_sta->st_vap_base_info);

    pst_mgmt_rx_event = (dmac_wlan_crx_event_stru *)pst_msg;
    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_mgmt_rx_event->pst_netbuf);
    pst_rx_info = (mac_rx_ctl_stru *)(&(pst_rx_ctrl->st_rx_info));
    puc_mac_hdr = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_info);
    puc_payload = (uint8_t *)(puc_mac_hdr) + pst_rx_info->uc_mac_header_len;
    us_msg_len = pst_rx_info->us_frame_len - pst_rx_info->uc_mac_header_len; /* 消息总长度,不包括FCS */
    us_hdr_len = pst_rx_info->uc_mac_header_len;

    if (hmac_sta_wait_asoc_rx_judge_frame_subtype(pst_hmac_sta, puc_mac_hdr, pst_mgmt_rx_event) != OAL_SUCC) {
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_SNIFFER
#ifdef CONFIG_HW_SNIFFER
    proc_sniffer_write_file(NULL, 0, puc_mac_hdr, pst_rx_info->us_frame_len, 0);
#endif
#endif
    if (hmac_sta_wait_asoc_rx_frame_para_check(pst_hmac_sta, puc_payload, us_msg_len) != OAL_SUCC) {
        return OAL_FAIL;
    }
    rslt = hmac_sta_wait_asoc_rx_get_user(pst_hmac_sta, puc_mac_hdr, auc_addr_sa, &us_user_idx, &pst_hmac_user_ap);
    if (rslt != OAL_SUCC) {
        return rslt;
    }

    /* 取消定时器 */
    frw_timer_immediate_destroy_timer_m(&(pst_hmac_sta->st_mgmt_timer));

    ret = hmac_process_assoc_rsp(pst_hmac_sta, pst_hmac_user_ap, puc_mac_hdr, us_hdr_len, puc_payload, us_msg_len);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_sta_wait_asoc_rx::hmac_process_assoc_rsp failed[%d].}", ret);
        return ret;
    }
    hmac_sta_wait_asoc_rx_succ_handle(pst_hmac_user_ap, pst_hmac_sta, auc_addr_sa, us_user_idx);

    /* 将用户(AP)在本地的状态信息设置为已关联状态 */
    /* 准备消息，上报给APP */
    hmac_sta_wait_asoc_rx_cfg_rsp_para(&st_asoc_rsp, puc_mac_hdr, us_msg_len, pst_hmac_user_ap);
    hmac_send_rsp_to_sme_sta(pst_hmac_sta, HMAC_SME_ASOC_RSP, (uint8_t *)(&st_asoc_rsp));

    /* 1102 STA 入网后，上报VAP 信息和用户信息 */
    hmac_sta_wait_asoc_rx_rpt_user_chan_check(pst_mac_vap, &ret, pst_hmac_sta, us_user_idx);
    // will not do any sync if proxysta enabled
    // allow running DBAC on different channels of same band while P2P defined
    return OAL_SUCC;
}

uint32_t hmac_sta_get_mngpkt_sendstat(mac_vap_stru *pst_mac_vap, mac_cfg_query_mngpkt_sendstat_stru *pst_sendstat_info)
{
    mac_device_stru *pst_mac_dev = NULL;

    if (pst_sendstat_info == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_sta_get_mngpkt_sendstat::pst_sendstat_info null.}");
        return OAL_FAIL;
    }

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_sta_get_mngpkt_sendstat::mac_res_get_dev failed.}");
        return OAL_FAIL;
    }

    pst_sendstat_info->uc_auth_req_st = pst_mac_dev->uc_auth_req_sendst;
    pst_sendstat_info->uc_asoc_req_st = pst_mac_dev->uc_asoc_req_sendst;

    return OAL_SUCC;
}

void hmac_sta_clear_auth_req_sendstat(mac_vap_stru *pst_mac_vap)
{
    mac_device_stru *pst_mac_dev;

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_sta_clear_auth_req_sendstat::mac_res_get_dev failed.}");
        return;
    }

    pst_mac_dev->uc_auth_req_sendst = 0;
}

void hmac_sta_clear_asoc_req_sendstat(mac_vap_stru *pst_mac_vap)
{
    mac_device_stru *pst_mac_dev;

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_sta_clear_asoc_req_sendstat::mac_res_get_dev failed.}");
        return;
    }

    pst_mac_dev->uc_asoc_req_sendst = 0;
}


uint32_t hmac_sta_auth_timeout(hmac_vap_stru *pst_hmac_sta, void *p_param)
{
    hmac_auth_rsp_stru st_auth_rsp = {
        {
            0,
        },
    };
    mac_cfg_query_mngpkt_sendstat_stru st_mngpkt_sendstat_info;

    /* and send it to the host.                                          */
    if (MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2 == pst_hmac_sta->st_vap_base_info.en_vap_state) {
        st_auth_rsp.us_status_code = MAC_AUTH_RSP2_TIMEOUT;
    } else if (MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4 == pst_hmac_sta->st_vap_base_info.en_vap_state) {
        st_auth_rsp.us_status_code = MAC_AUTH_RSP4_TIMEOUT;
    } else {
        st_auth_rsp.us_status_code = HMAC_MGMT_TIMEOUT;
    }

    if (OAL_SUCC == hmac_sta_get_mngpkt_sendstat(&pst_hmac_sta->st_vap_base_info, &st_mngpkt_sendstat_info)) {
        if (st_mngpkt_sendstat_info.uc_auth_req_st > 0) {
            st_auth_rsp.us_status_code = MAC_AUTH_REQ_SEND_FAIL_BEGIN + st_mngpkt_sendstat_info.uc_auth_req_st;
            hmac_sta_clear_auth_req_sendstat(&pst_hmac_sta->st_vap_base_info);
        }
    }

    /* Send the response to host now. */
    hmac_send_rsp_to_sme_sta(pst_hmac_sta, HMAC_SME_AUTH_RSP, (uint8_t *)&st_auth_rsp);

    return OAL_SUCC;
}


uint32_t hmac_get_phy_rate_and_protocol(uint8_t pre_rate, wlan_protocol_enum_uint8 *protocol, uint8_t *rate_idx)
{
    uint32_t uc_idx;

    for (uc_idx = 0; uc_idx < WLAN_11B_BUTT; uc_idx++) {
        if (g_st_data_11b_rates[uc_idx].uc_expand_rate == pre_rate ||
            g_st_data_11b_rates[uc_idx].uc_mac_rate == pre_rate) {
            *rate_idx = uc_idx;
            *protocol = 0; /* 11b 协议速率 */
            return OAL_SUCC;
        }
    }

    for (uc_idx = 0; uc_idx < WLAN_11AG_BUTT; uc_idx++) {
        if (g_st_data_legacy_ofdm_rates[uc_idx].uc_expand_rate == pre_rate ||
            g_st_data_legacy_ofdm_rates[uc_idx].uc_mac_rate == pre_rate) {
            *rate_idx = uc_idx;
            *protocol = 1; /* 11ag 协议速率 */
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}


void hmac_sta_get_min_rate(dmac_set_rate_stru *pst_rate_params, hmac_join_req_stru *pst_join_req)
{
    uint32_t uc_idx;
    uint8_t auc_min_rate_idx[2] = { 0 }; /* 第一个存储11b协议对应的速率，第二个存储11ag协议对应的速率 */
    uint8_t uc_min_rate_idx = 0;
    uint8_t uc_protocol = 0; /* 表示基础速率级的协议，0:11b  1:legacy ofdm */

    if (oal_any_null_ptr2(pst_rate_params, pst_join_req)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_sta_get_min_rate::NULL param}");
        return;
    }

    memset_s(pst_rate_params, sizeof(dmac_set_rate_stru), 0, sizeof(dmac_set_rate_stru));

    // get min rate
    for (uc_idx = 0; uc_idx < pst_join_req->st_bss_dscr.uc_num_supp_rates; uc_idx++) {
        if (hmac_get_phy_rate_and_protocol(pst_join_req->st_bss_dscr.auc_supp_rates[uc_idx], &uc_protocol,
            &uc_min_rate_idx) != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_SCAN, "{hmac_sta_get_min_rate::hmac_get_rate_protocol failed (%d).[%d]}",
                           uc_min_rate_idx, uc_idx);
            continue;
        }

        /* 根据枚举wlan_phy_protocol_enum 填写数组对应位置的值 */
        if (pst_rate_params->un_capable_flag.uc_value & ((uint32_t)1 << (uc_protocol))) {
            if (auc_min_rate_idx[uc_protocol] > uc_min_rate_idx) {
                auc_min_rate_idx[uc_protocol] = uc_min_rate_idx;
            }
        } else {
            auc_min_rate_idx[uc_protocol] = uc_min_rate_idx;
            pst_rate_params->un_capable_flag.uc_value |= ((uint32_t)1 << (uc_protocol));
        }

        if (uc_min_rate_idx == 0x08) { // st_data_rates第八个是24M
            pst_rate_params->un_capable_flag.st_capable.bit_ht_capable = OAL_TRUE;
            pst_rate_params->un_capable_flag.st_capable.bit_vht_capable = OAL_TRUE;
        }
    }

    /* 与储存在扫描结果描述符中的能力进行比较，看能力是否匹配 */
    pst_rate_params->un_capable_flag.st_capable.bit_ht_capable &= pst_join_req->st_bss_dscr.en_ht_capable;
    pst_rate_params->un_capable_flag.st_capable.bit_vht_capable &= pst_join_req->st_bss_dscr.en_vht_capable;
    pst_rate_params->uc_min_rate[0] = g_st_data_11b_rates[auc_min_rate_idx[0]].uc_hal_wlan_rate_index;
    pst_rate_params->uc_min_rate[1] = g_st_data_legacy_ofdm_rates[auc_min_rate_idx[1]].uc_hal_wlan_rate_index;

    oam_warning_log4(0, OAM_SF_ASSOC,
                     "{hmac_sta_get_min_rate:: uc_min_rate_idx[%d]un_capable_flag.uc_value[%d]legacy rate[%d][%d]}",
                     uc_min_rate_idx, pst_rate_params->un_capable_flag.uc_value,
                     pst_rate_params->uc_min_rate[0], pst_rate_params->uc_min_rate[1]);

    return;
}

#ifdef _PRE_WLAN_FEATURE_M2S
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_sta_support_opmode_rules(hmac_join_req_stru *pst_join_req)
{
    
    /* 双流及其以下，三四流ht设备都配置为不支持opmode */
    return ((pst_join_req->st_bss_dscr.en_support_max_nss == WLAN_SINGLE_NSS) ||
            (pst_join_req->st_bss_dscr.en_vht_capable == OAL_FALSE) ||
            (pst_join_req->st_bss_dscr.en_support_max_nss == WLAN_DOUBLE_NSS &&
             pst_join_req->st_bss_dscr.en_support_opmode == OAL_FALSE));
}
#endif


void hmac_sta_update_join_req_params_for_roam(mac_vap_stru *pst_mac_vap, hmac_join_req_stru *join_req,
                                              mac_ap_type_enum_uint16 *pen_ap_type)
{
    hmac_user_stru *pst_hmac_user = NULL;

    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING) {
        pst_hmac_user = mac_res_get_hmac_user(pst_mac_vap->us_assoc_vap_id);
        if (pst_hmac_user != NULL) {
            *pen_ap_type = hmac_identify_type_of_ap_with_compatibility_issues(pst_mac_vap, pst_mac_vap->auc_bssid);

            oam_warning_log1(0, OAM_SF_TX,
                             "{hmac_sta_update_join_req_params_for_roam::ROAM vap en_ap_type[%d].}\r\n",
                             *pen_ap_type);

            pst_hmac_user->en_user_ap_type = *pen_ap_type; /* AP类型 */
            pst_hmac_user->sap_mode = join_req->st_bss_dscr.sap_mode;
        }
        hmac_roam_down_dc_p2p_group();
    }
}

OAL_STATIC void hmac_sta_update_join_bw(mac_vap_stru *mac_vap,  hmac_join_req_stru *p_join_req)
{
    /* 更新频带、主20MHz信道号，与AP通信 DMAC切换信道时直接调用 */
    mac_vap->st_channel.en_bandwidth = mac_vap_get_bandwith(mac_mib_get_dot11VapMaxBandWidth(mac_vap),
        p_join_req->st_bss_dscr.en_channel_bandwidth);

    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_SCAN, "hmac_sta_update_join_bw::bss_info:he_cap=%d,vht_cap=%d \
        p2p_scenes=%d, bw=%d",
        p_join_req->st_bss_dscr.en_he_capable, p_join_req->st_bss_dscr.en_vht_capable,
        p_join_req->st_bss_dscr.en_p2p_scenes, p_join_req->st_bss_dscr.en_channel_bandwidth);

    /* 5G 回退到40M */
    if (p_join_req->st_bss_dscr.en_p2p_scenes == MAC_P2P_SCENES_LOW_LATENCY &&
        mac_vap_p2p_bw_back_to_40m(mac_vap, mac_vap->st_channel.uc_chan_number, &mac_vap->st_channel.en_bandwidth) ==
        OAL_TRUE) {
        p_join_req->st_bss_dscr.en_channel_bandwidth = mac_vap->st_channel.en_bandwidth;
        oam_warning_log1(mac_vap->uc_vap_id, 0, "{hmac_sta_update_join_bw::change bss_bw =%d",
            p_join_req->st_bss_dscr.en_channel_bandwidth);
    }
    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_SCAN, "hmac_sta_update_join_bw::dot11VapMaxBandWidth=%d, \
        bss_bw=%d, vap_bw=%d", mac_mib_get_dot11VapMaxBandWidth(mac_vap),
        p_join_req->st_bss_dscr.en_channel_bandwidth, mac_vap->st_channel.en_bandwidth);
}


/* wlan0入网或者漫游时检查是否需要断开wlan1 */
OAL_STATIC uint32_t hmac_dual_sta_join_check(hmac_vap_stru *hmac_vap, hmac_join_req_stru *join_req)
{
    int32_t need_disconnect;
    mac_vap_stru *other_vap = NULL;
    mac_vap_stru *mac_vap = &hmac_vap->st_vap_base_info;
    mac_cfg_kick_user_param_stru kick_user_param;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    if (!mac_is_dual_sta_mode() || !mac_is_primary_legacy_sta(mac_vap)) {
        return OAL_SUCC;
    }

    other_vap = mac_vap_find_another_up_vap_by_mac_vap(mac_vap);
    if (other_vap == NULL) {
        return OAL_SUCC;
    }

    if (!mac_is_secondary_sta(other_vap)) {
        oam_error_log2(other_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_dual_sta_join_check::"
            "unexpect vap, vap mode[%d], p2p mode[%d]}", other_vap->en_vap_mode, other_vap->en_p2p_mode);
        return OAL_SUCC;
    }

    if (mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING) {
        if (hmac_roam_pri_sta_join_check(hmac_vap, &join_req->st_bss_dscr, other_vap) == OAL_SUCC) {
            need_disconnect = OAL_FALSE;
        } else {
            need_disconnect = OAL_TRUE;
        }
    } else {
        /* wlan0入网时, wlan1已关联。需要先断开wlan1。 */
        need_disconnect = OAL_TRUE;
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC,
            "{hmac_dual_sta_join_check::sec sta is connected before pri sta, disconnect sec sta!}");
    }

    if (need_disconnect) {
        kick_user_param.us_reason_code = MAC_UNSPEC_REASON;
        memcpy_s(kick_user_param.auc_mac_addr, WLAN_MAC_ADDR_LEN, auc_mac_addr, WLAN_MAC_ADDR_LEN);
        if (hmac_config_kick_user(other_vap, sizeof(kick_user_param),
            (uint8_t *)&kick_user_param) != OAL_SUCC) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                "{hmac_dual_sta_join_check::secondary sta disconnect failed!");
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}
OAL_STATIC OAL_INLINE void hmac_sta_set_mode_params(mac_cfg_mode_param_stru *cfg_mode,
    wlan_protocol_enum_uint8 protocol, wlan_channel_band_enum_uint8 band, uint8_t channel_idx,
    wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    cfg_mode->en_protocol = protocol;
    cfg_mode->en_band = band;
    cfg_mode->en_bandwidth = bandwidth;
    cfg_mode->en_channel_idx = channel_idx;
}

#ifdef _PRE_WLAN_FEATURE_11D
/*
 * 函 数 名  : hmac_sta_update_desired_country_info
 * 功能描述  : 更新sta期望加入的国家字符串
 */
OAL_STATIC OAL_INLINE void hmac_sta_update_desired_country_info(hmac_vap_stru *pst_hmac_vap,
    hmac_join_req_stru *pst_join_req)
{
    /* 更新sta期望加入的国家字符串 */
    pst_hmac_vap->ac_desired_country[0] = pst_join_req->st_bss_dscr.ac_country[0];
    pst_hmac_vap->ac_desired_country[1] = pst_join_req->st_bss_dscr.ac_country[1];
    pst_hmac_vap->ac_desired_country[BYTE_OFFSET_2] = pst_join_req->st_bss_dscr.ac_country[BYTE_OFFSET_2];
}
#endif

static uint32_t hmac_sta_update_mib_cap_off_cfg_protocol(hmac_vap_stru *pst_hmac_vap,
    hmac_join_req_stru *pst_join_req, mac_cfg_mode_param_stru *pst_cfg_mode)
{
    mac_vap_stru *pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    uint32_t ret;

    memset_s(pst_cfg_mode, sizeof(mac_cfg_mode_param_stru), 0, sizeof(mac_cfg_mode_param_stru));

    mac_mib_set_HighThroughputOptionImplemented(pst_mac_vap, pst_join_req->st_bss_dscr.en_ht_capable);
    mac_mib_set_VHTOptionImplemented(pst_mac_vap, pst_join_req->st_bss_dscr.en_vht_capable);

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        hmac_sta_join_update_protocol_mib(pst_mac_vap, &pst_join_req->st_bss_dscr);
    }
#endif
    if (hmac_dual_sta_join_check(pst_hmac_vap, pst_join_req) != OAL_SUCC) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
            "{hmac_sta_update_join_req_params::dual sta check fail!}");
        return OAL_FAIL;
    }

    /* STA更新LDPC和STBC的能力,更新能力保存到Activated mib中 */
    mac_mib_set_LDPCCodingOptionActivated(pst_mac_vap, (oal_bool_enum_uint8)(pst_join_req->st_bss_dscr.en_ht_ldpc &
        mac_mib_get_LDPCCodingOptionImplemented(pst_mac_vap)));
    mac_mib_set_TxSTBCOptionActivated(pst_mac_vap, (oal_bool_enum_uint8)(pst_join_req->st_bss_dscr.en_ht_stbc &
        mac_mib_get_TxSTBCOptionImplemented(pst_mac_vap)));

    /* 关联2G AP，且2ght40禁止位为1时，不学习AP的HT 40能力 */
    if (!(pst_mac_vap->st_channel.en_band == WLAN_BAND_2G && pst_mac_vap->st_cap_flag.bit_disable_2ght40)) {
        if (pst_join_req->st_bss_dscr.en_bw_cap == WLAN_BW_CAP_20M) {
            mac_mib_set_FortyMHzOperationImplemented(pst_mac_vap, OAL_FALSE);
        } else {
            mac_mib_set_FortyMHzOperationImplemented(pst_mac_vap, OAL_TRUE);
        }
    }

    /* 根据要加入AP的协议模式更新STA侧速率集 */
    ret = hmac_sta_get_user_protocol_etc(&(pst_join_req->st_bss_dscr), &(pst_cfg_mode->en_protocol));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_SCAN,
                       "{hmac_sta_update_join_req_params::hmac_sta_get_user_protocol fail %d.}", ret);
        return ret;
    }

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_sta_update_join_req_params::mac_vap_pro[%d], \
        cfg_pro[%d]}", pst_mac_vap->en_protocol, pst_cfg_mode->en_protocol);

    pst_cfg_mode->en_band = pst_join_req->st_bss_dscr.st_channel.en_band;
    pst_cfg_mode->en_bandwidth = mac_vap_get_bandwith(mac_mib_get_dot11VapMaxBandWidth(pst_mac_vap),
        pst_join_req->st_bss_dscr.en_channel_bandwidth);
    pst_cfg_mode->en_channel_idx = pst_join_req->st_bss_dscr.st_channel.uc_chan_number;

    wlan_chip_update_cur_chn_cali_data(pst_mac_vap, WLAN_NORMAL_CHN);
    ret = hmac_config_sta_update_rates(pst_mac_vap, pst_cfg_mode, (void *)&pst_join_req->st_bss_dscr);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_SCAN,
                       "{hmac_sta_update_join_req_params::hmac_config_sta_update_rates fail %d.}", ret);
        return ret;
    }
    return OAL_SUCC;
}

static uint32_t hmac_sta_update_join_req_params_update_mib_cap(hmac_vap_stru *pst_hmac_vap,
    hmac_join_req_stru *pst_join_req, mac_cfg_mode_param_stru *pst_cfg_mode)
{
    mac_vap_stru *pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    uint32_t ret;
    uint8_t *puc_cur_ssid = NULL;

    /* 设置BSSID */
    mac_vap_set_bssid(pst_mac_vap, pst_join_req->st_bss_dscr.auc_bssid);

    /* 更新mib库对应的dot11BeaconPeriod值 */
    mac_mib_set_BeaconPeriod(pst_mac_vap, (uint32_t)(pst_join_req->st_bss_dscr.us_beacon_period));

    /* 更新mib库对应的ul_dot11CurrentChannel值 */
    mac_vap_set_current_channel(pst_mac_vap, pst_join_req->st_bss_dscr.st_channel.en_band,
        pst_join_req->st_bss_dscr.st_channel.uc_chan_number, pst_join_req->st_bss_dscr.st_channel.ext6g_band);

#ifdef _PRE_WLAN_FEATURE_11D
    hmac_sta_update_desired_country_info(pst_hmac_vap, pst_join_req);
#endif

    /* 更新mib库对应的ssid */
    puc_cur_ssid = mac_mib_get_DesiredSSID(pst_mac_vap);
    memcpy_s(puc_cur_ssid, WLAN_SSID_MAX_LEN, pst_join_req->st_bss_dscr.ac_ssid, WLAN_SSID_MAX_LEN);
    puc_cur_ssid[WLAN_SSID_MAX_LEN - 1] = '\0';

    /* 更新频带、主20MHz信道号，与AP通信 DMAC切换信道时直接调用 */
    pst_mac_vap->st_channel.uc_chan_number = pst_join_req->st_bss_dscr.st_channel.uc_chan_number;
    pst_mac_vap->st_channel.en_band = pst_join_req->st_bss_dscr.st_channel.en_band;
    pst_mac_vap->st_channel.ext6g_band = pst_join_req->st_bss_dscr.st_channel.ext6g_band;
    hmac_sta_update_join_bw(pst_mac_vap, pst_join_req);
    hmac_p2p_sta_join_go_back_to_80m_handle(pst_mac_vap);

    /* 在STA未配置协议模式情况下，根据要关联的AP，更新mib库中对应的HT/VHT能力 */
    if (pst_hmac_vap->bit_sta_protocol_cfg == OAL_SWITCH_OFF) {
        ret = hmac_sta_update_mib_cap_off_cfg_protocol(pst_hmac_vap, pst_join_req, pst_cfg_mode);
        if (ret != OAL_SUCC) {
            return ret;
        }
    }

    return OAL_SUCC;
}

static uint32_t hmac_sta_update_join_req_params_bw_chan(hmac_vap_stru *pst_hmac_vap,
    hmac_join_req_stru *pst_join_req)
{
    mac_vap_stru *pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    uint32_t ret;

    /* STA首先以20MHz运行，如果要切换到40 or 80MHz运行，需要满足一下条件: */
    /* (1) 用户支持40 or 80MHz运行 */
    /* (2) AP支持40 or 80MHz运行(HT Supported Channel Width Set = 1 && VHT Supported Channel Width Set = 0) */
    /* (3) AP在40 or 80MHz运行(SCO = SCA or SCB && VHT Channel Width = 1) */
    pst_mac_vap->st_channel.en_bandwidth = WLAN_BAND_WIDTH_20M;

    ret = mac_get_channel_idx_from_num(pst_mac_vap->st_channel.en_band, pst_mac_vap->st_channel.uc_chan_number,
        pst_mac_vap->st_channel.ext6g_band, &(pst_mac_vap->st_channel.uc_chan_idx));
    if (ret != OAL_SUCC) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
            "{hmac_sta_update_join_req_params::band and channel_num are not compatible.band[%d], channel_num[%d]}",
            pst_mac_vap->st_channel.en_band, pst_mac_vap->st_channel.uc_chan_number);
        return ret;
    }

    /* 更新协议相关信息，包括WMM P2P 11I 20/40M等 */
    hmac_update_join_req_params_prot_sta(pst_hmac_vap, pst_join_req);
    /* 入网优化，不同频段下的能力不一样 */
    if (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) {
        mac_mib_set_ShortPreambleOptionImplemented(pst_mac_vap, WLAN_LEGACY_11B_MIB_SHORT_PREAMBLE);
        mac_mib_set_SpectrumManagementRequired(pst_mac_vap, OAL_FALSE);
    } else {
        mac_mib_set_ShortPreambleOptionImplemented(pst_mac_vap, WLAN_LEGACY_11B_MIB_LONG_PREAMBLE);
        mac_mib_set_SpectrumManagementRequired(pst_mac_vap, OAL_TRUE);
    }

    /* 根据协议信道做带宽约束, 此时已经跟进join参数刷新好带宽和信道，此时直接根据当前信道再次刷新带宽 */
    hmac_sta_bandwidth_down_by_channel(pst_mac_vap);
    wlan_chip_update_cur_chn_cali_data(pst_mac_vap, WLAN_NORMAL_CHN);

    return OAL_SUCC;
}


static uint16_t hmac_sta_update_ap_tpye_degradation(mac_vap_stru *mac_vap,
    mac_bss_dscr_stru *bss_dscr)
{
    if (hmac_degradation_is_whitelist_type(bss_dscr->auc_mac_addr) == OAL_TRUE) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_M2S, "{hmac_sta_update_ap_tpye_mcm_degradation:: \
            AP is in whitelist, user_mac_%d:%02X:XX:XX:XX:%02X:%02X }", bss_dscr->auc_mac_addr[0],
            bss_dscr->auc_mac_addr[4], bss_dscr->auc_mac_addr[5]); /* 0,4,5为mac地址 */
        return MAC_AP_TYPE_MCM_DEGRADARION_WRITELIST;
    }
    return 0;
}

uint32_t hmac_sta_update_join_req_params_post_event(hmac_vap_stru *hmac_vap,
    hmac_join_req_stru *join_req, mac_ap_type_enum_uint16 ap_type)
{
    mac_vap_stru *mac_vap = &(hmac_vap->st_vap_base_info);
    dmac_ctx_join_req_set_reg_stru *reg_params = NULL;
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;

    /* 抛事件到DMAC, 申请事件内存 */
    event_mem = frw_event_alloc_m(sizeof(dmac_ctx_join_req_set_reg_stru));
    if (event_mem == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_sta_update_join_req_params::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_JOIN_SET_REG,
        sizeof(dmac_ctx_join_req_set_reg_stru), FRW_EVENT_PIPELINE_STAGE_1, hmac_vap->st_vap_base_info.uc_chip_id,
        hmac_vap->st_vap_base_info.uc_device_id, hmac_vap->st_vap_base_info.uc_vap_id);

    reg_params = (dmac_ctx_join_req_set_reg_stru *)event->auc_event_data;

    /* 设置需要写入寄存器的BSSID信息 */
    oal_set_mac_addr(reg_params->auc_bssid, join_req->st_bss_dscr.auc_bssid);

    /* 填写信道相关信息 */
    memcpy_s(&reg_params->st_current_channel, sizeof(mac_channel_stru),
        &mac_vap->st_channel, sizeof(mac_channel_stru));

    /* 填写速率相关信息 */
    hmac_sta_get_min_rate(&reg_params->st_min_rate, join_req);

    /* 设置beaocn period信息 */
    reg_params->us_beacon_period = (join_req->st_bss_dscr.us_beacon_period);

    /* 同步FortyMHzOperationImplemented */
    reg_params->en_dot11FortyMHzOperationImplemented = mac_mib_get_FortyMHzOperationImplemented(mac_vap);

    /* 设置beacon filter关闭 */
    reg_params->beacon_filter = OAL_FALSE;

    /* 设置no frame filter打开 */
    reg_params->non_frame_filter = OAL_TRUE;

    /* 保存黑名单类型 */
    reg_params->en_ap_type = ap_type | hmac_sta_update_ap_tpye_degradation(mac_vap, &join_req->st_bss_dscr);

    /* 保存AP支持的最大空间流数 */
    reg_params->ap_support_max_nss = join_req->st_bss_dscr.en_support_max_nss;

    reg_params->sap_mode = join_req->st_bss_dscr.sap_mode;

    /* 下发ssid */
    memcpy_s(reg_params->auc_ssid, WLAN_SSID_MAX_LEN, join_req->st_bss_dscr.ac_ssid, WLAN_SSID_MAX_LEN);
    reg_params->auc_ssid[WLAN_SSID_MAX_LEN - 1] = '\0';

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        reg_params->set_he_cap = mac_mib_get_HEOptionImplemented(mac_vap);
        hmac_sta_update_join_multi_bssid_info(mac_vap, reg_params, &join_req->st_bss_dscr.st_mbssid_info);
    }
#endif
    reg_params->set_vht_cap = mac_mib_get_VHTOptionImplemented(mac_vap);
    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);

    return OAL_SUCC;
}


uint32_t hmac_sta_update_join_req_params(hmac_vap_stru *pst_hmac_vap, hmac_join_req_stru *pst_join_req)
{
    mac_vap_stru *pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    uint32_t ret;
    mac_device_stru *pst_mac_device = NULL;
    wlan_mib_ieee802dot11_stru *pst_mib_info;
    mac_cfg_mode_param_stru st_cfg_mode;
    mac_ap_type_enum_uint16 en_ap_type = 0;

    pst_mib_info = pst_mac_vap->pst_mib_info;
    if (pst_mib_info == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }
    ret = hmac_sta_update_join_req_params_update_mib_cap(pst_hmac_vap, pst_join_req, &st_cfg_mode);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 有些加密协议只能工作在legacy */
    hmac_sta_protocol_down_by_chipher(pst_mac_vap, &pst_join_req->st_bss_dscr);
    hmac_sta_set_mode_params(&st_cfg_mode, pst_mac_vap->en_protocol, pst_mac_vap->st_channel.en_band,
        pst_join_req->st_bss_dscr.st_channel.uc_chan_number, pst_mac_vap->st_channel.en_bandwidth);

    hmac_config_sta_update_rates(pst_mac_vap, &st_cfg_mode, (void *)&pst_join_req->st_bss_dscr);
    ret = hmac_sta_update_join_req_params_bw_chan(pst_hmac_vap, pst_join_req);
    if (ret != OAL_SUCC) {
        return ret;
    }
    if (hmac_calc_up_ap_num(pst_mac_device) == 0) {
        pst_mac_device->uc_max_channel = pst_mac_vap->st_channel.uc_chan_number;
        pst_mac_device->en_max_band = pst_mac_vap->st_channel.en_band;
        pst_mac_device->en_max_bandwidth = pst_mac_vap->st_channel.en_bandwidth;
    }

    hmac_sta_update_join_req_params_for_roam(pst_mac_vap, pst_join_req, &en_ap_type);
    ret = hmac_sta_update_join_req_params_post_event(pst_hmac_vap, pst_join_req, en_ap_type);
    if (ret != OAL_SUCC) {
        return ret;
    }
#ifdef _PRE_WLAN_FEATURE_M2S
    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                     "{hmac_sta_update_join_req_params::ap nss[%d],ap band[%d],ap ht cap[%d],ap vht cap[%d].}",
                     pst_join_req->st_bss_dscr.en_support_max_nss, pst_join_req->st_bss_dscr.st_channel.en_band,
                     pst_join_req->st_bss_dscr.en_ht_capable, pst_join_req->st_bss_dscr.en_vht_capable);

    pst_mac_vap->st_cap_flag.bit_opmode = !hmac_sta_support_opmode_rules(pst_join_req);

    /* 同步vap修改信息到device侧 */
    hmac_config_vap_m2s_info_syn(pst_mac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_PMF
    hmac_sta_roam_update_pmf(pst_hmac_vap, &pst_join_req->st_bss_dscr);
#endif

    return OAL_SUCC;
}


uint32_t hmac_sta_wait_asoc_timeout(hmac_vap_stru *pst_hmac_sta, void *p_param)
{
    hmac_asoc_rsp_stru st_asoc_rsp = { 0 };
    hmac_mgmt_timeout_param_stru *pst_timeout_param = NULL;
    mac_cfg_query_mngpkt_sendstat_stru st_mngpkt_sendstat_info;

    if (oal_any_null_ptr2(pst_hmac_sta, p_param)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_sta_wait_asoc_timeout::hmac_sta or param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_timeout_param = (hmac_mgmt_timeout_param_stru *)p_param;

    /* 填写关联结果 */
    st_asoc_rsp.en_result_code = HMAC_MGMT_TIMEOUT;

    /* 关联超时失败,原因码上报wpa_supplicant */
    st_asoc_rsp.en_status_code = pst_timeout_param->en_status_code;

    if (OAL_SUCC == hmac_sta_get_mngpkt_sendstat(&pst_hmac_sta->st_vap_base_info, &st_mngpkt_sendstat_info)) {
        if (st_mngpkt_sendstat_info.uc_asoc_req_st > 0 && st_asoc_rsp.en_status_code == MAC_ASOC_RSP_TIMEOUT) {
            st_asoc_rsp.en_status_code = MAC_ASOC_REQ_SEND_FAIL_BEGIN + st_mngpkt_sendstat_info.uc_asoc_req_st;
            hmac_sta_clear_asoc_req_sendstat(&pst_hmac_sta->st_vap_base_info);
        }
    }

    /* 发送关联结果给SME */
    hmac_send_rsp_to_sme_sta(pst_hmac_sta, HMAC_SME_ASOC_RSP, (uint8_t *)&st_asoc_rsp);

    return OAL_SUCC;
}

void hmac_mgmt_sta_roam_rx_action(hmac_vap_stru *p_hmac_vap, oal_netbuf_stru *netbuf,
    oal_bool_enum_uint8 isProtected)
{
    dmac_rx_ctl_stru *rx_ctrl = NULL;
    uint8_t *data = NULL;
    hmac_user_stru *p_hmac_user = NULL;
    mac_ieee80211_frame_stru *p_mac_hdr = NULL;

    if (p_hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_ROAMING) {
        return;
    }

    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    p_mac_hdr = (mac_ieee80211_frame_stru *)MAC_GET_RX_CB_MAC_HEADER_ADDR(&rx_ctrl->st_rx_info);
    data = MAC_GET_RX_PAYLOAD_ADDR(&rx_ctrl->st_rx_info, netbuf);

    if (hmac_get_frame_body_len(netbuf) < MAC_ACTION_CATEGORY_AND_CODE_LEN) {
        oam_warning_log1(0, OAM_SF_RX, "{sta_roam::frame len too short[%d].}", rx_ctrl->st_rx_info.us_frame_len);
        return;
    }

    p_hmac_user = mac_res_get_hmac_user(p_hmac_vap->st_vap_base_info.us_assoc_vap_id);
    if (p_hmac_user == NULL) {
        return;
    }
    if (oal_compare_mac_addr(p_hmac_user->st_user_base_info.auc_user_mac_addr, p_mac_hdr->auc_address3)) {
        return;
    }

    switch (data[MAC_ACTION_OFFSET_CATEGORY]) {
#ifdef _PRE_WLAN_FEATURE_PMF
        case MAC_ACTION_CATEGORY_SA_QUERY: {
            hmac_mgmt_sta_roam_rx_sa_query(p_hmac_vap, netbuf, data, isProtected);
            break;
        }
#endif
        default:
            break;
    }
}
