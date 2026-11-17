
#ifndef HMAC_FTM_H
#define HMAC_FTM_H

#include "hmac_user.h"
#include "hmac_vap.h"
#include "host_hal_device.h"

#ifdef _PRE_WLAN_FEATURE_FTM

#define DMAC_FTM_BUFF_SIZE 216
#define MAX_FTM_INITIATOR_SESSION 4  /* FTM同时在线的会话 */
#define MAX_FTM_RESPONDER_SESSION 4 /* FTM同时在线的会话 */

/* 2 宏定义 */
#define FTM_TIME_CALC_RATIO 6  /* FTM时间计算系数 */
#define FTM_TOD_INTP_TIME_CLK_PERIOD 960  /* 960M 时钟是640M, 但折算960M计算，因为960M约等于1ns */

#define FTM_FRAME_DIALOG_TOKEN_OFFSET          2  /* ftm帧dialog token偏移 */
#define FTM_FRAME_DIALOG_TOKEN_LEN             2
#define FTM_FRAME_IE_OFFSET                    3  /* ftm req帧ie偏移 */
#define FTM_FRAME_FOLLOWUP_DIALOG_TOKEN_OFFSET 3  /* ftm帧follow up dialog token偏移 */
#define FTM_FRAME_TOD_OFFSET                   4  /* ftm帧tod偏移 */
#define FTM_FRAME_TOA_OFFSET                   10 /* ftm帧toa偏移 */
#define FTM_FRAME_TOA_ERROR_OFFSET             18 /* ftm帧toa偏移 */
#define FTM_FRAME_OPTIONAL_IE_OFFSET           20
#define FTM_FRAME_TSF_SYNC_INFO_OFFSET         3
#define FTM_RANGE_IE_OFFSET                    3
#define FTM_REQ_TRIGGER_OFFSET                 2 /* ftm req帧Trigger偏移 */
#define FTM_TSF_SYNC_INFO_LEN                  5

#define FTM_WAIT_TIMEOUT        10  /* 等待接收FTM_1的时间10ms */
#define FTM_RES_ASAP_SET_PTSF_OFFSET 3000 /* responder sta设置ptsf偏移时间3ms */
#define FTM_RES_FTM_TRIGGER_TIME_OFFSET 5000 /* responder sta 发FTM Trigger时间偏移5ms */
#define FTM_RES_ASAP_TX_FTM_TIMEOUT 4

#define FTM_TODA_LENGTH       6
#define FTM_TODA_ERROR_LENGTH 2
#define FTM_TIME_MASK         0xFFFFFFFFFFFF /* 48位 */

#define MAC_FTM_MAX_FTMS_PER_BURST 10
#define MAC_FTM_MAX_NUM_OF_BURSTS_EXPONENT 4 /* 最大ftm burst num指数 */

/* 默认ftm交互的参数 */
/* 一个会话ftm的帧 */
#define MIN_FTMS_PER_BURST 2
#define MAX_FTMS_PER_BURST 15
/* FTM帧之间的间隙 */
#define FTM_MIN_DELTA 40 /* 单位100us，即4ms */
/* FTM 一个会话中ftm有效交互时间 */
#define FTM_BURST_DURATION 15 /* No preference */  // 11 /*128ms*/
#define FTM_MIN_DURATION   4
#define FTM_MAX_DURATION   11
/* FTM 一个会话时间 */
#define FTM_BURST_PERIOD      2 /* 单位100ms，即200ms */
#define FTM_DEFAULT_SCAN_TIME 300
#define FTM_DISTANCE_SPEED_OF_LIGHT_PARA 3 /* 3*10^8 m/s */
#define FTM_INI_STA_NO_PREF_BURST_NUM 32768
#define FTM_INI_STA_NO_PREF_RES_STA_BURST_NUM 2
#define FTM_RES_STA_DIALOG_TOKEN_MAX 256
#define FTM_MIN_DELTA_TO_DURATION_PARA 10 /* FTM min delta转成burst duration (min delta * 100us / 1000)ms 即除以10 */
#define FTM_BURST_DURATION_OFFSET 10 /* burst duration offset */
#define FTM_BURST_PERIOD_UINT 100 /* the interval between two consecutive burst instances, in units of 100 ms */

#define MAX_FTM_RANGE_ENTRY_COUNT 15
#define MAX_FTM_ERROR_ENTRY_COUNT 11

#define MAC_MEASUREMENT_REQUEST_IE_OFFSET     5
#define MEAS_RPT_IE_LEN 100

typedef enum {
    FTM_BW_20M = 0,
    FTM_BW_40M = 1,
    FTM_BW_80M = 2,
    FTM_BW_160M = 3,
    FTM_BW_BUTT,
} ftm_time_enum;

typedef enum {
    FTM_STATUS_RESERVED = 0,
    FTM_STATUS_SUCCESS = 1,
    FTM_STATUS_REQUEST_INCAPABLE = 2,
    FTM_STATUS_REQUEST_FAILED = 3,

    STATUS_INDICATION_BUTT,
}ftm_param_status_indication_enum;

typedef struct {
    uint64_t tod;
    uint64_t toa;
    uint16_t tod_error;
    uint16_t toa_error;

    union {
        uint32_t reg_value;
        struct {
            uint32_t bit_phase_incr : 13;
            uint32_t bit_reserved0 : 3;
            uint32_t bit_snr : 13;
            uint32_t bit_reserved1 : 3;
        } reg0;
    } ftm_rpt_reg0;

    union {
        uint32_t reg_value;
        struct {
            uint32_t bit_max_power : 13;
            uint32_t bit_reserved0 : 3;
            uint32_t bit_left0_power : 13;
            uint32_t bit_reserved1 : 3;
        } reg1;
    } ftm_rpt_reg1;

    union {
        uint32_t reg_value;
        struct {
            uint32_t bit_r0_power : 13;
            uint32_t bit_reserved0 : 3;
            uint32_t bit_l1_power : 13;
            uint32_t bit_reserved1 : 3;
        } reg2;
    } ftm_rpt_reg2;

    union {
        uint32_t reg_value;
        struct {
            uint32_t bit_l2_power : 13;
            uint32_t bit_reserved0 : 3;
            uint32_t bit_l3_power : 13;
            uint32_t bit_reserved1 : 3;
        } reg3;
    } ftm_rpt_reg3;

    union {
        uint32_t reg_value;
        struct {
            uint32_t bit_l4_power : 13;
            uint32_t bit_reserved0 : 3;
            uint32_t bit_l5_power : 13;
            uint32_t bit_reserved1 : 3;
        } reg4;
    } ftm_rpt_reg4;

    uint8_t dialog_token;
    uint8_t mcs;
    int8_t c_rssi_dbm;
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV))
    int8_t c_snr_ant0;
    int8_t c_snr_ant1;
    uint8_t rsv[3];
#else
    int8_t c_snr_ant0;
    int8_t c_snr_ant1;
    int8_t c_snr_ant2;
    int8_t c_snr_ant3;
    uint8_t rsv;
#endif
} ftm_toda_stru;


typedef struct {
    uint32_t measurement_start_time;
    uint8_t bssid[WLAN_MAC_ADDR_LEN];
    uint32_t bit_range : 24,
               bit_max_range_error_exponent : 8;
    uint8_t reserve[1];
} ftm_range_entry_stru;

typedef struct {
    uint32_t measurement_start_time;
    uint8_t bssid[WLAN_MAC_ADDR_LEN];
    uint8_t error_code;
} ftm_error_entry_stru;

typedef struct {
    uint8_t range_entry_count;
    uint8_t error_entry_count;
    uint8_t reserve[2];

    ftm_range_entry_stru ftm_range_entry[MAX_FTM_RANGE_ENTRY_COUNT];
    ftm_error_entry_stru ftm_error_entry[MAX_FTM_ERROR_ENTRY_COUNT];
} ftm_range_rpt_stru;

typedef struct {
    uint8_t mac[WLAN_MAC_ADDR_LEN];
    uint8_t dialog_token;
    uint8_t meas_token;

    uint16_t num_rpt;

    uint16_t start_delay;
    frw_timeout_stru ftm_range_req_timer;
    uint8_t meas_done;
    uint8_t minimum_ap_count;
    uint8_t bssid[MAX_MINIMUN_AP_COUNT][WLAN_MAC_ADDR_LEN];
    uint8_t channel[MAX_MINIMUN_AP_COUNT];
} ftm_range_req_stru; /* 和mac_send_ftm_range_req_stru 同步修改 */

typedef struct {
    void *hmac_vap;
    void *ftm_session;
} ftm_timeout_arg_stru;

typedef struct {
    uint16_t burst_cnt;     /* 回合个数 */
    uint8_t ftms_per_burst; /* 每个回合FTM帧的个数 */
    uint8_t min_delta_ftm;  /* FTM帧之间的间隙 */

    uint8_t burst_duration;       /* FTM 一个会话中ftm有效交互时间 */
    oal_bool_enum_uint8 is_asap_on; /* 指示 as soon as posible */
    uint16_t burst_period;        /* FTM 一个会话时间 */

    wlan_phy_protocol_enum_uint8 prot_format; /* 指示 协议类型 */
    wlan_bw_cap_enum_uint8 band_cap;          /* 指示 带宽 */
    uint16_t partial_tsf_timer;

    uint32_t tsf_sync_info;

    uint8_t dialog_token;
    uint8_t follow_up_dialog_token;
    uint8_t bssid[WLAN_MAC_ADDR_LEN]; /* FTM交互AP的BSSID */

    /* 命令控制 */
    mac_channel_stru channel_ftm;   /* FTM交互所在信道 */
    oal_bool_enum_uint8 session_on; /* STA that supports fine timing measurement as an initiator */
    oal_bool_enum_uint8 lci_ie;     /* 指示 携带LCI Measurement request/response elements */
    oal_bool_enum_uint8 civic_ie;   /* 指示 携带Location Civic Measurement request/response elements */

    /* 其他 */
    oal_bool_enum_uint8 is_initial_ftm_request; /* 第一帧ftm req已发送,为true时，ftm req不带ftm para */
    uint16_t current_left_burst_cnt; /* multi burst时记录当前还剩余的burst num */
    uint8_t resv;

    uint32_t tx_ftmr_timeout_fix; /* FTM认证，用户设置FTMR的发送时间 */

    frw_timeout_stru ftm_period_timer;   /* ftm回合开始定时器 */
    frw_timeout_stru burst_period_timer; /* ftm整个回合时器 */
    ftm_timeout_arg_stru arg;            /* 定时器超时函数的入参 */

    uint32_t bit_range : 24, /* 记录本次测量的距离和误差 */
               bit_max_error_expt : 8;
    ftm_toda_stru toda_info; /* 记录FTM时间戳相关信息 */
    uint32_t initiator_send_ftm_req_tsf; /* 发送FTM req帧tsf */
    uint32_t initiator_rx_ftm_tsf; /* 收FTM帧tsf */
    int8_t rssi_max;
    int8_t rssi_min;
    uint8_t is_app_wifi_rtt;
} ftm_initiator_stru;

typedef struct {
    uint16_t burst_cnt;     /* 回合个数 */
    uint8_t ftms_per_burst; /* 每个回合FTM帧的个数 */
    uint8_t min_delta_ftm;  /* FTM帧之间的间隙 */

    uint8_t burst_duration;       /* FTM 一个会话中ftm有效交互时间 */
    oal_bool_enum_uint8 is_asap_on; /* 指示 as soon as posible */
    uint16_t burst_period;        /* FTM 一个会话时间 */

    wlan_phy_protocol_enum_uint8 prot_format; /* 指示 协议类型 */
    wlan_bw_cap_enum_uint8 band_cap;          /* 指示 带宽 */
    oal_bool_enum_uint8 received_iftmr;       /* 指示 是否接受过第一帧ftm req */

    uint8_t mac_ra[WLAN_MAC_ADDR_LEN];
    uint8_t dialog_token;
    uint8_t follow_up_dialog_token;

    /* 命令控制 */
    mac_channel_stru channel_ftm; /* FTM交互所在信道 */
    oal_bool_enum_uint8 lci_ie;
    oal_bool_enum_uint8 civic_ie;
    oal_bool_enum_uint8 ftm_parameters;
    oal_bool_enum_uint8 ftm_sync_info;

    frw_timeout_stru ftm_period_timer;   /* ftm回合开始定时器 */
    frw_timeout_stru burst_period_timer; /* ftm整个回合时器 */
    frw_timeout_stru iftm_timer; /* 发iFTM帧定时器 */
    ftm_timeout_arg_stru arg;            /* 定时器超时函数的入参 */

    ftm_toda_stru toda_info; /* 记录FTM时间戳相关信息 */
    uint32_t tsf;          /* 填写tsf */
    uint32_t res_ptsf_offset; /* 设置收到iFTMR帧offset时间后开始FTM会话 */

    uint16_t current_left_burst_cnt; /* multi burst时记录当前还剩余的burst num */
    uint8_t status_indication : 2;
    uint8_t resv0 : 6;
    uint8_t resv1;
} ftm_responder_stru;

typedef struct {
    ftm_initiator_stru ftm_initiator[MAX_FTM_INITIATOR_SESSION];
    ftm_responder_stru ftm_responder[MAX_FTM_RESPONDER_SESSION];
    ftm_range_rpt_stru ftm_range_rpt; /* FTM测量结果 */
    ftm_range_req_stru ftm_range_req; /* FTM测量要求 */

    oal_bool_enum_uint8 report_range;
    oal_bool_enum_uint8 cali;
    uint8_t divider;
    oal_bool_enum_uint8 multipath;
    uint8_t ftm_chain_selection; /* FTM定位通道: */
    uint8_t ftm_rssi_selection;
    uint32_t res_ptsf_delta;
    uint32_t initiator_tsf_fix_offset; /* initiating sta发送第一个FTMR(trigger)帧定时器时间fix offset */

    uint32_t ftm_cali_time;   /* 校准参数 */
    uint32_t air_overhead;
    ftm_initiator_stru *initiator_session; /* 记录中断对应关系 */

#ifdef _PRE_WLAN_FEATURE_CSI
    mac_cfg_csi_param_stru csi_cfg;
#endif
#ifdef _PRE_WLAN_FTM_PPU_SUPPORT
    uint32_t ftm_info[DMAC_FTM_BUFF_SIZE];
#endif
} hmac_ftm_stru;

void hmac_ftm_vap_init_1106(hmac_vap_stru *hmac_vap);

void hmac_ftm_handle(hal_host_location_isr_stru *ftm_csi_isr);

void hmac_ftm_responder_rx_req(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf);

uint32_t hmac_ftm_initiator_rx_rsp(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf);

uint32_t hmac_event_ftm_cali(frw_event_mem_stru *event_mem);

uint32_t hmac_config_wifi_rtt_config_1106(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);

void hmac_rrm_parse_ftm_range_req(hmac_vap_stru *hmac_vap_sta, mac_meas_req_ie_stru *meas_req_ie);

uint32_t hmac_config_ftm_dbg_1106(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);

oal_bool_enum_uint8 hmac_rrm_proc_rm_request_1106(hmac_vap_stru *hmac_vap_sta, oal_netbuf_stru *netbuf);

#endif
#endif
