

#ifndef __MAC_MIB_H__
#define __MAC_MIB_H__

#include "mac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_MIB_H

/*****************************************************************************
 * mib操作函数
 *****************************************************************************/
uint32_t mac_mib_get_beacon_period(mac_vap_stru *mac_vap, uint8_t *len, uint8_t *param);
uint32_t mac_mib_get_dtim_period(mac_vap_stru *mac_vap, uint8_t *len, uint8_t *param);
uint32_t mac_mib_get_bss_type(mac_vap_stru *mac_vap, uint8_t *len, uint8_t *param);
uint32_t mac_mib_get_ssid(mac_vap_stru *mac_vap, uint8_t *len, uint8_t *param);
uint32_t mac_mib_set_beacon_period(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
uint32_t mac_mib_set_dtim_period(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
uint32_t mac_mib_set_bss_type(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
uint32_t mac_mib_set_ssid(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
uint32_t mac_mib_set_station_id(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
uint32_t mac_mib_get_shpreamble(mac_vap_stru *mac_vap, uint8_t *len, uint8_t *param);
uint32_t mac_mib_set_shpreamble(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
uint32_t mac_vap_set_bssid(mac_vap_stru *mac_vap, uint8_t *bssid);

void mac_vap_init_mib_11ax_1103(mac_vap_stru *mac_vap, uint32_t nss_num);
void mac_vap_init_mib_11ax(mac_vap_stru *mac_vap);
void mac_vap_init_mib_11ax_1105(mac_vap_stru *mac_vap, uint32_t nss_num);
void mac_vap_init_mib_11ax_1106(mac_vap_stru *mac_vap, uint32_t nss_num);
void mac_init_mib_extend(mac_vap_stru *mac_vap);
void mac_vap_init_mib_11n_txbf(mac_vap_stru *mac_vap);
void mac_init_mib(mac_vap_stru *mac_vap);

void mac_vap_change_mib_by_bandwidth(mac_vap_stru *mac_vap,
    wlan_channel_band_enum_uint8 band, wlan_channel_bandwidth_enum_uint8 bandwidth);
void mac_mib_set_wep(mac_vap_stru *mac_vap, uint8_t key_id, uint8_t key_value);
#ifdef _PRE_WLAN_FEATURE_11R
uint32_t mac_mib_init_ft_cfg(mac_vap_stru *mac_vap, uint8_t *mde);
uint32_t mac_mib_get_md_id(mac_vap_stru *mac_vap, uint16_t *mdid);
#endif  // _PRE_WLAN_FEATURE_11R

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_pre_auth_actived(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAPreauthenticationActivated;
}

OAL_STATIC OAL_INLINE void mac_mib_set_pre_auth_actived(mac_vap_stru *mac_vap, oal_bool_enum_uint8 pre_auth)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAPreauthenticationActivated = pre_auth;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_rsnacfg_ptksareplaycounters(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.uc_dot11RSNAConfigPTKSAReplayCounters;
}

OAL_STATIC OAL_INLINE void mac_mib_set_rsnacfg_ptksareplaycounters(mac_vap_stru *mac_vap, uint8_t ptksa_replay_counters)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.uc_dot11RSNAConfigPTKSAReplayCounters =
        ptksa_replay_counters;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_rsnacfg_gtksareplaycounters(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.uc_dot11RSNAConfigGTKSAReplayCounters;
}

OAL_STATIC OAL_INLINE void mac_mib_set_rsnacfg_gtksareplaycounters(mac_vap_stru *mac_vap, uint8_t gtksa_replay_counters)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.uc_dot11RSNAConfigGTKSAReplayCounters =
        gtksa_replay_counters;
}

OAL_STATIC OAL_INLINE void mac_mib_init_rsnacfg_suites(mac_vap_stru *mac_vap)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.wpa_group_suite = MAC_WPA_CHIPER_TKIP;
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites[0] = MAC_WPA_CHIPER_CCMP;
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites[1] = MAC_WPA_CHIPER_TKIP;

    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites[0] = MAC_WPA_AKM_PSK;
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites[1] = MAC_WPA_AKM_PSK_SHA256;

    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.rsn_group_suite = MAC_RSN_CHIPER_CCMP;
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites[0] = MAC_RSN_CHIPER_CCMP;
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites[1] = MAC_RSN_CHIPER_TKIP;

    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites[0] = MAC_RSN_AKM_PSK;
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites[1] = MAC_RSN_AKM_PSK_SHA256;
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.rsn_group_mgmt_suite =
        MAC_RSN_CIPHER_SUITE_AES_128_CMAC;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_wpa_group_suite(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.wpa_group_suite;
}

OAL_STATIC OAL_INLINE void mac_mib_set_wpa_group_suite(mac_vap_stru *mac_vap, uint32_t suite)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.wpa_group_suite = suite;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_rsn_group_suite(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.rsn_group_suite;
}

OAL_STATIC OAL_INLINE void mac_mib_set_rsn_group_suite(mac_vap_stru *mac_vap, uint32_t suite)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.rsn_group_suite = suite;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_rsn_group_mgmt_suite(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.rsn_group_mgmt_suite;
}

OAL_STATIC OAL_INLINE void mac_mib_set_rsn_group_mgmt_suite(mac_vap_stru *mac_vap, uint32_t suite)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.rsn_group_mgmt_suite = suite;
}

OAL_STATIC OAL_INLINE void mac_mib_set_wpa_pair_suites(mac_vap_stru *mac_vap, uint32_t *suites)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites[0] = suites[0];
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites[1] = suites[1];
}

OAL_STATIC OAL_INLINE void mac_mib_set_rsn_pair_suites(mac_vap_stru *mac_vap, uint32_t *suites)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites[0] = suites[0];
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites[1] = suites[1];
}

OAL_STATIC OAL_INLINE void mac_mib_set_wpa_akm_suites(mac_vap_stru *mac_vap, uint32_t *suites)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites[0] = suites[0];
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites[1] = suites[1];
}

OAL_STATIC OAL_INLINE void mac_mib_set_rsn_akm_suites(mac_vap_stru *mac_vap, uint32_t *suites)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites[0] = suites[0];
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites[1] = suites[1];
}
OAL_STATIC OAL_INLINE uint32_t mac_mib_rsna_cfg_params_match_suites(
    uint32_t *suites, uint32_t *rsna_cfg_params, uint8_t cipher_len)
{
    uint8_t idx_local;
    uint8_t idx_peer;

    for (idx_local = 0; idx_local < cipher_len; idx_local++) {
        for (idx_peer = 0; idx_peer < cipher_len; idx_peer++) {
            if (rsna_cfg_params[idx_local] == suites[idx_peer]) {
                return suites[idx_peer];
            }
        }
    }
    return 0;
}
OAL_STATIC OAL_INLINE uint32_t mac_mib_wpa_pair_match_suites(mac_vap_stru *mac_vap, uint32_t *suites)
{
    return mac_mib_rsna_cfg_params_match_suites(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites,
        WLAN_PAIRWISE_CIPHER_SUITES);
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_rsn_pair_match_suites(mac_vap_stru *mac_vap, uint32_t *suites)
{
    return mac_mib_rsna_cfg_params_match_suites(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites,
        WLAN_PAIRWISE_CIPHER_SUITES);
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_wpa_akm_match_suites(mac_vap_stru *mac_vap, uint32_t *suites)
{
    return mac_mib_rsna_cfg_params_match_suites(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites,
        WLAN_PAIRWISE_CIPHER_SUITES);
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_rsn_akm_match_suites(mac_vap_stru *mac_vap, uint32_t *suites)
{
    return mac_mib_rsna_cfg_params_match_suites(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites,
        WLAN_PAIRWISE_CIPHER_SUITES);
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_wpa_pair_match_suites_s(mac_vap_stru *mac_vap,
    uint32_t *suites, uint8_t cipher_len)
{
    return mac_mib_rsna_cfg_params_match_suites(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites,
        cipher_len / sizeof(uint32_t));
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_rsn_pair_match_suites_s(mac_vap_stru *mac_vap,
    uint32_t *suites, uint8_t cipher_len)
{
    return mac_mib_rsna_cfg_params_match_suites(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites,
        cipher_len / sizeof(uint32_t));
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_wpa_akm_match_suites_s(mac_vap_stru *mac_vap,
    uint32_t *suites, uint8_t cipher_len)
{
    return mac_mib_rsna_cfg_params_match_suites(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites,
        cipher_len / sizeof(uint32_t));
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_rsn_akm_match_suites_s(mac_vap_stru *mac_vap,
    uint32_t *suites, uint8_t cipher_len)
{
    return mac_mib_rsna_cfg_params_match_suites(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites,
        cipher_len / sizeof(uint32_t));
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_rsna_cfg_params(
    uint32_t *suites, uint32_t *rsna_cfg_params, uint8_t cipher_len)
{
    uint8_t idx;
    uint8_t nums = 0;

    for (idx = 0; idx < cipher_len; idx++) {
        if (rsna_cfg_params[idx] != 0) {
            suites[nums++] = rsna_cfg_params[idx];
        }
    }
    return nums;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_wpa_pair_suites(mac_vap_stru *mac_vap, uint32_t *suites)
{
    return mac_mib_get_rsna_cfg_params(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites,
        WLAN_PAIRWISE_CIPHER_SUITES);
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_rsn_pair_suites(mac_vap_stru *mac_vap, uint32_t *suites)
{
    return mac_mib_get_rsna_cfg_params(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites,
        WLAN_PAIRWISE_CIPHER_SUITES);
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_wpa_akm_suites(mac_vap_stru *mac_vap, uint32_t *suites)
{
    return mac_mib_get_rsna_cfg_params(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites,
        WLAN_AUTHENTICATION_SUITES);
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_rsn_akm_suites(mac_vap_stru *mac_vap, uint32_t *suites)
{
    return mac_mib_get_rsna_cfg_params(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites,
        WLAN_AUTHENTICATION_SUITES);
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_wpa_pair_suites_s(mac_vap_stru *mac_vap,
    uint32_t *suites, uint8_t cipher_len)
{
    return mac_mib_get_rsna_cfg_params(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites,
        cipher_len / sizeof(uint32_t));
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_rsn_pair_suites_s(mac_vap_stru *mac_vap,
    uint32_t *suites, uint8_t cipher_len)
{
    return mac_mib_get_rsna_cfg_params(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites,
        cipher_len / sizeof(uint32_t));
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_wpa_akm_suites_s(mac_vap_stru *mac_vap,
    uint32_t *suites, uint8_t cipher_len)
{
    return mac_mib_get_rsna_cfg_params(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites,
        cipher_len / sizeof(uint32_t));
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_rsn_akm_suites_s(mac_vap_stru *mac_vap,
    uint32_t *suites, uint8_t cipher_len)
{
    return mac_mib_get_rsna_cfg_params(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites,
        cipher_len / sizeof(uint32_t));
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_maxmpdu_length(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.dot11MaxMPDULength;
}

OAL_STATIC OAL_INLINE void mac_mib_set_maxmpdu_length(mac_vap_stru *mac_vap, uint32_t maxmpdu_length)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.dot11MaxMPDULength = maxmpdu_length;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_vht_max_rx_ampdu_factor(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.dot11VHTMaxRxAMPDUFactor;
}

OAL_STATIC OAL_INLINE void mac_mib_set_vht_max_rx_ampdu_factor(mac_vap_stru *mac_vap,
    uint32_t vht_max_rx_ampdu_factor)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.dot11VHTMaxRxAMPDUFactor = vht_max_rx_ampdu_factor;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_vht_ctrl_field_cap(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTControlFieldSupported;
}

OAL_STATIC OAL_INLINE void mac_mib_set_vht_ctrl_field_cap(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 vht_ctrl_field_supported)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTControlFieldSupported =
        vht_ctrl_field_supported;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_txopps(mac_vap_stru *pst_vap)
{
    return pst_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTTXOPPowerSaveOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_txopps(mac_vap_stru *pst_vap, oal_bool_enum_uint8 vht_txop_ps)
{
    pst_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTTXOPPowerSaveOptionImplemented = vht_txop_ps;
}

OAL_STATIC OAL_INLINE uint16_t mac_mib_get_vht_rx_mcs_map(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTRxMCSMap;
}

OAL_STATIC OAL_INLINE void *mac_mib_get_ptr_vht_rx_mcs_map(mac_vap_stru *mac_vap)
{
    return &(mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTRxMCSMap);
}

OAL_STATIC OAL_INLINE void mac_mib_set_vht_rx_mcs_map(mac_vap_stru *mac_vap, uint16_t vht_mcs_mpa)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTRxMCSMap = vht_mcs_mpa;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_us_rx_highest_rate(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.dot11VHTRxHighestDataRateSupported;
}

OAL_STATIC OAL_INLINE void mac_mib_set_us_rx_highest_rate(mac_vap_stru *mac_vap, uint32_t rx_highest_rate)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.dot11VHTRxHighestDataRateSupported = rx_highest_rate;
}

OAL_STATIC OAL_INLINE uint16_t mac_mib_get_vht_tx_mcs_map(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTTxMCSMap;
}

OAL_STATIC OAL_INLINE void *mac_mib_get_ptr_vht_tx_mcs_map(mac_vap_stru *mac_vap)
{
    return &(mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTTxMCSMap);
}

OAL_STATIC OAL_INLINE void mac_mib_set_vht_tx_mcs_map(mac_vap_stru *mac_vap, uint16_t vht_mcs_mpa)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTTxMCSMap = vht_mcs_mpa;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_us_tx_highest_rate(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.dot11VHTTxHighestDataRateSupported;
}

OAL_STATIC OAL_INLINE void mac_mib_set_us_tx_highest_rate(mac_vap_stru *mac_vap, uint32_t tx_highest_rate)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.dot11VHTTxHighestDataRateSupported = tx_highest_rate;
}

#ifdef _PRE_WLAN_FEATURE_SMPS
OAL_STATIC OAL_INLINE wlan_mib_mimo_power_save_enum mac_mib_get_smps(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MIMOPowerSave;
}

OAL_STATIC OAL_INLINE void mac_mib_set_smps(mac_vap_stru *mac_vap, wlan_mib_mimo_power_save_enum sm_power_save)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MIMOPowerSave =  sm_power_save;
}
#endif  // _PRE_WLAN_FEATURE_SMPS

OAL_STATIC OAL_INLINE wlan_mib_max_amsdu_lenth_enum mac_mib_get_max_amsdu_length(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MaxAMSDULength;
}

OAL_STATIC OAL_INLINE void mac_mib_set_max_amsdu_length(mac_vap_stru *mac_vap,
    wlan_mib_max_amsdu_lenth_enum max_amsdu_length)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MaxAMSDULength = max_amsdu_length;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_lsig_txop_protection(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11LsigTxopProtectionOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_lsig_txop_protection(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 lsig_txop_protection)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11LsigTxopProtectionOptionImplemented = lsig_txop_protection;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_max_ampdu_len_exponent(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.dot11MaxRxAMPDUFactor;
}

OAL_STATIC OAL_INLINE void mac_mib_set_max_ampdu_len_exponent(mac_vap_stru *mac_vap, uint32_t max_ampdu_len_exponent)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.dot11MaxRxAMPDUFactor = max_ampdu_len_exponent;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_min_mpdu_start_spacing(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.dot11MinimumMPDUStartSpacing;
}

OAL_STATIC OAL_INLINE void mac_mib_set_min_mpdu_start_spacing(mac_vap_stru *mac_vap, uint8_t min_mpdu_start_spacing)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.dot11MinimumMPDUStartSpacing = min_mpdu_start_spacing;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_pco_option_implemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11PCOOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_pco_option_implemented(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 pco_option_implemented)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11PCOOptionImplemented = pco_option_implemented;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_transition_time(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.dot11TransitionTime;
}

OAL_STATIC OAL_INLINE void mac_mib_set_transition_time(mac_vap_stru *mac_vap, uint32_t transition_time)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.dot11TransitionTime = transition_time;
}

OAL_STATIC OAL_INLINE wlan_mib_mcs_feedback_opt_implt_enum mac_mib_get_mcs_fdbk(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MCSFeedbackOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_mcs_fdbk(mac_vap_stru *mac_vap,
    wlan_mib_mcs_feedback_opt_implt_enum mcs_fdbk)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MCSFeedbackOptionImplemented = mcs_fdbk;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_htc_sup(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11HTControlFieldSupported;
}

OAL_STATIC OAL_INLINE void mac_mib_set_htc_sup(mac_vap_stru *mac_vap, oal_bool_enum_uint8 htc_sup)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11HTControlFieldSupported = htc_sup;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_rd_rsp(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11RDResponderOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_rd_rsp(mac_vap_stru *mac_vap, oal_bool_enum_uint8 rd_rsp)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11RDResponderOptionImplemented = rd_rsp;
}

#if defined(_PRE_WLAN_FEATURE_11R)
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ft_trainsistion(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FastBSSTransitionActivated;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ft_trainsistion(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 ft_trainsistion)
{
    mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FastBSSTransitionActivated = ft_trainsistion;
}

OAL_STATIC OAL_INLINE uint16_t mac_mib_get_ft_mdid(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.us_dot11FTMobilityDomainID;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ft_mdid(mac_vap_stru *mac_vap, uint16_t ft_mdid)
{
    mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.us_dot11FTMobilityDomainID = ft_mdid;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ft_over_ds(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FTOverDSActivated;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ft_over_ds(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 ft_over_ds)
{
    mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FTOverDSActivated = ft_over_ds;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ft_resource_req(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FTResourceRequestSupported;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ft_resource_req(mac_vap_stru *mac_vap, oal_bool_enum_uint8 ft_resource_req)
{
    mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FTResourceRequestSupported = ft_resource_req;
}
#endif  // _PRE_WLAN_FEATURE_11R

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_privacyinvoked(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11PrivacyInvoked;
}

OAL_STATIC OAL_INLINE void mac_mib_set_privacyinvoked(mac_vap_stru *mac_vap, oal_bool_enum_uint8 privacyinvoked)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11PrivacyInvoked = privacyinvoked;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_rsnaactivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAActivated;
}

OAL_STATIC OAL_INLINE void mac_mib_set_rsnaactivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 rsnaactivated)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAActivated = rsnaactivated;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_wep_enabled(mac_vap_stru *mac_vap)
{
    if (mac_vap == NULL) {
        return OAL_FALSE;
    }

    if ((mac_mib_get_privacyinvoked(mac_vap) == OAL_FALSE) ||
        (mac_mib_get_rsnaactivated(mac_vap) == OAL_TRUE)) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_wep_allowed(mac_vap_stru *mac_vap)
{
    if (mac_mib_get_rsnaactivated(mac_vap) == OAL_TRUE) {
        return OAL_FALSE;
    } else {
        return mac_is_wep_enabled(mac_vap);
    }
}

OAL_STATIC OAL_INLINE void mac_set_wep_default_keyid(mac_vap_stru *mac_vap, uint8_t default_key_id)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.uc_dot11WEPDefaultKeyID = default_key_id;
}

OAL_STATIC OAL_INLINE uint8_t mac_get_wep_default_keyid(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_privacy.uc_dot11WEPDefaultKeyID);
}

OAL_STATIC OAL_INLINE uint8_t mac_get_wep_default_keysize(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_privacy.ast_wlan_mib_wep_dflt_key[mac_get_wep_default_keyid(mac_vap)].\
        auc_dot11WEPDefaultKeyValue[WLAN_WEP_SIZE_OFFSET]);
}

OAL_STATIC OAL_INLINE uint8_t mac_get_wep_keysize(mac_vap_stru *mac_vap, uint8_t uc_idx)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_privacy.ast_wlan_mib_wep_dflt_key[uc_idx].\
        auc_dot11WEPDefaultKeyValue[WLAN_WEP_SIZE_OFFSET]);
}

OAL_STATIC OAL_INLINE wlan_ciper_protocol_type_enum_uint8 mac_get_wep_type(mac_vap_stru *mac_vap, uint8_t key_id)
{
    wlan_ciper_protocol_type_enum_uint8 cipher_type;

    switch (mac_get_wep_keysize(mac_vap, key_id)) {
        case 40: /* 40 keysize */
            cipher_type = WLAN_80211_CIPHER_SUITE_WEP_40;
            break;
        case 104: /* 104 keysize */
            cipher_type = WLAN_80211_CIPHER_SUITE_WEP_104;
            break;
        default:
            cipher_type = WLAN_80211_CIPHER_SUITE_WEP_40;
            break;
    }
    return cipher_type;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_tkip_only(mac_vap_stru *mac_vap)
{
    uint8_t pair_suites_num;
    uint32_t pcip[WLAN_PAIRWISE_CIPHER_SUITES] = { 0 };

    if (mac_vap == NULL) {
        return OAL_FALSE;
    }

    if ((mac_vap->st_cap_flag.bit_wpa2 == OAL_FALSE) && (mac_vap->st_cap_flag.bit_wpa == OAL_FALSE)) {
        return OAL_FALSE;
    }

    if ((mac_mib_get_privacyinvoked(mac_vap) == OAL_FALSE) ||
        (mac_mib_get_rsnaactivated(mac_vap) == OAL_FALSE)) { // 不加密或者是WEP加密时，返回false
        return OAL_FALSE;
    }

    if (mac_vap->st_cap_flag.bit_wpa == OAL_TRUE) {
        pair_suites_num = mac_mib_get_wpa_pair_suites_s(mac_vap, &pcip[0], sizeof(pcip));
        if ((pair_suites_num != 1) || (pcip[0] != MAC_WPA_CHIPER_TKIP)) { // wpa加密时，若非tkip only，返回false
            return OAL_FALSE;
        }
    }

    if (mac_vap->st_cap_flag.bit_wpa2 == OAL_TRUE) {
        pair_suites_num = mac_mib_get_rsn_pair_suites_s(mac_vap, &pcip[0], sizeof(pcip));
        if ((pair_suites_num != 1) || (pcip[0] != MAC_RSN_CHIPER_TKIP)) { // wpa2加密时，若非tkip only，返回false
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}

OAL_STATIC OAL_INLINE uint8_t *mac_mib_get_StationID(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID;
}

OAL_STATIC OAL_INLINE void mac_mib_set_StationID(mac_vap_stru *mac_vap, uint8_t *sta_id)
{
    oal_set_mac_addr(mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID, sta_id);
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_OBSSScanPassiveDwell(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanPassiveDwell;
}

OAL_STATIC OAL_INLINE void mac_mib_set_OBSSScanPassiveDwell(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanPassiveDwell = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_OBSSScanActiveDwell(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanActiveDwell;
}

OAL_STATIC OAL_INLINE void mac_mib_set_OBSSScanActiveDwell(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanActiveDwell = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_BSSWidthTriggerScanInterval(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11BSSWidthTriggerScanInterval;
}

OAL_STATIC OAL_INLINE void mac_mib_set_BSSWidthTriggerScanInterval(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11BSSWidthTriggerScanInterval = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_OBSSScanPassiveTotalPerChannel(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanPassiveTotalPerChannel;
}

OAL_STATIC OAL_INLINE void mac_mib_set_OBSSScanPassiveTotalPerChannel(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanPassiveTotalPerChannel = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_OBSSScanActiveTotalPerChannel(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanActiveTotalPerChannel;
}

OAL_STATIC OAL_INLINE void mac_mib_set_OBSSScanActiveTotalPerChannel(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanActiveTotalPerChannel = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_BSSWidthChannelTransitionDelayFactor(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11BSSWidthChannelTransitionDelayFactor;
}

OAL_STATIC OAL_INLINE void mac_mib_set_BSSWidthChannelTransitionDelayFactor(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11BSSWidthChannelTransitionDelayFactor = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_OBSSScanActivityThreshold(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanActivityThreshold;
}

OAL_STATIC OAL_INLINE void mac_mib_set_OBSSScanActivityThreshold(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanActivityThreshold = value;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_HighThroughputOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11HighThroughputOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_HighThroughputOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11HighThroughputOptionImplemented = val;
}

OAL_STATIC OAL_INLINE void mac_mib_set_VHTOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_VHTOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTOptionImplemented;
}

#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
OAL_STATIC OAL_INLINE void mac_mib_set_HEOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11HEOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_HEOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11HEOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_OMIOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11OMIOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_OMIOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11OMIOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_TWTOptionActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11TWTOptionActivated = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_TWTOptionActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11TWTOptionActivated;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_OperatingModeIndication(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11OperatingModeIndication;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_OperatingModeIndication(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11OperatingModeIndication = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_BSRSupport(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11BSRSupport;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_BSRSupport(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11BSRSupport = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_BQRSupport(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11BQRSupport;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_BQRSupport(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11BQRSupport = val;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_OFDMARandomAccess(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11TOFDMARandomAccess = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_OFDMARandomAccess(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11TOFDMARandomAccess;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_TriggerMacPaddingDuration(mac_vap_stru *mac_vap, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.uc_dot11TriggerMacPaddingDuration = value;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_he_TriggerMacPaddingDuration(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.uc_dot11TriggerMacPaddingDuration;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_MultiBSSIDImplemented(mac_vap_stru *mac_vap, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11MultiBSSIDImplemented = value;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_he_MultiBSSIDImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11MultiBSSIDImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_MaxAMPDULengthExponent(mac_vap_stru *mac_vap, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.uc_dot11MaxAMPDULengthExponent = value;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_he_MaxAMPDULengthExponent(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.uc_dot11MaxAMPDULengthExponent;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_rx_mcs_map(mac_vap_stru *mac_vap, uint32_t he_mcs_map)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.dot11HERxMCSMap = he_mcs_map;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_he_rx_mcs_map(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.dot11HERxMCSMap;
}

OAL_STATIC OAL_INLINE void *mac_mib_get_ptr_he_rx_mcs_map(mac_vap_stru *mac_vap)
{
    return &(mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.dot11HERxMCSMap);
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_tx_mcs_map(mac_vap_stru *mac_vap, uint32_t he_mcs_map)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.dot11HETxMCSMap = he_mcs_map;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_he_tx_mcs_map(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.dot11HETxMCSMap;
}

OAL_STATIC OAL_INLINE void *mac_mib_get_ptr_he_tx_mcs_map(mac_vap_stru *mac_vap)
{
    return &(mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.dot11HETxMCSMap);
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_DualBandSupport(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HEDualBandSupport;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_DualBandSupport(mac_vap_stru *mac_vap, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HEDualBandSupport = value;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_LDPCCodingInPayload(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HELDPCCodingInPayload;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_LDPCCodingInPayload(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HELDPCCodingInPayload = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_STBCTxBelow80M(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HESTBCTxBelow80MHz;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_STBCTxBelow80M(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HESTBCTxBelow80MHz = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_STBCRxBelow80M(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HESTBCRxBelow80MHz;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_STBCRxBelow80M(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HESTBCRxBelow80MHz = val;
}


OAL_STATIC OAL_INLINE void mac_mib_set_he_SUBeamformer(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11SUBeamformer = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_SUBeamformer(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11SUBeamformer;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_SUBeamformee(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11SUBeamformee = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_SUBeamformee(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11SUBeamformee;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_MUBeamformer(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11MUBeamformer = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_MUBeamformer(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11MUBeamformer;
}

#ifdef _PRE_WLAN_FEATURE_11AX
OAL_STATIC OAL_INLINE void mac_mib_set_he_BeamformeeSTSBelow80Mhz(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.uc_dot11HEBeamformeeSTSBelow80Mhz = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_BeamformeeSTSBelow80Mhz(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.uc_dot11HEBeamformeeSTSBelow80Mhz;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_BeamformeeSTSOver80Mhz(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.uc_dot11HEBeamformeeSTSOver80Mhz = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_BeamformeeSTSOver80Mhz(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.uc_dot11HEBeamformeeSTSOver80Mhz;
}

OAL_STATIC OAL_INLINE void mac_mib_set_HENumberSoundingDimensionsBelow80Mhz(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.uc_dot11HENumberSoundingDimensionsBelow80Mhz = val;
}
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_HENumberSoundingDimensionsBelow80Mhz(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.uc_dot11HENumberSoundingDimensionsBelow80Mhz;
}

OAL_STATIC OAL_INLINE void mac_mib_set_HENumberSoundingDimensionsOver80Mhz(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.uc_dot11HENumberSoundingDimensionsOver80Mhz = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_HENumberSoundingDimensionsOver80Mhz(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.uc_dot11HENumberSoundingDimensionsOver80Mhz;
}

OAL_STATIC OAL_INLINE void mac_mib_set_HENg16SUFeedback(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11Ng16SUFeedback = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_HENg16SUFeedback(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11Ng16SUFeedback;
}

OAL_STATIC OAL_INLINE void mac_mib_set_HENg16MUFeedback(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11Ng16MUFeedback = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_HENg16MUFeedback(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11Ng16MUFeedback;
}

OAL_STATIC OAL_INLINE void mac_mib_set_HECodebook42SUFeedback(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11Codebook42SUFeedback = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_HECodebook42SUFeedback(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11Codebook42SUFeedback;
}

OAL_STATIC OAL_INLINE void mac_mib_set_HECodebook75MUFeedback(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11Codebook75MUFeedback = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_HECodebook75MUFeedback(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11Codebook75MUFeedback;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_TriggeredSUBeamformingFeedback(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11TriggeredSUBeamformingFeedback = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_TriggeredSUBeamformingFeedback(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11TriggeredSUBeamformingFeedback;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_TriggeredCQIFeedback(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11TriggeredCQIFeedback = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_TriggeredCQIFeedback(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11TriggeredCQIFeedback;
}

OAL_STATIC OAL_INLINE void mac_mib_set_HESUPPDUwith1xHELTFand0point8GIlmplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11HESUPPDUwith1xHELTFand0point8GIlmplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_HESUPPDUwith1xHELTFand0point8GIlmplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11HESUPPDUwith1xHELTFand0point8GIlmplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_MultiBSSIDActived(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_sta_config.en_dot11MultiBSSIDActived = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_he_MultiBSSIDActived(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_sta_config.en_dot11MultiBSSIDActived;
}
#endif

OAL_STATIC OAL_INLINE void mac_mib_set_he_HEChannelWidthSet(mac_vap_stru *mac_vap, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HEChannelWidthSet = value;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_he_HEChannelWidthSet(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HEChannelWidthSet;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_HighestNSS(mac_vap_stru *mac_vap, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HighestNSS = value;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_he_HighestNSS(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HighestNSS;
}

OAL_STATIC OAL_INLINE void mac_mib_set_he_HighestMCS(mac_vap_stru *mac_vap, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HighestMCS = value;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_he_HighestMCS(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HighestMCS;
}
#endif

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_FortyMHzOperationImplemented(mac_vap_stru *mac_vap)
{
    return (mac_vap->st_channel.en_band == WLAN_BAND_2G) ? \
        mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GFortyMHzOperationImplemented : \
        mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GFortyMHzOperationImplemented;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_2GFortyMHzOperationImplemented(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GFortyMHzOperationImplemented);
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_5GFortyMHzOperationImplemented(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GFortyMHzOperationImplemented);
}

OAL_STATIC OAL_INLINE void mac_mib_set_FortyMHzOperationImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    if (mac_vap->st_channel.en_band == WLAN_BAND_2G) {
        mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GFortyMHzOperationImplemented = val;
    } else {
        mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GFortyMHzOperationImplemented = val;
    }
}

OAL_STATIC OAL_INLINE void mac_mib_set_2GFortyMHzOperationImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GFortyMHzOperationImplemented = val;
}

OAL_STATIC OAL_INLINE void mac_mib_set_5GFortyMHzOperationImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GFortyMHzOperationImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_SpectrumManagementImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_SpectrumManagementImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_FortyMHzIntolerant(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11FortyMHzIntolerant;
}

OAL_STATIC OAL_INLINE void mac_mib_set_FortyMHzIntolerant(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11FortyMHzIntolerant = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_2040BSSCoexistenceManagementSupport(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot112040BSSCoexistenceManagementSupport;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11RSNAMFPC(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAMFPC;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11RSNAMFPR(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAMFPR;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11RSNAMFPC(mac_vap_stru *mac_vap, oal_bool_enum_uint8 value)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAMFPC = value;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11RSNAMFPR(mac_vap_stru *mac_vap, oal_bool_enum_uint8 value)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAMFPR = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_dot11AssociationSAQueryMaximumTimeout(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11AssociationSAQueryMaximumTimeout;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_dot11AssociationSAQueryRetryTimeout(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11AssociationSAQueryRetryTimeout;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11AssociationSAQueryMaximumTimeout(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11AssociationSAQueryMaximumTimeout = value;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11AssociationSAQueryRetryTimeout(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11AssociationSAQueryRetryTimeout = value;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11ExtendedChannelSwitchActivated(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11ExtendedChannelSwitchActivated = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11ExtendedChannelSwitchActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11ExtendedChannelSwitchActivated;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11RadioMeasurementActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RadioMeasurementActivated = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11RadioMeasurementActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RadioMeasurementActivated;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11QBSSLoadImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QBSSLoadImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11QBSSLoadImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QBSSLoadImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11VHTExtendedNSSBWCapable(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    ((mac_vap_stru *)mac_vap)->en_dot11VHTExtendedNSSBWCapable = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11VHTExtendedNSSBWCapable(mac_vap_stru *mac_vap)
{
    return ((mac_vap_stru *)mac_vap)->en_dot11VHTExtendedNSSBWCapable;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11APSDOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11APSDOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11APSDOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11APSDOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11DelayedBlockAckOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11DelayedBlockAckOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11DelayedBlockAckOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11DelayedBlockAckOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11ImmediateBlockAckOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11ImmediateBlockAckOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11ImmediateBlockAckOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11ImmediateBlockAckOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11QosOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QosOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11QosOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QosOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11MultiDomainCapabilityActivated(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11MultiDomainCapabilityActivated = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11MultiDomainCapabilityActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11MultiDomainCapabilityActivated;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11AssociationResponseTimeOut(mac_vap_stru *mac_vap, uint32_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11AssociationResponseTimeOut = val;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_dot11AssociationResponseTimeOut(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11AssociationResponseTimeOut;
}

OAL_STATIC OAL_INLINE void mac_mib_set_MaxAssocUserNums(mac_vap_stru *mac_vap, uint16_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.us_dot11MaxAssocUserNums = val;
}

OAL_STATIC OAL_INLINE uint16_t mac_mib_get_MaxAssocUserNums(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.us_dot11MaxAssocUserNums;
}

OAL_STATIC OAL_INLINE void mac_mib_set_SupportRateSetNums(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11SupportRateSetNums = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_SupportRateSetNums(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11SupportRateSetNums;
}

OAL_STATIC OAL_INLINE void mac_mib_set_CfgAmsduTxAtive(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11CfgAmsduTxAtive = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_CfgAmsduTxAtive(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11CfgAmsduTxAtive;
}

OAL_STATIC OAL_INLINE void mac_mib_set_AmsduAggregateAtive(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AmsduAggregateAtive = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_AmsduAggregateAtive(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AmsduAggregateAtive;
}

OAL_STATIC OAL_INLINE void mac_mib_set_AmsduPlusAmpduActive(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AmsduPlusAmpduActive = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_AmsduPlusAmpduActive(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AmsduPlusAmpduActive;
}

OAL_STATIC OAL_INLINE void mac_mib_set_WPSActive(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11WPSActive = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_WPSActive(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11WPSActive;
}

OAL_STATIC OAL_INLINE void mac_mib_set_2040SwitchProhibited(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot112040SwitchProhibited = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_2040SwitchProhibited(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot112040SwitchProhibited;
}

OAL_STATIC OAL_INLINE void mac_mib_set_TxAggregateActived(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11TxAggregateActived = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_TxAggregateActived(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11TxAggregateActived;
}

OAL_STATIC OAL_INLINE void mac_mib_set_CfgAmpduTxAtive(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11CfgAmpduTxAtive = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_CfgAmpduTxAtive(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11CfgAmpduTxAtive;
}

OAL_STATIC OAL_INLINE void mac_mib_set_RxBASessionNumber(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11RxBASessionNumber = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_RxBASessionNumber(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11RxBASessionNumber;
}

OAL_STATIC OAL_INLINE void mac_mib_incr_RxBASessionNumber(mac_vap_stru *mac_vap)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11RxBASessionNumber++;
}

OAL_STATIC OAL_INLINE void mac_mib_decr_RxBASessionNumber(mac_vap_stru *mac_vap)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11RxBASessionNumber--;
}

OAL_STATIC OAL_INLINE void mac_mib_set_TxBASessionNumber(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11TxBASessionNumber = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_TxBASessionNumber(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11TxBASessionNumber;
}

OAL_STATIC OAL_INLINE void mac_mib_incr_TxBASessionNumber(mac_vap_stru *mac_vap)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11TxBASessionNumber++;
}

OAL_STATIC OAL_INLINE void mac_mib_decr_TxBASessionNumber(mac_vap_stru *mac_vap)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11TxBASessionNumber--;
}

OAL_STATIC OAL_INLINE void mac_mib_set_VAPClassifyTidNo(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11VAPClassifyTidNo = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_VAPClassifyTidNo(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11VAPClassifyTidNo;
}

OAL_STATIC OAL_INLINE void mac_mib_set_AuthenticationMode(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AuthenticationMode = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_AuthenticationMode(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AuthenticationMode;
}

OAL_STATIC OAL_INLINE void mac_mib_set_AddBaMode(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AddBaMode = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_AddBaMode(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AddBaMode;
}

OAL_STATIC OAL_INLINE void mac_mib_set_80211iMode(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot1180211iMode = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_80211iMode(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot1180211iMode;
}

OAL_STATIC OAL_INLINE void mac_mib_set_TxTrafficClassifyFlag(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11TxTrafficClassifyFlag = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_TxTrafficClassifyFlag(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11TxTrafficClassifyFlag;
}

OAL_STATIC OAL_INLINE void mac_mib_set_StaAuthCount(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAuthCount = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_StaAuthCount(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAuthCount;
}

OAL_STATIC OAL_INLINE void mac_mib_set_StaAssocCount(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAssocCount = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_StaAssocCount(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAssocCount;
}

OAL_STATIC OAL_INLINE void mac_mib_incr_StaAuthCount(mac_vap_stru *mac_vap)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAuthCount++;
}

OAL_STATIC OAL_INLINE void mac_mib_incr_StaAssocCount(mac_vap_stru *mac_vap)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAssocCount++;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11RMBeaconTableMeasurementActivated(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMBeaconTableMeasurementActivated = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11RMBeaconTableMeasurementActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMBeaconTableMeasurementActivated;
}

OAL_STATIC OAL_INLINE void mac_mib_set_2040BSSCoexistenceManagementSupport(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot112040BSSCoexistenceManagementSupport = val;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_dot11dtimperiod(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11DTIMPeriod;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11VapMaxBandWidth(mac_vap_stru *mac_vap,
    wlan_bw_cap_enum_uint8 bandwidth)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11VapMaxBandWidth = bandwidth;
}

OAL_STATIC OAL_INLINE wlan_bw_cap_enum_uint8 mac_mib_get_dot11VapMaxBandWidth(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11VapMaxBandWidth;
}

OAL_STATIC OAL_INLINE void mac_mib_set_BeaconPeriod(mac_vap_stru *mac_vap, uint32_t value)
{
    if (value != 0) {
        mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11BeaconPeriod = value;
    }
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_BeaconPeriod(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11BeaconPeriod;
}

OAL_STATIC OAL_INLINE void mac_mib_set_DesiredBSSType(mac_vap_stru *mac_vap, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11DesiredBSSType = value;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_DesiredBSSType(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11DesiredBSSType;
}

OAL_STATIC OAL_INLINE uint8_t *mac_mib_get_DesiredSSID(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11DesiredSSID;
}

OAL_STATIC OAL_INLINE void mac_mib_set_AuthenticationResponseTimeOut(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11AuthenticationResponseTimeOut = value;
}
OAL_STATIC OAL_INLINE uint32_t mac_mib_get_AuthenticationResponseTimeOut(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11AuthenticationResponseTimeOut;
}

OAL_STATIC OAL_INLINE uint8_t *mac_mib_get_p2p0_dot11StationID(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID;
}

#if (defined(_PRE_WLAN_FEATURE_FTM)) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_MgmtOptionBSSTransitionActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11MgmtOptionBSSTransitionActivated;
}

OAL_STATIC OAL_INLINE void mac_mib_set_MgmtOptionBSSTransitionActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11MgmtOptionBSSTransitionActivated = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_MgmtOptionBSSTransitionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11MgmtOptionBSSTransitionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_MgmtOptionBSSTransitionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11MgmtOptionBSSTransitionImplemented = val;
}
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_WirelessManagementImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11WirelessManagementImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_WirelessManagementImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11WirelessManagementImplemented = val;
}
#endif  // _PRE_WLAN_FEATURE_11V_ENABLE

#if (defined(_PRE_WLAN_FEATURE_FTM)) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
OAL_STATIC OAL_INLINE void mac_mib_set_FtmInitiatorModeActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11FtmInitiatorModeActivated = val;
}
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_FtmInitiatorModeActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11FtmInitiatorModeActivated;
}
OAL_STATIC OAL_INLINE void mac_mib_set_FtmResponderModeActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11FtmResponderModeActivated = val;
}
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_FtmResponderModeActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11FtmResponderModeActivated;
}
OAL_STATIC OAL_INLINE void mac_mib_set_FtmRangeReportActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11RMFtmRangeReportActivated = val;
}
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_FtmRangeReportActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11RMFtmRangeReportActivated;
}
#endif

OAL_STATIC OAL_INLINE void mac_mib_init_2040(mac_vap_stru *mac_vap)
{
    mac_mib_set_FortyMHzIntolerant(mac_vap, OAL_FALSE);
    mac_mib_set_SpectrumManagementImplemented(mac_vap, OAL_TRUE);
    mac_mib_set_2040BSSCoexistenceManagementSupport(mac_vap, OAL_TRUE);
}

OAL_STATIC OAL_INLINE void mac_mib_init_obss_scan(mac_vap_stru *mac_vap)
{
    mac_mib_set_OBSSScanPassiveDwell(mac_vap, 20); /* 20 OBSSScanPassiveDwel 值 */
    mac_mib_set_OBSSScanActiveDwell(mac_vap, 10); /* 10 OBSSScanActiveDwell 值 */
    mac_mib_set_BSSWidthTriggerScanInterval(mac_vap, 300); /* 300 BSSWidthTriggerScanInterval 值 */
    mac_mib_set_OBSSScanPassiveTotalPerChannel(mac_vap, 200); /* 200 OBSSScanPassiveTotalPerChannel 值 */
    mac_mib_set_OBSSScanActiveTotalPerChannel(mac_vap, 20); /* 20 OBSSScanActiveTotalPerChannel 值 */
    mac_mib_set_BSSWidthChannelTransitionDelayFactor(mac_vap, 5); /* 5 BSSWidthChannelTransitionDelayFactor 值 */
    mac_mib_set_OBSSScanActivityThreshold(mac_vap, 25); /* 25 OBSSScanActivityThreshold 值 */
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11dtimperiod(mac_vap_stru *mac_vap, uint32_t value)
{
    if (value != 0) {
        mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11DTIMPeriod = value;
    }
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_powermanagementmode(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11PowerManagementMode;
}

OAL_STATIC OAL_INLINE void mac_mib_set_powermanagementmode(mac_vap_stru *mac_vap, uint8_t value)
{
    if (value != 0) {
        mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11PowerManagementMode = value;
    }
}

#if defined _PRE_WLAN_FEATURE_OPMODE_NOTIFY || (_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION)
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_OperatingModeNotificationImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11OperatingModeNotificationImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_OperatingModeNotificationImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11OperatingModeNotificationImplemented = val;
}
#endif

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_LsigTxopFullProtectionActivated(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11LSIGTXOPFullProtectionActivated);
}

OAL_STATIC OAL_INLINE void mac_mib_set_LsigTxopFullProtectionActivated(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 en_lsig_txop_full_protection_activated)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11LSIGTXOPFullProtectionActivated =
        en_lsig_txop_full_protection_activated;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_NonGFEntitiesPresent(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11NonGFEntitiesPresent);
}

OAL_STATIC OAL_INLINE void mac_mib_set_NonGFEntitiesPresent(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 en_non_gf_entities_present)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11NonGFEntitiesPresent = en_non_gf_entities_present;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_RifsMode(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11RIFSMode);
}

OAL_STATIC OAL_INLINE void mac_mib_set_RifsMode(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 en_rifs_mode)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11RIFSMode = en_rifs_mode;
}

OAL_STATIC OAL_INLINE wlan_mib_ht_protection_enum mac_mib_get_HtProtection(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11HTProtection);
}

OAL_STATIC OAL_INLINE void mac_mib_set_DualCTSProtection(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11DualCTSProtection = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_DualCTSProtection(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11DualCTSProtection);
}

OAL_STATIC OAL_INLINE void mac_mib_set_PCOActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11PCOActivated = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_PCOActivated(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11PCOActivated);
}

OAL_STATIC OAL_INLINE void mac_mib_set_HtProtection(mac_vap_stru *mac_vap,
    wlan_mib_ht_protection_enum en_ht_protection)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11HTProtection = en_ht_protection;
}

OAL_STATIC OAL_INLINE void mac_mib_set_SpectrumManagementRequired(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementRequired = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11SpectrumManagementRequired(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementRequired;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ShortGIOptionInFortyImplemented(mac_vap_stru *mac_vap)
{
    return (mac_vap->st_channel.en_band == WLAN_BAND_2G) ?
        mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GShortGIOptionInFortyImplemented :
        mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GShortGIOptionInFortyImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ShortGIOptionInFortyImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    if (mac_vap->st_channel.en_band == WLAN_BAND_2G) {
        mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GShortGIOptionInFortyImplemented = val;
    } else {
        mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GShortGIOptionInFortyImplemented = val;
    }
}

OAL_STATIC OAL_INLINE void mac_mib_set_2GShortGIOptionInFortyImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GShortGIOptionInFortyImplemented = val;
}

OAL_STATIC OAL_INLINE void mac_mib_set_5GShortGIOptionInFortyImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GShortGIOptionInFortyImplemented = val;
}

OAL_STATIC OAL_INLINE void mac_mib_set_FragmentationThreshold(mac_vap_stru *mac_vap, uint32_t frag_threshold)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11FragmentationThreshold = frag_threshold;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_FragmentationThreshold(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11FragmentationThreshold;
}

OAL_STATIC OAL_INLINE void mac_mib_set_RTSThreshold(mac_vap_stru *mac_vap, uint32_t rts_threshold)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11RTSThreshold = rts_threshold;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_RTSThreshold(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11RTSThreshold;
}

OAL_STATIC OAL_INLINE void mac_mib_set_AntennaSelectionOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11AntennaSelectionOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_AntennaSelectionOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11AntennaSelectionOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_TransmitExplicitCSIFeedbackASOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitExplicitCSIFeedbackASOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_TransmitExplicitCSIFeedbackASOptionImplemented(mac_vap_stru *vap)
{
    return vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitExplicitCSIFeedbackASOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_TransmitIndicesFeedbackASOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitIndicesFeedbackASOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_TransmitIndicesFeedbackASOptionImplemented(mac_vap_stru *vap)
{
    return vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitIndicesFeedbackASOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ExplicitCSIFeedbackASOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11ExplicitCSIFeedbackASOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ExplicitCSIFeedbackASOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11ExplicitCSIFeedbackASOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ReceiveAntennaSelectionOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11ReceiveAntennaSelectionOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ReceiveAntennaSelectionOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11ReceiveAntennaSelectionOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_TransmitSoundingPPDUOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitSoundingPPDUOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_TransmitSoundingPPDUOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitSoundingPPDUOptionImplemented;
}

OAL_STATIC OAL_INLINE wlan_11b_mib_preamble_enum_uint8 mac_mib_get_ShortPreambleOptionImplemented(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11ShortPreambleOptionImplemented);
}

OAL_STATIC OAL_INLINE void mac_mib_set_ShortPreambleOptionImplemented(
    mac_vap_stru *mac_vap, wlan_11b_mib_preamble_enum_uint8 en_preamble)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11ShortPreambleOptionImplemented = en_preamble;
}

OAL_STATIC OAL_INLINE void mac_mib_set_PBCCOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11PBCCOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_PBCCOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11PBCCOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ChannelAgilityPresent(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11ChannelAgilityPresent = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ChannelAgilityPresent(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11ChannelAgilityPresent;
}

OAL_STATIC OAL_INLINE void mac_mib_set_DSSSOFDMOptionActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11DSSSOFDMOptionActivated = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_DSSSOFDMOptionActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11DSSSOFDMOptionActivated;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ShortSlotTimeOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11ShortSlotTimeOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ShortSlotTimeOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11ShortSlotTimeOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ShortSlotTimeOptionActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11ShortSlotTimeOptionActivated = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ShortSlotTimeOptionActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11ShortSlotTimeOptionActivated;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ReceiveStaggerSoundingOptionImplemented(
    mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ReceiveStaggerSoundingOptionImplemented =
        val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ReceiveStaggerSoundingOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.\
        st_wlan_mib_txbf_config.en_dot11ReceiveStaggerSoundingOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_TransmitStaggerSoundingOptionImplemented(
    mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11TransmitStaggerSoundingOptionImplemented =
        val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_TransmitStaggerSoundingOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        en_dot11TransmitStaggerSoundingOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ReceiveNDPOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ReceiveNDPOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ReceiveNDPOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ReceiveNDPOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_TransmitNDPOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11TransmitNDPOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_TransmitNDPOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11TransmitNDPOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ImplicitTransmitBeamformingOptionImplemented(
    mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ImplicitTransmitBeamformingOptionImplemented =
        val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ImplicitTransmitBeamformingOptionImplemented(
    mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        en_dot11ImplicitTransmitBeamformingOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_CalibrationOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11CalibrationOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_CalibrationOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11CalibrationOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ExplicitCSITransmitBeamformingOptionImplemented(
    mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        en_dot11ExplicitCSITransmitBeamformingOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ExplicitCSITransmitBeamformingOptionImplemented(
    mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        en_dot11ExplicitCSITransmitBeamformingOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ExplicitNonCompressedBeamformingMatrixOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        en_dot11ExplicitNonCompressedBeamformingMatrixOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ExplicitNonCompressedBeamformingMatrixOptionImplemented(
    mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        en_dot11ExplicitNonCompressedBeamformingMatrixOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ExplicitTransmitBeamformingCSIFeedbackOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11ExplicitTransmitBeamformingCSIFeedbackOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ExplicitTransmitBeamformingCSIFeedbackOptionImplemented(
    mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11ExplicitTransmitBeamformingCSIFeedbackOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ExplicitNonCompressedBeamformingFeedbackOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11ExplicitNonCompressedBeamformingFeedbackOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ExplicitNonCompressedBeamformingFeedbackOptionImplemented(
    mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11ExplicitNonCompressedBeamformingFeedbackOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11ExplicitCompressedBeamformingFeedbackOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_ExplicitCompressedBeamformingFeedbackOptionImplemented(
    mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11ExplicitCompressedBeamformingFeedbackOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_NumberBeamFormingCSISupportAntenna(
    mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11NumberBeamFormingCSISupportAntenna = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_NumberBeamFormingCSISupportAntenna(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11NumberBeamFormingCSISupportAntenna;
}

OAL_STATIC OAL_INLINE void mac_mib_set_NumberNonCompressedBeamformingMatrixSupportAntenna(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11NumberNonCompressedBeamformingMatrixSupportAntenna = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_NumberNonCompressedBeamformingMatrixSupportAntenna(
    mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11NumberNonCompressedBeamformingMatrixSupportAntenna;
}

OAL_STATIC OAL_INLINE void mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(
    mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11NumberCompressedBeamformingMatrixSupportAntenna = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_NumberCompressedBeamformingMatrixSupportAntenna(
    mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11NumberCompressedBeamformingMatrixSupportAntenna;
}

OAL_STATIC OAL_INLINE void mac_mib_set_VHTSUBeamformeeOptionImplemented(mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformeeOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_VHTSUBeamformeeOptionImplemented(mac_vap_stru *vap)
{
    return vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformeeOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_VHTSUBeamformerOptionImplemented(mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformerOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_VHTSUBeamformerOptionImplemented(mac_vap_stru *vap)
{
    return vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformerOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_VHTMUBeamformeeOptionImplemented(mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTMUBeamformeeOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_VHTMUBeamformeeOptionImplemented(mac_vap_stru *vap)
{
    return vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTMUBeamformeeOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_VHTMUBeamformerOptionImplemented(mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTMUBeamformerOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_VHTMUBeamformerOptionImplemented(
    mac_vap_stru *vap)
{
    return vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.\
        en_dot11VHTMUBeamformerOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_VHTNumberSoundingDimensions(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.uc_dot11VHTNumberSoundingDimensions = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_VHTNumberSoundingDimensions(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.uc_dot11VHTNumberSoundingDimensions;
}

OAL_STATIC OAL_INLINE void mac_mib_set_VHTBeamformeeNTxSupport(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.uc_dot11VHTBeamformeeNTxSupport = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_VHTBeamformeeNTxSupport(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.uc_dot11VHTBeamformeeNTxSupport;
}

OAL_STATIC OAL_INLINE void mac_mib_set_VHTChannelWidthOptionImplemented(
    mac_vap_stru *mac_vap, wlan_mib_vht_supp_width_enum val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTChannelWidthOptionImplemented = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_VHTChannelWidthOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTChannelWidthOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_VHTShortGIOptionIn80Implemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn80Implemented = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_VHTShortGIOptionIn80Implemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn80Implemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_VHTShortGIOptionIn160and80p80Implemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn160and80p80Implemented = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_VHTShortGIOptionIn160and80p80Implemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn160and80p80Implemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_VHTLDPCCodingOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTLDPCCodingOptionImplemented = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_VHTLDPCCodingOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTLDPCCodingOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_VHTTxSTBCOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTTxSTBCOptionImplemented = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_VHTTxSTBCOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTTxSTBCOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_VHTRxSTBCOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTRxSTBCOptionImplemented = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_VHTRxSTBCOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTRxSTBCOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_HTGreenfieldOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11HTGreenfieldOptionImplemented = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_HTGreenfieldOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11HTGreenfieldOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_ShortGIOptionInTwentyImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11ShortGIOptionInTwentyImplemented = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_ShortGIOptionInTwentyImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11ShortGIOptionInTwentyImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_LDPCCodingOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11LDPCCodingOptionImplemented = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_LDPCCodingOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11LDPCCodingOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_LDPCCodingOptionActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11LDPCCodingOptionActivated = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_LDPCCodingOptionActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11LDPCCodingOptionActivated;
}

OAL_STATIC OAL_INLINE void mac_mib_set_TxSTBCOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxSTBCOptionImplemented = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_TxSTBCOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxSTBCOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_TxSTBCOptionActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxSTBCOptionActivated = val;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_TxSTBCOptionActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxSTBCOptionActivated;
}

OAL_STATIC OAL_INLINE void mac_mib_set_RxSTBCOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11RxSTBCOptionImplemented = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_RxSTBCOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11RxSTBCOptionImplemented;
}

OAL_STATIC OAL_INLINE void mac_mib_set_HighestSupportedDataRate(mac_vap_stru *mac_vap, uint32_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.dot11HighestSupportedDataRate = val;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_HighestSupportedDataRate(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.dot11HighestSupportedDataRate;
}

OAL_STATIC OAL_INLINE void mac_mib_set_TxMCSSetDefined(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxMCSSetDefined = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_TxMCSSetDefined(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxMCSSetDefined;
}

OAL_STATIC OAL_INLINE void mac_mib_set_TxRxMCSSetNotEqual(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxRxMCSSetNotEqual = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_TxRxMCSSetNotEqual(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxRxMCSSetNotEqual;
}

OAL_STATIC OAL_INLINE void mac_mib_set_TxMaximumNumberSpatialStreamsSupported(mac_vap_stru *mac_vap, uint32_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.dot11TxMaximumNumberSpatialStreamsSupported = val;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_TxMaximumNumberSpatialStreamsSupported(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.dot11TxMaximumNumberSpatialStreamsSupported;
}

OAL_STATIC OAL_INLINE void mac_mib_set_TxUnequalModulationSupported(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxUnequalModulationSupported = val;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_TxUnequalModulationSupported(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxUnequalModulationSupported;
}

OAL_STATIC OAL_INLINE void mac_mib_set_SupportedMCSTxValue(mac_vap_stru *mac_vap, uint8_t idx, uint8_t value)
{
    mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcstx.auc_dot11SupportedMCSTxValue[idx] = value;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_SupportedMCSTxValue(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcstx.auc_dot11SupportedMCSTxValue[idx];
}

OAL_STATIC OAL_INLINE uint8_t *mac_mib_get_SupportedMCSTx(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcstx.auc_dot11SupportedMCSTxValue;
}

OAL_STATIC OAL_INLINE void mac_mib_set_SupportedMCSRxValue(mac_vap_stru *mac_vap, uint8_t idx, uint8_t value)
{
    mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcsrx.auc_dot11SupportedMCSRxValue[idx] = value;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_SupportedMCSRxValue(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcsrx.auc_dot11SupportedMCSRxValue[idx];
}

OAL_STATIC OAL_INLINE uint8_t *mac_mib_get_SupportedMCSRx(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcsrx.auc_dot11SupportedMCSRxValue;
}

OAL_STATIC OAL_INLINE void mac_mib_set_EDCATableIndex(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableIndex = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_EDCATableIndex(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableIndex;
}

OAL_STATIC OAL_INLINE void mac_mib_set_EDCATableCWmin(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableCWmin = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_EDCATableCWmin(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableCWmin;
}

OAL_STATIC OAL_INLINE void mac_mib_set_EDCATableCWmax(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableCWmax = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_EDCATableCWmax(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableCWmax;
}

OAL_STATIC OAL_INLINE void mac_mib_set_EDCATableAIFSN(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableAIFSN = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_EDCATableAIFSN(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableAIFSN;
}

OAL_STATIC OAL_INLINE void mac_mib_set_EDCATableTXOPLimit(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableTXOPLimit = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_EDCATableTXOPLimit(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableTXOPLimit;
}

OAL_STATIC OAL_INLINE void mac_mib_set_EDCATableMandatory(mac_vap_stru *mac_vap, uint8_t idx, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].en_dot11EDCATableMandatory = value;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_EDCATableMandatory(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].en_dot11EDCATableMandatory;
}

OAL_STATIC OAL_INLINE void mac_mib_set_QAPEDCATableIndex(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableIndex = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_QAPEDCATableIndex(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableIndex;
}

OAL_STATIC OAL_INLINE void mac_mib_set_QAPEDCATableCWmin(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableCWmin = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_QAPEDCATableCWmin(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableCWmin;
}

OAL_STATIC OAL_INLINE void mac_mib_set_QAPEDCATableCWmax(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableCWmax = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_QAPEDCATableCWmax(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableCWmax;
}

OAL_STATIC OAL_INLINE void mac_mib_set_QAPEDCATableAIFSN(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableAIFSN = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_QAPEDCATableAIFSN(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableAIFSN;
}

OAL_STATIC OAL_INLINE void mac_mib_set_QAPEDCATableTXOPLimit(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableTXOPLimit = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_QAPEDCATableTXOPLimit(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableTXOPLimit;
}

OAL_STATIC OAL_INLINE void mac_mib_set_QAPEDCATableMandatory(mac_vap_stru *mac_vap, uint8_t idx, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].en_dot11QAPEDCATableMandatory = value;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_QAPEDCATableMandatory(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].en_dot11QAPEDCATableMandatory;
}

OAL_STATIC OAL_INLINE void mac_mib_set_QAPEDCATableMSDULifetime(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableMSDULifetime = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_QAPEDCATableMSDULifetime(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableMSDULifetime;
}

#ifdef _PRE_WLAN_FEATURE_11AX
OAL_STATIC OAL_INLINE void mac_mib_set_QAPMUEDCATableIndex(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableIndex = value;
}

OAL_STATIC OAL_INLINE void mac_mib_set_QAPMUEDCATableCWmin(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableCWmin = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_QAPMUEDCATableCWmin(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableCWmin;
}

OAL_STATIC OAL_INLINE void mac_mib_set_QAPMUEDCATableCWmax(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableCWmax = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_QAPMUEDCATableCWmax(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableCWmax;
}

OAL_STATIC OAL_INLINE void mac_mib_set_QAPMUEDCATableAIFSN(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableAIFSN = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_QAPMUEDCATableAIFSN(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableAIFSN;
}

OAL_STATIC OAL_INLINE void mac_mib_set_QAPMUEDCATableTXOPLimit(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableTXOPLimit = value;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_QAPMUEDCATableTXOPLimit(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableTXOPLimit;
}

OAL_STATIC OAL_INLINE void mac_mib_set_QAPMUEDCATableMandatory(mac_vap_stru *mac_vap, uint8_t idx, uint8_t value)
{
    mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].en_dot11QAPEDCATableMandatory = value;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_QAPMUEDCATableMandatory(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].en_dot11QAPEDCATableMandatory;
}

OAL_STATIC OAL_INLINE void mac_mib_set_PPEThresholdsRequired(mac_vap_stru *mac_vap, oal_bool_enum_uint8 value)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11PPEThresholdsRequired = value;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_PPEThresholdsRequired(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11PPEThresholdsRequired;
}
#endif // _PRE_WLAN_FEATURE_11AX

#endif

