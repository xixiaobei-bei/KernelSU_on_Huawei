
#ifndef __HMAC_HT_SELF_CURE_H_
#define __HMAC_HT_SELF_CURE_H_

#include "frw_ext_if.h"
#include "oal_ext_if.h"
#include "mac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_HT_SELF_CURE_H

#define HMAC_HT_BLACKLIST_MAX_NUM 16

typedef enum {
    HMAC_HT_SELF_CURE_EVENT_ASSOC         = 1,
    HAMC_HT_SELF_CURE_EVENT_RX_EAPOL      = 2,
    HMAC_HT_SELF_CURE_EVENT_TX_DHCP_FRAME = 3,
    HMAC_HT_SELF_CURE_EVENT_RX_DHCP_FRAME = 4,

    HMAC_HT_SELF_CURE_EVENT_BUTT
} hmac_ht_self_cure_event_enum;

typedef enum {
    HMAC_HT_SELF_CURE_STATE_INIT          = 0,
    HMAC_HT_SELF_CURE_STATE_ASSOC         = 1,
    HMAC_HT_SELF_CURE_STATE_HT_TX_DHCP    = 2,
    HMAC_HT_SELF_CURE_STATE_CLOSE_HT      = 3,
    HMAC_HT_SELF_CURE_STATE_11BG_ASSOC    = 4,
    HMAC_HT_SELF_CURE_STATE_11BG_TX_DHCP  = 5,

    HMAC_HT_SELF_CURE_STATE_BUTT
} hmac_ht_self_cure_state_enum;

typedef struct {
    uint8_t user_mac_addr[WLAN_MAC_ADDR_LEN];
    uint8_t rsv[2]; /* 2 resv字节数 */
} hmac_ht_blacklist_stru;

typedef struct {
    uint8_t ht_assoc_user[WLAN_MAC_ADDR_LEN];
    uint8_t check_ht_user[WLAN_MAC_ADDR_LEN]; /* 观察期的user */
    hmac_ht_self_cure_state_enum ht_self_cure_check_state;
    uint8_t tx_dhcp_times;
} hmac_ht_blacklist_checkinfo_stru;

typedef struct {
    hmac_ht_blacklist_stru ht_black_list[HMAC_HT_BLACKLIST_MAX_NUM];
    hmac_ht_blacklist_checkinfo_stru ht_check_info;
    uint8_t black_list_num;
    uint8_t check_flag;
    uint8_t rsv[2]; /* 2 resv字节数 */
} hmac_ht_self_cure_stru;

void  hmac_ht_self_cure_event_set(mac_vap_stru *mac_vap, uint8_t *user_mac, uint8_t type);
oal_bool_enum_uint8 hmac_ht_self_cure_in_blacklist(uint8_t *user_mac);
void hmac_ht_self_cure_need_check_flag_set(uint8_t flag);
uint32_t hmac_ht_self_cure_clear_blacklist(void);
#endif /* end of HMAC_HT_SELF_CURE_H */
