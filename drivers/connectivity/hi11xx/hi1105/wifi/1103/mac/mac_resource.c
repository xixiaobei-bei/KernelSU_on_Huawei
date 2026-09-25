

/* 1 头文件包含 */
#include "wlan_spec.h"
#include "mac_resource.h"
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "dmac_vap.h"
#include "dmac_user.h"
#endif
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
#include "hmac_vap.h"
#include "hmac_user.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_RESOURCE_C

/* 2 全局变量定义 */
mac_res_stru g_st_mac_res_etc;
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
OAL_STATIC wlan_mib_ieee802dot11_stru g_ast_mib_res[WLAN_SERVICE_VAP_SUPPORT_MAX_NUM_LIMIT];
#endif

/* 3 函数实现 */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32 mac_res_alloc_hmac_dev_etc(oal_uint8 *puc_dev_idx)
{
    oal_uint ul_dev_idx_temp;

    if (OAL_UNLIKELY(puc_dev_idx == OAL_PTR_NULL)) {
        OAL_IO_PRINT("mac_res_alloc_hmac_dev_etc: OAL_PTR_NULL == pul_dev_idx");
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_res_alloc_hmac_dev_etc::puc_dev_idx null.}");

        return OAL_FAIL;
    }

    ul_dev_idx_temp = (oal_uint)(uintptr_t)oal_queue_dequeue(&(g_pst_mac_res->st_dev_res.st_queue));

    /* 0为无效值 */
    if (ul_dev_idx_temp == 0) {
        OAL_IO_PRINT("mac_res_alloc_hmac_dev_etc: 0 == ul_dev_idx_temp");
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_res_alloc_hmac_dev_etc::ul_dev_idx_temp=0.}");

        return OAL_FAIL;
    }

    *puc_dev_idx = (oal_uint8)(ul_dev_idx_temp - 1);

    (g_pst_mac_res->st_dev_res.auc_user_cnt[ul_dev_idx_temp - 1])++;

    return OAL_SUCC;
}
#endif

oal_uint8 mac_chip_get_max_multi_user(oal_void)
{
    /* 组播最大用户总数 */
    return (WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP *
            (WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT - WLAN_CONFIG_VAP_MAX_NUM_PER_DEVICE));
}

wlan_mib_ieee802dot11_stru *mac_res_get_mib_info_etc(oal_uint8 uc_vap_id)
{
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    if (OAL_UNLIKELY(uc_vap_id == 0 || uc_vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)) {
        return OAL_PTR_NULL;
    }

    return &g_ast_mib_res[uc_vap_id - 1];
#else
    return (wlan_mib_ieee802dot11_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_MIB,
                                                       OAL_SIZEOF(wlan_mib_ieee802dot11_stru),
                                                       OAL_TRUE);
#endif
}
