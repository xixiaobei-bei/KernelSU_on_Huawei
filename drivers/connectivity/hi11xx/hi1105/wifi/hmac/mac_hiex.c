

#ifdef _PRE_WLAN_FEATURE_HIEX
#include "mac_hiex.h"

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
#ifdef _HIEX_CHIP_TYPE_110X
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
#ifdef _HIEX_CHIP_TYPE_110X
    MAC_HIEX_CHIP_TYPE_HI1105,    // bit_chip_type
#else
    MAC_HIEX_CHIP_TYPE_HI1152,    // bit_chip_type
#endif
    0,                            // bit_resv
#endif
};


mac_hiex_mgr_stru *mac_hiex_get_mgr(void)
{
    return &g_st_hiex_mgr;
}


OAL_STATIC oal_bool_enum_uint8 mac_hiex_cap_compatible(mac_hiex_cap_stru *local, mac_hiex_cap_stru *peer)
{
    if (local->bit_himit_enable && peer->bit_himit_enable &&
        local->bit_he_htc_himit_id != peer->bit_he_htc_himit_id) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

uint32_t mac_hiex_nego_cap(mac_hiex_cap_stru *target, mac_hiex_cap_stru *local, mac_hiex_cap_stru *peer)
{
    mac_hiex_cap_stru  *tmp = NULL;

    if (target == NULL) {
        return OAL_FAIL;
    }

    if (memset_s(target, sizeof(mac_hiex_cap_stru), 0, sizeof(mac_hiex_cap_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_HIEX, "{mac_hiex_nego_cap::memset_s failed}");
        return OAL_FAIL;
    }

    /* 无对端设置，直接用本地能力，如封装beacon、probe request/rsp 、assoc request/rsp */
    if (local != NULL && peer == NULL) {
        tmp = local;
    } else if (local != NULL && peer != NULL) {
        if (!mac_hiex_cap_compatible(local, peer)) {
            return OAL_FAIL;
        }
        tmp = peer;
    } else {
        oam_warning_log0(0, OAM_SF_HIEX, "{mac_hiex_nego_cap::invalid config}");
        return OAL_FAIL;
    }

    target->bit_hiex_enable      = tmp->bit_hiex_enable && local->bit_hiex_enable;
    target->bit_hiex_version     = oal_min(tmp->bit_hiex_version, local->bit_hiex_version);
    target->bit_himit_enable     = tmp->bit_himit_enable && local->bit_himit_enable;
    target->bit_ht_himit_enable  = tmp->bit_ht_himit_enable && local->bit_ht_himit_enable;
    target->bit_vht_himit_enable = tmp->bit_vht_himit_enable && local->bit_vht_himit_enable;
    target->bit_he_himit_enable  = tmp->bit_he_himit_enable && local->bit_he_himit_enable;
    target->bit_he_htc_himit_id  = tmp->bit_he_htc_himit_id;
    target->bit_himit_version    = oal_min(tmp->bit_himit_version, local->bit_himit_version);
    target->bit_chip_type        = tmp->bit_chip_type;
    /* 不协商,本地能力 */
    target->bit_ersru_enable     = local->bit_ersru_enable;

    return OAL_SUCC;
}

uint8_t mac_hiex_set_vendor_ie(mac_hiex_cap_stru *band_cap, mac_hiex_cap_stru *user_cap, uint8_t *buffer)
{
    mac_hiex_vendor_ie_stru *vendor_ie = NULL;
    mac_hiex_cap_stru       *ie_cap = NULL;
    uint8_t                size = 0;

    if (!band_cap->bit_hiex_enable || (user_cap && !user_cap->bit_hiex_enable)) {
        return size;
    }

    vendor_ie = (mac_hiex_vendor_ie_stru *)buffer;
    vendor_ie->vender.uc_element_id = MAC_EID_VENDOR;
    vendor_ie->vender.uc_len = sizeof(mac_hiex_vendor_ie_stru) - MAC_IE_HDR_LEN;

    vendor_ie->vender.auc_oui[0] = (uint8_t)(((uint32_t)MAC_WLAN_OUI_HUAWEI >> NUM_16_BITS) & 0xff); //lint !e572
    vendor_ie->vender.auc_oui[1] = (uint8_t)(((uint32_t)MAC_WLAN_OUI_HUAWEI >> NUM_8_BITS) & 0xff);
    vendor_ie->vender.auc_oui[2] = (uint8_t)(((uint32_t)MAC_WLAN_OUI_HUAWEI) & 0xff);  /* hiex oui 数组2 */

    vendor_ie->vender.uc_oui_type = MAC_HISI_HIEX_IE;

    ie_cap = &vendor_ie->cap;
    if (mac_hiex_nego_cap(ie_cap, band_cap, user_cap) != OAL_SUCC) {
        return size;
    }

    return sizeof(mac_hiex_vendor_ie_stru);
}

#ifdef _PRE_WLAN_FEATURE_HIEX_DBG_TST

oal_bool_enum_uint8 mac_hiex_is_valid_udp(uint8_t *ip_header)
{
    mac_ip_header_stru    *ip = (mac_ip_header_stru *)ip_header;
    udp_hdr_stru          *udp = NULL;
    uint16_t             len, src_port, dst_port;

    if (ip == NULL || ip->uc_protocol != MAC_UDP_PROTOCAL) {
        return OAL_FALSE;
    }
    udp = (udp_hdr_stru *)(ip + 1);
    len      = oal_net2host_short(udp->us_udp_len);
    dst_port = oal_net2host_short(udp->us_des_port);
    src_port = oal_net2host_short(udp->us_src_port);
    if (dst_port != DPERF_BASE_PORT && src_port != DPERF_BASE_PORT) {
        return OAL_FALSE;
    }
    if (len < sizeof(udp_hdr_stru) + sizeof(mac_hiex_rtt_stru) + sizeof(mac_hiex_rtt_stru)) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

oal_bool_enum_uint8 mac_hiex_pst_rx_mark(oal_netbuf_stru *netbuf, skb_header_type_enum hdr_type)
{
    uint8_t *ip_header = NULL;
    udp_hdr_stru *udp_hdr = NULL;
    mac_ieee80211_frame_stru *mac_hdr = NULL;

    mac_hiex_rx_traced(netbuf) = 0;

    /* data at snap header */
    if (hdr_type == SKB_HEADER_SNAP) {
        ip_header = (uint8_t *)((mac_llc_snap_stru *)oal_netbuf_data(netbuf) + 1);
    } else if (hdr_type == SKB_HEADER_ETH) {
        ip_header = (uint8_t *)((mac_ether_header_stru *)oal_netbuf_data(netbuf) + 1);
    } else if ((hdr_type == SKB_HEADER_80211)) {
        mac_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr((mac_rx_ctl_stru *)oal_netbuf_cb(netbuf));
        ip_header = (uint8_t *)mac_hdr + MAC_GET_RX_CB_MAC_HEADER_LEN((mac_rx_ctl_stru *)oal_netbuf_cb(netbuf));
    } else if ((hdr_type == SKB_HEADER_IP)) {
        ip_header = oal_netbuf_data(netbuf);
    } else {
        oam_warning_log1(0, OAM_SF_HIEX, "{mac_hiex_pst_rx_mark::invalid type=%d}", hdr_type);
        return OAL_FALSE;
    }

    if (ip_header == NULL || !mac_hiex_is_valid_udp(ip_header)) {
        return OAL_FALSE;
    }

    mac_hiex_rx_traced(netbuf) = 1;
    udp_hdr = (udp_hdr_stru *)(ip_header + sizeof(mac_ip_header_stru));
    udp_hdr->us_check_sum = 0x0; /* disable udp checksum */

    return OAL_TRUE;
}

oal_bool_enum_uint8 mac_hiex_pst_tx_mark(oal_netbuf_stru *netbuf, skb_header_type_enum hdr_type)
{
    mac_ether_header_stru *ether_hdr = NULL;
    udp_hdr_stru          *udp_hdr = NULL;

    mac_hiex_tx_traced(netbuf) = 0;

    /* 待开发:other type */
    if (hdr_type != SKB_HEADER_ETH) {
        oam_error_log0(0, OAM_SF_HIEX, "{mac_hiex_pst_tx_mark::only supprot ether_hdr}");
        return OAL_FALSE;
    }

    /* data at ether header */
    ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(netbuf);
    if (ether_hdr->us_ether_type != oal_host2net_short(ETHER_TYPE_IP)) {
        return OAL_FALSE;
    }

    /* 待开发:vlan not supported */
    if (!mac_hiex_is_valid_udp((uint8_t *)(ether_hdr + 1))) {
        return OAL_FALSE;
    }

    udp_hdr = (udp_hdr_stru *)((mac_ip_header_stru *)(ether_hdr + 1) + 1);
    udp_hdr->us_check_sum = 0x0; /* disable udp checksum */

    mac_hiex_tx_traced(netbuf) = 1;

    return OAL_TRUE;
}

uint32_t mac_hiex_get_timestamp(mac_vap_stru *vap)
{
    oal_time_us_stru st_tst;
    oal_time_get_stamp_us(&st_tst);

    return (uint32_t)((uint64_t)st_tst.i_sec * 1000000ull + (uint64_t)st_tst.i_usec);
}


void mac_hiex_tst_update(mac_vap_stru *vap, oal_netbuf_stru *skb, skb_header_type_enum hdr_type,
    mac_hiex_tst_point_enum point)
{
    mac_hiex_rtt_stru *rtt = NULL;
    uint32_t         tst;

    if (point >= MAC_HIEX_TST_POINT_CNT) {
        oam_error_log1(0, OAM_SF_HIEX, "invalid tst point=%d", point);
        return;
    }

    if (mac_hiex_is_tx_point(point) && !mac_hiex_tx_traced(skb)) {
        return;
    }

    if (mac_hiex_is_rx_point(point) && !mac_hiex_rx_traced(skb)) {
        return;
    }

    if (mac_hiex_is_device_point(point) && (vap == NULL)) {
        oam_error_log1(0, OAM_SF_HIEX, "{mac_hiex_tst_update::VAP==null in device, point=%d}", point);
        return;
    }

    rtt = mac_hiex_get_rtt_stru(skb, hdr_type);
    if (rtt == NULL) {
        return;
    }

    tst = mac_hiex_get_timestamp(vap);

    mac_hiex_tst_idx(&rtt->tst, point) = oal_host2net_long(tst);
    mac_hiex_tst_map(&rtt->tst) |= mac_hiex_tst_point_bit(point);
}


void mac_hiex_show_delay(oal_netbuf_stru *netbuf, skb_header_type_enum header_type,
    mac_hiex_tst_point_enum point_start, mac_hiex_tst_point_enum point_done)
{
    mac_hiex_tst_stru   *tst = NULL;
    mac_hiex_rtt_stru   *rtt = NULL;
    uint32_t           seq, start, done;

    if (mac_hiex_tx_traced(netbuf) || mac_hiex_rx_traced(netbuf)) {
        rtt  = mac_hiex_get_rtt_stru(netbuf, header_type);
        if (rtt == NULL) {
            return;
        }

        tst  = &rtt->tst;

        seq   = oal_net2host_long(rtt->iperf.seq);
        start = mac_hiex_tst_idx(tst, point_start);
        done  = mac_hiex_tst_idx(tst, point_done);
        start = oal_net2host_long(start);
        done  = oal_net2host_long(done);

        oam_warning_log1(0, OAM_SF_HIEX, "{mac_hiex_show_tst::map=0x%08x}", mac_hiex_tst_map(tst));
        if (!(mac_hiex_tst_map(tst) & mac_hiex_tst_point_bit(point_start))
                || !(mac_hiex_tst_map(tst) & mac_hiex_tst_point_bit(point_done))) {
            oam_error_log4(0, OAM_SF_HIEX, "{mac_hiex_show_tst::not all valid:point=%d seq=%d t1=%d t2=%d}",
                point_done, seq, start, done);
        } else {
            oam_warning_log3(0, OAM_SF_HIEX, "{HIEX_SHOW_START:point=%d seq=%d delay=%d HIEX_SHOW_DONE}",
                point_done, seq, oal_delta_count32(start, done));
        }
    }
}
#endif /* endif of _PRE_WLAN_FEATURE_HIEX_DBG_TST */
#endif /* endif of _PRE_WLAN_FEATURE_HIEX */
