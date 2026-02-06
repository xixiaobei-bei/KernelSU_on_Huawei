
#ifdef _PRE_WLAN_FEATURE_TWT

#ifndef __HMAC_TWT_H__
#define __HMAC_TWT_H__

#include "mac_vap.h"
#include "mac_device.h"
#include "hmac_user.h"
#include "mac_frame_common.h"
#include "oam_wdk.h"
#include "hmac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TWT_H

#define HMAC_TWT_P2P_RUNNING_VAP_NUM    3

typedef enum {
    HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_0 = 0,
    HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_32 = 1,
    HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_48 = 2,
    HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_64 = 3,

    HMAC_NEXT_TWT_SUBFIELD_SIZE_BUTT
} hmac_next_twt_subfield_size_enum;

typedef struct {
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /* 用户的MAC ADDR */
    uint8_t uc_twt_setup_cmd;
    uint8_t uc_twt_flow_type;
    uint8_t uc_twt_flow_id;
    uint8_t twt_nego_type;
    uint8_t resv[NUM_3_BYTES];
    uint32_t twt_arg;               /* [setup_cmd/flow type/flow ID] */
    uint32_t twt_start_time_offset; /* us after TSF */
    uint32_t twt_exponent;          /* interval_exponent */
    uint32_t twt_duration;          /* wake_duration */
    uint32_t intrval_mantissa;      /* interval_mantissa */
} mac_cfg_twt_setup_req_param_stru;

typedef struct {
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /* 用户的MAC ADDR */
    uint8_t uc_twt_flow_id;                  /* flow ID */
    uint8_t twt_nego_type;
} mac_cfg_twt_teardown_req_param_stru;

/* host存储TWT管理参数 */
typedef struct {
    uint8_t category; /* ACTION的类别 */
    uint8_t action;   /* 不同ACTION类别下的分类 */
    uint8_t twt_setup_cmd;
    uint8_t twt_flow_type;
    uint8_t twt_flow_id; /* 单播和广播的twt id标识 */
    uint8_t twt_nego_type; /* 单播和广播的twt id标识 */
    uint64_t twt; /* us after TSF */
    uint32_t twt_exponent;          /* interval_exponent */
    uint32_t twt_duration;          /* wake_duration */
    uint32_t intrval_mantissa;      /* interval_mantissa */
} mac_twt_action_mgmt_args_stru; /* only host */

uint32_t hmac_mgmt_tx_twt_setup(hmac_vap_stru *pst_hmac_vap,
                                hmac_user_stru *pst_hmac_user,
                                mac_twt_action_mgmt_args_stru *pst_twt_action_args);
uint32_t hmac_mgmt_tx_twt_teardown(hmac_vap_stru *pst_hmac_vap,
                                   hmac_user_stru *pst_hmac_user,
                                   mac_twt_action_mgmt_args_stru *pst_twt_action_args);
uint32_t hmac_sta_rx_twt_setup_frame(hmac_vap_stru *pst_hmac_vap,
                                     hmac_user_stru *pst_hmac_user,
                                     uint8_t *puc_payload);
uint32_t hmac_sta_rx_twt_teardown_frame(hmac_vap_stru *pst_hmac_vap,
                                        hmac_user_stru *pst_hmac_user,
                                        uint8_t *puc_payload);
uint32_t hmac_sta_rx_twt_information_frame(hmac_vap_stru *pst_hmac_vap,
                                           hmac_user_stru *pst_hmac_user,
                                           uint8_t *puc_payload);
uint32_t hmac_dbac_teardown_twt_session(mac_device_stru *pst_dev);
uint32_t hmac_twt_auto_setup_session(mac_device_stru *pst_device);
uint32_t hmac_get_chip_vap_num(mac_chip_stru *pst_chip);
uint32_t hmac_config_twt_setup_req_auto(mac_vap_stru *pst_mac_vap);
uint32_t hmac_config_twt_teardown_req_auto(mac_vap_stru *pst_mac_vap);
uint32_t hmac_config_twt_setup_req(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_twt_teardown_req(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_proc_rx_process_twt_sync_event_tx_adapt(frw_event_mem_stru *pst_event_mem);
uint32_t mac_device_find_up_sta_wlan(mac_device_stru *pst_mac_device,
                                     mac_vap_stru **ppst_mac_vap);
void hmac_sta_rx_btwt(uint8_t *payload, uint16_t msg_len, hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);

#endif

#endif /* end of hmac_twt.h */
