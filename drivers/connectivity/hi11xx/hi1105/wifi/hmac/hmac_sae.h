

#ifndef __HMAC_SAE_H__
#define __HMAC_SAE_H__

#include "oal_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SAE_H

#ifdef _PRE_WLAN_FEATURE_SAE
uint32_t hmac_sta_sae_connect_info_wpas(hmac_vap_stru *hmac_sta, hmac_auth_req_stru *auth_req);
uint32_t hmac_sta_process_sae_commit(hmac_vap_stru *pst_sta, oal_netbuf_stru *pst_netbuf);
uint32_t hmac_sta_process_sae_confirm(hmac_vap_stru *pst_sta, oal_netbuf_stru *pst_netbuf);
void hmac_report_ext_auth_worker(oal_work_stru *pst_sae_report_ext_auth_worker);
void hmac_update_sae_connect_param(hmac_vap_stru *pst_hmac_vap, uint8_t *puc_ie, uint32_t ie_len);
#endif
#endif /* end of hmac_sae.h */
