

#include "oam_ext_if.h"
#include "mac_ie.h"
#include "mac_resource.h"
#include "mac_function.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_mgmt_sta.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_encap_frame_sta.h"
#include "hmac_tx_amsdu.h"
#include "hmac_rx_data.h"
#include "hmac_chan_mgmt.h"
#include "hmac_11i.h"
#include "hmac_roam_main.h"
#include "hmac_roam_connect.h"
#include "hmac_roam_alg.h"
#include "hmac_dfx.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif
#include "securec.h"
#include "hmac_mgmt_sta_up.h"
#ifdef _PRE_WLAN_FEATURE_11K
#include "wal_config.h"
#include "wal_linux_ioctl.h"
#endif
#include "hmac_11r.h"

#ifdef _PRE_WINDOWS_SUPPORT
#include "wal_windows_ioctl.h"
#else
#include "wal_linux_ioctl.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ROAM_CONNECT_C

OAL_STATIC hmac_roam_fsm_func g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_BUTT][ROAM_CONNECT_FSM_EVENT_TYPE_BUTT];
OAL_STATIC uint32_t hmac_roam_connect_null_fn(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_roam_start_join(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_roam_send_auth_seq1(hmac_roam_info_stru *roam_info);
#ifdef _PRE_WLAN_FEATURE_11R
OAL_STATIC uint32_t hmac_roam_send_ft_req(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_roam_process_ft_rsp(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_roam_process_ft_preauth_rsp(hmac_roam_info_stru *roam_info, void *param);
#endif  // _PRE_WLAN_FEATURE_11R
OAL_STATIC uint32_t hmac_roam_process_auth_seq2(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_roam_process_assoc_rsp(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_roam_process_action(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_roam_connect_succ(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_roam_connect_fail(hmac_roam_info_stru *roam_info);
OAL_STATIC uint32_t hmac_roam_auth_timeout(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_roam_assoc_timeout(hmac_roam_info_stru *roam_info, void *param);
OAL_STATIC uint32_t hmac_roam_handshaking_timeout(hmac_roam_info_stru *roam_info, void *param);
#ifdef _PRE_WLAN_FEATURE_11R
OAL_STATIC uint32_t hmac_roam_ft_timeout(hmac_roam_info_stru *roam_info, void *p_param);
OAL_STATIC uint32_t hmac_roam_ft_preauth_timeout(hmac_roam_info_stru *roam_info, void *param);
#endif  // _PRE_WLAN_FEATURE_11R
OAL_STATIC uint32_t hmac_roam_send_reassoc_req(hmac_roam_info_stru *roam_info);
#ifdef _PRE_WLAN_FEATURE_11K
OAL_STATIC uint32_t  hmac_sta_up_send_neighbor_req(hmac_roam_info_stru *roam_info);
#endif


void hmac_roam_connect_fsm_init(void)
{
    uint32_t state;
    uint32_t event;

    for (state = 0; state < ROAM_CONNECT_STATE_BUTT; state++) {
        for (event = 0; event < ROAM_CONNECT_FSM_EVENT_TYPE_BUTT; event++) {
            g_hmac_roam_connect_fsm_func[state][event] = hmac_roam_connect_null_fn;
        }
    }
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_INIT][ROAM_CONNECT_FSM_EVENT_START] = hmac_roam_start_join;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_WAIT_AUTH_COMP][ROAM_CONNECT_FSM_EVENT_MGMT_RX] =
        hmac_roam_process_auth_seq2;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_WAIT_AUTH_COMP][ROAM_CONNECT_FSM_EVENT_TIMEOUT] =
        hmac_roam_auth_timeout;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_WAIT_ASSOC_COMP][ROAM_CONNECT_FSM_EVENT_MGMT_RX] =
        hmac_roam_process_assoc_rsp;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_WAIT_ASSOC_COMP][ROAM_CONNECT_FSM_EVENT_TIMEOUT] =
        hmac_roam_assoc_timeout;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_HANDSHAKING][ROAM_CONNECT_FSM_EVENT_MGMT_RX] =
        hmac_roam_process_action;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_HANDSHAKING][ROAM_CONNECT_FSM_EVENT_KEY_DONE] =
        hmac_roam_connect_succ;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_HANDSHAKING][ROAM_CONNECT_FSM_EVENT_TIMEOUT] =
        hmac_roam_handshaking_timeout;
#ifdef _PRE_WLAN_FEATURE_11R
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_INIT][ROAM_CONNECT_FSM_EVENT_FT_OVER_DS] = hmac_roam_send_ft_req;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_WAIT_FT_COMP][ROAM_CONNECT_FSM_EVENT_MGMT_RX] =
        hmac_roam_process_ft_rsp;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_WAIT_FT_COMP][ROAM_CONNECT_FSM_EVENT_TIMEOUT] =
        hmac_roam_ft_timeout;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_WAIT_PREAUTH_COMP][ROAM_CONNECT_FSM_EVENT_MGMT_RX] =
        hmac_roam_process_ft_preauth_rsp;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_WAIT_PREAUTH_COMP][ROAM_CONNECT_FSM_EVENT_TIMEOUT] =
        hmac_roam_ft_preauth_timeout;
#endif  // _PRE_WLAN_FEATURE_11R
}


uint32_t hmac_roam_connect_fsm_action(hmac_roam_info_stru *roam_info,
                                      roam_connect_fsm_event_type_enum event, void *param)
{
    if (roam_info == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (roam_info->st_connect.en_state >= ROAM_CONNECT_STATE_BUTT) {
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }

    if (event >= ROAM_CONNECT_FSM_EVENT_TYPE_BUTT) {
        return OAL_ERR_CODE_ROAM_EVENT_UXEXPECT;
    }

    return g_hmac_roam_connect_fsm_func[roam_info->st_connect.en_state][event](roam_info, param);
}


OAL_STATIC void hmac_roam_connect_change_state(hmac_roam_info_stru *roam_info,
                                               roam_connect_state_enum state)
{
    if (roam_info != NULL) {
        oam_warning_log2(0, OAM_SF_ROAM,
                         "{hmac_roam_connect_change_state::[%d]->[%d]}", roam_info->st_connect.en_state, state);
        roam_info->st_connect.en_state = state;
    }
}


OAL_STATIC uint32_t hmac_roam_connect_check_state(hmac_roam_info_stru *roam_info,
                                                  mac_vap_state_enum_uint8 vap_state,
                                                  roam_main_state_enum main_state,
                                                  roam_connect_state_enum connect_state)
{
    if (roam_info == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (roam_info->pst_hmac_vap == NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    if (roam_info->pst_hmac_user == NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_USER;
    }

    if (roam_info->uc_enable == 0) {
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    if ((roam_info->pst_hmac_vap->st_vap_base_info.en_vap_state != vap_state) ||
        (roam_info->en_main_state != main_state) ||
        (roam_info->st_connect.en_state != connect_state)) {
        oam_warning_log3(0, OAM_SF_ROAM,
                         "{hmac_roam_connect_check_state::unexpect vap_state[%d] main_state[%d] connect_state[%d]!}",
                         roam_info->pst_hmac_vap->st_vap_base_info.en_vap_state,
                         roam_info->en_main_state, roam_info->st_connect.en_state);
        return OAL_ERR_CODE_ROAM_INVALID_VAP_STATUS;
    }

    return OAL_SUCC;
}

uint32_t hmac_roam_connect_timeout(void *arg)
{
    hmac_roam_info_stru *roam_info = (hmac_roam_info_stru *)arg;

    if (roam_info == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_timeout::roam_info is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log2(0, OAM_SF_ROAM, "{hmac_roam_connect_timeout::MAIN_STATE[%d] CONNECT_STATE[%d].}",
                     roam_info->en_main_state, roam_info->st_connect.en_state);

    return hmac_roam_connect_fsm_action(roam_info, ROAM_CONNECT_FSM_EVENT_TIMEOUT, NULL);
}

OAL_STATIC uint32_t hmac_roam_connect_null_fn(hmac_roam_info_stru *roam_info, void *param)
{
    oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_null_fn .}");

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    if (OAL_TRUE == wlan_pm_wkup_src_debug_get()) {
        wlan_pm_wkup_src_debug_set(OAL_FALSE);
        oam_warning_log0(0, OAM_SF_RX, "{wifi_wake_src:hmac_roam_connect_null_fn::rcv mgmt frame,drop it}");
    }
#endif

    return OAL_SUCC;
}

OAL_STATIC void hmac_roam_connect_pm_wkup(void)
{
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    if (OAL_TRUE == wlan_pm_wkup_src_debug_get()) {
        wlan_pm_wkup_src_debug_set(OAL_FALSE);
        oam_warning_log0(0, OAM_SF_RX, "{wifi_wake_src:hmac_roam_connect_pm_wkup::rcv mgmt frame}");
    }
#endif
}


OAL_STATIC void hmac_roam_connect_start_timer(hmac_roam_info_stru *roam_info, uint32_t timeout)
{
    frw_timeout_stru *timer = &(roam_info->st_connect.st_timer);

    oam_info_log1(0, OAM_SF_ROAM, "{hmac_roam_connect_start_timer [%d].}", timeout);

    /* 启动认证超时定时器 */
    frw_timer_create_timer_m(timer,
                             hmac_roam_connect_timeout,
                             timeout,
                             roam_info,
                             OAL_FALSE,
                             OAM_MODULE_ID_HMAC,
                             roam_info->pst_hmac_vap->st_vap_base_info.core_id);
}


OAL_STATIC void hmac_roam_connect_del_timer(hmac_roam_info_stru *roam_info)
{
    frw_timer_immediate_destroy_timer_m(&(roam_info->st_connect.st_timer));
}


uint32_t hmac_roam_connect_set_join_reg(mac_vap_stru *mac_vap, hmac_user_stru *hmac_user)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    dmac_ctx_join_req_set_reg_stru *reg_params = NULL;
    hmac_join_req_stru st_join_req;

    memset_s(&st_join_req, sizeof(hmac_join_req_stru), 0, sizeof(hmac_join_req_stru));

    /* 抛事件DMAC_WLAN_CTX_EVENT_SUB_TYPE_JOIN_SET_REG到DMAC */
    event_mem = frw_event_alloc_m(sizeof(dmac_ctx_join_req_set_reg_stru));
    if (event_mem == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_connect_set_join_reg::event_mem ALLOC FAIL, size = %d.}",
                       sizeof(dmac_ctx_join_req_set_reg_stru));
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    /* 填写事件 */
    event = frw_get_event_stru(event_mem);
    frw_event_hdr_init(&(event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_JOIN_SET_REG,
                       sizeof(dmac_ctx_join_req_set_reg_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       mac_vap->uc_chip_id,
                       mac_vap->uc_device_id,
                       mac_vap->uc_vap_id);

    reg_params = (dmac_ctx_join_req_set_reg_stru *)event->auc_event_data;

    /* 设置需要写入寄存器的BSSID信息 */
    oal_set_mac_addr(reg_params->auc_bssid, mac_vap->auc_bssid);

    /* 填写信道相关信息 */
    reg_params->st_current_channel = mac_vap->st_channel;
    /* 以old user信息塑造虚假的入网结构体，调用函数 */
    /* 填写速率相关信息 */
    st_join_req.st_bss_dscr.uc_num_supp_rates = hmac_user->st_op_rates.uc_rs_nrates;
    if (memcpy_s(st_join_req.st_bss_dscr.auc_supp_rates, sizeof(uint8_t) * WLAN_USER_MAX_SUPP_RATES,
        hmac_user->st_op_rates.auc_rs_rates, sizeof(uint8_t) * WLAN_MAX_SUPP_RATES) != EOK) {
        oam_error_log0(0, OAM_SF_ROAM, "hmac_roam_connect_set_join_reg::memcpy fail!");
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }
    st_join_req.st_bss_dscr.en_ht_capable = hmac_user->st_user_base_info.st_ht_hdl.en_ht_capable;
    st_join_req.st_bss_dscr.en_vht_capable = hmac_user->st_user_base_info.st_vht_hdl.en_vht_capable;
    hmac_sta_get_min_rate(&reg_params->st_min_rate, &st_join_req);

    /* 设置dtim period信息 */
    reg_params->us_beacon_period = (uint16_t)mac_mib_get_BeaconPeriod(mac_vap);

    /* 同步FortyMHzOperationImplemented */
    reg_params->en_dot11FortyMHzOperationImplemented = mac_mib_get_FortyMHzOperationImplemented(mac_vap);

    /* 设置beacon filter关闭 */
    reg_params->beacon_filter = OAL_FALSE;

    /* 设置no frame filter打开 */
    reg_params->non_frame_filter = OAL_TRUE;

    reg_params->en_ap_type = hmac_user->en_user_ap_type;
    reg_params->sap_mode = hmac_user->sap_mode;

    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_roam_connect_set_dtim_param(mac_vap_stru *mac_vap, uint8_t dtim_cnt, uint8_t dtim_period)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    dmac_ctx_set_dtim_tsf_reg_stru *set_dtim_tsf_reg_params = NULL;

    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_set_dtim_param, mac_vap = NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛事件 DMAC_WLAN_CTX_EVENT_SUB_TYPE_JOIN_DTIM_TSF_REG 到DMAC, 申请事件内存 */
    event_mem = frw_event_alloc_m(sizeof(dmac_ctx_set_dtim_tsf_reg_stru));
    if (event_mem == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_connect_set_dtim_param::event_mem ALLOC FAIL, size = %d.}",
                       sizeof(dmac_ctx_set_dtim_tsf_reg_stru));
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_JOIN_DTIM_TSF_REG,
                       sizeof(dmac_ctx_set_dtim_tsf_reg_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       mac_vap->uc_chip_id,
                       mac_vap->uc_device_id,
                       mac_vap->uc_vap_id);

    set_dtim_tsf_reg_params = (dmac_ctx_set_dtim_tsf_reg_stru *)event->auc_event_data;

    /* 将dtim相关参数抛到dmac */
    set_dtim_tsf_reg_params->dtim_cnt = dtim_cnt;
    set_dtim_tsf_reg_params->dtim_period = dtim_period;
    memcpy_s(set_dtim_tsf_reg_params->auc_bssid, WLAN_MAC_ADDR_LEN, mac_vap->auc_bssid, WLAN_MAC_ADDR_LEN);
    set_dtim_tsf_reg_params->us_tsf_bit0 = BIT0;

    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);

    return OAL_SUCC;
}

static uint32_t hmac_roam_send_connect_event(hmac_roam_info_stru *roam_info, hmac_roam_rsp_stru roam_rsp,
                                             hmac_asoc_rsp_stru asoc_rsp, frw_event_mem_stru *event_mem)
{
    uint32_t result;
    int32_t ret;
    frw_event_stru *event = frw_get_event_stru(event_mem);
    hmac_vap_stru *hmac_vap = roam_info->pst_hmac_vap;

    if (!oal_memcmp(roam_rsp.auc_bssid, roam_info->st_old_bss.auc_bssid, WLAN_MAC_ADDR_LEN)) {
    /* Reassociation to the same BSSID: report NL80211_CMD_CONNECT event to supplicant
     * instead of NL80211_CMD_ROAM event in case supplicant ignore roam event to the
     * same bssid which will cause 4-way handshake failure */
    /* wpa_supplicant: wlan0: WPA: EAPOL-Key Replay Counter did not increase - dropping packet */
        oam_warning_log4(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_connect_notify_wpas::roam_to the same bssid [%02X:XX:XX:%02X:%02X:%02X]}",
                         roam_rsp.auc_bssid[BYTE_OFFSET_0], roam_rsp.auc_bssid[BYTE_OFFSET_3],
                         roam_rsp.auc_bssid[BYTE_OFFSET_4], roam_rsp.auc_bssid[BYTE_OFFSET_5]);
        frw_event_hdr_init(&(event->st_event_hdr),
                           FRW_EVENT_TYPE_HOST_CTX,
                           HMAC_HOST_CTX_EVENT_SUB_TYPE_ASOC_COMP_STA,
                           sizeof(hmac_asoc_rsp_stru),
                           FRW_EVENT_PIPELINE_STAGE_0,
                           hmac_vap->st_vap_base_info.uc_chip_id,
                           hmac_vap->st_vap_base_info.uc_device_id,
                           hmac_vap->st_vap_base_info.uc_vap_id);
        ret = memcpy_s((uint8_t *)frw_get_event_payload(event_mem), sizeof(hmac_roam_rsp_stru),
                       (uint8_t *)&asoc_rsp, sizeof(hmac_asoc_rsp_stru));
    } else {
        frw_event_hdr_init(&(event->st_event_hdr),
                           FRW_EVENT_TYPE_HOST_CTX,
                           HMAC_HOST_CTX_EVENT_SUB_TYPE_ROAM_COMP_STA,
                           sizeof(hmac_roam_rsp_stru),
                           FRW_EVENT_PIPELINE_STAGE_0,
                           hmac_vap->st_vap_base_info.uc_chip_id,
                           hmac_vap->st_vap_base_info.uc_device_id,
                           hmac_vap->st_vap_base_info.uc_vap_id);
        ret = memcpy_s((uint8_t *)frw_get_event_payload(event_mem), sizeof(hmac_roam_rsp_stru),
                       (uint8_t *)&roam_rsp, sizeof(hmac_roam_rsp_stru));
    }
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ROAM, "hmac_roam_connect_notify_wpas::memcpy fail!");
        return OAL_FAIL;
    }

    /* 分发事件 */
    result = frw_event_dispatch_event(event_mem);
    if (oal_unlikely(result != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{hmac_roam_connect_notify_wpas::frw_event_dispatch_event failed[%d].}", result);
        return OAL_FAIL;
    }

    return result;
}

static hmac_roam_rsp_stru hmac_roam_prepare_roam_rep(hmac_user_stru *hmac_user, uint8_t *mgmt_data,
    uint8_t *mgmt_data_req, uint8_t *mac_hdr, uint16_t msg_len)
{
    hmac_roam_rsp_stru roam_rsp;

    memset_s(&roam_rsp, sizeof(hmac_roam_rsp_stru), 0, sizeof(hmac_roam_rsp_stru));
    /* 获取AP的mac地址 */
    mac_get_address3(mac_hdr, roam_rsp.auc_bssid, WLAN_MAC_ADDR_LEN);

    roam_rsp.asoc_rsp_ie_len = msg_len - OAL_ASSOC_RSP_IE_OFFSET;
    if (mgmt_data != NULL) {
        memcpy_s(mgmt_data, roam_rsp.asoc_rsp_ie_len,
            (uint8_t *)(mac_hdr + OAL_ASSOC_RSP_IE_OFFSET), roam_rsp.asoc_rsp_ie_len);
        roam_rsp.puc_asoc_rsp_ie_buff = mgmt_data;
    }

    roam_rsp.asoc_req_ie_len = hmac_user->assoc_req_ie_len;
    if (mgmt_data_req != NULL) {
        memcpy_s(mgmt_data_req, hmac_user->assoc_req_ie_len,
            (uint8_t *)(hmac_user->puc_assoc_req_ie_buff), hmac_user->assoc_req_ie_len);
        roam_rsp.puc_asoc_req_ie_buff = mgmt_data_req;
    }
    return roam_rsp;
}


static hmac_asoc_rsp_stru hmac_roam_prepare_asoc_rep(hmac_roam_rsp_stru *roam_rsp, uint8_t *mac_hdr)
{
    hmac_asoc_rsp_stru asoc_rsp;

    memset_s(&asoc_rsp, sizeof(hmac_asoc_rsp_stru), 0, sizeof(hmac_asoc_rsp_stru));
    /* 获取AP的mac地址 */
    mac_get_address3(mac_hdr, asoc_rsp.auc_addr_ap, WLAN_MAC_ADDR_LEN);

    asoc_rsp.puc_asoc_req_ie_buff = roam_rsp->puc_asoc_req_ie_buff;
    asoc_rsp.asoc_req_ie_len = roam_rsp->asoc_req_ie_len;

    asoc_rsp.asoc_rsp_ie_len = roam_rsp->asoc_rsp_ie_len;
    asoc_rsp.puc_asoc_rsp_ie_buff = roam_rsp->puc_asoc_rsp_ie_buff;

    return asoc_rsp;
}

static uint32_t hmac_roam_copy_connect_notify_wpas(uint8_t **mgmt_data, uint8_t **mgmt_data_req,
    hmac_user_stru *hmac_user, hmac_vap_stru *hmac_vap, uint16_t msg_len)
{
    *mgmt_data = (uint8_t *)oal_memalloc(msg_len - OAL_ASSOC_RSP_IE_OFFSET);
    if (*mgmt_data == NULL) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
            "{hmac_handle_asoc_rsp_sta::mgmt_data alloc null,size %d.}", (msg_len - OAL_ASSOC_RSP_IE_OFFSET));
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    if (hmac_user->assoc_req_ie_len > 0 && hmac_user->puc_assoc_req_ie_buff != NULL) {
        *mgmt_data_req = (uint8_t *)oal_memalloc(hmac_user->assoc_req_ie_len);
        if (*mgmt_data_req == NULL) {
            oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                "{hmac_handle_asoc_rsp_sta::mgmt_data_req alloc null,size %d.}", hmac_user->assoc_req_ie_len);
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
    }

    return OAL_SUCC;
}

static void hmac_roam_free_copy_connect_notify_wpas(uint8_t **mgmt_data, uint8_t **mgmt_data_req)
{
    if (*mgmt_data != NULL) {
        oal_free(*mgmt_data);
        *mgmt_data = NULL;
    }
    if (*mgmt_data_req != NULL) {
        oal_free(*mgmt_data_req);
        *mgmt_data_req = NULL;
    }
}


OAL_STATIC uint32_t hmac_roam_connect_notify_wpas(hmac_roam_info_stru *roam_info,
                                                  uint8_t *mac_hdr, uint16_t msg_len)
{
    hmac_asoc_rsp_stru asoc_rsp;
    hmac_roam_rsp_stru roam_rsp;
    frw_event_mem_stru *event_mem = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = NULL;
    uint8_t *mgmt_data = NULL;
    uint8_t *mgmt_data_req = NULL;
    uint32_t ret;

    hmac_vap = roam_info->pst_hmac_vap;
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(hmac_vap->st_vap_base_info.us_assoc_vap_id);
    if (hmac_user == NULL) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_connect_notify_wpas, hmac_user[%d] = NULL!}",
                         hmac_vap->st_vap_base_info.us_assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 记录关联响应帧的部分内容，用于上报给内核 */
    if (msg_len < OAL_ASSOC_RSP_IE_OFFSET) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_handle_asoc_rsp_sta::msg_len is too short, %d.}", msg_len);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    event_mem = frw_event_alloc_m(sizeof(hmac_roam_rsp_stru));
    if (event_mem == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_handle_asoc_rsp_sta::frw_event_alloc_m fail!}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    // 本地申请空间保存数据，防止数据丢失 造成访问已释放内存的问题
    ret = hmac_roam_copy_connect_notify_wpas(&mgmt_data, &mgmt_data_req, hmac_user, hmac_vap, msg_len);
    if (ret != OAL_SUCC) {
        frw_event_free_m(event_mem);
        hmac_roam_free_copy_connect_notify_wpas(&mgmt_data, &mgmt_data_req);
        return ret;
    }

    roam_rsp = hmac_roam_prepare_roam_rep(hmac_user, mgmt_data, mgmt_data_req, mac_hdr, msg_len);
    asoc_rsp = hmac_roam_prepare_asoc_rep(&roam_rsp, mac_hdr);

    ret = hmac_roam_send_connect_event(roam_info, roam_rsp, asoc_rsp, event_mem);
    if (ret == OAL_FAIL) {
        hmac_roam_free_copy_connect_notify_wpas(&mgmt_data, &mgmt_data_req);
    }
    frw_event_free_m(event_mem);

    return ret;
}
#ifdef _PRE_WLAN_FEATURE_11R
OAL_STATIC hmac_roam_ft_stru *hmac_roam_ft_notify_wpas_get_event(mac_vap_stru *mac_vap,
    frw_event_stru *event)
{
    hmac_roam_ft_stru *ft_event = NULL;

    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_HOST_CTX,
        HMAC_HOST_CTX_EVENT_SUB_TYPE_FT_EVENT_STA, sizeof(hmac_roam_ft_stru), FRW_EVENT_PIPELINE_STAGE_0,
        mac_vap->uc_chip_id, mac_vap->uc_device_id, mac_vap->uc_vap_id);
    ft_event = (hmac_roam_ft_stru *)event->auc_event_data;

    return ft_event;
}

OAL_STATIC uint32_t hmac_roam_ft_notify_wpas(hmac_vap_stru *hmac_vap, uint8_t *mac_hdr, uint16_t msg_len)
{
    hmac_roam_ft_stru *ft_event = NULL;
    frw_event_stru *event = NULL;
    uint16_t ie_offset;
    uint8_t *ft_ie_buff = NULL;
    uint32_t ret;
    uint8_t *target_ap_addr = NULL;
    mac_vap_stru *mac_vap = &hmac_vap->st_vap_base_info;
    frw_event_mem_stru *event_mem = frw_event_alloc_m(sizeof(hmac_roam_rsp_stru));

    if (event_mem == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_ft_notify_wpas::frw_event_alloc_m fail!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    event = frw_get_event_stru(event_mem);
    ft_event = hmac_roam_ft_notify_wpas_get_event(mac_vap, event);
    if (mac_get_frame_type_and_subtype(mac_hdr) == (WLAN_FC0_SUBTYPE_AUTH | WLAN_FC0_TYPE_MGT)) {
        ie_offset = OAL_AUTH_IE_OFFSET;
        mac_get_address3(mac_hdr, ft_event->auc_bssid, WLAN_MAC_ADDR_LEN);
    } else {
        ie_offset = OAL_FT_ACTION_IE_OFFSET;
        /* 在ft response中，header结尾到target ap addr的偏移量为8字节 */
        if ((msg_len < MAC_80211_FRAME_LEN + 8) || (msg_len < MAC_80211_FRAME_LEN + 8 + OAL_MAC_ADDR_LEN)) {
            oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_ft_notify_wpas::frame len[%d] invalid!}", msg_len);
            return OAL_FAIL;
        }
        target_ap_addr = mac_hdr + MAC_80211_FRAME_LEN + 8;

        memcpy_s(ft_event->auc_bssid, OAL_MAC_ADDR_LEN, target_ap_addr, OAL_MAC_ADDR_LEN);
    }
    if (msg_len < ie_offset) {
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }
    ft_event->us_ft_ie_len = msg_len - ie_offset;
    /* 修改为hmac申请内存，wal释放
     * 避免hmac抛事件后netbuffer被释放，wal使用已经释放的内存
     */
    ft_ie_buff = oal_memalloc(ft_event->us_ft_ie_len);
    if (ft_ie_buff == NULL) {
        frw_event_free_m(event_mem);
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_roam_ft_notify_wpas::alloc ft_ie_buff fail.len [%d].}",
                       ft_event->us_ft_ie_len);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    memcpy_s(ft_ie_buff, ft_event->us_ft_ie_len, mac_hdr + ie_offset, ft_event->us_ft_ie_len);

    ft_event->puc_ft_ie_buff = ft_ie_buff;

    /* 分发事件 */
    ret = frw_event_dispatch_event(event_mem);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_roam_ft_notify_wpas::frw_event_dispatch_event failed[%d].}", ret);
        oal_free(ft_ie_buff);
        ft_ie_buff = NULL;
    }

    frw_event_free_m(event_mem);
    return ret;
}


OAL_STATIC uint32_t hmac_roam_send_ft_req(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = NULL;
    oal_netbuf_stru *ft_frame = NULL;
    uint8_t *ft_buff = NULL;
    mac_tx_ctl_stru *tx_ctl = NULL;
    uint8_t *mac_addr = NULL;
    uint8_t *current_bssid = NULL;
    uint16_t ft_len;
    uint16_t app_ie_len;

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_UP,
                                        ROAM_MAIN_STATE_CONNECTING, ROAM_CONNECT_STATE_INIT);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_send_ft_req::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_vap = roam_info->pst_hmac_vap;
    hmac_user = roam_info->pst_hmac_user;

    if (hmac_vap->bit_11r_enable != OAL_TRUE) {
        return OAL_SUCC;
    }

    ft_frame = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (ft_frame == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_send_ft_req::oal_mem_netbuf_alloc fail.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ft_buff = (uint8_t *)oal_netbuf_header(ft_frame);
    memset_s(oal_netbuf_cb(ft_frame), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    memset_s(ft_buff, MAC_80211_FRAME_LEN, 0, MAC_80211_FRAME_LEN);

    mac_addr = mac_mib_get_StationID(&hmac_vap->st_vap_base_info);
    current_bssid = hmac_vap->st_vap_base_info.auc_bssid;
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
    mac_hdr_set_frame_control(ft_buff, WLAN_FC0_SUBTYPE_ACTION);
    /*  Set DA  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)ft_buff)->auc_address1, current_bssid);
    /*  Set SA  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)ft_buff)->auc_address2, mac_addr);
    /*  Set SSID  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)ft_buff)->auc_address3, current_bssid);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*                  FT Request Frame - Frame Body                          */
    /* --------------------------------------------------------------------- */
    /* | Category | Action | STA Addr |Target AP Addr | FT Req frame body  | */
    /* --------------------------------------------------------------------- */
    /* |     1    |   1    |     6    |       6       |       varibal      | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    ft_buff += MAC_80211_FRAME_LEN;
    ft_len = MAC_80211_FRAME_LEN;

    ft_buff[0] = MAC_ACTION_CATEGORY_FAST_BSS_TRANSITION;
    ft_buff[1] = MAC_FT_ACTION_REQUEST;
    ft_buff += 2; /* ft_buff += 2表示指针指向STA Addr字段 */
    ft_len += 2; /* 2表示帧体当前的长度 */

    oal_set_mac_addr(ft_buff, mac_addr);
    ft_buff += OAL_MAC_ADDR_LEN;
    ft_len += OAL_MAC_ADDR_LEN;

    oal_set_mac_addr(ft_buff, roam_info->st_connect.pst_bss_dscr->auc_bssid);
    ft_buff += OAL_MAC_ADDR_LEN;
    ft_len += OAL_MAC_ADDR_LEN;

    mac_add_app_ie(&hmac_vap->st_vap_base_info, ft_buff, &app_ie_len, OAL_APP_FT_IE);
    ft_len += app_ie_len;
    ft_buff += app_ie_len;

    oal_netbuf_put(ft_frame, ft_len);

    /* 为填写发送描述符准备参数 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(ft_frame);
    MAC_GET_CB_MPDU_LEN(tx_ctl) = ft_len;
    MAC_GET_CB_TX_USER_IDX(tx_ctl) = hmac_user->st_user_base_info.us_assoc_id;
    MAC_GET_CB_NETBUF_NUM(tx_ctl) = 1;

    /* 抛事件让dmac将该帧发送 */
    ret = hmac_tx_mgmt_send_event(&hmac_vap->st_vap_base_info, ft_frame, ft_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(ft_frame);
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_send_ft_req::hmac_tx_mgmt_send_event failed[%d].}", ret);
        return ret;
    }

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_FT_COMP);

    /* 启动认证超时定时器 */
    hmac_roam_connect_start_timer(roam_info, ROAM_AUTH_TIME_MAX);

    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_roam_process_ft_rsp(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;
    hmac_vap_stru *hmac_vap = NULL;
    dmac_wlan_crx_event_stru *crx_event = NULL;
    mac_rx_ctl_stru *rx_ctrl = NULL;
    uint8_t *mac_hdr = NULL;
    uint8_t *ft_frame_body = NULL;
    uint16_t auth_status;

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_UP,
                                        ROAM_MAIN_STATE_CONNECTING, ROAM_CONNECT_STATE_WAIT_FT_COMP);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_process_ft_rsp::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_vap = roam_info->pst_hmac_vap;

    if (hmac_vap->bit_11r_enable != OAL_TRUE) {
        return OAL_SUCC;
    }

    crx_event = (dmac_wlan_crx_event_stru *)param;
    rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(crx_event->pst_netbuf);
    mac_hdr = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(rx_ctrl);
    hmac_roam_connect_pm_wkup();

    /* 只处理action帧 */
    if (mac_get_frame_type_and_subtype(mac_hdr) != (WLAN_FC0_SUBTYPE_ACTION | WLAN_FC0_TYPE_MGT)) {
        return OAL_SUCC;
    }

    ft_frame_body = mac_hdr + MAC_80211_FRAME_LEN;

    if ((ft_frame_body[0] != MAC_ACTION_CATEGORY_FAST_BSS_TRANSITION) ||
        (ft_frame_body[1] != MAC_FT_ACTION_RESPONSE)) {
        return OAL_SUCC;
    }

    auth_status = mac_get_ft_status(mac_hdr);
    if (auth_status != MAC_SUCCESSFUL_STATUSCODE) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_process_ft_rsp:: status code[%d], change to ft over the air!}", auth_status);
        roam_info->st_connect.uc_ft_force_air = OAL_TRUE;
        roam_info->st_connect.uc_ft_failed = OAL_TRUE;
        ret = hmac_roam_connect_ft_ds_change_to_air(hmac_vap, roam_info->st_connect.pst_bss_dscr);
        if (ret != OAL_SUCC) {
            oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                "{hmac_roam_process_ft_rsp::hmac_roam_connect_ft_ds_change_to_air failed[%d].}", ret);
            return hmac_roam_connect_fail(roam_info);
        }
        return OAL_SUCC;
    }

    roam_info->st_static.uc_roam_mode = HMAC_CHR_OVER_DS;

    /* 上报FT成功消息给APP，以便APP下发新的FT_IE用于发送reassociation */
    ret = hmac_roam_ft_notify_wpas(hmac_vap, mac_hdr, rx_ctrl->us_frame_len);
    if (ret != OAL_SUCC) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_process_ft_rsp::hmac_roam_ft_notify_wpas failed[%d].}", ret);
        return ret;
    }

    if (hmac_vap->bit_11r_private_preauth == OAL_TRUE) {
        hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_PREAUTH_COMP);
    } else {
        hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_ASSOC_COMP);
    }

    /* 启动关联超时定时器 */
    hmac_roam_connect_start_timer(roam_info, ROAM_ASSOC_TIME_MAX);
    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_roam_encap_ft_preauth_req(hmac_roam_info_stru *roam_info,
                                                   oal_netbuf_stru *ft_frame,
                                                   uint16_t *ft_len)
{
    uint8_t *ft_buff = NULL;
    uint8_t *my_mac_addr = NULL;
    uint8_t *current_bssid = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    uint16_t app_ie_len;

    if (roam_info == NULL || ft_frame == NULL || ft_len == NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_encap_ft_preauth_req::NULL pointer}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = roam_info->pst_hmac_vap;
    ft_buff = (uint8_t *)oal_netbuf_header(ft_frame);
    memset_s(oal_netbuf_cb(ft_frame), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    memset_s(ft_buff, MAC_80211_FRAME_LEN, 0, MAC_80211_FRAME_LEN);

    my_mac_addr     = mac_mib_get_StationID(&hmac_vap->st_vap_base_info);
    current_bssid   = hmac_vap->st_vap_base_info.auc_bssid;
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
    mac_hdr_set_frame_control(ft_buff, WLAN_FC0_SUBTYPE_ACTION);
    /*  Set DA  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)ft_buff)->auc_address1, current_bssid);
    /*  Set SA  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)ft_buff)->auc_address2, my_mac_addr);
    /*  Set SSID  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)ft_buff)->auc_address3, current_bssid);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*                  FT Request Frame - Frame Body                        */
    /* --------------------------------------------------------------------- */
    /* | Category | Action | STA Addr |Target AP Addr | FT Req frame body  | */
    /* --------------------------------------------------------------------- */
    /* |     1    |   1    |     6    |       6       |       varibal      | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    ft_buff += MAC_80211_FRAME_LEN;
    *ft_len = MAC_80211_FRAME_LEN;

    ft_buff[0] = MAC_ACTION_CATEGORY_FAST_BSS_TRANSITION;
    ft_buff[1] = MAC_FT_ACTION_PREAUTH_REQUEST;
    ft_buff += 2; /* ft_buff += 2表示指针指向STA Addr字段 */
    *ft_len += 2; /* 2表示帧体当前的长度 */

    oal_set_mac_addr(ft_buff, my_mac_addr);
    ft_buff += OAL_MAC_ADDR_LEN;
    *ft_len += OAL_MAC_ADDR_LEN;

    oal_set_mac_addr(ft_buff, roam_info->st_connect.pst_bss_dscr->auc_bssid);
    ft_buff += OAL_MAC_ADDR_LEN;
    *ft_len += OAL_MAC_ADDR_LEN;

    mac_add_app_ie(&hmac_vap->st_vap_base_info, ft_buff, &app_ie_len, OAL_APP_FT_IE);
    *ft_len += app_ie_len;
    ft_buff += app_ie_len;

    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_roam_send_ft_preauth_req(hmac_roam_info_stru *roam_info)
{
    uint32_t              ret;
    hmac_vap_stru        *hmac_vap = NULL;
    hmac_user_stru       *hmac_user = NULL;
    oal_netbuf_stru      *ft_frame = NULL;
    mac_tx_ctl_stru      *tx_ctl = NULL;
    uint16_t              ft_len = 0;

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_CONNECTING,
                                        ROAM_CONNECT_STATE_WAIT_PREAUTH_COMP);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_send_ft_preauth_req::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_vap  = roam_info->pst_hmac_vap;
    hmac_user = roam_info->pst_hmac_user;

    if (hmac_vap->bit_11r_enable != OAL_TRUE || hmac_vap->bit_11r_private_preauth != OAL_TRUE) {
        return OAL_SUCC;
    }

    ft_frame = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (ft_frame == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_send_ft_preauth_req::oal_mem_netbuf_alloc fail.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_roam_encap_ft_preauth_req(roam_info, ft_frame, &ft_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(ft_frame);
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_send_ft_preauth_req::encap ft preauth FAILED}");
        return ret;
    }

    oal_netbuf_put(ft_frame, ft_len);

    /* 为填写发送描述符准备参数 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(ft_frame);
    MAC_GET_CB_MPDU_LEN(tx_ctl) = ft_len;
    MAC_GET_CB_TX_USER_IDX(tx_ctl) = hmac_user->st_user_base_info.us_assoc_id;
    MAC_GET_CB_NETBUF_NUM(tx_ctl) = 1;

    oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_send_ft_preauth_req::sending preauth request}");

    /* 抛事件让dmac将该帧发送 */
    if (hmac_tx_mgmt_send_event(&hmac_vap->st_vap_base_info, ft_frame, ft_len) != OAL_SUCC) {
        oal_netbuf_free(ft_frame);
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_send_ft_preauth_req::send event failed}");
        return ret;
    }

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_PREAUTH_COMP);

    /* 启动认证超时定时器 */
    hmac_roam_connect_start_timer(roam_info, ROAM_AUTH_TIME_MAX);

    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_roam_process_ft_preauth_rsp(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t                   ret;
    hmac_vap_stru             *hmac_vap = NULL;
    mac_rx_ctl_stru           *rx_ctrl = NULL;
    uint8_t                   *mac_hdr = NULL;
    uint8_t                   *ft_frame_body = NULL;

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_UP,
                                        ROAM_MAIN_STATE_CONNECTING, ROAM_CONNECT_STATE_WAIT_PREAUTH_COMP);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_process_ft_preauth_rsp::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_vap = roam_info->pst_hmac_vap;
    if (hmac_vap->bit_11r_enable != OAL_TRUE || hmac_vap->bit_11r_private_preauth != OAL_TRUE) {
        return OAL_SUCC;
    }

    if (param == NULL) {
        return OAL_FAIL;
    }
    rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(((dmac_wlan_crx_event_stru *)param)->pst_netbuf);
    mac_hdr = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(rx_ctrl);
    hmac_roam_connect_pm_wkup();

    /* 只处理action帧 */
    if ((WLAN_FC0_SUBTYPE_ACTION | WLAN_FC0_TYPE_MGT) != mac_get_frame_type_and_subtype(mac_hdr)) {
        return OAL_SUCC;
    }

    ft_frame_body = mac_hdr + MAC_80211_FRAME_LEN;
    if ((ft_frame_body[0] != MAC_ACTION_CATEGORY_FAST_BSS_TRANSITION) ||
        (ft_frame_body[1] != MAC_FT_ACTION_PREAUTH_RESPONSE)) {
        return OAL_SUCC;
    }

    if (mac_get_ft_status(mac_hdr) != MAC_SUCCESSFUL_STATUSCODE) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_process_ft_preauth_rsp::bad status,change to ft}");
    }

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_ASSOC_COMP);
    hmac_roam_connect_start_timer(roam_info, ROAM_ASSOC_TIME_MAX);
    if (hmac_roam_reassoc(hmac_vap) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_process_ft_preauth_rsp::reassoc failed}");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_11R


OAL_STATIC uint32_t hmac_roam_set_owe_dh_ie(mac_vap_stru *mac_vap, uint8_t *buffer,
                                            uint8_t *buffer_len, uint32_t buffer_max_len)
{
    uint8_t *app_ie = NULL;
    uint32_t app_ie_len;
    uint8_t *ie = NULL;

    app_ie = mac_vap->ast_app_ie[OAL_APP_ASSOC_REQ_IE].puc_ie;
    app_ie_len = mac_vap->ast_app_ie[OAL_APP_ASSOC_REQ_IE].ie_len;
    ie = mac_find_ie_ext_ie(MAC_EID_EXTENSION, MAC_EID_EXT_OWE_DH_PARAM, app_ie, app_ie_len);
    if (ie == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ROAM, "hmac_roam_set_owe_dh_ie::not find owe dh param");
        return OAL_FAIL;
    }

    if (*buffer_len + ie[1] + MAC_IE_HDR_LEN > buffer_max_len) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ROAM, "hmac_roam_set_owe_dh_ie::app ie len exceed limit");
        return OAL_FAIL;
    }

    if (memcpy_s(buffer + *buffer_len, WLAN_WPS_IE_MAX_SIZE - *buffer_len,
        ie, ie[1] + MAC_IE_HDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ROAM, "hmac_roam_set_owe_dh_ie::memcpy fail!");
        return OAL_FAIL;
    }
    *buffer_len += ie[1] + MAC_IE_HDR_LEN;

    return OAL_SUCC;
}

OAL_STATIC void hmac_roam_set_app_ie(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr)
{
    uint32_t ret;
    uint8_t ie_len = 0;
    uint8_t *pmkid = NULL;
    uint32_t rsn_akm_suites[WLAN_PAIRWISE_CIPHER_SUITES] = { 0 };
    oal_app_ie_stru app_ie;

    if (hmac_vap->st_vap_base_info.st_cap_flag.bit_wpa == OAL_TRUE) {
        /* 设置 WPA Capability IE */
        ie_len = mac_set_wpa_ie(&hmac_vap->st_vap_base_info, app_ie.auc_ie);
    }

    if (hmac_vap->st_vap_base_info.st_cap_flag.bit_wpa2 == OAL_TRUE) {
        /* 设置 RSN Capability IE */
        pmkid = hmac_vap_get_pmksa(hmac_vap, bss_dscr->auc_bssid);
        ie_len = mac_set_rsn_ie(&hmac_vap->st_vap_base_info, pmkid, app_ie.auc_ie);

        /* 设置OWE DH参数     */
        mac_mib_get_rsn_akm_suites_s(&hmac_vap->st_vap_base_info, rsn_akm_suites, sizeof(rsn_akm_suites));
        if (is_support_owe((uint32_t *)rsn_akm_suites)) {
            hmac_roam_set_owe_dh_ie(&hmac_vap->st_vap_base_info, app_ie.auc_ie, &ie_len, WLAN_WPS_IE_MAX_SIZE);
        }
    }

    if (ie_len != 0) {
        app_ie.en_app_ie_type = OAL_APP_REASSOC_REQ_IE;
        app_ie.ie_len = ie_len;
        ret = hmac_config_set_app_ie_to_vap(&hmac_vap->st_vap_base_info, &app_ie, OAL_APP_REASSOC_REQ_IE);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ROAM,
                "{hmac_roam_set_app_ie::hmac_config_set_app_ie_to_vap fail[%d].}", ret);
        }
    } else {
        mac_vap_clear_app_ie(&hmac_vap->st_vap_base_info, OAL_APP_REASSOC_REQ_IE);
    }
    return;
}


OAL_STATIC uint32_t hmac_roam_start_join(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;
    hmac_vap_stru *hmac_vap = roam_info->pst_hmac_vap;
    mac_bss_dscr_stru *bss_dscr = (mac_bss_dscr_stru *)param;
    hmac_join_req_stru join_req;
    uint8_t rate_num;

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_ROAMING,
                                        ROAM_MAIN_STATE_CONNECTING, ROAM_CONNECT_STATE_INIT);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_start_join::check_state fail[%d]!}", ret);
        return ret;
    }

    rate_num = (bss_dscr->uc_num_supp_rates < WLAN_MAX_SUPP_RATES) ? bss_dscr->uc_num_supp_rates : WLAN_MAX_SUPP_RATES;
    if (memcpy_s(hmac_vap->auc_supp_rates, WLAN_MAX_SUPP_RATES, bss_dscr->auc_supp_rates, rate_num) != EOK) {
        oam_error_log0(0, OAM_SF_ROAM, "hmac_roam_start_join::memcpy fail!");
        return OAL_FAIL;
    }
    mac_mib_set_SupportRateSetNums(&hmac_vap->st_vap_base_info, bss_dscr->uc_num_supp_rates);

    /* 配置join参数 */
    hmac_prepare_join_req(&join_req, bss_dscr);
    ret = hmac_sta_update_join_req_params(hmac_vap, &join_req);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_start_join::hmac_sta_update_join_req_params fail[%d].}", ret);
        return ret;
    }

    hmac_roam_set_app_ie(hmac_vap, bss_dscr);

    ret = hmac_roam_connect_set_dtim_param(&hmac_vap->st_vap_base_info, bss_dscr->uc_dtim_cnt,
                                           bss_dscr->uc_dtim_period);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_start_join::hmac_roam_connect_set_dtim_param fail[%d].}", ret);
        return ret;
    }

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_JOIN);

    ret = hmac_roam_send_auth_seq1(roam_info);
    if (ret != OAL_SUCC) {
        hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_FAIL);
        /* 通知ROAM主状态机 */
        hmac_roam_connect_complete(hmac_vap, OAL_FAIL);
        oam_warning_log1(0, OAM_SF_SCAN, "{hmac_roam_process_beacon::hmac_roam_send_auth_seq1 fail[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_SAE

OAL_STATIC uint32_t hmac_roam_triger_sae_auth(hmac_roam_info_stru *roam_info)
{
    hmac_vap_stru *hmac_vap = roam_info->pst_hmac_vap;
    hmac_user_stru *hmac_user = roam_info->pst_hmac_user;
    uint8_t vap_id;

    vap_id = hmac_vap->st_vap_base_info.uc_vap_id;

    oam_warning_log0(vap_id, OAM_SF_ROAM, "hmac_roam_triger_sae_auth:: triger sae auth");

    /* 置位初始值 */
    hmac_vap->duplicate_auth_seq2_flag = OAL_FALSE;
    hmac_vap->duplicate_auth_seq4_flag = OAL_FALSE;

    oal_set_mac_addr(hmac_user->st_user_base_info.auc_user_mac_addr,
                     roam_info->st_connect.pst_bss_dscr->auc_bssid);

    hmac_report_external_auth_req(hmac_vap, NL80211_EXTERNAL_AUTH_START);

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_AUTH_COMP);

    hmac_roam_connect_start_timer(roam_info, ROAM_AUTH_TIME_MAX * 2); /* 2表示sae auth下auth超时时间翻倍 */

    return OAL_SUCC;
}


uint32_t hmac_roam_sae_config_reassoc_req(hmac_vap_stru *hmac_vap)
{
    uint8_t vap_id = hmac_vap->st_vap_base_info.uc_vap_id;
    hmac_roam_info_stru *roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    uint32_t ret;

    if (roam_info == NULL) {
        oam_error_log0(vap_id, OAM_SF_ROAM, "{hmac_roam_sae_config_reassoc_req::roam_info null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    ret = hmac_roam_send_reassoc_req(roam_info);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_sae_config_reassoc_req::send_reassoc_req failed[%d].}", ret);
        return ret;
    }
    return OAL_SUCC;
}

#endif


OAL_STATIC uint32_t hmac_roam_send_auth_seq1(hmac_roam_info_stru *roam_info)
{
    uint32_t ret;
    hmac_vap_stru *hmac_vap = roam_info->pst_hmac_vap;
    hmac_user_stru *hmac_user = roam_info->pst_hmac_user;
    oal_netbuf_stru *auth_frame = NULL;
    mac_tx_ctl_stru *tx_ctl = NULL;
    uint16_t auth_len;
    uint8_t vap_id;

#ifdef _PRE_WLAN_FEATURE_SAE
    if (hmac_vap->en_sae_connect == OAL_TRUE) {
        ret = hmac_roam_triger_sae_auth(roam_info);
        return ret;
    }
#endif

    vap_id = hmac_vap->st_vap_base_info.uc_vap_id;

    auth_frame = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (auth_frame == NULL) {
        oam_error_log0(vap_id, OAM_SF_ROAM, "{hmac_roam_send_auth_seq1::oal_mem_netbuf_alloc fail.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    oal_mem_netbuf_trace(auth_frame, OAL_TRUE);

    memset_s(oal_netbuf_cb(auth_frame), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    memset_s((uint8_t *)oal_netbuf_header(auth_frame), MAC_80211_FRAME_LEN, 0, MAC_80211_FRAME_LEN);

    /* 更新用户mac */
    oal_set_mac_addr(hmac_user->st_user_base_info.auc_user_mac_addr,
                     roam_info->st_connect.pst_bss_dscr->auc_bssid);

    auth_len = hmac_mgmt_encap_auth_req(hmac_vap, (uint8_t *)(oal_netbuf_header(auth_frame)));
    if (auth_len < OAL_AUTH_IE_OFFSET) {
        oam_warning_log0(vap_id, OAM_SF_ROAM, "{hmac_roam_send_auth_seq1::hmac_mgmt_encap_auth_req failed.}");
        oal_netbuf_free(auth_frame);
        return OAL_ERR_CODE_ROAM_FRAMER_LEN;
    }

    oal_netbuf_put(auth_frame, auth_len);

    /* 为填写发送描述符准备参数 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(auth_frame);
    MAC_GET_CB_MPDU_LEN(tx_ctl) = auth_len;
    MAC_GET_CB_TX_USER_IDX(tx_ctl) = hmac_user->st_user_base_info.us_assoc_id;
    MAC_GET_CB_NETBUF_NUM(tx_ctl) = 1;

    /* 抛事件让dmac将该帧发送 */
    ret = hmac_tx_mgmt_send_event(&hmac_vap->st_vap_base_info, auth_frame, auth_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(auth_frame);
        oam_error_log1(vap_id, OAM_SF_ROAM,
                       "{hmac_roam_send_auth_seq1::hmac_tx_mgmt_send_event failed[%d].}", ret);
        return ret;
    }

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_AUTH_COMP);

    /* 启动认证超时定时器 */
    hmac_roam_connect_start_timer(roam_info, ROAM_AUTH_TIME_MAX);

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_roam_send_reassoc_req(hmac_roam_info_stru *roam_info)
{
    uint32_t ret;
    hmac_vap_stru *hmac_vap = roam_info->pst_hmac_vap;
    hmac_user_stru *hmac_user = roam_info->pst_hmac_user;
    oal_netbuf_stru *assoc_req_frame = NULL;
    mac_tx_ctl_stru *tx_ctl = NULL;
    uint32_t assoc_len;
    mac_vap_stru *mac_vap = &hmac_vap->st_vap_base_info;

    if (hmac_user == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ROAM, "{hmac_roam_send_reassoc_req::hmac_user NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    assoc_req_frame = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (assoc_req_frame == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ROAM, "{hmac_roam_send_reassoc_req::alloc assoc_req_frame NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_mem_netbuf_trace(assoc_req_frame, OAL_TRUE);

    memset_s(oal_netbuf_cb(assoc_req_frame), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    /* 将mac header清零 */
    memset_s((uint8_t *)oal_netbuf_header(assoc_req_frame), MAC_80211_FRAME_LEN, 0, MAC_80211_FRAME_LEN);

    hmac_vap->bit_reassoc_flag = OAL_TRUE;

    assoc_len = hmac_mgmt_encap_asoc_req_sta(hmac_vap, (uint8_t *)(oal_netbuf_header(assoc_req_frame)),
                                             roam_info->st_old_bss.auc_bssid);

    oal_netbuf_put(assoc_req_frame, assoc_len);

    /* 帧长异常 */
    if (assoc_len <= OAL_ASSOC_REQ_IE_OFFSET) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ROAM, "{hmac_roam_send_reassoc_req::err assoc len[%d].}", assoc_len);
        oal_netbuf_free(assoc_req_frame);
        return OAL_FAIL;
    }
    hmac_user_free_asoc_req_ie(hmac_user->st_user_base_info.us_assoc_id);

    /* 记录关联请求帧的部分内容，用于上报给内核 */
    ret = hmac_user_set_asoc_req_ie(hmac_user, oal_netbuf_header(assoc_req_frame) + OAL_ASSOC_REQ_IE_OFFSET,
                                    assoc_len - OAL_ASSOC_REQ_IE_OFFSET, OAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ROAM, "{hmac_roam_send_reassoc_req::hmac_user_set_asoc_req_ie fail}");
        oal_netbuf_free(assoc_req_frame);
        return OAL_FAIL;
    }

    /* 为填写发送描述符准备参数 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(assoc_req_frame);
    MAC_GET_CB_MPDU_LEN(tx_ctl) = (uint16_t)assoc_len;
    MAC_GET_CB_TX_USER_IDX(tx_ctl) = hmac_user->st_user_base_info.us_assoc_id;
    MAC_GET_CB_NETBUF_NUM(tx_ctl) = 1;

    /* 抛事件让dmac将该帧发送 */
    ret = hmac_tx_mgmt_send_event(&hmac_vap->st_vap_base_info, assoc_req_frame, (uint16_t)assoc_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(assoc_req_frame);
        hmac_user_free_asoc_req_ie(hmac_user->st_user_base_info.us_assoc_id);
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_send_reassoc_req::hmac_tx_mgmt_send_event failed[%d].}", ret);
        return ret;
    }

    /* 启动关联超时定时器 */
    hmac_roam_connect_start_timer(roam_info, ROAM_ASSOC_TIME_MAX);

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_ASSOC_COMP);

    return OAL_SUCC;
}

uint32_t hmac_roam_rx_auth_pre_check(uint8_t *mac_hdr, hmac_user_stru *hmac_user)
{
    uint8_t bssid[WLAN_MAC_ADDR_LEN] = { 0 };
    uint8_t frame_sub_type = mac_get_frame_type_and_subtype(mac_hdr);

    mac_get_address3(mac_hdr, bssid, WLAN_MAC_ADDR_LEN);
    if (oal_memcmp(hmac_user->st_user_base_info.auc_user_mac_addr, bssid, sizeof(bssid))) {
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    if (wlan_pm_wkup_src_debug_get() == OAL_TRUE) {
        wlan_pm_wkup_src_debug_set(OAL_FALSE);
        oam_warning_log1(0, OAM_SF_RX, "{wifi_wk_src:hmac_roam_proc_auth_seq2:wkup mgmt type[0x%x]}", frame_sub_type);
    }
#endif

    /* auth_seq2帧校验，错误帧不处理，在超时中统一处理 */
    if (frame_sub_type != (WLAN_FC0_SUBTYPE_AUTH | WLAN_FC0_TYPE_MGT)) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_SAE

OAL_STATIC uint32_t hmac_roam_rx_auth_check_sae(hmac_vap_stru *hmac_vap, dmac_wlan_crx_event_stru *crx_event,
    uint16_t auth_status, uint16_t auth_seq_num)
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
#endif

#ifdef _PRE_WLAN_FEATURE_11R
OAL_STATIC uint32_t hmac_roam_rx_auth_handle_11r(hmac_vap_stru *hmac_vap, uint8_t *mac_hdr,
    mac_rx_ctl_stru *rx_ctrl, hmac_roam_info_stru *roam_info)
{
    uint32_t ret;

    roam_info->st_static.uc_roam_mode = HMAC_CHR_OVER_THE_AIR;
    /* 上报FT成功消息给APP，以便APP下发新的FT_IE用于发送reassociation */
    ret = hmac_roam_ft_notify_wpas(hmac_vap, mac_hdr, rx_ctrl->us_frame_len);
    if (ret != OAL_SUCC) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_process_auth_seq2::hmac_roam_ft_notify_wpas failed[%d].}", ret);
        return ret;
    }

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_ASSOC_COMP);
    /* 启动关联超时定时器 */
    hmac_roam_connect_start_timer(roam_info, ROAM_ASSOC_TIME_MAX);
    return OAL_SUCC;
}
#endif


OAL_STATIC uint32_t hmac_roam_process_auth_seq2(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;
    hmac_vap_stru *hmac_vap = roam_info->pst_hmac_vap;
    hmac_user_stru *hmac_user = roam_info->pst_hmac_user;
    dmac_wlan_crx_event_stru *crx_event = (dmac_wlan_crx_event_stru *)param;
    mac_rx_ctl_stru *rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(crx_event->pst_netbuf);
    uint8_t *mac_hdr = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(rx_ctrl);
    uint16_t auth_status = mac_get_auth_status(mac_hdr);
    uint16_t auth_seq_num = mac_get_auth_seq_num(mac_hdr);

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING,
                                        ROAM_CONNECT_STATE_WAIT_AUTH_COMP);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_process_auth_seq2::check_state fail[%d]!}", ret);
        return ret;
    }

    ret = hmac_roam_rx_auth_pre_check(mac_hdr, hmac_user);
    if (ret == OAL_FAIL) {
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_FEATURE_SAE
    if (hmac_vap->en_sae_connect == OAL_TRUE) {
        return hmac_roam_rx_auth_check_sae(hmac_vap, crx_event, auth_status, auth_seq_num);
    }
#endif

    if ((auth_seq_num != WLAN_AUTH_TRASACTION_NUM_TWO) || (auth_status != MAC_SUCCESSFUL_STATUSCODE)) {
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_FEATURE_11R
    if (hmac_vap->bit_11r_enable == OAL_TRUE) {
        if (mac_get_auth_alg(mac_hdr) == WLAN_WITP_AUTH_FT) {
            return hmac_roam_rx_auth_handle_11r(hmac_vap, mac_hdr, rx_ctrl, roam_info);
        }
    }
#endif  // _PRE_WLAN_FEATURE_11R

    if (mac_get_auth_alg(mac_hdr) != WLAN_WITP_AUTH_OPEN_SYSTEM) {
        return OAL_SUCC;
    }

    /* 发送关联请求 */
    roam_info->st_static.uc_roam_mode = HMAC_CHR_ROAM_NORMAL;
    ret = hmac_roam_send_reassoc_req(roam_info);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_process_auth_seq2::hmac_roam_send_assoc_req failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11K
OAL_STATIC uint32_t  hmac_sta_up_send_neighbor_req(hmac_roam_info_stru *roam_info)
{
    wal_msg_write_stru             write_msg;
    int32_t                        ret;
    uint8_t                       *cur_ssid = NULL;
    mac_cfg_ssid_param_stru       *ssid = NULL;
    oal_net_device_stru           *net_dev = NULL;
    hmac_vap_stru                 *hmac_vap = NULL;

    hmac_vap = roam_info->pst_hmac_vap;
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_sta_up_send_neighbor_req::hmac_vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }
    net_dev = hmac_vap->pst_net_device;

    if (roam_info->pst_hmac_vap->bit_nb_rpt_11k == OAL_FALSE ||
        roam_info->en_roam_trigger != ROAM_TRIGGER_DMAC ||
        roam_info->pst_hmac_vap->bit_11k_enable == OAL_FALSE ||
        hmac_roam_is_neighbor_report_allowed(roam_info->pst_hmac_vap) == OAL_FALSE) {
        oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "{hmac_sta_up_send_neighbor_req::not allowed,nb_rpt_11k=[%d],11k_enable=[%d]!}",
                         roam_info->pst_hmac_vap->bit_nb_rpt_11k,
                         roam_info->pst_hmac_vap->bit_11k_enable);
        return OAL_FAIL;
    }

    cur_ssid = mac_mib_get_DesiredSSID(&(roam_info->pst_hmac_vap->st_vap_base_info));
    if (cur_ssid == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_sta_up_send_neighbor_req::puc_cur_ssid null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SEND_NEIGHBOR_REQ, sizeof(mac_cfg_ssid_param_stru));
    ssid = (mac_cfg_ssid_param_stru *)write_msg.auc_value;
    if (EOK != memcpy_s(ssid->ac_ssid, WLAN_SSID_MAX_LEN, cur_ssid, WLAN_SSID_MAX_LEN)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_sta_up_send_neighbor_req::memcpy ssid fail!}");
    }
    ssid->ac_ssid[WLAN_SSID_MAX_LEN - 1] = '\0';
    ssid->uc_ssid_len = OAL_STRLEN(ssid->ac_ssid);

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_ssid_param_stru),
                             (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_sta_up_send_neighbor_req::return err code [%d]!}\r\n", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC uint32_t hmac_roam_process_assoc_rsp(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;
    hmac_vap_stru *hmac_vap = roam_info->pst_hmac_vap;
    hmac_user_stru *hmac_user = roam_info->pst_hmac_user;
    dmac_wlan_crx_event_stru *crx_event = NULL;
    mac_rx_ctl_stru *rx_ctrl = NULL;
    uint8_t *mac_hdr = NULL;
    uint8_t *payload = NULL;
    uint16_t msg_len;
    uint16_t hdr_len;
    mac_status_code_enum_uint16 asoc_status;
    uint8_t *bss_addr = NULL;
    uint8_t frame_sub_type;

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_ROAMING,
                                        ROAM_MAIN_STATE_CONNECTING, ROAM_CONNECT_STATE_WAIT_ASSOC_COMP);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_process_assoc_rsp::check_state fail[%d]!}", ret);
        return ret;
    }

    crx_event = (dmac_wlan_crx_event_stru *)param;
    rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(crx_event->pst_netbuf);
    mac_hdr = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(rx_ctrl);
    payload = mac_hdr + rx_ctrl->uc_mac_header_len;
    msg_len = rx_ctrl->us_frame_len - rx_ctrl->uc_mac_header_len;
    hdr_len = rx_ctrl->uc_mac_header_len;

    /* mac地址校验 */
    bss_addr = mac_hdr + BYTE_OFFSET_16;
    if (oal_compare_mac_addr(hmac_user->st_user_base_info.auc_user_mac_addr, bss_addr)) {
        return OAL_SUCC;
    }

    /* assoc帧校验，错误帧处理 */
    frame_sub_type = mac_get_frame_type_and_subtype(mac_hdr);
    asoc_status = mac_get_asoc_status(payload);
    hmac_roam_connect_pm_wkup();

    if ((frame_sub_type != (WLAN_FC0_SUBTYPE_REASSOC_RSP | WLAN_FC0_TYPE_MGT)) &&
        (frame_sub_type != (WLAN_FC0_SUBTYPE_ASSOC_RSP | WLAN_FC0_TYPE_MGT))) {
        return OAL_SUCC;
    }

    /* 关联响应帧长度校验 */
    if (rx_ctrl->us_frame_len <= OAL_ASSOC_RSP_IE_OFFSET) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_process_assoc_rsp::rsp ie length error, frame_len[%d].}",
                         rx_ctrl->us_frame_len);
        return OAL_ERR_CODE_ROAM_FRAMER_LEN;
    }

    roam_info->st_connect.en_status_code = MAC_SUCCESSFUL_STATUSCODE;
    if (asoc_status != MAC_SUCCESSFUL_STATUSCODE) {
        roam_info->st_connect.en_status_code = asoc_status;
        return OAL_SUCC;
    }

    ret = hmac_process_assoc_rsp(hmac_vap, hmac_user, mac_hdr, hdr_len, payload, msg_len);
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_process_assoc_rsp::hmac_process_assoc_rsp failed[%d].}", ret);
        return ret;
    }

    /* user已经关联上，抛事件给DMAC，在DMAC层挂用户算法钩子 */
    hmac_user_add_notify_alg(&(hmac_vap->st_vap_base_info), hmac_user->st_user_base_info.us_assoc_id);

    /* 上报关联成功消息给APP */
    ret = hmac_roam_connect_notify_wpas(roam_info, mac_hdr, rx_ctrl->us_frame_len);
    if (ret != OAL_SUCC) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_process_assoc_rsp::hmac_roam_connect_notify_wpas failed[%d].}", ret);
        return ret;
    }

/* 在STA上线后立即发送NR Req以获取邻居AP channel信息 */
#ifdef _PRE_WLAN_FEATURE_11K
    ret = hmac_sta_up_send_neighbor_req(roam_info);
#endif

    if (mac_mib_get_privacyinvoked(&hmac_vap->st_vap_base_info) != OAL_TRUE) {
        /* 非加密情况下，漫游成功 */
        hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_HANDSHAKING);
        hmac_roam_connect_succ(roam_info, NULL);
    } else {
#ifdef _PRE_WLAN_FEATURE_11R
        if (hmac_vap->bit_11r_enable == OAL_TRUE) {
            if (mac_mib_get_ft_trainsistion(&hmac_vap->st_vap_base_info) == OAL_TRUE) {
                /* FT情况下，漫游成功 */
                hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_HANDSHAKING);
                hmac_roam_connect_succ(roam_info, NULL);
                return OAL_SUCC;
            }
        }
#endif  // _PRE_WLAN_FEATURE_11R
        if (mac_mib_get_rsnaactivated(&hmac_vap->st_vap_base_info) == OAL_TRUE) {
            hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_HANDSHAKING);
            /* 启动握手超时定时器 */
            hmac_roam_connect_start_timer(roam_info, ROAM_HANDSHAKE_TIME_MAX);
        } else {
            /* 非 WPA 或者 WPA2 加密情况下(WEP_OPEN/WEP_SHARED)，漫游成功 */
            hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_HANDSHAKING);
            hmac_roam_connect_succ(roam_info, NULL);
        }
    }

    return OAL_SUCC;
}

static uint32_t hmac_roam_process_addba_action(uint8_t *payload, hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, uint32_t frame_body_len)
{
    uint32_t ret = OAL_SUCC;
    oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
        "{hmac_roam_process_addba_action::BA_ACTION_TYPE [%d].}", payload[MAC_ACTION_OFFSET_ACTION]);
    switch (payload[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_BA_ACTION_ADDBA_REQ:
            ret = hmac_mgmt_rx_addba_req(hmac_vap, hmac_user, payload, frame_body_len);
            break;

        case MAC_BA_ACTION_ADDBA_RSP:
            ret = hmac_mgmt_rx_addba_rsp(hmac_vap, hmac_user, payload, frame_body_len);
            break;

        case MAC_BA_ACTION_DELBA:
            ret = hmac_mgmt_rx_delba(hmac_vap, hmac_user, payload, frame_body_len);
            break;

        default:
            break;
    }
    return ret;
}


OAL_STATIC uint32_t hmac_roam_process_action(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret, frame_body_len;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = NULL;
    dmac_wlan_crx_event_stru *crx_event = NULL;
    mac_rx_ctl_stru *rx_ctrl = NULL;
    uint8_t *mac_hdr = NULL;
    uint8_t *payload = NULL;
    uint8_t *bss_addr = NULL;
    uint8_t frame_sub_type;

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_ROAMING,
                                        ROAM_MAIN_STATE_CONNECTING, ROAM_CONNECT_STATE_HANDSHAKING);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_process_action::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_vap = roam_info->pst_hmac_vap;
    hmac_user = roam_info->pst_hmac_user;

    crx_event = (dmac_wlan_crx_event_stru *)param;
    rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(crx_event->pst_netbuf);
    mac_hdr = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(rx_ctrl);
    frame_body_len = hmac_get_frame_body_len(crx_event->pst_netbuf);
    if (frame_body_len < MAC_ACTION_CATEGORY_AND_CODE_LEN) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_process_action::frame_body_len[%d]}", frame_body_len);
        return OAL_FAIL;
    }
    payload = mac_hdr + rx_ctrl->uc_mac_header_len;

    /* mac地址校验 */
    bss_addr = mac_hdr + BYTE_OFFSET_16;
    if (oal_compare_mac_addr(hmac_user->st_user_base_info.auc_user_mac_addr, bss_addr)) {
        return OAL_SUCC;
    }

    frame_sub_type = mac_get_frame_type_and_subtype(mac_hdr);

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    if (wlan_pm_wkup_src_debug_get() == OAL_TRUE) {
        wlan_pm_wkup_src_debug_set(OAL_FALSE);
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
            "{wifi_wake_src:hmac_roam_process_auth_seq2::wakeup mgmt type[0x%x]}", frame_sub_type);
    }
#endif

    if (frame_sub_type != (WLAN_FC0_SUBTYPE_ACTION | WLAN_FC0_TYPE_MGT)) {
        return OAL_SUCC;
    }

    if (payload[MAC_ACTION_OFFSET_CATEGORY] == MAC_ACTION_CATEGORY_BA) {
        ret = hmac_roam_process_addba_action(payload, hmac_vap, hmac_user, frame_body_len);
    }

    return ret;
}


OAL_STATIC uint32_t hmac_roam_connect_succ(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_ROAMING,
                                        ROAM_MAIN_STATE_CONNECTING, ROAM_CONNECT_STATE_HANDSHAKING);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_connect_succ::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_UP);

    /* 删除定时器 */
    hmac_roam_connect_del_timer(roam_info);

    /* 通知ROAM主状态机 */
    hmac_roam_connect_complete(roam_info->pst_hmac_vap, OAL_SUCC);
    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_roam_auth_timeout(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;
    hmac_vap_stru *hmac_vap = NULL;

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_ROAMING,
                                        ROAM_MAIN_STATE_CONNECTING, ROAM_CONNECT_STATE_WAIT_AUTH_COMP);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_auth_timeout::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_vap = roam_info->pst_hmac_vap;

    if (++roam_info->st_connect.uc_auth_num >= MAX_AUTH_CNT) {
        return hmac_roam_connect_fail(roam_info);
    }

#ifdef _PRE_WLAN_FEATURE_SAE
    if (hmac_vap->en_sae_connect == OAL_TRUE) {
        ret = hmac_roam_triger_sae_auth(roam_info);
        return ret;
    }
#endif

    ret = hmac_roam_send_auth_seq1(roam_info);
    if (ret != OAL_SUCC) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_auth_timeout::hmac_roam_send_auth_seq1 failed[%d].}", ret);
    }

    return ret;
}


OAL_STATIC uint32_t hmac_roam_assoc_timeout(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_ROAMING,
                                        ROAM_MAIN_STATE_CONNECTING, ROAM_CONNECT_STATE_WAIT_ASSOC_COMP);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_assoc_timeout::check_state fail[%d]!}", ret);
        return ret;
    }

    if ((++roam_info->st_connect.uc_assoc_num >= MAX_ASOC_CNT) ||
        (roam_info->st_connect.en_status_code == MAC_REJECT_TEMP)) {
        return hmac_roam_connect_fail(roam_info);
    }

    ret = hmac_roam_send_reassoc_req(roam_info);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_assoc_timeout::hmac_roam_send_reassoc_req failed[%d].}", ret);
    }
    return ret;
}


OAL_STATIC uint32_t hmac_roam_handshaking_timeout(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_ROAMING,
                                        ROAM_MAIN_STATE_CONNECTING, ROAM_CONNECT_STATE_HANDSHAKING);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_handshaking_timeout::check_state fail[%d]!}", ret);
        return ret;
    }

    return hmac_roam_connect_fail(roam_info);
}

#ifdef _PRE_WLAN_FEATURE_11R

OAL_STATIC uint32_t hmac_roam_ft_timeout(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t ret;
    hmac_vap_stru *hmac_vap = NULL;

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_UP,
                                        ROAM_MAIN_STATE_CONNECTING, ROAM_CONNECT_STATE_WAIT_FT_COMP);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_ft_timeout::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_vap = roam_info->pst_hmac_vap;

    if (hmac_vap->bit_11r_enable != OAL_TRUE) {
        return OAL_SUCC;
    }

    if (++roam_info->st_connect.uc_ft_num >= MAX_AUTH_CNT) {
        return hmac_roam_connect_fail(roam_info);
    }

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_INIT);

    oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                     "{hmac_roam_ft_timeout::change to ft over the air!}");
    roam_info->st_connect.uc_ft_force_air = OAL_TRUE;
    roam_info->st_connect.uc_ft_failed = OAL_TRUE;
    ret = hmac_roam_connect_ft_ds_change_to_air(hmac_vap, roam_info->st_connect.pst_bss_dscr);
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_ft_timeout::hmac_roam_connect_ft_ds_change_to_air failed[%d].}", ret);
        return hmac_roam_connect_fail(roam_info);
    }
    return ret;
}


OAL_STATIC uint32_t hmac_roam_ft_preauth_timeout(hmac_roam_info_stru *roam_info, void *param)
{
    uint32_t             ret;
    hmac_vap_stru       *hmac_vap = NULL;

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_UP,
        ROAM_MAIN_STATE_CONNECTING, ROAM_CONNECT_STATE_WAIT_PREAUTH_COMP);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_ft_preauth_timeout::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_vap = roam_info->pst_hmac_vap;

    if (hmac_vap->bit_11r_enable != OAL_TRUE) {
        return OAL_SUCC;
    }

    if (++roam_info->st_connect.uc_ft_num >= MAX_PREAUTH_CNT) {
        return hmac_roam_connect_fail(roam_info);
    }

    oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                     "{hmac_roam_ft_preauth_timeout::change to ft over ds!}");
    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_ASSOC_COMP);
    hmac_roam_connect_start_timer(roam_info, ROAM_ASSOC_TIME_MAX);
    if (hmac_roam_reassoc(hmac_vap) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_process_ft_preauth_rsp::reassoc failed}");
        return OAL_FAIL;
    }
    return ret;
}

#endif  // _PRE_WLAN_FEATURE_11R

OAL_STATIC uint32_t hmac_roam_connect_fail(hmac_roam_info_stru *roam_info)
{
    hmac_vap_stru *hmac_vap = roam_info->pst_hmac_vap;
    roam_connect_state_enum connect_state = roam_info->st_connect.en_state;
    /* connect状态切换 */
    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_FAIL);

    /* connect失败时，需要添加到黑名单 */
    hmac_roam_alg_add_blacklist(roam_info, roam_info->st_connect.pst_bss_dscr->auc_bssid,
                                ROAM_BLACKLIST_TYPE_REJECT_AP);
    oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                     "{hmac_roam_connect_fail::hmac_roam_alg_add_blacklist!}");

    /* 通知ROAM主状态机，BSS回退由主状态机完成 */
    if (connect_state == ROAM_CONNECT_STATE_HANDSHAKING) {
        hmac_roam_connect_complete(hmac_vap, OAL_ERR_CODE_ROAM_HANDSHAKE_FAIL);
    } else if (connect_state == ROAM_CONNECT_STATE_WAIT_ASSOC_COMP ||
               connect_state == ROAM_CONNECT_STATE_WAIT_AUTH_COMP) {
        hmac_roam_connect_complete(hmac_vap, OAL_ERR_CODE_ROAM_NO_RESPONSE);
    } else {
        hmac_roam_connect_complete(hmac_vap, OAL_FAIL);
    }

    return OAL_SUCC;
}

uint32_t hmac_roam_connect_start(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr)
{
    hmac_roam_info_stru *roam_info = NULL;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_start::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_start::roam_info null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 漫游开关没有开时，不处理tbtt中断 */
    if (roam_info->uc_enable == 0) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_start::roam disabled!}");
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    roam_info->st_connect.pst_bss_dscr = bss_dscr;
    roam_info->st_connect.uc_auth_num = 0;
    roam_info->st_connect.uc_assoc_num = 0;
    roam_info->st_connect.uc_ft_num = 0;

#ifdef _PRE_WLAN_FEATURE_11R
    if (hmac_vap->bit_11r_enable == OAL_TRUE) {
        if ((mac_mib_get_ft_trainsistion(&hmac_vap->st_vap_base_info) == OAL_TRUE &&
            mac_mib_get_ft_over_ds(&hmac_vap->st_vap_base_info)) &&
            (hmac_vap->bit_11r_over_ds == OAL_TRUE) &&
            (roam_info->st_connect.uc_ft_force_air != OAL_TRUE)) {
            return hmac_roam_connect_fsm_action(roam_info, ROAM_CONNECT_FSM_EVENT_FT_OVER_DS,
                                                (void *)bss_dscr);
        }
    } else {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_start::11r NOT Enabled}");
    }
#endif  // _PRE_WLAN_FEATURE_11R

    return hmac_roam_connect_fsm_action(roam_info, ROAM_CONNECT_FSM_EVENT_START, (void *)bss_dscr);
}

uint32_t hmac_roam_connect_stop(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru *roam_info = NULL;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_start::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_start::roam_info null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }
    roam_info->st_connect.en_state = ROAM_CONNECT_STATE_INIT;
    return OAL_SUCC;
}

void hmac_roam_connect_rx_mgmt(hmac_vap_stru *hmac_vap, dmac_wlan_crx_event_stru *crx_event)
{
    hmac_roam_info_stru *roam_info = NULL;
    uint32_t ret;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_rx_mgmt::vap null!}");
        return;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        return;
    }

    /* 漫游开关没有开时，不处理管理帧接收 */
    if (roam_info->uc_enable == 0) {
        return;
    }

    ret = hmac_roam_connect_fsm_action(roam_info, ROAM_CONNECT_FSM_EVENT_MGMT_RX, (void *)crx_event);
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                         "{hmac_roam_connect_rx_mgmt::MGMT_RX FAIL[%d]!}", ret);
    }

    return;
}

void hmac_roam_connect_key_done(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru *roam_info = NULL;
    uint32_t ret;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_key_done::vap null!}");
        return;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        return;
    }

    /* 漫游开关没有开时，不处理管理帧接收 */
    if (roam_info->uc_enable == 0) {
        return;
    }

    /* 主状态机为非CONNECTING状态/CONNECT状态机为非UP状态，失败 */
    if (roam_info->en_main_state != ROAM_MAIN_STATE_CONNECTING) {
        return;
    }

    ret = hmac_roam_connect_fsm_action(roam_info, ROAM_CONNECT_FSM_EVENT_KEY_DONE, NULL);
    if (ret != OAL_SUCC) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_connect_key_done::KEY_DONE FAIL[%d]!}", ret);
    }
    oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_key_done::KEY_DONE !}");

    return;
}

#ifdef _PRE_WLAN_FEATURE_11R
uint32_t hmac_roam_connect_ft_reassoc(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru *roam_info = NULL;
    hmac_join_req_stru join_req;
    uint32_t ret;
    mac_bss_dscr_stru *bss_dscr = NULL;
    uint8_t rate_num;

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    bss_dscr = roam_info->st_connect.pst_bss_dscr;

    if (roam_info->pst_hmac_user != NULL) {
        oal_set_mac_addr(roam_info->pst_hmac_user->st_user_base_info.auc_user_mac_addr, bss_dscr->auc_bssid);
    }

    rate_num = (bss_dscr->uc_num_supp_rates < WLAN_MAX_SUPP_RATES) ?
        bss_dscr->uc_num_supp_rates : WLAN_MAX_SUPP_RATES;
    if (memcpy_s(hmac_vap->auc_supp_rates, WLAN_MAX_SUPP_RATES, bss_dscr->auc_supp_rates, rate_num) != EOK) {
        oam_error_log0(0, OAM_SF_SCAN, "hmac_roam_connect_ft_reassoc::memcpy fail!");
        return OAL_FAIL;
    }
    mac_mib_set_SupportRateSetNums(&hmac_vap->st_vap_base_info, bss_dscr->uc_num_supp_rates);

    if (mac_mib_get_ft_over_ds(&hmac_vap->st_vap_base_info) && (hmac_vap->bit_11r_over_ds == OAL_TRUE)) {
        /* 配置join参数 */
        hmac_prepare_join_req(&join_req, bss_dscr);

        ret = hmac_sta_update_join_req_params(hmac_vap, &join_req);
        if (ret != OAL_SUCC) {
            oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                           "{hmac_roam_connect_ft_reassoc::hmac_sta_update_join_req_params fail[%d].}", ret);
            return ret;
        }
    }
    /* 发送关联请求 */
    ret = hmac_roam_send_reassoc_req(roam_info);
    if (ret != OAL_SUCC) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
                       "{hmac_roam_connect_ft_reassoc::hmac_roam_send_assoc_req failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_roam_connect_ft_preauth(hmac_vap_stru *hmac_vap)
{
    uint32_t ret;
    hmac_roam_info_stru *roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;

    /* 发送关联请求 */
    ret = hmac_roam_send_ft_preauth_req(roam_info);
    if (ret != OAL_SUCC) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ROAM,
            "{hmac_roam_connect_ft_preauth::hmac_roam_send_preauth_req failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

#endif

