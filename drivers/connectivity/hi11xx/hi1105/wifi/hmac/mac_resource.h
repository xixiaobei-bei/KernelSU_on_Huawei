

#ifndef __MAC_RESOURCE_H__
#define __MAC_RESOURCE_H__

#include "oal_ext_if.h"
#include "oal_queue.h"
#include "mac_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_RESOURCE_H

/* 最大dev数量 */
#define MAC_RES_MAX_DEV_NUM     WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC

#define MAC_RES_MAX_USER_LIMIT WLAN_USER_MAX_USER_LIMIT

typedef struct {
    unsigned long user_idx_size;
} mac_res_user_idx_size_stru;

typedef struct {
    uint8_t user_cnt_size;
} mac_res_user_cnt_size_stru;

typedef struct {
    mac_device_stru ast_dev_info[MAC_RES_MAX_DEV_NUM];
    oal_queue_stru st_queue;
    unsigned long aul_idx[MAC_RES_MAX_DEV_NUM];
    uint8_t auc_user_cnt[MAC_RES_MAX_DEV_NUM];
    uint8_t auc_resv[NUM_2_BYTES];
} mac_res_device_stru;

typedef struct {
    void *past_vap_info[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
    oal_queue_stru st_queue;
    unsigned long aul_idx[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
    uint16_t us_hmac_priv_size;
    uint8_t auc_user_cnt[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
    uint8_t auc_resv[NUM_2_BYTES];
} mac_res_vap_stru;

typedef struct {
    void *past_user_info[MAC_RES_MAX_USER_LIMIT];
    oal_queue_stru_16 st_queue;
    unsigned long *pul_idx;
    uint16_t us_hmac_priv_size;
    uint8_t auc_resv[NUM_2_BYTES];
    uint8_t *puc_user_cnt;
} mac_res_user_stru;

typedef struct {
    mac_res_device_stru st_dev_res;
    mac_res_vap_stru st_vap_res;
    mac_res_user_stru st_user_res;
} mac_res_stru;

extern mac_res_stru *g_pst_mac_res;

/* 10 函数声明 */
uint32_t mac_res_alloc_hmac_dev(uint8_t *dev_idx);
uint32_t mac_res_free_dev(uint32_t dev_idx);
uint32_t mac_res_free_mac_user(uint16_t idx);
uint32_t mac_res_free_mac_vap(uint32_t idx);
uint32_t mac_res_init(void);
uint32_t mac_res_exit(void);
mac_chip_stru *mac_res_get_mac_chip(uint32_t chip_idx);
uint16_t mac_chip_get_max_asoc_user(uint8_t chip_id);
uint8_t mac_chip_get_max_active_user(void);
uint16_t mac_board_get_max_user(void);
wlan_mib_ieee802dot11_stru *mac_res_get_mib_info(uint8_t vap_id);


OAL_STATIC OAL_INLINE uint32_t mac_res_alloc_hmac_vap(uint8_t *idx, uint16_t hmac_priv_size)
{
    unsigned long idx_temp;

    if (oal_unlikely(idx == NULL)) {
        return OAL_FAIL;
    }

    idx_temp = (unsigned long)(uintptr_t)oal_queue_dequeue(&(g_pst_mac_res->st_vap_res.st_queue));
    /* 0为无效值 */
    if (idx_temp == 0) {
        return OAL_FAIL;
    }

    *idx = (uint8_t)(idx_temp - 1);

    g_pst_mac_res->st_vap_res.us_hmac_priv_size = hmac_priv_size;

    (g_pst_mac_res->st_vap_res.auc_user_cnt[idx_temp - 1])++;

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE uint32_t mac_res_alloc_dmac_vap(uint8_t idx)
{
    if (oal_unlikely(idx >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)) {
        return OAL_FAIL;
    }

    (g_pst_mac_res->st_vap_res.auc_user_cnt[idx])++;

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE mac_device_stru *mac_res_get_dev(uint32_t dev_idx)
{
    if (oal_unlikely(dev_idx >= MAC_RES_MAX_DEV_NUM)) {
        return NULL;
    }

    return &(g_pst_mac_res->st_dev_res.ast_dev_info[dev_idx]);
}

OAL_STATIC OAL_INLINE mac_device_stru *mac_res_get_mac_dev(void)
{
    return &(g_pst_mac_res->st_dev_res.ast_dev_info[0]);
}

OAL_STATIC OAL_INLINE void *mac_res_get_hmac_vap(uint32_t idx)
{
    if (oal_unlikely(idx >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)) {
        return NULL;
    }

    return (void *)(g_pst_mac_res->st_vap_res.past_vap_info[idx]);
}


OAL_STATIC OAL_INLINE void *mac_res_get_mac_vap(uint8_t idx)
{
    if (oal_unlikely(idx >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)) {
        return NULL;
    }

    /* 这里返回偏移内存空间 */
    return (void *)((uint8_t *)(g_pst_mac_res->st_vap_res.past_vap_info[idx]) +
                                g_pst_mac_res->st_vap_res.us_hmac_priv_size);
}


OAL_STATIC OAL_INLINE void *mac_res_get_dmac_vap(uint8_t idx)
{
    return mac_res_get_mac_vap(idx);
}


OAL_STATIC OAL_INLINE uint32_t mac_res_alloc_hmac_user(uint16_t *idx, uint16_t hmac_priv_size)
{
    unsigned long idx_temp;

    if (oal_unlikely(idx == NULL)) {
        return OAL_FAIL;
    }

    idx_temp = (unsigned long)(uintptr_t)oal_queue_dequeue_16(&(g_pst_mac_res->st_user_res.st_queue));
    /* 0为无效值 */
    if (idx_temp == 0) {
        return OAL_FAIL;
    }

    *idx = (uint16_t)(idx_temp - 1);

    g_pst_mac_res->st_user_res.us_hmac_priv_size = hmac_priv_size;

    (g_pst_mac_res->st_user_res.puc_user_cnt[idx_temp - 1])++;

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE void *_mac_res_get_hmac_user(uint16_t idx)
{
    if (oal_unlikely(idx >= g_wlan_spec_cfg->max_user_limit)) {
        return NULL;
    }

    return (void *)(g_pst_mac_res->st_user_res.past_user_info[idx]);
}


OAL_STATIC OAL_INLINE void *_mac_res_get_mac_user(uint16_t idx)
{
    if (oal_unlikely(idx >= g_wlan_spec_cfg->max_user_limit)) {
        return NULL;
    }

    /* 这里偏移内存空间 */
    return (void *)((uint8_t *)(g_pst_mac_res->st_user_res.past_user_info[idx]) +
                                g_pst_mac_res->st_user_res.us_hmac_priv_size);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_resource.h */
