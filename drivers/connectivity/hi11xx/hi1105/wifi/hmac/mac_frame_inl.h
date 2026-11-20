

#ifndef __MAC_FRAME_INL_H__
#define __MAC_FRAME_INL_H__

#include "mac_frame.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_FRAME_INL_H

OAL_STATIC OAL_INLINE uint8_t mac_rsn_ie_akm_match_suites_s(uint32_t *pul_suites,
                                                            uint8_t uc_cipher_len,
                                                            uint32_t aul_akm_suite)
{
    uint8_t uc_idx_peer;

    for (uc_idx_peer = 0; uc_idx_peer < uc_cipher_len / sizeof(uint32_t); uc_idx_peer++) {
        if (pul_suites[uc_idx_peer] == aul_akm_suite) {
            return uc_idx_peer;
        }
    }
    return 0xff;
}


OAL_STATIC OAL_INLINE void mac_hdr_set_frame_control(uint8_t *puc_header, uint16_t us_fc)
{
    *(uint16_t *)puc_header = us_fc;
}


OAL_STATIC OAL_INLINE void mac_hdr_set_fragment_number(uint8_t *puc_header, uint8_t uc_frag_num)
{
    puc_header[WLAN_HDR_FRAG_OFFSET] &= 0xF0;
    puc_header[WLAN_HDR_FRAG_OFFSET] |= (uc_frag_num & 0x0F);
}


OAL_STATIC OAL_INLINE void mac_hdr_set_from_ds(uint8_t *puc_header, uint8_t uc_from_ds)
{
    ((mac_header_frame_control_stru *)(puc_header))->bit_from_ds = uc_from_ds;
}


OAL_STATIC OAL_INLINE uint8_t mac_hdr_get_from_ds(uint8_t *puc_header)
{
    return (uint8_t)((mac_header_frame_control_stru *)(puc_header))->bit_from_ds;
}


OAL_STATIC OAL_INLINE void mac_hdr_set_to_ds(uint8_t *puc_header, uint8_t uc_to_ds)
{
    ((mac_header_frame_control_stru *)(puc_header))->bit_to_ds = uc_to_ds;
}


OAL_STATIC OAL_INLINE uint8_t mac_hdr_get_to_ds(uint8_t *puc_header)
{
    return (uint8_t)((mac_header_frame_control_stru *)(puc_header))->bit_to_ds;
}


OAL_STATIC OAL_INLINE uint8_t mac_get_tid_value(uint8_t *puc_header, oal_bool_enum_uint8 en_is_4addr)
{
    if (en_is_4addr) {
        return (puc_header[MAC_QOS_CTRL_FIELD_OFFSET_4ADDR] & 0x07); /* B0 - B2 */
    } else {
        return (puc_header[MAC_QOS_CTRL_FIELD_OFFSET] & 0x07); /* B0 - B2 */
    }
}


OAL_STATIC OAL_INLINE uint16_t mac_get_seq_num(uint8_t *puc_header)
{
    uint16_t us_seq_num;

    us_seq_num = puc_header[BYTE_OFFSET_23];
    us_seq_num <<= BIT_OFFSET_4;
    us_seq_num |= (puc_header[BYTE_OFFSET_22] >> BIT_OFFSET_4);

    return us_seq_num;
}


OAL_STATIC OAL_INLINE uint16_t mac_get_bar_start_seq_num(uint8_t *puc_payload)
{
    return ((puc_payload[BYTE_OFFSET_2] & 0xF0) >> BIT_OFFSET_4) | (puc_payload[BYTE_OFFSET_3] << BIT_OFFSET_4);
}


OAL_STATIC OAL_INLINE void mac_rx_get_da(mac_ieee80211_frame_stru *pst_mac_header,
                                         uint8_t **puc_da)
{
    /* IBSS、from AP */
    if (pst_mac_header->st_frame_control.bit_to_ds == 0) {
        *puc_da = pst_mac_header->auc_address1;
    /* WDS、to AP */
    } else {
        *puc_da = pst_mac_header->auc_address3;
    }
}


OAL_STATIC OAL_INLINE void mac_rx_get_sa(mac_ieee80211_frame_stru *pst_mac_header,
                                         uint8_t **puc_sa)
{
    /* IBSS、to AP */
    if (pst_mac_header->st_frame_control.bit_from_ds == 0) {
        *puc_sa = pst_mac_header->auc_address2;
    /* from AP */
    } else if ((pst_mac_header->st_frame_control.bit_from_ds == 1) &&
             (pst_mac_header->st_frame_control.bit_to_ds == 0)) {
        *puc_sa = pst_mac_header->auc_address3;
    /* WDS */
    } else {
        *puc_sa = ((mac_ieee80211_frame_addr4_stru *)pst_mac_header)->auc_address4;
    }
}


OAL_STATIC OAL_INLINE void mac_get_transmit_addr(mac_ieee80211_frame_stru *pst_mac_header,
                                                 uint8_t **puc_bssid)
{
    /* 对于IBSS, STA, AP, WDS 场景下，获取发送端地址 */
    *puc_bssid = pst_mac_header->auc_address2;
}


OAL_STATIC OAL_INLINE void mac_get_submsdu_len(uint8_t *puc_submsdu_hdr, uint16_t *pus_submsdu_len)
{
    *pus_submsdu_len = *(puc_submsdu_hdr + MAC_SUBMSDU_LENGTH_OFFSET);
    *pus_submsdu_len = (uint16_t)((*pus_submsdu_len << BIT_OFFSET_8) +
        *(puc_submsdu_hdr + MAC_SUBMSDU_LENGTH_OFFSET + 1));
}


OAL_STATIC OAL_INLINE void mac_get_submsdu_pad_len(uint16_t us_msdu_len, uint8_t *puc_submsdu_pad_len)
{
    *puc_submsdu_pad_len = us_msdu_len & 0x3;

    if (*puc_submsdu_pad_len) {
        *puc_submsdu_pad_len = (MAC_BYTE_ALIGN_VALUE - *puc_submsdu_pad_len);
    }
}


OAL_STATIC OAL_INLINE uint8_t mac_get_frame_type_and_subtype(uint8_t *puc_mac_header)
{
    return (puc_mac_header[0] & 0xFC);
}


OAL_STATIC OAL_INLINE uint8_t mac_get_frame_sub_type(uint8_t *puc_mac_header)
{
    return (puc_mac_header[0] & 0xF0);
}


OAL_STATIC OAL_INLINE uint8_t mac_frame_get_subtype_value(uint8_t *puc_mac_header)
{
    return ((puc_mac_header[0] & 0xF0) >> BIT_OFFSET_4);
}


OAL_STATIC OAL_INLINE uint8_t mac_get_frame_type(uint8_t *puc_mac_header)
{
    return (puc_mac_header[0] & 0x0C);
}


OAL_STATIC OAL_INLINE uint8_t mac_frame_get_type_value(uint8_t *puc_mac_header)
{
    return (puc_mac_header[0] & 0x0C) >> BIT_OFFSET_2;
}


OAL_STATIC OAL_INLINE void mac_set_snap(oal_netbuf_stru *pst_buf,
                                        uint16_t us_ether_type)
{
    mac_llc_snap_stru *pst_llc;
    uint16_t uc_use_btep1;
    uint16_t uc_use_btep2;

    /* LLC */
    pst_llc = (mac_llc_snap_stru *)(oal_netbuf_data(pst_buf) + ETHER_HDR_LEN - SNAP_LLC_FRAME_LEN);
    pst_llc->uc_llc_dsap = SNAP_LLC_LSAP;
    pst_llc->uc_llc_ssap = SNAP_LLC_LSAP;
    pst_llc->uc_control = LLC_UI;

    uc_use_btep1 = oal_byteorder_host_to_net_uint16(ETHER_TYPE_AARP);
    uc_use_btep2 = oal_byteorder_host_to_net_uint16(ETHER_TYPE_IPX);
    if (oal_unlikely((uc_use_btep1 == us_ether_type) || (uc_use_btep2 == us_ether_type))) {
        pst_llc->auc_org_code[BYTE_OFFSET_0] = SNAP_BTEP_ORGCODE_0; /* 0x0 */
        pst_llc->auc_org_code[BYTE_OFFSET_1] = SNAP_BTEP_ORGCODE_1; /* 0x0 */
        pst_llc->auc_org_code[BYTE_OFFSET_2] = SNAP_BTEP_ORGCODE_2; /* 0xf8 */
    } else {
        pst_llc->auc_org_code[BYTE_OFFSET_0] = SNAP_RFC1042_ORGCODE_0; /* 0x0 */
        pst_llc->auc_org_code[BYTE_OFFSET_1] = SNAP_RFC1042_ORGCODE_1; /* 0x0 */
        pst_llc->auc_org_code[BYTE_OFFSET_2] = SNAP_RFC1042_ORGCODE_2; /* 0x0 */
    }

    pst_llc->us_ether_type = us_ether_type;

    oal_netbuf_pull(pst_buf, (ETHER_HDR_LEN - SNAP_LLC_FRAME_LEN));
}


OAL_STATIC OAL_INLINE uint16_t mac_get_auth_alg(uint8_t *puc_mac_hdr)
{
    uint16_t us_auth_alg;

    us_auth_alg = puc_mac_hdr[MAC_80211_FRAME_LEN + 1];
    us_auth_alg = (uint16_t)((us_auth_alg << BIT_OFFSET_8) | puc_mac_hdr[MAC_80211_FRAME_LEN + 0]);

    return us_auth_alg;
}


OAL_STATIC OAL_INLINE uint16_t mac_get_auth_status(uint8_t *puc_mac_hdr)
{
    uint16_t us_auth_status;

    us_auth_status = puc_mac_hdr[MAC_80211_FRAME_LEN + BYTE_OFFSET_5];
    us_auth_status = (uint16_t)((us_auth_status << BIT_OFFSET_8) | puc_mac_hdr[MAC_80211_FRAME_LEN + BYTE_OFFSET_4]);

    return us_auth_status;
}


OAL_STATIC OAL_INLINE uint16_t mac_get_ft_status(uint8_t *puc_mac_hdr)
{
    uint16_t us_auth_status;

    /* 读取ft rsp中的ft status, 相对header的偏移量为14个字节 */
    us_auth_status = puc_mac_hdr[MAC_80211_FRAME_LEN + BYTE_OFFSET_15];
    us_auth_status = (uint16_t)((us_auth_status << BIT_OFFSET_8) | puc_mac_hdr[MAC_80211_FRAME_LEN + BYTE_OFFSET_14]);

    return us_auth_status;
}


OAL_STATIC OAL_INLINE uint16_t mac_get_auth_seq_num(uint8_t *puc_mac_hdr)
{
    uint16_t us_auth_seq;

    us_auth_seq = puc_mac_hdr[MAC_80211_FRAME_LEN + BYTE_OFFSET_3];
    us_auth_seq = (uint16_t)((us_auth_seq << BIT_OFFSET_8) | puc_mac_hdr[MAC_80211_FRAME_LEN + BYTE_OFFSET_2]);

    return us_auth_seq;
}


OAL_STATIC OAL_INLINE void mac_set_wep(uint8_t *puc_hdr, uint8_t uc_wep)
{
    puc_hdr[1] &= 0xBF;
    puc_hdr[1] |= (uint8_t)(uc_wep << BIT_OFFSET_6);
}


OAL_STATIC OAL_INLINE void mac_set_protectedframe(uint8_t *puc_mac_hdr)
{
    puc_mac_hdr[1] |= 0x40;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_protectedframe(uint8_t *puc_mac_hdr)
{
    mac_ieee80211_frame_stru *pst_mac_hdr = NULL;
    pst_mac_hdr = (mac_ieee80211_frame_stru *)puc_mac_hdr;

    return (oal_bool_enum_uint8)(pst_mac_hdr->st_frame_control.bit_protected_frame);
}


OAL_STATIC OAL_INLINE uint16_t mac_get_auth_algo_num(oal_netbuf_stru *pst_netbuf)
{
    uint16_t us_auth_algo;
    uint8_t *puc_mac_payload = oal_netbuf_data(pst_netbuf) + MAC_80211_FRAME_LEN;

    us_auth_algo = puc_mac_payload[1];
    us_auth_algo = (uint16_t)((us_auth_algo << BIT_OFFSET_8) | puc_mac_payload[0]);

    return us_auth_algo;
}


OAL_STATIC OAL_INLINE uint8_t *mac_get_auth_ch_text(uint8_t *puc_mac_hdr)
{
    return &(puc_mac_hdr[MAC_80211_FRAME_LEN + BYTE_OFFSET_6]);
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_4addr(uint8_t *puc_mac_hdr)
{
    uint8_t uc_is_tods;
    uint8_t uc_is_from_ds;
    oal_bool_enum_uint8 en_is_4addr;

    uc_is_tods = mac_hdr_get_to_ds(puc_mac_hdr);
    uc_is_from_ds = mac_hdr_get_from_ds(puc_mac_hdr);

    en_is_4addr = uc_is_tods && uc_is_from_ds;

    return en_is_4addr;
}


OAL_STATIC OAL_INLINE void mac_get_addr1(uint8_t *puc_mac_hdr, uint8_t *puc_addr, uint8_t uc_addr_len)
{
    memcpy_s(puc_addr, uc_addr_len, puc_mac_hdr + BYTE_OFFSET_4, WLAN_MAC_ADDR_LEN);
}

OAL_STATIC OAL_INLINE void mac_get_address2(uint8_t *puc_mac_hdr, uint8_t *puc_addr, uint8_t uc_addr_len)
{
    memcpy_s(puc_addr, uc_addr_len, puc_mac_hdr + BYTE_OFFSET_10, WLAN_MAC_ADDR_LEN);
}


OAL_STATIC OAL_INLINE void mac_get_address3(uint8_t *puc_mac_hdr, uint8_t *puc_addr, uint8_t uc_addr_len)
{
    memcpy_s(puc_addr, uc_addr_len, puc_mac_hdr + BYTE_OFFSET_16, WLAN_MAC_ADDR_LEN);
}


OAL_STATIC OAL_INLINE void mac_get_qos_ctrl(uint8_t *puc_mac_hdr,
                                            uint8_t *puc_qos_ctrl,
                                            uint8_t uc_qos_ctl_len)
{
    if (OAL_TRUE != mac_is_4addr(puc_mac_hdr)) {
        memcpy_s(puc_qos_ctrl, uc_qos_ctl_len, puc_mac_hdr + MAC_QOS_CTRL_FIELD_OFFSET, MAC_QOS_CTL_LEN);
        return;
    }

    memcpy_s(puc_qos_ctrl, uc_qos_ctl_len, puc_mac_hdr + MAC_QOS_CTRL_FIELD_OFFSET_4ADDR, MAC_QOS_CTL_LEN);
    return;
}


OAL_STATIC OAL_INLINE mac_status_code_enum_uint16 mac_get_asoc_status(uint8_t *puc_mac_payload)
{
    mac_status_code_enum_uint16 en_asoc_status;

    en_asoc_status = (puc_mac_payload[BYTE_OFFSET_3] << BIT_OFFSET_8) | puc_mac_payload[BYTE_OFFSET_2];

    return en_asoc_status;
}


OAL_STATIC OAL_INLINE uint16_t mac_get_asoc_id(uint8_t *puc_mac_payload)
{
    uint16_t us_asoc_id;

    us_asoc_id = puc_mac_payload[BYTE_OFFSET_4] | (puc_mac_payload[BYTE_OFFSET_5] << BIT_OFFSET_8);
    us_asoc_id &= 0x3FFF; /* 取低14位 */

    return us_asoc_id;
}


OAL_STATIC OAL_INLINE void mac_get_bssid(uint8_t *puc_mac_hdr, uint8_t *puc_bssid, uint8_t uc_addr_len)
{
    if (1 == mac_hdr_get_from_ds(puc_mac_hdr)) {
        mac_get_address2(puc_mac_hdr, puc_bssid, uc_addr_len);
    } else if (1 == mac_hdr_get_to_ds(puc_mac_hdr)) {
        mac_get_addr1(puc_mac_hdr, puc_bssid, uc_addr_len);
    } else {
        mac_get_address3(puc_mac_hdr, puc_bssid, uc_addr_len);
    }
}

#endif /* end of mac_frame.h */
