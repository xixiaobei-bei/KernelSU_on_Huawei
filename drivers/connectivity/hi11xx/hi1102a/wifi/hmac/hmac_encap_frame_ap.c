
/* 1 头文件包含 */
#include "oal_cfg80211.h"
#include "mac_frame.h"
#include "mac_user.h"
#include "mac_vap.h"
#include "dmac_ext_if.h"
#include "hmac_encap_frame_ap.h"
#include "hmac_main.h"
#include "hmac_tx_data.h"
#include "hmac_mgmt_ap.h"
#include "hmac_11i.h"
#include "hmac_blockack.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_support_pmf.h"
#include "hmac_config.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ENCAP_FRAME_AP_C

/* 2 全局变量定义 */
/* 3 函数实现 */
OAL_STATIC oal_void hmac_set_ht_capabilities_ie(mac_vap_stru *pst_mac_ap, oal_uint8 **ppuc_asoc_rsp,
    oal_uint8 uc_ie_len, mac_user_stru *pst_mac_user, mac_txbf_cap_stru *pst_txbf_cap)
{
    if (pst_mac_user->st_ht_hdl.en_ht_capable == OAL_TRUE) {
        /* 设置 HT-Capabilities Information IE */
        mac_set_ht_capabilities_ie((oal_void *)pst_mac_ap, *ppuc_asoc_rsp, &uc_ie_len);
        if ((pst_mac_user->st_cap_info.bit_11ntxbf == OAL_TRUE) &&
            (pst_mac_ap->st_cap_flag.bit_11ntxbf == OAL_TRUE) && (uc_ie_len != 0)) {
            *ppuc_asoc_rsp += MAC_11N_TXBF_CAP_OFFSET;
            pst_txbf_cap = (mac_txbf_cap_stru *)*ppuc_asoc_rsp;
            pst_txbf_cap->bit_rx_stagg_sounding = OAL_TRUE;
            pst_txbf_cap->bit_explicit_compr_bf_fdbk = 1;
            pst_txbf_cap->bit_compr_steering_num_bf_antssup = 1;
            *ppuc_asoc_rsp -= MAC_11N_TXBF_CAP_OFFSET;
        }

        *ppuc_asoc_rsp += uc_ie_len;

        /* 设置 HT-Operation Information IE */
        mac_set_ht_opern_ie((oal_void *)pst_mac_ap, *ppuc_asoc_rsp, &uc_ie_len);
        *ppuc_asoc_rsp += uc_ie_len;

        /* 设置 Overlapping BSS Scan Parameters Information IE */
        mac_set_obss_scan_params((oal_void *)pst_mac_ap, *ppuc_asoc_rsp, &uc_ie_len);
        *ppuc_asoc_rsp += uc_ie_len;

        /* 设置 Extended Capabilities Information IE */
        mac_set_ext_capabilities_ie((oal_void *)pst_mac_ap, *ppuc_asoc_rsp, &uc_ie_len);
        *ppuc_asoc_rsp += uc_ie_len;
    }
}

oal_uint32 hmac_mgmt_encap_asoc_rsp_ap(mac_vap_stru *pst_mac_ap,
                                       oal_uint8 *puc_sta_addr,
                                       oal_uint16 us_assoc_id,
                                       mac_status_code_enum_uint16 en_status_code,
                                       oal_uint8 *puc_asoc_rsp,
                                       oal_uint16 us_type)
{
    oal_uint32                     us_asoc_rsp_len       = 0;
    oal_uint8                      uc_ie_len             = 0;
    oal_uint32                     ul_timeout;
    mac_device_stru               *pst_mac_device = OAL_PTR_NULL;
    oal_uint8                     *puc_asoc_rsp_original = OAL_PTR_NULL;
    mac_Timeout_Interval_type_enum en_tie_type           = MAC_TIE_BUTT;

    mac_user_stru                  *pst_mac_user  = OAL_PTR_NULL;
    hmac_user_stru                 *pst_hmac_user = OAL_PTR_NULL;
    oal_uint16                      us_app_ie_len;
    mac_txbf_cap_stru              *pst_txbf_cap  = OAL_PTR_NULL;

    if ((pst_mac_ap == OAL_PTR_NULL) || (puc_sta_addr == OAL_PTR_NULL) || (puc_asoc_rsp == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_ASSOC, "{pst_ap[0x%x], puc_data[0x%x], puc_asoc_req[0x%x].}", (uintptr_t)pst_mac_ap,
                       (uintptr_t)puc_sta_addr, (uintptr_t)puc_asoc_rsp);
        return us_asoc_rsp_len;
    }

    /* 保存起始地址，方便计算长度 */
    puc_asoc_rsp_original = puc_asoc_rsp;

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_ap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_mac_ap->uc_vap_id, OAM_SF_ASSOC, "{device is a null ptr.[0x%x]}", (uintptr_t)pst_mac_device);
        return us_asoc_rsp_len;
    }

    pst_mac_user = mac_res_get_mac_user(us_assoc_id);
    if (pst_mac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ASSOC, "{hmac_mgmt_encap_asoc_rsp_ap::pst_user[%d] is a null ptr.}", us_assoc_id);
        return us_asoc_rsp_len;
    }
    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(us_assoc_id);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ASSOC, "{hmac_mgmt_encap_asoc_rsp_ap::user(idx=%d) is a null ptr.}", us_assoc_id);
        return us_asoc_rsp_len;
    }

    if (en_status_code == MAC_REJECT_TEMP) {
        en_tie_type = MAC_TIE_ASSOCIATION_COMEBACK_TIME;
    }

    /*************************************************************************/
    /* Management Frame Format */
    /* -------------------------------------------------------------------- */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS| */
    /* -------------------------------------------------------------------- */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  | */
    /* -------------------------------------------------------------------- */
    /*************************************************************************/
    /*************************************************************************/
    /* Set the fields in the frame header */
    /*************************************************************************/
    /* 设置 Frame Control field */
    mac_hdr_set_frame_control(puc_asoc_rsp, us_type);

    /* 设置 DA address1: STA MAC地址 */
    oal_set_mac_addr(puc_asoc_rsp + WLAN_HDR_ADDR1_OFFSET, puc_sta_addr);

    /* 设置 SA address2: dot11MACAddress */
    oal_set_mac_addr(puc_asoc_rsp + WLAN_HDR_ADDR2_OFFSET,
                     pst_mac_ap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);

    /* 设置 DA address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr(puc_asoc_rsp + WLAN_HDR_ADDR3_OFFSET, pst_mac_ap->auc_bssid);

    puc_asoc_rsp += MAC_80211_FRAME_LEN;

    /*************************************************************************/
    /* Set the contents of the frame body */
    /*************************************************************************/
    /*************************************************************************/
    /* Association Response Frame - Frame Body */
    /* --------------------------------------------------------------------- */
    /* | Capability Information |   Status Code   | AID | Supported  Rates | */
    /* --------------------------------------------------------------------- */
    /* |2                       |2                |2    |3-10              | */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    /* 设置 capability information field */
    mac_set_cap_info_ap((oal_void *)pst_mac_ap, puc_asoc_rsp);
    puc_asoc_rsp += MAC_CAP_INFO_LEN;

    /* 设置 Status Code */
    mac_set_status_code_ie(puc_asoc_rsp, en_status_code);
    puc_asoc_rsp += MAC_STATUS_CODE_LEN;

    /* 设置 Association ID */
    mac_set_aid_ie(puc_asoc_rsp, us_assoc_id);
    puc_asoc_rsp += MAC_AID_LEN;

    /* 设置 Supported Rates IE */
    mac_set_supported_rates_ie((oal_void *)pst_mac_ap, puc_asoc_rsp, &uc_ie_len);
    puc_asoc_rsp += uc_ie_len;

    /* 设置 Extended Supported Rates IE */
    mac_set_exsup_rates_ie((oal_void *)pst_mac_ap, puc_asoc_rsp, &uc_ie_len);
    puc_asoc_rsp += uc_ie_len;

    /* 设置 EDCA IE */
    mac_set_wmm_params_ie((oal_void *)pst_mac_ap, puc_asoc_rsp, pst_mac_user->st_cap_info.bit_qos, &uc_ie_len);
    puc_asoc_rsp += uc_ie_len;

    /* 设置 Timeout Interval (Association Comeback time) IE */
    ul_timeout = hmac_get_assoc_comeback_time(pst_mac_ap, pst_hmac_user);
    mac_set_timeout_interval_ie((oal_void *)pst_mac_ap, puc_asoc_rsp, &uc_ie_len, en_tie_type, ul_timeout);
    puc_asoc_rsp += uc_ie_len;

    /* 设置 HT-Capabilities Information IE */
    hmac_set_ht_capabilities_ie(pst_mac_ap, &puc_asoc_rsp, uc_ie_len, pst_mac_user, pst_txbf_cap);

    if (pst_mac_user->st_vht_hdl.en_vht_capable == OAL_TRUE) {
        /* 设置 VHT Capabilities IE */
        mac_set_vht_capabilities_ie((oal_void *)pst_mac_ap, puc_asoc_rsp, &uc_ie_len);
        puc_asoc_rsp += uc_ie_len;

        /* 设置 VHT Opern IE */
        mac_set_vht_opern_ie((oal_void *)pst_mac_ap, puc_asoc_rsp, &uc_ie_len);
        puc_asoc_rsp += uc_ie_len;
    }

#ifdef _PRE_WLAN_FEATURE_TXBF
    mac_set_11ntxbf_vendor_ie((oal_void *)pst_mac_ap, puc_asoc_rsp, &uc_ie_len);
    puc_asoc_rsp += uc_ie_len;
#endif

    /* 填充WPS信息 */
    mac_add_app_ie((oal_void *)pst_mac_ap, puc_asoc_rsp, &us_app_ie_len, OAL_APP_ASSOC_RSP_IE);
    puc_asoc_rsp += us_app_ie_len;
    /* 填充epigram Vendor VHT IE,解决与STA的私有协议对通问题 */
    if (pst_hmac_user->en_user_epigram_vht_capable == OAL_TRUE) {
        mac_set_epigram_vht_ie((oal_void *)pst_mac_ap, puc_asoc_rsp, &uc_ie_len);
        puc_asoc_rsp += uc_ie_len;
    }
    /* 5G时，epigram私有vendor ie中不携带vht,需按照此格式组帧 */
    if (pst_hmac_user->en_user_epigram_novht_capable == OAL_TRUE) {
        mac_set_epigram_novht_ie((oal_void *)pst_mac_ap, puc_asoc_rsp, &uc_ie_len);
        puc_asoc_rsp += uc_ie_len;
    }
#ifdef _PRE_WLAN_NARROW_BAND
    if ((g_hitalk_status & NARROW_BAND_ON_MASK) && (pst_mac_ap->st_nb.en_open)) {
            mac_set_nb_ie(puc_asoc_rsp, &uc_ie_len);
            puc_asoc_rsp += uc_ie_len;
    }
#endif

    us_asoc_rsp_len = (oal_uint32)(puc_asoc_rsp - puc_asoc_rsp_original);

    if (en_status_code != MAC_SUCCESSFUL_STATUSCODE) {
#ifdef _PRE_WLAN_1102A_CHR
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, en_status_code);
#endif
    }

    return us_asoc_rsp_len;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_mgmt_is_challenge_txt_equal(oal_uint8 *puc_data,
                                                                           oal_uint8 *puc_chtxt,
                                                                           oal_uint32 ul_chtxt_size)
{
    oal_uint8  *puc_ch_text     = 0;
    oal_uint16  us_idx          = 0;
    oal_uint8   uc_ch_text_len;

    if ((puc_data == OAL_PTR_NULL) || (puc_chtxt == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY, "{hmac_mgmt_is_challenge_txt_equal::puc_data[0x%x] or puc_chtxt[0x%x] is NULL.}",
                       (uintptr_t)puc_data, (uintptr_t)puc_chtxt);
        return OAL_FALSE;
    }

    /* Challenge Text Element */
    /* --------------------------------------- */
    /* |Element ID | Length | Challenge Text | */
    /* --------------------------------------- */
    /* | 1         |1       |1 - 253         | */
    /* --------------------------------------- */
    uc_ch_text_len = puc_data[1];
    puc_ch_text = puc_data + 2; /* puc_data偏移2字节是Challenge Text */
    if (uc_ch_text_len > ul_chtxt_size) {
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


oal_err_code_enum hmac_encap_auth_rsp_get_user_idx(mac_vap_stru *pst_mac_vap,
                                                   oal_uint8 *puc_mac_addr,
                                                   oal_uint8   uc_mac_len,
                                                   oal_uint8 uc_is_seq1,
                                                   oal_uint8 *puc_auth_resend,
                                                   oal_uint16 *pus_user_index)
{
    oal_uint32                      ul_ret;
    hmac_vap_stru                  *pst_hmac_vap;
    hmac_user_stru                 *pst_hmac_user = OAL_PTR_NULL;
    hmac_user_stru                 *pst_user = OAL_PTR_NULL;
    mac_vap_stru                   *pst_vap = OAL_PTR_NULL;
    oal_uint16                      us_user_idx;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if ((pst_hmac_vap == OAL_PTR_NULL) || (uc_mac_len != ETHER_ADDR_LEN)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_encap_auth_rsp_get_user_idx::get_hmac_vap failed!}");
        return OAL_FAIL;
    }

    *puc_auth_resend = OAL_FALSE;
    ul_ret = mac_vap_find_user_by_macaddr(&(pst_hmac_vap->st_vap_base_info), puc_mac_addr, pus_user_index);
    /* 找到用户 */
    if (ul_ret == OAL_SUCC) {
        /* 获取hmac用户的状态，如果不是0，说明是重复帧 */
        pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(*pus_user_index);
        if (pst_hmac_user == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_encap_auth_rsp_get_user_idx::get_hmac_user nul}");
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
            if (pst_net_device != OAL_PTR_NULL) {
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
                oal_kobject_uevent_env_sta_leave(pst_net_device, puc_mac_addr);
#endif
#endif
            }
        }

        return OAL_SUCC;
    }

    /* 若在同一device下的其他VAP下找到给用户，删除之。否则导致业务不通。在DBAC下尤其常见 */
    if (mac_device_find_user_by_macaddr(&pst_hmac_vap->st_vap_base_info, puc_mac_addr, &us_user_idx) == OAL_SUCC) {
        pst_user = mac_res_get_hmac_user(us_user_idx);
        if (pst_user != OAL_PTR_NULL) {
            pst_vap = mac_res_get_mac_vap(pst_user->st_user_base_info.uc_vap_id);
            if (pst_vap != OAL_PTR_NULL) {
                hmac_user_del(pst_vap, pst_user);
            }
        }
    }

    /* 在收到第一个认证帧时用户已创建 */
    if (!uc_is_seq1) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{authrspgetuseridx::user shouldhavebeen added at seq1!}");
        return OAL_FAIL;
    }

    ul_ret = hmac_user_add(pst_mac_vap, puc_mac_addr, pus_user_index);
    if (ul_ret != OAL_SUCC) {
        if (ul_ret == OAL_ERR_CODE_CONFIG_EXCEED_SPEC) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{vap_add_assoc_user fail,users exceed config spec.}");
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        } else {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{add_assoc_user fail,user_idx[%d]}", *pus_user_index);
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}


hmac_ap_auth_process_code_enum_uint8 hmac_encap_auth_rsp_seq1(mac_vap_stru *pst_mac_vap,
                                                              hmac_auth_rsp_param_stru *pst_auth_rsp_param,
                                                              oal_uint8 *puc_code,
                                                              mac_user_asoc_state_enum_uint8 *pst_usr_ass_stat)
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
#ifdef _PRE_WLAN_1102A_CHR
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, *puc_code);
#endif
        return HMAC_AP_AUTH_BUTT;
    }

    /* 检查用户状态 */
    if ((pst_auth_rsp_param->en_user_asoc_state == MAC_USER_STATE_ASSOC) &&
        (pst_auth_rsp_param->us_auth_type == WLAN_WITP_AUTH_OPEN_SYSTEM)) {
        if (pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAMFPC == OAL_TRUE) {
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

#ifdef _PRE_WLAN_1102A_CHR
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                         CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, *puc_code);
#endif
    return HMAC_AP_AUTH_BUTT;
}

hmac_ap_auth_process_code_enum_uint8 hmac_encap_auth_rsp_seq3(mac_vap_stru *pst_mac_vap,
                                                              hmac_auth_rsp_param_stru *pst_auth_rsp_param,
                                                              oal_uint8 *puc_code,
                                                              mac_user_asoc_state_enum_uint8 *pst_usr_ass_stat)
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
        if (pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAMFPC == OAL_TRUE) {
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
#ifdef _PRE_WLAN_1102A_CHR
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                         CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, *puc_code);
#endif
    return HMAC_AP_AUTH_BUTT;
}


hmac_auth_rsp_fun hmac_encap_auth_rsp_get_func(oal_uint16 us_auth_seq)
{
    hmac_auth_rsp_fun st_auth_rsp_fun = OAL_PTR_NULL;
    switch (us_auth_seq) {
        case WLAN_AUTH_TRASACTION_NUM_ONE:
            st_auth_rsp_fun = hmac_encap_auth_rsp_seq1;
            break;
        case WLAN_AUTH_TRASACTION_NUM_THREE:
            st_auth_rsp_fun = hmac_encap_auth_rsp_seq3;
            break;
        default:
            st_auth_rsp_fun = OAL_PTR_NULL;
            break;
    }
    return st_auth_rsp_fun;
}


oal_uint32 hmac_encap_auth_rsp_support(hmac_vap_stru *pst_hmac_vap, oal_uint16 us_auth_type)
{
    if (pst_hmac_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 检测认证类型是否支持 不支持的话状态位置成UNSUPT_ALG */
    if (((pst_hmac_vap->en_auth_mode) != us_auth_type) &&
        (pst_hmac_vap->en_auth_mode != WLAN_WITP_AUTH_AUTOMATIC)) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }
    return OAL_SUCC;
}


oal_void hmac_tid_clear(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user)
{
    mac_device_stru   *pst_device = OAL_PTR_NULL;
    oal_uint8          uc_loop;
    hmac_amsdu_stru   *pst_amsdu = OAL_PTR_NULL;
    oal_netbuf_stru   *pst_amsdu_net_buf = OAL_PTR_NULL;
    hmac_tid_stru     *pst_tid = OAL_PTR_NULL;

#ifdef _PRE_WLAN_FEATURE_AMPDU_VAP
    hmac_vap_stru     *pst_hmac_vap;
#endif

    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_AUTH, "{hmac_tid_clear::pst_hmac_user is null.}");
        return;
    }
#ifdef _PRE_WLAN_FEATURE_AMPDU_VAP
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_tid_clear::pst_hmac_vap null.}");
        return;
    }
#endif
    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_tid_clear::pst_device null.}");
        return;
    }

    for (uc_loop = 0; uc_loop < WLAN_WME_MAX_TID_NUM; uc_loop++) {
        pst_amsdu = &(pst_hmac_user->ast_hmac_amsdu[uc_loop]);

        /* tid锁, 禁软中断 */
        oal_spin_lock_bh(&pst_amsdu->st_amsdu_lock);

        if (pst_amsdu->st_amsdu_timer.en_is_registerd == OAL_TRUE) {
            frw_immediate_destroy_timer(&(pst_amsdu->st_amsdu_timer));
        }

        /* 清空聚合队列 */
        if (pst_amsdu->uc_msdu_num != 0) {
            while (oal_netbuf_list_empty(&pst_amsdu->st_msdu_head) != OAL_TRUE) {
                pst_amsdu_net_buf = oal_netbuf_delist(&(pst_amsdu->st_msdu_head));

                oal_netbuf_free(pst_amsdu_net_buf);
            }
            pst_amsdu->uc_msdu_num = 0;
        }

        /* tid解锁, 使能软中断 */
        oal_spin_unlock_bh(&pst_amsdu->st_amsdu_lock);

        pst_tid = &(pst_hmac_user->ast_tid_info[uc_loop]);

        if (pst_tid->st_ba_tx_info.st_addba_timer.en_is_registerd == OAL_TRUE) {
            frw_immediate_destroy_timer(&(pst_tid->st_ba_tx_info.st_addba_timer));
        }

        pst_tid->uc_tid_no = (oal_uint8)uc_loop;
        pst_tid->us_hmac_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;

        /* 清除接收方向会话句柄 */
        if (pst_tid->pst_ba_rx_info != OAL_PTR_NULL) {
            hmac_ba_reset_rx_handle(pst_device, &pst_tid->pst_ba_rx_info, uc_loop, OAL_TRUE);
        }

        if (pst_tid->st_ba_tx_info.en_ba_status != DMAC_BA_INIT) {
            /* 存在TX BA会话句柄，要-- */
#ifdef _PRE_WLAN_FEATURE_AMPDU_VAP
            hmac_tx_ba_session_decr(pst_hmac_vap, uc_loop);
#else
            hmac_tx_ba_session_decr(pst_device, uc_loop);
#endif
        }

        /* 初始化ba tx操作句柄 */
        pst_tid->st_ba_tx_info.en_ba_status = DMAC_BA_INIT;
        pst_tid->st_ba_tx_info.uc_addba_attemps = 0;
        pst_tid->st_ba_tx_info.uc_dialog_token = 0;
        pst_tid->st_ba_tx_info.uc_ba_policy = 0;
        pst_tid->st_ba_tx_info.en_ba_switch = OAL_TRUE;
        pst_hmac_user->auc_ba_flag[uc_loop] = 0;
    }
}
void hmac_set_auth_resp_header(mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_auth_req,
    oal_uint8 *puc_data, oal_uint8 *puc_addr2, oal_uint32 ul_addr_size)
{
   /* 设置函数头的frame control字段 */
    mac_hdr_set_frame_control(puc_data, WLAN_FC0_SUBTYPE_AUTH);

    /* 获取STA的地址 */
    mac_get_address2(oal_netbuf_header(pst_auth_req), puc_addr2);

    /* 将DA设置为STA的地址 */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_data)->auc_address1, puc_addr2);

    /* 将SA设置为dot11MacAddress */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_data)->auc_address2,
                     pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_data)->auc_address3, pst_mac_vap->auc_bssid);
}
void hmac_set_auth_resp_body(oal_uint8 *puc_frame, oal_uint16 us_auth_type, oal_uint16 us_auth_seq)
{
   /* 设置认证类型IE */
    puc_frame[0] = (us_auth_type & 0x00FF);
    puc_frame[1] = (us_auth_type & 0xFF00) >> 8; /* puc_frame[1]为us_auth_type的高8位 */

    /* 将收到的transaction number + 1后复制给新的认证响应帧 */
    puc_frame[2] = ((us_auth_seq + 1) & 0x00FF); /* puc_frame[2]为us_auth_seq + 1的低8位 */
    puc_frame[3] = ((us_auth_seq + 1) & 0xFF00) >> 8;  /* puc_frame[3]为us_auth_seq + 1的高8位 */

    /* 状态为初始化为成功 */
    puc_frame[4] = MAC_SUCCESSFUL_STATUSCODE; /* puc_frame偏移4、5byte是Status Code(第1字节是关联帧状态码，第2字节置零) */
    puc_frame[5] = 0;
}

void hmac_process_auth_resp_mac_zero_fail(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_frame, mac_tx_ctl_stru *pst_tx_ctl,
    oal_uint16 us_auth_rsp_len)
{
    oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_AUTH, "{hmac_encap_auth_rsp::user mac is all 0 and invaild!}");
    puc_frame[4] = MAC_UNSPEC_FAIL; /* MAC_UNSPEC_FAIL赋值给puc_frame偏移4byte(认证和关联帧状态码) */
#ifdef _PRE_WLAN_1102A_CHR
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
        /* puc_frame[4](认证和关联帧状态码)作为异常上报的参数 */
        CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, puc_frame[4]);
#endif
    pst_tx_ctl->us_tx_user_idx = 0xffff;
    pst_tx_ctl->us_mpdu_len = us_auth_rsp_len;
}

void hmac_process_auth_resp_get_user_idx_fail(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_frame,
    mac_tx_ctl_stru *pst_tx_ctl, oal_uint16 us_auth_rsp_len, oal_uint32 ul_ret)
{
    if (ul_ret == OAL_ERR_CODE_CONFIG_EXCEED_SPEC) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_AUTH, "{hmac_encap_auth_rsp::users exceed config spec!}");
        puc_frame[4] = MAC_AP_FULL; /* MAC_AP_FULL赋值给puc_frame偏移4byte(认证和关联帧状态码) */
    } else {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_AUTH, "{hmac_encap_auth_rsp::hmac_ap_get_user_idx Err!}");
        puc_frame[4] = MAC_UNSPEC_FAIL; /* MAC_UNSPEC_FAIL赋值给puc_frame偏移4byte(认证和关联帧状态码) */
    }
#ifdef _PRE_WLAN_1102A_CHR
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
        /* puc_frame[4](认证和关联帧状态码)作为异常上报的参数 */
        CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, puc_frame[4]);
#endif
    pst_tx_ctl->us_tx_user_idx = 0xffff;
    pst_tx_ctl->us_mpdu_len = us_auth_rsp_len;  
}
void hmac_process_auth_resp_get_user_fail(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_frame, mac_tx_ctl_stru *pst_tx_ctl,
    oal_uint16 us_auth_rsp_len, oal_uint16 us_user_index)
{
    OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_AUTH, "{hmac_encap_auth_rsp::usersta[%d] is NUL}", us_user_index);
    puc_frame[4] = MAC_UNSPEC_FAIL; /* MAC_UNSPEC_FAIL赋值给puc_frame偏移4byte(认证和关联帧状态码) */
#ifdef _PRE_WLAN_1102A_CHR
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
        /* puc_frame[4](认证和关联帧状态码)作为异常上报的参数 */
        CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, puc_frame[4]);
#endif
    pst_tx_ctl->us_tx_user_idx = 0xffff;
    pst_tx_ctl->us_mpdu_len = us_auth_rsp_len;    
}
void hmac_process_auth_resp_get_vap_fail(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_frame, mac_tx_ctl_stru *pst_tx_ctl,
    hmac_user_stru *pst_user_sta, oal_uint16 us_auth_rsp_len)
{
    OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_AUTH, "{pst_hmac_vap is NUL,and change useridx=%d state to BUTT}",
        pst_user_sta->st_user_base_info.us_assoc_id);
    puc_frame[4] = MAC_UNSPEC_FAIL; /* MAC_UNSPEC_FAIL赋值给puc_frame偏移4byte(认证和关联帧状态码) */
#ifdef _PRE_WLAN_1102A_CHR
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
        /* puc_frame[4](认证和关联帧状态码)作为异常上报的参数 */
        CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, puc_frame[4]);
#endif
    pst_tx_ctl->us_tx_user_idx = 0xffff;
    pst_tx_ctl->us_mpdu_len = us_auth_rsp_len;
    mac_user_set_asoc_state(&(pst_user_sta->st_user_base_info), MAC_USER_STATE_BUTT);   
}
void hmac_process_auth_resp_alg_unsupport(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_frame, mac_tx_ctl_stru *pst_tx_ctl,
    hmac_user_stru *pst_user_sta, hmac_vap_stru *pst_hmac_vap)
{
    puc_frame[4] = MAC_UNSUPT_ALG; /* MAC_UNSUPT_ALG赋值给puc_frame偏移4byte(认证和关联帧状态码) */
#ifdef _PRE_WLAN_1102A_CHR
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
        /* puc_frame[4](认证和关联帧状态码)作为异常上报的参数 */
        CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, puc_frame[4]);
#endif
    hmac_user_set_asoc_state(&(pst_hmac_vap->st_vap_base_info), &pst_user_sta->st_user_base_info,
        MAC_USER_STATE_BUTT);
}
void hmac_process_auth_seq1_seq3(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_user_sta, oal_uint8 *puc_frame,
    hmac_auth_rsp_handle_stru *pst_auth_rsp_handle, hmac_ap_auth_process_code_enum_uint8 *pul_auth_proc_rst)
{
    if (pst_auth_rsp_handle->st_auth_rsp_fun != OAL_PTR_NULL) {
        *pul_auth_proc_rst = pst_auth_rsp_handle->st_auth_rsp_fun(pst_mac_vap,
                                                                  &(pst_auth_rsp_handle->st_auth_rsp_param),
                                                                  &puc_frame[4], /* puc_frame偏移4byte(认证和关联帧状态码) */
                                                                  &pst_user_sta->st_user_base_info.en_user_asoc_state);
        /* 清空 HMAC层TID信息 */
        hmac_tid_clear(pst_mac_vap, pst_user_sta);
    } else {
        *pul_auth_proc_rst = HMAC_AP_AUTH_BUTT;
        mac_user_set_asoc_state(&pst_user_sta->st_user_base_info, MAC_USER_STATE_BUTT);
        puc_frame[4] = MAC_AUTH_SEQ_FAIL; /* MAC_AUTH_SEQ_FAIL赋值给puc_frame偏移4byte(认证和关联帧状态码) */
    }
}
void hmac_process_auth_seq1_open(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_user_sta)
{
    hmac_user_init_key(&pst_user_sta->st_user_base_info);
    frw_create_timer(&pst_user_sta->st_mgmt_timer,
                     hmac_mgmt_timeout_ap,
                     WLAN_AUTH_AP_TIMEOUT,
                     pst_user_sta,
                     OAL_FALSE,
                     OAM_MODULE_ID_HMAC,
                     pst_mac_vap->ul_core_id);
}
void hmac_process_auth_seq1_wep_no_resend(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_chtxt,
    hmac_user_stru *pst_user_sta, oal_uint8 *puc_frame, oal_uint16 *pus_auth_rsp_len)
{
    /* len is 6 */
    hmac_config_11i_add_wep_entry(pst_mac_vap, 6, pst_user_sta->st_user_base_info.auc_user_mac_addr);

    hmac_mgmt_encap_chtxt(puc_frame, puc_chtxt, pus_auth_rsp_len, pst_user_sta);
    /* 为该用户启动一个定时器，超时认证失败 */
    frw_create_timer(&pst_user_sta->st_mgmt_timer,
                     hmac_mgmt_timeout_ap,
                     WLAN_AUTH_AP_TIMEOUT,
                     pst_user_sta,
                     OAL_FALSE,
                     OAM_MODULE_ID_HMAC,
                     pst_mac_vap->ul_core_id);
    pst_user_sta->st_user_base_info.st_key_info.en_cipher_type =
        mac_get_wep_type(pst_mac_vap, mac_get_wep_default_keyid(pst_mac_vap));
}
void hmac_process_auth_seq1_wep_resend(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_chtxt,
    hmac_user_stru *pst_user_sta, oal_uint8 *puc_frame, oal_uint16 *pus_auth_rsp_len)
{
    /* seq为1 的认证帧重传 */
    hmac_mgmt_encap_chtxt(puc_frame, puc_chtxt, pus_auth_rsp_len, pst_user_sta);

    if (pst_hmac_vap->st_mgmt_timer.en_is_registerd == OAL_TRUE) {
        /* 重启超时定时器 */
        frw_timer_restart_timer(&pst_user_sta->st_mgmt_timer,
                                pst_user_sta->st_mgmt_timer.ul_timeout,
                                OAL_FALSE);
    }   
}
void hmac_process_auth_seq3_wep_complete(mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_auth_req,
    hmac_user_stru  *pst_user_sta, oal_uint8 *puc_frame)
{
    oal_uint8 *puc_chtxt = NULL;
    puc_chtxt = mac_get_auth_ch_text(oal_netbuf_header(pst_auth_req));
    if (hmac_mgmt_is_challenge_txt_equal(puc_chtxt,
        pst_user_sta->auc_ch_text, OAL_SIZEOF(pst_user_sta->auc_ch_text)) == OAL_TRUE) {
        mac_user_set_asoc_state(&pst_user_sta->st_user_base_info, MAC_USER_STATE_AUTH_COMPLETE);

        /* cancel timer for auth */
        frw_immediate_destroy_timer(&pst_user_sta->st_mgmt_timer);
    } else {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_AUTH, "{encap_auth_rsp::challenage txt not equal.}");
        puc_frame[4] = MAC_CHLNG_FAIL; /* MAC_CHLNG_FAIL赋值给puc_frame偏移4byte(认证和关联帧状态码) */
        mac_user_set_asoc_state(&pst_user_sta->st_user_base_info, MAC_USER_STATE_BUTT);
    }
}
void hmac_process_auth_seq3_wep_assoc(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_chtxt, hmac_user_stru *pst_user_sta,
    oal_uint8 *puc_frame, oal_uint16 *pus_auth_rsp_len)
{
    hmac_mgmt_encap_chtxt(puc_frame, puc_chtxt, pus_auth_rsp_len, pst_user_sta);

    /* 开启超时定时器 */
    frw_create_timer(&pst_user_sta->st_mgmt_timer,
        hmac_mgmt_timeout_ap, WLAN_AUTH_AP_TIMEOUT, pst_user_sta, OAL_FALSE,
        OAM_MODULE_ID_HMAC, pst_mac_vap->ul_core_id);
}
void hmac_process_default(hmac_user_stru *pst_user_sta)
{
    hmac_user_init_key(&pst_user_sta->st_user_base_info);
    pst_user_sta->st_user_base_info.en_user_asoc_state = MAC_USER_STATE_BUTT;  
}

oal_uint16 hmac_encap_auth_rsp(mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_auth_rsp,
                               oal_netbuf_stru *pst_auth_req, oal_uint8 *puc_chtxt)
{
    oal_uint16       us_auth_rsp_len        = 0;
    hmac_user_stru  *pst_user_sta      = OAL_PTR_NULL;
    oal_uint8       *puc_frame              = OAL_PTR_NULL;
    oal_uint16       us_index;
    oal_uint16       us_auth_type;
    oal_uint8        uc_is_seq1;
    oal_uint16       us_auth_seq;
    oal_uint8        auc_addr2[6]           = {0};
    hmac_vap_stru   *pst_hmac_vap = OAL_PTR_NULL;
    oal_uint16       us_user_index          = 0xffff;
    oal_uint32       ul_ret;

    /* 认证方法 */
    hmac_ap_auth_process_code_enum_uint8 ul_auth_proc_rst;

    oal_uint8 *puc_data = OAL_PTR_NULL;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;
    hmac_auth_rsp_handle_stru st_auth_rsp_handle;
    oal_uint32 ul_alg_suppt;

    if (oal_any_null_ptr4(pst_mac_vap, pst_auth_rsp, pst_auth_req, puc_chtxt)) {
        oam_error_log4(0, OAM_SF_AUTH, "{pst_vap[0x%x], puc_data[0x%x], puc_auth_req[0x%x] and puc_chtxt[0x%x]}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)pst_auth_rsp, (uintptr_t)pst_auth_req, (uintptr_t)puc_chtxt);
        return us_auth_rsp_len;
    }

    puc_data = (oal_uint8 *)oal_netbuf_header(pst_auth_rsp);
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_auth_rsp);

    /*************************************************************************/
    /* Management Frame Format */
    /* -------------------------------------------------------------------- */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS| */
    /* -------------------------------------------------------------------- */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  | */
    /* -------------------------------------------------------------------- */
    /*************************************************************************/
    /*************************************************************************/
    /* Set the fields in the frame header */
    /*************************************************************************/
    hmac_set_auth_resp_header(pst_mac_vap, pst_auth_req, puc_data, auc_addr2, sizeof(auc_addr2));

    /*************************************************************************/
    /* Set the contents of the frame body */
    /*************************************************************************/
    /*************************************************************************/
    /* Authentication Frame - Frame Body */
    /* --------------------------------------------------------------------- */
    /* |Auth Algo Number|Auth Trans Seq Number|Status Code| Challenge Text | */
    /* --------------------------------------------------------------------- */
    /* | 2              |2                    |2          | 3 - 256        | */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    us_index = MAC_80211_FRAME_LEN;
    puc_frame = (oal_uint8 *)(puc_data + us_index);

    /* 计算认证相应帧的长度 */
    us_auth_rsp_len = MAC_80211_FRAME_LEN + MAC_AUTH_ALG_LEN + MAC_AUTH_TRANS_SEQ_NUM_LEN +
                      MAC_STATUS_CODE_LEN;

    /* 解析认证类型 */
    us_auth_type = mac_get_auth_algo_num(pst_auth_req);

    /* 解析auth transaction number */
    us_auth_seq = mac_get_auth_seq_num(oal_netbuf_header(pst_auth_req));
    if (us_auth_seq > HMAC_AP_AUTH_SEQ3_WEP_COMPLETE) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_AUTH, "{auth recieve invalid seq, auth seq [%d]}", us_auth_seq);
        return 0;
    }

    /* 设置auth resp帧体 */
    hmac_set_auth_resp_body(puc_frame, us_auth_type, us_auth_seq);

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_AUTH, "{hmac_encap_auth_rsp::user mac:%02X:XX:XX:%02X:%02X:%02X}",
                     auc_addr2[0], auc_addr2[3], auc_addr2[4], auc_addr2[5]); /* auc_addr2第0、3、4、5byte参数输出打印 */

    if (mac_addr_is_zero(auc_addr2)) {
        hmac_process_auth_resp_mac_zero_fail(pst_mac_vap, puc_frame, pst_tx_ctl, us_auth_rsp_len);
        return us_auth_rsp_len;
    }

    /* 获取用户idx */
    uc_is_seq1 = (us_auth_seq == WLAN_AUTH_TRASACTION_NUM_ONE);
    ul_ret = hmac_encap_auth_rsp_get_user_idx(pst_mac_vap, auc_addr2, sizeof(auc_addr2), uc_is_seq1,
                                              &st_auth_rsp_handle.st_auth_rsp_param.uc_auth_resend, &us_user_index);
    if (ul_ret != OAL_SUCC) {
        hmac_process_auth_resp_get_user_idx_fail(pst_mac_vap, puc_frame, pst_tx_ctl, us_auth_rsp_len, ul_ret);
        return us_auth_rsp_len;
    }

    /* 获取hmac user指针 */
    pst_user_sta = mac_res_get_hmac_user(us_user_index);
    if (pst_user_sta == OAL_PTR_NULL) {
        hmac_process_auth_resp_get_user_fail(pst_mac_vap, puc_frame, pst_tx_ctl, us_auth_rsp_len, us_user_index);
        return us_auth_rsp_len;
    }

    /* 获取hmac vap指针 */
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        hmac_process_auth_resp_get_vap_fail(pst_mac_vap, puc_frame, pst_tx_ctl, pst_user_sta, us_auth_rsp_len);
        return us_auth_rsp_len;
    }
    pst_tx_ctl->us_tx_user_idx = us_user_index;
    pst_tx_ctl->us_mpdu_len = us_auth_rsp_len;

    /* 判断算法是否支持 */
    ul_alg_suppt = hmac_encap_auth_rsp_support(pst_hmac_vap, us_auth_type);
    if (ul_alg_suppt != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_AUTH, "{auth type[%d] not support!}", us_auth_type);
        hmac_process_auth_resp_alg_unsupport(pst_mac_vap, puc_frame, pst_tx_ctl, pst_user_sta, pst_hmac_vap);
        return us_auth_rsp_len;
    }

    /* 初始化处理参数 */
    st_auth_rsp_handle.st_auth_rsp_param.en_is_wep_allowed = mac_is_wep_allowed(pst_mac_vap);
    st_auth_rsp_handle.st_auth_rsp_param.en_user_asoc_state = pst_user_sta->st_user_base_info.en_user_asoc_state;
    st_auth_rsp_handle.st_auth_rsp_param.us_auth_type = us_auth_type;
    st_auth_rsp_handle.st_auth_rsp_fun = hmac_encap_auth_rsp_get_func(us_auth_seq);

    /* 处理seq1或者seq3 */
    hmac_process_auth_seq1_seq3(pst_mac_vap, pst_user_sta, puc_frame, &st_auth_rsp_handle, &ul_auth_proc_rst);

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_AUTH, "{hmac_encap_auth_rsp::auth_proc_rst:%d}", ul_auth_proc_rst);

    /* 根据返回的code进行后续处理 */
    switch (ul_auth_proc_rst) {
        case HMAC_AP_AUTH_SEQ1_OPEN_ANY: {
            hmac_process_auth_seq1_open(pst_mac_vap, pst_user_sta);
            break;
        }
        case HMAC_AP_AUTH_SEQ1_WEP_NOT_RESEND: {
            hmac_process_auth_seq1_wep_no_resend(pst_mac_vap, puc_chtxt, pst_user_sta, puc_frame, &us_auth_rsp_len);
            break;
        }
        case HMAC_AP_AUTH_SEQ1_WEP_RESEND: {
            hmac_process_auth_seq1_wep_resend(pst_hmac_vap, puc_chtxt, pst_user_sta, puc_frame, &us_auth_rsp_len);
            break;
        }
        case HMAC_AP_AUTH_SEQ3_OPEN_ANY: {
            hmac_user_init_key(&pst_user_sta->st_user_base_info);
            break;
        }
        case HMAC_AP_AUTH_SEQ3_WEP_COMPLETE: {
            hmac_process_auth_seq3_wep_complete(pst_mac_vap, pst_auth_req, pst_user_sta, puc_frame);
            break;
        }
        case HMAC_AP_AUTH_SEQ3_WEP_ASSOC: {
            hmac_process_auth_seq3_wep_assoc(pst_mac_vap, puc_chtxt, pst_user_sta, puc_frame, &us_auth_rsp_len);
            break;
        }
        case HMAC_AP_AUTH_DUMMY: {
            break;
        }
        case HMAC_AP_AUTH_BUTT:
        default:
        {
            hmac_process_default(pst_user_sta);
            break;
        }
    }

    /* dmac offload架构下，同步user关联状态信息到dmac */
    ul_ret = hmac_config_user_asoc_state_syn(&(pst_hmac_vap->st_vap_base_info), &pst_user_sta->st_user_base_info);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH, "aprxauthreq::statesyn fail[%d]", ul_ret);
    }

    return us_auth_rsp_len;
}


