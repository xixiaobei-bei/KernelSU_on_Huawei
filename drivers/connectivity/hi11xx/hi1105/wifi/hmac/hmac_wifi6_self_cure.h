

#ifndef __HMAC_WIFI6_SELF_CURE_H_
#define __HMAC_WIFI6_SELF_CURE_H_

#ifdef _PRE_WLAN_FEATURE_11AX
/* 1 其他头文件包含 */
#include "frw_ext_if.h"
#include "oal_ext_if.h"
#include "mac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_WIFI6_SELF_CURE_H

/* 2 宏定义 */
#define HMAC_WIFI6_SELF_CURE_BLACK_LIST_MAX_NUM 16

/* 3 枚举定义 */
typedef enum {
    HMAC_WIFI6_SELF_CURE_HANDLE_TYPE_DRIVER   = 0, /* 驱动处理 */
    HMAC_WIFI6_SELF_CURE_HANDLE_TYPE_APP      = 1, /* 上层处理 */
} mac_device_wifi6_blacklist_type_enum;

typedef enum {
    HMAC_WIFI6_SELF_CURE_BLACKLIST_TYPE_HTC   = 0,
    HMAC_WIFI6_SELF_CURE_BLACKLIST_TYPE_WIFI6 = 1,
} hmac_wifi6_self_cure_blacklist_type_enum;

typedef struct {
    uint8_t user_mac_addr[WLAN_MAC_ADDR_LEN];
    uint8_t blacklist_type; /* 枚举定义 hmac_wifi6_self_cure_blacklist_type_enum */
    uint8_t rsv;
} hmac_wifi6_self_cure_blacklist_stru;

typedef struct {
    hmac_wifi6_self_cure_blacklist_stru wifi6_blacklist[HMAC_WIFI6_SELF_CURE_BLACK_LIST_MAX_NUM];
    uint8_t current_probe_close_htc_user[WLAN_MAC_ADDR_LEN];
    uint8_t blaclist_num;
    uint8_t selfcure_type; /* 枚举定义 mac_device_wifi6_blacklist_type_enum */
} hmac_wifi6_self_cure_info_stru;

uint32_t  hmac_wifi6_self_cure_black_list_set(mac_vap_stru *p_mac_vap, uint8_t len, uint8_t *p_param);
uint32_t  hmac_wifi6_self_cure_close_htc_handle(mac_vap_stru *p_mac_vap);
void hmac_wifi6_close_htc_user_info(hmac_user_stru *hmac_user);
oal_bool_enum_uint8 hmac_wifi6_self_cure_mac_is_wifi6_blacklist_type(uint8_t *p_mac);
oal_bool_enum_uint8 hmac_wifi6_self_cure_mac_is_htc_blacklist_type(uint8_t *p_mac);
void hmac_wifi6_self_cure_init(hmac_vap_stru *p_hmac_vap);
void hmac_wifi6_self_cure_set_handle_type(uint8_t type);
uint8_t hmac_wifi6_self_cure_get_handle_type(void);

#endif
#endif /* end of __HMAC_WIFI6_SELF_CURE_H__ */

