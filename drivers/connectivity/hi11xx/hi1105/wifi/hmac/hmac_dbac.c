

#include "hmac_dbac.h"
#include "mac_ie.h"
#include "mac_device.h"
#include "wlan_types.h"
#include "hmac_chan_mgmt.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DBAC_C

#define DBAC_RUNING_HANDLE_TIMEOUT 30000 /* 30秒 */
#define HMAC_DBAC_GO_CSA_COUNT 5

hmac_dbac_handle_stru g_st_hmac_dbac_handle_info;


OAL_STATIC void hmac_dbac_go_csa_handle(mac_vap_stru *pst_go_vap,
    mac_vap_stru *pst_legacy_sta)
{
    wlan_bw_cap_enum_uint8 en_bw_cap;
    wlan_channel_bandwidth_enum_uint8 en_go_new_bw;

    if (pst_legacy_sta->st_channel.en_band != pst_go_vap->st_channel.en_band) {
        oam_warning_log2(0, OAM_SF_DBAC, "{hmac_dbac_go_csa_handle::\
            legacy_band=[%d], go_band=[%d], not handle}", pst_legacy_sta->st_channel.en_band,
            pst_go_vap->st_channel.en_band);
        return;
    }

    if (pst_legacy_sta->st_channel.uc_chan_number == pst_go_vap->st_channel.uc_chan_number) {
        oam_warning_log2(0, OAM_SF_DBAC, "{hmac_dbac_go_csa_handle::\
            legacy_channel=[%d], go_channel=[%d], not handle}", pst_legacy_sta->st_channel.uc_chan_number,
            pst_go_vap->st_channel.uc_chan_number);
        return;
    }

    if (mac_is_dfs_channel(pst_legacy_sta->st_channel.en_band, pst_legacy_sta->st_channel.uc_chan_number)) {
        oam_warning_log1(0, OAM_SF_DBAC, "{hmac_dbac_go_csa_handle::[%d] is dfs channel, not handle}",
            pst_legacy_sta->st_channel.uc_chan_number);
        return;
    }

    en_bw_cap = mac_vap_bw_mode_to_bw(pst_go_vap->st_channel.en_bandwidth);
    en_go_new_bw = mac_regdomain_get_bw_by_channel_bw_cap(pst_legacy_sta->st_channel.uc_chan_number, en_bw_cap);

    oam_warning_log4(0, OAM_SF_DBAC, "{hmac_dbac_go_csa_handle::\
        go channel from [%d][%d] to [%d][%d]}",
        pst_go_vap->st_channel.uc_chan_number, pst_go_vap->st_channel.en_bandwidth,
        pst_legacy_sta->st_channel.uc_chan_number, en_go_new_bw);

    pst_go_vap->st_ch_switch_info.uc_ch_switch_cnt = HMAC_DBAC_GO_CSA_COUNT;
    pst_go_vap->st_ch_switch_info.en_csa_mode      = WLAN_CSA_MODE_TX_ENABLE;
    hmac_chan_initiate_switch_to_new_channel(pst_go_vap, pst_legacy_sta->st_channel.uc_chan_number,
        en_go_new_bw);
}


uint32_t hmac_go_flow_sta_channel_handle(mac_device_stru *mac_device)
{
    mac_vap_stru *vap[2] = {NULL}; /* 2为变量个数 */
    mac_vap_stru *p2p_vap = NULL;
    mac_vap_stru *legacy_sta = NULL;
    if (!mac_is_dbac_running(mac_device)) {
        return OAL_SUCC;
    }
    if (mac_device_find_2up_vap(mac_device, &vap[0], &vap[1]) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_DBAC, "{hmac_go_flow_sta_channel_handle::not find 2up vap,return}");
        return OAL_SUCC;
    }

    if (vap[0] == NULL || vap[1] == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (IS_LEGACY_STA(vap[0])) {
        legacy_sta = vap[0];
        p2p_vap    = vap[1];
    } else {
        legacy_sta = vap[1];
        p2p_vap    = vap[0];
    }

    /* 如果GO存在，则GO触发CSA切到与STA同频同信道 */
    if (IS_P2P_GO(p2p_vap) && p2p_vap->bit_vap_support_csa) {
        hmac_dbac_go_csa_handle(p2p_vap, legacy_sta);
    }
    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_dbac_timeout_handle(void *p_arg)
{
    mac_device_stru *pst_mac_device;
    pst_mac_device = (mac_device_stru *)p_arg;
    if (pst_mac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    return hmac_go_flow_sta_channel_handle(pst_mac_device);
}


OAL_STATIC void hmac_dbac_handle(mac_device_stru *pst_mac_device)
{
    hmac_dbac_handle_stru *pst_hmac_dbac_info;

    pst_hmac_dbac_info = &g_st_hmac_dbac_handle_info;

    if (!mac_is_dbac_running(pst_mac_device)) {
        return;
    }

    if (pst_hmac_dbac_info->st_dbac_timer.en_is_registerd == OAL_TRUE) {
        return;
    }

    frw_timer_create_timer_m(&pst_hmac_dbac_info->st_dbac_timer,
                             hmac_dbac_timeout_handle,
                             DBAC_RUNING_HANDLE_TIMEOUT,
                             pst_mac_device,
                             OAL_FALSE,
                             OAM_MODULE_ID_HMAC,
                             pst_mac_device->core_id);
}


uint32_t hmac_dbac_status_notify(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    mac_device_stru *pst_mac_device = NULL;
    oal_bool_enum_uint8 *pen_dbac_enable = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_DBAC, "{hmac_dbac_status_notify::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pen_dbac_enable = (oal_bool_enum_uint8 *)pst_event->auc_event_data;
    oam_warning_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_DBAC,
        "hmac_dbac_status_notify::dbac switch to enable=%d", *pen_dbac_enable);

    pst_mac_device = mac_res_get_dev(pst_event->st_event_hdr.uc_device_id);
    if (oal_unlikely(pst_mac_device == NULL)) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_DBAC,
                       "{hmac_dbac_status_notify::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device->en_dbac_running = *pen_dbac_enable;
    hmac_dbac_handle(pst_mac_device);

    return OAL_SUCC;
}
