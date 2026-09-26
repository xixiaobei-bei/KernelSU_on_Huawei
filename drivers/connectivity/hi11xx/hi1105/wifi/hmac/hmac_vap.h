

#ifndef __HMAC_VAP_H__
#define __HMAC_VAP_H__

#include "oal_ext_if.h"
#include "mac_vap.h"
#include "hmac_ext_if.h"
#include "hmac_user.h"
#include "hmac_main.h"
#include "mac_resource.h"
#include "mac_ftm.h"
#ifdef _PRE_WLAN_TCP_OPT
#include "hmac_tcp_opt_struc.h"
#include "oal_hcc_host_if.h"
#endif
#include "hmac_btcoex.h"
#include "hmac_arp_probe.h"

#include "hmac_host_performance.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_VAP_H

#ifdef _PRE_WLAN_DFT_STAT
#define HMAC_VAP_DFT_STATS_PKT_INCR(_member, _cnt) ((_member) += (_cnt))
#else
#define HMAC_VAP_DFT_STATS_PKT_INCR(_member, _cnt)
#endif
#define HMAC_VAP_STATS_PKT_INCR(_member, _cnt) ((_member) += (_cnt))

#ifdef _PRE_WLAN_FEATURE_HS20
#define MAX_QOS_UP_RANGE 8
#define MAX_DSCP_EXCEPT  21 /* maximum of DSCP Exception fields for QoS Map set */
#endif

#define HMAC_MAX_DSCP_VALUE_NUM 64
#define HMAC_DSCP_VALUE_INVALID 0xA5

#ifdef _PRE_WLAN_FEATURE_FTM
#define HMAC_GAS_DIALOG_TOKEN_INITIAL_VALUE 0x80        /* hmac gas的dialog token num */
#endif

extern uint8_t g_uc_host_rx_ampdu_amsdu;

typedef struct {
    oal_dlist_head_stru st_timeout_head;
} hmac_mgmt_timeout_stru;

typedef struct {
    uint16_t us_user_index;
    mac_vap_state_enum_uint8 en_state;
    uint8_t uc_vap_id;
    mac_status_code_enum_uint16 en_status_code;
    uint8_t auc_rsv[NUM_2_BYTES];
} hmac_mgmt_timeout_param_stru;

#ifdef _PRE_WLAN_FEATURE_HS20
typedef struct {
    uint8_t auc_up_low[MAX_QOS_UP_RANGE]; /* User Priority */
    uint8_t auc_up_high[MAX_QOS_UP_RANGE];
    uint8_t auc_dscp_exception_up[MAX_DSCP_EXCEPT]; /* User Priority of DSCP Exception field */
    uint8_t uc_valid;
    uint8_t uc_num_dscp_except;
    uint8_t auc_dscp_exception[MAX_DSCP_EXCEPT]; /* DSCP exception field  */
} hmac_cfg_qos_map_param_stru;
#endif

/* 装备测试 */
typedef struct {
    uint32_t rx_pkct_succ_num;    /* 接收数据包数 */
    uint32_t dbb_num;             /* DBB版本号 */
    uint32_t check_fem_pa_status; /* fem和pa是否烧毁标志 */
    int16_t s_rx_rssi;
    oal_bool_enum_uint8 uc_get_dbb_completed_flag; /* 获取DBB版本号成功上报标志 */
    oal_bool_enum_uint8 uc_check_fem_pa_flag;      /* fem和pa是否烧毁上报标志 */
    oal_bool_enum_uint8 uc_get_rx_pkct_flag;       /* 接收数据包上报标志位 */
    oal_bool_enum_uint8 uc_lte_gpio_check_flag;    /* 接收数据包上报标志位 */
    oal_bool_enum_uint8 uc_report_efuse_reg_flag;  /* efuse 寄存器读取 */
    oal_bool_enum_uint8 uc_report_reg_flag;        /* 单个寄存器读取 */
    uint32_t reg_value;
    uint8_t uc_ant_status : 4,
            uc_get_ant_flag : 4;
} hmac_atcmdsrv_get_stats_stru;

typedef struct {
    oal_dlist_head_stru st_entry;
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    uint8_t uc_reserved[NUM_2_BYTES];
    uint8_t auc_pmkid[WLAN_PMKID_LEN];
} hmac_pmksa_cache_stru;

typedef enum _hmac_tcp_opt_queue_ {
    HMAC_TCP_ACK_QUEUE = 0,
    HMAC_TCP_OPT_QUEUE_BUTT
} hmac_tcp_opt_queue;

#ifdef _PRE_WLAN_TCP_OPT
struct hmac_vap_tag;
typedef uint16_t (*hmac_trans_cb_func)(struct hmac_vap_tag *hmac_vap, hmac_tcp_opt_queue type,
                                       hcc_chan_type dir, oal_netbuf_head_stru *data);
/* tcp_ack优化 */
typedef struct {
    struct wlan_perform_tcp hmac_tcp_ack;
    struct wlan_perform_tcp_list hmac_tcp_ack_list;
    wlan_perform_tcp_impls filter_info;
    hmac_trans_cb_func filter[HMAC_TCP_OPT_QUEUE_BUTT];  // 过滤处理钩子函数
    uint64_t all_ack_count[HMAC_TCP_OPT_QUEUE_BUTT];   // 丢弃的TCP ACK统计
    uint64_t drop_count[HMAC_TCP_OPT_QUEUE_BUTT];      // 丢弃的TCP ACK统计
    oal_netbuf_head_stru data_queue[HMAC_TCP_OPT_QUEUE_BUTT];
    oal_spin_lock_stru data_queue_lock[HMAC_TCP_OPT_QUEUE_BUTT];
} hmac_tcp_ack_stru;
#endif

/* 查询事件ID枚举 */
typedef enum {
    QUERY_ID_KO_VERSION = 0,
    QUERY_ID_PWR_REF = 1,
    QUERY_ID_BCAST_RATE = 2,
    QUERY_ID_STA_INFO = 3,
    QUERY_ID_ANT_RSSI = 4,
    QUERY_ID_STA_DIAG_INFO = 5,
    QUERY_ID_VAP_DIAG_INFO = 6,
    QUERY_ID_SENSING_BSSID_INFO = 7,
    QUERY_ID_BG_NOISE = 8,
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    QUERY_ID_TCP_ACK_BUF = 10,
#endif
    QUERY_ID_MODE_BUTT
} hmac_vap_query_enent_id_enum;

typedef struct {
    uint8_t uc_type;
    uint8_t uc_eid;
    uint8_t auc_resv[NUM_2_BYTES];
} hmac_remove_ie_stru;

typedef enum {
    HMAC_VAP_ARP_PROBE_TYPE_HTC = BIT0, /* htc arp 探测失败会关闭htc触发重关联 */
    HMAC_VAP_ARP_PROBE_TYPE_M2S = BIT1, /* 切换成siso 触发重关联 */
    HMAC_VAP_ARP_PROBE_TYPE_DBAC = BIT2, /* 频繁发送NULL DATA场关闭RX AMSDU触发重关联 */
    HMAC_VAP_ARP_PROBE_TYPE_BTCOEX = BIT3, /* 频繁发送NULL DATA场关闭RX AMSDU触发重关联 */
    HMAC_VAP_ARP_PROBE_TYPE_BTCOEX_DELBA = BIT4, /* 共存触发的delba, 如果ARP不通需要恢复聚合个数并触发重关联 */
    HMAC_VAP_ARP_PROBE_TYPE_BUTT,
} hmac_vap_arp_probe_type_enum;

typedef struct {
    frw_timeout_stru st_arp_probe_timer; /* 发送ARP REQ后启动定时器 */
    oal_atomic rx_unicast_pkt_to_lan; /* 接收到的单播帧个数 */

    uint8_t uc_rx_no_pkt_count;        /* 超时时间内没有收到帧的次数 */
    oal_bool_enum_uint8 en_arp_probe_on; /* 是否打开arp统计，做重关联保护 */
    uint8_t uc_arp_probe_type;
    oal_bool_enum_uint8 en_init_succ;
} hmac_vap_arp_probe_stru;

/* 获取的字符串参数 */
typedef struct {
    int32_t l_buff_len;
    uint8_t auc_buff[WLAN_IWPRIV_MAX_BUFF_LEN];
} hmac_cfg_param_char_stru;

/* hmac vap结构体 */
/* 在向此结构体中增加成员的时候，请保持整个结构体8字节对齐 */
typedef struct hmac_vap_tag {
    /* ap sta公共字段 */
    oal_net_device_stru *pst_net_device;  /* VAP对应的net_devices */
    uint8_t auc_name[OAL_IF_NAME_SIZE]; /* VAP名字 */
    hmac_vap_cfg_priv_stru st_cfg_priv;   /* wal hmac配置通信接口 */

    oal_spin_lock_stru st_lock_state; /* 数据面和控制面对VAP状态进行互斥 */

    oal_mgmt_tx_stru st_mgmt_tx;
    frw_timeout_stru st_mgmt_timer;
    hmac_mgmt_timeout_param_stru st_mgmt_timetout_param;

    frw_timeout_stru st_scan_timeout; /* vap发起扫描时，会启动定时器，做超时保护处理 */
    uint8_t uc_scan_upload_num;

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    oal_delayed_work st_ampdu_work;
    oal_spin_lock_stru st_ampdu_lock;
    oal_delayed_work st_set_hw_work;
    mac_cfg_ampdu_tx_on_param_stru st_mode_set;
#endif

    hmac_remove_ie_stru st_remove_ie; /* 配置需要屏蔽的IE信息 */

    mac_blacklist_info_stru *pst_blacklist_info; /* 黑名单信息 */

    oal_net_device_stru *pst_p2p0_net_device; /* 指向p2p0 net device */
    oal_net_device_stru *pst_del_net_device;  /* 指向需要通过cfg80211 接口删除的 net device */
    oal_work_stru st_del_virtual_inf_worker;  /* 删除net_device 工作队列 */
    oal_bool_enum_uint8 en_wait_roc_end;

    uint8_t bit_in_p2p_listen : 1; /* vap是否在p2p listen流程 */
    uint8_t bit_resv : 7;
    uint8_t auc_resv6[NUM_2_BYTES];

    oal_completion st_roc_end_ready; /* roc end completion */

#ifdef _PRE_WLAN_FEATURE_HS20
    hmac_cfg_qos_map_param_stru st_cfg_qos_map_param;
#endif

    uint8_t bit_init_flag : 1;  /* 常发关闭再次打开标志 */
    uint8_t bit_ack_policy : 1; /* ack policy: 0:normal ack 1:normal ack */
    uint8_t bit_reserved : 6;
    uint8_t auc_resv1[NUM_2_BYTES];
    uint8_t ipaddr_obtained;  /* 标记是否获取到IPv4地址，在vap mode为STA时使用 */

    uint32_t *pul_roam_info;
    oal_bool_enum_uint8 en_roam_prohibit_on; /* 是否禁止漫游 */

#ifdef _PRE_WLAN_DFT_STAT
    uint8_t uc_device_distance;
    uint8_t uc_intf_state_cca;
    uint8_t uc_intf_state_co;
    uint8_t auc_resv2[1];
#endif

    uint16_t us_rx_timeout[WLAN_WME_AC_BUTT];     /* 不同业务重排序超时时间 */
    uint16_t us_rx_timeout_min[WLAN_WME_AC_BUTT]; /* 不同业务重排序超时时间 */

    uint16_t us_del_timeout;                     /* 多长时间超时删除ba会话 如果是0则不删除 */
    mac_cfg_mode_param_stru st_preset_para;        /* STA协议变更时变更前的协议模式 */
    uint8_t auc_supp_rates[WLAN_MAX_SUPP_RATES]; /* 支持的速率集 */

    uint8_t auc_resv4[1];

#ifdef _PRE_WLAN_FEATURE_WMMAC
    uint8_t uc_ts_dialog_token; /* TS会话创建伪随机值 */
    oal_bool_enum_uint8 en_wmmac_auth_flag;
    uint8_t uc_resv5[NUM_2_BYTES];
#endif  // _PRE_WLAN_FEATURE_WMMAC

    oal_bool_enum_uint8 auc_query_flag[QUERY_ID_MODE_BUTT];
    hmac_cfg_param_char_stru st_param_char;

    /* sta独有字段 */
    oal_bool_enum_uint8 en_no_beacon;
    uint8_t uc_no_smps_user_cnt_ap;              /* AP关联的不支持SMPS的用户数  */
    uint8_t uc_ba_dialog_token;                  /* BA会话创建伪随机值 */
    oal_bool_enum_uint8 en_updata_rd_by_ie_switch; /* 是否根据关联的ap跟新自己的国家码 */

    uint8_t bit_sta_protocol_cfg : 1;
    uint8_t bit_protocol_fall : 1; /* 降协议标志位 */
    uint8_t bit_reassoc_flag : 1;  /* 关联过程中判断是否为重关联动作 */
    
    uint8_t bit_rx_ampduplusamsdu_active : 3; /* 0关1开：bit0:5G；bit1: 2g非11ax。 bit2:2g 11ax(0开，1关) */
    uint8_t bit_sae_connect_with_pmkid : 1; /* 0:不包含PMKID的SAE连接；1:包含PMKID的SAE连接 */
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11R) ||      \
    defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    uint8_t bit_11k_auth_flag : 1; /* 11k 认证标志位 */
    uint8_t bit_voe_11r_auth : 1;
    uint8_t bit_11k_auth_oper_class : 2;
    uint8_t bit_11r_over_ds : 1; /* 是否使用11r over ds；0表示over ds走over air流程 */
    uint8_t bit_11k_enable : 1;
    uint8_t bit_11v_enable : 1;
    uint8_t bit_11r_enable : 1;
    uint8_t bit_bcn_table_switch : 1;
    uint8_t bit_adaptive11r : 1; /* 是否使用adaptive 11r流程 */
    uint8_t bit_nb_rpt_11k : 1;
    uint8_t bit_11r_private_preauth : 1;
    uint8_t bit_resv2 : 5;
    uint8_t uc_resv;
#else
    uint8_t bit_resv : 1;
#endif                              // _PRE_WLAN_FEATURE_11K
    int8_t ac_desired_country[NUM_3_BYTES]; /* 要加入的AP的国家字符串，前两个字节为国家字母，第三个为\0 */

    oal_dlist_head_stru st_pmksa_list_head;

    /* 信息上报 */
    oal_wait_queue_head_stru query_wait_q; /* 查询等待队列 */
    oal_station_info_stru station_info; /* 查询统计信息临时保存变量 */
    station_info_extend_stru st_station_info_extend; /* CHR2.0使用的STA统计信息 */
    hmac_vap_stat_stru vap_stats; /* 1106 host trx数据统计 */

    oal_bool_enum_uint8 station_info_query_completed_flag; /* 查询结束标志，OAL_TRUE，查询结束，OAL_FALSE，查询未结束 */
    int16_t s_free_power;                                /* 底噪 */
    uint8_t beacon_rssi; /* 收到的beacon信号强度 */
    oal_bool_enum_uint8 en_monitor_mode;

    int32_t center_freq; /* 中心频点 */
    mac_cfg_show_dieid_stru st_dieid;
    hmac_atcmdsrv_get_stats_stru st_atcmdsrv_get_status;

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    frw_timeout_stru st_edca_opt_timer; /* edca参数调整定时器 */
    uint32_t edca_opt_time_ms;     /* edca参数调整计时器周期 */
    uint8_t uc_edca_opt_flag_ap;      /* ap模式下是否使能edca优化特性 */
    uint8_t uc_edca_opt_flag_sta;     /* sta模式下是否使能edca优化特性 */
    uint8_t uc_edca_opt_weight_sta;   /* 调整beacon中edca参数的权重，最大值为 3 */
    uint8_t uc_idle_cycle_num;        /* vap连续处于idle状态的周期个数 */
#endif

#ifdef _PRE_WLAN_TCP_OPT
    hmac_tcp_ack_stru st_hmac_tcp_ack[HCC_DIR_COUNT];
#endif

    mac_h2d_protection_stru st_prot;

    frw_timeout_stru st_ps_sw_timer; /* 低功耗开关 */
    uint8_t uc_cfg_sta_pm_manual;  /* 手动设置sta pm mode的标志 */
    uint8_t uc_ps_mode;
    uint16_t us_check_timer_pause_cnt; /* 低功耗pause计数 */

    int32_t l_temp;
    hmac_vap_arp_probe_stru st_hmac_arp_probe;
#ifdef _PRE_WLAN_FEATURE_SAE
    oal_work_stru st_sae_report_ext_auth_worker; /* SAE report external auth req工作队列 */
    uint8_t duplicate_auth_seq2_flag;
    uint8_t duplicate_auth_seq4_flag;
    uint8_t resv[NUM_2_BYTES];
#endif
    uint8_t owe_group[MAC_OWE_GROUP_SUPPORTED_NUM];
    uint8_t owe_group_cap;

    oal_bool_enum_uint8 en_is_psk; /* 这次关联是否是psk */
    oal_bool_enum_uint8 en_sae_connect;
    oal_bool_enum_uint8 en_ps_rx_amsdu;
    oal_bool_enum_uint8 d2h_amsdu_switch;
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
    uint8_t en_is_hid2d_state;
    uint8_t  uc_seq_idx;
    uint16_t aus_last_seqnum[MAC_HID2D_SEQNUM_CNT];
    uint8_t  uc_resv10[NUM_2_BYTES];
#endif

#define MAX_TWT_NUM 4
#ifdef _PRE_WLAN_FEATURE_TWT
    mac_cfg_twt_stru st_twt_cfg[MAX_TWT_NUM]; /* TWT节能配置参数 */
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    uint8_t auc_resv11[1];
    uint8_t uc_mac_padding;
    uint8_t uc_disable_mu_edca;
    uint8_t uc_htc_order_flag;
    uint32_t htc_info;
#endif

#ifdef _PRE_WLAN_FEATURE_FTM // FTM认证，用于gas init comeback定时
    frw_timeout_stru    st_ftm_timer;
    mac_gas_mgmt_stru   st_gas_mgmt;
    void *feature_ftm;
#endif
    uint8_t             hal_dev_id; /* host vap对应的hal device id */
    oal_switch_enum_uint8 tcp_ack_buf_use_ctl_switch;   /* 由应用层控制的TCP ack缓存门限开关 */

    mac_vap_stru st_vap_base_info; /* MAC vap，只能放在最后! */
} hmac_vap_stru;

typedef enum _hmac_cac_event_ {
    HMAC_CAC_STARTED = 0,
    HMAC_CAC_FINISHED,
    HMAC_CAC_ABORTED,

    HMAC_CAC_BUTT
} hmac_cac_event;

typedef struct {
    hmac_cac_event en_type;
    uint32_t freq;
    wlan_channel_bandwidth_enum_uint8 en_bw_mode;
    uint8_t auc[NUM_23_BYTES];
} hmac_cac_event_stru;

typedef struct {
    uint32_t tx_throughput_mbps;
    uint32_t rx_throughput_mbps;
    uint32_t tx_pps;
} hmac_performance_stat_stru;
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
uint32_t hmac_vap_destroy(hmac_vap_stru *vap);
uint32_t hmac_vap_init(hmac_vap_stru *hmac_vap, uint8_t chip_id,
                       uint8_t device_id, uint8_t vap_id, mac_cfg_add_vap_param_stru *param);

uint32_t hmac_vap_creat_netdev(hmac_vap_stru *hmac_vap, int8_t *netdev_name,
                               uint8_t netdev_name_len, int8_t *mac_addr);

uint16_t hmac_vap_check_ht_capabilities_ap(hmac_vap_stru *hmac_vap, uint8_t *payload,
                                           uint32_t msg_len, hmac_user_stru *hmac_user_sta);
uint32_t hmac_search_ht_cap_ie_ap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user_sta,
                                  uint8_t *payload, uint16_t index, oal_bool_enum en_prev_asoc_ht);
oal_bool_enum_uint8 hmac_vap_addba_check(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, uint8_t tidno);

void hmac_vap_net_stopall(void);
void hmac_vap_net_startall(void);

oal_bool_enum_uint8 hmac_flowctl_check_device_is_sta_mode(void);
void hmac_vap_net_start_subqueue(uint16_t queue_idx);
void hmac_vap_net_stop_subqueue(uint16_t queue_idx);
void hmac_handle_disconnect_rsp(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
                                mac_reason_code_enum_uint16 en_disasoc_reason);
uint8_t *hmac_vap_get_pmksa(hmac_vap_stru *hmac_vap, uint8_t *bssid);
uint32_t hmac_tx_get_mac_vap(uint8_t vap_id, mac_vap_stru **vap_stru);
uint32_t hmac_restart_all_work_vap(uint8_t chip_id);
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
void hmac_set_ampdu_worker(oal_delayed_work *work);
void hmac_set_ampdu_hw_worker(oal_delayed_work *work);
#endif

void hmac_vap_state_restore(hmac_vap_stru *hmac_vap);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_vap.h */
