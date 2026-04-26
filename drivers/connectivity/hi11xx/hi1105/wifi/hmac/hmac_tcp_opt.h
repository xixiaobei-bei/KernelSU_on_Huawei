

#ifndef __HMAC_TCP_OPT_H__
#define __HMAC_TCP_OPT_H__

#ifdef _PRE_WLAN_TCP_OPT
#include "oal_hcc_host_if.h"
#include "hmac_vap.h"
#include "hmac_device.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TCP_OPT_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define FIN_FLAG_BIT           (1 << 0)
#define SYN_FLAG_BIT           (1 << 1)
#define RESET_FLAG_BIT         (1 << 2)
#define URGENT_FLAG_BIT        (1 << 5)
#define FILTER_FLAG_MASK       (FIN_FLAG_BIT | SYN_FLAG_BIT | RESET_FLAG_BIT | URGENT_FLAG_BIT)
#define MAX_TIME_BY_TWO        (1 << 16)
#define HCC_ASSEMBLE_INFO_SIZE 16

struct hcc_handler *hcc_get_110x_handler(void);
void hmac_tcp_opt_ack_count_reset(hmac_vap_stru *pst_hmac_vap, hcc_chan_type dir, uint16_t stream);
void hmac_tcp_opt_ack_all_count_reset(hmac_vap_stru *pst_hmac_vap);
void hmac_tcp_opt_ack_show_count(hmac_vap_stru *pst_hmac_vap);
struct tcp_list_node *hmac_tcp_opt_find_oldest_node(hmac_vap_stru *pst_hmac_vap, hcc_chan_type dir);
struct tcp_list_node *hmac_tcp_opt_get_buf(hmac_vap_stru *pst_hmac_vap, hcc_chan_type dir);

uint32_t hmac_tcp_opt_add_node(hmac_vap_stru *pst_hmac_vap, struct wlan_tcp_flow *tcp_info,
    hcc_chan_type dir);
uint32_t hmac_tcp_opt_init_filter_tcp_ack_pool(hmac_vap_stru *pst_hmac_vap);
void hmac_tcp_opt_deinit_list(hmac_vap_stru *pst_hmac_vap);
uint32_t hmac_tcp_opt_get_flow_index(hmac_vap_stru *pst_hmac_vap, oal_ip_header_stru *pst_ip_hdr,
    oal_tcp_header_stru *pst_tcp_hdr, hcc_chan_type dir);

oal_tcp_ack_type_enum hmac_tcp_opt_get_tcp_ack_type(hmac_vap_stru *pst_hmac_vap,
    oal_ip_header_stru *pst_ip_hdr, hcc_chan_type dir, uint16_t us_index);
oal_bool_enum_uint8 hmac_judge_rx_netbuf_classify(mac_llc_snap_stru *pst_netbuff, uint32_t buf_len);
oal_bool_enum_uint8 hmac_judge_rx_netbuf_is_tcp_ack(mac_llc_snap_stru *pst_snap, uint32_t buf_len);
oal_bool_enum_uint8 hmac_judge_tx_netbuf_is_tcp_ack(oal_ether_header_stru *ps_ethmac_hdr);
oal_tcp_ack_type_enum hmac_tcp_opt_rx_get_tcp_ack(oal_netbuf_stru *skb,
    hmac_vap_stru *pst_hmac_vap, uint16_t *p_us_index, uint8_t dir);
oal_tcp_ack_type_enum hmac_tcp_opt_tx_get_tcp_ack(oal_netbuf_stru *skb, hmac_vap_stru *pst_hmac_vap,
    uint16_t *p_us_index, uint8_t dir);
uint16_t hmac_tcp_opt_tcp_ack_list_filter(hmac_vap_stru *pst_hmac_vap, hmac_tcp_opt_queue type,
    hcc_chan_type dir, oal_netbuf_head_stru *head);
uint16_t hmac_tcp_opt_rx_tcp_ack_filter(hmac_vap_stru *hmac_vap, hmac_tcp_opt_queue type,
    hcc_chan_type dir, oal_netbuf_head_stru *data);
uint16_t hmac_tcp_opt_tx_tcp_ack_filter(hmac_vap_stru *hmac_vap, hmac_tcp_opt_queue type,
    hcc_chan_type dir, oal_netbuf_head_stru *data);
uint32_t hmac_tcp_opt_tcp_ack_filter(oal_netbuf_stru *skb, hmac_vap_stru *pst_hmac_vap,
    hcc_chan_type dir);

void hmac_trans_queue_filter(hmac_vap_stru *pst_hmac_vap, oal_netbuf_head_stru *head_t,
    hmac_tcp_opt_queue type, hcc_chan_type dir);
void hmac_tcp_ack_process_hcc_queue(hmac_vap_stru *pst_hmac_vap,
    hcc_chan_type dir, hmac_tcp_opt_queue type);
int32_t hmac_tcp_ack_process(void);
oal_bool_enum_uint8 hmac_tcp_ack_need_schedule(void);
void hmac_sched_transfer(void);
int32_t hmac_set_hmac_tcp_ack_process_func(hmac_tcp_ack_process_func p_func);
int32_t hmac_set_hmac_tcp_ack_need_schedule(hmac_tcp_ack_need_schedule_func p_func);
extern void hmac_transfer_rx_handle(hmac_device_stru *pst_hmac_device,
                                    hmac_vap_stru *pst_hmac_vap,
                                    oal_netbuf_head_stru *pst_netbuf_header);

/* 11 内联函数定义 */
#endif /* end of _PRE_WLAN_TCP_OPT */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_tcp_opt.h */
