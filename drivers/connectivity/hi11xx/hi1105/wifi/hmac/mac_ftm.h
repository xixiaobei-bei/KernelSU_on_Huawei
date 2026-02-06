

#ifndef __MAC_FTM_H__
#define __MAC_FTM_H__

#ifdef _PRE_WLAN_FEATURE_FTM
#include "mac_vap.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAL_FILE_ID_MAC_FTM_H

#define MAC_FTM_MAX_FTMS_PER_BURST 10
#define MAC_FTM_MAX_NUM_OF_BURSTS_EXPONENT 4 /* 最大ftm burst num指数 */

extern uint8_t g_mac_ftm_cap; /* FTM定制化 */

typedef enum {
    MAC_FTM_DISABLE_MODE = 0,
    MAC_FTM_RESPONDER_MODE = 1,
    MAC_FTM_INITIATOR_MODE = 2,
    MAC_FTM_MIX_MODE = 3,
    MAC_FTM_MODE_BUTT,
} mac_ftm_mode_enum;

typedef enum {
    FTM_FORMAT_BANDWIDTH_HTMIXED_20 = 9,
    FTM_FORMAT_BANDWIDTH_VHT20,
    FTM_FORMAT_BANDWIDTH_HTMIXED_40,
    FTM_FORMAT_BANDWIDTH_VHT_40,
    FTM_FORMAT_BANDWIDTH_VHT_80 = 13,
} mac_ftm_para_fomat_and_bandwidth_field_enum;

/* channel width */
typedef enum {
    MAC_FTM_WIFI_CHAN_WIDTH_20    = 0,
    MAC_FTM_WIFI_CHAN_WIDTH_40    = 1,
    MAC_FTM_WIFI_CHAN_WIDTH_80    = 2,
    MAC_FTM_WIFI_CHAN_WIDTH_160   = 3,
    MAC_FTM_WIFI_CHAN_WIDTH_80P80 = 4,
    MAC_FTM_WIFI_CHAN_WIDTH_5     = 5,
    MAC_FTM_WIFI_CHAN_WIDTH_10    = 6,
    MAC_FTM_WIFI_CHAN_WIDTH_INVALID = -1
} mac_ftm_wifi_channel_width;

typedef struct {
    uint8_t           uc_is_gas_request_sent;
    uint8_t           uc_gas_dialog_token;
    uint8_t           uc_gas_response_dialog_token;
    uint8_t           auc_resv[1];
} mac_gas_mgmt_stru;

/* channel information */
typedef struct {
    uint32_t width; /* enum:mac_ftm_wifi_channel_width channel width (20, 40, 80, 80+80, 160) */
    int32_t center_freq; /* primary 20 MHz channel */
    int32_t center_freq0; /* center frequency (MHz) first segment */
    int32_t center_freq1; /* center frequency (MHz) second segment */
} mac_ftm_wifi_channel_info;

/* RTT measurement params struct */
typedef struct {
    uint8_t addr[WLAN_MAC_ADDR_LEN];
    uint32_t type; /* enum:mac_ftm_wifi_rtt_type */
    uint32_t peer; /* enum:mac_ftm_rtt_peer_type */
    mac_ftm_wifi_channel_info channel;
    uint32_t burst_period;
    uint32_t num_burst;
    uint32_t num_frames_per_burst;
    uint32_t num_retries_per_rtt_frame;
    uint32_t num_retries_per_ftmr;
    uint8_t lci_request;
    uint8_t lcr_request;
    uint32_t burst_duration;
    uint32_t preamble; /* enum:mac_ftm_wifi_rtt_preamble */
    uint32_t bw; /* enum:mac_ftm_wifi_rtt_bw */
} mac_ftm_wifi_rtt_config;

typedef struct {
    int8_t rtt_target_cnt;
    mac_ftm_wifi_rtt_config *wifi_rtt_config;
} mac_ftm_wifi_rtt_config_params;


uint8_t mac_is_ftm_enable(mac_vap_stru *mac_vap);
void mac_ftm_mib_init(mac_vap_stru *mac_vap);
uint8_t mac_ftm_get_cap(void);
void mac_ftm_add_to_ext_capabilities_ie(mac_vap_stru *mac_vap, uint8_t *buffer, uint8_t *ie_len);
#endif /* _PRE_WLAN_FEATURE_FTM */
#endif /* __MAC_FTM_H__ */
