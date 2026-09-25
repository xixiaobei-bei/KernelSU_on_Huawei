

#ifdef _PRE_WLAN_FEATURE_HIEX
#include    "oal_types.h"
#include    "mac_hiex.h"
#include    "dmac_ext_if.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAL_FILE_ID_DRIVER_MAC_HIEX_C

/* hiex全局管理结构体 */
OAL_STATIC mac_hiex_mgr_stru g_st_hiex_mgr;

/* hiex芯片能力，由定制化下发 */
mac_hiex_cap_stru g_st_default_hiex_cap = {
#ifndef WIN32
    .bit_hiex_enable        = OAL_FALSE,
    .bit_hiex_version       = MAC_HIMIT_VERSION_V1,
    .bit_himit_enable       = OAL_FALSE,
    .bit_ht_himit_enable    = OAL_FALSE,
    .bit_vht_himit_enable   = OAL_FALSE,
    .bit_he_himit_enable    = OAL_FALSE,
    .bit_he_htc_himit_id    = 0,
    .bit_himit_version      = MAC_HIMIT_VERSION_V1,
#ifdef HIEX_CHIP_TYPE_1105
    .bit_chip_type          = MAC_HIEX_CHIP_TYPE_HI1105,
#else
    .bit_chip_type          = MAC_HIEX_CHIP_TYPE_HI1152,
#endif
    .bit_resv               = 0,
#else
    OAL_FALSE,                    // bit_hiex_enable
    MAC_HIMIT_VERSION_V1,         // bit_hiex_version
    OAL_FALSE,                    // bit_himit_enable
    OAL_FALSE,                    // bit_ht_himit_enable
    OAL_FALSE,                    // bit_vht_himit_enable
    OAL_FALSE,                    // bit_he_himit_enable
    0,                            // bit_he_htc_himit_id
    MAC_HIMIT_VERSION_V1,         // bit_himit_version
#ifdef HIEX_CHIP_TYPE_1105
    MAC_HIEX_CHIP_TYPE_HI1105,    // bit_chip_type
#else
    MAC_HIEX_CHIP_TYPE_HI1152,    // bit_chip_type
#endif
    0,                            // bit_resv
#endif
};


OAL_CONST mac_hiex_cap_stru *mac_hiex_max_cap(oal_void)
{
    return &g_st_default_hiex_cap;
}


mac_hiex_mgr_stru *mac_hiex_get_mgr(oal_void)
{
    return &g_st_hiex_mgr;
}


oal_void  mac_hiex_mgr_exit(oal_void)
{
    mac_hiex_mgr_stru   *mgr = mac_hiex_get_mgr();

    memset_s(mgr, OAL_SIZEOF(mac_hiex_mgr_stru), 0, OAL_SIZEOF(mac_hiex_mgr_stru));
}


oal_uint32  mac_hiex_mgr_init(oal_void)
{
    mac_hiex_mgr_stru   *mgr = mac_hiex_get_mgr();

    memset_s(mgr, OAL_SIZEOF(mac_hiex_mgr_stru), 0, OAL_SIZEOF(mac_hiex_mgr_stru));

    return OAL_SUCC;
}

OAL_STATIC oal_bool_enum_uint8 mac_hiex_cap_compatible(mac_hiex_cap_stru *local, mac_hiex_cap_stru *peer)
{
    if (local->bit_himit_enable && peer->bit_himit_enable &&
        local->bit_he_htc_himit_id != peer->bit_he_htc_himit_id) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

oal_uint32 mac_hiex_nego_cap(mac_hiex_cap_stru *target, mac_hiex_cap_stru *local, mac_hiex_cap_stru *peer)
{
    mac_hiex_cap_stru  *tmp = OAL_PTR_NULL;

    if (target == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    if (memset_s(target, sizeof(mac_hiex_cap_stru), 0, sizeof(mac_hiex_cap_stru)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_HIEX, "{mac_hiex_nego_cap::memset_s failed}");
        return OAL_FAIL;
    }

    /* 无对端设置，直接用本地能力，如封装beacon、probe request/rsp 、assoc request/rsp */
    if (local != OAL_PTR_NULL && peer == OAL_PTR_NULL) {
        tmp = local;
    } else if (local != OAL_PTR_NULL && peer != OAL_PTR_NULL) {
        if (!mac_hiex_cap_compatible(local, peer)) {
            return OAL_FAIL;
        }
        tmp = peer;
    } else {
        OAM_WARNING_LOG0(0, OAM_SF_HIEX, "{mac_hiex_nego_cap::invalid config}");
        return OAL_FAIL;
    }

    target->bit_hiex_enable      = tmp->bit_hiex_enable && local->bit_hiex_enable;
    target->bit_hiex_version     = OAL_MIN(tmp->bit_hiex_version, local->bit_hiex_version);
    target->bit_himit_enable     = tmp->bit_himit_enable && local->bit_himit_enable;
    target->bit_ht_himit_enable  = tmp->bit_ht_himit_enable && local->bit_ht_himit_enable;
    target->bit_vht_himit_enable = tmp->bit_vht_himit_enable && local->bit_vht_himit_enable;
    target->bit_he_himit_enable  = tmp->bit_he_himit_enable && local->bit_he_himit_enable;
    target->bit_he_htc_himit_id  = tmp->bit_he_htc_himit_id;
    target->bit_himit_version    = OAL_MIN(tmp->bit_himit_version, local->bit_himit_version);
    target->bit_chip_type        = tmp->bit_chip_type;
    /* 不协商,本地能力 */
    target->bit_ersru_enable     = local->bit_ersru_enable;

    return OAL_SUCC;
}

oal_uint8 mac_hiex_set_vendor_ie(mac_hiex_cap_stru *band_cap, mac_hiex_cap_stru *user_cap,
                                 oal_uint8 *buffer)
{
    mac_hiex_vendor_ie_stru *vendor_ie = OAL_PTR_NULL;
    mac_hiex_cap_stru       *ie_cap = OAL_PTR_NULL;
    oal_uint8                size = 0;

    if (!band_cap->bit_hiex_enable || (user_cap && !user_cap->bit_hiex_enable)) {
        return size;
    }

    vendor_ie = (mac_hiex_vendor_ie_stru *)buffer;
    vendor_ie->vender.uc_element_id = MAC_EID_VENDOR;
    vendor_ie->vender.uc_len = sizeof(mac_hiex_vendor_ie_stru) - MAC_IE_HDR_LEN;

    vendor_ie->vender.auc_oui[0] = (oal_uint8)(((oal_uint32)MAC_WLAN_OUI_HUAWEI >> NUM_16_BITS) & 0xff); //lint !e572
    vendor_ie->vender.auc_oui[1] = (oal_uint8)(((oal_uint32)MAC_WLAN_OUI_HUAWEI >> NUM_8_BITS) & 0xff);
    vendor_ie->vender.auc_oui[2] = (oal_uint8)(((oal_uint32)MAC_WLAN_OUI_HUAWEI) & 0xff);  /* hiex oui 数组2 */

    vendor_ie->vender.uc_oui_type = MAC_HISI_HIEX_IE;

    ie_cap = &vendor_ie->cap;
    if (mac_hiex_nego_cap(ie_cap, band_cap, user_cap) != OAL_SUCC) {
        return size;
    }

    return OAL_SIZEOF(mac_hiex_vendor_ie_stru);
}

#ifdef _PRE_WLAN_FEATURE_HIEX_DBG_TST

oal_bool_enum_uint8 mac_hiex_is_valid_udp(oal_uint8 *ip_header)
{
    mac_ip_header_stru    *ip = (mac_ip_header_stru *)ip_header;
    udp_hdr_stru          *udp = OAL_PTR_NULL;
    oal_uint16             len, src_port, dst_port;

    if (ip == OAL_PTR_NULL || ip->uc_protocol != MAC_UDP_PROTOCAL) {
        return OAL_FALSE;
    }
    udp = (udp_hdr_stru *)(ip + 1);
    len      = OAL_NET2HOST_SHORT(udp->us_udp_len);
    dst_port = OAL_NET2HOST_SHORT(udp->us_des_port);
    src_port = OAL_NET2HOST_SHORT(udp->us_src_port);
    if (dst_port != DPERF_BASE_PORT && src_port != DPERF_BASE_PORT) {
        return OAL_FALSE;
    }
    if (len < OAL_SIZEOF(udp_hdr_stru) + OAL_SIZEOF(mac_hiex_rtt_stru) + OAL_SIZEOF(mac_hiex_rtt_stru)) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

oal_bool_enum_uint8 mac_hiex_pst_rx_mark(oal_netbuf_stru *netbuf, skb_header_type_enum hdr_type)
{
    oal_uint8 *ip_header = OAL_PTR_NULL;
    udp_hdr_stru *udp_hdr = OAL_PTR_NULL;
    mac_ieee80211_frame_stru *mac_hdr = OAL_PTR_NULL;

    MAC_HIEX_RX_TRACED(netbuf) = 0;

    /* data at snap header */
    if (hdr_type == SKB_HEADER_SNAP) {
        ip_header = (oal_uint8 *)((mac_llc_snap_stru *)OAL_NETBUF_DATA(netbuf) + 1);
    } else if (hdr_type == SKB_HEADER_ETH) {
        ip_header = (oal_uint8 *)((mac_ether_header_stru *)OAL_NETBUF_DATA(netbuf) + 1);
    } else if ((hdr_type == SKB_HEADER_80211)) {
        mac_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr((mac_rx_ctl_stru *)OAL_NETBUF_CB(netbuf));
        ip_header = (oal_uint8 *)mac_hdr + MAC_GET_RX_CB_MAC_HEADER_LEN((mac_rx_ctl_stru *)OAL_NETBUF_CB(netbuf));
    } else if ((hdr_type == SKB_HEADER_IP)) {
        ip_header = OAL_NETBUF_DATA(netbuf);
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_HIEX, "{mac_hiex_pst_rx_mark::invalid type=%d}", hdr_type);
        return OAL_FALSE;
    }

    if (ip_header == OAL_PTR_NULL || !mac_hiex_is_valid_udp(ip_header)) {
        return OAL_FALSE;
    }

    MAC_HIEX_RX_TRACED(netbuf) = 1;
    udp_hdr = (udp_hdr_stru *)(ip_header + OAL_SIZEOF(mac_ip_header_stru));
    udp_hdr->us_check_sum = 0x0; /* disable udp checksum */

    return OAL_TRUE;
}

oal_bool_enum_uint8 mac_hiex_pst_tx_mark(oal_netbuf_stru *netbuf, skb_header_type_enum hdr_type)
{
    mac_ether_header_stru *ether_hdr = OAL_PTR_NULL;
    udp_hdr_stru          *udp_hdr = OAL_PTR_NULL;

    MAC_HIEX_TX_TRACED(netbuf) = 0;

    /* 待开发:other type */
    if (hdr_type != SKB_HEADER_ETH) {
        OAM_ERROR_LOG0(0, OAM_SF_HIEX, "{mac_hiex_pst_tx_mark::only supprot ether_hdr}");
        return OAL_FALSE;
    }

    /* data at ether header */
    ether_hdr = (mac_ether_header_stru *)OAL_NETBUF_DATA(netbuf);
    if (ether_hdr->us_ether_type != OAL_HOST2NET_SHORT(ETHER_TYPE_IP)) {
        return OAL_FALSE;
    }

    /* 待开发:vlan not supported */
    if (!mac_hiex_is_valid_udp((oal_uint8 *)(ether_hdr + 1))) {
        return OAL_FALSE;
    }

    udp_hdr = (udp_hdr_stru *)((mac_ip_header_stru *)(ether_hdr + 1) + 1);
    udp_hdr->us_check_sum = 0x0; /* disable udp checksum */

    MAC_HIEX_TX_TRACED(netbuf) = 1;

    return OAL_TRUE;
}

oal_uint32 mac_hiex_get_timestamp(mac_vap_stru *vap)
{
#if IS_DEVICE
    return dmac_hiex_get_timestamp(vap);
#else
    oal_time_us_stru st_tst;
    oal_time_get_stamp_us(&st_tst);

    return (oal_uint32)((oal_uint64)st_tst.i_sec * 1000000ull + (oal_uint64)st_tst.i_usec);
#endif
}


oal_void mac_hiex_tst_update(mac_vap_stru *vap, oal_netbuf_stru *skb, skb_header_type_enum hdr_type,
    mac_hiex_tst_point_enum point)
{
    mac_hiex_rtt_stru *rtt = OAL_PTR_NULL;
    oal_uint32         tst;

    if (point >= MAC_HIEX_TST_POINT_CNT) {
        OAM_ERROR_LOG1(0, OAM_SF_HIEX, "invalid tst point=%d", point);
        return;
    }

    if (MAC_HIEX_IS_TX_POINT(point) && !MAC_HIEX_TX_TRACED(skb)) {
        return;
    }

    if (MAC_HIEX_IS_RX_POINT(point) && !MAC_HIEX_RX_TRACED(skb)) {
        return;
    }

    if (MAC_HIEX_IS_DEVICE_POINT(point) && (vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_HIEX, "{mac_hiex_tst_update::VAP==null in device, point=%d}", point);
        return;
    }

    rtt = mac_hiex_get_rtt_stru(skb, hdr_type);
    if (rtt == OAL_PTR_NULL) {
        return;
    }

    tst = mac_hiex_get_timestamp(vap);

    MAC_HIEX_TST_IDX(&rtt->tst, point) = OAL_HOST2NET_LONG(tst);
    MAC_HIEX_TST_MAP(&rtt->tst) |= MAC_HIEX_TST_POINT_BIT(point);
}


oal_void mac_hiex_show_delay(oal_netbuf_stru *netbuf, skb_header_type_enum header_type,
                             mac_hiex_tst_point_enum point_start, mac_hiex_tst_point_enum point_done)
{
    mac_hiex_tst_stru   *tst = OAL_PTR_NULL;
    mac_hiex_rtt_stru   *rtt = OAL_PTR_NULL;
    oal_uint32           seq, start, done;

    if (MAC_HIEX_TX_TRACED(netbuf) || MAC_HIEX_RX_TRACED(netbuf)) {
        rtt  = mac_hiex_get_rtt_stru(netbuf, header_type);
        if (rtt == OAL_PTR_NULL) {
            return;
        }

        tst  = &rtt->tst;

        seq   = OAL_NET2HOST_LONG(rtt->iperf.seq);
        start = MAC_HIEX_TST_IDX(tst, point_start);
        done  = MAC_HIEX_TST_IDX(tst, point_done);
        start = OAL_NET2HOST_LONG(start);
        done  = OAL_NET2HOST_LONG(done);

        OAM_WARNING_LOG1(0, OAM_SF_HIEX, "{mac_hiex_show_tst::map=0x%08x}", MAC_HIEX_TST_MAP(tst));
        if (!(MAC_HIEX_TST_MAP(tst) & MAC_HIEX_TST_POINT_BIT(point_start))
                || !(MAC_HIEX_TST_MAP(tst) & MAC_HIEX_TST_POINT_BIT(point_done))) {
            OAM_ERROR_LOG4(0, OAM_SF_HIEX, "{mac_hiex_show_tst::not all valid:point=%d seq=%d t1=%d t2=%d}",
                point_done, seq, start, done);
        } else {
            OAM_WARNING_LOG3(0, OAM_SF_HIEX, "{HIEX_SHOW_START:point=%d seq=%d delay=%d HIEX_SHOW_DONE}",
                point_done, seq, OAL_DELTA_COUNT32(start, done));
        }
    }
}
#endif /* endif of _PRE_WLAN_FEATURE_HIEX_DBG_TST */
#endif /* endif of _PRE_WLAN_FEATURE_HIEX */
