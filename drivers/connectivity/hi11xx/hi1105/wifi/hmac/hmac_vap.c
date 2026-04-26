

#include "hmac_vap.h"
#include "oal_net.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"

#include "wlan_spec.h"

#include "mac_ie.h"

#include "frw_event_main.h"

#include "hmac_resource.h"
#include "hmac_tx_amsdu.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_fsm.h"
#include "hmac_ext_if.h"
#include "hmac_chan_mgmt.h"
#include "hmac_sme_sta.h"
#include "hmac_blockack.h"
#include "hmac_p2p.h"
#ifdef _PRE_WLAN_TCP_OPT
#include "hmac_tcp_opt.h"
#endif
#include "hmac_roam_main.h"
#include "hmac_mgmt_sta_up.h"
#include "hmac_mgmt_sta.h"
#include "hmac_mgmt_ap.h"
#include "hmac_blacklist.h"

#include "securec.h"
#ifdef _PRE_WLAN_FEATURE_11AX
#include "hmac_wifi6_self_cure.h"
#endif
#include "hmac_sae.h"
#include "plat_pm_wlan.h"
#ifdef _PRE_WLAN_FEATURE_FTM
#include "hmac_ftm.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_VAP_C

uint8_t g_uc_host_rx_ampdu_amsdu = OAL_FALSE;

#define HMAC_NETDEVICE_WDT_TIMEOUT (5 * OAL_TIME_HZ)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
OAL_STATIC int32_t hmac_cfg_vap_if_open(oal_net_device_stru *pst_dev);
OAL_STATIC int32_t hmac_cfg_vap_if_close(oal_net_device_stru *pst_dev);
OAL_STATIC oal_net_dev_tx_enum hmac_cfg_vap_if_xmit(oal_netbuf_stru *pst_buf, oal_net_device_stru *pst_dev);
#endif

OAL_STATIC oal_net_device_ops_stru g_st_vap_net_dev_cfg_vap_ops = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    .ndo_open = hmac_cfg_vap_if_open,
    .ndo_stop = hmac_cfg_vap_if_close,
    .ndo_start_xmit = hmac_cfg_vap_if_xmit,
#else
    .ndo_get_stats = oal_net_device_get_stats,
    .ndo_open = oal_net_device_open,
    .ndo_stop = oal_net_device_close,
    .ndo_start_xmit = NULL,
    .ndo_set_multicast_list = NULL,
    .ndo_do_ioctl = oal_net_device_ioctl,
    .ndo_change_mtu = oal_net_device_change_mtu,
    .ndo_init = oal_net_device_init,
    .ndo_set_mac_address = oal_net_device_set_macaddr
#endif
};

extern void hmac_del_virtual_inf_worker(oal_work_stru *del_virtual_inf_work);

#ifdef _PRE_WLAN_FEATURE_FTM

void hmac_vap_init_gas(hmac_vap_stru *hmac_vap)
{
    hmac_vap->st_gas_mgmt.uc_is_gas_request_sent = OAL_FALSE;
    hmac_vap->st_gas_mgmt.uc_gas_dialog_token = HMAC_GAS_DIALOG_TOKEN_INITIAL_VALUE;
    hmac_vap->st_gas_mgmt.uc_gas_response_dialog_token = HMAC_GAS_DIALOG_TOKEN_INITIAL_VALUE;
}
#endif


OAL_STATIC void hmac_vap_sta_init(hmac_vap_stru *hmac_vap,
    uint8_t chip_id, uint8_t device_id, uint8_t vap_id, mac_cfg_add_vap_param_stru *param)
{
    mac_blacklist_get_pointer(param->en_vap_mode, chip_id, device_id, vap_id, &(hmac_vap->pst_blacklist_info));
    if (hmac_vap->pst_blacklist_info != NULL) {
        memset_s(hmac_vap->pst_blacklist_info->ast_black_list,
                 sizeof(mac_blacklist_stru) * WLAN_BLACKLIST_MAX, 0,
                 sizeof(mac_blacklist_stru) * WLAN_BLACKLIST_MAX);
        memset_s(&(hmac_vap->pst_blacklist_info->st_autoblacklist_info),
                 sizeof(mac_autoblacklist_info_stru), 0, sizeof(mac_autoblacklist_info_stru));
        hmac_vap->pst_blacklist_info->uc_mode = 0;
        hmac_vap->pst_blacklist_info->uc_list_num = 0;
    }

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    hmac_vap->uc_edca_opt_flag_sta = 0;
    hmac_vap->uc_edca_opt_weight_sta = WLAN_EDCA_OPT_WEIGHT_STA;
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
    mac_mib_set_SpectrumManagementImplemented(&hmac_vap->st_vap_base_info, OAL_TRUE);
#endif
    hmac_vap->bit_sta_protocol_cfg = OAL_SWITCH_OFF;
    hmac_vap->uc_cfg_sta_pm_manual = 0xFF;
    hmac_vap->uc_ps_mode = g_wlan_ps_mode;
    hmac_vap->ipaddr_obtained = OAL_FALSE;

    /* STA模式默认业务识别功能开启 */
    mac_mib_set_TxTrafficClassifyFlag(&hmac_vap->st_vap_base_info, OAL_SWITCH_ON);
#ifdef _PRE_WLAN_FEATURE_11AX
    hmac_wifi6_self_cure_init(hmac_vap);
#endif
    hmac_vap->tcp_ack_buf_use_ctl_switch = OAL_SWITCH_OFF; /* 默认关闭由应用层控制的TCP ack缓存门限 */
}

OAL_STATIC void hmac_vap_init_set_rx_reorder_timeout(hmac_vap_stru *hmac_vap)
{
    hmac_vap->us_rx_timeout[WLAN_WME_AC_BK] = HMAC_BA_RX_BK_TIMEOUT;
    hmac_vap->us_rx_timeout[WLAN_WME_AC_BE] = HMAC_BA_RX_BE_TIMEOUT;
    hmac_vap->us_rx_timeout[WLAN_WME_AC_VI] = HMAC_BA_RX_VI_TIMEOUT;
    hmac_vap->us_rx_timeout[WLAN_WME_AC_VO] = HMAC_BA_RX_VO_TIMEOUT;

    hmac_vap->us_rx_timeout_min[WLAN_WME_AC_BE] = HMAC_BA_RX_BE_TIMEOUT_MIN;
    hmac_vap->us_rx_timeout_min[WLAN_WME_AC_BK] = HMAC_BA_RX_BK_TIMEOUT_MIN;
    hmac_vap->us_rx_timeout_min[WLAN_WME_AC_VI] = HMAC_BA_RX_VI_TIMEOUT_MIN;
    hmac_vap->us_rx_timeout_min[WLAN_WME_AC_VO] = HMAC_BA_RX_VO_TIMEOUT_MIN;
}
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP

void hmac_vap_init_hid2d_seqnum(hmac_vap_stru *hmac_vap)
{
    uint8_t idx;
    for (idx = 0; idx < MAC_HID2D_SEQNUM_CNT; idx++) {
        hmac_vap->aus_last_seqnum[idx] = MAC_HID2D_INVALID_SEQNUM;
    }
}
#endif
OAL_STATIC void hmac_vap_11kvr_cap_init(hmac_vap_stru *hmac_vap)
{
    hmac_vap->bit_11k_enable = g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11k;
    hmac_vap->bit_11v_enable = g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11v;
    hmac_vap->bit_bcn_table_switch = g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11k;
    hmac_vap->bit_11r_enable = g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11r;
    hmac_vap->bit_11r_over_ds = g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11r_ds;
    hmac_vap->bit_adaptive11r = g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_adaptive11r;
    hmac_vap->bit_nb_rpt_11k = g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_nb_rpt_11k;
    hmac_vap->bit_11r_private_preauth = hmac_vap->bit_adaptive11r;
}

uint32_t hmac_vap_init(hmac_vap_stru *hmac_vap, uint8_t chip_id,
    uint8_t device_id, uint8_t vap_id, mac_cfg_add_vap_param_stru *param)
{
    uint32_t ret;
    if (oal_unlikely(hmac_vap == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = mac_vap_init(&hmac_vap->st_vap_base_info, chip_id, device_id, vap_id, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }

    /* 统计信息清零 */
    oam_stats_clear_vap_stat_info(vap_id);

    /* 初始化预设参数 */
    hmac_vap->st_preset_para.en_protocol = hmac_vap->st_vap_base_info.en_protocol;
    hmac_vap->st_preset_para.en_bandwidth = hmac_vap->st_vap_base_info.st_channel.en_bandwidth;
    hmac_vap->st_preset_para.en_band = hmac_vap->st_vap_base_info.st_channel.en_band;

    /* 初始化配置私有结构体 */
    // 对于P2P CL 不能再初始化队列
    hmac_vap->st_cfg_priv.dog_tag = OAL_DOG_TAG;
    oal_wait_queue_init_head(&(hmac_vap->st_cfg_priv.st_wait_queue_for_sdt_reg));
    oal_wait_queue_init_head(&(hmac_vap->st_mgmt_tx.st_wait_queue));

#ifdef _PRE_WLAN_FEATURE_11D
    hmac_vap->en_updata_rd_by_ie_switch = OAL_FALSE;
#endif

    // 1103默认支持接收方向的AMPDU+AMSDU联合聚合,通过定制化决定
    hmac_vap->bit_rx_ampduplusamsdu_active = g_uc_host_rx_ampdu_amsdu;
    hmac_vap->en_ps_rx_amsdu = g_uc_host_rx_ampdu_amsdu;
    hmac_vap->hal_dev_id = 0xff; /* 0xff invalid */
    hmac_vap->d2h_amsdu_switch = OAL_TRUE; /* 默认支持amsdu */
    oal_wait_queue_init_head(&hmac_vap->query_wait_q); /* 根据配置VAP，将对应函数挂接在业务VAP，区分AP、STA和WDS模式 */
    switch (param->en_vap_mode) {
        case WLAN_VAP_MODE_BSS_AP:
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
            hmac_vap->edca_opt_time_ms = HMAC_EDCA_OPT_TIME_MS;
            frw_timer_create_timer_m(&(hmac_vap->st_edca_opt_timer), hmac_edca_opt_timeout_fn,
                                     hmac_vap->edca_opt_time_ms, hmac_vap, OAL_TRUE,
                                     OAM_MODULE_ID_HMAC, hmac_vap->st_vap_base_info.core_id);
            /* also open for 1102 at 2015-10-16 */
            hmac_vap->uc_edca_opt_flag_ap = 1;
            frw_timer_restart_timer_m(&(hmac_vap->st_edca_opt_timer), hmac_vap->edca_opt_time_ms, OAL_TRUE);
            hmac_vap->uc_idle_cycle_num = 0;
#endif

            /* AP模式默认业务识别功能开启 */
            mac_mib_set_TxTrafficClassifyFlag(&hmac_vap->st_vap_base_info, OAL_SWITCH_ON);
            mac_blacklist_get_pointer(param->en_vap_mode, chip_id, device_id,
                                      vap_id, &(hmac_vap->pst_blacklist_info));
            if (hmac_vap->pst_blacklist_info != NULL) {
                memset_s(hmac_vap->pst_blacklist_info->ast_black_list,
                         sizeof(mac_blacklist_stru) * WLAN_BLACKLIST_MAX, 0,
                         sizeof(mac_blacklist_stru) * WLAN_BLACKLIST_MAX);
                memset_s(&(hmac_vap->pst_blacklist_info->st_autoblacklist_info),
                         sizeof(mac_autoblacklist_info_stru), 0, sizeof(mac_autoblacklist_info_stru));
                hmac_vap->pst_blacklist_info->uc_mode = 0;
                hmac_vap->pst_blacklist_info->uc_list_num = 0;
            }
            break;

        case WLAN_VAP_MODE_BSS_STA:
            hmac_vap_sta_init(hmac_vap, chip_id, device_id, vap_id, param);
            break;

        case WLAN_VAP_MODE_WDS:
            break;

        case WLAN_VAP_MODE_CONFIG:
            /* 配置VAP直接返回 */
            return OAL_SUCC;

        default:
            return OAL_ERR_CODE_INVALID_CONFIG;
    }

    oal_spin_lock_init(&hmac_vap->st_lock_state);
    oal_dlist_init_head(&(hmac_vap->st_pmksa_list_head));

    /* 创建vap时 初始状态为init */
    mac_vap_state_change(&(hmac_vap->st_vap_base_info), MAC_VAP_STATE_INIT);

    /* 初始化重排序超时时间 */
    hmac_vap_init_set_rx_reorder_timeout(hmac_vap);

    /* 初始化删除虚拟网络接口工作队列 */
    oal_init_work(&(hmac_vap->st_del_virtual_inf_worker), hmac_del_virtual_inf_worker);
    hmac_vap->pst_del_net_device = NULL;
    hmac_vap->pst_p2p0_net_device = NULL;
#ifdef _PRE_WLAN_FEATURE_SAE
    oal_init_work(&(hmac_vap->st_sae_report_ext_auth_worker), hmac_report_ext_auth_worker);
    hmac_vap->duplicate_auth_seq2_flag = OAL_FALSE;
    hmac_vap->duplicate_auth_seq4_flag = OAL_FALSE;
#endif

#ifdef _PRE_WLAN_TCP_OPT
    hmac_vap->st_hmac_tcp_ack[HCC_TX].filter_info.ack_limit = DEFAULT_TX_TCP_ACK_THRESHOLD;
    hmac_vap->st_hmac_tcp_ack[HCC_RX].filter_info.ack_limit = DEFAULT_RX_TCP_ACK_THRESHOLD;
    hmac_tcp_opt_init_filter_tcp_ack_pool(hmac_vap);
#endif
    /* 双wlan模式下，只wlan0支持11k和11v */
    if (mac_is_primary_legacy_vap(&hmac_vap->st_vap_base_info) &&
        param->en_p2p_mode == WLAN_LEGACY_VAP_MODE && IS_STA(&(hmac_vap->st_vap_base_info))) {
        hmac_vap_11kvr_cap_init(hmac_vap);
    }

    mac_mib_set_dot11RadioMeasurementActivated(&(hmac_vap->st_vap_base_info), hmac_vap->bit_11k_enable);
    mac_mib_set_dot11RMBeaconTableMeasurementActivated(&(hmac_vap->st_vap_base_info), hmac_vap->bit_bcn_table_switch);

#ifdef _PRE_WLAN_FEATURE_FTM
    if (g_wlan_spec_cfg->feature_ftm_is_open) {
        wlan_chip_ftm_vap_init(hmac_vap);
        hmac_vap_init_gas(hmac_vap);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    mac_mib_set_MgmtOptionBSSTransitionActivated(&(hmac_vap->st_vap_base_info), hmac_vap->bit_11v_enable);
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
    hmac_vap_init_hid2d_seqnum(hmac_vap);
#endif
    return OAL_SUCC;
}


uint32_t hmac_vap_get_priv_cfg(mac_vap_stru *mac_vap, hmac_vap_cfg_priv_stru **cfg_priv)
{
    hmac_vap_stru *hmac_vap = NULL;

    if (oal_unlikely(mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_vap_get_priv_cfg::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log1(0, OAM_SF_ANY,
                       "{hmac_vap_get_priv_cfg::mac_res_get_hmac_vap fail.vap_id = %u}", mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    *cfg_priv = &hmac_vap->st_cfg_priv;

    return OAL_SUCC;
}


int8_t *hmac_vap_get_desired_country(uint8_t vap_id)
{
    hmac_vap_stru *hmac_vap;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(vap_id, OAM_SF_ANY, "{hmac_vap_get_desired_country::hmac_vap null.}");
        return NULL;
    }

    return hmac_vap->ac_desired_country;
}
#ifdef _PRE_WLAN_FEATURE_11D

uint32_t hmac_vap_get_updata_rd_by_ie_switch(uint8_t vap_id, oal_bool_enum_uint8 *updata_rd_by_ie_switch)
{
    hmac_vap_stru *hmac_vap;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(0);
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_vap_get_updata_rd_by_ie_switch::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *updata_rd_by_ie_switch = hmac_vap->en_updata_rd_by_ie_switch;
    return OAL_SUCC;
}
#endif

oal_net_device_stru *hmac_vap_get_net_device(uint8_t vap_id)
{
    hmac_vap_stru *hmac_vap;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(vap_id, OAM_SF_ANY, "{hmac_vap_get_net_device::hmac_vap null.}");
        return NULL;
    }

    return (hmac_vap->pst_net_device);
}


OAL_STATIC uint32_t hmac_vap_creat_netdev_set_net_device(oal_net_device_stru *net_device,
                                                         int8_t *mac_addr, mac_vap_stru *vap)
{
    int32_t ret;
    /* 如下对netdevice的赋值暂时按如下操作 */
    oal_netdevice_ops(net_device) = &g_st_vap_net_dev_cfg_vap_ops;
    oal_netdevice_destructor(net_device) = oal_net_free_netdev;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    /* need to check the net device diff */
#else
    oal_netdevice_master(net_device) = NULL;
#endif

    oal_netdevice_ifalias(net_device) = NULL;
    oal_netdevice_watchdog_timeo(net_device) = HMAC_NETDEVICE_WDT_TIMEOUT;
    ret = memcpy_s(oal_netdevice_mac_addr(net_device), WLAN_MAC_ADDR_LEN, mac_addr, WLAN_MAC_ADDR_LEN);
    oal_net_dev_priv(net_device) = vap;
    oal_netdevice_qdisc(net_device, NULL);

    return ret;
}


uint32_t hmac_vap_creat_netdev(hmac_vap_stru *hmac_vap, int8_t *netdev_name,
                               uint8_t netdev_name_len, int8_t *mac_addr)
{
    oal_net_device_stru *net_device = NULL;
#ifdef _PRE_CONFIG_HISI_110X_DISABLE_HISILICON0

#else
    uint32_t result;
#endif
    mac_vap_stru *vap = NULL;
    int32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(hmac_vap, netdev_name))) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_vap_creat_netdev::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (netdev_name_len > MAC_NET_DEVICE_NAME_LENGTH) {
        return OAL_FAIL;
    }

    vap = &hmac_vap->st_vap_base_info;
    net_device = oal_net_alloc_netdev(0, netdev_name, oal_ether_setup);
    if (oal_unlikely(net_device == NULL)) {
        oam_warning_log0(vap->uc_vap_id, OAM_SF_ANY, "{hmac_vap_creat_netdev::net_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_vap_creat_netdev_set_net_device(net_device, mac_addr, vap);

#ifdef _PRE_CONFIG_HISI_110X_DISABLE_HISILICON0

#else
    result = (uint32_t)hmac_net_register_netdev(net_device);
    if (oal_unlikely(result != OAL_SUCC)) {
        oam_warning_log0(vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_vap_creat_netdev::oal_net_register_netdev failed.}");
        oal_net_free_netdev(net_device);
        return result;
    }
#endif

    hmac_vap->pst_net_device = net_device;
    /* 包括'\0' */
    ret += memcpy_s(hmac_vap->auc_name, OAL_IF_NAME_SIZE, net_device->name, OAL_IF_NAME_SIZE);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_vap_creat_netdev::memcpy fail!");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


uint32_t hmac_vap_destroy(hmac_vap_stru *hmac_vap)
{
    mac_cfg_down_vap_param_stru down_vap;
    mac_cfg_del_vap_param_stru del_vap_param;
    uint32_t ret;

    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_vap_destroy::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_vap->uc_edca_opt_flag_ap = 0;
        frw_timer_immediate_destroy_timer_m(&(hmac_vap->st_edca_opt_timer));
    } else if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_vap->uc_edca_opt_flag_sta = 0;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_TWT
    if (g_wlan_spec_cfg->feature_twt_is_open) {
        memset_s(&(hmac_vap->st_twt_cfg), sizeof(mac_cfg_twt_stru), 0, sizeof(mac_cfg_twt_stru));
    }
#endif

    /* 先down vap */
    oal_cancel_work_sync(&(hmac_vap->st_del_virtual_inf_worker));
    down_vap.en_p2p_mode = hmac_vap->st_vap_base_info.en_p2p_mode;

    down_vap.pst_net_dev = hmac_vap->pst_net_device;
    ret = hmac_config_down_vap(&hmac_vap->st_vap_base_info,
                               sizeof(mac_cfg_down_vap_param_stru), (uint8_t *)&down_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                         "{hmac_vap_destroy::hmac_config_down_vap failed[%d].}", ret);
        return ret;
    }
    mac_blacklist_free_pointer(&hmac_vap->st_vap_base_info, hmac_vap->pst_blacklist_info);
    /* 然后再delete vap */
    del_vap_param.en_p2p_mode = hmac_vap->st_vap_base_info.en_p2p_mode;
    del_vap_param.en_vap_mode = hmac_vap->st_vap_base_info.en_vap_mode;
    ret = hmac_config_del_vap(&hmac_vap->st_vap_base_info, sizeof(mac_cfg_del_vap_param_stru),
                              (uint8_t *)&del_vap_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                         "{hmac_vap_destroy::hmac_config_del_vap failed[%d].}", ret);
        return ret;
    }
    return OAL_SUCC;
}


void hmac_vap_update_sta_nums_of_protection_param(hmac_user_stru *hmac_user_sta,
    mac_protection_stru *protection, mac_user_ht_hdl_stru *ht_hdl, oal_bool_enum prev_asoc_non_ht,
    oal_bool_enum prev_asoc_ht)
{
    if (ht_hdl->en_ht_capable == OAL_FALSE) { /* STA不支持HT */
        /* 如果STA之前没有与AP关联 */
        if (hmac_user_sta->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_ASSOC) {
            protection->uc_sta_non_ht_num++;
        } else if (prev_asoc_ht == OAL_TRUE) { /* 如果STA之前已经作为HT站点与AP关联 */
            protection->uc_sta_non_ht_num++;
            if ((ht_hdl->bit_supported_channel_width == OAL_FALSE) && (protection->uc_sta_20M_only_num != 0)) {
                protection->uc_sta_20M_only_num--;
            }
            if ((ht_hdl->bit_ht_green_field == OAL_FALSE) && (protection->uc_sta_non_gf_num != 0)) {
                protection->uc_sta_non_gf_num--;
            }
            if ((ht_hdl->bit_lsig_txop_protection == OAL_FALSE) && (protection->uc_sta_no_lsig_txop_num != 0)) {
                protection->uc_sta_no_lsig_txop_num--;
            }
        }
    } else { /* STA支持HT */
        hmac_amsdu_init_user(hmac_user_sta);
        /*  如果STA之前已经以non-HT站点与AP关联, 则将uc_sta_non_ht_num减1 */
        if ((prev_asoc_non_ht == OAL_TRUE) && (protection->uc_sta_non_ht_num != 0)) {
            protection->uc_sta_non_ht_num--;
        }
    }
}


uint16_t hmac_vap_check_ht_capabilities_ap(hmac_vap_stru *hmac_vap,
                                           uint8_t *payload,
                                           uint32_t msg_len,
                                           hmac_user_stru *hmac_user_sta)
{
    oal_bool_enum en_prev_asoc_ht = OAL_FALSE;
    oal_bool_enum en_prev_asoc_non_ht = OAL_FALSE;
    mac_user_ht_hdl_stru *ht_hdl = &(hmac_user_sta->st_user_base_info.st_ht_hdl);
    mac_protection_stru *protection = NULL;
    uint8_t *ie = NULL;

    if (mac_mib_get_HighThroughputOptionImplemented(&hmac_vap->st_vap_base_info) == OAL_FALSE) {
        return MAC_SUCCESSFUL_STATUSCODE;
    }
    /* 检查STA是否是作为一个HT capable STA与AP关联 */
    if ((hmac_user_sta->st_user_base_info.en_user_asoc_state == MAC_USER_STATE_ASSOC) &&
        (ht_hdl->en_ht_capable == OAL_TRUE)) {
        mac_user_set_ht_capable(&(hmac_user_sta->st_user_base_info), OAL_FALSE);
        en_prev_asoc_ht = OAL_TRUE;
    /* 检查STA是否是作为一个non HT capable STA与AP关联 */
    } else if (hmac_user_sta->st_user_base_info.en_user_asoc_state == MAC_USER_STATE_ASSOC) {
        en_prev_asoc_non_ht = OAL_TRUE;
    }

    ie = mac_find_ie(MAC_EID_HT_CAP, payload, (int32_t)msg_len);
    if (ie != NULL) {
        /* 不允许HT STA 使用 TKIP/WEP 加密 */
        if (mac_is_wep_enabled(&hmac_vap->st_vap_base_info)) {
            oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                "{hmac_vap_check_ht_capabilities_ap::Rejecting a HT STA because of its Pairwise Cipher[%d].}",
                hmac_user_sta->st_user_base_info.st_key_info.en_cipher_type);
            return MAC_MISMATCH_HTCAP;
        }

        /* 搜索 HT Capabilities Element */
        hmac_search_ht_cap_ie_ap(hmac_vap, hmac_user_sta, ie, 0, en_prev_asoc_ht);

        if ((ht_hdl->uc_rx_mcs_bitmask[BYTE_OFFSET_3] == 0) && (ht_hdl->uc_rx_mcs_bitmask[BYTE_OFFSET_2] == 0) &&
            (ht_hdl->uc_rx_mcs_bitmask[BYTE_OFFSET_1] == 0) && (ht_hdl->uc_rx_mcs_bitmask[BYTE_OFFSET_0]) == 0) {
            oam_warning_log0(0, OAM_SF_ANY,
                "{hmac_vap_check_ht_capabilities_ap::STA support ht capability but support none space_stream.}");
            /* 对端ht能力置为不支持 */
            mac_user_set_ht_capable(&(hmac_user_sta->st_user_base_info), OAL_FALSE);
        }
    }

    ie = mac_find_ie(MAC_EID_EXT_CAPS, payload, (int32_t)msg_len);
    if (ie != NULL) {
        mac_ie_proc_ext_cap_ie(&(hmac_user_sta->st_user_base_info), ie);
    }

    /* 走到这里，说明sta已经被统计到ht相关的统计量中 */
    hmac_user_sta->st_user_stats_flag.bit_no_ht_stats_flag = OAL_TRUE;
    hmac_user_sta->st_user_stats_flag.bit_no_gf_stats_flag = OAL_TRUE;
    hmac_user_sta->st_user_stats_flag.bit_20M_only_stats_flag = OAL_TRUE;
    hmac_user_sta->st_user_stats_flag.bit_no_lsig_txop_stats_flag = OAL_TRUE;
    hmac_user_sta->st_user_stats_flag.bit_no_40dsss_stats_flag = OAL_TRUE;

    protection = &(hmac_vap->st_vap_base_info.st_protection);
    hmac_vap_update_sta_nums_of_protection_param(hmac_user_sta, protection, ht_hdl, en_prev_asoc_non_ht,
        en_prev_asoc_ht);

    return MAC_SUCCESSFUL_STATUSCODE;
}


void hmac_search_txbf_cap_ie_ap(mac_user_ht_hdl_stru *ht_hdl, uint32_t info_elem)
{
    uint32_t tmp_txbf_elem = info_elem;

    ht_hdl->bit_imbf_receive_cap = (tmp_txbf_elem & BIT0);
    ht_hdl->bit_receive_staggered_sounding_cap = ((tmp_txbf_elem & BIT1) >> NUM_1_BITS);
    ht_hdl->bit_transmit_staggered_sounding_cap = ((tmp_txbf_elem & BIT2) >> NUM_2_BITS);
    ht_hdl->bit_receive_ndp_cap = ((tmp_txbf_elem & BIT3) >> NUM_3_BITS);
    ht_hdl->bit_transmit_ndp_cap = ((tmp_txbf_elem & BIT4) >> NUM_4_BITS);
    ht_hdl->bit_imbf_cap = ((tmp_txbf_elem & BIT5) >> NUM_5_BITS);
    ht_hdl->bit_calibration = ((tmp_txbf_elem & 0x000000C0) >> NUM_6_BITS);
    ht_hdl->bit_exp_csi_txbf_cap = ((tmp_txbf_elem & BIT8) >> NUM_8_BITS);
    ht_hdl->bit_exp_noncomp_txbf_cap = ((tmp_txbf_elem & BIT9) >> NUM_9_BITS);
    ht_hdl->bit_exp_comp_txbf_cap = ((tmp_txbf_elem & BIT10) >> NUM_10_BITS);
    ht_hdl->bit_exp_csi_feedback = ((tmp_txbf_elem & 0x00001800) >> NUM_11_BITS);
    ht_hdl->bit_exp_noncomp_feedback = ((tmp_txbf_elem & 0x00006000) >> NUM_13_BITS);

    ht_hdl->bit_exp_comp_feedback = ((tmp_txbf_elem & 0x0001C000) >> NUM_15_BITS);
    ht_hdl->bit_min_grouping = ((tmp_txbf_elem & 0x00060000) >> NUM_17_BITS);
    ht_hdl->bit_csi_bfer_ant_number = ((tmp_txbf_elem & 0x001C0000) >> NUM_19_BITS);
    ht_hdl->bit_noncomp_bfer_ant_number = ((tmp_txbf_elem & 0x00600000) >> NUM_21_BITS);
    ht_hdl->bit_comp_bfer_ant_number = ((tmp_txbf_elem & 0x01C00000) >> NUM_23_BITS);
    ht_hdl->bit_csi_bfee_max_rows = ((tmp_txbf_elem & 0x06000000) >> NUM_25_BITS);
    ht_hdl->bit_channel_est_cap = ((tmp_txbf_elem & 0x18000000) >> NUM_27_BITS);
}


uint32_t hmac_search_ht_cap_ie_ap(hmac_vap_stru *hmac_vap,
                                  hmac_user_stru *hmac_user_sta,
                                  uint8_t *payload,
                                  uint16_t current_offset,
                                  oal_bool_enum en_prev_asoc_ht)
{
    uint8_t smps;
    uint8_t supported_channel_width;
    uint8_t ht_green_field;
    uint8_t lsig_txop_protection_support;
    uint8_t mcs_bmp_index;
    uint8_t *tmp_payload = NULL;
    uint16_t tmp_info_elem;
    uint16_t tmp_txbf_low;
    uint32_t tmp_txbf_elem;

    mac_user_ht_hdl_stru *p_ht_hdl = NULL;
    mac_user_ht_hdl_stru ht_hdl;
    mac_vap_stru *mac_vap = NULL;
    mac_user_stru *mac_user = NULL;
    mac_device_stru *mac_device;

    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                       "{hmac_search_ht_cap_ie_ap::mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 保存 入参 */
    tmp_payload = payload;

    if (tmp_payload[1] < MAC_HT_CAP_LEN) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_search_ht_cap_ie_ap::invalid ht cap len[%d]}", tmp_payload[1]);
        return OAL_FAIL;
    }

    p_ht_hdl = &ht_hdl;
    mac_user_get_ht_hdl(&(hmac_user_sta->st_user_base_info), p_ht_hdl);
    mac_vap = &(hmac_vap->st_vap_base_info);
    mac_user = &(hmac_user_sta->st_user_base_info);

    /* 带有 HT Capability Element 的 STA，标示它具有HT capable. */
    p_ht_hdl->en_ht_capable = 1;
    current_offset += MAC_IE_HDR_LEN;

    /***************************************************************************
                    解析 HT Capabilities Info Field
    ***************************************************************************/
    tmp_info_elem = oal_make_word16(tmp_payload[current_offset], tmp_payload[current_offset + 1]);

    /* 检查STA所支持的LDPC编码能力 B0，0:不支持，1:支持 */
    p_ht_hdl->bit_ldpc_coding_cap = (tmp_info_elem & BIT0);

    /* 检查STA所支持的信道宽度 B1，0:仅20M运行，1:20M与40M运行 */
    supported_channel_width = (tmp_info_elem & BIT1) >> 1;
    p_ht_hdl->bit_supported_channel_width = mac_ie_proc_ht_supported_channel_width(mac_user, mac_vap,
        supported_channel_width, en_prev_asoc_ht);

    /* 检查空间复用节能模式 B2~B3 */
    smps = ((tmp_info_elem & (BIT3 | BIT2)) >> NUM_2_BITS);
    p_ht_hdl->bit_sm_power_save = mac_ie_proc_sm_power_save_field(mac_user, smps);

    /* 检查Greenfield 支持情况 B4， 0:不支持，1:支持 */
    ht_green_field = (tmp_info_elem & BIT4) >> NUM_4_BITS;
    p_ht_hdl->bit_ht_green_field = mac_ie_proc_ht_green_field(mac_user, mac_vap, ht_green_field, en_prev_asoc_ht);

    /* 检查20MHz Short-GI B5,  0:不支持，1:支持，之后与AP取交集  */
    p_ht_hdl->bit_short_gi_20mhz = ((tmp_info_elem & BIT5) >> NUM_5_BITS);
    p_ht_hdl->bit_short_gi_20mhz &= mac_mib_get_ShortGIOptionInTwentyImplemented(mac_vap);
    /* 检查40MHz Short-GI B6,  0:不支持，1:支持，之后与AP取交集 */
    p_ht_hdl->bit_short_gi_40mhz = ((tmp_info_elem & BIT6) >> NUM_6_BITS);
    p_ht_hdl->bit_short_gi_40mhz &= mac_mib_get_ShortGIOptionInFortyImplemented(&hmac_vap->st_vap_base_info);

    /* 检查支持接收STBC PPDU B8,  0:不支持，1:支持 */
    p_ht_hdl->bit_rx_stbc = ((tmp_info_elem & 0x0300) >> NUM_8_BITS);

    /* 检查最大A-MSDU长度 B11，0:3839字节, 1:7935字节 */
    hmac_user_sta->amsdu_maxsize = ((tmp_info_elem & BIT11) == 0) ?
        WLAN_MIB_MAX_AMSDU_LENGTH_SHORT : WLAN_MIB_MAX_AMSDU_LENGTH_LONG;

    /* 检查在40M上DSSS/CCK的支持情况 B12 */
    /* 在非Beacon帧或probe rsp帧中时 */
    /* 0: STA在40MHz上不使用DSSS/CCK, 1: STA在40MHz上使用DSSS/CCK */
    p_ht_hdl->bit_dsss_cck_mode_40mhz = ((tmp_info_elem & BIT12) >> NUM_12_BITS);

    if ((p_ht_hdl->bit_dsss_cck_mode_40mhz == 0) &&
        (p_ht_hdl->bit_supported_channel_width == 1)) {
        hmac_vap->st_vap_base_info.st_protection.uc_sta_no_40dsss_cck_num++;
    }

    /* 检查Forty MHz Intolerant */
    p_ht_hdl->bit_forty_mhz_intolerant = ((tmp_info_elem & BIT14) >> NUM_14_BITS);
    /*  检查对L-SIG TXOP 保护的支持情况 B15, 0:不支持，1:支持 */
    lsig_txop_protection_support = (tmp_info_elem & BIT15) >> NUM_15_BITS;
    p_ht_hdl->bit_lsig_txop_protection = mac_ie_proc_lsig_txop_protection_support(mac_user, mac_vap,
        lsig_txop_protection_support, en_prev_asoc_ht);

    current_offset += MAC_HT_CAPINFO_LEN;

    /***************************************************************************
                        解析 A-MPDU Parameters Field
    ***************************************************************************/
    /* 提取 Maximum Rx A-MPDU factor (B1 - B0) */
    p_ht_hdl->uc_max_rx_ampdu_factor = (tmp_payload[current_offset] & 0x03);

    /* 提取 the Minimum MPDU Start Spacing (B2 - B4) */
    p_ht_hdl->uc_min_mpdu_start_spacing = (tmp_payload[current_offset] >> 2) & 0x07;

    current_offset += MAC_HT_AMPDU_PARAMS_LEN;

    /***************************************************************************
                        解析 Supported MCS Set Field
    ***************************************************************************/
    for (mcs_bmp_index = 0; mcs_bmp_index < WLAN_HT_MCS_BITMASK_LEN; mcs_bmp_index++) {
        p_ht_hdl->uc_rx_mcs_bitmask[mcs_bmp_index] = (*(uint8_t *)(tmp_payload + current_offset + mcs_bmp_index));
    }

    p_ht_hdl->uc_rx_mcs_bitmask[WLAN_HT_MCS_BITMASK_LEN - 1] &= 0x1F;

    current_offset += MAC_HT_SUP_MCS_SET_LEN;

    /***************************************************************************
                        解析 HT Extended Capabilities Info Field
    ***************************************************************************/
    tmp_info_elem = oal_make_word16(tmp_payload[current_offset], tmp_payload[current_offset + 1]);
    /* 提取 HTC support Information */
    if ((tmp_info_elem & BIT10) != 0) {
        p_ht_hdl->uc_htc_support = 1;
    }

    current_offset += MAC_HT_EXT_CAP_LEN;

    /***************************************************************************
                        解析 Tx Beamforming Field
    ***************************************************************************/
    tmp_info_elem = oal_make_word16(tmp_payload[current_offset], tmp_payload[current_offset + 1]);
    tmp_txbf_low = oal_make_word16(tmp_payload[current_offset + BYTE_OFFSET_2],
                                   tmp_payload[current_offset + BYTE_OFFSET_3]);
    tmp_txbf_elem = oal_make_word32(tmp_info_elem, tmp_txbf_low);
    hmac_search_txbf_cap_ie_ap(p_ht_hdl, tmp_txbf_elem);

    mac_user_set_ht_hdl(&(hmac_user_sta->st_user_base_info), p_ht_hdl);
    hmac_chan_update_40m_intol_user(mac_vap);
    oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                     "{hmac_search_ht_cap_ie_ap::en_40M_intol_user[%d].}", mac_vap->en_40M_intol_user);

    /* 如果存在40M不容忍的user，则不允许AP在40MHz运行 */
    if (mac_vap->en_40M_intol_user == OAL_TRUE) {
        if (mac_vap->st_cap_flag.bit_2040_autoswitch) {
            hmac_40m_intol_sync_data(mac_vap, WLAN_BAND_WIDTH_BUTT, mac_vap->en_40M_intol_user);
        }
    } else {
        mac_mib_set_FortyMHzIntolerant(mac_vap, OAL_FALSE);
    }
    return OAL_SUCC;
}


void hmac_vap_net_stopall(void)
{
    uint8_t vap_id;
    oal_net_device_stru *net_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    for (vap_id = 0; vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; vap_id++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
        if (hmac_vap == NULL) {
            break;
        }

        net_device = hmac_vap->pst_net_device;

        if (net_device == NULL) {
            break;
        }

        oal_net_tx_stop_all_queues(net_device);
    }
}

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW

void hmac_set_ampdu_worker(oal_delayed_work *work)
{
    hmac_vap_stru *hmac_vap;

    hmac_vap = oal_container_of(work, hmac_vap_stru, st_ampdu_work);

    /* 配置完毕后再开启使能聚合 */
    mac_mib_set_CfgAmpduTxAtive(&hmac_vap->st_vap_base_info, OAL_TRUE);
    oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{hmac_set_ampdu_worker:: enable ampdu!}");
}


void hmac_set_ampdu_hw_worker(oal_delayed_work *work)
{
    uint32_t ret;
    hmac_vap_stru *hmac_vap;
    mac_cfg_ampdu_tx_on_param_stru ampdu_tx_on = { 0 };

    hmac_vap = oal_container_of(work, hmac_vap_stru, st_set_hw_work);
    /* 聚合硬化:4 */
    if ((hmac_vap->st_mode_set.uc_aggr_tx_on >> NUM_2_BITS) & BIT0) {
        ampdu_tx_on.uc_aggr_tx_on = 1;
        ampdu_tx_on.uc_snd_type = hmac_vap->st_mode_set.uc_snd_type;
    } else { /* 切换为软件聚合:8 */
        ampdu_tx_on.uc_aggr_tx_on = 0;
        ampdu_tx_on.uc_snd_type = 0;
    }

    /***************************************************************************
            抛事件到DMAC层, 同步DMAC数据
     ***************************************************************************/
    ret = hmac_config_send_event(&hmac_vap->st_vap_base_info, WLAN_CFGID_AMPDU_TX_ON,
                                 sizeof(mac_cfg_ampdu_tx_on_param_stru), (uint8_t *)&ampdu_tx_on);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                         "hmac_set_ampdu_hw_worker::hmac_config_send_event fail(%u)", ret);
    }
    oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                     "{hmac_set_ampdu_hw_worker:: en_tx_aggr_on[0x%x],snd type[%d]!}",
                     hmac_vap->st_mode_set.uc_aggr_tx_on, hmac_vap->st_mode_set.uc_snd_type);
}
#endif


oal_bool_enum_uint8 hmac_flowctl_check_device_is_sta_mode(void)
{
    mac_device_stru *dev = NULL;
    mac_vap_stru *vap = NULL;
    uint8_t vap_index;
    uint8_t device_max;
    uint8_t device;
    oal_bool_enum_uint8 en_device_is_sta = OAL_FALSE;
    mac_chip_stru *mac_chip;

    mac_chip = hmac_res_get_mac_chip(0);

    /* OAL接口获取支持device个数 */
    device_max = oal_chip_get_device_num(mac_chip->chip_ver);

    for (device = 0; device < device_max; device++) {
        dev = mac_res_get_dev(mac_chip->auc_device_id[device]);
        if (oal_unlikely(dev == NULL)) {
            continue;
        }

        if (dev->uc_vap_num < 1) {
            continue;
        }
        for (vap_index = 0; vap_index < dev->uc_vap_num; vap_index++) {
            vap = mac_res_get_mac_vap(dev->auc_vap_id[vap_index]);
            if (vap == NULL) {
                oam_info_log1(0, OAM_SF_BA,
                    "{hmac_flowctl_check_device_is_sta_mode::mac_res_get_mac_vap fail.vap_index = %u}", vap_index);
                continue;
            }

            if (vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
                en_device_is_sta = OAL_TRUE;
                break;
            }
        }
    }
    return en_device_is_sta;
}


OAL_STATIC void hmac_vap_wake_subq(uint8_t vap_id, uint16_t queue_idx)
{
    oal_net_device_stru *net_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == NULL) {
        return;
    }

    net_device = hmac_vap->pst_net_device;
    if (net_device == NULL) {
        return;
    }

    oal_net_wake_subqueue(net_device, queue_idx);
}


void hmac_vap_net_start_subqueue(uint16_t queue_idx)
{
    uint8_t vap_id;
    OAL_STATIC uint8_t g_uc_start_subq_flag = 0;

    /* 自旋锁内，任务和软中断都被锁住，不需要FRW锁 */
    if (g_uc_start_subq_flag == 0) {
        g_uc_start_subq_flag = 1;
        vap_id = oal_board_get_service_vap_start_id();

        /* vap id从低到高恢复 跳过配置vap */
        for (; vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; vap_id++) {
            hmac_vap_wake_subq(vap_id, queue_idx);
        }
    } else {
        g_uc_start_subq_flag = 0;
        vap_id = WLAN_VAP_SUPPORT_MAX_NUM_LIMIT;

        /* vap id从高到低恢复 */
        for (; vap_id > oal_board_get_service_vap_start_id(); vap_id--) {
            hmac_vap_wake_subq(vap_id - 1, queue_idx);
        }
    }
}


OAL_STATIC void hmac_vap_stop_subq(uint8_t vap_id, uint16_t queue_idx)
{
    oal_net_device_stru *net_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == NULL) {
        return;
    }

    net_device = hmac_vap->pst_net_device;
    if (net_device == NULL) {
        return;
    }

    oal_net_stop_subqueue(net_device, queue_idx);
}


void hmac_vap_net_stop_subqueue(uint16_t queue_idx)
{
    uint8_t vap_id;
    OAL_STATIC uint8_t g_uc_stop_subq_flag = 0;

    /* 自旋锁内，任务和软中断都被锁住，不需要FRW锁 */
    /* 由按照VAP ID顺序停止subq，改为不依据VAP ID顺序 */
    if (g_uc_stop_subq_flag == 0) {
        g_uc_stop_subq_flag = 1;
        vap_id = oal_board_get_service_vap_start_id();

        for (; vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; vap_id++) {
            hmac_vap_stop_subq(vap_id, queue_idx);
        }
    } else {
        g_uc_stop_subq_flag = 0;
        vap_id = WLAN_VAP_SUPPORT_MAX_NUM_LIMIT;

        for (; vap_id > oal_board_get_service_vap_start_id(); vap_id--) {
            hmac_vap_stop_subq(vap_id - 1, queue_idx);
        }
    }
}


void hmac_vap_net_startall(void)
{
    uint8_t vap_id;
    oal_net_device_stru *net_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    for (vap_id = 0; vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; vap_id++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
        if (hmac_vap == NULL) {
            break;
        }

        net_device = hmac_vap->pst_net_device;

        if (net_device == NULL) {
            break;
        }

        oal_net_tx_wake_all_queues(net_device);
    }
}


void hmac_del_virtual_inf_worker(oal_work_stru *del_virtual_inf_work)
{
    oal_net_device_stru *net_dev = NULL;
    hmac_vap_stru *hmac_vap;
    oal_wireless_dev_stru *wireless_dev = NULL;
    hmac_device_stru *hmac_device = NULL;

    hmac_vap = oal_container_of(del_virtual_inf_work, hmac_vap_stru, st_del_virtual_inf_worker);
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_P2P, "{hmac_del_virtual_inf_worker:: hmac_vap is null}");
        return;
    }
    net_dev = hmac_vap->pst_del_net_device;
    if (net_dev == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                       "{hmac_del_virtual_inf_worker:: net_dev is null}");
        return;
    }
    wireless_dev = oal_netdevice_wdev(net_dev);

    /* 不存在rtnl_lock锁问题 */
    oal_net_unregister_netdev(net_dev);

    /* 在释放net_device 后释放wireless_device 内存 */
    oal_mem_free_m(wireless_dev, OAL_TRUE);

    hmac_vap->pst_del_net_device = NULL;

    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                       "{hmac_del_virtual_inf_worker::hmac_device is null !device_id[%d]}",
                       hmac_vap->st_vap_base_info.uc_device_id);
        return;
    }

    hmac_clr_p2p_status(&hmac_device->p2p_intf_status, P2P_STATUS_IF_DELETING);

    oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                     "{hmac_del_virtual_inf_worker::end !hmac_device->p2p_intf_status[%x]}",
                     hmac_device->p2p_intf_status);
    oal_smp_mb();
    oal_wait_queue_wake_up_interrupt(&hmac_device->st_netif_change_event);
}


void hmac_handle_disconnect_rsp(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
                                mac_reason_code_enum_uint16 en_disasoc_reason)
{
    hmac_disconnect disconnect;

    /* 修改 state & 删除 user */
    switch (hmac_vap->st_vap_base_info.en_vap_mode) {
        case WLAN_VAP_MODE_BSS_AP: {
            /* 抛事件上报内核，已经去关联某个STA */
            hmac_handle_disconnect_rsp_ap(hmac_vap, hmac_user);
            break;
        }
        case WLAN_VAP_MODE_BSS_STA: {
            /* 上报内核sta已经和某个ap去关联 */
            disconnect.reason_code = en_disasoc_reason;
            disconnect.type = WLAN_FC0_SUBTYPE_DISASSOC;
            disconnect.is_roam = OAL_FALSE;
            hmac_sta_handle_disassoc_rsp(hmac_vap, &disconnect);
            break;
        }
        default:
            break;
    }
    return;
}


uint8_t *hmac_vap_get_pmksa(hmac_vap_stru *hmac_vap, uint8_t *bssid)
{
    hmac_pmksa_cache_stru *pmksa_cache = NULL;
    oal_dlist_head_stru *pmksa_entry = NULL;
    oal_dlist_head_stru *pmksa_entry_tmp = NULL;

    if (oal_any_null_ptr2(hmac_vap, bssid)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_vap_get_pmksa param null}\r\n");
        return NULL;
    }

    if (hmac_vap->en_sae_connect == OAL_TRUE) {
        return NULL;
    }

    if (oal_dlist_is_empty(&(hmac_vap->st_pmksa_list_head))) {
        return NULL;
    }

    oal_dlist_search_for_each_safe(pmksa_entry, pmksa_entry_tmp, &(hmac_vap->st_pmksa_list_head))
    {
        pmksa_cache = oal_dlist_get_entry(pmksa_entry, hmac_pmksa_cache_stru, st_entry);
        if (oal_compare_mac_addr(bssid, pmksa_cache->auc_bssid) == 0) {
            oal_dlist_delete_entry(pmksa_entry);
            oam_warning_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                             "{hmac_vap_get_pmksa:: FIND Pmksa of [%02X:XX:XX:XX:%02X:%02X]}",
                             bssid[MAC_ADDR_0], bssid[MAC_ADDR_4], bssid[MAC_ADDR_5]);
            break;
        }
        pmksa_cache = NULL;
    }

    if (pmksa_cache != NULL) {
        oal_dlist_add_head(&(pmksa_cache->st_entry), &(hmac_vap->st_pmksa_list_head));
        return pmksa_cache->auc_pmkid;
    }
    return NULL;
}


uint32_t hmac_tx_get_mac_vap(uint8_t vap_id, mac_vap_stru **vap_stru)
{
    mac_vap_stru *vap;

    /* 获取vap结构信息 */
    vap = (mac_vap_stru *)mac_res_get_mac_vap(vap_id);
    if (oal_unlikely(vap == NULL)) {
        oam_error_log0(vap_id, OAM_SF_TX, "{hmac_tx_get_mac_vap::pst_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* VAP模式判断 */
    if (vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log1(vap->uc_vap_id, OAM_SF_TX, "hmac_tx_get_mac_vap::vap_mode error[%d]", vap->en_vap_mode);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /* VAP状态判断 */
    if (vap->en_vap_state != MAC_VAP_STATE_UP && vap->en_vap_state != MAC_VAP_STATE_PAUSE) {
        oam_warning_log1(vap->uc_vap_id, OAM_SF_TX, "hmac_tx_get_mac_vap::vap_state[%d] error", vap->en_vap_state);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    *vap_stru = vap;

    return OAL_SUCC;
}


uint32_t hmac_restart_all_work_vap(uint8_t chip_id)
{
    mac_chip_stru *mac_chip = NULL;
    mac_device_stru *mac_dev = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_vap_stru *mac_vap = NULL;
    mac_cfg_down_vap_param_stru down_vap;
    mac_cfg_start_vap_param_stru start_vap;
    uint8_t dev_idx, vap_idx;
    uint32_t ret;

    mac_chip = hmac_res_get_mac_chip(chip_id);
    if (oal_unlikely(mac_chip == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_restart_all_work_vap::mac_chip null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 遍历chip下的所有vap，将work的vap down掉后再up */
    for (dev_idx = 0; dev_idx < mac_chip->uc_device_nums; dev_idx++) {
        mac_dev = mac_res_get_dev(mac_chip->auc_device_id[dev_idx]);
        if (oal_unlikely(mac_dev == NULL)) {
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_restart_all_work_vap::mac_dev null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        /* 遍历dev下的所有vap */
        for (vap_idx = 0; vap_idx < mac_dev->uc_vap_num; vap_idx++) {
            hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_dev->auc_vap_id[vap_idx]);
            if (oal_unlikely(hmac_vap == NULL)) {
                oam_warning_log0(mac_dev->auc_vap_id[vap_idx], OAM_SF_CFG,
                                 "{hmac_restart_all_work_vap::hmac_vap null.}");
                return OAL_ERR_CODE_PTR_NULL;
            }
            mac_vap = &hmac_vap->st_vap_base_info;
            /* update vap's maximum user num */
            mac_mib_set_MaxAssocUserNums(mac_vap, mac_chip_get_max_asoc_user(chip_id));

            /* return if not work vap */
            if (mac_vap->en_vap_state == MAC_VAP_STATE_INIT) {
                continue;
            }

            /* restart work vap */
            /* down vap */
            down_vap.pst_net_dev = hmac_vap->pst_net_device;
            ret = hmac_config_down_vap(mac_vap, sizeof(mac_cfg_down_vap_param_stru), (uint8_t *)&down_vap);
            if (ret != OAL_SUCC) {
                oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                                 "{hmac_restart_all_work_vap::hmac_config_down_vap failed[%d].}", ret);
                return ret;
            }
            /* down netdev */
            oal_net_device_close(hmac_vap->pst_net_device);

            /* start vap */
            start_vap.pst_net_dev = hmac_vap->pst_net_device;
            start_vap.en_mgmt_rate_init_flag = OAL_TRUE;
            ret = hmac_config_start_vap(mac_vap, sizeof(mac_cfg_start_vap_param_stru), (uint8_t *)&start_vap);
            if (ret != OAL_SUCC) {
                oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                                 "{hmac_config_wifi_enable::hmac_config_start_vap failed[%d].}", ret);
                return ret;
            }
            /* up netdev */
            oal_net_device_open(hmac_vap->pst_net_device);
        }
    }

    return OAL_SUCC;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
OAL_STATIC int32_t hmac_cfg_vap_if_open(oal_net_device_stru *dev)
{
    dev->flags |= OAL_IFF_RUNNING;

    return OAL_SUCC;
}

OAL_STATIC int32_t hmac_cfg_vap_if_close(oal_net_device_stru *dev)
{
    dev->flags &= ~OAL_IFF_RUNNING;

    return OAL_SUCC;
}

OAL_STATIC oal_net_dev_tx_enum hmac_cfg_vap_if_xmit(oal_netbuf_stru *buf, oal_net_device_stru *dev)
{
    if (buf != NULL) {
        oal_netbuf_free(buf);
    }
    return OAL_NETDEV_TX_OK;
}
#endif

void hmac_vap_state_restore(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = NULL;

    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                         "{hmac_vap_state_restore::hmac_device[%d] null.}",
                         hmac_vap->st_vap_base_info.uc_device_id);
        return;
    }

    if (hmac_vap->st_vap_base_info.en_p2p_mode != WLAN_LEGACY_VAP_MODE) {
        mac_vap_state_change(&hmac_vap->st_vap_base_info,
                             hmac_device->pst_device_base_info->st_p2p_info.en_last_vap_state);
    } else {
        mac_vap_state_change(&hmac_vap->st_vap_base_info, hmac_vap->st_vap_base_info.en_last_vap_state);
    }

    return;
}

