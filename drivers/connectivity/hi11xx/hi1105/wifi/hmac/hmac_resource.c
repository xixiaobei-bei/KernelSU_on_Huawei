

#include "hmac_resource.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RESOURCE_C

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 存储hmac res资源结构体 */
typedef struct {
    hmac_res_device_stru st_hmac_dev_res;
} hmac_res_stru;

OAL_STATIC hmac_res_stru g_st_hmac_res;

/*
 * 功能描述  : 获取一个HMAC DEV资源
 */
uint32_t hmac_res_alloc_mac_dev(uint32_t dev_idx)
{
    if (oal_unlikely(dev_idx >= MAC_RES_MAX_DEV_NUM)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_res_alloc_hmac_dev::invalid dev_idx[%d].}", dev_idx);

        return OAL_FAIL;
    }

    (g_st_hmac_res.st_hmac_dev_res.auc_user_cnt[dev_idx])++;

    return OAL_SUCC;
}


uint32_t hmac_res_free_mac_dev(uint32_t dev_idx)
{
    if (oal_unlikely(dev_idx >= MAC_RES_MAX_DEV_NUM)) {
        oam_error_log1(0, OAM_SF_ANY, "{mac_res_free_dev::invalid dev_idx[%d].}", dev_idx);

        return OAL_FAIL;
    }

    (g_st_hmac_res.st_hmac_dev_res.auc_user_cnt[dev_idx])--;

    if (g_st_hmac_res.st_hmac_dev_res.auc_user_cnt[dev_idx] != 0) {
        return OAL_SUCC;
    }

    /* 入队索引值需要加1操作 */
    oal_queue_enqueue (&(g_st_hmac_res.st_hmac_dev_res.st_queue),
                       (void *)(uintptr_t)((unsigned long) dev_idx + 1));

    return OAL_SUCC;
}


hmac_device_stru *hmac_res_get_mac_dev(uint32_t dev_idx)
{
    if (oal_unlikely(dev_idx >= MAC_RES_MAX_DEV_NUM)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_res_get_hmac_dev::invalid dev_idx[%d].}", dev_idx);

        return NULL;
    }

    return &(g_st_hmac_res.st_hmac_dev_res.ast_hmac_dev_info[dev_idx]);
}


mac_chip_stru *hmac_res_get_mac_chip(uint32_t chip_idx)
{
    return mac_res_get_mac_chip(chip_idx);
}


uint32_t hmac_res_init(void)
{
    uint32_t loop;

    memset_s(&g_st_hmac_res, sizeof(g_st_hmac_res), 0, sizeof(g_st_hmac_res));

    /***************************************************************************
            初始化HMAC DEV的资源管理内容
    ***************************************************************************/
    oal_queue_set(&(g_st_hmac_res.st_hmac_dev_res.st_queue),
                  g_st_hmac_res.st_hmac_dev_res.aul_idx,
                  MAC_RES_MAX_DEV_NUM);

    for (loop = 0; loop < MAC_RES_MAX_DEV_NUM; loop++) {
        /* 初始值保存的是对应数组下标值加1 */
        oal_queue_enqueue(&(g_st_hmac_res.st_hmac_dev_res.st_queue),
                          (void *)(uintptr_t)((unsigned long)loop + 1));

        /* 初始化对应的引用计数值为0 */
        g_st_hmac_res.st_hmac_dev_res.auc_user_cnt[loop] = 0;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 资源池退出，释放动态申请的内存
 */
uint32_t hmac_res_exit(mac_board_stru *hmac_board)
{
    uint8_t chip_idx;
    uint8_t device_max;
    uint8_t dev_idx;
    uint16_t chip_max_num;
    mac_chip_stru *mac_chip = NULL;
    uint32_t ret;

    /* chip支持的最大数由PCIe总线处理提供; */
    chip_max_num = oal_bus_get_chip_num();

    for (chip_idx = 0; chip_idx < chip_max_num; chip_idx++) {
        mac_chip = &hmac_board->ast_chip[chip_idx];

        /* OAL接口获取支持device个数 */
        device_max = oal_chip_get_device_num(mac_chip->chip_ver);

        for (dev_idx = 0; dev_idx < device_max; dev_idx++) {
            /* 释放hmac res资源 */
            ret = hmac_res_free_mac_dev(mac_chip->auc_device_id[dev_idx]);
            if (ret != OAL_SUCC) {
                oam_error_log1(0, OAM_SF_ANY, "{hmac_res_exit::hmac_res_free_mac_dev failed[%d].}", ret);
            }
        }
    }
    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

