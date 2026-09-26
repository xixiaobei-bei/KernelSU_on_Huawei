

#include "hmac_main.h"
#include "hmac_blacklist.h"
#include "mac_vap.h"
#include "hmac_vap.h"
#include "mac_resource.h"
#include "hmac_user.h"
#include "hmac_mgmt_ap.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_BLACKLIST_C


OAL_STATIC uint32_t hmac_blacklist_mac_is_zero(const uint8_t *mac_addr)
{
    return (mac_addr[MAC_ADDR_0] == 0 && mac_addr[MAC_ADDR_1] == 0 && mac_addr[MAC_ADDR_2] == 0 &&
            mac_addr[MAC_ADDR_3] == 0 && mac_addr[MAC_ADDR_4] == 0 && mac_addr[MAC_ADDR_5] == 0);
}


OAL_STATIC uint32_t hmac_blacklist_mac_is_bcast(const uint8_t *mac_addr)
{
    return ((*(const uint16_t *)(mac_addr + 0) &
             *(const uint16_t *)(mac_addr + MAC_ADDR_2) &
             *(const uint16_t *)(mac_addr + MAC_ADDR_4)) == 0xffff);
}


OAL_STATIC void hmac_blacklist_init(mac_vap_stru *mac_vap, cs_blacklist_mode_enum_uint8 en_mode,
                                    oal_bool_enum_uint8 en_flush)
{
    uint32_t size;
    mac_blacklist_info_stru *blacklist_info = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    if ((mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_blacklist_init::not AP or STA mode!}\r\n");
        return;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_blacklist_init::hmac_vap null.}");
        return;
    }

    blacklist_info = hmac_vap->pst_blacklist_info;
    if (blacklist_info == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_blacklist_init::blacklist_info null!}\r\n");
        return;
    }
    size = sizeof(mac_blacklist_info_stru);
    /* Max=32 => 新增加mac_vap结构大小= 0x494 = 1172 ; Max=8 => size = 308 */
    if (en_flush) {
        memset_s(blacklist_info->ast_black_list, sizeof(mac_blacklist_stru) * WLAN_BLACKLIST_MAX, 0,
            sizeof(mac_blacklist_stru) * WLAN_BLACKLIST_MAX);
        memset_s(&(blacklist_info->st_autoblacklist_info), sizeof(mac_autoblacklist_info_stru), 0,
            sizeof(mac_autoblacklist_info_stru));
        blacklist_info->uc_list_num = 0;
    }
    blacklist_info->uc_mode = en_mode;
}


OAL_STATIC oal_bool_enum_uint8 hmac_blacklist_is_aged(mac_vap_stru *mac_vap, mac_blacklist_stru *blacklist)
{
    oal_time_us_stru cur_time;
    mac_blacklist_info_stru *blacklist_info = NULL;
    uint8_t *mac_addr = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    /* 1.1 全零地址，属于非法地址 */
    if (hmac_blacklist_mac_is_zero(blacklist->auc_mac_addr)) {
        return OAL_FALSE;
    }

    /* 2.1 老化时间为0表示不需要老化 */
    if (blacklist->aging_time == 0) {
        return OAL_FALSE;
    }

    /* 2.2 没有超过老化时间 */
    oal_time_get_stamp_us(&cur_time);
    if (cur_time.i_sec < (signed long)(blacklist->cfg_time + blacklist->aging_time)) {
        return OAL_FALSE;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_blacklist_is_aged::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_addr = blacklist->auc_mac_addr;
    oam_info_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                  "{aging time reach delete MAC:=%02X:XX:XX:%02X:%02X:%02X}",
                  mac_addr[0], mac_addr[MAC_ADDR_3], mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5]);

    /* 3.1 直接从黑名单中删除 */
    blacklist_info = hmac_vap->pst_blacklist_info;
    if (blacklist_info == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_blacklist_is_aged::blacklist_info null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (blacklist_info->uc_mode == CS_BLACKLIST_MODE_BLACK) {
        memset_s(blacklist, sizeof(mac_blacklist_stru), 0, sizeof(mac_blacklist_stru));
        if (blacklist_info->uc_list_num > 0) {
            blacklist_info->uc_list_num--;
            oam_info_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{del from blacklist & num=%d}",
                blacklist_info->uc_list_num);
        }
        return OAL_TRUE;
    }

    /* 4.1 直接从白名单中恢复 */
    if (blacklist_info->uc_mode == CS_BLACKLIST_MODE_WHITE) {
        blacklist->aging_time = 0;
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


OAL_STATIC mac_blacklist_stru *hmac_blacklist_get(mac_vap_stru *mac_vap, const uint8_t *mac_addr)
{
    uint8_t blacklist_index;
    mac_blacklist_info_stru *blacklist_info = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_blacklist_stru *blacklist = NULL;
    /* 1.1 广播地址，属于非法地址 */
    if (hmac_blacklist_mac_is_bcast(mac_addr)) {
        return blacklist;
    }

    /* 1.2 全零地址，属于非法地址 */
    if (hmac_blacklist_mac_is_zero(mac_addr)) {
        return blacklist;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_blacklist_get::hmac_vap null.}");
        return blacklist;
    }

    blacklist_info = hmac_vap->pst_blacklist_info;
    if (blacklist_info == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_blacklist_get::blacklist_info null!}\r\n");
        return blacklist;
    }

    /* 2.1 找到表单 */
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        blacklist = &blacklist_info->ast_black_list[blacklist_index];
        if (!oal_memcmp(blacklist->auc_mac_addr, mac_addr, OAL_MAC_ADDR_LEN)) {
            break;
        }
        blacklist = NULL;
    }

    return blacklist;
}


uint32_t hmac_backlist_get_drop(mac_vap_stru *mac_vap, const uint8_t *mac_addr)
{
    mac_blacklist_stru *blacklist = NULL;

    /* 1.1 入参检查 */
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_backlist_get_drop::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    blacklist = hmac_blacklist_get(mac_vap, mac_addr);
    if (blacklist != NULL) {
        return blacklist->drop_counter;
    }

    return OAL_FAIL;
}


OAL_STATIC void hmac_backlist_sub_drop(mac_vap_stru *mac_vap, const uint8_t *mac_addr)
{
    mac_blacklist_stru *blacklist = NULL;

    blacklist = hmac_blacklist_get(mac_vap, mac_addr);
    if (blacklist != NULL) {
        if (blacklist->drop_counter > 0) {
            blacklist->drop_counter--;
        }
    }
}


uint8_t hmac_backlist_get_list_num(mac_vap_stru *mac_vap)
{
    mac_blacklist_info_stru *blacklist_info = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    /* 1.1 入参检查 */
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_backlist_get_list_num::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_backlist_get_list_num::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    blacklist_info = hmac_vap->pst_blacklist_info;
    if (blacklist_info == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_backlist_get_list_num::blacklist_info null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return blacklist_info->uc_list_num;
}

static mac_autoblacklist_stru *hmac_autoblacklist_get_from_history(mac_autoblacklist_info_stru *auto_blacklist_info,
    oal_time_us_stru cur_time, const uint8_t *mac_addr)
{
    uint8_t blacklist_index;
    mac_autoblacklist_stru *auto_blacklist = NULL;
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        auto_blacklist = &auto_blacklist_info->ast_autoblack_list[blacklist_index];
        /* 2.2 无效表单 */
        if (auto_blacklist->cfg_time == 0) {
            continue;
        }

        /* 2.2 过期表单直接清0 */
        if (cur_time.i_sec > (signed long)(auto_blacklist->cfg_time + auto_blacklist_info->reset_time)) {
            if (auto_blacklist_info->list_num > 0) {
                auto_blacklist_info->list_num--;
            }
            memset_s(auto_blacklist, sizeof(mac_autoblacklist_stru), 0, sizeof(mac_autoblacklist_stru));
            continue;
        }

        /* 2.3 有效表单，mac地址比对 */
        if (!oal_memcmp(auto_blacklist->auc_mac_addr, mac_addr, OAL_MAC_ADDR_LEN)) {
            break;
        }
    }

    return auto_blacklist;
}

static uint32_t hmac_autoblacklist_get_from_new(oal_time_us_stru cur_time, mac_autoblacklist_stru *auto_blacklist,
                                                mac_autoblacklist_info_stru *auto_blacklist_info)
{
    uint32_t cfg_time_old;
    uint32_t blacklist_index_old = 0;
    uint8_t blacklist_index;

    cfg_time_old = (cur_time.i_sec > 0) ? (uint32_t)(cur_time.i_sec) : 0;

    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        auto_blacklist = &auto_blacklist_info->ast_autoblack_list[blacklist_index];
        /* 2.2 找到空闲表单 */
        if (auto_blacklist->cfg_time == 0) {
            auto_blacklist_info->list_num++;
            break;
        }

        /* 2.3 记录最老配置的表单 */
        if (auto_blacklist->cfg_time < cfg_time_old) {
            cfg_time_old = auto_blacklist->cfg_time;
            blacklist_index_old = blacklist_index;
        }
    }

    return blacklist_index_old;
}


static mac_autoblacklist_stru *hmac_autoblacklist_get(mac_vap_stru *mac_vap, const uint8_t *mac_addr)
{
    mac_autoblacklist_info_stru *auto_blacklist_info = NULL;
    oal_time_us_stru cur_time = {0};
    uint32_t blacklist_index_old;
    mac_autoblacklist_stru *auto_blacklist = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    /* 1.1 广播地址，属于非法地址 */
    if (hmac_blacklist_mac_is_bcast(mac_addr)) {
        return NULL;
    }

    /* 1.2 全零地址，属于非法地址 */
    if (hmac_blacklist_mac_is_zero(mac_addr)) {
        return NULL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    auto_blacklist_info = &(hmac_vap->pst_blacklist_info->st_autoblacklist_info);
    oal_time_get_stamp_us(&cur_time);

    /* 2.1 找到历史表单 */
    auto_blacklist = hmac_autoblacklist_get_from_history(auto_blacklist_info, cur_time, mac_addr);
    if (auto_blacklist != NULL) {
        oam_info_log2(mac_vap->uc_vap_id, OAM_SF_ANY,
                      "{Get a history item from auto_blacklist.MAC=x.x.x.x.%02x.%02x.}",
                      mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5]);
        return auto_blacklist;
    }

    /* 3.1 找到新表单 */
    blacklist_index_old = hmac_autoblacklist_get_from_new(cur_time, auto_blacklist, auto_blacklist_info);

    /* 4.1 如果没有空闲的表单，就使用最老的表单 */
    if (auto_blacklist == NULL) {
        auto_blacklist = &auto_blacklist_info->ast_autoblack_list[blacklist_index_old];
    }

    /* 5.1 更新表单 */
    oam_info_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                  "{add to auto_blacklist MAC:=%02X:XX:XX:%02X:%02X:%02X. assoc_count=1.}",
                  mac_addr[0], mac_addr[MAC_ADDR_3], mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5]);
    if (auto_blacklist == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_autoblacklist_get::auto_blacklist NULL}");
        return NULL;
    }
    memcpy_s(auto_blacklist->auc_mac_addr, OAL_MAC_ADDR_LEN, mac_addr, OAL_MAC_ADDR_LEN);
    auto_blacklist->cfg_time = (cur_time.i_sec > 0) ? (uint32_t)(cur_time.i_sec) : 0;
    auto_blacklist->asso_counter = 0;

    return auto_blacklist;
}


OAL_STATIC void hmac_blacklist_update_delete_user(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    mac_vap_stru *mac_vap = &(hmac_vap->st_vap_base_info);
    mac_user_stru *mac_user = &(hmac_user->st_user_base_info);

    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                     "hmac_blacklist_update_delete_user: send disasoc frame to XX:XX:XX:%2X:%2X:%2X. blacklist_mode:%d",
                     mac_user->auc_user_mac_addr[MAC_ADDR_3],
                     mac_user->auc_user_mac_addr[MAC_ADDR_4],
                     mac_user->auc_user_mac_addr[MAC_ADDR_5],
                     hmac_vap->pst_blacklist_info->uc_mode);

    hmac_mgmt_send_disassoc_frame(mac_vap, mac_user->auc_user_mac_addr, MAC_DISAS_LV_SS, OAL_FALSE);

    /* 删除黑名单内用户，需要上报内核 */
    hmac_handle_disconnect_rsp(hmac_vap, hmac_user, MAC_DISAS_LV_SS);

    hmac_user_del(mac_vap, hmac_user);

    return;
}


OAL_STATIC void hmac_whitelist_del_user(mac_vap_stru *mac_vap)
{
    uint32_t ret;
    hmac_vap_stru *hmac_vap = NULL;
    oal_dlist_head_stru *entry = NULL;
    oal_dlist_head_stru *dlist_tmp = NULL;
    mac_user_stru *user_tmp = NULL;
    hmac_user_stru *hmac_user_tmp = NULL;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "hmac_whitelist_del_user::hmac_vap is null");
        return;
    }

    /* 遍历VAP下所有用户，不在白名单里的删除 */
    oal_dlist_search_for_each_safe(entry, dlist_tmp, &(mac_vap->st_mac_user_list_head))
    {
        user_tmp = oal_dlist_get_entry(entry, mac_user_stru, st_user_dlist);
        if (user_tmp == NULL) {
            continue;
        }

        hmac_user_tmp = mac_res_get_hmac_user(user_tmp->us_assoc_id);
        if (hmac_user_tmp == NULL) {
            continue;
        }

        ret = hmac_blacklist_filter(mac_vap, user_tmp->auc_user_mac_addr);
        if (ret != OAL_TRUE) {
            continue;
        }
        /* 2014.6.30 chenchongbao 在如上的hmac_blacklist_filter()中会 drop_counter++ 它不是实际的过滤数，所以--恢复 */
        hmac_backlist_sub_drop(mac_vap, user_tmp->auc_user_mac_addr);

        hmac_blacklist_update_delete_user(hmac_vap, hmac_user_tmp);
    }
}


OAL_STATIC uint32_t hmac_blacklist_vap_check_user_by_macaddr(mac_vap_stru *mac_vap, uint8_t *mac_addr)
{
    mac_user_stru *mac_user = NULL;
    hmac_user_stru *hmac_user = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t ret;
    uint16_t user_idx;
    oal_bool_enum_uint8 bool_ret;

    if ((mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_blacklist_vap_check_user_by_macaddr::not AP or STA mode!}\r\n");
        return OAL_SUCC;
    }
    ret = mac_vap_find_user_by_macaddr(mac_vap, mac_addr, &user_idx);
    if (ret != OAL_SUCC) {
        /* 该mac地址在此vap下不存在用户，不需要删除，直接返回成功 */
        return OAL_SUCC;
    }

    mac_user = (mac_user_stru *)mac_res_get_mac_user(user_idx);
    if (mac_user == NULL) {
        return OAL_FAIL; /* 异常返回 */
    }

    bool_ret = hmac_blacklist_filter(mac_vap, mac_user->auc_user_mac_addr);
    if (bool_ret != OAL_TRUE) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_blacklist_vap_check_user_by_macaddr::hmac_blacklist_filter bool_ret=%d}", bool_ret);
        return OAL_FAIL;
    }
    /* 2014.6.30 chenchongbao 在如上的hmac_blacklist_filter()中会 drop_counter++ 它不是实际的过滤数，所以--恢复 */
    hmac_backlist_sub_drop(mac_vap, mac_user->auc_user_mac_addr);

    hmac_user = mac_res_get_hmac_user(user_idx);
    if (hmac_user == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_blacklist_vap_check_user_by_macaddr::mac_res_get_hmac_user fail IDX:%d}", user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_blacklist_update_delete_user(hmac_vap, hmac_user);

    return ret;
}

void hmac_whitelist_check_device_user(uint8_t device_id)
{
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *mac_vap = NULL;
    uint8_t vap_index;

    mac_device = mac_res_get_dev(device_id);
    if (mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_whitelist_check_device_user::mac_res_get_dev failed.");
        return;
    }
    for (vap_index = 0; vap_index < mac_device->uc_vap_num; vap_index++) {
        mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(mac_device->auc_vap_id[vap_index]);
        if (mac_vap == NULL) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_whitelist_check_device_user::mac_res_get_mac_vap failed.");
            continue;
        }
        if ((mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_whitelist_check_device_user::not AP or STA mode.");
            continue;
        }
        hmac_whitelist_del_user(mac_vap);
    }
}


void hmac_whitelist_check_all_user(mac_vap_stru *mac_vap)
{
    if ((mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_whitelist_check_all_user::not AP or STA mode.");
        return;
    }
    hmac_whitelist_del_user(mac_vap);
}


uint32_t hmac_blacklist_add(mac_vap_stru *mac_vap, uint8_t *mac_addr, uint32_t aging_time)
{
    uint8_t blacklist_index;
    mac_blacklist_info_stru *blacklist_info = NULL;
    mac_blacklist_stru *blacklist = NULL;
    oal_time_us_stru cur_time;
    hmac_vap_stru *hmac_vap = NULL;

    /* 1.1 入参检查 */
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_blacklist_add::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if ((mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_blacklist_add::not AP or STA mode!}\r\n");
        return OAL_SUCC;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_blacklist_add::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    blacklist_info = hmac_vap->pst_blacklist_info;
    if (blacklist_info == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_blacklist_add::blacklist_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 3.1 找到表单 */
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        blacklist = &blacklist_info->ast_black_list[blacklist_index];
        if (!oal_memcmp(blacklist->auc_mac_addr, mac_addr, OAL_MAC_ADDR_LEN)) {
            break;
        }
        blacklist = NULL;
    }

    /* 4.1 表单已经存在，只更新老化时间 */
    if (blacklist != NULL) {
        oam_info_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{blacklist_add allready exist. update aging = %d}", aging_time);
        blacklist->aging_time = aging_time;
        return OAL_SUCC;
    }

    /* 5.1 找一个地址为空的表单 */
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        blacklist = &blacklist_info->ast_black_list[blacklist_index];
        if (hmac_blacklist_mac_is_zero(blacklist->auc_mac_addr)) {
            break;
        }
        blacklist = NULL;
    }

    /* 6.1 无可用表单 */
    if (blacklist == NULL) {
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_blacklist_add:: src mac=x.x.x.%02x.%02x.%02x. new count is %d; full return}",
                         mac_addr[MAC_ADDR_3], mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5], blacklist_info->uc_list_num);
        return OAL_ERR_CODE_SECURITY_LIST_FULL;
    }

    /* 7.1 更新表单 */
    oal_time_get_stamp_us(&cur_time);
    if (memcpy_s(blacklist->auc_mac_addr, sizeof(blacklist->auc_mac_addr), mac_addr, OAL_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_blacklist_add::memcpy fail!");
        return OAL_FAIL;
    }
    blacklist->cfg_time = (uint32_t)cur_time.i_sec;
    blacklist->aging_time = aging_time;
    blacklist->drop_counter = 0;
    blacklist_info->uc_list_num++;

    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                     "{hmac_blacklist_add:: src mac=x.x.x.x.%02x.%02x. new count is %d, time=%d}",
                     mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5], blacklist_info->uc_list_num, blacklist->cfg_time);

    /* 黑白名单添加成功后刷新一把用户 */
    if (blacklist_info->uc_mode == CS_BLACKLIST_MODE_BLACK) {
        hmac_blacklist_vap_check_user_by_macaddr(mac_vap, mac_addr);
    } else if (blacklist_info->uc_mode == CS_BLACKLIST_MODE_WHITE) {
        hmac_whitelist_check_all_user(mac_vap);
    }

    return OAL_SUCC;
}

oal_bool_enum_uint8 hmac_blacklist_get_vap_in_device(mac_device_stru *device,
    uint8_t *mac_addr, mac_vap_stru **assoc_vap)
{
    oal_bool_enum_uint8 en_assoc_vap = OAL_FALSE;
    uint8_t vap_index;
    mac_vap_stru *mac_vap = NULL;
    mac_user_stru *mac_user = NULL;
    oal_dlist_head_stru *entry = NULL;

    for (vap_index = 0; vap_index < device->uc_vap_num; vap_index++) {
        mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(device->auc_vap_id[vap_index]);
        if (mac_vap == NULL) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_blacklist_get_user_in_device::mac_res_get_mac_vap failed.");
            continue;
        }
        if ((mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_blacklist_get_user_in_device::not AP or STA mode.");
            break;
        }

        if (en_assoc_vap == OAL_TRUE) {
            break;
        }
        /* 遍历vap下所有用户，若能找到当前user，则返回该vap */
        oal_dlist_search_for_each(entry, &(mac_vap->st_mac_user_list_head))
        {
            mac_user = oal_dlist_get_entry(entry, mac_user_stru, st_user_dlist);
            if (mac_user == NULL) {
                continue;
            }

            if (!OAL_MEMCMP(mac_user->auc_user_mac_addr, mac_addr, WLAN_MAC_ADDR_LEN)) {
                *assoc_vap = mac_vap;
                en_assoc_vap = OAL_TRUE;
                break;
            }
        }
    }

    return en_assoc_vap;
}


oal_bool_enum_uint8 hmac_blacklist_get_assoc_ap(mac_vap_stru *cur_vap,
    uint8_t *mac_addr, mac_vap_stru **assoc_vap)
{
    oal_bool_enum_uint8 en_assoc_vap;

    *assoc_vap = cur_vap;
    en_assoc_vap = OAL_TRUE;

    return en_assoc_vap;
}

uint32_t hmac_blacklist_add_only(mac_vap_stru *mac_vap, uint8_t *mac_addr, uint32_t aging_time)
{
    uint8_t blacklist_index;
    mac_blacklist_info_stru *blacklist_info = NULL;
    mac_blacklist_stru *blacklist = NULL;
    oal_time_us_stru cur_time;
    hmac_vap_stru *hmac_vap = NULL;

    /* 1.1 入参检查 */
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_blacklist_add_only::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if ((mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_blacklist_add_only::not AP or STA mode!}\r\n");
        return OAL_SUCC;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_blacklist_add_only::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    blacklist_info = hmac_vap->pst_blacklist_info;
    if (blacklist_info == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_blacklist_add_only::blacklist_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 3.1 找到表单 */
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        blacklist = &blacklist_info->ast_black_list[blacklist_index];
        if (!oal_memcmp(blacklist->auc_mac_addr, mac_addr, OAL_MAC_ADDR_LEN)) {
            break;
        }
        blacklist = NULL;
    }

    /* 4.1 表单已经存在，只更新老化时间 */
    if (blacklist != NULL) {
        oam_info_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                      "{blacklist_add allready exist. update aging = %d}", aging_time);
        blacklist->aging_time = aging_time;
        return OAL_SUCC;
    }

    /* 5.1 找一个地址为空的表单 */
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        blacklist = &blacklist_info->ast_black_list[blacklist_index];
        if (hmac_blacklist_mac_is_zero(blacklist->auc_mac_addr)) {
            break;
        }
        blacklist = NULL;
    }

    /* 6.1 无可用表单 */
    if (blacklist == NULL) {
        oam_info_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                      "{hmac_blacklist_add_only:: src mac=x.x.x.%02x.%02x.%02x. new count is %d; full return}",
                      mac_addr[MAC_ADDR_3], mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5], blacklist_info->uc_list_num);
        return OAL_ERR_CODE_SECURITY_LIST_FULL;
    }

    /* 7.1 更新表单 */
    oal_time_get_stamp_us(&cur_time);
    if (memcpy_s(blacklist->auc_mac_addr, OAL_MAC_ADDR_LEN, mac_addr, OAL_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_blacklist_add_only::memcpy fail!");
        return OAL_FAIL;
    }
    blacklist->cfg_time = (uint32_t)cur_time.i_sec;
    blacklist->aging_time = aging_time;
    blacklist->drop_counter = 0;
    blacklist_info->uc_list_num++;

    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                     "{hmac_blacklist_add_only:: src mac=x.x.x.x.%02x.%02x. new count is %d, time=%d}",
                     mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5], blacklist_info->uc_list_num, blacklist->cfg_time);

    return OAL_SUCC;
}


uint32_t hmac_blacklist_del(mac_vap_stru *mac_vap, uint8_t *mac_addr)
{
    uint8_t blacklist_index;
    mac_blacklist_stru *blacklist = NULL;
    mac_blacklist_info_stru *blacklist_info = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t ret;
    uint16_t user_idx = 0xffff;
    hmac_user_stru *hmac_user_tmp = NULL;

    /* 1.1 入参检查 */
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_blacklist_del::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if ((mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_blacklist_del::not AP or STA mode.");
        return OAL_SUCC;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_blacklist_del::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 2.1 如果模式不一致，不需要删除，返回失败即可 */
    blacklist_info = hmac_vap->pst_blacklist_info;
    if (blacklist_info == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_blacklist_del::blacklist_info null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 3.1 广播地址，需要删除所有表单 */
    if (hmac_blacklist_mac_is_bcast(mac_addr)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_blacklist_del::broadcast addr; delete all hmac_blacklist}");
        hmac_blacklist_init(mac_vap, blacklist_info->uc_mode, OAL_TRUE);

        return OAL_SUCC;
    }

    /* 4.1 找到表单 */
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        blacklist = &blacklist_info->ast_black_list[blacklist_index];
        if (!oal_memcmp(blacklist->auc_mac_addr, mac_addr, OAL_MAC_ADDR_LEN)) {
            break;
        }
        blacklist = NULL;
    }

    /* 5.1 如果找到表单，删除 */
    if (blacklist != NULL) {
        memset_s(blacklist, sizeof(mac_blacklist_stru), 0, sizeof(mac_blacklist_stru));
        blacklist_info->uc_list_num--;
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_blacklist_del:: mac=x.x.x.%02x.%02x.%02x. new count is %d}",
                         mac_addr[MAC_ADDR_3], mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5], blacklist_info->uc_list_num);

        if (blacklist_info->uc_mode == CS_BLACKLIST_MODE_WHITE) {
            /* 根据mac addr找到sta索引 */
            ret = mac_vap_find_user_by_macaddr(mac_vap, mac_addr, &user_idx);
            if (ret != OAL_SUCC) {
                oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                                 "{hmac_blacklist_del::find_user_by_macaddr failed[%d].}", ret);
                return ret;
            }
            hmac_user_tmp = mac_res_get_hmac_user(user_idx);
            if (hmac_user_tmp == NULL) {
                oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                               "{hmac_blacklist_del::hmac_user null,user_idx=%d.}", user_idx);
                return OAL_ERR_CODE_PTR_NULL;
            }
            hmac_blacklist_update_delete_user(hmac_vap, hmac_user_tmp);
        }
    } else {
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_blacklist_del:: didn't find this mac: %02X:XX:XX:%02X:%02X:%02X}",
                         mac_addr[0], mac_addr[MAC_ADDR_3], mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5]);
    }
    return OAL_SUCC;
}


uint32_t hmac_blacklist_set_mode(mac_vap_stru *mac_vap, uint8_t mode)
{
    /* 1.1 入参检查 */
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_blacklist_set_mode::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if ((mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_blacklist_set_mode::not AP or STA mode!}\r\n");
        return OAL_SUCC;
    }

    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_blacklist_set_mode::mode = %d}", mode);
    /* 2.1 根据不同模式增加表单 */
    switch (mode) {
        case CS_BLACKLIST_MODE_NONE:
        case CS_BLACKLIST_MODE_BLACK:
        case CS_BLACKLIST_MODE_WHITE:
            /* hipriv.sh debug suggest OAL_FALSE: don't flush all bssids in blacklist */
            /* FWK configuration suggest OAL_TRUE:Flush all bssids in blacklist */
            hmac_blacklist_init(mac_vap, mode, OAL_TRUE); /* OAL_TRUE:Flush blacklist; OAL_FALSE:don't flush */
            break;
        default:
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{unknown mode = %d}", mode);
            return OAL_ERR_CODE_SECURITY_MODE_INVALID;
    }

    return OAL_SUCC;
}


uint32_t hmac_blacklist_get_mode(mac_vap_stru *mac_vap, uint8_t *mode)
{
    hmac_vap_stru *hmac_vap = NULL;

    /*  入参检查 */
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_blacklist_get_mode::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if ((mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        return OAL_SUCC;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_blacklist_get_mode::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *((uint32_t *)mode) = hmac_vap->pst_blacklist_info->uc_mode;
    return OAL_SUCC;
}

static uint8_t hmac_cfg_autoblacklist_info(mac_autoblacklist_info_stru *autoblacklist_info, int8_t *print_buff)
{
    uint8_t buff_index;
    oam_info_log4(0, OAM_SF_ANY,
        "{hmac_cfg_autoblacklist_info::AUTOBLACKLIST[%d] info: THRESHOLD: %u. RESET_TIME: %u sec. AGING_TIME: %u sec}",
        autoblacklist_info->uc_enabled, autoblacklist_info->threshold,
        autoblacklist_info->reset_time, autoblacklist_info->aging_time);

    buff_index = (uint8_t)snprintf_s(print_buff, OAM_REPORT_MAX_STRING_LEN,
                                     OAM_REPORT_MAX_STRING_LEN - 1,
                                     "\nAUTOBLACKLIST[%d] info:\n"
                                     "  THRESHOLD: %u\n"
                                     "  RESET_TIME: %u sec\n"
                                     "  AGING_TIME: %u sec\n",
                                     autoblacklist_info->uc_enabled,
                                     autoblacklist_info->threshold,
                                     autoblacklist_info->reset_time,
                                     autoblacklist_info->aging_time);
    return buff_index;
}


OAL_STATIC void hmac_show_autoblacklist_info(mac_autoblacklist_info_stru *autoblacklist_info)
{
    uint8_t blacklist_index, buff_index;
    uint8_t *sa = NULL;
    mac_autoblacklist_stru *autoblacklist = NULL;
    int8_t *print_buff;

    print_buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (print_buff == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_show_autoblacklist_info::print_buff null.}");
        return;
    }
    memset_s(print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);

    /* 2.1 黑名单配置信息 */
    buff_index = hmac_cfg_autoblacklist_info(autoblacklist_info, print_buff);
    /* 4.1 打印黑名单表单 */
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        autoblacklist = &autoblacklist_info->ast_autoblack_list[blacklist_index];
        if (hmac_blacklist_mac_is_zero(autoblacklist->auc_mac_addr)) {
            continue;
        }
        sa = autoblacklist->auc_mac_addr;
        oam_info_log4(0, OAM_SF_ANY,
            "{src mac=%02X:XX:XX:%02X:%02X:%02X}", sa[0], sa[3], sa[4], sa[5]); /* 黑名单MAC地址的0,3,4,5字节 */
        oam_info_log2(0, OAM_SF_ANY, "{ ASSO_CNT: %u. cfg_time=%d. }",
                      autoblacklist->asso_counter, autoblacklist->cfg_time);
        buff_index += (uint8_t)snprintf_s(print_buff + buff_index,
                                          (OAM_REPORT_MAX_STRING_LEN - buff_index),
                                          (OAM_REPORT_MAX_STRING_LEN - buff_index) - 1,
                                          "\n[%02d] MAC: %02X:XX:XX:%02X:%02X:%02X\n"
                                          " cfg_time=%d. ASSO_CNT: %d\n",
                                          blacklist_index,
                                          autoblacklist->auc_mac_addr[0],
                                          autoblacklist->auc_mac_addr[MAC_ADDR_3],
                                          autoblacklist->auc_mac_addr[MAC_ADDR_4],
                                          autoblacklist->auc_mac_addr[MAC_ADDR_5],
                                          autoblacklist->cfg_time,
                                          autoblacklist->asso_counter);
    }
    oam_print(print_buff);
    oal_mem_free_m(print_buff, OAL_TRUE);
}


void hmac_show_blacklist_info(mac_vap_stru *mac_vap)
{
    uint8_t *sa = NULL;
    uint8_t buff_index, blacklist_index;
    mac_blacklist_stru *blacklist = NULL;
    mac_blacklist_info_stru *blacklist_info = NULL;
    oal_time_us_stru cur_time;
    int8_t *print_buff = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    /* 1.1 入参检查 */
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_show_blacklist_info::null mac_vap}");
        return;
    }
    if ((mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_show_blacklist_info::not AP or STA mode!}\r\n");
        return;
    }

    print_buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (print_buff == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_show_blacklist_info::print_buff null.}");
        return;
    }
    memset_s(print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_show_blacklist_info::hmac_vap null.}");
        oal_mem_free_m(print_buff, OAL_TRUE);
        return;
    }

    blacklist_info = hmac_vap->pst_blacklist_info;
    if (blacklist_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_show_blacklist_info::blacklist_info null.}");
        oal_mem_free_m(print_buff, OAL_TRUE);
        return;
    }

    /* 3.1 黑名单模式信息 */
    if (blacklist_info->uc_mode == CS_BLACKLIST_MODE_BLACK) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{BLACKLIST[BLACK] num=%d info:}", blacklist_info->uc_list_num);
        buff_index = (uint8_t)snprintf_s(print_buff, OAM_REPORT_MAX_STRING_LEN, OAM_REPORT_MAX_STRING_LEN - 1,
                                         "BLACKLIST[BLACK] num=%d info:\n", blacklist_info->uc_list_num);
    } else if (blacklist_info->uc_mode == CS_BLACKLIST_MODE_WHITE) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{BLACKLIST[WHITE] num=%d info:}", blacklist_info->uc_list_num);
        buff_index = (uint8_t)snprintf_s(print_buff, OAM_REPORT_MAX_STRING_LEN, OAM_REPORT_MAX_STRING_LEN - 1,
                                         "BLACKLIST[WHITE] num=%d info:\n", blacklist_info->uc_list_num);
    } else {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{BLACKLIST not enable! num=%d info:}", blacklist_info->uc_list_num);
        buff_index = (uint8_t)snprintf_s(print_buff, OAM_REPORT_MAX_STRING_LEN, OAM_REPORT_MAX_STRING_LEN - 1,
                                         "BLACKLIST not enable! num=%d info:\n", blacklist_info->uc_list_num);
    }

    /* 5.1 打印黑名单表单 */
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        blacklist = &blacklist_info->ast_black_list[blacklist_index];
        if (hmac_blacklist_mac_is_zero(blacklist->auc_mac_addr)) {
            continue;
        }
        sa = blacklist->auc_mac_addr;
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{src mac=%02X:XX:XX:%02X:%02X:%02X}", sa[0], sa[3], sa[4], sa[5]); /* 黑名单MAC地址的0,3,4,5字节 */
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_ANY, "{ CFG_TIME: %u. AGE_TIME: %u. DROP_CNT: %u.}",
                         blacklist->cfg_time, blacklist->aging_time, blacklist->drop_counter);

        buff_index += (uint8_t)snprintf_s(print_buff + buff_index, (OAM_REPORT_MAX_STRING_LEN - buff_index),
                                          (OAM_REPORT_MAX_STRING_LEN - buff_index) - 1,
                                          "\n[%02d] MAC: %02X:XX:XX:%02X:%02X:%02X\n"
                                          " CFG_TIME: %u\t AGE_TIME: %u\t DROP_CNT: %u\n",
                                          blacklist_index, blacklist->auc_mac_addr[MAC_ADDR_0],
                                          blacklist->auc_mac_addr[MAC_ADDR_3],
                                          blacklist->auc_mac_addr[MAC_ADDR_4],
                                          blacklist->auc_mac_addr[MAC_ADDR_5],
                                          blacklist->cfg_time, blacklist->aging_time,
                                          blacklist->drop_counter);
    }
    oam_print(print_buff);
    oal_mem_free_m(print_buff, OAL_TRUE);

    /* 4.1 自动黑名单信息 */
    hmac_show_autoblacklist_info(&blacklist_info->st_autoblacklist_info);

    /* 加入当前时间 */
    oal_time_get_stamp_us(&cur_time);
    oam_info_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{ CURR_TIME: %d}", (uint32_t)cur_time.i_sec);
}


uint32_t hmac_autoblacklist_enable(mac_vap_stru *mac_vap, uint8_t enabled)
{
    mac_autoblacklist_info_stru *autoblacklist_info = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    /* 1.1 入参检查 */
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_autoblacklist_enable::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if ((mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_autoblacklist_enable::not AP or STA mode!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 2.1 获取参数 */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_autoblacklist_enable::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_vap->pst_blacklist_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_autoblacklist_enable::blacklist_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    autoblacklist_info = &(hmac_vap->pst_blacklist_info->st_autoblacklist_info);

    /* 参数跟原有一样 */
    if (enabled == autoblacklist_info->uc_enabled) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_autoblacklist_enable:autoblacklist is already %d}", enabled);
        return OAL_SUCC;
    }

    /* 重新初始化 */
    if (enabled == 1) {
        hmac_blacklist_init(mac_vap, CS_BLACKLIST_MODE_BLACK, OAL_TRUE);
        autoblacklist_info->uc_enabled = enabled;
        autoblacklist_info->aging_time = CS_DEFAULT_AGING_TIME;
        autoblacklist_info->reset_time = CS_DEFAULT_RESET_TIME;
        autoblacklist_info->threshold = CS_DEFAULT_THRESHOLD;
    } else {
        hmac_blacklist_init(mac_vap, CS_BLACKLIST_MODE_NONE, OAL_TRUE); /* 关闭自动黑名单，清零相关数据 */
    }

    oam_info_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_autoblacklist_enable:autoblacklist is %d}", enabled);

    return OAL_SUCC;
}


uint32_t hmac_autoblacklist_set_aging(mac_vap_stru *mac_vap, uint32_t aging_time)
{
    mac_autoblacklist_info_stru *autoblacklist_info = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    /* 1.1 入参检查 */
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_autoblacklist_set_aging::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 2.1 获取参数 */
    if (aging_time == 0) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{aging_time should not be 0}");
        return OAL_ERR_CODE_SECURITY_AGING_INVALID;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_autoblacklist_set_aging::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_vap->pst_blacklist_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_autoblacklist_set_aging::blacklist_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    autoblacklist_info = &(hmac_vap->pst_blacklist_info->st_autoblacklist_info);

    /* 3.1 自动黑名单没有使能 */
    if (autoblacklist_info->uc_enabled == 0) {
        hmac_autoblacklist_enable(mac_vap, 1);
    }

    /* 4.1 更新老化时间 */
    autoblacklist_info->aging_time = aging_time;

    oam_info_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{blacklist set auto aging = %d}", aging_time);
    return OAL_SUCC;
}


uint32_t hmac_autoblacklist_set_threshold(mac_vap_stru *mac_vap, uint32_t threshold)
{
    mac_autoblacklist_info_stru *autoblacklist_info = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    /* 1.1 入参检查 */
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_autoblacklist_set_threshold::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 2.1 获取参数 */
    if (threshold == 0) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{threshold should not be 0}");
        return OAL_ERR_CODE_SECURITY_THRESHOLD_INVALID;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_autoblacklist_set_threshold::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_vap->pst_blacklist_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_autoblacklist_set_threshold::blacklist_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    autoblacklist_info = &(hmac_vap->pst_blacklist_info->st_autoblacklist_info);

    /* 3.1 自动黑名单没有使能 */
    if (autoblacklist_info->uc_enabled == 0) {
        hmac_autoblacklist_enable(mac_vap, 1);
    }

    /* 4.1 更新门限 */
    autoblacklist_info->threshold = threshold;

    oam_info_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{blacklist set auto threshold = %d}", threshold);
    return OAL_SUCC;
}


uint32_t hmac_autoblacklist_set_reset_time(mac_vap_stru *mac_vap, uint32_t reset_time)
{
    mac_autoblacklist_info_stru *autoblacklist_info = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    /* 1.1 入参检查 */
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_autoblacklist_set_reset_time::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 2.1 获取参数 */
    if (reset_time == 0) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{ul_aging_time should not be 0}");
        return OAL_ERR_CODE_SECURITY_RESETIME_INVALID;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_autoblacklist_set_reset_time::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_vap->pst_blacklist_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_autoblacklist_set_reset_time::blacklist_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    autoblacklist_info = &(hmac_vap->pst_blacklist_info->st_autoblacklist_info);

    /* 3.1 自动黑名单没有使能 */
    if (autoblacklist_info->uc_enabled == 0) {
        hmac_autoblacklist_enable(mac_vap, 1);
    }

    /* 4.1 更新重置时间 */
    autoblacklist_info->reset_time = reset_time;

    oam_info_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{blacklist set auto reset_time = %d}", reset_time);
    return OAL_SUCC;
}


oal_bool_enum_uint8 hmac_blacklist_filter(mac_vap_stru *mac_vap, uint8_t *mac_addr)
{
    mac_blacklist_stru *blacklist = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_blacklist_info_stru *blacklist_info = NULL;
    oal_bool_enum_uint8 is_aged;

    /* 1.1 入参检查 */
    if (oal_any_null_ptr2(mac_vap, mac_addr)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_blacklist_filter::null mac_vap or null mac addr}");
        return OAL_FALSE;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_blacklist_filter::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_vap->pst_blacklist_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_blacklist_filter::blacklist_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    blacklist_info = hmac_vap->pst_blacklist_info;

    /* 1.1 功能没有开启，不需要过滤 */
    if (blacklist_info->uc_mode == CS_BLACKLIST_MODE_NONE) {
        return OAL_FALSE;
    }

    /* 2.1 黑名单模式下 */
    if (blacklist_info->uc_mode == CS_BLACKLIST_MODE_BLACK) {
        blacklist = hmac_blacklist_get(mac_vap, mac_addr);
        if (blacklist == NULL) {
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_ANY,
                             "{hmac_blacklist_filter::there is no MAC=x.x.x.%02x.%02x.%02x in blacklist}",
                             mac_addr[MAC_ADDR_3], mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5]);
            return OAL_FALSE;
        }

        /* 2.3 老化时间到了，不需要过滤 */
        is_aged = hmac_blacklist_is_aged(mac_vap, blacklist);
        if (is_aged == OAL_TRUE) {
            return OAL_FALSE;
        }
        /* 2.4 其他情况下都需要过滤 */
        blacklist->drop_counter++;
        oam_info_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                      "{hmac_blacklist_filter::blacklist_filter drop_counter = %d MAC:=x.x.x.%02x.%02x.%02x}",
                      blacklist->drop_counter, mac_addr[MAC_ADDR_3], mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5]);
        return OAL_TRUE;
    }

    /* 3.1 白名单模式下 */
    if (blacklist_info->uc_mode == CS_BLACKLIST_MODE_WHITE) {
        blacklist = hmac_blacklist_get(mac_vap, mac_addr);
        if (blacklist == NULL) {
            oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                             "{hmac_blacklist_filter::there is no MAC:=%02X:XX:XX:%02X:%02X:%02X}",
                             mac_addr[0], mac_addr[MAC_ADDR_3], mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5]);
            return OAL_TRUE;
        }

        /* 3.3 老化时间到了，不需要过滤 */
        is_aged = hmac_blacklist_is_aged(mac_vap, blacklist);
        if (is_aged == OAL_TRUE) {
            oam_info_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                          "{hmac_blacklist_filter::aging reach. go through=%02X:XX:XX:%02X:%02X:%02X}",
                          mac_addr[0], mac_addr[MAC_ADDR_3], mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5]);
            return OAL_FALSE;
        }

        /* 3.4 老化时间没有到，需要过滤 */
        if (blacklist->aging_time != 0) {
            oam_info_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                          "{hmac_blacklist_filter::aging not zero; whilte_filter MAC=%02X:XX:XX:%02X:%02X:%02X}",
                          mac_addr[0], mac_addr[MAC_ADDR_3], mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5]);

            return OAL_TRUE;
        }
    }
    /* 不满足黑白名单过滤条件的，均不需要过滤 */
    return OAL_FALSE;
}


void hmac_autoblacklist_filter(mac_vap_stru *mac_vap, uint8_t *mac_addr)
{
    mac_autoblacklist_stru *auto_blacklist = NULL;
    mac_autoblacklist_info_stru *auto_blacklist_info = NULL;
    uint32_t ret;
    hmac_vap_stru *hmac_vap = NULL;

    /* 1.1 入参检查 */
    if (oal_any_null_ptr2(mac_vap, mac_addr)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_autoblacklist_filter::null mac_vap or null mac addr}");
        return;
    }
    if ((mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_autoblacklist_filter::not AP or STA mode!}");
        return;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL || hmac_vap->pst_blacklist_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_autoblacklist_filter::hmac_vap null.}");
        return;
    }
    auto_blacklist_info = &(hmac_vap->pst_blacklist_info->st_autoblacklist_info);
    /* 1.1 功能没有开启 */
    if (auto_blacklist_info->uc_enabled == 0) {
        return;
    }

    /* 1.2 检查是否已经在黑名单中, 若在黑名单中，直接返回 */
    if (hmac_blacklist_get(mac_vap, mac_addr) != NULL) {
        return;
    }

    oam_info_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                  "{hmac_autoblacklist_filter:: MAC:=%02X:XX:XX:%02X:%02X:%02X}",
                  mac_addr[0], mac_addr[MAC_ADDR_3], mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5]);

    /* 2.1 找到关联请求统计表单  */
    auto_blacklist = hmac_autoblacklist_get(mac_vap, mac_addr);
    /* 2.2 如果找不到表单，不处理 */
    if (auto_blacklist == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{Can't find item to add}");
        return;
    }

    /* 3.1 关联计数器判断 */
    if (++auto_blacklist->asso_counter > auto_blacklist_info->threshold) {
        oam_info_log2(mac_vap->uc_vap_id, OAM_SF_ANY,
                      "{hmac_autoblacklist_filter: asso_counter=%d. threshold=%d. add to blacklist}",
                      auto_blacklist->asso_counter, auto_blacklist_info->threshold);
        ret = hmac_blacklist_add(mac_vap, mac_addr, auto_blacklist_info->aging_time);
        if (ret != OAL_SUCC) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_blacklist_add failed %d}", ret);
            return;
        }
        /* 3.2 删除统计表单 */
        memset_s(auto_blacklist, sizeof(mac_autoblacklist_stru), 0, sizeof(mac_autoblacklist_stru));
        if (auto_blacklist_info->list_num > 0) {
            auto_blacklist_info->list_num--;
        }
    }
}
