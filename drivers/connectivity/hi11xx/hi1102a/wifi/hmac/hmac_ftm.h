

#ifndef __HMAC_FTM_H__
#define __HMAC_FTM_H__

/* 1 其他头文件包含 */
#include "mac_vap.h"
#include "hmac_vap.h"
#include "oal_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_FTM_H

/*  函数声明 */
#ifdef _PRE_WLAN_FEATURE_FTM
extern oal_uint32  hmac_config_ftm_dbg(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_sta_not_up_rx_public_action(mac_public_action_type_enum_uint8 en_public_action_field,
    hmac_vap_stru *pst_hmac_vap_sta, oal_netbuf_stru *pst_netbuf);
extern oal_void hmac_vap_init_gas(hmac_vap_stru *pst_hmac_vap);
extern oal_uint8 hmac_ftm_is_in_scan_list(mac_vap_stru *pst_mac_vap, oal_uint8 uc_band, oal_uint8 uc_channel_number);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
