

#ifndef __HMAC_ROAM_ALG_H__
#define __HMAC_ROAM_ALG_H__

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
#include "hmac_11v.h"
#endif
#include "hmac_roam_connect.h"
#include "hmac_roam_main.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ROAM_ALG_H

/* 此处extern "C"删掉后编译不过 */
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_SAE
static inline bool is_only_support_sae(uint32_t *pul_suites)
{
    return ((*(pul_suites) == MAC_RSN_AKM_SAE) && (*((pul_suites) + 1) == 0));
}
static inline bool is_only_support_tbpeke(uint32_t *pul_suites)
{
    return ((*(pul_suites) == MAC_RSN_AKM_TBPEKE) && (*((pul_suites) + 1) == 0));
}
#endif

#define ROAM_LIST_MAX                     (4)     /* 名单规格 */
#define ROAM_BLACKLIST_NORMAL_AP_TIME_OUT (30000) /* 黑名单正常超时时间 */
#define ROAM_BLACKLIST_REJECT_AP_TIME_OUT (30000) /* 黑名单拒绝模式超时时间 */
#define ROAM_BLACKLIST_COUNT_LIMIT        (1)     /* 黑名单超时次数限制 */

#define ROAM_HISTORY_BSS_TIME_OUT   (20000) /* 历史优选名单超时时间 */
#define ROAM_HISTORY_COUNT_LIMIT    (1)     /* 历史优选超时次数限制 */
#define ROAM_RSSI_LEVEL             (3)
#define ROAM_CONCURRENT_USER_NUMBER (10)
#define ROAM_THROUGHPUT_THRESHOLD   (1000)

#define ROAM_RSSI_NE85_DB (-85)
#define ROAM_RSSI_NE80_DB (-80)
#define ROAM_RSSI_NE78_DB (-78)
#define ROAM_RSSI_NE75_DB (-75)
#define ROAM_RSSI_NE72_DB (-72)
#define ROAM_RSSI_NE70_DB (-70)
#define ROAM_RSSI_NE68_DB (-68)
#define ROAM_RSSI_NE65_DB (-65)
#define ROAM_RSSI_NE50_DB (-50)

#define ROAM_RSSI_DIFF_2_DB  (2)
#define ROAM_RSSI_DIFF_4_DB  (4)
#define ROAM_RSSI_DIFF_6_DB  (6)
#define ROAM_RSSI_DIFF_8_DB  (8)
#define ROAM_RSSI_DIFF_10_DB (10)
#define ROAM_RSSI_DIFF_15_DB (15)
#define ROAM_RSSI_DIFF_20_DB (20)
#define ROAM_RSSI_DIFF_30_DB (30)

#define ROAM_RSSI_CMD_TYPE      (-128)
#define ROAM_RSSI_LINKLOSS_TYPE (-121)
#define ROAM_RSSI_MAX_TYPE      (-126)

#define ROAM_NEIGHBOR_RPT_LIST_CHN_MAX_NUM (13)
#define ROAM_NEIGHBOR_RPT_LIST_BSSID_MAX_NUM (24)


/* 漫游黑名单类型 */
typedef enum {
    ROAM_BLACKLIST_TYPE_NORMAL_AP = 0,
    ROAM_BLACKLIST_TYPE_REJECT_AP = 1,
    ROAM_BLACKLIST_TYPE_BUTT
} roam_blacklist_type_enum;

typedef enum {
    ROAMING_DISABLE = 0,
    ROAMING_RESTART = 1,
    ROAMING_SCENARIO_ENTERPRISE = 2,
    ROAMING_SCENARIO_ENTERPRISE_DENSE = 3,
    ROAMING_SCENARIO_BETTER_AP = 4,
    ROAMING_SCENARIO_HOME = 5,
    ROAMING_SCENARIO_BUTT
} roam_scenario_type_enum;

typedef struct {
    uint32_t time_stamp;  /* 记录黑名单时间戳 */
    uint32_t timeout;     /* 记录黑名单超时时限 */
    uint16_t us_count_limit; /* 记录黑名单超时前的加入次数限制 */
    uint16_t us_count;       /* 黑名单超时过程中，添加该Bssid的次数 */
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
} hmac_roam_bss_info_stru;

/* 漫游blacklist结构体 */
typedef struct {
    hmac_roam_bss_info_stru ast_bss[ROAM_LIST_MAX];
} hmac_roam_bss_list_stru;

typedef struct {
    int8_t c_trigger_rssi; /* 家庭网络优选触发rssi */
    int8_t c_5g_rssi_th;   /* 家庭网络5g漫游门限 */
    int8_t c_5g_hold_th;   /* 家庭网络5g保持漫游门限 */
} hmac_roam_homenetwork_thr_stru;

#define MAX_RECORD_BSSID_NUM 2

typedef struct {
    uint8_t auc_bssid[MAX_RECORD_BSSID_NUM][WLAN_MAC_ADDR_LEN]; /* 记录网络的bssid */
    mac_channel_stru st_channel[MAX_RECORD_BSSID_NUM];            /* 记录网络信息 */
    uint8_t uc_bssid_num;                                       /* 记录bssid num的个数 */
} hmac_roam_record_stru;

/* 漫游家庭网络结构体 */
typedef struct {
    int8_t c_trigger_rssi;     /* 家庭网络优选触发rssi */
    int8_t c_5g_rssi_th;       /* 家庭网络5g漫游门限 */
    int8_t c_5g_hold_th;       /* 家庭网络5g保持漫游门限 */
    uint8_t uc_max_retry_cnt;  /* 每个rssi最多触发几次漫游 */
    uint8_t uc_roam_fail_cnt;  /* 漫游失败次数 */
    uint8_t uc_is_homenetwork; /* 是否是家庭网络场景 */
    uint8_t auc_resv[2]; /* 2实现字节对齐 */
    hmac_roam_record_stru st_2g_bssid; /* 记录扫描到的2g的bssid */
    hmac_roam_record_stru st_5g_bssid; /* 记录扫描到的5g的bssid */
    uint32_t scan_timestamp_5g;   /* 记录扫描5g rssi时间戳，30s扫描一次 */
} hmac_roam_homenetwork_stru;

/* 记录扫描到的信道信息 */
#define MAX_RECORD_CHAN_NUM    3

typedef struct {
    mac_channel_stru channel[MAX_RECORD_CHAN_NUM];
    uint8_t chan_num;
    uint8_t resv[BYTE_OFFSET_3];
} hmac_roam_record_chan_stru;

typedef struct {
    hmac_roam_record_chan_stru chan_2g;
    hmac_roam_record_chan_stru chan_5g;
} hmac_roam_scan_stru;

/* 漫游算法结构体 */
typedef struct {
    hmac_roam_bss_list_stru st_blacklist;    /* 漫游黑名单AP历史记录 */
    hmac_roam_bss_list_stru st_history;      /* 漫游优选AP历史记录 */
    uint32_t max_capacity;              /* 记录 scan 结果的最大 capacity */
    mac_bss_dscr_stru *pst_max_capacity_bss; /* 记录 scan 结果的最大 capacity 的 bss */
    int8_t c_current_rssi;                 /* 当前 dmac 触发 rssi */
    int8_t c_max_rssi;                     /* 记录 scan 结果的最大 rssi */
    uint8_t uc_another_bss_scaned;         /* 是否扫描到了非当前关联的 bss */
    uint8_t uc_invalid_scan_cnt;           /* 连续扫描到当前关联且首次关联的 bss 的次数 */
    uint8_t uc_candidate_bss_num;          /* 相同SSID的BSS Num总数目，包括good, weak, and roaming */
    uint8_t uc_candidate_good_rssi_num;    /* 相同SSID强信号强度的BSS Num */
    uint8_t uc_candidate_weak_rssi_num;    /* 相同SSID弱信号强度的BSS Num */
    uint8_t uc_scan_period;                /* 从高密场景向低密场景切换的扫描周期 */
    uint8_t uc_better_rssi_scan_period;    /* 有更好信号强度AP的连续扫描次数 */
    uint8_t uc_better_rssi_null_period;
    uint8_t uc_roam_fail_cnt;
    uint8_t uc_rsv[1];
    hmac_roam_homenetwork_stru st_home_network; /* 家庭网络结构体信息 */
    mac_bss_dscr_stru *pst_max_rssi_bss;        /* 记录 scan 结果的最大 rssi 的 bss */
} hmac_roam_alg_stru;

/* 漫游connect结构体 */
typedef struct {
    roam_connect_state_enum en_state;
    mac_status_code_enum_uint16 en_status_code; /* auth/assoc rsp frame status code */
    uint8_t uc_roam_main_fsm_state; // only for chr record main statemachine
    uint8_t uc_auth_num;
    uint8_t uc_assoc_num;
    uint8_t uc_ft_num;
    uint8_t uc_ft_force_air; /* 0:可以使用ds，1:ds失败，禁止使用ds */
    uint8_t uc_ft_failed;    /* 0:ds漫游未失败，1:ds漫游失败 */
    frw_timeout_stru st_timer; /* 漫游connect使用的定时器 */
    mac_bss_dscr_stru *pst_bss_dscr;
} hmac_roam_connect_stru;
typedef struct {
    int8_t c_rssi;
    uint32_t capacity_kbps;
} hmac_roam_rssi_capacity_stru;

/* 旧bss数据结构体 */
typedef struct {
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    uint16_t us_sta_aid;
#ifdef _PRE_WLAN_FEATURE_TXBF
    mac_vap_txbf_add_stru st_txbf_add_cap;
#endif
    mac_cap_flag_stru st_cap_flag;
    mac_channel_stru st_channel;
    wlan_mib_ieee802dot11_stru st_mib_info;
    mac_user_cap_info_stru st_cap_info;
    mac_key_mgmt_stru st_key_info;
    mac_user_tx_param_stru st_user_tx_info; /* TX相关参数 */
    hmac_rate_stru st_op_rates;
    mac_user_ht_hdl_stru st_ht_hdl;
    mac_vht_hdl_stru st_vht_hdl;
    wlan_bw_cap_enum_uint8 en_bandwidth_cap;
    wlan_bw_cap_enum_uint8 en_avail_bandwidth;
    wlan_bw_cap_enum_uint8 en_cur_bandwidth;
    wlan_protocol_enum_uint8 en_protocol_mode;
    wlan_protocol_enum_uint8 en_avail_protocol_mode;
    wlan_protocol_enum_uint8 en_cur_protocol_mode;
    wlan_nss_enum_uint8 en_user_num_spatial_stream;
    wlan_nss_enum_uint8 en_avail_num_spatial_stream;
    wlan_nss_enum_uint8 en_cur_num_spatial_stream;
    wlan_nss_enum_uint8 en_avail_bf_num_spatial_stream;
    uint16_t us_cap_info; /* 旧的bss的能力位信息 */
    mac_ap_type_enum_uint16 en_ap_type;
    mac_sap_mode_enum_uint8 sap_mode;
    uint8_t reservd[NUM_1_BYTES];
} hmac_roam_old_bss_stru;

/* 漫游主结构体 */
typedef struct {
    uint8_t uc_enable;                       /* 漫游使能开关 */
    roam_trigger_enum en_roam_trigger;   /* 漫游触发源 */
    roam_main_state_enum en_main_state;  /* 漫游主状态 */
    roam_scenario_enum uc_rssi_type;     /* 何种rssi类型触发漫游 */
    uint8_t uc_invalid_scan_cnt;             /* 无效扫描统计 */
    oal_bool_enum_uint8 en_current_bss_ignore; /* 是否支持漫游回自己(支持驱动发起重关联) */
    uint8_t auc_target_bssid[WLAN_MAC_ADDR_LEN];
#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    oal_work_stru hmac_11v_roam_work;        /* 11v 2次单信道+1次全信道扫描 工作队列 */
    hmac_bsst_rsp_info_stru st_bsst_rsp_info;
#endif
    hmac_vap_stru *pst_hmac_vap;                       /* 漫游对应的vap */
    hmac_user_stru *pst_hmac_user;                     /* 漫游对应的BSS user */
    hmac_roam_old_bss_stru st_old_bss;                 /* 漫游之前保存旧bss相关信息 */
    mac_scan_req_h2d_stru st_scan_h2d_params;          /* 漫游扫描参数 */
    hmac_roam_config_stru st_config;                   /* 漫游相关配置信息 */
    hmac_roam_connect_stru st_connect;                 /* 漫游connect信息 */
    hmac_roam_alg_stru st_alg;                         /* 漫游算法信息 */
    hmac_roam_static_stru st_static;                   /* 漫游统计信息 */
    hmac_roam_scan_stru scan_info;                     /* 漫游扫描信息 */
    frw_timeout_stru st_timer;                         /* 漫游使用的定时器 */
    hmac_wpas_connect_state_enum_uint32 connected_state; /* 外部关联的状态进度 */

    uint8_t auc_neighbor_rpt_bssid[ROAM_NEIGHBOR_RPT_LIST_BSSID_MAX_NUM][WLAN_MAC_ADDR_LEN];
    uint8_t uc_neighbor_rpt_bssid_num;
    uint8_t uc_roaming_scenario;
    oal_bool_enum_uint8 is_roaming_trigged_by_cmd; /* 是否由命令触发的强制漫游  0:不是 1:是 */

    uint8_t stay_on_current_bss;                    /* 需要驻留当前bss，拒绝漫游 */
    mac_bss_dscr_stru save_bss_dscr_info;           /* 保留当前选择的bss信息 */
} hmac_roam_info_stru;
typedef uint32_t (*hmac_roam_fsm_func)(hmac_roam_info_stru *pst_roam_info, void *p_param);

#ifdef _PRE_WINDOWS_SUPPORT
typedef struct {
    hmac_roam_info_stru roam_info;
    mac_bss_dscr_stru bss_dscr;
    uint8_t scan_timeout; // 漫游扫描超时标记，TRUE标识超时，FALSE标识未超时，默认未超时
    uint8_t rsv[NUM_3_BYTES];
} hmac_roam_info_context;
#endif

uint32_t hmac_roam_alg_add_blacklist(hmac_roam_info_stru *pst_roam_info, uint8_t *puc_bssid,
                                     roam_blacklist_type_enum list_type);
uint32_t hmac_roam_alg_add_history(hmac_roam_info_stru *pst_roam_info, uint8_t *puc_bssid);
uint32_t hmac_roam_alg_bss_check(hmac_roam_info_stru *pst_roam_info,
                                 mac_bss_dscr_stru *pst_bss_dscr);
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
void hmac_roam_neighbor_report_add_chan(mac_scan_req_stru *pst_scan_params,
    uint8_t uc_channum, wlan_channel_band_enum_uint8 en_band, uint8_t uc_channel_idx);
void hmac_roam_neighbor_report_add_bssid(hmac_roam_info_stru *pst_roam_info, uint8_t *puc_bssid);
#endif
void hmac_roam_alg_init(hmac_roam_info_stru *pst_roam_info, int8_t c_current_rssi);
void hmac_roam_alg_init_rssi(hmac_vap_stru *pst_hmac_vap, hmac_roam_info_stru *pst_roam_info);
mac_bss_dscr_stru *hmac_roam_alg_select_bss(hmac_roam_info_stru *pst_roam_info);
oal_bool_enum_uint8 hmac_roam_alg_find_in_blacklist(hmac_roam_info_stru *pst_roam_info,
                                                    uint8_t *puc_bssid);
oal_bool_enum_uint8 hmac_roam_alg_find_in_history(hmac_roam_info_stru *pst_roam_info,
                                                  uint8_t *puc_bssid);
oal_bool_enum_uint8 hmac_roam_alg_need_to_stop_roam_trigger(hmac_roam_info_stru *pst_roam_info);
uint32_t hmac_roam_alg_bss_in_ess(hmac_roam_info_stru *pst_roam_info,
                                  mac_bss_dscr_stru *pst_bss_dscr);
#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
oal_bool_enum_uint8 hmac_roam_alg_check_bsst_bssid_in_scan_list(hmac_roam_info_stru *pst_roam_info,
                                                                uint8_t *puc_mac_addr);
#endif
void hmac_chr_roam_info_report(hmac_roam_info_stru *pst_roam_info, uint8_t result);
uint32_t hmac_roam_main_fsm_action(hmac_roam_info_stru *roam_info,
    roam_main_fsm_event_type_enum event, void *param);
uint32_t hmac_roam_main_check_state(hmac_roam_info_stru *roam_info,
                                    mac_vap_state_enum_uint8 vap_state,
                                    roam_main_state_enum main_state);
void hmac_roam_main_change_state(hmac_roam_info_stru *roam_info, roam_main_state_enum state);
#ifdef _PRE_WLAN_FEATURE_11K
uint32_t hmac_roam_send_neighbor_req(hmac_roam_info_stru *roam_info, uint8_t  *cur_ssid);
#endif
void hmac_roam_main_start_timer(hmac_roam_info_stru *roam_info, uint32_t timeout);
uint32_t hmac_roam_handle_scan_result(hmac_roam_info_stru *roam_info, void *param);
void hmac_roam_main_clear(hmac_roam_info_stru *roam_info);
uint32_t hmac_roam_resume_pm(hmac_roam_info_stru *roam_info);
void hmac_roam_resume_security_port(hmac_roam_info_stru *roam_info);
uint32_t hmac_roam_scan_timeout(hmac_roam_info_stru *roam_info, void *param);
void hmac_roam_main_del_timer(hmac_roam_info_stru *roam_info);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_roam_alg.h */
