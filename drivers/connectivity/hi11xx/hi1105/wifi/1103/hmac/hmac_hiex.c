
#ifdef _PRE_WLAN_FEATURE_HIEX
#include "hmac_ext_if.h"
#include "hmac_hiex.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DRIVER_HMAC_HIEX_C

#ifdef HIEX_CHIP_TYPE_1105
#define hcc_msg_tx              frw_event_dispatch_event_etc
#endif


OAL_STATIC hmac_hiex_sii_stru *hmac_hiex_sit_find_oldest(hmac_hiex_sit_stru *table)
{
    oal_uint32   i;
    oal_uint32  idx = 0;
    oal_uint32  now = OAL_TIME_GET_STAMP_MS();
    oal_uint32  max = OAL_DELTA_COUNT32(table->item[0].last_active, now);
    oal_uint32  delta;

    if (table->count == 0) {
        OAM_ERROR_LOG0(0, OAM_SF_HIEX, "{hmac_hiex_sit_find_oldest:invalid count=0}");
        return table->item + idx;
    }

    /* assume count = max, never return null, no need locak */
    for (i = 1; i < table->count; i++) {
        delta = OAL_DELTA_COUNT32(table->item[i].last_active, now);
        if (delta > max) {
            max = delta;
            idx = i;
        }
    }

    return table->item + idx;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_hiex_sit_equal(hmac_hiex_sii_stru *item1, hmac_hiex_sii_stru *item2)
{
    return item1->server_port == item2->server_port &&
           item1->client_port == item2->client_port &&
           item1->server_ip   == item2->server_ip &&
           item1->client_ip   == item2->client_ip ? OAL_TRUE : OAL_FALSE;
}


OAL_STATIC hmac_hiex_sii_stru *hmac_hiex_sit_match(hmac_hiex_sii_stru *item, hmac_hiex_sit_stru *table)
{
    oal_uint32 i;

    for (i = 0; i < table->count; i++) {
        if (hmac_hiex_sit_equal(item, table->item + i)) {
            return table->item + i;
        }
    }

    return OAL_PTR_NULL;
}

oal_uint32 hmac_hiex_sit_aging_handle(void *arg)
{
    hmac_hiex_sit_stru *table = (hmac_hiex_sit_stru *)arg;
    oal_uint32          now = OAL_TIME_GET_STAMP_MS();
    oal_uint32          left = 0;
    oal_uint32          i;

    if (table == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    if (table->count == 0) {    /* safe peek */
        return OAL_SUCC;
    }

    oal_spin_lock_bh(&table->lock);
    for (i = 0; i < table->count; i++) {
        if (OAL_DELTA_COUNT32(table->item[i].last_active, now) < HMAC_HIEX_SII_AGING_MS) {
            if (left != i) {
                table->item[left] = table->item[i];
            }
            left++;
        }
    }

    OAM_WARNING_LOG2(0, OAM_SF_HIEX, "{hmac_hiex_sit_aging_handle:%d aged, %d left.}", table->count - left, left);
    table->count = left;
    oal_spin_unlock_bh(&table->lock);

    return OAL_SUCC;
}

oal_uint32 hmac_hiex_sit_init(hmac_hiex_sit_stru *table, oal_uint32 core_id)
{
    memset_s(table, OAL_SIZEOF(hmac_hiex_sit_stru), 0, OAL_SIZEOF(hmac_hiex_sit_stru));
    oal_spin_lock_init(&table->lock);

    FRW_TIMER_CREATE_TIMER(&table->aging_timer,
                           hmac_hiex_sit_aging_handle,
                           HMAC_HIEX_SII_AGING_INTERVAL_MS,
                           table,
                           OAL_TRUE,
                           OAM_MODULE_ID_HMAC,
                           core_id);
    return OAL_SUCC;
}

OAL_STATIC oal_uint32 hmac_hiex_sit_exit(hmac_hiex_sit_stru *table)
{
    if (table->aging_timer.en_is_registerd == OAL_TRUE) {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&table->aging_timer);
    }

    return OAL_SUCC;
}

oal_uint32 hmac_hiex_user_init(hmac_user_stru *user)
{
    mac_vap_stru                 *vap   = OAL_PTR_NULL;
    mac_hiex_ersru_ctrl_msg_stru *ersru = OAL_PTR_NULL;

    if (user == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    ersru = &user->st_ersru;
    memset_s(ersru, OAL_SIZEOF(mac_hiex_ersru_ctrl_msg_stru), 0, OAL_SIZEOF(mac_hiex_ersru_ctrl_msg_stru));

    vap = mac_res_get_mac_vap(user->st_user_base_info.uc_vap_id);
    if (vap == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    hmac_hiex_sit_init(HMAC_HIEX_GET_USER_SIT(user), vap->ul_core_id);
    return OAL_SUCC;
}

oal_uint32 hmac_hiex_user_exit(mac_vap_stru *pst_mac_vap, hmac_user_stru *user)
{

    if (user == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    if (user->st_ersru.bit_enable) {
        user->st_ersru.bit_enable = OAL_FALSE;
        mac_mib_set_CfgAmpduTxAtive(pst_mac_vap, OAL_TRUE);
    }

    hmac_hiex_sit_exit(HMAC_HIEX_GET_USER_SIT(user));

    return OAL_SUCC;
}


OAL_STATIC oal_bool_enum_uint8 hmac_hiex_sit_add_nolock(hmac_hiex_sii_stru *item, hmac_hiex_sit_stru *table)
{
    hmac_hiex_sii_stru *pos = OAL_PTR_NULL;

    if (table->count > HMAC_HIEX_MAX_SII_SIZE) {
        OAM_ERROR_LOG2(0, OAM_SF_HIEX, "{hmac_hiex_sit_add_nolock:invalid count=%d, max=%d}", table->count,
            HMAC_HIEX_MAX_SII_SIZE);
        return OAL_FALSE;
    } else if (table->count == HMAC_HIEX_MAX_SII_SIZE) {
        pos = hmac_hiex_sit_find_oldest(table);
    } else {
        pos = &table->item[table->count++];
    }

    if (pos == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    if (memcpy_s(pos, OAL_SIZEOF(hmac_hiex_sii_stru), item, OAL_SIZEOF(hmac_hiex_sii_stru)) != EOK) {
        return OAL_FALSE;
    }

    OAM_WARNING_LOG4(0, OAM_SF_HIEX, "{hmac_hiex_sit_add_nolock:add server_ip=0x%x client_ip=0x%x port=0x%08x \
        count=%d}", item->server_ip, item->client_ip,
        (item->server_port << NUM_16_BITS) | item->client_port, table->count);
    pos->last_active = OAL_TIME_GET_STAMP_MS();

    return OAL_TRUE;
}

OAL_STATIC oal_bool_enum_uint8 hmac_hiex_sit_add(hmac_hiex_sii_stru *item, hmac_hiex_sit_stru *table)
{
    oal_bool_enum_uint8 added;

    oal_spin_lock_bh(&table->lock);
    added = hmac_hiex_sit_add_nolock(item, table);
    oal_spin_unlock_bh(&table->lock);

    return added;
}

OAL_STATIC oal_bool_enum_uint8 hmac_hiex_sit_del(hmac_hiex_sii_stru *item, hmac_hiex_sit_stru *table)
{
    hmac_hiex_sii_stru *pos = OAL_PTR_NULL;
    oal_bool_enum_uint8 del = OAL_FALSE;

    oal_spin_lock_bh(&table->lock);

    if (table->count == 0) {
        oal_spin_unlock_bh(&table->lock);
        return OAL_FALSE;
    }

    if (table->count > HMAC_HIEX_MAX_SII_SIZE) {
        table->count = 0;
        oal_spin_unlock_bh(&table->lock);
        return OAL_FALSE;
    }

    pos = hmac_hiex_sit_match(item, table);
    if (pos != OAL_PTR_NULL) {
        OAM_WARNING_LOG3(0, OAM_SF_HIEX, "{hmac_hiex_sit_del:server_ip=0x%x port=0x%x, count=%d}",
                         item->server_ip, item->server_port, table->count - 1);

        if (pos != &table->item[table->count - 1]) {
            *pos = table->item[table->count - 1];
        }

        table->count--;
        del = OAL_TRUE;
    }

    oal_spin_unlock_bh(&table->lock);

    return del ;
}

OAL_STATIC oal_uint32 hmac_hiex_notify_sii(oal_uint8 vap_id, oal_uint16 user_idx,
    hmac_hiex_sii_stru *item, mac_hiex_sii_op_enum op)
{
    oal_uint8 dat[OAL_SIZEOF(mac_hiex_local_msg_stru)
                + OAL_SIZEOF(mac_hiex_msg_stru)
                + OAL_SIZEOF(mac_hiex_sii_report_msg_stru)];
    mac_hiex_local_msg_stru      *msg = (mac_hiex_local_msg_stru *)dat;
    mac_hiex_msg_stru            *ota = (mac_hiex_msg_stru *)(msg + 1);
    mac_hiex_sii_report_msg_stru *rpt = (mac_hiex_sii_report_msg_stru *)(ota + 1);
    mac_vap_stru *vap = mac_res_get_mac_vap(vap_id);

    if (vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(vap_id, OAM_SF_HIEX, "{hmac_hiex_notify_sii:invalid vap id=%d}", vap_id);
        return OAL_FAIL;
    }

    memset_s(dat, OAL_SIZEOF(dat), 0, OAL_SIZEOF(dat));
    msg->type = MAC_HIEX_LOCAL_MSG_TX_ACTION;
    msg->size = OAL_SIZEOF(mac_hiex_msg_stru) + OAL_SIZEOF(mac_hiex_sii_report_msg_stru);
    msg->user_idx = user_idx;

    MAC_HIEX_MSG_TYPE(ota) = HIEX_MSG_TYPE_SII_REPORT;
    MAC_HIEX_MSG_SIZE(ota) = OAL_SIZEOF(mac_hiex_sii_report_msg_stru);

    rpt->bit_op      = op;
    rpt->server_ip   = item->server_ip;
    rpt->client_ip   = item->client_ip;
    rpt->server_port = item->server_port;
    rpt->client_port = item->client_port;
    rpt->mark        = item->mark;

    OAM_WARNING_LOG4(vap_id, OAM_SF_HIEX, "{hmac_hiex_notify_sii:notify peer server ip=0x%x port=0x%x, \
        client ip=0x%x port=0x%x}", item->server_ip, item->server_port, item->client_ip, item->client_port);

    return hmac_hiex_tx_local_msg(vap, msg, OAL_SIZEOF(dat));
}

OAL_STATIC oal_bool_enum_uint8 hmac_hiex_sit_update(hmac_hiex_sii_stru *item, hmac_hiex_sit_stru *table,
    oal_bool_enum_uint8 *add)
{
    oal_bool_enum_uint8 match = OAL_FALSE;
    hmac_hiex_sii_stru *res = OAL_PTR_NULL;

    oal_spin_lock_bh(&table->lock);
    res = hmac_hiex_sit_match(item, table);
    if (res != OAL_PTR_NULL) {
        match = OAL_TRUE;
        *add = OAL_FALSE;
        res->last_active = OAL_TIME_GET_STAMP_MS();
        OAM_INFO_LOG4(0, OAM_SF_HIEX, "{hmac_hiex_sit_update::match sip=0x%x port=0x%x cip=0x%x port=0x%x}",
                      item->server_ip, item->server_port, item->client_ip, item->client_port);
    } else if (*add) {
        *add = hmac_hiex_sit_add_nolock(item, table);
    }

    oal_spin_unlock_bh(&table->lock);

    return (match || *add) ? OAL_TRUE : OAL_FALSE;
}


oal_void hmac_hiex_game_sit_mark(oal_netbuf_stru *skb, mac_vap_stru *vap)
{
    mac_device_stru        *pst_mac_device = OAL_PTR_NULL;

    pst_mac_device = mac_res_get_dev_etc(vap->uc_device_id);
    if (OAL_UNLIKELY(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_hiex_sit_mark::mac_device is NULL}");
        return;
    }

    if (pst_mac_device->ul_hiex_debug_switch != OAL_TRUE) {
        return;
    }

    if (vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        mac_ether_header_stru *_e = (mac_ether_header_stru *)OAL_NETBUF_DATA(skb);
        mac_ip_header_stru    *_ip = (mac_ip_header_stru *)(_e + 1);
        udp_hdr_stru   *_u = (udp_hdr_stru *)(_ip + 1);

        if (_e->us_ether_type == OAL_HOST2NET_SHORT(ETHER_TYPE_IP) && _ip->uc_protocol == MAC_UDP_PROTOCAL) {
            oal_uint16 port = OAL_NET2HOST_SHORT(_u->us_des_port);
            if (port >= DPERF_BASE_PORT && port <= DPERF_END_PORT)
                skb->mark = MAC_HIEX_GAME_MARK;
        }
    }
}


oal_void hmac_hiex_judge_is_game_marked_enter_to_vo(hmac_vap_stru *pst_hmac_vap, mac_user_stru *pst_user,
    oal_netbuf_stru *pst_buf, oal_uint8 *puc_tid)
{
    mac_tx_ctl_stru *pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);

    /* 如果是游戏标记数据，进VO队列 */
    if (hmac_hiex_sit_mark(pst_buf, pst_hmac_vap->st_vap_base_info.uc_vap_id, pst_user->us_assoc_id,
        pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA ? OAL_TRUE : OAL_FALSE)) {
        *puc_tid = WLAN_TIDNO_VOICE;
        /* 标记为游戏业务 */
        MAC_GET_CB_IS_GAME_MARKED_FRAME(pst_tx_ctl) = OAL_TRUE;
    }
}

OAL_STATIC mac_ip_header_stru *hmac_hiex_sit_match_ip_header(oal_netbuf_stru *skb)
{
    mac_ether_header_stru *ether =  (mac_ether_header_stru *)OAL_NETBUF_DATA(skb);
    mac_higame_dc_header_stru *higame = OAL_PTR_NULL;
    mac_ip_header_stru    *ip = OAL_PTR_NULL;

    /* :ipv6 not support; vlan not support */
    /* assume data point at ether header */
    if (ether->us_ether_type == OAL_HOST2NET_SHORT(ETHER_TYPE_IP)) {
        ip = (mac_ip_header_stru *)(ether + 1);
    } else if (ether->us_ether_type == OAL_HOST2NET_SHORT(ETHER_TYPE_HIGAME_DC)) {
        higame = (mac_higame_dc_header_stru *)(ether + 1);
        if (higame->type != OAL_HOST2NET_SHORT(ETHER_TYPE_IP)) {
            return OAL_PTR_NULL;
        }
        ip = (mac_ip_header_stru *)(higame + 1);
    }  else {
        return OAL_PTR_NULL;
    }

    if (ip->uc_protocol != MAC_UDP_PROTOCAL) {
        return OAL_PTR_NULL;
    }

    return ip;
}


oal_bool_enum_uint8 hmac_hiex_sit_mark(oal_netbuf_stru *skb, oal_uint8 vap_id, oal_uint16 user_idx,
    oal_bool_enum_uint8 to_server)
{
    udp_hdr_stru          *udp      = OAL_PTR_NULL;
    mac_ip_header_stru    *ip       = OAL_PTR_NULL;
    hmac_hiex_sit_stru    *table     = OAL_PTR_NULL;
    hmac_user_stru        *user      = OAL_PTR_NULL;
    oal_bool_enum_uint8    add       = to_server ? OAL_TRUE : OAL_FALSE;
    oal_bool_enum_uint8    marked;
    hmac_hiex_sii_stru     item;
    mac_vap_stru           *vap       = mac_res_get_mac_vap(vap_id);

    if (vap == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    hmac_hiex_game_sit_mark(skb, vap);

    /* STA模式下，只根据mark标准识别 */
    if ((vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) && (skb->mark != MAC_HIEX_GAME_MARK)) {
        return OAL_FALSE;
    }

    user = hmac_hiex_get_user(user_idx);
    if (user == OAL_PTR_NULL || !MAC_USER_HIEX_ENABLED(&user->st_user_base_info)) {
        return OAL_FALSE;
    }

    ip = hmac_hiex_sit_match_ip_header(skb);
    if (ip == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    table = HMAC_HIEX_GET_USER_SIT(user);

    /* assume wifi never used as backhaul, and after NAT process */
    memset_s(&item, sizeof(item), 0, sizeof(item));
    udp = (udp_hdr_stru *)(ip + 1);
    if (to_server) {
        item.server_ip   = ip->ul_daddr;
        item.client_ip   = ip->ul_saddr;
        item.server_port = udp->us_des_port;
        item.client_port = udp->us_src_port;
        item.mark = skb->mark;
    } else {
        item.server_ip   = ip->ul_saddr;
        item.client_ip   = ip->ul_daddr;
        item.server_port = udp->us_src_port;
        item.client_port = udp->us_des_port;
        item.mark = skb->mark;
    }

    /* 根据来包标记，STA模式下可以add新条目，AP模式下不add */
    marked = hmac_hiex_sit_update(&item, table, &add);

    if (add && vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_hiex_notify_sii(vap_id, user_idx, &item, MAC_HIEX_SII_ADD);
    }

    return marked;
}


oal_void hmac_hiex_rx_assoc_req(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_uint8 *payload, oal_uint32 payload_len)
{
    mac_hiex_band_stru *band = OAL_PTR_NULL;
    mac_hiex_vendor_ie_stru *ie   = OAL_PTR_NULL;

    if (OAL_IS_NULL_PTR3(hmac_vap, hmac_user, payload)) {
        return;
    }
    memset_s(MAC_USER_GET_HIEX_CAP(&hmac_user->st_user_base_info), OAL_SIZEOF(mac_hiex_cap_stru), 0,
        OAL_SIZEOF(mac_hiex_cap_stru));

    band = hmac_hiex_get_band(hmac_vap);
    if (band == OAL_PTR_NULL) {
        return;
    }

    if (!MAC_BAND_HIEX_ENABLED(band)) {
        OAM_WARNING_LOG1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_HIEX, "{hmac_hiex_rx_assoc_req::band not \
            enabled, flag=0x%x}\r\n", *(oal_uint32 *)MAC_BAND_GET_HIEX_CAP(band));
        return;
    }

    ie = (mac_hiex_vendor_ie_stru *)mac_find_vendor_ie(MAC_WLAN_OUI_HUAWEI, MAC_HISI_HIEX_IE, payload, payload_len);
    if (ie == OAL_PTR_NULL) {
        return;
    }

    if ((oal_uint32)ie->vender.uc_len + MAC_IE_HDR_LEN < sizeof(mac_hiex_vendor_ie_stru)) {
        return;
    }

    if (mac_hiex_nego_cap(MAC_USER_GET_HIEX_CAP(&hmac_user->st_user_base_info),
        MAC_BAND_GET_HIEX_CAP(band), &((mac_hiex_vendor_ie_stru *)ie)->cap) != OAL_SUCC) {
        return;
    }

    OAM_WARNING_LOG4(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_HIEX,
                     "{hmac_hiex_rx_assoc_req::hiex user connected, uid=%d, band_cap=0x%x, ie_cap=0x%x, user_cap=0x%x}",
                     MAC_HIEX_GET_USER_IDX(&hmac_user->st_user_base_info),
                     *(oal_uint32 *)MAC_BAND_GET_HIEX_CAP(band),
                     *(oal_uint32 *)&((mac_hiex_vendor_ie_stru *)ie)->cap,
                     *(oal_uint32 *)MAC_USER_GET_HIEX_CAP(&hmac_user->st_user_base_info));
}

oal_uint32 hmac_hiex_rx_assoc_rsp(hmac_vap_stru *hmac_sta, hmac_user_stru *hmac_user, oal_uint8 *payload,
    oal_uint32 payload_len)
{
    mac_user_stru           *mac_user = OAL_PTR_NULL;
    mac_hiex_band_stru      *band = OAL_PTR_NULL;
    mac_hiex_vendor_ie_stru *ie = OAL_PTR_NULL;

    if (OAL_UNLIKELY(OAL_IS_NULL_PTR3(hmac_sta, hmac_user, payload))) {
        return MAC_NO_CHANGE;
    }
    mac_user = &hmac_user->st_user_base_info;
    memset_s(MAC_USER_GET_HIEX_CAP(mac_user), OAL_SIZEOF(mac_hiex_cap_stru), 0, OAL_SIZEOF(mac_hiex_cap_stru));

    band = hmac_hiex_get_band(hmac_sta);
    if (band == OAL_PTR_NULL) {
        return MAC_NO_CHANGE;
    }

    ie = (mac_hiex_vendor_ie_stru *)mac_find_vendor_ie(MAC_WLAN_OUI_HUAWEI, MAC_HISI_HIEX_IE, payload, payload_len);
    if (ie == OAL_PTR_NULL) {
        return MAC_NO_CHANGE;
    }

    if ((oal_uint32)ie->vender.uc_len + MAC_IE_HDR_LEN < sizeof(mac_hiex_vendor_ie_stru)) {
        return MAC_NO_CHANGE;
    }

    if (mac_hiex_nego_cap(MAC_USER_GET_HIEX_CAP(mac_user), MAC_BAND_GET_HIEX_CAP(band), &ie->cap) != OAL_SUCC) {
        return MAC_NO_CHANGE;
    }

    OAM_WARNING_LOG4(hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_HIEX, "{hmac_hiex_rx_assoc_rsp::connected to \
        hiex ap, uid=%d, band_cap=0x%x, ie_cap=0x%x, user_cap=0x%x}",
        MAC_HIEX_GET_USER_IDX(&hmac_user->st_user_base_info), *(oal_uint32 *)MAC_BAND_GET_HIEX_CAP(band),
        *(oal_uint32 *)&ie->cap, *(oal_uint32 *)MAC_USER_GET_HIEX_CAP(&hmac_user->st_user_base_info));

    return MAC_HIEX_CAP_CHANGE;
}

oal_uint8  hmac_hiex_encap_ie(mac_vap_stru *mac_vap, mac_user_stru *user, oal_uint8 *frame)
{
    mac_hiex_band_stru             *band = OAL_PTR_NULL;
    hmac_vap_stru                  *hvap = OAL_PTR_NULL;
    oal_uint8                       size = 0;
    mac_hiex_cap_stru              *user_cap = OAL_PTR_NULL;

    hvap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hvap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ASSOC, "{hmac_hiex_encap_ie::hmac_vap ptr null, vid=%d}", mac_vap->uc_vap_id);
        return size;
    }

    band  = hmac_hiex_get_band(hvap);
    if (band == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ASSOC, "{hmac_hiex_encap_ie::band ptr null}");
        return size;
    }

    if (user != OAL_PTR_NULL) {
        if (!MAC_USER_HIEX_ENABLED(user)) {
            return size;
        }
        user_cap = MAC_USER_GET_HIEX_CAP(user);
    }

    return mac_hiex_set_vendor_ie(MAC_BAND_GET_HIEX_CAP(band), user_cap, frame);
}

OAL_STATIC oal_uint32 hmac_hiex_rx_sii_report(hmac_vap_stru *vap, hmac_user_stru *user, mac_hiex_local_msg_stru *msg)
{
    mac_hiex_msg_stru            *ota = (mac_hiex_msg_stru *)(msg + 1);
    mac_hiex_sii_report_msg_stru *rpt = (mac_hiex_sii_report_msg_stru *)(ota + 1);
    hmac_hiex_sii_stru            item;
    oal_bool_enum_uint8           ret = OAL_FALSE;

    if (msg->size != sizeof(mac_hiex_msg_stru) + sizeof(mac_hiex_sii_report_msg_stru)) {
        OAM_WARNING_LOG2(0, OAM_SF_HIEX, "{hmac_hiex_rx_sii_report::invalid size=%d expect=%d}",
            msg->size, sizeof(mac_hiex_msg_stru) + sizeof(mac_hiex_sii_report_msg_stru));
        return OAL_FAIL;
    }

    memset_s(&item, sizeof(hmac_hiex_sii_stru), 0, sizeof(hmac_hiex_sii_stru));
    item.client_ip   = rpt->client_ip;
    item.server_ip   = rpt->server_ip;
    item.client_port = rpt->client_port;
    item.server_port = rpt->server_port;
    item.mark        = rpt->mark;

    if (rpt->bit_op == MAC_HIEX_SII_DEL) {
        ret = hmac_hiex_sit_del(&item, HMAC_HIEX_GET_USER_SIT(user));
    } else {
        ret = hmac_hiex_sit_add(&item, HMAC_HIEX_GET_USER_SIT(user));
    }

    return ret ? OAL_SUCC : OAL_FAIL;
}


OAL_STATIC oal_uint32 hmac_hiex_rx_ersru_ctrl(hmac_vap_stru *vap, hmac_user_stru *user, mac_hiex_local_msg_stru *msg)
{
    mac_hiex_msg_stru            *ota = (mac_hiex_msg_stru *)(msg + 1);
    mac_hiex_ersru_ctrl_msg_stru *rpt = (mac_hiex_ersru_ctrl_msg_stru *)(ota + 1);

    if (msg->size != sizeof(mac_hiex_msg_stru) + sizeof(mac_hiex_ersru_ctrl_msg_stru)) {
        OAM_WARNING_LOG2(0, OAM_SF_HIEX, "{hmac_hiex_rx_ersru_report::invalid size=%d expect=%d}",
            msg->size, sizeof(mac_hiex_msg_stru) + sizeof(mac_hiex_ersru_ctrl_msg_stru));
        return OAL_FAIL;
    }

    /* 标记是否进入extend range small resource unit
     * 1: 删除用户聚合并且分片; 0：保持默认,用户可以建立聚合并且不分片
     */
    user->st_ersru.bit_enable = rpt->bit_enable;

    if (rpt->bit_enable) {
        /* 保证最小值 */
        user->st_ersru.bit_frag_len = OAL_HOST2NET_SHORT(rpt->bit_frag_len);
        user->st_ersru.bit_frag_len = OAL_MAX(user->st_ersru.bit_frag_len, MAC_HIEX_ERSRU_MINI_FRAG_LEN);

        /* 关闭聚合 */
        mac_mib_set_CfgAmpduTxAtive(&vap->st_vap_base_info, OAL_FALSE);
    } else {
        /* 清零,避免异常分片 */
        user->st_ersru.bit_frag_len = 0;

        /* 开启聚合 */
        mac_mib_set_CfgAmpduTxAtive(&vap->st_vap_base_info, OAL_TRUE);
    }

    OAM_WARNING_LOG2(0, OAM_SF_HIEX, "{hmac_hiex_rx_ersru_ctrl::enable=%d frag len=%d}",
        rpt->bit_enable, user->st_ersru.bit_frag_len);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_hiex_handle_rx_action(hmac_vap_stru *vap,
                                                 hmac_user_stru *user,
                                                 mac_hiex_local_msg_stru *msg)
{
    mac_hiex_msg_stru *ota = (mac_hiex_msg_stru *)(msg + 1);

    switch (ota->type) {
        case HIEX_MSG_TYPE_SII_REPORT:
            hmac_hiex_rx_sii_report(vap, user, msg);
            break;
        case HIEX_MSG_TYPE_ERSRU_CTRL:
            hmac_hiex_rx_ersru_ctrl(vap, user, msg);
            break;
        default:
            break;
    }
    return OAL_SUCC;
}

oal_uint32 hmac_hiex_rx_local_msg(hcc_msg_mem_stru *mem)
{
    mac_hiex_local_msg_stru *msg = OAL_PTR_NULL;
    hcc_msg_stru            *hcc_msg = OAL_PTR_NULL;
    hmac_vap_stru           *vap = OAL_PTR_NULL;
    hmac_user_stru          *user = OAL_PTR_NULL;

    if (mem == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    hcc_msg = HCC_MSG_GET_STRU(mem);
    vap = mac_res_get_hmac_vap(hcc_msg->st_event_hdr.uc_vap_id);
    if (vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_HIEX, "{hmac_hiex_rx_local_msg::invalid vap id=%d}",
            hcc_msg->st_event_hdr.uc_vap_id);
        return OAL_FAIL;
    }

    msg = (mac_hiex_local_msg_stru *)hcc_msg->auc_event_data;
    user = hmac_hiex_get_user(msg->user_idx);
    if (user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(vap->st_vap_base_info.uc_vap_id, OAM_SF_HIEX,
            "{hmac_hiex_rx_local_msg::invalid user id=%d}", msg->user_idx);
        return OAL_FAIL;
    }

    if (!MAC_USER_HIEX_ENABLED(&user->st_user_base_info)) {
        return OAL_FAIL;
    }

    OAM_WARNING_LOG2(vap->st_vap_base_info.uc_vap_id, OAM_SF_HIEX,
        "{hmac_hiex_rx_local_msg::recv msg from device, type=%d size=%d}", msg->type, msg->size);

    switch (msg->type) {
        case MAC_HIEX_LOCAL_MSG_RX_ACTION:
            hmac_hiex_handle_rx_action(vap, user, msg);
            break;

        default:
            break;
    }

    return OAL_SUCC;
}

oal_uint32 hmac_hiex_tx_local_msg(mac_vap_stru *vap, mac_hiex_local_msg_stru *msg, oal_uint32 size)
{
    oal_uint32              ret;
    hcc_msg_mem_stru       *mem = OAL_PTR_NULL;
    hcc_msg_stru           *event = OAL_PTR_NULL;

    if (OAL_IS_NULL_PTR2(vap, msg) || (size == 0)) {
        return OAL_FAIL;
    }

    if (size != msg->size + OAL_SIZEOF(mac_hiex_local_msg_stru)) {
        OAM_ERROR_LOG2(0, OAM_SF_HIEX, "{hmac_hiex_tx_local_msg::size mismatch, tx=%d msg=%d.}", size, msg->size);
        return OAL_FAIL;
    }

    mem = HCC_MSG_ALLOC(size);
    if (mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(vap->uc_vap_id, OAM_SF_HIEX, "{hmac_hiex_tx_local_msg::alloc mem failed, size=%d}", size);
        return OAL_ERR_CODE_PTR_NULL;
    }

    event = HCC_MSG_GET_STRU(mem);
    HCC_MSG_HDR_INIT_WLAN(&event->st_event_hdr,
#ifdef HIEX_CHIP_TYPE_1105
                          FRW_EVENT_TYPE_WLAN_CTX,
                          DMAC_WLAN_CTX_EVENT_SUB_TYPE_HIEX_H2D_MSG,
                          size,
                          FRW_EVENT_PIPELINE_STAGE_1,
                          vap->uc_chip_id,
                          vap->uc_device_id,
#else
                          MSG_H2D_HIEX_MSG,
                          vap->uc_chip_id,
                          vap->uc_mac_device_id,
#endif
                          vap->uc_vap_id);

    if (memcpy_s(event->auc_event_data, size, msg, size) != EOK) {
        return OAL_FAIL;
    }

    ret = hcc_msg_tx(mem);
    if (ret != OAL_SUCC) {
        OAM_WARNING_LOG1(vap->uc_vap_id, OAM_SF_HIEX, "{hmac_hiex_tx_local_msg::hcc_msg_tx failed[%d].}", ret);
        HCC_MSG_FREE(mem);
    }

    OAM_WARNING_LOG2(vap->uc_vap_id, OAM_SF_HIEX, "{hmac_hiex_tx_local_msg::send msg to device, type=%d size=%d}",
                     msg->type, msg->size);

#ifdef HIEX_CHIP_TYPE_1105
    HCC_MSG_FREE(mem);
#endif

    return ret;
}

#endif
