
#include "hmac_tx_amsdu.h"
#include "hmac_tx_data.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TX_AMSDU_C

#define AMSDU_HEADROOM_RSV_LEN 2  /* amsdu子头四字节对齐预留空间 */

#define AMSDU_ENABLE_ALL_TID 0xFF
/* amsdu下子msdu的最大长度 */
#define WLAN_MSDU_MAX_LEN 128
/* 1103 spec amsdu最大长度，对应WLAN_LARGE_NETBUF_SIZE，受制于一个buffer长度 */
#define WLAN_AMSDU_FRAME_MAX_LEN 7935

const mac_llc_snap_stru g_st_mac_11c_snap_header = {
    SNAP_LLC_LSAP,
    SNAP_LLC_LSAP,
    LLC_UI,
    {
        SNAP_RFC1042_ORGCODE_0,
        SNAP_RFC1042_ORGCODE_1,
        SNAP_RFC1042_ORGCODE_2,
    },
    0
};

OAL_STATIC uint32_t hmac_amsdu_tx_timeout_process(void *p_arg);
OAL_STATIC oal_bool_enum_uint8 hmac_tx_amsdu_is_overflow(hmac_amsdu_stru *pst_amsdu,
                                                         mac_tx_ctl_stru *pst_tx_ctl,
                                                         uint32_t frame_len,
                                                         hmac_user_stru *pst_user);
OAL_STATIC uint32_t hmac_amsdu_send(hmac_vap_stru *pst_vap, hmac_user_stru *pst_user, hmac_amsdu_stru *pst_amsdu);

/* 3 函数实现 */

OAL_STATIC void hmac_amsdu_prepare_to_send(hmac_vap_stru *pst_vap,
    hmac_user_stru *pst_user, hmac_amsdu_stru *pst_amsdu)
{
    uint32_t ret;

    /* 删除定时器 */
    frw_timer_immediate_destroy_timer_m(&pst_amsdu->st_amsdu_timer);
    oam_info_log4(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMSDU,
                  "{hmac_amsdu_prepare_to_send::amsdu size[%d],max masdu size[%d],msdu num[%d],max msdu num[%d].}",
                  pst_amsdu->amsdu_size, pst_amsdu->amsdu_maxsize,
                  pst_amsdu->msdu_num, pst_amsdu->amsdu_maxnum);

    ret = hmac_amsdu_send(pst_vap, pst_user, pst_amsdu);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log2(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMSDU,
                         "{hmac_amsdu_prepare_to_send::, amsdu send fails. erro[%d], short_pkt_num=%d.}",
                         ret, pst_amsdu->msdu_num);
    }
}


OAL_STATIC oal_bool_enum_uint8 hmac_tx_amsdu_is_overflow(hmac_amsdu_stru *pst_amsdu,
                                                         mac_tx_ctl_stru *pst_tx_ctl,
                                                         uint32_t frame_len,
                                                         hmac_user_stru *pst_user)
{
    mac_tx_ctl_stru *pst_head_ctl = NULL;
    oal_netbuf_stru *pst_head_buf = NULL;

    /* msdu链表中无msdu */
    pst_head_buf = oal_netbuf_peek(&pst_amsdu->st_msdu_head);
    if (pst_head_buf == NULL) {
        oam_info_log0(1, OAM_SF_TX, "{hmac_tx_amsdu_is_overflow:: The first msdu.}");
        return OAL_FALSE;
    }

    pst_head_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_head_buf);
    /* amsdu不为空，并且amsdu中的子帧来源(lan或者wlan)与当前要封装的netbuf不同，则将amsdu发送出去，
       这样做是因为在发送完成中释放一个mpdu时，是根据第一个netbuf的cb中填写的事件类型来选择释放策略，
       如果一个mpdu中的netbuf来源不同，会造成内存泄漏 */
    if (MAC_GET_CB_EVENT_TYPE(pst_tx_ctl) != MAC_GET_CB_EVENT_TYPE(pst_head_ctl)) {
        oam_info_log2(1, OAM_SF_TX, "{hmac_tx_amsdu_is_overflow::en_event_type mismatched. %d %d.}",
                      MAC_GET_CB_EVENT_TYPE(pst_tx_ctl), MAC_GET_CB_EVENT_TYPE(pst_head_ctl));
        return OAL_TRUE;
    }

    /* payload + padmax(3) 不能大于1568 */
    if (((pst_amsdu->amsdu_size + frame_len + SNAP_LLC_FRAME_LEN + 3) > WLAN_LARGE_NETBUF_SIZE) ||
        ((pst_amsdu->amsdu_size + frame_len + SNAP_LLC_FRAME_LEN) > WLAN_AMSDU_FRAME_MAX_LEN)) {
        oam_info_log4(1, OAM_SF_TX,
                      "{hmac_tx_amsdu_is_overflow::us_amsdu_size=%d us_amsdu_maxsize=%d framelen=%d uc_msdu_num=%d .}",
                      pst_amsdu->amsdu_size, pst_amsdu->amsdu_maxsize,
                      frame_len, pst_amsdu->msdu_num);
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


OAL_STATIC uint32_t hmac_amsdu_event_dispatch(hmac_vap_stru *pst_vap, oal_netbuf_stru *pst_net_buf)
{
    frw_event_mem_stru *pst_amsdu_send_event_mem = NULL;
    frw_event_stru *pst_amsdu_send_event = NULL;
    uint32_t ret;
    dmac_tx_event_stru *pst_amsdu_event = NULL;

    /* 抛事件 */
    pst_amsdu_send_event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (oal_unlikely(pst_amsdu_send_event_mem == NULL)) {
        oam_stat_vap_incr(pst_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
        oam_error_log0(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMPDU,
                       "{hmac_amsdu_send::pst_amsdu_send_event_mem null}");
        return OAL_FAIL;
    }

    /* 填事件头 */
    pst_amsdu_send_event = frw_get_event_stru(pst_amsdu_send_event_mem);
    if (oal_unlikely(pst_amsdu_send_event == NULL)) {
        oam_stat_vap_incr(pst_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
        oam_error_log0(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMPDU,
                       "{hmac_amsdu_send::pst_amsdu_send_event null}");
        frw_event_free_m(pst_amsdu_send_event_mem);
        return OAL_FAIL;
    }

    frw_event_hdr_init(&(pst_amsdu_send_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_DRX,
                       DMAC_TX_HOST_DRX,
                       sizeof(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_vap->st_vap_base_info.uc_chip_id,
                       pst_vap->st_vap_base_info.uc_device_id,
                       pst_vap->st_vap_base_info.uc_vap_id);

    pst_amsdu_event = (dmac_tx_event_stru *)(pst_amsdu_send_event->auc_event_data);
    pst_amsdu_event->pst_netbuf = pst_net_buf;

    ret = frw_event_dispatch_event(pst_amsdu_send_event_mem);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMPDU,
                         "hmac_amsdu_send::frw_event_dispatch_event fail[%d]", ret);
        return OAL_FAIL;
    }

    /* 释放事件内存 */
    frw_event_free_m(pst_amsdu_send_event_mem);
    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_amsdu_send(hmac_vap_stru *pst_vap, hmac_user_stru *pst_user, hmac_amsdu_stru *pst_amsdu)
{
    uint32_t ret;
    mac_tx_ctl_stru *pst_cb = NULL;
    oal_netbuf_stru *pst_net_buf = NULL;

    /* 给dmac传送的amsdu相关的信息以及802.11头挂接 */
    pst_net_buf = oal_netbuf_delist(&(pst_amsdu->st_msdu_head));
    if (oal_unlikely(pst_net_buf == NULL)) {
        oam_error_log0(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMPDU, "{hmac_amsdu_send::pst_net_buf null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_net_buf);

    /* amsdu只聚合一个帧时，回退成非amsdu，统一encap接口 */
    if (pst_amsdu->msdu_num == 1) {
        if (EOK != memmove_s(oal_netbuf_data(pst_net_buf) + SNAP_LLC_FRAME_LEN, OAL_MAC_ADDR_LEN + OAL_MAC_ADDR_LEN,
                             oal_netbuf_data(pst_net_buf), OAL_MAC_ADDR_LEN + OAL_MAC_ADDR_LEN)) {
            oam_error_log0(0, OAM_SF_AMPDU, "hmac_amsdu_send::memcpy fail!");
        }
        oal_netbuf_pull(pst_net_buf, SNAP_LLC_FRAME_LEN);

        MAC_GET_CB_IS_AMSDU(pst_cb) = OAL_FALSE;
        MAC_GET_CB_IS_FIRST_MSDU(pst_cb) = OAL_FALSE;
    }

    /* 把最后一个子帧的PAD去除 */
    oal_netbuf_trim(pst_net_buf, pst_amsdu->uc_last_pad_len);

    MAC_GET_CB_MPDU_LEN(pst_cb) = (uint16_t)oal_netbuf_len(pst_net_buf);
    MAC_GET_CB_MPDU_NUM(pst_cb) = 1;

    /* 为整个amsdu封装802.11头 */
    ret = hmac_tx_encap(pst_vap, pst_user, pst_net_buf);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_netbuf_free(pst_net_buf);
        oam_stat_vap_incr(pst_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
        oam_error_log1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMPDU,
                       "{hmac_amsdu_send::hmac_tx_encap failed[%d]}", ret);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_amsdu_event_dispatch(pst_vap, pst_net_buf);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_netbuf_free(pst_net_buf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 清零amsdu结构体信息 */
    pst_amsdu->amsdu_size = 0;
    pst_amsdu->msdu_num = 0;

    return ret;
}


OAL_STATIC uint32_t hmac_amsdu_append_submsdu(oal_netbuf_stru *pst_dest_buf,
    uint16_t us_msdu_offset, oal_netbuf_stru *src_msdu_buf)
{
    mac_ether_header_stru *pst_ether_head; /* 以太网过来的skb的以太网头 */
    mac_llc_snap_stru *pst_snap_head;      /* 为填写snap头的临时指针 */
    uint32_t frame_len;
    uint8_t *pst_msdu_payload;
    int32_t  l_ret;

    frame_len = oal_netbuf_get_len(src_msdu_buf);

    /* COPY ETH HEADER */
    pst_ether_head = (mac_ether_header_stru *)(oal_netbuf_data(pst_dest_buf) + us_msdu_offset);
    l_ret = memcpy_s((uint8_t *)pst_ether_head, ETHER_HDR_LEN, oal_netbuf_data(src_msdu_buf), ETHER_HDR_LEN);

    /* FILL LLC HEADER */
    pst_snap_head = (mac_llc_snap_stru *)((uint8_t *)pst_ether_head + ETHER_HDR_LEN);
    l_ret += memcpy_s((uint8_t *)pst_snap_head, SNAP_LLC_FRAME_LEN,
                      (uint8_t *)&g_st_mac_11c_snap_header, SNAP_LLC_FRAME_LEN);

    /* change type & length */
    pst_snap_head->us_ether_type = pst_ether_head->us_ether_type;
    pst_ether_head->us_ether_type =
        oal_byteorder_host_to_net_uint16((uint16_t)(frame_len - ETHER_HDR_LEN + SNAP_LLC_FRAME_LEN));

    /* COPY MSDU PAYLOAD */
    pst_msdu_payload = (uint8_t *)pst_snap_head + SNAP_LLC_FRAME_LEN;
    l_ret += memcpy_s(pst_msdu_payload, frame_len - ETHER_HDR_LEN,
                      oal_netbuf_data(src_msdu_buf) + ETHER_HDR_LEN, frame_len - ETHER_HDR_LEN);
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_AMSDU, "hmac_amsdu_append_submsdu::memcpy fail!");
        oal_netbuf_free(src_msdu_buf);
        return OAL_FAIL;
    }

    /* 释放旧msdu */
    oal_netbuf_free(src_msdu_buf);
    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE uint32_t hmac_amsdu_tx_encap_mpdu(hmac_vap_stru *pst_vap, hmac_user_stru *pst_user,
    hmac_amsdu_stru *pst_amsdu, oal_netbuf_stru *pst_buf)
{
    uint32_t msdu_len;
    uint32_t frame_len;
    uint32_t tailroom;
    oal_netbuf_stru *pst_dest_buf;
    uint16_t us_msdu_offset; /* 拷贝新msdu帧的偏移地址 */

    /* 协议栈来帧原始长 */
    frame_len = oal_netbuf_get_len(pst_buf);
    /* 4字节对齐后的msdu帧的长度 */
    msdu_len = OAL_ROUND_UP(frame_len, 4);
    /* msdu帧长 */
    msdu_len += SNAP_LLC_FRAME_LEN;

    pst_dest_buf = oal_netbuf_peek(&pst_amsdu->st_msdu_head);
    if (pst_dest_buf == NULL) {
        /* 链表中应该有netbuf */
        oam_error_log0(0, OAM_SF_AMSDU, "{hmac_amsdu_tx_encap_mpdu::oal_netbuf_peek return NULL}");
        return HMAC_TX_PASS;
    }

    /* 当期netbuf剩余空间少于msdu长 */
    tailroom = oal_netbuf_tailroom(pst_dest_buf);
    if (tailroom < msdu_len) {
        oam_error_log3(0, OAM_SF_AMSDU,
                       "{hmac_amsdu_tx_encap_mpdu::Notify1,tailroom[%d],msdu[%d],frame[%d]}",
                       tailroom, msdu_len, frame_len);
        /* 如果加上pad超出长度,尝试尾帧去掉pad */
        msdu_len = frame_len + SNAP_LLC_FRAME_LEN;
        if (tailroom < msdu_len) {
            hmac_amsdu_prepare_to_send(pst_vap, pst_user, pst_amsdu);
            return HMAC_TX_PASS;
        }
    }

    /* NEW MSDU OFFSET */
    us_msdu_offset = (uint16_t)oal_netbuf_get_len(pst_dest_buf);
    oam_info_log4(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMSDU,
                  "{hmac_amsdu_tx_encap_mpdu::frame len[%d], msdu len[%d], tailroom[%d] offset[%d].}",
                  frame_len, msdu_len, tailroom, us_msdu_offset);

    /* ETH HEAD + LLC + PAYLOAD */
    oal_netbuf_put(pst_dest_buf, msdu_len);

    if (hmac_amsdu_append_submsdu(pst_dest_buf, us_msdu_offset, pst_buf) != OAL_SUCC) {
        return HMAC_TX_PASS;
    }

    /* 更新amsdu信息 */
    pst_amsdu->msdu_num++;
    pst_amsdu->amsdu_size += (uint16_t)msdu_len;
    pst_amsdu->uc_last_pad_len = (uint8_t)(msdu_len - SNAP_LLC_FRAME_LEN - frame_len);

    tailroom = oal_netbuf_tailroom(pst_dest_buf);
    /* 当前netbuf剩余空间较少 || 已经达到聚合最大帧数 */
    if ((tailroom < HMAC_AMSDU_TX_MIN_LENGTH) || (pst_amsdu->msdu_num >= pst_amsdu->amsdu_maxnum)) {
        hmac_amsdu_prepare_to_send(pst_vap, pst_user, pst_amsdu);
    }

    /* 由于最新的msdu skb已经被释放,不管当前amsdu是否缓存或发送成功/失败,都需要返回TX BUFF */
    return HMAC_TX_BUFF;
}


OAL_STATIC OAL_INLINE uint32_t hmac_amsdu_alloc_netbuf(hmac_amsdu_stru *pst_amsdu, oal_netbuf_stru *pst_buf)
{
    oal_netbuf_stru *pst_dest_buf = NULL;
    mac_tx_ctl_stru *pst_cb = NULL;

    pst_dest_buf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_LARGE_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (pst_dest_buf == NULL) {
        return OAL_FAIL;
    }

    /* 子帧链入amsdu尾部 */
    oal_netbuf_add_to_list_tail(pst_dest_buf, &pst_amsdu->st_msdu_head);

    if (EOK != memcpy_s(oal_netbuf_cb(pst_dest_buf), sizeof(mac_tx_ctl_stru),
                        oal_netbuf_cb(pst_buf), sizeof(mac_tx_ctl_stru))) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_amsdu_alloc_netbuf::memcpy fail!");
        return OAL_FAIL;
    }

    oal_netbuf_copy_queue_mapping(pst_dest_buf, pst_buf);

    pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_dest_buf);
    MAC_GET_CB_IS_FIRST_MSDU(pst_cb) = OAL_TRUE;
    MAC_GET_CB_IS_AMSDU(pst_cb) = OAL_TRUE;
    MAC_GET_CB_NETBUF_NUM(pst_cb) = 1;

    return OAL_SUCC;
}


uint32_t hmac_amsdu_tx_process(hmac_vap_stru *pst_vap, hmac_user_stru *pst_user, oal_netbuf_stru *pst_buf)
{
    uint8_t uc_tid_no;
    uint32_t frame_len;
    uint32_t ret;
    hmac_amsdu_stru *pst_amsdu;
    mac_tx_ctl_stru *pst_tx_ctl;

    pst_tx_ctl = (mac_tx_ctl_stru *)(oal_netbuf_cb(pst_buf));
    frame_len = oal_netbuf_get_len(pst_buf);
    uc_tid_no = MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl);
    pst_amsdu = &(pst_user->ast_hmac_amsdu[uc_tid_no]);

    /* amsdu组帧溢出,将链表缓存帧发送并清空,新帧作为amsdu首帧入链表 */
    if (hmac_tx_amsdu_is_overflow(pst_amsdu, pst_tx_ctl, frame_len, pst_user)) {
        hmac_amsdu_prepare_to_send(pst_vap, pst_user, pst_amsdu);
    }

    if (pst_amsdu->msdu_num == 0) {
        oal_netbuf_list_head_init(&pst_amsdu->st_msdu_head);
        /* 申请netbuf用于聚合amsdu */
        if (OAL_SUCC != hmac_amsdu_alloc_netbuf(pst_amsdu, pst_buf)) {
            oam_warning_log0(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMSDU,
                             "{hmac_amsdu_tx_process::failed to alloc netbuf.}");
            return HMAC_TX_PASS;
        }

        /* 启动定时器 */
        frw_timer_create_timer_m(&pst_amsdu->st_amsdu_timer,
                                 hmac_amsdu_tx_timeout_process,
                                 HMAC_AMSDU_LIFE_TIME,
                                 pst_amsdu,
                                 OAL_FALSE,
                                 OAM_MODULE_ID_HMAC,
                                 pst_vap->st_vap_base_info.core_id);
    }

    /* 处理每一个msdu */
    ret = hmac_amsdu_tx_encap_mpdu(pst_vap, pst_user, pst_amsdu, pst_buf);
    return ret;
}

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU

void hmac_tx_encap_large_skb_amsdu_1103(hmac_vap_stru *hmac_vap, hmac_user_stru *user,
    oal_netbuf_stru *buf, mac_tx_ctl_stru *tx_ctl)
{
    mac_ether_header_stru *ether_hdr_temp;
    mac_ether_header_stru *ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(buf);
    uint8_t tid_no;
    uint16_t mpdu_len;
    uint16_t wlan_frame_len;
    int32_t l_ret;

    /* AMPDU+AMSDU功能未开启,由定制化门限决定，高于300Mbps时开启amsdu大包聚合 */
    if (g_st_tx_large_amsdu.en_tx_amsdu_level[hmac_vap->st_vap_base_info.uc_vap_id] == WLAN_TX_AMSDU_NONE) {
        return;
    }

    /* 针对关闭WMM，非QOS帧处理 */
    if (user->st_user_base_info.st_cap_info.bit_qos == OAL_FALSE) {
        return;
    }

    /* VO、组播队列不开启AMPDU+AMSDU */
    tid_no = MAC_GET_CB_WME_TID_TYPE(tx_ctl);
    if (tid_no >= WLAN_TIDNO_VOICE) {
        return;
    }

    /* 判断该tid是否支持AMPDU+AMSDU */
    if (OAL_FALSE == HMAC_USER_IS_AMSDU_SUPPORT(user, tid_no)) {
        return;
    }

    /* 非长帧不进行AMPDU+AMSDU */
    mpdu_len = (uint16_t)oal_netbuf_get_len(buf);
    if (mpdu_len > MAC_AMSDU_SKB_LEN_UP_LIMIT) {
        return;
    }

    /* 超出分片帧门限不进行AMPDU+AMSDU,计算时考虑需要新增的EHER HEAD LEN和字节对齐,MAC HEAD考虑最长帧头 */
    wlan_frame_len = mpdu_len + SNAP_LLC_FRAME_LEN + AMSDU_HEADROOM_RSV_LEN + MAC_80211_QOS_HTC_4ADDR_FRAME_LEN;
    if (wlan_frame_len > mac_mib_get_FragmentationThreshold(&hmac_vap->st_vap_base_info)) {
        return;
    }

    /* 已经是小包AMSDU聚合 */
    if (MAC_GET_CB_IS_AMSDU(tx_ctl) == OAL_TRUE) {
        return;
    }

    /* ETHER HEAD头部空闲空间,4字节对齐;一般此条件均成立,放置于最后 */
    if (oal_netbuf_headroom(buf) < (SNAP_LLC_FRAME_LEN + AMSDU_HEADROOM_RSV_LEN)) {
        return;
    }

    /* 80211 FRAME INCLUDE EHTER HEAD */
    MAC_GET_CB_AMSDU_LEVEL(tx_ctl) =
        oal_min(g_st_tx_large_amsdu.en_tx_amsdu_level[hmac_vap->st_vap_base_info.uc_vap_id],
                user->amsdu_level);

    /* 预留LLC HEAD长度 */
    oal_netbuf_push(buf, SNAP_LLC_FRAME_LEN);
    ether_hdr_temp = (mac_ether_header_stru *)oal_netbuf_data(buf);
    /* 拷贝mac head */
    l_ret = memmove_s((uint8_t *)ether_hdr_temp, SNAP_LLC_FRAME_LEN + ETHER_HDR_LEN,
                      (uint8_t *)ether_hdr, ETHER_HDR_LEN);
    if (l_ret != EOK) {
        oam_error_log1(0, OAM_SF_AMSDU, "{hmac_tx_encap_large_skb_amsdu::memmove fail[%d]}", l_ret);
        return;
    }

    /* 设置AMSDU帧长度 */
    ether_hdr_temp->us_ether_type =
        oal_byteorder_host_to_net_uint16((uint16_t)(mpdu_len - ETHER_HDR_LEN + SNAP_LLC_FRAME_LEN));
}

void hmac_tx_encap_large_skb_amsdu_1106(hmac_vap_stru *hmac_vap, hmac_user_stru *user,
    oal_netbuf_stru *buf, mac_tx_ctl_stru *tx_ctl)
{
    /* 06不生效大包AMPDU+AMSDU逻辑 */
    return;
}
#endif


OAL_STATIC uint32_t hmac_amsdu_check_tid_cap(hmac_vap_stru *pst_vap, hmac_user_stru *pst_user,
    oal_netbuf_stru *pst_buf)
{
    mac_tx_ctl_stru *pst_tx_ctl;
    uint8_t uc_tid_no;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_ip_header_stru *pst_ip = NULL;
    mac_ether_header_stru *pst_ether_header = (mac_ether_header_stru *)oal_netbuf_data(pst_buf);
#endif

    pst_tx_ctl = (mac_tx_ctl_stru *)(oal_netbuf_cb(pst_buf));
    uc_tid_no = MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl);

    /* 组播转单播数据不聚合 */
    if (pst_tx_ctl->bit_is_m2u_data) {
        return OAL_FALSE;
    }
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (pst_ether_header->us_ether_type == oal_host2net_short(ETHER_TYPE_IP)) {
        pst_ip = (oal_ip_header_stru *)(pst_ether_header + 1);
        /* 1103 允许TCP ACK聚合 */
        /* 为了解决业务量小时ping包延迟的问题 */
        if (oal_netbuf_is_icmp(pst_ip) == OAL_TRUE) {
            return OAL_FALSE;
        }
    }
#endif

    /* 判断该tid是否在ampdu情况下支持amsdu的发送,ampdu_amsdu 0/1 */
    if ((HMAC_USER_IS_AMSDU_SUPPORT(pst_user, uc_tid_no) == OAL_FALSE) ||
        (uc_tid_no == WLAN_TIDNO_VOICE)) {
        oam_info_log2(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMSDU,
                      "{hmac_amsdu_get_netbuf_cap::AMPDU NOT SUPPORT AMSDU uc_tid_no=%d uc_amsdu_supported=%d}",
                      uc_tid_no, pst_user->uc_amsdu_supported);
        return OAL_FALSE;
    }

    if (oal_unlikely(uc_tid_no >= WLAN_TID_MAX_NUM)) {
        oam_error_log0(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMSDU,
                       "{hmac_amsdu_get_netbuf_cap::invalid tid number from the cb in asmdu notify function}");
        return OAL_FALSE;
    }
    return OAL_TRUE;
}


OAL_STATIC uint32_t hmac_amsdu_check_vap_user_cap(hmac_vap_stru *pst_vap, hmac_user_stru *pst_user)
{
    /* 针对关闭WMM，非QOS帧处理 */
    if (pst_user->st_user_base_info.st_cap_info.bit_qos == OAL_FALSE) {
        oam_info_log0(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
            "{hmac_amsdu_check_vap_user_cap::UnQos Frame pass!!}");
        return OAL_FALSE;
    }

    /* 检查amsdu开关是否打开,amsdu_tx_on 0/1; VAP 是否支持聚合 */
    if ((mac_mib_get_CfgAmsduTxAtive(&pst_vap->st_vap_base_info) != OAL_TRUE) ||
        (mac_mib_get_AmsduAggregateAtive(&pst_vap->st_vap_base_info) != OAL_TRUE)) {
        oam_info_log0(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMSDU,
                      "{hmac_amsdu_check_vap_user_cap::amsdu is unenable in amsdu notify}");
        return OAL_FALSE;
    }

    /* 检查用户是否是HT/VHT */
    if (hmac_user_xht_support(pst_user) == OAL_FALSE) {
        oam_info_log0(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMSDU,
                      "{hmac_amsdu_check_vap_user_cap::user is not qos in amsdu notify}");
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

uint32_t hmac_amsdu_notify(hmac_vap_stru *pst_vap, hmac_user_stru *pst_user, oal_netbuf_stru *pst_buf)
{
    uint32_t ret; /* 所调用函数的返回值 */
    hmac_amsdu_stru *pst_amsdu = NULL;
    mac_tx_ctl_stru *pst_tx_ctl;
    uint8_t uc_tid_no;

    pst_tx_ctl = (mac_tx_ctl_stru *)(oal_netbuf_cb(pst_buf));
    uc_tid_no = MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl);

    if (hmac_amsdu_check_vap_user_cap(pst_vap, pst_user) == OAL_FALSE) {
        return HMAC_TX_PASS;
    }

    if (hmac_amsdu_check_tid_cap(pst_vap, pst_user, pst_buf) == OAL_FALSE) {
        return HMAC_TX_PASS;
    }

    pst_amsdu = &(pst_user->ast_hmac_amsdu[uc_tid_no]);
    oal_spin_lock_bh(&pst_amsdu->st_amsdu_lock);

    /* 新来帧是大帧,需将缓存帧发完 */
    if (oal_netbuf_get_len(pst_buf) > WLAN_MSDU_MAX_LEN) {
        /* 防止乱序,应该先发送旧帧 */
        if (pst_amsdu->msdu_num) {
            hmac_amsdu_prepare_to_send(pst_vap, pst_user, pst_amsdu);
        }
        oal_spin_unlock_bh(&pst_amsdu->st_amsdu_lock);
        return HMAC_TX_PASS;
    }

    ret = hmac_amsdu_tx_process(pst_vap, pst_user, pst_buf);
    oal_spin_unlock_bh(&pst_amsdu->st_amsdu_lock);
    return ret;
}


OAL_STATIC uint32_t hmac_amsdu_tx_timeout_process(void *p_arg)
{
    hmac_amsdu_stru *pst_temp_amsdu = NULL;
    mac_tx_ctl_stru *pst_cb = NULL;
    hmac_user_stru *pst_user = NULL;
    uint32_t ret;
    oal_netbuf_stru *pst_netbuf = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    if (oal_unlikely(p_arg == NULL)) {
        oam_error_log0(0, OAM_SF_AMPDU, "{hmac_amsdu_tx_timeout_process::input null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_temp_amsdu = (hmac_amsdu_stru *)p_arg;

    oal_spin_lock_bh(&pst_temp_amsdu->st_amsdu_lock);

    /* 根据要发送的amsdu下第一个msdu子帧的cb字段的信息寻找对应用户结构体 */
    pst_netbuf = oal_netbuf_peek(&pst_temp_amsdu->st_msdu_head);
    if (pst_netbuf == NULL) {
        oam_info_log1(0, OAM_SF_AMSDU,
                      "hmac_amsdu_tx_timeout_process::pst_netbuf NULL. msdu_num[%d]",
                      pst_temp_amsdu->msdu_num);
        oal_spin_unlock_bh(&pst_temp_amsdu->st_amsdu_lock);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(MAC_GET_CB_TX_VAP_INDEX(pst_cb));
    if (oal_unlikely(pst_hmac_vap == NULL)) {
        oal_spin_unlock_bh(&pst_temp_amsdu->st_amsdu_lock);
        oam_error_log0(0, OAM_SF_AMPDU, "{hmac_amsdu_tx_timeout_process::pst_hmac_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_user = (hmac_user_stru *)mac_res_get_hmac_user(MAC_GET_CB_TX_USER_IDX(pst_cb));
    if (oal_unlikely(pst_user == NULL)) {
        oal_spin_unlock_bh(&pst_temp_amsdu->st_amsdu_lock);
        oam_warning_log1(0, OAM_SF_AMPDU, "{hmac_amsdu_tx_timeout_process::pst_user[%d] null}",
            MAC_GET_CB_TX_USER_IDX(pst_cb));
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_amsdu_send(pst_hmac_vap, pst_user, pst_temp_amsdu);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMSDU,
                         "hmac_amsdu_tx_timeout_process::hmac_amsdu_send fail[%d]", ret);
    }

    oal_spin_unlock_bh(&pst_temp_amsdu->st_amsdu_lock);
    oam_info_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMSDU,
                  "hmac_amsdu_tx_timeout_process::hmac_amsdu_send SUCC");

    return OAL_SUCC;
}


void hmac_amsdu_init_user(hmac_user_stru *pst_hmac_user_sta)
{
    uint32_t amsdu_idx;
    hmac_amsdu_stru *pst_amsdu = NULL;

    if (pst_hmac_user_sta == NULL) {
        oam_error_log0(0, OAM_SF_AMPDU, "{hmac_amsdu_init_user::pst_hmac_user_sta null}");
        return;
    }

    pst_hmac_user_sta->amsdu_maxsize = WLAN_AMSDU_FRAME_MAX_LEN_LONG;

    pst_hmac_user_sta->uc_amsdu_supported = AMSDU_ENABLE_ALL_TID;
    hmac_user_set_amsdu_level(pst_hmac_user_sta, WLAN_TX_AMSDU_BY_4);

    /* 设置amsdu域 */
    for (amsdu_idx = 0; amsdu_idx < WLAN_TID_MAX_NUM; amsdu_idx++) {
        pst_amsdu = &(pst_hmac_user_sta->ast_hmac_amsdu[amsdu_idx]);

        oal_spin_lock_init(&pst_amsdu->st_amsdu_lock);
        oal_netbuf_list_head_init(&(pst_amsdu->st_msdu_head));
        pst_amsdu->amsdu_size = 0;

        hmac_amsdu_set_maxnum(pst_amsdu, WLAN_AMSDU_MAX_NUM);

        hmac_amsdu_set_maxsize(pst_amsdu, pst_hmac_user_sta, WLAN_AMSDU_FRAME_MAX_LEN_LONG);
    }
}
