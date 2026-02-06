

#ifndef __MAC_11KVR_H__
#define __MAC_11KVR_H__

/* 1 头文件包含 */
#include "oal_types.h"
#include "mac_frame.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_11KVR_H

/*  函数声明 */
#if defined(_PRE_WLAN_FEATURE_11K)  || defined(_PRE_WLAN_FEATURE_FTM)
extern oal_void mac_set_rrm_enabled_cap_field(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
#endif
#ifdef _PRE_WLAN_FEATURE_11R
extern oal_void mac_set_md_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void mac_set_rde_ie(oal_void *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void mac_set_tspec_ie(oal_void *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len,
                                 oal_uint8 uc_tid);
#endif // _PRE_WLAN_FEATURE_11R
#ifdef _PRE_WLAN_FEATURE_FTM
extern oal_void mac_ftm_add_to_ext_capabilities_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
#endif

/* WFA TPC RPT OUI 定义 */
extern OAL_CONST oal_uint8 g_auc_wfa_oui[MAC_OUI_LEN];

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
