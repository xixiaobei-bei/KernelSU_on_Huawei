

/* 1 头文件包含 */
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "mac_ie.h"
#include "mac_frame.h"
#include "mac_vap.h"
#include "mac_device.h"
#include "mac_regdomain.h"
#include "dmac_ext_if.h"
#include "mac_user.h"
#ifdef _PRE_WLAN_FEATURE_11AX
#include "mac_resource.h"
#endif
#include "securec.h"
#include "securectype.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_FRAME_C

/* 2 函数原型声明 */
/* 3 全局变量定义 */
/* 4 函数实现 */

oal_uint8 *mac_find_ie_ext_ie(oal_uint8 uc_eid, oal_uint8 uc_ext_eid, oal_uint8 *puc_ies, oal_int32 l_len)
{
    if (puc_ies == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    while ((l_len > MAC_IE_EXT_HDR_LEN) && ((puc_ies[0] != uc_eid) || (puc_ies[2] != uc_ext_eid))) {
        l_len -= puc_ies[1] + MAC_IE_HDR_LEN;
        puc_ies += puc_ies[1] + MAC_IE_HDR_LEN;
    }

    if ((l_len > MAC_IE_EXT_HDR_LEN) && (uc_eid == puc_ies[0]) && (uc_ext_eid == puc_ies[2]) &&
        (l_len >= (MAC_IE_HDR_LEN + puc_ies[1]))) {
        return puc_ies;
    }

    return OAL_PTR_NULL;
}

#ifdef _PRE_WLAN_FEATURE_11AX

OAL_STATIC oal_void mac_set_he_mac_capinfo_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_frame_he_mac_cap_stru *pst_he_mac_capinfo = (mac_frame_he_mac_cap_stru *)puc_buffer;

    /* 初始化置0，下面填充字段时对于不支持的能力位默认不写出来 */
    memset_s(pst_he_mac_capinfo, OAL_SIZEOF(mac_frame_he_mac_cap_stru), 0, OAL_SIZEOF(mac_frame_he_mac_cap_stru));
    /*********************** HE MAC 能力信息域 ************************************
    ----------------------------------------------------------------------------
     |-----------------------------------------------------------------------------------------|
     | +HTC    | TWT         | TWT         | Fragmentation | Max Num      | Min                |
     | HE        | Requester | Responder   | support       | Fragmented   | Fragment           |
     | Support | Support     |   Support   |               | MSDUs        |     Size           |
     |-----------------------------------------------------------------------------------------|
     | B0        | B1        | B2          |   B3  B4      |  B5 B6 B7    |  B8 B9             |
     |-----------------------------------------------------------------------------------------|
     |-----------------------------------------------------------------------------------------|
     | Trigger Frame | Muti-TID     | HE           | ALL        | UMRS        | BSR            |
     | MAC Padding   | Aggregation  | Link         | ACK        | Support     | Support        |
     | Duration      | Support      | Adaptation   | Support    |             |                |
     |-----------------------------------------------------------------------------------------|
     |    B10   B11  | B12      B14 | B15    B16   |   B17      |    B18      | B19            |
     |-----------------------------------------------------------------------------------------|
     |-----------------------------------------------------------------------------------------|
     | Broadcast | 32-bit BA | MU       | Ack-Enable             | Group Addressed   Multi-STA |
     | TWT       | bitmap    | Cascade  | Multi-TID              | BlockAck In DL MU           |
     | Support   | Support   | Support  | Aggregation Support    | Support                     |
     |-----------------------------------------------------------------------------------------|
     | B20       | B21       | B22      |   B23                  |   B24                       |
     |-----------------------------------------------------------------------------------------|
     |-----------------------------------------------------------------------------------------|
     | OM        | OFDMA   | Max A-MPDU  | A-MSDU          | Flexible  TWT| Rx Control         |
     | Control   | RA      | Length      | Fragmentation   |  schedule    | frame to           |
     | Support   | Support | Exponent    |  Support        | Support      | MultiBSS           |
     |---------------------------------------------------------------------------------------- |
     | B25       | B26     | B27  B28    |   B29           |   B30        |  B31               |
     |-----------------------------------------------------------------------------------------|
     |-----------------------------------------------------------------------------------------|
     | NSRP BQRP   | QTP     | BQR     | SR       | NDP     | OPS      | A-MSDU in |           |
     | A-MPDU      |         |         |          |Feedback |  Support | A-MPDU    |           |
     | Aggregation | Support | Support |Responder | Report  |          | Support   |           |
     |-----------------------------------------------------------------------------------------|
     | B32         | B33     | B34     |   B35    |   B36   |  B37     |   B38     |           |
     |-----------------------------------------------------------------------------------------|
     |-----------------------------------------------------------------------------------------|
     | Multi-TID            |                                                                  |
     | Aggregation  Tx      |         Reserved                                                 |
     | Support              |                                                                  |
     |-----------------------------------------------------------------------------------------|
     | B39  B40   B41       |         B42--B47                                                 |
     |-----------------------------------------------------------------------------------------|
    ***************************************************************************/
    /* B0:+HTC HE Support */
    pst_he_mac_capinfo->bit_htc_he_support = OAL_TRUE;

    /* B1:TWT Requester Support */
    pst_he_mac_capinfo->bit_twt_requester_support = mac_mib_get_he_TWTOptionActivated(pst_mac_vap);

    /* B10-11:Trigger Frame MAC Padding Duration   16us */
    pst_he_mac_capinfo->bit_trigger_mac_padding_duration = mac_mib_get_he_TriggerMacPaddingDuration(pst_mac_vap);
    /* WiFi6认证用命令配置,动态设置协商mac padding */
    if (pst_mac_vap->bit_mac_padding > 0) {
        pst_he_mac_capinfo->bit_trigger_mac_padding_duration = pst_mac_vap->bit_mac_padding - 1;
    }

    /* B12-B14:Multi-TID Aggregation Rx support */
    pst_he_mac_capinfo->bit_mtid_aggregation_rx_support = WLAN_TID_MAX_NUM - 1;

    /* B19:BSR Support */
    pst_he_mac_capinfo->bit_bsr_support = mac_mib_get_he_BSRSupport(pst_mac_vap);

    /* B25:OM Control Support */
    if (pst_he_mac_capinfo->bit_htc_he_support) {
        pst_he_mac_capinfo->bit_om_control_support = mac_mib_get_OMIOptionImplemented(pst_mac_vap);
    }

    /* B27-28:Maximum AMPDU Length Exponent */
    pst_he_mac_capinfo->bit_max_ampdu_length_exponent = mac_mib_get_he_MaxAMPDULengthExponent(pst_vap);


    if (IS_STA(pst_mac_vap)) {
        /* B32:BSRP BQRP AMPDU Aggregation Support */
        pst_he_mac_capinfo->bit_bsrp_bqrp_ampdu_addregation = 1;

        /* B34:BQR Support */
        pst_he_mac_capinfo->bit_bqr_support = 0;

        /* B36:NDP Feedback Report Support */
        pst_he_mac_capinfo->bit_ndp_feedback_report_support = 0;

        /* B43:UL 2×996-tone RU Support */
        pst_he_mac_capinfo->bit_ul_2x996_ru_support = 1;

        /* B47:接收HT/VHT格式的Basic trigger帧 */
        pst_he_mac_capinfo->bit_he_and_vht_trigger_frame_rx_support = 1;
    }

}


OAL_STATIC oal_void mac_set_he_phy_capinfo_field_part2(mac_vap_stru *pst_mac_vap,
    mac_frame_he_phy_cap_stru *pst_he_phy_capinfo)
{
    mac_vap_rom_stru *pst_mac_vap_rom = (mac_vap_rom_stru *)pst_mac_vap->_rom;


    /* B53: HE ER SU PPDU TRX */
    pst_he_phy_capinfo->bit_partial_bandwidth_extended_range = OAL_TRUE;

    /* B55:PPE Threshold Present */
    pst_he_phy_capinfo->bit_ppe_threshold_present = mac_mib_get_PPEThresholdsRequired(pst_mac_vap_rom);

    /* B58:HE SU PPDU And HE MU PPDU with 4x HE-LTF And 0.8usGI */
    pst_he_phy_capinfo->bit_he_su_mu_ppdu_4xltf_08us_gi = 1;

    /* B59-B61:Max Nc */
    pst_he_phy_capinfo->bit_max_nc = 1;

    if (IS_STA(pst_mac_vap)) {
        /* B62:UL OFDMA STBC Tx>80MHz */
        pst_he_phy_capinfo->bit_stbc_tx_over_80mhz = 1;
    }

    /* B63:STBC Rx>80MHz */
    pst_he_phy_capinfo->bit_stbc_rx_over_80mhz = OAL_FALSE;

    /* B64:HE ER SU PPDU with 4x HE-LTF And 0.8usGI */
    pst_he_phy_capinfo->bit_he_er_su_ppdu_4xltf_08us_gi = 1;

    /* B65: STA强制支持,AP reserve */
    if (IS_STA(pst_mac_vap)) {
        /* 20/40MHz HE PPDU 2.4G */
        pst_he_phy_capinfo->bit_20mhz_in_40mhz_he_ppdu_2g = 1;
        /* 5G支持160M,必须强制支持 */
        pst_he_phy_capinfo->bit_20mhz_in_160mhz_he_ppdu = 1;
        pst_he_phy_capinfo->bit_80mhz_in_160mhz_he_ppdu = 1;
    }

    /* B68:HE ER SU PPDU with 1x HE-LTF And 0.8usGI */
    pst_he_phy_capinfo->bit_he_er_su_ppdu_1xltf_08us_gi = 1;

    if (IS_STA(pst_mac_vap)) {
        pst_he_phy_capinfo->bit_longer_than_16_he_sigb_support = OAL_FALSE;
        pst_he_phy_capinfo->bit_tx_1024qam_below_242ru_support = OAL_TRUE;
        pst_he_phy_capinfo->bit_rx_1024qam_below_242ru_support = OAL_TRUE;
        /* B76:Rx Full BW SU Using HE MU PPDU With Compressed SIGB */
        pst_he_phy_capinfo->bit_rx_full_bw_su_ppdu_with_compressed_sigb = OAL_TRUE;
        pst_he_phy_capinfo->bit_rx_full_bw_su_ppdu_with_non_compressed_sigb = OAL_TRUE;
    }

}

#ifdef _PRE_WLAN_FEATURE_M2S
OAL_STATIC OAL_INLINE oal_void mac_set_he_sounding_bfmee_sts_field(mac_vap_stru *pst_mac_vap,
    mac_frame_he_phy_cap_stru *pst_he_phy_capinfo)
{
    mac_user_stru *pst_mac_user = OAL_PTR_NULL;
    mac_he_hdl_stru *pst_he_hdl = OAL_PTR_NULL;

    /* 参考标杆,该字段根据对端num of sounding和自己的能力取交集 */
    pst_mac_user = mac_res_get_mac_user_etc(pst_mac_vap->us_assoc_vap_id);
    if (IS_STA(pst_mac_vap) && pst_mac_user != OAL_PTR_NULL) {
        pst_he_hdl = MAC_USER_HE_HDL_STRU(pst_mac_user);
        if (pst_he_hdl->st_he_cap_ie.st_he_phy_cap.bit_below_80mhz_sounding_dimensions_num != 0) {
            pst_he_phy_capinfo->bit_beamformee_sts_below_80mhz = OAL_MIN(
                pst_he_phy_capinfo->bit_beamformee_sts_below_80mhz,
                pst_he_hdl->st_he_cap_ie.st_he_phy_cap.bit_below_80mhz_sounding_dimensions_num);
            pst_he_phy_capinfo->bit_beamformee_sts_below_80mhz =
                OAL_MAX(pst_he_phy_capinfo->bit_beamformee_sts_below_80mhz, 3);
        }
        if (pst_he_hdl->st_he_cap_ie.st_he_phy_cap.bit_over_80mhz_sounding_dimensions_num != 0) {
            pst_he_phy_capinfo->bit_beamformee_sts_over_80mhz = OAL_MIN(
                pst_he_phy_capinfo->bit_beamformee_sts_over_80mhz,
                pst_he_hdl->st_he_cap_ie.st_he_phy_cap.bit_over_80mhz_sounding_dimensions_num);
            pst_he_phy_capinfo->bit_beamformee_sts_over_80mhz =
                OAL_MAX(pst_he_phy_capinfo->bit_beamformee_sts_over_80mhz, 3);
        }
    }
}
#endif


OAL_STATIC oal_void mac_set_he_sounding_capinfo_field(mac_vap_stru *pst_mac_vap,
    mac_frame_he_phy_cap_stru *pst_he_phy_capinfo)
{
    oal_uint8 uc_below80mhz;
    oal_uint8 uc_over80mhz;

    mac_vap_rom_stru *pst_mac_vap_rom = (mac_vap_rom_stru *)pst_mac_vap->_rom;

    /* B31:SU Beamformer */
    pst_he_phy_capinfo->bit_su_beamformer = mac_mib_get_he_SUBeamformer(pst_mac_vap);

    /* B32:SU Beamformee */
    pst_he_phy_capinfo->bit_su_beamformee = mac_mib_get_he_SUBeamformee(pst_mac_vap);

    /* B33:MU Beamformer */
    pst_he_phy_capinfo->bit_mu_beamformer = mac_mib_get_he_MUBeamformer(pst_mac_vap);

    uc_below80mhz = mac_mib_get_he_BeamformeeSTSBelow80Mhz(pst_mac_vap_rom);
    uc_over80mhz = mac_mib_get_he_BeamformeeSTSOver80Mhz(pst_mac_vap_rom);
    if ((uc_below80mhz >= 1) && (uc_over80mhz >= 1)) {
        /* B34-B36:Beamformee STS ≤ 80MHz,最小值为3， 1103支持4*2 1105支持8*2 */
        pst_he_phy_capinfo->bit_beamformee_sts_below_80mhz = OAL_MAX(uc_below80mhz - 1, 3);
        /* B37-B39:Beamformee STS > 80MHz,最小值为3， 1103支持4*2 1105支持8*2 */
        pst_he_phy_capinfo->bit_beamformee_sts_over_80mhz = OAL_MAX(uc_over80mhz - 1, 3);
    }

    if (pst_he_phy_capinfo->bit_su_beamformer == 1) {
#if (WLAN_MAX_NSS_NUM >= WLAN_DOUBLE_NSS)
        /* B40-B42: beamformer支持的最大发送能力 */
        pst_he_phy_capinfo->bit_below_80mhz_sounding_dimensions_num =
            mac_mib_get_HENumberSoundingDimensionsBelow80Mhz(pst_mac_vap_rom);

        /* B43-B45:beamformer支持的最大发送能力 */
        pst_he_phy_capinfo->bit_over_80mhz_sounding_dimensions_num =
            mac_mib_get_HENumberSoundingDimensionsOver80Mhz(pst_mac_vap_rom);
#endif
    }

#ifdef _PRE_WLAN_FEATURE_M2S
    mac_set_he_sounding_bfmee_sts_field(pst_mac_vap, pst_he_phy_capinfo); // 封装函数降低行数
#endif

    /* B46:Ng = 16 SU Feedback */
    pst_he_phy_capinfo->bit_ng16_su_feedback = mac_mib_get_HENg16SUFeedback(pst_mac_vap_rom);

    /* B47:Ng = 16 MU Feedback */
    pst_he_phy_capinfo->bit_ng16_mu_feedback = mac_mib_get_HENg16MUFeedback(pst_mac_vap_rom);

    /* B48:CodeBook Size(Φ,Ψ)={4,2} SU Feedback */
    pst_he_phy_capinfo->bit_codebook_42_su_feedback = mac_mib_get_HECodebook42SUFeedback(pst_mac_vap_rom);

    /* B49:CodeBook Size(Φ,Ψ)={7,5} MU Feedback */
    pst_he_phy_capinfo->bit_codebook_75_mu_feedback = mac_mib_get_HECodebook75MUFeedback(pst_mac_vap_rom);

    /* B50:trigger_su_beamforming_feedback */
    pst_he_phy_capinfo->bit_trigger_su_beamforming_feedback = OAL_FALSE;

    /* B51:trigger_mu_beamforming_partialBW_feedback */
    pst_he_phy_capinfo->bit_trigger_mu_beamforming_partialBW_feedback = OAL_FALSE;

    /* B52:trigger CQI feedback */
    pst_he_phy_capinfo->bit_trigger_cqi_feedback = OAL_FALSE;

}


OAL_STATIC oal_uint8 mac_set_he_channel_width(mac_vap_stru *pst_mac_vap)
{
    if ((pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) && (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
            return mac_mib_get_2GFortyMHzOperationImplemented(pst_mac_vap);
    }

    return mac_device_trans_bandwith_to_he_capinfo(mac_mib_get_dot11VapMaxBandWidth(pst_mac_vap));

}


OAL_STATIC oal_void mac_set_he_phy_capinfo_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_frame_he_phy_cap_stru *pst_he_phy_capinfo = (mac_frame_he_phy_cap_stru *)puc_buffer;

    /************************************** HE PHY 能力信息域 **************************************
    -----------------------------------------------------------------------------------------------|
     |---------------------------------------------------------------------------------------------|
     | Reserved  | Channel    | Punctured  | Device  | LDPC        | HE SU PPDU With  | Midamble   |
     |           | Width      | Preamble   | Class   | Coding In   | 1x HE-LTF And    | Rx Max     |
     |           | Set        | RX         |         | Payload     | 0.8 us GI        | NSTS       |
     |---------------------------------------------------------------------------------------------|
     | B0        | B1 B7      | B8 B11     | B12     | B13         | B14              | B15 B16    |
     |---------------------------------------------------------------------------------------------|
     |---------------------------------------------------------------------------------------------|
     | NDP With      | STBC Tx   | STBC Rx   | Doppler | Doppler | Full           | Partial        |
     | 4x HE-LTF And | <= 80 MHz | <= 80 MHz | Tx      | Rx      | Bandwidth UL   | Bandwidth UL   |
     | 3.2 ms GI     |           |           |         |         | MU-MIMO        | MU-MIMO        |
     |---------------------------------------------------------------------------------------------|
     | B17           | B18       | B19       | B20     | B21     | B22            | B23            |
     |---------------------------------------------------------------------------------------------|
     |---------------------------------------------------------------------------------------------|
     | DCM Max       | DCM     | DCM Max       | DCM    | Rx HE MU PPDU  | SU         | SU         |
     | Constellation | Max     | Constellation | Max    | From           | Beamformer | Beamformee |
     | Tx            | NSS Tx  | Rx            | NSS Rx | Non-AP STA     |            |            |
     |---------------------------------------------------------------------------------------------|
     | B24 B25       | B26     | B27 B28       | B29    | B30            | B31        | B32        |
     |---------------------------------------------------------------------------------------------|
     |---------------------------------------------------------------------------------------------|
     | MU         | Beamformee | Beamformee  | Number Of Sounding | Number Of Sounding | Ng = 16   |
     | Beamformer | STS <= 80  | STS >= 80   | Dimensions         | Dimensions         | SU        |
     |            | MHz        | MHz         | <= 80              | >= 80              | Feedback  |
     |---------------------------------------------------------------------------------------------|
     | B33        | B34 B36    | B37  B39    | B40 B42            | B43 B45            | B46       |
     |---------------------------------------------------------------------------------------------|
     |---------------------------------------------------------------------------------------------|
     | Ng = 16   | Codebook     | Codebook     | Triggered SU  | Triggered MU         | Triggered  |
     | MU        | Size = {4,2} | Size = {7,5} | Beamforming   | Beamforming          | CQI        |
     | Feedback  | SU Feedback  | MU Feedback  | Feedback      | Partial BW Feedback  | Feedback   |
     |---------------------------------------------------------------------------------------------|
     | B47       | B48          | B49          | B50           | B51                  | B52        |
     |---------------------------------------------------------------------------------------------|
     |---------------------------------------------------------------------------------------------|
     | Partial        | Partial      | PPE       | SRP-based  | Power Boost  | HE SU PPDU And      |
     | Bandwidth      | Bandwidth    | Threshold | SR         | Factor       | HE MU PPDU With 4x  |
     | Extended Range | DL MU-MIMO   | Present   | Support    | Support      | HE-LTF And 0.8us GI |
     |---------------------------------------------------------------------------------------------|
     | B53            | B54          | B55       | B56        | B57          | B58                 |
     |---------------------------------------------------------------------------------------------|
     |---------------------------------------------------------------------------------------------|
     | Max     | STBC      | STBC      | HE ER SU PPDU   | 20 MHz In 40 MHz  | 20 MHz In           |
     | Nc      | Tx        | Rx        | With 4x HE-LTF  | HE PPDU In        | 160/80+80 MHZ       |
     |         | > 80MHz   | > 80MHz   | And 0.8us GI    | 2.4GHz Band       | HE PPDU             |
     |---------------------------------------------------------------------------------------------|
     | B59 B61 | B62       | B63       | B64             | B65               | B66                 |
     |---------------------------------------------------------------------------------------------|
     |---------------------------------------------------------------------------------------------|
     | 80 MHz In      | HE ER SU PPDU   | Midamble Rx 2x   |                                       |
     | 160/80+80 MHZ  | With 1x HE-LTF  | And              |                Reserve                |
     | HE PPDU        | And 0.8us GI    | 1x HE-LTF        |                                       |
     |---------------------------------------------------------------------------------------------|
     | B67            | B68             | B69              |                B70 B71                |
     |---------------------------------------------------------------------------------------------|
    ************************************************************************************************/
    /* 初始化置0，下面填充字段时对于不支持的能力位默认不写出来 */
    memset_s(pst_he_phy_capinfo, OAL_SIZEOF(mac_frame_he_phy_cap_stru), 0, OAL_SIZEOF(mac_frame_he_phy_cap_stru));

    /* B1-B7:channel width set, 设置vap下的带宽能力 */
    pst_he_phy_capinfo->bit_channel_width_set = mac_set_he_channel_width(pst_mac_vap);

    /* B8-B11:punctured preamble rx */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        pst_he_phy_capinfo->bit_punctured_preamble_rx = 0;
    }
    /* B12:device class */
    pst_he_phy_capinfo->bit_device_class = PHY_DEVICE_CLASS_A;

    /* B13:LDPC Coding in payload,认证用例5.24、5.25要求支持 */
    pst_he_phy_capinfo->bit_ldpc_coding_in_paylod = mac_mib_get_he_LDPCCodingInPayload(pst_mac_vap);

    /* B14:HE SU PPDU with 1x HE-LTF and 0.8us GI */
    pst_he_phy_capinfo->bit_he_su_ppdu_1xltf_08us_gi = 1;

    /* B17:HE NDP PPDU支持4x HE-LTF+3.2us Gi */
    pst_he_phy_capinfo->bit_ndp_4xltf_32us = 1;

    /* B18:UL OFDMA STBC Tx<=80MHz */
    if (IS_STA(pst_mac_vap)) {
        pst_he_phy_capinfo->bit_stbc_tx_below_80mhz = 1;
    }

    /* B19:STBC Rx<=80MHz */
    pst_he_phy_capinfo->bit_stbc_rx_below_80mhz =
        (IS_STA(pst_mac_vap)) ? pst_mac_vap->st_cap_flag.bit_rx_stbc : OAL_TRUE;

    /* B22:Full Bandwidth UL MU-MIMO */
    if (IS_STA(pst_mac_vap)) {
        pst_he_phy_capinfo->bit_full_bandwidth_ul_mu_mimo = OAL_FALSE;
    }

    /* sounding参数 */
    mac_set_he_sounding_capinfo_field(pst_mac_vap, pst_he_phy_capinfo);

    mac_set_he_phy_capinfo_field_part2(pst_mac_vap, pst_he_phy_capinfo);

}


OAL_STATIC oal_uint8 mac_set_he_tx_rx_mcs_nss_field(mac_vap_stru *pst_vap,
                                                    oal_uint8 uc_channel_width_b2,
                                                    oal_uint8 uc_channel_width_b3,
                                                    oal_uint8 *puc_buffer)
{
    oal_uint8 uc_len = 0;
    mac_device_stru *pst_mac_dev;
    mac_fram_he_mac_nsss_set_stru *pst_he_tx_rx_mcs_nss_info = (mac_fram_he_mac_nsss_set_stru *)puc_buffer;

    pst_mac_dev = (mac_device_stru *)mac_res_get_dev_etc(pst_vap->uc_device_id);
    if (pst_mac_dev == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY,
                       "{mac_set_he_tx_rx_mcs_nss_field::mac_res_get_dev_etc fail.device_id:[%d].}",
                       pst_vap->uc_device_id);
        return uc_len;
    }

    /******************************** HE Supported HE-MCS And NSS Set *********************************
    |-------------------------------------------------------------------------------------------------|
    | Rx HE-MCS Map | Tx HE-MCS Map | Rx HE-MCS Map  | Tx HE-MCS Map  | Rx HE-MCS Map | Tx HE-MCS Map |
    | <= 80 MHz     | <= 80 MHz     | 160 MHz        | 160 MHz        | 80+80 MHz     | 80+80 MHz     |
    |-------------------------------------------------------------------------------------------------|
    | 2 Octets      | 2 Octets      | 0 or 2 Octets  | 0 or 2 Octets  | 0 or 2 Octets | 0 or 2 Octets |
    |-------------------------------------------------------------------------------------------------|
    **************************************************************************************************/

    memset_s(pst_he_tx_rx_mcs_nss_info, OAL_SIZEOF(mac_fram_he_mac_nsss_set_stru),
             0, OAL_SIZEOF(mac_fram_he_mac_nsss_set_stru));
    /* 带宽<=80MHz的rx能力 */
    pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_below_80mhz.bit_max_he_mcs_for_1ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
    pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_below_80mhz.bit_max_he_mcs_for_2ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    if (WLAN_DOUBLE_NSS == MAC_DEVICE_GET_NSS_NUM(pst_mac_dev)) {
        pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_below_80mhz.bit_max_he_mcs_for_2ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
    }
    pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_below_80mhz.bit_max_he_mcs_for_3ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_below_80mhz.bit_max_he_mcs_for_4ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_below_80mhz.bit_max_he_mcs_for_5ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_below_80mhz.bit_max_he_mcs_for_6ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_below_80mhz.bit_max_he_mcs_for_7ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_below_80mhz.bit_max_he_mcs_for_8ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;

    uc_len += OAL_SIZEOF(mac_frame_he_mcs_nss_bit_map_stru);

    /* 带宽<=80MHz的tx能力 */
    pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_below_80mhz.bit_max_he_mcs_for_1ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
    pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_below_80mhz.bit_max_he_mcs_for_2ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    if (WLAN_DOUBLE_NSS == MAC_DEVICE_GET_NSS_NUM(pst_mac_dev)) {
        pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_below_80mhz.bit_max_he_mcs_for_2ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
    }
    pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_below_80mhz.bit_max_he_mcs_for_3ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_below_80mhz.bit_max_he_mcs_for_4ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_below_80mhz.bit_max_he_mcs_for_5ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_below_80mhz.bit_max_he_mcs_for_6ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_below_80mhz.bit_max_he_mcs_for_7ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_below_80mhz.bit_max_he_mcs_for_8ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;

    uc_len += OAL_SIZEOF(mac_frame_he_mcs_nss_bit_map_stru);

    if (uc_channel_width_b2 != 0) {
        /* Rx HE-MCS Map 160 MHz */
        pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_160mhz.bit_max_he_mcs_for_1ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
        pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_160mhz.bit_max_he_mcs_for_2ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
        if (WLAN_DOUBLE_NSS == MAC_DEVICE_GET_NSS_NUM(pst_mac_dev)) {
            pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_160mhz.bit_max_he_mcs_for_2ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
        }
        pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_160mhz.bit_max_he_mcs_for_3ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
        pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_160mhz.bit_max_he_mcs_for_4ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
        pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_160mhz.bit_max_he_mcs_for_5ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
        pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_160mhz.bit_max_he_mcs_for_6ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
        pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_160mhz.bit_max_he_mcs_for_7ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
        pst_he_tx_rx_mcs_nss_info->st_rx_he_mcs_160mhz.bit_max_he_mcs_for_8ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;

        uc_len += OAL_SIZEOF(mac_frame_he_mcs_nss_bit_map_stru);

        /* Tx HE-MCS Map 160 MHz */
        pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_160mhz.bit_max_he_mcs_for_1ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
        pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_160mhz.bit_max_he_mcs_for_2ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
        if (WLAN_DOUBLE_NSS == MAC_DEVICE_GET_NSS_NUM(pst_mac_dev)) {
            pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_160mhz.bit_max_he_mcs_for_2ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
        }
        pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_160mhz.bit_max_he_mcs_for_3ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
        pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_160mhz.bit_max_he_mcs_for_4ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
        pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_160mhz.bit_max_he_mcs_for_5ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
        pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_160mhz.bit_max_he_mcs_for_6ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
        pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_160mhz.bit_max_he_mcs_for_7ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
        pst_he_tx_rx_mcs_nss_info->st_tx_he_mcs_160mhz.bit_max_he_mcs_for_8ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;

        uc_len += OAL_SIZEOF(mac_frame_he_mcs_nss_bit_map_stru);
    }

    if (uc_channel_width_b3 != 0) {
        /* TRx HE-MCS Map 80+80 MHz: 05暂不支持 */
    }

    return uc_len;
}


OAL_STATIC oal_uint8 mac_set_he_ppe_thresholds_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    oal_uint8 uc_len;
    mac_frame_ppe_thresholds_pre_field_stru *pst_ppe_thresholds;

    pst_ppe_thresholds = (mac_frame_ppe_thresholds_pre_field_stru *)puc_buffer;

    memset_s(pst_ppe_thresholds, OAL_SIZEOF(mac_frame_ppe_thresholds_pre_field_stru),
             0, OAL_SIZEOF(mac_frame_ppe_thresholds_pre_field_stru));

    pst_ppe_thresholds->bit_nss = 1;            /* 双流 */
    pst_ppe_thresholds->bit_ru_index_mask = 0xC; /* 表示160M带宽接收HE SU PPDU只支持PE=16us */

    pst_ppe_thresholds->bit_ppet16_nss0_ru0 = 0; /* 表示任何mcs都需要16us */
    pst_ppe_thresholds->bit_ppet8_nss0_ru0 = 7;  /* PPET8设置为none */
    pst_ppe_thresholds->bit_ppet16_nss0_ru1 = 0;
    pst_ppe_thresholds->bit_ppet8_nss0_ru1 = 7;
    pst_ppe_thresholds->bit_ppet16_nss1_ru0 = 0;
    pst_ppe_thresholds->bit_ppet8_nss1_ru0 = 7;
    pst_ppe_thresholds->bit_ppet16_nss1_ru1 = 0;
    pst_ppe_thresholds->bit_ppet8_nss1_ru1 = 7;

    uc_len = OAL_SIZEOF(mac_frame_ppe_thresholds_pre_field_stru);

    return uc_len;
}


/*lint -save -e438 */
oal_void mac_set_he_capabilities_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    oal_uint8 *puc_ie_length = OAL_PTR_NULL;
    oal_uint8 uc_info_length;
    mac_vap_rom_stru *pst_mac_vap_rom = OAL_PTR_NULL;
    mac_frame_he_phy_cap_stru *pst_he_phy_capinfo = OAL_PTR_NULL;
    oal_uint8 uc_channel_width_b2;
    oal_uint8 uc_channel_width_b3;

    *puc_ie_len = 0;

    if (OAL_TRUE != mac_mib_get_HEOptionImplemented(pst_mac_vap)) {
        return;
    }

    /* PF认证要求对于WEP、TKIP 加密方式不能关联在HE模式 */
    if ((OAL_TRUE != mac_mib_get_HEOptionImplemented(pst_mac_vap)) ||
        ((OAL_TRUE == mac_is_wep_enabled(pst_mac_vap)) ||
        (OAL_TRUE == mac_is_tkip_only(pst_mac_vap)))) {
        *puc_ie_len = 0;
        return;
    }

    pst_mac_vap_rom = (mac_vap_rom_stru *)pst_mac_vap->_rom;

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID |Length |EID Extension|HE MAC Capa. Info |HE PHY Capa. Info|
    -------------------------------------------------------------------------
    |1   |1          |1                  |5                         |9                       |
    -------------------------------------------------------------------------
    |Tx Rx HE MCS NSS Support |PPE Thresholds(Optional)|
    -------------------------------------------------------------------------
    |2 or More                         |Variable                       |
    -------------------------------------------------------------------------
    ***************************************************************************/
    *puc_buffer = MAC_EID_HE;
    puc_ie_length = puc_buffer + 1;

    puc_buffer += MAC_IE_HDR_LEN;
    *puc_ie_len += MAC_IE_HDR_LEN;

    *puc_buffer = MAC_EID_EXT_HE_CAP;
    puc_buffer += 1;

    *puc_ie_len += 1;

    /* 填充HE mac capabilities information域信息 */
    mac_set_he_mac_capinfo_field(pst_vap, puc_buffer);
    puc_buffer += MAC_HE_MAC_CAP_LEN;
    *puc_ie_len += MAC_HE_MAC_CAP_LEN;

    /* 填充HE PHY Capabilities Information 域信息 */
    pst_he_phy_capinfo = (mac_frame_he_phy_cap_stru *)puc_buffer;
    mac_set_he_phy_capinfo_field(pst_vap, puc_buffer);
    puc_buffer += MAC_HE_PHY_CAP_LEN;
    *puc_ie_len += MAC_HE_PHY_CAP_LEN;

    /* 填充 HE tx rx he mcs nss support */
    uc_channel_width_b2 = pst_he_phy_capinfo->bit_channel_width_set & BIT2;
    uc_channel_width_b3 = pst_he_phy_capinfo->bit_channel_width_set & BIT3;
    uc_info_length = mac_set_he_tx_rx_mcs_nss_field(pst_vap, uc_channel_width_b2, uc_channel_width_b3, puc_buffer);
    puc_buffer += uc_info_length;
    *puc_ie_len += uc_info_length;

    /* 填充 PPE Thresholds field */
    if (mac_mib_get_PPEThresholdsRequired(pst_mac_vap_rom)) {
        uc_info_length = mac_set_he_ppe_thresholds_field(pst_vap, puc_buffer);
        puc_buffer += uc_info_length;
        *puc_ie_len += uc_info_length;
    }
    *puc_ie_length = *puc_ie_len - MAC_IE_HDR_LEN;
}

OAL_STATIC OAL_INLINE oal_void mac_set_he_nss_bit_map_para(
    mac_frame_he_mcs_nss_bit_map_stru *pst_he_mcs_nss_bit_map)
{
    pst_he_mcs_nss_bit_map->bit_max_he_mcs_for_1ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
    pst_he_mcs_nss_bit_map->bit_max_he_mcs_for_2ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    pst_he_mcs_nss_bit_map->bit_max_he_mcs_for_3ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    pst_he_mcs_nss_bit_map->bit_max_he_mcs_for_4ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    pst_he_mcs_nss_bit_map->bit_max_he_mcs_for_5ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    pst_he_mcs_nss_bit_map->bit_max_he_mcs_for_6ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    pst_he_mcs_nss_bit_map->bit_max_he_mcs_for_7ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    pst_he_mcs_nss_bit_map->bit_max_he_mcs_for_8ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
}


oal_void mac_set_he_operation_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru                         *pst_mac_vap   = (mac_vap_stru *)pst_vap;
    oal_uint8                            *puc_ie_length = OAL_PTR_NULL;
    mac_frame_he_operation_param_stru    *pst_he_opern_param = OAL_PTR_NULL;
    mac_frame_he_bss_color_info_stru     *pst_he_bss_color = OAL_PTR_NULL;
    mac_frame_he_mcs_nss_bit_map_stru    *pst_he_mcs_nss_bit_map = OAL_PTR_NULL;

    *puc_ie_len = 0;

    /* PF认证要求对于WEP、TKIP 加密方式不能关联在HE模式 */
    if ((OAL_TRUE != mac_mib_get_HEOptionImplemented(pst_mac_vap)) ||
       ((OAL_TRUE == mac_is_wep_enabled(pst_mac_vap)) || (OAL_TRUE == mac_is_tkip_only(pst_mac_vap)))) {
        *puc_ie_len = 0;
        return;
    }

    /*******************HE Operation element**********************************
    --------------------------------------------------------------------
    |EID |Length |EID Extension|HE Operation Parameters |BSS Color Info|
    --------------------------------------------------------------------
    |1   |1      |1            |3                       |1             |
    --------------------------------------------------------------------
    |Basic HE-MCS |VHT Operation |Max Co-Hosted    |6 GHz Operation    |
    |And NSS Set  |Info          |BSSID Indicator  |Info               |
    --------------------------------------------------------------------
    |2            |0 or 3        | 0 or 1          |0 or 4             |
    --------------------------------------------------------------------
    ***************************************************************************/
    *puc_buffer    = MAC_EID_HE;
    puc_ie_length  = puc_buffer + 1;

    puc_buffer    += MAC_IE_HDR_LEN;
    *puc_ie_len   += MAC_IE_HDR_LEN;

    *puc_buffer    = MAC_EID_EXT_HE_OPERATION;
    puc_buffer    += 1;
    *puc_ie_len   += 1;

    pst_he_opern_param = (mac_frame_he_operation_param_stru *)puc_buffer;
    /*******************HE Operation Parameters********************************
    --------------------------------------------------------------------
    |Default PE |TWT      |TXOP Duration |VHT Operation |Co-Hosted |
    |Duration   |Required |RTS Threshold |Info. Present |BSS       |
    --------------------------------------------------------------------
    |3          |1        |10            |1             |1         |
    --------------------------------------------------------------------
    |ER SU   |6 GHz Operation |Reserved |
    |Disable |Info. Present   |         |
    -------------------------------------
    |1       |1               |6        |
    -------------------------------------
    ***************************************************************************/
    /* B0-B2:Default PE Duration, 不支持，5-7reserved */
    pst_he_opern_param->bit_default_pe_duration = MAC_HE_DEFAULT_PE_DURATION;

    /* B3:TWT Required */
    pst_he_opern_param->bit_twt_required = 0;

    /* B4-B13:TXOP Duration RTS Threshold, 1023表示该字段不可用 */
    pst_he_opern_param->bit_he_duration_rts_threshold = MAC_HE_DURATION_RTS_THRESHOLD;

    /* B14:VHT Operation Info. Present */
    /* 由于beacon和probe rsp中一定携带VHT信息，故此位置0 */
    pst_he_opern_param->bit_vht_operation_info_present = 0;

    /* B15:Co-Hosted BSS */
    pst_he_opern_param->bit_co_located_bss = 0;

    /* B16:ER SU Disable的取值，0表示支持接收242-tone HE ER SU PPDU */
    pst_he_opern_param->bit_er_su_disable = 0;

    /* B17:6 GHz Operation Info. Present的取值, 不支持6GHz band */
    /* 此位置0,表示HE Operation中6GHz Operation Info字段不存在 */
    pst_he_opern_param->bit_6g_oper_info_present = 0;

    /* B18-B23保留 */
    pst_he_opern_param->bit_reserved = 0;

    puc_buffer    += MAC_HE_OPE_PARAM_LEN;
    *puc_ie_len   += MAC_HE_OPE_PARAM_LEN;

    pst_he_bss_color = (mac_frame_he_bss_color_info_stru *)puc_buffer;
    memset_s(pst_he_bss_color, OAL_SIZEOF(mac_frame_he_bss_color_info_stru),
             0, OAL_SIZEOF(mac_frame_he_bss_color_info_stru));
    /*******************BSS Color Info*********************
    -------------------------------------------------------
    |BSS Color |Partial BSS Color    |BSS Color Disabled |
    -------------------------------------------------------
    |6         |1                    |1                  |
    -------------------------------------------------------
    *******************************************************/
    /* 暂不支持bss color */
    pst_he_bss_color->bit_bss_color_disable = 1;
    puc_buffer    += 1;
    *puc_ie_len   += 1;

    /* 填充 Basic HE-MCS And NSS Set, 此字段为AP的基础能力, 适配支持接入AP的最小能力为单流MCS7 */
    pst_he_mcs_nss_bit_map = (mac_frame_he_mcs_nss_bit_map_stru *)puc_buffer;
    memset_s(pst_he_mcs_nss_bit_map, OAL_SIZEOF(mac_frame_he_mcs_nss_bit_map_stru),
             0, OAL_SIZEOF(mac_frame_he_mcs_nss_bit_map_stru));

    mac_set_he_nss_bit_map_para(pst_he_mcs_nss_bit_map); // 封装函数降低行数

    puc_buffer    += MAC_HE_OPE_BASIC_MCS_NSS_LEN;
    *puc_ie_len   += MAC_HE_OPE_BASIC_MCS_NSS_LEN;

    *puc_ie_length = *puc_ie_len - MAC_IE_HDR_LEN;

}


oal_void mac_get_htc_uph_om_value(oal_uint8 uc_nss, oal_uint8 uc_bw,
    oal_uint8 uc_mimo_resound, oal_uint8 uc_ul_mu_disable, oal_uint32 *pul_htc_value)
{
    mac_htc_a_control_field_union   *pst_he_om_info;
    wlan_bw_cap_enum_uint8           en_bw_cap;

    en_bw_cap = WLAN_BANDWIDTH_TO_BW_CAP(uc_bw);

    *pul_htc_value = HTC_INVALID_VALUE;
    pst_he_om_info = (mac_htc_a_control_field_union *)pul_htc_value;

    pst_he_om_info->un_a_control_info.st_uph_om_info.bit_vht_flag             = 1;
    pst_he_om_info->un_a_control_info.st_uph_om_info.bit_he_flag              = 1;

    pst_he_om_info->un_a_control_info.st_uph_om_info.bit_uph_id               = MAC_HTC_A_CONTROL_TYPE_UPH;
    pst_he_om_info->un_a_control_info.st_uph_om_info.bit_ul_power_headroom    = 0;
    pst_he_om_info->un_a_control_info.st_uph_om_info.bit_min_transmit_power_flag    = 0;
    pst_he_om_info->un_a_control_info.st_uph_om_info.bit_uph_rsv    = 0;

    pst_he_om_info->un_a_control_info.st_uph_om_info.bit_om_id                = MAC_HTC_A_CONTROL_TYPE_OM;
    pst_he_om_info->un_a_control_info.st_uph_om_info.bit_rx_nss               = uc_nss;
    pst_he_om_info->un_a_control_info.st_uph_om_info.bit_channel_width        = en_bw_cap;
    pst_he_om_info->un_a_control_info.st_uph_om_info.bit_ul_mu_disable        = uc_ul_mu_disable;
    pst_he_om_info->un_a_control_info.st_uph_om_info.bit_tx_nsts              = uc_nss;
    pst_he_om_info->un_a_control_info.st_uph_om_info.bit_er_su_disable        = 0;
    pst_he_om_info->un_a_control_info.st_uph_om_info.bit_dl_mu_mimo_resound_recommendation  = uc_mimo_resound;
    pst_he_om_info->un_a_control_info.st_uph_om_info.bit_ul_mu_data_disable   = 0;
}


oal_void mac_get_htc_om_value(oal_uint8 uc_nss, oal_uint8 uc_bw,
    oal_uint8 uc_mimo_resound, oal_uint8 uc_ul_mu_disable, oal_uint32 *pul_htc_value)
{
    mac_htc_a_control_field_union   *pst_he_om_info;
    wlan_bw_cap_enum_uint8           en_bw_cap;

    en_bw_cap = WLAN_BANDWIDTH_TO_BW_CAP(uc_bw);

    *pul_htc_value = HTC_INVALID_VALUE;
    pst_he_om_info = (mac_htc_a_control_field_union *)pul_htc_value;

    pst_he_om_info->un_a_control_info.st_om_info.bit_vht_flag             = 1;
    pst_he_om_info->un_a_control_info.st_om_info.bit_he_flag              = 1;
    pst_he_om_info->un_a_control_info.st_om_info.bit_om_id                = MAC_HTC_A_CONTROL_TYPE_OM;
    pst_he_om_info->un_a_control_info.st_om_info.bit_rx_nss               = uc_nss;
    pst_he_om_info->un_a_control_info.st_om_info.bit_channel_width        = en_bw_cap;
    pst_he_om_info->un_a_control_info.st_om_info.bit_ul_mu_disable        = uc_ul_mu_disable;
    pst_he_om_info->un_a_control_info.st_om_info.bit_tx_nsts              = uc_nss;
    pst_he_om_info->un_a_control_info.st_om_info.bit_er_su_disable        = 0;
    pst_he_om_info->un_a_control_info.st_om_info.bit_dl_mu_mimo_resound_recommendation  = uc_mimo_resound;
    pst_he_om_info->un_a_control_info.st_om_info.bit_ul_mu_data_disable   = 0;
}


oal_void mac_set_he_ie_in_assoc_rsp(oal_void *pst_mac_ap, oal_uint16 us_assoc_id,
    oal_uint8 *puc_asoc_rsp, oal_uint8 *puc_ie_len)
{
    mac_user_stru     *pst_mac_user = OAL_PTR_NULL;
    oal_uint8          uc_ie_len    = 0;
    mac_vap_stru      *p_mac_ap = (mac_vap_stru *)pst_mac_ap;
    pst_mac_user = mac_res_get_mac_user_etc(us_assoc_id);

    *puc_ie_len = 0;
    /* ap 没有工作ax, 就不携带he cap 了 */
    if (!MAC_VAP_IS_WORK_HE_PROTOCOL(p_mac_ap)) {
        return ;
    }

    if ((pst_mac_user != OAL_PTR_NULL) && (mac_user_get_he_capable(pst_mac_user) == OAL_TRUE) &&
        ((g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_11ax_aput_he_cap_switch == OAL_TRUE) ||
        (pst_mac_user->st_cap_info.bit_support_he_cap == OAL_TRUE))) {
        /* 设置 HE Capabilities IE */
        mac_set_he_capabilities_ie(pst_mac_ap, puc_asoc_rsp, &uc_ie_len);
        puc_asoc_rsp += uc_ie_len;
        *puc_ie_len  += uc_ie_len;

        /* 设置 HE Operation IE */
        mac_set_he_operation_ie(pst_mac_ap, puc_asoc_rsp, &uc_ie_len);
        *puc_ie_len  += uc_ie_len;
    }
}



oal_void mac_set_he_ie_in_beacon(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    oal_uint8          uc_ie_len    = 0;
    mac_vap_stru      *p_mac_ap = (mac_vap_stru *)pst_vap;

    *puc_ie_len = 0;

    if (!MAC_VAP_IS_WORK_HE_PROTOCOL(p_mac_ap)) {
        return ;
    }

    if (g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_11ax_aput_he_cap_switch == OAL_TRUE) {
        /* 设置 HE Capabilities IE */
        mac_set_he_capabilities_ie(pst_vap, puc_buffer, &uc_ie_len);
        puc_buffer += uc_ie_len;
        *puc_ie_len  += uc_ie_len;

        /* 设置 HE Operation IE */
        mac_set_he_operation_ie(pst_vap, puc_buffer, &uc_ie_len);
        puc_buffer += uc_ie_len;
        *puc_ie_len  += uc_ie_len;
    }
}
/*lint -restore */
#endif


oal_void mac_set_hisi_cap_vendor_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_ieee80211_vendor_ie_stru *pst_vendor_ie = OAL_PTR_NULL;
    mac_hisi_cap_vendor_ie_stru *pst_vendor_ie_param = OAL_PTR_NULL;

    pst_vendor_ie = (mac_ieee80211_vendor_ie_stru *)puc_buffer;
    pst_vendor_ie->uc_element_id = MAC_EID_VENDOR;
    pst_vendor_ie->uc_len = MAC_HISI_CAP_VENDOR_IE_LEN - MAC_IE_HDR_LEN;

    pst_vendor_ie->uc_oui_type = MAC_HISI_CAP_IE;

    pst_vendor_ie->auc_oui[0] = (oal_uint8)((MAC_HUAWEI_VENDER_IE >> 16) & 0xff);
    pst_vendor_ie->auc_oui[1] = (oal_uint8)((MAC_HUAWEI_VENDER_IE >> 8) & 0xff);
    pst_vendor_ie->auc_oui[2] = (oal_uint8)((MAC_HUAWEI_VENDER_IE)&0xff);

    puc_buffer += OAL_SIZEOF(mac_ieee80211_vendor_ie_stru);
    pst_vendor_ie_param = (mac_hisi_cap_vendor_ie_stru *)puc_buffer;
    memset_s(pst_vendor_ie_param, OAL_SIZEOF(mac_hisi_cap_vendor_ie_stru), 0,
        OAL_SIZEOF(mac_hisi_cap_vendor_ie_stru));
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_11ax_aput_he_cap_switch != OAL_TRUE) {
        pst_vendor_ie_param->bit_11ax_support = OAL_TRUE;
    }
#endif
#ifdef _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
    pst_vendor_ie_param->bit_dcm_support = OAL_TRUE;
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_vendor_ie_param->bit_p2p_csa_support = OAL_TRUE;
#endif
    *puc_ie_len = OAL_SIZEOF(mac_ieee80211_vendor_ie_stru) + OAL_SIZEOF(mac_hisi_cap_vendor_ie_stru);
}

oal_uint32 mac_get_hisi_cap_vendor_ie(oal_uint8 *puc_payload, oal_uint32 ul_msg_len,
    mac_hisi_cap_vendor_ie_stru *pst_hisi_cap_ie)
{
    oal_uint8 *puc_ie_tmp = OAL_PTR_NULL;
    mac_hisi_cap_vendor_ie_stru *pst_vendor_ie_param = OAL_PTR_NULL;

    puc_ie_tmp = mac_find_vendor_ie_etc(MAC_HUAWEI_VENDER_IE, MAC_HISI_CAP_IE, puc_payload, (oal_int32)ul_msg_len);
    if ((puc_ie_tmp != OAL_PTR_NULL) && (puc_ie_tmp[1] >= (MAC_HISI_CAP_VENDOR_IE_LEN - MAC_IE_HDR_LEN))) {
        puc_ie_tmp += MAC_HISI_CAP_VENDOR_IE_LEN - 1;
        pst_vendor_ie_param = (mac_hisi_cap_vendor_ie_stru *)puc_ie_tmp;
        memcpy_s(pst_hisi_cap_ie, OAL_SIZEOF(mac_hisi_cap_vendor_ie_stru),
            pst_vendor_ie_param, OAL_SIZEOF(mac_hisi_cap_vendor_ie_stru));
        return OAL_SUCC;
    }

    return OAL_FAIL;
}


oal_uint32 mac_vap_set_cb_tx_user_idx(mac_vap_stru *pst_mac_vap,
                                      mac_tx_ctl_stru *pst_tx_ctl,
                                      const unsigned char *puc_data)
{
    oal_uint16 us_user_idx = MAC_INVALID_USER_ID;
    oal_uint32 ul_ret;

    ul_ret = mac_vap_find_user_by_macaddr_etc(pst_mac_vap, puc_data, &us_user_idx);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{mac_vap_set_cb_tx_user_idx:: cannot find user_idx from xx:xx:xx:%x:%x:%x, set TX_USER_IDX %d.}",
            puc_data[3], puc_data[4], puc_data[5], MAC_INVALID_USER_ID);
        MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = MAC_INVALID_USER_ID;
        return ul_ret;
    }

    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = us_user_idx;

    return OAL_SUCC;
}
/*lint -e578*/ /*lint -e19*/
oal_module_symbol(mac_vap_set_cb_tx_user_idx);
/*lint -e578*/ /*lint -e19*/

#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE

oal_void mac_set_ie_field(oal_void *pst_data, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    oal_uint8 uc_index = 0;
    oal_uint8 *puc_ie_buff = OAL_PTR_NULL;
    mac_vap_ie_set_stru *pst_set_ie_info = OAL_PTR_NULL;
    oal_int32 l_ret = EOK;

    /* vap下对应的ie设置指针不为空则需要设置 */
    if (pst_data == OAL_PTR_NULL) {
        *puc_ie_len = 0;
        return;
    }
    pst_set_ie_info = (mac_vap_ie_set_stru *)pst_data;
    puc_ie_buff = pst_set_ie_info->auc_ie_content;
    /* 根据ie设置类型进行IE设置 */
    switch (pst_set_ie_info->en_set_type) {
        case OAL_IE_SET_TYPE_AND: /* 与操作 */
        {
            if (*puc_ie_len == 0) { /* 长度为0 说明帧不包含该IE 无法与操作 */
                return;
            }
            /* 与操作 */
            for (uc_index = 0; uc_index < pst_set_ie_info->us_ie_content_len; uc_index++) {
                /* 从IE的content 开始进行与操作 */
                puc_buffer[MAC_IE_HDR_LEN + uc_index] = puc_buffer[MAC_IE_HDR_LEN + uc_index] & puc_ie_buff[uc_index];
            }
            *puc_ie_len = 0; /* 没有新增BUFF DATA 将长度置0 */
            return;
        }
        case OAL_IE_SET_TYPE_OR: {
            if (*puc_ie_len == 0) { /* 长度为0 说明帧不包含该IE 则添加IE到BUFF */
                puc_buffer[0] = pst_set_ie_info->en_eid;
                puc_buffer[1] = (oal_uint8)pst_set_ie_info->us_ie_content_len;
                l_ret += memcpy_s(puc_buffer + MAC_IE_HDR_LEN, pst_set_ie_info->us_ie_content_len,
                                  puc_ie_buff, pst_set_ie_info->us_ie_content_len);
                *puc_ie_len = (oal_uint8)(pst_set_ie_info->us_ie_content_len) + MAC_IE_HDR_LEN; /* 新增IE 修改长度 */
            } else {
                /* 或操作 */
                for (uc_index = 0; uc_index < pst_set_ie_info->us_ie_content_len; uc_index++) {
                    /* 从IE的content 开始进行或操作 */
                    puc_buffer[MAC_IE_HDR_LEN + uc_index] =
                        puc_buffer[MAC_IE_HDR_LEN + uc_index] | puc_ie_buff[uc_index];
                }
                *puc_ie_len = 0; /* 没有新增BUFF DATA 将长度置0 */
            }
            return;
        }
        case OAL_IE_SET_TYPE_ADD: {
            if (*puc_ie_len == 0) { /* 长度为0 说明帧不包含该IE 则添加IE到BUFF */
                puc_buffer[0] = pst_set_ie_info->en_eid;
                puc_buffer[1] = (oal_uint8)pst_set_ie_info->us_ie_content_len;
                l_ret += memcpy_s(puc_buffer + MAC_IE_HDR_LEN, pst_set_ie_info->us_ie_content_len,
                                  puc_ie_buff, pst_set_ie_info->us_ie_content_len);
                *puc_ie_len = (oal_uint8)(pst_set_ie_info->us_ie_content_len) + MAC_IE_HDR_LEN; /* 新增IE 修改长度 */
            } else { /* 存在该IE 则替换现有IE的内容 */
                l_ret += memcpy_s(puc_buffer + MAC_IE_HDR_LEN, pst_set_ie_info->us_ie_content_len,
                                  puc_ie_buff, pst_set_ie_info->us_ie_content_len);
                *puc_ie_len = 0; /* 没有新增BUFF DATA 将长度置0 */
            }
            return;
        }
        default: /* 其他设置类型不支持 直接返回 */
            *puc_ie_len = 0;
            return;
    }
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_set_ie_field::memcpy fail!");
    }
}

#endif  // end of _PRE_WLAN_FEATURE_11KV_INTERFACE

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA

oal_void mac_set_vender_4addr_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    *puc_ie_len = 0;

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID | Length |HUAWEI OUI |WIFI OUT FOUR ADDR |HE PHY Capa. Info| Version |
    -------------------------------------------------------------------------
    |221 |variable|     3     |         4         |        待定     |         |
    -------------------------------------------------------------------------
    ***************************************************************************/
    /* 直接调用11KV接口的设置IE接口 宏未定义编译失败 */
#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE
    mac_set_ie_field(pst_mac_vap->pst_msta_ie_info, puc_buffer, puc_ie_len);
#else
#error "the virtual multi-sta feature dependent _PRE_WLAN_FEATURE_11KV_INTERFACE, please define it!"
#endif
}
#endif

#ifdef _PRE_WLAN_FEATURE_MBO

OAL_STATIC oal_void mac_set_mbo_cell_para(oal_void *pst_vap, oal_uint8 *puc_buffer,
    oal_uint8 *puc_ie_len, oal_uint8 uc_mbo_cell_capa)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_vap_rom_stru *pst_mac_vap_rom;
    mac_ieee80211_vendor_ie_stru *pst_vendor_ie = OAL_PTR_NULL;

    pst_mac_vap_rom = (mac_vap_rom_stru *)(pst_mac_vap->_rom);

    pst_mac_vap_rom->st_mbo_para_info.uc_mbo_cell_capa = uc_mbo_cell_capa;

    pst_vendor_ie = (mac_ieee80211_vendor_ie_stru *)puc_buffer;
    /* 设置MBO元素的OUI和OUI type = 0x16 */
    pst_vendor_ie->uc_element_id = MAC_EID_VENDOR;
    pst_vendor_ie->uc_len = 7;
    pst_vendor_ie->auc_oui[0] = (oal_uint8)((MAC_MBO_VENDOR_IE >> 16) & 0xff);
    pst_vendor_ie->auc_oui[1] = (oal_uint8)((MAC_MBO_VENDOR_IE >> 8) & 0xff);
    pst_vendor_ie->auc_oui[2] = (oal_uint8)((MAC_MBO_VENDOR_IE)&0xff);
    pst_vendor_ie->uc_oui_type = MAC_MBO_IE_OUI_TYPE;

    puc_buffer[6] = MBO_ATTR_ID_CELL_DATA_CAPA;
    puc_buffer[7] = 1;
    puc_buffer[8] = uc_mbo_cell_capa;

    *puc_ie_len = pst_vendor_ie->uc_len + MAC_IE_HDR_LEN;
}


oal_void mac_set_mbo_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    /***************************************************************************
                         --------------------------------------------------
                         |Element ID |Length |OUI|OUI Type|MBO Attributes |
                         --------------------------------------------------
          Octets:        |1          |1      |3  |1       |n              |
                         --------------------------------------------------
    ***************************************************************************/
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_vap_rom_stru *pst_mac_vap_rom;
    oal_uint8 uc_mbo_ie_len = 0;

    pst_mac_vap_rom = (mac_vap_rom_stru *)(pst_mac_vap->_rom);

    /* 若MBO定制化开关未打开，不带MBO IE 函数return */
    if (pst_mac_vap_rom->st_mbo_para_info.uc_mbo_enable != OAL_TRUE) {
        *puc_ie_len = 0;
        return;
    } else {
        mac_set_mbo_cell_para(pst_vap, puc_buffer, &uc_mbo_ie_len, pst_mac_vap_rom->st_mbo_para_info.uc_mbo_cell_capa);
        *puc_ie_len = uc_mbo_ie_len;
    }
}


oal_void mac_set_mbo_ie_ap_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    /***************************************************************************
                         --------------------------------------------------
                         |Element ID |Length |OUI|OUI Type|MBO Attributes |
                         --------------------------------------------------
          Octets:        |1          |1      |3  |1       |n              |
                         --------------------------------------------------
    ***************************************************************************/
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_vap_rom_stru *pst_mac_vap_rom;
    mac_ieee80211_vendor_ie_stru *pst_vendor_ie = OAL_PTR_NULL;

    pst_mac_vap_rom = (mac_vap_rom_stru *)(pst_mac_vap->_rom);

    /* 若MBO定制化开关未打开，不带MBO IE或测试开关未开 函数return */
    if ((pst_mac_vap_rom->st_mbo_para_info.uc_mbo_enable != OAL_TRUE) ||
        (pst_mac_vap_rom->st_mbo_para_info.uc_mbo_assoc_disallowed_test_switch != OAL_TRUE)) {
        *puc_ie_len = 0;
        return;
    } else {
        pst_vendor_ie = (mac_ieee80211_vendor_ie_stru *)puc_buffer;
        /* 设置MBO元素的OUI和OUI type = 0x16 */
        pst_vendor_ie->uc_element_id = MAC_EID_VENDOR;
        pst_vendor_ie->uc_len = 7;
        pst_vendor_ie->auc_oui[0] = (oal_uint8)((MAC_MBO_VENDOR_IE >> 16) & 0xff);
        pst_vendor_ie->auc_oui[1] = (oal_uint8)((MAC_MBO_VENDOR_IE >> 8) & 0xff);
        pst_vendor_ie->auc_oui[2] = (oal_uint8)((MAC_MBO_VENDOR_IE)&0xff);
        pst_vendor_ie->uc_oui_type = MAC_MBO_IE_OUI_TYPE;

        puc_buffer[6] = MBO_ATTR_ID_ASSOC_DISALLOW;
        puc_buffer[7] = 1;   /* length */
        puc_buffer[8] = 0x0; /* reason code */
        *puc_ie_len = pst_vendor_ie->uc_len + MAC_IE_HDR_LEN;
    }
}


oal_void mac_set_supported_operating_classes_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    /*******************************************************************************************
            ------------------------------------------------------------------------------------
            |ElementID | Length | Current Operating Class| Operating Class |
            ------------------------------------------------------------------------------------
    Octets: |1         | 1      | 1                      | variable        |
            ------------------------------------------------------------------------------------
            |Current Operating Class Extension Sequence(O) | Operating Class Duple Sequence(O) |
            ------------------------------------------------------------------------------------
    Octets: |              variable                        |             variable              |
            ------------------------------------------------------------------------------------

    *********************************************************************************************/
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_vap_rom_stru *pst_mac_vap_rom;

    pst_mac_vap_rom = (mac_vap_rom_stru *)(pst_mac_vap->_rom);

    /* 若MBO 定制化开关未打开，不带Supported Operating Classed IE */
    if (pst_mac_vap_rom->st_mbo_para_info.uc_mbo_enable != OAL_TRUE) {
        *puc_ie_len = 0;
        return;
    } else {
        puc_buffer[0] = MAC_EID_OPERATING_CLASS;
        puc_buffer[1] = 3;
        if (MAC_GET_VAP_BAND(pst_mac_vap) == WLAN_BAND_2G) {
            puc_buffer[2] = MAC_GLOBAL_OPERATING_CLASS_NUMBER81;
        }
        if (MAC_GET_VAP_BAND(pst_mac_vap) == WLAN_BAND_5G) {
            puc_buffer[2] = MAC_GLOBAL_OPERATING_CLASS_NUMBER115;
        }
        puc_buffer[3] = MAC_GLOBAL_OPERATING_CLASS_NUMBER81;
        puc_buffer[4] = MAC_GLOBAL_OPERATING_CLASS_NUMBER115;

        *puc_ie_len = MAC_IE_HDR_LEN + 3;
    }
}


oal_uint8 *mac_find_mbo_attribute_etc(oal_uint8 uc_eid, oal_uint8 *puc_ies, oal_int32 l_len)
{
    if (puc_ies == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    /* 查找MBO Attribute，如果不是直接找下一个 */
    while (l_len > MAC_MBO_ATTRIBUTE_HDR_LEN && puc_ies[0] != uc_eid) {
        l_len -= puc_ies[1] + MAC_MBO_ATTRIBUTE_HDR_LEN;
        puc_ies += puc_ies[1] + MAC_MBO_ATTRIBUTE_HDR_LEN;
    }

    /* 查找到MBO Attribute，剩余长度不匹配直接返回空指针 */
    if ((l_len < MAC_MBO_ATTRIBUTE_HDR_LEN) || (l_len < (MAC_MBO_ATTRIBUTE_HDR_LEN + puc_ies[1]))) {
        return OAL_PTR_NULL;
    }

    return puc_ies;
}
#endif


oal_void mac_ftm_add_to_ext_capabilities_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
#ifdef _PRE_WLAN_FEATURE_FTM
    mac_vap_stru               *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_ext_cap_ftm_ie_stru    *pst_ext_cap_ftm = OAL_PTR_NULL;

    if ((OAL_FALSE == mac_mib_get_FtmInitiatorModeActivated(pst_mac_vap)) &&
        (OAL_FALSE == mac_mib_get_FtmResponderModeActivated(pst_mac_vap))) {
        return;
    }

    puc_buffer[1] = MAC_XCAPS_EX_FTM_LEN;
    pst_ext_cap_ftm = (mac_ext_cap_ftm_ie_stru *)(puc_buffer + MAC_IE_HDR_LEN);
    pst_ext_cap_ftm->bit_ftm_responder = mac_mib_get_FtmResponderModeActivated(pst_mac_vap);
    pst_ext_cap_ftm->bit_ftm_initiator = mac_mib_get_FtmInitiatorModeActivated(pst_mac_vap);

    (*puc_ie_len)++;
#endif
}

#ifdef _PRE_WLAN_FEATURE_1024QAM

oal_void mac_set_1024qam_vendor_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_ieee80211_vendor_ie_stru *pst_vendor_ie = OAL_PTR_NULL;

    if (pst_mac_vap->st_cap_flag.bit_1024qam != OAL_TRUE) {
        *puc_ie_len = 0;
        return;
    }

    pst_vendor_ie = (mac_ieee80211_vendor_ie_stru *)puc_buffer;
    pst_vendor_ie->uc_element_id = MAC_EID_VENDOR;
    pst_vendor_ie->uc_len = sizeof(mac_ieee80211_vendor_ie_stru) - MAC_IE_HDR_LEN;

    pst_vendor_ie->uc_oui_type = MAC_HISI_1024QAM_IE;

    pst_vendor_ie->auc_oui[0] = (oal_uint8)((MAC_HUAWEI_VENDER_IE >> 16) & 0xff);
    pst_vendor_ie->auc_oui[1] = (oal_uint8)((MAC_HUAWEI_VENDER_IE >> 8) & 0xff);
    pst_vendor_ie->auc_oui[2] = (oal_uint8)((MAC_HUAWEI_VENDER_IE)&0xff);

    *puc_ie_len = OAL_SIZEOF(mac_ieee80211_vendor_ie_stru);
}
#endif

#ifdef _PRE_WLAN_FEATURE_PRIV_CLOSED_LOOP_TPC

oal_void mac_set_adjust_pow_vendor_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    /*******************************************************************************************
            ------------------------------------------------------------------------------------
            |ElementID | Length |          OUI           | OUI TYPE |Supported adjust pow|
            ------------------------------------------------------------------------------------
    Octets: |1         | 1      | 3                      | 1        | 1                  |
            ------------------------------------------------------------------------------------
    *********************************************************************************************/
    mac_hisi_adjust_pow_ie_stru *pst_vendor_ie;

    pst_vendor_ie = (mac_hisi_adjust_pow_ie_stru *)puc_buffer;
    pst_vendor_ie->uc_id = MAC_EID_ADJUST_POW_PRIVATE;
    pst_vendor_ie->uc_len = sizeof(mac_hisi_adjust_pow_ie_stru) - MAC_IE_HDR_LEN;
    pst_vendor_ie->uc_ouitype = MAC_HISI_ADJUST_POW_IE;

    pst_vendor_ie->auc_oui[0] = g_auc_huawei_oui[0];
    pst_vendor_ie->auc_oui[1] = g_auc_huawei_oui[1];
    pst_vendor_ie->auc_oui[2] = g_auc_huawei_oui[2];
    /* Supported adjust pow中4表示降低4dB，目前只支持0档和2档(降低8dB) */
    pst_vendor_ie->uc_adjust_pow = 8;

    *puc_ie_len = OAL_SIZEOF(mac_hisi_adjust_pow_ie_stru);
}
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)

oal_void mac_set_vendor_vht_ie(oal_void *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    oal_uint8 uc_ie_len;

    puc_buffer[0] = MAC_EID_VENDOR;
    puc_buffer[1] = MAC_WLAN_OUI_VENDOR_VHT_HEADER; /* The Vendor OUI, type and subtype */
    /*lint -e572*/ /*lint -e778*/
    puc_buffer[2] = (oal_uint8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM >> 16) & 0xff);
    puc_buffer[3] = (oal_uint8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM >> 8) & 0xff);
    puc_buffer[4] = (oal_uint8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM) & 0xff);
    puc_buffer[5] = MAC_WLAN_OUI_VENDOR_VHT_TYPE;
    puc_buffer[6] = MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE;
    /*lint +e572*/ /*lint +e778*/

    mac_set_vht_capabilities_ie_etc(pst_mac_vap, puc_buffer + puc_buffer[1] + MAC_IE_HDR_LEN, &uc_ie_len);
    if (uc_ie_len) {
        puc_buffer[1] += uc_ie_len;
        *puc_ie_len = puc_buffer[1] + MAC_IE_HDR_LEN;
    } else {
        *puc_ie_len = 0;
    }
}

oal_void mac_set_vendor_novht_ie(oal_void *pst_mac_vap, oal_uint8 *puc_buffer,
    oal_uint8 *puc_ie_len, oal_uint8 en_1024qam_capable)
{
    puc_buffer[0] = MAC_EID_VENDOR;
    puc_buffer[1] = MAC_WLAN_OUI_VENDOR_VHT_HEADER; /* The Vendor OUI, type and subtype */
    /*lint -e572*/ /*lint -e778*/
    puc_buffer[2] = (oal_uint8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM >> 16) & 0xff);
    puc_buffer[3] = (oal_uint8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM >> 8) & 0xff);
    puc_buffer[4] = (oal_uint8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM)&0xff);
    puc_buffer[5] = MAC_WLAN_OUI_VENDOR_VHT_TYPE;

    puc_buffer[6] = MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE3;
    /* 5G时，BCM私有vendor ie中携带1024QAM使能的话,从MCS9扩到MCS11 */
    if (en_1024qam_capable == OAL_TRUE) {
        puc_buffer[6] = MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE2;
    }
    /*lint +e572*/ /*lint +e778*/
    *puc_ie_len = puc_buffer[1] + MAC_IE_HDR_LEN;
}
#endif

oal_void mac_set_ext_capabilities_ie_rom_cb(oal_void *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
#ifdef _PRE_WLAN_FEATURE_TWT
    mac_ext_cap_twt_ie_stru *pst_ext_cap_twt = OAL_PTR_NULL;
#endif

#if defined(_PRE_WLAN_FEATURE_11V) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    mac_ext_cap_ie_stru *pst_ext_cap;

    pst_ext_cap = (mac_ext_cap_ie_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    /* 首先需先使能wirelessmanagerment标志 */
    /* 然后如果是站点本地能力位和扩展控制变量均支持BSS TRANSITION 设置扩展能力bit位 */
    if ((OAL_TRUE == mac_mib_get_WirelessManagementImplemented(pst_mac_vap)) &&
        (OAL_TRUE == mac_mib_get_MgmtOptionBSSTransitionImplemented(pst_mac_vap)) &&
        (OAL_TRUE == mac_mib_get_MgmtOptionBSSTransitionActivated(pst_mac_vap))) {
        pst_ext_cap->bit_bss_transition = 1;
    }

#ifdef _PRE_WLAN_FEATURE_FTM // FTM认证打桩
    if (mac_is_ftm_enable(pst_mac_vap) == OAL_TRUE) {
        pst_ext_cap->bit_interworking = 1;
    }
#endif

#endif


    mac_ftm_add_to_ext_capabilities_ie(pst_mac_vap, puc_buffer, puc_ie_len);
#ifdef _PRE_WLAN_FEATURE_TWT
    if (OAL_TRUE == mac_mib_get_he_TWTOptionActivated(pst_mac_vap)) {
        puc_buffer[1] = MAC_XCAPS_EX_TWT_LEN;
        pst_ext_cap_twt = (mac_ext_cap_twt_ie_stru *)(puc_buffer + MAC_IE_HDR_LEN);
        pst_ext_cap_twt->bit_resv16 = 0;
        pst_ext_cap_twt->bit_twt_requester_support = 1;
        pst_ext_cap_twt->bit_resv17 = 0;

        (*puc_ie_len) = MAC_XCAPS_EX_TWT_LEN + MAC_IE_HDR_LEN;
    }
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    pst_ext_cap->bit_multi_bssid = mac_mib_get_he_MultiBSSIDActived(((mac_vap_stru *)pst_mac_vap)->_rom);
#endif
}



oal_void mac_set_ht_cap_ie_rom_cb(oal_void *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_frame_ht_cap_stru *pst_ht_capinfo;

    /* 将puc_buffer地址指向ht_capinfo字段,puc_buffer当前地址在HT_ASEL字段 */
    puc_buffer -= MAC_HT_CAP_LEN - MAC_HT_ASEL_LEN;

    pst_ht_capinfo = (mac_frame_ht_cap_stru *)puc_buffer;

    /* 设置所支持的信道宽度集"，0:仅20MHz运行; 1:20MHz与40MHz运行 */
    pst_ht_capinfo->bit_supported_channel_width = mac_mib_get_FortyMHzOperationImplemented(pst_mac_vap);

    /* 只有支持40M的情况下，才可以宣称支持40M short GI */
    if (pst_ht_capinfo->bit_supported_channel_width) {
        pst_ht_capinfo->bit_short_gi_40mhz = mac_mib_get_ShortGIOptionInFortyImplemented(pst_mac_vap);
    } else {
        pst_ht_capinfo->bit_short_gi_40mhz = 0;
    }
}



oal_void mac_set_ht_opern_ie_rom_cb(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_ht_opern_ac_stru *pst_ht_opern = (mac_ht_opern_ac_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    /* 设置"次信道偏移量" */
    switch (pst_mac_vap->st_channel.en_bandwidth) {
        case WLAN_BAND_WIDTH_40PLUS:
        case WLAN_BAND_WIDTH_80PLUSPLUS:
        case WLAN_BAND_WIDTH_80PLUSMINUS:
            pst_ht_opern->bit_secondary_chan_offset = MAC_SCA;
            break;

        case WLAN_BAND_WIDTH_40MINUS:
        case WLAN_BAND_WIDTH_80MINUSPLUS:
        case WLAN_BAND_WIDTH_80MINUSMINUS:
            pst_ht_opern->bit_secondary_chan_offset = MAC_SCB;
            break;

#ifdef _PRE_WLAN_FEATURE_160M
        case WLAN_BAND_WIDTH_160PLUSPLUSPLUS:
        case WLAN_BAND_WIDTH_160PLUSPLUSMINUS:
        case WLAN_BAND_WIDTH_160PLUSMINUSPLUS:
        case WLAN_BAND_WIDTH_160PLUSMINUSMINUS:
            pst_ht_opern->bit_secondary_chan_offset = MAC_SCA;
            break;

        case WLAN_BAND_WIDTH_160MINUSPLUSPLUS:
        case WLAN_BAND_WIDTH_160MINUSPLUSMINUS:
        case WLAN_BAND_WIDTH_160MINUSMINUSPLUS:
        case WLAN_BAND_WIDTH_160MINUSMINUSMINUS:
            pst_ht_opern->bit_secondary_chan_offset = MAC_SCB;
            break;
#endif

        default:
            pst_ht_opern->bit_secondary_chan_offset = MAC_SCN;
            break;
    }

    pst_ht_opern->bit_chan_center_freq_seg2 = 0;
}


oal_void mac_set_vht_capinfo_field_cb(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
#ifdef _PRE_WLAN_FEATURE_M2S
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_vht_cap_info_stru *pst_vht_capinfo = (mac_vht_cap_info_stru *)puc_buffer;
    mac_user_stru *pst_mac_user = OAL_PTR_NULL;

    pst_vht_capinfo->bit_num_bf_ant_supported = mac_mib_get_VHTBeamformeeNTxSupport(pst_mac_vap) - 1;

    /* 参考标杆,该字段根据对端空间流能力和自己的能力取交集 */
    pst_mac_user = mac_res_get_mac_user_etc(pst_mac_vap->us_assoc_vap_id);
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA && pst_mac_user != OAL_PTR_NULL &&
        (pst_mac_user->st_vht_hdl.bit_num_sounding_dim != 0)) {
        pst_vht_capinfo->bit_num_bf_ant_supported = OAL_MIN(pst_vht_capinfo->bit_num_bf_ant_supported,
                                                            pst_mac_user->st_vht_hdl.bit_num_sounding_dim);
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 算法限定mu bfee只在WLAN0开启 */
    if (!IS_LEGACY_STA(pst_mac_vap)) {
        pst_vht_capinfo->bit_mu_beamformee_cap = OAL_FALSE;
    }
#endif
    if (IS_LEGACY_AP(pst_mac_vap) && ((pst_mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS) ||
        (pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_40M))) {
        pst_vht_capinfo->bit_supported_channel_width = 0;
        pst_vht_capinfo->bit_short_gi_160mhz = 0;
    }
#ifdef _PRE_WLAN_FEATURE_160M
    if ((pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS) &&
             (pst_mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS)) {
        pst_vht_capinfo->bit_rx_stbc = OAL_FALSE;
    }
#endif
#endif
}



oal_void mac_set_vht_opern_ie_rom_cb(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_vht_opern_stru *pst_vht_opern = (mac_vht_opern_stru *)puc_buffer;

    if ((pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) &&
        (pst_mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS)) {
        pst_vht_opern->uc_channel_width = WLAN_MIB_VHT_OP_WIDTH_80;
    }
#ifdef _PRE_WLAN_FEATURE_160M
    else if ((pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS) &&
             (pst_mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS)) {
        pst_vht_opern->uc_channel_width = WLAN_MIB_VHT_OP_WIDTH_80;
    }
#endif
    else {
        pst_vht_opern->uc_channel_width = WLAN_MIB_VHT_OP_WIDTH_20_40;
    }

    switch (pst_mac_vap->st_channel.en_bandwidth) {
#ifdef _PRE_WLAN_FEATURE_160M
        /* 从20信道+1, 从40信道+1, 从80信道+1 */
        case WLAN_BAND_WIDTH_160PLUSPLUSPLUS:
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number + 6;
            pst_vht_opern->uc_channel_center_freq_seg1 = pst_mac_vap->st_channel.uc_chan_number + 14;
            break;

        /* 从20信道+1, 从40信道+1, 从80信道-1 */
        case WLAN_BAND_WIDTH_160PLUSPLUSMINUS:
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number + 6;
            pst_vht_opern->uc_channel_center_freq_seg1 = pst_mac_vap->st_channel.uc_chan_number - 2;
            break;

        /* 从20信道+1, 从40信道-1, 从80信道+1 */
        case WLAN_BAND_WIDTH_160PLUSMINUSPLUS:
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number - 2;
            pst_vht_opern->uc_channel_center_freq_seg1 = pst_mac_vap->st_channel.uc_chan_number + 6;
            break;

        /* 从20信道+1, 从40信道-1, 从80信道-1 */
        case WLAN_BAND_WIDTH_160PLUSMINUSMINUS:
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number - 2;
            pst_vht_opern->uc_channel_center_freq_seg1 = pst_mac_vap->st_channel.uc_chan_number - 10;
            break;

        /* 从20信道-1, 从40信道+1, 从80信道+1 */
        case WLAN_BAND_WIDTH_160MINUSPLUSPLUS:
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number + 2;
            pst_vht_opern->uc_channel_center_freq_seg1 = pst_mac_vap->st_channel.uc_chan_number + 10;
            break;

        /* 从20信道-1, 从40信道+1, 从80信道-1 */
        case WLAN_BAND_WIDTH_160MINUSPLUSMINUS:
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number + 2;
            pst_vht_opern->uc_channel_center_freq_seg1 = pst_mac_vap->st_channel.uc_chan_number - 6;
            break;

        /* 从20信道-1, 从40信道-1, 从80信道+1 */
        case WLAN_BAND_WIDTH_160MINUSMINUSPLUS:
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number - 6;
            pst_vht_opern->uc_channel_center_freq_seg1 = pst_mac_vap->st_channel.uc_chan_number + 2;
            break;

        /* 从20信道-1, 从40信道-1, 从80信道-1 */
        case WLAN_BAND_WIDTH_160MINUSMINUSMINUS:
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number - 6;
            pst_vht_opern->uc_channel_center_freq_seg1 = pst_mac_vap->st_channel.uc_chan_number - 14;
            break;
#endif
        case WLAN_BAND_WIDTH_80PLUSPLUS:
            /***********************************************************************
            | 主20 | 从20 | 从40       |
                          |
                          |中心频率相对于主20偏6个信道
            ************************************************************************/
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number + 6;
            break;

        case WLAN_BAND_WIDTH_80PLUSMINUS:
            /***********************************************************************
            | 从40        | 主20 | 从20 |
                          |
                          |中心频率相对于主20偏-2个信道
            ************************************************************************/
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number - 2;
            break;

        case WLAN_BAND_WIDTH_80MINUSPLUS:
            /***********************************************************************
            | 从20 | 主20 | 从40       |
                          |
                          |中心频率相对于主20偏2个信道
            ************************************************************************/
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number + 2;
            break;

        case WLAN_BAND_WIDTH_80MINUSMINUS:
            /***********************************************************************
            | 从40        | 从20 | 主20 |
                          |
                          |中心频率相对于主20偏-6个信道
            ************************************************************************/
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number - 6;
            break;

        // 为了提高兼容性，40M及其以下都填0
        default:
            /* 中心频率直接填0  */
            pst_vht_opern->uc_channel_center_freq_seg0 = 0;
            break;
    }
}

#ifdef _PRE_WLAN_NARROW_BAND

oal_void mac_get_nb_ie(oal_void *pst_vap, oal_uint8 *puc_payload, oal_uint16 us_frame_len)
{
    oal_uint8 *puc_nb;
    oal_uint8 uc_nb_len;
    oal_uint8 uc_loop;
    oal_uint8 *puc_bw;

    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    puc_nb = mac_find_vendor_ie_etc(MAC_EID_VENDOR, MAC_HISI_NB_IE, puc_payload, us_frame_len);
    if (puc_nb == OAL_PTR_NULL) {
        return;
    }

    uc_nb_len = puc_nb[1];

    if (uc_nb_len <= MAC_OUI_LEN) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TX, "mac_get_nb_ie::nb_len[%u] err!", uc_nb_len);
        return;
    }

    puc_bw = puc_nb + MAC_IE_HDR_LEN + MAC_OUI_LEN + MAC_OUITYPE_LEN;
    for (uc_loop = 0; uc_loop < (uc_nb_len - (MAC_OUI_LEN + MAC_OUITYPE_LEN)); uc_loop++) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TX, "mac_get_nb_ie::bw[%x]!", puc_bw[uc_loop]);
    }
}
#endif


oal_uint8 *mac_get_wmm_ie_ram_etc(oal_uint8 *puc_beacon_body, oal_uint16 us_frame_len)
{
    oal_uint8 *puc_wmmie = OAL_PTR_NULL;

    puc_wmmie = mac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WMM,
                                       puc_beacon_body, us_frame_len);
    if (puc_wmmie == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    return mac_is_wmm_ie_etc(puc_wmmie) ? puc_wmmie : OAL_PTR_NULL;
}

/*lint -e19*/
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
oal_module_symbol(mac_set_vender_4addr_ie);
#endif /*lint +e19*/



