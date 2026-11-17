

#ifndef __HMAC_OTA_REPORT_H__
#define __HMAC_OTA_REPORT_H__

#include "mac_frame.h"
#include "mac_vap.h"
#include "hmac_vap.h"
#include "hmac_hid2d.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_OTA_REPORT_H

uint32_t mac_rx_report_80211_frame(uint8_t *pst_mac_vap, uint8_t *pst_rx_cb, oal_netbuf_stru *pst_netbuf,
    oam_ota_type_enum_uint8 en_ota_type);
uint32_t mac_report_80211_frame(uint8_t *puc_mac_vap, uint8_t *puc_rx_cb, oal_netbuf_stru *pst_netbuf,
    uint8_t *puc_des_addr, oam_ota_type_enum_uint8 en_ota_type);
uint32_t hmac_tx_report_eth_frame(mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_netbuf);
#ifdef _PRE_WLAN_DFT_DUMP_FRAME
void hmac_rx_report_eth_frame(mac_vap_stru *pst_mac_vap,
    oal_netbuf_stru *pst_netbuf);
#endif
#endif
