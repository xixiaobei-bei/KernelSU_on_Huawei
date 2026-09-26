

#ifndef __WLAN_MIB_H__
#define __WLAN_MIB_H__

#include "oal_types.h"
#include "wlan_types.h"
#include "hd_event.h"

#define WLAN_MIB_TOKEN_STRING_MAX_LENGTH 32 /* 支持与网管兼容的token的字符串最长长度，此定义后续可能与版本相关 */

#define MAC_PAIRWISE_CIPHER_SUITES_NUM 2 /* 表征WPA2有CCMP和TKIP两种加密套件，WPA有WEP和WEP104两种加密套件 */
#define MAC_AUTHENTICATION_SUITE_NUM   2 /* 表示RSN 认证方法数 */

#define WAL_HIPRIV_RSSI_DEFAULT_THRESHOLD (-95) /* 默认RSSI门限 -95dbm */

/* RowStatus ::= TEXTUAL-CONVENTION                                         */
/* The status column has six  values:`active', `notInService',`notReady',   */
/* `createAndGo', `createAndWait', `destroy'  as described in   rfc2579     */
typedef enum {
    WLAN_MIB_ROW_STATUS_ACTIVE = 1,
    WLAN_MIB_ROW_STATUS_NOT_INSERVICE,
    WLAN_MIB_ROW_STATUS_NOT_READY,
    WLAN_MIB_ROW_STATUS_CREATE_AND_GO,
    WLAN_MIB_ROW_STATUS_CREATE_AND_WAIT,
    WLAN_MIB_ROW_STATUS_DEATROY,

    WLAN_MIB_ROW_STATUS_BUTT
} wlan_mib_row_status_enum;

typedef enum {
    WLAN_MIB_PWR_MGMT_MODE_ACTIVE = 1,
    WLAN_MIB_PWR_MGMT_MODE_PWRSAVE = 2,

    WLAN_MIB_PWR_MGMT_MODE_BUTT
} wlan_mib_pwr_mgmt_mode_enum;

/* 根据802.11-2016 9.4.1.1 Authentication Algorithm Number field 重新定义认证算法枚举 */
typedef enum {
    WLAN_MIB_AUTH_ALG_OPEN_SYS = 0,
    WLAN_MIB_AUTH_ALG_SHARE_KEY = 1,
    WLAN_MIB_AUTH_ALG_FT = 2,
    WLAN_MIB_AUTH_ALG_SAE = 3,
    WLAN_MIB_AUTH_ALG_TBPEKE = 0x60,

    WLAN_MIB_AUTH_ALG_BUTT
} wlan_mib_auth_alg_enum;

typedef enum {
    WLAN_MIB_DESIRED_BSSTYPE_INFRA = 1,
    WLAN_MIB_DESIRED_BSSTYPE_INDEPENDENT = 2,
    WLAN_MIB_DESIRED_BSSTYPE_ANY = 3,

    WLAN_MIB_DESIRED_BSSTYPE_BUTT
} wlan_mib_desired_bsstype_enum;

typedef enum {
    WLAN_MIB_RSNACFG_GRPREKEY_DISABLED = 1,
    WLAN_MIB_RSNACFG_GRPREKEY_TIMEBASED = 2,
    WLAN_MIB_RSNACFG_GRPREKEY_PACKETBASED = 3,
    WLAN_MIB_RSNACFG_GRPREKEY_TIMEPACKETBASED = 4,

    WLAN_MIB_RSNACFG_GRPREKEY_BUTT
} wlan_mib_rsna_cfg_grp_rekey_enum;

typedef enum {
    WLAN_MIB_RMRQST_TYPE_CH_LOAD = 3,
    WLAN_MIB_RMRQST_TYPE_NOISE_HISTOGRAM = 4,
    WLAN_MIB_RMRQST_TYPE_BEACON = 5,
    WLAN_MIB_RMRQST_TYPE_FRAME = 6,
    WLAN_MIB_RMRQST_TYPE_STA_STATISTICS = 7,
    WLAN_MIB_RMRQST_TYPE_LCI = 8,
    WLAN_MIB_RMRQST_TYPE_TRANS_STREAM = 9,
    WLAN_MIB_RMRQST_TYPE_PAUSE = 255,

    WLAN_MIB_RMRQST_TYPE_BUTT
} wlan_mib_rmrqst_type_enum;

/* dot11RMRqstBeaconRqstMode INTEGER{  passive(0), active(1),beaconTable(2) */
typedef enum {
    WLAN_MIB_RMRQST_BEACONRQST_MODE_PASSIVE = 0,
    WLAN_MIB_RMRQST_BEACONRQST_MODE_ACTIVE = 1,
    WLAN_MIB_RMRQST_BEACON_MODE_BEACON_TABLE = 2,

    WLAN_MIB_RMRQST_BEACONRQST_MODE_BUTT
} wlan_mib_rmrqst_beaconrqst_mode_enum;

/* dot11RMRqstBeaconRqstDetail INTEGER {noBody(0),fixedFieldsAndRequestedElements(1),allBody(2) } */
typedef enum {
    WLAN_MIB_RMRQST_BEACONRQST_DETAIL_NOBODY = 0,
    WLAN_MIB_RMRQST_BEACONRQST_DETAIL_FIXED_FLDANDELMT = 1,
    WLAN_MIB_RMRQST_BEACONRQST_DETAIL_ALLBODY = 2,

    WLAN_MIB_RMRQST_BEACONRQST_DETAIL_BUTT
} wlan_mib_rmrqst_beaconrqst_detail_enum;

/* dot11RMRqstFrameRqstType INTEGER { frameCountRep(1) } */
typedef enum {
    WLAN_MIB_RMRQST_FRAMERQST_TYPE_FRAME_COUNTREP = 1,

    WLAN_MIB_RMRQST_FRAMERQST_TYPE_BUTT
} wlan_mib_rmrqst_famerqst_type_enum;

typedef enum {
    WLAN_MIB_RMRQST_BEACONRPT_CDT_AFTER_EVERY_MEAS = 0,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_ABOVE_ABS_THRESHOLD = 1,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_BELOW_ABS_THRESHOLD = 2,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_ABOVE_ABS_THRESHOLD = 3,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_BELOW_ABS_THRESHOLD = 4,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_ABOVE_OFFSET_THRESHOLD = 5,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_BELOW_OFFSET_THRESHOLD = 6,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_ABOVE_OFFSET_THRESHOLD = 7,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_BELOW_OFFSET_THRESHOLD = 8,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_IN_BOUND = 9,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_IN_BOUND = 10,

    WLAN_MIB_RMRQST_BEACONRPT_CDT_BUTT
} wlan_mib_rmrqst_beaconrpt_cdt_enum;

typedef enum {
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_TABLE = 0,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_MAC_STATS = 1,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP0 = 2,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP1 = 3,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP2 = 4,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP3 = 5,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP4 = 6,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP5 = 7,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP6 = 8,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP7 = 9,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_BSS_AVERG_ACCESS_DELAY = 10,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR31 = 11,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR32 = 12,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR33 = 13,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR34 = 14,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR35 = 15,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_RSNA_STATS_TABLE = 16,

    WLAN_MIB_RMRQST_STASTATRQST_GRPID_BUTT
} wlan_mib_rmrqst_stastatrqst_grpid_enum;

/* dot11RMRqstLCIRqstSubject OBJECT-TYPE  */
/* SYNTAX INTEGER { local(0), remote(1) } */
typedef enum {
    WLAN_MIB_RMRQST_LCIRQST_SUBJECT_LOCAL = 0,
    WLAN_MIB_RMRQST_LCIRQST_SUBJECT_REMOTE = 1,

    WLAN_MIB_RMRQST_LCIRQST_SUBJECT_BUTT
} wlan_mib_rmrqst_lcirpt_subject_enum;

/* dot11RMRqstLCIAzimuthType OBJECT-TYPE                  */
/* SYNTAX INTEGER { frontSurfaceofSta(0), radioBeam(1) }  */
typedef enum {
    WLAN_MIB_RMRQST_LCIAZIMUTH_TYPE_FRONT_SURFACE_STA = 0,
    WLAN_MIB_RMRQST_LCIAZIMUTH_TYPE_RADIO_BEAM = 1,

    WLAN_MIB_RMRQST_LCIAZIMUTH_TYPE_BUTT
} wlan_mib_rmrqst_lciazimuth_type_enum;

/* dot11RMRqstChannelLoadReportingCondition OBJECT-TYPE   */
/* SYNTAX INTEGER {                                       */
/* afterEveryMeasurement(0),                             */
/* chanLoadAboveReference(1),                            */
/* chanLoadBelowReference(2) }                           */
typedef enum {
    WLAN_MIB_RMRQST_CH_LOADRPT_CDT_AFTER_EVERY_MEAS = 0,
    WLAN_MIB_RMRQST_CH_LOADRPT_CDT_CH_LOAD_ABOVE_REF = 1,
    WLAN_MIB_RMRQST_CH_LOADRPT_CDT_CH_LOAD_BELOW_REF = 2,

    WLAN_MIB_RMRQST_CH_LOADRPT_CDT_BUTT
} wlan_mib_rmrqst_ch_loadrpt_cdt_type_enum;

/* dot11RMRqstNoiseHistogramReportingCondition OBJECT-TYPE  */
/* SYNTAX INTEGER {                                         */
/* afterEveryMeasurement(0),                                */
/* aNPIAboveReference(1),                                   */
/* aNPIBelowReference(2) }                                  */
typedef enum {
    WLAN_MIB_RMRQST_NOISE_HISTGRPT_CDT_AFTER_EVERY_MEAS = 0,
    WLAN_MIB_RMRQST_NOISE_HISTGRPT_CDT_ANPI_ABOVE_REF = 1,
    WLAN_MIB_RMRQST_NOISE_HISTGRPT_CDT_ANPI_BELOW_REF = 2,

    WLAN_MIB_RMRQST_NOISE_HISTGRPT_CDT_BUTT
} wlan_mib_rmrqst_noise_histgrpt_cdt_type_enum;

/* dot11LCIDSEAltitudeType OBJECT-TYPE              */
/* SYNTAX INTEGER { meters(1), floors(2), hagm(3) } */
typedef enum {
    WLAN_MIB_LCI_DSEALTITUDE_TYPE_METERS = 1,
    WLAN_MIB_LCI_DSEALTITUDE_TYPE_FLOORS = 2,
    WLAN_MIB_LCI_DSEALTITUDE_TYPE_HAGM = 3,

    WLAN_MIB_LCI_DSEALTITUDE_TYPE_BUTT
} wlan_mib_lci_dsealtitude_type_enum;

/* dot11MIMOPowerSave OBJECT-TYPE                    */
/* SYNTAX INTEGER { static(1), dynamic(2), mimo(3) } */
typedef enum {
    WLAN_MIB_MIMO_POWER_SAVE_STATIC = 1,
    WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC = 2,
    WLAN_MIB_MIMO_POWER_SAVE_MIMO = 3,

    WLAN_MIB_MIMO_POWER_SAVE_BUTT
} wlan_mib_mimo_power_save_enum;

/* dot11MaxAMSDULength OBJECT-TYPE            */
/* SYNTAX INTEGER { short(3839), long(7935) } */
typedef enum {
    WLAN_MIB_MAX_AMSDU_LENGTH_SHORT = 3839,
    WLAN_MIB_MAX_AMSDU_LENGTH_LONG = 7935,

    WLAN_MIB_MAX_AMSDU_LENGTH_BUTT
} wlan_mib_max_amsdu_lenth_enum;

/* dot11MCSFeedbackOptionImplemented OBJECT-TYPE         */
/* SYNTAX INTEGER { none(0), unsolicited (2), both (3) } */
typedef enum {
    WLAN_MIB_MCS_FEEDBACK_OPT_IMPLT_NONE = 0,
    WLAN_MIB_MCS_FEEDBACK_OPT_IMPLT_UNSOLICITED = 2,
    WLAN_MIB_MCS_FEEDBACK_OPT_IMPLT_BOTH = 3,

    WLAN_MIB_MCS_FEEDBACK_OPT_IMPLT_BUTT
} wlan_mib_mcs_feedback_opt_implt_enum;

/* dot11LocationServicesLIPReportIntervalUnits OBJECT-TYPE */
/* SYNTAX INTEGER {                                        */
/* hours(0),                                               */
/* minutes(1),                                             */
/* seconds(2),                                             */
/* milliseconds(3)                                         */
typedef enum {
    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_HOURS = 0,
    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_MINUTES = 1,
    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_SECONDS = 2,
    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_MILLISECDS = 3,

    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_BUTT
} wlan_mib_lct_servs_liprpt_interval_unit_enum;

/* dot11WirelessMGTEventType OBJECT-TYPE */
/* SYNTAX INTEGER {                      */
/* transition(0),                        */
/* rsna(1),                              */
/* peerToPeer(2),                        */
/* wnmLog(3),                            */
/* vendorSpecific(221)                   */
typedef enum {
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_TRANSITION = 0,
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_RSNA = 1,
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_PEERTOPEER = 2,
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_WNMLOG = 3,
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_VENDOR_SPECIFIC = 221,

    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_BUTT
} wlan_mib_wireless_mgt_event_type_enum;

/* dot11WirelessMGTEventStatus OBJECT-TYPE */
/* SYNTAX INTEGER {                        */
/* successful(0),                          */
/* requestFailed(1),                       */
/* requestRefused(2),                      */
/* requestIncapable(3),                    */
/* detectedFrequentTransition(4)           */
typedef enum {
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_SUCC = 0,
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_RQST_FAIL = 1,
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_RQST_REFUSE = 2,
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_RQST_INCAP = 3,
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_DETECT_FREQ_TRANSIT = 4,

    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_BUTT
} wlan_mib_wireless_mgt_event_status_enum;

/* dot11WirelessMGTEventTransitionReason OBJECT-TYPE   */
/* SYNTAX INTEGER {                                    */
/* unspecified(0),                                     */
/* excessiveFrameLossRatesPoorConditions(1),           */
/* excessiveDelayForCurrentTrafficStreams(2),          */
/* insufficientQosCapacityForCurrentTrafficStreams(3), */
/* firstAssociationToEss(4),                           */
/* loadBalancing(5),                                   */
/* betterApFound(6),                                   */
/* deauthenticatedDisassociatedFromPreviousAp(7),      */
/* certificateToken(8),                                */
/* apFailedIeee8021XEapAuthentication(9),              */
/* apFailed4wayHandshake(10),                          */
/* excessiveDataMICFailures(11),                       */
/* exceededFrameTransmissionRetryLimit(12),            */
/* ecessiveBroadcastDisassociations(13),               */
/* excessiveBroadcastDeauthentications(14),            */
/* previousTransitionFailed(15)                        */
typedef enum {
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_UNSPEC = 0,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_FRAME_LOSSRATE_POORCDT = 1,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_DELAY_CURRT_TRAFIC_STRMS = 2,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_INSUF_QOS_CAP_CURRT_TRAFIC_STRMS = 3,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_FIRST_ASSO_ESS = 4,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_LOAD_BALANCE = 5,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_BETTER_AP_FOUND = 6,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_DEAUTH_DISASSO_FROM_PRE_AP = 7,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_CERTIF_TOKEN = 8,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_AP_FAIL_IEEE8021X_EAP_AUTH = 9,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_AP_FAIL_4WAY_HANDSHAKE = 10,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_DATA_MIC_FAIL = 11,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCEED_FRAME_TRANS_RETRY_LIMIT = 12,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_BROAD_DISASSO = 13,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_BROAD_DISAUTH = 14,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_PRE_TRANSIT_FAIL = 15,

    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_BUTT
} wlan_mib_wireless_mgt_event_transit_reason_enum;

/* dot11WNMRqstType OBJECT-TYPE */
/* SYNTAX INTEGER {             */
/* mcastDiagnostics(0),         */
/* locationCivic(1),            */
/* locationIdentifier(2),       */
/* event(3),                    */
/* dignostic(4),                */
/* locationConfiguration(5),    */
/* bssTransitionQuery(6),       */
/* bssTransitionRqst(7),        */
/* fms(8),                      */
/* colocInterference(9)         */
typedef enum {
    WLAN_MIB_WNM_RQST_TYPE_MCAST_DIAG = 0,
    WLAN_MIB_WNM_RQST_TYPE_LOCATION_CIVIC = 1,
    WLAN_MIB_WNM_RQST_TYPE_LOCATION_IDTIF = 2,
    WLAN_MIB_WNM_RQST_TYPE_EVENT = 3,
    WLAN_MIB_WNM_RQST_TYPE_DIAGOSTIC = 4,
    WLAN_MIB_WNM_RQST_TYPE_LOCATION_CFG = 5,
    WLAN_MIB_WNM_RQST_TYPE_BSS_TRANSIT_QUERY = 6,
    WLAN_MIB_WNM_RQST_TYPE_BSS_TRANSIT_RQST = 7,
    WLAN_MIB_WNM_RQST_TYPE_FMS = 8,
    WLAN_MIB_WNM_RQST_TYPE_COLOC_INTERF = 9,

    WLAN_MIB_WNM_RQST_TYPE_BUTT
} wlan_mib_wnm_rqst_type_enum;

/* dot11WNMRqstLCRRqstSubject OBJECT-TYPE */
/* SYNTAX INTEGER {                       */
/* local(0),                              */
/* remote(1)                              */
typedef enum {
    WLAN_MIB_WNM_RQST_LCRRQST_SUBJ_LOCAL = 0,
    WLAN_MIB_WNM_RQST_LCRRQST_SUBJ_REMOTE = 1,

    WLAN_MIB_WNM_RQST_LCRRQST_SUBJ_BUTT
} wlan_mib_wnm_rqst_lcrrqst_subj_enum;

/* dot11WNMRqstLCRIntervalUnits OBJECT-TYPE */
/* SYNTAX INTEGER {                         */
/* seconds(0),                              */
/* minutes(1),                              */
/* hours(2)                                 */
typedef enum {
    WLAN_MIB_WNM_RQST_LCR_INTERVAL_UNIT_SECOND = 0,
    WLAN_MIB_WNM_RQST_LCR_INTERVAL_UNIT_MINUTE = 1,
    WLAN_MIB_WNM_RQST_LCR_INTERVAL_UNIT_HOUR = 2,

    WLAN_MIB_WNM_RQST_LCR_INTERVAL_UNIT_BUTT
} wlan_mib_wnm_rqst_lcr_interval_unit_enum;

/* dot11WNMRqstLIRRqstSubject OBJECT-TYPE */
/* SYNTAX INTEGER {                       */
/* local(0),                              */
/* remote(1)                              */
typedef enum {
    WLAN_MIB_WNM_RQST_LIRRQST_SUBJ_LOCAL = 0,
    WLAN_MIB_WNM_RQST_LIRRQST_SUBJ_REMOTE = 1,

    WLAN_MIB_WNM_RQST_LIRRQST_SUBJ_BUTT
} wlan_mib_wnm_rqst_lirrqst_subj_enum;

/* dot11WNMRqstLIRIntervalUnits OBJECT-TYPE */
/* SYNTAX INTEGER {                         */
/* seconds(0),                              */
/* minutes(1),                              */
/* hours(2)                                 */
typedef enum {
    WLAN_MIB_WNM_RQST_LIR_INTERVAL_UNIT_SECOND = 0,
    WLAN_MIB_WNM_RQST_LIR_INTERVAL_UNIT_MINUTE = 1,
    WLAN_MIB_WNM_RQST_LIR_INTERVAL_UNIT_HOUR = 2,

    WLAN_MIB_WNM_RQST_LIR_INTERVAL_UNIT_BUTT
} wlan_mib_wnm_rqst_lir_interval_unit_enum;

/* dot11WNMRqstEventType OBJECT-TYPE */
/* SYNTAX INTEGER {                      */
/* transition(0),                        */
/* rsna(1),                              */
/* peerToPeer(2),                        */
/* wnmLog(3),                            */
/* vendorSpecific(221)                   */
typedef enum {
    WLAN_MIB_WNM_RQST_EVENT_TYPE_TRANSITION = 0,
    WLAN_MIB_WNM_RQST_EVENT_TYPE_RSNA = 1,
    WLAN_MIB_WNM_RQST_EVENT_TYPE_PEERTOPEER = 2,
    WLAN_MIB_WNM_RQST_EVENT_TYPE_WNMLOG = 3,
    WLAN_MIB_WNM_RQST_EVENT_TYPE_VENDOR_SPECIFIC = 221,

    WLAN_MIB_WNM_RQST_EVENT_TYPE_BUTT
} wlan_mib_wnm_rqst_event_tpye_enum;

/* dot11WNMRqstDiagType OBJECT-TYPE */
/* SYNTAX INTEGER {                 */
/* cancelRequest(0),                */
/* manufacturerInfoStaRep(1),       */
/* configurationProfile(2),         */
/* associationDiag(3),              */
/* ieee8021xAuthDiag(4),            */
/* vendorSpecific(221)              */
typedef enum {
    WLAN_MIB_WNM_RQST_DIAG_TYPE_CANCEL_RQST = 0,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_MANUFCT_INFO_STA_REP = 1,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_CFG_PROFILE = 2,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_ASSO_DIAG = 3,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_IEEE8021X_AUTH_DIAG = 4,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_VENDOR_SPECIFIC = 221,

    WLAN_MIB_WNM_RQST_DIAG_TYPE_BUTT
} wlan_mib_wnm_rqst_diag_type_enum;

/* dot11WNMRqstDiagCredentials OBJECT-TYPE */
/* SYNTAX INTEGER {                        */
/* none(0),                                */
/* preSharedKey(1),                        */
/* usernamePassword(2),                    */
/* x509Certificate(3),                     */
/* otherCertificate(4),                    */
/* oneTimePassword(5),                     */
/* token(6)                                */
typedef enum {
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_NONT = 0,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_PRE_SHAREKEY = 1,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_USERNAME_PASSWORD = 2,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_X509_CTERTIFICATE = 3,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_OTHER_CTERTIFICATE = 4,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_ONE_TIME_PASSWORD = 5,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_TOKEN = 6,

    WLAN_MIB_WNM_RQST_DIAG_CREDENT_BUTT
} wlan_mib_wnm_rqst_diag_credent_enum;

/* dot11WNMRqstBssTransitQueryReason OBJECT-TYPE       */
/* SYNTAX INTEGER {                                    */
/* unspecified(0),                                     */
/* excessiveFrameLossRatesPoorConditions(1),           */
/* excessiveDelayForCurrentTrafficStreams(2),          */
/* insufficientQosCapacityForCurrentTrafficStreams(3), */
/* firstAssociationToEss(4),                           */
/* loadBalancing(5),                                   */
/* betterApFound(6),                                   */
/* deauthenticatedDisassociatedFromPreviousAp(7),      */
/* apFailedIeee8021XEapAuthentication(8),              */
/* apFailed4wayHandshake(9),                           */
/* receivedTooManyReplayCounterFailures(10),           */
/* receivedTooManyDataMICFailures(11),                 */
/* exceededMaxNumberOfRetransmissions(12),             */
/* receivedTooManyBroadcastDisassociations(13),        */
/* receivedTooManyBroadcastDeauthentications(14),      */
/* previousTransitionFailed(15),                       */
/* lowRSSI(16)                                         */
typedef enum {
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_UNSPEC = 0,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_EXCES_FRAME_LOSSRATE_POORCDT = 1,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_EXCES_DELAY_CURRT_TRAFIC_STRMS = 2,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_INSUF_QOS_CAP_CURRT_TRAFIC_STRMS = 3,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_FIRST_ASSO_ESS = 4,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_LOAD_BALANCE = 5,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_BETTER_AP_FOUND = 6,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_DEAUTH_DISASSO_FROM_PRE_AP = 7,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_AP_FAIL_IEEE8021X_EAP_AUTH = 8,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_AP_FAIL_4WAY_HANDSHAKE = 9,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_RECEIVE_TOOMANY_REPLAY_COUNT_FAIL = 10,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_RECEIVE_TOOMANY_DATA_MIC_FAIL = 11,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_EXCEED_MAXNUM_RETANS = 12,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_RECEIVE_TOOMANY_BRDCAST_DISASSO = 13,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_RECEIVE_TOOMANY_BRDCAST_DEAUTH = 14,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_PRE_TRANSIT_FAIL = 15,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_LOW_RSSI = 16,

    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_BUTT
} wlan_mib_wnm_rqst_bss_transit_query_reason_enum;

/* dot11WNMEventTransitRprtEventStatus OBJECT-TYPE */
/* SYNTAX INTEGER {                                */
/* successful(0),                                  */
/* requestFailed(1),                               */
/* requestRefused(2),                              */
/* requestIncapable(3),                            */
/* detectedFrequentTransition(4)                   */
typedef enum {
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_SUCC = 0,
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_RQST_FAIL = 1,
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_RQST_REFUSE = 2,
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_RQST_INCAP = 3,
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_DETECT_FREQ_TRANSIT = 4,

    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_BUTT
} wlan_mib_wnm_event_transit_rprt_event_status_enum;

/* dot11WNMEventTransitRprtTransitReason OBJECT-TYPE   */
/* SYNTAX INTEGER {                                    */
/* unspecified(0),                                     */
/* excessiveFrameLossRatesPoorConditions(1),           */
/* excessiveDelayForCurrentTrafficStreams(2),          */
/* insufficientQosCapacityForCurrentTrafficStreams(3), */
/* firstAssociationToEss(4),                           */
/* loadBalancing(5),                                   */
/* betterApFound(6),                                   */
/* deauthenticatedDisassociatedFromPreviousAp(7),      */
/* apFailedIeee8021XEapAuthentication(8),              */
/* apFailed4wayHandshake(9),                           */
/* receivedTooManyReplayCounterFailures(10),           */
/* receivedTooManyDataMICFailures(11),                 */
/* exceededMaxNumberOfRetransmissions(12),             */
/* receivedTooManyBroadcastDisassociations(13),        */
/* receivedTooManyBroadcastDeauthentications(14),      */
/* previousTransitionFailed(15),                       */
/* lowRSSI(16)                                         */
typedef enum {
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_UNSPEC = 0,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_EXCES_FRAME_LOSSRATE_POORCDT = 1,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_EXCES_DELAY_CURRT_TRAFIC_STRMS = 2,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_INSUF_QOS_CAP_CURRT_TRAFIC_STRMS = 3,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_FIRST_ASSO_ESS = 4,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_LOAD_BALANCE = 5,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_BETTER_AP_FOUND = 6,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_DEAUTH_DISASSO_FROM_PRE_AP = 7,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_AP_FAIL_IEEE8021X_EAP_AUTH = 8,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_AP_FAIL_4WAY_HANDSHAKE = 9,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_RECEIVE_TOOMANY_REPLAY_COUNT_FAIL = 10,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_RECEIVE_TOOMANY_DATA_MIC_FAIL = 11,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_EXCEED_MAXNUM_RETANS = 12,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_RECEIVE_TOOMANY_BRDCAST_DISASSO = 13,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_RECEIVE_TOOMANY_BRDCAST_DEAUTH = 14,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_PRE_TRANSIT_FAIL = 15,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_LOW_RSSI = 16,

    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_BUTT
} wlan_mib_wnm_event_transitrprt_transit_reason_enum;

/* dot11WNMEventRsnaRprtEventStatus OBJECT-TYPE */
/* SYNTAX INTEGER {                             */
/* successful(0),                               */
/* requestFailed(1),                            */
/* requestRefused(2),                           */
/* requestIncapable(3),                         */
/* detectedFrequentTransition(4)                */
typedef enum {
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_SUCC = 0,
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_RQST_FAIL = 1,
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_RQST_REFUSE = 2,
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_RQST_INCAP = 3,
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_DETECT_FREQ_TRANSIT = 4,

    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_BUTT
} wlan_mib_wnm_event_rsnarprt_event_status_enum;

/* dot11APLCIDatum OBJECT-TYPE */
/* SYNTAX INTEGER {            */
/* wgs84 (1),                  */
/* nad83navd88 (2),            */
/* nad93mllwvd (3)             */
typedef enum {
    WLAN_MIB_AP_LCI_DATUM_WGS84 = 1,
    WLAN_MIB_AP_LCI_DATUM_NAD83_NAVD88 = 2,
    WLAN_MIB_AP_LCI_DATUM_NAD93_MLLWVD = 3,

    WLAN_MIB_AP_LCI_DATUM_BUTT
} wlan_mib_ap_lci_datum_enum;

/* dot11APLCIAzimuthType OBJECT-TYPE */
/* SYNTAX INTEGER {                  */
/* frontSurfaceOfSTA(0),             */
/* radioBeam(1) }                    */
typedef enum {
    WLAN_MIB_AP_LCI_AZIMUTH_TYPE_FRONT_SURFACE_STA = 0,
    WLAN_MIB_AP_LCI_AZIMUTH_TYPE_RADIO_BEAM = 1,

    WLAN_MIB_AP_LCI_AZIMUTH_TYPE_BUTT
} wlan_mib_ap_lci_azimuth_type_enum;

/* dot11HTProtection 枚举定义 */
typedef enum {
    WLAN_MIB_HT_NO_PROTECTION = 0,
    WLAN_MIB_HT_NONMEMBER_PROTECTION = 1,
    WLAN_MIB_HT_20MHZ_PROTECTION = 2,
    WLAN_MIB_HT_NON_HT_MIXED = 3,

    WLAN_MIB_HT_PROTECTION_BUTT
} wlan_mib_ht_protection_enum;

/* dot11ExplicitCompressedBeamformingFeedbackOptionImplemented OBJECT-TYPE */
/* SYNTAX INTEGER {                                                        */
/* inCapable (0),                                                          */
/* delayed (1),                                                            */
/* immediate (2),                                                          */
/* unsolicitedImmediate (3),                                               */
/* aggregated (4),                                                         */
/* delayedAggregated (5),                                                  */
/* immediateAggregated (6),                                                */
/* unsolicitedImmediateAggregated (7)}                                     */
typedef enum {
    WLAN_MIB_HT_ECBF_INCAPABLE = 0,
    WLAN_MIB_HT_ECBF_DELAYED = 1,
    WLAN_MIB_HT_ECBF_IMMEDIATE = 2,

    /* Tip:if you select it(3) and use for HT cap, it stands for both 1 and 2. page656 */
    WLAN_MIB_HT_ECBF_UNSOLIMMEDI = 3,
    WLAN_MIB_HT_ECBF_AGGREGATE = 4,
    WLAN_MIB_HT_ECBF_DELAYAGGR = 5,
    WLAN_MIB_HT_ECBF_IMMEDIAGGR = 6,
    WLAN_MIB_HT_ECBF_UNSOLIMMAGGR = 7,

    WLAN_MIB_HT_ECBF_TYPE_BUTT
} wlan_mib_ht_ecbf_enum;

/* VHT Capabilities Info field 最大MPDU长度枚举 */
typedef enum {
    WLAN_MIB_VHT_MPDU_3895 = 0,
    WLAN_MIB_VHT_MPDU_7991 = 1,
    WLAN_MIB_VHT_MPDU_11454 = 2,

    WLAN_MIB_VHT_MPDU_LEN_BUTT
} wlan_mib_vht_mpdu_len_enum;

/* VHT Capabilites Info field 支持带宽枚举 */
typedef enum {
    WLAN_MIB_VHT_SUPP_WIDTH_80 = 0,       /* 不支持160或者80+80 */
    WLAN_MIB_VHT_SUPP_WIDTH_160 = 1,      /* 支持160 */
    WLAN_MIB_VHT_SUPP_WIDTH_80PLUS80 = 2, /* 支持160和80+80 */

    WLAN_MIB_VHT_SUPP_WIDTH_BUTT
} wlan_mib_vht_supp_width_enum;

/* VHT Operation Info  field 支持带宽枚举 */
typedef enum {
    WLAN_MIB_VHT_OP_WIDTH_20_40 = 0,    /* 工作在20/40 */
    WLAN_MIB_VHT_OP_WIDTH_80 = 1,       /* 工作在80 */
    WLAN_MIB_VHT_OP_WIDTH_160 = 2,      /* 工作在160 */
    WLAN_MIB_VHT_OP_WIDTH_80PLUS80 = 3, /* 工作在80+80 */

    WLAN_MIB_VHT_OP_WIDTH_BUTT
} wlan_mib_vht_op_width_enum;

/* 当前MIB表项还未稳定，暂不需要4字节对齐，待后续MIB项稳定后再来调整这些结构体 */ /*lint -e958*/ /*lint -e959*/
/* 是否需要该结构体表示变长的字符串，待确定 */
typedef struct {
    uint16_t us_octet_nums;    /* 字节长度，此值为0时表示没有字符串信息 */
    uint8_t *puc_octec_string; /* 字符串起始地址 */
} wlan_mib_octet_string_stru;

typedef struct {
    uint8_t uc_mcs_nums;
    uint8_t *pst_mcs_set;
} wlan_mib_ht_op_mcs_set_stru; /* dot11HTOperationalMCSSet */

/* dot11LocationServicesLocationIndicationIndicationParameters OBJECT-TYPE */
/* SYNTAX OCTET STRING (SIZE (1..255))                                     */
/* MAX-ACCESS read-create                                                  */
/* STATUS current                                                          */
/* DESCRIPTION                                                             */
/* "This attribute indicates the location Indication Parameters used for   */
/* transmitting Location Track Notification Frames."                       */
/* ::= { dot11LocationServicesEntry 15}                                    */
typedef struct {
    uint8_t uc_para_nums;
    uint8_t *uc_para;
} wlan_mib_local_serv_location_ind_ind_para_stru;

/* Start of dot11smt OBJECT IDENTIFIER ::= { ieee802dot11 1 } */
typedef struct {
    uint8_t auc_dot11StationID[WLAN_MAC_ADDR_LEN];      /* dot11StationID MacAddress, */
    uint8_t auc_p2p0_dot11StationID[WLAN_MAC_ADDR_LEN]; /* P2P0 dot11StationID MacAddress, */

    uint32_t dot11AuthenticationResponseTimeOut;    /* dot11AuthenticationResponseTimeOut Unsigned32, */
    uint32_t dot11BeaconPeriod;                     /* dot11BeaconPeriod Unsigned32, */
    uint32_t dot11DTIMPeriod;                       /* dot11DTIMPeriod Unsigned32, */
    uint32_t dot11AssociationResponseTimeOut;       /* dot11AssociationResponseTimeOut Unsigned32, */
    uint32_t dot11AssociationSAQueryMaximumTimeout; /* dot11AssociationSAQueryMaximumTimeout Unsigned32, */
    uint32_t dot11AssociationSAQueryRetryTimeout;   /* dot11AssociationSAQueryRetryTimeout Unsigned32, */

    uint8_t auc_dot11DesiredSSID[WLAN_SSID_MAX_LEN]; /* dot11DesiredSSID OCTET STRING, SIZE(0..32) */
    /* +1预留\0 */
    uint16_t us_dot11MaxAssocUserNums;

    oal_bool_enum_uint8 en_dot11TxAggregateActived;   /* VAP协议能力是否支持发送聚合 */
    oal_bool_enum_uint8 en_dot11CfgAmpduTxAtive;      /* 配置命令是否允许发送聚合 */
    oal_bool_enum_uint8 en_dot11AmsduPlusAmpduActive; /* ADDBA REQ中标志是否支持amsdu */
    oal_bool_enum_uint8 en_dot11CfgAmsduTxAtive;      /* 配置命令是否支持amsdu */

    oal_bool_enum_uint8 en_dot11AmsduAggregateAtive; /* VAP协议能力是否支持amsdu发送聚合 */
    uint8_t uc_dot11RxBASessionNumber;             /* 该vap下，rx BA会话的数目 */
    uint8_t uc_dot11TxBASessionNumber;             /* 该vap下，tx BA会话的数目 */
    uint8_t uc_dot11VAPClassifyTidNo;              /* 仅在基于vap的流分类使能后有效 */

    wlan_auth_alg_mode_enum_uint8 en_dot11AuthenticationMode; /* 认证算法 */
    wlan_addba_mode_enum_uint8 en_dot11AddBaMode;
    uint8_t en_dot11DesiredBSSType;    /* wlan_mib_desired_bsstype_enum: dot11DesiredBSSType INTEGER, */
    uint8_t dot11PowerManagementMode; /* wlan_mib_pwr_mgmt_mode_enum: dot11PowerManagementMode INTEGER, */

    uint8_t uc_dot11StaAuthCount; /* 记录STA发起关联的次数 */
    uint8_t uc_dot11StaAssocCount;
    uint8_t uc_dot11SupportRateSetNums;
    uint8_t en_dot11TxTrafficClassifyFlag; /* 业务识别功能开关 */

    /* Byte1 */
    oal_bool_enum_uint8 en_dot11MultiDomainCapabilityActivated : 1;     /* dot11MultiDomainCapabilityActivated */
    oal_bool_enum_uint8 en_dot11SpectrumManagementImplemented : 1;      /* dot11SpectrumManagementImplemented */
    oal_bool_enum_uint8 en_dot11SpectrumManagementRequired : 1;         /* dot11SpectrumManagementRequired */
    oal_bool_enum_uint8 en_dot11QosOptionImplemented : 1;               /* dot11QosOptionImplemented TruthValue, */
    oal_bool_enum_uint8 en_dot11ImmediateBlockAckOptionImplemented : 1; /* dot11ImmediateBlockAckOptionImplemented */
    oal_bool_enum_uint8 en_dot11DelayedBlockAckOptionImplemented : 1;   /* dot11DelayedBlockAckOptionImplemented */
    oal_bool_enum_uint8 en_dot11APSDOptionImplemented : 1;              /* dot11APSDOptionImplemented TruthValue, */
    oal_bool_enum_uint8 en_dot11QBSSLoadImplemented : 1;                /* dot11QBSSLoadImplemented TruthValue, */
    /* Byte2 */
    oal_bool_enum_uint8 en_dot11RadioMeasurementActivated : 1;           /* dot11RadioMeasurementActivated */
    oal_bool_enum_uint8 en_dot11ExtendedChannelSwitchActivated : 1;      /* dot11ExtendedChannelSwitchActivated */
    oal_bool_enum_uint8 en_dot11RMBeaconTableMeasurementActivated : 1;   /* dot11RMBeaconTableMeasurementActivated */
    oal_bool_enum_uint8 en_dot11WirelessManagementImplemented : 1;       /* dot11WirelessManagementImplemented */
    oal_bool_enum_uint8 en_resv1 : 4;
    /* Byte3 */
    oal_bool_enum_uint8 en_dot112040SwitchProhibited : 1;
    oal_bool_enum_uint8 en_dot11WPSActive : 1;
    oal_bool_enum_uint8 en_resv : 6;
    /* Byte4 */
    uint8_t en_dot1180211iMode; /* 指示当前的方式时WPA还是WPA2, bit0 = 1,WPA; bit1 = 1, RSN */

    wlan_channel_bandwidth_enum_uint8 en_dot11VapMaxBandWidth; /* 当前vap可用的最大带宽能力 */
} wlan_mib_Dot11StationConfigEntry_stru;

/******************************************************************************/
/* dot11WEPDefaultKeys  TABLE - members of Dot11WEPDefaultKeysEntry           */
/******************************************************************************/
/* Conceptual table for WEP default keys. This table contains the four WEP    */
/* default secret key values corresponding to the four possible KeyID values. */
/* The WEP default secret keys are logically WRITE-ONLY. Attempts to read the */
/* entries in this table return unsuccessful status and values of null or 0.  */
/* The default value of each WEP default key is null.                         */
typedef struct {
    uint8_t auc_dot11WEPDefaultKeyValue[WLAN_MAX_WEP_STR_SIZE]; /* dot11WEPDefaultKeyValue WEPKeytype  */
    uint8_t uc_rsv;
} wlan_mib_Dot11WEPDefaultKeysEntry_stru;

/******************************************************************************/
/* dot11RSNAConfig TABLE (RSNA and TSN) - members of dot11RSNAConfigEntry     */
/******************************************************************************/
/* An entry in the dot11RSNAConfigTable                                       */
typedef struct {
    uint8_t uc_dot11RSNAConfigPTKSAReplayCounters; /* dot11RSNAConfigNumberOfPTKSAReplayCountersImplemented */
    uint8_t uc_dot11RSNAConfigGTKSAReplayCounters; /* dot11RSNAConfigNumberOfGTKSAReplayCountersImplemented */
    uint32_t wpa_group_suite;
    uint32_t rsn_group_suite;
    uint32_t aul_wpa_pair_suites[WLAN_PAIRWISE_CIPHER_SUITES];
    uint32_t aul_wpa_akm_suites[WLAN_AUTHENTICATION_SUITES];
    uint32_t aul_rsn_pair_suites[WLAN_PAIRWISE_CIPHER_SUITES];
    uint32_t aul_rsn_akm_suites[WLAN_AUTHENTICATION_SUITES];
    uint32_t rsn_group_mgmt_suite;
} wlan_mib_dot11RSNAConfigEntry_stru;

/******************************************************************************/
/* dot11WEPKeyMappings  TABLE - members of Dot11WEPKeyMappingsEntry           */
/******************************************************************************/
/* Conceptual table for WEP Key Mappings. The MIB supports the ability to     */
/* share a separate WEP key for each RA/TA pair. The Key Mappings Table con-  */
/* tains zero or one entry for each MAC address and contains two fields for   */
/* each entry: WEPOn and the corresponding WEP key. The WEP key mappings are  */
/* logically WRITE-ONLY. Attempts to read the entries in this table return    */
/* unsuccessful status and values of null or 0. The default value for all     */
/* WEPOn fields is false                                                      */
/******************************************************************************/
/* dot11Privacy TABLE - members of Dot11PrivacyEntry                          */
/******************************************************************************/
/* Group containing attributes concerned with IEEE 802.11 Privacy. Created    */
/* as a table to allow multiple instantiations on an agent.                   */
typedef struct {
    oal_bool_enum_uint8 en_dot11PrivacyInvoked : 1;                 /* dot11PrivacyInvoked TruthValue */
    oal_bool_enum_uint8 en_dot11RSNAActivated : 1;                  /* dot11RSNAActivated TruthValue */
    oal_bool_enum_uint8 en_dot11RSNAPreauthenticationActivated : 1; /* dot11RSNAPreauthenticationActivated TruthValue */
    oal_bool_enum_uint8 en_dot11RSNAMFPC : 1;                       /* dot11RSNAManagementFramerProtectionCapbility */
    oal_bool_enum_uint8 en_dot11RSNAMFPR : 1;                       /* dot11RSNAManagementFramerProtectionRequired */
    oal_bool_enum_uint8 en_resv : 3;

    uint8_t uc_dot11WEPDefaultKeyID; /* dot11WEPDefaultKeyID Unsigned8,                 */
    uint8_t auc_reserve[NUM_2_BYTES];

    wlan_mib_Dot11WEPDefaultKeysEntry_stru ast_wlan_mib_wep_dflt_key[WLAN_NUM_DOT11WEPDEFAULTKEYVALUE];
    wlan_mib_dot11RSNAConfigEntry_stru st_wlan_mib_rsna_cfg;
} wlan_mib_Dot11PrivacyEntry_stru;

/******************************************************************************/
/* dot11MultiDomainCapability TABLE - members of Dot11MultiDomainCapabilityEntry            */
/******************************************************************************/
/* GThis (conceptual) table of attributes for cross-domain mobility           */
typedef struct {
    uint32_t dot11MultiDomainCapabilityIndex; /* dot11MultiDomainCapabilityIndex Unsigned32, */
    uint32_t dot11FirstChannelNumber;         /* dot11FirstChannelNumber Unsigned32,         */
    uint32_t dot11NumberofChannels;           /* dot11NumberofChannels Unsigned32,           */
    int32_t l_dot11MaximumTransmitPowerLevel;    /* dot11MaximumTransmitPowerLevel Integer32    */
} wlan_mib_Dot11MultiDomainCapabilityEntry_stru;

/******************************************************************************/
/* dot11SpectrumManagement TABLE - members of dot11SpectrumManagementEntry    */
/******************************************************************************/
/* An entry (conceptual row) in the Spectrum Management Table.                */
/* IfIndex - Each 802.11 interface is represented by an ifEntry. Interface    */
/* tables in this MIB are indexed by ifIndex.                                 */
typedef struct {
    uint32_t dot11SpectrumManagementIndex;     /* dot11SpectrumManagementIndex Unsigned32,      */
    int32_t l_dot11MitigationRequirement;         /* dot11MitigationRequirement Integer32,         */
    uint32_t dot11ChannelSwitchTime;           /* dot11ChannelSwitchTime Unsigned32,            */
    int32_t l_dot11PowerCapabilityMaxImplemented; /* dot11PowerCapabilityMaxImplemented Integer32, */
    int32_t l_dot11PowerCapabilityMinImplemented; /* dot11PowerCapabilityMinImplemented Integer32  */
} wlan_mib_dot11SpectrumManagementEntry_stru;

/****************************************************************************/
/* dot11RSNAStats TABLE - members of Dot11RSNAStatsEntry                    */
/****************************************************************************/
/* This table maintains per-STA statistics in an RSN. The entry with        */
/* dot11RSNAStatsSTAAddress equal to FF-FF-FF-FF-FF-FF contains statistics  */
/* for group addressed traffic                                              */
/***********************************************************************/
/* dot11OperatingClasses TABLE - members of Dot11OperatingClassesEntry */
/***********************************************************************/
/* (Conceptual) table of attributes for operating classes       */
typedef struct {
    uint32_t dot11OperatingClassesIndex; /* dot11OperatingClassesIndex Unsigned32,   */
    uint32_t dot11OperatingClass;        /* dot11OperatingClass Unsigned32,          */
    uint32_t dot11CoverageClass;         /* dot11CoverageClass Unsigned32            */
} wlan_mib_Dot11OperatingClassesEntry_stru;

/***********************************************************************/
/* dot11RMRequest TABLE  - members of dot11RadioMeasurement         */
/***********************************************************************/
typedef struct {
    /* dot11RMRequestNextIndex ::= { dot11RMRequest 1 } */
    uint32_t dot11RMRequestNextIndex; /* dot11RMRequestNextIndex  Unsigned32 */

    /*  dot11RMRequestTable OBJECT-TYPE ::= { dot11RMRequest 2 } */
    uint32_t dot11RMRqstIndex;
    uint8_t en_dot11RMRqstRowStatus;   /* wlan_mib_row_status_enum */
    uint8_t auc_dot11RMRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11RMRqstRepetitions;
    uint32_t dot11RMRqstIfIndex;
    uint16_t en_dot11RMRqstType;       /* wlan_mib_rmrqst_type_enum */
    uint8_t auc_dot11RMRqstTargetAdd[NUM_6_BYTES];
    uint32_t dot11RMRqstTimeStamp;
    uint32_t dot11RMRqstChanNumber;
    uint32_t dot11RMRqstOperatingClass;
    uint32_t dot11RMRqstRndInterval;
    uint32_t dot11RMRqstDuration;
    oal_bool_enum_uint8 en_dot11RMRqstParallel;
    oal_bool_enum_uint8 en_dot11RMRqstEnable;
    oal_bool_enum_uint8 en_dot11RMRqstRequest;
    oal_bool_enum_uint8 en_dot11RMRqstReport;
    oal_bool_enum_uint8 en_dot11RMRqstDurationMandatory;
    uint8_t en_dot11RMRqstBeaconRqstMode;              /* wlan_mib_rmrqst_beaconrqst_mode_enum */
    uint8_t en_dot11RMRqstBeaconRqstDetail;            /* wlan_mib_rmrqst_beaconrqst_detail_enum */
    uint8_t en_dot11RMRqstFrameRqstType;               /* wlan_mib_rmrqst_famerqst_type_enum */
    uint8_t auc_dot11RMRqstBssid[NUM_6_BYTES];
    uint8_t uc_dot11RMRqstSSID[NUM_32_BYTES];
    uint8_t en_dot11RMRqstBeaconReportingCondition;    /* wlan_mib_rmrqst_beaconrpt_cdt_enum */
    int32_t l_dot11RMRqstBeaconThresholdOffset;
    uint8_t en_dot11RMRqstSTAStatRqstGroupID;          /* wlan_mib_rmrqst_stastatrqst_grpid_enum */
    uint8_t en_dot11RMRqstLCIRqstSubject;              /* wlan_mib_rmrqst_lcirpt_subject_enum */
    uint32_t dot11RMRqstLCILatitudeResolution;
    uint32_t dot11RMRqstLCILongitudeResolution;
    uint32_t dot11RMRqstLCIAltitudeResolution;
    uint8_t en_dot11RMRqstLCIAzimuthType;              /* wlan_mib_rmrqst_lciazimuth_type_enum */
    uint32_t dot11RMRqstLCIAzimuthResolution;
    uint32_t dot11RMRqstPauseTime;
    uint8_t auc_dot11RMRqstTransmitStreamPeerQSTAAddress[NUM_6_BYTES];
    uint32_t dot11RMRqstTransmitStreamTrafficIdentifier;
    uint32_t dot11RMRqstTransmitStreamBin0Range;
    oal_bool_enum_uint8 en_dot11RMRqstTrigdQoSAverageCondition;
    oal_bool_enum_uint8 en_dot11RMRqstTrigdQoSConsecutiveCondition;
    oal_bool_enum_uint8 en_dot11RMRqstTrigdQoSDelayCondition;
    uint32_t dot11RMRqstTrigdQoSAverageThreshold;
    uint32_t dot11RMRqstTrigdQoSConsecutiveThreshold;
    uint32_t dot11RMRqstTrigdQoSDelayThresholdRange;
    uint32_t dot11RMRqstTrigdQoSDelayThreshold;
    uint32_t dot11RMRqstTrigdQoSMeasurementCount;
    uint32_t dot11RMRqstTrigdQoSTimeout;
    uint8_t en_dot11RMRqstChannelLoadReportingCondition;    /* wlan_mib_rmrqst_ch_loadrpt_cdt_type_enum */
    uint32_t dot11RMRqstChannelLoadReference;
    uint8_t en_dot11RMRqstNoiseHistogramReportingCondition; /* wlan_mib_rmrqst_noise_histgrpt_cdt_type_enum */
    uint32_t dot11RMRqstAnpiReference;
    uint8_t auc_dot11RMRqstAPChannelReport[NUM_255_BYTES];
    uint8_t auc_dot11RMRqstSTAStatPeerSTAAddress[NUM_6_BYTES];
    uint8_t auc_dot11RMRqstFrameTransmitterAddress[NUM_6_BYTES];
    uint8_t auc_dot11RMRqstVendorSpecific[NUM_255_BYTES];
    uint32_t dot11RMRqstSTAStatTrigMeasCount;
    uint32_t dot11RMRqstSTAStatTrigTimeout;
    uint8_t auc_dot11RMRqstSTAStatTrigCondition[NUM_2_BYTES];
    uint32_t dot11RMRqstSTAStatTrigSTAFailedCntThresh;
    uint32_t dot11RMRqstSTAStatTrigSTAFCSErrCntThresh;
    uint32_t dot11RMRqstSTAStatTrigSTAMultRetryCntThresh;
    uint32_t dot11RMRqstSTAStatTrigSTAFrameDupeCntThresh;
    uint32_t dot11RMRqstSTAStatTrigSTARTSFailCntThresh;
    uint32_t dot11RMRqstSTAStatTrigSTAAckFailCntThresh;
    uint32_t dot11RMRqstSTAStatTrigSTARetryCntThresh;
    uint32_t dot11RMRqstSTAStatTrigQoSTrigCondition;
    uint32_t dot11RMRqstSTAStatTrigQoSFailedCntThresh;
    uint32_t dot11RMRqstSTAStatTrigQoSRetryCntThresh;
    uint32_t dot11RMRqstSTAStatTrigQoSMultRetryCntThresh;
    uint32_t dot11RMRqstSTAStatTrigQoSFrameDupeCntThresh;
    uint32_t dot11RMRqstSTAStatTrigQoSRTSFailCntThresh;
    uint32_t dot11RMRqstSTAStatTrigQoSAckFailCntThresh;
    uint32_t dot11RMRqstSTAStatTrigQoSDiscardCntThresh;
    uint8_t auc_dot11RMRqstSTAStatTrigRsnaTrigCondition[NUM_2_BYTES];
    uint32_t dot11RMRqstSTAStatTrigRsnaCMACICVErrCntThresh;
    uint32_t dot11RMRqstSTAStatTrigRsnaCMACReplayCntThresh;
    uint32_t dot11RMRqstSTAStatTrigRsnaRobustCCMPReplayCntThresh;
    uint32_t dot11RMRqstSTAStatTrigRsnaTKIPICVErrCntThresh;
    uint32_t dot11RMRqstSTAStatTrigRsnaTKIPReplayCntThresh;
    uint32_t dot11RMRqstSTAStatTrigRsnaCCMPDecryptErrCntThresh;
    uint32_t dot11RMRqstSTAStatTrigRsnaCCMPReplayCntThresh;
} wlan_mib_dot11RadioMeasurement_stru;

/**************************************************************************************/
/* dot11FastBSSTransitionConfig TABLE  - members of Dot11FastBSSTransitionConfigEntry */
/**************************************************************************************/
/* The table containing fast BSS transition configuration objects                     */
typedef struct {
    oal_bool_enum_uint8 en_dot11FastBSSTransitionActivated : 1; /* dot11FastBSSTransitionActivated TruthValue, */
    oal_bool_enum_uint8 en_dot11FTOverDSActivated : 1;          /* dot11FTOverDSActivated TruthValue,          */
    oal_bool_enum_uint8 en_dot11FTResourceRequestSupported : 1; /* dot11FTResourceRequestSupported TruthValue, */
    oal_bool_enum_uint8 en_resv : 5;

    uint8_t resv;
    uint16_t us_dot11FTMobilityDomainID; /* dot11FTMobilityDomainID OCTET STRING,SIZE(2) */
} wlan_mib_Dot11FastBSSTransitionConfigEntry_stru;

/************************************************************************/
/* dot11LCIDSE TABLE  - members of Dot11LCIDSEEntry                     */
/************************************************************************/
/* Group contains conceptual table of attributes for Dependent Station  */
/* Enablement.                                                          */
typedef struct {
    uint32_t dot11LCIDSEIndex;
    uint32_t dot11LCIDSEIfIndex;
    uint32_t dot11LCIDSECurrentOperatingClass;
    uint32_t dot11LCIDSELatitudeResolution;
    int32_t l_dot11LCIDSELatitudeInteger;
    int32_t l_dot11LCIDSELatitudeFraction;
    uint32_t dot11LCIDSELongitudeResolution;
    int32_t l_dot11LCIDSELongitudeInteger;
    int32_t l_dot11LCIDSELongitudeFraction;
    uint8_t en_dot11LCIDSEAltitudeType;          /* wlan_mib_lci_dsealtitude_type_enum */
    uint32_t dot11LCIDSEAltitudeResolution;
    int32_t l_dot11LCIDSEAltitudeInteger;
    int32_t l_dot11LCIDSEAltitudeFraction;
    uint32_t dot11LCIDSEDatum;
    oal_bool_enum_uint8 en_dot11RegLocAgreement;
    oal_bool_enum_uint8 en_dot11RegLocDSE;
    oal_bool_enum_uint8 en_dot11DependentSTA;
    uint32_t dot11DependentEnablementIdentifier;
    uint32_t dot11DSEEnablementTimeLimit;
    uint32_t dot11DSEEnablementFailHoldTime;
    uint32_t dot11DSERenewalTime;
    uint32_t dot11DSETransmitDivisor;
} wlan_mib_Dot11LCIDSEEntry_stru;

/**************************************************************************************/
/* dot11HTStationConfig TABLE  - members of Dot11HTStationConfigEntry                 */
/**************************************************************************************/
/* Station Configuration attributes. In tabular form to allow for multiple            */
/* instances on an agent.                                                             */
typedef struct {
    oal_bool_enum_uint8 en_dot11HTControlFieldSupported : 1;             /* dot11HTControlFieldSupported TruthValue */
    oal_bool_enum_uint8 en_dot11RDResponderOptionImplemented : 1;        /* dot11RDResponderOptionImplemented */
    oal_bool_enum_uint8 en_dot11PCOOptionImplemented : 1;                /* dot11PCOOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11LsigTxopProtectionOptionImplemented : 1; /* dot11LsigTxopProtectionOptionImplemented */
    oal_bool_enum_uint8 en_dot11HighThroughputOptionImplemented : 1;     /* dot11HighThroughputOptionImplemented */
    uint8_t dot11MinimumMPDUStartSpacing : 3;                       /* dot11MinimumMPDUStartSpacing Unsigned32 */
    uint8_t resv[3];

    uint16_t en_dot11MaxAMSDULength; /* wlan_mib_max_amsdu_lenth_enum: dot11MaxAMSDULength INTEGER */
    uint8_t en_dot11MIMOPowerSave;   /* wlan_mib_mimo_power_save_enum: dot11MIMOPowerSave INTEGER */
    uint8_t en_dot11MCSFeedbackOptionImplemented; /* wlan_mib_mcs_feedback_opt_implt_enum */
    // wlan_mib_ht_op_mcs_set_stru st_dot11HTOperationalMCSSet; /* dot11HTOperationalMCSSet OCTET STRING */
    // oal_bool_enum_uint8 en_dot11NDelayedBlockAckOptionImplemented; /* dot11NDelayedBlockAckOptionImplemented */
    // oal_bool_enum_uint8 en_dot11STBCControlFrameOptionImplemented; /* dot11STBCControlFrameOptionImplemented */
    uint32_t dot11MaxRxAMPDUFactor; /* dot11MaxRxAMPDUFactor Unsigned32 */
    uint32_t dot11TransitionTime;   /* dot11TransitionTime Unsigned32 */
    // oal_bool_enum_uint8 en_dot11SPPAMSDUCapable; /* dot11SPPAMSDUCapable TruthValue */
    // oal_bool_enum_uint8 en_dot11SPPAMSDURequired; /* dot11SPPAMSDURequired TruthValue */
    // oal_bool_enum_uint8 en_dot11FortyMHzOptionImplemented; /* dot11FortyMHzOptionImplemented TruthValue */
} wlan_mib_Dot11HTStationConfigEntry_stru;

/**************************************************************************************/
/* dot11WirelessMgmtOptions TABLE  - members of Dot11WirelessMgmtOptionsEntry         */
/**************************************************************************************/
/* Wireless Management attributes. In tabular form to allow for multiple      */
/* instances on an agent. This table only applies to the interface if         */
/* dot11WirelessManagementImplemented is set to true in the                   */
/* dot11StationConfigTable. Otherwise this table should be ignored.           */
/* For all Wireless Management features, an Activated MIB variable is used    */
/* to activate/enable or deactivate/disable the corresponding feature.        */
/* An Implemented MIB variable is used for an optional feature to indicate    */
/* whether the feature is implemented. A mandatory feature does not have a    */
/* corresponding Implemented MIB variable. It is possible for there to be     */
/* multiple IEEE 802.11 interfaces on one agent, each with its unique MAC     */
/* address. The relationship between an IEEE 802.11 interface and an          */
/* interface in the context of the Internet standard MIB is one-to-one.       */
/* As such, the value of an ifIndex object instance can be directly used      */
/* to identify corresponding instances of the objects defined herein.         */
/* ifIndex - Each IEEE 802.11 interface is represented by an ifEntry.         */
/* Interface tables in this MIB module are indexed by ifIndex.                */
typedef struct {
    oal_bool_enum_uint8 en_dot11MgmtOptionLocationActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionFMSImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionFMSActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionEventsActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionDiagnosticsActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionMultiBSSIDImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionMultiBSSIDActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionTFSImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionTFSActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionWNMSleepModeImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionWNMSleepModeActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionTIMBroadcastImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionTIMBroadcastActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionProxyARPImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionProxyARPActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionBSSTransitionImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionBSSTransitionActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionQoSTrafficCapabilityImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionQoSTrafficCapabilityActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionACStationCountImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionACStationCountActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionCoLocIntfReportingImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionCoLocIntfReportingActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionMotionDetectionImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionMotionDetectionActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionTODImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionTODActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionTimingMsmtImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionTimingMsmtActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionChannelUsageImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionChannelUsageActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionTriggerSTAStatisticsActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionSSIDListImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionSSIDListActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionMulticastDiagnosticsActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionLocationTrackingImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionLocationTrackingActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionDMSImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionDMSActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionUAPSDCoexistenceImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionUAPSDCoexistenceActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionWNMNotificationImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionWNMNotificationActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionUTCTSFOffsetImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionUTCTSFOffsetActivated;
    oal_bool_enum_uint8 en_dot11FtmInitiatorModeActivated;
    oal_bool_enum_uint8 en_dot11FtmResponderModeActivated;
    oal_bool_enum_uint8 en_dot11LciCivicInNeighborReport;
    oal_bool_enum_uint8 en_dot11RMFtmRangeReportImplemented;
    oal_bool_enum_uint8 en_dot11RMFtmRangeReportActivated;
    oal_bool_enum_uint8 en_dot11RMLCIConfigured;
    oal_bool_enum_uint8 en_dot11RMCivicConfigured;
} wlan_mib_Dot11WirelessMgmtOptionsEntry_stru;

/* dot11LocationServicesLocationIndicationChannelList OBJECT-TYPE         */
/* SYNTAX OCTET STRING (SIZE (2..254))                                    */
/* MAX-ACCESS read-create                                                 */
/* STATUS current                                                         */
/* DESCRIPTION                                                            */
/* "This attribute contains one or more Operating Class and Channel octet */
/* pairs."                                                                */
/* ::= { dot11LocationServicesEntry 12}                                   */
typedef struct {
    uint8_t uc_channel_nums;
    uint8_t *uc_channel_list;
} wlan_mib_local_serv_location_ind_ch_list_stru;

/********************************************************************************/
/* dot11LocationServices TABLE  - members of Dot11LocationServicesEntry         */
/********************************************************************************/
/* Identifies a hint for the next value of dot11LocationServicesIndex to be     */
/* used in a row creation attempt for dot11LocationServicesTable. If no new     */
/* rows can be created for some reason, such as memory, processing require-     */
/* ments, etc, the SME shall set this attribute to 0. It shall update this      */
/* attribute to a proper value other than 0 as soon as it is capable of         */
/* receiving new measurement requests. The nextIndex is not necessarily         */
/* sequential nor monotonically increasing.                                     */
typedef struct {
    uint32_t dot11LocationServicesIndex;
    uint8_t auc_dot11LocationServicesMACAddress[NUM_6_BYTES];
    uint8_t auc_dot11LocationServicesLIPIndicationMulticastAddress[NUM_6_BYTES];
    uint8_t en_dot11LocationServicesLIPReportIntervalUnits; /* wlan_mib_lct_servs_liprpt_interval_unit_enum */
    uint32_t dot11LocationServicesLIPNormalReportInterval;
    uint32_t dot11LocationServicesLIPNormalFramesperChannel;
    uint32_t dot11LocationServicesLIPInMotionReportInterval;
    uint32_t dot11LocationServicesLIPInMotionFramesperChannel;
    uint32_t dot11LocationServicesLIPBurstInterframeInterval;
    uint32_t dot11LocationServicesLIPTrackingDuration;
    uint32_t dot11LocationServicesLIPEssDetectionInterval;
    wlan_mib_local_serv_location_ind_ch_list_stru st_dot11LocationServicesLocationIndicationChannelList;
    uint32_t dot11LocationServicesLocationIndicationBroadcastDataRate;
    uint8_t uc_dot11LocationServicesLocationIndicationOptionsUsed;
    wlan_mib_local_serv_location_ind_ind_para_stru st_dot11LocationServicesLocationIndicationIndicationParameters;
    uint32_t dot11LocationServicesLocationStatus;
} wlan_mib_Dot11LocationServicesEntry_stru;

/********************************************************************************/
/* dot11WirelessMGTEvent TABLE  - members of Dot11WirelessMGTEventEntry         */
/********************************************************************************/
/* Group contains the current list of WIRELESS Management reports that have     */
/* been received by the MLME. The report tables shall be maintained as FIFO     */
/* to preserve freshness, thus the rows in this table can be deleted for mem-   */
/* ory constraints or other implementation constraints determined by the ven-   */
/* dor. New rows shall have different RprtIndex values than those deleted within */
/* the range limitation of the index. One easy way is to monotonically          */
/* increase the EventIndex for new reports being written in the table*          */
typedef struct {
    uint32_t dot11WirelessMGTEventIndex;
    uint8_t auc_dot11WirelessMGTEventMACAddress[NUM_6_BYTES];
    uint8_t en_dot11WirelessMGTEventType;    /* wlan_mib_wireless_mgt_event_type_enum */
    uint8_t en_dot11WirelessMGTEventStatus;  /* wlan_mib_wireless_mgt_event_status_enum */
    uint8_t auc_dot11WirelessMGTEventTSF[NUM_8_BYTES];
    uint8_t auc_dot11WirelessMGTEventUTCOffset[NUM_10_BYTES];
    uint8_t auc_dot11WirelessMGTEventTimeError[NUM_5_BYTES];
    uint8_t auc_dot11WirelessMGTEventTransitionSourceBSSID[NUM_6_BYTES];
    uint8_t auc_dot11WirelessMGTEventTransitionTargetBSSID[NUM_6_BYTES];
    uint32_t dot11WirelessMGTEventTransitionTime;
    uint8_t en_dot11WirelessMGTEventTransitionReason; /* wlan_mib_wireless_mgt_event_transit_reason_enum */
    uint32_t dot11WirelessMGTEventTransitionResult;
    uint32_t dot11WirelessMGTEventTransitionSourceRCPI;
    uint32_t dot11WirelessMGTEventTransitionSourceRSNI;
    uint32_t dot11WirelessMGTEventTransitionTargetRCPI;
    uint32_t dot11WirelessMGTEventTransitionTargetRSNI;
    uint8_t auc_dot11WirelessMGTEventRSNATargetBSSID[NUM_6_BYTES];
    uint8_t auc_dot11WirelessMGTEventRSNAAuthenticationType[NUM_4_BYTES];
    uint8_t auc_dot11WirelessMGTEventRSNAEAPMethod[NUM_8_BYTES];
    uint32_t dot11WirelessMGTEventRSNAResult;
    uint8_t auc_dot11WirelessMGTEventRSNARSNElement[257]; /* 数组大小257字节 */
    uint8_t uc_dot11WirelessMGTEventPeerSTAAddress;
    uint32_t dot11WirelessMGTEventPeerOperatingClass;
    uint32_t dot11WirelessMGTEventPeerChannelNumber;
    int32_t l_dot11WirelessMGTEventPeerSTATxPower;
    uint32_t dot11WirelessMGTEventPeerConnectionTime;
    uint32_t dot11WirelessMGTEventPeerPeerStatus;
    uint8_t auc_dot11WirelessMGTEventWNMLog[2284]; /* 数组大小2284字节 */
} wlan_mib_Dot11WirelessMGTEventEntry_stru;

typedef struct {
    /* dot11WNMRequest OBJECT IDENTIFIER ::= { dot11WirelessNetworkManagement 1 }   */
    /* dot11WNMRequestNextIndex Unsigned32(0..4294967295) ::= { dot11WNMRequest 1 } */
    uint32_t dot11WNMRequestNextIndex;

    /* dot11WNMRequestTable ::= { dot11WNMRequest 2 } */
    uint32_t dot11WNMRqstIndex;
    uint8_t en_dot11WNMRqstRowStatus;         /* wlan_mib_row_status_enum */
    uint8_t auc_dot11WNMRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMRqstIfIndex;
    uint8_t en_dot11WNMRqstType;              /* wlan_mib_wnm_rqst_type_enum */
    uint8_t auc_dot11WNMRqstTargetAdd[NUM_6_BYTES];
    uint32_t dot11WNMRqstTimeStamp;
    uint32_t dot11WNMRqstRndInterval;
    uint32_t dot11WNMRqstDuration;
    uint8_t auc_dot11WNMRqstMcstGroup[NUM_6_BYTES];
    uint8_t uc_dot11WNMRqstMcstTrigCon;
    uint32_t dot11WNMRqstMcstTrigInactivityTimeout;
    uint32_t dot11WNMRqstMcstTrigReactDelay;
    uint8_t en_dot11WNMRqstLCRRqstSubject;    /* wlan_mib_wnm_rqst_lcrrqst_subj_enum */
    uint8_t en_dot11WNMRqstLCRIntervalUnits;  /* wlan_mib_wnm_rqst_lcr_interval_unit_enum */
    uint32_t dot11WNMRqstLCRServiceInterval;
    uint8_t en_dot11WNMRqstLIRRqstSubject;    /* wlan_mib_wnm_rqst_lirrqst_subj_enum */
    uint8_t en_dot11WNMRqstLIRIntervalUnits;  /* wlan_mib_wnm_rqst_lir_interval_unit_enum */
    uint32_t dot11WNMRqstLIRServiceInterval;
    uint32_t dot11WNMRqstEventToken;
    uint8_t en_dot11WNMRqstEventType;         /* wlan_mib_wnm_rqst_event_type_enum */
    uint32_t dot11WNMRqstEventResponseLimit;
    uint8_t auc_dot11WNMRqstEventTargetBssid[NUM_6_BYTES];
    uint8_t auc_dot11WNMRqstEventSourceBssid[NUM_6_BYTES];
    uint32_t dot11WNMRqstEventTransitTimeThresh;
    uint8_t uc_dot11WNMRqstEventTransitMatchValue;
    uint32_t dot11WNMRqstEventFreqTransitCountThresh;
    uint32_t dot11WNMRqstEventFreqTransitInterval;
    uint8_t auc_dot11WNMRqstEventRsnaAuthType[NUM_4_BYTES];
    uint32_t dot11WNMRqstEapType;
    uint8_t auc_dot11WNMRqstEapVendorId[NUM_3_BYTES];
    uint8_t auc_dot11WNMRqstEapVendorType[NUM_4_BYTES];
    uint8_t uc_dot11WNMRqstEventRsnaMatchValue;
    uint8_t auc_dot11WNMRqstEventPeerMacAddress[NUM_6_BYTES];
    uint32_t dot11WNMRqstOperatingClass;
    uint32_t dot11WNMRqstChanNumber;
    uint32_t dot11WNMRqstDiagToken;
    uint8_t en_dot11WNMRqstDiagType;         /* wlan_mib_wnm_rqst_diag_type_enum */
    uint32_t dot11WNMRqstDiagTimeout;
    uint8_t auc_dot11WNMRqstDiagBssid[NUM_6_BYTES];
    uint32_t dot11WNMRqstDiagProfileId;
    uint8_t en_dot11WNMRqstDiagCredentials;  /* wlan_mib_wnm_rqst_diag_credent_enum */
    uint8_t auc_dot11WNMRqstLocConfigLocIndParams[NUM_16_BYTES];
    uint8_t auc_dot11WNMRqstLocConfigChanList[252]; /* 数组大小252字节 */
    uint32_t dot11WNMRqstLocConfigBcastRate;
    uint8_t auc_dot11WNMRqstLocConfigOptions[NUM_255_BYTES];
    uint8_t en_dot11WNMRqstBssTransitQueryReason;  /* wlan_mib_wnm_rqst_bss_transit_query_reason_enum */
    uint8_t uc_dot11WNMRqstBssTransitReqMode;
    uint32_t dot11WNMRqstBssTransitDisocTimer;

    /* This is a control variable.It is written by an external management entity when making a management
       request. Changes take effect when dot11WNMRqstRowStatus is set to Active.
       This attribute contains a variable-length field formatted in accordance with IETF RFC 3986-2005." */
    uint8_t auc_dot11WNMRqstBssTransitSessInfoURL[NUM_255_BYTES];
    uint8_t auc_dot11WNMRqstBssTransitCandidateList[2304]; /* 数组大小2304字节 */
    oal_bool_enum_uint8 en_dot11WNMRqstColocInterfAutoEnable;
    uint32_t dot11WNMRqstColocInterfRptTimeout;
    uint8_t auc_dot11WNMRqstVendorSpecific[NUM_255_BYTES];
    uint8_t auc_dot11WNMRqstDestinationURI[253]; /* 数组大小253字节 */

    /* dot11WNMReport OBJECT IDENTIFIER ::= { dot11WirelessNetworkManagement 2 } */
    /* dot11WNMVendorSpecificReportTable::= { dot11WNMReport 1 }                 */
    uint32_t dot11WNMVendorSpecificRprtIndex;
    uint8_t auc_dot11WNMVendorSpecificRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMVendorSpecificRprtIfIndex;
    uint8_t auc_dot11WNMVendorSpecificRprtContent[NUM_255_BYTES];
    /* dot11WNMMulticastDiagnosticReportTable ::= { dot11WNMReport 2 } */
    uint32_t dot11WNMMulticastDiagnosticRprtIndex;
    uint8_t auc_dot11WNMMulticastDiagnosticRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMMulticastDiagnosticRprtIfIndex;
    uint8_t auc_dot11WNMMulticastDiagnosticRprtMeasurementTime[NUM_8_BYTES];
    uint32_t dot11WNMMulticastDiagnosticRprtDuration;
    uint8_t auc_dot11WNMMulticastDiagnosticRprtMcstGroup[NUM_6_BYTES];
    uint8_t uc_dot11WNMMulticastDiagnosticRprtReason;
    uint32_t dot11WNMMulticastDiagnosticRprtRcvdMsduCount;
    uint32_t dot11WNMMulticastDiagnosticRprtFirstSeqNumber;
    uint32_t dot11WNMMulticastDiagnosticRprtLastSeqNumber;
    uint32_t dot11WNMMulticastDiagnosticRprtMcstRate;

    /* dot11WNMLocationCivicReportTable ::= { dot11WNMReport 3 } */
    uint32_t dot11WNMLocationCivicRprtIndex;
    uint8_t auc_dot11WNMLocationCivicRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMLocationCivicRprtIfIndex;

    /* This is a status variable.                                             */
    /* It is written by the SME when a management report is completed.        */
    /* This attribute indicates a variable octet field and contains a list of */
    /* civic address elements in TLV format as defined in IETF RFC 4776-2006  */
    /* dot11WNMLocationCivicRprtCivicLocation OCTET STRING */
    uint8_t auc_dot11WNMLocationCivicRprtCivicLocation[NUM_255_BYTES];

    /* dot11WNMLocationIdentifierReportTable ::= { dot11WNMReport 4 } */
    uint32_t dot11WNMLocationIdentifierRprtIndex;
    uint8_t auc_dot11WNMLocationIdentifierRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMLocationIdentifierRprtIfIndex;
    uint8_t auc_dot11WNMLocationIdentifierRprtExpirationTSF[NUM_8_BYTES];
    uint8_t uc_dot11WNMLocationIdentifierRprtPublicIdUri;

    /* dot11WNMEventTransitReportTable ::= { dot11WNMReport 5 } */
    uint32_t dot11WNMEventTransitRprtIndex;
    uint8_t auc_dot11WNMEventTransitRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMEventTransitRprtIfIndex;
    uint8_t en_dot11WNMEventTransitRprtEventStatus; /* wlan_mib_wnm_event_transit_rprt_event_status_enum */
    uint8_t auc_dot11WNMEventTransitRprtEventTSF[NUM_8_BYTES];
    uint8_t auc_dot11WNMEventTransitRprtUTCOffset[NUM_10_BYTES];
    uint8_t auc_dot11WNMEventTransitRprtTimeError[NUM_5_BYTES];
    uint8_t auc_dot11WNMEventTransitRprtSourceBssid[NUM_6_BYTES];
    uint8_t auc_dot11WNMEventTransitRprtTargetBssid[NUM_6_BYTES];
    uint32_t dot11WNMEventTransitRprtTransitTime;
    uint8_t en_dot11WNMEventTransitRprtTransitReason; /* wlan_mib_wnm_event_transitrprt_transit_reason_enum */
    uint32_t dot11WNMEventTransitRprtTransitResult;
    uint32_t dot11WNMEventTransitRprtSourceRCPI;
    uint32_t dot11WNMEventTransitRprtSourceRSNI;
    uint32_t dot11WNMEventTransitRprtTargetRCPI;
    uint32_t dot11WNMEventTransitRprtTargetRSNI;

    /* dot11WNMEventRsnaReportTable ::= { dot11WNMReport 6 } */
    uint32_t dot11WNMEventRsnaRprtIndex;
    uint8_t auc_dot11WNMEventRsnaRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMEventRsnaRprtIfIndex;
    uint8_t en_dot11WNMEventRsnaRprtEventStatus; /* wlan_mib_wnm_event_rsnarprt_event_status_enum */
    uint8_t auc_dot11WNMEventRsnaRprtEventTSF[NUM_8_BYTES];
    uint8_t auc_dot11WNMEventRsnaRprtUTCOffset[NUM_10_BYTES];
    uint8_t auc_dot11WNMEventRsnaRprtTimeError[NUM_5_BYTES];
    uint8_t auc_dot11WNMEventRsnaRprtTargetBssid[NUM_6_BYTES];
    uint8_t auc_dot11WNMEventRsnaRprtAuthType[NUM_4_BYTES];
    uint8_t auc_dot11WNMEventRsnaRprtEapMethod[NUM_8_BYTES];
    uint32_t dot11WNMEventRsnaRprtResult;
    uint8_t uc_dot11WNMEventRsnaRprtRsnElement;

    /* dot11WNMEventPeerReportTable ::= { dot11WNMReport 7 } */
    uint32_t dot11WNMEventPeerRprtIndex;
    uint8_t auc_dot11WNMEventPeerRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMEventPeerRprtIfIndex;
    uint8_t uc_dot11WNMEventPeerRprtEventStatus;
    uint8_t auc_dot11WNMEventPeerRprtEventTSF[NUM_8_BYTES];
    uint8_t auc_dot11WNMEventPeerRprtUTCOffset[NUM_10_BYTES];
    uint8_t auc_dot11WNMEventPeerRprtTimeError[NUM_5_BYTES];
    uint8_t auc_dot11WNMEventPeerRprtPeerMacAddress[NUM_6_BYTES];
    uint32_t dot11WNMEventPeerRprtOperatingClass;
    uint32_t dot11WNMEventPeerRprtChanNumber;
    int32_t l_dot11WNMEventPeerRprtStaTxPower;
    uint32_t dot11WNMEventPeerRprtConnTime;
    uint8_t uc_dot11WNMEventPeerRprtPeerStatus;

    /* dot11WNMEventWNMLogReportTable ::= { dot11WNMReport 8 } */
    uint32_t dot11WNMEventWNMLogRprtIndex;
    uint8_t auc_dot11WNMEventWNMLogRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMEventWNMLogRprtIfIndex;
    uint8_t uc_dot11WNMEventWNMLogRprtEventStatus;
    uint8_t auc_dot11WNMEventWNMLogRprtEventTSF[NUM_8_BYTES];
    uint8_t auc_dot11WNMEventWNMLogRprtUTCOffset[NUM_10_BYTES];
    uint8_t auc_dot11WNMEventWNMLogRprtTimeError[NUM_5_BYTES];
    uint8_t auc_dot11WNMEventWNMLogRprtContent[2284]; /* 数组大小2284字节 */
    /* dot11WNMDiagMfrInfoReportTable ::= { dot11WNMReport 9 } */
    uint32_t dot11WNMDiagMfrInfoRprtIndex;
    uint8_t auc_dot11WNMDiagMfrInfoRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMDiagMfrInfoRprtIfIndex;
    uint8_t uc_dot11WNMDiagMfrInfoRprtEventStatus;
    uint8_t auc_dot11WNMDiagMfrInfoRprtMfrOi[NUM_5_BYTES];
    uint8_t auc_dot11WNMDiagMfrInfoRprtMfrIdString[NUM_255_BYTES];
    uint8_t auc_dot11WNMDiagMfrInfoRprtMfrModelString[NUM_255_BYTES];
    uint8_t auc_dot11WNMDiagMfrInfoRprtMfrSerialNumberString[NUM_255_BYTES];
    uint8_t auc_dot11WNMDiagMfrInfoRprtMfrFirmwareVersion[NUM_255_BYTES];
    uint8_t auc_dot11WNMDiagMfrInfoRprtMfrAntennaType[NUM_255_BYTES];
    uint8_t uc_dot11WNMDiagMfrInfoRprtCollocRadioType;
    uint8_t uc_dot11WNMDiagMfrInfoRprtDeviceType;
    uint8_t auc_dot11WNMDiagMfrInfoRprtCertificateID[251]; /* 数组大小251字节 */

    /* dot11WNMDiagConfigProfReportTable ::= { dot11WNMReport 10 } */
    uint32_t dot11WNMDiagConfigProfRprtIndex;
    uint8_t auc_dot11WNMDiagConfigProfRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMDiagConfigProfRprtIfIndex;
    uint8_t uc_dot11WNMDiagConfigProfRprtEventStatus;
    uint32_t dot11WNMDiagConfigProfRprtProfileId;
    uint8_t auc_dot11WNMDiagConfigProfRprtSupportedOperatingClasses[NUM_255_BYTES];
    uint8_t uc_dot11WNMDiagConfigProfRprtTxPowerMode;
    uint8_t auc_dot11WNMDiagConfigProfRprtTxPowerLevels[NUM_255_BYTES];
    uint8_t auc_dot11WNMDiagConfigProfRprtCipherSuite[NUM_4_BYTES];
    uint8_t auc_dot11WNMDiagConfigProfRprtAkmSuite[NUM_4_BYTES];
    uint32_t dot11WNMDiagConfigProfRprtEapType;
    uint8_t auc_dot11WNMDiagConfigProfRprtEapVendorID[NUM_3_BYTES];
    uint8_t auc_dot11WNMDiagConfigProfRprtEapVendorType[NUM_4_BYTES];
    uint8_t uc_dot11WNMDiagConfigProfRprtCredentialType;
    uint8_t auc_dot11WNMDiagConfigProfRprtSSID[NUM_32_BYTES];
    uint8_t uc_dot11WNMDiagConfigProfRprtPowerSaveMode;

    /* dot11WNMDiagAssocReportTable ::= { dot11WNMReport 11 } */
    uint32_t dot11WNMDiagAssocRprtIndex;
    uint8_t auc_dot11WNMDiagAssocRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMDiagAssocRprtIfIndex;
    uint8_t uc_dot11WNMDiagAssocRprtEventStatus;
    uint8_t auc_dot11WNMDiagAssocRprtBssid[NUM_6_BYTES];
    uint32_t dot11WNMDiagAssocRprtOperatingClass;
    uint32_t dot11WNMDiagAssocRprtChannelNumber;
    uint32_t dot11WNMDiagAssocRprtStatusCode;

    /* dot11WNMDiag8021xAuthReportTable ::= { dot11WNMReport 12 } */
    uint32_t dot11WNMDiag8021xAuthRprtIndex;
    uint8_t auc_dot11WNMDiag8021xAuthRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMDiag8021xAuthRprtIfIndex;
    uint8_t uc_dot11WNMDiag8021xAuthRprtEventStatus;
    uint8_t auc_dot11WNMDiag8021xAuthRprtBssid[NUM_6_BYTES];
    uint32_t dot11WNMDiag8021xAuthRprtOperatingClass;
    uint32_t dot11WNMDiag8021xAuthRprtChannelNumber;
    uint32_t dot11WNMDiag8021xAuthRprtEapType;
    uint8_t auc_dot11WNMDiag8021xAuthRprtEapVendorID[NUM_3_BYTES];
    uint8_t auc_dot11WNMDiag8021xAuthRprtEapVendorType[NUM_4_BYTES];
    uint8_t uc_dot11WNMDiag8021xAuthRprtCredentialType;
    uint32_t dot11WNMDiag8021xAuthRprtStatusCode;

    /* dot11WNMLocConfigReportTable ::= { dot11WNMReport 13 } */
    uint32_t dot11WNMLocConfigRprtIndex;
    uint8_t auc_dot11WNMLocConfigRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMLocConfigRprtIfIndex;
    uint8_t auc_dot11WNMLocConfigRprtLocIndParams[NUM_16_BYTES];
    uint8_t auc_dot11WNMLocConfigRprtLocIndChanList[254]; /* 数组大小254字节 */
    uint32_t dot11WNMLocConfigRprtLocIndBcastRate;
    uint8_t auc_dot11WNMLocConfigRprtLocIndOptions[NUM_255_BYTES];
    uint8_t uc_dot11WNMLocConfigRprtStatusConfigSubelemId;
    uint8_t uc_dot11WNMLocConfigRprtStatusResult;
    uint8_t auc_dot11WNMLocConfigRprtVendorSpecificRprtContent[NUM_255_BYTES];

    /* dot11WNMBssTransitReportTable ::= { dot11WNMReport 14 } */
    uint32_t dot11WNMBssTransitRprtIndex;
    uint8_t auc_dot11WNMBssTransitRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMBssTransitRprtIfIndex;
    uint8_t uc_dot11WNMBssTransitRprtStatusCode;
    uint32_t dot11WNMBssTransitRprtBSSTerminationDelay;
    uint8_t auc_dot11WNMBssTransitRprtTargetBssid[NUM_6_BYTES];
    uint8_t auc_dot11WNMBssTransitRprtCandidateList[2304]; /* 数组大小2304字节 */

    /* 备注: 没有找到 dot11WNMReport 15 */
    /* dot11WNMColocInterfReportTable ::= { dot11WNMReport 16 } */
    uint32_t dot11WNMColocInterfRprtIndex;
    uint8_t auc_dot11WNMColocInterfRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMColocInterfRprtIfIndex;
    uint32_t dot11WNMColocInterfRprtPeriod;
    int32_t l_dot11WNMColocInterfRprtInterfLevel;
    uint32_t dot11WNMColocInterfRprtInterfAccuracy;
    uint32_t dot11WNMColocInterfRprtInterfIndex;
    int32_t l_dot11WNMColocInterfRprtInterfInterval;
    int32_t l_dot11WNMColocInterfRprtInterfBurstLength;
    int32_t l_dot11WNMColocInterfRprtInterfStartTime;
    int32_t l_dot11WNMColocInterfRprtInterfCenterFreq;
    uint32_t dot11WNMColocInterfRprtInterfBandwidth;
} wlan_mib_dot11WirelessNetworkManagement_stru;

/****************************************************************************/
/* dot11MeshSTAConfig TABLE  - members of Dot11MeshSTAConfigEntry           */
/****************************************************************************/
/* Mesh Station Configuration attributes. In tabular form to allow for mul- */
/* tiple instances on an agent.                                             */
typedef struct {
    uint8_t auc_dot11MeshID[NUM_32_BYTES];                  /* dot11MeshID OCTET STRING,   (SIZE(0..32))       */
    uint32_t dot11MeshNumberOfPeerings;                     /* dot11MeshNumberOfPeerings Unsigned32,           */
    oal_bool_enum_uint8 en_dot11MeshAcceptingAdditionalPeerings; /* dot11MeshAcceptingAdditionalPeerings TruthValue, */
    oal_bool_enum_uint8 en_dot11MeshConnectedToMeshGate;         /* dot11MeshConnectedToMeshGate TruthValue,        */
    oal_bool_enum_uint8 en_dot11MeshSecurityActivated;           /* dot11MeshSecurityActivated TruthValue,          */
    uint8_t uc_dot11MeshActiveAuthenticationProtocol;          /* dot11MeshActiveAuthenticationProtocol INTEGER,  */
    uint32_t dot11MeshMaxRetries;                           /* dot11MeshMaxRetries Unsigned32,                 */
    uint32_t dot11MeshRetryTimeout;                         /* dot11MeshRetryTimeout Unsigned32,               */
    uint32_t dot11MeshConfirmTimeout;                       /* dot11MeshConfirmTimeout Unsigned32,             */
    uint32_t dot11MeshHoldingTimeout;                       /* dot11MeshHoldingTimeout Unsigned32,             */
    uint32_t dot11MeshConfigGroupUpdateCount;               /* dot11MeshConfigGroupUpdateCount Unsigned32,     */
    uint8_t uc_dot11MeshActivePathSelectionProtocol;           /* dot11MeshActivePathSelectionProtocol INTEGER,   */
    uint8_t uc_dot11MeshActivePathSelectionMetric;             /* dot11MeshActivePathSelectionMetric INTEGER,     */
    oal_bool_enum_uint8 en_dot11MeshForwarding;                  /* dot11MeshForwarding TruthValue,                 */
    uint32_t dot11MeshTTL;                                  /* dot11MeshTTL Unsigned32,                        */
    oal_bool_enum_uint8 en_dot11MeshGateAnnouncements;           /* dot11MeshGateAnnouncements TruthValue,          */
    uint32_t dot11MeshGateAnnouncementInterval;             /* dot11MeshGateAnnouncementInterval Unsigned32,   */
    uint8_t uc_dot11MeshActiveCongestionControlMode;           /* dot11MeshActiveCongestionControlMode INTEGER,   */
    uint8_t uc_dot11MeshActiveSynchronizationMethod;           /* dot11MeshActiveSynchronizationMethod INTEGER,   */
    uint32_t dot11MeshNbrOffsetMaxNeighbor;                 /* dot11MeshNbrOffsetMaxNeighbor Unsigned32,       */
    oal_bool_enum_uint8 en_dot11MBCAActivated;                   /* dot11MBCAActivated TruthValue,                  */
    uint32_t dot11MeshBeaconTimingReportInterval;           /* dot11MeshBeaconTimingReportInterval Unsigned32, */
    uint32_t dot11MeshBeaconTimingReportMaxNum;             /* dot11MeshBeaconTimingReportMaxNum Unsigned32,   */
    uint32_t dot11MeshDelayedBeaconTxInterval;              /* dot11MeshDelayedBeaconTxInterval Unsigned32,    */
    uint32_t dot11MeshDelayedBeaconTxMaxDelay;              /* dot11MeshDelayedBeaconTxMaxDelay Unsigned32,    */
    uint32_t dot11MeshDelayedBeaconTxMinDelay;              /* dot11MeshDelayedBeaconTxMinDelay Unsigned32,    */
    uint32_t dot11MeshAverageBeaconFrameDuration;           /* dot11MeshAverageBeaconFrameDuration Unsigned32, */
    uint32_t dot11MeshSTAMissingAckRetryLimit;              /* dot11MeshSTAMissingAckRetryLimit Unsigned32,    */
    uint32_t dot11MeshAwakeWindowDuration;                  /* dot11MeshAwakeWindowDuration Unsigned32,        */
    oal_bool_enum_uint8 en_dot11MCCAImplemented;                 /* dot11MCCAImplemented TruthValue,                */
    oal_bool_enum_uint8 en_dot11MCCAActivated;                   /* dot11MCCAActivated TruthValue,                  */
    uint32_t dot11MAFlimit;                                 /* dot11MAFlimit Unsigned32,                       */
    uint32_t dot11MCCAScanDuration;                         /* dot11MCCAScanDuration Unsigned32,               */
    uint32_t dot11MCCAAdvertPeriodMax;                      /* dot11MCCAAdvertPeriodMax Unsigned32,            */
    uint32_t dot11MCCAMinTrackStates;                       /* dot11MCCAMinTrackStates Unsigned32,             */
    uint32_t dot11MCCAMaxTrackStates;                       /* dot11MCCAMaxTrackStates Unsigned32,             */
    uint32_t dot11MCCAOPtimeout;                            /* dot11MCCAOPtimeout Unsigned32,                  */
    uint32_t dot11MCCACWmin;                                /* dot11MCCACWmin Unsigned32,                      */
    uint32_t dot11MCCACWmax;                                /* dot11MCCACWmax Unsigned32,                      */
    uint32_t dot11MCCAAIFSN;                                /* dot11MCCAAIFSN Unsigned32                       */
} wlan_mib_Dot11MeshSTAConfigEntry_stru;

/*****************************************************************************/
/* dot11MeshHWMPConfig TABLE  - members of Dot11MeshHWMPConfigEntry          */
/*****************************************************************************/
/* MMesh Station HWMP Configuration attributes. In tabular form to allow for */
/* tmultiple instances on an agent.                                          */
typedef struct {
    uint32_t dot11MeshHWMPmaxPREQretries;           /* dot11MeshHWMPmaxPREQretries Unsigned32,           */
    uint32_t dot11MeshHWMPnetDiameter;              /* dot11MeshHWMPnetDiameter Unsigned32,              */
    uint32_t dot11MeshHWMPnetDiameterTraversalTime; /* dot11MeshHWMPnetDiameterTraversalTime Unsigned32,, */
    uint32_t dot11MeshHWMPpreqMinInterval;          /* dot11MeshHWMPpreqMinInterval Unsigned32,          */
    uint32_t dot11MeshHWMPperrMinInterval;          /* dot11MeshHWMPperrMinInterval Unsigned32,          */
    uint32_t dot11MeshHWMPactivePathToRootTimeout;  /* dot11MeshHWMPactivePathToRootTimeout Unsigned32,  */
    uint32_t dot11MeshHWMPactivePathTimeout;        /* dot11MeshHWMPactivePathTimeout Unsigned32,        */
    uint8_t uc_dot11MeshHWMProotMode;                  /* dot11MeshHWMProotMode INTEGER,                    */
    uint32_t dot11MeshHWMProotInterval;             /* dot11MeshHWMProotInterval Unsigned32,             */
    uint32_t dot11MeshHWMPrannInterval;             /* dot11MeshHWMPrannInterval Unsigned32,             */
    uint8_t uc_dot11MeshHWMPtargetOnly;                /* dot11MeshHWMPtargetOnly INTEGER,                  */
    uint32_t dot11MeshHWMPmaintenanceInterval;      /* dot11MeshHWMPmaintenanceInterval Unsigned32,      */
    uint32_t dot11MeshHWMPconfirmationInterval;     /* dot11MeshHWMPconfirmationInterval Unsigned32      */
} wlan_mib_Dot11MeshHWMPConfigEntry_stru;

/**************************************************************************************/
/* dot11RSNAConfigPasswordValue TABLE  - members of Dot11RSNAConfigPasswordValueEntry */
/**************************************************************************************/
/* When SAE authentication is the selected AKM suite,     */
/* this table is used to locate the binary representation */
/* of a shared, secret, and potentially low-entropy word, */
/* phrase, code, or key that will be used as the          */
/* authentication credential between a TA/RA pair.        */
/* This table is logically write-only. Reading this table */
/* returns unsuccessful status or null or zero."          */
typedef struct {
    uint32_t dot11RSNAConfigPasswordValueIndex; /* dot11RSNAConfigPasswordValueIndex Unsigned32,   */
    uint8_t uc_dot11RSNAConfigPasswordCredential;  /* dot11RSNAConfigPasswordCredential OCTET STRING, */
    uint8_t auc_dot11RSNAConfigPasswordPeerMac[NUM_6_BYTES]; /* dot11RSNAConfigPasswordPeerMac MacAddress */
} wlan_mib_Dot11RSNAConfigPasswordValueEntry_stru;

/****************************************************************************/
/* dot11RSNAConfigDLCGroup TABLE  - members of Dot11RSNAConfigDLCGroupEntry */
/****************************************************************************/
/* This table gives a prioritized list of domain parameter set   */
/* Identifiers for discrete logarithm cryptography (DLC) groups. */
typedef struct {
    uint32_t dot11RSNAConfigDLCGroupIndex;      /* dot11RSNAConfigDLCGroupIndex Unsigned32,      */
    uint32_t dot11RSNAConfigDLCGroupIdentifier; /* dot11RSNAConfigDLCGroupIdentifier Unsigned32  */
} wlan_mib_Dot11RSNAConfigDLCGroupEntry_stru;

/****************************************************************************/
/* dot11VHTStationConfig TABLE  - members of Dot11VHTStationConfigEntry */
/****************************************************************************/
/* Station Configuration attributes. In tabular form to allow for multiple   */
/* instances on an agent. */
typedef struct {
    uint32_t dot11MaxMPDULength;                 /* dot11MaxMPDULength INTEGER, */
    uint32_t dot11VHTMaxRxAMPDUFactor;           /* dot11VHTMaxRxAMPDUFactor Unsigned32, */
    uint32_t dot11VHTRxHighestDataRateSupported; /* dot11VHTRxHighestDataRateSupported Unsigned32, */
    uint32_t dot11VHTTxHighestDataRateSupported; /* dot11VHTTxHighestDataRateSupported Unsigned32, */
    uint16_t us_dot11VHTTxMCSMap;                   /* dot11VHTTxMCSMap OCTET STRING, */
    uint16_t us_dot11VHTRxMCSMap;                   /* dot11VHTRxMCSMap OCTET STRING, */
    // uint32_t          dot11VHTOBSSScanCount;                      /* dot11VHTOBSSScanCount Unsigned32 */
    oal_bool_enum_uint8 en_dot11VHTControlFieldSupported;             /* dot11VHTControlFieldSupported TruthValue */
    oal_bool_enum_uint8 en_dot11VHTTXOPPowerSaveOptionImplemented;    /* dot11VHTTXOPPowerSaveOptionImplemented */
    oal_bool_enum_uint8 en_dot11VHTOptionImplemented;                 /* dot11VHTOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11OperatingModeNotificationImplemented; /* dot11OperatingModeNotificationImplemented */
} wlan_mib_Dot11VHTStationConfigEntry_stru;

#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
typedef struct {
    uint32_t dot11HERxHighestDataRateSupported; /* dot11HERxHighestDataRateSupported Unsigned32, */
    uint32_t dot11HETxHighestDataRateSupported; /* dot11HETxHighestDataRateSupported Unsigned32, */
    uint32_t dot11HETxMCSMap;                   /* us_dot11HETxMCSMap OCTET STRING, */
    uint32_t dot11HERxMCSMap;                   /* us_dot11HERxMCSMap OCTET STRING, */

    uint8_t uc_dot11TriggerMacPaddingDuration;
    uint8_t uc_dot11MaxAMPDULengthExponent;

    oal_bool_enum_uint8 en_dot11HEOptionImplemented;  /* dot11HEOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11OMIOptionImplemented; /* en_dot11OMIOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11TWTOptionActivated;   /* dot11TWTOptionActivated TruthValue */
    oal_bool_enum_uint8 en_dot11OperatingModeIndication;
    oal_bool_enum_uint8 en_dot11TOFDMARandomAccess;
    oal_bool_enum_uint8 en_dot11MultiBSSIDImplemented;
    oal_bool_enum_uint8 en_dot11BSRSupport;
    oal_bool_enum_uint8 en_dot11BQRSupport; /* bandwidth query report support */
} wlan_mib_Dot11HEStationConfigEntry_stru;

typedef struct {
    oal_bool_enum_uint8 en_dot11HEDualBandSupport;
    uint8_t uc_dot11HEChannelWidthSet;
    uint8_t uc_dot11HighestNSS;
    uint8_t uc_dot11HighestMCS;
    oal_bool_enum_uint8 en_dot11HELDPCCodingInPayload;
    oal_bool_enum_uint8 en_dot11SUBeamformer;
    oal_bool_enum_uint8 en_dot11SUBeamformee;
    oal_bool_enum_uint8 en_dot11MUBeamformer;
    oal_bool_enum_uint8 en_dot11SUBeamformingFeedback;
    oal_bool_enum_uint8 en_dot11MUBeamformingFeedback;
    oal_bool_enum_uint8 en_dot11HESTBCTxOver80MHz;
    oal_bool_enum_uint8 en_dot11HESTBCRxOver80MHz;
    oal_bool_enum_uint8 en_dot11SRPBasedSR;
    oal_bool_enum_uint8 en_dot11HESTBCTxBelow80MHz;
    oal_bool_enum_uint8 en_dot11HESTBCRxBelow80MHz;
} wlan_mid_Dot11HePhyCapability_stru;

typedef struct {
    oal_bool_enum_uint8 en_dot11PPEThresholdsRequired;
    uint8_t uc_dot11HEBeamformeeSTSBelow80Mhz;
    uint8_t uc_dot11HEBeamformeeSTSOver80Mhz;
    uint8_t uc_dot11HENumberSoundingDimensionsBelow80Mhz;
    uint8_t uc_dot11HENumberSoundingDimensionsOver80Mhz;
    oal_bool_enum_uint8 en_dot11Ng16SUFeedback;
    oal_bool_enum_uint8 en_dot11Ng16MUFeedback;
    oal_bool_enum_uint8 en_dot11Codebook42SUFeedback;
    oal_bool_enum_uint8 en_dot11Codebook75MUFeedback;
    oal_bool_enum_uint8 en_dot11TriggeredSUBeamformingFeedback;
    oal_bool_enum_uint8 en_dot11TriggeredMUBeamformingPartialBWFeedback;
    oal_bool_enum_uint8 en_dot11TriggeredCQIFeedback;
    oal_bool_enum_uint8 en_dot11HESUPPDUwith1xHELTFand0point8GIlmplemented;
    uint8_t auc_rsv[NUM_2_BYTES]; /* 2字节reserve */
} wlan_mid_Dot11HePhyCapability_rom_stru;

typedef struct {
    oal_bool_enum_uint8 en_dot11MultiBSSIDActived;
    uint8_t auc_rsv[NUM_3_BYTES];
} wlan_mib_Dot11HEStationConfigEntry_rom_stru;
#endif

/****************************************************************************************/
/*          Start of dot11mac OBJECT IDENTIFIER ::= { ieee802dot11 2 }                  */
/*              --  MAC GROUPS                                                          */
/*              --  dot11OperationTable ::= { dot11mac 1 }                              */
/*              --  dot11CountersTable ::= { dot11mac 2 }                               */
/*              --  dot11GroupAddressesTable ::= { dot11mac 3 }                         */
/*              --  dot11EDCATable ::= { dot11mac 4 }                                   */
/*              --  dot11QAPEDCATable ::= { dot11mac 5 }                                */
/*              --  dot11QosCountersTable ::= { dot11mac 6 }                            */
/*              --  dot11ResourceInfoTable    ::= { dot11mac 7 }                        */
/****************************************************************************************/
/****************************************************************************************/
/* dot11OperationTable OBJECT-TYPE                                                      */
/* SYNTAX SEQUENCE OF Dot11OperationEntry                                               */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Group contains MAC attributes pertaining to the operation of the MAC.          */
/*      This has been implemented as a table in order to allow for multiple             */
/*      instantiations on an agent."                                                    */
/* ::= { dot11mac 1 }                                                                   */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 en_dot11RIFSMode : 1;                            /* dot11RIFSMode TruthValue */
    oal_bool_enum_uint8 en_dot11DualCTSProtection : 1;                   /* dot11DualCTSProtection TruthValue */
    oal_bool_enum_uint8 en_dot11LSIGTXOPFullProtectionActivated : 1;     /* dot11LSIGTXOPFullProtectionActivated */
    oal_bool_enum_uint8 en_dot11NonGFEntitiesPresent : 1;                /* dot11NonGFEntitiesPresent TruthValue */
    oal_bool_enum_uint8 en_dot11PCOActivated : 1;                        /* dot11PCOActivated TruthValue */
    oal_bool_enum_uint8 en_dot11FortyMHzIntolerant : 1;                  /* dot11FortyMHzIntolerant TruthValue */
    oal_bool_enum_uint8 en_dot112040BSSCoexistenceManagementSupport : 1; /* dot112040BSSCoexistenceManagementSupport */
    oal_bool_enum_uint8 en_resv : 1;

    uint8_t en_dot11HTProtection; /* wlan_mib_ht_protection_enum: dot11HTProtection INTEGER */
    uint8_t resv[NUM_2_BYTES];
    uint32_t dot11RTSThreshold;                         /* dot11RTSThreshold Unsigned32 */
    uint32_t dot11FragmentationThreshold;               /* dot11FragmentationThreshold Unsigned32 */
    uint32_t dot11BSSWidthTriggerScanInterval;          /* dot11BSSWidthTriggerScanInterval Unsigned32 */
    uint32_t dot11BSSWidthChannelTransitionDelayFactor; /* dot11BSSWidthChannelTransitionDelayFactor Unsigned32 */
    uint32_t dot11OBSSScanPassiveDwell;                 /* dot11OBSSScanPassiveDwell Unsigned32 */
    uint32_t dot11OBSSScanActiveDwell;                  /* dot11OBSSScanActiveDwell Unsigned32 */
    uint32_t dot11OBSSScanPassiveTotalPerChannel;       /* dot11OBSSScanPassiveTotalPerChannel Unsigned32 */
    uint32_t dot11OBSSScanActiveTotalPerChannel;        /* dot11OBSSScanActiveTotalPerChannel Unsigned32 */
    uint32_t dot11OBSSScanActivityThreshold;            /* dot11OBSSScanActivityThreshold Unsigned32 */
} wlan_mib_Dot11OperationEntry_stru;

/****************************************************************************************/
/* dot11GroupAddressesTable OBJECT-TYPE                                                 */
/* SYNTAX SEQUENCE OF Dot11GroupAddressesEntry                                          */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "A conceptual table containing a set of MAC addresses identifying the mul-      */
/*      ticast-group addresses for which this STA receives frames. The default          */
/*      value of this attribute is null."                                               */
/* ::= { dot11mac 3 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11GroupAddressesIndex;                      /* dot11GroupAddressesIndex InterfaceIndex */
    uint8_t auc_dot11Address[NUM_6_BYTES];                               /* dot11Address MacAddress */
    uint8_t en_dot11GroupAddressesStatus; /* wlan_mib_row_status_enum: dot11GroupAddressesStatus RowStatus */
} wlan_mib_Dot11GroupAddressesEntry_stru;

/****************************************************************************************/
/* dot11EDCATable OBJECT-TYPE                                                           */
/* SYNTAX SEQUENCE OF Dot11EDCAEntry                                                    */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Conceptual table for EDCA default parameter values at a non-AP STA. This       */
/*      table contains the four entries of the EDCA parameters corresponding to         */
/*      four possible ACs. Index 1 corresponds to AC_BK, index 2 to AC_BE, index 3      */
/*      to AC_VI, and index 4 to AC_VO."                                                */
/*      REFERENCE                                                                       */
/*      "IEEE 802.11-<year>, 9.2.4.2"                                                   */
/* ::= { dot11mac 4 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11EDCATableIndex;     /* dot11EDCATableIndex     Unsigned32 */
    uint32_t dot11EDCATableCWmin;     /* dot11EDCATableCWmin Unsigned32 */
    uint32_t dot11EDCATableCWmax;     /* dot11EDCATableCWmax Unsigned32 */
    uint32_t dot11EDCATableAIFSN;     /* dot11EDCATableAIFSN Unsigned32 */
    uint32_t dot11EDCATableTXOPLimit; /* dot11EDCATableTXOPLimit Unsigned32 */
    // uint32_t  dot11EDCATableMSDULifetime;          /* dot11EDCATableMSDULifetime Unsigned32 */
    oal_bool_enum_uint8 en_dot11EDCATableMandatory; /* dot11EDCATableMandatory TruthValue */
} wlan_mib_Dot11CFGEDCAEntry_stru;

typedef struct {
    wlan_mib_Dot11CFGEDCAEntry_stru ast_wlan_mib_edca[WLAN_WME_AC_BUTT];
    /* --  dot11QAPEDCATable ::= { dot11mac 5 } */
    wlan_mib_Dot11QAPEDCAEntry_stru ast_wlan_mib_qap_edac[WLAN_WME_AC_BUTT];
} wlan_mib_Dot11EDCAEntry_stru;

/****************************************************************************************/
/* dot11QosCountersTable OBJECT-TYPE                                                    */
/* SYNTAX SEQUENCE OF Dot11QosCountersEntry                                             */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Group containing attributes that are MAC counters implemented as a table       */
/*      to allow for multiple instantiations on an agent."                              */
/* ::= { dot11mac 6 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11QosCountersIndex;            /* dot11QosCountersIndex Unsigned32 */
    uint32_t dot11QosTransmittedFragmentCount; /* dot11QosTransmittedFragmentCount Counter32 */
    uint32_t dot11QosFailedCount;              /* dot11QosFailedCount Counter32 */
    uint32_t dot11QosRetryCount;               /* dot11QosRetryCount Counter32 */
    uint32_t dot11QosMultipleRetryCount;       /* dot11QosMultipleRetryCount Counter32 */
    uint32_t dot11QosFrameDuplicateCount;      /* dot11QosFrameDuplicateCount Counter32 */
    uint32_t dot11QosRTSSuccessCount;          /* dot11QosRTSSuccessCount Counter32 */
    uint32_t dot11QosRTSFailureCount;          /* dot11QosRTSFailureCount Counter32 */
    uint32_t dot11QosACKFailureCount;          /* dot11QosACKFailureCount Counter32 */
    uint32_t dot11QosReceivedFragmentCount;    /* dot11QosReceivedFragmentCount Counter32 */
    uint32_t dot11QosTransmittedFrameCount;    /* dot11QosTransmittedFrameCount Counter32 */
    uint32_t dot11QosDiscardedFrameCount;      /* dot11QosDiscardedFrameCount Counter32 */
    uint32_t dot11QosMPDUsReceivedCount;       /* dot11QosMPDUsReceivedCount Counter32 */
    uint32_t dot11QosRetriesReceivedCount;     /* dot11QosRetriesReceivedCount Counter32 */
} wlan_mib_Dot11QosCountersEntry_stru;

/****************************************************************************************/
/* dot11ResourceInfoTable OBJECT-TYPE                                                   */
/* SYNTAX SEQUENCE OF Dot11ResourceInfoEntry                                            */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Provides a means of indicating, in data readable from a managed object,        */
/*      information that identifies the source of the implementation."                  */
/*      REFERENCE "IEEE Std 802.1F-1993, A.7. Note that this standard has been with-    */
/*      drawn."                                                                         */
/* ::= { dot11mac 7 }                                                                   */
/* ::= { dot11resAttribute 2 }                                                          */
/****************************************************************************************/
typedef struct {
    uint8_t auc_dot11manufacturerOUI[NUM_3_BYTES];              /* dot11manufacturerOUI OCTET STRING */
    uint8_t auc_dot11manufacturerName[128];           /* dot11manufacturerName DisplayString SIZE(0..128) */
    uint8_t auc_dot11manufacturerProductName[128];    /* dot11manufacturerProductName DisplayString SIZE(0..128) */
    uint8_t auc_dot11manufacturerProductVersion[128]; /* dot11manufacturerProductVersion DisplayString SIZE(0..128) */
} wlan_mib_Dot11ResourceInfoEntry_stru;

/****************************************************************************************/
/*          Start of dot11res    OBJECT IDENTIFIER ::= { ieee802dot11 3 }               */
/*              dot11resAttribute OBJECT IDENTIFIER ::= { dot11res 1 }                  */
/****************************************************************************************/
typedef struct {
    uint8_t auc_dot11ResourceTypeIDName[4]; /* dot11ResourceTypeIDName  DisplayString (SIZE(4)) */
    wlan_mib_Dot11ResourceInfoEntry_stru st_resource_info;
} wlan_mib_dot11resAttribute_stru;

/****************************************************************************************/
/*          Start of dot11phy OBJECT IDENTIFIER ::= { ieee802dot11 4 }                  */
/*              --  PHY GROUPS                                                          */
/*              --  dot11PhyOperationTable ::= { dot11phy 1 }                           */
/*              --  dot11PhyAntennaTable ::= { dot11phy 2 }                             */
/*              --  dot11PhyTxPowerTable ::= { dot11phy 3 }                             */
/*              --  dot11PhyFHSSTable ::= { dot11phy 4 }                                */
/*              --  dot11PhyDSSSTable ::= { dot11phy 5 }                                */
/*              --  dot11PhyIRTable ::= { dot11phy 6 }                                  */
/*              --  dot11RegDomainsSupportedTable ::= { dot11phy 7 }                    */
/*              --  dot11AntennasListTable ::= { dot11phy 8 }                           */
/*              --  dot11SupportedDataRatesTxTable ::= { dot11phy 9 }                   */
/*              --  dot11SupportedDataRatesRxTable ::= { dot11phy 10 }                  */
/*              --  dot11PhyOFDMTable ::= { dot11phy 11 }                               */
/*              --  dot11PhyHRDSSSTable ::= { dot11phy 12 }                             */
/*              --  dot11HoppingPatternTable ::= { dot11phy 13 }                        */
/*              --  dot11PhyERPTable ::= { dot11phy 14 }                                */
/*              --  dot11PhyHTTable  ::= { dot11phy 15 }                                */
/*              --  dot11SupportedMCSTxTable ::= { dot11phy 16 }                        */
/*              --  dot11SupportedMCSRxTable ::= { dot11phy 17 }                        */
/*              --  dot11TransmitBeamformingConfigTable ::= { dot11phy 18 }             */
/*              -- dot11PhyVHTTable ::= { dot11phy 23 } (802.11 ac)                     */
/*              -- dot11VHTTransmitBeamformingConfigTable ::= { dot11phy 24 }(802.11 ac) */
/****************************************************************************************/
/****************************************************************************************/
/* dot11PhyOperationTable OBJECT-TYPE                                                   */
/* SYNTAX SEQUENCE OF Dot11PhyOperationEntry                                            */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "PHY level attributes concerned with operation. Implemented as a table          */
/*      indexed on ifIndex to allow for multiple instantiations on an Agent."           */
/* ::= { dot11phy 1 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint8_t uc_dot11PHYType;           /* dot11PHYType INTEGER */
    uint32_t dot11CurrentRegDomain; /* dot11CurrentRegDomain Unsigned32 */
    uint8_t uc_dot11TempType;          /* dot11TempType INTEGER */
} wlan_mib_Dot11PhyOperationEntry_stru;

/****************************************************************************************/
/* dot11PhyAntennaTable OBJECT-TYPE                                                     */
/* SYNTAX SEQUENCE OF Dot11PhyAntennaEntry                                              */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Group of attributes for PhyAntenna. Implemented as a table indexed on          */
/*      ifIndex to allow for multiple instances on an agent."                           */
/* ::= { dot11phy 2}                                                                    */
/****************************************************************************************/
typedef struct {
    // uint32_t          dot11CurrentTxAntenna; /* dot11CurrentTxAntenna Unsigned32 */
    // uint8_t           uc_dot11DiversitySupportImplemented;
    // uint32_t          dot11CurrentRxAntenna;
    oal_bool_enum_uint8 en_dot11AntennaSelectionOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11TransmitExplicitCSIFeedbackASOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11TransmitIndicesFeedbackASOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11ExplicitCSIFeedbackASOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11ReceiveAntennaSelectionOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11TransmitSoundingPPDUOptionImplemented : 1;
    oal_bool_enum_uint8 en_resv : 2;

    oal_bool_enum_uint8 resv[NUM_3_BYTES];
    // oal_bool_enum_uint8 en_dot11TransmitIndicesComputationASOptionImplemented;
    // uint32_t          dot11NumberOfActiveRxAntennas;
} wlan_mib_Dot11PhyAntennaEntry_stru;

/****************************************************************************************/
/* dot11PhyTxPowerTable OBJECT-TYPE                                                     */
/* SYNTAX SEQUENCE OF Dot11PhyTxPowerEntry                                              */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Group of attributes for dot11PhyTxPowerTable. Implemented as a table           */
/*      indexed on STA ID to allow for multiple instances on an Agent."                 */
/* ::= { dot11phy 3}                                                                    */
/****************************************************************************************/
typedef struct {
    uint32_t dot11NumberSupportedPowerLevelsImplemented; /* dot11NumberSupportedPowerLevelsImplemented */
    uint32_t dot11TxPowerLevel1;                         /* dot11TxPowerLevel1 Unsigned32 */
    uint32_t dot11TxPowerLevel2;                         /* dot11TxPowerLevel2 Unsigned32 */
    uint32_t dot11TxPowerLevel3;                         /* dot11TxPowerLevel3 Unsigned32 */
    uint32_t dot11TxPowerLevel4;                         /* dot11TxPowerLevel4 Unsigned32 */
    uint32_t dot11TxPowerLevel5;                         /* dot11TxPowerLevel5 Unsigned32 */
    uint32_t dot11TxPowerLevel6;                         /* dot11TxPowerLevel6 Unsigned32 */
    uint32_t dot11TxPowerLevel7;                         /* dot11TxPowerLevel7 Unsigned32 */
    uint32_t dot11TxPowerLevel8;                         /* dot11TxPowerLevel8 Unsigned32 */
    uint32_t dot11CurrentTxPowerLevel;                   /* dot11CurrentTxPowerLevel Unsigned32 */
    uint8_t auc_dot11TxPowerLevelExtended;                  /* dot11TxPowerLevelExtended OCTET STRING */
    uint32_t dot11CurrentTxPowerLevelExtended;           /* dot11CurrentTxPowerLevelExtended Unsigned32 */
} wlan_mib_Dot11PhyTxPowerEntry_stru;

/****************************************************************************************/
/* dot11PhyFHSSTable OBJECT-TYPE                                                        */
/* SYNTAX SEQUENCE OF Dot11PhyFHSSEntry                                                 */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Group of attributes for dot11PhyFHSSTable. Implemented as a table indexed      */
/*      on STA ID to allow for multiple instances on an Agent."                         */
/* ::= { dot11phy 4 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11HopTime;                            /* dot11HopTime Unsigned32 */
    uint32_t dot11CurrentChannelNumber;               /* dot11CurrentChannelNumber Unsigned32 */
    uint32_t dot11MaxDwellTime;                       /* dot11MaxDwellTime Unsigned32 */
    uint32_t dot11CurrentDwellTime;                   /* dot11CurrentDwellTime Unsigned32 */
    uint32_t dot11CurrentSet;                         /* dot11CurrentSet Unsigned32 */
    uint32_t dot11CurrentPattern;                     /* dot11CurrentPattern Unsigned32 */
    uint32_t dot11CurrentIndex;                       /* dot11CurrentIndex Unsigned32 */
    uint32_t dot11EHCCPrimeRadix;                     /* dot11EHCCPrimeRadix Unsigned32 */
    uint32_t dot11EHCCNumberofChannelsFamilyIndex;    /* dot11EHCCNumberofChannelsFamilyIndex Unsigned32 */
    oal_bool_enum_uint8 en_dot11EHCCCapabilityImplemented; /* dot11EHCCCapabilityImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11EHCCCapabilityActivated;   /* dot11EHCCCapabilityActivated TruthValue */
    uint8_t uc_dot11HopAlgorithmAdopted;                 /* dot11HopAlgorithmAdopted INTEGER */
    oal_bool_enum_uint8 en_dot11RandomTableFlag;           /* dot11RandomTableFlag TruthValue */
    uint32_t dot11NumberofHoppingSets;                /* dot11NumberofHoppingSets Unsigned32 */
    uint32_t dot11HopModulus;                         /* dot11HopModulus Unsigned32 */
    uint32_t dot11HopOffset;                          /* dot11HopOffset Unsigned32 */
} wlan_mib_Dot11PhyFHSSEntry_stru;

/****************************************************************************************/
/* dot11PhyIRTable OBJECT-TYPE                                                          */
/* SYNTAX SEQUENCE OF Dot11PhyIREntry                                                   */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Group of attributes for dot11PhyIRTable. Implemented as a table indexed        */
/*      on ifIndex to allow for multiple instances on an Agent."                        */
/* ::= { dot11phy 6 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11CCAWatchdogTimerMax; /* dot11CCAWatchdogTimerMax Unsigned32 */
    uint32_t dot11CCAWatchdogCountMax; /* dot11CCAWatchdogCountMax Unsigned32 */
    uint32_t dot11CCAWatchdogTimerMin; /* dot11CCAWatchdogTimerMin Unsigned32 */
    uint32_t dot11CCAWatchdogCountMin; /* dot11CCAWatchdogCountMin Unsigned32 */
} wlan_mib_Dot11PhyIREntry_stru;

/****************************************************************************************/
/* dot11RegDomainsSupportedTable OBJECT-TYPE                                            */
/* SYNTAX SEQUENCE OF Dot11RegDomainsSupportedEntry                                     */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS deprecated                                                                    */
/* DESCRIPTION                                                                          */
/*      "Superceded by dot11OperatingClassesTable.                                      */
/*      There are different operational requirements dependent on the regulatory        */
/*      domain. This attribute list describes the regulatory domains the PLCP and       */
/*      PMD support in this implementation. Currently defined values and their          */
/*      corresponding Regulatory Domains are:                                           */
/*      FCC (USA) = X'10', DOC (Canada) = X'20', ETSI (most of Europe) = X'30',         */
/*      Spain = X'31', France = X'32', Japan = X'40', China = X'50', Other = X'00'"     */
/* ::= { dot11phy 7}                                                                    */
/****************************************************************************************/
typedef struct {
    uint32_t dot11RegDomainsSupportedIndex;  /* dot11RegDomainsSupportedIndex Unsigned32 */
    uint8_t uc_dot11RegDomainsImplementedValue; /* dot11RegDomainsImplementedValue INTEGER */
} wlan_mib_Dot11RegDomainsSupportedEntry_stru;

/****************************************************************************************/
/* dot11AntennasListTable OBJECT-TYPE                                                   */
/* SYNTAX SEQUENCE OF Dot11AntennasListEntry                                            */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "This table represents the list of antennae. An antenna can be marked to        */
/*      be capable of transmitting, receiving, and/or for participation in receive      */
/*      diversity. Each entry in this table represents a single antenna with its        */
/*      properties. The maximum number of antennae that can be contained in this        */
/*      table is 255."                                                                  */
/* ::= { dot11phy 8 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11AntennaListIndex;                /* dot11AntennaListIndex Unsigned32 */
    uint32_t dot11TxAntennaImplemented;            /* dot11TxAntennaImplemented TruthValue */
    uint32_t dot11RxAntennaImplemented;            /* dot11RxAntennaImplemented TruthValue */
    uint32_t dot11DiversitySelectionRxImplemented; /* dot11DiversitySelectionRxImplemented TruthValue */
} wlan_mib_Dot11AntennasListEntry_stru;

/****************************************************************************************/
/* dot11SupportedDataRatesTxTable OBJECT-TYPE                                           */
/* SYNTAX SEQUENCE OF Dot11SupportedDataRatesTxEntry                                    */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "The Transmit bit rates supported by the PLCP and PMD, represented by a         */
/*      count from X'02-X'7f, corresponding to data rates in increments of              */
/*      500kbit/s from 1 Mb/s to 63.5 Mb/s subject to limitations of each individ-      */
/*      ual PHY."                                                                       */
/* ::= { dot11phy 9 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11SupportedDataRatesTxIndex;   /* dot11SupportedDataRatesTxIndex Unsigned32 */
    uint32_t dot11ImplementedDataRatesTxValue; /* dot11ImplementedDataRatesTxValue Unsigned32 */
} wlan_mib_Dot11SupportedDataRatesTxEntry_stru;

/****************************************************************************************/
/* dot11SupportedDataRatesRxTable OBJECT-TYPE                                           */
/* SYNTAX SEQUENCE OF Dot11SupportedDataRatesRxEntry                                    */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "The receive bit rates supported by the PLCP and PMD, represented by a          */
/*      count from X'002-X'7f, corresponding to data rates in increments of             */
/*      500kbit/s from 1 Mb/s to 63.5 Mb/s."                                            */
/* ::= { dot11phy 10 }                                                                  */
/****************************************************************************************/
typedef struct {
    uint32_t dot11SupportedDataRatesRxIndex;   /* dot11SupportedDataRatesRxIndex Unsigned32 */
    uint32_t dot11ImplementedDataRatesRxValue; /* dot11ImplementedDataRatesRxValue Unsigned32 */
} wlan_mib_Dot11SupportedDataRatesRxEntry_stru;

/****************************************************************************************/
/* dot11PhyOFDMTable OBJECT-TYPE                                                        */
/* SYNTAX SEQUENCE OF Dot11PhyOFDMEntry                                                 */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Group of attributes for dot11PhyOFDMTable. Implemented as a table indexed      */
/*      on ifindex to allow for multiple instances on an Agent."                        */
/* ::= { dot11phy 11 }                                                                  */
/****************************************************************************************/
typedef struct {
    uint32_t dot11CurrentFrequency;                       /* dot11CurrentFrequency Unsigned32 */
    int32_t l_dot11TIThreshold;                              /* dot11TIThreshold Integer32 */
    uint32_t dot11FrequencyBandsImplemented;              /* dot11FrequencyBandsImplemented Unsigned32 */
    uint32_t dot11ChannelStartingFactor;                  /* dot11ChannelStartingFactor Unsigned32 */
    oal_bool_enum_uint8 en_dot11FiveMHzOperationImplemented;   /* dot11FiveMHzOperationImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11TenMHzOperationImplemented;    /* dot11TenMHzOperationImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11TwentyMHzOperationImplemented; /* dot11TwentyMHzOperationImplemented TruthValue */
    uint8_t uc_dot11PhyOFDMChannelWidth;                     /* dot11PhyOFDMChannelWidth INTEGER */
    oal_bool_enum_uint8 en_dot11OFDMCCAEDImplemented;          /* dot11OFDMCCAEDImplemented  TruthValue */
    oal_bool_enum_uint8 en_dot11OFDMCCAEDRequired;             /* dot11OFDMCCAEDRequired  TruthValue */
    uint32_t dot11OFDMEDThreshold;                        /* dot11OFDMEDThreshold  Unsigned32 */
    uint8_t uc_dot11STATransmitPowerClass;                   /* dot11STATransmitPowerClass INTEGER */
    uint8_t uc_dot11ACRType;                                 /* dot11ACRType INTEGER */
} wlan_mib_Dot11PhyOFDMEntry_stru;

/****************************************************************************************/
/* dot11PhyHRDSSSTable OBJECT-TYPE                                                      */
/* SYNTAX SEQUENCE OF Dot11PhyHRDSSSEntry                                               */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11PhyHRDSSSEntry. Implemented as a table            */
/*      indexed on ifIndex to allow for multiple instances on an Agent."                */
/* ::= { dot11phy 12 }                                                                  */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 en_dot11ShortPreambleOptionImplemented; /* dot11ShortPreambleOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11PBCCOptionImplemented;          /* dot11PBCCOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11ChannelAgilityPresent;          /* dot11ChannelAgilityPresent TruthValue */
    // oal_bool_enum_uint8 en_dot11ChannelAgilityActivated;        /* dot11ChannelAgilityActivated TruthValue */
    // uint32_t  dot11HRCCAModeImplemented;                   /* dot11HRCCAModeImplemented Unsigned32 */
} wlan_mib_Dot11PhyHRDSSSEntry_stru;

/****************************************************************************************/
/* dot11HoppingPatternTable OBJECT-TYPE                                                 */
/* SYNTAX SEQUENCE OF Dot11HoppingPatternEntry                                          */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "The (conceptual) table of attributes necessary for a frequency hopping         */
/*      implementation to be able to create the hopping sequences necessary to          */
/*      operate in the subband for the associated domain country string."               */
/* ::= { dot11phy 13 }                                                                  */
/****************************************************************************************/
typedef struct {
    uint32_t dot11HoppingPatternIndex;    /* dot11HoppingPatternIndex Unsigned32 */
    uint32_t dot11RandomTableFieldNumber; /* dot11RandomTableFieldNumber Unsigned32 */
} wlan_mib_Dot11HoppingPatternEntry_stru;

/****************************************************************************************/
/* dot11PhyERPTable OBJECT-TYPE                                                         */
/* SYNTAX SEQUENCE OF Dot11PhyERPEntry                                                  */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11PhyERPEntry. Implemented as a table indexed       */
/*      on ifIndex to allow for multiple instances on an Agent."                        */
/* ::= { dot11phy 14 }                                                                  */
/****************************************************************************************/
typedef struct {
    // oal_bool_enum_uint8 en_dot11ERPPBCCOptionImplemented;       /* dot11ERPPBCCOptionImplemented TruthValue */
    // oal_bool_enum_uint8 en_dot11ERPBCCOptionActivated;          /* dot11ERPBCCOptionActivated TruthValue */
    // oal_bool_enum_uint8 en_dot11DSSSOFDMOptionImplemented;      /* dot11DSSSOFDMOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11DSSSOFDMOptionActivated;        /* dot11DSSSOFDMOptionActivated TruthValue */
    oal_bool_enum_uint8 en_dot11ShortSlotTimeOptionImplemented; /* dot11ShortSlotTimeOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11ShortSlotTimeOptionActivated;   /* dot11ShortSlotTimeOptionActivated TruthValue */
} wlan_mib_Dot11PhyERPEntry_stru;

/****************************************************************************************/
/* dot11PhyDSSSTable OBJECT-TYPE                                                        */
/* SYNTAX SEQUENCE OF Dot11PhyDSSSEntry                                                 */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11PhyDSSSEntry. Implemented as a table indexed      */
/*      on ifIndex to allow for multiple instances on an Agent."                        */
/* ::= { dot11phy 5 }                                                                   */
/****************************************************************************************/
typedef struct {
    wlan_mib_Dot11PhyHRDSSSEntry_stru st_phy_hrdsss;
    wlan_mib_Dot11PhyERPEntry_stru st_phy_erp;
} wlan_mib_Dot11PhyDSSSEntry_stru;

/****************************************************************************************/
/* dot11TransmitBeamformingConfigTable OBJECT-TYPE                                      */
/* SYNTAX SEQUENCE OF Dot11TransmitBeamformingConfigEntry                               */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11TransmitBeamformingConfigTable. Implemented       */
/*      as a table indexed on ifIndex to allow for multiple instances on an             */
/*      Agent."                                                                         */
/* ::= { dot11phy 18 }                                                                  */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 en_dot11ReceiveStaggerSoundingOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11TransmitStaggerSoundingOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11ReceiveNDPOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11TransmitNDPOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11ImplicitTransmitBeamformingOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11ExplicitCSITransmitBeamformingOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11ExplicitNonCompressedBeamformingMatrixOptionImplemented : 1;
    oal_bool_enum_uint8 en_resv : 1;

    oal_bool_enum_uint8 uc_dot11CalibrationOptionImplemented : 2;
    oal_bool_enum_uint8 uc_dot11ExplicitTransmitBeamformingCSIFeedbackOptionImplemented : 2;
    oal_bool_enum_uint8 uc_dot11ExplicitNonCompressedBeamformingFeedbackOptionImplemented : 2;
    oal_bool_enum_uint8 uc_dot11ExplicitCompressedBeamformingFeedbackOptionImplemented : 2;

    uint8_t uc_dot11NumberBeamFormingCSISupportAntenna;
    uint8_t uc_dot11NumberNonCompressedBeamformingMatrixSupportAntenna;
    uint8_t uc_dot11NumberCompressedBeamformingMatrixSupportAntenna;
} wlan_mib_Dot11TransmitBeamformingConfigEntry_stru;
/****************************************************************************************/
/* dot11PhyHTTable OBJECT-TYPE                                                          */
/* SYNTAX SEQUENCE OF Dot11PhyHTEntry                                                   */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11PhyHTTable. Implemented as a table indexed        */
/*      on ifIndex to allow for multiple instances on an Agent."                        */
/* ::= { dot11phy 15 }                                                                  */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 en_dot112GFortyMHzOperationImplemented : 1;    /* dot11FortyMHzOperationImplemented */
    oal_bool_enum_uint8 en_dot115GFortyMHzOperationImplemented : 1;    /* dot11FortyMHzOperationImplemented */
    oal_bool_enum_uint8 en_dot11HTGreenfieldOptionImplemented : 1;     /* dot11HTGreenfieldOptionImplemented */
    oal_bool_enum_uint8 en_dot11ShortGIOptionInTwentyImplemented : 1;  /* dot11ShortGIOptionInTwentyImplemented */
    oal_bool_enum_uint8 en_dot112GShortGIOptionInFortyImplemented : 1; /* dot11ShortGIOptionInFortyImplemented */
    oal_bool_enum_uint8 en_dot115GShortGIOptionInFortyImplemented : 1; /* dot11ShortGIOptionInFortyImplemented */
    oal_bool_enum_uint8 en_dot11LDPCCodingOptionImplemented : 1;       /* dot11LDPCCodingOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11LDPCCodingOptionActivated : 1;         /* dot11LDPCCodingOptionActivated TruthValue */

    oal_bool_enum_uint8 en_dot11TxSTBCOptionImplemented : 1;      /* dot11TxSTBCOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11TxSTBCOptionActivated : 1;        /* dot11TxSTBCOptionActivated TruthValue */
    oal_bool_enum_uint8 en_dot11RxSTBCOptionImplemented : 1;      /* dot11RxSTBCOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11TxMCSSetDefined : 1;              /* dot11TxMCSSetDefined TruthValue */
    oal_bool_enum_uint8 en_dot11TxRxMCSSetNotEqual : 1;           /* dot11TxRxMCSSetNotEqual TruthValue */
    oal_bool_enum_uint8 en_dot11TxUnequalModulationSupported : 1; /* dot11TxUnequalModulationSupported TruthValue */
    oal_bool_enum_uint8 en_resv : 2;

    // oal_bool_enum_uint8 en_dot11FortyMHzOperationActivated;        /* dot11FortyMHzOperationActivated TruthValue */
    // uint32_t          dot11CurrentPrimaryChannel;             /* dot11CurrentPrimaryChannel Unsigned32 */
    // uint32_t          dot11CurrentSecondaryChannel;           /* dot11CurrentSecondaryChannel Unsigned32 */
    // uint32_t          dot11NumberOfSpatialStreamsImplemented; /* dot11NumberOfSpatialStreamsImplemented */
    // uint32_t          dot11NumberOfSpatialStreamsActivated;   /* dot11NumberOfSpatialStreamsActivated */
    // oal_bool_enum_uint8 en_dot11HTGreenfieldOptionActivated;       /* dot11HTGreenfieldOptionActivated TruthValue */
    // oal_bool_enum_uint8 en_dot11ShortGIOptionInTwentyActivated;    /* dot11ShortGIOptionInTwentyActivated */
    // oal_bool_enum_uint8 en_dot11ShortGIOptionInFortyActivated;     /* dot11ShortGIOptionInFortyActivated */
    // oal_bool_enum_uint8 en_dot11RxSTBCOptionActivated;             /* dot11RxSTBCOptionActivated TruthValue */
    // oal_bool_enum_uint8 en_dot11BeamFormingOptionImplemented;      /* dot11BeamFormingOptionImplemented TruthValue */
    // oal_bool_enum_uint8 en_dot11BeamFormingOptionActivated;        /* dot11BeamFormingOptionActivated TruthValue */
    uint32_t dot11HighestSupportedDataRate;               /* dot11HighestSupportedDataRate Unsigned32 */
    uint32_t dot11TxMaximumNumberSpatialStreamsSupported; /* dot11TxMaximumNumberSpatialStreamsSupported */

    wlan_mib_Dot11TransmitBeamformingConfigEntry_stru st_wlan_mib_txbf_config;
} wlan_mib_Dot11PhyHTEntry_stru;

/****************************************************************************************/
/* dot11SupportedMCSTxTable OBJECT-TYPE                                                 */
/* SYNTAX SEQUENCE OF Dot11SupportedMCSTxEntry                                          */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "he Transmit MCS supported by the PLCP and PMD, represented by a count          */
/*      from 1 to 127, subject to limitations of each individual PHY."                  */
/* ::= { dot11phy 16 }                                                                  */
/****************************************************************************************/
typedef struct {
    uint8_t auc_dot11SupportedMCSTxValue[WLAN_HT_MCS_BITMASK_LEN]; /* dot11SupportedMCSTxValue Unsigned32 */
} wlan_mib_Dot11SupportedMCSTxEntry_stru;

/****************************************************************************************/
/* dot11SupportedMCSRxTable OBJECT-TYPE                                                 */
/* SYNTAX SEQUENCE OF Dot11SupportedMCSRxEntry                                          */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "The receive MCS supported by the PLCP and PMD, represented by a count          */
/*      from 1 to 127, subject to limitations of each individual PHY."                  */
/* ::= { dot11phy 17 }                                                                  */
/****************************************************************************************/
typedef struct {
    // uint32_t  dot11SupportedMCSRxIndex;                             /* dot11SupportedMCSRxIndex Unsigned32 */
    uint8_t auc_dot11SupportedMCSRxValue[WLAN_HT_MCS_BITMASK_LEN]; /* dot11SupportedMCSRxValue Unsigned32 */
    // uint8_t   uc_num_rx_mcs;
} wlan_mib_Dot11SupportedMCSRxEntry_stru;

typedef struct {
    wlan_mib_Dot11SupportedMCSTxEntry_stru st_supported_mcstx;
    wlan_mib_Dot11SupportedMCSRxEntry_stru st_supported_mcsrx;
} wlan_mib_Dot11SupportedMCSEntry_stru;
/****************************************************************************************/
/* dot11VHTTransmitBeamformingConfigTable OBJECT-TYPE                                   */
/* SYNTAX SEQUENCE OF Dot11VHTTransmitBeamformingConfigEntry                            */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11VHTTransmitBeamformingConfigTable. Imple-         */
/*      mented as a table indexed on ifIndex to allow for multiple instances on an      */
/*      Agent."                                                                         */
/*  ::= { dot11phy 24 }                                                                 */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 en_dot11VHTSUBeamformeeOptionImplemented; /* dot11VHTSUBeamformeeOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11VHTSUBeamformerOptionImplemented; /* dot11VHTSUBeamformerOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11VHTMUBeamformeeOptionImplemented; /* dot11VHTMUBeamformeeOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11VHTMUBeamformerOptionImplemented; /* dot11VHTMUBeamformerOptionImplemented TruthValue */
    uint8_t uc_dot11VHTNumberSoundingDimensions;                /* dot11VHTNumberSoundingDimensions  Unsigned32 */
    uint8_t uc_dot11VHTBeamformeeNTxSupport;                    /* dot11VHTBeamformeeNTxSupport Unsigned32 */
} wlan_mib_Dot11VHTTransmitBeamformingConfigEntry_stru;

/****************************************************************************************/
/* dot11PhyVHTTable OBJECT-TYPE                                                         */
/* SYNTAX SEQUENCE OF Dot11PhyVHTEntry                                                  */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/*      DESCRIPTION                                                                     */
/*      "Entry of attributes for dot11PhyVHTTable. Implemented as a table indexed       */
/*      on ifIndex to allow for multiple instances on an Agent."                        */
/* ::= { dot11phy 23 }                                                                  */
/****************************************************************************************/
typedef struct {
    uint8_t en_dot11VHTChannelWidthOptionImplemented : 2; /* wlan_mib_vht_supp_width_enum */
    oal_bool_enum_uint8 en_dot11VHTShortGIOptionIn80Implemented : 1;
    oal_bool_enum_uint8 en_dot11VHTShortGIOptionIn160and80p80Implemented : 1;
    oal_bool_enum_uint8 en_dot11VHTLDPCCodingOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11VHTTxSTBCOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11VHTRxSTBCOptionImplemented : 1;
    uint8_t en_rev : 1;

    uint8_t resv[NUM_3_BYTES];
    // uint8_t           uc_dot11CurrentChannelwidth;                        /* dot11CurrentChannelWidth INTEGER */
    // uint32_t          dot11CurrentChannelCenterFrequencyIndex0;
    // uint32_t          dot11CurrentChannelCenterFrequencyIndex1;
    // oal_bool_enum_uint8 en_dot11VHTShortGIOptionIn80Activated;              /* dot11VHTShortGIOptionIn80Activated */
    // oal_bool_enum_uint8 en_dot11VHTShortGIOptionIn160and80p80Activated;
    // oal_bool_enum_uint8 en_dot11VHTLDPCCodingOptionActivated;               /* dot11VHTLDPCCodingOptionActivated */
    // oal_bool_enum_uint8 en_dot11VHTTxSTBCOptionActivated;                   /* dot11VHTTxSTBCOptionActivated */
    // oal_bool_enum_uint8 en_dot11VHTRxSTBCOptionActivated;                   /* dot11VHTRxSTBCOptionActivated */
    // uint32_t          dot11VHTMUMaxUsersImplemented;                   /* dot11VHTMUMaxUsersImplemented */
    // uint32_t          dot11VHTMUMaxNSTSPerUserImplemented;             /* dot11VHTMUMaxNSTSPerUserImplemented */
    // uint32_t          dot11VHTMUMaxNSTSTotalImplemented;               /* dot11VHTMUMaxNSTSTotalImplemented */
    wlan_mib_Dot11VHTTransmitBeamformingConfigEntry_stru st_wlan_mib_vht_txbf_config;
} wlan_mib_Dot11PhyVHTEntry_stru;

/* Start of dot11imt OBJECT IDENTIFIER ::= {ieee802dot11 6} */
/****************************************************************************************/
/*        Start of dot11imt OBJECT IDENTIFIER ::= {ieee802dot11 6}                      */
/*          -- IMT GROUPS                                                               */
/*          -- dot11BSSIdTable ::= { dot11imt 1 }                                       */
/*          -- dot11InterworkingTable ::= { dot11imt 2 }                                */
/*          -- dot11APLCI ::= { dot11imt 3 }                                            */
/*          -- dot11APCivicLocation ::= { dot11imt 4 }                                  */
/*          -- dot11RoamingConsortiumTable      ::= { dot11imt 5 }                      */
/*          -- dot11DomainNameTable ::= { dot11imt 6 }                                  */
/*          -- Generic Advertisement Service (GAS) Attributes                           */
/*          -- DEFINED AS "The Generic Advertisement Service management                 */
/*          -- object class provides the necessary support for an Advertisement         */
/*          -- service to interwork with external systems."                             */
/*          -- GAS GROUPS                                                               */
/*          -- dot11GASAdvertisementTable       ::= { dot11imt 7 }                      */
/****************************************************************************************/
/****************************************************************************************/
/* dot11BSSIdTable OBJECT-TYPE                                                          */
/* SYNTAX         SEQUENCE OF Dot11BSSIdEntry                                           */
/* MAX-ACCESS     not-accessible                                                        */
/* STATUS         current                                                               */
/* DESCRIPTION                                                                          */
/*      "This object is a table of BSSIDs contained within an Access Point (AP)."       */
/*  ::= { dot11imt 1 }                                                                  */
/****************************************************************************************/
typedef struct {
    uint8_t auc_dot11APMacAddress[WLAN_MAC_ADDR_LEN]; /* dot11APMacAddress OBJECT-TYPE MacAddress */
} wlan_mib_Dot11BSSIdEntry_stru;

/****************************************************************************************/
/* dot11InterworkingTable OBJECT-TYPE                                                   */
/* SYNTAX SEQUENCE OF Dot11InterworkingEntry                                            */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "This table represents the non-AP STAs associated to the AP. An entry is        */
/*      created automatically by the AP when the STA becomes associated to the AP.      */
/*      The corresponding entry is deleted when the STA disassociates. Each STA         */
/*      added to this table is uniquely identified by its MAC address. This table       */
/*      is moved to a new AP following a successful STA BSS transition event."          */
/* ::= { dot11imt 2 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint8_t auc_dot11NonAPStationMacAddress[WLAN_MAC_ADDR_LEN];    /* dot11NonAPStationMacAddress MacAddress */
    uint8_t auc_dot11NonAPStationUserIdentity[NUM_255_BYTES];      /* dot11NonAPStationUserIdentity DisplayString */
    uint8_t uc_dot11NonAPStationInterworkingCapability;        /* dot11NonAPStationInterworkingCapability BITS */
    uint8_t auc_dot11NonAPStationAssociatedSSID[NUM_32_BYTES];    /* dot11NonAPStationAssociatedSSID OCTET STRING */
    uint8_t auc_dot11NonAPStationUnicastCipherSuite[NUM_4_BYTES]; /* dot11NonAPStationUnicastCipherSuite OCTET STRING */
    uint8_t auc_dot11NonAPStationBroadcastCipherSuite[NUM_4_BYTES];   /* dot11NonAPStationBroadcastCipherSuite */
    uint8_t uc_dot11NonAPStationAuthAccessCategories;          /* dot11NonAPStationAuthAccessCategories BITS */
    uint32_t dot11NonAPStationAuthMaxVoiceRate;             /* dot11NonAPStationAuthMaxVoiceRate Unsigned32 */
    uint32_t dot11NonAPStationAuthMaxVideoRate;             /* dot11NonAPStationAuthMaxVideoRate Unsigned32 */
    uint32_t dot11NonAPStationAuthMaxBestEffortRate;        /* dot11NonAPStationAuthMaxBestEffortRate Unsigned32 */
    uint32_t dot11NonAPStationAuthMaxBackgroundRate;        /* dot11NonAPStationAuthMaxBackgroundRate Unsigned32 */
    uint32_t dot11NonAPStationAuthMaxVoiceOctets;           /* dot11NonAPStationAuthMaxVoiceOctets Unsigned32 */
    uint32_t dot11NonAPStationAuthMaxVideoOctets;           /* dot11NonAPStationAuthMaxVideoOctets Unsigned32 */
    uint32_t dot11NonAPStationAuthMaxBestEffortOctets;      /* dot11NonAPStationAuthMaxBestEffortOctets */
    uint32_t dot11NonAPStationAuthMaxBackgroundOctets;      /* dot11NonAPStationAuthMaxBackgroundOctets */
    uint32_t dot11NonAPStationAuthMaxHCCAHEMMOctets;        /* dot11NonAPStationAuthMaxHCCAHEMMOctets Unsigned32 */
    uint32_t dot11NonAPStationAuthMaxTotalOctets;           /* dot11NonAPStationAuthMaxTotalOctets Unsigned32 */
    oal_bool_enum_uint8 en_dot11NonAPStationAuthHCCAHEMM;        /* dot11NonAPStationAuthHCCAHEMM TruthValue */
    uint32_t dot11NonAPStationAuthMaxHCCAHEMMRate;          /* dot11NonAPStationAuthMaxHCCAHEMMRate Unsigned32 */
    uint32_t dot11NonAPStationAuthHCCAHEMMDelay;            /* dot11NonAPStationAuthHCCAHEMMDelay Unsigned32 */
    oal_bool_enum_uint8 en_dot11NonAPStationAuthSourceMulticast; /* dot11NonAPStationAuthSourceMulticast TruthValue */
    uint32_t dot11NonAPStationAuthMaxSourceMulticastRate;   /* dot11NonAPStationAuthMaxSourceMulticastRate */
    uint32_t dot11NonAPStationVoiceMSDUCount;               /* dot11NonAPStationVoiceMSDUCount Counter32 */
    uint32_t dot11NonAPStationDroppedVoiceMSDUCount;        /* dot11NonAPStationDroppedVoiceMSDUCount Counter32 */
    uint32_t dot11NonAPStationVoiceOctetCount;              /* dot11NonAPStationVoiceOctetCount Counter32 */
    uint32_t dot11NonAPStationDroppedVoiceOctetCount;       /* dot11NonAPStationDroppedVoiceOctetCount Counter32 */
    uint32_t dot11NonAPStationVideoMSDUCount;               /* dot11NonAPStationVideoMSDUCount Counter32 */
    uint32_t dot11NonAPStationDroppedVideoMSDUCount;        /* dot11NonAPStationDroppedVideoMSDUCount Counter32 */
    uint32_t dot11NonAPStationVideoOctetCount;              /* dot11NonAPStationVideoOctetCount Counter32 */
    uint32_t dot11NonAPStationDroppedVideoOctetCount;       /* dot11NonAPStationDroppedVideoOctetCount Counter32 */
    uint32_t dot11NonAPStationBestEffortMSDUCount;          /* dot11NonAPStationBestEffortMSDUCount Counter32 */
    uint32_t dot11NonAPStationDroppedBestEffortMSDUCount;   /* dot11NonAPStationDroppedBestEffortMSDUCount */
    uint32_t dot11NonAPStationBestEffortOctetCount;         /* dot11NonAPStationBestEffortOctetCount Counter32 */
    uint32_t dot11NonAPStationDroppedBestEffortOctetCount;  /* dot11NonAPStationDroppedBestEffortOctetCount */
    uint32_t dot11NonAPStationBackgroundMSDUCount;          /* dot11NonAPStationBackgroundMSDUCount Counter32 */
    uint32_t dot11NonAPStationDroppedBackgroundMSDUCount;   /* dot11NonAPStationDroppedBackgroundMSDUCount */
    uint32_t dot11NonAPStationBackgroundOctetCount;         /* dot11NonAPStationBackgroundOctetCount */
    uint32_t dot11NonAPStationDroppedBackgroundOctetCount;  /* dot11NonAPStationDroppedBackgroundOctetCount */
    uint32_t dot11NonAPStationHCCAHEMMMSDUCount;            /* dot11NonAPStationHCCAHEMMMSDUCount Counter32 */
    uint32_t dot11NonAPStationDroppedHCCAHEMMMSDUCount;     /* dot11NonAPStationDroppedHCCAHEMMMSDUCount */
    uint32_t dot11NonAPStationHCCAHEMMOctetCount;           /* dot11NonAPStationHCCAHEMMOctetCount Counter32 */
    uint32_t dot11NonAPStationDroppedHCCAHEMMOctetCount;    /* dot11NonAPStationDroppedHCCAHEMMOctetCount */
    uint32_t dot11NonAPStationMulticastMSDUCount;           /* dot11NonAPStationMulticastMSDUCount */
    uint32_t dot11NonAPStationDroppedMulticastMSDUCount;    /* dot11NonAPStationDroppedMulticastMSDUCount */
    uint32_t dot11NonAPStationMulticastOctetCount;          /* dot11NonAPStationMulticastOctetCount */
    uint32_t dot11NonAPStationDroppedMulticastOctetCount;   /* dot11NonAPStationDroppedMulticastOctetCount */
    uint8_t uc_dot11NonAPStationPowerManagementMode;           /* dot11NonAPStationPowerManagementMode INTEGER */
    oal_bool_enum_uint8 en_dot11NonAPStationAuthDls;             /* dot11NonAPStationAuthDls TruthValue */
    uint32_t dot11NonAPStationVLANId;                       /* dot11NonAPStationVLANId Unsigned32 */
    uint8_t auc_dot11NonAPStationVLANName[64];                 /* dot11NonAPStationVLANName DisplayString SIZE(64) */
    uint8_t uc_dot11NonAPStationAddtsResultCode;               /* dot11NonAPStationAddtsResultCode INTEGER */
} wlan_mib_Dot11InterworkingEntry_stru;

/****************************************************************************************/
/* dot11APLCITable OBJECT-TYPE                                                          */
/* SYNTAX         SEQUENCE OF Dot11APLCIEntry                                           */
/* MAX-ACCESS     not-accessible                                                        */
/* STATUS         current                                                               */
/* DESCRIPTION                                                                          */
/*      "This table represents the Geospatial location of the AP as specified in        */
/*      8.4.2.23.10."                                                                   */
/* ::= { dot11imt 3 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11APLCIIndex;                                    /* dot11APLCIIndex Unsigned32 */
    uint32_t dot11APLCILatitudeResolution;                       /* dot11APLCILatitudeResolution Unsigned32 */
    int32_t l_dot11APLCILatitudeInteger;                            /* dot11APLCILatitudeInteger Integer32 */
    int32_t l_dot11APLCILatitudeFraction;                           /* dot11APLCILatitudeFraction Integer32 */
    uint32_t dot11APLCILongitudeResolution;                      /* dot11APLCILongitudeResolution Unsigned32 */
    int32_t l_dot11APLCILongitudeInteger;                           /* dot11APLCILongitudeInteger Integer32 */
    int32_t l_dot11APLCILongitudeFraction;                          /* dot11APLCILongitudeFraction Integer32 */
    uint8_t uc_dot11APLCIAltitudeType;                              /* dot11APLCIAltitudeType INTEGER */
    uint32_t dot11APLCIAltitudeResolution;                       /* dot11APLCIAltitudeResolution Unsigned32 */
    int32_t l_dot11APLCIAltitudeInteger;                            /* dot11APLCIAltitudeInteger Integer32 */
    int32_t l_dot11APLCIAltitudeFraction;                           /* dot11APLCIAltitudeFraction Integer32 */
    uint8_t en_dot11APLCIDatum;              /* wlan_mib_ap_lci_datum_enum: dot11APLCIDatum INTEGER */
    uint8_t en_dot11APLCIAzimuthType; /* wlan_mib_ap_lci_azimuth_type_enum: dot11APLCIAzimuthType INTEGER */
    uint32_t dot11APLCIAzimuthResolution;                        /* dot11APLCIAzimuthResolution Unsigned32 */
    int32_t l_dot11APLCIAzimuth;                                    /* dot11APLCIAzimuth Integer32 */
} wlan_mib_Dot11APLCIEntry_stru;

/****************************************************************************************/
/* dot11APCivicLocationTable OBJECT-TYPE                                                */
/* SYNTAX SEQUENCE OF Dot11ApCivicLocationEntry                                         */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "This table represents the location of the AP in Civic format using the         */
/*      Civic Address Type elements defined in IETF RFC-5139 [B42]."                    */
/* ::= { dot11imt 4 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11APCivicLocationIndex;        /* dot11APCivicLocationIndex Unsigned32 */
    uint8_t auc_dot11APCivicLocationCountry[NUM_255_BYTES]; /* dot11APCivicLocationCountry OCTET STRING */
    uint8_t auc_dot11APCivicLocationA1[NUM_255_BYTES];      /* dot11APCivicLocationA1 OCTET STRING */
    uint8_t auc_dot11APCivicLocationA2[NUM_255_BYTES];      /* dot11APCivicLocationA2 OCTET STRING */
    uint8_t auc_dot11APCivicLocationA3[NUM_255_BYTES];      /* dot11APCivicLocationA3 OCTET STRING */
    uint8_t auc_dot11APCivicLocationA4[NUM_255_BYTES];      /* dot11APCivicLocationA4 OCTET STRING */
    uint8_t auc_dot11APCivicLocationA5[NUM_255_BYTES];      /* dot11APCivicLocationA5 OCTET STRING */
    uint8_t auc_dot11APCivicLocationA6[NUM_255_BYTES];      /* dot11APCivicLocationA6 OCTET STRING */
    uint8_t auc_dot11APCivicLocationPrd[NUM_255_BYTES];     /* dot11APCivicLocationPrd OCTET STRING */
    uint8_t auc_dot11APCivicLocationPod[NUM_255_BYTES];     /* dot11APCivicLocationPod OCTET STRING */
    uint8_t auc_dot11APCivicLocationSts[NUM_255_BYTES];     /* dot11APCivicLocationSts OCTET STRING */
    uint8_t auc_dot11APCivicLocationHno[NUM_255_BYTES];     /* dot11APCivicLocationHno OCTET STRING */
    uint8_t auc_dot11APCivicLocationHns[NUM_255_BYTES];     /* dot11APCivicLocationHns OCTET STRING */
    uint8_t auc_dot11APCivicLocationLmk[NUM_255_BYTES];     /* dot11APCivicLocationLmk OCTET STRING */
    uint8_t auc_dot11APCivicLocationLoc[NUM_255_BYTES];     /* dot11APCivicLocationLoc OCTET STRING */
    uint8_t auc_dot11APCivicLocationNam[NUM_255_BYTES];     /* dot11APCivicLocationNam OCTET STRING */
    uint8_t auc_dot11APCivicLocationPc[NUM_255_BYTES];      /* dot11APCivicLocationPc OCTET STRING */
    uint8_t auc_dot11APCivicLocationBld[NUM_255_BYTES];     /* dot11APCivicLocationBld OCTET STRING */
    uint8_t auc_dot11APCivicLocationUnit[NUM_255_BYTES];    /* dot11APCivicLocationUnit OCTET STRING */
    uint8_t auc_dot11APCivicLocationFlr[NUM_255_BYTES];     /* dot11APCivicLocationFlr OCTET STRING */
    uint8_t auc_dot11APCivicLocationRoom[NUM_255_BYTES];    /* dot11APCivicLocationRoom OCTET STRING */
    uint8_t auc_dot11APCivicLocationPlc[NUM_255_BYTES];     /* dot11APCivicLocationPlc OCTET STRING */
    uint8_t auc_dot11APCivicLocationPcn[NUM_255_BYTES];     /* dot11APCivicLocationPcn OCTET STRING */
    uint8_t auc_dot11APCivicLocationPobox[NUM_255_BYTES];   /* dot11APCivicLocationPobox OCTET STRING */
    uint8_t auc_dot11APCivicLocationAddcode[NUM_255_BYTES]; /* dot11APCivicLocationAddcode OCTET STRING */
    uint8_t auc_dot11APCivicLocationSeat[NUM_255_BYTES];    /* dot11APCivicLocationSeat OCTET STRING */
    uint8_t auc_dot11APCivicLocationRd[NUM_255_BYTES];      /* dot11APCivicLocationRd OCTET STRING */
    uint8_t auc_dot11APCivicLocationRdsec[NUM_255_BYTES];   /* dot11APCivicLocationRdsec OCTET STRING */
    uint8_t auc_dot11APCivicLocationRdbr[NUM_255_BYTES];    /* dot11APCivicLocationRdbr OCTET STRING */
    uint8_t auc_dot11APCivicLocationRdsubbr[NUM_255_BYTES]; /* dot11APCivicLocationRdsubbr OCTET STRING */
    uint8_t auc_dot11APCivicLocationPrm[NUM_255_BYTES];     /* dot11APCivicLocationPrm OCTET STRING */
    uint8_t auc_dot11APCivicLocationPom[NUM_255_BYTES];     /* dot11APCivicLocationPom OCTET STRING */
} wlan_mib_Dot11ApCivicLocationEntry_stru;

/****************************************************************************************/
/* dot11RoamingConsortiumTable OBJECT-TYPE                                              */
/* SYNTAX SEQUENCE OF Dot11RoamingConsortiumEntry                                       */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "This is a Table of OIs which are to be transmitted in an ANQP Roaming          */
/*      Consortium ANQP-element. Each table entry corresponds to a roaming consor-      */
/*      tium or single SSP. The first 3 entries in this table are transmitted in        */
/*      Beacon and Probe Response frames."                                              */
/* ::= { dot11imt 5 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint8_t auc_dot11RoamingConsortiumOI[NUM_16_BYTES];        /* dot11RoamingConsortiumOI OCTET STRING */
    uint8_t en_dot11RoamingConsortiumRowStatus; /* wlan_mib_row_status_enum: dot11RoamingConsortiumRowStatus */
} wlan_mib_Dot11RoamingConsortiumEntry_stru;

/****************************************************************************************/
/* dot11DomainNameTable   OBJECT-TYPE                                                   */
/* SYNTAX                SEQUENCE OF Dot11DomainNameEntry                               */
/* MAX-ACCESS            not-accessible                                                 */
/* STATUS                current                                                        */
/* DESCRIPTION                                                                          */
/*      "This is a table of Domain Names which form the Domain Name list in Access      */
/*      Network Query Protocol. The Domain Name list may be transmitted to a non-       */
/*      AP STA in a GAS Response. Each table entry corresponds to a single Domain       */
/*      Name."                                                                          */
/* ::= { dot11imt 6 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint8_t auc_dot11DomainName[NUM_255_BYTES];                         /* dot11DomainName OCTET STRING */
    uint8_t en_dot11DomainNameRowStatus; /* wlan_mib_row_status_enum: dot11DomainNameRowStatus RowStatus */
    uint8_t auc_dot11DomainNameOui[5];                   /* dot11DomainNameOui OCTET STRING (SIZE(3..5)) */
} wlan_mib_Dot11DomainNameEntry_stru;

/****************************************************************************************/
/* dot11GASAdvertisementTable OBJECT-TYPE                                               */
/* SYNTAX SEQUENCE OF Dot11GASAdvertisementEntry                                        */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "This object is a table of GAS counters that allows for multiple instanti-      */
/*      ations of those counters on an STA."                                            */
/* ::= { dot11imt 7 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11GASAdvertisementId;                 /* dot11GASAdvertisementId Unsigned32 */
    oal_bool_enum_uint8 en_dot11GASPauseForServerResponse; /* dot11GASPauseForServerResponse TruthValue */
    uint32_t dot11GASResponseTimeout;                 /* dot11GASResponseTimeout Unsigned32 */
    uint32_t dot11GASComebackDelay;                   /* dot11GASComebackDelay Unsigned32 */
    uint32_t dot11GASResponseBufferingTime;           /* dot11GASResponseBufferingTime Unsigned32 */
    uint32_t dot11GASQueryResponseLengthLimit;        /* dot11GASQueryResponseLengthLimit Unsigned32 */
    uint32_t dot11GASQueries;                         /* dot11GASQueries Counter32 */
    uint32_t dot11GASQueryRate;                       /* dot11GASQueryRate Gauge32 */
    uint32_t dot11GASResponses;                       /* dot11GASResponses Counter32 */
    uint32_t dot11GASResponseRate;                    /* dot11GASResponseRate Gauge32 */
    uint32_t dot11GASTransmittedFragmentCount;        /* dot11GASTransmittedFragmentCount Counter32 */
    uint32_t dot11GASReceivedFragmentCount;           /* dot11GASReceivedFragmentCount Counter32 */
    uint32_t dot11GASNoRequestOutstanding;            /* dot11GASNoRequestOutstanding Counter32 */
    uint32_t dot11GASResponsesDiscarded;              /* dot11GASResponsesDiscarded Counter32 */
    uint32_t dot11GASFailedResponses;                 /* dot11GASFailedResponses Counter32 */
} wlan_mib_Dot11GASAdvertisementEntry_stru;

/****************************************************************************************/
/*          Start of dot11MSGCF OBJECT IDENTIFIER ::= { ieee802dot11 7}                 */
/*              -- MAC State GROUPS                                                     */
/*              -- dot11MACStateConfigTable ::= { dot11MSGCF 1 }                        */
/*              -- dot11MACStateParameterTable ::= { dot11MSGCF 2 }                     */
/*              -- dot11MACStateESSLinkTable ::= { dot11MSGCF 3 }                       */
/****************************************************************************************/
/****************************************************************************************/
/* dot11MACStateConfigTable OBJECT-TYPE                                                 */
/* SYNTAX SEQUENCE OF Dot11MACStateConfigEntry                                          */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "This table holds configuration parameters for the 802.11 MAC                   */
/*      State Convergence Function."                                                    */
/* ::= { dot11MSGCF 1 }                                                                 */
/****************************************************************************************/
typedef struct {
    uint32_t dot11ESSDisconnectFilterInterval;   /* dot11ESSDisconnectFilterInterval Unsigned32 */
    uint32_t dot11ESSLinkDetectionHoldInterval;  /* dot11ESSLinkDetectionHoldInterval Unsigned32 */
    uint8_t auc_dot11MSCEESSLinkIdentifier[38];  /* dot11MSCEESSLinkIdentifier Dot11ESSLinkIdentifier size 38 BYTE */
    uint8_t auc_dot11MSCENonAPStationMacAddress[WLAN_MAC_ADDR_LEN]; /* dot11MSCENonAPStationMacAddress MacAddress */
} wlan_mib_Dot11MACStateConfigEntry_stru;

/****************************************************************************************/
/* dot11MACStateParameterTable OBJECT-TYPE                                              */
/* SYNTAX SEQUENCE OF Dot11MACStateParameterEntry                                       */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS     current                                                                   */
/* DESCRIPTION                                                                          */
/*      "This table holds the current parameters used for each 802.11 network for       */
/*      802.11 MAC convergence functions."                                              */
/* ::= { dot11MSGCF 2 }                                                                 */
/****************************************************************************************/
typedef struct {
    uint32_t dot11ESSLinkDownTimeInterval;                      /* dot11ESSLinkDownTimeInterval Unsigned32 */
    uint32_t dot11ESSLinkRssiDataThreshold;                     /* dot11ESSLinkRssiDataThreshold Unsigned32 */
    uint32_t dot11ESSLinkRssiBeaconThreshold;                   /* dot11ESSLinkRssiBeaconThreshold Unsigned32 */
    uint32_t dot11ESSLinkDataSnrThreshold;                      /* dot11ESSLinkDataSnrThreshold Unsigned32 */
    uint32_t dot11ESSLinkBeaconSnrThreshold;                    /* dot11ESSLinkBeaconSnrThreshold Unsigned32 */
    uint32_t dot11ESSLinkBeaconFrameErrorRateThresholdInteger;
    uint32_t dot11ESSLinkBeaconFrameErrorRateThresholdFraction;
    uint32_t dot11ESSLinkBeaconFrameErrorRateThresholdExponent;
    uint32_t dot11ESSLinkFrameErrorRateThresholdInteger;        /* dot11ESSLinkFrameErrorRateThresholdInteger */
    uint32_t dot11ESSLinkFrameErrorRateThresholdFraction;       /* dot11ESSLinkFrameErrorRateThresholdFraction */
    uint32_t dot11ESSLinkFrameErrorRateThresholdExponent;       /* dot11ESSLinkFrameErrorRateThresholdExponent */
    uint32_t dot11PeakOperationalRate;                          /* dot11PeakOperationalRate Unsigned32 */
    uint32_t dot11MinimumOperationalRate;                       /* dot11MinimumOperationalRate Unsigned32 */
    uint32_t dot11ESSLinkDataThroughputInteger;                 /* dot11ESSLinkDataThroughputInteger Unsigned32 */
    uint32_t dot11ESSLinkDataThroughputFraction;                /* dot11ESSLinkDataThroughputFraction Unsigned32 */
    uint32_t dot11ESSLinkDataThroughputExponent;                /* dot11ESSLinkDataThroughputExponent Unsigned32 */
    uint8_t auc_dot11MSPEESSLinkIdentifier[38];                     /* dot11MSPEESSLinkIdentifier size 38 BYTE */
    uint8_t auc_dot11MSPENonAPStationMacAddress[WLAN_MAC_ADDR_LEN]; /* dot11MSPENonAPStationMacAddress MacAddress */
} wlan_mib_Dot11MACStateParameterEntry_stru;

/****************************************************************************************/
/* dot11MACStateESSLinkDetectedTable OBJECT-TYPE                                        */
/* SYNTAX SEQUENCE OF Dot11MACStateESSLinkDetectedEntry                                 */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "This table holds the detected 802.11 network list used for MAC conver-         */
/*      gence functions."                                                               */
/* ::= { dot11MSGCF 3 }                                                                 */
/****************************************************************************************/
typedef struct {
    uint32_t dot11ESSLinkDetectedIndex;                 /* dot11ESSLinkDetectedIndex Unsigned32 */
    int8_t ac_dot11ESSLinkDetectedNetworkId[NUM_255_BYTES];   /* dot11ESSLinkDetectedNetworkId OCTET STRING */
    uint32_t dot11ESSLinkDetectedNetworkDetectTime;     /* dot11ESSLinkDetectedNetworkDetectTime Unsigned32 */
    uint32_t dot11ESSLinkDetectedNetworkModifiedTime;   /* dot11ESSLinkDetectedNetworkModifiedTime Unsigned32 */
    uint8_t uc_dot11ESSLinkDetectedNetworkMIHCapabilities; /* dot11ESSLinkDetectedNetworkMIHCapabilities BITS */
    uint8_t auc_dot11MSELDEESSLinkIdentifier[38]; /* dot11MSELDEESSLinkIdentifier Dot11ESSLinkIdentifier size 38 BYTE */
    uint8_t auc_dot11MSELDENonAPStationMacAddress[WLAN_MAC_ADDR_LEN]; /* dot11MSELDENonAPStationMacAddress */
} wlan_mib_Dot11MACStateESSLinkDetectedEntry_stru;

typedef struct {
    /***************************************************************************
        dot11smt OBJECT IDENTIFIER ::= { ieee802dot11 1 }
    ****************************************************************************/
    /* --  dot11StationConfigTable ::= { dot11smt 1 } */
    wlan_mib_Dot11StationConfigEntry_stru st_wlan_mib_sta_config;

    /* --  dot11AuthenticationAlgorithmsTable ::= { dot11smt 2 } */
    /* --  dot11WEPDefaultKeysTable ::= { dot11smt 3 } */
    /* --  dot11WEPKeyMappingsTable ::= { dot11smt 4 } */
    /* --  dot11PrivacyTable ::= { dot11smt 5 } */
    wlan_mib_Dot11PrivacyEntry_stru st_wlan_mib_privacy;

    /* --  dot11SMTnotification ::= { dot11smt 6 } */
    /* --  dot11MultiDomainCapabilityTable ::= { dot11smt 7 } */
    /* --  dot11SpectrumManagementTable ::= { dot11smt 8 } */
    /* --  dot11RSNAConfigTable ::= { dot11smt 9 } */
    /* --  dot11RSNAConfigPairwiseCiphersTable ::= { dot11smt 10 } */
    /* --  dot11RSNAConfigAuthenticationSuitesTable      ::= { dot11smt 11 } */
    /* --  dot11RSNAStatsTable ::= { dot11smt 12 } */
    /* --  dot11OperatingClassesTable ::= { dot11smt 13 } */
    /* --  dot11RadioMeasurement ::= { dot11smt 14 } */
#if defined(_PRE_WLAN_FEATURE_11R)
    /* --  dot11FastBSSTransitionConfigTable ::= { dot11smt 15 } */
    wlan_mib_Dot11FastBSSTransitionConfigEntry_stru st_wlan_mib_fast_bss_trans_cfg;
#endif
    /* --  dot11LCIDSETable        ::= { dot11smt 16 } */
    /* --  dot11HTStationConfigTable  ::= { dot11smt 17 } */
    wlan_mib_Dot11HTStationConfigEntry_stru st_wlan_mib_ht_sta_cfg;

#if (defined(_PRE_WLAN_FEATURE_FTM)) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    /* --  dot11WirelessMgmtOptionsTable ::= { dot11smt 18} */
    wlan_mib_Dot11WirelessMgmtOptionsEntry_stru st_wlan_mib_wireless_mgmt_op;
#endif

    /* --  dot11LocationServicesNextIndex ::= { dot11smt 19} */
    /* --  dot11LocationServicesTable ::= { dot11smt 20} */
    /* --  dot11WirelessMGTEventTable ::= { dot11smt 21} */
    /* --  dot11WirelessNetworkManagement ::= { dot11smt 22} */
    /* --  dot11MeshSTAConfigTable ::= { dot11smt 23 } */
    /* --  dot11MeshHWMPConfigTable ::= { dot11smt 24 } */
    /* --  dot11RSNAConfigPasswordValueTable ::= { dot11smt 25 } */
    /* --  dot11RSNAConfigDLCGroupTable ::= { dot11smt 26 } */
    /* --  dot11VHTStationConfig ::= { dot11smt 31 } */
    wlan_mib_Dot11VHTStationConfigEntry_stru st_wlan_mib_vht_sta_config;

#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
    wlan_mib_Dot11HEStationConfigEntry_stru st_wlan_mib_he_sta_config;
    wlan_mid_Dot11HePhyCapability_stru st_wlan_mib_he_phy_config;
#endif
    /***************************************************************************
        dot11mac OBJECT IDENTIFIER ::= { ieee802dot11 2 }
    ****************************************************************************/
    /* --  dot11OperationTable ::= { dot11mac 1 } */
    wlan_mib_Dot11OperationEntry_stru st_wlan_mib_operation;
    /* --  dot11CountersTable ::= { dot11mac 2 } */
    /* --  dot11GroupAddressesTable ::= { dot11mac 3 } */
    /* --  dot11EDCATable ::= { dot11mac 4 } */
    wlan_mib_Dot11EDCAEntry_stru st_wlan_mib_edca;
    /* --  dot11QAPEDCATable ::= { dot11mac 5 } */
    /* --  dot11QosCountersTable ::= { dot11mac 6 } */
    /* --  dot11ResourceInfoTable    ::= { dot11mac 7 } */
    /* --  dot11SupportedMCSTxTable ::= { dot11phy 16 } */
    /* --  dot11SupportedMCSRxTable ::= { dot11phy 17 } */
    wlan_mib_Dot11SupportedMCSEntry_stru st_supported_mcs;

    /***************************************************************************
        dot11res OBJECT IDENTIFIER ::= { ieee802dot11 3 }
    ****************************************************************************/
    /* -- dot11resAttribute OBJECT IDENTIFIER ::= { dot11res 1 } */
    /***************************************************************************
        dot11phy OBJECT IDENTIFIER ::= { ieee802dot11 4 }
    ****************************************************************************/
    /* --  dot11PhyOperationTable ::= { dot11phy 1 } */
    /* --  dot11PhyAntennaTable ::= { dot11phy 2 } */
    wlan_mib_Dot11PhyAntennaEntry_stru st_wlan_mib_phy_antenna;

    /* --  dot11PhyTxPowerTable ::= { dot11phy 3 } */
    /* --  dot11PhyFHSSTable ::= { dot11phy 4 } */
    /* --  dot11PhyDSSSTable ::= { dot11phy 5 } */
    wlan_mib_Dot11PhyDSSSEntry_stru st_wlan_mib_phy_dsss;

    /* --  dot11PhyIRTable ::= { dot11phy 6 } */
    /* --  dot11RegDomainsSupportedTable ::= { dot11phy 7 } */
    /* --  dot11AntennasListTable ::= { dot11phy 8 } */
    /* --  dot11SupportedDataRatesTxTable ::= { dot11phy 9 } */
    /* --  dot11SupportedDataRatesRxTable ::= { dot11phy 10 } */
    /* --  dot11PhyOFDMTable ::= { dot11phy 11 } */
    /* --  dot11PhyHRDSSSTable ::= { dot11phy 12 } */
    /* --  dot11HoppingPatternTable ::= { dot11phy 13 } */
    /* --  dot11PhyERPTable ::= { dot11phy 14 } */
    /* --  dot11PhyHTTable  ::= { dot11phy 15 } */
    wlan_mib_Dot11PhyHTEntry_stru st_wlan_mib_phy_ht;

    /* --  dot11SupportedMCSTxTable ::= { dot11phy 16 } */
    /* --  dot11SupportedMCSRxTable ::= { dot11phy 17 } */
    /* --  dot11TransmitBeamformingConfigTable ::= { dot11phy 18 } */
    /* -- dot11PhyVHTTable ::= { dot11phy 23 } (802.11 ac) */
    wlan_mib_Dot11PhyVHTEntry_stru st_wlan_mib_phy_vht;

    /* -- dot11VHTTransmitBeamformingConfigTable ::= { dot11phy 24 }(802.11 ac) */
    /***************************************************************************
        dot11Conformance OBJECT IDENTIFIER ::= { ieee802dot11 5 } (该组用于归类，暂不实现)
    ****************************************************************************/
    /***************************************************************************
        dot11imt OBJECT IDENTIFIER ::= {ieee802dot11 6}
    ****************************************************************************/
    /* -- dot11BSSIdTable ::= { dot11imt 1 } */
    /* -- dot11InterworkingTable ::= { dot11imt 2 } */
    /* -- dot11APLCI ::= { dot11imt 3 } */
    /* -- dot11APCivicLocation ::= { dot11imt 4 } */
    /* -- dot11RoamingConsortiumTable      ::= { dot11imt 5 } */
    /* -- dot11DomainNameTable ::= { dot11imt 6 } */
    /* -- dot11GASAdvertisementTable       ::= { dot11imt 7 } */
    /***************************************************************************
        dot11MSGCF OBJECT IDENTIFIER ::= { ieee802dot11 7}
    ****************************************************************************/
    /* -- dot11MACStateConfigTable ::= { dot11MSGCF 1 }     */
    /* -- dot11MACStateParameterTable ::= { dot11MSGCF 2 }  */
    /* -- dot11MACStateESSLinkTable ::= { dot11MSGCF 3 }    */
} wlan_mib_ieee802dot11_stru;

#endif /* end of wlan_mib.h */
