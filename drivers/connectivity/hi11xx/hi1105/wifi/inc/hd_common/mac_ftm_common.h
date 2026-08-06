
#ifndef __MAC_FTM_COMMON_H__
#define __MAC_FTM_COMMON_H__

#define MAX_MINIMUN_AP_COUNT      14
#ifdef _PRE_WLAN_FEATURE_FTM
/* RTT Type */
typedef enum {
    RTT_TYPE_1_SIDED = 0x1,
    RTT_TYPE_2_SIDED = 0x2,
} mac_ftm_wifi_rtt_type;

/* RTT peer type */
typedef enum {
    RTT_PEER_AP         = 0x1,
    RTT_PEER_STA        = 0x2,
    RTT_PEER_P2P_GO     = 0x3,
    RTT_PEER_P2P_CLIENT = 0x4,
    RTT_PEER_NAN        = 0x5,
} mac_ftm_rtt_peer_type;

/* RTT Measurement Preamble */
typedef enum {
    WIFI_RTT_PREAMBLE_LEGACY = 0x1,
    WIFI_RTT_PREAMBLE_HT     = 0x2,
    WIFI_RTT_PREAMBLE_VHT    = 0x4
} mac_ftm_wifi_rtt_preamble;

/* RTT Measurement Bandwidth */
typedef enum {
    WIFI_RTT_BW_5   = 0x01,
    WIFI_RTT_BW_10  = 0x02,
    WIFI_RTT_BW_20  = 0x04,
    WIFI_RTT_BW_40  = 0x08,
    WIFI_RTT_BW_80  = 0x10,
    WIFI_RTT_BW_160 = 0x20
} mac_ftm_wifi_rtt_bw;

/* wifi rate */
typedef struct {
    uint32_t preamble     :3; /* 0: OFDM, 1:CCK, 2:HT 3:VHT 4..7 reserved */
    uint32_t nss          :2; /* 0:1x1, 1:2x2, 3:3x3, 4:4x4 */
    uint32_t bw           :3; /* 0:20MHz, 1:40Mhz, 2:80Mhz, 3:160Mhz */
    uint32_t rate_mcs_idx :8; /* OFDM/CCK rate code would be as per ieee std in the units of 0.5mbps HT/VHT—mcs index */
    uint32_t reserved     :16;
    uint32_t bitrate; /* units of 100 Kbps */
} mac_ftm_wifi_rate;

/* Format of information elements found in the beacon */
typedef struct {
    uint8_t id; /* element identifier */
    uint8_t len; /* number of bytes to follow */
    uint8_t data[];
} mac_ftm_wifi_information_element;

/* RTT results */
typedef struct {
    uint8_t addr[WLAN_MAC_ADDR_LEN];
    uint32_t burst_num;
    uint32_t measurement_number;
    uint32_t success_number;
    uint8_t number_per_burst_peer;
    uint32_t status; /* enum:mac_ftm_wifi_rtt_status */
    uint8_t retry_after_duration;
    uint32_t type; /* enum:mac_ftm_wifi_rtt_type */
    int32_t rssi;
    int32_t rssi_spread;
    mac_ftm_wifi_rate tx_rate;
    mac_ftm_wifi_rate rx_rate;
    int64_t rtt;
    int64_t rtt_sd;
    int64_t rtt_spread;
    int32_t distance_mm;
    int32_t distance_sd_mm;
    int32_t distance_spread_mm;
    int64_t ts;
    int32_t burst_duration;
    int32_t negotiated_burst_num;
    mac_ftm_wifi_information_element *lci;
    mac_ftm_wifi_information_element *lcr;
} mac_ftm_wifi_rtt_result; /* hd_event */

/* FTM调试开关相关的结构体 */
typedef struct {
    uint8_t mac_addr[WLAN_MAC_ADDR_LEN];
    uint8_t lci_request;
    uint8_t lcr_request;
    uint32_t burst_num;
    uint32_t ftms_per_burst;
    uint32_t burst_duration;
    uint32_t burst_period;
    uint32_t preamble;
    uint32_t bw;
    uint8_t is_asap_on;
    uint8_t channel_num;
    uint8_t format_bw;
    uint8_t is_app_wifi_rtt;
    mac_channel_stru st_channel;
} mac_send_iftmr_stru; /* hd_event */

typedef struct {
    uint32_t initiator_tsf_fix_offset;
    uint32_t responder_ptsf_offset;
} mac_ftm_timeout_stru;

typedef struct {
    uint32_t             ftm_correct_time1;
    uint32_t             ftm_correct_time2;
    uint32_t             ftm_correct_time3;
    uint32_t             ftm_correct_time4;
} mac_set_ftm_time_stru;

typedef struct {
    uint8_t auc_resv[NUM_2_BYTES];
    uint8_t auc_mac[WLAN_MAC_ADDR_LEN];
} mac_send_ftm_stru;

typedef struct {
    uint8_t auc_mac[WLAN_MAC_ADDR_LEN];
    uint8_t uc_dialog_token;
    uint8_t uc_meas_token;

    uint16_t us_num_rpt;

    uint16_t us_start_delay;
    uint8_t auc_reserve[1];
    uint8_t uc_minimum_ap_count;
    uint8_t aauc_bssid[MAX_MINIMUN_AP_COUNT][WLAN_MAC_ADDR_LEN];
    uint8_t auc_channel[MAX_MINIMUN_AP_COUNT];
} mac_send_ftm_range_req_stru; /* 和ftm_range_req_stru 同步修改 */

typedef struct {
    uint8_t   auc_bssid[WLAN_MAC_ADDR_LEN];
} mac_neighbor_report_req_stru;

typedef struct {
    oal_bool_enum_uint8 en_lci_enable;
    oal_bool_enum_uint8 en_interworking_enable;
    oal_bool_enum_uint8 en_civic_enable;
    oal_bool_enum_uint8 en_geo_enable;
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
} mac_send_gas_init_req_stru;

typedef struct {
    uint8_t           uc_index;
    uint8_t           auc_resv[1];
    uint8_t           auc_mac[WLAN_MAC_ADDR_LEN];
}mac_ftm_set_white_list_stru;

typedef struct {
    uint32_t                      cmd_bit_map;
    uint32_t                      ftm_cali;
    oal_bool_enum_uint8             en_ftm_initiator_bit0;        /* ftm_initiator命令 */
    mac_send_iftmr_stru             st_send_iftmr_bit1;           /* 发送iftmr命令 */
    oal_bool_enum_uint8             en_enable_bit2;               /* 使能FTM */
    oal_bool_enum_uint8             en_cali_bit3;                 /* FTM环回 */
    mac_send_ftm_stru               st_send_ftm_bit4;             /* 发送ftm命令 */
    oal_bool_enum_uint8             en_ftm_resp_bit5;             /* ftm_resp命令 */
    mac_set_ftm_time_stru           st_ftm_time_bit6;             /* ftm_time命令 */
    mac_send_ftm_range_req_stru     st_send_range_req_bit7;       /* 发送ftm_range_req命令 */
    oal_bool_enum_uint8             en_ftm_range_bit8;            /* ftm_range命令 */
    uint8_t                         uc_get_cali_reserv_bit9;
    uint8_t                         ftm_chain_selection;
    oal_bool_enum_uint8             en_multipath_bit12;
    mac_ftm_timeout_stru            st_ftm_timeout_bit14;               /* 用户设置的ftm定时器超时时间 */
    mac_neighbor_report_req_stru    st_neighbor_report_req_bit15;       /* 发送neighbour report request命令 */
    mac_send_gas_init_req_stru      st_gas_init_req_bit16;              /* 发送gas init req 命令 */
    mac_ftm_set_white_list_stru     st_set_white_list_bit17;
} mac_ftm_debug_switch_stru; /* hd_event */
#endif

#endif /* end of mac_ftm_common.h */
