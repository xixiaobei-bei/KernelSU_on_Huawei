

#include "oam_ext_if.h"
#include "mac_ie.h"
#include "mac_resource.h"
#include "mac_mib.h"
#include "mac_frame_inl.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_mgmt_sta_up.h"
#include "hmac_mgmt_sta.h"
#include "hmac_resource.h"
#include "hmac_p2p.h"
#include "hmac_11i.h"
#include "hmac_11v.h"
#include "hmac_scan.h"
#include "hmac_roam_main.h"
#include "hmac_roam_connect.h"
#include "hmac_roam_alg.h"
#include "hmac_roam_scan.h"
#include "hmac_vowifi.h"
#ifdef _PRE_WLAN_FEATURE_11K
#include "wal_config.h"
#ifdef _PRE_WINDOWS_SUPPORT
#include "wal_windows_ioctl.h"
#else
#include "wal_linux_ioctl.h"
#endif
#endif
#include "hmac_dfx.h"
#include "hisi_customize_wifi.h"
#include "securec.h"
#include "securectype.h"
#include "hmac_11r.h"
#include "hmac_11w.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ROAM_MAIN_C

/* 30s */
#define HMAC_ROAM_HOST_5G_SCAN_INTERVAL 30000
#define HOME_NETWORK_MAC_CHECK_LEN      4

#define hmac_container_of(ptr, type, member) \
    (type *)((char *)(ptr) - (char *) &((type *)0)->member)

OAL_STATIC hmac_roam_homenetwork_thr_stru g_st_roam_homenetwork_thr = {-50, -60, -74};
OAL_STATIC hmac_roam_fsm_func g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_BUTT][ROAM_MAIN_FSM_EVENT_TYPE_BUTT];
OAL_STATIC uint32_t hmac_roam_main_null_fn(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_roam_connect_to_bss(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_roam_to_old_bss(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_roam_to_new_bss(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_roam_connecting_timeout(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_roam_connecting_fail(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_roam_handle_fail_handshake_phase(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC void hmac_roam_show(hmac_vap_stru *hmac_vap);
#ifdef _PRE_WLAN_FEATURE_11K
OAL_STATIC uint32_t hmac_roam_neighbor_response_process(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_wait_neighbor_response_timeout(hmac_roam_info_stru *roam_info, void *param);
#endif

OAL_STATIC void hmac_roam_fsm_init(void)
{
    uint32_t state_idx;
    uint32_t event_idx;

    for (state_idx = 0; state_idx < ROAM_MAIN_STATE_BUTT; state_idx++) {
        for (event_idx = 0; event_idx < ROAM_MAIN_FSM_EVENT_TYPE_BUTT; event_idx++) {
            g_hmac_roam_main_fsm_func[state_idx][event_idx] = hmac_roam_main_null_fn;
        }
    }
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_INIT][ROAM_MAIN_FSM_EVENT_START] = hmac_roam_scan_init;
#ifdef _PRE_WLAN_FEATURE_11K
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_NEIGHBOR_PROCESS][ROAM_MAIN_FSM_EVENT_START] =
        hmac_roam_neighbor_response_process;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_NEIGHBOR_PROCESS][ROAM_MAIN_FSM_EVENT_TIMEOUT] =
        hmac_wait_neighbor_response_timeout;
#endif
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


uint32_t hmac_roam_main_fsm_action(hmac_roam_info_stru *roam_info,
    roam_main_fsm_event_type_enum event, void *param)
{
    if (oal_unlikely(roam_info == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (roam_info->en_main_state >= ROAM_MAIN_STATE_BUTT) {
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }

    if (event >= ROAM_MAIN_FSM_EVENT_TYPE_BUTT) {
        return OAL_ERR_CODE_ROAM_EVENT_UXEXPECT;
    }

    return g_hmac_roam_main_fsm_func[roam_info->en_main_state][event](roam_info, param);
}


void hmac_roam_main_change_state(hmac_roam_info_stru *roam_info, roam_main_state_enum state)
{
    if (roam_info != NULL) {
        oam_warning_log2(0, OAM_SF_ROAM,
                         "{hmac_roam_main_change_state::[%d]->[%d]}", roam_info->en_main_state, state);
        roam_info->en_main_state = state;
    }
}


OAL_STATIC uint32_t hmac_roam_main_timeout(void *arg)
{
    hmac_roam_info_stru *roam_info = NULL;

    if (oal_unlikely(arg == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    roam_info = (hmac_roam_info_stru *)arg;

    oam_warning_log2(0, OAM_SF_ROAM, "{hmac_roam_main_timeout::MAIN_STATE[%d] CONNECT_STATE[%d].}",
                     roam_info->en_main_state, roam_info->st_connect.en_state);

    return hmac_roam_main_fsm_action(roam_info, ROAM_MAIN_FSM_EVENT_TIMEOUT, NULL);
}

void hmac_roam_check_psk(hmac_vap_stru *hmac_vap, mac_conn_security_stru *conn_sec)
{
    uint32_t akm_suite = oal_ntoh_32(conn_sec->st_crypto.aul_akm_suite[0]);
    uint8_t akm_type = akm_suite & 0xFF;

    if (akm_type == WLAN_AUTH_SUITE_PSK ||
        akm_type == WLAN_AUTH_SUITE_PSK_SHA256 ||
        akm_type == WLAN_AUTH_SUITE_FT_PSK) {
        hmac_vap->en_is_psk = OAL_TRUE;
    } else {
        hmac_vap->en_is_psk = OAL_FALSE;
    }
    return;
}


OAL_STATIC void hmac_roam_check_home_network(hmac_vap_stru *hmac_vap, hmac_roam_info_stru *roam_info)
{
    if (roam_info->st_config.uc_scenario_enable == OAL_FALSE) {
        return;
    }

    roam_info->uc_roaming_scenario = ROAMING_SCENARIO_BUTT;

    /* 家庭网络场景识别 */
    if (roam_info->st_alg.st_home_network.st_2g_bssid.uc_bssid_num == 1 &&
        roam_info->st_alg.st_home_network.st_5g_bssid.uc_bssid_num == 1 &&
        hmac_vap->en_is_psk == OAL_TRUE) {
        if (oal_memcmp(roam_info->st_alg.st_home_network.st_2g_bssid.auc_bssid[0],
                       roam_info->st_alg.st_home_network.st_5g_bssid.auc_bssid[0],
                       HOME_NETWORK_MAC_CHECK_LEN) == 0) {
            roam_info->st_alg.st_home_network.uc_is_homenetwork = OAL_TRUE;
            roam_info->uc_roaming_scenario = ROAMING_SCENARIO_HOME;
            return;
        }
    }

    roam_info->st_alg.st_home_network.uc_is_homenetwork = OAL_FALSE;
    return;
}


OAL_STATIC void hmac_roam_home_network_init(hmac_vap_stru *hmac_vap, hmac_roam_info_stru *roam_info)
{
    hmac_device_stru *hmac_device = NULL;
    hmac_bss_mgmt_stru *scan_bss_mgmt = NULL;
    oal_dlist_head_stru *entry = NULL;
    hmac_scanned_bss_info *scanned_bss = NULL;
    mac_bss_dscr_stru *bss_dscr = NULL;

    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_home_network_init::device is null}");
        return;
    }

    roam_info->st_alg.st_home_network.uc_is_homenetwork = OAL_FALSE;
    roam_info->st_alg.st_home_network.c_trigger_rssi = g_st_roam_homenetwork_thr.c_trigger_rssi;
    roam_info->st_alg.st_home_network.c_5g_rssi_th = g_st_roam_homenetwork_thr.c_5g_rssi_th;
    roam_info->st_alg.st_home_network.c_5g_hold_th = g_st_roam_homenetwork_thr.c_5g_hold_th;
    roam_info->st_alg.st_home_network.uc_max_retry_cnt = 6; /* 每个rssi最多触发6次漫游 */
    roam_info->st_alg.st_home_network.scan_timestamp_5g = oal_time_get_stamp_ms();

    scan_bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    oal_spin_lock(&(scan_bss_mgmt->st_lock));

    /* 遍历扫描到的bss信息，识别家庭网络漫游环境 */
    oal_dlist_search_for_each(entry, &(scan_bss_mgmt->st_bss_list_head))
    {
        scanned_bss = oal_dlist_get_entry(entry, hmac_scanned_bss_info, st_dlist_head);
        bss_dscr = &(scanned_bss->st_bss_dscr_info);

        hmac_roam_alg_bss_in_ess(roam_info, bss_dscr);

        bss_dscr = NULL;
    }

    oal_spin_unlock(&(scan_bss_mgmt->st_lock));

    hmac_roam_check_home_network(hmac_vap, roam_info);

    if (roam_info->uc_roaming_scenario == ROAMING_SCENARIO_HOME) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "hmac_roam_home_network_init::home network roaming scenario");
    }

    return;
}

OAL_STATIC void hmac_roam_init_roam_config_info(hmac_vap_stru *hmac_vap, hmac_roam_info_stru *roam_info)
{
    roam_info->st_config.uc_scan_orthogonal = g_wlan_cust.uc_roam_scan_orthogonal;
    roam_info->st_config.c_trigger_rssi_2g = g_wlan_cust.c_roam_trigger_b;
    roam_info->st_config.c_trigger_rssi_5g = g_wlan_cust.c_roam_trigger_a;
    roam_info->st_config.c_dense_env_trigger_rssi_2g = g_wlan_cust.c_dense_env_roam_trigger_b;
    roam_info->st_config.c_dense_env_trigger_rssi_5g = g_wlan_cust.c_dense_env_roam_trigger_a;
    roam_info->st_config.uc_delta_rssi_2g = g_wlan_cust.c_roam_delta_b;
    roam_info->st_config.uc_delta_rssi_5g = g_wlan_cust.c_roam_delta_a;
    roam_info->st_config.uc_scenario_enable = g_wlan_cust.uc_scenario_enable;
    roam_info->st_config.c_candidate_good_rssi = g_wlan_cust.c_candidate_good_rssi;
    roam_info->st_config.uc_candidate_good_num = g_wlan_cust.uc_candidate_good_num;
    roam_info->st_config.uc_candidate_weak_num = g_wlan_cust.uc_candidate_weak_num;
    roam_info->st_config.us_roam_interval = g_wlan_cust.us_roam_interval;
    roam_info->st_config.c_trigger_th1_2g = ROAM_ENV_RSSI_THRESHOLD1_2G;
    roam_info->st_config.c_trigger_th1_5g = ROAM_ENV_RSSI_THRESHOLD1_5G;
}

uint32_t hmac_roam_init(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru *roam_info = NULL;
    uint32_t i = 0;

    if (oal_unlikely(hmac_vap == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (!IS_LEGACY_VAP(&(hmac_vap->st_vap_base_info))) {
        return OAL_SUCC;
    }

    if (hmac_vap->pul_roam_info == NULL) {
        /* 漫游主结构体内存申请 */
        hmac_vap->pul_roam_info =
            (uint32_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_roam_info_stru), OAL_TRUE);
        if (hmac_vap->pul_roam_info == NULL) {
            oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                           "{hmac_roam_init::oal_mem_alloc_m fail.}");
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    } else {
        hmac_roam_main_del_timer((hmac_roam_info_stru *)hmac_vap->pul_roam_info);
#endif
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;

    memset_s(hmac_vap->pul_roam_info, sizeof(hmac_roam_info_stru),
             0, sizeof(hmac_roam_info_stru));
    roam_info->uc_enable = g_wlan_cust.uc_roam_switch;
    roam_info->en_roam_trigger = ROAM_TRIGGER_DMAC;
    roam_info->en_main_state = ROAM_MAIN_STATE_INIT;
    roam_info->en_current_bss_ignore = OAL_TRUE;
    roam_info->pst_hmac_vap = hmac_vap;
    roam_info->pst_hmac_user = NULL;
    roam_info->connected_state = WPAS_CONNECT_STATE_INIT;
    roam_info->uc_roaming_scenario = ROAMING_SCENARIO_BUTT;
#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    oal_init_work(&(roam_info->hmac_11v_roam_work), hmac_11v_roam_start_worker);
#endif

    hmac_roam_init_roam_config_info(hmac_vap, roam_info);

    for (i = 0; i < ROAM_LIST_MAX; i++) {
        roam_info->st_alg.st_history.ast_bss[i].us_count_limit = ROAM_HISTORY_COUNT_LIMIT;
        roam_info->st_alg.st_history.ast_bss[i].timeout = ROAM_HISTORY_BSS_TIME_OUT;
        roam_info->st_alg.st_blacklist.ast_bss[i].us_count_limit = ROAM_BLACKLIST_COUNT_LIMIT;
        roam_info->st_alg.st_blacklist.ast_bss[i].timeout = ROAM_BLACKLIST_NORMAL_AP_TIME_OUT;
    }

    hmac_roam_fsm_init();
    hmac_roam_connect_fsm_init();

    hmac_roam_home_network_init(hmac_vap, roam_info);

    oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM, "{hmac_roam_init::SUCC.}");
    return OAL_SUCC;
}



uint32_t hmac_roam_info_init(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru *roam_info = NULL;

    if (oal_unlikely(hmac_vap == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap->pul_roam_info == NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    roam_info->en_main_state = ROAM_MAIN_STATE_INIT;
    roam_info->uc_rssi_type = ROAM_ENV_BUTT;
    roam_info->uc_invalid_scan_cnt = 0;
    roam_info->st_connect.en_state = ROAM_CONNECT_STATE_INIT;
    roam_info->st_alg.max_capacity = 0;
    roam_info->st_alg.pst_max_capacity_bss = NULL;
    roam_info->st_alg.c_current_rssi = 0;
    roam_info->st_alg.c_max_rssi = 0;
    roam_info->st_alg.uc_another_bss_scaned = 0;
    roam_info->st_alg.uc_invalid_scan_cnt = 0;
    roam_info->st_alg.uc_candidate_bss_num = 0;
    roam_info->st_alg.uc_candidate_good_rssi_num = 0;
    roam_info->st_alg.uc_candidate_weak_rssi_num = 0;
    roam_info->st_alg.uc_scan_period = 0;
    roam_info->st_alg.pst_max_rssi_bss = NULL;
    roam_info->pst_hmac_user = mac_res_get_hmac_user(hmac_vap->st_vap_base_info.us_assoc_vap_id);
    if (roam_info->pst_hmac_user == NULL) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_info_init::assoc_vap_id[%d] can't found.}",
                       hmac_vap->st_vap_base_info.us_assoc_vap_id);
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 漫游双门限方案只针对WPA/WPA2/WPA3 Personal加密方式有效 */
    /* 关闭漫游双门限方案for STA+P2P scenario */
    hmac_roam_rssi_trigger_type(hmac_vap, ROAM_ENV_SPARSE_AP_TH2);

    oam_info_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM, "{hmac_roam_info_init::SUCC.}");
    return OAL_SUCC;
}


uint32_t hmac_roam_exit(hmac_vap_stru *hmac_vap)
{
    if (oal_unlikely(hmac_vap == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (!IS_LEGACY_VAP(&(hmac_vap->st_vap_base_info))) {
        return OAL_SUCC;
    }

    if (hmac_vap->pul_roam_info == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_exit::pul_roam_info is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_roam_show(hmac_vap);

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    oal_cancel_work_sync(&(((hmac_roam_info_stru *)hmac_vap->pul_roam_info)->hmac_11v_roam_work));
#endif

    frw_timer_immediate_destroy_timer_m(&(((hmac_roam_info_stru *)hmac_vap->pul_roam_info)->st_connect.st_timer));
    hmac_roam_main_del_timer((hmac_roam_info_stru *)hmac_vap->pul_roam_info);
    if (hmac_vap->pst_net_device != NULL) {
        oal_net_tx_wake_all_queues(hmac_vap->pst_net_device);
    }
    oal_mem_free_m(hmac_vap->pul_roam_info, OAL_TRUE);
    hmac_vap->pul_roam_info = NULL;

    oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM, "{hmac_roam_exit::SUCC.}");

    return OAL_SUCC;
}


uint32_t hmac_roam_show_info(hmac_vap_stru *hmac_vap)
{
    int8_t *print_buff = NULL;
    hmac_roam_info_stru *roam_info = NULL;

    if (oal_unlikely(hmac_vap == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (hmac_vap->pul_roam_info == NULL) {
        return OAL_SUCC;
    }

    print_buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (print_buff == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_vap_info::pc_print_buff null.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    memset_s(print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
    snprintf_s(print_buff, OAM_REPORT_MAX_STRING_LEN, OAM_REPORT_MAX_STRING_LEN - 1,
               "ROAM_EN[%d] MAIN_STATE[%d]\n"
               "ROAM_SCAN_ORTH[%d]\n",
               roam_info->uc_enable, roam_info->en_main_state,
               roam_info->st_config.uc_scan_orthogonal);

    oam_print(print_buff);

    oal_mem_free_m(print_buff, OAL_TRUE);

    return OAL_SUCC;
}


void hmac_roam_main_start_timer(hmac_roam_info_stru *roam_info, uint32_t timeout)
{
    frw_timeout_stru *timer = &(roam_info->st_timer);

    oam_info_log1(0, OAM_SF_ROAM, "{hmac_roam_main_start_timer [%d].}", timeout);

    /* 启动认证超时定时器 */
    frw_timer_create_timer_m(timer, hmac_roam_main_timeout, timeout, roam_info,
                             OAL_FALSE, OAM_MODULE_ID_HMAC,
                             roam_info->pst_hmac_vap->st_vap_base_info.core_id);
}


void hmac_roam_main_del_timer(hmac_roam_info_stru *roam_info)
{
    oam_info_log0(0, OAM_SF_ROAM, "{hmac_roam_main_del_timer.}");
    frw_timer_immediate_destroy_timer_m(&(roam_info->st_timer));
}


uint32_t hmac_roam_enable(hmac_vap_stru *hmac_vap, uint8_t enable)
{
    hmac_roam_info_stru *roam_info = NULL;

    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_enable::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_enable::roam_info null .}");
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    if (enable == roam_info->uc_enable) {
        oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_enable::SET[%d] fail. roam was[%d]}", enable, roam_info->uc_enable);
        return OAL_FAIL;
    }

    /* 设置漫游开关 */
    roam_info->uc_enable = enable;
    roam_info->en_main_state = ROAM_MAIN_STATE_INIT;
    oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                     "{hmac_roam_enable::SET[%d] OK!}", enable);

    return OAL_SUCC;
}


uint32_t hmac_roam_check_signal_bridge(hmac_vap_stru *hmac_vap)
{
    mac_vap_stru *mac_vap = NULL;
    mac_vap_stru *other_vap = NULL;

    mac_vap = &hmac_vap->st_vap_base_info;

    /* check all vap state in case other vap is signal bridge GO mode */
    other_vap = mac_vap_find_another_up_vap_by_mac_vap(mac_vap);
    if (other_vap == NULL) {
        return OAL_SUCC;
    }

    /* 终端需求: 打开信号桥，禁止漫游 */
    if (IS_BRIDGE_VAP(other_vap)) {
        /* 如果是P2P GO模式且Beacon帧不包含P2P ie即为信号桥 */
        oam_warning_log0(other_vap->uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_check_signal_bridge::vap is GO Signal Bridge, disable STA roaming!}");
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    return OAL_SUCC;
}


OAL_STATIC void hmac_roam_param_update(hmac_roam_info_stru *roam_info,
    roam_channel_org_enum scan_type, oal_bool_enum_uint8 current_bss_ignore,
    roam_trigger_enum roam_trigger, uint8_t *target_bssid)
{
    roam_info->st_config.uc_scan_orthogonal = scan_type;
    roam_info->en_current_bss_ignore = current_bss_ignore; /* false表示漫游到自己 */
    roam_info->en_roam_trigger = roam_trigger;
#ifdef _PRE_WLAN_FEATURE_11R
    roam_info->st_connect.uc_ft_force_air = OAL_TRUE;
    roam_info->st_connect.uc_ft_failed = OAL_FALSE;
#endif
    if (target_bssid != NULL) {
        oal_set_mac_addr(roam_info->auc_target_bssid, target_bssid);
    }
}
OAL_STATIC uint32_t hmac_roam_set_scan_param(uint8_t vap_id, roam_channel_org_enum scan_type,
                                             hmac_roam_info_stru *roam_info)
{
    uint32_t ret = OAL_SUCC;
    /* 触发漫游是否搭配扫描0表示不扫描 */
    if (scan_type == ROAM_SCAN_CHANNEL_ORG_0) {
        /* 对于不进行扫描的漫游流程,更新扫描/关联时间戳 */
        roam_info->st_static.scan_start_timetamp = (uint32_t)oal_time_get_stamp_ms();
        hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_SCANING);

        ret = hmac_roam_main_fsm_action(roam_info, ROAM_MAIN_FSM_EVENT_TIMEOUT, NULL);
        if (ret != OAL_SUCC) {
            oam_error_log1(vap_id, OAM_SF_ROAM, "{hmac_roam_start::START fail[%d].}", ret);
            return ret;
        }
    } else {
        ret = hmac_roam_main_fsm_action(roam_info, ROAM_MAIN_FSM_EVENT_START, NULL);
        if (ret != OAL_SUCC) {
            oam_error_log1(vap_id, OAM_SF_ROAM, "{hmac_roam_start::START fail[%d].}", ret);
            return ret;
        }
    }
    hmac_chr_roam_info_report(roam_info, HMAC_CHR_ROAM_START);
    return ret;
}


uint32_t hmac_roam_start(hmac_vap_stru *hmac_vap, roam_channel_org_enum scan_type,
                         oal_bool_enum_uint8 current_bss_ignore, uint8_t *target_bssid,
                         roam_trigger_enum roam_trigger)
{
    uint32_t ret;
    hmac_roam_info_stru *roam_info = NULL;

    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_start::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (current_bss_ignore == OAL_TRUE) {
        ret = hmac_roam_check_signal_bridge(hmac_vap);
        if (ret != OAL_SUCC) {
            return ret;
        }

        /* 非漫游到自己，黑名单路由器，不支持漫游，防止漫游出现异常 */
        if (hmac_vap->en_roam_prohibit_on == OAL_TRUE) {
            oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_start::blacklist ap not support roam!}");
            return OAL_FAIL;
        }
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_INIT);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_start::check_state fail[%d]!}", ret);
        return ret;
    }

#ifdef _PRE_WLAN_FEATURE_MBO
    if (hmac_vap->st_vap_base_info.st_mbo_para_info.uc_mbo_enable != OAL_TRUE) {
#endif
        if (hmac_vap->ipaddr_obtained == OAL_FALSE) {
            oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_start::ip addr not obtained!}");
            return OAL_ERR_CODE_ROAM_INVALID_VAP_STATUS;
        }
#ifdef _PRE_WLAN_FEATURE_MBO
    }
#endif

    /* 每次漫游前，刷新是否支持漫游到自己的参数 */
    hmac_roam_param_update(roam_info, scan_type, current_bss_ignore, roam_trigger, target_bssid);

    oam_warning_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                     "{hmac_roam_start::START succ, scan_type=%d, current_bss_ignore=%d roam_trigger=%d.}",
                     scan_type, current_bss_ignore, roam_trigger);

    hmac_roam_alg_init_rssi(hmac_vap, roam_info);

    return hmac_roam_set_scan_param(hmac_vap->st_vap_base_info.uc_vap_id, scan_type, roam_info);
}


uint32_t hmac_roam_handle_home_network(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru *roam_info = NULL;
    uint32_t current_timestamp;

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL || roam_info->uc_enable == 0) {
        return OAL_SUCC;
    }

    hmac_roam_check_home_network(hmac_vap, roam_info);

    /* 漫游场景识别5: 家庭双频路由器，关联RSSI>=-50dBm 2G AP，可以漫游到5G AP，5G RSSI >= -60dB */
    if ((roam_info->st_config.uc_scenario_enable == OAL_TRUE) &&
        (roam_info->uc_roaming_scenario == ROAMING_SCENARIO_HOME) &&
        (roam_info->st_alg.st_home_network.uc_is_homenetwork == OAL_TRUE) &&
        (hmac_vap->st_vap_base_info.st_channel.en_band == WLAN_BAND_2G) &&
        (hmac_vap->station_info.signal >=
        // 每漫游失败一次，漫游触发门限提高5dbm，提高漫游成功率
        (roam_info->st_alg.st_home_network.c_trigger_rssi + roam_info->st_alg.uc_roam_fail_cnt * 5))) {
        current_timestamp = oal_time_get_stamp_ms();
        if (oal_time_after32(current_timestamp,
            (roam_info->st_alg.st_home_network.scan_timestamp_5g + HMAC_ROAM_HOST_5G_SCAN_INTERVAL))) {
            oam_warning_log4(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                "{hmac_roam_handle_home_network::roam_to home network env:%d sign:%d max_rssi:%d current_rssi:%d}",
                roam_info->st_alg.st_home_network.uc_is_homenetwork, hmac_vap->station_info.signal,
                roam_info->st_alg.c_max_rssi, roam_info->st_alg.c_current_rssi);
            roam_info->st_alg.st_home_network.scan_timestamp_5g = current_timestamp;
            return hmac_roam_start(hmac_vap, ROAM_SCAN_CHANNEL_ORG_1,
                                   OAL_TRUE, NULL, ROAM_TRIGGER_HOME_NETWORK);
        }
    }

    return OAL_SUCC;
}


OAL_STATIC void hmac_roam_show(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru *roam_info = NULL;
    hmac_roam_static_stru *roam_static = NULL;
    hmac_roam_config_stru *config_info = NULL;
    uint8_t vap_id;

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    roam_static = &roam_info->st_static;
    config_info = &(roam_info->st_config);

    vap_id = hmac_vap->st_vap_base_info.uc_vap_id;

    if (roam_info->uc_rssi_type == ROAM_ENV_SPARSE_AP_TH1) {
        oam_warning_log3(vap_id, OAM_SF_ROAM,
            "{hmac_roam_show::scan_orthogonal[%d], c_trigger_th1_2g=%d c_trigger_th1_5g=%d}",
            config_info->uc_scan_orthogonal,
            config_info->c_trigger_th1_2g, config_info->c_trigger_th1_5g);
    } else if (roam_info->uc_rssi_type == ROAM_ENV_SPARSE_AP_TH2) {
        oam_warning_log3(vap_id, OAM_SF_ROAM,
            "{hmac_roam_show::scan_orthogonal[%d], c_trigger_rssi_2g=%d c_trigger_rssi_5g=%d}",
            config_info->uc_scan_orthogonal,
            config_info->c_trigger_rssi_2g, config_info->c_trigger_rssi_5g);
    } else if (roam_info->uc_rssi_type == ROAM_ENV_DENSE_AP) {
        oam_warning_log3(vap_id, OAM_SF_ROAM,
            "{hmac_roam_show::scan_orthogonal[%d], c_trigger_rssi_2G=%d c_trigger_rssi_5g=%d.}",
            config_info->uc_scan_orthogonal,
            config_info->c_dense_env_trigger_rssi_2g, config_info->c_dense_env_trigger_rssi_5g);
    } else {
        oam_warning_log2(vap_id, OAM_SF_ROAM,
            "{hmac_roam_show::scan_orthogonal[%d] rssi_type[%d]}",
            config_info->uc_scan_orthogonal, roam_info->uc_rssi_type);
    }

    oam_warning_log4(vap_id, OAM_SF_ROAM,
        "{hmac_roam_show::trigger_rssi_cnt[%u] trigger_linkloss_cnt[%u], scan_cnt[%u] scan_result_cnt[%u].}",
        roam_static->trigger_rssi_cnt, roam_static->trigger_linkloss_cnt,
        roam_static->scan_cnt, roam_static->scan_result_cnt);
    oam_warning_log3(vap_id, OAM_SF_ROAM,
                     "{hmac_roam_show::total_cnt[%u] roam_success_cnt[%u] roam_fail_cnt[%u].}",
                     roam_static->connect_cnt, roam_static->roam_new_cnt, roam_static->roam_old_cnt);
#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    oam_warning_log3(vap_id, OAM_SF_ROAM,
                     "{hmac_roam_show::roam_to, scan fail num=%d, eap fail num=%d, %d(11v)}",
                     roam_info->st_static.roam_scan_fail, roam_info->st_static.roam_eap_fail,
                     roam_info->st_static.roam_11v_scan_fail);
#else
    oam_warning_log2(vap_id, OAM_SF_ROAM, "{hmac_roam_show::roam_to, scan fail=%d, eap fail=%d}",
                     roam_info->st_static.roam_scan_fail, roam_info->st_static.roam_eap_fail);
#endif

    return;
}

OAL_STATIC uint32_t hmac_roam_main_null_fn(hmac_roam_info_stru *roam_info, void *param)
{
    oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_main_null_fn .}");

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11K

oal_bool_enum_uint8 hmac_roam_is_neighbor_report_allowed(hmac_vap_stru *hmac_vap)
{
    mac_bss_dscr_stru *curr_bss_dscr = NULL;
    hmac_roam_info_stru *roam_info = NULL;
    uint8_t *cowork_ie = NULL;
    uint8_t *frame_body = NULL;
    uint16_t frame_body_len;
    uint16_t offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    oal_sta_ap_cowork_ie_beacon *cowork_ie_beacon = NULL;

    curr_bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(&hmac_vap->st_vap_base_info,
        hmac_vap->st_vap_base_info.auc_bssid);
    if (curr_bss_dscr == NULL || curr_bss_dscr->en_support_neighbor != OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_is_neighbor_report_allowed:: disallowed neighbor report}");
        return OAL_FALSE;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        return OAL_FALSE;
    }

    /* c_current_rssi < -85 不做neighbor */
    if (roam_info->is_roaming_trigged_by_cmd != OAL_TRUE &&
        roam_info->st_alg.c_current_rssi < ROAM_RSSI_NE85_DB) {
        return OAL_FALSE;
    }

    frame_body = curr_bss_dscr->auc_mgmt_buff + MAC_80211_FRAME_LEN;
    frame_body_len = curr_bss_dscr->mgmt_len - MAC_80211_FRAME_LEN;
    cowork_ie = mac_find_vendor_ie(MAC_WLAN_OUI_HUAWEI,
                                   MAC_WLAN_OUI_TYPE_HAUWEI_COWORK,
                                   frame_body + offset,
                                   frame_body_len - offset);
    if (cowork_ie == NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_is_neighbor_report_allowed::cowork ie not found}");
        return OAL_TRUE;
    }

    cowork_ie_beacon = (oal_sta_ap_cowork_ie_beacon *)cowork_ie;

    if (cowork_ie_beacon->capa_bitmap.support_neighbor_report != 1) {
        oam_warning_log0(0, OAM_SF_ROAM,
                         "{hmac_roam_is_neighbor_report_allowed::support_neighbor_report in cowork ie is disabled}");
        return OAL_FALSE;
    } else {
        return OAL_TRUE;
    }
}
#endif

OAL_STATIC uint32_t hmac_roam_connecting_timeout(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;
    oal_bool_enum_uint8 post_dmac = OAL_TRUE;

    ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_connecting_timeout::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_chr_roam_info_report(roam_info, HMAC_CHR_ROAM_TIMEOUT_FAIL);

    roam_info->st_connect.uc_roam_main_fsm_state = ROAM_MAIN_FSM_EVENT_TIMEOUT;
    ret = hmac_roam_to_old_bss(roam_info, param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM,
                         "{hmac_roam_connecting_timeout:: hmac_roam_to_old_bss fail[%d]!}", ret);
    }

    /* 握手失败不需要恢复device漫游状态 */
    if (roam_info->st_connect.en_state == ROAM_CONNECT_STATE_HANDSHAKING) {
        post_dmac = OAL_FALSE;
    }

    /* 切换vap的状态为UP，恢复用户节能，恢复发送 */
    ret = hmac_fsm_call_func_sta(roam_info->pst_hmac_vap, HMAC_FSM_INPUT_ROAMING_STOP,
        (void *)&post_dmac);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM,
                       "{hmac_roam_connecting_timeout:: hmac_fsm_call_func_sta fail[%d]!}", ret);
    }

    hmac_roam_main_clear(roam_info);

    return ret;
}


OAL_STATIC uint32_t hmac_roam_connecting_fail(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;
    oal_bool_enum_uint8 is_protected;
    hmac_disconnect disconnect;

    ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_connecting_fail::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_chr_roam_info_report(roam_info, HMAC_CHR_ROAM_CONNECT_FAIL);

    roam_info->st_connect.uc_roam_main_fsm_state = ROAM_MAIN_FSM_EVENT_CONNECT_FAIL;
    hmac_roam_main_del_timer(roam_info);
    ret = hmac_roam_connecting_timeout(roam_info, param);
    if (ret) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_connecting_fail::hmac_roam_to_old_bss fail ret=[%d]!}", ret);
        return ret;
    }

    /* 为了提高漫游成功的概率，在Auth/Reassoc No rsp时立即触发重新漫游 */
    if (roam_info->st_alg.c_current_rssi > ROAM_RSSI_MAX_TYPE) {
        /* 如果是弱信号触发的漫游，先把rssi修改成ROAM_RSSI_LINKLOSS_TYPE来将弱信号跟LINKLOSS触发的重漫游归一 */
        if (roam_info->st_alg.c_current_rssi > ROAM_RSSI_LINKLOSS_TYPE) {
            roam_info->st_alg.c_current_rssi = ROAM_RSSI_LINKLOSS_TYPE;
        }
        /* 漫游失败时，rssi 逐次减1dBm，一直到到ROAM_RSSI_MAX_TYPE。这样可以最多触发5次重漫游 */
        return hmac_roam_trigger_handle(roam_info->pst_hmac_vap, roam_info->st_alg.c_current_rssi - 1, OAL_TRUE);
    }

    
    if (roam_info->st_connect.en_status_code == MAC_REJECT_TEMP) {
        return ret;
    }

    /* 管理帧加密是否开启 */
    is_protected = roam_info->pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active;

    /* 发去关联帧 */
    hmac_mgmt_send_disassoc_frame(&(roam_info->pst_hmac_vap->st_vap_base_info),
                                  roam_info->pst_hmac_user->st_user_base_info.auc_user_mac_addr,
                                  MAC_DEAUTH_LV_SS, is_protected);

    /* 删除对应用户 */
    hmac_user_del(&(roam_info->pst_hmac_vap->st_vap_base_info), roam_info->pst_hmac_user);

    disconnect.reason_code = MAC_NOT_ASSOCED;
    disconnect.type = WLAN_FC0_SUBTYPE_DISASSOC;
    disconnect.is_roam = OAL_TRUE;
    hmac_sta_handle_disassoc_rsp(roam_info->pst_hmac_vap, &disconnect);

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
                         CHR_LAYER_DRV, CHR_WIFI_DEV_EVENT_CHIP, ROAM_FAIL_FIVE_TIMES);

    return ret;
}


OAL_STATIC uint32_t hmac_roam_handle_fail_handshake_phase(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;
    oal_bool_enum_uint8 is_protected;
    oal_bool_enum_uint8 post_dmac = OAL_FALSE; /* 握手失败不需要恢复device漫游状态 */
    hmac_disconnect disconnect;

    ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_handle_fail_handshake_phase::check_state fail[%d]!}", ret);
        return ret;
    }
    roam_info->st_connect.uc_roam_main_fsm_state = ROAM_MAIN_FSM_EVENT_HANDSHAKE_FAIL;

    hmac_chr_roam_info_report(roam_info, HMAC_CHR_ROAM_HANDSHAKE_FAIL);

    /* 切换vap的状态为UP，恢复用户节能，恢复发送 */
    ret = hmac_fsm_call_func_sta(roam_info->pst_hmac_vap, HMAC_FSM_INPUT_ROAMING_STOP, &post_dmac);
    if (ret != OAL_SUCC) {
        oam_error_log1(roam_info->pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_handle_fail_handshake_phase:: hmac_fsm_call_func_sta fail[%d]!}", ret);
    }

    hmac_roam_main_clear(roam_info);
    hmac_roam_main_del_timer(roam_info);

    /* 为提高漫游成功的概率，在握手失败时触发立即重新漫游 */
    if (roam_info->st_alg.c_current_rssi > ROAM_RSSI_MAX_TYPE) {
        /* 如果是弱信号触发的漫游，先把rssi修改成ROAM_RSSI_LINKLOSS_TYPE来将弱信号跟LINKLOSS触发的重漫游归一 */
        if (roam_info->st_alg.c_current_rssi > ROAM_RSSI_LINKLOSS_TYPE) {
            roam_info->st_alg.c_current_rssi = ROAM_RSSI_LINKLOSS_TYPE;
        }
        /* 漫游握手失败时，rssi 逐次减1dBm，一直到到ROAM_RSSI_MAX_TYPE。这样可以最多触发5次重漫游 */
        return hmac_roam_trigger_handle(roam_info->pst_hmac_vap,
                                        roam_info->st_alg.c_current_rssi - 1, OAL_TRUE);
    }

    oam_warning_log1(0, OAM_SF_ROAM,
                     "{hmac_roam_handle_fail_handshake_phase:: report deauth to wpas! c_current_rssi=%d}",
                     roam_info->st_alg.c_current_rssi);

    /* 管理帧加密是否开启 */
    is_protected = roam_info->pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active;

    /* 发去关联帧 */
    hmac_mgmt_send_disassoc_frame(&(roam_info->pst_hmac_vap->st_vap_base_info),
                                  roam_info->pst_hmac_user->st_user_base_info.auc_user_mac_addr,
                                  MAC_DEAUTH_LV_SS, is_protected);

    /* 删除对应用户 */
    hmac_user_del(&(roam_info->pst_hmac_vap->st_vap_base_info), roam_info->pst_hmac_user);

    disconnect.reason_code = MAC_4WAY_HANDSHAKE_TIMEOUT;
    disconnect.type = WLAN_FC0_SUBTYPE_DISASSOC;
    disconnect.is_roam = OAL_TRUE;
    hmac_sta_handle_disassoc_rsp(roam_info->pst_hmac_vap, &disconnect);

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
                         CHR_LAYER_DRV, CHR_WIFI_DEV_EVENT_CHIP, ROAM_FAIL_FIVE_TIMES);

    return ret;
}

#ifdef _PRE_WLAN_FEATURE_ADAPTIVE11R

OAL_STATIC oal_bool_enum_uint8 hmac_roam_is_over_ds_allowed(hmac_roam_info_stru *roam_info)
{
    hmac_vap_stru *hmac_vap = roam_info->pst_hmac_vap;
    mac_bss_dscr_stru *curr_bss_dscr;
    uint8_t *puc_cowork_ie;
    uint8_t *puc_frame_body;
    uint16_t us_frame_body_len;
    uint16_t us_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    oal_sta_ap_cowork_ie_beacon *cowork_ie;

    /* 检查是否满足over ds的两个条件: 1. IsWithCoworkIE 2. current_AP_RSSI>-80 3. ft_failed不为ture */
    curr_bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(&hmac_vap->st_vap_base_info,
        hmac_vap->st_vap_base_info.auc_bssid);
    if (curr_bss_dscr == NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_is_over_ds_allowed::curr_bss_dscr is null}");
        return OAL_FALSE;
    }

    if (curr_bss_dscr->c_rssi < -80) { /* -80dbm */
        oam_warning_log1(0, OAM_SF_ROAM,
                         "{hmac_roam_is_over_ds_allowed::current rssi = [%d] is lower than -80 dbm}",
                         curr_bss_dscr->c_rssi);
        return OAL_FALSE;
    }

    if (roam_info->st_connect.uc_ft_failed == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_is_over_ds_allowed::11r over ds has been failed yet}");
        return OAL_FALSE;
    }

    puc_frame_body = curr_bss_dscr->auc_mgmt_buff + MAC_80211_FRAME_LEN;
    us_frame_body_len = curr_bss_dscr->mgmt_len - MAC_80211_FRAME_LEN;
    puc_cowork_ie = mac_find_vendor_ie(MAC_WLAN_OUI_HUAWEI,
                                       MAC_WLAN_OUI_TYPE_HAUWEI_COWORK,
                                       puc_frame_body + us_offset,
                                       us_frame_body_len - us_offset);
    if (puc_cowork_ie == NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_is_over_ds_allowed::cowork ie not found}");
        return OAL_FALSE;
    }

    /* 目前AP侧不会携带dc_roaming IE，sizeof的时候需要减去这个长度 */
    if ((puc_cowork_ie[1] + MAC_IE_HDR_LEN) <
        (sizeof(oal_sta_ap_cowork_ie_beacon) - sizeof(oal_cowork_ie_dc_roaming_stru))) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_is_over_ds_allowed::cowork ie too short[%d]}", puc_cowork_ie[1]);
        return OAL_FALSE;
    }

    cowork_ie = (oal_sta_ap_cowork_ie_beacon *)puc_cowork_ie;

    if (cowork_ie->capa_bitmap.ft_over_ds != 1) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_is_over_ds_allowed::ft_over_ds in cowork ie is disabled}");
        return OAL_FALSE;
    } else {
        return OAL_TRUE;
    }
}
#endif

OAL_STATIC uint32_t hmac_roam_connect_check_other_vap(hmac_roam_info_stru *roam_info, mac_bss_dscr_stru *bss_dscr)
{
    uint8_t vap_idx;
    hmac_vap_stru *hmac_vap = roam_info->pst_hmac_vap;
    mac_vap_stru *other_vap = NULL;
    mac_device_stru *mac_device;

    mac_device = (mac_device_stru *)mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (mac_device == NULL) {
        oam_error_log0(0, OAM_SF_ROAM,
                       "{hmac_roam_connect_check_other_vap::mac_device ptr is null!}");
        return OAL_FAIL;
    }

    for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
        other_vap = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
        if (other_vap == NULL) {
            oam_warning_log0(mac_device->auc_vap_id[vap_idx], OAM_SF_ROAM,
                             "{hmac_roam_connect_check_other_vap::vap is null!}");
            continue;
        }
        /* check all vap state in case other vap is connecting now */
        if (oal_value_in_valid_range(other_vap->en_vap_state,
                                     MAC_VAP_STATE_STA_JOIN_COMP,
                                     MAC_VAP_STATE_STA_WAIT_ASOC)) {
            oam_warning_log1(other_vap->uc_vap_id, OAM_SF_ROAM,
                             "{hmac_roam_connect_check_other_vap::vap is connecting, state=[%d]!}",
                             other_vap->en_vap_state);
            hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_INIT);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC oal_bool_enum_uint8 hmac_roam_connect_to_bss_11r(hmac_vap_stru *hmac_vap, hmac_roam_info_stru *roam_info)
{
    oal_bool_enum_uint8 stop = OAL_TRUE;
#ifdef _PRE_WLAN_FEATURE_11R
    if ((mac_mib_get_ft_trainsistion(&hmac_vap->st_vap_base_info) == OAL_TRUE &&
         mac_mib_get_ft_over_ds(&hmac_vap->st_vap_base_info))) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_connect_to_bss::cmd bit_11r_over_ds = [%d]!}",
                         hmac_vap->bit_11r_over_ds);

#ifdef _PRE_WLAN_FEATURE_ADAPTIVE11R
        if (hmac_roam_is_over_ds_allowed(roam_info) == OAL_TRUE) {
            roam_info->st_connect.uc_ft_force_air = OAL_FALSE;
        }
#endif

        /* modify仅第一次使用ds方式 */
        if ((hmac_vap->bit_11r_over_ds == OAL_TRUE) && (roam_info->st_connect.uc_ft_force_air != OAL_TRUE)) {
            stop = OAL_FALSE;
        }
    }
#endif  // _PRE_WLAN_FEATURE_11R
    return stop;
}

OAL_STATIC void hmac_roam_connect_save_old_bss(hmac_roam_old_bss_stru *old_bss, hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user)
{
     /* 原bss信息保存，以便回退 */
    old_bss->us_sta_aid = hmac_vap->st_vap_base_info.us_sta_aid;
    old_bss->en_protocol_mode = hmac_vap->st_vap_base_info.en_protocol;
    memcpy_s(&old_bss->st_cap_info, sizeof(mac_user_cap_info_stru),
             &(hmac_user->st_user_base_info.st_cap_info), sizeof(mac_user_cap_info_stru));
    memcpy_s(&old_bss->st_key_info, sizeof(mac_key_mgmt_stru),
             &(hmac_user->st_user_base_info.st_key_info), sizeof(mac_key_mgmt_stru));
    memcpy_s(&old_bss->st_user_tx_info, sizeof(mac_user_tx_param_stru),
             &(hmac_user->st_user_base_info.st_user_tx_info), sizeof(mac_user_tx_param_stru));
    memcpy_s(&old_bss->st_mib_info, sizeof(wlan_mib_ieee802dot11_stru),
             hmac_vap->st_vap_base_info.pst_mib_info, sizeof(wlan_mib_ieee802dot11_stru));
    memcpy_s(&old_bss->st_op_rates, sizeof(hmac_rate_stru),
             &(hmac_user->st_op_rates), sizeof(hmac_rate_stru));
    memcpy_s(&old_bss->st_ht_hdl, sizeof(mac_user_ht_hdl_stru),
             &(hmac_user->st_user_base_info.st_ht_hdl), sizeof(mac_user_ht_hdl_stru));
    memcpy_s(&old_bss->st_vht_hdl, sizeof(mac_vht_hdl_stru),
             &(hmac_user->st_user_base_info.st_vht_hdl), sizeof(mac_vht_hdl_stru));
    old_bss->en_avail_bandwidth = hmac_user->st_user_base_info.en_avail_bandwidth;
    old_bss->en_cur_bandwidth = hmac_user->st_user_base_info.en_cur_bandwidth;
    memcpy_s(&old_bss->st_channel, sizeof(mac_channel_stru),
             &(hmac_vap->st_vap_base_info.st_channel), sizeof(mac_channel_stru));
    memcpy_s(&old_bss->auc_bssid, WLAN_MAC_ADDR_LEN,
             &(hmac_vap->st_vap_base_info.auc_bssid), WLAN_MAC_ADDR_LEN);
    old_bss->us_cap_info = hmac_vap->st_vap_base_info.us_assoc_user_cap_info;
    old_bss->en_ap_type = hmac_user->en_user_ap_type;
    old_bss->sap_mode = hmac_user->sap_mode;
}

OAL_STATIC uint32_t hmac_roam_connect_to_bss(hmac_roam_info_stru *roam_info, void *param)
{
    hmac_vap_stru *hmac_vap = roam_info->pst_hmac_vap;
    hmac_user_stru *hmac_user = roam_info->pst_hmac_user;
    mac_bss_dscr_stru *bss_dscr = (mac_bss_dscr_stru *)param;
    uint32_t ret;
    oal_bool_enum_uint8 need_to_stop_user;

    ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_SCANING);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_connect_to_bss::check_state fail[%d]!}", ret);
        return ret;
    }

    if (hmac_roam_connect_check_other_vap(roam_info, bss_dscr) != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    need_to_stop_user = hmac_roam_connect_to_bss_11r(hmac_vap, roam_info);
    if (need_to_stop_user != OAL_FALSE) {
        roam_info->st_static.connect_start_timetamp = (uint32_t)oal_time_get_stamp_ms();

        /* 切换vap的状态为ROAMING，将用户节能，暂停发送 */
        ret = hmac_fsm_call_func_sta(hmac_vap, HMAC_FSM_INPUT_ROAMING_START, (void *)bss_dscr);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ROAM,
                             "{hmac_roam_connect_to_bss::hmac_fsm_call_func_sta fail[%ld]!}", ret);
            return ret;
        }
    }

    hmac_roam_connect_save_old_bss(&roam_info->st_old_bss, hmac_vap, hmac_user);

    /* 切换状态至connecting */
    hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_CONNECTING);

    hmac_user->rssi_last_timestamp = 0;
    roam_info->st_static.connect_cnt++;

    /* 设置漫游到的bss能力位，重关联请求使用 */
    hmac_vap->st_vap_base_info.us_assoc_user_cap_info = bss_dscr->us_cap_info;

    hmac_config_set_mgmt_log(&hmac_vap->st_vap_base_info, &hmac_user->st_user_base_info, OAL_TRUE);

#ifdef _PRE_WINDOWS_SUPPORT
    hmac_roam_main_change_state((hmac_roam_info_stru *)hmac_vap->pul_roam_info, ROAM_MAIN_STATE_CONNECTING);
#endif
    /* 启动connect状态机 */
    ret = hmac_roam_connect_start(hmac_vap, bss_dscr);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_connect_to_bss::hmac_roam_connect_start fail[%ld]!}", ret);
        hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_FAIL);
        return ret;
    }

    roam_info->st_connect.uc_roam_main_fsm_state = ROAM_MAIN_FSM_EVENT_START_CONNECT;

    /* 启动connect超时定时器 */
    hmac_roam_main_start_timer(roam_info, ROAM_CONNECT_TIME_MAX);

    return OAL_SUCC;
}

OAL_STATIC void hmac_roam_restore_old_bss(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
                                          hmac_roam_old_bss_stru *old_bss)
{
    hmac_vap->st_vap_base_info.us_sta_aid = old_bss->us_sta_aid;
    hmac_vap->st_vap_base_info.en_protocol = old_bss->en_protocol_mode;
    memcpy_s(&(hmac_user->st_user_base_info.st_cap_info), sizeof(mac_user_cap_info_stru),
             &old_bss->st_cap_info, sizeof(mac_user_cap_info_stru));
    memcpy_s(&(hmac_user->st_user_base_info.st_key_info), sizeof(mac_key_mgmt_stru),
             &old_bss->st_key_info, sizeof(mac_key_mgmt_stru));
    memcpy_s(&(hmac_user->st_user_base_info.st_user_tx_info), sizeof(mac_user_tx_param_stru),
             &old_bss->st_user_tx_info, sizeof(mac_user_tx_param_stru));
    memcpy_s(hmac_vap->st_vap_base_info.pst_mib_info, sizeof(wlan_mib_ieee802dot11_stru),
             &old_bss->st_mib_info, sizeof(wlan_mib_ieee802dot11_stru));
    memcpy_s(&(hmac_user->st_op_rates), sizeof(hmac_rate_stru),
             &old_bss->st_op_rates, sizeof(hmac_rate_stru));

    memcpy_s(&(hmac_user->st_user_base_info.st_ht_hdl), sizeof(mac_user_ht_hdl_stru),
             &old_bss->st_ht_hdl, sizeof(mac_user_ht_hdl_stru));
    memcpy_s(&(hmac_user->st_user_base_info.st_vht_hdl), sizeof(mac_vht_hdl_stru),
             &old_bss->st_vht_hdl, sizeof(mac_vht_hdl_stru));
    hmac_user->st_user_base_info.en_avail_bandwidth = old_bss->en_avail_bandwidth;
    hmac_user->st_user_base_info.en_cur_bandwidth = old_bss->en_cur_bandwidth;
    memcpy_s(&(hmac_vap->st_vap_base_info.st_channel), sizeof(mac_channel_stru),
             &old_bss->st_channel, sizeof(mac_channel_stru));
    memcpy_s(hmac_vap->st_vap_base_info.auc_bssid, WLAN_MAC_ADDR_LEN,
             old_bss->auc_bssid, WLAN_MAC_ADDR_LEN);
    hmac_vap->st_vap_base_info.us_assoc_user_cap_info = old_bss->us_cap_info;
    hmac_user->en_user_ap_type = old_bss->en_ap_type;
    hmac_user->sap_mode = old_bss->sap_mode;
}


OAL_STATIC uint32_t hmac_roam_to_old_bss(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;
    hmac_vap_stru *hmac_vap = roam_info->pst_hmac_vap;
    hmac_user_stru *hmac_user = roam_info->pst_hmac_user;
    hmac_roam_old_bss_stru *old_bss = &roam_info->st_old_bss;
    mac_h2d_roam_sync_stru *h2d_sync = NULL;

    if (old_bss == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_to_old_bss::old_bss null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    roam_info->st_static.roam_old_cnt++;
    roam_info->st_static.roam_eap_fail++;

    /* 恢复原来bss相关信息 */
    hmac_roam_restore_old_bss(hmac_vap, hmac_user, old_bss);

    /* 设置用户8021x端口合法性的状态为合法 */
    mac_user_set_port(&hmac_user->st_user_base_info, OAL_TRUE);

    ret = hmac_config_start_vap_event(&hmac_vap->st_vap_base_info, OAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_to_old_bss::hmac_config_start_vap_event failed[%d].}", ret);
        return ret;
    }

    /* 相关参数需要配置到dmac */
    hmac_roam_connect_set_join_reg(&hmac_vap->st_vap_base_info, hmac_user);

    /* 更新用户的mac地址，漫游时mac会更新 */
    oal_set_mac_addr(hmac_user->st_user_base_info.auc_user_mac_addr, hmac_vap->st_vap_base_info.auc_bssid);

#ifdef _PRE_WLAN_FEATURE_PMF
    /* 此函数要在同步user能力前调用，同步user能力后会开启硬件PMF使能 */
    hmac_config_vap_pmf_cap(&hmac_vap->st_vap_base_info, old_bss->st_cap_info.bit_pmf_active);
#endif

    ret = hmac_config_user_info_syn(&(hmac_vap->st_vap_base_info), &(hmac_user->st_user_base_info));
    if (ret != OAL_SUCC) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_to_old_bss::hmac_syn_vap_state failed[%d].}", ret);
    }

    /* 回退 bss 时，hmac 2 dmac 同步的相关信息，以便失败的时候回退 */
    h2d_sync = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, sizeof(mac_h2d_roam_sync_stru), OAL_TRUE);
    if (h2d_sync == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM, "{hmac_roam_to_old_bss::alloc fail!}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    // 填充同步信息
    h2d_sync->back_to_old = OAL_TRUE;
    h2d_sync->us_sta_aid = old_bss->us_sta_aid;
    memcpy_s(&(h2d_sync->st_channel), sizeof(mac_channel_stru), &old_bss->st_channel, sizeof(mac_channel_stru));
    memcpy_s(&(h2d_sync->st_cap_info), sizeof(mac_user_cap_info_stru),
             &old_bss->st_cap_info, sizeof(mac_user_cap_info_stru));
    memcpy_s(&(h2d_sync->st_key_info), sizeof(mac_key_mgmt_stru),
             &old_bss->st_key_info, sizeof(mac_key_mgmt_stru));
    memcpy_s(&(h2d_sync->st_user_tx_info), sizeof(mac_user_tx_param_stru),
             &old_bss->st_user_tx_info, sizeof(mac_user_tx_param_stru));

    /* 在漫游过程中可能又建立了聚合，因此回退时需要删除掉 */
    hmac_tid_clear(&hmac_vap->st_vap_base_info, hmac_user);

    // 发送同步信息
    ret = hmac_config_send_event(&hmac_vap->st_vap_base_info, WLAN_CFGID_ROAM_HMAC_SYNC_DMAC,
                                 sizeof(mac_h2d_roam_sync_stru), (uint8_t *)h2d_sync);
    if (ret != OAL_SUCC) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_to_old_bss::send event[WLAN_CFGID_ROAM_HMAC_SYNC_DMAC] failed[%d].}", ret);
    }

    /* 释放同步数据 */
    if (h2d_sync != NULL) {
        oal_mem_free_m(h2d_sync, OAL_TRUE);
    }

    /* user已经关联上，抛事件给DMAC，在DMAC层挂用户算法钩子 */
    hmac_user_add_notify_alg(&hmac_vap->st_vap_base_info, hmac_user->st_user_base_info.us_assoc_id);
    hmac_config_set_mgmt_log(&hmac_vap->st_vap_base_info, &hmac_user->st_user_base_info, OAL_FALSE);
    hmac_user->rssi_last_timestamp = 0;

    oam_warning_log4(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                     "{hmac_roam_to_old_bss::now resuming to [%02X:XX:XX:%02X:%02X:%02X]}",
                     old_bss->auc_bssid[MAC_ADDR_0], old_bss->auc_bssid[MAC_ADDR_3],
                     old_bss->auc_bssid[MAC_ADDR_4], old_bss->auc_bssid[MAC_ADDR_5]);
#ifdef PLATFORM_DEBUG_ENABLE
    hmac_config_reg_info(&(hmac_vap->st_vap_base_info), 4, "all"); /* 4为"all"的长度 */
#endif
    return ret;
}

OAL_STATIC uint8_t hmac_roam_to_new_band_type(hmac_roam_info_stru *roam_info, hmac_vap_stru *hmac_vap)
{
    wlan_roam_main_band_state_enum_uint8 roam_band_state;

    if (roam_info->st_old_bss.st_channel.en_band == WLAN_BAND_2G) {
        if (hmac_vap->st_vap_base_info.st_channel.en_band == WLAN_BAND_2G) {
            roam_band_state = WLAN_ROAM_MAIN_BAND_STATE_2TO2;
        } else {
            roam_band_state = WLAN_ROAM_MAIN_BAND_STATE_2TO5;
        }
    } else {
        if (hmac_vap->st_vap_base_info.st_channel.en_band == WLAN_BAND_2G) {
            roam_band_state = WLAN_ROAM_MAIN_BAND_STATE_5TO2;
        } else {
            roam_band_state = WLAN_ROAM_MAIN_BAND_STATE_5TO5;
        }
    }

    return roam_band_state;
}


OAL_STATIC uint32_t hmac_roam_to_new_bss(hmac_roam_info_stru *roam_info, void *param)
{
    hmac_vap_stru *hmac_vap = roam_info->pst_hmac_vap;
    hmac_user_stru *hmac_user = roam_info->pst_hmac_user;
    uint32_t ret;
    wlan_roam_main_band_state_enum_uint8 roam_band_state;
    oal_bool_enum_uint8 post_dmac = OAL_TRUE;

    ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_to_new_bss::check_state fail[%d]!}", ret);
        return ret;
    }

    roam_info->st_connect.uc_roam_main_fsm_state = ROAM_MAIN_FSM_EVENT_CONNECT_SUCC;
    roam_info->st_static.roam_new_cnt++;
    hmac_roam_alg_add_history(roam_info, hmac_vap->st_vap_base_info.auc_bssid);

    hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_UP);

    hmac_roam_main_del_timer(roam_info);

    /* 切换vap的状态为UP，恢复用户节能，恢复发送 */
    ret = hmac_fsm_call_func_sta(hmac_vap, HMAC_FSM_INPUT_ROAMING_STOP, (void *)&post_dmac);
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_to_new_bss::hmac_fsm_call_func_sta fail! erro code is %u}", ret);
    }
    hmac_config_set_mgmt_log(&hmac_vap->st_vap_base_info, &hmac_user->st_user_base_info, OAL_FALSE);

    hmac_vap->st_vap_base_info.bit_roam_scan_valid_rslt = OAL_TRUE;

    oam_warning_log4(0, OAM_SF_ROAM, "{hmac_roam_to_new_bss::roam to [%02X:XX:XX:%02X:%02X:%02X] succ}",
        hmac_vap->st_vap_base_info.auc_bssid[MAC_ADDR_0], hmac_vap->st_vap_base_info.auc_bssid[MAC_ADDR_3],
        hmac_vap->st_vap_base_info.auc_bssid[MAC_ADDR_4], hmac_vap->st_vap_base_info.auc_bssid[MAC_ADDR_5]);
    hmac_roam_main_clear(roam_info);

    /* 标识漫游类型，传到dmac做对应业务处理 */
    roam_band_state = hmac_roam_to_new_band_type(roam_info, hmac_vap);

    /* 漫游成功之后，同步到dmac给相关业务做处理 */
    ret = hmac_config_send_event(&hmac_vap->st_vap_base_info,
        WLAN_CFGID_ROAM_SUCC_H2D_SYNC, sizeof(uint8_t), (uint8_t *)&roam_band_state);
    if (ret != OAL_SUCC) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_to_new_bss::send event[WLAN_CFGID_ROAM_SUCC_H2D_SYNC] failed[%d].}", ret);
    }

    hmac_chr_roam_info_report(roam_info, HMAC_CHR_ROAM_SUCCESS);
    return ret;
}


uint32_t hmac_roam_rssi_trigger_type(hmac_vap_stru *hmac_vap, roam_scenario_enum en_val)
{
    hmac_roam_info_stru *roam_info = NULL;
    uint32_t ret;
    uint8_t vap_id;
    mac_roam_trigger_stru roam_trigger;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_rssi_trigger_type::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    vap_id = hmac_vap->st_vap_base_info.uc_vap_id;

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        oam_error_log0(vap_id, OAM_SF_ROAM, "{hmac_roam_rssi_trigger_type::roam info null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    if (roam_info->uc_rssi_type == en_val) {
        return OAL_SUCC;
    }

    if (en_val == ROAM_ENV_LINKLOSS) {
        roam_trigger.c_trigger_2G = ROAM_RSSI_LINKLOSS_TYPE;
        roam_trigger.c_trigger_5G = ROAM_RSSI_LINKLOSS_TYPE;
        roam_info->st_alg.uc_invalid_scan_cnt = 0;
    } else if (en_val == ROAM_ENV_DENSE_AP) {
        roam_trigger.c_trigger_2G = roam_info->st_config.c_dense_env_trigger_rssi_2g;
        roam_trigger.c_trigger_5G = roam_info->st_config.c_dense_env_trigger_rssi_5g;
    } else if (en_val == ROAM_ENV_SPARSE_AP_TH1) {
        roam_trigger.c_trigger_2G = roam_info->st_config.c_trigger_th1_2g;
        roam_trigger.c_trigger_5G = roam_info->st_config.c_trigger_th1_5g;
    } else {
        roam_trigger.c_trigger_2G = roam_info->st_config.c_trigger_rssi_2g;
        roam_trigger.c_trigger_5G = roam_info->st_config.c_trigger_rssi_5g;
    }
    roam_trigger.us_roam_interval = roam_info->st_config.us_roam_interval;

    ret = hmac_config_send_event(&hmac_vap->st_vap_base_info, WLAN_CFGID_SET_ROAM_TRIGGER,
                                 sizeof(mac_roam_trigger_stru), (uint8_t *)&roam_trigger);
    if (ret != OAL_SUCC) {
        oam_error_log2(vap_id, OAM_SF_CFG,
                       "{hmac_roam_rssi_trigger_type::send event[%d] failed[%d].}", en_val, ret);
        return ret;
    }
    roam_info->uc_rssi_type = en_val;

    oam_warning_log1(vap_id, OAM_SF_ROAM, "{hmac_roam_rssi_trigger_type::[%d]}", en_val);
    return OAL_SUCC;
}


uint32_t hmac_roam_pause_user(hmac_vap_stru *hmac_vap, void *param)
{
    hmac_roam_info_stru *roam_info = NULL;
    oal_net_device_stru *net_device = NULL;
    uint32_t ret;
    uint8_t vap_id;
    uint8_t roaming_mode;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_pause_user::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    vap_id = hmac_vap->st_vap_base_info.uc_vap_id;

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if ((roam_info == NULL) || (roam_info->uc_enable == 0)) {
        oam_error_log0(vap_id, OAM_SF_ROAM, "{hmac_roam_pause_user::roam disable!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    net_device = hmac_vap->pst_net_device;
    if (net_device == NULL) {
        oam_error_log0(vap_id, OAM_SF_ROAM, "{hmac_roam_pause_user::net_device null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 必须保证vap的状态是UP */
    if (hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_UP) {
        oam_warning_log1(vap_id, OAM_SF_ROAM,
            "{hmac_roam_pause_user::vap state =[%d]NOT FOR ROAMING!}", hmac_vap->st_vap_base_info.en_vap_state);
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }

    /* 暂停所有协议层数据，这样就不需要再hmac搞一个缓存队列了 */
    oal_net_tx_stop_all_queues(net_device);
    oal_net_wake_subqueue(net_device, WLAN_HI_QUEUE);

    /* 清空 HMAC层TID信息 */
    hmac_tid_clear(&hmac_vap->st_vap_base_info, roam_info->pst_hmac_user);
#ifdef _PRE_WLAN_FEATURE_HIEX
    memset_s(&roam_info->pst_hmac_user->st_ersru, sizeof(mac_hiex_ersru_ctrl_msg_stru),
        0, sizeof(mac_hiex_ersru_ctrl_msg_stru));
#endif
    ret = hmac_config_set_pm_by_module(&hmac_vap->st_vap_base_info, MAC_STA_PM_CTRL_TYPE_ROAM,
                                       MAC_STA_PM_SWITCH_OFF);
    if (ret != OAL_SUCC) {
        oam_error_log1(vap_id, OAM_SF_CFG,
                       "{hmac_roam_pause_user::hmac_config_set_pm_by_module failed[%d].}", ret);
        oal_net_tx_wake_all_queues(net_device);
        return ret;
    }

    roaming_mode = 1;
    ret = hmac_config_send_event(&hmac_vap->st_vap_base_info, WLAN_CFGID_SET_ROAMING_MODE,
                                 sizeof(uint8_t), (uint8_t *)&roaming_mode);
    if (ret != OAL_SUCC) {
        oam_error_log1(vap_id, OAM_SF_CFG,
                       "{hmac_roam_pause_user::send event[WLAN_CFGID_SET_ROAMING_MODE] failed[%d].}", ret);
        oal_net_tx_wake_all_queues(net_device);
        return ret;
    }

    if (hmac_vap->st_vap_base_info.pst_vowifi_cfg_param != NULL) {
        if (hmac_vap->st_vap_base_info.pst_vowifi_cfg_param->en_vowifi_mode == VOWIFI_LOW_THRES_REPORT) {
            /* 针对漫游和去关联场景,切换vowifi语音状态 */
            hmac_config_vowifi_report((&hmac_vap->st_vap_base_info), 0, NULL);
        }
    }
    hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_ROAMING);

    oam_warning_log0(vap_id, OAM_SF_ROAM, "{hmac_roam_pause_user::queues stopped!}");
    return OAL_SUCC;
}

OAL_STATIC void hmac_roam_mode_h2d_syn(hmac_vap_stru *hmac_vap,
    hmac_roam_info_stru *roam_info, oal_bool_enum_uint8 need_post_stop_event)
{
    uint8_t roaming_mode;
    uint32_t ret;

    if (need_post_stop_event ==  OAL_TRUE) {
        roaming_mode = 0;
        ret = hmac_config_send_event(&hmac_vap->st_vap_base_info, WLAN_CFGID_SET_ROAMING_MODE,
            sizeof(uint8_t), (uint8_t *)&roaming_mode);
        if (ret != OAL_SUCC) {
            oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                "{hmac_roam_mode_h2d_syn::send event[WLAN_CFGID_SET_ROAMING_MODE] failed[%d].}", ret);
        }
    }
}


uint32_t hmac_roam_resume_user(hmac_vap_stru *hmac_vap, void *param)
{
    hmac_roam_info_stru *roam_info = NULL;
    oal_net_device_stru *net_device = NULL;
    uint8_t vap_id;

    if (hmac_vap == NULL || param == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_resume_user::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    vap_id = hmac_vap->st_vap_base_info.uc_vap_id;

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        oam_error_log0(vap_id, OAM_SF_ROAM, "{hmac_roam_resume_user::roam info null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    net_device = hmac_vap->pst_net_device;
    if (net_device == NULL) {
        oam_error_log0(vap_id, OAM_SF_ROAM, "{hmac_roam_resume_user::net_device null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 必须保证vap的状态是roaming */
    if (hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_ROAMING) {
        hmac_roam_resume_pm(roam_info);
        hmac_roam_resume_security_port(roam_info);
        oal_net_tx_wake_all_queues(net_device);
        oam_warning_log1(vap_id, OAM_SF_ROAM,
                         "{hmac_roam_resume_user::vap state[%d] NOT ROAMING!}",
                         hmac_vap->st_vap_base_info.en_vap_state);
        return OAL_SUCC;
    }

    hmac_roam_mode_h2d_syn(hmac_vap, roam_info, *(uint8_t *)param);

    hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_UP);

    hmac_roam_resume_pm(roam_info);

    hmac_roam_resume_security_port(roam_info);

    oal_net_tx_wake_all_queues(net_device);

    oam_warning_log0(vap_id, OAM_SF_ROAM, "{hmac_roam_resume_user::all_queues awake!}");

    return OAL_SUCC;
}


uint32_t hmac_roam_scan_complete(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru *roam_info = NULL;
    hmac_device_stru *hmac_device = NULL;
    uint8_t vap_id;
    hmac_bss_mgmt_stru *scan_bss_mgmt = NULL;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_scan_complete::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    vap_id = hmac_vap->st_vap_base_info.uc_vap_id;

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 漫游开关没有开时，不处理扫描结果 */
    if (roam_info->uc_enable == 0) {
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_scan_complete::device null!}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    scan_bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    oam_info_log0(vap_id, OAM_SF_ROAM, "{hmac_roam_scan_complete::handling scan result!}");
    return hmac_roam_main_fsm_action(roam_info, ROAM_MAIN_FSM_EVENT_SCAN_RESULT, (void *)scan_bss_mgmt);
}


void hmac_roam_tbtt_handle(hmac_vap_stru *hmac_vap)
{
}

OAL_STATIC void hmac_roam_set_scan_channel(hmac_vap_stru *hmac_vap, hmac_roam_info_stru *roam_info)
{
    mac_vap_stru *mac_vap = &hmac_vap->st_vap_base_info;

    if (!mac_is_dual_sta_mode() || mac_is_primary_legacy_vap(mac_vap)) {
        mac_vap_stru *other_vap = NULL;
        roam_info->st_config.uc_scan_orthogonal = ROAM_SCAN_CHANNEL_ORG_SPECIFIED;

        /* check p2p up state */
        other_vap = mac_vap_find_another_up_vap_by_mac_vap(mac_vap);
        if (hmac_p2p_get_scenes(other_vap) == MAC_P2P_SCENES_LOW_LATENCY) {
            roam_info->st_config.uc_scan_orthogonal = ROAM_SCAN_CHANNEL_ORG_DBDC;
        }
    } else {
        /* WLAN1只需要扫描当前频段 */
        roam_info->st_config.uc_scan_orthogonal =
            mac_vap->st_channel.en_band == WLAN_BAND_2G ? ROAM_SCAN_CHANNEL_ORG_2G : ROAM_SCAN_CHANNEL_ORG_5G;
    }
}


uint32_t hmac_roam_trigger_handle(hmac_vap_stru *hmac_vap, int8_t c_rssi,
                                  oal_bool_enum_uint8 en_current_bss_ignore)
{
    uint32_t ret;
    hmac_roam_info_stru *roam_info = NULL;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_trigger_handle::hmac_vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 黑名单路由器，不支持漫游，防止漫游出现异常 */
    if (hmac_vap->en_roam_prohibit_on == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_trigger_handle::blacklist ap not support roam!}");
        return OAL_FAIL;
    }

    ret = hmac_roam_check_signal_bridge(hmac_vap);
    if (ret != OAL_SUCC) {
        return ret;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_trigger_handle::hmac_vap->pul_roam_info is NULL,Return!}");
        return OAL_ERR_CODE_ROAM_EVENT_UXEXPECT;
    }

    /* 每次漫游前，刷新是否支持漫游到自己的参数 */
    roam_info->en_current_bss_ignore = en_current_bss_ignore;
    roam_info->en_roam_trigger = ROAM_TRIGGER_DMAC;
#ifdef _PRE_WLAN_FEATURE_11R
    roam_info->st_connect.uc_ft_force_air = OAL_TRUE;
    roam_info->st_connect.uc_ft_failed = OAL_FALSE;
#endif
    hmac_roam_set_scan_channel(hmac_vap, roam_info);

    ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_INIT);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_trigger_handle::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_roam_alg_init(roam_info, c_rssi);

    ret = hmac_roam_main_fsm_action(roam_info, ROAM_MAIN_FSM_EVENT_START, NULL);
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_trigger_handle::START fail[%d].}", ret);
        return ret;
    }

    hmac_chr_roam_info_report(roam_info, HMAC_CHR_ROAM_START);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11K

uint32_t hmac_roam_send_neighbor_req(hmac_roam_info_stru *roam_info, uint8_t  *cur_ssid)
{
    wal_msg_write_stru write_msg;
    int32_t ret;
    mac_cfg_ssid_param_stru *ssid = NULL;
    oal_net_device_stru *net_dev = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    hmac_vap = roam_info->pst_hmac_vap;
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_send_neighbor_req::hmac_vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }
    net_dev = hmac_vap->pst_net_device;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SEND_NEIGHBOR_REQ, sizeof(mac_cfg_ssid_param_stru));
    ssid = (mac_cfg_ssid_param_stru *)write_msg.auc_value;
    if (memcpy_s(ssid->ac_ssid, WLAN_SSID_MAX_LEN, cur_ssid, WLAN_SSID_MAX_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_roam_send_neighbor_req::memcpy fail!");
    }
    ssid->ac_ssid[WLAN_SSID_MAX_LEN - 1] = '\0';
    ssid->uc_ssid_len = OAL_STRLEN(ssid->ac_ssid);

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_ssid_param_stru),
                             (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_roam_send_neighbor_req::return err code [%d]!}\r\n", ret);
        return (uint32_t)ret;
    }
    roam_info->st_timer.timeout = ROAM_NEIGHBOR_PROCESS_TIME_MAX;

    /* 启动扫描超时定时器 */
    hmac_roam_main_start_timer(roam_info, ROAM_NEIGHBOR_PROCESS_TIME_MAX);

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_wait_neighbor_response_timeout(hmac_roam_info_stru *roam_info, void *param)
{
    mac_scan_req_stru *scan_params = NULL;
    uint32_t ret;

    if (roam_info == NULL) {
        oam_warning_log0(0, OAM_SF_RRM, "{hmac_wait_neighbor_rsp_timeout::roam_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 更改状态 */
    hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_SCANING);

    /* 设置扫描参数 */
    scan_params = &roam_info->st_scan_h2d_params.st_scan_params;

    if (scan_params->uc_channel_nums != 0) {
        scan_params->uc_scan_channel_interval = 2; /* 间隔2个信道，切回工作信道工作一段时间 */
        scan_params->us_work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_PERFORMANCE;
        scan_params->uc_max_scan_count_per_channel = 2; /* 每个信道扫描2次 */
    } else {
        roam_info->st_static.uc_scan_mode = HMAC_CHR_NORMAL_SCAN;
        scan_params->uc_neighbor_report_process_flag = OAL_FALSE;
        scan_params->uc_max_scan_count_per_channel = 2; /* 每个信道扫描2次 */
        memset_s(scan_params->ast_mac_ssid_set[1].auc_ssid, WLAN_SSID_MAX_LEN, 0, WLAN_SSID_MAX_LEN);
        ret = hmac_roam_scan_channel_init(roam_info, scan_params);
        if (ret != OAL_SUCC) {
            oam_error_log1(roam_info->pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                           "{hmac_wait_neighbor_rsp_timeout::hmac_roam_alg_scan_channel_init fail[%d]}", ret);
            return ret;
        }
    }

    ret = hmac_roam_main_fsm_action(roam_info, ROAM_MAIN_FSM_EVENT_START, (void *)roam_info);
    if (ret != OAL_SUCC) {
        hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_FAIL);
        oam_warning_log1(roam_info->pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "{hmac_wait_neighbor_rsp_timeout::START fail[%d].}", ret);
        return ret;
    }

    /* only use last neighbor list once */
    scan_params->uc_channel_nums = 0;
    return OAL_SUCC;
}
#endif


uint32_t hmac_sta_roam_rx_mgmt(hmac_vap_stru *hmac_vap, void *param)
{
    dmac_wlan_crx_event_stru *crx_event = NULL;
    dmac_rx_ctl_stru *rx_ctrl = NULL;
    mac_rx_ctl_stru *rx_info = NULL;
    uint8_t *mac_hdr = NULL;
    uint8_t frame_sub_type;
    uint8_t frame_type;
    oal_bool_enum_uint8 is_protected;

    crx_event = (dmac_wlan_crx_event_stru *)param;
    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(crx_event->pst_netbuf);
    rx_info = (mac_rx_ctl_stru *)(&(rx_ctrl->st_rx_info));
    mac_hdr = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(rx_info);

    frame_type = mac_get_frame_type(mac_hdr);
    frame_sub_type = mac_get_frame_sub_type(mac_hdr);
    is_protected = mac_is_protectedframe(mac_hdr);

    if (frame_type == WLAN_FC0_TYPE_MGT && frame_sub_type == WLAN_FC0_SUBTYPE_ACTION) {
        hmac_mgmt_sta_roam_rx_action(hmac_vap, crx_event->pst_netbuf, is_protected);
    }

    hmac_roam_connect_rx_mgmt(hmac_vap, crx_event);
    return OAL_SUCC;
}

void hmac_roam_add_key_done(hmac_vap_stru *hmac_vap)
{
    hmac_roam_connect_key_done(hmac_vap);
}


void hmac_roam_wpas_connect_state_notify(hmac_vap_stru *hmac_vap, hmac_wpas_connect_state_enum_uint32 conn_state)
{
    hmac_roam_info_stru *roam_info = NULL;
    uint32_t ret;
    uint32_t notify_data;
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_wpas_connect_state_notify::vap null!}");
        return;
    }

    if (!IS_LEGACY_VAP(&hmac_vap->st_vap_base_info)) {
        return;
    }

    if (hmac_vap->st_vap_base_info.en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        return;
    }

    if (roam_info->connected_state == conn_state) {
        return;
    }

    oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                     "{hmac_roam_wpas_connect_state_notify:: state changed: [%d]-> [%d]}",
                     roam_info->connected_state, conn_state);

    roam_info->connected_state = conn_state;
    notify_data = hmac_vap->ipaddr_obtained;
    ret = hmac_config_send_event(&hmac_vap->st_vap_base_info, WLAN_CFGID_ROAM_NOTIFY_STATE,
                                 sizeof(uint32_t), (uint8_t *)&notify_data);
    if (ret != OAL_SUCC) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                       "{hmac_roam_wpas_connect_state_notify::send event failed[%d].}", ret);
        return;
    }
}

#ifdef _PRE_WLAN_FEATURE_11R

uint32_t hmac_roam_reassoc(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru *roam_info = NULL;
    uint32_t ret;
    uint8_t vap_id;
    mac_bss_dscr_stru *bss_dscr = NULL;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_reassoc::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    vap_id = hmac_vap->st_vap_base_info.uc_vap_id;

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        oam_warning_log0(vap_id, OAM_SF_ROAM, "{hmac_roam_reassoc::roam info null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 漫游开关没有开时，不处理 */
    if (roam_info->uc_enable == 0) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM, "{hmac_roam_reassoc::roam disable}");
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    /* 主状态机为非CONNECTING状态，失败 */
    if (roam_info->en_main_state != ROAM_MAIN_STATE_CONNECTING) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_reassoc::main_state=[%d] not CONNECTING}",
                         roam_info->en_main_state);
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }

    /* CONNECT状态机为非WAIT_JOIN状态，失败 */
    if (roam_info->st_connect.en_state != ROAM_CONNECT_STATE_WAIT_ASSOC_COMP) {
        if (roam_info->st_connect.en_state == ROAM_CONNECT_STATE_WAIT_PREAUTH_COMP) {
            ret =  hmac_roam_preauth(hmac_vap);
            return ret;
        }
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_reassoc::connect state[%d] error.}", roam_info->st_connect.en_state);
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }

    if ((mac_mib_get_ft_trainsistion(&hmac_vap->st_vap_base_info)) == OAL_TRUE) {
        if (mac_mib_get_ft_over_ds(&hmac_vap->st_vap_base_info) &&
            (hmac_vap->bit_11r_over_ds == OAL_TRUE)) { /* over ds 首先pause user */
            bss_dscr = roam_info->st_connect.pst_bss_dscr;
            if (bss_dscr == NULL) {
                oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                               "{hmac_roam_reassoc::bss_dscr is null.}");
                return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
            }

            roam_info->st_static.connect_start_timetamp = (uint32_t)oal_time_get_stamp_ms();

            ret = hmac_fsm_call_func_sta(hmac_vap, HMAC_FSM_INPUT_ROAMING_START, (void *)bss_dscr);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ROAM,
                                 "{hmac_roam_reassoc::hmac_fsm_call_func_sta fail[%ld]!}", ret);
                return ret;
            }
        }
        oam_warning_log1(vap_id, OAM_SF_ROAM, "{hmac_roam_reassoc::ft_over_ds=[%d],to send reassoc!}",
                         mac_mib_get_ft_over_ds(&hmac_vap->st_vap_base_info));
        ret = hmac_roam_connect_ft_reassoc(hmac_vap);
        if (ret != OAL_SUCC) {
            oam_warning_log1(vap_id, OAM_SF_ROAM, "{hmac_roam_reassoc::reassoc FAIL[%d]!}", ret);
            return ret;
        }
    }

    return OAL_SUCC;
}
#endif

oal_bool_enum_uint8 hmac_roam_check_dbac_channel(mac_vap_stru *p2p_vap, uint8_t chan_num)
{
    mac_p2p_scenes_enum_uint8 p2p_scene = hmac_p2p_get_scenes(p2p_vap);
    /* check dbac channel compared with p2p up channel */
    if (p2p_vap != NULL && p2p_scene == MAC_P2P_SCENES_LOW_LATENCY) {
        if (chan_num != p2p_vap->st_channel.uc_chan_number &&
            mac_get_band_by_channel_num(chan_num) == p2p_vap->st_channel.en_band) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

#ifdef _PRE_WLAN_FEATURE_11K
oal_bool_enum_uint8 hmac_neighbor_report_check_validity(hmac_vap_stru *hmac_vap,
    mac_neighbor_rpt_ie_stru *neighbor_rpt_ie, mac_vap_stru *p2p_vap)
{
    if (hmac_vap == NULL || neighbor_rpt_ie == NULL) {
        return OAL_FALSE;
    }

    if (neighbor_rpt_ie->uc_len < MAC_NEIGHBOR_REPORT_IE_LEN) {
        oam_error_log1(0, OAM_SF_ROAM,
            "{hmac_neighbor_report_check_validity:: invalid ie len[%d]}", neighbor_rpt_ie->uc_len);
        return OAL_FALSE;
    }

    /* check dbac channel compared with p2p up channel */
    if (hmac_roam_check_dbac_channel(p2p_vap, neighbor_rpt_ie->uc_channum) == OAL_TRUE) {
        oam_warning_log3(0, OAM_SF_ROAM,
            "{hmac_neighbor_report_check_validity::dbac channel ignored, %0x:%0x chanNum=%d}",
            neighbor_rpt_ie->auc_bssid[4], neighbor_rpt_ie->auc_bssid[5], /* mac addr 4 & 5 */
            neighbor_rpt_ie->uc_channum);
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

void hmac_roam_neighbor_report_add_chan_and_bssid(hmac_roam_info_stru *roam_info,
    mac_neighbor_rpt_ie_stru *neighbor_rpt_ie)
{
    wlan_channel_band_enum_uint8 en_band;
    uint8_t channel_idx = 0;
    uint8_t channl_num;
    mac_scan_req_stru *scan_params = NULL;

    if (roam_info == NULL || neighbor_rpt_ie == NULL) {
        return;
    }

    scan_params = &roam_info->st_scan_h2d_params.st_scan_params;

    channl_num = neighbor_rpt_ie->uc_channum;
    en_band = mac_get_band_by_channel_num(channl_num);
    if (mac_get_channel_idx_from_num(en_band, channl_num, OAL_FALSE, &channel_idx) == OAL_SUCC) {
        hmac_roam_neighbor_report_add_chan(scan_params, channl_num, en_band, channel_idx);
    }
    /* 将解析出来的 auc_bssid 存储到 roam_info->auc_neighbor_rpt_bssid[] 中 */
    /* 同时更新要扫描的邻居AP数量(roam_info->uc_neighbor_rpt_bssid_num) 的值 */
    hmac_roam_neighbor_report_add_bssid(roam_info, neighbor_rpt_ie->auc_bssid);
}


OAL_STATIC uint32_t hmac_roam_parse_neighbor_response(hmac_roam_info_stru *roam_info, void *param)
{
    oal_netbuf_stru *netbuf = (oal_netbuf_stru *)param;
    int32_t framebody_len;
    dmac_rx_ctl_stru *rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    uint8_t *puc_ie = NULL;
    uint8_t *puc_neighbor_rpt_ie = NULL;
    mac_neighbor_rpt_ie_stru *neighbor_rpt_ie = NULL;
    mac_scan_req_stru *scan_params = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_vap_stru *other_vap = NULL;

    if (rx_ctrl == NULL || roam_info == NULL || roam_info->pst_hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = roam_info->pst_hmac_vap;
    other_vap = mac_vap_find_another_up_vap_by_mac_vap(&(hmac_vap->st_vap_base_info));

    framebody_len = (int32_t)MAC_GET_RX_CB_PAYLOAD_LEN(&(rx_ctrl->st_rx_info));

    /*************************************************************************/
    /*                   Neighbor Report Response Frame - Frame Body         */
    /* --------------------------------------------------------------------- */
    /* |Category |Action |Dialog Token| Neighbor Report Elements           | */
    /* --------------------------------------------------------------------- */
    /* |1        |1      | 1          | var                                  */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    roam_info->uc_neighbor_rpt_bssid_num = 0;
    scan_params = &roam_info->st_scan_h2d_params.st_scan_params;
    scan_params->uc_channel_nums = 0;
    scan_params->uc_neighbor_report_process_flag = OAL_TRUE;
    scan_params->uc_max_scan_count_per_channel = 1;
    memcpy_s(scan_params->ast_mac_ssid_set[1].auc_ssid, WLAN_SSID_MAX_LEN,
             scan_params->ast_mac_ssid_set[0].auc_ssid, WLAN_SSID_MAX_LEN);

    if (framebody_len <= HMAC_NEIGHBOR_REPORT_ACTION_RSP_FIX_LEN) {
        return OAL_SUCC;
    }

    framebody_len -= HMAC_NEIGHBOR_REPORT_ACTION_RSP_FIX_LEN;
    while (framebody_len >= HMAC_NEIGHBOR_RPT_MIN_LEN + MAC_IE_HDR_LEN) {
        if (puc_ie == NULL) {
            puc_ie = ((mac_action_neighbor_rsp_stru *)mac_netbuf_get_payload(netbuf))->neighbor_rpt_ies;
        }
        puc_neighbor_rpt_ie = mac_find_ie(MAC_EID_NEIGHBOR_REPORT, puc_ie, framebody_len);
        if (puc_neighbor_rpt_ie == NULL) {
            break;
        }
        /* puc_neighbor_rpt_ie[1]最小为13, 此时mac_neighbor_rpt_ie_stru无可选subele, 右值需减掉1, 以正常解析 */
        if (puc_neighbor_rpt_ie[1] < sizeof(mac_neighbor_rpt_ie_stru) - 1 - MAC_IE_HDR_LEN) {
            oam_warning_log1(0, 0, "hmac_roam_parse_neighbor_response::ie len too short[%d]", puc_neighbor_rpt_ie[1]);
            break;
        }
        neighbor_rpt_ie = (mac_neighbor_rpt_ie_stru *)puc_neighbor_rpt_ie;
        if (hmac_neighbor_report_check_validity(hmac_vap, neighbor_rpt_ie, other_vap) == OAL_TRUE) {
            hmac_roam_neighbor_report_add_chan_and_bssid(roam_info, neighbor_rpt_ie);
        }

        framebody_len -= neighbor_rpt_ie->uc_len + MAC_IE_HDR_LEN;
        puc_ie += neighbor_rpt_ie->uc_len + MAC_IE_HDR_LEN;
    }
    oam_warning_log2(0, OAM_SF_RRM, "hmac_roam_parse_neighbor_response::chan num[%d] bssid num[%d]",
                     scan_params->uc_channel_nums, roam_info->uc_neighbor_rpt_bssid_num);

    return OAL_SUCC;
}


uint32_t hmac_roam_rx_neighbor_response_action(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    hmac_roam_info_stru *roam_info = NULL;
    uint32_t ret;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_rx_neighbor_rsp_action::vap null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_rx_neighbor_rsp_action::roam info null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* connecting state: hmac_roam_parse_neighbor_response 只解析Action Frame
     * neighbor_process: hmac_roam_neighbor_response_process: 解析Action Frame，并且切换Roaming FSM到扫描状态
     */
    ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_NEIGHBOR_PROCESS);
    if (ret != OAL_SUCC) {
        hmac_roam_parse_neighbor_response(roam_info, (void *)netbuf);
        return ret;
    }

    ret = hmac_roam_main_fsm_action(roam_info, ROAM_MAIN_FSM_EVENT_START, (void *)netbuf);
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_rx_neighbor_rsp_action::MGMT_RX FAIL[%d]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_roam_neighbor_response_process(hmac_roam_info_stru *roam_info, void *param)
{
    mac_scan_req_stru *scan_params = NULL;
    uint32_t ret;

    if (roam_info == NULL || param == NULL) {
        oam_error_log0(0, OAM_SF_RRM, "{hmac_roam_neighbor_response_process::param or roam_info is NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_roam_parse_neighbor_response(roam_info, param);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_RRM,
            "{hmac_roam_neighbor_response_process::hmac_roam_parse_neighbor_response ret=%d}", ret);
        return ret;
    }

    scan_params = &roam_info->st_scan_h2d_params.st_scan_params;

    if (scan_params->uc_channel_nums == 0) {
        scan_params->uc_neighbor_report_process_flag = OAL_FALSE;
        scan_params->uc_max_scan_count_per_channel = 2; /* 每个信道扫描2次 */
        memset_s(scan_params->ast_mac_ssid_set[1].auc_ssid, WLAN_SSID_MAX_LEN, 0, WLAN_SSID_MAX_LEN);
        ret = hmac_roam_scan_channel_init(roam_info, scan_params);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_neighbor_response_process::channel init fail[%d]}", ret);
            return ret;
        }
    } else {
        scan_params->en_need_switch_back_home_channel = OAL_TRUE;
        scan_params->uc_scan_channel_interval = MAC_SCAN_CHANNEL_INTERVAL_PERFORMANCE;
        scan_params->us_work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_PERFORMANCE;
        scan_params->bit_desire_fast_scan = OAL_TRUE; // 11k neighbor 触发的漫游扫描是并发扫描
    }

    roam_info->st_static.uc_scan_mode =
        (scan_params->uc_channel_nums == 0) ? HMAC_CHR_NORMAL_SCAN : HMAC_CHR_11K_SCAN;

    hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_SCANING);
    ret = hmac_roam_main_fsm_action(roam_info, ROAM_MAIN_FSM_EVENT_START, (void *)roam_info);
    if (ret != OAL_SUCC) {
        hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_FAIL);
        return ret;
    }

    return OAL_SUCC;
}
#endif


void hmac_roam_connect_complete(hmac_vap_stru *hmac_vap, uint32_t result)
{
    hmac_roam_info_stru *roam_info = NULL;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_complete::vap null!}");
        return;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_connect_complete::roam_info null!}");
        return;
    }

    /* 漫游开关没有开时，不处理扫描结果 */
    if (roam_info->uc_enable == 0) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_connect_complete::roam disabled!}");
        return;
    }

    roam_info->st_static.connect_end_timetamp = (uint32_t)oal_time_get_stamp_ms();

    if (roam_info->uc_roaming_scenario == ROAMING_SCENARIO_HOME) {
        roam_info->st_alg.st_home_network.scan_timestamp_5g = (uint32_t)oal_time_get_stamp_ms();
    }

    if (roam_info->en_roam_trigger == ROAM_TRIGGER_HOME_NETWORK) {
        if (result == OAL_SUCC) {
            roam_info->st_alg.uc_roam_fail_cnt = 0;
        } else {
            roam_info->st_alg.uc_roam_fail_cnt++;
        }
    }

    if (result == OAL_SUCC) {
        hmac_roam_main_fsm_action(roam_info, ROAM_MAIN_FSM_EVENT_CONNECT_SUCC, NULL);
    } else if (result == OAL_ERR_CODE_ROAM_HANDSHAKE_FAIL) {
        hmac_roam_main_fsm_action(roam_info, ROAM_MAIN_FSM_EVENT_HANDSHAKE_FAIL, NULL);
    } else if (result == OAL_ERR_CODE_ROAM_NO_RESPONSE) {
        hmac_roam_main_fsm_action(roam_info, ROAM_MAIN_FSM_EVENT_CONNECT_FAIL, NULL);
    } else {
        /* 上层触发停止漫游时，先删除相关定时器 */
        hmac_roam_main_del_timer(roam_info);
        frw_timer_immediate_destroy_timer_m(&(roam_info->st_connect.st_timer));
        hmac_roam_main_fsm_action(roam_info, ROAM_MAIN_FSM_EVENT_TIMEOUT, NULL);
    }
}


uint32_t hmac_roam_connect_ft_ds_change_to_air(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *param)
{
    uint32_t ret;
    hmac_roam_info_stru *roam_info = NULL;

    if (hmac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_ft_ds_change_to_air::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    if (param == NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_ft_ds_change_to_air::param null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_ft_ds_change_to_air::roam_info null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    hmac_roam_main_clear(roam_info);
    hmac_roam_main_del_timer(roam_info);

    hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_SCANING);

    ret = hmac_roam_connect_to_bss(roam_info, (void *)param);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}

uint32_t hmac_roam_pri_sta_join_check(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *new_bss, mac_vap_stru *other_vap)
{
    hmac_roam_info_stru *roam_info = (hmac_roam_info_stru*)hmac_vap->pul_roam_info;
    mac_vap_stru *mac_vap = &hmac_vap->st_vap_base_info;
    hmac_roam_old_bss_stru *old_bss = NULL;
    if (roam_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_roam_dual_sta_join_check::roam info is null!}");
        return OAL_SUCC;
    }
    old_bss = &roam_info->st_old_bss;
    if (new_bss->st_channel.en_band != old_bss->st_channel.en_band) {
        // wlan0漫游到wlan1工作的band
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC,
            "{hmac_roam_dual_sta_join_check::pri sta is roaming to diff band, disconnect sec sta!}");
        return OAL_FAIL;
    }
    if (g_wlan_spec_cfg->feature_slave_ax_is_support == OAL_FALSE &&
        (new_bss->en_he_capable || new_bss->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) &&
        (other_vap->en_protocol == WLAN_HE_MODE ||
        other_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS)) {
        // wlan1不能去辅路，wlan0漫游后也不能去辅路
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_roam_dual_sta_join_check::"
            "pri sta is roaming, both the new bss[HE %d, bandwidth %d] and sec sta[HE %d, bandwidth %d]"
            "cannot work on slave, disconnect sec sta!}", new_bss->en_he_capable, new_bss->st_channel.en_bandwidth,
            other_vap->en_protocol == WLAN_HE_MODE, other_vap->st_channel.en_bandwidth);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


OAL_STATIC oal_bool_enum_uint8 hmac_roam_is_vaild_report_chr_info(hmac_roam_info_stru *roam_info,
                                                                  uint8_t result)
{
    oam_warning_log3(0, OAM_SF_ROAM, "{hmac_roam_is_vaild_report_chr_info::[%d]->[%d]->[%d]}",
                     roam_info->en_roam_trigger, roam_info->st_static.uc_scan_mode, result);

    if (result == HMAC_CHR_ROAM_START) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}


OAL_STATIC void hmac_chr_report_roam_fail_info(hmac_roam_info_stru *roam_info,
                                               uint8_t result)
{
    hmac_chr_roam_connect_info_stru chr_roam_connect_info = { {0, 0, 0, 0, 0, 0}, 0, 0, {0, 0, 0, 0, 0, 0}, 0 };
    hmac_vap_stru *hmac_vap = NULL;
    mac_bss_dscr_stru *bss_dscr = NULL;
    uint32_t roam_dur;
    uint32_t now;

    if (!hmac_roam_is_vaild_report_chr_info(roam_info, result)) {
        return;
    }

    now = (uint32_t)oal_time_get_stamp_ms();
    roam_dur = oal_time_get_runtime(roam_info->st_static.scan_start_timetamp, now);

    chr_roam_connect_info.uc_roam_time = roam_dur;
    chr_roam_connect_info.uc_roam_result = result;
    chr_roam_connect_info.uc_roam_mode = roam_info->en_roam_trigger;
    chr_roam_connect_info.uc_roam_stage = roam_info->en_main_state;

    /* current AP bssid, rssi and channel */
    hmac_vap = roam_info->pst_hmac_vap;
    memcpy_s(&chr_roam_connect_info.auc_src_bssid, WLAN_MAC_ADDR_LEN,
             &(hmac_vap->st_vap_base_info.auc_bssid), WLAN_MAC_ADDR_LEN);
    chr_roam_connect_info.uc_src_channel = hmac_vap->st_vap_base_info.st_channel.uc_chan_number;
    chr_roam_connect_info.uc_src_rssi = roam_info->st_alg.c_current_rssi; // maybe not right

    /* roaming target AP bssid, rssi and channel */
    bss_dscr = roam_info->st_connect.pst_bss_dscr;
    if (bss_dscr != NULL && result != HMAC_CHR_ROAM_SCAN_FAIL) {
        memcpy_s(&chr_roam_connect_info.auc_target_bssid, WLAN_MAC_ADDR_LEN,
                 &(bss_dscr->auc_bssid), WLAN_MAC_ADDR_LEN);
        chr_roam_connect_info.uc_target_channel = bss_dscr->st_channel.uc_chan_number;
        chr_roam_connect_info.uc_target_rssi = bss_dscr->c_rssi;
    }

    oam_warning_log3(0, OAM_SF_ROAM, "{hmac_chr_report_roam_fail_info::1[%d]->[%d] [%d]}",
                     chr_roam_connect_info.uc_roam_mode, chr_roam_connect_info.uc_roam_result,
                     chr_roam_connect_info.uc_roam_time);

    chr_exception_p(CHR_WIFI_ROAM_FAIL_REPORT_EVENTID,
                    (uint8_t *)(&chr_roam_connect_info),
                    sizeof(hmac_chr_roam_connect_info_stru));
}


void hmac_chr_roam_info_report(hmac_roam_info_stru *roam_info, uint8_t result)
{
    hmac_chr_roam_info_stru chr_roam_info = { 0 };
    uint32_t scan_dur;
    uint32_t connect_dur;

    if (roam_info == NULL || roam_info->pst_hmac_vap == NULL) {
        return;
    }

    /* when connect fail, not double report timeout event */
    if (roam_info->st_connect.uc_roam_main_fsm_state == ROAM_MAIN_FSM_EVENT_CONNECT_FAIL
        && result == HMAC_CHR_ROAM_TIMEOUT_FAIL) {
        return;
    }

    /* chr report roam connect info */
    hmac_chr_report_roam_fail_info(roam_info, result);

    scan_dur = oal_time_get_runtime(roam_info->st_static.scan_start_timetamp,
                                    roam_info->st_static.scan_end_timetamp);
    connect_dur = oal_time_get_runtime(roam_info->st_static.connect_start_timetamp,
                                       roam_info->st_static.connect_end_timetamp);
    if (result == HMAC_CHR_ROAM_SUCCESS) {
        chr_roam_info.uc_scan_time = scan_dur;
        chr_roam_info.uc_connect_time = connect_dur;
    } else {
        chr_roam_info.uc_scan_time = 0;
        chr_roam_info.uc_connect_time = 0;
    }
    chr_roam_info.uc_trigger = roam_info->en_roam_trigger;
    chr_roam_info.uc_roam_result = result;
    chr_roam_info.uc_roam_mode = roam_info->st_static.uc_roam_mode;
    chr_roam_info.uc_scan_mode = roam_info->st_static.uc_scan_mode;

    oam_warning_log2(0, OAM_SF_ROAM, "{hmac_chr_roam_info_report::1[%d]->[%d]}",
                     chr_roam_info.uc_trigger, chr_roam_info.uc_roam_result);
    oam_warning_log2(0, OAM_SF_ROAM, "{hmac_chr_roam_info_report::2[%d]->[%d]}",
                     chr_roam_info.uc_scan_time, chr_roam_info.uc_connect_time);

    chr_exception_p(CHR_WIFI_ROAM_INFO_REPORT_EVENTID,
                    (uint8_t *)(&chr_roam_info),
                    sizeof(hmac_chr_roam_info_stru));
    return;
}


void hmac_roam_kick_secondary_sta(hmac_vap_stru *hmac_vap, int8_t rssi)
{
    mac_vap_stru *mac_vap = &hmac_vap->st_vap_base_info;
    hmac_device_stru *hmac_dev = NULL;
    mac_vap_stru *other_vap = NULL;
    mac_cfg_kick_user_param_stru kick_user_param;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    /* 只在双sta模式下，wlan0触发漫游时 */
    if (!mac_is_dual_sta_mode() || !mac_is_primary_legacy_sta(mac_vap)) {
        return;
    }

    hmac_dev = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (hmac_dev == NULL || hmac_dev->pst_device_base_info == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ROAM,
            "{hmac_roam_kick_secondary_sta::dev[%d] ptr is null}", mac_vap->uc_device_id);
        return;
    }
    /* 扫描无法立即停止，暂不考虑和扫描冲突的场景 */
    if (hmac_dev->st_scan_mgmt.en_is_scanning) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_kick_secondary_sta::scan is running, rssi[%d]!}", rssi);
        return;
    }

    other_vap = mac_device_find_another_up_vap(hmac_dev->pst_device_base_info, mac_vap->uc_vap_id);
    if (other_vap == NULL) {
        /* wlan1未关联 */
        return;
    }

    if (other_vap->en_vap_state == MAC_VAP_STATE_ROAMING) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_kick_secondary_sta::wlan0 rssi[%d], \
            wlan1 is roanming, kick now}", rssi);
        kick_user_param.us_reason_code = MAC_UNSPEC_REASON;
        memcpy_s(kick_user_param.auc_mac_addr, WLAN_MAC_ADDR_LEN, auc_mac_addr, WLAN_MAC_ADDR_LEN);
        if (hmac_config_kick_user(other_vap, sizeof(kick_user_param), (uint8_t *)&kick_user_param) != OAL_SUCC) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                "{hmac_roam_kick_secondary_sta::secondary sta disconnect failed!");
        }
    }
}

void hmac_roam_timeout_test(hmac_vap_stru *hmac_vap)
{
    hmac_roam_connect_timeout(hmac_vap->pul_roam_info);
}

/* DC 使能场景，wlan0 漫游时，提前删除DC */
void hmac_roam_down_dc_p2p_group(void)
{
    uint8_t vap_idx;
    mac_device_stru *mac_device = mac_res_get_mac_dev();

    if (mac_device->dc_status == OAL_FALSE) {
        return;
    }

    for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
        hmac_vap_stru *hmac_vap = NULL;
        wlan_p2p_mode_enum_uint8 en_p2p_mode;
        mac_vap_state_enum_uint8 en_vap_state;

        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_device->auc_vap_id[vap_idx]);
        if (hmac_vap == NULL) {
            oam_error_log1(0, OAM_SF_ANY, "hmac_roam_down_dc_p2p_group:vap is null. vap_id %d",
                mac_device->auc_vap_id[vap_idx]);
            continue;
        }

        en_vap_state = hmac_vap->st_vap_base_info.en_vap_state;
        en_p2p_mode = hmac_vap->st_vap_base_info.en_p2p_mode;
        if (en_vap_state == MAC_VAP_STATE_UP && (en_p2p_mode == WLAN_P2P_CL_MODE)) {
            mac_cfg_kick_user_param_stru kick_user_param = {
                .auc_mac_addr = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
                .us_reason_code = MAC_UNSPEC_REASON,
            };
            oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                "hmac_roam_down_dc_p2p_group:kick p2p user when DC enable!");
            hmac_config_kick_user(&(hmac_vap->st_vap_base_info), sizeof(mac_cfg_kick_user_param_stru),
                (uint8_t *)&kick_user_param);
        }
    }
}