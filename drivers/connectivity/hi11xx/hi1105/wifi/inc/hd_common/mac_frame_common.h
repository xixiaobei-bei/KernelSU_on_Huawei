

#ifndef __MAC_FRAME_COMMON_H__
#define __MAC_FRAME_COMMON_H__

/* 1 其他头文件包含 */
#include "wlan_types.h"
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "oal_util.h"
#include "securec.h"
#include "securectype.h"

/* 此文件中定义的结构体与协议相关，需要1字节对齐 */
/* 2 宏定义 */
#define MAC_80211_FRAME_LEN               24 /* 非四地址情况下，MAC帧头的长度 */
#define MAC_80211_CTL_HEADER_LEN          16 /* 控制帧帧头长度 */
#define MAC_80211_QOS_FRAME_LEN           26
#define MAC_80211_QOS_HTC_FRAME_LEN       30
#define MAC_80211_QOS_HTC_4ADDR_FRAME_LEN 36

/* 信息元素长度定义 */
#define MAC_IE_EXT_HDR_LEN         3  /* 信息元素头部 1字节EID + 1字节长度 + 1字节EXT_EID */
#define MAC_IE_HDR_LEN             2  /* 信息元素头部 1字节EID + 1字节长度 */
#define MAC_NEIGHBOR_REPORT_IE_LEN 13 /* NEIGHBOR_REPORT长度 */
#define MAC_TIME_STAMP_LEN         8
#define MAC_BEACON_INTERVAL_LEN    2
#define MAC_CAP_INFO_LEN           2
#define MAC_MAX_SUPRATES           8 /* WLAN_EID_RATES最大支持8个速率 */
#define MAC_DSPARMS_LEN            1 /* ds parameter set 长度 */
#define MAC_MIN_TIM_LEN            4
#define MAC_MIN_RSN_LEN            12
#define MAC_MAX_RSN_LEN            64
#define MAC_MIN_WPA_LEN            12
#define MAC_PWR_CONSTRAINT_LEN     1 /* 功率限制ie长度为1 */
#define MAC_TPCREP_IE_LEN          2
#define MAC_ERP_IE_LEN             1
#define MAC_OBSS_SCAN_IE_LEN       14

#ifdef _PRE_WLAN_FEATURE_11AX
/* 初始值为5，由于11ac Operating Mode Notification特性标志为bit62长度修改为8,ftm特性需要长度为9, twt特性需要长度为10 */
#define MAC_XCAPS_EX_TWT_LEN 10
#endif
#define MAC_HISI_CAP_VENDOR_IE_LEN 7   /* hisi cap vendor ie字段长度 */ /* 05所用私有ie长度，06长度为10 */

/* 初始值为5，由于11ac Operating Mode Notification特性标志为bit62长度修改为8,ftm特性需要长度为9 */
#define MAC_XCAPS_EX_FTM_LEN 9
/* 初始值为5，由于11ac Operating Mode Notification特性标志为bit62长度修改为8,ftm特性需要长度为9 */
#define MAC_XCAPS_EX_LEN  8
#define MAC_WMM_PARAM_LEN 24 /* WMM parameters ie */

#ifdef _PRE_WLAN_FEATURE_WMMAC
#define MAC_WMMAC_INFO_LEN  61 /* WMMAC info ie */
#define MAC_WMMAC_TSPEC_LEN 55 /* TSPEC元素长度 */
#endif
#define MAC_QOS_INFO_LEN                      1
#define MAC_AC_PARAM_LEN                      4
#define MAC_COUNTRY_REG_FIELD_LEN             3
#define MAC_VHT_CAP_IE_LEN                    12
#define MAC_VHT_INFO_IE_LEN                   5
#define MAC_VHT_CAP_INFO_FIELD_LEN            4
#define MAC_VHT_OPERN_LEN                     5 /* vht opern ie length */
#define MAC_2040_COEX_LEN                     1 /* 20/40 BSS Coexistence element */
#define MAC_2040_INTOLCHREPORT_LEN_MIN        1 /* 20/40 BSS Intolerant Channel Report element */
#define MAC_CHANSWITCHANN_LEN                 3 /* Channel Switch Announcement element */
#define MAC_SA_QUERY_LEN                      4 /* SA Query element len */
#define MAC_HT_NOTIFY_CHANNEL_WIDTH_LEN       3
#define MAC_MIN_WPS_IE_LEN                    5
#define MAC_ACTION_CATEGORY_AND_CODE_LEN      2

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#define MAC_OPMODE_NOTIFY_LEN 1 /* Operating Mode Notification element len */
#endif
#define MAC_MOBILITY_DOMAIN_LEN 5

/* RSN信息元素相关定义 */
#define MAC_RSN_IE_VERSION 1
#define MAC_RSN_CAP_LEN    2

/* OUI相关定义 */
#define MAC_OUI_LEN     3
#define MAC_OUITYPE_LEN 1
#define MAC_OUITYPE_WPA 1
#define MAC_OUITYPE_WMM 2
#define MAC_OUITYPE_P2P 9
#define MAC_WMM_OUI_BYTE_ONE            0x00
#define MAC_WMM_OUI_BYTE_TWO            0x50
#define MAC_WMM_OUI_BYTE_THREE          0xF2
#define MAC_OUISUBTYPE_WMM_INFO         0
#define MAC_OUISUBTYPE_WMM_PARAM        1
#define MAC_OUISUBTYPE_WMM_PARAM_OFFSET 6 /* wmm 字段中EDCA_INFO位置,表示是否携带EDCA参数 偏移6 */
#define MAC_OUI_WMM_VERSION             1
#define MAC_WMM_IE_LEN                  7 /* wmm ie长度为7 */
#define MAC_HT_CAP_LEN            26 /* HT能力信息长度为26 */
#define MAC_HT_CAPINFO_LEN        2  /* HT Capabilities Info域长度为2 */
#define MAC_HT_AMPDU_PARAMS_LEN   1  /* A-MPDU parameters域长度为1 */
#define MAC_HT_SUP_MCS_SET_LEN    16 /* Supported MCS Set域长度为16 */
#define MAC_HT_EXT_CAP_LEN        2  /* Extended cap.域长度为2 */
#define MAC_HT_TXBF_CAP_LEN       4  /* Transmit Beamforming Cap.域长度为4 */
#define MAC_HT_OPERN_LEN          22 /* HT Operation信息长度为22 */
#define MAC_HT_BASIC_MCS_SET_LEN  16 /* HT info中的basic mcs set信息的长度 */

#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
#define MAC_HE_MAC_CAP_LEN               6
#define MAC_HE_PHY_CAP_LEN               11
#define MAC_HE_OPE_PARAM_LEN             3
#define MAC_HE_OPE_BASIC_MCS_NSS_LEN     2
#define MAC_HE_VHT_OPERATION_INFO_LEN    3
#define MAC_HE_UORA_PARAMETER_SET_LEN    2
#define MAC_HE_6GHZ_BAND_CAP_LEN         2
#define MAC_HE_MU_EDCA_PARAMETER_SET_LEN 14
#define MAC_HE_OPERAION_MIN_LEN          7
#define MAC_HE_DEFAULT_PE_DURATION       7    /* 值5-7 reserved */
#define MAC_HE_DURATION_RTS_THRESHOLD    1023 /* 1023表示该字段不可用 */
#define PHY_DEVICE_CLASS_A         1  /* TB功率精度正负3dB */
#define HTC_ONLY_UPH_VALUE           0XFFFFC013  /* 仅含有UPH 字段 */
#define HTC_INVALID_VALUE            0XFFFFFFFF
#define HTC_WITH_OM_MAX_FRAME_NUMS   64
#define MAC_HTC_VHT_HE_PROTOCOL_CONTROL_BIT 2
#define MAC_HTC_HE_CONTROL_ID_BIT           4
#define MAC_HTC_HE_OM_LENGTH                12
#define MAC_HTC_HE_UPH_OR_CAS_LENGTH        8
#define MAC_HTC_HE_BQR_LENGTH               10
#define MAC_HTC_HE_PROTOCOL_BIT0_1  0x3 /* he control */
#ifdef _PRE_WLAN_FEATURE_HIEX
#define HTC_HIMIT_HE_VALUE           0X7B      /* HE模式himit插入HTC字段 */
#define HTC_HIMIT_VHT_VALUE          0X8000001 /* VHT模式himit插入HTC字段 */
#define HTC_HIMIT_HT_OR_LEGACY_VALUE 0X100000  /* HT/LEGACY模式himit插入HTC字段 */
#endif

/* EOCW Range: 0~7(3-bit), the following is EOCW default max/min */
#define MAC_HE_UORA_EOCW_MIN   3
#define MAC_HE_UORA_EOCW_MAX   5
/* OCW: (2^EOCW - 1) */
#define MAC_HE_UORA_OCW_MIN   1
#define MAC_HE_UORA_OCW_MAX   0x7F

typedef enum {
    MAC_FRAME_HE_TRIG_TYPE_BASIC = 0,
    MAC_FRAME_HE_TRIG_TYPE_BRRP = 1,
    MAC_FRAME_HE_TRIG_TYPE_MU_BAR = 2,
    MAC_FRAME_HE_TRIG_TYPE_MU_RTS = 3,
    MAC_FRAME_HE_TRIG_TYPE_BSRP = 4,
    MAC_FRAME_HE_TRIG_TYPE_GCR_MUBAR = 5,
    MAC_FRAME_HE_TRIG_TYPE_BQRP = 6,
    MAC_FRAME_HE_TRIG_TYPE_NFRP = 7,

    MAC_FRAME_HE_TRIG_TYPE_BUTT
} mac_frame_he_trig_type_enum;
typedef uint8_t mac_frame_he_trig_type_enum_uint8;

typedef enum {
    MAC_HTC_A_CONTROL_TYPE_TRS = 0,
    MAC_HTC_A_CONTROL_TYPE_OM = 1,
    MAC_HTC_A_CONTROL_TYPE_HLA = 2,
    MAC_HTC_A_CONTROL_TYPE_BSRP = 3,
    MAC_HTC_A_CONTROL_TYPE_UPH = 4,
    MAC_HTC_A_CONTROL_TYPE_BQR = 5,
    MAC_HTC_A_CONTROL_TYPE_CAS = 6,
#ifdef _PRE_WLAN_FEATURE_HIEX
    MAC_HTC_A_CONTROL_TYPE_HIMIT = 14,
#endif
    MAC_HTC_A_CONTROL_TYPE_INVALID = 15
} mac_htc_a_contorl_type_enum;
typedef uint8_t mac_htc_a_contorl_type_enum_uint8;
#endif

#define MAC_QOS_CTRL_FIELD_OFFSET       24
#define MAC_QOS_CTRL_FIELD_OFFSET_4ADDR 30

/* EDCA参数相关的宏 */
#define MAC_WMM_QOS_PARAM_AIFSN_MASK                    0x0F
#define MAC_WMM_QOS_PARAM_ACI_BIT_OFFSET                5
#define MAC_WMM_QOS_PARAM_ACI_MASK                      0x03
#define MAC_WMM_QOS_PARAM_ECWMIN_MASK                   0x0F
#define MAC_WMM_QOS_PARAM_ECWMAX_MASK                   0xF0
#define MAC_WMM_QOS_PARAM_ECWMAX_BIT_OFFSET             4
#define MAC_WMM_QOS_PARAM_TXOPLIMIT_MASK                0x00FF
#define MAC_WMM_QOS_PARAM_BIT_NUMS_OF_ONE_BYTE          8
#define MAC_WMM_QOS_PARAM_TXOPLIMIT_SAVE_TO_TRANS_TIMES 5

/* 关闭WMM后，所有帧进入此宏定义的队列 */
#define MAC_WMM_SWITCH_TID 6

/* TCP协议类型，chartiot tcp连接端口号 */
#define MAC_TCP_PROTOCAL 6
#define MAC_UDP_PROTOCAL 17

/* ICMP协议报文 */
#define MAC_ICMP_PROTOCAL 1
/* huawei申请多个OUI;
   http://standards-oui.ieee.org/oui.txt */
#define MAC_WLAN_OUI_HUAWEI              0x00E0fC

#ifdef _PRE_WLAN_FEATURE_HIEX
#define MAC_HISI_HIEX_IE    0x81
#endif
#define MAC_HUAWEI_VENDER_IE      0xAC853D /* 打桩HW IE */
#define MAC_HISI_HISTREAM_IE      0x11     /* histream IE */ /* 废弃 */
#define MAC_HISI_LOCATION_CSI_IE  0x13
#define MAC_HISI_CAP_IE 0x20
#define MAC_HISI_1024QAM_IE 0xbd /* 1024QAM IE */
#define MAC_HISI_NB_IE      0x31 /* nb(narrow band) IE */ /* 废弃 */
#define MAC_HISI_SAP_IE 0xb0

#ifdef _PRE_WLAN_FEATURE_MBO
#define MAC_MBO_VENDOR_IE          0x506F9A /* WFA specific OUI */
#define MAC_MBO_IE_OUI_TYPE        0x16     /* Identifying the type and version of the MBO-OCE IE */
#define MAC_MBO_ATTRIBUTE_HDR_LEN  2        /* Attribute ID + Attribute Length */
#endif

#define MAC_IPV6_UDP_SRC_PORT 546
#define MAC_IPV6_UDP_DES_PORT 547
#define MAC_CSI_LOCATION_INFO_LEN   57
#define MAC_FTM_LOCATION_INFO_LEN   52
#define MAC_CSI_REPORT_HEADER_LEN   53
#define MAC_HISI_SAP_IE_LEN 8

/* ARP types, 1: ARP request, 2:ARP response, 3:RARP request, 4:RARP response */
#define MAC_ARP_REQUEST   0x0001
#define MAC_ARP_RESPONSE  0x0002

/* Neighbor Discovery */
#define MAC_ND_RSOL  133 /* Router Solicitation */
#define MAC_ND_RADVT 134 /* Router Advertisement */
#define MAC_ND_NSOL  135 /* Neighbor Solicitation */
#define MAC_ND_NADVT 136 /* Neighbor Advertisement */
#define MAC_ND_RMES  137 /* Redirect Message */

/* p2p相关 */
#define P2P_PAF_GON_REQ  0
#define WFA_OUI_BYTE1       0x50
#define WFA_OUI_BYTE2       0x6F
#define WFA_OUI_BYTE3       0x9A
#define WFA_P2P_V1_0        0x09

#define MAC_VHT_CHANGE (BIT1)
#define MAC_HT_CHANGE  (BIT2)
#define MAC_BW_CHANGE  (BIT3)
#ifdef _PRE_WLAN_FEATURE_11AX
#define MAC_HE_BSS_COLOR_CHANGE         (BIT7)
#define MAC_HE_PARTIAL_BSS_COLOR_CHANGE (BIT8)
#endif

#define MAC_NO_CHANGE        (0)
#define MAC_RADIO_MEAS_START_TIME_LEN 8

#ifdef _PRE_WLAN_FEATURE_PRIV_CLOSED_LOOP_TPC
#define MAC_HISI_ADJUST_POW_IE              0x50  /* 功率调整IE */
#endif

/* 3 枚举定义 */
typedef enum {
    MAC_AP_TYPE_NORMAL = BIT0,
    MAC_AP_TYPE_GOLDENAP = BIT1,
    MAC_AP_TYPE_DDC_WHITELIST = BIT2,
    MAC_AP_TYPE_BTCOEX_PS_BLACKLIST = BIT3,
    MAC_AP_TYPE_BTCOEX_DISABLE_CTS = BIT4,
    MAC_AP_TYPE_TPLINK = BIT5,
    MAC_AP_TYPE_M2S = BIT6,
    MAC_AP_TYPE_ROAM = BIT7,
    MAC_AP_TYPE_160M_OP_MODE = BIT8,
    MAC_AP_TYPE_AGGR_BLACKLIST = BIT9,
    MAC_AP_TYPE_MIMO_BLACKLIST = BIT10,
    MAC_AP_TYPE_BTCOEX_SCO_RETRY_MIDDLE_PRIORITY = BIT11,
    MAC_AP_TYPE_BTCOEX_SCO_MIDDLE_PRIORITY = BIT12,
    MAC_AP_TYPE_BTCOEX_20DBM_BLACKLIST = BIT13,
    MAC_AP_TYPE_MCM_DEGRADARION_WRITELIST = BIT14,
    MAC_AP_TYPE_BUTT
} mac_ap_type_enum;
typedef uint16_t mac_ap_type_enum_uint16;

/* Action Frames: Category字段枚举 */
typedef enum {
    MAC_ACTION_CATEGORY_SPECMGMT = 0,
    MAC_ACTION_CATEGORY_QOS = 1,
    MAC_ACTION_CATEGORY_DLS = 2,
    MAC_ACTION_CATEGORY_BA = 3,
    MAC_ACTION_CATEGORY_PUBLIC = 4,
    MAC_ACTION_CATEGORY_RADIO_MEASURMENT = 5,
    MAC_ACTION_CATEGORY_FAST_BSS_TRANSITION = 6,
    MAC_ACTION_CATEGORY_HT = 7,
    MAC_ACTION_CATEGORY_SA_QUERY = 8,
    MAC_ACTION_CATEGORY_PROTECTED_DUAL_OF_ACTION = 9,
    MAC_ACTION_CATEGORY_WNM = 10,
    MAC_ACTION_CATEGORY_MESH = 13,
    MAC_ACTION_CATEGORY_MULTIHOP = 14,
#ifdef _PRE_WLAN_FEATURE_WMMAC
    MAC_ACTION_CATEGORY_WMMAC_QOS = 17,
#endif
    MAC_ACTION_CATEGORY_VHT = 21,
#ifdef _PRE_WLAN_FEATURE_11AX
    MAC_ACTION_CATEGORY_S1G = 22,
#endif
    MAC_ACTION_CATEGORY_HE = 30,
    MAC_ACTION_CATEGORY_VENDOR_SPECIFIC_PROTECTED = 126,
    MAC_ACTION_CATEGORY_VENDOR = 127,
} mac_action_category_enum;
typedef uint8_t mac_category_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_11AX
/* S1G下的Action值的枚举 */
typedef enum {
    MAC_S1G_ACTION_AID_SWITCH_REQ = 0,
    MAC_S1G_ACTION_AID_SWITCH_RESP = 1,
    MAC_S1G_ACTION_SYNC_CONTROL = 2,
    MAC_S1G_ACTION_STA_INFORMATION_ANNOUNCEMENT = 3,
    MAC_S1G_ACTION_EDCA_PARAMETER_SET = 4,
    MAC_S1G_ACTION_EL_OPERATION = 5,
    MAC_S1G_ACTION_TWT_SETUP = 6,
    MAC_S1G_ACTION_TWT_TEARDOWN = 7,
    MAC_S1G_ACTION_SECTORIZED_GROUP_ID_LIST = 8,
    MAC_S1G_ACTION_SECTOR_ID_FEEDBACK = 9,
    MAC_S1G_ACTION_RESERVE = 10,
    MAC_S1G_ACTION_TWT_INFORMATION = 11,
    MAC_S1G_ACTION_BUTT
} mac_s1g_action_type_enum;
typedef uint8_t mac_s1g_action_type_enum_uint8;
#endif

/* HT Category下的Action值的枚举 */
typedef enum {
    MAC_HT_ACTION_NOTIFY_CHANNEL_WIDTH = 0,
    MAC_HT_ACTION_SMPS = 1,
    MAC_HT_ACTION_PSMP = 2,
    MAC_HT_ACTION_SET_PCO_PHASE = 3,
    MAC_HT_ACTION_CSI = 4,
    MAC_HT_ACTION_NON_COMPRESSED_BEAMFORMING = 5,
    MAC_HT_ACTION_COMPRESSED_BEAMFORMING = 6,
    MAC_HT_ACTION_ASEL_INDICES_FEEDBACK = 7,

    MAC_HT_ACTION_BUTT
} mac_ht_action_type_enum;
typedef uint8_t mac_ht_action_type_enum_uint8;

/* SA QUERY Category下的Action值的枚举 */
typedef enum {
    MAC_SA_QUERY_ACTION_REQUEST = 0,
    MAC_SA_QUERY_ACTION_RESPONSE = 1
} mac_sa_query_action_type_enum;
typedef uint8_t mac_sa_query_action_type_enum_uint8;

/* VHT Category下的Action值的枚举 */
typedef enum {
    MAC_VHT_ACTION_COMPRESSED_BEAMFORMING = 0,
    MAC_VHT_ACTION_GROUPID_MANAGEMENT = 1,
    MAC_VHT_ACTION_OPREATING_MODE_NOTIFICATION = 2,

    MAC_VHT_ACTION_BUTT
} mac_vht_action_type_enum;
typedef uint8_t mac_vht_action_type_enum_uint8;

/* HE Category下的Action值的枚举 */
typedef enum {
    MAC_HE_ACTION_COMPRESSED_BEAMFORMING_AND_CQI = 0,
    MAC_HE_ACTION_QUICK_TIME_PERIOD = 1,
    MAC_HE_ACTION_OPS = 2,

    MAC_HE_ACTION_BUTT
} mac_he_action_type_enum;
typedef uint8_t mac_he_action_type_enum_uint8;

#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
/* WNM Category下的Action值的枚举 */
typedef enum {
    MAC_WNM_ACTION_EVENT_REQUEST = 0,
    MAC_WNM_ACTION_EVENT_REPORT = 1,
    MAC_WNM_ACTION_DIALGNOSTIC_REQUEST = 2,
    MAC_WNM_ACTION_DIALGNOSTIC_REPORT = 3,
    MAC_WNM_ACTION_LOCATION_CONF_REQUEST = 4,
    MAC_WNM_ACTION_LOCATION_CONF_RESPONSE = 5,
    MAC_WNM_ACTION_BSS_TRANSITION_MGMT_QUERY = 6,
    MAC_WNM_ACTION_BSS_TRANSITION_MGMT_REQUEST = 7,
    MAC_WNM_ACTION_BSS_TRANSITION_MGMT_RESPONSE = 8,
    MAC_WNM_ACTION_FMS_REQUEST = 9,
    MAC_WNM_ACTION_FMS_RESPONSE = 10,
    MAC_WNM_ACTION_COLLOCATED_INTER_REQUEST = 11,
    MAC_WNM_ACTION_COLLOCATEC_INTER_REPORT = 12,
    MAC_WNM_ACTION_TFS_REQUEST = 13,
    MAC_WNM_ACTION_TFS_RESPONSE = 14,
    MAC_WNM_ACTION_TFS_NOTIFY = 15,
    MAC_WNM_ACTION_SLEEP_MODE_REQUEST = 16,
    MAC_WNM_ACTION_SLEEP_MODE_RESPONSE = 17,
    MAC_WNM_ACTION_TIM_BROADCAST_REQUEST = 18,
    MAC_WNM_ACTION_TIM_BROADCAST_RESPONSE = 19,
    MAC_WNM_ACTION_QOS_TRAFFIC_CAP_UPDATE = 20,
    MAC_WNM_ACTION_CHANNEL_USAGE_REQUEST = 21,
    MAC_WNM_ACTION_CHANNEL_USAGE_RESPONSE = 22,
    MAC_WNM_ACTION_DMS_REQUEST = 23,
    MAC_WNM_ACTION_DMS_RESPONSE = 24,
    MAC_WNM_ACTION_TIMING_MEAS_REQUEST = 25,
    MAC_WNM_ACTION_NOTIFICATION_REQUEST = 26,
    MAC_WNM_ACTION_NOTIFICATION_RESPONSE = 27,

    MAC_WNM_ACTION_BUTT
} mac_wnm_action_type_enum;
typedef uint8_t mac_wnm_action_type_enum_uint8;
#endif  // _PRE_WLAN_FEATURE_11V_ENABLE

/* BlockAck Category下的Action值的枚举 */
typedef enum {
    MAC_BA_ACTION_ADDBA_REQ = 0,
    MAC_BA_ACTION_ADDBA_RSP = 1,
    MAC_BA_ACTION_DELBA = 2,

    MAC_BA_ACTION_BUTT
} mac_ba_action_type_enum;
typedef uint8_t mac_ba_action_type_enum_uint8;

/* Public Category下的Action枚举值 */
typedef enum {
    MAC_PUB_COEXT_MGMT = 0,            /* 20/40 BSS Coexistence Management */
    MAC_PUB_EX_CH_SWITCH_ANNOUNCE = 4, /* Extended Channel Switch Announcement */
    MAC_PUB_VENDOR_SPECIFIC = 9,
    MAC_PUB_GAS_INIT_REQ = 10,
    MAC_PUB_GAS_INIT_RESP = 11, /* public Action: GAS Initial Response(0x0b) */
    MAC_PUB_GAS_COMBAK_REQ = 12,
    MAC_PUB_GAS_COMBAK_RESP = 13, /* public Action: GAS Comeback Response(0x0d) */
    MAC_PUB_FTM_REQ = 32,
    MAC_PUB_FTM = 33
} mac_public_action_type_enum;
typedef uint8_t mac_public_action_type_enum_uint8;
#ifdef _PRE_WLAN_FEATURE_WMMAC
/* WMMAC中TSPEC相关ACTION的枚举值 */
typedef enum {
    MAC_WMMAC_ACTION_ADDTS_REQ = 0,
    MAC_WMMAC_ACTION_ADDTS_RSP = 1,
    MAC_WMMAC_ACTION_DELTS = 2,

    MAC_WMMAC_ACTION_BUTT
} mac_wmmac_action_type_enum;

/* ADDTS REQ中TSPEC Direction元素的枚举值 */
typedef enum {
    MAC_WMMAC_DIRECTION_UPLINK = 0,
    MAC_WMMAC_DIRECTION_DOWNLINK = 1,
    MAC_WMMAC_DIRECTION_RESERVED = 2,
    MAC_WMMAC_DIRECTION_BIDIRECTIONAL = 3,

    MAC_WMMAC_DIRECTION_BUTT
} mac_wmmac_direction_enum;
typedef uint8_t mac_wmmac_direction_enum_uint8;
/* TS建立的状态枚举 */
typedef enum {
    MAC_TS_NONE = 0,   /* TS不需要建立 */
    MAC_TS_INIT,       /* TS需要建立，未建立 */
    MAC_TS_INPROGRESS, /* TS建立过程中 */
    MAC_TS_SUCCESS,    /* TS建立成功 */

    MAC_TS_BUTT
} mac_ts_conn_status_enum;
typedef uint8_t mac_ts_conn_status_enum_uint8;
#endif

/* Block ack的确认类型 */
typedef enum {
    MAC_BACK_BASIC = 0,
    MAC_BACK_COMPRESSED = 2,
    MAC_BACK_MULTI_TID = 3,

    MAC_BACK_BUTT
} mac_back_variant_enum;
typedef uint8_t mac_back_variant_enum_uint8;

/* ACTION帧中，各个域的偏移量 */
typedef enum {
    MAC_ACTION_OFFSET_CATEGORY = 0,
    MAC_ACTION_OFFSET_ACTION = 1,
} mac_action_offset_enum;
typedef uint8_t mac_action_offset_enum_uint8;

/* Reason Codes for Deauthentication and Disassociation Frames */
typedef enum {
    MAC_UNSPEC_REASON = 1,
    MAC_AUTH_NOT_VALID = 2,
    MAC_DEAUTH_LV_SS = 3,
    MAC_INACTIVITY = 4,
    MAC_AP_OVERLOAD = 5,
    MAC_NOT_AUTHED = 6,
    MAC_NOT_ASSOCED = 7,
    MAC_DISAS_LV_SS = 8,
    MAC_ASOC_NOT_AUTH = 9,
    MAC_INVLD_ELEMENT = 13,
    MAC_MIC_FAIL = 14,
    MAC_4WAY_HANDSHAKE_TIMEOUT = 15,
    MAC_IEEE_802_1X_AUTH_FAIL = 23,
    MAC_UNSPEC_QOS_REASON = 32,
    MAC_QAP_INSUFF_BANDWIDTH = 33,
    MAC_POOR_CHANNEL = 34,
    MAC_STA_TX_AFTER_TXOP = 35,
    MAC_QSTA_LEAVING_NETWORK = 36,
    MAC_QSTA_INVALID_MECHANISM = 37,
    MAC_QSTA_SETUP_NOT_DONE = 38,
    MAC_QSTA_TIMEOUT = 39,
    MAC_QSTA_CIPHER_NOT_SUPP = 45
} mac_reason_code_enum;
typedef uint16_t mac_reason_code_enum_uint16;

/* Status Codes for Authentication and Association Frames */
typedef enum {
    MAC_SUCCESSFUL_STATUSCODE = 0,
    MAC_UNSPEC_FAIL = 1,
    MAC_UNSUP_CAP = 10,
    MAC_REASOC_NO_ASOC = 11,
    MAC_FAIL_OTHER = 12,
    MAC_UNSUPT_ALG = 13,
    MAC_AUTH_SEQ_FAIL = 14,
    MAC_CHLNG_FAIL = 15,
    MAC_AUTH_TIMEOUT = 16,
    MAC_AP_FULL = 17,
    MAC_UNSUP_RATE = 18,
    MAC_SHORT_PREAMBLE_UNSUP = 19,
    MAC_PBCC_UNSUP = 20,
    MAC_CHANNEL_AGIL_UNSUP = 21,
    MAC_MISMATCH_SPEC_MGMT = 22,
    MAC_MISMATCH_POW_CAP = 23,
    MAC_MISMATCH_SUPP_CHNL = 24,
    MAC_SHORT_SLOT_UNSUP = 25,
    MAC_OFDM_DSSS_UNSUP = 26,
    MAC_MISMATCH_HTCAP = 27,
    MAC_R0KH_UNREACHABLE = 28,
    MAC_MISMATCH_PCO = 29,
    MAC_REJECT_TEMP = 30,
    MAC_MFP_VIOLATION = 31,
    MAC_UNSPEC_QOS_FAIL = 32,
    MAC_QAP_INSUFF_BANDWIDTH_FAIL = 33,
    MAC_POOR_CHANNEL_FAIL = 34,
    MAC_REMOTE_STA_NOT_QOS = 35,
    MAC_REQ_DECLINED = 37,
    MAC_INVALID_REQ_PARAMS = 38,
    MAC_RETRY_NEW_TSPEC = 39,
    MAC_INVALID_INFO_ELMNT = 40,
    MAC_INVALID_GRP_CIPHER = 41,
    MAC_INVALID_PW_CIPHER = 42,
    MAC_INVALID_AKMP_CIPHER = 43,
    MAC_UNSUP_RSN_INFO_VER = 44,
    MAC_INVALID_RSN_INFO_CAP = 45,
    MAC_CIPHER_REJ = 46,
    MAC_RETRY_TS_LATER = 47,
    MAC_DLS_NOT_SUPP = 48,
    MAC_DST_STA_NOT_IN_QBSS = 49,
    MAC_DST_STA_NOT_QSTA = 50,
    MAC_LARGE_LISTEN_INT = 51,
    MAC_FINITE_CYCLIC_GROUP_NOT_SUPPORTED = 77,
    MAC_MISMATCH_VHTCAP = 104,

    /* 私有的定义 */
    MAC_JOIN_RSP_TIMEOUT = 5200,  /* 状态机没从join跳到auth超时 */
    MAC_AUTH_RSP2_TIMEOUT = 5201, /* auth seq2没收到auth rsp */
    MAC_AUTH_RSP4_TIMEOUT = 5202, /* auth seq4没收到auth rsp */
    MAC_ASOC_RSP_TIMEOUT = 5203,  /* asoc req发出去没收到asoc rsp */

    MAC_AUTH_REQ_SEND_FAIL_BEGIN = 5250,
    MAC_AUTH_REQ_SEND_FAIL_NO_ACK = 5251,  /* auth req发出去没收到ack */
    MAC_AUTH_REQ_SEND_FAIL_TIMEOUT = 5252, /* auth req发出去lifetime超时（没法送出去） */
    MAC_AUTH_REQ_SEND_FAIL_ABORT = 5261,   /* auth req发送失败(因为蓝牙abort) */

    MAC_ASOC_REQ_SEND_FAIL_BEGIN = 5300,
    MAC_ASOC_REQ_SEND_FAIL_NO_ACK = 5301,  /* asoc req发出去没收到ack */
    MAC_ASOC_REQ_SEND_FAIL_TIMEOUT = 5302, /* asoc req发出去lifetime超时（没法送出去） */
    MAC_ASOC_REQ_SEND_FAIL_ABORT = 5311,   /* asoc req发送失败(因为蓝牙abort) */

    MAC_AP_AUTH_RSP_TIMEOUT = 6100, /* softap auth rsp发送完wait asoc req超时 */
} mac_status_code_enum;
typedef uint16_t mac_status_code_enum_uint16;

/* BA会话管理确认策略 */
typedef enum {
    MAC_BA_POLICY_DELAYED = 0,
    MAC_BA_POLICY_IMMEDIATE,

    MAC_BA_POLICY_BUTT
} mac_ba_policy_enum;
typedef uint8_t mac_ba_policy_enum_uint8;

/* 发起DELBA帧的端点的枚举 */
typedef enum {
    MAC_RECIPIENT_DELBA = 0, /* 数据的接收端 */
    MAC_ORIGINATOR_DELBA,    /* 数据的发起端 */

    MAC_BUTT_DELBA
} mac_delba_initiator_enum;
typedef uint8_t mac_delba_initiator_enum_uint8;

/* 发起DELBA帧的业务类型的枚举 */
typedef enum {
    MAC_DELBA_TRIGGER_COMM = 0, /* 配置命令触发 */
    MAC_DELBA_TRIGGER_BTCOEX,   /* BT业务触发 */
    MAC_DELBA_TRIGGER_PS,       /* PS业务触发 */

    MAC_DELBA_TRIGGER_BUTT
} mac_delba_trigger_enum;
typedef uint8_t mac_delba_trigger_enum_uint8;

/*****************************************************************************
  信息元素(Infomation Element)的Element ID
  协议521页，Table 8-54—Element IDs
*****************************************************************************/
typedef enum {
    MAC_EID_SSID = 0,
    MAC_EID_RATES = 1,
    MAC_EID_FHPARMS = 2,
    MAC_EID_DSPARMS = 3,
    MAC_EID_CFPARMS = 4,
    MAC_EID_TIM = 5,
    MAC_EID_IBSSPARMS = 6,
    MAC_EID_COUNTRY = 7,
    MAC_EID_REQINFO = 10,
    MAC_EID_QBSS_LOAD = 11,
    MAC_EID_TSPEC = 13,
    MAC_EID_TCLAS = 14,
    MAC_EID_CHALLENGE = 16,
    /* 17-31 reserved */
    MAC_EID_PWRCNSTR = 32,
    MAC_EID_PWRCAP = 33,
    MAC_EID_TPCREQ = 34,
    MAC_EID_TPCREP = 35,
    MAC_EID_SUPPCHAN = 36,
    MAC_EID_CHANSWITCHANN = 37, /* Channel Switch Announcement IE */
    MAC_EID_MEASREQ = 38,
    MAC_EID_MEASREP = 39,
    MAC_EID_QUIET = 40,
    MAC_EID_IBSSDFS = 41,
    MAC_EID_ERP = 42,
    MAC_EID_TCLAS_PROCESS = 44,
    MAC_EID_HT_CAP = 45,
    MAC_EID_QOS_CAP = 46,
    MAC_EID_RESERVED_47 = 47,
    MAC_EID_RSN = 48,
    MAC_EID_RESERVED_49 = 49,
    MAC_EID_XRATES = 50,
    MAC_EID_AP_CHAN_REPORT = 51,
    MAC_EID_NEIGHBOR_REPORT = 52,
    MAC_EID_MOBILITY_DOMAIN = 54,
    MAC_EID_FT = 55,
    MAC_EID_TIMEOUT_INTERVAL = 56,
    MAC_EID_RDE = 57,
    MAC_EID_OPERATING_CLASS = 59,  /* Supported Operating Classes */
    MAC_EID_EXTCHANSWITCHANN = 60, /* Extended Channel Switch Announcement IE */
    MAC_EID_HT_OPERATION = 61,
    MAC_EID_SEC_CH_OFFSET = 62, /* Secondary Channel Offset IE */
    MAC_EID_WAPI = 68,          /* IE for WAPI */
    MAC_EID_TIME_ADVERTISEMENT = 69,
    MAC_EID_RRM = 70,                /* Radio resource measurement */
    MAC_EID_MULTI_BSSID = 71,        /* Multiple BSSID Element   */
    MAC_EID_2040_COEXT = 72,         /* 20/40 BSS Coexistence IE */
    MAC_EID_2040_INTOLCHREPORT = 73, /* 20/40 BSS Intolerant Channel Report IE */
    MAC_EID_OBSS_SCAN = 74,          /* Overlapping BSS Scan Parameters IE */
    MAC_EID_MMIE = 76,               /* 802.11w Management MIC IE */
    MAC_EID_NONTRANSMITTED_BSSID_CAP = 83,
    MAC_EID_MULTI_BSSID_INDEX = 85,  /*  m-bssid 相关 */
    MAC_EID_FMS_DESCRIPTOR = 86,     /* 802.11v FMS descriptor IE */
    MAC_EID_FMS_REQUEST = 87,        /* 802.11v FMS request IE */
    MAC_EID_FMS_RESPONSE = 88,       /* 802.11v FMS response IE */
    MAC_EID_BSSMAX_IDLE_PERIOD = 90, /* BSS MAX IDLE PERIOD */
    MAC_EID_TFS_REQUEST = 91,
    MAC_EID_TFS_RESPONSE = 92,
    MAC_EID_TIM_BCAST_REQUEST = 94,
    MAC_EID_TIM_BCAST_RESPONSE = 95,
    MAC_EID_INTERWORKING = 107,
    MAC_EID_ADVERTISEMENT_PROTOCOL = 108,
    MAC_EID_EXT_CAPS = 127, /* Extended Capabilities IE */
    MAC_EID_VHT_TYPE = 129,
    MAC_EID_11NTXBF = 130, /* 802.11n txbf */ /* 废弃 */
    MAC_EID_RESERVED_133 = 133,
    MAC_EID_TPC = 150,
    MAC_EID_CCKM = 156,
    MAC_EID_VHT_CAP = 191,
    MAC_EID_VHT_OPERN = 192,         /* VHT Operation IE */
    MAC_EID_WIDE_BW_CH_SWITCH = 194, /* Wide Bandwidth Channel Switch IE */
    MAC_EID_OPMODE_NOTIFY = 199,     /* Operating Mode Notification IE */
    MAC_EID_FTMP = 206,
#ifdef _PRE_WLAN_FEATURE_11AX
    MAC_EID_TWT = 216,
#endif
    MAC_EID_FTMSI = 255,
    MAC_EID_VENDOR = 221, /* vendor private */
    MAC_EID_WMM = 221,
    MAC_EID_WPA = 221,
    MAC_EID_WPS = 221,
    MAC_EID_P2P = 221,
    MAC_EID_WFA_TPC_RPT = 221,
    MAC_EID_ADJUST_POW_PRIVATE = 221, /* private pow adjust vendor */
    MAC_EID_EXTENSION = 255,
#ifdef _PRE_WLAN_FEATURE_11AX
    MAC_EID_HE = 255,
#endif
} mac_eid_enum;
typedef uint8_t mac_eid_enum_uint8;

/* Element ID Extension (EID 255) values */
typedef enum {
    MAC_EID_EXT_FTMSI = 9,
    MAC_EID_EXT_OWE_DH_PARAM = 32,
#ifdef _PRE_WLAN_FEATURE_11AX
    MAC_EID_EXT_HE_CAP = 35,
    MAC_EID_EXT_HE_OPERATION = 36,
    MAC_EID_EXT_UORA_PARAMETER_SET = 37,
    MAC_EID_EXT_HE_EDCA = 38,
    MAC_EID_EXT_HE_SRP = 39,
    MAC_EID_EXT_HE_NDP_FEEDBACK_REPORT = 41,
    MAC_EID_EXT_HE_BSS_COLOR_CHANGE_ANNOUNCEMENT = 42,
    MAC_EID_EXT_HE_6GHZ_BAND_CAP = 59,
#endif

    MAC_EID_EXT_BUT
} mac_eid_extension_enum;
typedef uint8_t mac_eid_extension_enum_uint8;

typedef enum {
    MAC_SMPS_STATIC_MODE = 0,  /*   静态SMPS   */
    MAC_SMPS_DYNAMIC_MODE = 1, /*   动态SMPS   */
    MAC_SMPS_MIMO_MODE = 3,    /* disable SMPS */

    MAC_SMPS_MODE_BUTT
} mac_mimo_power_save_enum;
typedef uint8_t mac_mimo_power_save_mode_enum_uint8;

typedef enum {
    MAC_SCN = 0, /* 不存在次信道 */
    MAC_SCA = 1, /* 次信道在主信道之上(Secondary Channel Above) */
    MAC_SCB = 3, /* 次信道在主信道之下(Secondary Channel Below) */

    MAC_SEC_CH_BUTT,
} mac_sec_ch_off_enum;
typedef uint8_t mac_sec_ch_off_enum_uint8;

typedef enum {
    P2P_PUB_ACT_OUI_OFF1 = 2,
    P2P_PUB_ACT_OUI_OFF2 = 3,
    P2P_PUB_ACT_OUI_OFF3 = 4,
    P2P_PUB_ACT_OUI_TYPE_OFF = 5,
    P2P_PUB_ACT_OUI_SUBTYPE_OFF = 6,
    P2P_PUB_ACT_DIALOG_TOKEN_OFF = 7,
    P2P_PUB_ACT_TAG_PARAM_OFF = 8
} p2p_pub_act_frm_off;

/* Radio Measurement下的Action枚举值 */
typedef enum {
    MAC_RM_ACTION_RADIO_MEASUREMENT_REQUEST = 0,
    MAC_RM_ACTION_RADIO_MEASUREMENT_REPORT = 1,
    MAC_RM_ACTION_LINK_MEASUREMENT_REQUEST = 2,
    MAC_RM_ACTION_LINK_MEASUREMENT_REPORT = 3,
    MAC_RM_ACTION_NEIGHBOR_REPORT_REQUEST = 4,
    MAC_RM_ACTION_NEIGHBOR_REPORT_RESPONSE = 5
} mac_rm_action_type_enum;
typedef uint8_t mac_rm_action_type_enum_uint8;

/* 度量类型的枚举 */
/* Basic 0
Clear Channel Assessment (CCA) 1
Receive Power Indication (RPI) Histogram 2
Channel Load 3
Noise Histogram 4
Beacon 5
Frame 6
STA Statistics 7
LCI 8
Transmit Stream/Category Measurement 9
Multicast Diagnostics 10
Location Civic 11
Location Identifier 12
Directional Channel Quality 13
Directional Measurement 14
Directional Statistics 15
Fine Timing Measurement Range 16
Reserved 17–254
Measurement Pause 255 */
typedef enum {
    RM_RADIO_MEASUREMENT_BASIC = 0,
    RM_RADIO_MEAS_CHANNEL_LOAD = 3,
    RM_RADIO_MEAS_BCN = 5,
    RM_RADIO_MEAS_FRM = 6,
    RM_RADIO_MEAS_STA_STATS = 7,
    RM_RADIO_MEASUREMENT_LCI = 8,
    RM_RADIO_MEAS_TSC = 9,
    RM_RADIO_MEASUREMENT_LOCATION_CIVIC = 11,
    RM_RADIO_MEASUREMENT_FTM_RANGE = 16,
    RM_RADIO_MEAS_PAUSE = 255
} mac_radio_meas_type_enum;
typedef uint8_t mac_radio_meas_type_enum_uint8;

typedef enum {
    RM_BCN_REQ_MEAS_MODE_PASSIVE = 0,
    RM_BCN_REQ_MEAS_MODE_ACTIVE = 1,
    RM_BCN_REQ_MEAS_MODE_TABLE = 2,
    RM_BCN_REQ_MEAS_MODE_BUTT
} rm_bcn_req_meas_mode_enum;
typedef uint8_t rm_bcn_req_meas_mode_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_TWT
/* TWT命令类型 */
typedef enum {
    MAC_TWT_COMMAND_REQUEST = 0,
    MAC_TWT_COMMAND_SUGGEST = 1,
    MAC_TWT_COMMAND_DEMAND = 2,
    MAC_TWT_COMMAND_GROUPING = 3,
    MAC_TWT_COMMAND_ACCEPT = 4,
    MAC_TWT_COMMAND_ALTERNATE = 5,
    MAC_TWT_COMMAND_DICTATE = 6,
    MAC_TWT_COMMAND_REJECT = 7,
} mac_twt_command_enum;
typedef uint8_t mac_twt_command_enum_uint8;

typedef enum {
    MAC_TWT_NEGO_ZERO = 0,
    MAC_TWT_NEGO_INDIVIDUAL = 1,
    MAC_TWT_NEGO_BROADCAST = 2,
    MAC_TWT_NEGO_TBTT = 3,
} mac_twt_nego_enum;
typedef uint8_t mac_twt_nego_enum_uint8;
#endif

#ifdef _PRE_WLAN_FEATURE_MBO
typedef enum {
    MBO_ATTR_ID_AP_CAPA_IND = 1,
    MBO_ATTR_ID_NON_PREF_CHAN_REPORT = 2,
    MBO_ATTR_ID_CELL_DATA_CAPA = 3,
    MBO_ATTR_ID_ASSOC_DISALLOW = 4,
    MBO_ATTR_ID_CELL_DATA_PREF = 5,
    MBO_ATTR_ID_TRANSITION_REASON = 6,
    MBO_ATTR_ID_TRANSITION_REJECT_REASON = 7,
    MBO_ATTR_ID_ASSOC_RETRY_DELAY = 8,
} mac_mbo_attr_id_enum;
typedef uint8_t mac_mbo_attr_id_enum_uint8;
#endif

typedef enum {
    MAC_SAP_OFF = 0,
    MAC_SAP_MASTER = 1,
    MAC_SAP_SLAVE = 2,
} mac_sap_mode_enum;
typedef uint8_t mac_sap_mode_enum_uint8;

#define MAC_WLAN_OUI_WFA                0x506f9a
#define MAC_WLAN_OUI_TYPE_WFA_P2P       9
#define MAC_WLAN_OUI_MICROSOFT          0x0050f2
#define MAC_WLAN_OUI_TYPE_MICROSOFT_WMM 2
#define MAC_WLAN_OUI_TYPE_MICROSOFT_WPS 4
#define MAC_WLAN_OUI_VENDOR_VHT_HEADER   5
#define MAC_WLAN_OUI_BROADCOM_EPIGRAM    0x00904c /* Broadcom (Epigram) */
#define MAC_WLAN_OUI_VENDOR_VHT_TYPE     0x04

/* eapol key 结构宏定义 */
#define WPA_REPLAY_COUNTER_LEN    8
#define WPA_NONCE_LEN             32
#define WPA_KEY_RSC_LEN           8
#define IEEE802_1X_TYPE_EAPOL_KEY 3
#define WPA_KEY_INFO_KEY_TYPE     BIT(3) /* 1 = Pairwise, 0 = Group key */
#define WPA_KEY_INFO_KEY_ACK BIT(7)
#define WPA_KEY_INFO_KEY_MIC BIT(0)

/* EAPOL数据帧子类型枚举定义 */
typedef enum {
    MAC_EAPOL_PTK_1_4 = 1,
    MAC_EAPOL_PTK_2_4,
    MAC_EAPOL_PTK_3_4,
    MAC_EAPOL_PTK_4_4,

    MAC_EAPOL_PTK_BUTT
} mac_eapol_type_enum_uint8;

/* P2P OUI 定义 */
extern const uint8_t g_auc_p2p_oui[MAC_OUI_LEN];

/* WFA TPC RPT OUI 定义 */
extern const uint8_t g_auc_wfa_oui[MAC_OUI_LEN];

/* 窄带 OUI 定义 */
extern const uint8_t g_auc_huawei_oui[MAC_OUI_LEN];

/* 6 消息定义 */
/* 7 STRUCT定义 */
/* 用#pragma pack(1)/#pragma pack()方式达到一字节对齐 */
#pragma pack(1)

/* 此文件中结构体与协议一致，要求1字节对齐，统一加__OAL_DECLARE_PACKED */
struct mac_ether_header {
    uint8_t auc_ether_dhost[ETHER_ADDR_LEN];
    uint8_t auc_ether_shost[ETHER_ADDR_LEN];
    uint16_t us_ether_type;
} __OAL_DECLARE_PACKED;
typedef struct mac_ether_header mac_ether_header_stru;

struct mac_llc_snap {
    uint8_t uc_llc_dsap;
    uint8_t uc_llc_ssap;
    uint8_t uc_control;
    uint8_t auc_org_code[NUM_3_BYTES];
    uint16_t us_ether_type;
} __OAL_DECLARE_PACKED;
typedef struct mac_llc_snap mac_llc_snap_stru;

/* eapol帧头 */
struct mac_eapol_header {
    uint8_t uc_version;
    uint8_t uc_type;
    uint16_t us_length;
} __OAL_DECLARE_PACKED;
typedef struct mac_eapol_header mac_eapol_header_stru;

/* IEEE 802.11, 8.5.2 EAPOL-Key frames */
/* EAPOL KEY 结构定义 */
struct mac_eapol_key {
    uint8_t uc_type;
    /* Note: key_info, key_length, and key_data_length are unaligned */
    uint8_t auc_key_info[NUM_2_BYTES];   /* big endian */
    uint8_t auc_key_length[NUM_2_BYTES]; /* big endian */
    uint8_t auc_replay_counter[WPA_REPLAY_COUNTER_LEN];
    uint8_t auc_key_nonce[WPA_NONCE_LEN];
    uint8_t auc_key_iv[NUM_16_BYTES];
    uint8_t auc_key_rsc[WPA_KEY_RSC_LEN];
    uint8_t auc_key_id[NUM_8_BYTES]; /* Reserved in IEEE 802.11i/RSN */
    uint8_t auc_key_mic[NUM_16_BYTES];
    uint8_t auc_key_data_length[NUM_2_BYTES]; /* big endian */
    /* followed by key_data_length bytes of key_data */
} __OAL_DECLARE_PACKED;
typedef struct mac_eapol_key mac_eapol_key_stru;

/* DC header for higame 2.0 */
struct mac_higame_dc_header {
        uint16_t seq;  /* 用于higame 2.0的重复帧过滤 */
        uint16_t type; /* 承载詄ther hdr中的type */
} __OAL_DECLARE_PACKED;
typedef struct mac_higame_dc_header mac_higame_dc_header_stru;

/*
 * Structure of the IP frame
 */
struct mac_ip_header {
    uint8_t uc_version_ihl;
    uint8_t uc_tos;
    uint16_t us_tot_len;
    uint16_t us_id;
    uint16_t us_frag_off;
    uint8_t uc_ttl;
    uint8_t uc_protocol;
    uint16_t us_check;
    uint32_t saddr;
    uint32_t daddr;
    /* The options start here. */
} __OAL_DECLARE_PACKED;
typedef struct mac_ip_header mac_ip_header_stru;

struct mac_tcp_header {
    uint16_t us_sport;
    uint16_t us_dport;
    uint32_t seqnum;
    uint32_t acknum;
    uint8_t uc_offset;
    uint8_t uc_flags;
    uint16_t us_window;
    uint16_t us_check;
    uint16_t us_urgent;
} __OAL_DECLARE_PACKED;
typedef struct mac_tcp_header mac_tcp_header_stru;

/* UDP头部结构 */
typedef struct {
    uint16_t us_src_port;
    uint16_t us_des_port;
    uint16_t us_udp_len;
    uint16_t us_check_sum;
} udp_hdr_stru;
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
/* hid2d头部结构，偏移prefix_udp之后 */
typedef struct {
    uint32_t bit_nalu_num : 8,
               bit_spn : 4,
               bit_data_type : 4,    /* 数据类型：区分音频、控制流 */
               bit_deadline : 16;    /* 单位为32us,允许发包截止时间 */
}hid2d_video_hdr_stru;
#endif
/* frame control字段结构体 */
struct mac_header_frame_control {
    uint16_t bit_protocol_version : 2, /* 协议版本 */
               bit_type : 2,                    /* 帧类型 */
               bit_sub_type : 4,                /* 子类型 */
               bit_to_ds : 1,                   /* 发送DS */
               bit_from_ds : 1,                 /* 来自DS */
               bit_more_frag : 1,               /* 分段标识 */
               bit_retry : 1,                   /* 重传帧 */
               bit_power_mgmt : 1,              /* 节能管理 */
               bit_more_data : 1,               /* 更多数据标识 */
               bit_protected_frame : 1,         /* 加密标识 */
               bit_order : 1;                   /* 次序位 */
} __OAL_DECLARE_PACKED;
typedef struct mac_header_frame_control mac_header_frame_control_stru;

/* 基础802.11帧结构 */
struct mac_ieee80211_frame {
    mac_header_frame_control_stru st_frame_control;
    uint16_t bit_duration_value : 15,
               bit_duration_flag  : 1;
    uint8_t auc_address1[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address2[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address3[WLAN_MAC_ADDR_LEN];
    uint16_t bit_frag_num : 4,
               bit_seq_num : 12;
} __OAL_DECLARE_PACKED;
typedef struct mac_ieee80211_frame mac_ieee80211_frame_stru;

/* ps poll帧结构 */
struct mac_ieee80211_pspoll_frame {
    mac_header_frame_control_stru st_frame_control;
    uint16_t bit_aid_value : 14, /* ps poll 下的AID字段 */
               bit_aid_flag1 : 1,
               bit_aid_flag2 : 1;
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    uint8_t auc_trans_addr[WLAN_MAC_ADDR_LEN];
} __OAL_DECLARE_PACKED;
typedef struct mac_ieee80211_pspoll_frame mac_ieee80211_pspoll_frame_stru;

/* qos帧结构 */
struct mac_ieee80211_qos_frame {
    mac_header_frame_control_stru st_frame_control;
    uint16_t bit_duration_value : 15, /* duration/id */
               bit_duration_flag : 1;
    uint8_t auc_address1[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address2[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address3[WLAN_MAC_ADDR_LEN];
    uint16_t bit_frag_num : 4, /* sequence control */
               bit_seq_num : 12;
    uint8_t bit_qc_tid : 4,
              bit_qc_eosp : 1,
              bit_qc_ack_polocy : 2,
              bit_qc_amsdu : 1;
    union {
        uint8_t bit_qc_txop_limit;
        uint8_t bit_qc_ps_buf_state_resv : 1,
                  bit_qc_ps_buf_state_inducated : 1,
                  bit_qc_hi_priority_buf_ac : 2,
                  bit_qc_qosap_buf_load : 4;
    } qos_control;
} __OAL_DECLARE_PACKED;
typedef struct mac_ieee80211_qos_frame mac_ieee80211_qos_frame_stru;

/* qos+HTC 帧结构 */
struct mac_ieee80211_qos_htc_frame {
    mac_header_frame_control_stru st_frame_control;
    uint16_t bit_duration_value : 15, /* duration/id */
               bit_duration_flag : 1;
    uint8_t auc_address1[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address2[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address3[WLAN_MAC_ADDR_LEN];
    uint16_t bit_frag_num : 4, /* sequence control */
               bit_seq_num : 12;
    uint8_t bit_qc_tid : 4,
              bit_qc_eosp : 1,
              bit_qc_ack_polocy : 2,
              bit_qc_amsdu : 1;
    union {
        uint8_t bit_qc_txop_limit;
        uint8_t bit_qc_ps_buf_state_resv : 1,
                  bit_qc_ps_buf_state_inducated : 1,
                  bit_qc_hi_priority_buf_ac : 2,
                  bit_qc_qosap_buf_load : 4;
    } qos_control;

    uint32_t htc;
} __OAL_DECLARE_PACKED;
typedef struct mac_ieee80211_qos_htc_frame mac_ieee80211_qos_htc_frame_stru;

/* 四地址帧结构体 */
struct mac_ieee80211_frame_addr4 {
    mac_header_frame_control_stru st_frame_control;
    uint16_t
    bit_duration_value : 15,
                       bit_duration_flag : 1;
    uint8_t auc_address1[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address2[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address3[WLAN_MAC_ADDR_LEN];
    uint16_t bit_frag_num : 4,
               bit_seq_num : 12;
    uint8_t auc_address4[WLAN_MAC_ADDR_LEN];
} __OAL_DECLARE_PACKED;
typedef struct mac_ieee80211_frame_addr4 mac_ieee80211_frame_addr4_stru;

/* qos四地址帧结构 */
struct mac_ieee80211_qos_frame_addr4 {
    mac_header_frame_control_stru st_frame_control;
    uint16_t bit_duration_value : 15, /* duration/id */
               bit_duration_flag : 1;
    uint8_t auc_address1[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address2[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address3[WLAN_MAC_ADDR_LEN];
    uint16_t bit_frag_num : 4, /* sequence control */
               bit_seq_num : 12;
    uint8_t auc_address4[WLAN_MAC_ADDR_LEN];
    uint8_t bit_qc_tid : 4,
              bit_qc_eosp : 1,
              bit_qc_ack_polocy : 2,
              bit_qc_amsdu : 1;
    union {
        uint8_t qc_txop_limit;                /* txop limit字段 */
        uint8_t qc_queue_size;                /* queue size字段 */
        uint8_t bit_qc_ps_buf_state_resv : 1, /* AP PS Buffer State */
                  bit_qc_ps_buf_state_inducated : 1,
                  bit_qc_hi_priority_buf_ac : 2,
                  bit_qc_qosap_buf_load : 4;
    } qos_control;
} __OAL_DECLARE_PACKED;
typedef struct mac_ieee80211_qos_frame_addr4 mac_ieee80211_qos_frame_addr4_stru;

/* qos htc 四地址帧结构 */
struct mac_ieee80211_qos_htc_frame_addr4 {
    mac_header_frame_control_stru st_frame_control;
    uint16_t bit_duration_value : 15, /* duration/id */
               bit_duration_flag : 1;
    uint8_t auc_address1[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address2[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address3[WLAN_MAC_ADDR_LEN];
    uint16_t bit_frag_num : 4, /* sequence control */
               bit_seq_num : 12;
    uint8_t auc_address4[WLAN_MAC_ADDR_LEN];
    uint8_t bit_qc_tid : 4,
              bit_qc_eosp : 1,
              bit_qc_ack_polocy : 2,
              bit_qc_amsdu : 1;
    union {
        uint8_t qc_txop_limit;                /* txop limit字段 */
        uint8_t qc_queue_size;                /* queue size字段 */
        uint8_t bit_qc_ps_buf_state_resv : 1, /* AP PS Buffer State */
                  bit_qc_ps_buf_state_inducated : 1,
                  bit_qc_hi_priority_buf_ac : 2,
                  bit_qc_qosap_buf_load : 4;
    } qos_control;

    uint32_t htc;
} __OAL_DECLARE_PACKED;
typedef struct mac_ieee80211_qos_htc_frame_addr4 mac_ieee80211_qos_htc_frame_addr4_stru;

/* Ref. 802.11-2012.pdf, 8.4.1.4 Capability information field, 中文注释参考白皮书 */
struct mac_cap_info {
    uint16_t bit_ess : 1,          /* 由BSS中的AP设置为1 */
               bit_ibss : 1,                /* 由一个IBSS中的站点设置为1，ap总是设置其为0 */
               bit_cf_pollable : 1,         /* 标识CF-POLL能力 */
               bit_cf_poll_request : 1,     /* 标识CF-POLL能力  */
               bit_privacy : 1,             /* 1=需要加密, 0=不需要加密 */
               bit_short_preamble : 1,      /* 802.11b短前导码 */
               bit_pbcc : 1,                /* 802.11g */
               bit_channel_agility : 1,     /* 802.11b */
               bit_spectrum_mgmt : 1,       /* 频谱管理: 0=不支持, 1=支持 */
               bit_qos : 1,                 /* QOS: 0=非QOS站点, 1=QOS站点 */
               bit_short_slot_time : 1,     /* 短时隙: 0=不支持, 1=支持 */
               bit_apsd : 1,                /* 自动节能: 0=不支持, 1=支持 */
               bit_radio_measurement : 1,   /* Radio检测: 0=不支持, 1=支持 */
               bit_dsss_ofdm : 1,           /* 802.11g */
               bit_delayed_block_ack : 1,   /* 延迟块确认: 0=不支持, 1=支持 */
               bit_immediate_block_ack : 1; /* 立即块确认: 0=不支持, 1=支持 */
} __OAL_DECLARE_PACKED;
typedef struct mac_cap_info mac_cap_info_stru;

/* Ref. 802.11-2012.pdf, 8.4.2.58.2 HT Capabilities Info field */
struct mac_frame_ht_cap {
    uint16_t bit_ldpc_coding_cap : 1,  /* LDPC 编码 capability    */
               bit_supported_channel_width : 1, /* STA 支持的带宽          */
               bit_sm_power_save : 2,           /* SM 省电模式             */
               bit_ht_green_field : 1,          /* 绿野模式                */
               bit_short_gi_20mhz : 1,          /* 20M下短保护间隔         */
               bit_short_gi_40mhz : 1,          /* 40M下短保护间隔         */
               bit_tx_stbc : 1,                 /* Indicates support for the transmission of PPDUs using STBC */
               bit_rx_stbc : 2,                 /* 支持 Rx STBC            */
               bit_ht_delayed_block_ack : 1,    /* Indicates support for HT-delayed Block Ack opera-tion. */
               bit_max_amsdu_length : 1,        /* Indicates maximum A-MSDU length. */
               bit_dsss_cck_mode_40mhz : 1,     /* 40M下 DSSS/CCK 模式     */
               bit_resv : 1,
               /*
                * Indicates whether APs receiving this information or reports of this informa-tion are
                * required to prohibit 40 MHz transmissions
                */
               bit_forty_mhz_intolerant : 1,
               bit_lsig_txop_protection : 1; /* 支持L-SIG TXOP保护 */
} __OAL_DECLARE_PACKED;
typedef struct mac_frame_ht_cap mac_frame_ht_cap_stru;

struct mac_vht_cap_info {
    uint32_t bit_max_mpdu_length : 2,
               bit_supported_channel_width : 2,
               bit_rx_ldpc : 1,
               bit_short_gi_80mhz : 1,
               bit_short_gi_160mhz : 1,
               bit_tx_stbc : 1,
               bit_rx_stbc : 3,
               bit_su_beamformer_cap : 1,
               bit_su_beamformee_cap : 1,
               bit_num_bf_ant_supported : 3,
               bit_num_sounding_dim : 3,
               bit_mu_beamformer_cap : 1,
               bit_mu_beamformee_cap : 1,
               bit_vht_txop_ps : 1,
               bit_htc_vht_capable : 1,
               bit_max_ampdu_len_exp : 3,
               bit_vht_link_adaptation : 2,
               bit_rx_ant_pattern : 1,
               bit_tx_ant_pattern : 1,
               bit_extend_nss_bw_supp : 2;
};
typedef struct mac_vht_cap_info mac_vht_cap_info_stru;

#define NOA_IE_ONE_NOA_LEN 13
#define NOA_IE_TWO_NOA_LEN 26
#define NOA_PERIOD_NOA_CNT 255

struct noa_ie_param {
    uint8_t  noacnt;
    uint32_t noaduration;
    uint32_t noainterval;
    uint32_t noastatrtime;
}__OAL_DECLARE_PACKED;
typedef struct noa_ie_param noa_ie_param_stru;

#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
struct mac_frame_he_trig {
    /* byte 0 */
    uint16_t bit_trig_type : 4;
    uint16_t bit_ppdu_len : 12;

    uint32_t bit_cascade : 1; /* 当前skb是否为amsdu的首个skb */
    uint32_t bit_cs_required : 1;
    uint32_t bit_bandwith : 2;
    uint32_t bit_ltf_gi : 2;
    uint32_t bit_mu_mimo_ltf_mode : 1;
    uint32_t bit_num_of_ltfs : 3;
    uint32_t bit_stbc : 1;
    uint32_t bit_ldpc_extra_symbol : 1;
    uint32_t bit_ap_tx_pwr : 6;
    uint32_t bit_pe_fec_pading : 2;
    uint32_t bit_pe_disambiguity : 1;

    uint32_t bit_spatial_reuse : 16;
    uint32_t bit_dopler : 1;
    uint32_t bit_he_siga : 9;
    uint32_t bit_resv : 1;
} __OAL_DECLARE_PACKED;
typedef struct mac_frame_he_trig mac_frame_he_trig_stru;

struct mac_frame_trig_depend_user_info {
    uint8_t bit_mu_spacing_factor : 2;
    uint8_t bit_multi_tid_aggr_limit : 3;
    uint8_t bit_resv : 1;
    uint8_t bit_perfer_ac : 2;
} __OAL_DECLARE_PACKED;
typedef struct mac_frame_trig_depend_user_info mac_frame_trig_depend_user_info_stru;

struct mac_frame_he_trig_user_info {
    /* byte 0 */
    uint32_t bit_user_aid : 12;
    uint32_t bit_ru_location : 8;
    uint32_t bit_coding_type : 1;
    uint32_t bit_mcs : 4;
    uint32_t bit_dcm : 1;
    uint32_t bit_starting_spatial_stream : 3;
    uint32_t bit_nss : 3;

    uint8_t bit_target_rssi : 7;
    uint8_t bit_resv : 1;

    mac_frame_trig_depend_user_info_stru st_user_info;
} __OAL_DECLARE_PACKED;
typedef struct mac_frame_he_trig_user_info mac_frame_he_trig_user_info_stru;

/* HE_Cap:he mac cap 字段，Len=6  */
struct mac_frame_he_mac_cap {
    /* B0-指示站点是否支持接收Qos Data、Qos Null、管理帧携带 HE 变体的HT Control field */
    uint8_t bit_htc_he_support : 1,
              bit_twt_requester_support : 1, /* B1-是否支持TWT Requester */
              bit_twt_responder_support : 1, /* B2-是否支持TWT Responder */
              bit_fragmentation_support : 2, /* B3-动态分片能力 */
              bit_msdu_max_fragment_num : 3; /* B5-msdu最大分片数 */

    uint16_t bit_min_fragment_size : 2,        /* B8-最小分片的长度 */
               bit_trigger_mac_padding_duration : 2,    /* B10-trigger mac padding 时长 */
               bit_mtid_aggregation_rx_support : 3,     /* B12-接收多tid聚合支持 */
               bit_he_link_adaptation : 2,              /* B15-使用HE 变体的HT Control field 调整 */
               bit_all_ack_support : 1,                 /* B17-支持接收M-BA */
               bit_trs_support : 1,                    /* B18-A-Control 支持 UL MU Response Scheduling */
               bit_bsr_support : 1,                     /* B19-A-Control 支持BSR */
               bit_broadcast_twt_support : 1,           /* B20-支持广播twt */
               bit_32bit_ba_bitmap_support : 1,         /* B21-支持32位bitmap ba */
               bit_mu_cascading_support : 1,            /* B22-mu csacade */
               bit_ack_enabled_aggregation_support : 1; /* B23-A-MPDU ack使能 */

    uint8_t bit_reserved_b24 : 1,           /* B24- */
              bit_om_control_support : 1,           /* B25-A-Control 支持OMI  */
              bit_ofdma_ra_support : 1,             /* B26-OFDMA 随机接入 */
              bit_max_ampdu_length_exponent : 2,    /* B27-A-MPDU最大长度 */
              bit_amsdu_fragment_support : 1,       /* B29-A-MSDU Fragmentation Support */
              bit_flex_twt_schedule_support : 1,    /* B30- */
              bit_rx_control_frame_to_multibss : 1; /* B31- */

    uint16_t bit_bsrp_bqrp_ampdu_addregation : 1,       /* B32- */
               bit_qtp_support : 1,                              /* B33- */
               bit_bqr_support : 1,                              /* B34- HTC BQR */
               bit_srp_responder : 1,                             /* B35- */
               bit_ndp_feedback_report_support : 1,              /* B36- */
               bit_ops_support : 1,                              /* B37- */
               bit_amsdu_no_ba_ack_enabled_support : 1,          /* B38-ack enabled related */
               bit_mtid_aggregation_tx_support : 3,              /* B39-41 */
               bit_he_subchannel_select_transmission : 1,        /* B42 */
               bit_ul_2x996_ru_support : 1,                      /* B43 */
               bit_om_control_ul_mu_data_disable_rx_support : 1, /* B44 */
               bit_he_dynamic_sm_power_save : 1,                 /* B45 */
               bit_punctured_sounding_support : 1,               /* B46 */
               bit_he_and_vht_trigger_frame_rx_support : 1;      /* B47 */
} __OAL_DECLARE_PACKED;
typedef struct mac_frame_he_mac_cap mac_frame_he_mac_cap_stru;

/* HE_CAP: PHY Cap 字段 Len=11   */
struct mac_frame_he_phy_cap {
    uint8_t bit_reserved : 1,    /* B0  */
              bit_channel_width_set : 7; /* B1-7: */

    uint16_t bit_punctured_preamble_rx : 4, /* B8-11: */
               bit_device_class : 1,                 /* B12- */
               bit_ldpc_coding_in_paylod : 1,        /* B13- */
               bit_he_su_ppdu_1x_he_ltf_08us_gi : 1,     /* B14-HE SU PPDU 1x HE-LTF + 0.8usGI  */
               bit_midamble_trx_max_nsts : 2,         /* B15 */
               bit_ndp_4x_he_ltf_32us_gi : 1,               /* B17-ndp 4x HE-LTF + 3.2usGI */
               bit_stbc_tx_below_80mhz : 1,          /* B18 */
               bit_stbc_rx_below_80MHz : 1,          /* B19 */
               bit_doppler_tx : 1,                   /* B20 */
               bit_doppler_rx : 1,                   /* B21 */
               bit_full_bandwidth_ul_mu_mimo : 1,    /* B22 */
               bit_partial_bandwidth_ul_mu_mimo : 1; /* B23 */

    uint8_t bit_dcm_max_constellation_tx : 2, /* B24- */
              bit_dcm_max_nss_tx : 1,                 /* B26 */
              bit_dcm_max_constellation_rx : 2,       /* B27- */
              bit_dcm_max_nss_rx : 1,                 /* B29- */
              bit_rx_partial_bw_su_in_20mhz_mu_ppdu : 1, /* B30- */
              bit_su_beamformer : 1;                  /* B31- */

    uint8_t bit_su_beamformee : 1,        /* B32- */
              bit_mu_beamformer : 1,              /* B33- */
              bit_beamformee_sts_below_80mhz : 3, /* B34- */
              bit_beamformee_sts_over_80mhz : 3;  /* B37- */

    uint8_t bit_below_80mhz_sounding_dimensions_num : 3, /* B40- */
              bit_over_80mhz_sounding_dimensions_num : 3,        /* B43- */
              bit_ng16_su_feedback : 1,                          /* B46- */
              bit_ng16_mu_feedback : 1;                          /* B47- */

    uint8_t bit_codebook_42_su_feedback : 1,             /* B48- */
              bit_codebook_75_mu_feedback : 1,                   /* B49- */
              bit_trigger_su_beamforming_feedback : 1,           /* B50- */
              bit_trigger_mu_beamforming_partialbw_feedback : 1, /* B51- */
              bit_trigger_cqi_feedback : 1,                      /* B52- */
              bit_partial_bandwidth_extended_range : 1,          /* B53- */
              bit_partial_bandwidth_dl_mu_mimo : 1,              /* B54- */
              bit_ppe_threshold_present : 1;                     /* B55- */

    uint8_t bit_srp_based_sr_support : 1,  /* B56- */
              bit_power_boost_factor_support : 1,  /* B57- */
              bit_he_su_ppdu_he_mu_ppdu_4x_he_ltf_08us_gi : 1, /* B58-he su ppdu and he mu ppdu 4x HE-LTF + 0.8usGI */
              bit_max_nc : 3,                      /* B59- */
              bit_stbc_tx_over_80mhz : 1,          /* B62- */
              bit_stbc_rx_over_80mhz : 1;          /* B63- */

    uint8_t bit_he_er_su_ppdu_4x_he_ltf_08us_gi : 1, /* B64- */
              bit_20mhz_in_40mhz_he_ppdu_2g : 1,         /* B65- */
              bit_20mhz_in_160mhz_he_ppdu : 1,           /* B66- */
              bit_80mhz_in_160mhz_he_ppdu : 1,           /* B67- */
              bit_he_er_su_ppdu_1x_he_ltf_08us_gi : 1,       /* B68- */
              bit_midamble_trx_2x_1x_he_ltf : 1,          /* B69- */
              bit_dcm_max_ru : 2;                        /* B70-71 */

    uint16_t bit_longer_than_16_he_sigb_support : 1,       /* B72- */
               bit_non_triggered_cqi_feedback : 1,                  /* B73 */
               bit_tx_1024qam_below_242ru_support : 1,                    /* B74 */
               bit_rx_1024qam_below_242ru_support : 1,                    /* B75 */
               bit_rx_full_bw_su_ppdu_with_compressed_sigb : 1,     /* B76 */
               bit_rx_full_bw_su_ppdu_with_non_compressed_sigb : 1, /* B77 */
               bit_nominal_packet_padding : 2,                      /* B78_79: */
               bit_reserv : 8;                                      /* B80_87: */
} __OAL_DECLARE_PACKED;
typedef struct mac_frame_he_phy_cap mac_frame_he_phy_cap_stru; /* 协议规定的 phy cap 格式 */

/* HE CAP:Tx Rx MCS NSS Support */
struct mac_frame_he_mcs_nss_bit_map {
    uint16_t bit_max_he_mcs_1ss : 2,
             bit_max_he_mcs_2ss : 2,
             bit_max_he_mcs_3ss : 2,
             bit_max_he_mcs_4ss : 2,
             bit_max_he_mcs_5ss : 2,
             bit_max_he_mcs_6ss : 2,
             bit_max_he_mcs_7ss : 2,
             bit_max_he_mcs_8ss : 2;
} __OAL_DECLARE_PACKED;
typedef struct mac_frame_he_mcs_nss_bit_map mac_frame_he_basic_mcs_nss_stru;

struct mac_fram_he_mac_nss_set {
    mac_frame_he_basic_mcs_nss_stru st_rx_he_mcs_map_below_80mhz;
    mac_frame_he_basic_mcs_nss_stru st_tx_he_mcs_map_below_80mhz;
    mac_frame_he_basic_mcs_nss_stru st_rx_he_mcs_map_160mhz;
    mac_frame_he_basic_mcs_nss_stru st_tx_he_mcs_map_160mhz;
    mac_frame_he_basic_mcs_nss_stru st_rx_he_mcs_map_8080mhz;
    mac_frame_he_basic_mcs_nss_stru st_tx_he_mcs_map_8080mhz;
} __OAL_DECLARE_PACKED;
typedef struct mac_fram_he_mac_nss_set mac_frame_he_supported_he_mcs_nss_set_stru;

/* HE CAP:PPE Thresholds */
/* 该IE用于声明设备接收SU PPDU时,支持的MAC PADDING类型, 当前受限160M接收PAD 16us */
typedef struct {
    uint32_t          bit_nss                                                 :3,
                        bit_ru_index_mask                                       :4,
                        bit_ppet16_nss0_ru0                                     :3,
                        bit_ppet8_nss0_ru0                                      :3,
                        bit_ppet16_nss0_ru1                                     :3,
                        bit_ppet8_nss0_ru1                                      :3,
                        bit_ppet16_nss1_ru0                                     :3,
                        bit_ppet8_nss1_ru0                                      :3,
                        bit_ppet16_nss1_ru1                                     :3,
                        bit_ppet8_nss1_ru1                                      :3,
                        bit_ppe_pad                                             :1;

}__OAL_DECLARE_PACKED mac_frame_ppe_thresholds_pre_field_stru;

typedef struct {
    mac_frame_he_mac_cap_stru st_he_mac_cap;
    mac_frame_he_phy_cap_stru st_he_phy_cap;
    mac_frame_he_supported_he_mcs_nss_set_stru st_he_mcs_nss;
} __OAL_DECLARE_PACKED mac_frame_he_cap_ie_stru;

struct mac_frame_he_mcs_nss {
    uint8_t bit_mcs : 4,
              bit_nss : 3,
              bit_last_mcs_nss : 1;
} __OAL_DECLARE_PACKED;
typedef struct mac_frame_he_mcs_nss mac_frame_he_mcs_nss_stru;

/* 协议规定帧格式 */
struct mac_frame_he_operation_param {
    uint16_t bit_default_pe_duration : 3, /* B0_2: */
               bit_twt_required : 1,               /* B3: */
               bit_he_duration_rts_threshold : 10, /* B4_13: */
               bit_vht_operation_info_present : 1, /* B14: */
               bit_co_located_bss : 1;             /* B15 */

    uint8_t bit_er_su_disable : 1,  /* B16； */
              bit_6g_oper_info_present : 1, /* B17: */
              bit_reserved : 6;             /* B18_23: */
} __OAL_DECLARE_PACKED;
typedef struct mac_frame_he_operation_param mac_frame_he_operation_param_stru;

struct mac_frame_he_bss_color_info {
    uint8_t bit_bss_color : 6,
              bit_partial_bss_color : 1,
              bit_bss_color_disable : 1;
} __OAL_DECLARE_PACKED;
typedef struct mac_frame_he_bss_color_info mac_frame_he_bss_color_info_stru;

struct mac_frame_vht_operation_info {
    uint8_t uc_channel_width;
    uint8_t uc_center_freq_seg0;
    uint8_t uc_center_freq_seg1;
} __OAL_DECLARE_PACKED;
typedef struct mac_frame_vht_operation_info mac_frame_vht_operation_info_stru;

typedef struct
{
    mac_frame_he_operation_param_stru st_he_oper_param;
    mac_frame_he_bss_color_info_stru st_bss_color;
    mac_frame_he_basic_mcs_nss_stru st_he_basic_mcs_nss;
    mac_frame_vht_operation_info_stru st_vht_operation_info;
} __OAL_DECLARE_PACKED mac_frame_he_oper_ie_stru;

typedef struct {
    uint8_t bit_uora_eocw_min : 3,
            bit_uora_eocw_max : 3,
            bit_uora_capable  : 1,
            bit_resv : 1;
} __OAL_DECLARE_PACKED mac_he_uora_para_stru;

typedef struct {
    uint16_t min_mpdu_start_spacing : 3,
             max_ampdu_len_exponent : 3,
             max_mpdu_len : 2,
             resv : 1,
             sm_power_save : 2,
             rd_responder : 1,
             rx_ant_pattern : 1,
             tx_ant_pattern : 1,
             resv_1 : 2;
} __OAL_DECLARE_PACKED mac_he_6ghz_band_cap_info_stru;

/* he mu edca param */
typedef struct
{
    uint8_t bit_edca_update_count : 4,
              bit_q_ack : 1,
              bit_queue_request : 1,
              bit_txop_request : 1,
              bit_more_data_ack : 1;
} __OAL_DECLARE_PACKED mac_frame_he_mu_qos_info;

typedef struct
{
    uint8_t bit_aifsn : 4,
              bit_acm : 1,
              bit_ac_index : 2,
              bit_reserv : 1;
    uint8_t bit_ecw_min : 4,
              bit_ecw_max : 4;
    uint8_t uc_mu_edca_timer; /* 单位 8TU */
} __OAL_DECLARE_PACKED mac_frame_he_mu_ac_parameter;

typedef struct
{
    mac_frame_he_mu_qos_info st_qos_info;
    mac_frame_he_mu_ac_parameter ast_mu_ac_parameter[WLAN_WME_AC_BUTT];
} __OAL_DECLARE_PACKED mac_frame_he_mu_edca_parameter_ie_stru;
/* he mu edca param */
typedef struct
{
    uint8_t bit_srp_disallowed : 1;
    uint8_t bit_non_srg_obss_pd_sr_disallowed : 1;
    uint8_t bit_non_srg_offset_present : 1;
    uint8_t bit_srg_information_present : 1;
    uint8_t bit_hesiga_sr_value15_allowed : 1;
    uint8_t bit_resvered : 3;
} __OAL_DECLARE_PACKED mac_frame_he_sr_control_stru;

#define MAC_HE_SRG_BSS_COLOR_BITMAP_LEN     8
#define MAC_HE_SRG_PARTIAL_BSSID_BITMAP_LEN 8
typedef struct
{
    mac_frame_he_sr_control_stru st_sr_control;
    uint8_t uc_non_srg_obss_pd_max_offset;
    uint8_t uc_srg_obss_pd_min_offset;
    uint8_t uc_srg_obss_pd_max_offset;
    uint8_t auc_srg_bss_color_bitmap[MAC_HE_SRG_BSS_COLOR_BITMAP_LEN];
    uint8_t auc_srg_partial_bssid_bitmap[MAC_HE_SRG_PARTIAL_BSSID_BITMAP_LEN];
} __OAL_DECLARE_PACKED mac_frame_he_spatial_reuse_parameter_set_ie_stru;

typedef struct
{
    uint8_t uc_color_switch_countdown;
    uint8_t bit_new_bss_color : 6,
              bit_reserved : 2;
} __OAL_DECLARE_PACKED mac_frame_bss_color_change_annoncement_ie_stru;

typedef union {
    uint32_t htc_value;
    union {
        struct
        {
            uint32_t bit_vht_flag : 1,
                       bit_he_flag : 1,
                       bit_om_id : 4,
                       bit_rx_nss : 3,
                       bit_channel_width : 2,
                       bit_ul_mu_disable : 1,
                       bit_tx_nsts : 3,
                       bit_er_su_disable : 1,
                       bit_dl_mu_mimo_resound_recommendation : 1,
                       bit_ul_mu_data_disable : 1,
                       bit_rsv : 14;
        } st_om_info;
        /* 后续数据帧中携带om 使用 */
        struct
        {
            uint32_t bit_vht_flag : 1,
                       bit_he_flag : 1,
                       bit_uph_id : 4,
                       bit_ul_power_headroom : 5,
                       bit_min_transmit_power_flag : 1,
                       bit_uph_rsv : 2,
                       bit_om_id : 4,
                       bit_rx_nss : 3,
                       bit_channel_width : 2,
                       bit_ul_mu_disable : 1,
                       bit_tx_nsts : 3,
                       bit_er_su_disable : 1,
                       bit_dl_mu_mimo_resound_recommendation : 1,
                       bit_ul_mu_data_disable : 1,
                       bit_rsv : 2;
        } st_uph_om_info;

    } un_a_control_info;
} __OAL_DECLARE_PACKED mac_htc_a_control_field_union;
#endif /* end _PRE_WLAN_FEATURE_11AX */

struct mac_11ntxbf_info {
    uint8_t bit_11ntxbf : 1, /* 11n txbf  能力 */
              bit_reserve : 7;
    uint8_t auc_reserve[NUM_3_BYTES];
};
typedef struct mac_11ntxbf_info mac_11ntxbf_info_stru;

struct mac_11ntxbf_vendor_ie {
    uint8_t uc_id;  /* element ID */
    uint8_t uc_len; /* length in bytes */
    uint8_t auc_oui[NUM_3_BYTES];
    uint8_t uc_ouitype;
    mac_11ntxbf_info_stru st_11ntxbf;
};
typedef struct mac_11ntxbf_vendor_ie mac_11ntxbf_vendor_ie_stru;

#ifdef _PRE_WLAN_FEATURE_PRIV_CLOSED_LOOP_TPC
/* 私有对通功率调整数据结构 */
struct mac_hisi_adjust_pow_ie {
    uint8_t uc_id;  /* element ID */
    uint8_t uc_len; /* length in bytes */
    uint8_t auc_oui[NUM_3_BYTES];
    uint8_t uc_ouitype;
    uint8_t uc_adjust_pow;
};
typedef struct mac_hisi_adjust_pow_ie mac_hisi_adjust_pow_ie_stru;
#endif

/* MAC_HISI_CAP_IE信息元素 能力字段结构体 */
struct mac_hisi_cap_vendor_ie {
    uint32_t bit_11ax_support      : 1,  /* 1:是否支持11AX bit0 */
             bit_dcm_support       : 1,  /* 1:是否支持dcm bit1 */
             bit_p2p_csa_support   : 1,  /* 1:是否支持p2p_csa bit2 */
             bit_p2p_scenes        : 1,  /* 1:是否支持p2p_scenes bit3 */
             bit_1024qam_cap       : 1,  /* 1:是否支持1024QAM bit4 */
             bit_4096qam_cap       : 1,  /* 1:是否支持4096QAM bit5 */
             bit_sgi_400ns_cap     : 1,  /* 1:是否支持SGI_400NS bit6 */
             bit_vht_3nss_80m_mcs6 : 1,  /* 1:是否支持VHT_3NSS_80M_MCS6 bit7 */
             bit_resv              : 24; /* 24:预留扩展字段 bit8~bit31 */
} __OAL_DECLARE_PACKED;
typedef struct mac_hisi_cap_vendor_ie mac_hisi_cap_vendor_ie_stru;

/* 厂家自定义IE 数据结构，摘自linux 内核 */
struct mac_ieee80211_vendor_ie {
    uint8_t uc_element_id;
    uint8_t uc_len;
    uint8_t auc_oui[MAC_OUI_LEN];
    uint8_t uc_oui_type;
} __OAL_DECLARE_PACKED;
typedef struct mac_ieee80211_vendor_ie mac_ieee80211_vendor_ie_stru;

/* MAC_HISI_CAP_IE 数据结构，私有IE收编，包括：1024QAM、4096QAM、SGI_400NS、VHT_3NSS_80M_MCS6 */
struct mac_hisi_cap_ie {
    mac_ieee80211_vendor_ie_stru ieee80211_vendor_ie;
    mac_hisi_cap_vendor_ie_stru vendor_ie_cap;
} __OAL_DECLARE_PACKED;
typedef struct mac_hisi_cap_ie mac_hisi_cap_ie_stru;

#ifdef _PRE_WLAN_FEATURE_MBO
struct mac_assoc_retry_delay_attr_mbo_ie {
    uint8_t uc_element_id;
    uint8_t uc_len;
    uint8_t auc_oui[NUM_3_BYTES];
    uint8_t uc_oui_type;
    uint8_t uc_attribute_id;
    uint8_t uc_attribute_len;
    uint32_t re_assoc_delay_time;
} __OAL_DECLARE_PACKED;
typedef struct mac_assoc_retry_delay_attr_mbo_ie mac_assoc_retry_delay_attr_mbo_ie_stru;
#endif

/* 建立BA会话时，BA参数域结构定义 */
struct mac_ba_parameterset {
#if (_PRE_BIG_CPU_ENDIAN == _PRE_CPU_ENDIAN) /* BIG_ENDIAN */
    uint16_t bit_buffersize : 10,          /* B6-15  buffer size */
               bit_tid : 4,                         /* B2-5   TID */
               bit_bapolicy : 1,                    /* B1   block ack policy */
               bit_amsdusupported : 1;              /* B0   amsdu supported */
#else
    uint16_t bit_amsdusupported : 1, /* B0   amsdu supported */
               bit_bapolicy : 1,              /* B1   block ack policy */
               bit_tid : 4,                   /* B2-5   TID */
               bit_buffersize : 10;           /* B6-15  buffer size */
#endif
} __OAL_DECLARE_PACKED;
typedef struct mac_ba_parameterset mac_ba_parameterset_stru;

/* BA会话过程中的序列号参数域定义 */
struct mac_ba_seqctrl {
#if (_PRE_BIG_CPU_ENDIAN == _PRE_CPU_ENDIAN) /* BIG_ENDIAN */
    uint16_t bit_startseqnum : 12,         /* B4-15  starting sequence number */
               bit_fragnum : 4;                     /* B0-3  fragment number */
#else
    uint16_t bit_fragnum : 4,        /* B0-3  fragment number */
               bit_startseqnum : 12;          /* B4-15  starting sequence number */
#endif
} __OAL_DECLARE_PACKED;
typedef struct mac_ba_seqctrl mac_ba_seqctrl_stru;

/* Quiet信息元素结构体 */
struct mac_quiet_ie {
    uint8_t quiet_count;
    uint8_t quiet_period;
    uint16_t quiet_duration;
    uint16_t quiet_offset;
} __OAL_DECLARE_PACKED;
typedef struct mac_quiet_ie mac_quiet_ie_stru;

/* erp 信息元素结构体 */
struct mac_erp_params {
    uint8_t bit_non_erp : 1,
              bit_use_protection : 1,
              bit_preamble_mode : 1,
              bit_resv : 5;
} __OAL_DECLARE_PACKED;
typedef struct mac_erp_params mac_erp_params_stru;

/* rsn信息元素 rsn能力字段结构体 */
struct mac_rsn_cap {
    uint16_t bit_pre_auth : 1,
               bit_no_pairwise : 1,
               bit_ptska_relay_counter : 2,
               bit_gtska_relay_counter : 2,
               bit_mfpr : 1,
               bit_mfpc : 1,
               bit_rsv0 : 1,
               bit_peer_key : 1,
               bit_spp_amsdu_capable : 1,
               bit_spp_amsdu_required : 1,
               bit_pbac : 1,
               bit_ext_key_id : 1,
               bit_rsv1 : 2;
} __OAL_DECLARE_PACKED;
typedef struct mac_rsn_cap mac_rsn_cap_stru;

/* obss扫描ie obss扫描参数结构体 */
struct mac_obss_scan_params {
    uint16_t us_passive_dwell;
    uint16_t us_active_dwell;
    uint16_t us_scan_interval;
    uint16_t us_passive_total_per_chan;
    uint16_t us_active_total_per_chan;
    uint16_t us_transition_delay_factor;
    uint16_t us_scan_activity_thresh;
} __OAL_DECLARE_PACKED;
typedef struct mac_obss_scan_params mac_obss_scan_params_stru;

/* 扩展能力信息元素结构体定义 */
struct mac_ext_cap_ie {
    uint8_t bit_2040_coexistence_mgmt : 1,
              bit_resv1 : 1,
              bit_ext_chan_switch : 1,
              bit_resv2 : 1,
              bit_psmp : 1,
              bit_resv3 : 1,
              bit_s_psmp : 1,
              bit_event : 1;
    uint8_t bit_resv4 : 4,
              bit_proxyarp : 1,
              bit_resv13 : 3; /* bit13~bit15 */
    uint8_t bit_resv5 : 3,
              bit_bss_transition : 1, /* bit19 */
              bit_resv14 : 2,         /* bit20~bit21 */
              bit_multi_bssid : 1,    /* bit22 Multi bssid */
              bit_resv15 : 1;         /* bit23 */
    uint8_t bit_resv6 : 7,
              bit_interworking : 1;
    uint8_t bit_resv7 : 5,
              bit_tdls_prhibited : 1,
              bit_tdls_channel_switch_prhibited : 1,
              bit_resv8 : 1;

    uint8_t bit_resv9 : 8;
    uint8_t bit_resv10 : 8;

    uint8_t bit_resv11 : 6,
              bit_operating_mode_notification : 1, /* 11ac Operating Mode Notification特性标志 */
              bit_resv12 : 1;
} __OAL_DECLARE_PACKED;
typedef struct mac_ext_cap_ie mac_ext_cap_ie_stru;

#ifdef _PRE_WLAN_FEATURE_FTM
struct mac_ext_cap_ftm_ie {
    uint8_t bit_2040_coexistence_mgmt : 1,
              bit_resv1 : 1,
              bit_ext_chan_switch : 1,
              bit_resv2 : 1,
              bit_psmp : 1,
              bit_resv3 : 1,
              bit_s_psmp : 1,
              bit_event : 1;
    uint8_t bit_resv4 : 4,
              bit_proxyarp : 1,
              bit_resv13 : 3; /* bit13~bit15 */
    uint8_t bit_resv5 : 3,
              bit_bss_transition : 1, /* bit19 */
              bit_resv14 : 4;         /* bit20~bit23 */
    uint8_t bit_resv6 : 7,
              bit_interworking : 1;
    uint8_t bit_resv7 : 5,
              bit_tdls_prhibited : 1,
              bit_tdls_channel_switch_prhibited : 1,
              bit_resv8 : 1;

    uint8_t bit_resv9 : 8;
    uint8_t bit_resv10 : 8;

    uint8_t bit_resv11 : 6,
              bit_operating_mode_notification : 1, /* 11ac Operating Mode Notification特性标志 */
              bit_resv12 : 1;

    uint8_t bit_resv15 : 6,
              bit_ftm_responder : 1, /* bit70 Fine Timing Measurement Responder */
              bit_ftm_initiator : 1; /* bit71 Fine Timing Measurement Initiator */
} __OAL_DECLARE_PACKED;
typedef struct mac_ext_cap_ftm_ie mac_ext_cap_ftm_ie_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_TWT
struct mac_ext_cap_twt_ie {
    uint8_t bit_2040_coexistence_mgmt : 1,
              bit_resv1 : 1,
              bit_ext_chan_switch : 1,
              bit_resv2 : 1,
              bit_psmp : 1,
              bit_resv3 : 1,
              bit_s_psmp : 1,
              bit_event : 1;
    uint8_t bit_resv4 : 4,
              bit_proxyarp : 1,
              bit_resv13 : 3; /* bit13~bit15 */
    uint8_t bit_resv5 : 3,
              bit_bss_transition : 1, /* bit19 */
              bit_resv14 : 4;         /* bit20~bit23 */
    uint8_t bit_resv6 : 7,
              bit_interworking : 1;
    uint8_t bit_resv7 : 5,
              bit_tdls_prhibited : 1,
              bit_tdls_channel_switch_prhibited : 1,
              bit_resv8 : 1;

    uint8_t bit_resv9 : 8;
    uint8_t bit_resv10 : 8;

    uint8_t bit_resv11 : 6,
              bit_operating_mode_notification : 1, /* 11ac Operating Mode Notification特性标志 */
              bit_resv12 : 1;

    uint8_t bit_resv15 : 6,
              bit_ftm_responder : 1, /* bit70 Fine Timing Measurement Responder */
              bit_ftm_initiator : 1; /* bit71 Fine Timing Measurement Initiator */

    uint8_t bit_resv16 : 5,
              bit_twt_requester_support : 1, /* 11ax TWT requester 特性标志 */
              bit_resv17 : 2;
} __OAL_DECLARE_PACKED;
typedef struct mac_ext_cap_twt_ie mac_ext_cap_twt_ie_stru;

struct mac_twt_request_type {
    uint16_t bit_request : 1;
    uint16_t bit_setup_command : 3;
    uint16_t bit_trigger : 1;
    uint16_t bit_implicit : 1;
    uint16_t bit_flow_type : 1;
    uint16_t bit_flow_id : 3;
    uint16_t bit_exponent : 5;
    uint16_t bit_protection : 1;
} __OAL_DECLARE_PACKED;
typedef struct mac_twt_request_type mac_twt_request_type_stru;
struct mac_btwt_request_type {
    uint16_t request : 1;
    uint16_t setup_command : 3;
    uint16_t trigger : 1;
    uint16_t implicit : 1;
    uint16_t flow_type : 1;
    uint16_t flow_id : 3;
    uint16_t exponent : 5;
    uint16_t protection : 1;
} __OAL_DECLARE_PACKED;
typedef struct mac_btwt_request_type mac_btwt_request_type_stru;

struct mac_twt_control_field {
    uint8_t bit_ndp_paging_indicator : 1;
    uint8_t bit_responder_pm_mode : 1;
    uint8_t bit_negotiation : 2;
    uint8_t bit_twt_information_frame_disabled : 1;
    uint8_t bit_wake_duration_unit : 1;
    uint8_t reserve: 2;
} __OAL_DECLARE_PACKED;
typedef struct mac_twt_control_field mac_twt_control_field_stru;

struct mac_twt_element_ie_individual {
    uint8_t category;
    uint8_t action;
    uint8_t dialog_token;
    uint8_t element_id;
    uint8_t len;
    mac_twt_control_field_stru st_control;
    mac_twt_request_type_stru st_request_type;
    uint64_t twt;
    uint8_t min_duration;
    uint16_t mantissa;  // 可能对不齐
    uint8_t channel;
} __OAL_DECLARE_PACKED;
typedef struct mac_twt_element_ie_individual mac_itwt_ie_stru;

struct mac_btwt_info {
    uint8_t bit_resv : 3;
    uint8_t bit_btwt_id : 5;
    uint8_t btwt_persistence;
} __OAL_DECLARE_PACKED;
typedef struct mac_btwt_info mac_btwt_info_stru;

struct mac_twt_element_ie_broadcast {
    uint8_t element_id;
    uint8_t ie_len;
    mac_twt_control_field_stru control; /* 1字节 */
    mac_btwt_request_type_stru request_type; /* 2字节 */
    uint16_t twt;
    uint8_t min_duration;
    uint16_t mantissa;
    mac_btwt_info_stru btwt_info;
} __OAL_DECLARE_PACKED;
typedef struct mac_twt_element_ie_broadcast mac_btwt_ie_stru;

struct mac_btwt_action {
    uint8_t category; /* ACTION的类别 */
    uint8_t action;   /* 不同ACTION类别下的分类 */
    uint8_t dialog_token;
    mac_btwt_ie_stru btwt_ie;
} __OAL_DECLARE_PACKED;
typedef struct mac_btwt_action mac_btwt_action_stru;

struct mac_twt_teardown {
    uint8_t uc_category;
    uint8_t uc_action;
    uint8_t bit_twt_flow_id : 5,
              bit_nego_type : 2,
              bit_resv2 : 1;
} __OAL_DECLARE_PACKED;
typedef struct mac_twt_teardown mac_twt_teardown_stru;

#ifdef _PRE_WLAN_FEATURE_TWT
struct mac_twt_information_field {
    uint8_t bit_twt_flow_id : 3;
    uint8_t bit_response_requested : 1;
    uint8_t bit_next_twt_request : 1;
    uint8_t bit_next_twt_subfield_size : 2;
    uint8_t bit_all_twt : 1;
} __OAL_DECLARE_PACKED;
typedef struct mac_twt_information_field mac_twt_information_field_stru;

struct mac_twt_information_frame {
    uint8_t uc_category;
    uint8_t uc_action;
    mac_twt_information_field_stru st_twt_information_filed;
} __OAL_DECLARE_PACKED;
typedef struct mac_twt_information_frame mac_twt_information_frame_stru;
#endif
#endif

/* qos info字段结构体定义 */
struct mac_qos_info {
    uint8_t bit_params_count : 4,
              bit_resv : 3,
              bit_uapsd : 1;
} __OAL_DECLARE_PACKED;
typedef struct mac_qos_info mac_qos_info_stru;

/* wmm信息元素 ac参数结构体 */
typedef struct {
    uint8_t bit_aifsn : 4,
              bit_acm : 1,
              bit_aci : 2,
              bit_resv : 1;
    uint8_t bit_ecwmin : 4,
              bit_ecwmax : 4;
    uint16_t us_txop;
} mac_wmm_ac_params_stru;

/* BSS load信息元素结构体 */
/*lint -e958*/                 /* 屏蔽字节对齐错误 */
struct mac_bss_load {
    uint16_t us_sta_count;       /* 关联的sta个数 */
    uint8_t uc_chan_utilization; /* 信道利用率 */
    uint16_t us_aac;
}__OAL_DECLARE_PACKED;
typedef struct mac_bss_load mac_bss_load_stru;
/*lint +e958*/
/* country信息元素 管制域字段 */
struct mac_country_reg_field {
    uint8_t uc_first_channel; /* 第一个信道号 */
    uint8_t uc_channel_num;   /* 信道个数 */
    uint16_t us_max_tx_pwr;   /* 最大传输功率，dBm */
} __OAL_DECLARE_PACKED;
typedef struct mac_country_reg_field mac_country_reg_field_stru;

/* ht capabilities信息元素支持的ampdu parameters字段结构体定义 */
struct mac_ampdu_params {
    uint8_t bit_max_ampdu_len_exponent : 2,
              bit_min_mpdu_start_spacing : 3,
              bit_resv : 3;
} __OAL_DECLARE_PACKED;
typedef struct mac_ampdu_params mac_ampdu_params_stru;

/* ht cap信息元素 支持的mcs集字段 结构体定义 */
struct mac_sup_mcs_set {
    uint8_t auc_rx_mcs[NUM_10_BYTES];
    uint16_t bit_rx_highest_rate : 10,
               bit_resv1 : 6;
    uint32_t bit_tx_mcs_set_def : 1,
               bit_tx_rx_not_equal : 1,
               bit_tx_max_stream : 2,
               bit_tx_unequal_modu : 1,
               bit_resv2 : 27;
} __OAL_DECLARE_PACKED;
typedef struct mac_sup_mcs_set mac_sup_mcs_set_stru;

/* vht信息元素，支持的mcs集字段,16版11ac协议新增bit30，涉及rom,原始结构体不再使用 */
typedef struct {
    uint32_t  bit_rx_mcs_rom_map            : 16,
                bit_rx_highest_rom_rate       : 13,
                bit_max_nsts                  : 3;
    uint32_t  bit_tx_mcs_rom_map            : 16,
                bit_tx_highest_rom_rate       : 13,
                bit_resv2                     : 3;
}mac_vht_sup_mcs_set_rom_stru;
/* vht信息元素，支持的mcs集字段 */
typedef struct {
    uint32_t  bit_rx_mcs_map                : 16,
                bit_rx_highest_rate           : 13,
                bit_max_nsts                  : 3;
    uint32_t  bit_tx_mcs_map                : 16,
                bit_tx_highest_rate           : 13,
                bit_vht_extend_nss_bw_capable : 1,
                bit_resv2                     : 2;
}mac_vht_sup_mcs_set_stru;

/* ht capabilities信息元素支持的extended cap.字段结构体定义 */
struct mac_ext_cap {
    uint16_t bit_pco : 1, /* */
               bit_pco_trans_time : 2,
               bit_resv1 : 5,
               bit_mcs_fdbk : 2,
               bit_htc_sup : 1,
               bit_rd_resp : 1,
               bit_resv2 : 4;
} __OAL_DECLARE_PACKED;
typedef struct mac_ext_cap mac_ext_cap_stru;

/* ht cap信息元素的Transmit Beamforming Capabilities字段结构体定义 */
typedef struct {
    uint32_t bit_implicit_txbf_rx : 1,
               bit_rx_stagg_sounding : 1,
               bit_tx_stagg_sounding : 1,
               bit_rx_ndp : 1,
               bit_tx_ndp : 1,
               bit_implicit_txbf : 1,
               bit_calibration : 2,
               bit_explicit_csi_txbf : 1,
               bit_explicit_noncompr_steering : 1,
               bit_explicit_compr_steering : 1,
               bit_explicit_txbf_csi_fdbk : 2,
               bit_explicit_noncompr_bf_fdbk : 2,
               bit_explicit_compr_bf_fdbk : 2,
               bit_minimal_grouping : 2,
               bit_csi_num_bf_antssup : 2,
               bit_noncompr_steering_num_bf_antssup : 2,
               bit_compr_steering_num_bf_antssup : 2,
               bit_csi_maxnum_rows_bf_sup : 2,
               bit_chan_estimation : 2,
               bit_resv : 3;
} mac_txbf_cap_stru;

/* ht cap信息元素的Asel(antenna selection) Capabilities字段结构体定义 */
struct mac_asel_cap {
    uint8_t bit_asel : 1,
              bit_explicit_sci_fdbk_tx_asel : 1,
              bit_antenna_indices_fdbk_tx_asel : 1,
              bit_explicit_csi_fdbk : 1,
              bit_antenna_indices_fdbk : 1,
              bit_rx_asel : 1,
              bit_trans_sounding_ppdu : 1,
              bit_resv : 1;
} __OAL_DECLARE_PACKED;
typedef struct mac_asel_cap mac_asel_cap_stru;

/* ht opern元素, ref 802.11-2012 8.4.2.59 */
struct mac_ht_opern {
    uint8_t uc_primary_channel;

    uint8_t bit_secondary_chan_offset : 2,
              bit_sta_chan_width : 1,
              bit_rifs_mode : 1,
              bit_resv1 : 4;
    uint8_t bit_ht_protection : 2,
              bit_nongf_sta_present : 1,
              bit_resv2 : 1,
              bit_obss_nonht_sta_present : 1,
              bit_resv3 : 3;
    uint8_t bit_resv4 : 8;
    uint8_t bit_resv5 : 6,
              bit_dual_beacon : 1,
              bit_dual_cts_protection : 1;
    uint8_t bit_secondary_beacon : 1,
              bit_lsig_txop_protection_full_support : 1,
              bit_pco_active : 1,
              bit_pco_phase : 1,
              bit_resv6 : 4;

    uint8_t auc_basic_mcs_set[MAC_HT_BASIC_MCS_SET_LEN];
} __OAL_DECLARE_PACKED;
typedef struct mac_ht_opern mac_ht_opern_stru;

/* ht opern元素, 802.11-2016 11ac */
struct mac_ht_opern_ac {
    uint8_t uc_primary_channel;

    uint8_t bit_secondary_chan_offset : 2,
              bit_sta_chan_width : 1,
              bit_rifs_mode : 1,
              bit_resv1 : 4;
    uint16_t bit_ht_protection : 2,
               bit_nongf_sta_present : 1,
               bit_resv2 : 1,
               bit_obss_nonht_sta_present : 1,
               bit_chan_center_freq_seg2 : 8,
               bit_resv3 : 3;
    uint8_t bit_resv4 : 6,
              bit_dual_beacon : 1,
              bit_dual_cts_protection : 1;
    uint8_t bit_secondary_beacon : 1,
              bit_lsig_txop_protection_full_support : 1,
              bit_pco_active : 1,
              bit_pco_phase : 1,
              bit_resv5 : 4;

    uint8_t auc_basic_mcs_set[MAC_HT_BASIC_MCS_SET_LEN];
} __OAL_DECLARE_PACKED;
typedef struct mac_ht_opern_ac mac_ht_opern_ac_stru;

/* vht opern结构体 */
/*lint -e958*/     /* 屏蔽字节对齐错误 */
struct mac_opmode_notify {
uint8_t bit_channel_width : 2, /* 当前最大允许带宽能力 */
bit_160or8080 : 1, /* 是否是160M */
              bit_noldpc : 1,
              bit_rx_nss : 3,      /* 当前最大允许空间流能力 */
              bit_rx_nss_type : 1; /* 是否为TXBF下的rx nss能力，1-是，0-不是 */
}
__OAL_DECLARE_PACKED;
typedef struct mac_opmode_notify mac_opmode_notify_stru;
/*lint +e958*/
/* vht opern结构体 */
/*lint -e958*/ /* 屏蔽字节对齐错误 */
struct mac_vht_opern {
uint8_t uc_channel_width;
uint8_t uc_channel_center_freq_seg0;
uint8_t uc_channel_center_freq_seg1;
uint16_t us_basic_mcs_set;
} __OAL_DECLARE_PACKED;
typedef struct mac_vht_opern mac_vht_opern_stru;
/*lint +e958*/
/*lint -e958*/ /* 屏蔽字节对齐错误 */
/* RRM ENABLED CAP信息元素结构体 */
typedef struct oal_rrm_enabled_cap_ie mac_rrm_enabled_cap_ie_stru;

/* Measurement Report Mode */
struct mac_mr_mode {
    uint8_t bit_parallel : 1,
              bit_enable : 1,
              bit_request : 1,
              bit_rpt : 1,
              bit_duration_mandatory : 1,
              bit_rsvd : 3;
} __OAL_DECLARE_PACKED;
typedef struct mac_mr_mode mac_mr_mode_stru;

/* Radio Measurement Request */
struct mac_action_rm_req {
    uint8_t uc_category;
    uint8_t uc_action_code;
    uint8_t uc_dialog_token;
    uint16_t us_num_rpt;
    uint8_t auc_req_ies[1];
} __OAL_DECLARE_PACKED;
typedef struct mac_action_rm_req mac_action_rm_req_stru;

/* Neighbor Report Request */
struct mac_action_neighbor_req {
    uint8_t uc_category;
    uint8_t uc_action_code;
    uint8_t uc_dialog_token;
    uint8_t auc_subelm[1];
} __OAL_DECLARE_PACKED;
typedef struct mac_action_neighbor_req mac_action_neighbor_req_stru;

/* Neighbor Report Response */
struct mac_action_neighbor_rsp {
    uint8_t uc_category;
    uint8_t uc_action_code;
    uint8_t uc_dialog_token;
    uint8_t neighbor_rpt_ies[1];
} __OAL_DECLARE_PACKED;
typedef struct mac_action_neighbor_rsp mac_action_neighbor_rsp_stru;

/* Radio Measurement Report */
struct mac_action_rm_rpt {
    uint8_t uc_category;
    uint8_t uc_action_code;
    uint8_t uc_dialog_token;
    uint8_t auc_rpt_ies[1];
} __OAL_DECLARE_PACKED;
typedef struct mac_action_rm_rpt mac_action_rm_rpt_stru;

/* Measurement Request Elements */
struct mac_meas_req_ie {
    uint8_t uc_eid; /* IEEE80211_ELEMID_MEASREQ */
    uint8_t uc_len;
    uint8_t uc_token;
    mac_mr_mode_stru st_reqmode;
    uint8_t uc_reqtype;
    uint8_t auc_meas_req[1]; /* varialbe len measurement requet */
} __OAL_DECLARE_PACKED;
typedef struct mac_meas_req_ie mac_meas_req_ie_stru;

struct mac_ftm_range_req_ie {
    uint16_t us_randomization_interval;
    uint8_t uc_minimum_ap_count;
    uint8_t auc_ftm_range_subelements[1];
} __OAL_DECLARE_PACKED;
typedef struct mac_ftm_range_req_ie mac_ftm_range_req_ie_stru;

/* Measurement Report Mode */
struct mac_meas_rpt_mode {
    uint8_t bit_late : 1,
              bit_incapable : 1,
              bit_refused : 1,
              bit_rsvd : 5;
} __OAL_DECLARE_PACKED;
typedef struct mac_meas_rpt_mode mac_meas_rpt_mode_stru;
/* Measurement Report Elements */
struct mac_meas_rpt_ie {
    uint8_t uc_eid; /* IEEE80211_ELEMID_MEASRPT */
    uint8_t uc_len;
    uint8_t uc_token;
    mac_meas_rpt_mode_stru st_rptmode;
    uint8_t uc_rpttype;
    uint8_t auc_meas_rpt[100]; /* varialbe len measurement report -- 1 ,数组长度100byte */
} __OAL_DECLARE_PACKED;
typedef struct mac_meas_rpt_ie mac_meas_rpt_ie_stru;

/* Neighbor Report Elements */
struct mac_bssid_info {
    uint32_t bit_ap_reachability : 2,
               bit_security : 1,
               bit_key_scope : 1,
               bit_spec_management : 1,
               bit_qos : 1,
               bit_apsd : 1,
               bit_radio_meas : 1,
               bit_delayed_ba : 1,
               bit_immediate_ba : 1,
               bit_rsv : 22;
} __OAL_DECLARE_PACKED;
typedef struct mac_bssid_info mac_bssid_info_stru;

struct mac_neighbor_rpt_ie {
    uint8_t uc_eid; /* IEEE80211_ELEMID_MEASRPT */
    uint8_t uc_len;
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    mac_bssid_info_stru st_bssid_info;
    uint8_t uc_optclass;
    uint8_t uc_channum;
    uint8_t uc_phy_type;
    uint8_t auc_subelm[1]; /* varialbe len sub element fileds */
} __OAL_DECLARE_PACKED;
typedef struct mac_neighbor_rpt_ie mac_neighbor_rpt_ie_stru;

struct mac_meas_sub_ie {
    uint8_t sub_eid; /* IEEE80211_ELEMID_MEASREQ */
    uint8_t len;
    uint8_t meas_sub_data[1]; /* varialbe len measurement requet */
} __OAL_DECLARE_PACKED;
typedef struct mac_meas_sub_ie mac_meas_sub_ie_stru;

/* Beacon Report SubElement: Reported Frame Body Fgrament ID */
struct mac_reported_frame_body_fragment_id {
    uint8_t sub_eid;
    uint8_t len;
    uint8_t bit_beacon_report_id; /* beacon report编号 */
    uint8_t bit_fragment_id_number : 7, /* beacon report分段编号 */
            bit_more_frame_body_fragments : 1; /* 当前beacon report后续是否还有更多分段，[1:有|0:没有] */
} __OAL_DECLARE_PACKED;
typedef struct mac_reported_frame_body_fragment_id mac_reported_frame_body_fragment_id_stru;

/* Beacon Report SubElement: Last Beacon Report Indication */
struct mac_last_beacon_report_indication {
    uint8_t sub_eid;
    uint8_t len;
    uint8_t last_beacon_report_indication; /* 是否是最后一个beacon report，[1:是|0:不是] */
} __OAL_DECLARE_PACKED;
typedef struct mac_last_beacon_report_indication mac_last_beacon_report_indication_stru;

/* Beacon report request */
struct mac_bcn_req {
    uint8_t uc_optclass;
    uint8_t uc_channum; /*  请求测量的信道号 */
    uint16_t us_random_ivl;
    uint16_t us_duration;
    rm_bcn_req_meas_mode_enum_uint8 en_mode;
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    uint8_t auc_subelm[]; /* varialbe len sub element fileds */
} __OAL_DECLARE_PACKED;
typedef struct mac_bcn_req mac_bcn_req_stru;

/* Beacon report */
struct mac_bcn_rpt {
    uint8_t uc_optclass;
    uint8_t uc_channum;
    uint32_t aul_act_meas_start_time[2]; /* 数组长为2个uint32_t */
    uint16_t us_duration;
    uint8_t bit_condensed_phy_type : 7,
              bit_rpt_frm_type : 1;
    uint8_t uc_rcpi;
    uint8_t uc_rsni;
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    uint8_t uc_antenna_id;
    uint32_t parent_tsf;
    uint8_t auc_subelm[]; /* varialbe len sub element fileds */
} __OAL_DECLARE_PACKED;
typedef struct mac_bcn_rpt mac_bcn_rpt_stru;

/* Channel Load request */
struct mac_chn_load_req {
    uint8_t uc_optclass;
    uint8_t uc_channum; /*  请求测量的信道号 */
    uint16_t us_random_ivl;
    uint16_t us_duration;
    uint8_t uc_chn_load;
    uint8_t auc_subelm[1]; /* varialbe len sub element fileds */
} __OAL_DECLARE_PACKED;
typedef struct mac_chn_load_req mac_chn_load_req_stru;

/* Channel Load Report */
struct mac_chn_load_rpt {
    uint8_t uc_optclass;
    uint8_t uc_channum;
    uint32_t aul_act_meas_start_time[2]; /* 数组长为2个uint32_t */
    uint16_t us_duration;
    uint8_t uc_channel_load;
} __OAL_DECLARE_PACKED;
typedef struct mac_chn_load_rpt mac_chn_load_rpt_stru;

/* Frame request */
struct mac_frm_req {
    uint8_t uc_optclass;
    uint8_t uc_channum; /*  请求测量的信道号 */
    uint16_t us_random_ivl;
    uint16_t us_duration;
    uint8_t uc_frm_req_type;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN];
    uint8_t auc_subelm[1]; /* varialbe len sub element fileds */
} __OAL_DECLARE_PACKED;
typedef struct mac_frm_req mac_frm_req_stru;

/* Frame Report */
struct mac_frm_cnt_rpt {
    uint8_t auc_tx_addr[WLAN_MAC_ADDR_LEN];
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    uint8_t uc_phy_type;
    uint8_t uc_avrg_rcpi;
    uint8_t uc_last_rsni;
    uint8_t uc_last_rcpi;
    uint8_t uc_ant_id;
    uint16_t us_frm_cnt;
} __OAL_DECLARE_PACKED;
typedef struct mac_frm_cnt_rpt mac_frm_cnt_rpt_stru;

/* Fine Timing Measurement Parameters */
struct mac_ftm_parameters_ie {
    uint8_t uc_eid; /* IEEE80211_ELEMID_MEASRPT */
    uint8_t uc_len;
    uint8_t bit_status_indication : 2,
              bit_value : 5,
              bit_reserved0 : 1;
    uint8_t bit_number_of_bursts_exponent : 4,
              bit_burst_duration : 4;
    uint8_t uc_min_delta_ftm;
    uint16_t us_partial_tsf_timer;
    uint8_t bit_partial_tsf_timer_no_preference : 1,
              bit_asap_capable : 1,
              bit_asap : 1,
              bit_ftms_per_burst : 5;
    uint8_t bit_reserved : 2,
              bit_format_and_bandwidth : 6;
    uint16_t us_burst_period;
} __OAL_DECLARE_PACKED;
typedef struct mac_ftm_parameters_ie mac_ftm_parameters_ie_stru;

struct mac_frm_rpt {
    uint8_t uc_optclass;
    uint8_t uc_channum;
    uint8_t auc_act_meas_start_time[MAC_RADIO_MEAS_START_TIME_LEN];
    uint16_t us_duration;
    uint8_t auc_subelm[1]; /* varialbe len sub element fileds */
} __OAL_DECLARE_PACKED;
typedef struct mac_frm_rpt mac_frm_rpt_stru;

/* Statistic Report basic */
struct mac_stats_rpt_basic {
    uint16_t us_duration;
    uint8_t uc_group_id;
    uint8_t auc_group[1];
} __OAL_DECLARE_PACKED;
typedef struct mac_stats_rpt_basic mac_stats_rpt_basic_stru;

/* Statistic Report Group0, dot11Counters Group */
struct mac_stats_cnt_rpt {
    uint32_t tx_frag_cnt;
    uint32_t multi_tx_cnt;
    uint32_t fail_cnt;
    uint32_t rx_frag_cnt;
    uint32_t multi_rx_cnt;
    uint32_t fcs_err_cnt;
    uint32_t tx_cnt;
} __OAL_DECLARE_PACKED;
typedef struct mac_stats_cnt_rpt mac_stats_cnt_rpt_stru;

/* Statistic Report Group1, dot11MACStatistics Group */
struct mac_stats_mac_rpt {
    uint32_t retry_cnt;
    uint32_t multi_retry_cnt;
    uint32_t dup_cnt;
    uint32_t rts_succ_cnt;
    uint32_t trs_fail_cnt;
    uint32_t ack_fail_cnt;
} __OAL_DECLARE_PACKED;
typedef struct mac_stats_mac_rpt mac_stats_mac_rpt_stru;

/* Statistic Report Group2~9 */
struct mac_stats_up_cnt_rpt {
    uint32_t tx_frag_cnt;
    uint32_t fail_cnt;
    uint32_t retry_cnt;
    uint32_t multi_retry_cnt;
    uint32_t dup_cnt;
    uint32_t rts_succ_cnt;
    uint32_t trs_fail_cnt;
    uint32_t ack_fail_cnt;
    uint32_t rx_frag_cnt;
    uint32_t tx_cnt;
    uint32_t discard_cnt;
    uint32_t rx_cnt;
    uint32_t rx_retry_cnt;
} __OAL_DECLARE_PACKED;
typedef struct mac_stats_up_cnt_rpt mac_stats_up_cnt_rpt_stru;

/* Statistic Report Group11 */
struct mac_stats_access_delay_rpt {
    uint32_t avrg_access_delay;
    uint32_t aul_avrg_access_delay[4]; /* 数组长为4个uint32_t */
    uint32_t sta_cnt;
    uint32_t channel_utilz;
} __OAL_DECLARE_PACKED;
typedef struct mac_stats_access_delay_rpt mac_stats_access_delay_rpt_stru;

/* TSC Report */
struct mac_tsc_rpt {
    uint8_t auc_act_meas_start_time[MAC_RADIO_MEAS_START_TIME_LEN];
    uint16_t us_duration;
    uint8_t auc_peer_sta_addr[WLAN_MAC_ADDR_LEN];
    uint8_t uc_tid;
    uint8_t uc_rpt_reason;

    uint32_t tx_frag_cnt;
    uint32_t fail_cnt;
    uint32_t retry_cnt;
    uint32_t multi_retry_cnt;
} __OAL_DECLARE_PACKED;
typedef struct mac_tsc_rpt mac_tsc_rpt_stru;

/* AP Channel Report */
struct mac_ap_chn_rpt {
    uint8_t uc_eid;
    uint8_t uc_length;
    uint8_t uc_oper_class;
    uint8_t auc_chan[1];
} __OAL_DECLARE_PACKED;
typedef struct mac_ap_chn_rpt mac_ap_chn_rpt_stru;

struct mac_pwr_constraint_frm {
    uint8_t uc_eid;
    uint8_t uc_len;
    uint8_t uc_local_pwr_constraint;
} __OAL_DECLARE_PACKED;
typedef struct mac_pwr_constraint_frm mac_pwr_constraint_frm_stru;
/*lint +e958*/
typedef struct {
    uint8_t en_app_ie_type;
    uint8_t ie_len;
    uint8_t huawei_oui[NUM_3_BYTES];
    uint8_t feature_id;
    uint8_t cap_tlv_type;  // 0
    uint8_t cap_len;       // 2
    uint8_t support_adaptive_11r : 1;
    uint8_t support_conflict_stat : 1;
    uint8_t support_5g : 1;
    uint8_t support_dual_5g : 1;
    uint8_t support_11ad : 1;
    uint8_t support_24g_channel_mode : 2;
    uint8_t support_neighbor_report : 1;
    uint8_t akm_suite_val : 4;
    uint8_t mdid_flag : 1;
    uint8_t ft_over_ds : 1;
    uint8_t support_res_req_proto : 1;
    uint8_t support_nallow_band : 1;
    uint8_t dc_roaming_type;  // 7
    uint8_t dc_roaming_len;   // 8
    uint8_t other_radio_mac[NUM_6_BYTES];
    uint8_t other_radio_channel;
    uint8_t other_radio_bandwidth;
    uint8_t other_radio_erip;
} oal_sta_ap_cowork_ie;

typedef struct {
    uint8_t real_akm_val;
    uint32_t akm_suite;
} oal_sta_ap_cowork_akm_para_stru;

typedef struct {
    uint8_t tlv_type;  // cap_tlv_type
    uint8_t tlv_len;   // cap_len
    uint8_t support_adaptive_11r : 1;
    uint8_t support_conflict_stat : 1;
    uint8_t support_5g : 1;
    uint8_t support_dual_5g : 1;
    uint8_t support_11ad : 1;
    uint8_t support_24g_channel_mode : 2;
    uint8_t support_neighbor_report : 1;
    uint8_t akm_suite_val : 4;
    uint8_t mdid_flag : 1;
    uint8_t ft_over_ds : 1;
    uint8_t support_res_req_proto : 1;
    uint8_t resv : 1;
    uint16_t mdid;
} oal_cowork_ie_capa_bitmap_stru;

typedef struct {
    uint8_t tlv_type;  // dc_roaming_type
    uint8_t tlv_len;   // dc_roaming_len
    uint8_t other_radio_mac[NUM_6_BYTES];
    uint8_t other_radio_channel;
    uint8_t other_radio_bandwidth;
    uint8_t other_radio_erip;
} oal_cowork_ie_dc_roaming_stru;

typedef struct {
    uint8_t en_app_ie_type;
    uint8_t ie_len;
    uint8_t huawei_oui[NUM_3_BYTES];
    uint8_t feature_id;
    oal_cowork_ie_capa_bitmap_stru capa_bitmap;
    oal_cowork_ie_dc_roaming_stru dc_roaming; /* 暂时不携带此部分信息，ie帧长度需要减去11 */
} oal_sta_ap_cowork_ie_beacon;

typedef struct {
    uint8_t tag_num;
    uint8_t tag_len;
    uint16_t mdid;
    uint8_t ft_over_ds : 1;
    uint8_t support_res_req_proto : 1;
    uint8_t resv : 6;
} oal_cowork_md_ie;

typedef struct {
    uint8_t en_app_ie_type;
    uint8_t ie_len;
    uint8_t huawei_oui[3]; // 3字节oui
    uint8_t feature_id;
    uint8_t cap_tlv_type;  // 0
    uint8_t cap_len;
    uint8_t tx_pwr_detail_2g[2]; /* EIRP、52-ToneEIRP */
    uint8_t tx_pwr_detail_5g[4][2]; /* EIRP、52-ToneEIRP，分4个band */
} oal_sta_max_power_ie;

typedef struct {
    uint8_t en_app_ie_type;
    uint8_t ie_len;
    uint8_t huawei_oui[3]; // 3字节oui
    uint8_t feature_id;
    uint8_t cap_tlv_type;  // 0
    uint8_t cap_len;
    uint8_t tb_frame_gain_gain_2g;
    uint8_t tb_frame_gain_gain_5g[4]; /* 分4个band */
} oal_ap_tb_frame_gain_ie;

/* 用#pragma pack(1)/#pragma pack()方式达到一字节对齐 */
#pragma pack()

#if defined(_PRE_WLAN_FEATURE_WMMAC) || (defined(_PRE_WLAN_FEATURE_11K))
struct mac_ts_info {
    uint16_t bit_traffic_type : 1,
               bit_tsid : 4,
               bit_direction : 2,
               bit_acc_policy : 2,
               bit_aggr : 1,
               bit_apsd : 1,
               bit_user_prio : 3,
               bit_ack_policy : 2;
    uint8_t bit_schedule : 1,
              bit_rsvd : 7;
} __OAL_DECLARE_PACKED;
typedef struct mac_ts_info mac_ts_info_stru;

struct mac_wmm_tspec {
    mac_ts_info_stru ts_info;
    uint16_t us_norminal_msdu_size;
    uint16_t us_max_msdu_size;
    uint32_t min_srv_interval;
    uint32_t max_srv_interval;
    uint32_t inactivity_interval;
    uint32_t suspension_interval;
    uint32_t srv_start_time;
    uint32_t min_data_rate;
    uint32_t mean_data_rate;
    uint32_t peak_data_rate;
    uint32_t max_burst_size;
    uint32_t delay_bound;
    uint32_t min_phy_rate;
    uint16_t us_surplus_bw;
    uint16_t us_medium_time;
} __OAL_DECLARE_PACKED;
typedef struct mac_wmm_tspec mac_wmm_tspec_stru;
#endif

/* ACTION帧的参数格式，注:不同的action帧下对应的参数不同 */
typedef struct {
    uint8_t uc_category; /* ACTION的类别 */
    uint8_t uc_action;   /* 不同ACTION类别下的分类 */
    uint8_t uc_resv[NUM_2_BYTES];
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
    uint32_t arg4;
    uint8_t *puc_arg5;
} mac_action_mgmt_args_stru;

/* 私有管理帧通用的设置参数信息的结构体 */
typedef struct {
    uint8_t uc_type;
    uint8_t uc_arg1;      /* 对应的tid序号 */
    uint8_t uc_arg2;      /* 接收端可接收的最大的mpdu的个数(针对AMPDU_START命令) */
    uint8_t uc_arg3;      /* 确认策略 */
    uint16_t us_user_idx; /* 对应的用户 */
    uint8_t auc_resv[NUM_2_BYTES];
} mac_priv_req_args_stru;

/* SMPS节能控制action帧体 */
typedef struct {
    uint8_t category;
    uint8_t action;
    uint8_t sm_ctl;
    uint8_t arg;
} mac_smps_action_mgt_stru;

/* operating mode notify控制action帧体 */
typedef struct {
    uint8_t category;
    uint8_t action;
    uint8_t opmode_ctl;
    uint8_t arg;
} mac_opmode_notify_action_mgt_stru;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV)
/* channel结构体, 1103/1105 device使用 */
typedef struct {
    oal_uint8 uc_chan_number;                       /* 主20MHz信道号 */
    wlan_channel_band_enum_uint8 en_band;           /* 频段 */
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽模式 */
    oal_uint8 uc_chan_idx;                          /* 信道索引号 */
} mac_channel_stru;
#else
/* channel结构体, host 和1106 device 以及后续产品使用 */
typedef struct {
    uint8_t uc_chan_number;                       /* 主20MHz信道号 */
    uint8_t en_band : 4,
            ext6g_band : 1, /* 指示是否为6G频段标志 */
            resv : 3;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽模式 */
    uint8_t uc_chan_idx;                          /* 信道索引号 */
} mac_channel_stru;
#endif

uint16_t mac_get_rsn_capability(const uint8_t *puc_rsn_ie);
#endif /* end of mac_frame.h */
