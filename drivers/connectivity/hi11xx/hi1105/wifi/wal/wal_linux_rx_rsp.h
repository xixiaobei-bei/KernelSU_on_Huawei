

#ifndef __WAL_LINUX_RX_RSP_H__
#define __WAL_LINUX_RX_RSP_H__

#include "oal_ext_if.h"
#include "oal_types.h"
#include "wal_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_RX_RSP_H

/* 驱动sta上报内核的扫描结果 */
typedef struct {
    int32_t l_signal; /* 信号强度 */

    int16_t s_freq; /* bss所在信道的中心频率 */
    uint8_t auc_arry[2]; /* 2实现字节对齐 */

    uint32_t mgmt_len;            /* 管理帧长度 */
    oal_ieee80211_mgmt_stru *pst_mgmt; /* 管理帧起始地址 */
} wal_scanned_bss_info_stru;

/* 驱动sta上报内核的关联结果 */
typedef struct {
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN]; /* sta关联的ap mac地址 */
    uint16_t us_status_code;              /* ieee协议规定的16位状态码 */

    uint8_t *puc_rsp_ie; /* asoc_req_ie  */
    uint8_t *puc_req_ie;

    uint32_t req_ie_len; /* asoc_req_ie len */
    uint32_t rsp_ie_len;

    uint16_t us_connect_status;
    uint8_t auc_resv[2]; /* 2实现字节对齐 */
} oal_connet_result_stru;

/* 驱动sta上报内核的去关联结果 */
typedef struct {
    uint16_t us_reason_code; /* 去关联 reason code */
    uint8_t auc_resv[2]; /* 2实现字节对齐 */

    uint8_t *pus_disconn_ie;    /* 去关联关联帧 ie */
    uint32_t us_disconn_ie_len; /* 去关联关联帧 ie 长度 */
} oal_disconnect_result_stru;

uint32_t wal_scan_comp_proc_sta(frw_event_mem_stru *pst_event_mem);
uint32_t wal_asoc_comp_proc_sta(frw_event_mem_stru *pst_event_mem);
uint32_t wal_disasoc_comp_proc_sta(frw_event_mem_stru *pst_event_mem);
uint32_t wal_connect_new_sta_proc_ap(frw_event_mem_stru *pst_event_mem);
uint32_t wal_disconnect_sta_proc_ap(frw_event_mem_stru *pst_event_mem);
uint32_t wal_mic_failure_proc(frw_event_mem_stru *pst_event_mem);
uint32_t wal_send_mgmt_to_host(frw_event_mem_stru *pst_event_mem);
uint32_t wal_p2p_listen_timeout(frw_event_mem_stru *pst_event_mem);
uint32_t wal_report_external_auth_req(frw_event_mem_stru *pst_event_mem);
uint32_t wal_report_channel_switch(frw_event_mem_stru *pst_event_mem);
uint32_t wal_nan_response_event_process(frw_event_mem_stru *event_mem);
uint32_t wal_report_rtt_result(frw_event_mem_stru *event_mem);
#endif /* end of wal_linux_rx_rsp.h */
