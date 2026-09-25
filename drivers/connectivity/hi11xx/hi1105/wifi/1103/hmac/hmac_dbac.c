

/* 1 头文件包含 */
#include "mac_ie.h"
#include "mac_device.h"
#include "wlan_types.h"
#include "hmac_chan_mgmt.h"
#include "mac_device.h"
#include "hmac_dbac.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DBAC_C

/* 2 全局变量定义 */
/* 3 函数声明 */
/* 4 函数实现 */
#ifdef _PRE_WLAN_FEATURE_DBAC
#define DBAC_RUNING_HANDLE_TIMEOUT 30000 /* 30秒 */
#define HMAC_DBAC_GO_CSA_COUNT 5

hmac_dbac_handle_stru g_st_hmac_dbac_handle_info;


OAL_STATIC oal_void hmac_dbac_go_csa_handle(mac_vap_stru *pst_go_vap,
    mac_vap_stru *pst_legacy_sta)
{
    wlan_bw_cap_enum_uint8 en_bw_cap;
    wlan_channel_bandwidth_enum_uint8 en_go_new_bw;

    if (pst_legacy_sta->st_channel.en_band != pst_go_vap->st_channel.en_band) {
        OAM_WARNING_LOG2(0, OAM_SF_DBAC, "{hmac_dbac_go_csa_handle::\
            legacy_band=[%d], go_band=[%d], not handle}", pst_legacy_sta->st_channel.en_band,
            pst_go_vap->st_channel.en_band);
        return;
    }

    if (pst_legacy_sta->st_channel.uc_chan_number == pst_go_vap->st_channel.uc_chan_number) {
        OAM_WARNING_LOG2(0, OAM_SF_DBAC, "{hmac_dbac_go_csa_handle::\
            legacy_channel=[%d], go_channel=[%d], not handle}", pst_legacy_sta->st_channel.uc_chan_number,
            pst_go_vap->st_channel.uc_chan_number);
        return;
    }

    if (mac_is_dfs_channel(pst_legacy_sta->st_channel.en_band, pst_legacy_sta->st_channel.uc_chan_number)) {
        OAM_WARNING_LOG1(0, OAM_SF_DBAC, "{hmac_dbac_go_csa_handle::[%d] is dfs channel, not handle}",
            pst_legacy_sta->st_channel.uc_chan_number);
        return;
    }

    en_bw_cap = mac_vap_bw_mode_to_bw(pst_go_vap->st_channel.en_bandwidth);
    en_go_new_bw = mac_regdomain_get_bw_by_channel_bw_cap(pst_legacy_sta->st_channel.uc_chan_number, en_bw_cap);

    OAM_WARNING_LOG4(0, OAM_SF_DBAC, "{hmac_dbac_go_csa_handle::\
        go channel from [%d][%d] to [%d][%d]}",
        pst_go_vap->st_channel.uc_chan_number, pst_go_vap->st_channel.en_bandwidth,
        pst_legacy_sta->st_channel.uc_chan_number, en_go_new_bw);

    pst_go_vap->st_ch_switch_info.uc_ch_switch_cnt = HMAC_DBAC_GO_CSA_COUNT;
    pst_go_vap->st_ch_switch_info.en_csa_mode      = WLAN_CSA_MODE_TX_ENABLE;
    hmac_chan_initiate_switch_to_new_channel(pst_go_vap, pst_legacy_sta->st_channel.uc_chan_number,
        en_go_new_bw);
}


OAL_STATIC oal_uint32 hmac_dbac_timeout_handle(oal_void *p_arg)
{
    mac_device_stru *pst_mac_device;
    mac_vap_stru *pst_vap[2] = {OAL_PTR_NULL}; /* 2为变量个数 */
    mac_vap_stru *pst_legacy_sta = OAL_PTR_NULL;
    mac_vap_stru *pst_p2p_vap = OAL_PTR_NULL;

    pst_mac_device = (mac_device_stru *)p_arg;
    if (pst_mac_device == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (!mac_is_dbac_running(pst_mac_device)) {
        return OAL_SUCC;
    }

    if (mac_device_find_2up_vap_etc(pst_mac_device, &pst_vap[0], &pst_vap[1]) != OAL_SUCC) {
        OAM_WARNING_LOG0(0, OAM_SF_DBAC, "{hmac_dbac_timeout_handle::not find 2up vap,return}");
        return OAL_SUCC;
    }

    if (pst_vap[0] == OAL_PTR_NULL || pst_vap[1] == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (IS_LEGACY_STA(pst_vap[0])) {
        pst_legacy_sta = pst_vap[0];
        pst_p2p_vap    = pst_vap[1];
    } else {
        pst_legacy_sta = pst_vap[1];
        pst_p2p_vap    = pst_vap[0];
    }

    /* 如果GO存在，则GO触发CSA切到与STA同频同信道 */
    if (IS_P2P_GO(pst_p2p_vap) && pst_p2p_vap->bit_vap_support_csa) {
        hmac_dbac_go_csa_handle(pst_p2p_vap, pst_legacy_sta);
    }

    return OAL_SUCC;
}


OAL_STATIC oal_void hamc_dbac_handle(mac_device_stru *pst_mac_device)
{
    hmac_dbac_handle_stru *pst_hmac_dbac_info;

    pst_hmac_dbac_info = &g_st_hmac_dbac_handle_info;

    if (!mac_is_dbac_running(pst_mac_device)) {
        return;
    }

    if (pst_hmac_dbac_info->st_dbac_timer.en_is_registerd == OAL_TRUE) {
        return;
    }

    FRW_TIMER_CREATE_TIMER(&pst_hmac_dbac_info->st_dbac_timer,
                           hmac_dbac_timeout_handle,
                           DBAC_RUNING_HANDLE_TIMEOUT,
                           pst_mac_device,
                           OAL_FALSE,
                           OAM_MODULE_ID_HMAC,
                           pst_mac_device->ul_core_id);
}


oal_uint32 hmac_dbac_status_notify_etc(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event;
    mac_device_stru *pst_mac_device;
    oal_bool_enum_uint8 *pen_dbac_enable;

    if (OAL_UNLIKELY(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_DBAC, "{hmac_dbac_status_notify_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pen_dbac_enable = (oal_bool_enum_uint8 *)pst_event->auc_event_data;
    OAM_WARNING_LOG1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_DBAC,
        "hmac_dbac_status_notify_etc::dbac switch to enable=%d", *pen_dbac_enable);

    pst_mac_device = mac_res_get_dev_etc(pst_event->st_event_hdr.uc_device_id);
    if (OAL_UNLIKELY(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_DBAC,
                       "{hmac_dbac_status_notify_etc::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device->en_dbac_running = *pen_dbac_enable;
    hamc_dbac_handle(pst_mac_device);

    return OAL_SUCC;
}
#endif
