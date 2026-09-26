

#ifndef __HMAC_11AX_H__
#define __HMAC_11AX_H__

#include "hmac_resource.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_11AX_H

#ifdef _PRE_WLAN_FEATURE_11AX

void hmac_mgmt_proc_he_uora(mac_vap_stru *mac_vap, hmac_scanned_bss_info *scaned_bss);
uint32_t hmac_proc_he_uora_para_update(mac_vap_stru *mac_vap, uint8_t uora_para_change);
uint8_t hmac_set_he_capabilities_all_ie(hmac_scanned_bss_info *scaned_bss, mac_vap_stru *mac_vap,
    uint8_t *buffer);
uint32_t hmac_sta_up_update_mu_edca_params_machw(hmac_vap_stru *pst_hmac_sta,
    mac_wmm_set_param_type_enum_uint8 en_wmm_set_param_type);
uint32_t hmac_proc_he_cap_ie(mac_vap_stru *pst_mac_vap,
                             hmac_user_stru *pst_hmac_user,
                             uint8_t *puc_he_cap_ie);
uint32_t hmac_proc_he_bss_color_change_announcement_ie(mac_vap_stru *pst_mac_vap,
                                                       hmac_user_stru *pst_hmac_user,
                                                       uint8_t *puc_bss_color_ie);
uint32_t hmac_ie_check_he_sta(hmac_vap_stru *pst_hmac_sta,
                              hmac_user_stru *pst_hmac_user,
                              uint8_t *puc_payload,
                              uint16_t us_msg_len);
void hmac_sta_up_update_assoc_rsp_sr_params(uint8_t *puc_payload,
                                            uint16_t us_msg_len,
                                            hmac_vap_stru *pst_hmac_sta,
                                            uint8_t uc_frame_sub_type,
                                            hmac_user_stru *pst_hmac_user);
void hmac_sta_up_update_he_nfrp_params(uint8_t *payload,
                                       uint16_t msg_len, hmac_vap_stru *hmac_sta,
                                       uint8_t frame_sub_type, hmac_user_stru *hmac_user);
void hmac_sta_up_update_he_edca_params(uint8_t *puc_payload,
                                       uint16_t us_msg_len,
                                       hmac_vap_stru *pst_hmac_sta,
                                       uint8_t uc_frame_sub_type,
                                       hmac_user_stru *pst_hmac_user);
uint32_t _hmac_config_ul_ofdma_amsdu(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param);
uint32_t _hmac_config_set_send_tb_ppdu_flag(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param);
uint32_t hmac_config_ax_debug(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
void hmac_scan_update_11ax_ie(hmac_vap_stru *pst_hmac_vap,
                              mac_bss_dscr_stru *pst_bss_dscr,
                              uint8_t *puc_frame_body,
                              uint16_t us_frame_len);
uint32_t hmac_scan_proc_multi_bssid_scanned_bss(hmac_vap_stru *pst_hmac_vap,
                                                oal_netbuf_stru *pst_netbuf,
                                                uint16_t us_buf_len);
uint8_t mac_device_trans_bandwith_to_he_capinfo(wlan_bw_cap_enum_uint8 en_max_op_bd);
void mac_vap_user_set_tx_data_include_om(mac_vap_stru *pst_mac_vap,
                                         mac_user_stru *pst_mac_user);
void mac_vap_11ax_cap_init(mac_vap_stru *pst_mac_vap);
void mac_vap_init_11ax_rates(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev);
void mac_set_he_capabilities_ie(mac_vap_stru *mac_vap, uint8_t *buffer, uint8_t *ie_len);
void mac_get_htc_uph_om_value(uint8_t uc_nss, uint8_t uc_bw,
                              uint8_t uc_mimo_resound, uint8_t uc_ul_mu_disable,
                              uint32_t *pul_htc_value);
void mac_get_htc_om_value(uint8_t uc_nss, uint8_t uc_bw,
                          uint8_t uc_mimo_resound, uint8_t uc_ul_mu_disable,
                          uint32_t *pul_htc_value);
void mac_set_htc_om_field(uint8_t uc_nss, uint8_t uc_bw, uint8_t uc_mimo_resound,
                          uint32_t *pul_htc_value);
void mac_set_he_operation_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
void mac_set_he_ie_in_assoc_rsp(mac_vap_stru *mac_vap, uint16_t us_assoc_id,
                                uint8_t *puc_asoc_rsp, uint8_t *puc_ie_len);
void hmac_sta_join_update_protocol_mib(mac_vap_stru *p_mac_vap,
    mac_bss_dscr_stru *p_bss_dscr);
void hmac_sta_update_join_multi_bssid_info(mac_vap_stru *pst_mac_vap, dmac_ctx_join_req_set_reg_stru *pst_reg_params,
    mac_multi_bssid_info *pst_mbssid_info);
uint8_t mac_set_he_6ghz_band_cap_ie(mac_vap_stru *mac_vap, uint8_t *buffer);
#endif

#endif
