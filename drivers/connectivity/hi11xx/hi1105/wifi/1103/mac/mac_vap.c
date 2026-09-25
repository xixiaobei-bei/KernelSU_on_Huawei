

/* 1 头文件包含 */
#include "oal_mem.h"
#include "wlan_spec.h"
#include "wlan_types.h"
#include "hal_ext_if.h"
#include "mac_vap.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#include "mac_ie.h"
#include "dmac_ext_if.h"
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST) ||  \
    (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_HOST)) &&  \
    (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "board.h"
#endif
#include "securec.h"
#include "securectype.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_VAP_C

/* 2 全局变量定义 */
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
mac_tx_large_amsdu_ampdu_stru g_st_tx_large_amsdu = { 0 };
#endif
#ifdef _PRE_WLAN_TCP_OPT
mac_tcp_ack_filter_stru g_st_tcp_ack_filter = { 0 };
#endif
mac_rx_buffer_size_stru g_st_rx_buffer_size_stru = { 0 };
mac_small_amsdu_switch_stru g_st_small_amsdu_switch = { 0 };

mac_tcp_ack_buf_switch_stru g_st_tcp_ack_buf_switch = { 0 };

mac_vap_rom_stru g_mac_vap_rom[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];

mac_rx_dyn_bypass_extlna_stru g_st_rx_dyn_bypass_extlna_switch = { 0 };
#ifdef _PRE_WLAN_FEATURE_M2S
oal_bool_enum_uint8 g_en_mimo_blacklist_etc = OAL_TRUE;
#endif
#ifdef _PRE_WLAN_FEATURE_MBO
oal_uint8 g_uc_mbo_switch = 0;
#endif
oal_uint8 g_uc_dbac_dynamic_switch = 0;
#ifdef _PRE_WLAN_FEATURE_TWT

oal_uint32 mac_device_find_up_sta_wlan_etc(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    oal_uint8 uc_vap_idx;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_UNLIKELY(pst_mac_vap == OAL_PTR_NULL)) {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN,
                             "mac_device_find_up_sta_wlan_etc::vap is null! vap id is %d",
                             pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if ((pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP || pst_mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE) &&
            (pst_mac_vap->en_p2p_mode == WLAN_LEGACY_VAP_MODE)) {
            *ppst_mac_vap = pst_mac_vap;

            return OAL_SUCC;
        }
    }

    *ppst_mac_vap = OAL_PTR_NULL;

    return OAL_FAIL;
}
#endif

mac_vap_stru *mac_vap_find_another_up_vap_by_mac_vap(mac_vap_stru* pst_src_vap)
{
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    mac_vap_stru *pst_another_vap_up = OAL_PTR_NULL;

    pst_mac_device = mac_res_get_dev_etc(pst_src_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(pst_src_vap->uc_vap_id,  OAM_SF_ANY,
            "mac_vap_find_another_vap_by_vap_id:: mac_device is NULL");
        return OAL_PTR_NULL;
    }

    /* 获取另一个vap */
    pst_another_vap_up = mac_device_find_another_up_vap_etc(pst_mac_device, pst_src_vap->uc_vap_id);
    if (pst_another_vap_up == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(pst_src_vap->uc_vap_id, OAM_SF_ANY, "mac_vap_find_another_vap_by_vap_id::\
            another vap is invaild");
        return OAL_PTR_NULL;
    }

    return pst_another_vap_up;

}

#ifdef _PRE_WLAN_FEATURE_11AX

oal_void mac_vap_init_mib_11ax(mac_vap_stru *pst_mac_vap)
{
    mac_device_stru *pst_mac_dev;
    mac_vap_rom_stru *pst_mac_vap_rom = OAL_PTR_NULL;
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_HOST)) && \
    (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_int32 l_chip_type = get_hi110x_subchip_type();
#endif
    pst_mac_dev = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);

    if (pst_mac_dev == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
            "{mac_vap_init_mib_11ax::pst_mac_dev[%d] null.}", pst_mac_vap->uc_device_id);

        return;
    }
    pst_mac_vap_rom = (mac_vap_rom_stru *)(pst_mac_vap->_rom);

    /* MAC Capabilities Info */
    mac_mib_set_HEOptionImplemented(pst_mac_vap, OAL_TRUE);
    mac_mib_set_OMIOptionImplemented(pst_mac_vap, pst_mac_vap->bit_rom_custom_switch);
    mac_mib_set_he_TWTOptionActivated(pst_mac_vap, OAL_TRUE); /* 第一阶段暂不支持 */
    mac_mib_set_he_OperatingModeIndication(pst_mac_vap, OAL_TRUE);
    mac_mib_set_he_TriggerMacPaddingDuration(pst_mac_vap, MAC_TRIGGER_FRAME_PADDING_DURATION16us); /* 16us */
    mac_mib_set_he_MaxAMPDULengthExponent(pst_mac_vap, MAC_HE_MAX_AMPDU_LEN_EXP_0); /* 2^(20+factor)-1字节 */
    mac_mib_set_he_MultiBSSIDImplemented(pst_mac_vap, OAL_TRUE);
    mac_mib_set_he_BSRSupport(pst_mac_vap, OAL_TRUE);
    mac_mib_set_he_OFDMARandomAccess(pst_mac_vap, OAL_TRUE);

    /* PHY Capabilities Info */
    if (WLAN_BAND_CAP_2G_5G == pst_mac_dev->en_band_cap) {
        mac_mib_set_he_DualBandSupport(pst_mac_vap, OAL_TRUE); /* 支持双频 */
    }
    mac_mib_set_he_LDPCCodingInPayload(pst_mac_vap, MAC_DEVICE_GET_CAP_LDPC(pst_mac_dev)); /* 支持LDPC编码 */
    mac_mib_set_he_SUBeamformer(pst_mac_vap, MAC_DEVICE_GET_CAP_SUBFER(pst_mac_dev));
    mac_mib_set_he_SUBeamformee(pst_mac_vap, MAC_DEVICE_GET_CAP_SUBFEE(pst_mac_dev));
    mac_mib_set_he_MUBeamformer(pst_mac_vap, MAC_DEVICE_GET_CAP_MUBFER(pst_mac_dev));
    /* 以下mib项只有host使用 */
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST) || \
    (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_HOST)) && \
    (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if ((l_chip_type == BOARD_VERSION_HI1105) && (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA)) {
        mac_mib_set_he_BeamformeeSTSBelow80Mhz(pst_mac_vap_rom, HE_BFEE_NTX_SUPP_STS_CAP_EIGHT);
        mac_mib_set_he_BeamformeeSTSOver80Mhz(pst_mac_vap_rom, HE_BFEE_NTX_SUPP_STS_CAP_EIGHT);
        mac_mib_set_HENumberSoundingDimensionsBelow80Mhz(pst_mac_vap_rom, MAC_DEVICE_GET_NSS_NUM(pst_mac_dev));
        mac_mib_set_HENumberSoundingDimensionsOver80Mhz(pst_mac_vap_rom, MAC_DEVICE_GET_NSS_NUM(pst_mac_dev));
        mac_mib_set_HENg16SUFeedback(pst_mac_vap_rom, OAL_TRUE);
        mac_mib_set_HENg16MUFeedback(pst_mac_vap_rom, OAL_TRUE);
        mac_mib_set_HECodebook75MUFeedback(pst_mac_vap_rom, OAL_TRUE);
        mac_mib_set_he_TriggeredSUBeamformingFeedback(pst_mac_vap_rom, OAL_TRUE);
        mac_mib_set_he_TriggeredCQIFeedback(pst_mac_vap_rom, OAL_TRUE);
    } else if ((l_chip_type == BOARD_VERSION_HI1105) && (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP)) {
        mac_mib_set_he_BeamformeeSTSBelow80Mhz(pst_mac_vap_rom, HE_BFEE_NTX_SUPP_STS_CAP_EIGHT);
        mac_mib_set_he_BeamformeeSTSOver80Mhz(pst_mac_vap_rom, HE_BFEE_NTX_SUPP_STS_CAP_EIGHT);
        mac_mib_set_HENumberSoundingDimensionsBelow80Mhz(pst_mac_vap_rom, MAC_DEVICE_GET_NSS_NUM(pst_mac_dev));
        mac_mib_set_HENumberSoundingDimensionsOver80Mhz(pst_mac_vap_rom, MAC_DEVICE_GET_NSS_NUM(pst_mac_dev));
        mac_mib_set_HENg16SUFeedback(pst_mac_vap_rom, OAL_TRUE);
        mac_mib_set_HENg16MUFeedback(pst_mac_vap_rom, OAL_FALSE);
        mac_mib_set_HECodebook75MUFeedback(pst_mac_vap_rom, OAL_FALSE);
        mac_mib_set_he_TriggeredSUBeamformingFeedback(pst_mac_vap_rom, OAL_FALSE);
        mac_mib_set_he_TriggeredCQIFeedback(pst_mac_vap_rom, OAL_FALSE);
    } else {
        mac_mib_set_he_BeamformeeSTSBelow80Mhz(pst_mac_vap_rom, HE_BFEE_NTX_SUPP_STS_CAP_FOUR);
        mac_mib_set_he_BeamformeeSTSOver80Mhz(pst_mac_vap_rom, HE_BFEE_NTX_SUPP_STS_CAP_FOUR);
        mac_mib_set_HENumberSoundingDimensionsBelow80Mhz(pst_mac_vap_rom, 0);
        mac_mib_set_HENumberSoundingDimensionsOver80Mhz(pst_mac_vap_rom, 0);
        mac_mib_set_HENg16SUFeedback(pst_mac_vap_rom, OAL_FALSE);
        mac_mib_set_HENg16MUFeedback(pst_mac_vap_rom, OAL_FALSE);
        mac_mib_set_HECodebook75MUFeedback(pst_mac_vap_rom, OAL_FALSE);
        mac_mib_set_he_TriggeredSUBeamformingFeedback(pst_mac_vap_rom, OAL_FALSE);
        mac_mib_set_he_TriggeredCQIFeedback(pst_mac_vap_rom, OAL_FALSE);
    }
#elif (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV)
    mac_mib_set_he_BeamformeeSTSBelow80Mhz(pst_mac_vap_rom, HE_BFEE_NTX_SUPP_STS_CAP_EIGHT);
    mac_mib_set_he_BeamformeeSTSOver80Mhz(pst_mac_vap_rom, HE_BFEE_NTX_SUPP_STS_CAP_EIGHT);
    mac_mib_set_HENumberSoundingDimensionsBelow80Mhz(pst_mac_vap_rom, MAC_DEVICE_GET_NSS_NUM(pst_mac_dev));
    mac_mib_set_HENumberSoundingDimensionsOver80Mhz(pst_mac_vap_rom, MAC_DEVICE_GET_NSS_NUM(pst_mac_dev));
    mac_mib_set_HENg16SUFeedback(pst_mac_vap_rom, OAL_TRUE);
    mac_mib_set_HENg16MUFeedback(pst_mac_vap_rom, OAL_FALSE);
    mac_mib_set_HECodebook75MUFeedback(pst_mac_vap_rom, OAL_FALSE);
    mac_mib_set_he_TriggeredSUBeamformingFeedback(pst_mac_vap_rom, OAL_FALSE);
    mac_mib_set_he_TriggeredCQIFeedback(pst_mac_vap_rom, OAL_FALSE);
#else
    mac_mib_set_he_BeamformeeSTSBelow80Mhz(pst_mac_vap_rom, HE_BFEE_NTX_SUPP_STS_CAP_FOUR);
    mac_mib_set_he_BeamformeeSTSOver80Mhz(pst_mac_vap_rom, HE_BFEE_NTX_SUPP_STS_CAP_FOUR);
    mac_mib_set_HENumberSoundingDimensionsBelow80Mhz(pst_mac_vap_rom, 0);
    mac_mib_set_HENumberSoundingDimensionsOver80Mhz(pst_mac_vap_rom, 0);
    mac_mib_set_HENg16SUFeedback(pst_mac_vap_rom, OAL_FALSE);
    mac_mib_set_HENg16MUFeedback(pst_mac_vap_rom, OAL_FALSE);
    mac_mib_set_HECodebook75MUFeedback(pst_mac_vap_rom, OAL_FALSE);
    mac_mib_set_he_TriggeredSUBeamformingFeedback(pst_mac_vap_rom, OAL_FALSE);
    mac_mib_set_he_TriggeredCQIFeedback(pst_mac_vap_rom, OAL_FALSE);
#endif

    mac_mib_set_HECodebook42SUFeedback(pst_mac_vap_rom, OAL_TRUE);

    mac_mib_set_PPEThresholdsRequired(pst_mac_vap_rom, OAL_TRUE);

    /* Tx Rx MCS NSS */
    mac_mib_set_he_HighestNSS(pst_mac_vap, MAC_DEVICE_GET_NSS_NUM(pst_mac_dev));

    /* Multi BSSID */
    mac_mib_set_he_MultiBSSIDActived(pst_mac_vap_rom, pst_mac_vap->bit_multi_bssid_custom_switch);

    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        mac_mib_set_he_MultiBSSIDActived(pst_mac_vap_rom, OAL_FALSE);
        mac_mib_set_he_TWTOptionActivated(pst_mac_vap, OAL_FALSE);
        mac_mib_set_he_TriggerMacPaddingDuration(pst_mac_vap, MAC_TRIGGER_FRAME_PADDING_DURATION0us);
        mac_mib_set_he_BSRSupport(pst_mac_vap, OAL_FALSE);
        mac_mib_set_he_SUBeamformee(pst_mac_vap, OAL_TRUE);
        mac_mib_set_he_MUBeamformer(pst_mac_vap, OAL_FALSE);
    }
}


OAL_STATIC oal_void mac_vap_init_11ax_mcs_singlenss(mac_vap_stru *pst_mac_vap,
                                                    wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    mac_tx_max_he_mcs_map_stru *pst_tx_max_mcs_map;
    mac_tx_max_he_mcs_map_stru *pst_rx_max_mcs_map;

    /* 获取mib值指针 */
    pst_rx_max_mcs_map = (mac_tx_max_he_mcs_map_stru *)(mac_mib_get_ptr_he_rx_mcs_map(pst_mac_vap));
    pst_tx_max_mcs_map = (mac_tx_max_he_mcs_map_stru *)(mac_mib_get_ptr_he_tx_mcs_map(pst_mac_vap));

    /* 1105支持mcs11 */
    pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
    pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
}


OAL_STATIC oal_void mac_vap_init_11ax_mcs_doublenss(mac_vap_stru *pst_mac_vap,
                                                    wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    mac_tx_max_he_mcs_map_stru *pst_tx_max_mcs_map;
    mac_rx_max_he_mcs_map_stru *pst_rx_max_mcs_map;

    /* 获取mib值指针 */
    pst_rx_max_mcs_map = (mac_tx_max_he_mcs_map_stru *)(mac_mib_get_ptr_he_rx_mcs_map(pst_mac_vap));
    pst_tx_max_mcs_map = (mac_tx_max_he_mcs_map_stru *)(mac_mib_get_ptr_he_tx_mcs_map(pst_mac_vap));

    /* 1105支持mcs11 */
    pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
    pst_rx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
    pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
    pst_tx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
}


oal_void mac_vap_init_11ax_rates(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev)
{
    /* 先将TX RX MCSMAP初始化为所有空间流都不支持 0xFFFF */
    mac_mib_set_he_rx_mcs_map(pst_mac_vap, 0xFFFF);
    mac_mib_set_he_tx_mcs_map(pst_mac_vap, 0xFFFF);

    if (WLAN_SINGLE_NSS == MAC_DEVICE_GET_NSS_NUM(pst_mac_dev)) {
        /* 1个空间流的情况 */
        mac_vap_init_11ax_mcs_singlenss(pst_mac_vap, pst_mac_vap->st_channel.en_bandwidth);
    } else if (WLAN_DOUBLE_NSS == MAC_DEVICE_GET_NSS_NUM(pst_mac_dev)) {
        /* 2个空间流的情况 */
        mac_vap_init_11ax_mcs_doublenss(pst_mac_vap, pst_mac_vap->st_channel.en_bandwidth);
    } else {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
            "{mac_vap_init_11ax_rates::invalid en_nss_num[%d].}", MAC_DEVICE_GET_NSS_NUM(pst_mac_dev));
    }
}
#else
oal_void mac_vap_init_mib_11ax(mac_vap_stru *pst_mac_vap)
{
    return;
}

oal_void mac_vap_init_11ax_rates(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev)
{
    return;
}
#endif


oal_void mac_vap_tx_data_set_user_htc_cap(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl_stru *pst_he_hdl = OAL_PTR_NULL;

    MAC_USER_TX_DATA_INCLUDE_HTC(pst_mac_user) = OAL_FALSE;
    MAC_USER_TX_DATA_INCLUDE_OM(pst_mac_user)  = OAL_FALSE;

    if (!IS_LEGACY_VAP(pst_mac_vap)) {
        return;
    }

    if ((pst_mac_vap->bit_htc_include_custom_switch == OAL_TRUE) &&
        MAC_VAP_IS_WORK_HE_PROTOCOL(pst_mac_vap) && MAC_USER_IS_HE_USER(pst_mac_user)) {
        pst_he_hdl = MAC_USER_HE_HDL_STRU(pst_mac_user);
        /* user he cap +HTC-HE Support 为1 时才会携带htc头 */
        if (pst_he_hdl->st_he_cap_ie.st_he_mac_cap.bit_htc_he_support) {
            MAC_USER_TX_DATA_INCLUDE_HTC(pst_mac_user) = OAL_TRUE;
            /* user 支持rom */
            if ((pst_mac_vap->bit_om_include_custom_switch == OAL_TRUE) &&
                (MAC_USER_HE_HDL_STRU(pst_mac_user)->st_he_cap_ie.st_he_mac_cap.bit_om_control_support)) {
                MAC_USER_TX_DATA_INCLUDE_OM(pst_mac_user) = OAL_TRUE;
            }
        }
    }

    if (MAC_USER_ARP_PROBE_CLOSE_HTC(pst_mac_user)) {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
            "{mac_vap_tx_data_set_user_htc_cap::bit_arp_probe_close_htc is true, to close htc.}");
        MAC_USER_TX_DATA_INCLUDE_HTC(pst_mac_user) = OAL_FALSE;
        MAC_USER_TX_DATA_INCLUDE_OM(pst_mac_user) = OAL_FALSE;
    }

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
        "{mac_vap_tx_data_set_user_htc_cap::tx_data_include_htc=[%d], tx_data_include_om=[%d], \
        vap_protocol=[%d], he_cap=[%d].}",
        MAC_USER_TX_DATA_INCLUDE_HTC(pst_mac_user),
        MAC_USER_TX_DATA_INCLUDE_OM(pst_mac_user),
        MAC_VAP_IS_WORK_HE_PROTOCOL(pst_mac_vap),
        MAC_USER_IS_HE_USER(pst_mac_user));

#endif
}


oal_void mac_vap_init_mib_11n_rom_cb(mac_vap_stru *pst_mac_vap)
{
    /* txbf能力信息 注:11n txbf用宏区分 */
#if (defined(_PRE_WLAN_FEATURE_TXBF) && defined(_PRE_WLAN_FEATURE_TXBF_HT))
    mac_device_stru *pst_dev;

    pst_dev = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (pst_dev == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_init_mib_11n_rom_cb::pst_dev null.}");
        return;
    }

    mac_mib_set_TransmitStaggerSoundingOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_SUBFER(pst_dev));
    mac_mib_set_ReceiveStaggerSoundingOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_SUBFEE(pst_dev));
    mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(pst_mac_vap, WLAN_MIB_HT_ECBF_DELAYED);
    mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(pst_mac_vap, HT_BFEE_NTX_SUPP_ANTA_NUM);
#else
    mac_mib_set_TransmitStaggerSoundingOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_ReceiveStaggerSoundingOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(pst_mac_vap, WLAN_MIB_HT_ECBF_INCAPABLE);
    mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(pst_mac_vap, 1);
#endif
}

oal_uint32 mac_vap_init_by_protocol_cb(mac_vap_stru *pst_mac_vap, wlan_protocol_enum_uint8 en_protocol)
{
#ifdef _PRE_WLAN_FEATURE_11AC2G
    if ((en_protocol == WLAN_HT_MODE) &&
        (pst_mac_vap->st_cap_flag.bit_11ac2g == OAL_TRUE) &&
        (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
        mac_mib_set_VHTOptionImplemented(pst_mac_vap, OAL_TRUE);
    }
#endif
    return OAL_SUCC;
}

oal_void mac_vap_init_11n_rates_cb(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev)
{
#ifdef _PRE_WLAN_FEATURE_11AC2G
    if ((pst_mac_vap->en_protocol == WLAN_HT_MODE) &&
        (pst_mac_vap->st_cap_flag.bit_11ac2g == OAL_TRUE) &&
        (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
        mac_vap_init_11ac_rates(pst_mac_vap, pst_mac_dev);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
    /* m2s spec模式下，需要根据vap nss能力 + cap是否支持siso，刷新空间流 */
    if (MAC_VAP_SPEC_IS_SW_NEED_M2S_SWITCH(pst_mac_vap) && IS_VAP_SINGLE_NSS(pst_mac_vap)) {
        /* 将MIB值的MCS MAP清零 */
        memset_s(mac_mib_get_SupportedMCSTx(pst_mac_vap), WLAN_HT_MCS_BITMASK_LEN, 0, WLAN_HT_MCS_BITMASK_LEN);
        memset_s(mac_mib_get_SupportedMCSRx(pst_mac_vap), WLAN_HT_MCS_BITMASK_LEN, 0, WLAN_HT_MCS_BITMASK_LEN);

        mac_mib_set_TxMaximumNumberSpatialStreamsSupported(pst_mac_vap, 1);
        mac_mib_set_SupportedMCSRxValue(pst_mac_vap, 0, 0xFF); /* 支持 RX MCS 0-7，8位全置为1 */
        mac_mib_set_SupportedMCSTxValue(pst_mac_vap, 0, 0xFF); /* 支持 TX MCS 0-7，8位全置为1 */

        mac_mib_set_HighestSupportedDataRate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_20M_11N);

        if ((pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS) ||
            (pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS)) {
            /* 40M 支持MCS32 */
            mac_mib_set_SupportedMCSRxValue(pst_mac_vap, 4, 0x01); /* 支持 RX MCS 32,最后一位为1 */
            mac_mib_set_SupportedMCSTxValue(pst_mac_vap, 4, 0x01); /* 支持 RX MCS 32,最后一位为1 */
            mac_mib_set_HighestSupportedDataRate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_40M_11N);
        }

        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{mac_vap_init_11n_rates_cb::m2s spec update rate to siso.}");
    }
#endif
}


oal_void mac_vap_init_11ac_rates_cb(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev)
{
#ifdef _PRE_WLAN_FEATURE_M2S
    /* m2s spec模式下，需要根据vap nss能力 + cap是否支持siso，刷新空间流 */
    if (MAC_VAP_SPEC_IS_SW_NEED_M2S_SWITCH(pst_mac_vap) && IS_VAP_SINGLE_NSS(pst_mac_vap)) {
        /* 先将TX RX MCSMAP初始化为所有空间流都不支持 0xFFFF */
        mac_mib_set_vht_rx_mcs_map(pst_mac_vap, 0xFFFF);
        mac_mib_set_vht_tx_mcs_map(pst_mac_vap, 0xFFFF);

        /* 1个空间流的情况 */
        mac_vap_init_11ac_mcs_singlenss(pst_mac_vap, pst_mac_vap->st_channel.en_bandwidth);

        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{mac_vap_init_11ac_rates_cb::m2s spec update rate to siso.}");
    }
#endif
}

oal_void mac_vap_init_rates_by_protocol_etc_cb(mac_vap_stru *pst_vap,
                                               wlan_protocol_enum_uint8 en_vap_protocol,
                                               mac_data_rate_stru *pst_rates)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_device_stru *pst_mac_dev;

    pst_mac_dev = (mac_device_stru *)mac_res_get_dev_etc(pst_vap->uc_device_id);
    if (pst_mac_dev == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY,
                       "{mac_vap_init_rates_by_protocol_etc::mac_res_get_dev_etc fail.device_id:[%d].}",
                       pst_vap->uc_device_id);
        return;
    }

    if ((pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) && (en_vap_protocol == WLAN_HE_MODE)) {
        if (pst_vap->st_channel.en_band == WLAN_BAND_2G) {
            mac_vap_init_11g_mixed_one_rates(pst_vap, pst_rates);
        } else {
            mac_vap_init_legacy_rates(pst_vap, pst_rates);
        }
        mac_vap_init_11n_rates(pst_vap, pst_mac_dev);
        mac_vap_init_11ac_rates(pst_vap, pst_mac_dev);
        mac_vap_rom_init_11ax_rates(pst_vap, pst_mac_dev);
    }
#endif
}


oal_void mac_vap_init_11ac_mcs_singlenss_rom_cb(mac_vap_stru *pst_mac_vap,
                                                wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
#ifdef _PRE_WLAN_FEATURE_160M
    mac_tx_max_mcs_map_stru *pst_tx_max_mcs_map;
    mac_rx_max_mcs_map_stru *pst_rx_max_mcs_map;

    /* 获取mib值指针 */
    pst_rx_max_mcs_map = (mac_rx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_rx_mcs_map(pst_mac_vap));
    pst_tx_max_mcs_map = (mac_tx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_tx_mcs_map(pst_mac_vap));

    if ((en_bandwidth == WLAN_BAND_WIDTH_160PLUSPLUSPLUS) ||
        (en_bandwidth == WLAN_BAND_WIDTH_160PLUSPLUSMINUS) ||
        (en_bandwidth == WLAN_BAND_WIDTH_160PLUSMINUSPLUS) ||
        (en_bandwidth == WLAN_BAND_WIDTH_160PLUSMINUSMINUS) ||
        (en_bandwidth == WLAN_BAND_WIDTH_160MINUSPLUSPLUS) ||
        (en_bandwidth == WLAN_BAND_WIDTH_160MINUSPLUSMINUS) ||
        (en_bandwidth == WLAN_BAND_WIDTH_160MINUSMINUSPLUS) ||
        (en_bandwidth == WLAN_BAND_WIDTH_160MINUSMINUSMINUS)) {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_160M_11AC);
        mac_mib_set_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_160M_11AC);
    }
#endif
}


oal_void mac_vap_init_11ac_mcs_doublenss_rom_cb(mac_vap_stru *pst_mac_vap,
                                                wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
#ifdef _PRE_WLAN_FEATURE_160M
    mac_tx_max_mcs_map_stru *pst_tx_max_mcs_map;
    mac_rx_max_mcs_map_stru *pst_rx_max_mcs_map;

    /* 获取mib值指针 */
    pst_rx_max_mcs_map = (mac_rx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_rx_mcs_map(pst_mac_vap));
    pst_tx_max_mcs_map = (mac_tx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_tx_mcs_map(pst_mac_vap));

    if ((en_bandwidth == WLAN_BAND_WIDTH_160PLUSPLUSPLUS) ||
        (en_bandwidth == WLAN_BAND_WIDTH_160PLUSPLUSMINUS) ||
        (en_bandwidth == WLAN_BAND_WIDTH_160PLUSMINUSPLUS) ||
        (en_bandwidth == WLAN_BAND_WIDTH_160PLUSMINUSMINUS) ||
        (en_bandwidth == WLAN_BAND_WIDTH_160MINUSPLUSPLUS) ||
        (en_bandwidth == WLAN_BAND_WIDTH_160MINUSPLUSMINUS) ||
        (en_bandwidth == WLAN_BAND_WIDTH_160MINUSMINUSPLUS) ||
        (en_bandwidth == WLAN_BAND_WIDTH_160MINUSMINUSMINUS)) {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_rx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_160M_11AC);
        mac_mib_set_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_160M_11AC);
    }
#endif
}


oal_void mac_vap_ch_mib_by_bw_cb(mac_vap_stru *pst_mac_vap,
                                 wlan_channel_band_enum_uint8 en_band,
                                 wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
#ifndef WIN32
    oal_bool_enum_uint8 en_40m_enable;

    en_40m_enable = (WLAN_BAND_WIDTH_20M != en_bandwidth);

    if (en_band == WLAN_BAND_2G) {
        mac_mib_set_2GFortyMHzOperationImplemented(pst_mac_vap, en_40m_enable);
    } else {
        mac_mib_set_5GFortyMHzOperationImplemented(pst_mac_vap, en_40m_enable);
    }
#endif
}

#if defined(_PRE_WLAN_FEATURE_11V) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
/* 默认支持11v 如需关闭请上层调用接口 */
OAL_STATIC oal_void mac_vap_init_mib_11v(mac_vap_stru *pst_vap)
{
    /* en_dot11MgmtOptionBSSTransitionActivated 初始化时为TRUE,由定制化或命令打开or关闭 */
    mac_mib_set_MgmtOptionBSSTransitionActivated(pst_vap, OAL_TRUE);
    mac_mib_set_MgmtOptionBSSTransitionImplemented(pst_vap, OAL_TRUE);
    mac_mib_set_WirelessManagementImplemented(pst_vap, OAL_TRUE);
}
#endif

oal_void mac_init_mib_rom_cb(mac_vap_stru *pst_mac_vap)
{
#if defined(_PRE_WLAN_FEATURE_11V) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    /* 11k */
    mac_vap_init_mib_11v(pst_mac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    /* txopps 初始化关闭状态 */
    mac_mib_set_txopps(pst_mac_vap, OAL_FALSE);
#endif

    mac_mib_set_dot11VHTExtendedNSSBWCapable(pst_mac_vap, OAL_TRUE);

    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        mac_mib_set_AuthenticationResponseTimeOut(pst_mac_vap, WLAN_AUTH_AP_TIMEOUT);
    }
}

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_VAP)
oal_void mac_blacklist_free_pointer(mac_vap_stru *pst_mac_vap, mac_blacklist_info_stru *pst_blacklist_info)
{
    if ((pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) ||
        (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA)) {
        memset_s(pst_blacklist_info, OAL_SIZEOF(mac_blacklist_info_stru), 0, OAL_SIZEOF(mac_blacklist_info_stru));
        pst_blacklist_info->uc_blacklist_device_index = 0xFF;
        pst_blacklist_info->uc_blacklist_vap_index = 0xFF;
    }
}
#endif
#ifdef _PRE_WLAN_FEATURE_PROXY_ARP

oal_err_code_enum_uint32 mac_proxy_init_vap(mac_vap_stru *pst_mac_vap)
{
    pst_mac_vap->pst_vap_proxyarp = OAL_PTR_NULL;
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_11AX
OAL_STATIC oal_void mac_vap_11ax_cap_init(mac_vap_stru *pst_mac_vap)
{
    pst_mac_vap->en_11ax_custom_switch            = g_pst_mac_device_capability[0].en_11ax_switch;
    pst_mac_vap->bit_multi_bssid_custom_switch    = g_pst_mac_device_capability[0].bit_multi_bssid_switch;
    pst_mac_vap->bit_htc_include_custom_switch    =
        g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_htc_include;
    pst_mac_vap->bit_om_include_custom_switch     =
        g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_om_in_data;
    pst_mac_vap->bit_rom_custom_switch            =
        g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_rom_cap_switch;
}
#endif
OAL_STATIC OAL_INLINE oal_void mac_vap_update_ip_filter(mac_vap_stru *pst_vap,
    mac_cfg_add_vap_param_stru *pst_param)
{
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    if (IS_STA(pst_vap) && (pst_param->en_p2p_mode == WLAN_LEGACY_VAP_MODE)) {
        /* 仅LEGACY_STA支持 */
        pst_vap->st_cap_flag.bit_ip_filter = OAL_TRUE;
        pst_vap->st_cap_flag.bit_icmp_filter = OAL_TRUE;
    } else
#endif /* _PRE_WLAN_FEATURE_IP_FILTER */
    {
        pst_vap->st_cap_flag.bit_ip_filter = OAL_FALSE;
        pst_vap->st_cap_flag.bit_icmp_filter = OAL_FALSE;
    }
}
#ifdef _PRE_WLAN_FEATURE_TXBF_HT
OAL_STATIC OAL_INLINE oal_void mac_vap_init_txbf_ht_cap(mac_vap_stru *pst_vap)
{
    if (OAL_TRUE == mac_mib_get_TransmitStaggerSoundingOptionImplemented(pst_vap)) {
        pst_vap->st_txbf_add_cap.bit_exp_comp_txbf_cap = OAL_TRUE;
    }
    pst_vap->st_txbf_add_cap.bit_imbf_receive_cap = 0;
    pst_vap->st_txbf_add_cap.bit_channel_est_cap = 0;
    pst_vap->st_txbf_add_cap.bit_min_grouping = 0;
    pst_vap->st_txbf_add_cap.bit_csi_bfee_max_rows = 0;
    pst_vap->bit_ap_11ntxbf = 0;
    pst_vap->st_cap_flag.bit_11ntxbf = OAL_TRUE;
}
#endif

oal_void mac_vap_rom_init_etc(mac_vap_rom_stru *pst_mac_vap_rom, mac_cfg_add_vap_param_stru *pst_param)
{
    oal_bool_enum_uint8 temp_flag;
    /* 仅p2p device支持probe req应答模式切换 */
    pst_mac_vap_rom->st_probe_resp_ctrl.en_probe_resp_enable = OAL_FALSE;
    pst_mac_vap_rom->st_probe_resp_ctrl.en_probe_resp_status = MAC_PROBE_RESP_MODE_ACTIVE;
    temp_flag = ((pst_param->en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
                 (pst_param->en_p2p_mode == WLAN_P2P_DEV_MODE));
    if (temp_flag) {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        pst_mac_vap_rom->st_probe_resp_ctrl.en_probe_resp_enable = (oal_uint8)pst_param->probe_resp_enable;
        pst_mac_vap_rom->st_probe_resp_ctrl.en_probe_resp_status = (oal_uint8)pst_param->probe_resp_status;
#else
        pst_mac_vap_rom->st_probe_resp_ctrl.en_probe_resp_enable = OAL_FALSE;
        pst_mac_vap_rom->st_probe_resp_ctrl.en_probe_resp_status = MAC_PROBE_RESP_MODE_ACTIVE;
#endif // _PRE_PLAT_FEATURE_CUSTOMIZE
    }
    pst_mac_vap_rom->en_ps_rx_amsdu = OAL_TRUE;

}

oal_uint32 mac_vap_init_etc(mac_vap_stru *pst_vap,
                            oal_uint8 uc_chip_id,
                            oal_uint8 uc_device_id,
                            oal_uint8 uc_vap_id,
                            mac_cfg_add_vap_param_stru *pst_param)
{
    oal_uint32 ul_loop;
    wlan_mib_ieee802dot11_stru *pst_mib_info = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device = mac_res_get_dev_etc(uc_device_id);
    oal_uint8 *puc_addr = OAL_PTR_NULL;
    oal_bool_enum_uint8 temp_flag;
    mac_vap_rom_stru *pst_mac_vap_rom = OAL_PTR_NULL;

    if (OAL_UNLIKELY(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_vap_init_etc::pst_mac_device[%d] null!}", uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_vap->uc_chip_id = uc_chip_id;
    pst_vap->uc_device_id = uc_device_id;
    pst_vap->uc_vap_id = uc_vap_id;
    pst_vap->en_vap_mode = pst_param->en_vap_mode;
    pst_vap->ul_core_id = pst_mac_device->ul_core_id;

    pst_vap->bit_has_user_bw_limit = OAL_FALSE;
    pst_vap->bit_vap_bw_limit = 0;
#ifdef _PRE_WLAN_FEATURE_VO_AGGR
    pst_vap->bit_voice_aggr = OAL_TRUE;
#else
    pst_vap->bit_voice_aggr = OAL_FALSE;
#endif
    pst_vap->uc_random_mac = OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_EDCA_NO_INTERF_OPT
    pst_vap->bit_one_tx_tcp_be = OAL_FALSE;
    pst_vap->bit_one_rx_tcp_be = OAL_FALSE;
    pst_vap->bit_no_tcp_or_udp = OAL_FALSE;
#endif
    pst_vap->bit_bw_fixed = OAL_FALSE;
    pst_vap->bit_use_rts_threshold = OAL_FALSE;

    oal_set_mac_addr_zero(pst_vap->auc_bssid);

    for (ul_loop = 0; ul_loop < MAC_VAP_USER_HASH_MAX_VALUE; ul_loop++) {
        oal_dlist_init_head(&(pst_vap->ast_user_hash[ul_loop]));
    }

    /* cache user 锁初始化 */
    oal_spin_lock_init(&pst_vap->st_cache_user_lock);

    oal_dlist_init_head(&pst_vap->st_mac_user_list_head);

    /* 初始化支持2.4G 11ac私有增强 */
#ifdef _PRE_WLAN_FEATURE_11AC2G
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    pst_vap->st_cap_flag.bit_11ac2g = pst_param->bit_11ac2g_enable;
#else
    pst_vap->st_cap_flag.bit_11ac2g = OAL_TRUE;
#endif
#endif
    /* 默认APUT不支持随环境进行自动2040带宽切换 */
    pst_vap->st_cap_flag.bit_2040_autoswitch = OAL_FALSE;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    /* 根据定制化刷新2g ht40能力 */
    pst_vap->st_cap_flag.bit_disable_2ght40 = pst_param->bit_disable_capab_2ght40;
#else
    pst_vap->st_cap_flag.bit_disable_2ght40 = OAL_FALSE;
#endif
    mac_vap_update_ip_filter(pst_vap, pst_param);

    /* 将hera用于设置ie的指针置空，当接口命令下发后再申请内存 */
#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE
    pst_vap->pst_rrm_ie_info = OAL_PTR_NULL;
    pst_vap->pst_excap_ie_info = OAL_PTR_NULL;
    pst_vap->pst_msta_ie_info = OAL_PTR_NULL;
#endif

    pst_vap->_rom = &g_mac_vap_rom[uc_vap_id];
    memset_s(pst_vap->_rom, OAL_SIZEOF(mac_vap_rom_stru), 0, OAL_SIZEOF(mac_vap_rom_stru));

    pst_mac_vap_rom  = (mac_vap_rom_stru *)pst_vap->_rom;
    if (pst_mac_vap_rom != OAL_PTR_NULL) {
        mac_vap_rom_init_etc(pst_mac_vap_rom, pst_param);
    }

    switch (pst_vap->en_vap_mode) {
        case WLAN_VAP_MODE_CONFIG:
            pst_vap->uc_init_flag = MAC_VAP_VAILD;  // CFG VAP也需置位保证不重复初始化
            return OAL_SUCC;
        case WLAN_VAP_MODE_BSS_STA:
        case WLAN_VAP_MODE_BSS_AP:
            /* 设置vap参数默认值 */
            pst_vap->us_assoc_vap_id = MAC_INVALID_USER_ID;
            pst_vap->us_cache_user_id = MAC_INVALID_USER_ID;
            pst_vap->uc_tx_power = WLAN_MAX_TXPOWER;
            pst_vap->st_protection.en_protection_mode = WLAN_PROT_NO;

            pst_vap->st_cap_flag.bit_dsss_cck_mode_40mhz = OAL_TRUE;

            /* 初始化特性标识 */
            pst_vap->st_cap_flag.bit_uapsd = WLAN_FEATURE_UAPSD_IS_OPEN;
#ifdef _PRE_WLAN_FEATURE_UAPSD
#if defined(_PRE_PRODUCT_ID_HI110X_HOST) || defined(_PRE_PRODUCT_ID_HI110X_DEV)
            if (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
                pst_vap->st_cap_flag.bit_uapsd = g_uc_uapsd_cap_etc;
            }
#endif
#endif
            /* 初始化dpd能力 */
            pst_vap->st_cap_flag.bit_dpd_enbale = OAL_TRUE;

            pst_vap->st_cap_flag.bit_dpd_done = OAL_FALSE;
            /* 初始化TDLS prohibited关闭 */
            pst_vap->st_cap_flag.bit_tdls_prohibited = OAL_FALSE;
            /* 初始化TDLS channel switch prohibited关闭 */
            pst_vap->st_cap_flag.bit_tdls_channel_switch_prohibited = OAL_FALSE;

            /* 初始化KeepALive开关 */
            pst_vap->st_cap_flag.bit_keepalive = OAL_TRUE;
            /* 初始化安全特性值 */
            pst_vap->st_cap_flag.bit_wpa = OAL_FALSE;
            pst_vap->st_cap_flag.bit_wpa2 = OAL_FALSE;

            mac_vap_set_peer_obss_scan_etc(pst_vap, OAL_FALSE);

            /* 初始化协议模式与带宽为非法值，需通过配置命令配置 */
            pst_vap->st_channel.en_band = WLAN_BAND_BUTT;
            pst_vap->st_channel.en_bandwidth = WLAN_BAND_WIDTH_BUTT;
            pst_vap->st_channel.uc_chan_number = 0;
            pst_vap->en_protocol = WLAN_PROTOCOL_BUTT;

            /* 设置自动保护开启 */
            pst_vap->st_protection.bit_auto_protection = OAL_SWITCH_ON;

            memset_s(pst_vap->ast_app_ie, OAL_SIZEOF(mac_app_ie_stru) * OAL_APP_IE_NUM,
                     0, OAL_SIZEOF(mac_app_ie_stru) * OAL_APP_IE_NUM);
            /* 初始化vap wmm开关，默认打开 */
            pst_vap->en_vap_wmm = OAL_TRUE;

            /* 设置初始化rx nss值,之后按协议初始化 */
            pst_vap->en_vap_rx_nss = WLAN_NSS_LIMIT;

            /* 设置VAP状态为初始状态INIT */
            mac_vap_state_change_etc(pst_vap, MAC_VAP_STATE_INIT);

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
            mac_proxy_init_vap(pst_vap);
#endif /* #ifdef _PRE_WLAN_FEATURE_PROXY_ARP */

            /* 清mac vap下的uapsd的状态,否则状态会有残留，导致host device uapsd信息不同步 */
#ifdef _PRE_WLAN_FEATURE_STA_PM
            memset_s(&(pst_vap->st_sta_uapsd_cfg), OAL_SIZEOF(mac_cfg_uapsd_sta_stru),
                     0, OAL_SIZEOF(mac_cfg_uapsd_sta_stru));
#endif /* #ifdef _PRE_WLAN_FEATURE_STA_PM */
#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
            if (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
                mac_tx_qos_enhance_list_init(pst_vap);
            }
#endif
            break;
        case WLAN_VAP_MODE_WDS:
        case WLAN_VAP_MODE_MONITOER:
        case WLAN_VAP_HW_TEST:
            break;
        default:
            OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{mac_vap_init_etc::invalid vap mode[%d].}", pst_vap->en_vap_mode);

            return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 申请MIB内存空间，配置VAP没有MIB */
    temp_flag = ((pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) ||
        (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) || (pst_vap->en_vap_mode == WLAN_VAP_MODE_WDS));
    if (temp_flag) {
        pst_vap->pst_mib_info = mac_res_get_mib_info_etc(pst_vap->uc_vap_id);
        if (pst_vap->pst_mib_info == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY,
                "{mac_vap_init_etc::pst_mib_info alloc null, size[%d].}", OAL_SIZEOF(wlan_mib_ieee802dot11_stru));
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        pst_mib_info = pst_vap->pst_mib_info;
        memset_s(pst_mib_info, OAL_SIZEOF(wlan_mib_ieee802dot11_stru), 0, OAL_SIZEOF(wlan_mib_ieee802dot11_stru));

        /* 设置mac地址 */
        puc_addr = mac_mib_get_StationID(pst_vap);
        oal_set_mac_addr(puc_addr, pst_mac_device->auc_hw_addr);
        puc_addr[WLAN_MAC_ADDR_LEN - 1] += uc_vap_id;

#ifdef _PRE_WLAN_FEATURE_11AX
        mac_vap_11ax_cap_init(pst_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_MBO
        pst_mac_vap_rom = (mac_vap_rom_stru *)pst_vap->_rom;
        pst_mac_vap_rom->st_mbo_para_info.uc_mbo_enable = g_uc_mbo_switch;
#endif
        /* 初始化mib值 */
        mac_init_mib_etc(pst_vap);

        /* 根据协议规定 ，VHT rx_ampdu_factor应设置为7             */
        mac_mib_set_vht_max_rx_ampdu_factor(pst_vap, MAC_VHT_AMPDU_MAX_LEN_EXP); /* 2^(13+factor)-1字节 */

#if (defined(_PRE_PRODUCT_ID_HI110X_HOST)  && defined(_PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU))
        /* 从定制化获取是否开启AMSDU_AMPDU */
        mac_mib_set_AmsduPlusAmpduActive(pst_vap, g_st_tx_large_amsdu.uc_host_large_amsdu_en);
#endif
#ifdef _PRE_WLAN_FEATURE_VOWIFI
        if (pst_param->en_p2p_mode == WLAN_LEGACY_VAP_MODE) {
            mac_vap_vowifi_init(pst_vap);
            if (pst_vap->pst_vowifi_cfg_param != OAL_PTR_NULL) {
                pst_vap->pst_vowifi_cfg_param->en_vowifi_mode = VOWIFI_MODE_BUTT;
            }
        }
#endif /* _PRE_WLAN_FEATURE_VOWIFI */
#ifdef _PRE_WLAN_FEATURE_TXBF_HT
        mac_vap_init_txbf_ht_cap(pst_vap);
#endif
#ifdef _PRE_WLAN_FEATURE_1024QAM
        pst_vap->st_cap_flag.bit_1024qam = 1;
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
        pst_vap->st_cap_flag.bit_opmode = 1;
#endif

        /* sta以最大能力启用 */
        if (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
            /* 初始化sta协议模式为11ac */
            switch (pst_mac_device->en_protocol_cap) {
                case WLAN_PROTOCOL_CAP_LEGACY:
                case WLAN_PROTOCOL_CAP_HT:
                    pst_vap->en_protocol = WLAN_HT_MODE;
                    break;

                case WLAN_PROTOCOL_CAP_VHT:
                    pst_vap->en_protocol = WLAN_VHT_MODE;
                    break;
#ifdef _PRE_WLAN_FEATURE_11AX
                case WLAN_PROTOCOL_CAP_HE:
                    pst_vap->en_protocol = g_pst_mac_device_capability[0].en_11ax_switch ? WLAN_HE_MODE : WLAN_VHT_MODE;
                    break;
#endif

                default:
                    OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG,
                        "{mac_vap_init_etc::en_protocol_cap[%d] is not supportted.}", pst_mac_device->en_protocol_cap);
                    return OAL_ERR_CODE_CONFIG_UNSUPPORT;
            }

            switch (MAC_DEVICE_GET_CAP_BW(pst_mac_device)) {
                case WLAN_BW_CAP_20M:
                    MAC_VAP_GET_CAP_BW(pst_vap) = WLAN_BAND_WIDTH_20M;
                    break;

                case WLAN_BW_CAP_40M:
                    MAC_VAP_GET_CAP_BW(pst_vap) = WLAN_BAND_WIDTH_40MINUS;
                    break;

                case WLAN_BW_CAP_80M:
                    MAC_VAP_GET_CAP_BW(pst_vap) = WLAN_BAND_WIDTH_80PLUSMINUS;
                    break;

#ifdef _PRE_WLAN_FEATURE_160M
                case WLAN_BW_CAP_160M:
                    MAC_VAP_GET_CAP_BW(pst_vap) = WLAN_BAND_WIDTH_160PLUSPLUSMINUS;
                    break;
#endif

                default:
                    OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG,
                        "{mac_vap_init_etc::en_bandwidth_cap[%d] is not supportted.}",
                        MAC_DEVICE_GET_CAP_BW(pst_mac_device));
                    return OAL_ERR_CODE_CONFIG_UNSUPPORT;
            }

            switch (pst_mac_device->en_band_cap) {
                case WLAN_BAND_CAP_2G:
                    pst_vap->st_channel.en_band = WLAN_BAND_2G;
                    break;

                case WLAN_BAND_CAP_5G:
                case WLAN_BAND_CAP_2G_5G:
                    pst_vap->st_channel.en_band = WLAN_BAND_5G;
                    break;

                default:
                    OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG,
                        "{mac_vap_init_etc::en_band_cap[%d] is not supportted.}", pst_mac_device->en_band_cap);
                    return OAL_ERR_CODE_CONFIG_UNSUPPORT;
            }

            if (OAL_SUCC != mac_vap_init_by_protocol_etc(pst_vap, pst_vap->en_protocol)) {
                mac_vap_free_mib_etc(pst_vap);
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
            mac_vap_init_rates_etc(pst_vap);
        }
    }

    pst_vap->uc_init_flag = MAC_VAP_VAILD;

    return OAL_SUCC;
}

OAL_STATIC wlan_channel_bandwidth_enum_uint8 mac_sta_get_user_bw_from_he_cap_ie(
        mac_vap_stru *pst_mac_sta, mac_user_stru *pst_mac_user)
{
    wlan_channel_bandwidth_enum_uint8 en_he_cap_bw = WLAN_BAND_WIDTH_BUTT;

#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl_stru *pst_mac_he_hdl;

    pst_mac_he_hdl = MAC_USER_HE_HDL_STRU(pst_mac_user);
    if (OAL_TRUE == MAC_USER_IS_HE_USER(pst_mac_user) &&
        pst_mac_he_hdl->st_he_oper_ie.st_he_oper_param.bit_vht_operation_info_present) {
            en_he_cap_bw = mac_get_bandwith_from_center_freq_seg0_seg1(
                pst_mac_he_hdl->st_he_oper_ie.st_vht_operation_info.uc_channel_width,
                pst_mac_sta->st_channel.uc_chan_number,
                pst_mac_he_hdl->st_he_oper_ie.st_vht_operation_info.uc_center_freq_seg0,
                pst_mac_he_hdl->st_he_oper_ie.st_vht_operation_info.uc_center_freq_seg1);
    }
#endif

    return en_he_cap_bw;

}

OAL_STATIC wlan_channel_bandwidth_enum_uint8 mac_sta_get_user_bw_from_non_he_cap_ie(mac_vap_stru *pst_mac_sta,
    mac_user_stru *pst_mac_user)
{
    mac_user_ht_hdl_stru *pst_mac_ht_hdl;
    mac_vht_hdl_stru *pst_mac_vht_hdl;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth_ap = WLAN_BAND_WIDTH_20M;
    oal_bool_enum_uint8 en_ignore_non_he_bw_cap;

    /* 获取HT和VHT结构体指针 */
    pst_mac_ht_hdl = &(pst_mac_user->st_ht_hdl);
    pst_mac_vht_hdl = &(pst_mac_user->st_vht_hdl);
    en_ignore_non_he_bw_cap = mac_sta_get_he_ignore_non_he_cap(pst_mac_sta);

    if (en_ignore_non_he_bw_cap) {
        return WLAN_BAND_WIDTH_BUTT;
    }

    if (pst_mac_vht_hdl->en_vht_capable == OAL_TRUE) {
        en_bandwidth_ap = mac_get_bandwith_from_center_freq_seg0_seg1(
                        pst_mac_vht_hdl->en_channel_width,
                        pst_mac_sta->st_channel.uc_chan_number,
                        pst_mac_vht_hdl->uc_channel_center_freq_seg0,
                        pst_mac_vht_hdl->uc_channel_center_freq_seg1);
    }

    /* ht 20/40M带宽的处理 */
    if ((pst_mac_ht_hdl->en_ht_capable == OAL_TRUE) &&
            (en_bandwidth_ap <= WLAN_BAND_WIDTH_40MINUS) &&
            (OAL_TRUE == mac_mib_get_FortyMHzOperationImplemented(pst_mac_sta))) {
        /* 更新带宽模式 */
        en_bandwidth_ap = mac_get_bandwidth_from_sco(pst_mac_ht_hdl->bit_secondary_chan_offset);
    }

    return en_bandwidth_ap;
}


oal_uint32 mac_vap_check_ap_usr_opern_bandwidth(mac_vap_stru *pst_mac_sta, mac_user_stru *pst_mac_user)
{
    wlan_channel_bandwidth_enum_uint8 en_sta_current_bw;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth_ap;
    wlan_channel_bandwidth_enum_uint8 en_he_bandwidth_ap;
    wlan_channel_bandwidth_enum_uint8 en_non_he_bandwidth_ap;
    oal_bool_enum_uint8 en_ignore_non_he_bw_cap;
    oal_uint32 ul_change = 0;

    en_sta_current_bw = MAC_VAP_GET_CAP_BW(pst_mac_sta);
    en_ignore_non_he_bw_cap = mac_sta_get_he_ignore_non_he_cap(pst_mac_sta);

    /* 获取he cap 中宣称的bw */
    en_he_bandwidth_ap = mac_sta_get_user_bw_from_he_cap_ie(pst_mac_sta, pst_mac_user);
    if (en_ignore_non_he_bw_cap && en_he_bandwidth_ap == WLAN_BAND_WIDTH_BUTT) {
        return ul_change;
    }

    /* 获取non he cap 中宣称的bw */
    en_non_he_bandwidth_ap = mac_sta_get_user_bw_from_non_he_cap_ie(pst_mac_sta, pst_mac_user);

    en_bandwidth_ap = OAL_MIN(en_he_bandwidth_ap, en_non_he_bandwidth_ap);

    if ((en_sta_current_bw != en_bandwidth_ap) &&
        (WLAN_BANDWIDTH_TO_BW_CAP(en_bandwidth_ap) <= mac_mib_get_dot11VapMaxBandWidth(pst_mac_sta))) {
        /* 防止每个beacon都去检查en_bandwidth_ap */
        if (OAL_FALSE == mac_regdomain_channel_is_support_bw(en_bandwidth_ap, pst_mac_sta->st_channel.uc_chan_number)) {
            en_bandwidth_ap = WLAN_BAND_WIDTH_20M;
            if (en_sta_current_bw != en_bandwidth_ap) { /* 防止刷屏打印 */
                OAM_WARNING_LOG2(pst_mac_sta->uc_vap_id, OAM_SF_ASSOC,
                    "{mac_vap_check_ap_usr_opern_bandwidth::channel[%d] is not support bw[%d],set 20MHz}",
                    pst_mac_sta->st_channel.uc_chan_number, en_bandwidth_ap);
            }
        }

        if (en_sta_current_bw != en_bandwidth_ap) {
            ul_change = MAC_BW_DIFF_AP_USER;
            OAM_WARNING_LOG3(pst_mac_sta->uc_vap_id, OAM_SF_FRAME_FILTER,
                "{mac_vap_check_ap_usr_opern_bandwidth:: en_sta_current_bw[%d], en_bandwidth_ap[%d], ul_change[%d]}",
                en_sta_current_bw, en_bandwidth_ap, ul_change);
        }
    }

    return ul_change;
}

#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1103_DEV)

oal_uint8 mac_vap_get_ap_usr_opern_bandwidth(mac_vap_stru *pst_mac_sta, mac_user_stru *pst_mac_user)
{
    mac_user_ht_hdl_stru *pst_mac_ht_hdl = OAL_PTR_NULL;
    mac_vht_hdl_stru *pst_mac_vht_hdl = OAL_PTR_NULL;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth_ap = WLAN_BAND_WIDTH_20M;
    wlan_channel_bandwidth_enum_uint8 en_sta_new_bandwidth;
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl_stru *pst_mac_he_hdl = OAL_PTR_NULL;
#endif
    oal_uint8 uc_channel_center_freq_seg;

    /* 更新用户的带宽能力cap */
    mac_user_update_ap_bandwidth_cap(pst_mac_user);
    /* 获取HT和VHT结构体指针 */
    pst_mac_ht_hdl = &(pst_mac_user->st_ht_hdl);
    pst_mac_vht_hdl = &(pst_mac_user->st_vht_hdl);
#ifdef _PRE_WLAN_FEATURE_11AX
    pst_mac_he_hdl = MAC_USER_HE_HDL_STRU(pst_mac_user);
#endif

    /******************* VHT BSS operating channel width ****************
     -----------------------------------------------------------------
     |HT Oper Chl Width |VHT Oper Chl Width |BSS Oper Chl Width|
     -----------------------------------------------------------------
     |       0          |        0          |    20MHZ         |
     -----------------------------------------------------------------
     |       1          |        0          |    40MHZ         |
     -----------------------------------------------------------------
     |       1          |        1          |    80MHZ         |
     -----------------------------------------------------------------
     |       1          |        2          |    160MHZ        |
     -----------------------------------------------------------------
     |       1          |        3          |    80+80MHZ      |
     -----------------------------------------------------------------
    **********************************************************************/
    if ((pst_mac_vht_hdl->bit_supported_channel_width == 0) &&
        (pst_mac_vht_hdl->bit_extend_nss_bw_supp != 0)) {
        uc_channel_center_freq_seg = (pst_mac_user->en_user_max_cap_nss == WLAN_SINGLE_NSS) ?
            0 : pst_mac_ht_hdl->uc_chan_center_freq_seg2;
    } else {
        uc_channel_center_freq_seg = pst_mac_vht_hdl->uc_channel_center_freq_seg1;
    }
    if (pst_mac_vht_hdl->en_vht_capable == OAL_TRUE) {
        en_bandwidth_ap = mac_get_bandwith_from_center_freq_seg0_seg1(pst_mac_vht_hdl->en_channel_width,
                                                                      pst_mac_sta->st_channel.uc_chan_number,
                                                                      pst_mac_vht_hdl->uc_channel_center_freq_seg0,
                                                                      uc_channel_center_freq_seg);
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if ((OAL_TRUE == MAC_USER_IS_HE_USER(pst_mac_user)) &&
        (pst_mac_he_hdl->st_he_oper_ie.st_he_oper_param.bit_vht_operation_info_present == OAL_TRUE)) {
        en_bandwidth_ap = mac_get_bandwith_from_center_freq_seg0_seg1(
            pst_mac_he_hdl->st_he_oper_ie.st_vht_operation_info.uc_channel_width,
            pst_mac_sta->st_channel.uc_chan_number,
            pst_mac_he_hdl->st_he_oper_ie.st_vht_operation_info.uc_center_freq_seg0,
            pst_mac_he_hdl->st_he_oper_ie.st_vht_operation_info.uc_center_freq_seg1);
    }
#endif

    /* ht 20/40M带宽的处理 */
    if ((pst_mac_ht_hdl->en_ht_capable == OAL_TRUE) && (en_bandwidth_ap <= WLAN_BAND_WIDTH_40MINUS) &&
        (OAL_TRUE == mac_mib_get_FortyMHzOperationImplemented(pst_mac_sta))) {
        /* 更新带宽模式 */
        en_bandwidth_ap = mac_get_bandwidth_from_sco(pst_mac_ht_hdl->bit_secondary_chan_offset);
    }

    /* 带宽不能超过mac device的最大能力 */
    en_sta_new_bandwidth = mac_vap_get_bandwith(mac_mib_get_dot11VapMaxBandWidth(pst_mac_sta), en_bandwidth_ap);
    if (mac_regdomain_channel_is_support_bw(en_sta_new_bandwidth,
        pst_mac_sta->st_channel.uc_chan_number) == OAL_FALSE) {
        OAM_WARNING_LOG2(pst_mac_sta->uc_vap_id, OAM_SF_ASSOC,
                         "{mac_vap_get_ap_usr_opern_bandwidth::channel[%d] is not support bw[%d],set 20MHz}",
                         pst_mac_sta->st_channel.uc_chan_number, en_sta_new_bandwidth);
        en_sta_new_bandwidth = WLAN_BAND_WIDTH_20M;
    }

    return en_sta_new_bandwidth;
}


oal_uint32 mac_device_find_up_vap_etc_ram(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    oal_uint8       uc_vap_idx;
    mac_vap_stru   *pst_mac_vap = OAL_PTR_NULL;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_UNLIKELY(pst_mac_vap == OAL_PTR_NULL)) {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN, "vap is null! vap id is %d", pst_mac_device->auc_vap_id[uc_vap_idx]);

            *ppst_mac_vap = OAL_PTR_NULL;

            return OAL_ERR_CODE_PTR_NULL;
        }

        if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP || pst_mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE
#ifdef _PRE_WLAN_FEATURE_P2P
            || (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN && pst_mac_vap->us_user_nums > 0)
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
            || (pst_mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING)
#endif  // _PRE_WLAN_FEATURE_ROAM
            ) {
            *ppst_mac_vap = pst_mac_vap;

            return OAL_SUCC;
        }
    }

    *ppst_mac_vap = OAL_PTR_NULL;

    return OAL_FAIL;
}

#endif


oal_uint8 mac_vap_set_bw_check(mac_vap_stru *pst_mac_sta,
                               wlan_channel_bandwidth_enum_uint8 en_sta_new_bandwidth)
{
    wlan_channel_bandwidth_enum_uint8 en_band_with_sta_old;
    oal_uint8 uc_change;

    en_band_with_sta_old = MAC_VAP_GET_CAP_BW(pst_mac_sta);
    MAC_VAP_GET_CAP_BW(pst_mac_sta) = en_sta_new_bandwidth;

    /* 判断是否需要通知硬件切换带宽 */
    uc_change = (en_band_with_sta_old == en_sta_new_bandwidth) ? MAC_NO_CHANGE : MAC_BW_CHANGE;

    OAM_WARNING_LOG2(pst_mac_sta->uc_vap_id, OAM_SF_ASSOC, "mac_vap_set_bw_check::bandwidth[%d]->[%d].",
                     en_band_with_sta_old, en_sta_new_bandwidth);

    return uc_change;
}

#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE

oal_void mac_tx_qos_enhance_list_init(mac_vap_stru *pst_mac_vap)
{
    /* 入参检查 */
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_QOS, "{mac_tx_qos_enhance_list_init::null pst_mac_vap}");
        return;
    }

    /* 初始化链表 */
    memset_s(&(pst_mac_vap->st_qos_enhance), OAL_SIZEOF(mac_qos_enhance_stru), 0, OAL_SIZEOF(mac_qos_enhance_stru));
    oal_spin_lock_init(&(pst_mac_vap->st_qos_enhance.st_lock));
    oal_dlist_init_head(&(pst_mac_vap->st_qos_enhance.st_list_head));
    pst_mac_vap->st_qos_enhance.en_qos_enhance_enable = OAL_TRUE;
}


oal_void mac_tx_clean_qos_enhance_list(mac_vap_stru *pst_mac_vap)
{
    mac_qos_enhance_stru *pst_qos_enhance;
    mac_qos_enhance_sta_stru *pst_qos_enhance_sta = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_sta_list_entry;
    oal_dlist_head_stru *pst_sta_list_entry_temp;

    /* 入参检查 */
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_QOS, "{mac_tx_qos_enhance_list_init::null pst_mac_vap}");
        return;
    }

    pst_qos_enhance = &(pst_mac_vap->st_qos_enhance);

    if (pst_qos_enhance->uc_qos_enhance_sta_count == 0) {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_QOS,
            "{mac_tx_qos_enhance_list_init::no sta in qos list while clean.}");
        return;
    }

    /* 对链表操作前加锁 */
    oal_spin_lock(&(pst_qos_enhance->st_lock));

    /* 遍历链表 */
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_sta_list_entry, pst_sta_list_entry_temp, &(pst_qos_enhance->st_list_head))
    {
        pst_qos_enhance_sta = OAL_DLIST_GET_ENTRY(pst_sta_list_entry, mac_qos_enhance_sta_stru, st_qos_enhance_entry);
        oal_dlist_delete_entry(&(pst_qos_enhance_sta->st_qos_enhance_entry));
        OAL_MEM_FREE(pst_qos_enhance_sta, OAL_TRUE);
    }

    /* 释放锁 */
    oal_spin_unlock(&(pst_qos_enhance->st_lock));

    pst_mac_vap->st_qos_enhance.uc_qos_enhance_sta_count = 0;
}


mac_qos_enhance_sta_stru *mac_tx_find_qos_enhance_list(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_sta_member_addr)
{
    mac_qos_enhance_sta_stru *pst_qos_enhance_sta = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_sta_list_entry;
    oal_dlist_head_stru *pst_sta_list_entry_temp;
    mac_qos_enhance_stru *pst_qos_enhance;

    /* 入参检查 */
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_QOS, "{mac_tx_find_qos_enhance_list::null pst_mac_vap}");
        return OAL_PTR_NULL;
    }

    pst_qos_enhance = &(pst_mac_vap->st_qos_enhance);

    if (pst_qos_enhance->uc_qos_enhance_sta_count == 0) {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_QOS,
            "{mac_tx_qos_enhance_list_init::no sta in qos list while find.}");
        return OAL_PTR_NULL;
    }

    /* 遍历链表 */
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_sta_list_entry, pst_sta_list_entry_temp, &(pst_qos_enhance->st_list_head))
    {
        pst_qos_enhance_sta = OAL_DLIST_GET_ENTRY(pst_sta_list_entry, mac_qos_enhance_sta_stru, st_qos_enhance_entry);
        if (!oal_compare_mac_addr(puc_sta_member_addr, pst_qos_enhance_sta->auc_qos_enhance_mac)) {
            return pst_qos_enhance_sta;
        }
    }

    return OAL_PTR_NULL;
}
#endif

OAL_STATIC oal_void mac_vap_csa_go_support_set(mac_vap_stru *pst_mac_vap)
{
    mac_user_stru *pst_mac_user = OAL_PTR_NULL;
    oal_uint8 uc_hash_user_index;
    oal_dlist_head_stru *pst_entry = OAL_PTR_NULL;

    if (pst_mac_vap->us_user_nums > 1) {
        pst_mac_vap->bit_vap_support_csa = OAL_FALSE;
        return;
    }

    for (uc_hash_user_index = 0; uc_hash_user_index < MAC_VAP_USER_HASH_MAX_VALUE; uc_hash_user_index++) {
        OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_mac_vap->ast_user_hash[uc_hash_user_index]))
        {
            pst_mac_user = (mac_user_stru *)OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_hash_dlist);
            if (pst_mac_user->st_cap_info.bit_p2p_support_csa) {
                pst_mac_vap->bit_vap_support_csa = OAL_TRUE;
            } else {
                pst_mac_vap->bit_vap_support_csa = OAL_FALSE;
                return;
            }
        }
    }
}


oal_void mac_vap_csa_support_set(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_cap)
{
    pst_mac_vap->bit_vap_support_csa = OAL_FALSE;

    if (IS_LEGACY_VAP(pst_mac_vap)) {
        pst_mac_vap->bit_vap_support_csa = OAL_TRUE;
        return;
    }

    if (IS_P2P_CL(pst_mac_vap)) {
        pst_mac_vap->bit_vap_support_csa = en_cap;
        return;
    }
    if (IS_P2P_GO(pst_mac_vap)) {
        mac_vap_csa_go_support_set(pst_mac_vap);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_csa_support_set::go_support_csa=[%d]!}",
            pst_mac_vap->bit_vap_support_csa);
    }
}


oal_bool_enum_uint8 mac_vap_go_can_not_in_160M_check(mac_vap_stru *p_mac_vap, oal_uint8 vap_channel)
{
    mac_vap_stru *p_another_mac_vap;

    p_another_mac_vap = mac_vap_find_another_up_vap_by_mac_vap(p_mac_vap);
    if (p_another_mac_vap == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    if (p_another_mac_vap->st_channel.uc_chan_number != vap_channel) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*lint -e19*/
oal_module_symbol(mac_vap_get_ap_usr_opern_bandwidth);
oal_module_symbol(mac_vap_set_bw_check);
oal_module_symbol(mac_vap_init_etc);
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_VAP)
oal_module_symbol(mac_blacklist_free_pointer);
#endif
#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
oal_module_symbol(mac_tx_find_qos_enhance_list);
#endif /*lint +e19*/


