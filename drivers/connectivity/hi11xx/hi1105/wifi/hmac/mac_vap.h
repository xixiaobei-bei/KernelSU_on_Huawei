
#ifndef __MAC_VAP_H__
#define __MAC_VAP_H__

/* 1 其他头文件包含 */
#include "mac_device.h"
#include "mac_user.h"
#include "mac_common.h"
#include "mac_cfg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" { // extern C 删了ut编译不过
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_VAP_H

/* 2 宏定义 */
#define MAC_VAP_USER_HASH_MAX_VALUE     (WLAN_ASSOC_USER_MAX_NUM * 2) /* 2为扩展因子 */
#define MAC_CALCULATE_HASH_VALUE(_puc_mac_addr) \
    (((_puc_mac_addr)[0] + (_puc_mac_addr)[1]           \
    + (_puc_mac_addr)[2] + (_puc_mac_addr)[3]           \
    + (_puc_mac_addr)[4] + (_puc_mac_addr)[5])          \
    & (MAC_VAP_USER_HASH_MAX_VALUE - 1))

#ifdef _PRE_WLAN_FEATURE_11AX
#define IS_HAL_DEVICE_SUPPORT_11AX(_pst_mac_vap) (OAL_TRUE == (_pst_mac_vap)->en_11ax_hal_cap)
#define IS_CUSTOM_OPEN_11AX_SWITCH(_pst_mac_vap) (OAL_TRUE == (_pst_mac_vap)->en_11ax_custom_switch)
#define MAC_VAP_IS_SUPPORT_11AX(_pst_mac_vap)                \
    (IS_HAL_DEVICE_SUPPORT_11AX(_pst_mac_vap) && IS_CUSTOM_OPEN_11AX_SWITCH(_pst_mac_vap))
#define IS_CUSTOM_OPEN_MULTI_BSSID_SWITCH(_pst_mac_vap) (OAL_TRUE == (_pst_mac_vap)->bit_multi_bssid_custom_switch)
#define IS_NON_TRANSMITTED_BSSID_BSS(_pst_mbss_info)         \
    ((OAL_TRUE == (_pst_mbss_info)->bit_is_non_transimitted_bss) && \
    (OAL_TRUE == (_pst_mbss_info)->bit_ext_cap_multi_bssid_activated))
#define IS_CUSTOM_OPEN_HTC_TOM(_pst_mac_vap)              \
    ((OAL_TRUE == (_pst_mac_vap)->bit_htc_include_custom_switch) && \
    (OAL_TRUE == (_pst_mac_vap)->bit_om_include_custom_switch))
#define VAP_SUPPORT_TOM(_pst_mac_vap) (MAC_VAP_IS_WORK_HE_PROTOCOL(_pst_mac_vap) && \
    IS_CUSTOM_OPEN_HTC_TOM(_pst_mac_vap))
#define MAC_VAP_IS_WORK_HE_PROTOCOL(_pst_mac_vap) ((_pst_mac_vap)->en_protocol == WLAN_HE_MODE)
// for csec, cycle complexity
#define MAC_VAP_IS_STA_HE_MODE(_pst_vap, _en_vap_protocol) \
    ((_pst_vap)->en_vap_mode == WLAN_VAP_MODE_BSS_STA && (_en_vap_protocol) == WLAN_HE_MODE)
#define MAC_VAP_IS_STA_VHT_MODE(_pst_vap, _en_vap_protocol) \
    ((_pst_vap)->en_vap_mode == WLAN_VAP_MODE_BSS_STA && (_en_vap_protocol) == WLAN_VHT_MODE)
#endif /* _PRE_WLAN_FEATURE_11AX */

#define IS_AP(_pst_mac_vap)  (WLAN_VAP_MODE_BSS_AP  == (_pst_mac_vap)->en_vap_mode)
#define IS_STA(_pst_mac_vap) (WLAN_VAP_MODE_BSS_STA == (_pst_mac_vap)->en_vap_mode)

#define IS_P2P_DEV(_pst_mac_vap)    (WLAN_P2P_DEV_MODE == (_pst_mac_vap)->en_p2p_mode)
#define IS_P2P_GO(_pst_mac_vap)     (WLAN_P2P_GO_MODE == (_pst_mac_vap)->en_p2p_mode)
#define IS_P2P_CL(_pst_mac_vap)     (WLAN_P2P_CL_MODE == (_pst_mac_vap)->en_p2p_mode)
#define IS_LEGACY_VAP(_pst_mac_vap) (WLAN_LEGACY_VAP_MODE == (_pst_mac_vap)->en_p2p_mode)
#define IS_LEGACY_STA(_pst_mac_vap) (IS_STA(_pst_mac_vap) && IS_LEGACY_VAP(_pst_mac_vap))
#define IS_LEGACY_AP(_pst_mac_vap)  (IS_AP(_pst_mac_vap) && IS_LEGACY_VAP(_pst_mac_vap))
#define IS_BRIDGE_VAP(_pst_mac_vap) (IS_P2P_GO(_pst_mac_vap) && \
    ((_pst_mac_vap)->ast_app_ie[OAL_APP_BEACON_IE].ie_len == 0))


#define MAC_SEND_TWO_DEAUTH_FLAG 0xf000

#define MAC_VAP_AMPDU_HW 8
#define MAC_VAP_CHECK_VAP_STATUS_VALID(_pst_mac_vap) (MAC_VAP_VAILD == (_pst_mac_vap)->uc_init_flag)

#ifdef _PRE_WLAN_FEATURE_DFS
#define MAC_VAP_GET_SUPPORT_DFS(_pst_mac_vap) ((_pst_mac_vap)->en_radar_detector_is_supp)
#define MAC_VAP_IS_SUPPORT_DFS(_pst_mac_vap)  (OAL_TRUE == (_pst_mac_vap)->en_radar_detector_is_supp)
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
#define MAC_VAP_IS_2G_SPEC_SISO(_pst_mac_vap) \
    ((WLAN_BAND_2G == (_pst_mac_vap)->st_channel.en_band && OAL_TRUE == (_pst_mac_vap)->st_cap_flag.bit_2g_custom_siso))
#define MAC_VAP_IS_5G_SPEC_SISO(_pst_mac_vap) \
    ((WLAN_BAND_5G == (_pst_mac_vap)->st_channel.en_band && OAL_TRUE == (_pst_mac_vap)->st_cap_flag.bit_5g_custom_siso))

#define MAC_VAP_SPEC_IS_SW_NEED_M2S_SWITCH(_pst_mac_vap) \
    (MAC_VAP_IS_2G_SPEC_SISO(_pst_mac_vap) || MAC_VAP_IS_5G_SPEC_SISO(_pst_mac_vap))
#endif /* _PRE_WLAN_FEATURE_M2S */

#define IS_USER_SINGLE_NSS(_pst_mac_user) (WLAN_SINGLE_NSS == (_pst_mac_user)->en_user_max_cap_nss)

#define IS_VAP_SINGLE_NSS(_pst_mac_vap) (WLAN_SINGLE_NSS == (_pst_mac_vap)->en_vap_rx_nss)
enum {
    FREQ_IDLE = 0,

    FREQ_MIDIUM = 1,

    FREQ_HIGHER = 2,

    FREQ_HIGHEST = 3,

    FREQ_BUTT = 4
};
#define FREQ_TC_EN   1
#define FREQ_TC_EXIT 0

/* 4 全局变量声明 */
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
extern uint8_t g_mcm_mask_custom;
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
extern oal_bool_enum_uint8 g_en_mimo_blacklist;
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
extern oal_bool_enum_uint8 g_en_hid2d_debug;
#endif
#ifdef _PRE_WLAN_FEATURE_MBO
extern uint8_t g_uc_mbo_switch;
#endif
extern uint8_t g_uc_dbac_dynamic_switch;

typedef struct {
    uint8_t uc_rs_nrates; /* 速率个数 */
    uint8_t auc_resv[NUM_3_BYTES];
    mac_data_rate_stru ast_rs_rates[WLAN_MAX_SUPP_RATES];
} mac_rateset_stru;

typedef struct {
    uint8_t uc_br_rate_num;  /* 基本速率个数 */
    uint8_t uc_nbr_rate_num; /* 非基本速率个数 */
    uint8_t uc_max_rate;     /* 最大基本速率 */
    uint8_t uc_min_rate;     /* 最小基本速率 */
    mac_rateset_stru st_rate;
} mac_curr_rateset_stru;

/* wme参数 */
typedef struct {
    uint32_t aifsn;      /* AIFSN parameters */
    uint32_t logcwmin;   /* cwmin in exponential form, 单位2^n -1 slot */
    uint32_t logcwmax;   /* cwmax in exponential form, 单位2^n -1 slot */
    uint32_t txop_limit; /* txopLimit, us */
} mac_wme_param_stru;

#ifdef _PRE_WLAN_FEATURE_TXBF
typedef struct {
    uint8_t bit_imbf_receive_cap : 1, /* 隐式TxBf接收能力 */
              bit_exp_comp_txbf_cap : 1,      /* 应用压缩矩阵进行TxBf的能力 */
              bit_min_grouping : 2,           /* 0=不分组，1=1,2分组，2=1,4分组，3=1,2,4分组 */
              bit_csi_bfee_max_rows : 2,      /* bfer支持的来自bfee的CSI显示反馈的最大行数 */
              bit_channel_est_cap : 2;        /* 信道估计的能力，0=1空时流，依次递增 */
    uint8_t auc_resv0[NUM_3_BYTES];
} mac_vap_txbf_add_stru;
#endif

typedef struct {
    uint8_t *puc_ie;        /* APP 信息元素 */
    uint32_t ie_len;     /* APP 信息元素长度 */
    uint32_t ie_max_len; /* APP 信息元素最大长度 */
} mac_app_ie_stru;


typedef struct {
    /*  MODE
    0: disable report of rssi change
    1: enable report when rssi lower than threshold(vowifi_low_thres)
    2: enable report when rssi higher than threshold(vowifi_high_thres)
    */
    uint8_t en_vowifi_mode;          /* 枚举mac_vowifi_mode */
    /* 【1，100】, the continuous counters of lower or higher than threshold which will trigger the report to host */
    uint8_t uc_trigger_count_thres;
    int8_t c_rssi_low_thres;        /* [-1, -100],vowifi_low_thres */
    int8_t c_rssi_high_thres;       /* [-1, -100],vowifi_high_thres */
    uint16_t us_rssi_period_ms;     /* 单位ms, 范围【1s，30s】, the period of monitor the RSSI when host suspended */

    oal_bool_enum_uint8 en_vowifi_reported; /* 标记vowifi是否上报过一次"状态切换申请"，避免多次上报 */
    /* 上层下发的配置vowifi参数的次数统计，用于辨别是否整套参数都下发齐全，防止下发参数流程和vowifi触发上报流程重叠 */
    uint8_t uc_cfg_cmd_cnt;
} mac_vowifi_param_stru;

#define MAC_VAP_INVAILD 0x0 /* 0为vap无效 */
#define MAC_VAP_VAILD   0x2b

#ifdef _PRE_WLAN_FEATURE_MBO
typedef struct {
    uint8_t uc_mbo_enable;    /* MBO特性定制化开关 */
    uint8_t uc_mbo_cell_capa; /* MBO STA移动数据能力 */
    uint8_t uc_mbo_assoc_disallowed_test_switch;
    uint8_t en_disable_connect;                                 /* 是否发送re-assoc req帧给对应BSS标志位 */
    uint32_t btm_req_received_time;                          /* 收到BTM Req帧的时间 */
    uint32_t re_assoc_delay_time;                            /* Neighbor元素中获得的re-assoc delay time ms */
    uint8_t auc_re_assoc_delay_bss_mac_addr[WLAN_MAC_ADDR_LEN]; /* 有re-assoc delay的BSS地址 */
    uint8_t auc_resv[NUM_2_BYTES];
} mac_mbo_para_stru;
#endif

/* VAP的数据结构 */
typedef struct {
    /* VAP为AP或者STA均有成员 */
    /* word0~word1 */
    uint8_t uc_vap_id; /* vap ID   */
    /* 即资源池索引值 */
    uint8_t uc_device_id;           /* 设备ID   */
    uint8_t uc_chip_id;                 /* chip ID  */
    wlan_vap_mode_enum_uint8 en_vap_mode; /* vap模式  */
    uint32_t core_id;

    /* word2~word3 */
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN]; /* BSSID，非MAC地址，MAC地址是mib中的auc_dot11StationID  */
    mac_vap_state_enum_uint8 en_vap_state;  /* VAP状态 */
    wlan_protocol_enum_uint8 en_protocol;   /* 工作的协议模式 */

    /* word4~word5 */
    mac_channel_stru st_channel; /* vap所在的信道 */
    mac_ch_switch_info_stru st_ch_switch_info;

    /* word6 */
    uint8_t bit_has_user_bw_limit : 1; /* 该vap是否存在user限速 */
    uint8_t bit_vap_bw_limit : 1;      /* 该vap是否已限速 */
    uint8_t bit_voice_aggr : 1;        /* 该vap是否针对VO业务支持聚合 */
    uint8_t bit_one_tx_tcp_be : 1;     /* 该vap是否只有1路发送TCP BE业务 */
    uint8_t bit_one_rx_tcp_be : 1;     /* 该vap是否只有1路接收TCP BE业务 */
    uint8_t bit_no_tcp_or_udp : 1;     /* 该vap没有跑TCP或UDP业务 */
    uint8_t bit_bw_fixed : 1;          /* AP模式的VAP的带宽是否固定20M */
    uint8_t bit_use_rts_threshold : 1; /* 该vap的RTS策略是否使用协议规定的RTS门限 */

    uint8_t uc_tx_power;                /* 传输功率, 单位dBm */
    uint8_t uc_channel_utilization;     /* 当前信道利用率 */
    /*
     * 初始为0，AP模式下，每跟新一次wmm参数这个变量加1,在beacon帧和assoc rsp中会填写，4bit，不能超过15；
     * STA模式下解析帧并更新这个值
     */
    uint8_t uc_wmm_params_update_count;

    /* word7 */
    uint16_t us_user_nums;                                        /* VAP下已挂接的用户个数 */
    uint16_t us_multi_user_idx;                                   /* 组播用户ID */
    uint8_t auc_cache_user_mac_addr[WLAN_MAC_ADDR_LEN];           /* cache user对应的MAC地址 */
    uint16_t us_cache_user_id;                                    /* cache user对应的userID */
    oal_dlist_head_stru ast_user_hash[MAC_VAP_USER_HASH_MAX_VALUE]; /* hash数组,使用HASH结构内的DLIST */
    oal_dlist_head_stru st_mac_user_list_head;                      /* 关联用户节点双向链表,使用USER结构内的DLIST */

    /* word8 */
    wlan_nss_enum_uint8 en_vap_rx_nss; /* vap的接收空间流个数 */
    /* vap为静态资源，标记VAP有没有被申请
      DMAC OFFLOAD模式VAP被删除后过滤缓冲的帧 */
    uint8_t uc_init_flag;
    uint16_t us_mgmt_frame_filters; /* 管理帧过滤，参考 en_fbt_mgmt_frame_filter_enum */

    mac_cap_flag_stru st_cap_flag;            /* vap能力特性标识 */
    wlan_mib_ieee802dot11_stru *pst_mib_info; /* mib信息(当时配置vap时，可以直接将指针值为NULL，节省空间)  */

    mac_curr_rateset_stru st_curr_sup_rates;                    /* 当前支持的速率集 */
    mac_curr_rateset_stru ast_sta_sup_rates_ie[WLAN_BAND_BUTT]; /* sta全信道扫描时使用，用于填写支持的速率集ie，分2.4和5G */

#ifdef _PRE_WLAN_DFT_STAT
    /* user 链表维测 */
    uint32_t dlist_cnt; /* dlsit统计 */
    uint32_t hash_cnt;  /* hash个数统计 */
#endif

#ifdef _PRE_WLAN_FEATURE_TXBF
    mac_vap_txbf_add_stru st_txbf_add_cap;
#endif

    /* VAP为AP或者STA均有成员 定义结束 */
    /* VAP为AP特有成员， 定义开始 */
    mac_protection_stru st_protection; /* 与保护相关变量 */
    mac_app_ie_stru ast_app_ie[OAL_APP_IE_NUM];
    oal_bool_enum_uint8 en_40M_intol_user; /* ap下是否有关联的40M intolerant的user */
    oal_bool_enum_uint8 en_vap_wmm;        /* ap wmm 开关 */

    uint8_t uc_random_mac;

    /* VAP为AP特定成员， 定义结束 */
    /* VAP为STA特有成员， 定义开始 */
    uint16_t us_sta_aid; /* VAP为STA模式时保存AP分配给STA的AID(从响应帧获取),取值范围1~2007; VAP为AP模式时，不用此成员变量 */
    uint16_t us_assoc_vap_id;        /* VAP为STA模式时保存user(ap)的资源池索引；VAP为AP模式时，不用此成员变量 */
    uint8_t uc_uapsd_cap;            /* 保存与STA关联的AP是否支持uapsd能力信息 */
    uint16_t us_assoc_user_cap_info; /* sta要关联的用户的能力信息 */
    uint8_t bit_ap_11ntxbf : 1,      /* sta要关联的用户的11n txbf能力信息 */
              bit_rsp_soft_ctl_launcher : 1,
              bit_roam_scan_valid_rslt : 1,
              bit_ap_chip_oui : 3, /* sta要关联的用户的芯片类型 */
              bit_mac_padding : 2; /* 11ax协商时mac padding配置值,用于WiFi6认证 */

#ifdef _PRE_WLAN_NARROW_BAND
    mac_cfg_narrow_bw_stru st_nb;
#endif

    /* 常发测试使用 */
    uint8_t bit_al_tx_flag : 1;   /* 常发标志 */
    uint8_t bit_payload_flag : 2; /* payload内容:0:全0  1:全1  2:random */
    uint8_t bit_ack_policy : 1;   /* ack_policy内容:0:normal 1:no ack , 目前不使用 */

    /* 常收测试使用 */
    uint8_t bit_al_rx_flag : 1;         /* 常收标志 */
    uint8_t bit_bss_include_rrm_ie : 1; /* 关联的bss 含有rrm ie */
    uint8_t bit_vap_support_csa : 1;
    uint8_t bit_reserved : 1;

    wlan_p2p_mode_enum_uint8 en_p2p_mode; /* 0:非P2P设备; 1:P2P_GO; 2:P2P_Device; 3:P2P_CL */
    uint8_t uc_p2p_gocl_hal_vap_id;     /* p2p go / cl的hal vap id */
    uint8_t uc_p2p_listen_channel;      /* P2P Listen channel */

    mac_cfg_uapsd_sta_stru st_sta_uapsd_cfg; /* UAPSD的配置信息 */

#ifdef _PRE_WLAN_FEATURE_PMF
    oal_bool_enum_uint8 en_user_pmf_cap; /* STA侧在未创建user前，存储目标user的pmf使能信息 */
#endif

    oal_spin_lock_stru st_cache_user_lock; /* cache_user lock */
    mac_vowifi_param_stru *pst_vowifi_cfg_param; /* 上层下发的"VoWiFi信号质量评估"参数结构体 */
    mac_fix_rate_cfg_para_stru st_fix_rate_pre_para;

#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
    uint8_t uc_he_mu_edca_update_count;
    uint8_t en_11ax_custom_switch : 1,
            bit_multi_bssid_custom_switch : 1,
            bit_htc_include_custom_switch : 1,
            bit_om_include_custom_switch : 1,
            bit_ofdma_aggr_num : 4;        /* 4*32以内，做amsdu，大于4*32，直接填8,做ampdu硬件聚合 */
    uint8_t en_11ax_hal_cap : 1,
            aput_bss_color_info : 7;
    uint8_t bit_om_auth_flag : 1,
            bit_om_auth_bw : 2,
            bit_om_auth_nss : 3,
            bit_om_auth_ul_mu_disable : 1,
            bit_rom_custom_switch : 1;

#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_uora_para_stru st_he_uora_eocw;
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
    oal_bool_enum_uint8 en_radar_detector_is_supp; /* 辅路hal device不支持雷达探测，增加判断 */
    uint8_t auc_resv10[NUM_3_BYTES];                       /* 4字节对齐 */
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    wlan_mib_Dot11EDCAEntry_stru st_wlan_mib_mu_edca;
    wlan_mid_Dot11HePhyCapability_rom_stru st_wlan_rom_mib_he_phy_config;
    wlan_mib_Dot11HEStationConfigEntry_rom_stru st_wlan_rom_mib_he_sta_config;
    mac_multi_bssid_info st_mbssid_info;
#endif
    uint32_t resv[NUM_10_BYTES];                              /* 由于下面结构体被ROM化,偏移不可变 */
    oal_bool_enum_uint8 en_dot11VHTExtendedNSSBWCapable; /* VHT cap Extended NSS BW Support字段是否有效 */
    mac_vap_state_enum_uint8 en_last_vap_state;          /* wlan0 监听场景下保存VAP 进入监听前的状态 */
    mac_probe_resp_ctrl_stru st_probe_resp_ctrl;
    uint8_t uc_ps_type;
    oal_bool_enum_uint8 en_ps_rx_amsdu;

#ifdef _PRE_WLAN_FEATURE_MBO
    mac_mbo_para_stru st_mbo_para_info;
#endif

#ifdef _PRE_WLAN_FEATURE_HIEX
    uint8_t uc_himit_enable;
    uint8_t uc_himit_set_htc;      /* 组帧时插入htc域，由配置命令标记 */
    uint8_t auc_resv[NUM_2_BYTES];
#endif
    uint32_t tb_aggr_bytes;        /* AP UL LENGTH指定的聚合字节数，单位：字节 */
    uint8_t is_primary_vap; /* 标记是否为wlan0 */
} mac_vap_stru;

extern uint8_t g_uc_uapsd_cap;

/* 1102 wiphy Vendor CMD参数 对应cfgid: WLAN_CFGID_VENDOR_CMD */
typedef struct mac_vendor_cmd_channel_list_info {
    uint8_t uc_channel_num_2g;
    uint8_t uc_channel_num_5g;
    uint8_t auc_channel_list_2g[MAC_CHANNEL_FREQ_2_BUTT];
    uint8_t auc_channel_list_5g[MAC_CHANNEL_FREQ_5_BUTT];
} mac_vendor_cmd_channel_list_stru; // only host

typedef struct {
    int8_t *pc_cmd_name;          /* 命令字符串 */
    uint8_t uc_is_check_para;     /* 是否检查获取的参数 */
    uint32_t bit;              /* 需要置位的命令 */
} wal_ftm_cmd_entry_stru; // only host

#ifdef _PRE_WLAN_FEATURE_PMF
typedef struct {
    oal_bool_enum_uint8 en_mac_mib_dot11RSNAMFPC;
    oal_bool_enum_uint8 en_mac_mib_dot11RSNAMFPR;
} mac_vap_pmf_mab_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
typedef struct {
    /* 定制化是否打开amsdu大包聚合 */
    uint8_t uc_host_large_amsdu_en;
    /* 当前聚合是否为amsdu聚合 */
    wlan_tx_amsdu_enum_uint8 en_tx_amsdu_level[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
    uint16_t us_amsdu_throughput_high;
    uint16_t us_amsdu_throughput_low;
    uint16_t us_amsdu_throughput_middle;
} mac_tx_large_amsdu_ampdu_stru; // only host
extern mac_tx_large_amsdu_ampdu_stru g_st_tx_large_amsdu;
#endif
#ifdef _PRE_WLAN_TCP_OPT
typedef struct {
    /* 定制化是否打开tcp ack过滤 */
    uint8_t uc_tcp_ack_filter_en;
    /* 当前状态 */
    uint8_t uc_cur_filter_status;
    uint16_t us_rx_filter_throughput_high;
    uint16_t us_rx_filter_throughput_low;
    uint16_t us_resv;
} mac_tcp_ack_filter_stru; // only host
extern mac_tcp_ack_filter_stru g_st_tcp_ack_filter;
#endif

typedef struct {
    /* 是否使能手动配置接收聚合个数 */
    oal_bool_enum_uint8 en_rx_ampdu_bitmap_cmd;
    /* 是否定制化使能配置接收聚合个数 */
    oal_bool_enum_uint8 en_rx_ampdu_bitmap_ini;
    /* addba rsp回复的聚合BAW SIZE */
    uint16_t us_rx_buffer_size;
} mac_rx_buffer_size_stru; // only host

typedef struct {
    /* 定制化小包amsdu开关 */
    uint8_t uc_ini_small_amsdu_en;
    uint8_t uc_cur_small_amsdu_en;
    uint16_t us_small_amsdu_throughput_high;
    uint16_t us_small_amsdu_throughput_low;
    uint16_t us_small_amsdu_pps_high;
    uint16_t us_small_amsdu_pps_low;
    uint16_t us_resv;
} mac_small_amsdu_switch_stru; // only host

typedef struct {
    uint8_t uc_ini_tcp_ack_buf_en;
    uint8_t uc_ini_tcp_ack_buf_userctl_test_en;
    uint8_t uc_cur_tcp_ack_buf_en[2]; // 2代表2个vap
    uint16_t us_tcp_ack_buf_throughput_high;
    uint16_t us_tcp_ack_buf_throughput_low;
    uint16_t us_tcp_ack_buf_throughput_high_40M;
    uint16_t us_tcp_ack_buf_throughput_low_40M;
    uint16_t us_tcp_ack_buf_throughput_high_80M;
    uint16_t us_tcp_ack_buf_throughput_low_80M;
    uint16_t us_tcp_ack_buf_throughput_high_160M;
    uint16_t us_tcp_ack_buf_throughput_low_160M;
    uint16_t us_tcp_ack_buf_userctl_high;
    uint16_t us_tcp_ack_buf_userctl_low;
    uint16_t us_tcp_ack_smooth_throughput;
} mac_tcp_ack_buf_switch_stru; // only host

typedef struct {
    uint8_t uc_ini_en;     /* 定制化根据吞吐动态bypass extLNA开关 */
    uint8_t uc_cur_status; /* 当前是否为低功耗测试状态 */
    uint16_t us_throughput_high;
    uint16_t us_throughput_low;
    uint16_t us_resv;
} mac_rx_dyn_bypass_extlna_stru; // only host

static inline uint8_t mac_is_primary_legacy_vap(mac_vap_stru *mac_vap)
{
    return mac_vap->is_primary_vap;
}

static inline uint8_t mac_is_primary_legacy_sta(mac_vap_stru *mac_vap)
{
    return mac_is_primary_legacy_vap(mac_vap) && IS_LEGACY_STA(mac_vap);
}

static inline uint8_t mac_is_secondary_sta(mac_vap_stru *mac_vap)
{
    return !mac_is_primary_legacy_vap(mac_vap) && IS_LEGACY_STA(mac_vap);
}

/* 10 函数声明 */
uint32_t mac_vap_init(mac_vap_stru *pst_vap, uint8_t uc_chip_id,
    uint8_t uc_device_id, uint8_t uc_vap_id, mac_cfg_add_vap_param_stru *pst_param);
void mac_vap_init_rates(mac_vap_stru *pst_vap);
void mac_sta_init_bss_rates(mac_vap_stru *pst_vap, mac_bss_dscr_stru *pst_bss_dscr);
void mac_vap_init_rates_by_protocol(mac_vap_stru *pst_vap,
    wlan_protocol_enum_uint8 en_vap_protocol, mac_data_rate_stru *pst_rates);
uint32_t mac_vap_exit(mac_vap_stru *pst_vap);
uint32_t mac_vap_del_user(mac_vap_stru *pst_vap, uint16_t us_user_idx);
uint32_t mac_vap_find_user_by_macaddr(mac_vap_stru *pst_vap,
    const unsigned char *puc_sta_mac_addr, uint16_t *pus_user_idx);
uint32_t mac_device_find_user_by_macaddr(uint8_t uc_device_id,
    const unsigned char *puc_sta_mac_addr, uint16_t *pus_user_idx);
uint32_t mac_chip_find_user_by_macaddr(uint8_t uc_chip_id,
    const unsigned char *puc_sta_mac_addr, uint16_t *pus_user_idx);
uint32_t mac_board_find_user_by_macaddr(const unsigned char *puc_sta_mac_addr, uint16_t *pus_user_idx);
uint32_t mac_vap_add_assoc_user(mac_vap_stru *pst_vap, uint16_t us_user_idx);

uint8_t mac_vap_get_bandwith(wlan_bw_cap_enum_uint8 en_dev_cap, wlan_channel_bandwidth_enum_uint8 en_bss_cap);
uint32_t mac_vap_set_current_channel(mac_vap_stru *pst_vap, wlan_channel_band_enum_uint8 en_band,
    uint8_t uc_channel, uint8_t is_6ghz);
void mac_vap_state_change(mac_vap_stru *pst_mac_vap, mac_vap_state_enum_uint8 en_vap_state);
uint32_t mac_vap_config_vht_ht_mib_by_protocol(mac_vap_stru *pst_mac_vap);
uint32_t mac_vap_init_wme_param(mac_vap_stru *pst_mac_vap);
#ifdef _PRE_WLAN_FEATURE_TXOPPS
uint8_t mac_vap_get_txopps(mac_vap_stru *pst_vap);
void mac_vap_set_txopps(mac_vap_stru *pst_vap, uint8_t uc_value);
void mac_vap_update_txopps(mac_vap_stru *pst_vap, mac_user_stru *pst_user);
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS
wlan_mib_mimo_power_save_enum mac_vap_get_smps_mode(mac_vap_stru *pst_mac_vap);
wlan_mib_mimo_power_save_enum mac_vap_get_smps_en(mac_vap_stru *pst_mac_vap);
void mac_vap_set_smps(mac_vap_stru *pst_vap, uint8_t uc_value);
#endif
uint32_t mac_vap_set_uapsd_en(mac_vap_stru *pst_mac_vap, uint8_t uc_value);
uint8_t mac_vap_get_uapsd_en(mac_vap_stru *pst_mac_vap);
uint32_t mac_vap_init_by_protocol(mac_vap_stru *pst_mac_vap, wlan_protocol_enum_uint8 en_protocol);

oal_bool_enum_uint8 mac_vap_check_bss_cap_info_phy_ap(uint16_t us_cap_info, mac_vap_stru *pst_mac_vap);
mac_wme_param_stru *mac_get_wmm_cfg(wlan_vap_mode_enum_uint8 en_vap_mode);
#ifdef _PRE_WLAN_FEATURE_EDCA_MULTI_USER_MULTI_AC
mac_wme_param_stru *mac_get_wmm_cfg_multi_user_multi_ac(oal_traffic_type_enum_uint8 en_vap_mode);
#endif
void mac_vap_get_bandwidth_cap(mac_vap_stru *pst_mac_vap, wlan_bw_cap_enum_uint8 *pen_cap);

wlan_bw_cap_enum_uint8 mac_vap_bw_mode_to_bw(wlan_channel_bandwidth_enum_uint8 en_mode);
void mac_vap_init_rx_nss_by_protocol(mac_vap_stru *pst_mac_vap);
uint8_t mac_vap_get_ap_usr_opern_bandwidth(mac_vap_stru *pst_mac_sta, mac_user_stru *pst_mac_user);
uint32_t mac_device_find_up_vap_ram(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap);
uint8_t mac_vap_set_bw_check(mac_vap_stru *pst_mac_sta,
    wlan_channel_bandwidth_enum_uint8 en_sta_new_bandwidth);
void mac_dec_p2p_num(mac_vap_stru *pst_vap);
void mac_inc_p2p_num(mac_vap_stru *pst_vap);
void mac_vap_set_p2p_mode(mac_vap_stru *pst_vap, wlan_p2p_mode_enum_uint8 en_p2p_mode);
wlan_p2p_mode_enum_uint8 mac_get_p2p_mode(mac_vap_stru *pst_vap);
void mac_vap_set_aid(mac_vap_stru *pst_vap, uint16_t us_aid);
void mac_vap_set_uapsd_cap(mac_vap_stru *pst_vap, uint8_t uc_uapsd_cap);
void mac_vap_set_assoc_id(mac_vap_stru *pst_vap, uint16_t us_assoc_vap_id);
void mac_vap_set_tx_power(mac_vap_stru *pst_vap, uint8_t uc_tx_power);
void mac_vap_set_al_tx_flag(mac_vap_stru *pst_vap, oal_bool_enum_uint8 en_flag);
void mac_vap_set_al_tx_payload_flag(mac_vap_stru *pst_vap, uint8_t uc_paylod);
uint32_t mac_dump_protection(mac_vap_stru *pst_mac_vap, uint8_t *puc_param);
void mac_vap_set_multi_user_idx(mac_vap_stru *pst_vap, uint16_t us_multi_user_idx);
void mac_vap_set_uapsd_para(mac_vap_stru *pst_mac_vap, mac_cfg_uapsd_sta_stru *pst_uapsd_info);
void mac_vap_set_wmm_params_update_count(mac_vap_stru *pst_vap, uint8_t uc_update_count);
void mac_vap_set_rifs_tx_on(mac_vap_stru *pst_vap, uint8_t uc_value);
/* 后续tdls功能可能会打开 */
void mac_vap_set_11ac2g(mac_vap_stru *pst_vap, uint8_t uc_value);
void mac_vap_set_hide_ssid(mac_vap_stru *pst_vap, uint8_t uc_value);
uint8_t mac_vap_get_peer_obss_scan(mac_vap_stru *pst_vap);
void mac_vap_set_peer_obss_scan(mac_vap_stru *pst_vap, uint8_t uc_value);
uint32_t mac_vap_clear_app_ie(mac_vap_stru *pst_mac_vap, en_app_ie_type_uint8 en_type);
uint32_t mac_vap_save_app_ie(mac_vap_stru *pst_mac_vap, oal_app_ie_stru *pst_app_ie, en_app_ie_type_uint8 en_type);
void mac_vap_set_rx_nss(mac_vap_stru *pst_vap, wlan_nss_enum_uint8 en_rx_nss);
uint32_t mac_vap_init_privacy(mac_vap_stru *pst_mac_vap, mac_conn_security_stru *pst_conn_sec);
mac_user_stru *mac_vap_get_user_by_addr(mac_vap_stru *pst_mac_vap, uint8_t *puc_mac_addr);
uint32_t mac_vap_set_security(mac_vap_stru *pst_mac_vap, mac_beacon_param_stru *pst_beacon_param);
uint32_t mac_vap_add_key(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user,
    uint8_t uc_key_id, mac_key_params_stru *pst_key);
uint8_t mac_vap_get_default_key_id(mac_vap_stru *pst_mac_vap);
uint32_t mac_vap_set_default_wep_key(mac_vap_stru *pst_mac_vap, uint8_t uc_key_index);
uint32_t mac_vap_set_default_mgmt_key(mac_vap_stru *pst_mac_vap, uint8_t uc_key_index);
void mac_vap_init_user_security_port(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
uint8_t *mac_vap_get_mac_addr(mac_vap_stru *pst_mac_vap);

oal_switch_enum_uint8 mac_vap_protection_autoprot_is_enabled(mac_vap_stru *pst_mac_vap);
wlan_prot_mode_enum_uint8 mac_vap_get_user_protection_mode(mac_vap_stru *pst_mac_vap_sta,
    mac_user_stru *pst_mac_user);
oal_bool_enum mac_protection_lsigtxop_check(mac_vap_stru *pst_mac_vap);
void mac_protection_set_lsig_txop_mechanism(mac_vap_stru *pst_mac_vap, oal_switch_enum_uint8 en_flag);
void mac_device_set_vap_id(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap, uint8_t uc_vap_idx,
    wlan_vap_mode_enum_uint8 en_vap_mode, wlan_p2p_mode_enum_uint8 en_p2p_mode, uint8_t is_add_vap);
uint32_t mac_device_find_up_vap(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap);
mac_vap_stru *mac_device_find_another_up_vap(mac_device_stru *pst_mac_device, uint8_t uc_vap_id_self);
uint32_t mac_device_find_up_ap(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap);
uint32_t mac_device_calc_up_vap_num(mac_device_stru *pst_mac_device);
uint32_t mac_device_calc_work_vap_num(mac_device_stru *pst_mac_device);
uint32_t mac_device_find_up_p2p_go(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap);
uint32_t mac_device_get_up_vap_num(mac_device_stru *pst_mac_device);
uint32_t mac_device_find_2up_vap(mac_device_stru *pst_mac_device,
    mac_vap_stru **ppst_mac_vap1, mac_vap_stru **ppst_mac_vap2);
uint32_t mac_fcs_dbac_state_check(mac_device_stru *pst_mac_device);
uint32_t mac_device_find_up_sta(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap);
uint32_t mac_device_is_p2p_connected(mac_device_stru *pst_mac_device);

#ifdef _PRE_WLAN_FEATURE_SMPS
uint32_t mac_device_find_smps_mode_en(mac_device_stru *pst_mac_device, wlan_mib_mimo_power_save_enum en_smps_mode);
#endif
void mac_device_set_channel(mac_device_stru *pst_mac_device, mac_cfg_channel_param_stru *pst_channel_param);
void mac_device_get_channel(mac_device_stru *pst_mac_device, mac_cfg_channel_param_stru *pst_channel_param);
uint32_t mac_fcs_get_prot_datarate(mac_vap_stru *pst_src_vap);

uint32_t mac_fcs_get_prot_mode(mac_vap_stru *pst_src_vap);
void mac_blacklist_free_pointer(mac_vap_stru *pst_mac_vap, mac_blacklist_info_stru *pst_blacklist_info);
void mac_vap_ini_get_nss_num(mac_device_stru *mac_device,
    wlan_nss_enum_uint8 *nss_num_rx, wlan_nss_enum_uint8 *nss_num_tx);
void mac_vap_init_legacy_rates(mac_vap_stru *pst_vap, mac_data_rate_stru *pst_rates);
void mac_vap_init_11n_rates(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev);
void mac_vap_init_11g_mixed_one_rates(mac_vap_stru *pst_vap, mac_data_rate_stru *pst_rates);
#ifdef _PRE_WLAN_FEATURE_11AX
void mac_vap_stru_init_11ax_rates(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev);
#endif

void mac_vap_init_11ac_rates(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev);
void mac_vap_init_11n_rates_extend(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev);
void mac_vap_init_11ac_mcs_singlenss(mac_vap_stru *pst_mac_vap, wlan_channel_bandwidth_enum_uint8 en_bandwidth);
uint32_t mac_vap_add_wep_key(mac_vap_stru *pst_mac_vap, uint8_t us_key_idx, uint8_t uc_key_len, uint8_t *puc_key);
oal_bool_enum_uint8 mac_vap_need_set_user_htc_cap_1103(mac_vap_stru *mac_vap);
oal_bool_enum_uint8 mac_vap_need_set_user_htc_cap_1106(mac_vap_stru *mac_vap);
oal_bool_enum_uint8 mac_get_rx_6g_flag_1106(dmac_rx_ctl_stru *rx_ctrl);
oal_bool_enum_uint8 mac_get_rx_6g_flag_1103(dmac_rx_ctl_stru *rx_ctrl);
void mac_vap_tx_data_set_user_htc_cap(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
mac_vap_stru *mac_vap_find_another_up_vap_by_mac_vap(mac_vap_stru *pst_vap);
void mac_vap_csa_support_set(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_cap);
oal_bool_enum_uint8 mac_vap_go_can_not_in_160m_check(mac_vap_stru *p_mac_vap, uint8_t vap_channel);
uint32_t mac_vap_set_cb_tx_user_idx(mac_vap_stru *pst_mac_vap,
    mac_tx_ctl_stru *pst_tx_ctl, const unsigned char *puc_data);
extern oal_bool_enum_uint8 mac_vap_can_not_start_he_protocol(mac_vap_stru *p_mac_vap);
extern oal_bool_enum_uint8 mac_vap_p2p_bw_back_to_40m(mac_vap_stru *p_mac_vap,
    int32_t channel,  wlan_channel_bandwidth_enum_uint8  *p_channlBandWidth);
extern oal_bool_enum_uint8 mac_vap_avoid_dbac_close_vht_protocol(mac_vap_stru *p_mac_vap);
mac_rx_dyn_bypass_extlna_stru *mac_vap_get_rx_dyn_bypass_extlna_switch(void);
mac_tcp_ack_buf_switch_stru *mac_vap_get_tcp_ack_buf_switch(void);
mac_small_amsdu_switch_stru *mac_vap_get_small_amsdu_switch(void);
mac_rx_buffer_size_stru *mac_vap_get_rx_buffer_size(void);
oal_bool_enum_uint8 mac_vap_need_proto_downgrade(mac_vap_stru *vap);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_vap.h */
