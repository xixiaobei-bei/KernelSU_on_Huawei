

#ifndef __HMAC_RX_FILTER_H__
#define __HMAC_RX_FILTER_H__

/* 1 其他头文件包含 */
#include "oal_types.h"
#include "mac_vap.h"
#include "mac_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RX_FILTER_H
/* 2 宏定义 */
/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
oal_bool_enum_uint8 hmac_find_is_sta_up(mac_device_stru *pst_mac_device);

uint32_t hmac_calc_up_ap_num(mac_device_stru *pst_mac_device);
uint32_t hmac_find_up_vap(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_rx_filter.h */
