

#ifndef __HD_EVENT_H__
#define __HD_EVENT_H__

#include "mac_common.h"

#define HD_EVENT_RF_NUM 4
#define HD_EVENT_SLAVE_RF_NUM 2

/* p2p结构中包含此状态成员，该结构挂在mac device下，此VAP状态枚举移动到mac_device.h中 */
/* VAP状态机，AP STA共用一个状态枚举 */
typedef enum {
    /* ap sta公共状态 */
    MAC_VAP_STATE_INIT = 0,
    MAC_VAP_STATE_UP = 1,    /* VAP UP */
    MAC_VAP_STATE_PAUSE = 2, /* pause , for ap &sta */

    /* ap 独有状态 */
    MAC_VAP_STATE_AP_WAIT_START = 3,

    /* sta独有状态 */
    MAC_VAP_STATE_STA_FAKE_UP = 4,
    MAC_VAP_STATE_STA_WAIT_SCAN = 5,
    MAC_VAP_STATE_STA_SCAN_COMP = 6,
    MAC_VAP_STATE_STA_JOIN_COMP = 7,
    MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2 = 8,
    MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4 = 9,
    MAC_VAP_STATE_STA_AUTH_COMP = 10,
    MAC_VAP_STATE_STA_WAIT_ASOC = 11,
    MAC_VAP_STATE_STA_OBSS_SCAN = 12,
    MAC_VAP_STATE_STA_BG_SCAN = 13,
    MAC_VAP_STATE_STA_LISTEN = 14, /* p2p0 监听 */
    MAC_VAP_STATE_ROAMING = 15,    /* 漫游 */
    MAC_VAP_STATE_BUTT,
} mac_vap_state_enum;
typedef uint8_t mac_vap_state_enum_uint8;


typedef struct {
    uint16_t us_num_networks;
    mac_ch_type_enum_uint8 en_ch_type;
#ifdef _PRE_WLAN_FEATURE_DFS
    mac_chan_status_enum_uint8 en_ch_status;
#else
    uint8_t auc_resv[1];
#endif
} mac_ap_ch_info_stru; /* hd_event */

typedef void (*mac_scan_cb_fn)(void *p_scan_record);

#define MAX_PNO_SSID_COUNT      16

/* PNO扫描信息结构体 */
typedef struct {
    uint8_t auc_ssid[WLAN_SSID_MAX_LEN];
    oal_bool_enum_uint8 en_scan_ssid;
    uint8_t auc_resv[NUM_2_BYTES];
} pno_match_ssid_stru;

typedef struct {
    pno_match_ssid_stru ast_match_ssid_set[MAX_PNO_SSID_COUNT];
    int32_t l_ssid_count;                         /* 下发的需要匹配的ssid集的个数 */
    int32_t l_rssi_thold;                         /* 可上报的rssi门限 */
    uint32_t pno_scan_interval;                /* pno扫描间隔 */
    uint8_t auc_sour_mac_addr[WLAN_MAC_ADDR_LEN]; /* probe req帧中携带的发送端地址 */
    uint8_t uc_pno_scan_repeat;                   /* pno扫描重复次数 */
    oal_bool_enum_uint8 en_is_random_mac_addr_scan; /* 是否随机mac */

    mac_scan_cb_fn p_fn_cb; /* 函数指针必须放最后否则核间通信出问题 */
} mac_pno_scan_stru; /* hd_event */

typedef struct {
    uint8_t auc_ssid[WLAN_SSID_MAX_LEN];
    uint8_t auc_resv[NUM_3_BYTES];
} mac_ssid_stru;

typedef struct {
    uint8_t uc_mac_rate; /* MAC对应速率 */
    uint8_t uc_phy_rate; /* PHY对应速率 */
    uint8_t uc_mbps;     /* 速率 */
    uint8_t auc_resv[1];
} mac_data_rate_stru; /* hd_event */

typedef struct {
    uint16_t us_wifi_center_freq;  /* WiFi中心频率 */
    uint16_t us_modem_center_freq; /* Modem中心频率 */
    uint8_t uc_wifi_bw;            /* WiFi带宽 */
    uint8_t uc_modem_bw;           /* Modem带宽 */
    uint8_t uc_wifi_avoid_flag;    /* WiFi当前规避手段 */
    uint8_t uc_wifi_priority;      /* WiFi优先级 */
} mac_nrcoex_stat_stru; /* hd_event */

typedef struct {
    uint8_t auc_mac_addr[OAL_MAC_ADDR_LEN];
    uint8_t auc_rsv[NUM_2_BYTES];
} mac_cfg80211_init_port_stru; /* hd_event */

/* 扫描参数结构体 */
typedef struct {
    uint8_t en_bss_type; /* wlan_mib_desired_bsstype_enum 要扫描的bss类型 */
    wlan_scan_type_enum_uint8 en_scan_type;          /* 主动/被动 */
    uint8_t uc_bssid_num;                          /* 期望扫描的bssid个数 */
    uint8_t uc_ssid_num;                           /* 期望扫描的ssid个数 */

    uint8_t auc_sour_mac_addr[WLAN_MAC_ADDR_LEN]; /* probe req帧中携带的发送端地址 */
    uint8_t uc_p2p0_listen_channel;
    uint8_t uc_max_scan_count_per_channel; /* 每个信道的扫描次数 */

    uint8_t auc_bssid[WLAN_SCAN_REQ_MAX_BSSID][WLAN_MAC_ADDR_LEN]; /* 期望的bssid */
    mac_ssid_stru ast_mac_ssid_set[WLAN_SCAN_REQ_MAX_SSID];          /* 期望的ssid */

    uint8_t uc_max_send_probe_req_count_per_channel; /* 每次信道发送扫描请求帧的个数，默认为1 */
    uint8_t bit_is_p2p0_scan : 1;                    /* 是否为p2p0 发起扫描 */
    uint8_t bit_is_radom_mac_saved : 1;              /* 是否已经保存随机mac */
    uint8_t bit_radom_mac_saved_to_dev : 2;          /* 用于并发扫描 */
    uint8_t bit_desire_fast_scan : 1;                /* 本次扫描期望使用并发 */
    uint8_t bit_roc_type_tx_mgmt : 1;             /* remain on channel类型是否为发送管理帧 */
    uint8_t bit_rsv : 2;                          /* 保留位 */

    oal_bool_enum_uint8 en_abort_scan_flag;         /* 终止扫描 */
    oal_bool_enum_uint8 en_is_random_mac_addr_scan; /* 是否是随机mac addr扫描 */

    oal_bool_enum_uint8 en_need_switch_back_home_channel; /* 背景扫描时，扫描完一个信道，判断是否需要切回工作信道工作 */
    uint8_t uc_scan_channel_interval;                   /* 间隔n个信道，切回工作信道工作一段时间 */
    uint16_t us_work_time_on_home_channel;              /* 背景扫描时，返回工作信道工作的时间 */

    mac_channel_stru ast_channel_list[WLAN_MAX_CHANNEL_NUM];

    uint8_t uc_channel_nums; /* 信道列表中信道的个数 */
    uint8_t uc_probe_delay;  /* 主动扫描发probe request帧之前的等待时间 */
    uint16_t us_scan_time;   /* 扫描在某一信道停留此时间后，扫描结束, ms，必须是10的整数倍 */

    wlan_scan_mode_enum_uint8 en_scan_mode; /* 扫描模式:前景扫描 or 背景扫描 */
    uint8_t uc_resv;
    uint8_t uc_scan_func; /* DMAC SCANNER 扫描模式 */
    uint8_t uc_vap_id;    /* 下发扫描请求的vap id */
    uint64_t ull_cookie;  /* P2P 监听下发的cookie 值 */

    oal_bool_enum_uint8 uc_neighbor_report_process_flag; /* 标记此次扫描是否是neighbor report扫描 */

    oal_bool_enum_uint8 uc_bss_transition_process_flag;  /* 标记此次扫描是否是bss transition扫描 */

    /* 重要:回调函数指针:函数指针必须放最后否则核间通信出问题 */
    mac_scan_cb_fn p_fn_cb;
} mac_scan_req_stru;

/* 内核flag保持一致 */
typedef struct {
    uint32_t scan_flag; /* 内核下发的扫描模式,每个bit意义见wlan_scan_flag_enum，暂时只解析是否为并发扫描 */
    mac_scan_req_stru st_scan_params;
} mac_scan_req_h2d_stru;

typedef struct {
    uint8_t uc_chan_number;                       /* 主20MHz信道号 */
    uint8_t en_band : 4,
            ext6g_band : 1, /* 指示是否为6G频段标志 */
            resv : 3;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽模式 */
    uint8_t uc_chan_idx;                          /* 信道索引号 */
    wlan_scan_type_enum_uint8 scan_policy; /* 扫描策略 */
    uint8_t scan_time;
} mac_scan_channel_stru;

typedef struct {
    /* 内核下发的扫描模式,每个bit意义见wlan_scan_flag_enum，暂时只解析是否为并发扫描 */
    uint32_t scan_flag;

    uint8_t en_bss_type; /* wlan_mib_desired_bsstype_enum 要扫描的bss类型 */
    wlan_scan_type_enum_uint8 en_scan_type;          /* 主动/被动 */
    uint8_t uc_bssid_num;                          /* 期望扫描的bssid个数 */
    uint8_t uc_ssid_num;                           /* 期望扫描的ssid个数 */

    uint8_t auc_sour_mac_addr[WLAN_MAC_ADDR_LEN]; /* probe req帧中携带的发送端地址 */
    uint8_t uc_p2p0_listen_channel;
    uint8_t uc_max_scan_count_per_channel; /* 每个信道的扫描次数 */

    uint8_t auc_bssid[WLAN_SCAN_REQ_MAX_BSSID][WLAN_MAC_ADDR_LEN]; /* 期望的bssid */
    mac_ssid_stru ast_mac_ssid_set[WLAN_SCAN_REQ_MAX_SSID];          /* 期望的ssid */

    uint8_t uc_max_send_probe_req_count_per_channel; /* 每次信道发送扫描请求帧的个数，默认为1 */
    uint8_t bit_is_p2p0_scan : 1;                    /* 是否为p2p0 发起扫描 */
    uint8_t bit_is_radom_mac_saved : 1;              /* 是否已经保存随机mac */
    uint8_t bit_radom_mac_saved_to_dev : 2;          /* 用于并发扫描 */
    uint8_t bit_desire_fast_scan : 1;                /* 本次扫描期望使用并发 */
    uint8_t bit_roc_type_tx_mgmt : 1;             /* remain on channel类型是否为发送管理帧 */
    uint8_t bit_rsv : 2;                          /* 保留位 */

    oal_bool_enum_uint8 en_abort_scan_flag;         /* 终止扫描 */
    oal_bool_enum_uint8 en_is_random_mac_addr_scan; /* 是否是随机mac addr扫描 */

    oal_bool_enum_uint8 en_need_switch_back_home_channel; /* 背景扫描时，扫描完一个信道，判断是否需要切回工作信道工作 */
    uint8_t uc_scan_channel_interval;                   /* 间隔n个信道，切回工作信道工作一段时间 */
    uint16_t us_work_time_on_home_channel;              /* 背景扫描时，返回工作信道工作的时间 */

    mac_scan_channel_stru ast_channel_list[WLAN_MAX_CHANNEL_NUM];

    uint8_t uc_channel_nums; /* 信道列表中信道的个数 */
    uint8_t uc_probe_delay;  /* 主动扫描发probe request帧之前的等待时间 */
    uint16_t us_scan_time;   /* 扫描在某一信道停留此时间后，扫描结束, ms，必须是10的整数倍 */

    wlan_scan_mode_enum_uint8 en_scan_mode; /* 扫描模式:前景扫描 or 背景扫描 */
    uint8_t uc_resv;
    uint8_t uc_scan_func; /* DMAC SCANNER 扫描模式 */
    uint8_t uc_vap_id;    /* 下发扫描请求的vap id */
    uint64_t ull_cookie;  /* P2P 监听下发的cookie 值 */

    oal_bool_enum_uint8 uc_neighbor_report_process_flag; /* 标记此次扫描是否是neighbor report扫描 */

    oal_bool_enum_uint8 uc_bss_transition_process_flag;  /* 标记此次扫描是否是bss transition扫描 */

    /* 重要:回调函数指针:函数指针必须放最后否则核间通信出问题 */
    mac_scan_cb_fn p_fn_cb;
} mac_scan_req_ex_stru; /* hd_event 1106 only */

/* host/device管制域同步结构体 */
typedef struct {
    mac_rc_start_freq_enum_uint8 en_start_freq; /* 起始频率 */
    mac_ch_spacing_enum_uint8 en_ch_spacing;    /* 信道间距 */
    uint8_t uc_behaviour_bmap;                /* 允许的行为位图 位图定义见mac_behaviour_bmap_enum */
    uint8_t uc_coverage_class;                /* 覆盖类 */
    uint8_t uc_max_reg_tx_pwr;                /* 管制类规定的最大发送功率, 单位dBm */
    /* 实际使用的最大发送功率,扩大了10倍用于计算, 单位dBm，可以比管制域规定功率大 */
    uint16_t us_max_tx_pwr;
    uint8_t auc_resv[1];
    uint32_t channel_bmap; /* 支持信道位图，例 0011表示支持的信道的index为0 1 */
} mac_regclass_info_stru;

/* 管制域信息结构体 */
/* 管制类值、管制类位图与管制类信息 数组下表的关系
    管制类取值        : .... 7  6  5  4  3  2  1  0
    管制类位图        : .... 1  1  0  1  1  1  0  1
    管制类信息数组下标: .... 5  4  x  3  2  1  x  0
*/
typedef struct {
    int8_t ac_country[WLAN_COUNTRY_STR_LEN]; /* 国家字符串 */
    mac_dfs_domain_enum_uint8 en_dfs_domain;   /* DFS 雷达标准 */
    uint8_t uc_regclass_num;                 /* 管制类个数 */
    regdomain_enum_uint8 en_regdomain;
    uint8_t auc_resv[2]; /* 预留2字节 */
    mac_regclass_info_stru ast_regclass[WLAN_MAX_RC_NUM]; /* 管制域包含的管制类信息，注意 此成员只能放在最后一项! */
} mac_regdomain_info_stru; /* hd_event */

#define MAC_RD_INFO_LEN 12 /* mac_regdomain_info_stru去掉mac_regclass_info_stru的长度 */

/* device reset事件同步结构体 */
typedef struct {
    mac_reset_sys_type_enum_uint8 en_reset_sys_type; /* 复位同步类型 */
    uint8_t uc_value;                              /* 同步信息值 */
    uint8_t uc_resv[NUM_2_BYTES];
} mac_reset_sys_stru; /* hd_event */

/* HAL模块和DMAC模块共用的WLAN RX结构体 */
typedef struct {
    oal_netbuf_stru *pst_netbuf; /* netbuf链表一个元素 */
    uint16_t us_netbuf_num;    /* netbuf链表的个数 */
    uint8_t auc_resv[BYTE_OFFSET_2];       /* 字节对齐 */
} hal_cali_hal2hmac_event_stru; /* hd_event */

typedef struct {
    uint32_t packet_idx;

    uint8_t auc_payoald[4];  /* 地址 4 */
} hal_cali_hal2hmac_payload_stru; /* hd_event 1103/1105 */

/* begin:from mac_common.h */
typedef struct {
    dmac_disasoc_misc_reason_enum_uint16 en_disasoc_reason;
    uint16_t us_user_idx;
} dmac_disasoc_misc_stru; /* hd_event */
/* DMAC与HMAC模块共用的WLAN DRX事件结构体 */
typedef struct {
    oal_netbuf_stru *pst_netbuf; /* netbuf链表一个元素 */
    uint16_t us_netbuf_num;    /* netbuf链表的个数 */
    uint8_t auc_resv[NUM_2_BYTES];       /* 字节对齐 */
} dmac_wlan_drx_event_stru; /* hd_event */

/* DMAC与HMAC模块共用的WLAN CRX事件结构体 */
typedef struct {
    oal_netbuf_stru *pst_netbuf; /* 指向管理帧对应的netbuf */
    //    uint8_t              *puc_chtxt;          /* Shared Key认证用的challenge text */
} dmac_wlan_crx_event_stru; /* hd_event */

typedef struct {
    uint16_t us_user_id;
    uint8_t uc_ba_size;
    oal_bool_enum_uint8 en_need_delba;
} dmac_to_hmac_btcoex_rx_delba_trigger_event_stru; /* hd_event */

typedef struct {
    uint16_t us_user_id;
    oal_bool_enum_uint8 en_rx_amsdu;
    uint8_t uc_ps_type;
} dmac_to_hmac_ps_rx_delba_event_stru; /* hd_event */

typedef struct {
    uint16_t us_user_id;
    oal_bool_enum_uint8 en_arp_probe;
    uint8_t uc_ps_type;
} dmac_to_hmac_ps_arp_probe_event_stru; /* hd_event */

typedef struct {
    uint16_t us_user_index;
    uint8_t uc_tid;
    uint8_t uc_vap_id;
    uint8_t uc_cur_protocol;
    uint8_t auc_reserve[NUM_3_BYTES];
} dmac_to_hmac_ctx_event_stru; /* hd_event */

typedef struct {
    uint16_t us_user_index;
    uint8_t uc_cur_bandwidth;
    uint8_t uc_cur_protocol;
} dmac_to_hmac_syn_info_event_stru; /* hd_event */

typedef struct {
    uint8_t uc_vap_id;
    uint8_t en_voice_aggr; /* 是否支持Voice聚合 */
    uint8_t auc_resv[NUM_2_BYTES];
} dmac_to_hmac_voice_aggr_event_stru; /* hd_event */

typedef struct {
    uint8_t uc_device_id;
    wlan_nss_enum_uint8 en_m2s_nss;
    wlan_m2s_type_enum_uint8 en_m2s_type; /* 0:软切换 1:硬切换 */
    uint8_t auc_reserve[1];
} dmac_to_hmac_m2s_event_stru; /* hd_event */

/* d2h hal vap信息同步 */
typedef struct {
    uint8_t hal_dev_id;
    uint8_t mac_vap_id;
    uint8_t hal_vap_id;
    uint8_t valid;
} d2h_hal_vap_info_syn_event; /* hd_event */

/* d2h lut信息同步 */
typedef struct {
    uint16_t usr_idx;
    uint8_t  lut_idx;
    uint8_t  hal_dev_id;
    uint8_t  valid;
} d2h_usr_lut_info_syn_event; /* hd_event */

/* mic攻击 */
typedef struct {
    uint8_t auc_user_mac[WLAN_MAC_ADDR_LEN];
    uint8_t auc_reserve[NUM_2_BYTES];
    oal_nl80211_key_type en_key_type;
    int32_t l_key_id;
} dmac_to_hmac_mic_event_stru;  /* hd_event */

/* DMAC与HMAC模块共用的DTX事件结构体 */
typedef struct {
    oal_netbuf_stru *pst_netbuf; /* netbuf链表一个元素 */
    uint16_t us_frame_len;
    uint16_t us_remain;
} dmac_tx_event_stru; /* hd_event */
/* 存在MIMO兼容性AP，需抛事件通知hmac重关联，事件结构体 */
typedef struct {
    uint16_t us_user_id;
    mac_ap_type_enum_uint16 en_ap_type;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN];
    uint8_t auc_resv[NUM_2_BYTES];
} dmac_to_hmac_mimo_compatibility_event_stru; /* hd_event */

typedef struct {
    int8_t rssi0;
    int8_t rssi1;
} dmac_to_hmac_sniffer_info_event_stru;

typedef struct {
    mac_channel_stru st_channel;
    mac_ch_switch_info_stru st_ch_switch_info;

    oal_bool_enum_uint8 en_switch_immediately; /* 1 - 马上切换  0 - 暂不切换, 推迟到tbtt中切换 */
    oal_bool_enum_uint8 en_check_cac;
    oal_bool_enum_uint8 en_dot11FortyMHzIntolerant;
    uint8_t auc_resv[1];
} dmac_set_chan_stru; /* hd_event */

/* 挂载在dmac_ctx_action_event_stru传递给dmac */
typedef struct {
    uint8_t uc_ac;
    uint8_t bit_psb : 1;
    uint8_t bit_direction : 7;
} dmac_addts_info_stru;

/*
 *  (1)DMAC与HMAC模块共用的CTX子类型ACTION对应的事件的结构体
 *  (2)当DMAC自身产生DELBA帧时，使用该结构体向HMAC模块抛事件
 */
typedef struct {
    mac_category_enum_uint8 en_action_category; /* ACTION帧的类型 */
    uint8_t uc_action;                        /* 不同ACTION类下的子帧类型 */
    uint16_t us_user_idx;
    uint16_t us_frame_len; /* 帧长度 */
    uint8_t uc_hdr_len;    /* 帧头长度 */
    uint8_t uc_tidno;      /* tidno，部分action帧使用 */
    uint8_t uc_initiator;  /* 触发端方向 */

    /* 以下为接收到req帧，发送rsp帧后，需要同步到dmac的内容 */
    uint8_t uc_status;                       /* rsp帧中的状态 */
    uint16_t us_baw_start;                   /* 窗口开始序列号 */
    uint16_t us_baw_size;                    /* 窗口大小 */
    uint8_t uc_ampdu_max_num;                /* BA会话下的最多聚合的AMPDU的个数 */
    oal_bool_enum_uint8 en_amsdu_supp;         /* 是否支持AMSDU */
    uint16_t us_ba_timeout;                  /* BA会话交互超时时间 */
    mac_back_variant_enum_uint8 en_back_var;   /* BA会话的变体 */
    uint8_t uc_dialog_token;                 /* ADDBA交互帧的dialog token */
    uint8_t uc_ba_policy;                    /* Immediate=1 Delayed=0 */
    uint8_t uc_lut_index;                    /* LUT索引 */
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /* 用于DELBA/DELTS查找HMAC用户 */
#ifdef _PRE_WLAN_FEATURE_WMMAC
    uint8_t uc_tsid;            /* TS相关Action帧中的tsid值 */
    uint8_t uc_ts_dialog_token; /* ADDTS交互帧的dialog token */
    dmac_addts_info_stru st_addts_info;
#endif
} dmac_ctx_action_event_stru; /* hd_event */

typedef struct {
    uint8_t uc_scan_idx;
    uint8_t auc_resv[NUM_3_BYTES];
    wlan_scan_chan_stats_stru st_chan_result;
} dmac_crx_chan_result_stru; /* hd_event */

/* hmac to dmac配置同步消息结构 */
typedef struct {
    wlan_cfgid_enum_uint16 en_syn_id; /* 同步事件ID */
    uint16_t us_len;                /* 事件payload长度 */
    uint8_t auc_msg_body[NUM_4_BYTES];        /* 事件payload */
} hmac_to_dmac_cfg_msg_stru; /* hd_event */
typedef hmac_to_dmac_cfg_msg_stru dmac_to_hmac_cfg_msg_stru; /* hd_event */

typedef struct {
    uint8_t uc_reason;
    uint8_t uc_event;
    uint8_t auc_des_addr[WLAN_MAC_ADDR_LEN];
} dmac_diasoc_deauth_event; /* hd_event */

typedef struct {
    uint32_t event_para; /* 消息传输的数据 */
    uint32_t fail_num;
    int16_t s_always_rx_rssi;
    uint8_t uc_event_id; /* 消息号 */
    uint8_t uc_reserved;
} dmac_atcmdsrv_atcmd_response_event; /* hd_event */

typedef struct {
    uint32_t tx_ring_base_addr;
} mac_d2h_tx_ring_base_addr_stru; /* hd_event */

typedef struct {
    uint8_t flags;
    uint8_t mcs;
    uint16_t legacy;
    uint8_t nss;
    uint8_t bw;
    uint8_t rsv[1];
} tx_rate_info_stru;

typedef struct {
    tx_rate_info_stru tx_rate;
    uint16_t        rx_rate_legacy;
} txrx_rate_info_stru;

typedef struct {
    uint8_t flags;
    uint8_t mcs;
    uint16_t legacy;
    uint8_t nss;
    uint8_t bw;
    uint8_t beacon_rssi; /* 收到的beacon信号强度 */
    uint8_t rsv[NUM_2_BYTES];
} mac_rate_info_stru; /* hd_event */

typedef struct {
    uint8_t query_event;         /* 消息号 */
    int8_t c_signal;             /* 信号强度 */
    uint16_t uplink_dealy;       /* 上行时延 */
    uint32_t rx_packets;      /* total packets received */
    uint32_t tx_packets;      /* total packets transmitted */
    uint32_t rx_bytes;        /* total bytes received     */
    uint32_t tx_bytes;        /* total bytes transmitted  */
    uint32_t tx_retries;      /* 发送重传次数 */
    uint32_t rx_dropped_misc; /* 接收失败次数 */
    uint32_t tx_failed;       /* 发送失败次数 */
    int16_t s_free_power;        /* 底噪 */
    uint16_t s_chload;           /* 信道繁忙程度 */
    station_info_extend_stru st_station_info_extend;
    union {
        mac_rate_info_stru  tx_rate;   /* ROM部分使用，不可更改，新增函数使用txrx_rate */
        txrx_rate_info_stru txrx_rate;
    } rate_info;
} dmac_query_station_info_response_event; /* hd_event */

typedef struct {
    uint32_t uc_dscr_status;
    uint8_t mgmt_frame_id;
    uint8_t auc_resv[1];
    uint16_t us_user_idx;
} dmac_crx_mgmt_tx_status_stru; /* hd_event */

#ifdef _PRE_WLAN_FEATURE_M2S
/* MSS使用时命令形式 */
typedef enum {
    MAC_M2S_COMMAND_MODE_SET_AUTO = 0,
    MAC_M2S_COMMAND_MODE_SET_SISO_C0 = 1,
    MAC_M2S_COMMAND_MODE_SET_SISO_C1 = 2,
    MAC_M2S_COMMAND_MODE_SET_MIMO = 3,
    MAC_M2S_COMMAND_MODE_GET_STATE = 4,
    MAC_MCM_COMMAND_MODE_PROTECT = 5,

    MAC_M2S_COMMAND_MODE_BUTT,
} mac_m2s_command_mode_enum;
typedef uint8_t mac_m2s_command_mode_enum_uint8;

/* 配置命令使用，mimo-siso切换mode */
typedef enum {
    MAC_M2S_MODE_QUERY          = 0,    /* 参数查询模式 */
    MAC_M2S_MODE_MSS            = 1,    /* MSS下发模式 */
    MAC_M2S_MODE_DELAY_SWITCH   = 2,    /* 延迟切换测试模式 */
    MAC_M2S_MODE_SW_TEST        = 3,    /* 软切换测试模式,测试siso和mimo */
    MAC_M2S_MODE_HW_TEST        = 4,    /* 硬切换测试模式,测试siso和mimo */
    MAC_M2S_MODE_RSSI           = 5,    /* rssi切换 */
    MAC_M2S_MODE_MODEM          = 6,    /* MODEM切换模式 */
    MAC_M2S_MODE_BTCOEX         = 7,    /* btcoex切换模式 */

    MAC_M2S_MODE_BUTT,
} mac_m2s_mode_enum;
typedef uint8_t mac_m2s_mode_enum_uint8;

/* MODEM命令使用，期望切换状态 */
typedef enum {
    MAC_MCM_MODEM_STATE_SISO_C0     = 0,
    MAC_MCM_MODEM_STATE_MIMO        = 2,

    MAC_MCM_MODEM_STATE_BUTT,
} mac_mcm_modem_state_enum;
typedef uint8_t mac_mcm_modem_state_enum_uint8;

typedef struct {
    /* 0:参数查询模式; 1:参数配置模式;2.切换模式;3.软切换测试模式;4.硬切换测试模式 5.RSSI配置命令 */
    mac_m2s_mode_enum_uint8         en_cfg_m2s_mode;
    union {
        struct {
            oal_bool_enum_uint8      en_m2s_type;    /* 切换类型 */
            uint8_t                uc_master_id;   /* 主辅路id */
            uint8_t uc_m2s_state;   /* 期望切换到状态 */
            wlan_m2s_trigger_mode_enum_uint8 uc_trigger_mode; /* 切换触发业务模式 */
        } test_mode;

        struct {
            oal_bool_enum_uint8 en_mss_on;
        } mss_mode;

        struct {
            uint8_t uc_opt;
            int8_t c_value;
        } rssi_mode;

        struct {
            mac_mcm_modem_state_enum_uint8    en_m2s_state;   /* 期望切换到状态 */
        } modem_mode;
        struct {
            uint8_t chain_apply;
            uint8_t ant_occu_prio;
        } btcoex_mode;
    } pri_data;
} mac_m2s_mgr_stru; /* hd_event */

typedef struct {
    wlan_m2s_mgr_vap_stru ast_m2s_comp_vap[WLAN_SERVICE_STA_MAX_NUM_PER_DEVICE];
    uint8_t uc_m2s_state; /* 当前m2s状态 */
    union {
        struct {
            oal_bool_enum_uint8 en_m2s_result;
            uint8_t uc_m2s_mode; /* 当前切换业务 */
            uint8_t uc_vap_num;
        } mss_result;

        struct {
            oal_bool_enum_uint8 en_arp_detect_on;
        } arp_detect_result;
    } pri_data;
} dmac_m2s_complete_syn_stru; /* hd_event */
#endif

typedef struct {
    uint16_t user_id;
    uint8_t ac_idx;
    uint8_t need_degrade;
} dmac_wmmac_info_sync_stru; /* hd_event */

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
typedef struct {
    int32_t l_core_idx;
    int32_t l_rssi;
    int32_t aul_rsv[NUM_4_BYTES];
} dmac_tas_rssi_notify_stru; /* hd_event */

typedef struct {
    uint8_t uc_core_idx;
    oal_bool_enum_uint8 en_need_improved;
    uint8_t auc_rev[NUM_2_BYTES];
} mac_cfg_tas_pwr_ctrl_stru; /* hd_event */
#endif

#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
typedef struct {
    uint32_t type;
    uint32_t reg_num;
    uint32_t count;
    uint32_t event_data;
} dmac_sdt_sample_frame_stru; /* hd_event */
#endif

#ifdef _PRE_WLAN_FEATURE_APF
typedef enum {
    APF_SET_FILTER_CMD,
    APF_GET_FILTER_CMD
} mac_apf_cmd_type_enum;
typedef uint8_t mac_apf_cmd_type_uint8;

typedef struct {
    void *p_program; /* netbuf链表一个元素 */
    uint16_t us_program_len;
} dmac_apf_report_event_stru; /* hd_event */
#endif

typedef struct {
    wlan_ch_switch_status_enum_uint8 en_ch_switch_status;     /* 信道切换状态 */
    uint8_t uc_announced_channel;                           /* 新信道号 */
    wlan_channel_bandwidth_enum_uint8 en_announced_bandwidth; /* 新带宽模式 */
    uint8_t uc_ch_switch_cnt;                               /* 信道切换计数 */
    oal_bool_enum_uint8 en_csa_present_in_bcn;                /* Beacon帧中是否包含CSA IE */
    uint8_t uc_csa_vap_cnt;                                 /* 需要发送csa的vap个数 */
    wlan_csa_mode_tx_enum_uint8 en_csa_mode;
    oal_bool_enum_uint8 en_go_csa_is_running;
} dmac_set_ch_switch_info_stru; /* hd_event */

typedef struct {
    uint8_t uc_cac_machw_en; /* 1min cac 发送队列开关状态 */
} dmac_set_cac_machw_info_stru; /* hd_event */


#ifdef _PRE_WLAN_FEATURE_TWT
typedef struct {
    mac_category_enum_uint8 en_action_category; /* ACTION帧的类型 */
    uint8_t uc_action;                        /* 不同ACTION类下的子帧类型 */
    uint16_t us_user_idx;
    mac_cfg_twt_stru st_twt_cfg;
    uint8_t twt_idx; /* 在host占用第几个资源 */
    uint8_t resv[NUM_3_BYTES];
} dmac_ctx_update_twt_stru; /* hd_event */
#endif

/* 添加用户事件payload结构体 */
typedef struct {
    uint16_t us_user_idx; /* 用户index */
    uint8_t auc_user_mac_addr[WLAN_MAC_ADDR_LEN];
    uint16_t us_sta_aid;
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];

    mac_vht_hdl_stru st_vht_hdl;
    mac_user_ht_hdl_stru st_ht_hdl;
    mac_ap_type_enum_uint16 en_ap_type;
    int8_t c_rssi; /* 用户bss的信号强度 */
    uint8_t lut_index;
} dmac_ctx_add_user_stru; /* hd_event */

/* 删除用户事件结构体 */
typedef struct {
    uint16_t us_user_idx; /* 用户index */
    uint8_t auc_user_mac_addr[WLAN_MAC_ADDR_LEN];
    mac_ap_type_enum_uint16 en_ap_type;
    uint8_t auc_resv[NUM_2_BYTES];
} dmac_ctx_del_user_stru; /* hd_event */

/* Update join req 需要配置的速率集参数 */
typedef struct {
    union {
        uint8_t uc_value;
        struct {
            uint8_t bit_support_11b : 1;  /* 该AP是否支持11b */
            uint8_t bit_support_11ag : 1; /* 该AP是否支持11ag */
            uint8_t bit_ht_capable : 1;   /* 是否支持ht */
            uint8_t bit_vht_capable : 1;  /* 是否支持vht */
            uint8_t bit_reserved : 4;
        } st_capable; /* 与dmac层wlan_phy_protocol_enum对应 */
    } un_capable_flag;
    uint8_t uc_min_rate[NUM_2_BYTES]; /* 第一个存储11b协议对应的速率，第二个存储11ag协议对应的速率 */
    uint8_t uc_reserved;
} dmac_set_rate_stru;

typedef struct {
    uint8_t auc_transmitted_bssid[WLAN_MAC_ADDR_LEN]; /* transmitted bssid */
    uint8_t bit_is_non_transimitted_bss : 1,
              bit_ext_cap_multi_bssid_activated : 1,
              bit_rsv : 6;
    uint8_t auc_rcv[1];
} mac_multi_bssid_info; /* hd_event */

/* Update join req 参数写寄存器的结构体定义 */
typedef struct {
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN]; /* 加入的AP的BSSID  */
    uint16_t us_beacon_period;
    mac_channel_stru st_current_channel;                      /* 要切换的信道信息 */
    uint32_t beacon_filter;                              /* 过滤beacon帧的滤波器开启标识位 */
    uint32_t non_frame_filter;                           /* 过滤no_frame帧的滤波器开启标识位 */
    uint8_t auc_ssid[WLAN_SSID_MAX_LEN];                    /* 加入的AP的SSID  */
    uint8_t uc_dtim_period;                                 /* dtim period      */
    oal_bool_enum_uint8 en_dot11FortyMHzOperationImplemented; /* dot11FortyMHzOperationImplemented */
    uint8_t set_he_cap : 1,
            set_vht_cap : 1,
              bit_rsv : 6;
    dmac_set_rate_stru st_min_rate; /* Update join req 需要配置的速率集参数 */
    mac_multi_bssid_info st_mbssid_info;
    mac_ap_type_enum_uint16 en_ap_type;
    mac_sap_mode_enum_uint8 sap_mode;
    uint8_t ap_support_max_nss;
} dmac_ctx_join_req_set_reg_stru; /* hd_event */

/* wait join写寄存器参数的结构体定义 */
typedef struct {
    uint32_t dtim_period;              /* dtim period */
    uint32_t dtim_cnt;                 /* dtim count  */
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN]; /* 加入的AP的BSSID  */
    uint16_t us_tsf_bit0;                 /* tsf bit0  */
} dmac_ctx_set_dtim_tsf_reg_stru; /* hd_event */

/* wait join写寄存器参数的结构体定义 */
typedef struct {
    uint16_t uc_user_index; /* user index */
    uint8_t auc_resv[NUM_2_BYTES];
} dmac_ctx_asoc_set_reg_stru; /* hd_event */

/****************************************************************************************/
/* dot11QAPEDCATable OBJECT-TYPE                                                        */
/* SYNTAX SEQUENCE OF Dot11QAPEDCAEntry                                                 */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Conceptual table for EDCA default parameter values at the AP. This table       */
/*      contains the four entries of the EDCA parameters corresponding to four          */
/*      possible ACs. Index 1 corresponds to AC_BK, index 2 to AC_BE, index 3 to        */
/*      AC_VI, and index 4 to AC_VO."                                                   */
/*      REFERENCE                                                                       */
/*      "IEEE 802.11-<year>, 9.19.2"                                                    */
/* ::= { dot11mac 5 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11QAPEDCATableIndex;              /* dot11QAPEDCATableIndex    Unsigned32 */
    uint32_t dot11QAPEDCATableCWmin;              /* dot11QAPEDCATableCWmin Unsigned32 */
    uint32_t dot11QAPEDCATableCWmax;              /* dot11QAPEDCATableCWmax Unsigned32 */
    uint32_t dot11QAPEDCATableAIFSN;              /* dot11QAPEDCATableAIFSN Unsigned32 */
    uint32_t dot11QAPEDCATableTXOPLimit;          /* dot11QAPEDCATableTXOPLimit Unsigned32 */
    uint32_t dot11QAPEDCATableMSDULifetime;       /* dot11QAPEDCATableMSDULifetime Unsigned32 */
    oal_bool_enum_uint8 en_dot11QAPEDCATableMandatory; /* dot11QAPEDCATableMandatory TruthValue */
} wlan_mib_Dot11QAPEDCAEntry_stru; /* 注意：该结构为host/device抛事件使用,host/device需要采用相同结构体 */

/* sta更新edca参数寄存器的结构体定义 */
typedef struct {
    uint8_t uc_vap_id;
    mac_wmm_set_param_type_enum_uint8 en_set_param_type;
    uint16_t us_user_index;
    wlan_mib_Dot11QAPEDCAEntry_stru ast_wlan_mib_qap_edac[WLAN_WME_AC_BUTT];
} dmac_ctx_sta_asoc_set_edca_reg_stru; /* hd_event */
/* sta更新edca参数寄存器的结构体定义 */
/* sta更新sr参数寄存器的结构体定义 */
#ifdef _PRE_WLAN_FEATURE_11AX
typedef struct {
    mac_frame_he_spatial_reuse_parameter_set_ie_stru st_sr_ie;
} dmac_ctx_sta_asoc_set_sr_reg_stru; /* hd_event */
#endif

typedef struct {
    uint8_t query_event;
    uint8_t auc_query_sta_addr[WLAN_MAC_ADDR_LEN];
} dmac_query_request_event; /* host only */

typedef struct {
    uint8_t query_event;
    uint8_t auc_query_sta_addr[WLAN_MAC_ADDR_LEN]; /* sta mac地址 */
} dmac_query_station_info_request_event; /* hd_event */

/* TID事件用于更新指针和调度信息 */
typedef struct {
    tid_cmd_enum_uint8 cmd;        /* 0-creat tid; 1-del tid; 2-tid enqueue; 3-tid dequeue; */
    uint16_t user_id;
    uint8_t update_device_ring;
    uint8_t tid_no;                /* TID属性，no大于64时，说明是建立了ba会话 */
    uint32_t cur_msdus;            /* TID状态，队列中的msdu个数 */
    uint32_t cur_bytes;            /* TID状态，队列中的字节数 */
    msdu_info_ring_stru msdu_ring; /* TID属性，msdu_info_ring参数 */
    uint32_t timestamp_us;         /* MSG属性，此MSG生成的时间搓 */
} tid_update_stru; /* hd_event */

typedef enum {
    DMAC_CONFIG_IPV4 = 0, /* 配置IPv4地址 */
    DMAC_CONFIG_IPV6,     /* 配置IPv6地址 */
    DMAC_CONFIG_BUTT
} dmac_ip_type;
typedef uint8_t dmac_ip_type_enum_uint8;

typedef enum {
    DMAC_IP_ADDR_ADD = 0, /* 增加IP地址 */
    DMAC_IP_ADDR_DEL,     /* 删除IP地址 */
    DMAC_IP_OPER_BUTT
} dmac_ip_oper;
typedef uint8_t dmac_ip_oper_enum_uint8;

typedef struct {
    dmac_ip_type_enum_uint8 en_type;
    dmac_ip_oper_enum_uint8 en_oper;

    uint8_t auc_resv[NUM_2_BYTES];

    uint8_t auc_ip_addr[OAL_IP_ADDR_MAX_SIZE];
    uint8_t auc_mask_addr[OAL_IP_ADDR_MAX_SIZE];
} dmac_ip_addr_config_stru; /* hd_event */

typedef struct {
    mac_cfg_cali_hdr_stru cali_cfg_hdr;

    char input_args[WLAN_CALI_CFG_MAX_ARGS_NUM][WLAN_CALI_CFG_CMD_MAX_LEN];
    uint8_t input_args_num;
    uint8_t resv1[NUM_3_BYTES]; /* 4 字节对齐 */

    int32_t al_output_param[WLAN_CFG_MAX_ARGS_NUM];
    uint8_t uc_output_param_num;
    uint8_t resv2[NUM_3_BYTES]; /* 4 字节对齐 */
} mac_cfg_cali_param_stru; /* hd_event */

#ifdef _PRE_WLAN_FEATURE_NAN
typedef enum {
    NAN_CFG_TYPE_SET_PARAM = 0,
    NAN_CFG_TYPE_SET_TX_MGMT,

    NAN_CFG_TYPE_BUTT
} mac_nan_cfg_type_enum;
typedef uint8_t mac_nan_cfg_type_enum_uint8;

typedef enum {
    NAN_RSP_TYPE_SET_PARAM = 0,
    NAN_RSP_TYPE_SET_TX_MGMT,
    NAN_RSP_TYPE_CANCEL_TX,
    NAN_RSP_TYPE_ERROR,

    NAN_RSP_TYPE_BUTT
} dmac_nan_rsp_type_enum;
typedef uint8_t dmac_nan_rsp_type_enum_uint8;

typedef struct {
    uint32_t duration;
    uint32_t period;
    uint8_t type;
    uint8_t band;
    uint8_t channel;
} mac_nan_param_stru; /* hd_event */

typedef struct {
    uint8_t action;
    uint8_t periodic;
    uint16_t len;
    uint8_t *data;
} mac_nan_mgmt_info_stru; /* hd_event */

typedef struct {
    mac_nan_cfg_type_enum_uint8 type;
    uint16_t transaction_id;
    mac_nan_param_stru param;
    mac_nan_mgmt_info_stru mgmt_info;
} mac_nan_cfg_msg_stru; /* hd_event */

typedef struct {
    uint16_t transaction;
    uint8_t type;
    uint8_t status;
    uint8_t action; /* nan action类型 */
} mac_nan_rsp_msg_stru; /* hd_event */
#endif /* _PRE_WLAN_FEATURE_NAN */
/* end:from mac_common.h */
/* begin:from mac_device_common.h */
/* 扫描结果 */
typedef struct {
    mac_scan_status_enum_uint8 en_scan_rsp_status;
    uint8_t auc_resv[NUM_3_BYTES];
    uint64_t ull_cookie;
} mac_scan_rsp_stru; /* hd_event */

/* dmac将bt状态同步到hmac,用于hmac在共存场景做处理 */
typedef struct {
    uint16_t bt_reg0;
    uint16_t bt_reg1;
    uint16_t bt_reg2;
    uint16_t bt_reg3;
} mac_btcoex_bt_state_stru; /* hd_event */

typedef struct {
    uint8_t uc_cfg_btcoex_mode; /* 0:参数查询模式; 1:参数配置模式 */
    uint8_t uc_cfg_btcoex_type; /* 0:门限类型; 1:聚合大小类型 2.rssi detect门限参数配置模式 */
    union {
        struct {
            wlan_nss_enum_uint8 en_btcoex_nss; /* 0:siso 1:mimo */
            uint8_t uc_20m_low;              /* 2G 20M low */
            uint8_t uc_20m_high;             /* 2G 20M high */
            uint8_t uc_40m_low;              /* 2G 40M low */
            uint16_t us_40m_high;            /* 2G 40M high */
        } threhold;
        struct {
            wlan_nss_enum_uint8 en_btcoex_nss; /* 0:siso 1:mimo */
            uint8_t uc_grade;                /* 0级或者1级 */
            uint8_t uc_rx_size0;             /* size0大小 */
            uint8_t uc_rx_size1;             /* size1大小 */
            uint8_t uc_rx_size2;             /* size2大小 */
            uint8_t uc_rx_size3;             /* size3大小 */
        } rx_size;
        struct {
            oal_bool_enum_uint8 en_rssi_limit_on;
            oal_bool_enum_uint8 en_rssi_log_on;
            uint8_t uc_cfg_rssi_detect_cnt; /* 6 rssi 配置时用于防护次数  高低门限配置 */
            int8_t c_cfg_rssi_detect_mcm_down_th; /* m2s */
            int8_t c_cfg_rssi_detect_mcm_up_th; /* s2m */
        } rssi_param;
    } pri_data;
} mac_btcoex_mgr_stru; /* hd_event */

typedef struct {
    /* 0:硬件preempt; 1:软件preempt 2:ps 提前slot量 */
    uint8_t uc_cfg_preempt_mode;
    /* 0 noframe 1 self-cts 2 nulldata 3 qosnull  0/1 软件ps打开或者关闭 */
    uint8_t uc_cfg_preempt_type;
} mac_btcoex_preempt_mgr_stru; /* hd_event */

typedef struct {
    wlan_m2s_mgr_vap_stru ast_m2s_blacklist[WLAN_M2S_BLACKLIST_MAX_NUM];
    uint8_t uc_blacklist_cnt;
} mac_m2s_ie_stru; /* hd_event */

/* m2s device信息结构体 */
typedef struct {
    uint8_t uc_device_id;                           /* 业务vap id */
    wlan_nss_enum_uint8 en_nss_num;                   /* device的接收空间流个数 */
    /* mac device的smps能力，用于切换后vap能力初始化 */
    uint8_t en_smps_mode;   /* wlan_mib_mimo_power_save_enum */
    uint8_t auc_reserved[1];
} mac_device_m2s_stru; /* hd_event */

typedef enum {
    MAC_PM_DEBUG_SISO_RECV_BCN = 0,
    MAC_PM_DEBUG_DYN_TBTT_OFFSET = 1,
    MAC_PM_DEBUG_NO_PS_FRM_INT = 2,
    MAC_PM_DEBUG_APF = 3,
    MAC_PM_DEBUG_AO = 4,

    MAC_PM_DEBUG_CFG_BUTT
} mac_pm_debug_cfg_enum_uint8; /* hd_event */

typedef struct {
    uint32_t cmd_bit_map;
    uint8_t uc_srb_switch; /* siso收beacon开关 */
    uint8_t uc_dto_switch; /* 动态tbtt offset开关 */
    uint8_t uc_nfi_switch;
    uint8_t uc_apf_switch;
    uint8_t uc_ao_switch;
} mac_pm_debug_cfg_stru; /* hd_event */

typedef enum {
    MAC_PSM_QUERY_FLT_STAT = 0,
    MAC_PSM_QUERY_FASTSLEEP_STAT = 1,
#ifdef _PRE_WLAN_FEATURE_PSM_DFX_EXT
    MAC_PSM_QUERY_BEACON_CNT = 2,
#endif

#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
    MAC_PSM_QUERY_RX_LISTEN_STATE = 3,
#endif
#ifdef _PRE_WLAN_FEATURE_PSM_ABN_PKTS_STAT
    MAC_PSM_QUERY_ABNORMAL_PKTS_CNT = 4,
#endif

    MAC_PSM_QUERY_TYPE_BUTT
} mac_psm_query_type_enum;
typedef uint8_t mac_psm_query_type_enum_uint8;
#define MAC_PSM_QUERY_MSG_MAX_STAT_ITEM  10
typedef struct {
    uint32_t                          query_item;
    uint32_t                          aul_val[MAC_PSM_QUERY_MSG_MAX_STAT_ITEM];
}mac_psm_query_stat_stru;

typedef struct {
    mac_psm_query_type_enum_uint8 en_query_type;
    mac_psm_query_stat_stru st_stat;
}mac_psm_query_msg; /* hd_event */


typedef enum {
    MAC_STA_PM_SWITCH_OFF = 0,      /* 关闭低功耗 */
    MAC_STA_PM_SWITCH_ON = 1,       /* 打开低功耗 */
    MAC_STA_PM_MANUAL_MODE_ON = 2,  /* 开启手动sta pm mode */
    MAC_STA_PM_MANUAL_MODE_OFF = 3, /* 关闭手动sta pm mode */
    MAC_STA_PM_SWITCH_BUTT,         /* 最大类型 */
} mac_pm_switch_enum;
typedef uint8_t mac_pm_switch_enum_uint8; /* hd_event */

typedef enum {
    MAC_STA_PM_CTRL_TYPE_HOST = 0, /* 低功耗控制类型 HOST  */
    MAC_STA_PM_CTRL_TYPE_DBAC = 1, /* 低功耗控制类型 DBAC  */
    MAC_STA_PM_CTRL_TYPE_ROAM = 2, /* 低功耗控制类型 ROAM  */
    MAC_STA_PM_CTRL_TYPE_CMD = 3,  /* 低功耗控制类型 CMD   */
    MAC_STA_PM_CTRL_TYPE_ERSRU = 4,  /* 低功耗控制类型 ERSRU     */
    MAC_STA_PM_CTRL_TYPE_CSI = 5,  /* 低功耗控制类型 CSI */
    MAC_STA_PM_CTRL_TYPE_BUTT,     /* 最大类型，应小于 8  */
} mac_pm_ctrl_type_enum;
typedef uint8_t mac_pm_ctrl_type_enum_uint8; /* hd_event */

typedef struct {
    mac_pm_ctrl_type_enum_uint8 uc_pm_ctrl_type; /* mac_pm_ctrl_type_enum */
    mac_pm_switch_enum_uint8 uc_pm_enable;       /* mac_pm_switch_enum */
} mac_cfg_ps_open_stru; /* hd_event */

#pragma pack(push, 1)
/* 上报的扫描结果的扩展信息，放在上报host侧的管理帧netbuf的后面 */
typedef struct {
    int32_t l_rssi;                                /* 信号强度 */
    uint8_t en_bss_type; /* wlan_mib_desired_bsstype_enum 扫描到的bss类型 */
    uint8_t auc_resv[BYTE_OFFSET_2]; /* 预留字段 */

    int8_t rssi[HD_EVENT_RF_NUM]; /* 4天线的rssi */
    int8_t snr[HD_EVENT_RF_NUM];  /* 4天线的snr */

    wlan_nss_enum_uint8 en_support_max_nss; /* 该AP支持的最大空间流数 */
#ifdef _PRE_WLAN_FEATURE_M2S
    oal_bool_enum_uint8 en_support_opmode;  /* 该AP是否支持OPMODE */
    uint8_t uc_num_sounding_dim;          /* 该AP发送txbf的天线数 */
#endif
} mac_scanned_result_extend_info_stru; /* hd_event */
#pragma pack(pop)

/* 带宽调试开关相关的结构体 */
typedef enum {
    MAC_CSA_FLAG_NORMAL = 0,
    MAC_CSA_FLAG_START_DEBUG, /* 固定csa ie 在beacon帧中 */
    MAC_CSA_FLAG_CANCLE_DEBUG,
    MAC_CSA_FLAG_GO_DEBUG,

    MAC_CSA_FLAG_BUTT
} mac_csa_flag_enum;
typedef uint8_t mac_csa_flag_enum_uint8;

typedef struct {
    wlan_csa_mode_tx_enum_uint8 en_mode;
    uint8_t uc_channel;
    uint8_t uc_cnt;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth;
    mac_csa_flag_enum_uint8 en_debug_flag; /* 0:正常切信道; 1:仅beacon帧中含有csa,信道不切换;2:取消beacon帧中含有csa */
    uint8_t auc_reserv[NUM_3_BYTES];
} mac_csa_debug_stru;

typedef struct {
    uint32_t cmd_bit_map;
    oal_bool_enum_uint8 en_band_force_switch_bit0; /* 恢复40M带宽命令 */
    oal_bool_enum_uint8 en_2040_ch_swt_prohi_bit1; /* 不允许20/40带宽切换开关 */
    oal_bool_enum_uint8 en_40_intolerant_bit2;     /* 不容忍40M带宽开关 */
    uint8_t uc_resv;
    mac_csa_debug_stru st_csa_debug_bit3;
    oal_bool_enum_uint8 en_lsigtxop_bit5; /* lsigtxop使能 */
    uint8_t auc_resv0[NUM_3_BYTES];
} mac_protocol_debug_switch_stru; /* hd_event */

/* 配置命令格式 */
typedef struct {
    uint8_t uc_item_count;
    oal_bool_enum_uint8 en_enable; /* 下发功能使能标志 */
    mac_ip_filter_cmd_enum_uint8 en_cmd;
    uint8_t uc_resv;
    mac_ip_filter_item_stru ast_filter_items[1];
} mac_ip_filter_cmd_stru; /* hd_event */

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
typedef struct {
    oal_bool_enum_uint8 en_is_pk_mode;
    wlan_bw_cap_enum_uint8 en_curr_bw_cap;             /* 目前使用的带宽，本参数仅在lagency sta模式下生效 */
    wlan_protocol_cap_enum_uint8 en_curr_protocol_cap; /* 目前使用的协议模式，本参数仅在lagency sta模式下生效 */
    wlan_nss_enum_uint8 en_curr_num_spatial_stream;    /* 目前单双流的计数 */

    uint32_t tx_bytes; /* WIFI 业务发送帧统计 */
    uint32_t rx_bytes; /* WIFI 业务接收帧统计 */
    uint32_t dur_time; /* 统计时间间隔 */
} mac_cfg_pk_mode_stru; /* hd_event */
#endif
/* end:from mac_device_common.h */
/* begin:from mac_user_common.h */
/* m2s user信息结构体 */
typedef struct {
    uint16_t              us_user_idx;
    wlan_nss_enum_uint8     en_user_max_cap_nss;                    /* 用户自身空间流能力 */
    wlan_nss_enum_uint8     en_avail_num_spatial_stream;            /* Tx和Rx支持Nss的交集,供算法调用 */

    wlan_nss_enum_uint8     en_avail_bf_num_spatial_stream;         /* 用户支持的Beamforming空间流个数 */
    wlan_bw_cap_enum_uint8  en_avail_bandwidth;                     /* 用户和VAP带宽能力交集,供算法调用 */
    wlan_bw_cap_enum_uint8  en_cur_bandwidth;                       /* 默认值与en_avail_bandwidth相同,供算法调用修改 */
    uint8_t en_cur_smps_mode;     /* wlan_mib_mimo_power_save_enum */
    wlan_nss_enum_uint8     en_smps_opmode_nodify_nss;              /* opmode 或者smps 通知修改的nss值 */
    uint8_t auc_rsv[3];                                             // 3代表保留字节， 结构体4字节对齐
}mac_user_m2s_stru; /* hd_event */

typedef struct {
    mac_channel_stru st_channel;
    uint16_t us_user_idx;
    wlan_bw_cap_enum_uint8 en_bandwidth_cap;   /* 用户带宽能力信息 */
    wlan_bw_cap_enum_uint8 en_avail_bandwidth; /* 用户和VAP带宽能力交集,供算法调用 */
    wlan_bw_cap_enum_uint8 en_cur_bandwidth;   /* 默认值与en_avail_bandwidth相同,供算法调用修改 */
    uint8_t auc_rsv[3];                        // 3代表保留字节， 4字节对齐
} mac_d2h_syn_info_stru; /* hd_event */

typedef struct {
    uint8_t uc_qos_info;   /* 关联请求中的WMM IE的QOS info field */
    uint8_t uc_max_sp_len; /* 从qos info字段中提取翻译的最大服务长度 */
    uint8_t auc_resv[2]; // 2代表保留字节， 4字节对齐
    uint8_t uc_ac_trigger_ena[WLAN_WME_AC_BUTT]; /* 4个AC的trigger能力 */
    uint8_t uc_ac_delievy_ena[WLAN_WME_AC_BUTT]; /* 4个AC的delivery能力 */
} mac_user_uapsd_status_stru; /* hd_event */

#pragma pack(push, 1)
typedef struct {
    uint16_t user_id;
    uint8_t uapsd_flag;
    mac_user_uapsd_status_stru mac_user_uapsd_status;
} h2d_event_user_uapsd_info; /* AP同步uapsd参数，03/05 rom化，需要按字节对齐 */
#pragma pack(pop)

typedef struct {
    uint16_t us_user_idx;
    uint8_t uc_arg1;
    uint8_t uc_arg2;

    /* 协议模式信息 */
    wlan_protocol_enum_uint8 en_cur_protocol_mode;
    wlan_protocol_enum_uint8 en_protocol_mode;
    uint8_t en_avail_protocol_mode; /* 用户和VAP协议模式交集, 供算法调用 */

    wlan_bw_cap_enum_uint8 en_bandwidth_cap;   /* 用户带宽能力信息 */
    wlan_bw_cap_enum_uint8 en_avail_bandwidth; /* 用户和VAP带宽能力交集,供算法调用 */
    wlan_bw_cap_enum_uint8 en_cur_bandwidth;   /* 默认值与en_avail_bandwidth相同,供算法调用修改 */

    oal_bool_enum_uint8 en_user_pmf;
    mac_user_asoc_state_enum_uint8 en_user_asoc_state; /* 用户关联状态 */
#ifdef _PRE_WLAN_FEATURE_11AX
    uint8_t   bit_in_htc_blacklist   : 1, /* 当前user是否在htc blacklist表里 */
              bit_rev                : 7;
    uint8_t   rsv[3]; /* 3为数组大小 */
#endif
} mac_h2d_usr_info_stru; /* hd_event */

typedef struct {
    mac_user_cap_info_stru st_user_cap_info; /* 用户能力信息 */
    uint16_t us_user_idx;
    uint16_t amsdu_maxsize; /* amsdu的最大字节长度 */
#ifdef _PRE_WLAN_FEATURE_HIEX
    mac_hiex_cap_stru st_hiex_cap;
#endif
} mac_h2d_usr_cap_stru; /* hd_event */


typedef struct {
    uint16_t us_user_idx;

#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
    mac_he_hdl_stru st_he_hdl;
#endif
    /* vht速率集信息 */
    mac_vht_hdl_stru st_vht_hdl;

    /* ht速率集信息 */
    mac_user_ht_hdl_stru st_ht_hdl;

    /* legacy速率集信息 */
    uint8_t uc_avail_rs_nrates;
    uint8_t auc_avail_rs_rates[WLAN_MAX_SUPP_RATES];

    wlan_protocol_enum_uint8 en_protocol_mode; /* 用户协议模式 */
} mac_h2d_usr_rate_info_stru; /* hd_event */

typedef struct {
    uint16_t us_user_idx;
    mac_user_asoc_state_enum_uint8 en_asoc_state;
    uint8_t uc_rsv[1];
} mac_h2d_user_asoc_state_stru; /* hd_event */

/* 空间流信息结构体 */
typedef struct {
    uint16_t            us_user_idx;
    wlan_nss_enum_uint8   en_avail_num_spatial_stream;            /* Tx和Rx支持Nss的交集,供算法调用 */
    wlan_nss_enum_uint8   en_user_max_cap_nss;                    /* 用户支持的空间流个数 */
    wlan_nss_enum_uint8   en_user_num_spatial_stream_160M;        /* 用户带宽为160M时支持的空间流个数 */
    wlan_nss_enum_uint8   en_smps_opmode_notify_nss;              /* opmode 或者smps 通知修改的nss值  */
    uint8_t             auc_reserv[2];                            // 2代表保留字节， 4字节对齐
} mac_user_nss_stru; /* hd_event */
/* end:from mac_user_common.h */
/* begin:from mac_vap_common.h */
/* MAC vap能力特性标识 */
typedef struct {
    uint32_t bit_uapsd : 1,
               bit_txop_ps : 1,
               bit_wpa : 1,
               bit_wpa2 : 1,
               bit_dsss_cck_mode_40mhz : 1, /* 是否允许在40M上使用DSSS/CCK, 1-允许, 0-不允许 */
               bit_rifs_tx_on : 1,
               bit_tdls_prohibited : 1,                /* tdls全局禁用开关， 0-不关闭, 1-关闭 */
               bit_tdls_channel_switch_prohibited : 1, /* tdls信道切换全局禁用开关， 0-不关闭, 1-关闭 */
               bit_hide_ssid : 1,                      /* AP开启隐藏ssid,  0-关闭, 1-开启 */
               bit_wps : 1,                            /* AP WPS功能:0-关闭, 1-开启 */
               bit_11ac2g : 1,                         /* 2.4G下的11ac:0-关闭, 1-开启 */
               bit_keepalive : 1,                      /* vap KeepAlive功能开关: 0-关闭, 1-开启 */
               bit_smps : 2,                           /* vap 当前SMPS能力(暂时不使用) */
               bit_dpd_enbale : 1,                     /* dpd是否开启 */
               bit_dpd_done : 1,                       /* dpd是否完成 */
               bit_11ntxbf : 1,                        /* 11n txbf能力 */
               bit_disable_2ght40 : 1,                 /* 2ght40禁止位，1-禁止，0-不禁止 */
               bit_peer_obss_scan : 1,                 /* 对端支持obss scan能力: 0-不支持, 1-支持 */
               bit_1024qam : 1,                        /* 对端支持1024qam能力: 0-不支持, 1-支持 */ /* 1106不使用 */
               bit_wmm_cap : 1,                        /* 保存与STA关联的AP是否支持wmm能力信息 */
               bit_is_interworking : 1,                /* 保存与STA关联的AP是否支持interworking能力 */
               bit_ip_filter : 1,                      /* rx方向ip包过滤的功能 */
               bit_opmode : 1,                         /* 对端的probe rsp中的extended capbilities 是否支持OPMODE */
               bit_nb : 1,                             /* 硬件是否支持窄带 */
               bit_2040_autoswitch : 1,                /* 是否支持随环境自动2040带宽切换 */
               bit_2g_custom_siso : 1,                 /* 2g是否定制化单天线siso,默认等于0,初始双天线 */
               bit_5g_custom_siso : 1,                 /* 5g是否定制化单天线siso,默认等于0,初始双天线 */
               bit_bt20dbm : 1,                        /* 20dbm是否使能，用于host做sta做删聚合判断 */
               bit_icmp_filter : 1,                    /* rx方向icmp的过滤filter */
               bit_dcm : 1,                            /* 11ax dcm能力 */
               bit_rx_stbc : 1;                        /* 关联的bss 支持stbc */
} mac_cap_flag_stru; /* hd_event */

typedef struct {
    oal_bool_enum_uint8 en_11ax_cap; /* 是否支持11ax能力 */
    oal_bool_enum_uint8 en_radar_detector_cap;
    oal_bool_enum_uint8 en_11n_sounding;
    wlan_bw_cap_enum_uint8 en_wlan_bw_max;

    uint16_t us_beacon_period;
    oal_bool_enum_uint8 en_green_field;
    oal_bool_enum_uint8 en_mu_beamformee_cap;

    oal_bool_enum_uint8 en_txopps_is_supp;
    oal_bool_enum_uint8 uc_su_bfee_num;
    oal_bool_enum_uint8 en_40m_shortgi;
    oal_bool_enum_uint8 en_11n_txbf;

    oal_bool_enum_uint8 en_40m_enable;
    uint8_t uc_bfer_num_sounding_dim;
    oal_bool_enum_uint8 en_su_bfee;
#ifdef _PRE_WLAN_FEATURE_11AX
    uint8_t uc_he_ntx_sts_below_80m;
    uint8_t uc_he_ntx_sts_over_80m;
    uint8_t uc_he_num_dim_below_80m;
    uint8_t uc_he_num_dim_over_80m;
#endif
    uint8_t uc_rsv[1];
} mac_d2h_mib_update_info_stru; /* hd_event */

/* 用于同步保护相关的参数 */
typedef struct {
    uint8_t en_dot11HTProtection; /* wlan_mib_ht_protection_enum */
    oal_bool_enum_uint8 en_dot11RIFSMode;
    oal_bool_enum_uint8 en_dot11LSIGTXOPFullProtectionActivated;
    oal_bool_enum_uint8 en_dot11NonGFEntitiesPresent;

    mac_protection_stru st_protection;
} mac_h2d_protection_stru; /* hd_event */

/* HOSTAPD 设置工作频段，信道和带宽参数 */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV)
typedef struct {
    wlan_channel_band_enum_uint8 en_band;           /* 频带 */
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽 */
    uint8_t uc_channel;                           /* 信道编号 */
    uint8_t auc_resv[1];                          /* 保留位 */
} mac_cfg_channel_param_stru; /* hd_event */
#else
typedef struct {
    uint8_t en_band : 4,
            ext6g_band : 1, /* 指示是否为6G频段标志 */
            resv : 3;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽 */
    uint8_t uc_channel;                           /* 信道编号 */
    uint8_t auc_resv[1];                          /* 保留位 */
} mac_cfg_channel_param_stru; /* hd_event */
#endif
typedef struct {
    uint32_t length;
    uint32_t aul_dieid[WLAN_DIEID_MAX_LEN];
} mac_cfg_show_dieid_stru; /* hd_event */

typedef struct {
    uint16_t us_user_id;
    int8_t c_rssi;
    int8_t c_free_power;
} mac_cfg_query_rssi_stru; /* hd_event */

typedef struct {
    uint8_t uc_auth_req_st;
    uint8_t uc_asoc_req_st;
    uint8_t auc_resv[NUM_2_BYTES];
} mac_cfg_query_mngpkt_sendstat_stru; /* hd_event */

typedef struct {
    uint16_t us_user_id;
    uint8_t uc_ps_st;
    uint8_t auc_resv[1];
} mac_cfg_query_psst_stru; /* hd_event */

#ifdef _PRE_WLAN_DFT_STAT
typedef struct {
    uint8_t uc_device_distance;
    uint8_t uc_intf_state_cca;
    uint8_t uc_intf_state_co;
    uint8_t auc_resv[1];
} mac_cfg_query_ani_stru; /* hd_event */
#endif

typedef struct {
    uint16_t us_user_id;
#ifdef _PRE_WLAN_DFT_STAT
    uint8_t uc_cur_per;
    uint8_t uc_bestrate_per;
#else
    uint8_t auc_resv[NUM_2_BYTES];
#endif
    uint32_t tx_rate;     /* 当前发送速率 */
    uint32_t tx_rate_min; /* 一段时间内最小发送速率 */
    uint32_t tx_rate_max; /* 一段时间内最大发送速率 */
    uint32_t rx_rate;     /* 当前接收速率 */
    uint32_t rx_rate_min; /* 一段时间内最小接收速率 */
    uint32_t rx_rate_max; /* 一段时间内最大接收速率 */
} mac_cfg_query_rate_stru; /* hd_event */

/* m2s vap信息结构体 */
typedef struct {
    uint8_t uc_vap_id;                  /* 业务vap id */
    wlan_nss_enum_uint8 en_vap_rx_nss;    /* vap的接收空间流个数 */
    wlan_m2s_type_enum_uint8 en_m2s_type; /* 0:软切换 1:硬切换 */
    uint8_t en_sm_power_save; /* wlan_mib_mimo_power_save_enum */

    oal_bool_enum_uint8 en_tx_stbc;
    oal_bool_enum_uint8 en_transmit_stagger_sounding;
    oal_bool_enum_uint8 en_vht_ctrl_field_supported;
    oal_bool_enum_uint8 en_vht_number_sounding_dimensions;

    oal_bool_enum_uint8 en_vht_su_beamformer_optionimplemented;
    oal_bool_enum_uint8 en_tx_vht_stbc_optionimplemented;
    oal_bool_enum_uint8 en_support_opmode;
    uint8_t uc_vht_ntx_sts;
    oal_bool_enum_uint8 en_su_bfee;
    uint8_t auc_reserved1[NUM_3_BYTES];
#ifdef _PRE_WLAN_FEATURE_11AX
    oal_bool_enum_uint8 en_he_su_bfer;
    uint8_t uc_he_num_dim_below_80m;
    uint8_t uc_he_num_dim_over_80m;
    uint8_t uc_he_ntx_sts_below_80m;
    uint8_t uc_he_ntx_sts_over_80m;
    uint8_t auc_reserved2[NUM_3_BYTES];
#endif
} mac_vap_m2s_stru; /* hd_event */

typedef struct {
    uint8_t new_rx_mode;
    uint8_t hal_device_id;
}mac_rx_switch_stru; /* hd_event */

/* 创建VAP参数结构体, 对应cfgid: WLAN_CFGID_ADD_VAP */
typedef struct {
    wlan_vap_mode_enum_uint8 en_vap_mode;
    uint8_t uc_cfg_vap_indx;
    uint16_t us_muti_user_id; /* 添加vap 对应的muti user index */

    uint8_t uc_vap_id;                  /* 需要添加的vap id */
    wlan_p2p_mode_enum_uint8 en_p2p_mode; /* 0:非P2P设备; 1:P2P_GO; 2:P2P_Device; 3:P2P_CL */

    uint8_t bit_11ac2g_enable : 1;
    uint8_t bit_disable_capab_2ght40 : 1;
    uint8_t bit_reserve : 6;
    uint8_t uc_dst_hal_dev_id;  // vap挂接的hal device id

    uint8_t bit_uapsd_enable : 1;
    uint8_t bit_reserve1 : 7;
    uint8_t probe_resp_enable;
    uint8_t probe_resp_status;
    uint8_t is_primary_vap;
    uint8_t resv2[1];
    oal_net_device_stru *pst_net_dev;
} mac_cfg_add_vap_param_stru; /* hd_event */
typedef mac_cfg_add_vap_param_stru mac_cfg_del_vap_param_stru; /* hd_event */

/* 启用VAP参数结构体 对应cfgid: WLAN_CFGID_START_VAP */
typedef struct {
    oal_bool_enum_uint8 en_mgmt_rate_init_flag; /* start vap时候，管理帧速率是否需要初始化 */
    uint8_t uc_protocol;
    uint8_t uc_band;
    uint8_t uc_bandwidth;
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    uint8_t auc_resv2[NUM_3_BYTES];
    int32_t l_ifindex;
    oal_net_device_stru *pst_net_dev; /* 此成员仅供Host(WAL&HMAC)使用，Device侧(DMAC&ALG&HAL层)不使用 */
} mac_cfg_start_vap_param_stru; /* hd_event */
typedef mac_cfg_start_vap_param_stru mac_cfg_down_vap_param_stru; /* hd_event */

typedef struct {
    uint16_t us_sta_aid;
    uint8_t uc_uapsd_cap;
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    oal_bool_enum_uint8 en_txop_ps;
#else
    uint8_t auc_resv[1];
#endif /* #ifdef _PRE_WLAN_FEATURE_TXOPPS */
    uint8_t bit_ap_chip_oui : 3,
              bit_resv : 5;
    uint8_t auc_resv1[NUM_3_BYTES];
} mac_h2d_vap_info_stru; /* hd_event */

/* 用于同步带宽切换的参数 */
typedef struct {
    wlan_channel_bandwidth_enum_uint8 en_40M_bandwidth;
    oal_bool_enum_uint8 en_40M_intol_user;
    uint8_t auc_resv[NUM_2_BYTES];
} mac_bandwidth_stru; /* hd_event */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV)
/* 协议参数 对应cfgid: WLAN_CFGID_MODE */
typedef struct {
    wlan_protocol_enum_uint8 en_protocol;           /* 协议 */
    wlan_channel_band_enum_uint8 en_band;           /* 频带 */
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽 */
    uint8_t en_channel_idx;                       /* 主20M信道号 */
} mac_cfg_mode_param_stru; /* hd_event */
#else
typedef struct {
    wlan_protocol_enum_uint8 en_protocol;           /* 协议 */
    uint8_t en_band : 4,                            /* 频带 */
            ext6g_band : 1,                         /* 指示是否为6G频段标志 */
            resv : 3;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽 */
    uint8_t en_channel_idx;                       /* 主20M信道号 */
} mac_cfg_mode_param_stru; /* hd_event */
#endif
typedef struct {
    wlan_channel_band_enum_uint8 en_band;           /* 频带 */
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽 */
} mac_cfg_mib_by_bw_param_stru; /* hd_event */

/* 设置mac地址参数 对应cfgid: WLAN_CFGID_STATION_ID */
typedef struct {
    uint8_t auc_station_id[WLAN_MAC_ADDR_LEN];
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    uint8_t auc_resv[1];
} mac_cfg_staion_id_param_stru; /* hd_event */

/* SSID参数 对应cfgid: WLAN_CFGID_SSID */
typedef struct {
    uint8_t uc_ssid_len;
    uint8_t auc_resv[NUM_2_BYTES];
    int8_t ac_ssid[WLAN_SSID_MAX_LEN];
} mac_cfg_ssid_param_stru; /* hd_event */

/* HOSTAPD 设置 Beacon 信息 */
typedef struct {
    int32_t l_interval;    /* beacon interval */
    int32_t l_dtim_period; /* DTIM period     */
    oal_bool_enum_uint8 en_privacy;
    uint8_t auc_rsn_ie[MAC_MAX_RSN_LEN];
    uint8_t auc_wpa_ie[MAC_MAX_RSN_LEN];
    oal_bool_enum_uint8 uc_hidden_ssid;
    oal_bool_enum_uint8 en_shortgi_20;
    oal_bool_enum_uint8 en_shortgi_40;
    oal_bool_enum_uint8 en_shortgi_80;
    wlan_protocol_enum_uint8 en_protocol;

    uint8_t uc_smps_mode;
    mac_beacon_operation_type_uint8 en_operation_type;
    uint8_t auc_resv1[NUM_2_BYTES];
} mac_beacon_param_stru; /* hd_event */

/* 用户相关的配置命令参数 */
typedef struct {
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /* MAC地址 */
    oal_bool_enum_uint8 en_ht_cap;             /* ht能力 */
    uint8_t auc_resv[NUM_3_BYTES];
    uint16_t us_user_idx; /* 用户索引 */
} mac_cfg_add_user_param_stru; /* hd_event */
typedef mac_cfg_add_user_param_stru mac_cfg_del_user_param_stru; /* hd_event */

/* sniffer配置命令参数 */
typedef struct {
    uint8_t uc_sniffer_mode;                 /* sniffer模式 */
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /* 需要过滤的MAC地址 */
    uint8_t auc_resv[1];
} mac_cfg_sniffer_param_stru; /* hd_event */

/* 暂停tid配置命令参数 */
typedef struct {
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /* MAC地址 */
    uint8_t uc_tid;
    uint8_t uc_is_paused;
} mac_cfg_pause_tid_param_stru; /* hd_event */

/* 暂停tid配置命令参数 */
typedef struct {
    uint8_t uc_aggr_tx_on;
    uint8_t uc_snd_type;
    mac_ampdu_switch_mode_enum_uint8 en_aggr_switch_mode;
    uint8_t uc_rsv;
} mac_cfg_ampdu_tx_on_param_stru; /* hd_event */

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
typedef struct {
    mac_tcp_ack_buf_cfg_cmd_enum_uint8 en_cmd;
    oal_bool_enum_uint8 en_enable;
    uint8_t uc_timeout_ms;
    uint8_t uc_count_limit;
} mac_cfg_tcp_ack_buf_stru; /* hd_event */
#endif

#ifdef _PRE_WLAN_FEATURE_CSI
#define WLAN_CSI_BLACK_LIST_NUM 8
typedef struct {
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /* csi对应的MAC ADDR */
    oal_bool_enum_uint8 en_cfg_csi_on;         /* csi使能 */
    uint8_t uc_csi_bw;                       /* csi采集带宽 */
    uint8_t uc_csi_frame_type;               /* csi采集的帧类型 */
    uint8_t uc_csi_sample_period;            /* csi采样周期 */
    uint8_t uc_csi_phy_report_mode;          /* 配置CSI PHY的上报模式  */
    uint8_t auc_reserve[1];
} mac_cfg_csi_param_stru; /* hd_event */

typedef struct {
    uint8_t csi_black_list_num;
    uint8_t csi_black_list_mac_map[WLAN_CSI_BLACK_LIST_NUM][WLAN_MAC_ADDR_LEN];
} mac_csi_black_list_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_WMMAC
typedef struct {
    oal_switch_enum_uint8 en_wmm_ac_switch;
    oal_switch_enum_uint8 en_auth_flag; /* WMM AC认证开关 */
    uint16_t us_timeout_period;       /* 定时器超时时间 */
    uint8_t uc_factor;                /* 除数因子，medium_time/2^factor */
    uint8_t auc_rsv[NUM_3_BYTES];
} mac_cfg_wmm_ac_param_stru; /* hd_event */
#endif

/* 发送功率配置参数 */
typedef struct {
    mac_set_pow_type_enum_uint8 en_type;
    uint8_t uc_reserve;
    uint8_t auc_value[NUM_18_BYTES];
} mac_cfg_set_tx_pow_param_stru; /* hd_event */

/* 设置用户配置参数 */
typedef struct {
    uint8_t uc_function_index;
    uint8_t auc_reserve[NUM_2_BYTES];
    mac_vap_config_dscr_frame_type_uint8 en_type; /* 配置的帧类型 */
    int32_t l_value;
} mac_cfg_set_dscr_param_stru; /* hd_event */

/* non-HT协议模式下速率配置结构体 */
typedef struct {
    wlan_legacy_rate_value_enum_uint8 en_rate;     /* 速率值 */
    wlan_phy_protocol_enum_uint8 en_protocol_mode; /* 对应的协议 */
    uint8_t auc_reserve[NUM_2_BYTES];                      /* 保留 */
} mac_cfg_non_ht_rate_stru; /* hd_event */

/* 用户信息相关的配置命令参数 */
typedef struct {
    uint16_t us_user_idx; /* 用户索引 */
    uint8_t auc_reserve[NUM_2_BYTES];
} mac_cfg_user_info_param_stru; /* hd_event */

/* STA PS 发送参数 */
typedef struct {
    uint8_t uc_vap_ps_mode;
} mac_cfg_ps_mode_param_stru; /* hd_event */

typedef struct {
    uint16_t us_beacon_timeout;
    uint16_t us_tbtt_offset;
    uint16_t us_ext_tbtt_offset;
    uint16_t us_dtim3_on;
} mac_cfg_ps_param_stru; /* hd_event */

typedef struct {
    uint8_t uc_psm_info_enable : 2;  // 开启psm的统计维测输出
    uint8_t uc_psm_debug_mode : 2;   // 开启psm的debug打印模式
    uint8_t uc_psm_resd : 4;
} mac_cfg_ps_info_stru; /* hd_event */

typedef struct {
    uint8_t uc_show_ip_addr : 4;          // show ip addr
    uint8_t uc_show_arpoffload_info : 4;  // show arpoffload 维测
} mac_cfg_arpoffload_info_stru; /* hd_event */

typedef struct {
    uint8_t uc_in_suspend;         // 亮暗屏
    uint8_t uc_arpoffload_switch;  // arpoffload开关
} mac_cfg_suspend_stru; /* hd_event */


/* P2P NOA节能配置参数 */
typedef struct {
    uint32_t start_time;
    uint32_t duration;
    uint32_t interval;
    uint8_t uc_count;
    uint8_t auc_rsv[NUM_3_BYTES];
} mac_cfg_p2p_noa_param_stru; /* hd_event */

typedef struct {
    oal_bool_enum_uint8 en_open;    /* 打开关闭此特性 */
    mac_narrow_bw_enum_uint8 en_bw; /* 1M,5M,10M */
    uint8_t auc_rsv[NUM_2_BYTES];
} mac_cfg_narrow_bw_stru; /* hd_event */

#ifdef _PRE_WLAN_FEATURE_TXOPPS
/* STA txopps aid同步 */
typedef struct {
    uint16_t us_partial_aid;
    uint8_t en_protocol;
    uint8_t uc_enable;
} mac_cfg_txop_sta_stru; /* hd_event */

/* 软件配置mac txopps使能寄存器需要的三个参数 */
typedef struct {
    oal_switch_enum_uint8 en_machw_txopps_en;         /* sta是否使能txopps */
    oal_switch_enum_uint8 en_machw_txopps_condition1; /* txopps条件1 */
    oal_switch_enum_uint8 en_machw_txopps_condition2; /* txopps条件2 */
    uint8_t auc_resv[1];
} mac_txopps_machw_param_stru; /* hd_event */
#endif

typedef enum mac_vowifi_mkeep_alive_type {
    VOWIFI_MKEEP_ALIVE_TYPE_STOP = 0,
    VOWIFI_MKEEP_ALIVE_TYPE_START = 1,
    VOWIFI_MKEEP_ALIVE_TYPE_BUTT
} mac_vowifi_nat_keep_alive_type_enum;
typedef uint8_t mac_vowifi_nat_keep_alive_type_enum_uint8;

typedef struct {
    uint8_t uc_keep_alive_id;
    mac_vowifi_nat_keep_alive_type_enum_uint8 en_type;
    uint8_t auc_rsv[NUM_2_BYTES];
} mac_vowifi_nat_keep_alive_basic_info_stru; /* hd_event */

typedef struct {
    mac_vowifi_nat_keep_alive_basic_info_stru st_basic_info;
    uint8_t auc_src_mac[WLAN_MAC_ADDR_LEN];
    uint8_t auc_dst_mac[WLAN_MAC_ADDR_LEN];
    uint32_t period_msec;
    uint16_t us_ip_pkt_len;
    uint8_t auc_rsv[NUM_2_BYTES];
    uint8_t auc_ip_pkt_data[NUM_4_BYTES];
} mac_vowifi_nat_keep_alive_start_info_stru; /* hd_event */

/* vowifi质量评估参数配置命令集合 */
typedef enum {
    VOWIFI_SET_MODE = 0,
    VOWIFI_GET_MODE,
    VOWIFI_SET_PERIOD,
    VOWIFI_GET_PERIOD,
    VOWIFI_SET_LOW_THRESHOLD,
    VOWIFI_GET_LOW_THRESHOLD,
    VOWIFI_SET_HIGH_THRESHOLD,
    VOWIFI_GET_HIGH_THRESHOLD,
    VOWIFI_SET_TRIGGER_COUNT,
    VOWIFI_GET_TRIGGER_COUNT,
    VOWIFI_SET_IS_SUPPORT,

    VOWIFI_CMD_BUTT
} mac_vowifi_cmd_enum;
typedef uint8_t mac_vowifi_cmd_enum_uint8;

/* vowifi质量评估参数配置命令结构体 */
typedef struct {
    mac_vowifi_cmd_enum_uint8 en_vowifi_cfg_cmd; /* 配置命令 */
    uint8_t uc_value;                          /* 配置值 */
    uint8_t auc_resv[NUM_2_BYTES];
} mac_cfg_vowifi_stru; /* hd_event */

/* Number of Cipher Suites Implemented */
#define WLAN_PAIRWISE_CIPHER_SUITES 2
#define WLAN_AUTHENTICATION_SUITES  2
/* 解析内核配置的connect参数后，下发给驱动的connect参数 */
typedef struct {
    uint32_t wpa_versions;
    uint32_t group_suite;
    uint32_t aul_pair_suite[WLAN_PAIRWISE_CIPHER_SUITES];
    uint32_t aul_akm_suite[WLAN_AUTHENTICATION_SUITES];
    uint32_t group_mgmt_suite;
} mac_crypto_settings_stru; /* hd_event */

typedef struct {
    oal_bool_enum_uint8 en_privacy;                /* 是否加密标志 */
    oal_nl80211_auth_type_enum_uint8 en_auth_type; /* 认证类型，OPEN or SHARE-KEY */
    uint8_t uc_wep_key_len;                      /* WEP KEY长度 */
    uint8_t uc_wep_key_index;                    /* WEP KEY索引 */
    uint8_t auc_wep_key[WLAN_WEP104_KEY_LEN];    /* WEP KEY密钥 */
    mac_nl80211_mfp_enum_uint8 en_mgmt_proteced;   /* 此条链接pmf是否使能 */
    wlan_pmf_cap_status_uint8 en_pmf_cap;          /* 设备pmf能力 */
    oal_bool_enum_uint8 en_wps_enable;
    mac_crypto_settings_stru st_crypto; /* 密钥套件信息 */
#ifdef _PRE_WLAN_FEATURE_11R
    uint8_t auc_mde[NUM_8_BYTES]; /* MD IE信息 */
#endif                    // _PRE_WLAN_FEATURE_11R
    int8_t c_rssi;      /* 关联AP的RSSI信息 */
    int8_t rssi[HD_EVENT_RF_NUM];     /* 4天线的rssi */
    oal_bool_enum_uint8 is_wapi_connect;
    uint8_t auc_rsv[NUM_2_BYTES];
} mac_conn_security_stru; /* hd_event */

typedef struct {
    uint8_t uc_key_index;
    oal_bool_enum_uint8 en_pairwise;
    uint8_t auc_mac_addr[OAL_MAC_ADDR_LEN];
    mac_key_params_stru st_key;
} mac_addkey_param_stru; /* hd_event */

typedef struct {
    uint8_t uc_key_index;
    oal_bool_enum_uint8 en_pairwise;
    uint8_t auc_mac_addr[OAL_MAC_ADDR_LEN];
} mac_removekey_param_stru; /* hd_event */
/* end:from mac_vap_common.h */
typedef struct {
    uint8_t auc_ssid[WLAN_SSID_MAX_LEN];
    uint8_t uc_action_dialog_token;
    uint8_t uc_meas_token;
    uint8_t uc_oper_class;
    uint16_t us_ssid_len;
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    uint16_t us_duration;
    uint8_t auc_resv[NUM_2_BYTES];
} mac_vap_rrm_trans_req_info_stru; /* hd_event */

#ifdef _PRE_WLAN_DELAY_STATISTIC
#define DL_TIME_ARRAY_LEN 10
#define TID_DELAY_STAT    0
#define AIR_DELAY_STAT    1
typedef struct {
    uint16_t dl_time_array[DL_TIME_ARRAY_LEN];
    uint8_t dl_measure_type;
} user_delay_info_stru; /* hd_event */

typedef struct {
    uint32_t dmac_packet_count_num;
    uint32_t dmac_report_interval;
    uint8_t dmac_stat_enable;
    uint8_t reserved[NUM_3_BYTES];
} user_delay_switch_stru;
#endif

/* 漫游切换状态 */
typedef enum {
    WLAN_ROAM_MAIN_BAND_STATE_2TO2 = 0,
    WLAN_ROAM_MAIN_BAND_STATE_5TO2 = 1,
    WLAN_ROAM_MAIN_BAND_STATE_2TO5 = 2,
    WLAN_ROAM_MAIN_BAND_STATE_5TO5 = 3,

    WLAN_ROAM_MAIN_BAND_STATE_BUTT
} wlan_roam_main_band_state_enum;
typedef uint8_t wlan_roam_main_band_state_enum_uint8; /* hd_event */

/* roam trigger 数据结构体 */
typedef struct {
    int8_t c_trigger_2G;
    int8_t c_trigger_5G;
    uint16_t us_roam_interval;
} mac_roam_trigger_stru; /* hd_event */

/* roam hmac 同步 dmac数据结构体 */
typedef struct {
    uint16_t us_sta_aid;
    uint16_t us_pad;
    mac_channel_stru st_channel;
    mac_user_cap_info_stru st_cap_info;
    mac_key_mgmt_stru st_key_info;
    mac_user_tx_param_stru st_user_tx_info;
    uint32_t back_to_old;
} mac_h2d_roam_sync_stru; /* hd_event */

/* zero wait dfs 设置工作频段，信道和带宽参数,检测时间 */
typedef struct {
    mac_channel_stru channel_param;
    uint32_t radar_detect_time;
} zero_wait_dfs_param_stru; /* hd_event */
#endif /* end of hd_event.h */

