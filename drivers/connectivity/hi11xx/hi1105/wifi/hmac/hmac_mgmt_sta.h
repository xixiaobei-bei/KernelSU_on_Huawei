

#ifndef __HMAC_MGMT_STA_H__
#define __HMAC_MGMT_STA_H__

#include "oal_ext_if.h"
#include "hmac_vap.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_mgmt_join.h"
#include "hmac_cali_mgmt.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MGMT_STA_H

#define HMAC_WMM_QOS_PARAMS_HDR_LEN   8
#define HMAC_WMM_QOSINFO_AND_RESV_LEN 2
#define HMAC_WMM_AC_PARAMS_RECORD_LEN 4
#define DATARATES_80211G_NUM          12

/* 去关联原因 */
typedef struct {
    hmac_mgmt_status_enum_uint8 en_disasoc_reason_code;
    uint8_t auc_resv[3];
} hmac_disasoc_rsp_stru;

typedef struct {
    hmac_mgmt_status_enum_uint8 en_result_code;
    uint8_t auc_resv[3];
} hmac_ap_start_rsp_stru;

/* 与dmac层的hi11xx_date_rate_stru g_ast_hi11xx_rates_11g同步修改 */
typedef struct {
    uint8_t uc_expand_rate;         /* 扩展速率集 */
    uint8_t uc_mac_rate;            /* MAC对应速率 */
    uint8_t uc_hal_wlan_rate_index; /* 速率index */
    uint8_t uc_rsv;
} hmac_data_rate_stru;

uint32_t hmac_sta_wait_asoc_rx(hmac_vap_stru *pst_sta, void *pst_msg);
uint32_t hmac_sta_auth_timeout(hmac_vap_stru *pst_hmac_sta, void *p_param);
uint32_t hmac_sta_wait_asoc_timeout(hmac_vap_stru *pst_hmac_sta, void *p_param);
uint32_t hmac_sta_not_up_rx_mgmt(hmac_vap_stru *pst_hmac_vap_sta, void *p_param);
void hmac_sta_set_txopps_partial_aid(mac_vap_stru *pst_mac_vap);
uint32_t hmac_process_assoc_rsp(hmac_vap_stru *pst_hmac_sta, hmac_user_stru *pst_hmac_user, uint8_t *puc_mac_hdr,
    uint16_t us_hdr_len, uint8_t *puc_payload, uint16_t us_msg_len);
uint8_t *hmac_sta_find_ie_in_probe_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_eid, uint16_t *pus_index);
void hmac_sta_get_min_rate(dmac_set_rate_stru *pst_rate_params, hmac_join_req_stru *pst_join_req);
void hmac_mgmt_sta_roam_rx_action(hmac_vap_stru *p_hmac_vap, oal_netbuf_stru *p_netbuf,
    oal_bool_enum_uint8 en_is_protected);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_mgmt_sta.h */