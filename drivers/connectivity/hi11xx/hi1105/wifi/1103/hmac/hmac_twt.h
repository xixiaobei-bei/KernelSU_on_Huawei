

#ifndef __HMAC_TWT_H__
#define __HMAC_TWT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "hmac_fsm.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TWT_H

/* 1 其他头文件包含 */
/* 2 宏定义 */
/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
/* 8 UNION定义 */
#define HMAC_TWT_P2P_RUNNING_VAP_NUM    3

typedef enum {
    HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_0 = 0,
    HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_32 = 1,
    HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_48 = 2,
    HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_64 = 3,

    HMAC_NEXT_TWT_SUBFIELD_SIZE_BUTT
} hmac_next_twt_subfield_size_enum;

/* 9 OTHERS定义 */
/* 10 函数声明 */
#ifdef _PRE_WLAN_FEATURE_TWT
extern oal_uint32 hmac_dbac_teardown_twt_session(mac_device_stru *pst_dev);
extern oal_uint32 hmac_twt_auto_setup_session(mac_device_stru *pst_device);
#endif /* _PRE_WLAN_FEATURE_P2P */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_twt.h */
