
#ifndef __HMAC_HT_SELF_CURE_H_
#define __HMAC_HT_SELF_CURE_H_

/* 1 其他头文件包含 */
#include "frw_ext_if.h"
#include "oal_ext_if.h"
#include "mac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_HT_SELF_CURE_H

#define HMAC_HT_BLACKLIST_MAX_NUM 16

typedef enum
{
    HMAC_HT_SELF_CURE_EVENT_ASSOC         = 1,
    HMAC_HT_SELF_CURE_EVENT_TX_DHCP_FRAME = 2,
    HMAC_HT_SELF_CURE_EVENT_RX_DHCP_FRAME = 3,

    HMAC_HT_SELF_CURE_EVENT_BUTT
} hmac_ht_self_cure_event_enum;
typedef uint8_t hmac_ht_self_cure_event_enum_uint8;

typedef enum
{
    HMAC_HT_SELF_CURE_STATE_INIT          = 0,
    HMAC_HT_SELF_CURE_STATE_ASSOC         = 1,
    HMAC_HT_SELF_CURE_STATE_HT_TX_DHCP    = 2,
    HMAC_HT_SELF_CURE_STATE_CLOSE_HT      = 3,
    HMAC_HT_SELF_CURE_STATE_11BG_ASSOC    = 4,
    HMAC_HT_SELF_CURE_STATE_11BG_TX_DHCP  = 5,

    HMAC_HT_SELF_CURE_STATE_BUTT
} hmac_ht_self_cure_state_enum;
typedef uint8_t hmac_ht_self_cure_state_enum_uint8;

typedef struct {
    uint8_t userMacAddr[WLAN_MAC_ADDR_LEN];
    uint8_t rsc[2];
} hmac_ht_blacklist_stru;

typedef struct {
    uint8_t htAssocUser[WLAN_MAC_ADDR_LEN];
    uint8_t checkHtUser[WLAN_MAC_ADDR_LEN]; /* 观察期的user */
    hmac_ht_self_cure_state_enum_uint8 htSelfCureCheckState;
    uint8_t txDhcpTimes;
} hmac_ht_blacklist_checkinfo_stru;

typedef struct {
    hmac_ht_blacklist_stru htBlackList[HMAC_HT_BLACKLIST_MAX_NUM];
    hmac_ht_blacklist_checkinfo_stru htCheckInfo;
    uint8_t blackListNum;
    uint8_t needCheckFlag;
    uint8_t rsv[2];
} hmac_ht_self_cure_stru;

extern oal_void  hmac_ht_self_cure_event_set(mac_vap_stru *p_mac_vap, uint8_t *userMac, uint8_t type);
extern oal_bool_enum_uint8 hmac_ht_self_cure_in_blacklist(uint8_t *p_UserMac);
extern oal_void hmac_ht_self_cure_need_check_flag_set(uint8_t flag);

#endif /* end of HMAC_HT_SELF_CURE_H */
