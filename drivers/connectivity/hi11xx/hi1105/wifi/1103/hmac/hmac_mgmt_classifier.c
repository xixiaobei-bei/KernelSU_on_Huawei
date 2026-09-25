

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 头文件包含 */
#include "hmac_rx_data.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_mgmt_classifier.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_mgmt_sta.h"
#include "hmac_mgmt_ap.h"

#ifdef _PRE_WLAN_CHIP_TEST
#include "dmac_test_main.h"
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
#include "hmac_roam_main.h"
#endif  // _PRE_WLAN_FEATURE_ROAM

#ifdef _PRE_WLAN_1103_CHR
#include "hmac_dfx.h"
#endif

#include "securec.h"
#include "securectype.h"
#ifndef WIN32
#include "oal_net.h"
#endif

#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include <hwnet/ipv4/sysctl_sniffer.h>
#include "wal_linux_ioctl.h"
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MGMT_CLASSIFIER_C

/* 2 全局变量定义 */
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) && !defined(_PRE_PC_LINT) && !defined(WIN32))
OAL_STATIC oal_uint8 g_ucLinklossLogSwitch = 0;
#endif

/* 3 函数实现 */
#ifdef _PRE_WLAN_FEATURE_TWT

oal_uint32 hmac_mgmt_tx_twt_action_etc(hmac_vap_stru *pst_hmac_vap,
                                       hmac_user_stru *pst_hmac_user,
                                       mac_twt_action_mgmt_args_stru *pst_twt_action_args)
{
    if (OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_hmac_user, pst_twt_action_args)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    switch (pst_twt_action_args->uc_category) {
        case MAC_ACTION_CATEGORY_S1G:
            switch (pst_twt_action_args->uc_action) {
                case MAC_S1G_ACTION_TWT_SETUP:
                    OAM_INFO_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_11AX,
                                  "{hmac_mgmt_tx_twt_action_etc::MAC_TWT_SETUP.}");
                    hmac_mgmt_tx_twt_setup_etc(pst_hmac_vap, pst_hmac_user, pst_twt_action_args);
                    break;

                case MAC_S1G_ACTION_TWT_TEARDOWN:
                    OAM_INFO_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_11AX,
                                  "{hmac_mgmt_tx_twt_action_etc::MAC_TWT_TEARDOWN.}");
                    hmac_mgmt_tx_twt_teardown_etc(pst_hmac_vap, pst_hmac_user, pst_twt_action_args);
                    break;

                default:
                    return OAL_FAIL; /* 错误类型待修改 */
            }
            break;

        default:
            OAM_INFO_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_11AX,
                          "{hmac_mgmt_tx_twt_action_etc::invalid action type[%d].}",
                          pst_twt_action_args->uc_category);
            break;
    }

    return OAL_SUCC;
}
#endif


oal_uint32 hmac_mgmt_tx_action_etc(hmac_vap_stru *pst_hmac_vap,
                                   hmac_user_stru *pst_hmac_user,
                                   mac_action_mgmt_args_stru *pst_action_args)
{
    if (OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_hmac_user, pst_action_args)) {
        OAM_ERROR_LOG3(0, OAM_SF_TX, "{hmac_mgmt_tx_action_etc::param null, %x %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_hmac_user, (uintptr_t)pst_action_args);
        return OAL_ERR_CODE_PTR_NULL;
    }

    switch (pst_action_args->uc_category) {
        case MAC_ACTION_CATEGORY_BA:

            switch (pst_action_args->uc_action) {
                case MAC_BA_ACTION_ADDBA_REQ:
                    OAM_INFO_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                                  "{hmac_mgmt_tx_action_etc::MAC_BA_ACTION_ADDBA_REQ.}");
                    hmac_mgmt_tx_addba_req_etc(pst_hmac_vap, pst_hmac_user, pst_action_args);
                    break;

                case MAC_BA_ACTION_DELBA:
                    OAM_INFO_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                                  "{hmac_mgmt_tx_action_etc::MAC_BA_ACTION_DELBA.}");
                    hmac_mgmt_tx_delba_etc(pst_hmac_vap, pst_hmac_user, pst_action_args);
                    break;

                default:
                    OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                                     "{hmac_mgmt_tx_action_etc::invalid ba type[%d].}",
                                     pst_action_args->uc_action);
                    return OAL_FAIL; /* 错误类型待修改 */
            }
            break;

        default:
            OAM_INFO_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                          "{hmac_mgmt_tx_action_etc::invalid ba type[%d].}",
                          pst_action_args->uc_category);
            break;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_mgmt_tx_priv_req_etc(hmac_vap_stru *pst_hmac_vap,
                                     hmac_user_stru *pst_hmac_user,
                                     mac_priv_req_args_stru *pst_priv_req)
{
    mac_priv_req_11n_enum_uint8 en_req_type;

    if (OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_hmac_user, pst_priv_req)) {
        OAM_ERROR_LOG3(0, OAM_SF_TX, "{hmac_mgmt_tx_priv_req_etc::param null, %x %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_hmac_user, (uintptr_t)pst_priv_req);
        return OAL_ERR_CODE_PTR_NULL;
    }

    en_req_type = pst_priv_req->uc_type;

    switch (en_req_type) {
        case MAC_A_MPDU_START:

            hmac_mgmt_tx_ampdu_start_etc(pst_hmac_vap, pst_hmac_user, pst_priv_req);
            break;

        case MAC_A_MPDU_END:
            hmac_mgmt_tx_ampdu_end_etc(pst_hmac_vap, pst_hmac_user, pst_priv_req);
            break;

        default:

            OAM_INFO_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                          "{hmac_mgmt_tx_priv_req_etc::invalid en_req_type[%d].}",
                          en_req_type);
            break;
    };

    return OAL_SUCC;
}


oal_uint32 hmac_mgmt_rx_delba_event_etc(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = OAL_PTR_NULL;
    frw_event_hdr_stru *pst_event_hdr = OAL_PTR_NULL;
    dmac_ctx_action_event_stru *pst_delba_event = OAL_PTR_NULL;
    oal_uint8 *puc_da = OAL_PTR_NULL;      /* 保存用户目的地址的指针 */
    hmac_vap_stru *pst_vap = OAL_PTR_NULL; /* vap指针 */
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    mac_action_mgmt_args_stru st_action_args;

    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_BA, "{hmac_mgmt_rx_delba_event_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件头和事件结构体指针 */
    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);
    pst_delba_event = (dmac_ctx_action_event_stru *)(pst_event->auc_event_data);

    /* 获取vap结构信息 */
    pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_UNLIKELY(pst_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_rx_delba_event_etc::pst_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取目的用户的MAC ADDR */
    puc_da = pst_delba_event->auc_mac_addr;

    /* 获取发送端的用户指针 */
    pst_hmac_user = mac_vap_get_hmac_user_by_addr_etc(&pst_vap->st_vap_base_info, puc_da);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_rx_delba_event_etc::mac_vap_find_user_by_macaddr_etc failed.}");
        return OAL_FAIL;
    }

    st_action_args.uc_category = MAC_ACTION_CATEGORY_BA;
    st_action_args.uc_action = MAC_BA_ACTION_DELBA;
    st_action_args.ul_arg1 = pst_delba_event->uc_tidno;     /* 该数据帧对应的TID号 */
    st_action_args.ul_arg2 = pst_delba_event->uc_initiator; /* DELBA中，触发删除BA会话的发起端 */
    st_action_args.ul_arg3 = pst_delba_event->uc_status;    /* DELBA中代表删除reason */
    st_action_args.puc_arg5 = puc_da;                       /* DELBA中代表目的地址 */

    hmac_mgmt_tx_action_etc(pst_vap, pst_hmac_user, &st_action_args);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_MONITOR

OAL_STATIC OAL_INLINE oal_uint32 hmac_fill_radiotap_set_bit(oal_uint8 *puc_flags,
                                                            oal_uint8 uc_capable,
                                                            oal_uint8 uc_ieee80211_radiotap_flags)
{
    if (uc_capable == 1) {
        *puc_flags = *puc_flags | uc_ieee80211_radiotap_flags;
    }

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE oal_uint32 hmac_fill_radiotap_add_cap(oal_uint32 *puc_flags,
                                                            oal_uint8 uc_capable,
                                                            oal_uint32 uc_ieee80211_radiotap_flags)
{
    if (uc_capable == 1) {
        *puc_flags = *puc_flags | (1 << uc_ieee80211_radiotap_flags);
    }

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE oal_uint32 hmac_fill_radiotap_antenna_noise(oal_int8 *pc_ssi_noise,
    oal_int8 c_ssi_signal, hal_sniffer_rx_statistic_stru *pst_sniffer_rx_statistic)
{
    *pc_ssi_noise = c_ssi_signal - (pst_sniffer_rx_statistic->c_snr_ant0 / 2) -
                    (pst_sniffer_rx_statistic->c_snr_ant1 / 2);

    if (((*pc_ssi_noise) >= HMAC_PKT_CAP_NOISE_MAX) || ((*pc_ssi_noise) < HMAC_PKT_CAP_NOISE_MIN)) {
        (*pc_ssi_noise) = HMAC_PKT_CAP_NOISE_MIN;
    }

    return OAL_SUCC;
}


oal_void hmac_sniffer_fill_radiotap(ieee80211_radiotap_stru *pst_radiotap,
                                    mac_rx_ctl_stru *pst_rx_ctrl,
                                    hal_sniffer_rx_status_stru *pst_rx_status,
                                    hal_sniffer_rx_statistic_stru *pst_sniffer_rx_statistic,
                                    oal_uint8 *puc_mac_hdr,
                                    oal_uint32 *pul_rate_kbps,
                                    hal_statistic_stru *pst_per_rate)
{
    oal_uint8 uc_flags = 0;     /* Do not include FCS at the end, HMAC_IEEE80211_RADIOTAP_F_FCS */
    oal_uint8 uc_data_rate = 0; /* data rate信息, 11ag和11b协议时该字段有效 */
    oal_uint16 us_ch_freq;
    oal_uint16 us_ch_type;
    oal_int8 c_ssi_signal;
    oal_int8 c_ssi_noise;
    oal_int16 s_signal_quality;
    oal_uint8 uc_mcs_info_known = 0; /* mcs信息, 11n协议时该字段有效 */
    oal_uint8 uc_mcs_info_flags = 0;
    oal_uint8 uc_mcs_info_rate = 0;
    oal_uint16 us_vht_known = 0; /* vht信息, 11ac协议时该字段有效 */
    oal_uint8 uc_vht_flags = 0;
    oal_uint8 uc_vht_bandwidth = 0;
    oal_uint8 uc_vht_mcs_nss[4] = { 0 };
    oal_uint8 uc_vht_coding = 0;
    oal_uint8 uc_vht_group_id = 0;
    oal_uint16 us_vht_partial_aid = 0;
    mac_ieee80211_frame_stru *pst_mac_head = OAL_PTR_NULL;
    oal_uint8 uc_frame_type;
    oal_uint32 ul_loop = 0;

    /* 填写fields字段里的flags成员 */
    pst_mac_head = (mac_ieee80211_frame_stru *)puc_mac_hdr;
    uc_frame_type = mac_get_frame_type((oal_uint8 *)puc_mac_hdr);

    /* 设置 pst_rx_status->bit_preabmle，1105没有，暂时设置为0 */
    /* 1103 rx himit flag = 1103 bit_preabmle */
    hmac_fill_radiotap_set_bit(&uc_flags, pst_rx_status->bit_rx_himit_flag, (oal_uint8)IEEE80211_RADIOTAP_F_SHORTPRE);
    hmac_fill_radiotap_set_bit(&uc_flags, pst_mac_head->st_frame_control.bit_more_frag,
                               (oal_uint8)IEEE80211_RADIOTAP_F_FRAG);
    hmac_fill_radiotap_set_bit(&uc_flags, pst_rx_status->bit_gi_type, (oal_uint8)IEEE80211_RADIOTAP_F_SHORTGI);

    /* 填写fields字段中的其他成员ch_freq、ch_type、ssi_signal、ssi_noise、signal_quality */
    /* todo:增加一个对FCS校验错误帧个数的统计 */
    hmac_fill_radiotap_set_bit(&uc_flags, (pst_rx_status->bit_dscr_status == HAL_RX_FCS_ERROR),
        IEEE80211_RADIOTAP_F_BADFCS);

    c_ssi_signal =
        (pst_sniffer_rx_statistic->c_rssi_dbm != OAL_RSSI_INIT_VALUE) ? (pst_sniffer_rx_statistic->c_rssi_dbm) : 0;

    /* snr_ant是以0.5dB为单位，实际使用前需要先除以2，且超出正常snr表示范围的用最小snr表示 */
    hmac_fill_radiotap_antenna_noise(&c_ssi_noise, c_ssi_signal, pst_sniffer_rx_statistic);

    s_signal_quality = c_ssi_signal - HMAC_PKT_CAP_SIGNAL_OFFSET;

    /* 接收的信道需通过查找接收描述符里接收帧的信道获得 */
    if (pst_rx_ctrl->uc_channel_number < 36) {
        us_ch_freq = 5 * pst_rx_ctrl->uc_channel_number + WLAN_2G_CENTER_FREQ_BASE;
        us_ch_type = (oal_uint16)IEEE80211_CHAN_2GHZ | (oal_uint16)IEEE80211_CHAN_DYN;
    } else {
        us_ch_freq = 5 * pst_rx_ctrl->uc_channel_number + WLAN_5G_CENTER_FREQ_BASE;
        us_ch_type = (oal_uint16)IEEE80211_CHAN_5GHZ | (oal_uint16)IEEE80211_CHAN_OFDM;
    }

#ifdef _PRE_WLAN_FEATURE_MONITOR_DEBUG
    OAM_WARNING_LOG1(0, OAM_SF_11AX, "{hmac_sniffer_fill_radiotap::bit_protocol_mode[%d]}",
                     pst_per_rate->un_nss_rate.st_ht_rate.bit_protocol_mode);
#endif
    /*
     * 填写fields字段中的速率信息, 根据radiotap的要求,
     * 11n时mcs_info有效、11ac时vht_info有效、11ag和11b时data_rate有效
     */
    if (pst_per_rate->un_nss_rate.st_ht_rate.bit_protocol_mode == WLAN_HT_PHY_PROTOCOL_MODE) {
        uc_mcs_info_known = (oal_uint8)IEEE80211_RADIOTAP_MCS_HAVE_BW |
                            (oal_uint8)IEEE80211_RADIOTAP_MCS_HAVE_MCS |
                            (oal_uint8)IEEE80211_RADIOTAP_MCS_HAVE_GI |
                            (oal_uint8)IEEE80211_RADIOTAP_MCS_HAVE_FMT |
                            (oal_uint8)IEEE80211_RADIOTAP_MCS_HAVE_FEC;
        /* 描述符里BW只有20, 20L和20U, 并没有40M的选项 */
        hmac_fill_radiotap_set_bit(&uc_mcs_info_flags,
                                   (oal_uint8)(pst_per_rate->uc_bandwidth != WLAN_BAND_ASSEMBLE_20M),
                                   (1 << (oal_uint8)IEEE80211_RADIOTAP_MCS_BW_40));
        hmac_fill_radiotap_set_bit(&uc_mcs_info_flags,
                                   pst_per_rate->uc_short_gi,
                                   (oal_uint8)IEEE80211_RADIOTAP_MCS_SGI);
        hmac_fill_radiotap_set_bit(&uc_mcs_info_flags,
                                   pst_per_rate->bit_preamble,
                                   (oal_uint8)IEEE80211_RADIOTAP_MCS_FMT_GF);
        hmac_fill_radiotap_set_bit(&uc_mcs_info_flags,
                                   pst_per_rate->bit_channel_code,
                                   (oal_uint8)IEEE80211_RADIOTAP_MCS_FEC_LDPC);
        hmac_fill_radiotap_set_bit(&uc_mcs_info_flags,
                                   pst_per_rate->bit_stbc,
                                   (oal_uint8)IEEE80211_RADIOTAP_MCS_STBC_SHIFT);

        /* 填写对应的mcs速率信息 */
        uc_mcs_info_rate = pst_per_rate->un_nss_rate.st_ht_rate.bit_ht_mcs;
#ifdef _PRE_WLAN_FEATURE_MONITOR_DEBUG
        OAM_WARNING_LOG2(0, OAM_SF_11AX, "{hmac_sniffer_fill_radiotap::uc_mcs_info_flags[0x%x] uc_mcs_info_rate[%d]}",
                         uc_mcs_info_flags, uc_mcs_info_rate);
#endif
    } else if (pst_per_rate->un_nss_rate.st_ht_rate.bit_protocol_mode == WLAN_VHT_PHY_PROTOCOL_MODE) {
        us_vht_known = (oal_uint16)IEEE80211_RADIOTAP_VHT_KNOWN_STBC |
                       (oal_uint16)IEEE80211_RADIOTAP_VHT_KNOWN_TXOP_PS_NA |
                       (oal_uint16)IEEE80211_RADIOTAP_VHT_KNOWN_GI |
                       (oal_uint16)IEEE80211_RADIOTAP_VHT_KNOWN_BEAMFORMED |
                       (oal_uint16)IEEE80211_RADIOTAP_VHT_KNOWN_BANDWIDTH |
                       (oal_uint16)IEEE80211_RADIOTAP_VHT_KNOWN_GROUP_ID |
                       (oal_uint16)IEEE80211_RADIOTAP_VHT_KNOWN_PARTIAL_AID;
        /* vht对应的flags信息, 包括STBC、Short GI等 */
        hmac_fill_radiotap_set_bit(&uc_vht_flags, (pst_per_rate->bit_stbc != 0), IEEE80211_RADIOTAP_VHT_FLAG_STBC);
        hmac_fill_radiotap_set_bit(&uc_vht_flags, pst_per_rate->uc_short_gi, IEEE80211_RADIOTAP_VHT_FLAG_SGI);

        /* 填写对应的vht带宽信息 */
        if (pst_per_rate->uc_bandwidth == WLAN_BAND_ASSEMBLE_20M) {
            uc_vht_bandwidth = uc_vht_bandwidth | IEEE80211_RADIOTAP_VHT_BW_20;
        } else if (pst_per_rate->uc_bandwidth == WLAN_BAND_ASSEMBLE_40M ||
                   pst_per_rate->uc_bandwidth == WLAN_BAND_ASSEMBLE_40M_DUP) {
            uc_vht_bandwidth = uc_vht_bandwidth | IEEE80211_RADIOTAP_VHT_BW_40;
        } else if (pst_per_rate->uc_bandwidth == WLAN_BAND_ASSEMBLE_80M ||
                   pst_per_rate->uc_bandwidth == WLAN_BAND_ASSEMBLE_80M_DUP) {
            uc_vht_bandwidth = uc_vht_bandwidth | IEEE80211_RADIOTAP_VHT_BW_80;
        } else {
            uc_vht_bandwidth = uc_vht_bandwidth | IEEE80211_RADIOTAP_VHT_BW_160;
        }

        /* 填写对应的vht速率信息、编码方式 */
        uc_vht_mcs_nss[0] = (pst_per_rate->un_nss_rate.st_vht_nss_mcs.bit_vht_mcs << 4) +
                            (pst_per_rate->un_nss_rate.st_vht_nss_mcs.bit_nss_mode + 1);
        for (ul_loop = 1; (ul_loop < 4) && (ul_loop < (pst_per_rate->un_nss_rate.st_vht_nss_mcs.bit_nss_mode + 1));
             ul_loop++) {
            uc_vht_mcs_nss[ul_loop] = uc_vht_mcs_nss[0];
        }
        hmac_fill_radiotap_set_bit(&uc_vht_coding, pst_per_rate->bit_channel_code,
                                   (oal_uint8)IEEE80211_RADIOTAP_CODING_LDPC_USER0);
#ifdef _PRE_WLAN_FEATURE_MONITOR_DEBUG
        OAM_WARNING_LOG3(0, OAM_SF_11AX,
                         "{hmac_sniffer_fill_radiotap::uc_vht_flags[0x%x] uc_vht_bandwidth[%d], uc_vht_mcs_nss[%d]}",
                         uc_vht_flags, uc_vht_bandwidth, uc_vht_mcs_nss[0]);
#endif
    } else {
        uc_data_rate = (oal_uint8)((*pul_rate_kbps) / HMAC_PKT_CAP_RATE_UNIT);
    }

#ifdef _PRE_WLAN_FEATURE_MONITOR_DEBUG
    OAM_WARNING_LOG3(0, OAM_SF_11AX,
                     "{hmac_sniffer_fill_radiotap::frame type[0x%x], uc_data_rate[%d], bit_protocol_mode[%d]}",
                     uc_frame_type, uc_data_rate,
                     pst_rx_status->un_nss_rate.st_ht_rate.bit_protocol_mode);

#endif
    pst_radiotap->st_radiotap_header.it_version = PKTHDR_RADIOTAP_VERSION;
    pst_radiotap->st_radiotap_header.it_pad = 0;
    pst_radiotap->st_radiotap_header.it_len = OAL_SIZEOF(ieee80211_radiotap_stru);
    pst_radiotap->st_radiotap_header.it_present = 0;

    /* API for 32bit integer */
    hmac_fill_radiotap_add_cap(&pst_radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_TSFT);
    hmac_fill_radiotap_add_cap(&pst_radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_FLAGS);
    hmac_fill_radiotap_add_cap(&pst_radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_RATE);
    hmac_fill_radiotap_add_cap(&pst_radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_CHANNEL);
    hmac_fill_radiotap_add_cap(&pst_radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_DBM_ANTSIGNAL);
    hmac_fill_radiotap_add_cap(&pst_radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_DBM_ANTNOISE);
    hmac_fill_radiotap_add_cap(&pst_radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_LOCK_QUALITY);
    hmac_fill_radiotap_add_cap(&pst_radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_MCS);
    hmac_fill_radiotap_add_cap(&pst_radiotap->st_radiotap_header.it_present, 1, IEEE80211_RADIOTAP_VHT);

    pst_radiotap->st_radiotap_fields.ull_timestamp = 0;
    pst_radiotap->st_radiotap_fields.uc_flags = uc_flags;
    pst_radiotap->st_radiotap_fields.uc_data_rate = uc_data_rate;
    pst_radiotap->st_radiotap_fields.us_channel_freq = us_ch_freq;
    pst_radiotap->st_radiotap_fields.us_channel_type = us_ch_type;
    pst_radiotap->st_radiotap_fields.c_ssi_signal = c_ssi_signal;
    pst_radiotap->st_radiotap_fields.c_ssi_noise = c_ssi_noise;
    pst_radiotap->st_radiotap_fields.s_signal_quality = s_signal_quality;

    pst_radiotap->st_radiotap_fields.uc_mcs_info_known = uc_mcs_info_known;
    pst_radiotap->st_radiotap_fields.uc_mcs_info_flags = uc_mcs_info_flags;
    pst_radiotap->st_radiotap_fields.uc_mcs_info_rate = uc_mcs_info_rate;

    pst_radiotap->st_radiotap_fields.us_vht_known = us_vht_known;
    pst_radiotap->st_radiotap_fields.uc_vht_flags = uc_vht_flags;
    pst_radiotap->st_radiotap_fields.uc_vht_bandwidth = uc_vht_bandwidth;
    pst_radiotap->st_radiotap_fields.uc_vht_mcs_nss[0] = uc_vht_mcs_nss[0];
    pst_radiotap->st_radiotap_fields.uc_vht_mcs_nss[1] = uc_vht_mcs_nss[1];
    pst_radiotap->st_radiotap_fields.uc_vht_mcs_nss[2] = uc_vht_mcs_nss[2];
    pst_radiotap->st_radiotap_fields.uc_vht_mcs_nss[3] = uc_vht_mcs_nss[3];
    pst_radiotap->st_radiotap_fields.uc_vht_coding = uc_vht_coding;
    pst_radiotap->st_radiotap_fields.uc_vht_group_id = uc_vht_group_id;
    pst_radiotap->st_radiotap_fields.us_vht_partial_aid = us_vht_partial_aid;
}
#endif
#ifdef _PRE_WLAN_FEATURE_SNIFFER

oal_void hmac_init_sniffer_cmd_function()
{
    static int isFuncSet = 0;
    if (!isFuncSet) {
        isFuncSet = 1;
        proc_set_hipriv_func(wal_hipriv_parse_cmd_etc);
    }
}
#endif

oal_uint32 hmac_rx_process_mgmt_event_etc(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = OAL_PTR_NULL;
    frw_event_hdr_stru *pst_event_hdr = OAL_PTR_NULL;
    dmac_wlan_crx_event_stru *pst_crx_event = OAL_PTR_NULL;
    oal_netbuf_stru *pst_netbuf = OAL_PTR_NULL; /* 用于保存管理帧指向的NETBUF */
    hmac_vap_stru *pst_vap = OAL_PTR_NULL;      /* vap指针 */
    oal_uint32 ul_ret;
#ifdef _PRE_WLAN_FEATURE_MONITOR
    mac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL; /* 指向MPDU控制块信息的指针 */
    oal_uint8 *puc_mac_hdr = OAL_PTR_NULL;
    hmac_device_stru *pst_hmac_device;
#endif

    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_process_mgmt_event_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件头和事件结构体指针 */
    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);
    pst_crx_event = (dmac_wlan_crx_event_stru *)(pst_event->auc_event_data);
    pst_netbuf = pst_crx_event->pst_netbuf;

#ifdef _PRE_WLAN_FEATURE_MONITOR
    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_event_hdr->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{hmac_rx_process_mgmt_event_etc::hmac_res_get_mac_dev_etc fail.device_id = %u}",
                       pst_event_hdr->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_SNIFFER
    hmac_init_sniffer_cmd_function();
#endif

    if (pst_hmac_device->en_monitor_mode == OAL_TRUE) {
        /* 获取该MPDU的控制信息 */
        pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
        puc_mac_hdr = (oal_uint8 *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_ctrl);
        if (mac_get_frame_type(puc_mac_hdr) == WLAN_FC0_TYPE_CTL) {
            pst_hmac_device->ul_control_frames_cnt++;
        } else if (mac_get_frame_type(puc_mac_hdr) == WLAN_FC0_TYPE_MGT) {
            pst_hmac_device->ul_mgmt_frames_cnt++;
        } else {
            pst_hmac_device->ul_others_frames_cnt++;
        }

        hmac_sniffer_save_data(pst_netbuf, 1);
        oal_netbuf_free(pst_netbuf); /* todo: 由使用者释放 */
        return OAL_SUCC;
    }
#endif

    /* 获取vap结构信息 */
    pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_UNLIKELY(pst_vap == OAL_PTR_NULL)) {
        OAM_WARNING_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_rx_delba_event_etc::pst_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 接收管理帧是状态机的一个输入，调用状态机接口 */
    if (pst_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        ul_ret = hmac_fsm_call_func_ap_etc(pst_vap, HMAC_FSM_INPUT_RX_MGMT, pst_crx_event);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_BA,
                             "{hmac_rx_process_mgmt_event_etc::hmac_fsm_call_func_ap_etc fail.err code1 [%u]}", ul_ret);
        }
    } else if (pst_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        ul_ret = hmac_fsm_call_func_sta_etc(pst_vap, HMAC_FSM_INPUT_RX_MGMT, pst_crx_event);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_BA,
                             "{hmac_rx_process_mgmt_event_etc::hmac_fsm_call_func_ap_etc fail.err code2 [%u]}", ul_ret);
        }
    }

    /* 管理帧统一释放接口 */
    oal_netbuf_free(pst_netbuf);

    return OAL_SUCC;
}


oal_uint32 hmac_mgmt_tbtt_event_etc(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = OAL_PTR_NULL;
    frw_event_hdr_stru *pst_event_hdr = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    hmac_misc_input_stru st_misc_input;
    oal_uint32 ul_ret;

    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_mgmt_tbtt_event_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(&st_misc_input, OAL_SIZEOF(hmac_misc_input_stru), 0, OAL_SIZEOF(hmac_misc_input_stru));

    /* 获取事件头和事件结构体指针 */
    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);

    pst_hmac_vap = mac_res_get_hmac_vap(pst_event_hdr->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_ANY, "{hmac_mgmt_tbtt_event_etc::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_misc_input.en_type = HMAC_MISC_TBTT;

    /* 调用sta状态机，只有sta的tbtt事件上报到hmac */
    ul_ret = hmac_fsm_call_func_sta_etc(pst_hmac_vap, HMAC_FSM_INPUT_MISC, &st_misc_input);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_ANY,
                         "{hmac_mgmt_tbtt_event_etc::hmac_fsm_call_func_sta_etc fail. erro code is %u}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_mgmt_send_disasoc_deauth_event_etc(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = OAL_PTR_NULL;
    frw_event_hdr_stru *pst_event_hdr = OAL_PTR_NULL;
    dmac_diasoc_deauth_event *pst_disasoc_deauth_event = OAL_PTR_NULL;
    oal_uint8 *puc_da = OAL_PTR_NULL;      /* 保存用户目的地址的指针 */
    hmac_vap_stru *pst_vap = OAL_PTR_NULL; /* vap指针 */
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_uint32 ul_rslt;
    oal_uint16 us_user_idx;
    oal_uint8 uc_event;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    oal_uint16 us_err_code;
#ifdef _PRE_WLAN_FEATURE_P2P
    mac_vap_stru *pst_up_vap1;
    mac_vap_stru *pst_up_vap2;
    mac_device_stru *pst_mac_device;
#endif

    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ASSOC, "{hmac_mgmt_send_disasoc_deauth_event_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件头和事件结构体指针 */
    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);
    pst_disasoc_deauth_event = (dmac_diasoc_deauth_event *)(pst_event->auc_event_data);

    /* 获取vap结构信息 */
    pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_UNLIKELY(pst_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_mgmt_send_disasoc_deauth_event_etc::pst_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap = &pst_vap->st_vap_base_info;

    /* 获取目的用户的MAC ADDR */
    puc_da = pst_disasoc_deauth_event->auc_des_addr;
    uc_event = pst_disasoc_deauth_event->uc_event;
    us_err_code = pst_disasoc_deauth_event->uc_reason;

    /* 发送去认证, 未关联状态收到第三类帧 */
    if (uc_event == DMAC_WLAN_CRX_EVENT_SUB_TYPE_DEAUTH) {
        hmac_mgmt_send_deauth_frame_etc(pst_mac_vap,
                                        puc_da,
                                        us_err_code,
                                        OAL_FALSE);  // 非PMF

#ifdef _PRE_WLAN_FEATURE_P2P
        pst_mac_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
        if (pst_mac_device == OAL_PTR_NULL) {
            return OAL_SUCC;
        }
        /* 判断是异频DBAC模式时，无法判断是哪个信道收到的数据帧，两个信道都需要发去认证 */
        ul_rslt = mac_device_find_2up_vap_etc(pst_mac_device, &pst_up_vap1, &pst_up_vap2);
        if (ul_rslt != OAL_SUCC) {
            return OAL_SUCC;
        }

        if (pst_up_vap1->st_channel.uc_chan_number == pst_up_vap2->st_channel.uc_chan_number) {
            return OAL_SUCC;
        }

        /* 获取另一个VAP */
        if (pst_mac_vap->uc_vap_id != pst_up_vap1->uc_vap_id) {
            pst_up_vap2 = pst_up_vap1;
        }

        /* 另外一个VAP也发去认证帧。error code加上特殊标记，组去认证帧时要修改源地址 */
        hmac_mgmt_send_deauth_frame_etc(pst_up_vap2,
                                        puc_da,
                                        us_err_code | MAC_SEND_TWO_DEAUTH_FLAG,
                                        OAL_FALSE);
#endif

        return OAL_SUCC;
    }

    /* 获取发送端的用户指针 */
    ul_rslt = mac_vap_find_user_by_macaddr_etc(pst_mac_vap, puc_da, &us_user_idx);
    if (ul_rslt != OAL_SUCC) {
        OAM_WARNING_LOG4(0, OAM_SF_RX,
            "{hmac_mgmt_send_disasoc_deauth_event_etc::Hmac cannot find USER by addr[%02X:XX:XX:%02X:%02X:%02X], \
            just del DMAC user}", puc_da[0], puc_da[3], puc_da[4], puc_da[5]);

        /*
         * 找不到用户，说明用户已经删除，直接返回成功，
         * 不需要再抛事件到dmac删除用户(统一由hmac_user_del来管理删除用户)
         */
        return OAL_SUCC;
    }

    /* 获取到hmac user,使用protected标志 */
    pst_hmac_user = mac_res_get_hmac_user_etc(us_user_idx);

    hmac_mgmt_send_disassoc_frame_etc(pst_mac_vap, puc_da, us_err_code,
                                      ((pst_hmac_user == OAL_PTR_NULL) ?
                                      OAL_FALSE : pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active));

    if (pst_hmac_user != OAL_PTR_NULL) {
        hmac_handle_disconnect_rsp_etc(pst_vap, pst_hmac_user, us_err_code);
    }

    /* 删除用户 */
    hmac_user_del_etc(pst_mac_vap, pst_hmac_user);

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
    OAM_WARNING_LOG1(0, OAM_SF_ASSOC, "{hmac_disassoc_reason_exchange::Unkown driver_disasoc_reason[%d].}",
                     en_driver_disasoc_reason);

    return MAC_UNSPEC_REASON;
}


oal_uint32 hmac_proc_disasoc_misc_event_etc(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_bool_enum_uint8 en_is_protected = OAL_FALSE; /* PMF */
    dmac_disasoc_misc_stru *pdmac_disasoc_misc_stru = OAL_PTR_NULL;
    mac_reason_code_enum_uint16 en_disasoc_reason_code = MAC_UNSPEC_REASON;

    if (OAL_UNLIKELY(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ASSOC, "{hmac_proc_disasoc_misc_event_etc::pst_event_mem is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pdmac_disasoc_misc_stru = (dmac_disasoc_misc_stru *)pst_event->auc_event_data;
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ASSOC, "{hmac_proc_disasoc_misc_event_etc::pst_hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAM_WARNING_LOG2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                     "{hmac_proc_disasoc_misc_event_etc::Device noticed to dissasoc user[%d] within reason[%d]!}",
                     pdmac_disasoc_misc_stru->us_user_idx,
                     pdmac_disasoc_misc_stru->en_disasoc_reason);

#ifdef _PRE_WLAN_1103_CHR
    hmac_chr_set_disasoc_reason(pdmac_disasoc_misc_stru->us_user_idx, pdmac_disasoc_misc_stru->en_disasoc_reason);
#endif

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) && !defined(_PRE_PC_LINT) && !defined(WIN32))
    /* 较强信号(大于-65dBm)下出现link loss则通过Bcpu输出全部寄存器，同时该维测默认不生效 */
    /* 因为一旦出错需要重启手机才能正常使用，使用时需要手动修改g_ucLinklossLogSwitch=1，并编译版本 */
    if (g_ucLinklossLogSwitch &&
        (pdmac_disasoc_misc_stru->en_disasoc_reason == DMAC_DISASOC_MISC_LINKLOSS) &&
        (pst_hmac_vap->station_info.signal > -65)) {
        wifi_open_bcpu_set_etc(1);

#ifdef PLATFORM_DEBUG_ENABLE
        debug_uart_read_wifi_mem_etc(OAL_TRUE);
#endif
    }
#endif

#ifdef _PRE_WLAN_CHIP_TEST
    /* CCA测试时，需要保持STA不去关联, 如果有打开chip_test宏开关 */
    if (0 != g_st_dmac_test_mng.uc_chip_test_open) {
        /* 如果有打开cca测试 */
        if (1 == g_st_dmac_test_mng.uc_cca_flag) {
        }
    }
#endif

    if (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        pst_hmac_user = mac_res_get_hmac_user_etc(pdmac_disasoc_misc_stru->us_user_idx);
        if (pst_hmac_user == OAL_PTR_NULL) {
            OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                             "{hmac_proc_disasoc_misc_event_etc::pst_hmac_user[%d] is null.}",
                             pdmac_disasoc_misc_stru->us_user_idx);
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_is_protected = pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active;
#ifdef _PRE_WLAN_1103_CHR
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
                             CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_SOFTAP_DISCONNECT,
                             pdmac_disasoc_misc_stru->en_disasoc_reason);
#endif
        /* 抛事件上报内核，已经去关联某个STA */
        hmac_handle_disconnect_rsp_ap_etc(pst_hmac_vap, pst_hmac_user);

        en_disasoc_reason_code = MAC_ASOC_NOT_AUTH;

        /* 发去关联帧 */
        hmac_mgmt_send_disassoc_frame_etc(&pst_hmac_vap->st_vap_base_info,
                                          pst_hmac_user->st_user_base_info.auc_user_mac_addr,
                                          en_disasoc_reason_code, en_is_protected);
        /* 删除用户 */
        hmac_user_del_etc(&pst_hmac_vap->st_vap_base_info, pst_hmac_user);

    } else {
        /* 获用户 */
        pst_hmac_user = mac_res_get_hmac_user_etc(pst_hmac_vap->st_vap_base_info.us_assoc_vap_id);
        if (pst_hmac_user == OAL_PTR_NULL) {
            /* 和ap侧一样，上层已经删除了的话，属于正常 */
            OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                             "{hmac_proc_disasoc_misc_event_etc::pst_hmac_user[%d] is null.}",
                             pst_hmac_vap->st_vap_base_info.us_assoc_vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_is_protected = pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active;

        /* 上报断链类型的转化 */
        en_disasoc_reason_code = hmac_disassoc_reason_exchange(pdmac_disasoc_misc_stru->en_disasoc_reason);

        if (pdmac_disasoc_misc_stru->en_disasoc_reason != DMAC_DISASOC_MISC_CHANNEL_MISMATCH) {
            /* 发送去认证帧到AP */
            hmac_mgmt_send_disassoc_frame_etc(&pst_hmac_vap->st_vap_base_info,
                                              pst_hmac_user->st_user_base_info.auc_user_mac_addr,
                                              en_disasoc_reason_code, en_is_protected);
        }

        /* 删除对应用户 */
        hmac_user_del_etc(&pst_hmac_vap->st_vap_base_info, pst_hmac_user);
        hmac_sta_handle_disassoc_rsp_etc(pst_hmac_vap, en_disasoc_reason_code);
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_ROAM

oal_uint32 hmac_proc_roam_trigger_event_etc(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event;
    hmac_vap_stru *pst_hmac_vap;
    oal_int8 c_rssi;

    if (OAL_UNLIKELY(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_proc_roam_trigger_event_etc::pst_event_mem is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    c_rssi = *(oal_int8 *)pst_event->auc_event_data;
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{hmac_proc_roam_trigger_event_etc::pst_hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_roam_trigger_handle_etc(pst_hmac_vap, c_rssi, OAL_TRUE);

    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_ROAM

/*lint -e19*/
oal_module_symbol(hmac_mgmt_tx_priv_req_etc);
/*lint +e19*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

