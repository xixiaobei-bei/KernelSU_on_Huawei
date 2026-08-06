

#ifndef __HMAC_BTCOEX_H__
#define __HMAC_BTCOEX_H__
/* 1 其他头文件包含 */
#include "frw_ext_if.h"
#include "oal_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_BTCOEX_H

/* 2 宏定义 */
#define BTCOEX_BSS_NUM_IN_BLACKLIST 16
#define BTCOEX_ARP_FAIL_DELBA_NUM   2
#define BTCOEX_ARP_FAIL_REASSOC_NUM 4
#define BTCOEX_ARP_PROBE_TIMEOUT    (300)  // ms

/* 3 枚举定义 */
typedef enum {
    BTCOEX_BLACKLIST_TPYE_FIX_BASIZE = 0,
    BTCOEX_BLACKLIST_TPYE_NOT_AGGR = 1,

    BTCOEX_BLACKLIST_TPYE_BUTT
} btcoex_blacklist_type_enum;
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
typedef struct {
    uint16_t us_last_baw_start;                       /* 上一次接收到ADDBA REQ中的baw_start值 */
    uint16_t us_last_seq_num;                         /* 上一次接收到ADDBA REQ中的seq_num值 */
    uint8_t en_blacklist_tpye; /* 黑名单方案 */
    oal_bool_enum_uint8 en_ba_handle_allow;          /* 黑名单方案1:是否允许共存删建聚合 黑名单方案2:是否允许建立聚合 */
} hmac_btcoex_addba_req_stru;

typedef struct {
    uint8_t auc_user_mac_addr[WLAN_MAC_ADDR_LEN]; /* 黑名单MAC地址 */
    uint8_t uc_type;                              /* 写入黑名单的类型 */
    uint8_t uc_used;                              /* 是否已经写过黑名单MAC地址 */
} hmac_btcoex_delba_exception_stru;
typedef struct {
    hmac_btcoex_delba_exception_stru btcoex_delba_exception[BTCOEX_BSS_NUM_IN_BLACKLIST];
    uint8_t uc_exception_bss_index; /* 黑名单MAC地址的数组下标 */
    uint8_t auc_resv[3]; /* 4字节对齐补3字节 */
} hmac_device_btcoex_stru;

typedef struct {
    hmac_btcoex_addba_req_stru st_hmac_btcoex_addba_req;
    uint16_t us_ba_size;
    oal_bool_enum_uint8 en_delba_btcoex_trigger; /* 是否btcoex触发删建BA */
    uint8_t res;
} hmac_user_btcoex_stru;

#define HMAC_BTCOEX_GET_BLACKLIST_TYPE(_pst_hmac_user) \
    ((_pst_hmac_user)->st_hmac_user_btcoex.st_hmac_btcoex_addba_req.en_blacklist_tpye)

#define HMAC_BTCOEX_GET_BLACKLIST_DELBA_HANDLE_ALLOW(_pst_hmac_user) \
    ((_pst_hmac_user)->st_hmac_user_btcoex.st_hmac_btcoex_addba_req.en_ba_handle_allow)

/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
void hmac_btcoex_check_rx_same_baw_start_from_addba_req(void *p_arg, void *p_arg1,
    mac_ieee80211_frame_stru *pst_frame_hdr, uint8_t *puc_action, uint32_t frame_body_len);
uint32_t hmac_btcoex_check_exception_in_list(void *p_arg, uint8_t *auc_addr);
void hmac_btcoex_blacklist_handle_init(void *p_arg);
uint8_t hmac_btcoex_check_addba_req(void *vap, void *user);
void hmac_btcoex_set_addba_rsp_ba_param(
    void *vap, void *user, void *ba_rx_info, uint16_t *ba_param);
uint32_t hmac_btcoex_d2h_bt_state_syn(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
#endif /* end of __HMAC_BTCOEX_H__ */

