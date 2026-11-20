

#ifndef __WAL_LINUX_CFGVENDOR_H__
#define __WAL_LINUX_CFGVENDOR_H__

#include "oal_ext_if.h"
#include "oal_types.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_CFGVENDOR_H

#define ATTRIBUTE_U32_LEN      (OAL_NLMSG_HDRLEN + 4)
#define VENDOR_ID_OVERHEAD     ATTRIBUTE_U32_LEN
#define VENDOR_SUBCMD_OVERHEAD ATTRIBUTE_U32_LEN
#define VENDOR_DATA_OVERHEAD   (OAL_NLMSG_HDRLEN)

#define VENDOR_REPLY_OVERHEAD (VENDOR_ID_OVERHEAD +     \
                               VENDOR_SUBCMD_OVERHEAD + \
                               VENDOR_DATA_OVERHEAD)
/* Feature set */
#define VENDOR_DBG_MEMORY_DUMP_SUPPORTED (1 << (0)) /* Memory dump of FW */

/* WIFI GSCAN/Hotlist/EPNO/Roaming Capabilities:wal_wifi_gscan_capabilities */
#define MAX_BLACKLIST_BSSID 32
#define MAX_WHITELIST_SSID  0

/* WifiScanner.java */
#define WIFI_BAND_24_GHZ         (1) /* 2.4 GHz Band */
#define WIFI_BAND_5_GHZ          (2) /* 5 GHz Band without DFS channels */
#define WIFI_BAND_5_GHZ_DFS_ONLY (4) /* 5 GHz Band with DFS channels only */
#define WIFI_BAND_5_GHZ_WITH_DFS (6) /* 5 GHz band with DFS channels */
#define WIFI_BAND_BOTH           (3) /* both bands without DFS channels */
#define WIFI_BAND_BOTH_WITH_DFS  (7) /* both bands with DFS channels */

/* 2G信道与5G信道总数 */
#define VENDOR_CHANNEL_LIST_ALL (MAC_CHANNEL_FREQ_2_BUTT + MAC_CHANNEL_FREQ_5_BUTT)

/* firmware event ring, ring id 2 */
#define VENDOR_FW_EVENT_RING_NAME "fw_event"
#define VENDOR_FW_EVENT_RING_SIZE (64 * 1024)

/* Feature enums */ /* 需和wifi_hal.h中定义保持一致 */
#define WIFI_FEATURE_INFRA             0x0001    /** Basic infrastructure mode */
#define WIFI_FEATURE_INFRA_5G          0x0002    /** Support for 5 GHz Band */
#define WIFI_FEATURE_HOTSPOT           0x0004    /** Support for GAS/ANQP */
#define WIFI_FEATURE_P2P               0x0008    /** Wifi-Direct */
#define WIFI_FEATURE_SOFT_AP           0x0010    /** Soft AP */
#define WIFI_FEATURE_GSCAN             0x0020    /** G-Scan APIs */
#define WIFI_FEATURE_NAN               0x0040    /** Neighbor Awareness Networking */
#define WIFI_FEATURE_D2D_RTT           0x0080    /** Device-to-device RTT */
#define WIFI_FEATURE_D2AP_RTT          0x0100    /** Device-to-AP RTT */
#define WIFI_FEATURE_BATCH_SCAN        0x0200    /** Batched Scan (legacy) */
#define WIFI_FEATURE_PNO               0x0400    /** Preferred network offload */
#define WIFI_FEATURE_ADDITIONAL_STA    0x0800    /** Support for two STAs */
#define WIFI_FEATURE_TDLS              0x1000    /** Tunnel directed link setup */
#define WIFI_FEATURE_TDLS_OFFCHANNEL   0x2000    /** Support for TDLS off channel */
#define WIFI_FEATURE_EPR               0x4000    /** Enhanced power reporting */
#define WIFI_FEATURE_AP_STA            0x8000    /** Support for AP STA Concurrency */
#define WIFI_FEATURE_LINK_LAYER_STATS  0x10000   /** Link layer stats collection */
#define WIFI_FEATURE_LOGGER            0x20000   /** WiFi Logger */
#define WIFI_FEATURE_HAL_EPNO          0x40000   /** WiFi PNO enhanced */
#define WIFI_FEATURE_RSSI_MONITOR      0x80000   /** RSSI Monitor */
#define WIFI_FEATURE_MKEEP_ALIVE       0x100000  /** WiFi mkeep_alive */
#define WIFI_FEATURE_CONFIG_NDO        0x200000  /* ND offload configure */
#define WIFI_FEATURE_TX_TRANSMIT_POWER 0x400000  /* Capture Tx transmit power levels */
#define WIFI_FEATURE_CONTROL_ROAMING   0x800000  /* Enable/Disable firmware roaming */
#define WIFI_FEATURE_IE_WHITELIST      0x1000000 /* Support Probe IE white listing */
#define WIFI_FEATURE_SCAN_RAND         0x2000000 /* Support MAC & Probe Sequence Number randomization */
/* Add more features here */
#define MKEEP_ALIVE_IP_PKT_MAXLEN 128

typedef enum {
    VENDOR_NL80211_SUBCMD_UNSPECIFIED,

    VENDOR_NL80211_SUBCMD_RANGE_START = 0x0001,
    VENDOR_NL80211_SUBCMD_RANGE_END = 0x0FFF,

    VENDOR_NL80211_SUBCMD_GSCAN_RANGE_START = 0x1000,
    VENDOR_NL80211_SUBCMD_GSCAN_RANGE_END = 0x10FF,

    VENDOR_NL80211_SUBCMD_RTT_RANGE_START = 0x1100,
    VENDOR_NL80211_SUBCMD_RTT_RANGE_END = 0x11FF,

    VENDOR_NL80211_SUBCMD_LSTATS_RANGE_START = 0x1200,
    VENDOR_NL80211_SUBCMD_LSTATS_RANGE_END = 0x12FF,

    VENDOR_NL80211_SUBCMD_TDLS_RANGE_START = 0x1300,
    VENDOR_NL80211_SUBCMD_TDLS_RANGE_END = 0x13FF,

    VENDOR_NL80211_SUBCMD_DEBUG_RANGE_START = 0x1400,
    VENDOR_NL80211_SUBCMD_DEBUG_RANGE_END = 0x14FF,

    VENDOR_NL80211_SUBCMD_NBD_RANGE_START = 0x1500,
    VENDOR_NL80211_SUBCMD_NBD_RANGE_END = 0x15FF,

    VENDOR_NL80211_SUBCMD_WIFI_OFFLOAD_RANGE_START = 0x1600,
    VENDOR_NL80211_SUBCMD_WIFI_OFFLOAD_RANGE_END = 0x16FF,

    /* define all NAN related commands between 0x1700 and 0x17FF */
    VENDOR_NL80211_SUBCMD_NAN_RANGE_START = 0x1700,
    VENDOR_NL80211_SUBCMD_NAN_RANGE_END = 0x17FF,

    /* define all packet filter related commands between 0x1800 and 0x18FF */
    VENDOR_NL80211_SUBCMD_PKT_FILTER_RANGE_START = 0x1800,
    VENDOR_NL80211_SUBCMD_PKT_FILTER_RANGE_END = 0x18FF,
} wal_vendor_sub_command_enum;

enum wal_vendor_subcmd {
    GSCAN_SUBCMD_GET_CAPABILITIES = VENDOR_NL80211_SUBCMD_GSCAN_RANGE_START,
    GSCAN_SUBCMD_SET_CONFIG,
    GSCAN_SUBCMD_SET_SCAN_CONFIG,
    GSCAN_SUBCMD_ENABLE_GSCAN,
    GSCAN_SUBCMD_GET_SCAN_RESULTS,
    GSCAN_SUBCMD_SCAN_RESULTS,
    GSCAN_SUBCMD_SET_HOTLIST,
    GSCAN_SUBCMD_SET_SIGNIFICANT_CHANGE_CONFIG,
    GSCAN_SUBCMD_ENABLE_FULL_SCAN_RESULTS,
    GSCAN_SUBCMD_GET_CHANNEL_LIST,
    ANDR_WIFI_SUBCMD_GET_FEATURE_SET,
    ANDR_WIFI_SUBCMD_GET_FEATURE_SET_MATRIX,
    ANDR_WIFI_RANDOM_MAC_OUI,
    ANDR_WIFI_NODFS_CHANNELS,
    ANDR_WIFI_SET_COUNTRY,
    GSCAN_SUBCMD_SET_EPNO_SSID,
    WIFI_SUBCMD_SET_SSID_WHITELIST, /* 0x1010 */
    WIFI_SUBCMD_SET_LAZY_ROAM_PARAMS,
    WIFI_SUBCMD_ENABLE_LAZY_ROAM,
    WIFI_SUBCMD_SET_BSSID_PREF,
    WIFI_SUBCMD_SET_BSSID_BLACKLIST, /* 0x1014 */
    GSCAN_SUBCMD_ANQPO_CONFIG,
    WIFI_SUBCMD_SET_RSSI_MONITOR,
    WIFI_SUBCMD_CONFIG_ND_OFFLOAD, /* 0x1017 */
    WIFI_SUBCMD_CONFIG_TCPACK_SUP, /* 0x1018 */
    WIFI_SUBCMD_FW_ROAM_POLICY,    /* 0x1019 */

    RTT_SUBCMD_SET_CONFIG = VENDOR_NL80211_SUBCMD_RTT_RANGE_START,
    RTT_SUBCMD_CANCEL_CONFIG,
    RTT_SUBCMD_GETCAPABILITY,

    LSTATS_SUBCMD_GET_INFO = VENDOR_NL80211_SUBCMD_LSTATS_RANGE_START,
    DEBUG_START_LOGGING = VENDOR_NL80211_SUBCMD_DEBUG_RANGE_START,

    DEBUG_TRIGGER_MEM_DUMP,
    DEBUG_GET_MEM_DUMP,
    DEBUG_GET_VER,
    DEBUG_GET_RING_STATUS,
    DEBUG_GET_RING_DATA,
    DEBUG_GET_FEATURE,
    DEBUG_RESET_LOGGING,
    WIFI_OFFLOAD_SUBCMD_START_MKEEP_ALIVE = VENDOR_NL80211_SUBCMD_WIFI_OFFLOAD_RANGE_START,
    WIFI_OFFLOAD_SUBCMD_STOP_MKEEP_ALIVE,

    WIFI_SUBCMD_NAN_SET_PARAM = VENDOR_NL80211_SUBCMD_NAN_RANGE_START,
    WIFI_SUBCMD_NAN_ENABLE,
    WIFI_SUBCMD_NAN_DISABLE,
    WIFI_SUBCMD_NAN_PUBLISH_REQUEST,
    WIFI_SUBCMD_NAN_CANCEL_PUBLISH_REQUEST,
    WIFI_SUBCMD_NAN_SUBSCRIBE_REQUEST,
    WIFI_SUBCMD_NAN_SUBSCRIBE_CANCEL_REQUEST,
    WIFI_SUBCMD_NAN_FOLLOWUP_REQUEST,
    WIFI_SUBCMD_NAN_GET_CAPA,

    APF_SUBCMD_GET_CAPABILITIES = VENDOR_NL80211_SUBCMD_PKT_FILTER_RANGE_START,
    APF_SUBCMD_SET_FILTER,

    VENDOR_SUBCMD_MAX
};

enum andr_wifi_attr {
    ANDR_WIFI_ATTRIBUTE_NUM_FEATURE_SET,
    ANDR_WIFI_ATTRIBUTE_FEATURE_SET,
    ANDR_WIFI_ATTRIBUTE_RANDOM_MAC_OUI,
    ANDR_WIFI_ATTRIBUTE_NODFS_SET,
    ANDR_WIFI_ATTRIBUTE_COUNTRY
};

typedef enum wal_vendor_event {
    HISI_VENDOR_EVENT_UNSPEC,
    HISI_VENDOR_EVENT_PRIV_STR,
    VENDOR_GSCAN_SIGNIFICANT_EVENT,
    VENDOR_GSCAN_GEOFENCE_FOUND_EVENT,
    VENDOR_GSCAN_BATCH_SCAN_EVENT,
    VENDOR_SCAN_FULL_RESULTS_EVENT,
    VENDOR_RTT_COMPLETE_EVENT,
    VENDOR_SCAN_COMPLETE_EVENT,
    VENDOR_GSCAN_GEOFENCE_LOST_EVENT,
    VENDOR_SCAN_EPNO_EVENT,
    VENDOR_DEBUG_RING_EVENT,
    VENDOR_FW_DUMP_EVENT,
    VENDOR_PNO_HOTSPOT_FOUND_EVENT,
    VENDOR_RSSI_MONITOR_EVENT,
    VENDOR_MKEEP_ALIVE_EVENT,

    NAN_EVENT_RX  = 32,
} wal_vendor_event_enum;

enum {
    DEBUG_RING_ID_INVALID = 0,
    FW_VERBOSE_RING_ID,
    FW_EVENT_RING_ID,
    DHD_EVENT_RING_ID,
    /* add new id here */
    DEBUG_RING_ID_MAX
};

typedef struct {
    uint32_t max_scan_cache_size;              /* total space allocated for scan (in bytes) */
    uint32_t max_scan_buckets;                 /* maximum number of channel buckets */
    uint32_t max_ap_cache_per_scan;            /* maximum number of APs that can be stored per scan */
    uint32_t max_rssi_sample_size;             /* number of RSSI samples used for averaging RSSI */
    uint32_t max_scan_reporting_threshold;     /* max possible report_threshold as described
                                                       in wifi_scan_cmd_params */
    uint32_t max_hotlist_bssids;               /* maximum number of entries for hotlist BSSIDs */
    uint32_t max_hotlist_ssids;                /* maximum number of entries for hotlist SSIDs */
    uint32_t max_significant_wifi_change_aps;  /* maximum number of entries for
                                                       significant wifi change APs */
    uint32_t max_bssid_history_entries;        /* number of BSSID/RSSI entries that device can hold */
    uint32_t max_number_epno_networks;         /* max number of epno entries */
    uint32_t max_number_epno_networks_by_ssid; /* max number of epno entries if ssid is specified,
                                                       that is, epno entries for which an exact match is
                                                       required, or entries corresponding to hidden ssids */
    uint32_t max_number_of_white_listed_ssid;  /* max number of white listed SSIDs, M target is 2 to 4 */
    uint32_t max_number_blacklist_bssids;      /* max number of black listed BSSIDs */
} wal_wifi_gscan_capabilities;

typedef struct debug_ring_status_st {
    uint8_t uc_name[NUM_32_BYTES];
    uint32_t flags;
    signed long ring_id; /* unique integer representing the ring */
    /* total memory size allocated for the buffer */
    uint32_t ring_buffer_byte_size;
    uint32_t verbose_level;
    /* number of bytes that was written to the buffer by driver */
    uint32_t written_bytes;
    /* number of bytes that was read from the buffer by user land */
    uint32_t read_bytes;
    /* number of records that was read from the buffer by user land */
    uint32_t written_records;
} debug_ring_status_st;
typedef enum wifi_channel_width {
    WIFI_CHAN_WIDTH_20 = 0,
    WIFI_CHAN_WIDTH_40 = 1,
    WIFI_CHAN_WIDTH_80 = 2,
    WIFI_CHAN_WIDTH_160 = 3,
    WIFI_CHAN_WIDTH_80P80 = 4,
    WIFI_CHAN_WIDTH_5 = 5,
    WIFI_CHAN_WIDTH_10 = 6,
    WIFI_CHAN_WIDTH_INVALID = -1
} wal_wifi_channel_width;
#define VENDOR_NUM_RATE 32
#define VENDOR_NUM_PEER 1
#define VENDOR_NUM_CHAN 11

/* channel information */
typedef struct {
    wal_wifi_channel_width width; /* channel width:20, 40, 80, 80+80, 160 */
    int l_center_freq;            /* primary 20 MHz channel */
    int l_center_freq0;           /* center frequency (MHz) first segment */
    int l_center_freq1;           /* center frequency (MHz) second segment */
} wal_wifi_channel_info_stru;

/* channel statistics */
typedef struct {
    wal_wifi_channel_info_stru channel;
    uint32_t on_time; /* msecs the radio is awake */
    uint32_t cca_busy_time;
} wal_wifi_channel_stat_stru;
/* radio statistics */
typedef struct {
    int l_radio;
    uint32_t on_time;
    uint32_t tx_time;
    uint32_t rx_time;
    uint32_t on_time_scan;
    uint32_t on_time_nbd;
    uint32_t on_time_gscan;
    uint32_t on_time_roam_scan;
    uint32_t on_time_pno_scan;
    uint32_t on_time_hs20;
    uint32_t num_channels;
    wal_wifi_channel_stat_stru channels[VENDOR_NUM_CHAN];
} wal_wifi_radio_stat_stru;
typedef struct {
    uint64_t ull_wifi_on_time_stamp;
    uint32_t wifi_on_time;
    uint32_t wifi_tx_time;
    uint32_t wifi_rx_time;
} wal_cfgvendor_radio_stat_stru;

/* access categories */
typedef enum {
    WAL_WIFI_AC_VO = 0,
    WAL_WIFI_AC_VI = 1,
    WAL_WIFI_AC_BE = 2,
    WAL_WIFI_AC_BK = 3,
    WAL_WIFI_AC_MAX = 4,
} wal_wifi_traffic_ac;

/* wifi peer type */
typedef enum {
    WIFI_PEER_STA,
    WIFI_PEER_AP,
    WIFI_PEER_P2P_GO,
    WIFI_PEER_P2P_CLIENT,
    WIFI_PEER_NAN,
    WIFI_PEER_TDLS,
    WIFI_PEER_INVALID,
} wal_wifi_peer_type;

typedef enum {
    WIFI_INTERFACE_STA = 0,
    WIFI_INTERFACE_SOFTAP = 1,
    WIFI_INTERFACE_IBSS = 2,
    WIFI_INTERFACE_P2P_CLIENT = 3,
    WIFI_INTERFACE_P2P_GO = 4,
    WIFI_INTERFACE_NAN = 5,
    WIFI_INTERFACE_MESH = 6,
} wal_wifi_interface_mode;

typedef enum {
    WIFI_DISCONNECTED = 0,
    WIFI_AUTHENTICATING = 1,
    WIFI_ASSOCIATING = 2,
    WIFI_ASSOCIATED = 3,
    WIFI_EAPOL_STARTED = 4,
    WIFI_EAPOL_COMPLETED = 5,
} wal_wifi_connection_state;

typedef uint8_t mac_address[NUM_6_BYTES];

/* BSSID blacklist */
typedef struct {
    uint32_t num_bssid;                    /* number of blacklisted BSSIDs */
    mac_address bssids[MAX_BLACKLIST_BSSID]; /* blacklisted BSSIDs */
} wal_wifi_bssid_params;

typedef enum {
    VENDOR_WIFI_ROAMING_DISABLE = 0,
    VENDOR_WIFI_ROAMING_ENABLE = 1,
} wal_wifi_roaming_state;

/* interface info */
typedef struct {
    wal_wifi_interface_mode mode;
    uint8_t uc_mac_addr[NUM_6_BYTES];        /* interface mac address (self) */
    wal_wifi_connection_state state; /* connection state (valid for STA, CLI only) */
    wal_wifi_roaming_state roaming;
    uint32_t capabilities; /* WIFI_CAPABILITY_XXX (self) */
    uint8_t uc_ssid[33];      /* null terminated SSID, 32为SSID最大长度，用大小为33的数组存放 */
    uint8_t uc_bssid[NUM_6_BYTES];
    uint8_t uc_ap_country_str[NUM_3_BYTES];
    uint8_t uc_country_str[NUM_3_BYTES];
} wal_wifi_interface_info_stru;

typedef wal_wifi_interface_info_stru *wifi_interface_handle;

/* wifi rate */
typedef struct {
    uint32_t preamble : 3;   /* 0: OFDM, 1:CCK, 2:HT 3:VHT 4..7 reserved */
    uint32_t nss : 2;        /* 0:1x1, 1:2x2, 3:3x3, 4:4x4  */
    uint32_t bw : 3;         /* 0:20MHz, 1:40Mhz, 2:80Mhz, 3:160Mhz */
    uint32_t rate_mcs_idx : 8; /* OFDM/CCK rate code would be as per ieee std in the units of 0.5mbps */
    /* HT/VHT it would be mcs index */
    uint32_t reserved : 16;
    uint32_t bitrate; /* units of 100 Kbps */
} wal_wifi_rate_stru;

/* per rate statistics */
typedef struct {
    wal_wifi_rate_stru rate;
    uint32_t tx_mpdu;
    uint32_t rx_mpdu;
    uint32_t mpdu_lost;
    uint32_t retries;
    uint32_t retries_short;
    uint32_t retries_long;
} wal_wifi_rate_stat_stru;

/* per peer statistics */
typedef struct {
    wal_wifi_peer_type type; /* peer type:AP, TDLS, GO etc. */
    uint8_t uc_peer_mac_address[NUM_6_BYTES];
    uint32_t capabilities;
    uint32_t num_rate;
    wal_wifi_rate_stat_stru rate_stats[VENDOR_NUM_RATE];
} wal_wifi_peer_info_stru;

typedef struct {
    wal_wifi_traffic_ac ac; /* access category:VI, VO, BE, BK */
    uint32_t tx_mpdu;
    uint32_t rx_mpdu;
    uint32_t tx_mcast; /* number of succesfully transmitted multicast data packets */
    uint32_t rx_mcast;
    uint32_t rx_ampdu;
    uint32_t tx_ampdu;
    uint32_t mpdu_lost;
    uint32_t retries;
    uint32_t retries_short;
    uint32_t retries_long;
    uint32_t contention_time_min;
    uint32_t contention_time_max;
    uint32_t contention_time_avg;
    uint32_t contention_num_samples;
} wal_wifi_wmm_ac_stat_stru;

/* wifi interface statistics */
typedef struct {
    wifi_interface_handle iface;
    wal_wifi_interface_info_stru info;
    uint32_t beacon_rx;
    uint64_t ull_average_tsf_offset;            /* average beacon offset encountered (beacon_TSF - TBTT) */
    /* indicate that this AP typically leaks packets beyond the driver guard time. */
    uint32_t leaky_ap_detected;
    /* average number of frame leaked by AP after frame with PM bit set was ACK'ed by AP */
    uint32_t leaky_ap_avg_num_frames_leaked;
    uint32_t leaky_ap_guard_time;
    uint32_t mgmt_rx;
    uint32_t mgmt_action_rx;
    uint32_t mgmt_action_tx;
    int l_rssi_mgmt;
    int l_rssi_data;
    int l_rssi_ack;
    wal_wifi_wmm_ac_stat_stru ac[WAL_WIFI_AC_MAX];
    uint32_t num_peers;
    wal_wifi_peer_info_stru peer_info[VENDOR_NUM_PEER];
} wal_wifi_iface_stat_stru;

extern wal_cfgvendor_radio_stat_stru g_st_wifi_radio_stat;
static inline void wal_check_free(uint8_t *buff)
{
    if (buff != NULL) {
        oal_mem_free_m(buff, OAL_TRUE);
    }
}
void wal_cfgvendor_init(oal_wiphy_stru *wiphy);
void wal_cfgvendor_deinit(oal_wiphy_stru *wiphy);
void wal_clear_continuous_disable_enable_wifi_cnt(void);
#endif /* end of wal_linux_cfgvendor.h */
