

#ifndef __HMAC_RESOURCE_H__
#define __HMAC_RESOURCE_H__

#include "oal_types.h"
#include "oal_queue.h"
#include "mac_resource.h"
#include "mac_device.h"
#include "hmac_device.h"
#include "hmac_main.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RESOURCE_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 存储hmac device结构的资源结构体 */
typedef struct {
    hmac_device_stru ast_hmac_dev_info[MAC_RES_MAX_DEV_NUM];
    oal_queue_stru st_queue;
    unsigned long aul_idx[MAC_RES_MAX_DEV_NUM];
    uint8_t auc_user_cnt[MAC_RES_MAX_DEV_NUM];
    uint8_t auc_resv[3]; /* 保留3个字节 */
} hmac_res_device_stru;

uint32_t hmac_res_alloc_mac_dev(uint32_t dev_idx);
uint32_t hmac_res_free_mac_dev(uint32_t dev_idx);
hmac_device_stru *hmac_res_get_mac_dev(uint32_t dev_idx);
mac_chip_stru *hmac_res_get_mac_chip(uint32_t chip_idx);
uint32_t hmac_res_init(void);
uint32_t hmac_res_exit(mac_board_stru *hmac_board);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_resource.h */
