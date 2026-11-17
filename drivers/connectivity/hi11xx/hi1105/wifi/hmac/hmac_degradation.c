

#ifndef __HMAC_DEGRADATION_C_
#define __HMAC_DEGRADATION_C_

#include "hmac_degradation.h"
#include "hmac_vap.h"
#include "hmac_user.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DEGRADATION_C

OAL_STATIC hmac_degradation_info_stru g_degradation_info;

void hmac_degradation_whitelist_set(mac_vap_stru *mac_vap)
{
    hmac_degradation_info_stru *degradation_list_info = &g_degradation_info;
    hmac_user_stru *hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(mac_vap->us_assoc_vap_id);

    if (hmac_user == NULL) {
        return;
    }

    oam_warning_log1(0, OAM_SF_M2S, "{hmac_degradation_whitelist_set::list_num=[%d].}",
        degradation_list_info->list_num);

    /* 白名单满了按顺序从头替换 */
    if (degradation_list_info->list_num == HMAC_DEGRADATION_WRITELIST_MAX_NUM) {
        if (degradation_list_info->curr_location == HMAC_DEGRADATION_WRITELIST_MAX_NUM) {
            degradation_list_info->curr_location = 0;
        }
    } else {
        degradation_list_info->list_num++;
    }
    memcpy_s(degradation_list_info->writelist[degradation_list_info->curr_location].user_mac_addr,
        WLAN_MAC_ADDR_LEN, hmac_user->st_user_base_info.auc_user_mac_addr, WLAN_MAC_ADDR_LEN);
    degradation_list_info->curr_location++;
}


uint8_t hmac_degradation_is_whitelist_type(uint8_t *mac_addr)
{
    uint8_t loop;
    hmac_degradation_info_stru *list_info = &g_degradation_info;
    hmac_degradation_writelist_stru *writelist = NULL;

    for (loop = 0; loop < list_info->list_num; loop++) {
        writelist = &list_info->writelist[loop];
        if (oal_memcmp(writelist->user_mac_addr, mac_addr, WLAN_MAC_ADDR_LEN) == 0) {
            oam_warning_log3(0, OAM_SF_M2S, "{hmac_degradation_is_whitelist_type::ap is in writelist, \
                user_mac_%d:%02X:XX:XX:XX:%02X:%02X.}", mac_addr[0], mac_addr[4], mac_addr[5]); /* 0,4,5为mac地址 */
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}
#endif /* end of __HMAC_MCM_DEGRADATION_C_ */

