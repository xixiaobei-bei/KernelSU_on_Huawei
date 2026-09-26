

#ifndef __HMAC_TX_AMSDU_H__
#define __HMAC_TX_AMSDU_H__

#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_tx_data.h"
#include "hmac_main.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TX_AMSDU_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* HT控制信息的amsdu能力位 */
#define HT_CAP_AMSDU_LEN 0x0800

/* amsdu生命周期15ms FPGA 1500 */
#define HMAC_AMSDU_LIFE_TIME 5

/* amsdu聚合最小帧长(对照ack帧长,拍板决定) */
#define HMAC_AMSDU_TX_MIN_PKT_LENGTH 14
/* 发送方向组masdu帧的netbuf剩余tailroom最小值, LLC+PKT LENTH+PAD */
#define HMAC_AMSDU_TX_MIN_LENGTH (SNAP_LLC_FRAME_LEN + HMAC_AMSDU_TX_MIN_PKT_LENGTH + 3)

/* 短包聚合最大个数 */
#define HMAC_AMSDU_SHORT_PACKET_NUM 0x02

/* 小于500字节的包为短包    */
#define HMAC_AMSDU_SHORT_PACKET_LEN 500

/* CCMP 和TKIP 最大都是16字节 IV + MIC */
#define HMAC_CYPHER_MAX_LEN 16

/* 一个amsdu下允许拥有的msdu的最大个数 */
#define WLAN_AMSDU_MAX_NUM 4


OAL_STATIC OAL_INLINE void hmac_amsdu_set_maxnum(hmac_amsdu_stru *pst_amsdu, uint8_t uc_max_num)
{
    if (uc_max_num > WLAN_AMSDU_MAX_NUM) {
        pst_amsdu->amsdu_maxnum = WLAN_AMSDU_MAX_NUM;
    } else {
        pst_amsdu->amsdu_maxnum = uc_max_num;
    }
}


OAL_STATIC OAL_INLINE void hmac_amsdu_set_maxsize(hmac_amsdu_stru *pst_amsdu,
    hmac_user_stru *pst_hmac_user, uint16_t us_max_size)
{
    if (us_max_size > pst_hmac_user->amsdu_maxsize) {
        pst_amsdu->amsdu_maxsize = pst_hmac_user->amsdu_maxsize;
    } else {
        pst_amsdu->amsdu_maxsize = us_max_size;
    }

    if (pst_hmac_user->st_user_base_info.st_ht_hdl.uc_htc_support == 1) {
        /* Account for HT-MAC Header, FCS & Security headers */
        pst_amsdu->amsdu_maxsize -= (MAC_80211_QOS_HTC_FRAME_LEN + WLAN_HDR_FCS_LENGTH + HMAC_CYPHER_MAX_LEN);
    } else {
        /* Account for QoS-MAC Header, FCS & Security headers */
        pst_amsdu->amsdu_maxsize -= (MAC_80211_QOS_FRAME_LEN + WLAN_HDR_FCS_LENGTH + HMAC_CYPHER_MAX_LEN);
    }

    if (pst_hmac_user->uc_is_wds == 1) {
        /* Account for the 4th address in WDS-MAC Header */
        pst_amsdu->amsdu_maxsize -= ETHER_ADDR_LEN;
    }
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_amsdu_is_short_pkt(uint32_t frame_len)
{
    if (frame_len < HMAC_AMSDU_SHORT_PACKET_LEN) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/* 10 函数声明 */
uint32_t hmac_amsdu_notify(hmac_vap_stru *pst_vap, hmac_user_stru *pst_user,
    oal_netbuf_stru *pst_buf);
void hmac_amsdu_init_user(hmac_user_stru *pst_hmac_user_sta);
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
void hmac_tx_encap_large_skb_amsdu_1103(hmac_vap_stru *vap, hmac_user_stru *user,
    oal_netbuf_stru *buf, mac_tx_ctl_stru *tx_ctl);
void hmac_tx_encap_large_skb_amsdu_1106(hmac_vap_stru *hmac_vap, hmac_user_stru *user,
    oal_netbuf_stru *buf, mac_tx_ctl_stru *tx_ctl);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_tx_amsdu.h */
