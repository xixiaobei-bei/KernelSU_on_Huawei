

/* 1 头文件包含 */
#include "wlan_types.h"
#include "mac_device.h"
#include "mac_resource.h"

#include "hmac_rx_filter.h"
#include "hmac_device.h"
#include "hmac_resource.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RX_FILTER_C

/* 2 全局变量定义 */
/* 3 函数实现 */

oal_bool_enum_uint8 hmac_find_is_sta_up(mac_device_stru *pst_mac_device)
{
    mac_vap_stru *pst_vap = NULL;
    uint8_t uc_vap_idx;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == NULL) {
            oam_error_log1(0, OAM_SF_SCAN, "{hmac_find_is_sta_up::pst_mac_vap null,vap_idx=%d.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if ((pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) && (pst_vap->en_vap_state == MAC_VAP_STATE_UP)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


oal_bool_enum_uint8 hmac_find_is_ap_up(mac_device_stru *pst_mac_device)
{
    mac_vap_stru *pst_vap = NULL;
    uint8_t uc_vap_idx;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == NULL) {
            oam_error_log1(0, OAM_SF_SCAN, "{hmac_find_is_ap_up::pst_mac_vap null,vap_idx=%d.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if ((pst_vap->en_vap_state != MAC_VAP_STATE_INIT) && (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


uint32_t hmac_calc_up_ap_num(mac_device_stru *pst_mac_device)
{
    mac_vap_stru *pst_vap = NULL;
    uint8_t uc_vap_idx;
    uint8_t up_ap_num = 0;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == NULL) {
            oam_error_log1(0, OAM_SF_SCAN, "{hmac_calc_up_ap_num::pst_mac_vap null,vap_idx=%d.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if ((pst_vap->en_vap_state != MAC_VAP_STATE_INIT) && (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP)) {
            up_ap_num++;
        } else if ((pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) && (pst_vap->en_vap_state == MAC_VAP_STATE_UP)) {
            up_ap_num++;
        }
    }

    return up_ap_num;
}


uint32_t hmac_find_up_vap(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    uint32_t ret;
    mac_vap_stru *pst_vap_up = NULL; /* 处在UP状态的VAP */

    /* find up VAP */
    ret = mac_device_find_up_vap(pst_mac_device, &pst_vap_up);
    if ((ret == OAL_SUCC) && (pst_vap_up != NULL)) {
        *ppst_mac_vap = pst_vap_up;
        /* find up AP */
        ret = mac_device_find_up_ap(pst_mac_device, &pst_vap_up);
        if ((ret == OAL_SUCC) && (pst_vap_up != NULL)) {
            *ppst_mac_vap = pst_vap_up;
        }

        return OAL_SUCC;
    } else {
        *ppst_mac_vap = NULL;
        return OAL_FAIL;
    }
}