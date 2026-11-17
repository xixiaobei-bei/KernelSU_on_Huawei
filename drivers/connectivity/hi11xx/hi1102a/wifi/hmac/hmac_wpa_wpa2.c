

/* 1 头文件包含 */
#include "mac_ie.h"
#include "hmac_11i.h"
#include "hmac_wpa_wpa2.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_WPA_WPA2_C

#if defined(_PRE_WLAN_FEATURE_WPA) || defined(_PRE_WLAN_FEATURE_WPA2)

oal_uint32 hmac_check_assoc_req_security_cap_common(mac_vap_stru *pst_mac_vap,
                                                    oal_uint8 *puc_ie,
                                                    oal_uint32 ul_msg_len,
                                                    oal_uint8 uc_80211i_mode,
                                                    oal_uint8 uc_offset,
                                                    mac_status_code_enum_uint16 *pen_status_code)
{
    oal_uint8 auc_pcip_policy[WLAN_PAIRWISE_CIPHER_SUITES] = { 0 };
    oal_uint8 auc_auth_policy[WLAN_AUTHENTICATION_SUITES] = { 0 };
    wlan_mib_ieee802dot11_stru *pst_mib_info = OAL_PTR_NULL;
    oal_uint8 uc_grp_policy;
    OAL_CONST oal_uint8 *puc_oui;
    oal_uint8 uc_index = uc_offset;
    oal_uint8 uc_len;
    oal_uint16 us_pmkid_count;
    oal_uint16 us_pmkid_idx;

    pst_mib_info = pst_mac_vap->pst_mib_info;

    puc_oui = hmac_get_security_oui(uc_80211i_mode);
    if (puc_oui == OAL_PTR_NULL) {
        *pen_status_code = MAC_UNSUP_RSN_INFO_VER;
        return OAL_FAIL;
    }

    /* 检测版本信息 */
    if (oal_make_word16(puc_ie[uc_index], puc_ie[uc_index + 1]) != MAC_RSN_IE_VERSION) {
        *pen_status_code = MAC_UNSUP_RSN_INFO_VER;
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_check_assoc_req_security_cap_common::unsup rsn version=%d.}",
                       oal_make_word16(puc_ie[uc_index], puc_ie[uc_index + 1]));
        return OAL_FAIL;
    }

    /* 忽略版本信息(2字节)  */
    uc_index += 2;

    if (oal_memcmp(puc_oui, puc_ie + uc_index, MAC_OUI_LEN) != 0) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_check_assoc_req_security_cap_common::invalid group OUI.}");
    }
    uc_index += MAC_OUI_LEN;
    uc_grp_policy = puc_ie[uc_index++]; /* 获取组播密钥 */

    /* 获取单播加密套件 */
    uc_len = hmac_get_pcip_policy_auth(puc_ie + uc_index, auc_pcip_policy);
    if (uc_len == 0) {
        *pen_status_code = MAC_INVALID_PW_CIPHER;
        return OAL_FAIL;
    }

    uc_index += uc_len;

    /* 获取认证套件信息 */
    uc_len = hmac_get_auth_policy_auth(puc_ie + uc_index, auc_auth_policy);
    if (uc_len == 0) {
        *pen_status_code = MAC_INVALID_AKMP_CIPHER;
        return OAL_FAIL;
    }

    uc_index += uc_len;

    /* 成对密钥套件是否为组播密钥套件 */
    if (auc_pcip_policy[0] == WLAN_80211_CIPHER_SUITE_GROUP_CIPHER) {
        auc_pcip_policy[0] = uc_grp_policy;
    }

    /* 检测组播套件 */
    if (mac_check_group_policy(pst_mac_vap, uc_grp_policy, uc_80211i_mode) != OAL_SUCC) {
        *pen_status_code = MAC_INVALID_GRP_CIPHER;
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_check_assoc_req_security_cap_common::invalid group[%d].}", uc_grp_policy);
        return OAL_FAIL;
    }

    /* 检测单播密钥套件 */
    if (hmac_check_pcip_policy(pst_mac_vap, auc_pcip_policy, uc_80211i_mode) != OAL_SUCC) {
        *pen_status_code = MAC_INVALID_PW_CIPHER;
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_check_assoc_req_security_cap_common::invalid pcip[%d].}", auc_pcip_policy[0]);
        return OAL_FAIL;
    }

    /* 检测认证套件 */
    if (mac_check_auth_policy(pst_mac_vap, auc_auth_policy, uc_80211i_mode) != OAL_SUCC) {
        *pen_status_code = MAC_INVALID_AKMP_CIPHER;
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_check_assoc_req_security_cap_common::invalid auth[%d].}", auc_auth_policy[0]);
        return OAL_FAIL;
    }

    /* 禁止单播套件为TKIP, 组播套件为CCMP */
    if ((uc_grp_policy == WLAN_80211_CIPHER_SUITE_CCMP) &&
        (auc_pcip_policy[0] == WLAN_80211_CIPHER_SUITE_TKIP)) {
        *pen_status_code = MAC_CIPHER_REJ;
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_check_assoc_req_security_cap_common::group and pairwise cipher conflict.}");
        return OAL_FAIL;
    }

    /* 无RSN能力字段 ，前者2为ie的ID和len长度,RSN Capabilities长度2字节 */
    if (MAC_IE_REAMIN_LEN_IS_ENOUGH(2, uc_index, puc_ie[1], 2) == OAL_FALSE) {
        return OAL_SUCC;
    }

    /* 略过RSN能力字段(2字节) */
    uc_index += 2;

    /* 无pmkid count/pmkid list字段，前者2为ie的ID和len长度,pmkid count长度2字节 */
    if (MAC_IE_REAMIN_LEN_IS_ENOUGH(2, uc_index, puc_ie[1], 2) == OAL_FALSE) {
        return OAL_SUCC;
    }

    us_pmkid_count = oal_make_word16(puc_ie[uc_index], puc_ie[uc_index + 1]);
    for (us_pmkid_idx = 0; us_pmkid_idx < us_pmkid_count; us_pmkid_idx++) {
        /* 前者2为ie的ID和len长度,PMKID List一组长度是16字节 */
        if (MAC_IE_REAMIN_LEN_IS_ENOUGH(2, uc_index, puc_ie[1], 16) == OAL_FALSE) {
            *pen_status_code = MAC_INVALID_PMKID;
            return OAL_FAIL;
        }

        uc_index += 16; /* PMKID List一组长度是16字节 */
    }

    return OAL_SUCC;
}


oal_uint32 hmac_check_assoc_req_security_cap_authenticator(
    hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_mac_hdr, oal_uint8 *puc_payload,
    oal_uint32 ul_msg_len, hmac_user_stru *pst_hmac_user, mac_status_code_enum_uint16 *pen_status_code)
{
    oal_uint8 uc_pcip_policy = WLAN_80211_CIPHER_SUITE_NO_ENCRYP;
    oal_uint8 uc_grp_policy = WLAN_80211_CIPHER_SUITE_NO_ENCRYP;
    oal_uint8 uc_80211i_mode = 0;
    oal_uint8 *puc_ie = OAL_PTR_NULL;
    oal_uint8 *puc_rsn_ie = OAL_PTR_NULL;
    oal_uint8 *puc_wpa_ie = OAL_PTR_NULL;
    oal_uint8 uc_index = 0;
    oal_bool_enum_uint8 en_rsn_found = OAL_FALSE; /* AP 接收到的ASSOC REQ帧IE 是否匹配 */
    mac_vap_stru *pst_mac_vap = NULL;
    mac_user_stru *pst_mac_user = NULL;
    wlan_mib_ieee802dot11_stru *pst_mib_info = NULL;
    oal_uint32 ul_ret;
    oal_uint8 uc_offset;

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    pst_mib_info = pst_mac_vap->pst_mib_info;

    if (mac_mib_get_rsnaactivated(pst_mac_vap) != OAL_TRUE) {
        return OAL_SUCC;
    }

    uc_offset = MAC_CAP_INFO_LEN + MAC_LIS_INTERVAL_IE_LEN;

    if (mac_get_frame_sub_type(puc_mac_hdr) == WLAN_FC0_SUBTYPE_REASSOC_REQ) {
        uc_offset += OAL_MAC_ADDR_LEN;
    }

    /* 获取RSNA和WPA IE信息 */
    puc_rsn_ie = mac_find_ie(MAC_EID_RSN, puc_payload + uc_offset, (oal_int32)(ul_msg_len - uc_offset));
    puc_wpa_ie = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA, puc_payload + uc_offset,
                                    (oal_int32)(ul_msg_len - uc_offset));
    if ((puc_rsn_ie == OAL_PTR_NULL) && (puc_wpa_ie == OAL_PTR_NULL)) {
        if (pst_hmac_vap->en_wps_active == OAL_TRUE) {
            hmac_user_init_key(&pst_hmac_user->st_user_base_info);
            return OAL_SUCC;
        } else {
            *pen_status_code = MAC_INVALID_INFO_ELMNT;

            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                           "{hmac_check_assoc_req_security_cap_authenticator::not WPA/WPA2.}");

            return OAL_FAIL;
        }
    } else { /* RSNA 或 WPA加密方式处理 */
        if (pst_mac_vap->st_cap_flag.bit_wpa2 == OAL_TRUE) {
            if (puc_rsn_ie != OAL_PTR_NULL) {
                en_rsn_found = OAL_TRUE;
                uc_80211i_mode = DMAC_RSNA_802_11I;

                /* 略过 IE + LEN(2字节) */
                uc_index += 2;

                puc_ie = puc_rsn_ie;
            }
        }

        if ((en_rsn_found == OAL_FALSE) &&
            (pst_mac_vap->st_cap_flag.bit_wpa == OAL_TRUE)) {
            if (puc_wpa_ie != OAL_PTR_NULL) {
                en_rsn_found = OAL_TRUE;
                uc_80211i_mode = DMAC_WPA_802_11I;

                uc_index += 6; /* 略过 IE(1字节) + LEN(1字节) + WPA OUI(4字节)，共6字节 */

                puc_ie = puc_wpa_ie;
            } else {
                *pen_status_code = MAC_CIPHER_REJ;
                return OAL_FAIL;
            }
        }

        if ((en_rsn_found == OAL_TRUE) && (puc_ie != OAL_PTR_NULL)) {
            ul_ret = hmac_check_assoc_req_security_cap_common(pst_mac_vap, puc_ie, ul_msg_len,
                                                              uc_80211i_mode, uc_index, pen_status_code);
            if (ul_ret != OAL_SUCC) {
                return OAL_FAIL;
            }
        }
    }

    if (*pen_status_code == MAC_SUCCESSFUL_STATUSCODE) {
        /* 检测出来的这个硬件加密方式应该保存到 mac_user -> en_cipher_type 中 */
        pst_mac_user = &(pst_hmac_user->st_user_base_info);

        /* 保存用户的加密方式 */
        pst_mac_user->st_key_info.en_cipher_type = (uc_pcip_policy);
    }

    oam_info_log3(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_check_assoc_req_security_cap_authenticator::\
                  mode=%d, group=%d, pairwise=%d, auth=%d}\r\n",
                  uc_80211i_mode, uc_grp_policy, uc_pcip_policy);

    return OAL_SUCC;
}
oal_void hmac_set_group_mgmt_policy(mac_bss_80211i_info_stru *pst_bss_80211i_info,
                                    oal_uint8 *puc_ie,
                                    oal_uint8 *puc_index,
                                    oal_int32 *pl_ie_len_left)
{
    oal_uint16 us_pmkid_count;
    if ((*pl_ie_len_left) == 4) { /* 4是固定长度 */
        pst_bss_80211i_info->uc_group_mgmt_policy = puc_ie[(*puc_index) + MAC_OUI_LEN];
        (*puc_index) += 4; /* 4是固定长度 */
        (*pl_ie_len_left) -= 4; /* 4是固定长度 */
    } else if ((*pl_ie_len_left) > 4) { /* 4是固定长度 */
        us_pmkid_count = oal_make_word16(puc_ie[(*puc_index)], puc_ie[(*puc_index) + 1]); /* 1是固定偏移 */
        (*pl_ie_len_left) -= 2; /* 2是固定长度 */
        (*puc_index) += 2; /* 2是固定长度 */
        (*pl_ie_len_left) -= us_pmkid_count * 16; /* 16是固定长度 */
        (*puc_index) += us_pmkid_count * 16; /* 16是固定长度 */
        if ((*pl_ie_len_left) >= 4) { /* 4是固定长度 */
            pst_bss_80211i_info->uc_group_mgmt_policy = puc_ie[(*puc_index) + MAC_OUI_LEN];
            (*puc_index) += 4; /* 4是固定长度 */
            (*pl_ie_len_left) -= 4; /* 4是固定长度 */
        }
    }
}
oal_void hmac_init_cihper_policy(oal_uint8 *puc_pcip_policy,
                                 oal_uint8 *puc_auth_policy,
                                 mac_bss_80211i_info_stru *pst_bss_80211i_info)
{
    memset_s(puc_pcip_policy, MAC_PAIRWISE_CIPHER_SUITES_NUM, 0xFF, MAC_PAIRWISE_CIPHER_SUITES_NUM);
    memset_s(puc_auth_policy, MAC_AUTHENTICATION_SUITE_NUM, 0xFF, MAC_AUTHENTICATION_SUITE_NUM);
    pst_bss_80211i_info->uc_group_mgmt_policy = 0; /* 0是无效值，防止进正常逻辑 */
}

oal_bool_enum_uint8 hmac_parse_cipher_suit(mac_bss_dscr_stru *pst_bss_dscr, oal_uint8 uc_cipher_type,
                                           oal_uint8 *puc_ie, oal_int32 l_len)
{
    oal_uint8 uc_index = 0;
    oal_uint16 us_temp = 0;
    oal_uint16 us_pcip_num = 0;
    oal_uint16 us_auth_num = 0;
    oal_uint16 us_suite_count;

    oal_int32 l_ie_len_left = l_len;

    OAL_CONST oal_uint8 *puc_oui;
    oal_uint8 *puc_pcip_policy = OAL_PTR_NULL;
    oal_uint8 *puc_grp_policy = OAL_PTR_NULL;
    oal_uint8 *puc_auth_policy = OAL_PTR_NULL;
    mac_bss_80211i_info_stru *pst_bss_80211i_info = &(pst_bss_dscr->st_bss_sec_info);

    /*************************************************************************/
    /* RSN Element Format */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Cipher */
    /* --------------------------------------------------------------------- */
    /* |     1     |    1   |    2    |         4          |       2 */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Suite Count| Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /* |         4*m                |     2           |   4*n */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |        2       |    2      |   16 *s  |               4           | */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    /* 解析WPA/RSN公共的cipher suit */
    /* puc_ie指向Group Cipher Suite,l_len指puc_ie长度 */
    if (uc_cipher_type == DMAC_WPA_802_11I) {
        puc_oui = g_auc_wpa_oui;
        puc_grp_policy = &(pst_bss_80211i_info->uc_wpa_grp_policy);
        puc_pcip_policy = pst_bss_80211i_info->auc_wpa_pairwise_policy;
        puc_auth_policy = pst_bss_80211i_info->auc_wpa_auth_policy;
    } else if (uc_cipher_type == DMAC_RSNA_802_11I) {
        puc_oui = g_auc_rsn_oui;
        puc_grp_policy = &(pst_bss_80211i_info->uc_rsn_grp_policy);
        puc_pcip_policy = pst_bss_80211i_info->auc_rsn_pairwise_policy;
        puc_auth_policy = pst_bss_80211i_info->auc_rsn_auth_policy;
    } else {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_parse_cipher_suit::invalid OUI.}");
        return OAL_FALSE;
    }

    hmac_init_cihper_policy(puc_pcip_policy, puc_auth_policy, pst_bss_80211i_info);
    /* 获取组播密钥套件,4个字节 */
    if (l_ie_len_left > 4) {
        if (oal_memcmp(puc_oui, puc_ie + uc_index, MAC_OUI_LEN) != 0) {
            oam_warning_log0(0, OAM_SF_SCAN, "{hmac_parse_cipher_suit::invalid group OUI.}");
            return OAL_FALSE;
        }
        *puc_grp_policy = puc_ie[uc_index + MAC_OUI_LEN];

        uc_index += 4; /* 忽略组播密钥套件长度(4字节) */
        l_ie_len_left -= 4;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{hmac_parse_cipher_suit::ie too short group policy:%d.}", l_ie_len_left);
        return OAL_FALSE;
    }

    /* 获取成对密钥套件个数,最少2个字节长度 */
    if (l_ie_len_left > 2) {
        us_pcip_num = oal_make_word16(puc_ie[uc_index], puc_ie[uc_index + 1]) & 0xFF;
        uc_index += 2; /* Suite Count2字节 */
        l_ie_len_left -= 2;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{hmac_parse_cipher_suit::ie short for pairwise key mgmt:%d.}", l_ie_len_left);
        return OAL_FALSE;
    }

    /* 确认单播套件个数是否与实际ie长度相匹配 */
    if ((us_pcip_num == 0) || (us_pcip_num > l_ie_len_left / 4)) { /* Pairwise Cipher Suite 一组长度是4字节 */
        oam_warning_log2(0, OAM_SF_SCAN, "{hmac_parse_cipher_suit::ie count botch pairwise:%d,left:%d}",
                         us_pcip_num, l_ie_len_left);
        return OAL_FALSE;
    }

    us_suite_count = 0;
    for (us_temp = 0; us_temp < us_pcip_num; us_temp++) {
        if (oal_memcmp(puc_oui, puc_ie + uc_index, MAC_OUI_LEN) != 0) {
            oam_warning_log0(0, OAM_SF_SCAN, "{hmac_parse_cipher_suit::invalid pairwise OUI,ignore this ie.}");
            /* 对于不识别的成对密钥套件，忽略保存 */
            uc_index += 4; /* Pairwise Cipher Suite 1组长度4字节 */
            l_ie_len_left -= 4;
            continue;
        }
        if (us_suite_count < MAC_PAIRWISE_CIPHER_SUITES_NUM) {
            /* 成对密钥套件个数驱动最大为2，超过则不再继续保存 */
            puc_pcip_policy[us_suite_count++] = puc_ie[uc_index + MAC_OUI_LEN];
        } else {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN,
                             "{hmac_parse_cipher_suit::us_suite_count reach max,ignore this ie,us_pcip_num:%d.}",
                             us_pcip_num);
        }

        uc_index += 4; /* Pairwise Cipher Suite 1组长度4字节 */
        l_ie_len_left -= 4;
    }

    /* 获取认证套件计数,最少2个字节长度 */
    if (l_ie_len_left > 2) {
        us_auth_num = oal_make_word16(puc_ie[uc_index], puc_ie[uc_index + 1]) & 0xFF;
        uc_index += 2; /* AKM Suite Count2字节 */
        l_ie_len_left -= 2;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{hmac_parse_cipher_suit::ie too short for aka suite:%d.}", l_ie_len_left);
        return OAL_FALSE;
    }

    /* 确认认证套件个数(l_ie_len_left / 4)是否与实际ie长度相匹配 */
    if ((us_auth_num == 0) || (us_auth_num > l_ie_len_left / 4)) {
        oam_warning_log2(0, OAM_SF_SCAN, "{hmac_parse_cipher_suit::ie count botch aka:%d,left:%d}",
                         us_auth_num, l_ie_len_left);
        return OAL_FALSE;
    }

    /* 获取认证类型 */
    us_suite_count = 0;
    for (us_temp = 0; us_temp < us_auth_num; us_temp++) {
        if (oal_memcmp(puc_oui, puc_ie + uc_index, MAC_OUI_LEN) != 0) {
            oam_warning_log0(0, OAM_SF_SCAN, "{hmac_parse_cipher_suit::invalid WPA auth OUI,ignore this ie.}");
            /* 对于不识别的AKM套件，忽略保存 */
            uc_index += 4; /* AKM Suite 一组长度是4字节 */
            l_ie_len_left -= 4;
            continue;
        }

        if (us_suite_count < WLAN_AUTHENTICATION_SUITES) {
            /* AKM套件个数驱动最大为2，超过则不再继续保存 */
            puc_auth_policy[us_suite_count++] = puc_ie[uc_index + MAC_OUI_LEN];
        } else {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN,
                             "{hmac_parse_cipher_suit::suite_count is max,ignore the ie,us_auth_num:%d.}", us_auth_num);
        }
        uc_index += 4; /* AKM Suite 一组长度是4字节 */
        l_ie_len_left -= 4;
    }

    /* 获取 RSN 能力,2个字节 */
    if (uc_cipher_type != DMAC_RSNA_802_11I) {
        return OAL_TRUE;
    }

    if (l_ie_len_left >= 2) { /* RSN Capabilities2字节 */
        pst_bss_80211i_info->auc_rsn_cap[0] = *(puc_ie + uc_index++);
        pst_bss_80211i_info->auc_rsn_cap[1] = *(puc_ie + uc_index++);

        l_ie_len_left -= 2; /* RSN Capabilities2字节 */
    } else {
        /* 能力信息不关注失败情况,不返回失败,wpa_supplicant在解析时也不关注,可以正常关联 */
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{hmac_scan_update_bss_list_rsn::ie_rsncap too short:%d!}", l_ie_len_left);
        return OAL_TRUE;
    }

    hmac_set_group_mgmt_policy(pst_bss_80211i_info, puc_ie, &uc_index, &l_ie_len_left);
    return OAL_TRUE;
}


oal_void hmac_scan_update_bss_list_security(mac_bss_dscr_stru *pst_bss_dscr,
                                            oal_uint8 *puc_frame_body,
                                            oal_uint16 us_frame_len,
                                            oal_uint16 us_offset)
{
    oal_uint8 *puc_ie;
    oal_bool_enum_uint8 en_ret;

    /* 安全相关信息元素 */
    /* 清空当前 bss_info 结构中的安全信息 */
    memset_s(&(pst_bss_dscr->st_bss_sec_info), OAL_SIZEOF(mac_bss_80211i_info_stru),
             0xff, OAL_SIZEOF(mac_bss_80211i_info_stru));
    pst_bss_dscr->st_bss_sec_info.uc_bss_80211i_mode = 0;
    pst_bss_dscr->st_bss_sec_info.auc_rsn_cap[0] = 0;
    pst_bss_dscr->st_bss_sec_info.auc_rsn_cap[1] = 0;

#if defined(_PRE_WLAN_FEATURE_WPA2)

    puc_ie = mac_find_ie(MAC_EID_RSN, puc_frame_body + us_offset, (oal_int32)(us_frame_len - us_offset));
    if (puc_ie != OAL_PTR_NULL) {
        /* 更新从beacon 中收到的 RSN 安全相关信息到 pst_bss_dscr 中 */
        en_ret = hmac_scan_update_bss_list_rsn(pst_bss_dscr, puc_ie);
        if (en_ret == OAL_FALSE) {
            /* 如果获取RSN 信息失败，则上报帧体信息到SDT */
            oam_report_80211_frame(BROADCAST_MACADDR,
                                   puc_frame_body - MAC_80211_FRAME_LEN,
                                   MAC_80211_FRAME_LEN,
                                   puc_frame_body,
                                   us_frame_len + MAC_80211_FRAME_LEN,
                                   OAM_OTA_FRAME_DIRECTION_TYPE_RX);
        }
    }
#endif

#if defined(_PRE_WLAN_FEATURE_WPA)

    puc_ie = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA, puc_frame_body + us_offset,
                                (oal_int32)(us_frame_len - us_offset));
    if (puc_ie != OAL_PTR_NULL) {
        /* 更新从beacon 中收到的 WPA 安全相关信息到 pst_bss_dscr 中 */
        en_ret = hmac_scan_update_bss_list_wpa(pst_bss_dscr, puc_ie);
        if (en_ret == OAL_FALSE) {
            /* 如果获取安全信息失败，则上报帧体信息到SDT */
            oam_report_80211_frame(BROADCAST_MACADDR,
                                   puc_frame_body - MAC_80211_FRAME_LEN,
                                   MAC_80211_FRAME_LEN,
                                   puc_frame_body,
                                   us_frame_len + MAC_80211_FRAME_LEN,
                                   OAM_OTA_FRAME_DIRECTION_TYPE_RX);
        }
    }
#endif
}
#endif /* defined(_PRE_WLAN_FEATURE_WPA) || defined(_PRE_WLAN_FEATURE_WPA2) */


#if defined(_PRE_WLAN_FEATURE_WPA)

oal_bool_enum_uint8 hmac_scan_update_bss_list_wpa(mac_bss_dscr_stru *pst_bss_dscr, oal_uint8 *puc_ie)
{
    oal_uint8 uc_index;
    oal_uint16 us_ver = 0;
    oal_int32 l_ie_len_left;

    /*************************************************************************/
    /* WPA Element Format */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length |    WPA OUI    |  Version |  Group Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |     1     |   1    |        4      |     2    |         4 */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Pairwise Cipher |  Pairwise Cipher   |                 | */
    /* Suite Count     |    Suite List      | AKM Suite Count |AKM Suite List */
    /* --------------------------------------------------------------------- */
    /* 2        |          4*m       |         2       |     4*n */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    /* WPA IE长度,不包括Element ID 和 Length字段 */
    l_ie_len_left = puc_ie[1];
    /* 最短WPA OUI(4 字节) */
    if (l_ie_len_left < 4) {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{hmac_scan_update_bss_list_wpa::invalid WPA IE LEN:%d.}", l_ie_len_left);
        return OAL_FALSE;
    }
    /* 忽略 WPA OUI(4 字节) */
    l_ie_len_left -= 4;

    /* 忽略 WPA IE，IE 长度(2 字节) ，WPA OUI(4 字节) */
    uc_index = 2 + 4;

    /* 获取版本号,2个字节 */
    if (l_ie_len_left > 2) {
        us_ver = oal_make_word16(puc_ie[uc_index], puc_ie[uc_index + 1]);
        /* 对比WPA 版本信息 */
        if (us_ver != MAC_WPA_IE_VERSION) {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{hmac_scan_update_bss_list_wpa::invalid WPA version[%d].}", us_ver);

            return OAL_FALSE;
        }

        uc_index += 2; /* 忽略 版本号 长度(2字节) */
        l_ie_len_left -= 2;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{hmac_scan_update_bss_list_wpa::ie version too short %d!}", l_ie_len_left);
        return OAL_FALSE;
    }

    /* 设置 WPA */
    pst_bss_dscr->st_bss_sec_info.uc_bss_80211i_mode |= DMAC_WPA_802_11I;

    return hmac_parse_cipher_suit(pst_bss_dscr, DMAC_WPA_802_11I, puc_ie + uc_index, l_ie_len_left);
}
#endif
#if defined(_PRE_WLAN_FEATURE_WPA2)

oal_bool_enum_uint8 hmac_scan_update_bss_list_rsn(mac_bss_dscr_stru *pst_bss_dscr, oal_uint8 *puc_ie)
{
    oal_uint8 uc_index = 0;
    oal_uint16 us_ver = 0;
    oal_int32 l_ie_len_left;

    /*************************************************************************/
    /* RSN Element Format */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Cipher */
    /* --------------------------------------------------------------------- */
    /* |     1     |    1   |    2    |         4          |       2 */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Suite Count| Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /* |         4*m                |     2           |   4*n */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |        2       |    2      |   16 *s  |               4           | */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    /* RSN IE长度,不包括Element ID 和 Length字段 */
    l_ie_len_left = puc_ie[1];

    /* 忽略 RSN IE 和 IE 长度(2字节) */
    uc_index += 2;

    /* 获取RSN 版本号,2个字节 */
    if (l_ie_len_left > 2) {
        us_ver = oal_make_word16(puc_ie[uc_index], puc_ie[uc_index + 1]);
        if (us_ver != MAC_RSN_IE_VERSION) {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{hmac_scan_update_bss_list_rsn::invalid us_ver[%d].}", us_ver);

            return OAL_FALSE;
        }

        uc_index += 2; /* 忽略 RSN 版本号长度,2个字节 */
        l_ie_len_left -= 2;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{hmac_scan_update_bss_list_rsn::ie_version too short %d!}", l_ie_len_left);
        return OAL_FALSE;
    }

    /* 设置 RSNA */
    pst_bss_dscr->st_bss_sec_info.uc_bss_80211i_mode |= DMAC_RSNA_802_11I;

    return hmac_parse_cipher_suit(pst_bss_dscr, DMAC_RSNA_802_11I, puc_ie + uc_index, l_ie_len_left);
}
#endif
