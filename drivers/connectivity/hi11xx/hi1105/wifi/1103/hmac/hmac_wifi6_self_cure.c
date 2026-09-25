

#ifndef __HMAC_WIFI6_SELF_CURE_C_
#define __HMAC_WIFI6_SELF_CURE_C_
#ifdef _PRE_WLAN_FEATURE_11AX

/* 1 头文件包含 */
#include "hmac_ext_if.h"
#include "mac_data.h"
#include "hmac_resource.h"
#include "hmac_wifi6_self_cure.h"
#include "hmac_vap.h"
#include "hmac_user.h"
#include "hmac_fsm.h"
#include "mac_user.h"
#ifdef _PRE_WLAN_FEATURE_ROAM
#include "hmac_roam_main.h"
#endif
#include "hmac_arp_probe.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_WIFI6_SELF_CURE_C

OAL_STATIC hmac_wifi6_self_cure_info_stru g_st_wifi6_self_cure_info;


/* 2 函数声明 */
/* 3 函数实现 */


oal_uint32  hmac_wifi6_self_cure_close_htc_handle(mac_vap_stru *pst_mac_vap)
{
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    hmac_wifi6_self_cure_info_stru *pst_save_info;

    if (!IS_LEGACY_STA(pst_mac_vap)) {
        return OAL_FALSE;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,
                       "{hmac_wifi6_self_cure_close_htc_handle::mac_res_get_hmac_vap fail.vap_id = %u}",
                       pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(pst_mac_vap->us_assoc_vap_id);
    if (pst_hmac_user == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    if (!MAC_USER_TX_DATA_INCLUDE_HTC(&pst_hmac_user->st_user_base_info)) {
        return OAL_SUCC;
    }
    pst_save_info = &g_st_wifi6_self_cure_info;

    memcpy_s(pst_save_info->auc_current_probe_close_htc_user, WLAN_MAC_ADDR_LEN,
            pst_hmac_user->st_user_base_info.auc_user_mac_addr, WLAN_MAC_ADDR_LEN);

    OAM_WARNING_LOG3(0, OAM_SF_SCAN, "hmac_wifi6_self_cure_close_htc_handle::\
        user_mac:%02X:XX:XX:XX:%02X:%02X close HTC to reassoc",
        pst_hmac_user->st_user_base_info.auc_user_mac_addr[0], /* 0为mac地址第一个字节 */
        pst_hmac_user->st_user_base_info.auc_user_mac_addr[4], /* 4为mac地址第一个字节 */
        pst_hmac_user->st_user_base_info.auc_user_mac_addr[5]); /* 5为mac地址第一个字节 */
    return hmac_arp_probe_fail_reassoc_trigger(pst_hmac_vap, pst_hmac_user);
}


oal_uint32  hmac_wifi6_self_cure_black_list_set(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8 uc_cure_info_len;
    hmac_wifi6_self_cure_info_stru *pst_save_info;
    hmac_wifi6_self_cure_info_stru *pst_wifi6_self_cure_info;
    oal_uint8 uc_index;

    uc_cure_info_len = OAL_SIZEOF(hmac_wifi6_self_cure_info_stru);
    if (uc_len != uc_cure_info_len) {
        return OAL_FAIL;
    }

    if (!IS_LEGACY_STA(pst_mac_vap)) {
        return OAL_FAIL;
    }

    pst_wifi6_self_cure_info = (hmac_wifi6_self_cure_info_stru *)puc_param;
    OAM_WARNING_LOG1(0, OAM_SF_M2S, "{hmac_wifi6_self_cure_black_list_set::uc_blaclist_num=[%d] .}",
        pst_wifi6_self_cure_info->uc_blaclist_num);

    pst_save_info = &g_st_wifi6_self_cure_info;
    if (pst_save_info->en_self_cure_type == HMAC_WIFI6_SELF_CURE_HANDLE_TYPE_DRIVER) {
        return OAL_SUCC;
    }

    memcpy_s(pst_save_info, uc_cure_info_len, pst_wifi6_self_cure_info, uc_cure_info_len);

    for (uc_index = 0; uc_index < pst_save_info->uc_blaclist_num; uc_index++) {
            OAM_WARNING_LOG4(0, OAM_SF_SCAN, "hmac_wifi6_self_cure_black_list_set::\
                user_mac_%d:%02X:XX:XX:XX:%02X:%02X",
                uc_index,
                pst_save_info->ast_wifi6_blacklist[uc_index].auc_user_mac_addr[0], /* 0为mac地址第一个字节 */
                pst_save_info->ast_wifi6_blacklist[uc_index].auc_user_mac_addr[4], /* 4为mac地址第一个字节 */
                pst_save_info->ast_wifi6_blacklist[uc_index].auc_user_mac_addr[5]); /* 5为mac地址第一个字节 */
    }

    return OAL_SUCC;
}


oal_bool_enum_uint8 hmac_wifi6_self_cure_mac_is_wifi6_blacklist_type(oal_uint8 *puc_mac)
{
    oal_uint8 uc_loop;
    hmac_wifi6_self_cure_info_stru *pst_save_info;
    hmac_wifi6_self_cure_blacklist_stru *pst_wifi6_list;

    pst_save_info = &g_st_wifi6_self_cure_info;

    for (uc_loop = 0; uc_loop < pst_save_info->uc_blaclist_num; uc_loop++) {
        pst_wifi6_list = &pst_save_info->ast_wifi6_blacklist[uc_loop];
        if (!OAL_MEMCMP(pst_wifi6_list->auc_user_mac_addr, puc_mac, WLAN_MAC_ADDR_LEN) &&
            pst_wifi6_list->en_blacklist_type == HMAC_WIFI6_SELF_CURE_BLACKLIST_TYPE_WIFI6) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


oal_bool_enum_uint8 hmac_wifi6_self_cure_mac_is_htc_blacklist_type(oal_uint8 *puc_mac)
{
    oal_uint8 uc_loop;
    hmac_wifi6_self_cure_info_stru *pst_save_info;
    hmac_wifi6_self_cure_blacklist_stru *pst_wifi6_list;

    pst_save_info = &g_st_wifi6_self_cure_info;

    if (!OAL_MEMCMP(pst_save_info->auc_current_probe_close_htc_user, puc_mac, WLAN_MAC_ADDR_LEN)) {
        return OAL_TRUE;
    }

    for (uc_loop = 0; uc_loop < pst_save_info->uc_blaclist_num; uc_loop++) {
        pst_wifi6_list = &pst_save_info->ast_wifi6_blacklist[uc_loop];
        if (!OAL_MEMCMP(pst_wifi6_list->auc_user_mac_addr, puc_mac, WLAN_MAC_ADDR_LEN) &&
            pst_wifi6_list->en_blacklist_type == HMAC_WIFI6_SELF_CURE_BLACKLIST_TYPE_HTC) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


oal_void hmac_wifi6_self_cure_init(hmac_vap_stru *pst_hmac_vap)
{
    g_st_wifi6_self_cure_info.en_self_cure_type = HMAC_WIFI6_SELF_CURE_HANDLE_TYPE_APP;
}


oal_void hmac_wifi6_self_cure_set_handle_type(hmac_wifi6_self_cure_handle_type_enum_uint8 en_type)
{
    OAM_WARNING_LOG1(0, OAM_SF_M2S, "{hmac_wifi6_self_cure_set_handle_type::en_type=[%d].}", en_type);
    g_st_wifi6_self_cure_info.en_self_cure_type = en_type;
}

uint8_t hmac_wifi6_self_cure_get_handle_type(oal_void)
{
    return g_st_wifi6_self_cure_info.en_self_cure_type;
}

#endif
#endif /* end of __HMAC_WIFI6_SELF_CURE_C__ */

