

#ifndef __HMAC_DEVICE_H__
#define __HMAC_DEVICE_H__

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "mac_vap.h"
#include "hmac_vap.h"
#ifdef _PRE_WLAN_TCP_OPT
#include "hmac_tcp_opt_struc.h"
#include "oal_hcc_host_if.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DEVICE_H

#ifdef __cplusplus // windows ut编译不过，后续下线清理
#if __cplusplus
extern "C" {
#endif
#endif

/* 2 宏定义 */
#ifdef _PRE_WLAN_TCP_OPT
#define HCC_TRANS_THREAD_POLICY   SCHED_FIFO
#define HCC_TRANS_THERAD_PRIORITY 10
#define HCC_TRANS_THERAD_NICE     (-10)
#endif

#define IS_EQUAL_RATES(rate1, rate2) (((rate1)&0x7f) == ((rate2)&0x7f))

typedef enum {
    HMAC_ACS_TYPE_INIT = 0, /* 初始自动信道选择 */
    HMAC_ACS_TYPE_CMD = 1,  /* 命令触发的自动信道选择 */
    HMAC_ACS_TYPE_FREE = 2, /* 空闲的自动信道选择 */
} hmac_acs_type_enum;

#ifdef _PRE_WLAN_FEATURE_PMF
#define PMF_BLACK_LIST_MAX_CNT 6
#endif

/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
/* 存储每个扫描到的bss信息 */
typedef struct {
    oal_dlist_head_stru st_dlist_head; /* 链表指针 */
    // oal_spin_lock_stru     st_lock;             /* 锁 */
    mac_bss_dscr_stru st_bss_dscr_info; /* bss描述信息，包括上报的管理帧 */
} hmac_scanned_bss_info;

/* 存储在hmac device下的扫描结果维护的结构体 */
typedef struct {
    oal_spin_lock_stru st_lock;
    oal_dlist_head_stru st_bss_list_head;
    uint32_t bss_num;
} hmac_bss_mgmt_stru;


/* 扫描运行结果记录 */
typedef struct {
    hmac_bss_mgmt_stru st_bss_mgmt;                                   /* 存储扫描BSS结果的管理结构 */
    wlan_scan_chan_stats_stru ast_chan_results[WLAN_MAX_CHANNEL_NUM]; /* 信道统计/测量结果 */
    uint8_t uc_chan_numbers;                                        /* 此次扫描总共需要扫描的信道个数 */
    uint8_t uc_device_id : 4;
    uint8_t uc_chip_id : 4;
    uint8_t uc_vap_id;                           /* 本次执行扫描的vap id */
    mac_scan_status_enum_uint8 en_scan_rsp_status; /* 本次扫描完成返回的状态码，是成功还是被拒绝 */

    oal_time_us_stru st_scan_start_timestamp; /* 扫描维测使用 */
    mac_scan_cb_fn p_fn_cb;                   /* 此次扫描结束的回调函数指针 */

    uint64_t ull_cookie;                      /* 保存P2P 监听结束上报的cookie 值 */
    mac_vap_state_enum_uint8 en_vap_last_state; /* 保存VAP进入扫描前的状态,AP/P2P GO模式下20/40M扫描专用 */
    oal_time_t_stru st_scan_start_time;         /* 扫描起始时间戳 */

    // 增加记录扫描类型，以便识别HiD2D扫描的相关处理
    wlan_scan_mode_enum_uint8 en_scan_mode;
} hmac_scan_record_stru;

/* 扫描相关相关控制信息 */
typedef struct {
    /* scan 相关控制信息 */
    oal_bool_enum_uint8 en_is_scanning;             /* host侧的扫描请求是否正在执行 */
    oal_bool_enum_uint8 en_is_random_mac_addr_scan; /* 是否为随机mac addr扫描，默认关闭(定制化宏开启下废弃) */
    oal_bool_enum_uint8 en_complete;                /* 内核普通扫描请求是否完成标志 */
    oal_bool_enum_uint8 en_sched_scan_complete;     /* 调度扫描是否正在运行标记 */

    oal_cfg80211_scan_request_stru *pst_request;              /* 内核下发的扫描请求结构体 */
    oal_cfg80211_sched_scan_request_stru *pst_sched_scan_req; /* 内核下发的调度扫描请求结构体 */

    oal_wait_queue_head_stru st_wait_queue;
    oal_spin_lock_stru st_scan_request_spinlock; /* 内核下发的request资源锁 */

    frw_timeout_stru st_scan_timeout; /* 扫描模块host侧的超时保护所使用的定时器 */
    frw_timeout_stru st_init_scan_timeout;
    uint8_t auc_random_mac[WLAN_MAC_ADDR_LEN]; /* 扫描时候用的随机MAC */
    uint8_t auc_resv[2]; // 2代表保留字节个数，4字节对齐

    hmac_scan_record_stru st_scan_record_mgmt; /* 扫描运行记录管理信息，包括扫描结果和发起扫描者的相关信息 */
} hmac_scan_stru;

typedef struct {
    oal_wait_queue_head_stru st_wait_queue;
    oal_bool_enum_uint8 auc_complete_flag[MAC_PSM_QUERY_TYPE_BUTT];
    mac_psm_query_stat_stru ast_psm_stat[MAC_PSM_QUERY_TYPE_BUTT];
}hmac_psm_flt_stat_query_stru;

#ifdef _PRE_WLAN_FEATURE_NRCOEX
typedef struct {
    oal_wait_queue_head_stru st_wait_queue;
    mac_nrcoex_stat_stru st_nrcoex_stat;
    oal_bool_enum_uint8 en_query_completed_flag;
} hmac_nrcoex_stat_query_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_PMF
typedef struct {
    uint8_t uc_cnt;
    uint8_t uc_first_idx; /* 用于记录最早的黑名单下标 */
    uint8_t auc_black_list[PMF_BLACK_LIST_MAX_CNT][WLAN_MAC_ADDR_LEN];
} hmac_pmf_black_list_stru;
#endif
/* hmac device结构体，记录只保存在hmac的device公共信息 */
typedef struct hmac_device_tag {
    hmac_scan_stru st_scan_mgmt; /* 扫描管理结构体 */
    uint32_t p2p_intf_status;
    oal_wait_queue_head_stru st_netif_change_event;
    mac_device_stru *pst_device_base_info; /* 指向公共部分mac device */
    oal_bool_enum_uint8 en_init_scan : 1;
    oal_bool_enum_uint8 en_rfu : 1;
    oal_bool_enum_uint8 en_in_init_scan : 1;
    oal_bool_enum_uint8 en_rescan_idle : 1;
    uint8_t uc_resv_bit : 4;
    uint8_t auc_resvx[3]; // 3代表保留字节个数，4字节对齐
    mac_channel_stru ast_best_channel[WLAN_BAND_BUTT];

#ifdef _PRE_WLAN_TCP_OPT
    oal_bool_enum_uint8 sys_tcp_rx_ack_opt_enable;
    oal_bool_enum_uint8 sys_tcp_tx_ack_opt_enable;
    oal_bool_enum_uint8 en_start_via_priv : 1;
    uint8_t uc_rfu_bit : 7;
    uint8_t uc_rfu;
#else
    oal_bool_enum_uint8 en_start_via_priv : 1;
    uint8_t uc_rfu_bit : 7;
    uint8_t auc_rfu[3]; // 3代表保留字节个数，4字节对齐
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef CONFIG_HAS_EARLYSUSPEND
    struct early_suspend early_suspend;  // early_suspend支持
#endif
    oal_spin_lock_stru st_suspend_lock;
#endif
    hmac_device_btcoex_stru st_hmac_device_btcoex;

    frw_timeout_stru st_scan_timer; /* host扫描定时器用于切换信道 */
    hmac_psm_flt_stat_query_stru st_psm_flt_stat_query;

    oal_bool_enum_uint8 en_ap_support_160m;

#ifdef _PRE_WLAN_FEATURE_PMF
    hmac_pmf_black_list_stru st_pmf_black_list;
#endif

#ifdef _PRE_WLAN_FEATURE_MONITOR
    oal_bool_enum_uint8 en_monitor_mode;
    uint8_t sniffer_mode;
    int8_t rssi0;
    int8_t rssi1;
    oal_spin_lock_stru d2h_sniffer_info;
    uint32_t mgmt_frames_cnt;
    uint32_t control_frames_cnt;
    uint32_t data_frames_cnt;
    uint32_t others_frames_cnt;
    oal_bool_enum_uint8 addr_filter_on;
    uint8_t monitor_mac_addr[WLAN_MAC_ADDR_LEN]; /* 需要过滤的MAC地址 */
#endif
#ifdef _PRE_WLAN_FEATURE_NRCOEX
    hmac_nrcoex_stat_query_stru st_nrcoex_stat_query;
#endif
#ifdef _PRE_WLAN_FEATURE_PSM_ABN_PKTS_STAT
    uint32_t addba_req_cnt; // addba req总数
    uint32_t group_rekey_cnt; // group_rekey总数
#endif
    hmac_device_stat_stru *device_stats;
} hmac_device_stru;

/* 8 UNION定义 */
/* 9 OTHERS定义 */
uint32_t hmac_board_exit(mac_board_stru *pst_board);

uint32_t hmac_config_host_dev_init(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_host_dev_exit(mac_vap_stru *pst_mac_vap);
uint32_t hmac_board_init(mac_board_stru *pst_board);

void hmac_device_create_random_mac_addr(mac_device_stru *pst_mac_dev, mac_vap_stru *pst_mac_vap);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_device.h */
