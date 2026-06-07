

#ifndef __HMAC_TX_OPT_H__
#define __HMAC_TX_OPT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 头文件包含 */
#include "oal_ext_if.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TX_OPT_H

/* 2 宏定义 */
#define MAX_TX_OPT_SWITCH_CNT 3

/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
extern oal_void hmac_tx_opt_switch(oal_uint32 ul_tx_large_pps);
extern oal_void hmac_set_tx_opt_switch_cnt(oal_uint8 uc_opt_switch_cnt);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif
