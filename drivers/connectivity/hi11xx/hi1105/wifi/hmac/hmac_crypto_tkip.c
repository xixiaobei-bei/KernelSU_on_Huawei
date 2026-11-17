
/* 1 头文件包含 */
#include "hmac_crypto_tkip.h"

/* 2 全局变量定义 */
#define MAX_TKIP_PN_GAP_ERR 1 /* Max. gap in TKIP PN before doing MIC sanity check */

/* 3 函数实现 */

OAL_STATIC void hmac_crypto_tkip_michael_hdr(mac_ieee80211_frame_stru *pst_frame_header,
                                             uint8_t *puc_hdr, uint8_t uc_hdr_size)
{
    mac_ieee80211_frame_addr4_stru *pst_frame_4addr_hdr = NULL;
    uint8_t uc_frame_dir;
    int32_t l_ret = EOK;

    pst_frame_4addr_hdr = (mac_ieee80211_frame_addr4_stru *)pst_frame_header;

    uc_frame_dir = (pst_frame_4addr_hdr->st_frame_control.bit_to_ds) ? 1 : 0;
    uc_frame_dir += (pst_frame_4addr_hdr->st_frame_control.bit_from_ds) ? 2 : 0;  /* 来自ds的bit值为2 */

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
        oam_error_log0(0, OAM_SF_ANY, "hmac_crypto_tkip_michael_hdr::memcpy fail!");
        return;
    }

    puc_hdr[BYTE_OFFSET_12] = 0;

    if (pst_frame_4addr_hdr->st_frame_control.bit_sub_type == WLAN_QOS_DATA) {
        if (IEEE80211_FC1_DIR_DSTODS == uc_frame_dir) {
            puc_hdr[BYTE_OFFSET_12] = ((mac_ieee80211_qos_frame_addr4_stru *)pst_frame_4addr_hdr)->bit_qc_tid;
        } else {
            puc_hdr[BYTE_OFFSET_12] = ((mac_ieee80211_qos_frame_stru *)pst_frame_4addr_hdr)->bit_qc_tid;
        }
    }

    puc_hdr[BYTE_OFFSET_13] = puc_hdr[BYTE_OFFSET_14] = puc_hdr[BYTE_OFFSET_15] = 0; /* reserved */
}

OAL_STATIC void hmac_crypto_tkip_micheal_padding(uint32_t data_len,
                                                 uint8_t uc_pd_src0, uint8_t uc_pd_src1,
                                                 uint8_t uc_pd_src2, uint32_t *pul_msb)
{
    switch (data_len) {
        case 0:  /* 长度是0 */
            *pul_msb ^= get_le32_split(0x5a, 0, 0, 0);
            break;
        case 1:  /* 长度是1 */
            *pul_msb ^= get_le32_split(uc_pd_src0, 0x5a, 0, 0);
            break;
        case 2:  /* 长度是2 */
            *pul_msb ^= get_le32_split(uc_pd_src0, uc_pd_src1, 0x5a, 0);
            break;
        case 3:  /* 长度是3 */
            *pul_msb ^= get_le32_split(uc_pd_src0, uc_pd_src1, uc_pd_src2, 0x5a);
            break;
        default:
            break;
    }
}


OAL_STATIC uint32_t hmac_crypto_tkip_michael_mic(const unsigned char *puc_key,
                                                 oal_netbuf_stru *pst_netbuf,
                                                 mac_ieee80211_frame_stru *pst_frame_header,
                                                 uint32_t offset, uint32_t data_len,
                                                 uint8_t auc_mic[IEEE80211_WEP_MICLEN],
                                                 uint8_t uc_mic_len)
{
    uint8_t auc_hdr[WLAN_TKIP_MIC_LEN] = { 0 };
    uint32_t msb;
    uint32_t lsb;
    const uint8_t *data = NULL;
    uint32_t space;
    const uint8_t *data_next = NULL;
    uint8_t *puc_hdr = NULL;

    hmac_crypto_tkip_michael_hdr(pst_frame_header, auc_hdr, sizeof(auc_hdr));

    msb = get_le32(puc_key);
    lsb = get_le32(puc_key + BYTE_OFFSET_4);

    /* Michael MIC pseudo header: DA, SA, 3 x 0, Priority */
    puc_hdr = &auc_hdr[0];
    msb ^= get_le32(puc_hdr);
    michael_block(msb, lsb);
    puc_hdr = &auc_hdr[BYTE_OFFSET_4];
    msb ^= get_le32(puc_hdr);
    michael_block(msb, lsb);
    puc_hdr = &auc_hdr[BYTE_OFFSET_8];
    msb ^= get_le32(puc_hdr);
    michael_block(msb, lsb);
    puc_hdr = &auc_hdr[BYTE_OFFSET_12];
    msb ^= get_le32(&auc_hdr[BYTE_OFFSET_12]);
    michael_block(msb, lsb);

    /* first buffer has special handling */
    data = oal_netbuf_data(pst_netbuf) + offset;
    space = oal_netbuf_len(pst_netbuf) - offset;
    for (;;) {
        if (space > data_len) {
            space = data_len;
        }

        /* collect 32-bit blocks from current buffer */
        while (space >= sizeof(uint32_t)) {
            msb ^= get_le32(data);
            michael_block(msb, lsb);
            data += sizeof(uint32_t);
            space -= sizeof(uint32_t);
            data_len -= sizeof(uint32_t);
        }

        if (data_len < sizeof(uint32_t)) {
            break;
        }

        pst_netbuf = oal_netbuf_list_next(pst_netbuf);
        if (pst_netbuf == NULL) {
            return OAL_ERR_CODE_SECURITY_BUFF_NUM;
        }
        if (space != 0) {
            /*
             * Block straddles buffers, split references.
             */
            data_next = oal_netbuf_data(pst_netbuf);
            if (sizeof(uint32_t) - space > oal_netbuf_len(pst_netbuf)) {
                return OAL_ERR_CODE_SECURITY_BUFF_LEN;
            }
            switch (space) {
                case 1:  /* 剩余空间为1 */
                    msb ^= get_le32_split(data[0], data_next[0], data_next[1], data_next[BYTE_OFFSET_2]);
                    data = data_next + BYTE_OFFSET_3;
                    space = oal_netbuf_len(pst_netbuf) - BYTE_OFFSET_3;
                    break;
                case 2:  /* 剩余空间为2 */
                    msb ^= get_le32_split(data[0], data[1], data_next[0], data_next[1]);
                    data = data_next + BYTE_OFFSET_2;
                    space = oal_netbuf_len(pst_netbuf) - BYTE_OFFSET_2;
                    break;
                case 3:  /* 剩余空间为3 */
                    msb ^= get_le32_split(data[0], data[1], data[BYTE_OFFSET_2], data_next[0]);
                    data = data_next + 1;
                    space = oal_netbuf_len(pst_netbuf) - 1;
                    break;
                default:
                    break;
            }

            michael_block(msb, lsb);
            data_len -= sizeof(uint32_t);
        } else {
            /*
            * Setup for next buffer.
            */
            data = oal_netbuf_data(pst_netbuf);
            space = oal_netbuf_len(pst_netbuf);
        }
    }
    /* Last block and padding (0x5a, 4..7 x 0) */
    hmac_crypto_tkip_micheal_padding(data_len, data[0], data[1], data[BYTE_OFFSET_2], &msb);

    michael_block(msb, lsb);
    michael_block(msb, lsb);

    if (uc_mic_len < IEEE80211_WEP_MICLEN) {
        return OAL_FAIL;
    }

    put_le32(auc_mic, msb);
    put_le32(auc_mic + BYTE_OFFSET_4, lsb);

    return OAL_SUCC;
}

uint32_t hmac_crypto_tkip_enmic(wlan_priv_key_param_stru *pst_key, oal_netbuf_stru *pst_netbuf)
{
    uint32_t hdrlen = 0; /* 发送时，ul_pktlen里不含80211mac头 */
    uint32_t pktlen;
    uint32_t ret;
    uint8_t *puc_mic_tail = NULL;
    uint8_t *puc_tx_mic_key = NULL;
    uint8_t auc_mic[IEEE80211_WEP_MICLEN] = { 0 };
    mac_tx_ctl_stru *pst_cb = NULL;
    uint32_t tailroom;

    if (WLAN_80211_CIPHER_SUITE_TKIP != ((uint8_t)pst_key->cipher)) {
        oam_warning_log1(0, OAM_SF_WPA, "hmac_crypto_tkip_enmic::pst_key->cipher=0x%x", pst_key->cipher);
        return OAL_ERR_CODE_SECURITY_CHIPER_TYPE;
    }

    /* 1.1 计算整个报文的长度，不处理存在多个netbuf的情况 */
    pktlen = oal_netbuf_len(pst_netbuf);
    if (NULL != oal_netbuf_next(pst_netbuf)) {
        return OAL_ERR_CODE_SECURITY_BUFF_NUM;
    }

    /* 2.1 在netbuf上增加mic空间 */
    tailroom = oal_netbuf_tailroom(pst_netbuf);
    if (oal_unlikely(tailroom < IEEE80211_WEP_MICLEN)) {
        /* 2.2 如果原来的netbuf长度不够，需要重新申请 */
        pst_netbuf = oal_netbuf_realloc_tailroom(pst_netbuf, IEEE80211_WEP_MICLEN);
        if (pst_netbuf == NULL) {
            oam_error_log1(0, OAM_SF_WPA, "{hmac_crypto_tkip_enmic::tailroom[%d] realloc fail.}", tailroom);
            return OAL_ERR_CODE_PTR_NULL;
        }
    }

    /* 3.1 获取mic及密钥 */
    puc_mic_tail = (uint8_t *)OAL_NETBUF_TAIL(pst_netbuf);
    puc_tx_mic_key = pst_key->auc_key + WLAN_TEMPORAL_KEY_LENGTH;

    oal_netbuf_put(pst_netbuf, IEEE80211_WEP_MICLEN);

    pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    /* 4.1 计算mic */
    ret = hmac_crypto_tkip_michael_mic(puc_tx_mic_key, pst_netbuf,
        MAC_GET_CB_FRAME_HEADER_ADDR(pst_cb), hdrlen,
        pktlen - hdrlen, auc_mic, sizeof(auc_mic));
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 4.1 拷贝mic到帧尾部 */
    ret = memcpy_s(puc_mic_tail, tailroom, auc_mic, IEEE80211_WEP_MICLEN);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_WPA, "{hmac_crypto_tkip_enmic::memcpy error!}");
    }
    return OAL_SUCC;
}


uint32_t hmac_crypto_tkip_demic(wlan_priv_key_param_stru *pst_key, oal_netbuf_stru *pst_netbuf)
{
    mac_rx_ctl_stru *pst_cb = NULL;
    uint32_t hdrlen; /* 接收时，ul_pktlen里包含80211mac头 */
    uint8_t *puc_rx_mic_key = NULL;
    uint32_t ret;
    uint32_t pktlen;
    uint8_t auc_mic[IEEE80211_WEP_MICLEN] = { 0 };
    uint8_t auc_mic_peer[IEEE80211_WEP_MICLEN] = { 0 };

    if (WLAN_80211_CIPHER_SUITE_TKIP != ((uint8_t)pst_key->cipher)) {
        oam_warning_log1(0, OAM_SF_WPA, "hmac_crypto_tkip_demic::pst_key->cipher=0x%x", pst_key->cipher);
        return OAL_ERR_CODE_SECURITY_CHIPER_TYPE;
    }

    /* 1.1 计算整个报文的长度，不处理存在多个netbuf的情况 */
    pktlen = oal_netbuf_len(pst_netbuf);
    if (NULL != oal_netbuf_list_next(pst_netbuf)) {
        return OAL_ERR_CODE_SECURITY_BUFF_NUM;
    }

    /* 2.1 从CB中获取80211头长度 */
    pst_cb = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    hdrlen = pst_cb->uc_mac_header_len;

    /* 3.1 只处理数据帧 */
    /* 4.1 获取解密密钥，接收密钥需要偏移8个字节 */
    puc_rx_mic_key = pst_key->auc_key + WLAN_TEMPORAL_KEY_LENGTH + WLAN_MIC_KEY_LENGTH;

    /* 5.1 计算mic */
    ret = hmac_crypto_tkip_michael_mic(puc_rx_mic_key, pst_netbuf,
        (mac_ieee80211_frame_stru *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_cb),
        hdrlen, pktlen - (hdrlen + IEEE80211_WEP_MICLEN), auc_mic, sizeof(auc_mic));
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 6.1 获取对端的mic并跟本地计算的mic进行比较 */
    oal_netbuf_copydata(pst_netbuf, pktlen - IEEE80211_WEP_MICLEN, (void *)auc_mic_peer, IEEE80211_WEP_MICLEN);
    if (oal_memcmp(auc_mic, auc_mic_peer, IEEE80211_WEP_MICLEN)) {
        /* NB: 802.11 layer handles statistic and debug msg */
        return OAL_ERR_CODE_SECURITY_WRONG_KEY;
    }

    /* 7.1 去掉mic尾部 */
    oal_netbuf_trim(pst_netbuf, IEEE80211_WEP_MICLEN);
    return OAL_SUCC;
}

