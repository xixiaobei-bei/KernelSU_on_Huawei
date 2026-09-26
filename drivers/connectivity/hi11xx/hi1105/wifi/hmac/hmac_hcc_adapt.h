

#ifndef __HMAC_HCC_ADAPT_H__
#define __HMAC_HCC_ADAPT_H__

#ifdef __cplusplus // windows ut编译不过，后续下线清理
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 其他头文件包含 */
#include "oam_ext_if.h"
#include "hmac_ext_if.h"

#include "frw_ext_if.h"
#include "frw_event_main.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_HCC_ADAPT_H

/* 2 宏定义 */
/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
/* Hcc 事件处理 */
uint32_t hmac_hcc_rx_event_comm_adapt(frw_event_mem_stru *pst_hcc_event_mem);
uint32_t hmac_hcc_rx_event_wow_comm_adapt(frw_event_mem_stru *pst_hcc_event_mem);
frw_event_mem_stru *hmac_hcc_test_rx_adapt(frw_event_mem_stru *pst_hcc_event_mem);

/* Rx适配部分 */
frw_event_mem_stru *hmac_rx_process_data_rx_adapt(frw_event_mem_stru *pst_hcc_event_mem);
frw_event_mem_stru *hmac_rx_process_wow_data_rx_adapt(frw_event_mem_stru *pst_hcc_event_mem);

frw_event_mem_stru *hmac_rx_process_mgmt_event_rx_adapt(
    frw_event_mem_stru *pst_hcc_event_mem);
frw_event_mem_stru *hmac_rx_process_wow_mgmt_event_rx_adapt(
    frw_event_mem_stru *pst_hcc_event_mem);

frw_event_mem_stru *hmac_hcc_rx_convert_netbuf_to_event_default(
    frw_event_mem_stru *pst_hcc_event_mem);
/* Tx适配部分 */
uint32_t hmac_proc_add_user_tx_adapt(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_proc_del_user_tx_adapt(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_proc_config_syn_tx_adapt(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_proc_config_syn_alg_tx_adapt(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_proc_tx_host_tx_adapt(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_proc_tx_device_ring_tx_adapt(frw_event_mem_stru *frw_event);
uint32_t hmac_test_hcc_tx_adapt(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_proc_set_edca_param_tx_adapt(frw_event_mem_stru *pst_event_mem);

uint32_t hmac_scan_proc_scan_req_event_tx_adapt(frw_event_mem_stru *pst_event_mem);
#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
uint32_t hmac_sdt_recv_sample_cmd_tx_adapt(frw_event_mem_stru *pst_event_mem);
#endif
#ifdef _PRE_WLAN_FEATURE_HIEX
uint32_t hmac_hiex_h2d_msg_tx_adapt(frw_event_mem_stru *pst_event_mem);
#endif
uint32_t hmac_send_event_netbuf_tx_adapt(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_config_update_ip_filter_tx_adapt(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_scan_proc_sched_scan_req_event_tx_adapt(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_proc_join_set_dtim_reg_event_tx_adapt(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_hcc_tx_convert_event_to_netbuf_uint32(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_hcc_tx_convert_event_to_netbuf_uint16(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_hcc_tx_convert_event_to_netbuf_uint8(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_hcc_tx_event_payload_to_netbuf(frw_event_mem_stru *pst_event_mem,
                                             uint32_t payload_size);
frw_event_mem_stru *hmac_rx_convert_netbuf_to_netbuf_default(
    frw_event_mem_stru *pst_hcc_event_mem);
frw_event_mem_stru *hmac_rx_convert_netbuf_to_netbuf_ring(frw_event_mem_stru *phcc_event_mem);

uint32_t hmac_proc_join_set_reg_event_tx_adapt(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_user_add_notify_alg_tx_adapt(frw_event_mem_stru *pst_event_mem);

uint32_t hmac_proc_rx_process_sync_event_tx_adapt(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_chan_select_channel_mac_tx_adapt(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_chan_initiate_switch_to_new_channel_tx_adapt(
    frw_event_mem_stru *pst_event_mem);
uint32_t hmac_sdt_recv_reg_cmd_tx_adapt(frw_event_mem_stru *pst_event_mem);
frw_event_mem_stru *hmac_cali2hmac_misc_event_rx_adapt(
    frw_event_mem_stru *pst_hcc_event_mem);

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
uint32_t hmac_edca_opt_stat_event_tx_adapt(frw_event_mem_stru *pst_event_mem);
#endif

frw_event_mem_stru *hmac_dpd_rx_adapt(frw_event_mem_stru *pst_hcc_event_mem);

#ifdef _PRE_WLAN_FEATURE_11AX
uint32_t hmac_proc_set_mu_edca_param_tx_adapt(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_proc_set_spatial_reuse_param_tx_adapt(frw_event_mem_stru *pst_event_mem);
#endif

#ifdef _PRE_WLAN_FEATURE_APF
frw_event_mem_stru *hmac_apf_program_report_rx_adapt(frw_event_mem_stru *pst_hcc_event_mem);
#endif
uint32_t check_headroom_add_length(mac_tx_ctl_stru *tx_ctl,
    uint8_t nest_type, uint8_t nest_sub_type, uint8_t cb_length);
uint32_t check_headroom_length(mac_tx_ctl_stru *tx_ctl,
    uint8_t nest_type, uint8_t nest_sub_type, uint8_t cb_length);

void hmac_adjust_netbuf_data(oal_netbuf_stru *netbuf, mac_tx_ctl_stru *tx_ctrl,
    uint8_t en_nest_type, uint8_t uc_nest_sub_type);
void hmac_format_netbuf_header(oal_netbuf_stru *netbuf, mac_tx_ctl_stru *tx_ctrl,
    uint8_t en_nest_type, uint8_t uc_nest_sub_type);
int32_t hmac_hcc_adapt_init(void);
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
int32_t hmac_tx_event_pkts_info_print(void *data, char *buf, int32_t buf_len);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_main */

