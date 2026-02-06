

#ifndef __HMAC_BLAKLIST_H__
#define __HMAC_BLAKLIST_H__

#include "oal_ext_if.h"
#include "mac_vap.h"
#include "hmac_vap.h"

#define CS_INVALID_AGING_TIME 0
#define CS_DEFAULT_AGING_TIME 3600
#define CS_DEFAULT_RESET_TIME 3600
#define CS_DEFAULT_THRESHOLD  100

/* 黑名单类型 */
typedef enum {
    CS_BLACKLIST_TYPE_ADD, /* 增加       */
    CS_BLACKLIST_TYPE_DEL, /* 删除       */
    CS_BLACKLIST_TYPE_BUTT
} cs_blacklist_type_enum;

/* 自动黑名单配置参数 */
typedef struct {
    uint8_t uc_enabled;      /* 使能标志 0:未使能  1:使能 */
    uint8_t auc_reserved[BYTE_OFFSET_3]; /* 字节对齐                  */
    uint32_t threshold;   /* 门限                      */
    uint32_t reset_time;  /* 重置时间                  */
    uint32_t aging_time;  /* 老化时间                  */
} hmac_autoblacklist_cfg_stru;

/* 黑白名单配置 */
typedef struct {
    uint8_t uc_type;   /* 配置类型    */
    uint8_t uc_mode;   /* 配置模式    */
    uint8_t auc_sa[WLAN_MAC_ADDR_LEN]; /* mac地址     */
} hmac_blacklist_cfg_stru;

uint32_t hmac_autoblacklist_enable(mac_vap_stru *mac_vap, uint8_t enabled);
uint32_t hmac_autoblacklist_set_aging(mac_vap_stru *mac_vap, uint32_t aging_time);
uint32_t hmac_autoblacklist_set_threshold(mac_vap_stru *mac_vap, uint32_t threshold);
uint32_t hmac_autoblacklist_set_reset_time(mac_vap_stru *mac_vap, uint32_t reset_time);
void hmac_autoblacklist_filter(mac_vap_stru *mac_vap, uint8_t *mac_addr);
uint32_t hmac_blacklist_set_mode(mac_vap_stru *mac_vap, uint8_t mode);
uint32_t hmac_blacklist_get_mode(mac_vap_stru *mac_vap, uint8_t *mode);
uint32_t hmac_blacklist_add(mac_vap_stru *mac_vap, uint8_t *mac_addr, uint32_t aging_time);
uint32_t hmac_blacklist_add_only(mac_vap_stru *mac_vap, uint8_t *mac_addr, uint32_t aging_time);
uint32_t hmac_blacklist_del(mac_vap_stru *mac_vap, uint8_t *mac_addr);
oal_bool_enum_uint8 hmac_blacklist_filter(mac_vap_stru *mac_vap, uint8_t *mac_addr);
void hmac_show_blacklist_info(mac_vap_stru *mac_vap);
uint32_t hmac_backlist_get_drop(mac_vap_stru *mac_vap, const uint8_t *mac_addr);
uint8_t hmac_backlist_get_list_num(mac_vap_stru *mac_vap);
oal_bool_enum_uint8 hmac_blacklist_get_assoc_ap(mac_vap_stru *cur_vap, uint8_t *mac_addr,
    mac_vap_stru **assoc_vap);

#endif /* end of hmac_blacklist.h */
