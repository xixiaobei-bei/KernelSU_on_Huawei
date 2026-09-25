

#ifndef __HMAC_ARP_PROBEH__
#define __HMAC_ARP_PROBEH__

/* 1 其他头文件包含 */
#include "frw_ext_if.h"
#include "oal_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ARP_PROBE_H

/* 2 宏定义 */
#define ARP_PROBE_FAIL_REASSOC_NUM 9 /* 9*300=2.7秒 */
#define ARP_PROBE_TIMEOUT    300  // ms
/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
extern oal_void hmac_arp_probe_type_set(oal_void *p_arg, oal_bool_enum_uint8 en_arp_detect_on,
    oal_uint8 en_probe_type);
extern oal_void hma_arp_probe_timer_start(oal_netbuf_stru *pst_netbuf, oal_void *p_arg);
extern oal_void hmac_arp_probe_init(oal_void *p_hmac_vap, oal_void *p_hmac_user);
extern oal_void hmac_arp_probe_destory(oal_void *p_hmac_vap, oal_void *p_hmac_user);
extern oal_uint32 hmac_ps_rx_amsdu_arp_probe_process(mac_vap_stru *pst_mac_vap,
    oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_bool_enum_uint8 hmac_arp_probe_fail_reassoc_trigger(oal_void *p_hmac_vap, oal_void *p_hmac_user);
#endif /* end of __HMAC_M2S_H__ */

