

#ifndef __HMAC_11R_H__
#define __HMAC_11R_H__

#include "oal_types.h"
#include "hmac_vap.h"
#include "hmac_roam_main.h"
#include "hmac_roam_connect.h"
#include "hmac_roam_alg.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_11R_H

#ifdef _PRE_WLAN_FEATURE_ADAPTIVE11R
#define HMAC_RSN_AKM_FT_OVER_8021X_VAL        1
#define HMAC_RSN_AKM_FT_PSK_VAL               2
#define HMAC_RSN_AKM_FT_OVER_SAE_VAL          4
#define HMAC_RSN_AKM_FT_OVER_8021X_SHA384_VAL 8
#endif

#ifdef _PRE_WLAN_FEATURE_11R
uint32_t hmac_config_set_ft_ies(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_roam_rx_ft_action(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf);
uint32_t hmac_roam_preauth(hmac_vap_stru *pst_hmac_vap);
void mac_set_md_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
void mac_set_rde_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
void mac_set_tspec_ie(mac_vap_stru *mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len, uint8_t uc_tid);
#endif  // _PRE_WLAN_FEATURE_11R
#ifdef _PRE_WLAN_FEATURE_ADAPTIVE11R
int32_t hmac_scan_attach_akm_suite_to_rsn_ie(uint8_t *mgmt_frame, uint16_t *frame_len);
uint8_t hmac_scan_extend_mgmt_len_needed(oal_netbuf_stru *netbuf, uint16_t netbuf_len);
#endif // _PRE_WLAN_FEATURE_ADAPTIVE11R
#endif
