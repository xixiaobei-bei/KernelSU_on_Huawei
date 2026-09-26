

#ifndef __HAL_COMMON_H__
#define __HAL_COMMON_H__

/* 1 头文件包含 */
#include "oal_types.h"
#include "wlan_spec.h"
#include "wlan_types.h"
#ifdef _PRE_WLAN_FEATURE_HIEX
#include "hiex_msg.h"
#endif
#include "hal_common_descriptor.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_COMMON_H
/*****************************************************************************/
/*****************************************************************************/
/*                        公共宏定义、枚举、结构体                           */
/*****************************************************************************/
/*****************************************************************************/
#define MAX_MAC_AGGR_MPDU_NUM  256
#define AL_TX_MSDU_NETBUF_MAX_LEN 2000

#define HAL_POW_11B_RATE_NUM 4 /* 11b速率数目 */
#define HAL_POW_11G_RATE_NUM 8 /* 11g速率数目 */
#define HAL_POW_11A_RATE_NUM 8 /* 11a速率数目 */

#ifdef _PRE_WLAN_FEATURE_11AC_20M_MCS9
#ifdef _PRE_WLAN_FEATURE_1024QAM
#define HAL_POW_11AC_20M_NUM 12 /* 11n_11ac_2g速率数目 */
#else
#define HAL_POW_11AC_20M_NUM 10 /* 11n_11ac_2g速率数目 */
#endif
#else
#define HAL_POW_11AC_20M_NUM 9 /* 11n_11ac_2g速率数目 */
#endif

#ifdef _PRE_WLAN_FEATURE_1024QAM
#define HAL_POW_11AC_40M_NUM 13 /* 11n_11ac_2g速率数目 */
#define HAL_POW_11AC_80M_NUM 12 /* 11n_11ac_2g速率数目 */
#else
#define HAL_POW_11AC_40M_NUM 11 /* 11n_11ac_2g速率数目 */
#define HAL_POW_11AC_80M_NUM 10 /* 11n_11ac_2g速率数目 */
#endif

#ifdef _PRE_WLAN_FEATURE_160M
#ifdef _PRE_WLAN_FEATURE_1024QAM
#define HAL_POW_11AC_160M_NUM 12 /* 11n_11ac_2g速率数目 */
#else
#define HAL_POW_11AC_160M_NUM 10 /* 11n_11ac_2g速率数目 */
#endif
#else
#define HAL_POW_11AC_160M_NUM 0 /* 11n_11ac_2g速率数目(用于打桩计算HAL_POW_RATE_POW_CODE_TABLE_LEN) */
#endif

#define HAL_POW_RATE_CODE_TBL_LEN (HAL_POW_11B_RATE_NUM + HAL_POW_11G_RATE_NUM + \
    HAL_POW_11AC_20M_NUM + HAL_POW_11AC_40M_NUM + \
    HAL_POW_11AC_80M_NUM + HAL_POW_11AC_160M_NUM) /* rate-tpccode table中速率个数 */

#define HAL_POW_CUSTOM_24G_11B_RATE_NUM        2 /* 定制化11b速率数目 */
#define HAL_POW_CUSTOM_11G_11A_RATE_NUM        5 /* 定制化11g/11a速率数目 */
#define HAL_POW_CUSTOM_HT20_VHT20_RATE_NUM     6 /* 定制化HT20_VHT20速率数目 */
#define HAL_POW_CUSTOM_24G_HT40_VHT40_RATE_NUM 8
#define HAL_POW_CUSTOM_5G_HT40_VHT40_RATE_NUM  7
#define HAL_POW_CUSTOM_5G_VHT80_RATE_NUM       6
/* 定制化全部速率 */
#define HAL_POW_CUSTOM_MCS9_10_11_RATE_NUM     3
#define HAL_POW_CUSTOM_MCS10_11_RATE_NUM       2
#define HAL_POW_CUSTOM_5G_VHT160_RATE_NUM      12 /* 定制化5G_11ac_VHT160速率数目 */
#define HAL_POW_CUSTOM_HT20_VHT20_DPD_RATE_NUM 5  /* 定制化DPD速率数目 */
#define HAL_POW_CUSTOM_HT40_VHT40_DPD_RATE_NUM 5
/* 定制化相关宏 */
/* NVRAM中存储的各协议速率最大发射功率参数的个数 From:24G_11b_1M To:5G_VHT80_MCS7 */
#define NUM_OF_NV_NORMAL_MAX_TXPOWER (HAL_POW_CUSTOM_24G_11B_RATE_NUM +                                            \
                                      HAL_POW_CUSTOM_11G_11A_RATE_NUM + HAL_POW_CUSTOM_HT20_VHT20_RATE_NUM +       \
                                      HAL_POW_CUSTOM_24G_HT40_VHT40_RATE_NUM + HAL_POW_CUSTOM_11G_11A_RATE_NUM +   \
                                      HAL_POW_CUSTOM_HT20_VHT20_RATE_NUM + HAL_POW_CUSTOM_5G_HT40_VHT40_RATE_NUM + \
                                      HAL_POW_CUSTOM_5G_VHT80_RATE_NUM)
#define NUM_OF_NV_MAX_TXPOWER (NUM_OF_NV_NORMAL_MAX_TXPOWER + \
                               HAL_POW_CUSTOM_MCS9_10_11_RATE_NUM * 4 + \
                               HAL_POW_CUSTOM_5G_VHT160_RATE_NUM + \
                               HAL_POW_CUSTOM_MCS10_11_RATE_NUM)

#define NUM_OF_NV_DPD_MAX_TXPOWER (HAL_POW_CUSTOM_HT20_VHT20_DPD_RATE_NUM + HAL_POW_CUSTOM_HT40_VHT40_DPD_RATE_NUM)
#define NUM_OF_NV_11B_DELTA_TXPOWER      2
#define NUM_OF_NV_5G_UPPER_UPC           4
#define NUM_OF_IQ_CAL_POWER              2
#define NUM_OF_NV_2G_LOW_POW_DELTA_VAL   4
#define NUM_OF_NV_5G_LPF_LVL             4
/* TPC档位设置 */
#define HAL_POW_LEVEL_NUM          5                              /* 算法总档位数目 */

#define HAL_MU_MAX_USER_NUM 1

/* 每个用户支持的最大速率集个数 */
#define HAL_TX_RATE_MAX_NUM 4

#define HAL_MULTI_TID_MAX_NUM      2  /* 支持Multi-TID的最大TID数目 */
#define HAL_MAX_ANT_NUM            2  /* 2G/5G下的天线数 */

#define HAL_TX_MSDU_DSCR_LEN 24 /* mac tx msdu描述符大小 */

/* 3.4 中断相关枚举定义 */
/* 因为mac error和dmac misc优先级一致，03将high prio做实时事件队列来处理，mac error并入dmac misc */
/* 3.4.1 实时事件中断类型 */
typedef enum {
    HAL_EVENT_DMAC_HIGH_PRIO_BTCOEX_PS,   /* BTCOEX ps中断, 因为rom化，目前只能放置一个 */
    HAL_EVENT_DMAC_HIGH_PRIO_BTCOEX_LDAC, /* BTCOEX LDAC中断 */

    HAL_EVENT_DMAC_HIGH_PRIO_SUB_TYPE_BUTT
} hal_event_dmac_high_prio_sub_type_enum;
typedef uint8_t hal_event_dmac_high_prio_sub_type_enum_uint8;

/* 性能测试相关 */
typedef enum {
    HAL_ALWAYS_TX_DISABLE,      /* 禁用常发 */
    HAL_ALWAYS_TX_RF,           /* 保留给RF测试广播报文 */
    HAL_ALWAYS_TX_AMPDU_ENABLE, /* 使能AMPDU聚合包常发 */
    HAL_ALWAYS_TX_MPDU,         /* 使能非聚合包常发 */
    HAL_ALWAYS_TX_BUTT
} hal_device_always_tx_state_enum;
typedef uint8_t hal_device_always_tx_enum_uint8;

typedef enum {
    HAL_ALWAYS_RX_DISABLE,      /* 禁用常收 */
    HAL_ALWAYS_RX_RESERVED,     /* 保留给RF测试广播报文 */
    HAL_ALWAYS_RX_AMPDU_ENABLE, /* 使能AMPDU聚合包常收 */
    HAL_ALWAYS_RX_ENABLE,       /* 使能非聚合包常收 */
    HAL_ALWAYS_RX_BUTT
} hal_device_always_rx_state_enum;
typedef uint8_t hal_device_always_rx_enum_uint8;

typedef enum {
    HAL_TXBF_PROTOCOL_LEGACY = 0,  // 不包括11b
    HAL_TXBF_PROTOCOL_HT = 1,
    HAL_TXBF_PROTOCOL_VHT = 2,
    HAL_TXBF_PROTOCOL_HE = 3,

    HAL_TXBF_PROTOCOL_TYPE_BUTT
} hal_txbf_protocol_type_enum;
typedef uint8_t hal_txbf_protocol_type_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_MONITOR
typedef enum {
    WLAN_SINFFER_OFF, /* sniffer抓包功能关闭 */
    WLAN_SNIFFER_TRAVEL_CAP_ON, /* sniffer出行场景抓包功能开启 */
    WLAN_SINFFER_ON,  /* sniffer抓包功能开启 */
    WLAN_SNIFFER_STATE_BUTT
} wlan_sniffer_state_enum;
typedef uint8_t wlan_sniffer_state_enum_uint8;

typedef enum {
    WLAN_MONITOR_OTA_HOST_RPT, /* host ota上报 */
    WLAN_MONITOR_OTA_DEVICE_RPT,  /* device ota上报 */
    WLAN_MONITOR_OTA_ALL_RPT,

    WLAN_MONITOR_OTA_RPT_BUTT
} wlan_monitor_ota_state_enum;
typedef uint8_t wlan_monitor_ota_mode_enum_uint8;

/* 同1105的hal_rx_status_stru，与device要保持一致 */
typedef struct {
    /* byte 0 */
    uint8_t bit_cipher_protocol_type : 4; /* 接收帧加密类型 */
    uint8_t bit_dscr_status : 4;          /* 接收状态 */

    /* byte 1 */
    uint8_t bit_AMPDU : 1;
    uint8_t bit_last_mpdu_flag : 1; /* 固定位置 */
    uint8_t bit_gi_type : 2;
    uint8_t bit_he_ltf_type : 2;
    uint8_t bit_sounding_mode : 2;

    /* byte 2 */
    uint8_t bit_freq_bandwidth_mode : 3;
    uint8_t bit_rx_himit_flag : 1; /* rx himit flag = 1103 bit_preabmle */
    uint8_t bit_ext_spatial_streams : 2;
    uint8_t bit_smoothing : 1;
    uint8_t bit_fec_coding : 1; /* channel code */

    /* byte 3-4 */
    union {
        struct {
            uint8_t bit_rate_mcs : 4;
            uint8_t bit_nss_mode : 2;

            uint8_t bit_protocol_mode : 4;
            uint8_t bit_is_rx_vip : 1; /* place dcm bit */
            uint8_t bit_rsp_flag : 1;  /* beaforming时，是否上报信道矩阵的标识，0:上报，1:不上报 */
            uint8_t bit_mu_mimo_flag : 1;
            uint8_t bit_ofdma_flag : 1;
            uint8_t bit_beamforming_flag : 1; /* 接收帧是否开启了beamforming */
            uint8_t bit_STBC : 1;
        } st_rate; /* 11a/b/g/11ac/11ax的速率集定义 */
        struct {
            uint8_t bit_ht_mcs : 6;
            uint8_t bit_protocol_mode : 4;
            uint8_t bit_is_rx_vip : 1; /* place dcm bit */
            uint8_t bit_rsp_flag : 1;  /* beaforming时，是否上报信道矩阵的标识，0:上报，1:不上报 */
            uint8_t bit_mu_mimo_flag : 1;
            uint8_t bit_ofdma_flag : 1;
            uint8_t bit_beamforming_flag : 1; /* 接收帧是否开启了beamforming */
            uint8_t bit_STBC : 1;
        } st_ht_rate; /* 11n的速率集定义 */
    } un_nss_rate;
} hal_sniffer_rx_status_stru;

typedef struct {
    /* byte 0 */
    int8_t c_rssi_dbm;

    /* byte 1-4 */
    uint8_t uc_code_book;
    uint8_t uc_grouping;
    uint8_t uc_row_number;

    /* byte 5-6 */
    int8_t c_snr_ant0; /* ant0 SNR */
    int8_t c_snr_ant1; /* ant1 SNR */

    /* byte 7-8 */
    int8_t c_ant0_rssi; /* ANT0上报当前帧RSSI */
    int8_t c_ant1_rssi; /* ANT1上报当前帧RSSI */
} hal_sniffer_rx_statistic_stru;

#define SNIFFER_RX_INFO_SIZE                                                              \
    (sizeof(hal_sniffer_rx_status_stru) + sizeof(hal_sniffer_rx_statistic_stru) + \
        sizeof(uint32_t) + sizeof(hal_statistic_stru))
#endif
/* 3.6 加密相关枚举定义 */
/* 3.6.1  芯片密钥类型定义 */
typedef enum {
    HAL_KEY_TYPE_TX_GTK = 0, /* Hi1102:HAL_KEY_TYPE_TX_IGTK */
    HAL_KEY_TYPE_PTK = 1,
    HAL_KEY_TYPE_RX_GTK = 2,
    HAL_KEY_TYPE_RX_GTK2 = 3, /* 02使用，03和51不使用 */
    HAL_KEY_TYPE_BUTT
} hal_cipher_key_type_enum;
typedef uint8_t hal_cipher_key_type_enum_uint8;

/* 7.5 对外部接收提供接口所用数据结构 */
typedef struct {
    /* byte 0 */
    int8_t c_rssi_dbm;

    /* byte 1 */
    uint8_t bit_resv4;

#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV))
    int8_t snr[2];  /* 2天线的SNR */
    int8_t rssi[2]; /* 2天线的RSSI */
#else
    int8_t snr[4];  /* 4天线的SNR */
    int8_t rssi[4]; /* 4天线的RSSI */
#endif
} hal_rx_statistic_stru;

/* dmac_pkt_captures使用,tx rx均会使用 */
typedef struct {
    union {
        uint16_t rate_value;
        /* 除HT外的速率集定义 */
        struct {
            uint16_t bit_rate_mcs : 4;
            uint16_t bit_nss_mode : 2;
            uint16_t bit_protocol_mode : 4;  // wlan_phy_protocol_mode_enum_uint8
            uint16_t bit_reserved2 : 6;
        } rate_stru;
        struct {
            uint16_t bit_ht_mcs : 6;
            uint16_t bit_protocol_mode : 4;
            uint16_t bit_reserved2 : 6;
        } st_ht_rate; /* 11n的速率集定义 */
    } un_nss_rate;

    uint8_t uc_short_gi;
    uint8_t uc_bandwidth;

    uint8_t bit_preamble : 1,
            bit_channel_code : 1,
            bit_stbc : 2,
            bit_reserved2 : 4;
} hal_statistic_stru;

/* HOST专属 */
typedef struct {
    uint32_t *pul_mac_hdr_start_addr; /* 对应的帧的帧头地址,虚拟地址 */
    uint16_t us_da_user_idx;          /* 目的地址用户索引 */
    uint16_t us_rsv;                  /* 对齐 */
} mac_rx_expand_cb_stru;

/* 裸系统下需要传输给HMAC模块的信息 */
/* hal_rx_ctl_stru结构的修改要考虑hi110x_rx_get_info_dscr函数中的优化 */
/* 1字节对齐 */
#pragma pack(push, 1)
typedef struct {
    /* byte 0 */
    uint8_t bit_vap_id : 5;
    uint8_t bit_amsdu_enable : 1;    /* 是否为amsdu帧,每个skb标记 */
    uint8_t bit_is_first_buffer : 1; /* 当前skb是否为amsdu的首个skb */
    uint8_t bit_is_fragmented : 1;

    /* byte 1 */
    uint8_t uc_msdu_in_buffer; /* 每个skb包含的msdu数,amsdu用,每帧标记 */

    /* byte 2 */
    /* host add macro then open */
    uint8_t bit_ta_user_idx : 5;
    uint8_t bit_tid : 1;
    uint8_t bit_nan_flag : 1;
    uint8_t bit_is_key_frame : 1;
    /* byte 3 */
    uint8_t uc_mac_header_len : 6; /* mac header帧头长度 */
    uint8_t bit_is_beacon : 1;
    uint8_t bit_is_last_buffer : 1;

    /* byte 4-5 */
    uint16_t us_frame_len; /* 帧头与帧体的总长度,AMSDU非首帧不填 */

    /* byte 6 */
    /* host add macro then open 修改原因:host与device不一致导致uc_mac_vap_id获取错误，出现HOST死机 */
    uint8_t uc_mac_vap_id : 4; /* 业务侧vap id号 */
    uint8_t bit_buff_nums : 4; /* 每个MPDU占用的SKB数,AMSDU帧占多个 */
    /* byte 7 */
    uint8_t uc_channel_number; /* 接收帧的信道 */
#if (defined(_PRE_PRODUCT_ID_HI110X_HOST) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1106_DEV) || \
    (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_BISHENG_DEV))
    /* 适配06; 对于03/05 host skb cb 48字节，hcc层copy会有冗余，不会有功能问题 */
    uint16_t   us_frame_control;
    uint16_t   bit_start_seqnum   : 12;
    uint16_t   bit_cipher_type    : 4;

    uint16_t   bit_release_end_sn : 12;
    uint16_t   bit_fragment_num   : 4;

    uint16_t   bit_rx_user_id     : 12;
    uint16_t   bit_frame_format   : 2;
    uint16_t   bit_dst_is_valid   : 1;
    uint16_t   bit_mcast_bcast    : 1;

    uint16_t   us_seq_num         : 12;
    uint16_t   bit_process_flag   : 3;
    uint16_t   bit_release_valid  : 1;

    uint8_t    rx_tid;
    uint8_t    dst_user_id;

    uint16_t   bit_release_start_sn : 12;
    uint16_t   bit_is_reo_timeout   : 1;
    uint16_t   bit_first_sub_msdu   : 1;
    uint16_t   bit_is_ampdu         : 1;
    uint16_t   is_6ghz_flag         : 1;

    uint8_t    dst_hal_vap_id;
    uint8_t    bit_band_id          : 4;
    uint8_t    bit_dst_band_id      : 4;
    uint8_t    rx_status;

    uint32_t   rx_lsb_pn;
    uint16_t   us_rx_msb_pn;
#endif
#ifdef _PRE_PRODUCT_ID_HI110X_HOST
    /* OFFLOAD架构下，HOST相对DEVICE的CB增量 */
    mac_rx_expand_cb_stru st_expand_cb;
#endif
} __OAL_DECLARE_PACKED hal_rx_ctl_stru;
#pragma pack(pop)

/*
 * 裸系统下针对接收，提供读取接口
 * frame_len长度
 * 802.11帧头长度(uc_mac_hdr_len)
*/
#pragma pack(push, 1)
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1106_DEV) || \
    (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_BISHENG_DEV))
typedef struct {
    /* byte 0 */
    uint8_t bit_cipher_protocol_type : 4; /* 接收帧加密类型 */
    uint8_t bit_dscr_status : 4;          /* 接收状态 */

    /* byte 1 */
    uint8_t bit_AMPDU : 1;
    uint8_t bit_last_mpdu_flag : 1; /* 固定位置 */
    uint8_t bit_gi_type : 2;
    uint8_t bit_he_ltf_type : 2;
    uint8_t bit_sounding_mode : 2;

    /* byte 2 */
    uint8_t bit_freq_bandwidth_mode : 3;
    uint8_t bit_rx_himit_flag : 1; /* rx himit flag */
    uint8_t bit_ext_spatial_streams : 2;
    uint8_t bit_smoothing : 1;
    uint8_t bit_fec_coding : 1; /* channel code */

    /* byte 3-4 */
    union { /* todo */
        struct {
            uint8_t bit_rate_mcs : 4;
            uint8_t bit_nss_mode : 2;

            uint8_t bit_protocol_mode : 4;
            uint8_t bit_is_rx_vip : 1; /* place dcm bit */
            uint8_t bit_rsp_flag : 1;  /* beaforming时，是否上报信道矩阵的标识，0:上报，1:不上报 */
            uint8_t bit_mu_mimo_flag : 1;
            uint8_t bit_ofdma_flag : 1;
            uint8_t bit_beamforming_flag : 1; /* 接收帧是否开启了beamforming */
            uint8_t bit_STBC : 1;
        } st_rate; /* 11a/b/g/11ac/11ax的速率集定义 */
        struct {
            uint8_t bit_ht_mcs : 6;

            uint8_t bit_protocol_mode : 4;
            uint8_t bit_is_rx_vip : 1; /* place dcm bit */
            uint8_t bit_rsp_flag : 1;  /* beaforming时，是否上报信道矩阵的标识，0:上报，1:不上报 */
            uint8_t bit_mu_mimo_flag : 1;
            uint8_t bit_ofdma_flag : 1;
            uint8_t bit_beamforming_flag : 1; /* 接收帧是否开启了beamforming */
            uint8_t bit_STBC : 1;
        } st_ht_rate; /* 11n的速率集定义 */
    } un_nss_rate;
} hal_rx_status_stru;

#else // (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
typedef struct {
    /* byte 0 */
    uint8_t bit_cipher_protocol_type : 4; /* 接收帧加密类型 */
    uint8_t bit_dscr_status : 4;          /* 接收状态 */

    /* byte 1 */
    uint8_t bit_fec_coding : 1;
    uint8_t bit_last_mpdu_flag : 1;
    uint8_t bit_resv : 1;
    uint8_t bit_gi_type : 2;
    uint8_t bit_AMPDU : 1;
    uint8_t bit_sounding_mode : 2;

    /* byte 2 */
    uint8_t bit_ext_spatial_streams : 2;
    uint8_t bit_smoothing : 1;
    uint8_t bit_freq_bandwidth_mode : 4;
    uint8_t bit_preabmle : 1;

    /* byte 3-4 */
    union {
        struct {
            uint8_t bit_rate_mcs : 4;
            uint8_t bit_nss_mode : 2;
            uint8_t bit_protocol_mode : 2;

            uint8_t bit_rsp_flag : 1;
            uint8_t bit_STBC : 2;
            uint8_t bit_he_flag : 1;
            uint8_t bit_is_rx_vip : 1;
            uint8_t bit_he_ltf_type : 2;
            uint8_t bit_reserved3 : 1;
        } st_rate; /* 11a/b/g/11ac/11ax的速率集定义 */
        struct {
            uint8_t bit_ht_mcs : 6;
            uint8_t bit_protocol_mode : 2;
            uint8_t bit_rsp_flag : 1;
            uint8_t bit_STBC : 2;
            uint8_t bit_he_flag : 1;
            uint8_t bit_is_rx_vip : 1;
            uint8_t bit_he_ltf_type : 2;
            uint8_t bit_reserved3 : 1;
        } st_ht_rate; /* 11n的速率集定义 */
    } un_nss_rate;
} hal_rx_status_stru;
#endif // (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV)
#pragma pack(pop)
/*****************************************************************************
  结构名  : hal_rx_dscr_queue_header_stru
  结构说明: 接收描述符队列头的结构体
*****************************************************************************/
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
typedef struct {
    oal_dlist_head_stru st_header;                    /* 发送描述符队列头结点 */
    uint16_t us_element_cnt;                        /* 接收描述符队列中元素的个数 */
    hal_dscr_queue_status_enum_uint8 uc_queue_status; /* 接收描述符队列的状态 */
    uint8_t uc_available_res_cnt;                   /* 当前队列中硬件可用描述符个数 */
} hal_rx_dscr_queue_header_stru;
#else
typedef struct {
    uint32_t *pul_element_head;                     /* 指向接收描述符链表的第一个元素 */
    uint32_t *pul_element_tail;                     /* 指向接收描述符链表的最后一个元素 */
    uint16_t us_element_cnt;                        /* 接收描述符队列中元素的个数 */
    hal_dscr_queue_status_enum_uint8 uc_queue_status; /* 接收描述符队列的状态 */
    uint8_t auc_resv[1]; /* 对齐 1 */
} hal_rx_dscr_queue_header_stru;
#endif

/* 扫描状态，通过判断当前扫描的状态，判断多个扫描请求的处理策略以及上报扫描结果的策略 */
typedef enum {
    MAC_SCAN_STATE_IDLE,
    MAC_SCAN_STATE_RUNNING,

    MAC_SCAN_STATE_BUTT
} mac_scan_state_enum;
typedef uint8_t mac_scan_state_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_11AX
typedef enum {
    HAL_HE_HTC_CONFIG_UPH_AUTO = 0,                   /* 默认auto, mac 插入和计算uph */
    HAL_HE_HTC_CONFIG_NO_HTC = 1,                     /* 数据帧不含有htc 头 */
    HAL_HE_HTC_CONFIG_SOFT_INSERT_SOFT_ADD = 2,       /* 软件添加 htc头 */
    HAL_HE_HTC_CONFIG_UPH_MAC_INSERT_TRIGGER_CAL = 3, /* mac插入htc,软件通过trigger帧计算uph */
    HAL_HE_HTC_CONFIG_MAC_INSERT_SOFT_ADD = 4,        /* mac插入软件固定的htc值 */

    HAL_HE_HTC_CONFIG_BUTT
} hal_he_htc_config_enum;
#endif

/* 接收方向帧类型，值不可修改 */
typedef enum {
    MAC_FRAME_TYPE_RTH   = 0,   /* Ethernet II帧格式 */
    MAC_FRAME_TYPE_RSV0  = 1,
    MAC_FRAME_TYPE_RSV1  = 2,
    MAC_FRAME_TYPE_80211 = 3,   /* 802.11帧格式 */

    FRAME_TYPE_RESEREVD         /* 保留 */
} mac_frame_type_enum;
typedef uint8_t mac_frame_type_enum_uint8;

typedef enum {
    MGMT_COMP_RING = 0,
    DATA_COMP_RING,
    RX_PPDU_COMP_RING,
    BA_INFO_RING,

    RX_COMP_RING_TYPE_BUTT
} RX_COMP_RING_TYPE;
typedef uint8_t hal_comp_ring_type_enum_uint8;

#define HAL_RX_MPDU_QNUM 2

typedef struct tag_hal_device_rx_mpdu_stru {
    /* MPDU缓存队列，中断上半部与下半部乒乓切换，以减少并发访问 */
    oal_netbuf_head_stru  ast_rx_mpdu_list[HAL_RX_MPDU_QNUM];
    oal_spin_lock_stru    st_spin_lock;

    /* 中断<下>半部当前使用的MPDU描述符队列 */
    uint8_t              cur_idx;
    uint8_t              auc_resv[3]; /* 3 resv */

    /* 每次最多出队列数目 */
    uint32_t             process_num_per_round;
} hal_rx_mpdu_que;

typedef struct {
    volatile uint32_t tx_msdu_info_ring_base_lsb;
    volatile uint32_t tx_msdu_info_ring_base_msb;

    volatile uint16_t write_ptr;
    volatile uint16_t tx_msdu_ring_depth : 4;
    volatile uint16_t max_aggr_amsdu_num : 4;
    volatile uint16_t reserved : 8;

    volatile uint16_t read_ptr;
    volatile uint16_t reserved1;
} hal_tx_msdu_info_ring_stru;

typedef struct hal_pwr_fit_para_stru {
    int32_t l_pow_par2; /* 二次项系数 */
    int32_t l_pow_par1; /* 一次 */
    int32_t l_pow_par0; /* 常数项 */
} hal_pwr_fit_para_stru;

#define HAL_BA_BITMAP_SIZE 8 /* MAC上报的ba info中ba bitmap大小为8个word */

typedef struct {
    uint16_t user_id;
    uint16_t ba_ssn;
    uint8_t tid;
    uint8_t ba_info_vld;
    uint32_t ba_bitmap[HAL_BA_BITMAP_SIZE];
} hal_tx_ba_info_stru;

typedef struct {
    uint16_t seq_num;
    oal_bool_enum_uint8 sn_vld;
    uint8_t tx_count;
} hal_tx_msdu_dscr_info_stru;

#ifdef _PRE_WLAN_FEATURE_HIEX
typedef mac_hiex_cap_stru hal_hiex_cap_stru;
#endif

/* hal device id 枚举 */
typedef enum {
    HAL_DEVICE_ID_MASTER        = 0,    /* master的hal device id */
    HAL_DEVICE_ID_SLAVE         = 1,    /* slave的hal device id */

    HAL_DEVICE_ID_BUTT                  /* 最大id */
} hal_device_id_enum;
typedef uint8_t hal_device_id_enum_enum_uint8;

typedef enum {
    HAL_RAM_RX = 0,  /* Device侧接收数据 */
    HAL_DDR_RX = 1,  /* Host DDR接收数据 */
    HAL_SWTICH_ING = 2,  /* DDR切换过程中 */

    HAL_RX_BUTT
} hal_rx_switch_enum;

#endif /* end of hal_common.h */
