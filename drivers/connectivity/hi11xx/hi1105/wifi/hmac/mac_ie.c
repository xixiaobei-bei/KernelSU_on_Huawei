

#include "mac_ie.h"
#include "mac_frame.h"
#include "mac_device.h"
#include "securec.h"
#include "mac_mib.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_IE_C
#define IE_BUFFER_HEADER 3

/* 加密套件获取来源 */
#define SUITE_FORM_WPA    0
#define SUITE_FORM_RSN    1
/* srp ie 最大最小长度 */
#define MAC_HE_SRP_IE_MIN_LEN 2
#define MAC_HE_SRP_IE_MAX_LEN 21


oal_bool_enum_uint8 mac_ie_proc_ht_supported_channel_width(mac_user_stru *mac_user_sta, mac_vap_stru *mac_vap,
    uint8_t supported_channel_width, oal_bool_enum prev_asoc_ht)
{
    /* 不支持20/40Mhz频宽 */
    if (supported_channel_width == 0) {
        if ((prev_asoc_ht == OAL_FALSE) || (mac_user_sta->st_ht_hdl.bit_supported_channel_width == OAL_TRUE)) {
            mac_vap->st_protection.uc_sta_20M_only_num++;
        }

        return OAL_FALSE;
    } else { /* 支持20/40Mhz频宽 */
        /*  如果STA之前已经作为不支持20/40Mhz频宽的HT站点与AP关联 */
        if ((prev_asoc_ht == OAL_TRUE) && (mac_user_sta->st_ht_hdl.bit_supported_channel_width == OAL_FALSE)) {
            mac_vap->st_protection.uc_sta_20M_only_num--;
        }

        return OAL_TRUE;
    }
}


oal_bool_enum_uint8 mac_ie_proc_ht_green_field(mac_user_stru *mac_user_sta, mac_vap_stru *mac_vap,
    uint8_t ht_green_field, oal_bool_enum prev_asoc_ht)
{
    /* 不支持Greenfield */
    if (ht_green_field == 0) {
        if ((prev_asoc_ht == OAL_FALSE) || (mac_user_sta->st_ht_hdl.bit_ht_green_field == OAL_TRUE)) {
            mac_vap->st_protection.uc_sta_non_gf_num++;
        }

        return OAL_FALSE;
    } else { /* 支持Greenfield */
        /*  如果STA之前已经作为不支持GF的HT站点与AP关联 */
        if ((prev_asoc_ht == OAL_TRUE) && (mac_user_sta->st_ht_hdl.bit_ht_green_field == OAL_FALSE)) {
            mac_vap->st_protection.uc_sta_non_gf_num--;
        }

        return OAL_TRUE;
    }
}


oal_bool_enum_uint8 mac_ie_proc_lsig_txop_protection_support(mac_user_stru *mac_user_sta,
    mac_vap_stru *mac_vap, uint8_t lsig_txop_protection_support, oal_bool_enum prev_asoc_ht)
{
    /* 不支持L-sig txop protection */
    if (lsig_txop_protection_support == 0) {
        if ((prev_asoc_ht == OAL_FALSE) || (mac_user_sta->st_ht_hdl.bit_lsig_txop_protection == OAL_TRUE)) {
            mac_vap->st_protection.uc_sta_no_lsig_txop_num++;
        }

        return OAL_FALSE;
    } else { /* 支持L-sig txop protection */
        /*  如果STA之前已经作为不支持Lsig txop protection的HT站点与AP关联 */
        if ((prev_asoc_ht == OAL_TRUE) && (mac_user_sta->st_ht_hdl.bit_lsig_txop_protection == OAL_FALSE)) {
            mac_vap->st_protection.uc_sta_no_lsig_txop_num--;
        }

        return OAL_TRUE;
    }
}

OAL_STATIC void mac_ie_proc_ht_sta_ht_info(mac_user_stru *mac_user, mac_user_ht_hdl_stru *ht_hdl,
    uint16_t ht_cap_info)
{
    uint8_t smps;

    /* 检查STA所支持的LDPC编码能力 B0，0:不支持，1:支持 */
    ht_hdl->bit_ldpc_coding_cap = (ht_cap_info & BIT0);

    /* 提取AP所支持的带宽能力  */
    ht_hdl->bit_supported_channel_width = ((ht_cap_info & BIT1) >> 1); /* 1 offset */

    /* 检查空间复用节能模式 B2~B3 */
    smps = ((ht_cap_info & (BIT3 | BIT2)) >> 2); /* 2 offset */
    ht_hdl->bit_sm_power_save = mac_ie_proc_sm_power_save_field(mac_user, smps);

    /* 提取AP支持Greenfield情况 */
    ht_hdl->bit_ht_green_field = ((ht_cap_info & BIT4) >> 4); /* 4 offset */

    /* 提取AP支持20MHz Short-GI情况 */
    ht_hdl->bit_short_gi_20mhz = ((ht_cap_info & BIT5) >> 5); /* 5 offset */

    /* 提取AP支持40MHz Short-GI情况 */
    ht_hdl->bit_short_gi_40mhz = ((ht_cap_info & BIT6) >> 6); /* 6 offset */

    /* 提取AP支持STBC PPDU情况 */
    ht_hdl->bit_rx_stbc = (uint8_t)((ht_cap_info & (BIT9 | BIT8)) >> 8); /* 8 offset */

    /* 提取AP 40M上DSSS/CCK的支持情况 */
    ht_hdl->bit_dsss_cck_mode_40mhz = ((ht_cap_info & BIT12) >> 12); /* 12 offset */

    /* 提取AP L-SIG TXOP 保护的支持情况 */
    ht_hdl->bit_lsig_txop_protection = ((ht_cap_info & BIT15) >> 15); /* 15 offset */
}

OAL_STATIC void mac_ie_proc_ht_sta_txbf(mac_user_ht_hdl_stru *ht_hdl, uint32_t txbf_elem)
{
    ht_hdl->bit_imbf_receive_cap = (txbf_elem & BIT0);
    ht_hdl->bit_receive_staggered_sounding_cap = ((txbf_elem & BIT1) >> 1); /* 1 offset */
    ht_hdl->bit_transmit_staggered_sounding_cap = ((txbf_elem & BIT2) >> 2); /* 2 offset */
    ht_hdl->bit_receive_ndp_cap = ((txbf_elem & BIT3) >> 3); /* 3 offset */
    ht_hdl->bit_transmit_ndp_cap = ((txbf_elem & BIT4) >> 4); /* 4 offset */
    ht_hdl->bit_imbf_cap = ((txbf_elem & BIT5) >> 5); /* 5 offset */
    ht_hdl->bit_calibration = ((txbf_elem & 0x000000C0) >> 6); /* 6 offset */
    ht_hdl->bit_exp_csi_txbf_cap = ((txbf_elem & BIT8) >> 8); /* 8 offset */
    ht_hdl->bit_exp_noncomp_txbf_cap = ((txbf_elem & BIT9) >> 9); /* 9 offset */
    ht_hdl->bit_exp_comp_txbf_cap = ((txbf_elem & BIT10) >> 10); /* 10 offset */
    ht_hdl->bit_exp_csi_feedback = ((txbf_elem & 0x00001800) >> 11); /* 11 offset */
    ht_hdl->bit_exp_noncomp_feedback = ((txbf_elem & 0x00006000) >> 13); /* 13 offset */
    ht_hdl->bit_exp_comp_feedback = ((txbf_elem & 0x0001C000) >> 15); /* 15 offset */
    ht_hdl->bit_min_grouping = ((txbf_elem & 0x00060000) >> 17); /* 17 offset */
    ht_hdl->bit_csi_bfer_ant_number = ((txbf_elem & 0x001C0000) >> 19); /* 19 offset */
    ht_hdl->bit_noncomp_bfer_ant_number = ((txbf_elem & 0x00600000) >> 21); /* 21 offset */
    ht_hdl->bit_comp_bfer_ant_number = ((txbf_elem & 0x01C00000) >> 23); /* 23 offset */
    ht_hdl->bit_csi_bfee_max_rows = ((txbf_elem & 0x06000000) >> 25); /* 25 offset */
    ht_hdl->bit_channel_est_cap = ((txbf_elem & 0x18000000) >> 27); /* 27 offset */
}


uint32_t mac_ie_proc_ht_sta(mac_vap_stru *mac_sta, uint8_t *payload, uint16_t offset,
    mac_user_stru *mac_user_ap, uint16_t *amsdu_maxsize)
{
    uint8_t mcs_bmp_index;
    mac_user_ht_hdl_stru *ht_hdl = NULL;
    mac_user_ht_hdl_stru st_ht_hdl;
    uint16_t tmp_info_elem, tmp_txbf_low, ht_cap_info;
    uint32_t tmp_txbf_elem;

    if ((mac_sta == NULL) || (payload == NULL) || (mac_user_ap == NULL) ||
        (amsdu_maxsize == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_ie_proc_ht_sta::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ht_hdl = &st_ht_hdl;
    mac_user_get_ht_hdl(mac_user_ap, ht_hdl);

    /* 带有 HT Capability Element 的 AP，标示它具有HT capable. */
    ht_hdl->en_ht_capable = OAL_TRUE;

    offset += MAC_IE_HDR_LEN;

    /* 解析 HT Capabilities Info Field */
    ht_cap_info = oal_make_word16(payload[offset], payload[offset + 1]);

    mac_ie_proc_ht_sta_ht_info(mac_user_ap, ht_hdl, ht_cap_info);
    offset += MAC_HT_CAPINFO_LEN;

    /* 提取AP支持最大A-MSDU长度情况 */
    if ((ht_cap_info & BIT11) == 0) {
        *amsdu_maxsize = WLAN_MIB_MAX_AMSDU_LENGTH_SHORT;
    } else {
        *amsdu_maxsize = WLAN_MIB_MAX_AMSDU_LENGTH_LONG;
    }

    /* 解析 A-MPDU Parameters Field */
    /* 提取 Maximum Rx A-MPDU factor (B1 - B0) */
    ht_hdl->uc_max_rx_ampdu_factor = (payload[offset] & 0x03);

    /* 提取 Minmum Rx A-MPDU factor (B3 - B2) */
    ht_hdl->uc_min_mpdu_start_spacing = (payload[offset] >> 2) & 0x07;

    offset += MAC_HT_AMPDU_PARAMS_LEN;

    /* 解析 Supported MCS Set Field */
    for (mcs_bmp_index = 0; mcs_bmp_index < WLAN_HT_MCS_BITMASK_LEN; mcs_bmp_index++) {
        ht_hdl->uc_rx_mcs_bitmask[mcs_bmp_index] = (*(uint8_t*)(payload + offset + mcs_bmp_index));
    }
    ht_hdl->uc_rx_mcs_bitmask[WLAN_HT_MCS_BITMASK_LEN - 1] &= 0x1F;
    offset += MAC_HT_SUP_MCS_SET_LEN;

    /* 解析 HT Extended Capabilities Info Field */
    ht_cap_info = oal_make_word16(payload[offset], payload[offset + BYTE_OFFSET_1]);
    /* 提取 HTC support Information */
    if ((ht_cap_info & BIT10) != 0) {
        ht_hdl->uc_htc_support = 1;
    }

    offset += MAC_HT_EXT_CAP_LEN;

    /* 解析 Tx Beamforming Field */
    tmp_info_elem = oal_make_word16(payload[offset], payload[offset + BYTE_OFFSET_1]);
    tmp_txbf_low = oal_make_word16(payload[offset + BYTE_OFFSET_2], payload[offset + BYTE_OFFSET_3]);
    tmp_txbf_elem = oal_make_word32(tmp_info_elem, tmp_txbf_low);
    mac_ie_proc_ht_sta_txbf(ht_hdl, tmp_txbf_elem);
    mac_user_set_ht_hdl(mac_user_ap, ht_hdl);

    return OAL_SUCC;
}


oal_bool_enum_uint8 mac_ie_check_p2p_action(uint8_t *payload)
{
    /* 找到WFA OUI */
    if ((oal_memcmp(payload, g_auc_p2p_oui, MAC_OUI_LEN) == 0) && (payload[MAC_OUI_LEN] == MAC_OUITYPE_P2P)) {
        /*  找到WFA P2P v1.0 oui type */
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


wlan_mib_mimo_power_save_enum mac_ie_proc_sm_power_save_field(mac_user_stru *mac_user, uint8_t uc_smps)
{
    if (uc_smps == MAC_SMPS_STATIC_MODE) {
        return WLAN_MIB_MIMO_POWER_SAVE_STATIC;
    } else if (uc_smps == MAC_SMPS_DYNAMIC_MODE) {
        return WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC;
    } else {
        return WLAN_MIB_MIMO_POWER_SAVE_MIMO;
    }
}

#ifdef _PRE_WLAN_FEATURE_11AX
uint32_t mac_ie_from_he_cap_get_nss(mac_he_hdl_stru *mac_he_hdl, wlan_nss_enum_uint8 *nss)
{
    if (mac_he_hdl->st_he_cap_ie.st_he_mcs_nss.st_rx_he_mcs_map_below_80mhz.bit_max_he_mcs_4ss !=
        MAC_MAX_SUP_INVALID_11AX_EACH_NSS) {
        *nss = WLAN_FOUR_NSS;
    } else if (mac_he_hdl->st_he_cap_ie.st_he_mcs_nss.st_rx_he_mcs_map_below_80mhz.bit_max_he_mcs_3ss !=
        MAC_MAX_SUP_INVALID_11AX_EACH_NSS) {
        *nss = WLAN_TRIPLE_NSS;
    } else if (mac_he_hdl->st_he_cap_ie.st_he_mcs_nss.st_rx_he_mcs_map_below_80mhz.bit_max_he_mcs_2ss !=
        MAC_MAX_SUP_INVALID_11AX_EACH_NSS) {
        *nss = WLAN_DOUBLE_NSS;
    } else if (mac_he_hdl->st_he_cap_ie.st_he_mcs_nss.st_rx_he_mcs_map_below_80mhz.bit_max_he_mcs_1ss !=
        MAC_MAX_SUP_INVALID_11AX_EACH_NSS) {
        *nss = WLAN_SINGLE_NSS;
    } else {
        *nss = WLAN_SINGLE_NSS;
        oam_warning_log0(0, OAM_SF_ANY, "{mac_ie_from_he_cap_get_nss::invalid he nss.}");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

static uint32_t mac_ie_parse_he_cap_copy(uint8_t **he_buffer, mac_frame_he_cap_ie_stru *he_cap_value)
{
    int32_t ret;
    mac_frame_he_mac_cap_stru *he_mac_cap = NULL;
    mac_frame_he_phy_cap_stru *pst_he_phy_cap = NULL;
    /* mac cap */
    he_mac_cap = (mac_frame_he_mac_cap_stru*)(*he_buffer);
    ret = memcpy_s(&he_cap_value->st_he_mac_cap, sizeof(mac_frame_he_mac_cap_stru), he_mac_cap,
        sizeof(mac_frame_he_mac_cap_stru));
    *he_buffer += sizeof(mac_frame_he_mac_cap_stru);

    /* PHY Cap */
    pst_he_phy_cap = (mac_frame_he_phy_cap_stru*)(*he_buffer);
    ret += memcpy_s(&he_cap_value->st_he_phy_cap, sizeof(mac_frame_he_phy_cap_stru), pst_he_phy_cap,
        sizeof(mac_frame_he_phy_cap_stru));
    *he_buffer += sizeof(mac_frame_he_phy_cap_stru);

    return ret;
}


uint32_t mac_ie_parse_he_cap(uint8_t *he_cap_ie, mac_frame_he_cap_ie_stru *he_cap_value)
{
    uint8_t *he_buffer = NULL;
    uint8_t uc_mcs_nss_set_size = 2;
    mac_frame_he_supported_he_mcs_nss_set_stru *pst_mac_nss_set = NULL;
    int32_t l_ret, ie_buf_len;

    /* 解析he cap IE */
    if (oal_any_null_ptr2(he_cap_ie, he_cap_value)) {
        oam_error_log0(0, OAM_SF_11AX,
                       "{mac_ie_parse_he_cap::he_cap_ie or he_cap_value is null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /*
     * -------------------------------------------------------------------------
     * |EID |Length |EID Extension|HE MAC Capa. Info |HE PHY Capa. Info|
     * -------------------------------------------------------------------------
     * |1   |1      |1            |6                 |9                |
     * -------------------------------------------------------------------------
     * |Tx Rx HE MCS NSS Support |PPE Thresholds(Optional)|
     * -------------------------------------------------------------------------
     * |4or8or12                 |Variable                |
     * -------------------------------------------------------------------------
     */
    ie_buf_len = MAC_HE_CAP_MIN_LEN; // 此长度包含 supported HE-MCS and NSS Set 的rx 和tx MCS Maps
    if (he_cap_ie[1] < MAC_HE_CAP_MIN_LEN) {
        oam_warning_log1(0, OAM_SF_11AX, "{hmac_proc_he_cap_ie::invalid he cap ie len[%d].}", he_cap_ie[1]);
        return OAL_FAIL;
    }

    he_buffer = he_cap_ie + IE_BUFFER_HEADER;
    l_ret = mac_ie_parse_he_cap_copy(&he_buffer, he_cap_value);

    /* 解析Support HE-MCS NSS Set */
    /*
     * |-------------------------------------------------------------------------------------------------|
     * | Rx HE-MCS Map | Tx HE-MCS Map | Rx HE-MCS Map  | Tx HE-MCS Map  | Rx HE-MCS Map | Tx HE-MCS Map |
     * | <= 80 MHz     | <= 80 MHz     | 160 MHz        | 160 MHz        | 80+80 MHz     | 80+80 MHz     |
     * |-------------------------------------------------------------------------------------------------|
     * | 2 Octets      | 2 Octets      | 0 or 2 Octets  | 0 or 2 Octets  | 0 or 2 Octets | 0 or 2 Octets |
     * |-------------------------------------------------------------------------------------------------|
     */
    /*
     * 1. HE PHY Capabilities Info中Channel Width Set字段bit2为1时，
     * HE Supported HE-MCS And NSS Set中存在Rx HE-MCS Map 160 MHz和Tx HE-MCS Map 160 MHz字段
     * 2. HE PHY Capabilities Info中Channel Width Set字段bit3为1时，
     * HE Supported HE-MCS And NSS Set中存在Rx HE-MCS Map 80+80 MHz和Tx HE-MCS Map 80+80 MHz字段
     */
    pst_mac_nss_set = (mac_frame_he_supported_he_mcs_nss_set_stru*)he_buffer;
    if ((he_cap_value->st_he_phy_cap.bit_channel_width_set & BIT2) != 0) {
        uc_mcs_nss_set_size += 2;  /* 2表示Rx HE-MCS Map 160 MHz和Tx HE-MCS Map 160 MHz字段 */
        ie_buf_len += 2 * sizeof(mac_frame_he_basic_mcs_nss_stru); /* 2*size：表示字段大小 */
    }
    if ((he_cap_value->st_he_phy_cap.bit_channel_width_set & BIT3) != 0) {
        uc_mcs_nss_set_size += 2;  /* 2表示Rx HE-MCS Map 80+80 MHz和Tx HE-MCS Map 80+80 MHz字段 */
        ie_buf_len += 2 * sizeof(mac_frame_he_basic_mcs_nss_stru); /* 2*size：表示字段大小 */
    }

    /* 再次判断长度是否符合要求：对可选项 进行长度判断，不满足条件直接返回，防止读越界 */
    if (he_cap_ie[1] < ie_buf_len) {
        oam_warning_log1(0, OAM_SF_11AX, "{hmac_proc_he_cap_ie::invalid he cap ie len[%d].}", he_cap_ie[1]);
        return OAL_FAIL;
    }

    /* AP只需记录对端发过来的值 */
    l_ret += memcpy_s((uint8_t*)(&he_cap_value->st_he_mcs_nss), sizeof(mac_frame_he_supported_he_mcs_nss_set_stru),
        (uint8_t*)pst_mac_nss_set, uc_mcs_nss_set_size * sizeof(mac_frame_he_basic_mcs_nss_stru));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_11AX, "mac_ie_parse_he_cap::memcpy fail!");
        return OAL_FAIL;
    }

    /* PPE thresholds 暂不解析 */
    return OAL_SUCC;
}


/*lint -save -e438 */
uint32_t mac_ie_parse_he_oper(uint8_t *puc_he_oper_ie, mac_frame_he_oper_ie_stru *pst_he_oper_ie_value)
{
    uint8_t *puc_ie_buffer = NULL;
    mac_frame_he_operation_param_stru *pst_he_oper_param = NULL;
    mac_frame_he_basic_mcs_nss_stru *pst_he_basic_mcs_nss = NULL;
    mac_frame_vht_operation_info_stru *pst_vht_operation_info = NULL;
    mac_frame_he_bss_color_info_stru  *pst_bss_color_info = NULL;
    int32_t l_ret;

    if (oal_unlikely(oal_any_null_ptr2(puc_he_oper_ie, pst_he_oper_ie_value))) {
        oam_error_log0(0, OAM_SF_11AX, "{mac_ie_parse_he_oper::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /*
     * -------------------------------------------------------------------------
     * |EID |Length |EID Extension|HE Operation Parameters|Basic HE MCS Ans NSS Set|
     * -------------------------------------------------------------------------
     * |1   |1          |1                  |           4                     |              3                       |
     * -------------------------------------------------------------------------
     * |VHT Operation Info  |MaxBssid Indicator|
     * -------------------------------------------------------------------------
     * |      0 or 3              |0 or More             |
     * -------------------------------------------------------------------------
     */
    if (puc_he_oper_ie[1] < MAC_HE_OPERAION_MIN_LEN) {
        oam_warning_log1(0, OAM_SF_11AX, "{mac_ie_parse_he_oper::invalid he oper ie len[%d].}", puc_he_oper_ie[1]);
        return OAL_FAIL;
    }

    puc_ie_buffer = puc_he_oper_ie + IE_BUFFER_HEADER;

    /* 解析HE Operation Parameters */
    pst_he_oper_param = (mac_frame_he_operation_param_stru*)puc_ie_buffer;
    puc_ie_buffer += MAC_HE_OPE_PARAM_LEN;
    l_ret = memcpy_s(&pst_he_oper_ie_value->st_he_oper_param, sizeof(mac_frame_he_operation_param_stru),
        pst_he_oper_param, MAC_HE_OPE_PARAM_LEN);

    pst_bss_color_info = (mac_frame_he_bss_color_info_stru*)puc_ie_buffer;
    puc_ie_buffer += sizeof(mac_frame_he_bss_color_info_stru);
    l_ret += memcpy_s(&pst_he_oper_ie_value->st_bss_color, sizeof(mac_frame_he_bss_color_info_stru),
        pst_bss_color_info, sizeof(mac_frame_he_bss_color_info_stru));

    /* 解析Basic HE MCS And NSS Set */
    pst_he_basic_mcs_nss = (mac_frame_he_basic_mcs_nss_stru*)puc_ie_buffer;
    puc_ie_buffer += MAC_HE_OPE_BASIC_MCS_NSS_LEN;
    l_ret += memcpy_s((uint8_t *)(&pst_he_oper_ie_value->st_he_basic_mcs_nss),
                      sizeof(mac_frame_he_basic_mcs_nss_stru), pst_he_basic_mcs_nss,
                      sizeof(mac_frame_he_basic_mcs_nss_stru));

    if ((puc_he_oper_ie[1] >= (MAC_HE_OPERAION_MIN_LEN + sizeof(mac_frame_vht_operation_info_stru))) &&
        (pst_he_oper_ie_value->st_he_oper_param.bit_vht_operation_info_present == 1)) {
        pst_vht_operation_info = (mac_frame_vht_operation_info_stru*)puc_ie_buffer;
        puc_ie_buffer += MAC_HE_VHT_OPERATION_INFO_LEN;
        l_ret += memcpy_s((uint8_t*)(&pst_he_oper_ie_value->st_vht_operation_info),
            sizeof(mac_frame_vht_operation_info_stru), pst_vht_operation_info,
            sizeof(mac_frame_vht_operation_info_stru));
    } else {
        pst_he_oper_ie_value->st_he_oper_param.bit_vht_operation_info_present = OAL_FALSE;
    }

    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_11AX, "mac_ie_parse_he_oper::memcpy fail!");
        return OAL_FAIL;
    }

    /* MaxBssid Indicator */
    return OAL_SUCC;
}


oal_bool_enum_uint8 mac_proc_he_uora_parameter_set_ie(mac_vap_stru *mac_vap, uint8_t *payload, uint16_t msg_len)
{
    uint8_t *uora_ie = NULL;
    mac_he_uora_para_stru *vap_he_uora_para = NULL;
    mac_he_uora_para_stru he_uora_para = { 0 };

    if (oal_any_null_ptr2(mac_vap, payload)) {
        oam_error_log0(0, OAM_SF_11AX, "{mac_proc_he_uora_parameter_set_ie::mac_vap or payload is null.}");
        return OAL_FALSE;
    }

    /* 不支持UORA，直接返回；支持UORA，才进行后续的处理 */
    if (mac_mib_get_he_OFDMARandomAccess(mac_vap) == OAL_FALSE) {
        return OAL_FALSE;
    }

    /*
     * -------------------------------------------------------------------------
     * |EID |Length |EID Extension|OCW Range |
     * -------------------------------------------------------------------------
     * | 1  |   1   |     1       |     1    |
     * -------------------------------------------------------------------------
     * |255 | xx    |      37     |    xxx   |
     * -------------------------------------------------------------------------
     * |          UORA Parameter Set         |
     * -------------------------------------------------------------------------
     */
    uora_ie = mac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_UORA_PARAMETER_SET, payload, msg_len);
    if ((uora_ie == NULL) || (uora_ie[1] < MAC_HE_UORA_PARAMETER_SET_LEN)) {
        return OAL_FALSE;
    }

    /*
     * -------------------------------------------------------------------------
     * |B0    B2 |B3    B5 |B6     B7 |
     * -------------------------------------------------------------------------
     * | EOCWmin | EOCWmax | Reserved |
     * -------------------------------------------------------------------------
     * |    3    |    3    |    2     |
     * -------------------------------------------------------------------------
     * |           OCW Range          |
     */
    he_uora_para.bit_uora_eocw_min = uora_ie[3] & 0x07; /* OCW Range: uora ie 3 */
    he_uora_para.bit_uora_eocw_max = (uora_ie[3] & 0x38) >> 3; /* OCW Range: uora ie 3 */

    vap_he_uora_para = (mac_he_uora_para_stru*)&(mac_vap->st_he_uora_eocw);

    /* When OCW range changed, update dmac and register */
    if (he_uora_para.bit_uora_eocw_min != vap_he_uora_para->bit_uora_eocw_min ||
        he_uora_para.bit_uora_eocw_max != vap_he_uora_para->bit_uora_eocw_max) {
        oam_warning_log4(0, OAM_SF_11AX,
            "{mac_proc_he_uora_parameter_set_ie::uora ie para changed, "
            "uora_eocw_min %d uora_eocw_max=%d, orginal vap_uora_min=%d vap_uora_max=%d}",
            he_uora_para.bit_uora_eocw_min, he_uora_para.bit_uora_eocw_max, vap_he_uora_para->bit_uora_eocw_min,
            vap_he_uora_para->bit_uora_eocw_max);

        vap_he_uora_para->bit_uora_eocw_min = he_uora_para.bit_uora_eocw_min;
        vap_he_uora_para->bit_uora_eocw_max = he_uora_para.bit_uora_eocw_max;
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*lint -restore */

uint32_t mac_ie_parse_mu_edca_parameter(uint8_t *puc_he_edca_ie,
    mac_frame_he_mu_edca_parameter_ie_stru *pst_he_mu_edca_value)
{
    mac_frame_he_mu_edca_parameter_ie_stru *pst_he_edca = NULL;

    if (oal_unlikely(oal_any_null_ptr2(puc_he_edca_ie, pst_he_mu_edca_value))) {
        oam_error_log0(0, OAM_SF_11AX, "{mac_ie_parse_mu_edca_parameter::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* MU EDCA Parameter Set Element */
    /*
     * -------------------------------------------------------------------------------------------
     * | EID | LEN | Ext EID|MU Qos Info |MU AC_BE Parameter Record | MU AC_BK Parameter Record  |
     * -------------------------------------------------------------------------------------------
     * |  1  |  1  |   1    |    1       |     3                    |        3                   |
     * -------------------------------------------------------------------------------------------
     * ------------------------------------------------------------------------------ -------------
     * | MU AC_VI Parameter Record | MU AC_VO Parameter Record                                   |
     * -------------------------------------------------------------------------------------------
     * |    3                      |     3                                                       |
     */
    /* QoS Info field when sent from WMM AP */
    /*
     * --------------------------------------------------------------------------------------------
     *    | EDCA Parameter Set Update Count | Q-Ack | Queue Request |TXOP Request | More Data Ack|
     * ---------------------------------------------------------------------------------------------
     * bit |        0~3                      |  1    |  1            |   1         |     1        |
     * ---------------------------------------------------------------------------------------------
     */
    if (puc_he_edca_ie[1] != MAC_HE_MU_EDCA_PARAMETER_SET_LEN) {
        oam_warning_log1(0, OAM_SF_11AX, "{mac_ie_parse_mu_edca_parameter::invalid mu edca ie len[%d].}",
            puc_he_edca_ie[1]);
        return OAL_FAIL;
    }

    puc_he_edca_ie = puc_he_edca_ie + IE_BUFFER_HEADER;

    /* 解析HE MU EDCA  Parameters Set Element */
    pst_he_edca = (mac_frame_he_mu_edca_parameter_ie_stru*)puc_he_edca_ie;
    if (EOK != memcpy_s((uint8_t*)(pst_he_mu_edca_value), sizeof(mac_frame_he_mu_edca_parameter_ie_stru),
        pst_he_edca, sizeof(mac_frame_he_mu_edca_parameter_ie_stru))) {
        oam_error_log0(0, OAM_SF_11AX, "mac_ie_parse_mu_edca_parameter::memcpy fail!");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


/*lint -save -e438 */
uint32_t mac_ie_parse_spatial_reuse_parameter(uint8_t *puc_he_srp_ie,
    mac_frame_he_spatial_reuse_parameter_set_ie_stru *pst_he_srp_value)
{
    uint8_t *he_buffer = NULL;
    mac_frame_he_sr_control_stru *pst_he_sr_control = NULL;
    int32_t l_ret;
    uint32_t ie_len;
    if (oal_any_null_ptr2(puc_he_srp_ie, pst_he_srp_value)) {
        oam_error_log0(0, OAM_SF_11AX,
            "{mac_ie_parse_spatial_reuse_parameter::puc_he_srp_ie or pst_he_srp_value is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if ((puc_he_srp_ie[1] < MAC_HE_SRP_IE_MIN_LEN) || (puc_he_srp_ie[1] > MAC_HE_SRP_IE_MAX_LEN)) {
        oam_error_log0(0, OAM_SF_11AX, "mac_ie_parse_spatial_reuse_parameter::IE len invalid!");
        return OAL_FAIL;
    }
    /* Spatial Reuse Parameter Set Element */
    /*
     * ------------------------------------------------------------------------------
     * | EID | LEN | Ext EID|SR Control |Non-SRG OBSS PD Max Offset | SRG OBSS PD Min Offset  |
     * ------------------------------------------------------------------------------
     * |  1   |  1   |   1       |    1          |     0 or 1              | 0 or 1 |
     * ------------------------------------------------------------------------------
     * -----------------------------------------------------------------------------/
     * |SRG OBSS PD Max Offset |SRG BSS Color Bitmap  | SRG Partial BSSID Bitmap |
     * ------------------------------------------------------------------------------
     * |    0 or 1                       |     0 or 8                     | 0 or 8  |
     */

    he_buffer = puc_he_srp_ie + IE_BUFFER_HEADER;
    ie_len = IE_BUFFER_HEADER - MAC_HE_SRP_IE_MIN_LEN; /* 获取 Element ID Extension 长度：规范长度1字节 */
    /* SR Control */
    pst_he_sr_control = (mac_frame_he_sr_control_stru *)he_buffer;
    he_buffer += sizeof(mac_frame_he_sr_control_stru);
    ie_len += sizeof(mac_frame_he_sr_control_stru); /* 获取 SR Control 长度：规范长度1字节 */
    l_ret = memcpy_s((uint8_t*)(&pst_he_srp_value->st_sr_control), sizeof(mac_frame_he_sr_control_stru),
        (uint8_t*)pst_he_sr_control, sizeof(mac_frame_he_sr_control_stru));

    if (pst_he_sr_control->bit_non_srg_offset_present == 1) {
        /* Non-SRG OBSS PD Max Offset  */
        pst_he_srp_value->uc_non_srg_obss_pd_max_offset = *he_buffer;
        he_buffer += 1;
        ie_len += 1; /* (可选字段)获取 Non-SRG OBSS PD Max Offset 长度：规范长度1字节 */
    }

    if (pst_he_sr_control->bit_srg_information_present == 1) {
        /* SRG OBSS PD Min Offset */
        pst_he_srp_value->uc_srg_obss_pd_min_offset = *he_buffer;
        he_buffer += 1; /* (可选字段)获取 SRG OBSS PD Min Offset 长度：规范长度1字节 */

        /* SRG OBSS PD Max Offset */
        pst_he_srp_value->uc_srg_obss_pd_max_offset = *he_buffer;
        he_buffer += 1; /* (可选字段)获取 SRG BSS PD Max Offset 长度：规范长度1字节 */

        /* SRG BSS Color Bitmap */
        l_ret += memcpy_s((uint8_t*)(&pst_he_srp_value->auc_srg_bss_color_bitmap), MAC_HE_SRG_BSS_COLOR_BITMAP_LEN,
            (uint8_t*)he_buffer, MAC_HE_SRG_BSS_COLOR_BITMAP_LEN);
        he_buffer += MAC_HE_SRG_BSS_COLOR_BITMAP_LEN; /* (可选字段)获取 SRG BSS PD Color Bitmap 长度：规范长度0~8字节 */

        /* SRG BSS Color Bitmap */
        l_ret += memcpy_s((uint8_t*)(&pst_he_srp_value->auc_srg_partial_bssid_bitmap),
            MAC_HE_SRG_PARTIAL_BSSID_BITMAP_LEN, (uint8_t*)he_buffer, MAC_HE_SRG_PARTIAL_BSSID_BITMAP_LEN);
        he_buffer += MAC_HE_SRG_PARTIAL_BSSID_BITMAP_LEN; /* (可选字段)获取 SRG Partial BSSID Bitmap长度：长度0~8字节 */
        ie_len += (1 + 1 + MAC_HE_SRG_BSS_COLOR_BITMAP_LEN + MAC_HE_SRG_PARTIAL_BSSID_BITMAP_LEN); /* 实际长度 */
    }
    if (l_ret != EOK || puc_he_srp_ie[1] < ie_len) {
        oam_error_log0(0, OAM_SF_11AX, "mac_ie_parse_spatial_reuse_parameter::memcpy fail!");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*lint -restore */

uint32_t mac_ie_proc_he_opern_ie(mac_vap_stru *mac_vap, uint8_t *payload, mac_user_stru *mac_user)
{
    mac_frame_he_oper_ie_stru st_he_oper_ie_value;
    mac_he_hdl_stru st_he_hdl;
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr3(mac_vap, mac_user, payload))) {
        oam_error_log0(0, OAM_SF_11AX, "{mac_ie_proc_he_opern_ie::mac_vap or mac_user or payload is null.}");
        return MAC_NO_CHANGE;
    }

    memset_s(&st_he_oper_ie_value, sizeof(st_he_oper_ie_value), 0, sizeof(st_he_oper_ie_value));
    ret = mac_ie_parse_he_oper(payload, &st_he_oper_ie_value);
    if (ret != OAL_SUCC) {
        return MAC_NO_CHANGE;
    }

    mac_user_get_he_hdl(mac_user, &st_he_hdl);

    /* 解析到he_opern_ie，即存在he_duration_rts_threshold */
    st_he_hdl.bit_he_duration_rts_threshold_exist = 1;

    if (st_he_oper_ie_value.st_bss_color.bit_bss_color != st_he_hdl.st_he_oper_ie.st_bss_color.bit_bss_color) {
        ret |= MAC_HE_BSS_COLOR_CHANGE;
        /* 识别bss color需要标记,dmac设置 */
        st_he_hdl.bit_he_oper_bss_color_exist = OAL_TRUE;
    }

    if (st_he_oper_ie_value.st_bss_color.bit_partial_bss_color !=
        st_he_hdl.st_he_oper_ie.st_bss_color.bit_partial_bss_color) {
        ret |= MAC_HE_PARTIAL_BSS_COLOR_CHANGE;
    }

    st_he_hdl.st_he_oper_ie = st_he_oper_ie_value;

    mac_user_set_he_hdl(mac_user, &st_he_hdl);

    return ret;
}


uint32_t mac_ie_parse_he_ndp_feedback_report_ie(uint8_t *he_ndp_ie, uint8_t *nfrp_buff_threshold_exp)
{
    uint8_t *data = NULL;

    if (he_ndp_ie[1] != MAC_HE_NDP_FEEDBACK_REPORT_LEN) {
        return OAL_FAIL;
    }

    data = he_ndp_ie + MAC_HE_NDP_FEEDBACK_REPORT_OFFSET;

    *nfrp_buff_threshold_exp = *data;

    return OAL_SUCC;
}


uint32_t mac_ie_parse_he_bss_color_change_announcement_ie(uint8_t *payload,
    mac_frame_bss_color_change_annoncement_ie_stru *pst_bss_color)
{
    mac_frame_bss_color_change_annoncement_ie_stru *pst_bss_color_info = NULL;
    uint8_t *data = NULL;

    if (payload[1] != MAC_HE_BSS_COLOR_CHANGE_ANNOUNCEMENT_LEN) {
        return OAL_FAIL;
    }

    data = payload + IE_BUFFER_HEADER;

    pst_bss_color_info = (mac_frame_bss_color_change_annoncement_ie_stru*)data;
    if (EOK != memcpy_s(pst_bss_color, sizeof(mac_frame_bss_color_change_annoncement_ie_stru), pst_bss_color_info,
        sizeof(mac_frame_bss_color_change_annoncement_ie_stru))) {
        oam_error_log0(0, OAM_SF_ANY, "mac_ie_parse_he_bss_color_change_announcement_ie::memcpy fail!");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


uint32_t mac_ie_parse_multi_bssid_opt_subie(uint8_t *frame_data, mac_multi_bssid_frame_info_stru *mbssid_frame_info,
    uint8_t *mbssid_body_ie_len)
{
    uint8_t *data = frame_data;
    uint8_t *ssid = NULL;
    uint8_t *non_trans_bssid_cap = NULL;
    uint8_t *mbssid_idx = NULL;
    uint8_t ie_len, left_len, bssid_profile_len;

    /*
     * -------------------------------------------------------------------------
     * |EID |Length |Max BSSID Indicator| Non-Transmitted BSSID Profile ID| BSSID Profile Len|
     * -------------------------------------------------------------------------
     * |1   |1      |1                  |               1                 |    1
     * -------------------------------------------------------------------------
     * |sub_ssid |Length | sub_ssid|
     * -------------------------------------------------------------------------
     * |1        |1      |    n    |
     * -------------------------------------------------------------------------
     * |sub_nonTxBssid_Cap |Length |Nontransmitted bssid CAP     |
     * -------------------------------------------------------------------------
     * |1                  |1      |    2                        |
     * -------------------------------------------------------------------------
     * |sub_Multi BSSID Index |Length |bssid index|
     * -------------------------------------------------------------------------
     * |1                     |1      |    1      |
     * 认证用例规定Non-transmitted BSSID Profile 至少要包含 NonTxBSSID Cap IE(4), SSID IE(34Bytes),M-BSSID Index IE(3)
     */
    /* 判断Non-transmitted BSSID Profile 是否存在 */
    if (data[0] != 0) {
        oam_warning_log0(0, OAM_SF_11AX, "{mac_ie_parse_multi_bssid_ie:: bssid profile not exist.}");
        return OAL_FAIL;
    }
    bssid_profile_len = data[1];
    data += MAC_IE_HDR_LEN;
    left_len = bssid_profile_len;

    /* 解析non-transmitted bssid cap IE83 */
    non_trans_bssid_cap = mac_find_ie(MAC_EID_NONTRANSMITTED_BSSID_CAP, data, left_len);
    /* 2：NonTxBSSID Cap IE长度(4) - MAC_IE_HDR_LEN */
    if (non_trans_bssid_cap == NULL || non_trans_bssid_cap[1] < 2) {
        oam_warning_log0(0, OAM_SF_11AX, "{mac_ie_parse_multi_bssid_ie:: parase ie83 fail.}");
        return OAL_FAIL;
    }
    ie_len = non_trans_bssid_cap[1];

    mbssid_frame_info->us_non_tramsmitted_bssid_cap = *(uint16_t*)(non_trans_bssid_cap + MAC_IE_HDR_LEN);
    data += (ie_len + MAC_IE_HDR_LEN);
    left_len -= (ie_len + MAC_IE_HDR_LEN);

    /* 解析 ssid元素 */
    ssid = mac_find_ie(MAC_EID_SSID, data, left_len);
    if (ssid == NULL || ssid[1] > WLAN_SSID_MAX_LEN) {
        return OAL_FAIL;
    }
    ie_len = ssid[1];
    if (EOK != memcpy_s(mbssid_frame_info->auc_non_transmitted_ssid, WLAN_SSID_MAX_LEN, ssid + MAC_IE_HDR_LEN,
        ie_len)) {
        oam_warning_log0(0, OAM_SF_11AX, "mac_ie_parse_multi_bssid_ie::memcpy fail!");
        return OAL_FAIL;
    }
    mbssid_frame_info->uc_non_transmitted_ssid_len = ie_len;

    data += (ie_len + MAC_IE_HDR_LEN);
    left_len -= (ie_len + MAC_IE_HDR_LEN);

    /* 解析Non-transmitted BSSID Profile 中m-bssid index ie85 */
    mbssid_idx = mac_find_ie(MAC_EID_MULTI_BSSID_INDEX, data, left_len);
    /* 1：M-BSSID Index IE(3)长度 - MAC_IE_HDR_LEN */
    if (mbssid_idx == NULL || mbssid_idx[1] < 1) {
        oam_warning_log0(0, OAM_SF_11AX, "{mac_ie_parse_multi_bssid_ie:: parase ie85 fail[%d].}");
        return OAL_FAIL;
    }

    mbssid_frame_info->uc_bssid_index = mbssid_idx[MAC_IE_HDR_LEN];

    *mbssid_body_ie_len = bssid_profile_len + MAC_IE_HDR_LEN;

    return OAL_SUCC;
}
#endif // _PRE_WLAN_FEATURE_11AX

uint32_t mac_ie_proc_ext_cap_ie(mac_user_stru *mac_user, uint8_t *payload)
{
    mac_user_cap_info_stru *cap_info = NULL;
    uint8_t len;
    uint8_t cap[NUM_8_BITS] = { 0 };

    if (oal_unlikely(oal_any_null_ptr2(mac_user, payload))) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_ie_proc_ext_cap_ie::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    cap_info = &(mac_user->st_cap_info);
    len = payload[BYTE_OFFSET_1];
    if (len >= MAC_IE_HDR_LEN && len <= 8) { /* ie除头以外，长度最大为8 */
        /* ie长度域的值本身不包含IE头长度，此处不需要另行减去头长 */
        if (memcpy_s(cap, sizeof(cap), &payload[MAC_IE_HDR_LEN], len) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "mac_ie_proc_ext_cap_ie::memcpy fail!");
            return OAL_FAIL;
        }
    }

    /* 提取 BIT12: 支持proxy arp */
    cap_info->bit_proxy_arp = ((cap[BYTE_OFFSET_1] & BIT4) == 0) ? OAL_FALSE : OAL_TRUE;
#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    /* 提取 BIT19: 支持bss transition */
    cap_info->bit_bss_transition = ((cap[BYTE_OFFSET_2] & BIT3) == 0) ? OAL_FALSE : OAL_TRUE;
#endif
    return OAL_SUCC;
}


uint8_t *mac_ie_find_vendor_vht_ie(uint8_t *frame, uint16_t frame_len)
{
    uint8_t  *vendor_ie = NULL;
    uint8_t  *vht_ie = NULL;
    uint16_t offset_vendor_vht = MAC_WLAN_OUI_VENDOR_VHT_HEADER + MAC_IE_HDR_LEN;

    vendor_ie =
        mac_find_vendor_ie(MAC_WLAN_OUI_BROADCOM_EPIGRAM, MAC_WLAN_OUI_VENDOR_VHT_TYPE, frame, frame_len);
    if ((vendor_ie != NULL) && (vendor_ie[1] >= MAC_WLAN_OUI_VENDOR_VHT_HEADER)) {
        vht_ie = mac_find_ie(MAC_EID_VHT_CAP, vendor_ie + offset_vendor_vht,
            vendor_ie[1] - MAC_WLAN_OUI_VENDOR_VHT_HEADER);
    }

    return vht_ie;
}


uint16_t mac_ie_get_max_mpdu_len_by_vht_cap(uint8_t vht_cap_max_mpdu_len_bit)
{
    uint16_t max_mpdu_length = 3895; /* 默认支持最大的mpdu长度 */

    if (vht_cap_max_mpdu_len_bit == 0) { /* 支持的最大mpdu len,0:3895 */
        max_mpdu_length = 3895;
    } else if (vht_cap_max_mpdu_len_bit == 1) { /* 支持的最大mpdu len,1:7991 */
        max_mpdu_length = 7991;
    } else if (vht_cap_max_mpdu_len_bit == 2) { /* 支持的最大mpdu len,2:11454 */
        max_mpdu_length = 11454;
    }

    return max_mpdu_length;
}

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY

OAL_STATIC uint32_t mac_ie_check_proc_opmode_param(mac_user_stru *mac_user,
    mac_opmode_notify_stru *opmode_notify)
{
    /* USER新限定带宽、空间流不允许大于其能力 */
    if ((mac_user->en_bandwidth_cap < opmode_notify->bit_channel_width) ||
        (mac_user->en_user_max_cap_nss < opmode_notify->bit_rx_nss)) {
        /* p20pro 2G 1*1热点，beacon携带opmode为80M,造成此处会刷屏,属于对端异常 */
        oam_warning_log4(mac_user->uc_vap_id, OAM_SF_OPMODE,
            "{mac_ie_check_proc_opmode_param::bw or nss over limit! work bw[%d]opmode bw[%d]user_nss[%d]rx_nss[%d]!}",
            mac_user->en_bandwidth_cap, opmode_notify->bit_channel_width, mac_user->en_user_max_cap_nss,
            opmode_notify->bit_rx_nss);

        return OAL_FAIL;
    }

    /* Nss Type值为1，则表示beamforming Rx Nss不能超过其声称值 */
    if (opmode_notify->bit_rx_nss_type == 1) {
        if (mac_user->st_vht_hdl.bit_num_bf_ant_supported < opmode_notify->bit_rx_nss) {
            oam_warning_log2(mac_user->uc_vap_id, OAM_SF_OPMODE,
                "{mac_ie_check_proc_opmode_param::rx_nss is over limit!bit_num_bf_ant_supported[%d], bit_rx_nss[%d]!}",
                mac_user->st_vht_hdl.bit_num_bf_ant_supported, opmode_notify->bit_rx_nss);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC wlan_bw_cap_enum_uint8 mac_ie_proc_opmode_channel_width(mac_user_stru *mac_user,
    mac_opmode_notify_stru *opmode_notify)
{
    wlan_bw_cap_enum_uint8 opmode_notify_bw;

    if (opmode_notify->bit_channel_width == WLAN_BW_CAP_80M) {
        opmode_notify_bw = (opmode_notify->bit_160or8080) ? WLAN_BW_CAP_160M : WLAN_BW_CAP_80M;
    } else {
        opmode_notify_bw = opmode_notify->bit_channel_width;
    }

    return opmode_notify_bw;
}

OAL_STATIC void mac_ie_proc_opmode_nss_handle(mac_vap_stru *mac_vap, mac_user_stru *mac_user,
    mac_opmode_notify_stru *opmode_notify, oal_bool_enum_uint8 from_beacon)
{
    wlan_nss_enum_uint8 avail_nss;

    avail_nss = oal_min(mac_vap->en_vap_rx_nss, opmode_notify->bit_rx_nss);
    mac_user_set_smps_opmode_notify_nss(mac_user, opmode_notify->bit_rx_nss);
    if (avail_nss != mac_user->en_avail_num_spatial_stream) {
#ifdef _PRE_WLAN_FEATURE_11AX
        if (g_wlan_spec_cfg->feature_11ax_is_open) {
            /* beacon 帧中的opmode && he_rom_nss < en_opmode_notify_nss 不做处理 */
            if (from_beacon && mac_user->bit_have_recv_he_rom_flag &&
                mac_user->en_he_rom_nss < avail_nss) {
                return;
            }
        }
#endif
        /* 需要获取vap和更新nss的取小，如果我们不支持mimo了，对端宣传切换mimo也不执行 */
        mac_user_set_avail_num_spatial_stream(mac_user, avail_nss);
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_OPMODE,
            "{mac_ie_proc_opmode_nss_handle::change nss.en_vap_rx_nss=[%x],\
            en_avail_num_spatial_stream=[%d],opmode_notify->bit_rx_nss=[%d],\
            bit_smps_opmode_notify_nss=[%d]!}",
            mac_vap->en_vap_rx_nss, mac_user->en_avail_num_spatial_stream, opmode_notify->bit_rx_nss,
            mac_user_get_smps_opmode_notify_nss(mac_user));
    }
}


uint32_t mac_ie_proc_opmode_field(mac_vap_stru *mac_vap, mac_user_stru *mac_user,
    mac_opmode_notify_stru *opmode_notify, oal_bool_enum_uint8 from_beacon)
{
    wlan_bw_cap_enum_uint8 vap_bw_cap = 0; /* vap自身带宽能力 */
    wlan_bw_cap_enum_uint8 avail_bw; /* vap自身带宽能力 */
    wlan_bw_cap_enum_uint8 opmode_notify_bw;

    /* 入参指针已经在调用函数保证非空，这里直接使用即可 */
    if (mac_ie_check_proc_opmode_param(mac_user, opmode_notify) == OAL_FAIL) {
        return OAL_FAIL;
    }

    opmode_notify_bw = mac_ie_proc_opmode_channel_width(mac_user, opmode_notify);
    /* 判断channel_width是否与user之前使用channel_width相同 */
    if (opmode_notify_bw != mac_user->en_avail_bandwidth) {
        /* 获取vap带宽能力与用户带宽能力的交集 */
        mac_vap_get_bandwidth_cap(mac_vap, &vap_bw_cap);
        if (vap_bw_cap == WLAN_BW_CAP_160M && opmode_notify_bw == WLAN_BW_CAP_80M &&
            mac_user->en_avail_bandwidth == WLAN_BW_CAP_160M) {
            avail_bw = oal_min(vap_bw_cap, WLAN_BW_CAP_160M);
        } else {
            avail_bw = oal_min(vap_bw_cap, opmode_notify_bw);
        }
#ifdef _PRE_WLAN_FEATURE_11AX
        if (g_wlan_spec_cfg->feature_11ax_is_open) {
            /* beacon 帧中的opmode && he_rom_bw < opmode_notify_bw 不做处理 */
            if (from_beacon && mac_user->bit_have_recv_he_rom_flag &&
                mac_user->en_he_rom_bw < avail_bw) {
                return OAL_SUCC;
            }
        }
#endif
        mac_user_set_bandwidth_info(mac_user, avail_bw, avail_bw);

        oam_info_log2(mac_vap->uc_vap_id, OAM_SF_OPMODE,
            "{mac_ie_proc_opmode_field::change bandwidth. vap_bw_cap[%x], user avail bandwidth = [%x]!}",
            vap_bw_cap, mac_user->en_avail_bandwidth);
    }

    /* 判断Rx Nss Type是否为beamforming模式 */
    if (opmode_notify->bit_rx_nss_type == 1) {
        oam_info_log0(mac_vap->uc_vap_id, OAM_SF_OPMODE,
            "{mac_ie_proc_opmode_field::opmode_notify->bit_rx_nss_type == 1!}\r\n");

        /* 判断Rx Nss是否与user之前使用Rx Nss相同 */
        if (opmode_notify->bit_rx_nss != mac_user->en_avail_bf_num_spatial_stream) {
            /* 需要获取vap和更新nss的取小，如果我们不支持mimo了，对端宣传切换mimo也不执行 */
            mac_user_avail_bf_num_spatial_stream(mac_user,
                oal_min(mac_vap->en_vap_rx_nss, opmode_notify->bit_rx_nss));
        }
    } else {
        mac_ie_proc_opmode_nss_handle(mac_vap, mac_user, opmode_notify, from_beacon);
    }

    return OAL_SUCC;
}
#endif


uint8_t mac_ie_get_chan_num(uint8_t *frame_body, uint16_t frame_len,
                            uint16_t offset, uint8_t curr_chan)
{
    uint8_t chan_num;
    uint8_t *ie_start_addr;

    /* 在DSSS Param set ie中解析chan num */
    ie_start_addr = mac_find_ie(MAC_EID_DSPARMS, frame_body + offset, frame_len - offset);
    if ((ie_start_addr != NULL) && (ie_start_addr[1] == MAC_DSPARMS_LEN)) {
        chan_num = ie_start_addr[BYTE_OFFSET_2]; /* ie第2字节包含的信息是信道号 */
        if (mac_is_channel_num_valid(mac_get_band_by_channel_num(chan_num), chan_num, OAL_FALSE) == OAL_SUCC) {
            return chan_num;
        }
    }

    /* 在HT operation ie中解析 chan num */
    ie_start_addr = mac_find_ie(MAC_EID_HT_OPERATION, frame_body + offset, frame_len - offset);
    if ((ie_start_addr != NULL) && (ie_start_addr[1] >= 1)) {
        chan_num = ie_start_addr[BYTE_OFFSET_2]; /* ie第2字节包含的信息是信道号 */
        if (mac_is_channel_num_valid(mac_get_band_by_channel_num(chan_num), chan_num, OAL_FALSE) == OAL_SUCC) {
            return chan_num;
        }
    }

    chan_num = curr_chan;
    return chan_num;
}


uint32_t mac_set_second_channel_offset_ie(wlan_channel_bandwidth_enum_uint8 en_bw,
                                          uint8_t *buffer,
                                          uint8_t *output_len)
{
    if (oal_any_null_ptr2(buffer, output_len)) {
        oam_error_log0(0, OAM_SF_SCAN, "{mac_set_second_channel_offset_ie::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 默认输出为空 */
    *buffer = '\0';
    *output_len = 0;

    /* 11n 设置Secondary Channel Offset Element */
    /******************************************************************/
    /* -------------------------------------------------------------- */
    /* |Ele. ID |Length |Secondary channel offset |                   */
    /* -------------------------------------------------------------- */
    /* |1       |1      |1                        |                   */
    /*                                                                */
    /******************************************************************/
    buffer[BYTE_OFFSET_0] = 62; /* buffer[0]是Ele.ID字段,62是ID号 */
    buffer[BYTE_OFFSET_1] = 1; /* 1表示除IE头以外，有1个字节 */

    switch (en_bw) {
        case WLAN_BAND_WIDTH_20M:
            buffer[BYTE_OFFSET_2] = 0; /* no secondary channel */
            break;
        case WLAN_BAND_WIDTH_40PLUS:
        case WLAN_BAND_WIDTH_80PLUSPLUS:
        case WLAN_BAND_WIDTH_80PLUSMINUS:
#ifdef _PRE_WLAN_FEATURE_160M
        case WLAN_BAND_WIDTH_160PLUSPLUSPLUS:
        case WLAN_BAND_WIDTH_160PLUSPLUSMINUS:
        case WLAN_BAND_WIDTH_160PLUSMINUSPLUS:
        case WLAN_BAND_WIDTH_160PLUSMINUSMINUS:
#endif
            buffer[BYTE_OFFSET_2] = 1; /* secondary 20M channel above */
            break;
        case WLAN_BAND_WIDTH_40MINUS:
        case WLAN_BAND_WIDTH_80MINUSPLUS:
        case WLAN_BAND_WIDTH_80MINUSMINUS:
#ifdef _PRE_WLAN_FEATURE_160M
        case WLAN_BAND_WIDTH_160MINUSPLUSPLUS:
        case WLAN_BAND_WIDTH_160MINUSPLUSMINUS:
        case WLAN_BAND_WIDTH_160MINUSMINUSPLUS:
        case WLAN_BAND_WIDTH_160MINUSMINUSMINUS:
#endif
            buffer[BYTE_OFFSET_2] = 3; /* 3表示secondary 20M channel below */
            break;
        default:
            oam_error_log1(0, OAM_SF_SCAN, "{mac_set_second_channel_offset_ie::invalid bandwidth[%d].}", en_bw);
            return OAL_FAIL;
    }

    *output_len = 3; /* 输出的buffer长度为3 */

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_160M
static uint32_t mac_set_new_ch_width_160m(uint8_t channel, wlan_channel_bandwidth_enum_uint8 en_bw, uint8_t *buffer)
{
    /* buffer[2]表示New Ch width字段,0表示没有新带宽，1表示有 */
    /* buffer[3]表示第1组带宽的中心频率对应的信道(Center Freq seg1)字段 */
    /* buffer[4]表示第2组带宽的中心频率对应的信道(Center Freq seg2)字段 */
    /* channel + n 代表中心频率增加n*5，每个信道增量为5MHZ */
    switch (en_bw) {
        case WLAN_BAND_WIDTH_160PLUSPLUSPLUS:
            buffer[BYTE_OFFSET_2] = 1;
            buffer[BYTE_OFFSET_3] = channel + CHAN_OFFSET_PLUS_6;
            buffer[BYTE_OFFSET_4] = channel + CHAN_OFFSET_PLUS_14;
            break;
        case WLAN_BAND_WIDTH_160PLUSPLUSMINUS:
            buffer[BYTE_OFFSET_2] = 1;
            buffer[BYTE_OFFSET_3] = channel + CHAN_OFFSET_PLUS_6;
            buffer[BYTE_OFFSET_4] = channel + CHAN_OFFSET_MINUS_2;
            break;
        /* 从20信道+1, 从40信道-1, 从80信道+1 */
        case WLAN_BAND_WIDTH_160PLUSMINUSPLUS:
            buffer[BYTE_OFFSET_2] = 1;
            buffer[BYTE_OFFSET_3] = channel + CHAN_OFFSET_MINUS_2;
            buffer[BYTE_OFFSET_4] = channel + CHAN_OFFSET_PLUS_6;
            break;
        /* 从20信道+1, 从40信道-1, 从80信道-1 */
        case WLAN_BAND_WIDTH_160PLUSMINUSMINUS:
            buffer[BYTE_OFFSET_2] = 1;
            buffer[BYTE_OFFSET_3] = channel + CHAN_OFFSET_MINUS_2;
            buffer[BYTE_OFFSET_4] = channel + CHAN_OFFSET_MINUS_10;
            break;
        /* 从20信道-1, 从40信道+1, 从80信道+1 */
        case WLAN_BAND_WIDTH_160MINUSPLUSPLUS:
            buffer[BYTE_OFFSET_2] = 1;
            buffer[BYTE_OFFSET_3] = channel + CHAN_OFFSET_PLUS_2;
            buffer[BYTE_OFFSET_4] = channel + CHAN_OFFSET_PLUS_10;
            break;
        /* 从20信道-1, 从40信道+1, 从80信道-1 */
        case WLAN_BAND_WIDTH_160MINUSPLUSMINUS:
            buffer[BYTE_OFFSET_2] = 1;
            buffer[BYTE_OFFSET_3] = channel + CHAN_OFFSET_PLUS_2;
            buffer[BYTE_OFFSET_4] = channel + CHAN_OFFSET_MINUS_6;
            break;
        /* 从20信道-1, 从40信道-1, 从80信道+1 */
        case WLAN_BAND_WIDTH_160MINUSMINUSPLUS:
            buffer[BYTE_OFFSET_2] = 1;
            buffer[BYTE_OFFSET_3] = channel + CHAN_OFFSET_MINUS_6;
            buffer[BYTE_OFFSET_4] = channel + CHAN_OFFSET_PLUS_2;
            break;
        /* 从20信道-1, 从40信道-1, 从80信道-1 */
        case WLAN_BAND_WIDTH_160MINUSMINUSMINUS:
            buffer[BYTE_OFFSET_2] = 1;
            buffer[BYTE_OFFSET_3] = channel + CHAN_OFFSET_MINUS_6;
            buffer[BYTE_OFFSET_4] = channel + CHAN_OFFSET_MINUS_14;
            break;
        default:
            oam_error_log1(0, OAM_SF_SCAN, "{mac_set_new_ch_width_160M::invalid bandwidth[%d].}", en_bw);
            return OAL_FAIL;
    }
    return OAL_SUCC;
}
#endif

static uint32_t mac_set_new_ch_width(uint8_t channel,
                                     wlan_channel_bandwidth_enum_uint8 en_bw,
                                     uint8_t *buffer)
{
    uint32_t ret = OAL_SUCC;
    /* buffer[2]表示New Ch width字段,0表示没有新带宽，1表示有 */
    /* buffer[3]表示第1组带宽的中心频率对应的信道(Center Freq seg1)字段 */
    /* channel + n 代表中心频率增加n*5，每个信道增量为5MHZ */
    switch (en_bw) {
        case WLAN_BAND_WIDTH_20M:
        case WLAN_BAND_WIDTH_40PLUS:
        case WLAN_BAND_WIDTH_40MINUS:
            buffer[BYTE_OFFSET_2] = 0;
            buffer[BYTE_OFFSET_3] = 0;
            break;
        case WLAN_BAND_WIDTH_80PLUSPLUS:
            buffer[BYTE_OFFSET_2] = 1;
            buffer[BYTE_OFFSET_3] = channel + CHAN_OFFSET_PLUS_6;
            break;
        case WLAN_BAND_WIDTH_80PLUSMINUS:
            buffer[BYTE_OFFSET_2] = 1;
            buffer[BYTE_OFFSET_3] = channel + CHAN_OFFSET_MINUS_2;
            break;
        case WLAN_BAND_WIDTH_80MINUSPLUS:
            buffer[BYTE_OFFSET_2] = 1;
            buffer[BYTE_OFFSET_3] = channel + CHAN_OFFSET_PLUS_2;
            break;
        case WLAN_BAND_WIDTH_80MINUSMINUS:
            buffer[BYTE_OFFSET_2] = 1;
            buffer[BYTE_OFFSET_3] = channel + CHAN_OFFSET_MINUS_6;
            break;
        default:
#ifdef _PRE_WLAN_FEATURE_160M
            ret = mac_set_new_ch_width_160m(channel, en_bw, buffer);
#else
            oam_error_log1(0, OAM_SF_SCAN, "{mac_set_new_ch_width::invalid bandwidth[%d].}", en_bw);
            return OAL_FAIL;
#endif
    }
    return ret;
}

uint32_t mac_set_11ac_wideband_ie(uint8_t channel,
                                  wlan_channel_bandwidth_enum_uint8 en_bw,
                                  uint8_t *buffer,
                                  uint8_t *output_len)
{
    if (oal_any_null_ptr2(buffer, output_len)) {
        oam_error_log0(0, OAM_SF_SCAN, "{mac_set_11ac_wideband_ie::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 默认输出为空 */
    *buffer = '\0';
    *output_len = 0;

    /* 11ac 设置Wide Bandwidth Channel Switch Element                 */
    /******************************************************************/
    /* -------------------------------------------------------------- */
    /* |ID |Length |New Ch width |Center Freq seg1 |Center Freq seg2  */
    /* -------------------------------------------------------------- */
    /* |1  |1      |1            |1                |1                 */
    /*                                                                */
    /******************************************************************/
    buffer[BYTE_OFFSET_0] = 194; /* 194是ID号 */
    buffer[BYTE_OFFSET_1] = 3; /* 3是除IE头以外的长度 */

    if (mac_set_new_ch_width(channel, en_bw, buffer) != OAL_SUCC) {
        return OAL_FAIL;
    }

    if ((en_bw < WLAN_BAND_WIDTH_160PLUSPLUSPLUS) || (en_bw > WLAN_BAND_WIDTH_160MINUSMINUSMINUS)) {
        buffer[BYTE_OFFSET_4] = 0; /* reserved. Not support 80M + 80M */
    }

    *output_len = 5; /* 输出的buffer长度为5 */

    return OAL_SUCC;
}


uint32_t mac_ie_proc_chwidth_field(mac_vap_stru *mac_vap, mac_user_stru *mac_user, uint8_t uc_chwidth)
{
    wlan_bw_cap_enum_uint8 vap_bw_cap = 0; /* vap自身带宽能力 */

    if (oal_unlikely(oal_any_null_ptr2(mac_vap, mac_user))) {
        oam_error_log0(0, OAM_SF_2040, "{mac_ie_proc_opmode_field::mac_user or mac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap_get_bandwidth_cap(mac_vap, &vap_bw_cap);
    vap_bw_cap = oal_min(vap_bw_cap, (wlan_bw_cap_enum_uint8)uc_chwidth);
    mac_user_set_bandwidth_info(mac_user, vap_bw_cap, vap_bw_cap);

    return OAL_SUCC;
}


uint32_t mac_proc_ht_opern_ie(mac_vap_stru *mac_vap, uint8_t *payload, mac_user_stru *mac_user)
{
    mac_ht_opern_ac_stru        *ht_opern = NULL; // todo::待将两个结构体整改成一个结构体
    mac_user_ht_hdl_stru         ht_hdl;
    uint32_t                     change = MAC_NO_CHANGE;
    mac_sec_ch_off_enum_uint8    secondary_chan_offset_old;
    uint8_t                      sta_chan_width_old;

    if (oal_unlikely(oal_any_null_ptr3(mac_vap, payload, mac_user))) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_proc_ht_opern_ie::param null.}");
        return change;
    }

    if (payload[BYTE_OFFSET_1] < 6) { /* 长度校验，此处仅用到前6字节，后面Basic MCS Set未涉及 */
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{mac_proc_ht_opern_ie::invalid ht opern ie len[%d].}", payload[BYTE_OFFSET_1]);
        return change;
    }

    mac_user_get_ht_hdl(mac_user, &ht_hdl);

    secondary_chan_offset_old = ht_hdl.bit_secondary_chan_offset;
    sta_chan_width_old = ht_hdl.bit_sta_chan_width;

    /************************ HT Operation Element *************************************
      ----------------------------------------------------------------------
      |EID |Length |PrimaryChannel |HT Operation Information |Basic MCS Set|
      ----------------------------------------------------------------------
      |1   |1      |1              |5                        |16           |
      ----------------------------------------------------------------------
    ***************************************************************************/
    /************************ HT Information Field ****************************
     |--------------------------------------------------------------------|
     | Primary | Seconday  | STA Ch | RIFS |           reserved           |
     | Channel | Ch Offset | Width  | Mode |                              |
     |--------------------------------------------------------------------|
     |    1    | B0     B1 |   B2   |  B3  |    B4                     B7 |
     |--------------------------------------------------------------------|

     |----------------------------------------------------------------|
     |     HT     | Non-GF STAs | resv      | OBSS Non-HT  | Reserved |
     | Protection |   Present   |           | STAs Present |          |
     |----------------------------------------------------------------|
     | B0     B1  |     B2      |    B3     |     B4       | B5   B15 |
     |----------------------------------------------------------------|

     |-------------------------------------------------------------|
     | Reserved |  Dual  |  Dual CTS  | Seconday | LSIG TXOP Protn |
     |          | Beacon | Protection |  Beacon  | Full Support    |
     |-------------------------------------------------------------|
     | B0    B5 |   B6   |     B7     |     B8   |       B9        |
     |-------------------------------------------------------------|

     |---------------------------------------|
     |  PCO   |  PCO  | Reserved | Basic MCS |
     | Active | Phase |          |    Set    |
     |---------------------------------------|
     |  B10   |  B11  | B12  B15 |    16     |
     |---------------------------------------|
    **************************************************************************/
    ht_opern = (mac_ht_opern_ac_stru *)(&payload[MAC_IE_HDR_LEN]);

    /* 提取HT Operation IE中的"Secondary Channel Offset" */
    ht_hdl.bit_secondary_chan_offset = ht_opern->bit_secondary_chan_offset;

    /* 在2.4G用户声称20M情况下该变量不切换 */
    if ((ht_opern->bit_sta_chan_width == WLAN_BAND_WIDTH_20M) &&
        (mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
        ht_hdl.bit_secondary_chan_offset = MAC_SCN;
    }

    /* 保护相关 */
    ht_hdl.bit_rifs_mode = ht_opern->bit_rifs_mode; /* 发送描述符填写时候需要此值 */
    ht_hdl.bit_ht_protection = ht_opern->bit_ht_protection;
    ht_hdl.bit_nongf_sta_present = ht_opern->bit_nongf_sta_present;
    ht_hdl.bit_obss_nonht_sta_present = ht_opern->bit_obss_nonht_sta_present;
    ht_hdl.bit_lsig_txop_protection_full_support = ht_opern->bit_lsig_txop_protection_full_support;
    ht_hdl.bit_sta_chan_width = ht_opern->bit_sta_chan_width;
    ht_hdl.uc_chan_center_freq_seg2 = ht_opern->bit_chan_center_freq_seg2;

    mac_user_set_ht_hdl(mac_user, &ht_hdl);

    if ((secondary_chan_offset_old != ht_hdl.bit_secondary_chan_offset) ||
        (sta_chan_width_old != ht_hdl.bit_sta_chan_width)) {
        change = MAC_HT_CHANGE;
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "mac_proc_ht_opern_ie:usr_bw is updated second_chan_offset from [%d] to [%d],   \
                          chan_with from [%d] to [%d]",
                         secondary_chan_offset_old, ht_hdl.bit_secondary_chan_offset,
                         sta_chan_width_old, ht_hdl.bit_sta_chan_width);
    }

    return change;
}


uint32_t mac_ie_proc_obss_scan_ie(mac_vap_stru *mac_vap, uint8_t *payload)
{
    uint16_t trigger_scan_interval;

    if (oal_unlikely(oal_any_null_ptr2(mac_vap, payload))) {
        oam_error_log0(0, OAM_SF_SCAN, "{mac_ie_proc_obss_scan_ie::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /********************Overlapping BSS Scan Parameters element******************
     |ElementID |Length |OBSS    |OBSS   |BSS Channel   |OBSS Scan  |OBSS Scan   |
     |          |       |Scan    |Scan   |Width Trigger |Passive    |Active Total|
     |          |       |Passive |Active |Scan Interval |Total Per  |Per         |
     |          |       |Dwell   |Dwell  |              |Channel    |Channel     |
     ----------------------------------------------------------------------------
     |1         |1      |2       |2      |2             |2          |2           |
     ----------------------------------------------------------------------------
     |BSS Width   |OBSS Scan|
     |Channel     |Activity |
     |Transition  |Threshold|
     |Delay Factor|         |
     ------------------------
     |2           |2        |
    ***************************************************************************/
    if (payload[1] < MAC_OBSS_SCAN_IE_LEN) {
        return OAL_FAIL;
    }

    trigger_scan_interval = oal_make_word16(payload[BYTE_OFFSET_6], payload[BYTE_OFFSET_7]);
    if (trigger_scan_interval == 0) {
        mac_vap_set_peer_obss_scan(mac_vap, OAL_FALSE);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    mac_mib_set_OBSSScanPassiveDwell(mac_vap,
        oal_make_word16(payload[BYTE_OFFSET_2], payload[BYTE_OFFSET_3]));
    mac_mib_set_OBSSScanActiveDwell(mac_vap,
        oal_make_word16(payload[BYTE_OFFSET_4], payload[BYTE_OFFSET_5]));
    /* obss扫描周期最小180秒,最大600S, 初始化默认为300秒 */
    mac_mib_set_BSSWidthTriggerScanInterval(mac_vap, oal_min(oal_max(trigger_scan_interval, 180), 600));
    mac_mib_set_OBSSScanPassiveTotalPerChannel(mac_vap,
        oal_make_word16(payload[BYTE_OFFSET_8], payload[BYTE_OFFSET_9]));
    mac_mib_set_OBSSScanActiveTotalPerChannel(mac_vap,
        oal_make_word16(payload[BYTE_OFFSET_10], payload[BYTE_OFFSET_11]));
    mac_mib_set_BSSWidthChannelTransitionDelayFactor(mac_vap,
        oal_make_word16(payload[BYTE_OFFSET_12], payload[BYTE_OFFSET_13]));
    mac_mib_set_OBSSScanActivityThreshold(mac_vap,
        oal_make_word16(payload[BYTE_OFFSET_14], payload[BYTE_OFFSET_15]));
    mac_vap_set_peer_obss_scan(mac_vap, OAL_TRUE);

    return OAL_SUCC;
}


uint32_t mac_ie_proc_vht_opern_ie(mac_vap_stru *mac_vap, uint8_t *payload, mac_user_stru *mac_user)
{
    mac_vht_hdl_stru                    vht_hdl;
    uint8_t                           ret = MAC_NO_CHANGE;
    uint16_t                          basic_mcs_set_all_user;
    wlan_mib_vht_op_width_enum    channel_width_old;
    uint8_t                           channel_center_freq_seg0_old;

    if (oal_unlikely(oal_any_null_ptr2(mac_vap, payload))) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_ie_proc_vht_opern_ie::param null.}");
        return ret;
    }

    /* 长度校验 */
    if (payload[BYTE_OFFSET_1] < MAC_VHT_OPERN_LEN) {
        oam_warning_log1(0, OAM_SF_ANY, "{mac_ie_proc_vht_opern_ie::invalid vht opern len[%d].}", payload[1]);
        return ret;
    }

    mac_user_get_vht_hdl(mac_user, &vht_hdl);

    channel_width_old = vht_hdl.en_channel_width;
    channel_center_freq_seg0_old = vht_hdl.uc_channel_center_freq_seg0;

    /* 解析 "VHT Operation Information" */
    vht_hdl.en_channel_width = payload[MAC_IE_HDR_LEN];
    vht_hdl.uc_channel_center_freq_seg0 = payload[MAC_IE_HDR_LEN + 1]; /* 第1组带宽的中心频率 */
    vht_hdl.uc_channel_center_freq_seg1 = payload[MAC_IE_HDR_LEN + 2]; /* 第2组带宽的中心频率 */

    /* 0 -- 20/40M, 1 -- 80M,160M,80+80 2 -- 160M, 3--80M+80M */
    if (vht_hdl.en_channel_width > WLAN_MIB_VHT_OP_WIDTH_80PLUS80) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{mac_ie_proc_vht_opern_ie::invalid channel width[%d], use 20M chn width.}",
                         vht_hdl.en_channel_width);
        vht_hdl.en_channel_width = WLAN_MIB_VHT_OP_WIDTH_20_40;
    }

    /* 解析 "VHT Basic MCS Set field" */
    basic_mcs_set_all_user = oal_make_word16(
        payload[MAC_IE_HDR_LEN + 3], payload[MAC_IE_HDR_LEN + 4]); /* 拼接+3,+4字节为基本mcs的设置 */
    vht_hdl.us_basic_mcs_set = basic_mcs_set_all_user;

    mac_user_set_vht_hdl(mac_user, &vht_hdl);

    if ((channel_width_old != vht_hdl.en_channel_width) ||
        (channel_center_freq_seg0_old != vht_hdl.uc_channel_center_freq_seg0)) {
        ret = MAC_VHT_CHANGE;
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "mac_ie_proc_vht_opern_ie:usr_bw is updated chanl_with from [%d] to [%d], \
                         chanl_center_freq_seg0 from [%d] to [%d]",
                         channel_width_old, vht_hdl.en_channel_width,
                         channel_center_freq_seg0_old, vht_hdl.uc_channel_center_freq_seg0);
    }

    return ret;
}

static uint32_t mac_ie_get_suite(uint8_t **ie, uint8_t *src_ie, uint32_t *suite,
                                 uint8_t wlan_suites, uint8_t type)
{
    uint8_t  suite_idx;
    uint8_t  ie_len = *(src_ie - 1);
    uint16_t suites_count;
    oal_bool_enum_uint8 ret;

    if (MAC_IE_REAMIN_LEN_IS_ENOUGH(src_ie, *ie, ie_len, 2) == OAL_FALSE) { /* 2表示Suite Count字段大小 */
        oam_warning_log1(0, OAM_SF_ANY,
            "mac_ie_get_wpa_cipher:no enough mem for suites_count, wpa ie len[%d]", ie_len);
        return OAL_ERR_CODE_MSG_LENGTH_ERR;
    }
    suites_count = oal_make_word16((*ie)[BYTE_OFFSET_0], (*ie)[BYTE_OFFSET_1]);
    *ie += BYTE_OFFSET_2; /* ie+=2表示ie移到Suite List字段 */

    ret = ((type == SUITE_FORM_RSN && suites_count == 0) || (suites_count > OAL_NL80211_MAX_NR_CIPHER_SUITES));
    if (ret != OAL_FALSE) {
        return OAL_ERR_CODE_MSG_LENGTH_ERR;
    }

    for (suite_idx = 0; suite_idx < suites_count; suite_idx++) {
        if (MAC_IE_REAMIN_LEN_IS_ENOUGH(src_ie, *ie, ie_len, 4) == OAL_FALSE) { /* 4表示套件大小 */
            oam_warning_log2(0, OAM_SF_ANY,
                "mac_ie_get_wpa_suite: invalid ie len[%d], suites_count[%d]",
                ie_len, suites_count);
            return OAL_ERR_CODE_MSG_LENGTH_ERR;
        }
        if (suite_idx < wlan_suites) {
            suite[suite_idx] = *(uint32_t *)(*ie);
        }
        *ie += 4; /* 4表示套件大小 */
    }

    return OAL_SUCC;
}


uint32_t mac_ie_get_wpa_cipher(uint8_t *ie, mac_crypto_settings_stru *crypto)
{
    uint8_t  ie_len;
    uint32_t ret;
    uint8_t *src_ie = ie + BYTE_OFFSET_2; /* ie+2表示WPA OUI字段 */

    if (oal_any_null_ptr2(ie, crypto)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(crypto, sizeof(mac_crypto_settings_stru), 0, sizeof(mac_crypto_settings_stru));
    /**************************************************************************/
    /*                  WPA Element Format                                    */
    /* ---------------------------------------------------------------------- */
    /* |Element ID | Length |    WPA OUI    |  Version |  Group Cipher Suite  */
    /* ---------------------------------------------------------------------- */
    /* |     1     |   1    |        4      |     2    |         4            */
    /* ---------------------------------------------------------------------- */
    /* ---------------------------------------------------------------------- */
    /* Pairwise Cipher |  Pairwise Cipher   |                 |               */
    /* Suite Count     |    Suite List      | AKM Suite Count |AKM Suite List */
    /* ---------------------------------------------------------------------- */
    /*        2        |          4*m       |         2       |     4*n       */
    /* ---------------------------------------------------------------------- */
    /*                                                                        */
    /**************************************************************************/
    ie_len = ie[BYTE_OFFSET_1];
    if (ie_len < MAC_MIN_WPA_LEN) {
        oam_warning_log1(0, OAM_SF_ANY, "mac_ie_get_wpa_cipher:invalid wpa ie len[%d]", ie_len);
        return  OAL_ERR_CODE_MSG_LENGTH_ERR;
    }

    ie += BYTE_OFFSET_8; /* ie+=8表示ie移到组加密套件(Group Cipher Suite)字段 */
    crypto->wpa_versions = WITP_WPA_VERSION_1;

    /* Group Cipher Suite */
    crypto->group_suite = *(uint32_t *)ie;
    ie += BYTE_OFFSET_4; /* ie+=4表示ie移到成对加密套件数(Pairwise Cipher Suite count)字段 */

    /* Pairwise Cipher */
    ret = mac_ie_get_suite(&ie, src_ie, crypto->aul_pair_suite, WLAN_PAIRWISE_CIPHER_SUITES, SUITE_FORM_WPA);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* AKM Suite */
    ret = mac_ie_get_suite(&ie, src_ie, crypto->aul_akm_suite, WLAN_AUTHENTICATION_SUITES, SUITE_FORM_WPA);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}

static uint32_t mac_ie_proc_pmk_info(uint8_t **ie, uint8_t *src_ie)
{
    uint16_t suite_idx, suites_count;
    uint8_t  ie_len = *(src_ie - 1);

    if (MAC_IE_REAMIN_LEN_IS_ENOUGH(src_ie, *ie, ie_len, 2) == OAL_FALSE) { /* 2表示PMKID Count字段大小 */
        return OAL_SUCC;
    }
    suites_count = oal_make_word16((*ie)[BYTE_OFFSET_0], (*ie)[BYTE_OFFSET_1]);
    *ie += BYTE_OFFSET_2; /* ie+=2表示ie移到PMKID List字段 */

    for (suite_idx = 0; suite_idx < suites_count; suite_idx++) {
        if (MAC_IE_REAMIN_LEN_IS_ENOUGH(src_ie, *ie, ie_len, 16) == OAL_FALSE) { /* 16表示PMKID的大小 */
            return OAL_FAIL;
        }
        *ie += BYTE_OFFSET_16; /* ie+=16表示ie向后移到下一个PMKID */
    }

    return OAL_SUCC;
}


uint32_t mac_ie_get_rsn_cipher(uint8_t *ie, mac_crypto_settings_stru *crypto)
{
    uint8_t  ie_len;
    uint32_t ret;
    uint8_t *src_ie = ie + BYTE_OFFSET_2; /* ie+2表示Version字段 */

    if (oal_any_null_ptr2(ie, crypto)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(crypto, sizeof(mac_crypto_settings_stru), 0, sizeof(mac_crypto_settings_stru));
    /**************************************************************************/
    /*                  RSN Element Format                                    */
    /* ---------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Suite   */
    /* ---------------------------------------------------------------------- */
    /* |     1     |    1   |    2    |         4          |       2          */
    /* ---------------------------------------------------------------------- */
    /* ---------------------------------------------------------------------- */
    /*  Count| Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List  */
    /* ---------------------------------------------------------------------- */
    /*       |         4*m                |     2           |   4*n           */
    /* ---------------------------------------------------------------------- */
    /* ---------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite */
    /* ---------------------------------------------------------------------- */
    /* |        2       |    2      |   16 *s  |               4           |  */
    /* ---------------------------------------------------------------------- */
    /*                                                                        */
    /**************************************************************************/
    ie_len = ie[BYTE_OFFSET_1];
    if (ie_len < MAC_MIN_RSN_LEN) {
        oam_warning_log1(0, OAM_SF_ANY, "mac_ie_get_rsn_cipher:invalid rsn ie len[%d]", ie_len);
        return OAL_FAIL;
    }

    /* version字段检查 */
    crypto->wpa_versions = *(uint16_t *)src_ie;
    if (crypto->wpa_versions != 1) {
        return OAL_FAIL;
    }
    crypto->wpa_versions = WITP_WPA_VERSION_2;  // wpa为1，rsn为2
    ie += BYTE_OFFSET_4; /* ie+=4表示ie移到组加密套件(Group Cipher Suite)字段 */

    /* Group Cipher Suite */
    crypto->group_suite = *(uint32_t *)ie;
    ie += BYTE_OFFSET_4; /* ie+=4表示ie移到成对加密套件数量(Pairwise Suite Count)字段 */

    /* Pairwise Cipher Suite */
    ret = mac_ie_get_suite(&ie, src_ie, crypto->aul_pair_suite, WLAN_PAIRWISE_CIPHER_SUITES, SUITE_FORM_RSN);
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* AKM Suite */
    ret = mac_ie_get_suite(&ie, src_ie, crypto->aul_akm_suite, WLAN_AUTHENTICATION_SUITES, SUITE_FORM_RSN);
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 越过RSN Capabilities */
    if (MAC_IE_REAMIN_LEN_IS_ENOUGH(src_ie, ie, ie_len, 2) == OAL_FALSE) { /* 2表示RSN Capabilities字段大小 */
        if (MAC_IE_REAMIN_LEN_IS_ENOUGH(src_ie, ie, ie_len, 1) == OAL_TRUE) { /* 1不够IE下一字段的大小 */
            return OAL_FAIL;
        }
        return OAL_SUCC;
    }
    ie += BYTE_OFFSET_2; /* ie+=2表示ie移到PMKID Count字段 */

    /* 目前PMK信息暂不做处理 */
    if (mac_ie_proc_pmk_info(&ie, src_ie) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 获取Group Management Cipher Suite信息 */
    if (MAC_IE_REAMIN_LEN_IS_ENOUGH(src_ie, ie, ie_len, 4) == OAL_FALSE) { /* 4表示Group Management Cipher Suite大小 */
        return OAL_SUCC;
    }
    if (((*(uint32_t *)ie) & 0xFFFFFF) == MAC_RSN_CHIPER_OUI(0)) {
        crypto->group_mgmt_suite = *(uint32_t *)ie;
        ie += BYTE_OFFSET_4; /* ie+=4表示ie移到IE的末尾 */
    }

    return OAL_SUCC;
}

OAL_STATIC wlan_channel_bandwidth_enum_uint8 mac_get_80mbandwith_from_offset_channel(int8_t offset_channel)
{
    switch (offset_channel) {
        case 6: /* 中心频率相对于主20偏6个信道 */
            return WLAN_BAND_WIDTH_80PLUSPLUS;
        case -2: /* 中心频率相对于主20偏-2个信道 */
            return WLAN_BAND_WIDTH_80PLUSMINUS;
        case 2: /* 中心频率相对于主20偏2个信道 */
            return WLAN_BAND_WIDTH_80MINUSPLUS;
        case -6: /* 中心频率相对于主20偏-6个信道 */
            return WLAN_BAND_WIDTH_80MINUSMINUS;
        default:
            return WLAN_BAND_WIDTH_20M;
    }
}

OAL_STATIC wlan_channel_bandwidth_enum_uint8 mac_get_160mbandwith_from_offset_channel(int8_t offset_channel)
{
    switch (offset_channel) {
        case 14: /* 中心频率相对于主20偏14个信道 */
            return WLAN_BAND_WIDTH_160PLUSPLUSPLUS;

        case 10: /* 中心频率相对于主20偏10个信道 */
            return WLAN_BAND_WIDTH_160MINUSPLUSPLUS;

        case 6: /* 中心频率相对于主20偏6个信道 */
            return WLAN_BAND_WIDTH_160PLUSMINUSPLUS;

        case 2: /* 中心频率相对于主20偏2个信道 */
            return WLAN_BAND_WIDTH_160MINUSMINUSPLUS;

        case -2: /* 中心频率相对于主20偏-2个信道 */
            return WLAN_BAND_WIDTH_160PLUSPLUSMINUS;

        case -6:  /* 中心频率相对于主20偏-6个信道 */
            return WLAN_BAND_WIDTH_160MINUSPLUSMINUS;

        case -10:  /* 中心频率相对于主20偏-10个信道 */
            return WLAN_BAND_WIDTH_160PLUSMINUSMINUS;

        case -14:  /* 中心频率相对于主20偏-14个信道 */
            return WLAN_BAND_WIDTH_160MINUSMINUSMINUS;
        default:
            return WLAN_BAND_WIDTH_20M;
    }
}

wlan_channel_bandwidth_enum_uint8 mac_get_bandwith_from_center_freq_seg0_seg1(
    uint8_t chan_width, uint8_t channel, uint8_t chan_center_freq0, uint8_t chan_center_freq1)
{
    /* 80+80不支持，暂按80M处理 */
    if (chan_width == WLAN_MIB_VHT_OP_WIDTH_80 || chan_width == WLAN_MIB_VHT_OP_WIDTH_80PLUS80) {
#ifdef _PRE_WLAN_FEATURE_160M
        if ((chan_center_freq1 - chan_center_freq0 == CHAN_OFFSET_PLUS_8) ||
            (chan_center_freq0 - chan_center_freq1 == CHAN_OFFSET_PLUS_8)) {
            return mac_get_160mbandwith_from_offset_channel((int8_t)(chan_center_freq1 - channel));
        }
#endif
        return mac_get_80mbandwith_from_offset_channel((int8_t)(chan_center_freq0 - channel));
    } else {
#ifdef _PRE_WLAN_FEATURE_160M
        if (chan_width == WLAN_MIB_VHT_OP_WIDTH_160) {
            if ((chan_center_freq1 - chan_center_freq0 == CHAN_OFFSET_PLUS_8) ||
                (chan_center_freq0 - chan_center_freq1 == CHAN_OFFSET_PLUS_8)) {
                return mac_get_160mbandwith_from_offset_channel((int8_t)(chan_center_freq1 - channel));
            } else {
                return mac_get_160mbandwith_from_offset_channel((int8_t)(chan_center_freq0 - channel));
            }
        }
#endif
        return WLAN_BAND_WIDTH_20M;
    }
}

