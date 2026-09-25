

#ifdef _PRE_WLAN_FEATURE_ROAM

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oam_ext_if.h"
#include "mac_ie.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "dmac_ext_if.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_mgmt_sta.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_roam_main.h"
#include "hmac_roam_connect.h"
#include "hmac_roam_alg.h"
#include "hmac_scan.h"
#include "hmac_config.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ROAM_MAIN_C
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
OAL_STATIC hmac_roam_fsm_func g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_BUTT][ROAM_MAIN_FSM_EVENT_TYPE_BUTT];
OAL_STATIC oal_uint32 hmac_roam_main_null_fn(hmac_roam_info_stru *pst_roam_info, oal_void *p_param);
OAL_STATIC oal_uint32 hmac_roam_scan_init(hmac_roam_info_stru *pst_roam_info, oal_void *p_param);
OAL_STATIC oal_uint32 hmac_roam_scan_channel(hmac_roam_info_stru *pst_roam_info, oal_void *p_param);
OAL_STATIC oal_uint32 hmac_roam_check_scan_result(
    hmac_roam_info_stru *pst_roam_info, oal_void *p_param, mac_bss_dscr_stru **ppst_bss_dscr_out);
OAL_STATIC oal_uint32 hmac_roam_connect_to_bss(hmac_roam_info_stru *pst_roam_info, oal_void *p_param);
OAL_STATIC oal_uint32 hmac_roam_to_old_bss(hmac_roam_info_stru *pst_roam_info, oal_void *p_param);
OAL_STATIC oal_uint32 hmac_roam_to_new_bss(hmac_roam_info_stru *pst_roam_info, oal_void *p_param);
OAL_STATIC oal_void hmac_roam_main_del_timer(hmac_roam_info_stru *pst_roam_info);
OAL_STATIC oal_uint32 hmac_roam_main_check_state(hmac_roam_info_stru *pst_roam_info,
                                                 mac_vap_state_enum_uint8 en_vap_state,
                                                 roam_main_state_enum_uint8 en_main_state);
OAL_STATIC oal_void hmac_roam_main_clear(hmac_roam_info_stru *pst_roam_info);
OAL_STATIC oal_uint32 hmac_roam_scan_timeout(hmac_roam_info_stru *pst_roam_info, oal_void *p_param);
OAL_STATIC oal_uint32 hmac_roam_connecting_timeout(hmac_roam_info_stru *pst_roam_info, oal_void *p_param);
OAL_STATIC oal_uint32 hmac_roam_connecting_fail(hmac_roam_info_stru *pst_roam_info, oal_void *p_param);
OAL_STATIC oal_uint32 hmac_roam_handle_fail_handshake_phase(hmac_roam_info_stru *pst_roam_info, oal_void *p_param);
OAL_STATIC oal_uint32 hmac_roam_handle_scan_result(hmac_roam_info_stru *pst_roam_info, oal_void *p_param);
#define WLAN_MAX_ROAM_PING_PONG_CNT (5)
#define WLAN_MIN_ROAM_TIME_INTERVAL (60000)
/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_void hmac_roam_reset_rssi(hmac_vap_stru *pst_hmac_vap)
{
    oal_uint32 ul_ret;

    ul_ret = hmac_config_send_event(&pst_hmac_vap->st_vap_base_info, WLAN_CFGID_RESET_RSSI, 0, OAL_PTR_NULL);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_reset_rssi::send event[WLAN_CFGID_RESET_RSSI] failed[%d].}", ul_ret);
    }
}

OAL_STATIC oal_void hmac_roam_fsm_init(oal_void)
{
    oal_uint32 ul_state;
    oal_uint32 ul_event;

    for (ul_state = 0; ul_state < ROAM_MAIN_STATE_BUTT; ul_state++) {
        for (ul_event = 0; ul_event < ROAM_MAIN_FSM_EVENT_TYPE_BUTT; ul_event++) {
            g_hmac_roam_main_fsm_func[ul_state][ul_event] = hmac_roam_main_null_fn;
        }
    }
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_INIT][ROAM_MAIN_FSM_EVENT_START] = hmac_roam_scan_init;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_SCANING][ROAM_MAIN_FSM_EVENT_START] = hmac_roam_scan_channel;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_SCANING][ROAM_MAIN_FSM_EVENT_SCAN_RESULT] = hmac_roam_handle_scan_result;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_SCANING][ROAM_MAIN_FSM_EVENT_TIMEOUT] = hmac_roam_scan_timeout;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_SCANING][ROAM_MAIN_FSM_EVENT_START_CONNECT] = hmac_roam_connect_to_bss;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_CONNECTING][ROAM_MAIN_FSM_EVENT_TIMEOUT] = hmac_roam_connecting_timeout;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_CONNECTING][ROAM_MAIN_FSM_EVENT_CONNECT_FAIL] = hmac_roam_connecting_fail;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_CONNECTING][ROAM_MAIN_FSM_EVENT_HANDSHAKE_FAIL] =
        hmac_roam_handle_fail_handshake_phase;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_CONNECTING][ROAM_MAIN_FSM_EVENT_CONNECT_SUCC] = hmac_roam_to_new_bss;
}


oal_uint32 hmac_roam_main_fsm_action(
    hmac_roam_info_stru *pst_roam_info, roam_main_fsm_event_type_enum en_event, oal_void *p_param)
{
    if (OAL_UNLIKELY(pst_roam_info == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_roam_info->en_main_state >= ROAM_MAIN_STATE_BUTT) {
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }

    if (en_event >= ROAM_MAIN_FSM_EVENT_TYPE_BUTT) {
        return OAL_ERR_CODE_ROAM_EVENT_UXEXPECT;
    }

    return g_hmac_roam_main_fsm_func[pst_roam_info->en_main_state][en_event](pst_roam_info, p_param);
}


OAL_STATIC oal_void hmac_roam_main_change_state(hmac_roam_info_stru *pst_roam_info, roam_main_state_enum_uint8 en_state)
{
    if (pst_roam_info != OAL_PTR_NULL) {
        OAM_WARNING_LOG2(0, OAM_SF_ROAM,
                         "{hmac_roam_main_change_state::[%d]->[%d]}", pst_roam_info->en_main_state, en_state);
        pst_roam_info->en_main_state = en_state;
    }
}


OAL_STATIC oal_uint32 hmac_roam_main_timeout(oal_void *p_arg)
{
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;

    if (OAL_UNLIKELY(p_arg == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_roam_info = (hmac_roam_info_stru *)p_arg;

    OAM_WARNING_LOG2(0, OAM_SF_ROAM, "{hmac_roam_main_timeout::MAIN_STATE[%d] CONNECT_STATE[%d].}",
                     pst_roam_info->en_main_state, pst_roam_info->st_connect.en_state);

    return hmac_roam_main_fsm_action(pst_roam_info, ROAM_MAIN_FSM_EVENT_TIMEOUT, OAL_PTR_NULL);
}


OAL_STATIC oal_bool_enum_uint8 hmac_roam_is_wpa_cipher(oal_uint8 uc_cipher_type)
{
    if ((uc_cipher_type == WLAN_80211_CIPHER_SUITE_TKIP) ||
        (uc_cipher_type == WLAN_80211_CIPHER_SUITE_CCMP) ||
        (uc_cipher_type == WLAN_80211_CIPHER_SUITE_GCMP) ||
        (uc_cipher_type == WLAN_80211_CIPHER_SUITE_CCMP_256) ||
        (uc_cipher_type == WLAN_80211_CIPHER_SUITE_GCMP_256)) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}


OAL_STATIC oal_uint8 hmac_roam_get_curr_cipher(
    mac_bss_dscr_stru *pst_bss_dscr, witp_wpa_versions_enum en_wpa_type, oal_uint8 uc_index)
{
    if (en_wpa_type == WITP_WPA_VERSION_2) {
        /* RSN */
        return pst_bss_dscr->st_bss_sec_info.auc_rsn_pairwise_policy[uc_index];
    } else {
        /* WPA */
        return pst_bss_dscr->st_bss_sec_info.auc_wpa_pairwise_policy[uc_index];
    }
}


OAL_STATIC oal_uint8 hmac_roam_select_best_cipher(mac_bss_dscr_stru *pst_bss_dscr, witp_wpa_versions_enum en_wpa_type)
{
    oal_uint8 uc_index;
    oal_uint8 uc_best_cipher = WLAN_80211_CIPHER_SUITE_TKIP; /* 使用优先级最低的加密能力进行初始化 */
    oal_uint8 uc_curr_cipher;

    /* pst_bss_dscr非空已在外层函数做过检查 */
    for (uc_index = 0; uc_index < MAC_PAIRWISE_CIPHER_SUITES_NUM; uc_index++) {
        uc_curr_cipher = hmac_roam_get_curr_cipher(pst_bss_dscr, en_wpa_type, uc_index);
        if (hmac_roam_is_wpa_cipher(uc_curr_cipher) &&
            (uc_curr_cipher > uc_best_cipher)) {
            /* 数值越大加密能力越强, 若将来协议扩展后出现反例, 此处记得修改 */
            uc_best_cipher = uc_curr_cipher;
        }
    }

    /* 增加兼容性: bss加密信息出错的情况下尝试以TKIP加密进行漫游重关联 */
    return uc_best_cipher;
}


OAL_STATIC oal_uint32 hmac_roam_renew_privacy(hmac_vap_stru *pst_hmac_vap, mac_bss_dscr_stru *pst_bss_dscr)
{
    oal_uint32 ul_ret;
    mac_cfg80211_connect_security_stru st_conn_sec = { 0 };
    mac_cap_info_stru *pst_cap_info;
    oal_uint8 uc_wpa_version;
    oal_uint8 uc_grp_policy;
    oal_uint8 uc_akm_policy;
    oal_uint8 uc_pairwise_policy;

    pst_cap_info = (mac_cap_info_stru *)&pst_bss_dscr->us_cap_info;

    if (pst_cap_info->bit_privacy == 0) {
        return OAL_SUCC;
    }

    if (pst_bss_dscr->st_bss_sec_info.uc_bss_80211i_mode == 0) {
        return OAL_SUCC;
    }

    if (pst_hmac_vap->en_auth_mode != NL80211_AUTHTYPE_OPEN_SYSTEM) {
        return OAL_SUCC;
    }

    if (pst_bss_dscr->st_bss_sec_info.uc_bss_80211i_mode & DMAC_RSNA_802_11I) {
        uc_wpa_version = WITP_WPA_VERSION_2;
        uc_grp_policy = pst_bss_dscr->st_bss_sec_info.uc_rsn_grp_policy;
        uc_akm_policy = pst_bss_dscr->st_bss_sec_info.auc_rsn_auth_policy[0];
        uc_pairwise_policy = hmac_roam_select_best_cipher(pst_bss_dscr, WITP_WPA_VERSION_2);
        st_conn_sec.st_crypto.uc_group_mgmt_suite = pst_bss_dscr->st_bss_sec_info.uc_group_mgmt_policy;
    } else {
        uc_wpa_version = WITP_WPA_VERSION_1;
        uc_grp_policy = pst_bss_dscr->st_bss_sec_info.uc_wpa_grp_policy;
        uc_akm_policy = pst_bss_dscr->st_bss_sec_info.auc_wpa_auth_policy[0];
        uc_pairwise_policy = hmac_roam_select_best_cipher(pst_bss_dscr, WITP_WPA_VERSION_1);
    }

    st_conn_sec.en_privacy = OAL_TRUE;
    st_conn_sec.st_crypto.wpa_versions = uc_wpa_version;
    st_conn_sec.st_crypto.cipher_group = uc_grp_policy;
    st_conn_sec.st_crypto.ciphers_pairwise[0] = uc_pairwise_policy;
    st_conn_sec.st_crypto.n_ciphers_pairwise = 1;
    st_conn_sec.st_crypto.akm_suites[0] = uc_akm_policy;
    st_conn_sec.st_crypto.n_akm_suites = 1;

    ul_ret = mac_vap_init_privacy(&pst_hmac_vap->st_vap_base_info, &st_conn_sec);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{hmac_roam_renew_privacy:: mac_11i_init_privacy failed[%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

oal_uint32 hmac_roam_init(hmac_vap_stru *pst_hmac_vap)
{
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;
    oal_uint32 i = 0;

    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (!IS_LEGACY_VAP(&(pst_hmac_vap->st_vap_base_info))) {
        return OAL_SUCC;
    }

    if (pst_hmac_vap->pul_roam_info == OAL_PTR_NULL) {
        /* 漫游主结构体内存申请 */
        pst_hmac_vap->pul_roam_info =
            (oal_uint32 *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(hmac_roam_info_stru), OAL_TRUE);
        if (pst_hmac_vap->pul_roam_info == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                           "{hmac_roam_init::OAL_MEM_ALLOC fail.}");
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
    }
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    else {
        hmac_roam_main_del_timer((hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info);
    }
#endif

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;

    memset_s(pst_hmac_vap->pul_roam_info, OAL_SIZEOF(hmac_roam_info_stru), 0, OAL_SIZEOF(hmac_roam_info_stru));
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    pst_roam_info->uc_enable = wlan_customize.uc_roam_switch;
#else
    pst_roam_info->uc_enable = 1;
#endif
    pst_roam_info->en_roam_trigger = ROAM_TRIGGER_DMAC;
    pst_roam_info->en_main_state = ROAM_MAIN_STATE_INIT;
    pst_roam_info->en_current_bss_ignore = OAL_TRUE;
    pst_roam_info->pst_hmac_vap = pst_hmac_vap;
    pst_roam_info->pst_hmac_user = OAL_PTR_NULL;
    pst_roam_info->ul_connected_state = WPAS_CONNECT_STATE_INIT;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    pst_roam_info->st_config.uc_scan_band = wlan_customize.uc_roam_scan_band;
    pst_roam_info->st_config.uc_scan_orthogonal = wlan_customize.uc_roam_scan_orthogonal;
    pst_roam_info->st_config.c_trigger_rssi_2G = wlan_customize.c_roam_trigger_b;
    pst_roam_info->st_config.c_trigger_rssi_5G = wlan_customize.c_roam_trigger_a;
    pst_roam_info->st_config.uc_delta_rssi_2G = wlan_customize.c_roam_delta_b;
    pst_roam_info->st_config.uc_delta_rssi_5G = wlan_customize.c_roam_delta_a;
#else
    pst_roam_info->st_config.uc_scan_band = ROAM_BAND_2G_BIT | ROAM_BAND_5G_BIT;
    pst_roam_info->st_config.uc_scan_orthogonal = ROAM_SCAN_CHANNEL_ORG_BUTT;
    pst_roam_info->st_config.c_trigger_rssi_2G = ROAM_RSSI_NE70_DB;
    pst_roam_info->st_config.c_trigger_rssi_5G = ROAM_RSSI_NE70_DB;
    pst_roam_info->st_config.uc_delta_rssi_2G = ROAM_RSSI_DIFF_10_DB;
    pst_roam_info->st_config.uc_delta_rssi_5G = ROAM_RSSI_DIFF_10_DB;
#endif

    for (i = 0; i < ROAM_LIST_MAX; i++) {
        pst_roam_info->st_alg.st_history.ast_bss[i].us_count_limit = ROAM_HISTORY_COUNT_LIMIT;
        pst_roam_info->st_alg.st_history.ast_bss[i].ul_timeout = ROAM_HISTORY_BSS_TIME_OUT;
    }

    for (i = 0; i < ROAM_LIST_MAX; i++) {
        pst_roam_info->st_alg.st_blacklist.ast_bss[i].us_count_limit = ROAM_BLACKLIST_COUNT_LIMIT;
        pst_roam_info->st_alg.st_blacklist.ast_bss[i].ul_timeout = ROAM_BLACKLIST_NORMAL_AP_TIME_OUT;
    }

    hmac_roam_fsm_init();
    hmac_roam_connect_fsm_init();

    OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM, "{hmac_roam_init::SUCC.}");
    return OAL_SUCC;
}


oal_uint32 hmac_roam_info_init(hmac_vap_stru *pst_hmac_vap)
{
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    mac_roam_trigger_stru st_roam_trigger;

    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_hmac_vap->pul_roam_info == OAL_PTR_NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;

    pst_roam_info->en_main_state = ROAM_MAIN_STATE_INIT;
    pst_roam_info->uc_rssi_ignore = 0;
    pst_roam_info->uc_invalid_scan_cnt = 0;
    pst_roam_info->st_connect.en_state = ROAM_CONNECT_STATE_INIT;
    pst_roam_info->st_alg.ul_max_capacity = 0;
    pst_roam_info->st_alg.pst_max_capacity_bss = OAL_PTR_NULL;
    pst_roam_info->st_alg.c_current_rssi = 0;
    pst_roam_info->st_alg.c_max_rssi = 0;
    pst_roam_info->st_alg.uc_another_bss_scaned = 0;
    pst_roam_info->st_alg.uc_invalid_scan_cnt = 0;
    pst_roam_info->st_alg.pst_max_rssi_bss = OAL_PTR_NULL;
    pst_roam_info->pst_hmac_user = mac_res_get_hmac_user((oal_uint16)pst_hmac_vap->st_vap_base_info.uc_assoc_vap_id);
    if (pst_roam_info->pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_info_init::assoc_vap_id[%d] can't found.}",
                       pst_hmac_vap->st_vap_base_info.uc_assoc_vap_id);
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    st_roam_trigger.c_trigger_2G = pst_roam_info->st_config.c_trigger_rssi_2G;
    st_roam_trigger.c_trigger_5G = pst_roam_info->st_config.c_trigger_rssi_5G;

    ul_ret = hmac_config_send_event(&pst_hmac_vap->st_vap_base_info,
                                    WLAN_CFGID_SET_ROAM_TRIGGER,
                                    OAL_SIZEOF(mac_roam_trigger_stru),
                                    (oal_uint8 *)&st_roam_trigger);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_info_init::send event[INIT_ROAM_TRIGGER] failed[%d].}", ul_ret);
        return ul_ret;
    }
    return OAL_SUCC;
}


oal_uint32 hmac_roam_exit(hmac_vap_stru *pst_hmac_vap)
{
    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (!IS_LEGACY_VAP(&(pst_hmac_vap->st_vap_base_info))) {
        return OAL_SUCC;
    }

    if (pst_hmac_vap->pul_roam_info == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM, "hmac_roam_exit:pul_roam_info is NULL");
        return OAL_ERR_CODE_PTR_NULL;
    }

    FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(((hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info)->st_connect.st_timer));
    hmac_roam_main_del_timer((hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info);
    if (pst_hmac_vap->pst_net_device != OAL_PTR_NULL) {
        oal_net_tx_wake_all_queues(pst_hmac_vap->pst_net_device);
    }
    OAL_MEM_FREE(pst_hmac_vap->pul_roam_info, OAL_TRUE);
    pst_hmac_vap->pul_roam_info = OAL_PTR_NULL;

    OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM, "{hmac_roam_exit::SUCC.}");

    return OAL_SUCC;
}


oal_uint32 hmac_roam_show_info(hmac_vap_stru *pst_hmac_vap)
{
    oal_int8 *pc_print_buff = OAL_PTR_NULL;
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;

    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_hmac_vap->pul_roam_info == OAL_PTR_NULL) {
        return OAL_SUCC;
    }

    pc_print_buff = (oal_int8 *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (pc_print_buff == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "hmac_config_vap_info:pc_print_buff null");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    memset_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
    snprintf_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, OAM_REPORT_MAX_STRING_LEN - 1,
        "ROAM_EN[%d] MAIN_STATE[%d]\n"
        "ROAM_SCAN_BAND[%d] ROAM_SCAN_ORTH[%d]\n",
        pst_roam_info->uc_enable, pst_roam_info->en_main_state,
        pst_roam_info->st_config.uc_scan_band, pst_roam_info->st_config.uc_scan_orthogonal);

    oam_print(pc_print_buff);

    OAL_MEM_FREE(pc_print_buff, OAL_TRUE);

    return OAL_SUCC;
}


OAL_STATIC oal_void hmac_roam_main_start_timer(hmac_roam_info_stru *pst_roam_info, oal_uint32 ul_timeout)
{
    frw_timeout_stru *pst_timer = &(pst_roam_info->st_timer);

    OAM_INFO_LOG1(0, OAM_SF_ROAM, "{hmac_roam_main_start_timer [%d].}", ul_timeout);

    /* 启动认证超时定时器 */
    FRW_TIMER_CREATE_TIMER(pst_timer,
                           hmac_roam_main_timeout,
                           ul_timeout,
                           pst_roam_info,
                           OAL_FALSE,
                           OAM_MODULE_ID_HMAC,
                           pst_roam_info->pst_hmac_vap->st_vap_base_info.ul_core_id);
}


OAL_STATIC oal_void hmac_roam_main_del_timer(hmac_roam_info_stru *pst_roam_info)
{
    OAM_INFO_LOG0(0, OAM_SF_ROAM, "{hmac_roam_main_del_timer.}");
    FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_roam_info->st_timer));
    return;
}


oal_uint32 hmac_roam_enable(hmac_vap_stru *pst_hmac_vap, oal_uint8 uc_enable)
{
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;

    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_enable::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_roam_info == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM, "hmac_roam_enable::pst_roam_info null");
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    if (uc_enable == pst_roam_info->uc_enable) {
        OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "hmac_roam_enable:SET[%d] fail", uc_enable);
        return OAL_FAIL;
    }

    /* 设置漫游开关 */
    pst_roam_info->uc_enable = uc_enable;
    pst_roam_info->en_main_state = ROAM_MAIN_STATE_INIT;
    OAM_INFO_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM, "{hmac_roam_enable::SET[%d] OK!}", uc_enable);

    return OAL_SUCC;
}


oal_uint32 hmac_roam_org(hmac_vap_stru *pst_hmac_vap, oal_uint8 uc_scan_orthogonal)
{
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;

    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_org::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_roam_info == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM, "{hmac_roam_org::pst_roam_info null .}");
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    /* 设置漫游正交 */
    pst_roam_info->st_config.uc_scan_orthogonal = uc_scan_orthogonal;

    return OAL_SUCC;
}


oal_uint32 hmac_roam_band(hmac_vap_stru *pst_hmac_vap, oal_uint8 uc_scan_band)
{
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;

    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_band::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_roam_info == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM, "{hmac_roam_band::pst_roam_info null .}");
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    /* 设置漫游频段 */
    pst_roam_info->st_config.uc_scan_band = uc_scan_band;
    OAM_INFO_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM, "{hmac_roam_band::SET[%d] OK!}", uc_scan_band);

    return OAL_SUCC;
}

oal_void hmac_roam_ping_pong_clear(hmac_vap_stru *pst_hmac_vap)
{
    pst_hmac_vap->en_last_roam_trigger = ROAM_TRIGGER_BUTT;
    pst_hmac_vap->uc_diff_roam_trigger_cnt = 0;
}


oal_void hmac_roam_ping_pong_check(hmac_vap_stru *pst_hmac_vap, roam_trigger_enum_uint8 en_roam_trigger)
{
    /* 判断是否存在漫游乒乓场景：1.每次漫游类型与前次类型都不同 2.两次漫游间隔时间小于1min */
    if ((pst_hmac_vap->en_last_roam_trigger != ROAM_TRIGGER_BUTT) &&
        (pst_hmac_vap->en_last_roam_trigger != en_roam_trigger) &&
        (OAL_TIME_GET_RUNTIME(pst_hmac_vap->ul_last_roam_timestamp, (oal_uint32)OAL_TIME_GET_STAMP_MS()) <
         WLAN_MIN_ROAM_TIME_INTERVAL)) {
        pst_hmac_vap->uc_diff_roam_trigger_cnt++;
    }

    pst_hmac_vap->en_last_roam_trigger = en_roam_trigger;
    pst_hmac_vap->ul_last_roam_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    if (pst_hmac_vap->uc_diff_roam_trigger_cnt >= WLAN_MAX_ROAM_PING_PONG_CNT) {
#ifdef _PRE_WLAN_1102A_CHR
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_11V_ROAM_FAIL, CHR_WIFI_DRV_ERROR_ROAM_PING_PONG);
#endif
        OAM_WARNING_LOG0(0, OAM_SF_ROAM, "{hmac_roam_ping_pong_check::roam ping pong occurred!}");
        hmac_roam_ping_pong_clear(pst_hmac_vap);
    }
}


oal_uint32 hmac_roam_start(hmac_vap_stru *pst_hmac_vap, roam_channel_org_enum_uint8 en_scan_type,
                           oal_bool_enum_uint8 en_current_bss_ignore, roam_trigger_enum_uint8 en_roam_trigger)
{
    oal_uint32 ul_ret;
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;

    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_start::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (en_current_bss_ignore == OAL_TRUE) {
        ul_ret = hmac_vap_check_signal_bridge(&pst_hmac_vap->st_vap_base_info);
        if (ul_ret != OAL_SUCC) {
            return ul_ret;
        }

        /* 非漫游到自己，黑名单路由器，不支持漫游，防止漫游出现异常 */
        if ((pst_hmac_vap->en_roam_prohibit_on == OAL_TRUE) && (en_roam_trigger != ROAM_TRIGGER_11V)) {
            OAM_WARNING_LOG0(0, OAM_SF_ROAM, "{hmac_roam_start::blacklist ap not support roam!}");
            return OAL_FAIL;
        }
    }

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_roam_info == OAL_PTR_NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 防止没获取到ip就触发漫游 */
    if (pst_roam_info->ul_ip_addr_obtained == OAL_FALSE) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP_STATUS;
    }

    ul_ret = hmac_roam_main_check_state(pst_roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_INIT);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_start::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    /* 判断是否出现漫游乒乓 */
    hmac_roam_ping_pong_check(pst_hmac_vap, en_roam_trigger);

    /* 每次漫游前，刷新是否支持漫游到自己的参数 */
    pst_roam_info->st_config.uc_scan_orthogonal = en_scan_type;
    pst_roam_info->en_current_bss_ignore = en_current_bss_ignore; /* false表示漫游到自己 */
    pst_roam_info->en_roam_trigger = en_roam_trigger;

    OAM_WARNING_LOG3(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                     "{hmac_roam_start::START succ, en_scan_type=%d, en_current_bss_ignore=%d,en_roam_trigger=%d}",
                     en_scan_type, en_current_bss_ignore, en_roam_trigger);

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    if (pst_roam_info->en_roam_trigger == ROAM_TRIGGER_11V) {
        hmac_roam_alg_init(pst_roam_info, pst_roam_info->st_bsst_rsp_info.c_rssi);
    } else
#endif
    {
        hmac_roam_alg_init(pst_roam_info, ROAM_RSSI_CMD_TYPE);
    }
    /* 触发漫游是否搭配扫描true表示不扫描 */
    if (en_scan_type == ROAM_SCAN_CHANNEL_ORG_0) {
        /* 对于不进行扫描的漫游流程,更新扫描/关联时间戳 */
        pst_roam_info->st_static.ul_scan_timetamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();
        hmac_roam_main_change_state(pst_roam_info, ROAM_MAIN_STATE_SCANING);
        ul_ret = hmac_roam_main_fsm_action(pst_roam_info, ROAM_MAIN_FSM_EVENT_TIMEOUT, OAL_PTR_NULL);
        if (ul_ret != OAL_SUCC) {
            OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                           "{hmac_roam_start::START fail[%d].}", ul_ret);
            return ul_ret;
        }
    } else {
        ul_ret = hmac_roam_main_fsm_action(pst_roam_info, ROAM_MAIN_FSM_EVENT_START, OAL_PTR_NULL);
        if (ul_ret != OAL_SUCC) {
            OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                           "{hmac_roam_start::START fail[%d].}", ul_ret);
            return ul_ret;
        }
    }
#ifdef _PRE_WLAN_1102A_CHR
    hmac_chr_roam_info_report(pst_roam_info, HMAC_CHR_ROAM_START);
#endif
    return OAL_SUCC;
}

oal_uint32 hmac_roam_show(hmac_vap_stru *pst_hmac_vap)
{
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;
    hmac_roam_static_stru *pst_static = OAL_PTR_NULL;
    oal_uint8 uc_vap_id;

    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_show::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_roam_info == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_show::pst_roam_info null.}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    pst_static = &pst_roam_info->st_static;

    uc_vap_id = pst_hmac_vap->st_vap_base_info.uc_vap_id;
    OAM_ERROR_LOG2(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_show::trigger_rssi_cnt[0x%x] trigger_linkloss_cnt[0x%x] .}",
                   pst_static->ul_trigger_rssi_cnt, pst_static->ul_trigger_linkloss_cnt);
    OAM_ERROR_LOG2(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_show::scan_cnt[0x%x] scan_result_cnt[0x%x].}",
                   pst_static->ul_scan_cnt, pst_static->ul_scan_result_cnt);
    OAM_ERROR_LOG3(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_show::connect_cnt[0x%x] roam_new_cnt[0x%x] roam_old_cnt[0x%x].}",
                   pst_static->ul_connect_cnt, pst_static->ul_roam_new_cnt, pst_static->ul_roam_old_cnt);
#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    OAM_WARNING_LOG3(uc_vap_id, OAM_SF_ROAM, "hmac_roam_show:roam_to, scan fail num = %d, %d(11v), eap fail num = %d",
                     pst_roam_info->st_static.ul_roam_scan_fail,
                     pst_roam_info->st_static.ul_roam_11v_scan_fail,
                     pst_roam_info->st_static.ul_roam_eap_fail);
#else
    OAM_WARNING_LOG2(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_show::roam_to, scan fail = %d, eap fail = %d}",
                     pst_roam_info->st_static.ul_roam_scan_fail,
                     pst_roam_info->st_static.ul_roam_eap_fail);
#endif

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_roam_main_null_fn(hmac_roam_info_stru *pst_roam_info, oal_void *p_param)
{
    OAM_WARNING_LOG0(0, OAM_SF_ROAM, "{hmac_roam_main_null_fn .}");

    return OAL_SUCC;
}


OAL_STATIC oal_void hmac_roam_scan_comp_cb(void *p_scan_record)
{
    hmac_scan_record_stru *pst_scan_record = (hmac_scan_record_stru *)p_scan_record;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    hmac_bss_mgmt_stru *pst_scan_bss_mgmt = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    /* 获取hmac vap */
    pst_hmac_vap = mac_res_get_hmac_vap(pst_scan_record->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{hmac_roam_scan_comp_cb::pst_hmac_vap is null.");
        return;
    }

    /* 获取hmac device */
    pst_hmac_device = hmac_res_get_mac_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_scan_complete::device null!}");
        return;
    }

    pst_scan_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_roam_info == OAL_PTR_NULL) {
        return;
    }

    /* 漫游开关没有开时，不处理扫描结果 */
    if (pst_roam_info->uc_enable == 0) {
        return;
    }

    OAM_INFO_LOG0(pst_scan_record->uc_vap_id, OAM_SF_ROAM, "{hmac_roam_scan_complete::handling scan result!}");

    ul_ret = hmac_roam_main_fsm_action(pst_roam_info, ROAM_MAIN_FSM_EVENT_SCAN_RESULT, (void *)pst_scan_bss_mgmt);
    if (ul_ret != OAL_SUCC) {
    }
    return;
}


OAL_STATIC oal_uint32 hmac_roam_scan_init(hmac_roam_info_stru *pst_roam_info, oal_void *p_param)
{
    oal_uint32 ul_ret;
    mac_scan_req_stru *pst_scan_params = OAL_PTR_NULL;
    oal_uint8 *puc_cur_ssid = OAL_PTR_NULL;

    ul_ret = hmac_roam_main_check_state(pst_roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_INIT);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_scan_init::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    if (mac_is_wep_enabled(&(pst_roam_info->pst_hmac_vap->st_vap_base_info))) {
        hmac_roam_ignore_rssi_trigger((pst_roam_info->pst_hmac_vap), OAL_TRUE);
        return OAL_SUCC;
    }

    pst_scan_params = &pst_roam_info->st_scan_params;
    puc_cur_ssid = mac_mib_get_DesiredSSID(&(pst_roam_info->pst_hmac_vap->st_vap_base_info));

    /* 扫描参数初始化 */
    pst_scan_params->en_bss_type = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    pst_scan_params->en_scan_type = WLAN_SCAN_TYPE_ACTIVE;
    pst_scan_params->us_scan_time = WLAN_DEFAULT_ACTIVE_SCAN_TIME;
    pst_scan_params->uc_probe_delay = 0;
    pst_scan_params->uc_scan_func = MAC_SCAN_FUNC_BSS; /* 默认扫描bss */
    pst_scan_params->p_fn_cb = hmac_roam_scan_comp_cb;
    pst_scan_params->uc_max_send_probe_req_count_per_channel = 2;
    pst_scan_params->uc_max_scan_count_per_channel = 2;

    if (memcpy_s(pst_scan_params->ast_mac_ssid_set[0].auc_ssid,
                 WLAN_SSID_MAX_LEN, puc_cur_ssid, WLAN_SSID_MAX_LEN) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "hmac_roam_scan_init::memcpy fail!");
    }
    pst_scan_params->uc_ssid_num = 1;

    /* 初始扫描请求只指定1个bssid，为广播地址 */
    memset_s(pst_scan_params->auc_bssid, WLAN_MAC_ADDR_LEN, 0xff, WLAN_MAC_ADDR_LEN);
    pst_scan_params->uc_bssid_num = 1;

    /* 更新扫描/关联时间戳 */
    pst_roam_info->st_static.ul_scan_timetamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    pst_roam_info->st_static.ul_connect_timetamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    pst_roam_info->st_static.ul_connect_comp_timetamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();
#ifdef _PRE_WLAN_1102A_CHR
    pst_roam_info->st_static.ul_roam_mode = HMAC_CHR_ROAM_NORMAL;
    pst_roam_info->st_static.ul_scan_mode = HMAC_CHR_NON_11K;
#endif
    ul_ret = hmac_roam_alg_scan_channel_init(pst_roam_info, pst_scan_params);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ROAM, "{hmac_roam_scan_init::hmac_roam_alg_scan_channel_init fail[%d]}", ul_ret);
        return ul_ret;
    }
    hmac_roam_main_change_state(pst_roam_info, ROAM_MAIN_STATE_SCANING);

    ul_ret = hmac_roam_main_fsm_action(pst_roam_info, ROAM_MAIN_FSM_EVENT_START, (oal_void *)pst_roam_info);
    if (ul_ret != OAL_SUCC) {
        hmac_roam_main_change_state(pst_roam_info, ROAM_MAIN_STATE_FAIL);
        return ul_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32 hmac_roam_scan_channel(hmac_roam_info_stru *pst_roam_info, oal_void *p_param)
{
    oal_uint32 ul_ret;

    ul_ret = hmac_roam_main_check_state(pst_roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_SCANING);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_scan_channel::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    pst_roam_info->st_static.ul_scan_cnt++;
    pst_roam_info->st_static.ul_scan_timetamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    /* 发起背景扫描 */
    ul_ret = hmac_fsm_call_func_sta(pst_roam_info->pst_hmac_vap, HMAC_FSM_INPUT_SCAN_REQ,
                                    (oal_void *)(&pst_roam_info->st_scan_params));
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG0(0, OAM_SF_ROAM, "{hmac_roam_scan_channel::start scan failed!}");
    }

    /* 启动扫描超时定时器 */
    hmac_roam_main_start_timer(pst_roam_info, ROAM_SCAN_TIME_MAX);

    return OAL_SUCC;
}


oal_uint32 hmac_roam_check_bkscan_result(hmac_vap_stru *pst_hmac_vap, void *p_scan_record)
{
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;
    hmac_scan_record_stru *pst_scan_record = (hmac_scan_record_stru *)p_scan_record;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    hmac_bss_mgmt_stru *pst_scan_bss_mgmt = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_entry = OAL_PTR_NULL;
    hmac_scanned_bss_info *pst_scanned_bss = OAL_PTR_NULL;
    mac_bss_dscr_stru *pst_bss_dscr = OAL_PTR_NULL;

    if ((pst_hmac_vap == OAL_PTR_NULL) || (pst_scan_record == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_check_bkscan_result::param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* STA背景扫描时，需要提前识别漫游场景 */
    if ((pst_hmac_vap->st_vap_base_info.en_vap_mode != WLAN_VAP_MODE_BSS_STA) ||
        (pst_hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_UP)) {
        return OAL_SUCC;
    }

    pst_hmac_device = hmac_res_get_mac_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_check_bkscan_result::device is null}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    if (mac_is_wep_enabled(&pst_hmac_vap->st_vap_base_info)) {
        return OAL_SUCC;
    }

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if ((pst_roam_info == OAL_PTR_NULL) || (pst_roam_info->uc_enable == 0)) {
        return OAL_SUCC;
    }
    pst_scan_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    oal_spin_lock(&(pst_scan_bss_mgmt->st_lock));

    /* 遍历扫描到的bss信息，查找可以漫游的bss */
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_scan_bss_mgmt->st_bss_list_head))
    {
        pst_scanned_bss = OAL_DLIST_GET_ENTRY(pst_entry, hmac_scanned_bss_info, st_dlist_head);
        pst_bss_dscr = &(pst_scanned_bss->st_bss_dscr_info);

        hmac_roam_alg_bss_in_ess(pst_roam_info, pst_bss_dscr);

        pst_bss_dscr = OAL_PTR_NULL;
    }

    oal_spin_unlock(&(pst_scan_bss_mgmt->st_lock));

    if (hmac_roam_alg_need_to_stop_roam_trigger(pst_roam_info) == OAL_TRUE) {
        return hmac_roam_ignore_rssi_trigger(pst_hmac_vap, OAL_TRUE);
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
OAL_STATIC oal_uint32 hmac_roam_check_11v_scan_result(
    hmac_roam_info_stru *pst_roam_info, oal_bool_enum_uint8 en_find_bss)
{
    hmac_vap_stru *pst_hmac_vap;
    hmac_user_stru *pst_hmac_user;
    hmac_user_11v_ctrl_stru *pst_11v_ctrl_info;
    hmac_scan_stru *pst_scan_mgmt;
    hmac_device_stru *pst_hmac_device;

    pst_hmac_vap = pst_roam_info->pst_hmac_vap;
    /* 获取发送端的用户指针 */
    pst_hmac_user = mac_res_get_hmac_user(pst_hmac_vap->st_vap_base_info.uc_assoc_vap_id);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                         "{hmac_roam_check_11v_scan_result::mac_res_get_hmac_user failed.}");
        return OAL_ERR_CODE_ROAM_INVALID_USER;
    }

    pst_hmac_device = hmac_res_get_mac_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_check_11v_scan_result::pst_hmac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_scan_mgmt = &(pst_hmac_device->st_scan_mgmt);

    pst_11v_ctrl_info = &(pst_hmac_user->st_11v_ctrl_info);

    OAM_WARNING_LOG2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                     "{hmac_roam_check_11v_scan_result::en_find_bss=[%d],uc_11v_roam_scan_times=[%d].}",
                     en_find_bss, pst_11v_ctrl_info->uc_11v_roam_scan_times);

    if (en_find_bss == OAL_TRUE) {
        pst_roam_info->st_bsst_rsp_info.uc_status_code = WNM_BSS_TM_ACCEPT;
        if (pst_roam_info->st_config.uc_scan_orthogonal == ROAM_SCAN_CHANNEL_ORG_1) {
            pst_11v_ctrl_info->uc_11v_roam_scan_times = MAC_11V_ROAM_SCAN_FINISH; /* 找到指定bss,本次11v漫游结束 */
        }
    } else {
        if (pst_roam_info->st_config.uc_scan_orthogonal == ROAM_SCAN_CHANNEL_ORG_1) {
            if ((pst_11v_ctrl_info->en_only_scan_one_time == OAL_FALSE) &&
                (pst_11v_ctrl_info->uc_11v_roam_scan_times <= MAC_11V_ROAM_SCAN_ONE_CHANNEL_LIMIT)) {
                /* 还需要再次触发漫游扫描 */
                pst_scan_mgmt->en_is_scanning = OAL_FALSE;
                return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
            }
        }
        /* When candidate BSSID is not in scan results, bss transition rsp with status code = 7 */
        pst_roam_info->st_bsst_rsp_info.uc_status_code = WNM_BSS_TM_REJECT_NO_SUITABLE_CANDIDATES;
    }

    if (pst_11v_ctrl_info->mac_11v_callback_fn) {
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                         "{hmac_roam_check_11v_scan_result:: start to send bsst rsp.}");
        pst_11v_ctrl_info->mac_11v_callback_fn(pst_hmac_vap, pst_hmac_user, &(pst_roam_info->st_bsst_rsp_info));
        pst_11v_ctrl_info->mac_11v_callback_fn = OAL_PTR_NULL;
        /* 记录连续11v成功触发漫游后拒绝的次数 */
        if (pst_roam_info->st_bsst_rsp_info.uc_status_code == WNM_BSS_TM_ACCEPT) {
            pst_11v_ctrl_info->uc_reject_bsstreq_times = 0;
        } else {
            pst_11v_ctrl_info->uc_reject_bsstreq_times++;
        }
        memset_s(&(pst_roam_info->st_bsst_rsp_info), OAL_SIZEOF(pst_roam_info->st_bsst_rsp_info),
                 0, OAL_SIZEOF(pst_roam_info->st_bsst_rsp_info));
    }

    if (en_find_bss == OAL_FALSE) {
        pst_roam_info->st_static.ul_roam_11v_scan_fail++;
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{roam_to::hmac_roam_check_11v_scan_result:11v candidate list no bss valid, \
            scan fail=%d}", pst_roam_info->st_static.ul_roam_11v_scan_fail);
        hmac_config_reg_info(&(pst_hmac_vap->st_vap_base_info), 4, "all");
        /* 根据连续返回失败数看是否开启过滤 */
        hmac_11v_bsst_req_filter(pst_hmac_vap, pst_11v_ctrl_info);
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    return OAL_SUCC;
}
#endif

oal_int8 hmac_get_rssi_from_scan_result(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_bssid)
{
    mac_bss_dscr_stru *pst_bss_dscr;

    pst_bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(&pst_hmac_vap->st_vap_base_info, puc_bssid);
    if (pst_bss_dscr != OAL_PTR_NULL) {
        return pst_bss_dscr->c_rssi;
    }

    return ROAM_RSSI_CMD_TYPE;
}


OAL_STATIC oal_uint32 hmac_roam_check_scan_result(
    hmac_roam_info_stru *pst_roam_info, oal_void *p_param, mac_bss_dscr_stru **ppst_bss_dscr_out)
{
    oal_uint32 ul_ret;
    hmac_bss_mgmt_stru *pst_bss_mgmt;
    hmac_vap_stru *pst_hmac_vap;
    oal_dlist_head_stru *pst_entry = OAL_PTR_NULL;
    hmac_scanned_bss_info *pst_scanned_bss = OAL_PTR_NULL;
    mac_bss_dscr_stru *pst_bss_dscr = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    oal_bool_enum_uint8 en_found_in_scan_results = OAL_FALSE;
#endif

    pst_hmac_vap = pst_roam_info->pst_hmac_vap;
    pst_bss_mgmt = (hmac_bss_mgmt_stru *)p_param;
    if ((pst_hmac_vap == OAL_PTR_NULL) || (pst_bss_mgmt == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_check_scan_result::vap invalid!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    pst_roam_info->st_static.ul_scan_result_cnt++;

    /* 如果扫描到的bss个数为0，退出 */
    if (pst_bss_mgmt->ul_bss_num == 0) {
        OAM_WARNING_LOG0(0, OAM_SF_ROAM, "{hmac_roam_check_scan_result::no bss scanned}");
#ifdef _PRE_WLAN_1102A_CHR
        hmac_chr_roam_info_report(pst_roam_info, HMAC_CHR_ROAM_SCAN_FAIL);
#endif
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }
#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    if (pst_roam_info->en_roam_trigger == ROAM_TRIGGER_11V) {
        pst_roam_info->st_alg.c_current_rssi =
            hmac_get_rssi_from_scan_result(pst_hmac_vap, pst_hmac_vap->st_vap_base_info.auc_bssid);
        pst_roam_info->st_bsst_rsp_info.c_rssi = pst_roam_info->st_alg.c_current_rssi;
    }
#endif
    /* 获取锁 */
    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    /* 遍历扫描到的bss信息，查找可以漫游的bss */
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_bss_mgmt->st_bss_list_head))
    {
        pst_scanned_bss = OAL_DLIST_GET_ENTRY(pst_entry, hmac_scanned_bss_info, st_dlist_head);
        pst_bss_dscr = &(pst_scanned_bss->st_bss_dscr_info);

        ul_ret = hmac_roam_alg_bss_check(pst_roam_info, pst_bss_dscr);
        if (ul_ret != OAL_SUCC) {
            continue;
        }

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
        if (pst_roam_info->en_roam_trigger == ROAM_TRIGGER_11V) {
            if (!oal_memcmp(pst_roam_info->st_bsst_rsp_info.auc_target_bss_addr,
                            pst_bss_dscr->auc_bssid, WLAN_MAC_ADDR_LEN)) {
                en_found_in_scan_results = OAL_TRUE;
                break;
            }
        }
#endif /* _PRE_WLAN_FEATURE_11V_ENABLE */
        pst_bss_dscr = OAL_PTR_NULL;
    }

    /* 解除锁 */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    if (pst_roam_info->en_roam_trigger == ROAM_TRIGGER_11V) {
        if (hmac_roam_check_11v_scan_result(pst_roam_info, en_found_in_scan_results) != OAL_SUCC) {
#ifdef _PRE_WLAN_1102A_CHR
            hmac_chr_roam_info_report(pst_roam_info, HMAC_CHR_ROAM_SCAN_FAIL);
#endif
            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
        }
    }
#endif /* _PRE_WLAN_FEATURE_11V_ENABLE */

    pst_bss_dscr = hmac_roam_alg_select_bss(pst_roam_info);
    if (pst_bss_dscr == OAL_PTR_NULL) {
        /* 没有扫描到可用的bss，等待定时器超时即可 */
        OAM_WARNING_LOG0(0, OAM_SF_ROAM, "{hmac_roam_check_scan_result::no bss valid}");
#ifdef _PRE_WLAN_1102A_CHR
        hmac_chr_roam_info_report(pst_roam_info, HMAC_CHR_ROAM_SCAN_FAIL);
#endif
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    ul_ret = hmac_roam_renew_privacy(pst_hmac_vap, pst_bss_dscr);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_check_scan_result::renew privacy failed, reason[%d]!", ul_ret);
        return ul_ret;
    }

    ul_ret = hmac_check_capability_mac_phy_supplicant(&(pst_hmac_vap->st_vap_base_info), pst_bss_dscr);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "hmac_roam_check_scan_result:check mac and phy capability fail[%d]", ul_ret);
    }

    *ppst_bss_dscr_out = pst_bss_dscr;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_roam_handle_scan_result(hmac_roam_info_stru *pst_roam_info, oal_void *p_param)
{
    mac_bss_dscr_stru *pst_bss_dscr = OAL_PTR_NULL;
    hmac_scan_rsp_stru st_scan_rsp;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    ul_ret = hmac_roam_main_check_state(pst_roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_SCANING);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_handle_scan_result::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    pst_mac_device = mac_res_get_dev(pst_roam_info->pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_handle_scan_result::pst_mac_device null.}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    ul_ret = hmac_roam_check_scan_result(pst_roam_info, p_param, &pst_bss_dscr);
    if (ul_ret == OAL_SUCC) {
        pst_roam_info->uc_invalid_scan_cnt = 0;
        pst_roam_info->st_static.ul_connect_timetamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();

        /* 扫描结果发给sme */
        memset_s(&st_scan_rsp, OAL_SIZEOF(st_scan_rsp), 0, OAL_SIZEOF(st_scan_rsp));

        st_scan_rsp.uc_result_code = MAC_SCAN_SUCCESS;

        hmac_send_rsp_to_sme_sta(pst_roam_info->pst_hmac_vap, HMAC_SME_SCAN_RSP, (oal_uint8 *)&st_scan_rsp);

        /* 扫描到可用的bss，开始connect */
        return hmac_roam_main_fsm_action(pst_roam_info, ROAM_MAIN_FSM_EVENT_START_CONNECT, (oal_void *)pst_bss_dscr);
    }

    /* 如果是亮屏的，不暂停漫游 */
    if (pst_mac_device->uc_in_suspend == OAL_FALSE) {
        pst_roam_info->uc_invalid_scan_cnt = 0;
    } else {
        pst_roam_info->uc_invalid_scan_cnt++;
    }

    /* 多次无效扫描暂停漫游，防止在某些场景下一直唤醒HOST */
    if (pst_roam_info->uc_invalid_scan_cnt >= ROAM_INVALID_SCAN_MAX) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_handle_scan_result::ignore_rssi_trigger after %d invalid_scan.}",
                         pst_roam_info->uc_invalid_scan_cnt);
        pst_roam_info->uc_invalid_scan_cnt = 0;
        hmac_roam_ignore_rssi_trigger(pst_roam_info->pst_hmac_vap, OAL_TRUE);
    }
    /* 删除定时器 */
    hmac_roam_main_del_timer(pst_roam_info);
    hmac_roam_main_clear(pst_roam_info);
#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    /* 重新触发漫游检查 */
    if (pst_roam_info->pst_hmac_vap->bit_11v_enable == OAL_TRUE) {
        hmac_11v_roam_scan_check(pst_roam_info->pst_hmac_vap);
    }
#endif

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_roam_main_check_state(hmac_roam_info_stru *pst_roam_info,
                                                 mac_vap_state_enum_uint8 en_vap_state,
                                                 roam_main_state_enum_uint8 en_main_state)
{
    if (pst_roam_info == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_roam_info->pst_hmac_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    if (pst_roam_info->pst_hmac_user == OAL_PTR_NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_USER;
    }

    if (pst_roam_info->uc_enable == 0) {
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    if ((pst_roam_info->pst_hmac_vap->st_vap_base_info.en_vap_state != en_vap_state) ||
        (pst_roam_info->en_main_state != en_main_state)) {
        OAM_WARNING_LOG2(0, OAM_SF_ROAM, "{hmac_roam_main_check_state::unexpect vap State[%d] main_state[%d]!}",
                         pst_roam_info->pst_hmac_vap->st_vap_base_info.en_vap_state, pst_roam_info->en_main_state);
        return OAL_ERR_CODE_ROAM_INVALID_VAP_STATUS;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_void hmac_roam_main_clear(hmac_roam_info_stru *pst_roam_info)
{
    /* 清理状态 */
    hmac_roam_main_change_state(pst_roam_info, ROAM_MAIN_STATE_INIT);

    hmac_roam_connect_stop(pst_roam_info->pst_hmac_vap);
}


OAL_STATIC oal_void hmac_roam_resume_pm(hmac_roam_info_stru *pst_roam_info, oal_void *p_param)
{
#ifdef _PRE_WLAN_FEATURE_STA_PM
    oal_uint32 ul_ret;

    ul_ret = hmac_config_set_pm_by_module(&pst_roam_info->pst_hmac_vap->st_vap_base_info,
                                          MAC_STA_PM_CTRL_TYPE_ROAM, MAC_STA_PM_SWITCH_ON);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_roam_info->pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                         "{hmac_roam_resume_pm::hmac_config_set_pm_by_module failed[%d].}", ul_ret);
    }
    return;
#else
    return;
#endif
}


OAL_STATIC oal_void hmac_roam_resume_security_port(hmac_roam_info_stru *pst_roam_info, oal_void *p_param)
{
    oal_uint32 ul_ret;
    mac_h2d_roam_sync_stru st_h2d_sync = { 0 };

    if (pst_roam_info == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_resume_security_port::pst_roam_info null!}");
        return;
    }

    if (pst_roam_info->pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_resume_security_port::vap null!}");
        return;
    }

    if (pst_roam_info->pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_resume_security_port::user null!}");
        return;
    }

    /* 设置用户8021x端口合法性的状态为合法 */
    mac_user_set_port(&pst_roam_info->pst_hmac_user->st_user_base_info, OAL_TRUE);

    // 填充同步信息
    st_h2d_sync.ul_back_to_old = OAL_FALSE;

    // 发送同步信息
    ul_ret = hmac_config_send_event(&pst_roam_info->pst_hmac_vap->st_vap_base_info, WLAN_CFGID_ROAM_HMAC_SYNC_DMAC,
                                    OAL_SIZEOF(mac_h2d_roam_sync_stru), (oal_uint8 *)&st_h2d_sync);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_roam_info->pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "hmac_roam_resume_security_port:send event[WLAN_CFGID_ROAM_HMAC_SYNC_DMAC] failed[%d]", ul_ret);
    }

    return;
}


OAL_STATIC oal_uint32 hmac_roam_scan_timeout(hmac_roam_info_stru *pst_roam_info, oal_void *p_param)
{
    oal_uint32 ul_ret;

    ul_ret = hmac_roam_main_check_state(pst_roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_SCANING);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_scan_timeout::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    ul_ret = hmac_roam_scan_complete(pst_roam_info->pst_hmac_vap);
    if (ul_ret != OAL_SUCC) {
    }
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_roam_connecting_timeout(hmac_roam_info_stru *pst_roam_info, oal_void *p_param)
{
    oal_uint32 ul_ret;
    oal_bool_enum_uint8 post_dmac = OAL_TRUE;

    ul_ret = hmac_roam_main_check_state(pst_roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_connecting_timeout::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    ul_ret = hmac_roam_to_old_bss(pst_roam_info, p_param);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM,
                         "{hmac_roam_handle_fail_connect_phase:: hmac_roam_to_old_bss fail[%d]!}", ul_ret);
    }

    /* 握手失败不需要恢复device漫游状态 */
    if (pst_roam_info->st_connect.en_state == ROAM_CONNECT_STATE_HANDSHAKING) {
        post_dmac = OAL_FALSE;
    }

#ifdef _PRE_WLAN_1102A_CHR
    hmac_chr_roam_info_report(pst_roam_info, HMAC_CHR_ROAM_TIMEOUT_FAIL);
#endif
    /* 切换vap的状态为UP，恢复用户节能，恢复发送 */
    ul_ret = hmac_fsm_call_func_sta(pst_roam_info->pst_hmac_vap, HMAC_FSM_INPUT_ROAMING_STOP, (oal_void *)&post_dmac);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ROAM,
                       "{hmac_roam_handle_fail_connect_phase:: hmac_fsm_call_func_sta fail[%d]!}", ul_ret);
    }

    hmac_roam_main_clear(pst_roam_info);

    return ul_ret;
}


OAL_STATIC oal_uint32 hmac_roam_connecting_fail(hmac_roam_info_stru *pst_roam_info, oal_void *p_param)
{
    oal_uint32 ul_ret;
    oal_bool_enum_uint8 en_is_protected;

    ul_ret = hmac_roam_main_check_state(pst_roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_connecting_fail::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    hmac_roam_main_del_timer(pst_roam_info);
    ul_ret = hmac_roam_connecting_timeout(pst_roam_info, p_param);
    if (ul_ret) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_connecting_fail::hmac_roam_to_old_bss fail ul_ret=[%d]!}", ul_ret);
        return ul_ret;
    }
#ifdef _PRE_WLAN_1102A_CHR
    hmac_chr_roam_info_report(pst_roam_info, HMAC_CHR_ROAM_CONNECT_FAIL);
#endif

    /* 为了提高漫游成功的概率，在Auth/Reassoc No rsp时立即触发重新漫游 */
    if (pst_roam_info->st_alg.c_current_rssi > ROAM_RSSI_MAX_TYPE) {
        /* 如果是弱信号触发的漫游，先把rssi修改成ROAM_RSSI_LINKLOSS_TYPE来将弱信号跟LINKLOSS触发的重漫游归一 */
        if (pst_roam_info->st_alg.c_current_rssi > ROAM_RSSI_LINKLOSS_TYPE) {
            pst_roam_info->st_alg.c_current_rssi = ROAM_RSSI_LINKLOSS_TYPE;
        }
        /* 漫游失败时，rssi 逐次减1dBm，一直到到ROAM_RSSI_MAX_TYPE。这样可以最多触发5次重漫游 */
        return hmac_roam_trigger_handle(pst_roam_info->pst_hmac_vap, pst_roam_info->st_alg.c_current_rssi - 1,
                                        OAL_TRUE);
    }

    
    if (pst_roam_info->st_connect.en_status_code == MAC_REJECT_TEMP) {
        return ul_ret;
    }

    /* 管理帧加密是否开启 */
    en_is_protected = pst_roam_info->pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active;

    /* 发去关联帧 */
    hmac_mgmt_send_disassoc_frame(&(pst_roam_info->pst_hmac_vap->st_vap_base_info),
                                  pst_roam_info->pst_hmac_user->st_user_base_info.auc_user_mac_addr,
                                  MAC_DEAUTH_LV_SS, en_is_protected);

    /* 删除对应用户 */
    hmac_user_del(&(pst_roam_info->pst_hmac_vap->st_vap_base_info), pst_roam_info->pst_hmac_user);

    hmac_sta_handle_disassoc_rsp(pst_roam_info->pst_hmac_vap, MAC_NOT_ASSOCED);

#ifdef _PRE_WLAN_1102A_CHR
    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                         CHR_WIFI_DEV_EVENT_CHIP, ROAM_FAIL_FIVE_TIMES);
#endif

    return ul_ret;
}


OAL_STATIC oal_uint32 hmac_roam_handle_fail_handshake_phase(hmac_roam_info_stru *pst_roam_info, oal_void *p_param)
{
    oal_uint32 ul_ret;
    oal_bool_enum_uint8 en_is_protected;
    oal_bool_enum_uint8 post_dmac = OAL_FALSE; /* 握手失败不需要恢复device漫游状态 */

    ul_ret = hmac_roam_main_check_state(pst_roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_handle_fail_handshake_phase::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    /* 切换vap的状态为UP，恢复用户节能，恢复发送 */
    ul_ret = hmac_fsm_call_func_sta(pst_roam_info->pst_hmac_vap, HMAC_FSM_INPUT_ROAMING_STOP, (oal_void *)&post_dmac);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_roam_info->pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_handle_fail_handshake_phase:: hmac_fsm_call_func_sta fail[%d]!}", ul_ret);
    }

    hmac_roam_main_clear(pst_roam_info);
    hmac_roam_main_del_timer(pst_roam_info);
#ifdef _PRE_WLAN_1102A_CHR
    hmac_chr_roam_info_report(pst_roam_info, HMAC_CHR_ROAM_HANDSHAKE_FAIL);
#endif

    /* 为提高漫游成功的概率，在握手失败时触发立即重新漫游 */
    if (pst_roam_info->st_alg.c_current_rssi > ROAM_RSSI_MAX_TYPE) {
        /* 如果是弱信号触发的漫游，先把rssi修改成ROAM_RSSI_LINKLOSS_TYPE来将弱信号跟LINKLOSS触发的重漫游归一 */
        if (pst_roam_info->st_alg.c_current_rssi > ROAM_RSSI_LINKLOSS_TYPE) {
            pst_roam_info->st_alg.c_current_rssi = ROAM_RSSI_LINKLOSS_TYPE;
        }
        /* 漫游握手失败时，rssi 逐次减1dBm，一直到到ROAM_RSSI_MAX_TYPE。这样可以最多触发5次重漫游 */
        return hmac_roam_trigger_handle(pst_roam_info->pst_hmac_vap, pst_roam_info->st_alg.c_current_rssi - 1,
                                        OAL_TRUE);
    }

    OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_handle_fail_handshake_phase:: report deauth to wpas! \
        c_current_rssi=%d}", pst_roam_info->st_alg.c_current_rssi);

    /* 管理帧加密是否开启 */
    en_is_protected = pst_roam_info->pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active;

    /* 发去关联帧 */
    hmac_mgmt_send_disassoc_frame(&(pst_roam_info->pst_hmac_vap->st_vap_base_info),
                                  pst_roam_info->pst_hmac_user->st_user_base_info.auc_user_mac_addr,
                                  MAC_DEAUTH_LV_SS, en_is_protected);

    /* 删除对应用户 */
    hmac_user_del(&(pst_roam_info->pst_hmac_vap->st_vap_base_info), pst_roam_info->pst_hmac_user);

    hmac_sta_handle_disassoc_rsp(pst_roam_info->pst_hmac_vap, MAC_4WAY_HANDSHAKE_TIMEOUT);

#ifdef _PRE_WLAN_1102A_CHR
    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                         CHR_WIFI_DEV_EVENT_CHIP, ROAM_FAIL_FIVE_TIMES);
#endif

    return ul_ret;
}

oal_uint32 hmac_roam_save_old_bss_info(
    mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user, hmac_roam_info_stru *pst_roam_info)
{
    hmac_roam_old_bss_stru *pst_old_bss;

    pst_old_bss = &pst_roam_info->st_old_bss;
    pst_old_bss->us_sta_aid = pst_mac_vap->us_sta_aid;
    pst_old_bss->en_protocol_mode = pst_mac_vap->en_protocol;
    memcpy_s(&pst_old_bss->st_cap_info, OAL_SIZEOF(mac_user_cap_info_stru),
             &(pst_hmac_user->st_user_base_info.st_cap_info), OAL_SIZEOF(mac_user_cap_info_stru));
    memcpy_s(&pst_old_bss->st_key_info, OAL_SIZEOF(mac_key_mgmt_stru), &(pst_hmac_user->st_user_base_info.st_key_info),
             OAL_SIZEOF(mac_key_mgmt_stru));
    memcpy_s(&pst_old_bss->st_user_tx_info, OAL_SIZEOF(mac_user_tx_param_stru),
             &(pst_hmac_user->st_user_base_info.st_user_tx_info), OAL_SIZEOF(mac_user_tx_param_stru));
    memcpy_s(&pst_old_bss->st_mib_info, OAL_SIZEOF(wlan_mib_ieee802dot11_stru),
             pst_mac_vap->pst_mib_info, OAL_SIZEOF(wlan_mib_ieee802dot11_stru));
    if (memcpy_s(&pst_old_bss->st_op_rates, OAL_SIZEOF(mac_rate_stru), &(pst_hmac_user->st_op_rates),
                 OAL_SIZEOF(mac_rate_stru)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "hmac_roam_save_old_bss_info::memcpy fail!");
        return OAL_FAIL;
    }
    memcpy_s(&pst_old_bss->st_ht_hdl, OAL_SIZEOF(mac_user_ht_hdl_stru), &(pst_hmac_user->st_user_base_info.st_ht_hdl),
             OAL_SIZEOF(mac_user_ht_hdl_stru));
    memcpy_s(&pst_old_bss->st_vht_hdl, OAL_SIZEOF(mac_vht_hdl_stru), &(pst_hmac_user->st_user_base_info.st_vht_hdl),
             OAL_SIZEOF(mac_vht_hdl_stru));
    pst_old_bss->en_avail_bandwidth = pst_hmac_user->st_user_base_info.en_avail_bandwidth;
    pst_old_bss->en_cur_bandwidth = pst_hmac_user->st_user_base_info.en_cur_bandwidth;
    memcpy_s(&pst_old_bss->st_channel, OAL_SIZEOF(mac_channel_stru), &(pst_mac_vap->st_channel),
             OAL_SIZEOF(mac_channel_stru));
    memcpy_s(&pst_old_bss->auc_bssid, WLAN_MAC_ADDR_LEN, &(pst_mac_vap->auc_bssid), WLAN_MAC_ADDR_LEN);
    pst_old_bss->us_cap_info = pst_mac_vap->us_assoc_user_cap_info;
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    pst_old_bss->en_pmf_required = mac_mib_get_dot11RSNAMFPR(pst_mac_vap);
#endif
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_roam_connect_to_bss(hmac_roam_info_stru *pst_roam_info, oal_void *p_param)
{
    hmac_vap_stru *pst_hmac_vap = pst_roam_info->pst_hmac_vap;
    hmac_user_stru *pst_hmac_user = pst_roam_info->pst_hmac_user;
    mac_bss_dscr_stru *pst_bss_dscr = (mac_bss_dscr_stru *)p_param;
    oal_uint32 ul_ret;
    oal_uint32 ul_need_to_stop_user = 1;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    oal_uint8 uc_vap_idx;
    mac_vap_stru *pst_other_vap = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_11R
    wlan_mib_Dot11FastBSSTransitionConfigEntry_stru *pst_wlan_mib_ft_cfg;
#endif  // _PRE_WLAN_FEATURE_11R

    ul_ret = hmac_roam_main_check_state(pst_roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_SCANING);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_connect_to_bss::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    pst_mac_vap = &pst_hmac_vap->st_vap_base_info;
    pst_mac_device = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "{hmac_roam_connect_to_bss::pst_mac_device ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* check all vap state in case other vap is connecting now */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_other_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_other_vap == OAL_PTR_NULL) {
            OAM_WARNING_LOG0(0, OAM_SF_ROAM, "hmac_roam_connect_to_bss:vap is null");
            continue;
        }

        if ((pst_other_vap->en_vap_state >= MAC_VAP_STATE_STA_JOIN_COMP) &&
            (pst_other_vap->en_vap_state <= MAC_VAP_STATE_STA_WAIT_ASOC)) {
            OAM_WARNING_LOG1(pst_other_vap->uc_vap_id, OAM_SF_ROAM,
                             "hmac_roam_connect_to_bss:vap is connecting, state=%d", pst_other_vap->en_vap_state);
            hmac_roam_main_change_state(pst_roam_info, ROAM_MAIN_STATE_INIT);
            return OAL_FAIL;
        }
    }

#ifdef _PRE_WLAN_FEATURE_11R
    pst_wlan_mib_ft_cfg = &pst_hmac_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_fast_bss_trans_cfg;
    if (pst_wlan_mib_ft_cfg->en_dot11FastBSSTransitionActivated && pst_wlan_mib_ft_cfg->en_dot11FTOverDSActivated) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "hmac_roam_connect_to_bss:bit_11r_over_ds=%d", pst_hmac_vap->bit_11r_over_ds);
        if (pst_hmac_vap->bit_11r_over_ds) {
            ul_need_to_stop_user = 0;
        }
    }
#endif  // _PRE_WLAN_FEATURE_11R

    if (ul_need_to_stop_user != 0) {
        /* 切换vap的状态为ROAMING，将用户节能，暂停发送 */
        ul_ret = hmac_fsm_call_func_sta(pst_hmac_vap, HMAC_FSM_INPUT_ROAMING_START, (oal_void *)pst_bss_dscr);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_connect_to_bss::hmac_fsm_call_func_sta fail[%ld]!}", ul_ret);
            return ul_ret;
        }
    }

    /* 原bss信息保存，以便回退 */
    if (hmac_roam_save_old_bss_info(pst_mac_vap, pst_hmac_user, pst_roam_info) != OAL_SUCC) {
        hmac_roam_main_change_state(pst_roam_info, ROAM_MAIN_STATE_INIT);
        return OAL_FAIL;
    }
    /* 切换状态至connecting */
    hmac_roam_main_change_state(pst_roam_info, ROAM_MAIN_STATE_CONNECTING);

    pst_roam_info->st_static.ul_connect_cnt++;

    /* 设置漫游到的bss能力位，重关联请求使用 */
    pst_hmac_vap->st_vap_base_info.us_assoc_user_cap_info = pst_bss_dscr->us_cap_info;

    hmac_config_set_mgmt_log(&pst_hmac_vap->st_vap_base_info, &pst_hmac_user->st_user_base_info, OAL_TRUE);

    /* 启动connect状态机 */
    ul_ret = hmac_roam_connect_start(pst_hmac_vap, pst_bss_dscr);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ROAM, "{hmac_roam_connect_to_bss::hmac_roam_connect_start fail[%ld]!}", ul_ret);
        hmac_roam_main_change_state(pst_roam_info, ROAM_MAIN_STATE_FAIL);
        return ul_ret;
    }
    /* 启动connect超时定时器 */
    hmac_roam_main_start_timer(pst_roam_info, ROAM_CONNECT_TIME_MAX);
    return OAL_SUCC;
}

oal_void hmac_roam_recover_old_bss_info(
    mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user, hmac_roam_old_bss_stru *pst_old_bss)
{
    pst_mac_vap->us_sta_aid = pst_old_bss->us_sta_aid;
    pst_mac_vap->en_protocol = pst_old_bss->en_protocol_mode;
    memcpy_s(&(pst_hmac_user->st_user_base_info.st_cap_info), OAL_SIZEOF(mac_user_cap_info_stru),
             &pst_old_bss->st_cap_info, OAL_SIZEOF(mac_user_cap_info_stru));
    memcpy_s(&(pst_hmac_user->st_user_base_info.st_key_info), OAL_SIZEOF(mac_key_mgmt_stru),
             &pst_old_bss->st_key_info, OAL_SIZEOF(mac_key_mgmt_stru));
    memcpy_s(&(pst_hmac_user->st_user_base_info.st_user_tx_info), OAL_SIZEOF(mac_user_tx_param_stru),
             &pst_old_bss->st_user_tx_info, OAL_SIZEOF(mac_user_tx_param_stru));
    memcpy_s(pst_mac_vap->pst_mib_info, OAL_SIZEOF(wlan_mib_ieee802dot11_stru),
             &pst_old_bss->st_mib_info, OAL_SIZEOF(wlan_mib_ieee802dot11_stru));
    memcpy_s(&(pst_hmac_user->st_op_rates), OAL_SIZEOF(mac_rate_stru),
             &pst_old_bss->st_op_rates, OAL_SIZEOF(mac_rate_stru));
    memcpy_s(&(pst_hmac_user->st_user_base_info.st_ht_hdl), OAL_SIZEOF(mac_user_ht_hdl_stru),
             &pst_old_bss->st_ht_hdl, OAL_SIZEOF(mac_user_ht_hdl_stru));
    memcpy_s(&(pst_hmac_user->st_user_base_info.st_vht_hdl), OAL_SIZEOF(mac_vht_hdl_stru),
             &pst_old_bss->st_vht_hdl, OAL_SIZEOF(mac_vht_hdl_stru));
    pst_hmac_user->st_user_base_info.en_avail_bandwidth = pst_old_bss->en_avail_bandwidth;
    pst_hmac_user->st_user_base_info.en_cur_bandwidth = pst_old_bss->en_cur_bandwidth;
    memcpy_s(&(pst_mac_vap->st_channel), OAL_SIZEOF(mac_channel_stru),
             &pst_old_bss->st_channel, OAL_SIZEOF(mac_channel_stru));
    memcpy_s(pst_mac_vap->auc_bssid, WLAN_MAC_ADDR_LEN, pst_old_bss->auc_bssid, WLAN_MAC_ADDR_LEN);
    pst_mac_vap->us_assoc_user_cap_info = pst_old_bss->us_cap_info;
}

OAL_STATIC oal_uint32 hmac_roam_to_old_bss(hmac_roam_info_stru *pst_roam_info, oal_void *p_param)
{
    oal_uint32 ul_ret;
    hmac_vap_stru *pst_hmac_vap = pst_roam_info->pst_hmac_vap;
    mac_vap_stru *pst_mac_vap = &pst_hmac_vap->st_vap_base_info;
    hmac_user_stru *pst_hmac_user = pst_roam_info->pst_hmac_user;
    hmac_roam_old_bss_stru *pst_old_bss = &pst_roam_info->st_old_bss;
    mac_h2d_roam_sync_stru *pst_h2d_sync = OAL_PTR_NULL;

    if (pst_old_bss == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_to_old_bss::pst_old_bss null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_roam_info->st_static.ul_roam_old_cnt++;
    pst_roam_info->st_static.ul_roam_eap_fail++;

    /* 恢复原来bss相关信息 */
    hmac_roam_recover_old_bss_info(pst_mac_vap, pst_hmac_user, pst_old_bss);
    /* 设置用户8021x端口合法性的状态为合法 */
    mac_user_set_port(&pst_hmac_user->st_user_base_info, OAL_TRUE);

    if (pst_old_bss->en_protocol_mode >= WLAN_HT_MODE) {
        pst_hmac_vap->en_tx_aggr_on = OAL_TRUE;
        pst_hmac_vap->en_amsdu_active = OAL_TRUE;
    } else {
        pst_hmac_vap->en_tx_aggr_on = OAL_FALSE;
        pst_hmac_vap->en_amsdu_active = OAL_FALSE;
    }

    ul_ret = hmac_config_start_vap_event(pst_mac_vap, OAL_FALSE);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "hmac_roam_to_old_bss:start vap failed[%d]", ul_ret);
        return ul_ret;
    }

    /* 相关参数需要配置到dmac */
    hmac_roam_connect_set_join_reg(pst_mac_vap);

    /* 更新用户的mac地址，漫游时mac会更新 */
    oal_set_mac_addr(pst_hmac_user->st_user_base_info.auc_user_mac_addr, pst_mac_vap->auc_bssid);

#if _PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT
    /* 此函数要在同步user能力前调用，同步user能力后会开启硬件PMF使能 */
    hmac_config_set_pmf_cap(pst_mac_vap, pst_old_bss->st_cap_info.bit_pmf_active, pst_old_bss->en_pmf_required);
#endif

    ul_ret = hmac_config_user_info_syn(&(pst_hmac_vap->st_vap_base_info), &(pst_hmac_user->st_user_base_info));
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "hmac_roam_to_old_bss:syn_vap_state failed[%d]", ul_ret);
    }

    /* 回退 bss 时，hmac 2 dmac 同步的相关信息，以便失败的时候回退 */
    pst_h2d_sync = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(mac_h2d_roam_sync_stru), OAL_TRUE);
    if (pst_h2d_sync == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "{hmac_roam_to_old_bss::no buff to alloc sync info!}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    // 填充同步信息
    pst_h2d_sync->ul_back_to_old = OAL_TRUE;
    pst_h2d_sync->us_sta_aid = pst_old_bss->us_sta_aid;
    memcpy_s(&(pst_h2d_sync->st_channel), OAL_SIZEOF(mac_channel_stru),
             &pst_old_bss->st_channel, OAL_SIZEOF(mac_channel_stru));
    memcpy_s(&(pst_h2d_sync->st_cap_info), OAL_SIZEOF(mac_user_cap_info_stru),
             &pst_old_bss->st_cap_info, OAL_SIZEOF(mac_user_cap_info_stru));
    memcpy_s(&(pst_h2d_sync->st_key_info), OAL_SIZEOF(mac_key_mgmt_stru),
             &pst_old_bss->st_key_info, OAL_SIZEOF(mac_key_mgmt_stru));
    memcpy_s(&(pst_h2d_sync->st_user_tx_info), OAL_SIZEOF(mac_user_tx_param_stru),
             &pst_old_bss->st_user_tx_info, OAL_SIZEOF(mac_user_tx_param_stru));

    /* 在漫游过程中可能又建立了聚合，因此回退时需要删除掉 */
    hmac_tid_clear(pst_mac_vap, pst_hmac_user);

    hmac_roam_reset_rssi(pst_hmac_vap);

    // 发送同步信息
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_ROAM_HMAC_SYNC_DMAC,
                                    OAL_SIZEOF(mac_h2d_roam_sync_stru), (oal_uint8 *)pst_h2d_sync);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "{hmac_roam_to_old_bss::send event failed[%d].}", ul_ret);
    }

    /* 释放同步数据 */
    if (pst_h2d_sync != OAL_PTR_NULL) {
        OAL_MEM_FREE(pst_h2d_sync, OAL_TRUE);
        pst_h2d_sync = OAL_PTR_NULL;
    }

    /* user已经关联上，抛事件给DMAC，在DMAC层挂用户算法钩子 */
    hmac_user_add_notify_alg(&pst_hmac_vap->st_vap_base_info, pst_hmac_user->st_user_base_info.us_assoc_id);
    hmac_config_set_mgmt_log(&pst_hmac_vap->st_vap_base_info, &pst_hmac_user->st_user_base_info, OAL_FALSE);

    OAM_WARNING_LOG4(0, OAM_SF_ROAM, "hmac_roam_to_old_bss:now resuming to [%02X:XX:XX:%02X:%02X:%02X]}",
                     pst_old_bss->auc_bssid[0], pst_old_bss->auc_bssid[3],
                     pst_old_bss->auc_bssid[4], pst_old_bss->auc_bssid[5]);

    return ul_ret;
}


OAL_STATIC oal_uint32 hmac_roam_to_new_bss(hmac_roam_info_stru *pst_roam_info, oal_void *p_param)
{
    hmac_vap_stru *pst_hmac_vap = pst_roam_info->pst_hmac_vap;
    hmac_user_stru *pst_hmac_user = pst_roam_info->pst_hmac_user;
    oal_uint32 ul_ret;
    oal_bool_enum_uint8 post_dmac = OAL_TRUE;

    ul_ret = hmac_roam_main_check_state(pst_roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_to_new_bss::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    pst_roam_info->st_static.ul_roam_new_cnt++;
    hmac_roam_alg_add_history(pst_roam_info, pst_hmac_vap->st_vap_base_info.auc_bssid);

    hmac_roam_main_change_state(pst_roam_info, ROAM_MAIN_STATE_UP);

    hmac_roam_main_del_timer(pst_roam_info);

    /* 切换vap的状态为UP，恢复用户节能，恢复发送 */
    ul_ret = hmac_fsm_call_func_sta(pst_hmac_vap, HMAC_FSM_INPUT_ROAMING_STOP, (oal_void *)&post_dmac);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_to_new_bss::hmac_fsm_call_func_sta fail! erro code is %u}", ul_ret);
    }
    hmac_config_set_mgmt_log(&pst_hmac_vap->st_vap_base_info, &pst_hmac_user->st_user_base_info, OAL_FALSE);
#ifdef _PRE_WLAN_1102A_CHR
    hmac_chr_roam_info_report(pst_roam_info, HMAC_CHR_ROAM_SUCCESS);
#endif
    OAM_WARNING_LOG4(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                     "{hmac_roam_to_new_bss::roam to [%02X:XX:XX:%02X:%02X:%02X] succ}",
                     pst_hmac_vap->st_vap_base_info.auc_bssid[0], pst_hmac_vap->st_vap_base_info.auc_bssid[3],
                     pst_hmac_vap->st_vap_base_info.auc_bssid[4], pst_hmac_vap->st_vap_base_info.auc_bssid[5]);
    hmac_roam_main_clear(pst_roam_info);

    return ul_ret;
}


oal_uint32 hmac_roam_ignore_rssi_trigger(hmac_vap_stru *pst_hmac_vap, oal_bool_enum_uint8 en_val)
{
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_uint8 uc_vap_id;
    mac_roam_trigger_stru st_roam_trigger;

    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_stop_roam_trigger::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    uc_vap_id = pst_hmac_vap->st_vap_base_info.uc_vap_id;

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_roam_info == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_stop_roam_trigger::roam info null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    if (pst_roam_info->uc_rssi_ignore == en_val) {
        return OAL_SUCC;
    }

    if (en_val == OAL_TRUE) {
        st_roam_trigger.c_trigger_2G = ROAM_RSSI_LINKLOSS_TYPE;
        st_roam_trigger.c_trigger_5G = ROAM_RSSI_LINKLOSS_TYPE;
    } else {
        st_roam_trigger.c_trigger_2G = pst_roam_info->st_config.c_trigger_rssi_2G;
        st_roam_trigger.c_trigger_5G = pst_roam_info->st_config.c_trigger_rssi_5G;
    }
    ul_ret = hmac_config_send_event(&pst_hmac_vap->st_vap_base_info,
                                    WLAN_CFGID_SET_ROAM_TRIGGER,
                                    OAL_SIZEOF(mac_roam_trigger_stru),
                                    (oal_uint8 *)&st_roam_trigger);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG2(uc_vap_id, OAM_SF_CFG, "hmac_roam_stop_roam_trigger:send event[%d] failed[%d]", en_val, ul_ret);
        return ul_ret;
    }
    pst_roam_info->uc_rssi_ignore = en_val;

    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_stop_roam_trigger::[%d] OK!}", en_val);
    return OAL_SUCC;
}


oal_uint32 hmac_roam_pause_user(hmac_vap_stru *pst_hmac_vap, oal_void *p_param)
{
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;
    oal_net_device_stru *pst_net_device = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_uint8 uc_vap_id;
    oal_uint8 uc_roaming_mode;

    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_pause_user::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    uc_vap_id = pst_hmac_vap->st_vap_base_info.uc_vap_id;

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_roam_info == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_pause_user::roam info null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 漫游开关没有开时，不暂停用户 */
    if (pst_roam_info->uc_enable == 0) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_pause_user::roam disabled!}");
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    pst_net_device = pst_hmac_vap->pst_net_device;
    if (pst_net_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_pause_user::net_device null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 必须保证vap的状态是UP */
    if (pst_hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_UP) {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_pause_user::vap state = [%d] NOT FOR ROAMING!}",
                         pst_hmac_vap->st_vap_base_info.en_vap_state);
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }

    /* 暂停所有协议层数据，这样就不需要再hmac搞一个缓存队列了 */
    oal_net_tx_stop_all_queues(pst_net_device);
    oal_net_wake_subqueue(pst_net_device, WLAN_HI_QUEUE);

    /* 清空 HMAC层TID信息 */
    hmac_tid_clear(&pst_hmac_vap->st_vap_base_info, pst_roam_info->pst_hmac_user);

#ifdef _PRE_WLAN_FEATURE_STA_PM
    ul_ret = hmac_config_set_pm_by_module(&pst_hmac_vap->st_vap_base_info, MAC_STA_PM_CTRL_TYPE_ROAM,
                                          MAC_STA_PM_SWITCH_OFF);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_CFG, "hmac_roam_pause_user:hmac_config_set_pm_by_module failed[%d]", ul_ret);
        oal_net_tx_wake_all_queues(pst_net_device);
        return ul_ret;
    }
#endif

    hmac_roam_reset_rssi(pst_hmac_vap);

    uc_roaming_mode = 1;
    ul_ret = hmac_config_send_event(&pst_hmac_vap->st_vap_base_info, WLAN_CFGID_SET_ROAMING_MODE,
                                    OAL_SIZEOF(oal_uint8), (oal_uint8 *)&uc_roaming_mode);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_CFG, "hmac_roam_pause_user:send event failed[%d]", ul_ret);
        oal_net_tx_wake_all_queues(pst_net_device);
        return ul_ret;
    }

#ifdef _PRE_WLAN_FEATURE_VOWIFI
    if (pst_hmac_vap->st_vap_base_info.pst_vowifi_cfg_param != OAL_PTR_NULL) {
        if (pst_hmac_vap->st_vap_base_info.pst_vowifi_cfg_param->en_vowifi_mode == VOWIFI_LOW_THRES_REPORT) {
            /* 针对漫游和去关联场景,切换vowifi语音状态 */
            hmac_config_vowifi_report((&pst_hmac_vap->st_vap_base_info), 0, OAL_PTR_NULL);
        }
    }
#endif /* _PRE_WLAN_FEATURE_VOWIFI */
    hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_ROAMING);

    OAM_WARNING_LOG0(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_pause_user::queues stopped!}");
    return OAL_SUCC;
}


oal_uint32 hmac_roam_resume_user(hmac_vap_stru *pst_hmac_vap, oal_void *p_param)
{
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;
    oal_net_device_stru *pst_net_device = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_uint8 uc_vap_id;
    oal_uint8 uc_roaming_mode;
    oal_bool_enum_uint8 need_post_stop_event;

    if ((pst_hmac_vap == OAL_PTR_NULL) || (p_param == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_resume_user::vap or p_param null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    uc_vap_id = pst_hmac_vap->st_vap_base_info.uc_vap_id;

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_roam_info == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_resume_user::roam info null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    pst_net_device = pst_hmac_vap->pst_net_device;
    if (pst_net_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_resume_user::net_device null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 必须保证vap的状态是roaming */
    if (pst_hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_ROAMING) {
        hmac_roam_resume_pm(pst_roam_info, OAL_PTR_NULL);
        hmac_roam_resume_security_port(pst_roam_info, OAL_PTR_NULL);
        oal_net_tx_wake_all_queues(pst_net_device);
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_resume_user::vap state[%d] NOT ROAMING!}",
                         pst_hmac_vap->st_vap_base_info.en_vap_state);
        return OAL_SUCC;
    }

    need_post_stop_event = *(oal_uint8 *)p_param;
    if (need_post_stop_event == OAL_TRUE) {
        uc_roaming_mode = 0;
        ul_ret = hmac_config_send_event(&pst_hmac_vap->st_vap_base_info, WLAN_CFGID_SET_ROAMING_MODE,
                                        OAL_SIZEOF(oal_uint8), (oal_uint8 *)&uc_roaming_mode);
        if (ul_ret != OAL_SUCC) {
            OAM_ERROR_LOG1(uc_vap_id, OAM_SF_CFG, "{hmac_roam_resume_user::send event failed[%d].}", ul_ret);
        }
    }

    hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_UP);

    hmac_roam_resume_pm(pst_roam_info, OAL_PTR_NULL);

    hmac_roam_resume_security_port(pst_roam_info, OAL_PTR_NULL);

    oal_net_tx_wake_all_queues(pst_net_device);

    OAM_WARNING_LOG0(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_resume_user::all_queues awake!}");

    return OAL_SUCC;
}


oal_uint32 hmac_roam_scan_complete(hmac_vap_stru *pst_hmac_vap)
{
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    oal_uint8 uc_vap_id;
    hmac_bss_mgmt_stru *pst_scan_bss_mgmt = OAL_PTR_NULL;

    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_scan_complete::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    uc_vap_id = pst_hmac_vap->st_vap_base_info.uc_vap_id;

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_roam_info == OAL_PTR_NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 漫游开关没有开时，不处理扫描结果 */
    if (pst_roam_info->uc_enable == 0) {
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    /* 获取hmac device */
    pst_hmac_device = hmac_res_get_mac_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_scan_complete::device null!}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    pst_scan_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    OAM_INFO_LOG0(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_scan_complete::handling scan result!}");
    return hmac_roam_main_fsm_action(pst_roam_info, ROAM_MAIN_FSM_EVENT_SCAN_RESULT, (void *)pst_scan_bss_mgmt);
}


oal_void hmac_roam_tbtt_handle(hmac_vap_stru *pst_hmac_vap)
{
}


oal_uint32 hmac_roam_trigger_handle(
    hmac_vap_stru *pst_hmac_vap, oal_int8 c_rssi, oal_bool_enum_uint8 en_current_bss_ignore)
{
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_trigger_handle::pst_hmac_vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_roam_info == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_ROAM, "{hmac_roam_trigger_handle::pst_hmac_vap->pul_roam_info is NULL,Return!}");
        return OAL_ERR_CODE_ROAM_EVENT_UXEXPECT;
    }

    /* 黑名单路由器，不支持漫游，防止漫游出现异常 */
    if ((pst_hmac_vap->en_roam_prohibit_on == OAL_TRUE) && (pst_roam_info->en_roam_trigger != ROAM_TRIGGER_11V)) {
        OAM_WARNING_LOG0(0, OAM_SF_ROAM, "{hmac_roam_trigger_handle::blacklist ap not support roam!}");
        return OAL_FAIL;
    }

    /* 信号桥时不触发漫游 */
    ul_ret = hmac_vap_check_signal_bridge(&pst_hmac_vap->st_vap_base_info);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_roam_info == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_ROAM, "{hmac_roam_trigger_handle_etc::pst_hmac_vap->pul_roam_info is NULL,Return!}");
        return OAL_ERR_CODE_ROAM_EVENT_UXEXPECT;
    }

    /* 每次漫游前，刷新是否支持漫游到自己的参数 */
    pst_roam_info->en_current_bss_ignore = en_current_bss_ignore;
    pst_roam_info->st_config.uc_scan_orthogonal = ROAM_SCAN_CHANNEL_ORG_BUTT;
    pst_roam_info->en_roam_trigger = ROAM_TRIGGER_DMAC;

    /* 判断是否出现漫游乒乓 */
    hmac_roam_ping_pong_check(pst_hmac_vap, pst_roam_info->en_roam_trigger);

    ul_ret = hmac_roam_main_check_state(pst_roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_INIT);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{hmac_roam_trigger_handle::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    hmac_roam_alg_init(pst_roam_info, c_rssi);

    ul_ret = hmac_roam_main_fsm_action(pst_roam_info, ROAM_MAIN_FSM_EVENT_START, OAL_PTR_NULL);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_trigger_handle::START fail[%d].}", ul_ret);
        return ul_ret;
    }
#ifdef _PRE_WLAN_1102A_CHR
    hmac_chr_roam_info_report(pst_roam_info, HMAC_CHR_ROAM_START);
#endif

    /*lint -e571*/
    OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                     "{hmac_roam_trigger_handle::RSSI[%d].}", c_rssi);
    /*lint +e571*/
    return OAL_SUCC;
}


oal_uint32 hmac_sta_roam_rx_mgmt(hmac_vap_stru *pst_hmac_vap, oal_void *p_param)
{
    dmac_wlan_crx_event_stru *pst_crx_event;

    pst_crx_event = (dmac_wlan_crx_event_stru *)p_param;

    hmac_roam_connect_rx_mgmt(pst_hmac_vap, pst_crx_event);
    return OAL_SUCC;
}

oal_void hmac_roam_add_key_done(hmac_vap_stru *pst_hmac_vap)
{
    hmac_roam_connect_key_done(pst_hmac_vap);
}


oal_void hmac_roam_wpas_connect_state_notify(hmac_vap_stru *pst_hmac_vap, wpas_connect_state_enum_uint32 conn_state)
{
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_wpas_connect_state_notify::vap null!}");
        return;
    }

    if (!IS_LEGACY_VAP(&pst_hmac_vap->st_vap_base_info)) {
        return;
    }

    if (pst_hmac_vap->st_vap_base_info.en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return;
    }

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_roam_info == OAL_PTR_NULL) {
        return;
    }

    if (pst_roam_info->ul_connected_state == conn_state) {
        return;
    }

    OAM_WARNING_LOG2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                     "{hmac_roam_wpas_connect_state_notify:: state changed: [%d]-> [%d]}",
                     pst_roam_info->ul_connected_state, conn_state);

    pst_roam_info->ul_connected_state = conn_state;
    if (conn_state == WPAS_CONNECT_STATE_IPADDR_OBTAINED) {
        pst_roam_info->ul_ip_addr_obtained = OAL_TRUE;
    }
    if (conn_state == WPAS_CONNECT_STATE_IPADDR_REMOVED) {
        pst_roam_info->ul_ip_addr_obtained = OAL_FALSE;
    }

    ul_ret = hmac_config_send_event(&pst_hmac_vap->st_vap_base_info,
                                    WLAN_CFGID_ROAM_NOTIFY_STATE,
                                    OAL_SIZEOF(oal_uint32),
                                    (oal_uint8 *)&pst_roam_info->ul_ip_addr_obtained);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                       "{hmac_roam_wpas_connect_state_notify::send event failed[%d].}", ul_ret);
        return;
    }
}

#ifdef _PRE_WLAN_FEATURE_11R

oal_uint32 hmac_roam_reassoc(hmac_vap_stru *pst_hmac_vap)
{
    wlan_mib_Dot11FastBSSTransitionConfigEntry_stru *pst_wlan_mib_ft_cfg;
    hmac_roam_info_stru *pst_roam_info;
    mac_bss_dscr_stru *pst_bss_dscr;
    oal_uint32 ul_ret;
    oal_uint8 uc_vap_id;

    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_reassoc::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    uc_vap_id = pst_hmac_vap->st_vap_base_info.uc_vap_id;

    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_roam_info == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_reassoc::roam info null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 漫游开关没有开时，不处理 */
    if (pst_roam_info->uc_enable == 0) {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_reassoc::roam disabled.}");
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    /* 主状态机为非CONNECTING状态，失败 */
    if (pst_roam_info->en_main_state != ROAM_MAIN_STATE_CONNECTING) {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_reassoc::roam_en_main_state=[%d] not ROAM_MAIN_STATE_CONNECTING, return.}",
                         pst_roam_info->en_main_state);
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }

    /* CONNECT状态机为非WAIT_JOIN状态，失败 */
    if (pst_roam_info->st_connect.en_state != ROAM_CONNECT_STATE_WAIT_ASSOC_COMP) {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_reassoc::connect state[%d] error.}", pst_roam_info->st_connect.en_state);
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }

    pst_wlan_mib_ft_cfg = &pst_hmac_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_fast_bss_trans_cfg;
    if ((pst_wlan_mib_ft_cfg->en_dot11FastBSSTransitionActivated == OAL_TRUE) &&
        (pst_wlan_mib_ft_cfg->en_dot11FTOverDSActivated == OAL_TRUE) &&
        (pst_hmac_vap->bit_11r_over_ds == OAL_TRUE)) {
        pst_bss_dscr = pst_roam_info->st_connect.pst_bss_dscr;
        if (pst_bss_dscr == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ROAM,
                           "{hmac_roam_reassoc::pst_bss_dscr is null.}");
            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
        }

        ul_ret = hmac_fsm_call_func_sta(pst_hmac_vap, HMAC_FSM_INPUT_ROAMING_START, (oal_void *)pst_bss_dscr);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ROAM, "hmac_roam_reassoc:hmac_fsm_call_func_sta_etc fail[%ld]", ul_ret);
            return ul_ret;
        }
    }
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ROAM, "{hmac_roam_reassoc::ft_over_ds=[%d],to send reassoc!}",
                     pst_wlan_mib_ft_cfg->en_dot11FTOverDSActivated);

    ul_ret = hmac_roam_connect_ft_reassoc(pst_hmac_vap);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_reassoc::hmac_roam_connect_process_ft FAIL[%d]!}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_roam_rx_ft_action(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    dmac_wlan_crx_event_stru st_crx_event;

    memset_s(&st_crx_event, OAL_SIZEOF(dmac_wlan_crx_event_stru), 0, OAL_SIZEOF(dmac_wlan_crx_event_stru));
    st_crx_event.pst_netbuf = pst_netbuf;

    hmac_roam_connect_rx_mgmt(pst_hmac_vap, &st_crx_event);
    return OAL_SUCC;
}

#endif  // _PRE_WLAN_FEATURE_11R

oal_void hmac_roam_connect_complete(hmac_vap_stru *pst_hmac_vap, oal_uint32 ul_result)
{
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_roam_connect_complete::vap null!}");
        return;
    }
    pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (pst_roam_info == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_connect_complete::pst_roam_info null!}");
        return;
    }
    /* 漫游开关没有开时，不处理扫描结果 */
    if (pst_roam_info->uc_enable == 0) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_connect_complete::roam disabled!}");
        return;
    }
#ifdef _PRE_WLAN_1102A_CHR
#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    if (pst_roam_info->en_roam_trigger == ROAM_TRIGGER_11V) {
        if (ul_result != OAL_SUCC) {
            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                                 CHR_WIFI_DRV_EVENT_11V_ROAM_FAIL, ul_result);
        }
    }
#endif
#endif
    pst_roam_info->st_static.ul_connect_comp_timetamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    if (ul_result == OAL_SUCC) {
        ul_ret = hmac_roam_main_fsm_action(pst_roam_info, ROAM_MAIN_FSM_EVENT_CONNECT_SUCC, OAL_PTR_NULL);
    } else if (ul_result == OAL_ERR_CODE_ROAM_HANDSHAKE_FAIL) {
        ul_ret = hmac_roam_main_fsm_action(pst_roam_info, ROAM_MAIN_FSM_EVENT_HANDSHAKE_FAIL, OAL_PTR_NULL);
    } else if (ul_result == OAL_ERR_CODE_ROAM_NO_RESPONSE) {
        ul_ret = hmac_roam_main_fsm_action(pst_roam_info, ROAM_MAIN_FSM_EVENT_CONNECT_FAIL, OAL_PTR_NULL);
    } else {
        /* 上层触发停止漫游时，先删除相关定时器 */
        hmac_roam_main_del_timer(pst_roam_info);
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_roam_info->st_connect.st_timer));
        ul_ret = hmac_roam_main_fsm_action(pst_roam_info, ROAM_MAIN_FSM_EVENT_TIMEOUT, OAL_PTR_NULL);
    }
    if (ul_ret != OAL_SUCC) {
    }
}

#ifdef _PRE_WLAN_1102A_CHR
oal_void hmac_chr_roam_info_report(hmac_roam_info_stru *pst_roam_info, roam_fail_reason_enum_uint8 ul_result)
{
    hmac_chr_roam_info_stru st_chr_roam_info = { 0 };
    oal_uint32 ul_scan_dur;
    oal_uint32 ul_connect_dur;

    if (pst_roam_info == OAL_PTR_NULL) {
        return;
    }
    if (pst_roam_info->pst_hmac_vap == OAL_PTR_NULL) {
        return;
    }
    ul_scan_dur =
        OAL_TIME_GET_RUNTIME(pst_roam_info->st_static.ul_scan_timetamp, pst_roam_info->st_static.ul_connect_timetamp);
    ul_connect_dur = OAL_TIME_GET_RUNTIME(pst_roam_info->st_static.ul_connect_timetamp,
                                          pst_roam_info->st_static.ul_connect_comp_timetamp);
    if (ul_result == HMAC_CHR_ROAM_SUCCESS) {
        st_chr_roam_info.uc_scan_time = ul_scan_dur;
        st_chr_roam_info.uc_connect_time = ul_connect_dur;
    } else {
        st_chr_roam_info.uc_scan_time = 0;
        st_chr_roam_info.uc_connect_time = 0;
    }
    st_chr_roam_info.uc_trigger = pst_roam_info->en_roam_trigger;
    st_chr_roam_info.uc_roam_result = ul_result;
    st_chr_roam_info.uc_roam_mode = pst_roam_info->st_static.ul_roam_mode;
    st_chr_roam_info.uc_scan_mode = pst_roam_info->st_static.ul_scan_mode;
    CHR_EXCEPTION_P(CHR_WIFI_ROAM_INFO_REPORT_EVENTID, (oal_uint8 *)(&st_chr_roam_info),
                    OAL_SIZEOF(hmac_chr_roam_info_stru));
    return;
}
#endif


oal_void hmac_roam_timeout_test(hmac_vap_stru *pst_hmac_vap)
{
    hmac_roam_connect_timeout(pst_hmac_vap->pul_roam_info);
}

#endif  // _PRE_WLAN_FEATURE_ROAM

