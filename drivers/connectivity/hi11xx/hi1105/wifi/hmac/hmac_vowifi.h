

#ifndef __HMAC_VOWIFI_H__
#define __HMAC_VOWIFI_H__

#include "mac_vap.h"
#include "oam_wdk.h"
#include "mac_vap_common.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_VOWIFI_H

/* VoWiFi相关参数的宏定义 */
#define MAC_VOWIFI_PERIOD_MIN         1  /* 单位s */
#define MAC_VOWIFI_PERIOD_MAX         30 /* 单位s */
#define MAC_VOWIFI_TRIGGER_COUNT_MIN  1
#define MAC_VOWIFI_TRIGGER_COUNT_MAX  100
#define MAC_VOWIFI_LOW_THRESHOLD_MIN  -100
#define MAC_VOWIFI_LOW_THRESHOLD_MAX  -1
#define MAC_VOWIFI_HIGH_THRESHOLD_MIN -100
#define MAC_VOWIFI_HIGH_THRESHOLD_MAX -1

#define MAC_VAP_VOWIFI_MODE_DEFAULT          VOWIFI_DISABLE_REPORT
#define MAC_VAP_VOWIFI_TRIGGER_COUNT_DEFAULT 5
#define MAC_VAP_VOWIFI_PERIOD_DEFAULT_MS     1000 /* 单位ms */
#define MAC_VAP_VOWIFI_HIGH_THRES_DEFAULT    -65
#define MAC_VAP_VOWIFI_LOW_THRES_DEFAULT     -80

void mac_vap_init_vowifi(mac_vap_stru *pst_vap, mac_cfg_add_vap_param_stru *pst_param);
void mac_vap_vowifi_init(mac_vap_stru *pst_mac_vap);
void mac_vap_vowifi_exit(mac_vap_stru *pst_mac_vap);
uint32_t mac_vap_set_vowifi_param(mac_vap_stru *pst_mac_vap, mac_vowifi_cmd_enum_uint8 en_vowifi_cfg_cmd,
                                  uint8_t uc_value);
uint32_t hmac_config_vowifi_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_set_vowifi_nat_keep_alive_params(mac_vap_stru *pst_mac_vap,
                                                      uint16_t us_len, uint8_t *puc_param);
uint32_t hmac_config_vowifi_report(mac_vap_stru *pst_mac_vap,
                                   uint8_t uc_len, uint8_t *puc_param);
#endif /* end of hmac_vowifi.h */
