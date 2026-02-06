

#ifndef __MAC_DEVICE_H__
#define __MAC_DEVICE_H__

/* 1 其他头文件包含 */
#include "frw_ext_if.h"
#include "wlan_mib.h"
#include "mac_regdomain.h"
#include "mac_frame.h"
#include "mac_device_common.h"
#include "mac_dfs.h"
#include "hd_event.h"
#include "hd_command.h"
#ifdef _PRE_WLAN_FEATURE_HIEX
#include "hiex_msg.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_DEVICE_H

#define MAC_NET_DEVICE_NAME_LENGTH 16
#define WLAN_USER_MAX_SUPP_RATES 16 /* 用于记录对端设备支持的速率最大个数 */

#define MAC_SCAN_CHANNEL_INTERVAL_PERFORMANCE     2   /* 间隔2个信道，切回工作信道工作一段时间 */
#define MAC_WORK_TIME_ON_HOME_CHANNEL_PERFORMANCE 60  /* WLAN未关联，P2P关联，返回工作信道工作的时间 */
#define MAC_SCAN_CHANNEL_INTERVAL_HIDDEN_SSID     3   /* 携带隐藏SSID的背景扫描，间隔3个信道回工作信道工作一段时间 */

#define MAC_DEVICE_GET_CAP_BW(_pst_device)     ((_pst_device)->st_device_cap.en_channel_width)
#define MAC_DEVICE_GET_NSS_NUM(_pst_device)    ((_pst_device)->st_device_cap.en_nss_num)
#define MAC_DEVICE_GET_CAP_LDPC(_pst_device)   ((_pst_device)->st_device_cap.en_ldpc_is_supp)
#define MAC_DEVICE_GET_CAP_TXSTBC(_pst_device) ((_pst_device)->st_device_cap.en_tx_stbc_is_supp)
#define MAC_DEVICE_GET_CAP_RXSTBC(_pst_device) ((_pst_device)->st_device_cap.en_rx_stbc_is_supp)
#define MAC_DEVICE_GET_CAP_SUBFER(_pst_device) ((_pst_device)->st_device_cap.en_su_bfmer_is_supp)
#define MAC_DEVICE_GET_CAP_SUBFEE(_pst_device) ((_pst_device)->st_device_cap.en_su_bfmee_is_supp)
#define MAC_DEVICE_GET_CAP_MUBFER(_pst_device) ((_pst_device)->st_device_cap.en_mu_bfmer_is_supp)
#define MAC_DEVICE_GET_CAP_MUBFEE(_pst_device) ((_pst_device)->st_device_cap.en_mu_bfmee_is_supp)
#ifdef _PRE_WLAN_FEATURE_SMPS
#define MAC_DEVICE_GET_MODE_SMPS(_pst_device) ((_pst_device)->en_mac_smps_mode)
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
#define MAC_M2S_CALI_NSS_FROM_SMPS_MODE(en_smps_mode) \
    (((en_smps_mode) == WLAN_MIB_MIMO_POWER_SAVE_STATIC) ? WLAN_SINGLE_NSS : g_pst_mac_device_capability[0].en_nss_num)
#endif
typedef enum {
    MAC_DEVICE_CAP_DISABLE = 0,
    MAC_DEVICE_CAP_2G = BIT0, /* 当前已经不支持2G能力开关 */
    MAC_DEVICE_CAP_5G = BIT1,
    MAC_DEVICE_CAP_6G_LOW_BAND = BIT2, /* 欧盟6G频段 */
    MAC_DEVICE_CAP_6G_ALL_BAND = BIT3, /* 美国6G频段 */
    MAC_DEVICE_CAP_BUTT,
} mac_device_radio_cap_enum;
typedef uint8_t mac_device_radio_cap_enum_uint8;
/* btcoex黑名单类型 */
typedef enum {
    MAC_BTCOEX_BLACKLIST_LEV0 = BIT0, /* 0级条件，还需要其他判断，比如看mac地址 */
    MAC_BTCOEX_BLACKLIST_LEV1 = BIT1, /* 一级条件，不看mac地址 */
    MAC_BTCOEX_BLACKLIST_BUTT,
} mac_btcoex_blacklist_enum;
typedef uint8_t mac_btcoex_blacklist_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_APF
typedef struct {
    mac_apf_cmd_type_uint8 en_cmd_type;
    uint16_t us_program_len;
    uint8_t *puc_program;
} mac_apf_filter_cmd_stru;
#endif


typedef enum {
    MAC_P2P_SCENES_LOW_LATENCY   = 0, /* 低时延场景 */
    MAC_P2P_SCENES_HI_THROUGHPUT = 1, /* 高吞吐场景,先阶段只适用hishare */

    AC_P2P_SCENES_BUTT
} mac_p2p_scenes_enum;
typedef uint8_t mac_p2p_scenes_enum_uint8;

typedef struct {
    uint8_t uc_p2p_device_num;                /* 当前device下的P2P_DEVICE数量 */
    uint8_t uc_p2p_goclient_num;              /* 当前device下的P2P_CL/P2P_GO数量 */
    uint8_t uc_p2p0_vap_idx;                  /* P2P 共存场景下，P2P_DEV(p2p0) 指针 */
    mac_vap_state_enum_uint8 en_last_vap_state; /* P2P0/P2P_CL 共用VAP 结构，监听场景下保存VAP 进入监听前的状态 */
    mac_p2p_scenes_enum_uint8 p2p_scenes;       /* p2p 业务场景:低时延、高吞吐 */
    uint8_t uc_resv[1];                       /* 保留 */
    uint8_t en_roc_need_switch;               /* remain on channel后需要切回原信道 */
    uint8_t en_p2p_ps_pause;                  /* P2P 节能是否处于pause状态 */
    oal_net_device_stru *pst_p2p_net_device;    /* P2P 共存场景下主net_device(p2p0) 指针 */
    uint64_t ull_send_action_id;              /* P2P action id/cookie */
    uint64_t ull_last_roc_id;
    oal_ieee80211_channel_stru st_listen_channel;
    oal_nl80211_channel_type en_listen_channel_type;
    oal_net_device_stru *pst_primary_net_device; /* P2P 共存场景下主net_device(wlan0) 指针 */
    oal_net_device_stru *pst_second_net_device;  /* 信道跟随增加,后续不使用可以删除 */
} mac_p2p_info_stru;

typedef struct {
    uint16_t us_num_networks; /* 记录当前信道下扫描到的BSS个数 */
    uint8_t auc_resv[NUM_2_BYTES];
    uint8_t auc_bssid_array[WLAN_MAX_SCAN_BSS_PER_CH][WLAN_MAC_ADDR_LEN]; /* 记录当前信道下扫描到的所有BSSID */
} mac_bss_id_list_stru;


#define MAX_PNO_REPEAT_TIMES    4
#define PNO_SCHED_SCAN_INTERVAL (60 * 1000)

/* 扫描到的BSS描述结构体 */
typedef struct {
    /* 基本信息 */
    uint8_t en_bss_type; /* wlan_mib_desired_bsstype_enum bss网络类型 */
    uint8_t uc_dtim_period;                        /* dtime周期 */
    uint8_t uc_dtim_cnt;                           /* dtime cnt */
    oal_bool_enum_uint8 en_11ntxbf;                  /* 11n txbf */
    oal_bool_enum_uint8 en_new_scan_bss;             /* 是否是新扫描到的BSS */
    wlan_ap_chip_oui_enum_uint8 en_is_tplink_oui;
    int8_t c_rssi;                                    /* bss的信号强度 */
    int8_t ac_ssid[WLAN_SSID_MAX_LEN];                /* 网络ssid */
    uint16_t us_beacon_period;                        /* beacon周期 */
    uint16_t us_cap_info;                             /* 基本能力信息 */
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN];          /* 基础型网络 mac地址与bssid相同 */
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];             /* 网络bssid */
    mac_channel_stru st_channel;                        /* bss所在的信道 */
    uint8_t uc_wmm_cap;                               /* 是否支持wmm */
    uint8_t uc_uapsd_cap;                             /* 是否支持uapsd */
    oal_bool_enum_uint8 en_desired;                     /* 标志位，此bss是否是期望的 */
    uint8_t uc_num_supp_rates;                        /* 支持的速率集个数 */
    uint8_t auc_supp_rates[WLAN_USER_MAX_SUPP_RATES]; /* 支持的速率集 */

#ifdef _PRE_WLAN_FEATURE_11D
    int8_t ac_country[WLAN_COUNTRY_STR_LEN]; /* 国家字符串 */
    uint8_t auc_resv2[1];
    uint8_t *puc_country_ie; /* 用于存放country ie */
#endif

    /* 安全相关的信息 */
    uint8_t *puc_rsn_ie; /* 用于存放beacon、probe rsp的rsn ie */
    uint8_t *puc_wpa_ie; /* 用于存放beacon、probe rsp的wpa ie */

    /* 11n 11ac信息 */
    oal_bool_enum_uint8 en_ht_capable;                      /* 是否支持ht */
    oal_bool_enum_uint8 en_vht_capable;                     /* 是否支持vht */
    oal_bool_enum_uint8 en_vendor_vht_capable;              /* 是否支持hidden vendor vht */
    wlan_bw_cap_enum_uint8 en_bw_cap;                       /* 支持的带宽 0-20M 1-40M */
    wlan_channel_bandwidth_enum_uint8 en_channel_bandwidth; /* 信道带宽 */
    uint8_t uc_coex_mgmt_supp;                            /* 是否支持共存管理 */
    oal_bool_enum_uint8 en_ht_ldpc;                         /* 是否支持ldpc */
    oal_bool_enum_uint8 en_ht_stbc;                         /* 是否支持stbc */
    uint8_t uc_wapi;
    uint8_t en_vendor_novht_capable; /* 私有vendor中不需再携带 */
    oal_bool_enum_uint8 en_atheros_chip_oui;
    oal_bool_enum_uint8 en_vendor_1024qam_capable;          /* 私有vendor中是否携带1024QAM使能 */

    mac_btcoex_blacklist_enum_uint8 en_btcoex_blacklist_chip_oui; /* ps机制one pkt帧类型需要修订为self-cts等 */
    oal_bool_enum_uint8 en_punctured_preamble; /* 是否支持punctured preamble技术 */
    uint32_t timestamp; /* 更新此bss的时间戳 */

#ifdef _PRE_WLAN_FEATURE_M2S
    wlan_nss_enum_uint8 en_support_max_nss; /* 该AP支持的最大空间流数 */
    oal_bool_enum_uint8 en_support_opmode;  /* 该AP是否支持OPMODE */
    uint8_t uc_num_sounding_dim;          /* 该AP发送txbf的天线数 */
#endif

    oal_bool_enum_uint8 en_he_capable; /* 是否支持11ax */
    oal_bool_enum_uint8 en_he_uora;
    oal_bool_enum_uint8 en_dcm_support;
    mac_p2p_scenes_enum_uint8 en_p2p_scenes;

    oal_bool_enum_uint8 en_support_rrm; /* 是否支持RRM */
    oal_bool_enum_uint8 en_support_neighbor; /* 是否支持Neighbor report */

#ifdef _PRE_WLAN_FEATURE_1024QAM
    oal_bool_enum_uint8 en_support_1024qam;
#endif

#ifdef _PRE_WLAN_NARROW_BAND
    oal_bool_enum_uint8 en_nb_capable; /* 是否支持nb */
#endif
    oal_bool_enum_uint8 en_roam_blacklist_chip_oui; /* 不支持roam */
    oal_bool_enum_uint8 en_txbf_blacklist_chip_oui; /* 不支持txbf */
    mac_sap_mode_enum_uint8 sap_mode;
    uint8_t resv4[NUM_1_BYTES];

    int8_t rssi[HD_EVENT_RF_NUM]; /* 4天线的rssi */
    int8_t snr[HD_EVENT_RF_NUM];  /* 4天线的snr */
    uint8_t  ht_opern_ccsf2; /* 保存ht operation的ccsf2,用于带宽计算 */
    uint8_t  supported_channel_width : 2, /* vht cap能力，用于带宽计算 */
             extend_nss_bw_supp      : 2, /* vht cap能力，用于带宽计算 */
             resv                    : 4;

#ifdef _PRE_WLAN_FEATURE_MBO
    uint8_t uc_bss_assoc_disallowed; /* MBO IE中指示AP是否允许关联 */
#endif
    /* 管理帧信息 */
    uint32_t mgmt_len; /* 管理帧的长度 */

    /* multi_bssid 相关 */
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_multi_bssid_info st_mbssid_info;
#endif
    uint64_t wpa_rpt_time;                                 /* 扫描结果上报supplicant时间 */
    uint8_t auc_mgmt_buff[MAC_80211_QOS_HTC_4ADDR_FRAME_LEN]; /* 记录beacon帧或probe rsp帧 */
    /* 不要在管理帧数组后添加元素，请添加在管理帧长度之前 */
    uint8_t auc_mgmt_frame_body[NUM_4_BYTES]; /* *puc_mgmt_buff --- 记录beacon帧或probe rsp帧 */
} mac_bss_dscr_stru; // only host

typedef struct {
    /* 支持2*2 */                            /* 支持MU-MIMO */
    wlan_nss_enum_uint8 en_nss_num;          /* device Nss 空间流最大个数 */
    wlan_bw_cap_enum_uint8 en_channel_width; /* 支持的带宽 */
    oal_bool_enum_uint8 en_nb_is_supp;       /* 支持窄带 */
    oal_bool_enum_uint8 en_1024qam_is_supp;  /* 支持1024QAM速率 */

    oal_bool_enum_uint8 en_80211_mc_is_supp; /* 支持80211 mc */
    oal_bool_enum_uint8 en_ldpc_is_supp;     /* 是否支持接收LDPC编码的包 */
    oal_bool_enum_uint8 en_tx_stbc_is_supp;  /* 是否支持最少2x1 STBC发送 */
    oal_bool_enum_uint8 en_rx_stbc_is_supp;  /* 是否支持stbc接收,支持2个空间流 */

    oal_bool_enum_uint8 en_su_bfmer_is_supp; /* 是否支持单用户beamformer */
    oal_bool_enum_uint8 en_su_bfmee_is_supp; /* 是否支持单用户beamformee */
    oal_bool_enum_uint8 en_mu_bfmer_is_supp; /* 是否支持多用户beamformer */
    oal_bool_enum_uint8 en_mu_bfmee_is_supp; /* 是否支持多用户beamformee */
    uint8_t en_11ax_switch : 1,            /* 11ax开关 */
              bit_multi_bssid_switch : 1,          /* mbssid 开关 */
              bit_11ax_rsv : 6;
    uint8_t _rom[NUM_3_BYTES];
    mac_hisi_priv_cap_union hisi_priv_cap;
} mac_device_capability_stru;

typedef struct {
    oal_bool_enum_uint8 en_11k;
    oal_bool_enum_uint8 en_11v;
    oal_bool_enum_uint8 en_11r;
    oal_bool_enum_uint8 en_11r_ds;
    oal_bool_enum_uint8 en_adaptive11r;
    oal_bool_enum_uint8 en_nb_rpt_11k;
    uint8_t auc_rsv[NUM_2_BYTES];
} mac_device_voe_custom_stru;

#ifdef _PRE_WLAN_FEATURE_11AX
typedef struct {
    uint8_t   bit_htc_include : 1, /* 数据帧包含htc 头,默认初始化值未全 F */
                bit_om_in_data  : 1, /* 每个发送数据帧中携带om字段 */
                bit_rom_cap_switch : 1,
                bit_11ax_aput_switch  : 1,  /* 11ax aput开关 */
                bit_ignore_non_he_cap_from_beacon : 1, /* he sta 不处理beacon 中的non he cap ie 开关 */
                bit_11ax_aput_he_cap_switch : 1, /* 11ax aput携带he能力开关 */
                bit_twt_responder_support : 1, /* 单播twt resp能力开关 */
                bit_twt_requester_support : 1; /* 单播twt req能力开关 */
    uint8_t   bit_btwt_requester_support : 1, /* 广播twt req能力开关 */
                bit_flex_twt_support  : 1, /* flex twt能力开关 */
                bit_resv : 6; /* 保留6bit */
} mac_device_11ax_custom_stru;
#endif

typedef struct {
    mac_device_voe_custom_stru st_voe_custom_cfg;
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_device_11ax_custom_stru st_11ax_custom_cfg;
#endif
    uint16_t us_cmd_auth_rsp_timeout;
    uint8_t  bit_forbit_open_auth : 1,
               bit_rsv              : 7;
    uint8_t  auc_rsv[1];
} mac_device_custom_cfg_stru;

#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
typedef struct {
    oal_bool_enum_uint8 mcast_ampdu_enable; /* 是否开启组播聚合 */
    uint8_t mcast_ampdu_bitmap; /* 支持的组播聚合模式 */
    uint8_t mcast_ampdu_retry; /* 组播聚合帧重传次数 */
    uint8_t resv;
} mac_mcast_ampdu_cfg_stru;

enum mcast_ampdu_feature {
    MCAST_AMPDU_APUT = 0,
    MCAST_AMPDU_STAUT = 1,
    MCAST_AMPDU_BUTT,
};
#endif

/* device结构体 */
typedef struct {
    /* device下的业务vap，此处只记录VAP ID */
    uint32_t core_id;
    uint8_t auc_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE];
    uint8_t uc_cfg_vap_id;               /* 配置vap ID */
    uint8_t uc_device_id;                /* 芯片ID */
    uint8_t uc_chip_id;                  /* 设备ID */
    uint8_t uc_device_reset_in_progress; /* 复位处理中 */

    oal_bool_enum_uint8 en_device_state; /* 标识是否已经被分配，(OAL_TRUE初始化完成，OAL_FALSE未初始化 ) */
    uint8_t uc_vap_num;                /* 当前device下的业务VAP数量(AP+STA) */
    uint8_t uc_sta_num;                /* 当前device下的STA数量 */
    /* begin: P2P */
    mac_p2p_info_stru st_p2p_info; /* P2P 相关信息 */
    /* end: P2P */
    uint8_t auc_hw_addr[WLAN_MAC_ADDR_LEN]; /* 从eeprom或flash获得的mac地址，ko加载时调用hal接口赋值 */
    /* device级别参数 */
    uint8_t uc_max_channel;                 /* 已配置VAP的信道号，其后的VAP配置值不能与此值矛盾，仅在非DBAC时使用 */
    wlan_channel_band_enum_uint8 en_max_band; /* 已配置VAP的频段，其后的VAP配置值不能与此值矛盾，仅在非DBAC时使用 */

    oal_bool_enum_uint8 en_wmm; /* wmm使能开关 */
    wlan_tidno_enum_uint8 en_tid;
    uint8_t en_reset_switch; /* 是否使能复位功能 */
    uint8_t uc_csa_vap_cnt;  /* 每个running AP发送一次CSA帧,该计数减1,减到零后,AP停止当前硬件发送,准备开始切换信道 */

    uint32_t beacon_interval; /* device级别beacon interval,device下所有VAP约束为同一值 */

    uint8_t uc_auth_req_sendst;
    uint8_t uc_asoc_req_sendst;

#ifdef _PRE_WLAN_FEATURE_HIEX
    mac_hiex_cap_stru st_hiex_cap;
    uint32_t hiex_debug_switch;
#else
    uint32_t resv1;
    uint32_t resv2;
#endif
    uint32_t resv3;

    /* device能力 */
    wlan_protocol_cap_enum_uint8 en_protocol_cap; /* 协议能力 */
    wlan_band_cap_enum_uint8 en_band_cap;         /* 频段能力 */
    /* 已配置VAP的最带带宽值，其后的VAP配置值不能与此值矛盾，仅在非DBAC时使用 */
    wlan_channel_bandwidth_enum_uint8 en_max_bandwidth;

    int16_t s_upc_amend; /* UPC修正值 */

    uint16_t us_device_reset_num; /* 复位的次数统计 */

    mac_data_rate_stru st_mac_rates_11g[MAC_DATARATES_PHY_80211G_NUM]; /* 11g速率 */

    mac_scan_req_stru st_scan_params;                      /* 最新一次的扫描参数信息 */
    frw_timeout_stru st_obss_scan_timer;                   /* obss扫描定时器，循环定时器 */
    mac_channel_stru st_p2p_vap_channel;                   /* p2p listen时记录p2p的信道，用于p2p listen结束后恢复 */

    oal_bool_enum_uint8 en_2040bss_switch; /* 20/40 bss检测开关 */
    uint8_t uc_in_suspend;

    /* linux内核中的device物理信息 */
    /* 用于存放和VAP相关的wiphy设备信息，在AP/STA模式下均要使用；可以多个VAP对应一个wiphy */
    oal_wiphy_stru *pst_wiphy;
    mac_bss_id_list_stru st_bss_id_list; /* 当前信道下的扫描结果 */

    uint8_t uc_mac_vap_id; /* 多vap共存时，保存睡眠前的mac vap id */
    oal_bool_enum_uint8 en_dbac_enabled;
    oal_bool_enum_uint8 en_dbac_running;       /* DBAC是否在运行 */
    oal_bool_enum_uint8 en_dbac_has_vip_frame; /* 标记DBAC运行时收到了关键帧 */

    uint8_t uc_arpoffload_switch;
    uint8_t uc_wapi;
#ifdef _PRE_WLAN_FEATURE_MONITOR
    uint8_t uc_monitor_ota_mode;
#else
    uint8_t uc_reserve;
#endif
    oal_bool_enum_uint8 en_is_random_mac_addr_scan; /* 随机mac扫描开关,从hmac下发 */

    uint8_t auc_mac_oui[WLAN_RANDOM_MAC_OUI_LEN]; /* 随机mac地址OUI,由系统下发 */
    oal_bool_enum_uint8 en_dbdc_running;            /* DBDC是否在运行 */

    mac_device_capability_stru st_device_cap; /* device的部分能力，包括定制化 */

#ifdef _PRE_WLAN_FEATURE_SMPS
    /* SMPS是MAC的能力，需要device上所有的VAP都支持SMPS才会开启MAC的SMPS能力 */
    /* 记录当前MAC工作的SMPS能力(放在mac侧，是因为hmac也会用于做判断) */
    uint8_t en_mac_smps_mode; /* wlan_mib_mimo_power_save_enum */
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
    mac_dfs_core_stru st_dfs;
#endif
    mac_ap_ch_info_stru st_ap_channel_list[MAC_MAX_SUPP_CHANNEL];

    /* 针对Host的成员，待移动到hmac_device */
    oal_bool_enum_uint8 en_vap_classify; /* 是否使能基于vap的业务分类 */
    uint8_t uc_ap_chan_idx;            /* 当前扫描信道索引 */

    oal_bool_enum_uint8 en_40MHz_intol_bit_recd;
    uint8_t uc_ftm_vap_id; /* ftm中断对应 vap ID */

    frw_timeout_stru st_send_frame; /* send frame定时器 */

    uint8_t is_ready_to_get_scan_result;
    mac_channel_stru st_best_chan_for_hid2d;
    /* 发布会场景标志位: 0表示非发布会模式，1表示发布会模式 */
    uint8_t is_presentation_mode;
    /* 双sta模式标记，wlan1对应vap创建时置true，删除时置false */
    uint8_t is_dual_sta_mode;
    uint8_t dc_status;  /* host标记是否DC状态 */
} mac_device_stru;

#ifdef _PRE_WLAN_FEATURE_HID2D
typedef struct {
    uint8_t link_meas_cmd_type;
    uint8_t scan_chan;
    uint16_t scan_interval;
    uint16_t scan_time;
} mac_hid2d_link_meas_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_HIEX
#define MAC_BAND_GET_HIEX_CAP(band)     (&(band)->st_hiex_cap)
#define MAC_BAND_HIEX_ENABLED(band)     ((band)->st_hiex_cap.bit_hiex_enable)
#define MAC_BAND_HIMIT_ENABLED(band)    ((band)->st_hiex_cap.bit_himit_enable)
#define MAC_BAND_ERSRU_ENABLED(band)    ((band)->st_hiex_cap.bit_ersru_enable)
#endif

typedef struct {
    uint8_t auc_tx_ba_index_table[MAC_TX_BA_LUT_BMAP_LEN];   /* 发送端BA LUT表位图 */
    uint8_t auc_rx_ba_lut_idx_table[MAC_RX_BA_LUT_BMAP_LEN]; /* 接收端BA LUT表位图 */
    uint8_t auc_ra_lut_index_table[WLAN_ACTIVE_USER_IDX_BMAP_LEN]; /* 关联用户 LUT表位图 */
} mac_lut_table_stru;

/* chip结构体 */
typedef struct {
    uint8_t auc_device_id[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP]; /* CHIP下挂的DEV，仅记录对应的ID索引值 */
    uint8_t uc_device_nums;                                      /* chip下device的数目 */
    uint8_t uc_chip_id;                                          /* 芯片ID */
    /* 标识是否已初始化，OAL_TRUE初始化完成，OAL_FALSE未初始化 */
    oal_bool_enum_uint8 en_chip_state;
    uint32_t chip_ver;              /* 芯片版本 */
    mac_lut_table_stru st_lut_table;     /* 软件维护LUT表资源的结构体 */
    void *p_alg_priv;                /* chip级别算法私有结构体 */

    /* 用户相关成员变量 */
    frw_timeout_stru st_active_user_timer; /* 用户活跃定时器 */
    uint8_t uc_assoc_user_cnt;  /* 关联用户数 */
    uint8_t uc_active_user_cnt; /* 活跃用户数 */
} mac_chip_stru;

typedef enum {
    MAC_RX_IP_FILTER_STOPED = 0,   // 功能关闭，未使能、或者其他状况不允许过滤动作。
    MAC_RX_IP_FILTER_WORKING = 1,  // 功能打开，按照规则正常过滤
    MAC_RX_IP_FILTER_BUTT
} mac_ip_filter_state_enum;
typedef uint8_t mac_ip_filter_state_enum_uint8;

typedef struct {
    mac_ip_filter_state_enum_uint8 en_state;  // 功能状态：过滤、非过滤等
    uint8_t uc_btable_items_num;            // 黑名单中目前存储的items个数
    uint8_t uc_btable_size;                 // 黑名单大小，表示最多存储的items个数
    uint8_t uc_resv;
    mac_ip_filter_item_stru *pst_filter_btable;  // 黑名单指针
} mac_rx_ip_filter_struc;

/* board结构体 */
typedef struct {
    mac_chip_stru ast_chip[WLAN_CHIP_MAX_NUM_PER_BOARD]; /* board挂接的芯片 */
    uint8_t uc_chip_id_bitmap;                         /* 标识chip是否被分配的位图 */
    uint8_t auc_resv[NUM_3_BYTES];                               /* 字节对齐 */
    mac_rx_ip_filter_struc st_rx_ip_filter; /* rx ip过滤功能的管理结构体 */
    mac_rx_icmp_filter_struc st_rx_icmp_filter;
} mac_board_stru;

typedef struct {
    mac_device_stru *pst_mac_device;
} mac_wiphy_priv_stru;

/* 黑名单 */
typedef struct {
    uint8_t auc_mac_addr[OAL_MAC_ADDR_LEN]; /* mac地址          */
    uint8_t auc_reserved[NUM_2_BYTES];                /* 字节对齐         */
    uint32_t cfg_time;                   /* 加入黑名单的时间 */
    uint32_t aging_time;                 /* 老化时间         */
    uint32_t drop_counter;               /* 报文丢弃统计     */
} mac_blacklist_stru;

/* 自动黑名单 */
typedef struct {
    uint8_t auc_mac_addr[OAL_MAC_ADDR_LEN]; /* mac地址  */
    uint8_t auc_reserved[NUM_2_BYTES];                /* 字节对齐 */
    uint32_t cfg_time;                   /* 初始时间 */
    uint32_t asso_counter;               /* 关联计数 */
} mac_autoblacklist_stru;

/* 2.9.12 私有安全增强 */
#define WLAN_BLACKLIST_MAX 32

/* 自动黑名单信息 */
typedef struct {
    uint8_t uc_enabled;                                          /* 使能标志 0:未使能  1:使能 */
    uint8_t list_num;                                            /* 有多少个自动黑名单        */
    uint8_t auc_reserved[NUM_2_BYTES];                                     /* 字节对齐                  */
    uint32_t threshold;                                       /* 门限                      */
    uint32_t reset_time;                                      /* 重置时间                  */
    uint32_t aging_time;                                      /* 老化时间                  */
    mac_autoblacklist_stru ast_autoblack_list[WLAN_BLACKLIST_MAX]; /* 自动黑名单表              */
} mac_autoblacklist_info_stru;

/* 黑白名单信息 */
typedef struct {
    uint8_t uc_mode;                                     /* 黑白名单模式   */
    uint8_t uc_list_num;                                 /* 名单数         */
    uint8_t uc_blacklist_vap_index;                      /* 黑名单vap index */
    uint8_t uc_blacklist_device_index;                   /* 黑名单device index */
    mac_autoblacklist_info_stru st_autoblacklist_info;     /* 自动黑名单信息 */
    mac_blacklist_stru ast_black_list[WLAN_BLACKLIST_MAX]; /* 有效黑白名单表 */
} mac_blacklist_info_stru;

typedef struct {
    uint32_t                          ao_drop_cnt;
    uint32_t                          ao_send_rsp_cnt;
    uint32_t                          apf_flt_drop_cnt;
    uint32_t                          icmp_flt_drop_cnt;
}mac_psm_flt_stat_stru; // only device
typedef struct {
    uint32_t                          ps_short_idle_cnt;
    uint32_t                          ps_long_idle_cnt;
}mac_psm_fastsleep_stat_stru; // only device

#ifdef _PRE_WLAN_FEATURE_PSM_ABN_PKTS_STAT
typedef struct {
    uint32_t small_amsdu_total_cnt; // 小包amsdu总数
    uint32_t large_amsdu_total_cnt; // 大包amsdu总数
    uint32_t small_amsdu_mcast_ucast_cnt; // 小包amsdu组播单播混杂数
} mac_psm_abnormal_stat_stru;
#endif

/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 主逻辑中不想看到宏 */
#define MAC_DBAC_ENABLE(_pst_device) ((_pst_device)->en_dbac_enabled == OAL_TRUE)

extern mac_device_capability_stru *g_pst_mac_device_capability;

#ifdef _PRE_WLAN_FEATURE_WMMAC
extern oal_bool_enum_uint8 g_en_wmmac_switch;
#endif

extern mac_board_stru *g_pst_mac_board;


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_dbac_enabled(mac_device_stru *pst_device)
{
    return pst_device->en_dbac_enabled;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_dbac_running(mac_device_stru *pst_device)
{
    if (pst_device->en_dbac_enabled == OAL_FALSE) {
        return OAL_FALSE;
    }

    return pst_device->en_dbac_running;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_dbdc_running(mac_device_stru *pst_mac_device)
{
    return pst_mac_device->en_dbdc_running;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_get_2040bss_switch(mac_device_stru *pst_mac_device)
{
    return pst_mac_device->en_2040bss_switch;
}
OAL_STATIC OAL_INLINE void mac_set_2040bss_switch(mac_device_stru *pst_mac_device, oal_bool_enum_uint8 en_switch)
{
    pst_mac_device->en_2040bss_switch = en_switch;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_chip_run_band(uint8_t uc_chip_id, wlan_channel_band_enum_uint8 en_band)
{
    /*
     * 判断指定芯片是否可以运行在指定BAND：
     *     -双芯片时各芯片只运行在指定的BAND。若后续有双芯片四频，修改此处
     *     -单芯片双频时可以运行在两个BAND
     *     -单芯片单频时只可以运行在宏定义指定的BAND
     *     -note:目前所有witp wifi芯片均支持双频，若后续有单频芯片，需要增加诸如
     *      plat_chip_supp_band(chip_id, band)的接口，并在此处额外判断
     */
    if ((en_band != WLAN_BAND_2G) && (en_band != WLAN_BAND_5G)) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

uint32_t mac_device_init(mac_device_stru *pst_mac_device, uint32_t chip_ver,
    uint8_t chip_id, uint8_t uc_device_id);
uint32_t mac_chip_init(mac_chip_stru *pst_chip, uint8_t uc_device_max);
uint32_t mac_board_init(void);

uint32_t mac_device_exit(mac_device_stru *pst_device);
uint32_t mac_chip_exit(mac_board_stru *pst_board, mac_chip_stru *pst_chip);
uint32_t mac_board_exit(mac_board_stru *pst_board);

/* 10.2 公共成员访问部分 */
void mac_chip_inc_assoc_user(mac_chip_stru *pst_mac_chip);
void mac_chip_dec_assoc_user(mac_chip_stru *pst_mac_chip);

void mac_device_set_dfr_reset(mac_device_stru *pst_mac_device, uint8_t uc_device_reset_in_progress);
void mac_device_set_state(mac_device_stru *pst_mac_device, uint8_t en_device_state);

void mac_device_set_beacon_interval(mac_device_stru *pst_mac_device, uint32_t beacon_interval);

void mac_blacklist_get_pointer(wlan_vap_mode_enum_uint8 en_vap_mode, uint8_t uc_dev_id,
    uint8_t uc_chip_id, uint8_t uc_vap_id, mac_blacklist_info_stru **pst_blacklist_info);

void *mac_device_get_all_rates(mac_device_stru *pst_dev);
oal_bool_enum_uint8 mac_device_band_is_support(uint8_t mac_device_id, mac_device_radio_cap_enum_uint8 radio_cap);

extern uint32_t g_ddr_freq;
extern mac_device_custom_cfg_stru g_st_mac_device_custom_cfg;
extern uint8_t g_optimized_feature_switch_bitmap;
extern uint8_t g_rx_filter_frag;
extern uint8_t g_auc_mac_device_radio_cap[];
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
extern mac_mcast_ampdu_cfg_stru g_mcast_ampdu_cfg;
#endif
#ifdef _PRE_WLAN_CHBA_MGMT
extern int32_t g_d2h_rate_complete;
#endif


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_device_check_5g_enable_per_chip(void)
{
    uint8_t uc_dev_idx = 0;

    while (uc_dev_idx < WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP) {
        if (mac_device_band_is_support(uc_dev_idx, MAC_DEVICE_CAP_5G) == OAL_TRUE) {
            return OAL_TRUE;
        }
        uc_dev_idx++;
    }

    return OAL_FALSE;
}

wlan_mib_vht_supp_width_enum mac_device_trans_bandwith_to_vht_capinfo(wlan_bw_cap_enum_uint8 en_max_op_bd);
void mac_set_dual_sta_mode(uint8_t mode);
uint8_t mac_is_dual_sta_mode(void);
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
oal_bool_enum mac_get_mcast_ampdu_switch(void);
#endif
#endif /* end of mac_device.h */

