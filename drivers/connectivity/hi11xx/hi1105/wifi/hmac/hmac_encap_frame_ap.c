
#include "oal_cfg80211.h"
#include "oal_net.h"
#include "wlan_spec.h"
#include "mac_frame.h"
#include "mac_user.h"
#include "mac_vap.h"
#include "mac_function.h"

#include "hmac_encap_frame_ap.h"
#include "hmac_main.h"
#include "hmac_tx_data.h"
#include "hmac_mgmt_ap.h"
#include "hmac_11i.h"
#include "hmac_blockack.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_blacklist.h"

#ifdef _PRE_WLAN_FEATURE_HIEX
#include "hmac_hiex.h"
#endif
#include "hmac_11ax.h"
#include "hmac_opmode.h"
#include "hmac_11w.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ENCAP_FRAME_AP_C

#define CTE_PAYLOAD_OFFSET    2 /* Challenge Text Element  */


void hmac_set_supported_rates_ie_asoc_rsp(mac_user_stru *pst_mac_user, uint8_t *puc_buffer, uint8_t *puc_ie_len)
{
    uint8_t uc_nrates;
    uint8_t uc_idx;
    /**************************************************************************
                        ---------------------------------------
                        |Element ID | Length | Supported Rates|
                        ---------------------------------------
             Octets:    |1          | 1      | 1~8            |
                        ---------------------------------------
    The Information field is encoded as 1 to 8 octets, where each octet describes a single Supported
    Rate or BSS membership selector.
    **************************************************************************/
    puc_buffer[0] = MAC_EID_RATES;
    uc_nrates = pst_mac_user->st_avail_op_rates.uc_rs_nrates;

    if (uc_nrates > MAC_MAX_SUPRATES) {
        uc_nrates = MAC_MAX_SUPRATES;
    }

    for (uc_idx = 0; uc_idx < uc_nrates; uc_idx++) {
        puc_buffer[MAC_IE_HDR_LEN + uc_idx] = pst_mac_user->st_avail_op_rates.auc_rs_rates[uc_idx];
    }

    puc_buffer[1] = uc_nrates;
    *puc_ie_len = MAC_IE_HDR_LEN + uc_nrates;
}


void hmac_set_exsup_rates_ie_asoc_rsp(mac_user_stru *pst_mac_user, uint8_t *puc_buffer, uint8_t *puc_ie_len)
{
    uint8_t uc_nrates;
    uint8_t uc_idx;

    /***************************************************************************
                   -----------------------------------------------
                   |ElementID | Length | Extended Supported Rates|
                   -----------------------------------------------
       Octets:     |1         | 1      | 1-255                   |
                   -----------------------------------------------
    ***************************************************************************/
    if (pst_mac_user->st_avail_op_rates.uc_rs_nrates <= MAC_MAX_SUPRATES) {
        *puc_ie_len = 0;
        return;
    }

    puc_buffer[0] = MAC_EID_XRATES;
    uc_nrates = pst_mac_user->st_avail_op_rates.uc_rs_nrates - MAC_MAX_SUPRATES;
    puc_buffer[1] = uc_nrates;

    for (uc_idx = 0; uc_idx < uc_nrates; uc_idx++) {
        puc_buffer[MAC_IE_HDR_LEN + uc_idx] = pst_mac_user->st_avail_op_rates.auc_rs_rates[uc_idx + MAC_MAX_SUPRATES];
    }

    *puc_ie_len = MAC_IE_HDR_LEN + uc_nrates;
}

#ifdef _PRE_WLAN_FEATURE_TXBF_HT
OAL_STATIC OAL_INLINE void hmac_mgmt_set_txbf_cap_para(uint8_t *puc_asoc_rsp)
{
    mac_txbf_cap_stru *pst_txbf_cap;

    pst_txbf_cap = (mac_txbf_cap_stru *)puc_asoc_rsp;
    pst_txbf_cap->bit_rx_stagg_sounding = OAL_TRUE;
    pst_txbf_cap->bit_explicit_compr_bf_fdbk = 1;
    pst_txbf_cap->bit_compr_steering_num_bf_antssup = 1;
    pst_txbf_cap->bit_minimal_grouping = MAC_TXBF_CAP_MIN_GROUPING;
    pst_txbf_cap->bit_chan_estimation = 1;
}
#endif

#if defined(_PRE_WLAN_FEATURE_160M)
OAL_STATIC void hmac_mgmt_encap_asoc_rsp_ap_set_vht_oper(mac_vap_stru *pst_mac_ap,
    uint8_t uc_ie_len, mac_user_stru *pst_mac_user, uint8_t *puc_asoc_rsp)
{
    mac_vht_opern_stru *pst_vht_opern = NULL;

    if ((pst_mac_ap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS) &&
        (pst_mac_ap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS)) {
        if ((uc_ie_len != 0) && (pst_mac_user->st_vht_hdl.bit_short_gi_160mhz == 0)) {
            pst_vht_opern = (mac_vht_opern_stru *)(puc_asoc_rsp + MAC_IE_HDR_LEN);
            pst_vht_opern->uc_channel_center_freq_seg1 = 0;
        }
    }
}
#endif

OAL_STATIC uint8_t *hmac_mgmt_encap_asoc_rsp_ap_set_ht_ie(mac_vap_stru *mac_ap,
    mac_user_stru *mac_user, uint8_t *asoc_rsp)
{
    uint8_t ie_len = 0;

    if (mac_user->st_ht_hdl.en_ht_capable == OAL_TRUE) {
        /* 设置 HT-Capabilities Information IE */
        ie_len = mac_set_ht_capabilities_ie(mac_ap, asoc_rsp);
#ifdef _PRE_WLAN_FEATURE_TXBF_HT
        if (oal_all_true_value2(mac_user->st_cap_info.bit_11ntxbf, mac_ap->st_cap_flag.bit_11ntxbf) &&
            (ie_len != 0)) {
            asoc_rsp += MAC_11N_TXBF_CAP_OFFSET;
            hmac_mgmt_set_txbf_cap_para(asoc_rsp); // 封装函数降低行数
            asoc_rsp -= MAC_11N_TXBF_CAP_OFFSET;
        }
#endif
        asoc_rsp += ie_len;

        /* 设置 HT-Operation Information IE */
        asoc_rsp += mac_set_ht_opern_ie(mac_ap, asoc_rsp);

        /* 设置 Overlapping BSS Scan Parameters Information IE */
        asoc_rsp += mac_set_obss_scan_params(mac_ap, asoc_rsp);

        /* 设置 Extended Capabilities Information IE */
        mac_set_ext_capabilities_ie(mac_ap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;
    }
    return asoc_rsp;
}
OAL_STATIC uint8_t *hmac_mgmt_encap_asoc_rsp_ap_set_vht_ie(mac_vap_stru *mac_ap,
    mac_user_stru *mac_user, uint8_t *asoc_rsp)
{
    uint8_t ie_len = 0;

    if (mac_user->st_vht_hdl.en_vht_capable == OAL_TRUE) {
        /* 设置 VHT Capabilities IE */
        mac_set_vht_capabilities_ie(mac_ap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;

        /* 设置 VHT Opern IE */
        mac_set_vht_opern_ie(mac_ap, asoc_rsp, &ie_len);
        /* AP 160M 适配对端能力来设置VHT Oper字段 */
#if defined(_PRE_WLAN_FEATURE_160M)
        hmac_mgmt_encap_asoc_rsp_ap_set_vht_oper(mac_ap, ie_len, mac_user, asoc_rsp);
#endif
        asoc_rsp += ie_len;

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
        mac_set_opmode_notify_ie((void *)mac_ap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;
#endif

#ifdef _PRE_WLAN_FEATURE_1024QAM
        mac_set_1024qam_vendor_ie(mac_ap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;
#endif
    }
    return asoc_rsp;
}

OAL_STATIC uint32_t hmac_mgmt_encap_asoc_rsp_header(mac_vap_stru *mac_ap,
    uint8_t *puc_asoc_rsp, uint16_t us_type, const uint8_t *puc_sta_addr, uint16_t us_assoc_id)
{

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
    /* 设置 Frame Control field */
    mac_hdr_set_frame_control(puc_asoc_rsp, us_type);

    /* 设置 DA address1: STA MAC地址 */
    oal_set_mac_addr(puc_asoc_rsp + WLAN_HDR_ADDR1_OFFSET, puc_sta_addr);

    /* 设置 SA address2: dot11MACAddress */
    oal_set_mac_addr(puc_asoc_rsp + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(mac_ap));

    /* 设置 DA address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr(puc_asoc_rsp + WLAN_HDR_ADDR3_OFFSET, mac_ap->auc_bssid);

    return MAC_80211_FRAME_LEN;
}
#ifdef _PRE_WLAN_FEATURE_11AX

OAL_STATIC uint8_t *hmac_mgmt_encap_asoc_rsp_ap_set_he_ie(mac_vap_stru *mac_ap, uint8_t *asoc_rsp,
    uint16_t assoc_id)
{
    uint8_t ie_len = 0;

    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        /* 设置 HE Capabilities 和 HE Operation IE */
        mac_set_he_ie_in_assoc_rsp(mac_ap, assoc_id, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;
    }
    return asoc_rsp;
}
#endif

uint32_t hmac_mgmt_encap_asoc_rsp_ap(mac_vap_stru *mac_ap, const unsigned char *puc_sta_addr,
    uint16_t us_assoc_id, mac_status_code_enum_uint16 en_status_code, uint8_t *puc_asoc_rsp, uint16_t us_type)
{
    uint32_t us_asoc_rsp_len = 0;
    uint32_t timeout;
    uint8_t uc_ie_len = 0;
    mac_device_stru *pst_mac_device = NULL;
    uint8_t *puc_asoc_rsp_original = NULL;
    mac_timeout_interval_type_enum en_tie_type = MAC_TIE_BUTT;

    mac_user_stru *pst_mac_user = NULL;
    hmac_user_stru *hmac_user = NULL;
    uint16_t us_app_ie_len = 0;

    if (oal_any_null_ptr3(mac_ap, puc_sta_addr, puc_asoc_rsp)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_mgmt_encap_asoc_rsp_ap::mac_ap or puc_data or \
            puc_asoc_req is null.}");
        return us_asoc_rsp_len;
    }

    /* 保存起始地址，方便计算长度 */
    puc_asoc_rsp_original = puc_asoc_rsp;

    /* 获取device */
    pst_mac_device = mac_res_get_dev(mac_ap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log0(mac_ap->uc_vap_id, OAM_SF_ASSOC, "{hmac_mgmt_encap_asoc_rsp_ap::pst_mac_device NULL}");
    }

    pst_mac_user = mac_res_get_mac_user(us_assoc_id);
    if (pst_mac_user == NULL) {
        oam_warning_log1(0, OAM_SF_ASSOC, "{hmac_mgmt_encap_asoc_rsp_ap::pst_mac_user[%d] null ptr.}", us_assoc_id);
        return us_asoc_rsp_len;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(us_assoc_id);
    if (hmac_user == NULL) {
        oam_warning_log1(0, OAM_SF_ASSOC, "{hmac_mgmt_encap_asoc_rsp_ap::hmac_user(idx=%d) NULL}", us_assoc_id);
        return 0;
    }
    if (en_status_code == MAC_REJECT_TEMP) {
        en_tie_type = MAC_TIE_ASSOCIATION_COMEBACK_TIME;
    }
    puc_asoc_rsp += hmac_mgmt_encap_asoc_rsp_header(mac_ap, puc_asoc_rsp, us_type, puc_sta_addr, us_assoc_id);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*              Association Response Frame - Frame Body                  */
    /* --------------------------------------------------------------------- */
    /* | Capability Information |   Status Code   | AID | Supported  Rates | */
    /* --------------------------------------------------------------------- */
    /* |2                       |2                |2    |3-10              | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    /* 设置 capability information field */
    mac_set_cap_info_ap(mac_ap, puc_asoc_rsp);
    puc_asoc_rsp += MAC_CAP_INFO_LEN;

    /* 设置 Status Code */
    mac_set_status_code_ie(puc_asoc_rsp, en_status_code);
    puc_asoc_rsp += MAC_STATUS_CODE_LEN;

    /* 设置 Association ID */
    mac_set_aid_ie(puc_asoc_rsp, us_assoc_id);
    puc_asoc_rsp += MAC_AID_LEN;

    /* 设置 Supported Rates IE */
    hmac_set_supported_rates_ie_asoc_rsp(pst_mac_user, puc_asoc_rsp, &uc_ie_len);
    puc_asoc_rsp += uc_ie_len;

    /* 设置 Extended Supported Rates IE */
    hmac_set_exsup_rates_ie_asoc_rsp(pst_mac_user, puc_asoc_rsp, &uc_ie_len);
    puc_asoc_rsp += uc_ie_len;

    /* 设置 EDCA IE */
    puc_asoc_rsp += mac_set_wmm_params_ie(mac_ap, puc_asoc_rsp, pst_mac_user->st_cap_info.bit_qos);

    /* 设置 Timeout Interval (Association Comeback time) IE */
    timeout = hmac_get_assoc_comeback_time(mac_ap, hmac_user);
    mac_set_timeout_interval_ie(mac_ap, puc_asoc_rsp, &uc_ie_len, en_tie_type, timeout);
    puc_asoc_rsp += uc_ie_len;

    /* 设置HT IE */
    puc_asoc_rsp = hmac_mgmt_encap_asoc_rsp_ap_set_ht_ie(mac_ap, pst_mac_user, puc_asoc_rsp);
    /* 设置VHT IE */
    puc_asoc_rsp = hmac_mgmt_encap_asoc_rsp_ap_set_vht_ie(mac_ap, pst_mac_user, puc_asoc_rsp);
#ifdef _PRE_WLAN_FEATURE_11AX
    /* 设置HE IE */
    puc_asoc_rsp = hmac_mgmt_encap_asoc_rsp_ap_set_he_ie(mac_ap, puc_asoc_rsp, us_assoc_id);
#endif
    mac_set_hisi_cap_vendor_ie(mac_ap, puc_asoc_rsp, &uc_ie_len);
    puc_asoc_rsp += uc_ie_len;

#ifdef _PRE_WLAN_FEATURE_HIEX
    if (g_wlan_spec_cfg->feature_hiex_is_open) {
        puc_asoc_rsp += hmac_hiex_encap_ie((void *)mac_ap, NULL, puc_asoc_rsp);
    }
#endif

    /* 填充WPS信息 */
    mac_add_app_ie(mac_ap, puc_asoc_rsp, &us_app_ie_len, OAL_APP_ASSOC_RSP_IE);
    puc_asoc_rsp += us_app_ie_len;

    /* 填充 BCM Vendor VHT IE,解决与BCM STA的私有协议对通问题 */
    if (hmac_user->en_user_vendor_vht_capable == OAL_TRUE) {
        mac_set_vendor_vht_ie(mac_ap, puc_asoc_rsp, &uc_ie_len);
        puc_asoc_rsp += uc_ie_len;
    }
    /* 5G时，BCM私有vendor ie中不携带vht,需按照此格式组帧 */
    if (hmac_user->en_user_vendor_novht_capable == OAL_TRUE) {
        mac_set_vendor_novht_ie(mac_ap, puc_asoc_rsp, &uc_ie_len, OAL_FALSE);
        puc_asoc_rsp += uc_ie_len;
    }
    us_asoc_rsp_len = (uint32_t)(puc_asoc_rsp - puc_asoc_rsp_original);

    if (en_status_code != MAC_SUCCESSFUL_STATUSCODE) {
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, en_status_code);
    }
    return us_asoc_rsp_len;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_mgmt_is_challenge_txt_equal(uint8_t *puc_data, uint8_t *puc_chtxt)
{
    uint8_t *puc_ch_text = 0;
    uint16_t us_idx = 0;
    uint8_t uc_ch_text_len;

    if (oal_any_null_ptr2(puc_data, puc_chtxt)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_mgmt_is_challenge_txt_equal::puc_data or puc_chtxt is NULL.}");
        return OAL_FALSE;
    }

    /* Challenge Text Element                  */
    /* --------------------------------------- */
    /* |Element ID | Length | Challenge Text | */
    /* --------------------------------------- */
    /* | 1         |1       |1 - 253         | */
    /* --------------------------------------- */
    uc_ch_text_len = puc_data[1];
    puc_ch_text = puc_data + CTE_PAYLOAD_OFFSET;
    if (uc_ch_text_len > WLAN_CHTXT_SIZE) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_mgmt_is_challenge_txt_equal::uc_ch_text_len > 128.}");
        return OAL_FALSE;
    }
    for (us_idx = 0; us_idx < uc_ch_text_len; us_idx++) {
        /* Return false on mismatch */
        if (puc_ch_text[us_idx] != puc_chtxt[us_idx]) {
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}


oal_err_code_enum hmac_encap_auth_rsp_get_user_idx(mac_vap_stru *pst_mac_vap, uint8_t *puc_mac_addr, uint8_t uc_mac_len,
    uint8_t uc_is_seq1, uint8_t *puc_auth_resend, uint16_t *pus_user_index)
{
    uint32_t ret;
    hmac_vap_stru *pst_hmac_vap = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    hmac_user_stru *pst_user = NULL;
    hmac_vap_stru *pst_hmac_other_vap = NULL;
    uint16_t us_user_idx;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if ((pst_hmac_vap == NULL) || (uc_mac_len != ETHER_ADDR_LEN)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_encap_auth_rsp_get_user_idx::mac_res_get_hmac_vap failed!}");
        return OAL_FAIL;
    }

    *puc_auth_resend = OAL_FALSE;
    ret = mac_vap_find_user_by_macaddr(&(pst_hmac_vap->st_vap_base_info), puc_mac_addr, pus_user_index);
    /* 找到用户 */
    if (ret == OAL_SUCC) {
        /* 获取hmac用户的状态，如果不是0，说明是重复帧 */
        pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(*pus_user_index);
        if (pst_hmac_user == NULL) {
            oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                           "{hmac_encap_auth_rsp_get_user_idx::mac_res_get_hmac_user return null}");
            return OAL_FAIL;
        }
        /* en_user_asoc_state为枚举变量，取值为1~4，初始化为MAC_USER_STATE_BUTT，
         * 应该使用!=MAC_USER_STATE_BUTT作为判断，否则会导致WEP share加密关联不上问题
         */
        if (pst_hmac_user->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_BUTT) {
            *puc_auth_resend = OAL_TRUE;
        }

        if (pst_hmac_user->st_user_base_info.en_user_asoc_state == MAC_USER_STATE_ASSOC) {
            oal_net_device_stru *pst_net_device;

            pst_net_device = hmac_vap_get_net_device(pst_mac_vap->uc_vap_id);
            if (pst_net_device != NULL) {
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
                oal_kobject_uevent_env_sta_leave(pst_net_device, puc_mac_addr);
#endif
            }
        }

        return OAL_SUCC;
    }

    /* 若在同一chip下的其他VAP下找到给用户，删除之。否则导致业务不通。在DBAC下尤其常见 */
    if (OAL_SUCC == mac_chip_find_user_by_macaddr(pst_hmac_vap->st_vap_base_info.uc_chip_id,
                                                  puc_mac_addr, &us_user_idx)) {
        pst_user = mac_res_get_hmac_user(us_user_idx);
        if (pst_user != NULL) {
            pst_hmac_other_vap = mac_res_get_hmac_vap(pst_user->st_user_base_info.uc_vap_id);
            if (pst_hmac_other_vap != NULL && pst_hmac_other_vap != pst_hmac_vap) {
                /* 抛事件上报内核，已经删除某个STA */
                hmac_mgmt_send_disassoc_frame(&(pst_hmac_other_vap->st_vap_base_info), puc_mac_addr, MAC_DISAS_LV_SS,
                    OAL_FALSE);
                hmac_handle_disconnect_rsp_ap(pst_hmac_other_vap, pst_user);
                hmac_user_del(&(pst_hmac_other_vap->st_vap_base_info), pst_user);
            }
        }
    }

    /* 在收到第一个认证帧时用户已创建 */
    if (!uc_is_seq1) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_encap_auth_rsp_get_user_idx::user should have been added at seq1!}");
        return OAL_FAIL;
    }

    ret = hmac_user_add(pst_mac_vap, puc_mac_addr, pus_user_index);
    if (ret != OAL_SUCC) {
        return (ret == OAL_ERR_CODE_CONFIG_EXCEED_SPEC) ? OAL_ERR_CODE_CONFIG_EXCEED_SPEC : OAL_FAIL;
    }

    return OAL_SUCC;
}


hmac_ap_auth_process_code_enum hmac_encap_auth_rsp_seq1(mac_vap_stru *pst_mac_vap,
    hmac_auth_rsp_param_stru *pst_auth_rsp_param, uint8_t *puc_code, mac_user_asoc_state_enum_uint8 *pst_usr_ass_stat)
{
    *puc_code = MAC_SUCCESSFUL_STATUSCODE;
    *pst_usr_ass_stat = MAC_USER_STATE_BUTT;
    /* 如果不是重传 */
    if (pst_auth_rsp_param->uc_auth_resend != OAL_TRUE) {
        if (pst_auth_rsp_param->us_auth_type == WLAN_WITP_AUTH_OPEN_SYSTEM) {
            *pst_usr_ass_stat = MAC_USER_STATE_AUTH_COMPLETE;

            return HMAC_AP_AUTH_SEQ1_OPEN_ANY;
        }

        if (pst_auth_rsp_param->en_is_wep_allowed == OAL_TRUE) {
            *pst_usr_ass_stat = MAC_USER_STATE_AUTH_KEY_SEQ1;
            /* 此处返回后需要wep后操作 */
            return HMAC_AP_AUTH_SEQ1_WEP_NOT_RESEND;
        }

        /* 不支持算法 */
        *puc_code = MAC_UNSUPT_ALG;
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, *puc_code);
        return HMAC_AP_AUTH_BUTT;
    }

    /* 检查用户状态 */
    if ((pst_auth_rsp_param->en_user_asoc_state == MAC_USER_STATE_ASSOC) &&
        (pst_auth_rsp_param->us_auth_type == WLAN_WITP_AUTH_OPEN_SYSTEM)) {
        if (OAL_TRUE == mac_mib_get_dot11RSNAMFPC(pst_mac_vap)) {
            *pst_usr_ass_stat = MAC_USER_STATE_ASSOC;
        } else {
            *pst_usr_ass_stat = MAC_USER_STATE_AUTH_COMPLETE;
        }

        return HMAC_AP_AUTH_DUMMY;
    }

    if (pst_auth_rsp_param->us_auth_type == WLAN_WITP_AUTH_OPEN_SYSTEM) {
        *pst_usr_ass_stat = MAC_USER_STATE_AUTH_COMPLETE;

        return HMAC_AP_AUTH_SEQ1_OPEN_ANY;
    }

    if (pst_auth_rsp_param->en_is_wep_allowed == OAL_TRUE) {
        /* seq为1 的认证帧重传 */
        *pst_usr_ass_stat = MAC_USER_STATE_AUTH_COMPLETE;
        return HMAC_AP_AUTH_SEQ1_WEP_RESEND;
    }
    /* 不支持算法 */
    *puc_code = MAC_UNSUPT_ALG;
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                         CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, *puc_code);
    return HMAC_AP_AUTH_BUTT;
}

hmac_ap_auth_process_code_enum hmac_encap_auth_rsp_seq3(mac_vap_stru *pst_mac_vap,
    hmac_auth_rsp_param_stru *pst_auth_rsp_param, uint8_t *puc_code, mac_user_asoc_state_enum_uint8 *pst_usr_ass_stat)
{
    /* 如果不存在，返回错误 */
    if (pst_auth_rsp_param->uc_auth_resend == OAL_FALSE) {
        *pst_usr_ass_stat = MAC_USER_STATE_BUTT;
        *puc_code = MAC_SUCCESSFUL_STATUSCODE;
        return HMAC_AP_AUTH_BUTT;
    }
    /* 检查用户状态 */
    if ((pst_auth_rsp_param->en_user_asoc_state == MAC_USER_STATE_ASSOC) &&
        (pst_auth_rsp_param->us_auth_type == WLAN_WITP_AUTH_OPEN_SYSTEM)) {
        if (OAL_TRUE == mac_mib_get_dot11RSNAMFPC(pst_mac_vap)) {
            *pst_usr_ass_stat = MAC_USER_STATE_ASSOC;
        } else {
            *pst_usr_ass_stat = MAC_USER_STATE_AUTH_COMPLETE;
        }

        *puc_code = MAC_SUCCESSFUL_STATUSCODE;
        return HMAC_AP_AUTH_DUMMY;
    }

    if (pst_auth_rsp_param->us_auth_type == WLAN_WITP_AUTH_OPEN_SYSTEM) {
        *pst_usr_ass_stat = MAC_USER_STATE_AUTH_COMPLETE;
        *puc_code = MAC_SUCCESSFUL_STATUSCODE;
        return HMAC_AP_AUTH_SEQ3_OPEN_ANY;
    }

    if (pst_auth_rsp_param->en_user_asoc_state == MAC_USER_STATE_AUTH_KEY_SEQ1) {
        *pst_usr_ass_stat = MAC_USER_STATE_AUTH_COMPLETE;
        *puc_code = MAC_SUCCESSFUL_STATUSCODE;
        return HMAC_AP_AUTH_SEQ3_WEP_COMPLETE;
    }

    if (pst_auth_rsp_param->en_user_asoc_state == MAC_USER_STATE_AUTH_COMPLETE) {
        *pst_usr_ass_stat = MAC_USER_STATE_AUTH_COMPLETE;
        *puc_code = MAC_SUCCESSFUL_STATUSCODE;
        return HMAC_AP_AUTH_SEQ3_WEP_COMPLETE;
    }

    if (pst_auth_rsp_param->en_user_asoc_state == MAC_USER_STATE_ASSOC) {
        *pst_usr_ass_stat = MAC_USER_STATE_AUTH_KEY_SEQ1;
        *puc_code = MAC_SUCCESSFUL_STATUSCODE;
        return HMAC_AP_AUTH_SEQ3_WEP_ASSOC;
    }

    /* 不支持算法 */
    *pst_usr_ass_stat = MAC_USER_STATE_BUTT;
    *puc_code = MAC_UNSUPT_ALG;
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                         CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, *puc_code);
    return HMAC_AP_AUTH_BUTT;
}


hmac_auth_rsp_fun hmac_encap_auth_rsp_get_func(uint16_t us_auth_seq)
{
    hmac_auth_rsp_fun st_auth_rsp_fun = NULL;
    switch (us_auth_seq) {
        case WLAN_AUTH_TRASACTION_NUM_ONE:
            st_auth_rsp_fun = hmac_encap_auth_rsp_seq1;
            break;
        case WLAN_AUTH_TRASACTION_NUM_THREE:
            st_auth_rsp_fun = hmac_encap_auth_rsp_seq3;
            break;
        default:
            st_auth_rsp_fun = NULL;
            break;
    }
    return st_auth_rsp_fun;
}


uint32_t hmac_encap_auth_rsp_support(hmac_vap_stru *pst_hmac_vap, uint16_t us_auth_type)
{
    if (pst_hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 检测认证类型是否支持 不支持的话状态位置成UNSUPT_ALG */
    if ((mac_mib_get_AuthenticationMode(&pst_hmac_vap->st_vap_base_info) != us_auth_type)
        && (WLAN_WITP_AUTH_AUTOMATIC != mac_mib_get_AuthenticationMode(&pst_hmac_vap->st_vap_base_info))) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }
    return OAL_SUCC;
}


void hmac_tid_clear(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user)
{
    mac_device_stru *pst_device = NULL;
    uint8_t uc_loop;
    hmac_amsdu_stru *pst_amsdu = NULL;
    oal_netbuf_stru *pst_amsdu_net_buf = NULL;
    hmac_tid_stru *pst_tid = NULL;

    hmac_vap_stru *pst_hmac_vap = NULL;

    if (pst_hmac_user == NULL) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_tid_clear::pst_hmac_user is null.}");
        return;
    }
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_tid_clear::pst_hmac_vap null.}");
        return;
    }
    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_device == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_tid_clear::pst_device null.}");
        return;
    }

    for (uc_loop = 0; uc_loop < WLAN_TID_MAX_NUM; uc_loop++) {
        pst_amsdu = &(pst_hmac_user->ast_hmac_amsdu[uc_loop]);

        /* tid锁, 禁软中断 */
        oal_spin_lock_bh(&pst_amsdu->st_amsdu_lock);

        if (pst_amsdu->st_amsdu_timer.en_is_registerd == OAL_TRUE) {
            frw_timer_immediate_destroy_timer_m(&(pst_amsdu->st_amsdu_timer));
        }

        /* 清空聚合队列 */
        if (pst_amsdu->msdu_num != 0) {
            while (OAL_TRUE != oal_netbuf_list_empty(&pst_amsdu->st_msdu_head)) {
                pst_amsdu_net_buf = oal_netbuf_delist(&(pst_amsdu->st_msdu_head));

                oal_netbuf_free(pst_amsdu_net_buf);
            }
            pst_amsdu->msdu_num = 0;
            pst_amsdu->amsdu_size = 0;
        }

        /* tid解锁, 使能软中断 */
        oal_spin_unlock_bh(&pst_amsdu->st_amsdu_lock);

        pst_tid = &(pst_hmac_user->ast_tid_info[uc_loop]);

        if (pst_tid->st_ba_tx_info.st_addba_timer.en_is_registerd == OAL_TRUE) {
            frw_timer_immediate_destroy_timer_m(&(pst_tid->st_ba_tx_info.st_addba_timer));
        }

        pst_tid->uc_tid_no = (uint8_t)uc_loop;
        pst_tid->us_hmac_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;

        /* 清除接收方向会话句柄 */
        if (pst_tid->pst_ba_rx_info != NULL) {
            hmac_ba_reset_rx_handle(pst_device, pst_hmac_user, uc_loop, OAL_TRUE);
        }

        if (pst_tid->st_ba_tx_info.en_ba_status != DMAC_BA_INIT) {
            /* 存在TX BA会话句柄，要-- */
            hmac_tx_ba_session_decr(pst_hmac_vap, uc_loop);
        }

        /* 初始化ba tx操作句柄 */
        pst_tid->st_ba_tx_info.en_ba_status = DMAC_BA_INIT;
        pst_tid->st_ba_tx_info.uc_addba_attemps = 0;
        pst_tid->st_ba_tx_info.uc_dialog_token = 0;
        pst_tid->st_ba_tx_info.uc_ba_policy = 0;
        pst_hmac_user->auc_ba_flag[uc_loop] = 0;
    }
}

OAL_STATIC uint16_t hmac_get_auth_rsp_timout(mac_vap_stru *pst_mac_vap)
{
    uint16_t us_auth_rsp_timeout;

    us_auth_rsp_timeout = ((g_st_mac_device_custom_cfg.us_cmd_auth_rsp_timeout != 0) ? \
            g_st_mac_device_custom_cfg.us_cmd_auth_rsp_timeout : \
                (uint16_t)mac_mib_get_AuthenticationResponseTimeOut(pst_mac_vap));
    return us_auth_rsp_timeout;
}

static void hmac_encap_auth_rsp_frame_head(uint8_t *data, mac_vap_stru *mac_vap,
                                           uint8_t *addr2, oal_netbuf_stru *auth_req)
{
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
    /* 设置函数头的frame control字段 */
    mac_hdr_set_frame_control(data, WLAN_FC0_SUBTYPE_AUTH);

    /* 获取STA的地址 */
    mac_get_address2(oal_netbuf_header(auth_req), addr2, WLAN_MAC_ADDR_LEN);

    /* 将DA设置为STA的地址 */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->auc_address1, addr2);

    /* 将SA设置为dot11MacAddress */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->auc_address2, mac_mib_get_StationID(mac_vap));
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->auc_address3, mac_vap->auc_bssid);
}

static void hmac_set_auth_resp_body_begin5_byte(uint8_t *frame, uint16_t auth_type, uint16_t auth_seq)
{
    /* 设置认证类型IE */
    frame[0] = (auth_type & 0x00FF);
    frame[BYTE_OFFSET_1] = (auth_type & 0xFF00) >> BIT_OFFSET_8;

    /* 将收到的transaction number + 1后复制给新的认证响应帧 */
    frame[BYTE_OFFSET_2] = ((auth_seq + 1) & 0x00FF);
    frame[BYTE_OFFSET_3] = ((auth_seq + 1) & 0xFF00) >> BIT_OFFSET_8;

    /* 状态为初始化为成功 */
    frame[BYTE_OFFSET_4] = MAC_SUCCESSFUL_STATUSCODE;
    frame[BYTE_OFFSET_5] = 0;
}

static void hmac_process_auth_resp_mac_zero_fail(uint8_t *frame, mac_vap_stru *mac_vap,
                                                 mac_tx_ctl_stru *tx_ctl, uint8_t *addr2,
                                                 uint16_t auth_rsp_len)
{
    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_AUTH,
                     "{hmac_encap_auth_rsp::user mac:%02X:XX:XX:%02X:%02X:%02X is all 0 and invaild!}",
                     addr2[MAC_ADDR_0], addr2[MAC_ADDR_3], addr2[MAC_ADDR_4], addr2[MAC_ADDR_5]);
    frame[BYTE_OFFSET_4] = MAC_UNSPEC_FAIL;
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                         CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, frame[BYTE_OFFSET_4]);
    MAC_GET_CB_TX_USER_IDX(tx_ctl) = g_wlan_spec_cfg->invalid_user_id;
    MAC_GET_CB_MPDU_LEN(tx_ctl) = auth_rsp_len;
}

static void hmac_process_auth_resp_blacklist_check_fail(uint8_t *frame, mac_vap_stru *mac_vap,
                                                        mac_tx_ctl_stru *tx_ctl, uint8_t *addr2,
                                                        uint16_t auth_rsp_len)
{
    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_AUTH,
                     "{hmac_encap_auth_rsp::user mac:%02X:XX:XX:%02X:%02X:%02X is all 0 and invaild!}",
                     addr2[MAC_ADDR_0], addr2[MAC_ADDR_3], addr2[MAC_ADDR_4], addr2[MAC_ADDR_5]);
    frame[BYTE_OFFSET_4] = MAC_REQ_DECLINED;
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                         CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, frame[BYTE_OFFSET_4]);
    MAC_GET_CB_TX_USER_IDX(tx_ctl) = g_wlan_spec_cfg->invalid_user_id;
    MAC_GET_CB_MPDU_LEN(tx_ctl) = auth_rsp_len;
}

static void hmac_process_auth_resp_get_user_idx_fail(uint8_t *frame, mac_vap_stru *mac_vap,
                                                     mac_tx_ctl_stru *tx_ctl, uint32_t errorcode,
                                                     uint16_t auth_rsp_len)
{
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_AUTH,
                     "{hmac_encap_auth_rsp::hmac_ap_get_user_idx fail[%d]! (1002:exceed config spec)}", errorcode);

    frame[BYTE_OFFSET_4] = (errorcode == OAL_ERR_CODE_CONFIG_EXCEED_SPEC) ? MAC_AP_FULL : MAC_UNSPEC_FAIL;

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
                         CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, frame[BYTE_OFFSET_4]);
    MAC_GET_CB_TX_USER_IDX(tx_ctl) = g_wlan_spec_cfg->invalid_user_id;
    MAC_GET_CB_MPDU_LEN(tx_ctl) = auth_rsp_len;
}

static void hmac_process_auth_resp_get_user_fail(uint8_t *frame, mac_vap_stru *mac_vap,
                                                 mac_tx_ctl_stru *tx_ctl, uint16_t auth_rsp_len,
                                                 uint16_t user_index)
{
    oam_error_log1(mac_vap->uc_vap_id, OAM_SF_AUTH,
                   "{hmac_encap_auth_rsp::pst_hmac_user_sta[%d] is NULL}",
                   user_index);
    frame[BYTE_OFFSET_4] = MAC_UNSPEC_FAIL;
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
                         CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, frame[BYTE_OFFSET_4]);
    MAC_GET_CB_TX_USER_IDX(tx_ctl) = g_wlan_spec_cfg->invalid_user_id;
    MAC_GET_CB_MPDU_LEN(tx_ctl) = auth_rsp_len;
}

static void hmac_process_auth_resp_get_vap_fail(uint8_t *frame, mac_vap_stru *mac_vap,
                                                mac_tx_ctl_stru *tx_ctl, uint16_t auth_rsp_len,
                                                hmac_user_stru *hmac_user_sta)
{
    oam_error_log1(mac_vap->uc_vap_id, OAM_SF_AUTH,
                   "{hmac_encap_auth_rsp::pst_hmac_vap is NULL,and change user[idx==%d] state to BUTT!}",
                   hmac_user_sta->st_user_base_info.us_assoc_id);
    frame[BYTE_OFFSET_4] = MAC_UNSPEC_FAIL;
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
                         CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, frame[BYTE_OFFSET_4]);
    MAC_GET_CB_TX_USER_IDX(tx_ctl) = g_wlan_spec_cfg->invalid_user_id;
    MAC_GET_CB_MPDU_LEN(tx_ctl) = auth_rsp_len;
    mac_user_set_asoc_state(&(hmac_user_sta->st_user_base_info), MAC_USER_STATE_BUTT);
}

static void hmac_process_auth_resp_alg_unsupport(uint8_t *frame, mac_vap_stru *mac_vap,
                                                 hmac_user_stru *hmac_user_sta, uint16_t auth_type,
                                                 hmac_vap_stru *hmac_vap)
{
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_AUTH,
                     "{hmac_encap_auth_rsp::auth type[%d] not support!}",
                     auth_type);
    frame[BYTE_OFFSET_4] = MAC_UNSUPT_ALG;
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
                         CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, frame[BYTE_OFFSET_4]);

    hmac_user_set_asoc_state(&(hmac_vap->st_vap_base_info), &hmac_user_sta->st_user_base_info,
        MAC_USER_STATE_BUTT);
}

static void hmac_process_auth_resp_init_rsp_param(hmac_auth_rsp_handle_stru *auth_rsp_handle, mac_vap_stru *mac_vap,
                                                  hmac_user_stru *hmac_user_sta, uint16_t auth_type,
                                                  uint16_t auth_seq)
{
    /*  初始化处理参数 */
    auth_rsp_handle->st_auth_rsp_param.en_is_wep_allowed = mac_is_wep_allowed(mac_vap);
    auth_rsp_handle->st_auth_rsp_param.en_user_asoc_state = hmac_user_sta->st_user_base_info.en_user_asoc_state;
    auth_rsp_handle->st_auth_rsp_param.us_auth_type = auth_type;
    auth_rsp_handle->st_auth_rsp_fun = hmac_encap_auth_rsp_get_func(auth_seq);
}

static void hmac_process_auth_seq1_seq3(hmac_auth_rsp_handle_stru *auth_rsp_handle, mac_vap_stru *mac_vap,
                                        hmac_user_stru *hmac_user_sta, uint8_t *frame,
                                        hmac_ap_auth_process_code_enum *auth_proc_rst)
{
    /*  处理seq1或者seq3 */
    if (auth_rsp_handle->st_auth_rsp_fun != NULL) {
        *auth_proc_rst = auth_rsp_handle->st_auth_rsp_fun(mac_vap, &(auth_rsp_handle->st_auth_rsp_param),
            &frame[BYTE_OFFSET_4], &hmac_user_sta->st_user_base_info.en_user_asoc_state);
        /* 清空 HMAC层TID信息 */
        hmac_tid_clear(mac_vap, hmac_user_sta);
    } else {
        *auth_proc_rst = HMAC_AP_AUTH_BUTT;

        mac_user_set_asoc_state(&hmac_user_sta->st_user_base_info, MAC_USER_STATE_BUTT);
        frame[BYTE_OFFSET_4] = MAC_AUTH_SEQ_FAIL;
    }
}

static void hmac_process_auth_seq1_open(hmac_user_stru *hmac_user_sta, mac_vap_stru *mac_vap,
                                        uint16_t auth_rsp_timeout)
{
    mac_user_init_key(&hmac_user_sta->st_user_base_info);
    frw_timer_create_timer_m(&hmac_user_sta->st_mgmt_timer, hmac_mgmt_timeout_ap, auth_rsp_timeout,
        hmac_user_sta, OAL_FALSE, OAM_MODULE_ID_HMAC, mac_vap->core_id);
}

static void hmac_process_auth_seq1_wep_no_resend(hmac_user_stru *hmac_user_sta, mac_vap_stru *mac_vap,
                                                 uint8_t *frame, uint8_t *chtxt,
                                                 uint16_t *auth_rsp_len)
{
    uint16_t auth_rsp_timeout  = hmac_get_auth_rsp_timout(mac_vap);
    hmac_config_11i_add_wep_entry(mac_vap, WLAN_MAC_ADDR_LEN,
        hmac_user_sta->st_user_base_info.auc_user_mac_addr);

    hmac_mgmt_encap_chtxt(frame, chtxt, auth_rsp_len, hmac_user_sta);
    /* 为该用户启动一个定时器，超时认证失败 */
    frw_timer_create_timer_m(&hmac_user_sta->st_mgmt_timer, hmac_mgmt_timeout_ap, auth_rsp_timeout,
        hmac_user_sta, OAL_FALSE, OAM_MODULE_ID_HMAC, mac_vap->core_id);
    hmac_user_sta->st_user_base_info.st_key_info.en_cipher_type =
    mac_get_wep_type(mac_vap, mac_get_wep_default_keyid(mac_vap));
}

static void hmac_process_auth_seq1_wep_resend(hmac_user_stru *hmac_user_sta, uint8_t *frame, uint8_t *chtxt,
                                              hmac_vap_stru *hmac_vap, uint16_t *auth_rsp_len)
{
    /* seq为1 的认证帧重传 */
    hmac_mgmt_encap_chtxt(frame, chtxt, auth_rsp_len, hmac_user_sta);

    if (hmac_vap->st_mgmt_timer.en_is_registerd == OAL_TRUE) {
        /* 重启超时定时器 */
        frw_timer_restart_timer(&hmac_user_sta->st_mgmt_timer, hmac_user_sta->st_mgmt_timer.timeout,
            OAL_FALSE);
    }
}

static void hmac_process_auth_seq3_wep_complete(hmac_user_stru *hmac_user_sta, uint8_t *frame, uint8_t *chtxt,
                                                oal_netbuf_stru *auth_req, mac_vap_stru *mac_vap)
{
    chtxt = mac_get_auth_ch_text(oal_netbuf_header(auth_req));
    if (hmac_mgmt_is_challenge_txt_equal(chtxt, hmac_user_sta->auc_ch_text) == OAL_TRUE) {
        mac_user_set_asoc_state(&hmac_user_sta->st_user_base_info, MAC_USER_STATE_AUTH_COMPLETE);
        /* cancel timer for auth */
        frw_timer_immediate_destroy_timer_m(&hmac_user_sta->st_mgmt_timer);
    } else {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_AUTH, "{hmac_encap_auth_rsp::ch txt not equal.}");
        frame[BYTE_OFFSET_4] = MAC_CHLNG_FAIL;

        mac_user_set_asoc_state(&hmac_user_sta->st_user_base_info, MAC_USER_STATE_BUTT);
    }
}

static void hmac_process_auth_seq3_wep_assoc(hmac_user_stru *hmac_user_sta, mac_vap_stru *mac_vap,
                                             uint8_t *frame, uint8_t *chtxt,
                                             uint16_t *auth_rsp_len)
{
    uint16_t auth_rsp_timeout  = hmac_get_auth_rsp_timout(mac_vap);
    hmac_mgmt_encap_chtxt(frame, chtxt, auth_rsp_len, hmac_user_sta);

    /* 开启超时定时器 */
    frw_timer_create_timer_m(&hmac_user_sta->st_mgmt_timer, hmac_mgmt_timeout_ap, auth_rsp_timeout,
        hmac_user_sta, OAL_FALSE, OAM_MODULE_ID_HMAC, mac_vap->core_id);
}

void hmac_process_auth_default(hmac_user_stru *hmac_user_sta)
{
    mac_user_init_key(&hmac_user_sta->st_user_base_info);
    hmac_user_sta->st_user_base_info.en_user_asoc_state = MAC_USER_STATE_BUTT;
}

uint16_t hmac_encap_auth_rsp(mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_auth_rsp, oal_netbuf_stru *pst_auth_req,
    uint8_t *puc_chtxt, uint8_t uc_chtxt_len)
{
    uint16_t us_auth_rsp_len = 0;
    hmac_user_stru *pst_hmac_user_sta = NULL;
    uint8_t *puc_frame = NULL;
    uint16_t us_index;
    uint16_t us_auth_type;
    uint8_t uc_is_seq1;
    uint16_t us_auth_seq;
    uint8_t auc_addr2[WLAN_MAC_ADDR_LEN] = { 0 };
    hmac_vap_stru *pst_hmac_vap = NULL;
    uint16_t us_user_index = 0xffff;
    uint32_t ret;
    uint16_t us_auth_rsp_timeout;

    /* 认证方法 */
    hmac_ap_auth_process_code_enum auth_proc_rst = 0;

    uint8_t *puc_data = NULL;
    mac_tx_ctl_stru *pst_tx_ctl = NULL;
    hmac_auth_rsp_handle_stru st_auth_rsp_handle;
    uint32_t alg_suppt;

    if (oal_any_null_ptr4(pst_mac_vap, pst_auth_rsp, pst_auth_req, puc_chtxt)) {
        oam_error_log0(0, OAM_SF_AUTH,
                       "{hmac_encap_auth_rsp::pst_mac_vap or p_data or p_auth_req or p_chtxt is null.}");
        return us_auth_rsp_len;
    }

    if (uc_chtxt_len > WLAN_CHTXT_SIZE) {
        return us_auth_rsp_len;
    }

    puc_data = (uint8_t *)oal_netbuf_header(pst_auth_rsp);
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_auth_rsp);

    hmac_encap_auth_rsp_frame_head(puc_data, pst_mac_vap, (uint8_t *)auc_addr2, pst_auth_req);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*              Authentication Frame - Frame Body                        */
    /* --------------------------------------------------------------------- */
    /* |Auth Algo Number|Auth Trans Seq Number|Status Code| Challenge Text | */
    /* --------------------------------------------------------------------- */
    /* | 2              |2                    |2          | 3 - 256        | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    us_index = MAC_80211_FRAME_LEN;
    puc_frame = (uint8_t *)(puc_data + us_index);

    /* 计算认证相应帧的长度 */
    us_auth_rsp_len = MAC_80211_FRAME_LEN + MAC_AUTH_ALG_LEN + MAC_AUTH_TRANS_SEQ_NUM_LEN + MAC_STATUS_CODE_LEN;

    /* 解析认证类型 */
    us_auth_type = mac_get_auth_algo_num(pst_auth_req);

    /* 解析auth transaction number */
    us_auth_seq = mac_get_auth_seq_num(oal_netbuf_header(pst_auth_req));
    if (us_auth_seq > HMAC_AP_AUTH_SEQ3_WEP_COMPLETE) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_AUTH,
                         "{hmac_encap_auth_rsp::auth recieve invalid seq, auth seq [%d]}",
                         us_auth_seq);
        return 0;
    }

    hmac_set_auth_resp_body_begin5_byte(puc_frame, us_auth_type, us_auth_seq);

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CONN,
                     "{hmac_encap_auth_rsp:: AUTH_RSP tx : user mac:%02X:XX:XX:%02X:%02X:%02X}",
                     auc_addr2[MAC_ADDR_0], auc_addr2[MAC_ADDR_3], auc_addr2[MAC_ADDR_4], auc_addr2[MAC_ADDR_5]);

    if (mac_addr_is_zero(auc_addr2)) {
        hmac_process_auth_resp_mac_zero_fail(puc_frame, pst_mac_vap, pst_tx_ctl, auc_addr2, us_auth_rsp_len);
        return us_auth_rsp_len;
    }
    /* 因黑名单拒绝回auth,status code回37 */
    if (hmac_blacklist_filter(pst_mac_vap, auc_addr2) == OAL_TRUE) {
        hmac_process_auth_resp_blacklist_check_fail(puc_frame, pst_mac_vap, pst_tx_ctl, auc_addr2, us_auth_rsp_len);
        return us_auth_rsp_len;
    }
    /* 获取用户idx */
    uc_is_seq1 = (us_auth_seq == WLAN_AUTH_TRASACTION_NUM_ONE);
    ret = hmac_encap_auth_rsp_get_user_idx(pst_mac_vap, auc_addr2, sizeof(auc_addr2), uc_is_seq1,
        &st_auth_rsp_handle.st_auth_rsp_param.uc_auth_resend, &us_user_index);
    if (ret != OAL_SUCC) {
        hmac_process_auth_resp_get_user_idx_fail(puc_frame, pst_mac_vap, pst_tx_ctl, ret, us_auth_rsp_len);
        return us_auth_rsp_len;
    }

    /* 获取hmac user指针 */
    pst_hmac_user_sta = mac_res_get_hmac_user(us_user_index);
    if (pst_hmac_user_sta == NULL) {
        hmac_process_auth_resp_get_user_fail(puc_frame, pst_mac_vap, pst_tx_ctl, us_auth_rsp_len, us_user_index);
        return us_auth_rsp_len;
    }

    /* 获取hmac vap指针 */
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        hmac_process_auth_resp_get_vap_fail(puc_frame, pst_mac_vap, pst_tx_ctl, us_auth_rsp_len, pst_hmac_user_sta);
        return us_auth_rsp_len;
    }

    /* CB字段user idx进行赋值 */
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = us_user_index;
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_auth_rsp_len;

    /* 判断算法是否支持 */
    alg_suppt = hmac_encap_auth_rsp_support(pst_hmac_vap, us_auth_type);
    if (alg_suppt != OAL_SUCC) {
        hmac_process_auth_resp_alg_unsupport(puc_frame, pst_mac_vap, pst_hmac_user_sta, us_auth_type, pst_hmac_vap);
        return us_auth_rsp_len;
    }

    hmac_process_auth_resp_init_rsp_param(&st_auth_rsp_handle, pst_mac_vap, pst_hmac_user_sta,
        us_auth_type, us_auth_seq);

    hmac_process_auth_seq1_seq3(&st_auth_rsp_handle, pst_mac_vap, pst_hmac_user_sta, puc_frame, &auth_proc_rst);

    us_auth_rsp_timeout = hmac_get_auth_rsp_timout(pst_mac_vap);

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_AUTH,
                     "{hmac_encap_auth_rsp::ul_auth_proc_rst:%d,us_auth_rsp_timeout:%d}",
                     auth_proc_rst, us_auth_rsp_timeout);

    /*  根据返回的code进行后续处理 */
    switch (auth_proc_rst) {
        case HMAC_AP_AUTH_SEQ1_OPEN_ANY:
            hmac_process_auth_seq1_open(pst_hmac_user_sta, pst_mac_vap, us_auth_rsp_timeout);
            break;
        case HMAC_AP_AUTH_SEQ1_WEP_NOT_RESEND:
            hmac_process_auth_seq1_wep_no_resend(pst_hmac_user_sta, pst_mac_vap,
                puc_frame, puc_chtxt, &us_auth_rsp_len);
            break;
        case HMAC_AP_AUTH_SEQ1_WEP_RESEND:
            hmac_process_auth_seq1_wep_resend(pst_hmac_user_sta, puc_frame, puc_chtxt, pst_hmac_vap, &us_auth_rsp_len);
            break;
        case HMAC_AP_AUTH_SEQ3_OPEN_ANY:
            mac_user_init_key(&pst_hmac_user_sta->st_user_base_info);
            break;
        case HMAC_AP_AUTH_SEQ3_WEP_COMPLETE:
            hmac_process_auth_seq3_wep_complete(pst_hmac_user_sta, puc_frame, puc_chtxt, pst_auth_req, pst_mac_vap);
            break;
        case HMAC_AP_AUTH_SEQ3_WEP_ASSOC:
            hmac_process_auth_seq3_wep_assoc(pst_hmac_user_sta, pst_mac_vap, puc_frame, puc_chtxt, &us_auth_rsp_len);
            break;
        case HMAC_AP_AUTH_DUMMY:
            break;
        case HMAC_AP_AUTH_BUTT:
        default:
            hmac_process_auth_default(pst_hmac_user_sta);
            break;
    }

    /* dmac offload架构下，同步user关联状态信息到dmac */
    ret = hmac_config_user_asoc_state_syn(&(pst_hmac_vap->st_vap_base_info), &pst_hmac_user_sta->st_user_base_info);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                       "{hmac_ap_rx_auth_req::hmac_config_user_asoc_state_syn failed[%d].}", ret);
    }
    return us_auth_rsp_len;
}
