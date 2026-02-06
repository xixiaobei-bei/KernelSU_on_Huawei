
#ifdef _PRE_WLAN_TCP_OPT
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/*lint -e322*/
#include <linux/jiffies.h>
/*lint +e322*/
#endif
#include "oam_ext_if.h"
#include "hmac_tcp_opt_struc.h"
#include "oal_hcc_host_if.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_tcp_opt.h"
#include "mac_data.h"
#include "oal_net.h"
#include "oal_types.h"
#include "hmac_rx_data.h"
#include "frw_event_main.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TCP_OPT_C

/* defined for ut test */
#if defined(WIN32)
uint32_t jiffies;

oal_bool_enum_uint8 time_before_eq(uint32_t a, uint32_t b)
{
    return OAL_TRUE;
}
#endif

/* 4 全局变量定义 */
/* 5 内部静态函数声明 */
/* 6 函数实现 */

void hmac_tcp_opt_ack_count_reset(hmac_vap_stru *pst_hmac_vap, hcc_chan_type dir, uint16_t stream)
{
    if (oal_warn_on(!pst_hmac_vap)) {
        oal_io_print("%s error:pst_hmac_vap is null", __FUNCTION__);
        return;
    };
    oal_spin_lock_bh(&pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.hmac_tcp_ack_lock);
    pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.aul_hcc_ack_count[stream] = 0;
    pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.aul_hcc_duplicate_ack_count[stream] = 0;
    oal_spin_unlock_bh(&pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.hmac_tcp_ack_lock);
}


void hmac_tcp_opt_ack_all_count_reset(hmac_vap_stru *pst_hmac_vap)
{
    uint16_t us_dir_index = 0;
    uint16_t us_tcp_index;

    for (us_tcp_index = 0; us_tcp_index < HMAC_TCP_STREAM; us_tcp_index++) {
        hmac_tcp_opt_ack_count_reset(pst_hmac_vap, (hcc_chan_type)us_dir_index, us_tcp_index);
    }
}


void hmac_tcp_opt_ack_show_count(hmac_vap_stru *pst_hmac_vap)
{
    uint16_t us_dir_index;
    uint16_t us_tcp_index;

    for (us_dir_index = 0; us_dir_index < HCC_DIR_COUNT; us_dir_index++) {
        for (us_tcp_index = 0; us_tcp_index < HMAC_TCP_OPT_QUEUE_BUTT; us_tcp_index++) {
            oal_spin_lock_bh(&pst_hmac_vap->st_hmac_tcp_ack[us_dir_index].hmac_tcp_ack.hmac_tcp_ack_lock);
            oal_io_print("dir = %u,tcp_index = %u,all_ack_count = %llu,drop_count = %llu\n",
                         us_dir_index,
                         us_tcp_index,
                         pst_hmac_vap->st_hmac_tcp_ack[us_dir_index].all_ack_count[us_tcp_index],
                         pst_hmac_vap->st_hmac_tcp_ack[us_dir_index].drop_count[us_tcp_index]);

            pst_hmac_vap->st_hmac_tcp_ack[us_dir_index].all_ack_count[us_tcp_index] = 0;
            pst_hmac_vap->st_hmac_tcp_ack[us_dir_index].drop_count[us_tcp_index] = 0;
            oal_spin_unlock_bh(&pst_hmac_vap->st_hmac_tcp_ack[us_dir_index].hmac_tcp_ack.hmac_tcp_ack_lock);
        }
    }
}

struct tcp_list_node *hmac_tcp_opt_find_oldest_node(hmac_vap_stru *pst_hmac_vap, hcc_chan_type dir)
{
    struct tcp_list_node *node = NULL;
    struct tcp_list_node *oldest_node = NULL;
    unsigned long oldest_time = jiffies; /* init current time */
    struct wlan_perform_tcp_list *tmp_list;
    oal_dlist_head_stru *pst_entry = NULL;

    tmp_list = &pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack_list;
    oal_dlist_search_for_each(pst_entry, &tmp_list->tcp_list)
    {
        node = oal_dlist_get_entry(pst_entry, struct tcp_list_node, list);
        if (OAL_TRUE == time_before_eq(node->last_ts, oldest_time)) {
            oldest_time = node->last_ts;
            oldest_node = node;
            oam_info_log3(0, OAM_SF_ANY,
                          "{dir:%d find last_ts %ld   oldest_time %ld.}",
                          dir, node->last_ts, oldest_time);
        }
    }
    if (oldest_node != NULL) {
        oal_dlist_delete_entry(&oldest_node->list);
        oal_dlist_init_head(&oldest_node->list);
    } else {
        oam_error_log0(0, OAM_SF_ANY, "can't find oldest node xx");
    }
    return oldest_node;
}

struct tcp_list_node *hmac_tcp_opt_get_buf(hmac_vap_stru *pst_hmac_vap, hcc_chan_type dir)
{
    struct tcp_list_node *node = NULL;
    uint16_t us_tcp_stream_index;
    struct wlan_perform_tcp_list *tmp_ack_list = &pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack_list;

    if (tmp_ack_list->free_count == 0) {
        node = hmac_tcp_opt_find_oldest_node(pst_hmac_vap, dir);
        return node;
    }

    for (us_tcp_stream_index = 0; us_tcp_stream_index < HMAC_TCP_STREAM; us_tcp_stream_index++) {
        if (tmp_ack_list->tcp_pool[us_tcp_stream_index].used == 0) {
            tmp_ack_list->tcp_pool[us_tcp_stream_index].used = 1;
            tmp_ack_list->free_count--;
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            oal_io_print("\r\n====dir:%d get buf %d free:%d====\r\n}",
                         dir, us_tcp_stream_index, tmp_ack_list->free_count);
#endif
            node = &tmp_ack_list->tcp_pool[us_tcp_stream_index];
            break;
        }
    }
    return node;
}


uint32_t hmac_tcp_opt_add_node(hmac_vap_stru *pst_hmac_vap, struct wlan_tcp_flow *tcp_info, hcc_chan_type dir)
{
    struct tcp_list_node *node = NULL;

    if ((pst_hmac_vap == NULL) || (tcp_info == NULL)) {
        return OAL_FAIL;
    }

    node = hmac_tcp_opt_get_buf(pst_hmac_vap, dir);
    if (node == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "Invalid NULL node!");
        return OAL_FAIL;
    }

    node->wlan_tcp_info.dst_ip = tcp_info->dst_ip;
    node->wlan_tcp_info.src_ip = tcp_info->src_ip;
    node->wlan_tcp_info.us_src_port = tcp_info->us_src_port;
    node->wlan_tcp_info.us_dst_port = tcp_info->us_dst_port;
    node->wlan_tcp_info.uc_protocol = tcp_info->uc_protocol;
    node->last_ts = jiffies;

    oal_dlist_add_tail(&node->list, &pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack_list.tcp_list);

#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    oal_io_print("\r\n====dir:%d,ul_index = %d,add node succ====\r\n", dir, node->index);
#endif

    return node->index;
}


uint32_t hmac_tcp_opt_init_filter_tcp_ack_pool(hmac_vap_stru *pst_hmac_vap)
{
    uint16_t us_dir_index;
    uint16_t us_tcp_index;
    uint16_t us_tcp_queue_index;
    hmac_tcp_ack_stru *hmac_tcp_ack = NULL;
    if (oal_warn_on(pst_hmac_vap == NULL)) {
        oam_info_log0(0, OAM_SF_ANY, "{hmac_tcp_opt_init_filter_tcp_ack_poolr fail:pst_hmac_vap is null}");
        return OAL_FAIL;
    }

    /* init downline tcp ack pool */
    /* init tx_worker_state */
    hmac_tcp_ack = &pst_hmac_vap->st_hmac_tcp_ack[0];
    for (us_dir_index = 0; us_dir_index < HCC_DIR_COUNT; us_dir_index++) {
        for (us_tcp_index = 0; us_tcp_index < HMAC_TCP_STREAM; us_tcp_index++) {
            oal_spin_lock_init(&pst_hmac_vap->st_hmac_tcp_ack[us_dir_index].hmac_tcp_ack.hmac_tcp_ack_lock);
            oal_netbuf_list_head_init(&hmac_tcp_ack[us_dir_index].hmac_tcp_ack.hcc_ack_queue[us_tcp_index]);
            hmac_tcp_ack[us_dir_index].hmac_tcp_ack_list.tcp_pool[us_tcp_index].used = 0;
            hmac_tcp_ack[us_dir_index].hmac_tcp_ack_list.tcp_pool[us_tcp_index].index = us_tcp_index;
            hmac_tcp_ack[us_dir_index].hmac_tcp_ack_list.tcp_pool[us_tcp_index].last_ts = jiffies;
        }
        for (us_tcp_queue_index = 0; us_tcp_queue_index < HMAC_TCP_OPT_QUEUE_BUTT; us_tcp_queue_index++) {
            oal_spin_lock_init(&pst_hmac_vap->st_hmac_tcp_ack[us_dir_index].data_queue_lock[us_tcp_queue_index]);
            oal_netbuf_head_init(&pst_hmac_vap->st_hmac_tcp_ack[us_dir_index].data_queue[us_tcp_queue_index]);
        }
        oal_dlist_init_head(&(pst_hmac_vap->st_hmac_tcp_ack[us_dir_index].hmac_tcp_ack_list.tcp_list));
        pst_hmac_vap->st_hmac_tcp_ack[us_dir_index].hmac_tcp_ack_list.free_count = HMAC_TCP_STREAM;
        oam_info_log1(0, OAM_SF_ANY, "{wifi tcp perform dir:%d init done.}", us_dir_index);
    }
    pst_hmac_vap->st_hmac_tcp_ack[HCC_TX].filter[HMAC_TCP_ACK_QUEUE] = hmac_tcp_opt_tx_tcp_ack_filter;
    pst_hmac_vap->st_hmac_tcp_ack[HCC_RX].filter[HMAC_TCP_ACK_QUEUE] = hmac_tcp_opt_rx_tcp_ack_filter;
    return OAL_SUCC;
}


void hmac_tcp_opt_free_ack_list(hmac_vap_stru *pst_hmac_vap, uint8_t dir, uint8_t type)
{
#if !defined(WIN32)
    oal_netbuf_head_stru st_head_t;
    oal_netbuf_head_stru *head = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;
    oal_netbuf_head_stru *hcc_ack_queue = NULL;
    struct wlan_perform_tcp_list *tmp_list = NULL;
    oal_dlist_head_stru *pst_entry = NULL;
    oal_dlist_head_stru *pst_entry_temp = NULL;
    struct tcp_list_node *node = NULL;

    oal_spin_lock_bh(&pst_hmac_vap->st_hmac_tcp_ack[dir].data_queue_lock[type]);

    oal_netbuf_head_init(&st_head_t);
    head = &pst_hmac_vap->st_hmac_tcp_ack[dir].data_queue[type];
    oal_netbuf_queue_splice_tail_init(head, &st_head_t);
    while (!!(pst_netbuf = oal_netbuf_delist(&st_head_t))) {
        oal_netbuf_free(pst_netbuf);
    }

    tmp_list = &pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack_list;
    oal_dlist_search_for_each_safe(pst_entry, pst_entry_temp, &tmp_list->tcp_list)
    {
        node = oal_dlist_get_entry(pst_entry, struct tcp_list_node, list);

        if (node->used == 0) {
            continue;
        }
        hcc_ack_queue = &pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.hcc_ack_queue[node->index];
        while (!!(pst_netbuf = oal_netbuf_delist(hcc_ack_queue))) {
            oal_netbuf_free(pst_netbuf);
        }
    }

    oal_spin_unlock_bh(&pst_hmac_vap->st_hmac_tcp_ack[dir].data_queue_lock[type]);
#endif
}

void hmac_tcp_opt_deinit_list(hmac_vap_stru *pst_hmac_vap)
{
    hmac_tcp_opt_free_ack_list(pst_hmac_vap, HCC_TX, HMAC_TCP_ACK_QUEUE);
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    hmac_tcp_opt_free_ack_list(pst_hmac_vap, HCC_RX, HMAC_TCP_ACK_QUEUE);
#endif
}


uint32_t hmac_tcp_opt_get_flow_index(hmac_vap_stru *pst_hmac_vap,
    oal_ip_header_stru *pst_ip_hdr,
    oal_tcp_header_stru *pst_tcp_hdr,
    hcc_chan_type dir)
{
    struct wlan_tcp_flow tcp_flow_info;
    struct tcp_list_node *node = NULL;
    oal_dlist_head_stru *pst_entry = NULL;
    struct wlan_perform_tcp_list *tmp_list = NULL;

    tcp_flow_info.src_ip = pst_ip_hdr->saddr;
    tcp_flow_info.dst_ip = pst_ip_hdr->daddr;
    tcp_flow_info.us_src_port = pst_tcp_hdr->us_sport;
    tcp_flow_info.us_dst_port = pst_tcp_hdr->us_dport;
    tcp_flow_info.uc_protocol = pst_ip_hdr->uc_protocol;

#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    oal_io_print("\r\n====hmac_tcp_opt_get_flow_index enter====\r\n");
#endif
    /* get the queue index of tcp ack */
    tmp_list = &pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack_list;
    oal_dlist_search_for_each(pst_entry, &tmp_list->tcp_list)
    {
        node = (struct tcp_list_node *)pst_entry;
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oal_io_print("\r\n====dir:%d node [index:%d],ul_used = %d====\r\n",
                     dir, node->index, node->used);
#endif
        if ((node->wlan_tcp_info.src_ip == tcp_flow_info.src_ip) &&
            (node->wlan_tcp_info.dst_ip == tcp_flow_info.dst_ip) &&
            (node->wlan_tcp_info.us_src_port == tcp_flow_info.us_src_port) &&
            (node->wlan_tcp_info.us_dst_port == tcp_flow_info.us_dst_port) &&
            (node->wlan_tcp_info.uc_protocol == tcp_flow_info.uc_protocol) &&
            (node->used == 1)) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            oal_io_print("\r\n====dir:%d find the same tcp flow info [index:%d]====\r\n",
                         dir, node->index);
#endif
            node->last_ts = jiffies; /* renew the last pkt time */
            return node->index;
        }
    }

    /* new follow, save in new node */
    return hmac_tcp_opt_add_node(pst_hmac_vap, &tcp_flow_info, dir);
}


oal_tcp_ack_type_enum hmac_tcp_opt_get_tcp_ack_type(hmac_vap_stru *pst_hmac_vap,
    oal_ip_header_stru *pst_ip_hdr,
    hcc_chan_type dir,
    uint16_t us_index)
{
    oal_tcp_header_stru *pst_tcp_hdr = NULL;
    uint32_t tcp_ack_no;
    uint32_t *tmp_tcp_ack_no = NULL;

#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    oal_io_print("\r\n====hmac_tcp_opt_get_tcp_ack_type:us_index = %d ====\r\n", us_index);
#endif
    /*lint -e522*/
    if (oal_warn_on(us_index >= HMAC_TCP_STREAM)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_tcp_opt_get_tcp_ack_type::us_index[%d] error!}", us_index);
        return TCP_TYPE_ERROR;
    }
    /*lint +e522*/
    pst_tcp_hdr = (oal_tcp_header_stru *)(pst_ip_hdr + 1);
    tcp_ack_no = pst_tcp_hdr->acknum;

    /* 检测duplicat ack是否存在，如果存在则累计ack流最大成员数 */
    tmp_tcp_ack_no = pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.aul_hcc_tcp_ack_no;
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    oal_io_print("\r\n====stream:%d ack no:%u  tcp ack no:%u ====\r\n", us_index, tcp_ack_no, tmp_tcp_ack_no[us_index]);
#endif
    if (tcp_ack_no == tmp_tcp_ack_no[us_index]) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oam_warning_log1(0, OAM_SF_ANY, "{dir:%d:**duplicate ack is coming**.}", dir);
#endif
        pst_hmac_vap->st_hmac_tcp_ack[dir].filter_info.st_tcp_info[us_index].ull_dup_ack_count++;
        return TCP_ACK_DUP_TYPE;
    }

    tmp_tcp_ack_no[us_index] = pst_tcp_hdr->acknum;

    /* 赋值tcp_cb */
    return TCP_ACK_FILTER_TYPE;
}


oal_bool_enum_uint8 hmac_judge_rx_netbuf_is_tcp_ack(mac_llc_snap_stru *pst_snap, uint32_t buf_len)
{
    oal_ip_header_stru *pst_ip_hdr = NULL;
    oal_bool_enum_uint8 en_is_tcp_ack = OAL_FALSE;
    oal_tcp_header_stru *pst_tcp_hdr = NULL;

    if (pst_snap == NULL || buf_len == 0) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_judge_rx_netbuf_is_tcp_ack:  pst_snap is null!}");
        return OAL_FALSE;
    }
    switch (pst_snap->us_ether_type) {
        /*lint -e778*/ /* 屏蔽Info-- Constant expression evaluates to 0 in operation '&' */
        case oal_host2net_short(ETHER_TYPE_IP):
            if (buf_len < sizeof(oal_ip_header_stru)) {
                oam_error_log1(0, OAM_SF_ANY, "{hmac_judge_rx_netbuf_is_tcp_ack:buf_len[%d].}", buf_len);
                return OAL_FALSE;
            }
            pst_ip_hdr = (oal_ip_header_stru *)(pst_snap + 1); /* 偏移一个snap，取ip头 */
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            oam_warning_log1(0, OAM_SF_RX, "{oal_judge_rx_netbuf_is_tcp_ack:  pst_ip_hdr->uc_protocol = %d**!}",
                             pst_ip_hdr->uc_protocol);
#endif
            if (buf_len < sizeof(oal_ip_header_stru) + sizeof(oal_tcp_header_stru)) {
                oam_error_log1(0, OAM_SF_ANY, "{hmac_judge_rx_netbuf_is_tcp_ack:buf_len[%d].}", buf_len);
                return OAL_FALSE;
            }
            if ((pst_ip_hdr->uc_protocol == MAC_TCP_PROTOCAL) && (oal_netbuf_is_tcp_ack(pst_ip_hdr) == OAL_TRUE)) {
                pst_tcp_hdr = (oal_tcp_header_stru *)(pst_ip_hdr + 1);
                en_is_tcp_ack = OAL_TRUE;
                /* option3:SYN FIN RST URG有为1的时候不过滤 */
                if ((pst_tcp_hdr->uc_flags) & FILTER_FLAG_MASK) {
                    en_is_tcp_ack = OAL_FALSE;
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
                    oam_warning_log0(0, OAM_SF_RX,
                        "{hmac_judge_rx_netbuf_is_tcp_ack:  **specific tcp pkt, can't be filter**!}");
#endif
                }
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
                oam_warning_log0(0, OAM_SF_RX, "{oal_judge_rx_netbuf_is_tcp_ack:: tcp ack frame!}");
#endif
            }
            break;
        /*lint +e778*/
        default:
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            oal_io_print("{oal_judge_rx_netbuf_is_tcp_ack::unkown us_ether_type[%d]}\r\n", pst_snap->us_ether_type);
#endif
            break;
    }

    return en_is_tcp_ack;
}


oal_bool_enum_uint8 hmac_judge_rx_netbuf_classify(mac_llc_snap_stru *pst_snap, uint32_t buf_len)
{
    if (pst_snap == NULL) {
        return OAL_FALSE;
    }
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    oam_warning_log1(0, OAM_SF_ANY, "**hmac_judge_rx_netbuf_classify, us_ether_type = %d**", pst_snap->us_ether_type);
#endif
    return hmac_judge_rx_netbuf_is_tcp_ack(pst_snap, buf_len);
}


oal_bool_enum_uint8 hmac_judge_tx_netbuf_is_tcp_ack(oal_ether_header_stru *ps_ethmac_hdr)
{
    oal_ip_header_stru *pst_ip = NULL;
    oal_tcp_header_stru *pst_tcp_hdr = NULL;
    oal_bool_enum_uint8 en_is_tcp_ack = OAL_FALSE;

    if (ps_ethmac_hdr == NULL) {
        return OAL_FALSE;
    }
    switch (ps_ethmac_hdr->us_ether_type) {
        /*lint -e778*/ /* 屏蔽Info-- Constant expression evaluates to 0 in operation '&' */
        case oal_host2net_short(ETHER_TYPE_IP):
            pst_ip = (oal_ip_header_stru *)(ps_ethmac_hdr + 1); /* 偏移一个snap，取ip头 */
            if ((pst_ip->uc_protocol == MAC_TCP_PROTOCAL) && (oal_netbuf_is_tcp_ack(pst_ip) == OAL_TRUE)) {
                pst_tcp_hdr = (oal_tcp_header_stru *)(pst_ip + 1);
                en_is_tcp_ack = OAL_TRUE;
                /* option3:SYN FIN RST URG有为1的时候不过滤 */
                if ((pst_tcp_hdr->uc_flags) & FILTER_FLAG_MASK) {
                    en_is_tcp_ack = OAL_FALSE;
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
                    oam_warning_log0(0, OAM_SF_ANY, "**specific tcp pkt, can't be filter**");
#endif
                }
            }
            break;

        /*lint +e778*/
        default:
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            oal_io_print("{oal_netbuf_select_queue::unkown us_ether_type[%d]}\r\n", ps_ethmac_hdr->us_ether_type);
#endif
            break;
    }

    return en_is_tcp_ack;
}


oal_tcp_ack_type_enum hmac_tcp_opt_rx_get_tcp_ack(oal_netbuf_stru *skb, hmac_vap_stru *pst_hmac_vap,
    uint16_t *p_us_index, uint8_t dir)
{
    oal_ip_header_stru *pst_ip_hdr = NULL;
    oal_tcp_header_stru *pst_tcp_hdr = NULL;
    mac_llc_snap_stru *pst_snap = NULL;
    mac_rx_ctl_stru *pst_rx_ctrl; /* 指向MPDU控制块信息的指针 */
    uint32_t buf_len = oal_netbuf_len(skb);
    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(skb);
    if (buf_len < pst_rx_ctrl->uc_mac_header_len + sizeof(mac_llc_snap_stru)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_tcp_opt_rx_get_tcp_ack:buf_len[%d].}", buf_len);
        return TCP_TYPE_ERROR;
    }
    buf_len -= pst_rx_ctrl->uc_mac_header_len + sizeof(mac_llc_snap_stru);
    pst_snap = (mac_llc_snap_stru *)(skb->data + pst_rx_ctrl->uc_mac_header_len);

    if (OAL_FALSE == hmac_judge_rx_netbuf_is_tcp_ack(pst_snap, buf_len)) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        /* not tcp ack data */
        oam_info_log0(0, OAM_SF_ANY, "{**not tcp packet return direct**}\r\n");
#endif
        return TCP_TYPE_ERROR;
    }
    pst_ip_hdr = (oal_ip_header_stru *)(pst_snap + 1); /* 偏移一个snap，取ip头 */
    pst_tcp_hdr = (oal_tcp_header_stru *)(pst_ip_hdr + 1);

    /* option4:flow index取不到时不过滤 */
    *p_us_index =
        (uint16_t)hmac_tcp_opt_get_flow_index(pst_hmac_vap, pst_ip_hdr, pst_tcp_hdr, (hcc_chan_type)dir);
    if (*p_us_index == 0xFFFF) {
        return TCP_TYPE_ERROR;
    }

    return hmac_tcp_opt_get_tcp_ack_type(pst_hmac_vap, pst_ip_hdr, (hcc_chan_type)dir, *p_us_index);
}


oal_tcp_ack_type_enum hmac_tcp_opt_tx_get_tcp_ack(oal_netbuf_stru *skb,
    hmac_vap_stru *pst_hmac_vap,
    uint16_t *p_us_index,
    uint8_t dir)
{
    oal_ip_header_stru *pst_ip_hdr = NULL;
    oal_tcp_header_stru *pst_tcp_hdr = NULL;
    oal_ether_header_stru *eth_hdr;

    eth_hdr = (oal_ether_header_stru *)oal_netbuf_data(skb);
    if (OAL_FALSE == hmac_judge_tx_netbuf_is_tcp_ack(eth_hdr)) {
        /* not tcp ack data */
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oal_io_print("\r\n====**not tcp packet return direct**====\r\n");
#endif
        return TCP_TYPE_ERROR;
    }
    pst_ip_hdr = (oal_ip_header_stru *)(eth_hdr + 1); /* 偏移一个snap，取ip头 */
    pst_tcp_hdr = (oal_tcp_header_stru *)(pst_ip_hdr + 1);
    /* option4:flow index取不到时不过滤 */
    *p_us_index =
        (uint16_t)hmac_tcp_opt_get_flow_index(pst_hmac_vap, pst_ip_hdr, pst_tcp_hdr, (hcc_chan_type)dir);
    if (*p_us_index == 0xFFFF) {
        return TCP_TYPE_ERROR;
    }

    return hmac_tcp_opt_get_tcp_ack_type(pst_hmac_vap, pst_ip_hdr, (hcc_chan_type)dir, *p_us_index);
}


uint16_t hmac_tcp_opt_tcp_ack_list_filter(hmac_vap_stru *pst_hmac_vap,
    hmac_tcp_opt_queue type,
    hcc_chan_type dir,
    oal_netbuf_head_stru *head)
{
    struct tcp_list_node *node = NULL;
    oal_netbuf_stru *skb = NULL;
    oal_netbuf_head_stru head_t;
    struct wlan_perform_tcp_list *tmp_list = NULL;
    oal_dlist_head_stru *pst_entry = NULL;
    oal_dlist_head_stru *pst_entry_temp = NULL;

    if (!oal_netbuf_list_len(head)) {
        return 0;
    }
    oal_netbuf_head_init(&head_t);

#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    oal_io_print("\r\n====hmac_tcp_opt_tcp_ack_list_filter:uc_vap_id = %d,dir=%d filter queue qlen %u====\r\n",
                 pst_hmac_vap->st_vap_base_info.uc_vap_id, dir, oal_netbuf_list_len(head));
#endif
    while (!!(skb = oal_netbuf_delist(head))) {
        if (hmac_tcp_opt_tcp_ack_filter(skb, pst_hmac_vap, dir) != OAL_SUCC) {
            oal_netbuf_list_tail(&head_t, skb);
        }
    }

    /*lint -e522*/
    oal_warn_on(!oal_netbuf_list_empty(head));
    /*lint +e522*/
    oal_netbuf_splice_init(&head_t, head);
    pst_hmac_vap->st_hmac_tcp_ack[dir].filter_info.ull_ignored_count += oal_netbuf_list_len(head);

    tmp_list = &pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack_list;
    oal_dlist_search_for_each_safe(pst_entry, pst_entry_temp, &tmp_list->tcp_list) {
        oal_netbuf_head_stru *hcc_ack_queue = NULL;

        node = oal_dlist_get_entry(pst_entry, struct tcp_list_node, list);
        oam_info_log1(0, OAM_SF_ANY, "{dir:%d --begin to recv packet--.}", dir);
        if (node->used == 0) {
            continue;
        }
        hcc_ack_queue = &pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.hcc_ack_queue[node->index];

#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oal_io_print("\r\n====dir:%d ----recv packet----[qlen:%u].====\r\n", dir, oal_netbuf_list_len(hcc_ack_queue));
#endif
        hmac_tcp_opt_ack_count_reset(pst_hmac_vap, (hcc_chan_type)dir, (uint16_t)node->index);
        pst_hmac_vap->st_hmac_tcp_ack[dir].filter_info.st_tcp_info[node->index].ull_send_count +=
            oal_netbuf_list_len(hcc_ack_queue);

        oal_netbuf_queue_splice_tail_init(hcc_ack_queue, head);
    }
    return 0;
}


uint16_t hmac_tcp_opt_rx_tcp_ack_filter(hmac_vap_stru *hmac_vap, hmac_tcp_opt_queue type, hcc_chan_type dir,
    oal_netbuf_head_stru *head)
{
    hmac_device_stru *pst_hmac_device = NULL;
    if (oal_warn_on(!hmac_vap)) {
        oam_info_log0(0, OAM_SF_ANY, "{hmac_tcp_opt_rx_tcp_ack_filter fail:hmac_vap is null}");
        return OAL_FAIL;
    }

    if (oal_warn_on(!head)) {
        oam_info_log0(0, OAM_SF_ANY, "{hmac_tcp_opt_rx_tcp_ack_filter fail:data is null}");
        return OAL_FAIL;
    }

    if (oal_warn_on(type != HMAC_TCP_ACK_QUEUE)) {
        oam_info_log2(0, OAM_SF_ANY, "{hmac_tcp_opt_rx_tcp_ack_filter fail:type:%d not equal %d}",
                      type, HMAC_TCP_ACK_QUEUE);
        return OAL_FAIL;
    }

    if (oal_warn_on(dir != HCC_RX)) {
        oam_info_log2(0, OAM_SF_ANY, "{hmac_tcp_opt_rx_tcp_ack_filter fail:dir:%d not equal %d}", dir, HCC_RX);
        return OAL_FAIL;
    }

    pst_hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                       "{hmac_tcp_opt_rx_tcp_ack_filter fail: pst_hmac_device is null}\r\n");
        return OAL_FAIL;
    }

    if (!pst_hmac_device->sys_tcp_rx_ack_opt_enable) {
        return 0;
    }
    return hmac_tcp_opt_tcp_ack_list_filter(hmac_vap, type, dir, head);
}


uint16_t hmac_tcp_opt_tx_tcp_ack_filter(hmac_vap_stru *hmac_vap, hmac_tcp_opt_queue type, hcc_chan_type dir,
    oal_netbuf_head_stru *data)
{
    if (oal_warn_on(!hmac_vap)) {
        oam_info_log0(0, OAM_SF_ANY, "{hmac_tcp_opt_rx_tcp_ack_filter fail:hmac_vap is null}");
        return OAL_FAIL;
    }

    if (oal_warn_on(!data)) {
        oam_info_log0(0, OAM_SF_ANY, "{hmac_tcp_opt_rx_tcp_ack_filter fail:data is null}");
        return OAL_FAIL;
    }

    if (oal_warn_on(type != HMAC_TCP_ACK_QUEUE)) {
        oam_info_log2(0, OAM_SF_ANY, "{hmac_tcp_opt_rx_tcp_ack_filter fail:type:%d not equal %d}",
                      type, HMAC_TCP_ACK_QUEUE);
        return OAL_FAIL;
    }

    if (oal_warn_on(dir != HCC_TX)) {
        oam_info_log2(0, OAM_SF_ANY, "{hmac_tcp_opt_rx_tcp_ack_filter fail:dir:%d not equal %d}", dir, HCC_TX);
        return OAL_FAIL;
    }

    return hmac_tcp_opt_tcp_ack_list_filter(hmac_vap, type, dir, data);
}


OAL_STATIC void hmac_tcp_opt_dup_ack_process(oal_netbuf_stru *skb, hmac_vap_stru *pst_hmac_vap, hcc_chan_type dir,
    uint16_t us_tcp_stream_index)
{
    oal_netbuf_head_stru *hcc_ack_queue = NULL;
    oal_netbuf_stru *ack = NULL;
    uint32_t ret;

    oal_spin_lock_bh(&pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.hmac_tcp_ack_lock);
    hcc_ack_queue = &pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.hcc_ack_queue[us_tcp_stream_index];
    /* 将dup ack帧流队列中的帧全部发送 */
    while (!!(ack = oal_netbuf_delist(hcc_ack_queue))) {
        ret = hmac_tx_lan_to_wlan_no_tcp_opt(&(pst_hmac_vap->st_vap_base_info), ack);
        /* 调用失败，要释放内核申请的netbuff内存池 */
        if (oal_unlikely(ret != OAL_SUCC)) {
            oal_netbuf_free(ack);
        } else {
            pst_hmac_vap->st_hmac_tcp_ack[dir].filter_info.st_tcp_info[us_tcp_stream_index].ull_send_count++;
        }

        pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.aul_hcc_ack_count[us_tcp_stream_index]--;
    }
    /* 当前dup ack帧发送 */
    ret = hmac_tx_lan_to_wlan_no_tcp_opt(&(pst_hmac_vap->st_vap_base_info), skb);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_netbuf_free(skb);
    } else {
        pst_hmac_vap->st_hmac_tcp_ack[dir].filter_info.st_tcp_info[us_tcp_stream_index].ull_send_count++;
    }
    oal_spin_unlock_bh(&pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.hmac_tcp_ack_lock);
}


OAL_STATIC void hmac_tcp_opt_normal_ack_process(oal_netbuf_stru *skb, hmac_vap_stru *pst_hmac_vap,
    hcc_chan_type dir, uint16_t us_tcp_stream_index)
{
    uint32_t ack_limit;
    oal_netbuf_head_stru *hcc_ack_queue = NULL;
    oal_netbuf_stru *ack = NULL;

    oal_spin_lock_bh(&pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.hmac_tcp_ack_lock);
    ack_limit = pst_hmac_vap->st_hmac_tcp_ack[dir].filter_info.ack_limit;
    hcc_ack_queue = &pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.hcc_ack_queue[us_tcp_stream_index];

    /* if normal ack received, del until ack_limit ack left */
    while (pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.aul_hcc_ack_count[us_tcp_stream_index] >= ack_limit) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oam_warning_log4(0, OAM_SF_ANY, "dir:%d:stream:%d : ack count:%u, qlen:%u", dir, us_tcp_stream_index,
                         pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.aul_hcc_ack_count[us_tcp_stream_index],
                         oal_netbuf_list_len(hcc_ack_queue));
#endif
        ack = oal_netbuf_delist(hcc_ack_queue);
        if (ack == NULL) {
            break;
        }
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oam_warning_log1(0, OAM_SF_ANY, "{dir:%d ------drop packet------.}", dir);
#endif
        pst_hmac_vap->st_hmac_tcp_ack[dir].filter_info.st_tcp_info[us_tcp_stream_index].ull_drop_count++;
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oam_warning_log4(0, OAM_SF_ANY, "{dir:%d: ack count:%d , dupcount:%d ull_drop_count:%d.}", dir,
            pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.aul_hcc_ack_count[us_tcp_stream_index],
            pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.aul_hcc_duplicate_ack_count[us_tcp_stream_index],
            pst_hmac_vap->st_hmac_tcp_ack[dir].filter_info.st_tcp_info[us_tcp_stream_index].ull_drop_count);
#endif
        oal_netbuf_free_any(ack);
        pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.aul_hcc_ack_count[us_tcp_stream_index]--;
    }
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    oam_warning_log3(0, OAM_SF_ANY, "{dir:%d: stream:%d qlen:%u.}", dir,
                     us_tcp_stream_index, oal_netbuf_list_len(hcc_ack_queue));
#endif
    oal_netbuf_add_to_list_tail(skb, hcc_ack_queue);
    pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.aul_hcc_ack_count[us_tcp_stream_index]++;
    oal_spin_unlock_bh(&pst_hmac_vap->st_hmac_tcp_ack[dir].hmac_tcp_ack.hmac_tcp_ack_lock);
}


uint32_t hmac_tcp_opt_tcp_ack_filter(oal_netbuf_stru *skb, hmac_vap_stru *pst_hmac_vap, hcc_chan_type dir)
{
    oal_tcp_ack_type_enum uc_ret;
    uint16_t us_tcp_stream_index;

    if (dir == HCC_TX) {
        uc_ret = hmac_tcp_opt_tx_get_tcp_ack(skb, pst_hmac_vap, &us_tcp_stream_index, dir);
    } else {
        uc_ret = hmac_tcp_opt_rx_get_tcp_ack(skb, pst_hmac_vap, &us_tcp_stream_index, dir);
    }

    if (uc_ret == TCP_TYPE_ERROR) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oal_io_print("\r\n====not found tcp ack...====\r\n");
#endif
        /* TCP_TYPE_ERROR 异常逻辑，将帧放入vap queue发送 */
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oal_io_print("\r\n====found tcp ack...====\r\n");
#endif
    if (uc_ret == TCP_ACK_DUP_TYPE) {
        hmac_tcp_opt_dup_ack_process(skb, pst_hmac_vap, dir, us_tcp_stream_index);
    } else if (uc_ret == TCP_ACK_FILTER_TYPE) {
        hmac_tcp_opt_normal_ack_process(skb, pst_hmac_vap, dir, us_tcp_stream_index);
    }
    return OAL_SUCC;
}
void hmac_trans_queue_filter(hmac_vap_stru *pst_hmac_vap, oal_netbuf_head_stru *head_t,
    hmac_tcp_opt_queue type, hcc_chan_type dir)
{
    uint32_t old_len, new_len;

    if (pst_hmac_vap->st_hmac_tcp_ack[dir].filter[type]) {
        old_len = oal_netbuf_list_len(head_t);
        pst_hmac_vap->st_hmac_tcp_ack[dir].filter[type](pst_hmac_vap, type, dir, head_t);
        new_len = oal_netbuf_list_len(head_t);
        pst_hmac_vap->st_hmac_tcp_ack[dir].all_ack_count[type] += old_len;
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oal_io_print("\r\n====hmac_trans_queue_filter[Queue:%d]Before filter len:%u,After filter len:%u====\r\n",
                     type, old_len, new_len);
#endif
        if (oal_unlikely(new_len > old_len)) {
            oam_warning_log2(0, OAM_SF_TX, "The filter len:%u is more than before filter:%u",
                             new_len, old_len);
        } else {
            pst_hmac_vap->st_hmac_tcp_ack[dir].drop_count[type] += (old_len - new_len);
        }
    }
}
void hmac_tcp_ack_process_hcc_queue(hmac_vap_stru *pst_hmac_vap,
    hcc_chan_type dir, hmac_tcp_opt_queue type)
{
    oal_netbuf_head_stru st_head_t;
    oal_netbuf_head_stru *head = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;
    uint32_t ret;

    oal_spin_lock_bh(&pst_hmac_vap->st_hmac_tcp_ack[dir].data_queue_lock[type]);
    if (pst_hmac_vap->st_hmac_tcp_ack[dir].filter[type] != NULL) {
        oal_netbuf_head_init(&st_head_t);
        head = &pst_hmac_vap->st_hmac_tcp_ack[dir].data_queue[type];
        oal_netbuf_queue_splice_tail_init(head, &st_head_t);
        hmac_trans_queue_filter(pst_hmac_vap, &st_head_t, type, dir);
        oal_netbuf_splice_init(&st_head_t, head);
    }
    if (dir == HCC_RX) {
        oal_netbuf_head_init(&st_head_t);
        head = &pst_hmac_vap->st_hmac_tcp_ack[dir].data_queue[type];
        oal_netbuf_queue_splice_tail_init(head, &st_head_t);
        if (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
            hmac_rx_process_data_ap_tcp_ack_opt(pst_hmac_vap, &st_head_t);
        } else if (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
            hmac_rx_process_data_sta_tcp_ack_opt(pst_hmac_vap, &st_head_t);
        }
    } else {
        oal_netbuf_head_init(&st_head_t);
        head = &pst_hmac_vap->st_hmac_tcp_ack[dir].data_queue[type];
        oal_netbuf_queue_splice_tail_init(head, &st_head_t);
        while (!!(pst_netbuf = oal_netbuf_delist(&st_head_t))) {
            ret = hmac_tx_lan_to_wlan_no_tcp_opt(&(pst_hmac_vap->st_vap_base_info), pst_netbuf);
            /* 调用失败，要释放内核申请的netbuff内存池 */
            if (oal_unlikely(ret != OAL_SUCC)) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
                oal_io_print("\r\n====hmac_tcp_ack_process_hcc_queue send fail:uc_vap_id = %d====\r\n",
                             pst_hmac_vap->st_vap_base_info.uc_vap_id);
#endif
                oal_netbuf_free(pst_netbuf);
            }
        }
    }
    oal_spin_unlock_bh(&pst_hmac_vap->st_hmac_tcp_ack[dir].data_queue_lock[type]);
}
// 此函数最好带入参
int32_t hmac_tcp_ack_process(void)
{
    uint8_t uc_vap_idx;
    uint8_t uc_device_max;
    uint8_t uc_device;
    mac_chip_stru *pst_mac_chip = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    hmac_device_stru *pst_hmac_device = NULL;

    /*lint -save -e522 */
    if (!oal_in_interrupt()) {
        frw_event_task_lock();
    }
    /*lint -restore */
    pst_mac_chip = hmac_res_get_mac_chip(0);

    /* OAL接口获取支持device个数 */
    uc_device_max = oal_chip_get_device_num(pst_mac_chip->chip_ver);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++) {
        pst_hmac_device =
            hmac_res_get_mac_dev(pst_mac_chip->auc_device_id[uc_device]);  // 多device循环chip下的所有device
        if (pst_hmac_device == NULL) {
            oam_warning_log1(0, OAM_SF_TX, "{hmac_tcp_ack_process::pst_hmac_device[%d] null.}", uc_device);
            continue;
        }
        for (uc_vap_idx = 0; uc_vap_idx < pst_hmac_device->pst_device_base_info->uc_vap_num; uc_vap_idx++) {
            pst_hmac_vap =
                (hmac_vap_stru *)mac_res_get_hmac_vap(pst_hmac_device->pst_device_base_info->auc_vap_id[uc_vap_idx]);
            if (pst_hmac_vap == NULL) {
                oam_error_log0(uc_vap_idx, OAM_SF_ANY, "{hmac_config_add_vap::pst_hmac_vap null.}");
                continue;
            }
            if ((pst_hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_UP) &&
                (pst_hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_PAUSE)) {
                continue;
            }
            hmac_tcp_ack_process_hcc_queue(pst_hmac_vap, HCC_TX, HMAC_TCP_ACK_QUEUE);
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            hmac_tcp_ack_process_hcc_queue(pst_hmac_vap, HCC_RX, HMAC_TCP_ACK_QUEUE);
#endif
        }
    }

    if (!oal_in_interrupt()) {
        frw_event_task_unlock();
    }
    return 0;
}
oal_bool_enum_uint8 hmac_tcp_ack_need_schedule(void)
{
    uint8_t uc_vap_idx;
    uint8_t uc_device_max;
    uint8_t uc_device;
    hmac_vap_stru *pst_hmac_vap = NULL;
    oal_netbuf_head_stru *head = NULL;
    mac_chip_stru *pst_mac_chip;
    mac_device_stru *pst_mac_device = NULL;

    pst_mac_chip = hmac_res_get_mac_chip(0);

    /* OAL接口获取支持device个数 */
    uc_device_max = oal_chip_get_device_num(pst_mac_chip->chip_ver);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++) {
        pst_mac_device = mac_res_get_dev(pst_mac_chip->auc_device_id[uc_device]);
        if (pst_mac_device == NULL) {
            continue;
        }

        /* 如果队列中有帧，则可以调度 */
        for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
            pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
            if (pst_hmac_vap == NULL) {
                oam_error_log0(uc_vap_idx, OAM_SF_ANY, "{hmac_tcp_ack_need_schedule::pst_hmac_vap null.}");
                continue;
            }

            if ((pst_hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_UP) &&
                (pst_hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_PAUSE)) {
                continue;
            }

            oal_spin_lock_bh(&pst_hmac_vap->st_hmac_tcp_ack[HCC_TX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
            head = &pst_hmac_vap->st_hmac_tcp_ack[HCC_TX].data_queue[HMAC_TCP_ACK_QUEUE];
            /* 队列中有ack帧，则需要调度线程 */
            if (oal_netbuf_list_len(head) > 0) {
                oal_spin_unlock_bh(&pst_hmac_vap->st_hmac_tcp_ack[HCC_TX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
                return OAL_TRUE;
            }
            oal_spin_unlock_bh(&pst_hmac_vap->st_hmac_tcp_ack[HCC_TX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
        }
    }
    return OAL_FALSE;
}

void hmac_sched_transfer(void)
{
    struct hcc_handler *hcc;

    hcc = hcc_get_110x_handler();
    oal_wait_queue_wake_up_interrupt(&hcc->hcc_transer_info.hcc_transfer_wq);
}
int32_t hmac_set_hmac_tcp_ack_process_func(hmac_tcp_ack_process_func p_func)
{
    struct hcc_handler *hcc;

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_set_hmac_tcp_ack_process_func::hcc null.}");
    } else {
        hcc->p_hmac_tcp_ack_process_func = p_func;
    }
    return OAL_SUCC;
}
int32_t hmac_set_hmac_tcp_ack_need_schedule(hmac_tcp_ack_need_schedule_func p_func)
{
    struct hcc_handler *hcc;

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_set_hmac_tcp_ack_process_func::hcc null.}");
    } else {
        hcc->p_hmac_tcp_ack_need_schedule_func = p_func;
    }
    return OAL_SUCC;
}
OAL_STATIC oal_bool_enum_uint8 hmac_transfer_rx_tcp_ack_handler(hmac_device_stru *pst_hmac_device,
    hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
#ifndef WIN32
    mac_rx_ctl_stru *pst_rx_ctrl = NULL; /* 指向MPDU控制块信息的指针 */
    mac_llc_snap_stru *pst_mac_llc_snap_netbuf = NULL;
    uint32_t buf_len = oal_netbuf_len(netbuf);

    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    if (buf_len < pst_rx_ctrl->uc_mac_header_len + sizeof(mac_llc_snap_stru)) {
        oam_error_log1(0, OAM_SF_TX, "{hmac_transfer_rx_tcp_ack_handler::buf_len[%d].}", buf_len);
        return OAL_FALSE;
    }
    buf_len -= pst_rx_ctrl->uc_mac_header_len + sizeof(mac_llc_snap_stru);
    pst_mac_llc_snap_netbuf = (mac_llc_snap_stru *)(netbuf->data + pst_rx_ctrl->uc_mac_header_len);
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    oam_warning_log1(0, OAM_SF_TX,
                     "{hmac_transfer_rx_handler::uc_mac_header_len = %d}\r\n", pst_rx_ctrl->uc_mac_header_len);
#endif
    if (OAL_TRUE == hmac_judge_rx_netbuf_classify(pst_mac_llc_snap_netbuf, buf_len)) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oam_warning_log0(0, OAM_SF_TX,
                         "{hmac_transfer_rx_handler::netbuf is tcp ack.}\r\n");
#endif
        oal_spin_lock_bh(&hmac_vap->st_hmac_tcp_ack[HCC_RX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
        oal_netbuf_list_tail(&hmac_vap->st_hmac_tcp_ack[HCC_RX].data_queue[HMAC_TCP_ACK_QUEUE],
                             netbuf);

        oal_spin_unlock_bh(&hmac_vap->st_hmac_tcp_ack[HCC_RX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
        hmac_sched_transfer();
        return OAL_TRUE;
    }
#endif
    return OAL_FALSE;
}


void hmac_transfer_rx_handle(hmac_device_stru *pst_hmac_device,
                             hmac_vap_stru *pst_hmac_vap,
                             oal_netbuf_head_stru *pst_netbuf_header)
{
    oal_netbuf_head_stru st_temp_header;
    oal_netbuf_stru *pst_netbuf = NULL;

    oal_netbuf_head_init(&st_temp_header);
    while (!!(pst_netbuf = oal_netbuf_delist_nolock(pst_netbuf_header))) {
        if (OAL_FALSE == hmac_transfer_rx_tcp_ack_handler(pst_hmac_device, pst_hmac_vap, pst_netbuf)) {
            oal_netbuf_list_tail_nolock(&st_temp_header, pst_netbuf);
        }
    }
    /*lint -e522*/
    oal_warn_on(!oal_netbuf_list_empty(pst_netbuf_header));
    /*lint +e522*/
    oal_netbuf_splice_init(&st_temp_header, pst_netbuf_header);
}
#endif /* end of _PRE_WLAN_TCP_OPT */