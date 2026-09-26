
#include "hmac_mgmt_classifier.h"
#include "hmac_rx_data.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_mgmt_sta_up.h"
#include "hmac_mgmt_sta.h"
#include "hmac_mgmt_ap.h"
#include "hmac_tx_data.h"
#include "hmac_monitor.h"
#include "hmac_roam_main.h"

#include "hmac_dfx.h"

#ifdef _PRE_WLAN_FEATURE_FTM
#include "hmac_ftm.h"
#endif

#include "securec.h"
#ifndef WIN32
#include "oal_net.h"
#endif

#ifdef _PRE_WLAN_FEATURE_SNIFFER
#ifdef CONFIG_HW_SNIFFER
#include <hwnet/ipv4/sysctl_sniffer.h>
#endif
#ifdef _PRE_WINDOWS_SUPPORT
#include "wal_windows_ioctl.h"
#else
#include "wal_linux_ioctl.h"
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MGMT_CLASSIFIER_C
#define MAX_VHT_MCS_NSS 4

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

uint32_t hmac_mgmt_tx_action(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, mac_action_mgmt_args_stru *action_args)
{
    if (oal_any_null_ptr3(hmac_vap, hmac_user, action_args)) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_mgmt_tx_action::hmac_vap or hmac_user or action_args is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (action_args->uc_category != (uint8_t)MAC_ACTION_CATEGORY_BA) {
        oam_info_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                      "{hmac_mgmt_tx_action::invalid ba type[%d].}", action_args->uc_category);
        return OAL_FAIL;
    }
    switch (action_args->uc_action) {
        case MAC_BA_ACTION_ADDBA_REQ:
            oam_info_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                          "{hmac_mgmt_tx_action::MAC_BA_ACTION_ADDBA_REQ.}");
            hmac_mgmt_tx_addba_req(hmac_vap, hmac_user, action_args);
            break;
        case MAC_BA_ACTION_DELBA:
            oam_info_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                          "{hmac_mgmt_tx_action::MAC_BA_ACTION_DELBA.}");
            hmac_mgmt_tx_delba(hmac_vap, hmac_user, action_args);
            break;

        default:
            oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                             "{hmac_mgmt_tx_action::invalid ba type[%d].}", action_args->uc_action);
            return OAL_FAIL; /* 错误类型待修改 */
    }

    return OAL_SUCC;
}

uint32_t hmac_mgmt_rx_delba_event(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    frw_event_hdr_stru *event_hdr = NULL;
    dmac_ctx_action_event_stru *delba_event = NULL;
    uint8_t *da = NULL;             /* 保存用户目的地址的指针 */
    hmac_vap_stru *hmac_vap = NULL; /* vap指针 */
    hmac_user_stru *hmac_user = NULL;
    mac_action_mgmt_args_stru action_args;

    if (event_mem == NULL) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_rx_delba_event::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件头和事件结构体指针 */
    event = frw_get_event_stru(event_mem);
    event_hdr = &(event->st_event_hdr);
    delba_event = (dmac_ctx_action_event_stru *)(event->auc_event_data);

    /* 获取vap结构信息 */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(event_hdr->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(event_hdr->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_rx_delba_event::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取目的用户的MAC ADDR */
    da = delba_event->auc_mac_addr;

    /* 获取发送端的用户指针 */
    hmac_user = mac_vap_get_hmac_user_by_addr(&hmac_vap->st_vap_base_info, da);
    if (hmac_user == NULL) {
        oam_warning_log0(event_hdr->uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_rx_delba_event::mac_vap_find_user_by_macaddr failed.}");
        return OAL_FAIL;
    }
    action_args.uc_category = MAC_ACTION_CATEGORY_BA;
    action_args.uc_action = MAC_BA_ACTION_DELBA;
    action_args.arg1 = delba_event->uc_tidno;     /* 该数据帧对应的TID号 */
    action_args.arg2 = delba_event->uc_initiator; /* DELBA中，触发删除BA会话的发起端 */
    action_args.arg3 = delba_event->uc_status;    /* DELBA中代表删除reason */
    action_args.puc_arg5 = da;                       /* DELBA中代表目的地址 */
    if (hmac_mgmt_tx_action(hmac_vap, hmac_user, &action_args) != OAL_SUCC) {
        oam_warning_log0(event_hdr->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_rx_delba_event::hmac_mgmt_tx_action failed.}");
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_MONITOR
OAL_STATIC OAL_INLINE void hmac_fill_radiotap_set_bit(uint8_t *flags, uint8_t capable, uint8_t radiotap_flags)
{
    if (capable == 1) {
        *flags = *flags | radiotap_flags;
    }
}

OAL_STATIC void hmac_fill_radiotap_set_fields_wifi6_info(ieee80211_radiotap_stru *radiotap,
    hal_sniffer_rx_status_stru *rx_status, uint8_t bandwidth)
{
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_info[ARRAY_NUM_0] = 0;
    // fixed for GI known, pri/sec 80 known;
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_info[ARRAY_NUM_1] =
        RADIO_TAP_HE_DATA2_PRI_SEC_80MHZ_KNOWN | RADIO_TAP_HE_DATA2_GI_KNOWN;
    // MCS
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_info[ARRAY_NUM_2] |=
        (rx_status->un_nss_rate.st_rate.bit_rate_mcs << RADIO_TAP_HE_DATA3_MCS);
    // LDPC
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_info[ARRAY_NUM_2] |=
        (rx_status->bit_fec_coding << RADIO_TAP_HE_DATA3_LDPC);
    // STBC
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_info[ARRAY_NUM_2] |=
        (rx_status->un_nss_rate.st_rate.bit_STBC << RADIO_TAP_HE_DATA3_STBC);
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_info[ARRAY_NUM_3] = 0;

    // HE_Data 5
    // 0=20, 1=40, 2=80, 3=160/80+80, 4=26-tone RU, 5=52-tone RU
    // 6=106-tone RU, 7=242-tone RU, 8=484-tone RU, 9=996-tone RU, 10=2x996-tone RU
    if (bandwidth == WLAN_BAND_ASSEMBLE_20M) {
        radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_info[ARRAY_NUM_4] = 0;
    } else if (bandwidth == WLAN_BAND_ASSEMBLE_40M || bandwidth == WLAN_BAND_ASSEMBLE_40M_DUP) {
        radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_info[ARRAY_NUM_4] = 1;
    } else if (bandwidth == WLAN_BAND_ASSEMBLE_80M || bandwidth == WLAN_BAND_ASSEMBLE_80M_DUP) {
        radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_info[ARRAY_NUM_4] = WLAN_BAND_ASSEMBLE_80M;
    } else {
        radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_info[ARRAY_NUM_4] = WLAN_BAND_ASSEMBLE_160M;
    }

    // GI (0=0.8us, 1=1.6us, 2=3.2us, 3=reserved), what about 3.2us case?
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_info[ARRAY_NUM_4] |=
        (rx_status->bit_gi_type << NUM_4_BITS);
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_info[ARRAY_NUM_5] =
        (rx_status->un_nss_rate.st_rate.bit_nss_mode + 1);
    // set to all zero first
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_mu_info[ARRAY_NUM_0] = 0;
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_mu_info[ARRAY_NUM_1] = 0;
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_mu_ru_ch1[ARRAY_NUM_0] = 0;
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_mu_ru_ch1[ARRAY_NUM_1] = 0;
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_mu_ru_ch1[ARRAY_NUM_2] = 0;
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_mu_ru_ch1[ARRAY_NUM_3] = 0;
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_mu_ru_ch2[ARRAY_NUM_0] = 0;
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_mu_ru_ch2[ARRAY_NUM_1] = 0;
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_mu_ru_ch2[ARRAY_NUM_2] = 0;
    radiotap->st_radiotap_fields.extra_info.st_wifi6_info.he_mu_ru_ch2[ARRAY_NUM_3] = 0;

    return;
}

OAL_STATIC OAL_INLINE void hmac_fill_radiotap_add_cap(uint32_t *flags, uint8_t capable, uint32_t radiotap_flags)
{
    if (capable == 1) {
        *flags = *flags | (1 << radiotap_flags);
    }
    return;
}

#ifndef _PRE_LINUX_TEST
#define     IEEE80211_RADIOTAP_AMPDU_STATUS_INFO (21 - 1) //  0x00100000,
#define     IEEE80211_RADIOTAP_HE_INFO       (24 - 1)     // (0x00800000)
#define     IEEE80211_RADIOTAP_HE_MU_INFO    (25 - 1)     // (0x01000000)
#define     IEEE80211_RADIOTAP_LSIG          (28 - 1)     // (0x08000000)

OAL_STATIC OAL_INLINE void hmac_fill_radiotap_set_hdr_add_cap(ieee80211_radiotap_stru *radiotap,
    uint8_t is_HE)
{
    radiotap->st_radiotap_header.it_version = PKTHDR_RADIOTAP_VERSION;
    radiotap->st_radiotap_header.it_pad = 0;
    radiotap->st_radiotap_header.it_len = sizeof(ieee80211_radiotap_stru);
    radiotap->st_radiotap_header.it_present = 0;

    /* API for 32bit integer */
    hmac_fill_radiotap_add_cap(&radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_TSFT);
    hmac_fill_radiotap_add_cap(&radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_FLAGS);
    hmac_fill_radiotap_add_cap(&radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_RATE);
    hmac_fill_radiotap_add_cap(&radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_CHANNEL);
    hmac_fill_radiotap_add_cap(&radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_DBM_ANTSIGNAL);
    hmac_fill_radiotap_add_cap(&radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_DBM_ANTNOISE);
    if (is_HE) {
        hmac_fill_radiotap_add_cap(&radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_HE_INFO);
        hmac_fill_radiotap_add_cap(&radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_HE_MU_INFO);
    } else {
        hmac_fill_radiotap_add_cap(&radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_LOCK_QUALITY);
        hmac_fill_radiotap_add_cap(&radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_MCS);
        hmac_fill_radiotap_add_cap(&radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_VHT);
    }
    return;
}
#endif

OAL_STATIC OAL_INLINE void hmac_fill_radiotap_antenna_noise(int8_t *ssi_noise, int8_t ssi_signal,
    hal_sniffer_rx_statistic_stru *sniffer_rx_statistic)
{
    /* snr_ant是以0.5dB为单位，实际使用前需要先除以2 */
    *ssi_noise = ssi_signal - (sniffer_rx_statistic->c_snr_ant0 / 2) - (sniffer_rx_statistic->c_snr_ant1 / 2);
    if (((*ssi_noise) >= HMAC_PKT_CAP_NOISE_MAX) || ((*ssi_noise) < HMAC_PKT_CAP_NOISE_MIN)) {
        (*ssi_noise) = HMAC_PKT_CAP_NOISE_MIN;
    }

    return;
}
uint8_t hmac_sniffer_fill_radiotap_vht_bw(hal_statistic_stru *per_rate)
{
    uint8_t vht_bandwidth = 0;

    /* 填写对应的vht带宽信息 */
    if (per_rate->uc_bandwidth == WLAN_BAND_ASSEMBLE_20M) {
        vht_bandwidth = vht_bandwidth | IEEE80211_RADIOTAP_VHT_BW_20;
    } else if (per_rate->uc_bandwidth == WLAN_BAND_ASSEMBLE_40M ||
               per_rate->uc_bandwidth == WLAN_BAND_ASSEMBLE_40M_DUP) {
        vht_bandwidth = vht_bandwidth | IEEE80211_RADIOTAP_VHT_BW_40;
    } else if (per_rate->uc_bandwidth == WLAN_BAND_ASSEMBLE_80M ||
               per_rate->uc_bandwidth == WLAN_BAND_ASSEMBLE_80M_DUP) {
        vht_bandwidth = vht_bandwidth | IEEE80211_RADIOTAP_VHT_BW_80;
    } else {
        vht_bandwidth = vht_bandwidth | IEEE80211_RADIOTAP_VHT_BW_160;
    }

    return vht_bandwidth;
}

#ifndef _PRE_LINUX_TEST
static void hmac_sniffer_set_himit_flag(hal_sniffer_rx_status_stru *rx_status,
                                        mac_ieee80211_frame_stru *mac_head, uint8_t *flags)
{
    /* 设置 rx_status->bit_preabmle，1105没有，暂时设置为0 */
    /* 1103 rx himit flag = 1103 bit_preabmle */
    hmac_fill_radiotap_set_bit(flags, rx_status->bit_rx_himit_flag, (uint8_t)IEEE80211_RADIOTAP_F_SHORTPRE);
    hmac_fill_radiotap_set_bit(flags, mac_head->st_frame_control.bit_more_frag, (uint8_t)IEEE80211_RADIOTAP_F_FRAG);
    hmac_fill_radiotap_set_bit(flags, rx_status->bit_gi_type, (uint8_t)IEEE80211_RADIOTAP_F_SHORTGI);

    /* 填写fields字段中的其他成员ch_freq、ch_type、ssi_signal、ssi_noise、signal_quality */
    /* todo:增加一个对FCS校验错误帧个数的统计 */
    hmac_fill_radiotap_set_bit(flags, (rx_status->bit_dscr_status == HAL_RX_FCS_ERROR),
        IEEE80211_RADIOTAP_F_BADFCS);
}

static void hmac_sniffer_set_mcs_info(hal_statistic_stru *per_rate, uint8_t *mcs_info_known,
                                      uint8_t *mcs_info_flags, uint8_t *mcs_info_rate)
{
    *mcs_info_known = (uint8_t)IEEE80211_RADIOTAP_MCS_HAVE_BW | (uint8_t)IEEE80211_RADIOTAP_MCS_HAVE_MCS |
                      (uint8_t)IEEE80211_RADIOTAP_MCS_HAVE_GI | (uint8_t)IEEE80211_RADIOTAP_MCS_HAVE_FMT |
                      (uint8_t)IEEE80211_RADIOTAP_MCS_HAVE_FEC;
    /* 描述符里BW只有20, 20L和20U, 并没有40M的选项 */
    hmac_fill_radiotap_set_bit(mcs_info_flags, (uint8_t)(per_rate->uc_bandwidth != WLAN_BAND_ASSEMBLE_20M),
                               (1 << (uint8_t)IEEE80211_RADIOTAP_MCS_BW_40));
    hmac_fill_radiotap_set_bit(mcs_info_flags, per_rate->uc_short_gi, (uint8_t)IEEE80211_RADIOTAP_MCS_SGI);
    hmac_fill_radiotap_set_bit(mcs_info_flags, per_rate->bit_preamble, (uint8_t)IEEE80211_RADIOTAP_MCS_FMT_GF);
    hmac_fill_radiotap_set_bit(mcs_info_flags, per_rate->bit_channel_code,
                               (uint8_t)IEEE80211_RADIOTAP_MCS_FEC_LDPC);
    hmac_fill_radiotap_set_bit(mcs_info_flags, per_rate->bit_stbc, (uint8_t)IEEE80211_RADIOTAP_MCS_STBC_SHIFT);

    /* 填写对应的mcs速率信息 */
    *mcs_info_rate = per_rate->un_nss_rate.st_ht_rate.bit_ht_mcs;
#ifdef _PRE_WLAN_FEATURE_MONITOR_DEBUG
    oam_warning_log2(0, OAM_SF_11AX, "{hmac_sniffer_fill_radiotap::uc_mcs_info_flags[0x%x] uc_mcs_info_rate[%d]}",
                     mcs_info_flags, *mcs_info_rate);
#endif
}

static uint16_t hmac_sniffer_set_vht_info(hal_statistic_stru *per_rate, uint8_t *vht_flags,
                                          uint8_t *vht_bandwidth, uint8_t *vht_mcs_nss,
                                          uint8_t *vht_coding)
{
    uint32_t loop;
    uint16_t vht_known;
    vht_known = (uint16_t)IEEE80211_RADIOTAP_VHT_KNOWN_STBC | (uint16_t)IEEE80211_RADIOTAP_VHT_KNOWN_TXOP_PS_NA |
                (uint16_t)IEEE80211_RADIOTAP_VHT_KNOWN_GI | (uint16_t)IEEE80211_RADIOTAP_VHT_KNOWN_BEAMFORMED |
                (uint16_t)IEEE80211_RADIOTAP_VHT_KNOWN_BANDWIDTH |
                (uint16_t)IEEE80211_RADIOTAP_VHT_KNOWN_GROUP_ID |
                (uint16_t)IEEE80211_RADIOTAP_VHT_KNOWN_PARTIAL_AID;
    /* vht对应的flags信息, 包括STBC、Short GI等 */
    hmac_fill_radiotap_set_bit(vht_flags, (per_rate->bit_stbc != 0), IEEE80211_RADIOTAP_VHT_FLAG_STBC);
    hmac_fill_radiotap_set_bit(vht_flags, per_rate->uc_short_gi, IEEE80211_RADIOTAP_VHT_FLAG_SGI);
    *vht_bandwidth = hmac_sniffer_fill_radiotap_vht_bw(per_rate);

    /* 填写对应的vht速率信息、编码方式 */
    vht_mcs_nss[0] = (per_rate->un_nss_rate.rate_stru.bit_rate_mcs << NUM_4_BITS) +
                     (per_rate->un_nss_rate.rate_stru.bit_nss_mode + 1);
    for (loop = 1; (loop < MAX_VHT_MCS_NSS) &&
        (loop < (per_rate->un_nss_rate.rate_stru.bit_nss_mode + 1)); loop++) {
        vht_mcs_nss[loop] = vht_mcs_nss[0];
    }
    hmac_fill_radiotap_set_bit(vht_coding, per_rate->bit_channel_code,
                               (uint8_t)IEEE80211_RADIOTAP_CODING_LDPC_USER0);
#ifdef _PRE_WLAN_FEATURE_MONITOR_DEBUG
    oam_warning_log3(0, OAM_SF_11AX,
                     "{hmac_sniffer_fill_radiotap::uc_vht_flags[0x%x] uc_vht_bandwidth[%d], uc_vht_mcs_nss[%d]}",
                     *vht_flags, *vht_bandwidth, vht_mcs_nss[0]);
#endif
    return vht_known;
}

static void hmac_radiotap_fields_vht_mcs_nss_set(uint8_t *dest_vht_mcs_nss, uint8_t *src_vht_mcs_nss)
{
    dest_vht_mcs_nss[ARRAY_NUM_0] = src_vht_mcs_nss[ARRAY_NUM_0];
    dest_vht_mcs_nss[ARRAY_NUM_1] = src_vht_mcs_nss[ARRAY_NUM_1];
    dest_vht_mcs_nss[ARRAY_NUM_2] = src_vht_mcs_nss[ARRAY_NUM_2];
    dest_vht_mcs_nss[ARRAY_NUM_3] = src_vht_mcs_nss[ARRAY_NUM_3];
}
#endif

void hmac_sniffer_fill_radiotap(ieee80211_radiotap_stru *radiotap, mac_rx_ctl_stru *rx_ctrl,
                                hal_sniffer_rx_status_stru *rx_status,
                                hal_sniffer_rx_statistic_stru *sniffer_rx_statistic,
                                uint8_t *mac_hdr, uint32_t *rate_kbps, hal_statistic_stru *per_rate)
{
#ifndef _PRE_LINUX_TEST
    uint8_t flags = 0;     /* Do not include FCS at the end, HMAC_IEEE80211_RADIOTAP_F_FCS */
    uint8_t data_rate; /* data rate信息, 11ag和11b协议时该字段有效 */
    uint16_t ch_freq;
    uint16_t ch_type;
    int8_t ssi_signal;
    int8_t ssi_noise;
    int16_t signal_quality;
    uint8_t mcs_info_known = 0; /* mcs信息, 11n协议时该字段有效 */
    uint8_t mcs_info_flags = 0;
    uint8_t mcs_info_rate = 0;
    uint16_t vht_known = 0; /* vht信息, 11ac协议时该字段有效 */
    uint8_t vht_flags = 0;
    uint8_t vht_bandwidth = 0;
    uint8_t vht_mcs_nss[MAX_VHT_MCS_NSS] = { 0 };
    uint8_t vht_coding = 0;
    uint8_t vht_group_id = 0;
    uint16_t vht_partial_aid = 0;
    mac_ieee80211_frame_stru *mac_head = NULL;
    uint8_t frame_type;
    uint8_t  is_HE = 0;
    uint8_t protocol_mode = rx_status->un_nss_rate.st_rate.bit_protocol_mode;
    uint8_t bandwidth = rx_status->bit_freq_bandwidth_mode;

    memset_s(radiotap, sizeof(ieee80211_radiotap_stru), 0, sizeof(ieee80211_radiotap_stru));

    /* 填写fields字段里的flags成员 */
    mac_head = (mac_ieee80211_frame_stru *)mac_hdr;
    frame_type = mac_get_frame_type((uint8_t *)mac_hdr);

    hmac_sniffer_set_himit_flag(rx_status, mac_head, &flags);

    ssi_signal =
        (sniffer_rx_statistic->c_rssi_dbm != OAL_RSSI_INIT_VALUE) ? (sniffer_rx_statistic->c_rssi_dbm) : 0;

    /* snr_ant是以0.5dB为单位，实际使用前需要先除以2，且超出正常snr表示范围的用最小snr表示 */
    hmac_fill_radiotap_antenna_noise(&ssi_noise, ssi_signal, sniffer_rx_statistic);

    signal_quality = ssi_signal - HMAC_PKT_CAP_SIGNAL_OFFSET;

    /* 接收的信道需通过查找接收描述符里接收帧的信道获得 */
    if (rx_ctrl->uc_channel_number < 36) { /* 36信道 */
        ch_freq = WLAN_CHN_NUM2FREQ_STEP * rx_ctrl->uc_channel_number + WLAN_2G_CENTER_FREQ_BASE;
        ch_type = (uint16_t)IEEE80211_CHAN_2GHZ | (uint16_t)IEEE80211_CHAN_DYN;
    } else {
        ch_freq = WLAN_CHN_NUM2FREQ_STEP * rx_ctrl->uc_channel_number + WLAN_5G_CENTER_FREQ_BASE;
        ch_type = (uint16_t)IEEE80211_CHAN_5GHZ | (uint16_t)IEEE80211_CHAN_OFDM;
    }

#ifdef _PRE_WLAN_FEATURE_MONITOR_DEBUG
    oam_warning_log1(0, OAM_SF_11AX, "{hmac_sniffer_fill_radiotap::bit_protocol_mode[%d]}",
                     per_rate->un_nss_rate.st_ht_rate.bit_protocol_mode);
#endif
    /*
     * 填写fields字段中的速率信息, 根据radiotap的要求,
     * 11n时mcs_info有效、11ac时vht_info有效、11ag和11b时data_rate有效
     */
    if (protocol_mode == WLAN_HT_PHY_PROTOCOL_MODE) {
        hmac_sniffer_set_mcs_info(per_rate, &mcs_info_known, &mcs_info_flags, &mcs_info_rate);
    } else if (protocol_mode == WLAN_VHT_PHY_PROTOCOL_MODE) {
    vht_known = hmac_sniffer_set_vht_info(per_rate, &vht_flags, &vht_bandwidth, (uint8_t *)vht_mcs_nss, &vht_coding);
    } else if (protocol_mode >= WLAN_HE_SU_FORMAT_MODE && protocol_mode <= WLAN_HE_TRIG_FORMAT_MODE) {
        is_HE = 1;
        oam_warning_log3(0, OAM_SF_11AX,
            "{hmac_sniffer_fill_radiotap::bandwidth[%d], VHT_MCS[%d], bit_protocol_mode[%d]}",
            bandwidth, rx_status->un_nss_rate.st_rate.bit_rate_mcs, protocol_mode);
    }
    data_rate = (uint8_t)((*rate_kbps) / HMAC_PKT_CAP_RATE_UNIT);

#ifdef _PRE_WLAN_FEATURE_MONITOR_DEBUG
    oam_warning_log3(0, OAM_SF_11AX,
        "{hmac_sniffer_fill_radiotap::frame type:0x%x, data_rate[%d], bit_protocol_mode[%d]}", uc_frame_type, data_rate,
        rx_status->un_nss_rate.st_ht_rate.bit_protocol_mode);
#endif
    hmac_fill_radiotap_set_hdr_add_cap(radiotap, is_HE);

    radiotap->st_radiotap_fields.ull_timestamp = 0;
    radiotap->st_radiotap_fields.uc_flags = flags;
    radiotap->st_radiotap_fields.uc_data_rate = data_rate;
    radiotap->st_radiotap_fields.us_channel_freq = ch_freq;
    radiotap->st_radiotap_fields.us_channel_type = ch_type;
    radiotap->st_radiotap_fields.c_ssi_signal = ssi_signal;
    radiotap->st_radiotap_fields.c_ssi_noise = ssi_noise;
    radiotap->st_radiotap_fields.extra_info.st_legacy_wifi_info.s_signal_quality = signal_quality;

    if (is_HE) {
        hmac_fill_radiotap_set_fields_wifi6_info(radiotap, rx_status, bandwidth);
    } else {
        radiotap->st_radiotap_fields.extra_info.st_legacy_wifi_info.s_signal_quality = signal_quality;
        radiotap->st_radiotap_fields.extra_info.st_legacy_wifi_info.uc_mcs_info_known = mcs_info_known;
        radiotap->st_radiotap_fields.extra_info.st_legacy_wifi_info.uc_mcs_info_flags = mcs_info_flags;
        radiotap->st_radiotap_fields.extra_info.st_legacy_wifi_info.uc_mcs_info_rate = mcs_info_rate;
        radiotap->st_radiotap_fields.extra_info.st_legacy_wifi_info.us_vht_known = vht_known;
        radiotap->st_radiotap_fields.extra_info.st_legacy_wifi_info.uc_vht_flags = vht_flags;
        radiotap->st_radiotap_fields.extra_info.st_legacy_wifi_info.uc_vht_bandwidth = vht_bandwidth;
        hmac_radiotap_fields_vht_mcs_nss_set((uint8_t *)
                radiotap->st_radiotap_fields.extra_info.st_legacy_wifi_info.uc_vht_mcs_nss, (uint8_t *)vht_mcs_nss);
        radiotap->st_radiotap_fields.extra_info.st_legacy_wifi_info.uc_vht_coding = vht_coding;
        radiotap->st_radiotap_fields.extra_info.st_legacy_wifi_info.uc_vht_group_id = vht_group_id;
        radiotap->st_radiotap_fields.extra_info.st_legacy_wifi_info.us_vht_partial_aid = vht_partial_aid;
    }
#endif
}
#endif
#if defined(_PRE_WLAN_FEATURE_LOCATION) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
OAL_STATIC uint32_t hmac_monitor_rx_action(oal_netbuf_stru *netbuf,
    frw_event_hdr_stru *event_hdr)
{
    mac_rx_ctl_stru *rx_ctrl = NULL; /* 指向MPDU控制块信息的指针 */
    uint8_t *mac_hdr = NULL;
    uint8_t *data = NULL;
    hmac_vap_stru *hmac_vap = NULL;      /* vap指针 */

    rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_hdr = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(rx_ctrl);
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(event_hdr->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_warning_log0(event_hdr->uc_vap_id, OAM_SF_BA, "{hmac_monitor_rx_action::pst_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_get_frame_body_len(netbuf) < (MAC_ACTION_CATEGORY_AND_CODE_LEN + MAC_OUI_LEN)) {
        oam_warning_log1(0, OAM_SF_RX, "{monitor::frame len too short[%d].}", rx_ctrl->us_frame_len);
        return OAL_FAIL;
    }

    if (mac_get_frame_sub_type(mac_hdr) != WLAN_FC0_SUBTYPE_ACTION) {
        oam_warning_log0(event_hdr->uc_vap_id, OAM_SF_BA, "{hmac_monitor_rx_action::not action.}");
        return OAL_SUCC;
    }

    data = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(rx_ctrl) + rx_ctrl->uc_mac_header_len;
    if ((data[MAC_ACTION_OFFSET_CATEGORY] == MAC_ACTION_CATEGORY_PUBLIC) &&
        (data[MAC_ACTION_OFFSET_ACTION] == MAC_PUB_VENDOR_SPECIFIC)) {
        if (oal_memcmp(data + MAC_ACTION_CATEGORY_AND_CODE_LEN, g_auc_huawei_oui, MAC_OUI_LEN) == 0) {
            oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                             "{hmac_monitor_rx_action::hmac location get.}");
            hmac_huawei_action_process(hmac_vap, netbuf,
                                       data[MAC_ACTION_CATEGORY_AND_CODE_LEN + MAC_OUI_LEN]);
        }
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_MONITOR
OAL_STATIC uint32_t hmac_rx_process_mgmt_event_monitor_handle(hmac_device_stru *hmac_device,
    frw_event_hdr_stru *event_hdr, oal_netbuf_stru *netbuf)
{
    mac_rx_ctl_stru *rx_ctrl = NULL; /* 指向MPDU控制块信息的指针 */
    uint8_t *mac_hdr = NULL;

    rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);

    /* 获取该MPDU的控制信息 */
    mac_hdr = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(rx_ctrl);
    if (mac_get_frame_type(mac_hdr) == WLAN_FC0_TYPE_CTL) {
        hmac_device->control_frames_cnt++;
    } else if (mac_get_frame_type(mac_hdr) == WLAN_FC0_TYPE_MGT) {
        hmac_device->mgmt_frames_cnt++;

#if defined(_PRE_WLAN_FEATURE_LOCATION) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
        if (hmac_monitor_rx_action(netbuf, event_hdr) == OAL_ERR_CODE_PTR_NULL) {
            return OAL_ERR_CODE_PTR_NULL;
        }
#endif
    } else {
        hmac_device->others_frames_cnt++;
    }
    if (oal_value_eq_any2(hmac_device->pst_device_base_info->uc_monitor_ota_mode,
        WLAN_MONITOR_OTA_HOST_RPT, WLAN_MONITOR_OTA_ALL_RPT)) {
        hmac_monitor_report_frame_to_sdt(rx_ctrl, netbuf);
    }
    hmac_sniffer_save_data(hmac_device, netbuf, 1);
    oal_netbuf_free(netbuf); /* todo: 由使用者释放 */
    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
OAL_STATIC uint32_t hmac_rx_ftm_action(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    mac_rx_ctl_stru *rx_ctrl)
{
    uint8_t *data = NULL;

    data = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(rx_ctrl) + rx_ctrl->uc_mac_header_len;
    if ((data[MAC_ACTION_OFFSET_CATEGORY] == MAC_ACTION_CATEGORY_PUBLIC) &&
        (data[MAC_ACTION_OFFSET_ACTION] == MAC_PUB_FTM_REQ)) {
        hmac_ftm_responder_rx_req(hmac_vap, netbuf);
        return TRUE;
    }

    if ((data[MAC_ACTION_OFFSET_CATEGORY] == MAC_ACTION_CATEGORY_PUBLIC) &&
        (data[MAC_ACTION_OFFSET_ACTION] == MAC_PUB_FTM)) {
        hmac_ftm_initiator_rx_rsp(hmac_vap, netbuf);
        return TRUE;
    }

    return FALSE;
}

OAL_STATIC uint32_t hmac_rx_process_ftm_mgmt_event(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    frw_event_hdr_stru *event_hdr, mac_rx_ctl_stru *rx_ctrl)
{
    uint8_t *mac_hdr = NULL;

    if (!g_wlan_spec_cfg->feature_ftm_is_open) {
        return FALSE;
    }

    /* 获取该MPDU的控制信息 */
    mac_hdr = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(rx_ctrl);
    if (mac_get_frame_type(mac_hdr) == WLAN_FC0_TYPE_MGT) {
            if (mac_get_frame_sub_type(mac_hdr) == WLAN_FC0_SUBTYPE_ACTION) {
                return hmac_rx_ftm_action(hmac_vap, netbuf, rx_ctrl);
            }
    }
    return FALSE;
}
#endif
uint32_t hmac_rx_process_mgmt_event(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = frw_get_event_stru(event_mem);
    frw_event_hdr_stru *event_hdr = &(event->st_event_hdr);
    dmac_wlan_crx_event_stru *crx_event = (dmac_wlan_crx_event_stru *)(event->auc_event_data);
    oal_netbuf_stru *netbuf = crx_event->pst_netbuf; /* 用于保存管理帧指向的NETBUF */
    hmac_vap_stru *hmac_vap = NULL;      /* vap指针 */
    uint32_t ret;
    mac_rx_ctl_stru *rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf); /* 指向MPDU控制块信息的指针 */

#ifdef _PRE_WLAN_FEATURE_MONITOR
    hmac_device_stru *hmac_device = NULL;
#endif

#ifdef _PRE_WLAN_FEATURE_MONITOR
    hmac_device = hmac_res_get_mac_dev(event_hdr->uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log1(0, OAM_SF_SCAN, "{hmac_rx_process_mgmt_event::hmac_res_get_mac_dev fail.device_id = %u}",
                       event_hdr->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_device->en_monitor_mode == OAL_TRUE) {
        return hmac_rx_process_mgmt_event_monitor_handle(hmac_device, event_hdr, netbuf);
    }
#endif
    /* 获取vap结构信息 */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(event_hdr->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_warning_log0(event_hdr->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_rx_delba_event::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_FTM
    if (hmac_rx_process_ftm_mgmt_event(hmac_vap, netbuf, event_hdr, rx_ctrl) == TRUE) {
        oal_netbuf_free(netbuf);
        return OAL_SUCC;
    }
#endif

    /* NAN帧直接上报，否则调用状态机接口 */
    if (MAC_GET_RX_CB_NAN_FLAG(rx_ctrl)) {
        hmac_rx_mgmt_send_to_host(hmac_vap, netbuf);
    } else if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
#ifdef _PRE_WINDOWS_SUPPORT
        hmac_rx_mgmt_send_to_host(hmac_vap, netbuf);
#endif
        ret = hmac_fsm_call_func_ap(hmac_vap, HMAC_FSM_INPUT_RX_MGMT, crx_event);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_BA, "{hmac_rx_process_mgmt_event::hmac_fsm_call_func_ap fail.ret[%u]}", ret);
        }
    } else if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        ret = hmac_fsm_call_func_sta(hmac_vap, HMAC_FSM_INPUT_RX_MGMT, crx_event);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_BA, "{hmac_rx_process_mgmt_event::hmac_fsm_call_func_sta fail.ret[%u]}", ret);
        }
    }

    /* 管理帧统一释放接口 */
    oal_netbuf_free(netbuf);

    return OAL_SUCC;
}

uint32_t hmac_rx_process_wow_mgmt_event(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    frw_event_hdr_stru *event_hdr = NULL;
    dmac_wlan_crx_event_stru *crx_event = NULL;
    oal_netbuf_stru *netbuf = NULL; /* 用于保存管理帧指向的NETBUF */
    hmac_vap_stru *hmac_vap = NULL; /* vap指针 */
    uint32_t ret;
    mac_rx_ctl_stru *rx_ctrl = NULL;

    if (event_mem == NULL) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_rx_process_mgmt_event::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件头和事件结构体指针 */
    event = frw_get_event_stru(event_mem);
    event_hdr = &(event->st_event_hdr);
    crx_event = (dmac_wlan_crx_event_stru *)(event->auc_event_data);
    netbuf = crx_event->pst_netbuf;
    rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);

    /* 获取vap结构信息 */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(event_hdr->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_warning_log0(event_hdr->uc_vap_id, OAM_SF_BA, "{hmac_rx_process_wow_mgmt_event::hmac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 适配联调sdio驱动不做修改 */
    if (rx_ctrl->us_frame_len > rx_ctrl->uc_mac_header_len) {
        rx_ctrl->us_frame_len -= rx_ctrl->uc_mac_header_len;
    }

    /* 接收管理帧是状态机的一个输入，调用状态机接口 */
    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        ret = hmac_fsm_call_func_ap(hmac_vap, HMAC_FSM_INPUT_RX_MGMT, crx_event);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, 0, "{hmac_rx_process_wow_mgmt_event::fsm_call_func_ap fail.err[%u]}", ret);
        }
    } else if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        ret = hmac_fsm_call_func_sta(hmac_vap, HMAC_FSM_INPUT_RX_MGMT, crx_event);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, 0, "{hmac_rx_process_wow_mgmt_event::fsm_call_func_sta fail.err[%u]}", ret);
        }
    }

    /* 管理帧统一释放接口 */
    oal_netbuf_free(netbuf);

    return OAL_SUCC;
}


uint32_t hmac_mgmt_send_disasoc_deauth_event(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    frw_event_hdr_stru *event_hdr = NULL;
    dmac_diasoc_deauth_event *disasoc_deauth_event = NULL;
    uint8_t *da = NULL;      /* 保存用户目的地址的指针 */
    hmac_vap_stru *hmac_vap = NULL; /* vap指针 */
    hmac_user_stru *hmac_user = NULL;
    uint32_t rslt;
    uint16_t us_user_idx;
    uint8_t uc_event;
    mac_vap_stru *mac_vap = NULL;
    uint16_t err_code;
    mac_vap_stru *up_vap1 = NULL;
    mac_vap_stru *up_vap2 = NULL;
    mac_device_stru *mac_device = NULL;

    if (event_mem == NULL) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_mgmt_send_disasoc_deauth_event::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件头和事件结构体指针 */
    event = frw_get_event_stru(event_mem);
    event_hdr = &(event->st_event_hdr);
    disasoc_deauth_event = (dmac_diasoc_deauth_event *)(event->auc_event_data);

    /* 获取vap结构信息 */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(event_hdr->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(event_hdr->uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_mgmt_send_disasoc_deauth_event::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    mac_vap = &hmac_vap->st_vap_base_info;

    /* 获取目的用户的MAC ADDR */
    da = disasoc_deauth_event->auc_des_addr;
    uc_event = disasoc_deauth_event->uc_event;
    err_code = disasoc_deauth_event->uc_reason;

    /* 发送去认证, 未关联状态收到第三类帧 */
    if (uc_event == DMAC_WLAN_CRX_EVENT_SUB_TYPE_DEAUTH) {
        hmac_mgmt_send_deauth_frame(mac_vap, da, err_code, OAL_FALSE);  // 非PMF

        mac_device = mac_res_get_dev(mac_vap->uc_device_id);
        if (mac_device == NULL) {
            return OAL_SUCC;
        }
        /* 判断是异频DBAC模式时，无法判断是哪个信道收到的数据帧，两个信道都需要发去认证 */
        rslt = mac_device_find_2up_vap(mac_device, &up_vap1, &up_vap2);
        if (rslt != OAL_SUCC) {
            return OAL_SUCC;
        }

        if (up_vap1->st_channel.uc_chan_number == up_vap2->st_channel.uc_chan_number) {
            return OAL_SUCC;
        }

        /* 获取另一个VAP */
        if (mac_vap->uc_vap_id != up_vap1->uc_vap_id) {
            up_vap2 = up_vap1;
        }

        /* 另外一个VAP也发去认证帧。error code加上特殊标记，组去认证帧时要修改源地址 */
        hmac_mgmt_send_deauth_frame(up_vap2, da, err_code | MAC_SEND_TWO_DEAUTH_FLAG, OAL_FALSE);

        return OAL_SUCC;
    }

    /* 获取发送端的用户指针 */
    rslt = mac_vap_find_user_by_macaddr(mac_vap, da, &us_user_idx);
    if (rslt != OAL_SUCC) {
        oam_warning_log4(0, OAM_SF_RX,
            "{hmac_mgmt_send_disasoc_deauth_event::Hmac cannot find USER by addr[%02X:XX:XX:%02X:%02X:%02X], \
            just del DMAC user}", da[MAC_ADDR_0], da[MAC_ADDR_3], da[MAC_ADDR_4], da[MAC_ADDR_5]);
        /*
         * 找不到用户，说明用户已经删除，直接返回成功，
         * 不需要再抛事件到dmac删除用户(统一由hmac_user_del来管理删除用户)
         */
        return OAL_SUCC;
    }

    /* 获取到hmac user,使用protected标志 */
    hmac_user = mac_res_get_hmac_user(us_user_idx);

    hmac_mgmt_send_disassoc_frame(mac_vap, da, err_code, ((hmac_user == NULL) ?
                                  OAL_FALSE : hmac_user->st_user_base_info.st_cap_info.bit_pmf_active));

    if (hmac_user != NULL) {
        hmac_handle_disconnect_rsp(hmac_vap, hmac_user, err_code);
    }

    /* 删除用户 */
    hmac_user_del(mac_vap, hmac_user);

    return OAL_SUCC;
}

OAL_STATIC mac_reason_code_enum_uint16 hmac_disassoc_reason_exchange(
    dmac_disasoc_misc_reason_enum_uint16 en_driver_disasoc_reason)
{
    switch (en_driver_disasoc_reason) {
        case DMAC_DISASOC_MISC_LINKLOSS:
        case DMAC_DISASOC_MISC_KEEPALIVE:
        case DMAC_DISASOC_MISC_GET_CHANNEL_IDX_FAIL:
            return MAC_DEAUTH_LV_SS;

        case DMAC_DISASOC_MISC_CHANNEL_MISMATCH:
            return MAC_UNSPEC_REASON;
        default:
            break;
    }
    oam_warning_log1(0, OAM_SF_ASSOC, "{hmac_disassoc_reason_exchange::Unkown driver_disasoc_reason[%d].}",
                     en_driver_disasoc_reason);

    return MAC_UNSPEC_REASON;
}

static void hmac_proc_disasoc_inform(hmac_vap_stru *hmac_vap, mac_reason_code_enum_uint16 reason_code)
{
    hmac_disconnect disconnect;

    disconnect.reason_code = reason_code;
    disconnect.type = WLAN_FC0_SUBTYPE_DISASSOC;
    disconnect.is_roam = OAL_FALSE;
    hmac_sta_handle_disassoc_rsp(hmac_vap, &disconnect);
    return;
}


uint32_t hmac_proc_disasoc_misc_event(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = NULL;
    oal_bool_enum_uint8 en_is_protected;
    dmac_disasoc_misc_stru *pdmac_disasoc_misc_stru = NULL;
    mac_reason_code_enum_uint16 reason_code;

    if (oal_unlikely(event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_proc_disasoc_misc_event::event_mem is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    event = frw_get_event_stru(event_mem);
    pdmac_disasoc_misc_stru = (dmac_disasoc_misc_stru *)event->auc_event_data;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(event->st_event_hdr.uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_proc_disasoc_misc_event::hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                     "{hmac_proc_disasoc_misc_event::Device noticed to dissasoc user[%d] within reason[%d]!}",
                     pdmac_disasoc_misc_stru->us_user_idx,
                     pdmac_disasoc_misc_stru->en_disasoc_reason);

    hmac_chr_set_disasoc_reason(pdmac_disasoc_misc_stru->us_user_idx, pdmac_disasoc_misc_stru->en_disasoc_reason);

    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_user = mac_res_get_hmac_user(pdmac_disasoc_misc_stru->us_user_idx);
        if (hmac_user == NULL) {
            oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                             "{hmac_proc_disasoc_misc_event::hmac_user[%d] is null.}",
                             pdmac_disasoc_misc_stru->us_user_idx);
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_is_protected = hmac_user->st_user_base_info.st_cap_info.bit_pmf_active;
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
                             CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_SOFTAP_DISCONNECT,
                             pdmac_disasoc_misc_stru->en_disasoc_reason);
        /* 抛事件上报内核，已经去关联某个STA */
        hmac_handle_disconnect_rsp_ap(hmac_vap, hmac_user);

        reason_code = MAC_ASOC_NOT_AUTH;

        /* 发去关联帧 */
        hmac_mgmt_send_disassoc_frame(&hmac_vap->st_vap_base_info, hmac_user->st_user_base_info.auc_user_mac_addr,
                                      reason_code, en_is_protected);
        /* 删除用户 */
        hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);
    } else {
        /* 获用户 */
        hmac_user = mac_res_get_hmac_user(hmac_vap->st_vap_base_info.us_assoc_vap_id);
        if (hmac_user == NULL) {
            /* 和ap侧一样，上层已经删除了的话，属于正常 */
            oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                             "{hmac_proc_disasoc_misc_event::hmac_user[%d] is null.}",
                             hmac_vap->st_vap_base_info.us_assoc_vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_is_protected = hmac_user->st_user_base_info.st_cap_info.bit_pmf_active;

        /* 上报断链类型的转化 */
        reason_code = hmac_disassoc_reason_exchange(pdmac_disasoc_misc_stru->en_disasoc_reason);

        if (pdmac_disasoc_misc_stru->en_disasoc_reason != DMAC_DISASOC_MISC_CHANNEL_MISMATCH) {
            /* 发送去认证帧到AP */
            hmac_mgmt_send_disassoc_frame(&hmac_vap->st_vap_base_info, hmac_user->st_user_base_info.auc_user_mac_addr,
                                          reason_code, en_is_protected);
        }

        /* 删除对应用户 */
        hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);
        hmac_proc_disasoc_inform(hmac_vap, reason_code);
    }

    return OAL_SUCC;
}


uint32_t hmac_proc_roam_trigger_event(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    int8_t c_rssi;

    if (oal_unlikely(event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_proc_roam_trigger_event::event_mem is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    event = frw_get_event_stru(event_mem);
    c_rssi = *(int8_t *)event->auc_event_data;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(event->st_event_hdr.uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_proc_roam_trigger_event::hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_roam_kick_secondary_sta(hmac_vap, c_rssi);
    hmac_roam_trigger_handle(hmac_vap, c_rssi, OAL_TRUE);

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif