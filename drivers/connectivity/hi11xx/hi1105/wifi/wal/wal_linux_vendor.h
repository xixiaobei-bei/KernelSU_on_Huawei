

#ifndef __WAL_LINUX_VENDOR_H__
#define __WAL_LINUX_VENDOR_H__

/* 1 其他头文件包含 */
#include "oal_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_VENDOR_H

/* wifi 能力开关状态枚举值，通知到上层使用 */
typedef enum {
    WAL_WIFI_FEATURE_SUPPORT_11K = 0,
    WAL_WIFI_FEATURE_SUPPORT_11V = 1,
    WAL_WIFI_FEATURE_SUPPORT_11R = 2,
    WAL_WIFI_FEATURE_SUPPORT_VOWIFI_NAT_KEEP_ALIVE = 3,
    WAL_WIFI_FEATURE_SUPPORT_NARROWBAND = 4,
    WAL_WIFI_FEATURE_SUPPORT_160M_STA = 5,
    WAL_WIFI_FEATURE_SUPPORT_160M_AP = 6,

    WAL_WIFI_FEATURE_SUPPORT_BUTT
} wal_wifi_feature_capbility_enum;

/*
  私有命令函数表. 私有命令格式:
         设备名 命令名 参数
  hipriv "Hisilicon0 create vap0"
*/
/* private command strings */
#define CMD_SET_AP_WPS_P2P_IE         "SET_AP_WPS_P2P_IE"
#define CMD_SET_MLME_IE               "SET_MLME_IE"
#define CMD_P2P_SET_NOA               "P2P_SET_NOA"
#define CMD_P2P_SET_PS                "P2P_SET_PS"
#define CMD_SET_POWER_ON              "SET_POWER_ON"
#define CMD_SET_POWER_MGMT_ON         "SET_POWER_MGMT_ON"
#define CMD_COUNTRY                   "COUNTRY"
#define CMD_GET_CAPA_DBDC             "GET_CAPAB_RSDB"
#define CMD_CAPA_DBDC_SUPP            "RSDB:1"
#define CMD_CAPA_DBDC_NOT_SUPP        "RSDB:0"
#define CMD_SET_DC_STATE              "SET_DC_STATE"
#ifdef _PRE_WLAN_FEATURE_NRCOEX
#define CMD_SET_NRCOEX_PRIOR          "SET_NRCOEX_PRIOR"
#define CMD_GET_NRCOEX_INFO           "GET_NRCOEX_INFO"
#endif
#define CMD_SET_CALI_FEM_MODE         "SET_CALI_FEM_MODE"
#define CMD_SET_QOS_MAP               "SET_QOS_MAP"
#define CMD_TX_POWER                  "TX_POWER"
#define CMD_WPAS_GET_CUST             "WPAS_GET_CUST"
#define CMD_SET_STA_PM_ON             "SET_STA_PM_ON"
#define CMD_SET_DYNAMIC_DBAC_MODE     "SET_DYNAMIC_DBAC_MODE"

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
/* TAS天线切换命令 */
#define CMD_SET_MEMO_CHANGE           "SET_ANT_CHANGE"
/* 测量天线 */
#define CMD_MEASURE_TAS_RSSI          "SET_TAS_MEASURE"
/* 抬功率 */
#define CMD_SET_TAS_TXPOWER           "SET_TAS_TXPOWER"
/* 获取天线 */
#define CMD_TAS_GET_ANT               "TAS_GET_ANT"
#endif

#define CMD_SET_CLOSE_GO_CAC            "SET_CLOSE_GO_CAC"
#define CMD_SET_CLOSE_GO_CAC_LEN        (OAL_STRLEN(CMD_SET_CLOSE_GO_CAC))
#define CMD_SET_P2P_SCENE               "CMD_SET_P2P_SCENES"
#define CMD_SET_P2P_SCENE_LEN           (OAL_STRLEN(CMD_SET_P2P_SCENE))
#define CMD_SET_GO_DETECT_RADAR         "CMD_SET_GO_DETECT_RADAR"
#define CMD_SET_GO_DETECT_RADAR_LEN     (OAL_STRLEN(CMD_SET_GO_DETECT_RADAR))
#define CMD_SET_CHANGE_GO_CHANNEL       "CMD_SET_CHANGE_GO_CHANNEL"
#define CMD_SET_CHANGE_GO_CHANNEL_LEN   (OAL_STRLEN(CMD_SET_CHANGE_GO_CHANNEL))

#ifdef _PRE_WLAN_FEATURE_M2S
#define CMD_SET_M2S_SWITCH            "SET_M2S_SWITCH"
#define CMD_SET_M2S_BLACKLIST         "SET_M2S_BLACKLIST"
#define CMD_SET_M2S_MODEM             "SET_M2S_MODEM"
#endif

#define CMD_VOWIFI_SET_MODE           "VOWIFI_DETECT SET MODE"
#define CMD_VOWIFI_GET_MODE           "VOWIFI_DETECT GET MODE"
#define CMD_VOWIFI_SET_PERIOD         "VOWIFI_DETECT SET PERIOD"
#define CMD_VOWIFI_GET_PERIOD         "VOWIFI_DETECT GET PERIOD"
#define CMD_VOWIFI_SET_LOW_THRESHOLD  "VOWIFI_DETECT SET LOW_THRESHOLD"
#define CMD_VOWIFI_GET_LOW_THRESHOLD  "VOWIFI_DETECT GET LOW_THRESHOLD"
#define CMD_VOWIFI_SET_HIGH_THRESHOLD "VOWIFI_DETECT SET HIGH_THRESHOLD"
#define CMD_VOWIFI_GET_HIGH_THRESHOLD "VOWIFI_DETECT GET HIGH_THRESHOLD"
#define CMD_VOWIFI_SET_TRIGGER_COUNT  "VOWIFI_DETECT SET TRIGGER_COUNT"
#define CMD_VOWIFI_GET_TRIGGER_COUNT  "VOWIFI_DETECT GET TRIGGER_COUNT"

#define CMD_VOWIFI_SET_PARAM          "VOWIFI_DETECT SET"
#define CMD_VOWIFI_GET_PARAM          "VOWIFI_DETECT GET"

#define CMD_VOWIFI_IS_SUPPORT_REPLY   "true"

#define CMD_VOWIFI_IS_SUPPORT         "VOWIFI_DETECT VOWIFI_IS_SUPPORT"

#define CMD_GET_WIFI_PRIV_FEATURE_CAPABILITY "GET_WIFI_PRIV_FEATURE_CAPABILITY"

#define CMD_SETSUSPENDOPT             "SETSUSPENDOPT"
#define CMD_SETSUSPENDMODE            "SETSUSPENDMODE"

#define CMD_SET_SOFTAP_MIMOMODE       "SET_AP_MODE"

#define CMD_GET_AP_BANDWIDTH          "GET_AP_BANDWIDTH"

#define CMD_GET_VHT160_SUPPORTED      "GET_VHT160_SUPPORTED"

#define CMD_SET_VHT160_FEM_LOWER      "SET_VHT160_FEM_LOWER"

#define CMD_PCIE_ASPM_STATE           "CMD_PCIE_ASPM_STATE"
#define CMD_WLAN_FREQ                 "CMD_WLAN_FREQ"
#define CMD_NAPI_STATE                "CMD_NAPI_STATE"
#define CMD_PM_STATE                  "CMD_PM_STATE"
#define CMD_GSO_STATE                 "CMD_GSO_STATE"
#define CMD_LOW_LATENCY_ON            "CMD_LOW_LATENCY_ON"
#define CMD_LOW_LATENCY_OFF           "CMD_LOW_LATENCY_OFF"
#define CMD_HID2D_PARAMS              "CMD_HID2D_PARAMS"

#define CMD_SET_RX_FILTER_ENABLE       "set_rx_filter_enable"
#define CMD_ADD_RX_FILTER_ITEMS        "add_rx_filter_items"
#define CMD_CLEAR_RX_FILTERS           "clear_rx_filters"
#define CMD_GET_RX_FILTER_PKT_STATE    "get_rx_filter_pkt_state"
#define CMD_FILTER_SWITCH              "FILTER"

#define CMD_GET_APF_PKTS_CNT           "GET_APF_PKTS_CNT"
#define CMD_GET_APF_PKTS_CNT_LEN       (OAL_STRLEN(CMD_GET_APF_PKTS_CNT))
#ifdef _PRE_WLAN_FEATURE_PSM_ABN_PKTS_STAT
#define CMD_GET_ABNORMAL_PKTS_CNT      "GET_ABNORMAL_PKTS_CNT"
#define CMD_GET_ABNORMAL_PKTS_CNT_LEN  (OAL_STRLEN(CMD_GET_ABNORMAL_PKTS_CNT))
#endif
#ifdef _PRE_WLAN_FEATURE_MONITOR
#define CMD_QUERY_SNIFFER      "CMD_QUERY_SNIFFER"
#define CMD_QUERY_SNIFFER_LEN  (OAL_STRLEN(CMD_QUERY_SNIFFER))
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
#define CMD_QUERY_CSI      "CMD_QUERY_CSI"
#define CMD_QUERY_CSI_LEN  (OAL_STRLEN(CMD_QUERY_CSI))
#endif
#define CMD_SET_FASTSLEEP_SWITCH       "SET_FASTSLEEP_SWITCH "
#define CMD_SET_FASTSLEEP_SWITCH_LEN   (OAL_STRLEN(CMD_SET_FASTSLEEP_SWITCH))

#define CMD_GET_FAST_SLEEP_CNT         "GET_FAST_SLEEP_CNT"
#define CMD_GET_FAST_SLEEP_CNT_LEN     (OAL_STRLEN(CMD_GET_FAST_SLEEP_CNT))

#define CMD_GET_BEACON_CNT             "GET_BEACON_CNT"
#define CMD_GET_BEACON_CNT_LEN         (OAL_STRLEN(CMD_GET_BEACON_CNT))

#define CMD_SET_TCP_ACK_CTL             "SET_TCP_ACK_SWITCH "
#define CMD_SET_TCP_ACK_CTL_LEN        (OAL_STRLEN(CMD_SET_TCP_ACK_CTL))

#define CMD_GET_WIFI6_SUPPORT          "CMD_WIFI_CATEGORY"
#define CMD_GET_WIFI6_SUPPORT_LEN      (OAL_STRLEN(CMD_GET_WIFI6_SUPPORT))

#define CMD_SET_AX_CLOSE_HTC            "SET_AX_CLOSE_HTC"
#define CMD_SET_AX_CLOSE_HTC_LEN        (OAL_STRLEN(CMD_SET_AX_CLOSE_HTC))
#define CMD_SET_AX_BLACKLIST            "SET_AX_BLACKLIST"
#define CMD_SET_AX_BLACKLIST_LEN        (OAL_STRLEN(CMD_SET_AX_BLACKLIST))
#define CMD_SET_CLREAR_11N_BLACKLIST    "CMD_CLEAR_11N_BLACKLIST"
#define CMD_SET_CLREAR_11N_BLACKLIST_LEN (OAL_STRLEN(CMD_SET_CLREAR_11N_BLACKLIST))
#define CMD_GET_RADAR_RESULT           "GET_RADAR_RESULT"
#define CMD_GET_RADAR_RESULT_LEN       (OAL_STRLEN(CMD_GET_RADAR_RESULT))
#define CMD_GET_TB_FRAME_GAIN        "GET_TB_FRAME_GAIN"
#define CMD_GET_TB_FRAME_GAIN_LEN    (OAL_STRLEN(CMD_GET_TB_FRAME_GAIN))
#define CMD_RXFILTER_START             "RXFILTER-START"
#define CMD_RXFILTER_STOP              "RXFILTER-STOP"
#define CMD_MIRACAST_START             "MIRACAST 1"
#define CMD_MIRACAST_STOP              "MIRACAST 0"
#define PROTOCOL_DEFAULT               1
#define PROTOCOL_WIFI6_SUPPORT         2

#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
#define CMD_SET_RX_LISTEN_PS_SWITCH    "SET_RX_LISTEN_PS_SWITCH "
#define CMD_SET_RX_LISTEN_PS_SWITCH_LEN (OAL_STRLEN(CMD_SET_RX_LISTEN_PS_SWITCH))
#define CMD_GET_RX_LISTEN_STATE        "GET_RX_LISTEN_STATE"
#define CMD_GET_RX_LISTEN_STATE_LEN    (OAL_STRLEN(CMD_GET_RX_LISTEN_STATE))
#endif

/* 私有命令参数长度宏 */
#define   CMD_SET_AP_WPS_P2P_IE_LEN                              (OAL_STRLEN(CMD_SET_AP_WPS_P2P_IE))
#define   CMD_P2P_SET_NOA_LEN                                    (OAL_STRLEN(CMD_P2P_SET_NOA))
#define   CMD_P2P_SET_PS_LEN                                     (OAL_STRLEN(CMD_P2P_SET_PS))
#define   CMD_SET_STA_PM_ON_LEN                                  (OAL_STRLEN(CMD_SET_STA_PM_ON))
#define   CMD_SET_QOS_MAP_LEN                                    (OAL_STRLEN(CMD_SET_QOS_MAP))
#define   CMD_COUNTRY_LEN                                        (OAL_STRLEN(CMD_COUNTRY))
#define   CMD_LTECOEX_MODE_LEN                                   (OAL_STRLEN(CMD_LTECOEX_MODE))
#define   CMD_TX_POWER_LEN                                       (OAL_STRLEN(CMD_TX_POWER))
#define   CMD_WPAS_GET_CUST_LEN                                  (OAL_STRLEN(CMD_WPAS_GET_CUST))
#define   CMD_VOWIFI_SET_PARAM_LEN                               (OAL_STRLEN(CMD_VOWIFI_SET_PARAM))
#define   CMD_VOWIFI_GET_PARAM_LEN                               (OAL_STRLEN(CMD_VOWIFI_GET_PARAM))
#define   CMD_SETSUSPENDOPT_LEN                                  (OAL_STRLEN(CMD_SETSUSPENDOPT))
#define   CMD_SETSUSPENDMODE_LEN                                 (OAL_STRLEN(CMD_SETSUSPENDMODE))
#define   CMD_GET_WIFI_PRIV_FEATURE_CAPABILITY_LEN               (OAL_STRLEN(CMD_GET_WIFI_PRIV_FEATURE_CAPABILITY))
#define   CMD_VOWIFI_IS_SUPPORT_LEN                              (OAL_STRLEN(CMD_VOWIFI_IS_SUPPORT))
#define   CMD_VOWIFI_IS_SUPPORT_REPLY_LEN                        (OAL_STRLEN(CMD_VOWIFI_IS_SUPPORT_REPLY))
#define   CMD_FILTER_SWITCH_LEN                                  (OAL_STRLEN(CMD_FILTER_SWITCH))
#define   CMD_RXFILTER_START_LEN                                 (OAL_STRLEN(CMD_RXFILTER_START))
#define   CMD_RXFILTER_STOP_LEN                                  (OAL_STRLEN(CMD_RXFILTER_STOP))
#define   CMD_SET_MLME_IE_LEN                                    (OAL_STRLEN(CMD_SET_MLME_IE))
#define   CMD_MIRACAST_START_LEN                                 (OAL_STRLEN(CMD_MIRACAST_START))
#define   CMD_MIRACAST_STOP_LEN                                  (OAL_STRLEN(CMD_MIRACAST_STOP))
#define   CMD_SET_POWER_ON_LEN                                   (OAL_STRLEN(CMD_SET_POWER_ON))
#define   CMD_SET_POWER_MGMT_ON_LEN                              (OAL_STRLEN(CMD_SET_POWER_MGMT_ON))
#define   CMD_GET_CAPA_DBDC_LEN                                  (OAL_STRLEN(CMD_GET_CAPA_DBDC))
#define   CMD_CAPA_DBDC_SUPP_LEN                                 (OAL_STRLEN(CMD_CAPA_DBDC_SUPP))
#define   CMD_CAPA_DBDC_NOT_SUPP_LEN                             (OAL_STRLEN(CMD_CAPA_DBDC_NOT_SUPP))
#define   CMD_SET_TAS_TXPOWER_LEN                                (OAL_STRLEN(CMD_SET_TAS_TXPOWER))
#define   CMD_MEASURE_TAS_RSSI_LEN                               (OAL_STRLEN(CMD_MEASURE_TAS_RSSI))
#define   CMD_TAS_GET_ANT_LEN                                    (OAL_STRLEN(CMD_TAS_GET_ANT))
#define   CMD_SET_MEMO_CHANGE_LEN                                (OAL_STRLEN(CMD_SET_MEMO_CHANGE))
#define   CMD_SET_M2S_BLACKLIST_LEN                              (OAL_STRLEN(CMD_SET_M2S_BLACKLIST))
#define   CMD_GET_AP_BANDWIDTH_LEN                               (OAL_STRLEN(CMD_GET_AP_BANDWIDTH))
#define   CMD_GET_VHT160_SUPPORTED_LEN                           (OAL_STRLEN(CMD_GET_VHT160_SUPPORTED))
#define   CMD_SET_VHT160_FEM_LOWER_LEN                           (OAL_STRLEN(CMD_SET_VHT160_FEM_LOWER))
#define   CMD_SET_DC_STATE_LEN                                   (OAL_STRLEN(CMD_SET_DC_STATE))
#define   CMD_PCIE_ASPM_STATE_LEN                                (OAL_STRLEN(CMD_PCIE_ASPM_STATE))
#define   CMD_WLAN_FREQ_LEN                                      (OAL_STRLEN(CMD_WLAN_FREQ))
#define   CMD_NAPI_STATE_LEN                                     (OAL_STRLEN(CMD_NAPI_STATE))
#define   CMD_PM_STATE_LEN                                       (OAL_STRLEN(CMD_PM_STATE))
#define   CMD_GSO_STATE_LEN                                      (OAL_STRLEN(CMD_GSO_STATE))
#define   CMD_LOW_LATENCY_ON_LEN                                 (OAL_STRLEN(CMD_LOW_LATENCY_ON))
#define   CMD_LOW_LATENCY_OFF_LEN                                (OAL_STRLEN(CMD_LOW_LATENCY_OFF))
#define   CMD_SET_DYNAMIC_DBAC_MODE_LEN                          (OAL_STRLEN(CMD_SET_DYNAMIC_DBAC_MODE))
#define   CMD_HID2D_PARAMS_LEN                                   (OAL_STRLEN(CMD_HID2D_PARAMS))

/* wpa_supplicant 下发命令 */
typedef struct wal_wifi_priv_cmd {
    uint32_t total_len;
    uint32_t used_len;
    uint8_t *puc_buf;
} wal_wifi_priv_cmd_stru;

uint32_t wal_vendor_cmd_gather_handler(oal_net_device_stru *pst_net_dev, int8_t *pc_command);
uint8_t wal_vendor_cmd_gather(int8_t *pc_command);
int32_t wal_vendor_priv_cmd(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr, int32_t cmd);
int32_t wal_netdev_stop(oal_net_device_stru *pst_net_dev);

#endif
