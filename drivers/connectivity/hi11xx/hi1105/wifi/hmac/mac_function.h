

#ifndef __MAC_FUNCTION_H__
#define __MAC_FUNCTION_H__

#include "mac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

void mac_set_beacon_interval_field(mac_vap_stru *mac_vap, uint8_t *puc_buffer);
void mac_set_cap_info_ap(mac_vap_stru *mac_vap, uint8_t *puc_cap_info);
void mac_set_cap_info_sta(mac_vap_stru *mac_vap, uint8_t *puc_cap_info);
uint8_t mac_set_ssid_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint16_t us_frm_type);
uint8_t mac_set_supported_rates_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer);
uint8_t mac_set_dsss_params(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t uc_channel_num);
#ifdef _PRE_WLAN_FEATURE_1024QAM
void mac_set_1024qam_vendor_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
#endif
#ifdef _PRE_WLAN_FEATURE_PRIV_CLOSED_LOOP_TPC
void mac_set_adjust_pow_vendor_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
#endif
#ifdef _PRE_WLAN_FEATURE_11D
uint8_t mac_set_country_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer);
#endif
uint8_t mac_set_pwrconstraint_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer);
uint8_t mac_set_erp_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer);
uint8_t mac_set_exsup_rates_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer);
uint8_t mac_set_bssload_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer);
void mac_set_timeout_interval_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len, uint32_t type,
    uint32_t timeout);
uint8_t mac_set_ht_capabilities_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer);
uint8_t mac_set_ht_opern_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer);
uint8_t mac_set_obss_scan_params(mac_vap_stru *mac_vap, uint8_t *puc_buffer);
void mac_set_ext_capabilities_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
uint8_t mac_set_tpc_report_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer);
uint8_t mac_set_wmm_params_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, oal_bool_enum_uint8 en_is_qos);
uint8_t mac_set_rsn_ie(mac_vap_stru *mac_vap, uint8_t *puc_pmkid, uint8_t *puc_buffer);
uint8_t mac_set_wpa_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer);
void mac_set_vht_capabilities_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
void mac_set_vht_opern_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
void mac_set_hisi_cap_vendor_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
uint32_t mac_set_csa_bw_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
#if defined(_PRE_WLAN_FEATURE_11K)
void mac_set_rrm_enabled_cap_field(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
#endif
void mac_set_ext_capabilities_ftm_twt(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
void mac_set_vht_capinfo_field_extend(mac_vap_stru *mac_vap, uint8_t *puc_buffer);
void mac_set_vht_opern_ie_rom_cb(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);

void mac_add_app_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer,
    uint16_t *pus_ie_len, en_app_ie_type_uint8 en_type);
void mac_add_wps_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint16_t *pus_ie_len,
    en_app_ie_type_uint8 en_type);
uint16_t mac_encap_2040_coext_mgmt(mac_vap_stru *mac_vap, oal_netbuf_stru *pst_buffer,
    uint8_t uc_coext_info, uint32_t chan_report);
void mac_set_vendor_vht_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
void mac_set_vendor_novht_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer,
    uint8_t *puc_ie_len, uint8_t en_1024qam_capable);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_function.h */
