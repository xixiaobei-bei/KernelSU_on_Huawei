

#include "oal_ext_if.h"
#include "wlan_types.h"
#include "frw_ext_if.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "mac_vap.h"
#include "mac_regdomain.h"
#include "hmac_ext_if.h"
#include "wal_ext_if.h"
#include "wal_main.h"
#include "wal_config.h"
#include "wal_linux_scan.h"
#include "wal_linux_cfg80211.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_flowctl.h"
#include "wal_linux_cfgvendor.h"
#include "wal_linux_cfgvendor_attributes.h"
#include "oal_cfg80211.h"
#include "oal_net.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_scan.h"
#include "hmac_user.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif
#include "securec.h"
#include "hmac_dfx.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_RX_RSP_C


OAL_STATIC void wal_scan_report(hmac_scan_stru *pst_scan_mgmt, oal_bool_enum en_is_aborted)
{
    /* 通知 kernel scan 已经结束 */
    oal_cfg80211_scan_done(pst_scan_mgmt->pst_request, en_is_aborted);

    pst_scan_mgmt->pst_request = NULL;
    pst_scan_mgmt->en_complete = OAL_TRUE;

    oam_warning_log1(0, OAM_SF_SCAN, "{wal_scan_report::scan complete.abort flag[%d]!}", en_is_aborted);

    /* 让编译器优化时保证OAL_WAIT_QUEUE_WAKE_UP在最后执行 */
    oal_smp_mb();
    oal_wait_queue_wake_up_interrupt(&pst_scan_mgmt->st_wait_queue);
}


OAL_STATIC void wal_schedule_scan_report(oal_wiphy_stru *pst_wiphy, hmac_scan_stru *pst_scan_mgmt)
{
    /* 上报调度扫描结果 */
    oal_cfg80211_sched_scan_result(pst_wiphy);

    pst_scan_mgmt->pst_sched_scan_req = NULL;
    pst_scan_mgmt->en_sched_scan_complete = OAL_TRUE;

    oam_warning_log0(0, OAM_SF_SCAN, "{wal_schedule_scan_report::sched scan complete.!}");
}


OAL_STATIC void wal_normal_scan_comp_handle(hmac_scan_stru *pst_scan_mgmt,
    hmac_vap_stru *pst_hmac_vap, hmac_scan_rsp_stru *pst_scan_rsp)
{
    oal_bool_enum en_is_aborted;

    /* 普通扫描结束事件 */
    if (pst_scan_mgmt->pst_request != NULL) {
        if (hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
            if (!pst_hmac_vap->bit_in_p2p_listen) {
                en_is_aborted = (pst_scan_rsp->uc_result_code == MAC_SCAN_ABORT) ? OAL_TRUE : OAL_FALSE;
                wal_scan_report(pst_scan_mgmt, en_is_aborted);
            }
        } else {
            /* 是scan abort的话告知内核 */
            en_is_aborted = (pst_scan_rsp->uc_result_code == MAC_SCAN_ABORT) ? OAL_TRUE : OAL_FALSE;
            wal_scan_report(pst_scan_mgmt, en_is_aborted);
        }
    } else {
        oam_warning_log0(0, OAM_SF_SCAN, "{wal_scan_comp_proc_sta::scan already complete!}");
    }
}

OAL_STATIC void wal_scan_upper_issued_proc(hmac_scan_rsp_stru *pst_scan_rsp, hmac_scan_stru *pst_scan_mgmt,
                                           oal_wiphy_stru *pst_wiphy, hmac_vap_stru *pst_hmac_vap)
{
    /* 上层下发的普通扫描进行对应处理 */
    if (pst_scan_rsp->uc_result_code == MAC_SCAN_PNO) {
        /* PNO扫描结束事件 */
        if (pst_scan_mgmt->pst_sched_scan_req != NULL) {
            
            wal_schedule_scan_report(pst_wiphy, pst_scan_mgmt);
        } else {
            oam_warning_log0(0, OAM_SF_SCAN, "{wal_scan_comp_proc_sta::sched scan already complete!}");
        }
    } else {
        /* 普通扫描结束事件 */
        wal_normal_scan_comp_handle(pst_scan_mgmt, pst_hmac_vap, pst_scan_rsp);
    }
}


uint32_t wal_scan_comp_proc_sta(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    hmac_scan_rsp_stru *pst_scan_rsp = NULL;
    hmac_device_stru *pst_hmac_device = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    hmac_bss_mgmt_stru *pst_bss_mgmt = NULL;
    hmac_scan_stru *pst_scan_mgmt = NULL;
    oal_wiphy_stru *pst_wiphy = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{wal_scan_comp_proc_sta::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 获取hmac vap结构体 */
    pst_hmac_vap = mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    /* 获取hmac device 指针 */
    pst_hmac_device = hmac_res_get_mac_dev(pst_event->st_event_hdr.uc_device_id);
    if (oal_any_null_ptr2(pst_hmac_vap, pst_hmac_device)) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, 0, "{wal_scan_comp_proc_sta::hmac_vap or hmac_dev null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 删除等待扫描超时定时器 */
    if (pst_hmac_vap->st_scan_timeout.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(pst_hmac_vap->st_scan_timeout));
    }

    pst_scan_mgmt = &(pst_hmac_device->st_scan_mgmt);
    pst_wiphy = pst_hmac_device->pst_device_base_info->pst_wiphy;

    /* 获取扫描结果的管理结构地址 */
    pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    /* 获取驱动上报的扫描结果结构体指针 */
    pst_scan_rsp = (hmac_scan_rsp_stru *)pst_event->auc_event_data;

    /* 如果扫描返回结果的非成功，打印维测信息 */
    if ((pst_scan_rsp->uc_result_code != MAC_SCAN_SUCCESS) && (pst_scan_rsp->uc_result_code != MAC_SCAN_PNO)) {
        oam_warning_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_SCAN,
            "{wal_scan_comp_proc_sta::scan not succ, err_code[%d]!}", pst_scan_rsp->uc_result_code);
    }
    if (hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
        if (!pst_hmac_vap->bit_in_p2p_listen) {
            /* P2P listen在强制终止时会调用到本函数，此时不需要上报扫描结果 */
            oam_warning_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_SCAN,
                "{wal_scan_comp_proc_sta::P2P listen NOT inform all bss, bit_in_p2p_listen[%d].}",
                pst_hmac_vap->bit_in_p2p_listen);
            wal_inform_all_bss(pst_wiphy, pst_bss_mgmt, pst_event->st_event_hdr.uc_vap_id);
        }
    } else {
        /* 上报所有扫描到的bss, 无论扫描结果成功与否，统一上报扫描结果，有几个上报几个 */
        wal_inform_all_bss(pst_wiphy, pst_bss_mgmt, pst_event->st_event_hdr.uc_vap_id);
    }
    /* 对于内核下发的扫描request资源加锁 */
    oal_spin_lock(&(pst_scan_mgmt->st_scan_request_spinlock));

    /* 没有未释放的扫描资源，直接返回 */
    if ((pst_scan_mgmt->pst_request == NULL) && (pst_scan_mgmt->pst_sched_scan_req == NULL)) {
        /* 通知完内核，释放资源后解锁 */
        oal_spin_unlock(&(pst_scan_mgmt->st_scan_request_spinlock));
        oam_warning_log0(0, OAM_SF_SCAN, "{wal_scan_comp_proc_sta::legacy scan and pno scan are complete!}");

        return OAL_SUCC;
    }
    if (!oal_any_null_ptr2(pst_scan_mgmt->pst_request, pst_scan_mgmt->pst_sched_scan_req)) {
        /* 一般情况下,2个扫描同时存在是一种异常情况,在此添加打印,暂不做异常处理 */
        oam_warning_log0(0, OAM_SF_SCAN, "{wal_scan_comp_proc_sta::legacy scan and pno scan are all started!!!}");
    }

    /* 上层下发的普通扫描进行对应处理 */
    wal_scan_upper_issued_proc(pst_scan_rsp, pst_scan_mgmt, pst_wiphy, pst_hmac_vap);

    /* 通知完内核，释放资源后解锁 */
    oal_spin_unlock(&(pst_scan_mgmt->st_scan_request_spinlock));
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
    /* 扫描完成国家码更新 */
    wal_counrtycode_selfstudy_scan_comp(pst_hmac_vap);
#endif
    return OAL_SUCC;
}

static void wal_free_asoc_comp_proc_sta_ie_buf(hmac_asoc_rsp_stru *pst_asoc_rsp)
{
    if (pst_asoc_rsp->puc_asoc_rsp_ie_buff != NULL) {
        oal_free(pst_asoc_rsp->puc_asoc_rsp_ie_buff);
        pst_asoc_rsp->puc_asoc_rsp_ie_buff = NULL;
    }
    if (pst_asoc_rsp->puc_asoc_req_ie_buff != NULL) {
        oal_free(pst_asoc_rsp->puc_asoc_req_ie_buff);
        pst_asoc_rsp->puc_asoc_req_ie_buff = NULL;
    }
}


uint32_t wal_asoc_comp_proc_sta(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_connet_result_stru st_connet_result;
    oal_net_device_stru *net_device = NULL;
    hmac_asoc_rsp_stru *pst_asoc_rsp = NULL;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 34))
    uint32_t ret;
#endif

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_asoc_comp_proc_sta::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_asoc_rsp = (hmac_asoc_rsp_stru *)pst_event->auc_event_data;

    /* 获取net_device */
    net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (net_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                       "{wal_asoc_comp_proc_sta::get net device ptr is null!}");
        wal_free_asoc_comp_proc_sta_ie_buf(pst_asoc_rsp);
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(&st_connet_result, sizeof(oal_connet_result_stru), 0, sizeof(oal_connet_result_stru));

    /* 准备上报内核的关联结果结构体 */
    memcpy_s(st_connet_result.auc_bssid, WLAN_MAC_ADDR_LEN, pst_asoc_rsp->auc_addr_ap, WLAN_MAC_ADDR_LEN);

    st_connet_result.puc_req_ie = pst_asoc_rsp->puc_asoc_req_ie_buff;
    st_connet_result.req_ie_len = pst_asoc_rsp->asoc_req_ie_len;
    st_connet_result.puc_rsp_ie = pst_asoc_rsp->puc_asoc_rsp_ie_buff;
    st_connet_result.rsp_ie_len = pst_asoc_rsp->asoc_rsp_ie_len;
    st_connet_result.us_status_code = pst_asoc_rsp->en_status_code;

    
    if (st_connet_result.us_status_code == MAC_SUCCESSFUL_STATUSCODE) {
        hmac_device_stru *pst_hmac_device;
        oal_wiphy_stru *pst_wiphy = NULL;
        hmac_bss_mgmt_stru *pst_bss_mgmt = NULL;
        pst_hmac_device = hmac_res_get_mac_dev(pst_event->st_event_hdr.uc_device_id);
        if (oal_any_null_ptr3(pst_hmac_device, pst_hmac_device->pst_device_base_info,
                              pst_hmac_device->pst_device_base_info->pst_wiphy)) {
            oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                           "{wal_asoc_comp_proc_sta::get ptr is null!}");
            wal_free_asoc_comp_proc_sta_ie_buf(pst_asoc_rsp);
            return OAL_ERR_CODE_PTR_NULL;
        }
        pst_wiphy = pst_hmac_device->pst_device_base_info->pst_wiphy;
        pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

        wal_update_bss(pst_wiphy, pst_bss_mgmt, st_connet_result.auc_bssid);
    }
    
    /* 调用内核接口，上报关联结果 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
    oal_cfg80211_connect_result(net_device, st_connet_result.auc_bssid, st_connet_result.puc_req_ie,
        st_connet_result.req_ie_len, st_connet_result.puc_rsp_ie, st_connet_result.rsp_ie_len,
        st_connet_result.us_status_code, GFP_ATOMIC);
#else
    ret = oal_cfg80211_connect_result(net_device, st_connet_result.auc_bssid, st_connet_result.puc_req_ie,
        st_connet_result.req_ie_len, st_connet_result.puc_rsp_ie, st_connet_result.rsp_ie_len,
        st_connet_result.us_status_code, GFP_ATOMIC);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
            "{wal_asoc_comp_proc_sta::oal_cfg80211_connect_result fail[%d]!}", ret);
    }
#endif

    wal_free_asoc_comp_proc_sta_ie_buf(pst_asoc_rsp);

#ifdef _PRE_WLAN_FEATURE_11D
    /* 如果关联成功，sta根据AP的国家码设置自己的管制域 */
    if (pst_asoc_rsp->en_result_code == HMAC_MGMT_SUCCESS) {
        wal_regdomain_update_sta(pst_event->st_event_hdr.uc_vap_id);
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
        wal_selfstudy_regdomain_update_by_ap(pst_event);
#endif
    }
#endif

    /* 启动发送队列，防止发送队列被漫游关闭后无法恢复 */
    oal_net_tx_wake_all_queues(net_device);

    oal_wifi_report_sta_action(net_device->ifindex, OAL_WIFI_STA_JOIN, st_connet_result.auc_bssid, OAL_MAC_ADDR_LEN);

    oam_warning_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
        "{wal_asoc_comp_proc_sta status_code[%d] OK!}", st_connet_result.us_status_code);

    return OAL_SUCC;
}

static void wal_roam_comp_proc_sta_free_ie_buf(hmac_roam_rsp_stru *pst_roam_rsp)
{
    if (pst_roam_rsp->puc_asoc_rsp_ie_buff != NULL) {
        oal_free(pst_roam_rsp->puc_asoc_rsp_ie_buff);
        pst_roam_rsp->puc_asoc_rsp_ie_buff = NULL;
    }
    if (pst_roam_rsp->puc_asoc_req_ie_buff != NULL) {
        oal_free(pst_roam_rsp->puc_asoc_req_ie_buff);
        pst_roam_rsp->puc_asoc_req_ie_buff = NULL;
    }
}


uint32_t wal_roam_comp_proc_sta(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *pst_net_device = NULL;
    mac_device_stru *pst_mac_device = NULL;
    hmac_roam_rsp_stru *pst_roam_rsp = NULL;
    struct ieee80211_channel *pst_channel = NULL;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
    enum nl80211_band en_band = NUM_NL80211_BANDS;
#else
    enum ieee80211_band en_band = IEEE80211_NUM_BANDS;
#endif
    int32_t l_freq;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_roam_comp_proc_sta::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_roam_rsp = (hmac_roam_rsp_stru *)pst_event->auc_event_data;

    /* 获取net_device */
    pst_net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (pst_net_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ROAM,
                       "{wal_asoc_comp_proc_sta::get net device ptr is null!}");
        wal_roam_comp_proc_sta_free_ie_buf(pst_roam_rsp);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获取device id 指针 */
    pst_mac_device = mac_res_get_dev(pst_event->st_event_hdr.uc_device_id);
    if (pst_mac_device == NULL) {
        oam_warning_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_SCAN,
                         "{wal_asoc_comp_proc_sta::pst_mac_device is null ptr!}");
        wal_roam_comp_proc_sta_free_ie_buf(pst_roam_rsp);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_roam_rsp->st_channel.en_band >= WLAN_BAND_BUTT) {
        oam_error_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ROAM,
                       "{wal_asoc_comp_proc_sta::unexpected band[%d]!}",
                       pst_roam_rsp->st_channel.en_band);
        wal_roam_comp_proc_sta_free_ie_buf(pst_roam_rsp);
        return OAL_FAIL;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
    if (pst_roam_rsp->st_channel.en_band == WLAN_BAND_2G) {
        en_band = NL80211_BAND_2GHZ;
    }
    if (pst_roam_rsp->st_channel.en_band == WLAN_BAND_5G) {
        en_band = NL80211_BAND_5GHZ;
    }
#else
    if (pst_roam_rsp->st_channel.en_band == WLAN_BAND_2G) {
        en_band = IEEE80211_BAND_2GHZ;
    }
    if (pst_roam_rsp->st_channel.en_band == WLAN_BAND_5G) {
        en_band = IEEE80211_BAND_5GHZ;
    }
#endif

    l_freq = oal_ieee80211_channel_to_frequency(pst_roam_rsp->st_channel.uc_chan_number, en_band);

    pst_channel = (struct ieee80211_channel *)oal_ieee80211_get_channel(pst_mac_device->pst_wiphy, l_freq);

    /* 调用内核接口，上报关联结果 */
    oal_cfg80211_roamed(pst_net_device,
                        pst_channel,
                        pst_roam_rsp->auc_bssid,
                        pst_roam_rsp->puc_asoc_req_ie_buff,
                        pst_roam_rsp->asoc_req_ie_len,
                        pst_roam_rsp->puc_asoc_rsp_ie_buff,
                        pst_roam_rsp->asoc_rsp_ie_len,
                        GFP_ATOMIC);
    oam_warning_log2(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
        "{wal_roam_comp_proc_sta::oal_cfg80211_roamed OK asoc_req_ie len[%d] asoc_rsp_ie len[%d]!}",
        pst_roam_rsp->asoc_req_ie_len,
        pst_roam_rsp->asoc_rsp_ie_len);
    wal_roam_comp_proc_sta_free_ie_buf(pst_roam_rsp);

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_11R

uint32_t wal_ft_event_proc_sta(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *pst_net_device = NULL;
    hmac_roam_ft_stru *pst_ft_event = NULL;
    oal_cfg80211_ft_event_stru st_cfg_ft_event;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_ft_event_proc_sta::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_ft_event = (hmac_roam_ft_stru *)pst_event->auc_event_data;

    /* 获取net_device */
    pst_net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (pst_net_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ROAM,
                       "{wal_ft_event_proc_sta::get net device ptr is null!}");
        oal_free(pst_ft_event->puc_ft_ie_buff);
        pst_ft_event->puc_ft_ie_buff = NULL;
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_cfg_ft_event.ies = pst_ft_event->puc_ft_ie_buff;
    st_cfg_ft_event.ies_len = pst_ft_event->us_ft_ie_len;
    st_cfg_ft_event.target_ap = pst_ft_event->auc_bssid;
    st_cfg_ft_event.ric_ies = NULL;
    st_cfg_ft_event.ric_ies_len = pst_ft_event->us_ft_ie_len;

    /* 调用内核接口，上报关联结果 */
    oal_cfg80211_ft_event(pst_net_device, &st_cfg_ft_event);

    oal_free(pst_ft_event->puc_ft_ie_buff);
    pst_ft_event->puc_ft_ie_buff = NULL;
    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_11R

uint32_t wal_disasoc_comp_proc_sta(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_disconnect_result_stru st_disconnect_result;
    oal_net_device_stru *pst_net_device = NULL;
    hmac_disconnect *disconnect = NULL;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 34))
    uint32_t ret;
#endif

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_disasoc_comp_proc_sta::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 获取net_device */
    pst_net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (pst_net_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                       "{wal_disasoc_comp_proc_sta::get net device ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取去关联原因码指针 */
    disconnect = (hmac_disconnect *)(pst_event->auc_event_data);

    memset_s(&st_disconnect_result, sizeof(oal_disconnect_result_stru), 0, sizeof(oal_disconnect_result_stru));

    /* 准备上报内核的关联结果结构体 */
    st_disconnect_result.us_reason_code = disconnect->reason_code;

    /* 调用内核接口，上报去关联结果 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
    oal_cfg80211_disconnected(pst_net_device,
                              st_disconnect_result.us_reason_code,
                              st_disconnect_result.pus_disconn_ie,
                              st_disconnect_result.us_disconn_ie_len,
                              GFP_ATOMIC);
#else
    ret = oal_cfg80211_disconnected(pst_net_device,
                                    st_disconnect_result.us_reason_code,
                                    st_disconnect_result.pus_disconn_ie,
                                    st_disconnect_result.us_disconn_ie_len,
                                    GFP_ATOMIC);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                         "{wal_disasoc_comp_proc_sta::oal_cfg80211_disconnected fail[%d]!}", ret);
        return ret;
    }
#endif

    // sta模式下，不需要传mac地址
    oal_wifi_report_sta_action(pst_net_device->ifindex, OAL_WIFI_STA_LEAVE, 0, 0);

    oam_warning_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                     "{wal_disasoc_comp_proc_sta reason_code[%d] OK!}",
                     st_disconnect_result.us_reason_code);
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
    g_country_code_self_study_flag = OAL_TRUE;
#endif
    return OAL_SUCC;
}


uint32_t wal_connect_new_sta_proc_ap(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *pst_net_device = NULL;
    uint8_t auc_connect_user_addr[WLAN_MAC_ADDR_LEN];
    oal_station_info_stru st_station_info;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 34))
    uint32_t ret;
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    hmac_asoc_user_req_ie_stru *pst_asoc_user_req_info;
#endif

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_connect_new_sta_proc_ap::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 获取net_device */
    pst_net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (pst_net_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                       "{wal_connect_new_sta_proc_ap::get net device ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(&st_station_info, sizeof(oal_station_info_stru), 0, sizeof(oal_station_info_stru));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    /* 向内核标记填充了关联请求帧的ie信息 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    /* Linux 4.0 版本不需要STATION_INFO_ASSOC_REQ_IES 标识 */
#else
    st_station_info.filled |= STATION_INFO_ASSOC_REQ_IES;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) */
    pst_asoc_user_req_info = (hmac_asoc_user_req_ie_stru *)(pst_event->auc_event_data);
    st_station_info.assoc_req_ies = pst_asoc_user_req_info->puc_assoc_req_ie_buff;
    if (st_station_info.assoc_req_ies == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                       "{wal_connect_new_sta_proc_ap::asoc ie is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    st_station_info.assoc_req_ies_len = pst_asoc_user_req_info->assoc_req_ie_len;

    /* 获取关联user mac addr */
    memcpy_s(auc_connect_user_addr, WLAN_MAC_ADDR_LEN,
             (uint8_t *)pst_asoc_user_req_info->auc_user_mac_addr, WLAN_MAC_ADDR_LEN);
#else
    /* 获取关联user mac addr */
    memcpy_s(auc_connect_user_addr, WLAN_MAC_ADDR_LEN,
             (uint8_t *)pst_event->auc_event_data, WLAN_MAC_ADDR_LEN);
#endif

    /* 调用内核接口，上报STA关联结果 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
    oal_cfg80211_new_sta(pst_net_device, auc_connect_user_addr, &st_station_info, GFP_ATOMIC);
#else
    ret = oal_cfg80211_new_sta(pst_net_device, &auc_connect_user_addr[0], &st_station_info, GFP_ATOMIC);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                         "{wal_connect_new_sta_proc_ap::oal_cfg80211_new_sta fail[%d]!}", ret);
        return ret;
    }
#endif

    oal_wifi_report_sta_action(pst_net_device->ifindex, OAL_WIFI_STA_JOIN, auc_connect_user_addr, OAL_MAC_ADDR_LEN);

    oam_warning_log4(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                     "{wal_connect_new_sta_proc_ap mac[%02X:XX:XX:%02X:%02X:%02X] OK!}",
                     auc_connect_user_addr[MAC_ADDR_0],
                     auc_connect_user_addr[MAC_ADDR_3],
                     auc_connect_user_addr[MAC_ADDR_4],
                     auc_connect_user_addr[MAC_ADDR_5]);

    return OAL_SUCC;
}


uint32_t wal_disconnect_sta_proc_ap(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *pst_net_device = NULL;
    uint8_t auc_disconn_user_addr[WLAN_MAC_ADDR_LEN];
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 44))
    int32_t l_ret;
#endif

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_disconnect_sta_proc_ap::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 获取net_device */
    pst_net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (pst_net_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                       "{wal_disconnect_sta_proc_ap::get net device ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取去关联user mac addr */
    if (EOK != memcpy_s(auc_disconn_user_addr, WLAN_MAC_ADDR_LEN,
                        (uint8_t *)pst_event->auc_event_data, WLAN_MAC_ADDR_LEN)) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ANY, "{wal_disconnect_sta_proc_ap:memcopy fail!}");
        return OAL_FAIL;
    }

    /* 调用内核接口，上报STA去关联结果 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    oal_cfg80211_del_sta(pst_net_device, auc_disconn_user_addr, GFP_ATOMIC);
#else
    l_ret = oal_cfg80211_del_sta(pst_net_device, &auc_disconn_user_addr[0], GFP_ATOMIC);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                         "{wal_disconnect_sta_proc_ap::oal_cfg80211_del_sta return fail[%d]!}", l_ret);
        return OAL_FAIL;
    }
#endif
    oam_warning_log3(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                     "{wal_disconnect_sta_proc_ap mac[%x %x %x] OK!}",
                     auc_disconn_user_addr[MAC_ADDR_3], auc_disconn_user_addr[MAC_ADDR_4],
                     auc_disconn_user_addr[MAC_ADDR_5]);

    oal_wifi_report_sta_action(pst_net_device->ifindex, OAL_WIFI_STA_LEAVE, auc_disconn_user_addr, OAL_MAC_ADDR_LEN);

    return OAL_SUCC;
}


uint32_t wal_mic_failure_proc(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *pst_net_device = NULL;
    hmac_mic_event_stru *pst_mic_event = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_CRYPTO, "{wal_mic_failure_proc::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_mic_event = (hmac_mic_event_stru *)(pst_event->auc_event_data);

    /* 获取net_device */
    pst_net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (pst_net_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_CRYPTO,
                       "{wal_mic_failure_proc::get net device ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 调用内核接口，上报mic攻击 */
    oal_cfg80211_mic_failure(pst_net_device, pst_mic_event->auc_user_mac,
                             pst_mic_event->en_key_type, pst_mic_event->l_key_id,
                             NULL, GFP_ATOMIC);

    oam_warning_log3(pst_event->st_event_hdr.uc_vap_id, OAM_SF_CRYPTO, "{wal_mic_failure_proc::mac[%x %x %x] OK!}",
                     pst_mic_event->auc_user_mac[MAC_ADDR_3],
                     pst_mic_event->auc_user_mac[MAC_ADDR_4],
                     pst_mic_event->auc_user_mac[MAC_ADDR_5]);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_NAN
OAL_STATIC oal_netbuf_stru *wal_nan_alloc_rx_event(int32_t mem_needed, oal_gfp_enum_uint8 kflags)
{
    oal_netbuf_stru *skb = NULL;
    oal_net_device_stru *nan0_dev = NULL;
    oal_wireless_dev_stru *wdev = NULL;

    /* 使用nan0上报vendor event */
    nan0_dev = oal_dev_get_by_name("nan0");
    if (!nan0_dev) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_nan_rx_send_event::cannot find nan netdev}");
        return NULL;
    }

    wdev = oal_netdevice_wdev(nan0_dev);
    if (wdev == NULL || wdev->wiphy == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "wal_nan_rx_mgmt::wdev or wdev->wiphy is null");
        oal_dev_put(nan0_dev);
        return NULL;
    }

    skb = oal_cfg80211_vendor_event_alloc(wdev->wiphy, wdev, mem_needed, NAN_EVENT_RX, kflags);
    oal_dev_put(nan0_dev); /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    return skb;
}

uint32_t wal_nan_send_response(int32_t attr, uint16_t transaction)
{
    oal_netbuf_stru *skb;
    int32_t mem_needed;
    oal_gfp_enum_uint8 kflags = oal_in_atomic() ? GFP_ATOMIC : GFP_KERNEL;

    mem_needed = VENDOR_DATA_OVERHEAD + oal_nlmsg_length(sizeof(transaction));
    skb = wal_nan_alloc_rx_event(mem_needed, kflags);
    if (oal_unlikely(!skb)) {
        oam_warning_log2(0, OAM_SF_ANY, "wal_nan_rx_mgmt::skb alloc failed, len %d, flags %d", mem_needed, kflags);
        return -OAL_ENOMEM;
    }

    oal_nla_put_u16(skb, attr, transaction);
    oal_cfg80211_vendor_event(skb, kflags);
    return OAL_SUCC;
}

OAL_STATIC int32_t wal_nan_tx_mgmt_get_rsp_attr(uint8_t action)
{
    int32_t attr;
    switch (action) {
        case WLAN_ACTION_NAN_PUBLISH:
            attr = NAN_ATTRIBUTE_PUBLISH_SERVICE_RSP;
            break;
        case WLAN_ACTION_NAN_SUBSCRIBE:
            attr = NAN_ATTRIBUTE_SUBSCRIBE_SERVICE_RSP;
            break;
        case WLAN_ACTION_NAN_FLLOWUP:
            attr = NAN_ATTRIBUTE_TRANSMIT_FOLLOWUP_RSP;
            break;
        default:
            attr = NAN_ATTRIBUTE_ERROR_RSP;
    }
    return attr;
}

uint32_t wal_nan_response_event_process(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    mac_nan_rsp_msg_stru *nan_rsp;
    int32_t attr = NAN_ATTRIBUTE_ERROR_RSP;

    event = frw_get_event_stru(event_mem);
    nan_rsp = (mac_nan_rsp_msg_stru*)(event->auc_event_data);

    if (nan_rsp->status) {
        if (nan_rsp->type == NAN_RSP_TYPE_SET_PARAM) {
            attr = NAN_ATTRIBUTE_CAPABILITIES_RSP;
        } else if (nan_rsp->type == NAN_RSP_TYPE_SET_TX_MGMT) {
            attr = wal_nan_tx_mgmt_get_rsp_attr(nan_rsp->action);
        } else {
            attr = NAN_ATTRIBUTE_ERROR_RSP;
        }
    } else {
        attr = NAN_ATTRIBUTE_ERROR_RSP;
    }
    return wal_nan_send_response(attr, nan_rsp->transaction);
}
#endif



void wal_nan_rx_mgmt(oal_net_device_stru *net_device, uint8_t *buf, uint16_t len)
{
#ifdef _PRE_WLAN_FEATURE_NAN
    int32_t mem_needed;
    oal_netbuf_stru *skb = NULL;
    oal_gfp_enum_uint8 kflags = oal_in_atomic() ? GFP_ATOMIC : GFP_KERNEL;

    mem_needed = VENDOR_DATA_OVERHEAD + oal_nlmsg_length(len);
    skb = wal_nan_alloc_rx_event(mem_needed, kflags);
    if (oal_unlikely(!skb)) {
        oam_warning_log2(0, OAM_SF_ANY, "wal_nan_rx_mgmt::skb alloc failed, len %d, flags %d", mem_needed, kflags);
        return;
    }

    oal_nla_put(skb, NAN_ATTRIBUTE_RECEIVE_DATA, len, buf);

    oal_cfg80211_vendor_event(skb, kflags);
    return;
#endif
}


uint32_t wal_send_mgmt_to_host(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *pst_net_device = NULL;
    int32_t l_freq;
    uint8_t uc_rssi;
    uint8_t *puc_buf = NULL;
    uint16_t us_len;
    uint32_t ret;
    hmac_rx_mgmt_event_stru *pst_mgmt_frame = NULL;
    oal_ieee80211_mgmt *pst_ieee80211_mgmt = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_send_mgmt_to_host::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_mgmt_frame = (hmac_rx_mgmt_event_stru *)(pst_event->auc_event_data);

    /* 获取net_device */
    pst_net_device = wal_config_get_netdev(pst_mgmt_frame->ac_name, OAL_STRLEN(pst_mgmt_frame->ac_name));
    if (pst_net_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ANY,
                       "{wal_send_mgmt_to_host::get net device ptr is null!}");
        oal_free(pst_mgmt_frame->puc_buf);
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_dev_put(pst_net_device);

    puc_buf = pst_mgmt_frame->puc_buf;
    us_len = pst_mgmt_frame->us_len;
    l_freq = pst_mgmt_frame->l_freq;
    uc_rssi = pst_mgmt_frame->uc_rssi;

    pst_ieee80211_mgmt = (oal_ieee80211_mgmt *)puc_buf;
    if (pst_mgmt_frame->event_type == HMAC_RX_MGMT_EVENT_TYPE_NAN) {
        wal_nan_rx_mgmt(pst_net_device, puc_buf, us_len);
    } else {
        /**********************************************************************************************
            NOTICE:  AUTH ASSOC DEAUTH DEASSOC 这几个帧上报给host的时候 可能给ioctl调用死锁
                     需要添加这些帧上报的时候 请使用workqueue来上报
        ***********************************************************************************************/
        /* 调用内核接口，上报接收到管理帧 */
        ret = oal_cfg80211_rx_mgmt(pst_net_device, l_freq, uc_rssi, puc_buf, us_len, GFP_ATOMIC);
        if (ret != OAL_SUCC) {
            oam_warning_log2(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ANY,
                             "{wal_send_mgmt_to_host::fc[0x%02x], if_type[%d]!}\r\n",
                             pst_ieee80211_mgmt->frame_control, pst_net_device->ieee80211_ptr->iftype);
            oam_warning_log3(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ANY,
                             "{wal_send_mgmt_to_host::oal_cfg80211_rx_mgmt fail[%d]!len[%d], freq[%d]}",
                             ret, us_len, l_freq);
            oal_free(puc_buf);
            return OAL_FAIL;
        }
    }
    oal_free(puc_buf);
    return OAL_SUCC;
}


uint32_t wal_p2p_listen_timeout(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_wireless_dev_stru *pst_wdev = NULL;
    uint64_t ull_cookie;
    oal_ieee80211_channel_stru st_listen_channel;
    hmac_p2p_listen_expired_stru *pst_p2p_listen_expired = NULL;
    mac_device_stru *pst_mac_device = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_P2P, "{wal_p2p_listen_timeout::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    /* 获取mac_device_stru */
    pst_mac_device = mac_res_get_dev(pst_event->st_event_hdr.uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_PROXYSTA,
                       "{wal_p2p_listen_timeout::get mac_device ptr is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_p2p_listen_expired = (hmac_p2p_listen_expired_stru *)(pst_event->auc_event_data);

    pst_wdev = pst_p2p_listen_expired->pst_wdev;
    ull_cookie = pst_mac_device->st_p2p_info.ull_last_roc_id;
    st_listen_channel = pst_p2p_listen_expired->st_listen_channel;
    oal_cfg80211_remain_on_channel_expired(pst_wdev, ull_cookie, &st_listen_channel, GFP_ATOMIC);
    if (!hmac_get_feature_switch(HMAC_MIRACAST_REDUCE_LOG_SWITCH)) {
        oam_warning_log1(0, OAM_SF_P2P, "{wal_p2p_listen_timeout::END!cookie [%x]}\r\n", ull_cookie);
    }
    return OAL_SUCC;
}


uint32_t wal_report_external_auth_req(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *pst_net_device = NULL;
    hmac_external_auth_req_stru *pst_ext_auth_req = NULL;
    oal_cfg80211_external_auth_stru st_external_auth_req;
    int l_ret;
    int32_t l_memcpy_ret;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_SAE, "{wal_report_external_auth_req::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 获取net_device */
    pst_net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (pst_net_device == NULL) {
        oam_error_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_SAE,
                       "{wal_report_external_auth_req::get net device ptr is null! vap_id %d}",
                       pst_event->st_event_hdr.uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ext_auth_req = (hmac_external_auth_req_stru *)(pst_event->auc_event_data);

    st_external_auth_req.action = pst_ext_auth_req->en_action;
    st_external_auth_req.key_mgmt_suite = pst_ext_auth_req->key_mgmt_suite;
    st_external_auth_req.status = pst_ext_auth_req->us_status;
    st_external_auth_req.ssid.ssid_len = oal_min(pst_ext_auth_req->st_ssid.uc_ssid_len, OAL_IEEE80211_MAX_SSID_LEN);
    l_memcpy_ret = memcpy_s(st_external_auth_req.ssid.ssid, sizeof(st_external_auth_req.ssid.ssid),
                            pst_ext_auth_req->st_ssid.auc_ssid, st_external_auth_req.ssid.ssid_len);
    l_memcpy_ret += memcpy_s(st_external_auth_req.bssid, OAL_ETH_ALEN_SIZE, pst_ext_auth_req->auc_bssid, OAL_ETH_ALEN);
    if (l_memcpy_ret != EOK) {
        oam_error_log0(0, OAM_SF_SAE, "wal_report_external_auth_req::memcpy fail!");
        return OAL_FAIL;
    }

    l_ret = oal_cfg80211_external_auth_request(pst_net_device,
                                               &st_external_auth_req,
                                               GFP_ATOMIC);

    oam_warning_log4(pst_event->st_event_hdr.uc_vap_id, OAM_SF_SAE,
                     "{wal_report_external_auth_req::action %x, status %d, key_mgmt 0x%X, ssid_len %d}",
                     st_external_auth_req.action,
                     st_external_auth_req.status,
                     st_external_auth_req.key_mgmt_suite,
                     st_external_auth_req.ssid.ssid_len);

    oam_warning_log4(pst_event->st_event_hdr.uc_vap_id, OAM_SF_SAE,
                     "{wal_report_external_auth_req::mac[%02X:XX:XX:XX:%02X:%02X], ret[%d]}",
                     st_external_auth_req.bssid[MAC_ADDR_0],
                     st_external_auth_req.bssid[MAC_ADDR_4],
                     st_external_auth_req.bssid[MAC_ADDR_5],
                     l_ret);

    return l_ret;
}


uint32_t wal_report_channel_switch(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *pst_net_device = NULL;
    hmac_channel_switch_stru *pst_channel_switch = NULL;
    oal_cfg80211_chan_def st_chandef = {0};
    hmac_device_stru *pst_hmac_device = NULL;
    oal_wiphy_stru *pst_wiphy = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_CHAN, "{wal_report_channel_switch::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 获取net_device */
    pst_net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (oal_unlikely(pst_net_device == NULL)) {
        oam_error_log1(0, OAM_SF_CHAN, "{wal_report_channel_switch::get net device ptr is null! vap_id %d}",
                       pst_event->st_event_hdr.uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取hmac_device */
    pst_hmac_device = hmac_res_get_mac_dev(pst_event->st_event_hdr.uc_device_id);
    if (oal_unlikely(pst_hmac_device == NULL)) {
        oam_error_log1(0, OAM_SF_CHAN, "{wal_report_channel_switch::get pst_hmac_device is null! device_id %d}",
                       pst_event->st_event_hdr.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取wiphy */
    pst_wiphy = pst_hmac_device->pst_device_base_info->pst_wiphy;
    if (oal_unlikely(pst_wiphy == NULL)) {
        oam_error_log0(0, OAM_SF_CHAN, "{wal_report_channel_switch:: pst_wiphy is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_channel_switch = (hmac_channel_switch_stru *)(pst_event->auc_event_data);

    st_chandef.chan = oal_ieee80211_get_channel(pst_wiphy, pst_channel_switch->center_freq);
    if (oal_unlikely(st_chandef.chan == NULL)) {
        oam_error_log1(0, OAM_SF_CHAN, "{wal_report_channel_switch::can't get chan pointer from wiphy(freq=%d)!}",
            pst_channel_switch->center_freq);
        return OAL_FAIL;
    }
    st_chandef.chan->center_freq = pst_channel_switch->center_freq;
    st_chandef.width = pst_channel_switch->width;
    st_chandef.center_freq1 = pst_channel_switch->center_freq1;
    st_chandef.center_freq2 = pst_channel_switch->center_freq2;

    oal_cfg80211_ch_switch_notify(pst_net_device, &st_chandef);

    oam_warning_log4(pst_event->st_event_hdr.uc_vap_id, OAM_SF_CHAN,
                     "{wal_report_channel_switch::center_freq=%d, width=%d, center_freq1=%d, center_freq2=%d}",
                     st_chandef.chan->center_freq, st_chandef.width,
                     st_chandef.center_freq1, st_chandef.center_freq2);

    wal_update_bss_for_csa(pst_wiphy, pst_net_device);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_FTM

uint32_t wal_report_rtt_result(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    mac_ftm_wifi_rtt_result* wifi_rtt_result;
    oal_net_device_stru *net_dev;
    oal_wireless_dev_stru *wdev;
    oal_netbuf_stru *skb;
    int32_t mem_needed;
    oal_gfp_enum_uint8 kflags = oal_in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
    oal_nlattr_stru *rtt_nl_hdr = NULL;

    event = frw_get_event_stru(event_mem);
    wifi_rtt_result = (mac_ftm_wifi_rtt_result*)(event->auc_event_data);
    if (wifi_rtt_result == NULL) {
        oam_warning_log0(0, OAM_SF_FTM, "{wal_report_rtt_result::wifi_rtt_result NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    net_dev = hmac_vap_get_net_device(event->st_event_hdr.uc_vap_id);
    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_FTM, "{wal_report_rtt_result::cannot find netdev}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    wdev = oal_netdevice_wdev(net_dev);
    if (wdev == NULL || wdev->wiphy == NULL) {
        oam_error_log0(0, OAM_SF_FTM, "wal_report_rtt_result::wdev or wdev->wiphy is null");
        oal_dev_put(net_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }
    mem_needed = VENDOR_DATA_OVERHEAD + oal_nlmsg_length(sizeof(mac_ftm_wifi_rtt_result));
    skb = oal_cfg80211_vendor_event_alloc(wdev->wiphy, wdev, mem_needed, VENDOR_RTT_COMPLETE_EVENT, kflags);
    if (oal_unlikely(!skb)) {
        oam_warning_log2(0, OAM_SF_FTM, "wal_report_rtt_result::skb alloc failed len:%d flags:%d", mem_needed, kflags);
        return -OAL_ENOMEM;
    }
    oal_nla_put_u32(skb, RTT_ATTRIBUTE_RESULTS_COMPLETE, 1);
    rtt_nl_hdr = oal_nla_nest_start(skb, RTT_ATTRIBUTE_RESULTS_PER_TARGET);
    if (!rtt_nl_hdr) {
        oam_warning_log0(0, 0, "wal_report_rtt_result::rtt_nl_hdr is NULL");
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_nla_put(skb, RTT_ATTRIBUTE_TARGET_MAC, ETHER_ADDR_LEN, &wifi_rtt_result->addr);
    oal_nla_put_u32(skb, RTT_ATTRIBUTE_RESULT_CNT, 1);
    oal_nla_put(skb, RTT_ATTRIBUTE_RESULT, sizeof(mac_ftm_wifi_rtt_result), (uint8_t*)wifi_rtt_result);
    oal_nla_nest_end(skb, rtt_nl_hdr);
    oam_warning_log3(0, OAM_SF_FTM, "wal_report_rtt_result::status:%d negotiated_num:%d succ_number:%d",
        wifi_rtt_result->status, wifi_rtt_result->negotiated_burst_num, wifi_rtt_result->success_number);
    oam_warning_log4(0, OAM_SF_FTM, "wal_report_rtt_result::rssi:%d distance_mm:%d distance_sd_mm:%d distance_sp_mm:%d",
        wifi_rtt_result->rssi, wifi_rtt_result->distance_mm, wifi_rtt_result->distance_sd_mm,
        wifi_rtt_result->distance_spread_mm);
    oal_cfg80211_vendor_event(skb, kflags);

    return OAL_SUCC;
}
#endif
