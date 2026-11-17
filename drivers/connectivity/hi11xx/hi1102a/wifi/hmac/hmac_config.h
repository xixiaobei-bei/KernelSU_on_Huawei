

#ifndef __HMAC_CONFIG_H__
#define __HMAC_CONFIG_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "mac_vap.h"
#include "mac_device.h"
#include "hmac_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define WAL_ATCMDSRV_EFUSE_BUFF_LEN        16
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_BTCOEX
extern oal_bool_enum_uint8 g_en_btcoex_reject_addba;
#endif
extern hmac_rxdata_thread_stru g_st_rxdata_thread;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
extern oal_uint8 g_uc_dev_lte_gpio_level;
extern oal_uint16 g_us_efuse_buffer[16];
#endif
extern oal_bool_enum_uint8 g_ht_mcs_set_check;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
extern oal_uint32 g_ul_tx_ba_policy_select;
#endif

/*****************************************************************************
  5 消息头定义
*****************************************************************************/
/*****************************************************************************
  6 消息定义
*****************************************************************************/
/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/* hmac_vap结构中，一部分成员的大小，这些成员在linux和windows下的定义可能不同 */
typedef struct {
    oal_uint32      ul_hmac_vap_cfg_priv_stru_size;
    oal_uint32      ul_frw_timeout_stru_size;
    oal_uint32      ul_oal_spin_lock_stru_size;
    oal_uint32      ul_mac_key_mgmt_stru_size;
    oal_uint32      ul_mac_pmkid_cache_stru_size;
    oal_uint32      ul_mac_curr_rateset_stru_size;
    oal_uint32      ul_hmac_vap_stru_size;
}hmac_vap_member_size_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/
/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
/*****************************************************************************
  10 函数声明
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
extern mac_tx_large_amsdu_ampdu_stru *mac_get_tx_large_amsdu_addr(oal_void);
#endif
extern oal_uint8 wlan_pm_get_switch(void);
extern void wlan_pm_set_switch(oal_uint8 pm_switch);
extern oal_uint8 wlan_pm_get_fast_check_cnt(void);
extern void wlan_pm_set_fast_check_cnt(oal_uint8 fast_check_cnt);
extern oal_uint16 *hmac_get_efuse_buffer_first_addr(oal_void);
extern oal_uint16 hmac_get_efuse_buffer(oal_uint16 us_efuse_buffer_index);
extern oal_void hmac_set_efuse_buffer(oal_uint16 us_efuse_buffer_index, oal_uint16 us_efuse_buffer);
extern oal_uint8 hmac_config_get_ps_mode(void);
extern void hmac_config_set_ps_mode(oal_uint8 ps_mode);
extern oal_uint8 hmac_atcmsrv_get_lte_gpio_level(oal_void);
extern oal_void hmac_atcmsrv_set_lte_gpio_level(oal_uint8 uc_dev_lte_gpio_level);
extern oal_void hmac_rx_filter_init_multi_vap(oal_uint32 ul_proxysta_enabled);
extern oal_uint32 hmac_config_start_vap_event(mac_vap_stru  *pst_mac_vap, oal_bool_enum_uint8 en_mgmt_rate_init_flag);
extern oal_uint32 hmac_set_mode_event(mac_vap_stru *pst_mac_vap);
extern oal_uint32 hmac_config_update_opmode_event(
    mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, oal_uint8 uc_mgmt_frm_type);
extern oal_uint32  hmac_config_sta_update_rates(
    mac_vap_stru *pst_mac_vap, mac_cfg_mode_param_stru *pst_cfg_mode, mac_bss_dscr_stru *pst_bss_dscr);
extern oal_uint32  hmac_event_config_syn(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  hmac_event_log_syn(frw_event_mem_stru *pst_event_mem);

extern oal_uint32  hmac_config_set_protection(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);

extern oal_uint32  hmac_config_alg_send_event(
                        mac_vap_stru                     *pst_mac_vap,
                        wlan_cfgid_enum_uint16            en_cfg_id,
                        oal_uint16                        us_len,
                        oal_uint8                        *puc_param);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
extern oal_uint32 hmac_get_thruput_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_void hcc_msg_slave_thruput_bypass(oal_void);
#ifdef _PRE_WLAN_FEATURE_STA_PM
extern oal_uint32  hmac_config_sta_pm_on_syn(mac_vap_stru *pst_mac_vap);
extern oal_uint32  hmac_set_ipaddr_timeout(void   *puc_para);
#endif
extern oal_uint32 hmac_config_pm_debug_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
extern oal_uint32 hmac_config_set_ip_addr(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
oal_uint32 hmac_config_roam_enable(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_roam_org(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_roam_band(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_roam_start(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_roam_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_roam_start_reconnect(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_target_bssid);
#endif //_PRE_WLAN_FEATURE_ROAM

#ifdef _PRE_WLAN_FEATURE_11R
oal_uint32 hmac_config_set_ft_ies(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif //_PRE_WLAN_FEATURE_11R
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
oal_uint32 hmac_config_set_device_freq_enable(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_device_freq_value(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_STA_PM
oal_uint32  hmac_config_set_sta_pm_on(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_sta_pm_mode(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
oal_uint32 hmac_config_set_linkloss_threshold(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_all_log_level(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_d2h_hcc_assemble_cnt(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_cus_rf(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_cus_dts_cali(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_cus_nvram_params(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_dev_customize_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32  hmac_config_set_cus_base_power_params(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_5g_high_band_max_pow_params(
    mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_cus_fcc_ce_power_params(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_ext_fcc_ce_power_params(
                                           mac_vap_stru *pst_mac_vap,
                                           oal_uint16    us_len,
                                           oal_uint8    *puc_param);

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
extern oal_uint32  hmac_config_set_cus_dyn_cali(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#ifdef _PRE_WLAN_FEATURE_11K
oal_uint32 hmac_scan_rrm_proc_save_bss(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_VOWIFI
extern oal_uint32  hmac_config_vowifi_report(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif /* _PRE_WLAN_FEATURE_VOWIFI */
oal_uint32 hmac_config_mcs_set_check_enable(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);

extern oal_uint32  hmac_config_set_tx_pow_param(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#ifdef _PRE_WLAN_FEATURE_APF
extern oal_uint32  hmac_config_set_apf(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_void hmac_set_apf_switch_by_pps(hmac_device_stru *pst_hmac_device, mac_cfg_suspend_stru *pst_suspend);
#endif
extern oal_uint32 hmac_config_set_2040_coext_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_filter_11v_bsstreq_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_enable_filter);
extern oal_uint32 hmac_config_fft_window_offset(
    mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_fft_window_offset_enable);
extern oal_uint32 hmac_config_wfd_aggr_limit_syn(mac_vap_stru *pst_mac_vap,
    oal_bool_enum_uint8 en_wfd_status, oal_bool_enum_uint8 en_aggr_limit_on);
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
extern oal_uint32 hmac_config_tas_rssi_access(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_NRCOEX
extern oal_uint32 hmac_config_set_nrcoex_params(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_wifi_priority(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_nrcoex_cmd(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_nrcoex_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
extern oal_uint32 hmac_config_add_vap(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_del_vap(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_start_vap(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_down_vap(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_mac_addr(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_mode(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_mode(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_concurrent(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_concurrent(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_ssid(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_ssid(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_ampdu(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_shpreamble(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_shpreamble(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_shortgi20(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_shortgi20(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_shortgi40(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_shortgi40(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_shortgi80(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_shortgi80(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_addr_filter(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len,
                                              oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_prot_mode(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_prot_mode(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_auth_mode(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_auth_mode(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_max_user(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint32 ul_max_user);
extern oal_uint32 hmac_config_set_bintval(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_bintval(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_nobeacon(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_nobeacon(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_txchain(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_txchain(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_rxchain(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_rxchain(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_txpower(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_ota_beacon_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                oal_uint8 *puc_param);
extern oal_uint32 hmac_config_ota_rx_dscr_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                 oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_all_ota(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_oam_output(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_dhcp_arp_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                  oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_random_mac_addr_scan(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                       oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_random_mac_oui(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                 oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_txpower(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_freq(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#ifdef _PRE_BT_FITTING_DATA_COLLECT
extern oal_uint32 hmac_config_init_bt_env(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_bt_freq(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_bt_upc_by_freq(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                 oal_uint8 *puc_param);
extern oal_uint32 hmac_config_print_bt_gm(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
extern oal_uint32 hmac_config_get_freq(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_vap_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_event_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_eth_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_80211_ucast_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                 oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_mgmt_log(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user,
                                           oal_bool_enum_uint8 en_start);
extern oal_uint32 hmac_config_80211_mcast_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                 oal_uint8 *puc_param);
extern oal_uint32 hmac_config_probe_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_phy_debug_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
extern oal_uint32 hmac_config_wfa_cfg_aifsn(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_wfa_cfg_cw(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_lte_gpio_mode(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_VOWIFI_NAT
extern oal_uint32 hmac_config_set_vowifi_nat_keep_alive_params(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                               oal_uint8 *puc_param);
#endif
extern oal_uint32 hmac_config_set_power_rf_ctl(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_log_level(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_dtimperiod(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_dtimperiod(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_user_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_add_user(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_del_user(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_auto_ba_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_addba_req(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_delba_req(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_dscr_param(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_rate(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_mcs(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_mcsac(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_bw(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_always_tx_1102(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_always_tx_num(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_always_rx(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_rx_filter_frag(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_reg_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_dbb_scaling_amend(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                oal_uint8 *puc_param);

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))
extern oal_uint32 hmac_config_sdio_flowctrl(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif

extern oal_uint32 hmac_config_amsdu_start(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_list_ap(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_list_sta(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_sta_list(mac_vap_stru *pst_mac_vap, oal_uint16 *us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_list_channel(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
extern oal_uint32 hmac_config_get_lauch_cap(mac_vap_stru *pst_mac_vap, oal_uint16 *us_len, oal_uint8 *puc_param);
#endif
extern oal_uint32 hmac_config_send_deauth(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_da);
extern oal_uint32 hmac_config_11i_add_key(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_11i_get_key(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_11i_remove_key(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_11i_set_default_key(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                  oal_uint8 *puc_param);

extern oal_uint32 hmac_config_kick_user(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#ifdef _PRE_WLAN_FEATURE_VOWIFI
extern oal_uint32 hmac_config_vowifi_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif /* _PRE_WLAN_FEATURE_VOWIFI */
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
oal_uint32 hmac_config_update_ip_filter(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_assigned_filter(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif  // _PRE_WLAN_FEATURE_IP_FILTER
extern oal_uint32 hmac_config_pause_tid(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_reg_write(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_dump_all_rx_dscr(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_vap_pkt_stat(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);

extern oal_uint32 hmac_config_amsdu_ampdu_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                 oal_uint8 *puc_param);
extern oal_uint32 hmac_config_11i_add_wep_entry(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                oal_uint8 *puc_param);
extern oal_uint32 hmac_config_dump_rx_dscr(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_dump_tx_dscr(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_alg(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);

#ifdef _PRE_DEBUG_MODE_USER_TRACK
extern oal_uint32 hmac_config_report_thrput_stat(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                 oal_uint8 *puc_param);

#endif
#ifdef _PRE_WLAN_FEATURE_TXOPPS
oal_uint32 hmac_config_set_txop_ps_machw(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_BTCOEX
extern oal_uint32 hmac_config_print_btcoex_status(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                  oal_uint8 *puc_param);
extern oal_uint32 hmac_config_print_btcoex_status(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                  oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_LTECOEX
oal_uint32 hmac_config_ltecoex_mode_set(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_UAPSD
extern oal_uint32 hmac_config_set_uapsden(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_uapsden(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
#endif
extern oal_uint32 hmac_config_set_reset_state(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#ifdef _PRE_WLAN_DFT_STAT
extern oal_uint32 hmac_config_usr_queue_stat(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif

oal_uint32 hmac_config_get_hipkt_stat(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_flowctl_param(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_get_flowctl_stat(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);

#ifdef _PRE_WLAN_FEATURE_11D
extern oal_uint32 hmac_config_set_rd_by_ie_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                  oal_uint8 *puc_param);
#endif
extern oal_uint32 hmac_config_set_amsdu_tx_on(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_ampdu_tx_on(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_country(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_regdomain_pwr(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                oal_uint8 *puc_param);
extern oal_uint32 hmac_config_reduce_sar(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_country(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_connect(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_tid(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_reset_operate(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);

extern oal_uint32 hmac_config_set_channel(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_beacon(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_wps_p2p_ie(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_wps_ie(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_app_ie_to_vap(mac_vap_stru *pst_mac_vap,
                                                oal_app_ie_stru *pst_wps_ie,
                                                en_app_ie_type_uint8 en_type);

extern oal_uint32 hmac_config_alg_param(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_acs_cmd(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);

extern oal_uint32 hmac_config_2040_channel_switch_prohibited(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                             oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_fortymhzintolerant(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                     oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_2040_coext_support(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                     oal_uint8 *puc_param);
extern oal_uint32 hmac_config_rx_fcs_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
extern oal_uint32 hmac_config_set_edca_opt_switch_sta(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                      oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_edca_opt_weight_sta(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                      oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_edca_opt_switch_ap(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                     oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_edca_opt_cycle_ap(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                    oal_uint8 *puc_param);
#endif
extern oal_uint32 hmac_config_sync_cmd_common(mac_vap_stru *pst_mac_vap, wlan_cfgid_enum_uint16 en_cfg_id,
                                              oal_uint16 us_len, oal_uint8 *puc_param);

#ifdef _PRE_WLAN_FEATURE_STA_PM
extern oal_uint32 hmac_config_set_pm_by_module(mac_vap_stru *pst_mac_vap, mac_pm_ctrl_type_enum pm_ctrl_type,
                                               mac_pm_switch_enum pm_enable);
#endif

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
extern oal_uint32 hmac_config_tcp_ack_buf(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
extern oal_uint32 hmac_config_bindcpu(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_napi_weight(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_ps_params(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_rts_param(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_update_protection_tx_param(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                         oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_coex(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_auto_protection(mac_vap_stru *pst_mac_vap, oal_uint8 uc_auto_protection_flag);
extern oal_uint32 hmac_config_set_dfx(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_send_2040_coext(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_2040_coext_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_version(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_ant(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_dscr_th_opt(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_tcp_ack_filter(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
extern oal_uint32 hmac_config_set_pmf_cap(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_pmf_capable,
                                          oal_bool_enum_uint8 en_pmf_required);
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
extern oal_uint32 hmac_config_set_opmode_notify(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_user_rssbw(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
extern oal_uint32 hmac_config_bgscan_enable(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_ampdu_aggr_num(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                 oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_stbc_cap(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_ldpc_cap(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);

extern oal_uint32 hmac_config_set_pmksa(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_del_pmksa(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_flush_pmksa(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_scan_abort(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_remain_on_channel(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                oal_uint8 *puc_param);
extern oal_uint32 hmac_config_cancel_remain_on_channel(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                       oal_uint8 *puc_param);
#ifdef _PRE_WLAN_FEATURE_HS20
extern oal_uint32 hmac_config_set_qos_map(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_interworking_switch(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                  oal_uint8 *puc_param);
extern oal_uint32 hmac_interworking_check(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_param);
#endif
extern oal_uint32 hmac_config_vap_classify_en(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_vap_classify_tid(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_get_fem_pa_status(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                oal_uint8 *puc_param);
extern oal_uint32 hmac_config_query_station_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                 oal_uint8 *puc_param);
extern oal_uint32 hmac_config_query_chr_info_ext(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                                 oal_uint8 *puc_param);
extern oal_uint32 hmac_config_query_rssi(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_query_rate(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);

#ifdef _PRE_WLAN_DFT_STAT
extern oal_uint32 hmac_config_query_ani(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
extern oal_uint32 hmac_config_vap_state_syn(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
extern oal_uint32 hmac_config_set_uapsd_para(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
oal_uint32 hmac_config_set_ip_addr(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
oal_uint32 hmac_config_cfg_vap_h2d(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
extern oal_uint32 hmac_config_user_asoc_state_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
extern oal_uint32 hmac_config_user_cap_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
extern oal_uint32 hmac_config_user_rate_info_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
extern oal_uint32 hmac_config_user_info_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
extern oal_uint32 hmac_config_sta_vap_info_syn(mac_vap_stru *pst_mac_vap);
#ifdef _PRE_WLAN_TCP_OPT
oal_uint32 hmac_config_get_tcp_ack_stream_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_tx_tcp_ack_opt_enable(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_rx_tcp_ack_opt_enable(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_tx_tcp_ack_limit(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_rx_tcp_ack_limit(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
extern oal_void hmac_config_del_p2p_ie(oal_uint8 *puc_ie, oal_uint32 *pul_ie_len);

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
oal_uint32 hmac_config_set_device_freq(oal_uint8 uc_device_freq_type);
#endif

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
extern oal_uint32 hmac_config_dyn_cali_param(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_main */
