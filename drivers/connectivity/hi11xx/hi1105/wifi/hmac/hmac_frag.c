
#include "hmac_frag.h"
#include "hmac_11i.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_FRAG_C

#define HMAC_FRAG_TIMEOUT  2000
#define HMAC_MAX_FRAG_SIZE 2500
#define HMAC_FRAG_HEADER_RESV_LEN 32


OAL_STATIC mac_ieee80211_frame_stru *hmac_frag_init_frag_netbuf(oal_netbuf_stru *pst_netbuf_original,
    mac_tx_ctl_stru *pst_tx_ctl, uint32_t offset, oal_netbuf_stru *pst_netbuf, uint32_t payload)
{
    mac_tx_ctl_stru *pst_tx_ctl_copy = NULL;
    mac_ieee80211_frame_stru *pst_mac_header;
    uint32_t mac_hdr_size;
    mac_ieee80211_frame_stru *pst_frag_header = NULL;

    mac_hdr_size = MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl);
    pst_mac_header = MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl);
    pst_mac_header->st_frame_control.bit_more_frag = OAL_TRUE;

    pst_tx_ctl_copy = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    /* 拷贝cb字段 */
    if (EOK != memcpy_s(pst_tx_ctl_copy, MAC_TX_CTL_SIZE, pst_tx_ctl, MAC_TX_CTL_SIZE)) {
        return NULL;
    }
    oal_netbuf_copy_queue_mapping(pst_netbuf, pst_netbuf_original);

    /* netbuf的headroom大于802.11 mac头长度 */
    pst_frag_header =
        (mac_ieee80211_frame_stru *)(get_netbuf_payload(pst_netbuf) - MAC_80211_QOS_HTC_4ADDR_FRAME_LEN);
    MAC_GET_CB_80211_MAC_HEAD_TYPE(pst_tx_ctl_copy) = 1; /* 指示mac头部在skb中 */

    /* 拷贝帧头内容 */
    if (EOK != memcpy_s(pst_frag_header, mac_hdr_size, pst_mac_header, mac_hdr_size)) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_frag_init_netbuf::memcpy fail!");
        return NULL;
    }

    oal_netbuf_copydata(pst_netbuf_original, offset, get_netbuf_payload(pst_netbuf), payload);
    oal_netbuf_set_len(pst_netbuf, payload);
    MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl_copy) = pst_frag_header;
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl_copy) = (uint16_t)payload;

    return pst_frag_header;
}

uint32_t hmac_frag_process(hmac_vap_stru *pst_hmac_vap,
    oal_netbuf_stru *pst_netbuf_original, mac_tx_ctl_stru *pst_tx_ctl,
    uint32_t cip_hdrsize, uint32_t max_tx_unit)
{
    mac_ieee80211_frame_stru *pst_frag_header = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;
    oal_netbuf_stru *pst_netbuf_prev = NULL;
    uint32_t total_hdrsize, frag_num, frag_size, payload, offset, mac_hdr_size;
    int32_t l_remainder;

    mac_hdr_size = MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl);
    total_hdrsize = mac_hdr_size + cip_hdrsize;
    frag_num = 1;
    /* 加密字节数包含在分片门限中，预留加密字节长度，由硬件填写加密头 */
    offset = max_tx_unit - cip_hdrsize - mac_hdr_size;
    l_remainder = (int32_t)(oal_netbuf_len(pst_netbuf_original) - offset);
    pst_netbuf_prev = pst_netbuf_original;

    do {
        frag_size = total_hdrsize + (uint32_t)l_remainder;

        /* 判断是否还有更多的分片 */
        if (frag_size > max_tx_unit) {
            frag_size = max_tx_unit;
        }

        /* 防止反复扩充头部空间预留32字节 */
        pst_netbuf = oal_netbuf_alloc(frag_size + MAC_80211_QOS_HTC_4ADDR_FRAME_LEN + HMAC_FRAG_HEADER_RESV_LEN,
            MAC_80211_QOS_HTC_4ADDR_FRAME_LEN + HMAC_FRAG_HEADER_RESV_LEN, 4); /* 4 字节对齐 */
        if (pst_netbuf == NULL) {
            /* 在外部释放之前申请的报文 */
            oam_error_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{hmac_frag_process::netbuf null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        /* 计算分片报文帧体长度 */
        payload = frag_size - total_hdrsize;

        /* 赋值分片号 */
        pst_frag_header = hmac_frag_init_frag_netbuf(pst_netbuf_original, pst_tx_ctl, offset,
                                                     pst_netbuf, payload);
        if (pst_frag_header == NULL) {
            oal_netbuf_free(pst_netbuf);
            return OAL_FAIL;
        }
        pst_frag_header->bit_frag_num = frag_num;
        frag_num++;
        oal_netbuf_next(pst_netbuf_prev) = pst_netbuf;
        pst_netbuf_prev = pst_netbuf;

        /* 计算下一个分片报文的长度和偏移 */
        l_remainder -= (int32_t)payload;
        offset += payload;
    } while (l_remainder > 0);

    pst_frag_header->st_frame_control.bit_more_frag = OAL_FALSE;
    oal_netbuf_next(pst_netbuf) = NULL;

    /* 原始报文作为分片报文的第一个 */
    oal_netbuf_trim(pst_netbuf_original, oal_netbuf_len(pst_netbuf_original) -
                    (max_tx_unit - cip_hdrsize - mac_hdr_size));
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = (uint16_t)(oal_netbuf_len(pst_netbuf_original));

    return OAL_SUCC;
}


uint32_t hmac_defrag_timeout_fn(void *p_arg)
{
    hmac_user_stru *pst_hmac_user;
    oal_netbuf_stru *pst_netbuf = NULL;
    pst_hmac_user = (hmac_user_stru *)p_arg;

    /* 超时后释放正在重组的分片报文 */
    if (pst_hmac_user->pst_defrag_netbuf != NULL) {
        pst_netbuf = pst_hmac_user->pst_defrag_netbuf;

        oal_mem_netbuf_trace(pst_netbuf, OAL_FALSE);
        oal_netbuf_free(pst_netbuf);
        pst_hmac_user->pst_defrag_netbuf = NULL;
    }

    return OAL_SUCC;
}

OAL_STATIC void hmac_defrag_same_frag_netbuf_check(hmac_user_stru *pst_hmac_user,
    mac_ieee80211_frame_stru *pst_mac_hdr)
{
    mac_ieee80211_frame_stru *pst_last_hdr = NULL;
    uint16_t us_rx_seq;
    uint8_t  uc_frag_num;
    uint16_t us_last_seq;
    uint8_t  uc_last_frag_num;

    us_rx_seq = pst_mac_hdr->bit_seq_num;
    uc_frag_num = (uint8_t)pst_mac_hdr->bit_frag_num;

    if (pst_hmac_user->pst_defrag_netbuf != NULL) {
        frw_timer_restart_timer(&pst_hmac_user->st_defrag_timer, HMAC_FRAG_TIMEOUT, OAL_FALSE);

        pst_last_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_data(pst_hmac_user->pst_defrag_netbuf);

        us_last_seq = pst_last_hdr->bit_seq_num;
        uc_last_frag_num = (uint8_t)pst_last_hdr->bit_frag_num;

        /* 如果地址不匹配，序列号不匹配，分片号不匹配则释放现在正在重组的报文 */
        if (us_rx_seq != us_last_seq ||
            uc_frag_num != (uc_last_frag_num + 1) ||
            oal_compare_mac_addr(pst_last_hdr->auc_address1, pst_mac_hdr->auc_address1) ||
            oal_compare_mac_addr(pst_last_hdr->auc_address2, pst_mac_hdr->auc_address2)) {
            frw_timer_immediate_destroy_timer_m(&pst_hmac_user->st_defrag_timer);
            oal_netbuf_free(pst_hmac_user->pst_defrag_netbuf);
            pst_hmac_user->pst_defrag_netbuf = NULL;
        }
    }
}

OAL_STATIC uint32_t hmac_defrag_first_frag_netbuf_process(hmac_user_stru *pst_hmac_user,
    oal_netbuf_stru *pst_netbuf)
{
    oal_netbuf_stru *pst_new_buf = NULL;
    mac_rx_ctl_stru *pst_rx_ctl = NULL;
    uint8_t uc_device_id;
    mac_device_stru *pst_mac_device = NULL;

    uc_device_id = pst_hmac_user->st_user_base_info.uc_device_id;
    pst_mac_device = mac_res_get_dev((uint32_t)uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log4(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY,
                       "{hmac_defrag_process::user index[%d] user mac:XX:XX:XX:%02X:%02X:%02X}",
                       pst_hmac_user->st_user_base_info.us_assoc_id,
                       pst_hmac_user->st_user_base_info.auc_user_mac_addr[3],  /* mac地址3 第四个字节 */
                       pst_hmac_user->st_user_base_info.auc_user_mac_addr[4],  /* mac地址4 第五个字节 */
                       pst_hmac_user->st_user_base_info.auc_user_mac_addr[5]); /* mac地址5 第六个字节 */
        /* user异常，丢弃报文 */
        oal_netbuf_free(pst_netbuf);
        oam_stat_vap_incr(pst_hmac_user->st_user_base_info.uc_vap_id, rx_defrag_process_dropped, 1);
        return OAL_FAIL;
    }

    /* 启动超时定时器，超时释放重组报文 */
    frw_timer_create_timer_m(&pst_hmac_user->st_defrag_timer,
                             hmac_defrag_timeout_fn,
                             HMAC_FRAG_TIMEOUT,
                             pst_hmac_user,
                             OAL_FALSE,
                             OAM_MODULE_ID_HMAC,
                             pst_mac_device->core_id);

    /* 内存池netbuf只有1600 可能不够，参照A公司申请2500操作系统原生态报文 */
    pst_new_buf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, HMAC_MAX_FRAG_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (pst_new_buf == NULL) {
        oam_error_log1(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY,
                       "{hmac_defrag_process::Alloc new_buf null,size[%d].}",
                       HMAC_MAX_FRAG_SIZE);
        /* 未申请成功重组分片的netbuf        需要释放分片netbuf */
        oal_netbuf_free(pst_netbuf);
        return OAL_FAIL;
    }
    oal_mem_netbuf_trace(pst_new_buf, OAL_FALSE);

#ifdef _PRE_WINDOWS_SUPPORT
    pst_new_buf->dev = pst_netbuf->dev;
#endif

    /* 将分片报文拷贝到新申请的报文中并挂接到用户结构体下，释放原有的报文 */
    oal_netbuf_init(pst_new_buf, oal_netbuf_len(pst_netbuf));
    oal_netbuf_copydata(pst_netbuf, 0, oal_netbuf_data(pst_new_buf), oal_netbuf_len(pst_netbuf));
    memcpy_s(oal_netbuf_cb(pst_new_buf), MAC_TX_CTL_SIZE, oal_netbuf_cb(pst_netbuf), MAC_TX_CTL_SIZE);
    pst_rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_new_buf);
    MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_ctl) = (uint32_t *)oal_netbuf_header(pst_new_buf);
    pst_hmac_user->pst_defrag_netbuf = pst_new_buf;

    /* 已拷贝到pst_defrag_netbuf 释放分片netbuf */
    oal_netbuf_free(pst_netbuf);
    return OAL_SUCC;
}

OAL_STATIC void hmac_defrag_non_first_frag_netbuf_process(hmac_user_stru *pst_hmac_user,
    oal_netbuf_stru *pst_netbuf, uint32_t hrdsize)
{
    mac_ieee80211_frame_stru *pst_mac_hdr = NULL;
    mac_ieee80211_frame_stru *pst_last_hdr = NULL;

    pst_mac_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_data(pst_netbuf);
    pst_last_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_data(pst_hmac_user->pst_defrag_netbuf);

    /* 此分片是期望的到来的分片，重启定时器，并进行重组 */
    frw_timer_restart_timer(&pst_hmac_user->st_defrag_timer, HMAC_FRAG_TIMEOUT, OAL_FALSE);

    /* 记录最新分片报文的分片号 */
    pst_last_hdr->bit_seq_num = pst_mac_hdr->bit_seq_num;
    pst_last_hdr->bit_frag_num = pst_mac_hdr->bit_frag_num;

    oal_netbuf_pull(pst_netbuf, hrdsize);

    /* 此接口内会调用dev_kfree_skb */
    oal_netbuf_concat(pst_hmac_user->pst_defrag_netbuf, pst_netbuf);
}

OAL_STATIC oal_netbuf_stru *hmac_defrag_last_netbuf_process(hmac_user_stru *pst_hmac_user)
{
    oal_netbuf_stru *netbuf = NULL;
    mac_ieee80211_frame_stru *pst_last_hdr = NULL;
    uint32_t ret;

    frw_timer_immediate_destroy_timer_m(&pst_hmac_user->st_defrag_timer);
    netbuf = pst_hmac_user->pst_defrag_netbuf;
    pst_hmac_user->pst_defrag_netbuf = NULL;

    /* 对重组好的报文进行mic检查 */
    ret = hmac_de_mic(pst_hmac_user, netbuf);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(netbuf);
        oam_stat_vap_incr(pst_hmac_user->st_user_base_info.uc_vap_id, rx_de_mic_fail_dropped, 1);
        oam_warning_log1(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY,
                         "{hmac_defrag_process::hmac_de_mic failed[%d].}", ret);
        return NULL;
    }

    pst_last_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_data(netbuf);
    pst_last_hdr->bit_frag_num = 0;
    return netbuf;
}

oal_netbuf_stru *hmac_defrag_process(hmac_user_stru *pst_hmac_user,
    oal_netbuf_stru *pst_netbuf, uint32_t hrdsize)
{
    mac_ieee80211_frame_stru *pst_mac_hdr = NULL;
    uint16_t us_rx_seq;
    uint8_t uc_frag_num;
    oal_bool_enum_uint8 en_more_frag;

    if (hrdsize == 0) {
        oal_netbuf_free(pst_netbuf);
        oam_warning_log0(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY,
                         "{hmac_defrag_process::mac head len is 0.}");
        return NULL;
    }

    pst_mac_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_data(pst_netbuf);
    us_rx_seq = pst_mac_hdr->bit_seq_num;
    uc_frag_num = (uint8_t)pst_mac_hdr->bit_frag_num;
    en_more_frag = (oal_bool_enum_uint8)pst_mac_hdr->st_frame_control.bit_more_frag;

    /* 如果没有什么可以去分片的则直接返回 */
    if (!en_more_frag && uc_frag_num == 0 && pst_hmac_user->pst_defrag_netbuf == NULL) {
        return pst_netbuf;
    }

    oal_mem_netbuf_trace(pst_netbuf, OAL_FALSE);

    /* 首先检查到来的分片报文是不是属于正在重组的分片报文 */
    hmac_defrag_same_frag_netbuf_check(pst_hmac_user, pst_mac_hdr);

    if (pst_hmac_user->pst_defrag_netbuf == NULL) {
        /* 没有分片,直接返回 */
        if (en_more_frag == 0 && uc_frag_num == 0) {
            return pst_netbuf;
        }
        /* 首片分片的分片号不为0则释放 */
        if (uc_frag_num != 0) {
            oal_netbuf_free(pst_netbuf);
            oam_stat_vap_incr(pst_hmac_user->st_user_base_info.uc_vap_id, rx_defrag_process_dropped, 1);
            oam_info_log3(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY,
                          "{hmac_defrag_process::The first frag_num is not Zero(%d), seq_num[%d], mor_frag[%d].}",
                          uc_frag_num, us_rx_seq, en_more_frag);
            return NULL;
        }
        /* 处理第一个分片报文 */
        if (hmac_defrag_first_frag_netbuf_process(pst_hmac_user, pst_netbuf) != OAL_SUCC) {
            return NULL;
        }
    } else {
        hmac_defrag_non_first_frag_netbuf_process(pst_hmac_user, pst_netbuf, hrdsize);
    }

    /* 未重组完毕，存在更多报文返回空指针 */
    if (en_more_frag) {
        return NULL;
    }
    /* 重组完毕返回组好的报文 */
    return hmac_defrag_last_netbuf_process(pst_hmac_user);
}