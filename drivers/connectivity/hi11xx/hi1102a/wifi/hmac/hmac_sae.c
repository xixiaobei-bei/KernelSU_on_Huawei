

/* 1 头文件包含 */
#include "hmac_mgmt_sta.h"
#include "hmac_sae.h"
#include "hmac_fsm.h"
#include "hmac_encap_frame_ap.h"
#include "hmac_sme_sta.h"
#include "hmac_mgmt_join.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SAE_C

#ifdef _PRE_WLAN_FEATURE_SAE

OAL_STATIC oal_uint32 hmac_config_external_auth_param_check(mac_vap_stru *pst_mac_vap,
                                                            hmac_external_auth_req_stru *pst_ext_auth)
{
    if (oal_any_null_ptr2(pst_mac_vap, pst_ext_auth)) {
        oam_error_log2(0, OAM_SF_SAE,
                       "{hmac_config_external_auth_param_check::null ptr, pst_mac_vap=%p, pst_ext_auth=%p}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)pst_ext_auth);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 非legacy sta，直接返回 */
    if (!IS_LEGACY_STA(pst_mac_vap)) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_SAE,
                         "{hmac_config_external_auth_param_check::wrong vap. vap_mode %d, p2p_mode %d}",
                         pst_mac_vap->en_vap_mode,
                         pst_mac_vap->en_p2p_mode);
        return OAL_FAIL;
    }

    /* 非关联的bssid， 直接返回 */
    if (oal_memcmp(pst_mac_vap->auc_bssid, pst_ext_auth->auc_bssid, WLAN_MAC_ADDR_LEN) != 0) {
        oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_SAE,
                         "{hmac_config_external_auth_param_check::wrong bssid %02X:XX:XX:XX:%02X:%02X}",
                         pst_ext_auth->auc_bssid[0],
                         pst_ext_auth->auc_bssid[4], /* auc_bssid第0、4、5byte为参数输出打印 */
                         pst_ext_auth->auc_bssid[5]);
        return OAL_FAIL;
    }

    /* 非关联的SSID，直接返回 */
    if ((pst_ext_auth->st_ssid.uc_ssid_len != OAL_STRLEN(mac_mib_get_DesiredSSID(pst_mac_vap))) ||
        oal_memcmp(mac_mib_get_DesiredSSID(pst_mac_vap),
                   pst_ext_auth->st_ssid.auc_ssid, pst_ext_auth->st_ssid.uc_ssid_len) != 0) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_SAE,
                         "{hmac_config_external_auth_param_check::DesiredSSID len [%d], ext_auth ssid len [%d]}",
                         OAL_STRLEN(mac_mib_get_DesiredSSID(pst_mac_vap)),
                         pst_ext_auth->st_ssid.uc_ssid_len);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_external_auth
 * 功能描述  : 执行SAE external auth命令
 */
oal_uint32 hmac_config_external_auth(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    hmac_external_auth_req_stru *pst_ext_auth;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    mac_cfg_kick_user_param_stru st_kick_user_param;
    hmac_auth_rsp_stru st_auth_rsp = {
        { 0 },
    };

    pst_ext_auth = (hmac_external_auth_req_stru *)puc_param;

    ul_ret = hmac_config_external_auth_param_check(pst_mac_vap, pst_ext_auth);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SAE, "{hmac_config_external_auth::hmac_vap null, vap_idx:%d}",
                         pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取用户指针 */
    pst_hmac_user = mac_res_get_hmac_user(pst_hmac_vap->st_vap_base_info.uc_assoc_vap_id);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
                         "hmac_config_external_auth::pst_hmac_user[%d] NULL.",
                         pst_hmac_vap->st_vap_base_info.uc_assoc_vap_id);
        return OAL_FAIL;
    }

    /* ext_auth 状态为失败情况下，断开连接 */
    if (pst_ext_auth->us_status == MAC_UNSPEC_FAIL) {
        /* 对于SAE输错密码的场景, 需要先上报status = 1的connect事件 */
        hmac_handle_connect_failed_result(pst_hmac_vap, MAC_UNSPEC_FAIL);
        return OAL_SUCC;
    } else if (pst_ext_auth->us_status != MAC_SUCCESSFUL_STATUSCODE) {
        st_kick_user_param.us_reason_code = pst_ext_auth->us_status;
        memcpy_s(st_kick_user_param.auc_mac_addr, WLAN_MAC_ADDR_LEN, pst_mac_vap->auc_bssid, WLAN_MAC_ADDR_LEN);
        hmac_config_kick_user (pst_mac_vap, OAL_SIZEOF(st_kick_user_param), (oal_uint8 *)(&st_kick_user_param));
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_FEATURE_ROAM
    if (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_ROAMING) {
        hmac_roam_sae_config_reassoc_req(pst_hmac_vap);
        return OAL_SUCC;
    }
#endif

    /* ext_auth 状态为成功，执行SAE关联 */
    /* 取消定时器 */
    frw_immediate_destroy_timer(&pst_hmac_vap->st_mgmt_timer);

    /* 将状态更改为AUTH_COMP */
    hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_STA_AUTH_COMP);
    st_auth_rsp.us_status_code = HMAC_MGMT_SUCCESS;

    /* 上报SME认证成功，执行关联 */
    hmac_send_rsp_to_sme_sta(pst_hmac_vap, HMAC_SME_AUTH_RSP, (oal_uint8 *)&st_auth_rsp);

    return OAL_SUCC;
}

/*
 * 函 数 名  : is_sae_connect_with_pmkid
 * 功能描述  : 判断wpa_s下发关联rsn ie 是否包含PMKID.
 *             如果AUTH_TYPE = SAE方式关联，且关联请求IE中RSN IE包含PMKID，则不使用SAE关联；
 *             如果AUTH_TYPE = SAE方式关联，且关联请求IE中RSN IE不包含PMKID，则使用SAE关联；
 */
OAL_STATIC oal_bool_enum_uint8 is_sae_connect_with_pmkid(oal_uint8 *puc_rsn_ie, oal_uint8 uc_rsn_ie_len)
{
    /*************************************************************************/
    /* RSN Element Format */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Suite */
    /* --------------------------------------------------------------------- */
    /* |     1     |    1   |    2    |      0 or 4        |     0 or 2 */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Count| Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /* |       0 or 4*m             |     0 or 2      |   0 or 4*n */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |      0 or 2    |   0 or 2  | 0 or 16 *s  |          0 or 4        | */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    /*
     * 对于wpa_s下发关联的rsn_ie包含组播加密套件、单播加密套件、秘钥管理套件、RSN capability。
     * 可以不用考虑没有以上套件的场景
     */
    struct hmac_rsn_connect_pmkid_stru {
        oal_uint8 uc_eid;
        oal_uint8 uc_ie_len;
        oal_uint16 us_rsn_ver;
        oal_uint32 ul_group_cipher;
        oal_uint16 us_pairwise_cipher_cnt;
        oal_uint32 ul_pairwise_cipher;
        oal_uint16 us_akm_cnt;
        oal_uint32 ul_akm;
        oal_uint16 us_rsn_cap;
        oal_uint16 us_pmkid_cnt;
        oal_uint8 auc_pmkid[WLAN_PMKID_LEN];
    } __OAL_DECLARE_PACKED;

    /* RSN长度过短，不包含PMKID */
    if (uc_rsn_ie_len < OAL_SIZEOF(struct hmac_rsn_connect_pmkid_stru)) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

/*
 * 函 数 名  : hmac_update_sae_connect_param
 * 功能描述  : 关联时先判断是否为SAE关联
 *             如果AUTH_TYPE = SAE方式关联，且关联请求IE中RSN IE包含PMKID，则不使用SAE关联；
 *             如果AUTH_TYPE = SAE方式关联，且关联请求IE中RSN IE不包含PMKID，则使用SAE关联；
 */
oal_void hmac_update_sae_connect_param(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_ie, oal_uint32 ul_ie_len)
{
    oal_uint8 *puc_rsn_ie;

    pst_hmac_vap->bit_sae_connect_with_pmkid = OAL_FALSE;

    if (puc_ie == OAL_PTR_NULL) {
        return;
    }

    if (pst_hmac_vap->en_auth_mode != WLAN_WITP_AUTH_SAE) {
        return;
    }

    /* wpa_s下发SAE关联必然携带RSN IE */
    puc_rsn_ie = mac_find_ie(MAC_EID_RSN, puc_ie, ul_ie_len);
    if (puc_rsn_ie == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
                       "hmac_update_sae_connect_param:SAE connect without RSN ie",
                       ul_ie_len);
        return;
    }

    /*
     * 在RSN IE中查找携带PMKID内容，则设置sae_connect_with_pmkid = true;
     * 未携带PMKID内容，设置sae_connect_with_pmkid = false
 */
    pst_hmac_vap->bit_sae_connect_with_pmkid = is_sae_connect_with_pmkid(puc_rsn_ie, puc_rsn_ie[1]);

    OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
                     "hmac_update_sae_connect_param::use sae connect. with PMKID [%d]",
                     pst_hmac_vap->bit_sae_connect_with_pmkid);
}

/*
 * 函 数 名  : hmac_ap_up_rx_auth_req_to_host
 * 功能描述  : AP收到ft/sae auth帧上报hostapd的处理
 */
oal_void hmac_ap_up_rx_auth_req_to_host(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_auth_req)
{
    oal_uint8 auc_addr2[ETHER_ADDR_LEN] = { 0 };
    oal_uint8 uc_is_seq1;
    oal_uint16 us_auth_seq;
    oal_uint16 us_user_index = 0xffff;
    oal_uint8 uc_auth_resend = 0;
    oal_uint32 ul_ret;

    hmac_rx_mgmt_send_to_host(pst_hmac_vap, pst_auth_req);

    /* 获取STA的地址 */
    mac_get_address2(oal_netbuf_header(pst_auth_req), auc_addr2);
    if (mac_addr_is_zero(auc_addr2)) {
        oam_warning_log4(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                         "{hmac_ap_rx_auth_req::user mac:%02X:XX:XX:%02X:%02X:%02X is all 0 and invaild!}",
                         auc_addr2[0], auc_addr2[3], auc_addr2[4], auc_addr2[5]); /* auc_addr2 0、3、4、5字节为输出打印 */
        return;
    }

    /* 解析auth transaction number */
    us_auth_seq = mac_get_auth_seq_num(oal_netbuf_header(pst_auth_req));
    if (us_auth_seq > HMAC_AP_AUTH_SEQ3_WEP_COMPLETE) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                         "{hmac_ap_rx_auth_req::auth recieve invalid seq, auth seq [%d]}", us_auth_seq);
        return;
    }

    /* 获取用户idx */
    uc_is_seq1 = (us_auth_seq == WLAN_AUTH_TRASACTION_NUM_ONE);
    ul_ret = hmac_encap_auth_rsp_get_user_idx(&(pst_hmac_vap->st_vap_base_info),
                                              auc_addr2,
                                              sizeof(auc_addr2),
                                              uc_is_seq1,
                                              &uc_auth_resend,
                                              &us_user_index);
    if (ul_ret != OAL_SUCC) {
        if (ul_ret == OAL_ERR_CODE_CONFIG_EXCEED_SPEC) {
            oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                             "{hmac_ft_ap_up_rx_auth_req::hmac_ap_get_user_idx fail, users exceed config spec!}");
        } else {
            oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                             "{hmac_ft_ap_up_rx_auth_req::hmac_ap_get_user_idx Err!}");
        }
    }

    return;
}


OAL_STATIC oal_uint8 hmac_sta_sae_commit_handle_exception(
    hmac_vap_stru *pst_sta, oal_netbuf_stru *pst_netbuf, oal_uint16 us_status, oal_uint16 us_seq_num)
{
    hmac_auth_rsp_stru st_auth_rsp = {{0}, };

    if (us_status != MAC_SUCCESSFUL_STATUSCODE) {
        OAM_WARNING_LOG1(pst_sta->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                         "{hmac_sta_sae_commit_handle_exception::AP refuse STA auth reason[%d]!}", us_status);

        frw_immediate_destroy_timer(&pst_sta->st_mgmt_timer);

        st_auth_rsp.us_status_code = us_status;
        hmac_send_rsp_to_sme_sta(pst_sta, HMAC_SME_AUTH_RSP, (oal_uint8 *)&st_auth_rsp);

        return OAL_RETURN;
    }

    /* SAE commit帧的seq number是1，confirm帧的seq number是2 */
    if (us_seq_num != WLAN_AUTH_TRASACTION_NUM_ONE) {
        OAM_WARNING_LOG1(pst_sta->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                         "{hmac_sta_sae_commit_handle_exception::SAE frame dropped, seq num[%d]}", us_seq_num);

        return OAL_RETURN;
    }

    return OAL_CONTINUE;
}


oal_uint32 hmac_sta_process_sae_commit(hmac_vap_stru *pst_sta, oal_netbuf_stru *pst_netbuf)
{
    oal_uint16 us_status;
    oal_uint16 us_seq_num;
    hmac_user_stru *pst_hmac_user_ap = OAL_PTR_NULL;
    mac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;
    oal_uint8 *puc_mac_hdr = OAL_PTR_NULL;

    pst_hmac_user_ap = mac_res_get_hmac_user(pst_sta->st_vap_base_info.uc_assoc_vap_id);
    if (pst_hmac_user_ap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_sta->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                       "{hmac_sta_process_sae_commit::pst_hmac_user[%d] null.}",
                       pst_sta->st_vap_base_info.uc_assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    puc_mac_hdr = (oal_uint8 *)mac_get_rx_cb_mac_hdr(pst_rx_ctrl);

    us_status   = mac_get_auth_status(puc_mac_hdr);
    us_seq_num  = mac_get_auth_seq_num(puc_mac_hdr);

    oam_warning_log2(pst_sta->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
                     "{hmac_sta_sae_commit_handle_exception::rx SAE auth frame, status_code[%d], seq_num[%d]}",
                     us_status, us_seq_num);

    if (hmac_sta_sae_commit_handle_exception(pst_sta, pst_netbuf, us_status, us_seq_num) == OAL_RETURN) {
        /* 按照原逻辑, 出现异常也返回SUCC */
        return OAL_SUCC;
    }

    pst_sta->duplicate_auth_seq4_flag = OAL_FALSE; // seq4置位

    /* 取消定时器 */
    frw_immediate_destroy_timer(&pst_sta->st_mgmt_timer);

    /* 切换STA 到MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4 */
    hmac_fsm_change_state(pst_sta, MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4);

    /* SAE 判断seq number以后，上传给wpas 处理 */
    hmac_rx_mgmt_send_to_host(pst_sta, pst_netbuf);

    /* 启动认证超时定时器 */
    pst_sta->st_mgmt_timetout_param.en_state = MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4;
    pst_sta->st_mgmt_timetout_param.uc_vap_id = pst_sta->st_vap_base_info.uc_vap_id;
    pst_sta->st_mgmt_timetout_param.us_user_index = pst_hmac_user_ap->st_user_base_info.us_assoc_id;
    frw_create_timer(&pst_sta->st_mgmt_timer,
                     hmac_mgmt_timeout_sta,
                     pst_sta->st_mgmt_timer.ul_timeout,
                     &pst_sta->st_mgmt_timetout_param,
                     OAL_FALSE,
                     OAM_MODULE_ID_HMAC,
                     pst_sta->st_vap_base_info.ul_core_id);

    return OAL_SUCC;
}


oal_uint32 hmac_sta_process_sae_confirm(hmac_vap_stru *pst_sta, oal_netbuf_stru *pst_netbuf)
{
    mac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL; /* 每一个MPDU的控制信息 */
    oal_uint8 *puc_mac_hdr = OAL_PTR_NULL;

    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf); /* 每一个MPDU的控制信息 */
    puc_mac_hdr = (oal_uint8 *)mac_get_rx_cb_mac_hdr(pst_rx_ctrl);
    /* SAE commit帧的seq number是1，confirm帧的seq number是2 */
    if (mac_get_auth_seq_num(puc_mac_hdr) != WLAN_AUTH_TRASACTION_NUM_TWO) {
        oam_warning_log2(pst_sta->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
                         "{hmac_sta_process_sae_confirm::drop sae auth frame, status_code %x, seq_num %d.}",
                         mac_get_auth_status(puc_mac_hdr),
                         mac_get_auth_seq_num(puc_mac_hdr));
        return OAL_SUCC;
    }

    if (pst_sta->duplicate_auth_seq4_flag == OAL_TRUE) { // wpa3 auth seq4重复帧过滤，禁止上报waps
        oam_warning_log2(pst_sta->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
            "{hmac_sta_wait_auth_seq4_rx::drop sae auth frame, status_code %d, seq_num %d.}",
            mac_get_auth_status(puc_mac_hdr), mac_get_auth_seq_num(puc_mac_hdr));
        return OAL_SUCC;
    }

    /* 取消定时器 */
    frw_immediate_destroy_timer(&pst_sta->st_mgmt_timer);

    /* SAE 判断seq number以后，上传给wpas 处理 */
    hmac_rx_mgmt_send_to_host(pst_sta, pst_netbuf);

    oam_warning_log2(pst_sta->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
                     "{hmac_sta_process_sae_confirm::rx sae auth frame, status_code %x, seq_num %d.}",
                     mac_get_auth_status(puc_mac_hdr),
                     mac_get_auth_seq_num(puc_mac_hdr));
    pst_sta->duplicate_auth_seq4_flag = OAL_TRUE;

    return OAL_SUCC;
}


OAL_STATIC oal_bool_enum_uint8 hmac_only_support_sae(oal_uint8 *puc_rsn_akm)
{
    return ((puc_rsn_akm[0] == WLAN_AUTH_SUITE_SAE_SHA256) && ((puc_rsn_akm[1] == 0xFF) || puc_rsn_akm[1] == 0)) ||
           ((puc_rsn_akm[1] == WLAN_AUTH_SUITE_SAE_SHA256) && ((puc_rsn_akm[0] == 0xFF) || puc_rsn_akm[0] == 0));
}


oal_bool_enum_uint8 hmac_check_illegal_sae_roam(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_bss_rsn_akm)
{
    oal_uint8 auc_sta_rsn_akm[MAC_AUTHENTICATION_SUITE_NUM] = { 0 };

    /* 存在相同akm, 允许漫游 */
    if (mac_mib_rsn_akm_match_suites(pst_mac_vap, puc_bss_rsn_akm, MAC_AUTHENTICATION_SUITE_NUM) != 0) {
        /* not illegal */
        return OAL_FALSE;
    }

    mac_mib_get_rsn_akm_suites(pst_mac_vap, auc_sta_rsn_akm);

    /* 无相同akm, 且某一方只支持SAE, 说明另一端不支持SAE, 不允许漫游 */
    return hmac_only_support_sae(puc_bss_rsn_akm) || hmac_only_support_sae(auc_sta_rsn_akm);
}


oal_uint32 hmac_roam_rx_auth_check_sae(hmac_vap_stru *hmac_vap, dmac_wlan_crx_event_stru *crx_event,
    oal_uint16 auth_status, oal_uint16 auth_seq_num)
{
    oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                     "{hmac_roam_process_auth_seq2::rx sae auth frame, status_code %d, seq_num %d.}",
                     auth_status, auth_seq_num);
    /* 4帧交互的wpa3认证(1,1,2,2)，第二帧是seq1，第四帧是seq2 */
    if (auth_seq_num == WLAN_AUTH_TRASACTION_NUM_ONE && hmac_vap->duplicate_auth_seq2_flag == OAL_FALSE) {
        hmac_vap->duplicate_auth_seq2_flag = OAL_TRUE;
        hmac_rx_mgmt_send_to_host(hmac_vap, crx_event->pst_netbuf);
        return OAL_SUCC;
    } else if (auth_seq_num == WLAN_AUTH_TRASACTION_NUM_ONE && hmac_vap->duplicate_auth_seq2_flag == OAL_TRUE) {
        return OAL_SUCC;
    } else if (auth_seq_num == WLAN_AUTH_TRASACTION_NUM_TWO && hmac_vap->duplicate_auth_seq4_flag == OAL_FALSE) {
        hmac_vap->duplicate_auth_seq4_flag = OAL_TRUE;
        hmac_rx_mgmt_send_to_host(hmac_vap, crx_event->pst_netbuf);
        return OAL_SUCC;
    } else if (auth_seq_num == WLAN_AUTH_TRASACTION_NUM_TWO && hmac_vap->duplicate_auth_seq4_flag == OAL_TRUE) {
        return OAL_SUCC;
    } else {
        return OAL_SUCC;
    }
}

/*
 * 函 数 名  : hmac_report_external_auth_req
 * 功能描述  : 发送SAE external auth请求到wpa_supplicant
 */
oal_uint32 hmac_report_external_auth_req(hmac_vap_stru *pst_hmac_vap, oal_nl80211_external_auth_action en_action)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event;
    hmac_external_auth_req_stru st_ext_auth_req;
    oal_uint8 auc_akm[WLAN_AUTHENTICATION_SUITES] = {0};
    oal_uint8 uc_akm_suites_num;
    oal_int32 l_ret;

    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_SAE, "{hmac_report_external_auth_req:: hmac_vap is NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(&st_ext_auth_req, OAL_SIZEOF(st_ext_auth_req), 0, OAL_SIZEOF(st_ext_auth_req));

    st_ext_auth_req.en_action = en_action;
    st_ext_auth_req.us_status = MAC_SUCCESSFUL_STATUSCODE;

    l_ret = memcpy_s(st_ext_auth_req.auc_bssid, WLAN_MAC_ADDR_LEN,
                     pst_hmac_vap->st_vap_base_info.auc_bssid, WLAN_MAC_ADDR_LEN);

    uc_akm_suites_num = mac_mib_get_rsn_akm_suites(&pst_hmac_vap->st_vap_base_info, auc_akm);
    if (uc_akm_suites_num != 1) {
        OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
                       "{hmac_report_external_auth_req::get AKM suite failed! akm_suite_num [%d]}",
                       uc_akm_suites_num);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    st_ext_auth_req.ul_key_mgmt_suite = CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, auc_akm[0]);
    st_ext_auth_req.st_ssid.uc_ssid_len =
        oal_min(OAL_SIZEOF(st_ext_auth_req.st_ssid.auc_ssid),
                OAL_STRLEN(mac_mib_get_DesiredSSID(&(pst_hmac_vap->st_vap_base_info))));
    l_ret += memcpy_s(st_ext_auth_req.st_ssid.auc_ssid,
                      OAL_IEEE80211_MAX_SSID_LEN,
                      mac_mib_get_DesiredSSID(&(pst_hmac_vap->st_vap_base_info)),
                      st_ext_auth_req.st_ssid.uc_ssid_len);

    pst_event_mem = frw_event_alloc_m(OAL_SIZEOF(st_ext_auth_req));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
                         "{hmac_report_connect_failed_result::frw_event_alloc fail! size[%d]}",
                         OAL_SIZEOF(st_ext_auth_req));
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    pst_event = frw_get_event_stru(pst_event_mem);

    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_EXT_AUTH_REQ,
                       OAL_SIZEOF(st_ext_auth_req), FRW_EVENT_PIPELINE_STAGE_0,
                       pst_hmac_vap->st_vap_base_info.uc_chip_id,
                       pst_hmac_vap->st_vap_base_info.uc_device_id,
                       pst_hmac_vap->st_vap_base_info.uc_vap_id);

    l_ret += memcpy_s(frw_get_event_payload(pst_event_mem), OAL_SIZEOF(st_ext_auth_req),
                      &st_ext_auth_req, OAL_SIZEOF(st_ext_auth_req));
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_SAE, "hmac_report_external_auth_req::memcpy fail!");
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_report_ext_auth_worker
 * 功能描述  : 上报EXTERNAL_AUTH_START事件到wal
 */
oal_void hmac_report_ext_auth_worker(oal_work_stru *pst_sae_report_ext_auth_worker)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = oal_container_of(pst_sae_report_ext_auth_worker, hmac_vap_stru, st_sae_report_ext_auth_worker);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_report_ext_auth_worker:: hmac_vap is null}");
        return;
    }

    oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                     "{hmac_report_ext_auth_worker:: hmac_report_external_auth_req_etc auth start}");

    if (hmac_report_external_auth_req(pst_hmac_vap, NL80211_EXTERNAL_AUTH_START) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_AUTH, "{hmac_report_ext_auth_worker::hmac_report_external_auth_req fail.}");
    }
}
#endif
