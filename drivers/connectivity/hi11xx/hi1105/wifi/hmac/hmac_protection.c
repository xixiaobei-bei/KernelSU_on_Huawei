

/* 1 头文件包含 */
#include "hmac_user.h"
#include "hmac_main.h"
#include "hmac_vap.h"
#include "mac_mib.h"
#include "hmac_protection.h"
#include "mac_vap.h"
#include "mac_ie.h"
#include "hmac_config.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_PROTECTION_C


uint32_t hmac_user_protection_sync_data(mac_vap_stru *pst_mac_vap)
{
    mac_h2d_protection_stru st_h2d_prot;
    oal_bool_enum_uint8 en_lsig_txop_full_protection_activated;
    oal_bool_enum_uint8 en_non_gf_entities_present;
    oal_bool_enum_uint8 en_rifs_mode;
    oal_bool_enum_uint8 en_ht_protection;

    memset_s(&st_h2d_prot, sizeof(st_h2d_prot), 0, sizeof(st_h2d_prot));

    /* 更新vap的en_dot11NonGFEntitiesPresent字段 */
    en_non_gf_entities_present = (0 != pst_mac_vap->st_protection.uc_sta_non_gf_num) ? OAL_TRUE : OAL_FALSE;
    mac_mib_set_NonGFEntitiesPresent(pst_mac_vap, en_non_gf_entities_present);

    /* 更新vap的en_dot11LSIGTXOPFullProtectionActivated字段 */
    en_lsig_txop_full_protection_activated =
        (pst_mac_vap->st_protection.uc_sta_no_lsig_txop_num == 0) ? OAL_TRUE : OAL_FALSE;
    mac_mib_set_LsigTxopFullProtectionActivated(pst_mac_vap, en_lsig_txop_full_protection_activated);

    /* 更新vap的en_dot11HTProtection和en_dot11RIFSMode字段 */
    if (pst_mac_vap->st_protection.uc_sta_non_ht_num != 0) {
        en_ht_protection = WLAN_MIB_HT_NON_HT_MIXED;
        en_rifs_mode = OAL_FALSE;
    } else if (pst_mac_vap->st_protection.bit_obss_non_ht_present == OAL_TRUE) {
        en_ht_protection = WLAN_MIB_HT_NONMEMBER_PROTECTION;
        en_rifs_mode = OAL_FALSE;
    } else if ((pst_mac_vap->st_channel.en_bandwidth != WLAN_BAND_WIDTH_20M) &&
               (pst_mac_vap->st_protection.uc_sta_20M_only_num != 0)) {
        en_ht_protection = WLAN_MIB_HT_20MHZ_PROTECTION;
        en_rifs_mode = OAL_TRUE;
    } else {
        en_ht_protection = WLAN_MIB_HT_NO_PROTECTION;
        en_rifs_mode = OAL_TRUE;
    }

    mac_mib_set_HtProtection(pst_mac_vap, en_ht_protection);
    mac_mib_set_RifsMode(pst_mac_vap, en_rifs_mode);

    memcpy_s((uint8_t *)&st_h2d_prot.st_protection,
             sizeof(mac_protection_stru),
             (uint8_t *)&pst_mac_vap->st_protection,
             sizeof(mac_protection_stru));

    st_h2d_prot.en_dot11HTProtection = mac_mib_get_HtProtection(pst_mac_vap);
    st_h2d_prot.en_dot11RIFSMode = mac_mib_get_RifsMode(pst_mac_vap);
    st_h2d_prot.en_dot11LSIGTXOPFullProtectionActivated = mac_mib_get_LsigTxopFullProtectionActivated(pst_mac_vap);
    st_h2d_prot.en_dot11NonGFEntitiesPresent = mac_mib_get_NonGFEntitiesPresent(pst_mac_vap);

    return hmac_protection_update_from_user(pst_mac_vap, sizeof(st_h2d_prot), (uint8_t *)&st_h2d_prot);
}


OAL_STATIC void hmac_protection_del_user_stat_legacy_ap(mac_vap_stru *pst_mac_vap,
    mac_user_stru *pst_mac_user)
{
    mac_protection_stru *pst_protection = &(pst_mac_vap->st_protection);
    hmac_user_stru *pst_hmac_user;

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(pst_mac_user->us_assoc_id);
    if (pst_hmac_user == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "hmac_protection_del_user_stat_legacy_ap::Get Hmac_user(idx=%d) NULL POINT!",
                       pst_mac_user->us_assoc_id);
        return;
    }

    /* 如果去关联的站点不支持ERP */
    if ((pst_hmac_user->st_hmac_cap_info.bit_erp == OAL_FALSE) &&
        (pst_hmac_user->st_user_stats_flag.bit_no_erp_stats_flag == OAL_TRUE) &&
        (pst_protection->uc_sta_non_erp_num != 0)) {
        pst_protection->uc_sta_non_erp_num--;
    }

    /* 如果去关联的站点不支持short preamble */
    if ((pst_hmac_user->st_hmac_cap_info.bit_short_preamble == OAL_FALSE) &&
        (pst_hmac_user->st_user_stats_flag.bit_no_short_preamble_stats_flag == OAL_TRUE) &&
        (pst_protection->uc_sta_no_short_preamble_num != 0)) {
        pst_protection->uc_sta_no_short_preamble_num--;
    }

    /* 如果去关联的站点不支持short slot */
    if ((pst_hmac_user->st_hmac_cap_info.bit_short_slot_time == OAL_FALSE) &&
        (pst_hmac_user->st_user_stats_flag.bit_no_short_slot_stats_flag == OAL_TRUE) &&
        (pst_protection->uc_sta_no_short_slot_num != 0)) {
        pst_protection->uc_sta_no_short_slot_num--;
    }

    pst_hmac_user->st_user_stats_flag.bit_no_short_slot_stats_flag = OAL_FALSE;
    pst_hmac_user->st_user_stats_flag.bit_no_short_preamble_stats_flag = OAL_FALSE;
    pst_hmac_user->st_user_stats_flag.bit_no_erp_stats_flag = OAL_FALSE;

    return;
}


OAL_STATIC void hmac_protection_del_user_stat_ht_ap(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    mac_user_ht_hdl_stru *pst_ht_hdl = &(pst_mac_user->st_ht_hdl);
    mac_protection_stru *pst_protection = &(pst_mac_vap->st_protection);
    hmac_user_stru *pst_hmac_user;

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(pst_mac_user->us_assoc_id);
    if (pst_hmac_user == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "hmac_protection_del_user_stat_ht_ap::Get Hmac_user(idx=%d) NULL POINT!",
                       pst_mac_user->us_assoc_id);
        return;
    }

    /* 如果去关联的站点不支持HT */
    if ((pst_ht_hdl->en_ht_capable == OAL_FALSE) &&
        (pst_hmac_user->st_user_stats_flag.bit_no_ht_stats_flag == OAL_TRUE) &&
        (pst_protection->uc_sta_non_ht_num != 0)) {
        pst_protection->uc_sta_non_ht_num--;
    } else { /* 支持HT */
        /* 如果去关联的站点不支持20/40Mhz频宽 */
        if ((pst_ht_hdl->bit_supported_channel_width == OAL_FALSE) &&
            (pst_hmac_user->st_user_stats_flag.bit_20M_only_stats_flag == OAL_TRUE) &&
            (pst_protection->uc_sta_20M_only_num != 0)) {
            pst_protection->uc_sta_20M_only_num--;
        }

        /* 如果去关联的站点不支持GF */
        if ((pst_ht_hdl->bit_ht_green_field == OAL_FALSE) &&
            (pst_hmac_user->st_user_stats_flag.bit_no_gf_stats_flag == OAL_TRUE) &&
            (pst_protection->uc_sta_non_gf_num != 0)) {
            pst_protection->uc_sta_non_gf_num--;
        }

        /* 如果去关联的站点不支持L-SIG TXOP Protection */
        if ((pst_ht_hdl->bit_lsig_txop_protection == OAL_FALSE) &&
            (pst_hmac_user->st_user_stats_flag.bit_no_lsig_txop_stats_flag == OAL_TRUE) &&
            (pst_protection->uc_sta_no_lsig_txop_num != 0)) {
            pst_protection->uc_sta_no_lsig_txop_num--;
        }

        /* 如果去关联的站点不支持40Mhz cck */
        if ((pst_ht_hdl->bit_dsss_cck_mode_40mhz == OAL_FALSE) &&
            (pst_ht_hdl->bit_supported_channel_width == OAL_TRUE) &&
            (pst_hmac_user->st_user_stats_flag.bit_no_40dsss_stats_flag == OAL_TRUE) &&
            (pst_protection->uc_sta_no_40dsss_cck_num != 0)) {
            pst_protection->uc_sta_no_40dsss_cck_num--;
        }
    }

    pst_hmac_user->st_user_stats_flag.bit_no_ht_stats_flag = OAL_FALSE;
    pst_hmac_user->st_user_stats_flag.bit_no_gf_stats_flag = OAL_FALSE;
    pst_hmac_user->st_user_stats_flag.bit_20M_only_stats_flag = OAL_FALSE;
    pst_hmac_user->st_user_stats_flag.bit_no_40dsss_stats_flag = OAL_FALSE;
    pst_hmac_user->st_user_stats_flag.bit_no_lsig_txop_stats_flag = OAL_FALSE;

    return;
}


OAL_STATIC void hmac_protection_del_user_stat_ap(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    hmac_protection_del_user_stat_legacy_ap(pst_mac_vap, pst_mac_user);
    hmac_protection_del_user_stat_ht_ap(pst_mac_vap, pst_mac_user);
}


uint32_t hmac_protection_del_user(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    uint32_t ret = OAL_SUCC;

    if (oal_any_null_ptr2(pst_mac_vap, pst_mac_user)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* AP 更新VAP结构体统计量，更新保护机制 */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 删除保护模式相关user统计 */
        hmac_protection_del_user_stat_ap(pst_mac_vap, pst_mac_user);
    }

    return ret;
}