

/* 1 头文件包含 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/pm_qos.h>
#endif

#include "oam_ext_if.h"
#include "frw_ext_if.h"

#include "wlan_chip_i.h"

#include "hmac_device.h"
#include "mac_resource.h"
#include "hmac_resource.h"
#include "mac_vap.h"
#include "mac_ie.h"
#include "mac_function.h"
#include "oal_hcc_host_if.h"
#include "mac_user.h"
#include "mac_regdomain.h"
#include "host_hal_device.h"

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
#include "hmac_arp_probe.h"
#include "hmac_blacklist.h"
#include "hmac_degradation.h"

#include "hmac_scan.h"
#include "hmac_dfs.h"
#include "hmac_reset.h"
#include "hmac_scan.h"
#include "hmac_blockack.h"
#include "hmac_p2p.h"
#include "hmac_mgmt_ap.h"

#include "hmac_roam_main.h"
#include "hmac_roam_connect.h"
#include "hmac_roam_alg.h"

#include "hmac_arp_offload.h"
#include "hmac_vowifi.h"
#ifdef _PRE_WLAN_TCP_OPT
#include "mac_data.h"
#include "hmac_tcp_opt.h"
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif
#ifdef _PRE_WLAN_DFT_STAT
#include "mac_board.h"
#endif
#include "hmac_auto_adjust_freq.h"

#include "hmac_dfx.h"
#include "hmac_tx_msdu_dscr.h"
#include "hmac_host_al_tx.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "board.h"
#endif

#include "hmac_sme_sta.h"
#include "securec.h"
#include "securectype.h"

#include "hmac_roam_connect.h"
#ifdef _PRE_WLAN_FEATURE_TWT
#include "hmac_twt.h"
#endif

#include "wlan_mib.h"
#include "hmac_ht_self_cure.h"

#ifdef _PRE_WLAN_FEATURE_HIEX
#include "mac_hiex.h"
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
#include "hmac_wifi6_self_cure.h"
#include "hmac_11ax.h"
#endif
#include "host_hal_ext_if.h"
#ifdef _PRE_WLAN_RR_PERFORMENCE_DEBUG
#include "hmac_rr_performance.h"
#endif

#include "hmac_tx_complete.h"
#ifdef _PRE_WLAN_FEATURE_HID2D
#include "hmac_hid2d.h"
#endif
#ifdef _PRE_WLAN_FEATURE_HIEX
#include "hmac_hiex.h"
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
#include "mac_ftm.h"
#endif
#ifdef _PRE_WLAN_FEATURE_MBO
#include "hmac_mbo.h"
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif
#include "hmac_sae.h"
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
#include "hmac_tcp_ack_buf.h"
#endif
#include "mac_mib.h"
#include "hmac_11w.h"
#ifdef _PRE_WLAN_FEATURE_WMMAC
#include "hmac_wmmac.h"
#endif
#include "hmac_btcoex.h"

#ifdef _PRE_WLAN_FEATURE_DFR
#include "hmac_dfx.h"
#endif  /* _PRE_WLAN_FEATURE_DFR */

#include "wal_linux_bridge.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "wal_linux_atcmdsrv.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_C

#define ROAM_REENABLE_TIMEOUT_MS  120000 /* bssid驻留需求下发漫游关闭, 2分钟后使能漫游 */
hmac_tx_pkts_stat_stru g_host_tx_pkts;

typedef struct {
    wlan_protocol_enum_uint8 en_protocol_mode; /* wid枚举 */
    uint8_t auc_resv[NUM_3_BYTES];
    int8_t *puc_protocol_desc;
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
    { WLAN_HT_11G_MODE,        { 0 }, "11ng" },
#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
    { WLAN_HE_MODE, { 0 }, "11ax" },
#endif
};

#ifdef _PRE_WLAN_FEATURE_DDR_BUGFIX
int32_t g_ddr_qos_req_count = 0;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
struct pm_qos_request *g_pst_auto_ddr_freq = NULL; /* DDR频率申请结构体 */
#endif

#endif

uint32_t hmac_config_set_freq(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);

/*****************************************************************************
  3 函数实现
*****************************************************************************/
int8_t *hmac_config_index2string(uint32_t index, int8_t *pst_string[], uint32_t max_str_nums)
{
    if (index >= max_str_nums) {
        return (int8_t *)"unkown";
    }
    return pst_string[index];
}

int8_t *hmac_config_protocol2string(uint32_t protocol)
{
    int8_t *pac_protocol2string[] = {
        "11a", "11b", "error", "11bg", "11g", "11n",
        "11ac", "11nonly", "11aconly", "11ng", "11ax", "error"
    };
    return hmac_config_index2string(protocol, pac_protocol2string, sizeof(pac_protocol2string) / sizeof(int8_t *));
}

int8_t *hmac_config_band2string(uint32_t band)
{
    int8_t *pac_band2string[] = { "2.4G", "5G", "error" };
    return hmac_config_index2string(band, pac_band2string, sizeof(pac_band2string) / sizeof(int8_t *));
}

int8_t *hmac_config_bw2string(uint32_t bw)
{
    int8_t *pac_bw2string[] = {
        "20M", "40+", "40-", "80++", "80+-", "80-+", "80--",
#ifdef _PRE_WLAN_FEATURE_160M
        "160+++", "160++-", "160+-+", "160+--", "160-++", "160-+-", "160--+", "160---",
#endif
        "error"
    };
    return hmac_config_index2string(bw, pac_bw2string, sizeof(pac_bw2string) / sizeof(int8_t *));
}

int8_t *hmac_config_ciper2string(uint32_t ciper2)
{
    int8_t *pac_ciper2string[] = { "GROUP", "WEP40", "TKIP", "RSV", "CCMP", "WEP104", "BIP", "NONE" };
    return hmac_config_index2string(ciper2, pac_ciper2string, sizeof(pac_ciper2string) / sizeof(int8_t *));
}

int8_t *hmac_config_akm2string(uint32_t akm2)
{
    int8_t *pac_akm2string[] = { "RSV", "1X", "PSK", "FT_1X", "FT_PSK", "1X_SHA256", "PSK_SHA256", "NONE" };
    return hmac_config_index2string(akm2, pac_akm2string, sizeof(pac_akm2string) / sizeof(int8_t *));
}

int8_t *hmac_config_keytype2string(uint32_t keytype)
{
    int8_t *pac_keytype2string[] = { "GTK", "PTK", "RX_GTK", "ERR" };
    return hmac_config_index2string(keytype, pac_keytype2string,
                                    sizeof(pac_keytype2string) / sizeof(int8_t *));
}

int8_t *hmac_config_cipher2string(uint32_t cipher)
{
    int8_t *pac_cipher2string[] = { "GROUP", "WEP40", "TKIP", "NO_ENCRYP", "CCMP", "WEP104", "BIP", "GROUP_DENYD" };
    return hmac_config_index2string(cipher, pac_cipher2string, sizeof(pac_cipher2string) / sizeof(int8_t *));
}

int8_t *hmac_config_smps2string(uint32_t smps)
{
    int8_t *pac_smps2string[] = { "Static", "Dynamic", "MIMO", "error" };
    return hmac_config_index2string(smps, pac_smps2string, sizeof(pac_smps2string) / sizeof(int8_t *));
}

int8_t *hmac_config_dev2string(uint32_t dev)
{
    int8_t *pac_dev2string[] = { "Close", "Open", "error" };
    return hmac_config_index2string(dev, pac_dev2string, sizeof(pac_dev2string) / sizeof(int8_t *));
}

int8_t *hmac_config_nss2string(uint32_t nss)
{
    int8_t *pac_nss2string[] = { "Single Nss", "Double Nss", "error" };
    return hmac_config_index2string(nss, pac_nss2string, sizeof(pac_nss2string) / sizeof(int8_t *));
}

int8_t *hmac_config_b_w2string(uint32_t b_w)
{
    int8_t *pac_bw2string[] = { "20M", "40M", "80M", "error" };
    return hmac_config_index2string(b_w, pac_bw2string, sizeof(pac_bw2string) / sizeof(int8_t *));
}


OAL_STATIC uint32_t hmac_config_alloc_event(mac_vap_stru *pst_mac_vap,
                                            hmac_to_dmac_syn_type_enum_uint8 en_syn_type,
                                            hmac_to_dmac_cfg_msg_stru **ppst_syn_msg,
                                            frw_event_mem_stru **ppst_event_mem,
                                            uint16_t us_len)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event = NULL;

    pst_event_mem = frw_event_alloc_m(us_len + sizeof(hmac_to_dmac_cfg_msg_stru) - 4); /* 4:事件id + payload Length */
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_alloc_event::pst_event_mem null, us_len = %d }", us_len);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填充事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr), FRW_EVENT_TYPE_HOST_CRX, en_syn_type,
        (us_len + sizeof(hmac_to_dmac_cfg_msg_stru) - 4), /* 4:事件id + payload Length */
        FRW_EVENT_PIPELINE_STAGE_1, pst_mac_vap->uc_chip_id, pst_mac_vap->uc_device_id, pst_mac_vap->uc_vap_id);

    /* 出参赋值 */
    *ppst_event_mem = pst_event_mem;
    *ppst_syn_msg = (hmac_to_dmac_cfg_msg_stru *)pst_event->auc_event_data;

    return OAL_SUCC;
}


uint32_t hmac_config_send_event(mac_vap_stru *pst_mac_vap,
                                wlan_cfgid_enum_uint16 en_cfg_id,
                                uint16_t us_len,
                                uint8_t *puc_param)
{
    uint32_t ret;
    frw_event_mem_stru *pst_event_mem = NULL;
    hmac_to_dmac_cfg_msg_stru *pst_syn_msg = NULL;

    if (wlan_chip_h2d_cmd_need_filter(en_cfg_id) == OAL_TRUE) {
        return OAL_SUCC;
    }

    ret = hmac_config_alloc_event(pst_mac_vap, HMAC_TO_DMAC_SYN_CFG, &pst_syn_msg, &pst_event_mem, us_len);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_send_event::hmac_config_alloc_event failed[%d].}", ret);
        return ret;
    }

    HMAC_INIT_SYN_MSG_HDR(pst_syn_msg, en_cfg_id, us_len);

    /* 填写配置同步消息内容 */
    if ((puc_param != NULL) && (us_len)) {
        if (EOK != memcpy_s(pst_syn_msg->auc_msg_body, (uint32_t)us_len, puc_param, (uint32_t)us_len)) {
            oam_error_log0(0, OAM_SF_CFG, "hmac_config_send_event::memcpy fail!");
            frw_event_free_m(pst_event_mem);
            return OAL_FAIL;
        }
    }

    /* 抛出事件 */
    ret = frw_event_dispatch_event(pst_event_mem);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_send_event::frw_event_dispatch_event failed[%d].}", ret);
        frw_event_free_m(pst_event_mem);
        return ret;
    }

    frw_event_free_m(pst_event_mem);

    return OAL_SUCC;
}


uint32_t hmac_config_h2d_send_app_ie(mac_vap_stru *mac_vap, oal_app_ie_stru *app_ie)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    oal_netbuf_stru *netbuf_app_ie = NULL;
    uint16_t frame_len;
    dmac_tx_event_stru *app_ie_event = NULL;
    uint32_t ret;
    uint8_t *param = NULL;
    uint8_t app_ie_header_len;

    if (oal_any_null_ptr2(mac_vap, app_ie)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_h2d_send_app_ie::param is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 帧长校验 */
    app_ie_header_len = sizeof(oal_app_ie_stru) - sizeof(app_ie->auc_ie) / sizeof(app_ie->auc_ie[0]);

    frame_len = app_ie_header_len + app_ie->ie_len;
    if (frame_len >= WLAN_LARGE_NETBUF_SIZE || app_ie->ie_len > WLAN_WPS_IE_MAX_SIZE) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_h2d_send_app_ie::frame_len =[%d] ie_len=[%d] invalid.}", frame_len, app_ie->ie_len);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请netbuf内存  */
    netbuf_app_ie = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, frame_len, OAL_NETBUF_PRIORITY_MID);
    if (netbuf_app_ie == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_h2d_send_app_ie::netbuf_app_ie alloc fail}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 申请event 事件内存    */
    event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (event_mem == NULL) {
        oal_netbuf_free(netbuf_app_ie);
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_h2d_send_app_ie::event_mem alloc fail}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    event = frw_get_event_stru(event_mem);
    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_APP_IE_H2D, sizeof(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1, mac_vap->uc_chip_id, mac_vap->uc_device_id,
                       mac_vap->uc_vap_id);

    memset_s(oal_netbuf_cb(netbuf_app_ie), OAL_TX_CB_LEN, 0, OAL_TX_CB_LEN);

    param = (uint8_t *)(oal_netbuf_data(netbuf_app_ie));
    if (memcpy_s(param, frame_len, (uint8_t *)app_ie, frame_len) != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_h2d_send_app_ie::memcpy fail!");
        oal_netbuf_free(netbuf_app_ie);
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }

    app_ie_event = (dmac_tx_event_stru *)event->auc_event_data;
    app_ie_event->pst_netbuf = netbuf_app_ie;
    app_ie_event->us_frame_len = frame_len;
    app_ie_event->us_remain = 0;

    ret = frw_event_dispatch_event(event_mem);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_h2d_send_app_ie:: dispatch failed, app_ie_type=[%d],app_ie_len=[%d].}",
                         app_ie->en_app_ie_type, app_ie->ie_len);
        oal_netbuf_free(netbuf_app_ie);
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }

    oal_netbuf_free(netbuf_app_ie);
    frw_event_free_m(event_mem);
    return OAL_SUCC;
}


uint32_t hmac_config_alg_send_event(mac_vap_stru *mac_vap, wlan_cfgid_enum_uint16 cfg_id,
    uint16_t len, uint8_t *param)
{
    uint32_t ret;
    frw_event_mem_stru *event_mem = NULL;
    hmac_to_dmac_cfg_msg_stru *syn_msg = NULL;

    ret = hmac_config_alloc_event(mac_vap, HMAC_TO_DMAC_SYN_ALG, &syn_msg, &event_mem, len);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_alg_send_event::hmac_config_alloc_event failed[%d].}", ret);
        return ret;
    }

    HMAC_INIT_SYN_MSG_HDR(syn_msg, cfg_id, len);

    /* 填写配置同步消息内容 */
    if (memcpy_s(syn_msg->auc_msg_body, len, param, len) != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_alg_send_event::memcpy fail!");
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }

    /* 抛出事件 */
    ret = frw_event_dispatch_event(event_mem);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_alg_send_event::frw_event_dispatch_event failed[%d].}", ret);
        frw_event_free_m(event_mem);
        return ret;
    }

    frw_event_free_m(event_mem);

    return OAL_SUCC;
}


uint32_t hmac_config_start_vap_event(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_mgmt_rate_init_flag)
{
    uint32_t ret;
    mac_cfg_start_vap_param_stru st_start_vap_param;

    /* DMAC不使用netdev成员 */
    st_start_vap_param.pst_net_dev = NULL;
    st_start_vap_param.en_mgmt_rate_init_flag = en_mgmt_rate_init_flag;
    st_start_vap_param.uc_protocol = pst_mac_vap->en_protocol;
    st_start_vap_param.uc_band = pst_mac_vap->st_channel.en_band;
    st_start_vap_param.uc_bandwidth = pst_mac_vap->st_channel.en_bandwidth;
    st_start_vap_param.en_p2p_mode = pst_mac_vap->en_p2p_mode;
    ret = hmac_config_send_event(pst_mac_vap,
        WLAN_CFGID_START_VAP, sizeof(mac_cfg_start_vap_param_stru), (uint8_t *)&st_start_vap_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_start_vap_event::Start_vap failed[%d].}", ret);
    }
    return ret;
}

uint32_t hmac_set_mode_event(mac_vap_stru *mac_vap)
{
    uint32_t ret;
    mac_cfg_mode_param_stru prot_param = {0};

    /* 设置带宽模式，直接抛事件到DMAC配置寄存器 */
    prot_param.en_protocol = mac_vap->en_protocol;
    prot_param.en_band = mac_vap->st_channel.en_band;
    prot_param.en_bandwidth = mac_vap->st_channel.en_bandwidth;

    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_MODE, sizeof(mac_cfg_mode_param_stru), (uint8_t *)&prot_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_set_mode_event::mode_set failed[%d],protocol[%d], band[%d], bandwidth[%d].}",
            ret, mac_vap->en_protocol, mac_vap->st_channel.en_band, mac_vap->st_channel.en_bandwidth);
    }
    return ret;
}


uint32_t hmac_config_sync_cmd_common(mac_vap_stru *pst_mac_vap,
                                     wlan_cfgid_enum_uint16 en_cfg_id,
                                     uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, en_cfg_id, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_sync_cmd_common::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

uint32_t hmac_config_open_wmm(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    oal_bool_enum_uint8 en_wmm;

    ret = hmac_config_sync_cmd_common(pst_mac_vap, WLAN_CFGID_WMM_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC,
                       "{hmac_config_open_wmm::hmac_config_sync_cmd_common failed[%d].}", ret);
        return ret;
    }

    en_wmm = *(oal_bool_enum_uint8 *)puc_param;
    /* 开关WMM，更新host侧mib信息位中的Qos位置 */
    mac_mib_set_dot11QosOptionImplemented(pst_mac_vap, en_wmm);

    return ret;
}


OAL_STATIC OAL_INLINE uint32_t hmac_normal_check_legacy_vap_num(mac_device_stru *pst_mac_device,
                                                                wlan_vap_mode_enum_uint8 en_vap_mode)
{
    /* VAP个数判断 */
    if (en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        if ((pst_mac_device->uc_sta_num == WLAN_SERVICE_STA_MAX_NUM_PER_DEVICE) &&
            (pst_mac_device->uc_vap_num == WLAN_AP_STA_COEXIST_VAP_NUM)) {
            /* AP STA共存场景，只能创建4个AP + 1个STA */
            oam_warning_log0(0, OAM_SF_CFG,
                             "{hmac_normal_check_legacy_vap_num::have created 4AP + 1STA, cannot create another AP.}");
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }

        if ((pst_mac_device->uc_vap_num - pst_mac_device->uc_sta_num) >= WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE) {
            /* 已创建的AP个数达到最大值4 */
            oam_warning_log2(0, OAM_SF_CFG,
                "{hmac_normal_check_legacy_vap_num::ap num exceeds the supported spec,vap_num[%u],sta_num[%u].}",
                pst_mac_device->uc_vap_num, pst_mac_device->uc_sta_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }
    } else if (en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (pst_mac_device->uc_sta_num >= WLAN_SERVICE_STA_MAX_NUM_PER_DEVICE) {
            /* 已创建的STA个数达到最大值 */
            oam_warning_log1(0, OAM_SF_CFG,
                             "{hmac_normal_check_legacy_vap_num::have created 2+ AP.can not create STA any more[%d].}",
                             pst_mac_device->uc_sta_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }
    }

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE uint32_t hmac_config_normal_check_vap_num(mac_device_stru *pst_mac_device,
                                                                mac_cfg_add_vap_param_stru *pst_param)
{
    wlan_vap_mode_enum_uint8 en_vap_mode;
    wlan_p2p_mode_enum_uint8 en_p2p_mode;

    en_p2p_mode = pst_param->en_p2p_mode;
    if (en_p2p_mode != WLAN_LEGACY_VAP_MODE) {
        return hmac_check_p2p_vap_num(pst_mac_device, en_p2p_mode);
    }

    en_vap_mode = pst_param->en_vap_mode;
    return hmac_normal_check_legacy_vap_num(pst_mac_device, en_vap_mode);
}


OAL_STATIC uint32_t hmac_config_check_vap_num(mac_device_stru *pst_mac_device, mac_cfg_add_vap_param_stru *pst_param)
{
    return hmac_config_normal_check_vap_num(pst_mac_device, pst_param);
}

uint32_t hmac_cfg_vap_send_event(mac_device_stru *pst_device)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event = NULL;
    uint32_t ret;

    /* 抛事件给DMAC,让DMAC完成配置VAP创建 */
    pst_event_mem = frw_event_alloc_m(0);
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_cfg_vap_send_event::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CRX,
                       HMAC_TO_DMAC_SYN_CREATE_CFG_VAP,
                       0,
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_device->uc_chip_id,
                       pst_device->uc_device_id,
                       pst_device->uc_cfg_vap_id);

    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_cfg_vap_send_event::frw_event_dispatch_event failed[%d].}", ret);
    }

    /* 释放事件 */
    frw_event_free_m(pst_event_mem);

    return ret;
}

OAL_STATIC uint32_t hmac_config_add_vap_uapsd_is_enable(mac_cfg_add_vap_param_stru *pst_param,
    hmac_vap_stru *pst_hmac_vap)
{
    switch (pst_param->en_vap_mode) {
        case WLAN_VAP_MODE_BSS_AP:
            pst_param->bit_uapsd_enable = pst_hmac_vap->st_vap_base_info.st_cap_flag.bit_uapsd;
            break;

        case WLAN_VAP_MODE_BSS_STA:
        case WLAN_VAP_MODE_WDS:
            break;

        default:
            return OAL_ERR_CODE_INVALID_CONFIG;
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_get_p2p_max_user(void)
{
    return g_wlan_spec_cfg->p2p_go_max_user_num;
}


void hmac_config_add_vap_cfg(hmac_vap_stru *pst_hmac_vap, mac_cfg_add_vap_param_stru *pst_param)
{
    pst_hmac_vap->pst_net_device = pst_param->pst_net_dev;

    /* 包括'\0' */
    memcpy_s(pst_hmac_vap->auc_name, OAL_IF_NAME_SIZE, pst_param->pst_net_dev->name, OAL_IF_NAME_SIZE);

    /* 将申请到的mac_vap空间挂到net_device ml_priv指针上去 */
    oal_net_dev_priv(pst_param->pst_net_dev) = &pst_hmac_vap->st_vap_base_info;
    /* 申请hmac组播用户 */
    hmac_user_add_multi_user(&(pst_hmac_vap->st_vap_base_info), &pst_param->us_muti_user_id);
    mac_vap_set_multi_user_idx(&(pst_hmac_vap->st_vap_base_info), pst_param->us_muti_user_id);
}


OAL_STATIC uint32_t hmac_config_check_vap_comb(mac_device_stru *mac_device, mac_cfg_add_vap_param_stru *add_vap_param)
{
    /* 双sta模式下，不能创建gc/go */
    if (mac_is_dual_sta_mode() &&
        (add_vap_param->en_p2p_mode == WLAN_P2P_CL_MODE || add_vap_param->en_p2p_mode == WLAN_P2P_GO_MODE)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_check_vap_comb::cannot add gc/go[%d] in dual sta mode!}",
            add_vap_param->en_p2p_mode);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    /* 已创建gc/go，不能创建wlan1 */
    if (mac_device->st_p2p_info.uc_p2p_goclient_num != 0 && !add_vap_param->is_primary_vap &&
        add_vap_param->en_vap_mode == WLAN_VAP_MODE_BSS_STA && add_vap_param->en_p2p_mode == WLAN_LEGACY_VAP_MODE) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_check_vap_comb::cannot add secondary wlan while p2p is work!}");
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_CHBA_MGMT
static hmac_join_req_stru g_join_param = {
    .st_bss_dscr = {
        .auc_bssid = {0x10, 0xe9, 0x53, 0x44, 0x55, 0x66},
        .us_beacon_period = 100,
        .st_channel = {
            .en_band= WLAN_BAND_5G,
            .ext6g_band = OAL_FALSE,
            .uc_chan_number = 36,
            .en_bandwidth = WLAN_BAND_WIDTH_20M,
            .uc_chan_idx = 0,
        },

        .ac_country = {'C', 'N'},
        .en_channel_bandwidth = WLAN_BAND_WIDTH_20M, // hmac_sta_update_join_bw
        .en_ht_capable = 1,
        .auc_supp_rates = {0x8c, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6c},
        .uc_num_supp_rates = 8,
    }
};

uint8_t g_dummy_user_mac[] = {0x10, 0xe9, 0x53, 0x66, 0x55, 0x44};
uint32_t hmac_sta_update_join_req_params_post_event(hmac_vap_stru *pst_hmac_vap,
    hmac_join_req_stru *pst_join_req, mac_ap_type_enum_uint16 en_ap_type);
OAL_STATIC OAL_INLINE void hmac_sta_set_mode_params(mac_cfg_mode_param_stru *cfg_mode,
    wlan_protocol_enum_uint8 protocol, wlan_channel_band_enum_uint8 band, uint8_t channel_idx,
    wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    cfg_mode->en_protocol = protocol;
    cfg_mode->en_band = band;
    cfg_mode->en_bandwidth = bandwidth;
    cfg_mode->en_channel_idx = channel_idx;
}

void hmac_demo_main(hmac_vap_stru *hmac_vap)
{
    mac_cfg_mode_param_stru cfg_mode = {WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_20M};
    mac_cfg_add_user_param_stru add_user_param;
    /* 1.vap初始化 */
    mac_vap_set_bssid(&hmac_vap->st_vap_base_info, g_join_param.st_bss_dscr.auc_bssid);
    mac_vap_set_current_channel(&hmac_vap->st_vap_base_info, WLAN_BAND_5G, 36, OAL_FALSE);
    hmac_vap->st_vap_base_info.st_channel.en_bandwidth = WLAN_BAND_WIDTH_20M;
    hmac_vap->st_vap_base_info.en_protocol = WLAN_VHT_MODE;
    hmac_sta_set_mode_params(&cfg_mode, hmac_vap->st_vap_base_info.en_protocol, \
        hmac_vap->st_vap_base_info.st_channel.en_band, \
        g_join_param.st_bss_dscr.st_channel.uc_chan_number, hmac_vap->st_vap_base_info.st_channel.en_bandwidth);
    // 协议模式11n，速率 && start vap。
    hmac_config_sta_update_rates(&hmac_vap->st_vap_base_info, &cfg_mode, &g_join_param.st_bss_dscr);
    // join event h2d
    hmac_sta_update_join_req_params_post_event(hmac_vap, &g_join_param, 0);
    hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_JOIN_COMP);
    // open
    mac_mib_set_AuthenticationMode(&hmac_vap->st_vap_base_info, WLAN_WITP_AUTH_OPEN_SYSTEM);
    /* 2.创建user */
    oal_set_mac_addr(add_user_param.auc_mac_addr, g_dummy_user_mac);
    add_user_param.en_ht_cap = OAL_TRUE;
    hmac_config_add_user(&hmac_vap->st_vap_base_info, sizeof(add_user_param), (uint8_t*)&add_user_param);
}

uint32_t hmac_config_start_demo(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *hmac_vap = NULL;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_demo::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_demo_main(hmac_vap);
    return OAL_SUCC;
}
#endif

uint32_t hmac_config_add_vap(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_add_vap_param_stru *param = (mac_cfg_add_vap_param_stru *)puc_param;
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t ret;
    mac_device_stru *mac_dev = NULL;
    uint8_t vap_id;

    mac_dev = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_dev == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::mac_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    ret = hmac_config_check_vap_comb(mac_dev, param);
    if (ret != OAL_SUCC) {
        return ret;
    }
    param->uc_dst_hal_dev_id = 0;  // 默认创建在主路

    if (param->en_p2p_mode == WLAN_P2P_CL_MODE) {
        return hmac_add_p2p_cl_vap(mac_vap, us_len, puc_param);
    }

    /* VAP个数判断 */
    ret = hmac_config_check_vap_num(mac_dev, param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::check_vap_num failed[%d].}", ret);
        return ret;
    }

    if (oal_net_dev_priv(param->pst_net_dev) != NULL) {
        return OAL_SUCC;
    }
    /* 从资源池申请hmac vap */
    /*lint -e413*/
    ret = mac_res_alloc_hmac_vap(&vap_id, OAL_OFFSET_OF(hmac_vap_stru, st_vap_base_info));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::mac_res_alloc_hmac_vap failed[%d]}", ret);
        return ret;
    }
    /*lint +e413*/
    /* 从资源池获取新申请到的hmac vap */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    param->uc_vap_id = vap_id;

    /* 初始清0 */
    memset_s(hmac_vap, sizeof(hmac_vap_stru), 0, sizeof(hmac_vap_stru));

    /* 初始化HMAC VAP */
    ret = hmac_vap_init(hmac_vap, mac_dev->uc_chip_id, mac_dev->uc_device_id, vap_id, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::hmac_vap_init failed[%d].}", ret);
        if (hmac_vap->st_vap_base_info.pst_mib_info != NULL) {
            oal_mem_free_m(hmac_vap->st_vap_base_info.pst_mib_info, OAL_TRUE);
        }
        if (hmac_vap->st_vap_base_info.en_p2p_mode == WLAN_LEGACY_VAP_MODE) {
            mac_vap_vowifi_exit(&(hmac_vap->st_vap_base_info));
        }
        /* 异常处理，释放内存 */
        mac_res_free_mac_vap(vap_id);
        return ret;
    }

    if (IS_LEGACY_STA(&hmac_vap->st_vap_base_info)) {
        /* 记录TxBASessionNumber mib值到chr全局变量中 */
        hmac_chr_set_ba_session_num(mac_mib_get_TxBASessionNumber(&hmac_vap->st_vap_base_info));
    }

    /* 设置反挂的net_device指针 */
    if (param->en_p2p_mode == WLAN_P2P_DEV_MODE) {
        /* p2p0 DEV 模式vap，采用pst_p2p0_net_device 成员指向对应的net_device */
        hmac_vap->pst_p2p0_net_device = param->pst_net_dev;
        mac_dev->st_p2p_info.uc_p2p0_vap_idx = hmac_vap->st_vap_base_info.uc_vap_id;
    }

    hmac_config_add_vap_cfg(hmac_vap, param);

    mac_device_set_vap_id(mac_dev, &(hmac_vap->st_vap_base_info), vap_id, param->en_vap_mode, param->en_p2p_mode, OAL_TRUE);
    if (hmac_config_add_vap_uapsd_is_enable(param, hmac_vap) != OAL_SUCC) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(&hmac_vap->st_vap_base_info, WLAN_CFGID_ADD_VAP, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        /* 此处回退有误，需要对应mac_device_set_vap_id，做回退操作 */
        mac_device_set_vap_id(mac_dev, &(hmac_vap->st_vap_base_info), vap_id, param->en_vap_mode, param->en_p2p_mode, OAL_FALSE);
        hmac_user_del_multi_user(&(hmac_vap->st_vap_base_info));
        if (hmac_vap->st_vap_base_info.en_p2p_mode == WLAN_LEGACY_VAP_MODE) {
            mac_vap_vowifi_exit(&(hmac_vap->st_vap_base_info));
        }

        /* 异常处理，释放内存 */
        oal_mem_free_m(hmac_vap->st_vap_base_info.pst_mib_info, OAL_TRUE);

        mac_res_free_mac_vap(vap_id);
        oal_net_dev_priv(param->pst_net_dev) = NULL;

        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::hmac_config_alloc_event fail[%d]}", ret);
        return ret;
    }
    if (IS_P2P_GO(&hmac_vap->st_vap_base_info)) {
        hmac_config_set_max_user(&hmac_vap->st_vap_base_info, hmac_get_p2p_max_user());
    }
    oam_warning_log4(vap_id, OAM_SF_ANY, "{hmac_config_add_vap::SUCCESS!!vap_mode[%d], p2p_mode[%d]}, multi user idx[%d], device id[%d]",
        param->en_vap_mode, param->en_p2p_mode, hmac_vap->st_vap_base_info.us_multi_user_idx,
        hmac_vap->st_vap_base_info.uc_device_id);

#ifdef _PRE_WLAN_FEATURE_TWT
    if (g_wlan_spec_cfg->feature_twt_is_open) {
        hmac_dbac_teardown_twt_session(mac_dev);
    }
#endif
    if (mac_is_secondary_sta(&hmac_vap->st_vap_base_info)) {
        mac_set_dual_sta_mode(OAL_TRUE);
    }
    return OAL_SUCC;
}

uint32_t hmac_get_chip_vap_num(mac_chip_stru *pst_chip)
{
    mac_device_stru *pst_mac_device = NULL;
    uint8_t uc_device;
    uint8_t uc_vap_num = 0;

    for (uc_device = 0; uc_device < pst_chip->uc_device_nums; uc_device++) {
        pst_mac_device = mac_res_get_dev(pst_chip->auc_device_id[uc_device]);
        if (pst_mac_device == NULL) {
            oam_error_log1(0, OAM_SF_ANY,
                           "hmac_get_chip_vap_num::mac_res_get_dev id[%d] NULL", pst_chip->auc_device_id[uc_device]);
            continue;
        }

        uc_vap_num += pst_mac_device->uc_vap_num;
    }
    return uc_vap_num;
}


void hmac_config_del_scaning_flag(mac_vap_stru *pst_mac_vap)
{
    hmac_device_stru *pst_hmac_device;
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_del_scaning_flag::pst_hmac_device is null, dev_id[%d].}", pst_mac_vap->uc_device_id);
        return;
    }

    if (pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.uc_vap_id == pst_mac_vap->uc_vap_id) {
        pst_hmac_device->st_scan_mgmt.en_is_scanning = OAL_FALSE;
    }
}

OAL_STATIC void hmac_del_vap_destroy_timer(hmac_vap_stru *pst_hmac_vap)
{
    if (pst_hmac_vap->st_mgmt_timer.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(pst_hmac_vap->st_mgmt_timer));
    }
    if (pst_hmac_vap->st_scan_timeout.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(pst_hmac_vap->st_scan_timeout));
    }
    if (pst_hmac_vap->st_ps_sw_timer.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(pst_hmac_vap->st_ps_sw_timer));
    }

#ifdef _PRE_WLAN_FEATURE_FTM
    if (g_wlan_spec_cfg->feature_ftm_is_open) {
        if (pst_hmac_vap->st_ftm_timer.en_is_registerd == OAL_TRUE) {
            frw_timer_immediate_destroy_timer_m(&(pst_hmac_vap->st_ftm_timer));
        }
    }
#endif
}

/* 功能描述 : rx ddr 接收数据帧 host侧hal vap与mac vap， hal device 之间的映射关系同步 */
OAL_STATIC uint32_t hmac_config_d2h_hal_vap_syn(mac_vap_stru *mac_vap, uint8_t len, uint8_t *p_param)
{
    d2h_hal_vap_info_syn_event *p_hal_vap_info = NULL;
    hal_host_device_stru       *p_hal_dev = NULL;
    hmac_vap_stru              *hmac_vap = NULL;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, 0, "hmac_config_d2h_hal_vap_syn::hmac vap null");
        return OAL_FAIL;
    }

    p_hal_vap_info = (d2h_hal_vap_info_syn_event *)p_param;
    if ((p_hal_vap_info->hal_dev_id >= WLAN_DEVICE_MAX_NUM_PER_CHIP) ||
        (p_hal_vap_info->hal_vap_id >= HAL_MAX_VAP_NUM)) {
        return OAL_FAIL;
    }

    p_hal_dev = hal_get_host_device(p_hal_vap_info->hal_dev_id);
    if (p_hal_dev == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, 0, "hmac_config_d2h_hal_vap_syn:: hal_dev null");
        return OAL_FAIL;
    }

    if (!p_hal_vap_info->valid) {
        p_hal_dev->hal_vap_sts_info[p_hal_vap_info->hal_vap_id].hal_vap_valid = 0;
        hmac_vap->hal_dev_id = 0xff;
    } else {
        p_hal_dev->hal_vap_sts_info[p_hal_vap_info->hal_vap_id].hal_vap_valid = 1;
        p_hal_dev->hal_vap_sts_info[p_hal_vap_info->hal_vap_id].mac_vap_id = p_hal_vap_info->mac_vap_id;
        hmac_vap->hal_dev_id = p_hal_vap_info->hal_dev_id;
    }

    oam_warning_log4(mac_vap->uc_vap_id, 0, "hmac_config_d2h_hal_vap_syn::haldev[%d],hal vap[%d],mac vap[%d],valid[%d]",
        p_hal_vap_info->hal_dev_id, p_hal_vap_info->hal_vap_id,
        p_hal_vap_info->mac_vap_id, p_hal_vap_info->valid);

    return OAL_SUCC;
}


/* 功能描述 : rx ddr 接收数据帧 host侧hal vap与mac vap， hal device 之间的映射关系同步 */
OAL_STATIC uint32_t hmac_config_d2h_usr_lut_syn(mac_vap_stru *mac_vap, uint8_t len, uint8_t *p_param)
{
    d2h_usr_lut_info_syn_event *p_usr_ldx_info = NULL;
    hal_host_device_stru       *p_hal_dev = NULL;

    p_usr_ldx_info = (d2h_usr_lut_info_syn_event *)p_param;
    if (p_usr_ldx_info->lut_idx >= HAL_MAX_LUT) {
        return OAL_FAIL;
    }

    p_hal_dev = hal_get_host_device(p_usr_ldx_info->hal_dev_id);
    if (p_hal_dev == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, 0, "hmac_config_d2h_usr_lut_syn: hal_get_host_device return null.\n");
        return OAL_FAIL;
    }
    if (!p_usr_ldx_info->valid) {
        p_hal_dev->user_sts_info[p_usr_ldx_info->lut_idx].user_valid = 0;
    } else {
        p_hal_dev->user_sts_info[p_usr_ldx_info->lut_idx].user_valid = 1;
        p_hal_dev->user_sts_info[p_usr_ldx_info->lut_idx].user_id = p_usr_ldx_info->usr_idx;
    }

    oam_warning_log4(mac_vap->uc_vap_id, 0, "hmac_config_d2h_usr_lut_syn::haldev[%d],lut_idx[%d],usr_idx[%d],valid[%d]",
        p_usr_ldx_info->hal_dev_id, p_usr_ldx_info->lut_idx,
        p_usr_ldx_info->usr_idx, p_usr_ldx_info->valid);

    return OAL_SUCC;
}
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef CONFIG_ARCH_KIRIN_PCIE
static void hmac_del_vap_disable_hal_vap(hmac_vap_stru *hmac_vap)
{
    hal_host_device_stru *haldev = NULL;
    uint8_t loop;

    haldev = hal_get_host_device(hmac_vap->hal_dev_id);
    if (haldev == NULL) {
        return;
    }

    for (loop = 0; loop < HAL_MAX_VAP_NUM; loop++) {
        if (haldev->hal_vap_sts_info[loop].mac_vap_id ==
            hmac_vap->st_vap_base_info.uc_vap_id) {
            haldev->hal_vap_sts_info[loop].hal_vap_valid = 0;
            oam_warning_log2(0, OAM_SF_ANY, "{hmac_del_vap_disable_hal_vap::mac id[%d],hal id[%d].}",
                hmac_vap->st_vap_base_info.uc_vap_id, loop);
            return;
        }
    }

    oam_error_log1(0, OAM_SF_ANY, "{hmac_del_vap_disable_hal_vap::mac id[%d] not find hal}",
        hmac_vap->st_vap_base_info.uc_vap_id);

    return;
}
#endif
#endif
static uint32_t hmac_config_d2h_rx_mode_syn(mac_vap_stru *mac_vap, uint8_t len, uint8_t *p_param)
{
    mac_rx_switch_stru  *rx_switch = (mac_rx_switch_stru *)p_param;
    hal_host_device_stru *hal_device  = hal_get_host_device(rx_switch->hal_device_id);
    if (hal_device == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_RX, "hmac_config_d2h_rx_mode_syn::hal device[%d]null ",
                       rx_switch->hal_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (rx_switch->new_rx_mode > HAL_DDR_RX || hal_device->rx_mode == rx_switch->new_rx_mode) {
        oam_error_log2(mac_vap->uc_vap_id, OAM_SF_RX, "hmac_config_d2h_rx_mode_syn::new[%d]==old[%d][0:ram,1:ddr]",
                       rx_switch->new_rx_mode, hal_device->rx_mode);
        return OAL_FAIL;
    }

    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_RX, "hmac_config_d2h_rx_mode_syn:: \
        switch rx from %d to %d [0:ram, 1:ddr]", hal_device->rx_mode, rx_switch->new_rx_mode);

    if (rx_switch->new_rx_mode == HAL_DDR_RX) {
        /* host完成接收的准备工作,抛事件到device使能DDR接收 */
        if (hal_rx_host_start_dscr_queue(rx_switch->hal_device_id) != OAL_SUCC) {
            return OAL_FAIL;
        }
    }

    /* 统一host切换接收模式，防止d2h没内存抛事件失败导致上下状态不一致 */
    hmac_config_send_event(mac_vap, WLAN_CFGID_RX_MODE_SWITCH, sizeof(mac_rx_switch_stru), p_param);
    hal_device->rx_mode = rx_switch->new_rx_mode;
    return OAL_SUCC;
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OAL_STATIC uint32_t hmac_del_vap_post_process(mac_vap_stru *pst_vap)
{
    mac_chip_stru *pst_chip = NULL;
    uint8_t uc_vap_num;
    hmac_device_stru *pst_hmac_device = NULL;

    // 如果WIFI sta，wlan0 stop，下电
    pst_chip = hmac_res_get_mac_chip(pst_vap->uc_chip_id);
    if (pst_chip == NULL) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_del_vap_post_process::hmac_res_get_mac_chip id[%d] NULL",
            pst_vap->uc_chip_id);
        return OAL_FAIL;
    }

    uc_vap_num = hmac_get_chip_vap_num(pst_chip);
    if (uc_vap_num != 0) {
        return OAL_SUCC;
    }

    pst_hmac_device = hmac_res_get_mac_dev(pst_vap->uc_device_id);
    if (oal_likely(pst_hmac_device != NULL)) {
        hmac_scan_clean_scan(&(pst_hmac_device->st_scan_mgmt));
#ifdef _PRE_WLAN_FEATURE_MONITOR
        // wlan0下电清除sniffer所置标记
        if (pst_hmac_device->en_monitor_mode == OAL_TRUE) {
            pst_hmac_device->en_monitor_mode = OAL_FALSE;
        }
        if (pst_hmac_device->sniffer_mode != WLAN_SINFFER_OFF) {
            pst_hmac_device->sniffer_mode = WLAN_SINFFER_OFF;
        }
#endif
    } else {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_del_vap_post_process::pst_hmac_device[%d] null!}", pst_vap->uc_device_id);
    }

    /* chip下的所有device的业务vap个数为0,才能给device下电 */
    if (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* APUT启动时不关闭device */
        if (OAL_ERR_CODE_FOBID_CLOSE_DEVICE != wlan_pm_close()) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{hmac_del_vap_post_process::hmac_config_host_dev_exit! pst_hmac_device[%d]}", pst_vap->uc_device_id);
            hmac_config_host_dev_exit(pst_vap);
        }
    }

    return OAL_SUCC;
}
#endif


uint32_t hmac_config_del_vap(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    hmac_vap_stru *hmac_vap = NULL;
    mac_device_stru *mac_dev = NULL;
    mac_cfg_del_vap_param_stru *del_vap_param = NULL;
    uint32_t ret;

    if (oal_any_null_ptr2(mac_vap, param)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_del_vap::mac_vap or param null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    del_vap_param = (mac_cfg_del_vap_param_stru *)param;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_vap::mac_res_get_hmac_vap failed.}");
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_SAE
    oal_cancel_work_sync(&(hmac_vap->st_sae_report_ext_auth_worker));
#endif

    if (WLAN_P2P_CL_MODE == mac_vap->en_p2p_mode) {
        return hmac_del_p2p_cl_vap(mac_vap, len, param);
    }

    if (mac_is_secondary_sta(&hmac_vap->st_vap_base_info)) {
        mac_set_dual_sta_mode(OAL_FALSE);
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    /* can't return when dfr process! */
    if ((mac_vap->en_vap_state != MAC_VAP_STATE_INIT) && (g_st_dfr_info.bit_device_reset_process_flag != OAL_TRUE)) {
#else
    if (mac_vap->en_vap_state != MAC_VAP_STATE_INIT) {
#endif
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_vap::deleting vap failed. vap state is not "
                         "INIT, en_vap_state=%d,en_vap_mode=%d}", mac_vap->en_vap_state, mac_vap->en_vap_mode);
        oam_report_backtrace();
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_vap->uc_edca_opt_flag_ap = 0;
        frw_timer_immediate_destroy_timer_m(&(hmac_vap->st_edca_opt_timer));
    } else if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_vap->uc_edca_opt_flag_sta = 0;
    }
#endif

    /* 如果是配置VAP, 去注册配置vap对应的net_device, 释放，返回 */
    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        /* 在注销netdevice之前先将指针赋为空 */
        oal_net_device_stru *pst_net_device = hmac_vap->pst_net_device;
        hmac_vap->pst_net_device = NULL;
        oal_smp_mb();
        oal_net_unregister_netdev(pst_net_device);

        mac_res_free_mac_vap(hmac_vap->st_vap_base_info.uc_vap_id);
        return OAL_SUCC;
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef CONFIG_ARCH_KIRIN_PCIE
    hmac_del_vap_disable_hal_vap(hmac_vap);
#endif
#endif
    /* 业务vap net_device已在WAL释放，此处置为null */
    if (WLAN_P2P_DEV_MODE == del_vap_param->en_p2p_mode) {
        /* 针对p2p0,需要删除hmac 中对应的p2p0 netdevice 指针 */
        hmac_vap->pst_p2p0_net_device = NULL;
    }
    hmac_vap->pst_net_device = NULL;

    /* 业务vap已删除，从device上去掉 */
    mac_dev = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_dev == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_vap::mac_res_get_dev failed.}");
        return OAL_FAIL;
    }

    /* 清理所有的timer */
    hmac_del_vap_destroy_timer(hmac_vap);

    /* 删除vap时删除TCP ACK的队列 */
#ifdef _PRE_WLAN_TCP_OPT
    hmac_tcp_opt_deinit_list(hmac_vap);
#endif

    hmac_user_del_multi_user(mac_vap);

    /* 释放pmksa */
    hmac_config_flush_pmksa(mac_vap, len, param);

    hmac_config_del_scaning_flag(mac_vap);

    mac_vap_exit(&(hmac_vap->st_vap_base_info));

    mac_res_free_mac_vap(hmac_vap->st_vap_base_info.uc_vap_id);

    /***************************************************************************
                          抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_DEL_VAP, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_vap::hmac_config_send_event failed[%d]}", ret);
        // 不退出，保证Devce挂掉的情况下可以下电。
    }

    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_del_vap::Del succ.vap_mode[%d], p2p_mode[%d], multi user idx[%d], device_id[%d]}",
        mac_vap->en_vap_mode, del_vap_param->en_p2p_mode, mac_vap->us_multi_user_idx, mac_vap->uc_device_id);

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        return OAL_SUCC;
    }
#endif  // _PRE_WLAN_FEATURE_DFR

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    ret = hmac_del_vap_post_process(mac_vap);
#endif
    return ret;
}


uint32_t hmac_config_def_chan(mac_vap_stru *pst_mac_vap)
{
    uint8_t uc_channel;
    mac_cfg_mode_param_stru st_param;

    if (((pst_mac_vap->st_channel.en_band == WLAN_BAND_BUTT) ||
        (pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_BUTT) ||
        (pst_mac_vap->en_protocol == WLAN_PROTOCOL_BUTT))
        && (!IS_P2P_GO(pst_mac_vap))) {
        st_param.en_band = WLAN_BAND_2G;
        st_param.en_bandwidth = WLAN_BAND_WIDTH_20M;
        st_param.en_protocol = WLAN_HT_MODE;
        hmac_config_set_mode(pst_mac_vap, sizeof(st_param), (uint8_t *)&st_param);
    }

    if ((pst_mac_vap->st_channel.uc_chan_number == 0) && (!IS_P2P_GO(pst_mac_vap))) {
        pst_mac_vap->st_channel.uc_chan_number = 6; /* 主20MHz信道号 6信道 */
        uc_channel = pst_mac_vap->st_channel.uc_chan_number;
        hmac_config_set_freq(pst_mac_vap, sizeof(uint32_t), &uc_channel);
    }

    return OAL_SUCC;
}

OAL_INLINE void hmac_config_set_ap_bssid_and_spectrum_mgmt(mac_vap_stru *mac_vap)
{
    /* 设置bssid */
    mac_vap_set_bssid(mac_vap, mac_mib_get_StationID(mac_vap));

    /* 入网优化，不同频段下的能力不一样 */
    if (mac_vap->st_channel.en_band == WLAN_BAND_2G) {
        mac_mib_set_SpectrumManagementRequired(mac_vap, OAL_FALSE);
    } else {
        mac_mib_set_SpectrumManagementRequired(mac_vap, OAL_TRUE);
    }
}

OAL_INLINE void hmac_config_start_vap_set_sta_state(mac_vap_stru *mac_vap, hmac_vap_stru *hmac_vap,
                                                    mac_cfg_start_vap_param_stru *start_vap_param)
{
    /* p2p0和p2p-p2p0 共VAP 结构，对于p2p cl不用修改vap 状态 */
    if (start_vap_param->en_p2p_mode != WLAN_P2P_CL_MODE ||
        (start_vap_param->en_p2p_mode == WLAN_P2P_CL_MODE && mac_vap->en_vap_state == MAC_VAP_STATE_INIT)) {
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
#ifndef WIN32
    /* 启动p2p device时，vap_param的p2p模式和mac_vap的p2p模式不同 */
    
    } else if (mac_vap->en_p2p_mode == WLAN_P2P_DEV_MODE) {
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
#endif
    }
}
#ifdef CONFIG_HI110X_SOFT_AP_LIMIT_CPU_FREQ
OAL_STATIC oal_bool_enum_uint8 hmac_need_limit_cpu_freq(mac_vap_stru *mac_vap)
{
    return IS_LEGACY_VAP(mac_vap) && (mac_vap->st_channel.en_band == WLAN_BAND_5G) &&
        (mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS &&
        mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS);
}
#endif
uint32_t hmac_config_set_vap_param_by_vap_mode(mac_vap_stru *mac_vap, hmac_vap_stru *hmac_vap,
                                               mac_cfg_start_vap_param_stru *start_vap_param)
{
    uint8_t *ssid = NULL;

    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        ssid = mac_mib_get_DesiredSSID(mac_vap);
        /* P2P GO 创建后，未设置ssid 信息，设置为up 状态不需要检查ssid 参数 */
        if (OAL_STRLEN((int8_t *)ssid) == 0 && (!IS_P2P_GO(mac_vap))) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_vap_param_by_vap_mode::ssid length=0.}");
            return OAL_FAIL; /* 没设置SSID，则不启动VAP */
        }

        /* 设置AP侧状态机为 WAIT_START */
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_AP_WAIT_START);

        if (IS_LEGACY_VAP(&(hmac_vap->st_vap_base_info))) {
            hmac_config_def_chan(mac_vap);
        }

        /* 这里 en_status 等于 MAC_CHNL_AV_CHK_NOT_REQ(无需检测) 或者 MAC_CHNL_AV_CHK_COMPLETE(检测完成) */
        /* 检查协议 频段 带宽是否设置 */
        if ((mac_vap->st_channel.en_band == WLAN_BAND_BUTT) || (mac_vap->en_protocol == WLAN_PROTOCOL_BUTT) ||
            (mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_BUTT)) {
            if (IS_P2P_GO(mac_vap)) {
                /* wpa_supplicant 会先设置vap up， 此时并未给vap 配置信道、带宽和协议模式信息，
                   wpa_supplicant 在cfg80211_start_ap 接口配置GO 信道、带宽和协议模式信息，
                   故此处如果没有设置信道、带宽和协议模式，直接返回成功，不返回失败。 */
                hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_INIT);
                oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_vap_param_by_vap_mode::set band \
                    bandwidth protocol first band[%d], bw[%d], protocol[%d]}", mac_vap->st_channel.en_band,
                    mac_vap->st_channel.en_bandwidth, mac_vap->en_protocol);
                return OAL_SUCC;
            } else {
                hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_INIT);
                oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                               "{hmac_config_set_vap_param_by_vap_mode::set band bandwidth protocol first.}");
                return OAL_FAIL;
            }
        }

        /* 检查信道号是否设置 */
        if ((mac_vap->st_channel.uc_chan_number == 0) && (!IS_P2P_GO(mac_vap))) {
            hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_INIT);
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_set_vap_param_by_vap_mode::set channel number first.}");
            return OAL_FAIL;
        }

        /* 设置AP的bssid和频谱管理能力 */
        hmac_config_set_ap_bssid_and_spectrum_mgmt(mac_vap);
#ifdef CONFIG_HI110X_SOFT_AP_LIMIT_CPU_FREQ
        hmac_cpu_freq_upper_limit_switch(hmac_need_limit_cpu_freq(mac_vap));
#endif
        /* 设置AP侧状态机为 UP */
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_UP);
    } else if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_config_start_vap_set_sta_state(mac_vap, hmac_vap, start_vap_param);
    } else {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_vap_param_by_vap_mode::Do not surport other mode[%d].}", mac_vap->en_vap_mode);
    }

    return OAL_SUCC_GO_ON;
}


uint32_t hmac_config_start_vap(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *param)
{
    uint32_t ret;
    mac_device_stru *mac_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_cfg_start_vap_param_stru *start_vap_param = (mac_cfg_start_vap_param_stru *)param;
#if defined(_PRE_WLAN_FEATURE_DFS)
    hmac_device_stru *hmac_device;
#endif
    if (oal_unlikely(oal_any_null_ptr2(mac_vap, param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_start_vap::mac_vap or param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::the vap has been deleted.}");
        return OAL_FAIL;
    }

    /* 如果已经在up状态，则返回成功 */
    if ((mac_vap->en_vap_state == MAC_VAP_STATE_UP) || (mac_vap->en_vap_state == MAC_VAP_STATE_AP_WAIT_START) ||
        (mac_vap->en_vap_state == MAC_VAP_STATE_STA_FAKE_UP)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_start_vap::state=%d, duplicate start again}", mac_vap->en_vap_state);
        return OAL_SUCC;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    oal_init_delayed_work(&hmac_vap->st_ampdu_work, (void *)hmac_set_ampdu_worker);
    oal_spin_lock_init(&hmac_vap->st_ampdu_lock);
    oal_init_delayed_work(&hmac_vap->st_set_hw_work, (void *)hmac_set_ampdu_hw_worker);
#endif

    /* 根据vap mode设置vap 参数 */
    ret = hmac_config_set_vap_param_by_vap_mode(mac_vap, hmac_vap, start_vap_param);
    if (ret != OAL_SUCC_GO_ON) {
        return ret;
    }

    mac_vap_init_rates(mac_vap);
    ret = hmac_config_start_vap_event(mac_vap, start_vap_param->en_mgmt_rate_init_flag);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::send event failed[%d].}", ret);
        return ret;
    }

#if defined(_PRE_WLAN_FEATURE_DFS)
    if (IS_AP(mac_vap)) {
        hmac_device = hmac_res_get_mac_dev(mac_device->uc_device_id);
        hmac_dfs_try_cac(hmac_device, mac_vap);
    }
#endif

    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::host start vap ok. event to dmac. vap\
        mode[%d], p2p mode[%d] bw[%d]}", mac_vap->en_vap_mode, mac_vap->en_p2p_mode, mac_vap->st_channel.en_bandwidth);
    return OAL_SUCC;
}


uint32_t hmac_config_sta_update_rates(mac_vap_stru *pst_mac_vap,
                                      mac_cfg_mode_param_stru *pst_cfg_mode, mac_bss_dscr_stru *pst_bss_dscr)
{
    uint32_t ret;
    mac_device_stru *pst_mac_device = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == NULL)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_sta_update_rates::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_sta_update_rates::vap has been deleted.}");

        return OAL_FAIL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_sta_update_rates::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_cfg_mode->en_protocol >= WLAN_HT_MODE) {
        mac_mib_set_TxAggregateActived(&pst_hmac_vap->st_vap_base_info, OAL_TRUE);
        mac_mib_set_AmsduAggregateAtive(pst_mac_vap, OAL_TRUE);
    } else {
        mac_mib_set_TxAggregateActived(&pst_hmac_vap->st_vap_base_info, OAL_FALSE);
        mac_mib_set_AmsduAggregateAtive(pst_mac_vap, OAL_FALSE);
    }

    mac_vap_init_by_protocol(pst_mac_vap, pst_cfg_mode->en_protocol);
    pst_mac_vap->st_channel.en_band = pst_cfg_mode->en_band;
    pst_mac_vap->st_channel.en_bandwidth = pst_cfg_mode->en_bandwidth;

    mac_sta_init_bss_rates(pst_mac_vap, pst_bss_dscr);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_start_vap_event(pst_mac_vap, OAL_FALSE);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_sta_update_rates::hmac_config_send_event failed[%d].}", ret);
        mac_vap_init_by_protocol(pst_mac_vap, pst_hmac_vap->st_preset_para.en_protocol);
        pst_mac_vap->st_channel.en_band = pst_hmac_vap->st_preset_para.en_band;
        pst_mac_vap->st_channel.en_bandwidth = pst_hmac_vap->st_preset_para.en_bandwidth;
        return ret;
    }

    return OAL_SUCC;
}


uint8_t hmac_calc_up_and_wait_vap(hmac_device_stru *pst_hmac_dev)
{
    mac_vap_stru *pst_vap = NULL;
    uint8_t uc_vap_idx;
    uint8_t up_ap_num = 0;
    mac_device_stru *pst_mac_device = NULL;

    if (pst_hmac_dev->pst_device_base_info == NULL) {
        return 0;
    }

    pst_mac_device = pst_hmac_dev->pst_device_base_info;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == NULL) {
            oam_warning_log1(0, OAM_SF_ANY, "vap is null, vap id is %d", pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if (pst_vap->en_vap_state == MAC_VAP_STATE_UP || pst_vap->en_vap_state == MAC_VAP_STATE_AP_WAIT_START) {
            up_ap_num++;
        }
    }

    return up_ap_num;
}


uint32_t hmac_down_vap_kick_all_user(mac_device_stru *mac_device, mac_vap_stru *mac_vap)
{
    oal_dlist_head_stru *entry          = NULL;
    oal_dlist_head_stru *dlist_tmp      = NULL;
    mac_user_stru       *user_tmp       = NULL;
    hmac_user_stru      *hmac_user_tmp  = NULL;
    oal_bool_enum_uint8  is_protected;

    /* 遍历vap下所有用户, 删除用户 */
    oal_dlist_search_for_each_safe(entry, dlist_tmp, &(mac_vap->st_mac_user_list_head))
    {
        user_tmp = oal_dlist_get_entry(entry, mac_user_stru, st_user_dlist);
        if (user_tmp == NULL) {
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::pst_user_tmp null.}");
            continue;
        }

        hmac_user_tmp = mac_res_get_hmac_user(user_tmp->us_assoc_id);
        if (hmac_user_tmp == NULL) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::pst_hmac_user_tmp null.idx:%u}",
                user_tmp->us_assoc_id);
            continue;
        }

        /* 管理帧加密是否开启 */
        is_protected = user_tmp->st_cap_info.bit_pmf_active;

        /* 发去关联帧 */
        hmac_mgmt_send_disassoc_frame(mac_vap, user_tmp->auc_user_mac_addr, MAC_DISAS_LV_SS, is_protected);

        /* 删除用户 */
        hmac_user_del(mac_vap, hmac_user_tmp);
    }

    /* DBDC单GO无用户场景,GO DOWN且另一路VAP正在入网，优先去关联入网状态的VAP */
    if (IS_P2P_GO(mac_vap)) {
        hmac_dbdc_need_kick_user(mac_vap, mac_device);
    }

    /* VAP下user链表应该为空 */
    if (OAL_FALSE == oal_dlist_is_empty(&mac_vap->st_mac_user_list_head)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_down_vap::st_mac_user_list_head is not empty.}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

static void hmac_config_down_vap_ap_mode_handle(mac_vap_stru *mac_vap,
    mac_device_stru *mac_device)
{
    hmac_device_stru *hmac_dev = NULL;
    uint32_t pedding_data = 0;

#ifdef _PRE_WLAN_FEATURE_DFS
    /* 取消 CAC 定时器 */
    hmac_dfs_cac_stop(mac_device, mac_vap);
    hmac_dfs_off_cac_stop(mac_device, mac_vap);
#endif
#ifdef CONFIG_HI110X_SOFT_AP_LIMIT_CPU_FREQ
    if (IS_LEGACY_VAP(mac_vap)) {
        hmac_cpu_freq_upper_limit_switch(OAL_FALSE);
    }
#endif
    hmac_dev = hmac_res_get_mac_dev(mac_device->uc_device_id);
    if (hmac_dev != NULL) {
        if (hmac_dev->en_in_init_scan && hmac_dev->st_scan_mgmt.en_is_scanning) {
            hmac_config_scan_abort(mac_vap, sizeof(uint32_t), (uint8_t *)&pedding_data);
        }
        if (hmac_calc_up_and_wait_vap(hmac_dev) <= 1) {
            hmac_init_scan_cancel_timer(hmac_dev);
            hmac_dev->en_in_init_scan = OAL_FALSE;
        }
    }
}


uint32_t hmac_config_down_vap(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_device = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    mac_cfg_down_vap_param_stru *pst_param = NULL;
    uint32_t ret;
    mac_user_stru *pst_multi_user = NULL;
    mac_vap_state_enum_uint8 vap_state;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_down_vap::pst_mac_vap or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_param = (mac_cfg_down_vap_param_stru *)puc_param;

    if (pst_param->pst_net_dev == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::pst_param->pst_net_dev is null.}");
        return OAL_SUCC;
    }

    /* 如果vap已经在down的状态，直接返回 */
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_INIT) {
        /* 设置net_device里flags标志 */
        if (oal_netdevice_flags(pst_param->pst_net_dev) & OAL_IFF_RUNNING) {
            oal_netdevice_flags(pst_param->pst_net_dev) &= (~OAL_IFF_RUNNING);
        }

        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::vap already down.}");
        return OAL_SUCC;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == NULL)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_multi_user = mac_res_get_mac_user(pst_mac_vap->us_multi_user_idx);
    if (pst_multi_user == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::multi_user[%d] null.}",
                         pst_mac_vap->us_multi_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    oal_cancel_delayed_work_sync(&pst_hmac_vap->st_ampdu_work);
    oal_cancel_delayed_work_sync(&pst_hmac_vap->st_set_hw_work);
#endif

    /* 设置net_device里flags标志 */
    oal_netdevice_flags(pst_param->pst_net_dev) &= (~OAL_IFF_RUNNING);

    if (oal_unlikely(hmac_down_vap_kick_all_user(pst_mac_device, pst_mac_vap) != OAL_SUCC)) {
        return OAL_FAIL;
    }

    /* sta模式时 将desired ssid MIB项置空，并清空配置协议标志 */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        pst_hmac_vap->bit_sta_protocol_cfg = OAL_SWITCH_OFF;
        if (pst_mac_vap->pst_mib_info != NULL) {
            memset_s(mac_mib_get_DesiredSSID(pst_mac_vap), WLAN_SSID_MAX_LEN, 0, WLAN_SSID_MAX_LEN);
        } else {
            oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::mib pointer is NULL!!}");
        }
        oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::sta protocol cfg clear}");
    } else if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_config_down_vap_ap_mode_handle(pst_mac_vap, pst_mac_device);
    }

    /***************************************************************************
                         抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DOWN_VAP, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, 0, "{hmac_config_down_vap::hmac_config_send_event fail[%d].}", ret);
        return ret;
    }

    /* 110x也同步下去,不host device写两遍和staut接口统一 */
    vap_state = (pst_param->en_p2p_mode == WLAN_P2P_CL_MODE) ? MAC_VAP_STATE_STA_SCAN_COMP : MAC_VAP_STATE_INIT;
    hmac_fsm_change_state(pst_hmac_vap, vap_state);
    /*
     * 上层发送GAS REQUEST帧vap进入listen，同时遇到上层netdev stop会将vap状态置为INIT，hmac_mgmt_tx_roc_comp_cb函数会
     * 恢复vap状态为en_last_vap_state，此时vap状态已为INIT，所以此处流程应正确更新last vap state,保证vap状态的正确
     */
    pst_hmac_vap->st_vap_base_info.en_last_vap_state = vap_state;

    if (pst_mac_vap->pst_mib_info != NULL) {
        mac_mib_set_AuthenticationMode(pst_mac_vap, WLAN_WITP_AUTH_OPEN_SYSTEM);
    }
#ifdef _PRE_WLAN_FEATURE_DFS
    hmac_dfs_radar_wait(pst_mac_device, pst_hmac_vap);
#endif

    oam_warning_log2(pst_mac_vap->uc_vap_id, 0, "{hmac_config_down_vap:: SUCC! Now remaining %d vaps in device[%d].}",
                     pst_mac_device->uc_vap_num, pst_mac_device->uc_device_id);
    return OAL_SUCC;
}


uint32_t hmac_config_set_bss_type(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    /* 设置mib值 */
    mac_mib_set_bss_type(pst_mac_vap, (uint8_t)us_len, puc_param);

    return OAL_SUCC;
}


uint32_t hmac_config_get_bss_type(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    /* 读取mib值 */
    return mac_mib_get_bss_type(pst_mac_vap, (uint8_t *)pus_len, puc_param);
}


uint32_t hmac_config_get_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    mac_cfg_mode_param_stru *pst_prot_param;

    pst_prot_param = (mac_cfg_mode_param_stru *)puc_param;

    pst_prot_param->en_protocol = pst_mac_vap->en_protocol;
    pst_prot_param->en_band = pst_mac_vap->st_channel.en_band;
    pst_prot_param->en_bandwidth = pst_mac_vap->st_channel.en_bandwidth;

    *pus_len = sizeof(mac_cfg_mode_param_stru);

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_config_set_mode_check_freq(wlan_channel_band_enum_uint8 en_band_config,
                                                    wlan_channel_band_enum_uint8 en_band_device)
{
    if ((WLAN_BAND_5G == en_band_config) && (WLAN_BAND_CAP_2G == en_band_device)) {
        /* 设置5G频带，但device不支持5G */
        oam_warning_log2(0, OAM_SF_CFG,
                         "{hmac_config_check_mode_param::not support 5GHz band,en_protocol=%d en_protocol_cap=%d.}",
                         en_band_config, en_band_device);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    } else if ((WLAN_BAND_2G == en_band_config) && (WLAN_BAND_CAP_5G == en_band_device)) {
        /* 设置2G频带，但device不支持2G */
        oam_warning_log2(0, OAM_SF_CFG,
                         "{hmac_config_check_mode_param::not support 2GHz band,en_protocol=%d en_protocol_cap=%d.}",
                         en_band_config, en_band_device);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_config_check_dev_protocol_cap_para(
    mac_cfg_mode_param_stru *pst_prot_param, mac_device_stru *pst_mac_device)
{
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
                    "{hmac_config_check_mode_param::not support HT mode,en_protocol=%d en_protocol_cap=%d.}",
                    pst_prot_param->en_protocol, pst_mac_device->en_protocol_cap);
                return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
            }
            break;

        case WLAN_VHT_MODE:
        case WLAN_VHT_ONLY_MODE:
            if (pst_mac_device->en_protocol_cap < WLAN_PROTOCOL_CAP_VHT) {
                /* 设置11ac协议，但device不支持VHT模式 */
                oam_warning_log2(0, OAM_SF_CFG,
                    "{hmac_config_check_mode_param::not support VHT mode,en_protocol=%d en_protocol_cap=%d.}",
                    pst_prot_param->en_protocol, pst_mac_device->en_protocol_cap);
                return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
            }
            break;
#ifdef _PRE_WLAN_FEATURE_11AX
        case WLAN_HE_MODE:
            if (g_wlan_spec_cfg->feature_11ax_is_open) {
                if (pst_mac_device->en_protocol_cap < WLAN_PROTOCOL_CAP_HE) {
                    /* 设置11aX协议，但device不支持HE模式 */
                    oam_error_log2(0, OAM_SF_CFG,
                        "{hmac_config_check_mode_param::not support HE mode,en_protocol=%d en_protocol_cap=%d.}",
                        pst_prot_param->en_protocol, pst_mac_device->en_protocol_cap);
                    return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
                }
                break;
            }
            oam_error_log0(0, OAM_SF_CFG, "{hmac_config_check_mode_param::mode param does not in the list.}");
            break;
#endif

        default:
            oam_error_log0(0, OAM_SF_CFG, "{hmac_config_check_mode_param::mode param does not in the list.}");
            break;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_config_check_mode_param(mac_vap_stru *pst_mac_vap, mac_cfg_mode_param_stru *pst_prot_param)
{
    mac_device_stru *pst_mac_device;

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == NULL)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_check_mode_param::pst_mac_device null.}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    /* 根据device能力对参数进行检查 */
    if (hmac_config_check_dev_protocol_cap_para(pst_prot_param, pst_mac_device) != OAL_SUCC) {
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    if ((mac_vap_bw_mode_to_bw(pst_prot_param->en_bandwidth) >= WLAN_BW_CAP_80M)
        && (mac_mib_get_dot11VapMaxBandWidth(pst_mac_vap) < WLAN_BW_CAP_80M)) {
        /* 设置80M带宽，但device能力不支持80M，返回错误码 */
        oam_warning_log2(0, OAM_SF_CFG,
            "{hmac_config_check_mode_param::not support 80MHz bandwidth,en_protocol=%d en_protocol_cap=%d.}",
            pst_prot_param->en_bandwidth, mac_mib_get_dot11VapMaxBandWidth(pst_mac_vap));
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    return hmac_config_set_mode_check_freq(pst_prot_param->en_band, pst_mac_device->en_band_cap);
}


OAL_STATIC uint32_t hmac_config_set_mode_check_bandwith(wlan_channel_bandwidth_enum_uint8 en_bw_device,
                                                        wlan_channel_bandwidth_enum_uint8 en_bw_config)
{
    /* 要配置带宽是20M */
    if (WLAN_BAND_WIDTH_20M == en_bw_config) {
        return OAL_SUCC;
    }

    /* 要配置带宽与首次配置带宽相同 */
    if (en_bw_device == en_bw_config) {
        return OAL_SUCC;
    }

    switch (en_bw_device) {
        case WLAN_BAND_WIDTH_80PLUSPLUS:
        case WLAN_BAND_WIDTH_80PLUSMINUS:
            if (WLAN_BAND_WIDTH_40PLUS == en_bw_config) {
                return OAL_SUCC;
            }
            break;

        case WLAN_BAND_WIDTH_80MINUSPLUS:
        case WLAN_BAND_WIDTH_80MINUSMINUS:
            if (WLAN_BAND_WIDTH_40MINUS == en_bw_config) {
                return OAL_SUCC;
            }
            break;

        case WLAN_BAND_WIDTH_40PLUS:
            if ((WLAN_BAND_WIDTH_80PLUSPLUS == en_bw_config) || (WLAN_BAND_WIDTH_80PLUSMINUS == en_bw_config)) {
                return OAL_SUCC;
            }
            break;
        case WLAN_BAND_WIDTH_40MINUS:
            if ((WLAN_BAND_WIDTH_80MINUSPLUS == en_bw_config) || (WLAN_BAND_WIDTH_80MINUSMINUS == en_bw_config)) {
                return OAL_SUCC;
            }
            break;
        case WLAN_BAND_WIDTH_20M:
            return OAL_SUCC;

        default:
            break;
    }

    return OAL_FAIL;
}


void hmac_config_set_protocol_and_bw(mac_vap_stru *mac_vap, mac_cfg_mode_param_stru *prot_param)
{
    mac_vap->st_cap_flag.bit_11ac2g = wlan_chip_get_11ac2g_enable();

    if ((mac_vap->st_cap_flag.bit_11ac2g == OAL_FALSE) && (mac_vap->en_protocol == WLAN_VHT_MODE) &&
        (mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_protocol_and_bw::11ac2g not supported.");
        oal_io_print("{hmac_config_set_protocol_and_bw::11ac2g not supported.\n");
        prot_param->en_protocol = WLAN_HT_MODE;
    }

    if ((mac_vap->st_cap_flag.bit_disable_2ght40 == OAL_TRUE) && (prot_param->en_bandwidth != WLAN_BAND_WIDTH_20M) &&
        (mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_protocol_and_bw::2ght40 not supported.");
        oal_io_print("{hmac_config_set_protocol_and_bw::2ght40 not supported.\n");
        prot_param->en_bandwidth = WLAN_BAND_WIDTH_20M;
    }
}


uint32_t hmac_config_set_mode(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *param)
{
    mac_cfg_mode_param_stru *prot_param = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t ret;
    mac_device_stru *mac_device;

    /* 获取device */
    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::mac_device null.}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    /* 设置模式时，device下必须至少有一个vap */
    if (mac_device->uc_vap_num == 0) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::no vap in device.}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    prot_param = (mac_cfg_mode_param_stru *)param;

    /* 检查配置参数是否在device能力内 */
    ret = hmac_config_check_mode_param(mac_vap, prot_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::check_mode_param failed[%d].}", ret);
        return ret;
    }

    /* device已经配置时，需要校验下频段、带宽是否一致 */
    if ((mac_device->en_max_bandwidth != WLAN_BAND_WIDTH_BUTT) && (!MAC_DBAC_ENABLE(mac_device))
        && (mac_device->uc_vap_num > 1)) {
        if (mac_device->en_max_band != prot_param->en_band) {
            oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::previous vap band[%d] \
                mismatch with [%d].}", mac_device->en_max_band, prot_param->en_band);
            return OAL_FAIL;
        }

        ret = hmac_config_set_mode_check_bandwith(mac_device->en_max_bandwidth, prot_param->en_bandwidth);
        if (ret != OAL_SUCC) {
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_set_mode::set_mode_check_bandwith failed[%d],previous vap bandwidth[%d, current[%d].}",
                ret, mac_device->en_max_bandwidth, prot_param->en_bandwidth);
            return ret;
        }
    }

    if (prot_param->en_protocol >= WLAN_HT_MODE) {
        mac_mib_set_TxAggregateActived(&hmac_vap->st_vap_base_info, OAL_TRUE);
        mac_mib_set_AmsduAggregateAtive(mac_vap, OAL_TRUE);
    } else {
        mac_mib_set_TxAggregateActived(&hmac_vap->st_vap_base_info, OAL_FALSE);
        mac_mib_set_AmsduAggregateAtive(mac_vap, OAL_FALSE);
    }

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hmac_config_set_protocol_and_bw(mac_vap, prot_param);
#endif

    /* 更新STA协议配置标志位 */
    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_vap->bit_sta_protocol_cfg = OAL_SWITCH_ON;
        hmac_vap->st_preset_para.en_protocol = prot_param->en_protocol;
        hmac_vap->st_preset_para.en_bandwidth = prot_param->en_bandwidth;
        hmac_vap->st_preset_para.en_band = prot_param->en_band;
    }

    /* 记录协议模式, band, bandwidth到mac_vap下 */
    mac_vap->en_protocol = prot_param->en_protocol;
    mac_vap->st_channel.en_band = prot_param->en_band;
    mac_vap->st_channel.en_bandwidth = prot_param->en_bandwidth;
    mac_vap->st_ch_switch_info.en_user_pref_bandwidth = prot_param->en_bandwidth;

#ifdef _PRE_WLAN_FEATURE_TXBF_HT
    if ((prot_param->en_protocol < WLAN_HT_MODE)
        || (OAL_TRUE != MAC_DEVICE_GET_CAP_SUBFEE(mac_device))) {
        mac_vap->st_cap_flag.bit_11ntxbf = OAL_FALSE;
    }
#endif
    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_mode::protocol=%d, band=%d, bandwidth=%d.}",
                     prot_param->en_protocol, mac_vap->st_channel.en_band, mac_vap->st_channel.en_bandwidth);

    /* 根据协议更新vap能力 */
    mac_vap_init_by_protocol(mac_vap, prot_param->en_protocol);

    /* 更新device的频段及最大带宽信息 */
    if ((mac_device->en_max_bandwidth == WLAN_BAND_WIDTH_BUTT) || (hmac_calc_up_ap_num(mac_device) == 0)) {
        mac_device->en_max_bandwidth = prot_param->en_bandwidth;
        mac_device->en_max_band = prot_param->en_band;
    }

    /***************************************************************************
     抛事件到DMAC层, 配置寄存器
    ***************************************************************************/
    ret = hmac_set_mode_event(mac_vap);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::send_event failed[%d].}", ret);
        return ret;
    }

    return ret;
}


uint32_t hmac_config_set_mac_addr(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_staion_id_param_stru *pst_station_id_param = NULL;
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    uint32_t ret;

    if (pst_mac_vap->pst_mib_info == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_set_mac_addr::vap->mib_info is NULL !}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* P2P 设置MAC 地址mib 值需要区分P2P DEV 或P2P_CL/P2P_GO,P2P_DEV MAC 地址设置到p2p0 MIB 中 */
    pst_station_id_param = (mac_cfg_staion_id_param_stru *)puc_param;
    en_p2p_mode = pst_station_id_param->en_p2p_mode;
    if (en_p2p_mode == WLAN_P2P_DEV_MODE) {
        /* 如果是p2p0 device，则配置MAC 地址到auc_p2p0_dot11StationID 成员中 */
        oal_set_mac_addr(mac_mib_get_p2p0_dot11StationID(pst_mac_vap), pst_station_id_param->auc_station_id);
    } else {
        /* 设置mib值, Station_ID */
        mac_mib_set_station_id(pst_mac_vap, (uint8_t)us_len, puc_param);
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_STATION_ID, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mac_addr::send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_get_wmmswitch(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_get_wmmswitch::wmm switch[%d].}", pst_mac_device->en_wmm);

    /* get wmm_en status */
    *puc_param = pst_mac_device->en_wmm;
    *pus_len = sizeof(int32_t);
    return OAL_SUCC;
}


uint32_t hmac_config_get_vap_wmm_switch(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    if (pst_mac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_get_vap_wmm_switch::wmm switch[%d].}", pst_mac_vap->en_vap_wmm);
    *puc_param = pst_mac_vap->en_vap_wmm;
    *pus_len = sizeof(uint32_t);
    return OAL_SUCC;
}


uint32_t hmac_config_set_vap_wmm_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    if (pst_mac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* set wmm_en status */
    pst_mac_vap->en_vap_wmm = (oal_bool_enum_uint8)*puc_param;
    /* 开关WMM，修改mib信息位中的Qos位 */
    mac_mib_set_dot11QosOptionImplemented(pst_mac_vap, pst_mac_vap->en_vap_wmm);

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_vap_wmm_switch::wmm switch[%d].}", pst_mac_vap->en_vap_wmm);
    return OAL_SUCC;
}


uint32_t hmac_config_get_max_user(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    *((int32_t *)puc_param) = mac_mib_get_MaxAssocUserNums(pst_mac_vap);
    *pus_len = sizeof(int32_t);

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_get_max_user::chip us_user_num_max[%d], us_user_nums_max[%d].}",
                     mac_chip_get_max_asoc_user(pst_mac_vap->uc_chip_id), mac_mib_get_MaxAssocUserNums(pst_mac_vap));

    return OAL_SUCC;
}


uint32_t hmac_config_get_ssid(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    /* 读取mib值 */
    return mac_mib_get_ssid(pst_mac_vap, (uint8_t *)pus_len, puc_param);
}


uint32_t hmac_config_set_ssid(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    /* 设置mib值 */
    mac_mib_set_ssid(pst_mac_vap, (uint8_t)us_len, puc_param);

    return hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SSID, us_len, puc_param);
}


uint32_t hmac_config_set_shpreamble(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /* 设置mib值 */
    mac_mib_set_shpreamble(pst_mac_vap, (uint8_t)us_len, puc_param);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SHORT_PREAMBLE, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_concurrent::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_get_shpreamble(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    /* 读mib值 */
    return mac_mib_get_shpreamble(pst_mac_vap, (uint8_t *)pus_len, puc_param);
}


uint32_t hmac_config_set_shortgi20(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    int32_t l_value;
    uint32_t ret;
    shortgi_cfg_stru shortgi_cfg;

    shortgi_cfg.uc_shortgi_type = SHORTGI_20_CFG_ENUM;
    l_value = *((int32_t *)puc_param);

    if (l_value != 0) {
        shortgi_cfg.uc_enable = OAL_TRUE;
        mac_mib_set_ShortGIOptionInTwentyImplemented(pst_mac_vap, OAL_TRUE);
    } else {
        shortgi_cfg.uc_enable = OAL_FALSE;
        mac_mib_set_ShortGIOptionInTwentyImplemented(pst_mac_vap, OAL_FALSE);
    }

    /* 配置事件的子事件 WLAN_CFGID_SHORTGI 通过新加的接口函数取出关键数据存入skb后通过sdio发出 */
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SHORTGI, SHORTGI_CFG_STRU_LEN, (uint8_t *)&shortgi_cfg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_shortgi20::hmac_config_send_event failed[%u].}", ret);
    }

    return ret;
}


uint32_t hmac_config_set_shortgi40(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    int32_t l_value;
    uint32_t ret;
    shortgi_cfg_stru shortgi_cfg;

    shortgi_cfg.uc_shortgi_type = SHORTGI_40_CFG_ENUM;
    l_value = *((int32_t *)puc_param);

    if (l_value != 0) {
        shortgi_cfg.uc_enable = OAL_TRUE;
        mac_mib_set_ShortGIOptionInFortyImplemented(pst_mac_vap, OAL_TRUE);
    } else {
        shortgi_cfg.uc_enable = OAL_FALSE;
        mac_mib_set_ShortGIOptionInFortyImplemented(pst_mac_vap, OAL_FALSE);
    }

    /* ======================================================================== */
    /* hi1102-cb : Need to send to Dmac via sdio */
    /* 配置事件的子事件 WLAN_CFGID_SHORTGI 通过新加的接口函数取出关键数据存入skb后通过sdio发出 */
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SHORTGI, SHORTGI_CFG_STRU_LEN, (uint8_t *)&shortgi_cfg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_shortgi40::hmac_config_send_event failed[%u].}", ret);
    }
    /* ======================================================================== */
    return ret;
}


uint32_t hmac_config_set_shortgi80(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    int32_t l_value;
    uint32_t ret;
    shortgi_cfg_stru shortgi_cfg;

    shortgi_cfg.uc_shortgi_type = SHORTGI_40_CFG_ENUM;

    l_value = *((int32_t *)puc_param);

    if (l_value != 0) {
        shortgi_cfg.uc_enable = OAL_TRUE;
        mac_mib_set_VHTShortGIOptionIn80Implemented(pst_mac_vap, OAL_TRUE);
    } else {
        shortgi_cfg.uc_enable = OAL_FALSE;
        mac_mib_set_VHTShortGIOptionIn80Implemented(pst_mac_vap, OAL_FALSE);
    }

    /* ======================================================================== */
    /* hi1102-cb : Need to send to Dmac via sdio */
    /* 配置事件的子事件 WLAN_CFGID_SHORTGI 通过新加的接口函数取出关键数据存入skb后通过sdio发出 */
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SHORTGI, SHORTGI_CFG_STRU_LEN, (uint8_t *)&shortgi_cfg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_shortgi80::hmac_config_send_event failed[%u].}", ret);
    }
    /* ======================================================================== */
    return ret;
}


uint32_t hmac_config_get_shortgi20(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    int32_t value;

    value = mac_mib_get_ShortGIOptionInTwentyImplemented(pst_mac_vap);

    *((int32_t *)puc_param) = value;

    *pus_len = sizeof(value);

    return OAL_SUCC;
}


uint32_t hmac_config_get_shortgi40(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    int32_t value;

    value = (int32_t)mac_mib_get_ShortGIOptionInFortyImplemented(pst_mac_vap);

    *((int32_t *)puc_param) = value;

    *pus_len = sizeof(value);

    return OAL_SUCC;
}


uint32_t hmac_config_get_shortgi80(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    int32_t value;

    value = mac_mib_get_VHTShortGIOptionIn80Implemented(pst_mac_vap);

    *((int32_t *)puc_param) = value;

    *pus_len = sizeof(value);

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_MONITOR

uint32_t hmac_config_set_sniffer(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return g_wlan_chip_ops->set_sniffer_config(mac_vap, len, param);
}

uint32_t hmac_config_set_sniffer_1103(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;
    mac_cfg_sniffer_param_stru *cfg_sniffer_param = NULL;

    cfg_sniffer_param = (mac_cfg_sniffer_param_stru *)param;
#ifdef _PRE_WLAN_FEATURE_MONITOR
    if (oal_value_eq_any2(cfg_sniffer_param->uc_sniffer_mode, WLAN_SNIFFER_TRAVEL_CAP_ON, WLAN_SINFFER_ON)) {
        uint32_t pedding_data = 0;
        hmac_config_scan_abort(mac_vap, sizeof(uint32_t), (uint8_t *)&pedding_data);
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_sniffer: in sniffer mode, scan abort!}");
    }
#endif
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_sniffer::set sniffer %d",
        cfg_sniffer_param->uc_sniffer_mode);

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SNIFFER, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_sniffer::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

uint32_t hmac_config_set_sniffer_1106(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;
    mac_cfg_sniffer_param_stru *cfg_sniffer_param = NULL;
    mac_rx_switch_stru  rx_switch = {0};
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);

    if (hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_set_sniffer::pst_hmac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    cfg_sniffer_param = (mac_cfg_sniffer_param_stru *)param;

    if (oal_value_eq_any2(cfg_sniffer_param->uc_sniffer_mode, WLAN_SNIFFER_TRAVEL_CAP_ON, WLAN_SINFFER_ON)) {
        uint32_t pedding_data = 0;
        hmac_config_scan_abort(mac_vap, sizeof(uint32_t), (uint8_t *)&pedding_data);
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_sniffer: in sniffer mode, scan abort!}");
    }

    hmac_device->sniffer_mode = (cfg_sniffer_param->uc_sniffer_mode != 0) ? WLAN_SINFFER_ON : WLAN_SINFFER_OFF;
    if (hmac_device->sniffer_mode != WLAN_SINFFER_OFF) {
        hmac_device->rssi0 = 0;
        hmac_device->rssi1 = 0;
        hmac_device->mgmt_frames_cnt = 0;
        hmac_device->control_frames_cnt = 0;
        hmac_device->data_frames_cnt = 0;
        hmac_device->others_frames_cnt = 0;
    }

    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_sniffer::set sniffer %d.",
        cfg_sniffer_param->uc_sniffer_mode);

    oal_set_mac_addr(hmac_device->monitor_mac_addr, cfg_sniffer_param->auc_mac_addr);

    if (!ether_is_broadcast(cfg_sniffer_param->auc_mac_addr)) {
        hmac_device->addr_filter_on = OAL_TRUE;
    } else {
        hmac_device->addr_filter_on = OAL_FALSE;
    }

    /* 切换rx mode */
    rx_switch.hal_device_id = 0;
    if (hmac_device->sniffer_mode == WLAN_SINFFER_ON) {
        rx_switch.new_rx_mode = HAL_DDR_RX;
    } else {
        rx_switch.new_rx_mode = HAL_RAM_RX;
    }
    if (hmac_config_d2h_rx_mode_syn(mac_vap, len, (uint8_t *)(&rx_switch)) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_sniffer_1106::hmac_config_d2h_rx_mode_syn fail.}");
    }

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SNIFFER, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_sniffer::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

uint32_t hmac_config_set_monitor_mode(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    hmac_device_stru *hmac_device;
    uint32_t ret;

    hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_SCAN,
            "{hmac_config_set_monitor_mode::hmac_res_get_mac_dev fail.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_device->en_monitor_mode = *param;
    if (hmac_device->en_monitor_mode == OAL_TRUE) {
        uint32_t pedding_data = 0;
        hmac_config_scan_abort(mac_vap, sizeof(uint32_t), (uint8_t *)&pedding_data);
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_monitor_mode: in sniffer monitor mode, scan abort!}");
    }

    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
        "{hmac_config_set_monitor_mode::set monitor mode %d", hmac_device->en_monitor_mode);

    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CFG,
        "{hmac_config_set_monitor_mode::mgmt[%d],control[%d],data[%d],others[%d]",
        hmac_device->mgmt_frames_cnt, hmac_device->control_frames_cnt, hmac_device->data_frames_cnt,
        hmac_device->others_frames_cnt);

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_MONITOR_MODE, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_monitor_mode::send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_get_monitor(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_monitor::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *((int32_t *)puc_param) = pst_hmac_vap->en_monitor_mode;
    *pus_len = sizeof(int32_t);
    return OAL_SUCC;
}
#endif


uint32_t hmac_config_set_prot_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    int32_t l_value;
    uint32_t ret;

    l_value = *((int32_t *)puc_param);

    if (oal_unlikely(l_value >= WLAN_PROT_BUTT)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_prot_mode::invalid l_value[%d].}", l_value);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_mac_vap->st_protection.en_protection_mode = (uint8_t)l_value;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PROT_MODE, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_prot_mode::send_event failed[%d]}", ret);
    }

    return ret;
}


uint32_t hmac_config_get_prot_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    *((int32_t *)puc_param) = pst_mac_vap->st_protection.en_protection_mode;
    *pus_len = sizeof(int32_t);

    return OAL_SUCC;
}


uint32_t hmac_config_set_auth_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_mib_set_AuthenticationMode(pst_mac_vap, *puc_param);

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
        "{hmac_config_set_auth_mode::set auth mode[%d] succ.}", mac_mib_get_AuthenticationMode(pst_mac_vap));
    return OAL_SUCC;
}


uint32_t hmac_config_get_auth_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    *((int32_t *)puc_param) = mac_mib_get_AuthenticationMode(pst_mac_vap);
    *pus_len = sizeof(int32_t);

    return OAL_SUCC;
}


uint32_t hmac_config_set_max_user(mac_vap_stru *mac_vap, uint32_t max_user_num)
{
    uint32_t p2p_max_user_num = hmac_get_p2p_max_user();
    uint32_t max_assoc_num = mac_chip_get_max_asoc_user(mac_vap->uc_chip_id);
    /* P2P GO最大用户数不能超过p2p限制，普通模式不能超过芯片最大用户数约束 */
    if ((IS_P2P_GO(mac_vap) && (max_user_num > p2p_max_user_num)) || (max_user_num > max_assoc_num) ||
        (max_user_num == 0)) {
        oam_error_log2(mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_set_max_user::the max_user_value[%d] out of range [0 - %d]!}",
                       max_user_num, (IS_P2P_GO(mac_vap) ? p2p_max_user_num : max_assoc_num));
        return OAL_FAIL;
    }

    mac_mib_set_MaxAssocUserNums(mac_vap, (uint16_t)max_user_num);

    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_max_user::chip us_user_num_max[%d], us_user_nums_max[%d].}",
                     max_assoc_num, mac_mib_get_MaxAssocUserNums(mac_vap));

    return OAL_SUCC;
}


uint32_t hmac_config_set_bintval(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_device_stru *pst_mac_device;
    uint8_t uc_vap_idx;
    mac_vap_stru *pst_vap = NULL;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_config_set_bintval::mac_res_get_dev fail.device_id = %u}",
                       pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置device下的值 */
    mac_device_set_beacon_interval(pst_mac_device, *((uint32_t *)puc_param));

    /* 遍历device下所有vap */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == NULL) {
            oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_config_set_bintval::pst_mac_vap(%d) null.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        /* 只有AP VAP需要beacon interval */
        if (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
            /* 设置mib值 */
            mac_mib_set_beacon_period(pst_vap, (uint8_t)us_len, puc_param);
        }
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_BEACON_INTERVAL, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_bintval::send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_get_bintval(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    /* 读取mib值 */
    return mac_mib_get_beacon_period(pst_mac_vap, (uint8_t *)pus_len, puc_param);
}


uint32_t hmac_config_set_dtimperiod(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    /* 设置mib值 */
    mac_mib_set_dtim_period(pst_mac_vap, (uint8_t)us_len, puc_param);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DTIM_PERIOD, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_bintval::send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_get_dtimperiod(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    /* 读取mib值 */
    return mac_mib_get_dtim_period(pst_mac_vap, (uint8_t *)pus_len, puc_param);
}


uint32_t hmac_config_set_nobeacon(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    uint32_t ret;
    int32_t l_value;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_nobeacon::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    l_value = *((int32_t *)puc_param);
    pst_hmac_vap->en_no_beacon = (uint8_t)l_value;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_NO_BEACON, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_nobeacon::send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_get_nobeacon(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_nobeacon::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *((int32_t *)puc_param) = pst_hmac_vap->en_no_beacon;
    *pus_len = sizeof(int32_t);

    return OAL_SUCC;
}


uint32_t hmac_config_set_txpower(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    int32_t l_value;
    uint8_t uc_value;
    uint32_t ret;

    l_value = (*((int32_t *)puc_param) < 0) ? 0 : (*((int32_t *)puc_param));

    uc_value = (uint8_t)((l_value + 5) / 10); /* 加5除以10 四舍五入 */
    mac_vap_set_tx_power(pst_mac_vap, uc_value);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TX_POWER, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_txpower::send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_get_txpower(mac_vap_stru *mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    *((int32_t *)puc_param) = mac_vap->uc_tx_power;
    *pus_len = sizeof(int32_t);

    return OAL_SUCC;
}


uint32_t hmac_config_set_freq(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint8_t channel = *puc_param;
    mac_device_stru *mac_device;
    mac_cfg_channel_param_stru channel_param = {0};

    /* 获取device */
    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_freq::mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = mac_is_channel_num_valid(mac_vap->st_channel.en_band, channel, mac_vap->st_channel.ext6g_band);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_freq::channel num[%d] invalid.}", channel);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

#ifdef _PRE_WLAN_FEATURE_11D
    /* 信道14特殊处理，只在11b协议模式下有效 */
    if ((channel == 14) && (mac_vap->en_protocol != WLAN_LEGACY_11B_MODE)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_freq::channel-14 only available in 11b, curr protocol=%d.}", mac_vap->en_protocol);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
#endif

    mac_vap->st_channel.uc_chan_number = channel;
    ret = mac_get_channel_idx_from_num(mac_vap->st_channel.en_band, channel,
        mac_vap->st_channel.ext6g_band, &(mac_vap->st_channel.uc_chan_idx));
    if (ret != OAL_SUCC) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_freq::mac_get_channel_idx_from_num fail. \
            band[%u] channel[%u]!}", mac_vap->st_channel.en_band, mac_vap->st_channel.uc_chan_idx);
        return ret;
    }

    /* 非DBAC时，首次配置信道时设置到硬件 */
    if (mac_device->uc_vap_num == 1 || mac_device->uc_max_channel == 0) {
        mac_device_get_channel(mac_device, &channel_param);
        channel_param.uc_channel = channel;
        mac_device_set_channel(mac_device, &channel_param);
        wlan_chip_update_cur_chn_cali_data(mac_vap, WLAN_NORMAL_CHN);

        /***************************************************************************
            抛事件到DMAC层, 同步DMAC数据
        ***************************************************************************/
        ret = hmac_config_send_event(mac_vap, WLAN_CFGID_CURRENT_CHANEL, us_len, puc_param);
        if (oal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_freq::send_event failed[%d].}", ret);
            return ret;
        }
    } else if (mac_is_dbac_enabled(mac_device)) {
        wlan_chip_update_cur_chn_cali_data(mac_vap, WLAN_DBDC_CHN);

        /***************************************************************************
            抛事件到DMAC层, 同步DMAC数据
        ***************************************************************************/
        ret = hmac_config_send_event(mac_vap, WLAN_CFGID_CURRENT_CHANEL, us_len, puc_param);
        if (oal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_freq::send_event failed[%d].}", ret);
            return ret;
        }
    } else {
        if (mac_device->uc_max_channel != channel) {
            oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_freq::previous vap channel number=[%d] \
                mismatch [%d].}", mac_device->uc_max_channel, channel);

            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}


uint32_t hmac_config_get_freq(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    hmac_device_stru *pst_hmac_dev;

    *pus_len = sizeof(uint32_t);
    pst_hmac_dev = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_dev != NULL && pst_hmac_dev->en_in_init_scan) {
        *((uint32_t *)puc_param) = 0;
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_get_freq::get channel while init scan, retry please}");
        return OAL_SUCC;
    }

    *((uint32_t *)puc_param) = pst_mac_vap->st_channel.uc_chan_number;

    return OAL_SUCC;
}

static uint32_t hmac_wmm_params_set_EDCATableCWmin(mac_vap_stru *mac_vap, uint32_t ac, uint32_t value)
{
    uint32_t ret = OAL_SUCC;
    /*lint -e685*/ /*lint -e568*/
    if ((value > WLAN_QEDCA_TABLE_CWMIN_MAX) || (value < WLAN_QEDCA_TABLE_CWMIN_MIN)) {
        ret = OAL_FAIL;
    }
    mac_mib_set_EDCATableCWmin(mac_vap, (uint8_t)ac, value);
    return ret;
}

static uint32_t hmac_wmm_params_set_EDCATableCWmax(mac_vap_stru *mac_vap, uint32_t ac, uint32_t value)
{
    uint32_t ret = OAL_SUCC;
    if ((value > WLAN_QEDCA_TABLE_CWMAX_MAX) || (value < WLAN_QEDCA_TABLE_CWMAX_MIN)) {
        ret = OAL_FAIL;
    }

    mac_mib_set_EDCATableCWmax(mac_vap, (uint8_t)ac, value);
    return ret;
}

static uint32_t hmac_wmm_params_set_EDCATableAIFSN(mac_vap_stru *mac_vap, uint32_t ac, uint32_t value)
{
    uint32_t ret = OAL_SUCC;
    if ((value < WLAN_QEDCA_TABLE_AIFSN_MIN) || (value > WLAN_QEDCA_TABLE_AIFSN_MAX)) {
        ret = OAL_FAIL;
    }
    /*lint +e685*/ /*lint +e568*/
    mac_mib_set_EDCATableAIFSN(mac_vap, (uint8_t)ac, value);
    return ret;
}

static uint32_t hmac_wmm_params_set_EDCATableTXOPLimit(mac_vap_stru *mac_vap, uint32_t ac, uint32_t value)
{
    uint32_t ret = OAL_SUCC;
    if (value > WLAN_QEDCA_TABLE_TXOP_LIMIT_MAX) {
        ret = OAL_FAIL;
    }

    mac_mib_set_EDCATableTXOPLimit(mac_vap, (uint8_t)ac, value);
    return ret;
}

static uint32_t hmac_wmm_params_set_EDCATableMandatory(mac_vap_stru *mac_vap, uint32_t ac, uint32_t value)
{
    uint32_t ret = OAL_SUCC;
    if ((value != OAL_TRUE) && (value != OAL_FALSE)) {
        ret = OAL_FAIL;
    }
    mac_mib_set_EDCATableMandatory(mac_vap, (uint8_t)ac, (uint8_t)value);
    return ret;
}

static uint32_t hmac_wmm_params_set_QAPEDCATableCWmin(mac_vap_stru *mac_vap, uint32_t ac, uint32_t value)
{
    uint32_t ret = OAL_SUCC;
    /*lint -e685*/ /*lint -e568*/
    if ((value > WLAN_QEDCA_TABLE_CWMIN_MAX) || (value < WLAN_QEDCA_TABLE_CWMIN_MIN)) {
        ret = OAL_FAIL;
    }
    mac_mib_set_QAPEDCATableCWmin(mac_vap, (uint8_t)ac, value);
    return ret;
}

static uint32_t hmac_wmm_params_set_QAPEDCATableCWmax(mac_vap_stru *mac_vap, uint32_t ac, uint32_t value)
{
    uint32_t ret = OAL_SUCC;
    if ((value > WLAN_QEDCA_TABLE_CWMAX_MAX) || (value < WLAN_QEDCA_TABLE_CWMAX_MIN)) {
        ret = OAL_FAIL;
    }

    mac_mib_set_QAPEDCATableCWmax(mac_vap, (uint8_t)ac, value);
    return ret;
}

static uint32_t hmac_wmm_params_set_QAPEDCATableAIFSN(mac_vap_stru *mac_vap, uint32_t ac, uint32_t value)
{
    uint32_t ret = OAL_SUCC;
    if ((value < WLAN_QEDCA_TABLE_AIFSN_MIN) || (value > WLAN_QEDCA_TABLE_AIFSN_MAX)) {
        ret = OAL_FAIL;
    }
    /*lint +e685*/ /*lint +e568*/

    mac_mib_set_QAPEDCATableAIFSN(mac_vap, (uint8_t)ac, value);
    return ret;
}

static uint32_t hmac_wmm_params_set_QAPEDCATableTXOPLimit(mac_vap_stru *mac_vap, uint32_t ac, uint32_t value)
{
    uint32_t ret = OAL_SUCC;
    if (value > WLAN_QEDCA_TABLE_TXOP_LIMIT_MAX) {
        ret = OAL_FAIL;
    }

    mac_mib_set_QAPEDCATableTXOPLimit(mac_vap, (uint8_t)ac, value);
    return ret;
}

static uint32_t hmac_wmm_params_set_QAPEDCATableMSDULifetime(mac_vap_stru *mac_vap, uint32_t ac, uint32_t value)
{
    uint32_t ret = OAL_SUCC;
    if (value > WLAN_QEDCA_TABLE_MSDU_LIFETIME_MAX) {
        ret = OAL_FAIL;
    }
    mac_mib_set_QAPEDCATableMSDULifetime(mac_vap, (uint8_t)ac, value);
    return ret;
}

static uint32_t hmac_wmm_params_set_QAPEDCATableMandatory(mac_vap_stru *mac_vap, uint32_t ac, uint32_t value)
{
    uint32_t ret = OAL_SUCC;
    if ((value != OAL_TRUE) && (value != OAL_FALSE)) {
        ret = OAL_FAIL;
    }

    mac_mib_set_QAPEDCATableMandatory(mac_vap, (uint8_t)ac, (uint8_t)value);
    return ret;
}
typedef struct {
    uint8_t cfg_id;
    uint32_t (*hmac_set_wmm_params_case)(mac_vap_stru *mac_vap, uint32_t ac, uint32_t value);
} hmac_set_wmm_params_ops;

uint32_t hmac_config_set_wmm_params(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint32_t ac;
    uint32_t value;
    wlan_cfgid_enum_uint16 en_cfg_id;
    hmac_config_wmm_para_stru *pst_cfg_stru = NULL;
    uint8_t idx;
    const hmac_set_wmm_params_ops hmac_set_wmm_params_ops_table[] = {
        { WLAN_CFGID_EDCA_TABLE_CWMIN,          hmac_wmm_params_set_EDCATableCWmin },
        { WLAN_CFGID_EDCA_TABLE_CWMAX,          hmac_wmm_params_set_EDCATableCWmax },
        { WLAN_CFGID_EDCA_TABLE_AIFSN,          hmac_wmm_params_set_EDCATableAIFSN },
        { WLAN_CFGID_EDCA_TABLE_TXOP_LIMIT,     hmac_wmm_params_set_EDCATableTXOPLimit },
        { WLAN_CFGID_EDCA_TABLE_MANDATORY,      hmac_wmm_params_set_EDCATableMandatory },
        { WLAN_CFGID_QEDCA_TABLE_CWMIN,         hmac_wmm_params_set_QAPEDCATableCWmin },
        { WLAN_CFGID_QEDCA_TABLE_CWMAX,         hmac_wmm_params_set_QAPEDCATableCWmax },
        { WLAN_CFGID_QEDCA_TABLE_AIFSN,         hmac_wmm_params_set_QAPEDCATableAIFSN },
        { WLAN_CFGID_QEDCA_TABLE_TXOP_LIMIT,    hmac_wmm_params_set_QAPEDCATableTXOPLimit },
        { WLAN_CFGID_QEDCA_TABLE_MSDU_LIFETIME, hmac_wmm_params_set_QAPEDCATableMSDULifetime },
        { WLAN_CFGID_QEDCA_TABLE_MANDATORY,     hmac_wmm_params_set_QAPEDCATableMandatory },
    };

    pst_cfg_stru = (hmac_config_wmm_para_stru *)puc_param;
    en_cfg_id = (uint16_t)pst_cfg_stru->cfg_id;
    ac = pst_cfg_stru->ac;
    value = pst_cfg_stru->value;

    ret = OAL_SUCC;
    if (ac >= WLAN_WME_AC_BUTT) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wmm_params::invalid param, \
                         en_cfg_id=%d, ac=%d, value=%d.}", en_cfg_id, ac, value);
        return OAL_FAIL;
    }

    /* 根据sub-ioctl id填写WID */
    for (idx = 0; idx < (sizeof(hmac_set_wmm_params_ops_table) / sizeof(hmac_set_wmm_params_ops)); ++idx) {
        if (hmac_set_wmm_params_ops_table[idx].cfg_id == en_cfg_id) {
            ret = hmac_set_wmm_params_ops_table[idx].hmac_set_wmm_params_case(mac_vap, ac, value);
            break;
        }
    }
    if (idx == (sizeof(hmac_set_wmm_params_ops_table) / sizeof(hmac_set_wmm_params_ops))) {
        ret = OAL_FAIL;
    }
    if (ret == OAL_FAIL) {
        return ret;
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, en_cfg_id, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wmm_params::send event fail[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_get_wmm_params(mac_vap_stru *mac_vap, uint8_t *puc_param)
{
    uint32_t ac;
    uint32_t value;
    wlan_cfgid_enum_uint16 en_cfg_id;
    hmac_config_wmm_para_stru *pst_cfg_stru;

    value = 0xFFFFFFFF;
    pst_cfg_stru = (hmac_config_wmm_para_stru *)puc_param;
    en_cfg_id = (uint16_t)pst_cfg_stru->cfg_id;
    ac = pst_cfg_stru->ac;

    if (ac >= WLAN_WME_AC_BUTT) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_wmm_params::invalid param, \
                         en_cfg_id=%d, ac=%d, value=%d.}", en_cfg_id, ac, value);
        return value;
    }

    switch (en_cfg_id) { /* 根据sub-ioctl id填写WID */
        case WLAN_CFGID_EDCA_TABLE_CWMIN:
            value = mac_mib_get_EDCATableCWmin(mac_vap, (uint8_t)ac);
            break;

        case WLAN_CFGID_EDCA_TABLE_CWMAX:
            value = mac_mib_get_EDCATableCWmax(mac_vap, (uint8_t)ac);
            break;

        case WLAN_CFGID_EDCA_TABLE_AIFSN:
            value = mac_mib_get_EDCATableAIFSN(mac_vap, (uint8_t)ac);
            break;

        case WLAN_CFGID_EDCA_TABLE_TXOP_LIMIT:
            value = mac_mib_get_EDCATableTXOPLimit(mac_vap, (uint8_t)ac);
            break;

        case WLAN_CFGID_EDCA_TABLE_MANDATORY:
            value = mac_mib_get_EDCATableMandatory(mac_vap, (uint8_t)ac);
            break;

        case WLAN_CFGID_QEDCA_TABLE_CWMIN:
            value = mac_mib_get_QAPEDCATableCWmin(mac_vap, (uint8_t)ac);
            break;

        case WLAN_CFGID_QEDCA_TABLE_CWMAX:
            value = mac_mib_get_QAPEDCATableCWmax(mac_vap, (uint8_t)ac);
            break;

        case WLAN_CFGID_QEDCA_TABLE_AIFSN:
            value = mac_mib_get_QAPEDCATableAIFSN(mac_vap, (uint8_t)ac);
            break;

        case WLAN_CFGID_QEDCA_TABLE_TXOP_LIMIT:
            value = mac_mib_get_QAPEDCATableTXOPLimit(mac_vap, (uint8_t)ac);
            break;

        case WLAN_CFGID_QEDCA_TABLE_MSDU_LIFETIME:
            value = mac_mib_get_QAPEDCATableMSDULifetime(mac_vap, (uint8_t)ac);
            break;

        case WLAN_CFGID_QEDCA_TABLE_MANDATORY:
            value = mac_mib_get_QAPEDCATableMandatory(mac_vap, (uint8_t)ac);
            break;

        default:
            break;
    }

    return value;
}


uint32_t hmac_config_get_uapsden(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    *puc_param = mac_vap_get_uapsd_en(pst_mac_vap);
    *pus_len = sizeof(uint8_t);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_DFT_STAT

uint32_t hmac_config_usr_queue_stat(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return OAL_SUCC;
}

#endif


uint32_t hmac_config_reset_hw(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_RESET_HW, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_reset_hw::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

uint32_t hmac_config_set_reset_state(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret = OAL_SUCC;
    mac_reset_sys_stru *pst_reset_sys = NULL;
    hmac_device_stru *pst_hmac_device;

    pst_reset_sys = (mac_reset_sys_stru *)puc_param;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
                       "{hmac_config_set_reset_state::pst_hmac_device[%d] is null.}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    mac_device_set_dfr_reset(pst_hmac_device->pst_device_base_info, pst_reset_sys->uc_value);

    /* 待整改，当前存在DMAC TO HMAC SYNC，待处理。处理后做抛事件处理 */
    return ret;
}


OAL_STATIC uint8_t hmac_set_channel_160m_invalid(mac_vap_stru *pst_mac_vap, mac_cfg_channel_param_stru *pst_prot_param)
{
    hmac_device_stru *pst_hmac_device;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hmac_set_channel_160m_invalid::pst_hmac_device null,divice_id=%d.}", pst_mac_vap->uc_device_id);
        return OAL_TRUE;
    }

    if ((pst_prot_param->en_band == WLAN_BAND_5G) &&
        (pst_prot_param->en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS) &&
        (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) && (pst_hmac_device->en_ap_support_160m == OAL_FALSE)) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_set_channel_160m_invalid::aput not support 160M");
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


OAL_STATIC uint32_t hmac_config_set_channel_check_param(mac_vap_stru *pst_mac_vap,
                                                        mac_cfg_channel_param_stru *pst_prot_param)
{
    mac_device_stru *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
                       "{hmac_config_set_channel_check_param::pst_mac_device null,divice_id=%d.}",
                       pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 根据device能力对参数进行检查 */
    if ((pst_prot_param->en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS)
        && (mac_mib_get_dot11VapMaxBandWidth(pst_mac_vap) < WLAN_BW_CAP_80M)) {
        /* 设置80M带宽，但device能力不支持80M，返回错误码 */
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_channel_check_param::"
                         "not support 80MHz bandwidth,en_protocol=%d en_dot11VapMaxBandWidth=%d.}",
                         pst_prot_param->en_bandwidth, mac_mib_get_dot11VapMaxBandWidth(pst_mac_vap));
        return OAL_ERR_CODE_CONFIG_BW_EXCEED;
    }

    if ((WLAN_BAND_5G == pst_prot_param->en_band) && (WLAN_BAND_CAP_2G == pst_mac_device->en_band_cap)) {
        /* 设置5G频带，但device不支持5G */
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_channel_check_param::"
                         "not support 5GHz band,en_protocol=%d en_protocol_cap=%d.}",
                         pst_prot_param->en_band, pst_mac_device->en_band_cap);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    } else if ((WLAN_BAND_2G == pst_prot_param->en_band) && (WLAN_BAND_CAP_5G == pst_mac_device->en_band_cap)) {
        /* 设置2G频带，但device不支持2G */
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_channel_check_param::"
                         "not support 2GHz band,en_protocol=%d en_protocol_cap=%d.}",
                         pst_prot_param->en_band, pst_mac_device->en_band_cap);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    /* 若vap不支持2g 40M则，返回不支持该带宽的错误码 */
    if ((WLAN_BAND_2G == pst_prot_param->en_band) && (WLAN_BAND_WIDTH_20M < pst_prot_param->en_bandwidth) &&
        (OAL_FALSE == mac_mib_get_2GFortyMHzOperationImplemented(pst_mac_vap))) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /* 不支持160M，但是上层下发160M带宽 */
    if (OAL_TRUE == hmac_set_channel_160m_invalid(pst_mac_vap, pst_prot_param)) {
        return OAL_ERR_CODE_CONFIG_BW_EXCEED;
    }

    return OAL_SUCC;
}

uint32_t hmac_config_set_mib_by_bw(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_mib_by_bw_param_stru *pst_cfg = (mac_cfg_mib_by_bw_param_stru *)puc_param;
    uint32_t ret;

    if ((pst_mac_vap == NULL) || (puc_param == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_mib_by_bw::input params null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap_change_mib_by_bandwidth(pst_mac_vap, pst_cfg->en_band, pst_cfg->en_bandwidth);

    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_CFG80211_SET_MIB_BY_BW, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_mib_by_bw::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}


void hmac_config_chan_bw_exceed(mac_vap_stru *pst_mac_vap, mac_cfg_channel_param_stru *pst_channel_param)
{
    if (WLAN_BAND_WIDTH_160PLUSPLUSPLUS <= pst_channel_param->en_bandwidth) {
        pst_channel_param->en_bandwidth = mac_vap_get_bandwith(WLAN_BW_CAP_80M,
                                                               pst_channel_param->en_bandwidth);
    } else {
        pst_channel_param->en_bandwidth = mac_vap_get_bandwith(mac_mib_get_dot11VapMaxBandWidth(pst_mac_vap),
                                                               pst_channel_param->en_bandwidth);
    }
}


void hmac_config_go_not_start_160m_handle(mac_vap_stru *p_go_mac_vap,
    uint8_t go_channel, wlan_channel_bandwidth_enum_uint8 *p_bandwidth)
{
#ifdef _PRE_WLAN_FEATURE_160M
    wlan_channel_bandwidth_enum_uint8 new_80bw;
    wlan_bw_cap_enum_uint8 bw_cap;
    mac_cfg_mib_by_bw_param_stru st_cfg;

    new_80bw = mac_vap_bw_mode_to_bw(*p_bandwidth);
    if (!IS_P2P_GO(p_go_mac_vap) || new_80bw != WLAN_BW_CAP_160M) {
        return;
    }

    /* 只有GO 且 160M 带宽时才处理 */
    if (OAL_TRUE == mac_vap_go_can_not_in_160m_check(p_go_mac_vap, go_channel)) {
        new_80bw = mac_regdomain_get_support_bw_mode(WLAN_BW_CAP_80M, go_channel);
        bw_cap = mac_vap_bw_mode_to_bw(new_80bw);
        if (bw_cap == WLAN_BW_CAP_80M) {
            oam_warning_log2(0, OAM_SF_QOS, "{hmac_config_go_not_start_160m_handle::\
                bw from [%d] change to [%d]}", *p_bandwidth, new_80bw);
            *p_bandwidth = new_80bw;

            st_cfg.en_band = p_go_mac_vap->st_channel.en_band;
            st_cfg.en_bandwidth = *p_bandwidth;
            hmac_config_set_mib_by_bw(p_go_mac_vap, (uint16_t)sizeof(st_cfg), (uint8_t *)&st_cfg);
        }
    }
#endif
}

uint32_t hmac_config_set_channel_check(mac_vap_stru *mac_vap, mac_cfg_channel_param_stru *channel_param)
{
    mac_channel_stru st_set_mac_channel;
    uint8_t ap_follow_channel = 0;
    uint32_t ret;

    /* 信道跟随检查 */
    if (IS_AP(mac_vap)) {
        st_set_mac_channel.en_band = channel_param->en_band;
        st_set_mac_channel.en_bandwidth = channel_param->en_bandwidth;
        st_set_mac_channel.uc_chan_number = channel_param->uc_channel;
        ret = hmac_check_ap_channel_follow_sta(mac_vap, &st_set_mac_channel, &ap_follow_channel);
        if (ret == OAL_SUCC) {
            channel_param->uc_channel = ap_follow_channel;
            oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CHAN, "{hmac_config_set_channel::channel from %d change to %d}",
                             st_set_mac_channel.uc_chan_number, ap_follow_channel);
        }
    }

    /* 检查配置参数是否在device能力内 */
    ret = hmac_config_set_channel_check_param(mac_vap, channel_param);
    if (ret == OAL_ERR_CODE_CONFIG_BW_EXCEED) {
        hmac_config_chan_bw_exceed(mac_vap, channel_param);
    }

    if (ret == OAL_ERR_CODE_CONFIG_EXCEED_SPEC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CHAN, "{hmac_config_set_channel::channel check failed[%d].}", ret);
        return ret;
    }

    /* 该错误码表示不支持2g 40M,故更改带宽为20M */
    if (ret == OAL_ERR_CODE_CONFIG_UNSUPPORT) {
        channel_param->en_bandwidth = WLAN_BAND_WIDTH_20M;
    }

    return OAL_SUCC;
}
uint32_t hmac_config_set_channel_check_band_and_bandwidth(mac_device_stru *mac_device, mac_vap_stru *mac_vap,
    mac_cfg_channel_param_stru *channel_param, uint32_t up_vap_cnt, oal_bool_enum_uint8 *en_set_reg)
{
    uint32_t ret;

    if (up_vap_cnt <= 1) {
        /* 记录首次配置的带宽值 */
        mac_device_set_channel(mac_device, channel_param);

        /***************************************************************************
         抛事件到DMAC层, 配置寄存器  置标志位
        ***************************************************************************/
        *en_set_reg = OAL_TRUE;
    } else if (mac_is_dbac_enabled(mac_device) == OAL_TRUE) {
        /* 开启DBAC不进行信道判断 */
        /* 信道设置只针对AP模式，非AP模式则跳出 */
    } else {
        /* 信道不是当前信道 */
        if (mac_device->uc_max_channel != channel_param->uc_channel) {
            oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CHAN, "{hmac_config_set_channel::previous vap channel"
                "number=%d mismatch [%d].}", mac_device->uc_max_channel, channel_param->uc_channel);

            return OAL_FAIL;
        }

        /* 带宽不能超出已配置的带宽 */
        ret = hmac_config_set_mode_check_bandwith(mac_device->en_max_bandwidth, channel_param->en_bandwidth);
        if (ret != OAL_SUCC) {
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CHAN, "{hmac_config_set_channel:: \
                hmac_config_set_mode_check_bandwith failed[%d], previous vap bandwidth[%d, current[%d].}",
                ret, mac_device->en_max_bandwidth, channel_param->en_bandwidth);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

uint32_t hmac_config_set_channel_set_band_and_bandwidth(mac_device_stru *mac_device, mac_vap_stru *mac_vap,
    mac_cfg_channel_param_stru *channel_param, oal_bool_enum_uint8 *en_set_reg)
{
    mac_vap_stru *mac_vap_tmp = NULL;
    uint32_t ret;
    uint8_t vap_idx;

    if (mac_is_dbac_enabled(mac_device) == OAL_TRUE) {
        mac_vap->st_channel.uc_chan_number = channel_param->uc_channel;
        mac_vap->st_channel.en_band = channel_param->en_band;
        mac_vap->st_channel.en_bandwidth = channel_param->en_bandwidth;
        mac_vap->st_channel.ext6g_band = channel_param->ext6g_band;

        ret = mac_get_channel_idx_from_num(channel_param->en_band, channel_param->uc_channel,
                                           channel_param->ext6g_band, &(mac_vap->st_channel.uc_chan_idx));
        if (ret != OAL_SUCC) {
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CHAN,
                "{hmac_config_set_channel::mac_get_channel_idx_from_num failed[%d], band[%d], channel[%d].}",
                ret, channel_param->en_band, channel_param->uc_channel);
            return OAL_FAIL;
        }

        *en_set_reg = OAL_TRUE;
    } else {
        for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
            mac_vap_tmp = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
            if (mac_vap_tmp == NULL) {
                continue;
            }
            mac_vap_tmp->st_channel.uc_chan_number = channel_param->uc_channel;
            mac_vap_tmp->st_channel.en_band = channel_param->en_band;
            mac_vap_tmp->st_channel.en_bandwidth = channel_param->en_bandwidth;
            mac_vap_tmp->st_channel.ext6g_band = channel_param->ext6g_band;
            ret = mac_get_channel_idx_from_num(channel_param->en_band, channel_param->uc_channel,
                channel_param->ext6g_band, &(mac_vap_tmp->st_channel.uc_chan_idx));
            if (ret != OAL_SUCC) {
                oam_warning_log3(mac_vap_tmp->uc_vap_id, OAM_SF_CHAN,
                    "{hmac_config_set_channel::mac_get_channel_idx_from_num failed[%d], band[%d], channel[%d].}",
                    ret, channel_param->en_band, channel_param->uc_channel);
                continue;
            }
        }
    }

    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CHAN, "hmac_config_set_channel: channel_num:%d, bw:%d, band:%d",
                     channel_param->uc_channel, channel_param->en_bandwidth, channel_param->en_band);

    return OAL_SUCC;
}


uint32_t hmac_config_set_channel(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    oal_bool_enum_uint8 en_set_reg = OAL_FALSE;
    uint32_t up_vap_cnt;
    oal_bool_enum_uint8 en_override = OAL_FALSE;

    mac_cfg_channel_param_stru *channel_param = (mac_cfg_channel_param_stru *)puc_param;

    /* 获取device */
    mac_device_stru *mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        oam_error_log1(0, OAM_SF_CHAN, "{hmac_config_set_channel::mac_device null,dev_id=%d.}", mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* p2p go场景下启动160m时，切换至80m */
    hmac_config_go_not_start_160m_handle(mac_vap, channel_param->uc_channel, &channel_param->en_bandwidth);
#ifdef _PRE_WLAN_FEATURE_DFS
    /* zero wait dfs(aput)场景下启动160m时，切换至80m,目前:1106支持，1103/05不支持 */
    wlan_chip_start_zero_wait_dfs(mac_vap, channel_param);
#endif
    ret = hmac_config_set_channel_check(mac_vap, channel_param);
    if (ret != OAL_SUCC) {
        return ret;
    }

    up_vap_cnt = hmac_calc_up_ap_num(mac_device);

#ifdef _PRE_WLAN_FEATURE_DFS
    en_override |= ((mac_vap_get_dfs_enable(mac_vap) && mac_dfs_get_cac_enable(mac_device))) ? OAL_TRUE : OAL_FALSE;
#endif

    en_override |= (mac_get_2040bss_switch(mac_device) && (!mac_vap->bit_bw_fixed)) ? OAL_TRUE : OAL_FALSE;

    en_override &= !mac_is_dbac_enabled(mac_device) ? OAL_TRUE : OAL_FALSE;

    en_override &= (up_vap_cnt > 1) ? OAL_TRUE : OAL_FALSE;

    /* 初始扫描使能时，运行时临时设置不同的信道 */
    if (en_override) {
        mac_device_get_channel(mac_device, channel_param);
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CHAN, "{hmac_config_set_channel::force chan band=%d ch=%d bw=%d}",
                         channel_param->en_band, channel_param->uc_channel, channel_param->en_bandwidth);
    }

    /* 仅在没有VAP up的情况下，配置硬件频带、带宽寄存器 */
    ret = hmac_config_set_channel_check_band_and_bandwidth(mac_device, mac_vap, channel_param, up_vap_cnt, &en_set_reg);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = hmac_config_set_channel_set_band_and_bandwidth(mac_device, mac_vap, channel_param, &en_set_reg);
    if (ret != OAL_SUCC) {
        return ret;
    }
    /***************************************************************************
     抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    if (en_set_reg == OAL_TRUE) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CHAN, "hmac_config_set_channel::send event to dmac, set register");
        ret = hmac_config_send_event(mac_vap, WLAN_CFGID_CFG80211_SET_CHANNEL, us_len, puc_param);
        if (oal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CHAN, "{hmac_config_set_channel::send event failed[%d].}", ret);
            return ret;
        }
    }

    /* hostapd配置带宽记录信息，同步dmac操作40m恢复定时器 */
    hmac_40m_intol_sync_data(mac_vap, mac_vap->st_channel.en_bandwidth, OAL_FALSE);

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_config_set_beacon_check_param(mac_device_stru *pst_mac_device,
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
                oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_beacon_check_param::"
                                 "not support HT mode,en_protocol=%d en_protocol_cap=%d.}",
                                 pst_prot_param->en_protocol, pst_mac_device->en_protocol_cap);
                return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
            }
            break;

        case WLAN_VHT_MODE:
        case WLAN_VHT_ONLY_MODE:
            if (pst_mac_device->en_protocol_cap < WLAN_PROTOCOL_CAP_VHT) {
                /* 设置11ac协议，但device不支持VHT模式 */
                oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_beacon_check_param::"
                                 "not support VHT mode,en_protocol=%d en_protocol_cap=%d.}",
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


OAL_STATIC void hmac_init_vap_and_rate_by_protocol(mac_vap_stru *pst_mac_vap,
    wlan_protocol_enum_uint8 en_protocol)
{
    wlan_nss_enum_uint8 en_ori_rx_nss = pst_mac_vap->en_vap_rx_nss;

    mac_vap_init_by_protocol(pst_mac_vap, en_protocol);

    if ((pst_mac_vap->en_p2p_mode == WLAN_P2P_GO_MODE) &&
        (pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP)) {
        pst_mac_vap->en_vap_rx_nss = oal_min(pst_mac_vap->en_vap_rx_nss, en_ori_rx_nss);
    }

    mac_vap_init_rates(pst_mac_vap);
}


uint32_t hmac_config_set_beacon(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_beacon_param_stru *pst_beacon_param = NULL;
    mac_device_stru *mac_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t ret;
    uint32_t aul_akm_suite[WLAN_AUTHENTICATION_SUITES] = { 0, 0 };
    uint16_t us_suite_idx;

    /* 获取device */
    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (oal_unlikely(oal_any_null_ptr2(mac_device, puc_param))) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_beacon:: \
            mac_device or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_beacon::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_beacon_param = (mac_beacon_param_stru *)puc_param;

    /* 检查协议配置参数是否在device能力内 */
    ret = hmac_config_set_beacon_check_param(mac_device, pst_beacon_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_beacon::check param failed[%d].}", ret);
        return ret;
    }

    if (pst_beacon_param->en_protocol >= WLAN_HT_MODE) {
        mac_mib_set_TxAggregateActived(mac_vap, OAL_TRUE);
        mac_mib_set_AmsduAggregateAtive(mac_vap, OAL_TRUE);
    } else {
        mac_mib_set_TxAggregateActived(mac_vap, OAL_FALSE);
        mac_mib_set_AmsduAggregateAtive(mac_vap, OAL_FALSE);
    }

    mac_vap_set_hide_ssid(mac_vap, pst_beacon_param->uc_hidden_ssid);

    /* 1102适配新内核start ap和change beacon接口复用此接口，不同的是change beacon时，不再设置beacon周期
       和dtim周期，因此，change beacon时，interval和dtim period参数为全零，此时不应该被设置到mib中 */
    /* 设置VAP beacon interval， dtim_period */
    if ((pst_beacon_param->l_dtim_period != 0) || (pst_beacon_param->l_interval != 0)) {
        mac_mib_set_dot11dtimperiod(mac_vap, (uint32_t)pst_beacon_param->l_dtim_period);
        mac_mib_set_BeaconPeriod(mac_vap, (uint32_t)pst_beacon_param->l_interval);
    }

    /* 设置short gi */
    mac_mib_set_ShortGIOptionInTwentyImplemented(mac_vap, pst_beacon_param->en_shortgi_20);
    mac_mib_set_ShortGIOptionInFortyImplemented(mac_vap, pst_beacon_param->en_shortgi_40);

    if ((mac_vap->st_channel.en_band == WLAN_BAND_2G) && (mac_vap->st_cap_flag.bit_disable_2ght40 == OAL_TRUE)) {
        mac_mib_set_ShortGIOptionInFortyImplemented(mac_vap, OAL_FALSE);
    }

    mac_mib_set_VHTShortGIOptionIn80Implemented(mac_vap, pst_beacon_param->en_shortgi_80);

    ret = mac_vap_set_security(mac_vap, pst_beacon_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_beacon::set_security failed[%d].}", ret);
        return ret;
    }

    mac_mib_get_rsn_akm_suites_s(&hmac_vap->st_vap_base_info, (uint32_t *)aul_akm_suite, sizeof(aul_akm_suite));

    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG,
        "{hmac_config_set_beacon::mac_mib_get_rsn_akm_suites 0x[%x], 0x[%x].}", aul_akm_suite[0], aul_akm_suite[1]);

    for (us_suite_idx = 0; us_suite_idx < WLAN_AUTHENTICATION_SUITES; us_suite_idx++) {
        if (aul_akm_suite[us_suite_idx] == MAC_RSN_AKM_OWE) {
            hmac_vap->owe_group[BYTE_OFFSET_0] = MAC_OWE_GROUP_19; /* APUT default supported OWE group */
            hmac_vap->owe_group[BYTE_OFFSET_1] = MAC_OWE_GROUP_20;
            hmac_vap->owe_group[BYTE_OFFSET_2] = MAC_OWE_GROUP_21;
            break;
        }
    }

    hmac_init_vap_and_rate_by_protocol(mac_vap, pst_beacon_param->en_protocol);

    /***************************************************************************
     抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_CFG80211_CONFIG_BEACON, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_beacon::send event failed[%d].}", ret);
    }

#ifdef _PRE_WLAN_FEATURE_M2S
    /* 同步vap修改信息到device侧 */
    hmac_config_vap_m2s_info_syn(mac_vap);
#endif

    return ret;
}

OAL_STATIC int32_t hmac_print_vap_basic_info(mac_vap_stru *mac_vap, mac_device_stru *mac_device,
                                             int8_t *print_buff, uint32_t string_len)
{
    int32_t string_tmp_len;

    string_tmp_len = snprintf_s(print_buff + string_len, ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
        ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
        "vap id: %d  device id: %d  chip id: %d\n"
        "vap state: %d\n"
        "vap mode: %d   P2P mode:%d\n"
        "ssid: %.32s\n"
        "hide_ssid :%d\n"
        "vap nss[%d] devicve nss[%d]\n",
        mac_vap->uc_vap_id, mac_vap->uc_device_id, mac_vap->uc_chip_id, mac_vap->en_vap_state, mac_vap->en_vap_mode,
        mac_vap->en_p2p_mode, mac_mib_get_DesiredSSID(mac_vap), mac_vap->st_cap_flag.bit_hide_ssid,
        mac_vap->en_vap_rx_nss, MAC_DEVICE_GET_NSS_NUM(mac_device));
    return string_tmp_len;
}

OAL_STATIC int32_t hmac_print_vap_protocol_info(mac_vap_stru *mac_vap, int8_t *print_buff, uint32_t string_len)
{
    int32_t string_tmp_len;
    mac_user_stru *mac_user = NULL;

    mac_user = mac_res_get_mac_user(mac_vap->us_assoc_vap_id);
    if ((mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) && (mac_user != NULL)) {
        string_tmp_len = snprintf_s(print_buff + string_len, ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
                                    ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
                                    "protocol: sta|ap[%s|%s],\n"
                                    "user bandwidth_cap:[%d] avail_bandwidth[%d] cur_bandwidth[%d],\n"
                                    "user id[%d] user nss:[%d] user_avail_nss[%d] smpd_opmode_nss[%d]\n",
                                    hmac_config_protocol2string(mac_vap->en_protocol),
                                    hmac_config_protocol2string(mac_user->en_cur_protocol_mode),
                                    mac_user->en_bandwidth_cap, mac_user->en_avail_bandwidth,
                                    mac_user->en_cur_bandwidth,
                                    mac_vap->us_assoc_vap_id,
                                    mac_user->en_user_max_cap_nss,
                                    mac_user->en_avail_num_spatial_stream,
                                    mac_user->en_smps_opmode_notify_nss);
    } else {
        string_tmp_len = snprintf_s(print_buff + string_len, ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
                                    ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
                                    "protocol: %s\n",
                                    hmac_config_protocol2string(mac_vap->en_protocol));
    }

    return string_tmp_len;
}

OAL_STATIC int32_t hmac_print_vap_detail_info(mac_vap_stru *mac_vap, hmac_vap_stru *hmac_vap,
                                              int8_t *print_buff, uint32_t string_len)
{
    int32_t string_tmp_len;

    string_tmp_len = snprintf_s(print_buff + string_len, ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
                                ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
                                "band: %s  bandwidth: %s\nchannel number:%d \n"
                                "associated user number:%d/%d \nBeacon interval:%d \nvap feature info:\n"
                                "amsdu     uapsd   wpa   wpa2   wps  keepalive\n"
                                "%d         %d       %d    %d     %d     %d\n"
                                "vap cap info:\n"
                                "shpreamble  shslottime  nobeacon  shortgi   2g11ac \n"
                                "%d           %d          %d         %d         %d\n"
                                "tx power: %d \nprotect mode: %d, auth mode: %d\n"
                                "erp aging cnt: %d, ht aging cnt: %d\n"
                                "auto_protection: %d\nobss_non_erp_present: %d\nobss_non_ht_present: %d\n"
                                "rts_cts_protect_mode: %d\ntxop_protect_mode: %d\n"
                                "no_short_slot_num: %d\nno_short_preamble_num: %d\nnon_erp_num: %d\n"
                                "non_ht_num: %d\nnon_gf_num: %d\n20M_only_num: %d\n"
                                "no_40dsss_cck_num: %d\nno_lsig_txop_num: %d\n",
                                hmac_config_band2string(mac_vap->st_channel.en_band),
                                hmac_config_bw2string((mac_vap->en_protocol <= WLAN_MIXED_TWO_11G_MODE) ?
                                WLAN_BAND_WIDTH_20M : mac_vap->st_channel.en_bandwidth),
                                mac_vap->st_channel.uc_chan_number, mac_vap->us_user_nums,
                                mac_mib_get_MaxAssocUserNums(mac_vap), mac_mib_get_BeaconPeriod(mac_vap),
                                mac_mib_get_CfgAmsduTxAtive(mac_vap), mac_vap->st_cap_flag.bit_uapsd,
                                mac_vap->st_cap_flag.bit_wpa, mac_vap->st_cap_flag.bit_wpa2,
                                mac_mib_get_WPSActive(mac_vap), mac_vap->st_cap_flag.bit_keepalive,
                                mac_mib_get_ShortPreambleOptionImplemented(mac_vap),
                                mac_mib_get_ShortSlotTimeOptionImplemented(mac_vap),
                                hmac_vap->en_no_beacon, mac_mib_get_ShortGIOptionInTwentyImplemented(mac_vap),
                                mac_vap->st_cap_flag.bit_11ac2g, mac_vap->uc_tx_power,
                                mac_vap->st_protection.en_protection_mode,
                                mac_mib_get_AuthenticationMode(mac_vap),
                                mac_vap->st_protection.uc_obss_non_erp_aging_cnt,
                                mac_vap->st_protection.uc_obss_non_ht_aging_cnt,
                                mac_vap->st_protection.bit_auto_protection,
                                mac_vap->st_protection.bit_obss_non_erp_present,
                                mac_vap->st_protection.bit_obss_non_ht_present,
                                mac_vap->st_protection.bit_rts_cts_protect_mode,
                                mac_vap->st_protection.bit_lsig_txop_protect_mode,
                                mac_vap->st_protection.uc_sta_no_short_slot_num,
                                mac_vap->st_protection.uc_sta_no_short_preamble_num,
                                mac_vap->st_protection.uc_sta_non_erp_num,
                                mac_vap->st_protection.uc_sta_non_ht_num,
                                mac_vap->st_protection.uc_sta_non_gf_num,
                                mac_vap->st_protection.uc_sta_20M_only_num,
                                mac_vap->st_protection.uc_sta_no_40dsss_cck_num,
                                mac_vap->st_protection.uc_sta_no_lsig_txop_num);
    return string_tmp_len;
}

OAL_STATIC int32_t hmac_print_vap_wpa_info(mac_vap_stru *mac_vap, int8_t *print_buff, uint32_t string_len)
{
    uint32_t pair_suites[WLAN_PAIRWISE_CIPHER_SUITES] = { 0 };
    uint32_t akm_suites[WLAN_PAIRWISE_CIPHER_SUITES]  = { 0 };
    uint32_t group_suit;
    int32_t string_tmp_len;

    group_suit = mac_mib_get_wpa_group_suite(mac_vap);

    mac_mib_get_wpa_pair_suites_s(mac_vap, pair_suites, sizeof(pair_suites));
    mac_mib_get_wpa_akm_suites_s(mac_vap, akm_suites, sizeof(akm_suites));
    string_tmp_len = snprintf_s(print_buff + string_len,
                                ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
                                ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
                                "Privacy Invoked: \nRSNA-WPA \n "
                                "GRUOP     WPA PAIREWISE0[Actived]    WPA PAIRWISE1[Actived]     "
                                "AUTH1[Active]     AUTH2[Active]\n "
                                "%s        %s[%s]          %s[%s]            %s[%s]             %s[%s]\n",
                                hmac_config_ciper2string((uint8_t)group_suit),
                                hmac_config_ciper2string((uint8_t)pair_suites[0]),
                                (pair_suites[0] == 0) ? "Actived" : "Inactived",
                                hmac_config_ciper2string((uint8_t)pair_suites[1]),
                                (pair_suites[1] == 0) ? "Actived" : "Inactived",
                                hmac_config_akm2string((uint8_t)akm_suites[0]),
                                (akm_suites[0] == 0) ? "Actived" : "Inactived",
                                hmac_config_akm2string((uint8_t)akm_suites[1]),
                                (akm_suites[1] == 0) ? "Actived" : "Inactived");
    return string_tmp_len;
}

OAL_STATIC int32_t hmac_print_vap_wpa2_info(mac_vap_stru *mac_vap, int8_t *print_buff, uint32_t string_len)
{
    uint32_t pair_suites[WLAN_PAIRWISE_CIPHER_SUITES] = { 0 };
    uint32_t akm_suites[WLAN_PAIRWISE_CIPHER_SUITES]  = { 0 };
    uint32_t group_suit;
    int32_t string_tmp_len;

    group_suit = mac_mib_get_rsn_group_suite(mac_vap);

    mac_mib_get_rsn_pair_suites_s(mac_vap, pair_suites, sizeof(pair_suites));
    mac_mib_get_rsn_akm_suites_s(mac_vap, akm_suites, sizeof(akm_suites));
    string_tmp_len = snprintf_s(print_buff + string_len,
                                ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
                                ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
                                "Privacy Invoked: \nRSNA-WPA2 \n"
                                "GRUOP     WPA2 PAIREWISE0[Actived]    WPA2 PAIRWISE1[Actived]     "
                                "AUTH1[Active]     AUTH2[Active]\n"
                                "%s        %s[%s]          %s[%s]            %s[%s]             %s[%s]\n",
                                hmac_config_ciper2string((uint8_t)group_suit),
                                hmac_config_ciper2string((uint8_t)pair_suites[0]),
                                (pair_suites[0] == 0) ? "Actived" : "Inactived",
                                hmac_config_ciper2string((uint8_t)pair_suites[1]),
                                (pair_suites[1] == 0) ? "Actived" : "Inactived",
                                hmac_config_akm2string((uint8_t)akm_suites[0]),
                                (akm_suites[0] == 0) ? "Actived" : "Inactived",
                                hmac_config_akm2string((uint8_t)akm_suites[1]),
                                (akm_suites[1] == 0) ? "Actived" : "Inactived");
    return string_tmp_len;
}

OAL_STATIC int32_t hmac_print_vap_app_ie_info(mac_vap_stru *mac_vap, int8_t *print_buff, uint32_t *string_len)
{
    int32_t string_tmp_len;
    uint8_t loop;
    /* APP IE 信息 */
    for (loop = 0; loop < OAL_APP_IE_NUM; loop++) {
        string_tmp_len = snprintf_s(print_buff + *string_len,
                                    ((OAM_REPORT_MAX_STRING_LEN - *string_len) - 1),
                                    ((OAM_REPORT_MAX_STRING_LEN - *string_len) - 1) - 1,
                                    "APP IE:type= %d, addr = %p, len = %d, max_len = %d\n",
                                    loop,
                                    mac_vap->ast_app_ie[loop].puc_ie,
                                    mac_vap->ast_app_ie[loop].ie_len,
                                    mac_vap->ast_app_ie[loop].ie_max_len);
        if (string_tmp_len < 0) {
            return string_tmp_len;
        }
        *string_len += (uint32_t)string_tmp_len;
    }
    return string_tmp_len;
}

/*lint -e801*/

uint32_t hmac_config_vap_info(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_device_stru *mac_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_user_stru *multi_user = NULL;
    int8_t *print_buff = NULL;
    uint32_t string_len = 0;
    int32_t string_tmp_len;
    uint32_t ret;

    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vap_info::config vap! can't get info.}");
        return OAL_FAIL;
    }

    if (mac_vap->pst_mib_info == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vap_info::mib info is null.}");
        return OAL_FAIL;
    }

    mac_device = (mac_device_stru *)mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vap_info::mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    print_buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (print_buff == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vap_info::print_buff null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vap_info::hmac_vap null.}");
        oal_mem_free_m(print_buff, OAL_TRUE);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* vap的基本信息 */
    string_tmp_len = hmac_print_vap_basic_info(mac_vap, mac_device, print_buff, string_len);
    if (string_tmp_len < 0) {
        goto sprint_fail;
    }
    string_len += (uint32_t)string_tmp_len;

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    if ((g_tas_switch_en[WLAN_RF_CHANNEL_ZERO] == OAL_TRUE) || (OAL_TRUE == g_tas_switch_en[WLAN_RF_CHANNEL_ONE])) {
        string_tmp_len = snprintf_s(print_buff + string_len, ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
                                    ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
                                    "tas_gpio[%d]\n", board_get_wifi_tas_gpio_state());
        if (string_tmp_len < 0) {
            goto sprint_fail;
        }
        string_len += (uint32_t)string_tmp_len;
    }
#endif

    /* AP/STA协议模式显示 */
    string_tmp_len = hmac_print_vap_protocol_info(mac_vap, print_buff, string_len);
    if (string_tmp_len < 0) {
        goto sprint_fail;
    }
    string_len += (uint32_t)string_tmp_len;

    string_tmp_len = hmac_print_vap_detail_info(mac_vap, hmac_vap, print_buff, string_len);
    if (string_tmp_len < 0) {
        goto sprint_fail;
    }

    print_buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    oam_print(print_buff);

    /* 上述日志量超过OAM_REPORT_MAX_STRING_LEN，分多次oam_print */
    memset_s(print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
    string_len = 0;

    /* WPA/WPA2 加密参数 */
    if (mac_mib_get_privacyinvoked(mac_vap) == OAL_TRUE) {
        multi_user = mac_res_get_mac_user(mac_vap->us_multi_user_idx);
        if (multi_user == NULL) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vap_info::multi_user[%d] null.}",
                             mac_vap->us_multi_user_idx);
            oal_mem_free_m(print_buff, OAL_TRUE);
            return OAL_ERR_CODE_PTR_NULL;
        }

        if (mac_mib_get_rsnaactivated(mac_vap) == OAL_TRUE) {
            if (mac_vap->st_cap_flag.bit_wpa == 1) {
                string_tmp_len = hmac_print_vap_wpa_info(mac_vap, print_buff, string_len);
                if (string_tmp_len < 0) {
                    goto sprint_fail;
                }
                string_len += (uint32_t)string_tmp_len;
            }

            if (mac_vap->st_cap_flag.bit_wpa2 == 1) {
                string_tmp_len = hmac_print_vap_wpa2_info(mac_vap, print_buff, string_len);
                if (string_tmp_len < 0) {
                    goto sprint_fail;
                }
                string_len += (uint32_t)string_tmp_len;
            }
        }

        string_tmp_len = snprintf_s(print_buff + string_len, ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
                                    ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
                                    "MULTI_USER: cipher_type:%s, key_type:%d \n",
                                    hmac_config_ciper2string(multi_user->st_key_info.en_cipher_type),
                                    multi_user->st_user_tx_info.st_security.en_cipher_key_type);
        if (string_tmp_len < 0) {
            goto sprint_fail;
        }
        string_len += (uint32_t)string_tmp_len;
    } else {
        string_tmp_len = snprintf_s(print_buff + string_len, ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
                                    ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1, "Privacy NOT Invoked\n");
        if (string_tmp_len < 0) {
            goto sprint_fail;
        }
        string_len += (uint32_t)string_tmp_len;
    }

    /* APP IE 信息 */
    string_tmp_len = hmac_print_vap_app_ie_info(mac_vap, print_buff, &string_len);
    if (string_tmp_len < 0) {
        goto sprint_fail;
    }

    print_buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    oam_print(print_buff);
    oal_mem_free_m(print_buff, OAL_TRUE);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_VAP_INFO, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vap_info::send event failed[%d].}", ret);
    }

    return ret;

sprint_fail:

    oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_vap_info:: OAL_SPRINTF return error!}");
    print_buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    oam_print(print_buff);
    oal_mem_free_m(print_buff, OAL_TRUE);

    return OAL_FAIL;
}


uint32_t hmac_config_eth_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_eth_switch_param_stru *pst_eth_switch_param;
    uint16_t us_user_idx = 0;
    uint32_t ret;

    pst_eth_switch_param = (mac_cfg_eth_switch_param_stru *)puc_param;

    ret = mac_vap_find_user_by_macaddr(pst_mac_vap, pst_eth_switch_param->auc_user_macaddr, &us_user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_eth_switch::mac_vap_find_user_by_macaddr failed[%d].}", ret);
        return ret;
    }

    ret = oam_report_eth_frame_set_switch(us_user_idx, pst_eth_switch_param->en_switch,
        pst_eth_switch_param->en_frame_direction);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_eth_switch::oam_report_eth_frame_set_switch failed[%d].}", ret);
        return ret;
    }

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_ETH_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_eth_switch::hmac_config_send_event fail[%d].", ret);
    }
    return OAL_SUCC;
}

/* 将配置命令结构转换为OAM结构 */
OAL_STATIC void hmac_config_init_oam_switch(mac_cfg_80211_ucast_switch_stru *mac_80211_switch_param,
    oam_80211_frame_ctx_union *oam_switch_param)
{
    oam_switch_param->frame_ctx.bit_content = mac_80211_switch_param->en_frame_switch;
    oam_switch_param->frame_ctx.bit_cb = mac_80211_switch_param->en_cb_switch;
    oam_switch_param->frame_ctx.bit_dscr = mac_80211_switch_param->en_dscr_switch;
    oam_switch_param->frame_ctx.bit_msdu_dscr = mac_80211_switch_param->sub_switch.stru.bit_msdu_dscr;
    oam_switch_param->frame_ctx.bit_ba_info = mac_80211_switch_param->sub_switch.stru.bit_ba_info;
    oam_switch_param->frame_ctx.bit_himit_dscr = mac_80211_switch_param->sub_switch.stru.bit_himit_dscr;
    oam_switch_param->frame_ctx.bit_mu_dscr = mac_80211_switch_param->sub_switch.stru.bit_mu_dscr;
}


uint32_t hmac_config_80211_ucast_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_80211_ucast_switch_stru *mac_80211_switch_param;
    oam_80211_frame_ctx_union oam_switch_param = {0};
    uint16_t us_user_idx = 0;
    uint16_t us_max_user_idx;
    uint32_t ret;

    mac_80211_switch_param = (mac_cfg_80211_ucast_switch_stru *)puc_param;

    us_max_user_idx = mac_board_get_max_user();

    /* 将配置命令结构转换为OAM结构 */
    hmac_config_init_oam_switch(mac_80211_switch_param, &oam_switch_param);

    /* 广播地址，操作所有用户的单播帧开关 */
    if (ether_is_broadcast(mac_80211_switch_param->auc_user_macaddr)) {
        for (us_user_idx = 0; us_user_idx < us_max_user_idx; us_user_idx++) {
            oam_report_80211_ucast_set_switch(mac_80211_switch_param->en_frame_direction,
                                              mac_80211_switch_param->en_frame_type,
                                              &oam_switch_param, us_user_idx);
        }
    } else {
        ret = mac_vap_find_user_by_macaddr(pst_mac_vap, mac_80211_switch_param->auc_user_macaddr, &us_user_idx);
        if (ret != OAL_SUCC) {
            oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_80211_ucast_switch::mac_vap_find_user_by_macaddr[%02X:XX:XX:%02X:%02X:%02X]failed!}",
                mac_80211_switch_param->auc_user_macaddr[MAC_ADDR_0],
                mac_80211_switch_param->auc_user_macaddr[MAC_ADDR_3],
                mac_80211_switch_param->auc_user_macaddr[MAC_ADDR_4],
                mac_80211_switch_param->auc_user_macaddr[MAC_ADDR_5]);
            return ret;
        }

        ret = oam_report_80211_ucast_set_switch(mac_80211_switch_param->en_frame_direction,
                                                mac_80211_switch_param->en_frame_type,
                                                &oam_switch_param, us_user_idx);
        if (ret != OAL_SUCC) {
            oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_80211_ucast_switch::Set switch of report_ucast failed[%d]!"
                "frame_switch[%d], cb_switch[%d], dscr_switch[%d].}",
                ret, mac_80211_switch_param->en_frame_switch,
                mac_80211_switch_param->en_cb_switch,
                mac_80211_switch_param->en_dscr_switch);
            return ret;
        }
    }

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_80211_UCAST_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_80211_ucast_switch::hmac_config_send_event fail[%d].", ret);
    }

    return OAL_SUCC;
}


uint32_t hmac_config_set_mgmt_log(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_start)
{
    mac_cfg_80211_ucast_switch_stru st_80211_ucast_switch = {0};

    if (oal_any_null_ptr2(pst_mac_vap, pst_mac_user)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (en_start != OAL_TRUE) {
        st_80211_ucast_switch.en_frame_direction = OAM_OTA_FRAME_DIRECTION_TYPE_TX;
        st_80211_ucast_switch.en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
        st_80211_ucast_switch.en_frame_switch = OAL_SWITCH_OFF;
        st_80211_ucast_switch.en_cb_switch = OAL_SWITCH_OFF;
        st_80211_ucast_switch.en_dscr_switch = OAL_SWITCH_OFF;
        oal_set_mac_addr(st_80211_ucast_switch.auc_user_macaddr, pst_mac_user->auc_user_mac_addr);
        hmac_config_80211_ucast_switch(pst_mac_vap, sizeof(st_80211_ucast_switch),
                                       (uint8_t *)&st_80211_ucast_switch);

        st_80211_ucast_switch.en_frame_direction = OAM_OTA_FRAME_DIRECTION_TYPE_RX;
        st_80211_ucast_switch.en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
        st_80211_ucast_switch.en_frame_switch = OAL_SWITCH_OFF;
        st_80211_ucast_switch.en_cb_switch = OAL_SWITCH_OFF;
        st_80211_ucast_switch.en_dscr_switch = OAL_SWITCH_OFF;
        hmac_config_80211_ucast_switch(pst_mac_vap, sizeof(st_80211_ucast_switch),
                                       (uint8_t *)&st_80211_ucast_switch);
    } else {
        st_80211_ucast_switch.en_frame_direction = OAM_OTA_FRAME_DIRECTION_TYPE_TX;
        st_80211_ucast_switch.en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
        st_80211_ucast_switch.en_frame_switch = OAL_SWITCH_ON;
        st_80211_ucast_switch.en_cb_switch = OAL_SWITCH_ON;
        st_80211_ucast_switch.en_dscr_switch = OAL_SWITCH_ON;
        oal_set_mac_addr(st_80211_ucast_switch.auc_user_macaddr, pst_mac_user->auc_user_mac_addr);

        hmac_config_80211_ucast_switch(pst_mac_vap, sizeof(st_80211_ucast_switch),
                                       (uint8_t *)&st_80211_ucast_switch);

        st_80211_ucast_switch.en_frame_direction = OAM_OTA_FRAME_DIRECTION_TYPE_RX;
        st_80211_ucast_switch.en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
        st_80211_ucast_switch.en_frame_switch = OAL_SWITCH_ON;
        st_80211_ucast_switch.en_cb_switch = OAL_SWITCH_ON;
        st_80211_ucast_switch.en_dscr_switch = OAL_SWITCH_ON;
        hmac_config_80211_ucast_switch(pst_mac_vap, sizeof(st_80211_ucast_switch),
                                       (uint8_t *)&st_80211_ucast_switch);
    }
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_NRCOEX

uint32_t hmac_config_query_nrcoex_stat(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_NRCOEX_STAT, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_COEX,
                         "{hmac_config_query_nrcoex_stat::send event return err code [%d].}", ret);
    }

    return ret;
}


OAL_STATIC uint32_t hmac_config_report_nrcoex_stat(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_device_stru *pst_hmac_device;
    mac_nrcoex_stat_stru *pst_mac_nrcoex_stat;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_COEX,
                       "hmac_config_report_nrcoex_stat: pst_hmac_device is null ptr. device id:%d",
                       pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_nrcoex_stat = (mac_nrcoex_stat_stru *)puc_param;

    if (memcpy_s(&pst_hmac_device->st_nrcoex_stat_query.st_nrcoex_stat, sizeof(mac_nrcoex_stat_stru),
        pst_mac_nrcoex_stat, sizeof(mac_nrcoex_stat_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_COEX, "hmac_config_report_nrcoex_stat::memcpy fail!");
        return OAL_FAIL;
    }
    pst_hmac_device->st_nrcoex_stat_query.en_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_device->st_nrcoex_stat_query.st_wait_queue));
    return OAL_SUCC;
}
uint32_t hmac_config_nrcoex_test(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_NRCOEX_TEST, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_COEX,
                         "{hmac_config_nrcoex::send event return err code [%d].}", ret);
    }

    return ret;
}
#endif

uint32_t hmac_config_phy_debug_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
#ifdef _PRE_WLAN_FEATURE_DFS
    mac_phy_debug_switch_stru *pst_phy_debug_switch;

    pst_phy_debug_switch = (mac_phy_debug_switch_stru *)puc_param;
    if (pst_phy_debug_switch->uc_report_radar_switch == OAL_TRUE) {
        ret = hmac_dfs_radar_detect_event_test(pst_mac_vap->uc_vap_id);
        if (ret != OAL_SUCC) {
            oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_phy_debug_switch::hmac_dfs_radar_detect_event_test failed[%d].}", ret);
            return ret;
        }
    }
#endif
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PHY_DEBUG_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_phy_debug_switch::hmac_config_send_event fail[%d].", ret);
        return ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11AX
OAL_STATIC void hmac_config_protocol_debug_printf_11ax_info(mac_vap_stru *pst_mac_vap)
{
    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
        "{hmac_config_protocol_debug_printf_11ax_info::he_mib_enable=[%d],hal_dev_11ax_enable=[%d],11ax_switch=[%d].",
        mac_mib_get_HEOptionImplemented(pst_mac_vap), IS_HAL_DEVICE_SUPPORT_11AX(pst_mac_vap),
        IS_CUSTOM_OPEN_11AX_SWITCH(pst_mac_vap));
}
#endif

OAL_STATIC void hmac_config_csa_debug_handle(mac_vap_stru *mac_vap,
    mac_protocol_debug_switch_stru *cfg_info, uint8_t *go_on_flag)
{
    uint32_t ret;
    uint8_t go_debug_csa_param[2] = {0}; // go csa参数共2字节
    *go_on_flag = OAL_FALSE;

    if (cfg_info->st_csa_debug_bit3.en_debug_flag == MAC_CSA_FLAG_GO_DEBUG) {
        go_debug_csa_param[0] = cfg_info->st_csa_debug_bit3.uc_channel;
        go_debug_csa_param[1] = cfg_info->st_csa_debug_bit3.en_bandwidth;

        hmac_p2p_change_go_channel(mac_vap, 2 * sizeof(uint8_t), go_debug_csa_param);  /* 信道号+带宽 2字节 */
        return;
    }

    if (!IS_AP(mac_vap)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_csa_debug_handle::is not ap,return.");
        return;
    }

    ret = mac_is_channel_num_valid(mac_vap->st_channel.en_band,
        cfg_info->st_csa_debug_bit3.uc_channel, mac_vap->st_channel.ext6g_band);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_csa_debug_handle::mac_is_channel_num_valid(%d),return.",
            cfg_info->st_csa_debug_bit3.uc_channel);
        return;
    }

    /* 根据device能力对参数进行检查 */
    if ((cfg_info->st_csa_debug_bit3.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) &&
        (mac_mib_get_dot11VapMaxBandWidth(mac_vap) < WLAN_BW_CAP_80M)) {
        cfg_info->st_csa_debug_bit3.en_bandwidth =
            mac_vap_get_bandwith(mac_mib_get_dot11VapMaxBandWidth(mac_vap), cfg_info->st_csa_debug_bit3.en_bandwidth);

        /* 设置80M带宽，但device能力不支持80M， 刷新成合适带宽做业务 */
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_csa_debug_handle::not support \
            80MHz bandwidth, csa_new_bandwidth=%d en_dot11VapMaxBandWidth=%d.}",
            cfg_info->st_csa_debug_bit3.en_bandwidth, mac_mib_get_dot11VapMaxBandWidth(mac_vap));
    }

    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_csa_debug_handle::csa_mode=%d csa_channel=%d\
        csa_cnt=%d debug_flag=%d \n", cfg_info->st_csa_debug_bit3.en_mode, cfg_info->st_csa_debug_bit3.uc_channel,
        cfg_info->st_csa_debug_bit3.uc_cnt, cfg_info->st_csa_debug_bit3.en_debug_flag);
    *go_on_flag = OAL_TRUE;

    if (cfg_info->st_csa_debug_bit3.en_debug_flag == MAC_CSA_FLAG_NORMAL) {
        mac_vap->st_ch_switch_info.en_csa_mode = cfg_info->st_csa_debug_bit3.en_mode;
        mac_vap->st_ch_switch_info.uc_ch_switch_cnt = cfg_info->st_csa_debug_bit3.uc_cnt;
        hmac_chan_initiate_switch_to_new_channel(mac_vap, cfg_info->st_csa_debug_bit3.uc_channel,
            cfg_info->st_csa_debug_bit3.en_bandwidth);
        *go_on_flag = OAL_FALSE;
        return;
    }
}


uint32_t hmac_config_protocol_debug_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint8_t go_on_flag;
    mac_protocol_debug_switch_stru *pst_protocol_debug;
    oal_bool_enum_uint8 en_40_intolerant;

    pst_protocol_debug = (mac_protocol_debug_switch_stru *)puc_param;

    /* 恢复40M带宽命令 */
    if (pst_protocol_debug->cmd_bit_map & BIT0) {
        /* host无需处理，dmac处理 */
    }

    /* 不容忍40M带宽开关 */
    if (pst_protocol_debug->cmd_bit_map & BIT2) {
        /* 只有2.4G才设置该mib值 */
        if (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band) {
            en_40_intolerant = (pst_protocol_debug->en_40_intolerant_bit2 == OAL_TRUE) ? OAL_TRUE : OAL_FALSE;
            mac_mib_set_FortyMHzIntolerant(pst_mac_vap, en_40_intolerant);
        } else {
            oam_warning_log1(0, OAM_SF_CFG,
                "{hmac_config_protocol_debug_switch::band is not 2G,but [%d].", pst_mac_vap->st_channel.en_band);
        }
    }
    /* csa cmd */
    if (pst_protocol_debug->cmd_bit_map & BIT3) {
        hmac_config_csa_debug_handle(pst_mac_vap, pst_protocol_debug, &go_on_flag);
        if (go_on_flag == OAL_FALSE) {
            return OAL_SUCC;
        }
    }
    /* lsigtxop使能 */
    if (pst_protocol_debug->cmd_bit_map & BIT5) {
        mac_mib_set_LsigTxopFullProtectionActivated(pst_mac_vap, pst_protocol_debug->en_lsigtxop_bit5);
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        if (pst_protocol_debug->cmd_bit_map & BIT6) {
            hmac_config_protocol_debug_printf_11ax_info(pst_mac_vap);
        }
    }
#endif
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PROTOCOL_DBG, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_protocol_debug_switch::hmac_config_send_event fail[%d].", ret);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_config_lte_gpio_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_CHECK_LTE_GPIO, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_lte_gpio_mode::hmac_config_send_event fail[%d].", ret);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_config_ota_rx_dscr_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    int32_t l_value;

    l_value = *((int32_t *)puc_param);

    /* 此处为了兼容03/05,06不使用 */
    oam_ota_set_switch(OAM_OTA_SWITCH_RX_PPDU_DSCR, (oal_switch_enum_uint8)l_value);
    oam_ota_set_switch(OAM_OTA_SWITCH_RX_MSDU_DSCR, (oal_switch_enum_uint8)l_value);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_OTA_RX_DSCR_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_ota_rx_dscr_switch::hmac_config_send_event fail[%d].", ret);
    }

    return ret;
}

 
void hmac_config_ota_switch(oam_ota_switch_param_stru *ota_swicth)
{
    oam_ota_set_switch(ota_swicth->ota_type, ota_swicth->ota_switch);
}

uint32_t hmac_config_set_random_mac_addr_scan(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_device_stru *pst_hmac_device = NULL;
    oal_bool_enum_uint8 en_random_mac_addr_scan_switch;

    en_random_mac_addr_scan_switch = *((oal_bool_enum_uint8 *)puc_param);

    /* 获取hmac device结构体 */
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_random_mac_addr_scan::pst_hmac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    g_wlan_cust.uc_random_mac_addr_scan = en_random_mac_addr_scan_switch;
#else
    pst_hmac_device->st_scan_mgmt.en_is_random_mac_addr_scan = en_random_mac_addr_scan_switch;
#endif

    return OAL_SUCC;
}


uint32_t hmac_config_set_random_mac_oui(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_device_stru *pst_mac_device = NULL;
    hmac_device_stru *pst_hmac_device = NULL;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_random_mac_oui::pst_mac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_random_mac_oui::pst_hmac_device is null.device_id %d}",
                         pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (us_len < WLAN_RANDOM_MAC_OUI_LEN) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_random_mac_oui::len is short:%d.}", us_len);
        return OAL_FAIL;
    }

    if (memcpy_s(pst_mac_device->auc_mac_oui, WLAN_RANDOM_MAC_OUI_LEN, puc_param, WLAN_RANDOM_MAC_OUI_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_set_random_mac_oui::memcpy fail!");
        return OAL_FAIL;
    }

    /* 系统会在启动wifi 后下发非零mac_oui, wps扫描或hilink连接的场景中,将mac_oui清0,
     * mac_oui 非零时生成扫描随机MAC, wifi 扫描时使用该MAC地址作为源地址 */
    if ((pst_mac_device->auc_mac_oui[0] != 0) ||
        (pst_mac_device->auc_mac_oui[BYTE_OFFSET_1] != 0) || (pst_mac_device->auc_mac_oui[BYTE_OFFSET_2] != 0)) {
        oal_random_ether_addr(pst_hmac_device->st_scan_mgmt.auc_random_mac);
        pst_hmac_device->st_scan_mgmt.auc_random_mac[BYTE_OFFSET_0] =
            pst_mac_device->auc_mac_oui[BYTE_OFFSET_0] & 0xfe; /* 保证是单播mac */
        pst_hmac_device->st_scan_mgmt.auc_random_mac[BYTE_OFFSET_1] = pst_mac_device->auc_mac_oui[BYTE_OFFSET_1];
        pst_hmac_device->st_scan_mgmt.auc_random_mac[BYTE_OFFSET_2] = pst_mac_device->auc_mac_oui[BYTE_OFFSET_2];

        oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_set_random_mac_oui::rand_mac_addr[%02X:XX:XX:%02X:%02X:%02X].}",
                         pst_hmac_device->st_scan_mgmt.auc_random_mac[MAC_ADDR_0],
                         pst_hmac_device->st_scan_mgmt.auc_random_mac[MAC_ADDR_3],
                         pst_hmac_device->st_scan_mgmt.auc_random_mac[MAC_ADDR_4],
                         pst_hmac_device->st_scan_mgmt.auc_random_mac[MAC_ADDR_5]);
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_RANDOM_MAC_OUI, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_random_mac_oui::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

#ifdef _PRE_WLAN_CHIP_FPGA_PCIE_TEST

uint32_t hmac_config_pcie_test(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PCIE_TEST, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_pcie_test::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif


uint32_t hmac_config_amsdu_ampdu_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_set_tlv_stru *pst_amsdu_ampdu_cfg;
    uint8_t uc_amsdu_en;
    uint8_t uc_index;
    pst_amsdu_ampdu_cfg = (mac_cfg_set_tlv_stru *)puc_param;

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    for (uc_index = 0; uc_index < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_index++) {
        g_st_tx_large_amsdu.en_tx_amsdu_level[uc_index] = (uint8_t)pst_amsdu_ampdu_cfg->value;
    }
#endif

    if (pst_amsdu_ampdu_cfg->value > 0) {
        uc_amsdu_en = OAL_TRUE;
    } else {
        uc_amsdu_en = OAL_FALSE;
    }

    /* 关闭动态切换算法 */
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    /* 关闭动态切换算法 */
    g_st_tx_large_amsdu.uc_host_large_amsdu_en = !uc_amsdu_en;
#endif

    /* 设置amsdu状态 */
    mac_mib_set_AmsduPlusAmpduActive(pst_mac_vap, uc_amsdu_en);
    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_amsdu_ampdu_switch::ENABLE LEVEL:%d,en %d.}",
        pst_amsdu_ampdu_cfg->value, !uc_amsdu_en);

    return OAL_SUCC;
}


uint32_t hmac_config_rx_ampdu_amsdu(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_set_tlv_stru *pst_rx_ampdu_amsdu_cfg = NULL;
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_rx_ampdu_amsdu::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_rx_ampdu_amsdu_cfg = (mac_cfg_set_tlv_stru *)puc_param;

    pst_hmac_vap->bit_rx_ampduplusamsdu_active = (uint8_t)pst_rx_ampdu_amsdu_cfg->value;

    g_uc_host_rx_ampdu_amsdu = pst_hmac_vap->bit_rx_ampduplusamsdu_active;
    pst_mac_vap->en_ps_rx_amsdu = g_uc_host_rx_ampdu_amsdu;
    pst_hmac_vap->en_ps_rx_amsdu = g_uc_host_rx_ampdu_amsdu;
    pst_mac_vap->uc_ps_type |= MAC_PS_TYPE_CMD;
    hmac_arp_probe_type_set(pst_mac_vap, OAL_FALSE, HMAC_VAP_ARP_PROBE_TYPE_DBAC);
    hmac_arp_probe_type_set(pst_mac_vap, OAL_FALSE, HMAC_VAP_ARP_PROBE_TYPE_BTCOEX);
    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_rx_ampdu_amsdu::ENABLE MODE[%d][0:disable,1:enable].}",
                     pst_rx_ampdu_amsdu_cfg->value);

    return OAL_SUCC;
}

uint32_t hmac_config_set_addba_rsp_buffer(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_set_tlv_stru *pst_rx_buffer_size_cfg = NULL;
    hmac_vap_stru *pst_hmac_vap;
    mac_rx_buffer_size_stru *rx_buffer_size = mac_vap_get_rx_buffer_size();

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_rx_ampdu_amsdu::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_rx_buffer_size_cfg = (mac_cfg_set_tlv_stru *)puc_param;

    rx_buffer_size->en_rx_ampdu_bitmap_cmd = pst_rx_buffer_size_cfg->uc_cmd_type;
    if (rx_buffer_size->en_rx_ampdu_bitmap_cmd == OAL_TRUE) {
        rx_buffer_size->us_rx_buffer_size = pst_rx_buffer_size_cfg->value;
    }

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_addba_rsp_buffer::ENABLE MODE[%d],buffer size [%d].}",
                     rx_buffer_size->en_rx_ampdu_bitmap_cmd, pst_rx_buffer_size_cfg->value);

    return OAL_SUCC;
}


uint32_t hmac_config_addba_req(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_addba_req_param_stru *pst_addba_req = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    hmac_vap_stru *pst_hmac_vap;
    mac_action_mgmt_args_stru st_action_args; /* 用于填写ACTION帧的参数 */
    oal_bool_enum_uint8 en_ampdu_support;

    pst_addba_req = (mac_cfg_addba_req_param_stru *)puc_param;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_addba_req::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取用户对应的索引 */
    pst_hmac_user = mac_vap_get_hmac_user_by_addr(pst_mac_vap, pst_addba_req->auc_mac_addr);
    if (pst_hmac_user == NULL) {
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
            (3)arg1:BA会话对应的TID
            (4)arg2:BUFFER SIZE大小
            (5)arg3:BA会话的确认策略
            (6)arg4:TIMEOUT时间
        */
        st_action_args.uc_category = MAC_ACTION_CATEGORY_BA;
        st_action_args.uc_action = MAC_BA_ACTION_ADDBA_REQ;
        st_action_args.arg1 = pst_addba_req->uc_tidno;     /* 该数据帧对应的TID号 */
        st_action_args.arg2 = pst_addba_req->us_buff_size; /* ADDBA_REQ中，buffer_size的默认大小 */
        st_action_args.arg3 = pst_addba_req->en_ba_policy; /* BA会话的确认策略 */
        st_action_args.arg4 = pst_addba_req->us_timeout;   /* BA会话的超时时间设置为0 */

        /* 建立BA会话 */
        hmac_mgmt_tx_action(pst_hmac_vap, pst_hmac_user, &st_action_args);
    }

    return OAL_SUCC;
}


uint32_t hmac_config_delba_req(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_delba_req_param_stru *pst_delba_req = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    mac_action_mgmt_args_stru st_action_args; /* 用于填写ACTION帧的参数 */
    hmac_tid_stru *pst_hmac_tid = NULL;

    pst_delba_req = (mac_cfg_delba_req_param_stru *)puc_param;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);

    /* 获取用户对应的索引 */
    pst_hmac_user = mac_vap_get_hmac_user_by_addr(pst_mac_vap, pst_delba_req->auc_mac_addr);
    if (pst_hmac_user == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_delba_req::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_tid = &(pst_hmac_user->ast_tid_info[pst_delba_req->uc_tidno]);

    /* 查看会话是否存在 */
    if (pst_delba_req->en_direction == MAC_RECIPIENT_DELBA) {
        if (pst_delba_req->en_trigger == MAC_DELBA_TRIGGER_COMM) {
            pst_hmac_tid->en_ba_handle_rx_enable = OAL_FALSE;
        }

        if (pst_hmac_tid->pst_ba_rx_info == NULL) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_delba_req::the rx hdl is not exist.}");
            return OAL_SUCC;
        }
    } else {
        if (pst_delba_req->en_trigger == MAC_DELBA_TRIGGER_COMM) {
            pst_hmac_tid->en_ba_handle_tx_enable = OAL_FALSE;
        }

        if (pst_hmac_tid->st_ba_tx_info.en_ba_status == DMAC_BA_INIT) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_delba_req::the tx hdl is not exist.}");
            return OAL_SUCC;
        }
    }

    /*
        建立BA会话时，st_action_args(DELBA_REQ)结构各个成员意义如下
        (1)uc_category:action的类别
        (2)uc_action:BA action下的类别
        (3)arg1:BA会话对应的TID
        (4)arg2:删除ba会话的发起端
        (5)arg3:删除ba会话的原因
        (6)arg5:ba会话对应的用户
    */
    st_action_args.uc_category = MAC_ACTION_CATEGORY_BA;
    st_action_args.uc_action = MAC_BA_ACTION_DELBA;
    st_action_args.arg1 = pst_delba_req->uc_tidno;      /* 该数据帧对应的TID号 */
    st_action_args.arg2 = pst_delba_req->en_direction;  /* ADDBA_REQ中，buffer_size的默认大小 */
    st_action_args.arg3 = MAC_QSTA_TIMEOUT;             /* BA会话的确认策略 */
    st_action_args.puc_arg5 = pst_delba_req->auc_mac_addr; /* ba会话对应的user */

    /* 建立BA会话 */
    hmac_mgmt_tx_action(pst_hmac_vap, pst_hmac_user, &st_action_args);

    return OAL_SUCC;
}


uint32_t hmac_config_user_info(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_user_stru *hmac_user;
    uint32_t ret;
    uint8_t tid_idx;
    oam_output_type_enum_uint8 en_output_type = OAM_OUTPUT_TYPE_BUTT;
    mac_cfg_user_info_param_stru *hmac_event;

    hmac_event = (mac_cfg_user_info_param_stru *)puc_param;
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(hmac_event->us_user_idx);
    if (hmac_user == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_user_info::hmac_user[%d] null.}", hmac_event->us_user_idx);
        return OAL_FAIL;
    }

    oam_get_output_type(&en_output_type);
    if (en_output_type != OAM_OUTPUT_TYPE_SDT) {
        oal_io_print("en_user_asoc_state :  %d \n", hmac_user->st_user_base_info.en_user_asoc_state);
        oal_io_print("uc_is_wds :           %d \n", hmac_user->uc_is_wds);
        oal_io_print("us_amsdu_maxsize :    %d \n", hmac_user->amsdu_maxsize);
        oal_io_print("11ac2g :              %d \n", hmac_user->st_hmac_cap_info.bit_11ac2g);
        oal_io_print("\n");

        for (tid_idx = 0; tid_idx < WLAN_TID_MAX_NUM; tid_idx++) {
            oal_io_print("tid               %d \n", tid_idx);
            oal_io_print("uc_amsdu_maxnum : %d \n", hmac_user->ast_hmac_amsdu[tid_idx].amsdu_maxnum);
            oal_io_print("us_amsdu_maxsize :%d \n", hmac_user->ast_hmac_amsdu[tid_idx].amsdu_maxsize);
            oal_io_print("us_amsdu_size :   %d \n", hmac_user->ast_hmac_amsdu[tid_idx].amsdu_size);
            oal_io_print("uc_msdu_num :     %d \n", hmac_user->ast_hmac_amsdu[tid_idx].msdu_num);
            oal_io_print("\n");
        }

        oal_io_print("us_user_hash_idx :    %d \n", hmac_user->st_user_base_info.us_user_hash_idx);
        oal_io_print("us_assoc_id :         %d \n", hmac_user->st_user_base_info.us_assoc_id);
        oal_io_print("uc_vap_id :           %d \n", hmac_user->st_user_base_info.uc_vap_id);
        oal_io_print("uc_device_id :        %d \n", hmac_user->st_user_base_info.uc_device_id);
        oal_io_print("uc_chip_id :          %d \n", hmac_user->st_user_base_info.uc_chip_id);
        oal_io_print("uc_amsdu_supported :  %d \n", hmac_user->uc_amsdu_supported);
        oal_io_print("uc_htc_support :      %d \n", hmac_user->st_user_base_info.st_ht_hdl.uc_htc_support);
        oal_io_print("en_ht_support :       %d \n", hmac_user->st_user_base_info.st_ht_hdl.en_ht_capable);
        oal_io_print("short gi 20 40 80:    %d %d %d \n", hmac_user->st_user_base_info.st_ht_hdl.bit_short_gi_20mhz,
                     hmac_user->st_user_base_info.st_ht_hdl.bit_short_gi_40mhz,
                     hmac_user->st_user_base_info.st_vht_hdl.bit_short_gi_80mhz);
        oal_io_print("\n");

        oal_io_print("Privacy info : \r\n");
        oal_io_print("    port_valid   :                     %d \r\n",
                     hmac_user->st_user_base_info.en_port_valid);
        oal_io_print("    user_tx_info.security.cipher_key_type:      %s \r\n"
            "    user_tx_info.security.cipher_protocol_type: %s \r\n",
            hmac_config_keytype2string(hmac_user->st_user_base_info.st_user_tx_info.st_security.en_cipher_key_type),
            hmac_config_cipher2string(hmac_user->st_user_base_info.st_key_info.en_cipher_type));
        if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
            oal_io_print("    STA:cipher_type :                           %s \r\n",
                         hmac_config_cipher2string(hmac_user->st_user_base_info.st_key_info.en_cipher_type));
        }
        oal_io_print("\n");
    } else {
        oam_ota_report((uint8_t *)hmac_user, (uint16_t)(sizeof(hmac_user_stru) - sizeof(mac_user_stru)),
                       0, 0, OAM_OTA_TYPE_HMAC_USER);
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_USER_INFO, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_user_info::send event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_config_update_ip_filter(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    dmac_tx_event_stru *pst_tx_event = NULL;
    frw_event_mem_stru *pst_event_mem = NULL;
    oal_netbuf_stru *pst_netbuf_cmd = NULL;
    frw_event_stru *pst_hmac_to_dmac_ctx_event = NULL;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_update_ip_filter::pst_mac_vap or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_netbuf_cmd = *((oal_netbuf_stru **)puc_param);
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    pst_event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (pst_event_mem == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_update_ip_filter::pst_event_mem null.}");
        oal_netbuf_free(pst_netbuf_cmd);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_to_dmac_ctx_event = (frw_event_stru *)pst_event_mem->puc_data;
    frw_event_hdr_init(&(pst_hmac_to_dmac_ctx_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_IP_FILTER,
                       sizeof(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    pst_tx_event = (dmac_tx_event_stru *)(pst_hmac_to_dmac_ctx_event->auc_event_data);
    pst_tx_event->pst_netbuf = pst_netbuf_cmd;
    pst_tx_event->us_frame_len = oal_netbuf_len(pst_netbuf_cmd);

    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_update_ip_filter::frw_event_dispatch_event failed[%d].}", ret);
    }
    oal_netbuf_free(pst_netbuf_cmd);
    frw_event_free_m(pst_event_mem);

    return ret;
}

static uint32_t hmac_get_kicking_user_idx(mac_vap_stru *mac_vap, uint8_t *user_mac, uint16_t *user_idx)
{
    uint32_t ret;
    ret = mac_vap_find_user_by_macaddr(mac_vap, user_mac, user_idx);
    if (ret == OAL_SUCC) {
        return ret;
    }

    if (mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING) {
        // 漫游状态下，user的mac已经被修改，返回vap记录的idx
        *user_idx = mac_vap->us_assoc_vap_id;
        return OAL_SUCC;
    }

    return ret;
}


uint32_t hmac_config_kick_user(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *param)
{
    mac_cfg_kick_user_param_stru *kick_user_param = NULL;
    hmac_user_stru *hmac_user = NULL;
    oal_bool_enum_uint8 en_is_protected;
    oal_dlist_head_stru *entry = NULL;
    oal_dlist_head_stru *dlist_tmp = NULL;
    mac_user_stru *mac_user_tmp = NULL;
    hmac_user_stru *hmac_user_tmp = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    uint16_t user_idx = 0;
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(mac_vap, param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_kick_user::mac_vap or param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_kick_user::en_vap_mode is WLAN_VAP_MODE_CONFIG.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    kick_user_param = (mac_cfg_kick_user_param_stru *)param;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_config_kick_user::null param,hmac_vap[%d].}", mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_kick_user::kick user mac[%02X:XX:XX:XX:%02X:%02X] reason code[%d]}",
        kick_user_param->auc_mac_addr[0], kick_user_param->auc_mac_addr[4], kick_user_param->auc_mac_addr[5], kick_user_param->us_reason_code);

    /* 踢掉全部user */
    if (oal_is_broadcast_ether_addr(kick_user_param->auc_mac_addr)) {
        /* 遍历vap下所有用户, 删除用户 */
        oal_dlist_search_for_each_safe(entry, dlist_tmp, &(mac_vap->st_mac_user_list_head))
        {
            mac_user_tmp = oal_dlist_get_entry(entry, mac_user_stru, st_user_dlist);
            if (mac_user_tmp == NULL) {
                oam_error_log0(mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_kick_user::mac_user_tmp null.}");
                continue;
            }

            hmac_user_tmp = mac_res_get_hmac_user(mac_user_tmp->us_assoc_id);
            if (oal_unlikely(hmac_user_tmp == NULL)) {
                oam_error_log1(0, OAM_SF_UM, "{hmac_config_kick_user::null param,hmac_user_tmp[%d].}", mac_user_tmp->us_assoc_id);
                continue;
            }

            /* 管理帧加密是否开启 */
            en_is_protected = mac_user_tmp->st_cap_info.bit_pmf_active;

            /* 发去关联帧 */
            hmac_mgmt_send_disassoc_frame(mac_vap, mac_user_tmp->auc_user_mac_addr, kick_user_param->us_reason_code, en_is_protected);

            /* 修改 state & 删除 user */
            hmac_handle_disconnect_rsp(hmac_vap, hmac_user_tmp, kick_user_param->us_reason_code);

            /* 删除用户 */
            hmac_user_del(mac_vap, hmac_user_tmp);
        }

        /* VAP下user头指针不应该为空 */
        if (oal_dlist_is_empty(&mac_vap->st_mac_user_list_head) == OAL_FALSE) {
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_kick_user::mac_user_list_head is not empty.}");
        }

        return OAL_SUCC;
    }

    ret = hmac_get_kicking_user_idx(mac_vap, kick_user_param->auc_mac_addr, &user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_UM,
            "{hmac_config_kick_user::find_user_by_addr failed[%d], vap state[%d].}", ret, mac_vap->en_vap_state);
        if (mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING) {
            user_idx = mac_vap->us_assoc_vap_id;
        } else {
            hmac_fsm_change_state_check(hmac_vap, MAC_VAP_STATE_BUTT, MAC_VAP_STATE_STA_FAKE_UP);
            return ret;
        }
    }

    hmac_user = mac_res_get_hmac_user(user_idx);
    if (hmac_user == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_kick_user::user[%d] is null.}", user_idx);
        hmac_fsm_change_state_check(hmac_vap, MAC_VAP_STATE_BUTT, MAC_VAP_STATE_STA_FAKE_UP);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_user->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_ASSOC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_kick_user::user[%d] is unassociated.}", user_idx);
    }

    en_is_protected = hmac_user->st_user_base_info.st_cap_info.bit_pmf_active;

    /* 发去认证帧 */
    hmac_mgmt_send_disassoc_frame(mac_vap, hmac_user->st_user_base_info.auc_user_mac_addr, kick_user_param->us_reason_code, en_is_protected);

    /* 修改 state & 删除 user */
    hmac_handle_disconnect_rsp(hmac_vap, hmac_user, kick_user_param->us_reason_code);
    hmac_user->assoc_err_code = kick_user_param->us_reason_code;
    /* 删除用户 */
    hmac_user_del(mac_vap, hmac_user);

    return OAL_SUCC;
}


uint32_t hmac_config_set_tx_pow_param(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_TX_POW, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_tx_pow_param::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_set_dscr_param(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_DSCR, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_dscr_param::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_log_level(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_LOG_LEVEL, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_log_level::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_set_rate(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_RATE, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_rate::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_set_mcs(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_MCS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_mcs::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_set_mcsac(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_MCSAC, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_mcsac::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

#ifdef _PRE_WLAN_FEATURE_11AX
uint32_t hmac_config_set_mcsax(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_MCSAX, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_mcsac::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
uint32_t hmac_config_set_mcsax_er(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_MCSAX_ER, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_mcsax_er::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif
#endif


uint32_t hmac_config_set_nss(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_NSS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_nss::hmac_config_send_event failed[%d].}", ret);
    }
    return ret;
}


uint32_t hmac_config_set_rfch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_RFCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_rfch::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_set_bw(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_BW, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_bw::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_set_band(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SET_BAND, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_band::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_always_tx(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
#ifdef CONFIG_ARCH_KIRIN_PCIE
    if (((mac_cfg_tx_comp_stru *)puc_param)->uc_param == HAL_ALWAYS_TX_MPDU) {
        ret = hmac_always_tx_proc(puc_param);
        if (ret != OAL_SUCC) {
            oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_always_tx::host tx failed[%d].}",
                ret);
            return ret;
        }
    } else if ((g_always_tx_ring.msdu_cnt != 0) &&
        (((mac_cfg_tx_comp_stru *)puc_param)->uc_param == HAL_ALWAYS_TX_DISABLE)) {
        hmac_tx_host_ring_release(&g_always_tx_ring);
    }
#endif
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_ALWAYS_TX, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_always_tx::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_ru_index(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SET_RU_INDEX, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_ru_index::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_always_rx(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_ALWAYS_RX, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_always_rx::send event fail[%d].}", ret);
    }
#ifdef CONFIG_ARCH_KIRIN_PCIE
#ifndef _PRE_LINUX_TEST
    /* 设置host常收状态 */
    hal_set_alrx_status(OAL_TRUE);
#endif
#endif
    return ret;
}


uint32_t hmac_config_list_sta(mac_vap_stru *pst_mac_vap)
{
    uint16_t us_user_idx;
    mac_user_stru *pst_mac_user = NULL;
    oal_dlist_head_stru *pst_head = NULL;
    wlan_protocol_enum_uint8 en_protocol_mode;
    int8_t ac_tmp_buff[256] = { 0 }; /* 临时存放user信息的buffer长度 256 */
    int32_t l_remainder_len;
    int8_t *pc_print_tmp = NULL;
    int8_t *pc_print_buff = NULL;
    int32_t ret;
    /* AP侧的信息才能打印相关信息 */
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_list_sta::invalid en_vap_mode[%d].}",
                         pst_mac_vap->en_vap_mode);
        return OAL_FAIL;
    }

    pc_print_buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (pc_print_buff == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);

    snprintf_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, OAM_REPORT_MAX_STRING_LEN - 1,
               "Total user num is %d \nUser assoc id ADDR Protocol Type \n", pst_mac_vap->us_user_nums);

    pc_print_tmp = pc_print_buff;
    l_remainder_len = (int32_t)(OAM_REPORT_MAX_STRING_LEN - OAL_STRLEN(pc_print_tmp));

    oal_spin_lock_bh(&pst_mac_vap->st_cache_user_lock);

    /* AP侧的USER信息 */
    for (us_user_idx = 0; us_user_idx < MAC_VAP_USER_HASH_MAX_VALUE; us_user_idx++) {
        oal_dlist_search_for_each(pst_head, &(pst_mac_vap->ast_user_hash[us_user_idx]))
        {
            /* 找到相应用户 */
            pst_mac_user = (mac_user_stru *)oal_dlist_get_entry(pst_head, mac_user_stru, st_user_hash_dlist);
            if (pst_mac_user == NULL) {
                oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_list_sta::pst_mac_user null.}");
                continue;
            }
            /* user结构体下的协议模式不区分a和g，需要根据频段区分 */
            en_protocol_mode = pst_mac_user->en_protocol_mode;
            if (en_protocol_mode >= WLAN_PROTOCOL_BUTT) {
                oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                               "{hmac_config_list_sta:: protocol_mode wrong.}", en_protocol_mode);
                continue;
            }
            if ((en_protocol_mode == WLAN_LEGACY_11G_MODE) && (pst_mac_vap->st_channel.en_band == WLAN_BAND_5G)) {
                en_protocol_mode = WLAN_LEGACY_11A_MODE;
            }

            ret = snprintf_s(ac_tmp_buff, sizeof(ac_tmp_buff), sizeof(ac_tmp_buff) - 1,
                             "     %d       %02X:XX:XX:%02X:%02X:%02X       %s \n",
                             pst_mac_user->us_assoc_id,
                             pst_mac_user->auc_user_mac_addr[MAC_ADDR_0],
                             pst_mac_user->auc_user_mac_addr[MAC_ADDR_3],
                             pst_mac_user->auc_user_mac_addr[MAC_ADDR_4],
                             pst_mac_user->auc_user_mac_addr[MAC_ADDR_5],
                             g_st_protocol_mode_list[en_protocol_mode].puc_protocol_desc);
            if (ret < 0) {
                oam_error_log0(0, OAM_SF_CFG, "hmac_config_list_sta::snprintf_s error!");
                oal_mem_free_m(pc_print_buff, OAL_TRUE);
                oal_spin_unlock_bh(&pst_mac_vap->st_cache_user_lock);
                return OAL_FAIL;
            }

            ret = strncat_s(pc_print_tmp, OAM_REPORT_MAX_STRING_LEN, ac_tmp_buff, l_remainder_len - 1);
            if (ret != EOK) {
                oam_error_log0(0, OAM_SF_CFG, "hmac_config_list_sta::strncat_s error!");
                oal_mem_free_m(pc_print_buff, OAL_TRUE);
                oal_spin_unlock_bh(&pst_mac_vap->st_cache_user_lock);
                return OAL_FAIL;
            }

            if (strncat_s(pc_print_tmp, OAM_REPORT_MAX_STRING_LEN, ac_tmp_buff, l_remainder_len - 1) != EOK) {
                oal_spin_unlock_bh(&pst_mac_vap->st_cache_user_lock);
                oal_mem_free_m(pc_print_buff, OAL_TRUE);
                oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_list_sta::strncat_s fail.}");
                oal_spin_unlock_bh(&pst_mac_vap->st_cache_user_lock);
                return OAL_FAIL;
            }

            memset_s(ac_tmp_buff, sizeof(ac_tmp_buff), 0, sizeof(ac_tmp_buff));
            l_remainder_len = (int32_t)(OAM_REPORT_MAX_STRING_LEN - OAL_STRLEN(pc_print_tmp));
        }
    }

    oal_spin_unlock_bh(&pst_mac_vap->st_cache_user_lock);

    oam_print(pc_print_buff);
    oal_mem_free_m(pc_print_buff, OAL_TRUE);
    return OAL_SUCC;
}


uint32_t hmac_config_get_sta_list(mac_vap_stru *pst_mac_vap, uint16_t *us_len, uint8_t *puc_param)
{
    uint16_t us_user_idx;
    mac_user_stru *pst_mac_user = NULL;
    oal_dlist_head_stru *pst_head = NULL;
    int8_t ac_tmp_buff[256] = { 0 }; /* 临时存放user信息的buffer长度 256 */
    int32_t l_remainder_len;
    int8_t *pc_sta_list_buff = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;
    uint32_t netbuf_len;
    int32_t ret;

    /* 事件传递指针值，此处异常返回前将其置为NULL */
    *(uintptr_t *)puc_param = (uintptr_t)NULL;

    /* AP侧的信息才能打印相关信息 */
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_get_sta_list::invalid en_vap_mode[%d].}", pst_mac_vap->en_vap_mode);
        return OAL_FAIL;
    }

    pc_sta_list_buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (pc_sta_list_buff == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_get_sta_list, oal_mem_alloc_m failed.");
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(pc_sta_list_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
    l_remainder_len = (int32_t)(OAM_REPORT_MAX_STRING_LEN - OAL_STRLEN(pc_sta_list_buff));

    oal_spin_lock_bh(&pst_mac_vap->st_cache_user_lock);

    /* AP侧的USER信息 */
    for (us_user_idx = 0; us_user_idx < MAC_VAP_USER_HASH_MAX_VALUE; us_user_idx++) {
        oal_dlist_search_for_each(pst_head, &(pst_mac_vap->ast_user_hash[us_user_idx]))
        {
            /* 找到相应用户 */
            pst_mac_user = (mac_user_stru *)oal_dlist_get_entry(pst_head, mac_user_stru, st_user_hash_dlist);
            if (pst_mac_user == NULL) {
                oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_sta_list::pst_mac_user null.}");
                continue;
            }
            /* 检查用户关联状态 */
            if (pst_mac_user->en_user_asoc_state != MAC_USER_STATE_ASSOC) {
                continue;
            }
            snprintf_s(ac_tmp_buff, sizeof(ac_tmp_buff), sizeof(ac_tmp_buff) - 1,
                       "%02X:%02X:%02X:%02X:%02X:%02X\n",
                       pst_mac_user->auc_user_mac_addr[MAC_ADDR_0], pst_mac_user->auc_user_mac_addr[MAC_ADDR_1],
                       pst_mac_user->auc_user_mac_addr[MAC_ADDR_2], pst_mac_user->auc_user_mac_addr[MAC_ADDR_3],
                       pst_mac_user->auc_user_mac_addr[MAC_ADDR_4], pst_mac_user->auc_user_mac_addr[MAC_ADDR_5]);

            oal_io_print("hmac_config_get_sta_list,STA:%02X:XX:XX:%02X:%02X:%02X\n",
                         pst_mac_user->auc_user_mac_addr[MAC_ADDR_0], pst_mac_user->auc_user_mac_addr[MAC_ADDR_3],
                         pst_mac_user->auc_user_mac_addr[MAC_ADDR_4], pst_mac_user->auc_user_mac_addr[MAC_ADDR_5]);

            ret = strncat_s(pc_sta_list_buff, OAM_REPORT_MAX_STRING_LEN, ac_tmp_buff, l_remainder_len - 1);
            if (ret != EOK) {
                oam_error_log0(0, OAM_SF_CFG, "hmac_config_get_sta_list::strncat_s error!");
                oal_mem_free_m(pc_sta_list_buff, OAL_TRUE);
                oal_spin_unlock_bh(&pst_mac_vap->st_cache_user_lock);
                return OAL_FAIL;
            }
            memset_s(ac_tmp_buff, sizeof(ac_tmp_buff), 0, sizeof(ac_tmp_buff));
            l_remainder_len = (int32_t)(OAM_REPORT_MAX_STRING_LEN - OAL_STRLEN(pc_sta_list_buff));
        }
    }

    oal_spin_unlock_bh(&pst_mac_vap->st_cache_user_lock);

    netbuf_len = OAL_STRLEN(pc_sta_list_buff);
    pst_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, netbuf_len, OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf != NULL) {
        ret = memcpy_s(oal_netbuf_put(pst_netbuf, netbuf_len), netbuf_len, pc_sta_list_buff, netbuf_len);
        if (ret != EOK) {
            oam_error_log0(0, OAM_SF_CFG, "hmac_config_get_sta_list::memcpy fail!");
            oal_mem_free_m(pc_sta_list_buff, OAL_TRUE);
            oal_netbuf_free(pst_netbuf);
            return OAL_FAIL;
        }
    } else {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_config_get_sta_list::Alloc netbuf(size %d) NULL in normal_netbuf_pool!", netbuf_len);
    }

    *(uintptr_t *)puc_param = (uintptr_t)pst_netbuf;

    /* 事件传递指针，此处记录指针长度 */
    *us_len = (uint16_t)sizeof(oal_netbuf_stru *);

    oal_mem_free_m(pc_sta_list_buff, OAL_TRUE);
    return OAL_SUCC;
}

#if defined(_PRE_WLAN_FEATURE_HID2D) && defined(_PRE_WLAN_FEATURE_HID2D_PRESENTATION)

uint32_t hmac_fix_country_in_hid2d_mode(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_device)
{
    mac_regdomain_info_stru *pst_regdomain_info = NULL;
    mac_get_regdomain_info(&pst_regdomain_info);

    /* HiD2D 发布会场景拦截 */
    /* 如果发布会模式为1，且当前国家码为99，则不允许修改国家码；其他情况会下发修改国家码指令 */
    if (pst_mac_device->is_presentation_mode == OAL_TRUE &&
        (pst_regdomain_info->ac_country[0] == '9' && pst_regdomain_info->ac_country[1] == '9')) {
        oam_warning_log0(0, OAM_SF_DFS, "{Hmac: In HiD2D Presentation Mode, Can not change the country code!!!}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}
#endif

void hmac_config_set_dfs_params(mac_device_stru *pst_mac_device, mac_regdomain_info_stru *pst_mac_regdom)
{
#ifdef _PRE_WLAN_FEATURE_DFS
    int8_t *pc_current_country = mac_regdomain_get_country();
    pst_mac_device->st_dfs.st_dfs_info.en_dfs_init = OAL_FALSE;
    /* 当前国家码与要设置的国家码不一致，需要重新初始化雷达信道 */
    if ((pst_mac_regdom->ac_country[0] != pc_current_country[0])
        || (pst_mac_regdom->ac_country[1] != pc_current_country[1])) {
        pst_mac_device->st_dfs.st_dfs_info.en_dfs_init = OAL_FALSE;
    }
#endif
}

uint32_t hmac_config_set_country(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_country_stru *pst_country_param;
    mac_regdomain_info_stru *pst_mac_regdom;
    mac_device_stru *pst_mac_device = NULL;
    uint32_t ret;
    uint8_t uc_rc_num;
    uint32_t size;

    pst_country_param = (mac_cfg_country_stru *)puc_param;
    pst_mac_regdom = (mac_regdomain_info_stru *)pst_country_param->p_mac_regdom;
    if (pst_mac_regdom == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_country::pst_mac_regdom null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == NULL)) {
        if (pst_mac_regdom != NULL) {
            oal_mem_free_m(pst_mac_regdom, OAL_TRUE);
            pst_mac_regdom = NULL;
        }
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_country::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

#if defined(_PRE_WLAN_FEATURE_HID2D) && defined(_PRE_WLAN_FEATURE_HID2D_PRESENTATION)
    ret = hmac_fix_country_in_hid2d_mode(pst_mac_vap, pst_mac_device);
    if (ret != OAL_SUCC) {
        return ret;
    }
#endif

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_country::sideband_flag[%d].}",
                     pst_mac_regdom->en_regdomain);

    hmac_config_set_dfs_params(pst_mac_device, pst_mac_regdom);

    mac_regdomain_set_country(us_len, puc_param);

#ifdef _PRE_WLAN_FEATURE_DFS
    /* 只有5G 芯片才进行雷达信道初始化 */
    if (pst_mac_device->st_dfs.st_dfs_info.en_dfs_init == OAL_FALSE) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_country::hmac_dfs_channel_list_init.}");
        hmac_dfs_channel_list_init(pst_mac_device);
        pst_mac_device->st_dfs.st_dfs_info.en_dfs_init = OAL_TRUE;
        hmac_config_ch_status_sync(pst_mac_device);
    }
#endif

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    /* 获取管制类的个数 */
    uc_rc_num = pst_mac_regdom->uc_regclass_num;

    /* 计算配置命令 */
    size = (uint32_t)(sizeof(mac_regclass_info_stru) * uc_rc_num + MAC_RD_INFO_LEN);

    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_COUNTRY, (uint16_t)size, (uint8_t *)pst_mac_regdom);
    if (oal_unlikely(ret != OAL_SUCC)) {
        if (pst_mac_regdom != NULL) {
            oal_mem_free_m(pst_mac_regdom, OAL_TRUE);
            pst_mac_regdom = NULL;
        }
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_country::send event failed[%d].}", ret);

        return ret;
    }

    /* WAL层抛内存下来，此处释放 */
    if (pst_mac_regdom != NULL) {
        oal_mem_free_m(pst_mac_regdom, OAL_TRUE);
        pst_mac_regdom = NULL;
    }

    return OAL_SUCC;
}


uint32_t hmac_config_set_amsdu_tx_on(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_ampdu_tx_on_param_stru *pst_ampdu_tx_on_param = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_set_amsdu_tx_on:: parma null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_amsdu_tx_on::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_ampdu_tx_on_param = (mac_cfg_ampdu_tx_on_param_stru *)puc_param;

    mac_mib_set_CfgAmsduTxAtive(&pst_hmac_vap->st_vap_base_info, pst_ampdu_tx_on_param->uc_aggr_tx_on);

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_amsdu_tx_on::ENABLE[%d].}", pst_ampdu_tx_on_param->uc_aggr_tx_on);

    return OAL_SUCC;
}


uint32_t hmac_config_set_ampdu_tx_on(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_ampdu_tx_on_param_stru *p_ampdu_tx_on = NULL;
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    hmac_vap_stru *hmac_vap;
    uint8_t tid_idx;
    uint8_t vap_idx;
    uint8_t dev_idx;
    uint8_t dev_max;
    hmac_user_stru *hmac_user;
    mac_chip_stru *mac_chip;
    mac_device_stru *mac_device;
    oal_dlist_head_stru *entry;
    oal_dlist_head_stru *next_entry;
    mac_user_stru *mac_user;
#endif

    if (oal_unlikely(oal_any_null_ptr2(mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_ampdu_tx_on:: param null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    p_ampdu_tx_on = (mac_cfg_ampdu_tx_on_param_stru *)puc_param;

    oam_warning_log3(0, OAM_SF_CFG, "{hmac_config_set_ampdu_tx_on:: tx_aggr_on[0x%x], snd type[%d],aggr mode[%d]!}",
        p_ampdu_tx_on->uc_aggr_tx_on, p_ampdu_tx_on->uc_snd_type, p_ampdu_tx_on->en_aggr_switch_mode);

    /* ampdu_tx_on为0、1,删建聚合 */
    if ((uint8_t)(p_ampdu_tx_on->uc_aggr_tx_on & (~(BIT1 | BIT0))) == 0) {
        mac_mib_set_CfgAmpduTxAtive(mac_vap, p_ampdu_tx_on->uc_aggr_tx_on & BIT0);

        /* ampdu_tx_on为2、3,删建聚合,并且切换硬件聚合 */
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
        /* 切换为硬件聚合时才需要下发事件 */
        if (p_ampdu_tx_on->uc_aggr_tx_on & BIT1) {
            p_ampdu_tx_on->uc_aggr_tx_on &= BIT0; /* enable hw ampdu */
            hmac_config_send_event(mac_vap, WLAN_CFGID_AMPDU_TX_ON, us_len, puc_param);
        }
#endif
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    /* 1.无需删建BA切换方式 */
    if (p_ampdu_tx_on->en_aggr_switch_mode == AMPDU_SWITCH_BY_BA_LUT) {
        p_ampdu_tx_on->uc_aggr_tx_on &= BIT2; /* 4:enable hw ampdu; 8:disable */
        p_ampdu_tx_on->uc_aggr_tx_on = p_ampdu_tx_on->uc_aggr_tx_on >> NUM_2_BITS;
        hmac_config_send_event(mac_vap, WLAN_CFGID_AMPDU_TX_ON, us_len, puc_param);
        return OAL_SUCC;
    }

    /* 2.需删建BA切换方式 */
    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_ampdu_tx_on:: hmac_vap null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (p_ampdu_tx_on->uc_aggr_tx_on & (~(BIT1 | BIT0))) {
        /* 提交切换硬件聚合work */
        memcpy_s(&hmac_vap->st_mode_set, sizeof(mac_cfg_ampdu_tx_on_param_stru),
                 p_ampdu_tx_on, sizeof(mac_cfg_ampdu_tx_on_param_stru));
        oal_workqueue_delay_schedule(&(hmac_vap->st_set_hw_work), oal_msecs_to_jiffies(2000)); /* 延迟2000ms */

        /* 遍历删除BA */
        mac_chip = hmac_res_get_mac_chip(mac_vap->uc_chip_id);
        if (oal_unlikely(mac_chip == NULL)) {
            oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_ampdu_tx_on:: mac_chip null!}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        dev_max = oal_chip_get_device_num(mac_chip->chip_ver);

        for (dev_idx = 0; dev_idx < dev_max; dev_idx++) {
            mac_device = mac_res_get_dev(mac_chip->auc_device_id[dev_idx]);
            if (mac_device == NULL) {
                continue;
            }

            for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
                hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_device->auc_vap_id[vap_idx]);
                if (hmac_vap == NULL) {
                    continue;
                }

                oal_spin_lock_bh(&hmac_vap->st_ampdu_lock);

                /* 先删除聚合,切换完毕后再使能聚合 */
                mac_mib_set_CfgAmpduTxAtive(&hmac_vap->st_vap_base_info, OAL_FALSE);

                oal_dlist_search_for_each_safe(entry, next_entry, &(hmac_vap->st_vap_base_info.st_mac_user_list_head))
                {
                    mac_user = oal_dlist_get_entry(entry, mac_user_stru, st_user_dlist);
                    if (mac_user == NULL) {
                        continue;
                    }

                    hmac_user = mac_res_get_hmac_user(mac_user->us_assoc_id);
                    if (hmac_user == NULL) {
                        continue;
                    }

                    for (tid_idx = 0; tid_idx < WLAN_TID_MAX_NUM; tid_idx++) {
                        if (hmac_user->ast_tid_info[tid_idx].st_ba_tx_info.en_ba_status == DMAC_BA_COMPLETE) {
                            hmac_tx_ba_del(hmac_vap, hmac_user, tid_idx);
                        }
                    }
                }

                oal_spin_unlock_bh(&hmac_vap->st_ampdu_lock);
                oal_workqueue_delay_schedule(&(hmac_vap->st_ampdu_work), oal_msecs_to_jiffies(3000)); /* 延迟3000ms */
            }
        }
    }
#endif

    return OAL_SUCC;
}


uint32_t hmac_config_get_ampdu_tx_on(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    *((uint32_t *)puc_param) = mac_mib_get_CfgAmpduTxAtive(pst_mac_vap);
    *pus_len = sizeof(int32_t);

    return OAL_SUCC;
}


uint32_t hmac_config_get_amsdu_tx_on(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    *((uint32_t *)puc_param) = mac_mib_get_CfgAmsduTxAtive(pst_mac_vap);
    *pus_len = sizeof(int32_t);

    return OAL_SUCC;
}


uint32_t hmac_test_encap_mgmt_action(mac_vap_stru *pst_mac_vap,
                                     uint8_t *puc_buff, uint8_t *puc_param, uint32_t param_len)
{
    oal_netbuf_stru *pst_buffer = (oal_netbuf_stru *)puc_buff;
    uint8_t *puc_mac_header = oal_netbuf_header(pst_buffer);
    uint8_t *puc_payload_addr = mac_netbuf_get_payload(pst_buffer);
    uint8_t *puc_payload_addr_origin = puc_payload_addr;
    mac_cfg_send_frame_param_stru *pst_param = (mac_cfg_send_frame_param_stru *)puc_param;
    uint32_t frame_length = 0;
    uint16_t us_frame_control;
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
    /*                        设置帧头                                      */
    /*************************************************************************/
    /* 帧控制字段全为0，除了type和subtype */
    us_frame_control = WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION;
    mac_hdr_set_frame_control(puc_mac_header, us_frame_control);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(puc_mac_header, 0);

    /* 设置地址1 */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR1_OFFSET, pst_param->auc_mac_ra);

    /* 设置地址2为自己的MAC地址 */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(pst_mac_vap));

    /* 地址3 bssid */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR3_OFFSET, pst_mac_vap->auc_bssid);

    if (puc_payload_addr == NULL) {
        return frame_length;
    }

    if (memcpy_s(puc_payload_addr, WLAN_MEM_NETBUF_SIZE2,
        pst_param->uc_frame_body, pst_param->uc_frame_body_length) != EOK) {
        oam_warning_log0(0, OAM_SF_ANY, "dmac_test_encap_mgmt_action::memcpy fail!");
    }

    puc_payload_addr += pst_param->uc_frame_body_length;
    frame_length = (uint16_t)((puc_payload_addr - puc_payload_addr_origin) + MAC_80211_FRAME_LEN);

    return frame_length;
}

uint32_t hmac_config_send_frame_timer(void *p_arg)
{
    oal_netbuf_stru *pst_netbuf = NULL;
    mac_tx_ctl_stru *pst_tx_ctl = NULL;
    uint32_t ret;
    mac_vap_stru *pst_mac_vap = NULL;
    mac_device_stru *pst_mac_device = NULL;
    mac_user_stru *pst_mac_user = NULL;
    mac_cfg_send_frame_param_stru *pst_param = (mac_cfg_send_frame_param_stru *)p_arg;
    uint32_t len = 0;
    uint16_t us_assoc_id = 0;
    uint32_t frame_len;
    oal_bool_enum_uint8 en_is_bcast_addr;

    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_param->uc_vap_idx);
    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_send_frame_timer::mac_res_get_mac_vap fail}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    en_is_bcast_addr = oal_is_broadcast_ether_addr(pst_param->auc_mac_ra);
    if (en_is_bcast_addr) {
        us_assoc_id = pst_mac_vap->us_multi_user_idx;
    } else {
        pst_mac_user = mac_vap_get_user_by_addr(pst_mac_vap, pst_param->auc_mac_ra);
        if (pst_mac_user == NULL) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_send_frame_timer::cannot find user by addr}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        us_assoc_id = pst_mac_user->us_assoc_id;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 发送帧计数 */
    pst_param->uc_frame_cnt++;

    /* 申请帧内存 */
    pst_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_send_frame_timer::cannot alloc netbuff fail.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    oal_mem_netbuf_trace(pst_netbuf, OAL_TRUE);
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    /* 使用这个接口时要小心，cb是不是已经被之前的流程填了一部分 */
    memset_s(pst_tx_ctl, sizeof(mac_tx_ctl_stru), 0, sizeof(mac_tx_ctl_stru));

    /* 组帧 */
    frame_len = hmac_test_encap_mgmt_action(pst_mac_vap, (uint8_t *)(pst_netbuf), p_arg, len);
    if (frame_len == 0) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_send_frame_timer::ul_frame_len=0.}");
        oal_netbuf_free(pst_netbuf);
        return OAL_FAIL;
    }

    oal_netbuf_put(pst_netbuf, frame_len);

    if (en_is_bcast_addr) {
        MAC_GET_CB_IS_MCAST(pst_tx_ctl) = OAL_TRUE;
    }

    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = us_assoc_id;
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl) = WLAN_WME_AC_MGMT;
    MAC_GET_CB_MPDU_NUM(pst_tx_ctl) = 1;
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = frame_len; /* dmac发送需要的mpdu长度 */

    /* Buffer this frame in the Memory Queue for transmission */
    ret = hmac_tx_mgmt_send_event(pst_mac_vap, pst_netbuf, frame_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_send_frame_timer::hmac_tx_mgmt_send_event failed[%d].}", ret);
        return ret;
    }

    if (pst_param->uc_frame_cnt < pst_param->uc_pkt_num) {
        frw_timer_create_timer_m(&(pst_mac_device->st_send_frame), hmac_config_send_frame_timer, 10, /* 10ms 触发一次 */
                                 pst_param, OAL_FALSE, OAM_MODULE_ID_HMAC, pst_mac_device->core_id);
    } else {
        oal_mem_free_m(pst_param, OAL_TRUE);
    }

    return OAL_SUCC;
}


uint32_t hmac_config_send_frame(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_send_frame_param_stru *pst_param = NULL;
    mac_device_stru *pst_mac_device = NULL;
    int32_t ret;

    pst_param = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, sizeof(mac_cfg_send_frame_param_stru), OAL_TRUE);
    if (pst_param == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_config_send_frame::melloc err!");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = memcpy_s(pst_param, sizeof(mac_cfg_send_frame_param_stru),
                   puc_param, sizeof(mac_cfg_send_frame_param_stru));
    if (ret != EOK) {
        oal_mem_free_m(pst_param, OAL_TRUE);
        oam_error_log1(0, OAM_SF_SCAN, "hmac_config_send_frame::memcpy fail! ret[%d]", ret);
        return OAL_FAIL;
    }

    pst_param->uc_vap_idx = pst_mac_vap->uc_vap_id;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oal_mem_free_m(pst_param, OAL_TRUE);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                     "{hmac_config_send_frame::frame_types = %d send_times = %d body_len=%d.}",
                     pst_param->en_frame_type, pst_param->uc_pkt_num, pst_param->uc_frame_body_length);

    if (pst_param->uc_pkt_num > 0) {
        frw_timer_create_timer_m(&(pst_mac_device->st_send_frame), hmac_config_send_frame_timer, 10, /* 10ms 触发一次 */
                                 pst_param, OAL_FALSE, OAM_MODULE_ID_HMAC, pst_mac_device->core_id);
    } else {
        oal_mem_free_m(pst_param, OAL_TRUE);
    }

    return OAL_SUCC;
}


uint32_t hmac_config_set_country_for_dfs(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_COUNTRY_FOR_DFS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_country_for_dfs::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_TPC_OPT

uint32_t hmac_config_reduce_sar(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_REDUCE_SAR, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_config_reduce_sar::hmac_config_send_event failed, error no[%d]!", ret);
        return ret;
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH

uint32_t hmac_config_tas_pwr_ctrl(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TAS_PWR_CTRL, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_config_tas_pwr_ctrl::hmac_config_send_event failed, error no[%d]!", ret);
        return ret;
    }
    return OAL_SUCC;
}
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH

uint32_t hmac_config_tas_rssi_access(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TAS_RSSI_ACCESS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_config_tas_rssi_access::hmac_config_send_event failed, error no[%d]!", ret);
        return ret;
    }
    return OAL_SUCC;
}
#endif


uint32_t hmac_config_get_country(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    mac_regdomain_info_stru *pst_regdomain_info = NULL;
    mac_cfg_get_country_stru *pst_param;

    pst_param = (mac_cfg_get_country_stru *)puc_param;

    mac_get_regdomain_info(&pst_regdomain_info);

    pst_param->ac_country[BYTE_OFFSET_0] = pst_regdomain_info->ac_country[BYTE_OFFSET_0];
    pst_param->ac_country[BYTE_OFFSET_1] = pst_regdomain_info->ac_country[BYTE_OFFSET_1];
    pst_param->ac_country[BYTE_OFFSET_2] = pst_regdomain_info->ac_country[BYTE_OFFSET_2];
    *pus_len = WLAN_COUNTRY_STR_LEN;

    oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_get_country");

    return OAL_SUCC;
}


OAL_STATIC void hmac_free_connect_param_resource(hmac_conn_param_stru *pst_conn_param)
{
    if (pst_conn_param->puc_wep_key != NULL) {
        oal_free(pst_conn_param->puc_wep_key);
        pst_conn_param->puc_wep_key = NULL;
    }
    if (pst_conn_param->puc_ie != NULL) {
        oal_free(pst_conn_param->puc_ie);
        pst_conn_param->puc_ie = NULL;
    }
}

OAL_STATIC uint32_t hmac_config_connect_param_check(uint16_t us_len, uint8_t *puc_param)
{
    if (oal_unlikely(puc_param == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_connect:: connect failed, puc_param is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (us_len != sizeof(hmac_conn_param_stru)) {
        oam_error_log1(0, OAM_SF_ANY,
            "{hmac_config_connect:: connect failed, unexpected param len ! [%x]!}", us_len);
        hmac_free_connect_param_resource((hmac_conn_param_stru *)puc_param);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef _PRE_WLAN_FEATURE_DDR_BUGFIX

OAL_STATIC void hmac_remove_ddr_freq_req(void)
{
    if (g_pst_auto_ddr_freq == NULL || g_ddr_qos_req_count <= 0) {
        return;
    }

    pm_qos_remove_request(g_pst_auto_ddr_freq);
    g_ddr_qos_req_count--;
}


OAL_STATIC void hmac_add_ddr_freq_req(void)
{
    if (g_pst_auto_ddr_freq == NULL) {
        g_pst_auto_ddr_freq = kzalloc(sizeof(*g_pst_auto_ddr_freq), GFP_KERNEL);
        return;
    }

#ifdef CONFIG_DEVFREQ_GOV_PM_QOS
    if (g_pst_auto_ddr_freq != NULL && g_ddr_qos_req_count == 0) {
        pm_qos_add_request(g_pst_auto_ddr_freq, PM_QOS_MEMORY_LATENCY, g_ddr_freq);
        g_ddr_qos_req_count++;
    }
#endif
}


OAL_STATIC uint32_t hmac_set_ddr(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    if (*puc_param) {
        hmac_add_ddr_freq_req();
    } else {
        hmac_remove_ddr_freq_req();
    }
    return OAL_SUCC;
}
#endif
#endif


OAL_STATIC uint32_t hmac_connect_dual_sta_check(mac_vap_stru *mac_vap, hmac_conn_param_stru *connect_param)
{
    mac_device_stru *mac_dev = NULL;
    mac_vap_stru *another_up_vap = NULL;

    /* 只在双sta模式下，检查wlan1 */
    if (!mac_is_dual_sta_mode() || !mac_is_secondary_sta(mac_vap)) {
        return OAL_SUCC;
    }

    mac_dev = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_dev == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_connect_dual_sta_check::dev[%d] ptr is null}", mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    another_up_vap = mac_device_find_another_up_vap(mac_dev, mac_vap->uc_vap_id);
    if (another_up_vap == NULL) {
        /* wlan1先于wlan0入网 */
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_dual_sta_connect_check::wlan1 trying to connect before wlan0!!!}");
        return OAL_FAIL;
    }

    /* wlan0关联了wapi网络，不允许wlan1连接 */
    if (hmac_user_is_wapi_connected(mac_vap->uc_device_id)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_dual_sta_connect_check::wlan1 cannot connect while wlan0 is connected using wapi!!!}");
        return OAL_FAIL;
    }

    if (!mac_is_primary_legacy_vap(another_up_vap)) {
        oam_warning_log2(another_up_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_dual_sta_connect_check::Not primary vap, vap mode[%d] p2p mode[%d]}", another_up_vap->en_vap_mode,
            another_up_vap->en_p2p_mode);
        return OAL_FAIL;
    }

    /* wlan1关联频段和wlan0相同 */
    if (another_up_vap->st_channel.en_band == mac_get_band_by_channel_num(connect_param->uc_channel)) {
        oam_warning_log0(another_up_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_dual_sta_connect_check::wlan1 trying to connect the same band of wlan0!!!}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

uint32_t hmac_process_connect_by_vap_state(mac_vap_stru *mac_vap, hmac_vap_stru *hmac_vap,
                                           hmac_conn_param_stru *connect_param)
{
    hmac_user_stru *hmac_user = NULL;
    uint32_t ret;

    if (mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING) {
        /* 通知ROAM主状态机, ABORT Roaming FSM */
        hmac_roam_connect_complete(hmac_vap, OAL_FAIL);
    }

    if (mac_vap->en_vap_state == MAC_VAP_STATE_UP) {
        /* 相同ssid时，走漫游流程 */
        if (!oal_memcmp(mac_mib_get_DesiredSSID(mac_vap), connect_param->auc_ssid, connect_param->uc_ssid_len) &&
            (connect_param->uc_ssid_len == OAL_STRLEN(mac_mib_get_DesiredSSID(mac_vap)))) {
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_process_connect_by_vap_state::roaming AP with ressoc frame, XX:XX:XX:%02X:%02X:%02X}",
                connect_param->auc_bssid[MAC_ADDR_3],
                connect_param->auc_bssid[MAC_ADDR_4], connect_param->auc_bssid[MAC_ADDR_5]);

            if (!oal_memcmp(mac_vap->auc_bssid, connect_param->auc_bssid, OAL_MAC_ADDR_LEN)) {
                /* reassociation */
                ret = hmac_roam_start(hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_FALSE, NULL, ROAM_TRIGGER_APP);
            } else {
                /* roaming */
                ret = hmac_roam_start(hmac_vap, ROAM_SCAN_CHANNEL_ORG_DBDC, OAL_TRUE,
                    connect_param->auc_bssid, ROAM_TRIGGER_BSSID);
            }

            /* reassociation failure need delete user first, and then connect again (MaxDepth) */
            if (ret == OAL_SUCC) {
                hmac_free_connect_param_resource(connect_param);
                return OAL_SUCC;
            }
        }

        /* 先删除用户，再connect */
        hmac_user = mac_res_get_hmac_user(mac_vap->us_assoc_vap_id);
        if (hmac_user != NULL) {
            hmac_user_del(mac_vap, hmac_user);
        }
    }

    if ((mac_vap->en_vap_state >= MAC_VAP_STATE_STA_JOIN_COMP) &&
        (mac_vap->en_vap_state <= MAC_VAP_STATE_STA_WAIT_ASOC)) {
        hmac_user = mac_res_get_hmac_user(mac_vap->us_assoc_vap_id);
        if (hmac_user != NULL) {
            oal_bool_enum_uint8 en_is_protected;

            oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_process_connect_by_vap_state:: \
                             deauth the connecting AP, vap id=%d, state=%d, XX:XX:XX:XX:%02X:%02X}",
                             mac_vap->us_assoc_vap_id, mac_vap->en_vap_state,
                             hmac_user->st_user_base_info.auc_user_mac_addr[MAC_ADDR_4],
                             hmac_user->st_user_base_info.auc_user_mac_addr[MAC_ADDR_5]);

            en_is_protected = hmac_user->st_user_base_info.st_cap_info.bit_pmf_active;
            /* 发去认证帧 */
            hmac_mgmt_send_disassoc_frame(mac_vap, hmac_user->st_user_base_info.auc_user_mac_addr,
                                          MAC_DISAS_LV_SS, en_is_protected);

            /* 删除用户 */
            hmac_user_del(mac_vap, hmac_user);
        }
    }

    return OAL_SUCC_GO_ON;
}

#ifdef _PRE_WLAN_FEATURE_HS20

static uint32_t hmac_interworking_check(hmac_vap_stru *pst_hmac_vap, uint8_t *puc_param)
{
    uint8_t *puc_extend_cap_ie;
    mac_bss_dscr_stru *pst_bss_dscr;

    if (oal_unlikely(oal_any_null_ptr2(pst_hmac_vap, puc_param))) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_interworking_check:: check failed, null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_bss_dscr = (mac_bss_dscr_stru *)puc_param;
    if (pst_bss_dscr->mgmt_len < (MAC_80211_FRAME_LEN + MAC_SSID_OFFSET)) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                         "{hmac_interworking_check:: mgmt_len(%d) < (80211_FRAME_LEN+SSID_OFFSET).}",
                         pst_bss_dscr->mgmt_len);
        return OAL_FAIL;
    }

    /* 查找interworking ie */
    /*lint -e416*/
    puc_extend_cap_ie = mac_find_ie(MAC_EID_EXT_CAPS,
                                    pst_bss_dscr->auc_mgmt_buff + MAC_80211_FRAME_LEN + MAC_SSID_OFFSET,
                                    (int32_t)(pst_bss_dscr->mgmt_len - MAC_80211_FRAME_LEN - MAC_SSID_OFFSET));
    /*lint +e416*/
    if (puc_extend_cap_ie == NULL) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                         "{hmac_interworking_check:: puc_extend_cap_ie is NULL, the mgmt_len is %d.}",
                         pst_bss_dscr->mgmt_len);
        return OAL_FAIL;
    }

    /*  未检测到interworking能力位，返回fail */
    if (puc_extend_cap_ie[1] < NUM_4_BITS || !(puc_extend_cap_ie[BYTE_OFFSET_5] & 0x80)) {
        pst_hmac_vap->st_vap_base_info.st_cap_flag.bit_is_interworking = OAL_FALSE;
        return OAL_FAIL;
    }

    pst_hmac_vap->st_vap_base_info.st_cap_flag.bit_is_interworking = OAL_TRUE;
    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_HS20


uint32_t hmac_config_connect(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    hmac_vap_stru *hmac_vap = NULL;
    mac_bss_dscr_stru *bss_dscr = NULL;
    hmac_conn_param_stru *connect_param = NULL;
#ifdef _PRE_WLAN_FEATURE_11R
    uint8_t *mde = NULL;
    uint8_t akm_type;
#endif
    oal_app_ie_stru app_ie;
    mac_conn_security_stru conn_sec;
#ifdef _PRE_WLAN_FEATURE_WAPI
    mac_device_stru *mac_device;
#endif
    int32_t l_ret;
    uint8_t chain_index;

    l_ret = hmac_config_connect_param_check(us_len, puc_param);
    if (l_ret != OAL_SUCC) {
        return l_ret;
    }

    connect_param = (hmac_conn_param_stru *)puc_param;
    if (connect_param->ie_len > WLAN_WPS_IE_MAX_SIZE) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_config_connect::connect failed, \
                       connect_param ie_len[%x] error!}", connect_param->ie_len);
        hmac_free_connect_param_resource(connect_param);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    if (hmac_connect_dual_sta_check(mac_vap, connect_param) != OAL_SUCC) {
        hmac_free_connect_param_resource(connect_param);
        return OAL_FAIL;
    }
    /* 先判断其他VAP 的状态是否允许本VAP 入网连接 */
    /* 如果允许本VAP 入网，则返回设备忙状态           */
    /* 根据内核下发的关联能力，赋值加密相关的mib 值 */
    /* 根据下发的join,提取出安全相关的内容 */
    memset_s(&conn_sec, sizeof(mac_conn_security_stru), 0, sizeof(mac_conn_security_stru));
    conn_sec.uc_wep_key_len = connect_param->uc_wep_key_len;
    conn_sec.en_auth_type = connect_param->en_auth_type;
    conn_sec.en_privacy = connect_param->en_privacy;
    conn_sec.st_crypto = connect_param->st_crypto;
    conn_sec.uc_wep_key_index = connect_param->uc_wep_key_index;
    conn_sec.en_mgmt_proteced = connect_param->en_mfp;
    if (conn_sec.uc_wep_key_len > WLAN_WEP104_KEY_LEN) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_config_connect:: wep_key_len[%d] > WLAN_WEP104_KEY_LEN!}", conn_sec.uc_wep_key_len);
        conn_sec.uc_wep_key_len = WLAN_WEP104_KEY_LEN;
    }
    l_ret = memcpy_s(conn_sec.auc_wep_key, sizeof(conn_sec.auc_wep_key),
                     connect_param->puc_wep_key, conn_sec.uc_wep_key_len);

#ifdef _PRE_WLAN_FEATURE_PMF
    conn_sec.en_pmf_cap = mac_get_pmf_cap(connect_param->puc_ie, connect_param->ie_len);
    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_connect:: connect param en_mfp[%d] pmf_cap[%d]!}",
                     conn_sec.en_mgmt_proteced, conn_sec.en_pmf_cap);
#endif
    conn_sec.en_wps_enable = OAL_FALSE;
    if (mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS,
        connect_param->puc_ie, (int32_t)(connect_param->ie_len))) {
        conn_sec.en_wps_enable = OAL_TRUE;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_connect::connect failed, hmac_vap null.uc_vap_id[%d]}", mac_vap->uc_vap_id);
        hmac_free_connect_param_resource(connect_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_mib_set_AuthenticationMode(mac_vap, conn_sec.en_auth_type);

#ifdef _PRE_WLAN_FEATURE_11R
    if (hmac_vap->bit_11r_enable == OAL_TRUE) {
        akm_type = oal_ntoh_32(conn_sec.st_crypto.aul_akm_suite[0]) & 0xFF;
        if (oal_value_eq_any3(akm_type, WLAN_AUTH_SUITE_FT_1X, WLAN_AUTH_SUITE_FT_PSK, WLAN_AUTH_SUITE_FT_SHA256)) {
            mac_mib_set_AuthenticationMode(&hmac_vap->st_vap_base_info, WLAN_WITP_AUTH_FT);
        }
    }
#endif

    /* 获取扫描的bss信息 */
    bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(mac_vap, connect_param->auc_bssid);
    if (bss_dscr == NULL) {
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_connect::find bss failed by bssid:%02X:XX:XX:%02X:%02X:%02X}", connect_param->auc_bssid[0],
            connect_param->auc_bssid[3], connect_param->auc_bssid[4], connect_param->auc_bssid[5]);
        hmac_free_connect_param_resource(connect_param);
        return OAL_FAIL;
    }
#ifdef _PRE_WLAN_FEATURE_MBO
    /* MBO STA获取AP的MBO IE中表示不允许关联或re-assoc delay的子元素，STA不与(或re-assoc delay time内不与)AP关联 */
    ret = hmac_mbo_check_is_assoc_or_re_assoc_allowed(mac_vap, connect_param, bss_dscr);
    if (ret != OAL_TRUE) {
        hmac_free_connect_param_resource(connect_param);
        return OAL_FAIL;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_PMF
    hmac_pmf_update_pmf_black_list(mac_vap, bss_dscr, conn_sec.en_mgmt_proteced);
#endif

    /* check bssid blacklist from Framework/WIFI HAL Configuration */
    ret = hmac_blacklist_filter(mac_vap, bss_dscr->auc_bssid);
    if (ret == OAL_TRUE) {
        hmac_free_connect_param_resource(connect_param);
        return OAL_FAIL;
    }

    if (oal_memcmp(connect_param->auc_ssid, bss_dscr->ac_ssid, (uint32_t)connect_param->uc_ssid_len)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_connect::find the bss failed by ssid.}");
        hmac_free_connect_param_resource(connect_param);
        return OAL_FAIL;
    }

    if ((connect_param->uc_channel != bss_dscr->st_channel.uc_chan_number) && (connect_param->uc_channel != 0)) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_connect::p2p_mode=%d, connect req channel=%u, bss_dscr chan_number=%u in scan results.}",
            mac_vap->en_p2p_mode, connect_param->uc_channel, bss_dscr->st_channel.uc_chan_number);
        hmac_free_connect_param_resource(connect_param);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_HS20
    if (oal_memcmp(mac_vap->auc_bssid, connect_param->auc_bssid, OAL_MAC_ADDR_LEN) ||
        (hmac_interworking_check(hmac_vap, (uint8_t *)bss_dscr))) {
        hmac_vap->bit_reassoc_flag = OAL_FALSE;
    } else {
        hmac_vap->bit_reassoc_flag = OAL_TRUE;
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_connect:: assoc ap with ressoc frame.}");
    }

    /* passpoint网络中暂不考虑重关联 */
#endif  // _PRE_WLAN_FEATURE_HS20
    hmac_vap->bit_reassoc_flag = OAL_FALSE;

    /* 不同的vap state走不同的流程 */
    ret = hmac_process_connect_by_vap_state(mac_vap, hmac_vap, connect_param);
    if (ret != OAL_SUCC_GO_ON) {
        return ret;
    }

#ifdef _PRE_WLAN_FEATURE_WAPI
    bss_dscr->uc_wapi = connect_param->uc_wapi;
    if (bss_dscr->uc_wapi) {
        conn_sec.is_wapi_connect = OAL_TRUE;
        mac_device = mac_res_get_dev(mac_vap->uc_device_id);
        if (mac_device == NULL) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_connect::connect failed, mac_device null! uc_device_id[%d]}", mac_vap->uc_device_id);
            hmac_free_connect_param_resource(connect_param);
            return OAL_ERR_CODE_MAC_DEVICE_NULL;
        }

        if (mac_device_is_p2p_connected(mac_device) == OAL_SUCC) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_connect:: wapi connect failed for p2p having been connected!.}");
            hmac_free_connect_param_resource(connect_param);
            return OAL_FAIL;
        }
    }
#endif

    /* 设置P2P/WPS IE 信息到 vap 结构体中 */
    if (IS_LEGACY_VAP(mac_vap)) {
        hmac_config_del_p2p_ie(connect_param->puc_ie, &(connect_param->ie_len));
    }
    app_ie.ie_len = connect_param->ie_len;
    l_ret += memcpy_s(app_ie.auc_ie, WLAN_WPS_IE_MAX_SIZE, connect_param->puc_ie, app_ie.ie_len);
    app_ie.en_app_ie_type = OAL_APP_ASSOC_REQ_IE;
    hmac_free_connect_param_resource(connect_param);
    ret = hmac_config_set_app_ie_to_vap(mac_vap, &app_ie, app_ie.en_app_ie_type);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_connect::hmac_config_set_app_ie_to_vap fail,err_code=%d.}", ret);
        return ret;
    }
    mac_mib_set_dot11dtimperiod(mac_vap, bss_dscr->uc_dtim_period);

    /* 设置关联用户的能力信息 */
    mac_vap->us_assoc_user_cap_info = bss_dscr->us_cap_info;
    mac_vap->bit_ap_11ntxbf = (bss_dscr->en_11ntxbf == OAL_TRUE);

    /* 入网选择的热点rssi，同步到dmac传给tpc算法做管理报文tpc */
    conn_sec.c_rssi = bss_dscr->c_rssi;
    for (chain_index = 0; chain_index < HD_EVENT_RF_NUM; chain_index++) {
        conn_sec.rssi[chain_index] = bss_dscr->rssi[chain_index];
    }

    ret = mac_vap_init_privacy(mac_vap, &conn_sec);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_connect::mac_vap_init_privacy fail[%d]!}", ret);
        return ret;
    }

    hmac_roam_check_psk(hmac_vap, &conn_sec);
    mac_mib_set_WPSActive(mac_vap, conn_sec.en_wps_enable);

#ifdef _PRE_WLAN_FEATURE_11R
    if (hmac_vap->bit_11r_enable == OAL_TRUE) {
        mde = mac_find_ie(MAC_EID_MOBILITY_DOMAIN, bss_dscr->auc_mgmt_buff + MAC_80211_FRAME_LEN + MAC_SSID_OFFSET,
                          bss_dscr->mgmt_len - MAC_80211_FRAME_LEN - MAC_SSID_OFFSET);
        if (mde != NULL) {
            l_ret += memcpy_s(conn_sec.auc_mde, sizeof(conn_sec.auc_mde), mde, mde[1] + MAC_IE_HDR_LEN);
        }
        mac_mib_init_ft_cfg(mac_vap, conn_sec.auc_mde);
    }
#endif  // _PRE_WLAN_FEATURE_11R
    if (l_ret != EOK) {
        oam_warning_log1(0, OAM_SF_WPA, "hmac_config_connect::memcpy fail! l_ret[%d]", l_ret);
    }

#ifdef _PRE_WLAN_FEATURE_SAE
    hmac_update_sae_connect_param(hmac_vap, app_ie.auc_ie, app_ie.ie_len);
#endif /* _PRE_WLAN_FEATURE_SAE */

    ret = hmac_check_capability_mac_phy_supplicant(mac_vap, bss_dscr);
    /***************************************************************************
    抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_CONNECT_REQ, sizeof(conn_sec), (uint8_t *)&conn_sec);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_WPA,
                       "{hmac_config_connect::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return hmac_sta_initiate_join(mac_vap, bss_dscr);
}


uint32_t hmac_config_get_tid(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_dev;
    mac_cfg_get_tid_stru *pst_tid;

    pst_tid = (mac_cfg_get_tid_stru *)puc_param;
    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_tid->en_tid = pst_mac_dev->en_tid;
    *pus_len = sizeof(pst_tid->en_tid);

    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_tid::en_tid=%d.}", pst_tid->en_tid);
    return OAL_SUCC;
}


uint32_t hmac_config_list_channel(mac_vap_stru *pst_mac_vap)
{
    uint8_t chan_num;
    uint8_t chan_idx;
    uint32_t ret;

    for (chan_idx = 0; chan_idx < MAC_CHANNEL_FREQ_2_BUTT; chan_idx++) {
        ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_2, chan_idx, OAL_FALSE);
        if (ret == OAL_SUCC) {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_2, chan_idx, OAL_FALSE, &chan_num);

            /* 输出2G信道号 */
            oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_list_channel::2gCHA.NO=%d}", chan_num);
        }
    }
    if (mac_device_band_is_support(pst_mac_vap->uc_device_id, MAC_DEVICE_CAP_5G) == OAL_FALSE) {
        return OAL_SUCC;
    }

    for (chan_idx = 0; chan_idx < MAC_CHANNEL_FREQ_5_BUTT / 2; chan_idx++) { /* 2：36-124信道 */
        ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, chan_idx, OAL_FALSE);
        if (ret == OAL_SUCC) {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_5, chan_idx, OAL_FALSE, &chan_num);

#ifdef _PRE_WLAN_FEATURE_DFS
            /* 检测5G 36~120信道上的DFS雷达标记 */
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_list_channel::5gCHA.NO=%d,DFS_REQUIRED[%c]}",
                chan_num, ((mac_is_ch_in_radar_band(MAC_RC_START_FREQ_5, chan_idx) == OAL_TRUE) ? 'Y' : 'N'));
#endif
        }
    }

    for (chan_idx = MAC_CHANNEL_FREQ_5_BUTT / 2; chan_idx < MAC_CHANNEL_FREQ_5_BUTT; chan_idx++) { /* 2：128-196信道 */
        ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, chan_idx, OAL_FALSE);
        if (ret == OAL_SUCC) {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_5, chan_idx, OAL_FALSE, &chan_num);
#ifdef _PRE_WLAN_FEATURE_DFS
            /* 检测5G 124~196信道上的DFS雷达标记 */
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_list_channel::5gCHA.NO=%d,DFS_REQUIRED[%c]}",
                chan_num, ((mac_is_ch_in_radar_band(MAC_RC_START_FREQ_5, chan_idx) == OAL_TRUE) ? 'Y' : 'N'));
#endif
        }
    }
    return OAL_SUCC;
}


uint32_t hmac_config_set_app_ie_to_vap(mac_vap_stru *mac_vap, oal_app_ie_stru *app_ie, en_app_ie_type_uint8 en_type)
{
    uint32_t ret;
    uint8_t *ie = NULL;
    uint32_t remain_len;
    hmac_vap_stru *hmac_vap = NULL;
    int32_t l_ret = EOK;

    if (oal_any_null_ptr2(mac_vap, app_ie)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_app_ie_to_vap::scan failed, mac_vap or app_ie null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_app_ie_to_vap::hmac_vap null ptr.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 移除驱动侧重复MAC_EID_EXT_CAPS */
    ie = mac_find_ie(MAC_EID_EXT_CAPS, app_ie->auc_ie, (int32_t)app_ie->ie_len);
    if (ie != NULL) {
        app_ie->ie_len -= (uint32_t)(ie[1] + MAC_IE_HDR_LEN);
        remain_len = app_ie->ie_len - (uint32_t)(ie - app_ie->auc_ie);
        l_ret += memmove_s(ie, remain_len + (ie[1] + MAC_IE_HDR_LEN), ie + (ie[1] + MAC_IE_HDR_LEN), remain_len);
    }

    /* remove type 为1表示移除该IE，0为恢复IE，不处理即可，支持处理多个IE需在此扩展 */
    if (hmac_vap->st_remove_ie.uc_type) {
        ie = mac_find_ie(hmac_vap->st_remove_ie.uc_eid, app_ie->auc_ie, (int32_t)app_ie->ie_len);
        if (ie != NULL) {
            app_ie->ie_len -= (uint32_t)(ie[1] + MAC_IE_HDR_LEN);
            remain_len = app_ie->ie_len - (uint32_t)(ie - app_ie->auc_ie);
            l_ret += memmove_s(ie, remain_len + (ie[1] + MAC_IE_HDR_LEN), ie + (ie[1] + MAC_IE_HDR_LEN), remain_len);
        }
    }

    ie = mac_find_ie(MAC_EID_OPERATING_CLASS, app_ie->auc_ie, (int32_t)app_ie->ie_len);
    if ((ie != NULL) && (!mac_mib_get_dot11ExtendedChannelSwitchActivated(mac_vap))) {
        app_ie->ie_len -= (uint32_t)(ie[1] + MAC_IE_HDR_LEN);
        remain_len = app_ie->ie_len - (uint32_t)(ie - app_ie->auc_ie);
        l_ret += memmove_s(ie, remain_len + (ie[1] + MAC_IE_HDR_LEN), ie + (ie[1] + MAC_IE_HDR_LEN), remain_len);
    }
    if (l_ret != EOK) {
        oam_warning_log1(0, OAM_SF_CFG, "hmac_config_set_app_ie_to_vap::memmove fail![%d]", l_ret);
    }

    ret = mac_vap_save_app_ie(mac_vap, app_ie, en_type);
    if (ret != OAL_SUCC) {
        oam_error_log3(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_app_ie_to_vap::save app ie failed[%d], \
                       en_type[%d], len[%d].}", ret, en_type, app_ie->ie_len);
        return ret;
    }

    if (app_ie->en_app_ie_type >= OAL_APP_ASSOC_REQ_IE) {
        /* 只有OAL_APP_BEACON_IE、OAL_APP_PROBE_REQ_IE、OAL_APP_PROBE_RSP_IE 才需要保存到device */
        return OAL_SUCC;
    }

    if (hmac_config_h2d_send_app_ie(mac_vap, app_ie) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_config_set_app_ie_to_vap::hmac_config_h2d_send_app_ie fail!");
    }

    return ret;
}


OAL_STATIC oal_bool_enum_uint8 hmac_config_set_wps_p2p_ie_param_check(mac_vap_stru *pst_mac_vap,
                                                                      oal_w2h_app_ie_stru *pst_w2h_wps_p2p_ie)
{
    if ((pst_w2h_wps_p2p_ie->en_app_ie_type >= OAL_APP_IE_NUM) ||
        (pst_w2h_wps_p2p_ie->ie_len >= WLAN_WPS_IE_MAX_SIZE)) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_wps_p2p_ie::app_ie_type=[%d] app_ie_len=[%d],param invalid.}",
                         pst_w2h_wps_p2p_ie->en_app_ie_type, pst_w2h_wps_p2p_ie->ie_len);
        return OAL_FALSE;
    }
    return OAL_TRUE;
}


uint32_t hmac_config_set_wps_p2p_ie(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    oal_app_ie_stru app_ie;
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t ret;
    uint32_t delta_time;
    oal_w2h_app_ie_stru *w2h_wps_p2p_ie = (oal_w2h_app_ie_stru *)puc_param;

    if (hmac_config_set_wps_p2p_ie_param_check(mac_vap, w2h_wps_p2p_ie) == OAL_FALSE) {
        return OAL_FAIL;
    }

    /* 如果wal to hmac的事件超时，则pst_w2h_wps_p2p_ie->puc_data_ie指向的内存已经释放，不能再处理该事件 */
    delta_time = oal_time_get_runtime(w2h_wps_p2p_ie->delivery_time, oal_time_get_stamp_ms());
    if (delta_time >= (HMAC_WAIT_EVENT_RSP_TIME * HMAC_S_TO_MS)) {
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_wps_p2p_ie::schedule event timeout[%d]ms, \
            delta_time[%d]ms, app_ie_type[%d] app_ie_len[%d].}", HMAC_WAIT_EVENT_RSP_TIME * HMAC_S_TO_MS, delta_time,
            w2h_wps_p2p_ie->en_app_ie_type, w2h_wps_p2p_ie->ie_len);
        return OAL_FAIL;
    }

    if (!hmac_get_feature_switch(HMAC_MIRACAST_REDUCE_LOG_SWITCH)) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_wps_p2p_ie::p2p_ie_type=[%d],p2p_ie_len=[%d].}",
                         w2h_wps_p2p_ie->en_app_ie_type, w2h_wps_p2p_ie->ie_len);
    }

    memset_s(&app_ie, sizeof(app_ie), 0, sizeof(app_ie));
    app_ie.en_app_ie_type = w2h_wps_p2p_ie->en_app_ie_type;
    app_ie.ie_len = w2h_wps_p2p_ie->ie_len;
    if (memcpy_s(app_ie.auc_ie, WLAN_WPS_IE_MAX_SIZE, w2h_wps_p2p_ie->puc_data_ie, app_ie.ie_len) != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_set_wps_p2p_ie::memcpy fail!");
        return OAL_FAIL;
    }

    /* 设置WPS/P2P 信息 */
    ret = hmac_config_set_app_ie_to_vap(mac_vap, &app_ie, app_ie.en_app_ie_type);
    if (ret != OAL_SUCC) {
        return ret;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_p2p_ie::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 检测beacon 信息中是否有WPS 信息元素 */
    if (app_ie.en_app_ie_type == OAL_APP_BEACON_IE) {
        if (app_ie.ie_len != 0 && mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS,
            app_ie.auc_ie, (int32_t)(app_ie.ie_len)) != NULL) {
            /* 设置WPS 功能使能 */
            mac_mib_set_WPSActive(mac_vap, OAL_TRUE);
            oam_info_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_p2p_ie::set wps enable.}");
        } else {
            mac_mib_set_WPSActive(mac_vap, OAL_FALSE);
        }
    }

    return ret;
}


uint32_t hmac_config_set_wps_ie(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap = NULL;
    oal_app_ie_stru *pst_wps_ie;
    uint8_t *puc_ie = NULL;
    uint32_t ret;

    pst_wps_ie = (oal_app_ie_stru *)puc_param;

    /* 设置WPS 信息 */
    ret = hmac_config_set_app_ie_to_vap(pst_mac_vap, pst_wps_ie, pst_wps_ie->en_app_ie_type);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_ie::ret=[%d].}", ret);
        return ret;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_ie::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 检测beacon 信息中是否有WPS 信息元素 */
    if ((pst_wps_ie->en_app_ie_type == OAL_APP_BEACON_IE) && (pst_wps_ie->ie_len != 0)) {
        puc_ie = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS,
                                    pst_wps_ie->auc_ie, (int32_t)(pst_wps_ie->ie_len));
        if (puc_ie != NULL) {
            /* 设置WPS 功能使能 */
            mac_mib_set_WPSActive(pst_mac_vap, OAL_TRUE);
            oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_ie::set wps enable.}");
        }
    } else if ((pst_wps_ie->ie_len == 0) && (pst_wps_ie->en_app_ie_type == OAL_APP_BEACON_IE)) {
        mac_mib_set_WPSActive(pst_mac_vap, OAL_FALSE);
        oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_ie::set wps disable.}");
    }

    return ret;
}
#ifdef PLATFORM_DEBUG_ENABLE

uint32_t hmac_config_reg_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_REG_INFO, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_reg_info::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif

uint32_t hmac_config_sdio_flowctrl(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SDIO_FLOWCTRL, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_sdio_flowctrl::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

#ifdef _PRE_WLAN_DELAY_STATISTIC

uint32_t hmac_config_pkt_time_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    /***************************************************************************
    抛事件到DMAC层, 同步DMAC数
    **************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PKT_TIME_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_pkt_time_switch::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif /* _PRE_WLAN_DELAY_STATISTIC */

uint32_t hmac_config_send_bar(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SEND_BAR, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_send_bar::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#ifdef PLATFORM_DEBUG_ENABLE

uint32_t hmac_config_reg_write(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_REG_WRITE, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_reg_write::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif

uint32_t hmac_config_dpd_cfg(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DPD, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_reg_write::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_alg_param(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    /***************************************************************************
        抛事件到ALG层, 同步ALG数据
    ***************************************************************************/
    return hmac_config_alg_send_event(pst_mac_vap, WLAN_CFGID_ALG_PARAM, us_len, puc_param);
}


uint32_t hmac_config_cali_debug(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    /***************************************************************************
    抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_CALI_CFG, len, param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_cali_debug::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)

uint32_t hmac_11v_sta_tx_query(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    /* 仅STA模式下支持触发发送query帧 */
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_11v_sta_tx_query::vap mode:[%d] not support this.}", pst_mac_vap->en_vap_mode);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    if (mac_mib_get_MgmtOptionBSSTransitionActivated(pst_mac_vap) == OAL_FALSE) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_11v_sta_tx_query::en_dot11MgmtOptionBSSTransitionActivated is FALSE.}");
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_11V_TX_QUERY, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_11v_sta_tx_query::hmac_config_send_event failed[%d].}", ret);
    }
    return OAL_SUCC;
}
#endif


uint32_t hmac_config_set_2040_coext_support(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    if ((*puc_param != 0) && (*puc_param != 1)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_2040_coext_support::invalid param[%d].", *puc_param);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 如果是配置VAP, 直接返回 */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_2040_coext_support::this is config vap! can't set.}");
        return OAL_FAIL;
    }

    mac_mib_set_2040BSSCoexistenceManagementSupport(pst_mac_vap, (oal_bool_enum_uint8)(*puc_param));
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                  "{hmac_config_set_2040_coext_support::end func,puc_param=%d.}", *puc_param);
    return OAL_SUCC;
}
#define WLAN_RX_SWITCH_BIT 1
#define WLAN_RX_SWITCH_OFFSET 1

void hmac_config_show_fcs_info(mac_vap_stru *mac_vap)
{
#ifdef CONFIG_ARCH_KIRIN_PCIE
#ifndef _PRE_LINUX_TEST
    dmac_atcmdsrv_atcmd_response_event atcmdsrv_get_rx_pkcg_event;
    hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return;
    }

    if ((oal_get_bits(wlan_chip_get_trx_switch(), WLAN_RX_SWITCH_BIT, WLAN_RX_SWITCH_OFFSET)) == DEVICE_RX) {
        return;
    }

    hal_host_al_rx_fcs_info(hmac_vap);
    hal_host_get_rx_pckt_info(hmac_vap, &atcmdsrv_get_rx_pkcg_event);
    hmac_atcmdsrv_get_rx_pkcg(mac_vap, sizeof(dmac_atcmdsrv_atcmd_response_event),
        (uint8_t *)&atcmdsrv_get_rx_pkcg_event);
#endif
#endif
    return;
}


uint32_t hmac_config_rx_fcs_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_RX_FCS_INFO, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_rx_fcs_info::send event fail[%d].}", ret);
        return ret;
    }
    /* host常收功能维测输出 */
    hmac_config_show_fcs_info(pst_mac_vap);
    return OAL_SUCC;
}


uint32_t hmac_config_set_pm_by_module(mac_vap_stru *pst_mac_vap,
                                      mac_pm_ctrl_type_enum pm_ctrl_type, mac_pm_switch_enum pm_enable)
{
    uint32_t ret;
    mac_cfg_ps_open_stru st_ps_open = { 0 };

    if (pm_enable >= MAC_STA_PM_SWITCH_BUTT ||
        pm_ctrl_type >= MAC_STA_PM_CTRL_TYPE_BUTT ||
        pst_mac_vap == NULL) {
        oam_error_log2(0, OAM_SF_ANY,
                       "hmac_config_set_pm_by_module, PARAM ERROR! pm_ctrl_type=%d, pm_enable=%d ",
                       pm_ctrl_type, pm_enable);
        return OAL_FAIL;
    }

    st_ps_open.uc_pm_enable = pm_enable;
    st_ps_open.uc_pm_ctrl_type = pm_ctrl_type;

    ret = hmac_config_set_sta_pm_on(pst_mac_vap, sizeof(mac_cfg_ps_open_stru), (uint8_t *)&st_ps_open);

    oam_warning_log3(0, OAM_SF_PWR, "hmac_config_set_pm_by_module, pm_module=%d, pm_enable=%d, cfg ret=%d ",
                     pm_ctrl_type, pm_enable, ret);

    return ret;
}


uint32_t hmac_config_alg(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_user_stru *pst_user = NULL;
    int8_t *pac_argv[DMAC_ALG_CONFIG_MAX_ARG + 1] = { 0 };
    mac_ioctl_alg_config_stru *pst_alg_config;
    uint8_t uc_idx;
    uint32_t bw_limit_kbps;
    oal_dlist_head_stru *pst_list_pos = NULL;

    pst_alg_config = (mac_ioctl_alg_config_stru *)puc_param;

    for (uc_idx = sizeof(mac_ioctl_alg_config_stru); uc_idx < us_len; uc_idx++) {
        if (puc_param[uc_idx] == ' ') {
            puc_param[uc_idx] = 0;
        }
    }

    for (uc_idx = 0; uc_idx < pst_alg_config->uc_argc; uc_idx++) {
        pac_argv[uc_idx] = (int8_t *)puc_param +
                           sizeof(mac_ioctl_alg_config_stru) + pst_alg_config->auc_argv_offset[uc_idx];
    }

    /* 如果为用户限速，则需要同步hmac_vap的状态信息 */
    if ((oal_strcmp(pac_argv[0], "sch") == 0)
        && (oal_strcmp(pac_argv[1], "usr_bw") == 0)) {
        pst_user = mac_vap_get_user_by_addr(pst_mac_vap, (uint8_t *)(pac_argv[BYTE_OFFSET_2]));
        if (pst_user == NULL) {
            oam_error_log0(0, OAM_SF_MULTI_TRAFFIC,
                           "{alg_schedule_config_user_bw_limit: mac_vap_find_user_by_macaddr failed}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        bw_limit_kbps = (uint32_t)oal_atoi(pac_argv[BYTE_OFFSET_3]);

        pst_mac_vap->bit_has_user_bw_limit = OAL_FALSE;
        for ((pst_list_pos) = (pst_mac_vap)->st_mac_user_list_head.pst_next,
             (pst_user) = oal_dlist_get_entry((pst_list_pos), mac_user_stru, st_user_dlist);
             (pst_list_pos) != &((pst_mac_vap)->st_mac_user_list_head);
             (pst_list_pos) = (pst_list_pos)->pst_next,
             (pst_user) = oal_dlist_get_entry((pst_list_pos), mac_user_stru, st_user_dlist)) {
            /* 如果限速值不为0,表示该user已被限速，则更新vap的状态 */
            if ((pst_user != NULL) && (bw_limit_kbps != 0)) {
                pst_mac_vap->bit_has_user_bw_limit = OAL_TRUE;
                break;
            }
        }
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_ALG, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_alg::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_40M_intol_sync_event(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_40M_INTOL_UPDATE, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                         "{hmac_40M_intol_sync_event::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_protection_update_from_user(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    mac_dump_protection(pst_mac_vap, puc_param);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PROTECTION_UPDATE_STA_USER, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_protection::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_vap_state_syn(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_VAP_STATE_SYN, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_vap_state_syn::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_set_ipaddr_timeout(void *puc_para)
{
    uint32_t ret;
    hmac_vap_stru *pst_hmac_vap = (hmac_vap_stru *)puc_para;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if ((pst_hmac_vap->uc_ps_mode == MAX_FAST_PS) || (pst_hmac_vap->uc_ps_mode == AUTO_FAST_PS)) {
        wlan_pm_set_timeout((g_wlan_min_fast_ps_idle > 1) ? (g_wlan_min_fast_ps_idle - 1) : g_wlan_min_fast_ps_idle);
    } else {
        wlan_pm_set_timeout(WLAN_SLEEP_DEFAULT_CHECK_CNT);
    }
#endif

    /* 未主动dhcp成功,超时开低功耗 */
    ret = hmac_config_set_pm_by_module(&pst_hmac_vap->st_vap_base_info,
                                       MAC_STA_PM_CTRL_TYPE_HOST, MAC_STA_PM_SWITCH_ON);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                         "{hmac_set_ipaddr_timeout::hmac_config_set_pm_by_module failed[%d].}", ret);
    }

    return OAL_SUCC;
}


uint32_t hmac_config_user_asoc_state_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    uint32_t ret;
    mac_h2d_user_asoc_state_stru st_h2d_user_asoc_state_stru;

    st_h2d_user_asoc_state_stru.us_user_idx = pst_mac_user->us_assoc_id;
    st_h2d_user_asoc_state_stru.en_asoc_state = pst_mac_user->en_user_asoc_state;

    /***************************************************************************
        抛事件到DMAC层, 同步user关联状态到device侧
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_USER_ASOC_STATE_SYN,
                                 sizeof(mac_h2d_user_asoc_state_stru), (uint8_t *)(&st_h2d_user_asoc_state_stru));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_user_asoc_state_syn::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


OAL_STATIC void hmac_config_user_htc_cap_update(mac_vap_stru *p_mac_vap, mac_user_stru *p_mac_user)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    MAC_USER_ARP_PROBE_CLOSE_HTC(p_mac_user) = OAL_FALSE;
    if (IS_LEGACY_STA(p_mac_vap) && MAC_USER_IS_HE_USER(p_mac_user)) {
        if (hmac_wifi6_self_cure_mac_is_htc_blacklist_type(p_mac_user->auc_user_mac_addr)) {
            MAC_USER_ARP_PROBE_CLOSE_HTC(p_mac_user) = OAL_TRUE;
            oam_warning_log3(0, OAM_SF_SCAN, "hmac_config_user_htc_cap_update::\
                mac:%02X:XX:XX:XX:%02X:%02X in htc balcklist, close HTC",
                p_mac_user->auc_user_mac_addr[0],  /* 0为mac地址字节位置 */
                p_mac_user->auc_user_mac_addr[4],  /* 4为mac地址字节位置 */
                p_mac_user->auc_user_mac_addr[5]); /* 5为mac地址字节位置 */
        }
    }
    mac_vap_tx_data_set_user_htc_cap(p_mac_vap, p_mac_user);
#endif
}


uint32_t hmac_config_user_cap_syn(mac_vap_stru *mac_vap, mac_user_stru *mac_user)
{
    uint32_t ret;
    mac_h2d_usr_cap_stru mac_h2d_usr_cap;
    hmac_user_stru *hmac_user = mac_res_get_hmac_user(mac_user->us_assoc_id);
    if (hmac_user == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "hmac_config_user_cap_syn:hmac_user null");
        return OAL_FAIL;
    }
    mac_h2d_usr_cap.us_user_idx = mac_user->us_assoc_id;
    mac_h2d_usr_cap.amsdu_maxsize = hmac_user->amsdu_maxsize;
    memcpy_s((uint8_t *)(&mac_h2d_usr_cap.st_user_cap_info), sizeof(mac_user_cap_info_stru),
             (uint8_t *)(&mac_user->st_cap_info), sizeof(mac_user_cap_info_stru));
#ifdef _PRE_WLAN_FEATURE_HIEX
    if (g_wlan_spec_cfg->feature_hiex_is_open) {
        /* hmac向dmac同步user HIEX能力 */
        memcpy_s((uint8_t *)(&mac_h2d_usr_cap.st_hiex_cap), sizeof(mac_hiex_cap_stru),
                 (uint8_t *)MAC_USER_GET_HIEX_CAP(mac_user), sizeof(mac_hiex_cap_stru));
    }
#endif
    hmac_config_user_htc_cap_update(mac_vap, mac_user);
    mac_vap_csa_support_set(mac_vap, mac_h2d_usr_cap.st_user_cap_info.bit_p2p_support_csa);

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_USER_CAP_SYN,
        sizeof(mac_h2d_usr_cap_stru), (uint8_t *)(&mac_h2d_usr_cap));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_user_cap_syn::hmac_config_sta_vap_info_syn failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_user_rate_info_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    uint32_t ret;
    mac_h2d_usr_rate_info_stru st_mac_h2d_usr_rate_info;

    st_mac_h2d_usr_rate_info.us_user_idx = pst_mac_user->us_assoc_id;
    st_mac_h2d_usr_rate_info.en_protocol_mode = pst_mac_user->en_protocol_mode;

    /* legacy速率集信息，同步到dmac */
    st_mac_h2d_usr_rate_info.uc_avail_rs_nrates = pst_mac_user->st_avail_op_rates.uc_rs_nrates;
    memcpy_s(st_mac_h2d_usr_rate_info.auc_avail_rs_rates, WLAN_MAX_SUPP_RATES,
             pst_mac_user->st_avail_op_rates.auc_rs_rates, WLAN_MAX_SUPP_RATES);

    /* ht速率集信息，同步到dmac */
    mac_user_get_ht_hdl(pst_mac_user, &st_mac_h2d_usr_rate_info.st_ht_hdl);

    /* vht速率集信息，同步到dmac */
    mac_user_get_vht_hdl(pst_mac_user, &st_mac_h2d_usr_rate_info.st_vht_hdl);

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        /* he速率集信息，同步到dmac */
        mac_user_get_he_hdl(pst_mac_user, &(st_mac_h2d_usr_rate_info.st_he_hdl));
    }
#endif

    /***************************************************************************
        抛事件到DMAC层, 同步user关联状态到device侧
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_USER_RATE_SYN,
                                 sizeof(mac_h2d_usr_rate_info_stru), (uint8_t *)(&st_mac_h2d_usr_rate_info));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_user_rate_info_syn::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_user_info_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    uint32_t ret;
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
#ifdef _PRE_WLAN_FEATURE_11AX
    st_mac_h2d_usr_info.bit_in_htc_blacklist = MAC_USER_ARP_PROBE_CLOSE_HTC(pst_mac_user);
#endif

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_USR_INFO_SYN,
                                 sizeof(st_mac_h2d_usr_info), (uint8_t *)(&st_mac_h2d_usr_info));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_user_info_syn::hmac_config_send_event failed[%d],user_id[%d].}",
                         ret, pst_mac_user->us_assoc_id);
    }

    return ret;
}


uint32_t hmac_config_set_probe_resp_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_probe_resp_mode_enum *pen_probe_resp_status = NULL;

    if (!IS_P2P_DEV(pst_mac_vap)) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_probe_resp_mode:: [vap_mode:%d] \
            [p2p_mode:%d] is not P2P_dev. return INVALID_CONFIG.}", pst_mac_vap->en_vap_mode,pst_mac_vap->en_p2p_mode);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pen_probe_resp_status = (mac_probe_resp_mode_enum *)puc_param;

    /* probe_resp_flag未开启，不允许改动probe_resp_mode的默认值 */
    if (pst_mac_vap->st_probe_resp_ctrl.en_probe_resp_enable == OAL_FALSE) {
        oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                      "{hmac_config_set_probe_resp_mode::Host set probe_resp_status[%d],buf Drv_enable == FALSE.}",
                      *pen_probe_resp_status);
        return OAL_SUCC;
    }

    if (*pen_probe_resp_status >= MAC_PROBE_RESP_MODE_BUTT) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_TX,
                       "{hmac_config_set_probe_resp_mode::config probe_resp_status error %d.}", *pen_probe_resp_status);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_probe_resp_mode::Host set probe_resp_status from [%d] to [%d].}",
                     pst_mac_vap->st_probe_resp_ctrl.en_probe_resp_status, *pen_probe_resp_status);
    pst_mac_vap->st_probe_resp_ctrl.en_probe_resp_status = *pen_probe_resp_status;

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PROBE_PESP_MODE,
                                 sizeof(mac_probe_resp_mode_enum), puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_probe_resp_mode::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_sta_vap_info_syn(mac_vap_stru *pst_mac_vap)
{
    uint32_t ret;
    mac_h2d_vap_info_stru st_mac_h2d_vap_info;

    st_mac_h2d_vap_info.us_sta_aid = pst_mac_vap->us_sta_aid;
    st_mac_h2d_vap_info.uc_uapsd_cap = pst_mac_vap->uc_uapsd_cap;
    st_mac_h2d_vap_info.bit_ap_chip_oui = pst_mac_vap->bit_ap_chip_oui;
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    st_mac_h2d_vap_info.en_txop_ps = mac_vap_get_txopps(pst_mac_vap);
#endif /* #ifdef _PRE_WLAN_FEATURE_TXOPPS */

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_STA_VAP_INFO_SYN,
                                 sizeof(mac_h2d_vap_info_stru), (uint8_t *)(&st_mac_h2d_vap_info));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_sta_vap_info_syn::hmac_config_sta_vap_info_syn failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_init_user_security_port(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    uint32_t ret;
    mac_cfg80211_init_port_stru st_init_port;

    /* 初始化认证端口信息 */
    mac_vap_init_user_security_port(pst_mac_vap, pst_mac_user);

    memcpy_s(st_init_port.auc_mac_addr, OAL_MAC_ADDR_LEN, pst_mac_user->auc_user_mac_addr, OAL_MAC_ADDR_LEN);
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_INIT_SECURTIY_PORT,
                                 sizeof(st_init_port), (uint8_t *)&st_init_port);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_WPA,
                       "{hmac_config_user_security_port::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_user_set_asoc_state(mac_vap_stru *pst_mac_vap,
                                  mac_user_stru *pst_mac_user,
                                  mac_user_asoc_state_enum_uint8 en_value)
{
    uint32_t ret;

    mac_user_set_asoc_state(pst_mac_user, en_value);

    /* dmac offload架构下，同步user关联状态信息到dmac */
    ret = hmac_config_user_asoc_state_syn(pst_mac_vap, pst_mac_user);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_user_set_asoc_state::hmac_config_user_asoc_state_syn failed[%d].}", ret);
    }

    return ret;
}

uint32_t hmac_config_ch_status_sync(mac_device_stru *pst_mac_dev)
{
    uint32_t ret;
    mac_ap_ch_info_stru ast_ap_channel_list[MAC_MAX_SUPP_CHANNEL] = { { 0 } };
    mac_vap_stru *pst_mac_vap = NULL;

    if (pst_mac_dev == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_ch_status_sync::pst_mac_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = mac_res_get_mac_vap(pst_mac_dev->auc_vap_id[0]);
    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_ch_status_sync::pst_mac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memcpy_s((uint8_t *)ast_ap_channel_list, sizeof(ast_ap_channel_list),
             (uint8_t *)(pst_mac_dev->st_ap_channel_list), sizeof(pst_mac_dev->st_ap_channel_list));

    /***************************************************************************
        抛事件到DMAC层
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SYNC_CH_STATUS,
                                 sizeof(ast_ap_channel_list), (uint8_t *)ast_ap_channel_list);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_ch_status_sync::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_send_2040_coext(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_set_2040_coexist_stru *pst_2040_coexist = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;
    mac_tx_ctl_stru *pst_tx_ctl = NULL;
    uint32_t ret;
    uint16_t us_frame_len;

    /* 只有STA需要处理 */
    if (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_2040,
                         "hmac_config_send_2040_coext::en_vap_mode is[%d] not STAUT,return", mac_vap->en_vap_mode);
        return OAL_SUCC;
    }

    pst_2040_coexist = (mac_cfg_set_2040_coexist_stru *)puc_param;
    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_2040, "hmac_config_send_2040_coext::coinfo=%d chan=%d",
                     pst_2040_coexist->coext_info, pst_2040_coexist->channel_report);

    /* 申请管理帧内存 */
    pst_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_2040, "{hmac_config_send_2040_coext::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_netbuf_prev(pst_netbuf) = NULL;
    oal_netbuf_next(pst_netbuf) = NULL;

    /* 封装20/40 共存管理帧 */
    us_frame_len = mac_encap_2040_coext_mgmt(mac_vap, pst_netbuf, (uint8_t)pst_2040_coexist->coext_info,
                                             pst_2040_coexist->channel_report);
    oal_netbuf_put(pst_netbuf, us_frame_len);

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    memset_s(pst_tx_ctl, oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_frame_len;
    ret = mac_vap_set_cb_tx_user_idx(mac_vap, pst_tx_ctl, mac_vap->auc_bssid);
    if (ret != OAL_SUCC) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_2040,
            "(hmac_config_send_2040_coext::fail to find user by xx:xx:xx:0x:0x:0x.}",
            mac_vap->auc_bssid[MAC_ADDR_3], mac_vap->auc_bssid[MAC_ADDR_4], mac_vap->auc_bssid[MAC_ADDR_5]);
    }
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl) = WLAN_WME_AC_MGMT;

    /* 抛事件到DMAC发送管理帧 */
    ret = hmac_tx_mgmt_send_event(mac_vap, pst_netbuf, us_frame_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);

        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_2040,
                         "{hmac_config_send_2040_coext::hmac_tx_mgmt_send_event failed.}", ret);
    }

    return OAL_SUCC;
}

/* FTM认证，由于需要访问扫描结果列表，因此该用例在host侧实现 */
#ifdef _PRE_WLAN_FEATURE_FTM
typedef enum {
    ANQP_INFO_ID_RESERVED        = 255,
    ANQP_INFO_ID_QUERY_LIST      = 256,
    ANQP_INFO_ID_AP_GEO_LOC      = 265,
    ANQP_INFO_ID_AP_CIVIC_LOC    = 266,
    ANQP_INFO_ID_BUTT,
} anqp_info_id_enum;

typedef struct {
    void *pst_hmac_vap;
} ftm_timer_arg_stru;

typedef struct {
    uint8_t  uc_dialog_token;
    uint16_t status_code;
    uint8_t  fragment_id;
    uint16_t comeback_delay;
    uint8_t  adv_protocol_ele;
    uint16_t query_resp_len;
} gas_init_resp_stru;


OAL_STATIC uint16_t hmac_encap_query_list(uint8_t *puc_buffer, mac_send_gas_init_req_stru *pst_gas_req)
{
    uint16_t idx = 0;
    uint16_t *ptemp = NULL;

    /* AP支持AP_GEO_LOC， EC_IE_bit15 */
    if (pst_gas_req->en_geo_enable) {
        ptemp = (uint16_t *)&puc_buffer[idx];    /* AP Geospatial Location */
       *ptemp = ANQP_INFO_ID_AP_GEO_LOC;
        idx += BYTE_OFFSET_2;
    }

    /* AP支持AP_CIVIC_LOC， EC_IE_BIT14 */
    if (pst_gas_req->en_civic_enable) {
        ptemp = (uint16_t *)&puc_buffer[idx];    /* AP Civic Location */
       *ptemp = ANQP_INFO_ID_AP_CIVIC_LOC;
        idx += BYTE_OFFSET_2;
    }

    return idx;
}


OAL_STATIC uint16_t hmac_encap_anqp_query(uint8_t *puc_buffer, mac_send_gas_init_req_stru *pst_gas_req)
{
    uint16_t  idx   = 0;
    uint16_t *ptemp = NULL;

    /* Query Request - ANQP elements */
    /* **************************** */
    /* |Info ID|Length|Informatino| */
    /* ---------------------------- */
    /* |2      |2     |variable   | */
    /* **************************** */
    ptemp = (uint16_t *)&puc_buffer[idx];    /* Info ID - Query List, Table 9-271 */
    *ptemp = ANQP_INFO_ID_QUERY_LIST;
    idx += BYTE_OFFSET_2;

    ptemp = (uint16_t *)&puc_buffer[idx];    /* Length, 先跳过 */
    idx += BYTE_OFFSET_2;

    *ptemp = hmac_encap_query_list(&puc_buffer[idx], pst_gas_req);
    return 4 + *ptemp; /* Info ID 和 Length 的长度 4 */
}


OAL_STATIC uint16_t hmac_encap_gas_init_req(mac_vap_stru *pst_mac_vap,
                                            oal_netbuf_stru *pst_buffer,
                                            mac_send_gas_init_req_stru *pst_gas_req)
{
    uint8_t      *puc_mac_header   = oal_netbuf_header(pst_buffer);
    uint8_t      *puc_payload_addr = mac_netbuf_get_payload(pst_buffer);
    uint16_t     *ptemp;
    uint16_t      us_index         = 0;
    hmac_vap_stru  *pst_hmac_vap     = NULL;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_encap_gas_init_req::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_hdr_set_frame_control(puc_mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);
    mac_hdr_set_fragment_number(puc_mac_header, 0);
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR1_OFFSET, pst_gas_req->auc_bssid);
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(pst_mac_vap));
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR3_OFFSET, pst_gas_req->auc_bssid);

    /*************************************************************************************/
    /*         GAS Initial Request  frame - Frame Body                                   */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Public Action |Dialog Token  | Advertisement Protocol element |        */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1             |1             |Variable                        |        */
    /* --------------------------------------------------------------------------------- */
    /* |Query Request length|Query Request|Multi-band (optional)                         */
    /* --------------------------------------------------------------------------------- */
    /* |2                   |variable     |                                              */
    /* --------------------------------------------------------------------------------- */
    /*                                                                                   */
    /*************************************************************************************/
    puc_payload_addr[us_index++] = MAC_ACTION_CATEGORY_PUBLIC;      /* Category */
    puc_payload_addr[us_index++] = MAC_PUB_GAS_INIT_REQ;            /* Public Action */
    puc_payload_addr[us_index++] = pst_hmac_vap->st_gas_mgmt.uc_gas_dialog_token;   /* Dialog Token */

    /*****************************************************************************************************/
    /*                   Advertisement Protocol element                                */
    /* ------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |Advertisement Protocol Tuple #1| ... |Advertisement Protocol Tuple #n(optional) | */
    /* ------------------------------------------------------------------------------------------------- */
    /* |1          |1      | variable                      |     |variable                              | */
    /* ------------------------------------------------------------------------------------------------- */
    /*                                                                                                   */
    /*****************************************************************************************************/
    puc_payload_addr[us_index++] = MAC_EID_ADVERTISEMENT_PROTOCOL;    /* Element ID */
    puc_payload_addr[us_index++] = 2;                                 /* Length ：2 */
    puc_payload_addr[us_index++] = 0;                                 /* Query Response Info */
    puc_payload_addr[us_index++] = 0;                                 /* Advertisement Protocol ID, 0: ANQP */

    /* Query Request length */
    ptemp = (uint16_t *)(&puc_payload_addr[us_index]);
    us_index += BYTE_OFFSET_2;

   *ptemp = hmac_encap_anqp_query(&puc_payload_addr[us_index], pst_gas_req);
    return (uint16_t)(us_index + MAC_80211_FRAME_LEN + *ptemp);
}


OAL_STATIC mac_bss_dscr_stru *check_interworking_support(mac_vap_stru *pst_mac_vap,
    mac_send_gas_init_req_stru *pst_gas_req)
{
    hmac_bss_mgmt_stru      *pst_bss_mgmt;
    oal_dlist_head_stru     *pst_entry;
    hmac_scanned_bss_info   *pst_scanned_bss;
    mac_bss_dscr_stru       *pst_bss_dscr = NULL;
    mac_bss_dscr_stru       *pst_sel_dscr = NULL;

    /* 获取hmac device */
    hmac_device_stru *pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_roam_scan_complete::device null!}");
        return NULL;
    }

    pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    /* 获取锁 */
    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    /* 遍历扫描到的bss信息，查找可以漫游的bss */
    oal_dlist_search_for_each(pst_entry, &(pst_bss_mgmt->st_bss_list_head)) {
        pst_scanned_bss = oal_dlist_get_entry(pst_entry, hmac_scanned_bss_info, st_dlist_head);
        pst_bss_dscr    = &(pst_scanned_bss->st_bss_dscr_info);

        if (!oal_compare_mac_addr(pst_gas_req->auc_bssid, pst_bss_dscr->auc_bssid)) {
            pst_sel_dscr = pst_bss_dscr;
            oam_warning_log0(0, OAM_SF_CFG, "{FTM Certificate:: Find AP device}");
        }

        pst_bss_dscr = NULL;
    }

    /* 解除锁 */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));

    return pst_sel_dscr;
}


uint32_t hmac_ftm_send_gas_init_req(mac_vap_stru *pst_mac_vap, mac_send_gas_init_req_stru *pst_gas_req)
{
    uint32_t                ret;
    oal_netbuf_stru        *gas_init_req_frame;
    mac_tx_ctl_stru        *tx_ctl;
    uint32_t                len;
    uint8_t                *ec_ie    = NULL;
    mac_bss_dscr_stru      *sel_dscr = NULL;
    hmac_join_req_stru      join_req;
    hmac_vap_stru          *hmac_vap;
    hmac_join_rsp_stru      join_rsp;
    uint8_t                *frame_body;

    sel_dscr = check_interworking_support(pst_mac_vap, pst_gas_req);
    if (sel_dscr == NULL) {
        return OAL_SUCC;
    }

    /* 寻找EC_IE信元 */
    frame_body = (uint8_t *)sel_dscr->auc_mgmt_frame_body;
    ec_ie = mac_find_ie(MAC_EID_EXT_CAPS, frame_body, sel_dscr->mgmt_len - MAC_80211_QOS_HTC_4ADDR_FRAME_LEN);
    if (ec_ie == NULL || ec_ie[1] < BYTE_OFFSET_3) {
        oam_warning_log0(0, OAM_SF_CFG, "{FTM Certificate:: Can't find EC_IE}");
        return OAL_FAIL;
    }

    /* 获取相关能力，BIT14， BIT15，BIT31， 先跳过ID与len字段 */
    pst_gas_req->en_interworking_enable = (ec_ie[BYTE_OFFSET_5] & 0x80) >> NUM_7_BITS;
    pst_gas_req->en_civic_enable = (ec_ie[BYTE_OFFSET_3] & 0x40) >> NUM_6_BITS;
    pst_gas_req->en_geo_enable = (ec_ie[BYTE_OFFSET_3] & 0x80) >> NUM_7_BITS;

    oam_warning_log3(0, OAM_SF_CFG, "{FTM Certificate:: interworking = %d, civic_enable = %d, geo_enable = %d}",
        pst_gas_req->en_interworking_enable, pst_gas_req->en_civic_enable, pst_gas_req->en_geo_enable);

    if (!pst_gas_req->en_interworking_enable) {
        oam_warning_log0(0, OAM_SF_CFG, "{FTM Certificate:: AP does not support interworking!}");
        return OAL_SUCC;
    }

    /* 配置join参数 */
    hmac_prepare_join_req(&join_req, sel_dscr);

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_sta_update_join_req_params(hmac_vap, &join_req);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_ftm_send_gas_init_req::hmac_sta_update_join_req_params fail[%d].}", ret);
        return ret;
    }

    join_rsp.en_result_code = HMAC_MGMT_SUCCESS;

    /* 切换STA状态到JOIN_COMP */
    hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_JOIN_COMP);

    /* 组装gas init req帧 */
    gas_init_req_frame = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (gas_init_req_frame == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_mem_netbuf_trace(gas_init_req_frame, OAL_TRUE);

    memset_s(oal_netbuf_cb(gas_init_req_frame), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    /* 将mac header清零 */
    memset_s((uint8_t *)oal_netbuf_header(gas_init_req_frame), MAC_80211_FRAME_LEN, 0, MAC_80211_FRAME_LEN);

    len = hmac_encap_gas_init_req(pst_mac_vap, gas_init_req_frame, pst_gas_req);

    oal_netbuf_put(gas_init_req_frame, len);

    /* 为填写发送描述符准备参数 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(gas_init_req_frame);
    MAC_GET_CB_MPDU_LEN(tx_ctl) = (uint16_t)len;

    /* 抛事件让dmac将该帧发送 */
    ret = hmac_tx_mgmt_send_event(pst_mac_vap, gas_init_req_frame, (uint16_t)len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(gas_init_req_frame);
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_ftm_send_gas_init_req::hmac_tx_mgmt_send_event failed[%d].}", ret);
        return ret;
    }

    /* 设置gas request frame sent标志位 */
    hmac_vap->st_gas_mgmt.uc_is_gas_request_sent = OAL_TRUE;

    return OAL_SUCC;
}


OAL_STATIC uint16_t hmac_encap_gas_comeback_req(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buffer)
{
    uint8_t      *p_mac_header   = oal_netbuf_header(buffer);
    uint8_t      *p_payload_addr = mac_netbuf_get_payload(buffer);
    uint16_t      idx         = 0;
    mac_vap_stru   *pst_mac_vap      = &hmac_vap->st_vap_base_info;

    mac_hdr_set_frame_control(p_mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    mac_hdr_set_fragment_number(p_mac_header, 0);

    oal_set_mac_addr(p_mac_header + WLAN_HDR_ADDR1_OFFSET, pst_mac_vap->auc_bssid);

    oal_set_mac_addr(p_mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(pst_mac_vap));

    oal_set_mac_addr(p_mac_header + WLAN_HDR_ADDR3_OFFSET, pst_mac_vap->auc_bssid);

    /*************************************************************************************/
    /*        GAS Comeback Request  frame - Frame Body                                   */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Public Action |Dialog Token  | Multi-band (optional)  |        */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1             |1             |Variable                        |        */
    /* --------------------------------------------------------------------------------- */
    /*                                                                                   */
    /*************************************************************************************/
    p_payload_addr[idx++] = MAC_ACTION_CATEGORY_PUBLIC;                     /* Category */
    p_payload_addr[idx++] = MAC_PUB_GAS_COMBAK_REQ;                         /* Public Action */
    p_payload_addr[idx++] = hmac_vap->st_gas_mgmt.uc_gas_response_dialog_token; /* 要填入AP gas resp帧中的Dialog Token */

    oam_warning_log1(0, OAM_SF_FTM,
        "{hmac_encap_gas_comeback_req::dialog token = %d", hmac_vap->st_gas_mgmt.uc_gas_response_dialog_token);

    return (uint16_t)(idx + MAC_80211_FRAME_LEN);
}


OAL_STATIC uint32_t hmac_ftm_send_gas_comeback_req(hmac_vap_stru *pst_hmac_vap)
{
    oal_netbuf_stru                    *pst_netbuf;
    mac_tx_ctl_stru                    *pst_tx_ctl;
    uint32_t                           ret;
    uint32_t                           len;
    mac_vap_stru                       *pst_mac_vap      = &pst_hmac_vap->st_vap_base_info;
    int32_t                            l_ret;

    /* 组装gas comeback req帧 */
    pst_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_ftm_send_gas_comeback_req::pst_gas_comeback_req_frame is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_mem_netbuf_trace(pst_netbuf, OAL_TRUE);

    l_ret = memset_s(oal_netbuf_cb(pst_netbuf), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    /* 将mac header清零 */
    l_ret += memset_s((uint8_t *)oal_netbuf_header(pst_netbuf), MAC_80211_FRAME_LEN, 0, MAC_80211_FRAME_LEN);
    if (l_ret != EOK) {
        oam_warning_log1(0, OAM_SF_FTM, "hmac_ftm_send_gas_comeback_req::memcpy fail![%d]", l_ret);
    }

    len = hmac_encap_gas_comeback_req(pst_hmac_vap, pst_netbuf);

    oal_netbuf_put(pst_netbuf, len);

    /* 为填写发送描述符准备参数 */
    pst_tx_ctl                   = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl)      = (uint16_t)len;

    /* 抛事件让dmac将该帧发送 */
    ret = hmac_tx_mgmt_send_event(pst_mac_vap, pst_netbuf, (uint16_t)len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_ftm_send_gas_comeback_req::hmac_tx_mgmt_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t  hmac_ftm_gas_comeback_timeout(void *p_arg)
{
    hmac_vap_stru *pst_hmac_vap;
    uint32_t ret;

    pst_hmac_vap = (hmac_vap_stru *)p_arg;
    if (pst_hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_ftm_send_gas_comeback_req(pst_hmac_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_FTM, "hmac_ftm_gas_comeback_timeout::hmac_ftm_send_gas_comeback_req fail![%d]", ret);
    }
    return OAL_SUCC;
}


OAL_STATIC void  hmac_ftm_start_gas_comeback_timer(hmac_vap_stru *pst_hmac_vap, gas_init_resp_stru *pst_gas_init_resp)
{
    frw_timer_create_timer_m(&(pst_hmac_vap->st_ftm_timer),
        hmac_ftm_gas_comeback_timeout, (uint32_t)pst_gas_init_resp->comeback_delay, (void *)pst_hmac_vap,
        OAL_FALSE, OAM_MODULE_ID_HMAC, pst_hmac_vap->st_vap_base_info.core_id);
}


uint32_t  hmac_ftm_rx_gas_initial_response_frame(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    mac_rx_ctl_stru *pst_rx_info;
    uint8_t *puc_data;
    gas_init_resp_stru st_gas_resp = { 0, 0, 0, 0, 0, 0 };
    uint16_t idx = 0;

    if (oal_unlikely((pst_hmac_vap == NULL) || (pst_netbuf == NULL))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (!pst_hmac_vap->st_gas_mgmt.uc_is_gas_request_sent) {
        oam_warning_log0(0, OAM_SF_RX, "{hmac_ftm_rx_gas_initial_response_frame::it isn't for hmac, forward to wpa.}");
        return OAL_FAIL;
    }
    pst_rx_info = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    puc_data = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_info) + pst_rx_info->uc_mac_header_len;

    idx += BYTE_OFFSET_2; // 跳过category、public action字段
    st_gas_resp.uc_dialog_token = *(uint8_t *)(&puc_data[idx]);
    if (st_gas_resp.uc_dialog_token != pst_hmac_vap->st_gas_mgmt.uc_gas_dialog_token) {
        oam_warning_log2(0, OAM_SF_RX, "{hmac_ftm_rx_gas_initial_response_frame::gas response frame's \
            dialog token [%d] != hmac gas dialog token [%d].}",
            st_gas_resp.uc_dialog_token, pst_hmac_vap->st_gas_mgmt.uc_gas_dialog_token);
        return OAL_FAIL;
    }

    idx += BYTE_OFFSET_1;

    st_gas_resp.status_code = *(uint16_t *)(&puc_data[idx]);
    idx += BYTE_OFFSET_2;

    st_gas_resp.comeback_delay = *(uint16_t *)(&puc_data[idx]);

    oam_warning_log4(0, OAM_SF_FTM,
        "{hmac_ftm_rx_gas_initial_response_frame::dialog token=%d, status code=%d, fragment_id=%d, comebakc_delay=%d",
        st_gas_resp.uc_dialog_token, st_gas_resp.status_code, st_gas_resp.fragment_id, st_gas_resp.comeback_delay);

    pst_hmac_vap->st_gas_mgmt.uc_gas_response_dialog_token = st_gas_resp.uc_dialog_token;

    // 启动comeback定时器
    if (st_gas_resp.comeback_delay > 0) {
        hmac_ftm_start_gas_comeback_timer(pst_hmac_vap, &st_gas_resp);
    }

    pst_hmac_vap->st_gas_mgmt.uc_gas_dialog_token++;
    pst_hmac_vap->st_gas_mgmt.uc_is_gas_request_sent = OAL_FALSE;

    return OAL_SUCC;
}


void hmac_config_ftm_dbg_set_bw_and_preamble(mac_ftm_debug_switch_stru* ftm_debug)
{
    switch (ftm_debug->st_send_iftmr_bit1.format_bw) {
        case FTM_FORMAT_BANDWIDTH_HTMIXED_20:
            ftm_debug->st_send_iftmr_bit1.bw = WIFI_RTT_BW_20;
            ftm_debug->st_send_iftmr_bit1.preamble = WIFI_RTT_PREAMBLE_HT;
            break;
        case FTM_FORMAT_BANDWIDTH_VHT20:
            ftm_debug->st_send_iftmr_bit1.bw = WIFI_RTT_BW_20;
            ftm_debug->st_send_iftmr_bit1.preamble = WIFI_RTT_PREAMBLE_VHT;
            break;
        case FTM_FORMAT_BANDWIDTH_HTMIXED_40:
            ftm_debug->st_send_iftmr_bit1.bw = WIFI_RTT_BW_40;
            ftm_debug->st_send_iftmr_bit1.preamble = WIFI_RTT_PREAMBLE_HT;
            break;
        case FTM_FORMAT_BANDWIDTH_VHT_40:
            ftm_debug->st_send_iftmr_bit1.bw = WIFI_RTT_BW_40;
            ftm_debug->st_send_iftmr_bit1.preamble = WIFI_RTT_PREAMBLE_VHT;
            break;
        case FTM_FORMAT_BANDWIDTH_VHT_80:
            ftm_debug->st_send_iftmr_bit1.bw = WIFI_RTT_BW_80;
            ftm_debug->st_send_iftmr_bit1.preamble = WIFI_RTT_PREAMBLE_VHT;
            break;
        default:
            oam_warning_log1(0, OAM_SF_FTM, "hmac_config_ftm_dbg_set_bw_and_preamble::invalid format_bw:%d",
                ftm_debug->st_send_iftmr_bit1.format_bw);
            break;
    }
}


uint32_t hmac_ftm_check_dbg_params(mac_send_iftmr_stru* send_iftmr)
{
    if (mac_addr_is_zero(send_iftmr->mac_addr)) {
        oam_warning_log0(0, OAM_SF_FTM, "{hmac_ftm_check_dbg_params::bssid should not be all zero!}");
        return OAL_FAIL;
    }
    if (mac_is_channel_num_valid(send_iftmr->st_channel.en_band, send_iftmr->channel_num,
        send_iftmr->st_channel.ext6g_band) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_FTM, "hmac_ftm_check_dbg_params::channel num[%u] invalid!", send_iftmr->channel_num);
        return OAL_FAIL;
    }
    if (send_iftmr->burst_num > MAC_FTM_MAX_NUM_OF_BURSTS_EXPONENT) {
        oam_warning_log1(0, OAM_SF_FTM, "hmac_ftm_check_dbg_params::burst num[%u] over 16", send_iftmr->burst_num);
        return OAL_FAIL;
    }
    if (send_iftmr->ftms_per_burst > MAC_FTM_MAX_FTMS_PER_BURST) {
        oam_warning_log1(0, OAM_SF_FTM, "{hmac_ftm_check_dbg_params::ftms_per_burst[%u] over 10!}",
            send_iftmr->ftms_per_burst);
        return OAL_FAIL;
    }
    if ((send_iftmr->lci_request != 0 && send_iftmr->lci_request != 1) ||
        (send_iftmr->lcr_request != 0 && send_iftmr->lcr_request != 1)) {
        oam_warning_log2(0, OAM_SF_FTM, "hmac_ftm_check_dbg_params::lci_request[%u] lcr_request[%u] not valid,\
            should be 1 or 0", send_iftmr->lci_request, send_iftmr->lcr_request);
        return OAL_FAIL;
    }
    if ((send_iftmr->format_bw < FTM_FORMAT_BANDWIDTH_HTMIXED_20) ||
        (send_iftmr->format_bw > FTM_FORMAT_BANDWIDTH_VHT_80)) {
        oam_warning_log1(0, OAM_SF_FTM, "hmac_ftm_check_dbg_params::format_bw[%u] valid!", send_iftmr->format_bw);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


uint32_t hmac_config_ftm_dbg_1103(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t                      ret;
    mac_ftm_debug_switch_stru      *pst_ftm_debug;
    mac_bss_dscr_stru              *pst_bss_dscr = NULL;
    pst_ftm_debug = (mac_ftm_debug_switch_stru *)puc_param;

    /* 发送iftmr命令 */
    if (pst_ftm_debug->cmd_bit_map & BIT1) {
        pst_bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(pst_mac_vap,
            pst_ftm_debug->st_send_iftmr_bit1.mac_addr);
        if (pst_bss_dscr != NULL) {
            if (pst_ftm_debug->st_send_iftmr_bit1.channel_num != pst_bss_dscr->st_channel.uc_chan_number) {
                oam_warning_log4(0, OAM_SF_FTM, "{hmac_config_ftm_dbg::send iftmr:AP [**:%x:%x] tx channel %d, "
                                 "set to ap's operation channel %d.}",
                                 pst_ftm_debug->st_send_iftmr_bit1.mac_addr[MAC_ADDR_4],
                                 pst_ftm_debug->st_send_iftmr_bit1.mac_addr[MAC_ADDR_5],
                                 pst_ftm_debug->st_send_iftmr_bit1.channel_num,
                                 pst_bss_dscr->st_channel.uc_chan_number);
                pst_ftm_debug->st_send_iftmr_bit1.channel_num = pst_bss_dscr->st_channel.uc_chan_number;
            }
            if (memcpy_s(&pst_ftm_debug->st_send_iftmr_bit1.st_channel, sizeof(mac_channel_stru),
                &pst_bss_dscr->st_channel, sizeof(mac_channel_stru)) != EOK) {
                oam_error_log0(0, OAM_SF_FTM, "hmac_config_ftm_dbg::st_channel memcpy fail!");
            }
            if (hmac_ftm_check_dbg_params(&pst_ftm_debug->st_send_iftmr_bit1) != OAL_SUCC) {
                return OAL_FAIL;
            }
            hmac_config_ftm_dbg_set_bw_and_preamble(pst_ftm_debug);
        }
    }

    /* gas init req,需要访问扫描列表，因此在host侧实现，消息不用抛到dmac */
    if (pst_ftm_debug->cmd_bit_map & BIT16) {
        return hmac_ftm_send_gas_init_req(pst_mac_vap, &pst_ftm_debug->st_gas_init_req_bit16);
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_FTM_DBG, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_ftm_dbg::hmac_config_send_event fail[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}


wlan_channel_bandwidth_enum_uint8 hmac_ftm_get_channel_bandwidth(int32_t center_freq0, uint8_t channel_num,
    uint32_t channel_width)
{
    int32_t channel_center_freq0;
    wlan_channel_bandwidth_enum_uint8 channel_bandwidth;

    channel_center_freq0 = oal_ieee80211_frequency_to_channel(center_freq0);

    if (channel_width == MAC_FTM_WIFI_CHAN_WIDTH_80) {
        switch (channel_center_freq0 - channel_num) {
            case CHAN_OFFSET_PLUS_6: /* 中心频率相对于主20偏6个信道 */
                channel_bandwidth = WLAN_BAND_WIDTH_80PLUSPLUS;
                break;
            case CHAN_OFFSET_MINUS_2: /* 中心频率相对于主20偏-2个信道 */
                channel_bandwidth = WLAN_BAND_WIDTH_80PLUSMINUS;
                break;
            case CHAN_OFFSET_PLUS_2: /* 中心频率相对于主20偏2个信道 */
                channel_bandwidth = WLAN_BAND_WIDTH_80MINUSPLUS;
                break;
            case CHAN_OFFSET_MINUS_6: /* 中心频率相对于主20偏-6个信道 */
                channel_bandwidth = WLAN_BAND_WIDTH_80MINUSMINUS;
                break;
            default:
                channel_bandwidth = WLAN_BAND_WIDTH_20M;
                break;
        }
    } else if (channel_width == MAC_FTM_WIFI_CHAN_WIDTH_40) {
        switch (channel_center_freq0 - channel_num) {
            case CHAN_OFFSET_MINUS_2:
                channel_bandwidth = WLAN_BAND_WIDTH_40MINUS;
                break;
            case CHAN_OFFSET_PLUS_2:
                channel_bandwidth = WLAN_BAND_WIDTH_40PLUS;
                break;
            default:
                channel_bandwidth = WLAN_BAND_WIDTH_20M;
                break;
        }
    } else {
        channel_bandwidth = WLAN_BAND_WIDTH_20M;
    }

    return channel_bandwidth;
}


uint32_t hmac_config_wifi_rtt_config_1103(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t result;
    mac_ftm_wifi_rtt_config *wifi_rtt_config = NULL;
    mac_bss_dscr_stru *mac_bss_dscr = NULL;
    mac_send_iftmr_stru mac_send_iftmr;

    wifi_rtt_config = (mac_ftm_wifi_rtt_config *)param;
    memset_s(&mac_send_iftmr, sizeof(mac_send_iftmr_stru), 0, sizeof(mac_send_iftmr_stru));
    oal_set_mac_addr(mac_send_iftmr.mac_addr, wifi_rtt_config->addr);
    mac_send_iftmr.is_asap_on = OAL_TRUE;
    mac_send_iftmr.lci_request = wifi_rtt_config->lci_request;
    mac_send_iftmr.lcr_request = wifi_rtt_config->lcr_request;
    mac_send_iftmr.burst_num = wifi_rtt_config->num_burst;
    mac_send_iftmr.ftms_per_burst = wifi_rtt_config->num_frames_per_burst;
    mac_send_iftmr.burst_duration = wifi_rtt_config->burst_duration;
    mac_send_iftmr.burst_period = wifi_rtt_config->burst_period;
    mac_send_iftmr.channel_num = oal_ieee80211_frequency_to_channel(wifi_rtt_config->channel.center_freq);
    mac_send_iftmr.preamble = wifi_rtt_config->preamble;
    mac_send_iftmr.bw = wifi_rtt_config->bw;

    /* 发送iftmr命令 */
    mac_bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(mac_vap, wifi_rtt_config->addr);
    if (mac_bss_dscr != NULL) {
        mac_send_iftmr.channel_num = oal_ieee80211_frequency_to_channel(wifi_rtt_config->channel.center_freq);
        if (mac_send_iftmr.channel_num != mac_bss_dscr->st_channel.uc_chan_number) {
            oam_warning_log4(0, OAM_SF_FTM, "{hmac_config_wifi_rtt_config::send iftmr:AP [**:%x:%x] tx channel %d, \
                set to ap's operation channel %d}",wifi_rtt_config->addr[MAC_ADDR_4], wifi_rtt_config->addr[MAC_ADDR_5],
                mac_send_iftmr.channel_num, mac_bss_dscr->st_channel.uc_chan_number);
            mac_send_iftmr.channel_num = mac_bss_dscr->st_channel.uc_chan_number;
        }
        mac_send_iftmr.st_channel.uc_chan_number = mac_send_iftmr.channel_num;
        mac_send_iftmr.st_channel.en_band = mac_get_band_by_channel_num(mac_send_iftmr.channel_num);
        mac_send_iftmr.st_channel.en_bandwidth = hmac_ftm_get_channel_bandwidth(wifi_rtt_config->channel.center_freq0,
            mac_send_iftmr.channel_num, wifi_rtt_config->channel.width);
        mac_get_channel_idx_from_num(mac_send_iftmr.st_channel.en_band, mac_send_iftmr.st_channel.uc_chan_number,
            mac_send_iftmr.st_channel.ext6g_band, &(mac_send_iftmr.st_channel.uc_chan_idx));
    }

    /* 抛事件到DMAC层, 同步DMAC数据 */
    result = hmac_config_send_event(mac_vap, WLAN_CFGID_RTT_CONFIG, sizeof(mac_send_iftmr_stru),
        (uint8_t *)&mac_send_iftmr);
    if (oal_unlikely(result != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_wifi_rtt_config::hmac_config_send_event fail[%d]}", result);
        return result;
    }

    return OAL_SUCC;
}


uint32_t hmac_config_rtt_result_report(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    frw_event_mem_stru *event_mem;
    frw_event_stru *event;
    uint32_t ul_ret;

    if ((mac_vap->uc_init_flag == MAC_VAP_INVAILD) || (param == NULL)) {
        oam_warning_log1(0, OAM_SF_FTM, "{hmac_wifi_rtt_result_report::pst_mac_vap->uc_init_flag[%d]!}",
            mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛RTT wifi result上报事件到WAL */
    event_mem = frw_event_alloc_m(sizeof(mac_ftm_wifi_rtt_result));
    if (event_mem == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_wifi_rtt_result_report::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 填写事件 */
    event = frw_get_event_stru(event_mem);
    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_HOST_CTX, HMAC_HOST_CTX_EVENT_SUB_TYPE_RTT_RESULT_RPT,
        sizeof(mac_ftm_wifi_rtt_result), FRW_EVENT_PIPELINE_STAGE_0, mac_vap->uc_chip_id, mac_vap->uc_device_id,
        mac_vap->uc_vap_id);

    if (memcpy_s((uint8_t *)frw_get_event_payload(event_mem), sizeof(mac_ftm_wifi_rtt_result), param,
        sizeof(mac_ftm_wifi_rtt_result)) != EOK) {
        oam_error_log0(0, OAM_SF_FTM, "hmac_wifi_rtt_result_report::memcpy fail!");
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    ul_ret = frw_event_dispatch_event(event_mem);
    if (ul_ret != OAL_SUCC) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_FTM, "{hmac_wifi_rtt_result_report::dispatch_event fail}");
    }
    frw_event_free_m(event_mem);

    return ul_ret;
}
#endif


uint32_t hmac_config_get_version(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_GET_VERSION, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_get_version::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY

uint32_t hmac_config_get_user_rssbw(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_add_user_param_stru *pst_user;
    hmac_vap_stru *pst_hmac_vap;
    hmac_user_stru *pst_hmac_user;
    uint32_t ret;

    pst_user = (mac_cfg_add_user_param_stru *)puc_param;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_OPMODE, "{hmac_config_get_user_rssbw::pst_hmac_vap null.}");
        return OAL_FAIL;
    }

    pst_hmac_user = mac_vap_get_hmac_user_by_addr(pst_mac_vap, pst_user->auc_mac_addr);
    if (pst_hmac_user == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE, "{hmac_config_get_user_rssbw::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE,
                     "{hmac_config_get_user_rssbw:: user macaddr %02X:XX:XX:%02X:%02X:%02X.}",
                     pst_user->auc_mac_addr[MAC_ADDR_0], pst_user->auc_mac_addr[MAC_ADDR_3],
                     pst_user->auc_mac_addr[MAC_ADDR_4], pst_user->auc_mac_addr[MAC_ADDR_5]);
    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE,
                     "{hmac_config_get_user_rssbw::nss_cap[%d]avail_nss[%d]user bw_cap[%d]avail_bw[%d].}",
                     pst_hmac_user->st_user_base_info.en_user_max_cap_nss,
                     pst_hmac_user->st_user_base_info.en_avail_num_spatial_stream,
                     pst_hmac_user->st_user_base_info.en_bandwidth_cap,
                     pst_hmac_user->st_user_base_info.en_avail_bandwidth);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_GET_USER_RSSBW, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE,
                       "{hmac_config_get_user_rssbw::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

#endif

#ifdef _PRE_WLAN_FEATURE_M2S

uint32_t hmac_config_set_m2s_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{hmac_config_set_m2s_switch::\
                en_vap_state is MAC_VAP_STATE_ROAMING m2s_switch is forbid.}");
        return OAL_SUCC;
    }
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_M2S_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                       "{hmac_config_set_m2s_switch::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif


uint32_t hmac_config_ru_set(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_RU_TEST, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_config_radar_set::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_HIEX

uint32_t hmac_config_set_user_hiex_enable(mac_vap_stru *pst_mac_vap,
    uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_cfg_user_hiex_param_stru *pst_cfg       = NULL;
    hmac_user_stru               *pst_hmac_user = NULL;
    mac_hiex_cap_stru            *pst_cap       = NULL;

    pst_cfg = (mac_cfg_user_hiex_param_stru *)puc_param;
    pst_hmac_user = mac_vap_get_hmac_user_by_addr(pst_mac_vap, pst_cfg->auc_mac_addr);
    if (pst_hmac_user == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_HIEX, "{hmac_config_set_user_hiex_enable::pst_hmac_user null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cap = MAC_USER_GET_HIEX_CAP(&pst_hmac_user->st_user_base_info);
    mac_hiex_nego_cap(pst_cap, &pst_cfg->st_hiex_cap, NULL);

    oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_user_hiex_enable::user id %d, enable %d}",
                     pst_hmac_user->st_user_base_info.us_assoc_id, pst_cap->bit_himit_enable);

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_USER_HIEX_ENABLE, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_HIEX,
                         "{hmac_config_set_user_hiex_enable::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif


uint32_t hmac_config_set_ampdu_aggr_num(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_AGGR_NUM, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_ampdu_aggr_num::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

#ifdef _PRE_WLAN_FEATURE_CSI

uint32_t hmac_roam_enable_timeout_handle(void *p_arg)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)p_arg;
    hmac_roam_info_stru *roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;

    roam_info->stay_on_current_bss = OAL_FALSE; // 恢复接受ap发的漫游请求
    return hmac_roam_enable(hmac_vap, OAL_TRUE);
}

uint32_t hmac_csi_enable_roam(mac_vap_stru *mac_vap, uint8_t *param)
{
    uint8_t roam_enable_value;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_roam_info_stru *roam_info = NULL;
    mac_cfg_csi_param_stru *cfg_csi_param = NULL;

    cfg_csi_param = (mac_cfg_csi_param_stru *)param;
    roam_enable_value = cfg_csi_param->en_cfg_csi_on ? 0 : 1; // csi打开，漫游关闭；csi关闭，漫游打开

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_roam_enable::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_roam_enable::roam_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (roam_enable_value == 0) {
        /* 当前不在漫游，可以关闭漫游 */
        if (mac_vap->en_vap_state != MAC_VAP_STATE_ROAMING) {
            if (hmac_roam_enable(hmac_vap, roam_enable_value) != OAL_SUCC) {
                oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_roam_enable::roam was disable}");
            } else {
                /* 起定时器，2分钟后驱动侧自动恢复漫游使能 */
                frw_timer_create_timer_m(&roam_info->st_timer, hmac_roam_enable_timeout_handle,
                    ROAM_REENABLE_TIMEOUT_MS, hmac_vap, OAL_FALSE, OAM_MODULE_ID_HMAC, mac_vap->core_id);

                roam_info->stay_on_current_bss = OAL_TRUE; // 拒绝ap发的漫游请求，驻留当前bss
            }
        } else {
            /* 当前正在漫游，不能关闭漫游，此次关闭漫游命令不执行 */
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_set_roam_enable::roam is running! can't disable roam.}");
        }
    } else {
        /* 上层bssid驻留不到2分钟提前使能漫游 */
        if (hmac_roam_enable(hmac_vap, roam_enable_value) != OAL_SUCC) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_roam_enable::roam was enable}");
        } else {
            /* 删除定时器 */
            frw_timer_immediate_destroy_timer_m(&roam_info->st_timer);
            roam_info->stay_on_current_bss = OAL_FALSE; // 恢复接受ap发的漫游请求
        }
    }

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_CSI
mac_csi_black_list_stru g_csi_black_list = {0};
OAL_STATIC mac_csi_black_list_stru *hmac_get_csi_black_list_handler(void)
{
    return &g_csi_black_list;
}

OAL_STATIC uint32_t hmac_config_csi_set_black_list_rsp(mac_vap_stru *pst_mac_vap,
    uint8_t uc_len, uint8_t *puc_param)
{
    mac_csi_black_list_stru *csi_black_list = hmac_get_csi_black_list_handler();

    csi_black_list->csi_black_list_num++;
    if (csi_black_list->csi_black_list_num >= WLAN_CSI_BLACK_LIST_NUM) {
        memcpy_s(&csi_black_list->csi_black_list_mac_map[WLAN_CSI_BLACK_LIST_NUM - 1][MAC_ADDR_0],
            WLAN_MAC_ADDR_LEN, puc_param, WLAN_MAC_ADDR_LEN);
    } else {
        memcpy_s(&csi_black_list->csi_black_list_mac_map[csi_black_list->csi_black_list_num - 1][MAC_ADDR_0],
            WLAN_MAC_ADDR_LEN, puc_param, WLAN_MAC_ADDR_LEN);
    }
    return OAL_SUCC;
}
#endif

uint32_t hmac_csi_config_1103(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;
    mac_cfg_csi_param_stru *cfg_csi_param = (mac_cfg_csi_param_stru *)param;
    mac_csi_black_list_stru *csi_black_list = hmac_get_csi_black_list_handler();

    if (mac_vap->en_vap_state == MAC_VAP_STATE_UP) {
        ret = hmac_csi_enable_roam(mac_vap, param);
        if (ret != OAL_SUCC) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_csi::hmac_csi_enable_roam fail.}");
        }
    }
    if (cfg_csi_param->en_cfg_csi_on) { // csi开启后同步黑名单信息到dmac
        ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SET_CSI_BLACK_LIST,
            sizeof(mac_csi_black_list_stru), (uint8_t *)csi_black_list);
        if (ret != OAL_SUCC) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_csi::set black list fail.}");
        }
    }
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SET_CSI, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_csi_config_1103::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_csi_config(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return wlan_chip_csi_config(mac_vap, len, param);
}
#endif
#ifdef _PRE_WLAN_FEATURE_TXBF

uint32_t hmac_config_vap_close_txbf_cap(mac_vap_stru *pst_mac_vap)
{
    uint32_t ret;
    uint16_t us_len;
    uint8_t uc_rx_sts_num = 1;

    oal_bool_enum_uint8 en_txbf_rx_cap = OAL_FALSE;
    /* 当前mac device只支持bfee 不支持bfer 当前只处理bfee能力变化，bfer待开发 */
#ifdef _PRE_WLAN_FEATURE_TXBF_HT
    mac_mib_set_ReceiveStaggerSoundingOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(pst_mac_vap, uc_rx_sts_num);
    mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(pst_mac_vap, OAL_FALSE);
    pst_mac_vap->st_txbf_add_cap.bit_channel_est_cap = OAL_FALSE;
#endif
    mac_mib_set_VHTSUBeamformeeOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_VHTBeamformeeNTxSupport(pst_mac_vap, uc_rx_sts_num);

    oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_TXBF, "hmac_config_vap_close_txbf_cap::close txbf.");

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    us_len = sizeof(en_txbf_rx_cap);

    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TXBF_MIB_UPDATE, us_len, &en_txbf_rx_cap);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_TXBF,
                         "{hmac_config_vap_update_txbf_cap::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif


uint32_t hmac_config_set_blacklist_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint32_t *pul_cfg_mode;

    pul_cfg_mode = (uint32_t *)puc_param;
    ret = hmac_blacklist_set_mode(pst_mac_vap, (uint8_t)*pul_cfg_mode);
    if (ret != OAL_SUCC) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_blacklist_set_mode fail: ret=%d; mode=%d}", ret, *pul_cfg_mode);
        return ret;
    }
    return OAL_SUCC;
}


uint32_t hmac_config_get_blacklist_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    uint32_t ret;

    ret = hmac_blacklist_get_mode(pst_mac_vap, puc_param);
    if (ret != OAL_SUCC) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_blacklist_get_mode fail: ret=%d; mode=%d}", ret, *puc_param);
        return ret;
    }
    return OAL_SUCC;
}


uint32_t hmac_config_blacklist_add(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_blacklist_stru *pst_blklst;
    mac_vap_stru *pst_assoc_vap = NULL;
    oal_bool_enum_uint8 en_assoc_vap;

    pst_blklst = (mac_blacklist_stru *)puc_param;
    en_assoc_vap = hmac_blacklist_get_assoc_ap(pst_mac_vap, pst_blklst->auc_mac_addr, &pst_assoc_vap);
    if (en_assoc_vap == OAL_TRUE) {
        ret = hmac_blacklist_add(pst_assoc_vap, pst_blklst->auc_mac_addr, pst_blklst->aging_time);
    } else {
        ret = hmac_blacklist_add(pst_mac_vap, pst_blklst->auc_mac_addr, pst_blklst->aging_time);
    }
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_blacklist_add fail: ret=%d;}", ret);
        return ret;
    }
    return OAL_SUCC;
}


uint32_t hmac_config_blacklist_add_only(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_blacklist_stru *pst_blklst;
    mac_vap_stru *pst_assoc_vap = NULL;
    oal_bool_enum_uint8 en_assoc_vap;

    pst_blklst = (mac_blacklist_stru *)puc_param;
    en_assoc_vap = hmac_blacklist_get_assoc_ap(pst_mac_vap, pst_blklst->auc_mac_addr, &pst_assoc_vap);
    if (en_assoc_vap == OAL_TRUE) {
        ret = hmac_blacklist_add_only(pst_assoc_vap, pst_blklst->auc_mac_addr, 0);
    } else {
        ret = hmac_blacklist_add_only(pst_mac_vap, pst_blklst->auc_mac_addr, 0);
    }
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_config_blacklist_add_only fail: ret=%d;}", ret);
        return ret;
    }
    return OAL_SUCC;
}


uint32_t hmac_config_blacklist_del(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_vap_stru *pst_assoc_vap = NULL;
    oal_bool_enum_uint8 en_assoc_vap;

    en_assoc_vap = hmac_blacklist_get_assoc_ap(pst_mac_vap, puc_param, &pst_assoc_vap);
    if (en_assoc_vap == OAL_TRUE) {
        ret = hmac_blacklist_del(pst_assoc_vap, puc_param);
    } else {
        ret = hmac_blacklist_del(pst_mac_vap, puc_param);
    }
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_config_blacklist_del::blacklist_del fail: ret=%d;}", ret);

        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_config_autoblacklist_enable(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint8_t uc_enable;

    uc_enable = *puc_param;

    ret = hmac_autoblacklist_enable(pst_mac_vap, uc_enable);
    if (ret != OAL_SUCC) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_autoblacklist_enable fail: ret=%d; cfg=%d}", ret, *puc_param);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_config_set_autoblacklist_aging(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint32_t *pul_cfg_aging_time;

    pul_cfg_aging_time = (uint32_t *)puc_param;
    ret = hmac_autoblacklist_set_aging(pst_mac_vap, *pul_cfg_aging_time);
    if (ret != OAL_SUCC) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_autoblacklist_set_aging fail: ret=%d; cfg=%d}", ret, *pul_cfg_aging_time);
        return ret;
    }
    return OAL_SUCC;
}


uint32_t hmac_config_set_autoblacklist_threshold(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint32_t *pul_cfg_threshold;

    pul_cfg_threshold = (uint32_t *)puc_param;
    ret = hmac_autoblacklist_set_threshold(pst_mac_vap, *pul_cfg_threshold);
    if (ret != OAL_SUCC) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_autoblacklist_set_threshold fail: ret=%d; cfg=%d}", ret, *pul_cfg_threshold);
        return ret;
    }
    return OAL_SUCC;
}


uint32_t hmac_config_set_autoblacklist_reset_time(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint32_t *pul_cfg_reset_time;

    pul_cfg_reset_time = (uint32_t *)puc_param;
    ret = hmac_autoblacklist_set_reset_time(pst_mac_vap, *pul_cfg_reset_time);
    if (ret != OAL_SUCC) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_autoblacklist_set_reset_time fail: ret=%d; cfg=%d}", ret, *pul_cfg_reset_time);
        return ret;
    }
    return OAL_SUCC;
}


uint32_t hmac_config_set_pmksa(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_pmksa_param_stru *cfg_pmksa = NULL;
    hmac_pmksa_cache_stru *p_pmksa_cache = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    oal_dlist_head_stru *p_pmksa_entry = NULL;
    oal_dlist_head_stru *p_pmksa_entry_tmp = NULL;
    uint32_t pmksa_cnt = 0;

    if (oal_any_null_ptr2(mac_vap, puc_param)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_pmksa param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    cfg_pmksa = (mac_cfg_pmksa_param_stru *)puc_param;

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_pmksa::hmac_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (oal_dlist_is_empty(&(hmac_vap->st_pmksa_list_head))) {
        oal_dlist_init_head(&(hmac_vap->st_pmksa_list_head));
    }

    oal_dlist_search_for_each_safe(p_pmksa_entry, p_pmksa_entry_tmp, &(hmac_vap->st_pmksa_list_head))
    {
        p_pmksa_cache = oal_dlist_get_entry(p_pmksa_entry, hmac_pmksa_cache_stru, st_entry);
        /* 已存在时，先删除，保证最新的pmk在dlist头部 */
        if (oal_compare_mac_addr(cfg_pmksa->auc_bssid, p_pmksa_cache->auc_bssid) == 0) {
            oal_dlist_delete_entry(p_pmksa_entry);
            oal_mem_free_m(p_pmksa_cache, OAL_TRUE);
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_set_pmksa:: DEL first [%02X:XX:XX:XX:%02X:%02X]}",
                cfg_pmksa->auc_bssid[MAC_ADDR_0], cfg_pmksa->auc_bssid[MAC_ADDR_4], cfg_pmksa->auc_bssid[MAC_ADDR_5]);
        }
        pmksa_cnt++;
    }

    if (pmksa_cnt > WLAN_PMKID_CACHE_SIZE) {
        /* 超过最大个数时，先队列尾，保证最新的pmk在dlist头部 */
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_pmksa:: can't store more pmksa for [%02X:XX:XX:XX:%02X:%02X]}",
            cfg_pmksa->auc_bssid[MAC_ADDR_0], cfg_pmksa->auc_bssid[MAC_ADDR_4], cfg_pmksa->auc_bssid[MAC_ADDR_5]);
        p_pmksa_entry = oal_dlist_delete_tail(&(hmac_vap->st_pmksa_list_head));
        p_pmksa_cache = oal_dlist_get_entry(p_pmksa_entry, hmac_pmksa_cache_stru, st_entry);
        oal_mem_free_m(p_pmksa_cache, OAL_TRUE);
    }

    p_pmksa_cache = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_pmksa_cache_stru), OAL_TRUE);
    if (p_pmksa_cache == NULL) {
        oam_error_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_pmksa:: oal_mem_alloc_m fail [%02X:XX:XX:XX:%02X:%02X]}",
            cfg_pmksa->auc_bssid[MAC_ADDR_0], cfg_pmksa->auc_bssid[MAC_ADDR_4], cfg_pmksa->auc_bssid[MAC_ADDR_5]);
        return OAL_ERR_CODE_PTR_NULL;
    }

    memcpy_s(p_pmksa_cache->auc_bssid, OAL_MAC_ADDR_LEN, cfg_pmksa->auc_bssid, OAL_MAC_ADDR_LEN);
    memcpy_s(p_pmksa_cache->auc_pmkid, WLAN_PMKID_LEN, cfg_pmksa->auc_pmkid, WLAN_PMKID_LEN);

    oal_dlist_add_head(&(p_pmksa_cache->st_entry), &(hmac_vap->st_pmksa_list_head));

    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
        "{hmac_config_set_pmksa:: SET pmksa for [%02X:XX:XX:XX:%02X:%02X] OK!}",
        cfg_pmksa->auc_bssid[MAC_ADDR_0], cfg_pmksa->auc_bssid[MAC_ADDR_4], cfg_pmksa->auc_bssid[MAC_ADDR_5]);

    return OAL_SUCC;
}


uint32_t hmac_config_del_pmksa(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_pmksa_param_stru *pst_cfg_pmksa = NULL;
    hmac_pmksa_cache_stru *pst_pmksa_cache = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    oal_dlist_head_stru *pst_pmksa_entry = NULL;
    oal_dlist_head_stru *pst_pmksa_entry_tmp = NULL;

    if (oal_any_null_ptr2(pst_mac_vap, puc_param)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_del_pmksa param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cfg_pmksa = (mac_cfg_pmksa_param_stru *)puc_param;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_pmksa::pst_hmac_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (oal_dlist_is_empty(&(pst_hmac_vap->st_pmksa_list_head))) {
        oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_del_pmksa:: pmksa dlist is null [%02X:XX:XX:XX:%02X:%02X]}",
                         pst_cfg_pmksa->auc_bssid[MAC_ADDR_0],
                         pst_cfg_pmksa->auc_bssid[MAC_ADDR_4],
                         pst_cfg_pmksa->auc_bssid[MAC_ADDR_5]);
    }

    oal_dlist_search_for_each_safe(pst_pmksa_entry, pst_pmksa_entry_tmp, &(pst_hmac_vap->st_pmksa_list_head))
    {
        pst_pmksa_cache = oal_dlist_get_entry(pst_pmksa_entry, hmac_pmksa_cache_stru, st_entry);
        if (oal_compare_mac_addr(pst_cfg_pmksa->auc_bssid, pst_pmksa_cache->auc_bssid) == 0) {
            oal_dlist_delete_entry(pst_pmksa_entry);
            oal_mem_free_m(pst_pmksa_cache, OAL_TRUE);
            oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_del_pmksa:: DEL pmksa of [%02X:XX:XX:XX:%02X:%02X]}",
                             pst_cfg_pmksa->auc_bssid[MAC_ADDR_0],
                             pst_cfg_pmksa->auc_bssid[MAC_ADDR_4],
                             pst_cfg_pmksa->auc_bssid[MAC_ADDR_5]);
            return OAL_SUCC;
        }
    }

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_del_pmksa:: NO pmksa of [%02X:XX:XX:XX:%02X:%02X]}",
                     pst_cfg_pmksa->auc_bssid[MAC_ADDR_0],
                     pst_cfg_pmksa->auc_bssid[MAC_ADDR_4],
                     pst_cfg_pmksa->auc_bssid[MAC_ADDR_5]);
    return OAL_SUCC;
}


uint32_t hmac_config_flush_pmksa(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_pmksa_cache_stru *pst_pmksa_cache = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    oal_dlist_head_stru *pst_pmksa_entry = NULL;
    oal_dlist_head_stru *pst_pmksa_entry_tmp = NULL;

    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_flush_pmksa param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_flush_pmksa::pst_hmac_vap null}");
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
                         pst_pmksa_cache->auc_bssid[MAC_ADDR_0],
                         pst_pmksa_cache->auc_bssid[MAC_ADDR_4],
                         pst_pmksa_cache->auc_bssid[MAC_ADDR_5]);
    }

    return OAL_SUCC;
}


uint32_t hmac_config_scan_abort(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap = NULL;
    hmac_device_stru *pst_hmac_device;
    uint32_t ret;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_scan_abort::pst_hmac_device is null, dev_id[%d].}",
                         pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
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
        }

        if (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
            hmac_p2p_listen_timeout(pst_hmac_vap, &pst_hmac_vap->st_vap_base_info);
        }
    }

    if ((pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP)
        && (pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.en_vap_last_state != MAC_VAP_STATE_BUTT)) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_scan_abort::en_vap_last_state:%d}",
                         pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.en_vap_last_state);
        hmac_fsm_change_state(pst_hmac_vap, pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.en_vap_last_state);
        pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.en_vap_last_state = MAC_VAP_STATE_BUTT;
    }
    /* 删除HMAC 扫描超时定时器 */
    if (pst_hmac_device->st_scan_mgmt.st_scan_timeout.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(pst_hmac_device->st_scan_mgmt.st_scan_timeout));
    }

    /***************************************************************************
                         抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SCAN_ABORT, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_scan_abort::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_CONFIG_HISIFASTPOWER_DOWN_STATE

void hmac_config_force_scan_complete_for_power_down(oal_net_device_stru *net_dev, hmac_vap_stru *hmac_vap,
                                                    hmac_device_stru *hmac_device, mac_vap_stru *mac_vap)
{
    hmac_scan_stru *scan_mgmt = NULL;
    uint32_t pedding_data = 0;

    scan_mgmt = &(hmac_device->st_scan_mgmt);

    /* 如果是来自内部的扫描 */
    if (scan_mgmt->pst_request == NULL) {
        /* 判断是否存在内部扫描，如果存在，也需要停止 */
        if ((scan_mgmt->en_is_scanning == OAL_TRUE) &&
            (mac_vap->uc_vap_id == scan_mgmt->st_scan_record_mgmt.uc_vap_id)) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_scan_force_complete_for_power_down:stop scan!}");
            /* 终止扫描 */
            hmac_config_scan_abort(&hmac_vap->st_vap_base_info, sizeof(uint32_t), (uint8_t *)&pedding_data);
        }

        return;
    }

    /* 如果是上层下发的扫描请求，停止本netdev 发起的扫描 */
    if ((scan_mgmt->pst_request != NULL) && oal_wdev_match(net_dev, scan_mgmt->pst_request)) {
        /* 删除等待扫描超时定时器 */
        if (hmac_vap->st_scan_timeout.en_is_registerd == OAL_TRUE) {
            frw_timer_immediate_destroy_timer_m(&(hmac_vap->st_scan_timeout));
        }

        scan_mgmt->pst_request = NULL;
        scan_mgmt->en_complete = OAL_TRUE;

        /* 下发device终止扫描 */
        hmac_config_scan_abort(&hmac_vap->st_vap_base_info, sizeof(uint32_t), (uint8_t *)&pedding_data);

        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_scan_force_complete_for_power_down:scan abort!}");
    }

    return;
}


void hmac_config_change_wifi_state_for_power_down(void)
{
    oal_net_device_stru *net_dev = NULL;
    mac_vap_stru *mac_vap = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_device_stru *hmac_device = NULL;

    net_dev = oal_dev_get_by_name("wlan0");
    if (net_dev == NULL) {
        return;
    }
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        return;
    }

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_change_wifi_state_for_power_down:hmac_device[%d] is null!}",
                         mac_vap->uc_device_id);
        return;
    }

    /* 获取hmac vap */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_change_wifi_state_for_power_down:hmac_vap is null, vap_id[%d]!}",
                         mac_vap->uc_vap_id);
        return;
    }

    /* 如果wifi业务侧正在进行扫描，则扫描终止 */
    hmac_config_force_scan_complete_for_power_down(net_dev, hmac_vap, hmac_device, mac_vap);

    /* 如果wifi业务侧正在进行关联，删除关联定时器 */
    if (hmac_vap->st_mgmt_timer.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(hmac_vap->st_mgmt_timer));
    }

    return;
}
#endif


uint32_t hmac_config_remain_on_channel(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_remain_on_channel_param_stru *remain_on_channel = NULL;
    mac_device_stru *mac_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_device_stru *hmac_device = NULL;
    uint32_t ret;

    /* 1.1 判断入参 */
    if (oal_any_null_ptr2(mac_vap, puc_param)) {
        oam_error_log0(0, OAM_SF_P2P, "{hmac_config_remain_on_channel::mac_vap or puc_param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    remain_on_channel = (hmac_remain_on_channel_param_stru *)puc_param;
    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_config_remain_on_channel::mac_device[%d] null!}", mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (oal_unlikely(hmac_device == NULL)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_config_remain_on_channel::hmac_device[%d] is null.}", mac_vap->uc_device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_config_remain_on_channel::mac_res_get_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 1.2 检查是否能进入监听状态 */
    ret = hmac_scan_check_is_dispatch_scan_req(hmac_vap, hmac_device);
    if (ret != OAL_SUCC) {
        /* 不能进入监听状态，返回设备忙 */
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_config_remain_on_channel::not dispatch scan, device busy: ret=%d}", ret);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    /* 1.3 获取home 信道和信道类型。如果返回主信道为0，表示没有设备处于up 状态，监听后不需要返回主信道 */
    /* 保存内核下发的监听信道信息，用于监听超时或取消监听时返回 */
    mac_device->st_p2p_info.st_listen_channel = remain_on_channel->st_listen_channel;

    /* 由于p2p0和 p2p cl 共用一个VAP 结构，故在进入监听时，需要保存之前的状态，便于监听结束时返回 */
    if (mac_vap->en_p2p_mode != WLAN_LEGACY_VAP_MODE) {
        if (mac_vap->en_vap_state != MAC_VAP_STATE_STA_LISTEN) {
            mac_device->st_p2p_info.en_last_vap_state = mac_vap->en_vap_state;
        }
        remain_on_channel->en_last_vap_state = mac_device->st_p2p_info.en_last_vap_state;
    } else {
        if (mac_vap->en_vap_state != MAC_VAP_STATE_STA_LISTEN) {
            mac_vap->en_last_vap_state = mac_vap->en_vap_state;
        }
        remain_on_channel->en_last_vap_state = mac_vap->en_last_vap_state;
    }
    oam_info_log3(mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_config_remain_on_channel:: \
        listen_channel=%d, current_channel=%d, last_state=%d}", remain_on_channel->uc_listen_channel,
        mac_vap->st_channel.uc_chan_number, remain_on_channel->en_last_vap_state);

    /* 3.1 修改VAP 状态为监听 */
    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_config_remain_on_channel::fail! hmac_vap is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 状态机调用:  hmac_p2p_remain_on_channel */
    ret = hmac_fsm_call_func_sta(hmac_vap, HMAC_FSM_INPUT_LISTEN_REQ, (void *)(remain_on_channel));
    if (ret != OAL_SUCC) {
        /* DMAC 设置切换信道失败 */
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_config_remain_on_channel::fail, ret=%d}", ret);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    oam_info_log3(mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_config_remain_on_channel::channel=%d, duration=%d, band=%d}",
        remain_on_channel->uc_listen_channel, remain_on_channel->listen_duration, remain_on_channel->en_band);
    return OAL_SUCC;
}


uint32_t hmac_config_cancel_remain_on_channel(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
            "hmac_config_cancel_remain_on_channel::mac_res_get_hmac_vap fail.vap_id = %u", pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
        hmac_p2p_listen_timeout(pst_hmac_vap, pst_mac_vap);
    } else {
        hmac_p2p_send_listen_expired_to_host(pst_hmac_vap);
    }
    return OAL_SUCC;
}


uint32_t hmac_config_vap_classify_en(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t val;
    mac_device_stru *pst_mac_device = NULL;
    int8_t ac_string[OAM_PRINT_FORMAT_LENGTH];

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_warning_log1(0, OAM_SF_P2P, "hmac_config_vap_classify_en::mac_res_get_dev fail.device_id = %u",
                         pst_mac_vap->uc_device_id);

        return OAL_ERR_CODE_PTR_NULL;
    }

    val = *((uint32_t *)puc_param);

    if (val == 0xff) {
        /* 打印当前的值 */
        snprintf_s(ac_string, sizeof(ac_string), sizeof(ac_string) - 1,
                   "device classify en is %d\n",
                   pst_mac_device->en_vap_classify);

        oam_print(ac_string);

        return OAL_SUCC;
    }

    if (val == 0) {
        pst_mac_device->en_vap_classify = OAL_FALSE;
    } else {
        pst_mac_device->en_vap_classify = OAL_TRUE;
    }

    return OAL_SUCC;
}

uint32_t hmac_config_query_station_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_STATION_STATS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_phy_stat_info::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_query_rssi(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_RSSI, us_len, puc_param);

    return ret;
}


uint32_t hmac_config_query_psst(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_PSST, us_len, puc_param);

    return ret;
}


uint32_t hmac_config_query_rate(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_RATE, us_len, puc_param);

    return ret;
}

#ifdef _PRE_WLAN_DFT_STAT

uint32_t hmac_config_query_ani(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_ANI, us_len, puc_param);

    return ret;
}
#endif


uint32_t hmac_config_query_psm_flt_stat(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_PSM_STAT, us_len, puc_param);

    return ret;
}


uint32_t hmac_config_vap_classify_tid(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t val;
    hmac_vap_stru *pst_hmac_vap = NULL;
    int8_t ac_string[OAM_PRINT_FORMAT_LENGTH];

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_config_vap_classify_tid::mac_res_get_hmac_vap fail.vap_id = %u", pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    val = *((uint32_t *)puc_param);

    if (val == 0xff) {
        /* 打印当前的值 */
        snprintf_s(ac_string, sizeof(ac_string), sizeof(ac_string) - 1,
                   "vap classify tid is %d\n",
                   mac_mib_get_VAPClassifyTidNo(pst_mac_vap));

        oam_print(ac_string);

        return OAL_SUCC;
    }

    if (val >= WLAN_TIDNO_BUTT) {
        /* 打印当前的值 */
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "val is invalid:%d, vap classify tid is %d",
                         val, mac_mib_get_VAPClassifyTidNo(pst_mac_vap));
        return OAL_SUCC;
    }

    mac_mib_set_VAPClassifyTidNo(pst_mac_vap, (uint8_t)val);

    return OAL_SUCC;
}


uint32_t hmac_atcmdsrv_dbb_num_response(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    dmac_atcmdsrv_atcmd_response_event *pst_atcmdsrv_dbb_num_response_event = NULL;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_dbb_num_response::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    pst_atcmdsrv_dbb_num_response_event = (dmac_atcmdsrv_atcmd_response_event *)(puc_param);
    if (pst_atcmdsrv_dbb_num_response_event->uc_event_id == OAL_ATCMDSRV_DBB_NUM_INFO_EVENT) {
        pst_hmac_vap->st_atcmdsrv_get_status.dbb_num = pst_atcmdsrv_dbb_num_response_event->event_para;
    }
    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_get_dbb_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}


uint32_t hmac_atcmdsrv_get_ant_response(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    dmac_atcmdsrv_atcmd_response_event *pst_atcmdsrv_dbb_num_response_event = NULL;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_dbb_num_response::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_atcmdsrv_dbb_num_response_event = (dmac_atcmdsrv_atcmd_response_event *)(puc_param);
    if (pst_atcmdsrv_dbb_num_response_event->uc_event_id == OAL_ATCMDSRV_GET_ANT) {
        pst_hmac_vap->st_atcmdsrv_get_status.uc_ant_status = pst_atcmdsrv_dbb_num_response_event->event_para;
    }
    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_get_ant_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}

uint32_t hmac_atcmdsrv_get_rx_pkcg(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    dmac_atcmdsrv_atcmd_response_event *pst_atcmdsrv_get_rx_pkcg_event = NULL;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_get_rx_pkcg::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_atcmdsrv_get_rx_pkcg_event = (dmac_atcmdsrv_atcmd_response_event *)(puc_param);
    if (pst_atcmdsrv_get_rx_pkcg_event->uc_event_id == OAL_ATCMDSRV_GET_RX_PKCG) {
        pst_hmac_vap->st_atcmdsrv_get_status.rx_pkct_succ_num = pst_atcmdsrv_get_rx_pkcg_event->event_para;
        pst_hmac_vap->st_atcmdsrv_get_status.s_rx_rssi = pst_atcmdsrv_get_rx_pkcg_event->s_always_rx_rssi;
    }
    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_get_rx_pkct_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}

uint32_t hmac_atcmdsrv_lte_gpio_check(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    dmac_atcmdsrv_atcmd_response_event *pst_atcmdsrv_lte_gpio_check_event = NULL;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_lte_gpio_check::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_atcmdsrv_lte_gpio_check_event = (dmac_atcmdsrv_atcmd_response_event *)(puc_param);
    if (pst_atcmdsrv_lte_gpio_check_event->uc_event_id == OAL_ATCMDSRV_LTE_GPIO_CHECK) {
        /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
        pst_hmac_vap->st_atcmdsrv_get_status.uc_lte_gpio_check_flag = OAL_TRUE;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        g_uc_dev_lte_gpio_level = pst_atcmdsrv_lte_gpio_check_event->uc_reserved;
#endif
        oal_wait_queue_wake_up(&(pst_hmac_vap->query_wait_q));
    }

    return OAL_SUCC;
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef PLATFORM_DEBUG_ENABLE


uint32_t hmac_atcmdsrv_report_efuse_reg(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    uint16_t loop;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_lte_gpio_check::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    for (loop = 0; loop < 32; loop++) { /* buffer length 32 */
        g_us_efuse_buffer[loop] = *(uint16_t *)(puc_param);
        puc_param += BYTE_OFFSET_2;
    }
    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_report_efuse_reg_flag = OAL_TRUE;
    oal_wait_queue_wake_up(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}
#endif
#endif

OAL_STATIC uint32_t hmac_config_d2h_user_info_syn(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_d2h_syn_info_stru *pst_syn_info = NULL;
    mac_user_stru *pst_mac_user = NULL;
    uint8_t uc_idx;
    uint32_t ret;
    uint8_t band;
#ifdef _PRE_WLAN_FEATURE_160M
    uint8_t fem_enable = 0;
#endif

    if ((pst_mac_vap->uc_init_flag == MAC_VAP_INVAILD) || oal_any_null_ptr2(pst_mac_vap->pst_mib_info, puc_param)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_d2h_user_info_syn::pst_mac_vap->uc_init_flag[%d], \
                         pst_mac_vap->pst_mib_info or puc_param null!}", pst_mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_syn_info = (mac_d2h_syn_info_stru *)puc_param;

    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(pst_syn_info->us_user_idx);
    if (oal_unlikely(pst_mac_user == NULL)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_user_info_syn::pst_mac_user null.user idx [%d]}", pst_syn_info->us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 同步USR带宽 */
    mac_user_set_bandwidth_cap(pst_mac_user, pst_syn_info->en_bandwidth_cap);
    mac_user_set_bandwidth_info(pst_mac_user, pst_syn_info->en_avail_bandwidth, pst_syn_info->en_cur_bandwidth);

#ifdef _PRE_WLAN_FEATURE_160M
    /* STA从160M切换到非160M带宽时，关闭fem低功耗 */
    if (pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS &&
        pst_syn_info->st_channel.en_bandwidth < WLAN_BAND_WIDTH_160PLUSPLUSPLUS) {
        hmac_config_fem_lp_flag(pst_mac_vap, sizeof(uint8_t), &fem_enable);
    }
#endif

    /* 同步信道信息 */
    band = mac_get_band_by_channel_num(pst_syn_info->st_channel.uc_chan_number);
    ret = mac_get_channel_idx_from_num(band, pst_syn_info->st_channel.uc_chan_number,
                                       pst_syn_info->st_channel.ext6g_band, &uc_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                         "{hmac_d2h_user_info_syn::mac_get_channel_idx_from_num failed[%d].}", ret);

        return ret;
    }

    pst_mac_vap->st_channel.uc_chan_number = pst_syn_info->st_channel.uc_chan_number;
    pst_mac_vap->st_channel.en_band = pst_syn_info->st_channel.en_band;
    pst_mac_vap->st_channel.en_bandwidth = pst_syn_info->st_channel.en_bandwidth;
    pst_mac_vap->st_channel.uc_chan_idx = uc_idx;

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_d2h_user_info_syn::channe[%d],bandwidth[%d]avail bw_cap[%d],cur_bw_cap[%d]!}",
                     pst_mac_vap->st_channel.uc_chan_number, pst_mac_vap->st_channel.en_bandwidth,
                     pst_mac_user->en_avail_bandwidth, pst_mac_user->en_cur_bandwidth);

    return OAL_SUCC;
}

#if (defined(_PRE_WLAN_FEATURE_TXBF) && defined(_PRE_WLAN_FEATURE_TXBF_HT))
OAL_STATIC void hmac_config_d2h_vap_update_11n_sounding_mib(
    mac_d2h_mib_update_info_stru *pst_mib_update_info, mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev)
{
    if (pst_mib_update_info->en_11n_sounding == OAL_TRUE) {
        mac_mib_set_TransmitStaggerSoundingOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_SUBFER(pst_mac_dev));
        mac_mib_set_ReceiveStaggerSoundingOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_SUBFEE(pst_mac_dev));
        mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(pst_mac_vap, WLAN_MIB_HT_ECBF_DELAYED);
        mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(pst_mac_vap, HT_BFEE_NTX_SUPP_ANTA_NUM);
    } else {
        mac_mib_set_TransmitStaggerSoundingOptionImplemented(pst_mac_vap, OAL_FALSE);
        mac_mib_set_ReceiveStaggerSoundingOptionImplemented(pst_mac_vap, OAL_FALSE);
        mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(pst_mac_vap, WLAN_MIB_HT_ECBF_INCAPABLE);
        mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(pst_mac_vap, 1);
    }
}
#endif

OAL_STATIC uint32_t hmac_config_d2h_vap_mib_update(mac_vap_stru *mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_d2h_mib_update_info_stru *mib_update_info = NULL;
    mac_device_stru *mac_dev = NULL;
    oal_bool_enum_uint8 en_vht_sgi_is_supp_160_80p80;

    if ((mac_vap->uc_init_flag == MAC_VAP_INVAILD) || oal_any_null_ptr2(mac_vap->pst_mib_info, puc_param)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_d2h_vap_mib_update::uc_init_flag[%d], pst_mib_info or \
            puc_param null!}", mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_dev = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_dev == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_vap_mib_update::mac_dev[%d] null.}", mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    mib_update_info = (mac_d2h_mib_update_info_stru *)puc_param;
    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_d2h_vap_mib_update::11ax_cap=%d,radar_cap=%d \
        lan_bw_max=%d, beacon_period=%d}", mib_update_info->en_11ax_cap,mib_update_info->en_radar_detector_cap,
        mib_update_info->en_wlan_bw_max, mib_update_info->us_beacon_period);

    /* 设置带宽mib能力 */
    mac_mib_set_dot11VapMaxBandWidth(mac_vap, mib_update_info->en_wlan_bw_max);
    /* 更新VHT相关mib能力 */
    mac_mib_set_VHTChannelWidthOptionImplemented(mac_vap, mac_device_trans_bandwith_to_vht_capinfo(mac_mib_get_dot11VapMaxBandWidth(mac_vap)));
    en_vht_sgi_is_supp_160_80p80 = (mac_mib_get_dot11VapMaxBandWidth(mac_vap) >= WLAN_BW_CAP_160M) ?
        WLAN_HAL0_VHT_SGI_SUPP_160_80P80 : OAL_FALSE;
    mac_mib_set_VHTShortGIOptionIn160and80p80Implemented(mac_vap, en_vht_sgi_is_supp_160_80p80);

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        mac_vap->en_11ax_hal_cap = mib_update_info->en_11ax_cap;
        if (!MAC_VAP_IS_SUPPORT_11AX(mac_vap)) {
            mac_mib_set_HEOptionImplemented(mac_vap, OAL_FALSE);
        }
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_d2h_vap_mib_update::en_11ax_hal=[%d],ax_custom=[%d],mib_11ax=[%d]}",
            mac_vap->en_11ax_hal_cap, mac_vap->en_11ax_custom_switch, mac_mib_get_HEOptionImplemented(mac_vap));
    }
#endif

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    mac_mib_set_txopps(mac_vap, mib_update_info->en_txopps_is_supp);
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
    MAC_VAP_GET_SUPPORT_DFS(mac_vap) = mib_update_info->en_radar_detector_cap;
#endif

    /* 11n sounding mib设置 */
#if (defined(_PRE_WLAN_FEATURE_TXBF) && defined(_PRE_WLAN_FEATURE_TXBF_HT))
    hmac_config_d2h_vap_update_11n_sounding_mib(mib_update_info, mac_vap, mac_dev);
#endif

    /* green filed mib设置 */
    /* 暂时未使用，使用时打开 */
#ifdef _PRE_WLAN_FEATURE_TXBF
    mac_mib_set_VHTMUBeamformeeOptionImplemented(mac_vap, mib_update_info->en_mu_beamformee_cap);

    mac_mib_set_VHTNumberSoundingDimensions(mac_vap, mib_update_info->uc_bfer_num_sounding_dim);
    mac_mib_set_VHTSUBeamformeeOptionImplemented(mac_vap, mib_update_info->en_su_bfee);
    /* 设置su_bfee能力:bfee支持的接收空时流数 */
    mac_mib_set_VHTBeamformeeNTxSupport(mac_vap, mib_update_info->uc_su_bfee_num);
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        mac_mib_set_he_SUBeamformee(mac_vap, mib_update_info->en_su_bfee);
        mac_mib_set_he_BeamformeeSTSBelow80Mhz(mac_vap, mib_update_info->uc_he_ntx_sts_below_80m);
        mac_mib_set_he_BeamformeeSTSOver80Mhz(mac_vap, mib_update_info->uc_he_ntx_sts_over_80m);
        mac_mib_set_HENumberSoundingDimensionsBelow80Mhz(mac_vap, mib_update_info->uc_he_num_dim_below_80m);
        mac_mib_set_HENumberSoundingDimensionsOver80Mhz(mac_vap, mib_update_info->uc_he_num_dim_over_80m);
    }
#endif
#endif

    mac_mib_set_BeaconPeriod(mac_vap, mib_update_info->us_beacon_period);

    mac_mib_set_ShortGIOptionInFortyImplemented(mac_vap, mib_update_info->en_40m_shortgi);

    mac_mib_set_FortyMHzOperationImplemented(mac_vap, mib_update_info->en_40m_enable);

    /* 设置11n txbf 的能力 */
    mac_vap->st_cap_flag.bit_11ntxbf = mib_update_info->en_11n_txbf;

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_config_d2h_vap_cap_update(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_cap_flag_stru *pst_mac_cap_flag;

    pst_mac_cap_flag = (mac_cap_flag_stru *)(puc_param);
    pst_mac_vap->st_cap_flag.bit_1024qam = !!pst_mac_cap_flag->bit_1024qam;
    pst_mac_vap->st_cap_flag.bit_nb = pst_mac_cap_flag->bit_nb;

    /* 2g5g定制化能力刷新 */
    pst_mac_vap->st_cap_flag.bit_2g_custom_siso = pst_mac_cap_flag->bit_2g_custom_siso;
    pst_mac_vap->st_cap_flag.bit_5g_custom_siso = pst_mac_cap_flag->bit_5g_custom_siso;
    pst_mac_vap->st_cap_flag.bit_bt20dbm = pst_mac_cap_flag->bit_bt20dbm;

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_d2h_vap_cap_update::1024_cap=[%d],nb_cap=[%d],2g_custom_siso[%d],bt20dbm[%d].}",
                     pst_mac_vap->st_cap_flag.bit_1024qam, pst_mac_vap->st_cap_flag.bit_nb,
                     pst_mac_vap->st_cap_flag.bit_2g_custom_siso, pst_mac_vap->st_cap_flag.bit_bt20dbm);

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_config_d2h_vap_channel_info(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_cfg_channel_param_stru *pst_channel_param;
    uint32_t ret;

    pst_channel_param = (mac_cfg_channel_param_stru *)puc_param;
    pst_mac_vap->st_channel.en_band = pst_channel_param->en_band;
    pst_mac_vap->st_channel.en_bandwidth = pst_channel_param->en_bandwidth;
    pst_mac_vap->st_channel.uc_chan_number = pst_channel_param->uc_channel;
    pst_mac_vap->st_channel.ext6g_band = pst_channel_param->ext6g_band;
    ret = mac_get_channel_idx_from_num(pst_channel_param->en_band,
                                       pst_channel_param->uc_channel,
                                       pst_channel_param->ext6g_band, &pst_mac_vap->st_channel.uc_chan_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_config_d2h_vap_channel_info::mac_get_channel_idx_from_num fail!");
    }

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "hmac_config_d2h_vap_channel_info::chan[%d] band[%d] bandwidth[%d]",
                     pst_mac_vap->st_channel.uc_chan_number,
                     pst_mac_vap->st_channel.en_band, pst_mac_vap->st_channel.en_bandwidth);
    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_config_get_dbdc_info(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "hmac_config_get_dbdc_info::device id [%d] is null", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device->en_dbdc_running = (oal_bool_enum_uint8)*puc_param;
    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "hmac_config_get_dbdc_info::dbdc state[%d]", pst_mac_device->en_dbdc_running);

    return OAL_SUCC;
}

#if defined(_PRE_WLAN_FEATURE_OPMODE_NOTIFY) || defined(_PRE_WLAN_FEATURE_SMPS)

void hmac_show_m2s_sync_cap(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, mac_user_m2s_stru *pst_syn_info)
{
    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "hmac_show_m2s_sync_cap:: \
                     original user cap user_nss[%d] avail_nss[%d] bf_nss[%d] smps_mode[%d]",
                     pst_mac_user->en_user_max_cap_nss, pst_mac_user->en_avail_num_spatial_stream,
                     pst_mac_user->en_avail_bf_num_spatial_stream, pst_mac_user->st_ht_hdl.bit_sm_power_save);

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "hmac_show_m2s_sync_cap::avail_bw[%d] cur_bw[%d]",
                     pst_mac_user->en_avail_bandwidth, pst_mac_user->en_cur_bandwidth);

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "hmac_show_m2s_sync_cap:: \
                     new user cap user_nss[%d] avail_nss[%d] bf_nss[%d] smps_mode[%d]",
                     pst_syn_info->en_user_max_cap_nss, pst_syn_info->en_avail_num_spatial_stream,
                     pst_syn_info->en_avail_bf_num_spatial_stream, pst_syn_info->en_cur_smps_mode);

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "hmac_show_m2s_sync_cap::avail_bw[%d] cur_bw[%d]",
                     pst_syn_info->en_avail_bandwidth, pst_syn_info->en_cur_bandwidth);
}


uint32_t hmac_config_user_m2s_info_syn(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_user_m2s_stru *pst_syn_info = NULL;
    mac_user_stru *pst_mac_user = NULL;

    if ((pst_mac_vap->uc_init_flag == MAC_VAP_INVAILD) || oal_any_null_ptr2(pst_mac_vap->pst_mib_info, puc_param)) {
        oam_warning_log1(0, OAM_SF_M2S, "{hmac_config_user_m2s_info_syn::pst_mac_vap->uc_init_flag[%d], \
                         pst_mac_vap->pst_mib_info or puc_param null!}",
                         pst_mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_syn_info = (mac_user_m2s_stru *)puc_param;

    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(pst_syn_info->us_user_idx);
    if (oal_unlikely(pst_mac_user == NULL)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{hmac_config_user_m2s_info_syn:: \
                         pst_mac_user null.user idx [%d]}", pst_syn_info->us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 打印同步前后user的带宽空间流信息 */
    hmac_show_m2s_sync_cap(pst_mac_vap, pst_mac_user, pst_syn_info);

    /* 同步USR带宽 */
    mac_user_set_bandwidth_info(pst_mac_user, pst_syn_info->en_avail_bandwidth, pst_syn_info->en_cur_bandwidth);

    /* 同步用户空间流 */
    mac_user_set_num_spatial_stream(pst_mac_user, pst_syn_info->en_user_max_cap_nss);
    mac_user_set_avail_num_spatial_stream(pst_mac_user, pst_syn_info->en_avail_num_spatial_stream);
    mac_user_avail_bf_num_spatial_stream(pst_mac_user, pst_syn_info->en_avail_bf_num_spatial_stream);
#ifdef _PRE_WLAN_FEATURE_SMPS
    mac_user_set_sm_power_save(pst_mac_user, pst_syn_info->en_cur_smps_mode);
#endif
    mac_user_set_smps_opmode_notify_nss(pst_mac_user, pst_syn_info->en_smps_opmode_nodify_nss);

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_M2S

uint32_t hmac_config_d2h_device_m2s_info_syn(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_device_m2s_stru *pst_syn_info;
    mac_device_stru *pst_mac_device;

    /* 该vap是配置vap，mib指针为空，此处不需要判断mib */
    if ((pst_mac_vap->uc_init_flag == MAC_VAP_INVAILD) || (puc_param == NULL)) {
        oam_warning_log1(0, OAM_SF_M2S, "{hmac_config_device_m2s_info_syn::pst_mac_vap->uc_init_flag[%d], \
                         pst_mac_vap->pst_mib_info or puc_param null!}",
                         pst_mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_syn_info = (mac_device_m2s_stru *)puc_param;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log1(0, OAM_SF_M2S, "{hmac_config_device_m2s_info_syn: mac device is null ptr. device id:%d.}",
                       pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 1.mac device的能力需要同步, 防止硬切换重启之后host没有恢复，其他能力待开发 */
    MAC_DEVICE_GET_NSS_NUM(pst_mac_device) = pst_syn_info->en_nss_num;

    /* 2.smps能力需要刷新，防止硬切换时候，vap smps mib初始化不正常 */
    MAC_DEVICE_GET_MODE_SMPS(pst_mac_device) = pst_syn_info->en_smps_mode;
    /* 关键信息同步提示 */
    oam_warning_log2(0, OAM_SF_M2S,
                     "{hmac_config_device_m2s_info_syn::en_nss_num:[%d] smps mode[%d].}",
                     pst_syn_info->en_nss_num, MAC_DEVICE_GET_MODE_SMPS(pst_mac_device));

    return OAL_SUCC;
}


uint32_t hmac_config_vap_m2s_info_syn(mac_vap_stru *pst_mac_vap)
{
    uint32_t ret;
    mac_vap_m2s_stru st_m2s_vap_info;
    mac_device_stru *pst_mac_dev;

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                       "{hmac_config_vap_m2s_info_syn::pst_mac_dev[%d] null.}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band) {
        mac_mib_set_VHTShortGIOptionIn160and80p80Implemented(pst_mac_vap, OAL_FALSE);

        /* 根据当前带宽刷新mib，5g默认160M支持 */
        mac_mib_set_VHTChannelWidthOptionImplemented(pst_mac_vap, WLAN_MIB_VHT_SUPP_WIDTH_80);
    } else { /* 5G恢复成最大默认能力 */
        if (MAC_DEVICE_GET_CAP_BW(pst_mac_dev) >= WLAN_BW_CAP_160M) {
            mac_mib_set_VHTShortGIOptionIn160and80p80Implemented(pst_mac_vap, WLAN_HAL0_VHT_SGI_SUPP_160_80P80);
        } else {
            mac_mib_set_VHTShortGIOptionIn160and80p80Implemented(pst_mac_vap, OAL_FALSE);
        }

        mac_mib_set_VHTChannelWidthOptionImplemented(pst_mac_vap,
            mac_device_trans_bandwith_to_vht_capinfo(MAC_DEVICE_GET_CAP_BW(pst_mac_dev)));
    }

    /* host暂时只是opmode的cap flag需要同步 */
    st_m2s_vap_info.en_support_opmode = pst_mac_vap->st_cap_flag.bit_opmode;

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_VAP_M2S_INFO_SYN,
                                 sizeof(mac_vap_m2s_stru), (uint8_t *)(&st_m2s_vap_info));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                         "{hmac_config_vap_m2s_info_syn::hmac_config_send_event failed[%d],user_id[%d].}", ret);
    }

    return ret;
}

OAL_INLINE void hmac_config_d2h_vap_m2s_info_syn_set_cap(mac_vap_stru *pst_mac_vap, mac_vap_m2s_stru *pst_syn_info)
{
#ifdef _PRE_WLAN_FEATURE_SMPS
    mac_mib_set_smps(pst_mac_vap, pst_syn_info->en_sm_power_save);
#endif
#ifdef _PRE_WLAN_FEATURE_TXBF_HT
    mac_mib_set_TransmitStaggerSoundingOptionImplemented(pst_mac_vap, pst_syn_info->en_transmit_stagger_sounding);
#endif
    mac_mib_set_TxSTBCOptionImplemented(pst_mac_vap, pst_syn_info->en_tx_stbc);

    mac_mib_set_vht_ctrl_field_cap(pst_mac_vap, pst_syn_info->en_vht_ctrl_field_supported);
#ifdef _PRE_WLAN_FEATURE_TXBF
    mac_mib_set_VHTTxSTBCOptionImplemented(pst_mac_vap, pst_syn_info->en_tx_vht_stbc_optionimplemented);
    mac_mib_set_VHTNumberSoundingDimensions(pst_mac_vap, pst_syn_info->en_vht_number_sounding_dimensions);
    mac_mib_set_VHTSUBeamformerOptionImplemented(pst_mac_vap, pst_syn_info->en_vht_su_beamformer_optionimplemented);

    mac_mib_set_VHTSUBeamformeeOptionImplemented(pst_mac_vap, pst_syn_info->en_su_bfee);
    mac_mib_set_VHTBeamformeeNTxSupport(pst_mac_vap, pst_syn_info->uc_vht_ntx_sts);
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        mac_mib_set_he_SUBeamformer(pst_mac_vap, pst_syn_info->en_he_su_bfer);
        mac_mib_set_he_BeamformeeSTSBelow80Mhz(pst_mac_vap, pst_syn_info->uc_he_ntx_sts_below_80m);
        mac_mib_set_he_BeamformeeSTSOver80Mhz(pst_mac_vap, pst_syn_info->uc_he_ntx_sts_over_80m);
        mac_mib_set_HENumberSoundingDimensionsBelow80Mhz(pst_mac_vap, pst_syn_info->uc_he_num_dim_below_80m);
        mac_mib_set_HENumberSoundingDimensionsOver80Mhz(pst_mac_vap, pst_syn_info->uc_he_num_dim_over_80m);
    }
#endif
#endif
}


uint32_t hmac_config_d2h_vap_m2s_info_syn(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_vap_m2s_stru *pst_syn_info;
    mac_cfg_kick_user_param_stru st_kick_user_param;
    uint32_t ret;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    
    if ((pst_mac_vap->uc_init_flag == MAC_VAP_INVAILD) || oal_any_null_ptr2(pst_mac_vap->pst_mib_info, puc_param)) {
        oam_warning_log1(0, OAM_SF_M2S,
                         "{hmac_config_d2h_vap_m2s_info_syn::pst_mac_vap->uc_init_flag[%d], \
                         pst_mac_vap->pst_mib_info or puc_param null!}",
                         pst_mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_syn_info = (mac_vap_m2s_stru *)puc_param;

    /* 1.m2s vap信息同步hmac */
    mac_vap_set_rx_nss(pst_mac_vap, pst_syn_info->en_vap_rx_nss);

    if (MAC_VAP_SPEC_IS_SW_NEED_M2S_SWITCH(pst_mac_vap)) {
        mac_vap_init_rates(pst_mac_vap);
    }

    /* 2. 修改HT能力 */
    hmac_config_d2h_vap_m2s_info_syn_set_cap(pst_mac_vap, pst_syn_info);

    if (WLAN_M2S_TYPE_HW == pst_syn_info->en_m2s_type) {
        /* 硬切换需要踢除所有主路上的用户 */
        st_kick_user_param.us_reason_code = MAC_UNSPEC_REASON;
        oal_set_mac_addr(st_kick_user_param.auc_mac_addr, auc_mac_addr);

        /* 1. 踢掉该vap的用户 */
        ret = hmac_config_kick_user(pst_mac_vap, sizeof(uint32_t), (uint8_t *)&st_kick_user_param);
        if (ret != OAL_SUCC) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                "{hmac_config_d2h_vap_m2s_info_syn::hmac_config_kick_user fail!}");
        }

        /* 2. 更新速率集，vap的空间流，暂时未看到其他需要更新的地方 */
        mac_vap_init_rates(pst_mac_vap);
    }

    /* 关键信息同步提示 */
    oam_warning_log3(0, 0, "{hmac_config_d2h_vap_m2s_info_syn::en_vap_rx_nss:%d, en_sm_power_save:%d, en_m2s_type:%d.}",
                     pst_syn_info->en_vap_rx_nss, pst_syn_info->en_sm_power_save, pst_syn_info->en_m2s_type);

    return OAL_SUCC;
}


uint32_t hmac_m2s_switch_protect_comp_event_status(mac_vap_stru *mac_vap, uint8_t uc_len, uint8_t *param)
{
    dmac_m2s_complete_syn_stru *m2s_switch_comp_status = NULL;
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    uint32_t ret = OAL_SUCC;

    if ((mac_vap->uc_init_flag == MAC_VAP_INVAILD) || (param == NULL)) {
        oam_warning_log1(0, OAM_SF_M2S, "{hmac_m2s_switch_protect_comp_event_status::mac_vap->uc_init_flag[%d], \
            or param null!}", mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    m2s_switch_comp_status = (dmac_m2s_complete_syn_stru *)param;

    /* WTDB 对于驱动自身触发不用上报，对应于butt状态，触发arp probe即可 */
    if (m2s_switch_comp_status->uc_m2s_state == MAC_M2S_COMMAND_MODE_BUTT) {
        hmac_arp_probe_type_set(mac_vap, m2s_switch_comp_status->pri_data.arp_detect_result.en_arp_detect_on,
            HMAC_VAP_ARP_PROBE_TYPE_M2S);
    } else if (m2s_switch_comp_status->uc_m2s_state == MAC_MCM_COMMAND_MODE_PROTECT) {
        /* 将AP加入切换白名单 */
        hmac_degradation_whitelist_set(mac_vap);
    } else {
        /* 抛扫描完成事件到WAL */
        event_mem = frw_event_alloc_m(sizeof(dmac_m2s_complete_syn_stru));
        if (event_mem == NULL) {
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_M2S,
                "{hmac_m2s_switch_protect_comp_event_status::event_mem null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 填写事件 */
        event = frw_get_event_stru(event_mem);

        frw_event_hdr_init(&(event->st_event_hdr),
                           FRW_EVENT_TYPE_HOST_CTX,
                           HMAC_HOST_CTX_EVENT_SUB_TYPE_M2S_STATUS,
                           sizeof(dmac_m2s_complete_syn_stru),
                           FRW_EVENT_PIPELINE_STAGE_0,
                           mac_vap->uc_chip_id,
                           mac_vap->uc_device_id,
                           mac_vap->uc_vap_id);

        if (memcpy_s((uint8_t *)frw_get_event_payload(event_mem), sizeof(dmac_m2s_complete_syn_stru),
            (uint8_t *)m2s_switch_comp_status, sizeof(dmac_m2s_complete_syn_stru)) != EOK) {
            oam_error_log0(0, OAM_SF_M2S, "hmac_m2s_switch_protect_comp_event_status::memcpy fail!");
            frw_event_free_m(event_mem);
            return OAL_FAIL;
        }

        /* 分发事件 */
        ret = frw_event_dispatch_event(event_mem);
        if (ret != OAL_SUCC) {
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_M2S,
                "{hmac_m2s_switch_protect_comp_event_status::frw_event_dispatch_event fail.}");
        }
        frw_event_free_m(event_mem);
    }
    return ret;
}

#ifdef _PRE_WLAN_FEATURE_M2S_MSS

uint32_t hmac_config_set_m2s_switch_blacklist(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_M2S_BLACKLIST, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                         "{hmac_config_set_m2s_switch_blacklist::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_set_m2s_switch_mss(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_M2S_MSS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                         "{hmac_config_set_m2s_switch_blacklist::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif

#ifdef _PRE_WLAN_FEATURE_M2S_MODEM

uint32_t hmac_config_set_m2s_switch_modem(mac_vap_stru *pst_mac_vap, uint16_t us_len,
    uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_M2S_MODEM, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
            "{hmac_config_set_m2s_switch_modem::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_M2S

uint32_t hmac_config_mimo_compatibility(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    dmac_to_hmac_mimo_compatibility_event_stru *pst_dmac_to_hmac_mimo_compatibility;
    hmac_vap_stru *pst_hmac_vap;
    hmac_user_stru *pst_hmac_user;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (oal_unlikely(pst_hmac_vap == NULL)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{hmac_config_mimo_compatibility::pst_hmac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_to_hmac_mimo_compatibility = (dmac_to_hmac_mimo_compatibility_event_stru *)puc_param;

    pst_hmac_user = mac_res_get_hmac_user(pst_dmac_to_hmac_mimo_compatibility->us_user_id);
    if (oal_unlikely(pst_hmac_user == NULL)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                         "{hmac_config_mimo_compatibility::pst_hmac_user is null! user_id is %d.}",
                         pst_dmac_to_hmac_mimo_compatibility->us_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user->en_user_ap_type = pst_dmac_to_hmac_mimo_compatibility->en_ap_type;
    oal_set_mac_addr(pst_hmac_user->auc_mimo_blacklist_mac, pst_dmac_to_hmac_mimo_compatibility->auc_mac_addr);

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                     "{hmac_config_mimo_compatibility:AP TYPE(mimo compatibility):%d.}",
                     pst_dmac_to_hmac_mimo_compatibility->en_ap_type);
    if (pst_hmac_user->en_user_ap_type & MAC_AP_TYPE_MIMO_BLACKLIST) {
        /* 发起reassoc req */
        hmac_roam_start(pst_hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_FALSE, NULL, ROAM_TRIGGER_M2S);
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH

uint32_t hmac_ant_tas_switch_rssi_notify_event_status(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event;
    uint32_t ret;

    if ((pst_mac_vap->uc_init_flag == MAC_VAP_INVAILD) || (puc_param == NULL)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{hmac_ant_tas_switch_rssi_notify_event_status::pst_mac_vap->uc_init_flag[%d], or puc_param null!}",
            pst_mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛RSSI测量完成事件到WAL */
    pst_event_mem = frw_event_alloc_m(sizeof(dmac_tas_rssi_notify_stru));
    if (pst_event_mem == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_ant_tas_switch_rssi_notify_event_status::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    pst_event = frw_get_event_stru(pst_event_mem);
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_TAS_NOTIFY_RSSI,
                       sizeof(dmac_tas_rssi_notify_stru),
                       FRW_EVENT_PIPELINE_STAGE_0,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    if (memcpy_s((uint8_t *)frw_get_event_payload(pst_event_mem), sizeof(dmac_tas_rssi_notify_stru),
        puc_param, sizeof(dmac_tas_rssi_notify_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_ant_tas_switch_rssi_notify_event_status::memcpy fail!");
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{hmac_ant_tas_switch_rssi_notify_event_status::frw_event_dispatch_event fail.}");
    }

    frw_event_free_m(pst_event_mem);

    return ret;
}
#endif

#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP

uint32_t hmac_hid2d_drop_report(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event;
    uint32_t ret;

    if ((pst_mac_vap->uc_init_flag == MAC_VAP_INVAILD) || (puc_param == NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_hid2d_drop_report::pst_mac_vap->uc_init_flag[%d], or puc_param null!}",
                         pst_mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛hid2d丢包事件到WAL */
    pst_event_mem = frw_event_alloc_m(sizeof(uint16_t));
    if (pst_event_mem == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_hid2d_drop_report::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 填写事件 */
    pst_event = frw_get_event_stru(pst_event_mem);
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_HID2D_SEQNUM,
                       sizeof(uint16_t),
                       FRW_EVENT_PIPELINE_STAGE_0,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    if (EOK != memcpy_s((uint8_t *)frw_get_event_payload(pst_event_mem), sizeof(uint16_t),
        puc_param, sizeof(uint16_t))) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_hid2d_drop_report::memcpy fail!");
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,    \
            "{hmac_hid2d_drop_report::frw_event_dispatch_event fail.}");
    }
    frw_event_free_m(pst_event_mem);

    return ret;
}
#endif
uint32_t hmac_d2h_amsdu_switch_report(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    hmac_vap_stru *hmac_vap = NULL;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_M2S, "{hmac_d2h_amsdu_switch_report::hmac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_vap->d2h_amsdu_switch = *param;
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_M2S, "{d2h_amsdu_switch::flag[%d]!}", hmac_vap->d2h_amsdu_switch);
    return OAL_SUCC;
}


uint32_t hmac_config_stop_altx(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    pst_mac_vap->bit_al_tx_flag = OAL_SWITCH_OFF;

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_config_query_sta_mngpkt_sendstat_rsp(mac_vap_stru *pst_mac_vap,
                                                              uint8_t uc_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_dev = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    mac_cfg_query_mngpkt_sendstat_stru *pst_param;
    pst_param = (mac_cfg_query_mngpkt_sendstat_stru *)puc_param;

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_query_sta_mngpkt_sendstat_rsp::mac_res_get_dev failed.}");
        return OAL_FAIL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "hmac_config_query_sta_mngpkt_sendstat_rsp: pst_hmac_vap is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_dev->uc_auth_req_sendst = pst_param->uc_auth_req_st;
    pst_mac_dev->uc_asoc_req_sendst = pst_param->uc_asoc_req_st;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_config_query_rssi_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_cfg_query_rssi_stru *pst_param = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    pst_param = (mac_cfg_query_rssi_stru *)puc_param;

    pst_hmac_user = mac_res_get_hmac_user(pst_param->us_user_id);
    if (pst_hmac_user == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "hmac_config_query_rssi_rsp: pst_hmac_user is null ptr. user id:%d", pst_param->us_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_query_rssi_rsp: pst_hmac_vap is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user->c_rssi = pst_param->c_rssi;
    pst_hmac_user->c_free_power = pst_param->c_free_power;

    pst_hmac_vap->station_info_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_DELAY_STATISTIC

OAL_STATIC uint32_t hmac_config_receive_sta_delay(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    user_delay_info_stru *pst_param;
    char *type_name[] = { "tid_sta_delay", "air_sta_delay" };
    pst_param = (user_delay_info_stru *)puc_param;
#ifndef _PRE_LINUX_TEST
    printk(KERN_INFO "%s, 0: %d, 1~5 ms: %d, 5~10 ms: %d, 10~15 ms: %d, 15~20 ms: %d, 20~25 ms: %d, \
           25~30 ms: %d, 30~35 ms: %d, 35~40 ms: %d, >=40 ms: %d",
           type_name[pst_param->dl_measure_type % 2],
           pst_param->dl_time_array[BYTE_OFFSET_0], pst_param->dl_time_array[BYTE_OFFSET_1],
           pst_param->dl_time_array[BYTE_OFFSET_2], pst_param->dl_time_array[BYTE_OFFSET_3],
           pst_param->dl_time_array[BYTE_OFFSET_4], pst_param->dl_time_array[BYTE_OFFSET_5],
           pst_param->dl_time_array[BYTE_OFFSET_6], pst_param->dl_time_array[BYTE_OFFSET_7],
           pst_param->dl_time_array[BYTE_OFFSET_8], pst_param->dl_time_array[BYTE_OFFSET_9]);
#endif
    return OAL_SUCC;
}
#endif /* _PRE_WLAN_DELAY_STATISTIC */

OAL_STATIC uint32_t hmac_config_query_psst_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_cfg_query_psst_stru *pst_param = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    pst_param = (mac_cfg_query_psst_stru *)puc_param;

    pst_hmac_user = mac_res_get_hmac_user(pst_param->us_user_id);
    if (pst_hmac_user == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "hmac_config_query_psst_rsp: pst_hmac_user is null ptr. user id:%d", pst_param->us_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_query_psst_rsp: pst_hmac_vap is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user->uc_ps_st = pst_param->uc_ps_st;

    pst_hmac_vap->station_info_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));
    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_config_query_rate_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_cfg_query_rate_stru *pst_param = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    pst_param = (mac_cfg_query_rate_stru *)puc_param;
    pst_hmac_user = mac_res_get_hmac_user(pst_param->us_user_id);
    if (pst_hmac_user == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "hmac_config_query_rate_rsp: pst_hmac_user is null ptr. user id:%d", pst_param->us_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_query_rate_rsp: pst_hmac_vap is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user->tx_rate = pst_param->tx_rate;
    pst_hmac_user->tx_rate_min = pst_param->tx_rate_min;
    pst_hmac_user->tx_rate_max = pst_param->tx_rate_max;
    pst_hmac_user->rx_rate = pst_param->rx_rate;
    pst_hmac_user->rx_rate_min = pst_param->rx_rate_min;
    pst_hmac_user->rx_rate_max = pst_param->rx_rate_max;
#ifdef _PRE_WLAN_DFT_STAT
    pst_hmac_user->uc_cur_per = pst_param->uc_cur_per;
    pst_hmac_user->uc_bestrate_per = pst_param->uc_bestrate_per;
#endif

    pst_hmac_vap->station_info_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_config_psm_query_rsp_proc(hmac_psm_flt_stat_query_stru *pst_hmac_psm_query,
                                                   mac_psm_query_msg *pst_query_msg)
{
    mac_psm_query_stat_stru  *pst_psm_stat = &pst_hmac_psm_query->ast_psm_stat[pst_query_msg->en_query_type];

    pst_psm_stat->query_item = pst_query_msg->st_stat.query_item;
    if (memcpy_s(pst_psm_stat->aul_val, sizeof(pst_psm_stat->aul_val), pst_query_msg->st_stat.aul_val,
        (pst_psm_stat->query_item + 1) * sizeof(uint32_t)) != EOK) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_psm_query_rsp_proc::memcpy fail, query type[%d] query_item[%d]}",
                       pst_query_msg->en_query_type, pst_query_msg->st_stat.query_item);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t  hmac_config_query_psm_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_device_stru        *pst_hmac_device = NULL;
    hmac_psm_flt_stat_query_stru *pst_hmac_psm_query = NULL;
    mac_psm_query_msg       *pst_query_rsp_msg = NULL;
    uint32_t query_item = 0;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "hmac_config_query_psm_flt_stat: pst_hmac_device is null ptr. device id:%d",
            pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_hmac_psm_query = &pst_hmac_device->st_psm_flt_stat_query;
    pst_query_rsp_msg = (mac_psm_query_msg*)puc_param;

    if (OAL_SUCC == hmac_config_psm_query_rsp_proc(pst_hmac_psm_query, pst_query_rsp_msg)) {
#ifdef _PRE_WLAN_FEATURE_PSM_ABN_PKTS_STAT
        if (pst_query_rsp_msg->en_query_type == MAC_PSM_QUERY_ABNORMAL_PKTS_CNT) {
            query_item = pst_query_rsp_msg->st_stat.query_item;
            pst_hmac_psm_query->ast_psm_stat[pst_query_rsp_msg->en_query_type].aul_val[query_item] =
                pst_hmac_device->addba_req_cnt;
            pst_hmac_psm_query->ast_psm_stat[pst_query_rsp_msg->en_query_type].aul_val[query_item + 1] =
                pst_hmac_device->group_rekey_cnt;
            /* addba req count和 group rekey count 2个item */
            pst_hmac_psm_query->ast_psm_stat[pst_query_rsp_msg->en_query_type].query_item += 2;
            oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_query_psm_rsp::addba_req_cnt[%d], group_rekey_cnt[%d]}",
                             pst_hmac_device->addba_req_cnt, pst_hmac_device->group_rekey_cnt);
        }
#endif

        /* 设置wait条件为true */
        pst_hmac_psm_query->auc_complete_flag[pst_query_rsp_msg->en_query_type] = OAL_TRUE;
        oal_wait_queue_wake_up_interrupt(&(pst_hmac_device->st_psm_flt_stat_query.st_wait_queue));
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_DFT_STAT

OAL_STATIC uint32_t hmac_config_query_ani_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_cfg_query_ani_stru *pst_param;
    hmac_vap_stru *pst_hmac_vap;

    pst_param = (mac_cfg_query_ani_stru *)puc_param;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_query_ani_rsp: pst_hmac_vap is null ptr.");
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

OAL_STATIC uint32_t hmac_config_d2h_trigger_reassoc(mac_vap_stru *pst_mac_vap, uint8_t uc_len,
    uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "hmac_config_d2h_trigger_reassoc: pst_hmac_vap is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 发起reassoc req */
    return hmac_roam_start(pst_hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_FALSE, NULL, ROAM_TRIGGER_D2H_REASSOC);
}

#ifdef _PRE_WLAN_FEATURE_NAN

OAL_STATIC uint32_t hmac_config_nan_response(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;

    event_mem = frw_event_alloc_m(sizeof(mac_nan_rsp_msg_stru));
    if (event_mem == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_nan_response::frw_event_alloc_m fail!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 填写事件 */
    event = frw_get_event_stru(event_mem);
    frw_event_hdr_init(&(event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_NAN_RSP,
                       sizeof(mac_nan_rsp_msg_stru),
                       FRW_EVENT_PIPELINE_STAGE_0,
                       mac_vap->uc_chip_id,
                       mac_vap->uc_device_id,
                       mac_vap->uc_vap_id);

    if (memcpy_s((uint8_t *)frw_get_event_payload(event_mem), sizeof(mac_nan_rsp_msg_stru),
        param, sizeof(mac_nan_rsp_msg_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_CHAN, "hmac_config_nan_response::memcpy fail!");
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }
    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);
    return OAL_SUCC;
}
#endif

/*****************************************************************************
    g_ast_hmac_config_syn: dmac向hmac同步控制面数据处理函数表
*****************************************************************************/
OAL_STATIC const hmac_config_syn_stru g_ast_hmac_config_syn[] = {
    /* 同步ID                    保留2个字节            函数操作 */
    { WLAN_CFGID_QUERY_STATION_STATS, { 0, 0 }, hmac_proc_query_response_event },
    { WLAN_CFGID_RESET_HW_OPERATE, { 0, 0 }, hmac_reset_sys_event },
    { WLAN_CFGID_BTCOEX_RX_DELBA_TRIGGER, { 0, 0 }, hmac_btcoex_rx_delba_trigger },
    { WLAN_CFGID_QUERY_RSSI, { 0, 0 }, hmac_config_query_rssi_rsp },
    { WLAN_CFGID_QUERY_PSST, { 0, 0 }, hmac_config_query_psst_rsp },
    {WLAN_CFGID_QUERY_PSM_STAT,         {0, 0}, hmac_config_query_psm_rsp},
#ifdef _PRE_WLAN_DELAY_STATISTIC
    { WLAN_CFGID_NOTIFY_STA_DELAY, { 0, 0 }, hmac_config_receive_sta_delay },
#endif
    { WLAN_CFGID_QUERY_RATE, { 0, 0 }, hmac_config_query_rate_rsp },
#ifdef _PRE_WLAN_DFT_STAT
    { WLAN_CFGID_QUERY_ANI, { 0, 0 }, hmac_config_query_ani_rsp },
#endif

    { WLAN_CFGID_GET_VERSION,  { 0, 0 }, hmac_atcmdsrv_dbb_num_response },
    { WLAN_CFGID_GET_ANT,      { 0, 0 }, hmac_atcmdsrv_get_ant_response },

    { WLAN_CFGID_RX_FCS_INFO, { 0, 0 }, hmac_atcmdsrv_get_rx_pkcg },
    { WLAN_CFGID_GET_DIEID, { 0, 0 }, hmac_config_get_dieid_rsp },
    { WLAN_CFGID_CHECK_LTE_GPIO, { 0, 0 }, hmac_atcmdsrv_lte_gpio_check },
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef PLATFORM_DEBUG_ENABLE
    { WLAN_CFGID_REG_INFO, { 0, 0 }, hmac_atcmdsrv_report_efuse_reg },
#endif
#endif
    { WLAN_CFGID_CFG80211_MGMT_TX_STATUS, { 0, 0 }, hmac_mgmt_tx_event_status },
    { WLAN_CFGID_USR_INFO_SYN,            { 0, 0 }, hmac_config_d2h_user_info_syn },
    { WLAN_CFGID_VAP_MIB_UPDATE,          { 0, 0 }, hmac_config_d2h_vap_mib_update },
    { WLAN_CFGID_VAP_CAP_UPDATE,          { 0, 0 }, hmac_config_d2h_vap_cap_update },
    { WLAN_CFGID_VAP_CHANNEL_INFO_SYN,    { 0, 0 }, hmac_config_d2h_vap_channel_info },
    { WLAN_CFGID_GET_MNGPKT_SENDSTAT,     { 0, 0 }, hmac_config_query_sta_mngpkt_sendstat_rsp },

#if defined(_PRE_WLAN_FEATURE_OPMODE_NOTIFY) || defined(_PRE_WLAN_FEATURE_SMPS)
    { WLAN_CFGID_USER_M2S_INFO_SYN, { 0, 0 }, hmac_config_user_m2s_info_syn },
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
    { WLAN_CFGID_DEVICE_M2S_INFO_SYN, { 0, 0 }, hmac_config_d2h_device_m2s_info_syn },
    { WLAN_CFGID_VAP_M2S_INFO_SYN,    { 0, 0 }, hmac_config_d2h_vap_m2s_info_syn },
    { WLAN_CFGID_M2S_SWITCH_COMP,     { 0, 0 }, hmac_m2s_switch_protect_comp_event_status },
#endif
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    { WLAN_CFGID_ANT_TAS_SWITCH_RSSI_NOTIFY, { 0, 0 }, hmac_ant_tas_switch_rssi_notify_event_status },
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
    { WLAN_CFGID_HID2D_SEQNUM_REPORT, { 0, 0 }, hmac_hid2d_drop_report },
#endif
    { WLAN_CFGID_AMSDU_SWITCH_REPORT, { 0, 0 }, hmac_d2h_amsdu_switch_report },
#ifdef _PRE_WLAN_FEATURE_11K
    { WLAN_CFGID_REQ_SAVE_BSS_INFO, { 0, 0 }, hmac_scan_rrm_proc_save_bss },
#endif

    { WLAN_CFGID_VOWIFI_REPORT, { 0, 0 }, hmac_config_vowifi_report },

    { HAL_TX_COMP_SUB_TYPE_AL_TX, { 0, 0 }, hmac_config_stop_altx },

    { WLAN_CFGID_GET_DBDC_INFO, { 0, 0 }, hmac_config_get_dbdc_info },

#ifdef _PRE_WLAN_FEATURE_M2S
    { WLAN_CFGID_MIMO_COMPATIBILITY, { 0, 0 }, hmac_config_mimo_compatibility },
#endif

#ifdef _PRE_WLAN_FEATURE_NRCOEX
    { WLAN_CFGID_QUERY_NRCOEX_STAT, { 0, 0 }, hmac_config_report_nrcoex_stat },
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef _PRE_WLAN_FEATURE_DDR_BUGFIX
    { WLAN_CFGID_SET_DDR, { 0, 0 }, hmac_set_ddr },
#endif
#endif
    { WLAN_CFGID_HAL_VAP_UPDATE, { 0, 0 }, hmac_config_d2h_hal_vap_syn},
    { WLAN_CFGID_LUT_USR_UPDATE, { 0, 0 }, hmac_config_d2h_usr_lut_syn},
    { WLAN_CFGID_RX_MODE_SWITCH, { 0, 0 }, hmac_config_d2h_rx_mode_syn},

#ifdef _PRE_WLAN_FEATURE_11AX
    { WLAN_UL_OFDMA_AMSDU_SYN, { 0, 0 }, _hmac_config_ul_ofdma_amsdu },
    { WLAN_CFGID_SET_SEND_TB_PPDU_FLAG, { 0, 0 }, _hmac_config_set_send_tb_ppdu_flag },
#endif
    { WLAN_CFGID_TRIGGER_REASSOC, { 0, 0 }, hmac_config_d2h_trigger_reassoc },
    { WLAN_CFGID_PS_RX_DELBA_TRIGGER, { 0, 0 }, hmac_ps_rx_delba },
    { WLAN_CFGID_PS_ARP_PROBE, { 0, 0 }, hmac_ps_rx_amsdu_arp_probe_process },
#ifdef _PRE_WLAN_FEATURE_NAN
    { WLAN_CFGID_NAN, { 0, 0 }, hmac_config_nan_response },
#endif

#ifdef _PRE_WLAN_FEATURE_HID2D
    { WLAN_CFGID_HID2D_ACS_REPORT, { 0, 0 }, hmac_hid2d_cur_chan_stat_handler },
#endif
#ifdef _PRE_WLAN_FEATURE_HIEX
    { WLAN_CFGID_D2H_MAX_TX_POWER, { 0, 0 }, hmac_save_max_tx_power_info },
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
    { WLAN_CFGID_D2H_WMMAC_PARAM, { 0, 0 }, hmac_config_d2h_wmmac_update_info },
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    { WLAN_CFGID_RTT_RESULT_REPORT, { 0, 0}, hmac_config_rtt_result_report},
#endif
    { WLAN_CFGID_SYNC_BT_STATE, { 0, 0 }, hmac_btcoex_d2h_bt_state_syn },
#ifdef _PRE_WLAN_FEATURE_DFS
    { WLAN_CFGID_ZERO_WAIT_DFS_REPORT, { 0, 0 }, hmac_dfs_zero_wait_report_event },
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
    { WLAN_CFGID_SET_CSI_BLACK_LIST, { 0, 0 }, hmac_config_csi_set_black_list_rsp },
#endif
    { WLAN_CFGID_BUTT, { 0, 0 }, NULL },
};


uint32_t hmac_event_config_syn(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    frw_event_hdr_stru *pst_event_hdr = NULL;
    dmac_to_hmac_cfg_msg_stru *pst_dmac2hmac_msg = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    mac_device_stru *pst_mac_device = NULL;
    uint32_t ret;
    uint16_t us_cfgid;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_event_config_syn::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件 */
    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);
    pst_dmac2hmac_msg = (dmac_to_hmac_cfg_msg_stru *)pst_event->auc_event_data;

    oam_info_log1(pst_event_hdr->uc_vap_id, OAM_SF_CFG,
                  "{hmac_event_config_syn::a dmac config syn event occur, cfg_id=%d.}", pst_dmac2hmac_msg->en_syn_id);

    /* 获取dmac vap */
    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_event_hdr->uc_vap_id);
    if (pst_mac_vap == NULL) {
        oam_error_log0(pst_event_hdr->uc_vap_id, OAM_SF_CFG, "{hmac_event_config_syn::pst_mac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取mac device */
    pst_mac_device = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log0(pst_event_hdr->uc_vap_id, OAM_SF_CFG, "{hmac_event_config_syn::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得cfg id对应的操作函数 */
    for (us_cfgid = 0; WLAN_CFGID_BUTT != g_ast_hmac_config_syn[us_cfgid].en_cfgid; us_cfgid++) {
        if (g_ast_hmac_config_syn[us_cfgid].en_cfgid == pst_dmac2hmac_msg->en_syn_id) {
            break;
        }
    }

    /* 异常情况，cfgid在g_ast_dmac_config_syn中不存在 */
    if (g_ast_hmac_config_syn[us_cfgid].en_cfgid == WLAN_CFGID_BUTT) {
        oam_warning_log1(pst_event_hdr->uc_vap_id, OAM_SF_CFG, "{hmac_event_config_syn::invalid en_cfgid[%d].",
                         pst_dmac2hmac_msg->en_syn_id);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 执行操作函数 */
    ret = g_ast_hmac_config_syn[us_cfgid].p_set_func(pst_mac_vap, (uint8_t)(pst_dmac2hmac_msg->us_len),
        (uint8_t *)pst_dmac2hmac_msg->auc_msg_body);
    if (ret != OAL_SUCC) {
        oam_warning_log2(pst_event_hdr->uc_vap_id, OAM_SF_CFG,
                         "{hmac_event_config_syn::p_set_func failed, ret=%d en_syn_id=%d.",
                         ret, pst_dmac2hmac_msg->en_syn_id);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_config_set_sta_pm_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_sync_cmd_common(pst_mac_vap, WLAN_CFGID_SET_PS_MODE, us_len, puc_param);
}

uint32_t hmac_config_set_sta_pm_on(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_cfg_ps_mode_param_stru st_ps_mode_param;
    hmac_vap_stru *pst_hmac_vap;
    mac_cfg_ps_open_stru *pst_sta_pm_open = (mac_cfg_ps_open_stru *)puc_param;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
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

    pst_sta_pm_open->uc_pm_enable =
        (pst_sta_pm_open->uc_pm_enable > MAC_STA_PM_SWITCH_OFF) ? MAC_STA_PM_SWITCH_ON : MAC_STA_PM_SWITCH_OFF;

    st_ps_mode_param.uc_vap_ps_mode = pst_sta_pm_open->uc_pm_enable ?
        ((pst_hmac_vap->uc_cfg_sta_pm_manual != 0xFF) ? MAX_PSPOLL_PS : pst_hmac_vap->uc_ps_mode) : NO_POWERSAVE;

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "hmac_config_set_sta_pm_on,enable[%d], ps_mode[%d]",
                     pst_sta_pm_open->uc_pm_enable, st_ps_mode_param.uc_vap_ps_mode);
    /* 先下发设置低功耗模式 */
    ret = hmac_config_set_sta_pm_mode(pst_mac_vap, sizeof(st_ps_mode_param), (uint8_t *)&st_ps_mode_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "sta_pm sta_pm mode[%d]fail", ret);
        return ret;
    }

    /* 再下发打开低功耗 */
    return hmac_config_sync_cmd_common(pst_mac_vap, WLAN_CFGID_SET_STA_PM_ON, us_len, puc_param);
}

#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
uint32_t hmac_config_set_rx_listen_ps_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len,
    uint8_t *puc_param)
{
    return hmac_config_sync_cmd_common(pst_mac_vap, WLAN_CFGID_RX_LISTEN_PS_SWITCH, us_len, puc_param);
}
#endif

#ifdef _PRE_WLAN_FEATURE_HS20

uint32_t hmac_config_set_qos_map(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint8_t uc_idx;
    hmac_cfg_qos_map_param_stru *pst_qos_map;
    hmac_vap_stru *pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);

    if (oal_any_null_ptr2(pst_hmac_vap, puc_param)) {
        oam_error_log2(0, OAM_SF_ANY, "hmac_config_set_qos_map:: pointer is null: pst_hmac_vap[%d],puc_param[%d]",
                       pst_hmac_vap, puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_qos_map = (hmac_cfg_qos_map_param_stru *)puc_param;
    oam_info_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_qos_map:uc_num_dscp_except=%d, uc_valid=%d}",
                  pst_qos_map->uc_num_dscp_except,
                  pst_qos_map->uc_valid);

    /* 判断QOS MAP SET的使能开关是否打开 */
    if (!pst_qos_map->uc_valid) {
        return OAL_FAIL;
    }

    /* 检查下发的QoS Map Set参数中的DSCP Exception fields 是否超过最大数目21 */
    if (pst_qos_map->uc_num_dscp_except > MAX_DSCP_EXCEPT) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
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
#endif  // _PRE_WLAN_FEATURE_HS20

uint32_t hmac_config_set_dc_status(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    mac_device_stru *mac_device = mac_res_get_mac_dev();
    mac_device->dc_status = *puc_param;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DC_STATUS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_dc_status::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_set_p2p_miracast_status(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_P2P_MIRACAST_STATUS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_p2p_miracast_status::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_set_p2p_ps_ops(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_cfg_p2p_ops_param_stru *pst_p2p_ops;
    pst_p2p_ops = (mac_cfg_p2p_ops_param_stru *)puc_param;
    oam_info_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_p2p_ps_ops:ctrl:%d, ct_window:%d}",
                  pst_p2p_ops->en_ops_ctrl,
                  pst_p2p_ops->uc_ct_window);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_P2P_PS_OPS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_p2p_ps_ops::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_set_p2p_ps_noa(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_cfg_p2p_noa_param_stru *pst_p2p_noa;
    pst_p2p_noa = (mac_cfg_p2p_noa_param_stru *)puc_param;
    oam_info_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                  "{hmac_config_set_p2p_ps_noa:start_time:%d, duration:%d, interval:%d, count:%d}",
                  pst_p2p_noa->start_time,
                  pst_p2p_noa->duration,
                  pst_p2p_noa->interval,
                  pst_p2p_noa->uc_count);
    /* ms to us */
    pst_p2p_noa->start_time *= HMAC_MS_TO_US;
    pst_p2p_noa->duration *= HMAC_MS_TO_US;
    pst_p2p_noa->interval *= HMAC_MS_TO_US;
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_P2P_PS_NOA, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_p2p_ps_noa::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_set_p2p_ps_stat(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_cfg_p2p_stat_param_stru *pst_p2p_stat;
    pst_p2p_stat = (mac_cfg_p2p_stat_param_stru *)puc_param;
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "(hmac_config_set_p2p_ps_stat::ctrl %d}",
                  pst_p2p_stat->uc_p2p_statistics_ctrl);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_P2P_PS_STAT, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_p2p_ps_stat::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_set_ip_addr(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 配置DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_IP_ADDR, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_ip_addr::hmac_config_send_event fail[%d].", ret);
    }

    return ret;
}


uint32_t hmac_config_user_num_spatial_stream_cap_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    mac_user_nss_stru st_user_nss;
    uint32_t ret = OAL_FALSE;
    if (pst_mac_vap == NULL) {
        oam_error_log0(pst_mac_user->uc_vap_id, OAM_SF_CFG,
                       "hmac_config_user_num_spatial_stream_cap_syn::mac vap(idx=%d) is null!");
        return ret;
    }

    st_user_nss.en_avail_num_spatial_stream = pst_mac_user->en_avail_num_spatial_stream;
    st_user_nss.en_user_max_cap_nss = pst_mac_user->en_user_max_cap_nss;
    st_user_nss.us_user_idx = pst_mac_user->us_assoc_id;
    st_user_nss.en_user_num_spatial_stream_160M = pst_mac_user->st_vht_hdl.bit_user_num_spatial_stream_160M;
    st_user_nss.en_smps_opmode_notify_nss       = pst_mac_user->en_smps_opmode_notify_nss;

    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_NSS,
                                 sizeof(mac_user_nss_stru), (uint8_t *)(&st_user_nss));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_user->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_user_num_spatial_stream_cap_syn:: \
                         hmac_config_user_num_spatial_stream_cap_syn send event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_cfg_vap_h2d(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_device_stru *pst_dev = NULL;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_add_vap::pst_vap or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_dev == NULL)) {
        oam_error_log1(0, OAM_SF_ANY,
                       "{hmac_config_cfg_vap_h2d::mac_res_get_dev fail. vap_id[%u]}",
                       pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
    抛事件到DMAC层, 创建dmac cfg vap
    ***************************************************************************/
    ret = hmac_cfg_vap_send_event(pst_dev);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_cfg_vap_send_event::hmac_config_send_event fail[%d].", ret);
    }

    return ret;
}
#ifdef _PRE_WLAN_TCP_OPT

void hmac_set_device_freq_mode(uint8_t uc_device_enable)
{
    uint32_t ret;
    config_device_freq_h2d_stru st_device_freq_type;
    mac_vap_stru *pst_mac_vap = NULL;
    uint8_t idx;

    /* 设置Host 调频使能控制 */
    hmac_set_auto_freq_mod(uc_device_enable);

    /* 设置Device 调频使能控制 */
    pst_mac_vap = mac_res_get_mac_vap(0);
    if (pst_mac_vap == NULL) {
        return;
    }

    for (idx = 0; idx < 4; idx++) { /* 共计4个device data */
        st_device_freq_type.st_device_data[idx].speed_level = g_host_speed_freq_level[idx].speed_level;
        st_device_freq_type.st_device_data[idx].cpu_freq_level =
            g_device_speed_freq_level[idx].uc_device_type;
    }

    st_device_freq_type.uc_device_freq_enable = uc_device_enable;
    st_device_freq_type.uc_set_type = FREQ_SET_MODE;

    oam_warning_log1(0, OAM_SF_CFG, "{hmac_set_device_freq_mode: enable mode[%d][1:enable,0:disable].}",
                     st_device_freq_type.uc_device_freq_enable);

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_DEVICE_FREQ,
                                 sizeof(config_device_freq_h2d_stru), (uint8_t *)(&st_device_freq_type));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_set_device_freq_mode::hmac_set_device_freq failed[%d].}", ret);
    }
}


uint32_t hmac_config_set_device_freq(uint8_t uc_device_freq_type)
{
    uint32_t ret;
    config_device_freq_h2d_stru st_device_freq_type;
    mac_vap_stru *pst_mac_vap;

    pst_mac_vap = mac_res_get_mac_vap(0);
    if (pst_mac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_device_freq_type.uc_set_freq = uc_device_freq_type;
    /* 调频类型 */
    if (uc_device_freq_type <= FREQ_HIGHEST) {
        st_device_freq_type.uc_set_type = FREQ_SET_FREQ;
    } else {
        st_device_freq_type.uc_set_type = FREQ_SET_PLAT_FREQ;
    }

    oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_device_freq: set mode[%d],device freq level[%d].}",
                     st_device_freq_type.uc_set_type, uc_device_freq_type);

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_DEVICE_FREQ,
                                 sizeof(config_device_freq_h2d_stru), (uint8_t *)(&st_device_freq_type));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_device_freq::hmac_set_device_freq failed[%d].}", ret);
    }

    return ret;
}

uint32_t hmac_config_set_device_freq_testcase(uint8_t uc_device_freq_type)
{
    uint32_t ret;
    config_device_freq_h2d_stru st_device_freq_type;
    mac_vap_stru *pst_mac_vap;

    pst_mac_vap = mac_res_get_mac_vap(0);
    if (pst_mac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_device_freq_type.uc_set_freq = uc_device_freq_type;
    /* 调频类型 */
    if (uc_device_freq_type == FREQ_TC_EN) {
        st_device_freq_type.uc_set_type = FREQ_SET_FREQ_TC_EN;
    } else if (uc_device_freq_type == FREQ_TC_EXIT) {
        st_device_freq_type.uc_set_type = FREQ_SET_FREQ_TC_EXIT;
    } else {
        st_device_freq_type.uc_set_type = FREQ_SET_BUTT;
    }

    oam_warning_log2(0, OAM_SF_CFG,
                     "{hmac_config_set_device_freq_testcase: set mode[%d],device freq testcase enable[%d].}",
                     st_device_freq_type.uc_set_type, uc_device_freq_type);

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_DEVICE_FREQ,
                                 sizeof(config_device_freq_h2d_stru), (uint8_t *)(&st_device_freq_type));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_device_freq_testcase:: failed[%d].}", ret);
    }

    return ret;
}

uint32_t hmac_config_get_device_freq(void)
{
    uint32_t ret;
    config_device_freq_h2d_stru st_device_freq_type;
    mac_vap_stru *pst_mac_vap;

    pst_mac_vap = mac_res_get_mac_vap(0);
    if (pst_mac_vap != NULL) {
        st_device_freq_type.uc_set_type = FREQ_GET_FREQ;

        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_get_device_freq!].}");

        /***************************************************************************
            抛事件到DMAC层, 同步VAP最新状态到DMAC
        ***************************************************************************/
        ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_DEVICE_FREQ,
                                     sizeof(config_device_freq_h2d_stru), (uint8_t *)(&st_device_freq_type));
        if (oal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_device_freq::hmac_set_device_freq failed[%d].}", ret);
        }
    } else {
        ret = OAL_ERR_CODE_PTR_NULL;
    }

    return ret;
}
#endif


uint32_t hmac_find_p2p_listen_channel(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint8_t *puc_p2p_ie = NULL;
    uint8_t *puc_listen_channel_ie = NULL;

    /* 查找P2P IE信息 */
    puc_p2p_ie = mac_find_vendor_ie(MAC_WLAN_OUI_WFA, MAC_WLAN_OUI_TYPE_WFA_P2P, puc_param, (int32_t)us_len);
    if (puc_p2p_ie == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_find_p2p_listen_channel::p2p ie is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 长度校验 */
    if (puc_p2p_ie[1] < MAC_P2P_MIN_IE_LEN) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_find_p2p_listen_channel::invalid p2p ie len[%d].}", puc_p2p_ie[1]);
        return OAL_FAIL;
    }

    /* 查找P2P Listen channel信息 */
    puc_listen_channel_ie =
        /* 4：OUI长度 + Vendor Specific OUI Type长度 */
        mac_find_p2p_attribute(MAC_P2P_ATTRIBUTE_LISTEN_CHAN, puc_p2p_ie + BYTE_OFFSET_6, (puc_p2p_ie[1] - 4));
    if (puc_listen_channel_ie == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_find_p2p_listen_channel::p2p listen channel ie is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* listen channel长度校验，大端 */
    if (MAC_P2P_LISTEN_CHN_ATTR_LEN != (int32_t)((puc_listen_channel_ie[BYTE_OFFSET_2] << NUM_8_BITS) +
        puc_listen_channel_ie[1])) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_find_p2p_listen_channel::invalid p2p listen channel ie len[%d].}",
                         (int32_t)((puc_listen_channel_ie[BYTE_OFFSET_2] << NUM_8_BITS) + puc_listen_channel_ie[1]));
        return OAL_FAIL;
    }

    /* 获取P2P Listen channel信息 */
    pst_mac_vap->uc_p2p_listen_channel = puc_listen_channel_ie[BYTE_OFFSET_7];
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_find_p2p_listen_channel::END CHANNEL[%d].}",
                  pst_mac_vap->uc_p2p_listen_channel);

    return OAL_SUCC;
}

void hmac_config_del_p2p_ie(uint8_t *puc_ie, uint32_t *pul_ie_len)
{
    uint8_t *puc_p2p_ie = NULL;
    uint32_t p2p_ie_len;
    uint8_t *puc_ie_end = NULL;
    uint8_t *puc_p2p_ie_end = NULL;

    if ((oal_any_null_ptr2(puc_ie, pul_ie_len)) || (*pul_ie_len == 0)) {
        return;
    }

    puc_p2p_ie = mac_find_vendor_ie(MAC_WLAN_OUI_WFA, MAC_WLAN_OUI_TYPE_WFA_P2P, puc_ie, (int32_t)(*pul_ie_len));
    if ((puc_p2p_ie == NULL) || (puc_p2p_ie[1] < MAC_P2P_MIN_IE_LEN)) {
        return;
    }

    p2p_ie_len = puc_p2p_ie[1] + MAC_IE_HDR_LEN;

    /* 将p2p ie 后面的内容拷贝到p2p ie 所在位置 */
    puc_ie_end = (puc_ie + *pul_ie_len);
    puc_p2p_ie_end = (puc_p2p_ie + p2p_ie_len);

    if (puc_ie_end >= puc_p2p_ie_end) {
        if (memmove_s(puc_p2p_ie, *pul_ie_len, puc_p2p_ie_end, (uint32_t)(puc_ie_end - puc_p2p_ie_end)) != EOK) {
            oam_error_log0(0, OAM_SF_CFG, "hmac_config_del_p2p_ie::memmove fail!");
            return;
        }
        *pul_ie_len -= p2p_ie_len;
    }
    return;
}

uint32_t hmac_config_roam_start(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    mac_cfg_set_roam_start_stru *pst_roam_start = NULL;
    hmac_roam_info_stru *roam_info = NULL;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_roam_start::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_roam_start = (mac_cfg_set_roam_start_stru *)(puc_param);
    roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    roam_info->is_roaming_trigged_by_cmd = pst_roam_start->is_roaming_trigged_by_cmd;
    if (ether_is_all_zero(pst_roam_start->auc_bssid)) {
        /* reassociation or roaming */
        return hmac_roam_start(pst_hmac_vap, (roam_channel_org_enum)pst_roam_start->uc_scan_type,
                               pst_roam_start->en_current_bss_ignore, NULL, ROAM_TRIGGER_APP);
    } else if (!oal_memcmp(pst_mac_vap->auc_bssid, pst_roam_start->auc_bssid, OAL_MAC_ADDR_LEN)) {
        /* reassociation */
        return hmac_roam_start(pst_hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_FALSE, NULL, ROAM_TRIGGER_APP);
    } else {
        /* roaming for specified BSSID */
        return hmac_roam_start(pst_hmac_vap, ROAM_SCAN_CHANNEL_ORG_DBDC,
                               OAL_TRUE, pst_roam_start->auc_bssid, ROAM_TRIGGER_BSSID);
    }
}


uint32_t hmac_config_enable_2040bss(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_device = NULL;
    oal_bool_enum_uint8 en_2040bss_switch;
    uint8_t uc_vap_idx;
    mac_vap_stru *pst_vap = NULL;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_enable_2040bss::param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_enable_2040bss:: pst_mac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    en_2040bss_switch = (*puc_param == 0) ? OAL_FALSE : OAL_TRUE;
    // 同步device下所有vap的mib 2040特性的配置开关
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == NULL) {
            oam_error_log1(0, OAM_SF_ANY, "{dmac_config_enable_2040bss::pst_mac_vap(%d) null.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }
        mac_mib_set_2040SwitchProhibited(pst_vap, ((en_2040bss_switch == OAL_TRUE) ? OAL_FALSE : OAL_TRUE));
    }
    mac_set_2040bss_switch(pst_mac_device, en_2040bss_switch);

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_enable_2040bss:: set 2040bss switch[%d]}", en_2040bss_switch);

    return hmac_config_sync_cmd_common(pst_mac_vap, WLAN_CFGID_2040BSS_ENABLE, us_len, puc_param);
}


uint32_t hmac_config_get_2040bss_sw(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_device = NULL;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_get_2040bss_sw::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_2040bss_sw:: pst_mac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *pus_len = sizeof(uint32_t);
    *((uint32_t *)puc_param) = (uint32_t)mac_get_2040bss_switch(pst_mac_device);
    return OAL_SUCC;
}


uint32_t hmac_config_get_dieid_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    mac_cfg_show_dieid_stru *pst_dieid = NULL;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_get_polynomial_params::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dieid = (mac_cfg_show_dieid_stru *)(puc_param);

    if (memcpy_s(&pst_hmac_vap->st_dieid, sizeof(mac_cfg_show_dieid_stru),
        pst_dieid, sizeof(mac_cfg_show_dieid_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_config_get_dieid_rsp::memcpy fail!");
        return OAL_FAIL;
    }

    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}


uint32_t hmac_config_get_dieid(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_GET_DIEID, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_cali_power:hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

void hmac_config_set_device_pkt_stat(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_tx_pkts_stat_stru *pst_pkts_stat;
    uint32_t durance;
    uint32_t snd_mbits;
    mac_cfg_set_tlv_stru *pst_config_para;

    pst_pkts_stat = &g_host_tx_pkts;
    pst_config_para = (mac_cfg_set_tlv_stru *)puc_param;

    if (pst_config_para->uc_cmd_type == PKT_STAT_SET_START_STAT) {
        if (pst_config_para->value == OAL_TRUE) {
            pst_pkts_stat->snd_pkts = 0;
            pst_pkts_stat->start_time = (uint32_t)oal_time_get_stamp_ms();
        } else {
            durance = (uint32_t)oal_time_get_stamp_ms();
            durance -= pst_pkts_stat->start_time;
            if (durance == 0) {
                oam_error_log2(0, OAM_SF_CFG, "{hmac_config_set_device_pkt_stat::START TIME[%d],NOW TINE[%d].}",
                               pst_pkts_stat->start_time, oal_time_get_stamp_ms());
                return;
            }
            snd_mbits = ((pst_pkts_stat->snd_pkts * pst_pkts_stat->pkt_len) / durance) >> NUM_7_BITS;

            oam_error_log4(0, OAM_SF_CFG,
                           "{hmac_config_set_device_pkt_stat::snd rate[%d]Mbits,snd pkts[%d],pktlen[%d],time[%d].}",
                           snd_mbits, pst_pkts_stat->snd_pkts, pst_pkts_stat->pkt_len, durance);
        }
    } else if (pst_config_para->uc_cmd_type == PKT_STAT_SET_FRAME_LEN) {
        pst_pkts_stat->pkt_len = pst_config_para->value;
    }
}

uint32_t hmac_config_set_tx_ampdu_type(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_set_tlv_stru *pst_set_ampdu_type = (mac_cfg_set_tlv_stru *)puc_param;

    g_uc_tx_ba_policy_select = (uint8_t)pst_set_ampdu_type->value;
    oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_tx_ampdu_type::SET TX AMPDU TYPE[%d].}",
                     g_uc_tx_ba_policy_select);

    return OAL_SUCC;
}


uint32_t hmac_config_set_auto_freq_enable(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint8_t uc_value;
    mac_cfg_set_tlv_stru *pst_set_auto_freq = (mac_cfg_set_tlv_stru *)puc_param;

    uc_value = (uint8_t)pst_set_auto_freq->value;

    if (pst_set_auto_freq->uc_cmd_type == CMD_SET_AUTO_FREQ_ENDABLE) {
        if (uc_value == FREQ_LOCK_ENABLE) {
            // 设置device调频使能
            hmac_set_device_freq_mode(FREQ_LOCK_ENABLE);
        } else {
            // 设置device调频使能
            hmac_set_device_freq_mode(FREQ_LOCK_DISABLE);
        }
    } else if (pst_set_auto_freq->uc_cmd_type == CMD_SET_DEVICE_FREQ_VALUE) {
        /* 单独设置DEVICE CPU频率 */
        hmac_config_set_device_freq(uc_value);
    } else if (pst_set_auto_freq->uc_cmd_type == CMD_SET_CPU_FREQ_VALUE) {
        if (uc_value == SCALING_MAX_FREQ) {
        } else {
        }
    } else if (pst_set_auto_freq->uc_cmd_type == CMD_SET_DDR_FREQ_VALUE) {
        if (uc_value == SCALING_MAX_FREQ) {
        } else {
        }
    } else if (pst_set_auto_freq->uc_cmd_type == CMD_GET_DEVICE_AUTO_FREQ) {
        hmac_config_get_device_freq();
    } else if (pst_set_auto_freq->uc_cmd_type == CMD_SET_DEVICE_FREQ_TC) {
        hmac_config_set_device_freq_testcase(uc_value);
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_set_device_freq:parameter error!}");
    }
    oam_warning_log2(0, OAM_SF_ANY,
                     "{hmac_config_set_auto_freq_enable:set_auto_freq_enable:uc_cmd_type = %d, uc_value = %d}",
                     pst_set_auto_freq->uc_cmd_type, uc_value);
    return OAL_SUCC;
}

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

uint32_t hmac_config_set_all_log_level(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret = 0;
    uint8_t vap_idx;
    uint8_t level;

    if (oal_any_null_ptr2(pst_mac_vap, puc_param)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_all_log_level::pst_mac_vap or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    level = (uint8_t)(*puc_param);

    for (vap_idx = 0; vap_idx < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; vap_idx++) {
        oam_log_set_vap_level(vap_idx, level);
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_ALL_LOG_LEVEL, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_all_log_level::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_set_cus_rf(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_CUS_RF, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_rf::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

uint32_t hmac_config_set_cus_dts_cali(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_CUS_DTS_CALI, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_dts_cali::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

uint32_t hmac_config_set_cus_nvram_params(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_CUS_NVRAM_PARAM, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_nvram_params::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_set_cus_dyn_cali(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_CUS_DYN_CALI_PARAM, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_dyn_cali::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */


uint32_t hmac_config_set_vendor_ie(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    oal_app_ie_stru *pst_okc_ie;
    uint32_t ret;

    pst_okc_ie = (oal_app_ie_stru *)puc_param;

    /* 设置WPS 信息 */
    ret = hmac_config_set_app_ie_to_vap(pst_mac_vap, pst_okc_ie, pst_okc_ie->en_app_ie_type);
    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "hmac_config_set_vendor_ie::vap_id=%d, ie_type=%d, ie_length=%d",
                     pst_mac_vap->uc_vap_id, pst_okc_ie->en_app_ie_type, pst_okc_ie->ie_len);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_vendor_ie::ret=[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_config_get_sta_11h_abillty(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    oal_hilink_get_sta_11h_ability *pst_sta_11h_abillty = NULL;
    hmac_vap_stru *pst_hmac_vap;
    oal_dlist_head_stru *pst_entry = NULL;
    oal_dlist_head_stru *pst_dlist_tmp = NULL;
    mac_user_stru *pst_user_tmp = NULL;
    uintptr_t temp;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log0(pst_mac_vap->uc_device_id, OAM_SF_CFG, "{hmac_config_get_sta_11h_abillty::hmac_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (puc_param == NULL) {
        oam_warning_log0(pst_mac_vap->uc_device_id, OAM_SF_CFG, "{hmac_config_get_sta_11h_abillty::puc_param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (us_len == sizeof(oal_hilink_get_sta_11h_ability) + sizeof(temp)) {
        temp = *((uintptr_t *)(puc_param + sizeof(oal_hilink_get_sta_11h_ability)));
        pst_sta_11h_abillty = (oal_hilink_get_sta_11h_ability *)(uintptr_t)(temp);
    } else {
        pst_sta_11h_abillty = (oal_hilink_get_sta_11h_ability *)puc_param;
    }

    oal_dlist_search_for_each_safe(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_user_tmp = oal_dlist_get_entry(pst_entry, mac_user_stru, st_user_dlist);
        if (pst_user_tmp == NULL) {
            oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_get_sta_11h_abillty:: pst_user_tmp NULL !!!}");
            continue;
        }

        if (oal_memcmp(pst_user_tmp->auc_user_mac_addr, pst_sta_11h_abillty->auc_sta_mac, WLAN_MAC_ADDR_LEN) == 0) {
            if (pst_user_tmp->st_cap_info.bit_spectrum_mgmt == 1) {
                pst_sta_11h_abillty->en_support_11h = OAL_TRUE;
            }
            return OAL_SUCC;
        }
    }
    return OAL_FAIL;
}


uint32_t hmac_config_set_mlme(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret = OAL_SUCC;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = NULL;
    uint16_t user_idx = 0;
    oal_w2h_mlme_ie_stru *mlme_ie;
    oal_app_ie_stru app_ie;
    uint8_t mgmt_frm_type = WLAN_FC0_SUBTYPE_ASSOC_REQ;
    mac_status_code_enum_uint16 en_status_code;

    mlme_ie = (oal_w2h_mlme_ie_stru *)puc_param;

    if ((mlme_ie->en_mlme_type == OAL_IEEE80211_MLME_ASSOC_RSP) ||
        (mlme_ie->en_mlme_type == OAL_IEEE80211_MLME_REASSOC_RSP)) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mlme::hmac assoc rsp ie type=%d, status=%d, \
            ie length[%d].}", mlme_ie->en_mlme_type, mlme_ie->us_status, mlme_ie->us_ie_len);
        if (mlme_ie->us_ie_len > WLAN_WPS_IE_MAX_SIZE) {
            return OAL_SUCC;
        }

        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
        if (hmac_vap == NULL) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                           "{hmac_config_set_mlme::hmac_vap[%d] is NULL!}", mac_vap->uc_vap_id);

            return OAL_ERR_CODE_PTR_NULL;
        }

        ret = mac_vap_find_user_by_macaddr(mac_vap, mlme_ie->auc_macaddr, &user_idx);
        if (ret != OAL_SUCC) {
            oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                "{hmac_config_set_mlme::failed find user:%02X:XX:XX:%02X:%02X:%02X.}", mlme_ie->auc_macaddr[MAC_ADDR_0],
                mlme_ie->auc_macaddr[MAC_ADDR_3], mlme_ie->auc_macaddr[MAC_ADDR_4], mlme_ie->auc_macaddr[MAC_ADDR_5]);
            hmac_mgmt_send_deauth_frame(mac_vap, mlme_ie->auc_macaddr, MAC_ASOC_NOT_AUTH, OAL_FALSE);

            return ret;
        }

        hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(user_idx);
        if (hmac_user == NULL) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_config_set_mlme::hmac_user[%d] null.}", user_idx);

            /* 没有查到对应的USER,发送去认证消息 */
            hmac_mgmt_send_deauth_frame(mac_vap, mlme_ie->auc_macaddr, MAC_ASOC_NOT_AUTH, OAL_FALSE);

            return OAL_ERR_CODE_PTR_NULL;
        }

        if (mlme_ie->en_mlme_type == OAL_IEEE80211_MLME_ASSOC_RSP) {
            mgmt_frm_type = WLAN_FC0_SUBTYPE_ASSOC_REQ;
        } else if (mlme_ie->en_mlme_type == OAL_IEEE80211_MLME_REASSOC_RSP) {
            mgmt_frm_type = WLAN_FC0_SUBTYPE_REASSOC_REQ;
        }

        app_ie.en_app_ie_type = OAL_APP_ASSOC_RSP_IE;
        app_ie.ie_len = mlme_ie->us_ie_len;
        if (memcpy_s(app_ie.auc_ie, WLAN_WPS_IE_MAX_SIZE, mlme_ie->puc_data_ie, app_ie.ie_len) != EOK) {
            oam_error_log0(0, OAM_SF_CFG, "hmac_config_set_mlme::memcpy fail!");
            return OAL_FAIL;
        }

        ret = hmac_config_set_app_ie_to_vap(mac_vap, &app_ie, app_ie.en_app_ie_type);
        if (ret != OAL_SUCC) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_set_mlme:: hmac_config_set_app_ie_to_vap fail, err_code=%d.}", ret);
            return ret;
        }

        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ASSOC,
            "{hmac_config_set_mlme::find user:%02X:XX:XX:XX:%02X:%02X, user_idx=%d.}", mlme_ie->auc_macaddr[MAC_ADDR_0],
            mlme_ie->auc_macaddr[MAC_ADDR_4], mlme_ie->auc_macaddr[MAC_ADDR_5], user_idx);

        en_status_code = mlme_ie->us_status;
        ret = hmac_ap_send_assoc_rsp(hmac_vap, hmac_user, mlme_ie->auc_macaddr, mgmt_frm_type, &en_status_code);
        if ((ret != OAL_SUCC) || (en_status_code != MAC_SUCCESSFUL_STATUSCODE)) {
            hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);
        }
    }

    return ret;
}


uint32_t hmac_config_dyn_cali_param(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DYN_CALI_CFG, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_dyn_cali_param::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

#ifdef _PRE_WLAN_FEATURE_11K

uint32_t hmac_config_send_neighbor_req(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret = OAL_SUCC;
    oal_netbuf_stru *action_neighbor_req;
    uint16_t neighbor_req_frm_len;
    mac_tx_ctl_stru *tx_ctl;
    uint16_t idx;
    uint8_t *data = NULL;
    mac_cfg_ssid_param_stru *ssid;
    mac_user_stru *mac_user;

    ssid = (mac_cfg_ssid_param_stru *)puc_param;

    action_neighbor_req =
        (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (action_neighbor_req == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_config_send_neighbor_req::action_neighbor_req null.}");
        return ret;
    }

    memset_s(oal_netbuf_cb(action_neighbor_req), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    data = (uint8_t *)oal_netbuf_header(action_neighbor_req);

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
    mac_hdr_set_frame_control(data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* duration */
    data[BYTE_OFFSET_2] = 0;
    data[BYTE_OFFSET_3] = 0;

    mac_user = mac_res_get_mac_user(mac_vap->us_assoc_vap_id);
    if (mac_user == NULL) {
        oam_warning_log1(0, OAM_SF_TX, "{hmac_config_send_neighbor_req::mac_user[%d] null.", mac_vap->us_assoc_vap_id);
        oal_netbuf_free(action_neighbor_req);

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* DA is address of STA requesting association */
    oal_set_mac_addr(data + BYTE_OFFSET_4, mac_user->auc_user_mac_addr);

    /* SA is the dot11MACAddress */
    oal_set_mac_addr(data + BYTE_OFFSET_10, mac_mib_get_StationID(mac_vap));

    oal_set_mac_addr(data + BYTE_OFFSET_16, mac_vap->auc_bssid);

    /* seq control */
    data[BYTE_OFFSET_22] = 0;
    data[BYTE_OFFSET_23] = 0;

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*             Neighbor report request Frame - Frame Body                */
    /*        -------------------------------------------------              */
    /*        | Category | Action |  Dialog Token | Opt SubEle |             */
    /*        -------------------------------------------------              */
    /*        | 1        | 1      |       1       | Var        |             */
    /*        -------------------------------------------------              */
    /*************************************************************************/
    /* Initialize index and the frame data pointer */
    idx = MAC_80211_FRAME_LEN;

    /* Category */
    data[idx++] = MAC_ACTION_CATEGORY_RADIO_MEASURMENT;

    /* Action */
    data[idx++] = MAC_RM_ACTION_NEIGHBOR_REPORT_REQUEST;

    /* Dialog Token */
    data[idx++] = 1;

    if (ssid->uc_ssid_len != 0) {
        /* Subelement ID */
        data[idx++] = 0;

        /* length */
        data[idx++] = ssid->uc_ssid_len;

        /* SSID */
        if (memcpy_s(data + idx, WLAN_MEM_NETBUF_SIZE2 - idx, ssid->ac_ssid, ssid->uc_ssid_len) != EOK) {
            oam_error_log0(0, OAM_SF_TX, "hmac_config_send_neighbor_req::memcpy fail!");
            oal_netbuf_free(action_neighbor_req);
            return OAL_FAIL;
        }
        idx += ssid->uc_ssid_len;
    }

    neighbor_req_frm_len = idx;

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(action_neighbor_req);
    MAC_GET_CB_MPDU_LEN(tx_ctl) = neighbor_req_frm_len;
    /* 发送完成需要获取user结构体 */
    ret = mac_vap_set_cb_tx_user_idx(mac_vap, tx_ctl, mac_user->auc_user_mac_addr);
    if (ret != OAL_SUCC) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_TX, "(hmac_config_send_neighbor_req::fail to find user by \
            xx:xx:xx:0x:0x:0x.}", mac_user->auc_user_mac_addr[MAC_ADDR_3],
            mac_user->auc_user_mac_addr[MAC_ADDR_4], mac_user->auc_user_mac_addr[MAC_ADDR_5]);
    }

    oal_netbuf_put(action_neighbor_req, neighbor_req_frm_len);

    ret = hmac_tx_mgmt_send_event(mac_vap, action_neighbor_req, neighbor_req_frm_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(action_neighbor_req);
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_send_neighbor_req::send_event failed[%d]}", ret);
    }

    return ret;
}
#endif  // _PRE_WLAN_FEATURE_11K

#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU

void hmac_config_set_mcast_ampdu_retry(mac_vap_stru *mac_vap, uint32_t cmd)
{
    uint32_t ret;

    if (mac_get_mcast_ampdu_switch() != OAL_TRUE) {
        return;
    }
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_MCAST_AMPDU_RETRY, sizeof(uint32_t), (uint8_t *)&cmd);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_TX,
                         "{hmac_config_set_mcast_ampdu_retry:hmac_config_send_event fail[%d]", ret);
    }
}
#endif

OAL_STATIC void hmac_config_common_debug_part1(mac_vap_stru *pst_mac_vap, uint16_t us_set_id, uint32_t value)
{
#ifdef _PRE_WLAN_FEATURE_HIEX
    mac_device_stru *pst_mac_device = NULL;
#endif

    if (us_set_id == RIFS_ENABLE) {
        mac_mib_set_RifsMode(pst_mac_vap, (oal_bool_enum_uint8)value);
    } else if (us_set_id == GREENFIELD_ENABLE) {
        mac_mib_set_HTGreenfieldOptionImplemented(pst_mac_vap, (oal_bool_enum_uint8)value);
    } else if (us_set_id == AUTH_RSP_TIMEOUT) {
        g_st_mac_device_custom_cfg.us_cmd_auth_rsp_timeout = (uint16_t)value;
    } else if (us_set_id == FORBIT_OPEN_AUTH) {
        g_st_mac_device_custom_cfg.bit_forbit_open_auth = (value == 0 ? OAL_FALSE : OAL_TRUE);
    } else if (us_set_id == HT_SELF_CURE_DEBUG) {
        hmac_ht_self_cure_need_check_flag_set((uint8_t)value);
    } else if (us_set_id == USERCTL_BINDCPU) {
        hmac_userctl_bindcpu_get_cmd(value);
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    } else if (us_set_id == MCAST_AMPDU_RETRY) {
        hmac_config_set_mcast_ampdu_retry(pst_mac_vap, value);
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    } else if (us_set_id == SU_PPDU_1XLTF_08US_GI_SWITCH) {
        mac_mib_set_HESUPPDUwith1xHELTFand0point8GIlmplemented(pst_mac_vap, !!value);
#endif
    }
#ifdef _PRE_WLAN_FEATURE_HIEX
    if (g_wlan_spec_cfg->feature_hiex_is_open) {
        pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
        if (oal_unlikely((oal_any_null_ptr1(pst_mac_device)))) {
            oam_warning_log0(0, OAM_SF_HIEX, "{hmac_config_common_debug_part1::hiex pst_mac_device is null.}");
            return;
        }
        if (us_set_id == HIEX_DEV_CAP) {
            mac_hiex_nego_cap(MAC_BAND_GET_HIEX_CAP(pst_mac_device), (mac_hiex_cap_stru *)(&value), NULL);
        }
        if (us_set_id == HIEX_DEBUG) {
            pst_mac_device->hiex_debug_switch = value;
        }
    }
#endif
}


OAL_STATIC uint32_t hmac_config_common_debug(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint8_t uc_cmd_cnt;
    uint8_t uc_cmd_index = 0;
    uint16_t us_set_id;
    uint32_t value;
    mac_cfg_set_str_stru *pst_set_cmd = NULL;

    pst_set_cmd = (mac_cfg_set_str_stru *)puc_param;
    uc_cmd_cnt = pst_set_cmd->uc_cmd_cnt;
    if (uc_cmd_cnt > MAC_STR_CMD_MAX_SIZE) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_common_debug::cnt err[%d].}", uc_cmd_cnt);
        return OAL_ERR_CODE_PTR_NULL;
    }

    while (uc_cmd_cnt > 0) {
        us_set_id = pst_set_cmd->us_set_id[uc_cmd_index];
        value = pst_set_cmd->value[uc_cmd_index];

#ifdef _PRE_WLAN_FEATURE_MBO
        if (us_set_id == MBO_SWITCH) {
            pst_mac_vap->st_mbo_para_info.uc_mbo_enable = (uint8_t)value;
        }
        if (us_set_id == MBO_CELL_CAP) {
            pst_mac_vap->st_mbo_para_info.uc_mbo_cell_capa = (uint8_t)value;
        }
        if (us_set_id == MBO_ASSOC_DISALLOWED_SWITCH) {
            pst_mac_vap->st_mbo_para_info.uc_mbo_assoc_disallowed_test_switch = (uint8_t)value;
        }
#endif

        hmac_config_common_debug_part1(pst_mac_vap, us_set_id, value);
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_common_debug:set id[%d] set data[%d].}",
                         us_set_id, value);
        uc_cmd_index++;
        uc_cmd_cnt--;
    }

    return OAL_SUCC;
}

OAL_STATIC void hmac_config_log_debug(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint8_t uc_cmd_cnt;
    uint8_t uc_cmd_index = 0;
    uint16_t us_set_id;
    uint32_t value;
    mac_cfg_set_str_stru *pst_set_cmd = NULL;
#ifdef _PRE_WLAN_FEATURE_MONITOR
    mac_device_stru *pst_mac_device = NULL;
#endif

    pst_set_cmd = (mac_cfg_set_str_stru *)puc_param;
    uc_cmd_cnt = pst_set_cmd->uc_cmd_cnt;
    if (uc_cmd_cnt > MAC_STR_CMD_MAX_SIZE) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_log_debug::cnt err[%d].}", uc_cmd_cnt);
        return;
    }

    while (uc_cmd_cnt > 0) {
        us_set_id = pst_set_cmd->us_set_id[uc_cmd_index];
        value = pst_set_cmd->value[uc_cmd_index];
        if (us_set_id == MAC_LOG_MONITOR_OTA_RPT) {
#ifdef _PRE_WLAN_FEATURE_MONITOR
            pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
            if (oal_unlikely(pst_mac_device == NULL)) {
                oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_log_debug::mac_device null.}");
                return;
            }
            pst_mac_device->uc_monitor_ota_mode = (uint8_t)value;
#endif
        } else if (us_set_id == MAC_LOG_SET_HIMIT_HTC) {
#ifdef _PRE_WLAN_FEATURE_HIEX
            pst_mac_vap->uc_himit_set_htc = (uint8_t)value;
#endif
        }
        uc_cmd_index++;
        uc_cmd_cnt--;
    }
}


uint32_t hmac_config_vendor_cmd_get_channel_list(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    uint8_t uc_chan_idx;
    uint8_t uc_chan_num;
    uint8_t uc_chan_number;
    uint8_t *puc_channel_list = NULL;
    mac_vendor_cmd_channel_list_stru *pst_channel_list = NULL;
    uint32_t ret;

    if (oal_any_null_ptr2(pus_len, puc_param)) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_config_vendor_cmd_get_channel_list::len or param is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_channel_list = (mac_vendor_cmd_channel_list_stru *)puc_param;
    *pus_len = sizeof(mac_vendor_cmd_channel_list_stru);

    /* 获取2G 信道列表 */
    uc_chan_num = 0;
    puc_channel_list = pst_channel_list->auc_channel_list_2g;

    for (uc_chan_idx = 0; uc_chan_idx < MAC_CHANNEL_FREQ_2_BUTT; uc_chan_idx++) {
        ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_2, uc_chan_idx, OAL_FALSE);
        if (ret == OAL_SUCC) {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_2, uc_chan_idx, OAL_FALSE, &uc_chan_number);
            puc_channel_list[uc_chan_num++] = uc_chan_number;
        }
    }
    pst_channel_list->uc_channel_num_2g = uc_chan_num;

    /* 检查定制化5g开关是否使能 */
    if (mac_device_band_is_support(pst_mac_vap->uc_device_id, MAC_DEVICE_CAP_5G) == OAL_FALSE) {
        pst_channel_list->uc_channel_num_5g = 0;
        return OAL_SUCC;
    }

    /* 获取5G 信道列表 */
    uc_chan_num = 0;
    puc_channel_list = pst_channel_list->auc_channel_list_5g;

    for (uc_chan_idx = 0; uc_chan_idx < MAC_CHANNEL_FREQ_5_BUTT; uc_chan_idx++) {
        ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, uc_chan_idx, OAL_FALSE);
        if (ret == OAL_SUCC) {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_5, uc_chan_idx, OAL_FALSE, &uc_chan_number);
            puc_channel_list[uc_chan_num++] = uc_chan_number;
        }
    }
    pst_channel_list->uc_channel_num_5g = uc_chan_num;

    return OAL_SUCC;
}


uint32_t hmac_config_dbdc_debug_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    hmac_device_stru *pst_hmac_device;
    mac_dbdc_debug_switch_stru *pst_dbdc_debug_switch;

    pst_dbdc_debug_switch = (mac_dbdc_debug_switch_stru *)puc_param;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_DBDC,
                         "{hmac_config_dbdc_debug_switch::hmac device[%d] is null.", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DBDC_DEBUG_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_DBDC,
                         "{hmac_config_dbdc_debug_switch::hmac_config_send_event fail[%d].", ret);
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC void hmac_config_set_dada_collect_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len,
    uint8_t *puc_param)
{
    g_st_data_collect_cfg.uc_type = (uint8_t)((mac_cfg_set_tlv_stru *)puc_param)->uc_cmd_type;
    g_st_data_collect_cfg.reg_num = (uint8_t)((mac_cfg_set_tlv_stru *)puc_param)->value;
    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{set_dada_collect::type:%d,reg num %d.}",
        g_st_data_collect_cfg.uc_type, g_st_data_collect_cfg.reg_num);
}

OAL_STATIC void hmac_config_set_tlv_cmd_2(mac_vap_stru *pst_mac_vap, uint16_t us_len,
    uint8_t *puc_param)
{
    mac_cfg_set_tlv_stru *pst_config_para = (mac_cfg_set_tlv_stru *)puc_param;

    /* HOST需要处理的事件 */
    switch (pst_config_para->us_cfg_id) {
#ifdef _PRE_WLAN_FEATURE_M2S
        case WLAN_CFGID_MIMO_BLACKLIST:
            g_en_mimo_blacklist = (uint8_t)((mac_cfg_set_tlv_stru *)puc_param)->value;
            break;
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
        case WLAN_CFGID_HID2D_DEBUG_MODE:
            g_en_hid2d_debug = (uint8_t)((mac_cfg_set_tlv_stru *)puc_param)->value;
            break;
#endif
#if defined(_PRE_WLAN_FEATURE_HID2D) && defined(_PRE_WLAN_FEATURE_HID2D_PRESENTATION)
        case WLAN_CFGID_HID2D_PRESENTATION_MODE:
            hmac_config_set_hid2d_presentation_mode(pst_mac_vap, us_len, puc_param);
            break;
#endif
        case WLAN_CFGID_DATA_COLLECT:
            hmac_config_set_dada_collect_switch(pst_mac_vap, us_len, puc_param);
            break;
        case WLAN_CFGID_SET_ADC_DAC_FREQ:
        case WLAN_CFGID_SET_MAC_FREQ:
        case WLAN_CFGID_SET_WARNING_MODE:
        case WLAN_CFGID_SET_CHR_MODE:
#ifdef _PRE_WLAN_FEATURE_DFS_ENABLE
        case WLAN_CFGID_SET_DFS_MODE:
#endif
            break;
        default:
            break;
    }
}

uint32_t hmac_config_set_tlv_cmd(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_cfg_set_tlv_stru *pst_config_para = (mac_cfg_set_tlv_stru *)puc_param;

    /* HOST需要处理的事件 */
    switch (pst_config_para->us_cfg_id) {
        case WLAN_CFGID_SET_DEVICE_PKT_STAT:
            hmac_config_set_device_pkt_stat(pst_mac_vap, us_len, puc_param);
            return OAL_SUCC;

        case WLAN_CFGID_SET_TX_AMPDU_TYPE:
            hmac_config_set_tx_ampdu_type(pst_mac_vap, us_len, puc_param);
            return OAL_SUCC;

        case WLAN_CFGID_SET_DEVICE_FREQ:
            hmac_config_set_auto_freq_enable(pst_mac_vap, us_len, puc_param);
            return OAL_SUCC;

        case WLAN_CFGID_AMSDU_AMPDU_SWITCH:
            hmac_config_amsdu_ampdu_switch(pst_mac_vap, us_len, puc_param);
            return OAL_SUCC;

        case WLAN_CFGID_SET_RX_AMPDU_AMSDU:
            hmac_config_rx_ampdu_amsdu(pst_mac_vap, us_len, puc_param);
            /* 1103 mpw2 由于存在译码错误导致去使能amsdu默认打开可能导致死机 默认是关闭,不再开启 */
            return OAL_SUCC;
        case WLAN_CFGID_SET_SK_PACING_SHIFT:
            g_sk_pacing_shift = (uint8_t)((mac_cfg_set_tlv_stru *)puc_param)->value;
            return OAL_SUCC;
        case WLAN_CFGID_SET_TRX_STAT_LOG:
            g_freq_lock_control.uc_trx_stat_log_en = (uint8_t)((mac_cfg_set_tlv_stru *)puc_param)->value;
            return OAL_SUCC;
        case WLAN_CFGID_SET_ADDBA_RSP_BUFFER:
            hmac_config_set_addba_rsp_buffer(pst_mac_vap, us_len, puc_param);
            return OAL_SUCC;
        default:
            break;
    }
    hmac_config_set_tlv_cmd_2(pst_mac_vap, us_len, puc_param);
    /* DEVICE需要处理的事件 */
    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, pst_config_para->us_cfg_id, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_tlv_cmd::send msg failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_set_str_cmd(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_cfg_set_str_stru *pst_config_para;

    pst_config_para = (mac_cfg_set_str_stru *)puc_param;

    /* HOST需要处理的事件 */
    switch (pst_config_para->us_cfg_id) {
        case WLAN_CFGID_COMMON_DEBUG:
            hmac_config_common_debug(pst_mac_vap, us_len, puc_param);
            break;

#ifdef _PRE_WLAN_FEATURE_11AX
        case WLAN_CFGID_11AX_DEBUG:
            if (g_wlan_spec_cfg->feature_11ax_is_open) {
                hmac_config_ax_debug(pst_mac_vap, us_len, puc_param);
                break;
            }
            hmac_config_log_debug(pst_mac_vap, us_len, puc_param);
            break;
#endif
        case WLAN_CFGID_LOG_DEBUG:
            hmac_config_log_debug(pst_mac_vap, us_len, puc_param);
            break;
        default:
            break;
    }

    /* DEVICE需要处理的事件 */
    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, pst_config_para->us_cfg_id, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_str_cmd::send msg failed[%d].}", ret);
    }

    return ret;
}


uint32_t hmac_config_pm_debug_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PM_DEBUG_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_DBDC,
                         "{hmac_config_pm_debug_switch::hmac_config_send_event fail[%d].", ret);
        return ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_APF

void hmac_print_apf_program(uint8_t *puc_program, uint32_t program_len)
{
    uint32_t idx, string_len;
    int32_t l_string_tmp_len;
    uint8_t *pc_print_buff;

    pc_print_buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (pc_print_buff == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_print_apf_program::pc_print_buff null.}");
        return;
    }
    memset_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
    string_len = 0;
    l_string_tmp_len = snprintf_s(pc_print_buff + string_len,
                                  (OAM_REPORT_MAX_STRING_LEN - string_len) - 1,
                                  ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
                                  "Id           :200\n"
                                  "Program len  :%d\n",
                                  program_len);
    if (l_string_tmp_len < 0) {
        oam_error_log1(0, OAM_SF_CFG, "hmac_print_apf_program:sprintf return error[%d]", l_string_tmp_len);
        oal_mem_free_m(pc_print_buff, OAL_TRUE);
        return;
    }
    string_len += l_string_tmp_len;

    for (idx = 0; idx < program_len; idx++) {
        l_string_tmp_len = snprintf_s(pc_print_buff + string_len,
                                      ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
                                      ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
                                      "%02x",
                                      puc_program[idx]);
        if (l_string_tmp_len < 0) {
            break;
        }
        string_len += l_string_tmp_len;
    }

    pc_print_buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    oam_print(pc_print_buff);
    oal_mem_free_m(pc_print_buff, OAL_TRUE);
}

uint32_t hmac_config_apf_filter_cmd(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;
    dmac_tx_event_stru *tx_event = NULL;
    frw_event_mem_stru *event_mem = NULL;
    oal_netbuf_stru *cmd_netbuf = NULL;
    frw_event_stru *hmac_to_dmac_ctx_event = NULL;
    mac_apf_filter_cmd_stru *apf_filter_cmd = NULL;
    int32_t l_ret;

    if (oal_unlikely(oal_any_null_ptr2(mac_vap, param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_apf_program::mac_vap or param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    apf_filter_cmd = (mac_apf_filter_cmd_stru *)param;

    /* 申请netbuf */
    cmd_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF,
        sizeof(mac_apf_cmd_type_uint8) + apf_filter_cmd->us_program_len, OAL_NETBUF_PRIORITY_MID);
    if (cmd_netbuf == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_set_apf_program::netbuf alloc null!}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 拷贝命令结构体到netbuf */
    l_ret = memcpy_s(oal_netbuf_data(cmd_netbuf), sizeof(mac_apf_cmd_type_uint8) + apf_filter_cmd->us_program_len,
                     &apf_filter_cmd->en_cmd_type, sizeof(mac_apf_cmd_type_uint8));
    oal_netbuf_put(cmd_netbuf, sizeof(mac_apf_cmd_type_uint8));

    if (apf_filter_cmd->en_cmd_type == APF_SET_FILTER_CMD) {
        /* program内容拷贝到netbuf */
        l_ret += memcpy_s(oal_netbuf_data(cmd_netbuf) + sizeof(mac_apf_cmd_type_uint8),
            apf_filter_cmd->us_program_len, apf_filter_cmd->puc_program, apf_filter_cmd->us_program_len);
        oal_netbuf_put(cmd_netbuf, apf_filter_cmd->us_program_len);
        /* 打印到sdt */
        hmac_print_apf_program(apf_filter_cmd->puc_program, apf_filter_cmd->us_program_len);
    }

    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_apf_filter_cmd::memcpy fail!");
        oal_netbuf_free(cmd_netbuf);
        return OAL_FAIL;
    }

    /***************************************************************************
      抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (event_mem == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_apf_program::event_mem null.}");
        oal_netbuf_free(cmd_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_to_dmac_ctx_event = (frw_event_stru *)event_mem->puc_data;
    frw_event_hdr_init(&(hmac_to_dmac_ctx_event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX,
        DMAC_WLAN_CTX_EVENT_SUB_TYPE_APF_CMD, sizeof(dmac_tx_event_stru), FRW_EVENT_PIPELINE_STAGE_1,
        mac_vap->uc_chip_id, mac_vap->uc_device_id, mac_vap->uc_vap_id);

    tx_event = (dmac_tx_event_stru *)(hmac_to_dmac_ctx_event->auc_event_data);
    tx_event->pst_netbuf = cmd_netbuf;
    tx_event->us_frame_len = oal_netbuf_len(cmd_netbuf);

    ret = frw_event_dispatch_event(event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_apf_program::dispatch_event fail[%d]!}", ret);
    }

    oal_netbuf_free(cmd_netbuf);
    frw_event_free_m(event_mem);

    return ret;
}


uint32_t hmac_apf_program_report_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event;
    frw_event_hdr_stru *pst_event_hdr;
    dmac_apf_report_event_stru *pst_report_event;
    oal_netbuf_stru *pst_netbuf;
    uint8_t *puc_program = NULL;

    /* 获取事件头和事件结构体指针 */
    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);
    pst_report_event = (dmac_apf_report_event_stru *)pst_event->auc_event_data;

    pst_netbuf = (oal_netbuf_stru *)pst_report_event->p_program;
    if (!pst_netbuf) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_apf_program_report_event:netbuf is null");
        return OAL_FAIL;
    }
    puc_program = oal_netbuf_data(pst_netbuf);
    hmac_print_apf_program(oal_netbuf_data(pst_netbuf), oal_netbuf_len(pst_netbuf));

    oal_netbuf_free(pst_netbuf);
    return OAL_SUCC;
}

#endif
uint32_t hmac_config_force_stop_filter(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_rx_filter_force_switch::pst_mac_vap or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return hmac_config_sync_cmd_common(pst_mac_vap, WLAN_CFGID_FORCE_STOP_FILTER, us_len, puc_param);
}


uint32_t hmac_config_set_owe(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    uint8_t uc_owe_group;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_owe::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_owe::not AP mode!}");
        return OAL_SUCC;
    }

    uc_owe_group = *puc_param;
    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_owe::AP mode support owe group=0x%02x}", uc_owe_group);

    pst_hmac_vap->owe_group_cap = uc_owe_group;

    /* MAC_OWE_GROUP_19 */
    pst_hmac_vap->owe_group[0] = (uc_owe_group & WAL_HIPRIV_OWE_19) ? MAC_OWE_GROUP_19 : 0;
    /* MAC_OWE_GROUP_20 */
    pst_hmac_vap->owe_group[1] = (uc_owe_group & WAL_HIPRIV_OWE_20) ? MAC_OWE_GROUP_20 : 0;
    /* MAC_OWE_GROUP_21 */
    pst_hmac_vap->owe_group[2] = (uc_owe_group & WAL_HIPRIV_OWE_21) ? MAC_OWE_GROUP_21 : 0;

    return OAL_SUCC;
}


uint32_t hmac_config_fem_lp_flag(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
       ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SPEC_FEMLP_EN, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_fem_lp_flag::hmac_config_fem_lp_flag fail[%d].", ret);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_config_softap_mimo_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
       ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SOFTAP_MIMO_MODE, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_softap_mimo_mode::config_softap_mimo_mode fail[%d].", ret);
        return ret;
    }

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_NAN

OAL_STATIC int32_t hmac_nan_tx_mgmt(mac_vap_stru *pst_mac_vap, mac_nan_mgmt_info_stru *mgmt_info, uint16_t transaction)
{
#define NAN_TAIL_INFO_LEN 3
    oal_netbuf_stru *netbuf_mgmt_tx = NULL;
    mac_tx_ctl_stru *tx_ctl = NULL;
    uint8_t *tail;
    uint32_t ret;
    if (mgmt_info->data == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_nan_tx_mgmt::tx data ptr is null}");
        return OAL_FAIL;
    }
    if (mgmt_info->len + NAN_TAIL_INFO_LEN + sizeof(dmac_ctx_action_event_stru) > WLAN_MEM_NETBUF_SIZE2) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_nan_tx_mgmt::len[%d] invalid}", mgmt_info->len);
        return OAL_FAIL;
    }

    /*  申请netbuf */
    netbuf_mgmt_tx = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (netbuf_mgmt_tx == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_nan_tx_mgmt::pst_mgmt_tx null.}");
        return OAL_FAIL;
    }

    /* 填充netbuf */
    if (memcpy_s((uint8_t *)oal_netbuf_header(netbuf_mgmt_tx), WLAN_MEM_NETBUF_SIZE2,
        mgmt_info->data, mgmt_info->len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_nan_tx_mgmt::memcpy fail!");
        oal_netbuf_free(netbuf_mgmt_tx);
        return OAL_FAIL;
    }

    /* 帧体尾部增加3字节信息字段用来标记是否周期性发送和transaction id */
    oal_netbuf_put(netbuf_mgmt_tx,  mgmt_info->len + NAN_TAIL_INFO_LEN + sizeof(dmac_ctx_action_event_stru));
    tail = oal_netbuf_header(netbuf_mgmt_tx) + mgmt_info->len;
    *tail = (mgmt_info->periodic == 0); /* 帧体尾部字节0存放是否周期性发送 */
    tail[1] = transaction & 0xff; /* 帧体尾部字节1和字节2存放transaction id */
    tail[2] = (transaction >> 8) & 0xff; /* 帧体尾部字节1和字节2存放transaction id, 右移8位获取高位 */

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf_mgmt_tx);
    memset_s(tx_ctl, sizeof(mac_tx_ctl_stru), 0, sizeof(mac_tx_ctl_stru));
    /* mpdu长度加上尾部3字节信息 */
    MAC_GET_CB_MPDU_LEN(tx_ctl) = mgmt_info->len + NAN_TAIL_INFO_LEN + sizeof(dmac_ctx_action_event_stru);
    MAC_GET_CB_TX_USER_IDX(tx_ctl) = g_wlan_spec_cfg->invalid_user_id; /* NAN发送给未关联的设备 */
    MAC_GET_CB_FRAME_TYPE(tx_ctl) = WLAN_CB_FRAME_TYPE_ACTION;
    MAC_GET_CB_WME_AC_TYPE(tx_ctl) = WLAN_WME_AC_MGMT;
    MAC_GET_CB_FRAME_SUBTYPE(tx_ctl) = mgmt_info->action;
    ret = hmac_tx_mgmt_send_event(pst_mac_vap, netbuf_mgmt_tx, mgmt_info->len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(netbuf_mgmt_tx);
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_nan_tx_mgmt::hmac_tx_mgmt_send_event failed[%d].}", ret);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

uint32_t hmac_config_nan(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    mac_nan_cfg_msg_stru *nan_msg = (mac_nan_cfg_msg_stru*)param;
    uint32_t ret;
    /* 目前限制sta模式下支持NAN */
    if (!IS_LEGACY_STA(mac_vap)) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_nan::Not STA mode}");
        return OAL_SUCC;
    }

    switch (nan_msg->type) {
        case NAN_CFG_TYPE_SET_PARAM:
            ret = hmac_config_sync_cmd_common(mac_vap, WLAN_CFGID_NAN, len, param);
            break;
        case NAN_CFG_TYPE_SET_TX_MGMT:
            ret = hmac_nan_tx_mgmt(mac_vap, &nan_msg->mgmt_info, nan_msg->transaction_id);
            break;
        default:
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_nan::invalid nan cfg type[%d]}", nan_msg->type);
            ret = OAL_SUCC;
            break;
    }

    return ret;
}
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
uint32_t hmac_config_set_hid2d_acs_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len,
    uint8_t *puc_param)
{
    uint8_t uc_acs_mode;
    uc_acs_mode = *puc_param;

    /* 设置HiD2D acs mode */
    hmac_hid2d_set_acs_mode(pst_mac_vap, uc_acs_mode);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_HID2D_PRESENTATION

uint32_t hmac_config_set_hid2d_presentation_mode(mac_vap_stru *pst_mac_vap,
    uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_set_tlv_stru *pst_config_para = (mac_cfg_set_tlv_stru *)puc_param;
    mac_device_stru *pst_device = NULL;

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if ((pst_device != NULL) && (pst_device->pst_wiphy != NULL)) {
        pst_device->is_presentation_mode = pst_config_para->value;
        oam_warning_log1(0, OAM_SF_ANY, "{HiD2D Presentation::set HiD2D Presentation mode[%d].}",
            pst_device->is_presentation_mode);
    }

    return OAL_SUCC;
}

uint32_t hmac_config_hid2d_switch_channel(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_csa_debug_stru *csa_cfg = NULL;
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *p2p_mac_vap = NULL;
    uint8_t vap_index;
    uint8_t no_p2p_vap_flag = OAL_TRUE;

    /* 找到mac_device */
    mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (mac_device == NULL) {
        return OAL_SUCC;
    }

    /* 找到挂接在该device上的p2p vap */
    for (vap_index = 0; vap_index < mac_device->uc_vap_num; vap_index++) {
        p2p_mac_vap = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_index]);
        if (p2p_mac_vap == NULL) {
            continue;
        }
        if (p2p_mac_vap->en_p2p_mode == WLAN_P2P_GO_MODE && p2p_mac_vap->en_vap_state == MAC_VAP_STATE_UP) {
            no_p2p_vap_flag = OAL_FALSE;
            break;
        }
    }
    if (no_p2p_vap_flag == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ANTI_INTF, "{HiD2D Presentation::no p2p vap error!}");
        return OAL_FAIL;
    }

    csa_cfg = (mac_csa_debug_stru *)puc_param;
    mac_device->uc_csa_vap_cnt = 1;
    p2p_mac_vap->st_ch_switch_info.en_csa_mode = WLAN_CSA_MODE_TX_DISABLE;
    p2p_mac_vap->st_ch_switch_info.uc_ch_switch_cnt = HMAC_CHANNEL_SWITCH_COUNT; /* CSA cnt 设置为5 */
    hmac_chan_initiate_switch_to_new_channel(p2p_mac_vap, csa_cfg->uc_channel, csa_cfg->en_bandwidth);
    return OAL_SUCC;
}
#endif
#endif
uint32_t hmac_send_custom_data(mac_vap_stru *pst_mac_vap, uint16_t len, uint8_t *param, uint8_t syn_id)
{
    oal_netbuf_stru *netbuf = NULL;
    uint32_t ret;
    frw_event_mem_stru       *event_mem = NULL;
    frw_event_stru           *event = NULL;
    dmac_tx_event_stru       *h2d_custom_event = NULL;
    int32_t sec_ret;

    /*  申请netbuf */
    netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_LARGE_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (netbuf == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_send_custom_data::netbuf alloc failed");
        return OAL_FAIL;
    }
    /* 填充netbuf sys id(1字节) + 定制化数据 */
    sec_ret = memcpy_s(oal_netbuf_data(netbuf), WLAN_LARGE_NETBUF_SIZE, &syn_id, sizeof(syn_id));
    sec_ret += memcpy_s(oal_netbuf_data(netbuf) + sizeof(syn_id),
        (WLAN_LARGE_NETBUF_SIZE - sizeof(syn_id)), param, len);
    if (sec_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_send_custom_data::memcpy fail!");
        oal_netbuf_free(netbuf);
        return OAL_FAIL;
    }
    event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (event_mem == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_send_custom_data::pst_event_mem null.}");
        oal_netbuf_free(netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }
    event = frw_get_event_stru(event_mem);
    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_CUST_HMAC2DMAC,
        sizeof(dmac_tx_event_stru), FRW_EVENT_PIPELINE_STAGE_1, pst_mac_vap->uc_chip_id, pst_mac_vap->uc_device_id,
        pst_mac_vap->uc_vap_id);
    h2d_custom_event               = (dmac_tx_event_stru *)event->auc_event_data;
    h2d_custom_event->pst_netbuf   = netbuf;
    h2d_custom_event->us_frame_len = len + sizeof(syn_id);

    ret = frw_event_dispatch_event(event_mem);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log3(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{hmac_send_cali_data::dispatch event fail ret[%d], len[%d] syn_id[%d].}", ret, len, syn_id);
        oal_netbuf_free(netbuf);
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }

    oal_netbuf_free(netbuf);
    frw_event_free_m(event_mem);
    return OAL_SUCC;
}
#ifdef RND_HUAWEI_LOW_LATENCY_SWITCHING
OAL_STATIC void hmac_config_freq_boost(bool enable)
{
    mac_cfg_set_tlv_stru tlv_cmd;
    if (enable) {
        /* disable auto-switching first */
        tlv_cmd.uc_cmd_type = CMD_SET_AUTO_FREQ_ENDABLE;
        tlv_cmd.value    = FREQ_LOCK_DISABLE;
        hmac_config_set_auto_freq_enable(NULL, 0, (uint8_t *)&tlv_cmd);

        /* then set the highest chip frequency */
        tlv_cmd.uc_cmd_type = CMD_SET_DEVICE_FREQ_VALUE;
        tlv_cmd.value    = FREQ_HIGHEST;
        hmac_config_set_auto_freq_enable(NULL, 0, (uint8_t *)&tlv_cmd);
    } else {
        /* enable auto-switching */
        tlv_cmd.uc_cmd_type = CMD_SET_AUTO_FREQ_ENDABLE;
        tlv_cmd.value    = FREQ_LOCK_ENABLE;
        hmac_config_set_auto_freq_enable(NULL, 0, (uint8_t *)&tlv_cmd);
    }
}

void hmac_low_latency_freq_high(void)
{
    hmac_config_freq_boost(OAL_TRUE);
}

void hmac_low_latency_freq_default(void)
{
    hmac_config_freq_boost(OAL_FALSE);
}
#endif
