

#ifndef __HMAC_11W_H__
#define __HMAC_11W_H__

#include "mac_vap.h"
#include "hmac_device.h"
#include "hmac_user.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_11W_H

#ifdef _PRE_WLAN_FEATURE_PMF
void hmac_pmf_update_pmf_black_list(mac_vap_stru *pst_mac_vap, mac_bss_dscr_stru *pst_bss_dscr,
                                    mac_nl80211_mfp_enum_uint8 en_conn_pmf_enable);
uint32_t hmac_enable_pmf(mac_vap_stru *pst_mac_vap, uint8_t *puc_param);
uint32_t hmac_config_vap_pmf_cap(mac_vap_stru *pst_mac_vap, wlan_pmf_cap_status_uint8 en_pmf_cap);
oal_bool_enum_uint8 hmac_device_pmf_find_black_list(hmac_device_stru *pst_hmac_dev, uint8_t *puc_mac_addr);
void hmac_device_pmf_add_black_list(hmac_device_stru *pst_hmac_dev, uint8_t *puc_mac_addr);
uint32_t hmac_get_assoc_comeback_time(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user);
uint32_t hmac_pmf_check_err_code(mac_user_stru *pst_user_base_info, oal_bool_enum_uint8 en_is_protected,
    uint8_t *puc_mac_hdr);
uint32_t hmac_sa_query_interval_timeout(void *p_arg);
uint32_t hmac_start_sa_query(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user,
    oal_bool_enum_uint8 en_is_protected);
void hmac_send_sa_query_rsp(mac_vap_stru *pst_mac_vap, uint8_t *pst_hdr, oal_bool_enum_uint8 en_is_protected);
void hmac_rx_sa_query_req(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf,
    oal_bool_enum_uint8 en_is_protected);
void hmac_rx_sa_query_rsp(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf,
    oal_bool_enum_uint8 en_is_protected);
void hmac_sta_roam_update_pmf(hmac_vap_stru *pst_hmac_vap, mac_bss_dscr_stru *pst_mac_bss_dscr);
void hmac_stop_sa_query_timer(hmac_user_stru *pst_hmac_user);
wlan_pmf_cap_status_uint8 mac_get_pmf_cap(uint8_t *puc_ie, uint32_t ie_len);
uint32_t mac_vap_init_pmf(mac_vap_stru *pst_mac_vap, uint8_t uc_pmf_cap, uint8_t uc_mgmt_proteced);
void hmac_mgmt_sta_roam_rx_sa_query(hmac_vap_stru *p_hmac_vap, oal_netbuf_stru *p_netbuf,
    uint8_t *data, oal_bool_enum_uint8 isProtected);
#endif

#endif

