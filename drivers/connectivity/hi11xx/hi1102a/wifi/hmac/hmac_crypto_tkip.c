

/* 1 头文件包含 */
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "mac_frame.h"
#include "dmac_ext_if.h"
#include "hmac_crypto_tkip.h"
#include "securec.h"
#include "securectype.h"

/* 2 全局变量定义 */
#define MAX_TKIP_PN_GAP_ERR 1 /* Max. gap in TKIP PN before doing MIC sanity check */

/* 3 函数实现 */

OAL_STATIC oal_void hmac_crypto_tkip_michael_hdr(mac_ieee80211_frame_stru *pst_frame_header,
                                                 oal_uint8 *puc_hdr, oal_uint8 uc_hdr_size)
{
    mac_ieee80211_frame_addr4_stru *pst_frame_4addr_hdr = OAL_PTR_NULL;
    oal_uint8 uc_frame_dir;
    oal_int32 l_ret = EOK;

    pst_frame_4addr_hdr = (mac_ieee80211_frame_addr4_stru *)pst_frame_header;

    uc_frame_dir = (pst_frame_4addr_hdr->st_frame_control.bit_to_ds) ? 1 : 0;
    uc_frame_dir += (pst_frame_4addr_hdr->st_frame_control.bit_from_ds) ? 2 : 0;

    switch (uc_frame_dir) {
        case IEEE80211_FC1_DIR_NODS:
            l_ret += memcpy_s(puc_hdr, uc_hdr_size, pst_frame_4addr_hdr->auc_address1, WLAN_MAC_ADDR_LEN);
            l_ret += memcpy_s(puc_hdr + WLAN_MAC_ADDR_LEN, uc_hdr_size - WLAN_MAC_ADDR_LEN,
                              pst_frame_4addr_hdr->auc_address2, WLAN_MAC_ADDR_LEN);
            break;
        case IEEE80211_FC1_DIR_TODS:
            l_ret += memcpy_s(puc_hdr, uc_hdr_size, pst_frame_4addr_hdr->auc_address3, WLAN_MAC_ADDR_LEN);
            l_ret += memcpy_s(puc_hdr + WLAN_MAC_ADDR_LEN, uc_hdr_size - WLAN_MAC_ADDR_LEN,
                              pst_frame_4addr_hdr->auc_address2, WLAN_MAC_ADDR_LEN);
            break;
        case IEEE80211_FC1_DIR_FROMDS:
            l_ret += memcpy_s(puc_hdr, uc_hdr_size, pst_frame_4addr_hdr->auc_address1, WLAN_MAC_ADDR_LEN);
            l_ret += memcpy_s(puc_hdr + WLAN_MAC_ADDR_LEN, uc_hdr_size - WLAN_MAC_ADDR_LEN,
                              pst_frame_4addr_hdr->auc_address3, WLAN_MAC_ADDR_LEN);
            break;
        default:
            l_ret += memcpy_s(puc_hdr, uc_hdr_size, pst_frame_4addr_hdr->auc_address3, WLAN_MAC_ADDR_LEN);
            l_ret += memcpy_s(puc_hdr + WLAN_MAC_ADDR_LEN, uc_hdr_size - WLAN_MAC_ADDR_LEN,
                              pst_frame_4addr_hdr->auc_address4, WLAN_MAC_ADDR_LEN);
            break;
    }
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_crypto_tkip_michael_hdr::memcpy fail!");
        return;
    }
    puc_hdr[12] = 0;

    if (pst_frame_4addr_hdr->st_frame_control.bit_sub_type == WLAN_QOS_DATA) {
        if (uc_frame_dir == IEEE80211_FC1_DIR_DSTODS) {
            puc_hdr[12] = ((mac_ieee80211_qos_frame_addr4_stru *)pst_frame_4addr_hdr)->bit_qc_tid;
        } else {
            puc_hdr[12] = ((mac_ieee80211_qos_frame_stru *)pst_frame_4addr_hdr)->bit_qc_tid;
        }
    }

    puc_hdr[13] = puc_hdr[14] = puc_hdr[15] = 0; /* reserved */
}

OAL_STATIC oal_uint32 hmac_crypto_tkip_michael_mic(oal_uint8 *puc_key,
                                                   oal_netbuf_stru *pst_netbuf,
                                                   mac_ieee80211_frame_stru *pst_frame_header,
                                                   oal_uint32 ul_offset,
                                                   oal_uint32 ul_data_len,
                                                   oal_uint8 auc_mic[IEEE80211_WEP_MICLEN])
{
    oal_uint8 auc_hdr[16] = { 0 };
    oal_uint32 ul_msb;
    oal_uint32 ul_lsb;
    const oal_uint8 *puc_data = OAL_PTR_NULL;
    oal_uint32 ul_space;
    const oal_uint8 *puc_data_next = OAL_PTR_NULL;

    hmac_crypto_tkip_michael_hdr(pst_frame_header, auc_hdr, OAL_SIZEOF(auc_hdr));

    ul_msb = get_le32(puc_key);
    ul_lsb = get_le32(puc_key + 4);

    /* Michael MIC pseudo header: DA, SA, 3 x 0, Priority */
    ul_msb ^= get_le32(auc_hdr);
    michael_block(ul_msb, ul_lsb);
    ul_msb ^= get_le32(&auc_hdr[4]);
    michael_block(ul_msb, ul_lsb);
    ul_msb ^= get_le32(&auc_hdr[8]);
    michael_block(ul_msb, ul_lsb);
    ul_msb ^= get_le32(&auc_hdr[12]);
    michael_block(ul_msb, ul_lsb);

    /* first buffer has special handling */
    puc_data = OAL_NETBUF_DATA(pst_netbuf) + ul_offset;
    ul_space = OAL_NETBUF_LEN(pst_netbuf) - ul_offset;
    for (;;) {
        if (ul_space > ul_data_len) {
            ul_space = ul_data_len;
        }

        /* collect 32-bit blocks from current buffer */
        while (ul_space >= sizeof(oal_uint32)) {
            ul_msb ^= get_le32(puc_data);
            michael_block(ul_msb, ul_lsb);
            puc_data += sizeof(oal_uint32);
            ul_space -= sizeof(oal_uint32);
            ul_data_len -= sizeof(oal_uint32);
        }

        if (ul_data_len < sizeof(oal_uint32)) {
            break;
        }

        pst_netbuf = oal_netbuf_list_next(pst_netbuf);
        if (pst_netbuf == NULL) {
            return OAL_ERR_CODE_SECURITY_BUFF_NUM;
        }
        if (ul_space != 0) {
            /*
            * Block straddles buffers, split references.
 */
            puc_data_next = OAL_NETBUF_DATA(pst_netbuf);
            if (sizeof(oal_uint32) - ul_space > OAL_NETBUF_LEN(pst_netbuf)) {
                return OAL_ERR_CODE_SECURITY_BUFF_LEN;
            }
            switch (ul_space) {
                case 1:
                    ul_msb ^= get_le32_split(puc_data[0], puc_data_next[0], puc_data_next[1], puc_data_next[2]);
                    puc_data = puc_data_next + 3;
                    ul_space = OAL_NETBUF_LEN(pst_netbuf) - 3;
                    break;
                case 2:
                    ul_msb ^= get_le32_split(puc_data[0], puc_data[1], puc_data_next[0], puc_data_next[1]);
                    puc_data = puc_data_next + 2;
                    ul_space = OAL_NETBUF_LEN(pst_netbuf) - 2;
                    break;
                case 3:
                    ul_msb ^= get_le32_split(puc_data[0], puc_data[1], puc_data[2], puc_data_next[0]);
                    puc_data = puc_data_next + 1;
                    ul_space = OAL_NETBUF_LEN(pst_netbuf) - 1;
                    break;
                default:
                    break;
            }

            michael_block(ul_msb, ul_lsb);
            ul_data_len -= sizeof(oal_uint32);
        } else {
            /*
            * Setup for next buffer.
 */
            puc_data = OAL_NETBUF_DATA(pst_netbuf);
            ul_space = OAL_NETBUF_LEN(pst_netbuf);
        }
    }
    /* Last block and padding (0x5a, 4..7 x 0) */
    switch (ul_data_len) {
        case 0:
            ul_msb ^= get_le32_split(0x5a, 0, 0, 0);
            break;
        case 1:
            ul_msb ^= get_le32_split(puc_data[0], 0x5a, 0, 0);
            break;
        case 2:
            ul_msb ^= get_le32_split(puc_data[0], puc_data[1], 0x5a, 0);
            break;
        case 3:
            ul_msb ^= get_le32_split(puc_data[0], puc_data[1], puc_data[2], 0x5a);
            break;
        default:
            break;
    }
    michael_block(ul_msb, ul_lsb);
    /* l ^= 0; */
    michael_block(ul_msb, ul_lsb);

    put_le32(auc_mic, ul_msb);
    put_le32(auc_mic + 4, ul_lsb);

    return OAL_SUCC;
}

oal_uint32 hmac_crypto_tkip_enmic(wlan_priv_key_param_stru *pst_key, oal_netbuf_stru *pst_netbuf)
{
    oal_uint32 ul_hdrlen = 0; /* 发送时，ul_pktlen里不含80211mac头 */
    oal_uint32 ul_pktlen;
    oal_uint32 ul_ret;
    oal_uint8 *puc_mic_tail = OAL_PTR_NULL;
    oal_uint8 *puc_tx_mic_key = OAL_PTR_NULL;
    oal_uint8 auc_mic[IEEE80211_WEP_MICLEN] = { 0 };
    mac_tx_ctl_stru *pst_cb = OAL_PTR_NULL;
    oal_uint32 ul_tailroom;

    if (((oal_uint8)pst_key->ul_cipher) != WLAN_80211_CIPHER_SUITE_TKIP) {
        return OAL_ERR_CODE_SECURITY_CHIPER_TYPE;
    }

    /* 1.1 计算整个报文的长度，不处理存在多个netbuf的情况 */
    ul_pktlen = OAL_NETBUF_LEN(pst_netbuf);
    if (OAL_NETBUF_NEXT(pst_netbuf) != OAL_PTR_NULL) {
        return OAL_ERR_CODE_SECURITY_BUFF_NUM;
    }

    /* 2.1 在netbuf上增加mic空间 */
    ul_tailroom = oal_netbuf_tailroom(pst_netbuf);
    if (oal_netbuf_tailroom(pst_netbuf) < IEEE80211_WEP_MICLEN) {
        /* 2.2 如果原来的netbuf长度不够，需要重新申请 */
        pst_netbuf = oal_netbuf_realloc_tailroom(pst_netbuf, IEEE80211_WEP_MICLEN);
        if (pst_netbuf == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(0, OAM_SF_WPA, "{hmac_crypto_tkip_enmic:: tailroom [%d] realloc failed!}", ul_tailroom);
            return OAL_ERR_CODE_PTR_NULL;
        }
    }

    /* 3.1 获取mic及密钥 */
    puc_mic_tail = (oal_uint8 *)OAL_NETBUF_TAIL(pst_netbuf);
    puc_tx_mic_key = pst_key->auc_key + WLAN_TEMPORAL_KEY_LENGTH;

    oal_netbuf_put(pst_netbuf, IEEE80211_WEP_MICLEN);

    pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    /* 4.1 计算mic */
    ul_ret = hmac_crypto_tkip_michael_mic(puc_tx_mic_key, pst_netbuf, pst_cb->pst_frame_header, ul_hdrlen,
                                          ul_pktlen - ul_hdrlen, auc_mic);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    /* 4.1 拷贝mic到帧尾部 */
    memcpy_s(puc_mic_tail, ul_tailroom, auc_mic, IEEE80211_WEP_MICLEN);

    return OAL_SUCC;
}


oal_uint32 hmac_crypto_tkip_demic(wlan_priv_key_param_stru *pst_key, oal_netbuf_stru *pst_netbuf)
{
    mac_rx_ctl_stru *pst_cb = OAL_PTR_NULL;
    oal_uint32 ul_hdrlen; /* 接收时，ul_pktlen里包含80211mac头 */
    oal_uint8 *puc_rx_mic_key = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_uint32 ul_pktlen;
    oal_uint8 auc_mic[IEEE80211_WEP_MICLEN] = { 0 };
    oal_uint8 auc_mic_peer[IEEE80211_WEP_MICLEN] = { 0 };

    if (((oal_uint8)pst_key->ul_cipher) != WLAN_80211_CIPHER_SUITE_TKIP) {
        return OAL_ERR_CODE_SECURITY_CHIPER_TYPE;
    }

    /* 1.1 计算整个报文的长度，不处理存在多个netbuf的情况 */
    ul_pktlen = OAL_NETBUF_LEN(pst_netbuf);
    if (oal_netbuf_list_next(pst_netbuf) != OAL_PTR_NULL) {
        return OAL_ERR_CODE_SECURITY_BUFF_NUM;
    }

    /* 2.1 从CB中获取80211头长度 */
    pst_cb = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    ul_hdrlen = pst_cb->uc_mac_header_len;

    /* 4.1 获取解密密钥，接收密钥需要偏移8个字节 */
    puc_rx_mic_key = pst_key->auc_key + WLAN_TEMPORAL_KEY_LENGTH + WLAN_MIC_KEY_LENGTH;

    /* 5.1 计算mic */
    ul_ret = hmac_crypto_tkip_michael_mic(puc_rx_mic_key, pst_netbuf,
                                          (mac_ieee80211_frame_stru *)pst_cb->pul_mac_hdr_start_addr,
                                          ul_hdrlen, ul_pktlen - (ul_hdrlen + IEEE80211_WEP_MICLEN), auc_mic);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    /* 6.1 获取对端的mic并跟本地计算的mic进行比较 */
    oal_netbuf_copydata(pst_netbuf, ul_pktlen - IEEE80211_WEP_MICLEN, (oal_void *)auc_mic_peer, IEEE80211_WEP_MICLEN);
    if (oal_memcmp(auc_mic, auc_mic_peer, IEEE80211_WEP_MICLEN)) {
        return OAL_ERR_CODE_SECURITY_WRONG_KEY;
    }

    /* 7.1 去掉mic尾部 */
    oal_netbuf_trim(pst_netbuf, IEEE80211_WEP_MICLEN);
    return OAL_SUCC;
}
