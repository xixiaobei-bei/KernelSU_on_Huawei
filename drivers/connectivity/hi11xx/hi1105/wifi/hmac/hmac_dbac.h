

#ifndef __HMAC_DBAC_H__
#define __HMAC_DBAC_H__

/* 1 其他头文件包含 */
#include "hmac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DBAC_H
/* 2 宏定义 */
/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
typedef struct {
    frw_timeout_stru st_dbac_timer;
} hmac_dbac_handle_stru;

/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
uint32_t hmac_dbac_status_notify(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_go_flow_sta_channel_handle(mac_device_stru *mac_device);
#endif /* end of hmac_dbac.h */
