

#ifndef __HMAC_204080_COEXIST_H__
#define __HMAC_204080_COEXIST_H__

/* 1 其他头文件包含 */
#include "hmac_chan_mgmt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_204080_COEXIST_H

/*  函数声明 */
#if defined(_PRE_WLAN_FEATURE_20_40_80_COEXIST)
extern oal_void hmac_get_pri_sec_chan(mac_bss_dscr_stru *pst_bss_dscr, oal_uint32 *pul_pri_chan,
                                      oal_uint32 *pul_sec_chan);
extern oal_void hmac_switch_pri_sec(mac_vap_stru *pst_mac_vap);
extern oal_bool_enum_uint8 hmac_obss_check_40mhz_2g(mac_vap_stru *pst_mac_vap,
                                                    hmac_scan_record_stru *pst_scan_record);
extern oal_bool_enum_uint8 hmac_obss_check_40mhz_5g(mac_vap_stru *pst_mac_vap,
                                                    hmac_scan_record_stru *pst_scan_record);
extern oal_uint32 hmac_obss_init_scan_hook(hmac_scan_record_stru *pst_scan_record,
                                           hmac_device_stru *pst_dev);
extern oal_uint32 hmac_init_scan_process(hmac_device_stru *pst_mac_dev, mac_vap_stru *pst_mac_vap,
    mac_scan_op_enum_uint8 en_op_type);
extern oal_uint32 hmac_init_scan_try(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap,
    mac_try_init_scan_type en_type);
extern oal_void hmac_init_scan_cancel_timer(hmac_device_stru *pst_hmac_dev);
extern oal_bool_enum_uint8 hmac_device_in_init_scan(mac_device_stru *pst_mac_device);
extern mac_need_init_scan_res_enum_uint8 hmac_need_init_scan(hmac_device_stru *pst_hmac_device,
                                                             mac_vap_stru *pst_in_mac_vap,
                                                             mac_try_init_scan_type en_type);
extern oal_uint32 hmac_init_scan_timeout(void *p_arg);
extern oal_void hmac_init_scan_cb(void *p_scan_record);
extern oal_uint32 hmac_init_scan_do(hmac_device_stru *pst_hmac_dev, mac_vap_stru *pst_mac_vap,
                                    mac_init_scan_req_stru *pst_cmd);
extern oal_bool_enum_uint8 hmac_init_scan_skip_channel(hmac_device_stru *pst_hmac_dev,
                                                       wlan_channel_band_enum_uint8 en_band,
                                                       oal_uint8 uc_idx);
extern oal_void hmac_chan_start_40m_recovery_timer(mac_vap_stru *pst_mac_vap);
extern oal_uint32 hmac_chan_update_40m_intol_user(mac_vap_stru *pst_mac_vap,
                                                  mac_user_stru *pst_mac_user,
                                                  oal_bool_enum_uint8 en_40m_intol_bit);
extern oal_bool_enum_uint8 hmac_chan_is_this_40MHz_set_possible(
    mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap, oal_uint8 uc_pri_chan_idx,
    mac_sec_ch_off_enum_uint8 en_sec_chan_offset, hmac_eval_scan_report_stru *pst_chan_scan_report);
extern oal_bool_enum_uint8 hmac_chan_get_2040_op_chan_list(mac_vap_stru *pst_mac_vap,
                                                           hmac_eval_scan_report_stru *pst_chan_scan_report);
extern oal_void hmac_chan_prepare_for_40m_recovery(
    hmac_vap_stru *pst_hmac_vap, wlan_channel_bandwidth_enum_uint8 en_bandwidth);
extern oal_uint32 hmac_chan_40m_recovery_timeout_fn(void *p_arg);
extern oal_void hmac_chan_stop_40m_recovery_timer(mac_vap_stru *pst_mac_vap);
extern oal_uint8 hmac_calc_up_and_wait_vap(hmac_device_stru *pst_hmac_dev);
extern oal_uint32 hmac_config_enable_2040bss(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_bool_enum_uint8 hmac_ap_up_rx_should_ingore_2040(
    wlan_channel_band_enum_uint8 en_band, oal_bool_enum_uint8 en_support_2040);
extern oal_void hmac_ap_up_rx_2040_coext(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf);
extern oal_bool_enum_uint8 hmac_ap_is_40mhz_intol_bit_set(oal_uint8 *puc_payload, oal_uint16 us_payload_len);
extern oal_bool_enum_uint8 hmac_ap_update_2040_chan_info(
    mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload,
    oal_uint16 us_payload_len, oal_uint8 uc_pri_chan_idx, mac_sec_ch_off_enum_uint8 en_sec_ch_offset);
extern oal_void hmac_ap_process_obss_40mhz_intol(
    hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_payload, oal_uint16 us_payload_len);
extern oal_void hmac_rx_notify_channel_width(mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_netbuf);
extern oal_void  hmac_update_join_req_params_2040(mac_vap_stru *pst_mac_vap, mac_bss_dscr_stru *pst_bss_dscr);
#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
