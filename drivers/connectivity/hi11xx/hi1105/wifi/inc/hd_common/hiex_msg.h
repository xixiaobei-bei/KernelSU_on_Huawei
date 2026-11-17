

#ifndef __HIEX_MSG_H__
#define __HIEX_MSG_H__

#ifdef _PRE_WLAN_FEATURE_HIEX
#include "oal_types.h"
#include "wlan_types.h"

#ifdef _HIEX_CHIP_TYPE_110X

#ifdef WIN32
#define __OAL_DECLARE_PACKED_ROM
#else
#define __OAL_DECLARE_PACKED_ROM __attribute__((__packed__))
#endif

#ifndef oal_delta_count16
#define oal_delta_count16(_us_start, _us_end) \
        ((uint16_t)((_us_start) < (_us_end) ? ((_us_end) - (_us_start)) : \
        (((uint16_t)0XFFFF) - (_us_start) + (_us_end) + 1)))
#define oal_delta_count32(_ul_start, _ul_end) \
        ((uint32_t)((_ul_start) < (_ul_end) ? ((_ul_end) - (_ul_start)) : \
        ((((uint32_t)0XFFFFFFFF) - (_ul_start)) + (_ul_end) + 1)))
#endif

#define WLAN_MGMT_NETBUF_SIZE2 WLAN_MGMT_NETBUF_SIZE
#define HIEX_HIMIT_ADN_NUM 5
#define HIEX_HIMIT_V2_ADN_NUM 8

/* Himit desc total size is 36 bytes, keep same with hi1152/hi1105 */
typedef struct {
    uint8_t uc_himit_evm_min; /* 符号级别EVM最小值。上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    uint8_t uc_himit_evm_max; /* 符号级别EVM最大值。上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    uint8_t uc_himit_evm_avg; /* 符号级别EVM平均值。上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    uint8_t bit_himit_multipath_delay : 5; /* 多径延迟，单位db */
    uint8_t bit_reserved1 : 2;
    uint8_t bit_himit_valid : 1; /* 回填的私有信息有效指示：0：无效 1：有效 */

    uint8_t uc_himit_evm_pri20;   /* 主20M EVM平均值。上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    uint8_t uc_himit_evm_pri40;   /* 主40M EVM平均值。上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    uint8_t uc_himit_evm_pri80;   /* 主80M EVM平均值。上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    /* 空间流0基于symbol的EVM平均值 上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    uint8_t uc_himit_evm_ss0_avg;

    int8_t c_himit_rssi_hdr;             /* 前导部分滤除干扰后的RSSI，单位1db，范围-128db~127db */
    uint16_t bit_himit_cond_num_avg : 5; /* MIMO两流时信道平均条件数 */
    uint16_t bit_reserved2 : 11;
    /* 空间流1基于symbol的EVM平均值 上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    uint8_t uc_himit_evm_ss1_avg;

    int8_t c_himit_rssi_hltf_ant0; /* HLTF全带宽的天线0的RSSI，单位1db，范围-128db~127db */
    int8_t c_himit_rssi_hltf_ant1; /* HLTF全带宽的天线1的RSSI，单位1db，范围-128db~127db */
    uint8_t uc_himit_snr_ant0;     /* 天线0的时域SNR，单位db */
    uint8_t uc_himit_snr_ant1;     /* 天线1的时域SNR，单位db */

    uint32_t aul_himit_tx_adn[HIEX_HIMIT_ADN_NUM];
}__OAL_DECLARE_PACKED_ROM hiex_tx_himit_dscr_stru;

typedef struct {
    uint8_t uc_himit_evm_min; /* 符号级别EVM最小值。上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    uint8_t uc_himit_evm_max; /* 符号级别EVM最大值。上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    uint8_t uc_himit_evm_avg; /* 符号级别EVM平均值。上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    uint8_t bit_himit_multipath_delay : 5; /* 多径延迟，单位db */
    uint8_t bit_reserved1   : 1;
    uint8_t bit_himit_mode  : 2; /* 回填的私有信息有效指示：0：无效 1：v1格式 2: v2格式 */

    uint8_t uc_himit_evm_pri20; /* 主20M EVM平均值。上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    uint8_t uc_himit_evm_pri40; /* 主40M EVM平均值。上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    uint8_t uc_himit_evm_pri80; /* 主80M EVM平均值。上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    /* 空间流0基于symbol的EVM平均值。上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    uint8_t uc_himit_evm_ss0_avg;

    uint8_t bit_reserved2;
    uint16_t bit_cond_num_12_avg : 7;/* MIMO两流时信道平均条件数 */
    uint16_t bit_reserved3       : 9;
    /* 空间流1基于symbol的EVM平均值。上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    uint8_t uc_himit_evm_ss1_avg;

    int8_t c_himit_rssi_hltf_ant0; /* HLTF全带宽的天线0的RSSI，单位1db，范围-128db~127db */
    int8_t c_himit_rssi_hltf_ant1; /* HLTF全带宽的天线1的RSSI，单位1db，范围-128db~127db */
    uint8_t uc_himit_snr_ant0;     /* 天线0的时域SNR，单位db */
    uint8_t uc_himit_snr_ant1;     /* 天线1的时域SNR，单位db */

    /* 空间流2基于symbol的EVM平均值。上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    uint8_t uc_himit_evm_ss2_avg;
    /* 空间流3基于symbol的EVM平均值。上报绝对值，取值范围[0,255],对应EVM是（-64,0]dB，精度1/4dB */
    uint8_t uc_himit_evm_ss3_avg;
    int8_t c_himit_rssi_hltf_ant2; /* HLTF全带宽的天线2的RSSI，单位1db，范围-128db~127db */
    int8_t c_himit_rssi_hltf_ant3; /* HLTF全带宽的天线3的RSSI，单位1db，范围-128db~127db */

    uint8_t uc_himit_snr_ant2;     /* 天线2的时域SNR，单位db */
    uint8_t uc_himit_snr_ant3;     /* 天线3的时域SNR，单位db */
    uint16_t bit_cond_num_13_avg : 7;/* MIMO两流时信道平均条件数 */
    uint16_t bit_cond_num_14_avg : 7;/* MIMO两流时信道平均条件数 */
    uint16_t bit_reserved4       : 2;

    int8_t c_himit_rssi_hdr;             /* 前导部分滤除干扰后的RSSI，单位1db，范围-128db~127db */
    uint8_t bit_reserved5[3];

    uint32_t aul_himit_tx_adn[HIEX_HIMIT_V2_ADN_NUM];
}__OAL_DECLARE_PACKED_ROM hiex_tx_himit_v2_dscr_stru;

typedef union {
    hiex_tx_himit_v2_dscr_stru v2_dscr;
    hiex_tx_himit_dscr_stru    v1_dscr;
} hiex_tx_himit_dscr_union;

typedef struct {
    uint32_t  bit_valid   : 2;                  /* HIMIT_V1: valid; HIMIT_V2: recv_mode */
    uint32_t  bit_resv    : 30;
    uint32_t  rx_txadn[HIEX_HIMIT_V2_ADN_NUM];  /* HIMIT_V1: 使用前5个word */
} hiex_rx_himit_dscr_stru;
#endif

/* hiex msg used in transaction between nodes */
typedef enum mac_hiex_msg_type {
    HIEX_MSG_TYPE_NO_MSG = 0,
    HIEX_MSG_TYPE_QUERY_CHS = 1,        /* query channel stat  */
    HIEX_MSG_TYPE_QUERY_AC_DELAY = 2,   /* query ac delay */
    HIEX_MSG_TYPE_SII_REPORT = 3,       /* stream info item report */
    HIEX_MSG_TYPE_AC_DELAY_REPORT = 4,  /* ac delay report  */
    HIEX_MSG_TYPE_CHS_REPORT = 5,       /* channel stat report */
    HIEX_MSG_TYPE_EDCA_CTRL = 6,        /* EDCA param ctrl */
    HIEX_MSG_TYPE_CCA_TH_CTRL = 7,      /* CCA TH ctrl */
    HIEX_MSG_TYPE_ERSRU_CTRL = 8,       /* ERSRU ctrl */

    HIEX_MSG_TYPE_CNT
} mac_hiex_msg_type_enum;
typedef uint8_t mac_hiex_msg_type_enum_uint8;

typedef struct mac_hiex_msg {
    mac_hiex_msg_type_enum_uint8  type;
    uint8_t                     size; /* header size no included */
    uint8_t                     data[0];
} __OAL_DECLARE_PACKED_ROM mac_hiex_msg_stru;

#define mac_hiex_msg_type(msg) ((msg)->type)
#define mac_hiex_msg_size(msg) ((msg)->size)
#define mac_hiex_msg_size_wh(msg) ((msg)->size + sizeof(mac_hiex_msg_stru))
#define MAC_HIEX_MSG_TYPE mac_hiex_msg_type
#define MAC_HIEX_MSG_SIZE mac_hiex_msg_size
#define MAC_HIEX_MSG_SIZE_WH mac_hiex_msg_size_wh

#ifdef _PRE_WLAN_FEATURE_HIEX_DBG_TST
/* insert POINT as required sequence */
typedef enum {
    MAC_HIEX_TST_POINT_APP_TX = 0,

    MAC_HIEX_TST_POINT_HOST_TX_START,
    MAC_HIEX_TST_POINT_HOST_HMAC_TX_START,
    MAC_HIEX_TST_POINT_HOST_TX_DONE,

    MAC_HIEX_TST_POINT_DEVICE_TX_START,
    MAC_HIEX_TST_POINT_3,
    MAC_HIEX_TST_POINT_4,
    MAC_HIEX_TST_POINT_DEVICE_TX_DONE,

    MAC_HIEX_TST_POINT_DEVICE_RX_START,
    MAC_HIEX_TST_POINT_5,
    MAC_HIEX_TST_POINT_DEVICE_RX_DONE,

    MAC_HIEX_TST_POINT_HOST_RX_START,
    MAC_HIEX_TST_POINT_6,
    MAC_HIEX_TST_POINT_HOST_RX_DONE,

    MAC_HIEX_TST_POINT_APP_RX,

    MAC_HIEX_TST_POINT_CNT
} mac_hiex_tst_point_enum;

/* ttm timestamp */
typedef struct mac_hiex_tst {
    uint32_t         bit_map; // hiex_map
    uint32_t         tst[MAC_HIEX_TST_POINT_CNT]; // hiex_tst
} __OAL_DECLARE_PACKED_ROM mac_hiex_tst_stru;

typedef struct mac_hiex_iperf {
    uint32_t  seq;
    int32_t   tv_sec;
    int32_t   tv_usec;
} __OAL_DECLARE_PACKED_ROM mac_hiex_iperf_stru;

typedef struct mac_hiex_rtt {
    mac_hiex_iperf_stru iperf;
    mac_hiex_tst_stru   tst;
    int32_t           offset;
    int32_t           owd;
} __OAL_DECLARE_PACKED_ROM mac_hiex_rtt_stru;

#define mac_hiex_tst_map(tst)           ((tst)->bit_map)
#define mac_hiex_tst_point_bit(point)   (1u << (uint32_t)(point))
#define mac_hiex_tst_idx(__tst, point)  ((__tst)->tst[point])


#define mac_hiex_is_tx_point(point) (((point) >= MAC_HIEX_TST_POINT_HOST_TX_START) && \
    ((point) <= MAC_HIEX_TST_POINT_DEVICE_TX_DONE))
#define mac_hiex_is_rx_point(point) (((point) >= MAC_HIEX_TST_POINT_DEVICE_RX_START) && \
    ((point) <= MAC_HIEX_TST_POINT_HOST_RX_DONE))

#define mac_hiex_is_device_point(point) ((((point) >= MAC_HIEX_TST_POINT_DEVICE_TX_START) && \
    ((point) <= MAC_HIEX_TST_POINT_DEVICE_TX_DONE)) || \
    (((point) >= MAC_HIEX_TST_POINT_DEVICE_RX_START) && ((point) <= MAC_HIEX_TST_POINT_DEVICE_RX_DONE)))
#endif

enum {
    MAC_HIMIT_VERSION_V1 = 0,
    MAC_HIMIT_VERSION_V2,
};

enum {
    MAC_HIEX_CHIP_TYPE_HI1152 = 0,
    MAC_HIEX_CHIP_TYPE_HI1105,
};

#define MAC_HE_HIMIT_HTC_ID          (0xE) /* HE模式时，himit私有 htc id */

typedef struct mac_hiex_cap {
    uint32_t  bit_hiex_enable         : 1;
    uint32_t  bit_hiex_version        : 3;
    uint32_t  bit_himit_enable        : 1;
    uint32_t  bit_ht_himit_enable     : 1;
    uint32_t  bit_vht_himit_enable    : 1;
    uint32_t  bit_he_himit_enable     : 1;

    uint32_t  bit_he_htc_himit_id     : 4;
    uint32_t  bit_chip_type           : 4;

    uint32_t  bit_himit_version       : 2;
    uint32_t  bit_ersru_enable        : 1;
    uint32_t  bit_resv                : 13;
} __OAL_DECLARE_PACKED_ROM mac_hiex_cap_stru;

#ifdef _PRE_WLAN_FEATURE_HIEX_DBG_TST
typedef enum {
    SKB_HEADER_ETH = 0,
    SKB_HEADER_IP,
    SKB_HEADER_SNAP,
    SKB_HEADER_80211,
} skb_header_type_enum;

#define mac_hiex_tx_traced(skb)    MAC_GET_CB_HIEX_TRACED((mac_tx_ctl_stru *)oal_netbuf_cb(skb))
#define mac_hiex_rx_traced(skb)    MAC_GET_RX_CB_HIEX_TRACED((mac_rx_ctl_stru *)oal_netbuf_cb(skb))
#endif

#define MAC_HIEX_GAME_MARK  0x5a

/* stream info item operation */
typedef enum {
    MAC_HIEX_SII_ADD = 0,
    MAC_HIEX_SII_DEL = 1,
} mac_hiex_sii_op_enum;

typedef struct mac_hiex_sii_report_msg {
    uint32_t bit_op    : 1;
    uint32_t bit_resv  : 31;
    uint32_t mark;
    uint32_t server_ip;
    uint32_t client_ip;
    uint16_t server_port;
    uint16_t client_port;
} __OAL_DECLARE_PACKED_ROM mac_hiex_sii_report_msg_stru;

typedef struct mac_hiex_ersru_ctrl_msg {
    uint16_t bit_enable    : 1;
    uint16_t bit_resv      : 15;
    uint16_t bit_frag_len;
} __OAL_DECLARE_PACKED_ROM mac_hiex_ersru_ctrl_msg_stru;

#define MAC_HIEX_ERSRU_MINI_FRAG_LEN 100
#define MAC_HIEX_ERSRU_DELTA_FRAG_LEN 10 /* WiFi6+下分片门限判定的delta值,AP调度限制帧长与分片帧长之间的允许差值 */

/* channel stat report, max size is 18 bytes, included in himit tx adn */
typedef struct {
    uint8_t   channel;
    uint8_t   bandwidth     : 4;
    uint8_t   resv0         : 4;
    uint16_t  stat_time;

    uint8_t   sync_err_ratio;
    uint8_t   abort_time_ratio;

    uint8_t   rx_time_ratio;  // 255-scale
    uint8_t   tx_time_ratio;
    uint8_t   direct_rx_ratio;
    uint8_t   nondir_rx_ratio;

    uint8_t   duty_ratio_20;  /* 0xff for invalid */
    uint8_t   duty_ratio_40;
    uint8_t   duty_ratio_80;
    uint8_t   duty_ratio_160;

    int8_t    free_power_20;  /* 0 for invalid  */
    int8_t    free_power_40;
    int8_t    free_power_80;
    int8_t    free_power_160;
} __OAL_DECLARE_PACKED_ROM mac_hiex_chs_report_msg_stru;

typedef struct mac_hiex_edca_param {
    uint8_t   txop          : 4; // TU
    uint16_t  aifsn         : 4;
    uint16_t  cw_max        : 4;
    uint16_t  cw_min        : 4;
}__OAL_DECLARE_PACKED_ROM mac_hiex_edca_param_stru;

typedef struct mac_hiex_edca_ctrl_msg {
    uint32_t  channel        : 8;
    uint32_t  bandwidth      : 4;
    uint32_t  enabled        : 1; /* 1105 edca ctrled */
    uint32_t  resv0          : 19;
    mac_hiex_edca_param_stru ac[WLAN_WME_AC_BUTT];
} __OAL_DECLARE_PACKED_ROM mac_hiex_edca_ctrl_msg_stru;

typedef struct mac_hiex_cca_ctrl_msg {
    uint32_t  channel       : 8;
    uint32_t  bandwidth     : 4;
    uint32_t  enabled       : 1; /* 1105 cca ctrled */
    uint32_t  resv0         : 3;

    int8_t    cca_ed_high_20; /* 0 for invalid */
    int8_t    cca_ed_high_40;
    int8_t    cca_ed_high_80;
    int8_t    c_ed_cca_th_dsss;
    int8_t    c_ed_cca_th_ofdm;
    int8_t    c_ed_cca_th_40m_high;
    int8_t    c_ed_cca_th_80m_high;
} __OAL_DECLARE_PACKED_ROM mac_hiex_cca_ctrl_msg_stru;

#define MAC_HIEX_AC_DELAY_CNT  11
typedef struct {
    uint32_t tx_count;
    uint32_t tx_duration; /* ms */
    uint8_t ac;

    /*
     * histogram[0] = ratio of [0, 512*2^n), n=0
     * histogram[n] = ratio of [512*2^n, 512*2^(n+1)), n=[1, 7]
     * histogram[0] = ratio of [512*2^(n+1), max), n=8
     * 8-bit scaled
     */
    uint8_t histogram[MAC_HIEX_AC_DELAY_CNT];
} __OAL_DECLARE_PACKED_ROM mac_hiex_ac_delay_report_msg_stru;

typedef struct {
    uint8_t   ac;
} __OAL_DECLARE_PACKED_ROM mac_hiex_ac_delay_request_msg_stru;

typedef struct {
    uint16_t  channel       : 8;
    uint16_t  bandwidth     : 4;
    uint16_t  resv0         : 4;
} __OAL_DECLARE_PACKED_ROM mac_hiex_chs_request_msg_stru;


typedef enum {
    MAC_HIEX_LOCAL_MSG_TX_ACTION,
    MAC_HIEX_LOCAL_MSG_RX_ACTION,
} mac_hiex_local_msg_type_enum;
typedef oal_bool_enum_uint8 mac_hiex_local_msg_type_enum_uint8;

typedef struct {
    mac_hiex_local_msg_type_enum_uint8  type;
    uint8_t                           size;
    uint16_t                          user_idx;
    uint8_t                           dat[];
} __OAL_DECLARE_PACKED_ROM mac_hiex_local_msg_stru; /* hd_event */

/* 配置user himit 功能 */
typedef struct {
    uint8_t               auc_mac_addr[WLAN_MAC_ADDR_LEN]; /* MAC地址 */
    mac_hiex_cap_stru       st_hiex_cap;
} mac_cfg_user_hiex_param_stru; /* hd_event */

typedef struct {
    uint8_t tx_pwr_detail_2g[NUM_2_BYTES]; // 2.4G 2字节功率信息(su功率，tb功率)
    uint8_t tx_pwr_detail_5g[NUM_4_BYTES][NUM_2_BYTES]; // 5G 4个band, 每个band 2字节功率信息(su功率，tb功率)
} mac_sta_max_tx_power_info_stru; /* hd_event */

#endif
#endif
