

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
typedef oal_uint8 hmac_wifi6_self_cure_handle_type_enum_uint8;

typedef enum {
    HMAC_WIFI6_SELF_CURE_BLACKLIST_TYPE_HTC   = 0,
    HMAC_WIFI6_SELF_CURE_BLACKLIST_TYPE_WIFI6 = 1,
} hmac_wifi6_self_cure_blacklist_type_enum;
typedef oal_uint8 hmac_wifi6_self_cure_blacklist_type_enum_uint8;

/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
typedef struct {
    oal_uint8 auc_user_mac_addr[WLAN_MAC_ADDR_LEN];
    hmac_wifi6_self_cure_blacklist_type_enum_uint8 en_blacklist_type;
    oal_uint8 uc_rsv;
} hmac_wifi6_self_cure_blacklist_stru;

typedef struct {
    hmac_wifi6_self_cure_blacklist_stru ast_wifi6_blacklist[HMAC_WIFI6_SELF_CURE_BLACK_LIST_MAX_NUM];
    oal_uint8 auc_current_probe_close_htc_user[WLAN_MAC_ADDR_LEN];
    oal_uint8 uc_blaclist_num;
    hmac_wifi6_self_cure_handle_type_enum_uint8 en_self_cure_type;
} hmac_wifi6_self_cure_info_stru;

/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
extern oal_uint32  hmac_wifi6_self_cure_black_list_set(mac_vap_stru *pst_mac_vap,
    oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  hmac_wifi6_self_cure_close_htc_handle(mac_vap_stru *pst_mac_vap);
extern oal_bool_enum_uint8 hmac_wifi6_self_cure_mac_is_wifi6_blacklist_type(oal_uint8 *puc_mac);
extern oal_bool_enum_uint8 hmac_wifi6_self_cure_mac_is_htc_blacklist_type(oal_uint8 *puc_mac);
extern oal_void hmac_wifi6_self_cure_init(hmac_vap_stru *pst_hmac_vap);
extern oal_void hmac_wifi6_self_cure_set_handle_type(hmac_wifi6_self_cure_handle_type_enum_uint8 en_type);
extern uint8_t hmac_wifi6_self_cure_get_handle_type(oal_void);

#endif
#endif /* end of __HMAC_WIFI6_SELF_CURE_H__ */

