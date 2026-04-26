

#ifndef    __MAC_TTM_H__
#define    __MAC_TTM_H__

#ifdef _PRE_WLAN_FEATURE_HIEX
#include    "oal_net.h"
#include    "mac_frame.h"
#include    "mac_data.h"
#include    "mac_vap.h"

#include    "hiex_msg.h"
#ifdef _HIEX_CHIP_TYPE_110X
#include    "mac_device.h"
#else
#include    "mac_band.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAL_FILE_ID_DRIVER_MAC_HIEX_H

#ifdef _HIEX_CHIP_TYPE_110X
typedef mac_device_stru  mac_hiex_band_stru;
#define MAC_HIEX_GET_USER_IDX(mac_user) ((mac_user)->us_assoc_id)
#define HCC_MSG_ALLOC           frw_event_alloc_m
#define HCC_MSG_HDR_INIT_WLAN   frw_event_hdr_init
#define HCC_MSG_FREE            frw_event_free_m
#define HCC_MSG_GET_STRU        frw_get_event_stru
#else
typedef mac_band_stru  mac_hiex_band_stru;
#define MAC_HIEX_GET_USER_IDX(mac_user) ((mac_user)->us_user_idx)
#endif

#define MAC_HIEX_FRM_BUF_SIZE    400 /* TTM最长帧体 */
#define MAC_HIEX_CHECK_INTERVAL  100 /* 上报周期 */
#define DPERF_PAYLOAD_SIZE       ((int)sizeof(mac_hiex_iperf_stru))
#define DPERF_BASE_PORT          5008
#define DPERF_END_PORT           5018

typedef struct mac_hiex_mgr {
    uint16_t             ota_seq; /* send to peer */
} mac_hiex_mgr_stru;

struct mac_hiex_vendor_ie {
    mac_ieee80211_vendor_ie_stru vender;
    mac_hiex_cap_stru            cap;
} __OAL_DECLARE_PACKED_ROM;
typedef struct mac_hiex_vendor_ie mac_hiex_vendor_ie_stru;

/* presence response Frame - Frame Body */
/* ---------------------------------------------------------------------- */
/* |Category |OUI   |OUI type|OUI subtype  | Dialog Token   |Element    | */
/* ---------------------------------------------------------------------- */
/* |1        |3     |1       |1            |1               |variable   | */
/* ---------------------------------------------------------------------- */
/**************************************************************************/
typedef struct {
    uint8_t           category;
    uint8_t           oui[3];
    uint8_t           oui_type;
    uint8_t           oui_sub_type;
    uint8_t           token;
} __OAL_DECLARE_PACKED_ROM  mac_hiex_action_hdr_stru;

#define MAC_HIEX_RTT_OFFSET_FROM_ETHER (sizeof(mac_ether_header_stru) \
                                        + sizeof(mac_ip_header_stru)  \
                                        + sizeof(udp_hdr_stru))
#define MAC_HIEX_RTT_OFFSET_FROM_IP    (MAC_HIEX_RTT_OFFSET_FROM_ETHER - sizeof(mac_ether_header_stru))
#define MAC_HIEX_RTT_OFFSET_FROM_SNAP  (MAC_HIEX_RTT_OFFSET_FROM_IP + sizeof(mac_llc_snap_stru))

extern mac_hiex_cap_stru g_st_default_hiex_cap;

#ifdef _PRE_WLAN_FEATURE_HIEX_DBG_TST
OAL_STATIC OAL_INLINE mac_hiex_rtt_stru *mac_hiex_get_rtt_stru(oal_netbuf_stru *skb, skb_header_type_enum type)
{
    if (type == SKB_HEADER_ETH) {
        return (mac_hiex_rtt_stru *)(oal_netbuf_data(skb) + MAC_HIEX_RTT_OFFSET_FROM_ETHER);
    } else if (type == SKB_HEADER_IP) {
        return (mac_hiex_rtt_stru *)(oal_netbuf_data(skb) + MAC_HIEX_RTT_OFFSET_FROM_IP);
    } else if (type == SKB_HEADER_SNAP) {
        return (mac_hiex_rtt_stru *)(oal_netbuf_data(skb) + MAC_HIEX_RTT_OFFSET_FROM_SNAP);
    } else {
        return NULL;
    }
}
#endif
mac_hiex_mgr_stru *mac_hiex_get_mgr(void);
uint8_t  mac_hiex_set_vendor_ie(mac_hiex_cap_stru *band_cap, mac_hiex_cap_stru *user_cap,
    uint8_t *buffer);
uint32_t mac_hiex_nego_cap(mac_hiex_cap_stru *target, mac_hiex_cap_stru *local, mac_hiex_cap_stru *peer);

#ifdef _PRE_WLAN_FEATURE_HIEX_DBG_TST
oal_bool_enum_uint8 mac_hiex_pst_tx_mark(oal_netbuf_stru *netbuf, skb_header_type_enum hdr_type);
oal_bool_enum_uint8 mac_hiex_pst_rx_mark(oal_netbuf_stru *netbuf, skb_header_type_enum hdr_type);
void mac_hiex_tst_update(mac_vap_stru *vap, oal_netbuf_stru *skb, skb_header_type_enum hdr_type,
    mac_hiex_tst_point_enum point);
void mac_hiex_show_delay(oal_netbuf_stru *netbuf, skb_header_type_enum header_type,
    mac_hiex_tst_point_enum point_start, mac_hiex_tst_point_enum point_done);
#endif

#endif /* _PRE_WLAN_FEATURE_HIEX */
#endif /* __MAC_TTM_H__ */
