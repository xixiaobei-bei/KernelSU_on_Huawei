

#include "hmac_11k.h"
#include "mac_frame_inl.h"
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_FTM)

void hmac_scan_update_bss_list_rrm(mac_bss_dscr_stru *bss_dscr,
                                   uint8_t *frame_body,
                                   uint16_t frame_len)
{
    uint8_t *ie;
    oal_rrm_enabled_cap_ie_stru *rrm_ie = NULL;
    ie = mac_find_ie(MAC_EID_RRM, frame_body, frame_len);
    bss_dscr->en_support_rrm = OAL_FALSE;
    bss_dscr->en_support_neighbor = OAL_FALSE;
    if (ie != NULL) {
        bss_dscr->en_support_rrm = OAL_TRUE;
        if (ie[1] >= sizeof(oal_rrm_enabled_cap_ie_stru)) {
            rrm_ie = (oal_rrm_enabled_cap_ie_stru*)(ie + MAC_IE_HDR_LEN);
            bss_dscr->en_support_neighbor = (rrm_ie->bit_neighbor_rpt_cap == 1) ? OAL_TRUE : OAL_FALSE;
        }
    }
}
#endif
#if defined(_PRE_WLAN_FEATURE_11K)

static inline uint8_t snr_ant(uint8_t snr_ant0)
{
    /* RSNI=(信噪比 + 10) * 2 */
    return (((snr_ant0) >> 1) + 10) << 1;
}

static uint16_t hmac_set_netbuf_head_and_meas_report_tag_flag(uint8_t *data, mac_vap_stru *mac_vap,
                                                              mac_user_stru *mac_user,
                                                              mac_vap_rrm_trans_req_info_stru *trans_req_info)
{
    uint16_t len;
    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* All the fields of the Frame Control Field are set to zero. Only the   */
    /* Type/Subtype field is set.                                            */
    mac_hdr_set_frame_control(data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* duration */
    data[BYTE_OFFSET_2] = 0;
    data[BYTE_OFFSET_3] = 0;

    /* DA is address of STA requesting association */
    oal_set_mac_addr(data + BYTE_OFFSET_4, mac_user->auc_user_mac_addr);

    /* SA is the dot11MACAddress */
    oal_set_mac_addr(data + BYTE_OFFSET_10, mac_mib_get_StationID(mac_vap));

    oal_set_mac_addr(data + BYTE_OFFSET_16, mac_vap->auc_bssid);

    /* seq control */
    data[BYTE_OFFSET_22] = 0;
    data[BYTE_OFFSET_23] = 0;

    /*************************************************************************/
    /*                    Radio Measurement Report Frame - Frame Body        */
    /* --------------------------------------------------------------------- */
    /* |Category |Action |Dialog Token| Measurement Report Elements         | */
    /* --------------------------------------------------------------------- */
    /* |1        |1      | 1          |  var                                 */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    /* Initialize index and the frame data pointer */
    len = MAC_80211_FRAME_LEN;

    /* Category */
    data[len++] = MAC_ACTION_CATEGORY_RADIO_MEASURMENT;

    /* Action */
    data[len++] = MAC_RM_ACTION_RADIO_MEASUREMENT_REPORT;

    /* Dialog Token  */
    data[len++] = trans_req_info->uc_action_dialog_token;
    return len;
}

static mac_bcn_rpt_stru *hmac_set_meas_report_ie(mac_meas_rpt_ie_stru *meas_rpt_ie, hmac_vap_stru *hmac_vap,
                                                 hmac_scanned_bss_info *scanned_bss,
                                                 mac_vap_rrm_trans_req_info_stru *trans_req_info)
{
    uint8_t chain_index;
    uint8_t snr = 0;
    mac_bcn_rpt_stru *bcn_rpt = NULL;

    /*************************************************************************/
    /*                   Measurement Report IE - Frame Body                  */
    /* --------------------------------------------------------------------- */
    /* |Element ID |Length |Meas Token| Meas Rpt Mode | Meas Type | Meas Rpt| */
    /* --------------------------------------------------------------------- */
    /* |1          |1      | 1        |  1            | 1         | var      */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    meas_rpt_ie->uc_eid = MAC_EID_MEASREP;
    meas_rpt_ie->uc_len = HMAC_MEASUREMENT_RPT_FIX_LEN - MAC_IE_HDR_LEN + HMAC_BEACON_RPT_FIX_LEN;
    meas_rpt_ie->uc_token = trans_req_info->uc_meas_token;
    memset_s(&(meas_rpt_ie->st_rptmode),
             sizeof(mac_meas_rpt_mode_stru), 0,
             sizeof(mac_meas_rpt_mode_stru));
    meas_rpt_ie->uc_rpttype = RM_RADIO_MEAS_BCN;

    bcn_rpt = (mac_bcn_rpt_stru *)meas_rpt_ie->auc_meas_rpt;
    memset_s(bcn_rpt, sizeof(mac_bcn_rpt_stru), 0, sizeof(mac_bcn_rpt_stru));
    memcpy_s(bcn_rpt->auc_bssid, WLAN_MAC_ADDR_LEN,
             scanned_bss->st_bss_dscr_info.auc_bssid, WLAN_MAC_ADDR_LEN);
    bcn_rpt->uc_channum = scanned_bss->st_bss_dscr_info.st_channel.uc_chan_number;
    if (hmac_vap->bit_11k_auth_oper_class != 0) {
        bcn_rpt->uc_optclass = hmac_vap->bit_11k_auth_oper_class;
    } else {
        bcn_rpt->uc_optclass = trans_req_info->uc_oper_class;
    }

    oam_warning_log4(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                     "{hmac_scan_rrm_proc_save_bss::In Channel [%d] Find BSS %02X:XX:XX:XX:%02X:%02X.}",
                     scanned_bss->st_bss_dscr_info.st_channel.uc_chan_number,
                     scanned_bss->st_bss_dscr_info.auc_bssid[MAC_ADDR_0],
                     scanned_bss->st_bss_dscr_info.auc_bssid[MAC_ADDR_4],
                     scanned_bss->st_bss_dscr_info.auc_bssid[MAC_ADDR_5]);

    bcn_rpt->uc_rcpi = (uint8_t)((uint32_t)(scanned_bss->st_bss_dscr_info.c_rssi + 110) << 1); /* bss的信号强度加110 */

    /* 获取信噪比  RSNI=(信噪比 + 10) * 2 */
    for (chain_index = 0; chain_index < HD_EVENT_RF_NUM; chain_index++) {
        if (scanned_bss->st_bss_dscr_info.snr[chain_index] != OAL_SNR_INIT_VALUE) {
            snr = snr > scanned_bss->st_bss_dscr_info.snr[chain_index] ?
                snr : scanned_bss->st_bss_dscr_info.snr[chain_index];
        }
    }
    if (snr == 0) {
        bcn_rpt->uc_rsni = 0xFF;
    } else {
        bcn_rpt->uc_rsni = snr_ant(snr);
    }
    return bcn_rpt;
}

static void hmac_scan_meas_rpt_data_set(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap, uint8_t *data,
                                        uint16_t *len, mac_vap_rrm_trans_req_info_stru *trans_req_info)
{
    mac_bcn_rpt_stru *bcn_rpt = NULL;
    hmac_bss_mgmt_stru *bss_mgmt = NULL;
    hmac_scanned_bss_info *scanned_bss = NULL;
    oal_dlist_head_stru *entry = NULL;
    mac_meas_rpt_ie_stru *meas_rpt_ie = NULL;
    uint8_t bss_idx = 0;

    /* 获取管理扫描的bss结果的结构体 */
    bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);
    meas_rpt_ie = (mac_meas_rpt_ie_stru *)(data + *len);

    /* 对链表删操作前加锁 */
    oal_spin_lock(&(bss_mgmt->st_lock));
    oal_dlist_search_for_each(entry, &(bss_mgmt->st_bss_list_head))
    {
        scanned_bss = oal_dlist_get_entry(entry, hmac_scanned_bss_info, st_dlist_head);
        /* BSSID过滤 */
        if (!ether_is_broadcast(trans_req_info->auc_bssid) &&
            OAL_MEMCMP(scanned_bss->st_bss_dscr_info.auc_bssid, trans_req_info->auc_bssid, WLAN_MAC_ADDR_LEN)) {
            continue;
        }

        /* SSID过滤，若请求中ssid长度为0，则不过滤 */
        if (trans_req_info->us_ssid_len != 0 &&
            OAL_MEMCMP(scanned_bss->st_bss_dscr_info.ac_ssid, trans_req_info->auc_ssid, trans_req_info->us_ssid_len)) {
            continue;
        }

        bcn_rpt = hmac_set_meas_report_ie(meas_rpt_ie, hmac_vap, scanned_bss, trans_req_info);

        *len += (HMAC_MEASUREMENT_RPT_FIX_LEN + HMAC_BEACON_RPT_FIX_LEN);
        bss_idx++;
        if ((WLAN_MEM_NETBUF_SIZE2 - *len) < (HMAC_MEASUREMENT_RPT_FIX_LEN + HMAC_BEACON_RPT_FIX_LEN)) {
            break;
        }

        meas_rpt_ie = (mac_meas_rpt_ie_stru *)bcn_rpt->auc_subelm;
    }
    oal_spin_unlock(&(bss_mgmt->st_lock));

    oam_warning_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                     "{hmac_scan_rrm_proc_save_bss::Find BssNum=[%d],us_len=[%d].buf_size=[%d].}",
                     bss_idx, *len, WLAN_MEM_NETBUF_SIZE2);
}

static uint32_t hmac_get_base_mac_res_stru(mac_vap_stru *mac_vap, mac_user_stru **mac_user,
                                           hmac_vap_stru **hmac_vap, hmac_device_stru **hmac_device)
{
    /* 获取hmac vap */
    *hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (*hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_scan_rrm_proc_save_bss::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (oal_unlikely(*hmac_device == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_scan_rrm_proc_save_bss::hmac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *mac_user = mac_res_get_mac_user(mac_vap->us_assoc_vap_id);
    if (*mac_user == NULL) {
        oam_warning_log1(0, OAM_SF_TX, "{hmac_scan_rrm_proc_save_bss::mac_user[%d] null.",
                         mac_vap->us_assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    return OAL_SUCC;
}

static uint32_t hmac_scan_set_tx_cb_and_post_event(mac_vap_stru *mac_vap, oal_netbuf_stru *action_table_bcn_rpt,
                                                   mac_user_stru *mac_user, uint16_t len)
{
    uint32_t ret;
    mac_tx_ctl_stru *tx_ctl;
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(action_table_bcn_rpt);
    MAC_GET_CB_MPDU_LEN(tx_ctl) = len;
    ret = mac_vap_set_cb_tx_user_idx(mac_vap, tx_ctl, mac_user->auc_user_mac_addr);
    if (ret != OAL_SUCC) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "(hmac_scan_rrm_proc_save_bss::fail to find user by xx:xx:xx:0x:0x:0x.}",
                         mac_user->auc_user_mac_addr[MAC_ADDR_3],
                         mac_user->auc_user_mac_addr[MAC_ADDR_4],
                         mac_user->auc_user_mac_addr[MAC_ADDR_5]);
    }

    if (MAC_GET_CB_MPDU_LEN(tx_ctl) > WLAN_MEM_NETBUF_SIZE2) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_scan_rrm_proc_save_bss::invalid us_len=[%d].}",
                         MAC_GET_CB_MPDU_LEN(tx_ctl));
        return OAL_FAIL;
    }
    oal_netbuf_put(action_table_bcn_rpt, MAC_GET_CB_MPDU_LEN(tx_ctl));

    ret = hmac_tx_mgmt_send_event(mac_vap, action_table_bcn_rpt, MAC_GET_CB_MPDU_LEN(tx_ctl));
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_scan_rrm_proc_save_bss::hmac_tx_mgmt_send_event failed[%d].}", ret);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

uint32_t hmac_scan_rrm_proc_save_bss(mac_vap_stru *mac_vap, uint8_t uc_len, uint8_t *param)
{
    oal_netbuf_stru *action_table_bcn_rpt;
    uint8_t *data;
    mac_user_stru *mac_user;
    mac_vap_rrm_trans_req_info_stru *trans_req_info;
    hmac_device_stru *hmac_device;
    hmac_vap_stru *hmac_vap;
    uint16_t len;

    if (param == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_scan_rrm_proc_save_bss::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_get_base_mac_res_stru(mac_vap, &mac_user, &hmac_vap, &hmac_device) != OAL_SUCC) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    trans_req_info = (mac_vap_rrm_trans_req_info_stru *)param;

    action_table_bcn_rpt = (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF,
                                                                   WLAN_MEM_NETBUF_SIZE2,
                                                                   OAL_NETBUF_PRIORITY_MID);
    if (action_table_bcn_rpt == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_scan_rrm_proc_save_bss::action_table_bcn_rpt null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(oal_netbuf_cb(action_table_bcn_rpt), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    data = (uint8_t *)oal_netbuf_header(action_table_bcn_rpt);

    len = hmac_set_netbuf_head_and_meas_report_tag_flag(data, mac_vap, mac_user, trans_req_info);
    if ((WLAN_MEM_NETBUF_SIZE2 - len) < (HMAC_MEASUREMENT_RPT_FIX_LEN + HMAC_BEACON_RPT_FIX_LEN)) {
        /* 释放 */
        oal_netbuf_free(action_table_bcn_rpt);
        return OAL_SUCC;
    }

    hmac_scan_meas_rpt_data_set(hmac_device, hmac_vap, data, &len, trans_req_info);

    if (hmac_scan_set_tx_cb_and_post_event(mac_vap, action_table_bcn_rpt, mac_user, len) != OAL_SUCC) {
        oal_netbuf_free(action_table_bcn_rpt);
    }

    return OAL_SUCC;
}
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_11K_C


