

#ifndef __HMAC_ENCAP_FRAME_H__
#define __HMAC_ENCAP_FRAME_H__

#include "oal_ext_if.h"
#include "hmac_user.h"
#include "mac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

uint16_t hmac_mgmt_encap_deauth(mac_vap_stru *pst_mac_vap, uint8_t *puc_data, const unsigned char *puc_da,
    uint16_t us_err_code);
uint16_t hmac_mgmt_encap_disassoc(mac_vap_stru *pst_mac_vap, uint8_t *puc_data, uint8_t *puc_da, uint16_t us_err_code);
uint16_t hmac_encap_sa_query_req(mac_vap_stru *pst_mac_vap, uint8_t *puc_data, uint8_t *puc_da, uint16_t us_trans_id);
uint16_t hmac_encap_sa_query_rsp(mac_vap_stru *pst_mac_vap, uint8_t *pst_hdr, uint8_t *puc_data);
uint16_t hmac_encap_notify_chan_width(mac_vap_stru *pst_mac_vap, uint8_t *puc_data, uint8_t *puc_da);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_encap_frame.h */