

/* 1 头文件包含 */
#include "wlan_spec.h"
#include "wlan_types.h"
#include "wlan_chip_i.h"

#include "mac_vap.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#include "mac_ie.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "board.h"
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
#include "mac_ftm.h"
#endif
#include "hmac_vowifi.h"
#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif
#include "mac_mib.h"
#include "hmac_11ax.h"
#include "hmac_11w.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_VAP_C

/* 2 全局变量定义 */
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
mac_tx_large_amsdu_ampdu_stru g_st_tx_large_amsdu = { 0 };
#endif
#ifdef _PRE_WLAN_TCP_OPT
mac_tcp_ack_filter_stru g_st_tcp_ack_filter = { 0 };
#endif
mac_rx_buffer_size_stru g_st_rx_buffer_size_stru = { 0 };
mac_small_amsdu_switch_stru g_st_small_amsdu_switch = { 0 };

mac_tcp_ack_buf_switch_stru g_st_tcp_ack_buf_switch = { 0 };

mac_rx_dyn_bypass_extlna_stru g_st_rx_dyn_bypass_extlna_switch = { 0 };
#ifdef _PRE_WLAN_FEATURE_M2S
oal_bool_enum_uint8 g_en_mimo_blacklist = OAL_TRUE;
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
oal_bool_enum_uint8 g_en_hid2d_debug = OAL_FALSE;
mac_hid2d_debug_switch_stru g_st_hid2d_debug_switch = { 0 };
#endif
mac_data_collect_cfg_stru g_st_data_collect_cfg = {0};
#ifdef _PRE_WLAN_FEATURE_MBO
uint8_t g_uc_mbo_switch = 0;
#endif
uint8_t g_uc_dbac_dynamic_switch = 0;

mac_rx_dyn_bypass_extlna_stru *mac_vap_get_rx_dyn_bypass_extlna_switch(void)
{
    return &g_st_rx_dyn_bypass_extlna_switch;
}

mac_tcp_ack_buf_switch_stru *mac_vap_get_tcp_ack_buf_switch(void)
{
    return &g_st_tcp_ack_buf_switch;
}

mac_small_amsdu_switch_stru *mac_vap_get_small_amsdu_switch(void)
{
    return &g_st_small_amsdu_switch;
}

mac_rx_buffer_size_stru *mac_vap_get_rx_buffer_size(void)
{
    return &g_st_rx_buffer_size_stru;
}

mac_vap_stru *mac_vap_find_another_up_vap_by_mac_vap(mac_vap_stru *pst_src_vap)
{
    mac_device_stru *pst_mac_device = NULL;
    mac_vap_stru *pst_another_vap_up = NULL;

    pst_mac_device = mac_res_get_dev(pst_src_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_warning_log0(pst_src_vap->uc_vap_id,  OAM_SF_ANY,
            "mac_vap_find_another_vap_by_vap_id:: mac_device is NULL");
        return NULL;
    }

    /* 获取另一个vap */
    pst_another_vap_up = mac_device_find_another_up_vap(pst_mac_device, pst_src_vap->uc_vap_id);
    if (pst_another_vap_up == NULL) {
        return NULL;
    }

    return pst_another_vap_up;
}

oal_bool_enum_uint8 mac_vap_need_set_user_htc_cap_1103(mac_vap_stru *mac_vap)
{
    return (IS_LEGACY_STA(mac_vap));
}

oal_bool_enum_uint8 mac_vap_need_set_user_htc_cap_1106(mac_vap_stru *mac_vap)
{
    return (IS_LEGACY_VAP(mac_vap));
}

oal_bool_enum_uint8 mac_get_rx_6g_flag_1106(dmac_rx_ctl_stru *rx_ctrl)
{
    return rx_ctrl->st_rx_info.is_6ghz_flag;
}

oal_bool_enum_uint8 mac_get_rx_6g_flag_1103(dmac_rx_ctl_stru *rx_ctrl)
{
    return OAL_FALSE;
}


void mac_vap_tx_data_set_user_htc_cap(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
#ifdef _PRE_WLAN_FEATURE_11AX
        mac_he_hdl_stru *pst_he_hdl = NULL;

        MAC_USER_TX_DATA_INCLUDE_HTC(pst_mac_user) = OAL_FALSE;
        MAC_USER_TX_DATA_INCLUDE_OM(pst_mac_user)  = OAL_FALSE;

        if (!wlan_chip_mac_vap_need_set_user_htc_cap(pst_mac_vap)) {
            return;
        }

        if ((pst_mac_vap->bit_htc_include_custom_switch == OAL_TRUE) &&
            MAC_VAP_IS_WORK_HE_PROTOCOL(pst_mac_vap) && MAC_USER_IS_HE_USER(pst_mac_user)) {
            pst_he_hdl = MAC_USER_HE_HDL_STRU(pst_mac_user);
            /* user he cap +HTC-HE Support 为1 时才会携带htc头 */
            if (pst_he_hdl->st_he_cap_ie.st_he_mac_cap.bit_htc_he_support) {
                MAC_USER_TX_DATA_INCLUDE_HTC(pst_mac_user) = OAL_TRUE;
                /* user 支持rom */ /* 嵌套深度优化封装 */
                mac_vap_user_set_tx_data_include_om(pst_mac_vap, pst_mac_user);
            }
        }

        if (MAC_USER_ARP_PROBE_CLOSE_HTC(pst_mac_user)) {
            oam_warning_log0(0, 0, "{mac_vap_tx_data_set_user_htc_cap::bit_arp_probe_close_htc true, to close htc.}");
            MAC_USER_TX_DATA_INCLUDE_HTC(pst_mac_user) = OAL_FALSE;
            MAC_USER_TX_DATA_INCLUDE_OM(pst_mac_user)  = OAL_FALSE;
        }

        oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_11AX, "{mac_vap_tx_data_set_user_htc_cap:: \
            tx_data_include_htc=[%d], tx_data_include_om=[%d], vap_protocol=[%d], he_cap=[%d].}",
            MAC_USER_TX_DATA_INCLUDE_HTC(pst_mac_user), MAC_USER_TX_DATA_INCLUDE_OM(pst_mac_user),
            MAC_VAP_IS_WORK_HE_PROTOCOL(pst_mac_vap), MAC_USER_IS_HE_USER(pst_mac_user));

#endif
    } else {
        return;
    }
}


void mac_vap_init_mib_11n_txbf(mac_vap_stru *pst_mac_vap)
{
    /* txbf能力信息 注:11n txbf用宏区分 */
#if (defined(_PRE_WLAN_FEATURE_TXBF) && defined(_PRE_WLAN_FEATURE_TXBF_HT))
    mac_device_stru *pst_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_dev == NULL) {
        return;
    }

    mac_mib_set_TransmitStaggerSoundingOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_SUBFER(pst_dev));
    mac_mib_set_ReceiveStaggerSoundingOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_SUBFEE(pst_dev));
    mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(pst_mac_vap, WLAN_MIB_HT_ECBF_DELAYED);
    mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(pst_mac_vap, HT_BFEE_NTX_SUPP_ANTA_NUM);
#else
    mac_mib_set_TransmitStaggerSoundingOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_ReceiveStaggerSoundingOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(pst_mac_vap, WLAN_MIB_HT_ECBF_INCAPABLE);
    mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(pst_mac_vap, 1);
#endif
}

void mac_vap_init_11n_rates_extend(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev)
{
#ifdef _PRE_WLAN_FEATURE_11AC2G
    if ((pst_mac_vap->en_protocol == WLAN_HT_MODE) &&
        (pst_mac_vap->st_cap_flag.bit_11ac2g == OAL_TRUE) &&
        (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
        mac_vap_init_11ac_rates(pst_mac_vap, pst_mac_dev);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
    /* m2s spec模式下，需要根据vap nss能力 + cap是否支持siso，刷新空间流 */
    if (MAC_VAP_SPEC_IS_SW_NEED_M2S_SWITCH(pst_mac_vap) && IS_VAP_SINGLE_NSS(pst_mac_vap)) {
        /* 将MIB值的MCS MAP清零 */
        memset_s(mac_mib_get_SupportedMCSTx(pst_mac_vap), WLAN_HT_MCS_BITMASK_LEN, 0, WLAN_HT_MCS_BITMASK_LEN);
        memset_s(mac_mib_get_SupportedMCSRx(pst_mac_vap), WLAN_HT_MCS_BITMASK_LEN, 0, WLAN_HT_MCS_BITMASK_LEN);

        mac_mib_set_TxMaximumNumberSpatialStreamsSupported(pst_mac_vap, 1);
        mac_mib_set_SupportedMCSRxValue(pst_mac_vap, 0, 0xFF); /* 支持 RX MCS 0-7，8位全置为1 */
        mac_mib_set_SupportedMCSTxValue(pst_mac_vap, 0, 0xFF); /* 支持 TX MCS 0-7，8位全置为1 */

        mac_mib_set_HighestSupportedDataRate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_20M_11N);

        if ((pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS) ||
            (pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS)) {
            /* 40M 支持MCS32 */
            mac_mib_set_SupportedMCSRxValue(pst_mac_vap, 4, 0x01); /* 4 MCSRxValue 支持 RX MCS 32,最后一位为1 */
            mac_mib_set_SupportedMCSTxValue(pst_mac_vap, 4, 0x01); /* 4 MCSRxValue 支持 RX MCS 32,最后一位为1 */
            mac_mib_set_HighestSupportedDataRate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_40M_11N);
        }

        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{mac_vap_init_11n_rates_cb::m2s spec update rate to siso.}");
    }
#endif
}


void mac_blacklist_free_pointer(mac_vap_stru *pst_mac_vap, mac_blacklist_info_stru *pst_blacklist_info)
{
    if ((pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) ||
        (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA)) {
        memset_s(pst_blacklist_info, sizeof(mac_blacklist_info_stru), 0, sizeof(mac_blacklist_info_stru));
        pst_blacklist_info->uc_blacklist_device_index = 0xFF;
        pst_blacklist_info->uc_blacklist_vap_index = 0xFF;
    }
}

OAL_STATIC OAL_INLINE void mac_vap_update_ip_filter(mac_vap_stru *pst_vap,
    mac_cfg_add_vap_param_stru *pst_param)
{
    if (IS_STA(pst_vap) && (pst_param->en_p2p_mode == WLAN_LEGACY_VAP_MODE)) {
        /* 仅LEGACY_STA支持 */
        pst_vap->st_cap_flag.bit_ip_filter = OAL_TRUE;
        pst_vap->st_cap_flag.bit_icmp_filter = OAL_TRUE;
    } else {
        pst_vap->st_cap_flag.bit_ip_filter = OAL_FALSE;
        pst_vap->st_cap_flag.bit_icmp_filter = OAL_FALSE;
    }
}
#ifdef _PRE_WLAN_FEATURE_TXBF_HT
OAL_STATIC OAL_INLINE void mac_vap_init_txbf_ht_cap(mac_vap_stru *pst_vap)
{
    if (OAL_TRUE == mac_mib_get_TransmitStaggerSoundingOptionImplemented(pst_vap)) {
        pst_vap->st_txbf_add_cap.bit_exp_comp_txbf_cap = OAL_TRUE;
    }
    pst_vap->st_txbf_add_cap.bit_imbf_receive_cap = 0;
    pst_vap->st_txbf_add_cap.bit_channel_est_cap = 0;
    pst_vap->st_txbf_add_cap.bit_min_grouping = 0;
    pst_vap->st_txbf_add_cap.bit_csi_bfee_max_rows = 0;
    pst_vap->bit_ap_11ntxbf = 0;
    pst_vap->st_cap_flag.bit_11ntxbf = OAL_TRUE;
}
#endif

void mac_vap_rom_init(mac_vap_stru *pst_mac_vap, mac_cfg_add_vap_param_stru *pst_param)
{
    oal_bool_enum_uint8 temp_flag;
    /* 仅p2p device支持probe req应答模式切换 */
    pst_mac_vap->st_probe_resp_ctrl.en_probe_resp_enable = OAL_FALSE;
    pst_mac_vap->st_probe_resp_ctrl.en_probe_resp_status = MAC_PROBE_RESP_MODE_ACTIVE;
    temp_flag = ((pst_param->en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
                 (pst_param->en_p2p_mode == WLAN_P2P_DEV_MODE));
    if (temp_flag) {
        pst_mac_vap->st_probe_resp_ctrl.en_probe_resp_enable = (uint8_t)pst_param->probe_resp_enable;
        pst_mac_vap->st_probe_resp_ctrl.en_probe_resp_status = (uint8_t)pst_param->probe_resp_status;
    }
    pst_mac_vap->en_ps_rx_amsdu = OAL_TRUE;
}

uint32_t mac_vap_protocol_init(mac_vap_stru *pst_vap, mac_device_stru *pst_mac_device)
{
    switch (pst_mac_device->en_protocol_cap) {
        case WLAN_PROTOCOL_CAP_LEGACY:
        case WLAN_PROTOCOL_CAP_HT:
            pst_vap->en_protocol = WLAN_HT_MODE;
            break;

        case WLAN_PROTOCOL_CAP_VHT:
            pst_vap->en_protocol = WLAN_VHT_MODE;
            break;
#ifdef _PRE_WLAN_FEATURE_11AX
        case WLAN_PROTOCOL_CAP_HE:
            if (g_wlan_spec_cfg->feature_11ax_is_open) {
                pst_vap->en_protocol = g_pst_mac_device_capability[0].en_11ax_switch ?
                    WLAN_HE_MODE : WLAN_VHT_MODE;
                break;
            }
            oam_warning_log1(pst_vap->uc_vap_id, OAM_SF_CFG,
                "{mac_vap_init::en_protocol_cap[%d] is not supportted.}", pst_mac_device->en_protocol_cap);
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
#endif

        default:
            oam_warning_log1(pst_vap->uc_vap_id, OAM_SF_CFG,
                "{mac_vap_init::en_protocol_cap[%d] is not supportted.}", pst_mac_device->en_protocol_cap);
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    return OAL_SUCC;
}

uint32_t mac_vap_init_band(mac_vap_stru *pst_vap, mac_device_stru *pst_mac_device)
{
    switch (pst_mac_device->en_band_cap) {
        case WLAN_BAND_CAP_2G:
            pst_vap->st_channel.en_band = WLAN_BAND_2G;
            break;

        case WLAN_BAND_CAP_5G:
        case WLAN_BAND_CAP_2G_5G:
            pst_vap->st_channel.en_band = WLAN_BAND_5G;
            break;

        default:
          oam_warning_log1(pst_vap->uc_vap_id, OAM_SF_CFG,
              "{mac_vap_init::en_band_cap[%d] is not supportted.}", pst_mac_device->en_band_cap);
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
            }
    return OAL_SUCC;
}

void mac_vap_ap_init(mac_vap_stru *vap)
{
    vap->us_assoc_vap_id = g_wlan_spec_cfg->invalid_user_id;
    vap->us_cache_user_id = g_wlan_spec_cfg->invalid_user_id;
    vap->uc_tx_power = WLAN_MAX_TXPOWER;
    vap->st_protection.en_protection_mode = WLAN_PROT_NO;
    vap->st_cap_flag.bit_dsss_cck_mode_40mhz = OAL_TRUE;

    /* 初始化特性标识 */
    vap->st_cap_flag.bit_uapsd = WLAN_FEATURE_UAPSD_IS_OPEN;
    if (vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        vap->st_cap_flag.bit_uapsd = g_uc_uapsd_cap;
    }
    /* 初始化dpd能力 */
    vap->st_cap_flag.bit_dpd_enbale = OAL_TRUE;

    vap->st_cap_flag.bit_dpd_done = OAL_FALSE;
    /* 初始化TDLS prohibited关闭 */
    vap->st_cap_flag.bit_tdls_prohibited = OAL_FALSE;
    /* 初始化TDLS channel switch prohibited关闭 */
    vap->st_cap_flag.bit_tdls_channel_switch_prohibited = OAL_FALSE;

    /* 初始化KeepALive开关 */
    vap->st_cap_flag.bit_keepalive = OAL_TRUE;
    /* 初始化安全特性值 */
    vap->st_cap_flag.bit_wpa = OAL_FALSE;
    vap->st_cap_flag.bit_wpa2 = OAL_FALSE;

    mac_vap_set_peer_obss_scan(vap, OAL_FALSE);

    /* 初始化协议模式与带宽为非法值，需通过配置命令配置 */
    vap->st_channel.en_band = WLAN_BAND_BUTT;
    vap->st_channel.en_bandwidth = WLAN_BAND_WIDTH_BUTT;
    vap->st_channel.uc_chan_number = 0;
    vap->st_channel.ext6g_band = OAL_FALSE;
    vap->en_protocol = WLAN_PROTOCOL_BUTT;

    /* 设置自动保护开启 */
    vap->st_protection.bit_auto_protection = OAL_SWITCH_ON;

    memset_s(vap->ast_app_ie, sizeof(mac_app_ie_stru) * OAL_APP_IE_NUM,
             0, sizeof(mac_app_ie_stru) * OAL_APP_IE_NUM);
    /* 初始化vap wmm开关，默认打开 */
    vap->en_vap_wmm = OAL_TRUE;

    /* 设置初始化rx nss值,之后按协议初始化 */
    vap->en_vap_rx_nss = WLAN_NSS_LIMIT;

    /* 设置VAP状态为初始状态INIT */
    mac_vap_state_change(vap, MAC_VAP_STATE_INIT);

    /* 清mac vap下的uapsd的状态,否则状态会有残留，导致host device uapsd信息不同步 */
    memset_s(&(vap->st_sta_uapsd_cfg), sizeof(mac_cfg_uapsd_sta_stru),
             0, sizeof(mac_cfg_uapsd_sta_stru));
}

void mac_vap_init_basic(mac_vap_stru *vap, mac_device_stru *mac_device, uint8_t vap_id,
    mac_cfg_add_vap_param_stru *param)
{
    uint32_t loop;

    vap->uc_chip_id = mac_device->uc_chip_id;
    vap->uc_device_id = mac_device->uc_device_id;
    vap->uc_vap_id = vap_id;
    vap->en_vap_mode = param->en_vap_mode;
    vap->en_p2p_mode = param->en_p2p_mode;
    vap->core_id = mac_device->core_id;
    vap->is_primary_vap = param->is_primary_vap;

    vap->bit_has_user_bw_limit = OAL_FALSE;
    vap->bit_vap_bw_limit = 0;
#ifdef _PRE_WLAN_FEATURE_VO_AGGR
    vap->bit_voice_aggr = OAL_TRUE;
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    /* 组播聚合场景，普通的vo队列不支持聚合;非组播场景下，支持普通vo队列聚合 */
    vap->bit_voice_aggr = (mac_get_mcast_ampdu_switch() == OAL_FALSE);
#endif
#else
    vap->bit_voice_aggr = OAL_FALSE;
#endif
    vap->uc_random_mac = OAL_FALSE;
    vap->bit_bw_fixed = OAL_FALSE;
    vap->bit_use_rts_threshold = OAL_FALSE;

    oal_set_mac_addr_zero(vap->auc_bssid);

    for (loop = 0; loop < MAC_VAP_USER_HASH_MAX_VALUE; loop++) {
        oal_dlist_init_head(&(vap->ast_user_hash[loop]));
    }

    /* cache user 锁初始化 */
    oal_spin_lock_init(&vap->st_cache_user_lock);
    oal_dlist_init_head(&vap->st_mac_user_list_head);

    /* 初始化支持2.4G 11ac私有增强 */
#ifdef _PRE_WLAN_FEATURE_11AC2G
    vap->st_cap_flag.bit_11ac2g = param->bit_11ac2g_enable;
#endif
    /* 默认APUT不支持随环境进行自动2040带宽切换 */
    vap->st_cap_flag.bit_2040_autoswitch = OAL_FALSE;
    /* 根据定制化刷新2g ht40能力 */
    vap->st_cap_flag.bit_disable_2ght40 = param->bit_disable_capab_2ght40;
    mac_vap_update_ip_filter(vap, param);
    mac_vap_rom_init(vap, param);
}

uint32_t mac_vap_sta_init(mac_vap_stru *mac_vap, mac_device_stru *mac_device)
{
    /* 初始化sta协议模式为11ac */ /* 嵌套深度优化封装 */
    if (mac_vap_protocol_init(mac_vap, mac_device) != OAL_SUCC) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    switch (MAC_DEVICE_GET_CAP_BW(mac_device)) {
        case WLAN_BW_CAP_20M:
            mac_vap->st_channel.en_bandwidth = WLAN_BAND_WIDTH_20M;
            break;
        case WLAN_BW_CAP_40M:
            mac_vap->st_channel.en_bandwidth = WLAN_BAND_WIDTH_40MINUS;
            break;
        case WLAN_BW_CAP_80M:
            mac_vap->st_channel.en_bandwidth = WLAN_BAND_WIDTH_80PLUSMINUS;
            break;
#ifdef _PRE_WLAN_FEATURE_160M
        case WLAN_BW_CAP_160M:
            mac_vap->st_channel.en_bandwidth = WLAN_BAND_WIDTH_160PLUSPLUSMINUS;
            break;
#endif
        default:
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_init::bandwidth_cap[%d] is not supportted.}",
                MAC_DEVICE_GET_CAP_BW(mac_device));
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }
    if (mac_vap_init_band(mac_vap, mac_device) != OAL_SUCC) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    if (mac_vap_init_by_protocol(mac_vap, mac_vap->en_protocol) != OAL_SUCC) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    mac_vap_init_rates(mac_vap);
    return OAL_SUCC;
}

uint32_t mac_vap_mib_feature_init(mac_vap_stru *vap,
    mac_device_stru *mac_device, mac_cfg_add_vap_param_stru *param)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        mac_vap_11ax_cap_init(vap);
    }
    if (IS_LEGACY_AP(vap)) {
        vap->aput_bss_color_info = 1;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_MBO
    vap->st_mbo_para_info.uc_mbo_enable = g_uc_mbo_switch;
#endif
    /* 初始化mib值 */
    mac_init_mib(vap);
    /* 根据协议规定 ，VHT rx_ampdu_factor应设置为7             */
    mac_mib_set_vht_max_rx_ampdu_factor(vap, MAC_VHT_AMPDU_MAX_LEN_EXP); /* 2^(13+factor)-1字节 */
#if defined(_PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU)
    /* 从定制化获取是否开启AMSDU_AMPDU */
    mac_mib_set_AmsduPlusAmpduActive(vap, g_st_tx_large_amsdu.uc_host_large_amsdu_en);
#endif
    /* 嵌套深度优化封装 */
    mac_vap_init_vowifi(vap, param);
#ifdef _PRE_WLAN_FEATURE_TXBF_HT
    mac_vap_init_txbf_ht_cap(vap);
#endif

#ifdef _PRE_WLAN_FEATURE_1024QAM
    vap->st_cap_flag.bit_1024qam = 1;
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    vap->st_cap_flag.bit_opmode = 1;
#endif
    /* sta以最大能力启用 */
    if (vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        return mac_vap_sta_init(vap, mac_device);
    }
    return OAL_SUCC;
}


uint32_t mac_vap_init(mac_vap_stru *pst_vap, uint8_t uc_chip_id, uint8_t uc_device_id, uint8_t uc_vap_id,
    mac_cfg_add_vap_param_stru *pst_param)
{
    wlan_mib_ieee802dot11_stru *pst_mib_info = NULL;
    mac_device_stru *mac_device = mac_res_get_dev(uc_device_id);
    uint8_t *puc_addr = NULL;
    oal_bool_enum_uint8 temp_flag;
    uint32_t ret;

    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{mac_vap_init::pst_mac_device[%d] null!}", uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap_init_basic(pst_vap, mac_device, uc_vap_id, pst_param);
    switch (pst_vap->en_vap_mode) {
        case WLAN_VAP_MODE_CONFIG:
            pst_vap->uc_init_flag = MAC_VAP_VAILD;  // CFG VAP也需置位保证不重复初始化
            return OAL_SUCC;
        case WLAN_VAP_MODE_BSS_STA:
        case WLAN_VAP_MODE_BSS_AP:
            mac_vap_ap_init(pst_vap);
            break;
        case WLAN_VAP_MODE_WDS:
        case WLAN_VAP_MODE_MONITOER:
        case WLAN_VAP_HW_TEST:
            break;
        default:
            oam_warning_log1(uc_vap_id, OAM_SF_ANY, "{mac_vap_init::invalid vap mode[%d].}", pst_vap->en_vap_mode);
            return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 申请MIB内存空间，配置VAP没有MIB */
    temp_flag = ((pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) ||
        (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) || (pst_vap->en_vap_mode == WLAN_VAP_MODE_WDS));
    if (temp_flag) {
        pst_vap->pst_mib_info = mac_res_get_mib_info(pst_vap->uc_vap_id);
        if (pst_vap->pst_mib_info == NULL) {
            oam_error_log1(pst_vap->uc_vap_id, OAM_SF_ANY,
                "{mac_vap_init::pst_mib_info alloc null, size[%d].}", sizeof(wlan_mib_ieee802dot11_stru));
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        pst_mib_info = pst_vap->pst_mib_info;
        memset_s(pst_mib_info, sizeof(wlan_mib_ieee802dot11_stru), 0, sizeof(wlan_mib_ieee802dot11_stru));

        /* 设置mac地址 */
        puc_addr = mac_mib_get_StationID(pst_vap);
        oal_set_mac_addr(puc_addr, mac_device->auc_hw_addr);
        puc_addr[WLAN_MAC_ADDR_LEN - 1] += uc_vap_id;
        ret = mac_vap_mib_feature_init(pst_vap, mac_device, pst_param);
        if (ret != OAL_SUCC) {
            return ret;
        }
    }
    pst_vap->uc_init_flag = MAC_VAP_VAILD;

    return OAL_SUCC;
}


uint8_t mac_vap_get_ap_usr_opern_bandwidth(mac_vap_stru *pst_mac_sta, mac_user_stru *pst_mac_user)
{
    mac_user_ht_hdl_stru *pst_mac_ht_hdl = NULL;
    mac_vht_hdl_stru *pst_mac_vht_hdl = NULL;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth_ap = WLAN_BAND_WIDTH_20M;
    wlan_channel_bandwidth_enum_uint8 en_sta_new_bandwidth;
    uint8_t uc_channel_center_freq_seg;
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl_stru *pst_mac_he_hdl = NULL;

    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        pst_mac_he_hdl = MAC_USER_HE_HDL_STRU(pst_mac_user);
    }
#endif

    /* 更新用户的带宽能力cap */
    mac_user_update_ap_bandwidth_cap(pst_mac_user);
    /* 获取HT和VHT结构体指针 */
    pst_mac_ht_hdl = &(pst_mac_user->st_ht_hdl);
    pst_mac_vht_hdl = &(pst_mac_user->st_vht_hdl);

    /******************* VHT BSS operating channel width ****************
     -----------------------------------------------------------------
     |HT Oper Chl Width |VHT Oper Chl Width |BSS Oper Chl Width|
     -----------------------------------------------------------------
     |       0          |        0          |    20MHZ         |
     -----------------------------------------------------------------
     |       1          |        0          |    40MHZ         |
     -----------------------------------------------------------------
     |       1          |        1          |    80MHZ         |
     -----------------------------------------------------------------
     |       1          |        2          |    160MHZ        |
     -----------------------------------------------------------------
     |       1          |        3          |    80+80MHZ      |
     -----------------------------------------------------------------
    **********************************************************************/
    if ((pst_mac_vht_hdl->bit_supported_channel_width == 0) && (pst_mac_vht_hdl->bit_extend_nss_bw_supp != 0)) {
        uc_channel_center_freq_seg = (pst_mac_user->en_user_max_cap_nss == WLAN_SINGLE_NSS) ?
            0 : pst_mac_ht_hdl->uc_chan_center_freq_seg2;
    } else {
        uc_channel_center_freq_seg = pst_mac_vht_hdl->uc_channel_center_freq_seg1;
    }
    if (pst_mac_vht_hdl->en_vht_capable == OAL_TRUE) {
        en_bandwidth_ap = mac_get_bandwith_from_center_freq_seg0_seg1(pst_mac_vht_hdl->en_channel_width,
            pst_mac_sta->st_channel.uc_chan_number, pst_mac_vht_hdl->uc_channel_center_freq_seg0,
            uc_channel_center_freq_seg);
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        if ((OAL_TRUE == MAC_USER_IS_HE_USER(pst_mac_user)) &&
            (pst_mac_he_hdl->st_he_oper_ie.st_he_oper_param.bit_vht_operation_info_present == OAL_TRUE)) {
            en_bandwidth_ap = mac_get_bandwith_from_center_freq_seg0_seg1(
                pst_mac_he_hdl->st_he_oper_ie.st_vht_operation_info.uc_channel_width,
                pst_mac_sta->st_channel.uc_chan_number,
                pst_mac_he_hdl->st_he_oper_ie.st_vht_operation_info.uc_center_freq_seg0,
                pst_mac_he_hdl->st_he_oper_ie.st_vht_operation_info.uc_center_freq_seg1);
        }
    }
#endif

    /* ht 20/40M带宽的处理 */
    if ((pst_mac_ht_hdl->en_ht_capable == OAL_TRUE) && (en_bandwidth_ap <= WLAN_BAND_WIDTH_40MINUS) &&
        (OAL_TRUE == mac_mib_get_FortyMHzOperationImplemented(pst_mac_sta))) {
        /* 更新带宽模式 */
        en_bandwidth_ap = mac_get_bandwidth_from_sco(pst_mac_ht_hdl->bit_secondary_chan_offset);
    }

    /* 带宽不能超过mac device的最大能力 */
    en_sta_new_bandwidth = mac_vap_get_bandwith(mac_mib_get_dot11VapMaxBandWidth(pst_mac_sta), en_bandwidth_ap);
    if (mac_regdomain_channel_is_support_bw(en_sta_new_bandwidth,
        pst_mac_sta->st_channel.uc_chan_number) == OAL_FALSE) {
        oam_warning_log2(0, 0, "{mac_vap_get_ap_usr_opern_bandwidth::channel[%d] is not support bw[%d],set 20MHz}",
                         pst_mac_sta->st_channel.uc_chan_number, en_sta_new_bandwidth);
        en_sta_new_bandwidth = WLAN_BAND_WIDTH_20M;
    }

    return en_sta_new_bandwidth;
}


uint32_t mac_device_find_up_vap_ram(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    uint8_t       uc_vap_idx;
    mac_vap_stru   *pst_mac_vap = NULL;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_mac_vap == NULL)) {
            oam_warning_log1(0, OAM_SF_SCAN, "vap is null! vap id is %d", pst_mac_device->auc_vap_id[uc_vap_idx]);

            *ppst_mac_vap = NULL;

            return OAL_ERR_CODE_PTR_NULL;
        }

        if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP || pst_mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE
            || (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN && pst_mac_vap->us_user_nums > 0)
            || (pst_mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING)
            ) {
            *ppst_mac_vap = pst_mac_vap;

            return OAL_SUCC;
        }
    }

    *ppst_mac_vap = NULL;

    return OAL_FAIL;
}

uint8_t mac_vap_set_bw_check(mac_vap_stru *mac_vap,
    wlan_channel_bandwidth_enum_uint8 en_sta_new_bandwidth)
{
    wlan_channel_bandwidth_enum_uint8 en_band_with_sta_old;
    uint8_t uc_change;

    en_band_with_sta_old = mac_vap->st_channel.en_bandwidth;
    mac_vap->st_channel.en_bandwidth = en_sta_new_bandwidth;

    /* 判断是否需要通知硬件切换带宽 */
    uc_change = (en_band_with_sta_old == en_sta_new_bandwidth) ? MAC_NO_CHANGE : MAC_BW_CHANGE;

    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_ASSOC, "mac_vap_set_bw_check::bandwidth[%d]->[%d].",
                     en_band_with_sta_old, en_sta_new_bandwidth);

    return uc_change;
}

OAL_STATIC void mac_vap_csa_go_support_set(mac_vap_stru *pst_mac_vap)
{
    mac_user_stru *pst_mac_user = NULL;
    uint8_t uc_hash_user_index;
    oal_dlist_head_stru *pst_entry = NULL;

    if (pst_mac_vap->us_user_nums > 1) {
        pst_mac_vap->bit_vap_support_csa = OAL_FALSE;
        return;
    }

    for (uc_hash_user_index = 0; uc_hash_user_index < MAC_VAP_USER_HASH_MAX_VALUE; uc_hash_user_index++) {
        oal_dlist_search_for_each(pst_entry, &(pst_mac_vap->ast_user_hash[uc_hash_user_index]))
        {
            pst_mac_user = (mac_user_stru *)oal_dlist_get_entry(pst_entry, mac_user_stru, st_user_hash_dlist);
            if (pst_mac_user->st_cap_info.bit_p2p_support_csa) {
                pst_mac_vap->bit_vap_support_csa = OAL_TRUE;
            } else {
                pst_mac_vap->bit_vap_support_csa = OAL_FALSE;
                return;
            }
        }
    }
}


void mac_vap_csa_support_set(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_cap)
{
    pst_mac_vap->bit_vap_support_csa = OAL_FALSE;

    if (IS_LEGACY_VAP(pst_mac_vap)) {
        pst_mac_vap->bit_vap_support_csa = OAL_TRUE;
        return;
    }

    if (IS_P2P_CL(pst_mac_vap)) {
        pst_mac_vap->bit_vap_support_csa = en_cap;
        return;
    }
    if (IS_P2P_GO(pst_mac_vap)) {
        mac_vap_csa_go_support_set(pst_mac_vap);
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_csa_support_set::go_support_csa=[%d]!}",
            pst_mac_vap->bit_vap_support_csa);
    }
}


oal_bool_enum_uint8 mac_vap_go_can_not_in_160m_check(mac_vap_stru *p_mac_vap, uint8_t vap_channel)
{
    mac_vap_stru *p_another_mac_vap;

    p_another_mac_vap = mac_vap_find_another_up_vap_by_mac_vap(p_mac_vap);
    if (p_another_mac_vap == NULL) {
        return OAL_FALSE;
    }

    if (p_another_mac_vap->st_channel.uc_chan_number != vap_channel) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

oal_bool_enum_uint8 mac_vap_need_proto_downgrade(mac_vap_stru *vap)
{
    if (g_wlan_spec_cfg->feature_slave_ax_is_support) {
        return OAL_FALSE;
    }
    if (mac_is_primary_legacy_vap(vap)) {
        // 双sta模式下，wlan0不降协议。
        if (mac_is_dual_sta_mode()) {
            return OAL_FALSE;
        }
        // 非双sta模式，wlan0只在漫游时可以降协议。
        if (vap->en_vap_state != MAC_VAP_STATE_ROAMING) {
            return OAL_FALSE;
        }
    }
    return OAL_TRUE;
}


oal_bool_enum_uint8 mac_vap_avoid_dbac_close_vht_protocol(mac_vap_stru *p_mac_vap)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_vap_stru *p_first_vap = NULL;

    if (mac_vap_need_proto_downgrade(p_mac_vap) == OAL_FALSE) {
        return OAL_FALSE;
    }

    p_first_vap = mac_vap_find_another_up_vap_by_mac_vap(p_mac_vap);
    if (p_first_vap == NULL) {
        return OAL_FALSE;
    }

    if (p_first_vap->st_channel.en_band == WLAN_BAND_2G && MAC_VAP_IS_WORK_HE_PROTOCOL(p_first_vap) &&
        p_mac_vap->st_channel.en_band == WLAN_BAND_5G) {
        /* 2G ax先入网，p2p 5G后入网时 */
        oam_warning_log4(p_mac_vap->uc_vap_id, OAM_SF_ANY,
            "mac_vap_p2p_avoid_dbac_close_vht_protocol::first start vap: band=[%d] is_he_protocol=[%d], \
            start vap:band=[%d], p2p mode[%d] close vht", p_first_vap->st_channel.en_band,
            MAC_VAP_IS_WORK_HE_PROTOCOL(p_first_vap), p_mac_vap->st_channel.en_band, p_mac_vap->en_p2p_mode);
        return OAL_TRUE;
    }
#endif
    return OAL_FALSE;
}


oal_bool_enum_uint8 mac_vap_can_not_start_he_protocol(mac_vap_stru *p_mac_vap)
{
    uint8_t close_he_flag = OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_vap_stru *p_first_vap = NULL;

    if (mac_vap_need_proto_downgrade(p_mac_vap) == OAL_FALSE) {
        return OAL_FALSE;
    }

    p_first_vap = mac_vap_find_another_up_vap_by_mac_vap(p_mac_vap);
    if (p_first_vap == NULL) {
        return OAL_FALSE;
    }

    /* 5G先入网，p2p/wlan1 2G后入网时 */
    if (p_first_vap->st_channel.en_band == WLAN_BAND_5G && (MAC_VAP_IS_WORK_HE_PROTOCOL(p_first_vap) ||
        p_first_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) &&
        p_mac_vap->st_channel.en_band == WLAN_BAND_2G) {
        close_he_flag =  OAL_TRUE;
    } else if (OAL_TRUE == mac_vap_avoid_dbac_close_vht_protocol(p_mac_vap)) {
        /* 2G ax先入网，p2p/wlan1 5G后入网时 */
        close_he_flag = OAL_TRUE;
    }

    if (close_he_flag == OAL_TRUE) {
        oam_warning_log3(p_mac_vap->uc_vap_id, OAM_SF_ANY,
            "mac_vap_p2p_can_not_start_he_protocol::first start vap: band=[%d] is_he_protocol=[%d], \
            now start vap:band=[%d] can not star he", p_first_vap->st_channel.en_band,
            MAC_VAP_IS_WORK_HE_PROTOCOL(p_first_vap), p_mac_vap->st_channel.en_band);
    }
#endif
    return close_he_flag;
}


oal_bool_enum_uint8 mac_vap_p2p_bw_back_to_40m(mac_vap_stru *p_mac_vap,
    int32_t channel,  wlan_channel_bandwidth_enum_uint8  *channel_bw)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_vap_stru *p_first_vap = NULL;
    wlan_channel_bandwidth_enum_uint8  channl_band_width;

    /* 辅路不支持ax 直接返回 OAL_FALSE */
    if (mac_vap_need_proto_downgrade(p_mac_vap) == OAL_FALSE) {
        return OAL_FALSE;
    }

    p_first_vap = mac_vap_find_another_up_vap_by_mac_vap(p_mac_vap);
    if (p_first_vap == NULL) {
        return OAL_FALSE;
    }

    /* 当前入网vap 大于等于80M && 2g ax先入网 , p2p回退到40M */
    if (*channel_bw >= WLAN_BAND_WIDTH_80PLUSPLUS &&
        p_first_vap->st_channel.en_band == WLAN_BAND_2G && MAC_VAP_IS_WORK_HE_PROTOCOL(p_first_vap)) {
        channl_band_width = mac_regdomain_get_bw_by_channel_bw_cap(channel, WLAN_BW_CAP_40M);
        if (channl_band_width == WLAN_BAND_WIDTH_40PLUS || channl_band_width == WLAN_BAND_WIDTH_40MINUS) {
            oam_warning_log1(0, 0, "{mac_vap_p2p_bw_back_to_40m::5G bw back to =%d", channl_band_width);
            *channel_bw = channl_band_width;
            return OAL_TRUE;
        }
    }
#endif
    return OAL_FALSE;
}


uint32_t mac_vap_set_cb_tx_user_idx(mac_vap_stru *pst_mac_vap, mac_tx_ctl_stru *pst_tx_ctl,
    const unsigned char *puc_data)
{
    uint16_t us_user_idx = g_wlan_spec_cfg->invalid_user_id;
    uint32_t ret;

    ret = mac_vap_find_user_by_macaddr(pst_mac_vap, puc_data, &us_user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{mac_vap_set_cb_tx_user_idx:: cannot find user_idx from xx:xx:xx:%x:%x:%x, set TX_USER_IDX %d.}",
            puc_data[BYTE_OFFSET_3], puc_data[BYTE_OFFSET_4],
            puc_data[BYTE_OFFSET_5], g_wlan_spec_cfg->invalid_user_id);
        MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = g_wlan_spec_cfg->invalid_user_id;
        return ret;
    }

    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = us_user_idx;

    return OAL_SUCC;
}

uint8_t g_uc_uapsd_cap = WLAN_FEATURE_UAPSD_IS_OPEN;

/* WME初始参数定义，按照OFDM初始化 AP模式 值来自于TGn 9 Appendix D: Default WMM AC Parameters */

mac_wme_param_stru g_ast_wmm_initial_params_ap[WLAN_WME_AC_BUTT] = {
    {
        /* BE AIFS, cwmin, cwmax, txop */
        3,          4,     6,     0,
    },

    {
        /* BK AIFS, cwmin, cwmax, txop */
        7,          4,     10,    0,
    },

    {
        /* VI AIFS, cwmin, cwmax, txop */
        1,          3,     4,     3008,
    },

    {
        /* VO AIFS, cwmin, cwmax, txop */
        1,          2,     3,     1504,
    },
};

/* WMM初始参数定义，按照OFDM初始化 STA模式 */
mac_wme_param_stru g_ast_wmm_initial_params_sta[WLAN_WME_AC_BUTT] = {
    {
        /* BE AIFS, cwmin, cwmax, txop */
        3,          3,     10,     0,
    },

    {
        /* BK AIFS, cwmin, cwmax, txop */
        7,          4,     10,     0,
    },

    {
        /* VI AIFS, cwmin, cwmax, txop */
        2,          3,     4,     3008,
    },

    {
        /* VO AIFS, cwmin, cwmax, txop */
        2,          2,     3,     1504,
    },
};

/* WMM初始参数定义，aput建立的bss中STA的使用的EDCA参数 */
mac_wme_param_stru g_ast_wmm_initial_params_bss[WLAN_WME_AC_BUTT] = {
    {
        /* BE AIFS, cwmin, cwmax, txop */
        3,          4,     10,     0,
    },

    {
        /* BK AIFS, cwmin, cwmax, txop */
        7,          4,     10,     0,
    },

    {
        /* VI AIFS, cwmin, cwmax, txop */
        2,          3,     4,     3008,
    },

    {
        /* VO AIFS, cwmin, cwmax, txop */
        2,          2,     3,     1504,
    },
};

#ifdef _PRE_WLAN_FEATURE_EDCA_MULTI_USER_MULTI_AC
/* 多用户多优先级使用的EDCA参数 */
mac_wme_param_stru g_ast_wmm_multi_user_multi_ac_params_ap[WLAN_WME_AC_BUTT] = {
    {
        /* BE AIFS, cwmin, cwmax, txop */
        3,          5,     10,     0,
    },

    {
        /* BK AIFS, cwmin, cwmax, txop */
        3,          5,     10,     0,
    },

    {
        /* VI AIFS, cwmin, cwmax, txop */
        3,          5,     10,     0,
    },

    {
        /* VO AIFS, cwmin, cwmax, txop */
        3,          5,     10,     0,
    },
};
#endif


mac_wme_param_stru *mac_get_wmm_cfg(wlan_vap_mode_enum_uint8 en_vap_mode)
{
    /* 参考认证项配置，没有按照协议配置，WLAN_VAP_MODE_BUTT表示是ap广播给sta的edca参数 */
    if (en_vap_mode == WLAN_VAP_MODE_BUTT) {
        return (mac_wme_param_stru *)g_ast_wmm_initial_params_bss;
    } else if (en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        return (mac_wme_param_stru *)g_ast_wmm_initial_params_ap;
    }
    return (mac_wme_param_stru *)g_ast_wmm_initial_params_sta;
}

#ifdef _PRE_WLAN_FEATURE_EDCA_MULTI_USER_MULTI_AC

mac_wme_param_stru *mac_get_wmm_cfg_multi_user_multi_ac(oal_traffic_type_enum_uint8 uc_traffic_type)
{
    /* 多用户下业务类型采用新参数，否则采用ap模式下的默认值 */
    if (uc_traffic_type == OAL_TRAFFIC_MULTI_USER_MULTI_AC) {
        return (mac_wme_param_stru *)g_ast_wmm_multi_user_multi_ac_params_ap;
    }

    return (mac_wme_param_stru *)g_ast_wmm_initial_params_ap;
}
#endif

#ifdef _PRE_WLAN_FEATURE_TXOPPS

uint8_t mac_vap_get_txopps(mac_vap_stru *pst_vap)
{
    return pst_vap->st_cap_flag.bit_txop_ps;
}


void mac_vap_set_txopps(mac_vap_stru *pst_vap, uint8_t uc_value)
{
    pst_vap->st_cap_flag.bit_txop_ps = uc_value;
}


void mac_vap_update_txopps(mac_vap_stru *pst_vap, mac_user_stru *pst_user)
{
    /* 如果用户使能txop ps，则vap使能 */
    if (pst_user->st_vht_hdl.bit_vht_txop_ps == OAL_TRUE && OAL_TRUE == mac_mib_get_txopps(pst_vap)) {
        mac_vap_set_txopps(pst_vap, OAL_TRUE);
    }
}

#endif

#ifdef _PRE_WLAN_FEATURE_SMPS

wlan_mib_mimo_power_save_enum mac_vap_get_smps_mode(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MIMOPowerSave;
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

wlan_mib_mimo_power_save_enum mac_vap_get_smps_en(mac_vap_stru *pst_mac_vap)
{
    mac_device_stru *pst_mac_device;

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_SMPS,
                       "{mac_vap_get_smps_en::pst_mac_device[%d] null.}",
                       pst_mac_vap->uc_device_id);

        return (wlan_mib_mimo_power_save_enum)OAL_ERR_CODE_PTR_NULL;
    }

    return (wlan_mib_mimo_power_save_enum)(pst_mac_device->en_mac_smps_mode);
}
#endif


void mac_vap_set_smps(mac_vap_stru *pst_vap, uint8_t uc_value)
{
    pst_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MIMOPowerSave = uc_value;
}
#endif


uint32_t mac_vap_set_uapsd_en(mac_vap_stru *pst_mac_vap, uint8_t uc_value)
{
    pst_mac_vap->st_cap_flag.bit_uapsd = (uc_value == OAL_TRUE) ? 1 : 0;

    return OAL_SUCC;
}


uint8_t mac_vap_get_uapsd_en(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->st_cap_flag.bit_uapsd;
}


uint32_t mac_vap_user_exist(oal_dlist_head_stru *pst_new, oal_dlist_head_stru *pst_head)
{
    oal_dlist_head_stru      *pst_user_list_head = NULL;
    oal_dlist_head_stru      *pst_member_entry = NULL;

    oal_dlist_search_for_each_safe(pst_member_entry, pst_user_list_head, pst_head)
    {
        if (pst_new == pst_member_entry) {
            oam_error_log0(0, OAM_SF_ASSOC, "{oal_dlist_check_head:dmac user doule add.}");
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}


uint32_t mac_vap_add_assoc_user(mac_vap_stru *pst_vap, uint16_t us_user_idx)
{
    mac_user_stru              *pst_user = NULL;
    oal_dlist_head_stru        *pst_dlist_head = NULL;

    if (oal_unlikely(pst_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{mac_vap_add_assoc_user::pst_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_user = (mac_user_stru *)mac_res_get_mac_user(us_user_idx);
    if (oal_unlikely(pst_user == NULL)) {
        oam_warning_log1(pst_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{mac_vap_add_assoc_user::pst_user[%d] null.}", us_user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_user->us_user_hash_idx = MAC_CALCULATE_HASH_VALUE(pst_user->auc_user_mac_addr);

    if (OAL_SUCC == mac_vap_user_exist(&(pst_user->st_user_dlist), &(pst_vap->st_mac_user_list_head))) {
        oam_error_log1(pst_vap->uc_vap_id, OAM_SF_ASSOC,
                       "{mac_vap_add_assoc_user::user[%d] already exist.}", us_user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_spin_lock_bh(&pst_vap->st_cache_user_lock);

    pst_dlist_head = &(pst_vap->ast_user_hash[pst_user->us_user_hash_idx]);
#ifdef _PRE_WLAN_DFT_STAT
    (pst_vap->hash_cnt)++;
#endif

    /* 加入双向hash链表表头 */
    oal_dlist_add_head(&(pst_user->st_user_hash_dlist), pst_dlist_head);

    /* 加入双向链表表头 */
    pst_dlist_head = &(pst_vap->st_mac_user_list_head);
    oal_dlist_add_head(&(pst_user->st_user_dlist), pst_dlist_head);
#ifdef _PRE_WLAN_DFT_STAT
    (pst_vap->dlist_cnt)++;
#endif

    /* 更新cache user */
    oal_set_mac_addr(pst_vap->auc_cache_user_mac_addr, pst_user->auc_user_mac_addr);
    pst_vap->us_cache_user_id = us_user_idx;

    /* 记录STA模式下的与之关联的VAP的id */
    if (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        mac_vap_set_assoc_id(pst_vap, us_user_idx);
    }

    /* vap已关联 user个数++ */
    pst_vap->us_user_nums++;

    oal_spin_unlock_bh(&pst_vap->st_cache_user_lock);

    return OAL_SUCC;
}


uint32_t mac_vap_del_user(mac_vap_stru *pst_vap, uint16_t us_user_idx)
{
    mac_user_stru          *pst_user            = NULL;
    mac_user_stru          *pst_user_temp       = NULL;
    oal_dlist_head_stru    *pst_hash_head       = NULL;
    oal_dlist_head_stru    *pst_entry           = NULL;
    oal_dlist_head_stru    *pst_dlist_tmp       = NULL;
    uint32_t              ret              = OAL_FAIL;
    uint8_t               uc_txop_ps_user_cnt = 0;

    if (oal_unlikely(pst_vap == NULL)) {
        oam_error_log1(0, OAM_SF_ASSOC,
                       "{mac_vap_del_user::pst_vap null,us_user_idx is %d}", us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_spin_lock_bh(&pst_vap->st_cache_user_lock);

    /* 与cache user id对比 , 相等则清空cache user */
    if (us_user_idx == pst_vap->us_cache_user_id) {
        oal_set_mac_addr_zero(pst_vap->auc_cache_user_mac_addr);
        pst_vap->us_cache_user_id = g_wlan_spec_cfg->invalid_user_id;
    }

    pst_user = (mac_user_stru *)mac_res_get_mac_user(us_user_idx);
    if (oal_unlikely(pst_user == NULL)) {
        oal_spin_unlock_bh(&pst_vap->st_cache_user_lock);

        oam_warning_log1(pst_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{mac_vap_del_user::pst_user[%d] null.}", us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_user_set_asoc_state(pst_user, MAC_USER_STATE_BUTT);

    if (pst_user->us_user_hash_idx >= MAC_VAP_USER_HASH_MAX_VALUE) {
        /* ADD USER命令丢失，或者重复删除User都可能进入此分支。 */
        oal_spin_unlock_bh(&pst_vap->st_cache_user_lock);

        oam_error_log1(pst_vap->uc_vap_id, OAM_SF_ASSOC,
                       "{mac_vap_del_user::hash idx invaild %u}",
                       pst_user->us_user_hash_idx);
        return OAL_FAIL;
    }

    pst_hash_head = &(pst_vap->ast_user_hash[pst_user->us_user_hash_idx]);

    oal_dlist_search_for_each_safe(pst_entry, pst_dlist_tmp, pst_hash_head)
    {
        pst_user_temp = (mac_user_stru *)oal_dlist_get_entry(pst_entry, mac_user_stru, st_user_hash_dlist);
        /*lint -save -e774 */
        if (pst_user_temp == NULL) {
            oam_error_log1(pst_vap->uc_vap_id, OAM_SF_ASSOC,
                           "{mac_vap_del_user::pst_user_temp null,us_user_idx is %d}", us_user_idx);

            continue;
        }
        /*lint -restore */
        if (pst_user_temp->st_vht_hdl.bit_vht_txop_ps) {
            uc_txop_ps_user_cnt++;
        }

        if (!oal_compare_mac_addr(pst_user->auc_user_mac_addr, pst_user_temp->auc_user_mac_addr)) {
            oal_dlist_delete_entry(pst_entry);

            /* 从双向链表中拆掉 */
            oal_dlist_delete_entry(&(pst_user->st_user_dlist));

            oal_dlist_delete_entry(&(pst_user->st_user_hash_dlist));
            ret = OAL_SUCC;

#ifdef _PRE_WLAN_DFT_STAT
            (pst_vap->hash_cnt)--;
            (pst_vap->dlist_cnt)--;
#endif
            /* 初始化相应成员 */
            pst_user->us_user_hash_idx = 0xffff;
            pst_user->us_assoc_id      = us_user_idx;
            pst_user->en_is_multi_user = OAL_FALSE;
            memset_s(pst_user->auc_user_mac_addr, WLAN_MAC_ADDR_LEN, 0, WLAN_MAC_ADDR_LEN);
            pst_user->uc_vap_id        = 0xff;
            pst_user->uc_device_id     = 0xff;
            pst_user->uc_chip_id       = 0xff;
            pst_user->en_user_asoc_state = MAC_USER_STATE_BUTT;
        }
    }

    /* 没有关联的用户则去初始化vap能力 */
    if (uc_txop_ps_user_cnt == 0) {
#ifdef _PRE_WLAN_FEATURE_TXOPPS
        mac_vap_set_txopps(pst_vap, OAL_FALSE);
#endif
    }

    if (ret == OAL_SUCC) {
        /* vap已关联 user个数-- */
        if (pst_vap->us_user_nums) {
            pst_vap->us_user_nums--;
        }
        /* STA模式下将关联的VAP的id置为非法值 */
        if (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
            mac_vap_set_assoc_id(pst_vap, g_wlan_spec_cfg->invalid_user_id);
        }

        oal_spin_unlock_bh(&pst_vap->st_cache_user_lock);

        return OAL_SUCC;
    }

    oal_spin_unlock_bh(&pst_vap->st_cache_user_lock);

    oam_warning_log1(pst_vap->uc_vap_id, OAM_SF_ASSOC,
                     "{mac_vap_del_user::delete user failed,user idx is %d.}", us_user_idx);
    return OAL_FAIL;
}


uint32_t mac_vap_find_user_by_macaddr(mac_vap_stru *pst_vap, const unsigned char *puc_sta_mac_addr,
    uint16_t *pus_user_idx)
{
    mac_user_stru              *pst_mac_user = NULL;
    uint32_t                  user_hash_value;
    oal_dlist_head_stru        *pst_entry = NULL;

    if (oal_unlikely(oal_any_null_ptr3(pst_vap, puc_sta_mac_addr, pus_user_idx))) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_vap_find_user_by_macaddr::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*
     * 1.本函数*pus_user_idx先初始化为无效值，防止调用者没有初始化，可能出现使用返回值异常;
     * 2.根据查找结果刷新*pus_user_idx值，如果是有效，返回SUCC,无效MAC_INVALID_USER_ID返回FAIL;
     * 3.调用函数根据首先根据本函数返回值做处理，其次根据*pus_user_idx进行其他需要的判断和操作
     */
    *pus_user_idx = g_wlan_spec_cfg->invalid_user_id;

    if (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(pst_vap->us_assoc_vap_id);
        if (pst_mac_user == NULL) {
            return OAL_FAIL;
        }

        if (!oal_compare_mac_addr(pst_mac_user->auc_user_mac_addr, puc_sta_mac_addr)) {
            *pus_user_idx = pst_vap->us_assoc_vap_id;
        }
    } else {
        oal_spin_lock_bh(&pst_vap->st_cache_user_lock);

        /* 与cache user对比 , 相等则直接返回cache user id */
        if (!oal_compare_mac_addr(pst_vap->auc_cache_user_mac_addr, puc_sta_mac_addr)) {
            /* 用户删除后，user macaddr和cache user macaddr地址均为0，但实际上用户已经删除，此时user id无效 */
            *pus_user_idx = pst_vap->us_cache_user_id;
        } else {
            user_hash_value = MAC_CALCULATE_HASH_VALUE(puc_sta_mac_addr);

            oal_dlist_search_for_each(pst_entry, &(pst_vap->ast_user_hash[user_hash_value]))
            {
                pst_mac_user = (mac_user_stru *)oal_dlist_get_entry(pst_entry, mac_user_stru, st_user_hash_dlist);
                /*lint -save -e774 */
                if (pst_mac_user == NULL) {
                    oam_error_log0(pst_vap->uc_vap_id, OAM_SF_ANY,
                                   "{mac_vap_find_user_by_macaddr::pst_mac_user null.}");
                    continue;
                }
                /*lint -restore */
                /* 相同的MAC地址 */
                if (!oal_compare_mac_addr(pst_mac_user->auc_user_mac_addr, puc_sta_mac_addr)) {
                    *pus_user_idx = pst_mac_user->us_assoc_id;
                    /* 更新cache user */
                    oal_set_mac_addr(pst_vap->auc_cache_user_mac_addr, pst_mac_user->auc_user_mac_addr);
                    pst_vap->us_cache_user_id = pst_mac_user->us_assoc_id;
                }
            }
        }
        oal_spin_unlock_bh(&pst_vap->st_cache_user_lock);
    }

    /*
     * user id有效的话，返回SUCC给调用者处理，user id无效的话，
     * 返回user id为MAC_INVALID_USER_ID，并返回查找结果FAIL给调用者处理
     */
    if (*pus_user_idx == g_wlan_spec_cfg->invalid_user_id) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


uint32_t mac_device_find_user_by_macaddr(uint8_t uc_device_id, const unsigned char *puc_sta_mac_addr,
    uint16_t *pus_user_idx)
{
    mac_device_stru            *pst_device = NULL;
    mac_vap_stru               *pst_mac_vap = NULL;
    uint8_t                   uc_vap_idx;
    uint32_t                  ret;

    /* 获取device */
    pst_device = mac_res_get_dev(uc_device_id);
    if (pst_device == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "mac_device_find_user_by_macaddr:get_dev return null ");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 对device下的所有vap进行遍历 */
    for (uc_vap_idx = 0; uc_vap_idx < pst_device->uc_vap_num; uc_vap_idx++) {
        /* 配置vap不需要处理 */
        if (pst_device->auc_vap_id[uc_vap_idx] == pst_device->uc_cfg_vap_id) {
            continue;
        }

        pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_device->auc_vap_id[uc_vap_idx]);
        if (pst_mac_vap == NULL) {
            continue;
        }

        /* 只处理AP模式 */
        if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
            continue;
        }

        ret = mac_vap_find_user_by_macaddr(pst_mac_vap, puc_sta_mac_addr, pus_user_idx);
        if (ret == OAL_SUCC) {
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}


uint32_t mac_chip_find_user_by_macaddr(uint8_t uc_chip_id, const unsigned char *puc_sta_mac_addr,
    uint16_t *pus_user_idx)
{
    mac_chip_stru              *pst_mac_chip;
    uint8_t                   uc_device_idx;
    uint32_t                  ret;

    /* 获取device */
    pst_mac_chip = mac_res_get_mac_chip(uc_chip_id);
    if (pst_mac_chip == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_chip_find_user_by_macaddr:get_chip return nul!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (uc_device_idx = 0; uc_device_idx < pst_mac_chip->uc_device_nums; uc_device_idx++) {
        ret = mac_device_find_user_by_macaddr(pst_mac_chip->auc_device_id[uc_device_idx],
            puc_sta_mac_addr, pus_user_idx);
        if (ret == OAL_SUCC) {
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}

uint32_t mac_board_find_user_by_macaddr(const unsigned char *puc_sta_mac_addr, uint16_t *pus_user_idx)
{
    uint8_t  uc_chip_idx;
    uint32_t ret;

    /* 遍历board下所有chip */
    for (uc_chip_idx = 0; uc_chip_idx < WLAN_CHIP_MAX_NUM_PER_BOARD; uc_chip_idx++) {
        ret = mac_chip_find_user_by_macaddr(uc_chip_idx, puc_sta_mac_addr, pus_user_idx);
        if (ret == OAL_SUCC) {
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}


uint32_t mac_vap_init_wme_param(mac_vap_stru *pst_mac_vap)
{
    const mac_wme_param_stru   *pst_wmm_param = NULL;
    const mac_wme_param_stru   *pst_wmm_param_sta = NULL;
    uint8_t                       uc_ac_type;

    pst_wmm_param = mac_get_wmm_cfg(pst_mac_vap->en_vap_mode);
    if (pst_wmm_param == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (uc_ac_type = 0; uc_ac_type < WLAN_WME_AC_BUTT; uc_ac_type++) {
        /* VAP自身的EDCA参数 */
        mac_mib_set_QAPEDCATableIndex(pst_mac_vap, uc_ac_type, uc_ac_type + 1); /* 注: 协议规定取值1 2 3 4 */
        mac_mib_set_QAPEDCATableAIFSN(pst_mac_vap, uc_ac_type, pst_wmm_param[uc_ac_type].aifsn);
        mac_mib_set_QAPEDCATableCWmin(pst_mac_vap, uc_ac_type, pst_wmm_param[uc_ac_type].logcwmin);
        mac_mib_set_QAPEDCATableCWmax(pst_mac_vap, uc_ac_type, pst_wmm_param[uc_ac_type].logcwmax);
        mac_mib_set_QAPEDCATableTXOPLimit(pst_mac_vap, uc_ac_type, pst_wmm_param[uc_ac_type].txop_limit);
    }

    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* AP模式时广播给STA的EDCA参数，只在AP模式需要初始化此值，使用WLAN_VAP_MODE_BUTT， */
        pst_wmm_param_sta = mac_get_wmm_cfg(WLAN_VAP_MODE_BUTT);

        for (uc_ac_type = 0; uc_ac_type < WLAN_WME_AC_BUTT; uc_ac_type++) {
            mac_mib_set_EDCATableIndex(pst_mac_vap, uc_ac_type, uc_ac_type + 1); /* 注: 协议规定取值1 2 3 4 */
            mac_mib_set_EDCATableAIFSN(pst_mac_vap, uc_ac_type, pst_wmm_param_sta[uc_ac_type].aifsn);
            mac_mib_set_EDCATableCWmin(pst_mac_vap, uc_ac_type, pst_wmm_param_sta[uc_ac_type].logcwmin);
            mac_mib_set_EDCATableCWmax(pst_mac_vap, uc_ac_type, pst_wmm_param_sta[uc_ac_type].logcwmax);
            mac_mib_set_EDCATableTXOPLimit(pst_mac_vap, uc_ac_type, pst_wmm_param_sta[uc_ac_type].txop_limit);
        }
    }

    return OAL_SUCC;
}



void mac_vap_init_11ac_mcs_singlenss(mac_vap_stru *pst_mac_vap,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    mac_tx_max_mcs_map_stru         *pst_tx_max_mcs_map;
    mac_rx_max_mcs_map_stru         *pst_rx_max_mcs_map;

    /* 获取mib值指针 */
    pst_rx_max_mcs_map = (mac_rx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_rx_mcs_map(pst_mac_vap));
    pst_tx_max_mcs_map = (mac_tx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_tx_mcs_map(pst_mac_vap));

    /* 20MHz带宽的情况下，支持MCS0-MCS8 */
    if (en_bandwidth == WLAN_BAND_WIDTH_20M) {
#ifdef _PRE_WLAN_FEATURE_11AC_20M_MCS9
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
#else
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
#endif
        mac_mib_set_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_20M_11AC);
        mac_mib_set_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_20M_11AC);
    } else if ((en_bandwidth == WLAN_BAND_WIDTH_40MINUS) || (en_bandwidth == WLAN_BAND_WIDTH_40PLUS)) {
        /* 40MHz带宽的情况下，支持MCS0-MCS9 */
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_40M_11AC);
        mac_mib_set_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_40M_11AC);
    } else if ((en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) && (en_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS)) {
        /* 80MHz带宽的情况下，支持MCS0-MCS9 */
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_80M_11AC);
        mac_mib_set_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_80M_11AC);
#ifdef _PRE_WLAN_FEATURE_160M
    } else if ((en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS) &&
        (en_bandwidth <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS)) {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_160M_11AC);
        mac_mib_set_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_160M_11AC);
#endif
    }
}


void mac_vap_init_11ac_mcs_doublenss(mac_vap_stru *pst_mac_vap,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    mac_tx_max_mcs_map_stru *pst_tx_max_mcs_map;
    mac_rx_max_mcs_map_stru *pst_rx_max_mcs_map;

    /* 获取mib值指针 */
    pst_rx_max_mcs_map = (mac_rx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_rx_mcs_map(pst_mac_vap));
    pst_tx_max_mcs_map = (mac_tx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_tx_mcs_map(pst_mac_vap));

    /* 20MHz带宽的情况下，支持MCS0-MCS8 */
    if (en_bandwidth == WLAN_BAND_WIDTH_20M) {
#ifdef _PRE_WLAN_FEATURE_11AC_20M_MCS9
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_rx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
#else
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
        pst_rx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
#endif
        mac_mib_set_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_20M_11AC);
        mac_mib_set_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_20M_11AC);
    } else if ((en_bandwidth == WLAN_BAND_WIDTH_40MINUS) || (en_bandwidth == WLAN_BAND_WIDTH_40PLUS)) {
        /* 40MHz带宽的情况下，支持MCS0-MCS9 */
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_rx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_40M_11AC);
        mac_mib_set_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_40M_11AC);
    } else if ((en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) && (en_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS)) {
        /* 80MHz带宽的情况下，支持MCS0-MCS9 */
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_rx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_80M_11AC);
        mac_mib_set_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_80M_11AC);
#ifdef _PRE_WLAN_FEATURE_160M
    } else if ((en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS) &&
        (en_bandwidth <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS)) {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_rx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_160M_11AC);
        mac_mib_set_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_160M_11AC);
#endif
    }
}

typedef enum {
    MAC_VAP_LEGACY_RATE_1M  = 1,
    MAC_VAP_LEGACY_RATE_2M  = 2,
    MAC_VAP_LEGACY_RATE_5M  = 5,
    MAC_VAP_LEGACY_RATE_6M  = 6,
    MAC_VAP_LEGACY_RATE_9M  = 9,
    MAC_VAP_LEGACY_RATE_11M = 11,
    MAC_VAP_LEGACY_RATE_12M = 12,
    MAC_VAP_LEGACY_RATE_18M = 18,
    MAC_VAP_LEGACY_RATE_24M = 24,
    MAC_VAP_LEGACY_RATE_36M = 36,
    MAC_VAP_LEGACY_RATE_48M = 48,
    MAC_VAP_LEGACY_RATE_54M = 54,
} mac_vap_legacy_rate_enum;


void mac_vap_init_legacy_rates(mac_vap_stru *pst_vap, mac_data_rate_stru *pst_rates)
{
    uint8_t                      uc_rate_index;
    uint8_t                      uc_curr_rate_index = 0;
    mac_data_rate_stru            *puc_orig_rate = NULL;
    mac_data_rate_stru            *puc_curr_rate = NULL;
    uint8_t                      uc_rates_num;
    int32_t                      l_ret = EOK;

    /* 初始化速率集 */
    uc_rates_num = MAC_DATARATES_PHY_80211G_NUM;

    /* 初始化速率个数，基本速率个数，非基本速率个数 */
    pst_vap->st_curr_sup_rates.st_rate.uc_rs_nrates = MAC_NUM_DR_802_11A;
    pst_vap->st_curr_sup_rates.uc_br_rate_num       = MAC_NUM_BR_802_11A;
    pst_vap->st_curr_sup_rates.uc_nbr_rate_num      = MAC_NUM_NBR_802_11A;
    pst_vap->st_curr_sup_rates.uc_min_rate          = MAC_VAP_LEGACY_RATE_6M;
    pst_vap->st_curr_sup_rates.uc_max_rate          = MAC_VAP_LEGACY_RATE_24M;

    /* 将速率拷贝到VAP结构体下的速率集中 */
    for (uc_rate_index = 0; uc_rate_index < uc_rates_num; uc_rate_index++) {
        puc_orig_rate = &pst_rates[uc_rate_index];
        puc_curr_rate = &(pst_vap->st_curr_sup_rates.st_rate.ast_rs_rates[uc_curr_rate_index]);

        /* Basic Rates */
        if ((puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_6M) ||
            (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_12M) ||
            (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_24M)) {
            l_ret = memcpy_s(puc_curr_rate, sizeof(mac_data_rate_stru), puc_orig_rate, sizeof(mac_data_rate_stru));
            puc_curr_rate->uc_mac_rate |= 0x80;
            uc_curr_rate_index++;
        } else if ((puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_9M) ||
            (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_18M) ||
            (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_36M) ||
            (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_48M) ||
            (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_54M)) {
            /* Non-basic rates */
            l_ret = memcpy_s(puc_curr_rate, sizeof(mac_data_rate_stru), puc_orig_rate, sizeof(mac_data_rate_stru));
            uc_curr_rate_index++;
        }
        if (l_ret != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "mac_vap_init_legacy_rates::memcpy fail!");
            return;
        }

        if (uc_curr_rate_index == pst_vap->st_curr_sup_rates.st_rate.uc_rs_nrates) {
            break;
        }
    }
}


void mac_vap_init_11b_rates(mac_vap_stru *pst_vap, mac_data_rate_stru *pst_rates)
{
    uint8_t                      uc_rate_index;
    uint8_t                      uc_curr_rate_index = 0;
    mac_data_rate_stru            *puc_orig_rate = NULL;
    mac_data_rate_stru            *puc_curr_rate = NULL;
    uint8_t                      uc_rates_num;
    int32_t                      l_ret = EOK;

    /* 初始化速率集 */
    uc_rates_num = MAC_DATARATES_PHY_80211G_NUM;

    /* 初始化速率个数，基本速率个数，非基本速率个数 */
    pst_vap->st_curr_sup_rates.st_rate.uc_rs_nrates = MAC_NUM_DR_802_11B;
    pst_vap->st_curr_sup_rates.uc_br_rate_num       = 0;
    pst_vap->st_curr_sup_rates.uc_nbr_rate_num      = MAC_NUM_NBR_802_11B;
    pst_vap->st_curr_sup_rates.uc_min_rate          = MAC_VAP_LEGACY_RATE_1M;
    pst_vap->st_curr_sup_rates.uc_max_rate          = MAC_VAP_LEGACY_RATE_2M;

    /* 将速率拷贝到VAP结构体下的速率集中 */
    for (uc_rate_index = 0; uc_rate_index < uc_rates_num; uc_rate_index++) {
        puc_orig_rate = &pst_rates[uc_rate_index];
        puc_curr_rate = &(pst_vap->st_curr_sup_rates.st_rate.ast_rs_rates[uc_curr_rate_index]);

        /* Basic Rates */
        if ((puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_1M) || (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_2M) ||
            ((pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) && ((puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_5M) ||
            (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_11M)))) {
            pst_vap->st_curr_sup_rates.uc_br_rate_num++;
            l_ret = memcpy_s(puc_curr_rate, sizeof(mac_data_rate_stru), puc_orig_rate, sizeof(mac_data_rate_stru));
            puc_curr_rate->uc_mac_rate |= 0x80;
            uc_curr_rate_index++;
        } else if ((pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
            ((puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_5M) ||
            (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_11M))) {
            /* Non-basic rates */
            l_ret = memcpy_s(puc_curr_rate, sizeof(mac_data_rate_stru), puc_orig_rate, sizeof(mac_data_rate_stru));
            uc_curr_rate_index++;
        } else {
            continue;
        }

        if (l_ret != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "mac_vap_init_11b_rates::memcpy fail!");
            return;
        }

        if (uc_curr_rate_index == pst_vap->st_curr_sup_rates.st_rate.uc_rs_nrates) {
            break;
        }
    }
}


void mac_vap_init_11g_mixed_one_rates(mac_vap_stru *pst_vap, mac_data_rate_stru *pst_rates)
{
    uint8_t                      uc_rate_index;
    mac_data_rate_stru            *puc_orig_rate = NULL;
    mac_data_rate_stru            *puc_curr_rate = NULL;
    uint8_t                      uc_rates_num;
    int32_t                      l_ret;

    /* 初始化速率集 */
    uc_rates_num = MAC_DATARATES_PHY_80211G_NUM;

    /* 初始化速率个数，基本速率个数，非基本速率个数 */
    pst_vap->st_curr_sup_rates.st_rate.uc_rs_nrates = MAC_NUM_DR_802_11G_MIXED;
    pst_vap->st_curr_sup_rates.uc_br_rate_num       = MAC_NUM_BR_802_11G_MIXED_ONE;
    pst_vap->st_curr_sup_rates.uc_nbr_rate_num      = MAC_NUM_NBR_802_11G_MIXED_ONE;
    pst_vap->st_curr_sup_rates.uc_min_rate          = MAC_VAP_LEGACY_RATE_1M;
    pst_vap->st_curr_sup_rates.uc_max_rate          = MAC_VAP_LEGACY_RATE_11M;

    /* 将速率拷贝到VAP结构体下的速率集中 */
    for (uc_rate_index = 0; uc_rate_index < uc_rates_num; uc_rate_index++) {
        puc_orig_rate = &pst_rates[uc_rate_index];
        puc_curr_rate = &(pst_vap->st_curr_sup_rates.st_rate.ast_rs_rates[uc_rate_index]);

        l_ret = memcpy_s(puc_curr_rate, sizeof(mac_data_rate_stru), puc_orig_rate, sizeof(mac_data_rate_stru));
        if (l_ret != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "mac_vap_init_11g_mixed_one_rates::memcpy fail!");
            return;
        }

        /* Basic Rates */
        if ((puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_1M) ||
            (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_2M) ||
            (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_5M) ||
            (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_11M)) {
            puc_curr_rate->uc_mac_rate |= 0x80;
        }
    }
}


void mac_vap_init_11g_mixed_two_rates(mac_vap_stru *pst_vap, mac_data_rate_stru *pst_rates)
{
    uint8_t                      uc_rate_index;
    mac_data_rate_stru            *puc_orig_rate = NULL;
    mac_data_rate_stru            *puc_curr_rate = NULL;
    uint8_t                      uc_rates_num;
    int32_t                      l_ret;

    /* 初始化速率集 */
    uc_rates_num = MAC_DATARATES_PHY_80211G_NUM;

    /* 初始化速率个数，基本速率个数，非基本速率个数 */
    pst_vap->st_curr_sup_rates.st_rate.uc_rs_nrates = MAC_NUM_DR_802_11G_MIXED;
    pst_vap->st_curr_sup_rates.uc_br_rate_num       = MAC_NUM_BR_802_11G_MIXED_TWO;
    pst_vap->st_curr_sup_rates.uc_nbr_rate_num      = MAC_NUM_NBR_802_11G_MIXED_TWO;
    pst_vap->st_curr_sup_rates.uc_min_rate          = MAC_VAP_LEGACY_RATE_1M;
    pst_vap->st_curr_sup_rates.uc_max_rate          = MAC_VAP_LEGACY_RATE_24M;

    /* 将速率拷贝到VAP结构体下的速率集中 */
    for (uc_rate_index = 0; uc_rate_index < uc_rates_num; uc_rate_index++) {
        puc_orig_rate = &pst_rates[uc_rate_index];
        puc_curr_rate = &(pst_vap->st_curr_sup_rates.st_rate.ast_rs_rates[uc_rate_index]);

        l_ret = memcpy_s(puc_curr_rate, sizeof(mac_data_rate_stru), puc_orig_rate, sizeof(mac_data_rate_stru));
        if (l_ret != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "mac_vap_init_11g_mixed_two_rates::memcpy fail!");
            return;
        }

        /* Basic Rates */
        if ((puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_1M) || (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_2M) ||
            (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_5M) || (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_11M) ||
            (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_6M) || (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_12M) ||
            (puc_orig_rate->uc_mbps == MAC_VAP_LEGACY_RATE_24M)) {
            puc_curr_rate->uc_mac_rate |= 0x80;
        }
    }
}
uint8_t g_mcm_mask_custom = 0;


void mac_vap_ini_get_nss_num(mac_device_stru *mac_device,
    wlan_nss_enum_uint8 *nss_num_rx, wlan_nss_enum_uint8 *nss_num_tx)
{
    wlan_nss_enum_uint8 dev_nss_num;
    dev_nss_num = MAC_DEVICE_GET_NSS_NUM(mac_device);
    /* FPGA阶段四天线支持四发双收，可根据定制化控制tx rx的空间流支持四流or双流 */
    /* mcm_mask_custom定制化bit4置1表示4天线时只支持双发，置0表示默认支持四发 */
    /* mcm_mask_custom定制化bit5置1表示4天线时只支持双收，置0表示默认支持四发 */
    if (dev_nss_num == WLAN_FOUR_NSS) {
        *nss_num_tx = ((g_mcm_mask_custom & BIT4) == 0 ? dev_nss_num : dev_nss_num >> 1);
        *nss_num_rx = ((g_mcm_mask_custom & BIT5) == 0 ? dev_nss_num : dev_nss_num >> 1);
    } else {
        *nss_num_tx = dev_nss_num;
        *nss_num_rx = dev_nss_num;
    }
}


OAL_STATIC void mac_vap_init_11n_40m_rates(mac_vap_stru *mac_vap, wlan_nss_enum_uint8 nss_num)
{
    /* 40M 支持MCS32 */
    if ((mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS) ||
        (mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS)) {
        mac_mib_set_SupportedMCSRxValue(mac_vap, 4, 0x01); /* 4 mcsindex, 支持 RX MCS 32,最后一位为1 */
        mac_mib_set_SupportedMCSTxValue(mac_vap, 4, 0x01); /* 4 mcsindex, 支持 TX MCS 32,最后一位为1 */
        switch (nss_num) {
            case WLAN_SINGLE_NSS:
                mac_mib_set_HighestSupportedDataRate(mac_vap, MAC_MAX_RATE_SINGLE_NSS_40M_11N);
                break;
            case WLAN_DOUBLE_NSS:
                mac_mib_set_HighestSupportedDataRate(mac_vap, MAC_MAX_RATE_DOUBLE_NSS_40M_11N);
                break;
            case WLAN_TRIPLE_NSS:
                mac_mib_set_HighestSupportedDataRate(mac_vap, MAC_MAX_RATE_TRIPLE_NSS_40M_11N);
                break;
            case WLAN_FOUR_NSS:
                mac_mib_set_HighestSupportedDataRate(mac_vap, MAC_MAX_RATE_FOUR_NSS_40M_11N);
                break;
            default:
                break;
        }
    }
}


void mac_vap_init_11n_rates(mac_vap_stru *mac_vap, mac_device_stru *mac_dev)
{
    wlan_nss_enum_uint8 nss_num_tx = 0;
    wlan_nss_enum_uint8 nss_num_rx = 0;
    uint8_t nss_idx;
    uint32_t high_rate[] = {
        (uint32_t)MAC_MAX_RATE_SINGLE_NSS_20M_11N,
        (uint32_t)MAC_MAX_RATE_DOUBLE_NSS_20M_11N,
        (uint32_t)MAC_MAX_RATE_TRIPLE_NSS_20M_11N,
        (uint32_t)MAC_MAX_RATE_FOUR_NSS_20M_11N,
    };
    mac_vap_ini_get_nss_num(mac_dev, &nss_num_rx, &nss_num_tx);

    /* MCS相关MIB值初始化 */
    mac_mib_set_TxMCSSetDefined(mac_vap, OAL_TRUE);
    mac_mib_set_TxUnequalModulationSupported(mac_vap, OAL_FALSE);
    nss_num_rx == nss_num_tx ? mac_mib_set_TxRxMCSSetNotEqual(mac_vap, OAL_FALSE) :
        mac_mib_set_TxRxMCSSetNotEqual(mac_vap, OAL_TRUE);

    /* 将MIB值的MCS MAP清零 */
    memset_s(mac_mib_get_SupportedMCSTx(mac_vap), WLAN_HT_MCS_BITMASK_LEN, 0, WLAN_HT_MCS_BITMASK_LEN);
    memset_s(mac_mib_get_SupportedMCSRx(mac_vap), WLAN_HT_MCS_BITMASK_LEN, 0, WLAN_HT_MCS_BITMASK_LEN);

    mac_mib_set_TxMaximumNumberSpatialStreamsSupported(mac_vap, nss_num_tx);

    /* 设置TX MCS */
    for (nss_idx = 0; nss_idx <= nss_num_tx; nss_idx++) {
        mac_mib_set_SupportedMCSTxValue(mac_vap, nss_idx, 0xFF);
    }

    /* 设置RX MCS */
    for (nss_idx = 0; nss_idx <= nss_num_rx; nss_idx++) {
        mac_mib_set_SupportedMCSRxValue(mac_vap, nss_idx, 0xFF);
    }
    nss_num_tx > nss_num_rx ? mac_mib_set_HighestSupportedDataRate(mac_vap, high_rate[nss_num_tx]) :
        mac_mib_set_HighestSupportedDataRate(mac_vap, high_rate[nss_num_rx]);

    mac_vap_init_11n_40m_rates(mac_vap, oal_max(nss_num_tx, nss_num_rx));
    mac_vap_init_11n_rates_extend(mac_vap, mac_dev);
}

OAL_STATIC void mac_vap_init_11ac_highest_rate(mac_vap_stru *mac_vap,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth, wlan_nss_enum_uint8 nss_num_rx, wlan_nss_enum_uint8 nss_num_tx)
{
    uint32_t high_rate_20m[] = {
        (uint32_t)MAC_MAX_RATE_SINGLE_NSS_20M_11AC,
        (uint32_t)MAC_MAX_RATE_DOUBLE_NSS_20M_11AC,
        (uint32_t)MAC_MAX_RATE_TRIPLE_NSS_20M_11AC,
        (uint32_t)MAC_MAX_RATE_FOUR_NSS_20M_11AC,
    };
    uint32_t high_rate_40m[] = {
        (uint32_t)MAC_MAX_RATE_SINGLE_NSS_40M_11AC,
        (uint32_t)MAC_MAX_RATE_DOUBLE_NSS_40M_11AC,
        (uint32_t)MAC_MAX_RATE_TRIPLE_NSS_40M_11AC,
        (uint32_t)MAC_MAX_RATE_FOUR_NSS_40M_11AC,
    };
    uint32_t high_rate_80m[] = {
        (uint32_t)MAC_MAX_RATE_SINGLE_NSS_80M_11AC,
        (uint32_t)MAC_MAX_RATE_DOUBLE_NSS_80M_11AC,
        (uint32_t)MAC_MAX_RATE_TRIPLE_NSS_80M_11AC,
        (uint32_t)MAC_MAX_RATE_FOUR_NSS_80M_11AC,
    };
    uint32_t high_rate_160m[] = {
        (uint32_t)MAC_MAX_RATE_SINGLE_NSS_160M_11AC,
        (uint32_t)MAC_MAX_RATE_DOUBLE_NSS_160M_11AC,
        (uint32_t)MAC_MAX_RATE_TRIPLE_NSS_160M_11AC,
        (uint32_t)MAC_MAX_RATE_FOUR_NSS_160M_11AC,
    };

    if (en_bandwidth == WLAN_BAND_WIDTH_20M) {
        mac_mib_set_us_rx_highest_rate(mac_vap, high_rate_20m[nss_num_rx]);
        mac_mib_set_us_tx_highest_rate(mac_vap, high_rate_20m[nss_num_tx]);
    } else if ((en_bandwidth == WLAN_BAND_WIDTH_40MINUS) || (en_bandwidth == WLAN_BAND_WIDTH_40PLUS)) {
        mac_mib_set_us_rx_highest_rate(mac_vap, high_rate_40m[nss_num_rx]);
        mac_mib_set_us_tx_highest_rate(mac_vap, high_rate_40m[nss_num_tx]);
    } else if ((en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) && (en_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS)) {
        mac_mib_set_us_rx_highest_rate(mac_vap, high_rate_80m[nss_num_rx]);
        mac_mib_set_us_tx_highest_rate(mac_vap, high_rate_80m[nss_num_tx]);
#ifdef _PRE_WLAN_FEATURE_160M
    } else if ((en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS) &&
        (en_bandwidth <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS)) {
        mac_mib_set_us_rx_highest_rate(mac_vap, high_rate_160m[nss_num_rx]);
        mac_mib_set_us_tx_highest_rate(mac_vap, high_rate_160m[nss_num_tx]);
#endif
    }
}


OAL_STATIC void mac_set_tx_rx_vht_mcs(mac_max_mcs_map_stru *tx_rx_vht_mcs, uint8_t nss_num, uint8_t support_nss)
{
    tx_rx_vht_mcs->us_max_mcs_1ss = support_nss;
    tx_rx_vht_mcs->us_max_mcs_2ss = (nss_num >= WLAN_DOUBLE_NSS ? support_nss : MAC_MAX_UNSUP_MCS_11AC_EACH_NSS);
    tx_rx_vht_mcs->us_max_mcs_3ss = (nss_num >= WLAN_TRIPLE_NSS ? support_nss : MAC_MAX_UNSUP_MCS_11AC_EACH_NSS);
    tx_rx_vht_mcs->us_max_mcs_4ss = (nss_num >= WLAN_FOUR_NSS ? support_nss : MAC_MAX_UNSUP_MCS_11AC_EACH_NSS);
}


void mac_vap_init_11ac_rates(mac_vap_stru *mac_vap, mac_device_stru *mac_dev)
{
    wlan_nss_enum_uint8 nss_num_tx = 0;
    wlan_nss_enum_uint8 nss_num_rx = 0;
    mac_tx_max_mcs_map_stru *tx_max_mcs_map = NULL;
    mac_rx_max_mcs_map_stru *rx_max_mcs_map = NULL;

    /* 先将TX RX MCSMAP初始化为所有空间流都不支持 0xFFFF */
    mac_mib_set_vht_rx_mcs_map(mac_vap, 0xFFFF);
    mac_mib_set_vht_tx_mcs_map(mac_vap, 0xFFFF);

    mac_vap_ini_get_nss_num(mac_dev, &nss_num_rx, &nss_num_tx);

    /* 获取mib值指针 */
    rx_max_mcs_map = (mac_rx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_rx_mcs_map(mac_vap));
    tx_max_mcs_map = (mac_tx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_tx_mcs_map(mac_vap));

    /* 填rx_mcs能力 */
    mac_set_tx_rx_vht_mcs((mac_max_mcs_map_stru *)rx_max_mcs_map, nss_num_rx, (uint8_t)MAC_MAX_SUP_MCS9_11AC_EACH_NSS);
    /* 填tx_mcs能力 */
    mac_set_tx_rx_vht_mcs((mac_max_mcs_map_stru *)tx_max_mcs_map, nss_num_tx, (uint8_t)MAC_MAX_SUP_MCS9_11AC_EACH_NSS);

    /* 20MHz带宽的情况下，支持MCS0-MCS8 */
    if (mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_20M) {
#ifndef _PRE_WLAN_FEATURE_11AC_20M_MCS9
        /* 填rx_mcs能力 */
        mac_set_tx_rx_vht_mcs((mac_max_mcs_map_stru *)rx_max_mcs_map, nss_num_rx,
            (uint8_t)MAC_MAX_SUP_MCS8_11AC_EACH_NSS);
        /* 填tx_mcs能力 */
        mac_set_tx_rx_vht_mcs((mac_max_mcs_map_stru *)tx_max_mcs_map, nss_num_tx,
            (uint8_t)MAC_MAX_SUP_MCS8_11AC_EACH_NSS);
#endif
    }
    mac_vap_init_11ac_highest_rate(mac_vap, mac_vap->st_channel.en_bandwidth, nss_num_rx, nss_num_tx);
#ifdef _PRE_WLAN_FEATURE_M2S
    /* m2s spec模式下，需要根据vap nss能力 + cap是否支持siso，刷新空间流 */
    if (MAC_VAP_SPEC_IS_SW_NEED_M2S_SWITCH(mac_vap) && IS_VAP_SINGLE_NSS(mac_vap)) {
        /* 先将TX RX MCSMAP初始化为所有空间流都不支持 0xFFFF */
        mac_mib_set_vht_rx_mcs_map(mac_vap, 0xFFFF);
        mac_mib_set_vht_tx_mcs_map(mac_vap, 0xFFFF);

        /* 1个空间流的情况 */
        mac_vap_init_11ac_mcs_singlenss(mac_vap, mac_vap->st_channel.en_bandwidth);

        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_init_11ac_rates::m2s spec update rate to siso.}");
    }
#endif
}


void mac_vap_init_p2p_rates(mac_vap_stru *pst_vap, wlan_protocol_enum_uint8 en_vap_protocol,
    mac_data_rate_stru *pst_rates)
{
    mac_device_stru *pst_mac_dev;
    int32_t        l_ret;

    pst_mac_dev = mac_res_get_dev(pst_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_error_log1(pst_vap->uc_vap_id, OAM_SF_ANY,
                       "{mac_vap_init_p2p_rates::pst_mac_dev[%d] null.}",
                       pst_vap->uc_device_id);

        return;
    }

    mac_vap_init_legacy_rates(pst_vap, pst_rates);

    l_ret = memcpy_s(&pst_vap->ast_sta_sup_rates_ie[WLAN_BAND_5G],
                     sizeof(mac_curr_rateset_stru),
                     &pst_vap->st_curr_sup_rates,
                     sizeof(pst_vap->st_curr_sup_rates));
    l_ret += memcpy_s(&pst_vap->ast_sta_sup_rates_ie[WLAN_BAND_2G],
                      sizeof(mac_curr_rateset_stru),
                      &pst_vap->st_curr_sup_rates,
                      sizeof(pst_vap->st_curr_sup_rates));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_vap_init_p2p_rates::memcpy fail!");
        return;
    }
    mac_vap_init_11n_rates(pst_vap, pst_mac_dev);
    if (en_vap_protocol == WLAN_VHT_MODE) {
        mac_vap_init_11ac_rates(pst_vap, pst_mac_dev);
    }
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        if (en_vap_protocol == WLAN_HE_MODE) {
            mac_vap_init_11ac_rates(pst_vap, pst_mac_dev);
            mac_vap_init_11ax_rates(pst_vap, pst_mac_dev);
        }
    }
#endif
}

#ifdef _PRE_WLAN_FEATURE_11AX
void mac_vap_init_rates_in_he_protocol(mac_vap_stru *vap, mac_device_stru *mac_dev, mac_data_rate_stru *rates)
{
    if (vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* 用于STA全信道扫描 5G时 填写支持速率集ie */
        mac_vap_init_legacy_rates(vap, rates);
        memcpy_s(&vap->ast_sta_sup_rates_ie[WLAN_BAND_5G], sizeof(mac_curr_rateset_stru),
                 &vap->st_curr_sup_rates, sizeof(vap->st_curr_sup_rates));

        /* 用于STA全信道扫描 2G时 填写支持速率集ie */
        mac_vap_init_11g_mixed_one_rates(vap, rates);
        memcpy_s(&vap->ast_sta_sup_rates_ie[WLAN_BAND_2G], sizeof(mac_curr_rateset_stru),
                 &vap->st_curr_sup_rates, sizeof(vap->st_curr_sup_rates));
        // todo 以下三个语句以及feature_11ax_is_open重复，待进一步整改
        mac_vap_init_11n_rates(vap, mac_dev);
        mac_vap_init_11ac_rates(vap, mac_dev);
        mac_vap_init_11ax_rates(vap, mac_dev);
    } else if (vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        if (vap->st_channel.en_band == WLAN_BAND_2G) {
            mac_vap_init_11g_mixed_one_rates(vap, rates);
        } else {
            mac_vap_init_legacy_rates(vap, rates);
        }
        mac_vap_init_11n_rates(vap, mac_dev);
        mac_vap_init_11ac_rates(vap, mac_dev);
        mac_vap_init_11ax_rates(vap, mac_dev);
    }
}
#endif

void mac_vap_init_rates_by_protocol(mac_vap_stru *pst_vap,
    wlan_protocol_enum_uint8 en_vap_protocol, mac_data_rate_stru *pst_rates)
{
    mac_device_stru *pst_mac_dev = (mac_device_stru *)mac_res_get_dev(pst_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        return;
    }

    /* STA模式默认协议模式是11ac，初始化速率集为所有速率集 */
    if (!IS_LEGACY_VAP(pst_vap)) {
        mac_vap_init_p2p_rates(pst_vap, en_vap_protocol, pst_rates);
        return;
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open && en_vap_protocol == WLAN_HE_MODE) {
        mac_vap_init_rates_in_he_protocol(pst_vap, pst_mac_dev, pst_rates);
    }
#endif
    if (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA && en_vap_protocol == WLAN_VHT_MODE) {
        /* 用于STA全信道扫描 5G时 填写支持速率集ie */
        mac_vap_init_legacy_rates(pst_vap, pst_rates);
        memcpy_s(&pst_vap->ast_sta_sup_rates_ie[WLAN_BAND_5G], sizeof(mac_curr_rateset_stru),
                 &pst_vap->st_curr_sup_rates, sizeof(pst_vap->st_curr_sup_rates));

        /* 用于STA全信道扫描 2G时 填写支持速率集ie */
        mac_vap_init_11g_mixed_one_rates(pst_vap, pst_rates);
        memcpy_s(&pst_vap->ast_sta_sup_rates_ie[WLAN_BAND_2G], sizeof(mac_curr_rateset_stru),
                 &pst_vap->st_curr_sup_rates, sizeof(pst_vap->st_curr_sup_rates));

        mac_vap_init_11n_rates(pst_vap, pst_mac_dev);
        mac_vap_init_11ac_rates(pst_vap, pst_mac_dev);
    } else if (oal_value_eq_any2(en_vap_protocol, WLAN_VHT_ONLY_MODE, WLAN_VHT_MODE)) {
#ifdef _PRE_WLAN_FEATURE_11AC2G
        (pst_vap->st_channel.en_band == WLAN_BAND_2G) ? mac_vap_init_11g_mixed_one_rates(pst_vap, pst_rates) :
            mac_vap_init_legacy_rates(pst_vap, pst_rates);
#else
        mac_vap_init_legacy_rates(pst_vap, pst_rates);
#endif
        mac_vap_init_11n_rates(pst_vap, pst_mac_dev);
        mac_vap_init_11ac_rates(pst_vap, pst_mac_dev);
    } else if (oal_value_eq_any2(en_vap_protocol, WLAN_HT_ONLY_MODE, WLAN_HT_MODE)) {
        if (pst_vap->st_channel.en_band == WLAN_BAND_5G) {
            mac_vap_init_legacy_rates(pst_vap, pst_rates);
        } else if (pst_vap->st_channel.en_band == WLAN_BAND_2G) {
            mac_vap_init_11g_mixed_one_rates(pst_vap, pst_rates);
        }

        mac_vap_init_11n_rates(pst_vap, pst_mac_dev);
    } else if (oal_value_eq_any2(en_vap_protocol, WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11G_MODE)) {
        mac_vap_init_legacy_rates(pst_vap, pst_rates);
    } else if (en_vap_protocol == WLAN_LEGACY_11B_MODE) {
        mac_vap_init_11b_rates(pst_vap, pst_rates);
    } else if (en_vap_protocol == WLAN_MIXED_ONE_11G_MODE) {
        mac_vap_init_11g_mixed_one_rates(pst_vap, pst_rates);
    } else if (en_vap_protocol == WLAN_MIXED_TWO_11G_MODE) {
        mac_vap_init_11g_mixed_two_rates(pst_vap, pst_rates);
    }
}


void mac_vap_init_rates(mac_vap_stru *pst_vap)
{
    mac_device_stru               *pst_mac_dev;
    wlan_protocol_enum_uint8       en_vap_protocol;
    mac_data_rate_stru            *pst_rates = NULL;

    pst_mac_dev = mac_res_get_dev(pst_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_error_log1(pst_vap->uc_vap_id, OAM_SF_ANY,
            "{mac_vap_init_rates::pst_mac_dev[%d] null.}", pst_vap->uc_device_id);

        return;
    }

    /* 初始化速率集 */
    pst_rates = mac_device_get_all_rates(pst_mac_dev);

    en_vap_protocol = pst_vap->en_protocol;

    mac_vap_init_rates_by_protocol(pst_vap, en_vap_protocol, pst_rates);
}



void mac_vap_set_tx_power(mac_vap_stru *pst_vap, uint8_t uc_tx_power)
{
    pst_vap->uc_tx_power = uc_tx_power;
}


void mac_vap_set_aid(mac_vap_stru *pst_vap, uint16_t us_aid)
{
    pst_vap->us_sta_aid = us_aid;
}


void mac_vap_set_assoc_id(mac_vap_stru *pst_vap, uint16_t us_assoc_vap_id)
{
    pst_vap->us_assoc_vap_id = us_assoc_vap_id;
}


void mac_vap_set_uapsd_cap(mac_vap_stru *pst_vap, uint8_t uc_uapsd_cap)
{
    pst_vap->uc_uapsd_cap = uc_uapsd_cap;
}


void mac_vap_set_p2p_mode(mac_vap_stru *pst_vap, wlan_p2p_mode_enum_uint8 en_p2p_mode)
{
    pst_vap->en_p2p_mode = en_p2p_mode;
}


void mac_vap_set_multi_user_idx(mac_vap_stru *pst_vap, uint16_t us_multi_user_idx)
{
    pst_vap->us_multi_user_idx = us_multi_user_idx;
}


void mac_vap_set_rx_nss(mac_vap_stru *pst_vap, wlan_nss_enum_uint8 en_rx_nss)
{
    pst_vap->en_vap_rx_nss = en_rx_nss;
}


void mac_vap_set_al_tx_payload_flag(mac_vap_stru *pst_vap, uint8_t uc_paylod)
{
    pst_vap->bit_payload_flag = uc_paylod;
}


void mac_vap_set_al_tx_flag(mac_vap_stru *pst_vap, oal_bool_enum_uint8 en_flag)
{
    pst_vap->bit_al_tx_flag = en_flag;
}


void mac_vap_set_uapsd_para(mac_vap_stru *pst_mac_vap, mac_cfg_uapsd_sta_stru *pst_uapsd_info)
{
    uint8_t uc_ac;

    pst_mac_vap->st_sta_uapsd_cfg.uc_max_sp_len = pst_uapsd_info->uc_max_sp_len;

    for (uc_ac = 0; uc_ac < WLAN_WME_AC_BUTT; uc_ac++) {
        pst_mac_vap->st_sta_uapsd_cfg.uc_delivery_enabled[uc_ac] = pst_uapsd_info->uc_delivery_enabled[uc_ac];
        pst_mac_vap->st_sta_uapsd_cfg.uc_trigger_enabled[uc_ac] = pst_uapsd_info->uc_trigger_enabled[uc_ac];
    }
}


void mac_vap_set_wmm_params_update_count(mac_vap_stru *pst_vap, uint8_t uc_update_count)
{
    pst_vap->uc_wmm_params_update_count = uc_update_count;
}

/* 后续tdls功能可能会打开 */
#ifdef _PRE_DEBUG_CODE

void mac_vap_set_tdls_prohibited(mac_vap_stru *pst_vap, uint8_t uc_value)
{
    pst_vap->st_cap_flag.bit_tdls_prohibited = uc_value;
}


void mac_vap_set_tdls_channel_switch_prohibited(mac_vap_stru *pst_vap, uint8_t uc_value)
{
    pst_vap->st_cap_flag.bit_tdls_channel_switch_prohibited = uc_value;
}
#endif


void mac_vap_set_hide_ssid(mac_vap_stru *pst_vap, uint8_t uc_value)
{
    pst_vap->st_cap_flag.bit_hide_ssid = uc_value;
}


uint8_t mac_vap_get_peer_obss_scan(mac_vap_stru *pst_vap)
{
    return pst_vap->st_cap_flag.bit_peer_obss_scan;
}


void mac_vap_set_peer_obss_scan(mac_vap_stru *pst_vap, uint8_t uc_value)
{
    pst_vap->st_cap_flag.bit_peer_obss_scan = uc_value;
}


wlan_p2p_mode_enum_uint8 mac_get_p2p_mode(mac_vap_stru *pst_vap)
{
    return (pst_vap->en_p2p_mode);
}


void mac_dec_p2p_num(mac_vap_stru *pst_vap)
{
    mac_device_stru *pst_device;

    pst_device = mac_res_get_dev(pst_vap->uc_device_id);
    if (oal_unlikely(pst_device == NULL)) {
        oam_error_log1(pst_vap->uc_vap_id, OAM_SF_ANY,
            "{mac_p2p_dec_num::pst_device[%d] null.}", pst_vap->uc_device_id);
        return;
    }

    if (IS_P2P_DEV(pst_vap)) {
        pst_device->st_p2p_info.uc_p2p_device_num--;
    } else if (IS_P2P_GO(pst_vap) || IS_P2P_CL(pst_vap)) {
        pst_device->st_p2p_info.uc_p2p_goclient_num--;
    }
}

void mac_inc_p2p_num(mac_vap_stru *pst_vap)
{
    mac_device_stru *pst_dev;

    pst_dev = mac_res_get_dev(pst_vap->uc_device_id);
    if (oal_unlikely(pst_dev == NULL)) {
        oam_error_log1(pst_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_inc_p2p_num::pst_dev[%d] null}", pst_vap->uc_device_id);
        return;
    }

    if (IS_P2P_DEV(pst_vap)) {
        /* device下sta个数加1 */
        pst_dev->st_p2p_info.uc_p2p_device_num++;
    } else if (IS_P2P_GO(pst_vap)) {
        pst_dev->st_p2p_info.uc_p2p_goclient_num++;
    } else if (IS_P2P_CL(pst_vap)) {
        pst_dev->st_p2p_info.uc_p2p_goclient_num++;
    }
}


uint32_t mac_vap_save_app_ie(mac_vap_stru *mac_vap, oal_app_ie_stru *pst_app_ie, en_app_ie_type_uint8 en_type)
{
    uint8_t           *puc_ie = NULL;
    uint32_t           ie_len;
    oal_app_ie_stru      st_tmp_app_ie;
    int32_t            l_ret;

    memset_s(&st_tmp_app_ie, sizeof(st_tmp_app_ie), 0, sizeof(st_tmp_app_ie));

    if (en_type >= OAL_APP_IE_NUM) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_save_app_ie::invalid en_type[%d].}", en_type);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    ie_len = pst_app_ie->ie_len;

    /* 如果输入WPS 长度为0， 则直接释放VAP 中资源 */
    if (ie_len == 0) {
        if (mac_vap->ast_app_ie[en_type].puc_ie != NULL) {
            oal_mem_free_m(mac_vap->ast_app_ie[en_type].puc_ie, OAL_TRUE);
        }

        mac_vap->ast_app_ie[en_type].puc_ie         = NULL;
        mac_vap->ast_app_ie[en_type].ie_len      = 0;
        mac_vap->ast_app_ie[en_type].ie_max_len  = 0;

        return OAL_SUCC;
    }

    /* 检查该类型的IE是否需要申请内存 */
    if ((mac_vap->ast_app_ie[en_type].ie_max_len < ie_len) ||
        (mac_vap->ast_app_ie[en_type].puc_ie == NULL)) {
        /* 这种情况不应该出现，维测需要 */
        if (mac_vap->ast_app_ie[en_type].puc_ie == NULL && mac_vap->ast_app_ie[en_type].ie_max_len != 0) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_save_app_ie::invalid len[%d].}",
                mac_vap->ast_app_ie[en_type].ie_max_len);
        }

        /* 如果以前的内存空间小于新信息元素需要的长度，则需要重新申请内存 */
        puc_ie = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, (uint16_t)(ie_len), OAL_TRUE);
        if (puc_ie == NULL) {
            oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_save_app_ie::LOCAL_MEM_POOL is empty!,\
                len[%d], en_type[%d].}", pst_app_ie->ie_len, en_type);
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        oal_mem_free_m(mac_vap->ast_app_ie[en_type].puc_ie, OAL_TRUE);

        mac_vap->ast_app_ie[en_type].puc_ie = puc_ie;
        mac_vap->ast_app_ie[en_type].ie_max_len = ie_len;
    }

    l_ret = memcpy_s((void *)mac_vap->ast_app_ie[en_type].puc_ie, ie_len, (void *)pst_app_ie->auc_ie, ie_len);
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "mac_vap_save_app_ie::memcpy fail!");
        return OAL_FAIL;
    }

    mac_vap->ast_app_ie[en_type].ie_len = ie_len;
    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_save_app_ie::IE:[0x%2x][0x%2x][0x%2x][0x%2x]}",
        *(mac_vap->ast_app_ie[en_type].puc_ie), *(mac_vap->ast_app_ie[en_type].puc_ie + BYTE_OFFSET_1),
        *(mac_vap->ast_app_ie[en_type].puc_ie + BYTE_OFFSET_2),
        *(mac_vap->ast_app_ie[en_type].puc_ie + ie_len - BYTE_OFFSET_1));
    return OAL_SUCC;
}

uint32_t mac_vap_clear_app_ie(mac_vap_stru *pst_mac_vap, en_app_ie_type_uint8 en_type)
{
    if (en_type < OAL_APP_IE_NUM) {
        if (pst_mac_vap->ast_app_ie[en_type].puc_ie != NULL) {
            oal_mem_free_m(pst_mac_vap->ast_app_ie[en_type].puc_ie, OAL_TRUE);
            pst_mac_vap->ast_app_ie[en_type].puc_ie = NULL;
        }
        pst_mac_vap->ast_app_ie[en_type].ie_len = 0;
        pst_mac_vap->ast_app_ie[en_type].ie_max_len = 0;
    }

    return OAL_SUCC;
}


uint32_t mac_vap_exit(mac_vap_stru *vap)
{
    mac_device_stru               *mac_dev = NULL;
    uint8_t                      uc_index;

    if (oal_unlikely(vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_vap_exit::pst_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    vap->uc_init_flag = MAC_VAP_INVAILD;

    /* 释放vowifi相关内存 */
    mac_vap_vowifi_exit(vap);

    /* 释放WPS信息元素内存 */
    for (uc_index = 0; uc_index < OAL_APP_IE_NUM; uc_index++) {
        if (mac_vap_clear_app_ie(vap, uc_index) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{mac_vap_exit::mac_vap_clear_app_ie failed.}");
        }
    }

    /* 业务vap已删除，从device上去掉 */
    mac_dev = mac_res_get_dev(vap->uc_device_id);
    if (oal_unlikely(mac_dev == NULL)) {
        oam_error_log1(vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_exit::mac_dev[%d] null.}", vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 业务vap已经删除，从device中去掉 */
    for (uc_index = 0; uc_index < mac_dev->uc_vap_num; uc_index++) {
        /* 从device中找到vap id */
        if (mac_dev->auc_vap_id[uc_index] == vap->uc_vap_id) {
            /* 如果不是最后一个vap，则把最后一个vap id移动到这个位置，使得该数组是紧凑的 */
            if (uc_index < (mac_dev->uc_vap_num - 1)) {
                mac_dev->auc_vap_id[uc_index] = mac_dev->auc_vap_id[mac_dev->uc_vap_num - 1];
                break;
            }
        }
    }

    if (mac_dev->uc_vap_num != 0) {
        /* device下的vap总数减1 */
        mac_dev->uc_vap_num--;
    } else {
        oam_error_log1(vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_exit::vap_num is 0. sta_num = %d}", mac_dev->uc_sta_num);
    }
    /* 清除数组中已删除的vap id，保证非零数组元素均为未删除vap */
    mac_dev->auc_vap_id[mac_dev->uc_vap_num] = 0;

    /* device下sta个数减1 */
    if (vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (mac_dev->uc_sta_num != 0) {
            mac_dev->uc_sta_num--;
        } else {
            oam_error_log1(vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_exit::sta is 0. vap_num = %d}", mac_dev->uc_vap_num);
        }
    }

    mac_dec_p2p_num(vap);

    vap->en_protocol = WLAN_PROTOCOL_BUTT;

    /* 最后1个vap删除时，清除device级带宽信息 */
    if (mac_dev->uc_vap_num == 0) {
        mac_dev->uc_max_channel = 0;
        mac_dev->en_max_band = WLAN_BAND_BUTT;
        mac_dev->en_max_bandwidth = WLAN_BAND_WIDTH_BUTT;
    }

    /* 删除之后将vap的状态置位非法 */
    mac_vap_state_change(vap, MAC_VAP_STATE_BUTT);

    return OAL_SUCC;
}



void mac_vap_cap_init_legacy(mac_vap_stru *pst_mac_vap)
{
    pst_mac_vap->st_cap_flag.bit_rifs_tx_on = OAL_FALSE;

    /* 非VHT不使能 txop ps */
    pst_mac_vap->st_cap_flag.bit_txop_ps = OAL_FALSE;

    if (pst_mac_vap->pst_mib_info != NULL) {
        mac_mib_set_txopps(pst_mac_vap, OAL_FALSE);
    }

    return;
}


uint32_t mac_vap_cap_init_htvht(mac_vap_stru *pst_mac_vap)
{
    pst_mac_vap->st_cap_flag.bit_rifs_tx_on = OAL_FALSE;

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    if (pst_mac_vap->pst_mib_info == NULL) {
        oam_error_log3(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                       "{mac_vap_cap_init_htvht::pst_mib_info null,vap mode[%d] state[%d] user num[%d].}",
                       pst_mac_vap->en_vap_mode, pst_mac_vap->en_vap_state, pst_mac_vap->us_user_nums);
        return OAL_FAIL;
    }

    pst_mac_vap->st_cap_flag.bit_txop_ps = OAL_FALSE;
    if ((pst_mac_vap->en_protocol == WLAN_VHT_MODE || pst_mac_vap->en_protocol == WLAN_VHT_ONLY_MODE ||
        (g_wlan_spec_cfg->feature_11ax_is_open && (pst_mac_vap->en_protocol == WLAN_HE_MODE))) &&
        mac_mib_get_txopps(pst_mac_vap)) {
        mac_mib_set_txopps(pst_mac_vap, OAL_TRUE);
    } else {
        mac_mib_set_txopps(pst_mac_vap, OAL_FALSE);
    }
#endif

    return OAL_SUCC;
}


uint32_t mac_vap_config_vht_ht_mib_by_protocol(mac_vap_stru *pst_mac_vap)
{
    if (pst_mac_vap->pst_mib_info == NULL) {
        oam_error_log3(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                       "{mac_vap_config_vht_ht_mib_by_protocol::pst_mib_info null,  \
                       vap mode[%d] state[%d] user num[%d].}",
                       pst_mac_vap->en_vap_mode, pst_mac_vap->en_vap_state, pst_mac_vap->us_user_nums);
        return OAL_FAIL;
    }
    /* 根据协议模式更新 HT/VHT mib值 */
    if (pst_mac_vap->en_protocol == WLAN_HT_MODE || pst_mac_vap->en_protocol == WLAN_HT_ONLY_MODE) {
        mac_mib_set_HighThroughputOptionImplemented(pst_mac_vap, OAL_TRUE);
        mac_mib_set_VHTOptionImplemented(pst_mac_vap, OAL_FALSE);

#ifdef _PRE_WLAN_FEATURE_11AX
        if (g_wlan_spec_cfg->feature_11ax_is_open) {
            mac_mib_set_HEOptionImplemented(pst_mac_vap, OAL_FALSE);
        }
#endif
    } else if (pst_mac_vap->en_protocol == WLAN_VHT_MODE || pst_mac_vap->en_protocol == WLAN_VHT_ONLY_MODE) {
        mac_mib_set_HighThroughputOptionImplemented(pst_mac_vap, OAL_TRUE);
        mac_mib_set_VHTOptionImplemented(pst_mac_vap, OAL_TRUE);
#ifdef _PRE_WLAN_FEATURE_11AX
        if (g_wlan_spec_cfg->feature_11ax_is_open) {
            mac_mib_set_HEOptionImplemented(pst_mac_vap, OAL_FALSE);
        }
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    } else if (g_wlan_spec_cfg->feature_11ax_is_open && pst_mac_vap->en_protocol == WLAN_HE_MODE) {
        mac_mib_set_HighThroughputOptionImplemented(pst_mac_vap, OAL_TRUE);
        mac_mib_set_VHTOptionImplemented(pst_mac_vap, OAL_TRUE);
        mac_mib_set_HEOptionImplemented(pst_mac_vap, OAL_TRUE);
#endif
    } else {
        mac_mib_set_HighThroughputOptionImplemented(pst_mac_vap, OAL_FALSE);
        mac_mib_set_VHTOptionImplemented(pst_mac_vap, OAL_FALSE);
#ifdef _PRE_WLAN_FEATURE_11AX
        if (g_wlan_spec_cfg->feature_11ax_is_open) {
            mac_mib_set_HEOptionImplemented(pst_mac_vap, OAL_FALSE);
        }
#endif
    }

    if (!pst_mac_vap->en_vap_wmm) {
        mac_mib_set_HighThroughputOptionImplemented(pst_mac_vap, OAL_FALSE);
        mac_mib_set_VHTOptionImplemented(pst_mac_vap, OAL_FALSE);
    }

    return OAL_SUCC;
}


void mac_vap_init_rx_nss_by_protocol(mac_vap_stru *pst_mac_vap)
{
    wlan_protocol_enum_uint8     en_protocol;
    mac_device_stru             *pst_mac_device;

    en_protocol = pst_mac_vap->en_protocol;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == NULL)) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_init_rx_nss_by_protocol::\
            pst_mac_device[%d] null.}", pst_mac_vap->uc_device_id);
        return;
    }
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open && (en_protocol == WLAN_HE_MODE)) {
        pst_mac_vap->en_vap_rx_nss = pst_mac_device->st_device_cap.en_nss_num;
    }
#endif
    switch (en_protocol) {
        case WLAN_HT_MODE:
        case WLAN_VHT_MODE:
        case WLAN_HT_ONLY_MODE:
        case WLAN_VHT_ONLY_MODE:
        case WLAN_HT_11G_MODE:
            pst_mac_vap->en_vap_rx_nss = pst_mac_device->st_device_cap.en_nss_num;
            break;
        case WLAN_PROTOCOL_BUTT:
            pst_mac_vap->en_vap_rx_nss = WLAN_NSS_LIMIT;
            return;
        default:
            pst_mac_vap->en_vap_rx_nss = WLAN_SINGLE_NSS;
            break;
    }
#ifdef _PRE_WLAN_FEATURE_M2S
    pst_mac_vap->en_vap_rx_nss = oal_min(
        pst_mac_vap->en_vap_rx_nss,
        oal_min(MAC_DEVICE_GET_NSS_NUM(pst_mac_device),   \
                MAC_M2S_CALI_NSS_FROM_SMPS_MODE(MAC_DEVICE_GET_MODE_SMPS(pst_mac_device))));
#else
    pst_mac_vap->en_vap_rx_nss = oal_min(pst_mac_vap->en_vap_rx_nss, MAC_DEVICE_GET_NSS_NUM(pst_mac_device));
#endif
}


uint32_t mac_vap_init_by_protocol(mac_vap_stru *pst_mac_vap, wlan_protocol_enum_uint8 en_protocol)
{
    pst_mac_vap->en_protocol = en_protocol;

    if (en_protocol < WLAN_HT_MODE) {
        mac_vap_cap_init_legacy(pst_mac_vap);
    } else {
        if (OAL_SUCC != mac_vap_cap_init_htvht(pst_mac_vap)) {
            return OAL_FAIL;
        }
    }

    /* 根据协议模式更新mib值 */
    if (mac_vap_config_vht_ht_mib_by_protocol(pst_mac_vap) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 根据协议更新初始化空间流个数 */
    mac_vap_init_rx_nss_by_protocol(pst_mac_vap);
#ifdef _PRE_WLAN_FEATURE_11AC2G
    if ((en_protocol == WLAN_HT_MODE) &&
        (pst_mac_vap->st_cap_flag.bit_11ac2g == OAL_TRUE) &&
        (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
        mac_mib_set_VHTOptionImplemented(pst_mac_vap, OAL_TRUE);
    }
#endif

    return OAL_SUCC;
}


wlan_bw_cap_enum_uint8 mac_vap_bw_mode_to_bw(wlan_channel_bandwidth_enum_uint8 en_mode)
{
    OAL_STATIC wlan_bw_cap_enum_uint8 g_bw_mode_to_bw_table[] = {
        WLAN_BW_CAP_20M,  // WLAN_BAND_WIDTH_20M                 = 0,
        WLAN_BW_CAP_40M,  // WLAN_BAND_WIDTH_40PLUS              = 1,    /* 从20信道+1 */
        WLAN_BW_CAP_40M,  // WLAN_BAND_WIDTH_40MINUS             = 2,    /* 从20信道-1 */
        WLAN_BW_CAP_80M,  // WLAN_BAND_WIDTH_80PLUSPLUS          = 3,    /* 从20信道+1, 从40信道+1 */
        WLAN_BW_CAP_80M,  // WLAN_BAND_WIDTH_80PLUSMINUS         = 4,    /* 从20信道+1, 从40信道-1 */
        WLAN_BW_CAP_80M,  // WLAN_BAND_WIDTH_80MINUSPLUS         = 5,    /* 从20信道-1, 从40信道+1 */
        WLAN_BW_CAP_80M,  // WLAN_BAND_WIDTH_80MINUSMINUS        = 6,    /* 从20信道-1, 从40信道-1 */
#ifdef _PRE_WLAN_FEATURE_160M
        WLAN_BW_CAP_160M,  // WLAN_BAND_WIDTH_160PLUSPLUSPLUS,            /* 从20信道+1, 从40信道+1, 从80信道+1 */
        WLAN_BW_CAP_160M,  // WLAN_BAND_WIDTH_160PLUSPLUSMINUS,           /* 从20信道+1, 从40信道+1, 从80信道-1 */
        WLAN_BW_CAP_160M,  // WLAN_BAND_WIDTH_160PLUSMINUSPLUS,           /* 从20信道+1, 从40信道-1, 从80信道+1 */
        WLAN_BW_CAP_160M,  // WLAN_BAND_WIDTH_160PLUSMINUSMINUS,          /* 从20信道+1, 从40信道-1, 从80信道-1 */
        WLAN_BW_CAP_160M,  // WLAN_BAND_WIDTH_160MINUSPLUSPLUS,           /* 从20信道-1, 从40信道+1, 从80信道+1 */
        WLAN_BW_CAP_160M,  // WLAN_BAND_WIDTH_160MINUSPLUSMINUS,          /* 从20信道-1, 从40信道+1, 从80信道-1 */
        WLAN_BW_CAP_160M,  // WLAN_BAND_WIDTH_160MINUSMINUSPLUS,          /* 从20信道-1, 从40信道-1, 从80信道+1 */
        WLAN_BW_CAP_160M,  // WLAN_BAND_WIDTH_160MINUSMINUSMINUS,         /* 从20信道-1, 从40信道-1, 从80信道-1 */
#endif

        WLAN_BW_CAP_40M,  // WLAN_BAND_WIDTH_40M,
        WLAN_BW_CAP_80M,  // WLAN_BAND_WIDTH_80M,
    };

    if (en_mode >= WLAN_BAND_WIDTH_BUTT) {
        oam_error_log1(0, OAM_SF_ANY, "mac_vap_bw_mode_to_bw::invalid en_mode = %d, force 20M", en_mode);
        en_mode = 0;
    }

    return g_bw_mode_to_bw_table[en_mode];
}



uint32_t mac_vap_set_bssid(mac_vap_stru *pst_mac_vap, uint8_t *puc_bssid)
{
    if (EOK != memcpy_s(pst_mac_vap->auc_bssid, WLAN_MAC_ADDR_LEN, puc_bssid, WLAN_MAC_ADDR_LEN)) {
        oam_error_log0(0, OAM_SF_ANY, "mac_vap_set_bssid::memcpy fail!");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}



void mac_vap_state_change(mac_vap_stru *pst_mac_vap, mac_vap_state_enum_uint8 en_vap_state)
{
    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
        "{mac_vap_state_change:from[%d]to[%d]}", pst_mac_vap->en_vap_state, en_vap_state);
    pst_mac_vap->en_vap_state = en_vap_state;
}


void mac_vap_get_bandwidth_cap(mac_vap_stru *mac_vap, wlan_bw_cap_enum_uint8 *pen_cap)
{
    wlan_bw_cap_enum_uint8 en_band_cap = WLAN_BW_CAP_20M;

    if (WLAN_BAND_WIDTH_40PLUS == mac_vap->st_channel.en_bandwidth ||
        WLAN_BAND_WIDTH_40MINUS == mac_vap->st_channel.en_bandwidth) {
        en_band_cap = WLAN_BW_CAP_40M;
    } else if (WLAN_BAND_WIDTH_80PLUSPLUS <= mac_vap->st_channel.en_bandwidth &&
               mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS) {
        en_band_cap = WLAN_BW_CAP_80M;
    }
#ifdef _PRE_WLAN_FEATURE_160M
    if (WLAN_BAND_WIDTH_160PLUSPLUSPLUS <= mac_vap->st_channel.en_bandwidth &&
        mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS) {
        en_band_cap = WLAN_BW_CAP_160M;
    }
#endif

    *pen_cap = en_band_cap;
}

wlan_channel_bandwidth_enum_uint8 mac_vap_cap_40m_get_bandwith(wlan_channel_bandwidth_enum_uint8 bss_bandwith)
{
    wlan_channel_bandwidth_enum_uint8 band_with = WLAN_BAND_WIDTH_20M;

    if (bss_bandwith <= WLAN_BAND_WIDTH_40MINUS) {
        band_with = bss_bandwith;
    } else if ((WLAN_BAND_WIDTH_80PLUSPLUS <= bss_bandwith) && (bss_bandwith <= WLAN_BAND_WIDTH_80PLUSMINUS)) {
        band_with = WLAN_BAND_WIDTH_40PLUS;
    } else if ((WLAN_BAND_WIDTH_80MINUSPLUS <= bss_bandwith) && (bss_bandwith <= WLAN_BAND_WIDTH_80MINUSMINUS)) {
        band_with = WLAN_BAND_WIDTH_40MINUS;
    }
#ifdef _PRE_WLAN_FEATURE_160M
    if ((WLAN_BAND_WIDTH_160PLUSPLUSPLUS <= bss_bandwith) && (bss_bandwith <= WLAN_BAND_WIDTH_160PLUSMINUSMINUS)) {
        band_with = WLAN_BAND_WIDTH_40PLUS;
    } else if ((WLAN_BAND_WIDTH_160MINUSPLUSPLUS <= bss_bandwith) &&
        (bss_bandwith <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS)) {
        band_with = WLAN_BAND_WIDTH_40MINUS;
    }
#endif
    return band_with;
}

wlan_channel_bandwidth_enum_uint8 mac_vap_cap_80m_get_bandwith(wlan_channel_bandwidth_enum_uint8 bss_bandwith)
{
    wlan_channel_bandwidth_enum_uint8 band_with = WLAN_BAND_WIDTH_20M;

    if (bss_bandwith <= WLAN_BAND_WIDTH_80MINUSMINUS) {
        band_with = bss_bandwith;
    }
#ifdef _PRE_WLAN_FEATURE_160M
    if ((WLAN_BAND_WIDTH_160PLUSPLUSPLUS <= bss_bandwith) && (bss_bandwith <= WLAN_BAND_WIDTH_160PLUSPLUSMINUS)) {
        band_with = WLAN_BAND_WIDTH_80PLUSPLUS;
    } else if ((WLAN_BAND_WIDTH_160PLUSMINUSPLUS <= bss_bandwith) &&
        (bss_bandwith <= WLAN_BAND_WIDTH_160PLUSMINUSMINUS)) {
        band_with = WLAN_BAND_WIDTH_80PLUSMINUS;
    } else if ((WLAN_BAND_WIDTH_160MINUSPLUSPLUS <= bss_bandwith) &&
        (bss_bandwith <= WLAN_BAND_WIDTH_160MINUSPLUSMINUS)) {
        band_with = WLAN_BAND_WIDTH_80MINUSPLUS;
    } else if ((WLAN_BAND_WIDTH_160MINUSMINUSPLUS <= bss_bandwith) &&
        (bss_bandwith <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS)) {
        band_with = WLAN_BAND_WIDTH_80MINUSMINUS;
    }
#endif

    return band_with;
}


uint8_t mac_vap_get_bandwith(wlan_bw_cap_enum_uint8 en_dev_cap, wlan_channel_bandwidth_enum_uint8 en_bss_cap)
{
    wlan_channel_bandwidth_enum_uint8 en_band_with = WLAN_BAND_WIDTH_20M;

    if (en_bss_cap >= WLAN_BAND_WIDTH_BUTT) {
        oam_error_log2(0, OAM_SF_ANY, "mac_vap_get_bandwith:bss cap is invaild en_dev_cap[%d] to en_bss_cap[%d]",
            en_dev_cap, en_bss_cap);
        return en_band_with;
    }

    switch (en_dev_cap) {
        case WLAN_BW_CAP_20M:
            break;
        case WLAN_BW_CAP_40M:
            en_band_with = mac_vap_cap_40m_get_bandwith(en_bss_cap);
            break;
        case WLAN_BW_CAP_80M:
            en_band_with = mac_vap_cap_80m_get_bandwith(en_bss_cap);
            break;
#ifdef _PRE_WLAN_FEATURE_160M
        case WLAN_BW_CAP_160M:
            if (en_bss_cap < WLAN_BAND_WIDTH_BUTT) {
                en_band_with = en_bss_cap;
            }
#endif
            break;
        default:
            oam_error_log2(0, OAM_SF_ANY,
                "mac_vap_get_bandwith: bandwith en_dev_cap[%d] to en_bss_cap[%d]", en_dev_cap, en_bss_cap);
            break;
    }

    return en_band_with;
}


void mac_sta_init_bss_rates(mac_vap_stru *pst_vap, mac_bss_dscr_stru *bss_dscr)
{
    mac_device_stru               *pst_mac_dev;
    wlan_protocol_enum_uint8       en_vap_protocol;
    mac_data_rate_stru            *pst_rates = NULL;
    uint32_t                     i, j;

    pst_mac_dev = mac_res_get_dev(pst_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_error_log1(pst_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_init_rates::pst_mac_dev[%d] null.}",
            pst_vap->uc_device_id);
        return;
    }

    /* 初始化速率集 */
    pst_rates = mac_device_get_all_rates(pst_mac_dev);
    if (bss_dscr != NULL) {
        for (i = 0; i < bss_dscr->uc_num_supp_rates; i++) {
            for (j = 0; j < MAC_DATARATES_PHY_80211G_NUM; j++) {
                if (IS_EQUAL_RATES(pst_rates[j].uc_mac_rate, bss_dscr->auc_supp_rates[i])) {
                    pst_rates[j].uc_mac_rate = bss_dscr->auc_supp_rates[i];
                    break;
                }
            }
        }
    }

    en_vap_protocol = pst_vap->en_protocol;

    mac_vap_init_rates_by_protocol(pst_vap, en_vap_protocol, pst_rates);
}


void mac_vap_set_rifs_tx_on(mac_vap_stru *pst_vap, uint8_t uc_value)
{
    pst_vap->st_cap_flag.bit_rifs_tx_on = uc_value;
}



void mac_vap_set_11ac2g(mac_vap_stru *pst_vap, uint8_t uc_value)
{
    pst_vap->st_cap_flag.bit_11ac2g = uc_value;
}



uint32_t mac_vap_set_current_channel(mac_vap_stru *pst_vap, wlan_channel_band_enum_uint8 en_band,
    uint8_t uc_channel, uint8_t is_6ghz)
{
    uint8_t  uc_channel_idx = 0;
    uint32_t ret;

    /* 检查信道号 */
    ret = mac_is_channel_num_valid(en_band, uc_channel, is_6ghz);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 根据信道号找到索引号 */
    ret = mac_get_channel_idx_from_num(en_band, uc_channel, is_6ghz, &uc_channel_idx);
    if (ret != OAL_SUCC) {
        return ret;
    }

    pst_vap->st_channel.uc_chan_number = uc_channel;
    pst_vap->st_channel.en_band = en_band;
    pst_vap->st_channel.uc_chan_idx = uc_channel_idx;
    pst_vap->st_channel.ext6g_band = is_6ghz;

    return OAL_SUCC;
}



oal_bool_enum_uint8 mac_vap_check_bss_cap_info_phy_ap(uint16_t us_cap_info, mac_vap_stru *pst_mac_vap)
{
    mac_cap_info_stru *pst_cap_info = (mac_cap_info_stru *)(&us_cap_info);

    if (pst_mac_vap->st_channel.en_band != WLAN_BAND_2G) {
        return OAL_TRUE;
    }

    /* PBCC */
    if ((OAL_FALSE == mac_mib_get_PBCCOptionImplemented(pst_mac_vap)) &&
        (pst_cap_info->bit_pbcc == 1)) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{mac_vap_check_bss_cap_info_phy_ap::PBCC is different.}");
    }

    /* Channel Agility */
    if ((OAL_FALSE == mac_mib_get_ChannelAgilityPresent(pst_mac_vap)) &&
        (pst_cap_info->bit_channel_agility == 1)) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{mac_vap_check_bss_cap_info_phy_ap::Channel Agility is different.}");
    }

    /* DSSS-OFDM Capabilities */
    if ((OAL_FALSE == mac_mib_get_DSSSOFDMOptionActivated(pst_mac_vap)) &&
        (pst_cap_info->bit_dsss_ofdm == 1)) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{mac_vap_check_bss_cap_info_phy_ap::DSSS-OFDM Capabilities is different.}");
    }

    return OAL_TRUE;
}


uint32_t mac_dump_protection(mac_vap_stru *pst_mac_vap, uint8_t *puc_param)
{
    mac_h2d_protection_stru     *pst_h2d_prot = NULL;
    mac_protection_stru         *pst_protection = NULL;

    if (puc_param == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_h2d_prot = (mac_h2d_protection_stru *)puc_param;
    pst_protection = &pst_h2d_prot->st_protection;

    oam_info_log4(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                  "11RIFSMode=%d, LSIGTXOPFullProtectionActivated=%d, NonGFEntitiesPresent=%d, protection_mode=%d\r\n",
                  pst_h2d_prot->en_dot11RIFSMode, pst_h2d_prot->en_dot11LSIGTXOPFullProtectionActivated,
                  pst_h2d_prot->en_dot11NonGFEntitiesPresent, pst_protection->en_protection_mode);
    oam_info_log4(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                  "non_erp_aging_cnt=%d, non_ht_aging_cnt=%d, auto_protection=%d, non_erp_present=%d\r\n",
                  pst_protection->uc_obss_non_erp_aging_cnt, pst_protection->uc_obss_non_ht_aging_cnt,
                  pst_protection->bit_auto_protection, pst_protection->bit_obss_non_erp_present);
    oam_info_log4(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                  "non_ht_present=%d, rts_cts_protect_mode=%d, lsig_txop_protect_mode=%d, sta_no_short_slot_num=%d\r\n",
                  pst_protection->bit_obss_non_ht_present, pst_protection->bit_rts_cts_protect_mode,
                  pst_protection->bit_lsig_txop_protect_mode, pst_protection->uc_sta_no_short_slot_num);
    oam_info_log4(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                  "uc_sta_no_short_preamble_num=%d uc_sta_non_erp_num=%d uc_sta_non_ht_num=%d uc_sta_non_gf_num=%d\r\n",
                  pst_protection->uc_sta_no_short_preamble_num, pst_protection->uc_sta_non_erp_num,
                  pst_protection->uc_sta_non_ht_num, pst_protection->uc_sta_non_gf_num);
    oam_info_log3(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                  "uc_sta_20M_only_num=%d uc_sta_no_40dsss_cck_num=%d uc_sta_no_lsig_txop_num=%d\r\n",
                  pst_protection->uc_sta_20M_only_num, pst_protection->uc_sta_no_40dsss_cck_num,
                  pst_protection->uc_sta_no_lsig_txop_num);

    return OAL_SUCC;
}


wlan_prot_mode_enum_uint8 mac_vap_get_user_protection_mode(mac_vap_stru *pst_mac_vap_sta,
    mac_user_stru *pst_mac_user)
{
    wlan_prot_mode_enum_uint8 en_protection_mode = WLAN_PROT_NO;

    if (oal_any_null_ptr2(pst_mac_vap_sta, pst_mac_user)) {
        return en_protection_mode;
    }

    /* 在2G频段下，如果AP发送的beacon帧ERP ie中Use Protection bit置为1，则将保护级别设置为ERP保护 */
    if ((pst_mac_vap_sta->st_channel.en_band == WLAN_BAND_2G) &&
        (pst_mac_user->st_cap_info.bit_erp_use_protect == OAL_TRUE)) {
        en_protection_mode = WLAN_PROT_ERP;
    } else if ((pst_mac_user->st_ht_hdl.bit_ht_protection == WLAN_MIB_HT_NON_HT_MIXED) ||
        (pst_mac_user->st_ht_hdl.bit_ht_protection == WLAN_MIB_HT_NONMEMBER_PROTECTION)) {
        /* 如果AP发送的beacon帧ht operation ie中ht protection字段为mixed或non-member，则将保护级别设置为HT保护 */
        en_protection_mode = WLAN_PROT_HT;
    } else if (pst_mac_user->st_ht_hdl.bit_nongf_sta_present == OAL_TRUE) {
        /* 如果AP发送的beacon帧ht operation ie中non-gf sta present字段为1，则将保护级别设置为GF保护 */
        en_protection_mode = WLAN_PROT_GF;
    } else {
        /* 剩下的情况不做保护 */
        en_protection_mode = WLAN_PROT_NO;
    }

    return en_protection_mode;
}

oal_bool_enum mac_protection_lsigtxop_check(mac_vap_stru *pst_mac_vap)
{
    mac_user_stru *pst_mac_user = NULL;

    /* 如果不是11n站点，则不支持lsigtxop保护 */
    if ((pst_mac_vap->en_protocol != WLAN_HT_MODE) &&
        (pst_mac_vap->en_protocol != WLAN_HT_ONLY_MODE) &&
        (pst_mac_vap->en_protocol != WLAN_HT_11G_MODE)) {
        return OAL_FALSE;
    }

    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        pst_mac_user =
            (mac_user_stru *)mac_res_get_mac_user(pst_mac_vap->us_assoc_vap_id); /* user保存的是AP的信息 */
        if ((pst_mac_user == NULL) ||
            (pst_mac_user->st_ht_hdl.bit_lsig_txop_protection_full_support == OAL_FALSE)) {
            return OAL_FALSE;
        } else {
            return OAL_TRUE;
        }
    }
    /* lint -e644 */
    /* BSS 中所有站点都支持Lsig txop protection, 则使用Lsig txop protection机制，开销小, AP和STA采用不同的判断 */
    if ((pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (OAL_TRUE == mac_mib_get_LsigTxopFullProtectionActivated(pst_mac_vap))) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
    /* lint +e644 */
}

void mac_protection_set_lsig_txop_mechanism(mac_vap_stru *pst_mac_vap, oal_switch_enum_uint8 en_flag)
{
    /* 数据帧/管理帧发送时候，需要根据bit_lsig_txop_protect_mode值填写发送描述符中的L-SIG TXOP enable位 */
    pst_mac_vap->st_protection.bit_lsig_txop_protect_mode = en_flag;
    oam_warning_log1(0, OAM_SF_CFG, "mac_protection_set_lsig_txop_mechanism:on[%d]?", en_flag);
}


uint32_t mac_vap_add_wep_key(mac_vap_stru *pst_mac_vap, uint8_t us_key_idx,
    uint8_t uc_key_len, uint8_t *puc_key)
{
    mac_user_stru                   *pst_multi_user        = NULL;
    wlan_priv_key_param_stru        *pst_wep_key           = NULL;
    uint32_t                       cipher_type        = WLAN_CIPHER_SUITE_WEP40;
    uint8_t                        uc_wep_cipher_type    = WLAN_80211_CIPHER_SUITE_WEP_40;
    int32_t                        l_ret;

    /* wep 密钥最大为4个 */
    if (us_key_idx >= WLAN_MAX_WEP_KEY_COUNT) {
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }

    switch (uc_key_len) {
        case WLAN_WEP40_KEY_LEN:
            uc_wep_cipher_type = WLAN_80211_CIPHER_SUITE_WEP_40;
            cipher_type = WLAN_CIPHER_SUITE_WEP40;
            break;
        case WLAN_WEP104_KEY_LEN:
            uc_wep_cipher_type = WLAN_80211_CIPHER_SUITE_WEP_104;
            cipher_type = WLAN_CIPHER_SUITE_WEP104;
            break;
        default:
            return OAL_ERR_CODE_SECURITY_KEY_LEN;
    }

    /* WEP密钥信息记录到组播用户中 */
    pst_multi_user = mac_res_get_mac_user(pst_mac_vap->us_multi_user_idx);
    if (pst_multi_user == NULL) {
        return OAL_ERR_CODE_SECURITY_USER_INVAILD;
    }
    mac_mib_set_privacyinvoked(pst_mac_vap, OAL_TRUE);

    /* 初始化组播用户的安全信息 */
    pst_multi_user->st_key_info.en_cipher_type     = uc_wep_cipher_type;
    pst_multi_user->st_key_info.uc_default_index   = us_key_idx;
    pst_multi_user->st_key_info.uc_igtk_key_index  = 0xff;      /* wep时设置为无效 */
    pst_multi_user->st_key_info.bit_gtk            = 0;

    pst_wep_key = &pst_multi_user->st_key_info.ast_key[us_key_idx];

    pst_wep_key->cipher        = cipher_type;
    pst_wep_key->key_len       = (uint32_t)uc_key_len;

    l_ret = memcpy_s(pst_wep_key->auc_key, WLAN_WPA_KEY_LEN, puc_key, WLAN_WEP104_KEY_LEN);
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_vap_add_wep_key::memcpy fail!");
        return OAL_FAIL;
    }

    /* 初始化组播用户的发送信息 */
    pst_multi_user->st_user_tx_info.st_security.en_cipher_key_type = us_key_idx + HAL_KEY_TYPE_PTK;
    pst_multi_user->st_user_tx_info.st_security.en_cipher_protocol_type = uc_wep_cipher_type;

    return OAL_SUCC;
}


uint32_t mac_vap_init_privacy(mac_vap_stru *pst_mac_vap, mac_conn_security_stru *pst_conn_sec)
{
    mac_crypto_settings_stru           *pst_crypto = NULL;
    uint32_t                          ret;

    mac_mib_set_privacyinvoked(pst_mac_vap, OAL_FALSE);
    /* 初始化 RSNActive 为FALSE */
    mac_mib_set_rsnaactivated(pst_mac_vap, OAL_FALSE);
    /* 清除加密套件信息 */
    mac_mib_init_rsnacfg_suites(pst_mac_vap);

    pst_mac_vap->st_cap_flag.bit_wpa = OAL_FALSE;
    pst_mac_vap->st_cap_flag.bit_wpa2 = OAL_FALSE;

#ifdef _PRE_WLAN_FEATURE_PMF
    ret = mac_vap_init_pmf(pst_mac_vap, pst_conn_sec->en_pmf_cap, pst_conn_sec->en_mgmt_proteced);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_WPA,
                       "{mac_11i_init_privacy::mac_11w_init_privacy failed[%d].}", ret);
        return ret;
    }
#endif
    /* 不加密 */
    if (pst_conn_sec->en_privacy == OAL_FALSE) {
        return OAL_SUCC;
    }

    /* WEP加密 */
    if (pst_conn_sec->uc_wep_key_len != 0) {
        return mac_vap_add_wep_key(pst_mac_vap, pst_conn_sec->uc_wep_key_index,
                                   pst_conn_sec->uc_wep_key_len, pst_conn_sec->auc_wep_key);
    }

    /* WPA/WPA2加密 */
    pst_crypto = &(pst_conn_sec->st_crypto);

    /* 初始化RSNA mib 为 TRUR */
    mac_mib_set_privacyinvoked(pst_mac_vap, OAL_TRUE);
    mac_mib_set_rsnaactivated(pst_mac_vap, OAL_TRUE);

    /* 初始化单播密钥套件 */
    if (pst_crypto->wpa_versions == WITP_WPA_VERSION_1) {
        pst_mac_vap->st_cap_flag.bit_wpa = OAL_TRUE;
        mac_mib_set_wpa_pair_suites(pst_mac_vap, pst_crypto->aul_pair_suite);
        mac_mib_set_wpa_akm_suites(pst_mac_vap, pst_crypto->aul_akm_suite);
        mac_mib_set_wpa_group_suite(pst_mac_vap, pst_crypto->group_suite);
    } else if (pst_crypto->wpa_versions == WITP_WPA_VERSION_2) {
        pst_mac_vap->st_cap_flag.bit_wpa2 = OAL_TRUE;
        mac_mib_set_rsn_pair_suites(pst_mac_vap, pst_crypto->aul_pair_suite);
        mac_mib_set_rsn_akm_suites(pst_mac_vap, pst_crypto->aul_akm_suite);
        mac_mib_set_rsn_group_suite(pst_mac_vap, pst_crypto->group_suite);
        mac_mib_set_rsn_group_mgmt_suite(pst_mac_vap, pst_crypto->group_mgmt_suite);
    }

    return OAL_SUCC;
}


mac_user_stru *mac_vap_get_user_by_addr(mac_vap_stru *pst_mac_vap, uint8_t *puc_mac_addr)
{
    uint32_t              ret;
    uint16_t              us_user_idx  = 0xffff;
    mac_user_stru          *pst_mac_user = NULL;

    /* 根据mac addr找到sta索引 */
    ret = mac_vap_find_user_by_macaddr(pst_mac_vap, puc_mac_addr, &us_user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{mac_vap_get_user_by_addr::find_user_by_macaddr failed[%d].}", ret);
        if (puc_mac_addr != NULL) {
            oam_warning_log3(0, OAM_SF_ANY, "{mac_vap_get_user_by_addr::\
                mac[%x:XX:XX:XX:%x:%x] cant be found!}",puc_mac_addr[BYTE_OFFSET_0],
                puc_mac_addr[BYTE_OFFSET_4], puc_mac_addr[BYTE_OFFSET_5]);
        }
        return NULL;
    }

    /* 根据sta索引找到user内存区域 */
    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(us_user_idx);
    if (pst_mac_user == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{mac_vap_get_user_by_addr::user[%d] ptr null.}", us_user_idx);
    }

    return pst_mac_user;
}

OAL_STATIC uint32_t mac_vap_set_rsn_security(mac_vap_stru *mac_vap, mac_beacon_param_stru *beacon_param,
    mac_crypto_settings_stru             *crypto)
{
    uint32_t ret;
    uint16_t rsn_cap;

    mac_vap->st_cap_flag.bit_wpa2 = OAL_TRUE;
    mac_mib_set_rsnaactivated(mac_vap, OAL_TRUE);
    mac_mib_set_AuthenticationMode(mac_vap, WLAN_WITP_AUTH_OPEN_SYSTEM);
    ret = mac_ie_get_rsn_cipher(beacon_param->auc_rsn_ie, crypto);
    if (ret != OAL_SUCC) {
        return ret;
    }
    rsn_cap = mac_get_rsn_capability(beacon_param->auc_rsn_ie);
    mac_mib_set_rsn_group_suite(mac_vap, crypto->group_suite);
    mac_mib_set_rsn_pair_suites(mac_vap, crypto->aul_pair_suite);
    mac_mib_set_rsn_akm_suites(mac_vap, crypto->aul_akm_suite);
    mac_mib_set_rsn_group_mgmt_suite(mac_vap, crypto->group_mgmt_suite);
    /* RSN 能力 */
    mac_mib_set_dot11RSNAMFPR(mac_vap, (rsn_cap & BIT6) ? OAL_TRUE : OAL_FALSE);
    mac_mib_set_dot11RSNAMFPC(mac_vap, (rsn_cap & BIT7) ? OAL_TRUE : OAL_FALSE);
    mac_mib_set_pre_auth_actived(mac_vap, rsn_cap & BIT0);
    mac_mib_set_rsnacfg_ptksareplaycounters(mac_vap, (uint8_t)(rsn_cap & 0x0C) >> BYTE_OFFSET_2);
    mac_mib_set_rsnacfg_gtksareplaycounters(mac_vap, (uint8_t)(rsn_cap & 0x30) >> BYTE_OFFSET_4);

    return OAL_SUCC;
}


uint32_t mac_vap_set_security(mac_vap_stru *pst_mac_vap, mac_beacon_param_stru *pst_beacon_param)
{
    mac_user_stru                        *pst_multi_user = NULL;
    mac_crypto_settings_stru              st_crypto;
    uint32_t                            ret;

    if (oal_any_null_ptr2(pst_mac_vap, pst_beacon_param)) {
        oam_error_log0(0, OAM_SF_WPA, "{mac_vap_add_beacon::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 清除之前的加密配置信息 */
    mac_mib_set_privacyinvoked(pst_mac_vap, OAL_FALSE);
    mac_mib_set_rsnaactivated(pst_mac_vap, OAL_FALSE);
    mac_mib_init_rsnacfg_suites(pst_mac_vap);
    pst_mac_vap->st_cap_flag.bit_wpa = OAL_FALSE;
    pst_mac_vap->st_cap_flag.bit_wpa2 = OAL_FALSE;
    mac_mib_set_dot11RSNAMFPR(pst_mac_vap, OAL_FALSE);
    mac_mib_set_dot11RSNAMFPC(pst_mac_vap, OAL_FALSE);

    /* 清除组播密钥信息 */
    pst_multi_user = mac_res_get_mac_user(pst_mac_vap->us_multi_user_idx);
    if (pst_multi_user == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_add_beacon::pst_multi_user[%d] null.}",
                         pst_mac_vap->us_multi_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_beacon_param->en_privacy == OAL_FALSE) {
        /* 只在非加密场景下清除，加密场景会重新设置覆盖 */
        mac_user_init_key(pst_multi_user);
        pst_multi_user->st_user_tx_info.st_security.en_cipher_key_type = WLAN_KEY_TYPE_TX_GTK;
        return OAL_SUCC;
    }

    /* 使能加密 */
    mac_mib_set_privacyinvoked(pst_mac_vap, OAL_TRUE);

    memset_s(&st_crypto, sizeof(mac_crypto_settings_stru), 0, sizeof(mac_crypto_settings_stru));

    if (pst_beacon_param->auc_wpa_ie[0] == MAC_EID_VENDOR) {
        pst_mac_vap->st_cap_flag.bit_wpa = OAL_TRUE;
        mac_mib_set_rsnaactivated(pst_mac_vap, OAL_TRUE);
        mac_mib_set_AuthenticationMode(pst_mac_vap, WLAN_WITP_AUTH_OPEN_SYSTEM);
        mac_ie_get_wpa_cipher(pst_beacon_param->auc_wpa_ie, &st_crypto);
        mac_mib_set_wpa_group_suite(pst_mac_vap, st_crypto.group_suite);
        mac_mib_set_wpa_pair_suites(pst_mac_vap, st_crypto.aul_pair_suite);
        mac_mib_set_wpa_akm_suites(pst_mac_vap, st_crypto.aul_akm_suite);
    }

    if (pst_beacon_param->auc_rsn_ie[0] == MAC_EID_RSN) {
        ret = mac_vap_set_rsn_security(pst_mac_vap, pst_beacon_param, &st_crypto);
        if (ret != OAL_SUCC) {
            return ret;
        }
    }

    return OAL_SUCC;
}

uint32_t mac_vap_add_key(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user,
    uint8_t uc_key_id, mac_key_params_stru *pst_key)
{
    uint32_t ret;

    switch ((uint8_t)pst_key->cipher) {
        case WLAN_80211_CIPHER_SUITE_WEP_40:
        case WLAN_80211_CIPHER_SUITE_WEP_104:
            /* 设置mib */
            mac_mib_set_privacyinvoked(pst_mac_vap, OAL_TRUE);
            mac_mib_set_rsnaactivated(pst_mac_vap, OAL_FALSE);
            // 设置组播密钥套件应该放在set default key
            ret = mac_user_add_wep_key(pst_mac_user, uc_key_id, pst_key);
            break;
        case WLAN_80211_CIPHER_SUITE_TKIP:
        case WLAN_80211_CIPHER_SUITE_CCMP:
        case WLAN_80211_CIPHER_SUITE_GCMP:
        case WLAN_80211_CIPHER_SUITE_GCMP_256:
        case WLAN_80211_CIPHER_SUITE_CCMP_256:
            ret = mac_user_add_rsn_key(pst_mac_user, uc_key_id, pst_key);
            break;
        case WLAN_80211_CIPHER_SUITE_BIP:
        case WLAN_80211_CIPHER_SUITE_BIP_GMAC_128:
        case WLAN_80211_CIPHER_SUITE_BIP_GMAC_256:
        case WLAN_80211_CIPHER_SUITE_BIP_CMAC_256:
            ret = mac_user_add_bip_key(pst_mac_user, uc_key_id, pst_key);
            break;
        default:
            return OAL_ERR_CODE_SECURITY_CHIPER_TYPE;
    }

    return ret;
}


uint8_t mac_vap_get_default_key_id(mac_vap_stru *pst_mac_vap)
{
    mac_user_stru                *pst_multi_user;
    uint8_t                     uc_default_key_id;

    /* 根据索引，从组播用户密钥信息中查找密钥 */
    pst_multi_user = mac_res_get_mac_user(pst_mac_vap->us_multi_user_idx);
    if (pst_multi_user == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_WPA,
                         "{mac_vap_get_default_key_id::multi_user[%d] NULL}",
                         pst_mac_vap->us_multi_user_idx);
        return 0;
    }

    if ((pst_multi_user->st_key_info.en_cipher_type != WLAN_80211_CIPHER_SUITE_WEP_40) &&
        (pst_multi_user->st_key_info.en_cipher_type != WLAN_80211_CIPHER_SUITE_WEP_104)) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_WPA,
                       "{mac_vap_get_default_key_id::unexpectd cipher_type[%d]}",
                       pst_multi_user->st_key_info.en_cipher_type);
        return 0;
    }
    uc_default_key_id = pst_multi_user->st_key_info.uc_default_index;
    if (uc_default_key_id >= WLAN_NUM_TK) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_WPA,
                       "{mac_vap_get_default_key_id::unexpectd keyid[%d]}",
                       uc_default_key_id);
        return 0;
    }

    return uc_default_key_id;
}


uint32_t mac_vap_set_default_wep_key(mac_vap_stru *pst_mac_vap, uint8_t uc_key_index)
{
    wlan_priv_key_param_stru     *pst_wep_key = NULL;
    mac_user_stru                *pst_multi_user = NULL;

    /* 1.1 如果非wep 加密，则直接返回 */
    if (OAL_TRUE != mac_is_wep_enabled(pst_mac_vap)) {
        return OAL_SUCC;
    }

    /* 2.1 根据索引，从组播用户密钥信息中查找密钥 */
    pst_multi_user = mac_res_get_mac_user(pst_mac_vap->us_multi_user_idx);
    if (pst_multi_user == NULL) {
        return OAL_ERR_CODE_SECURITY_USER_INVAILD;
    }
    pst_wep_key = &pst_multi_user->st_key_info.ast_key[uc_key_index];

    if (pst_wep_key->cipher != WLAN_CIPHER_SUITE_WEP40 &&
        pst_wep_key->cipher != WLAN_CIPHER_SUITE_WEP104) {
        return OAL_ERR_CODE_SECURITY_CHIPER_TYPE;
    }

    /* 3.1 更新密钥类型及default id */
    pst_multi_user->st_key_info.en_cipher_type     = (uint8_t)(pst_wep_key->cipher);
    pst_multi_user->st_key_info.uc_default_index   = uc_key_index;

    /* 4.1 设置mib属性 */
    mac_set_wep_default_keyid(pst_mac_vap, uc_key_index);

    return OAL_SUCC;
}


uint32_t mac_vap_set_default_mgmt_key(mac_vap_stru *pst_mac_vap, uint8_t uc_key_index)
{
    mac_user_stru *pst_multi_user;

    /* 管理帧加密信息保存在组播用户中 */
    pst_multi_user = mac_res_get_mac_user(pst_mac_vap->us_multi_user_idx);
    if (pst_multi_user == NULL) {
        return OAL_ERR_CODE_SECURITY_USER_INVAILD;
    }

    /* keyid校验 */
    if (uc_key_index < WLAN_NUM_TK || uc_key_index > WLAN_MAX_IGTK_KEY_INDEX) {
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }

    switch ((uint8_t)pst_multi_user->st_key_info.ast_key[uc_key_index].cipher) {
        case WLAN_80211_CIPHER_SUITE_BIP:
        case WLAN_80211_CIPHER_SUITE_BIP_GMAC_128:
        case WLAN_80211_CIPHER_SUITE_BIP_GMAC_256:
        case WLAN_80211_CIPHER_SUITE_BIP_CMAC_256:
            /* 更新IGTK的keyid */
            pst_multi_user->st_key_info.uc_igtk_key_index = uc_key_index;
            break;
        default:
            return OAL_ERR_CODE_SECURITY_CHIPER_TYPE;
    }

    return OAL_SUCC;
}


void mac_vap_init_user_security_port(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    if (OAL_TRUE != mac_mib_get_rsnaactivated(pst_mac_vap)) {
        mac_user_set_port(pst_mac_user, OAL_TRUE);
        return;
    }
}


uint8_t *mac_vap_get_mac_addr(mac_vap_stru *pst_mac_vap)
{
    if (IS_P2P_DEV(pst_mac_vap)) {
        /* 获取P2P DEV MAC 地址，赋值到probe req 帧中 */
        return mac_mib_get_p2p0_dot11StationID(pst_mac_vap);
    } else {
        /* 设置地址2为自己的MAC地址 */
        return mac_mib_get_StationID(pst_mac_vap);
    }
}


oal_switch_enum_uint8 mac_vap_protection_autoprot_is_enabled(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->st_protection.bit_auto_protection;
}
