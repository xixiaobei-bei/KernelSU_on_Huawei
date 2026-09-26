

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oam_ext_if.h"
#include "oam_trace.h"
#include "frw_ext_if.h"
#include "mac_device.h"
#include "hmac_device.h"
#include "mac_resource.h"
#include "hmac_resource.h"
#include "mac_vap.h"
#include "mac_ie.h"
#include "oal_hcc_host_if.h"
#include "mac_regdomain.h"
#include "dmac_ext_if.h"
#include "hmac_ext_if.h"
#include "hmac_fsm.h"
#include "hmac_main.h"
#include "hmac_vap.h"
#include "hmac_tx_amsdu.h"
#include "hmac_rx_data.h"
#include "hmac_mgmt_classifier.h"
#include "hmac_config.h"
#include "hmac_chan_mgmt.h"
#include "hmac_rx_filter.h"
#include "hmac_psm_ap.h"
#include "hmac_protection.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_rx_filter.h"
#include "hmac_mgmt_sta.h"
#include "hmac_reset.h"
#include "hmac_scan.h"
#include "hmac_blockack.h"
#include "hmac_p2p.h"
#include "hmac_mgmt_ap.h"
#include "hmac_sme_sta.h"
#include "hmac_204080_coexist.h"
#include "hmac_support_pmf.h"
#include "hmac_sae.h"

#ifdef _PRE_WLAN_FEATURE_ROAM
#include "hmac_roam_main.h"
#endif  // _PRE_WLAN_FEATURE_ROAM

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
#include "hmac_arp_offload.h"
#endif
#ifdef _PRE_WLAN_TCP_OPT
#include "mac_data.h"
#include "hmac_tcp_opt.h"
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif
#ifdef _PRE_WLAN_DFT_STAT
#include "mac_board.h"
#endif

#include "hmac_dscr_th_opt.h"
#include "hmac_tcp_ack_filter.h"
#include "hmac_statistic_data_flow.h"
#include "hmac_tx_data.h"
#include "hmac_dfx.h"
#include "hal_commom_ops.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#ifdef _PRE_WLAN_FEATURE_ROAM
#include "hmac_roam_connect.h"
#include "hmac_roam_alg.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
typedef struct {
    wlan_protocol_enum_uint8 en_protocol_mode; /* wid枚举 */
    oal_uint8 auc_resv[3];
    oal_int8 *puc_protocol_desc;
} hmac_protocol_stru;

OAL_STATIC hmac_protocol_stru g_st_protocol_mode_list[WLAN_PROTOCOL_BUTT] = {
    { WLAN_LEGACY_11A_MODE,    { 0 }, "11a" },
    { WLAN_LEGACY_11B_MODE,    { 0 }, "11b" },
    { WLAN_LEGACY_11G_MODE,    { 0 }, "abandon_mode" },
    { WLAN_MIXED_ONE_11G_MODE, { 0 }, "11bg" },
    { WLAN_MIXED_TWO_11G_MODE, { 0 }, "11g" },
    { WLAN_HT_MODE,            { 0 }, "11n" },
    { WLAN_VHT_MODE,           { 0 }, "11ac" },
    { WLAN_HT_ONLY_MODE,       { 0 }, "11n_only" },
    { WLAN_VHT_ONLY_MODE,      { 0 }, "11ac_only" },
};

oal_uint32 g_customize_interworking = OAL_FALSE;
oal_uint8 g_wlan_ps_mode = 1;
oal_uint32 hmac_config_set_freq(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_mode(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
/*****************************************************************************
  3 函数实现
*****************************************************************************/
oal_uint8 hmac_config_get_ps_mode(void)
{
    return g_wlan_ps_mode;
}
void hmac_config_set_ps_mode(oal_uint8 ps_mode)
{
    g_wlan_ps_mode = ps_mode;
}
oal_int8 *hmac_config_index2string(oal_uint32 ul_index, oal_int8 *pst_string[], oal_uint32 ul_max_str_nums)
{
    if (oal_warn_on(ul_index >= ul_max_str_nums)) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_index2string::wrong index nums:%u, max:%u}",
                       ul_index, ul_max_str_nums);
        return (oal_int8 *)"unkown";
    }
    return pst_string[ul_index];
}

oal_int8 *hmac_config_protocol2string(oal_uint32 ul_protocol)
{
    oal_int8 *pac_protocol2string[] = { "11a", "11b", "error", "11bg", "11g", "11n", "11ac", "11nonly", "11aconly",
                                        "11ng", "error" };
    return hmac_config_index2string(ul_protocol, pac_protocol2string,
                                    OAL_SIZEOF(pac_protocol2string) / OAL_SIZEOF(oal_int8 *));
}

oal_int8 *hmac_config_band2string(oal_uint32 ul_band)
{
    oal_int8 *pac_band2string[] = { "2.4G", "5G", "error" };
    return hmac_config_index2string(ul_band, pac_band2string, OAL_SIZEOF(pac_band2string) / OAL_SIZEOF(oal_int8 *));
}

oal_int8 *hmac_config_bw2string(oal_uint32 ul_bw)
{
    oal_int8 *pac_bw2string[] = { "20M", "40+", "40-", "80++", "80+-", "80-+", "80--", "error" };
    return hmac_config_index2string(ul_bw, pac_bw2string, OAL_SIZEOF(pac_bw2string) / OAL_SIZEOF(oal_int8 *));
}

oal_int8 *hmac_config_cipher2string(oal_uint32 ul_ciper2)
{
    oal_int8 *pac_ciper2string[] = { "GROUP", "WEP40", "TKIP", "RSV", "CCMP", "WEP104", "BIP",
                                     "GROUP_DENY", "GCMP", "GCMP_256", "CCMP_256", "BIP_GMAC_128",
                                     "BIP_GMAC_256", "BIP_CMAC_256", "WAPI", "NONE" };
    return hmac_config_index2string(ul_ciper2, pac_ciper2string,
                                    OAL_SIZEOF(pac_ciper2string) / OAL_SIZEOF(oal_int8 *));
}

oal_int8 *hmac_config_akm2string(oal_uint32 ul_akm2)
{
    oal_int8 *pac_akm2string[] = { "RSV", "1X", "PSK", "FT_1X", "FT_PSK", "1X_SHA256", "PSK_SHA256",
                                   "TDLS", "SAE_SHA256", "FT_SHA256", "NONE" };
    return hmac_config_index2string(ul_akm2, pac_akm2string, OAL_SIZEOF(pac_akm2string) / OAL_SIZEOF(oal_int8 *));
}

oal_int8 *hmac_config_keytype2string(oal_uint32 ul_keytype)
{
    oal_int8 *pac_keytype2string[] = { "GTK", "PTK", "RX_GTK", "ERR" };
    return hmac_config_index2string(ul_keytype, pac_keytype2string,
                                    OAL_SIZEOF(pac_keytype2string) / OAL_SIZEOF(oal_int8 *));
}
oal_int8 *hmac_config_smps2string(oal_uint32 ul_smps)
{
    oal_int8 *pac_smps2string[] = { "Static", "Dynamic", "MIMO", "error" };
    return hmac_config_index2string(ul_smps, pac_smps2string, OAL_SIZEOF(pac_smps2string) / OAL_SIZEOF(oal_int8 *));
}

oal_int8 *hmac_config_dev2string(oal_uint32 ul_dev)
{
    oal_int8 *pac_dev2string[] = { "Close", "Open", "error" };
    return hmac_config_index2string(ul_dev, pac_dev2string, OAL_SIZEOF(pac_dev2string) / OAL_SIZEOF(oal_int8 *));
}

oal_int8 *hmac_config_nss2string(oal_uint32 ul_nss)
{
    oal_int8 *pac_nss2string[] = { "Single Nss", "Double Nss", "error" };
    return hmac_config_index2string(ul_nss, pac_nss2string, OAL_SIZEOF(pac_nss2string) / OAL_SIZEOF(oal_int8 *));
}

oal_int8 *hmac_config_b_w2string(oal_uint32 ul_b_w)
{
    oal_int8 *pac_bw2string[] = { "20M", "40M", "80M", "error" };
    return hmac_config_index2string(ul_b_w, pac_bw2string, OAL_SIZEOF(pac_bw2string) / OAL_SIZEOF(oal_int8 *));
}

oal_uint16 g_us_efuse_buffer[WAL_ATCMDSRV_EFUSE_BUFF_LEN];
oal_uint16 hmac_get_efuse_buffer(oal_uint16 us_efuse_buffer_index)
{
    return g_us_efuse_buffer[us_efuse_buffer_index];
}
oal_void hmac_set_efuse_buffer(oal_uint16 us_efuse_buffer_index, oal_uint16 us_efuse_buffer)
{
    g_us_efuse_buffer[us_efuse_buffer_index] = us_efuse_buffer;
}
oal_uint16 *hmac_get_efuse_buffer_first_addr(oal_void)
{
    return g_us_efuse_buffer;
}


OAL_STATIC oal_uint32 hmac_config_alloc_event(mac_vap_stru *pst_mac_vap,
                                              hmac_to_dmac_syn_type_enum_uint8 en_syn_type,
                                              hmac_to_dmac_cfg_msg_stru **ppst_syn_msg,
                                              frw_event_mem_stru **ppst_event_mem,
                                              oal_uint16 us_len)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    /* 获取事件内存为us_len + OAL_SIZEOF(hmac_to_dmac_cfg_msg_stru) - 4 */
    pst_event_mem = frw_event_alloc_m(us_len + OAL_SIZEOF(hmac_to_dmac_cfg_msg_stru) - 4);
    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_alloc_event::pst_event_mem null, us_len = %d }", us_len);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    /* 填充事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CRX,
                       en_syn_type,
                       (us_len + OAL_SIZEOF(hmac_to_dmac_cfg_msg_stru) - 4), /* 事件内存大小（4是计算内存大小的一个参数） */
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    /* 出参赋值 */
    *ppst_event_mem = pst_event_mem;
    *ppst_syn_msg = (hmac_to_dmac_cfg_msg_stru *)pst_event->auc_event_data;

    return OAL_SUCC;
}


oal_uint32 hmac_config_send_event(mac_vap_stru *pst_mac_vap,
                                  wlan_cfgid_enum_uint16 en_cfg_id,
                                  oal_uint16 us_len,
                                  oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    hmac_to_dmac_cfg_msg_stru *pst_syn_msg = OAL_PTR_NULL;

    ul_ret = hmac_config_alloc_event(pst_mac_vap, HMAC_TO_DMAC_SYN_CFG, &pst_syn_msg, &pst_event_mem, us_len);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_send_event::hmac_config_alloc_event failed[%d].}", ul_ret);
        return ul_ret;
    }

    hmac_init_syn_msg_hdr(pst_syn_msg, en_cfg_id, us_len);

    /* 填写配置同步消息内容 */
    if ((puc_param != OAL_PTR_NULL) && (us_len)) {
        if (memcpy_s(pst_syn_msg->auc_msg_body, (oal_uint32)us_len, puc_param, (oal_uint32)us_len) != EOK) {
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "hmac_config_send_event::memcpy fail!");
            frw_event_free_m(pst_event_mem);
            return OAL_FAIL;
        }
    }

    /* 抛出事件 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_send_event::frw_event_dispatch_event failed[%d].}", ul_ret);
        frw_event_free_m(pst_event_mem);
        return ul_ret;
    }

    frw_event_free_m(pst_event_mem);

    return OAL_SUCC;
}


oal_uint32 hmac_config_alg_send_event(mac_vap_stru *pst_mac_vap,
                                      wlan_cfgid_enum_uint16 en_cfg_id,
                                      oal_uint16 us_len,
                                      oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    hmac_to_dmac_cfg_msg_stru *pst_syn_msg = OAL_PTR_NULL;

    ul_ret = hmac_config_alloc_event(pst_mac_vap, HMAC_TO_DMAC_SYN_ALG, &pst_syn_msg, &pst_event_mem, us_len);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_alg_send_event::hmac_config_alloc_event failed[%d].}", ul_ret);
        return ul_ret;
    }

    hmac_init_syn_msg_hdr(pst_syn_msg, en_cfg_id, us_len);

    /* 填写配置同步消息内容 */
    if (memcpy_s(pst_syn_msg->auc_msg_body, us_len, puc_param, us_len) != EOK) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_alg_send_event::memcpy fail.}");
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    /* 抛出事件 */
    frw_event_dispatch_event(pst_event_mem);

    frw_event_free_m(pst_event_mem);

    return OAL_SUCC;
}


oal_uint32 hmac_config_start_vap_event(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_mgmt_rate_init_flag)
{
    oal_uint32 ul_ret;
    mac_cfg_start_vap_param_stru st_start_vap_param;

    /* DMAC不使用netdev成员 */
    st_start_vap_param.pst_net_dev = OAL_PTR_NULL;
    st_start_vap_param.en_mgmt_rate_init_flag = en_mgmt_rate_init_flag;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    st_start_vap_param.uc_protocol = pst_mac_vap->en_protocol;
    st_start_vap_param.uc_band = pst_mac_vap->st_channel.en_band;
    st_start_vap_param.uc_bandwidth = pst_mac_vap->st_channel.en_bandwidth;
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
    st_start_vap_param.en_p2p_mode = pst_mac_vap->en_p2p_mode;
#endif

    ul_ret = hmac_config_send_event(pst_mac_vap,
                                    WLAN_CFGID_START_VAP,
                                    OAL_SIZEOF(mac_cfg_start_vap_param_stru),
                                    (oal_uint8 *)&st_start_vap_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap_event::Start_vap failed[%d].}",
                         ul_ret);
    }
    return ul_ret;
}

oal_uint32 hmac_set_mode_event(mac_vap_stru *pst_mac_vap)
{
    oal_uint32 ul_ret;
    mac_cfg_mode_param_stru st_prot_param;

    /* 设置带宽模式，直接抛事件到DMAC配置寄存器 */
    st_prot_param.en_protocol = pst_mac_vap->en_protocol;
    st_prot_param.en_band = pst_mac_vap->st_channel.en_band;
    st_prot_param.en_bandwidth = pst_mac_vap->st_channel.en_bandwidth;

    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_MODE, OAL_SIZEOF(mac_cfg_mode_param_stru),
                                    (oal_uint8 *)&st_prot_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_set_mode_event::mode_set failed[%d],protocol[%d], band[%d], bandwidth[%d].}",
                         ul_ret, pst_mac_vap->en_protocol, pst_mac_vap->st_channel.en_band,
                         pst_mac_vap->st_channel.en_bandwidth);
    }
    return ul_ret;
}

oal_uint32 hmac_config_update_opmode_event(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user,
                                           oal_uint8 uc_mgmt_frm_type)
{
    oal_uint32 ul_relt;
    mac_user_opmode_stru st_user_opmode;
    /* opmode息同步dmac */
    st_user_opmode.uc_avail_num_spatial_stream = pst_mac_user->uc_avail_num_spatial_stream;
    st_user_opmode.uc_avail_bf_num_spatial_stream = pst_mac_user->uc_avail_bf_num_spatial_stream;
    st_user_opmode.en_avail_bandwidth = pst_mac_user->en_avail_bandwidth;
    st_user_opmode.en_cur_bandwidth = pst_mac_user->en_cur_bandwidth;
    st_user_opmode.us_user_idx = pst_mac_user->us_assoc_id;
    st_user_opmode.uc_frame_type = uc_mgmt_frm_type;

    ul_relt = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_UPDATE_OPMODE,
                                     OAL_SIZEOF(mac_user_opmode_stru),
                                     (oal_uint8 *)(&st_user_opmode));
    if (oal_unlikely(ul_relt != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_user->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_update_opmode_event::opmode_event send failed[%d].}", ul_relt);
    }

    return ul_relt;
}


oal_uint32 hmac_config_sync_cmd_common(mac_vap_stru *pst_mac_vap, wlan_cfgid_enum_uint16 en_cfg_id,
                                       oal_uint16 us_len,
                                       oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, en_cfg_id, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_sync_cmd_common::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_get_hipkt_stat(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_GET_HIPKT_STAT, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_alg::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_flowctl_param(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_flowctl_param_stru *pst_flowctl_param = (mac_cfg_flowctl_param_stru *)puc_param;

    /* 设置host flowctl 相关参数 */
    hcc_host_set_flowctl_param(pst_flowctl_param->uc_queue_type, pst_flowctl_param->us_burst_limit,
                               pst_flowctl_param->us_low_waterline, pst_flowctl_param->us_high_waterline);

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                     "hcc_host_set_flowctl_param, queue[%d]: burst limit = %d, low_waterline = %d, high_waterline =%d",
                     pst_flowctl_param->uc_queue_type, pst_flowctl_param->us_burst_limit,
                     pst_flowctl_param->us_low_waterline,
                     pst_flowctl_param->us_high_waterline);

    return OAL_SUCC;
}


oal_uint32 hmac_config_get_flowctl_stat(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    /* 设置host flowctl 相关参数 */
    hcc_host_get_flowctl_stat();

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE oal_uint32 hmac_normal_check_legacy_vap_num(mac_device_stru *pst_mac_device,
                                                                  wlan_vap_mode_enum_uint8 en_vap_mode)
{
    /* VAP个数判断 */
    if (en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        if ((pst_mac_device->uc_sta_num == 1) && (pst_mac_device->uc_vap_num == WLAN_AP_STA_COEXIST_VAP_NUM)) {
            /* AP STA共存场景，只能创建4个AP + 1个STA */
            oam_warning_log0(0, OAM_SF_CFG,
                             "{hmac_normal_check_legacy_vap_num::have created 4AP + 1STA, cannot create another AP.}");
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }

        if ((pst_mac_device->uc_vap_num - pst_mac_device->uc_sta_num) >= WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE) {
            /* 已创建的AP个数达到最大值4 */
            oam_warning_log2(0, OAM_SF_CFG, "{ap num exceeds the supported spec,vap_num[%u],sta_num[%u].}",
                             pst_mac_device->uc_vap_num, pst_mac_device->uc_sta_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }
    } else if (en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (pst_mac_device->uc_sta_num >= WLAN_SERVICE_STA_MAX_NUM_PER_DEVICE) {
            /* 已创建的STA个数达到最大值 */
            OAM_WARNING_LOG1(0, OAM_SF_CFG,
                             "{hmac_normal_check_legacy_vap_num::have created 2+ AP.can not create STA any more[%d].}",
                             pst_mac_device->uc_sta_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_normal_check_vap_num(mac_device_stru *pst_mac_device, mac_cfg_add_vap_param_stru *pst_param)
{
    wlan_vap_mode_enum_uint8 en_vap_mode;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
#endif

#ifdef _PRE_WLAN_FEATURE_P2P
    en_p2p_mode = pst_param->en_p2p_mode;
    if (en_p2p_mode != WLAN_LEGACY_VAP_MODE) {
        return hmac_check_p2p_vap_num(pst_mac_device, en_p2p_mode);
    }
#endif

    en_vap_mode = pst_param->en_vap_mode;
    return hmac_normal_check_legacy_vap_num(pst_mac_device, en_vap_mode);
}


oal_uint32 hmac_config_check_vap_num(mac_device_stru *pst_mac_device, mac_cfg_add_vap_param_stru *pst_param)
{
    return hmac_config_normal_check_vap_num(pst_mac_device, pst_param);
}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32 hmac_cfg_vap_send_event(mac_device_stru *pst_device)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    /* 抛事件给DMAC,让DMAC完成配置VAP创建 */
    pst_event_mem = frw_event_alloc_m(0);
    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_cfg_vap_send_event::pst_event_mem null.}");
        return OAL_PTR_NULL;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CRX,
                       HMAC_TO_DMAC_SYN_CREATE_CFG_VAP,
                       0,
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_device->uc_chip_id,
                       pst_device->uc_device_id,
                       pst_device->uc_cfg_vap_id);

    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_cfg_vap_send_event::frw_event_dispatch_event failed[%d].}", ul_ret);
    }

    /* 释放事件 */
    frw_event_free_m(pst_event_mem);

    return ul_ret;
}
#endif


oal_void hmac_vap_clear_timer(hmac_vap_stru *pst_hmac_vap)
{
    /* 清理所有的timer ,防止del vap时没有清理掉，这里马上就要memzero，会造成timer list broken */
    if (pst_hmac_vap->st_mgmt_timer.en_is_registerd == OAL_TRUE) {
        frw_immediate_destroy_timer(&(pst_hmac_vap->st_mgmt_timer));
        pst_hmac_vap->st_mgmt_timer.en_is_registerd = OAL_FALSE;
        OAM_ERROR_LOG0(0, 0, "hmac_vap_clear_timer st_mgmt_timer");
    }
    if (pst_hmac_vap->st_scan_timeout.en_is_registerd == OAL_TRUE) {
        frw_immediate_destroy_timer(&(pst_hmac_vap->st_scan_timeout));
        pst_hmac_vap->st_scan_timeout.en_is_registerd = OAL_FALSE;
        OAM_ERROR_LOG0(0, 0, "hmac_vap_clear_timer st_scan_timeout");
    }
    if (pst_hmac_vap->st_40m_recovery_timer.en_is_registerd == OAL_TRUE) {
        frw_immediate_destroy_timer(&(pst_hmac_vap->st_40m_recovery_timer));
        pst_hmac_vap->st_40m_recovery_timer.en_is_registerd = OAL_FALSE;
        OAM_ERROR_LOG0(0, 0, "hmac_vap_clear_timer st_40m_recovery_timer");
    }
#ifdef _PRE_WLAN_FEATURE_STA_PM
    if (pst_hmac_vap->st_ps_sw_timer.en_is_registerd == OAL_TRUE) {
        frw_immediate_destroy_timer(&(pst_hmac_vap->st_ps_sw_timer));
        pst_hmac_vap->st_ps_sw_timer.en_is_registerd = OAL_FALSE;
        OAM_ERROR_LOG0(0, 0, "hmac_vap_clear_timer st_ps_sw_timer");
    }
#endif
}
static void hmac_process_vap_init_fail(mac_vap_stru *vap, hmac_vap_stru *hmac_vap,
                                       oal_uint8 vap_id, oal_uint32 ret)
{
    OAM_WARNING_LOG1(vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::hmac_vap_init failed[%d].}", ret);
    if (hmac_vap->st_vap_base_info.pst_mib_info != NULL) {
        oal_mem_free_m(hmac_vap->st_vap_base_info.pst_mib_info, OAL_TRUE);
    }
#ifdef _PRE_WLAN_FEATURE_VOWIFI
    if (hmac_vap->st_vap_base_info.pst_vowifi_cfg_param != NULL) {
        oal_mem_free_m(hmac_vap->st_vap_base_info.pst_vowifi_cfg_param, OAL_TRUE);
        hmac_vap->st_vap_base_info.pst_vowifi_cfg_param = NULL;
    }
#endif
    /* 异常处理，释放内存 */
    mac_res_free_mac_vap(vap_id);
}

oal_uint32 hmac_config_add_vap(mac_vap_stru *pst_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    mac_device_stru *pst_dev = OAL_PTR_NULL;
    oal_uint8 uc_vap_id;
    mac_cfg_add_vap_param_stru *pst_param = (mac_cfg_add_vap_param_stru *)puc_param;

    if (oal_unlikely((pst_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_CFG, "{param null,vap=%x param=%x.}", (uintptr_t)pst_vap, (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dev = mac_res_get_dev(pst_vap->uc_device_id);

    if (pst_param->en_p2p_mode == WLAN_P2P_CL_MODE) {
        return hmac_add_p2p_cl_vap(pst_vap, us_len, puc_param);
    }

    if (oal_unlikely(pst_dev == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::pst_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* VAP个数判断 */
    ul_ret = hmac_config_check_vap_num(pst_dev, pst_param);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::check_vap_num failed[%d].}", ul_ret);
        return ul_ret;
    }
    if (oal_net_dev_priv(pst_param->pst_net_dev) != OAL_PTR_NULL) {
        oam_warning_log0(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::vap created.}");
        return OAL_SUCC;
    }
    /* 从资源池申请hmac vap */
    /*lint -e413*/
    ul_ret = mac_res_alloc_hmac_vap(&uc_vap_id, OAL_OFFSET_OF(hmac_vap_stru, st_vap_base_info));
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{mac_res_alloc_hmac_vap failed[%d].}", ul_ret);
        return ul_ret;
    }
    /*lint +e413*/
    /* 从资源池获取新申请到的hmac vap */
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::pst_hmac_vap null.vap_id=%d}", uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_param->uc_vap_id = uc_vap_id;

    hmac_vap_clear_timer(pst_hmac_vap);

    /* 初始清0 */
    memset_s(pst_hmac_vap, OAL_SIZEOF(hmac_vap_stru), 0, OAL_SIZEOF(hmac_vap_stru));

    /* 初始化HMAC VAP */
    ul_ret = hmac_vap_init(pst_hmac_vap, pst_dev->uc_chip_id, pst_dev->uc_device_id, uc_vap_id, pst_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        hmac_process_vap_init_fail(pst_vap, pst_hmac_vap, uc_vap_id, ul_ret);
        return ul_ret;
    }

    /* 设置反挂的net_device指针 */
#ifdef _PRE_WLAN_FEATURE_P2P
    if (pst_param->en_p2p_mode == WLAN_P2P_DEV_MODE) {
        /* p2p0 DEV 模式vap，采用pst_p2p0_net_device 成员指向对应的net_device */
        pst_hmac_vap->pst_p2p0_net_device = pst_param->pst_net_dev;
        pst_dev->st_p2p_info.uc_p2p0_vap_idx = pst_hmac_vap->st_vap_base_info.uc_vap_id;
    }
#endif
    pst_hmac_vap->pst_net_device = pst_param->pst_net_dev;

    /* 包括'\0' */
    if (memcpy_s(pst_hmac_vap->auc_name, OAL_IF_NAME_SIZE, pst_param->pst_net_dev->name, OAL_IF_NAME_SIZE) != EOK) {
        /* 异常处理，释放内存 */
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::memcpy failed.}");
    }

    /* 将申请到的mac_vap空间挂到net_device ml_priv指针上去 */
    oal_net_dev_priv(pst_param->pst_net_dev) = &pst_hmac_vap->st_vap_base_info;

    /* 申请hmac组播用户 */
    hmac_user_add_multi_user(&(pst_hmac_vap->st_vap_base_info), &pst_param->us_muti_user_id);
    mac_vap_set_multi_user_idx(&(pst_hmac_vap->st_vap_base_info), pst_param->us_muti_user_id);
    mac_device_set_vap_id(pst_dev, &(pst_hmac_vap->st_vap_base_info), uc_vap_id, pst_param->en_vap_mode,
                          pst_param->en_p2p_mode, OAL_TRUE);

    switch (pst_param->en_vap_mode) {
        case WLAN_VAP_MODE_BSS_AP:
#ifdef _PRE_WLAN_FEATURE_UAPSD
            pst_param->bit_uapsd_enable = pst_hmac_vap->st_vap_base_info.st_cap_flag.bit_uapsd;
#endif
            break;
        case WLAN_VAP_MODE_BSS_STA:

            break;
        default:
            return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(&pst_hmac_vap->st_vap_base_info, WLAN_CFGID_ADD_VAP, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        /* 此处回退有误，需要对应mac_device_set_vap_id，做回退操作 */
        mac_device_set_vap_id(pst_dev, &(pst_hmac_vap->st_vap_base_info), uc_vap_id, pst_param->en_vap_mode,
                              pst_param->en_p2p_mode, OAL_FALSE);

        hmac_user_del_multi_user(&(pst_hmac_vap->st_vap_base_info));

        /* 异常处理，释放内存 */
        oal_mem_free_m(pst_hmac_vap->st_vap_base_info.pst_mib_info, OAL_TRUE);
#ifdef _PRE_WLAN_FEATURE_VOWIFI
        /* 释放vowifi 申请的内存 */
        if (pst_hmac_vap->st_vap_base_info.pst_vowifi_cfg_param != OAL_PTR_NULL) {
            oal_mem_free_m(pst_hmac_vap->st_vap_base_info.pst_vowifi_cfg_param, OAL_TRUE);
            pst_hmac_vap->st_vap_base_info.pst_vowifi_cfg_param = OAL_PTR_NULL;
        }
#endif

        mac_res_free_mac_vap(uc_vap_id);

        oal_net_dev_priv(pst_param->pst_net_dev) = OAL_PTR_NULL;

        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::alloc_event failed[%d].}", ul_ret);
        return ul_ret;
    }

    if (IS_P2P_GO(&pst_hmac_vap->st_vap_base_info)) {
        hmac_config_set_max_user(&pst_hmac_vap->st_vap_base_info, 0, WLAN_P2P_GO_ASSOC_USER_MAX_NUM_SPEC);
        mac_vap_disable_amsdu_ampdu(&(pst_hmac_vap->st_vap_base_info));
    }
    /* 设置帧过滤 */
    hmac_set_rx_filter_value(&pst_hmac_vap->st_vap_base_info);

    oam_warning_log3(uc_vap_id, OAM_SF_ANY, "{hmac_config_add_vap::SUCC!vap_mode[%d],p2p_mode[%d]},multi user idx[%d]",
                     pst_param->en_vap_mode, pst_param->en_p2p_mode, pst_vap->us_multi_user_idx);

    return OAL_SUCC;
}


oal_void hmac_config_del_scaning_flag(mac_vap_stru *pst_mac_vap)
{
    hmac_device_stru *pst_hmac_device;
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_del_scaning_flag::pst_hmac_device is null, dev_id[%d].}",
                         pst_mac_vap->uc_device_id);
        return;
    }

    if (pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.uc_vap_id == pst_mac_vap->uc_vap_id) {
        pst_hmac_device->st_scan_mgmt.en_is_scanning = OAL_FALSE;
    }
}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OAL_STATIC oal_uint32 hmac_del_vap_post_process(mac_device_stru *pst_device, mac_vap_stru *pst_vap)
{
    oal_uint32 ul_pm_close_ret;
    hmac_device_stru *pst_hmac_device = hmac_res_get_mac_dev(pst_vap->uc_device_id);

    if (pst_device->uc_vap_num != 0) {
        return OAL_SUCC;
    }

    if (oal_likely(pst_hmac_device != OAL_PTR_NULL)) {
        hmac_scan_clean_scan_record(&(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt));
    } else {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_config_del_vap::pst_hmac_device[%d] null!}", pst_vap->uc_device_id);
    }

    // 如果WIFI sta，wlan0 stop，下电
    if (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
#ifdef _PRE_WLAN_NARROW_BAND
        if (g_hitalk_status) {
            ul_pm_close_ret = hitalk_pm_close();
            if (ul_pm_close_ret == OAL_SUCC) {
                g_hitalk_status = 0;
            }
        } else
#endif
        {
            ul_pm_close_ret = wlan_pm_close();
        }

        if (ul_pm_close_ret != OAL_ERR_CODE_FOBID_CLOSE_DEVICE) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY,
                             "{hmac_del_vap_post_process::hmac_config_host_dev_exit_etc! pst_hmac_device[%d]}",
                             pst_vap->uc_device_id);
            hmac_config_host_dev_exit(pst_vap);
        }
    }
    return OAL_SUCC;
}
#endif


OAL_STATIC oal_void hmac_del_vap_destroy_timer(hmac_vap_stru *pst_hmac_vap)
{
    if (pst_hmac_vap->st_mgmt_timer.en_is_registerd == OAL_TRUE) {
        frw_immediate_destroy_timer(&(pst_hmac_vap->st_mgmt_timer));
    }
    if (pst_hmac_vap->st_scan_timeout.en_is_registerd == OAL_TRUE) {
        frw_immediate_destroy_timer(&(pst_hmac_vap->st_scan_timeout));
    }
    if (pst_hmac_vap->st_40m_recovery_timer.en_is_registerd == OAL_TRUE) {
        frw_immediate_destroy_timer(&(pst_hmac_vap->st_40m_recovery_timer));
    }
#ifdef _PRE_WLAN_FEATURE_STA_PM
    if (pst_hmac_vap->st_ps_sw_timer.en_is_registerd == OAL_TRUE) {
        frw_immediate_destroy_timer(&(pst_hmac_vap->st_ps_sw_timer));
    }
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    if (pst_hmac_vap->st_ftm_timer.en_is_registerd == OAL_TRUE) {
        frw_immediate_destroy_timer(&(pst_hmac_vap->st_ftm_timer));
    }
#endif
}
static void hmac_vap_edca_disable(hmac_vap_stru *hmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_vap->uc_edca_opt_flag_ap = 0;
        frw_immediate_destroy_timer(&(hmac_vap->st_edca_opt_timer));
    } else if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_vap->uc_edca_opt_flag_sta = 0;
    }
#endif
}
static void hmac_net_dev_reset(hmac_vap_stru *hmac_vap,
                               mac_cfg_del_vap_param_stru *del_vap_param)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    if (del_vap_param->en_p2p_mode == WLAN_P2P_DEV_MODE) {
        /* 针对p2p0,需要删除hmac 中对应的p2p0 netdevice 指针 */
        hmac_vap->pst_p2p0_net_device = OAL_PTR_NULL;
    }
#endif
    hmac_vap->pst_net_device = OAL_PTR_NULL;
}

oal_uint32 hmac_config_del_vap(mac_vap_stru *pst_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    mac_device_stru *pst_device = OAL_PTR_NULL;
    mac_cfg_del_vap_param_stru *pst_del_vap_param = (mac_cfg_del_vap_param_stru *)puc_param;

    if (oal_unlikely((pst_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_config_del_vap::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_vap::mac_res_get_hmac_vap failed.}");
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_SAE
    oal_cancel_work_sync(&(pst_hmac_vap->st_sae_report_ext_auth_worker));
#endif

#ifdef _PRE_WLAN_FEATURE_P2P
    if (pst_vap->en_p2p_mode == WLAN_P2P_CL_MODE) {
        return hmac_del_p2p_cl_vap(pst_vap, us_len, puc_param);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_DFR
    /* can't return when dfr process! */
    if ((pst_vap->en_vap_state != MAC_VAP_STATE_INIT) && (g_st_dfr_info.bit_device_reset_process_flag != OAL_TRUE))
#else
    if (pst_vap->en_vap_state != MAC_VAP_STATE_INIT)
#endif
    {
        oam_warning_log2(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_vap::deleting vap failed. \
            vap state not INIT, vap_state=%d, vap_mode=%d}",
                         pst_vap->en_vap_state, pst_vap->en_vap_mode);
        return OAL_FAIL;
    }

    hmac_vap_edca_disable(pst_hmac_vap);

    /* 如果是配置VAP, 去注册配置vap对应的net_device, 释放，返回 */
    if (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        /* 在注销netdevice之前先将指针赋为空 */
        oal_net_device_stru *pst_net_device = pst_hmac_vap->pst_net_device;
        pst_hmac_vap->pst_net_device = OAL_PTR_NULL;
        oal_smp_mb();
        oal_net_unregister_netdev(pst_net_device);

        mac_res_free_mac_vap(pst_hmac_vap->st_vap_base_info.uc_vap_id);
        return OAL_SUCC;
    }

    /* 业务vap net_device已在WAL释放，此处置为null */
    hmac_net_dev_reset(pst_hmac_vap, pst_del_vap_param);

    if (pst_hmac_vap->puc_asoc_req_ie_buff != OAL_PTR_NULL) {
        oal_mem_free_m(pst_hmac_vap->puc_asoc_req_ie_buff, OAL_TRUE);
        pst_hmac_vap->puc_asoc_req_ie_buff = OAL_PTR_NULL;
        pst_hmac_vap->ul_asoc_req_ie_len = 0;
    }

    /* 业务vap已删除，从device上去掉 */
    pst_device = mac_res_get_dev(pst_vap->uc_device_id);
    if (pst_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_vap::mac_res_get_dev failed.}");
        return OAL_FAIL;
    }

    /* 清理所有的timer */
    hmac_del_vap_destroy_timer(pst_hmac_vap);

    /* 删除vap时删除TCP ACK的队列 */
#ifdef _PRE_WLAN_TCP_OPT
    hmac_tcp_opt_deinit_list(pst_hmac_vap);
#endif
    {
        hmac_user_del_multi_user(pst_vap);
    }

    /* 释放pmksa */
    ul_ret = hmac_config_flush_pmksa(pst_vap, us_len, puc_param);

    hmac_config_del_scaning_flag(pst_vap);

    mac_vap_exit(&(pst_hmac_vap->st_vap_base_info));

    /* 最后1个vap删除时，清除device级带宽信息 */
    if (pst_device->uc_vap_num == 0) {
        pst_device->en_40MHz_intol_bit_recd = OAL_FALSE;
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    if (pst_device->uc_vap_num == 0) {
#if (!defined(_PRE_PRODUCT_ID_HI110X_HOST))
        /* 1102 wlan0网络设备一直存在 */
        pst_device->st_p2p_info.pst_primary_net_device = OAL_PTR_NULL;
#endif
    }
#endif

    mac_res_free_mac_vap(pst_hmac_vap->st_vap_base_info.uc_vap_id);

    /***************************************************************************
                          抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_vap, WLAN_CFGID_DEL_VAP,
                                    us_len,
                                    puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_vap::send_event failed[%d].}", ul_ret);
        // 不退出，保证Devce挂掉的情况下可以下电。
    }

    oam_warning_log4(pst_vap->uc_vap_id, OAM_SF_ANY,
                     "{hmac_config_del_vap::Del succ.vap_mode[%d], p2p_mode[%d], multi user idx[%d], uc_vap_num[%d]}",
                     pst_vap->en_vap_mode, pst_del_vap_param->en_p2p_mode,
                     pst_vap->us_multi_user_idx, pst_device->uc_vap_num);

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        // 在wal_dfx.c文件中标记开始恢复
        return OAL_SUCC;
    }
#endif  // _PRE_WLAN_FEATURE_DFR

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    ul_ret = hmac_del_vap_post_process(pst_device, pst_vap);
#endif

    return ul_ret;
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32 hmac_config_def_chan(mac_vap_stru *pst_mac_vap)
{
    oal_uint8 uc_channel;
    mac_cfg_mode_param_stru st_param;

    if (((pst_mac_vap->st_channel.en_band == WLAN_BAND_BUTT) ||
         (pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_BUTT) ||
         (pst_mac_vap->en_protocol == WLAN_PROTOCOL_BUTT))
        && (!IS_P2P_GO(pst_mac_vap))) {
        st_param.en_band = WLAN_BAND_2G;
        st_param.en_bandwidth = WLAN_BAND_WIDTH_20M;
        st_param.en_protocol = WLAN_HT_MODE;
        hmac_config_set_mode(pst_mac_vap, OAL_SIZEOF(st_param), (oal_uint8 *)&st_param);
    }

    if ((pst_mac_vap->st_channel.uc_chan_number == 0) && (!IS_P2P_GO(pst_mac_vap))) {
        pst_mac_vap->st_channel.uc_chan_number = 6; /* 主20MHz信道号为6 */
        uc_channel = pst_mac_vap->st_channel.uc_chan_number;
        hmac_config_set_freq(pst_mac_vap, OAL_SIZEOF(oal_uint32), &uc_channel);
    }

    return OAL_SUCC;
}
#endif
OAL_STATIC oal_void hmac_set_mib_cap_for_join(mac_vap_stru *pst_mac_vap)
{
    /* 入网优化，不同频段下的能力不一样 */
    if (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) {
        mac_mib_set_ShortPreambleOptionImplemented(pst_mac_vap, WLAN_LEGACY_11B_MIB_SHORT_PREAMBLE);
        mac_mib_set_SpectrumManagementRequired(pst_mac_vap, OAL_FALSE);
    } else {
        mac_mib_set_ShortPreambleOptionImplemented(pst_mac_vap, WLAN_LEGACY_11B_MIB_LONG_PREAMBLE);
        mac_mib_set_SpectrumManagementRequired(pst_mac_vap, OAL_TRUE);
    }
}
OAL_STATIC oal_void hmac_vap_change_state_for_ap(hmac_vap_stru *pst_hmac_vap,
    mac_device_stru *pst_mac_device)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 设置AP侧状态机为 UP */
    hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_UP);
#else
#if defined(_PRE_WLAN_FEATURE_20_40_80_COEXIST)
    
    if (hmac_device_in_init_scan(pst_mac_device) == OAL_FALSE)
#endif
    {
        /* 设置AP侧状态机为 UP */
        hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_UP);
    }
#endif
}
OAL_STATIC oal_void hmac_vap_change_state_for_sta(hmac_vap_stru *pst_hmac_vap,
    mac_vap_stru *pst_mac_vap, mac_cfg_start_vap_param_stru *pst_start_vap_param)
{
#ifdef _PRE_WLAN_FEATURE_P2P
#ifndef WIN32
    /* 启动p2p device时，vap_param的p2p模式和mac_vap的p2p模式不同 */
    /* :hishare从p2p device转化为gc，若p2p device启动时置fake up状态，会导致不配置vap_ctrl寄存器，gc mac地址无效化 */
    if (pst_mac_vap->en_p2p_mode == WLAN_P2P_DEV_MODE) {
        hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
    }

    /* p2p0和p2p-p2p0 共VAP 结构，对于p2p cl不用修改vap 状态 */
    /* 上层hishare下，优化了不执行扫描的话，直接创建p2p cl会在init状态，此时需要置fake up */
    else
#endif
        if ((pst_start_vap_param->en_p2p_mode != WLAN_P2P_CL_MODE)
            || (pst_mac_vap->en_vap_state == MAC_VAP_STATE_INIT))
#endif
        {
            hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
        }
}

oal_uint32 hmac_config_start_vap(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint8 *puc_ssid = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    mac_cfg_start_vap_param_stru *pst_start_vap_param = (mac_cfg_start_vap_param_stru *)puc_param;

    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_config_start_vap::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::the vap has been deleted.}");

        return OAL_FAIL;
    }

    if ((pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP) ||
        (pst_mac_vap->en_vap_state == MAC_VAP_STATE_AP_WAIT_START) ||
        (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_FAKE_UP)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{state=%d,dupl start again}", pst_mac_vap->en_vap_state);
        return OAL_SUCC;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        puc_ssid = pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11DesiredSSID;
        /* P2P GO 创建后，未设置ssid 信息，设置为up 状态不需要检查ssid 参数 */
        if ((OAL_STRLEN((oal_int8 *)puc_ssid) == 0) && (!IS_P2P_GO(pst_mac_vap))) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::ssid length=0.}");
            return OAL_FAIL; /* 没设置SSID，则不启动VAP */
        }

        /* 设置AP侧状态机为 WAIT_START */
        hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_AP_WAIT_START);

        if (IS_LEGACY_VAP(&(pst_hmac_vap->st_vap_base_info))) {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
            hmac_config_def_chan(pst_mac_vap);
#else
#if defined(_PRE_WLAN_FEATURE_20_40_80_COEXIST)
            if (hmac_init_scan_try(pst_mac_device, pst_mac_vap, MAC_TRY_INIT_SCAN_VAP_UP) == OAL_SUCC) {
                return OAL_SUCC;
            }
#endif
#endif
        }

        /* 这里 en_status 等于 MAC_CHNL_AV_CHK_NOT_REQ(无需检测) 或者 MAC_CHNL_AV_CHK_COMPLETE(检测完成) */
        /* 检查协议 频段 带宽是否设置 */
        if (((pst_mac_vap->st_channel.en_band == WLAN_BAND_BUTT) ||
             (pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_BUTT) ||
             (pst_mac_vap->en_protocol == WLAN_PROTOCOL_BUTT))) {
            if (IS_P2P_GO(pst_mac_vap)) {
                /* wpa_supplicant 会先设置vap up， 此时并未给vap 配置信道、带宽和协议模式信息，
                   wpa_supplicant 在cfg80211_start_ap 接口配置GO 信道、带宽和协议模式信息，
                   故此处如果没有设置信道、带宽和协议模式，直接返回成功，不返回失败。 */
                hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_INIT);
                oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                                 "{hmac_config_start_vap::set band bw protocol first.band[%d],bw[%d], protocol[%d]}",
                                 pst_mac_vap->st_channel.en_band,
                                 pst_mac_vap->st_channel.en_bandwidth, pst_mac_vap->en_protocol);
                return OAL_SUCC;
            } else {
                hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_INIT);
                OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::set bw protocol first.}");
                return OAL_FAIL;
            }
        }

        /* 检查信道号是否设置 */
        if ((pst_mac_vap->st_channel.uc_chan_number == 0) && (!IS_P2P_GO(pst_mac_vap))) {
            hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_INIT);
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::set channel number first.}");
            return OAL_FAIL;
        }

        /* 设置bssid */
        mac_vap_set_bssid(pst_mac_vap, pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);

        /* 入网优化，不同频段下的能力不一样 */
        hmac_set_mib_cap_for_join(pst_mac_vap);
        hmac_vap_change_state_for_ap(pst_hmac_vap, pst_mac_device);
    } else if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_vap_change_state_for_sta(pst_hmac_vap, pst_mac_vap, pst_start_vap_param);
    } else {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{Do not surport mode[%d].}", pst_mac_vap->en_vap_mode);
    }

    mac_vap_init_rates(pst_mac_vap);
    ul_ret = hmac_config_start_vap_event(pst_mac_vap, pst_start_vap_param->en_mgmt_rate_init_flag);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_start_vap::hmac_config_send_event failed[%d].}", ul_ret);
        return ul_ret;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    /* 异常恢复后需要告诉对端"已去关联"的消息 */
    if (g_st_dfr_info.bit_user_disconnect_flag == OAL_TRUE) {
        g_st_dfr_info.bit_user_disconnect_flag = OAL_FALSE;
        hmac_mgmt_send_disassoc_frame(pst_mac_vap, BROADCAST_MACADDR, MAC_UNSPEC_REASON, OAL_FALSE);
    }
#endif  // _PRE_WLAN_FEATURE_DFR
    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::ok. vap mode[%d],p2p mode[%d]}",
                     pst_mac_vap->en_vap_mode, pst_mac_vap->en_p2p_mode);
    return OAL_SUCC;
}


oal_uint32 hmac_config_sta_update_rates(mac_vap_stru *pst_mac_vap, mac_cfg_mode_param_stru *pst_cfg_mode,
                                        mac_bss_dscr_stru *pst_bss_dscr)
{
    oal_uint32 ul_ret;
    mac_device_stru *pst_mac_device;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_sta_update_rates::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_sta_update_rates::the vap has been deleted.}");
        return OAL_FAIL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_sta_update_rates::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_cfg_mode->en_protocol >= WLAN_HT_MODE) {
        pst_hmac_vap->en_tx_aggr_on = OAL_TRUE;
        pst_hmac_vap->en_amsdu_active = OAL_TRUE;
    } else {
        pst_hmac_vap->en_tx_aggr_on = OAL_FALSE;
        pst_hmac_vap->en_amsdu_active = OAL_FALSE;
    }

    mac_vap_init_by_protocol(pst_mac_vap, pst_cfg_mode->en_protocol);
    pst_mac_vap->st_channel.en_band = pst_cfg_mode->en_band;
    pst_mac_vap->st_channel.en_bandwidth = pst_cfg_mode->en_bandwidth;
    mac_sta_init_bss_rates(pst_mac_vap, (oal_void *)pst_bss_dscr);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_start_vap_event(pst_mac_vap, OAL_FALSE);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_sta_update_rates::hmac_config_send_event failed[%d].}", ul_ret);
        mac_vap_init_by_protocol(pst_mac_vap, pst_hmac_vap->st_preset_para.en_protocol);
        pst_mac_vap->st_channel.en_band = pst_hmac_vap->st_preset_para.en_band;
        pst_mac_vap->st_channel.en_bandwidth = pst_hmac_vap->st_preset_para.en_bandwidth;
        return ul_ret;
    }

    return OAL_SUCC;
}
OAL_STATIC oal_void hmac_process_vap_already_down(mac_cfg_down_vap_param_stru *pst_param,
    mac_vap_stru *pst_mac_vap)
{
    /* 设置net_device里flags标志 */
    if ((pst_param->pst_net_dev != OAL_PTR_NULL) &&
        (oal_netdevice_flags(pst_param->pst_net_dev) & OAL_IFF_RUNNING)) {
        oal_netdevice_flags(pst_param->pst_net_dev) &= (~OAL_IFF_RUNNING);
    }
    oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::vap already down.}");
}

oal_uint32 hmac_config_down_vap(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    mac_cfg_down_vap_param_stru *pst_param = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_dlist_head_stru *pst_entry = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_user_list_head = OAL_PTR_NULL;
    mac_user_stru *pst_user_tmp = OAL_PTR_NULL;
    hmac_user_stru *pst_hmac_user_tmp = OAL_PTR_NULL;
    oal_bool_enum_uint8 en_is_protected = OAL_FALSE;
    mac_user_stru *pst_multi_user = OAL_PTR_NULL;

    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_config_down_vap::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_param = (mac_cfg_down_vap_param_stru *)puc_param;

    if (pst_param->pst_net_dev == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::pst_param->pst_net_dev is null.}");
        return OAL_SUCC;
    }

    /* 如果vap已经在down的状态，直接返回 */
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_INIT) {
        /* 设置net_device里flags标志 */
        hmac_process_vap_already_down(pst_param, pst_mac_vap);
        return OAL_SUCC;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_multi_user = mac_res_get_mac_user(pst_mac_vap->us_multi_user_idx);
    if (pst_multi_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{multi_user[%d] null.}", pst_mac_vap->us_multi_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 考虑与数据面互斥，需要加锁保护 */
    /* 设置net_device里flags标志 */
    oal_netdevice_flags(pst_param->pst_net_dev) &= (~OAL_IFF_RUNNING);

    /* 遍历vap下所有用户, 删除用户 */
    pst_user_list_head = &(pst_mac_vap->st_mac_user_list_head);
    for (pst_entry = pst_user_list_head->pst_next; pst_entry != pst_user_list_head;) {
        pst_user_tmp = oal_dlist_get_entry(pst_entry, mac_user_stru, st_user_dlist);
        pst_hmac_user_tmp = mac_res_get_hmac_user(pst_user_tmp->us_assoc_id);
        if (pst_hmac_user_tmp == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_WPA, "{hmac_user null.idx:%u}", pst_user_tmp->us_assoc_id);
            continue;
        }

        /* 指向双向链表下一个 */
        pst_entry = pst_entry->pst_next;

        /* 管理帧加密是否开启 */
        en_is_protected = pst_user_tmp->st_cap_info.bit_pmf_active;

        /* 发去关联帧 */
        hmac_mgmt_send_disassoc_frame(pst_mac_vap, pst_user_tmp->auc_user_mac_addr, MAC_DISAS_LV_SS, en_is_protected);
        /* 删除用户 */
        hmac_user_del(pst_mac_vap, pst_hmac_user_tmp);
    }

    /* VAP下user链表应该为空 */
    if (oal_dlist_is_empty(&pst_mac_vap->st_mac_user_list_head) == OAL_FALSE) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::user_list_head is not empty.}");
        return OAL_FAIL;
    }

    /* sta模式时 将desired ssid MIB项置空，并清空配置协议标志 */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        pst_hmac_vap->bit_sta_protocol_cfg = OAL_SWITCH_OFF;
        if (pst_mac_vap->pst_mib_info != OAL_PTR_NULL) {
            memset_s(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11DesiredSSID, WLAN_SSID_MAX_LEN,
                     0, WLAN_SSID_MAX_LEN);
        } else {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::mib pointer is NULL!!}");
        }
    } else if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
#if defined(_PRE_WLAN_FEATURE_20_40_80_COEXIST)
        {
            hmac_device_stru *pst_hmac_dev = hmac_res_get_mac_dev(pst_mac_device->uc_device_id);

            if (pst_hmac_dev && (hmac_calc_up_and_wait_vap(pst_hmac_dev) <= 1)) {
                hmac_init_scan_cancel_timer(pst_hmac_dev);
                pst_hmac_dev->en_in_init_scan = OAL_FALSE;
            }
        }
#endif
    }

    /***************************************************************************
                         抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap,
                                    WLAN_CFGID_DOWN_VAP,
                                    us_len,
                                    puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::send_event failed[%d].}", ul_ret);

        return ul_ret;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    if (pst_param->en_p2p_mode == WLAN_P2P_CL_MODE) {
        mac_vap_state_change(pst_mac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
    } else
#endif
    {
        mac_vap_state_change(pst_mac_vap, MAC_VAP_STATE_INIT);
    }
    pst_hmac_vap->en_auth_mode = WLAN_WITP_AUTH_OPEN_SYSTEM;

    hmac_set_rx_filter_value(pst_mac_vap);

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_down_vap:: SUCC! Now remaining %d vaps in device[%d].}",
                     pst_mac_device->uc_vap_num, pst_mac_device->uc_device_id);
    return OAL_SUCC;
}


oal_uint32 hmac_config_get_mode(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    mac_cfg_mode_param_stru *pst_prot_param;

    pst_prot_param = (mac_cfg_mode_param_stru *)puc_param;

    pst_prot_param->en_protocol = pst_mac_vap->en_protocol;
    pst_prot_param->en_band = pst_mac_vap->st_channel.en_band;
    pst_prot_param->en_bandwidth = pst_mac_vap->st_channel.en_bandwidth;

    *pus_len = OAL_SIZEOF(mac_cfg_mode_param_stru);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_config_check_mode_param(mac_device_stru *pst_mac_device,
                                                   mac_cfg_mode_param_stru *pst_prot_param)
{
    /* 根据device能力对参数进行检查 */
    switch (pst_prot_param->en_protocol) {
        case WLAN_LEGACY_11A_MODE:
        case WLAN_LEGACY_11B_MODE:
        case WLAN_LEGACY_11G_MODE:
        case WLAN_MIXED_ONE_11G_MODE:
        case WLAN_MIXED_TWO_11G_MODE:
            break;

        case WLAN_HT_MODE:
        case WLAN_HT_ONLY_MODE:
        case WLAN_HT_11G_MODE:
            if (pst_mac_device->en_protocol_cap < WLAN_PROTOCOL_CAP_HT) {
                /* 设置11n协议，但device不支持HT模式 */
                oam_warning_log2(0, OAM_SF_CFG,
                                 "{hmac_config_check_mode_param::not support HT mode,protocol=%d en_protocol_cap=%d.}",
                                 pst_prot_param->en_protocol, pst_mac_device->en_protocol_cap);
                return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
            }
            break;

        case WLAN_VHT_MODE:
        case WLAN_VHT_ONLY_MODE:
            if (pst_mac_device->en_protocol_cap < WLAN_PROTOCOL_CAP_VHT) {
                /* 设置11ac协议，但device不支持VHT模式 */
                oam_warning_log2(0, OAM_SF_CFG,
                                 "{hmac_config_check_mode_param::not support VHT mode,protocol=%d en_protocol_cap=%d.}",
                                 pst_prot_param->en_protocol, pst_mac_device->en_protocol_cap);
                return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
            }
            break;

        default:
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_check_mode_param::mode param does not in the list.}");
            break;
    }

    if ((pst_prot_param->en_bandwidth > WLAN_BAND_WIDTH_40MINUS) &&
        (pst_mac_device->en_bandwidth_cap < WLAN_BW_CAP_80M)) {
        /* 设置80M带宽，但device能力不支持80M，返回错误码 */
        oam_warning_log2(0, OAM_SF_CFG,
                         "{hmac_config_check_mode_param::not support 80MHz bandwidth,protocol=%d en_protocol_cap=%d.}",
                         pst_prot_param->en_bandwidth, pst_mac_device->en_bandwidth_cap);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    if ((pst_prot_param->en_band == WLAN_BAND_5G) && (pst_mac_device->en_band_cap == WLAN_BAND_CAP_2G)) {
        /* 设置5G频带，但device不支持5G */
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_check_mode_param::not support 5G,protocol=%d protocol_cap=%d.}",
                         pst_prot_param->en_band, pst_mac_device->en_band_cap);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    } else if ((pst_prot_param->en_band == WLAN_BAND_2G) && (pst_mac_device->en_band_cap == WLAN_BAND_CAP_5G)) {
        /* 设置2G频带，但device不支持2G */
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_check_mode_param::not support 2G,protocol=%d protocol_cap=%d.}",
                         pst_prot_param->en_band, pst_mac_device->en_band_cap);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_config_set_mode_check_bandwith(wlan_channel_bandwidth_enum_uint8 en_bw_device,
                                                          wlan_channel_bandwidth_enum_uint8 en_bw_config)
{
    /* 要配置带宽是20M */
    if (en_bw_config == WLAN_BAND_WIDTH_20M) {
        return OAL_SUCC;
    }

    /* 要配置带宽与首次配置带宽相同 */
    if (en_bw_device == en_bw_config) {
        return OAL_SUCC;
    }

    switch (en_bw_device) {
        case WLAN_BAND_WIDTH_80PLUSPLUS:
        case WLAN_BAND_WIDTH_80PLUSMINUS:
            if (en_bw_config == WLAN_BAND_WIDTH_40PLUS) {
                return OAL_SUCC;
            }
            break;

        case WLAN_BAND_WIDTH_80MINUSPLUS:
        case WLAN_BAND_WIDTH_80MINUSMINUS:
            if (en_bw_config == WLAN_BAND_WIDTH_40MINUS) {
                return OAL_SUCC;
            }
            break;

        default:
            break;
    }

    return OAL_FAIL;
}


oal_uint32 hmac_config_set_mode(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_mode_param_stru *pst_prot_param = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    mac_device_stru *pst_mac_device;

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::pst_mac_device null.}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    /* 设置模式时，device下必须至少有一个vap */
    if (pst_mac_device->uc_vap_num == 0) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::no vap in device.}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_prot_param = (mac_cfg_mode_param_stru *)puc_param;

    /* 检查配置参数是否在device能力内 */
    ul_ret = hmac_config_check_mode_param(pst_mac_device, pst_prot_param);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::check_mode failed[%d]}", ul_ret);
        return ul_ret;
    }

    /* device已经配置时，需要校验下频段、带宽是否一致 */
    if ((pst_mac_device->en_max_bandwidth != WLAN_BAND_WIDTH_BUTT) && (!MAC_DBAC_ENABLE(pst_mac_device))
        && (pst_mac_device->uc_vap_num > 1)) {
        if (pst_mac_device->en_max_band != pst_prot_param->en_band) {
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_set_mode::previous vap band[%d] mismatch with [%d].}",
                             pst_mac_device->en_max_band, pst_prot_param->en_band);
            return OAL_FAIL;
        }

        ul_ret = hmac_config_set_mode_check_bandwith(pst_mac_device->en_max_bandwidth, pst_prot_param->en_bandwidth);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_set_mode::set_mode_check_bw failed[%d],previous vap bw[%d, current[%d].}",
                             ul_ret, pst_mac_device->en_max_bandwidth, pst_prot_param->en_bandwidth);
            return ul_ret;
        }
    }

    if (pst_prot_param->en_protocol >= WLAN_HT_MODE) {
        pst_hmac_vap->en_tx_aggr_on = OAL_TRUE;
        pst_hmac_vap->en_amsdu_active = OAL_TRUE;
    } else {
        pst_hmac_vap->en_tx_aggr_on = OAL_FALSE;
        pst_hmac_vap->en_amsdu_active = OAL_FALSE;
    }

    /* 更新STA协议配置标志位 */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        pst_hmac_vap->bit_sta_protocol_cfg = OAL_SWITCH_ON;
        pst_hmac_vap->st_preset_para.en_protocol = pst_prot_param->en_protocol;
        pst_hmac_vap->st_preset_para.en_bandwidth = pst_prot_param->en_bandwidth;
        pst_hmac_vap->st_preset_para.en_band = pst_prot_param->en_band;
    }

    /* 记录协议模式, band, bandwidth到mac_vap下 */
    pst_mac_vap->en_protocol = pst_prot_param->en_protocol;
    pst_mac_vap->st_channel.en_band = pst_prot_param->en_band;
    pst_mac_vap->st_channel.en_bandwidth = pst_prot_param->en_bandwidth;
    pst_mac_vap->st_ch_switch_info.en_user_pref_bandwidth = pst_prot_param->en_bandwidth;

#ifdef _PRE_WLAN_FEATURE_TXBF
    if ((pst_prot_param->en_protocol >= WLAN_HT_MODE)
        && (pst_mac_device->bit_su_bfmee == OAL_TRUE)) {
        pst_mac_vap->st_cap_flag.bit_11ntxbf = OAL_TRUE;
    } else {
        pst_mac_vap->st_cap_flag.bit_11ntxbf = OAL_FALSE;
    }
#endif
    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_mode::protocol=%d, band=%d, bandwidth=%d.}", pst_prot_param->en_protocol,
                     pst_mac_vap->st_channel.en_band, pst_mac_vap->st_channel.en_bandwidth);

    /* 根据协议更新vap能力 */
    mac_vap_init_by_protocol(pst_mac_vap, pst_prot_param->en_protocol);

    /* 根据带宽信息更新Mib */
    mac_vap_change_mib_by_bandwidth(pst_mac_vap, pst_prot_param->en_bandwidth);

    /* 更新device的频段及最大带宽信息 */
    if ((pst_mac_device->en_max_bandwidth == WLAN_BAND_WIDTH_BUTT) || (hmac_calc_up_ap_num(pst_mac_device) == 0)) {
        pst_mac_device->en_max_bandwidth = pst_prot_param->en_bandwidth;
        pst_mac_device->en_max_band = pst_prot_param->en_band;
    }

    /***************************************************************************
     抛事件到DMAC层, 配置寄存器
    ***************************************************************************/
    ul_ret = hmac_set_mode_event(pst_mac_vap);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_mac_addr(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    mac_cfg_staion_id_param_stru *pst_station_id_param = OAL_PTR_NULL;
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
#endif
    oal_uint32 ul_ret;

    if (pst_mac_vap->pst_mib_info == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_set_mac_addr::vap->mib_info is NULL !}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    /* P2P 设置MAC 地址mib 值需要区分P2P DEV 或P2P_CL/P2P_GO,P2P_DEV MAC 地址设置到p2p0 MIB 中 */
    pst_station_id_param = (mac_cfg_staion_id_param_stru *)puc_param;
    en_p2p_mode = pst_station_id_param->en_p2p_mode;
    if (en_p2p_mode == WLAN_P2P_DEV_MODE) {
        /* 如果是p2p0 device，则配置MAC 地址到auc_p2p0_dot11StationID 成员中 */
        oal_set_mac_addr(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID,
                         pst_station_id_param->auc_station_id);
    } else
#endif
    {
        /* 设置mib值, Station_ID */
        mac_mib_set_station_id(pst_mac_vap, (oal_uint8)us_len, puc_param);
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_STATION_ID, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_mac_addr::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_concurrent(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_int32 l_value;

    l_value = *((oal_int32 *)puc_param);

    mac_res_set_max_asoc_user((oal_uint16)l_value);

    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_concurrent::l_value=%d.}", l_value);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_HMAC == _PRE_MULTI_CORE_MODE)
    return hmac_config_send_event(pst_mac_vap, WLAN_CFGID_CONCURRENT, us_len, puc_param);
#else
    return OAL_SUCC;
#endif
}


oal_uint32 hmac_config_get_concurrent(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    oal_int32 *pl_value;

    pl_value = (oal_int32 *)puc_param;
    *pl_value = mac_res_get_max_asoc_user();
    *pus_len = OAL_SIZEOF(oal_int32);

    return OAL_SUCC;
}


oal_uint32 hmac_config_get_ssid(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    /* 读取mib值 */
    return mac_mib_get_ssid(pst_mac_vap, (oal_uint8 *)pus_len, puc_param);
}


oal_uint32 hmac_config_set_ssid(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    /* 设置mib值 */
    mac_mib_set_ssid(pst_mac_vap, (oal_uint8)us_len, puc_param);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) /* hi1102-cb set at both side (HMAC to DMAC) */
    return hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SSID, us_len, puc_param);
#else
    return OAL_SUCC;
#endif
}


oal_uint32 hmac_config_set_shpreamble(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /* 设置mib值 */
    mac_mib_set_shpreamble(pst_mac_vap, (oal_uint8)us_len, puc_param);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SHORT_PREAMBLE, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_concurrent::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_get_shpreamble(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    /* 读mib值 */
    return mac_mib_get_shpreamble(pst_mac_vap, (oal_uint8 *)pus_len, puc_param);
}


oal_uint32 hmac_config_set_shortgi20(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_int32 l_value;
    oal_uint32 ul_ret;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    shortgi_cfg_stru shortgi_cfg;

    shortgi_cfg.uc_shortgi_type = SHORTGI_20_CFG_ENUM;
#endif
    l_value = *((oal_int32 *)puc_param);

    if (l_value != 0) {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        shortgi_cfg.uc_enable = OAL_TRUE;
#endif
        pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11ShortGIOptionInTwentyImplemented = OAL_TRUE;
    } else {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        shortgi_cfg.uc_enable = OAL_FALSE;
#endif
        pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11ShortGIOptionInTwentyImplemented = OAL_FALSE;
    }

    /* ======================================================================== */
    /* hi1102-cb : Need to send to Dmac via sdio */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 配置事件的子事件 WLAN_CFGID_SHORTGI 通过新加的接口函数取出关键数据存入skb后通过sdio发出 */
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SHORTGI, SHORTGI_CFG_STRU_LEN, (oal_uint8 *)&shortgi_cfg);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_shortgi20::hmac_config_send_event failed[%u].}", ul_ret);
    }
#endif
    /* ======================================================================== */
    return ul_ret;
}


oal_uint32 hmac_config_set_shortgi40(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_int32 l_value;
    oal_uint32 ul_ret;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    shortgi_cfg_stru shortgi_cfg;

    shortgi_cfg.uc_shortgi_type = SHORTGI_40_CFG_ENUM;
#endif
    l_value = *((oal_int32 *)puc_param);

    if (l_value != 0) {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        shortgi_cfg.uc_enable = OAL_TRUE;
#endif
        mac_mib_set_ShortGIOptionInFortyImplemented(pst_mac_vap, OAL_TRUE);
    } else {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        shortgi_cfg.uc_enable = OAL_FALSE;
#endif
        mac_mib_set_ShortGIOptionInFortyImplemented(pst_mac_vap, OAL_FALSE);
    }

    /* ======================================================================== */
    /* hi1102-cb : Need to send to Dmac via sdio */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 配置事件的子事件 WLAN_CFGID_SHORTGI 通过新加的接口函数取出关键数据存入skb后通过sdio发出 */
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SHORTGI, SHORTGI_CFG_STRU_LEN, (oal_uint8 *)&shortgi_cfg);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_shortgi40::hmac_config_send_event failed[%u].}", ul_ret);
    }
#endif
    /* ======================================================================== */
    return ul_ret;
}


oal_uint32 hmac_config_set_shortgi80(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_int32 l_value;
    oal_uint32 ul_ret;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    shortgi_cfg_stru shortgi_cfg;

    shortgi_cfg.uc_shortgi_type = SHORTGI_40_CFG_ENUM;
#endif

    l_value = *((oal_int32 *)puc_param);

    if (l_value != 0) {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        shortgi_cfg.uc_enable = OAL_TRUE;
#endif
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn80Implemented = OAL_TRUE;
    } else {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        shortgi_cfg.uc_enable = OAL_FALSE;
#endif
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn80Implemented = OAL_FALSE;
    }

    /* ======================================================================== */
    /* hi1102-cb : Need to send to Dmac via sdio */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 配置事件的子事件 WLAN_CFGID_SHORTGI 通过新加的接口函数取出关键数据存入skb后通过sdio发出 */
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SHORTGI, SHORTGI_CFG_STRU_LEN, (oal_uint8 *)&shortgi_cfg);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_shortgi80::hmac_config_send_event failed[%u].}", ul_ret);
    }
#endif
    /* ======================================================================== */
    return ul_ret;
}


oal_uint32 hmac_config_get_shortgi20(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    oal_int32 l_value;

    l_value = pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11ShortGIOptionInTwentyImplemented;

    *((oal_int32 *)puc_param) = l_value;

    *pus_len = OAL_SIZEOF(l_value);

    return OAL_SUCC;
}


oal_uint32 hmac_config_get_shortgi40(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    oal_int32 l_value;

    l_value = (oal_int32)mac_mib_get_ShortGIOptionInFortyImplemented(pst_mac_vap);

    *((oal_int32 *)puc_param) = l_value;

    *pus_len = OAL_SIZEOF(l_value);

    return OAL_SUCC;
}


oal_uint32 hmac_config_get_shortgi80(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    oal_int32 l_value;

    l_value = pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn80Implemented;

    *((oal_int32 *)puc_param) = l_value;

    *pus_len = OAL_SIZEOF(l_value);

    return OAL_SUCC;
}

oal_uint32 hmac_config_get_addr_filter(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_addr_filter::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *((oal_int32 *)puc_param) = pst_hmac_vap->en_addr_filter;
    *pus_len = OAL_SIZEOF(oal_int32);

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_prot_mode(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_int32 l_value;
    oal_uint32 ul_ret;

    l_value = *((oal_int32 *)puc_param);

    if (oal_unlikely(l_value >= WLAN_PROT_BUTT)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_prot_mode::invalid l_value[%d].}",
                         l_value);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_mac_vap->st_protection.en_protection_mode = (oal_uint8)l_value;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PROT_MODE, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_prot_mode::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_get_prot_mode(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    *((oal_int32 *)puc_param) = pst_mac_vap->st_protection.en_protection_mode;
    *pus_len = OAL_SIZEOF(oal_int32);

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_auth_mode(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint8 uc_value;
    hmac_vap_stru *pst_hmac_vap;

    uc_value = *((oal_uint8 *)puc_param);

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_auth_mode::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap->en_auth_mode = uc_value;

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_auth_mode::set auth mode[%d] succ.}",
                     pst_hmac_vap->en_auth_mode);

    return OAL_SUCC;
}


oal_uint32 hmac_config_get_auth_mode(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_auth_mode::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *((oal_int32 *)puc_param) = pst_hmac_vap->en_auth_mode;
    *pus_len = OAL_SIZEOF(oal_int32);

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_max_user(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint32 ul_max_user)
{
    hmac_vap_stru *pst_hmac_vap;

    /*
      此函数是设置vap级别的最大用户数，计算device下配置的总用户数方式不对，同时没有考虑双芯片下实现。
      device下关联用户总数会受到软件总用户数限制，这里不需要去计算device下用户总数限制，回归函数本身只是设置vap最大用户数的初衷
 */
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_max_user::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (ul_max_user >= WLAN_ASSOC_USER_MAX_NUM_SPEC) {
        pst_hmac_vap->us_user_nums_max = WLAN_ASSOC_USER_MAX_NUM_SPEC;
    } else {
        pst_hmac_vap->us_user_nums_max = (oal_uint16)ul_max_user;
    }

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_max_user::vap[%d] us_user_nums_max[%d].}",
                     pst_mac_vap->uc_vap_id, pst_hmac_vap->us_user_nums_max);

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_bintval(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    mac_device_stru *pst_mac_device;
    oal_uint8 uc_vap_idx;
    mac_vap_stru *pst_vap = OAL_PTR_NULL;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_config_set_bintval::mac_res_get_dev fail.device_id = %u}",
                       pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置device下的值 */
    mac_device_set_beacon_interval(pst_mac_device, *((oal_uint32 *)puc_param));

    /* 遍历device下所有vap */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_config_set_bintval::pst_mac_vap(%d) null.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        /* 只有AP VAP需要beacon interval */
        if (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
            /* 设置mib值 */
            mac_mib_set_beacon_period(pst_vap, (oal_uint8)us_len, puc_param);
        }
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_BEACON_INTERVAL, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_bintval::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_get_bintval(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    /* 读取mib值 */
    return mac_mib_get_beacon_period(pst_mac_vap, (oal_uint8 *)pus_len, puc_param);
}


oal_uint32 hmac_config_set_dtimperiod(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /* 设置mib值 */
    mac_mib_set_dtim_period(pst_mac_vap, (oal_uint8)us_len, puc_param);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DTIM_PERIOD, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_bintval::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_get_dtimperiod(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    /* 读取mib值 */
    return mac_mib_get_dtim_period(pst_mac_vap, (oal_uint8 *)pus_len, puc_param);
}


oal_uint32 hmac_config_set_nobeacon(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    oal_uint32 ul_ret;
    oal_int32 l_value;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_nobeacon::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    l_value = *((oal_int32 *)puc_param);
    pst_hmac_vap->en_no_beacon = (oal_uint8)l_value;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_NO_BEACON, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_nobeacon::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_get_nobeacon(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_nobeacon::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *((oal_int32 *)puc_param) = pst_hmac_vap->en_no_beacon;
    *pus_len = OAL_SIZEOF(oal_int32);

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_txchain(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_int32 l_value;
    oal_uint32 ul_ret;
    mac_device_stru *pst_mac_device;

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_txchain::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    l_value = *((oal_int32 *)puc_param);

    mac_device_set_txchain(pst_mac_device, (oal_uint8)l_value);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TX_CHAIN, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_txchain::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_get_txchain(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    mac_device_stru *pst_mac_device;

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_txchain::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *((oal_int32 *)puc_param) = pst_mac_device->uc_tx_chain;
    *pus_len = OAL_SIZEOF(oal_int32);

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_rxchain(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_int32 l_value;
    oal_uint32 ul_ret;
    mac_device_stru *pst_mac_device;

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_rxchain::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    l_value = *((oal_int32 *)puc_param);

    mac_device_set_rxchain(pst_mac_device, (oal_uint8)l_value);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_RX_CHAIN, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_rxchain::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_get_rxchain(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    mac_device_stru *pst_mac_device;

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_rxchain::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *((oal_int32 *)puc_param) = pst_mac_device->uc_rx_chain;
    *pus_len = OAL_SIZEOF(oal_int32);

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_txpower(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_int32 l_value;
    oal_uint32 ul_ret;

    l_value = *((oal_int32 *)puc_param);

    /* 如果上层配置的值为异常值则直接返回,增加求和溢出异常判断。异常情况，l_value超过0xff或小于等于0或l_value大于l_value偏移5 */
    if ((l_value >= 0xff) || (l_value <= 0) || ((l_value + 5) < l_value)) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_set_txpower:l_value[%d] is error", l_value);
        return OAL_FAIL;
    }
    /* 命令配置下来的功率值单位为0.1db,这里转换为1db（10倍功率值单位）保存到vap下,4舍5入 */
    mac_vap_set_tx_power (pst_mac_vap, (oal_uint8)((l_value + 5) / 10));

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TX_POWER, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_txpower::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_get_txpower(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    *((oal_int32 *)puc_param) = pst_mac_vap->uc_tx_power;
    *pus_len = OAL_SIZEOF(oal_int32);

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_freq(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    oal_uint8 uc_channel = *puc_param;
    mac_cfg_channel_param_stru l_channel_param;
    mac_device_stru *pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_freq::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 2.4G支持11ac */
    if (uc_channel <= 14) { /* uc_channel信道数14及以下是2G频段，否则是5G频段 */
        pst_mac_vap->st_channel.en_band = WLAN_BAND_2G;
    } else {
        pst_mac_vap->st_channel.en_band = WLAN_BAND_5G;
    }

    ul_ret = mac_is_channel_num_valid(pst_mac_vap->st_channel.en_band, uc_channel);
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{channel_num_valid[%d] failed[%d].}", uc_channel, ul_ret);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

#ifdef _PRE_WLAN_FEATURE_11D
    /* 信道14特殊处理，只在11b协议模式下有效 */
    if ((uc_channel == 14) && (pst_mac_vap->en_protocol != WLAN_LEGACY_11B_MODE)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{channel-14 only available in 11b,curr protocol=%d.}",
                         pst_mac_vap->en_protocol);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
#endif
    /* gaolin: CAC在初始检查时统一执行 */
    pst_mac_vap->st_channel.uc_chan_number = uc_channel;
    ul_ret = mac_get_channel_idx_from_num(pst_mac_vap->st_channel.en_band, uc_channel,
                                          &(pst_mac_vap->st_channel.uc_idx));
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_get_channel_idx_from_num fail.band[%u]channel[%u]!}",
                         pst_mac_vap->st_channel.en_band, pst_mac_vap->st_channel.uc_idx);
        return ul_ret;
    }

    /* 非DBAC时，首次配置信道时设置到硬件 */
    if ((pst_mac_device->uc_vap_num == 1) || (pst_mac_device->uc_max_channel == 0)) {
        mac_device_get_channel(pst_mac_device, &l_channel_param);
        l_channel_param.uc_channel = uc_channel;
        mac_device_set_channel(pst_mac_device, &l_channel_param);

        /***************************************************************************
            抛事件到DMAC层, 同步DMAC数据
        ***************************************************************************/
        ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_CURRENT_CHANEL, us_len, puc_param);
        if (oal_unlikely(ul_ret != OAL_SUCC)) {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{config_set_freq::send_event failed[%d]}", ul_ret);
            return ul_ret;
        }
    }
#ifdef _PRE_WLAN_FEATURE_DBAC
    else if (mac_is_dbac_enabled(pst_mac_device)) {
        /***************************************************************************
            抛事件到DMAC层, 同步DMAC数据
        ***************************************************************************/
        ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_CURRENT_CHANEL, us_len, puc_param);
        if (oal_unlikely(ul_ret != OAL_SUCC)) {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{config_set_freq::send_event failed[%d]}", ul_ret);
            return ul_ret;
        }
    }
#endif
    else {
        if (pst_mac_device->uc_max_channel != uc_channel) {
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_set_freq::previous vap channel number=%d mismatch [%d].}",
                             pst_mac_device->uc_max_channel, uc_channel);

            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

#ifdef _PRE_BT_FITTING_DATA_COLLECT
oal_uint32 hmac_config_init_bt_env(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_INIT_BT_ENV, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_init_bt_env::hmac_config_send_event failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

oal_uint32 hmac_config_set_bt_freq(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_BT_FREQ, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_bt_freq::hmac_config_send_event failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

oal_uint32 hmac_config_set_bt_upc_by_freq(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_BT_UPC_BY_FREQ, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_bt_upc_by_freq::hmac_config_send_event failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

oal_uint32 hmac_config_print_bt_gm(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PRINT_BT_GM, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_print_bt_gm::hmac_config_send_event failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}
#endif


oal_uint32 hmac_config_get_freq(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    *((oal_uint32 *)puc_param) = pst_mac_vap->st_channel.uc_chan_number;

    *pus_len = OAL_SIZEOF(oal_uint32);

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_UAPSD

oal_uint32 hmac_config_set_uapsden(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /* 设置mib值 */
    mac_vap_set_uapsd_en(pst_mac_vap, *puc_param);
    mac_set_uapsd_cap(*puc_param);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_UAPSD_EN, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_uapsden::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_get_uapsden(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    *puc_param = mac_vap_get_uapsd_en(pst_mac_vap);
    *pus_len = OAL_SIZEOF(oal_uint8);

    return OAL_SUCC;
}

#endif

#ifdef _PRE_WLAN_DFT_STAT

oal_uint32 hmac_config_usr_queue_stat(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    return OAL_SUCC;
}
#endif
oal_uint32 hmac_config_set_reset_state(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret = OAL_SUCC;
    mac_reset_sys_stru *pst_reset_sys;
    hmac_device_stru *pst_hmac_device;

    pst_reset_sys = (mac_reset_sys_stru *)puc_param;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{hmac_config_set_reset_state::pst_hmac_device[%d] is null.}",
                       pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }
    mac_device_set_dfr_reset(pst_hmac_device->pst_device_base_info, pst_reset_sys->uc_value);

    /* 待整改，当前存在DMAC TO HMAC SYNC，待处理。处理后做抛事件处理 */
    return ul_ret;
}


oal_uint32 hmac_config_dump_rx_dscr(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DUMP_RX_DSCR, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_dump_rx_dscr::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_dump_tx_dscr(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DUMP_TX_DSCR, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_dump_tx_dscr::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}

OAL_STATIC oal_uint32 hmac_config_set_channel_check_param(mac_device_stru *pst_mac_device,
                                                          mac_cfg_channel_param_stru *pst_prot_param)
{
    /* 根据device能力对参数进行检查 */
    if ((pst_prot_param->en_bandwidth > WLAN_BAND_WIDTH_40MINUS) &&
        (pst_mac_device->en_bandwidth_cap < WLAN_BW_CAP_80M)) {
        /* 设置80M带宽，但device能力不支持80M，返回错误码 */
        oam_warning_log2(0, OAM_SF_CFG,
                         "{hmac_config_set_channel_check_param::not support 80MHz bw,protocol=%d protocol_cap=%d.}",
                         pst_prot_param->en_bandwidth, pst_mac_device->en_bandwidth_cap);
        return OAL_ERR_CODE_CONFIG_BW_EXCEED;
    }

    if ((pst_prot_param->en_band == WLAN_BAND_5G) && (pst_mac_device->en_band_cap == WLAN_BAND_CAP_2G)) {
        /* 设置5G频带，但device不支持5G */
        oam_warning_log2(0, OAM_SF_CFG,
                         "{hmac_config_set_channel_check_param::not support 5GHz band,en_protocol=%d protocol_cap=%d.}",
                         pst_prot_param->en_band, pst_mac_device->en_band_cap);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    } else if ((pst_prot_param->en_band == WLAN_BAND_2G) && (pst_mac_device->en_band_cap == WLAN_BAND_CAP_5G)) {
        /* 设置2G频带，但device不支持2G */
        oam_warning_log2(0, OAM_SF_CFG,
                         "{hmac_config_set_channel_check_param::not support 2GHz band,en_protocol=%d protocol_cap=%d.}",
                         pst_prot_param->en_band, pst_mac_device->en_band_cap);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }
    /* gaolin: CAC在初始检查时统一执行 */
    return OAL_SUCC;
}
OAL_STATIC oal_void hmac_change_mib_by_bandwidth_for_no_dbac(mac_cfg_channel_param_stru *pst_channel_param,
    mac_device_stru *pst_mac_device)
{
    mac_vap_stru *pst_mac_vap_tmp = OAL_PTR_NULL;
    oal_uint8 uc_vap_idx;
    oal_uint32 ul_ret;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap_tmp = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_mac_vap_tmp == OAL_PTR_NULL) {
            continue;
        }
        pst_mac_vap_tmp->st_channel.uc_chan_number = pst_channel_param->uc_channel;
        pst_mac_vap_tmp->st_channel.en_band = pst_channel_param->en_band;
        pst_mac_vap_tmp->st_channel.en_bandwidth = pst_channel_param->en_bandwidth;
        ul_ret = mac_get_channel_idx_from_num(pst_channel_param->en_band, pst_channel_param->uc_channel,
                                              &(pst_mac_vap_tmp->st_channel.uc_idx));
        if (ul_ret != OAL_SUCC) {
            oam_warning_log3(pst_mac_vap_tmp->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_set_channel::get_channel_idx_from_num failed[%d],band[%d],channel[%d].}",
                             ul_ret, pst_channel_param->en_band, pst_channel_param->uc_channel);
            continue;
        }

        /* 根据带宽信息更新Mib */
        mac_vap_change_mib_by_bandwidth(pst_mac_vap_tmp, pst_channel_param->en_bandwidth);
    }
}

oal_uint32 hmac_config_set_channel(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_channel_param_stru *pst_channel_param;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_bool_enum_uint8 en_set_reg = OAL_FALSE;
    oal_uint32 ul_up_vap_cnt;

    pst_channel_param = (mac_cfg_channel_param_stru *)puc_param;

    /* 窄带下强制设置20M带宽 */
    if (g_hitalk_status) {
        pst_channel_param->en_bandwidth = WLAN_BAND_WIDTH_20M;
    }

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{hmac_config_set_channel::mac_device[%d] null.}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 检查配置参数是否在device能力内 */
    ul_ret = hmac_config_set_channel_check_param(pst_mac_device, pst_channel_param);
    if (ul_ret != OAL_SUCC) {
        if (ul_ret != OAL_ERR_CODE_CONFIG_BW_EXCEED) {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_set_channel::hmac_config_set_channel_check_param failed[%d].}", ul_ret);
            return ul_ret;
        }

        pst_channel_param->en_bandwidth = mac_vap_get_bandwith(pst_mac_device->en_bandwidth_cap,
                                                               pst_channel_param->en_bandwidth);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_channel::Updated bw = %d}",
                         pst_channel_param->en_bandwidth);
    }
    ul_up_vap_cnt = hmac_calc_up_ap_num(pst_mac_device);
    /* 仅在没有VAP up的情况下，配置硬件频带、带宽寄存器 */
    if (ul_up_vap_cnt <= 1) {
        /* 记录首次配置的带宽值 */
        mac_device_set_channel(pst_mac_device, pst_channel_param);

        /***************************************************************************
         抛事件到DMAC层, 配置寄存器  置标志位
        ***************************************************************************/
        en_set_reg = OAL_TRUE;
    } else if (mac_is_dbac_enabled(pst_mac_device) == OAL_TRUE) {
        /* 开启DBAC不进行信道判断 */
        /* 信道设置只针对AP模式，非AP模式则跳出 */
    } else {
        /* 信道不是当前信道 */
        if (pst_mac_device->uc_max_channel != pst_channel_param->uc_channel) {
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{previous vap channel number=%d mismatch [%d].}",
                             pst_mac_device->uc_max_channel, pst_channel_param->uc_channel);

            return OAL_FAIL;
        }

        /* 带宽不能超出已配置的带宽 */
        ul_ret = hmac_config_set_mode_check_bandwith(pst_mac_device->en_max_bandwidth, pst_channel_param->en_bandwidth);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_set_channel::set_mode_check_bw failed[%d],previous vap bw[%d,current[%d]}",
                             ul_ret, pst_mac_device->en_max_bandwidth, pst_channel_param->en_bandwidth);
            return OAL_FAIL;
        }
    }
    if (mac_is_dbac_enabled(pst_mac_device) == OAL_TRUE) {
        pst_mac_vap->st_channel.uc_chan_number = pst_channel_param->uc_channel;
        pst_mac_vap->st_channel.en_band = pst_channel_param->en_band;
        pst_mac_vap->st_channel.en_bandwidth = pst_channel_param->en_bandwidth;
        ul_ret = mac_get_channel_idx_from_num(pst_channel_param->en_band,
                                              pst_channel_param->uc_channel, &(pst_mac_vap->st_channel.uc_idx));
        if (ul_ret != OAL_SUCC) {
            oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_set_channel::get_channel_idx_from_num failed[%d], band[%d], channel[%d].}",
                             ul_ret, pst_channel_param->en_band, pst_channel_param->uc_channel);
            return OAL_FAIL;
        }

        /* 根据带宽信息更新Mib */
        mac_vap_change_mib_by_bandwidth(pst_mac_vap, pst_channel_param->en_bandwidth);

        en_set_reg = OAL_TRUE;
    } else {
        hmac_change_mib_by_bandwidth_for_no_dbac(pst_channel_param, pst_mac_device);
    }

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_set_channel: channel_num:%d, bw:%d, band:%d",
                     pst_channel_param->uc_channel,
                     pst_channel_param->en_bandwidth,
                     pst_channel_param->en_band);
    /***************************************************************************
     抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    if (en_set_reg == OAL_TRUE) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_config_set_channel: post event to dmac to set register");
        ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_CFG80211_SET_CHANNEL, us_len, puc_param);
        if (oal_unlikely(ul_ret != OAL_SUCC)) {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_set_channel::hmac_config_send_event failed[%d].}", ul_ret);
            return ul_ret;
        }
    }

#if defined(_PRE_WLAN_FEATURE_20_40_80_COEXIST)
    ul_up_vap_cnt = hmac_calc_up_ap_num(pst_mac_device);
    if (ul_up_vap_cnt < 2) { /* up 的vap 个数小于2 */
        hmac_init_scan_try(pst_mac_device, pst_mac_vap, MAC_TRY_INIT_SCAN_SET_CHANNEL);
    }
#endif

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_config_set_beacon_check_param(mac_device_stru *pst_mac_device,
                                                         mac_beacon_param_stru *pst_prot_param)
{
    /* 根据device能力对参数进行检查 */
    switch (pst_prot_param->en_protocol) {
        case WLAN_LEGACY_11A_MODE:
        case WLAN_LEGACY_11B_MODE:
        case WLAN_LEGACY_11G_MODE:
        case WLAN_MIXED_ONE_11G_MODE:
        case WLAN_MIXED_TWO_11G_MODE:
            break;

        case WLAN_HT_MODE:
        case WLAN_HT_ONLY_MODE:
        case WLAN_HT_11G_MODE:
            if (pst_mac_device->en_protocol_cap < WLAN_PROTOCOL_CAP_HT) {
                /* 设置11n协议，但device不支持HT模式 */
                oam_warning_log2(0, OAM_SF_CFG,
                                 "{hmac_config_set_beacon_check_param:not support HT mode,protocol=%d protocol_cap=%d}",
                                 pst_prot_param->en_protocol, pst_mac_device->en_protocol_cap);
                return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
            }
            break;

        case WLAN_VHT_MODE:
        case WLAN_VHT_ONLY_MODE:
            if (pst_mac_device->en_protocol_cap < WLAN_PROTOCOL_CAP_VHT) {
                /* 设置11ac协议，但device不支持VHT模式 */
                oam_warning_log2(0, OAM_SF_CFG,
                                 "{not support VHT mode,protocol=%d protocol_cap=%d.}",
                                 pst_prot_param->en_protocol, pst_mac_device->en_protocol_cap);
                return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
            }
            break;

        default:
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_beacon_check_param::mode param does not in the list.}");
            break;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_beacon(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_beacon_param_stru *pst_bcn_param = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    wlan_mib_ieee802dot11_stru *pst_mib_info = OAL_PTR_NULL;

    mac_device_stru *pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if ((oal_unlikely(pst_mac_device == OAL_PTR_NULL)) || (puc_param == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG, "{null,mac_device=%x,param=%x}", (uintptr_t)pst_mac_device, (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_beacon::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_bcn_param = (mac_beacon_param_stru *)puc_param;

    /* 检查协议配置参数是否在device能力内 */
    ul_ret = hmac_config_set_beacon_check_param(pst_mac_device, pst_bcn_param);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    if (pst_bcn_param->en_protocol >= WLAN_HT_MODE) {
        pst_hmac_vap->en_tx_aggr_on = OAL_TRUE;
        pst_hmac_vap->en_amsdu_active = OAL_TRUE;
    } else {
        pst_hmac_vap->en_tx_aggr_on = OAL_FALSE;
        pst_hmac_vap->en_amsdu_active = OAL_FALSE;
    }

    /* 设置协议模式 */
    if ((pst_bcn_param->en_privacy == OAL_TRUE) && (pst_bcn_param->uc_crypto_mode & (WLAN_WPA_BIT | WLAN_WPA2_BIT))) {
        pst_hmac_vap->en_auth_mode = WLAN_WITP_AUTH_OPEN_SYSTEM; /* 强制设置VAP 认证方式为OPEN */
    }
    mac_vap_set_hide_ssid(pst_mac_vap, pst_bcn_param->uc_hidden_ssid);

    /* 1102适配新内核start ap和change beacon接口复用此接口，不同的是change beacon时，不再设置beacon周期
       和dtim周期，因此，change beacon时，interval和dtim period参数为全零，此时不应该被设置到mib中 */
    /* 设置VAP beacon interval， dtim_period */
    pst_mib_info = pst_mac_vap->pst_mib_info;
    if ((pst_bcn_param->l_dtim_period != 0) || (pst_bcn_param->l_interval != 0)) {
        pst_mib_info->st_wlan_mib_sta_config.ul_dot11DTIMPeriod = (oal_uint32)pst_bcn_param->l_dtim_period;
        pst_mib_info->st_wlan_mib_sta_config.ul_dot11BeaconPeriod = (oal_uint32)pst_bcn_param->l_interval;
    }

    /* 设置short gi */
    pst_mib_info->st_phy_ht.en_dot11ShortGIOptionInTwentyImplemented = pst_bcn_param->en_shortgi_20;
    mac_mib_set_ShortGIOptionInFortyImplemented(pst_mac_vap, pst_bcn_param->en_shortgi_40);

    pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn80Implemented = pst_bcn_param->en_shortgi_80;

    mac_vap_set_beacon(pst_mac_vap, pst_bcn_param);

    mac_vap_init_by_protocol(pst_mac_vap, pst_bcn_param->en_protocol);
    mac_vap_init_rates(pst_mac_vap);

    /***************************************************************************
     抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_CFG80211_CONFIG_BEACON, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_beacon::send_event failed[%d]}", ul_ret);
    }

    return ul_ret;
}
/*lint -e801*/
oal_int32 hmac_print_vap_info(mac_vap_stru *pst_mac_vap,
    oal_int8 *pc_print_buff, oal_uint32 ul_string_len)
{
    oal_int32 l_string_tmp_len;
    l_string_tmp_len = snprintf_s(pc_print_buff + ul_string_len,
        (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1), (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1) - 1,
        "vap id: %d  device id: %d  chip id: %d\n"
        "vap state: %d\n"
        "vap mode: %d   P2P mode:%d\n"
        "ssid: %.32s\n"
        "hide_ssid :%d\n",
        pst_mac_vap->uc_vap_id, pst_mac_vap->uc_device_id, pst_mac_vap->uc_chip_id,
        pst_mac_vap->en_vap_state, pst_mac_vap->en_vap_mode,
        pst_mac_vap->en_p2p_mode,
        pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11DesiredSSID,
        pst_mac_vap->st_cap_flag.bit_hide_ssid);
        return l_string_tmp_len;
}

oal_int32 hmac_print_ap_sta_protocol_mode(mac_vap_stru *pst_mac_vap, oal_int8 *pc_print_buff,
    oal_uint32 ul_string_len)
{
    oal_int32 l_string_tmp_len;
    wlan_protocol_enum_uint8 en_disp_protocol;
    mac_user_stru *pst_mac_user = OAL_PTR_NULL;
    /* AP/STA协议模式显示 */
    if ((pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        ((pst_mac_user = mac_res_get_mac_user(pst_mac_vap->uc_assoc_vap_id)) != NULL)) {
        en_disp_protocol = pst_mac_user->en_cur_protocol_mode;
        l_string_tmp_len = snprintf_s(pc_print_buff + ul_string_len,
            (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1),
            (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1) - 1, "protocol: sta|ap[%s|%s]\n",
            hmac_config_protocol2string(pst_mac_vap->en_protocol),
            hmac_config_protocol2string(en_disp_protocol));
    } else {
        en_disp_protocol = pst_mac_vap->en_protocol;
        l_string_tmp_len = snprintf_s(pc_print_buff + ul_string_len,
            (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1),
            (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1) - 1,
            "protocol: %s\n", hmac_config_protocol2string(en_disp_protocol));
    }
    return l_string_tmp_len;
}

oal_int32 hmac_print_feature_and_cap_info(mac_vap_stru *pst_mac_vap, hmac_vap_stru *pst_hmac_vap,
    oal_int8 *pc_print_buff, oal_uint32 ul_string_len, mac_device_stru *pst_mac_device)
{
    oal_int32 l_string_tmp_len;
    l_string_tmp_len = snprintf_s(pc_print_buff + ul_string_len, (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1),
        (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1) - 1, "band: %s  bandwidth: %s\n"
        "channel number:%d \n"
        "associated user number:%d/%d \n"
        "Beacon interval:%d \n"
        "vap feature info:\n"
        "amsdu  wme  msdu_defrag   uapsd   psm  wpa   wpa2   wps  keepalive\n"
        "%d      %d       %d       %d      %d    %d    %d     %d     %d\n"
        "vap cap info:\n"
        "shpreamble  shslottime  nobeacon  shortgi   2g11ac \n"
        "%d           %d          %d         %d         %d\n"
        "rx chain: 0x%x, tx_chain: 0x%x\n"
        "tx power: %d \n"
        "protect mode: %d, auth mode: %d\n"
        "erp aging cnt: %d, ht aging cnt: %d\n"
        "auto_protection: %d\nobss_non_erp_present: %d\nobss_non_ht_present: %d\n"
        "rts_cts_protect_mode: %d\ntxop_protect_mode: %d\n"
        "no_short_slot_num: %d\nno_short_preamble_num: %d\nnon_erp_num: %d\n"
        "non_ht_num: %d\nnon_gf_num: %d\n20M_only_num: %d\n"
        "no_40dsss_cck_num: %d\nno_lsig_txop_num: %d\n",
        hmac_config_band2string(pst_mac_vap->st_channel.en_band),
        hmac_config_bw2string(pst_mac_vap->en_protocol <= WLAN_MIXED_TWO_11G_MODE ?
            WLAN_BAND_WIDTH_20M : pst_mac_vap->st_channel.en_bandwidth),
        pst_mac_vap->st_channel.uc_chan_number,
        pst_mac_vap->us_user_nums, pst_hmac_vap->us_user_nums_max,
        pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11BeaconPeriod,
        pst_hmac_vap->en_amsdu_active,
        pst_hmac_vap->en_wme_active,
        pst_hmac_vap->en_msdu_defrag_active,
        pst_mac_vap->st_cap_flag.bit_uapsd,
        pst_hmac_vap->en_psm_active,
        pst_mac_vap->st_cap_flag.bit_wpa,
        pst_mac_vap->st_cap_flag.bit_wpa2,
        pst_hmac_vap->en_wps_active,
        pst_mac_vap->st_cap_flag.bit_keepalive,
        mac_mib_get_ShortPreambleOptionImplemented(pst_mac_vap),
        pst_mac_vap->pst_mib_info->st_phy_erp.en_dot11ShortSlotTimeOptionImplemented,
        pst_hmac_vap->en_no_beacon,
        pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11ShortGIOptionInTwentyImplemented,
        pst_mac_vap->st_cap_flag.bit_11ac2g,
        pst_mac_device->uc_rx_chain, pst_mac_device->uc_tx_chain,
        pst_mac_vap->uc_tx_power,
        pst_mac_vap->st_protection.en_protection_mode,
        pst_hmac_vap->en_auth_mode,
        pst_mac_vap->st_protection.uc_obss_non_erp_aging_cnt,
        pst_mac_vap->st_protection.uc_obss_non_ht_aging_cnt,
        pst_mac_vap->st_protection.bit_auto_protection,
        pst_mac_vap->st_protection.bit_obss_non_erp_present,
        pst_mac_vap->st_protection.bit_obss_non_ht_present,
        pst_mac_vap->st_protection.bit_rts_cts_protect_mode,
        pst_mac_vap->st_protection.bit_lsig_txop_protect_mode,
        pst_mac_vap->st_protection.uc_sta_no_short_slot_num,
        pst_mac_vap->st_protection.uc_sta_no_short_preamble_num,
        pst_mac_vap->st_protection.uc_sta_non_erp_num,
        pst_mac_vap->st_protection.uc_sta_non_ht_num,
        pst_mac_vap->st_protection.uc_sta_non_gf_num,
        pst_mac_vap->st_protection.uc_sta_20M_only_num,
        pst_mac_vap->st_protection.uc_sta_no_40dsss_cck_num,
        pst_mac_vap->st_protection.uc_sta_no_lsig_txop_num);
        return l_string_tmp_len;
}
oal_bool_enum_uint8 hmac_print_wpa_v1_info(oal_uint8 uc_group_suite, oal_uint8 *puc_pairwise_suite,
    oal_uint8 *puc_akm_suite, oal_int8 *pc_print_buff, oal_uint32 *pul_string_len)
{
    oal_int32 l_string_tmp_len;
    oal_uint8 uc_loop;
    l_string_tmp_len = snprintf_s(pc_print_buff + *pul_string_len,
        (OAM_REPORT_MAX_STRING_LEN - *pul_string_len - 1),
        (OAM_REPORT_MAX_STRING_LEN - *pul_string_len - 1) - 1,
        "Privacy Invoked: \nRSNA-WPA \n GROUP: %s\n", hmac_config_cipher2string(uc_group_suite));
    if (l_string_tmp_len < 0) {
        return OAL_FALSE;
    }
    *pul_string_len += (oal_uint32)l_string_tmp_len;

    uc_loop = 0;
    while ((uc_loop < WLAN_PAIRWISE_CIPHER_SUITES) && (puc_pairwise_suite[uc_loop] != 0)) {
        l_string_tmp_len = snprintf_s(pc_print_buff + *pul_string_len,
            (OAM_REPORT_MAX_STRING_LEN - *pul_string_len - 1),
            (OAM_REPORT_MAX_STRING_LEN - *pul_string_len - 1) - 1, "WPA PAIREWISE[%d]: %s\n", uc_loop,
            hmac_config_cipher2string(oal_min(puc_pairwise_suite[uc_loop], 15))); /* puc_pairwise_suite[uc_loop]与15取小 */
        if (l_string_tmp_len < 0) {
            return OAL_FALSE;
        }
        *pul_string_len += (oal_uint32)l_string_tmp_len;
        uc_loop++;
    }

    uc_loop = 0;
    while ((uc_loop < WLAN_AUTHENTICATION_SUITES) && (puc_akm_suite[uc_loop] != 0)) {
        l_string_tmp_len = snprintf_s(pc_print_buff + *pul_string_len,
            (OAM_REPORT_MAX_STRING_LEN - *pul_string_len - 1),
            (OAM_REPORT_MAX_STRING_LEN - *pul_string_len - 1) - 1,
            /* puc_akm_suite[uc_loop]与10之间取小 */
            "WPA AKM[%d]: %s\n", uc_loop, hmac_config_akm2string(oal_min(puc_akm_suite[uc_loop], 10)));
        if (l_string_tmp_len < 0) {
            return OAL_FALSE;
        }
        *pul_string_len += (oal_uint32)l_string_tmp_len;
        uc_loop++;
    }
    return OAL_TRUE;
}

oal_bool_enum_uint8 hmac_print_wpa_v2_info(oal_uint8 uc_group_suite, oal_uint8 *puc_pairwise_suite,
    oal_uint8 *puc_akm_suite, oal_int8 *pc_print_buff, oal_uint32 *pul_string_len)
{
    oal_int32 l_string_tmp_len;
    oal_uint8 uc_loop;
    l_string_tmp_len = snprintf_s(pc_print_buff + *pul_string_len,
        (OAM_REPORT_MAX_STRING_LEN - *pul_string_len - 1),
        (OAM_REPORT_MAX_STRING_LEN - *pul_string_len - 1) - 1,
        "Privacy Invoked: \nRSNA-WPA \n GROUP: %s\n", hmac_config_cipher2string(uc_group_suite));
    if (l_string_tmp_len < 0) {
        return OAL_FALSE;
    }
    *pul_string_len += (oal_uint32)l_string_tmp_len;

    uc_loop = 0;
    while ((uc_loop < WLAN_PAIRWISE_CIPHER_SUITES) && (puc_pairwise_suite[uc_loop] != 0)) {
        l_string_tmp_len = snprintf_s(pc_print_buff + *pul_string_len,
            (OAM_REPORT_MAX_STRING_LEN - *pul_string_len - 1),
            (OAM_REPORT_MAX_STRING_LEN - *pul_string_len - 1) - 1,
            "WPA PAIREWISE[%d]: %s\n", uc_loop,
            hmac_config_cipher2string(oal_min(puc_pairwise_suite[uc_loop], 15))); /* puc_pairwise_suite[uc_loop]与15取小 */
        if (l_string_tmp_len < 0) {
            return OAL_FALSE;
        }
        *pul_string_len += (oal_uint32)l_string_tmp_len;
        uc_loop++;
    }

    uc_loop = 0;
    while ((uc_loop < WLAN_AUTHENTICATION_SUITES) && (puc_akm_suite[uc_loop] != 0)) {
        l_string_tmp_len = snprintf_s(pc_print_buff + *pul_string_len,
            (OAM_REPORT_MAX_STRING_LEN - *pul_string_len - 1),
            (OAM_REPORT_MAX_STRING_LEN - *pul_string_len - 1) - 1,
            /* puc_akm_suite[uc_loop]与10之间取小 */
            "WPA AKM[%d]: %s\n", uc_loop, hmac_config_akm2string(oal_min(puc_akm_suite[uc_loop], 10)));
        if (l_string_tmp_len < 0) {
            return OAL_FALSE;
        }
        *pul_string_len += (oal_uint32)l_string_tmp_len;
        uc_loop++;
    }
    return OAL_TRUE;
}

oal_bool_enum_uint8 hmac_print_app_ie_info(mac_vap_stru *pst_mac_vap, oal_int8 *pc_print_buff,
    oal_uint32 *pul_string_len)
{
    oal_int32 l_string_tmp_len;
    oal_uint8 uc_loop;
    /* APP IE 信息 */
    for (uc_loop = 0; uc_loop < OAL_APP_IE_NUM; uc_loop++) {
        l_string_tmp_len = snprintf_s(pc_print_buff + *pul_string_len,
            (OAM_REPORT_MAX_STRING_LEN - *pul_string_len - 1),
            (OAM_REPORT_MAX_STRING_LEN - *pul_string_len - 1) - 1,
            "APP IE:type= %d, addr = %p, len = %d, max_len = %d\n", uc_loop,
            pst_mac_vap->ast_app_ie[uc_loop].puc_ie, pst_mac_vap->ast_app_ie[uc_loop].ul_ie_len,
            pst_mac_vap->ast_app_ie[uc_loop].ul_ie_max_len);
        if (l_string_tmp_len < 0) {
            return OAL_FALSE;
        }
        *pul_string_len += (oal_uint32)l_string_tmp_len;
    }
    return OAL_TRUE;
}


oal_uint32 hmac_config_vap_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    mac_user_stru *pst_multi_user = OAL_PTR_NULL;
    oal_uint8 uc_group_suite;
    oal_uint8 *puc_pairwise_suite = OAL_PTR_NULL;
    oal_uint8 *puc_akm_suite = OAL_PTR_NULL;
    oal_int8 *pc_print_buff = OAL_PTR_NULL;
    oal_uint32 ul_string_len;
    oal_int32 l_string_tmp_len;

    wlan_mib_dot11RSNAConfigEntry_stru *pst_wlan_mib_rsna_cfg = OAL_PTR_NULL;

    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_vap_info::this is config vap! can't get info.}");
        return OAL_FAIL;
    }

    pst_mac_device = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vap_info::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pc_print_buff = (oal_int8 *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (pc_print_buff == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vap_info::pc_print_buff null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
    ul_string_len = 0;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vap_info::pst_hmac_vap null.}");
        oal_mem_free_m(pc_print_buff, OAL_TRUE);
        return OAL_ERR_CODE_PTR_NULL;
    }

    l_string_tmp_len = hmac_print_vap_info(pst_mac_vap, pc_print_buff, ul_string_len);
    if (l_string_tmp_len < 0) {
        goto sprint_fail;
    }
    ul_string_len += (oal_uint32)l_string_tmp_len;

    /* AP/STA协议模式显示 */
    l_string_tmp_len = hmac_print_ap_sta_protocol_mode(pst_mac_vap, pc_print_buff, ul_string_len);
    if (l_string_tmp_len < 0) {
        goto sprint_fail;
    }
    ul_string_len += (oal_uint32)l_string_tmp_len;

    l_string_tmp_len = hmac_print_feature_and_cap_info(pst_mac_vap, pst_hmac_vap,
        pc_print_buff, ul_string_len, pst_mac_device);
    if (l_string_tmp_len < 0) {
        goto sprint_fail;
    }

    pc_print_buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    oam_print(pc_print_buff);

    /* 上述日志量超过OAM_REPORT_MAX_STRING_LEN，分多次oam_print */
    memset_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
    ul_string_len = 0;

    /* WPA/WPA2 加密参数 */
    if (mac_mib_get_privacyinvoked(pst_mac_vap) == OAL_TRUE) {
        pst_multi_user = mac_res_get_mac_user(pst_mac_vap->us_multi_user_idx);
        if (pst_multi_user == OAL_PTR_NULL) {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vap_info::pst_multi_user[%d] null.}",
                             pst_mac_vap->us_multi_user_idx);
            oal_mem_free_m(pc_print_buff, OAL_TRUE);
            return OAL_ERR_CODE_PTR_NULL;
        }

        if (mac_mib_get_rsnaactivated(pst_mac_vap) == OAL_TRUE) {
            pst_wlan_mib_rsna_cfg = &pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg;
            if (pst_mac_vap->st_cap_flag.bit_wpa == 1) {
                uc_group_suite = oal_min(pst_wlan_mib_rsna_cfg->uc_wpa_group_suite, 15); /* wpa_group_suite与15取小 */
                puc_pairwise_suite = pst_wlan_mib_rsna_cfg->auc_wpa_pair_suites;
                puc_akm_suite = pst_wlan_mib_rsna_cfg->auc_wpa_akm_suites;

                if (hmac_print_wpa_v1_info(uc_group_suite, puc_pairwise_suite,
                    puc_akm_suite, pc_print_buff, &ul_string_len) == OAL_FALSE) {
                    goto sprint_fail;
                }
            }

            if (pst_mac_vap->st_cap_flag.bit_wpa2 == 1) {
                uc_group_suite = oal_min(pst_wlan_mib_rsna_cfg->uc_rsn_group_suite, 15); /* rsn_group_suite与15取小 */
                puc_pairwise_suite = pst_wlan_mib_rsna_cfg->auc_rsn_pair_suites;
                puc_akm_suite = pst_wlan_mib_rsna_cfg->auc_rsn_akm_suites;

                if (hmac_print_wpa_v2_info(uc_group_suite, puc_pairwise_suite,
                    puc_akm_suite, pc_print_buff, &ul_string_len) == OAL_FALSE) {
                    goto sprint_fail;
                }
            }
        }

        l_string_tmp_len = snprintf_s(pc_print_buff + ul_string_len, (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1),
            (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1) - 1, "MULTI_USER: cipher_type:%s, key_type:%d \n",
            hmac_config_cipher2string(pst_multi_user->st_key_info.en_cipher_type),
            pst_multi_user->st_user_tx_info.st_security.en_cipher_key_type);
        if (l_string_tmp_len < 0) {
            goto sprint_fail;
        }
        ul_string_len += (oal_uint32)l_string_tmp_len;
    } else {
        l_string_tmp_len = snprintf_s(pc_print_buff + ul_string_len, (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1),
            (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1) - 1, "Privacy NOT Invoked\n");
        if (l_string_tmp_len < 0) {
            goto sprint_fail;
        }
        ul_string_len += (oal_uint32)l_string_tmp_len;
    }

    /* APP IE 信息 */
    if (hmac_print_app_ie_info(pst_mac_vap, pc_print_buff, &ul_string_len) == OAL_FALSE) {
        goto sprint_fail;
    }

    pc_print_buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    oam_print(pc_print_buff);
    oal_mem_free_m(pc_print_buff, OAL_TRUE);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_VAP_INFO, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vap_info::hmac_config_send_event failed[%d].}",
                       ul_ret);
    }

    return ul_ret;

sprint_fail:

    oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_vap_info:: OAL_SPRINTF return error!}");
    pc_print_buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    oam_print(pc_print_buff);
    oal_mem_free_m(pc_print_buff, OAL_TRUE);

    return OAL_FAIL;
}
/*lint +e801*/

oal_uint32 hmac_config_event_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_int32 l_value;
    oal_uint32 ul_ret;
    oal_uint8 uc_loop_vap_id;

    l_value = *((oal_int32 *)puc_param);

    /* 设置OAM event模块的开关 */
    for (uc_loop_vap_id = 0; uc_loop_vap_id < WLAN_VAP_SUPPOTR_MAX_NUM_SPEC; uc_loop_vap_id++) {
        ul_ret = oam_event_set_switch(uc_loop_vap_id, (oal_switch_enum_uint8)l_value);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(uc_loop_vap_id, OAM_SF_CFG, "{hmac_config_event_switch::oam_event_set_switch failed[%d].}",
                             ul_ret);
            return ul_ret;
        }
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_eth_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_eth_switch_param_stru *pst_eth_switch_param;
    oal_uint16 us_user_idx = 0;
    oal_uint32 ul_ret;

    pst_eth_switch_param = (mac_cfg_eth_switch_param_stru *)puc_param;

    ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap,
                                          pst_eth_switch_param->auc_user_macaddr,
                                          &us_user_idx);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_eth_switch::mac_vap_find_user_by_macaddr failed[%d].}", ul_ret);
        return ul_ret;
    }

    ul_ret = oam_report_eth_frame_set_switch(us_user_idx,
                                             pst_eth_switch_param->en_switch,
                                             pst_eth_switch_param->en_frame_direction);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_eth_switch::oam_report_eth_frame_set_switch failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_80211_ucast_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_80211_ucast_switch_stru *pst_80211_switch_param;
    oal_uint16 us_user_idx = 0;
    oal_uint32 ul_ret;

    pst_80211_switch_param = (mac_cfg_80211_ucast_switch_stru *)puc_param;

    /* 广播地址，操作所有用户的单播帧开关 */
    if (ether_is_broadcast(pst_80211_switch_param->auc_user_macaddr)) {
        for (us_user_idx = 0; us_user_idx < WLAN_ACTIVE_USER_MAX_NUM + WLAN_MAX_MULTI_USER_NUM_SPEC; us_user_idx++) {
            oam_report_80211_ucast_set_switch(pst_80211_switch_param->en_frame_direction,
                                              pst_80211_switch_param->en_frame_type,
                                              pst_80211_switch_param->en_frame_switch,
                                              pst_80211_switch_param->en_cb_switch,
                                              pst_80211_switch_param->en_dscr_switch,
                                              us_user_idx);
        }
    } else {
        ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap,
                                              pst_80211_switch_param->auc_user_macaddr,
                                              &us_user_idx);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_80211_ucast_switch::find_user_by_macaddr[%02X:XX:XX:%02X:%02X:%02X]failed}",
                             /* auc_user_macaddr第0、3byte为参数输出打印 */
                             pst_80211_switch_param->auc_user_macaddr[0], pst_80211_switch_param->auc_user_macaddr[3],
                             /* auc_user_macaddr第4、5byte为参数输出打印 */
                             pst_80211_switch_param->auc_user_macaddr[4], pst_80211_switch_param->auc_user_macaddr[5]);
            return ul_ret;
        }

        ul_ret = oam_report_80211_ucast_set_switch(pst_80211_switch_param->en_frame_direction,
                                                   pst_80211_switch_param->en_frame_type,
                                                   pst_80211_switch_param->en_frame_switch,
                                                   pst_80211_switch_param->en_cb_switch,
                                                   pst_80211_switch_param->en_dscr_switch,
                                                   us_user_idx);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{Set switch of report_ucast failed[%d]!frame_switch[%d], cb_switch[%d], dscr_switch[%d]}",
                             ul_ret,
                             pst_80211_switch_param->en_frame_switch,
                             pst_80211_switch_param->en_cb_switch,
                             pst_80211_switch_param->en_dscr_switch);
            return ul_ret;
        }
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_80211_UCAST_SWITCH, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_80211_ucast_switch::hmac_config_send_event fail[%d].", ul_ret);
    }
#endif /* DMAC_OFFLOAD */

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_mgmt_log(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user,
                                    oal_bool_enum_uint8 en_start)
{
    mac_cfg_80211_ucast_switch_stru st_80211_ucast_switch;

    if ((pst_mac_vap == OAL_PTR_NULL) || (pst_mac_user == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (en_start != OAL_TRUE) {
        st_80211_ucast_switch.en_frame_direction = OAM_OTA_FRAME_DIRECTION_TYPE_TX;
        st_80211_ucast_switch.en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
        st_80211_ucast_switch.en_frame_switch = OAL_SWITCH_OFF;
        st_80211_ucast_switch.en_cb_switch = OAL_SWITCH_OFF;
        st_80211_ucast_switch.en_dscr_switch = OAL_SWITCH_OFF;
        oal_set_mac_addr(st_80211_ucast_switch.auc_user_macaddr, pst_mac_user->auc_user_mac_addr);
        hmac_config_80211_ucast_switch(pst_mac_vap, OAL_SIZEOF(st_80211_ucast_switch),
                                       (oal_uint8 *)&st_80211_ucast_switch);

        st_80211_ucast_switch.en_frame_direction = OAM_OTA_FRAME_DIRECTION_TYPE_RX;
        st_80211_ucast_switch.en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
        st_80211_ucast_switch.en_frame_switch = OAL_SWITCH_OFF;
        st_80211_ucast_switch.en_cb_switch = OAL_SWITCH_OFF;
        st_80211_ucast_switch.en_dscr_switch = OAL_SWITCH_OFF;
        hmac_config_80211_ucast_switch(pst_mac_vap, OAL_SIZEOF(st_80211_ucast_switch),
                                       (oal_uint8 *)&st_80211_ucast_switch);
    } else {
        st_80211_ucast_switch.en_frame_direction = OAM_OTA_FRAME_DIRECTION_TYPE_TX;
        st_80211_ucast_switch.en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
        st_80211_ucast_switch.en_frame_switch = OAL_SWITCH_ON;
        st_80211_ucast_switch.en_cb_switch = OAL_SWITCH_ON;
        st_80211_ucast_switch.en_dscr_switch = OAL_SWITCH_ON;
        oal_set_mac_addr(st_80211_ucast_switch.auc_user_macaddr, pst_mac_user->auc_user_mac_addr);

        hmac_config_80211_ucast_switch(pst_mac_vap, OAL_SIZEOF(st_80211_ucast_switch),
                                       (oal_uint8 *)&st_80211_ucast_switch);

        st_80211_ucast_switch.en_frame_direction = OAM_OTA_FRAME_DIRECTION_TYPE_RX;
        st_80211_ucast_switch.en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
        st_80211_ucast_switch.en_frame_switch = OAL_SWITCH_ON;
        st_80211_ucast_switch.en_cb_switch = OAL_SWITCH_ON;
        st_80211_ucast_switch.en_dscr_switch = OAL_SWITCH_ON;
        hmac_config_80211_ucast_switch(pst_mac_vap, OAL_SIZEOF(st_80211_ucast_switch),
                                       (oal_uint8 *)&st_80211_ucast_switch);
    }
    return OAL_SUCC;
}

#ifdef _PRE_DEBUG_MODE_USER_TRACK

oal_uint32 hmac_config_report_thrput_stat(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_USR_THRPUT_STAT, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_report_thrput_stat::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}

#endif

#ifdef _PRE_WLAN_FEATURE_TXOPPS


oal_uint32 hmac_config_set_txop_ps_machw(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TXOP_PS_MACHW, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TXOP,
                         "{hmac_config_set_txop_ps_machw::send event return err code [%d].}", ul_ret);
    }

    return ul_ret;
}

#endif

#ifdef _PRE_WLAN_FEATURE_LTECOEX

oal_uint32 hmac_config_ltecoex_mode_set(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_LTECOEX_MODE_SET, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_COEX,
                         "{hmac_config_ltecoex_mode_set::send event return err code [%d].}", ul_ret);
    }

    return ul_ret;
}
#endif


oal_uint32 hmac_config_80211_mcast_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_80211_mcast_switch_stru *pst_80211_switch_param;
    oal_uint32 ul_ret;

    pst_80211_switch_param = (mac_cfg_80211_mcast_switch_stru *)puc_param;

    ul_ret = oam_report_80211_mcast_set_switch(pst_80211_switch_param->en_frame_direction,
                                               pst_80211_switch_param->en_frame_type,
                                               pst_80211_switch_param->en_frame_switch,
                                               pst_80211_switch_param->en_cb_switch,
                                               pst_80211_switch_param->en_dscr_switch);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_80211_mcast_switch::oam_report_80211_mcast_set_switch failed[%d].}", ul_ret);
        return ul_ret;
    }
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_80211_MCAST_SWITCH, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_80211_mcast_switch::hmac_config_send_event fail[%d].", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_probe_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_probe_switch_stru *pst_probe_switch;
    oal_uint32 ul_ret;

    pst_probe_switch = (mac_cfg_probe_switch_stru *)puc_param;

    ul_ret = oam_report_80211_probe_set_switch(pst_probe_switch->en_frame_direction,
                                               pst_probe_switch->en_frame_switch,
                                               pst_probe_switch->en_cb_switch,
                                               pst_probe_switch->en_dscr_switch);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_probe_switch::oam_report_80211_probe_set_switch failed[%d].}", ul_ret);
        return ul_ret;
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PROBE_SWITCH, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_probe_switch::hmac_config_send_event fail[%d].", ul_ret);
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_phy_debug_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PHY_DEBUG_SWITCH, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_rssi_switch::hmac_config_send_event fail[%d].", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

oal_uint32 hmac_config_opmode_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    mac_opmode_switch_stru *pst_opmode_switch = (mac_opmode_switch_stru *)puc_param;

    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11OperatingModeNotificationImplemented =
        pst_opmode_switch->uc_opmode_switch;
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_OPMODE_SWITCH, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_rssi_switch::hmac_config_send_event fail[%d].", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32 hmac_config_wfa_cfg_aifsn(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_WFA_CFG_AIFSN, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_wfa_cfg_aifsn::hmac_config_send_event fail[%d].", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_wfa_cfg_cw(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_WFA_CFG_CW, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_wfa_cfg_cw::hmac_config_send_event fail[%d].", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

oal_uint32 hmac_config_lte_gpio_mode(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_CHECK_LTE_GPIO, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_lte_gpio_mode::hmac_config_send_event fail[%d].", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

#endif


oal_uint32 hmac_config_get_mpdu_num(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_GET_MPDU_NUM, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_send_event::hmac_config_send_event fail[%d].", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_2040_coext_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint8 l_value;

    l_value = *puc_param;
    if (l_value == 0) {
        pst_mac_vap->st_cap_flag.bit_2040_autoswitch = 0;
    } else {
        pst_mac_vap->st_cap_flag.bit_2040_autoswitch = 1;
    }

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_2040_coext_switch::2040_autoswitch = [%d].",
                     pst_mac_vap->st_cap_flag.bit_2040_autoswitch);
    return OAL_SUCC;
}


oal_uint32 hmac_config_ota_beacon_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint8 uc_vap_id_loop;
    oal_uint32 ul_ret;
    oal_int32 l_value;

    l_value = *((oal_int32 *)puc_param);

    for (uc_vap_id_loop = 0; uc_vap_id_loop < WLAN_VAP_SUPPOTR_MAX_NUM_SPEC; uc_vap_id_loop++) {
        ul_ret = oam_ota_set_beacon_switch(uc_vap_id_loop,
                                           (oam_sdt_print_beacon_rxdscr_type_enum_uint8)l_value);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log0(uc_vap_id_loop, OAM_SF_ANY, "{hmac_config_ota_beacon_switch::ota bcn switch set failed!}");
            return ul_ret;
        }
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_OTA_BEACON_SWITCH, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_ota_beacon_switch::hmac_config_send_event fail[%d].", ul_ret);
    }

    return ul_ret;
}

oal_uint32 hmac_config_ota_rx_dscr_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint8 uc_vap_id_loop;
    oal_uint32 ul_ret;
    oal_int32 l_value;

    l_value = *((oal_int32 *)puc_param);

    for (uc_vap_id_loop = 0; uc_vap_id_loop < WLAN_VAP_SUPPOTR_MAX_NUM_SPEC; uc_vap_id_loop++) {
        ul_ret = oam_ota_set_rx_dscr_switch(uc_vap_id_loop,
                                            (oal_switch_enum_uint8)l_value);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log0(uc_vap_id_loop, OAM_SF_ANY,
                             "{hmac_config_ota_rx_dscr_switch::ota rx_dscr switch set failed!}\r\n");
            return ul_ret;
        }
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_OTA_RX_DSCR_SWITCH, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_ota_rx_dscr_switch::hmac_config_send_event fail[%d].", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_all_ota(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    oal_switch_enum_uint8 en_switch;

    en_switch = *((oal_switch_enum_uint8 *)puc_param);
    oam_report_set_all_switch(en_switch);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_ALL_OTA, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_set_all_ota::hmac_config_send_event fail[%d].", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_oam_output(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_int32 l_value;
    oal_uint32 ul_ret;

    l_value = *((oal_int32 *)puc_param);

    /* 设置OAM log模块的开关 */
    ul_ret = oam_set_output_type((oam_output_type_enum_uint8)l_value);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_oam_output::oam_set_output_type failed[%d]}",
                         ul_ret);
        return ul_ret;
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_OAM_OUTPUT_TYPE, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_oam_output::hmac_config_send_event fail[%d].", ul_ret);
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_dhcp_arp_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_switch_enum_uint8 en_switch;

    en_switch = *((oal_switch_enum_uint8 *)puc_param);
    oam_report_dhcp_arp_set_switch(en_switch);

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_random_mac_addr_scan(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    oal_bool_enum_uint8 en_random_mac_addr_scan_switch;

    en_random_mac_addr_scan_switch = *((oal_bool_enum_uint8 *)puc_param);

    /* 获取hmac device结构体 */
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_random_mac_addr_scan::pst_hmac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    g_wlan_customize.uc_random_mac_addr_scan = en_random_mac_addr_scan_switch;
#else
    pst_hmac_device->st_scan_mgmt.en_is_random_mac_addr_scan = en_random_mac_addr_scan_switch;
#endif

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_random_mac_oui(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_random_mac_oui::pst_mac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_set_random_mac_oui::pst_hmac_device is null.device_id %d}",
                         pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (us_len < WLAN_RANDOM_MAC_OUI_LEN) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_set_random_mac_oui::len is short:%d.}", us_len);
        return OAL_FAIL;
    }

    if (memcpy_s(pst_mac_device->auc_mac_oui, WLAN_RANDOM_MAC_OUI_LEN, puc_param, WLAN_RANDOM_MAC_OUI_LEN) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "hmac_config_set_random_mac_oui::memcpy fail!");
        return OAL_FAIL;
    }
    /* 系统会在启动wifi 后下发非零mac_oui, wps扫描或hilink连接的场景中,将mac_oui清0,
     * mac_oui 非零时生成扫描随机MAC, wifi 扫描时使用该MAC地址作为源地址 */
    if ((pst_mac_device->auc_mac_oui[0] != 0) || (pst_mac_device->auc_mac_oui[1] != 0) ||
        (pst_mac_device->auc_mac_oui[2] != 0)) { /* 判断由系统下发的 随机mac地址OUI是否非0(auc_mac_oui 0、1、2byte) */
        oal_random_ether_addr(pst_hmac_device->st_scan_mgmt.auc_random_mac);
        pst_hmac_device->st_scan_mgmt.auc_random_mac[0] = pst_mac_device->auc_mac_oui[0] & 0xfe; /* 保证是单播mac */
        pst_hmac_device->st_scan_mgmt.auc_random_mac[1] = pst_mac_device->auc_mac_oui[1];
        /* auc_mac_oui第2byte(系统下发的 随机mac地址OUI)赋值给auc_random_mac第2byte(扫描时候用的随机MAC) */
        pst_hmac_device->st_scan_mgmt.auc_random_mac[2] = pst_mac_device->auc_mac_oui[2];

        oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_set_random_mac_oui::rand_mac_addr[%02X:XX:XX:%02X:%02X:%02X].}",
                         pst_hmac_device->st_scan_mgmt.auc_random_mac[0],
                         pst_hmac_device->st_scan_mgmt.auc_random_mac[3], /* auc_random_mac 第3、4byte为参数输出打印 */
                         pst_hmac_device->st_scan_mgmt.auc_random_mac[4],
                         pst_hmac_device->st_scan_mgmt.auc_random_mac[5]); /* auc_random_mac 第5byte为参数输出打印 */
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_RANDOM_MAC_OUI, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_random_mac_oui::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}

#ifdef _PRE_WLAN_FEATURE_VOWIFI_NAT

oal_uint32 hmac_config_set_vowifi_nat_keep_alive_params(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                        oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;

    if (IS_LEGACY_STA(pst_mac_vap) == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_vowifi_nat_keep_alive_params::vap is not legacy sta.}");
        return OAL_FAIL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_vowifi_nat_keep_alive_params::pst_mac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_VOWIFI_KEEP_ALIVE, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_vowifi_nat_keep_alive_params::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}
#endif


oal_uint32 hmac_config_set_power_rf_ctl(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_POW_RF_CTL, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_power_rf_ctl::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_amsdu_ampdu_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    oal_int32 l_value;
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    mac_tx_large_amsdu_ampdu_stru *tx_large_amsdu = mac_get_tx_large_amsdu_addr();
#endif

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_amsdu_ampdu_switch::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    l_value = *((oal_int32 *)puc_param);

    pst_hmac_vap->en_amsdu_ampdu_active = (oal_uint8)l_value;
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    tx_large_amsdu->uc_cur_amsdu_ampdu_enable[pst_mac_vap->uc_vap_id] = (oal_uint8)l_value;
#endif

    return OAL_SUCC;
}


oal_uint32 hmac_config_auto_ba_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    oal_int32 l_value;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_auto_ba_switch::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    l_value = *((oal_int32 *)puc_param);

    /* 设置自动开启BA会话的开关，0代表关闭，1代表开启 */
    if (l_value == 0) {
        pst_hmac_vap->en_addba_mode = HMAC_ADDBA_MODE_MANUAL;
    } else {
        pst_hmac_vap->en_addba_mode = HMAC_ADDBA_MODE_AUTO;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_addba_req(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_addba_req_param_stru *pst_addba_req;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap;
    mac_action_mgmt_args_stru st_action_args; /* 用于填写ACTION帧的参数 */
    oal_bool_enum_uint8 en_ampdu_support;

    pst_addba_req = (mac_cfg_addba_req_param_stru *)puc_param;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_addba_req::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取用户对应的索引 */
    pst_hmac_user = mac_vap_get_hmac_user_by_addr(pst_mac_vap, pst_addba_req->auc_mac_addr);
    if (pst_hmac_user == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_addba_req::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 建立BA会话，是否需要判断VAP的AMPDU的支持情况，因为需要实现建立BA会话时，一定发AMPDU */
    en_ampdu_support = hmac_user_xht_support(pst_hmac_user);
    /* 手动创建ba会话，不受其他额外限制 */
    if (en_ampdu_support) {
        /*
            建立BA会话时，st_action_args(ADDBA_REQ)结构各个成员意义如下
            (1)uc_category:action的类别
            (2)uc_action:BA action下的类别
            (3)ul_arg1:BA会话对应的TID
            (4)ul_arg2:BUFFER SIZE大小
            (5)ul_arg3:BA会话的确认策略
            (6)ul_arg4:TIMEOUT时间
 */
        st_action_args.uc_category = MAC_ACTION_CATEGORY_BA;
        st_action_args.uc_action = MAC_BA_ACTION_ADDBA_REQ;
        st_action_args.ul_arg1 = pst_addba_req->uc_tidno;     /* 该数据帧对应的TID号 */
        st_action_args.ul_arg2 = pst_addba_req->us_buff_size; /* ADDBA_REQ中，buffer_size的默认大小 */
        st_action_args.ul_arg3 = pst_addba_req->en_ba_policy; /* BA会话的确认策略 */
        st_action_args.ul_arg4 = pst_addba_req->us_timeout;   /* BA会话的超时时间设置为0 */

        /* 建立BA会话 */
        hmac_mgmt_tx_action(pst_hmac_vap, pst_hmac_user, &st_action_args);
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_delba_req(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_delba_req_param_stru *pst_delba_req;
    hmac_user_stru *pst_hmac_user;
    hmac_vap_stru *pst_hmac_vap;
    mac_action_mgmt_args_stru st_action_args; /* 用于填写ACTION帧的参数 */
    hmac_tid_stru *pst_hmac_tid = OAL_PTR_NULL;

    pst_delba_req = (mac_cfg_delba_req_param_stru *)puc_param;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);

    /* 获取用户对应的索引 */
    pst_hmac_user = mac_vap_get_hmac_user_by_addr(pst_mac_vap, pst_delba_req->auc_mac_addr);
    if (pst_hmac_user == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_delba_req::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_tid = &(pst_hmac_user->ast_tid_info[pst_delba_req->uc_tidno]);

    /* 查看会话是否存在 */
    if (pst_delba_req->en_direction == MAC_RECIPIENT_DELBA) {
        if (pst_hmac_tid->pst_ba_rx_info == OAL_PTR_NULL) {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_delba_req::the rx hdl[tid_no=%d] is not exist.}", pst_delba_req->uc_tidno);
            return OAL_SUCC;
        }
    } else {
        if (pst_hmac_tid->st_ba_tx_info.en_ba_status == DMAC_BA_INIT) {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_delba_req::the tx hdl[tid_no=%d] is not exist.}", pst_delba_req->uc_tidno);
            return OAL_SUCC;
        }
    }

    /*
        建立BA会话时，st_action_args(DELBA_REQ)结构各个成员意义如下
        (1)uc_category:action的类别
        (2)uc_action:BA action下的类别
        (3)ul_arg1:BA会话对应的TID
        (4)ul_arg2:删除ba会话的发起端
        (5)ul_arg3:删除ba会话的原因
        (6)ul_arg5:ba会话对应的用户
 */
    st_action_args.uc_category = MAC_ACTION_CATEGORY_BA;
    st_action_args.uc_action = MAC_BA_ACTION_DELBA;
    st_action_args.ul_arg1 = pst_delba_req->uc_tidno;      /* 该数据帧对应的TID号 */
    st_action_args.ul_arg2 = pst_delba_req->en_direction;  /* ADDBA_REQ中，buffer_size的默认大小 */
    st_action_args.ul_arg3 = MAC_QSTA_TIMEOUT;            /* BA会话的确认策略 */
    st_action_args.puc_arg5 = pst_delba_req->auc_mac_addr; /* ba会话对应的user */

    /* 建立BA会话 */
    hmac_mgmt_tx_action(pst_hmac_vap, pst_hmac_user, &st_action_args);

    return OAL_SUCC;
}


oal_uint32 hmac_config_amsdu_start(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_amsdu_start_param_stru *pst_amsdu_param;
    hmac_user_stru *pst_hmac_user;
    oal_uint8 uc_tid_index;

    pst_amsdu_param = (mac_cfg_amsdu_start_param_stru *)puc_param;

    /* 获取用户对应的索引 */
    pst_hmac_user = mac_vap_get_hmac_user_by_addr(pst_mac_vap, pst_amsdu_param->auc_mac_addr);
    if (pst_hmac_user == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_amsdu_start::pst_hamc_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (uc_tid_index = 0; uc_tid_index < WLAN_WME_MAX_TID_NUM; uc_tid_index++) {
        hmac_amsdu_set_maxnum(&pst_hmac_user->ast_hmac_amsdu[uc_tid_index], pst_amsdu_param->uc_amsdu_max_num);
        hmac_amsdu_set_maxsize(&pst_hmac_user->ast_hmac_amsdu[uc_tid_index], pst_hmac_user,
                               pst_amsdu_param->us_amsdu_max_size);
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_user_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_user_stru *pst_hmac_user;
    oal_uint32 ul_ret;
    oal_uint8 uc_tid_index;
    oam_output_type_enum_uint8 en_output_type = OAM_OUTPUT_TYPE_BUTT;
    mac_cfg_user_info_param_stru *pst_hmac_event;

    pst_hmac_event = (mac_cfg_user_info_param_stru *)puc_param;
    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(pst_hmac_event->us_user_idx);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{pst_hmac_user null[%d].}", pst_hmac_event->us_user_idx);
        return OAL_FAIL;
    }

    oam_get_output_type(&en_output_type);
    if (en_output_type != OAM_OUTPUT_TYPE_SDT) {
        OAL_IO_PRINT("en_user_asoc_state :  %d \n", pst_hmac_user->st_user_base_info.en_user_asoc_state);
        OAL_IO_PRINT("uc_is_wds :           %d \n", pst_hmac_user->uc_is_wds);
        OAL_IO_PRINT("us_amsdu_maxsize :    %d \n", pst_hmac_user->us_amsdu_maxsize);
        OAL_IO_PRINT("11ac2g :              %d \n", pst_hmac_user->st_hmac_cap_info.bit_11ac2g);
        OAL_IO_PRINT("\n");

        for (uc_tid_index = 0; uc_tid_index < 8; uc_tid_index++) { /* ast_hmac_amsdu数组长度是8 */
            OAL_IO_PRINT("tid               %d \n", uc_tid_index);
            OAL_IO_PRINT("uc_amsdu_maxnum : %d \n", pst_hmac_user->ast_hmac_amsdu[uc_tid_index].uc_amsdu_maxnum);
            OAL_IO_PRINT("us_amsdu_maxsize :%d \n", pst_hmac_user->ast_hmac_amsdu[uc_tid_index].us_amsdu_maxsize);
            OAL_IO_PRINT("us_amsdu_size :   %d \n", pst_hmac_user->ast_hmac_amsdu[uc_tid_index].us_amsdu_size);
            OAL_IO_PRINT("uc_msdu_num :     %d \n", pst_hmac_user->ast_hmac_amsdu[uc_tid_index].uc_msdu_num);
            OAL_IO_PRINT("\n");
        }

        OAL_IO_PRINT("us_user_hash_idx :    %d \n", pst_hmac_user->st_user_base_info.us_user_hash_idx);
        OAL_IO_PRINT("us_assoc_id :         %d \n", pst_hmac_user->st_user_base_info.us_assoc_id);
        OAL_IO_PRINT("uc_vap_id :           %d \n", pst_hmac_user->st_user_base_info.uc_vap_id);
        OAL_IO_PRINT("uc_device_id :        %d \n", pst_hmac_user->st_user_base_info.uc_device_id);
        OAL_IO_PRINT("uc_chip_id :          %d \n", pst_hmac_user->st_user_base_info.uc_chip_id);
        OAL_IO_PRINT("uc_amsdu_supported :  %d \n", pst_hmac_user->uc_amsdu_supported);
        OAL_IO_PRINT("uc_htc_support :      %d \n", pst_hmac_user->st_user_base_info.st_ht_hdl.uc_htc_support);
        OAL_IO_PRINT("en_ht_support :       %d \n", pst_hmac_user->st_user_base_info.st_ht_hdl.en_ht_capable);
        OAL_IO_PRINT("short gi 20 40 80:    %d %d %d \n", pst_hmac_user->st_user_base_info.st_ht_hdl.bit_short_gi_20mhz,
                     pst_hmac_user->st_user_base_info.st_ht_hdl.bit_short_gi_40mhz,
                     pst_hmac_user->st_user_base_info.st_vht_hdl.bit_short_gi_80mhz);
        OAL_IO_PRINT("\n");

        OAL_IO_PRINT("Privacy info : \r\n");
        OAL_IO_PRINT("    port_valid   :                     %d \r\n",
                     pst_hmac_user->st_user_base_info.en_port_valid);
        OAL_IO_PRINT("    user_tx_info.security.cipher_key_type:      %s \r\n"
                     "    user_tx_info.security.cipher_protocol_type: %s \r\n",
                     hmac_config_keytype2string(pst_hmac_user->
                                                st_user_base_info.st_user_tx_info.st_security.en_cipher_key_type),
                     hmac_config_cipher2string(pst_hmac_user->st_user_base_info.st_key_info.en_cipher_type));

        if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
            OAL_IO_PRINT("    STA:cipher_type :                           %s \r\n",
                         hmac_config_cipher2string(pst_hmac_user->st_user_base_info.st_key_info.en_cipher_type));
        }
        OAL_IO_PRINT("\n");
    } else {
        oam_ota_report((oal_uint8 *)pst_hmac_user,
                       (oal_uint16)(OAL_SIZEOF(hmac_user_stru) - OAL_SIZEOF(mac_user_stru)),
                       0, 0, OAM_OTA_TYPE_HMAC_USER);
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_USER_INFO, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}
#ifdef _PRE_WLAN_FEATURE_VOWIFI

oal_uint32 hmac_config_vowifi_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    mac_cfg_vowifi_stru *pst_cfg_vowifi;

    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_vowifi_info::null param,pst_mac_vap=%x puc_param=%x.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_vap->pst_vowifi_cfg_param == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_vowifi_info::pst_vowifi_cfg_param is null.}");
        return OAL_SUCC;
    }

    pst_cfg_vowifi = (mac_cfg_vowifi_stru *)puc_param;

    ul_ret = mac_vap_set_vowifi_param(pst_mac_vap, pst_cfg_vowifi->en_vowifi_cfg_cmd, pst_cfg_vowifi->uc_value);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vowifi_info::param[%d] set failed[%d].}",
                         pst_cfg_vowifi->en_vowifi_cfg_cmd, ul_ret);
        return ul_ret;
    }

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_VOWIFI,
                     "{hmac_config_vowifi_info::Mode[%d],rssi_thres[%d],period_ms[%d],trigger_count[%d].}",
                     pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_mode,
                     ((pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_mode == VOWIFI_LOW_THRES_REPORT) ?
                     pst_mac_vap->pst_vowifi_cfg_param->c_rssi_low_thres :
                     pst_mac_vap->pst_vowifi_cfg_param->c_rssi_high_thres),
                     pst_mac_vap->pst_vowifi_cfg_param->us_rssi_period_ms,
                     pst_mac_vap->pst_vowifi_cfg_param->uc_trigger_count_thres);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_VOWIFI_INFO, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_user_info::hmac_config_vowifi_info failed[%d].}", ul_ret);
    }

    return ul_ret;
}
#endif /* _PRE_WLAN_FEATURE_VOWIFI */
#ifdef _PRE_WLAN_FEATURE_IP_FILTER

oal_uint32 hmac_config_update_ip_filter(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    dmac_tx_event_stru *pst_tx_event;
    frw_event_mem_stru *pst_event_mem;
    oal_netbuf_stru *pst_netbuf_cmd;
    frw_event_stru *pst_hmac_to_dmac_ctx_event;

    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_update_ip_filter::null param,pst_mac_vap=%x puc_param=%x.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_netbuf_cmd = *((oal_netbuf_stru **)puc_param);
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    pst_event_mem = frw_event_alloc_m(OAL_SIZEOF(dmac_tx_event_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_update_ip_filter::pst_event_mem null.}");
        oal_netbuf_free(pst_netbuf_cmd);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_to_dmac_ctx_event = (frw_event_stru *)pst_event_mem->puc_data;
    frw_event_hdr_init(&(pst_hmac_to_dmac_ctx_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_IP_FILTER,
                       OAL_SIZEOF(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    pst_tx_event = (dmac_tx_event_stru *)(pst_hmac_to_dmac_ctx_event->auc_event_data);
    pst_tx_event->pst_netbuf = pst_netbuf_cmd;
    pst_tx_event->us_frame_len = oal_netbuf_len(pst_netbuf_cmd);

    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_update_ip_filter::frw_event_dispatch_event failed[%d].}", ul_ret);
    }
    oal_netbuf_free(pst_netbuf_cmd);
    frw_event_free_m(pst_event_mem);

    return ul_ret;
}
oal_uint32 hmac_config_assigned_filter(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_assigned_filter::null param,pst_mac_vap=%x puc_param=%x.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_ASSIGNED_FILTER, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_assigned_filter:h2d fail[%d].", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

#endif  // _PRE_WLAN_FEATURE_IP_FILTER

oal_uint32 hmac_config_kick_user(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint16 us_user_idx;
    oal_uint32 ul_ret;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_bool_enum_uint8 en_is_protected = OAL_FALSE;
    oal_dlist_head_stru *pst_entry = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_user_list_head = OAL_PTR_NULL;
    mac_user_stru *pst_user_tmp = OAL_PTR_NULL;
    hmac_user_stru *pst_hmac_user_tmp = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    mac_cfg_kick_user_param_stru *pst_kick_user_param = (mac_cfg_kick_user_param_stru *)puc_param;

    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_kick_user::en_vap_mode is CONFIG.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (oal_unlikely(pst_hmac_vap == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{kick user mac[%02X:XX:XX:XX:%02X:%02X] reacon code[%d]}",
                     /* auc_mac_addr第0、4byte为参数输出打印 */
                     pst_kick_user_param->auc_mac_addr[0], pst_kick_user_param->auc_mac_addr[4],
                     /* auc_mac_addr第5byte为参数输出打印 */
                     pst_kick_user_param->auc_mac_addr[5], pst_kick_user_param->us_reason_code);

    /* 踢掉全部user */
    if (oal_is_broadcast_ether_addr(pst_kick_user_param->auc_mac_addr)) {
        
#if (_PRE_TEST_MODE == _PRE_TEST_MODE_UT)
        /* STAUT模式不需要发广播去关联帧，staut自身linkloss机制能保证ap突然下电等情况，能及时去关联；另外dfr重启wifi，staut发的此帧可能不释放，平台无法及时睡眠而出现异常 */
        if (IS_AP(pst_mac_vap)) {
            hmac_mgmt_send_disassoc_frame(pst_mac_vap, pst_kick_user_param->auc_mac_addr,
                                          pst_kick_user_param->us_reason_code, OAL_FALSE);
        }
#endif
        /* 遍历vap下所有用户, 删除用户 */
        pst_user_list_head = &(pst_mac_vap->st_mac_user_list_head);
        for (pst_entry = pst_user_list_head->pst_next; pst_entry != pst_user_list_head;) {
            pst_user_tmp = oal_dlist_get_entry(pst_entry, mac_user_stru, st_user_dlist);
            pst_hmac_user_tmp = mac_res_get_hmac_user(pst_user_tmp->us_assoc_id);
            if (oal_unlikely(pst_hmac_user_tmp == OAL_PTR_NULL)) {
                OAM_ERROR_LOG1(0, OAM_SF_CFG, "{kick_user::null param,hmac_user[%d].}", pst_user_tmp->us_assoc_id);
                continue;
            }

            /* 指向双向链表下一个 */
            pst_entry = pst_entry->pst_next;

            /* 管理帧加密是否开启 */
            en_is_protected = pst_user_tmp->st_cap_info.bit_pmf_active;

            /* 发去关联帧 */
            hmac_mgmt_send_disassoc_frame(pst_mac_vap, pst_user_tmp->auc_user_mac_addr,
                                          pst_kick_user_param->us_reason_code, en_is_protected);

            /* 修改 state & 删除 user */
            hmac_handle_disconnect_rsp(pst_hmac_vap, pst_hmac_user_tmp, pst_kick_user_param->us_reason_code);

            /* 删除用户 */
            hmac_user_del(pst_mac_vap, pst_hmac_user_tmp);
        }

        /* VAP下user头指针不应该为空 */
        if (oal_dlist_is_empty(&pst_mac_vap->st_mac_user_list_head) == OAL_FALSE) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{kick_user::st_mac_user_list_head is not empty.}");
        }
        return OAL_SUCC;
    }

    ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap, pst_kick_user_param->auc_mac_addr, &us_user_idx);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{kick_user::find_user_by_macaddr failed[%d].}", ul_ret);
        if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
            hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
        }
        return ul_ret;
    }

    pst_hmac_user = mac_res_get_hmac_user(us_user_idx);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{kick_user::hmac_user null,us_user_idx:%d}", us_user_idx);
        if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
            hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
        }
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_hmac_user->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_ASSOC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{kick_user::user is unassociated,idx:%d}", us_user_idx);
    }

    en_is_protected = pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active;

    /* 发去认证帧 */
    hmac_mgmt_send_disassoc_frame(pst_mac_vap, pst_hmac_user->st_user_base_info.auc_user_mac_addr,
                                  pst_kick_user_param->us_reason_code, en_is_protected);

    /* 修改 state & 删除 user */
    hmac_handle_disconnect_rsp(pst_hmac_vap, pst_hmac_user, pst_kick_user_param->us_reason_code);

    /* 删除用户 */
    hmac_user_del(pst_mac_vap, pst_hmac_user);

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_dscr_param(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_DSCR, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_dscr_param::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_log_level(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_LOG_LEVEL, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_log_level::send_event failed[%d].}",
                         ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_rate(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_RATE, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_rate::send_event failed[%d].}",
                         ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_mcs(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_MCS, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mcs::send_event failed[%d].}",
                         ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_mcsac(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_MCSAC, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mcsac::send_event failed[%d].}",
                         ul_ret);
    }

    return ul_ret;
}

oal_uint32 hmac_config_set_rfch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_RFCH, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_rfch::send_event failed[%d].}",
                         ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_bw(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_BW, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_bw::hmac_config_send_event failed[%d].}",
                         ul_ret);
    }

    return ul_ret;
}

oal_uint32 hmac_config_always_tx_1102(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    mac_cfg_tx_comp_stru *pst_event_set_bcast;

    /* 使能常发 */
    pst_event_set_bcast = (mac_cfg_tx_comp_stru *)puc_param;
    mac_vap_set_al_tx_flag(pst_mac_vap, OAL_SWITCH_OFF);
    if (pst_event_set_bcast->uc_param == OAL_SWITCH_ON) {
        mac_vap_set_al_tx_flag(pst_mac_vap, OAL_SWITCH_ON);
        pst_mac_vap->st_cap_flag.bit_keepalive = OAL_FALSE;
    } else {
        mac_vap_set_al_tx_first_run(pst_mac_vap, OAL_FALSE);
    }
    mac_vap_set_al_tx_payload_flag(pst_mac_vap, pst_event_set_bcast->en_payload_flag);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_ALWAYS_TX_1102, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_always_tx_1102::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_always_tx_num(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_ALWAYS_TX_NUM, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_always_tx_num::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_always_rx(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_ALWAYS_RX, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_always_rx::send_event failed[%d].}",
                         ul_ret);
    }

    return ul_ret;
}

oal_uint32 hmac_config_rx_filter_frag(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_RX_FILTER_FRAG, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_rx_filter_frag::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_list_ap(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_list_ap::invalid vap mode[%d].}",
                         pst_mac_vap->en_vap_mode);
        return OAL_FAIL;
    }

    /* 打印扫描到的bss信息 */
    hmac_scan_print_scanned_bss_info(pst_mac_vap->uc_device_id);

    return OAL_SUCC;
}
oal_uint32 hmac_update_print_info(mac_vap_stru *pst_mac_vap, oal_int8 **pc_print_buff)
{
    oal_int8 *pc_user_asoc_id_addr = "User assoc id         ADDR         Protocol Type \n";
    oal_int32 l_ret;

    /* AP侧的信息才能打印相关信息 */
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{invalid vap_mode[%d].}", pst_mac_vap->en_vap_mode);
        return OAL_FAIL;
    }

    *pc_print_buff = (oal_int8 *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (*pc_print_buff == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(*pc_print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);

    snprintf_s(*pc_print_buff, OAM_REPORT_MAX_STRING_LEN, OAM_REPORT_MAX_STRING_LEN - 1,
        "Total user num is %d \n", pst_mac_vap->us_user_nums);
    l_ret = strncat_s(*pc_print_buff, OAM_REPORT_MAX_STRING_LEN,
        pc_user_asoc_id_addr, OAL_STRLEN(pc_user_asoc_id_addr));
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_config_list_sta::strncat_s error!");
    }
    return OAL_SUCC;
}

oal_uint32 hmac_config_list_sta(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint8 uc_user_idx;
    mac_user_stru *pst_mac_user = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_head = OAL_PTR_NULL;
    wlan_protocol_enum_uint8 en_protocol_mode;
    oal_int8 ac_tmp_buff[256] = { 0 };
    oal_int32 l_remainder_len;
    oal_int8 *pc_print_buff = OAL_PTR_NULL;
    oal_int32 l_ret;
    oal_uint32 ul_ret;

    ul_ret = hmac_update_print_info(pst_mac_vap, &pc_print_buff);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_list_sta::update_print_info fail.}");
        return ul_ret;
    }
    l_remainder_len = (oal_int32)(OAM_REPORT_MAX_STRING_LEN - OAL_STRLEN(pc_print_buff));

    oal_spin_lock_bh(&pst_mac_vap->st_cache_user_lock);
    /* AP侧的USER信息 */
    for (uc_user_idx = 0; uc_user_idx < MAC_VAP_USER_HASH_MAX_VALUE; uc_user_idx++) {
        oal_dlist_search_for_each(pst_head, &(pst_mac_vap->ast_user_hash[uc_user_idx])) {
            /* 找到相应用户 */
            pst_mac_user = (mac_user_stru *)oal_dlist_get_entry(pst_head, mac_user_stru, st_user_hash_dlist);
            if (pst_mac_user == OAL_PTR_NULL) {
                oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_list_sta::pst_mac_user null.}");
                continue;
            }
            /* user结构体下的协议模式不区分a和g，需要根据频段区分 */
            en_protocol_mode = pst_mac_user->en_protocol_mode;
            if (en_protocol_mode >= WLAN_PROTOCOL_BUTT) {
                OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{protocol_mode wrong.}", en_protocol_mode);
                continue;
            }
            if ((en_protocol_mode == WLAN_LEGACY_11G_MODE) && (pst_mac_vap->st_channel.en_band == WLAN_BAND_5G)) {
                en_protocol_mode = WLAN_LEGACY_11A_MODE;
            }
            l_ret = snprintf_s(ac_tmp_buff, OAL_SIZEOF(ac_tmp_buff), OAL_SIZEOF(ac_tmp_buff) - 1,
                "     %d       %02X:XX:XX:%02X:%02X:%02X       %s \n",
                pst_mac_user->us_assoc_id, pst_mac_user->auc_user_mac_addr[0], /* auc_user_mac_addr第0、3、4byte输出打印 */
                pst_mac_user->auc_user_mac_addr[3], pst_mac_user->auc_user_mac_addr[4],
                /* auc_user_mac_addr第5byte为参数输出打印 */
                pst_mac_user->auc_user_mac_addr[5], g_st_protocol_mode_list[en_protocol_mode].puc_protocol_desc);
            if (l_ret < 0) {
                OAM_ERROR_LOG0(0, OAM_SF_CFG, "hmac_config_list_sta::snprintf_s error!");
            }
            l_ret = strncat_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, ac_tmp_buff, l_remainder_len - 1);
            if (l_ret != EOK) {
                OAM_ERROR_LOG0(0, OAM_SF_CFG, "hmac_config_list_sta::strncat_s error!");
            }
            memset_s(ac_tmp_buff, OAL_SIZEOF(ac_tmp_buff), 0, OAL_SIZEOF(ac_tmp_buff));
            l_remainder_len = (oal_int32)(OAM_REPORT_MAX_STRING_LEN - OAL_STRLEN(pc_print_buff));
        }
    }
    oal_spin_unlock_bh(&pst_mac_vap->st_cache_user_lock);

    oam_print(pc_print_buff);
    oal_mem_free_m(pc_print_buff, OAL_TRUE);
    return OAL_SUCC;
}


oal_uint32 hmac_config_get_sta_list(mac_vap_stru *pst_mac_vap, oal_uint16 *us_len, oal_uint8 *puc_param)
{
    oal_uint8 uc_user_idx;
    mac_user_stru *pst_mac_user = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_head = OAL_PTR_NULL;
    oal_int8 ac_tmp_buff[256] = { 0 };
    oal_int32 l_remainder_len;
    oal_int8 *pc_sta_list_buff = OAL_PTR_NULL;
    oal_netbuf_stru *pst_netbuf = OAL_PTR_NULL;
    oal_uint32 ul_skb_len;

    /* 事件传递指针值，此处异常返回前将其置为NULL */
    *(oal_ulong *)puc_param = (oal_ulong)OAL_PTR_NULL;

    /* AP侧的信息才能打印相关信息 */
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{invalid en_vap_mode[%d].}", pst_mac_vap->en_vap_mode);
        return OAL_FAIL;
    }

    pc_sta_list_buff = (oal_int8 *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (pc_sta_list_buff == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_get_sta_list, oal_mem_alloc_m failed.\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(pc_sta_list_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
    l_remainder_len = (oal_int32)(OAM_REPORT_MAX_STRING_LEN - OAL_STRLEN(pc_sta_list_buff));

    oal_spin_lock_bh(&pst_mac_vap->st_cache_user_lock);

    /* AP侧的USER信息 */
    for (uc_user_idx = 0; uc_user_idx < MAC_VAP_USER_HASH_MAX_VALUE; uc_user_idx++) {
        oal_dlist_search_for_each(pst_head, &(pst_mac_vap->ast_user_hash[uc_user_idx])) {
            /* 找到相应用户 */
            pst_mac_user = (mac_user_stru *)oal_dlist_get_entry(pst_head, mac_user_stru, st_user_hash_dlist);
            if (pst_mac_user == OAL_PTR_NULL) {
                oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_sta_list::pst_mac_user null.}");
                continue;
            }
            /* 检查用户关联状态 */
            if (pst_mac_user->en_user_asoc_state != MAC_USER_STATE_ASSOC) {
                continue;
            }
            snprintf_s(ac_tmp_buff, OAL_SIZEOF(ac_tmp_buff), OAL_SIZEOF(ac_tmp_buff) - 1,
                "%02X:%02X:%02X:%02X:%02X:%02X\n",
                /* auc_user_mac_addr第0、1byte为参数输出打印 */
                pst_mac_user->auc_user_mac_addr[0], pst_mac_user->auc_user_mac_addr[1],
                /* auc_user_mac_addr第2、3byte为参数输出打印 */
                pst_mac_user->auc_user_mac_addr[2], pst_mac_user->auc_user_mac_addr[3],
                /* auc_user_mac_addr第4、5byte为参数输出打印 */
                pst_mac_user->auc_user_mac_addr[4], pst_mac_user->auc_user_mac_addr[5]);

            if (strncat_s(pc_sta_list_buff, OAM_REPORT_MAX_STRING_LEN, ac_tmp_buff, l_remainder_len - 1) != EOK) {
                OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_get_sta_list::strncat_s error!");
            }
            memset_s(ac_tmp_buff, OAL_SIZEOF(ac_tmp_buff), 0, OAL_SIZEOF(ac_tmp_buff));
            l_remainder_len = (oal_int32)(OAM_REPORT_MAX_STRING_LEN - OAL_STRLEN(pc_sta_list_buff));
        }
    }
    oal_spin_unlock_bh(&pst_mac_vap->st_cache_user_lock);

    ul_skb_len = OAL_STRLEN(pc_sta_list_buff);
    pst_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, ul_skb_len, OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf != OAL_PTR_NULL) {
        if ((memcpy_s(oal_netbuf_put(pst_netbuf, ul_skb_len), ul_skb_len, pc_sta_list_buff, ul_skb_len) != EOK) &&
            (ul_skb_len != 0)) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_get_sta_list::memcpy fail!");
        }
    } else {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "Alloc netbuf(%d) fail!", ul_skb_len);
    }

    *(oal_ulong *)puc_param = (oal_ulong)(uintptr_t)pst_netbuf;

    /* 事件传递指针，此处记录指针长度 */
    *us_len = (oal_uint16)OAL_SIZEOF(oal_netbuf_stru *);

    oal_mem_free_m(pc_sta_list_buff, OAL_TRUE);
    return OAL_SUCC;
}


oal_uint32 hmac_tx_data_send_event(mac_vap_stru *pst_vap, oal_netbuf_stru *pst_mgmt_frame, oal_uint16 us_frame_len)
{
    oal_uint32 ul_return;
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    dmac_tx_event_stru *pst_ctx_stru = OAL_PTR_NULL;

    if ((pst_vap == OAL_PTR_NULL) || (pst_mgmt_frame == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG,
                       "{hmac_tx_data_send_event::param null, %x %x.}",
                       (uintptr_t)pst_vap, (uintptr_t)pst_mgmt_frame);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛事件给DMAC,让DMAC完成配置VAP创建 */
    pst_event_mem = frw_event_alloc_m(OAL_SIZEOF(dmac_tx_event_stru));
    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_tx_data_send_event::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_DRX,
                       DMAC_TX_HOST_DRX,
                       OAL_SIZEOF(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_vap->uc_chip_id,
                       pst_vap->uc_device_id,
                       pst_vap->uc_vap_id);

    pst_ctx_stru = (dmac_tx_event_stru *)pst_event->auc_event_data;
    pst_ctx_stru->pst_netbuf = pst_mgmt_frame;
    pst_ctx_stru->us_frame_len = us_frame_len;

    ul_return = frw_event_dispatch_event(pst_event_mem);
    if (ul_return != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_tx_data_send_event::frw_event_dispatch_event failed[%d].}", ul_return);
        frw_event_free_m(pst_event_mem);
        return ul_return;
    }

    /* 释放事件 */
    frw_event_free_m(pst_event_mem);

    return OAL_SUCC;
}

oal_void hmac_encap_check_data(oal_netbuf_stru *pst_netbuf, oal_uint8 uc_type, oal_uint8 *puc_mac_bssid,
                               oal_uint8 *puc_mac_sa)
{
    oal_uint16 us_fc;
    oal_uint16 us_index = SNAP_LLC_FRAME_LEN;
    oal_uint8 *puc_mac_header;
    oal_uint8 *puc_mac_payload = OAL_PTR_NULL;
    mac_llc_snap_stru *pst_snap = OAL_PTR_NULL;

    /*************************************************************************/
    /* Frame Header */
    /* -------------------------------------------------------------------- */
    /* |Frame Control|Duration|   DA    |    SA    |BSSID|Sequence Control| */
    /* -------------------------------------------------------------------- */
    /* | 2           |2       |    6    |    6     |6    |2               | */
    /* -------------------------------------------------------------------- */
    /* Frame Body */
    /* -------------------------------------------------------------------- */
    /* |dsap|ssap|control|org code|ether type|datatype|datatype| data |FCS| */
    /* -------------------------------------------------------------------- */
    /* | 1  | 1  |   1   |    3   |     2    |    1   |    1   | 1550 | 4 | */
    /* -------------------------------------------------------------------- */
    /*************************************************************************/
    puc_mac_header = (oal_uint8 *)(oal_netbuf_header(pst_netbuf));

    us_fc = (WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_QOS | WLAN_FRAME_TO_AP);
    mac_hdr_set_frame_control(puc_mac_header, us_fc);

    /* 设置 BSSID address1: AP MAC地址(BSSID) */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR1_OFFSET, puc_mac_bssid);

    /* 设置 SA address2: dot11MACAddress */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR2_OFFSET, puc_mac_sa);

    /* 设置 DA address3: AP MAC地址 */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR3_OFFSET, BROADCAST_MACADDR);

    puc_mac_payload = puc_mac_header + MAC_80211_QOS_FRAME_LEN;

    /* 填充snap头 */
    pst_snap = (mac_llc_snap_stru *)puc_mac_payload;

    pst_snap->uc_llc_dsap = SNAP_LLC_LSAP;
    pst_snap->uc_llc_ssap = SNAP_LLC_LSAP;
    pst_snap->uc_control = LLC_UI;
    pst_snap->auc_org_code[0] = SNAP_RFC1042_ORGCODE_0;
    pst_snap->auc_org_code[1] = SNAP_RFC1042_ORGCODE_1;
    pst_snap->auc_org_code[2] = SNAP_RFC1042_ORGCODE_2; /* auc_org_code第2字节置零 */
    /* ether_type使用自定义的type ,防止AP根据type对帧内容做校验 */
    pst_snap->us_ether_type = oal_ntoh_16(ETHER_TYPE_PACKET_CHECK);

    /* 根据type将data区填充为和type一致的内容 */
    for (; us_index < WLAN_PACKET_CHECK_DATA_LEN; us_index++) {
        puc_mac_payload[us_index] = uc_type;
    }
}


oal_uint32 hmac_config_send_check_data(mac_vap_stru *pst_mac_vap, oal_uint8 uc_type)
{
    oal_uint32 ul_ret;
    oal_netbuf_stru *pst_netbuf = OAL_PTR_NULL;
    oal_uint8 *puc_mac_header = OAL_PTR_NULL;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;

    if (pst_mac_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_netbuf = (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_LARGE_NETBUF_SIZE,
                                                         OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_send_check_data::Alloc netbuf null in normal_netbuf.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    memset_s(oal_netbuf_cb(pst_netbuf), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    /* 将mac header清零 */
    memset_s((oal_uint8 *)oal_netbuf_header(pst_netbuf), MAC_80211_QOS_FRAME_LEN, 0, MAC_80211_QOS_FRAME_LEN);
    puc_mac_header = (oal_uint8 *)(oal_netbuf_header(pst_netbuf));

    hmac_encap_check_data(pst_netbuf,
                          uc_type,
                          pst_mac_vap->auc_bssid,
                          pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);

    oal_netbuf_put(pst_netbuf, (WLAN_PACKET_CHECK_DATA_LEN + MAC_80211_QOS_FRAME_LEN));

    oal_netbuf_push(pst_netbuf, (MAC_80211_QOS_HTC_4ADDR_FRAME_LEN - MAC_80211_QOS_FRAME_LEN));

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    pst_tx_ctl->us_mpdu_len = WLAN_PACKET_CHECK_DATA_LEN;
    pst_tx_ctl->uc_mpdu_num = 1;
    pst_tx_ctl->uc_netbuf_num = 1;
    pst_tx_ctl->en_frame_type = WLAN_DATA_BASICTYPE;
    pst_tx_ctl->en_is_probe_data = DMAC_USER_ALG_NON_PROBE;
    pst_tx_ctl->en_ack_policy = WLAN_TX_NORMAL_ACK;
    pst_tx_ctl->uc_tx_vap_index = pst_mac_vap->uc_vap_id;
    pst_tx_ctl->us_tx_user_idx = pst_mac_vap->uc_assoc_vap_id;
    pst_tx_ctl->uc_ac = WLAN_WME_AC_VI;
    pst_tx_ctl->uc_tid = WLAN_WME_AC_TO_TID(pst_tx_ctl->uc_ac);
    pst_tx_ctl->bit_80211_mac_head_type = 1; /* 指示mac头部在skb中 */
    pst_tx_ctl->pst_frame_header = (mac_ieee80211_frame_stru *)puc_mac_header;
    pst_tx_ctl->uc_frame_header_length = MAC_80211_QOS_FRAME_LEN;
    pst_tx_ctl->en_is_amsdu = OAL_FALSE;

    oal_netbuf_next(pst_netbuf) = OAL_PTR_NULL;
    oal_netbuf_prev(pst_netbuf) = OAL_PTR_NULL;

    /* Buffer this frame in the Memory Queue for transmission */
    ul_ret = hmac_tx_data_send_event(pst_mac_vap, pst_netbuf, (WLAN_PACKET_CHECK_DATA_LEN + MAC_80211_QOS_FRAME_LEN));
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_send_check_data::hmac_tx_data_send_event failed[%d].}", ul_ret);
        return ul_ret;
    }
    return OAL_SUCC;
}


oal_uint32 hmac_config_dump_all_rx_dscr(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DUMP_ALL_RX_DSCR, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_dump_all_rx_dscr::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_vap_pkt_stat(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    return OAL_SUCC;
}


oal_uint32 hmac_config_set_country(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_country_stru *pst_country_param;
    mac_regdomain_info_stru *pst_mac_regdom;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    oal_uint8 uc_rc_num;
    oal_uint32 ul_size;

    pst_country_param = (mac_cfg_country_stru *)puc_param;
    pst_mac_regdom = (mac_regdomain_info_stru *)pst_country_param->p_mac_regdom;
    if (pst_mac_regdom == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_country::pst_mac_regdom null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_regdomain_set_country(us_len, puc_param);

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        if (pst_mac_regdom != OAL_PTR_NULL) {
            oal_mem_free_m(pst_mac_regdom, OAL_TRUE);
            pst_mac_regdom = OAL_PTR_NULL;
        }
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_country::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    /* 获取管制类的个数 */
    uc_rc_num = pst_mac_regdom->uc_regclass_num;

    /* 计算配置命令 */
    ul_size = (oal_uint32)(OAL_SIZEOF(mac_regclass_info_stru) * uc_rc_num + MAC_RD_INFO_LEN);

    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_COUNTRY, (oal_uint16)ul_size,
                                    (oal_uint8 *)pst_mac_regdom);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        if (pst_mac_regdom != OAL_PTR_NULL) {
            oal_mem_free_m(pst_mac_regdom, OAL_TRUE);
            pst_mac_regdom = OAL_PTR_NULL;
        }
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_country::hmac_config_send_event failed[%d].}", ul_ret);

        return ul_ret;
    }

    /* WAL层抛内存下来，此处释放 */
    if (pst_mac_regdom != OAL_PTR_NULL) {
        oal_mem_free_m(pst_mac_regdom, OAL_TRUE);
        pst_mac_regdom = OAL_PTR_NULL;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_amsdu_tx_on(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
#ifdef _PRE_WLAN_FEATURE_AMSDU
    mac_cfg_ampdu_tx_on_param_stru *pst_ampdu_tx_on_param;
    hmac_vap_stru *pst_hmac_vap;

    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_config_set_amsdu_tx_on:: parma null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_amsdu_tx_on::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_ampdu_tx_on_param = (mac_cfg_ampdu_tx_on_param_stru *)puc_param;

    pst_hmac_vap->en_amsdu_active = pst_ampdu_tx_on_param->uc_aggr_tx_on;
#endif

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_ampdu_tx_on(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_ampdu_tx_on_param_stru *pst_ampdu_tx_on_param = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;

    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_config_set_ampdu_tx_on:: param null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_ampdu_tx_on::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ampdu_tx_on_param = (mac_cfg_ampdu_tx_on_param_stru *)puc_param;

    pst_hmac_vap->en_ampdu_tx_on_switch = pst_ampdu_tx_on_param->uc_aggr_tx_on;
    oam_info_log1(0, OAM_SF_CFG, "{hmac_config_set_ampdu_tx_on:: en_tx_aggr_on[%d] null!}\r\n",
                  pst_hmac_vap->en_tx_aggr_on);

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_regdomain_pwr(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_regdomain_max_pwr_stru *pst_cfg;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint32 ul_ret;
#endif
    pst_cfg = (mac_cfg_regdomain_max_pwr_stru *)puc_param;

    mac_regdomain_set_max_power(pst_cfg->uc_pwr, pst_cfg->en_exceed_reg);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_REGDOMAIN_PWR, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_regdomain_pwr::hmac_config_send_event failed[%d].}", ul_ret);

        return ul_ret;
    }

#endif
    return OAL_SUCC;
}

oal_uint32 hmac_config_reduce_sar(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_REDUCE_SAR, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_config_reduce_sar::hmac_config_send_event failed, error no[%d]!", ul_ret);
        return ul_ret;
    }
    return OAL_SUCC;
}

oal_uint32 hmac_config_get_country(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_regdomain_info_stru *pst_regdomain_info = OAL_PTR_NULL;
    mac_cfg_get_country_stru *pst_param;

    pst_param = (mac_cfg_get_country_stru *)puc_param;

    mac_get_regdomain_info(&pst_regdomain_info);

    pst_param->ac_country[0] = pst_regdomain_info->ac_country[0];
    pst_param->ac_country[1] = pst_regdomain_info->ac_country[1];
    pst_param->ac_country[2] = pst_regdomain_info->ac_country[2]; /* 获取国家字符串（ac_country的第2byte） */
    *pus_len = WLAN_COUNTRY_STR_LEN;

#else
    oal_int8 *pc_curr_cntry;
    mac_cfg_get_country_stru *pst_param;

    pst_param = (mac_cfg_get_country_stru *)puc_param;

    pc_curr_cntry = mac_regdomain_get_country();

    pst_param->ac_country[0] = pc_curr_cntry[0];
    pst_param->ac_country[1] = pc_curr_cntry[1];
    pst_param->ac_country[2] = pc_curr_cntry[2]; /* 获取当前国家字符串（pc_curr_cntry的第2byte） */

    *pus_len = OAL_SIZEOF(mac_cfg_get_country_stru);

    oam_info_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_country::country[0]=%c, country[1]=%c.}",
                  (oal_uint8)pst_param->ac_country[0], (oal_uint8)pst_param->ac_country[1]);
#endif

    oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_get_country");

    return OAL_SUCC;
}


OAL_STATIC oal_void hmac_free_connect_param_resource(mac_cfg80211_connect_param_stru *pst_conn_param)
{
    if (pst_conn_param->puc_wep_key != OAL_PTR_NULL) {
        oal_free(pst_conn_param->puc_wep_key);
        pst_conn_param->puc_wep_key = OAL_PTR_NULL;
    }
    if (pst_conn_param->puc_ie != OAL_PTR_NULL) {
        oal_free(pst_conn_param->puc_ie);
        pst_conn_param->puc_ie = OAL_PTR_NULL;
    }
}
 
#ifdef _PRE_WLAN_FEATURE_ROAM
oal_uint32 hmac_roam_start_reconnect(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_target_bssid)
{
    oal_uint32 ul_ret;
    hmac_roam_info_stru *pst_roam_info = OAL_PTR_NULL;

    oam_warning_log3(0, OAM_SF_CFG,
                     "{hmac_roam_start_reconnect:: roaming AP with ressoc frame, %02X:XX:XX:XX:%02X:%02X}",
                     puc_target_bssid[0], puc_target_bssid[4], puc_target_bssid[5]); /* 打印输出target bssid第0、4、5byte */

    /* 相同bssid时，走重关联流程 */
    if (oal_memcmp(pst_hmac_vap->st_vap_base_info.auc_bssid, puc_target_bssid, OAL_MAC_ADDR_LEN) == 0) {
        /* 目标BSSID与当前BSSID相同，支持重关联 */
        ul_ret = hmac_roam_start(pst_hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_FALSE, ROAM_TRIGGER_APP);
    } else {
        /* roaming */
        pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
        if (!ether_is_all_zero(puc_target_bssid) && (pst_roam_info != OAL_PTR_NULL)) {
            oal_set_mac_addr(pst_roam_info->auc_target_bssid, puc_target_bssid);
            ul_ret = hmac_roam_start(pst_hmac_vap, ROAM_SCAN_CHANNEL_ORG_BUTT, OAL_TRUE, ROAM_TRIGGER_BSSID);
        } else {
            ul_ret = hmac_roam_start(pst_hmac_vap, ROAM_SCAN_CHANNEL_ORG_BUTT, OAL_TRUE, ROAM_TRIGGER_APP);
        }
    }
    return ul_ret;
}
#endif

oal_void hmac_connect_set_security(mac_cfg80211_connect_param_stru *pst_connect_param,
                                   mac_cfg80211_connect_security_stru *pst_conn_sec)
{
    pst_conn_sec->uc_wep_key_len = pst_connect_param->uc_wep_key_len;
    pst_conn_sec->en_auth_type = pst_connect_param->en_auth_type;
    pst_conn_sec->en_privacy = pst_connect_param->en_privacy;
    pst_conn_sec->st_crypto = pst_connect_param->st_crypto;
    pst_conn_sec->uc_wep_key_index = pst_connect_param->uc_wep_key_index;
    pst_conn_sec->en_mgmt_proteced = pst_connect_param->en_mfp;
    if (pst_conn_sec->uc_wep_key_len > WLAN_WEP104_KEY_LEN) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,
                       "{hmac_config_connect:: wep_key_len[%d] > WLAN_WEP104_KEY_LEN!}", pst_conn_sec->uc_wep_key_len);
        pst_conn_sec->uc_wep_key_len = WLAN_WEP104_KEY_LEN;
    }
}
OAL_STATIC oal_void hmac_config_set_ressoc_flag(hmac_vap_stru *pst_hmac_vap,
                                                mac_cfg80211_connect_param_stru *pst_connect_param,
                                                mac_bss_dscr_stru *pst_bss_dscr)
{
#ifdef _PRE_WLAN_FEATURE_HS20
    if (oal_memcmp(pst_hmac_vap->st_vap_base_info.auc_bssid, pst_connect_param->auc_bssid, OAL_MAC_ADDR_LEN) ||
        (hmac_interworking_check(pst_hmac_vap, (oal_uint8 *)pst_bss_dscr))) {
        pst_hmac_vap->bit_reassoc_flag = OAL_FALSE;
    } else {
        pst_hmac_vap->bit_reassoc_flag = OAL_TRUE;
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_ressoc_flag:: assoc ap with ressoc frame.}");
    }
#endif  // _PRE_WLAN_FEATURE_HS20
    pst_hmac_vap->bit_reassoc_flag = OAL_FALSE;
}

oal_uint32 hmac_config_connect(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    mac_bss_dscr_stru *pst_bss_dscr = OAL_PTR_NULL;
    mac_cfg80211_connect_param_stru *pst_connect_param = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_11R
    oal_uint8 *puc_mde;
#endif
    oal_app_ie_stru st_app_ie;
    mac_cfg80211_connect_security_stru st_conn_sec;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;

#ifdef _PRE_WLAN_FEATURE_WAPI
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
#endif
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_int32 l_ret;

    if (oal_unlikely(puc_param == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_config_connect:: connect failed, null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_connect_param = (mac_cfg80211_connect_param_stru *)puc_param;

    if ((oal_unlikely(pst_mac_vap == OAL_PTR_NULL)) || (us_len != OAL_SIZEOF(mac_cfg80211_connect_param_stru))) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,
                       "{hmac_config_connect::connect failed,mac_vap is null,or unexpected param len[%x]!}", us_len);
        hmac_free_connect_param_resource(pst_connect_param);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (pst_connect_param->ul_ie_len > WLAN_WPS_IE_MAX_SIZE) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_config_connect:: connect failed, pst_connect_param ie_len[%x] error!}\r\n",
                       pst_connect_param->ul_ie_len);
        hmac_free_connect_param_resource(pst_connect_param);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 如果允许本VAP 入网，则返回设备忙状态 */
    /* 根据内核下发的关联能力，赋值加密相关的mib 值 */
    /* 根据下发的join,提取出安全相关的内容 */
    hmac_connect_set_security(pst_connect_param, &st_conn_sec);

    l_ret = memcpy_s(st_conn_sec.auc_wep_key, OAL_SIZEOF(st_conn_sec.auc_wep_key),
        pst_connect_param->puc_wep_key, st_conn_sec.uc_wep_key_len);

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    st_conn_sec.en_pmf_cap = mac_get_pmf_cap(pst_connect_param->puc_ie, pst_connect_param->ul_ie_len);
#endif
    st_conn_sec.en_wps_enable = OAL_FALSE;
    if (mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS, pst_connect_param->puc_ie,
                           (oal_int32)(pst_connect_param->ul_ie_len))) {
        st_conn_sec.en_wps_enable = OAL_TRUE;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_connect::connect failed, pst_hmac_vap null.vap_id[%d]}", pst_mac_vap->uc_vap_id);
        hmac_free_connect_param_resource(pst_connect_param);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_hmac_vap->en_auth_mode = st_conn_sec.en_auth_type;

#ifdef _PRE_WLAN_FEATURE_11R
    if (pst_hmac_vap->bit_11r_enable && ((st_conn_sec.st_crypto.akm_suites[0] == WLAN_AUTH_SUITE_FT_1X) ||
                                         (st_conn_sec.st_crypto.akm_suites[0] == WLAN_AUTH_SUITE_FT_PSK) ||
                                         (st_conn_sec.st_crypto.akm_suites[0] == WLAN_AUTH_SUITE_FT_SHA256))) {
        pst_hmac_vap->en_auth_mode = WLAN_WITP_AUTH_FT;
    }
#endif
    /* 获取hmac device 结构 */
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_find_scanned_bss_by_bssid::connect failed, hmac_device is null.}");
        hmac_free_connect_param_resource(pst_connect_param);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    pst_bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(pst_mac_vap, pst_connect_param->auc_bssid);
    if (pst_bss_dscr == OAL_PTR_NULL) {
        oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_connect::find the bss failed by bssid:%02X:XX:XX:%02X:%02X:%02X}",
                         pst_connect_param->auc_bssid[0], pst_connect_param->auc_bssid[3], /* 打印输出bssid第0、3byte */
                         pst_connect_param->auc_bssid[4], pst_connect_param->auc_bssid[5]); /* 打印输出bssid第4、5byte */
        hmac_free_connect_param_resource(pst_connect_param);
        return OAL_FAIL;
    }

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    hmac_pmf_update_pmf_black_list(pst_mac_vap, pst_bss_dscr, st_conn_sec.en_mgmt_proteced);
#endif

    if (oal_memcmp(pst_connect_param->auc_ssid, pst_bss_dscr->ac_ssid, (oal_uint32)pst_connect_param->uc_ssid_len)) {
        /* 解锁 */
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_connect::find the bss failed by ssid.}");
        hmac_free_connect_param_resource(pst_connect_param);
        return OAL_FAIL;
    }

    hmac_config_set_ressoc_flag(pst_hmac_vap, pst_connect_param, pst_bss_dscr);

    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING) {
        /* 通知ROAM主状态机, ABORT Roaming FSM */
        hmac_roam_connect_complete(pst_hmac_vap, OAL_FAIL);

        /* After roam_to_old_bss, pst_mac_vap->en_vap_state should be MAC_VAP_STATE_UP,
         * pst_roam_info->en_main_state and pst_roam_info->st_connect.en_state should be 0 */
    }

    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP) {
        if (!oal_memcmp(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11DesiredSSID,
                        pst_connect_param->auc_ssid, pst_connect_param->uc_ssid_len) &&
            (OAL_STRLEN(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11DesiredSSID) ==
             pst_connect_param->uc_ssid_len)) {
            ul_ret = hmac_roam_start_reconnect(pst_hmac_vap, pst_connect_param->auc_bssid);
            /* reassociation failure need delete user first, and then connect again */
            if (ul_ret == OAL_SUCC) {
                hmac_free_connect_param_resource(pst_connect_param);
                return OAL_SUCC;
            }
        }
        /* 先删除用户，再connect */
        pst_hmac_user = mac_res_get_hmac_user((oal_uint16)pst_mac_vap->uc_assoc_vap_id);
        if (pst_hmac_user != OAL_PTR_NULL) {
            hmac_user_del(pst_mac_vap, pst_hmac_user);
        }
    }

#ifdef _PRE_WLAN_FEATURE_WAPI
    pst_bss_dscr->uc_wapi = pst_connect_param->uc_wapi;
    if (pst_bss_dscr->uc_wapi) {
        pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
        if (pst_mac_device == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_connect::connect failed, device[%d] null!",
                           pst_mac_vap->uc_device_id);
            hmac_free_connect_param_resource(pst_connect_param);
            return OAL_ERR_CODE_MAC_DEVICE_NULL;
        }

        if (mac_device_is_p2p_connected(pst_mac_device)) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wapi connect failed for p2p having been connected}");
            hmac_free_connect_param_resource(pst_connect_param);
            return OAL_FAIL;
        }
    }
#endif
    /* 设置P2P/WPS IE 信息到 vap 结构体中 */
    if (IS_LEGACY_VAP(pst_mac_vap)) {
        hmac_config_del_p2p_ie(pst_connect_param->puc_ie, &(pst_connect_param->ul_ie_len));
    }
    st_app_ie.ul_ie_len = pst_connect_param->ul_ie_len;
    l_ret += memcpy_s(st_app_ie.auc_ie, WLAN_WPS_IE_MAX_SIZE, pst_connect_param->puc_ie, st_app_ie.ul_ie_len);
    st_app_ie.en_app_ie_type = OAL_APP_ASSOC_REQ_IE;
    ul_ret = hmac_config_set_app_ie_to_vap(pst_mac_vap, &st_app_ie, st_app_ie.en_app_ie_type);

    hmac_free_connect_param_resource(pst_connect_param);

    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11DTIMPeriod = pst_bss_dscr->uc_dtim_period;

    /* 设置关联用户的能力信息 */
    pst_mac_vap->us_assoc_user_cap_info = pst_bss_dscr->us_cap_info;
    pst_mac_vap->bit_ap_11ntxbf = (pst_bss_dscr->en_11ntxbf == OAL_TRUE) ? 1 : 0;
    pst_mac_vap->bit_sta_11v_info = pst_bss_dscr->en_11v_capable;
    pst_mac_vap->bit_sta_11k_info = pst_bss_dscr->en_11k_capable;

    /* 入网选择的热点rssi，同步到dmac传给tpc算法做管理报文tpc */
    st_conn_sec.c_rssi = pst_bss_dscr->c_rssi;
    st_conn_sec.en_ap_support_triple_nss = pst_bss_dscr->en_support_max_nss >= WLAN_TRIPLE_NSS;

    ul_ret = mac_vap_init_privacy(pst_mac_vap, &st_conn_sec);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_connect:11i_init_privacy failed[%d]}", ul_ret);
        return ul_ret;
    }

    pst_hmac_vap->en_wps_active = st_conn_sec.en_wps_enable;

#ifdef _PRE_WLAN_FEATURE_11R
    if (pst_hmac_vap->bit_11r_enable) {
        puc_mde = mac_find_ie(MAC_EID_MOBILITY_DOMAIN,
                              pst_bss_dscr->auc_mgmt_buff + MAC_80211_FRAME_LEN + MAC_SSID_OFFSET,
                              pst_bss_dscr->ul_mgmt_len - MAC_80211_FRAME_LEN - MAC_SSID_OFFSET);
        if (puc_mde != OAL_PTR_NULL) {
            l_ret += memcpy_s(st_conn_sec.auc_mde, OAL_SIZEOF(st_conn_sec.auc_mde),
                puc_mde, puc_mde[1] + MAC_IE_HDR_LEN);
        }
        ul_ret = mac_mib_init_ft_cfg(pst_mac_vap, st_conn_sec.auc_mde);
        if (ul_ret != OAL_SUCC) {
            oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{init_ft fail[%d]MDE[%p]}", ul_ret, (uintptr_t)puc_mde);
            return ul_ret;
        }
    }

#endif  // _PRE_WLAN_FEATURE_11R
    if (l_ret != EOK) {
        OAM_WARNING_LOG1(0, OAM_SF_WPA, "hmac_config_connect::memcpy fail! l_ret[%d]", l_ret);
    }

#ifdef _PRE_WLAN_FEATURE_SAE
    hmac_update_sae_connect_param(pst_hmac_vap, st_app_ie.auc_ie, st_app_ie.ul_ie_len);
#endif /* _PRE_WLAN_FEATURE_SAE */

    ul_ret = hmac_check_capability_mac_phy_supplicant(pst_mac_vap, pst_bss_dscr);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_connect::check capa fail[%d]}", ul_ret);
    }

    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_CONNECT_REQ, OAL_SIZEOF(st_conn_sec),
                                    (oal_uint8 *)&st_conn_sec);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_WPA, "{hmac_config_connect::send_event failed[%d].}", ul_ret);
        return ul_ret;
    }

    return hmac_sta_initiate_join(pst_mac_vap, pst_bss_dscr);
}

#ifdef _PRE_WLAN_FEATURE_11D

oal_uint32 hmac_config_set_rd_by_ie_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_bool_enum_uint8 *pc_param;
    hmac_vap_stru *pst_hmac_vap;

    pc_param = (oal_bool_enum_uint8 *)puc_param;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(0);
    if (oal_unlikely(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_BA, "{hmac_config_set_rd_by_ie_switch::pst_mac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_hmac_vap->en_updata_rd_by_ie_switch = *pc_param;

    return OAL_SUCC;
}
#endif

oal_uint32 hmac_config_get_tid(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    mac_device_stru *pst_mac_dev;
    mac_cfg_get_tid_stru *pst_tid;

    pst_tid = (mac_cfg_get_tid_stru *)puc_param;
    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_tid->en_tid = pst_mac_dev->en_tid;
    *pus_len = OAL_SIZEOF(pst_tid->en_tid);

    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_tid::en_tid=%d.}", pst_tid->en_tid);
    return OAL_SUCC;
}


oal_uint32 hmac_config_list_channel(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint8 uc_ch_num;
    oal_uint8 uc_chan_idx;
    oal_uint32 ul_ret = OAL_FAIL;

    if ((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_list_channel::null param,pst_mac_vap=%x puc_param=%x.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)puc_param);
        return OAL_FAIL;
    }

    for (uc_chan_idx = 0; uc_chan_idx < MAC_CHANNEL_FREQ_2_BUTT; uc_chan_idx++) {
        ul_ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_2, uc_chan_idx);
        if (ul_ret == OAL_SUCC) {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_2, uc_chan_idx, &uc_ch_num);

            /* 输出2G信道号 */
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_list_channel::2gCHA.NO=%d}", uc_ch_num);
        }
    }
    if (!mac_get_band_5g_enabled()) {
        return OAL_SUCC;
    }
    for (uc_chan_idx = 0; uc_chan_idx < MAC_CHANNEL_FREQ_5_BUTT / 2; uc_chan_idx++) { /* 除以2表示从0-14信道索引值查找 */
        ul_ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, uc_chan_idx);
        if (ul_ret == OAL_SUCC) {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_5, uc_chan_idx, &uc_ch_num);

            /* 检测5G 36~120信道上的DFS雷达标记 */
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_list_channel::5gCHA.NO=%d,DFS_REQUIRED[%c]}\n",
                             uc_ch_num,
                             ((mac_is_ch_in_radar_band(MAC_RC_START_FREQ_5, uc_chan_idx) == OAL_TRUE) ? 'Y' : 'N'));
        }
    }
    /* 除以2表示从14-29信道索引值查找 */
    for (uc_chan_idx = MAC_CHANNEL_FREQ_5_BUTT / 2; uc_chan_idx < MAC_CHANNEL_FREQ_5_BUTT; uc_chan_idx++) {
        ul_ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, uc_chan_idx);
        if (ul_ret == OAL_SUCC) {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_5, uc_chan_idx, &uc_ch_num);

            /* 检测5G 124~196信道上的DFS雷达标记 */
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_list_channel::5gCHA.NO=%d,DFS_REQUIRED[%c]}\n",
                             uc_ch_num,
                             ((mac_is_ch_in_radar_band(MAC_RC_START_FREQ_5, uc_chan_idx) == OAL_TRUE) ? 'Y' : 'N'));
        }
    }
    return OAL_SUCC;
}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32 hmac_config_h2d_send_app_ie(mac_vap_stru *pst_mac_vap, oal_app_ie_stru *pst_app_ie)
{
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    oal_netbuf_stru *pst_netbuf_app_ie = OAL_PTR_NULL;
    oal_uint16 us_frame_len;
    dmac_tx_event_stru *pst_app_ie_event = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_uint8 *puc_param = OAL_PTR_NULL;
    oal_uint8 uc_app_ie_header_len;

    if ((pst_mac_vap == OAL_PTR_NULL) || (pst_app_ie == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG, "{param is NULL,%p, %p.}", (uintptr_t)pst_mac_vap, (uintptr_t)pst_app_ie);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 帧长校验 */
    uc_app_ie_header_len = OAL_SIZEOF(oal_app_ie_stru) - OAL_SIZEOF(pst_app_ie->auc_ie) /
                           OAL_SIZEOF(pst_app_ie->auc_ie[0]);

    us_frame_len = uc_app_ie_header_len + pst_app_ie->ul_ie_len;
    if ((us_frame_len >= WLAN_LARGE_NETBUF_SIZE) || (pst_app_ie->ul_ie_len > WLAN_WPS_IE_MAX_SIZE)) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{frame_len[%d]ie_len=[%d]invalid.}",
                         us_frame_len, pst_app_ie->ul_ie_len);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请netbuf内存 */
    pst_netbuf_app_ie = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, us_frame_len, OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf_app_ie == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_h2d_send_app_ie::alloc netbuf failed.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 申请event 事件内存 */
    pst_event_mem = frw_event_alloc_m(OAL_SIZEOF(dmac_tx_event_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        oal_netbuf_free(pst_netbuf_app_ie);
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_h2d_send_app_ie::event_mem alloc failed.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_APP_IE_H2D,
                       OAL_SIZEOF(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    memset_s(oal_netbuf_cb(pst_netbuf_app_ie), OAL_TX_CB_LEN, 0, OAL_TX_CB_LEN);

    puc_param = (oal_uint8 *)(oal_netbuf_data(pst_netbuf_app_ie));
    if (memcpy_s(puc_param, us_frame_len, (oal_uint8 *)pst_app_ie, us_frame_len) != EOK) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_h2d_send_app_ie::memcpy fail!}");
        oal_netbuf_free(pst_netbuf_app_ie);
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }
    pst_app_ie_event = (dmac_tx_event_stru *)pst_event->auc_event_data;
    pst_app_ie_event->pst_netbuf = pst_netbuf_app_ie;
    pst_app_ie_event->us_frame_len = us_frame_len;
    pst_app_ie_event->us_remain = 0;

    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_h2d_send_app_ie:: dispatch failed, app_ie_type=[%d],app_ie_len=[%d].}",
                         pst_app_ie->en_app_ie_type, pst_app_ie->ul_ie_len);
        oal_netbuf_free(pst_netbuf_app_ie);
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    oal_netbuf_free(pst_netbuf_app_ie);
    frw_event_free_m(pst_event_mem);
    return OAL_SUCC;
}
#endif


oal_uint32 hmac_config_set_app_ie_to_vap(mac_vap_stru *pst_mac_vap,
                                         oal_app_ie_stru *pst_app_ie,
                                         en_app_ie_type_uint8 en_type)
{
    oal_uint32 ul_ret;
    oal_uint32 ul_ret_send_app_ie;
    oal_uint8 *puc_ie = OAL_PTR_NULL;
    oal_uint32 remain_len;
    oal_int32 l_ret = EOK;

    if ((pst_mac_vap == OAL_PTR_NULL) || (pst_app_ie == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_set_app_ie_to_vap::scan failed, set ie null ptr, %p, %p.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)pst_app_ie);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 移除驱动侧重复MAC_EID_EXT_CAPS */
    puc_ie = mac_find_ie(MAC_EID_EXT_CAPS, pst_app_ie->auc_ie, (oal_int32)pst_app_ie->ul_ie_len);
    if (puc_ie != OAL_PTR_NULL) {
        pst_app_ie->ul_ie_len -= (oal_uint32)(puc_ie[1] + MAC_IE_HDR_LEN);
        remain_len = pst_app_ie->ul_ie_len - (oal_uint32)(puc_ie - pst_app_ie->auc_ie);
        if (remain_len != 0) {
            l_ret += memmove_s (puc_ie, remain_len, puc_ie + (oal_uint32)(puc_ie[1] + MAC_IE_HDR_LEN), remain_len);
        }
    }

    puc_ie = mac_find_ie(MAC_EID_OPERATING_CLASS, pst_app_ie->auc_ie, (oal_int32)pst_app_ie->ul_ie_len);
    if ((puc_ie != OAL_PTR_NULL) &&
        (!pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11ExtendedChannelSwitchActivated)) {
        pst_app_ie->ul_ie_len -= (oal_uint32)(puc_ie[1] + MAC_IE_HDR_LEN);
        remain_len = pst_app_ie->ul_ie_len - (oal_uint32)(puc_ie - pst_app_ie->auc_ie);
        if (remain_len != 0) {
            l_ret += memmove_s (puc_ie, remain_len, puc_ie + (oal_uint32)(puc_ie[1] + MAC_IE_HDR_LEN), remain_len);
        }
    }

    if (l_ret != EOK) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "hmac_config_set_app_ie_to_vap::memmove fail![%d]", l_ret);
    }

    ul_ret = mac_vap_save_app_ie(pst_mac_vap, pst_app_ie, en_type);
    if (ul_ret != OAL_SUCC) {
        oam_error_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_set_app_ie_to_vap::save_app_ie failed[%d], en_type[%d],len[%d].}", ul_ret, en_type,
                       pst_app_ie->ul_ie_len);
        return ul_ret;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (pst_app_ie->en_app_ie_type >= OAL_APP_ASSOC_REQ_IE) {
        /* 只有OAL_APP_BEACON_IE、OAL_APP_PROBE_REQ_IE、OAL_APP_PROBE_RSP_IE 才需要保存到device */
        return OAL_SUCC;
    }
    ul_ret_send_app_ie = hmac_config_h2d_send_app_ie(pst_mac_vap, pst_app_ie);
#endif

    return ul_ret;
}


oal_uint32 hmac_config_set_wps_p2p_ie(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_w2h_app_ie_stru *pst_w2h_wps_p2p_ie;
    oal_app_ie_stru st_app_ie;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    pst_w2h_wps_p2p_ie = (oal_w2h_app_ie_stru *)puc_param;

    if ((pst_w2h_wps_p2p_ie->en_app_ie_type >= OAL_APP_IE_NUM) ||
        (pst_w2h_wps_p2p_ie->ul_ie_len >= WLAN_WPS_IE_MAX_SIZE)) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_wps_p2p_ie::app_ie_type=[%d] app_ie_len=[%d],param invalid.}",
                         pst_w2h_wps_p2p_ie->en_app_ie_type, pst_w2h_wps_p2p_ie->ul_ie_len);
        return OAL_FAIL;
    }

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_wps_p2p_ie::p2p_ie_type=[%d], p2p_ie_len=[%d].}",
                     pst_w2h_wps_p2p_ie->en_app_ie_type, pst_w2h_wps_p2p_ie->ul_ie_len);

    memset_s(&st_app_ie, OAL_SIZEOF(st_app_ie), 0, OAL_SIZEOF(st_app_ie));
    st_app_ie.en_app_ie_type = pst_w2h_wps_p2p_ie->en_app_ie_type;
    st_app_ie.ul_ie_len = pst_w2h_wps_p2p_ie->ul_ie_len;
    if (memcpy_s(st_app_ie.auc_ie, WLAN_WPS_IE_MAX_SIZE, pst_w2h_wps_p2p_ie->puc_data_ie,
                 st_app_ie.ul_ie_len) != EOK) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_p2p_ie::memcpy fail!}");
        return OAL_FAIL;
    }

    /* 设置WPS/P2P 信息 */
    ul_ret = hmac_config_set_app_ie_to_vap(pst_mac_vap, &st_app_ie, st_app_ie.en_app_ie_type);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_p2p_ie::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 检测beacon 信息中是否有WPS 信息元素 */
    if (st_app_ie.en_app_ie_type == OAL_APP_BEACON_IE) {
        if ((st_app_ie.ul_ie_len != 0)
            /* && OAL_PTR_NULL != mac_get_wps_ie(pst_wps_p2p_ie->auc_ie, (oal_uint16)pst_wps_p2p_ie->ul_ie_len, 0)) */
            &&
            (mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS, st_app_ie.auc_ie,
                                (oal_int32)(st_app_ie.ul_ie_len)) != OAL_PTR_NULL)) {
            /* 设置WPS 功能使能 */
            pst_hmac_vap->en_wps_active = OAL_TRUE;
            oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_p2p_ie::set wps enable.}");
        } else {
            pst_hmac_vap->en_wps_active = OAL_FALSE;
        }
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_wps_ie(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_app_ie_stru *pst_wps_ie;
    oal_uint8 *puc_ie = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    pst_wps_ie = (oal_app_ie_stru *)puc_param;

    /* 设置WPS 信息 */
    ul_ret = hmac_config_set_app_ie_to_vap(pst_mac_vap, pst_wps_ie, pst_wps_ie->en_app_ie_type);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_wps_ie::ul_ret=[%d].}",
                         ul_ret);
        return ul_ret;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_ie::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 检测beacon 信息中是否有WPS 信息元素 */
    if ((pst_wps_ie->en_app_ie_type == OAL_APP_BEACON_IE) && (pst_wps_ie->ul_ie_len != 0)) {
        puc_ie = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS, pst_wps_ie->auc_ie,
                                    (oal_int32)(pst_wps_ie->ul_ie_len));
        if (puc_ie != OAL_PTR_NULL) {
            /* 设置WPS 功能使能 */
            pst_hmac_vap->en_wps_active = OAL_TRUE;
            oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_ie::set wps enable.}");
        }
    } else if ((pst_wps_ie->ul_ie_len == 0) &&
               (pst_wps_ie->en_app_ie_type == OAL_APP_BEACON_IE)) {
        pst_hmac_vap->en_wps_active = OAL_FALSE;
        oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_ie::set wps disable.}");
    }

    return ul_ret;
}


oal_uint32 hmac_config_pause_tid(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PAUSE_TID, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_pause_tid::send_event failed[%d].}",
                         ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_reg_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_REG_INFO, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_reg_info::send_event failed[%d].}",
                         ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_dbb_scaling_amend(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DBB_SCALING_AMEND, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_dbb_scaling_amend::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))


oal_uint32 hmac_config_sdio_flowctrl(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SDIO_FLOWCTRL, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_sdio_flowctrl::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}
#endif


oal_uint32 hmac_config_reg_write(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_REG_WRITE, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_reg_write::send_event failed[%d].}",
                         ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_alg_param(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    /***************************************************************************
        抛事件到ALG层, 同步ALG数据
    ***************************************************************************/
#ifdef _PRE_WLAN_FEATURE_TXBF
    /* 同步更新txbf的mib值 */
    mac_ioctl_alg_param_stru *pst_alg_param;
    wlan_mib_ieee802dot11_stru *pst_mib_info = pst_mac_vap->pst_mib_info;
    pst_alg_param = (mac_ioctl_alg_param_stru *)puc_param;

    if (pst_alg_param->en_alg_cfg == MAC_ALG_CFG_TXBF_TXBFEE_ENABLE) {
        pst_mib_info->st_wlan_mib_txbf_config.en_dot11ReceiveStaggerSoundingOptionImplemented =
            pst_alg_param->ul_value;
        pst_mib_info->st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformeeOptionImplemented =
            pst_alg_param->ul_value;
        pst_mac_vap->en_host_txbf_mode = pst_alg_param->ul_value;
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
        pst_mib_info->st_wlan_mib_vht_txbf_config.en_dot11VHTMUBeamformeeOptionImplemented =
            pst_alg_param->ul_value;
#endif
        pst_mib_info->st_wlan_mib_vht_txbf_config.ul_dot11VHTBeamformeeNTxSupport = pst_alg_param->ul_value;
        if (pst_mib_info->st_wlan_mib_txbf_config.en_dot11ReceiveStaggerSoundingOptionImplemented ==
            OAL_TRUE) {
            pst_mib_info->st_wlan_mib_txbf_config.uc_dot11ExplicitCompressedBeamformingFeedbackOptionImplemented =
                1;
            pst_mib_info->st_wlan_mib_txbf_config.ul_dot11NumberCompressedBeamformingMatrixSupportAntenna = 1;
        } else {
            pst_mib_info->st_wlan_mib_txbf_config.uc_dot11ExplicitCompressedBeamformingFeedbackOptionImplemented =
                0;
            pst_mib_info->st_wlan_mib_txbf_config.ul_dot11NumberCompressedBeamformingMatrixSupportAntenna = 0;
        }
    } else if (pst_alg_param->en_alg_cfg == MAC_ALG_CFG_TXBF_RXSTBC_ENABLE) {
        pst_mib_info->st_phy_ht.en_dot11RxSTBCOptionImplemented = pst_alg_param->ul_value;
        pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTRxSTBCOptionImplemented = pst_alg_param->ul_value;
    } else if (pst_alg_param->en_alg_cfg == MAC_ALG_CFG_TXBF_11N_BFEE_ENABLE) {
        pst_mac_vap->st_cap_flag.bit_11ntxbf = pst_alg_param->ul_value;
    }

#endif
    return hmac_config_alg_send_event(pst_mac_vap, WLAN_CFGID_ALG_PARAM, us_len, puc_param);
}


oal_uint32 hmac_config_2040_channel_switch_prohibited(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                      oal_uint8 *puc_param)
{
    mac_device_stru *pst_device;
    oal_uint8 uc_vap_idx;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_2040_channel_switch_prohibited::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_device->uc_vap_num == 0) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_2040_channel_switch_prohibited::vap_num=0.}");
        return OAL_FAIL;
    }

    /* 遍历device下所有vap，设置20/40信道切换禁止参数 */
    for (uc_vap_idx = 0; uc_vap_idx < pst_device->uc_vap_num; uc_vap_idx++) {
        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_device->auc_vap_id[uc_vap_idx]);
        if (pst_hmac_vap == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_2040_channel_switch_prohibited:vap null}");
            continue;
        }

        pst_hmac_vap->en_2040_switch_prohibited = *((oal_uint8 *)puc_param);
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_fortymhzintolerant(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    if ((*puc_param != 0) && (*puc_param != 1)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_fortymhzintolerant::invalid param[%d].",
                         *puc_param);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    mac_mib_set_FortyMHzIntolerant (pst_mac_vap, (oal_bool_enum_uint8)(*puc_param));

    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_fortymhzintolerant::end func,puc_param=%d.}",
                  *puc_param);
    return OAL_SUCC;
}


oal_uint32 hmac_config_set_2040_coext_support(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    if ((*puc_param != 0) && (*puc_param != 1)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_2040_coext_support::invalid param[%d].",
                         *puc_param);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 如果是配置VAP, 直接返回 */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_2040_coext_support::this is config vap! can't set.}");
        return OAL_FAIL;
    }

    mac_mib_set_2040BSSCoexistenceManagementSupport (pst_mac_vap, (oal_bool_enum_uint8)(*puc_param));

    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_2040_coext_support::end func,puc_param=%d.}",
                  *puc_param);
    return OAL_SUCC;
}


oal_uint32 hmac_config_rx_fcs_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_RX_FCS_INFO, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_rx_fcs_info::send_event fail[%d]}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_dscr_th_opt(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_dscr_th_stru *pst_dscr_th = OAL_PTR_NULL;

    if (oal_unlikely(puc_param == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_config_dscr_th_opt::puc_param is NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dscr_th = (mac_cfg_dscr_th_stru *)puc_param;

    /* 0: 大包队列 1: 小包队列, wal已验证输入 */
    if (pst_dscr_th->ul_queue_id == 0) {
        g_ul_large_start_th = pst_dscr_th->ul_start_th;
        g_ul_large_interval = pst_dscr_th->ul_interval;
    } else {
        g_ul_small_start_th = pst_dscr_th->ul_start_th;
        g_ul_small_interval = pst_dscr_th->ul_interval;
    }

    hmac_rx_dscr_th_init(OAL_PTR_NULL);

    return OAL_SUCC;
}


oal_uint32 hmac_config_tcp_ack_filter(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_tcp_ack_filter *pst_tcp_ack_filter = OAL_PTR_NULL;

    if (oal_unlikely(puc_param == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_config_tcp_ack_filter::puc_param is NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_tcp_ack_filter = (mac_cfg_tcp_ack_filter *)puc_param;

    /* 0: 功能关 1: 功能关, wal已验证输入 */
    g_en_tcp_ack_filter_switch = pst_tcp_ack_filter->ul_switch;

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP

oal_uint32 hmac_config_set_edca_opt_switch_sta(mac_vap_stru *pst_mac_vap,
                                               oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint8 uc_flag;
    oal_uint32 ul_ret = 0;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;

    /* 获取hmac_vap */
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "hmac_config_set_edca_opt_switch_sta, mac_res_get_hmac_vap fail.vap_id = %u",
                         pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取配置参数 */
    uc_flag = *puc_param;

    /* 参数没有更改，不需要重新配置 */
    if (uc_flag == pst_hmac_vap->uc_edca_opt_flag_sta) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "hmac_config_set_edca_opt_switch_sta, change nothing to flag:%d",
                         pst_hmac_vap->uc_edca_opt_flag_sta);
        return OAL_SUCC;
    }

    /* 设置参数，并启动或者停止edca参数调整定时器 */
    pst_hmac_vap->uc_edca_opt_flag_sta = uc_flag;

    if (pst_hmac_vap->uc_edca_opt_flag_sta == 0) {
        ul_ret = mac_vap_init_wme_param(pst_mac_vap);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "hmac_config_set_edca_opt_switch_sta: mac_vap_init_wme_param failed");
            return ul_ret;
        }

        oam_warning_log0(0, OAM_SF_ANY, "mac_vap_init_wme_param succ");
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "hmac_edca_opt_adj_param_sta succ");
    }

    /* 更新EDCA相关的MAC寄存器 */
    ul_ret = hmac_sta_up_update_edca_params_machw(pst_hmac_vap, MAC_WMM_SET_PARAM_TYPE_UPDATE_EDCA);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY,
                         "hmac_config_set_edca_opt_switch_sta: hmac_sta_up_update_edca_params_machw failed");
        return ul_ret;
    }

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "hmac_config_set_edca_opt_switch_sta,config sucess, %d",
                     pst_hmac_vap->uc_edca_opt_flag_sta);

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_edca_opt_weight_sta(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint8 uc_weight;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;

    /* 获取hmac_vap */
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "hmac_config_set_edca_opt_weight_sta, mac_res_get_hmac_vap fail.vap_id = %u",
                         pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_weight = *puc_param;

    /* 判断edca权重是否有调整 */
    if (uc_weight == pst_hmac_vap->uc_edca_opt_weight_sta) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "hmac_config_set_edca_opt_weight_sta, change nothing to cycle:%d",
                         pst_hmac_vap->uc_edca_opt_weight_sta);
        return OAL_SUCC;
    }

    /* 更新权重 */
    pst_hmac_vap->uc_edca_opt_weight_sta = uc_weight;
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "hmac_config_set_edca_opt_weight_sta succ, wieight = %d",
                     pst_hmac_vap->uc_edca_opt_weight_sta);

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_edca_opt_switch_ap(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint8 uc_flag;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;

    /* 获取hmac_vap */
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "hmac_config_set_edca_opt_switch_ap, mac_res_get_hmac_vap fail.vap_id = %u",
                         pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取配置参数 */
    uc_flag = *puc_param;

    /* 参数没有更改，不需要重新配置 */
    if (uc_flag == pst_hmac_vap->uc_edca_opt_flag_ap) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_hipriv_set_edca_opt_switch_ap, change nothing to flag:%d",
                         pst_hmac_vap->uc_edca_opt_flag_ap);
        return OAL_SUCC;
    }

    /* 设置参数，并启动或者停止edca参数调整定时器 */
    if (uc_flag == 1) {
        pst_hmac_vap->uc_edca_opt_flag_ap = 1;
        frw_timer_restart_timer(&(pst_hmac_vap->st_edca_opt_timer), pst_hmac_vap->ul_edca_opt_time_ms, OAL_TRUE);
    } else {
        pst_hmac_vap->uc_edca_opt_flag_ap = 0;
        frw_timer_stop_timer(&(pst_hmac_vap->st_edca_opt_timer));
    }

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "hmac_config_set_edca_opt_switch_ap succ, flag = %d",
                     pst_hmac_vap->uc_edca_opt_flag_ap);

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_edca_opt_cycle_ap(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_cycle_ms;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;

    /* 获取hmac_vap */
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "hmac_config_set_edca_opt_cycle_ap, mac_res_get_hmac_vap fail.vap_id = %u",
                         pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_cycle_ms = *((oal_uint32 *)puc_param);

    /* 判断edca调整周期是否有更新 */
    if (ul_cycle_ms == pst_hmac_vap->ul_edca_opt_time_ms) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "hmac_config_set_edca_opt_cycle_ap, change nothing to cycle:%d",
                         pst_hmac_vap->ul_edca_opt_time_ms);
        return OAL_SUCC;
    }

    /* 如果edca调整定时器正在运行，则需要先停止后，再根据新的参数restart */
    if (pst_hmac_vap->uc_edca_opt_flag_ap == 1) {
        pst_hmac_vap->ul_edca_opt_time_ms = ul_cycle_ms;
        frw_timer_stop_timer(&(pst_hmac_vap->st_edca_opt_timer));
        frw_timer_restart_timer(&(pst_hmac_vap->st_edca_opt_timer), pst_hmac_vap->ul_edca_opt_time_ms, OAL_TRUE);
    } else { /* 仅更新参数即可 */
        pst_hmac_vap->ul_edca_opt_time_ms = ul_cycle_ms;
    }

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "hmac_config_set_edca_opt_cycle_ap succ, cycle = %d",
                     pst_hmac_vap->ul_edca_opt_time_ms);

    return OAL_SUCC;
}

#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM

oal_uint32 hmac_config_set_pm_by_module(mac_vap_stru *pst_mac_vap, mac_pm_ctrl_type_enum pm_ctrl_type,
                                        mac_pm_switch_enum pm_enable)
{
    oal_uint32 ul_ret;
    mac_cfg_ps_open_stru st_ps_open = { 0 };

    if ((pm_enable >= MAC_STA_PM_SWITCH_BUTT) ||
        (pm_ctrl_type >= MAC_STA_PM_CTRL_TYPE_BUTT) ||
        (pst_mac_vap == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_ANY,
                       "hmac_config_set_pm_by_module, PARAM ERROR!mac_vap = 0x%X, pm_ctrl_type = %d, pm_enable = %d ",
                       (uintptr_t)pst_mac_vap, pm_ctrl_type, pm_enable);
        return OAL_FAIL;
    }

    st_ps_open.uc_pm_enable = pm_enable;
    st_ps_open.uc_pm_ctrl_type = pm_ctrl_type;

#ifdef _PRE_WLAN_FEATURE_STA_PM
    ul_ret = hmac_config_set_sta_pm_on(pst_mac_vap, OAL_SIZEOF(mac_cfg_ps_open_stru), (oal_uint8 *)&st_ps_open);
#endif

    oam_warning_log3(0, OAM_SF_PWR, "hmac_config_set_pm_by_module, pm_module = %d, pm_enable = %d, cfg ret = %d ",
                     pm_ctrl_type, pm_enable, ul_ret);

    return ul_ret;
}
#endif


oal_uint32 hmac_config_alg(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    mac_user_stru *pst_user = OAL_PTR_NULL;
    oal_int8 *pac_argv[DMAC_ALG_CONFIG_MAX_ARG + 1] = { 0 };
    mac_ioctl_alg_config_stru *pst_alg_config;
    oal_uint8 uc_idx;
    oal_uint32 ul_bw_limit_kbps;
    oal_dlist_head_stru *pst_list_pos = OAL_PTR_NULL;

    pst_alg_config = (mac_ioctl_alg_config_stru *)puc_param;

    for (uc_idx = OAL_SIZEOF(mac_ioctl_alg_config_stru); uc_idx < us_len; uc_idx++) {
        if (puc_param[uc_idx] == ' ') {
            puc_param[uc_idx] = 0;
        }
    }

    for (uc_idx = 0; uc_idx < pst_alg_config->uc_argc; uc_idx++) {
        pac_argv[uc_idx] = (oal_int8 *)puc_param + OAL_SIZEOF(mac_ioctl_alg_config_stru) +
                           pst_alg_config->auc_argv_offset[uc_idx];
    }

    /* 如果为用户限速，则需要同步hmac_vap的状态信息 */
    if ((oal_strcmp(pac_argv[0], "sch") == 0)
        && (oal_strcmp(pac_argv[1], "usr_bw") == 0)) {
        /* 传入pac_argv[2]的地址，返回mac_user指针 */
        pst_user = mac_vap_get_user_by_addr(pst_mac_vap, (oal_uint8 *)(pac_argv[2]));
        if (pst_user == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(0, OAM_SF_MULTI_TRAFFIC, "{alg_schedule_config_user_bw_limit: find_user_by_macaddr failed}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_bw_limit_kbps = (oal_uint32)oal_atoi(pac_argv[3]); /* 将pac_argv[3]字符串转换为整型值 */

        pst_mac_vap->bit_has_user_bw_limit = OAL_FALSE;
        for ((pst_list_pos) = (pst_mac_vap)->st_mac_user_list_head.pst_next,
             (pst_user) = oal_dlist_get_entry((pst_list_pos), mac_user_stru, st_user_dlist);
             (pst_list_pos) != &((pst_mac_vap)->st_mac_user_list_head);
             (pst_list_pos) = (pst_list_pos)->pst_next, (pst_user) = oal_dlist_get_entry((pst_list_pos), mac_user_stru,
                 st_user_dlist)) {
            /* 如果限速值不为0,表示该user已被限速，则更新vap的状态 */
            if ((pst_user != OAL_PTR_NULL) && (ul_bw_limit_kbps != 0)) {
                pst_mac_vap->bit_has_user_bw_limit = OAL_TRUE;
                break;
            }
        }
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_ALG, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_alg::hmac_config_send_event failed[%d].}",
                         ul_ret);
    }

    return ul_ret;
}

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
oal_uint32 hmac_config_tcp_ack_buf(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    return hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TCP_ACK_BUF, us_len, puc_param);
}
#endif

oal_uint32 hmac_config_set_ps_params(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    return hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_PS_PARAMS, us_len, puc_param);
}

#ifdef _PRE_WLAN_FEATURE_BTCOEX

oal_uint32 hmac_config_set_btcoex_params(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_btcoex_cfg_stru *pst_btcoex_params;

    pst_btcoex_params = (mac_btcoex_cfg_stru *)puc_param;

    if ((pst_btcoex_params->en_cfg_type == MAC_BTCOEX_CFG_BA_SIZE) && (pst_btcoex_params->uc_cfg_value == 0)) {
        g_en_btcoex_reject_addba = OAL_TRUE;
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_set_btcoex_params:reject addba");
        return OAL_SUCC;
    }
    g_en_btcoex_reject_addba = OAL_FALSE;
    return hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_BTCOEX_PARAMS, us_len, puc_param);
}
#endif

oal_uint32 hmac_config_bindcpu(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)
#if defined(CONFIG_ARCH_HISI)
    mac_cfg_set_bindcpu_stru *pst_bindcpu;
    struct cpumask cpus_mask;
    oal_uint8 uc_cpumask;
    oal_uint8 uc_cpuid = 0;

    if (puc_param == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "hmac_config_bindcpu:puc_param=%x", (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_bindcpu = (mac_cfg_set_bindcpu_stru *)(puc_param);
    uc_cpumask = (pst_bindcpu->uc_thread_mask);

    oam_warning_log2(0, OAM_SF_CFG,
                     "{hmac_config_bindcpu::bindcpu thread[%d](0:hisi_hcc;1:hmac_rxdata;2:rx_tsk), cpumask[%x]}",
                     pst_bindcpu->uc_thread_id, uc_cpumask);

    cpumask_clear(&cpus_mask);
    while (uc_cpumask) {
        if (uc_cpumask & 0x1) {
            if (uc_cpuid >= 8) { /* cpuid大于等于8，直接break跳出 */
                break;
            }
            cpumask_set_cpu(uc_cpuid, &cpus_mask);
        }
        uc_cpuid++;
        uc_cpumask = uc_cpumask >> 1;
    }

    switch (pst_bindcpu->uc_thread_id) {
        case 0:
            set_cpus_allowed_ptr(hcc_get_110x_handler()->hcc_transer_info.hcc_transfer_thread, &cpus_mask);
            break;
        case 1:
            set_cpus_allowed_ptr(g_st_rxdata_thread.pst_rxdata_thread, &cpus_mask);
            break;
        case 2:
            set_cpus_allowed_ptr(hcc_get_110x_handler()->bus_dev->cur_bus->pst_rx_tsk, &cpus_mask);
            break;
        default:
            break;
    }
#endif
#endif
    return OAL_SUCC;
}

oal_uint32 hmac_config_napi_weight(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_set_napi_weight_stru *pst_napiweight = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    oal_uint8 uc_vap_idx;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_netdev_priv_stru *pst_netdev_priv = OAL_PTR_NULL;

    if (puc_param == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "hmac_config_napi_weight:puc_param=%x", (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(0);
    pst_napiweight = (mac_cfg_set_napi_weight_stru *)(puc_param);
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_hmac_vap == OAL_PTR_NULL) {
            continue;
        }

        if (pst_hmac_vap->pst_net_device == OAL_PTR_NULL) {
            continue;
        }

        pst_netdev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(pst_hmac_vap->pst_net_device);

        pst_netdev_priv->uc_napi_dyn_weight = pst_napiweight->en_napi_weight_adjust;
        pst_netdev_priv->uc_napi_weight = pst_napiweight->uc_napi_weight;
        pst_netdev_priv->st_napi.weight = pst_napiweight->uc_napi_weight;
    }
    return OAL_SUCC;
}

oal_uint32 hmac_config_set_rts_param(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_RTS_PARAM, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_rts_param::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_update_protection_tx_param(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                  oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_UPDTAE_PROT_TX_PARAM, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_update_protection_tx_param::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_protection(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    mac_dump_protection(pst_mac_vap, puc_param);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_PROTECTION, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_protection::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_auto_protection(mac_vap_stru *pst_mac_vap, oal_uint8 uc_auto_protection_flag)
{
    return hmac_protection_set_autoprot(pst_mac_vap, uc_auto_protection_flag);
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)


oal_uint32 hmac_config_vap_state_syn(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_VAP_STATE_SYN, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_vap_state_syn::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}

#ifdef _PRE_WLAN_FEATURE_STA_PM


oal_uint32 hmac_set_ipaddr_timeout(void *puc_para)
{
    oal_uint32 ul_ret;
    hmac_vap_stru *pst_hmac_vap = (hmac_vap_stru *)puc_para;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (hmac_config_get_ps_mode() == MAX_FAST_PS) {
        wlan_pm_set_timeout(wlan_pm_get_fast_check_cnt());
    } else {
        wlan_pm_set_timeout(WLAN_SLEEP_DEFAULT_CHECK_CNT);
    }
#endif

    /* 未主动dhcp成功,超时开低功耗 */
    ul_ret = hmac_config_set_pm_by_module(&pst_hmac_vap->st_vap_base_info, MAC_STA_PM_CTRL_TYPE_HOST,
                                          MAC_STA_PM_SWITCH_ON);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                         "{hmac_set_ipaddr_timeout::hmac_config_set_pm_by_module failed[%d].}", ul_ret);
    }
    return OAL_SUCC;
}

#endif
#endif


oal_uint32 hmac_config_user_asoc_state_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    oal_uint32 ul_ret;
    mac_h2d_user_asoc_state_stru st_h2d_user_asoc_state_stru;

    st_h2d_user_asoc_state_stru.us_user_idx = pst_mac_user->us_assoc_id;
    st_h2d_user_asoc_state_stru.en_asoc_state = pst_mac_user->en_user_asoc_state;

    /***************************************************************************
        抛事件到DMAC层, 同步user关联状态到device侧
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_USER_ASOC_STATE_SYN,
                                    OAL_SIZEOF(mac_h2d_user_asoc_state_stru),
                                    (oal_uint8 *)(&st_h2d_user_asoc_state_stru));
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_user_asoc_state_syn::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_user_cap_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    oal_uint32 ul_ret;
    mac_h2d_usr_cap_stru st_mac_h2d_usr_cap;

    st_mac_h2d_usr_cap.us_user_idx = pst_mac_user->us_assoc_id;
    memcpy_s((oal_uint8 *)(&st_mac_h2d_usr_cap.st_user_cap_info), OAL_SIZEOF(mac_user_cap_info_stru),
             (oal_uint8 *)(&pst_mac_user->st_cap_info), OAL_SIZEOF(mac_user_cap_info_stru));

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_USER_CAP_SYN, OAL_SIZEOF(mac_h2d_usr_cap_stru),
                                    (oal_uint8 *)(&st_mac_h2d_usr_cap));
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_user_cap_syn::hmac_config_sta_vap_info_syn failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_user_rate_info_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    oal_uint32 ul_ret;
    mac_h2d_usr_rate_info_stru st_mac_h2d_usr_rate_info;

    st_mac_h2d_usr_rate_info.us_user_idx = pst_mac_user->us_assoc_id;
    st_mac_h2d_usr_rate_info.en_protocol_mode = pst_mac_user->en_protocol_mode;

    /* legacy速率集信息，同步到dmac */
    st_mac_h2d_usr_rate_info.uc_avail_rs_nrates = pst_mac_user->st_avail_op_rates.uc_rs_nrates;
    memcpy_s(st_mac_h2d_usr_rate_info.auc_avail_rs_rates, WLAN_RATE_MAXSIZE,
             pst_mac_user->st_avail_op_rates.auc_rs_rates, WLAN_RATE_MAXSIZE);

    /* ht速率集信息，同步到dmac */
    mac_user_get_ht_hdl(pst_mac_user, &st_mac_h2d_usr_rate_info.st_ht_hdl);

    /* vht速率集信息，同步到dmac */
    mac_user_get_vht_hdl(pst_mac_user, &st_mac_h2d_usr_rate_info.st_vht_hdl);

    /***************************************************************************
        抛事件到DMAC层, 同步user关联状态到device侧
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_USER_RATE_SYN, sizeof(mac_h2d_usr_rate_info_stru),
                                    (oal_uint8 *)(&st_mac_h2d_usr_rate_info));
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_user_rate_info_syn::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_user_info_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    oal_uint32 ul_ret;
    mac_h2d_usr_info_stru st_mac_h2d_usr_info;

    st_mac_h2d_usr_info.en_avail_bandwidth = pst_mac_user->en_avail_bandwidth;
    st_mac_h2d_usr_info.en_cur_bandwidth = pst_mac_user->en_cur_bandwidth;
    st_mac_h2d_usr_info.us_user_idx = pst_mac_user->us_assoc_id;
    st_mac_h2d_usr_info.en_user_pmf = pst_mac_user->st_cap_info.bit_pmf_active;
    st_mac_h2d_usr_info.uc_arg1 = pst_mac_user->st_ht_hdl.uc_max_rx_ampdu_factor;
    st_mac_h2d_usr_info.uc_arg2 = pst_mac_user->st_ht_hdl.uc_min_mpdu_start_spacing;
    st_mac_h2d_usr_info.en_user_asoc_state = pst_mac_user->en_user_asoc_state;

    /* 协议模式信息同步到dmac */
    st_mac_h2d_usr_info.en_avail_protocol_mode = pst_mac_user->en_avail_protocol_mode;

    st_mac_h2d_usr_info.en_cur_protocol_mode = pst_mac_user->en_cur_protocol_mode;
    st_mac_h2d_usr_info.en_protocol_mode = pst_mac_user->en_protocol_mode;
    st_mac_h2d_usr_info.en_bandwidth_cap = pst_mac_user->en_bandwidth_cap;

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_USR_INFO_SYN, OAL_SIZEOF(st_mac_h2d_usr_info),
                                    (oal_uint8 *)(&st_mac_h2d_usr_info));
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_user_info_syn::hmac_config_send_event failed[%d],user_id[%d].}",
                         ul_ret, pst_mac_user->us_assoc_id);
    }

    return ul_ret;
}


oal_uint32 hmac_config_sta_vap_info_syn(mac_vap_stru *pst_mac_vap)
{
    oal_uint32 ul_ret;
    mac_h2d_vap_info_stru st_mac_h2d_vap_info;

    st_mac_h2d_vap_info.us_sta_aid = pst_mac_vap->us_sta_aid;
    st_mac_h2d_vap_info.uc_uapsd_cap = pst_mac_vap->uc_uapsd_cap;
    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_STA_VAP_INFO_SYN, OAL_SIZEOF(mac_h2d_vap_info_stru),
                                    (oal_uint8 *)(&st_mac_h2d_vap_info));
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_sta_vap_info_syn::hmac_config_sta_vap_info_syn failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_init_user_security_port(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    oal_uint32 ul_ret;
    mac_cfg80211_init_port_stru st_init_port;

    /* 初始化认证端口信息 */
    mac_vap_init_user_security_port(pst_mac_vap, pst_mac_user);

    memcpy_s(st_init_port.auc_mac_addr, OAL_MAC_ADDR_LEN, pst_mac_user->auc_user_mac_addr, OAL_MAC_ADDR_LEN);
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_INIT_SECURTIY_PORT, OAL_SIZEOF(st_init_port),
                                    (oal_uint8 *)&st_init_port);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_WPA,
                       "{hmac_config_user_security_port::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_user_set_asoc_state(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user,
                                    mac_user_asoc_state_enum_uint8 en_value)
{
    oal_uint32 ul_ret;

    mac_user_set_asoc_state(pst_mac_user, en_value);

    /* dmac offload架构下，同步user关联状态信息到dmac */
    ul_ret = hmac_config_user_asoc_state_syn(pst_mac_vap, pst_mac_user);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_user_set_asoc_state::hmac_config_user_asoc_state_syn failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_send_2040_coext(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_set_2040_coexist_stru *pst_2040_coexist;
    oal_netbuf_stru *pst_netbuf = OAL_PTR_NULL;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_uint16 us_frame_len;

    pst_2040_coexist = (mac_cfg_set_2040_coexist_stru *)puc_param;
    oam_warning_log2(0, 0, "hmac_config_send_2040_coext::coinfo=%d chan=%d",
                     pst_2040_coexist->ul_coext_info, pst_2040_coexist->ul_channel_report);

    /* 申请管理帧内存 */
    pst_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_send_2040_coext::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_netbuf_prev(pst_netbuf) = OAL_PTR_NULL;
    oal_netbuf_next(pst_netbuf) = OAL_PTR_NULL;

    /* 封装20/40 共存管理帧 */
    us_frame_len = mac_encap_2040_coext_mgmt((oal_void *)pst_mac_vap, pst_netbuf,
                                             (oal_uint8)pst_2040_coexist->ul_coext_info,
                                             pst_2040_coexist->ul_channel_report);

    oal_netbuf_put(pst_netbuf, us_frame_len);

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    memset_s(pst_tx_ctl, oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    pst_tx_ctl->us_mpdu_len = us_frame_len;
    pst_tx_ctl->us_tx_user_idx = 0xFFFF;
    pst_tx_ctl->uc_ac = WLAN_WME_AC_MGMT;

    /* 抛事件到DMAC发送管理帧 */
    ul_ret = hmac_tx_mgmt_send_event(pst_mac_vap, pst_netbuf, us_frame_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);

        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_send_2040_coext::hmac_tx_mgmt_send_event failed.}", ul_ret);
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_2040_coext_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_device_stru *pst_mac_device;
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    mac_ap_ch_info_stru *pst_ch_list;
    oal_uint32 ul_idx;
#endif
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_2040_coext_info::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    for (ul_idx = 0; ul_idx < MAC_MAX_SUPP_CHANNEL; ul_idx++) {
        pst_ch_list = &(pst_mac_device->st_ap_channel_list[ul_idx]);
        oam_error_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_2040_coext_info::chan_idx=%d num_networks=%d, chan_type=%d.}",
                       ul_idx, pst_ch_list->us_num_networks, pst_ch_list->en_ch_type);
    }
#endif

    return OAL_SUCC;
}


oal_uint32 hmac_config_get_version(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_GET_VERSION, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_get_version::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_get_ant(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_GET_ANT, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_get_version::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_get_fem_pa_status(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_CHECK_FEM_PA, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_get_fem_pa_status::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY

oal_uint32 hmac_config_set_opmode_notify(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint8 uc_value;

    uc_value = *puc_param;

    if (uc_value >= OAL_BUTT) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_set_opmode_notify::uc_value is limit! value = [%d].}\r\n", uc_value);
        return OAL_FAIL;
    }

    if (mac_mib_get_VHTOptionImplemented(pst_mac_vap) == OAL_TRUE) {
        mac_mib_set_OperatingModeNotificationImplemented(pst_mac_vap, (oal_bool_enum_uint8)uc_value);
    } else {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_opmode_notify::pst_mac_vap is not 11ac. en_protocol = [%d].}\r\n",
                         pst_mac_vap->en_protocol);
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_get_user_rssbw(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_add_user_param_stru *pst_user;
    hmac_vap_stru *pst_hmac_vap;
    hmac_user_stru *pst_hmac_user;
    oal_int8 ac_tmp_buff[200];

    pst_user = (mac_cfg_add_user_param_stru *)puc_param;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_get_user_rssbw::pst_hmac_vap null.}");
        return OAL_FAIL;
    }

    pst_hmac_user = mac_vap_get_hmac_user_by_addr(pst_mac_vap, pst_user->auc_mac_addr);
    if (pst_hmac_user == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_get_user_rssbw::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    snprintf_s(ac_tmp_buff, OAL_SIZEOF(ac_tmp_buff), OAL_SIZEOF(ac_tmp_buff) - 1,
        "%02X:XX:XX:%02X:%02X:%02X\r\nnss_cap: %s, avail_nss: %s.\n\ruser bw_cap: %s, avail_bw: %s.",
        pst_user->auc_mac_addr[0], /* auc_mac_addr第0byte为参数输出打印 */
        pst_user->auc_mac_addr[3], /* auc_mac_addr第3byte为参数输出打印 */
        pst_user->auc_mac_addr[4], /* auc_mac_addr第4byte为参数输出打印 */
        pst_user->auc_mac_addr[5], /* auc_mac_addr第5byte为参数输出打印 */
        hmac_config_nss2string(pst_hmac_user->st_user_base_info.uc_num_spatial_stream),
        hmac_config_nss2string(pst_hmac_user->st_user_base_info.uc_avail_num_spatial_stream),
        hmac_config_b_w2string(pst_hmac_user->st_user_base_info.en_bandwidth_cap),
        hmac_config_b_w2string(pst_hmac_user->st_user_base_info.en_avail_bandwidth));
    oam_print(ac_tmp_buff);

    return OAL_SUCC;
}

#endif


oal_uint32 hmac_config_set_ampdu_aggr_num(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_AGGR_NUM, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_ampdu_aggr_num::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_stbc_cap(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_bool_enum_uint8 uc_value;

    uc_value = *puc_param;

    if (oal_unlikely(pst_mac_vap->pst_mib_info == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_stbc_cap::pst_mib_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (uc_value == 1) {
        pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11TxSTBCOptionImplemented = OAL_TRUE;
        pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11RxSTBCOptionImplemented = OAL_TRUE;
        pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11TxSTBCOptionActivated = OAL_TRUE;
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTTxSTBCOptionImplemented = OAL_TRUE;
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTRxSTBCOptionImplemented = OAL_TRUE;
    } else if (uc_value == 0) {
        pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11TxSTBCOptionImplemented = OAL_FALSE;
        pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11RxSTBCOptionImplemented = OAL_FALSE;
        pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11TxSTBCOptionActivated = OAL_FALSE;
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTTxSTBCOptionImplemented = OAL_FALSE;
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTRxSTBCOptionImplemented = OAL_FALSE;
    } else {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_set_stbc_cap::stbc_value is limit! value = [%d].}\r\n", uc_value);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) /* hi1102-cb set at both side (HMAC to DMAC) */
    return hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_STBC_CAP, us_len, puc_param);
#else
    return OAL_SUCC;
#endif
}


oal_uint32 hmac_config_set_ldpc_cap(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint8 uc_value;

    uc_value = (oal_bool_enum_uint8)(*puc_param);

    if (oal_unlikely(pst_mac_vap->pst_mib_info == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_ldpc_cap::pst_mib_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (uc_value == 1) {
        pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11LDPCCodingOptionImplemented = OAL_TRUE;
        pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11LDPCCodingOptionActivated = OAL_TRUE;
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTLDPCCodingOptionImplemented = OAL_TRUE;
    } else if (uc_value == 0) {
        pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11LDPCCodingOptionImplemented = OAL_FALSE;
        pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11LDPCCodingOptionActivated = OAL_FALSE;
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTLDPCCodingOptionImplemented = OAL_FALSE;
    } else {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_set_ldpc_cap::ldpc_value is limit! value = [%d].}\r\n", uc_value);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) /* hi1102-cb set at both side (HMAC to DMAC) */
    return hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_LDPC_CAP, us_len, puc_param);
#else
    return OAL_SUCC;
#endif
}

oal_uint32 hmac_config_set_pmksa(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_pmksa_param_stru *pst_cfg_pmksa = OAL_PTR_NULL;
    hmac_pmksa_cache_stru *pst_pmksa_cache = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_pmksa_entry = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_pmksa_entry_tmp = OAL_PTR_NULL;
    oal_uint32 ul_pmksa_count = 0;

    if ((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_config_set_pmksa param null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cfg_pmksa = (mac_cfg_pmksa_param_stru *)puc_param;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_pmksa::pst_hmac_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (oal_dlist_is_empty(&(pst_hmac_vap->st_pmksa_list_head))) {
        oal_dlist_init_head(&(pst_hmac_vap->st_pmksa_list_head));
    }

    oal_dlist_search_for_each_safe(pst_pmksa_entry, pst_pmksa_entry_tmp, &(pst_hmac_vap->st_pmksa_list_head))
    {
        pst_pmksa_cache = oal_dlist_get_entry(pst_pmksa_entry, hmac_pmksa_cache_stru, st_entry);
        /* 已存在时，先删除，保证最新的pmk在dlist头部 */
        if (oal_compare_mac_addr(pst_cfg_pmksa->auc_bssid, pst_pmksa_cache->auc_bssid) == 0) {
            oal_dlist_delete_entry(pst_pmksa_entry);
            oal_mem_free_m(pst_pmksa_cache, OAL_TRUE);
            oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_set_pmksa:: DEL first [%02X:XX:XX:XX:%02X:%02X]}",
                             /* auc_bssid第0、4、5byte为参数输出打印 */
                             pst_cfg_pmksa->auc_bssid[0], pst_cfg_pmksa->auc_bssid[4], pst_cfg_pmksa->auc_bssid[5]);
        }
        ul_pmksa_count++;
    }

    if (ul_pmksa_count > WLAN_PMKID_CACHE_SIZE) {
        /* 超过最大个数时，先队列尾，保证最新的pmk在dlist头部 */
        oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_pmksa:: can't store more pmksa for [%02X:XX:XX:XX:%02X:%02X]}",
                         /* auc_bssid第0、4、5byte为参数输出打印 */
                         pst_cfg_pmksa->auc_bssid[0], pst_cfg_pmksa->auc_bssid[4], pst_cfg_pmksa->auc_bssid[5]);
        pst_pmksa_entry = oal_dlist_delete_tail(&(pst_hmac_vap->st_pmksa_list_head));
        pst_pmksa_cache = oal_dlist_get_entry(pst_pmksa_entry, hmac_pmksa_cache_stru, st_entry);
        oal_mem_free_m(pst_pmksa_cache, OAL_TRUE);
    }

    pst_pmksa_cache = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(hmac_pmksa_cache_stru), OAL_TRUE);
    if (pst_pmksa_cache == OAL_PTR_NULL) {
        oam_error_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_set_pmksa:: oal_mem_alloc_m fail [%02X:XX:XX:XX:%02X:%02X]}",
                       /* auc_bssid第0、4、5byte为参数输出打印 */
                       pst_cfg_pmksa->auc_bssid[0], pst_cfg_pmksa->auc_bssid[4], pst_cfg_pmksa->auc_bssid[5]);
        return OAL_ERR_CODE_PTR_NULL;
    }

    memcpy_s(pst_pmksa_cache->auc_bssid, OAL_MAC_ADDR_LEN, pst_cfg_pmksa->auc_bssid, OAL_MAC_ADDR_LEN);
    memcpy_s(pst_pmksa_cache->auc_pmkid, WLAN_PMKID_LEN, pst_cfg_pmksa->auc_pmkid, WLAN_PMKID_LEN);

    oal_dlist_add_head(&(pst_pmksa_cache->st_entry), &(pst_hmac_vap->st_pmksa_list_head));

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_pmksa:: SET pmksa for [%02X:XX:XX:XX:%02X:%02X] OK!}",
                     /* auc_bssid第0、4、5byte为参数输出打印 */
                     pst_cfg_pmksa->auc_bssid[0], pst_cfg_pmksa->auc_bssid[4], pst_cfg_pmksa->auc_bssid[5]);

    return OAL_SUCC;
}


oal_uint32 hmac_config_del_pmksa(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_pmksa_param_stru *pst_cfg_pmksa = OAL_PTR_NULL;
    hmac_pmksa_cache_stru *pst_pmksa_cache = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_pmksa_entry = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_pmksa_entry_tmp = OAL_PTR_NULL;

    if ((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_config_del_pmksa param null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cfg_pmksa = (mac_cfg_pmksa_param_stru *)puc_param;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_pmksa::pst_hmac_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (oal_dlist_is_empty(&(pst_hmac_vap->st_pmksa_list_head))) {
        oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_del_pmksa:: pmksa dlist is null [%02X:XX:XX:XX:%02X:%02X]}",
                         /* auc_bssid第0、4、5byte为参数输出打印 */
                         pst_cfg_pmksa->auc_bssid[0], pst_cfg_pmksa->auc_bssid[4], pst_cfg_pmksa->auc_bssid[5]);
    }

    oal_dlist_search_for_each_safe(pst_pmksa_entry, pst_pmksa_entry_tmp, &(pst_hmac_vap->st_pmksa_list_head))
    {
        pst_pmksa_cache = oal_dlist_get_entry(pst_pmksa_entry, hmac_pmksa_cache_stru, st_entry);
        if (oal_compare_mac_addr(pst_cfg_pmksa->auc_bssid, pst_pmksa_cache->auc_bssid) == 0) {
            oal_dlist_delete_entry(pst_pmksa_entry);
            oal_mem_free_m(pst_pmksa_cache, OAL_TRUE);
            oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_del_pmksa:: DEL pmksa of [%02X:XX:XX:XX:%02X:%02X]}",
                             /* auc_bssid第0、4、5byte为参数输出打印 */
                             pst_cfg_pmksa->auc_bssid[0], pst_cfg_pmksa->auc_bssid[4], pst_cfg_pmksa->auc_bssid[5]);
            return OAL_SUCC;
        }
    }
    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_del_pmksa:: NO pmksa of [%02X:XX:XX:XX:%02X:%02X]}",
                     /* auc_bssid第0、4、5byte为参数输出打印 */
                     pst_cfg_pmksa->auc_bssid[0], pst_cfg_pmksa->auc_bssid[4], pst_cfg_pmksa->auc_bssid[5]);
    return OAL_SUCC;
}


oal_uint32 hmac_config_flush_pmksa(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_pmksa_cache_stru *pst_pmksa_cache = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_pmksa_entry = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_pmksa_entry_tmp = OAL_PTR_NULL;

    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_config_flush_pmksa param null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_flush_pmksa::pst_hmac_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (oal_dlist_is_empty(&(pst_hmac_vap->st_pmksa_list_head))) {
        return OAL_SUCC;
    }

    oal_dlist_search_for_each_safe(pst_pmksa_entry, pst_pmksa_entry_tmp, &(pst_hmac_vap->st_pmksa_list_head))
    {
        pst_pmksa_cache = oal_dlist_get_entry(pst_pmksa_entry, hmac_pmksa_cache_stru, st_entry);

        oal_dlist_delete_entry(pst_pmksa_entry);
        oal_mem_free_m(pst_pmksa_cache, OAL_TRUE);
        oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_flush_pmksa:: DEL pmksa of [%02X:XX:XX:XX:%02X:%02X]}",
                         /* auc_bssid第0、4、5byte为参数输出打印 */
                         pst_pmksa_cache->auc_bssid[0], pst_pmksa_cache->auc_bssid[4], pst_pmksa_cache->auc_bssid[5]);
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_scan_abort(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    hmac_device_stru *pst_hmac_device;
    oal_uint32 ul_ret;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_scan_abort::pst_hmac_device is null, dev_id[%d].}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_scan_abort::pst_hmac_vap is null.}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                     "{hmac_config_scan_abort::scan abort,curr_scan_vap_id:%d vap state: %d.}",
                     pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.uc_vap_id,
                     pst_hmac_vap->st_vap_base_info.en_vap_state);

    /* 根据当前扫描的类型和当前vap的状态，决定切换vap的状态，如果是前景扫描，才需要切换vap的状态 */
    if (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_WAIT_SCAN) {
            /* 改变vap状态到SCAN_COMP */
            hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
        } else if (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_UP) {
            /* 背景扫描时需要进行帧过滤的配置 */
            hmac_set_rx_filter_value(&(pst_hmac_vap->st_vap_base_info));
        } else if (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
            hmac_p2p_listen_timeout(pst_hmac_vap, &pst_hmac_vap->st_vap_base_info);
        }
    }

    if ((pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP)
        && (pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.en_vap_last_state != MAC_VAP_STATE_BUTT)) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_scan_abort::en_vap_last_state:%d}",
                         pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.en_vap_last_state);
        hmac_fsm_change_state(pst_hmac_vap, pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.en_vap_last_state);
        pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.en_vap_last_state = MAC_VAP_STATE_BUTT;
    }
    /* 清除扫描结果上报的回调函数，无需上报 */
    if (pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.uc_vap_id == pst_mac_vap->uc_vap_id) {
        pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.p_fn_cb = OAL_PTR_NULL;
    }

    /***************************************************************************
                         抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap,
                                    WLAN_CFGID_SCAN_ABORT,
                                    us_len,
                                    puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_scan_abort::hmac_config_send_event failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_remain_on_channel(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    mac_remain_on_channel_param_stru *pst_remain_on_channel = (mac_remain_on_channel_param_stru *)puc_param;

    /* 1.1 判断入参 */
    if ((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_P2P, "{hmac_config_remain_on_channel null ptr: pst_mac_vap=%x; puc_param=%x}\r\n",
                       (uintptr_t)pst_mac_vap, (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_config_remain_on_channel:mac_device[%d]null!}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获取hmac device */
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_config_remain_on_channel::hmac_device[%d] null.}",
                         pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    /* 1.2 检查是否能进入监听状态 */
    ul_ret = hmac_p2p_check_can_enter_state(pst_mac_vap, HMAC_FSM_INPUT_LISTEN_REQ);
    if (ul_ret != OAL_SUCC) {
        /* 不能进入监听状态，返回设备忙 */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{remain_on_channel fail,device busy:ret=%d}", ul_ret);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    /* 判断当前扫描是否正在执行 */
    if (pst_hmac_device->st_scan_mgmt.en_is_scanning == OAL_TRUE) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{remain_on_channel::scan request is rejected.}");
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

#ifdef _PRE_WLAN_FEATURE_ROAM
    /* 判断当前是否正在执行漫游 */
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_config_remain_on_channel::roam reject new scan.}");
        return OAL_ERR_CODE_CONFIG_BUSY;
    }
#endif  // _PRE_WLAN_FEATURE_ROAM

    /* 1.3 获取home 信道和信道类型。如果返回主信道为0，表示没有设备处于up 状态，监听后不需要返回主信道 */
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                       "{hmac_config_remain_on_channel::get_hmac_vap null.vap_id = %d}", pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 保存内核下发的监听信道信息，用于监听超时或取消监听时返回 */
    pst_mac_device->st_p2p_info.st_listen_channel = pst_remain_on_channel->st_listen_channel;

    /* 由于p2p0和 p2p cl 共用一个VAP 结构，故在进入监听时，需要保存之前的状态，便于监听结束时返回 */
    if (pst_mac_vap->en_vap_state != MAC_VAP_STATE_STA_LISTEN) {
        pst_mac_device->st_p2p_info.en_last_vap_state = pst_mac_vap->en_vap_state;
    }
    pst_remain_on_channel->en_last_vap_state = pst_mac_device->st_p2p_info.en_last_vap_state;

    /* 3.1 修改VAP 状态为监听 */
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                       "{hmac_config_remain_on_channel fail!pst_hmac_vap is null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 状态机调用:  hmac_p2p_remain_on_channel */
    ul_ret = hmac_fsm_call_func_sta(pst_hmac_vap, HMAC_FSM_INPUT_LISTEN_REQ, (oal_void *)(pst_remain_on_channel));
    if (ul_ret != OAL_SUCC) {
        /* DMAC 设置切换信道失败 */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_config_remain_on_channel fail: ul_ret=%d}\r\n",
                         ul_ret);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    oam_info_log3(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                  "{hmac_config_remain_on_channel succ: l_channel=%d, ul_duration=%d, band=%d}\r\n",
                  pst_remain_on_channel->uc_listen_channel, pst_remain_on_channel->ul_listen_duration,
                  pst_remain_on_channel->en_band);
    return OAL_SUCC;
}


oal_uint32 hmac_config_cancel_remain_on_channel(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                         "hmac_config_cancel_remain_on_channel::mac_res_get_hmac_vap fail.vap_id = %u",
                         pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
#ifdef _PRE_WLAN_FEATURE_P2P
        hmac_p2p_listen_timeout(pst_hmac_vap, pst_mac_vap);
#endif
    } else {
        hmac_p2p_send_listen_expired_to_host(pst_hmac_vap);
    }
    return OAL_SUCC;
}


oal_uint32 hmac_config_vap_classify_en(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_val;
    mac_device_stru *pst_mac_device;
    oal_int8 ac_string[OAM_PRINT_FORMAT_LENGTH];

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "hmac_config_vap_classify_en::mac_res_get_dev fail.device_id = %u",
                         pst_mac_vap->uc_device_id);

        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_val = *((oal_uint32 *)puc_param);

    if (ul_val == 0xff) {
        /* 打印当前的值 */
        snprintf_s(ac_string, sizeof(ac_string), sizeof(ac_string) - 1, "device classify en is %d\n",
            pst_mac_device->en_vap_classify);

        oam_print(ac_string);

        return OAL_SUCC;
    }

    if (ul_val == 0) {
        pst_mac_device->en_vap_classify = OAL_FALSE;
    } else {
        pst_mac_device->en_vap_classify = OAL_TRUE;
    }

    return OAL_SUCC;
}

oal_uint32 hmac_config_query_station_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_STATION_STATS, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_phy_stat_info::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}

oal_uint32 hmac_config_query_chr_info_ext(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_CHR_EXT_INFO, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_query_chr_info_ext::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_query_rssi(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_RSSI, us_len, puc_param);

    return ul_ret;
}


oal_uint32 hmac_config_query_rate(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_RATE, us_len, puc_param);

    return ul_ret;
}

#ifdef _PRE_WLAN_DFT_STAT

oal_uint32 hmac_config_query_ani(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_ANI, us_len, puc_param);

    return ul_ret;
}
#endif


oal_uint32 hmac_config_vap_classify_tid(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_val;
    hmac_vap_stru *pst_hmac_vap;
    oal_int8 ac_string[OAM_PRINT_FORMAT_LENGTH];

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_config_vap_classify_tid::mac_res_get_hmac_vap fail.vap_id = %u", pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_val = *((oal_uint32 *)puc_param);

    if (ul_val == 0xff) {
        /* 打印当前的值 */
        snprintf_s(ac_string, sizeof(ac_string), sizeof(ac_string) - 1, "vap classify tid is %d\n",
            pst_hmac_vap->uc_classify_tid);

        oam_print(ac_string);

        return OAL_SUCC;
    }

    if (ul_val >= WLAN_TIDNO_BUTT) {
        /* 打印当前的值 */
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "val is invalid:%d, vap classify tid is %d", ul_val,
                         pst_hmac_vap->uc_classify_tid);
        return OAL_SUCC;
    }

    pst_hmac_vap->uc_classify_tid = (oal_uint8)ul_val;

    return OAL_SUCC;
}


oal_uint32 hmac_atcmdsrv_fem_pa_response(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    dmac_atcmdsrv_atcmd_response_event *pst_atcmdsrv_fem_pa_response_event = OAL_PTR_NULL;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_fem_pa_response::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    pst_atcmdsrv_fem_pa_response_event = (dmac_atcmdsrv_atcmd_response_event *)(puc_param);
    if (pst_atcmdsrv_fem_pa_response_event->uc_event_id == OAL_ATCMDSRV_FEM_PA_INFO_EVENT) {
        pst_hmac_vap->st_atcmdsrv_get_status.ul_check_fem_pa_status = pst_atcmdsrv_fem_pa_response_event->ul_event_para;
    }
    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_check_fem_pa_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}

oal_uint32 hmac_atcmdsrv_dbb_num_response(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    dmac_atcmdsrv_atcmd_response_event *pst_atcmdsrv_dbb_num_response_event = OAL_PTR_NULL;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_dbb_num_response::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    pst_atcmdsrv_dbb_num_response_event = (dmac_atcmdsrv_atcmd_response_event *)(puc_param);
    if (pst_atcmdsrv_dbb_num_response_event->uc_event_id == OAL_ATCMDSRV_DBB_NUM_INFO_EVENT) {
        pst_hmac_vap->st_atcmdsrv_get_status.ul_dbb_num = pst_atcmdsrv_dbb_num_response_event->ul_event_para;
    }
    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_get_dbb_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}


oal_uint32 hmac_atcmdsrv_get_ant_response(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    dmac_atcmdsrv_atcmd_response_event *pst_atcmdsrv_dbb_num_response_event = OAL_PTR_NULL;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_dbb_num_response::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    pst_atcmdsrv_dbb_num_response_event = (dmac_atcmdsrv_atcmd_response_event *)(puc_param);
    if (pst_atcmdsrv_dbb_num_response_event->uc_event_id == OAL_ATCMDSRV_GET_ANT) {
        pst_hmac_vap->st_atcmdsrv_get_status.uc_ant_status = pst_atcmdsrv_dbb_num_response_event->ul_event_para;
    }
    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_get_ant_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}


oal_uint32 hmac_atcmdsrv_get_rx_pkcg(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    dmac_atcmdsrv_atcmd_response_event *pst_atcmdsrv_get_rx_pkcg_event = OAL_PTR_NULL;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_get_rx_pkcg::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    pst_atcmdsrv_get_rx_pkcg_event = (dmac_atcmdsrv_atcmd_response_event *)(puc_param);
    if (pst_atcmdsrv_get_rx_pkcg_event->uc_event_id == OAL_ATCMDSRV_GET_RX_PKCG) {
        pst_hmac_vap->st_atcmdsrv_get_status.ul_rx_pkct_succ_num = pst_atcmdsrv_get_rx_pkcg_event->ul_event_para;
        pst_hmac_vap->st_atcmdsrv_get_status.s_rx_rssi = pst_atcmdsrv_get_rx_pkcg_event->s_always_rx_rssi;
    }
    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_get_rx_pkct_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}
oal_uint8 g_uc_dev_lte_gpio_level = 0x0;
oal_uint8 hmac_atcmsrv_get_lte_gpio_level(oal_void)
{
    return g_uc_dev_lte_gpio_level;
}
oal_void hmac_atcmsrv_set_lte_gpio_level(oal_uint8 uc_dev_lte_gpio_level)
{
    g_uc_dev_lte_gpio_level = uc_dev_lte_gpio_level;
}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32 hmac_atcmdsrv_lte_gpio_check(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    dmac_atcmdsrv_atcmd_response_event *pst_atcmdsrv_lte_gpio_check_event = OAL_PTR_NULL;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_lte_gpio_check::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_atcmdsrv_lte_gpio_check_event = (dmac_atcmdsrv_atcmd_response_event *)(puc_param);
    if (pst_atcmdsrv_lte_gpio_check_event->uc_event_id == OAL_ATCMDSRV_LTE_GPIO_CHECK) {
        /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
        pst_hmac_vap->st_atcmdsrv_get_status.uc_lte_gpio_check_flag = OAL_TRUE;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        hmac_atcmsrv_set_lte_gpio_level(pst_atcmdsrv_lte_gpio_check_event->uc_reserved);
#endif
        oal_wait_queue_wake_up(&(pst_hmac_vap->query_wait_q));
    }

    return OAL_SUCC;
}
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

oal_uint32 hmac_atcmdsrv_report_efuse_reg(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    oal_uint16 ul_loop = 0;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_lte_gpio_check::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    for (ul_loop = 0; ul_loop < 16; ul_loop++) { /* g_us_efuse_buffer数组的长度是16 */
        hmac_set_efuse_buffer(ul_loop, *(oal_uint16 *)(puc_param));
        puc_param = puc_param + 2; /* 每次循环puc_param值加 2 */
    }
    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_efuse_reg_flag = OAL_TRUE;
    oal_wait_queue_wake_up(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}
#endif
#endif

OAL_STATIC oal_uint32 hmac_config_d2h_user_info_syn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len,
                                                    oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_d2h_syn_info_stru *pst_syn_info = OAL_PTR_NULL;
    mac_user_stru *pst_mac_user = OAL_PTR_NULL;
    oal_uint8 uc_idx;
    oal_uint32 ul_ret;

    if (puc_param == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_d2h_user_info_syn::puc_param[%p]!}",
                         (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_syn_info = (mac_d2h_syn_info_stru *)puc_param;

    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(pst_syn_info->us_user_idx);
    if (oal_unlikely(pst_mac_user == OAL_PTR_NULL)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_user_info_syn::pst_mac_user null.user idx [%d]}",
                         pst_syn_info->us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 同步USR带宽 */
    mac_user_set_bandwidth_cap(pst_mac_user, pst_syn_info->en_bandwidth_cap);
    mac_user_set_bandwidth_info(pst_mac_user, pst_syn_info->en_avail_bandwidth, pst_syn_info->en_cur_bandwidth);

    /* 同步信道信息 */
    ul_ret = mac_get_channel_idx_from_num(pst_mac_vap->st_channel.en_band,
                                          pst_syn_info->st_channel.uc_chan_number, &uc_idx);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                         "{hmac_d2h_user_info_syn::mac_get_channel_idx_from_num failed[%d].}", ul_ret);

        return ul_ret;
    }

    pst_mac_vap->st_channel.uc_chan_number = pst_syn_info->st_channel.uc_chan_number;
    pst_mac_vap->st_channel.en_bandwidth = pst_syn_info->st_channel.en_bandwidth;
    pst_mac_vap->st_channel.uc_idx = uc_idx;

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_d2h_user_info_syn::channe[%d],bw[%d]avail bw[%d],current[%d]!}",
                     pst_mac_vap->st_channel.uc_chan_number, pst_mac_vap->st_channel.en_bandwidth,
                     pst_mac_user->en_avail_bandwidth, pst_mac_user->en_cur_bandwidth);

#endif
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_BTCOEX
OAL_STATIC oal_uint32 hmac_config_btcoex_status_syn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len,
                                                    oal_uint8 *puc_param)
{
    hal_btcoex_btble_status_stru st_btble_status_old;
    hal_btcoex_btble_status_stru st_btble_status_new;
    hmac_device_stru *pst_hmac_device;

    if (puc_param == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_btcoex_status_syn::puc_param is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_btcoex_status_syn::pst_hmac_device is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    st_btble_status_old = pst_hmac_device->st_hmac_device_btcoex.st_btble_status;
    if (memcpy_s(&pst_hmac_device->st_hmac_device_btcoex.st_btble_status, OAL_SIZEOF(hal_btcoex_btble_status_stru),
                 puc_param, OAL_SIZEOF(hal_btcoex_btble_status_stru)) != EOK) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_btcoex_status_syn::memcpy fail}");
        return OAL_FAIL;
    }
    st_btble_status_new = pst_hmac_device->st_hmac_device_btcoex.st_btble_status;
    if (pst_hmac_device->pst_device_base_info != OAL_PTR_NULL) {
        hmac_btcoex_process_btble_status(pst_hmac_device->pst_device_base_info, &st_btble_status_old,
                                         &st_btble_status_new);
    }
    /* 音乐或者电话场景下将绑核门限采用共存的参数值 */
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    if ((st_btble_status_new.un_bt_status.st_bt_status.bit_bt_a2dp == OAL_TRUE) ||
        (st_btble_status_new.un_bt_status.st_bt_status.bit_bt_sco == OAL_TRUE)) {
        g_st_thread_bindcpu.en_btcoex_flag = OAL_TRUE;
    } else {
        g_st_thread_bindcpu.en_btcoex_flag = OAL_FALSE;
    }
#endif
    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_NRCOEX

OAL_STATIC oal_uint32 hmac_config_nrcoex_info_syn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len,
                                                  oal_uint8 *puc_param)
{
    hmac_device_stru *pst_hmac_device;
    hmac_nrcoex_info_query_stru *pst_nrcoex_query;
    wlan_nrcoex_info_stru *pst_nrcoex_info;

    if (puc_param == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_nrcoex_info_syn:puc_param is null");
        return OAL_FAIL;
    }
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_nrcoex_info_syn:pst_hmac_device is null");
        return OAL_FAIL;
    }
    pst_nrcoex_query = &pst_hmac_device->st_nrcoex_query;
    pst_nrcoex_info = (wlan_nrcoex_info_stru *)puc_param;
    memcpy_s(&pst_nrcoex_query->st_nrcoex_info, OAL_SIZEOF(wlan_nrcoex_info_stru),
        pst_nrcoex_info, OAL_SIZEOF(wlan_nrcoex_info_stru));
    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "hmac_config_nrcoex_info_syn:mode:freq[%d]bw[%d],wifi:freq[%d]bw[%d]",
                     pst_nrcoex_info->us_modem_center_freq, pst_nrcoex_info->en_modem_bw,
                     pst_nrcoex_info->us_wifi_center_freq, pst_nrcoex_info->en_wifi_bw);
    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_nrcoex_info_syn:wifi:avoid[%d],priority[%d]",
                     pst_nrcoex_info->en_wifi_avoid_flag, pst_nrcoex_info->en_wifi_priority);
    pst_nrcoex_query->en_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&pst_nrcoex_query->st_wait_queue);
    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_VOWIFI

oal_uint32 hmac_config_vowifi_report(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event;

    /* 目前仅Legacy sta支持这种操作 */
    if (pst_mac_vap->pst_vowifi_cfg_param == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vowifi_report::pst_vowifi_cfg_param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设备up，切使能了vowifi状态才能触发切换vowifi状态 */
    if (pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_mode == VOWIFI_DISABLE_REPORT) {
        return OAL_SUCC;
    }

    /* "申请vowifi逻辑切换"仅上报一次直到重新更新vowifi模式 */
    if (pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_reported == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_vowifi_report::vowifi been reported once!}");
        return OAL_SUCC;
    }

    pst_event_mem = frw_event_alloc_m(0);
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_config_vowifi_report::frw_event_alloc fail,size=0!}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    pst_event = (frw_event_stru *)pst_event_mem->puc_data;
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_VOWIFI_REPORT,
                       0,
                       FRW_EVENT_PIPELINE_STAGE_0,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);
    pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_reported = OAL_TRUE;
    return OAL_SUCC;
}
#endif /* _PRE_WLAN_FEATURE_VOWIFI */


OAL_STATIC oal_uint32 hmac_config_query_sta_mgmt_send_state_rsp(mac_vap_stru *pst_mac_vap,
    oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    mac_cfg_query_mgmt_send_status_stru *pst_param;
    pst_param = (mac_cfg_query_mgmt_send_status_stru *)puc_param;

    if (!IS_LEGACY_STA(pst_mac_vap)) {
        return OAL_FAIL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_query_sta_mngpkt_sendstat_rsp::mac_res_get_dev failed.}");
        return OAL_FAIL;
    }

    pst_mac_device->uc_auth_req_sendst = pst_param->uc_auth_req_st;
    pst_mac_device->uc_asoc_req_sendst = pst_param->uc_asoc_req_st;
    pst_mac_device->en_report_mgmt_req_status = OAL_TRUE;
    oam_warning_log2(0, OAM_SF_CFG, "hmac_config_query_sta_mgmt_send_state_rsp:auth_req_sendst=%d, asoc_req_sendst=%d",
                     pst_param->uc_auth_req_st, pst_param->uc_asoc_req_st);
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_config_query_rssi_rsp(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_query_rssi_stru *pst_param;
    hmac_user_stru *pst_hmac_user;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;

    pst_param = (mac_cfg_query_rssi_stru *)puc_param;

    pst_hmac_user = mac_res_get_hmac_user(pst_param->us_user_id);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "hmac_config_query_rssi_rsp: pst_hmac_user is null ptr. user id:%d", pst_param->us_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_query_rssi_rsp: pst_hmac_vap is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user->c_rssi = pst_param->c_rssi;

    pst_hmac_vap->station_info_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_config_query_rate_rsp(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_query_rate_stru *pst_param;
    hmac_user_stru *pst_hmac_user;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;

    pst_param = (mac_cfg_query_rate_stru *)puc_param;

    pst_hmac_user = mac_res_get_hmac_user(pst_param->us_user_id);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "hmac_config_query_rate_rsp: pst_hmac_user is null ptr. user id:%d", pst_param->us_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_query_rate_rsp: pst_hmac_vap is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user->ul_tx_rate = pst_param->ul_tx_rate;
    pst_hmac_user->ul_tx_rate_min = pst_param->ul_tx_rate_min;
    pst_hmac_user->ul_tx_rate_max = pst_param->ul_tx_rate_max;
    pst_hmac_user->ul_rx_rate = pst_param->ul_rx_rate;
    pst_hmac_user->ul_rx_rate_min = pst_param->ul_rx_rate_min;
    pst_hmac_user->ul_rx_rate_max = pst_param->ul_rx_rate_max;
#ifdef _PRE_WLAN_DFT_STAT
    pst_hmac_user->uc_cur_per = pst_param->uc_cur_per;
    pst_hmac_user->uc_bestrate_per = pst_param->uc_bestrate_per;
#endif

    pst_hmac_vap->station_info_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_DFT_STAT

OAL_STATIC oal_uint32 hmac_config_query_ani_rsp(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_query_ani_stru *pst_param;
    hmac_vap_stru *pst_hmac_vap;

    pst_param = (mac_cfg_query_ani_stru *)puc_param;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_query_ani_rsp: pst_hmac_vap is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap->uc_device_distance = pst_param->uc_device_distance;
    pst_hmac_vap->uc_intf_state_cca = pst_param->uc_intf_state_cca;
    pst_hmac_vap->uc_intf_state_co = pst_param->uc_intf_state_co;

    pst_hmac_vap->station_info_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}

#endif
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX

oal_uint32 hmac_config_stop_altx(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    pst_mac_vap->bit_al_tx_flag = OAL_SWITCH_OFF;

    return OAL_SUCC;
}

#endif


OAL_STATIC oal_uint32 hmac_config_d2h_vap_mib_update(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len,
                                                     oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_d2h_mib_update_info_stru *pst_mib_update_info = OAL_PTR_NULL;
    mac_device_stru *pst_mac_dev = OAL_PTR_NULL;

    if ((pst_mac_vap->uc_init_flag == MAC_VAP_INVAILD) || (pst_mac_vap->pst_mib_info == OAL_PTR_NULL) ||
        (puc_param == OAL_PTR_NULL)) {
        oam_warning_log3(0, OAM_SF_CFG,
                         "{hmac_config_d2h_vap_mib_update::init_flag[%d],mac_vap->pst_mib_info[%p], puc_param[%p]!}",
                         pst_mac_vap->uc_init_flag, (uintptr_t)(pst_mac_vap->pst_mib_info), (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_vap_mib_update::pst_mac_dev[%d] null.}",
                         pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mib_update_info = (mac_d2h_mib_update_info_stru *)puc_param;

    if (pst_mib_update_info->us_beacon_period != 0) {
        mac_mib_set_beacon_period (pst_mac_vap, uc_len, (oal_uint8 *)(&(pst_mib_update_info->us_beacon_period)));
    }
#endif

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH

oal_uint32 hmac_ant_tas_switch_rssi_notify_event_status(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len,
                                                        oal_uint8 *puc_param)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event;
    oal_uint32 ul_ret;
    mac_tas_rssi_notify_stru *puc_payload;
    mac_tas_rssi_notify_stru *pst_rssi_notify = (mac_tas_rssi_notify_stru *)puc_param;

    if ((pst_mac_vap->uc_init_flag == MAC_VAP_INVAILD) || (puc_param == OAL_PTR_NULL)) {
        oam_warning_log2(0, OAM_SF_ANY, "{tas_rssi_notify_event::pst_mac_vap->uc_init_flag[%d], puc_param[%p]!}",
                         pst_mac_vap->uc_init_flag, (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛RSSI测量完成事件到WAL */
    pst_event_mem = frw_event_alloc_m(OAL_SIZEOF(mac_tas_rssi_notify_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{tas_rssi_notify_event::mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    pst_event = frw_get_event_stru(pst_event_mem);
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_TAS_NOTIFY_RSSI,
                       OAL_SIZEOF(mac_tas_rssi_notify_stru),
                       FRW_EVENT_PIPELINE_STAGE_0,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;
    puc_payload = (mac_tas_rssi_notify_stru *)pst_event->auc_event_data;
    puc_payload->l_core_idx = pst_rssi_notify->l_core_idx;
    puc_payload->l_rssi = pst_rssi_notify->l_rssi;

    /* 分发事件 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{tas_rssi_notify_event::dispatch_event fail.}");
    }

    frw_event_free_m(pst_event_mem);

    return ul_ret;
}
#endif
OAL_STATIC oal_uint32 hmac_config_psm_query_rsp_proc(hmac_psm_flt_stat_query_stru *pst_hmac_psm_query,
                                                     mac_psm_query_msg *pst_query_msg)
{
    mac_psm_query_stat_stru  *pst_psm_stat = &pst_hmac_psm_query->ast_psm_stat[pst_query_msg->en_query_type];

    pst_psm_stat->ul_query_item = pst_query_msg->st_stat.ul_query_item;
    if (memcpy_s(pst_psm_stat->aul_val,
                 OAL_SIZEOF(pst_psm_stat->aul_val),
                 pst_query_msg->st_stat.aul_val,
                 (pst_psm_stat->ul_query_item) * OAL_SIZEOF(oal_uint32)) != EOK) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_psm_query_rsp_proc::memcpy fail, query type[%d] query_item[%d]}",
                       pst_query_msg->en_query_type, pst_query_msg->st_stat.ul_query_item);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

oal_uint32 hmac_config_query_psm_flt_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_PSM_STAT, us_len, puc_param);

    return ul_ret;
}


OAL_STATIC oal_uint32  hmac_config_query_psm_rsp(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hmac_device_stru        *pst_hmac_device;
    hmac_psm_flt_stat_query_stru *pst_hmac_psm_query = OAL_PTR_NULL;
    mac_psm_query_msg       *pst_query_rsp_msg = OAL_PTR_NULL;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "hmac_config_query_psm_rsp: pst_hmac_device is null ptr. device id:%d",
            pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_hmac_psm_query = &pst_hmac_device->st_psm_flt_stat_query;
    pst_query_rsp_msg = (mac_psm_query_msg*)puc_param;

    if (hmac_config_psm_query_rsp_proc(pst_hmac_psm_query, pst_query_rsp_msg) == OAL_SUCC) {
        /* 设置wait条件为true */
        pst_hmac_psm_query->auc_complete_flag[pst_query_rsp_msg->en_query_type] = OAL_TRUE;
        oal_wait_queue_wake_up_interrupt(&(pst_hmac_device->st_psm_flt_stat_query.st_wait_queue));
    }

    return OAL_SUCC;
}

/*****************************************************************************
    g_ast_hmac_config_syn: dmac向hmac同步控制面数据处理函数表
*****************************************************************************/
OAL_STATIC OAL_CONST hmac_config_syn_stru g_ast_hmac_config_syn[] = {
    /* 同步ID                    保留2个字节            函数操作 */
    { WLAN_CFGID_QUERY_STATION_STATS, { 0, 0 }, hmac_proc_query_response_event },
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    { WLAN_CFGID_RESET_HW_OPERATE, { 0, 0 }, hmac_reset_sys_event },
    { WLAN_CFGID_THRUPUT_INFO, { 0, 0 }, hmac_get_thruput_info },
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    { WLAN_CFGID_BTCOEX_RX_DELBA_TRIGGER, { 0, 0 }, hmac_btcoex_rx_delba_trigger },
    { WLAN_CFGID_BTCOEX_STATUS_SYN, { 0, 0 }, hmac_config_btcoex_status_syn },
#endif
#endif
    { WLAN_CFGID_QUERY_RSSI,         { 0, 0 }, hmac_config_query_rssi_rsp },
    { WLAN_CFGID_QUERY_RATE,         { 0, 0 }, hmac_config_query_rate_rsp },
    { WLAN_CFGID_QUERY_CHR_EXT_INFO, { 0, 0 }, hmac_config_query_chr_ext_info_rsp_event },
#ifdef _PRE_WLAN_DFT_STAT
    { WLAN_CFGID_QUERY_ANI, { 0, 0 }, hmac_config_query_ani_rsp },
#endif

    { WLAN_CFGID_CHECK_FEM_PA, { 0, 0 }, hmac_atcmdsrv_fem_pa_response },
    { WLAN_CFGID_GET_VERSION,  { 0, 0 }, hmac_atcmdsrv_dbb_num_response },
    { WLAN_CFGID_GET_ANT,      { 0, 0 }, hmac_atcmdsrv_get_ant_response },
    { WLAN_CFGID_RX_FCS_INFO, { 0, 0 }, hmac_atcmdsrv_get_rx_pkcg },
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    { WLAN_CFGID_CHECK_LTE_GPIO, { 0, 0 }, hmac_atcmdsrv_lte_gpio_check },
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    { WLAN_CFGID_REG_INFO, { 0, 0 }, hmac_atcmdsrv_report_efuse_reg },
#endif
#endif
    // {WLAN_CFGID_ADD_VAP,                   {0, 0},         hmac_add_vap_sysnc},
    { WLAN_CFGID_CFG80211_MGMT_TX_STATUS, { 0, 0 }, hmac_mgmt_tx_event_status },
    { WLAN_CFGID_USR_INFO_SYN, { 0, 0 }, hmac_config_d2h_user_info_syn },
#ifdef _PRE_WLAN_FEATURE_11K
    { WLAN_CFGID_REQ_SAVE_BSS_INFO, { 0, 0 }, hmac_scan_rrm_proc_save_bss },
#endif

#ifdef _PRE_WLAN_FEATURE_VOWIFI
    { WLAN_CFGID_VOWIFI_REPORT, { 0, 0 }, hmac_config_vowifi_report },
#endif

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    { HAL_TX_COMP_SUB_TYPE_AL_TX, { 0, 0 }, hmac_config_stop_altx },
#endif
    { WLAN_CFGID_VAP_MIB_UPDATE, { 0, 0 }, hmac_config_d2h_vap_mib_update },
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    { WLAN_CFGID_ANT_TAS_SWITCH_RSSI_NOTIFY, { 0, 0 }, hmac_ant_tas_switch_rssi_notify_event_status },
#endif
#ifdef _PRE_WLAN_FEATURE_NRCOEX
    { WLAN_CFGID_NRCOEX_INFO, { 0, 0 }, hmac_config_nrcoex_info_syn },
#endif
    { WLAN_CFGID_GET_MNGPKT_SENDSTAT, { 0, 0 }, hmac_config_query_sta_mgmt_send_state_rsp },
    { WLAN_CFGID_QUERY_PSM_STAT,      { 0, 0 }, hmac_config_query_psm_rsp},
    { WLAN_CFGID_BUTT, { 0, 0 }, OAL_PTR_NULL },
};


oal_uint32 hmac_event_config_syn(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = OAL_PTR_NULL;
    frw_event_hdr_stru *pst_event_hdr = OAL_PTR_NULL;
    dmac_to_hmac_cfg_msg_stru *pst_dmac2hmac_msg = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_uint16 us_cfgid;

    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_event_config_syn::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件 */
    pst_event = (frw_event_stru *)pst_event_mem->puc_data;
    pst_event_hdr = &(pst_event->st_event_hdr);
    pst_dmac2hmac_msg = (dmac_to_hmac_cfg_msg_stru *)pst_event->auc_event_data;

    /* 获取dmac vap */
    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_event_hdr->uc_vap_id);
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_CFG, "{hmac_event_config_syn::pst_mac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取mac device */
    pst_mac_device = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_CFG, "{hmac_event_config_syn::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得cfg id对应的操作函数 */
    for (us_cfgid = 0; g_ast_hmac_config_syn[us_cfgid].en_cfgid != WLAN_CFGID_BUTT; us_cfgid++) {
        if (g_ast_hmac_config_syn[us_cfgid].en_cfgid == pst_dmac2hmac_msg->en_syn_id) {
            break;
        }
    }

    /* 异常情况，cfgid在g_ast_dmac_config_syn中不存在 */
    if (g_ast_hmac_config_syn[us_cfgid].en_cfgid == WLAN_CFGID_BUTT) {
        OAM_WARNING_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_CFG, "{hmac_event_config_syn::invalid en_cfgid[%d].",
                         pst_dmac2hmac_msg->en_syn_id);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 执行操作函数 */
    ul_ret = g_ast_hmac_config_syn[us_cfgid].p_set_func(pst_mac_vap, (oal_uint8)(pst_dmac2hmac_msg->us_len),
                                                        (oal_uint8 *)pst_dmac2hmac_msg->auc_msg_body);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(pst_event_hdr->uc_vap_id, OAM_SF_CFG,
                         "{hmac_event_config_syn::p_set_func failed, ul_ret=%d en_syn_id=%d.",
                         ul_ret, pst_dmac2hmac_msg->en_syn_id);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_bgscan_enable(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    return hmac_bgscan_enable(pst_mac_vap, us_len, puc_param);
}


oal_uint32 hmac_config_mcs_set_check_enable(mac_vap_stru *pst_mac_vap,
                                            oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_bool_enum_uint8 en_mcs_set_check_enable;

    if (oal_unlikely(puc_param == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_config_mcs_set_check_enable::puc_param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
    en_mcs_set_check_enable = *((oal_bool_enum_uint8 *)puc_param); /* 11n速率集检测使能位 */

    if (en_mcs_set_check_enable) {
        g_ht_mcs_set_check = OAL_TRUE;
    } else {
        g_ht_mcs_set_check = OAL_FALSE;
    }

    OAM_WARNING_LOG1(0, OAM_SF_SCAN, "hmac_config_mcs_set_check_enable: g_ht_mcs_set_check_flag = %d.",
                     g_ht_mcs_set_check);

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_tx_pow_param(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_TX_POW, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_tx_pow_param::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


#ifdef _PRE_WLAN_FEATURE_STA_UAPSD

oal_uint32 hmac_config_set_uapsd_para(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_uapsd_sta_stru *pst_uapsd_param;
    oal_uint32 ul_ret;
    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_PWR, "{hmac_config_set_uapsd_para:: pst_mac_vap/puc_param is null ptr %x, %x!}\r\n",
                       (uintptr_t)pst_mac_vap, (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_uapsd_param = (mac_cfg_uapsd_sta_stru *)puc_param;

    /* uc_max_sp_len值超过6是非法的 */
    if (pst_uapsd_param->uc_max_sp_len > 6) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{hmac_config_set_uapsd_para::uc_max_sp_len[%d] > 6!}\r\n",
                       pst_uapsd_param->uc_max_sp_len);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_STA_PM
    mac_vap_set_uapsd_para(pst_mac_vap, pst_uapsd_param);
#endif

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_UAPSD_PARA, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_uapsd_para::hmac_config_send_event failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_STA_PM

oal_uint32 hmac_config_set_sta_pm_mode(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    return hmac_config_sync_cmd_common(pst_mac_vap, WLAN_CFGID_SET_PS_MODE, us_len, puc_param);
}

oal_uint32 hmac_config_set_sta_pm_on(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    mac_cfg_ps_mode_param_stru st_ps_mode_param;
    hmac_vap_stru *pst_hmac_vap;
    mac_cfg_ps_open_stru *pst_sta_pm_open = (mac_cfg_ps_open_stru *)puc_param;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{hmac_config_set_sta_pm_on::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 切换到手动设置为pspoll模式 */
    if (pst_sta_pm_open->uc_pm_enable == MAC_STA_PM_MANUAL_MODE_ON) {
        pst_hmac_vap->uc_cfg_sta_pm_manual = OAL_TRUE;
    } else if (pst_sta_pm_open->uc_pm_enable == MAC_STA_PM_MANUAL_MODE_OFF) {
        /* 关闭手动设置pspoll模式,回到fastps模式 */
        pst_hmac_vap->uc_cfg_sta_pm_manual = 0xFF;
    }

    pst_sta_pm_open->uc_pm_enable = (pst_sta_pm_open->uc_pm_enable > MAC_STA_PM_SWITCH_OFF) ?
                                    MAC_STA_PM_SWITCH_ON : MAC_STA_PM_SWITCH_OFF;

    st_ps_mode_param.uc_vap_ps_mode = pst_sta_pm_open->uc_pm_enable ?
                                      ((pst_hmac_vap->uc_cfg_sta_pm_manual != 0xFF) ?
                                      MIN_PSPOLL_PS : hmac_config_get_ps_mode()) : NO_POWERSAVE;

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "hmac_config_set_sta_pm_on,enable[%d], ps_mode[%d]",
                     pst_sta_pm_open->uc_pm_enable, st_ps_mode_param.uc_vap_ps_mode);

    /* 先下发设置低功耗模式 */
    ul_ret = hmac_config_set_sta_pm_mode(pst_mac_vap, OAL_SIZEOF(st_ps_mode_param), (oal_uint8 *)&st_ps_mode_param);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "sta_pm sta_pm mode[%d]fail", ul_ret);
        return ul_ret;
    }

    /* 再下发打开低功耗 */
    return hmac_config_sync_cmd_common(pst_mac_vap, WLAN_CFGID_SET_STA_PM_ON, us_len, puc_param);
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32 hmac_get_thruput_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_thruput_info_sync_stru *pst_thruput_info;

    pst_thruput_info = (dmac_thruput_info_sync_stru *)puc_param;

    if (pst_thruput_info != OAL_PTR_NULL) {
        OAL_IO_PRINT("interval cycles: %u \n", pst_thruput_info->ul_cycles);
        OAL_IO_PRINT("sw tx succ num: %u \n", pst_thruput_info->ul_sw_tx_succ_num);
        OAL_IO_PRINT("sw tx fail num: %u \n", pst_thruput_info->ul_sw_tx_fail_num);
        OAL_IO_PRINT("sw rx ampdu succ num: %u \n", pst_thruput_info->ul_sw_rx_ampdu_succ_num);
        OAL_IO_PRINT("sw rx mpdu succ num: %u \n", pst_thruput_info->ul_sw_rx_mpdu_succ_num);
        OAL_IO_PRINT("sw rx fail num: %u \n", pst_thruput_info->ul_sw_rx_ppdu_fail_num);
        OAL_IO_PRINT("hw rx ampdu fcs fail num: %u \n", pst_thruput_info->ul_hw_rx_ampdu_fcs_fail_num);
        OAL_IO_PRINT("hw rx mpdu fcs fail num: %u \n", pst_thruput_info->ul_hw_rx_mpdu_fcs_fail_num);
        return OAL_SUCC;
    } else {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_get_thruput_info::pst_thruput_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
}
#endif
#ifdef _PRE_WLAN_FEATURE_HS20

oal_uint32 hmac_config_set_qos_map(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint8 uc_idx;
    hmac_cfg_qos_map_param_stru *pst_qos_map;
    hmac_vap_stru *pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);

    if ((pst_hmac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY, "hmac_config_set_qos_map:: pointer is null: pst_hmac_vap[%x],puc_param[%x]",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_qos_map = (hmac_cfg_qos_map_param_stru *)puc_param;

    /* 判断QOS MAP SET的使能开关是否打开 */
    if (!pst_qos_map->uc_valid) {
        return OAL_FAIL;
    }

    /* 检查下发的QoS Map Set参数中的DSCP Exception fields 是否超过最大数目21 */
    if (pst_qos_map->uc_num_dscp_except > MAX_DSCP_EXCEPT) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_set_qos_map:: input exceeds maximum : pst_qos_map->num_dscp_except[%d]",
                       pst_qos_map->uc_num_dscp_except);
        return OAL_FAIL;
    }
    /* 判断DSCP Exception fields是否为空 */
    if ((pst_qos_map->uc_num_dscp_except != 0)) {
        pst_hmac_vap->st_cfg_qos_map_param.uc_num_dscp_except = pst_qos_map->uc_num_dscp_except;
        for (uc_idx = 0; uc_idx < pst_qos_map->uc_num_dscp_except; uc_idx++) {
            pst_hmac_vap->st_cfg_qos_map_param.auc_dscp_exception[uc_idx] = pst_qos_map->auc_dscp_exception[uc_idx];
            pst_hmac_vap->st_cfg_qos_map_param.auc_dscp_exception_up[uc_idx] =
                pst_qos_map->auc_dscp_exception_up[uc_idx];
        }
    }

    /* 配置DSCP Exception format中的User Priority的HIGHT和LOW VALUE值 */
    for (uc_idx = 0; uc_idx < MAX_QOS_UP_RANGE; uc_idx++) {
        pst_hmac_vap->st_cfg_qos_map_param.auc_up_high[uc_idx] = pst_qos_map->auc_up_high[uc_idx];
        pst_hmac_vap->st_cfg_qos_map_param.auc_up_low[uc_idx] = pst_qos_map->auc_up_low[uc_idx];
    }
    return OAL_SUCC;
}

oal_uint32 hmac_config_interworking_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    mac_set_customize_interworking(*((oal_uint32 *)puc_param));

    oam_info_log1(0, OAM_SF_CFG, "{hmac_config_interworking_switch::interworking_switch=%d.}",
                  mac_get_customize_interworking());

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_INTERWORKING_SWITCH, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_interworking_switch::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return OAL_SUCC;
}

oal_uint32 hmac_interworking_check(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_param)
{
    oal_uint8 *puc_extend_cap_ie;
    mac_bss_dscr_stru *pst_bss_dscr;

    if ((oal_unlikely(pst_hmac_vap == OAL_PTR_NULL)) || (oal_unlikely(puc_param == OAL_PTR_NULL))) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_interworking_check:: check failed, null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_bss_dscr = (mac_bss_dscr_stru *)puc_param;

    if (pst_bss_dscr->ul_mgmt_len < (MAC_80211_FRAME_LEN + MAC_SSID_OFFSET)) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                         "{hmac_interworking_check:: mgmt_len(%d) < (80211_FRAME_LEN+SSID_OFFSET).}",
                         pst_bss_dscr->ul_mgmt_len);
        return OAL_FAIL;
    }

    /* 查找interworking ie */
    /*lint -e416*/
    puc_extend_cap_ie = mac_find_ie(MAC_EID_EXT_CAPS,
                                    pst_bss_dscr->auc_mgmt_buff + MAC_80211_FRAME_LEN + MAC_SSID_OFFSET,
                                    (oal_int32)(pst_bss_dscr->ul_mgmt_len - MAC_80211_FRAME_LEN - MAC_SSID_OFFSET));
    /*lint +e416*/
    if (puc_extend_cap_ie == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                         "{hmac_interworking_check:: puc_extend_cap_ie is NULL, the ul_mgmt_len is %d.}",
                         pst_bss_dscr->ul_mgmt_len);
        return OAL_FAIL;
    }

    /* 未检测到interworking能力位，返回fail（puc_extend_cap_ie第1byte小于4或者puc_extend_cap_ie第5byte的3bit位为0） */
    if ((puc_extend_cap_ie[1] < 4) || !(puc_extend_cap_ie[5] & 0x80)) {
        pst_hmac_vap->uc_is_interworking = OAL_FALSE;
        return OAL_FAIL;
    }

    pst_hmac_vap->uc_is_interworking = OAL_TRUE;
    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_HS20

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
oal_uint32 hmac_config_filter_11v_bsstreq_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_enable_filter)
{
    oal_uint32 ul_ret;
    oal_uint8 *puc_param = &uc_enable_filter;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_11V_FILTER_SWITCH, OAL_SIZEOF(uc_enable_filter),
                                    puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_filter_11v_bsstreq_switch::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return OAL_SUCC;
}
#endif

oal_uint32 hmac_config_fft_window_offset(mac_vap_stru *pst_mac_vap,
                                         oal_bool_enum_uint8 en_fft_window_offset_enable)
{
    oal_uint32 ul_ret;
    oal_uint8 uc_param = (en_fft_window_offset_enable) ? 3 : 4; /* 判断ftt window offset是否成立，是返回3，否则返回4 */

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_FFT_WINDOW_OFFSET, OAL_SIZEOF(oal_uint8), &uc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_fft_window_offset::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return OAL_SUCC;
}

oal_uint32 hmac_config_wfd_aggr_limit_syn(mac_vap_stru *pst_mac_vap,
                                          oal_bool_enum_uint8 en_wfd_status, oal_bool_enum_uint8 en_aggr_limit_on)
{
    oal_uint32 ul_ret;
    wfd_status_aggr_limit_stru st_wfd_status = { 0 };

    st_wfd_status.en_wfd_status = en_wfd_status;
    st_wfd_status.en_aggr_limit_on = en_aggr_limit_on;

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_WFD_AGGR_LIMIT_SYN,
                                    OAL_SIZEOF(wfd_status_aggr_limit_stru), (oal_uint8 *)&st_wfd_status);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_wfd_aggr_limit::hmac_config_send_event failed[%d].}", ul_ret);
    }
    return ul_ret;
}
#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD


oal_uint32 hmac_config_set_ip_addr(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 配置DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_IP_ADDR, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_set_ip_addr::hmac_config_send_event fail[%d].", ul_ret);
    }

    return ul_ret;
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32 hmac_config_cfg_vap_h2d(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    mac_device_stru *pst_dev = OAL_PTR_NULL;

    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_add_vap::param null,pst_vap=%d puc_param=%d.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_dev == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_config_cfg_vap_h2d::mac_res_get_dev fail. vap_id[%u]}",
                       pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
    抛事件到DMAC层, 创建dmac cfg vap
    ***************************************************************************/
    ul_ret = hmac_cfg_vap_send_event(pst_dev);
    ;
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_cfg_vap_send_event::hmac_config_send_event fail[%d].", ul_ret);
    }

    return ul_ret;
}
#endif
#ifdef _PRE_WLAN_TCP_OPT

oal_uint32 hmac_config_get_tcp_ack_stream_info(mac_vap_stru *pst_mac_vap,
                                               oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_config_get_tcp_ack_stream_info fail: pst_hmac_vap is null}\r\n");
        return OAL_FAIL;
    }

    hmac_tcp_opt_ack_show_count(pst_hmac_vap);
    return OAL_SUCC;
}


oal_uint32 hmac_config_tx_tcp_ack_opt_enable(mac_vap_stru *pst_mac_vap,
                                             oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_val;
    hmac_device_stru *pst_hmac_device;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_config_tx_tcp_ack_opt_enable fail: pst_hmac_device is null}\r\n");
        return OAL_FAIL;
    }

    ul_val = *((oal_uint32 *)puc_param);

    if (ul_val == 0) {
        pst_hmac_device->sys_tcp_tx_ack_opt_enable = OAL_FALSE;
    } else {
        pst_hmac_device->sys_tcp_tx_ack_opt_enable = OAL_TRUE;
    }
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_config_tx_tcp_ack_opt_enable:sys_tcp_tx_ack_opt_enable = %d}\r\n",
                     pst_hmac_device->sys_tcp_tx_ack_opt_enable);
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ

oal_uint32 hmac_set_device_freq_mode(oal_uint8 uc_device_enable)
{
    oal_uint32 ul_ret;
    config_device_freq_h2d_stru st_device_freq_type;
    mac_vap_stru *pst_mac_vap;
    oal_uint8 uc_index;
    device_speed_freq_level_stru *pst_device_speed_freq_level = hmac_wifi_get_device_speed_freq_level_addr();
    host_speed_freq_level_stru *pst_host_speed_freq_level = hmac_wifi_get_host_speed_freq_level_addr();

    /* 设置Device 调频使能控制 */
    pst_mac_vap = mac_res_get_mac_vap(0);
    if (pst_mac_vap != OAL_PTR_NULL) {
        for (uc_index = 0; uc_index < FREQ_BUTT; uc_index++) {
            st_device_freq_type.st_device_data[uc_index].ul_speed_level =
                pst_host_speed_freq_level[uc_index].ul_speed_level;
            st_device_freq_type.st_device_data[uc_index].ul_cpu_freq_level =
                pst_device_speed_freq_level[uc_index].uc_device_type;
        }

        st_device_freq_type.uc_device_freq_enable = uc_device_enable;
        st_device_freq_type.uc_set_type = FREQ_SET_MODE;

        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_set_device_freq_mode: enable mode[%d][1:enable,0:disable].}",
                         st_device_freq_type.uc_device_freq_enable);

        /***************************************************************************
            抛事件到DMAC层, 同步VAP最新状态到DMAC
        ***************************************************************************/
        ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_DEVICE_FREQ,
                                        OAL_SIZEOF (config_device_freq_h2d_stru), (oal_uint8 *)(&st_device_freq_type));
        if (oal_unlikely(ul_ret != OAL_SUCC)) {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_set_device_freq_mode::hmac_set_device_freq failed[%d].}", ul_ret);
        }
    } else {
        ul_ret = OAL_ERR_CODE_PTR_NULL;
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_device_freq(oal_uint8 uc_device_freq_type)
{
    oal_uint32 ul_ret;
    config_device_freq_h2d_stru st_device_freq_type;
    mac_vap_stru *pst_mac_vap;

    pst_mac_vap = mac_res_get_mac_vap(0);
    if (pst_mac_vap != OAL_PTR_NULL) {
        st_device_freq_type.uc_set_freq = uc_device_freq_type;
        st_device_freq_type.uc_set_type = FREQ_SET_FREQ;

        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_set_device_freq: device freq level[%d].}", uc_device_freq_type);

        /***************************************************************************
            抛事件到DMAC层, 同步VAP最新状态到DMAC
        ***************************************************************************/
        ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_DEVICE_FREQ,
                                        OAL_SIZEOF (config_device_freq_h2d_stru), (oal_uint8 *)(&st_device_freq_type));
        if (oal_unlikely(ul_ret != OAL_SUCC)) {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_set_device_freq::hmac_set_device_freq failed[%d].}", ul_ret);
        }
    } else {
        ul_ret = OAL_ERR_CODE_PTR_NULL;
    }

    return ul_ret;
}


oal_uint32 hmac_config_get_device_freq(oal_void)
{
    oal_uint32 ul_ret;
    config_device_freq_h2d_stru st_device_freq_type;
    mac_vap_stru *pst_mac_vap;

    pst_mac_vap = mac_res_get_mac_vap(0);
    if (pst_mac_vap != OAL_PTR_NULL) {
        st_device_freq_type.uc_set_type = FREQ_GET_FREQ;

        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_get_device_freq!].}");

        /***************************************************************************
            抛事件到DMAC层, 同步VAP最新状态到DMAC
        ***************************************************************************/
        ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_DEVICE_FREQ,
                                        OAL_SIZEOF (config_device_freq_h2d_stru), (oal_uint8 *)(&st_device_freq_type));
        if (oal_unlikely(ul_ret != OAL_SUCC)) {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_set_device_freq::hmac_set_device_freq failed[%d].}", ul_ret);
        }
    } else {
        ul_ret = OAL_ERR_CODE_PTR_NULL;
    }

    return ul_ret;
}

#endif

oal_uint32 hmac_config_rx_tcp_ack_opt_enable(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_val;
    hmac_device_stru *pst_hmac_device;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_config_rx_tcp_ack_opt_enable fail: pst_hmac_device is null}\r\n");
        return OAL_FAIL;
    }

    ul_val = *((oal_uint32 *)puc_param);

    if (ul_val == 0) {
        pst_hmac_device->sys_tcp_rx_ack_opt_enable = OAL_FALSE;
    } else {
        pst_hmac_device->sys_tcp_rx_ack_opt_enable = OAL_TRUE;
    }
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_config_rx_tcp_ack_opt_enable:sys_tcp_tx_ack_opt_enable = %d}\r\n",
                     pst_hmac_device->sys_tcp_rx_ack_opt_enable);
    return OAL_SUCC;
}

oal_uint32 hmac_config_tx_tcp_ack_limit(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_val;
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_config_tx_tcp_ack_limit fail: pst_hmac_vap is null}\r\n");
        return OAL_FAIL;
    }

    ul_val = *((oal_uint32 *)puc_param);

    if (ul_val >= DEFAULT_TX_TCP_ACK_THRESHOLD) {
        pst_hmac_vap->ast_hmac_tcp_ack[HCC_TX].filter_info.ul_ack_limit = DEFAULT_TX_TCP_ACK_THRESHOLD;
    } else {
        pst_hmac_vap->ast_hmac_tcp_ack[HCC_TX].filter_info.ul_ack_limit = ul_val;
    }
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_config_tx_tcp_ack_limit:ul_ack_limit = %ld}\r\n",
                     pst_hmac_vap->ast_hmac_tcp_ack[HCC_TX].filter_info.ul_ack_limit);
    return OAL_SUCC;
}

oal_uint32 hmac_config_rx_tcp_ack_limit(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_val;
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_config_rx_tcp_ack_limit fail: pst_hmac_vap is null}\r\n");
        return OAL_FAIL;
    }

    ul_val = *((oal_uint32 *)puc_param);

    if (ul_val >= DEFAULT_RX_TCP_ACK_THRESHOLD) {
        pst_hmac_vap->ast_hmac_tcp_ack[HCC_RX].filter_info.ul_ack_limit = DEFAULT_RX_TCP_ACK_THRESHOLD;
    } else {
        pst_hmac_vap->ast_hmac_tcp_ack[HCC_RX].filter_info.ul_ack_limit = ul_val;
    }
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_config_rx_tcp_ack_limit:ul_ack_limit = %ld}\r\n",
                     pst_hmac_vap->ast_hmac_tcp_ack[HCC_RX].filter_info.ul_ack_limit);
    return OAL_SUCC;
}

#endif

oal_void hmac_config_del_p2p_ie(oal_uint8 *puc_ie, oal_uint32 *pul_ie_len)
{
    oal_uint8 *puc_p2p_ie = OAL_PTR_NULL;
    oal_uint32 ul_p2p_ie_len;
    oal_uint8 *puc_ie_end = OAL_PTR_NULL;
    oal_uint8 *puc_p2p_ie_end = OAL_PTR_NULL;

    if ((puc_ie == OAL_PTR_NULL) || (pul_ie_len == OAL_PTR_NULL) || (*pul_ie_len == 0)) {
        return;
    }

    puc_p2p_ie = mac_find_vendor_ie(MAC_WLAN_OUI_WFA, MAC_WLAN_OUI_TYPE_WFA_P2P, puc_ie, (oal_int32)(*pul_ie_len));
    if ((puc_p2p_ie == OAL_PTR_NULL) || (puc_p2p_ie[1] < MAC_P2P_MIN_IE_LEN)) {
        return;
    }

    ul_p2p_ie_len = puc_p2p_ie[1] + MAC_IE_HDR_LEN;

    /* 将p2p ie 后面的内容拷贝到p2p ie 所在位置 */
    puc_ie_end = (puc_ie + *pul_ie_len);
    puc_p2p_ie_end = (puc_p2p_ie + ul_p2p_ie_len);

    if (puc_ie_end >= puc_p2p_ie_end) {
        if (memmove_s(puc_p2p_ie, *pul_ie_len, puc_p2p_ie_end, (oal_uint32)(puc_ie_end - puc_p2p_ie_end)) != EOK) {
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "hmac_config_del_p2p_ie::memmove fail!");
            return;
        }
        *pul_ie_len -= ul_p2p_ie_len;
    }
    return;
}
#ifdef _PRE_WLAN_FEATURE_ROAM

oal_uint32 hmac_config_roam_enable(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    oal_uint8 uc_enable;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_roam_enable::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_enable = (*puc_param == 0) ? 0 : 1;

    return hmac_roam_enable(pst_hmac_vap, uc_enable);
}


oal_uint32 hmac_config_roam_band(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    oal_uint8 uc_band;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_roam_band::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_band = *puc_param;

    return hmac_roam_band(pst_hmac_vap, uc_band);
}


oal_uint32 hmac_config_roam_org(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    oal_uint8 uc_scan_orthogonal;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_roam_band::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_scan_orthogonal = *puc_param;

    return hmac_roam_org(pst_hmac_vap, uc_scan_orthogonal);
}


oal_uint32 hmac_config_roam_start(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    mac_cfg_set_roam_start_stru *pst_roam_start;
    hmac_roam_info_stru *pst_roam_info;

    if ((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG, "hmac_config_roam_start:pst_mac_vap=%x,puc_param=%x",
                       (uintptr_t)pst_mac_vap, (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_roam_enable::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_roam_start = (mac_cfg_set_roam_start_stru *)(puc_param);
    if (ether_is_all_zero(pst_roam_start->auc_bssid)) {
        /* reassociation or roaming */
        return hmac_roam_start(pst_hmac_vap, pst_roam_start->en_scan_type, pst_roam_start->en_current_bss_ignore,
                               ROAM_TRIGGER_APP);
    } else if (oal_memcmp(pst_mac_vap->auc_bssid, pst_roam_start->auc_bssid, OAL_MAC_ADDR_LEN) == 0) {
        /* reassociation */
        return hmac_roam_start(pst_hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_FALSE, ROAM_TRIGGER_APP);
    } else {
        /* roaming for specified BSSID */
        pst_roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
        if (pst_roam_info == OAL_PTR_NULL) {
            return OAL_ERR_CODE_PTR_NULL;
        }
        oal_set_mac_addr(pst_roam_info->auc_target_bssid, pst_roam_start->auc_bssid);
        return hmac_roam_start(pst_hmac_vap, ROAM_SCAN_CHANNEL_ORG_BUTT, OAL_TRUE, ROAM_TRIGGER_BSSID);
    }
}


oal_uint32 hmac_config_roam_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_roam_enable::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    return hmac_roam_show(pst_hmac_vap);
}
#endif  // _PRE_WLAN_FEATURE_ROAM

#ifdef _PRE_WLAN_FEATURE_11R

oal_uint32 hmac_config_set_ft_ies(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    mac_cfg80211_ft_ies_stru *pst_mac_ft_ies;
    oal_app_ie_stru st_ft_ie;
    oal_uint32 ul_ret;
    oal_uint16 us_md_id;

    if ((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_config_set_ft_ies::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_ft_ies::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (pst_hmac_vap->bit_11r_enable != OAL_TRUE) {
        return OAL_SUCC;
    }

    pst_mac_ft_ies = (mac_cfg80211_ft_ies_stru *)puc_param;
    ul_ret = mac_mib_get_md_id(pst_mac_vap, &us_md_id);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_ft_ies::get_md_id fail[%d].}", ul_ret);
        return ul_ret;
    }

    if (us_md_id != pst_mac_ft_ies->us_mdid) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_set_ft_ies::UNEXPECTED mdid[%d/%d].}", pst_mac_ft_ies->us_mdid, us_md_id);
        return OAL_FAIL;
    }

    st_ft_ie.en_app_ie_type = OAL_APP_FT_IE;
    st_ft_ie.ul_ie_len = pst_mac_ft_ies->us_len;
    if (memcpy_s(st_ft_ie.auc_ie, WLAN_WPS_IE_MAX_SIZE, pst_mac_ft_ies->auc_ie, pst_mac_ft_ies->us_len) != EOK) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_set_ft_ies::memcpy fail!");
        return OAL_FAIL;
    }

    ul_ret = hmac_config_set_app_ie_to_vap(pst_mac_vap, &st_ft_ie, OAL_APP_FT_IE);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_ft_ies::set_app_ie FAIL[%d].}", ul_ret);
        return ul_ret;
    }

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_ft_ies::set_app_ie OK LEN [%d].}",
                     pst_mac_ft_ies->us_len);

    hmac_roam_reassoc(pst_hmac_vap);

    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_11R

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
oal_uint32 hmac_config_set_device_freq_enable(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_set_auto_freq_stru *pst_set_h2d_freq = (mac_cfg_set_auto_freq_stru *)puc_param;

    if (pst_set_h2d_freq->uc_cmd_type == CMD_SET_DEVICE_FREQ_ENDABLE) {
        if (pst_set_h2d_freq->uc_value == H2D_FREQ_MODE_ENABLE) {
            hmac_set_device_freq_mode(H2D_FREQ_MODE_ENABLE);
        } else {
            hmac_set_device_freq_mode(H2D_FREQ_MODE_DISABLE);
        }
    } else if (pst_set_h2d_freq->uc_cmd_type == CMD_SET_DEVICE_FREQ_VALUE) {
        if (pst_set_h2d_freq->uc_value == FREQ_IDLE) {
            hmac_config_set_device_freq(FREQ_IDLE);
        } else if (pst_set_h2d_freq->uc_value == FREQ_MIDIUM) {
            hmac_config_set_device_freq(FREQ_MIDIUM);
        } else if (pst_set_h2d_freq->uc_value == FREQ_HIGHER) {
            hmac_config_set_device_freq(FREQ_HIGHER);
        } else if (pst_set_h2d_freq->uc_value == FREQ_HIGHEST) {
            hmac_config_set_device_freq(FREQ_HIGHEST);
        }
    } else if (pst_set_h2d_freq->uc_cmd_type == CMD_GET_DEVICE_AUTO_FREQ) {
        hmac_config_get_device_freq();
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_set_device_freq:parameter error!}\r\n");
    }
    oam_warning_log2(0, OAM_SF_ANY, "{hmac_config_set_device_freq_enable::uc_cmd_type = %d, uc_value = %d}\r\n",
                     pst_set_h2d_freq->uc_cmd_type, pst_set_h2d_freq->uc_value);
    return OAL_SUCC;
}


oal_uint32 hmac_config_set_device_freq_value(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint8 uc_sync_data;
    oal_uint32 ul_ret;
    config_device_freq_h2d_stru st_device_freq_data;
    oal_uint8 uc_index;
    device_speed_freq_level_stru *pst_device_speed_freq_level = hmac_wifi_get_device_speed_freq_level_addr();
    host_speed_freq_level_stru *pst_host_speed_freq_level = hmac_wifi_get_host_speed_freq_level_addr();

    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_config_set_device_freq_value: mac vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_sync_data = *puc_param;
    if (uc_sync_data == OAL_TRUE) {
        for (uc_index = 0; uc_index < 4; uc_index++) { /* st_device_data数组长度是4 */
            st_device_freq_data.st_device_data[uc_index].ul_speed_level =
                pst_host_speed_freq_level[uc_index].ul_speed_level;
            st_device_freq_data.st_device_data[uc_index].ul_cpu_freq_level =
                pst_device_speed_freq_level[uc_index].uc_device_type;
        }
        st_device_freq_data.uc_set_type = FREQ_SYNC_DATA;

        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_device_freq_value: set value succ.}");

        /***************************************************************************
            抛事件到DMAC层, 同步VAP最新状态到DMAC
        ***************************************************************************/
        ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_DEVICE_FREQ,
                                        OAL_SIZEOF (config_device_freq_h2d_stru), (oal_uint8 *)(&st_device_freq_data));
        if (oal_unlikely(ul_ret != OAL_SUCC)) {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_set_device_freq_mode::hmac_set_device_freq failed[%d].}", ul_ret);
        }
    }
    return OAL_SUCC;
}

#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE


OAL_STATIC oal_uint32 hmac_config_lauch_cap_show(mac_regclass_info_stru *pst_regclass_info, oal_uint8 channel_freq,
                                                 oal_bool_enum_uint8 en_edge_flag)
{
    /* 根据国家码输出当前支持中心频点各模式、各速率的天线口发射功率以及边带的天线口发射功率 */
    switch (channel_freq) {
        case MAC_RC_START_FREQ_2:
            break;

        /* 由于定制化参数数组g_auc_nv_params[NUM_OF_NV_PARAMS]从第46位开始存放5G功率数值，修改时，需要对应修改此处起始值 */
        case MAC_RC_START_FREQ_5:
            break;
        default:
            OAM_ERROR_LOG1(0, OAM_SF_CFG, "{hmac_config_lauch_cap_print: channel_freq type [%d] invalid!}",
                           channel_freq);
            return OAL_FAIL;
    }
    return OAL_SUCC;
}


oal_uint32 hmac_config_get_lauch_cap(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    oal_uint8 uc_chan_num;
    oal_uint8 uc_chan_2gup_idx = 0;
    oal_uint8 uc_chan_2gdown_idx = 0;
    oal_uint8 uc_chan_5gup_idx = 0;
    oal_uint8 uc_chan_5gdown_idx = 0;
    oal_uint8 uc_idx;
    oal_uint32 ul_ret = OAL_FAIL;
    mac_regclass_info_stru *pst_regclass_info = OAL_PTR_NULL;

    /* 入参检查 */
    if ((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL)) {
        return OAL_FAIL;
    }

    /* 查找2.4G信道边带 */
    for (uc_idx = 0; uc_idx < MAC_CHANNEL_FREQ_2_BUTT / 2; uc_idx++) { /* 除以2表示从0-6信道索引值查找 */
        if ((mac_is_channel_idx_valid(MAC_RC_START_FREQ_2, uc_idx)) == OAL_SUCC) {
            uc_chan_2gup_idx = uc_idx;
            break;
        }
    }
    /* 除以2表示从7-14信道索引值查找 */
    for (uc_idx = MAC_CHANNEL_FREQ_2_BUTT / 2; uc_idx < MAC_CHANNEL_FREQ_2_BUTT; uc_idx++) {
        if ((mac_is_channel_idx_valid(MAC_RC_START_FREQ_2, uc_idx)) == OAL_SUCC) {
            uc_chan_2gdown_idx = uc_idx;
        }
    }
    /* 查找5G信道边带 */
    for (uc_idx = 0; uc_idx < MAC_CHANNEL_FREQ_5_BUTT / 2; uc_idx++) { /* 除以2表示从0-14信道索引值查找 */
        if ((mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, uc_idx)) == OAL_SUCC) {
            uc_chan_5gup_idx = uc_idx;
            break;
        }
    }
    /* 除以2表示从14-29信道索引值查找 */
    for (uc_idx = MAC_CHANNEL_FREQ_5_BUTT / 2; uc_idx < MAC_CHANNEL_FREQ_5_BUTT; uc_idx++) {
        if ((mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, uc_idx)) == OAL_SUCC) {
            uc_chan_5gdown_idx = uc_idx;
        }
    }
    /* 输出2.4G信道下支持的天线口发射功率以及边带的天线口发射功率 */
    for (uc_idx = 0; uc_idx < MAC_CHANNEL_FREQ_2_BUTT; uc_idx++) {
        if ((mac_is_channel_idx_valid(MAC_RC_START_FREQ_2, uc_idx)) == OAL_SUCC) {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_2, uc_idx, &uc_chan_num);
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_lauch_cap:channel=%d}", uc_chan_num);
            pst_regclass_info = mac_get_channel_num_rc_info(MAC_RC_START_FREQ_2, uc_chan_num);
            ul_ret = hmac_config_lauch_cap_show(pst_regclass_info, MAC_RC_START_FREQ_2,
                                                (uc_idx == uc_chan_2gup_idx) || (uc_idx == uc_chan_2gdown_idx));
            if (ul_ret != OAL_SUCC) {
                return OAL_FAIL;
            }
        }
    }

    /* 检查定制化5g开关是否使能 */
    if (mac_get_band_5g_enabled() == OAL_FALSE) {
        return ul_ret;
    }

    /* 输出5G信道下支持的天线口发射功率以及边带的天线口发射功率 */
    for (uc_idx = 0; uc_idx < MAC_CHANNEL_FREQ_5_BUTT; uc_idx++) {
        if ((mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, uc_idx)) == OAL_SUCC) {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_5, uc_idx, &uc_chan_num);
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_lauch_cap:channel=%d}", uc_chan_num);
            pst_regclass_info = mac_get_channel_num_rc_info(MAC_RC_START_FREQ_5, uc_chan_num);
            ul_ret = hmac_config_lauch_cap_show(pst_regclass_info, MAC_RC_START_FREQ_5,
                                                (uc_idx == uc_chan_5gup_idx) || (uc_idx == uc_chan_5gdown_idx));
            if (ul_ret != OAL_SUCC) {
                return OAL_FAIL;
            }
        }
    }
    return OAL_SUCC;
}


oal_uint32 hmac_config_set_linkloss_threshold(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_LINKLOSS_THRESHOLD, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_linkloss_threshold::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}

oal_uint32 hmac_config_set_all_log_level(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret = 0;
    oal_uint8 uc_vap_idx;
    oal_uint8 uc_level;

    if ((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG,
                       "{hmac_config_set_all_log_level:: pointer is null,pst_mac_vap[0x%x], puc_param[0x%x] .}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_level = (oal_uint8)(*puc_param);
    for (uc_vap_idx = 0; uc_vap_idx < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_vap_idx++) {
        ul_ret += oam_log_set_vap_level(uc_vap_idx, uc_level);
        if (ul_ret != OAL_SUCC) {
            return ul_ret;
        }
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_ALL_LOG_LEVEL, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_all_log_level::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}

oal_uint32 hmac_config_set_d2h_hcc_assemble_cnt(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_D2H_HCC_ASSEMBLE_CNT, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_d2h_hcc_assemble_cnt::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}

oal_uint32 hmac_config_set_cus_rf(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_CUS_RF, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_rf::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}

#if (_PRE_PRODUCT_ID_HI1102A_HOST == _PRE_PRODUCT_ID)

OAL_STATIC oal_int32 hmac_config_init_hcc_flowctrl(oal_uint8 uc_hcc_flowctrl_type)
{
    if (hcc_bus_flowctrl_init(uc_hcc_flowctrl_type) != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG,
                       "{hmac_config_init_hcc_flowctrl::Flowctrl init failed! type = %d}", uc_hcc_flowctrl_type);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}
#endif


oal_uint32 hmac_config_set_cus_priv(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

#if (_PRE_PRODUCT_ID_HI1102A_HOST == _PRE_PRODUCT_ID)
    wlan_cfg_customize_priv *pst_cus_priv = (wlan_cfg_customize_priv *)puc_param;

    if (hmac_config_init_hcc_flowctrl(pst_cus_priv->uc_hcc_flowctrl_type) != OAL_SUCC) {
        /* GPIO流控中断注册失败，强制device使用SDIO流控(type = 0) */
        pst_cus_priv->uc_hcc_flowctrl_type = 0;
    }
#endif

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_CUS_PRIV, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_priv::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_cus_dts_cali(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_CUS_DTS_CALI, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_dts_cali::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}

oal_uint32 hmac_config_set_cus_nvram_params(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_CUS_NVRAM_PARAM, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_nvram_params::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}

oal_uint32 hmac_config_dev_customize_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SHOW_DEV_CUSTOMIZE_INFOS, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_dev_customize_info::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_5g_high_band_max_pow_params(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                       oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_5G_HIGH_BAND_MAX_POW, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_base_power_params::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_cus_base_power_params(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                 oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_CUS_BASE_POWER, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_base_power_params::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_cus_fcc_ce_power_params(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                   oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_CUS_FCC_CE_POWER, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_fcc_ce_power_params::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}
oal_uint32 hmac_config_set_ext_fcc_ce_power_params(mac_vap_stru *pst_mac_vap,
                                                   oal_uint16 us_len,
                                                   oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_EXT_FCC_CE_POWER, us_len, puc_param);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_ext_fcc_ce_power_params:: \
                         hmac_config_send_event failed[%d].}",
                         ul_ret);
    }
    return ul_ret;
}

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI

oal_uint32 hmac_config_set_cus_dyn_cali(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_CUS_DYN_CALI_PARAM, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_dyn_cali::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}
#endif

#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#ifdef _PRE_WLAN_FEATURE_11K

oal_uint32 hmac_config_send_neighbor_req(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret = OAL_SUCC;
    oal_netbuf_stru *pst_action_neighbor_req;
    oal_uint16 us_neighbor_req_frm_len;
    mac_tx_ctl_stru *pst_tx_ctl;
    oal_uint16 us_index;
    oal_uint8 *puc_data = OAL_PTR_NULL;
    mac_user_stru *pst_mac_user;
    mac_cfg_ssid_param_stru *pst_ssid = (mac_cfg_ssid_param_stru *)puc_param;

    pst_action_neighbor_req = (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2,
                                                                      OAL_NETBUF_PRIORITY_MID);
    if (pst_action_neighbor_req == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_config_send_neighbor_req::neighbor_req null.}");
        return ul_ret;
    }

    memset_s(oal_netbuf_cb(pst_action_neighbor_req), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    puc_data = (oal_uint8 *)oal_netbuf_header(pst_action_neighbor_req);

    /*************************************************************************/
    /* Management Frame Format */
    /* -------------------------------------------------------------------- */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS| */
    /* -------------------------------------------------------------------- */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  | */
    /* -------------------------------------------------------------------- */
    /*************************************************************************/
    /*************************************************************************/
    /* Set the fields in the frame header */
    /*************************************************************************/
    /* All the fields of the Frame Control Field are set to zero. Only the */
    /* Type/Subtype field is set. */
    mac_hdr_set_frame_control(puc_data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* duration */
    puc_data[2] = 0; /* puc_data 2、3byte(duration)置零 */
    puc_data[3] = 0;

    pst_mac_user = mac_res_get_mac_user(pst_mac_vap->uc_assoc_vap_id);
    if (pst_mac_user == OAL_PTR_NULL) {
        oal_netbuf_free(pst_action_neighbor_req);
        OAM_WARNING_LOG1(0, OAM_SF_TX, "{hmac_config_send_neighbor_req::user[%d] null.", pst_mac_vap->uc_assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* DA is address of STA requesting association(从puc_data第4byte开始是DA) */
    oal_set_mac_addr(puc_data + 4, pst_mac_user->auc_user_mac_addr);

    /* SA is the dot11MACAddress(从puc_data第10byte开始是SA) */
    oal_set_mac_addr(puc_data + 10, pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);

    oal_set_mac_addr(puc_data + 16, pst_mac_vap->auc_bssid); /* 从puc_data第16byte开始是BSSID */

    /* seq control */
    puc_data[22] = 0; /* puc_data 22、23byte(seq control)置零 */
    puc_data[23] = 0;

    /*************************************************************************/
    /* Set the contents of the frame body */
    /*************************************************************************/
    /*************************************************************************/
    /* Neighbor report request Frame - Frame Body */
    /* ------------------------------------------------- */
    /* | Category | Action |  Dialog Token | Opt SubEle | */
    /* ------------------------------------------------- */
    /* | 1        | 1      |       1       | Var        | */
    /* ------------------------------------------------- */
    /*************************************************************************/
    /* Initialize index and the frame data pointer */
    us_index = MAC_80211_FRAME_LEN;

    /* Category */
    puc_data[us_index++] = MAC_ACTION_CATEGORY_RADIO_MEASURMENT;

    /* Action */
    puc_data[us_index++] = MAC_RM_ACTION_NEIGHBOR_REPORT_REQUEST;

    /* Dialog Token */
    puc_data[us_index++] = 1;

    if (pst_ssid->uc_ssid_len != 0) {
        /* Subelement ID */
        puc_data[us_index++] = 0;

        /* length */
        puc_data[us_index++] = pst_ssid->uc_ssid_len;

        /* SSID */
        if (memcpy_s(puc_data + us_index, pst_ssid->uc_ssid_len, pst_ssid->ac_ssid, pst_ssid->uc_ssid_len) != EOK) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_TX, "{hmac_config_send_neighbor_req::memcpy fail!}");
            oal_netbuf_free(pst_action_neighbor_req);
            return OAL_FAIL;
        }
        us_index += pst_ssid->uc_ssid_len;
    }

    us_neighbor_req_frm_len = us_index;

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_action_neighbor_req);
    pst_tx_ctl->us_mpdu_len = us_neighbor_req_frm_len;
    pst_tx_ctl->us_tx_user_idx = 0xffff; /* 发送完成需要获取user结构体 */

    oal_netbuf_put(pst_action_neighbor_req, us_neighbor_req_frm_len);

    ul_ret = hmac_tx_mgmt_send_event(pst_mac_vap, pst_action_neighbor_req, us_neighbor_req_frm_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(pst_action_neighbor_req);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}

oal_uint32 hmac_config_bcn_table_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_BCN_TABLE_SWITCH, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_bcn_table_switch::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}
#endif  // _PRE_WLAN_FEATURE_11K


oal_uint32 hmac_config_voe_enable(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_bool_enum_uint8 en_read_flag;
    mac_device_voe_custom_stru *pst_voe_custom_param = mac_get_voe_custom_param_addr();

    oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_voe_enable::bit[4]11r_auth,bit[3]11k_auth_flag,bit[2]11k,bit[1]11v,bit[0]11r}");
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_get_rx_pkcg::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 显示voe定制化信息和当前信息 */
    en_read_flag = (((*puc_param) & 0xFF) & BIT7) ? OAL_TRUE : OAL_FALSE;
    if (en_read_flag == OAL_TRUE) {
        oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{voe_enable::11k=[%d]11v=[%d]11r=[%d]}",
                         pst_voe_custom_param->en_11k,
                         pst_voe_custom_param->en_11v, pst_voe_custom_param->en_11r);
        return OAL_SUCC;
    } else {
#ifdef _PRE_WLAN_FEATURE_11K
        pst_hmac_vap->bit_11k_auth_flag = (((*puc_param) & 0x0F) & BIT3) ? OAL_TRUE : OAL_FALSE;
        pst_hmac_vap->bit_11k_enable = (((*puc_param) & 0x0F) & BIT2) ? OAL_TRUE : OAL_FALSE;
        pst_hmac_vap->bit_11v_enable = (((*puc_param) & 0x0F) & BIT1) ? OAL_TRUE : OAL_FALSE;
        pst_hmac_vap->bit_11k_auth_oper_class = (((*puc_param) >> 5) & 0x3); /* 保留puc_param的5-6字节为auth_oper_class */
#endif
#ifdef _PRE_WLAN_FEATURE_11R
        pst_hmac_vap->bit_11r_enable = (((*puc_param) & 0x0F) & BIT0) ? OAL_TRUE : OAL_FALSE;
        pst_hmac_vap->bit_voe_11r_auth = (((*puc_param) & 0xFF) & BIT4) ? OAL_TRUE : OAL_FALSE;
        pst_hmac_vap->bit_11r_over_ds = (((puc_param[1]) & 0xFF) & BIT0) ? OAL_TRUE : OAL_FALSE;
#endif
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11R)
        oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_voe_enable::11k=[%d],11v=[%d],11r=[%d].}",
                         pst_hmac_vap->bit_11k_enable, pst_hmac_vap->bit_11v_enable,
                         pst_hmac_vap->bit_11r_enable);
#endif
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11R)
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_voe_enable::bit_voe_11r_auth = [%d],bit_11k_auth_oper_class=[%d].}",
                         pst_hmac_vap->bit_voe_11r_auth, pst_hmac_vap->bit_11k_auth_oper_class);
#endif
    }
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_VOE_ENABLE, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_voe_enable::send_event failed[%d]}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_vendor_cmd_get_channel_list(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len,
                                                   oal_uint8 *puc_param)
{
    oal_uint8 uc_chan_idx;
    oal_uint8 uc_chan_num;
    oal_uint8 uc_chan_number;
    oal_uint8 *puc_channel_list = OAL_PTR_NULL;
    mac_vendor_cmd_channel_list_stru *pst_channel_list = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    if ((pus_len == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL)) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_config_vendor_cmd_get_channel_list::len or param is NULL.len %p, param %p}",
                         (uintptr_t)pus_len, (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_channel_list = (mac_vendor_cmd_channel_list_stru *)puc_param;
    *pus_len = OAL_SIZEOF(mac_vendor_cmd_channel_list_stru);

    /* 获取2G 信道列表 */
    uc_chan_num = 0;
    puc_channel_list = pst_channel_list->auc_channel_list_2g;

    for (uc_chan_idx = 0; uc_chan_idx < MAC_CHANNEL_FREQ_2_BUTT; uc_chan_idx++) {
        ul_ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_2, uc_chan_idx);
        if (ul_ret == OAL_SUCC) {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_2, uc_chan_idx, &uc_chan_number);
            puc_channel_list[uc_chan_num++] = uc_chan_number;
        }
    }
    pst_channel_list->uc_channel_num_2g = uc_chan_num;

    /* 检查定制化5g开关是否使能 */
    if (mac_get_band_5g_enabled() == OAL_FALSE) {
        pst_channel_list->uc_channel_num_5g = 0;
        return OAL_SUCC;
    }

    /* 获取5G 信道列表 */
    uc_chan_num = 0;
    puc_channel_list = pst_channel_list->auc_channel_list_5g;

    for (uc_chan_idx = 0; uc_chan_idx < MAC_CHANNEL_FREQ_5_BUTT; uc_chan_idx++) {
        ul_ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, uc_chan_idx);
        if (ul_ret == OAL_SUCC) {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_5, uc_chan_idx, &uc_chan_number);
            puc_channel_list[uc_chan_num++] = uc_chan_number;
        }
    }
    pst_channel_list->uc_channel_num_5g = uc_chan_num;

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_CSI_RAM

oal_uint32 hmac_config_set_csi(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_CSI, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_csi::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32 hmac_config_pm_debug_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PM_DEBUG_SWITCH, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_pm_debug_switch::hmac_config_send_event fail[%d].", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI

oal_uint32 hmac_config_dyn_cali_param(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DYN_CALI_CFG, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_dyn_cali_param::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}
#endif

#ifdef _PRE_WLAN_FEATURE_APF

oal_void hmac_set_apf_switch_by_pps(hmac_device_stru *pst_hmac_device, mac_cfg_suspend_stru *pst_suspend)
{
    oal_uint32 ul_total_rxtx_pps;

    if (pst_hmac_device == OAL_PTR_NULL) {
        return;
    }

    ul_total_rxtx_pps = pst_hmac_device->st_dscr_th_opt.ul_tx_large_pps +
                        pst_hmac_device->st_dscr_th_opt.ul_tx_small_pps +
                        pst_hmac_device->st_dscr_th_opt.ul_rx_large_pps +
                        pst_hmac_device->st_dscr_th_opt.ul_rx_small_pps;

    if ((ul_total_rxtx_pps < g_st_thread_bindcpu.us_throughput_pps_irq_low) &&
        (g_us_apf_program_len > OAL_SIZEOF(oal_uint8))) {
        pst_hmac_device->pst_device_base_info->en_apf_switch = OAL_TRUE;
        pst_suspend->uc_apf_switch = OAL_TRUE;
    } else {
        pst_hmac_device->pst_device_base_info->en_apf_switch = OAL_FALSE;
        pst_suspend->uc_apf_switch = OAL_FALSE;
    }

    oam_warning_log4(0, OAM_SF_CFG,
                     "hmac_set_apf_switch_by_pps:1.enable[%d]2.program len [%d]3.ul_total_rxtx_pps[%d]4.pps_low_th[%d]",
                     pst_suspend->uc_apf_switch, g_us_apf_program_len, ul_total_rxtx_pps,
                     g_st_thread_bindcpu.us_throughput_pps_irq_low);
}


OAL_STATIC oal_void hmac_print_apf_program(oal_uint8 *puc_program, oal_uint32 ul_program_len)
{
    oal_uint32 ul_idx, ul_string_len;
    oal_int32 l_string_tmp_len;
    oal_uint8 *pc_print_buff;

    pc_print_buff = (oal_int8 *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (pc_print_buff == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_print_apf_program::pc_print_buff null.}");
        return;
    }
    memset_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
    ul_string_len = 0;
    l_string_tmp_len = snprintf_s(pc_print_buff + ul_string_len,
        (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1), (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1) - 1,
        "Id           :200\n"
        "Program len  :%d\n",
        ul_program_len);
    if (l_string_tmp_len < 0) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "hmac_print_apf_program:sprintf return error[%d]", l_string_tmp_len);
        oal_mem_free_m(pc_print_buff, OAL_TRUE);
        return;
    }
    ul_string_len += l_string_tmp_len;
    for (ul_idx = 0; ul_idx < ul_program_len; ul_idx++) {
        l_string_tmp_len = snprintf_s(pc_print_buff + ul_string_len,
            (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1), (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1) - 1,
            "%02x",
            puc_program[ul_idx]);
        if (l_string_tmp_len < 0) {
            break;
        }
        ul_string_len += l_string_tmp_len;
    }

    pc_print_buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    oam_print(pc_print_buff);
    oal_mem_free_m(pc_print_buff, OAL_TRUE);
}
oal_uint32 hmac_apf_dispatch_event(mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_cmd_netbuf)
{
    dmac_tx_event_stru *pst_tx_event;
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_hmac_to_dmac_ctx_event;
    oal_uint32 ul_ret;

    pst_event_mem = frw_event_alloc_m(OAL_SIZEOF(dmac_tx_event_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_apf_dispatch_event::pst_event_mem null.}");
        oal_netbuf_free(pst_cmd_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_to_dmac_ctx_event = (frw_event_stru *)pst_event_mem->puc_data;
    frw_event_hdr_init(&(pst_hmac_to_dmac_ctx_event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_APF, OAL_SIZEOF(dmac_tx_event_stru), FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id, pst_mac_vap->uc_device_id, pst_mac_vap->uc_vap_id);

    pst_tx_event = (dmac_tx_event_stru *)(pst_hmac_to_dmac_ctx_event->auc_event_data);
    pst_tx_event->pst_netbuf = pst_cmd_netbuf;
    pst_tx_event->us_frame_len = oal_netbuf_len(pst_cmd_netbuf);

    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_apf_dispatch_event::frw_event_dispatch_event failed[%d].}", ul_ret);
    }

    oal_netbuf_free(pst_cmd_netbuf);
    frw_event_free_m(pst_event_mem);
    return ul_ret;
}

oal_uint32 hmac_config_set_apf(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_netbuf_stru *pst_cmd_netbuf = OAL_PTR_NULL;
    mac_apf_filter_cmd_stru *pst_apf_filter_cmd;

    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_CFG, "{null param,mac_vap=%xparam=%x.}", (uintptr_t)pst_mac_vap, (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_apf_filter_cmd = (mac_apf_filter_cmd_stru *)puc_param;

    /* 申请netbuf */
    pst_cmd_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF,
                                          OAL_SIZEOF(mac_apf_cmd_type_uint8) + pst_apf_filter_cmd->us_program_len,
                                          OAL_NETBUF_PRIORITY_MID);
    if (pst_cmd_netbuf == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_set_apf_program::netbuf alloc null,size %d.}",
                       pst_apf_filter_cmd->us_program_len);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 拷贝命令结构体到netbuf */
    if (memcpy_s(oal_netbuf_data(pst_cmd_netbuf),
        OAL_SIZEOF(mac_apf_cmd_type_uint8), &pst_apf_filter_cmd->en_cmd_type,
        OAL_SIZEOF(mac_apf_cmd_type_uint8)) != EOK) {
        oal_netbuf_free(pst_cmd_netbuf);
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_apf::memcopy_s fail.}");
        return OAL_FAIL;
    }
    oal_netbuf_put(pst_cmd_netbuf, OAL_SIZEOF(mac_apf_cmd_type_uint8));

    if (pst_apf_filter_cmd->en_cmd_type == APF_SET_FILTER_CMD) {
        /* program内容拷贝到netbuf */
        if (memcpy_s(oal_netbuf_data(pst_cmd_netbuf) + OAL_SIZEOF(mac_apf_cmd_type_uint8),
            pst_apf_filter_cmd->us_program_len, pst_apf_filter_cmd->puc_program,
            pst_apf_filter_cmd->us_program_len) != EOK) {
            oal_netbuf_free(pst_cmd_netbuf);
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_apf::memcopy_s fail.}");
            return OAL_FAIL;
        }
        oal_netbuf_put(pst_cmd_netbuf, pst_apf_filter_cmd->us_program_len);
        /* 打印到sdt */
        hmac_print_apf_program(pst_apf_filter_cmd->puc_program, pst_apf_filter_cmd->us_program_len);
        g_us_apf_program_len = pst_apf_filter_cmd->us_program_len;
    }

    /***************************************************************************
      抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    return hmac_apf_dispatch_event(pst_mac_vap, pst_cmd_netbuf);
}


oal_uint32 hmac_apf_program_report_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event;
    frw_event_hdr_stru *pst_event_hdr;
    mac_apf_report_event_stru *pst_report_event;
    oal_netbuf_stru *pst_netbuf;
    mac_apf_stru *pst_apf_info;

    /* 获取事件头和事件结构体指针 */
    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);
    pst_report_event = (mac_apf_report_event_stru *)pst_event->auc_event_data;

    pst_netbuf = (oal_netbuf_stru *)pst_report_event->p_program;
    if (!pst_netbuf) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_apf_program_report_event:netbuf is null");
        return OAL_FAIL;
    }
    pst_apf_info = (mac_apf_stru *)oal_netbuf_data(pst_netbuf);
    oam_warning_log3(0, OAM_SF_CFG, "hmac_apf_program_report_event:program_len[%d],pkt cnt[%d],install_timestamp[%d]",
                     pst_apf_info->us_program_len, pst_apf_info->ul_flt_pkt_cnt, pst_apf_info->ul_install_timestamp);
    hmac_print_apf_program(pst_apf_info->auc_program, pst_apf_info->us_program_len);

    oal_netbuf_free(pst_netbuf);
    return OAL_SUCC;
}
#endif


oal_uint32 hmac_config_protocol_debug_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    mac_protocol_debug_switch_stru *pst_protocol_debug = OAL_PTR_NULL;
    wlan_channel_bandwidth_enum_uint8 en_new_40m_bandwidth;
    hmac_vap_stru *pst_hmac_vap;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取mac device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_protocol_debug = (mac_protocol_debug_switch_stru *)puc_param;

    if (pst_protocol_debug->ul_cmd_bit_map & BIT0) {
        if (!IS_LEGACY_AP(pst_mac_vap)) {
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{vap is not ap,vap_mode[%d],p2p_mode[%d],return.",
                             pst_mac_vap->en_vap_mode, pst_mac_vap->en_p2p_mode);
            return OAL_SUCC;
        }

        en_new_40m_bandwidth = pst_protocol_debug->en_band_force_switch;

        if (pst_mac_vap->st_channel.en_bandwidth == en_new_40m_bandwidth) {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_protocol_debug_switch::bw[%d].}", en_new_40m_bandwidth);
            return OAL_SUCC;
        }

        if ((en_new_40m_bandwidth > WLAN_BAND_WIDTH_40MINUS) && (pst_mac_device->en_bandwidth_cap < WLAN_BW_CAP_80M)) {
            pst_protocol_debug->en_band_force_switch =
                mac_vap_get_bandwith(pst_mac_device->en_bandwidth_cap, pst_protocol_debug->st_csa_debug.en_bandwidth);

            /* 设置80M带宽，但device能力不支持80M， 刷新成合适带宽做业务 */
            oam_warning_log3(0, OAM_SF_CFG, "{not support bw[%d],new_protol=%d protocol_cap=%d.}", en_new_40m_bandwidth,
                             pst_protocol_debug->st_csa_debug.en_bandwidth, pst_mac_device->en_bandwidth_cap);

            en_new_40m_bandwidth = pst_protocol_debug->en_band_force_switch;
        }

        pst_hmac_vap->en_2040_switch_prohibited = OAL_FALSE;

        if (en_new_40m_bandwidth == WLAN_BAND_WIDTH_20M) {
            pst_hmac_vap->en_40m_bandwidth = pst_mac_vap->st_channel.en_bandwidth;
            hmac_chan_start_40m_recovery_timer(pst_mac_vap);
            hmac_chan_multi_switch_to_20MHz_ap(pst_hmac_vap);

            mac_mib_set_ShortGIOptionInFortyImplemented(pst_mac_vap, OAL_FALSE);
        } else {
            hmac_chan_prepare_for_40m_recovery(pst_hmac_vap, en_new_40m_bandwidth);

            mac_mib_set_ShortGIOptionInFortyImplemented(pst_mac_vap, OAL_TRUE);

            mac_mib_set_FortyMHzOperationImplemented(pst_mac_vap, OAL_TRUE);
        }

        oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{chan_num[%d],bandwidth[%d],en_new_40m_bandwidth[%d]}",
                         pst_mac_vap->st_channel.uc_chan_number,
                         pst_mac_vap->st_channel.en_bandwidth, en_new_40m_bandwidth);

        hmac_chan_multi_select_channel_mac(pst_mac_vap, pst_mac_vap->st_channel.uc_chan_number, en_new_40m_bandwidth);
        hmac_send_ht_notify_chan_width(pst_mac_vap, BROADCAST_MACADDR);
    }
    /* csa cmd */
    if (pst_protocol_debug->ul_cmd_bit_map & BIT1) {
        en_new_40m_bandwidth = pst_protocol_debug->st_csa_debug.en_bandwidth;
        oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_config_protocol_debug_switch::csa_channel=%d csa_bw=%d csa_cnt=%d debug_flag=%d \n",
                         pst_protocol_debug->st_csa_debug.uc_channel, en_new_40m_bandwidth,
                         pst_protocol_debug->st_csa_debug.uc_cnt, pst_protocol_debug->st_csa_debug.en_debug_flag);

        if (!IS_LEGACY_AP(pst_mac_vap)) {
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{vap mode is not ap,vap_mode[%d],p2p_mode[%d],return",
                             pst_mac_vap->en_vap_mode, pst_mac_vap->en_p2p_mode);
            return OAL_SUCC;
        }

        ul_ret = mac_is_channel_num_valid(pst_mac_vap->st_channel.en_band, pst_protocol_debug->st_csa_debug.uc_channel);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log3(0, OAM_SF_CFG, "{hmac_config_protocol_debug_switch:channel_num_valid(%d),band[%d],ret[%d]",
                             pst_protocol_debug->st_csa_debug.uc_channel, pst_mac_vap->st_channel.en_band, ul_ret);
            return OAL_SUCC;
        }

        if ((en_new_40m_bandwidth > WLAN_BAND_WIDTH_40MINUS) && (pst_mac_device->en_bandwidth_cap < WLAN_BW_CAP_80M)) {
            pst_protocol_debug->st_csa_debug.en_bandwidth =
                mac_vap_get_bandwith(pst_mac_device->en_bandwidth_cap, pst_protocol_debug->st_csa_debug.en_bandwidth);

            /* 设置80M带宽，但device能力不支持80M， 刷新成合适带宽做业务 */
            oam_warning_log3(0, OAM_SF_CFG, "{not support bw[%d], en_new_protocol=%d en_protocol_cap=%d.}",
                             en_new_40m_bandwidth, pst_protocol_debug->st_csa_debug.en_bandwidth,
                             pst_mac_device->en_bandwidth_cap);

            en_new_40m_bandwidth = pst_protocol_debug->st_csa_debug.en_bandwidth;
        }

        if (pst_protocol_debug->st_csa_debug.en_debug_flag == MAC_CSA_FLAG_NORMAL) {
            if (en_new_40m_bandwidth != WLAN_BAND_WIDTH_20M) {
                mac_mib_set_FortyMHzOperationImplemented(pst_mac_vap, OAL_TRUE);
            }

            pst_mac_vap->st_ch_switch_info.en_csa_mode = pst_protocol_debug->st_csa_debug.en_mode;
            pst_mac_vap->st_ch_switch_info.uc_ch_switch_cnt = pst_protocol_debug->st_csa_debug.uc_cnt;
            hmac_chan_initiate_switch_to_new_channel(pst_mac_vap, pst_protocol_debug->st_csa_debug.uc_channel,
                                                     en_new_40m_bandwidth);
        }
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PROTOCOL_DBG, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_protocol_debug_switch::hmac_config_send_event fail[%d].", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_force_pass_filter(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_suspend_stru st_suspend;
    oal_uint32 ul_ret;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    oal_uint32 ul_is_wlan_poweron;
    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_force_pass_filter::null param}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_en_force_pass_filter = *puc_param;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    ul_is_wlan_poweron = wlan_pm_is_poweron();
    if (wlan_pm_get_switch() && ul_is_wlan_poweron) {
        pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
        if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_force_pass_filter::pst_mac_device null.}");

            return OAL_ERR_CODE_PTR_NULL;
        }

        pst_hmac_device = hmac_res_get_mac_dev(pst_mac_device->uc_device_id);
        if (pst_hmac_device == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_force_pass_filter::hmac_device null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        if ((g_en_force_pass_filter == OAL_TRUE) && pst_mac_device->uc_in_suspend == OAL_TRUE
#ifdef _PRE_WLAN_FEATURE_WAPI
            && (pst_hmac_device->pst_device_base_info->uc_wapi != OAL_TRUE)
#endif
) {
            pst_hmac_device->pst_device_base_info->uc_arpoffload_switch = OAL_TRUE;
            st_suspend.uc_arpoffload_switch = OAL_TRUE;
            hmac_set_apf_switch_by_pps(pst_hmac_device, &st_suspend);
        } else {
            pst_hmac_device->pst_device_base_info->uc_arpoffload_switch = OAL_FALSE;
            st_suspend.uc_arpoffload_switch = OAL_FALSE;
            pst_hmac_device->pst_device_base_info->en_apf_switch = OAL_FALSE;
            st_suspend.uc_apf_switch = OAL_FALSE;
        }
        st_suspend.uc_in_suspend = pst_mac_device->uc_in_suspend;

        /***************************************************************************
            抛事件到DMAC层, 同步屏幕最新状态到DMAC
        ***************************************************************************/
        ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_APF_SWITCH_SYN, OAL_SIZEOF(mac_cfg_suspend_stru),
                                        (oal_uint8 *)&st_suspend);
        if (oal_unlikely(ul_ret != OAL_SUCC)) {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_force_pass_filter::send_event failed[%d]}", ul_ret);
        }
    }
#endif
    return OAL_SUCC;
}


oal_uint32 hmac_config_set_tx_ba_policy(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    if (puc_param == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "hmac_config_set_tx_ba_policy:puc_param is null");
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_ul_tx_ba_policy_select = *(oal_uint32 *)puc_param;
    OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_set_tx_ba_policy::g_uc_tx_ba_policy_select=%d.}",
                     g_ul_tx_ba_policy_select);

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH

oal_uint32 hmac_config_tas_rssi_access(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TAS_RSSI_ACCESS, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "tas_rssi_access::send_event fail, error no[%d]!", ul_ret);
        return ul_ret;
    }
    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_NRCOEX

oal_uint32 hmac_config_set_nrcoex_params(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
       抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_NRCOEX_PARAMS, us_len, puc_param);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_nrcoex_params::hmac_config_send_event failed[%d].}", ul_ret);
    }
    return ul_ret;
}


oal_uint32 hmac_config_set_wifi_priority(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
       抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_WIFI_PRIORITY, us_len, puc_param);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_wifi_priority::hmac_config_send_event failed[%d].}", ul_ret);
    }
    return ul_ret;
}


oal_uint32 hmac_config_set_nrcoex_cmd(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
       抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_NRCOEX_CMD, us_len, puc_param);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_wifi_priority::hmac_config_send_event failed[%d].}", ul_ret);
    }
    return ul_ret;
}


oal_uint32 hmac_config_get_nrcoex_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;

    /***************************************************************************
       抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_NRCOEX_INFO, us_len, puc_param);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_wifi_priority::hmac_config_send_event failed[%d].}", ul_ret);
    }
    return ul_ret;
}
#endif

/*lint -e578*//*lint -e19*/
#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
oal_module_symbol(hmac_config_set_tx_ba_policy);
#endif /* _PRE_WLAN_FEATURE_CUSTOM_SECURITY */
oal_module_symbol(hmac_config_set_pmksa);
oal_module_symbol(hmac_config_del_pmksa);
oal_module_symbol(hmac_config_flush_pmksa);
oal_module_symbol(hmac_config_get_version);
oal_module_symbol(hmac_config_get_ant);

oal_module_symbol(hmac_config_send_2040_coext);
oal_module_symbol(hmac_config_2040_coext_info);
oal_module_symbol(hmac_config_set_auto_protection);
oal_module_symbol(hmac_config_phy_debug_switch);
oal_module_symbol(hmac_config_set_random_mac_addr_scan);
oal_module_symbol(hmac_config_set_random_mac_oui);
oal_module_symbol(hmac_config_start_vap);
oal_module_symbol(hmac_config_add_vap);
oal_module_symbol(hmac_config_del_vap);
oal_module_symbol(hmac_config_down_vap);
oal_module_symbol(hmac_config_set_mac_addr);
oal_module_symbol(hmac_config_set_mode);
oal_module_symbol(hmac_config_get_mode);
oal_module_symbol(hmac_config_set_concurrent);
oal_module_symbol(hmac_config_get_concurrent);
oal_module_symbol(hmac_config_set_ssid);
oal_module_symbol(hmac_config_get_ssid);
oal_module_symbol(hmac_config_set_shpreamble);
oal_module_symbol(hmac_config_get_shpreamble);
oal_module_symbol(hmac_config_set_shortgi20);
oal_module_symbol(hmac_config_set_shortgi40);
oal_module_symbol(hmac_config_set_shortgi80);
oal_module_symbol(hmac_config_get_shortgi20);
oal_module_symbol(hmac_config_get_shortgi40);
oal_module_symbol(hmac_config_get_shortgi80);
oal_module_symbol(hmac_config_get_addr_filter);
oal_module_symbol(hmac_config_set_prot_mode);
oal_module_symbol(hmac_config_get_prot_mode);
oal_module_symbol(hmac_config_set_auth_mode);
oal_module_symbol(hmac_config_get_auth_mode);
oal_module_symbol(hmac_config_set_max_user);
oal_module_symbol(hmac_config_set_bintval);
oal_module_symbol(hmac_config_get_bintval);
oal_module_symbol(hmac_config_set_nobeacon);
oal_module_symbol(hmac_config_get_nobeacon);
oal_module_symbol(hmac_config_set_txchain);
oal_module_symbol(hmac_config_get_txchain);
oal_module_symbol(hmac_config_set_rxchain);
oal_module_symbol(hmac_config_get_rxchain);
oal_module_symbol(hmac_config_set_txpower);
oal_module_symbol(hmac_config_get_txpower);
oal_module_symbol(hmac_config_set_freq);
oal_module_symbol(hmac_config_get_freq);
oal_module_symbol(hmac_config_vap_info);
oal_module_symbol(hmac_config_user_info);
oal_module_symbol(hmac_config_add_user);
oal_module_symbol(hmac_config_del_user);
oal_module_symbol(hmac_config_addba_req);
oal_module_symbol(hmac_config_set_dscr_param);
oal_module_symbol(hmac_config_set_rate);
oal_module_symbol(hmac_config_log_level);
oal_module_symbol(hmac_config_set_mcs);
oal_module_symbol(hmac_config_set_mcsac);
oal_module_symbol(hmac_config_set_bw);
oal_module_symbol(hmac_config_always_rx);
oal_module_symbol(hmac_config_connect);
oal_module_symbol(hmac_config_delba_req);
oal_module_symbol(hmac_config_event_switch);
oal_module_symbol(hmac_config_amsdu_start);
oal_module_symbol(hmac_config_auto_ba_switch);
oal_module_symbol(hmac_config_list_sta);
oal_module_symbol(hmac_config_get_sta_list);
oal_module_symbol(hmac_config_list_ap);
oal_module_symbol(hmac_config_pause_tid);
oal_module_symbol(hmac_config_set_dtimperiod);
oal_module_symbol(hmac_config_get_dtimperiod);
oal_module_symbol(hmac_config_alg_param);
oal_module_symbol(hmac_config_set_amsdu_tx_on);
oal_module_symbol(hmac_config_set_ampdu_tx_on);
oal_module_symbol(hmac_config_get_country);
oal_module_symbol(hmac_config_set_country);
oal_module_symbol(hmac_config_amsdu_ampdu_switch);
oal_module_symbol(hmac_config_dump_rx_dscr);
oal_module_symbol(hmac_config_dump_tx_dscr);
oal_module_symbol(hmac_config_set_channel);
oal_module_symbol(hmac_config_set_beacon);
oal_module_symbol(hmac_config_set_app_ie_to_vap);
oal_module_symbol(hmac_config_set_wps_p2p_ie);
oal_module_symbol(hmac_config_set_wps_ie);
oal_module_symbol(hmac_config_list_channel);
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
oal_module_symbol(hmac_config_get_lauch_cap);
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
oal_module_symbol(hmac_config_set_cus_dyn_cali);
#endif
#endif
oal_module_symbol(hmac_config_set_regdomain_pwr);
oal_module_symbol(hmac_config_reduce_sar);
oal_module_symbol(hmac_config_reg_write);
#ifdef _PRE_WLAN_FEATURE_11D
oal_module_symbol(hmac_config_set_rd_by_ie_switch);
#endif
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))
oal_module_symbol(hmac_config_sdio_flowctrl);
#endif
oal_module_symbol(hmac_config_reg_info);
oal_module_symbol(hmac_config_dbb_scaling_amend);
oal_module_symbol(hmac_config_dump_all_rx_dscr);
oal_module_symbol(hmac_config_alg);
oal_module_symbol(hmac_config_send_event);
oal_module_symbol(hmac_config_sync_cmd_common);
oal_module_symbol(hmac_config_2040_channel_switch_prohibited);
oal_module_symbol(hmac_config_set_fortymhzintolerant);
oal_module_symbol(hmac_config_set_2040_coext_support);
oal_module_symbol(hmac_config_rx_fcs_info);
oal_module_symbol(hmac_config_get_tid);
oal_module_symbol(hmac_config_eth_switch);
oal_module_symbol(hmac_config_80211_ucast_switch);

oal_module_symbol(hmac_config_80211_mcast_switch);
oal_module_symbol(hmac_config_probe_switch);
oal_module_symbol(hmac_config_ota_beacon_switch);
oal_module_symbol(hmac_config_ota_rx_dscr_switch);
oal_module_symbol(hmac_config_set_all_ota);
oal_module_symbol(hmac_config_oam_output);
oal_module_symbol(hmac_config_set_dhcp_arp_switch);
oal_module_symbol(hmac_config_vap_pkt_stat);

#ifdef _PRE_DEBUG_MODE_USER_TRACK
oal_module_symbol(hmac_config_report_thrput_stat);
#endif

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
oal_module_symbol(hmac_config_tcp_ack_buf);
#endif

#ifdef _PRE_WLAN_FEATURE_UAPSD
oal_module_symbol(hmac_config_get_uapsden);
oal_module_symbol(hmac_config_set_uapsden);
#endif
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
oal_module_symbol(hmac_config_set_opmode_notify);
oal_module_symbol(hmac_config_get_user_rssbw);

#endif

#ifdef _PRE_WLAN_FEATURE_TXOPPS
oal_module_symbol(hmac_config_set_txop_ps_machw);
#endif
#ifdef _PRE_WLAN_FEATURE_BTCOEX
oal_module_symbol(hmac_config_print_btcoex_status);
#endif
#ifdef _PRE_WLAN_FEATURE_LTECOEX
oal_module_symbol(hmac_config_ltecoex_mode_set);
#endif

oal_module_symbol(hmac_config_bgscan_enable);
oal_module_symbol(hmac_config_set_ampdu_aggr_num);
oal_module_symbol(hmac_config_set_stbc_cap);
oal_module_symbol(hmac_config_set_ldpc_cap);

#ifdef _PRE_WLAN_DFT_STAT
oal_module_symbol(hmac_config_usr_queue_stat);
#endif

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
oal_module_symbol(hmac_config_set_edca_opt_cycle_ap);
oal_module_symbol(hmac_config_set_edca_opt_switch_ap);
oal_module_symbol(hmac_config_set_edca_opt_weight_sta);
oal_module_symbol(hmac_config_set_edca_opt_switch_sta);
#endif
oal_module_symbol(hmac_config_remain_on_channel);
oal_module_symbol(hmac_config_cancel_remain_on_channel);

oal_module_symbol(hmac_config_vap_classify_en);
oal_module_symbol(hmac_config_vap_classify_tid);
oal_module_symbol(hmac_config_always_tx_1102);
oal_module_symbol(hmac_config_always_tx_num);
oal_module_symbol(hmac_config_scan_abort);

#ifdef _PRE_WLAN_FEATURE_HS20
oal_module_symbol(hmac_config_set_qos_map);
#endif

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
oal_module_symbol(hmac_config_set_ip_addr);
#endif
oal_module_symbol(hmac_config_get_fem_pa_status);
#ifdef _PRE_WLAN_FEATURE_ROAM
oal_module_symbol(hmac_config_roam_enable);
oal_module_symbol(hmac_config_roam_start);
oal_module_symbol(hmac_config_roam_band);
oal_module_symbol(hmac_config_roam_org);
oal_module_symbol(hmac_config_roam_info);
#endif  // _PRE_WLAN_FEATURE_ROAM

#ifdef _PRE_WLAN_FEATURE_STA_PM
oal_module_symbol(hmac_config_set_pm_by_module);
#endif  // _PRE_WLAN_FEATURE_STA_PM
oal_module_symbol(hmac_config_query_rssi);
oal_module_symbol(hmac_config_query_rate);

#ifdef _PRE_WLAN_DFT_STAT
oal_module_symbol(hmac_config_query_ani);
#endif
oal_module_symbol(hmac_config_vendor_cmd_get_channel_list);
oal_module_symbol(hmac_config_query_station_info);

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
oal_module_symbol(hmac_config_dyn_cali_param);
#endif

#ifdef _PRE_WLAN_FEATURE_CSI_RAM
oal_module_symbol(hmac_config_set_csi);
#endif

oal_module_symbol(hmac_config_protocol_debug_switch);

oal_module_symbol(hmac_config_force_pass_filter);

/*lint +e578*//*lint +e19*/
