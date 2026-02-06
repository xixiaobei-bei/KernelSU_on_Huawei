

#ifndef __MAC_CFG_H__
#define __MAC_CFG_H__

/* 1 其他头文件包含 */
#include "wlan_customize.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_CFG_H

#define MAC_EXT_PA_GAIN_MAX_LVL    2     /* 定制化外置PA最大档位数 */
#define CUS_RF_PA_BIAS_REG_NUM     43    /* 定制化RF部分PA偏置寄存器最大数 */
#define CUS_DY_CALI_PARAMS_NUM     14    /* 动态校准参数个数,2.4g 3个(ofdm 20/40 11b cw),5g 5*2个band */
#define CUS_DY_CALI_PARAMS_TIMES   3     /* 动态校准参数二次项系数个数 */
#define CUS_DY_CALI_DPN_PARAMS_NUM 4     /* 定制化动态校准2.4G DPN参数个数11b OFDM20/40 CW OR 5G 160/80/40/20 */
#define CUS_DY_CALI_2G_VAL_DPN_MAX 50  /* 动态校准2g dpn读取nvram最大值 */
#define CUS_DY_CALI_2G_VAL_DPN_MIN (-50) /* 动态校准2g dpn读取nvram最小值 */
#define CUS_DY_CALI_5G_VAL_DPN_MAX 50  /* 动态校准5g dpn读取nvram最大值 */
#define CUS_DY_CALI_5G_VAL_DPN_MIN (-50) /* 动态校准5g dpn读取nvram最小值 */

/* CFG VAP h2d */
typedef struct {
    oal_net_device_stru *pst_net_dev;
} mac_cfg_vap_h2d_stru;


/* 接收帧的FCS统计信息 */
typedef struct {
    uint32_t data_op;    /* 数据操作模式:<0>保留,<1>清除 */
    uint32_t print_info; /* 打印数据内容:<0>所有数据 <1>总帧数 <2>self fcs correct, <3>other fcs correct, <4>fcs error  */
} mac_cfg_rx_fcs_info_stru;

/* 剔除用户配置命令参数 */
typedef struct {
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /* MAC地址 */
    uint16_t us_reason_code;                 /* 去关联 reason code */
} mac_cfg_kick_user_param_stru;

/* BA会话相关的配置命令参数 */
typedef struct {
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /* 用户的MAC ADDR */
    uint8_t uc_tidno;                        /* 对应的tid号 */
    mac_ba_policy_enum_uint8 en_ba_policy;     /* BA确认策略 */
    uint16_t us_buff_size;                   /* BA窗口的大小 */
    uint16_t us_timeout;                     /* BA会话的超时时间 */
} mac_cfg_addba_req_param_stru;

typedef struct {
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN];   /* 用户的MAC ADDR */
    uint8_t uc_tidno;                          /* 对应的tid号 */
    mac_delba_initiator_enum_uint8 en_direction; /* 删除ba会话的发起端 */
    mac_delba_trigger_enum_uint8 en_trigger;     /* 删除原因 */
} mac_cfg_delba_req_param_stru;

/* 获取当前管制域国家码字符配置命令结构体 */
typedef struct {
    int8_t ac_country[NUM_3_BYTES];
    uint8_t auc_resv[1];
} mac_cfg_get_country_stru;

/* query消息格式:2字节WID x N */
typedef struct {
    wlan_tidno_enum_uint8 en_tid;
    uint8_t uc_resv[NUM_3_BYTES];
} mac_cfg_get_tid_stru;

/* P2P 节能控制命令 */
typedef struct {
    uint8_t uc_p2p_statistics_ctrl; /* 0:清除P2P 统计值； 1:打印输出统计值 */
    uint8_t auc_rsv[NUM_3_BYTES];
} mac_cfg_p2p_stat_param_stru;

/* 设置以太网开关需要的参数 */
typedef struct {
    oam_ota_frame_direction_type_enum_uint8 en_frame_direction;
    oal_switch_enum_uint8 en_switch;
    uint8_t auc_user_macaddr[WLAN_MAC_ADDR_LEN];
} mac_cfg_eth_switch_param_stru; /* hd_event */

typedef struct {
    oal_bool_enum_uint8 en_rate_cfg_tag;
    oal_bool_enum_uint8 en_tx_ampdu_last;
    uint8_t uc_reserv[NUM_2_BYTES];
} mac_fix_rate_cfg_para_stru;

/* STA UAPSD 配置命令 */
/* 该结构体host与device共用,需保证结构体一致 */
typedef struct {
    uint8_t uc_max_sp_len;
    uint8_t uc_delivery_enabled[WLAN_WME_AC_BUTT];
    uint8_t uc_trigger_enabled[WLAN_WME_AC_BUTT];
} mac_cfg_uapsd_sta_stru;

#ifdef _PRE_WLAN_FEATURE_11R
#define MAC_MAX_FTE_LEN 257

typedef struct {
    uint16_t us_mdid; /* Mobile Domain ID */
    uint16_t us_len;  /* FTE 的长度 */
    uint8_t auc_ie[MAC_MAX_FTE_LEN];
} mac_cfg80211_ft_ies_stru; // only host
#endif  // _PRE_WLAN_FEATURE_11R


#ifdef _PRE_WLAN_DFT_STAT
typedef struct {
    uint8_t auc_user_macaddr[WLAN_MAC_ADDR_LEN];
    uint8_t uc_param;
    uint8_t uc_resv;
} mac_cfg_usr_queue_param_stru;
#endif

typedef struct {
    uint32_t coext_info;
    uint32_t channel_report;
} mac_cfg_set_2040_coexist_stru; // only host

typedef struct {
    uint8_t uc_scan_type;
    oal_bool_enum_uint8 en_current_bss_ignore;
    uint8_t auc_bssid[OAL_MAC_ADDR_LEN];
    oal_bool_enum_uint8 is_roaming_trigged_by_cmd; // 是否由命令触发的漫游
    uint8_t auc_resv[NUM_3_BYTES];
} mac_cfg_set_roam_start_stru; // only host

typedef struct {
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    uint8_t auc_resv0[NUM_2_BYTES];
    uint8_t auc_pmkid[WLAN_PMKID_LEN];
} mac_cfg_pmksa_param_stru; // only host

/* 定制化 power ref 2g 5g配置参数 */
/* customize rf cfg struct */
typedef struct {
    int8_t c_rf_gain_db_mult4; /* 外部PA/LNA bypass时的增益(精度0.25dB) */
    uint8_t auc_resv[NUM_3_BYTES];
} mac_cfg_gain_db_per_band;

typedef struct {
    /* 2g */
    mac_cfg_gain_db_per_band ac_gain_db_2g[MAC_NUM_2G_BAND];
    /* 5g */
    mac_cfg_gain_db_per_band ac_gain_db_5g[MAC_NUM_5G_BAND];
} mac_cfg_custom_gain_db_stru;

typedef struct {
    int8_t c_cfg_delta_pwr_ref_rssi_2g[CUS_NUM_2G_DELTA_RSSI_NUM];
    int8_t c_cfg_delta_pwr_ref_rssi_5g[CUS_NUM_5G_DELTA_RSSI_NUM];
} mac_cfg_custom_delta_pwr_ref_stru;

typedef struct {
    int8_t ac_cfg_delta_amend_rssi_2g[CUS_NUM_2G_AMEND_RSSI_NUM];
    int8_t c_rsv;
    int8_t ac_cfg_delta_amend_rssi_5g[CUS_NUM_5G_AMEND_RSSI_NUM];
} mac_cfg_custom_amend_rssi_stru;

typedef struct {
    int8_t filter_narrowing_amend_rssi_2g[CUS_2G_FILTER_NARROW_AMEND_RSSI_NUM];
    int8_t filter_narrowing_amend_rssi_5g[CUS_5G_FILTER_NARROW_AMEND_RSSI_NUM];
    int8_t ac_rsv[NUM_3_BYTES];
} mac_cfg_custom_filter_narrow_amend_rssi_stru;

typedef struct {
    uint16_t us_lna_on2off_time_ns; /* LNA开到LNA关的时间(ns) */
    uint16_t us_lna_off2on_time_ns; /* LNA关到LNA开的时间(ns) */

    int8_t c_lna_bypass_gain_db;   /* 外部LNA bypass时的增益(dB) */
    int8_t c_lna_gain_db;          /* 外部LNA增益(dB) */
    int8_t c_pa_gain_b0_db;        /* 外部PA b0 增益(dB) */
    uint8_t uc_ext_switch_isexist; /* 是否使用外部switch */

    uint8_t uc_ext_pa_isexist;                  /* 是否使用外部pa */
    oal_fem_lp_state_enum_uint8 en_fem_lp_enable; /* 是否支持fem低功耗标志 */
    int8_t c_fem_spec_value;                    /* fem spec功率点 */
    uint8_t uc_ext_lna_isexist;                 /* 是否使用外部lna */

    int8_t c_pa_gain_b1_db;     /* 外部PA b1增益(dB) */
    uint8_t uc_pa_gain_lvl_num; /* 外部PA 增益档位数 */
    uint8_t auc_resv[NUM_2_BYTES];
} mac_cfg_custom_ext_rf_stru;

typedef struct {
    mac_cfg_custom_gain_db_stru ast_rf_gain_db_rf[WLAN_RF_CHANNEL_NUMS];           /* 2.4g 5g 插损 */
    mac_cfg_custom_ext_rf_stru ast_ext_rf[WLAN_BAND_BUTT][WLAN_RF_CHANNEL_NUMS];   /* 2.4g 5g fem */
    mac_cfg_custom_delta_pwr_ref_stru ast_delta_pwr_ref_cfg[WLAN_RF_CHANNEL_NUMS]; /* delta_rssi */
    mac_cfg_custom_amend_rssi_stru ast_rssi_amend_cfg[WLAN_RF_CHANNEL_NUMS];       /* rssi_amend */

    uint8_t uc_far_dist_pow_gain_switch;           /* 超远距离功率增益开关 */
    uint8_t uc_far_dist_dsss_scale_promote_switch; /* 超远距11b 1m 2m dbb scale提升使能开关 */
    uint8_t uc_chn_radio_cap;
    int8_t c_delta_cca_ed_high_80th_5g;

    /* 注意，如果修改了对应的位置，需要同步修改函数: hal_config_custom_rf  */
    int8_t c_delta_cca_ed_high_20th_2g;
    int8_t c_delta_cca_ed_high_40th_2g;
    int8_t c_delta_cca_ed_high_20th_5g;
    int8_t c_delta_cca_ed_high_40th_5g;
    uint32_t aul_5g_upc_mix_gain_rf_reg[MAC_NUM_5G_BAND];
    uint32_t aul_2g_pa_bias_rf_reg[CUS_RF_PA_BIAS_REG_NUM]; /* 定制化RF部分PA偏置寄存器reg236~244, reg253~284 */
    mac_cfg_custom_filter_narrow_amend_rssi_stru filter_narrow_rssi_amend[WLAN_RF_CHANNEL_NUMS];
} mac_cfg_customize_rf; // only host

/* RF寄存器定制化结构体 */
typedef struct {
    uint16_t us_rf_reg117;
    uint16_t us_rf_reg123;
    uint16_t us_rf_reg124;
    uint16_t us_rf_reg125;
    uint16_t us_rf_reg126;
    uint8_t auc_resv[NUM_2_BYTES];
} mac_cus_dts_rf_reg;

/* FCC认证 参数结构体 */
typedef struct {
    uint8_t uc_index;       /* 下标表示偏移 */
    uint8_t uc_max_txpower; /* 最大发送功率 */
    uint16_t us_dbb_scale;  /* dbb scale */
} mac_cus_band_edge_limit_stru;

/* 定制化 校准配置参数 */
typedef struct {
    /* dts */
    int16_t aus_cali_txpwr_pa_dc_ref_2g_val_chan[MAC_2G_CHANNEL_NUM]; /* txpwr分信道ref值 */
    int16_t aus_cali_txpwr_pa_dc_ref_5g_val_band[MAC_NUM_5G_BAND];
} mac_cus_cali_stru;

/* 定制化 DPD校准配置参数 */
typedef struct {
    uint32_t aul_dpd_cali_cus_dts[MAC_DPD_CALI_CUS_PARAMS_NUM];
} mac_dpd_cus_cali_stru;

typedef struct {
    int32_t al_dy_cali_base_ratio_params[CUS_DY_CALI_PARAMS_NUM][CUS_DY_CALI_PARAMS_TIMES]; /* 产测定制化参数数组 */
    int32_t al_dy_cali_base_ratio_ppa_params[CUS_DY_CALI_PARAMS_TIMES]; /* ppa-pow定制化参数数组 */
    int8_t ac_dy_cali_2g_dpn_params[MAC_2G_CHANNEL_NUM][CUS_DY_CALI_DPN_PARAMS_NUM];
    int8_t ac_dy_cali_5g_dpn_params[MAC_NUM_5G_BAND][WLAN_BW_CAP_80PLUS80];
    int16_t as_extre_point_val[CUS_DY_CALI_NUM_5G_BAND];
    uint8_t uc_rf_id;
} mac_cus_dy_cali_param_stru;

typedef struct {
    mac_cus_cali_stru ast_cali[WLAN_RF_CHANNEL_NUMS];
    mac_dpd_cus_cali_stru ast_dpd_cali_para[WLAN_RF_CHANNEL_NUMS];
    uint16_t aus_dyn_cali_dscr_interval[WLAN_BAND_BUTT]; /* 动态校准开关2.4g 5g */
    int16_t gm0_db10_amend[WLAN_RF_CHANNEL_NUMS];

    oal_bool_enum_uint8 en_dyn_cali_opt_switch;
    int8_t uc_band_5g_enable;
    uint8_t uc_tone_amp_grade;
    uint8_t auc_resv[1];
} mac_cus_dts_cali_stru;

typedef struct {
    uint16_t us_tx_ratio;        /* tx占空比 */
    uint16_t us_tx_pwr_comp_val; /* 发射功率补偿值 */
} mac_tx_ratio_vs_pwr_stru;

/* 定制化TX占空比&温度补偿发射功率的参数 */
typedef struct {
    mac_tx_ratio_vs_pwr_stru ast_txratio2pwr[3]; /* 3档占空比分别对应功率补偿值 */
    uint32_t more_pwr;                      /* 根据温度额外补偿的发射功率 */
} mac_cfg_customize_tx_pwr_comp_stru;

/* 以下为解析内核配置参数转化为驱动内部参数下发的结构体 */
/* 解析内核配置的扫描参数后，下发给驱动的扫描参数 */
typedef struct {
    oal_ssids_stru st_ssids[WLAN_SCAN_REQ_MAX_SSID];
    int32_t l_ssid_num;

    uint8_t *puc_ie;
    uint32_t ie_len;

    oal_scan_enum_uint8 en_scan_type;
    uint8_t num_channels_2g;
    uint8_t num_channels_5g;
    uint8_t num_channels_6g;

    uint32_t *channels_2g;
    uint32_t *channels_5g;
    uint32_t *channels_6g;
    uint32_t scan_flag;

    /* WLAN/P2P 特性情况下，p2p0 和p2p-p2p0 cl 扫描时候，需要使用不同设备，增加bit_is_p2p0_scan来区分 */
    uint8_t bit_is_p2p0_scan : 1; /* 是否为p2p0 发起扫描 */
    uint8_t bit_rsv : 7;          /* 保留位 */
    uint8_t auc_rsv[NUM_3_BYTES];           /* 保留位 */
} mac_cfg80211_scan_param_stru;

#endif /* end of mac_cfg.h */
