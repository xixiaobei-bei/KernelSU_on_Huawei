
#ifdef _PRE_WLAN_FEATURE_WAPI

#include "oal_types.h"
#include "wlan_spec.h"
#include "mac_data.h"
#include "hmac_wapi.h"
#include "hmac_wapi_sms4.h"
#include "hmac_wapi_wpi.h"
#include "hmac_rx_data.h"
#include "mac_frame_inl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_WAPI_C

const uint8_t g_auc_wapi_pn_init[WAPI_PN_LEN] = {
    0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36,
    0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c
};


oal_bool_enum_uint8 hmac_wapi_is_qos(mac_ieee80211_frame_stru *pst_mac_hdr)
{
    return ((pst_mac_hdr->st_frame_control.bit_type == WLAN_DATA_BASICTYPE) &&
            (pst_mac_hdr->st_frame_control.bit_sub_type & WLAN_QOS_DATA));
}


uint32_t hmac_wapi_calc_mic_data(mac_ieee80211_frame_stru *mac_hdr, uint8_t key_idx, uint8_t *payload, uint16_t pdu_len,
    uint8_t *mic, uint16_t mic_len)
{
    uint8_t is_qos;
    uint8_t *mic_ori = NULL;

    memset_s(mic, mic_len, 0, mic_len);
    mic_ori = mic;

    /* frame control */
    if (memcpy_s(mic, mic_len, (uint8_t *)&(mac_hdr->st_frame_control), sizeof(mac_hdr->st_frame_control)) != EOK) {
        return OAL_FAIL;
    }
    mic[0] &= ~(BIT4 | BIT5 | BIT6); /* sub type */
    mic[1] &= ~(BIT3 | BIT4 | BIT5); /* retry, pwr Mgmt, more data */
    mic[1] |= BIT6;

    mic += sizeof(mac_hdr->st_frame_control);

    /* addr1 */
    mac_get_addr1((uint8_t *)mac_hdr, mic, OAL_MAC_ADDR_LEN);
    mic += OAL_MAC_ADDR_LEN;

    /* addr2 */
    mac_get_address2((uint8_t *)mac_hdr, mic, OAL_MAC_ADDR_LEN);
    mic += OAL_MAC_ADDR_LEN;

    /* 序列控制 */
    memset_s(mic, OAL_SEQ_CTL_LEN, 0, OAL_SEQ_CTL_LEN);
    mic[0] = (uint8_t)(mac_hdr->bit_frag_num);
    mic += BYTE_OFFSET_2;

    /* addr3 */
    mac_get_address3((uint8_t *)mac_hdr, mic, OAL_MAC_ADDR_LEN);
    mic += OAL_MAC_ADDR_LEN;

    /* 跳过addr4 */
    mic += OAL_MAC_ADDR_LEN;

    /* qos ctrl */
    is_qos = hmac_wapi_is_qos(mac_hdr);
    if (is_qos == OAL_TRUE) {
        mac_get_qos_ctrl((uint8_t *)mac_hdr, mic, MAC_QOS_CTL_LEN);
        mic += MAC_QOS_CTL_LEN;
    }

    *mic = key_idx;
    mic += BYTE_OFFSET_2;

    /* 填充pdulen 协议写明大端字节序 */
    *mic = (uint8_t)((pdu_len & 0xff00) >> BIT_OFFSET_8);
    *(mic + 1) = (uint8_t)(pdu_len & 0x00ff);
    mic += BYTE_OFFSET_2;

    /************填充第2部分*******************/
    mic_ori += (OAL_TRUE == hmac_wapi_is_qos(mac_hdr)) ? SMS4_MIC_PART1_QOS_LEN : SMS4_MIC_PART1_NO_QOS_LEN;
    if (memcpy_s(mic_ori, pdu_len, payload, pdu_len) != EOK) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


uint8_t *hmac_wapi_mic_alloc(uint8_t is_qos, uint16_t us_pdu_len, uint16_t *pus_mic_len)
{
    uint16_t us_mic_part1_len;
    uint16_t us_mic_part2_len;
    uint16_t us_mic_len;

    us_mic_part1_len = (is_qos == OAL_TRUE) ? SMS4_MIC_PART1_QOS_LEN : SMS4_MIC_PART1_NO_QOS_LEN;

    /* 按照协议，补齐不足位，16字节对齐 */
    us_mic_part2_len = padding_m(us_pdu_len, SMS4_PADDING_LEN);

    us_mic_len = us_mic_part1_len + us_mic_part2_len;

    *pus_mic_len = us_mic_len;

    return oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, us_mic_len, OAL_TRUE);
}


OAL_STATIC void hmac_wapi_mic_free(uint8_t *puc_mic)
{
    if (puc_mic != NULL) {
        oal_mem_free_m(puc_mic, OAL_TRUE);
    }
}


uint8_t hmac_wapi_is_keyidx_valid(hmac_wapi_stru *pst_wapi, uint8_t uc_keyidx_rx)
{
    if (pst_wapi->uc_keyidx != uc_keyidx_rx && pst_wapi->uc_keyupdate_flg != OAL_TRUE) {
        oam_warning_log3(0, OAM_SF_ANY, "{hmac_wapi_is_keyidx_valid::keyidx==%u, uc_keyidx_rx==%u, update==%u.}",
                         pst_wapi->uc_keyidx, uc_keyidx_rx, pst_wapi->uc_keyupdate_flg);
        WAPI_RX_IDX_UPDATE_ERR(pst_wapi);
        return OAL_FALSE;
    }

    pst_wapi->uc_keyupdate_flg = OAL_FALSE; /* 更新完成取消标志 */

    /* key没有启用 */
    if (pst_wapi->ast_wapi_key[uc_keyidx_rx].uc_key_en != OAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_wapi_is_keyidx_valid::keyen==%u.}",
                         pst_wapi->ast_wapi_key[uc_keyidx_rx].uc_key_en);
        WAPI_RX_IDX_UPDATE_ERR(pst_wapi);
        return OAL_FALSE;
    }

    return OAL_TRUE;
}


oal_bool_enum_uint8 hmac_wapi_is_pn_odd_ucast(uint8_t *puc_pn)
{
    return (oal_bool_enum_uint8)(((*puc_pn & BIT0) == 0) ? OAL_FALSE : OAL_TRUE);
}


uint8_t hmac_wapi_is_pn_bigger(uint8_t *puc_pn, uint8_t *puc_pn_rx)
{
    uint8_t uc_pnidx;

    for (uc_pnidx = SMS4_PN_LEN - 1; uc_pnidx > 0; uc_pnidx--) {
        if ((puc_pn[uc_pnidx] != puc_pn_rx[uc_pnidx])) {
            if (puc_pn[uc_pnidx] > puc_pn_rx[uc_pnidx]) {
                oam_warning_log2(0, OAM_SF_ANY,
                                 "{hmac_wapi_is_pn_bigger::err! puc_pn==%u, puc_pn_rx==%u.}",
                                 puc_pn[uc_pnidx], puc_pn_rx[uc_pnidx]);
                return OAL_FALSE;
            }

            return OAL_TRUE;
        }
    }

    return OAL_TRUE;
}


uint8_t hmac_wapi_is_pn_odd_bcast(uint8_t *puc_pn)
{
    return OAL_TRUE;
}


void hmac_wapi_pn_update(uint8_t *puc_pn, uint8_t uc_inc)
{
    uint32_t loop;
    uint32_t loop_num;
    uint32_t overlow; /* 进位 */
    uint32_t *pul_pn;

    pul_pn = (uint32_t *)puc_pn;
    loop_num = WAPI_PN_LEN / sizeof(uint32_t);
    overlow = uc_inc;

    for (loop = 0; loop < loop_num; loop++) {
        if (*pul_pn > (*pul_pn + overlow)) {
            *pul_pn += overlow;
            overlow = 1; /* 溢出高位加1 */
        } else {
            *pul_pn += overlow;
            break;
        }
        pul_pn++;
    }
}


static oal_netbuf_stru *hmac_wapi_netbuff_tx_handle(hmac_wapi_stru *pst_wapi, oal_netbuf_stru *pst_buf)
{
    uint32_t ret;
    oal_netbuf_stru *pst_netbuf_tmp = NULL;  /* 指向需要释放的netbuff */
    oal_netbuf_stru *pst_netbuf_prev = NULL; /* 指向已经加密的netbuff */
    oal_netbuf_stru *pst_buf_first = NULL;   /* 指向还未加密的netbuff */

    /* buf的初始位置在snap头的llc处 */
    if (MAC_DATA_WAPI == mac_get_data_type_from_80211(pst_buf, 0)) {
        oam_warning_log0(0, OAM_SF_WAPI, "{hmac_wapi_netbuff_tx_handle::wapi, dont encrypt!.}");
        return pst_buf;
    }

    ret = pst_wapi->wapi_encrypt(pst_wapi, pst_buf);
    if (ret != OAL_SUCC) {
        hmac_free_netbuf_list(pst_buf);
        return NULL;
    }

    pst_netbuf_tmp = pst_buf;

    /* 使netbuff指针指向下一个需要加密的分片帧 */
    pst_netbuf_prev = oal_netbuf_next(pst_buf);
    if (pst_netbuf_prev == NULL) {
        return NULL;
    }
    pst_buf_first = pst_netbuf_prev;
    pst_buf = oal_netbuf_next(pst_netbuf_prev);

    oal_netbuf_free(pst_netbuf_tmp);

    while (pst_buf != NULL) {
        ret = pst_wapi->wapi_encrypt(pst_wapi, pst_buf);
        if (ret != OAL_SUCC) {
            hmac_free_netbuf_list(pst_buf_first);
            return NULL;
        }
        oal_netbuf_next(pst_netbuf_prev) = oal_netbuf_next(pst_buf);
        pst_netbuf_tmp = pst_buf;
        pst_netbuf_prev = oal_netbuf_next(pst_buf);
        if (pst_netbuf_prev == NULL) {
            return NULL;
        }
        pst_buf = oal_netbuf_next(pst_netbuf_prev);

        oal_netbuf_free(pst_netbuf_tmp);
    }
    return pst_buf_first;
}

/*
 * 函 数 名  : hmac_wapi_netbuf_tx_encrypt_1106
 * 功能描述  : 1106 host_tx场景，加密已经封装为802.11格式的数据帧
 */
oal_netbuf_stru *hmac_wapi_netbuf_tx_encrypt_1106(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf)
{
    hmac_wapi_stru *hmac_user_wapi = NULL;
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    uint32_t mac_hdr_len;
    int32_t ret;

    if (!IS_LEGACY_STA(&hmac_vap->st_vap_base_info)) {
        return netbuf;
    }

    hmac_user_wapi = &hmac_user->st_wapi;
    if (WAPI_PORT_FLAG(hmac_user_wapi) != OAL_TRUE ||
        hmac_user_wapi->wapi_netbuff_txhandle == NULL) {
        return netbuf;
    }

    mac_hdr_len = MAC_GET_CB_FRAME_HEADER_LENGTH(tx_ctl);
    oal_netbuf_pull(netbuf, mac_hdr_len); /* WAPI加密host_tx,需将netbuff->data指向SNAP头 */
    netbuf = hmac_user_wapi->wapi_netbuff_txhandle(hmac_user_wapi, netbuf);
    if (netbuf == NULL) {
        oam_stat_vap_incr(hmac_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
            "{hmac_wapi_netbuf_tx_encrypt_1106 fail!}");
        return NULL;
    }

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf); /* WAPI加密后返回新netbuf，更新tx_ctl指针 */

    /* MAC header和snap头部分存在空洞，需要调整mac_header内容位置 */
    ret = memmove_s(oal_netbuf_data(netbuf) - mac_hdr_len, mac_hdr_len,
        MAC_GET_CB_FRAME_HEADER_ADDR(tx_ctl), mac_hdr_len);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_WAPI,
            "hmac_wapi_netbuf_tx_encrypt_1106:memmove fail");
    }
    oal_netbuf_push(netbuf, mac_hdr_len); /* WAPI加密后，将netbuf->data重新指向mac_header */

    /* 记录新的MAC头的位置 */
    MAC_GET_CB_FRAME_HEADER_ADDR(tx_ctl) = (mac_ieee80211_frame_stru *)oal_netbuf_data(netbuf);

    return netbuf;
}


static oal_netbuf_stru *hmac_wapi_netbuff_rx_handle(hmac_wapi_stru *pst_wapi, oal_netbuf_stru *pst_buf)
{
    uint32_t ret;
    oal_netbuf_stru *pst_netbuf_tmp = NULL; /* 指向需要释放的netbuff */

    /* 非加密帧，不进行解密 */
    if (!((oal_netbuf_data(pst_buf))[1] & 0x40)) {
        return pst_buf;
    }

    ret = pst_wapi->wapi_decrypt(pst_wapi, pst_buf);
    if (ret != OAL_SUCC) {
        return NULL;
    }

    pst_netbuf_tmp = pst_buf;
    pst_buf = oal_netbuf_next(pst_buf);
    oal_netbuf_free(pst_netbuf_tmp);

    return pst_buf;
}

/*
 * 函 数 名  : hmac_wapi_netbuf_rx_decrypt_1106
 * 功能描述  : 1106 接收，解密802.11格式的数据帧
 */
uint32_t hmac_wapi_netbuf_rx_decrypt_1106(hmac_host_rx_context_stru *rx_context)
{
    hmac_vap_stru *hmac_vap = rx_context->hmac_vap;
    hmac_user_stru *hmac_user = rx_context->hmac_user;
    oal_netbuf_stru *netbuf = rx_context->netbuf;
    hmac_wapi_stru *hmac_user_wapi = NULL;
    mac_rx_ctl_stru *rx_ctrl = rx_context->cb;

    if (rx_ctrl->bit_frame_format != MAC_FRAME_TYPE_80211 ||
        rx_ctrl->bit_amsdu_enable == OAL_TRUE ||
        !IS_LEGACY_STA(&hmac_vap->st_vap_base_info)) {
        return OAL_SUCC;
    }

    hmac_user_wapi = &hmac_user->st_wapi;
    if ((WAPI_PORT_FLAG(hmac_user_wapi) != OAL_TRUE) || (hmac_user_wapi->wapi_netbuff_rxhandle == NULL)) {
        return OAL_SUCC;
    }

    netbuf = hmac_user_wapi->wapi_netbuff_rxhandle(hmac_user_wapi, netbuf);
    if (netbuf == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
            "{hmac_wapi_netbuf_rx_decrypt_1106:: wapi decrypt FAIL!}");
        HMAC_USER_STATS_PKT_INCR(hmac_user->rx_pkt_drop, 1);
        return OAL_ERR_CODE_WAPI_DECRYPT_FAIL;
    }

    /* WAPI解密完成后，返回新的netbuff，需要重新更新rx_context的netbuf/cb指针 */
    rx_context->netbuf = netbuf;
    rx_context->cb = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);

    return OAL_SUCC;
}


uint32_t hmac_wapi_add_key(hmac_wapi_stru *pst_wapi, uint8_t uc_key_index, uint8_t *puc_key)
{
    hmac_wapi_key_stru *pst_key = NULL;
    int32_t l_ret;

    hmac_wapi_set_port(pst_wapi, OAL_TRUE);
    pst_wapi->uc_keyidx = uc_key_index;
    pst_wapi->uc_keyupdate_flg = OAL_TRUE;
    pst_key = &(pst_wapi->ast_wapi_key[uc_key_index]);

    l_ret = memcpy_s(pst_key->auc_wpi_ek, WAPI_KEY_LEN, puc_key, WAPI_KEY_LEN);
    l_ret += memcpy_s(pst_key->auc_wpi_ck, WAPI_KEY_LEN, puc_key + WAPI_KEY_LEN, WAPI_KEY_LEN);
    pst_key->uc_key_en = OAL_TRUE;

    /* 重置PN */
    l_ret += memcpy_s(pst_key->auc_pn_rx, WAPI_PN_LEN, g_auc_wapi_pn_init, WAPI_PN_LEN);
    l_ret += memcpy_s(pst_key->auc_pn_tx, WAPI_PN_LEN, g_auc_wapi_pn_init, WAPI_PN_LEN);
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wapi_add_key::memcpy fail!");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


uint8_t hmac_wapi_is_wpihdr_valid(hmac_wapi_stru *pst_wapi, uint8_t *puc_wapi_hdr)
{
    uint8_t uc_keyidx_rx;
    uint8_t *puc_pn_rx = NULL;

    uc_keyidx_rx = *puc_wapi_hdr;

    if (OAL_TRUE != hmac_wapi_is_keyidx_valid(pst_wapi, uc_keyidx_rx)) {
        return OAL_FALSE;
    }

    puc_pn_rx = puc_wapi_hdr + SMS4_KEY_IDX + SMS4_WAPI_HDR_RESERVE;
    if (pst_wapi->wapi_is_pn_odd(puc_pn_rx) != OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_wapi_is_wpihdr_valid::wapi_is_pn_odd==false.}");
        WAPI_RX_PN_ODD_ERR(pst_wapi, puc_pn_rx);
        return OAL_FALSE;
    }

    /* 此处为重放检测，实际测试中非常影响性能，暂时屏蔽 */
    return OAL_TRUE;
}

// 获取WPAI-SMS4的MPDU帧格式的数据长度
OAL_STATIC uint32_t hmac_wapi_decrypt_check(hmac_wapi_stru *wapi, oal_netbuf_stru *netbuf)
{
    uint8_t key_idx;
    uint8_t mac_hdr_len;
    uint16_t netbuff_len;
    uint8_t *netbuff_local = NULL;
    uint8_t *wapi_hdr = NULL;
    mac_rx_ctl_stru *rx_ctl_in = NULL;
    mac_ieee80211_frame_stru *mac_hdr = NULL;

    netbuff_local = oal_netbuf_data(netbuf);  // for ut,del temprarily
    netbuff_len = (uint16_t)oal_netbuf_len(netbuf);

    /* 获取mac头 */
    mac_hdr = (mac_ieee80211_frame_stru *)netbuff_local;

    /* wapi的数据帧一般为QOS帧  */
    rx_ctl_in = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_hdr_len = rx_ctl_in->uc_mac_header_len;
    wapi_hdr = (uint8_t *)mac_hdr + mac_hdr_len;

    oam_warning_log1(0, OAM_SF_ANY, "{hmac_wapi_decrypt_check::mac_hdr_len %u!.}", mac_hdr_len);

    if (netbuff_len < (uint16_t)(mac_hdr_len + HMAC_WAPI_HDR_LEN + SMS4_MIC_LEN)) {
        oam_error_log2(0, OAM_SF_ANY, "{hmac_wapi_decrypt_check::buff_len %u, machdr_len %u err!}",
            netbuff_len, mac_hdr_len);
        WAPI_RX_NETBUF_LEN_ERR(wapi);
        return OAL_FAIL;
    }

    key_idx = *wapi_hdr;
    if (key_idx >= HMAC_WAPI_MAX_KEYID) {
        WAPI_RX_IDX_ERR(wapi);
        oam_error_log1(0, OAM_SF_ANY, "{hmac_wapi_decrypt_check::key_idx %u err!.}", key_idx);
        return OAL_FAIL;
    }

    if (hmac_wapi_is_wpihdr_valid(wapi, wapi_hdr) != OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_wapi_decrypt_check::hmac_wapi_is_wpihdr_valid err!.}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC void hmac_wapi_decrypt_get_para(oal_netbuf_stru *netbuf, uint16_t *pdu_len, uint8_t *mac_hdr_len,
    mac_ieee80211_frame_stru **mac_hdr, uint8_t **wapi_hdr)
{
    uint8_t *netbuff_local = NULL;
    uint16_t netbuff_len;
    mac_rx_ctl_stru *rx_ctl_in = NULL;

    netbuff_local = oal_netbuf_data(netbuf);  // for ut,del temprarily
    netbuff_len = (uint16_t)oal_netbuf_len(netbuf);
    /* 获取mac头 */
    *mac_hdr = (mac_ieee80211_frame_stru *)netbuff_local;
    /* wapi的数据帧一般为QOS帧  */
    rx_ctl_in = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    *mac_hdr_len = rx_ctl_in->uc_mac_header_len;
    *wapi_hdr = (uint8_t *)(*mac_hdr) + (*mac_hdr_len);
    *pdu_len = netbuff_len - (*mac_hdr_len) - HMAC_WAPI_HDR_LEN - SMS4_MIC_LEN;
    return;
}

OAL_STATIC uint32_t hmac_wapi_netbuff_init(hmac_wapi_stru *wapi, oal_netbuf_stru *netbuf, uint8_t mac_hdr_len,
    oal_netbuf_stru **buff_dst, mac_rx_ctl_stru **rx_ctl)
{
    *buff_dst = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (*buff_dst == NULL) {
        WAPI_RX_MEMALLOC_ERR(wapi);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 先拷贝mac头 */
    oal_netbuf_init(*buff_dst, mac_hdr_len);
    oal_netbuf_copydata(netbuf, 0, oal_netbuf_data(*buff_dst), mac_hdr_len);

    /* 拷贝cb */
    *rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(*buff_dst);
    if (memcpy_s(*rx_ctl, MAC_TX_CTL_SIZE, oal_netbuf_cb(netbuf), MAC_TX_CTL_SIZE) != EOK) {
        oal_netbuf_free(*buff_dst);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_wapi_calc_mic_proc(hmac_wapi_stru *wapi, mac_ieee80211_frame_stru *mac_hdr,
    oal_netbuf_stru *buff, uint8_t *pn, uint16_t pdu_len, uint8_t mac_hdr_len)
{
    uint16_t mic_len;
    uint8_t *mic_data = NULL; /* 按照协议，构造mic所需要的数据，见 wapi实施指南 图51 */
    uint8_t *key_ck = NULL;
    uint32_t ret;
    uint8_t calc_mic[SMS4_MIC_LEN];
    uint8_t *mic = NULL;
    uint8_t key_idx = *((uint8_t *)mac_hdr + mac_hdr_len);

    mic_data = hmac_wapi_mic_alloc(hmac_wapi_is_qos(mac_hdr), pdu_len, &mic_len);
    if (mic_data == NULL) {
        WAPI_RX_MEMALLOC_ERR(wapi);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 计算mic预备数据 */
    ret = hmac_wapi_calc_mic_data(mac_hdr, key_idx, oal_netbuf_data(buff) + mac_hdr_len, pdu_len, mic_data, mic_len);
    if (ret != OAL_SUCC) {
        return ret;
    }

    key_ck = wapi->ast_wapi_key[key_idx].auc_wpi_ck;
    ret = hmac_wpi_pmac(pn, mic_data, (mic_len >> BIT_OFFSET_4), key_ck, calc_mic, sizeof(calc_mic));

    /* 计算完mic后，释放mic data */
    hmac_wapi_mic_free(mic_data);
    if (ret != OAL_SUCC) {
        return OAL_ERR_CODE_WAPI_MIC_CALC_FAIL;
    }

    mic = oal_netbuf_data(buff) + mac_hdr_len + pdu_len;
    if (oal_memcmp(mic, calc_mic, SMS4_MIC_LEN) != 0) { /* 比较MIC */
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_wpi_decrypt::mic check fail!.}");
        WAPI_RX_MIC_ERR(wapi);
        return OAL_ERR_CODE_WAPI_MIC_CMP_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC void hmac_wapi_decrypt_set_cb(oal_netbuf_stru *buff_dst, uint8_t mac_hdr_len, mac_rx_ctl_stru *rx_ctl)
{
    MAC_GET_RX_CB_MAC_HEADER_ADDR(rx_ctl) = (uint32_t *)oal_netbuf_header(buff_dst);
    rx_ctl->uc_mac_header_len = mac_hdr_len;
    rx_ctl->us_frame_len = (uint16_t)oal_netbuf_len(buff_dst);
    return;
}


static uint32_t hmac_wapi_decrypt(hmac_wapi_stru *wapi, oal_netbuf_stru *netbuf)
{
    oal_netbuf_stru *buff_dst = NULL;
    uint8_t key_idx, mac_hdr_len;
    uint16_t pdu_len;
    uint32_t ret;
    mac_ieee80211_frame_stru *mac_hdr = NULL;
    uint8_t *wapi_hdr = NULL;
    uint8_t *pn = NULL;
    uint8_t *pdu = NULL;
    mac_rx_ctl_stru *rx_ctl = NULL;

    WAPI_RX_PORT_VALID(wapi);
    ret = hmac_wapi_decrypt_check(wapi, netbuf);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(netbuf);
        return ret;
    }

    hmac_wapi_decrypt_get_para(netbuf, &pdu_len, &mac_hdr_len, &mac_hdr, &wapi_hdr);
    key_idx = *wapi_hdr;
    pn = wapi_hdr + SMS4_KEY_IDX + SMS4_WAPI_HDR_RESERVE;
    pdu = pn + SMS4_PN_LEN;

    ret = hmac_wapi_netbuff_init(wapi, netbuf, mac_hdr_len, &buff_dst, &rx_ctl);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(netbuf);
        return ret;
    }

    hmac_wpi_swap_pn(pn, SMS4_PN_LEN);

    ret = hmac_wpi_decrypt(pn, pdu, (pdu_len + SMS4_MIC_LEN), wapi->ast_wapi_key[key_idx].auc_wpi_ek,
        (oal_netbuf_data(buff_dst) + mac_hdr_len));
    if (ret != OAL_SUCC) {
        oal_netbuf_free(buff_dst);
        oal_netbuf_free(netbuf);
        return OAL_ERR_CODE_WAPI_DECRYPT_FAIL;
    }

    /* mic作为校验数，不需要put */
    oal_netbuf_put(buff_dst, pdu_len);

    ret = hmac_wapi_calc_mic_proc(wapi, mac_hdr, buff_dst, pn, pdu_len, mac_hdr_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(buff_dst);
        oal_netbuf_free(netbuf);
        return ret;
    }

    /* 返回前清protected */
    (oal_netbuf_data(buff_dst))[1] &= ~0x40;

    /* 填写cb */
    hmac_wapi_decrypt_set_cb(buff_dst, mac_hdr_len, rx_ctl);

    oal_netbuf_next(buff_dst) = oal_netbuf_next(netbuf);
    oal_netbuf_next(netbuf) = buff_dst;

    hmac_wapi_pn_update(wapi->ast_wapi_key[wapi->uc_keyidx].auc_pn_rx, wapi->uc_pn_inc);

    oam_warning_log0(0, OAM_SF_ANY, "{hmac_wpi_decrypt::OK!.}");
    WAPI_RX_DECRYPT_OK(wapi);
    return OAL_SUCC;
}


static uint32_t hmac_wapi_encrypt(hmac_wapi_stru *pst_wapi, oal_netbuf_stru *pst_netbuf)
{
    mac_ieee80211_frame_stru *pst_mac_hdr = NULL;
    uint8_t uc_mac_hdr_len;
    uint8_t *puc_mic_data = NULL;
    uint16_t us_pdu_len;
    uint16_t us_mic_len;
    uint8_t uc_key_index;
    uint8_t *puc_key_ck = NULL;
    uint8_t *puc_key_ek = NULL;
    uint8_t auc_calc_mic[SMS4_MIC_LEN];
    uint8_t auc_pn_swap[SMS4_PN_LEN]; /* 保存变换后的pn,用来计算mic和加密 */
    oal_netbuf_stru *pst_netbuff_des = NULL;
    uint8_t *puc_wapi_hdr = NULL;
    uint8_t *puc_datain = NULL;
    uint32_t result;
    uint8_t *puc_payload = NULL;
    mac_tx_ctl_stru *pst_tx_ctl;
    int32_t ret;

    /************ 1. 加密前的数据准备,获取各头指针和内容长度 ************/
    /* 获取mac头 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    pst_mac_hdr = MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl);
    uc_mac_hdr_len = MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl);

    /* 设置加密位 注意，mac头涉及加密，所以需要在最开始设置 */
    mac_set_protectedframe((uint8_t *)pst_mac_hdr);

    oam_warning_log2(0, OAM_SF_ANY,
                     "{hmac_wapi_encrypt::uc_mac_hdr_len %u! bit_more==%u.}",
                     uc_mac_hdr_len, pst_mac_hdr->st_frame_control.bit_more_frag);
    us_pdu_len = MAC_GET_CB_MPDU_LEN(pst_tx_ctl);

    /* payload的起始位置在snap头的llc处 */
    puc_payload = oal_netbuf_data(pst_netbuf);

    /************ 2. 计算mic,wapi的数据帧一般为QOS帧  ************/
    puc_mic_data = hmac_wapi_mic_alloc(hmac_wapi_is_qos(pst_mac_hdr), us_pdu_len, &us_mic_len);
    if (puc_mic_data == NULL) {
        WAPI_TX_MEMALLOC_ERR(pst_wapi);

        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_encrypt::hmac_wapi_mic_alloc err!");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 计算mic预备数据 */
    uc_key_index = pst_wapi->uc_keyidx;
    ret = hmac_wapi_calc_mic_data(pst_mac_hdr, uc_key_index, puc_payload, us_pdu_len, puc_mic_data, us_mic_len);
    if (ret != OAL_SUCC) {
        return ret;
    }

    puc_key_ck = pst_wapi->ast_wapi_key[uc_key_index].auc_wpi_ck;
    ret = memcpy_s(auc_pn_swap, SMS4_PN_LEN, pst_wapi->ast_wapi_key[uc_key_index].auc_pn_tx, SMS4_PN_LEN);
    hmac_wpi_swap_pn(auc_pn_swap, SMS4_PN_LEN);
    /* 计算mic */
    result = hmac_wpi_pmac(auc_pn_swap, puc_mic_data, (us_mic_len >> BIT_OFFSET_4), puc_key_ck, auc_calc_mic,
        sizeof(auc_calc_mic));

    hmac_wapi_mic_free(puc_mic_data);
    if (result == OAL_FAIL) {
        WAPI_TX_MIC_ERR(pst_wapi);
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_encrypt::hmac_wpi_pmac mic calc err!");
        return OAL_ERR_CODE_WAPI_MIC_CALC_FAIL;
    }

    /************ 3. 准备新的netbuff,用来存放加密后的数据, 填写cb,并准备加密前的数据 ************/
    pst_netbuff_des = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuff_des == NULL) {
        WAPI_TX_MEMALLOC_ERR(pst_wapi);
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_encrypt::pst_netbuff_des alloc err!");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 填写cb */
    ret += memcpy_s(oal_netbuf_cb(pst_netbuff_des), MAC_TX_CTL_SIZE, oal_netbuf_cb(pst_netbuf), MAC_TX_CTL_SIZE);

    /* 先拷贝mac头,为了后续hcc处理，此处填写最大的空间 */
    oal_netbuf_init(pst_netbuff_des, MAC_80211_QOS_HTC_4ADDR_FRAME_LEN);
    ret += memcpy_s(oal_netbuf_data(pst_netbuff_des), uc_mac_hdr_len, pst_mac_hdr, uc_mac_hdr_len);

    puc_datain = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, us_pdu_len + SMS4_MIC_LEN, OAL_TRUE);
    if (puc_datain == NULL) {
        oal_netbuf_free(pst_netbuff_des);
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_encrypt::puc_datain alloc err!");
        WAPI_TX_MEMALLOC_ERR(pst_wapi);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    ret += memcpy_s(puc_datain, us_pdu_len + SMS4_MIC_LEN, puc_payload, us_pdu_len);
    /* 拷贝mic */
    ret += memcpy_s(puc_datain + us_pdu_len, SMS4_MIC_LEN, auc_calc_mic, SMS4_MIC_LEN);

    puc_key_ek = pst_wapi->ast_wapi_key[uc_key_index].auc_wpi_ek;
    /************************ 4. 加密 ************************/
    result = hmac_wpi_encrypt(auc_pn_swap,
        puc_datain, us_pdu_len + SMS4_MIC_LEN, puc_key_ek,
        oal_netbuf_data(pst_netbuff_des) + HMAC_WAPI_HDR_LEN + MAC_80211_QOS_HTC_4ADDR_FRAME_LEN);

    oal_mem_free_m(puc_datain, OAL_TRUE);
    if (result != OAL_SUCC) {
        oal_netbuf_free(pst_netbuff_des);
        oam_error_log1(0, OAM_SF_ANY, "{hmac_wapi_encrypt::hmac_wpi_encrypt err==%u!", result);
        return OAL_ERR_CODE_WAPI_ENRYPT_FAIL;
    }
    /* 此处put完之后，netbuff的len为mac头+pdulen+sms4+wapi的长度 */
    oal_netbuf_put(pst_netbuff_des, us_pdu_len + SMS4_MIC_LEN + HMAC_WAPI_HDR_LEN);

    /***************** 5. 填写wapi头 *****************/
    puc_wapi_hdr = oal_netbuf_data(pst_netbuff_des) + MAC_80211_QOS_HTC_4ADDR_FRAME_LEN;

    /* 填写WPI头 -- keyIndex */
    *(puc_wapi_hdr) = uc_key_index;
    /* 保留位清零 */
    *(puc_wapi_hdr + SMS4_KEY_IDX) = 0;
    /* 填写PN */
    ret += memcpy_s((puc_wapi_hdr + SMS4_KEY_IDX + SMS4_WAPI_HDR_RESERVE), SMS4_PN_LEN,
        pst_wapi->ast_wapi_key[uc_key_index].auc_pn_tx, SMS4_PN_LEN);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wapi_encrypt::memcpy fail!");
        oal_netbuf_free(pst_netbuff_des);
        return OAL_FAIL;
    }

    /* 再次填写cb */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuff_des);
    MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl) = (mac_ieee80211_frame_stru *)oal_netbuf_data(pst_netbuff_des);

    /* netbuf 的data指针指向payload */
    oal_netbuf_pull(pst_netbuff_des, MAC_80211_QOS_HTC_4ADDR_FRAME_LEN);

    /* 不包括mac hdr */
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = (uint16_t)(HMAC_WAPI_HDR_LEN + us_pdu_len + SMS4_MIC_LEN);
    oal_netbuf_next(pst_netbuff_des) = oal_netbuf_next(pst_netbuf);
    oal_netbuf_next(pst_netbuf) = pst_netbuff_des;
    /* 更新pn */
    hmac_wapi_pn_update(pst_wapi->ast_wapi_key[pst_wapi->uc_keyidx].auc_pn_tx, pst_wapi->uc_pn_inc);
    oam_warning_log0(0, OAM_SF_ANY, "{hmac_wapi_encrypt::hmac_wpi_encrypt OK!");

    WAPI_TX_ENCRYPT_OK(pst_wapi);
    return OAL_SUCC;
}


void hmac_wapi_deinit(hmac_wapi_stru *pst_wapi)
{
    memset_s(pst_wapi, sizeof(hmac_wapi_stru), 0, sizeof(hmac_wapi_stru));
}


void hmac_wapi_set_port(hmac_wapi_stru *pst_wapi, oal_bool_enum_uint8 value)
{
    WAPI_PORT_FLAG(pst_wapi) = value;
}


uint32_t hmac_wapi_init(hmac_wapi_stru *pst_wapi, uint8_t uc_pairwise)
{
    uint32_t loop;

    hmac_wapi_deinit(pst_wapi);

    if (uc_pairwise == OAL_TRUE) {
        pst_wapi->uc_pn_inc = WAPI_UCAST_INC;
        pst_wapi->wapi_is_pn_odd = hmac_wapi_is_pn_odd_ucast;
    } else {
        pst_wapi->uc_pn_inc = WAPI_BCAST_INC;
        pst_wapi->wapi_is_pn_odd = hmac_wapi_is_pn_odd_bcast;
    }

    for (loop = 0; loop < HMAC_WAPI_MAX_KEYID; loop++) {
        pst_wapi->ast_wapi_key[loop].uc_key_en = OAL_FALSE;
    }

    hmac_wapi_set_port(pst_wapi, OAL_FALSE);

    pst_wapi->wapi_decrypt = hmac_wapi_decrypt;
    pst_wapi->wapi_encrypt = hmac_wapi_encrypt;
    pst_wapi->wapi_netbuff_txhandle = hmac_wapi_netbuff_tx_handle;
    pst_wapi->wapi_netbuff_rxhandle = hmac_wapi_netbuff_rx_handle;
    return OAL_SUCC;
}

#ifdef _PRE_WAPI_DEBUG

void hmac_wapi_dump_frame(uint8_t *puc_info, uint8_t *puc_data, uint32_t len)
{
    uint32_t loop;
    oal_io_print("%s: \r\n", puc_info);
    for (loop = 0; loop < len; loop += 4) { // 按4字节轮询
        oal_io_print("%2x ", loop / 4); // 按4字节轮询
        oal_io_print("%2x %2x %2x %2x \r\n", puc_data[loop], puc_data[loop + 1],
                     puc_data[loop + BYTE_OFFSET_2], puc_data[loop + BYTE_OFFSET_3]);
    }
}


void hmac_wapi_display_usr_info(hmac_user_stru *pst_hmac_user)
{
    uint32_t loop;
    hmac_wapi_stru *pst_wapi = NULL;
    hmac_wapi_key_stru *pst_key = NULL;
    hmac_wapi_debug *pst_debug = NULL;

    oam_warning_log1(0, OAM_SF_ANY, "wapi port is %u!", WAPI_PORT_FLAG(&pst_hmac_user->st_wapi));
    if (WAPI_PORT_FLAG(&pst_hmac_user->st_wapi) != OAL_TRUE) {
        oal_io_print("Err! wapi port is not valid!\n");

        return;
    }

    pst_wapi = &(pst_hmac_user->st_wapi);
    oam_warning_log0(0, OAM_SF_ANY, "keyidx\tupdate\t\tpn_inc\t\n");
    oam_warning_log3(0, OAM_SF_ANY, "%u\t%08x%04x\t\n",
                     pst_wapi->uc_keyidx,
                     pst_wapi->uc_keyupdate_flg,
                     pst_wapi->uc_pn_inc);

    for (loop = 0; loop < HMAC_WAPI_MAX_KEYID; loop++) {
        pst_key = &pst_wapi->ast_wapi_key[loop];
        /*lint -e64*/
        hmac_wapi_dump_frame("ek :", pst_key->auc_wpi_ek, WAPI_KEY_LEN);
        hmac_wapi_dump_frame("ck :", pst_key->auc_wpi_ck, WAPI_KEY_LEN);
        hmac_wapi_dump_frame("pn_local_rx :", pst_key->auc_pn_rx, WAPI_PN_LEN);
        hmac_wapi_dump_frame("pn_local_tx :", pst_key->auc_pn_tx, WAPI_PN_LEN);
        oam_warning_log1(0, OAM_SF_ANY, "key_en: %u\n", pst_key->uc_key_en);
        /*lint +e64*/
    }

    pst_debug = &pst_wapi->st_debug;
    oam_warning_log0(0, OAM_SF_ANY, "TX DEBUG INFO:");
    /*lint -e64 */
    hmac_wapi_dump_frame("pn_rx :", pst_debug->aucrx_pn, WAPI_PN_LEN);
    /*lint +e64 */
    oam_warning_log4(0, OAM_SF_ANY, "tx_drop==%u, tx_wai==%u, tx_port_valid==%u, tx_memalloc_fail==%u",
                     pst_debug->ultx_ucast_drop,
                     pst_debug->ultx_wai,
                     pst_debug->ultx_port_valid,
                     pst_debug->ultx_memalloc_err);
    oam_warning_log3(0, OAM_SF_ANY, "tx_mic_calc_fail==%u, tx_encrypt_ok==%u, tx_memalloc_err==%u",
                     pst_debug->ultx_mic_calc_fail,
                     // pst_debug->ultx_drop_wai,
                     pst_debug->ultx_encrypt_ok,
                     pst_debug->ultx_memalloc_err);

    oam_warning_log0(0, OAM_SF_ANY, "RX DEBUG INFO:");
    oam_warning_log4(0, OAM_SF_ANY, "rx_port_valid==%u, rx_idx_err==%u, rx_netbuff_len_err==%u, rx_idx_update_err==%u",
                     pst_debug->ulrx_port_valid,
                     pst_debug->ulrx_idx_err,
                     pst_debug->ulrx_netbuff_len_err,
                     pst_debug->ulrx_idx_update_err);

    oam_warning_log4(0, OAM_SF_ANY, "rx_key_en_err==%u, rx_pn_odd_err==%u, rx_pn_replay_err==%u, rx_decrypt_ok==%u",
                     pst_debug->ulrx_key_en_err,
                     pst_debug->ulrx_pn_odd_err,
                     pst_debug->ulrx_pn_replay_err,
                     pst_debug->ulrx_decrypt_ok);
    return;
}


uint32_t hmac_wapi_display_info(mac_vap_stru *pst_mac_vap, uint16_t us_usr_idx)
{
    hmac_user_stru *pst_hmac_user = NULL;
    hmac_user_stru *pst_hmac_multi_user;

    pst_hmac_multi_user = (hmac_user_stru *)mac_res_get_hmac_user(pst_mac_vap->us_multi_user_idx);
    if (pst_hmac_multi_user == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "Err! multi usr %u does not exist!",
                         pst_mac_vap->us_multi_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "*****************multi usr info start****************");
    hmac_wapi_display_usr_info(pst_hmac_multi_user);
    oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "*****************multi usr info end****************");

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(us_usr_idx);
    if (pst_hmac_user == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "Err! ucast usr %u does not exist!", us_usr_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "*****************ucast usr info start****************");
    hmac_wapi_display_usr_info(pst_hmac_user);
    oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "*****************ucast usr info end****************");

    return OAL_SUCC;
}
#endif

#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

