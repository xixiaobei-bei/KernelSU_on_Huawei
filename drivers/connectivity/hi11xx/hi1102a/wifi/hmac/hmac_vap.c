

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_net.h"
#include "oam_ext_if.h"

#include "mac_ie.h"

#include "frw_event_main.h"

#include "hmac_vap.h"
#include "hmac_resource.h"
#include "hmac_tx_amsdu.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_fsm.h"
#include "hmac_ext_if.h"
#include "hmac_chan_mgmt.h"
#include "hmac_sme_sta.h"
#include "hmac_edca_opt.h"
#include "hmac_blockack.h"
#include "hmac_p2p.h"
#include "hmac_ftm.h"
#ifdef _PRE_WLAN_TCP_OPT
#include "hmac_tcp_opt.h"
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
#include "hmac_roam_main.h"
#endif
#include "hmac_mgmt_sta.h"
#include "hmac_mgmt_ap.h"
#include "hmac_tcp_ack_filter.h"
#include "hmac_204080_coexist.h"
#include "hmac_sae.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_VAP_C
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
hmac_vap_stru g_ast_hmac_vap[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
#endif

#define HMAC_NETDEVICE_WDT_TIMEOUT (5 * OAL_TIME_HZ)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
OAL_STATIC oal_int32 hmac_cfg_vap_if_open(oal_net_device_stru *pst_dev);
OAL_STATIC oal_int32 hmac_cfg_vap_if_close(oal_net_device_stru *pst_dev);
OAL_STATIC oal_net_dev_tx_enum hmac_cfg_vap_if_xmit(oal_netbuf_stru *pst_buf, oal_net_device_stru *pst_dev);
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OAL_STATIC oal_net_device_ops_stru g_st_vap_net_dev_cfg_vap_ops = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    .ndo_open = hmac_cfg_vap_if_open,
    .ndo_stop = hmac_cfg_vap_if_close,
    .ndo_start_xmit = hmac_cfg_vap_if_xmit,
#else
    .ndo_get_stats = oal_net_device_get_stats,
    .ndo_open = oal_net_device_open,
    .ndo_stop = oal_net_device_close,
    .ndo_start_xmit = OAL_PTR_NULL,
    .ndo_set_multicast_list = OAL_PTR_NULL,
    .ndo_do_ioctl = oal_net_device_ioctl,
    .ndo_change_mtu = oal_net_device_change_mtu,
    .ndo_init = oal_net_device_init,
    .ndo_set_mac_address = oal_net_device_set_macaddr
#endif
};
#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
OAL_STATIC oal_net_device_ops_stru g_st_vap_net_dev_cfg_vap_ops = {
    oal_net_device_init,
    oal_net_device_open,
    oal_net_device_close,
    OAL_PTR_NULL,
    OAL_PTR_NULL,
    oal_net_device_get_stats,
    oal_net_device_ioctl,
    oal_net_device_change_mtu,
    oal_net_device_set_macaddr
};
#endif
oal_uint8 g_uc_host_rx_ampdu_amsdu = OAL_FALSE;

#ifdef _PRE_WLAN_FEATURE_P2P
extern oal_void hmac_del_virtual_inf_worker(oal_work_stru *pst_del_virtual_inf_work);
#endif

#ifdef _PRE_WLAN_FEATURE_SAE
oal_void hmac_report_ext_auth_worker(oal_work_stru *pst_sae_report_ext_auth_worker);
#endif
/*****************************************************************************
  3 函数实现
*****************************************************************************/
oal_void hmac_vap_ampdu_amsdu_init(hmac_vap_stru *pst_hmac_vap)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    mac_tx_large_amsdu_ampdu_stru *tx_large_amsdu = mac_get_tx_large_amsdu_addr();
    /* 从定制化获取是否开启AMSDU_AMPDU */
    pst_hmac_vap->en_amsdu_ampdu_active = tx_large_amsdu->uc_tx_amsdu_ampdu_en;
    // 1102A默认支持接收方向的AMPDU+AMSDU联合聚合,通过定制化决定
    pst_hmac_vap->en_rx_ampduplusamsdu_active = g_uc_host_rx_ampdu_amsdu;
#endif
#endif
#else
    pst_hmac_vap->en_amsdu_ampdu_active = OAL_FALSE;
    pst_hmac_vap->en_rx_ampduplusamsdu_active = OAL_FALSE;
#endif
    pst_hmac_vap->en_ampdu_tx_on_switch = OAL_TRUE;
    pst_hmac_vap->en_small_amsdu_switch = OAL_FALSE;
}
OAL_STATIC oal_void hmac_vap_set_auth_open(hmac_vap_stru *pst_hmac_vap)
{
    pst_hmac_vap->en_auth_mode = WLAN_WITP_AUTH_OPEN_SYSTEM;
    pst_hmac_vap->uc_80211i_mode = OAL_FALSE;
    pst_hmac_vap->en_psm_active = WLAN_FEATURE_PSM_IS_OPEN;
    pst_hmac_vap->en_wme_active = WLAN_FEATURE_WME_IS_OPEN;
    pst_hmac_vap->en_msdu_defrag_active = WLAN_FEATURE_MSDU_DEFRAG_IS_OPEN;
}
OAL_STATIC oal_void hmac_vap_rx_timeout_init(hmac_vap_stru *pst_hmac_vap)
{
    pst_hmac_vap->us_rx_timeout[WLAN_WME_AC_BK] = HMAC_BA_RX_BK_TIMEOUT;
    pst_hmac_vap->us_rx_timeout[WLAN_WME_AC_BE] = HMAC_BA_RX_BE_TIMEOUT;
    pst_hmac_vap->us_rx_timeout[WLAN_WME_AC_VI] = HMAC_BA_RX_VI_TIMEOUT;
    pst_hmac_vap->us_rx_timeout[WLAN_WME_AC_VO] = HMAC_BA_RX_VO_TIMEOUT;

    pst_hmac_vap->us_rx_timeout_min[WLAN_WME_AC_BK] = HMAC_BA_RX_BK_TIMEOUT_MIN;
    pst_hmac_vap->us_rx_timeout_min[WLAN_WME_AC_BE] = HMAC_BA_RX_BE_TIMEOUT_MIN;
    pst_hmac_vap->us_rx_timeout_min[WLAN_WME_AC_VI] = HMAC_BA_RX_VI_TIMEOUT_MIN;
    pst_hmac_vap->us_rx_timeout_min[WLAN_WME_AC_VO] = HMAC_BA_RX_VO_TIMEOUT_MIN;
}
OAL_STATIC oal_void hmac_vap_kvr_init(hmac_vap_stru *pst_hmac_vap, mac_cfg_add_vap_param_stru *pst_param)
{
    mac_device_voe_custom_stru *pst_voe_custom_param = mac_get_voe_custom_param_addr();

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (pst_param->en_p2p_mode == WLAN_LEGACY_VAP_MODE && IS_STA(&(pst_hmac_vap->st_vap_base_info))) {
#ifdef _PRE_WLAN_FEATURE_11K
        pst_hmac_vap->bit_11k_enable = pst_voe_custom_param->en_11k;
        pst_hmac_vap->bit_bcn_table_switch = pst_voe_custom_param->en_11k;
        mac_mib_set_dot11RadioMeasurementActivated(&(pst_hmac_vap->st_vap_base_info), pst_hmac_vap->bit_11k_enable);
        mac_mib_set_dot11RMBeaconTableMeasurementActivated(&(pst_hmac_vap->st_vap_base_info),
                                                           pst_hmac_vap->bit_bcn_table_switch);
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
        hmac_vap_init_gas(pst_hmac_vap);
#endif
#ifdef _PRE_WLAN_FEATURE_11R
        pst_hmac_vap->bit_11r_enable = pst_voe_custom_param->en_11r;
#endif
#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
        pst_hmac_vap->bit_11v_enable = pst_voe_custom_param->en_11v;
        /* 根据定制化参数修改MIB项 */
        mac_mib_set_MgmtOptionBSSTransitionActivated(&(pst_hmac_vap->st_vap_base_info), pst_hmac_vap->bit_11v_enable);
#endif
    }
#endif
}

OAL_STATIC oal_void hmac_vap_edca_opt_ap_init(hmac_vap_stru *pst_hmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    pst_hmac_vap->ul_edca_opt_time_ms = HMAC_EDCA_OPT_TIME_MS;
    frw_create_timer(&(pst_hmac_vap->st_edca_opt_timer), hmac_edca_opt_timeout_fn,
                     pst_hmac_vap->ul_edca_opt_time_ms, pst_hmac_vap, OAL_TRUE,
                     OAM_MODULE_ID_ALG_SCHEDULE, pst_hmac_vap->st_vap_base_info.ul_core_id);
    /* also open for 1102 at 2015-10-16 */
    pst_hmac_vap->uc_edca_opt_flag_ap = 1;
    frw_timer_restart_timer(&(pst_hmac_vap->st_edca_opt_timer), pst_hmac_vap->ul_edca_opt_time_ms, OAL_TRUE);
#endif
}
OAL_STATIC oal_void hmac_vap_sta_init(hmac_vap_stru *pst_hmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    pst_hmac_vap->uc_edca_opt_flag_sta = 0;
    pst_hmac_vap->uc_edca_opt_weight_sta = WLAN_EDCA_OPT_WEIGHT_STA;
#endif
    pst_hmac_vap->bit_sta_protocol_cfg = OAL_SWITCH_OFF;
#ifdef _PRE_WLAN_FEATURE_STA_PM
    pst_hmac_vap->uc_cfg_sta_pm_manual = 0xFF;
#endif
}
#ifdef _PRE_WLAN_FEATURE_SAE
static void hmac_vap_sae_init(hmac_vap_stru *hmac_vap)
{
    oal_init_work(&(hmac_vap->st_sae_report_ext_auth_worker), hmac_report_ext_auth_worker);
    hmac_vap->duplicate_auth_seq2_flag = OAL_FALSE;
    hmac_vap->duplicate_auth_seq4_flag = OAL_FALSE;
}
#endif


oal_uint32 hmac_vap_init(
    hmac_vap_stru *pst_hmac_vap, oal_uint8 uc_chip_id, oal_uint8 uc_device_id,
    oal_uint8 uc_vap_id, mac_cfg_add_vap_param_stru *pst_param)
{
    oal_uint32 ul_ret;

    if (oal_unlikely(pst_hmac_vap == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = mac_vap_init(&pst_hmac_vap->st_vap_base_info, uc_chip_id, uc_device_id, uc_vap_id, pst_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        return ul_ret;
    }

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    /* 统计信息清零 */
    oam_stats_clear_vap_stat_info(uc_vap_id);
#endif

    /* 初始化预设参数 */
    pst_hmac_vap->st_preset_para.en_protocol = pst_hmac_vap->st_vap_base_info.en_protocol;
    pst_hmac_vap->st_preset_para.en_bandwidth = pst_hmac_vap->st_vap_base_info.st_channel.en_bandwidth;
    pst_hmac_vap->st_preset_para.en_band = pst_hmac_vap->st_vap_base_info.st_channel.en_band;
    /* 初始化配置私有结构体 */
    // 对于P2P CL 不能再初始化队列
    pst_hmac_vap->st_cfg_priv.dog_tag = OAL_DOG_TAG;
    oal_wait_queue_init_head(&(pst_hmac_vap->st_cfg_priv.st_wait_queue_for_sdt_reg));
    oal_wait_queue_init_head(&(pst_hmac_vap->st_mgmt_tx.st_wait_queue));

    /* 默认设置为自动触发BA回话的建立 */
    pst_hmac_vap->en_addba_mode = HMAC_ADDBA_MODE_AUTO;

    /* 1151默认不amsdu ampdu 联合聚合功能不开启 1102用于小包优化
     * 因tplink/syslink下行冲包兼容性问题，先关闭02的ampdu+amsdu
 */
    hmac_vap_ampdu_amsdu_init(pst_hmac_vap);
    pst_hmac_vap->uc_ip_addr_obtained_num = 0;
    /* 初始化认证类型为OPEN */
    hmac_vap_set_auth_open(pst_hmac_vap);
    /* 2040共存信道切换开关初始化 */
    pst_hmac_vap->en_2040_switch_prohibited = OAL_FALSE;
    pst_hmac_vap->en_wps_active = OAL_FALSE;

    /* 设置vap最大用户数 */
    pst_hmac_vap->us_user_nums_max = WLAN_ASSOC_USER_MAX_NUM_SPEC;

#ifdef _PRE_WLAN_FEATURE_AMPDU_VAP
    pst_hmac_vap->uc_rx_ba_session_num = 0;
    pst_hmac_vap->uc_tx_ba_session_num = 0;
#endif

#ifdef _PRE_WLAN_FEATURE_11D
    pst_hmac_vap->en_updata_rd_by_ie_switch = OAL_FALSE;
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
    hmac_roam_ping_pong_clear(pst_hmac_vap);
#endif

    oal_wait_queue_init_head(&pst_hmac_vap->query_wait_q);
    oal_wait_queue_init_head(&pst_hmac_vap->st_chr_wifi_ext_info_query.st_query_chr_wait_q);

    /* 根据配置VAP，将对应函数挂接在业务VAP，区分AP、STA和WDS模式 */
    switch (pst_param->en_vap_mode) {
        case WLAN_VAP_MODE_BSS_AP:
            hmac_vap_edca_opt_ap_init(pst_hmac_vap);
            break;

        case WLAN_VAP_MODE_BSS_STA:
            hmac_vap_sta_init(pst_hmac_vap);
            break;

        case WLAN_VAP_MODE_CONFIG:
            /* 配置VAP直接返回 */
            return OAL_SUCC;

        default:
            return OAL_ERR_CODE_INVALID_CONFIG;
    }

    oal_spin_lock_init(&pst_hmac_vap->st_lock_state);
    oal_dlist_init_head(&(pst_hmac_vap->st_pmksa_list_head));

    /* 创建vap时 初始状态为init */
    mac_vap_state_change(&(pst_hmac_vap->st_vap_base_info), MAC_VAP_STATE_INIT);

    /* 初始化重排序超时时间 */
    hmac_vap_rx_timeout_init(pst_hmac_vap);
#ifdef _PRE_WLAN_FEATURE_P2P
    /* 初始化删除虚拟网络接口工作队列 */
    oal_init_work(&(pst_hmac_vap->st_del_virtual_inf_worker), hmac_del_virtual_inf_worker);
    pst_hmac_vap->pst_del_net_device = OAL_PTR_NULL;
    pst_hmac_vap->pst_p2p0_net_device = OAL_PTR_NULL;
#endif
#ifdef _PRE_WLAN_FEATURE_SAE
    hmac_vap_sae_init(pst_hmac_vap);
#endif
#ifdef _PRE_WLAN_TCP_OPT
    pst_hmac_vap->ast_hmac_tcp_ack[HCC_TX].filter_info.ul_ack_limit = DEFAULT_TX_TCP_ACK_THRESHOLD;
    pst_hmac_vap->ast_hmac_tcp_ack[HCC_RX].filter_info.ul_ack_limit = DEFAULT_RX_TCP_ACK_THRESHOLD;
    hmac_tcp_opt_init_filter_tcp_ack_pool(pst_hmac_vap);
#endif
    hmac_vap_kvr_init(pst_hmac_vap, pst_param);
    g_aus_tcp_ack_filter_th[0][0] = g_ul_2g_tx_large_pps_th;
    g_aus_tcp_ack_filter_th[1][0] = g_ul_5g_tx_large_pps_th;

    return OAL_SUCC;
}


oal_uint32 hmac_vap_get_priv_cfg(mac_vap_stru *pst_mac_vap, hmac_vap_cfg_priv_stru **ppst_cfg_priv)
{
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;

    if (oal_unlikely(pst_mac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_vap_get_priv_cfg::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (oal_unlikely(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_vap_get_priv_cfg::mac_res_get_hmac_vap fail.vap_id = %u}",
                       pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    *ppst_cfg_priv = &pst_hmac_vap->st_cfg_priv;

    return OAL_SUCC;
}


oal_int8 *hmac_vap_get_desired_country(oal_uint8 uc_vap_id)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{hmac_vap_get_desired_country::pst_hmac_vap null.}");
        return OAL_PTR_NULL;
    }

    return pst_hmac_vap->ac_desired_country;
}
#ifdef _PRE_WLAN_FEATURE_11D

oal_uint32 hmac_vap_get_updata_rd_by_ie_switch(oal_uint8 uc_vap_id, oal_bool_enum_uint8 *us_updata_rd_by_ie_switch)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(0);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_vap_get_updata_rd_by_ie_switch::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *us_updata_rd_by_ie_switch = pst_hmac_vap->en_updata_rd_by_ie_switch;
    return OAL_SUCC;
}
#endif

oal_net_device_stru *hmac_vap_get_net_device(oal_uint8 uc_vap_id)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{hmac_vap_get_net_device::pst_hmac_vap null.}");
        return OAL_PTR_NULL;
    }

    return (pst_hmac_vap->pst_net_device);
}


oal_uint32 hmac_vap_free_asoc_req_ie_ptr(oal_uint8 uc_vap_id)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ASSOC, "{hmac_vap_free_asoc_req_ie_ptr::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_hmac_vap->puc_asoc_req_ie_buff != OAL_PTR_NULL) {
        oal_mem_free_m(pst_hmac_vap->puc_asoc_req_ie_buff, OAL_TRUE);
    }
    pst_hmac_vap->puc_asoc_req_ie_buff = OAL_PTR_NULL;
    pst_hmac_vap->ul_asoc_req_ie_len = 0;

    return OAL_SUCC;
}
OAL_STATIC oal_void hmac_vap_netdev_init(oal_net_device_stru *pst_net_device)
{
    oal_netdevice_ops(pst_net_device) = &g_st_vap_net_dev_cfg_vap_ops;
    oal_netdevice_destructor(pst_net_device) = oal_net_free_netdev;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
#else
    oal_netdevice_master(pst_net_device) = OAL_PTR_NULL;
#endif

    oal_netdevice_ifalias(pst_net_device) = OAL_PTR_NULL;
    oal_netdevice_watchdog_timeo(pst_net_device) = HMAC_NETDEVICE_WDT_TIMEOUT;
}

oal_uint32 hmac_vap_creat_netdev(hmac_vap_stru *pst_hmac_vap, oal_int8 *puc_netdev_name,
                                 oal_uint32 ul_netdev_name_len, oal_int8 *puc_mac_addr)
{
    oal_net_device_stru *pst_net_device = OAL_PTR_NULL;
    oal_uint32 ul_return;
    mac_vap_stru *pst_vap = OAL_PTR_NULL;
    oal_int32 l_ret;

    if (oal_unlikely((pst_hmac_vap == OAL_PTR_NULL) || (puc_netdev_name == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_ASSOC, "{hmac_vap_creat_netdev::param null %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)puc_netdev_name);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (ul_netdev_name_len > MAC_NET_DEVICE_NAME_LENGTH) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_vap_creat_netdev::netdev_name_len[%d] is over max limit!}",
                         ul_netdev_name_len);
        return OAL_FAIL;
    }

    pst_vap = &pst_hmac_vap->st_vap_base_info;

    pst_net_device = oal_net_alloc_netdev(0, puc_netdev_name, oal_ether_setup);
    if (oal_unlikely(pst_net_device == OAL_PTR_NULL)) {
        MAC_WARNING_LOG(pst_vap->uc_vap_id, "mac_device_init:p_vap_netdev_alloc return fail.");
        oam_warning_log0(pst_vap->uc_vap_id, OAM_SF_ANY, "{hmac_vap_creat_netdev::pst_net_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 如下对netdevice的赋值暂时按如下操作 */
    hmac_vap_netdev_init(pst_net_device);
    l_ret = memcpy_s(oal_netdevice_mac_addr(pst_net_device), WLAN_MAC_ADDR_LEN,
                     puc_mac_addr, WLAN_MAC_ADDR_LEN);
    oal_net_dev_priv(pst_net_device) = pst_vap;
    oal_netdevice_qdisc(pst_net_device, OAL_PTR_NULL);

    ul_return = (oal_uint32)oal_net_register_netdev(pst_net_device);
    if (oal_unlikely(ul_return != OAL_SUCC)) {
        MAC_WARNING_LOG(pst_vap->uc_vap_id, "mac_device_init:oal_net_register_netdev return fail.");
        oam_warning_log0(pst_vap->uc_vap_id, OAM_SF_ANY, "{hmac_vap_creat_netdev::oal_net_register_netdev failed.}");
        oal_net_free_netdev(pst_net_device);
        return ul_return;
    }

    pst_hmac_vap->pst_net_device = pst_net_device;
    /* 包括'\0' */
    l_ret += memcpy_s(pst_hmac_vap->auc_name, OAL_IF_NAME_SIZE, pst_net_device->name, OAL_IF_NAME_SIZE);
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_ANY, "hmac_vap_creat_netdev::memcpy fail!");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_vap_destroy(hmac_vap_stru *pst_hmac_vap)
{
    mac_cfg_down_vap_param_stru st_down_vap;
    mac_cfg_del_vap_param_stru st_del_vap_param;
    oal_uint32 ul_ret;

    if (oal_unlikely(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_vap_destroy::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    if (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        pst_hmac_vap->uc_edca_opt_flag_ap = 0;
        frw_immediate_destroy_timer(&(pst_hmac_vap->st_edca_opt_timer));
    } else if (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        pst_hmac_vap->uc_edca_opt_flag_sta = 0;
    }
#endif

    /* 先down vap */
#ifdef _PRE_WLAN_FEATURE_P2P
    st_down_vap.en_p2p_mode = pst_hmac_vap->st_vap_base_info.en_p2p_mode;
#endif
    st_down_vap.pst_net_dev = pst_hmac_vap->pst_net_device;
    ul_ret = hmac_config_down_vap(&pst_hmac_vap->st_vap_base_info,
                                  OAL_SIZEOF(mac_cfg_down_vap_param_stru),
                                  (oal_uint8 *)&st_down_vap);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                         "{hmac_vap_destroy::hmac_config_down_vap failed[%d].}", ul_ret);
        return ul_ret;
    }

    /* 然后再delete vap */
    st_del_vap_param.en_p2p_mode = pst_hmac_vap->st_vap_base_info.en_p2p_mode;
    st_del_vap_param.en_vap_mode = pst_hmac_vap->st_vap_base_info.en_vap_mode;
    ul_ret = hmac_config_del_vap(&pst_hmac_vap->st_vap_base_info,
                                 OAL_SIZEOF(mac_cfg_del_vap_param_stru),
                                 (oal_uint8 *)&st_del_vap_param);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                         "{hmac_vap_destroy::hmac_config_del_vap failed[%d].}", ul_ret);
        return ul_ret;
    }
    return OAL_SUCC;
}


oal_uint16 hmac_vap_check_ht_capabilities_ap(
    hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_payload, oal_uint16 us_info_elem_offset,
    oal_uint32 ul_msg_len, hmac_user_stru *pst_hmac_user_sta)
{
    oal_bool_enum en_prev_asoc_ht = OAL_FALSE;
    oal_bool_enum en_prev_asoc_non_ht = OAL_FALSE;
    mac_user_ht_hdl_stru *pst_ht_hdl = &(pst_hmac_user_sta->st_user_base_info.st_ht_hdl);
    hmac_amsdu_stru *pst_amsdu = OAL_PTR_NULL;
    oal_uint32 ul_amsdu_idx;
    mac_protection_stru *pst_protection = OAL_PTR_NULL;
    oal_uint8 *puc_ie = OAL_PTR_NULL;

    if (mac_mib_get_HighThroughputOptionImplemented(&pst_hmac_vap->st_vap_base_info) == OAL_FALSE) {
        return MAC_SUCCESSFUL_STATUSCODE;
    }
    /* 检查STA是否是作为一个HT capable STA与AP关联 */
    if ((pst_hmac_user_sta->st_user_base_info.en_user_asoc_state == MAC_USER_STATE_ASSOC) &&
        (pst_ht_hdl->en_ht_capable == OAL_TRUE)) {
        mac_user_set_ht_capable(&(pst_hmac_user_sta->st_user_base_info), OAL_FALSE);
        en_prev_asoc_ht = OAL_TRUE;
    } else if (pst_hmac_user_sta->st_user_base_info.en_user_asoc_state == MAC_USER_STATE_ASSOC) {
        /* 检查STA是否是作为一个non HT capable STA与AP关联 */
        en_prev_asoc_non_ht = OAL_TRUE;
    } else {
    }

    if (us_info_elem_offset < ul_msg_len) {
        ul_msg_len -= us_info_elem_offset;
        puc_payload += us_info_elem_offset;

        puc_ie = mac_find_ie(MAC_EID_HT_CAP, puc_payload, (oal_int32)ul_msg_len);
        if (puc_ie != OAL_PTR_NULL) {
            /* 不允许HT STA 使用 TKIP/WEP 加密 */
            if (mac_is_wep_enabled(&pst_hmac_vap->st_vap_base_info)) {
                OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                                 "hmac_vap_check_ht_capabilities_ap:Rejecting a HT STA because of Pairwise Cipher[%d]",
                                 pst_hmac_user_sta->st_user_base_info.st_key_info.en_cipher_type);
                return MAC_MISMATCH_HTCAP;
            }

            /* 搜索 HT Capabilities Element */
            hmac_search_ht_cap_ie_ap(pst_hmac_vap, pst_hmac_user_sta, puc_ie, 0, en_prev_asoc_ht);
            /* uc_rx_mcs_bitmask的0、1、2、3字节为0，则表示STA支持ht能力但不支持空间流 */
            if ((pst_ht_hdl->uc_rx_mcs_bitmask[3] == 0) && (pst_ht_hdl->uc_rx_mcs_bitmask[2] == 0)
                && (pst_ht_hdl->uc_rx_mcs_bitmask[1] == 0) && ((pst_ht_hdl->uc_rx_mcs_bitmask[0]) == 0)) {
                oam_warning_log0(0, OAM_SF_ANY, "{hmac_vap_check_ht_capabilities_ap_etc:: \
                    STA support ht capability but support none space_stream.}");
                /* 对端ht能力置为不支持 */
                mac_user_set_ht_capable(&(pst_hmac_user_sta->st_user_base_info), OAL_FALSE);
            }
        }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
        puc_ie = mac_find_ie(MAC_EID_EXT_CAPS, puc_payload, (oal_int32)ul_msg_len);
        if (puc_ie != OAL_PTR_NULL) {
            mac_ie_proc_ext_cap_ie(&(pst_hmac_user_sta->st_user_base_info), puc_ie);
        }
#endif /* _PRE_WLAN_FEATURE_20_40_80_COEXIST */
    }

    /* 走到这里，说明sta已经被统计到ht相关的统计量中 */
    pst_hmac_user_sta->st_user_stats_flag.bit_no_ht_stats_flag = OAL_TRUE;
    pst_hmac_user_sta->st_user_stats_flag.bit_no_gf_stats_flag = OAL_TRUE;
    pst_hmac_user_sta->st_user_stats_flag.bit_20M_only_stats_flag = OAL_TRUE;
    pst_hmac_user_sta->st_user_stats_flag.bit_no_lsig_txop_stats_flag = OAL_TRUE;
    pst_hmac_user_sta->st_user_stats_flag.bit_no_40dsss_stats_flag = OAL_TRUE;

    pst_protection = &(pst_hmac_vap->st_vap_base_info.st_protection);
    if (pst_ht_hdl->en_ht_capable == OAL_FALSE) { /* STA不支持HT */
        /* 如果STA之前没有与AP关联 */
        if (pst_hmac_user_sta->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_ASSOC) {
            pst_protection->uc_sta_non_ht_num++;
        } else if (en_prev_asoc_ht == OAL_TRUE) { /* 如果STA之前已经作为HT站点与AP关联 */
            pst_protection->uc_sta_non_ht_num++;

            if ((pst_ht_hdl->bit_supported_channel_width == OAL_FALSE) && (pst_protection->uc_sta_20M_only_num != 0)) {
                pst_protection->uc_sta_20M_only_num--;
            }

            if ((pst_ht_hdl->bit_ht_green_field == OAL_FALSE) && (pst_protection->uc_sta_non_gf_num != 0)) {
                pst_protection->uc_sta_non_gf_num--;
            }

            if ((pst_ht_hdl->bit_lsig_txop_protection == OAL_FALSE) && (pst_protection->uc_sta_no_lsig_txop_num != 0)) {
                pst_protection->uc_sta_no_lsig_txop_num--;
            }
        } else { /* STA 之前已经作为非HT站点与AP关联 */
        }
    } else { /* STA支持HT */
        for (ul_amsdu_idx = 0; ul_amsdu_idx < WLAN_WME_MAX_TID_NUM; ul_amsdu_idx++) {
            pst_amsdu = &(pst_hmac_user_sta->ast_hmac_amsdu[ul_amsdu_idx]);
            hmac_amsdu_set_maxsize(pst_amsdu, pst_hmac_user_sta, 7936); /* 配置amsdu最大长度,7936是us_max_size的值 */
            hmac_amsdu_set_maxnum(pst_amsdu, WLAN_AMSDU_MAX_NUM);
            oal_spin_lock_init(&pst_amsdu->st_amsdu_lock);
        }

        /* 设置amsdu域 */
        pst_hmac_user_sta->uc_amsdu_supported = 255; /* 255表示支持所有tid */

        /* 如果STA之前已经以non-HT站点与AP关联, 则将uc_sta_non_ht_num减1 */
        if ((en_prev_asoc_non_ht == OAL_TRUE) && (pst_protection->uc_sta_non_ht_num != 0)) {
            pst_protection->uc_sta_non_ht_num--;
        }
    }

    return MAC_SUCCESSFUL_STATUSCODE;
}


oal_uint16 hmac_vap_check_vht_capabilities_ap(
    hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_payload, oal_uint16 us_info_elem_offset,
    oal_uint32 ul_msg_len, hmac_user_stru *pst_hmac_user_sta)
{
    oal_uint8 *puc_vht_cap_ie = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    oal_uint8 *puc_vendor_vht_ie = OAL_PTR_NULL;
    oal_uint32 ul_vendor_vht_ie_offset = MAC_WLAN_OUI_VENDOR_VHT_HEADER + MAC_IE_HDR_LEN;
    oal_uint8 *puc_ie_tmp = OAL_PTR_NULL;

    if ((pst_hmac_vap == OAL_PTR_NULL) || (puc_payload == OAL_PTR_NULL) || (pst_hmac_user_sta == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_ANY, "{hmac_vap_check_vht_capabilities_ap::param null,%x %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)puc_payload, (uintptr_t)pst_hmac_user_sta);
        return MAC_UNSPEC_FAIL;
    }
    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);

    puc_vht_cap_ie =
        mac_find_ie(MAC_EID_VHT_CAP, puc_payload + us_info_elem_offset, (oal_int32)(ul_msg_len - us_info_elem_offset));
    if (puc_vht_cap_ie != OAL_PTR_NULL) {
        hmac_proc_vht_cap_ie(pst_mac_vap, pst_hmac_user_sta, puc_vht_cap_ie);
    } else {
        /* 不允许不支持11ac STA关联11aconly 模式的AP */
        if (pst_mac_vap->en_protocol == WLAN_VHT_ONLY_MODE) {
            oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                             "{hmac_vap_check_vht_capabilities_ap:AP 11ac only, but STA not support 11ac}");
            return MAC_MISMATCH_VHTCAP;
        }
    }

    if (pst_hmac_user_sta->st_user_base_info.st_vht_hdl.en_vht_capable == OAL_FALSE) {
        puc_vendor_vht_ie = mac_find_vendor_ie(MAC_WLAN_OUI_BROADCOM_EPIGRAM, MAC_WLAN_OUI_VENDOR_VHT_TYPE,
                                               puc_payload + us_info_elem_offset,
                                               (oal_int32)(ul_msg_len - us_info_elem_offset));
        if ((puc_vendor_vht_ie != OAL_PTR_NULL) && (puc_vendor_vht_ie[1] >= MAC_WLAN_OUI_VENDOR_VHT_HEADER)) {
            /* 进入此函数代表user支持2G 11ac */
            puc_ie_tmp = mac_find_ie(MAC_EID_VHT_CAP, puc_vendor_vht_ie + ul_vendor_vht_ie_offset,
                                     (oal_int32)(puc_vendor_vht_ie[1] - MAC_WLAN_OUI_VENDOR_VHT_HEADER));
            if (puc_ie_tmp != OAL_PTR_NULL) {
                pst_hmac_user_sta->en_user_epigram_vht_capable = OAL_TRUE;
                hmac_proc_vht_cap_ie(pst_mac_vap, pst_hmac_user_sta, puc_ie_tmp);
            } else { /* 表示支持5G 20M mcs9 */
                pst_hmac_user_sta->en_user_epigram_novht_capable = OAL_TRUE;
            }
        }
    }

    return MAC_SUCCESSFUL_STATUSCODE;
}


oal_void hmac_search_txbf_cap_ie_ap(mac_user_ht_hdl_stru *pst_ht_hdl,
                                    oal_uint32 ul_info_elem)
{
    oal_uint32 ul_tmp_txbf_elem = ul_info_elem;

    pst_ht_hdl->bit_imbf_receive_cap = (ul_tmp_txbf_elem & BIT0);
    pst_ht_hdl->bit_receive_staggered_sounding_cap = ((ul_tmp_txbf_elem & BIT1) >> 1);
    /* bit_transmit_staggered_sounding_cap是第2bit */
    pst_ht_hdl->bit_transmit_staggered_sounding_cap = ((ul_tmp_txbf_elem & BIT2) >> 2);
    pst_ht_hdl->bit_receive_ndp_cap = ((ul_tmp_txbf_elem & BIT3) >> 3); /* bit_receive_ndp_cap是第3bit */
    pst_ht_hdl->bit_transmit_ndp_cap = ((ul_tmp_txbf_elem & BIT4) >> 4); /* bit_transmit_ndp_cap是第4bit */
    pst_ht_hdl->bit_imbf_cap = ((ul_tmp_txbf_elem & BIT5) >> 5); /* bit_imbf_cap是第5bit */
    pst_ht_hdl->bit_calibration = ((ul_tmp_txbf_elem & 0x000000C0) >> 6); /* bit_calibration是第6bit */
    pst_ht_hdl->bit_exp_csi_txbf_cap = ((ul_tmp_txbf_elem & BIT8) >> 8); /* bit_exp_csi_txbf_cap是第8bit */
    pst_ht_hdl->bit_exp_noncomp_txbf_cap = ((ul_tmp_txbf_elem & BIT9) >> 9); /* bit_exp_noncomp_txbf_cap是第9bit */
    pst_ht_hdl->bit_exp_comp_txbf_cap = ((ul_tmp_txbf_elem & BIT10) >> 10); /* bit_exp_comp_txbf_cap是第10bit */
    pst_ht_hdl->bit_exp_csi_feedback = ((ul_tmp_txbf_elem & 0x00001800) >> 11); /* bit_exp_csi_feedback是第11bit */
    /* bit_exp_noncomp_feedback是第13bit */
    pst_ht_hdl->bit_exp_noncomp_feedback = ((ul_tmp_txbf_elem & 0x00006000) >> 13);

    pst_ht_hdl->bit_exp_comp_feedback = ((ul_tmp_txbf_elem & 0x0001C000) >> 15); /* bit_exp_comp_feedback是第15bit */
    pst_ht_hdl->bit_min_grouping = ((ul_tmp_txbf_elem & 0x00060000) >> 17); /* bit_min_grouping是第17bit */
    pst_ht_hdl->bit_csi_bfer_ant_number = ((ul_tmp_txbf_elem & 0x001C0000) >> 19); /* bit_csi_bfer_ant_number是19bit */
    /* bit_noncomp_bfer_ant_number是第21bit */
    pst_ht_hdl->bit_noncomp_bfer_ant_number = ((ul_tmp_txbf_elem & 0x00600000) >> 21);
    /* bit_comp_bfer_ant_number是第23bit */
    pst_ht_hdl->bit_comp_bfer_ant_number = ((ul_tmp_txbf_elem & 0x01C00000) >> 23);
    pst_ht_hdl->bit_csi_bfee_max_rows = ((ul_tmp_txbf_elem & 0x06000000) >> 25); /* bit_csi_bfee_max_rows是第25bit */
    pst_ht_hdl->bit_channel_est_cap = ((ul_tmp_txbf_elem & 0x18000000) >> 27); /* bit_channel_est_cap是第27bit */
}

oal_void hmac_check_40m_intolerant(hmac_vap_stru *pst_hmac_vap, mac_user_stru *pst_mac_user,
    mac_device_stru *pst_mac_device, mac_user_ht_hdl_stru *pst_ht_hdl, oal_uint16 us_tmp_info_elem)
{
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);

    /* 检查Forty MHz Intolerant */
    pst_ht_hdl->bit_forty_mhz_intolerant = ((us_tmp_info_elem & BIT14) >> 14); /* bit_forty_mhz_intolerant是第14bit */

    hmac_chan_update_40m_intol_user(pst_mac_vap, pst_mac_user,
                                    (oal_bool_enum_uint8)pst_ht_hdl->bit_forty_mhz_intolerant);

    /* 如果允许2040带宽切换开关打开，并且Forty MHz Intolerant为真,则不允许AP在40MHz运行 */
    if ((pst_mac_vap->st_cap_flag.bit_2040_autoswitch == OAL_TRUE) &&
        (pst_ht_hdl->bit_forty_mhz_intolerant == OAL_TRUE)) {
        mac_mib_set_FortyMHzIntolerant(pst_mac_vap, OAL_TRUE);
        oam_warning_log3(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                         "{hmac_search_ht_cap_ie_ap::40Int==%u, SMI==%u, 2040switch==%u.}",
                         pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11FortyMHzIntolerant,
                         pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementImplemented,
                         pst_hmac_vap->en_2040_switch_prohibited);

        hmac_chan_start_40m_recovery_timer(pst_mac_vap);
        if (pst_mac_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_20M) {
            /* AP准备切换置20MHz运行 */
            hmac_chan_multi_switch_to_20MHz_ap(pst_hmac_vap);
        } else {
            pst_mac_device->en_40MHz_intol_bit_recd = OAL_TRUE;
        }
    } else {
        mac_mib_set_FortyMHzIntolerant(pst_mac_vap, OAL_FALSE);
    }
#endif
}
oal_void hmac_parse_ht_capabilities(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user_sta,
    oal_uint16 us_tmp_info_elem, oal_bool_enum en_prev_asoc_ht, mac_user_ht_hdl_stru *pst_ht_hdl)
{
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    mac_user_stru *pst_mac_user = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    oal_uint8 uc_smps;
    oal_uint8 uc_supported_channel_width;
    oal_uint8 uc_ht_green_field;
    oal_uint8 uc_lsig_txop_protection_support;

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    pst_mac_user = &(pst_hmac_user_sta->st_user_base_info);
    pst_mac_device = mac_res_get_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    /* 检查STA所支持的LDPC编码能力 B0，0:不支持，1:支持 */
    pst_ht_hdl->bit_ldpc_coding_cap = (us_tmp_info_elem & BIT0);

    /* 检查STA所支持的信道宽度 B1，0:仅20M运行，1:20M与40M运行 */
    uc_supported_channel_width = (us_tmp_info_elem & BIT1) >> 1;
    pst_ht_hdl->bit_supported_channel_width =
        mac_ie_proc_ht_supported_channel_width(pst_mac_user, pst_mac_vap, uc_supported_channel_width, en_prev_asoc_ht);

    /* 检查空间复用节能模式 B2~B3 */
    uc_smps = ((us_tmp_info_elem & (BIT2 | BIT3)) >> 2);
    pst_ht_hdl->bit_sm_power_save = mac_ie_proc_sm_power_save_field(pst_mac_user, uc_smps);

    /* 检查Greenfield 支持情况 B4， 0:不支持，1:支持 */
    uc_ht_green_field = (us_tmp_info_elem & BIT4) >> 4;
    pst_ht_hdl->bit_ht_green_field =
        mac_ie_proc_ht_green_field(pst_mac_user, pst_mac_vap, uc_ht_green_field, en_prev_asoc_ht);

    /* 检查20MHz Short-GI B5,  0:不支持，1:支持，之后与AP取交集 */
    pst_ht_hdl->bit_short_gi_20mhz = ((us_tmp_info_elem & BIT5) >> 5);
    pst_ht_hdl->bit_short_gi_20mhz &=
        pst_hmac_vap->st_vap_base_info.pst_mib_info->st_phy_ht.en_dot11ShortGIOptionInTwentyImplemented;

    /* 检查40MHz Short-GI B6,  0:不支持，1:支持，之后与AP取交集 */
    pst_ht_hdl->bit_short_gi_40mhz = ((us_tmp_info_elem & BIT6) >> 6);
    pst_ht_hdl->bit_short_gi_40mhz &= mac_mib_get_ShortGIOptionInFortyImplemented(&pst_hmac_vap->st_vap_base_info);

    /* 检查支持接收STBC PPDU B8,  0:不支持，1:支持 */
    pst_ht_hdl->bit_rx_stbc = ((us_tmp_info_elem & 0x0300) >> 8);

    /* 检查最大A-MSDU长度 B11，0:3839字节, 1:7935字节 */
    pst_hmac_user_sta->us_amsdu_maxsize =
        ((us_tmp_info_elem & BIT11) == 0) ? WLAN_MIB_MAX_AMSDU_LENGTH_SHORT : WLAN_MIB_MAX_AMSDU_LENGTH_LONG;

    /* 检查在40M上DSSS/CCK的支持情况 B12 */
    /* 在非Beacon帧或probe rsp帧中时 */
    /* 0: STA在40MHz上不使用DSSS/CCK, 1: STA在40MHz上使用DSSS/CCK */
    pst_ht_hdl->bit_dsss_cck_mode_40mhz = ((us_tmp_info_elem & BIT12) >> 12); /* bit_dsss_cck_mode_40mhz是第12bit */

    if ((pst_ht_hdl->bit_dsss_cck_mode_40mhz == 0) &&
        (pst_ht_hdl->bit_supported_channel_width == 1)) {
        pst_hmac_vap->st_vap_base_info.st_protection.uc_sta_no_40dsss_cck_num++;
    }
    if (pst_mac_device != OAL_PTR_NULL) {
        hmac_check_40m_intolerant(pst_hmac_vap, pst_mac_user, pst_mac_device, pst_ht_hdl, us_tmp_info_elem);
    }
    /* 检查对L-SIG TXOP 保护的支持情况 B15, 0:不支持，1:支持 */
    uc_lsig_txop_protection_support = (us_tmp_info_elem & BIT15) >> 15;
    pst_ht_hdl->bit_lsig_txop_protection =
        mac_ie_proc_lsig_txop_protection_support(pst_mac_user, pst_mac_vap,
                                                 uc_lsig_txop_protection_support, en_prev_asoc_ht);
}

oal_uint32 hmac_search_ht_cap_ie_ap(
    hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user_sta, oal_uint8 *puc_payload,
    oal_uint16 us_current_offset, oal_bool_enum en_prev_asoc_ht)
{
    oal_uint8 uc_mcs_bmp_index;
    oal_uint8 *puc_tmp_payload = OAL_PTR_NULL;
    oal_uint16 us_tmp_info_elem;
    oal_uint16 us_tmp_txbf_low;
    oal_uint32 ul_tmp_txbf_elem;

    mac_user_ht_hdl_stru *pst_ht_hdl = OAL_PTR_NULL;
    mac_user_ht_hdl_stru st_ht_hdl;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    mac_user_stru *pst_mac_user = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                       "{hmac_search_ht_cap_ie_ap::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 保存 入参 */
    puc_tmp_payload = puc_payload;
    if (puc_tmp_payload[1] < MAC_HT_CAP_LEN) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_search_ht_cap_ie_ap::invalid ht cap len[%d]}", puc_tmp_payload[1]);
        return OAL_FAIL;
    }

    pst_ht_hdl = &st_ht_hdl;
    mac_user_get_ht_hdl(&(pst_hmac_user_sta->st_user_base_info), pst_ht_hdl);
    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    pst_mac_user = &(pst_hmac_user_sta->st_user_base_info);

    /* 带有 HT Capability Element 的 STA，标示它具有HT capable. */
    pst_ht_hdl->en_ht_capable = 1;
    us_current_offset += MAC_IE_HDR_LEN;

    /***************************************************************************
                    解析 HT Capabilities Info Field
    ***************************************************************************/
    us_tmp_info_elem = oal_make_word16(puc_tmp_payload[us_current_offset], puc_tmp_payload[us_current_offset + 1]);
    hmac_parse_ht_capabilities(pst_hmac_vap, pst_hmac_user_sta, us_tmp_info_elem, en_prev_asoc_ht, pst_ht_hdl);
    us_current_offset += MAC_HT_CAPINFO_LEN;

    /***************************************************************************
                        解析 A-MPDU Parameters Field
    ***************************************************************************/
    /* 提取 Maximum Rx A-MPDU factor (B1 - B0) */
    pst_ht_hdl->uc_max_rx_ampdu_factor = (puc_tmp_payload[us_current_offset] & 0x03);

    /* 提取 the Minimum MPDU Start Spacing (B2 - B4) */
    pst_ht_hdl->uc_min_mpdu_start_spacing = (puc_tmp_payload[us_current_offset] >> 2) & 0x07;
    us_current_offset += MAC_HT_AMPDU_PARAMS_LEN;

    /***************************************************************************
                        解析 Supported MCS Set Field
    ***************************************************************************/
    for (uc_mcs_bmp_index = 0; uc_mcs_bmp_index < WLAN_HT_MCS_BITMASK_LEN; uc_mcs_bmp_index++) {
        pst_ht_hdl->uc_rx_mcs_bitmask[uc_mcs_bmp_index] =
            (*(oal_uint8 *)(puc_tmp_payload + us_current_offset + uc_mcs_bmp_index));
    }

    pst_ht_hdl->uc_rx_mcs_bitmask[WLAN_HT_MCS_BITMASK_LEN - 1] &= 0x1F;

    us_current_offset += MAC_HT_SUP_MCS_SET_LEN;

    /***************************************************************************
                        解析 HT Extended Capabilities Info Field
    ***************************************************************************/
    us_tmp_info_elem = oal_make_word16(puc_tmp_payload[us_current_offset], puc_tmp_payload[us_current_offset + 1]);
    /* 提取 HTC support Information */
    if ((us_tmp_info_elem & BIT10) != 0) {
        pst_ht_hdl->uc_htc_support = 1;
    }

    us_current_offset += MAC_HT_EXT_CAP_LEN;

    /***************************************************************************
                        解析 Tx Beamforming Field
    ***************************************************************************/
    us_tmp_info_elem = oal_make_word16(puc_tmp_payload[us_current_offset], puc_tmp_payload[us_current_offset + 1]);
    /* 第2byte和第3byte拼接为txbf字段低16bit */
    us_tmp_txbf_low = oal_make_word16(puc_tmp_payload[us_current_offset + 2], puc_tmp_payload[us_current_offset + 3]);
    ul_tmp_txbf_elem = oal_make_word32(us_tmp_info_elem, us_tmp_txbf_low);
    hmac_search_txbf_cap_ie_ap(pst_ht_hdl, ul_tmp_txbf_elem);

    mac_user_set_ht_hdl(&(pst_hmac_user_sta->st_user_base_info), pst_ht_hdl);

    return OAL_SUCC;
}


oal_bool_enum_uint8 hmac_vap_addba_check(hmac_vap_stru *pst_hmac_vap,
                                         hmac_user_stru *pst_hmac_user,
                                         oal_uint8 uc_tidno)
{
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    hmac_tid_stru *pst_hmac_tid_info = OAL_PTR_NULL;

    /* 判断HMAC VAP的是否支持聚合 */
    if (!((pst_hmac_vap->en_tx_aggr_on) || (pst_hmac_vap->st_vap_base_info.st_cap_flag.bit_rifs_tx_on))) {
        return OAL_FALSE;
    }

    pst_hmac_tid_info = &(pst_hmac_user->ast_tid_info[uc_tidno]);
    if (pst_hmac_tid_info->st_ba_tx_info.en_ba_switch != OAL_TRUE) {
        return OAL_FALSE;
    }

    if (pst_hmac_tid_info->st_ba_tx_info.en_ba_status == DMAC_BA_COMPLETE) {
        return OAL_FALSE;
    }

    pst_mac_device = mac_res_get_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA, "{hmac_vap_addba_check::pst_mac_dev null");
        return OAL_FALSE;
    }
#ifdef _PRE_WLAN_FEATURE_AMPDU_VAP
    if (pst_hmac_vap->uc_tx_ba_session_num >= WLAN_MAX_TX_BA) {
        return OAL_FALSE;
    }
#else
    if (pst_mac_device->uc_tx_ba_session_num >= WLAN_MAX_TX_BA) {
        return OAL_FALSE;
    }
#endif
    /* 需要先发送5个单播帧，再进行BA会话的建立 */
    if ((pst_hmac_user->st_user_base_info.st_cap_info.bit_qos == OAL_TRUE) &&
        (pst_hmac_user->auc_ba_flag[uc_tidno] < DMAC_UCAST_FRAME_TX_COMP_TIMES)) {
        oam_info_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                      "{hmac_vap_addba_check::auc_ba_flag[%d]}", pst_hmac_user->auc_ba_flag[uc_tidno]);
        pst_hmac_user->auc_ba_flag[uc_tidno]++;
        return OAL_FALSE;
    }

    if ((pst_hmac_tid_info->st_ba_tx_info.en_ba_status != DMAC_BA_INPROGRESS)
        && (pst_hmac_tid_info->st_ba_tx_info.uc_addba_attemps < HMAC_ADDBA_EXCHANGE_ATTEMPTS)) {
        pst_hmac_tid_info->st_ba_tx_info.en_ba_status = DMAC_BA_INPROGRESS;
        pst_hmac_tid_info->st_ba_tx_info.uc_addba_attemps++;

        return OAL_TRUE;
    }
    return OAL_FALSE;
}


oal_void hmac_vap_net_stopall(oal_void)
{
    oal_uint8 uc_vap_id;
    oal_net_device_stru *pst_net_device = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    for (uc_vap_id = 0; uc_vap_id < WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE; uc_vap_id++) {
        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
        if (pst_hmac_vap == NULL)
            break;

        pst_net_device = pst_hmac_vap->pst_net_device;

        if (pst_net_device == NULL)
            break;

        oal_net_tx_stop_all_queues(pst_net_device);
    }
}


oal_bool_enum_uint8 hmac_flowctl_check_device_is_sta_mode(oal_void)
{
    mac_device_stru *pst_dev = OAL_PTR_NULL;
    mac_vap_stru *pst_vap = OAL_PTR_NULL;
    oal_uint8 uc_vap_index;
    oal_bool_enum_uint8 en_device_is_sta = OAL_FALSE;

    pst_dev = mac_res_get_dev(0);
    if (oal_unlikely(pst_dev == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_p2p_check_can_enter_state::pst_mac_device[0] null!}");
        return OAL_FALSE;
    }

    if (pst_dev->uc_vap_num >= 1) {
        for (uc_vap_index = 0; uc_vap_index < pst_dev->uc_vap_num; uc_vap_index++) {
            pst_vap = mac_res_get_mac_vap(pst_dev->auc_vap_id[uc_vap_index]);
            if (pst_vap == OAL_PTR_NULL) {
                continue;
            }

            if (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
                en_device_is_sta = OAL_TRUE;
                break;
            }
        }
    }

    return en_device_is_sta;
}


OAL_STATIC oal_void hmac_vap_wake_subq(oal_uint8 uc_vap_id, oal_uint16 us_queue_idx)
{
    oal_net_device_stru *pst_net_device = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
    if (pst_hmac_vap == NULL) {
        return;
    }

    pst_net_device = pst_hmac_vap->pst_net_device;
    if (pst_net_device == NULL) {
        return;
    }

    oal_net_wake_subqueue(pst_net_device, us_queue_idx);
}


oal_void hmac_vap_net_start_subqueue(oal_uint16 us_queue_idx)
{
    oal_uint8 uc_vap_id;
    OAL_STATIC oal_uint8 g_uc_start_subq_flag = 0;

    /* 自旋锁内，任务和软中断都被锁住，不需要FRW锁 */
    if (g_uc_start_subq_flag == 0) {
        g_uc_start_subq_flag = 1;

        /* vap id从低到高恢复 */
        for (uc_vap_id = 1; uc_vap_id < WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT; uc_vap_id++) {
            hmac_vap_wake_subq(uc_vap_id, us_queue_idx);
        }
    } else {
        g_uc_start_subq_flag = 0;

        /* vap id从高到低恢复 */
        for (uc_vap_id = WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT; uc_vap_id > 1; uc_vap_id--) {
            hmac_vap_wake_subq(uc_vap_id - 1, us_queue_idx);
        }
    }
}


OAL_STATIC oal_void hmac_vap_stop_subq(oal_uint8 uc_vap_id, oal_uint16 us_queue_idx)
{
    oal_net_device_stru *pst_net_device = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
    if (pst_hmac_vap == NULL) {
        return;
    }

    pst_net_device = pst_hmac_vap->pst_net_device;
    if (pst_net_device == NULL) {
        return;
    }

    oal_net_stop_subqueue(pst_net_device, us_queue_idx);
}


oal_void hmac_vap_net_stop_subqueue(oal_uint16 us_queue_idx)
{
    oal_uint8 uc_vap_id;
    OAL_STATIC oal_uint8 g_uc_stop_subq_flag = 0;

    /* 自旋锁内，任务和软中断都被锁住，不需要FRW锁 */
    /* 由按照VAP ID顺序停止subq，改为不依据VAP ID顺序 */
    if (g_uc_stop_subq_flag == 0) {
        g_uc_stop_subq_flag = 1;

        for (uc_vap_id = 1; uc_vap_id < WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT; uc_vap_id++) {
            hmac_vap_stop_subq(uc_vap_id, us_queue_idx);
        }
    } else {
        g_uc_stop_subq_flag = 0;

        for (uc_vap_id = WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT; uc_vap_id > 1; uc_vap_id--) {
            hmac_vap_stop_subq(uc_vap_id - 1, us_queue_idx);
        }
    }
}


oal_void hmac_vap_net_startall(oal_void)
{
    oal_uint8 uc_vap_id;
    oal_net_device_stru *pst_net_device = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    for (uc_vap_id = 0; uc_vap_id < WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE; uc_vap_id++) {
        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
        if (pst_hmac_vap == NULL)
            break;

        pst_net_device = pst_hmac_vap->pst_net_device;

        if (pst_net_device == NULL)
            break;

        oal_net_tx_wake_all_queues(pst_net_device);
    }
}

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY

oal_uint32 hmac_check_opmode_notify(hmac_vap_stru *pst_hmac_vap,
                                    oal_uint8 *puc_mac_hdr,
                                    oal_uint8 *puc_payload,
                                    oal_uint16 us_info_elem_offset,
                                    oal_uint32 ul_msg_len,
                                    hmac_user_stru *pst_hmac_user)
{
    oal_uint8 *puc_opmode_notify_ie;
    mac_vap_stru *pst_mac_vap;
    mac_user_stru *pst_mac_user;
    mac_opmode_notify_stru *pst_opmode_notify = OAL_PTR_NULL;
    oal_uint8 uc_mgmt_frm_type;
    oal_uint32 ul_relt;

    if ((pst_hmac_vap == OAL_PTR_NULL) || (puc_payload == OAL_PTR_NULL) ||
        (pst_hmac_user == OAL_PTR_NULL) || (puc_mac_hdr == OAL_PTR_NULL)) {
        oam_error_log4(0, OAM_SF_ANY, "{hmac_check_opmode_notify::param null, %x %x %x %x.}", (uintptr_t)pst_hmac_vap,
                       (uintptr_t)puc_payload, (uintptr_t)pst_hmac_user, (uintptr_t)puc_mac_hdr);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    pst_mac_user = &(pst_hmac_user->st_user_base_info);

    if ((mac_mib_get_VHTOptionImplemented(pst_mac_vap) == OAL_FALSE)
        || (mac_mib_get_OperatingModeNotificationImplemented(pst_mac_vap) == OAL_FALSE)) {
        return OAL_SUCC;
    }

    puc_opmode_notify_ie = mac_find_ie(MAC_EID_OPMODE_NOTIFY, puc_payload + us_info_elem_offset,
                                       (oal_int32)(ul_msg_len - us_info_elem_offset));
    if ((puc_opmode_notify_ie != OAL_PTR_NULL) && (puc_opmode_notify_ie[1] >= MAC_OPMODE_NOTIFY_LEN)) {
        uc_mgmt_frm_type = mac_get_frame_sub_type(puc_mac_hdr);
        pst_opmode_notify = (mac_opmode_notify_stru *)(puc_opmode_notify_ie + MAC_IE_HDR_LEN);
        ul_relt = mac_ie_proc_opmode_field(pst_mac_vap, pst_mac_user, pst_opmode_notify);
        if (oal_unlikely(ul_relt != OAL_SUCC)) {
            OAM_WARNING_LOG1(pst_mac_user->uc_vap_id, OAM_SF_CFG, "{hmac_check_opmode_notify:mac_ie_proc_opmode_field \
                failed[%d].}", ul_relt);
            return ul_relt;
        }
        /* opmode息同步dmac */
        ul_relt = hmac_config_update_opmode_event(pst_mac_vap, pst_mac_user, uc_mgmt_frm_type);
        if (oal_unlikely(ul_relt != OAL_SUCC)) {
            OAM_WARNING_LOG1(pst_mac_user->uc_vap_id, OAM_SF_CFG, "{hmac_check_opmode_notify: \
                hmac_config_update_opmode_event failed[%d].}", ul_relt);
            return ul_relt;
        }
    }
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_P2P

oal_void hmac_del_virtual_inf_worker(oal_work_stru *pst_del_virtual_inf_work)
{
    oal_net_device_stru *pst_net_dev;
    hmac_vap_stru *pst_hmac_vap;
    oal_wireless_dev_stru *pst_wireless_dev;
    hmac_device_stru *pst_hmac_device;

    pst_hmac_vap = oal_container_of(pst_del_virtual_inf_work, hmac_vap_stru, st_del_virtual_inf_worker);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{hmac_del_virtual_inf_worker:: hmac_vap is null}");
        return;
    }

    pst_net_dev = pst_hmac_vap->pst_del_net_device;
    if (pst_net_dev == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                       "{hmac_del_virtual_inf_worker:: net_dev is null}");
        return;
    }

    pst_wireless_dev = oal_netdevice_wdev(pst_net_dev);

    /* 不存在rtnl_lock锁问题 */
    oal_net_unregister_netdev(pst_net_dev);

    /* 在释放net_device 后释放wireless_device 内存 */
    oal_mem_free_m(pst_wireless_dev, OAL_TRUE);

    pst_hmac_vap->pst_del_net_device = OAL_PTR_NULL;

    pst_hmac_device = hmac_res_get_mac_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                       "{hmac_del_virtual_inf_worker::pst_hmac_device is null !device_id[%d]}",
                       pst_hmac_vap->st_vap_base_info.uc_device_id);
        return;
    }

    hmac_clr_p2p_status(&pst_hmac_device->ul_p2p_intf_status, P2P_STATUS_IF_DELETING);

    OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                     "{hmac_del_virtual_inf_worker::end !pst_hmac_device->ul_p2p_intf_status[%x]}",
                     pst_hmac_device->ul_p2p_intf_status);
    oal_smp_mb();
    oal_wait_queue_wake_up_interrupt(&pst_hmac_device->st_netif_change_event);
}
#endif /* _PRE_WLAN_FEATURE_P2P */


oal_void hmac_handle_disconnect_rsp(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user,
                                    mac_reason_code_enum_uint16 en_disasoc_reason)
{
    /* 修改 state & 删除 user */
    switch (pst_hmac_vap->st_vap_base_info.en_vap_mode) {
        case WLAN_VAP_MODE_BSS_AP: {
            /* 抛事件上报内核，已经去关联某个STA */
            hmac_handle_disconnect_rsp_ap(pst_hmac_vap, pst_hmac_user);
        }
        break;

        case WLAN_VAP_MODE_BSS_STA: {
            /* 上报内核sta已经和某个ap去关联 */
            hmac_sta_handle_disassoc_rsp(pst_hmac_vap, en_disasoc_reason);
        }
        break;
        default:
            break;
    }
    return;
}


oal_uint8 *hmac_vap_get_pmksa(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_bssid)
{
    hmac_pmksa_cache_stru *pst_pmksa_cache = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_pmksa_entry = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_pmksa_entry_tmp = OAL_PTR_NULL;

    if ((pst_hmac_vap == OAL_PTR_NULL) || (puc_bssid == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_vap_get_pmksa param null}\r\n");
        return OAL_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_SAE
    if (pst_hmac_vap->en_auth_mode == WLAN_WITP_AUTH_SAE) {
        return OAL_PTR_NULL;
    }
#endif /* _PRE_WLAN_FEATURE_SAE */

    if (oal_dlist_is_empty(&(pst_hmac_vap->st_pmksa_list_head))) {
        return OAL_PTR_NULL;
    }

    oal_dlist_search_for_each_safe(pst_pmksa_entry, pst_pmksa_entry_tmp, &(pst_hmac_vap->st_pmksa_list_head))
    {
        pst_pmksa_cache = oal_dlist_get_entry(pst_pmksa_entry, hmac_pmksa_cache_stru, st_entry);
        if (oal_compare_mac_addr(puc_bssid, pst_pmksa_cache->auc_bssid) == 0) {
            oal_dlist_delete_entry(pst_pmksa_entry);
            oam_warning_log3(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                             "{hmac_vap_get_pmksa:: FIND Pmksa of [%02X:XX:XX:XX:%02X:%02X]}",
                             puc_bssid[0], puc_bssid[4], puc_bssid[5]); /* puc_bssid 0、4、5字节为参数输出打印 */
            break;
        }
        pst_pmksa_cache = OAL_PTR_NULL;
    }

    if (pst_pmksa_cache != OAL_PTR_NULL) {
        oal_dlist_add_head(&(pst_pmksa_cache->st_entry), &(pst_hmac_vap->st_pmksa_list_head));
        return pst_pmksa_cache->auc_pmkid;
    }
    return OAL_PTR_NULL;
}


oal_uint32 hmac_tx_get_mac_vap(oal_uint8 uc_vap_id, mac_vap_stru **pst_vap_stru)
{
    mac_vap_stru *pst_vap;

    /* 获取vap结构信息 */
    pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(uc_vap_id);
    if (oal_unlikely(pst_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_TX, "{hmac_tx_get_mac_vap::pst_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* VAP模式判断 */
    if (pst_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_TX, "hmac_tx_get_mac_vap::vap_mode error[%d]",
                         pst_vap->en_vap_mode);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /* VAP状态判断 */
    if ((pst_vap->en_vap_state != MAC_VAP_STATE_UP) && (pst_vap->en_vap_state != MAC_VAP_STATE_PAUSE)) {
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_TX, "hmac_tx_get_mac_vap::vap_state[%d] error",
                         pst_vap->en_vap_state);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    *pst_vap_stru = pst_vap;

    return OAL_SUCC;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
OAL_STATIC oal_int32 hmac_cfg_vap_if_open(oal_net_device_stru *pst_dev)
{
    pst_dev->flags |= OAL_IFF_RUNNING;

    return OAL_SUCC;
}

OAL_STATIC oal_int32 hmac_cfg_vap_if_close(oal_net_device_stru *pst_dev)
{
    pst_dev->flags &= ~OAL_IFF_RUNNING;

    return OAL_SUCC;
}

OAL_STATIC oal_net_dev_tx_enum hmac_cfg_vap_if_xmit(oal_netbuf_stru *pst_buf, oal_net_device_stru *pst_dev)
{
    if (pst_buf) {
        oal_netbuf_free(pst_buf);
    }
    return OAL_NETDEV_TX_OK;
}
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM

oal_uint32 hmac_vap_check_signal_bridge(mac_vap_stru *pst_mac_vap)
{
    mac_device_stru *pst_mac_device;
    mac_vap_stru *pst_other_vap;
    oal_uint8 uc_vap_idx;

    pst_mac_device = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* check all vap state in case other vap is signal bridge GO mode */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_other_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_other_vap == OAL_PTR_NULL) {
            continue;
        }

#ifdef _PRE_WLAN_FEATURE_P2P
        /* 终端需求: 打开信号桥，禁止漫游 */
        if (pst_other_vap->en_p2p_mode == WLAN_P2P_GO_MODE) {
            /* 如果是P2P GO模式且Beacon帧不包含P2P ie即为信号桥 */
            if (mac_find_vendor_ie(MAC_WLAN_OUI_WFA, MAC_WLAN_OUI_TYPE_WFA_P2P,
                                   pst_other_vap->ast_app_ie[OAL_APP_BEACON_IE].puc_ie,
                                   (oal_int32)pst_other_vap->ast_app_ie[OAL_APP_BEACON_IE].ul_ie_len) == OAL_PTR_NULL) {
                return OAL_FAIL;
            }
        }
#endif
    }

    return OAL_SUCC;
}
#endif

oal_void hmac_vap_save_rx_rate(hmac_vap_stru *pst_hmac_vap)
{
    oal_uint32 ul_index;
    oal_uint32 ul_rx_rate;
    if (pst_hmac_vap->st_vap_base_info.en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return;
    }
    ul_rx_rate = (oal_uint32)pst_hmac_vap->station_info.rxrate.legacy;
    ul_index = pst_hmac_vap->st_history_rx_rate.ul_index % HMAC_STA_RSSI_LOG_NUM;
    pst_hmac_vap->st_history_rx_rate.aul_rate[ul_index] = ul_rx_rate * HAL_RATE_100KBPS_TO_KBPS; /* 换算成KBPS */
    pst_hmac_vap->st_history_rx_rate.ul_index++;
}

oal_void hmac_vap_clean_rx_rate(hmac_vap_stru *pst_hmac_vap)
{
    memset_s(&pst_hmac_vap->st_history_rx_rate, OAL_SIZEOF(hmac_sta_rx_rate_stru),
             0, OAL_SIZEOF(hmac_sta_rx_rate_stru));
}
oal_module_symbol(hmac_vap_get_priv_cfg);
oal_module_symbol(hmac_vap_get_net_device);
oal_module_symbol(hmac_vap_free_asoc_req_ie_ptr);
oal_module_symbol(hmac_vap_get_desired_country);
oal_module_symbol(hmac_vap_destroy);

#ifdef _PRE_WLAN_FEATURE_11D
oal_module_symbol(hmac_vap_get_updata_rd_by_ie_switch);
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
oal_module_symbol(hmac_del_virtual_inf_worker);
#endif /* _PRE_WLAN_FEATURE_P2P */
