

#ifndef __HMAC_DFX_H__
#define __HMAC_DFX_H__


#ifdef _PRE_WLAN_1103_CHR
#include "mac_frame.h"
#include "dmac_ext_if.h"
#include "hmac_vap.h"
#include "wlan_types.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DFX_H

/* 1 头文件包含 */
/* 2 宏定义 */
#define HMAC_CHR_NETBUF_ALLOC_SIZE  512
/* chr周期上报数据事件的频次:30s */
#define HMAC_CHR_REPORT_INTERVAL 30000

/* 3 枚举定义 */
#ifdef _PRE_WLAN_1103_CHR
typedef enum {
    HMAC_CHR_ROAM_SUCCESS = 0,
    HMAC_CHR_ROAM_SCAN_FAIL = 1,
    HMAC_CHR_ROAM_HANDSHAKE_FAIL = 2,
    HMAC_CHR_ROAM_CONNECT_FAIL = 3,
    HMAC_CHR_ROAM_TIMEOUT_FAIL = 4,
    HMAC_CHR_ROAM_START = 5,

    HMAC_CHR_ROAM_REASON_BUTT
} hmac_chr_roam_fail_reason;
typedef oal_uint8 roam_fail_reason_enum_uint8;

typedef enum {
    HMAC_CHR_ROAM_NORMAL = 0,
    HMAC_CHR_OVER_DS = 1,
    HMAC_CHR_OVER_THE_AIR = 2,

    HMAC_CHR_ROAM_MODE_BUTT
} hmac_chr_roam_mode;
typedef oal_uint8 roam_mode_enum_uint8;

typedef enum {
    HMAC_CHR_NON_11K_SCAN = 0,
    HMAC_CHR_11K_SCAN = 1,

    HMAC_CHR_SCAN_MODE_BUTT
} hmac_chr_scan_mode;
typedef oal_uint8 scan_mode_enum_uint8;
#endif

/* 4 全局变量声明 */
#ifdef _PRE_WLAN_1103_CHR
typedef struct {
    oal_uint8 uc_vap_state;
    oal_uint8 uc_vap_num;
    oal_uint8 uc_protocol;
    oal_uint8 uc_vap_rx_nss;
    oal_uint8 uc_ap_protocol_mode;
    oal_uint8 uc_ap_spatial_stream_num;
    oal_uint8 bit_ampdu_active : 1;
    oal_uint8 bit_amsdu_active : 1;
    oal_uint8 bit_is_dbac_running : 1;
    oal_uint8 bit_is_dbdc_running : 1;
    oal_uint8 bit_sta_11ntxbf : 1;
    oal_uint8 bit_ap_11ntxbf : 1;
    oal_uint8 bit_ap_qos : 1;
    oal_uint8 bit_ap_1024qam_cap : 1;
} hmac_chr_vap_info_stru;

typedef struct tag_hmac_chr_ba_info_stru {
    oal_uint8 uc_ba_num;
    oal_uint8 uc_del_ba_tid;
    mac_reason_code_enum_uint16 en_del_ba_reason;
} hmac_chr_del_ba_info_stru;

typedef struct tag_hmac_chr_disasoc_reason_stru {
    oal_uint16 us_user_id;
    dmac_disasoc_misc_reason_enum_uint16 en_disasoc_reason;
} hmac_chr_disasoc_reason_stru;

typedef struct tag_hamc_chr_info {
    hmac_chr_disasoc_reason_stru st_disasoc_reason;
    hmac_chr_del_ba_info_stru st_del_ba_info;
    hmac_chr_vap_info_stru st_vap_info;
    oal_uint16 us_connect_code;
    oal_uint8 _resv[2];
} hmac_chr_info;

typedef struct tag_hmac_chr_connect_fail_report_stru {
    oal_int32 ul_snr;
    oal_int32 ul_noise;  /* 底噪 */
    oal_int32 ul_chload; /* 信道繁忙程度 */
    oal_int8 c_signal;
    oal_uint8 uc_distance; /* 算法的tpc距离，对应dmac_alg_tpc_user_distance_enum */
    oal_uint8 uc_cca_intr; /* 算法的cca_intr干扰，对应alg_cca_opt_intf_enum */
    oal_uint16 us_err_code;
    oal_uint8 _resv[2];
} mac_chr_connect_fail_report_stru;

typedef struct {
    oal_uint8 uc_trigger;
    oal_uint8 uc_roam_result;
    oal_uint8 uc_scan_mode;
    oal_uint8 uc_roam_mode;
    oal_uint32 uc_scan_time;
    oal_uint32 uc_connect_time;
    oal_uint8 _resv[4];
} hmac_chr_roam_info_stru;

typedef struct {
    oal_uint8 auc_src_bssid[WLAN_MAC_ADDR_LEN];
    oal_uint8 uc_src_channel;
    oal_int8 uc_src_rssi;
    oal_uint8 auc_target_bssid[WLAN_MAC_ADDR_LEN];
    oal_uint8 uc_target_channel;
    oal_int8 uc_target_rssi;
    oal_uint8 uc_roam_mode;
    oal_uint8 uc_roam_stage;
    oal_uint8 uc_roam_result;
    oal_uint8 uc_roam_type;
    oal_uint32 uc_roam_time;
} hmac_chr_roam_connect_info_stru;
#endif

extern oal_uint32 hmac_dfx_init_etc(void);
extern oal_uint32 hmac_dfx_exit_etc(void);

#ifdef _PRE_WLAN_1103_CHR
hmac_chr_disasoc_reason_stru *hmac_chr_disasoc_reason_get_pointer(void);
oal_uint16 *hmac_chr_connect_code_get_pointer(void);
hmac_chr_del_ba_info_stru *hmac_chr_ba_info_get_pointer(void);
oal_void hmac_chr_info_clean(void);
oal_void hmac_chr_set_disasoc_reason(oal_uint16 user_id, oal_uint16 reason_id);
oal_void hmac_chr_get_disasoc_reason(hmac_chr_disasoc_reason_stru *pst_disasoc_reason);
oal_void hmac_chr_set_del_ba_info(oal_uint8 uc_tid, oal_uint16 reason_id);
oal_void hmac_chr_get_del_ba_info(mac_vap_stru *pst_mac_vap, hmac_chr_del_ba_info_stru *pst_del_ba_reason);
oal_void hmac_chr_set_ba_session_num(oal_uint8 uc_ba_num);
oal_void hmac_chr_set_connect_code(oal_uint16 connect_code);
oal_void hmac_chr_get_connect_code(oal_uint16 *pus_connect_code);
oal_void hmac_chr_get_vap_info(mac_vap_stru *pst_mac_vap, hmac_chr_vap_info_stru *pst_vap_info);
oal_uint32 hmac_chr_get_chip_info(oal_uint32 chr_event_id);
oal_uint32 hmac_get_chr_info_event_hander(oal_uint32 chr_event_id);
oal_void hmac_chr_connect_fail_query_and_report(hmac_vap_stru *pst_hmac_vap,
    mac_status_code_enum_uint16 connet_code);

#endif


#endif /* end of hmac_dfx.h */
