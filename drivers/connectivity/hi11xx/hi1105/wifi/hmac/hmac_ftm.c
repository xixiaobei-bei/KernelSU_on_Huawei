

/* 1 头文件包含 */
#include "oal_kernel_file.h"
#include "mac_data.h"
#include "hmac_resource.h"
#include "hmac_host_tx_data.h"
#include "hmac_tx_data.h"
#include "hmac_config.h"
#include "plat_pm_wlan.h"
#include "hmac_ftm.h"
#include "host_hal_ext_if.h"
#include "hmac_scan.h"

#include "securec.h"
#include "securectype.h"

#ifdef _PRE_WLAN_FEATURE_FTM
#define MAC_FTMP_LEN                          9
#define DIVIDER_2CLK    2

hmac_ftm_stru g_ftm = { { { 0 } }  };

void hmac_ftm_responder_start_ftms_timer(hmac_vap_stru *hmac_vap, ftm_responder_stru *ftm_responder_session);

void hmac_ftm_initiator_start_burst_timer(hmac_vap_stru *hmac_vap,
                                          ftm_initiator_stru *ftm_initiator_session);

void hmac_ftm_vap_init_1106(hmac_vap_stru *hmac_vap)
{
    hal_host_device_stru *hal_device = hal_get_host_device(hmac_vap->hal_dev_id);
    if (hal_device == NULL) {
        return;
    }
    hmac_vap->feature_ftm = &g_ftm;
    memset_s(hmac_vap->feature_ftm, sizeof(hmac_ftm_stru), 0, sizeof(hmac_ftm_stru));

    hal_host_ftm_set_buf_base_addr(hal_device, (uint64_t)(uintptr_t)&g_ftm);
    hal_host_ftm_set_buf_size(hal_device, DMAC_FTM_BUFF_SIZE);
    g_ftm.ftm_chain_selection = WLAN_TX_CHAIN_ZERO;
    g_ftm.divider = DIVIDER_2CLK;
}


uint8_t hmac_ftm_get_the_power_of_two(uint16_t num)
{
    uint8_t exponent = 0;
    while (num >> 1) {
        exponent++;
        num = num >> 1;
    }

    return exponent;
}


void hmac_ftm_set_mgmt_mac_hdr(uint8_t *mgmt_hdr, uint8_t *addr1, uint8_t *addr2, uint8_t *addr3)
{
    /* 设置 Frame Control field */
    mac_hdr_set_frame_control(mgmt_hdr, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mgmt_hdr, 0);

    /* 设置 address1(接收端): AP MAC地址 (BSSID) */
    oal_set_mac_addr(mgmt_hdr + WLAN_HDR_ADDR1_OFFSET, addr1);

     /* 设置 address2(发送端): dot11StationID */
    oal_set_mac_addr(mgmt_hdr + WLAN_HDR_ADDR2_OFFSET, addr2);

    /* 设置 address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr(mgmt_hdr + WLAN_HDR_ADDR3_OFFSET, addr3);
}


OAL_STATIC void hmac_ftm_responder_end_session(hal_host_device_stru *hal_device, ftm_responder_stru *ftm_rspder)
{
    frw_timer_destroy_timer(&(ftm_rspder->ftm_period_timer));
    frw_timer_destroy_timer(&(ftm_rspder->burst_period_timer));
    frw_timer_destroy_timer(&(ftm_rspder->iftm_timer));
    memset_s(ftm_rspder, sizeof(ftm_responder_stru), 0, sizeof(ftm_responder_stru));
    hal_host_ftm_set_enable(hal_device, OAL_FALSE);
}


void hmac_ftm_initiator_end_session(hal_host_device_stru *hal_device, ftm_initiator_stru *ftm_initiator_session)
{
    oam_warning_log3(0, OAM_SF_FTM, "hmac_ftm_initiator_end_session: end ftm session with ap [%x:%x:%x].",
                     ftm_initiator_session->bssid[MAC_ADDR_3],
                     ftm_initiator_session->bssid[MAC_ADDR_4],
                     ftm_initiator_session->bssid[MAC_ADDR_5]);
    frw_timer_destroy_timer(&(ftm_initiator_session->ftm_period_timer));
    frw_timer_destroy_timer(&(ftm_initiator_session->burst_period_timer));
    memset_s(ftm_initiator_session, sizeof(ftm_initiator_stru), 0, sizeof(ftm_initiator_stru));
    hal_host_ftm_set_enable(hal_device, OAL_FALSE);
}

OAL_STATIC uint16_t hmac_ftm_set_lci_field(uint8_t *payload_addr)
{
    uint16_t len = 0;
    payload_addr[len++] = MAC_EID_MEASREQ;          /* Element ID */
    payload_addr[len++] = 4;                        /* 4 is Length */
    payload_addr[len++] = 1;                        /* 1 is Measurement Token */
    payload_addr[len++] = 0;                        /* Measurement Req Mode */
    payload_addr[len++] = RM_RADIO_MEASUREMENT_LCI; /* Measurement Type */
    payload_addr[len++] = 1; /* Location Subjest, 0: local, 1: remote, 2: Third Party */
    return len;
}

OAL_STATIC uint16_t hmac_ftm_set_civic_field(uint8_t *payload_addr)
{
    uint16_t len = 0;
    payload_addr[len++] = MAC_EID_MEASREQ;                     /* Element ID */
    payload_addr[len++] = 8;                                   /* 8 is Length */
    payload_addr[len++] = 2;                                   /* 2 is Measurement Token */
    payload_addr[len++] = 0;                                   /* Measurement Req Mode */
    payload_addr[len++] = RM_RADIO_MEASUREMENT_LOCATION_CIVIC; /* Measurement Type */

    /*****************************************************************************************************/
    /* Location Civic Request */
    /* ------------------------------------------------------------------------------------------------- */
    /* |Location Subject |Civic Location Type |Location Service Interval Units|Location Service Interva|
        Optional Subelements| */
    /* ------------------------------------------------------------------------------------------------- */
    /* |1                |1                   | 1                             | 2                      |
        var                 | */
    /* ------------------------------------------------------------------------------------------------- */
    /*****************************************************************************************************/
    payload_addr[len++] = 1; /* Location Subjest, 0: local, 1: remote, 2: Third Party */
    payload_addr[len++] = 0;
    payload_addr[len++] = 0;
    payload_addr[len++] = 0; /* 0 means only a single Location Civic report is requested */
    payload_addr[len++] = 0;
    return len;
}


static uint8_t hmac_ftm_responder_set_dialog_token(uint8_t *payload_addr,
    ftm_responder_stru *ftm_resp_session)
{
    /* 立即测量和非立即测量模式dialog token值计算不同 */
    if (ftm_resp_session->is_asap_on == OAL_TRUE) {
        if ((ftm_resp_session->current_left_burst_cnt == 1) &&
            (ftm_resp_session->dialog_token >=
            (ftm_resp_session->ftms_per_burst * ftm_resp_session->burst_cnt))) {
            *payload_addr = 0;
            *(payload_addr + 1) = ftm_resp_session->dialog_token - 1;
            ftm_resp_session->current_left_burst_cnt--;
        } else {
            *payload_addr = ftm_resp_session->dialog_token;
            *(payload_addr + 1) = ftm_resp_session->dialog_token - 1;
            ftm_resp_session->dialog_token++;
            ftm_resp_session->follow_up_dialog_token++;
        }
    } else {
        if ((ftm_resp_session->current_left_burst_cnt == 1) &&
            (ftm_resp_session->dialog_token >=
            (ftm_resp_session->ftms_per_burst * ftm_resp_session->burst_cnt + 1))) {
            *payload_addr = 0;
            *(payload_addr + 1) = ftm_resp_session->dialog_token - 1;
            ftm_resp_session->current_left_burst_cnt--;
        } else {
            *payload_addr = ftm_resp_session->dialog_token;
            if (ftm_resp_session->dialog_token == 2) { /* 2: dialog_token */
                *(payload_addr + 1) = 0;
            } else {
                *(payload_addr + 1) = ftm_resp_session->dialog_token - 1;
            }
            ftm_resp_session->dialog_token++;
            ftm_resp_session->follow_up_dialog_token++;
        }
    }

    return FTM_FRAME_DIALOG_TOKEN_LEN;
}

static uint8_t hmac_ftm_responder_set_loc_civic_field(uint8_t *payload_addr,
    ftm_responder_stru *ftm_resp_session)
{
    uint8_t len = 0;
    /*****************************************************************************************************/
    /* LCI Measurement Request (Measurement Request IE) */
    /* -------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |Measurement Token| Measurement Req Mode|Measurement Type  | Measurement Req | */
    /* -------------------------------------------------------------------------------------------------- */
    /* |1          |1      | 1               | 1                   |1                  |var             | */
    /* -------------------------------------------------------------------------------------------------- */
    /******************************************************************************************************/
    /* 封装LCI   Report field */
    if (ftm_resp_session->lci_ie == OAL_TRUE) {
        payload_addr[len++] = MAC_EID_MEASREQ;                   /* Element ID */
        payload_addr[len++] = MAC_MEASUREMENT_REQUEST_IE_OFFSET; /* Length */
        payload_addr[len++] = 1;                                 /* Measurement Token */
        payload_addr[len++] = 0;                                 /* Measurement Req Mode */
        payload_addr[len++] = RM_RADIO_MEASUREMENT_LCI;          /* Measurement Type */
    }

    /* 封装LOCATION_CIVIC   field */
    if (ftm_resp_session->civic_ie == OAL_TRUE) {
        payload_addr[len++] = MAC_EID_MEASREQ;                     /* Element ID */
        payload_addr[len++] = MAC_MEASUREMENT_REQUEST_IE_OFFSET;   /* Length */
        payload_addr[len++] = 1;                                   /* Measurement Token */
        payload_addr[len++] = 0;                                   /* Measurement Req Mode */
        payload_addr[len++] = RM_RADIO_MEASUREMENT_LOCATION_CIVIC; /* Measurement Type */
    }

    return len;
}


static uint8_t hmac_ftm_responder_set_tsf_sync_info(uint8_t *payload_addr,
    ftm_responder_stru *ftm_resp_session)
{
    uint8_t len = 0;
    int32_t ret;

    /*************************************************************/
    /* FTM Synchronization Information element */
    /* --------------------------------------------------------- */
    /* |Element ID |Length |Element ID Extension | TSF Sync Info| */
    /* --------------------------------------------------------- */
    /* |1          |1      | 1                   | 4            | */
    /* --------------------------------------------------------- */
    /*************************************************************/
    if (ftm_resp_session->ftm_sync_info == OAL_TRUE) {
        payload_addr[len++] = MAC_EID_FTMSI; /* Element ID */
        payload_addr[len++] = FTM_TSF_SYNC_INFO_LEN; /* Length */
        payload_addr[len++] = MAC_EID_EXT_FTMSI;
        payload_addr += len;
        ret = memcpy_s(payload_addr, sizeof(uint32_t), &(ftm_resp_session->tsf), sizeof(uint32_t));
        if (ret != EOK) {
            oam_error_log1(0, 0, "memcpy fail!ret[%d]", ret);
        }

        len += sizeof(uint32_t);
    }

    return len;
}


OAL_STATIC uint8_t hmac_ftm_set_ht_format(wlan_bw_cap_enum_uint8 band_cap)
{
    uint8_t format_and_bandwidth;

    switch (band_cap) {
        case WLAN_BW_CAP_20M:
            format_and_bandwidth = 9;
            break;
        case WLAN_BW_CAP_40M:
            format_and_bandwidth = 11;
            break;
        default:
            format_and_bandwidth = 9;
            break;
    }

    return format_and_bandwidth;
}


OAL_STATIC OAL_INLINE uint8_t hmac_ftm_set_vht_format(wlan_bw_cap_enum_uint8 band_cap)
{
    uint8_t format_and_bandwidth;

    switch (band_cap) {
        case WLAN_BW_CAP_20M:
            format_and_bandwidth = 10;
            break;
        case WLAN_BW_CAP_40M:
            format_and_bandwidth = 12;
            break;
        case WLAN_BW_CAP_80M:
            format_and_bandwidth = 13;
            break;
        case WLAN_BW_CAP_80PLUS80:
            format_and_bandwidth = 14;
            break;
        case WLAN_BW_CAP_160M:
            format_and_bandwidth = 15;
            break;
        default:
            format_and_bandwidth = 9;
            break;
    }

    return format_and_bandwidth;
}


void hmac_ftm_get_format_and_bandwidth(wlan_bw_cap_enum_uint8 *band_cap,
                                       wlan_phy_protocol_enum_uint8 *protocol_mode,
                                       mac_ftm_parameters_ie_stru *mac_ftmp)
{
    switch (mac_ftmp->bit_format_and_bandwidth) {
        case 9:
            *band_cap = WLAN_BW_CAP_20M;
            *protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;
            break;
        case 10:
            *band_cap = WLAN_BW_CAP_20M;
            *protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;
            break;
        case 11:
            *band_cap = WLAN_BW_CAP_40M;
            *protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;
            break;
        case 12:
            *band_cap = WLAN_BW_CAP_40M;
            *protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;
            break;
        case 13:
            *band_cap = WLAN_BW_CAP_80M;
            *protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;
            break;
        case 14:
            *band_cap = WLAN_BW_CAP_80PLUS80;
            *protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;
            break;
        case 15:
        case 16:
            *band_cap = WLAN_BW_CAP_160M;
            *protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;
            break;
        default:
            *band_cap = WLAN_BW_CAP_20M;
            *protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;
            break;
    }
}

void hmac_ftm_set_format_and_bandwidth(wlan_bw_cap_enum_uint8 band_cap,
    wlan_phy_protocol_enum_uint8 protocol_mode, mac_ftm_parameters_ie_stru *hmac_mac_ftmp)
{
    hmac_mac_ftmp->bit_format_and_bandwidth = 9;

    if (protocol_mode == WLAN_HT_PHY_PROTOCOL_MODE) {
        hmac_mac_ftmp->bit_format_and_bandwidth = hmac_ftm_set_ht_format(band_cap);
    } else if (protocol_mode == WLAN_VHT_PHY_PROTOCOL_MODE) {
        hmac_mac_ftmp->bit_format_and_bandwidth = hmac_ftm_set_vht_format(band_cap);
    }
}


void hmac_ftm_initialize_parameter_ie(mac_ftm_parameters_ie_stru *mac_ftmp,
    ftm_responder_stru *ftm_resp_session)
{
    mac_ftmp->uc_eid = MAC_EID_FTMP;
    mac_ftmp->uc_len = MAC_FTMP_LEN;
    mac_ftmp->bit_number_of_bursts_exponent = 0;
    mac_ftmp->bit_burst_duration = FTM_BURST_DURATION;
    mac_ftmp->bit_partial_tsf_timer_no_preference = 0;
    mac_ftmp->bit_asap_capable = 1;
    mac_ftmp->bit_status_indication = OAL_TRUE; /* Reserved */

     /* ini STA no preference or res sta can't support ini sta's Number of Bursts Exponent */
    if (ftm_resp_session->burst_cnt == FTM_INI_STA_NO_PREF_BURST_NUM) {
        ftm_resp_session->burst_cnt = FTM_INI_STA_NO_PREF_RES_STA_BURST_NUM;
    } else {
        if ((ftm_resp_session->burst_cnt * ftm_resp_session->ftms_per_burst) > FTM_RES_STA_DIALOG_TOKEN_MAX) {
            mac_ftmp->bit_status_indication = FTM_STATUS_REQUEST_INCAPABLE;
        }
    }

    mac_ftmp->bit_burst_duration = hmac_ftm_get_the_power_of_two((uint16_t)ftm_resp_session->burst_duration) +
        FTM_MIN_DURATION;
    ftm_resp_session->status_indication = mac_ftmp->bit_status_indication;

    mac_ftmp->bit_value = 0;                    /* Reserved */
}


OAL_STATIC void hmac_ftm_responder_set_partial_tsf(ftm_responder_stru *ftm_res_session,
    mac_ftm_parameters_ie_stru *mac_ftm_paras)
{
    uint32_t pstf_offset;
    /* ASAP = 1 PTSF Timer set to a value less than 10 ms from the reception of the most recent initial FTM Req frame */
    if (ftm_res_session->is_asap_on == OAL_TRUE) {
        pstf_offset = FTM_RES_ASAP_SET_PTSF_OFFSET;
    } else {
        pstf_offset = FTM_RES_ASAP_SET_PTSF_OFFSET + ftm_res_session->res_ptsf_offset;
    }
    mac_ftm_paras->us_partial_tsf_timer =
        (uint16_t)(((ftm_res_session->tsf + pstf_offset) >> BIT_OFFSET_10) % BIT16);

    oam_warning_log4(0, OAM_SF_FTM, "hmac_ftm_responder_set_partial_tsf::tsf[0x%X], res_ptsf_offset[%u us] \
        (tsf + 3000us + res_ptsf_offset)[0x%X] PTSF[0x%X]", ftm_res_session->tsf,
        ftm_res_session->res_ptsf_offset, (ftm_res_session->tsf + pstf_offset),
        mac_ftm_paras->us_partial_tsf_timer);
}


static uint8_t hmac_ftm_responder_set_ftm_para(uint8_t *payload_addr,
    ftm_responder_stru *ftm_resp_session)
{
    mac_ftm_parameters_ie_stru *mac_ftm_param = NULL;
    uint16_t burst_cnt;

    /*******************************************************************/
    /* Fine Timing Measurement Parameters element */
    /* --------------------------------------------------------------- */
    /* |Element ID |Length |Fine Timing Measurement Parameters| */
    /* --------------------------------------------------------------- */
    /* |1          |1      | 9                                | */
    /* --------------------------------------------------------------- */
    /*******************************************************************/
    /*****************************************************************************************************************/
    /* Fine Timing Measurement Parameters */
    /* --------------------------------------------------------------------------------------------------------------*/
    /* |B0             B1 |B2 B6 |B7       | B8                   B11 |B12        B15 |B16       B23 |B24      B39 | */
    /* --------------------------------------------------------------------------------------------------------------*/
    /* |Status Indication|Value|Reserved|Number of Bursts Exponent|Burst Duration|Min Delta FTM |Partial TSF Timer | */
    /* --------------------------------------------------------------------------------------------------------------*/
    /* |B40                             |B41          |B42  |B43        B47 |B48  B49 |B50       B55 |B56      B71 | */
    /* --------------------------------------------------------------------------------------------------------------*/
    /* |Partial TSF Timer No Preference|ASAP Capable|ASAP|FTMs per Burst|Reserved|Format And Bandwidth|Burst Period| */
    /*****************************************************************************************************************/
    if (ftm_resp_session->ftm_parameters == OAL_TRUE) {
        /* 封装 Fine Timing Measurement Parameters element */
        mac_ftm_param = (mac_ftm_parameters_ie_stru *)payload_addr;
        memset_s(mac_ftm_param, sizeof(mac_ftm_parameters_ie_stru), 0, sizeof(mac_ftm_parameters_ie_stru));

        hmac_ftm_initialize_parameter_ie(mac_ftm_param, ftm_resp_session);
        burst_cnt = ftm_resp_session->burst_cnt;
        ftm_resp_session->current_left_burst_cnt = ftm_resp_session->burst_cnt;
        while (burst_cnt >> 1) {
            mac_ftm_param->bit_number_of_bursts_exponent++;
            burst_cnt = burst_cnt >> 1;
        }
        hmac_ftm_responder_set_partial_tsf(ftm_resp_session, mac_ftm_param);
        mac_ftm_param->bit_asap = ftm_resp_session->is_asap_on;

        /* 设置带宽协议 */
        hmac_ftm_set_format_and_bandwidth(ftm_resp_session->band_cap,
            ftm_resp_session->prot_format, mac_ftm_param);

        /* 每个burst发ftm帧的个数，ftm帧之间的间隔，以及burst的间隔 */
        mac_ftm_param->bit_ftms_per_burst = ftm_resp_session->ftms_per_burst;
        mac_ftm_param->uc_min_delta_ftm = ftm_resp_session->min_delta_ftm;
        mac_ftm_param->us_burst_period = ftm_resp_session->burst_period;

        return sizeof(mac_ftm_parameters_ie_stru);
    }

    return 0;
}


uint16_t hmac_encap_ftm_frame(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buffer,
                              ftm_responder_stru *ftm_responder_session)
{
    uint8_t *mgmt_hdr = oal_netbuf_header(buffer);
    uint8_t *payload_addr = NULL;
    uint16_t idx = 0;
    int32_t ret;
    uint8_t *addr3 = NULL;

    if (oal_any_null_ptr2(buffer, ftm_responder_session)) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{hmac_encap_ftm_frame::failed}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        addr3 = ftm_responder_session->mac_ra;
    } else {
        addr3 = hmac_vap->st_vap_base_info.auc_bssid;
    }

    hmac_ftm_set_mgmt_mac_hdr(mgmt_hdr, ftm_responder_session->mac_ra,
        mac_mib_get_StationID(&hmac_vap->st_vap_base_info), addr3);

    /***************************************************************************************************/
    /* Fine Timing Measurement frame format */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Category |Public Action |Dialog Token |Follow Up Dialog Token |TOD |TOA |TOD Error |TOA Error | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |1        |1             |1            |1                      |6   |6   |2         |2         | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |LCI Report (optional) | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Variable                                    | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Location Civic MeasuremenRequest (optional) | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Variable                                    | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Fine Timing MeasuremenParameters (optional) | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Variable                                    | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |FTM Synchronization Information (optional) | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Variable                                    | */
    /* ----------------------------------------------------------------------------------------------- */
    /***************************************************************************************************/
    payload_addr = mac_netbuf_get_payload(buffer);
    if (payload_addr == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "hmac_encap_ftm_frame::payload_addr NULL!");
        return 0;
    }

    payload_addr[idx++] = MAC_ACTION_CATEGORY_PUBLIC; /* Category */
    payload_addr[idx++] = MAC_PUB_FTM;                /* Public Action */

    idx += hmac_ftm_responder_set_dialog_token(payload_addr + idx, ftm_responder_session);
    ret = memcpy_s(payload_addr + idx, FTM_TODA_LENGTH, &ftm_responder_session->toda_info.tod,
        FTM_TODA_LENGTH);
    idx += FTM_TODA_LENGTH;
    ret += memcpy_s(payload_addr + idx, FTM_TODA_LENGTH, &ftm_responder_session->toda_info.toa,
        FTM_TODA_LENGTH);
    idx += FTM_TODA_LENGTH;
    ret += memcpy_s(payload_addr + idx, FTM_TODA_ERROR_LENGTH, &ftm_responder_session->toda_info.tod_error,
        FTM_TODA_ERROR_LENGTH);
    idx += FTM_TODA_ERROR_LENGTH;
    ret += memcpy_s(payload_addr + idx, FTM_TODA_ERROR_LENGTH, &ftm_responder_session->toda_info.toa_error,
        FTM_TODA_ERROR_LENGTH);
    idx += FTM_TODA_ERROR_LENGTH;

    if (ret != EOK) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, 0, "memcpy fail!ret[%d]", ret);
    }

    idx += hmac_ftm_responder_set_loc_civic_field(payload_addr + idx, ftm_responder_session);
    idx += hmac_ftm_responder_set_ftm_para(payload_addr + idx, ftm_responder_session);
    idx += hmac_ftm_responder_set_tsf_sync_info(payload_addr + idx, ftm_responder_session);

    return (uint16_t)(idx + MAC_80211_FRAME_LEN);
}

OAL_STATIC void hmac_ftm_initiator_set_mac_ftmp_field(mac_ftm_parameters_ie_stru *hmac_mac_ftmp,
    ftm_initiator_stru *ftm_initiator_session)
{
    hmac_mac_ftmp->uc_eid = MAC_EID_FTMP;
    hmac_mac_ftmp->uc_len = MAC_FTMP_LEN;

    hmac_mac_ftmp->bit_status_indication = 0;
    hmac_mac_ftmp->bit_value = 0;

    /* 回合个数(2^number_of_bursts_exponent) */
    hmac_mac_ftmp->bit_number_of_bursts_exponent = hmac_ftm_get_the_power_of_two(ftm_initiator_session->burst_cnt);

    hmac_mac_ftmp->bit_burst_duration = FTM_BURST_DURATION;
    hmac_mac_ftmp->us_partial_tsf_timer = 0;
    hmac_mac_ftmp->bit_partial_tsf_timer_no_preference = 1;
    hmac_mac_ftmp->bit_asap_capable = 0;  // FTM认证，协议9.4.2.168 iFTMR中该bit应为0
    hmac_mac_ftmp->bit_asap = ftm_initiator_session->is_asap_on;
    hmac_mac_ftmp->bit_ftms_per_burst = ftm_initiator_session->ftms_per_burst;
    hmac_mac_ftmp->uc_min_delta_ftm = ftm_initiator_session->min_delta_ftm;
    hmac_mac_ftmp->us_burst_period = ftm_initiator_session->burst_period;
}

OAL_STATIC uint16_t hmac_ftm_encap_request_frame(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *buffer, ftm_initiator_stru *ftm_initiator_session)
{
    uint8_t *mgmt_hdr = oal_netbuf_header(buffer);
    uint8_t *payload_addr = mac_netbuf_get_payload(buffer);
    uint16_t idx = 0;
    mac_ftm_parameters_ie_stru *hmac_mac_ftmp;
    uint8_t *addr3 = NULL;
    hal_host_device_stru *hal_device = hal_get_host_device(hmac_vap->hal_dev_id);

    if (oal_any_null_ptr4(hmac_vap, hal_device, buffer, ftm_initiator_session)) {
        oam_error_log0(0, OAM_SF_FTM,
            "{hmac_ftm_encap_request_frame::hmac_vap or buffer or tx_ba is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* FTM认证，非关联状态下，bssid为全F */
    if ((hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_UP) ||
        (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_PAUSE)) {
        addr3 = ftm_initiator_session->bssid;
    } else {
        addr3 = BROADCAST_MACADDR;
    }

    hmac_ftm_set_mgmt_mac_hdr(mgmt_hdr, ftm_initiator_session->bssid,
        mac_mib_get_StationID(&hmac_vap->st_vap_base_info), addr3);

    /* 配置FTM白名单寄存器，否则中断上报后PPU信息错误 */
    hal_host_ftm_set_white_list(hal_device, 0, ftm_initiator_session->bssid);

    /*************************************************************************************/
    /* FTM Request frame - Frame Body */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Public Action |Trigger | LCI Measurement Request(optional)| */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1             |1       |Variable                          | */
    /* --------------------------------------------------------------------------------- */
    /* |LCI Report (optional) | */
    /* --------------------------------------------------------------------------------- */
    /* |Variable                                    | */
    /* --------------------------------------------------------------------------------- */
    /* |Location Civic MeasuremenRequest (optional) | */
    /* --------------------------------------------------------------------------------- */
    /* |Variable                                    | */
    /* --------------------------------------------------------------------------------- */
    /* |Fine Timing MeasuremenParameters (optional) | */
    /* --------------------------------------------------------------------------------- */
    /* |Variable                                    | */
    /* --------------------------------------------------------------------------------- */
    /*************************************************************************************/
    if (payload_addr == NULL) {
        return 0;
    }

    payload_addr[idx++] = MAC_ACTION_CATEGORY_PUBLIC; /* Category */
    payload_addr[idx++] = MAC_PUB_FTM_REQ;            /* Public Action */
    payload_addr[idx++] = 1;                          /* Trigger */

    /* FTM认证，增加LCI，Location civic信元 */
    /*****************************************************************************************************/
    /* LCI Measurement Request (Measurement Request IE) */
    /* ------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |Measurement Token| Measurement Req Mode|Measurement Type  | Measurement Req | */
    /* ------------------------------------------------------------------------------------------------- */
    /* |1          |1      | 1               | 1                   |1                  |var             | */
    /* ------------------------------------------------------------------------------------------------- */
    /*****************************************************************************************************/
    if (ftm_initiator_session->is_initial_ftm_request == OAL_TRUE) {
        /* 封装Measurement Request IE */
        if (ftm_initiator_session->lci_ie == OAL_TRUE) {
            idx += hmac_ftm_set_lci_field(payload_addr + idx);
        }
        /* 封装Measurement Request IE */
        if (ftm_initiator_session->civic_ie == OAL_TRUE) {
            idx += hmac_ftm_set_civic_field(payload_addr + idx);
        }
    }

    /*******************************************************************/
    /* Fine Timing Measurement Parameters element */
    /* --------------------------------------------------------------- */
    /* |Element ID |Length |Fine Timing Measurement Parameters| */
    /* --------------------------------------------------------------- */
    /* |1          |1      | 9                                | */
    /* --------------------------------------------------------------- */
    /*******************************************************************/
    /*****************************************************************************************************************/
    /* Fine Timing Measurement Parameters */
    /* -------------------------------------------------------------------------------------------------------------*/
    /* |B0             B1 |B2 B6 |B7       | B8                   B11 |B12        B15 |B16       B23 |B24      B39 | */
    /* -------------------------------------------------------------------------------------------------------------*/
    /* |Status Indication |Value|Reserved|Number of Bursts Exponent|Burst Duration|Min Delta FTM|Partial TSF Timer| */
    /* --------------------------------------------------------------------------------------------------------------*/
    /* |B40                            |B41        |B42  |B43        B47 |B48  B49 |B50          B55 |B56      B71 | */
    /* --------------------------------------------------------------------------------------------------------------*/
    /* |Partial TSF Timer No Preference|ASAP Capable|ASAP|FTMs per Burst|Reserved|Format And Bandwidth|Burst Period| */
    /*****************************************************************************************************************/
    /* 封装 Fine Timing Measurement Parameters element */
    if (ftm_initiator_session->is_initial_ftm_request == OAL_TRUE) {
        hmac_mac_ftmp = (mac_ftm_parameters_ie_stru *)&(payload_addr[idx]);
        memset_s(hmac_mac_ftmp, sizeof(mac_ftm_parameters_ie_stru), 0, sizeof(mac_ftm_parameters_ie_stru));

        /* 填充参数 */
        hmac_ftm_initiator_set_mac_ftmp_field(hmac_mac_ftmp, ftm_initiator_session);

        /* 获取带宽 */
        hmac_ftm_set_format_and_bandwidth(ftm_initiator_session->band_cap,
                                          ftm_initiator_session->prot_format, hmac_mac_ftmp);

        idx = idx + sizeof(mac_ftm_parameters_ie_stru);
    }

    return (uint16_t)(idx + MAC_80211_FRAME_LEN);
}


OAL_STATIC oal_netbuf_stru *hmac_mgmt_ftm_frame_alloc(uint8_t vap_id)
{
    oal_netbuf_stru *ftm_frame = NULL;

    ftm_frame = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (ftm_frame == NULL) {
        oam_error_log0(vap_id, OAM_SF_FTM, "{hmac_mgmt_ftm_frame_alloc::ftm_frame null.}");
        return ftm_frame;
    }
    oal_mem_netbuf_trace(ftm_frame, OAL_TRUE);
    oal_netbuf_prev(ftm_frame) = NULL;
    oal_netbuf_next(ftm_frame) = NULL;
    return ftm_frame;
}

OAL_STATIC uint32_t hmac_ftm_tx_action_event(mac_vap_stru *mac_vap, oal_netbuf_stru *netbuf, uint16_t frame_len)
{
    frw_event_mem_stru *event_mem = NULL; /* 申请事件返回的内存指针 */
    frw_event_stru *event = NULL;
    dmac_tx_event_stru *tx_event = NULL;
    uint32_t ret;

    event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (event_mem == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_FTM, "{hmac_ftm_tx_action_event::event_mem alloc failed}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    event = frw_get_event_stru(event_mem);
    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT,
        sizeof(dmac_tx_event_stru), FRW_EVENT_PIPELINE_STAGE_1, mac_vap->uc_chip_id, mac_vap->uc_device_id,
        mac_vap->uc_vap_id);

    /* 填写事件payload */
    tx_event = (dmac_tx_event_stru *)(event->auc_event_data);
    tx_event->pst_netbuf = netbuf;
    tx_event->us_frame_len = frame_len + sizeof(mac_ftm_action_event_stru);
    ret = frw_event_dispatch_event(event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_FTM, "{hmac_ftm_tx_action_event::frw_event_dispatch failed[%d].}",
            ret);
    }
    frw_event_free_m(event_mem);
    return ret;
}

OAL_STATIC void hmac_mgmt_tx_set_responder_ftm_ctx(hmac_vap_stru *hmac_vap, uint16_t frame_len,
    mac_ftm_action_event_stru *ftm_ctx_action, ftm_responder_stru *ftm_responder_session)
{
    hmac_ftm_stru *hmac_ftm_info = NULL;

    hmac_ftm_info = (hmac_ftm_stru *)hmac_vap->feature_ftm;
    if (hmac_ftm_info == NULL) {
        return;
    }
    ftm_ctx_action->frame_len = frame_len;
    ftm_ctx_action->hdr_len = MAC_80211_FRAME_LEN;
    ftm_ctx_action->cali = hmac_ftm_info->cali;
    ftm_ctx_action->chain_selection = hmac_ftm_info->ftm_chain_selection;
    ftm_ctx_action->band_cap = ftm_responder_session->band_cap;
    ftm_ctx_action->prot_format = ftm_responder_session->prot_format;

    return;
}

OAL_STATIC void hmac_mgmt_tx_set_initiator_ftm_ctx(hmac_vap_stru *hmac_vap, uint16_t frame_len,
    mac_ftm_action_event_stru *ftm_ctx_action, ftm_initiator_stru *ftm_initiator_session)
{
    hmac_ftm_stru *hmac_ftm_info = NULL;

    hmac_ftm_info = (hmac_ftm_stru *)hmac_vap->feature_ftm;
    if (hmac_ftm_info == NULL) {
        return;
    }
    ftm_ctx_action->frame_len = frame_len;
    ftm_ctx_action->hdr_len = MAC_80211_FRAME_LEN;
    ftm_ctx_action->cali = hmac_ftm_info->cali;
    ftm_ctx_action->chain_selection = hmac_ftm_info->ftm_chain_selection;
    ftm_ctx_action->band_cap = ftm_initiator_session->band_cap;
    ftm_ctx_action->prot_format = ftm_initiator_session->prot_format;
    ftm_ctx_action->divider = hmac_ftm_info->divider;
    return;
}

OAL_STATIC void hmac_ftm_mgmt_tx_frame_cb_init(oal_netbuf_stru *addba_req, uint16_t frame_len,
    uint8_t frame_type, uint8_t frame_subtype)
{
    mac_tx_ctl_stru *tx_ctl = NULL;

    memset_s(oal_netbuf_cb(addba_req), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(addba_req);
    MAC_GET_CB_MPDU_LEN(tx_ctl) = frame_len + sizeof(mac_ftm_action_event_stru);
    MAC_GET_CB_FRAME_TYPE(tx_ctl) = frame_type;
    MAC_GET_CB_FRAME_SUBTYPE(tx_ctl) = frame_subtype;
}


OAL_STATIC uint32_t hmac_ftm_initiator_tx_ftm_request(hmac_vap_stru *hmac_vap,
    ftm_initiator_stru *ftm_initiator_session)
{
    mac_vap_stru *mac_vap = NULL;
    oal_netbuf_stru *ftm_frame = NULL;
    uint32_t ret;
    uint16_t frame_len;
    mac_ftm_action_event_stru ftm_ctx_action;

    mac_vap = &(hmac_vap->st_vap_base_info);
    ftm_frame = hmac_mgmt_ftm_frame_alloc(mac_vap->uc_vap_id);
    if (ftm_frame == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

     /* 调用封装管理帧接口 */
    frame_len = hmac_ftm_encap_request_frame(hmac_vap, ftm_frame, ftm_initiator_session);
    memset_s((uint8_t *)&ftm_ctx_action, sizeof(ftm_ctx_action), 0, sizeof(ftm_ctx_action));

     /* 赋值要传入Dmac的信息 */
    hmac_mgmt_tx_set_initiator_ftm_ctx(hmac_vap, frame_len, &ftm_ctx_action, ftm_initiator_session);
    if (memcpy_s((uint8_t *)(oal_netbuf_data(ftm_frame) + frame_len), sizeof(mac_ftm_action_event_stru),
        (uint8_t *)&ftm_ctx_action, sizeof(mac_ftm_action_event_stru)) != EOK) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
            "hmac_ftm_initiator_tx_ftm_request::memcpy fail!");
        oal_netbuf_free(ftm_frame);
        return OAL_FAIL;
    }
    oal_netbuf_put(ftm_frame, (frame_len + sizeof(mac_ftm_action_event_stru)));
    hmac_ftm_mgmt_tx_frame_cb_init(ftm_frame, frame_len, WLAN_CB_FRAME_TYPE_ACTION, WLAN_ACTION_FTM_REQUEST);

    ret = hmac_ftm_tx_action_event(mac_vap, ftm_frame, frame_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(ftm_frame);
        return ret;
    }

    return OAL_SUCC;
}


void hmac_save_ftm_range(hmac_vap_stru *hmac_vap, ftm_initiator_stru *ftm_initiator_session)
{
    hmac_ftm_stru *hmac_ftm_info;
    ftm_range_rpt_stru *ftm_range_rpt;
    uint8_t idx;
    uint8_t i;

    hmac_ftm_info = (hmac_ftm_stru *)hmac_vap->feature_ftm;
    if (hmac_ftm_info == NULL) {
        return;
    }
    ftm_range_rpt = &(hmac_ftm_info->ftm_range_rpt);
    /* 根据mac地址查找需要保存的位置 */
    for (i = 0; i < ftm_range_rpt->range_entry_count; i++) {
        ftm_range_entry_stru *entry = &ftm_range_rpt->ftm_range_entry[i];
        if (!oal_compare_mac_addr(entry->bssid, ftm_initiator_session->bssid)) {
            break;
        }
    }

    if (i == ftm_range_rpt->range_entry_count) {
        ftm_range_rpt->range_entry_count++;
    }

    idx = i;
    oal_set_mac_addr(ftm_range_rpt->ftm_range_entry[idx].bssid, ftm_initiator_session->bssid);
    ftm_range_rpt->ftm_range_entry[idx].bit_range = ftm_initiator_session->bit_range;
    ftm_range_rpt->ftm_range_entry[idx].bit_max_range_error_exponent =
        ftm_initiator_session->bit_max_error_expt;
    oam_warning_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                     "{hmac_save_ftm_range::AP[%d],MAC[*:%x:%x].}",
                     idx,
                     ftm_initiator_session->bssid[MAC_ADDR_4],
                     ftm_initiator_session->bssid[MAC_ADDR_5]);
    oam_warning_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                     "{hmac_save_ftm_range::entry count[%d], measurement_start_time[%d], \
                     bit_range[%d], bit_max_error_expt[%d].}",
                     ftm_range_rpt->range_entry_count,
                     ftm_initiator_session->bit_range,
                     ftm_initiator_session->bit_max_error_expt);
}

void hmac_location_ftm_process(hmac_vap_stru *hmac_vap, ftm_initiator_stru *ftm_initiator_session,
    uint8_t *payload)
{
    return;
}


uint32_t hmac_ftm_initiator_ftm_request_timeout(void *arg)
{
    hmac_vap_stru *hmac_vap;
    ftm_initiator_stru *ftm_initiator_session;
    ftm_timeout_arg_stru *timeout_arg;

    timeout_arg = (ftm_timeout_arg_stru *)arg;
    if (timeout_arg == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)(timeout_arg->hmac_vap);
    ftm_initiator_session = (ftm_initiator_stru *)(timeout_arg->ftm_session);

    if (ftm_initiator_session->burst_cnt == 0) {
        hmac_save_ftm_range(hmac_vap, ftm_initiator_session);
    }

    oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                     "{hmac_ftm_initiator_ftm_request_timeout::fail to rx ftm_1 in burst period.}");

    return OAL_SUCC;
}


uint32_t hmac_ftm_set_channel(hmac_vap_stru *hmac_vap, uint16_t scan_time,
                              ftm_initiator_stru *ftm_initiator_session)
{
    return OAL_SUCC;
}


void hmac_ftm_initiator_start_burst(hmac_vap_stru *hmac_vap, ftm_initiator_stru *ftm_initiator_session)
{
    mac_device_stru *hmac_device = NULL;
    mac_vap_stru *hmac_mac_vap = NULL;
    hal_host_device_stru *hal_device = hal_get_host_device(hmac_vap->hal_dev_id);

    hmac_mac_vap = &hmac_vap->st_vap_base_info;

    hmac_device = mac_res_get_dev(hmac_mac_vap->uc_device_id);
    if (oal_any_null_ptr2(hmac_device, hal_device)) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                       "{hmac_reorder_ba_timer_start::hmac_device[%d] null.}",
                       hmac_vap->st_vap_base_info.uc_vap_id);
        return;
    }

    if (ftm_initiator_session->burst_cnt == 0) {
        hmac_ftm_initiator_end_session(hal_device, ftm_initiator_session);
        return;
    }

    /* 发送ftm request帧前如果当前运行信道不是ftm的发送信道，则先切换信道 */
    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        oam_warning_log0(hmac_mac_vap->uc_vap_id, OAM_SF_FTM, "hmac_ftm_initiator_start_burst: switch channel.");
        hmac_ftm_set_channel(hmac_vap, FTM_DEFAULT_SCAN_TIME, ftm_initiator_session);
    }

    /* 每个burst周期开始需要发送一帧ftm_req */
    hmac_ftm_initiator_tx_ftm_request(hmac_vap, ftm_initiator_session);
    oam_warning_log0(hmac_mac_vap->uc_vap_id, OAM_SF_FTM,
                     "hmac_ftm_initiator_start_burst: send a ftm req frame, start ftm request timeout timer.");

    /* 软件定时器的单位是ms，这里是FTM_WAIT_TIMEOUT(10ms) */
    frw_timer_create_timer_m(&(ftm_initiator_session->ftm_period_timer),
                             hmac_ftm_initiator_ftm_request_timeout,
                             FTM_WAIT_TIMEOUT,
                             (void *)&(ftm_initiator_session->arg),
                             OAL_FALSE,
                             OAM_MODULE_ID_HMAC,
                             hmac_vap->st_vap_base_info.core_id);
}

uint32_t hmac_ftm_initiator_burst_timeout(void *arg)
{
    hmac_vap_stru *hmac_vap;
    ftm_initiator_stru *ftm_initiator_session;
    ftm_timeout_arg_stru *timeout_arg;

    timeout_arg = (ftm_timeout_arg_stru *)arg;
    if (timeout_arg == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)(timeout_arg->hmac_vap);
    ftm_initiator_session = (ftm_initiator_stru *)(timeout_arg->ftm_session);

    ftm_initiator_session->current_left_burst_cnt -= 1;

    hmac_ftm_initiator_start_burst(hmac_vap, ftm_initiator_session);
    hmac_ftm_initiator_start_burst_timer(hmac_vap, ftm_initiator_session);

    return OAL_SUCC;
}


void hmac_ftm_initiator_start_burst_timer(hmac_vap_stru *hmac_vap,
                                          ftm_initiator_stru *ftm_initiator_session)
{
    ftm_timeout_arg_stru *arg;
    uint32_t timeout;
    uint32_t tsf_diff_rx_ftm_and_tx_ftm_req;
    uint32_t tsf_diff_ptsf_and_tsf_sync_info;

    arg = &ftm_initiator_session->arg;
    if (ftm_initiator_session->is_asap_on == OAL_TRUE) {
        timeout = oal_max(ftm_initiator_session->burst_period, 1) * FTM_BURST_PERIOD_UINT;
    } else {
        if (ftm_initiator_session->partial_tsf_timer > 0) {
            tsf_diff_rx_ftm_and_tx_ftm_req =
                ftm_initiator_session->initiator_rx_ftm_tsf - ftm_initiator_session->initiator_send_ftm_req_tsf;
            tsf_diff_ptsf_and_tsf_sync_info = (ftm_initiator_session->partial_tsf_timer -
                ((ftm_initiator_session->tsf_sync_info >> BIT_OFFSET_10) % BIT16)) << BIT_OFFSET_10;

            timeout = (tsf_diff_ptsf_and_tsf_sync_info + FTM_RES_FTM_TRIGGER_TIME_OFFSET +
                ftm_initiator_session->tx_ftmr_timeout_fix - tsf_diff_rx_ftm_and_tx_ftm_req) / 1000; /* 1000:ms */
            ftm_initiator_session->partial_tsf_timer = 0;
        } else {
            /* FTM认证，修改为由set tsf命令配置 */
            timeout = ftm_initiator_session->burst_period * FTM_BURST_PERIOD_UINT; /* 100倍 */
        }
    }

    oam_warning_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
        "hmac_ftm_initiator_start_burst_timer::bursts timeout [0x%x] (user config), \
        current_left_burst_cnt[%d], burst_cnt[%d]!", timeout,
        ftm_initiator_session->current_left_burst_cnt, ftm_initiator_session->burst_cnt);

    if (ftm_initiator_session->current_left_burst_cnt > 0) {
        frw_timer_create_timer_m(&(ftm_initiator_session->burst_period_timer),
                                 hmac_ftm_initiator_burst_timeout,
                                 timeout,
                                 (void *)arg,
                                 OAL_FALSE,
                                 OAM_MODULE_ID_HMAC,
                                 hmac_vap->st_vap_base_info.core_id);
    }
}


OAL_STATIC ftm_responder_stru *hmac_ftm_responder_find_session(hmac_ftm_stru *hmac_ftm_info,
    uint8_t *mac_addr)
{
    uint8_t idx;

    for (idx = 0; idx < MAX_FTM_RESPONDER_SESSION; idx++) {
        if (hmac_ftm_info->ftm_responder[idx].dialog_token) {
            if (!oal_compare_mac_addr(hmac_ftm_info->ftm_responder[idx].mac_ra, mac_addr)) {
                return &hmac_ftm_info->ftm_responder[idx];
            }
        }
    }
    return NULL;
}

OAL_STATIC ftm_initiator_stru *hmac_ftm_initiator_find_session(hmac_ftm_stru *hmac_ftm_info, uint8_t *mac_addr)
{
    uint8_t idx;

    for (idx = 0; idx < MAX_FTM_INITIATOR_SESSION; idx++) {
        if (hmac_ftm_info->ftm_initiator[idx].session_on == OAL_TRUE) {
            if (!oal_compare_mac_addr(hmac_ftm_info->ftm_initiator[idx].bssid, mac_addr)) {
                return &hmac_ftm_info->ftm_initiator[idx];
            }
        }
    }
    return NULL;
}


void hmac_save_error_ftm_session(hmac_vap_stru *hmac_vap, mac_ftm_parameters_ie_stru *mac_ftmp,
    ftm_initiator_stru *ftm_initiator_session)
{
    hmac_ftm_stru *hamc_ftm_info;
    ftm_range_rpt_stru *ftm_range_rpt;
    uint8_t idx;

    hamc_ftm_info = (hmac_ftm_stru *)hmac_vap->feature_ftm;
    if (hamc_ftm_info == NULL) {
        return;
    }
    ftm_range_rpt = &(hamc_ftm_info->ftm_range_rpt);
    idx = ftm_range_rpt->error_entry_count;

    /* 根据mac地址查找需要保存的位置 */
    ftm_range_rpt->ftm_error_entry[idx].measurement_start_time =
        ftm_initiator_session->tsf_sync_info;
    oal_set_mac_addr(ftm_range_rpt->ftm_error_entry[idx].bssid, ftm_initiator_session->bssid);
    ftm_range_rpt->ftm_error_entry[idx].error_code = mac_ftmp->bit_status_indication;

    ftm_range_rpt->error_entry_count++;
}


uint32_t hmac_ftm_initiator_update_ftm_parameters(hmac_vap_stru *hmac_vap, uint8_t *payload,
    uint16_t us_frame_len, ftm_initiator_stru *ftm_initiator_session)
{
    uint8_t *puc_ie = NULL;
    mac_ftm_parameters_ie_stru *mac_ftmp = NULL;

    puc_ie = mac_find_ie_ext_ie(MAC_EID_FTMSI, MAC_EID_EXT_FTMSI, payload, us_frame_len);
    if (puc_ie != NULL && puc_ie[1] >= 1) {
        ftm_initiator_session->tsf_sync_info = *(uint32_t *)&puc_ie[FTM_FRAME_TSF_SYNC_INFO_OFFSET];
    }

    mac_ftmp = (mac_ftm_parameters_ie_stru *)mac_find_ie(MAC_EID_FTMP, payload, us_frame_len);
    if (mac_ftmp != NULL && (mac_ftmp->uc_len == (sizeof(mac_ftm_parameters_ie_stru) - MAC_IE_HDR_LEN))) {
        if (mac_ftmp->bit_status_indication != OAL_TRUE) {
            oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                             "{hmac_ftm_initiator_update_ftm_parameters::status indication is false!}");
            hmac_save_error_ftm_session(hmac_vap, mac_ftmp, ftm_initiator_session);
            return OAL_FAIL;
        }

        /* 更新ftm parameters */
        ftm_initiator_session->burst_cnt = BIT(mac_ftmp->bit_number_of_bursts_exponent);
        ftm_initiator_session->ftms_per_burst = oal_max(mac_ftmp->bit_ftms_per_burst, MIN_FTMS_PER_BURST);
        ftm_initiator_session->min_delta_ftm = oal_max(mac_ftmp->uc_min_delta_ftm, FTM_MIN_DELTA);
        ftm_initiator_session->burst_duration = (uint8_t)BIT(oal_max(oal_max(mac_ftmp->bit_burst_duration,
            FTM_MIN_DURATION), FTM_MAX_DURATION) - FTM_MIN_DURATION);
        ftm_initiator_session->burst_period = oal_max(mac_ftmp->us_burst_period,
            ftm_initiator_session->ftms_per_burst * ftm_initiator_session->min_delta_ftm / 1000 + 1);

        if (!mac_ftmp->bit_partial_tsf_timer_no_preference) {
            ftm_initiator_session->partial_tsf_timer = oal_max(mac_ftmp->us_partial_tsf_timer +
                (uint16_t)ftm_initiator_session->tx_ftmr_timeout_fix, 1);  // FTM认证，微调FTMR时间
        }

        if (ftm_initiator_session->is_asap_on == OAL_FALSE) {
            ftm_initiator_session->burst_cnt += 1;
        }
        ftm_initiator_session->current_left_burst_cnt = ftm_initiator_session->burst_cnt;

        hmac_ftm_get_format_and_bandwidth(&ftm_initiator_session->band_cap,
                                          &ftm_initiator_session->prot_format, mac_ftmp);

        oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
            "rx FTM_1, update ftm param,tsf_sync_info [0x%x], after fix partial_tsf_timer[0x%x], burst_cnt[%d].",
            ftm_initiator_session->tsf_sync_info,
            ftm_initiator_session->partial_tsf_timer);

        oam_warning_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
            "ftms_per_burst[%d], burst_period[%d]*100ms.",
            ftm_initiator_session->burst_cnt,
            ftm_initiator_session->ftms_per_burst,
            ftm_initiator_session->burst_period);
    }

    return OAL_SUCC;
}


OAL_STATIC ftm_responder_stru *hmac_ftm_responder_alloc_session(hmac_vap_stru *hmac_vap,
                                                                uint8_t *mac_addr)
{
    hmac_ftm_stru *hmac_ftm_info;
    ftm_responder_stru *ftm_responder_session;
    uint8_t idx;

    hmac_ftm_info = (hmac_ftm_stru *)hmac_vap->feature_ftm;
    if (hmac_ftm_info == NULL) {
        return NULL;
    }
    ftm_responder_session = hmac_ftm_responder_find_session(hmac_ftm_info, mac_addr);
    if (ftm_responder_session != NULL) {
        return ftm_responder_session;
    }

    for (idx = 0; idx < MAX_FTM_RESPONDER_SESSION; idx++) {
        ftm_responder_session = &hmac_ftm_info->ftm_responder[idx];
        if (ftm_responder_session->dialog_token == 0) {
            oal_set_mac_addr(ftm_responder_session->mac_ra, mac_addr);
            ftm_responder_session->dialog_token = 1;
            ftm_responder_session->received_iftmr = OAL_FALSE;
            ftm_responder_session->arg.hmac_vap = (void *)hmac_vap;
            ftm_responder_session->arg.ftm_session = (void *)ftm_responder_session;
            return ftm_responder_session;
        }
    }

    return NULL;
}


OAL_STATIC ftm_initiator_stru *hmac_ftm_initiator_alloc_session(hmac_vap_stru *hmac_vap,
                                                                uint8_t *peer_mac)
{
    hmac_ftm_stru *ftm_info;
    ftm_initiator_stru *ftm_initiator_session;
    uint8_t index;

    ftm_info = (hmac_ftm_stru *)hmac_vap->feature_ftm;
    if (ftm_info == NULL) {
        return NULL;
    }

    ftm_initiator_session = hmac_ftm_initiator_find_session(ftm_info, peer_mac);
    if (ftm_initiator_session != NULL) {
        return ftm_initiator_session;
    }

    for (index = 0; index < MAX_FTM_INITIATOR_SESSION; index++) {
        ftm_initiator_session = &ftm_info->ftm_initiator[index];
        if (ftm_initiator_session->session_on != OAL_TRUE) {
            memset_s(ftm_initiator_session, sizeof(ftm_initiator_stru), 0, sizeof(ftm_initiator_stru));
            oal_set_mac_addr(ftm_initiator_session->bssid, peer_mac);
            ftm_initiator_session->session_on = OAL_TRUE;
            ftm_initiator_session->is_initial_ftm_request = OAL_TRUE;
            ftm_initiator_session->arg.hmac_vap = (void *)hmac_vap;
            ftm_initiator_session->arg.ftm_session = (void *)ftm_initiator_session;
            return ftm_initiator_session;
        }
    }
    return NULL;
}


uint32_t hmac_ftm_responder_send_ftm_frame(hmac_vap_stru *hmac_vap, ftm_responder_stru *ftm_responder_session)
{
    mac_vap_stru *mac_vap = NULL;
    oal_netbuf_stru *ftm_frame = NULL;
    uint32_t ret;
    uint16_t frame_len;
    mac_ftm_action_event_stru ftm_ctx_action;

    mac_vap = &(hmac_vap->st_vap_base_info);
    ftm_frame = hmac_mgmt_ftm_frame_alloc(mac_vap->uc_vap_id);
    if (ftm_frame == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

     /* 调用封装管理帧接口 */
    frame_len = hmac_encap_ftm_frame(hmac_vap, ftm_frame, ftm_responder_session);
    memset_s((uint8_t *)&ftm_ctx_action, sizeof(ftm_ctx_action), 0, sizeof(ftm_ctx_action));

     /* 赋值要传入Dmac的信息 */
    hmac_mgmt_tx_set_responder_ftm_ctx(hmac_vap, frame_len, &ftm_ctx_action, ftm_responder_session);
    if (memcpy_s((uint8_t *)(oal_netbuf_data(ftm_frame) + frame_len), sizeof(mac_ftm_action_event_stru),
        (uint8_t *)&ftm_ctx_action, sizeof(mac_ftm_action_event_stru)) != EOK) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
            "hmac_ftm_responder_send_ftm_frame::memcpy fail!");
        oal_netbuf_free(ftm_frame);
        return OAL_FAIL;
    }
    oal_netbuf_put(ftm_frame, (frame_len + sizeof(mac_ftm_action_event_stru)));
    hmac_ftm_mgmt_tx_frame_cb_init(ftm_frame, frame_len, WLAN_CB_FRAME_TYPE_ACTION, WLAN_ACTION_FTM_RESPONE);

    ret = hmac_ftm_tx_action_event(mac_vap, ftm_frame, frame_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(ftm_frame);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_ftm_responder_ftms_timeout(void *arg)
{
    ftm_timeout_arg_stru *timeout_arg;
    hmac_vap_stru *hmac_vap;
    ftm_responder_stru *ftm_responder_session;

    timeout_arg = (ftm_timeout_arg_stru *)arg;
    if (timeout_arg == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)timeout_arg->hmac_vap;
    ftm_responder_session = (ftm_responder_stru *)timeout_arg->ftm_session;

    hmac_ftm_responder_send_ftm_frame(hmac_vap, ftm_responder_session);

    ftm_responder_session->ftm_parameters = OAL_FALSE;
    ftm_responder_session->ftm_sync_info = OAL_FALSE;

    hmac_ftm_responder_start_ftms_timer(hmac_vap, ftm_responder_session);

    return OAL_SUCC;
}


void hmac_ftm_responder_start_ftms_timer(hmac_vap_stru *hmac_vap, ftm_responder_stru *ftm_responder_session)
{
    uint32_t timeout;
    hal_host_device_stru *hal_device = hal_get_host_device(hmac_vap->hal_dev_id);

    mac_device_stru *hmac_device = NULL;
    mac_vap_stru *hmac_mac_vap = NULL;

    hmac_mac_vap = &hmac_vap->st_vap_base_info;

    hmac_device = mac_res_get_dev(hmac_mac_vap->uc_device_id);
    if ((hmac_device == NULL) || (ftm_responder_session->ftms_per_burst == 0)) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
            "{hmac_ftm_responder_start_ftms_timer::hmac_device[%d] null.}", hmac_vap->st_vap_base_info.uc_vap_id);
        return;
    }

    if (ftm_responder_session->status_indication != FTM_STATUS_SUCCESS) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "hmac_ftm_responder_start_ftms_timer:: \
            status not succ, end session");
        hmac_ftm_responder_end_session(hal_device, ftm_responder_session);
        return;
    }

    /* 立即模式 */
    if (ftm_responder_session->is_asap_on == OAL_FALSE) {
        if (((ftm_responder_session->follow_up_dialog_token - 1) % (ftm_responder_session->ftms_per_burst)) == 0) {
            ftm_responder_session->current_left_burst_cnt--;
            return;
        }
        if ((ftm_responder_session->dialog_token >=
            (ftm_responder_session->ftms_per_burst * ftm_responder_session->burst_cnt)) &&
            (ftm_responder_session->current_left_burst_cnt == 0)) {
            oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "hmac_ftm_responder_start_ftms_timer:: \
                responder end session asap 1!");
            hmac_ftm_responder_end_session(hal_device, ftm_responder_session);
            return;
        }
    } else { /* 非立即模式 */
        if (((ftm_responder_session->follow_up_dialog_token) % (ftm_responder_session->ftms_per_burst)) == 0) {
            ftm_responder_session->current_left_burst_cnt--;
            return;
        }
        if ((ftm_responder_session->dialog_token >=
            ((ftm_responder_session->ftms_per_burst * ftm_responder_session->burst_cnt) + 1)) &&
            (ftm_responder_session->current_left_burst_cnt == 0)) {
            oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "hmac_ftm_responder_start_ftms_timer:: \
                responder end session apap 0!");
            hmac_ftm_responder_end_session(hal_device, ftm_responder_session);
            return;
        }
    }

    timeout = ftm_responder_session->min_delta_ftm / 10; /* 10 单位转换 */

    frw_timer_create_timer_m(&(ftm_responder_session->ftm_period_timer), hmac_ftm_responder_ftms_timeout, timeout,
        (void *)&ftm_responder_session->arg, OAL_FALSE, OAM_MODULE_ID_HMAC, hmac_vap->st_vap_base_info.core_id);
}


void hmac_ftm_responder_start_send_iftm_timer(hmac_vap_stru *hmac_vap, ftm_responder_stru *ftm_responder_session)
{
    uint32_t timeout = FTM_RES_ASAP_TX_FTM_TIMEOUT;
    mac_device_stru *hmac_device = NULL;
    mac_vap_stru *hmac_mac_vap = NULL;

    hmac_mac_vap = &hmac_vap->st_vap_base_info;

    hmac_device = mac_res_get_dev(hmac_mac_vap->uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                       "{hmac_reorder_ba_timer_start::hmac_device[%d] null.}",
                       hmac_vap->st_vap_base_info.uc_vap_id);
        return;
    }

    frw_timer_create_timer_m(&(ftm_responder_session->ftm_period_timer),
                             hmac_ftm_responder_ftms_timeout,
                             timeout,
                             (void *)&ftm_responder_session->arg,
                             OAL_FALSE,
                             OAM_MODULE_ID_HMAC,
                             hmac_vap->st_vap_base_info.core_id);
}

OAL_STATIC void hmac_ftm_responder_set_session_field(hmac_vap_stru *hmac_vap,
    ftm_responder_stru  *ftm_responder_session, mac_ftm_parameters_ie_stru *mac_ftmp)
{
    ftm_responder_session->ftm_parameters = OAL_TRUE;
    ftm_responder_session->burst_cnt = BIT(mac_ftmp->bit_number_of_bursts_exponent);
    ftm_responder_session->current_left_burst_cnt = ftm_responder_session->burst_cnt;
    ftm_responder_session->ftms_per_burst = oal_max(mac_ftmp->bit_ftms_per_burst,
                                                    MIN_FTMS_PER_BURST);
    ftm_responder_session->min_delta_ftm = oal_max(mac_ftmp->uc_min_delta_ftm, FTM_MIN_DELTA);
    ftm_responder_session->burst_period = oal_max(mac_ftmp->us_burst_period,
        ftm_responder_session->ftms_per_burst * ftm_responder_session->min_delta_ftm / 1000 + 1);
    ftm_responder_session->is_asap_on = mac_ftmp->bit_asap;
    ftm_responder_session->burst_duration = (uint8_t)BIT(oal_max(oal_max(
        mac_ftmp->bit_burst_duration, FTM_MIN_DURATION), FTM_MAX_DURATION) - FTM_MIN_DURATION);
    ftm_responder_session->received_iftmr = OAL_TRUE;

    ftm_responder_session->lci_ie = OAL_FALSE;
    ftm_responder_session->civic_ie = OAL_FALSE;

    hmac_ftm_get_format_and_bandwidth(&ftm_responder_session->band_cap,
                                      &ftm_responder_session->prot_format, mac_ftmp);

    if (ftm_responder_session->is_asap_on == OAL_TRUE) {
        ftm_responder_session->ftm_sync_info = OAL_TRUE;
        hmac_ftm_responder_start_send_iftm_timer(hmac_vap, ftm_responder_session);
    } else {
        ftm_responder_session->ftm_sync_info = OAL_TRUE;
        hmac_ftm_responder_send_ftm_frame(hmac_vap, ftm_responder_session);
    }
}

void hmac_ftm_responder_rx_req(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    mac_rx_ctl_stru  *rx_info = (mac_rx_ctl_stru *)oal_netbuf_cb((netbuf));
    hmac_ftm_stru *hmac_ftm_info = NULL;
    uint8_t *payload = NULL;
    ftm_responder_stru  *ftm_responder_session = NULL;
    mac_ftm_parameters_ie_stru *mac_ftmp = NULL;
    mac_ieee80211_frame_stru *frame_hdr = NULL; /* 保存mac帧的指针 */
    int32_t frame_len; /* 消息总长度,不包括FCS */
    hal_host_device_stru *hal_device = hal_get_host_device(hmac_vap->hal_dev_id);

    hmac_ftm_info = (hmac_ftm_stru *)hmac_vap->feature_ftm;

    if (oal_any_null_ptr2(hmac_ftm_info, hal_device)) {
        return;
    }

    /* 获取帧头信息 */
    frame_hdr = (mac_ieee80211_frame_stru *)MAC_GET_RX_CB_MAC_HEADER_ADDR(rx_info);

    frame_len = MAC_GET_RX_CB_PAYLOAD_LEN(rx_info); /* 帧体长度 */
    /* 获取帧体指针 */
    payload = mac_netbuf_get_payload(netbuf);
    /* ftm initiator */
    if (OAL_FALSE == mac_mib_get_FtmResponderModeActivated(&hmac_vap->st_vap_base_info)) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                         "{hmac_ftm_responder_rx_req::not a ftm_initiator, ignored rx ftm_req.}");
        return;
    }

    /* 查找后初始化一个session responder收到ftm REQ 从帧体取出MAC地址 */
    ftm_responder_session = hmac_ftm_responder_alloc_session(hmac_vap, frame_hdr->auc_address2);
    if (ftm_responder_session == NULL) {
        return;
    }

     /* 收到结束ftm请求 */
    if (payload[FTM_REQ_TRIGGER_OFFSET] != OAL_TRUE) {
        hmac_ftm_responder_end_session(hal_device, ftm_responder_session);
        return;
    }

    /* 收到对端的FTM REQ，设置对端MAC地址到FTM白名单寄存器 */
    hal_host_ftm_set_white_list(hal_device, 0, frame_hdr->auc_address2);

    ftm_responder_session->res_ptsf_offset = hmac_ftm_info->res_ptsf_delta;
    /* iftmr，更新ftm parameters */
    payload += FTM_FRAME_IE_OFFSET;
    frame_len -= FTM_FRAME_IE_OFFSET;
    mac_ftmp = (mac_ftm_parameters_ie_stru *)mac_find_ie(MAC_EID_FTMP, payload, (int32_t)frame_len);
    /* 收到的FTM req带FTM req paras字段 说明是initial FTM req帧 */
    if (mac_ftmp != NULL && (mac_ftmp->uc_len == (sizeof(mac_ftm_parameters_ie_stru) - MAC_IE_HDR_LEN))) {
        hmac_ftm_responder_set_session_field(hmac_vap, ftm_responder_session, mac_ftmp);
        return;
    }

    /* 若收到非ini FTM req */
    if (ftm_responder_session->received_iftmr == OAL_FALSE) {
        hmac_ftm_responder_end_session(hal_device, ftm_responder_session);
        return;
    }

    ftm_responder_session->ftm_sync_info = OAL_TRUE;
    ftm_responder_session->ftm_parameters = OAL_FALSE;
    hmac_ftm_responder_send_ftm_frame(hmac_vap, ftm_responder_session);
    hmac_ftm_responder_start_ftms_timer(hmac_vap, ftm_responder_session);
    ftm_responder_session->ftm_sync_info = OAL_FALSE;
}


OAL_STATIC OAL_INLINE void hmac_ftm_initiator_save_rssi(ftm_initiator_stru *ftm_initiator_session,
    dmac_rx_ctl_stru *rx_ctl)
{
    ftm_initiator_session->toda_info.mcs = rx_ctl->st_rx_status.un_nss_rate.st_ht_rate.bit_ht_mcs;
    ftm_initiator_session->toda_info.c_rssi_dbm = rx_ctl->st_rx_statistic.c_rssi_dbm;
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV))
    ftm_initiator_session->toda_info.c_snr_ant0 = rx_ctl->st_rx_statistic.snr[0];
    ftm_initiator_session->toda_info.c_snr_ant1 = rx_ctl->st_rx_statistic.snr[1];
#else
    ftm_initiator_session->toda_info.c_snr_ant0 = rx_ctl->st_rx_statistic.snr[0];
    ftm_initiator_session->toda_info.c_snr_ant1 = rx_ctl->st_rx_statistic.snr[1];
    ftm_initiator_session->toda_info.c_snr_ant2 = rx_ctl->st_rx_statistic.snr[2];
    ftm_initiator_session->toda_info.c_snr_ant3 = rx_ctl->st_rx_statistic.snr[3];
#endif
}

uint64_t hmac_ftm_initiator_get_rtt(hmac_vap_stru *hmac_vap, ftm_initiator_stru *ftm_initiator_session,
    uint8_t *payload)
{
    uint64_t temp_t1;
    uint64_t temp_t2;
    uint64_t temp_t3;
    uint64_t temp_t4;
    uint32_t rtt;
    uint16_t t2_error;
    uint16_t t4_error;
    uint16_t error;
    uint32_t distance;

    if (payload[FTM_FRAME_FOLLOWUP_DIALOG_TOKEN_OFFSET] !=
        ftm_initiator_session->toda_info.dialog_token) {
        oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
            "{hmac_ftm_initiator_get_rtt::ftm's dialog token[%d] != initiator's dialog token[%d],did not calc rtt",
            payload[FTM_FRAME_FOLLOWUP_DIALOG_TOKEN_OFFSET],
            ftm_initiator_session->toda_info.dialog_token);
        return 0;
    }

    memcpy_s(&temp_t1, FTM_TODA_LENGTH, payload + FTM_FRAME_TOD_OFFSET, FTM_TODA_LENGTH);
    temp_t1 &= FTM_TIME_MASK;
    memcpy_s(&temp_t4, FTM_TODA_LENGTH, payload + FTM_FRAME_TOA_OFFSET, FTM_TODA_LENGTH);
    temp_t4 &= FTM_TIME_MASK;
    memcpy_s(&t4_error, FTM_TODA_ERROR_LENGTH, payload + FTM_FRAME_TOA_ERROR_OFFSET, FTM_TODA_ERROR_LENGTH);

    temp_t2 = ftm_initiator_session->toda_info.toa;
    temp_t3 = ftm_initiator_session->toda_info.tod;
    t2_error = ftm_initiator_session->toda_info.toa_error;

    if ((temp_t1 == temp_t4) || (temp_t2 == temp_t3)) {
        return 0; /* Need to be omitted for LOC certification */
    }

    /* 翻转处理 */
    if (temp_t4 < temp_t1) {
        temp_t4 += ~temp_t1;
        temp_t1 = 0;
    }

    /* 翻转处理 */
    if (temp_t3 < temp_t2) {
        temp_t3 += ~temp_t2;
        temp_t2 = 0;
    }

    /* rtt的单位是纳秒,异常值暂不处理 */
    rtt = (uint32_t)(((temp_t4 - temp_t1) - (temp_t3 - temp_t2)) >> 10);
    distance = (FTM_DISTANCE_SPEED_OF_LIGHT_PARA * rtt) >> 1;
    error = (t2_error + t4_error) >> 10;
    oam_warning_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "hmac_ftm_initiator_get_rtt::rtt[%d], \
        distance[%d], error[%d]", rtt, distance, error); /* 打印rtt相关参数5个 */
    oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "hmac_ftm_initiator_get_rtt::dialog_token[%d], \
        rssi[%d]", ftm_initiator_session->toda_info.dialog_token, ftm_initiator_session->toda_info.c_rssi_dbm);

    /* FTM认证 更新测量结果 */
    hmac_save_ftm_range(hmac_vap, ftm_initiator_session);

    hmac_location_ftm_process(hmac_vap, ftm_initiator_session, payload);

    return rtt;
}

OAL_STATIC void hmac_ftm_initiator_lci_civic_ie(ftm_initiator_stru *ftm_initiator_session, uint8_t *payload,
    int32_t frame_len, uint16_t *len_to_skip)
{
    uint8_t *meas_rep = NULL;
    if (ftm_initiator_session->lci_ie) {
        meas_rep = mac_find_ie(MAC_EID_MEASREP, payload, frame_len);
        if (meas_rep != NULL) {
            *len_to_skip += (*(meas_rep + 1) + 2);
        }
    }

    if (ftm_initiator_session->civic_ie) {
        meas_rep = mac_find_ie(MAC_EID_MEASREP, payload + *len_to_skip, frame_len - *len_to_skip);
        if (meas_rep != NULL) {
            *len_to_skip += (*(meas_rep + 1) + 2);
        }
    }
}

uint32_t hmac_ftm_initiator_update_first_ftm_param(hmac_vap_stru *hmac_vap, uint8_t *payload,
    int32_t frame_len, ftm_initiator_stru *ftm_initiator_session)
{
    uint16_t len_to_skip = FTM_FRAME_OPTIONAL_IE_OFFSET;
    mac_ftm_parameters_ie_stru *mac_ftmp = NULL;
    hal_host_device_stru *hal_device = hal_get_host_device(hmac_vap->hal_dev_id);

    mac_ftmp = (mac_ftm_parameters_ie_stru *)mac_find_ie(MAC_EID_FTMP, payload + len_to_skip,
                                                         (frame_len - len_to_skip));
    if (oal_any_null_ptr2(mac_ftmp, hal_device)) {
        return OAL_FAIL;
    }

    if (mac_ftmp->bit_status_indication == FTM_STATUS_REQUEST_INCAPABLE &&
        (mac_ftmp->uc_len == (sizeof(mac_ftm_parameters_ie_stru) - MAC_IE_HDR_LEN))) {
        hmac_ftm_initiator_end_session(hal_device, ftm_initiator_session);
        oam_warning_log0(0, OAM_SF_FTM, "hmac_ftm_initiator_rx_ftm::req incapable, end session");
        return OAL_SUCC;
    }

    /* 收到的是FTM_1 */
    memset_s(&(ftm_initiator_session->toda_info), sizeof(ftm_initiator_session->toda_info), 0,
             sizeof(ftm_initiator_session->toda_info));

    /* 初始化保存ftm测量结构体 */
    if (OAL_SUCC == hmac_ftm_initiator_update_ftm_parameters(hmac_vap, payload + len_to_skip,
        frame_len - len_to_skip, ftm_initiator_session)) {
        ftm_initiator_session->bit_range = 0;
        ftm_initiator_session->is_initial_ftm_request = OAL_FALSE;
    }

    hmac_ftm_initiator_start_burst_timer(hmac_vap, ftm_initiator_session);
    return OAL_SUCC;
}

uint32_t hmac_ftm_initiator_rx_rsp(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    dmac_rx_ctl_stru *rx_ctrl = NULL;
    uint8_t *payload = NULL;
    int32_t frame_len; /* 消息总长度,不包括FCS */
    uint16_t len_to_skip = FTM_FRAME_OPTIONAL_IE_OFFSET;
    ftm_initiator_stru *ftm_initiator_session = NULL;
    mac_ftm_parameters_ie_stru *mac_ftmp = NULL;
    mac_ieee80211_frame_stru *frame_hdr = NULL; /* 保存mac帧的指针 */
    hal_host_device_stru *hal_device = hal_get_host_device(hmac_vap->hal_dev_id);
    hmac_ftm_stru *hmac_ftm_info = (hmac_ftm_stru *)hmac_vap->feature_ftm;

    if (oal_any_null_ptr2(hmac_ftm_info, hal_device)) {
        return OAL_FAIL;
    }
    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);

    /* 获取帧头信息 */
    frame_hdr = (mac_ieee80211_frame_stru *)MAC_GET_RX_CB_MAC_HEADER_ADDR(&rx_ctrl->st_rx_info);

    /* 获取帧体指针 */
    payload = mac_netbuf_get_payload(netbuf);
    frame_len = MAC_GET_RX_CB_PAYLOAD_LEN(&(rx_ctrl->st_rx_info)); /* 帧体长度 */

    /* ftm initiator */
    if (OAL_FALSE == mac_mib_get_FtmInitiatorModeActivated(&hmac_vap->st_vap_base_info)) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                         "{hmac_ftm_initiator_rx_rsp::not a ftm_initiator, ignored rx ftm_req.}");
        return OAL_SUCC;
    }

    /* 查找session */
    ftm_initiator_session = hmac_ftm_initiator_find_session(hmac_ftm_info, frame_hdr->auc_address2);
    if (ftm_initiator_session == NULL) {
        return OAL_SUCC;
    }

    hmac_ftm_info->initiator_session = ftm_initiator_session;
    ftm_initiator_session->dialog_token = payload[FTM_FRAME_DIALOG_TOKEN_OFFSET];
    ftm_initiator_session->follow_up_dialog_token = payload[FTM_FRAME_FOLLOWUP_DIALOG_TOKEN_OFFSET];

    /* FTM认证，暂无要求需要解析，因此先跳过这些信元 */
    hmac_ftm_initiator_lci_civic_ie(ftm_initiator_session, payload, frame_len, &len_to_skip);

    mac_ftmp = (mac_ftm_parameters_ie_stru *)mac_find_ie(MAC_EID_FTMP, payload + len_to_skip,
                                                         (frame_len - len_to_skip));
    /* 每个burst的第一个ftm帧，需要更新ftm参数 */
    if (mac_ftmp != NULL && (mac_ftmp->uc_len == (sizeof(mac_ftm_parameters_ie_stru) - MAC_IE_HDR_LEN))) {
        return hmac_ftm_initiator_update_first_ftm_param(hmac_vap, payload, frame_len, ftm_initiator_session);
    }

    /* 记录rssi */
    hmac_ftm_initiator_save_rssi(ftm_initiator_session, rx_ctrl);

    /* dialog匹配时计算rtt */
    hmac_ftm_initiator_get_rtt(hmac_vap, ftm_initiator_session, payload);

    /* dialog token 为0时表示rspder想终止会话，需要处理已有数据 */
    if (ftm_initiator_session->dialog_token == 0) {
        hmac_ftm_initiator_end_session(hal_device, ftm_initiator_session);
        return OAL_SUCC;
    }

    return OAL_SUCC;
}


OAL_STATIC void hmac_ftm_dump_responder_session_info(ftm_responder_stru *ftm_responder_session)
{
    oam_warning_log3(0, OAM_SF_FTM,
        "hmac_ftm_dump_responder_session_info::reg0[%x], reg1[%x], reg2[%x]",
        ftm_responder_session->toda_info.ftm_rpt_reg0.reg_value,
        ftm_responder_session->toda_info.ftm_rpt_reg1.reg_value,
        ftm_responder_session->toda_info.ftm_rpt_reg2.reg_value);

    oam_warning_log4(0, OAM_SF_FTM,
        "hmac_ftm_dump_responder_session_info::reg3[%x], reg4[%x], mac = [%d], dialog_token[%d]",
        ftm_responder_session->toda_info.ftm_rpt_reg3.reg_value,
        ftm_responder_session->toda_info.ftm_rpt_reg4.reg_value,
        ftm_responder_session->mac_ra[5],
        ftm_responder_session->follow_up_dialog_token);
}

static void hmac_ftm_get_rssi_selection(hmac_ftm_stru *hmac_ftm_info, hal_ftm_info_stru *hal_ftm_info)
{
    uint8_t chan_idx;
    int8_t rssi_value;

    rssi_value = OAL_RSSI_INIT_VALUE;
    hmac_ftm_info->ftm_rssi_selection = 0;

    for (chan_idx = 0; chan_idx < HAL_HOST_MAX_RF_NUM; ++chan_idx) {
        if (hal_ftm_info->ftm_rssi_ant[chan_idx] > rssi_value) {
            rssi_value = hal_ftm_info->ftm_rssi_ant[chan_idx];
            hmac_ftm_info->ftm_rssi_selection = chan_idx;
        }
    }
}


static void hmac_ftm_save_correlation_peak(ftm_toda_stru *toda_info,
                                           hal_ftm_info_stru *ftm_info)
{
    toda_info->ftm_rpt_reg0.reg0.bit_phase_incr = ftm_info->ftm_phase_incr_ant;
    /* todo:ppu字段中有C0和C1的snr，当前只使用了C0的snr */
    toda_info->ftm_rpt_reg0.reg0.bit_snr = ftm_info->ftm_snr_ant[0];

    toda_info->ftm_rpt_reg1.reg1.bit_max_power = ftm_info->ftm_power[0].reg.bit_msb;
    toda_info->ftm_rpt_reg1.reg1.bit_left0_power = ftm_info->ftm_power[1].reg.bit_lsb;
    toda_info->ftm_rpt_reg2.reg2.bit_r0_power = ftm_info->ftm_power[0].reg.bit_lsb;
    toda_info->ftm_rpt_reg2.reg2.bit_l1_power = ftm_info->ftm_power[1].reg.bit_msb;
    toda_info->ftm_rpt_reg3.reg3.bit_l2_power = ftm_info->ftm_power[2].reg.bit_lsb;
    toda_info->ftm_rpt_reg3.reg3.bit_l3_power = ftm_info->ftm_power[2].reg.bit_msb;
    toda_info->ftm_rpt_reg4.reg4.bit_l4_power = ftm_info->ftm_power[3].reg.bit_lsb;
    toda_info->ftm_rpt_reg4.reg4.bit_l5_power = ftm_info->ftm_power[3].reg.bit_msb;
}


int32_t hmac_ftm_toa_error_estimate(ftm_toda_stru *toda_info)
{
    /* 最大功率门限，丢弃该测量 */
    if (toda_info->ftm_rpt_reg1.reg1.bit_left0_power < 8) {
        return (int32_t)255;
    }

    if (toda_info->ftm_rpt_reg4.reg4.bit_l5_power >= 2) {
        return (int32_t)36;
    }

    if (toda_info->ftm_rpt_reg4.reg4.bit_l4_power >= 3) {
        return (int32_t)30;
    }

    if (toda_info->ftm_rpt_reg3.reg3.bit_l3_power >= 5) {
        return (int32_t)24;
    }

    if (toda_info->ftm_rpt_reg3.reg3.bit_l2_power >= 6) {
        return (int32_t)18;
    }

    return (int32_t)0;
}


uint32_t hmac_ftm_initiator_handle_ppu(hmac_vap_stru *hmac_vap, hal_ftm_info_stru *hal_ftm_info)
{
    hmac_ftm_stru *hmac_ftm_info = (hmac_ftm_stru *)hmac_vap->feature_ftm;
    ftm_initiator_stru *ftm_initiator_session = NULL;
    uint64_t tmp_t2;
    uint64_t tmp_t3;
    int8_t c_time_intp;
    int32_t l_fix = 0;
    uint16_t toa_error = 0;
    hal_host_device_stru *hal_device = hal_get_host_device(hmac_vap->hal_dev_id);

    if (oal_any_null_ptr2(hmac_ftm_info, hal_device)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ftm_initiator_session = hmac_ftm_info->initiator_session;
    if (ftm_initiator_session == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(&ftm_initiator_session->toda_info, sizeof(ftm_toda_stru), 0, sizeof(ftm_toda_stru));
    if (hal_ftm_info->ftm_dialog_token == 0) {
        return OAL_FAIL;
    }

     /* 默认在rx complete中断已经处理完上次测量的rtt，因此在tx ack complete中断中直接保存测量值即可 */
    ftm_initiator_session->toda_info.dialog_token = hal_ftm_info->ftm_dialog_token;

    hal_host_ftm_get_divider(hal_device, &(hmac_ftm_info->divider));

    tmp_t2 = hal_ftm_info->ftm_rx_time;
    tmp_t3 = hal_ftm_info->ftm_tx_time;

    if ((tmp_t2 == 0) || (tmp_t3 == 0)) {
        return OAL_FAIL;
    }

    c_time_intp = hal_ftm_info->ftm_intp_time;

    hmac_ftm_get_rssi_selection(hmac_ftm_info, hal_ftm_info);
    hmac_ftm_save_correlation_peak(&ftm_initiator_session->toda_info, hal_ftm_info);

    /* 修复多径问题，多径无法修复时，直接丢弃测量值 */
    if (hmac_ftm_info->multipath) {
        l_fix = hmac_ftm_toa_error_estimate(&ftm_initiator_session->toda_info);
        if (l_fix == 255) {
            return OAL_SUCC;
        }

        if (l_fix) {
            /* 进行多径修复时，插值部分清零 */
            c_time_intp = 0;
            toa_error = ((6 * FTM_TIME_CALC_RATIO) << 10) / hmac_ftm_info->divider;
        }
    }

    /* 计算时间，单位皮秒 当前未考虑l_fix和aul_air_overhead时间，调试校准时再修改
    * 公式:tmp_t2(clk cnt) * 时钟周期 - c_time_intp(插值到960M的) =
    * (1/960M) * [tmp_t2 * 6 / divider - c_time_intp] 1/960M为us,转化为ps需乘10^6，10^6/960约等价于左移10bit
    */
    tmp_t2 = ((tmp_t2 * FTM_TIME_CALC_RATIO / hmac_ftm_info->divider - (uint8_t)c_time_intp) << BIT_OFFSET_10) -
        (uint32_t)l_fix;
    tmp_t3 = ((tmp_t3 * FTM_TIME_CALC_RATIO / hmac_ftm_info->divider + hmac_ftm_info->ftm_cali_time) << BIT_OFFSET_10)
        + hmac_ftm_info->air_overhead;

    ftm_initiator_session->toda_info.tod = tmp_t3;
    ftm_initiator_session->toda_info.toa = tmp_t2;
    ftm_initiator_session->toda_info.toa_error = toa_error;

    return OAL_SUCC;
}


uint32_t hmac_ftm_responder_handle_ppu(hmac_vap_stru *hmac_vap, hal_ftm_info_stru *hal_ftm_info)
{
    hmac_ftm_stru *hmac_ftm_info = NULL;
    ftm_responder_stru *ftm_responder_session = NULL;
    uint64_t tmp_t1;
    uint64_t tmp_t4;
    int8_t c_time_intp;
    int32_t l_fix = 0;
    uint16_t toa_error = 0;
    hal_host_device_stru *hal_device = hal_get_host_device(hmac_vap->hal_dev_id);

    /* responder模式，记录t1和t4 */
    hmac_ftm_info = (hmac_ftm_stru *)hmac_vap->feature_ftm;
    if (oal_any_null_ptr2(hmac_ftm_info, hal_device)) {
        return OAL_FAIL;
    }
    ftm_responder_session = hmac_ftm_responder_find_session(hmac_ftm_info, hal_ftm_info->ftm_ra);
    if (ftm_responder_session == NULL) {
        return OAL_SUCC;
    }

    memset_s(&ftm_responder_session->toda_info, sizeof(ftm_toda_stru), 0, sizeof(ftm_toda_stru));

    hal_host_ftm_get_divider(hal_device, &(hmac_ftm_info->divider));

    tmp_t1 = hal_ftm_info->ftm_tx_time;
    tmp_t4 = hal_ftm_info->ftm_rx_time;

    if ((tmp_t1 == 0) || (tmp_t4 == 0)) {
        return OAL_FAIL;
    }

     /* 计算时间，单位皮秒 */
    c_time_intp = hal_ftm_info->ftm_intp_time;

    hmac_ftm_get_rssi_selection(hmac_ftm_info, hal_ftm_info);
    hmac_ftm_save_correlation_peak(&ftm_responder_session->toda_info, hal_ftm_info);

    if (hmac_ftm_info->multipath) {
        l_fix = hmac_ftm_toa_error_estimate(&ftm_responder_session->toda_info);
        if (l_fix == 255) {
            oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                "hmac_ftm_responder_handle_ppu:can not fix, drop!");
            return OAL_SUCC;
        }

        if (l_fix) {
            c_time_intp = 0;
            toa_error = ((6 * FTM_TIME_CALC_RATIO) << 10) / hmac_ftm_info->divider;
        }
    }

    /* 计算时间，单位皮秒 当前未考虑l_fix和aul_air_overhead时间，调试校准时再修改
     * 公式:tmp_t1/t4(clk cnt) * 时钟周期 - c_time_intp(插值到960M的) =
     * (1/960M) * [tmp_t1 * 6 / divider - c_time_intp] 1/960M为us,转化为ps需乘10^6，10^6/960约等价于左移10bit
     */
    tmp_t1 = ((tmp_t1 * FTM_TIME_CALC_RATIO / hmac_ftm_info->divider + hmac_ftm_info->ftm_cali_time) << BIT_OFFSET_10)
        + hmac_ftm_info->air_overhead;

    tmp_t4 = ((tmp_t4 * FTM_TIME_CALC_RATIO / hmac_ftm_info->divider - (uint8_t)c_time_intp) << BIT_OFFSET_10) -
        (uint32_t)l_fix;

    ftm_responder_session->toda_info.tod = tmp_t1;
    ftm_responder_session->toda_info.toa = tmp_t4;
    ftm_responder_session->toda_info.toa_error = toa_error;
    hmac_ftm_dump_responder_session_info(ftm_responder_session);

    return OAL_SUCC;
}


void hmac_ftm_initiator_set_session(mac_send_iftmr_stru *send_iftmr, ftm_initiator_stru *ftm_ini_session)
{
    memcpy_s(&ftm_ini_session->channel_ftm, sizeof(mac_channel_stru),
             &send_iftmr->st_channel, sizeof(mac_channel_stru));
    switch (send_iftmr->bw) {
        case WIFI_RTT_BW_20:
            ftm_ini_session->band_cap = WLAN_BW_CAP_20M;
            break;
        case WIFI_RTT_BW_40:
            ftm_ini_session->band_cap = WLAN_BW_CAP_40M;
            break;
        case WIFI_RTT_BW_80:
            ftm_ini_session->band_cap = WLAN_BW_CAP_80M;
            break;
        default:
            ftm_ini_session->band_cap = WLAN_BW_CAP_20M;
            break;
    }
    if (send_iftmr->preamble == WIFI_RTT_PREAMBLE_HT) {
        ftm_ini_session->prot_format = WLAN_HT_PHY_PROTOCOL_MODE;
    } else if (send_iftmr->preamble == WIFI_RTT_PREAMBLE_VHT) {
        ftm_ini_session->prot_format = WLAN_VHT_PHY_PROTOCOL_MODE;
    } else {
        ftm_ini_session->prot_format = WLAN_HT_PHY_PROTOCOL_MODE;
    }
}


uint32_t hmac_ftm_initiator_initial_request(hmac_vap_stru *hmac_vap, mac_send_iftmr_stru *send_iftmr)
{
    ftm_initiator_stru *ftm_initiator_session;
    hmac_ftm_stru *ftm_info = NULL;
    ftm_info = (hmac_ftm_stru *)hmac_vap->feature_ftm;

    if (ftm_info == NULL) {
        return OAL_FAIL;
    }
    ftm_initiator_session = hmac_ftm_initiator_alloc_session(hmac_vap, send_iftmr->mac_addr);
    if (ftm_initiator_session == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
            "{hmac_ftm_initiator_initial_request::no more valid session.}");
        return OAL_FAIL;
    }

    hmac_ftm_initiator_set_session(send_iftmr, ftm_initiator_session);
    ftm_initiator_session->lci_ie = send_iftmr->lci_request;
    ftm_initiator_session->civic_ie = send_iftmr->lcr_request;
    ftm_initiator_session->burst_cnt = (uint16_t)BIT(oal_min(send_iftmr->burst_num,
                                                             MAC_FTM_MAX_NUM_OF_BURSTS_EXPONENT));
    ftm_initiator_session->is_asap_on = send_iftmr->is_asap_on;
    ftm_initiator_session->min_delta_ftm = FTM_MIN_DELTA;
    ftm_initiator_session->ftms_per_burst = (uint8_t)(oal_min(MAC_FTM_MAX_FTMS_PER_BURST,
        oal_max(send_iftmr->ftms_per_burst, MIN_FTMS_PER_BURST)));
    ftm_initiator_session->burst_period =
        (send_iftmr->burst_period == 0) ? 0 : (uint16_t)oal_min(FTM_BURST_PERIOD, send_iftmr->burst_period);
    ftm_initiator_session->bit_range = 0;
    ftm_initiator_session->is_initial_ftm_request = OAL_TRUE;
    ftm_initiator_session->tx_ftmr_timeout_fix = ftm_info->initiator_tsf_fix_offset;
    ftm_initiator_session->burst_duration =
        (uint8_t)(ftm_initiator_session->ftms_per_burst * (ftm_initiator_session->min_delta_ftm /
        FTM_MIN_DELTA_TO_DURATION_PARA) + FTM_BURST_DURATION_OFFSET);

    hmac_ftm_initiator_start_burst(hmac_vap, ftm_initiator_session);

    oam_warning_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                     "{hmac_ftm_initiator_initial_request:: ftm session with ap[%x:%x:%x] ACTIVE NOW !!!}",
                     ftm_initiator_session->bssid[MAC_ADDR_3],
                     ftm_initiator_session->bssid[MAC_ADDR_4],
                     ftm_initiator_session->bssid[MAC_ADDR_5]);

    return OAL_SUCC;
}


static uint32_t hmac_ftm_tx_neighbour_report_request(mac_vap_stru *mac_vap,
    mac_neighbor_report_req_stru *nbr_req)
{
    uint32_t ret;
    oal_netbuf_stru *action_neighbor_req = NULL;
    uint16_t neighbor_req_frm_len;
    mac_tx_ctl_stru *tx_ctl = NULL;
    uint16_t idx;
    uint8_t *mac_header = NULL;
    uint8_t *data = NULL;
    mac_user_stru *mac_user = NULL;

    mac_user = mac_res_get_mac_user(mac_vap->us_assoc_vap_id);
    if (mac_user == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_FTM, "{hmac_ftm_tx_neighbour_report_request::mac_user[%d] null.",
                         mac_vap->us_assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    action_neighbor_req =
        (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (action_neighbor_req == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_config_send_neighbor_req::action_neighbor_req null.}");
        return OAL_FAIL;
    }

    memset_s(oal_netbuf_cb(action_neighbor_req), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* All the fields of the Frame Control Field are set to zero. Only the   */
    /* Type/Subtype field is set.                                            */
    mac_header = (uint8_t *)oal_netbuf_header(action_neighbor_req);
    hmac_ftm_set_mgmt_mac_hdr(mac_header, nbr_req->auc_bssid, mac_mib_get_StationID(mac_vap), nbr_req->auc_bssid);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*             Neighbor report request Frame - Frame Body                */
    /*        -------------------------------------------------              */
    /*        | Category | Action |  Dialog Token | Opt SubEle |             */
    /*        -------------------------------------------------              */
    /*        | 1        | 1      |       1       | Var        |             */
    /*        -------------------------------------------------              */
    /*************************************************************************/
    /* Initialize index and the frame data pointer */
    data = mac_netbuf_get_payload(action_neighbor_req);
    idx = 0;
    /* Category */
    data[idx++] = MAC_ACTION_CATEGORY_RADIO_MEASURMENT;

    /* Action */
    data[idx++] = MAC_RM_ACTION_NEIGHBOR_REPORT_REQUEST;

    /* Dialog Token */
    data[idx++] = 1;

    /*
    * FTM认证，组帧neighbour report Request，需要包含lci和location civic信元
    */
    /*****************************************************************************************************/
    /* LCI Measurement Request (Measurement Request IE) */
    /* ------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |Measurement Token| Measurement Req Mode|Measurement Type  | Measurement Req | */
    /* ------------------------------------------------------------------------------------------------- */
    /* |1          |1      | 1               | 1                   |1                  |var             | */
    /* ------------------------------------------------------------------------------------------------- */
    /*****************************************************************************************************/
    /* 封装Measurement Request IE */
    idx += hmac_ftm_set_lci_field(data + idx);
    /* 封装Measurement Request IE */
    idx += hmac_ftm_set_civic_field(data + idx);

    neighbor_req_frm_len = idx + MAC_80211_FRAME_LEN;

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(action_neighbor_req);
    MAC_GET_CB_MPDU_LEN(tx_ctl) = neighbor_req_frm_len;
    /* 发送完成需要获取user结构体 */
    ret = mac_vap_set_cb_tx_user_idx(mac_vap, tx_ctl, mac_user->auc_user_mac_addr);
    if (ret != OAL_SUCC) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_TX, "(hmac_ftm_tx_neighbour_report_request::fail to find user by \
            xx:xx:xx:0x:0x:0x.}", mac_user->auc_user_mac_addr[MAC_ADDR_3],
            mac_user->auc_user_mac_addr[MAC_ADDR_4], mac_user->auc_user_mac_addr[MAC_ADDR_5]);
    }

    oal_netbuf_put(action_neighbor_req, neighbor_req_frm_len);

    ret = hmac_tx_mgmt_send_event(mac_vap, action_neighbor_req, neighbor_req_frm_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(action_neighbor_req);
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_ftm_tx_neighbour_report_request::failed[%d]}", ret);
    }
    return OAL_SUCC;
}


void hmac_ftm_handle(hal_host_location_isr_stru *ftm_csi_isr)
{
    hal_ftm_info_stru hal_ftm_info = {0};
    uint8_t *ftm_addr = NULL;
    dma_addr_t host_iova = 0;
    uint64_t ftm_base_addr;
    hmac_vap_stru *hmac_vap = NULL;

    ftm_base_addr = ftm_csi_isr->ftm_info_addr;
    if (OAL_SUCC != pcie_if_devva_to_hostca(0, ftm_base_addr, (uint64_t *)&host_iova)) {
        oam_warning_log0(0, OAM_SF_FTM, "{hmac_ftm_handle:get ftm addr fail.}");
        return;
    }

    ftm_addr = (uint8_t *)(uintptr_t)host_iova;
    hal_host_ftm_get_info(&hal_ftm_info, ftm_addr);
    hmac_vap = hal_host_vap_get_hal_vap_id(ftm_csi_isr->hal_device_id, hal_ftm_info.vap_index);
    if (hmac_vap == NULL) {
        return;
    }

    if (hal_ftm_info.ftm_init_resp_flag == 1) {
        hmac_ftm_initiator_handle_ppu(hmac_vap, &hal_ftm_info);
    } else {
        hmac_ftm_responder_handle_ppu(hmac_vap, &hal_ftm_info);
    }
}


uint32_t hmac_event_ftm_cali(frw_event_mem_stru *event_mem)
{
    hmac_vap_stru *hmac_vap = NULL;
    frw_event_stru *event = NULL;
    dmac_to_hmac_ftm_ctx_event_stru *ftm_cali_event = NULL;
    hmac_ftm_stru *hmac_ftm_info = NULL;

    event = frw_get_event_stru(event_mem);

    ftm_cali_event = (dmac_to_hmac_ftm_ctx_event_stru *)event->auc_event_data;

    hmac_vap = mac_res_get_hmac_vap(ftm_cali_event->vap_id);
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_ftm_info = hmac_vap->feature_ftm;
    if (hmac_ftm_info == NULL) {
        return OAL_FAIL;
    }

    hmac_ftm_info->ftm_cali_time = ftm_cali_event->ftm_cali_time;
    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_ftm_send_iftmr_info(mac_vap_stru *mac_vap, mac_ftm_debug_switch_stru *ftm_debug)
{
    mac_bss_dscr_stru              *bss_dscr = NULL;
    bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(mac_vap,
        ftm_debug->st_send_iftmr_bit1.mac_addr);
    if (bss_dscr != NULL) {
        if (ftm_debug->st_send_iftmr_bit1.channel_num != bss_dscr->st_channel.uc_chan_number) {
            oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_FTM,
                             "{hmac_ftm_send_iftmr_info::send iftmr:AP [*:%x:%x] tx channel %d, operation channel %d.}",
                             ftm_debug->st_send_iftmr_bit1.mac_addr[MAC_ADDR_4],
                             ftm_debug->st_send_iftmr_bit1.mac_addr[MAC_ADDR_5],
                             ftm_debug->st_send_iftmr_bit1.channel_num,
                             bss_dscr->st_channel.uc_chan_number);
            ftm_debug->st_send_iftmr_bit1.channel_num = bss_dscr->st_channel.uc_chan_number;
        }
        if (memcpy_s(&ftm_debug->st_send_iftmr_bit1.st_channel, sizeof(mac_channel_stru),
            &bss_dscr->st_channel, sizeof(mac_channel_stru)) != EOK) {
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_FTM,
                "hmac_ftm_send_iftmr_info::st_channel memcpy fail!");
        }
        if (hmac_ftm_check_dbg_params(&ftm_debug->st_send_iftmr_bit1) != OAL_SUCC) {
            return OAL_FAIL;
        }
        hmac_config_ftm_dbg_set_bw_and_preamble(ftm_debug);
    }
    return OAL_SUCC;
}

OAL_STATIC void hmac_ftm_print_info(mac_vap_stru *mac_vap, mac_ftm_debug_switch_stru *ftm_debug)
{
    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_FTM,
                     "{hmac_ftm_print_info::send initial ftm request to [*:%x:%x].}",
                     ftm_debug->st_send_iftmr_bit1.mac_addr[MAC_ADDR_4],
                     ftm_debug->st_send_iftmr_bit1.mac_addr[MAC_ADDR_5]);
    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_FTM,
                     "{hmac_ftm_print_info::send initial ftm request channel[%d],burst[%d->%d].}",
                     ftm_debug->st_send_iftmr_bit1.channel_num,
                     ftm_debug->st_send_iftmr_bit1.burst_num,
                     (uint16_t)BIT(ftm_debug->st_send_iftmr_bit1.burst_num));
    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_FTM,
                     "{hmac_ftm_print_info::send initial ftm request lci&civic[%d],\
                     ftms per burst[%d], asap[%d], bw[%d].}",
                     ftm_debug->st_send_iftmr_bit1.lci_request,
                     ftm_debug->st_send_iftmr_bit1.ftms_per_burst,
                     ftm_debug->st_send_iftmr_bit1.is_asap_on,
                     ftm_debug->st_send_iftmr_bit1.format_bw);
}


uint32_t hmac_ftm_responder_dbg_send(hmac_vap_stru *hmac_vap, uint8_t mac_addr[WLAN_MAC_ADDR_LEN])
{
    oam_warning_log4(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                     "{hmac_ftm_responder_dbg_send::send ftm frame to [%x:*:*:%x:%x:%x].}",
                     mac_addr[MAC_ADDR_0], mac_addr[MAC_ADDR_3], mac_addr[MAC_ADDR_3], mac_addr[MAC_ADDR_5]);

    return OAL_SUCC;
}


void hmac_ftm_set_air_overhead(mac_vap_stru *mac_vap, hmac_ftm_stru *ftm_info, uint32_t ftm_correct_time)
{
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_FTM,
                     "{hmac_ftm_set_air_overhead::set ftm air overhead: [%d].}", ftm_correct_time);
    ftm_info->air_overhead = ftm_correct_time;
}


OAL_STATIC void hmac_ftm_set_cali(mac_vap_stru *mac_vap, hmac_ftm_stru *ftm_info, uint32_t ftm_cali)
{
    ftm_info->ftm_cali_time = ftm_cali;
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_FTM, "{hmac_ftm_set_cali::set cali time[%d]", ftm_cali);
}


OAL_STATIC void hmac_ftm_set_multipath_on(hmac_ftm_stru *ftm_info,
    oal_bool_enum_uint8 multipath_on, uint32_t is_cmd_on)
{
    ftm_info->multipath = is_cmd_on ? multipath_on : ftm_info->multipath;
}


OAL_STATIC void hmac_ftm_set_tsf_fix(hmac_ftm_stru *ftm_info, mac_ftm_timeout_stru *ftm_timeout,
                                     uint32_t is_cmd_on)
{
    if (!is_cmd_on) {
        return;
    }
    ftm_info->initiator_tsf_fix_offset = ftm_timeout->initiator_tsf_fix_offset;
    ftm_info->res_ptsf_delta = ftm_timeout->responder_ptsf_offset;
}

OAL_STATIC void hmac_ftm_set_m2s(mac_vap_stru *mac_vap, hal_host_device_stru *hal_device,
    hmac_ftm_stru *ftm_info, uint8_t ftm_chain_selection)
{
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_FTM, "{hmac_ftm_set_m2s::selection [%d].}", ftm_chain_selection);
    ftm_info->ftm_chain_selection = ftm_chain_selection & 0x3;
    hal_host_ftm_set_m2s_phy(hal_device, ftm_info->ftm_chain_selection,
                             ftm_info->ftm_rssi_selection);
}


OAL_STATIC void hmac_ftm_cali_enable(mac_vap_stru *mac_vap, hmac_ftm_stru *ftm_info,
    oal_bool_enum_uint8 ftm_status)
{
    ftm_info->cali = ftm_status;
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_FTM, "{hmac_ftm_cali_enable::ftm cali enable[%d].}", ftm_status);
    if (mac_vap->bit_al_tx_flag == OAL_SWITCH_ON) {
        /* 常发环回校准值处理处理 */
    }
}

void hmac_config_rtt_config_iftmr(mac_vap_stru *mac_vap, mac_send_iftmr_stru *mac_send_iftmr, uint8_t *param)
{
    mac_ftm_wifi_rtt_config *wifi_rtt_config = NULL;
    mac_bss_dscr_stru *mac_bss_dscr = NULL;

    wifi_rtt_config = (mac_ftm_wifi_rtt_config *)param;

    oal_set_mac_addr(mac_send_iftmr->mac_addr, wifi_rtt_config->addr);

    mac_send_iftmr->is_asap_on = OAL_TRUE;
    mac_send_iftmr->lci_request = wifi_rtt_config->lci_request;
    mac_send_iftmr->lcr_request = wifi_rtt_config->lcr_request;
    mac_send_iftmr->burst_num = wifi_rtt_config->num_burst;
    mac_send_iftmr->ftms_per_burst = wifi_rtt_config->num_frames_per_burst;
    mac_send_iftmr->burst_duration = wifi_rtt_config->burst_duration;
    mac_send_iftmr->burst_period = wifi_rtt_config->burst_period;
    mac_send_iftmr->channel_num = oal_ieee80211_frequency_to_channel(wifi_rtt_config->channel.center_freq);
    mac_send_iftmr->preamble = wifi_rtt_config->preamble;
    mac_send_iftmr->bw = wifi_rtt_config->bw;

     /* 发送iftmr命令 */
    mac_bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(mac_vap, wifi_rtt_config->addr);
    if (mac_bss_dscr == NULL) {
        return;
    }
    mac_send_iftmr->channel_num = oal_ieee80211_frequency_to_channel(wifi_rtt_config->channel.center_freq);
    if (mac_send_iftmr->channel_num != mac_bss_dscr->st_channel.uc_chan_number) {
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_FTM,
            "{hmac_config_rtt_config_iftmr::send iftmr:AP [**:%x:%x] tx channel %d, set to ap's channel %d}",
            wifi_rtt_config->addr[MAC_ADDR_4], wifi_rtt_config->addr[MAC_ADDR_5],
            mac_send_iftmr->channel_num, mac_bss_dscr->st_channel.uc_chan_number);
            mac_send_iftmr->channel_num = mac_bss_dscr->st_channel.uc_chan_number;
    }
    mac_send_iftmr->st_channel.uc_chan_number = mac_send_iftmr->channel_num;
    mac_send_iftmr->st_channel.en_band = mac_get_band_by_channel_num(mac_send_iftmr->channel_num);
    mac_send_iftmr->st_channel.en_bandwidth = hmac_ftm_get_channel_bandwidth(wifi_rtt_config->channel.center_freq0,
        mac_send_iftmr->channel_num, wifi_rtt_config->channel.width);
    mac_get_channel_idx_from_num(mac_send_iftmr->st_channel.en_band, mac_send_iftmr->st_channel.uc_chan_number,
        mac_send_iftmr->st_channel.ext6g_band, &(mac_send_iftmr->st_channel.uc_chan_idx));
}

uint32_t hmac_config_wifi_rtt_config_1106(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    hal_host_device_stru *hal_device = NULL;

    hmac_ftm_stru *ftm_info = NULL;
    mac_send_iftmr_stru mac_send_iftmr;

    if (hmac_vap == NULL) {
        return OAL_FAIL;
    }

    hal_device = hal_get_host_device(hmac_vap->hal_dev_id);
    ftm_info = (hmac_ftm_stru *)hmac_vap->feature_ftm;
    if (oal_any_null_ptr2(ftm_info, hal_device)) {
        return OAL_FAIL;
    }
    memset_s(&mac_send_iftmr, sizeof(mac_send_iftmr_stru), 0, sizeof(mac_send_iftmr_stru));
    hmac_config_rtt_config_iftmr(mac_vap, &mac_send_iftmr, param);

    if (mac_is_ftm_enable(mac_vap)) {
        hal_host_ftm_set_enable(hal_device, OAL_TRUE);
        ftm_info->cali = OAL_TRUE;
        mac_send_iftmr.is_app_wifi_rtt = OAL_TRUE;
    }

    if (hmac_ftm_initiator_initial_request(hmac_vap, &mac_send_iftmr) != OAL_SUCC) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_FTM,
            "hmac_config_wifi_rtt_config_1106::initial_request fail!");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

static uint8_t hmac_rrm_parse_ftm_range_req_param_check(hmac_ftm_stru *ftm_info, hmac_vap_stru *hmac_vap_sta,
    mac_meas_req_ie_stru *meas_req_ie, ftm_range_req_stru *ftm_range_req)
{
    if (mac_mib_get_FtmRangeReportActivated(&hmac_vap_sta->st_vap_base_info) == OAL_FALSE) {
        return OAL_FALSE;
    }

    if (ftm_info->report_range == OAL_TRUE) {
        oam_error_log1(hmac_vap_sta->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
            "{hmac_rrm_parse_ftm_range_req_param_check::MEAS_TOKEN[%d] is in process.}", ftm_range_req->meas_token);
        return OAL_FALSE;
    }

    if (meas_req_ie->uc_len < MAC_MEASUREMENT_REQUEST_IE_OFFSET + FTM_RANGE_IE_OFFSET) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}


OAL_STATIC void hmac_initialize_meas_rpt_ie(mac_meas_rpt_ie_stru *meas_rpt_ie)
{
    meas_rpt_ie->uc_eid = MAC_EID_MEASREP;
    meas_rpt_ie->uc_token = 1;
    memset_s(&(meas_rpt_ie->st_rptmode), sizeof(mac_meas_rpt_mode_stru), 0, sizeof(mac_meas_rpt_mode_stru));
    meas_rpt_ie->uc_rpttype = RM_RADIO_MEASUREMENT_FTM_RANGE;
}


uint16_t hmac_set_meas_rpt_ie_field(mac_meas_rpt_ie_stru *meas_rpt_ie, ftm_range_rpt_stru *ftm_range_rpt)
{
    uint8_t *data = NULL;
    uint16_t idx = 0;
    uint16_t i;
    int32_t ret = EOK;
    uint32_t range = 0;
    /******************************************************************************************/
    /* Fine Timing Measurement Range report */
    /* --------------------------------------------------------------------------------------- */
    /* |Range Entry Count |Range Entry |Error Entry Count| Error Entry |Optional Subelements | */
    /* --------------------------------------------------------------------------------------- */
    /* |1                 |M x 15      |1                | N x 11      | var                 | */
    /* --------------------------------------------------------------------------------------- */
    /******************************************************************************************/
    data = meas_rpt_ie->auc_meas_rpt;
    /******************************************************************************/
    /* Range Entry */
    /* --------------------------------------------------------------------------- */
    /* |Measurement Start Time |BSSID |Range |Max Range Error Exponent |Reserved | */
    /* --------------------------------------------------------------------------- */
    /* |4                      |6     |3     |  1                     | 1        | */
    /* --------------------------------------------------------------------------- */
    /******************************************************************************/
    data[idx++] = ftm_range_rpt->range_entry_count;
    /* FTM认证，打桩部分字段为0 */
    for (i = 0; i < ftm_range_rpt->range_entry_count; i++) {
        ret += memcpy_s(&data[idx], MEAS_RPT_IE_LEN - idx,
                        &ftm_range_rpt->ftm_range_entry[i].measurement_start_time, sizeof(uint32_t));
        idx += sizeof(uint32_t);
        ret += memcpy_s(&data[idx], MEAS_RPT_IE_LEN - idx,
                        ftm_range_rpt->ftm_range_entry[i].bssid, WLAN_MAC_ADDR_LEN);
        idx += WLAN_MAC_ADDR_LEN;
        ret += memcpy_s(&data[idx], 3, &range, 3); /* 3 is Range len all is zero */
        idx += 3;  /* 3 is Range len all is zero */
        data[idx++] = 0;   /* reserved */
        data[idx++] = 0;  /* reserved */
    }

    /***********************************************/
    /* Error Entry */
    /* -------------------------------------------- */
    /* |Measurement Start Time |BSSID |Error Code | */
    /* -------------------------------------------- */
    /* |4                      |6     |1          | */
    /* -------------------------------------------- */
    /***********************************************/
    data[idx++] = ftm_range_rpt->error_entry_count;
    for (i = 0; i < ftm_range_rpt->error_entry_count; i++) {
        ret += memcpy_s(&data[idx], MEAS_RPT_IE_LEN - idx,
                        &ftm_range_rpt->ftm_error_entry[i].measurement_start_time, sizeof(uint32_t));
        idx += sizeof(uint32_t);
        ret += memcpy_s(&data[idx], MEAS_RPT_IE_LEN - idx,
                        ftm_range_rpt->ftm_error_entry[i].bssid, WLAN_MAC_ADDR_LEN);
        idx += WLAN_MAC_ADDR_LEN;
        data[idx++] = ftm_range_rpt->ftm_error_entry[i].error_code;
    }
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_FTM, "hmac_set_meas_rpt_ie_field: memcopy fail!\n");
    }
    return idx;
}


uint16_t hmac_encap_range_report_frame(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buffer, hmac_ftm_stru *ftm_info)
{
    mac_vap_stru *mac_vap = &hmac_vap->st_vap_base_info;
    uint8_t *mac_header = oal_netbuf_header(buffer);
    uint8_t *payload_addr = mac_netbuf_get_payload(buffer);
    mac_meas_rpt_ie_stru *meas_rpt_ie = NULL;
    uint16_t idx = 0;
    uint16_t ie_len;
    ftm_range_rpt_stru *ftm_range_rpt = NULL;

    ftm_range_rpt = &(ftm_info->ftm_range_rpt);

    /*************************************************************************/
    /* Management Frame Format */
    /* -------------------------------------------------------------------- */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS| */
    /* -------------------------------------------------------------------- */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  | */
    /* -------------------------------------------------------------------- */
    /*************************************************************************/
    hmac_ftm_set_mgmt_mac_hdr(mac_header, mac_vap->auc_bssid,
                              mac_mib_get_StationID(mac_vap), mac_vap->auc_bssid);

     /*************************************************************************************/
    /* Radio Measurement Report frame format */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Radio Measurement Action |Dialog Token | Measurement Report Elements| */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1                       |1             |Variable                    | */
    /* --------------------------------------------------------------------------------- */
    /*************************************************************************************/

    payload_addr[idx++] = MAC_ACTION_CATEGORY_RADIO_MEASURMENT;   /* Category */
    payload_addr[idx++] = MAC_RM_ACTION_RADIO_MEASUREMENT_REPORT; /* Public Action */
    payload_addr[idx++] = 1;                                      /* Dialog Token */

    /*************************************************************************/
    /* Measurement Report IE - Frame Body */
    /* --------------------------------------------------------------------- */
    /* |Element ID |Length |Meas Token| Meas Rpt Mode | Meas Type | Meas Rpt| */
    /* --------------------------------------------------------------------- */
    /* |1          |1      | 1        |  1            | 1         | var */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    meas_rpt_ie = (mac_meas_rpt_ie_stru *)&payload_addr[idx];
    hmac_initialize_meas_rpt_ie(meas_rpt_ie);
    ie_len = hmac_set_meas_rpt_ie_field(meas_rpt_ie, ftm_range_rpt);
    meas_rpt_ie->uc_len = 3 + (uint8_t)ie_len; /* 帧体偏移位置3 */
    idx += (2 + meas_rpt_ie->uc_len);  /* 帧体偏移位置2 */

    return (uint16_t)(idx + MAC_80211_FRAME_LEN);
}


static uint32_t hmac_ftm_tx_range_report(hmac_vap_stru *hmac_vap,
    hmac_ftm_stru *ftm_info)
{
    mac_vap_stru *mac_vap = &hmac_vap->st_vap_base_info;
    oal_netbuf_stru *netbuf = NULL;
    uint16_t frame_len;
    mac_tx_ctl_stru *tx_ctl = NULL;
    uint32_t ret = OAL_SUCC;

    netbuf = (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (netbuf == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_FTM, "{hmac_ftm_tx_range_report::action_neighbor_req null.}");
        return ret;
    }

    memset_s(oal_netbuf_cb(netbuf), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    frame_len = hmac_encap_range_report_frame(hmac_vap, netbuf, ftm_info);

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf); /* 获取cb结构体 */
    MAC_GET_CB_MPDU_LEN(tx_ctl) = frame_len;           /* dmac发送需要的mpdu长度 */

    oal_netbuf_put(netbuf, frame_len);

    /* 抛事件让dmac将该帧发送 */
    ret = hmac_tx_mgmt_send_event(mac_vap, netbuf, frame_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(netbuf);
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_FTM, "{hmac_ftm_tx_range_report::send_event failed.}");
        return ret;
    }
    return OAL_SUCC;
}


uint32_t hmac_ftm_range_request_timeout(void *arg)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)arg;
    hmac_ftm_stru *ftm_info = NULL;
    ftm_range_req_stru *ftm_range_req = NULL;
    mac_send_iftmr_stru send_iftmr;

    if (hmac_vap == NULL) {
        return OAL_FAIL;
    }
    ftm_info = (hmac_ftm_stru *)hmac_vap->feature_ftm;

    if (ftm_info == NULL) {
        return OAL_FAIL;
    }
    ftm_range_req = &ftm_info->ftm_range_req;

    if (ftm_info->report_range == OAL_FALSE) {
        return OAL_SUCC;
    }

    oam_warning_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
        "{hmac_ftm_range_request_timeout with AP[%d], MAC[**:%x:%x].}",
        ftm_range_req->meas_done, ftm_range_req->bssid[ftm_range_req->meas_done][MAC_ADDR_4],
        ftm_range_req->bssid[ftm_range_req->meas_done][MAC_ADDR_5]);

    if (ftm_range_req->meas_done >= ftm_range_req->minimum_ap_count) {
        ftm_range_req->meas_done = 0;
        return hmac_ftm_tx_range_report(hmac_vap, ftm_info);
    }

    send_iftmr.is_asap_on = OAL_TRUE;
    send_iftmr.lci_request = 0;
    send_iftmr.lcr_request = 0;
    send_iftmr.burst_num = 1;
    send_iftmr.channel_num = ftm_range_req->channel[ftm_range_req->meas_done];
    send_iftmr.ftms_per_burst = 2; /* 2:帧数 */
    send_iftmr.format_bw = FTM_FORMAT_BANDWIDTH_VHT_80;
    send_iftmr.bw = WIFI_RTT_BW_80;
    send_iftmr.preamble = WIFI_RTT_PREAMBLE_VHT;
    send_iftmr.st_channel.uc_chan_number = send_iftmr.channel_num;
    send_iftmr.st_channel.en_band = WLAN_BAND_5G;
    send_iftmr.st_channel.ext6g_band = OAL_FALSE;
    send_iftmr.st_channel.en_bandwidth = WLAN_BAND_WIDTH_80PLUSPLUS;  // FTM认证，改为80M
    mac_get_channel_idx_from_num(send_iftmr.st_channel.en_band, send_iftmr.st_channel.uc_chan_number,
        send_iftmr.st_channel.ext6g_band, &send_iftmr.st_channel.uc_chan_idx);

    memcpy_s(send_iftmr.mac_addr, WLAN_MAC_ADDR_LEN,
             &ftm_range_req->bssid[ftm_range_req->meas_done], WLAN_MAC_ADDR_LEN);

    if (hmac_ftm_initiator_initial_request(hmac_vap, &send_iftmr) != OAL_SUCC) {
        return OAL_FAIL;
    }

    ftm_range_req->meas_done += 1;

    frw_timer_create_timer_m(&(ftm_range_req->ftm_range_req_timer),
                             hmac_ftm_range_request_timeout, ftm_range_req->start_delay + 500,  /* 500 时间 */
                             (void *)hmac_vap, OAL_FALSE, OAM_MODULE_ID_HMAC, hmac_vap->st_vap_base_info.core_id);
    return OAL_SUCC;
}


void hmac_rrm_parse_ftm_range_req(hmac_vap_stru *hmac_vap_sta, mac_meas_req_ie_stru *meas_req_ie)
{
    uint8_t *subelements = NULL;
    uint8_t *ie = NULL;
    mac_ftm_range_req_ie_stru *mac_ftm_range_req_ie = NULL;
    mac_neighbor_rpt_ie_stru *neighbor_rpt_ie = NULL;
    ftm_range_req_stru *ftm_range_req = NULL;
    hmac_ftm_stru *ftm_info = (hmac_ftm_stru *)hmac_vap_sta->feature_ftm;
    uint8_t ap_cnt;
    uint8_t ap_index = 0;
    uint8_t len_left;

    if (ftm_info == NULL) {
        return;
    }

    ftm_range_req = &(ftm_info->ftm_range_req);
    if (hmac_rrm_parse_ftm_range_req_param_check(ftm_info, hmac_vap_sta,
                                                 meas_req_ie, ftm_range_req) == OAL_FALSE) {
        return;
    }

    mac_ftm_range_req_ie = (mac_ftm_range_req_ie_stru *)meas_req_ie->auc_meas_req;
    ftm_info->report_range = OAL_TRUE;

    /**************************************************************************/
    /*                 Fine Timing Measurement Range request                  */
    /* ----------------------------------------------------------------------- */
    /* |Randomization Interval|Minimum AP Count Number |FTM Range Subelements| */
    /* ----------------------------------------------------------------------- */
    /* |2                     |1                       | variable            | */
    /* ----------------------------------------------------------------------- */
    /*                                                                        */
    /**************************************************************************/
    ftm_range_req->start_delay = mac_ftm_range_req_ie->us_randomization_interval;
    ftm_range_req->minimum_ap_count = mac_ftm_range_req_ie->uc_minimum_ap_count;
    subelements = mac_ftm_range_req_ie->auc_ftm_range_subelements;
    len_left = meas_req_ie->uc_len - 6;

    if ((meas_req_ie->uc_len < 6) || (ftm_range_req->minimum_ap_count > MAX_MINIMUN_AP_COUNT)) {
        oam_error_log2(hmac_vap_sta->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{AP count[%d], delay[%d]}",
                       ftm_range_req->minimum_ap_count, ftm_range_req->start_delay);
        return;
    }

    /* 从Subelements中获取Maximum Age和Neighbor Report  */
    /* Subelement ID   Name             Extensible
      0–3            Reserved
      4               Maximum Age      Yes
      5-51            Reserved
      52              Neighbor Report  Subelements
      53–220         Reserved
      221             Vendor Specific
      222–255        Reserved */
    /******************************************************************************************************************/
    /*                   Neighbor Report element                                                                      */
    /* -------------------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |BSSID |BSSID Information|Operating Class |Channel Number |PHY Type |Optional Subelements| */
    /* -------------------------------------------------------------------------------------------------------------- */
    /* |1          |1      |6     |4                 |1               |1             |1         |variable           | */
    /* -------------------------------------------------------------------------------------------------------------- */
    /*                                                                                                                */
    /******************************************************************************************************************/
    for (ap_cnt = 0; ap_cnt < ftm_range_req->minimum_ap_count; ap_cnt++) {
        ie = mac_find_ie(MAC_EID_NEIGHBOR_REPORT, subelements, len_left);
        if (ie != NULL && ie[1] == (sizeof(mac_neighbor_rpt_ie_stru) - MAC_IE_HDR_LEN)) {
            neighbor_rpt_ie = (mac_neighbor_rpt_ie_stru *)ie;

            oal_set_mac_addr(ftm_range_req->bssid[ap_index], neighbor_rpt_ie->auc_bssid);

            ftm_range_req->channel[ap_index] = neighbor_rpt_ie->uc_channum;

            subelements += neighbor_rpt_ie->uc_len + MAC_IE_HDR_LEN;
            len_left -= neighbor_rpt_ie->uc_len + MAC_IE_HDR_LEN;
            ap_index++;
        }
    }

    memset_s(&(ftm_info->ftm_range_rpt), sizeof(ftm_range_rpt_stru), 0, sizeof(ftm_range_rpt_stru));
    ftm_range_req->minimum_ap_count = ap_index;
    ftm_range_req->meas_done = 0;

    /* 200 is ftm_range_req->start_delay */
    frw_timer_create_timer_m(&(ftm_range_req->ftm_range_req_timer), hmac_ftm_range_request_timeout, 200,
        (void *)hmac_vap_sta, OAL_FALSE, OAM_MODULE_ID_HMAC, hmac_vap_sta->st_vap_base_info.core_id);
}

static uint32_t hmac_config_ftm_debug_cmd(mac_vap_stru *mac_vap, hmac_vap_stru *hmac_vap, hmac_ftm_stru *ftm_info,
    hal_host_device_stru *hal_device, mac_ftm_debug_switch_stru *ftm_debug)
{
    if (ftm_debug->cmd_bit_map & BIT1) {
        /* 发送initial ftm request命令 */
        hmac_ftm_print_info(mac_vap, ftm_debug);
        if (hmac_ftm_send_iftmr_info(mac_vap, ftm_debug) != OAL_SUCC) {
            return OAL_FAIL;
        }
        hmac_ftm_initiator_initial_request(hmac_vap, &ftm_debug->st_send_iftmr_bit1);
    } else if (ftm_debug->cmd_bit_map & BIT16) {
        /* gas init req,需要访问扫描列表，因此在host侧实现，消息不用抛到dmac */
        return hmac_ftm_send_gas_init_req(mac_vap, &ftm_debug->st_gas_init_req_bit16);
    } else if (ftm_debug->cmd_bit_map & BIT2) {
        /* ftm enable命令 */
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_FTM, "{hmac_config_ftm_dbg_1106::set ftm enable[%d].}",
                         ftm_debug->en_enable_bit2);
        hal_host_ftm_set_enable(hal_device, ftm_debug->en_enable_bit2);
    } else if (ftm_debug->cmd_bit_map & BIT3) {
        /* ftm cali命令 */
        hmac_ftm_cali_enable(mac_vap, ftm_info, ftm_debug->en_cali_bit3);
    } else if (ftm_debug->cmd_bit_map & BIT4) {
        /* send_ftm -- 发送ftm命令 */
        hmac_ftm_responder_dbg_send(hmac_vap, ftm_debug->st_send_ftm_bit4.auc_mac);
    } else if (ftm_debug->cmd_bit_map & BIT6) {
        /* 设置校准时间 */
        hmac_ftm_set_air_overhead(mac_vap, ftm_info, ftm_debug->st_ftm_time_bit6.ftm_correct_time3);
    } else if (ftm_debug->cmd_bit_map & BIT9) {
        /* get_cali --- 获取ftm校准值 */
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_FTM, "{hmac_config_ftm_dbg_1106::cali_time = [%d].}",
                         ftm_info->ftm_cali_time);
    } else if (ftm_debug->cmd_bit_map & BIT11) {
        /* set_ftm_m2s -- 设置ftm PHY接收通路 */
        hmac_ftm_set_m2s(mac_vap, hal_device, ftm_info, ftm_debug->ftm_chain_selection);
    } else if (ftm_debug->cmd_bit_map & BIT12) {
        /* en_multipath -- 使能多径修正 */
        hmac_ftm_set_multipath_on(ftm_info, ftm_debug->en_multipath_bit12, (ftm_debug->cmd_bit_map & BIT12));
    } else if (ftm_debug->cmd_bit_map & BIT13) {
        /* set_cali --- 设置ftm校准值 */
        hmac_ftm_set_cali(mac_vap, ftm_info, ftm_debug->ftm_cali);
    }  else if (ftm_debug->cmd_bit_map & BIT14) {
        /* set_ftm_timeout */
        hmac_ftm_set_tsf_fix(ftm_info, &ftm_debug->st_ftm_timeout_bit14, (ftm_debug->cmd_bit_map & BIT14));
    } else if (ftm_debug->cmd_bit_map & BIT15) {
        /* send_nbr_rpt_req --- 发送邻居报告请求 */
        hmac_ftm_tx_neighbour_report_request(mac_vap, &ftm_debug->st_neighbor_report_req_bit15);
    }
    return OAL_SUCC;
}

uint32_t hmac_config_ftm_dbg_1106(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    mac_ftm_debug_switch_stru *ftm_debug = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_ftm_stru *ftm_info = NULL;
    hal_host_device_stru *hal_device = NULL;
    ftm_debug = (mac_ftm_debug_switch_stru *)param;

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return OAL_FAIL;
    }
    hal_device = hal_get_host_device(hmac_vap->hal_dev_id);
    ftm_info = (hmac_ftm_stru *)hmac_vap->feature_ftm;
    if (oal_any_null_ptr3(ftm_info, hal_device, ftm_debug)) {
        return OAL_FAIL;
    }
    if (hmac_config_ftm_debug_cmd(mac_vap, hmac_vap, ftm_info, hal_device, ftm_debug) == OAL_FAIL) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

oal_bool_enum_uint8 hmac_rrm_proc_rm_request_1106(hmac_vap_stru *hmac_vap_sta, oal_netbuf_stru *netbuf)
{
    oal_bool_enum_uint8 ret = OAL_FALSE;
    mac_meas_req_ie_stru *meas_req_ie = NULL;
    mac_rx_ctl_stru  *rx_info = (mac_rx_ctl_stru *)oal_netbuf_cb((netbuf));
    uint8_t *payload = mac_netbuf_get_payload(netbuf);
    uint16_t framebody_len = MAC_GET_RX_CB_PAYLOAD_LEN(rx_info);

    uint16_t framebody_len_left = framebody_len - MAC_MEASUREMENT_REQUEST_IE_OFFSET;
    while (framebody_len_left > MAC_IE_HDR_LEN) {
        meas_req_ie = (mac_meas_req_ie_stru *)&payload[framebody_len - framebody_len_left];
        if ((framebody_len_left < meas_req_ie->uc_len) ||
            (meas_req_ie->uc_len < (OAL_OFFSET_OF(mac_meas_req_ie_stru, auc_meas_req) - MAC_IE_HDR_LEN))) {
            oam_warning_log2(hmac_vap_sta->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{invalid Measurement_ie_len= [%d]  \
                framebody_len_tmp=[%d]!}", meas_req_ie->uc_len, framebody_len_left);
            break;
        }

        framebody_len_left -= meas_req_ie->uc_len;
        if (meas_req_ie->uc_eid == MAC_EID_MEASREQ) {
            /* Req中不允许发送对应的report */
            if ((meas_req_ie->st_reqmode.bit_enable == 1) && (meas_req_ie->st_reqmode.bit_rpt == 0)) {
                oam_warning_log0(hmac_vap_sta->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{hmac_rrm_proc_rm_request:: \
                    srpt now allowed!}");
                continue;
            }
            /* 处理FTM Range req */
            if (meas_req_ie->uc_reqtype == RM_RADIO_MEASUREMENT_FTM_RANGE) {
                hmac_rrm_parse_ftm_range_req(hmac_vap_sta, meas_req_ie);
                ret = OAL_TRUE;
            }
        } else {
            /* MR IE错误，不回，报错 */
            oam_warning_log1(hmac_vap_sta->st_vap_base_info.uc_vap_id, OAM_SF_RRM, "{hmac_rrm_proc_rm_request:: \
                Error Request, EID[%d]!}", meas_req_ie->uc_eid);
        }
    }
    return ret;
}
#endif
