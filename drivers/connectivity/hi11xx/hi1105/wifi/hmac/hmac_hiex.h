

#ifndef    __HMAC_HIEX_H__
#define    __HMAC_HIEX_H__

#ifdef _PRE_WLAN_FEATURE_HIEX
#include "mac_hiex.h"
#include "hmac_vap.h"
#include "hmac_user.h"
#include "mac_user.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DRIVER_HMAC_HIEX_H

#ifdef _HIEX_CHIP_TYPE_110X
#define hmac_hiex_get_user(idx) mac_res_get_hmac_user(idx)
#else
#define hmac_hiex_get_user(idx) hmac_user_get_valid(idx)
#endif

OAL_STATIC OAL_INLINE mac_hiex_band_stru *hmac_hiex_get_band(hmac_vap_stru *vap)
{
#ifdef _HIEX_CHIP_TYPE_110X
    if (vap != NULL) {
        return mac_res_get_dev(vap->st_vap_base_info.uc_device_id);
    }
    return NULL;
#else
    if (vap != NULL && vap->pst_band != NULL) {
        return &vap->pst_band->st_base_info;
    }
    return NULL;
#endif
}
uint32_t hmac_hiex_user_init(hmac_user_stru *user);
uint32_t hmac_hiex_user_exit(hmac_user_stru *user);
oal_bool_enum_uint8 hmac_hiex_sit_mark(oal_netbuf_stru *skb, uint8_t vap_id, uint16_t user_idx,
    oal_bool_enum_uint8 to_server);
void   hmac_hiex_rx_assoc_req(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    uint8_t *payload, uint32_t payload_len);
uint32_t hmac_hiex_rx_assoc_rsp(hmac_vap_stru *mac_sta, hmac_user_stru *hmac_user,
    uint8_t *payload, uint32_t payload_len);
uint8_t  hmac_hiex_encap_ie(mac_vap_stru *mac_vap, mac_user_stru *user, uint8_t *frame);
uint32_t hmac_hiex_rx_local_msg(frw_event_mem_stru *event_mem);
uint32_t hmac_hiex_tx_local_msg(mac_vap_stru *vap, mac_hiex_local_msg_stru *msg, uint32_t size);
void hmac_hiex_judge_is_game_marked_enter_to_vo(hmac_vap_stru *pst_hmac_vap, mac_user_stru *pst_user,
    oal_netbuf_stru *pst_buf, uint8_t *puc_tid);
uint32_t hmac_save_max_tx_power_info(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
uint16_t hmac_assoc_req_set_max_tx_power_ie(uint8_t *buffer);
void hmac_rx_assoc_rsp_parse_tb_frame_gain(uint8_t *payload, uint16_t msg_len);
uint8_t hmac_get_tb_frame_gain(wlan_channel_band_enum_uint8 band, uint8_t chan_number);
#endif
#endif
