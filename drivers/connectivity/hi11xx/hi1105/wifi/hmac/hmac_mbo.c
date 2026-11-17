

#include "oam_ext_if.h"
#include "mac_frame.h"
#include "mac_frame_common.h"
#include "hmac_config.h"
#include "mac_vap.h"
#include "mac_device.h"
#include "securec.h"
#include "hmac_mbo.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MBO_C

#ifdef _PRE_WLAN_FEATURE_MBO

OAL_STATIC void mac_set_mbo_cell_para(mac_vap_stru *pst_mac_vap, uint8_t *puc_buffer,
    uint8_t *puc_ie_len, uint8_t uc_mbo_cell_capa)
{
    mac_ieee80211_vendor_ie_stru *pst_vendor_ie = NULL;

    pst_mac_vap->st_mbo_para_info.uc_mbo_cell_capa = uc_mbo_cell_capa;

    pst_vendor_ie = (mac_ieee80211_vendor_ie_stru *)puc_buffer;
    /* 设置MBO元素的OUI和OUI type = 0x16 */
    pst_vendor_ie->uc_element_id = MAC_EID_VENDOR;
    pst_vendor_ie->uc_len = 7; /* ie长度为7 */
    pst_vendor_ie->auc_oui[0] = (uint8_t)((MAC_MBO_VENDOR_IE >> BIT_OFFSET_16) & 0xff);
    pst_vendor_ie->auc_oui[1] = (uint8_t)((MAC_MBO_VENDOR_IE >> BIT_OFFSET_8) & 0xff);
    pst_vendor_ie->auc_oui[BYTE_OFFSET_2] = (uint8_t)((MAC_MBO_VENDOR_IE)&0xff);
    pst_vendor_ie->uc_oui_type = MAC_MBO_IE_OUI_TYPE;

    puc_buffer[BYTE_OFFSET_6] = MBO_ATTR_ID_CELL_DATA_CAPA;
    puc_buffer[BYTE_OFFSET_7] = 1;
    puc_buffer[BYTE_OFFSET_8] = uc_mbo_cell_capa;

    *puc_ie_len = pst_vendor_ie->uc_len + MAC_IE_HDR_LEN;
}


void mac_set_mbo_ie(mac_vap_stru *pst_mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len)
{
    /***************************************************************************
                         --------------------------------------------------
                         |Element ID |Length |OUI|OUI Type|MBO Attributes |
                         --------------------------------------------------
          Octets:        |1          |1      |3  |1       |n              |
                         --------------------------------------------------
    ***************************************************************************/
    uint8_t uc_mbo_ie_len = 0;

    /* 若MBO定制化开关未打开，不带MBO IE 函数return */
    if (pst_mac_vap->st_mbo_para_info.uc_mbo_enable != OAL_TRUE) {
        *puc_ie_len = 0;
        return;
    } else {
        mac_set_mbo_cell_para(pst_mac_vap, puc_buffer, &uc_mbo_ie_len,
                              pst_mac_vap->st_mbo_para_info.uc_mbo_cell_capa);
        *puc_ie_len = uc_mbo_ie_len;
    }
}


void mac_set_mbo_ie_ap(mac_vap_stru *pst_mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len)
{
    /***************************************************************************
                         --------------------------------------------------
                         |Element ID |Length |OUI|OUI Type|MBO Attributes |
                         --------------------------------------------------
          Octets:        |1          |1      |3  |1       |n              |
                         --------------------------------------------------
    ***************************************************************************/
    mac_ieee80211_vendor_ie_stru *pst_vendor_ie = NULL;

    /* 若MBO定制化开关未打开，不带MBO IE或测试开关未开 函数return */
    if ((pst_mac_vap->st_mbo_para_info.uc_mbo_enable != OAL_TRUE) ||
        (pst_mac_vap->st_mbo_para_info.uc_mbo_assoc_disallowed_test_switch != OAL_TRUE)) {
        *puc_ie_len = 0;
        return;
    } else {
        pst_vendor_ie = (mac_ieee80211_vendor_ie_stru *)puc_buffer;
        /* 设置MBO元素的OUI和OUI type = 0x16 */
        pst_vendor_ie->uc_element_id = MAC_EID_VENDOR;
        pst_vendor_ie->uc_len = 7; /* ie长度为7 */
        pst_vendor_ie->auc_oui[0] = (uint8_t)((MAC_MBO_VENDOR_IE >> BIT_OFFSET_16) & 0xff);
        pst_vendor_ie->auc_oui[1] = (uint8_t)((MAC_MBO_VENDOR_IE >> BIT_OFFSET_8) & 0xff);
        pst_vendor_ie->auc_oui[BYTE_OFFSET_2] = (uint8_t)((MAC_MBO_VENDOR_IE)&0xff);
        pst_vendor_ie->uc_oui_type = MAC_MBO_IE_OUI_TYPE;

        puc_buffer[BYTE_OFFSET_6] = MBO_ATTR_ID_ASSOC_DISALLOW;
        puc_buffer[BYTE_OFFSET_7] = 1;   /* length */
        puc_buffer[BYTE_OFFSET_8] = 0x0; /* reason code */
        *puc_ie_len = pst_vendor_ie->uc_len + MAC_IE_HDR_LEN;
    }
}


void mac_set_supported_operating_classes_ie(mac_vap_stru *pst_mac_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len)
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
    /* 若MBO 定制化开关未打开，不带Supported Operating Classed IE */
    if (pst_mac_vap->st_mbo_para_info.uc_mbo_enable != OAL_TRUE) {
        *puc_ie_len = 0;
        return;
    } else {
        puc_buffer[0] = MAC_EID_OPERATING_CLASS;
        puc_buffer[1] = 3; /* length为3 */
        if (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) {
            puc_buffer[BYTE_OFFSET_2] = MAC_GLOBAL_OPERATING_CLASS_NUMBER81;
        }
        if (pst_mac_vap->st_channel.en_band == WLAN_BAND_5G) {
            puc_buffer[BYTE_OFFSET_2] = MAC_GLOBAL_OPERATING_CLASS_NUMBER115;
        }
        puc_buffer[BYTE_OFFSET_3] = MAC_GLOBAL_OPERATING_CLASS_NUMBER81;
        puc_buffer[BYTE_OFFSET_4] = MAC_GLOBAL_OPERATING_CLASS_NUMBER115;

        *puc_ie_len = MAC_IE_HDR_LEN + 3; /* length为3 */
    }
}


uint8_t *mac_find_mbo_attribute(uint8_t uc_eid, uint8_t *puc_ies, int32_t l_len)
{
    if (puc_ies == NULL) {
        return NULL;
    }

    /* 查找MBO Attribute，如果不是直接找下一个 */
    while (l_len > MAC_MBO_ATTRIBUTE_HDR_LEN && puc_ies[0] != uc_eid) {
        l_len -= puc_ies[1] + MAC_MBO_ATTRIBUTE_HDR_LEN;
        puc_ies += puc_ies[1] + MAC_MBO_ATTRIBUTE_HDR_LEN;
    }

    /* 查找到MBO Attribute，剩余长度不匹配直接返回空指针 */
    if ((l_len < MAC_MBO_ATTRIBUTE_HDR_LEN) || (l_len < (MAC_MBO_ATTRIBUTE_HDR_LEN + puc_ies[1]))) {
        return NULL;
    }

    return puc_ies;
}

void hmac_handle_ie_specific_mbo(uint8_t *puc_ie_data,
    hmac_neighbor_bss_info_stru *pst_bss_list_alloc, uint8_t uc_bss_list_index)
{
    if (EOK != memcpy_s(pst_bss_list_alloc[uc_bss_list_index].st_assoc_delay_attr_mbo_ie.auc_oui,
                        sizeof(pst_bss_list_alloc[uc_bss_list_index].st_assoc_delay_attr_mbo_ie.auc_oui),
                        puc_ie_data + MAC_IE_HDR_LEN, MAC_MBO_OUI_LENGTH)) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_handle_ie_by_data_length::memcpy oui fail!");
        return;
    }
    if ((puc_ie_data[BYTE_OFFSET_5] == MAC_MBO_IE_OUI_TYPE) &&
        (puc_ie_data[BYTE_OFFSET_6] == MBO_ATTR_ID_ASSOC_RETRY_DELAY)) {
        pst_bss_list_alloc[uc_bss_list_index].st_assoc_delay_attr_mbo_ie.re_assoc_delay_time =
           (((uint16_t)puc_ie_data[BYTE_OFFSET_9]) << BIT_OFFSET_8) | (puc_ie_data[BYTE_OFFSET_8]);
    }

    return;
}


uint32_t hmac_mbo_check_is_assoc_or_re_assoc_allowed(mac_vap_stru *pst_mac_vap,
    hmac_conn_param_stru *pst_connect_param, mac_bss_dscr_stru *pst_bss_dscr)
{
    uint32_t       current_time;

    /* MBO开关未打开，return true */
    if (pst_mac_vap->st_mbo_para_info.uc_mbo_enable == OAL_FALSE) {
        return OAL_TRUE;
    }

    /* 先判断此bss是不是含有不允许关联MBO IE的BSS，如果是则返回false */
    if (pst_bss_dscr->uc_bss_assoc_disallowed == OAL_TRUE) {
        oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_mbo_check_is_assoc_or_re_assoc_allowed::bssid:%02X:XX:XX:%02X:%02X:%02X mbo assoc disallowed}",
            pst_connect_param->auc_bssid[MAC_ADDR_0], pst_connect_param->auc_bssid[MAC_ADDR_3],
            pst_connect_param->auc_bssid[MAC_ADDR_4], pst_connect_param->auc_bssid[MAC_ADDR_5]);
        return OAL_FALSE;
    }

    /* re-assoc delay不允许关联标标记为0，return true */
    if (pst_mac_vap->st_mbo_para_info.en_disable_connect == OAL_FALSE) {
        return OAL_TRUE;
    }

    /* 判断此BSS是否是re-assoc delay bss，如果是进行相关处理 */
    if (!oal_memcmp(pst_mac_vap->st_mbo_para_info.auc_re_assoc_delay_bss_mac_addr, 
        pst_connect_param->auc_bssid, WLAN_MAC_ADDR_LEN)) {
        current_time = (uint32_t)oal_time_get_stamp_ms(); /* 获取当前时间 */
        /* 在收到btm req帧的ul_re_assoc_delay_time内不允许给该bss发重关联请求帧 */
        if ((current_time - pst_mac_vap->st_mbo_para_info.btm_req_received_time) <=
             pst_mac_vap->st_mbo_para_info.re_assoc_delay_time) {
            oam_warning_log3(0, OAM_SF_CFG, "hmac_mbo_check_is_assoc_or_re_assoc_allowed::current time[%u] < \
                             (btm recv time[%u] + re-assoc_delay time[%d]) not timed out!bss connect disallowed!",
                             current_time, pst_mac_vap->st_mbo_para_info.btm_req_received_time,
                             pst_mac_vap->st_mbo_para_info.re_assoc_delay_time);
            return OAL_FALSE;
        }

        /* 已超时，不允许关联标志位清零 */
        pst_mac_vap->st_mbo_para_info.en_disable_connect = OAL_FALSE;
    }

    return OAL_TRUE;
}


void hmac_scan_update_bss_assoc_disallowed_attr(mac_bss_dscr_stru *pst_bss_dscr,
                                                uint8_t *puc_frame_body,
                                                uint16_t us_frame_len)
{
    uint8_t *puc_mbo_ie = NULL;
    uint8_t *puc_mbo_attribute = NULL;

    puc_mbo_ie = mac_find_vendor_ie(MAC_MBO_VENDOR_IE, MAC_MBO_IE_OUI_TYPE, puc_frame_body, us_frame_len);
    if (puc_mbo_ie == NULL) {
        pst_bss_dscr->uc_bss_assoc_disallowed = OAL_FALSE;
        return;
    }

    puc_mbo_attribute = mac_find_mbo_attribute(MBO_ATTR_ID_ASSOC_DISALLOW,
                                               puc_mbo_ie + MBO_IE_HEADER,
                                               MAC_MBO_ASSOC_DIS_ATTR_LEN);
    if (puc_mbo_attribute == NULL) {
        pst_bss_dscr->uc_bss_assoc_disallowed = OAL_FALSE; /* 不带有Association Disallowed Attribute，允许关联 */
    } else {
        oam_warning_log0(0, OAM_SF_ASSOC, "{hmac_scan_update_bss_assoc_disallowed_attr::Assoc Disallowed Attr found!}");
        pst_bss_dscr->uc_bss_assoc_disallowed = OAL_TRUE; /* 带有Association Disallowed Attribute,AP 不允许被关联 */
    }

    return;
}

#endif

