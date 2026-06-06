

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 头文件包含 */
#include "frw_ext_if.h"
#include "hmac_arp_offload.h"
#include "hmac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ARP_OFFLOAD_C

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
/* 2 全局变量定义 */
/* 3 函数实现 */

oal_uint32 hmac_arp_offload_set_ip_addr(mac_vap_stru *pst_mac_vap,
                                        dmac_ip_type_enum_uint8 en_type,
                                        dmac_ip_oper_enum_uint8 en_oper,
                                        oal_void *pst_ip_addr,
                                        oal_void *pst_mask_addr)
{
    dmac_ip_addr_config_stru  st_ip_addr_config;
    oal_uint32                ul_ret;

    if (OAL_UNLIKELY((pst_mac_vap == OAL_PTR_NULL) ||
                      (pst_ip_addr == OAL_PTR_NULL) ||
                      (pst_mask_addr == OAL_PTR_NULL))) {
        OAM_ERROR_LOG3(0, OAM_SF_PWR,
                       "{hmac_arp_offload_set_ip_addr::The pst_mac_vap or ip_addr or mask_addr is NULL. %p, %p, %p}",
                       (uintptr_t)pst_mac_vap,
                       (uintptr_t)pst_ip_addr,
                       (uintptr_t)pst_mask_addr);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_memset(&st_ip_addr_config, 0, OAL_SIZEOF(st_ip_addr_config));

    if (en_type == DMAC_CONFIG_IPV4) {
        oal_memcopy(st_ip_addr_config.auc_ip_addr, pst_ip_addr, OAL_IPV4_ADDR_SIZE);
        oal_memcopy(st_ip_addr_config.auc_mask_addr, pst_mask_addr, OAL_IPV4_ADDR_SIZE);
    } else if (en_type == DMAC_CONFIG_IPV6) {
        oal_memcopy(st_ip_addr_config.auc_ip_addr, pst_ip_addr, OAL_IPV6_ADDR_SIZE);
    } else {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR,
                       "{hmac_arp_offload_set_ip_addr::The type[%d] is error.}", en_type);
        return OAL_ERR_CODE_MAGIC_NUM_FAIL;
    }
    st_ip_addr_config.en_type = en_type;

    if (OAL_UNLIKELY(en_oper >= DMAC_IP_OPER_BUTT)) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR,
                       "{hmac_arp_offload_set_ip_addr::The operation[%d] is error.}", en_type);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    st_ip_addr_config.en_oper = en_oper;

    ul_ret = hmac_config_set_ip_addr(pst_mac_vap,
                                     OAL_SIZEOF(dmac_ip_addr_config_stru),
                                     (oal_uint8 *)&st_ip_addr_config);
    return ul_ret;
}
#ifdef _PRE_DEBUG_MODE

oal_uint32 hmac_arp_offload_enable(mac_vap_stru *pst_mac_vap, oal_switch_enum_uint8 en_switch)
{
    oal_uint32 ul_ret;

    if (OAL_UNLIKELY(pst_mac_vap == NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{hmac_arp_offload_enable::The pst_mac_vap point is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(en_switch >= OAL_SWITCH_BUTT)) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR,
                       "{hmac_arp_offload_enable::The en_switch[%d] is error.}", en_switch);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    ul_ret = hmac_config_enable_arp_offload(pst_mac_vap,
                                            OAL_SIZEOF(oal_switch_enum_uint8),
                                            (oal_switch_enum_uint8 *)&en_switch);
    return ul_ret;
}
#endif  // #ifdef _PRE_DEBUG_MODE

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

