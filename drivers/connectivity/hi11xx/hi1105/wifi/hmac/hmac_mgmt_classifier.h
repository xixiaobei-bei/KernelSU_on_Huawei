

#ifndef __HMAC_MGMT_CLASSIFIER_H__
#define __HMAC_MGMT_CLASSIFIER_H__

#include "frw_ext_if.h"
#include "hmac_vap.h"
#include "host_hal_ops.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MGMT_CLASSIFIER_H

#ifdef _PRE_WLAN_FEATURE_MONITOR

#define SHIFT_4_BITS 4
#define ARRAY_NUM_0  0
#define ARRAY_NUM_1  1
#define ARRAY_NUM_2  2
#define ARRAY_NUM_3  3
#define ARRAY_NUM_4  4
#define ARRAY_NUM_5  5
/* radio tap HE field definition for data2. HE has data1 ~ data6, each for 2 bytes */
#define RADIO_TAP_HE_DATA2_PRI_SEC_80MHZ_KNOWN 1
#define RADIO_TAP_HE_DATA2_GI_KNOWN            2
/* radio tap HE field definition for data3. HE has data1 ~ data6, each for 2 bytes */
#define RADIO_TAP_HE_DATA3_MCS                 8
#define RADIO_TAP_HE_DATA3_LDPC                13
#define RADIO_TAP_HE_DATA3_STBC                15

void hmac_sniffer_fill_radiotap(ieee80211_radiotap_stru *radiotap, mac_rx_ctl_stru *rx_ctrl,
                                hal_sniffer_rx_status_stru *rx_status,
                                hal_sniffer_rx_statistic_stru *sniffer_rx_statistic,
                                uint8_t *mac_hdr, uint32_t *rate_kbps, hal_statistic_stru *per_rate);
#endif

uint32_t hmac_rx_process_mgmt_event(frw_event_mem_stru *event_mem);
uint32_t hmac_rx_process_wow_mgmt_event(frw_event_mem_stru *event_mem);
uint32_t hmac_mgmt_rx_delba_event(frw_event_mem_stru *event_mem);
uint32_t hmac_mgmt_tx_action(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
                             mac_action_mgmt_args_stru *action_args);
uint32_t hmac_mgmt_tbtt_event(frw_event_mem_stru *event_mem);
uint32_t hmac_mgmt_send_disasoc_deauth_event(frw_event_mem_stru *event_mem);
uint32_t hmac_proc_disasoc_misc_event(frw_event_mem_stru *event_mem);

uint32_t hmac_proc_roam_trigger_event(frw_event_mem_stru *event_mem);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_mgmt_classifier.h */
