

#ifndef    __HMAC_HIEX_H__
#define    __HMAC_HIEX_H__

#ifdef _PRE_WLAN_FEATURE_HIEX
#include "mac_hiex.h"
#include "hmac_vap.h"
#include "hmac_user.h"
#include "mac_user.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DRIVER_HMAC_HIEX_H

#ifdef HIEX_CHIP_TYPE_1105
#define hmac_hiex_get_user(idx) mac_res_get_hmac_user_etc(idx)
#else
#define hmac_hiex_get_user(idx) hmac_user_get_valid(idx)
#endif

OAL_STATIC OAL_INLINE mac_hiex_band_stru *hmac_hiex_get_band(hmac_vap_stru *vap)
{
#ifdef HIEX_CHIP_TYPE_1105
    if (vap != OAL_PTR_NULL) {
        return mac_res_get_dev_etc(vap->st_vap_base_info.uc_device_id);
    }
    return OAL_PTR_NULL;
#else
    if (vap != OAL_PTR_NULL && vap->pst_band != OAL_PTR_NULL) {
        return &vap->pst_band->st_base_info;
    }
    return OAL_PTR_NULL;
#endif
}

extern oal_uint32 hmac_hiex_user_init(hmac_user_stru *user);
extern oal_uint32 hmac_hiex_user_exit(mac_vap_stru *pst_mac_vap, hmac_user_stru *user);
extern oal_bool_enum_uint8 hmac_hiex_sit_mark(oal_netbuf_stru *skb, oal_uint8 vap_id, oal_uint16 user_idx,
    oal_bool_enum_uint8 to_server);
extern oal_void   hmac_hiex_rx_assoc_req(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_uint8 *payload, oal_uint32 payload_len);
extern oal_uint32 hmac_hiex_rx_assoc_rsp(hmac_vap_stru *mac_sta, hmac_user_stru *hmac_user,
    oal_uint8 *payload, oal_uint32 payload_len);
extern oal_uint8  hmac_hiex_encap_ie(mac_vap_stru *mac_vap, mac_user_stru *user, oal_uint8 *frame);
extern oal_uint32 hmac_hiex_rx_local_msg(hcc_msg_mem_stru *event_mem);
extern oal_uint32 hmac_hiex_tx_local_msg(mac_vap_stru *vap, mac_hiex_local_msg_stru *msg, oal_uint32 size);
extern oal_void hmac_hiex_judge_is_game_marked_enter_to_vo(hmac_vap_stru *pst_hmac_vap, mac_user_stru *pst_user,
    oal_netbuf_stru *pst_buf, oal_uint8 *puc_tid);
#endif
#endif
