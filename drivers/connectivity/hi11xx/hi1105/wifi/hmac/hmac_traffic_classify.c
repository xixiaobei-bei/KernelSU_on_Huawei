

/* 1 头文件包含 */
#include "hmac_traffic_classify.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TRAFFIC_CLASSIFY_C

/* 2 宏定义 */
#define RTP_VERSION          2      /* RTP协议版本号，占2位，当前协议版本号为2 */
#define RTP_VER_SHIFT        6      /* RTP协议版本号位移量 */
#define RTP_PT_MASK          BIT7   /* RTP的payload_type，有效载荷占前7位，bit7 mark位暂不使用 */
#define RTP_CSRC_MASK        0x0f   /* CSRC计数器，占4位，指示CSRC标识符的个数 */
#define RTP_CSRC_LEN_BYTE    4      /* 每个CSRC标识符占32位，共4字节 */
#define RTP_HDR_LEN_BYTE     12     /* RTP帧头固定字节数(不包含CSRC字段) */
#define TCP_HTTP_VI_LEN_THR  1000   /* HTTP视频流报文长度阈值 */
#define JUDGE_CACHE_LIFETIME 1      /* 待识别队列失效时间: 1s */
#define IP_FRAGMENT_MASK     0x1FFF /* IP分片Fragment Offset字段 */
/* RTP Payload_Type 编号:RFC3551 */
#define RTP_PT_VO_G729 18 /* RTP载荷类型:18-Audio-G729 */
#define RTP_PT_VI_CELB 25 /* RTP载荷类型:25-Video-CelB */
#define RTP_PT_VI_JPEG 26 /* RTP载荷类型:26-Video-JPEG */
#define RTP_PT_VI_NV   28 /* RTP载荷类型:28-Video-nv */
#define RTP_PT_VI_H261 31 /* RTP载荷类型:31-Video-H261 */
#define RTP_PT_VI_MPV  32 /* RTP载荷类型:32-Video-MPV */
#define RTP_PT_VI_MP2T 33 /* RTP载荷类型:33-Video-MP2T */
#define RTP_PT_VI_H263 34 /* RTP载荷类型:34-Video-H263 */
/* HTTP流媒体端口 */
#define HTTP_PORT_80   80   /* HTTP协议默认端口号80 */
#define HTTP_PORT_8080 8080 /* HTTP协议默认端口号8080 */

/* 3 函数实现 */

OAL_STATIC uint32_t hmac_tx_add_cfm_traffic(hmac_user_stru *pst_hmac_user,
                                            uint8_t uc_tid,
                                            hmac_tx_major_flow_stru *pst_max)
{
    unsigned long time_stamp = 0;
    uint8_t uc_mark = 0;
    uint8_t uc_traffic_idx = 0;

    hmac_tx_cfm_flow_stru *pst_cfm_info = NULL;

    if (pst_hmac_user->uc_cfm_num == MAX_CONFIRMED_FLOW_NUM) {
        /* 已识别列表已满，将列表中最长时间没有来包的业务进行替换 */
        time_stamp = pst_hmac_user->ast_cfm_flow_list[uc_traffic_idx].last_jiffies;

        for (uc_traffic_idx = 1; uc_traffic_idx < MAX_CONFIRMED_FLOW_NUM; uc_traffic_idx++) {
            pst_cfm_info = (pst_hmac_user->ast_cfm_flow_list + uc_traffic_idx);
            /*lint -e718 */ /*lint -e746 */
            if (oal_time_after(time_stamp, pst_cfm_info->last_jiffies)) {
                time_stamp = pst_cfm_info->last_jiffies;
                uc_mark = uc_traffic_idx;
            }
            /*lint +e718 */ /*lint +e746 */
        }
    } else {
        /* 已识别列表不满，找到可记录的index */
        for (uc_traffic_idx = 0; uc_traffic_idx < MAX_CONFIRMED_FLOW_NUM; uc_traffic_idx++) {
            pst_cfm_info = (pst_hmac_user->ast_cfm_flow_list + uc_traffic_idx);
            if (pst_cfm_info->us_cfm_flag == OAL_FALSE) {
                uc_mark = uc_traffic_idx;
                pst_hmac_user->uc_cfm_num++;
                pst_cfm_info->us_cfm_flag = OAL_TRUE;
                break;
            }
        }
    }

    /* 更新列表 */
    pst_cfm_info = (pst_hmac_user->ast_cfm_flow_list + uc_mark);

    memcpy_s(&pst_cfm_info->st_cfm_flow_info,
             sizeof(hmac_tx_flow_info_stru),
             &pst_max->st_flow_info,
             sizeof(hmac_tx_flow_info_stru));

    pst_cfm_info->us_cfm_tid = uc_tid;
    pst_cfm_info->last_jiffies = OAL_TIME_JIFFY;

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_tx_traffic_judge(hmac_user_stru *pst_hmac_user,
                                          hmac_tx_major_flow_stru *pst_major_flow,
                                          uint8_t *puc_tid)
{
    uint32_t ret = OAL_FAIL;
    uint8_t uc_cache_idx;
    uint32_t pt;

    hmac_tx_judge_list_stru *pst_judge_list = &(pst_hmac_user->st_judge_list);
    hmac_tx_judge_info_stru *pst_judge_info = NULL;

    /* 主要业务帧为UDP帧，进行RTP帧检测 */
    for (uc_cache_idx = 0; uc_cache_idx < MAX_JUDGE_CACHE_LENGTH; uc_cache_idx++) {
        pst_judge_info = (hmac_tx_judge_info_stru *)(pst_judge_list->ast_judge_cache + uc_cache_idx);

        if (!oal_memcmp(&pst_judge_info->st_flow_info,
                        &pst_major_flow->st_flow_info,
                        sizeof(hmac_tx_flow_info_stru))) {
            /* RTP帧判断标准:version位保持为2，SSRC、PT保持不变，且帧长度大于RTP包头长度 */
            if (((pst_judge_info->uc_rtpver >> RTP_VER_SHIFT) != RTP_VERSION) ||
                (pst_major_flow->rtpssrc != pst_judge_info->rtpssrc) ||
                (pst_major_flow->payload_type != pst_judge_info->payload_type) ||
                (pst_major_flow->average_len <
                 (uint32_t)(pst_judge_info->uc_rtpver & RTP_CSRC_MASK) * RTP_CSRC_LEN_BYTE + RTP_HDR_LEN_BYTE)) {
                pst_hmac_user->st_judge_list.to_judge_num = 0; /* 识别失败，清空队列 */
                pst_hmac_user->us_clear_judge_count += 1;
                return OAL_FAIL;
            }
        }
    }

    pt = pst_major_flow->payload_type;
    if (pt <= RTP_PT_VO_G729) { /* 依据PayloadType判断RTP载荷类型 */
        *puc_tid = WLAN_TIDNO_VOICE;
    } else if ((pt == RTP_PT_VI_CELB) || (pt == RTP_PT_VI_JPEG) ||
               (pt == RTP_PT_VI_NV) || ((pt >= RTP_PT_VI_H261) && (pt <= RTP_PT_VI_H263))) {
        *puc_tid = WLAN_TIDNO_VIDEO;
    }

    /* 识别成功，更新用户已识别流列表 */
    if ((*puc_tid == WLAN_TIDNO_VOICE) || (*puc_tid == WLAN_TIDNO_VIDEO)) {
        ret = hmac_tx_add_cfm_traffic(pst_hmac_user, *puc_tid, pst_major_flow);
    }
    pst_hmac_user->st_judge_list.to_judge_num = 0; /* 识别完成，清空队列 */

    return ret;
}

static uint32_t hmac_tx_find_major_traffic_succ(hmac_tx_major_flow_stru max, hmac_user_stru *hmac_user,
                                                uint8_t *tid)
{
    if (max.wait_check_num >= (MAX_JUDGE_CACHE_LENGTH >> 1)) {
        /* 已找到主要业务流，不必继续搜索 */
        max.average_len = max.average_len / max.wait_check_num;
        return OAL_SUCC;
    }

    if (max.wait_check_num < (MAX_JUDGE_CACHE_LENGTH >> 2)) {  /* 右移2位获得检查门限 */
        /* 认为没有主要业务流 */
        hmac_user->st_judge_list.to_judge_num = 0; /* 清空队列 */
        hmac_user->us_clear_judge_count += 1;
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_tx_find_major_traffic(hmac_user_stru *pst_hmac_user, uint8_t *puc_tid)
{
    uint8_t uc_cache_idx_i, uc_cache_idx_j;
    hmac_tx_major_flow_stru st_mark = { { 0 }, 0 };
    hmac_tx_major_flow_stru st_max = { { 0 }, 0 };
    hmac_tx_judge_list_stru *pst_judge_list = &(pst_hmac_user->st_judge_list);
    hmac_tx_judge_info_stru *pst_judge_info = NULL;

    /* 初始化 */
    st_max.wait_check_num = 0;

    /* 队列超时，清空队列记录 */
    if (((int32_t)pst_judge_list->jiffies_end - (int32_t)pst_judge_list->jiffies_st) >
        (JUDGE_CACHE_LIFETIME * OAL_TIME_HZ)) {           /* 强制转换为long防止jiffies溢出 */
        pst_hmac_user->st_judge_list.to_judge_num = 0; /* 清空队列 */
        return OAL_FAIL;
    }

    /* 队列已满且未超时 */
    for (uc_cache_idx_i = 0; uc_cache_idx_i < (MAX_JUDGE_CACHE_LENGTH >> 1); uc_cache_idx_i++) {
        pst_judge_info = (hmac_tx_judge_info_stru *)(pst_judge_list->ast_judge_cache + uc_cache_idx_i);

        if (pst_judge_info->uc_flag == OAL_FALSE) {
            continue;
        }

        pst_judge_info->uc_flag = OAL_FALSE;
        if (EOK != memcpy_s(&st_mark, sizeof(hmac_tx_major_flow_stru),
                            pst_judge_info, sizeof(hmac_tx_judge_info_stru))) {
            oam_error_log0(0, OAM_SF_ANY, "hmac_tx_find_major_traffic::memcpy fail!");
            return OAL_FAIL;
        }
        st_mark.wait_check_num = 1;

        for (uc_cache_idx_j = 0; uc_cache_idx_j < MAX_JUDGE_CACHE_LENGTH; uc_cache_idx_j++) {
            pst_judge_info = (hmac_tx_judge_info_stru *)(pst_judge_list->ast_judge_cache + uc_cache_idx_j);

            if ((pst_judge_info->uc_flag == OAL_TRUE) && !oal_memcmp(&pst_judge_info->st_flow_info,
                                                                     &st_mark.st_flow_info,
                                                                     sizeof(hmac_tx_flow_info_stru))) {
                pst_judge_info->uc_flag = OAL_FALSE;
                st_mark.average_len += pst_judge_info->len;
                st_mark.wait_check_num += 1;
            }

            if (st_mark.wait_check_num <= st_max.wait_check_num) {
                continue;
            }
            memcpy_s(&st_max, sizeof(hmac_tx_major_flow_stru),
                     &st_mark, sizeof(hmac_tx_major_flow_stru));
            if (hmac_tx_find_major_traffic_succ(st_max, pst_hmac_user, puc_tid) == OAL_SUCC) {
                return hmac_tx_traffic_judge(pst_hmac_user, &st_max, puc_tid);
            }
        }
    }

    if (hmac_tx_find_major_traffic_succ(st_max, pst_hmac_user, puc_tid) != OAL_SUCC) {
        return OAL_FAIL;
    }

    st_max.average_len = st_max.average_len / st_max.wait_check_num;
    return hmac_tx_traffic_judge(pst_hmac_user, &st_max, puc_tid);
}


static uint32_t hmac_tx_traffic_is_udp_ip(mac_ip_header_stru *ip, uint8_t *tid, mac_tx_ctl_stru *tx_ctl)
{   /* 功能裁剪，只处理UDP报文，以及识别WifiDisplay RTSP业务为VO */
    if (ip->uc_protocol != MAC_UDP_PROTOCAL) {
        if (ip->uc_protocol == MAC_TCP_PROTOCAL) {
            mac_tcp_header_stru *pst_tcp_hdr = (mac_tcp_header_stru *)(ip + 1);

            /* 识别WifiDisplay RTSP业务为VO */
            if (oal_ntoh_16(MAC_WFD_RTSP_PORT) == pst_tcp_hdr->us_sport) {
                *tid = WLAN_TIDNO_VOICE;
                MAC_GET_CB_FRAME_TYPE(tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
                MAC_GET_CB_FRAME_SUBTYPE(tx_ctl) = MAC_DATA_VIP_FRAME;
                MAC_GET_CB_IS_NEEDRETRY(tx_ctl) = OAL_TRUE;
                MAC_GET_CB_IS_RTSP(tx_ctl) = OAL_TRUE;
                return OAL_FAIL;
            }
        }
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

static uint32_t hmac_tx_traffic_identified_flow_info(hmac_user_stru *hmac_user, hmac_tx_flow_info_stru *flow_info,
                                                     uint8_t *tid)
{
    uint8_t loop;
    hmac_tx_cfm_flow_stru *cfm_info = NULL;

    for (loop = 0; loop < hmac_user->uc_cfm_num; loop++) {
        cfm_info = (hmac_tx_cfm_flow_stru *)(hmac_user->ast_cfm_flow_list + loop);
        if (!oal_memcmp(&cfm_info->st_cfm_flow_info,
                        flow_info,
                        sizeof(hmac_tx_flow_info_stru))) {
            *tid = (uint8_t)(cfm_info->us_cfm_tid);
            cfm_info->last_jiffies = OAL_TIME_JIFFY; /* 更新业务最新来包时间 */
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

static uint32_t hmac_tx_traffic_to_be_identified_flow_info(hmac_user_stru *hmac_user,
                                                           hmac_tx_judge_list_stru *judge_list,
                                                           hmac_tx_rtp_hdr *rtp_hdr,
                                                           hmac_tx_flow_info_stru *flow_info,
                                                           hmac_tx_judge_info_stru *judge_info)
{
    if (judge_list->to_judge_num >= MAX_JUDGE_CACHE_LENGTH) { /* 队列已满，识别过程中的来包不作记录 */
        return OAL_FAIL;
    }

    judge_list->jiffies_end = OAL_TIME_JIFFY;    /* 更新最新来包时间 */
    if (judge_list->to_judge_num == 0) {         /* 若队列为空 */
        judge_list->jiffies_st = OAL_TIME_JIFFY; /* 更新队列产生时间 */
    }
    judge_list->to_judge_num += 1; /* 更新队列长度 */

    memset_s(judge_info, sizeof(hmac_tx_judge_info_stru), 0, sizeof(hmac_tx_judge_info_stru));
    memcpy_s(&(judge_info->st_flow_info), sizeof(hmac_tx_flow_info_stru),
             flow_info, sizeof(hmac_tx_flow_info_stru));
    judge_info->uc_flag = OAL_TRUE;
    judge_info->uc_rtpver = rtp_hdr->uc_version_and_csrc;
    judge_info->payload_type = (uint32_t)((rtp_hdr->uc_payload_type) & (~RTP_PT_MASK));
    judge_info->rtpssrc = rtp_hdr->ssrc;

    return OAL_SUCC;
}

static void hmac_tx_traffic_pre_identify(hmac_user_stru *hmac_user, hmac_tx_judge_list_stru *judge_list,
                                         hmac_tx_rtp_hdr *rtp_hdr, uint8_t *tid)
{
    uint8_t rtp_payload_type;
    uint8_t rtp_ver;
    if ((hmac_user->us_clear_judge_count < MAX_CLEAR_JUDGE_TH) &&
        (judge_list->to_judge_num >= 1)) {
        rtp_payload_type = (rtp_hdr->uc_payload_type) & (~RTP_PT_MASK);
        rtp_ver = rtp_hdr->uc_version_and_csrc >> RTP_VER_SHIFT;
        if (rtp_ver == RTP_VERSION) {
            if (rtp_payload_type == RTP_PT_VO_G729) {
                oam_warning_log2(0, 0, "hmac_tx_traffic_classify:rtp seq=0x%x tid=%d to WLAN_TIDNO_VOICE",
                                 rtp_hdr->us_rtp_idx, *tid);
                *tid = WLAN_TIDNO_VOICE;
            } else if ((rtp_payload_type == RTP_PT_VI_CELB) ||
                       (rtp_payload_type == RTP_PT_VI_JPEG) || (rtp_payload_type == RTP_PT_VI_NV) ||
                       ((rtp_payload_type >= RTP_PT_VI_H261) && (rtp_payload_type <= RTP_PT_VI_H263))) {
                oam_warning_log2(0, 0, "hmac_tx_traffic_classify:rtp seq=0x%x tid=%d to WLAN_TIDNO_VIDEO",
                                 rtp_hdr->us_rtp_idx, *tid);
                *tid = WLAN_TIDNO_VIDEO;
            }
        }
    }
}
/*
 * 函 数 名  : hmac_tx_traffic_classify
 * 功能描述  : 对下行数据包进行处理:
 * 修改内容  : 生成新函数
 */
void hmac_tx_traffic_classify(mac_tx_ctl_stru *pst_tx_ctl,
                              mac_ip_header_stru *pst_ip,
                              uint8_t *puc_tid)
{
    udp_hdr_stru *pst_udp_hdr = NULL;
    hmac_tx_rtp_hdr *pst_rtp_hdr = NULL;
    hmac_tx_flow_info_stru st_flow_info;
    hmac_tx_judge_list_stru *pst_judge_list = NULL;
    hmac_tx_judge_info_stru *judge_info = NULL;
    
    hmac_user_stru *pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(MAC_GET_CB_TX_USER_IDX(pst_tx_ctl));
    if (pst_hmac_user == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_tx_traffic_classify::cannot find hmac_user[%d] may has freed!!",
                         MAC_GET_CB_TX_USER_IDX(pst_tx_ctl));
        return;
    }

    if (hmac_tx_traffic_is_udp_ip(pst_ip, puc_tid, pst_tx_ctl) != OAL_SUCC) {
        return;
    }

    /* 若为IP分片帧，没有端口号，直接返回 */
    if (0 != (oal_ntoh_16(pst_ip->us_frag_off) & IP_FRAGMENT_MASK)) {
        return;
    }

    pst_udp_hdr = (udp_hdr_stru *)(pst_ip + 1); /* 偏移一个IP头，取UDP头 */
    st_flow_info.us_dport = pst_udp_hdr->us_des_port;
    st_flow_info.us_sport = pst_udp_hdr->us_src_port;
    st_flow_info.dip = pst_ip->daddr;
    st_flow_info.sip = pst_ip->saddr;
    st_flow_info.proto = (uint32_t)(pst_ip->uc_protocol);

    /* 若来包属于用户已识别业务，直接返回TID */
    if (hmac_tx_traffic_identified_flow_info(pst_hmac_user, &st_flow_info, puc_tid) != OAL_SUCC) {
        return;
    }

    /* 来包尚未识别，存入用户待识别队列 */
    pst_rtp_hdr = (hmac_tx_rtp_hdr *)(pst_udp_hdr + 1); /* 偏移一个UDP头，取RTP头 */
    pst_judge_list = &(pst_hmac_user->st_judge_list);
    judge_info = (hmac_tx_judge_info_stru *)(pst_judge_list->ast_judge_cache + pst_judge_list->to_judge_num);
    judge_info->len =
        oal_net2host_short(pst_ip->us_tot_len) - sizeof(mac_ip_header_stru) - sizeof(udp_hdr_stru);
    if (hmac_tx_traffic_to_be_identified_flow_info(pst_hmac_user, pst_judge_list, pst_rtp_hdr,
                                                   &st_flow_info, judge_info) != OAL_SUCC) {
        return;
    }

    /* 在未识别门限到达前且该流未识别前先预试别 */
    hmac_tx_traffic_pre_identify(pst_hmac_user, pst_judge_list, pst_rtp_hdr, puc_tid);
    /* 若待识别队列已满，尝试提取队列主要业务并进行业务识别 */
    /* "<=":防止进程并发使此值大于待识别队列长度而踩内存 */
    if (pst_judge_list->to_judge_num >= MAX_JUDGE_CACHE_LENGTH) {
        if (hmac_tx_find_major_traffic(pst_hmac_user, puc_tid) != OAL_SUCC) {
            oam_info_log0(0, OAM_SF_TX, "hmac_tx_traffic_classify::the classify process failed.");
        }
    }
}

