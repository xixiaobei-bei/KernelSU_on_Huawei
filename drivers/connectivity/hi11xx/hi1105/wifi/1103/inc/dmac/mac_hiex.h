

#ifndef    __MAC_TTM_H__
#define    __MAC_TTM_H__

#ifdef _PRE_WLAN_FEATURE_HIEX

#include    "oal_mm.h"
#include    "oal_net.h"
#include    "mac_frame.h"
#include    "mac_data.h"
#include    "mac_vap.h"
#include    "dmac_ext_if.h"
#include    "hiex_msg.h"
#ifdef HIEX_CHIP_TYPE_1105
#include    "mac_device.h"
#else
#include    "mac_band.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAL_FILE_ID_DRIVER_MAC_HIEX_H

#ifdef HIEX_CHIP_TYPE_1105
typedef mac_device_stru  mac_hiex_band_stru;
#define MAC_HIEX_GET_USER_IDX(mac_user) ((mac_user)->us_assoc_id)
#define HCC_MSG_ALLOC           FRW_EVENT_ALLOC
#define HCC_MSG_HDR_INIT_WLAN   FRW_EVENT_HDR_INIT
#define HCC_MSG_FREE            FRW_EVENT_FREE
#define HCC_MSG_GET_STRU        frw_get_event_stru
#define hcc_msg_mem_stru        frw_event_mem_stru
#define hcc_msg_stru            frw_event_stru
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
    oal_uint16             ota_seq; /* send to peer */
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
    oal_uint8           category;
    oal_uint8           oui[3];
    oal_uint8           oui_type;
    oal_uint8           oui_sub_type;
    oal_uint8           token;
} __OAL_DECLARE_PACKED_ROM  mac_hiex_action_hdr_stru;

#define MAC_HIEX_RTT_OFFSET_FROM_ETHER (OAL_SIZEOF(mac_ether_header_stru) \
                                        + OAL_SIZEOF(mac_ip_header_stru)  \
                                        + OAL_SIZEOF(udp_hdr_stru))
#define MAC_HIEX_RTT_OFFSET_FROM_IP    (MAC_HIEX_RTT_OFFSET_FROM_ETHER - OAL_SIZEOF(mac_ether_header_stru))
#define MAC_HIEX_RTT_OFFSET_FROM_SNAP  (MAC_HIEX_RTT_OFFSET_FROM_IP + OAL_SIZEOF(mac_llc_snap_stru))

typedef enum {
    MAC_HIEX_LOCAL_MSG_TX_ACTION,
    MAC_HIEX_LOCAL_MSG_RX_ACTION,
} mac_hiex_local_msg_type_enum;
typedef oal_bool_enum_uint8 mac_hiex_local_msg_type_enum_uint8;

typedef struct {
    mac_hiex_local_msg_type_enum_uint8  type;
    oal_uint8                           size;
    oal_uint16                          user_idx;
    oal_uint8                           dat[];
} __OAL_DECLARE_PACKED_ROM mac_hiex_local_msg_stru;

extern mac_hiex_cap_stru g_st_default_hiex_cap;

#ifdef _PRE_WLAN_FEATURE_HIEX_DBG_TST
OAL_STATIC OAL_INLINE mac_hiex_rtt_stru *mac_hiex_get_rtt_stru(oal_netbuf_stru *skb, skb_header_type_enum type)
{
    if (type == SKB_HEADER_ETH) {
        return (mac_hiex_rtt_stru *)(OAL_NETBUF_DATA(skb) + MAC_HIEX_RTT_OFFSET_FROM_ETHER);
    } else if (type == SKB_HEADER_IP) {
        return (mac_hiex_rtt_stru *)(OAL_NETBUF_DATA(skb) + MAC_HIEX_RTT_OFFSET_FROM_IP);
    } else if (type == SKB_HEADER_SNAP) {
        return (mac_hiex_rtt_stru *)(OAL_NETBUF_DATA(skb) + MAC_HIEX_RTT_OFFSET_FROM_SNAP);
    } else {
        return NULL;
    }
}
#endif

extern oal_uint32 mac_hiex_mgr_init(oal_void);
extern oal_void   mac_hiex_mgr_exit(oal_void);
extern mac_hiex_mgr_stru *mac_hiex_get_mgr(oal_void);
extern OAL_CONST mac_hiex_cap_stru *mac_hiex_max_cap(void);
extern oal_uint8  mac_hiex_set_vendor_ie(mac_hiex_cap_stru *band_cap, mac_hiex_cap_stru *user_cap,
    oal_uint8 *buffer);
extern oal_uint32 mac_hiex_nego_cap(mac_hiex_cap_stru *target, mac_hiex_cap_stru *local, mac_hiex_cap_stru *peer);


#ifdef _PRE_WLAN_FEATURE_HIEX_DBG_TST
extern oal_bool_enum_uint8 mac_hiex_pst_tx_mark(oal_netbuf_stru *netbuf, skb_header_type_enum hdr_type);
extern oal_bool_enum_uint8 mac_hiex_pst_rx_mark(oal_netbuf_stru *netbuf, skb_header_type_enum hdr_type);
extern oal_void mac_hiex_tst_update(mac_vap_stru *vap, oal_netbuf_stru *skb, skb_header_type_enum hdr_type,
    mac_hiex_tst_point_enum point);
extern oal_void mac_hiex_show_delay(oal_netbuf_stru *netbuf, skb_header_type_enum header_type,
    mac_hiex_tst_point_enum point_start, mac_hiex_tst_point_enum point_done);
#endif

#endif /* _PRE_WLAN_FEATURE_HIEX */
#endif /* __MAC_TTM_H__ */
