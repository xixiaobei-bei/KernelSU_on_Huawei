

#ifndef __HMAC_RX_DATA_H__
#define __HMAC_RX_DATA_H__

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "mac_frame.h"

#include "hmac_user.h"
#include "oal_net.h"
#include "hmac_tcp_opt.h"
#include "hmac_device.h"
#include "hmac_vap.h"
#include "hmac_resource.h"
#include "host_hal_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RX_DATA_H
/* 2 宏定义 */
#define mac_tid_is_valid(tid)         (((uint16_t)(tid)) < WLAN_TID_MAX_NUM)
#define HMAC_RX_DATA_ETHER_OFFSET_LENGTH 6      /* 代表发送lan的包，protocol type的以太头需要向前偏移6 */
#define HMAC_RX_SMALL_PKT_MIPS_LIMIT     30000L /* rx小包MIPS极限，当前为30us */
/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
/* HMAC 处理 TKIP MIC FAILE 异常传递给定时器的参数 */
typedef struct {
    uint8_t uc_vap_id; /* 传递给定时器的 vap_id */
    uint8_t uc_rsv[3]; // 3代表4字节对齐，保留字段
    frw_timeout_stru *pst_timeout; /* MIC faile 处理定时器资源 */
} hmac_drx_mic_faile_stru;
typedef struct {
    hal_host_device_stru *hal_dev;
    hmac_vap_stru *hmac_vap;
    hmac_user_stru *hmac_user;
    oal_netbuf_stru *netbuf;
    mac_rx_ctl_stru *cb;
} hmac_host_rx_context_stru;

/* 处理MPDU的MSDU的处理状态的结构体的定义 */
typedef struct {
    oal_netbuf_stru *pst_curr_netbuf;     /* 当前处理的netbuf指针 */
    uint8_t *puc_curr_netbuf_data;      /* 当前处理的netbuf的data指针 */
    uint16_t us_submsdu_offset;         /* 当前处理的submsdu的偏移量,   */
    uint8_t uc_msdu_nums_in_netbuf;     /* 当前netbuf包含的总的msdu数目 */
    uint8_t uc_procd_msdu_in_netbuf;    /* 当前netbuf中已处理的msdu数目 */
    uint8_t uc_netbuf_nums_in_mpdu;     /* 当前MPDU的中的总的netbuf的数目 */
    uint8_t uc_procd_netbuf_nums;       /* 当前MPDU中已处理的netbuf的数目 */
    uint8_t uc_procd_msdu_nums_in_mpdu; /* 当前MPDU中已处理的MSDU数目 */

    uint8_t uc_flag;
} dmac_msdu_proc_state_stru;

/* 每一个MSDU包含的内容的结构体的定义 */
typedef struct {
    uint8_t auc_sa[WLAN_MAC_ADDR_LEN]; /* MSDU发送的源地址 */
    uint8_t auc_da[WLAN_MAC_ADDR_LEN]; /* MSDU接收的目的地址 */
    uint8_t auc_ta[WLAN_MAC_ADDR_LEN]; /* MSDU接收的发送地址 */
    uint8_t uc_priority;
    uint8_t auc_resv[1];

    oal_netbuf_stru *pst_netbuf; /* MSDU对应的netbuf指针(可以使clone的netbuf) */
} dmac_msdu_stru;


typedef enum {
    HMAC_RX_FRAME_CTRL_GOON,   /* 本数据帧需要继续处理 */
    HMAC_RX_FRAME_CTRL_DROP,   /* 本数据帧需要丢弃 */
    HMAC_RX_FRAME_CTRL_BUTT
} hmac_rx_frame_ctrl_enum;
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
uint32_t hmac_rx_process_data_event(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_dev_rx_process_data_event(frw_event_mem_stru *pst_event_mem);
void hmac_rx_process_data_ap_tcp_ack_opt(hmac_vap_stru *pst_vap,
    oal_netbuf_head_stru *pst_netbuf_header);
uint32_t hmac_rx_process_data_sta_tcp_ack_opt(hmac_vap_stru *pst_vap,
    oal_netbuf_head_stru *pst_netbuf_header);
#ifdef _PRE_WLAN_DFT_DUMP_FRAME
void hmac_rx_report_eth_frame(mac_vap_stru *pst_mac_vap,
    oal_netbuf_stru *pst_netbuf);
#endif
void hmac_rx_free_netbuf(oal_netbuf_stru *pst_netbuf, uint16_t us_nums);
void hmac_rx_free_netbuf_list(oal_netbuf_head_stru *pst_netbuf_hdr, uint16_t us_nums_buf);
void hmac_rx_lan_frame(oal_netbuf_head_stru *pst_netbuf_header);

void hmac_rx_mpdu_to_netbuf_list(oal_netbuf_head_stru *netbuf_head, oal_netbuf_stru *netbuf_mpdu);
void hmac_rx_data_sta_proc(oal_netbuf_head_stru *netbuf_header);
void hmac_rx_data_ap_proc(hmac_vap_stru *hmac_vap, oal_netbuf_head_stru *p_netbuf_head);
uint32_t hmac_rx_frame_80211_to_eth(oal_netbuf_stru *pst_netbuf,
    const unsigned char *puc_da, const unsigned char *puc_sa);
oal_bool_enum_uint8 hmac_rx_is_qos_data_frame(mac_rx_ctl_stru *rx_ctl);
void hmac_rx_update_submsdu_rxctl(mac_rx_ctl_stru *dst_rxcb, mac_rx_ctl_stru *src_rxcb);
void hmac_rx_netbuf_list_free(oal_netbuf_stru *netbuf);
void hmac_rx_enqueue(oal_netbuf_stru *netbuf, oal_net_device_stru *net_dev);
void hmac_rx_wlan_to_wlan_proc(hmac_vap_stru *hmac_vap,
    oal_netbuf_head_stru *pw2w_netbuf_hdr);
void hmac_rx_vip_info(mac_vap_stru *vap, uint8_t data_type,
                      oal_netbuf_stru *pst_buf, mac_rx_ctl_stru *pst_rx_ctrl);
uint8_t *hmac_rx_get_eth_da(oal_netbuf_stru *netbuf);
uint32_t hmac_rx_copy_netbuff(oal_netbuf_stru **ppst_dest_netbuf,
                              oal_netbuf_stru *pst_src_netbuf, uint8_t uc_vap_id,
                              mac_ieee80211_frame_stru **ppul_mac_hdr_start_addr);
void hmac_rx_prepare_to_wlan(oal_netbuf_head_stru *netbuf_header, oal_netbuf_stru *netbuf);
uint32_t hmac_rx_data_wnd_proc(hmac_host_rx_context_stru *rx_context,
    hmac_ba_rx_stru *ba_rx_hdl, oal_netbuf_head_stru *rpt_list, uint8_t *buff_is_valid);
uint32_t hmac_rx_location_data_event(frw_event_mem_stru *event_mem);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_rx_data.h */
