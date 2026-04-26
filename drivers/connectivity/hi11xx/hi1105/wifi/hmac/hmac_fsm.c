

/* 1 头文件包含 */
#include "hmac_fsm.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_scan.h"
#include "hmac_mgmt_sta_up.h"
#include "hmac_mgmt_sta.h"
#include "hmac_mgmt_ap.h"
#include "frw_ext_if.h"
#include "hmac_config.h"
#include "hmac_roam_main.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_FSM_C


#define MAC_VAP_AP_STATE_BUTT  (MAC_VAP_STATE_AP_WAIT_START + 1)
#define MAC_VAP_STA_STATE_BUTT MAC_VAP_STATE_BUTT

/* 全局状态机函数表 */
OAL_STATIC hmac_fsm_func g_pa_hmac_ap_fsm_func[MAC_VAP_AP_STATE_BUTT][HMAC_FSM_AP_INPUT_TYPE_BUTT];
OAL_STATIC hmac_fsm_func g_pa_hmac_sta_fsm_func[MAC_VAP_STA_STATE_BUTT][HMAC_FSM_STA_INPUT_TYPE_BUTT];


void hmac_fsm_change_state_check(hmac_vap_stru *pst_hmac_vap,
                                 mac_vap_state_enum_uint8 en_comp_vap_state,
                                 mac_vap_state_enum_uint8 en_new_vap_state)
{
    if (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA &&
        en_comp_vap_state != pst_hmac_vap->st_vap_base_info.en_vap_state) {
        hmac_fsm_change_state(pst_hmac_vap, en_new_vap_state);
    }
}


void hmac_fsm_change_state(hmac_vap_stru *pst_hmac_vap, mac_vap_state_enum_uint8 en_vap_state)
{
    mac_cfg_mode_param_stru st_cfg_mode;
    mac_vap_state_enum_uint8 en_old_state;
    uint32_t ret;
    en_old_state = pst_hmac_vap->st_vap_base_info.en_vap_state;

    /* 将vap状态改变信息上报 */
    mac_vap_state_change(&pst_hmac_vap->st_vap_base_info, en_vap_state);

    ret = hmac_config_vap_state_syn(&(pst_hmac_vap->st_vap_base_info),
                                    sizeof(en_vap_state),
                                    (uint8_t *)(&en_vap_state));
    if (ret != OAL_SUCC) {
        oam_error_log3(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_fsm_change_state::hmac_syn_vap_state failed[%d], old_state=%d, new_state=%d.}",
                       ret, en_old_state, en_vap_state);
    }

    if ((en_vap_state == MAC_VAP_STATE_UP) && (en_old_state != MAC_VAP_STATE_ROAMING) &&
        (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (pst_hmac_vap->st_ps_sw_timer.en_is_registerd == OAL_FALSE)) {
        frw_timer_create_timer_m(&(pst_hmac_vap->st_ps_sw_timer),
                                 hmac_set_ipaddr_timeout,
                                 HMAC_SWITCH_STA_PSM_PERIOD,
                                 (void *)pst_hmac_vap,
                                 OAL_FALSE,
                                 OAM_MODULE_ID_HMAC,
                                 pst_hmac_vap->st_vap_base_info.core_id);
        pst_hmac_vap->us_check_timer_pause_cnt = 0;
    }

    if ((en_vap_state == MAC_VAP_STATE_STA_FAKE_UP)
        && (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA)) {
        if (pst_hmac_vap->st_ps_sw_timer.en_is_registerd == OAL_TRUE) {
            frw_timer_immediate_destroy_timer_m(&pst_hmac_vap->st_ps_sw_timer);
        }
        st_cfg_mode.en_protocol = pst_hmac_vap->st_preset_para.en_protocol;
        st_cfg_mode.en_band = pst_hmac_vap->st_preset_para.en_band;
        st_cfg_mode.en_bandwidth = pst_hmac_vap->st_preset_para.en_bandwidth;

        pst_hmac_vap->st_vap_base_info.st_channel.uc_chan_number = 0;

        hmac_config_sta_update_rates(&(pst_hmac_vap->st_vap_base_info), &st_cfg_mode, NULL);
    }
}

/*****************************************************************************
  3 函数实现
*****************************************************************************/

uint32_t hmac_fsm_null_fn(hmac_vap_stru *pst_hmac_vap, void *p_param)
{
    /* 什么都不做 */
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    if (OAL_TRUE == wlan_pm_wkup_src_debug_get()) {
        wlan_pm_wkup_src_debug_set(OAL_FALSE);
        oam_warning_log0(0, OAM_SF_RX, "{wifi_wake_src:hmac_fsm_null_fn::rcv mgmt frame,drop it}");
    }
#endif
    oam_warning_log1(0, OAM_SF_RX, "{hmac_fsm_null_fn::vap state[%d]}", pst_hmac_vap->st_vap_base_info.en_vap_state);
    return OAL_SUCC;
}


OAL_STATIC void hmac_fsm_init_ap(void)
{
    uint32_t state;
    uint32_t input;

    for (state = 0; state < MAC_VAP_AP_STATE_BUTT; state++) {
        for (input = 0; input < HMAC_FSM_AP_INPUT_TYPE_BUTT; input++) {
            g_pa_hmac_ap_fsm_func[state][input] = hmac_fsm_null_fn;
        }
    }

    /* 接收管理帧输入
    +----------------------------------+---------------------
     | FSM State                        | FSM Function
     +----------------------------------+---------------------
     | MAC_VAP_STATE_INIT               | null_fn
     | MAC_VAP_STATE_UP                 | hmac_ap_up_rx_mgmt
     | MAC_VAP_STATE_PAUSE              | null_fn
     +----------------------------------+---------------------
    */
    g_pa_hmac_ap_fsm_func[MAC_VAP_STATE_AP_WAIT_START][HMAC_FSM_INPUT_MISC] = hmac_ap_wait_start_misc;

    g_pa_hmac_ap_fsm_func[MAC_VAP_STATE_UP][HMAC_FSM_INPUT_RX_MGMT] = hmac_ap_up_rx_mgmt;
    g_pa_hmac_ap_fsm_func[MAC_VAP_STATE_UP][HMAC_FSM_INPUT_MISC] = hmac_ap_up_misc;

    g_pa_hmac_ap_fsm_func[MAC_VAP_STATE_PAUSE][HMAC_FSM_INPUT_RX_MGMT] = hmac_ap_up_rx_mgmt;
    g_pa_hmac_ap_fsm_func[MAC_VAP_STATE_PAUSE][HMAC_FSM_INPUT_MISC] = hmac_ap_wait_start_misc;
}


OAL_STATIC void hmac_fsm_init_sta_listen_req(void)
{
    uint32_t state;

    /* Initialize the elements in the LISTEN_REQ input column. This input is */
    /* valid in the SCAN_COMP and FAKE_UP state.                             */
    /*                                                                       */
    /* +------------------+---------------------+                            */
    /* | FSM State        | FSM Function        |                            */
    /* +------------------+---------------------+                            */
    /* | FAKE_UP          | hmac_sta_wait_listen|                            */
    /* | SCAN_COMP        | hmac_sta_wait_listen|                            */
    /* | All other states | null_fn             |                            */
    /* +------------------+---------------------+                            */
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_LISTEN_REQ] = hmac_fsm_null_fn;
    }

    if (hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
        g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_INIT][HMAC_FSM_INPUT_LISTEN_REQ] = hmac_p2p_remain_on_channel;
    }

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_FAKE_UP][HMAC_FSM_INPUT_LISTEN_REQ] = hmac_p2p_remain_on_channel;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_SCAN_COMP][HMAC_FSM_INPUT_LISTEN_REQ] = hmac_p2p_remain_on_channel;
    
    // 在监听状态接收到新的监听命令，则执行新的监听。
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_LISTEN][HMAC_FSM_INPUT_LISTEN_REQ] = hmac_p2p_remain_on_channel;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_UP][HMAC_FSM_INPUT_LISTEN_REQ] = hmac_p2p_remain_on_channel;
}

OAL_STATIC void hmac_fsm_init_input_scan_req(void)
{
    uint32_t state;

    /* 初始化扫描请求输入函数 HMAC_FSM_INPUT_SCAN_REQ
     +----------------------------------+---------------------
     | FSM State                        | FSM Function
     +----------------------------------+---------------------
     | MAC_VAP_STATE_INIT               | null_fn
     | MAC_VAP_STATE_UP                 | null_fn
     | MAC_VAP_STATE_FAKE_UP            | hmac_mgmt_scan_req
     | MAC_VAP_STATE_WAIT_SCAN          | null_fn
     | MAC_VAP_STATE_SCAN_COMP          | hmac_mgmt_scan_req
     | MAC_VAP_STATE_WAIT_JOIN          | null_fn
     | MAC_VAP_STATE_JOIN_COMP          | hmac_mgmt_scan_req
     | MAC_VAP_STATE_WAIT_AUTH_SEQ2     | null_fn
     | MAC_VAP_STATE_WAIT_AUTH_SEQ4     | null_fn
     | MAC_VAP_STATE_AUTH_COMP          | hmac_mgmt_scan_req
     | MAC_VAP_STATE_WAIT_ASOC          | null_fn
     | MAC_VAP_STATE_OBSS_SCAN          | null_fn
     | MAC_VAP_STATE_BG_SCAN            | null_fn
     +----------------------------------+---------------------
    */
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_SCAN_REQ] = hmac_scan_proc_scan_req_event_exception;
    }

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_INIT][HMAC_FSM_INPUT_SCAN_REQ] = hmac_scan_proc_scan_req_event;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_FAKE_UP][HMAC_FSM_INPUT_SCAN_REQ] = hmac_scan_proc_scan_req_event;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_SCAN_COMP][HMAC_FSM_INPUT_SCAN_REQ] = hmac_scan_proc_scan_req_event;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_JOIN_COMP][HMAC_FSM_INPUT_SCAN_REQ] = hmac_scan_proc_scan_req_event;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_AUTH_COMP][HMAC_FSM_INPUT_SCAN_REQ] = hmac_scan_proc_scan_req_event;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_UP][HMAC_FSM_INPUT_SCAN_REQ] = hmac_scan_proc_scan_req_event;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_LISTEN][HMAC_FSM_INPUT_SCAN_REQ] = hmac_scan_proc_scan_req_event;
}

OAL_STATIC void hmac_fsm_init_input_rx_mgmt(void)
{
    uint32_t state;

    /*
     * +----------------------------------+---------------------
     * | FSM State                        | FSM Function
     * +----------------------------------+---------------------
     * | MAC_VAP_STATE_INIT               | null_fn
     * | MAC_VAP_STATE_UP                 | hmac_sta_up_rx_mgmt
     * | MAC_VAP_STATE_FAKE_UP            | null_fn
     * | MAC_VAP_STATE_WAIT_SCAN          | hmac_sta_wait_scan_rx
     * | MAC_VAP_STATE_SCAN_COMP          | null_fn
     * | MAC_VAP_STATE_WAIT_JOIN          | hmac_sta_wait_join_rx
     * | MAC_VAP_STATE_JOIN_COMP          | null_fn
     * | MAC_VAP_STATE_WAIT_AUTH_SEQ2     | hmac_sta_wait_auth_seq2_rx
     * | MAC_VAP_STATE_WAIT_AUTH_SEQ4     | hmac_sta_wait_auth_seq4_rx
     * | MAC_VAP_STATE_AUTH_COMP          | null_fn
     * | MAC_VAP_STATE_WAIT_ASOC          | hmac_sta_wait_asoc_rx
     * | MAC_VAP_STATE_OBSS_SCAN          | null_fn
     * | MAC_VAP_STATE_BG_SCAN            | null_fn
     * +----------------------------------+---------------------
     */
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_RX_MGMT] = hmac_fsm_null_fn;
    }

    /* 增加 HMAC_FSM_INPUT_RX_MGMT事件 处理函数 */
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2][HMAC_FSM_INPUT_RX_MGMT] = hmac_sta_wait_auth_seq2_rx;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4][HMAC_FSM_INPUT_RX_MGMT] = hmac_sta_wait_auth_seq4_rx;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_WAIT_ASOC][HMAC_FSM_INPUT_RX_MGMT] = hmac_sta_wait_asoc_rx;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_UP][HMAC_FSM_INPUT_RX_MGMT] = hmac_sta_up_rx_mgmt;

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_ROAMING][HMAC_FSM_INPUT_RX_MGMT] = hmac_sta_roam_rx_mgmt;

    /* 增加 HotSpot中状态机sta侧处理函数 */
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_SCAN_COMP][HMAC_FSM_INPUT_RX_MGMT] = hmac_sta_not_up_rx_mgmt;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_WAIT_SCAN][HMAC_FSM_INPUT_RX_MGMT] = hmac_sta_not_up_rx_mgmt;

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_FAKE_UP][HMAC_FSM_INPUT_RX_MGMT] = hmac_sta_not_up_rx_mgmt;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_LISTEN][HMAC_FSM_INPUT_RX_MGMT] = hmac_sta_not_up_rx_mgmt;

#ifdef _PRE_WLAN_FEATURE_FTM // FTM认证
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_JOIN_COMP][HMAC_FSM_INPUT_RX_MGMT] = hmac_sta_not_up_rx_mgmt;
#endif
}

OAL_STATIC void hmac_fsm_init_sta(void)
{
    uint32_t state;

    hmac_fsm_init_input_scan_req();

    /* Initialize the elements in the JOIN_REQ input column. This input is   */
    /* valid only in the SCAN_COMP state.                                    */
    /*                                                                       */
    /* +------------------+---------------------+                            */
    /* | FSM State        | FSM Function        |                            */
    /* +------------------+---------------------+                            */
    /* | SCAN_COMP        | hmac_sta_wait_join  |                            */
    /* | All other states | null_fn             |                            */
    /* +------------------+---------------------+                            */
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_JOIN_REQ] = hmac_fsm_null_fn;
    }

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_FAKE_UP][HMAC_FSM_INPUT_JOIN_REQ] = hmac_sta_wait_join;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_SCAN_COMP][HMAC_FSM_INPUT_JOIN_REQ] = hmac_sta_wait_join;

    /* Initialize the elements in the AUTH_REQ input column. This input is   */
    /* valid only in the JOIN_COMP state.                                    */
    /*                                                                       */
    /* +------------------+---------------------+                            */
    /* | FSM State        | FSM Function        |                            */
    /* +------------------+---------------------+                            */
    /* | JOIN_COMP        | hmac_sta_wait_auth  |                            */
    /* | All other states | null_fn             |                            */
    /* +------------------+---------------------+                            */
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_AUTH_REQ] = hmac_fsm_null_fn;
    }

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_JOIN_COMP][HMAC_FSM_INPUT_AUTH_REQ] = hmac_sta_wait_auth;

    /* Initialize the elements in the ASOC_REQ input column. This input is   */
    /* valid only in the AUTH_COMP state.                                    */
    /*                                                                       */
    /* +------------------+---------------------+                            */
    /* | FSM State        | FSM Function        |                            */
    /* +------------------+---------------------+                            */
    /* | AUTH_COMP        | hmac_sta_wait_asoc       |                            */
    /* | All other states | null_fn             |                            */
    /* +------------------+---------------------+                            */
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_ASOC_REQ] = hmac_fsm_null_fn;
    }

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_AUTH_COMP][HMAC_FSM_INPUT_ASOC_REQ] = hmac_sta_wait_asoc;

    hmac_fsm_init_input_rx_mgmt();
    /* 初始化timer0超时请求输入函数 HMAC_FSM_INPUT_TIMER0_OUT
     +----------------------------------+---------------------
     | FSM State                        | FSM Function
     +----------------------------------+---------------------
     | MAC_VAP_STATE_INIT               | null_fn
     | MAC_VAP_STATE_PAUSE              | null_fn
     | MAC_VAP_STATE_FAKE_UP            | null_fn
     | MAC_VAP_STATE_WAIT_SCAN          | null_fn
     | MAC_VAP_STATE_SCAN_COMP          | null_fn
     | MAC_VAP_STATE_WAIT_JOIN          | null_fn
     | MAC_VAP_STATE_JOIN_COMP          | null_fn
     | MAC_VAP_STATE_WAIT_AUTH_SEQ2     | null_fn
     | MAC_VAP_STATE_WAIT_AUTH_SEQ4     | null_fn
     | MAC_VAP_STATE_AUTH_COMP          | null_fn
     | MAC_VAP_STATE_WAIT_ASOC          | null_fn
     | MAC_VAP_STATE_OBSS_SCAN          | null_fn
     | MAC_VAP_STATE_BG_SCAN            | null_fn
     +----------------------------------+---------------------
    */
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_TIMER0_OUT] = hmac_fsm_null_fn;
    }

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2][HMAC_FSM_INPUT_TIMER0_OUT] = hmac_sta_auth_timeout;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4][HMAC_FSM_INPUT_TIMER0_OUT] = hmac_sta_auth_timeout;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_WAIT_ASOC][HMAC_FSM_INPUT_TIMER0_OUT] = hmac_sta_wait_asoc_timeout;

    /* 初始化HMAC_FSM_INPUT_MISC事件请求的处理函数
       +----------------------------------+---------------------
       | FSM State                        | FSM Function
       +----------------------------------+---------------------
       | MAC_VAP_STATE_INIT               | null_fn
       | MAC_VAP_STATE_PAUSE              | null_fn
       | MAC_VAP_STATE_FAKE_UP            | null_fn
       | MAC_VAP_STATE_WAIT_SCAN          | null_fn
       | MAC_VAP_STATE_SCAN_COMP          | null_fn
       | MAC_VAP_STATE_WAIT_JOIN          | null_fn
       | MAC_VAP_STATE_JOIN_COMP          | null_fn
       | MAC_VAP_STATE_WAIT_AUTH_SEQ2     | null_fn
       | MAC_VAP_STATE_WAIT_AUTH_SEQ4     | null_fn
       | MAC_VAP_STATE_AUTH_COMP          | null_fn
       | MAC_VAP_STATE_WAIT_ASOC          | null_fn
       | MAC_VAP_STATE_OBSS_SCAN          | null_fn
       | MAC_VAP_STATE_BG_SCAN            | null_fn
       +----------------------------------+---------------------
      */
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_MISC] = hmac_fsm_null_fn;
    }

    hmac_fsm_init_sta_listen_req();

    /* Initialize the elements in the HMAC_FSM_INPUT_LISTEN_TIMEOUT          */
    /* input column. This input is valid in MAC_VAP_STATE_STA_LISTEN state.  */
    /*                                                                       */
    /* +----------------------------+------------------------+               */
    /* | FSM State                  | FSM Function           |               */
    /* +----------------------------+------------------------+               */
    /* | MAC_VAP_STATE_STA_LISTEN   | hmac_p2p_listen_timeout|               */
    /* | All other states           | null_fn                |               */
    /* +----------------------------+------------------------+               */
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_LISTEN_TIMEOUT] = hmac_fsm_null_fn;
    }
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_LISTEN][HMAC_FSM_INPUT_LISTEN_TIMEOUT] = hmac_p2p_listen_timeout;

    /* Initialize the elements in the SCHED_SCAN_REQ input column. This input is */
    /* valid in the SCAN_COMP and FAKE_UP state.                             */
    /*                                                                       */
    /* +------------------+------------------------------------+             */
    /* | FSM State        | FSM Function                       |             */
    /* +------------------+------------------------------------+             */
    /* | FAKE_UP          | hmac_scan_proc_sched_scan_req_event|             */
    /* | SCAN_COMP        | hmac_scan_proc_sched_scan_req_event|             */
    /* | All other states | null_fn                            |             */
    /* +------------------+------------------------------------+             */
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_SCHED_SCAN_REQ] = hmac_fsm_null_fn;
    }

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_FAKE_UP][HMAC_FSM_INPUT_SCHED_SCAN_REQ] =
        hmac_scan_proc_sched_scan_req_event;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_SCAN_COMP][HMAC_FSM_INPUT_SCHED_SCAN_REQ] =
        hmac_scan_proc_sched_scan_req_event;
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_ROAMING_START] = hmac_fsm_null_fn;
    }

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_UP][HMAC_FSM_INPUT_ROAMING_START] = hmac_roam_pause_user;

    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_ROAMING_STOP] = hmac_roam_resume_user;
    }
}


void hmac_fsm_init(void)
{
    /* 初始化ap状态机函数表 */
    hmac_fsm_init_ap();

    /* 初始化sta状态机函数表 */
    hmac_fsm_init_sta();
}


uint32_t hmac_fsm_call_func_ap(hmac_vap_stru *pst_hmac_vap,
                               hmac_fsm_input_type_enum en_input,
                               void *p_param)
{
    if (oal_unlikely(pst_hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_fsm_call_func_ap::vap is null ptr.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
            "{hmac_fsm_call_func_ap::the vap has been deleted, its state is STATE_BUTT, input type is %d}",
            en_input);
        return OAL_SUCC;
    } else if (pst_hmac_vap->st_vap_base_info.en_vap_state >= MAC_VAP_AP_STATE_BUTT ||
               en_input >= HMAC_FSM_AP_INPUT_TYPE_BUTT) {
            oam_error_log2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                           "{hmac_fsm_call_func_ap::vap state[%d] or input type[%d] is over limit!}",
                           pst_hmac_vap->st_vap_base_info.en_vap_state, en_input);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    return g_pa_hmac_ap_fsm_func[pst_hmac_vap->st_vap_base_info.en_vap_state][en_input](pst_hmac_vap, p_param);
}


uint32_t hmac_fsm_call_func_sta(hmac_vap_stru *pst_hmac_vap,
                                hmac_fsm_input_type_enum en_input,
                                void *p_param)
{
    if (oal_unlikely(pst_hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_fsm_call_func_sta::vap is null ptr.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STA_STATE_BUTT) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
            "{hmac_fsm_call_func_sta::the vap has been deleted, its state is STATE_BUTT, input type is %d}",
            en_input);
        return OAL_SUCC;
    } else if (pst_hmac_vap->st_vap_base_info.en_vap_state >= MAC_VAP_STA_STATE_BUTT ||
               en_input >= HMAC_FSM_STA_INPUT_TYPE_BUTT) {
            oam_error_log2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                           "{hmac_fsm_call_func_sta::vap state[%d] or input type[%d] is over limit!}",
                           pst_hmac_vap->st_vap_base_info.en_vap_state, en_input);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    return g_pa_hmac_sta_fsm_func[pst_hmac_vap->st_vap_base_info.en_vap_state][en_input](pst_hmac_vap, p_param);
}

