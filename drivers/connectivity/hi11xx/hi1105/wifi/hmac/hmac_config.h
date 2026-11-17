

#ifndef __HMAC_CONFIG_H__
#define __HMAC_CONFIG_H__

#include "oal_ext_if.h"
#include "mac_vap.h"
#include "mac_device.h"
#include "hmac_device.h"
#include "hmac_11k.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_H

#define SHORTGI_CFG_STRU_LEN 4

#define HMAC_MAX_MCS_NUM               8 /* 单流和双流支持的mac最大个数 */
#define HAMC_QUERY_INFO_FROM_DMAC_TIME (5 * OAL_TIME_HZ)
#ifdef _PRE_EMU
#define HMAC_WAIT_EVENT_RSP_TIME 100 /* 等待配置命令事件完成的超时时间(s) */
#else
#define HMAC_WAIT_EVENT_RSP_TIME 10 /* 等待配置命令事件完成的超时时间(s) */
#endif
#define HMAC_S_TO_MS 1000   /* 秒和毫秒的转换系数 */
#define HMAC_MS_TO_US 1000   /* 毫秒和微秒的转换系数 */
#define HMAC_S_TO_US 1000000   /* 秒和微秒的转换系数 */
#define HMAC_US_TO_NS 1000   /* 微秒和纳秒的转换系数 */

/* APUT OWE group definition, hipriv.sh BIT format transit to pst_hmac_vap->owe_group */
#define WAL_HIPRIV_OWE_19 BIT(0)
#define WAL_HIPRIV_OWE_20 BIT(1)
#define WAL_HIPRIV_OWE_21 BIT(2)

/* hmac_vap结构中，一部分成员的大小，这些成员在linux和windows下的定义可能不同 */
typedef struct {
    uint32_t hmac_vap_cfg_priv_stru_size;
    uint32_t frw_timeout_stru_size;
    uint32_t oal_spin_lock_stru_size;
    uint32_t mac_key_mgmt_stru_size;
    uint32_t mac_pmkid_cache_stru_size;
    uint32_t mac_curr_rateset_stru_size;
    uint32_t hmac_vap_stru_size;
} hmac_vap_member_size_stru;

/* HMAC到DMAC配置同步操作 */
typedef struct {
    wlan_cfgid_enum_uint16 en_cfgid;
    uint8_t auc_resv[NUM_2_BYTES];
    uint32_t (*p_set_func)(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param);
} hmac_config_syn_stru;

enum {
    PKT_STAT_SET_ENABLE = 0,
    PKT_STAT_SET_START_STAT = 1,
    PKT_STAT_SET_FRAME_LEN = 2,
    PKT_STAT_SET_BUTT
};

typedef struct {
    uint8_t uc_channel;  /* ap所在信道编号，eg 1,2,11,36,40... */
    uint8_t uc_ssid_len; /* SSID 长度 */
    mac_nl80211_mfp_enum_uint8 en_mfp;
    uint8_t uc_wapi;

    uint8_t *puc_ie;
    uint8_t auc_ssid[NUM_32_BITS];       /* 期望关联的AP SSID  */
    uint8_t auc_bssid[OAL_MAC_ADDR_LEN]; /* 期望关联的AP BSSID  */

    oal_bool_enum_uint8 en_privacy;                /* 是否加密标志 */
    oal_nl80211_auth_type_enum_uint8 en_auth_type; /* 认证类型，OPEN or SHARE-KEY */

    uint8_t uc_wep_key_len;           /* WEP KEY长度 */
    uint8_t uc_wep_key_index;         /* WEP KEY索引 */
    uint8_t *puc_wep_key;             /* WEP KEY密钥 */
    mac_crypto_settings_stru st_crypto; /* 密钥套件信息 */
    uint32_t ie_len;
} hmac_conn_param_stru;

uint32_t hmac_config_start_vap_event(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_mgmt_rate_init_flag);
uint32_t hmac_set_mode_event(mac_vap_stru *pst_mac_vap);
uint32_t hmac_config_sta_update_rates(mac_vap_stru *pst_mac_vap,
                                      mac_cfg_mode_param_stru *pst_cfg_mode,
                                      mac_bss_dscr_stru *pst_bss_dscr);
uint32_t hmac_event_config_syn(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_event_log_syn(frw_event_mem_stru *pst_event_mem);

uint32_t hmac_protection_update_from_user(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_40M_intol_sync_event(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_tlv_cmd(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_str_cmd(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_tx_ampdu_type(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_sta_pm_on_syn(mac_vap_stru *pst_mac_vap);
uint32_t hmac_set_ipaddr_timeout(void *puc_para);
uint32_t hmac_config_pm_debug_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_ip_addr(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_roam_start(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);

uint32_t hmac_config_enable_2040bss(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_2040bss_sw(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_get_dieid_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param);
uint32_t hmac_config_get_dieid(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_auto_freq_enable(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);

uint32_t hmac_config_set_sta_pm_on(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_sta_pm_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);

#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
uint32_t hmac_config_set_rx_listen_ps_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif

uint32_t hmac_config_set_all_log_level(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_cus_rf(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_cus_dts_cali(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_cus_nvram_params(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_cus_dyn_cali(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_config_set_rf_front_para(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_config_set_rf_cali_para(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_config_set_dev_cap_para(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_config_set_nv_pow_para(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_config_set_nv_dyn_pow_para(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);

uint32_t hmac_config_stop_altx(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param);

#ifdef _PRE_WLAN_FEATURE_FTM
uint32_t  hmac_config_ftm_dbg_1103(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_ftm_check_dbg_params(mac_send_iftmr_stru* send_iftmr);
uint32_t hmac_ftm_send_gas_init_req(mac_vap_stru *pst_mac_vap, mac_send_gas_init_req_stru *pst_gas_req);
void hmac_config_ftm_dbg_set_bw_and_preamble(mac_ftm_debug_switch_stru* ftm_debug);
uint32_t  hmac_ftm_rx_gas_initial_response_frame(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf);
wlan_channel_bandwidth_enum_uint8 hmac_ftm_get_channel_bandwidth(int32_t center_freq0, uint8_t channel_num,
    uint32_t channel_width);
uint32_t hmac_config_wifi_rtt_config_1103(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
#endif
#ifdef _PRE_WLAN_FEATURE_APF
uint32_t hmac_config_apf_filter_cmd(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
uint32_t hmac_config_force_stop_filter(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);

uint32_t hmac_config_fem_lp_flag(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_softap_mimo_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);

uint32_t hmac_config_set_owe(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);

#ifdef _PRE_CONFIG_HISIFASTPOWER_DOWN_STATE
void hmac_config_change_wifi_state_for_power_down(void);
#endif

#ifdef _PRE_WLAN_FEATURE_NAN
uint32_t hmac_config_nan(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
uint32_t hmac_config_set_hid2d_acs_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_hid2d_switch_channel(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_hid2d_presentation_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
uint32_t hmac_send_custom_data(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param, uint8_t syn_id);
uint32_t hmac_config_send_event(mac_vap_stru *pst_mac_vap, wlan_cfgid_enum_uint16 en_cfg_id,
                                uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_alg_send_event(mac_vap_stru *mac_vap, wlan_cfgid_enum_uint16 cfg_id,
    uint16_t len, uint8_t *param);
#ifdef RND_HUAWEI_LOW_LATENCY_SWITCHING
void hmac_low_latency_freq_high(void);
void hmac_low_latency_freq_default(void);
#endif

uint32_t hmac_config_add_vap(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_del_vap(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_start_vap(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_down_vap(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_mac_addr(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_wmmswitch(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_get_vap_wmm_switch(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_set_vap_wmm_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_max_user(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);

#ifdef _PRE_WLAN_FEATURE_M2S
#ifdef _PRE_WLAN_FEATURE_M2S_MSS
uint32_t hmac_config_set_m2s_switch_blacklist(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_m2s_switch_mss(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
uint32_t hmac_config_mimo_compatibility(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param);
#ifdef _PRE_WLAN_FEATURE_M2S_MODEM
uint32_t hmac_config_set_m2s_switch_modem(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
#endif
uint32_t hmac_config_get_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_set_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_bss_type(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_bss_type(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_set_ssid(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_ssid(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_set_shpreamble(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_shpreamble(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_set_shortgi20(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_shortgi20(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_set_shortgi40(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_shortgi40(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_set_shortgi80(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_shortgi80(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
#ifdef _PRE_WLAN_FEATURE_MONITOR
uint32_t hmac_config_set_sniffer(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_sniffer_1103(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_config_set_sniffer_1106(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_config_set_monitor_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_monitor(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
#endif
uint32_t hmac_config_set_prot_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_prot_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_set_auth_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_auth_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_set_max_user(mac_vap_stru *mac_vap, uint32_t max_user_num);
uint32_t hmac_config_set_bintval(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_bintval(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_set_nobeacon(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_nobeacon(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_set_txpower(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
void hmac_config_ota_switch(oam_ota_switch_param_stru *ota_swicth);
uint32_t hmac_config_ota_rx_dscr_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_random_mac_addr_scan(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_random_mac_oui(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_txpower(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_set_freq(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
uint32_t hmac_hid2d_drop_report(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param);
#endif
uint32_t hmac_config_get_freq(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_set_wmm_params(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_wmm_params(mac_vap_stru *pst_mac_vap, uint8_t *puc_param);
uint32_t hmac_config_vap_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_eth_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_80211_ucast_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_mgmt_log(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_start);
uint32_t hmac_config_protocol_debug_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_phy_debug_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_lte_gpio_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_log_level(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_dtimperiod(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_dtimperiod(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_user_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#ifdef _PRE_WLAN_CHIP_FPGA_PCIE_TEST
uint32_t hmac_config_pcie_test(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
uint32_t hmac_config_addba_req(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_delba_req(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_tx_pow_param(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_dscr_param(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_rate(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_mcs(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_mcsac(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#ifdef _PRE_WLAN_FEATURE_11AX
uint32_t hmac_config_set_mcsax(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
uint32_t hmac_config_set_mcsax_er(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
#endif
uint32_t hmac_config_set_nss(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_rfch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_bw(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_band(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_config_always_tx(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_always_rx(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_ru_index(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
#ifdef PLATFORM_DEBUG_ENABLE
uint32_t hmac_config_reg_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
uint32_t hmac_config_sdio_flowctrl(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#ifdef _PRE_WLAN_DELAY_STATISTIC
uint32_t hmac_config_pkt_time_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
uint32_t hmac_config_list_sta(mac_vap_stru *pst_mac_vap);
uint32_t hmac_config_get_sta_list(mac_vap_stru *pst_mac_vap, uint16_t *us_len, uint8_t *puc_param);
uint32_t hmac_config_list_channel(mac_vap_stru *pst_mac_vap);
uint32_t hmac_config_kick_user(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_update_ip_filter(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_probe_resp_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_dpd_cfg(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_send_bar(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#ifdef PLATFORM_DEBUG_ENABLE
uint32_t hmac_config_reg_write(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
uint32_t hmac_config_amsdu_ampdu_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_alg(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#ifdef _PRE_WLAN_FEATURE_NRCOEX
uint32_t hmac_config_nrcoex_test(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_query_nrcoex_stat(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
uint32_t hmac_config_get_uapsden(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_set_reset_state(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_reset_hw(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#ifdef _PRE_WLAN_DFT_STAT
uint32_t hmac_config_usr_queue_stat(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
uint32_t hmac_config_send_frame(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_amsdu_tx_on(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_ampdu_tx_on(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_amsdu_tx_on(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_get_ampdu_tx_on(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_set_country_for_dfs(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_country(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#ifdef _PRE_WLAN_FEATURE_TPC_OPT
uint32_t hmac_config_reduce_sar(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
uint32_t hmac_config_tas_pwr_ctrl(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
uint32_t hmac_config_tas_rssi_access(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
uint32_t hmac_config_get_country(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_connect(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_tid(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_reset_hw(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_mib_by_bw(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_channel(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_beacon(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_wps_p2p_ie(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_wps_ie(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_app_ie_to_vap(mac_vap_stru *pst_mac_vap, oal_app_ie_stru *pst_wps_ie,
    en_app_ie_type_uint8 en_type);
uint32_t hmac_config_alg_param(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_cali_debug(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
uint32_t hmac_11v_sta_tx_query(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
uint32_t hmac_config_set_2040_coext_support(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_rx_fcs_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_sync_cmd_common(mac_vap_stru *pst_mac_vap, wlan_cfgid_enum_uint16 en_cfg_id,
    uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_open_wmm(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);

uint32_t hmac_config_set_pm_by_module(mac_vap_stru *pst_mac_vap, mac_pm_ctrl_type_enum pm_ctrl_type,
    mac_pm_switch_enum pm_enable);
uint32_t hmac_config_send_2040_coext(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_version(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
uint32_t hmac_config_get_user_rssbw(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
uint32_t hmac_config_set_m2s_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
uint32_t hmac_config_ru_set(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_blacklist_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_set_blacklist_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_blacklist_add(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_blacklist_add_only(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_blacklist_del(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_autoblacklist_enable(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_autoblacklist_aging(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_autoblacklist_threshold(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_autoblacklist_reset_time(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);

#ifdef _PRE_WLAN_FEATURE_HIEX
uint32_t hmac_config_set_user_hiex_enable(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
uint32_t hmac_config_set_ampdu_aggr_num(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#ifdef _PRE_WLAN_FEATURE_CSI
uint32_t hmac_csi_config_1103(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_csi_config(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
#endif
uint32_t hmac_config_vap_close_txbf_cap(mac_vap_stru *pst_mac_vap);
uint32_t hmac_config_set_pmksa(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_del_pmksa(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_flush_pmksa(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_scan_abort(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_remain_on_channel(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_cancel_remain_on_channel(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#ifdef _PRE_WLAN_FEATURE_HS20
uint32_t hmac_config_set_qos_map(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
uint32_t hmac_config_set_dc_status(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_p2p_miracast_status(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_p2p_ps_ops(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_p2p_ps_noa(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_p2p_ps_stat(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_vap_classify_en(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_vap_classify_tid(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_query_station_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_query_rssi(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_query_psst(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_query_psm_flt_stat(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_query_rate(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);

#ifdef _PRE_WLAN_DFT_STAT
uint32_t hmac_config_query_ani(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif

uint32_t hmac_config_vap_state_syn(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_ip_addr(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_user_num_spatial_stream_cap_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
uint32_t hmac_config_cfg_vap_h2d(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_user_asoc_state_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
uint32_t hmac_config_user_cap_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
uint32_t hmac_config_user_rate_info_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
uint32_t hmac_config_user_info_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
#ifdef _PRE_WLAN_FEATURE_M2S
uint32_t hmac_config_vap_m2s_info_syn(mac_vap_stru *pst_mac_vap);
#endif
uint32_t hmac_config_sta_vap_info_syn(mac_vap_stru *pst_mac_vap);
uint32_t hmac_init_user_security_port(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
uint32_t hmac_user_set_asoc_state(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user,
    mac_user_asoc_state_enum_uint8 en_value);
uint32_t hmac_config_ch_status_sync(mac_device_stru *pst_mac_dev);
void hmac_config_del_p2p_ie(uint8_t *puc_ie, uint32_t *pul_ie_len);
uint32_t hmac_find_p2p_listen_channel(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);

void hmac_set_device_freq_mode(uint8_t uc_device_freq_type);
uint32_t hmac_config_set_device_freq(uint8_t uc_device_freq_type);

uint32_t hmac_config_dyn_cali_param(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_get_sta_11h_abillty(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_vendor_ie(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_mlme(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#ifdef _PRE_WLAN_FEATURE_11K
uint32_t hmac_config_send_neighbor_req(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
#endif
uint32_t hmac_config_vendor_cmd_get_channel_list(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param);
uint32_t hmac_config_dbdc_debug_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_cali_debug(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_atcmdsrv_get_rx_pkcg(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_main */
