
#ifndef __MAC_VAP_COMMON_H__
#define __MAC_VAP_COMMON_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "mac_ftm_common.h"

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define MAC_STR_CMD_MAX_SIZE 16

#define MAC_FCS_DBAC_IGNORE     0 /* 不是DBAC场景 */
#define MAC_FCS_DBAC_NEED_CLOSE 1 /* DBAC需要关闭 */
#define MAC_FCS_DBAC_NEED_OPEN  2 /* DBAC需要开启 */

#define MAC_NUM_DR_802_11A  8 /* 11A 5g模式时的数据速率(DR)个数 */
#define MAC_NUM_BR_802_11A  3 /* 11A 5g模式时的基本速率(BR)个数 */
#define MAC_NUM_NBR_802_11A 5 /* 11A 5g模式时的非基本速率(NBR)个数 */

#define MAC_NUM_DR_802_11B  4 /* 11B 2.4G模式时的数据速率(DR)个数 */
#define MAC_NUM_BR_802_11B  2 /* 11B 2.4G模式时的数据速率(BR)个数 */
#define MAC_NUM_NBR_802_11B 2 /* 11B 2.4G模式时的数据速率(NBR)个数 */

#define MAC_NUM_DR_802_11G  8 /* 11G 2.4G模式时的数据速率(DR)个数 */
#define MAC_NUM_BR_802_11G  3 /* 11G 2.4G模式时的基本速率(BR)个数 */
#define MAC_NUM_NBR_802_11G 5 /* 11G 2.4G模式时的非基本速率(NBR)个数 */

#define MAC_NUM_DR_802_11G_MIXED      12 /* 11G 混合模式时的数据速率(DR)个数 */
#define MAC_NUM_BR_802_11G_MIXED_ONE  4  /* 11G 混合1模式时的基本速率(BR)个数 */
#define MAC_NUM_NBR_802_11G_MIXED_ONE 8  /* 11G 混合1模式时的非基本速率(NBR)个数 */

#define MAC_NUM_BR_802_11G_MIXED_TWO  7 /* 11G 混合2模式时的基本速率(BR)个数 */
#define MAC_NUM_NBR_802_11G_MIXED_TWO 5 /* 11G 混合2模式时的非基本速率(NBR)个数 */

/* 11N MCS相关的内容 */
#define MAC_MAX_RATE_SINGLE_NSS_20M_11N 0 /* 1个空间流20MHz的最大速率 */
#define MAC_MAX_RATE_SINGLE_NSS_40M_11N 0 /* 1个空间流40MHz的最大速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_20M_11N 0 /* 2个空间流20MHz的最大速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_40M_11N 0 /* 2个空间流40MHz的最大速率 */
#define MAC_MAX_RATE_TRIPLE_NSS_20M_11N 0 /* 3个空间流20MHz的最大速率 */
#define MAC_MAX_RATE_TRIPLE_NSS_40M_11N 0 /* 3个空间流40MHz的最大速率 */
#define MAC_MAX_RATE_FOUR_NSS_20M_11N   0 /* 4个空间流20MHz的最大速率 */
#define MAC_MAX_RATE_FOUR_NSS_40M_11N   0 /* 4个空间流40MHz的最大速率 */

/* 11AC MCS相关的内容 */
#define MAC_MAX_SUP_MCS7_11AC_EACH_NSS  0 /* 11AC各空间流支持的最大MCS序号，支持0-7 */
#define MAC_MAX_SUP_MCS8_11AC_EACH_NSS  1 /* 11AC各空间流支持的最大MCS序号，支持0-8 */
#define MAC_MAX_SUP_MCS9_11AC_EACH_NSS  2 /* 11AC各空间流支持的最大MCS序号，支持0-9 */
#define MAC_MAX_UNSUP_MCS_11AC_EACH_NSS 3 /* 11AC各空间流支持的最大MCS序号，不支持n个空间流 */

#ifdef _PRE_WLAN_FEATURE_1024QAM
#define MAC_MAX_RATE_SINGLE_NSS_20M_11AC  107  /* 1个空间流20MHz的最大Long GI速率 */
#define MAC_MAX_RATE_SINGLE_NSS_40M_11AC  225  /* 1个空间流40MHz的最大Long GI速率 */
#define MAC_MAX_RATE_SINGLE_NSS_80M_11AC  487  /* 1个空间流80MHz的最大Long GI速率 */
#define MAC_MAX_RATE_SINGLE_NSS_160M_11AC 975  /* 1个空间流160MHz的最大Long GI速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_20M_11AC  216  /* 2个空间流20MHz的最大Long GI速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_40M_11AC  450  /* 2个空间流40MHz的最大Long GI速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_80M_11AC  975  /* 2个空间流80MHz的最大Long GI速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_160M_11AC 1950 /* 2个空间流160MHz的最大Long GI速率 */
#define MAC_MAX_RATE_TRIPLE_NSS_20M_11AC  325  /* 3个空间流20MHz的最大Long GI速率 */
#define MAC_MAX_RATE_TRIPLE_NSS_40M_11AC  675  /* 3个空间流40MHz的最大Long GI速率 */
#define MAC_MAX_RATE_TRIPLE_NSS_80M_11AC  1462  /* 3个空间流80MHz的最大Long GI速率 */
#define MAC_MAX_RATE_TRIPLE_NSS_160M_11AC 2925 /* 3个空间流160MHz的最大Long GI速率 */
#define MAC_MAX_RATE_FOUR_NSS_20M_11AC  433  /* 4个空间流20MHz的最大Long GI速率 */
#define MAC_MAX_RATE_FOUR_NSS_40M_11AC  900  /* 4个空间流40MHz的最大Long GI速率 */
#define MAC_MAX_RATE_FOUR_NSS_80M_11AC  1950  /* 4个空间流80MHz的最大Long GI速率 */
#define MAC_MAX_RATE_FOUR_NSS_160M_11AC 3900 /* 4个空间流160MHz的最大Long GI速率 */
#else
/* 按照协议要求(9.4.2.158.3章节)，修改为long gi速率 */
#define MAC_MAX_RATE_SINGLE_NSS_20M_11AC  86   /* 1个空间流20MHz的最大速率 */
#define MAC_MAX_RATE_SINGLE_NSS_40M_11AC  180  /* 1个空间流40MHz的最大速率 */
#define MAC_MAX_RATE_SINGLE_NSS_80M_11AC  390  /* 1个空间流80MHz的最大速率 */
#define MAC_MAX_RATE_SINGLE_NSS_160M_11AC 780  /* 1个空间流160MHz的最大速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_20M_11AC  173  /* 2个空间流20MHz的最大速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_40M_11AC  360  /* 2个空间流40MHz的最大速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_80M_11AC  780  /* 2个空间流80MHz的最大速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_160M_11AC 1560 /* 2个空间流160MHz的最大速率 */
#define MAC_MAX_RATE_TRIPLE_NSS_20M_11AC  260  /* 3个空间流20MHz的最大Long GI速率 */
#define MAC_MAX_RATE_TRIPLE_NSS_40M_11AC  540  /* 3个空间流40MHz的最大Long GI速率 */
#define MAC_MAX_RATE_TRIPLE_NSS_80M_11AC  1170  /* 3个空间流80MHz的最大Long GI速率 */
#define MAC_MAX_RATE_TRIPLE_NSS_160M_11AC 2106 /* 3个空间流160MHz的最大Long GI速率 */
#define MAC_MAX_RATE_FOUR_NSS_20M_11AC  312  /* 4个空间流20MHz的最大Long GI速率 */
#define MAC_MAX_RATE_FOUR_NSS_40M_11AC  720  /* 4个空间流40MHz的最大Long GI速率 */
#define MAC_MAX_RATE_FOUR_NSS_80M_11AC  1560  /* 4个空间流80MHz的最大Long GI速率 */
#define MAC_MAX_RATE_FOUR_NSS_160M_11AC 3120 /* 4个空间流160MHz的最大Long GI速率 */
#endif

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    MAC_SET_BEACON = 0,
    MAC_ADD_BEACON = 1,

    MAC_BEACON_OPERATION_BUTT
} mac_beacon_operation_type;
typedef uint8_t mac_beacon_operation_type_uint8;

typedef enum {
    AMPDU_SWITCH_BY_DEL_BA = 0,
    AMPDU_SWITCH_BY_BA_LUT,

    AMPDU_SWITCH_MODE_BUTT
} mac_ampdu_switch_mode;
typedef uint8_t mac_ampdu_switch_mode_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
typedef enum {
    MAC_TCP_ACK_BUF_ENABLE,
    MAC_TCP_ACK_BUF_TIMEOUT,
    MAC_TCP_ACK_BUF_MAX,

    MAC_TCP_ACK_BUF_TYPE_BUTT
} mac_tcp_ack_buf_cfg_cmd_enum;
typedef uint8_t mac_tcp_ack_buf_cfg_cmd_enum_uint8;
#endif

typedef enum {
    MAC_RSSI_LIMIT_SHOW_INFO,
    MAC_RSSI_LIMIT_ENABLE,
    MAC_RSSI_LIMIT_DELTA,
    MAC_RSSI_LIMIT_THRESHOLD,

    MAC_RSSI_LIMIT_TYPE_BUTT
} mac_rssi_limit_type_enum;
typedef uint8_t mac_rssi_limit_type_enum_uint8;

/* 功率设置维测命令类型 */
typedef enum {
    MAC_SET_POW_RF_REG_CTL = 0, /* 功率是否RF寄存器控 */
    MAC_SET_POW_FIX_LEVEL,      /* 固定功率等级 */
    MAC_SET_POW_MAG_LEVEL,      /* 管理帧功率等级 */
    MAC_SET_POW_CTL_LEVEL,      /* 控制帧功率等级 */
    MAC_SET_POW_AMEND,          /* 修正UPC code */
    MAC_SET_POW_NO_MARGIN,      /* 功率不留余量 */
    MAC_SET_POW_SHOW_LOG,       /* 日志显示 */
    MAC_SET_POW_SAR_LVL_DEBUG,  /* 降sar  */
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    MAC_TAS_POW_CTRL_DEBUG,  /* TAS降功率  */
    MAC_TAS_RSSI_CALI_DEBUG, /* TAS天线测量  */
    MAC_TAS_ANT_SWITCH,      /* TAS天线切换  */
#endif
    MAC_SHOW_TPC_TABLE_GAIN,
    MAC_POW_SAVE,
    MAC_POW_GET_PD_INFO,
    MAC_POW_SET_TPC_IDX,

    MAC_SET_POW_BUTT
} mac_set_pow_type_enum;
typedef uint8_t mac_set_pow_type_enum_uint8;

typedef enum {
    MAC_VAP_CONFIG_UCAST_DATA = 0,
    MAC_VAP_CONFIG_MCAST_DATA,
    MAC_VAP_CONFIG_BCAST_DATA,
    MAC_VAP_CONFIG_UCAST_MGMT_2G,
    MAC_VAP_CONFIG_UCAST_MGMT_5G,
    MAC_VAP_CONFIG_MBCAST_MGMT_2G,
    MAC_VAP_CONFIG_MBCAST_MGMT_5G,
    MAC_VAP_CONFIG_BUTT,
} mac_vap_config_dscr_frame_type_enum;
typedef uint8_t mac_vap_config_dscr_frame_type_uint8;

typedef enum { /* hi1102-cb */
    SHORTGI_20_CFG_ENUM,
    SHORTGI_40_CFG_ENUM,
    SHORTGI_80_CFG_ENUM,
    SHORTGI_BUTT_CFG
} short_gi_cfg_type;

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    uint8_t uc_announced_channel;                           /* 新信道号 */
    wlan_channel_bandwidth_enum_uint8 en_announced_bandwidth; /* 新带宽模式 */
    uint8_t uc_ch_switch_cnt;                               /* 信道切换计数 */
    wlan_ch_switch_status_enum_uint8 en_ch_switch_status;     /* 信道切换状态 */
    wlan_bw_switch_status_enum_uint8 en_bw_switch_status;     /* 带宽切换状态 */
    oal_bool_enum_uint8 en_csa_present_in_bcn;                /* Beacon帧中是否包含CSA IE */
    wlan_csa_mode_tx_enum_uint8 en_csa_mode;
    uint8_t uc_start_chan_idx;
    uint8_t uc_end_chan_idx;
    wlan_channel_bandwidth_enum_uint8 en_user_pref_bandwidth;

    /* VAP为STA时，特有成员
     *
     *  ---|--------|--------------------|-----------------|-----------
     *     0        3                    0                 0
     *     X        A                    B                 C
     *
     *  sta的信道切换可通过上图帮助理解, 数字为切换计数器，
     *  X->A A之前为未发生任务信道切换时,切换计数器为0
     *  从A->B时间段为sta等待切换状态: en_waiting_to_shift_channel为true
     *  从B->C为sta信道切换中,即等待ap加beacon状态: en_waiting_for_ap为true
     *  C-> 为sta收到了ap的beacon，标准信道切换结束
     *
     *  A点通常中收到csa ie(beacon/action...), B点通常为tbtt中断中切换计数器变为
     *  0或者csa ie中计数器为0，C点则为收到beacon
     *
     *  从A->C的过程中，会过滤重复收到的csa ie或者信道切换动作
     *
     */
    uint8_t uc_new_channel;                           /* 可以考虑跟上面合并 */
    wlan_channel_bandwidth_enum_uint8 en_new_bandwidth; /* 可以考虑跟上面合并 */
    oal_bool_enum_uint8 en_waiting_to_shift_channel;    /* 等待切换信道 */
    uint8_t auc_rsv2[1];
    oal_bool_enum_uint8 en_te_b;
    uint8_t bit_wait_bw_change;        /* 收到action帧,等待切换带宽 */
    uint8_t uc_rsv1[NUM_2_BYTES];       /* ap上一次发送的切换个数 */
    uint8_t uc_linkloss_change_chanel; /* 可以考虑跟上面合并 */
    wlan_linkloss_scan_switch_chan_enum_uint8 en_linkloss_scan_switch_chan;
    uint32_t chan_report_for_te_a;
    mac_channel_stru st_old_channel; /* 信道切换时保存切换前信道信息 */
    uint8_t _rom[NUM_4_BYTES];
} mac_ch_switch_info_stru;

#ifdef _PRE_WLAN_FEATURE_TWT
/* TWT参数 */
typedef struct {
    uint64_t ull_twt_start_time;
    uint64_t ull_twt_interval;
    uint32_t twt_duration; /* Nominal Minimum TWT Wake Duration */
    uint8_t uc_twt_flow_id;
    uint8_t uc_twt_announce_bit;
    uint8_t uc_twt_session_enable;
    uint8_t uc_twt_is_trigger_enabled;
    uint8_t uc_next_twt_size;
    uint8_t uc_twt_dialog_token;
    uint8_t twt_type; /* 0为单播 1为广播 */
    uint8_t twt_idx; /* 使用vap下第几套资源 */
} mac_cfg_twt_stru;
#endif

/* CHR2.0使用的STA统计信息 */
typedef struct {
    uint8_t uc_distance;      /* 算法的tpc距离，对应dmac_alg_tpc_user_distance_enum */
    uint8_t uc_cca_intr;      /* 算法的cca_intr干扰，对应alg_cca_opt_intf_enum */
    int8_t c_snr_ant0;        /* 天线0上报的SNR值 */
    int8_t c_snr_ant1;        /* 天线1上报的SNR值 */
    uint32_t bcn_cnt;      /* 收到的beacon计数 */
    uint32_t bcn_tout_cnt; /* beacon丢失的计数 */
} station_info_extend_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/
typedef enum mac_rate_info_flags {
    MAC_RATE_INFO_FLAGS_MCS = BIT(0),
    MAC_RATE_INFO_FLAGS_VHT_MCS = BIT(1),
    MAC_RATE_INFO_FLAGS_40_MHZ_WIDTH = BIT(2),
    MAC_RATE_INFO_FLAGS_80_MHZ_WIDTH = BIT(3),
    MAC_RATE_INFO_FLAGS_SHORT_MIDDLE = BIT(4),
    MAC_RATE_INFO_FLAGS_160_MHZ_WIDTH = BIT(5),
    MAC_RATE_INFO_FLAGS_SHORT_GI = BIT(6),
    MAC_RATE_INFO_FLAGS_HE_MCS = BIT(7),
} mac_rate_info_flags_enum;

typedef enum {
    MAC_WMM_SET_PARAM_TYPE_DEFAULT,
    MAC_WMM_SET_PARAM_TYPE_UPDATE_EDCA,

    MAC_WMM_SET_PARAM_TYPE_BUTT
} mac_wmm_set_param_type_enum;
typedef uint8_t mac_wmm_set_param_type_enum_uint8;

typedef struct {
    /* word 0 */
    wlan_prot_mode_enum_uint8 en_protection_mode; /* 保护模式 */
    uint8_t uc_obss_non_erp_aging_cnt;          /* 指示OBSS中non erp 站点的老化时间 */
    uint8_t uc_obss_non_ht_aging_cnt;           /* 指示OBSS中non ht 站点的老化时间 */
    uint8_t bit_auto_protection : 1;            /* 指示保护策略是否开启，OAL_SWITCH_ON 打开， OAL_SWITCH_OFF 关闭 */
    uint8_t bit_obss_non_erp_present : 1;       /* 指示obss中是否存在non ERP的站点 */
    uint8_t bit_obss_non_ht_present : 1;        /* 指示obss中是否存在non HT的站点 */
    uint8_t bit_rts_cts_protect_mode : 1;       /* 指rts_cts 保护机制是否打开, OAL_SWITCH_ON 打开， OAL_SWITCH_OFF 关闭 */
    uint8_t bit_lsig_txop_protect_mode : 1;     /* 指示L-SIG protect是否打开, OAL_SWITCH_ON 打开， OAL_SWITCH_OFF 关闭 */
    uint8_t bit_reserved : 3;

    /* word 1 */
    uint8_t uc_sta_no_short_slot_num;     /* 不支持short slot的STA个数 */
    uint8_t uc_sta_no_short_preamble_num; /* 不支持short preamble的STA个数 */
    uint8_t uc_sta_non_erp_num;           /* 不支持ERP的STA个数 */
    uint8_t uc_sta_non_ht_num;            /* 不支持HT的STA个数 */
    /* word 2 */
    uint8_t uc_sta_non_gf_num;        /* 支持ERP/HT,不支持GF的STA个数 */
    uint8_t uc_sta_20M_only_num;      /* 只支持20M 频段的STA个数 */
    uint8_t uc_sta_no_40dsss_cck_num; /* 不用40M DSSS-CCK STA个数  */
    uint8_t uc_sta_no_lsig_txop_num;  /* 不支持L-SIG TXOP Protection STA个数 */
} mac_protection_stru;

/* VoWiFi信号质量评估 的 配置参数结构体 */
typedef enum {
    VOWIFI_DISABLE_REPORT = 0,
    VOWIFI_LOW_THRES_REPORT,
    VOWIFI_HIGH_THRES_REPORT,
    VOWIFI_CLOSE_REPORT = 3, /* 关闭VoWIFI */

    VOWIFI_MODE_BUTT = 3
} mac_vowifi_mode;

/*
 * enum nl80211_mfp - Management frame protection state
 * @NL80211_MFP_NO: Management frame protection not used
 * @NL80211_MFP_REQUIRED: Management frame protection required
 */
typedef enum {
    MAC_NL80211_MFP_NO,
    MAC_NL80211_MFP_REQUIRED,

    MAC_NL80211_MFP_BUTT
} mac_nl80211_mfp_enum;
typedef uint8_t mac_nl80211_mfp_enum_uint8;

/* 设置80211单播帧开关需要的参数 */
typedef struct {
    uint8_t bit_msdu_dscr : 1,
              bit_ba_info : 1,
              bit_himit_dscr : 1,
              bit_mu_dscr : 1,
              bit_rsv : 4;
} oam_sub_switch_stru;

typedef union {
    uint8_t value;
    oam_sub_switch_stru stru;
} oam_sub_switch_union;

typedef struct {
    uint8_t en_frame_direction;
    uint8_t en_frame_type;
    oal_switch_enum_uint8 en_frame_switch;
    oal_switch_enum_uint8 en_cb_switch;
    oal_switch_enum_uint8 en_dscr_switch;
    oam_sub_switch_union sub_switch;
    uint8_t auc_user_macaddr[WLAN_MAC_ADDR_LEN];
} mac_cfg_80211_ucast_switch_stru; /* hd_event */

/* 设置80211组播\广播帧开关需要的参数 */
typedef struct {
    uint8_t en_frame_direction;
    uint8_t en_frame_type;
    oal_switch_enum_uint8 en_frame_switch;
    oal_switch_enum_uint8 en_cb_switch;
    oal_switch_enum_uint8 en_dscr_switch;
    oam_sub_switch_union  sub_switch;
    uint8_t auc_resv[NUM_2_BYTES];
} mac_cfg_80211_mcast_switch_stru;

/* 设置probe request和probe response开关需要的参数 */
typedef struct {
    uint8_t en_frame_direction;
    oal_switch_enum_uint8 en_frame_switch;
    oal_switch_enum_uint8 en_cb_switch;
    oal_switch_enum_uint8 en_dscr_switch;
    oam_sub_switch_union  sub_switch;
    uint8_t             resv[NUM_3_BYTES];
} mac_cfg_probe_switch_stru;

typedef enum {
    MAC_LOG_ENCAP_AMPDU,
    MAC_LOG_SET_ALG_OTA,
    MAC_LOG_SET_TEMP_PROTECT_LOG,
    MAC_LOG_SET_HIMIT_PROBE_ENABLE,
    MAC_LOG_SET_HIMIT_HTC,
    MAC_LOG_MONITOR_OTA_RPT

} mac_debug_log_type_enum;

#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
typedef struct {
    uint8_t uc_hid2d_debug_en;
    uint8_t uc_hid2d_delay_time;
    uint16_t us_resv;
} mac_hid2d_debug_switch_stru;
extern mac_hid2d_debug_switch_stru g_st_hid2d_debug_switch;
#endif

typedef struct {
    wlan_data_collect_enum_uint8 uc_type;
    uint32_t reg_num;
} mac_data_collect_cfg_stru;
extern mac_data_collect_cfg_stru g_st_data_collect_cfg;

typedef struct {
    uint8_t cac_channel;
    uint8_t cac_bandwidth;
} mac_cfg_cac_stru;

#endif /* end of mac_vap_common.h */
