

/* 1 头文件包含 */
#include "mac_mib.h"
#include "mac_vap.h"
#include "hmac_resource.h"
#include "wlan_chip_i.h"
#include "mac_ftm.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_MIB_C


#ifdef _PRE_WLAN_FEATURE_11AX
void mac_vap_init_mib_11ax_1103(mac_vap_stru *mac_vap, uint32_t nss_num)
{
    if (mac_vap == NULL) {
        return;
    }

    mac_mib_set_he_BeamformeeSTSBelow80Mhz(mac_vap, HE_BFEE_NTX_SUPP_STS_CAP_FOUR);
    mac_mib_set_he_BeamformeeSTSOver80Mhz(mac_vap, HE_BFEE_NTX_SUPP_STS_CAP_FOUR);
    mac_mib_set_HENumberSoundingDimensionsBelow80Mhz(mac_vap, 0);
    mac_mib_set_HENumberSoundingDimensionsOver80Mhz(mac_vap, 0);
    mac_mib_set_HENg16SUFeedback(mac_vap, OAL_FALSE);
    mac_mib_set_HENg16MUFeedback(mac_vap, OAL_FALSE);
    mac_mib_set_HECodebook75MUFeedback(mac_vap, OAL_FALSE);
    mac_mib_set_he_TriggeredSUBeamformingFeedback(mac_vap, OAL_FALSE);
    mac_mib_set_he_TriggeredCQIFeedback(mac_vap, OAL_FALSE);
}

void mac_vap_init_mib_11ax_1105(mac_vap_stru *mac_vap, uint32_t nss_num)
{
    if (mac_vap == NULL) {
        return;
    }

    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        mac_mib_set_he_BeamformeeSTSBelow80Mhz(mac_vap, HE_BFEE_NTX_SUPP_STS_CAP_EIGHT);
        mac_mib_set_he_BeamformeeSTSOver80Mhz(mac_vap, HE_BFEE_NTX_SUPP_STS_CAP_EIGHT);
        mac_mib_set_HENumberSoundingDimensionsBelow80Mhz(mac_vap, nss_num);
        mac_mib_set_HENumberSoundingDimensionsOver80Mhz(mac_vap, nss_num);
        mac_mib_set_HENg16SUFeedback(mac_vap, OAL_FALSE);
        mac_mib_set_HENg16MUFeedback(mac_vap, OAL_FALSE);
        mac_mib_set_HECodebook75MUFeedback(mac_vap, OAL_TRUE);
        mac_mib_set_he_TriggeredSUBeamformingFeedback(mac_vap, OAL_TRUE);
        mac_mib_set_he_TriggeredCQIFeedback(mac_vap, OAL_TRUE);
    } else if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        mac_mib_set_he_BeamformeeSTSBelow80Mhz(mac_vap, HE_BFEE_NTX_SUPP_STS_CAP_EIGHT);
        mac_mib_set_he_BeamformeeSTSOver80Mhz(mac_vap, HE_BFEE_NTX_SUPP_STS_CAP_EIGHT);
        mac_mib_set_HENumberSoundingDimensionsBelow80Mhz(mac_vap, nss_num);
        mac_mib_set_HENumberSoundingDimensionsOver80Mhz(mac_vap, nss_num);
        mac_mib_set_HENg16SUFeedback(mac_vap, OAL_TRUE);
        mac_mib_set_HENg16MUFeedback(mac_vap, OAL_FALSE);
        mac_mib_set_HECodebook75MUFeedback(mac_vap, OAL_FALSE);
        mac_mib_set_he_TriggeredSUBeamformingFeedback(mac_vap, OAL_FALSE);
        mac_mib_set_he_TriggeredCQIFeedback(mac_vap, OAL_FALSE);
    }
}

void mac_vap_init_mib_11ax_1106(mac_vap_stru *mac_vap, uint32_t nss_num)
{
    if (mac_vap == NULL) {
        return;
    }

    mac_mib_set_he_BeamformeeSTSBelow80Mhz(mac_vap, HE_BFEE_NTX_SUPP_STS_CAP_FOUR);
    mac_mib_set_he_BeamformeeSTSOver80Mhz(mac_vap, HE_BFEE_NTX_SUPP_STS_CAP_FOUR);
    mac_mib_set_HENumberSoundingDimensionsBelow80Mhz(mac_vap, 0);
    mac_mib_set_HENumberSoundingDimensionsOver80Mhz(mac_vap, 0);
    mac_mib_set_HENg16SUFeedback(mac_vap, OAL_FALSE);
    mac_mib_set_HENg16MUFeedback(mac_vap, OAL_FALSE);
    mac_mib_set_HECodebook75MUFeedback(mac_vap, OAL_FALSE);
    mac_mib_set_he_TriggeredSUBeamformingFeedback(mac_vap, OAL_FALSE);
    mac_mib_set_he_TriggeredCQIFeedback(mac_vap, OAL_FALSE);
}


void mac_vap_init_mib_11ax(mac_vap_stru *pst_mac_vap)
{
    mac_device_stru *pst_mac_dev = NULL;
    if (g_wlan_spec_cfg->feature_11ax_is_open != OAL_TRUE) {
        return;
    }

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
            "{mac_vap_init_mib_11ax::pst_mac_dev[%d] null.}", pst_mac_vap->uc_device_id);

        return;
    }

    /* MAC Capabilities Info */
    mac_mib_set_HEOptionImplemented(pst_mac_vap, OAL_TRUE);
    mac_mib_set_OMIOptionImplemented(pst_mac_vap, pst_mac_vap->bit_rom_custom_switch);
    mac_mib_set_he_TWTOptionActivated(pst_mac_vap, OAL_TRUE); /* 第一阶段暂不支持 */
    mac_mib_set_he_OperatingModeIndication(pst_mac_vap, OAL_TRUE);
    mac_mib_set_he_TriggerMacPaddingDuration(pst_mac_vap, MAC_TRIGGER_FRAME_PADDING_DURATION16us); /* 16us */
    mac_mib_set_he_MaxAMPDULengthExponent(pst_mac_vap, MAC_HE_MAX_AMPDU_LEN_EXP_0); /* 2^(20+factor)-1字节 */
    mac_mib_set_he_MultiBSSIDImplemented(pst_mac_vap, OAL_TRUE);
    mac_mib_set_he_BSRSupport(pst_mac_vap, OAL_TRUE);

    if ((g_wlan_spec_cfg->feature_11ax_uora_is_supported == OAL_TRUE) && IS_LEGACY_STA(pst_mac_vap)) {
        mac_mib_set_he_OFDMARandomAccess(pst_mac_vap, OAL_TRUE);
    }

    /* PHY Capabilities Info */
    if (WLAN_BAND_CAP_2G_5G == pst_mac_dev->en_band_cap) {
        mac_mib_set_he_DualBandSupport(pst_mac_vap, OAL_TRUE); /* 支持双频 */
    }
    mac_mib_set_he_LDPCCodingInPayload(pst_mac_vap, MAC_DEVICE_GET_CAP_LDPC(pst_mac_dev)); /* 支持LDPC编码 */
    mac_mib_set_he_SUBeamformer(pst_mac_vap, MAC_DEVICE_GET_CAP_SUBFER(pst_mac_dev));
    mac_mib_set_he_SUBeamformee(pst_mac_vap, MAC_DEVICE_GET_CAP_SUBFEE(pst_mac_dev));
    mac_mib_set_he_MUBeamformer(pst_mac_vap, MAC_DEVICE_GET_CAP_MUBFER(pst_mac_dev));
    /* 以下mib项只有host使用 */
    wlan_chip_mac_vap_init_mib_11ax(pst_mac_vap, MAC_DEVICE_GET_NSS_NUM(pst_mac_dev));  /* 针对不同芯片初始化11ax能力 */

    mac_mib_set_HECodebook42SUFeedback(pst_mac_vap, OAL_TRUE);
    mac_mib_set_he_STBCTxBelow80M(pst_mac_vap, MAC_DEVICE_GET_CAP_TXSTBC(pst_mac_dev));
    mac_mib_set_he_STBCRxBelow80M(pst_mac_vap, MAC_DEVICE_GET_CAP_RXSTBC(pst_mac_dev));
    mac_mib_set_PPEThresholdsRequired(pst_mac_vap, OAL_TRUE);
    mac_mib_set_HESUPPDUwith1xHELTFand0point8GIlmplemented(pst_mac_vap, OAL_TRUE);

    /* Tx Rx MCS NSS */
    mac_mib_set_he_HighestNSS(pst_mac_vap, MAC_DEVICE_GET_NSS_NUM(pst_mac_dev));

    /* Multi BSSID */
    mac_mib_set_he_MultiBSSIDActived(pst_mac_vap, pst_mac_vap->bit_multi_bssid_custom_switch);

    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        mac_mib_set_he_MultiBSSIDActived(pst_mac_vap, OAL_FALSE);
        mac_mib_set_he_TWTOptionActivated(pst_mac_vap, OAL_FALSE);
        mac_mib_set_he_TriggerMacPaddingDuration(pst_mac_vap, MAC_TRIGGER_FRAME_PADDING_DURATION0us);
        mac_mib_set_he_BSRSupport(pst_mac_vap, OAL_FALSE);
        mac_mib_set_he_SUBeamformee(pst_mac_vap, OAL_TRUE);
        mac_mib_set_he_MUBeamformer(pst_mac_vap, OAL_FALSE);
    }
}

#endif

#if  defined(_PRE_WLAN_FEATURE_11V_ENABLE)
/* 默认支持11v 如需关闭请上层调用接口 */
OAL_STATIC void mac_vap_init_mib_11v(mac_vap_stru *pst_vap)
{
    /* en_dot11MgmtOptionBSSTransitionActivated 初始化时为TRUE,由定制化或命令打开or关闭 */
    mac_mib_set_MgmtOptionBSSTransitionActivated(pst_vap, OAL_TRUE);
    mac_mib_set_MgmtOptionBSSTransitionImplemented(pst_vap, OAL_TRUE);
    mac_mib_set_WirelessManagementImplemented(pst_vap, OAL_TRUE);
}
#endif


void mac_init_mib_extend(mac_vap_stru *pst_mac_vap)
{
#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    /* 11k */
    mac_vap_init_mib_11v(pst_mac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    /* txopps 初始化关闭状态 */
    mac_mib_set_txopps(pst_mac_vap, OAL_FALSE);
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
    mac_ftm_mib_init(pst_mac_vap);
#endif

    mac_mib_set_dot11VHTExtendedNSSBWCapable(pst_mac_vap, OAL_TRUE);
}


uint32_t mac_mib_set_station_id(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_cfg_staion_id_param_stru *pst_param;

    pst_param = (mac_cfg_staion_id_param_stru *)puc_param;

    oal_set_mac_addr(mac_mib_get_StationID(pst_mac_vap), pst_param->auc_station_id);

    return OAL_SUCC;
}


uint32_t mac_mib_set_bss_type(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    int32_t l_value;

    l_value = *((int32_t *)puc_param);

    mac_mib_set_DesiredBSSType(pst_mac_vap, (uint8_t)l_value);

    return OAL_SUCC;
}


uint32_t mac_mib_set_ssid(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_cfg_ssid_param_stru    *pst_param;
    uint8_t                   uc_ssid_len;
    uint8_t                  *puc_mib_ssid = NULL;
    int32_t                   l_ret;

    pst_param = (mac_cfg_ssid_param_stru *)puc_param;
    uc_ssid_len = pst_param->uc_ssid_len; /* 长度不包括字符串结尾'\0' */

    if (uc_ssid_len > WLAN_SSID_MAX_LEN - 1) {
        uc_ssid_len = WLAN_SSID_MAX_LEN - 1;
    }

    puc_mib_ssid = mac_mib_get_DesiredSSID(pst_mac_vap);

    l_ret = memcpy_s(puc_mib_ssid, WLAN_SSID_MAX_LEN, pst_param->ac_ssid, uc_ssid_len);
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_mib_set_ssid::memcpy fail!");
        return OAL_FAIL;
    }

    puc_mib_ssid[uc_ssid_len] = '\0';

    return OAL_SUCC;
}


uint32_t mac_mib_get_ssid(mac_vap_stru *pst_mac_vap, uint8_t *puc_len, uint8_t *puc_param)
{
    mac_cfg_ssid_param_stru *pst_param;
    uint8_t                uc_ssid_len;
    uint8_t               *puc_mib_ssid;
    int32_t                l_ret;

    puc_mib_ssid = mac_mib_get_DesiredSSID(pst_mac_vap);
    uc_ssid_len = (uint8_t)OAL_STRLEN((int8_t *)puc_mib_ssid);

    pst_param = (mac_cfg_ssid_param_stru *)puc_param;

    pst_param->uc_ssid_len = uc_ssid_len;
    l_ret = memcpy_s(pst_param->ac_ssid, WLAN_SSID_MAX_LEN, puc_mib_ssid, uc_ssid_len);
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_mib_get_ssid::memcpy fail!");
        return OAL_FAIL;
    }

    *puc_len = sizeof(mac_cfg_ssid_param_stru);

    return OAL_SUCC;
}


uint32_t mac_mib_set_beacon_period(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    uint32_t value;

    value = *((uint32_t *)puc_param);

    mac_mib_set_BeaconPeriod(pst_mac_vap, value);

    return OAL_SUCC;
}


uint32_t mac_mib_set_dtim_period(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    int32_t l_value;

    l_value = *((int32_t *)puc_param);

    mac_mib_set_dot11dtimperiod(pst_mac_vap, (uint32_t)l_value);

    return OAL_SUCC;
}


uint32_t mac_mib_set_shpreamble(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    int32_t l_value;

    l_value = *((int32_t *)puc_param);

    if (l_value != 0) {
        mac_mib_set_ShortPreambleOptionImplemented(pst_mac_vap, OAL_TRUE);
    } else {
        mac_mib_set_ShortPreambleOptionImplemented(pst_mac_vap, OAL_FALSE);
    }

    return OAL_SUCC;
}


uint32_t mac_mib_get_bss_type(mac_vap_stru *pst_mac_vap, uint8_t *puc_len, uint8_t *puc_param)
{
    *((int32_t *)puc_param) = mac_mib_get_DesiredBSSType(pst_mac_vap);

    *puc_len = sizeof(int32_t);

    return OAL_SUCC;
}


uint32_t mac_mib_get_beacon_period(mac_vap_stru *pst_mac_vap, uint8_t *puc_len, uint8_t *puc_param)
{
    *((uint32_t *)puc_param) = mac_mib_get_BeaconPeriod(pst_mac_vap);

    *puc_len = sizeof(uint32_t);

    return OAL_SUCC;
}



uint32_t mac_mib_get_dtim_period(mac_vap_stru *pst_mac_vap, uint8_t *puc_len, uint8_t *puc_param)
{
    *((uint32_t *)puc_param) = mac_mib_get_dot11dtimperiod(pst_mac_vap);

    *puc_len = sizeof(uint32_t);

    return OAL_SUCC;
}


uint32_t mac_mib_get_shpreamble(mac_vap_stru *pst_mac_vap, uint8_t *puc_len, uint8_t *puc_param)
{
    int32_t l_value;

    l_value = mac_mib_get_ShortPreambleOptionImplemented(pst_mac_vap);

    *((int32_t *)puc_param) = l_value;

    *puc_len = sizeof(l_value);

    return OAL_SUCC;
}


void mac_vap_init_mib_11n(mac_vap_stru *pst_mac_vap)
{
    wlan_mib_ieee802dot11_stru    *pst_mib_info = NULL;
    mac_device_stru               *pst_dev;

    pst_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_dev == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_init_mib_11n::pst_dev null.}");
        return;
    }

    pst_mib_info = pst_mac_vap->pst_mib_info;

    mac_mib_set_HighThroughputOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_LDPCCodingOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_LDPC(pst_dev));
    mac_mib_set_TxSTBCOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_TXSTBC(pst_dev));

    /* LDPC 和 STBC Activated能力初始化为device的能力，用于STA的协议协商，到关联请求时再更新该mib值  */
    mac_mib_set_LDPCCodingOptionActivated(pst_mac_vap, MAC_DEVICE_GET_CAP_LDPC(pst_dev));
    mac_mib_set_TxSTBCOptionActivated(pst_mac_vap, HT_TX_STBC_DEFAULT_VALUE);

    mac_mib_set_2GFortyMHzOperationImplemented(             \
        pst_mac_vap, (oal_bool_enum_uint8)(!pst_mac_vap->st_cap_flag.bit_disable_2ght40));
    mac_mib_set_5GFortyMHzOperationImplemented(pst_mac_vap, OAL_TRUE);

#ifdef _PRE_WLAN_FEATURE_SMPS
    /* 开启SMPS宏,能力需要按照mac device能力来刷，后续m2s会动态刷新 */
    mac_mib_set_smps(pst_mac_vap, MAC_DEVICE_GET_MODE_SMPS(pst_dev));
#endif

    mac_mib_set_HTGreenfieldOptionImplemented(pst_mac_vap, HT_GREEN_FILED_DEFAULT_VALUE);
    mac_mib_set_ShortGIOptionInTwentyImplemented(pst_mac_vap, OAL_TRUE);
    mac_mib_set_2GShortGIOptionInFortyImplemented(             \
        pst_mac_vap, (oal_bool_enum_uint8)(!pst_mac_vap->st_cap_flag.bit_disable_2ght40));
    mac_mib_set_5GShortGIOptionInFortyImplemented(pst_mac_vap, OAL_TRUE);

    mac_mib_set_RxSTBCOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_RXSTBC(pst_dev));
    pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MaxAMSDULength = 0;

    mac_mib_set_lsig_txop_protection(pst_mac_vap, OAL_FALSE);
    mac_mib_set_max_ampdu_len_exponent(pst_mac_vap, 3); /* 3 MaxRxAMPDUFactor */
    mac_mib_set_min_mpdu_start_spacing(pst_mac_vap, 5); /* 5 MinimumMPDUStartSpacing */
    mac_mib_set_pco_option_implemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_transition_time(pst_mac_vap, 3); /* 3 TransitionTime */
    mac_mib_set_mcs_fdbk(pst_mac_vap, (wlan_mib_mcs_feedback_opt_implt_enum)OAL_FALSE);
    mac_mib_set_htc_sup(pst_mac_vap, OAL_FALSE);
    mac_mib_set_rd_rsp(pst_mac_vap, OAL_FALSE);

    mac_mib_set_ReceiveNDPOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_TransmitNDPOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_ImplicitTransmitBeamformingOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_CalibrationOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_ExplicitCSITransmitBeamformingOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_ExplicitNonCompressedBeamformingMatrixOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_ExplicitTransmitBeamformingCSIFeedbackOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_ExplicitNonCompressedBeamformingFeedbackOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_NumberBeamFormingCSISupportAntenna(pst_mac_vap, 0);
    mac_mib_set_NumberNonCompressedBeamformingMatrixSupportAntenna(pst_mac_vap, 0);

    /* 天线选择能力信息 */
    mac_mib_set_AntennaSelectionOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_TransmitExplicitCSIFeedbackASOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_TransmitIndicesFeedbackASOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_ExplicitCSIFeedbackASOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_ReceiveAntennaSelectionOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_TransmitSoundingPPDUOptionImplemented(pst_mac_vap, OAL_FALSE);

    /* obss信息 */
    mac_mib_init_obss_scan(pst_mac_vap);

    /* 默认使用2040共存 */
    mac_mib_init_2040(pst_mac_vap);

    mac_vap_init_mib_11n_txbf(pst_mac_vap);
}


void mac_vap_init_mib_11ac(mac_vap_stru *pst_mac_vap)
{
    mac_device_stru *pst_mac_dev;

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{mac_vap_init_mib_11ac::pst_mac_dev[%d] null.}",
                       pst_mac_vap->uc_device_id);
        return;
    }

    mac_mib_set_VHTOptionImplemented(pst_mac_vap, OAL_TRUE);

    mac_mib_set_maxmpdu_length(pst_mac_vap, WLAN_MIB_VHT_MPDU_7991);

    mac_mib_set_VHTLDPCCodingOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_LDPC(pst_mac_dev));
    mac_mib_set_VHTShortGIOptionIn80Implemented(pst_mac_vap, OAL_TRUE);
    mac_mib_set_VHTChannelWidthOptionImplemented(
        pst_mac_vap, mac_device_trans_bandwith_to_vht_capinfo(MAC_DEVICE_GET_CAP_BW(pst_mac_dev)));

    // 暂时以HAL device0的能力代替，考虑dbdc以后还需要修改
    if (MAC_DEVICE_GET_CAP_BW(pst_mac_dev) >= WLAN_BW_CAP_160M) {
        mac_mib_set_VHTShortGIOptionIn160and80p80Implemented(pst_mac_vap, WLAN_HAL0_VHT_SGI_SUPP_160_80P80);
    } else {
        mac_mib_set_VHTShortGIOptionIn160and80p80Implemented(pst_mac_vap, OAL_FALSE);
    }

    mac_mib_set_VHTTxSTBCOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_TXSTBC(pst_mac_dev));
    mac_mib_set_VHTRxSTBCOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_RXSTBC(pst_mac_dev));

    /* TxBf相关 */
#ifdef _PRE_WLAN_FEATURE_TXBF
    mac_mib_set_VHTSUBeamformerOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_SUBFER(pst_mac_dev));
    mac_mib_set_VHTSUBeamformeeOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_SUBFEE(pst_mac_dev));
    mac_mib_set_VHTNumberSoundingDimensions(pst_mac_vap, MAC_DEVICE_GET_NSS_NUM(pst_mac_dev));
    mac_mib_set_VHTMUBeamformerOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_MUBFER(pst_mac_dev));
    mac_mib_set_VHTMUBeamformeeOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_MUBFEE(pst_mac_dev));
    mac_mib_set_VHTBeamformeeNTxSupport(pst_mac_vap, VHT_BFEE_NTX_SUPP_STS_CAP);
#else
    mac_mib_set_VHTSUBeamformerOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_VHTSUBeamformeeOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_VHTNumberSoundingDimensions(pst_mac_vap, WLAN_SINGLE_NSS);
    mac_mib_set_VHTMUBeamformerOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_VHTMUBeamformeeOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_VHTBeamformeeNTxSupport(pst_mac_vap, 1);
#endif

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    mac_mib_set_txopps(pst_mac_vap, OAL_TRUE);
#endif

    mac_mib_set_vht_ctrl_field_cap(pst_mac_vap, OAL_FALSE);

    mac_mib_set_vht_max_rx_ampdu_factor(pst_mac_vap, 5); /* 5 VHTMaxRxAMPDUFactor 2^(13+factor)-1字节 */

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    mac_mib_set_OperatingModeNotificationImplemented(pst_mac_vap, OAL_TRUE);
#endif
}


void mac_vap_init_mib_11i(mac_vap_stru *pst_vap)
{
    mac_mib_set_rsnaactivated(pst_vap, OAL_FALSE);
    mac_mib_set_dot11RSNAMFPR(pst_vap, OAL_FALSE);
    mac_mib_set_dot11RSNAMFPC(pst_vap, OAL_FALSE);
    mac_mib_set_pre_auth_actived(pst_vap, OAL_FALSE);
    mac_mib_set_privacyinvoked(pst_vap, OAL_FALSE);
    mac_mib_init_rsnacfg_suites(pst_vap);
    mac_mib_set_rsnacfg_gtksareplaycounters(pst_vap, 0);
    mac_mib_set_rsnacfg_ptksareplaycounters(pst_vap, 0);
}

void mac_vap_init_mib_11k(mac_vap_stru *pst_vap)
{
    if (!IS_LEGACY_STA(pst_vap)) {
        return;
    }
    mac_mib_set_dot11RadioMeasurementActivated(pst_vap, OAL_TRUE);
    mac_mib_set_dot11RMBeaconTableMeasurementActivated(pst_vap, OAL_TRUE);
}


void mac_init_mib(mac_vap_stru *pst_mac_vap)
{
    uint8_t        uc_idx;
    mac_device_stru *pst_mac_dev = NULL;

    if (oal_unlikely(pst_mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_init_mib::pst_mac_vap null.}");
        return;
    }

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{mac_init_mib::pst_mac_dev[%d] null.}", pst_mac_vap->uc_device_id);
        return;
    }

    /* 公共特性mib值初始化 */
    mac_mib_set_dot11dtimperiod(pst_mac_vap, WLAN_DTIM_DEFAULT);
    mac_mib_set_RTSThreshold(pst_mac_vap, WLAN_RTS_MAX);
    mac_mib_set_FragmentationThreshold(pst_mac_vap, WLAN_FRAG_THRESHOLD_MAX);
    mac_mib_set_DesiredBSSType(pst_mac_vap, WLAN_MIB_DESIRED_BSSTYPE_INFRA);
    mac_mib_set_BeaconPeriod(pst_mac_vap, WLAN_BEACON_INTVAL_DEFAULT);
    mac_mib_set_dot11VapMaxBandWidth(pst_mac_vap, MAC_DEVICE_GET_CAP_BW(pst_mac_dev));
    /* 设置vap最大用户数 */
    mac_mib_set_MaxAssocUserNums(pst_mac_vap, mac_chip_get_max_asoc_user(pst_mac_vap->uc_chip_id));
    mac_mib_set_WPSActive(pst_mac_vap, OAL_FALSE);

    /* 2040共存信道切换开关初始化 */
    /* P2P设备关闭20/40代管切换 */
    if (pst_mac_vap->en_p2p_mode != WLAN_LEGACY_VAP_MODE) {
        mac_mib_set_2040SwitchProhibited(pst_mac_vap, OAL_TRUE);
    } else {
        /* 非P2P设备开启20/40代管切换 */
        mac_mib_set_2040SwitchProhibited(pst_mac_vap, OAL_FALSE);
    }

    /* 初始化认证类型为OPEN */
    mac_mib_set_AuthenticationMode(pst_mac_vap, WLAN_WITP_AUTH_OPEN_SYSTEM);

    /* 默认设置为自动触发BA回话的建立 */
    mac_mib_set_AddBaMode(pst_mac_vap, WLAN_ADDBA_MODE_AUTO);

    mac_mib_set_CfgAmpduTxAtive(pst_mac_vap, OAL_TRUE);
    mac_mib_set_RxBASessionNumber(pst_mac_vap, 0);
    mac_mib_set_TxBASessionNumber(pst_mac_vap, 0);

    /*
     * 1151默认不amsdu ampdu 联合聚合功能不开启 1102用于小包优化
     * 因tplink/syslink下行冲包兼容性问题，先关闭02的ampdu+amsdu
     */
    mac_mib_set_CfgAmsduTxAtive(pst_mac_vap, OAL_FALSE);
    mac_mib_set_AmsduPlusAmpduActive(pst_mac_vap, OAL_FALSE);

    mac_mib_set_ShortPreambleOptionImplemented(pst_mac_vap, WLAN_LEGACY_11B_MIB_SHORT_PREAMBLE);
    mac_mib_set_PBCCOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_ChannelAgilityPresent(pst_mac_vap, OAL_FALSE);
    mac_mib_set_dot11MultiDomainCapabilityActivated(pst_mac_vap, OAL_TRUE);
    mac_mib_set_SpectrumManagementRequired(pst_mac_vap, OAL_TRUE);
    mac_mib_set_dot11ExtendedChannelSwitchActivated(pst_mac_vap, OAL_FALSE);
    mac_mib_set_dot11QosOptionImplemented(pst_mac_vap, OAL_TRUE);
    mac_mib_set_dot11APSDOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_dot11QBSSLoadImplemented(pst_mac_vap, OAL_TRUE);
    mac_mib_set_ShortSlotTimeOptionImplemented(pst_mac_vap, OAL_TRUE);
    mac_mib_set_ShortSlotTimeOptionActivated(pst_mac_vap, OAL_TRUE);
    mac_mib_set_dot11RadioMeasurementActivated(pst_mac_vap, OAL_FALSE);

    mac_mib_set_DSSSOFDMOptionActivated(pst_mac_vap, OAL_FALSE);
    mac_mib_set_dot11ImmediateBlockAckOptionImplemented(pst_mac_vap, OAL_TRUE);
    mac_mib_set_dot11DelayedBlockAckOptionImplemented(pst_mac_vap, OAL_FALSE);

    /* 不同芯片，auth超时时间配置不同值 */
    wlan_chip_mac_mib_set_auth_rsp_time_out(pst_mac_vap);

    mac_mib_set_HtProtection(pst_mac_vap, WLAN_MIB_HT_NO_PROTECTION);
    mac_mib_set_RifsMode(pst_mac_vap, OAL_TRUE);
    mac_mib_set_NonGFEntitiesPresent(pst_mac_vap, OAL_FALSE);
    mac_mib_set_LsigTxopFullProtectionActivated(pst_mac_vap, OAL_FALSE);

    mac_mib_set_DualCTSProtection(pst_mac_vap, OAL_FALSE);
    mac_mib_set_PCOActivated(pst_mac_vap, OAL_FALSE);

    mac_mib_set_dot11AssociationResponseTimeOut(pst_mac_vap, WLAN_ASSOC_TIMEOUT);
    mac_mib_set_dot11AssociationSAQueryMaximumTimeout(pst_mac_vap, WLAN_SA_QUERY_MAXIMUM_TIME);
    mac_mib_set_dot11AssociationSAQueryRetryTimeout(pst_mac_vap, WLAN_SA_QUERY_RETRY_TIME);

    /* WEP 缺省Key表初始化 */
    for (uc_idx = 0; uc_idx < WLAN_NUM_DOT11WEPDEFAULTKEYVALUE; uc_idx++) {
        /* 大小初始化为 WEP-40 */
        mac_mib_set_wep(pst_mac_vap, uc_idx, WLAN_WEP_40_KEY_SIZE);
    }

    /* 相关私有表初始化  */
    mac_mib_set_privacyinvoked(pst_mac_vap, OAL_FALSE);
    mac_set_wep_default_keyid(pst_mac_vap, 0);

    /* 更新wmm参数初始值 */
    mac_vap_init_wme_param(pst_mac_vap);

    /* 11i */
    mac_vap_init_mib_11i(pst_mac_vap);

    /* 默认11n 11ac使能关闭，配置协议模式时打开 */
    mac_vap_init_mib_11n(pst_mac_vap);
    mac_vap_init_mib_11ac(pst_mac_vap);
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_vap_init_mib_11ax(pst_mac_vap);
#endif

    /* staut低功耗mib项初始化 */
    mac_mib_set_powermanagementmode(pst_mac_vap, WLAN_MIB_PWR_MGMT_MODE_ACTIVE);

    /* 11k */
    mac_vap_init_mib_11k(pst_mac_vap);
    mac_init_mib_extend(pst_mac_vap);
}


void mac_vap_change_mib_by_bandwidth(mac_vap_stru *pst_mac_vap,
    wlan_channel_band_enum_uint8 en_band, wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    wlan_mib_ieee802dot11_stru *pst_mib_info;
    wlan_bw_cap_enum_uint8      en_bw;
    oal_bool_enum_uint8 en_40m_enable;

    pst_mib_info = pst_mac_vap->pst_mib_info;

    if (pst_mib_info == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_change_mib_by_bandwidth::pst_mib_info null.}");
        return;
    }

    /* 更新40M使能mib, 默认使能 */
    mac_mib_set_FortyMHzOperationImplemented(pst_mac_vap, OAL_TRUE);

    // 去掉shortGI强制使能，使用iwpriv命令设置，否则用户配置会被覆盖
    /* 更新short gi使能mib, 默认全使能，根据带宽信息更新 */
    mac_mib_set_ShortGIOptionInTwentyImplemented(pst_mac_vap, OAL_TRUE);
    mac_mib_set_ShortGIOptionInFortyImplemented(pst_mac_vap, OAL_TRUE);

    if ((pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) &&
        (pst_mac_vap->st_cap_flag.bit_disable_2ght40 == OAL_TRUE)) {
        mac_mib_set_FortyMHzOperationImplemented(pst_mac_vap, OAL_FALSE);
    }

    // 去掉shortGI强制使能，使用iwpriv命令设置，否则用户配置会被覆盖
    mac_mib_set_VHTShortGIOptionIn80Implemented(pst_mac_vap, OAL_TRUE);

    en_bw = mac_vap_bw_mode_to_bw(en_bandwidth);
    if (en_bw == WLAN_BW_CAP_20M) {
        mac_mib_set_FortyMHzOperationImplemented(pst_mac_vap, OAL_FALSE);
    }

#ifndef WIN32
    en_40m_enable = (WLAN_BAND_WIDTH_20M != en_bandwidth);

    if (en_band == WLAN_BAND_2G) {
        mac_mib_set_2GFortyMHzOperationImplemented(pst_mac_vap, en_40m_enable);
    } else {
        mac_mib_set_5GFortyMHzOperationImplemented(pst_mac_vap, en_40m_enable);
    }
#endif
}


void mac_mib_set_wep(mac_vap_stru *pst_mac_vap, uint8_t uc_key_id, uint8_t uc_key_value)
{
    wlan_mib_Dot11WEPDefaultKeysEntry_stru *pst_wlan_mib_wep_dflt_key;

    pst_wlan_mib_wep_dflt_key = &(pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.ast_wlan_mib_wep_dflt_key[0]);

    if (uc_key_id >= WLAN_NUM_DOT11WEPDEFAULTKEYVALUE) {
        return;
    }

    /* 初始化wep相关MIB信息 */
    memset_s(pst_wlan_mib_wep_dflt_key[uc_key_id].auc_dot11WEPDefaultKeyValue,
             WLAN_MAX_WEP_STR_SIZE, 0,
             WLAN_MAX_WEP_STR_SIZE);
    pst_wlan_mib_wep_dflt_key[uc_key_id].auc_dot11WEPDefaultKeyValue[WLAN_WEP_SIZE_OFFSET] = uc_key_value;
}

#ifdef _PRE_WLAN_FEATURE_11R

uint32_t mac_mib_init_ft_cfg(mac_vap_stru *pst_mac_vap, uint8_t *puc_mde)
{
    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_WPA, "{mac_mib_init_ft_cfg::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_vap->pst_mib_info == NULL) {
        oam_error_log0(0, OAM_SF_WPA, "{mac_mib_init_ft_cfg::pst_mib_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (WLAN_WITP_AUTH_FT != mac_mib_get_AuthenticationMode(pst_mac_vap)) {
        mac_mib_set_ft_trainsistion(pst_mac_vap, OAL_FALSE);
        return OAL_SUCC;
    }

    if ((puc_mde == NULL) || (puc_mde[0] != MAC_EID_MOBILITY_DOMAIN) || (puc_mde[1] != 3)) { /* MDIE lenth 3 */
        mac_mib_set_ft_trainsistion(pst_mac_vap, OAL_FALSE);
        return OAL_SUCC;
    }

    mac_mib_set_ft_trainsistion(pst_mac_vap, OAL_TRUE);
    mac_mib_set_ft_mdid (pst_mac_vap, (uint16_t)((puc_mde[BYTE_OFFSET_3] << BIT_OFFSET_8) | puc_mde[BYTE_OFFSET_2]));
    mac_mib_set_ft_over_ds(pst_mac_vap, (puc_mde[BYTE_OFFSET_4] & BIT0) ? OAL_TRUE : OAL_FALSE);
    mac_mib_set_ft_resource_req(pst_mac_vap, (puc_mde[BYTE_OFFSET_4] & BIT1) ? OAL_TRUE : OAL_FALSE);

    return OAL_SUCC;
}


uint32_t mac_mib_get_md_id(mac_vap_stru *pst_mac_vap, uint16_t *pus_mdid)
{
    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_WPA, "{mac_mib_init_ft_cfg::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_vap->pst_mib_info == NULL) {
        oam_error_log0(0, OAM_SF_WPA, "{mac_mib_init_ft_cfg::pst_mib_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_FALSE == mac_mib_get_ft_trainsistion(pst_mac_vap)) {
        return OAL_FAIL;
    }

    *pus_mdid = mac_mib_get_ft_mdid(pst_mac_vap);
    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_11R

