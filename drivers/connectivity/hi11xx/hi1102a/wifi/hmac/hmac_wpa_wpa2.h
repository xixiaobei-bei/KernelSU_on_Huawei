

#ifndef __HMAC_WPA_WPA2_H__
#define __HMAC_WPA_WPA2_H__
/* 1 其他头文件包含 */
#include "mac_device.h"
#include "oam_wdk.h"
#include "mac_vap.h"
#include "hmac_vap.h"
#include "hmac_user.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_WPA_WPA2_H

/*  函数声明 */
extern oal_uint32 hmac_check_assoc_req_security_cap_common(mac_vap_stru *pst_mac_vap,
                                                           oal_uint8 *puc_ie,
                                                           oal_uint32 ul_msg_len,
                                                           oal_uint8 uc_80211i_mode,
                                                           oal_uint8 uc_offset,
                                                           mac_status_code_enum_uint16 *pen_status_code);
extern oal_uint32 hmac_check_assoc_req_security_cap_authenticator(
    hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_mac_hdr, oal_uint8 *puc_payload,
    oal_uint32 ul_msg_len, hmac_user_stru *pst_hmac_user, mac_status_code_enum_uint16 *pen_status_code);
extern oal_void hmac_set_group_mgmt_policy(mac_bss_80211i_info_stru *pst_bss_80211i_info,
                                           oal_uint8 *puc_ie,
                                           oal_uint8 *puc_index,
                                           oal_int32 *pl_ie_len_left);
extern oal_void hmac_init_cihper_policy(oal_uint8 *puc_pcip_policy,
                                        oal_uint8 *puc_auth_policy,
                                        mac_bss_80211i_info_stru *pst_bss_80211i_info);
extern oal_bool_enum_uint8 hmac_parse_cipher_suit(mac_bss_dscr_stru *pst_bss_dscr, oal_uint8 uc_cipher_type,
                                                  oal_uint8 *puc_ie, oal_int32 l_len);
extern oal_void hmac_scan_update_bss_list_security(mac_bss_dscr_stru *pst_bss_dscr,
                                                   oal_uint8 *puc_frame_body,
                                                   oal_uint16 us_frame_len,
                                                   oal_uint16 us_offset);
extern oal_bool_enum_uint8 hmac_scan_update_bss_list_wpa(mac_bss_dscr_stru *pst_bss_dscr, oal_uint8 *puc_ie);
extern oal_bool_enum_uint8 hmac_scan_update_bss_list_rsn(mac_bss_dscr_stru *pst_bss_dscr, oal_uint8 *puc_ie);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
