

/* 1 头文件包含 */
#include "hmac_device.h"
#include "hmac_ftm.h"
#include "hmac_resource.h"
#include "hmac_mgmt_sta.h"
#include "hmac_sme_sta.h"
#include "hmac_fsm.h"
#include "hmac_scan.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_FTM_C

#ifdef _PRE_WLAN_FEATURE_FTM
typedef enum {
    ANQP_INFO_ID_RESERVED = 255,
    ANQP_INFO_ID_QUERY_LIST = 256,
    ANQP_INFO_ID_AP_GEO_LOC = 265,
    ANQP_INFO_ID_AP_CIVIC_LOC = 266,
    ANQP_INFO_ID_BUTT,
} anqp_info_id_enum;

typedef struct {
    oal_uint8 uc_dialog_token;
    oal_uint8 fragment_id;
    oal_uint8 adv_protocol_ele;
    oal_uint16 query_resp_len;
    oal_uint16 comeback_delay;
    oal_uint16 status_code;
} gas_init_resp_stru;


OAL_STATIC oal_uint16 hmac_encap_query_list(oal_uint8 *puc_buffer, mac_send_gas_init_req_stru *pst_gas_req)
{
    oal_uint16 us_index = 0;
    oal_uint16 *ptemp = NULL;

    /* AP支持AP_GEO_LOC， EC_IE_bit15 */
    if (pst_gas_req->en_geo_enable) {
        ptemp = (oal_uint16 *)&puc_buffer[us_index]; /* AP Geospatial Location */
        *ptemp = ANQP_INFO_ID_AP_GEO_LOC;
        us_index += 2; /* index加2，最后返回index的值Informatino的长度 */
    }

    /* AP支持AP_CIVIC_LOC， EC_IE_BIT14 */
    if (pst_gas_req->en_civic_enable) {
        ptemp = (oal_uint16 *)&puc_buffer[us_index]; /* AP Civic Location */
        *ptemp = ANQP_INFO_ID_AP_CIVIC_LOC;
        us_index += 2; /* index加2，最后返回index的值Informatino的长度 */
    }

    return us_index;
}


OAL_STATIC oal_uint16 hmac_encap_anqp_query(oal_uint8 *puc_buffer, mac_send_gas_init_req_stru *pst_gas_req)
{
    oal_uint16 us_index = 0;
    oal_uint16 *pus_query = NULL;

    /* Query Request - ANQP elements */
    /******************************/
    /* |Info ID|Length|Informatino| */
    /* ---------------------------- */
    /* |2      |2     |variable   | */
    /******************************/
    pus_query = (oal_uint16 *)&puc_buffer[us_index]; /* Info ID - Query List, Table 9-271 */
    *pus_query = ANQP_INFO_ID_QUERY_LIST;
    us_index += 2; /* 跳过Info ID(2byte) */

    pus_query = (oal_uint16 *)&puc_buffer[us_index]; /* Length(2byte), 先跳过 */
    us_index += 2;

    *pus_query = hmac_encap_query_list(&puc_buffer[us_index], pst_gas_req);

    us_index += *pus_query;

    return us_index;
}


OAL_STATIC mac_bss_dscr_stru *hmac_ftm_check_target_bss(mac_vap_stru *pst_mac_vap,
                                                        mac_send_gas_init_req_stru *pst_gas_req)
{
    hmac_bss_mgmt_stru *pst_bss_mgmt;
    oal_dlist_head_stru *pst_entry;
    hmac_scanned_bss_info *pst_scanned_bss;
    mac_bss_dscr_stru *pst_bss_dscr = OAL_PTR_NULL;
    mac_bss_dscr_stru *pst_sel_dscr = OAL_PTR_NULL;

    /* 获取hmac device */
    hmac_device_stru *pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_ftm_check_target_bss::device null!}");
        return OAL_PTR_NULL;
    }

    pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    /* 获取锁 */
    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    /* 遍历扫描到的bss信息，查找目标bss */
    oal_dlist_search_for_each(pst_entry, &(pst_bss_mgmt->st_bss_list_head))
    {
        pst_scanned_bss = oal_dlist_get_entry(pst_entry, hmac_scanned_bss_info, st_dlist_head);
        pst_bss_dscr = &(pst_scanned_bss->st_bss_dscr_info);

        if (!oal_compare_mac_addr(pst_gas_req->auc_bssid, pst_bss_dscr->auc_bssid)) {
            pst_sel_dscr = pst_bss_dscr;
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_ftm_check_target_bss::FTM Certificate, Find AP device}");
        }

        pst_bss_dscr = OAL_PTR_NULL;
    }

    /* 解除锁 */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));

    return pst_sel_dscr;
}


OAL_STATIC oal_uint16 hmac_encap_gas_init_req(mac_vap_stru *pst_mac_vap,
    oal_netbuf_stru *pst_buffer, mac_send_gas_init_req_stru *pst_gas_req)
{
    oal_uint8 *puc_payload_addr = get_netbuf_payload(pst_buffer);
    oal_uint16 *pus_gas_init_req;
    oal_uint16 us_index;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_encap_gas_init_req::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_hdr_set_frame_control(puc_payload_addr, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    mac_hdr_set_fragment_number(puc_payload_addr, 0);

    oal_set_mac_addr(puc_payload_addr + WLAN_HDR_ADDR1_OFFSET, pst_gas_req->auc_bssid);

    oal_set_mac_addr(puc_payload_addr + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(pst_mac_vap));

    oal_set_mac_addr(puc_payload_addr + WLAN_HDR_ADDR3_OFFSET, pst_gas_req->auc_bssid);

    /*************************************************************************************/
    /* GAS Initial Request  frame - Frame Body */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Public Action |Dialog Token  | Advertisement Protocol element | */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1             |1             |Variable                        | */
    /* --------------------------------------------------------------------------------- */
    /* |Query Request length|Query Request|Multi-band (optional) */
    /* --------------------------------------------------------------------------------- */
    /* |2                   |variable     | */
    /* --------------------------------------------------------------------------------- */
    /*************************************************************************************/
    us_index = MAC_80211_FRAME_LEN;

    puc_payload_addr[us_index++] = MAC_ACTION_CATEGORY_PUBLIC;                    /* Category */
    puc_payload_addr[us_index++] = MAC_PUB_GAS_INIT_REQ;                          /* Public Action */
    puc_payload_addr[us_index++] = pst_hmac_vap->st_gas_mgmt.uc_gas_dialog_token; /* Dialog Token */

    /*****************************************************************************************************/
    /* Advertisement Protocol element */
    /* ------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |Advertisement Protocol Tuple #1| ... |Advertisement Protocol Tuple #n(optional) | */
    /* ------------------------------------------------------------------------------------------------- */
    /* |1          |1      | variable                      |     |variable                             | */
    /* ------------------------------------------------------------------------------------------------- */
    /*****************************************************************************************************/
    puc_payload_addr[us_index++] = MAC_EID_ADVERTISEMENT_PROTOCOL; /* Element ID */
    puc_payload_addr[us_index++] = 2;                              /* Length is 2 */
    puc_payload_addr[us_index++] = 0;                              /* Query Response Info */
    puc_payload_addr[us_index++] = 0;                              /* Advertisement Protocol ID, 0: ANQP */

    /* Query Request length */
    pus_gas_init_req = (oal_uint16 *)(&puc_payload_addr[us_index]);
    us_index += 2; /* Element ID和Length(2byte) */
    oam_warning_log0(0, OAM_SF_CFG, "{hmac_encap_gas_init_req::encap gas init req.}");

    *pus_gas_init_req = hmac_encap_anqp_query(&puc_payload_addr[us_index], pst_gas_req);
    us_index += *pus_gas_init_req;
    return us_index;
}


oal_uint32 hmac_ftm_tx_gas_init_req(oal_netbuf_stru *pst_gas_init_req_frame, mac_vap_stru *pst_mac_vap,
    hmac_vap_stru *pst_hmac_vap, mac_send_gas_init_req_stru *pst_gas_req)
{
    mac_tx_ctl_stru *pst_tx_ctl;
    oal_uint32 ul_len;
    oal_uint32 ul_ret;

    /* 组装gas init req帧 */
    pst_gas_init_req_frame = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_gas_init_req_frame == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_mem_netbuf_trace(pst_gas_init_req_frame, OAL_TRUE);

    memset_s(oal_netbuf_cb(pst_gas_init_req_frame), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    /* 将mac header清零 */
    memset_s((oal_uint8 *)oal_netbuf_header(pst_gas_init_req_frame), MAC_80211_FRAME_LEN, 0, MAC_80211_FRAME_LEN);

    ul_len = hmac_encap_gas_init_req(pst_mac_vap, pst_gas_init_req_frame, pst_gas_req);

    oal_netbuf_put(pst_gas_init_req_frame, ul_len);

    /* 为填写发送描述符准备参数 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_gas_init_req_frame);
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = (oal_uint16)ul_len;

    /* 抛事件让dmac将该帧发送 */
    ul_ret = hmac_tx_mgmt_send_event(pst_mac_vap, pst_gas_init_req_frame, (oal_uint16)ul_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(pst_gas_init_req_frame);
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_ftm_tx_gas_init_req::hmac_tx_mgmt_send_event failed[%d].}", ul_ret);
        return ul_ret;
    }

    /* 设置gas request frame sent标志位 */
    pst_hmac_vap->st_gas_mgmt.uc_is_gas_request_sent = OAL_TRUE;
    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_ftm_tx_gas_init_req::gas sent[%d].}",
                     pst_hmac_vap->st_gas_mgmt.uc_is_gas_request_sent);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_ftm_send_gas_init_req(mac_vap_stru *pst_mac_vap,
                                                 mac_send_gas_init_req_stru *pst_gas_req)
{
    oal_uint32 ul_ret;
    oal_netbuf_stru *pst_gas_init_req_frame = OAL_PTR_NULL;
    oal_uint8 *puc_ec_ie = OAL_PTR_NULL;
    mac_bss_dscr_stru *pst_sel_dscr = OAL_PTR_NULL;
    hmac_join_req_stru st_join_req;
    hmac_vap_stru *pst_hmac_vap;
    hmac_join_rsp_stru st_join_rsp;

    pst_sel_dscr = hmac_ftm_check_target_bss(pst_mac_vap, pst_gas_req);
    if (pst_sel_dscr == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_ftm_send_gas_init_req:: Can't find target ap.}");
        return OAL_SUCC;
    }

    /* 寻找EC_IE信元 */
    puc_ec_ie = mac_find_ie(MAC_EID_EXT_CAPS, pst_sel_dscr->auc_mgmt_buff + MAC_80211_FRAME_LEN + MAC_SSID_OFFSET,
                            pst_sel_dscr->ul_mgmt_len - MAC_80211_FRAME_LEN - MAC_SSID_OFFSET);
    if (puc_ec_ie == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_ftm_send_gas_init_req:: Can't find EC_IE}");
        return OAL_FAIL;
    }

    /* 获取相关能力，BIT14， BIT15，BIT31， 先跳过ID与len字段 */
    pst_gas_req->en_interworking_enable = (puc_ec_ie[5] & 0x80) >> 7;  /* en_interworking_enable是puc_ec_ie[5]第7bit */
    pst_gas_req->en_civic_enable = (puc_ec_ie[3] & 0x40) >> 6; /* en_civic_enable是puc_ec_ie[3]第6bit */
    pst_gas_req->en_geo_enable = (puc_ec_ie[3] & 0x80) >> 7; /* en_geo_enable是puc_ec_ie[3]第7bit */

    oam_warning_log3(0, OAM_SF_CFG, "{hmac_ftm_send_gas_init_req:: interworking=%d, civic_enable=%d, geo_enable=%d}",
                     pst_gas_req->en_interworking_enable, pst_gas_req->en_civic_enable, pst_gas_req->en_geo_enable);

    if (!pst_gas_req->en_interworking_enable) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_ftm_send_gas_init_req:: AP does not support interworking!}");
        return OAL_SUCC;
    }

    /* 配置join参数 */
    hmac_prepare_join_req(&st_join_req, pst_sel_dscr);

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_sta_update_join_req_params(pst_hmac_vap, &st_join_req);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_ftm_send_gas_init_req::hmac_sta_update_join_req_params fail[%d].}", ul_ret);
        return ul_ret;
    }

    st_join_rsp.en_result_code = HMAC_MGMT_SUCCESS;

    /* 切换STA状态到JOIN_COMP,与罗令志沟通,驱动只需实现未关联时发包即可 */
    hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_STA_JOIN_COMP);

    /* 组装gas init req帧并发送 */
    ul_ret = hmac_ftm_tx_gas_init_req(pst_gas_init_req_frame, pst_mac_vap, pst_hmac_vap, pst_gas_req);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_ftm_send_gas_init_req::encape gas init req frame fail[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint16 hmac_encap_gas_comeback_req(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_buffer)
{
    oal_uint8 *puc_payload_addr = get_netbuf_payload(pst_buffer);
    oal_uint16 us_index;
    mac_vap_stru *pst_mac_vap = &pst_hmac_vap->st_vap_base_info;

    mac_hdr_set_frame_control(puc_payload_addr, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    mac_hdr_set_fragment_number(puc_payload_addr, 0);

    oal_set_mac_addr(puc_payload_addr + WLAN_HDR_ADDR1_OFFSET, pst_mac_vap->auc_bssid);

    oal_set_mac_addr(puc_payload_addr + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(pst_mac_vap));

    oal_set_mac_addr(puc_payload_addr + WLAN_HDR_ADDR3_OFFSET, pst_mac_vap->auc_bssid);

    /*************************************************************************************/
    /* GAS Comeback Request  frame - Frame Body */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Public Action |Dialog Token  | Multi-band (optional)  | */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1             |1             |Variable                        | */
    /* --------------------------------------------------------------------------------- */
    /*************************************************************************************/
    us_index = MAC_80211_FRAME_LEN;

    puc_payload_addr[us_index++] = MAC_ACTION_CATEGORY_PUBLIC;                             /* Category */
    puc_payload_addr[us_index++] = MAC_PUB_GAS_COMBAK_REQ;                                 /* Public Action */
    puc_payload_addr[us_index++] = pst_hmac_vap->st_gas_mgmt.uc_gas_response_dialog_token; /* Dialog Token */

    OAM_WARNING_LOG1(0, OAM_SF_FTM,
                     "{hmac_encap_gas_comeback_req::dialog token = %d",
                     pst_hmac_vap->st_gas_mgmt.uc_gas_response_dialog_token);

    return us_index;
}


OAL_STATIC oal_uint32 hmac_ftm_send_gas_comeback_req(hmac_vap_stru *pst_hmac_vap)
{
    oal_netbuf_stru *pst_netbuf;
    mac_tx_ctl_stru *pst_tx_ctl;
    oal_uint32 ul_ret;
    oal_uint32 ul_len;
    mac_vap_stru *pst_mac_vap = &pst_hmac_vap->st_vap_base_info;
    oal_int32 l_ret;

    /* 组装gas   comeback req帧 */
    pst_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_ftm_send_gas_comeback_req::pst_gas_comeback_req_frame is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_mem_netbuf_trace(pst_netbuf, OAL_TRUE);

    l_ret = memset_s(oal_netbuf_cb(pst_netbuf), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    /* 将mac header清零 */
    l_ret += memset_s((oal_uint8 *)oal_netbuf_header(pst_netbuf), MAC_80211_FRAME_LEN, 0, MAC_80211_FRAME_LEN);
    if (l_ret != EOK) {
        OAM_WARNING_LOG1(0, OAM_SF_FTM, "hmac_ftm_send_gas_comeback_req::memcpy fail![%d]", l_ret);
    }

    ul_len = hmac_encap_gas_comeback_req(pst_hmac_vap, pst_netbuf);

    oal_netbuf_put(pst_netbuf, ul_len);

    /* 为填写发送描述符准备参数 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = (oal_uint16)ul_len;

    /* 抛事件让dmac将该帧发送 */
    ul_ret = hmac_tx_mgmt_send_event(pst_mac_vap, pst_netbuf, (oal_uint16)ul_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_ftm_send_gas_comeback_req::hmac_tx_mgmt_send_event failed[%d].}", ul_ret);
        return ul_ret;
    }
    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_ftm_send_gas_comeback_req::hmac_tx_mgmt_send_event SUCC[%d].}", ul_ret);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_ftm_gas_comeback_timeout(oal_void *p_arg)
{
    hmac_vap_stru *pst_hmac_vap;
    oal_uint32 ul_ret;

    pst_hmac_vap = (hmac_vap_stru *)p_arg;
    if (pst_hmac_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_ftm_send_gas_comeback_req(pst_hmac_vap);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_FTM,
                       "{hmac_ftm_gas_comeback_timeout::ftm send gas comeback req failed[%d].}", ul_ret);
        return ul_ret;
    }
    return OAL_SUCC;
}


OAL_STATIC oal_void hmac_ftm_start_gas_comeback_timer(hmac_vap_stru *pst_hmac_vap,
                                                      gas_init_resp_stru *pst_gas_init_resp)
{
    frw_create_timer(&(pst_hmac_vap->st_ftm_timer),
                     hmac_ftm_gas_comeback_timeout,
                     (oal_uint32)pst_gas_init_resp->comeback_delay,
                     (oal_void *)pst_hmac_vap,
                     OAL_FALSE,
                     OAM_MODULE_ID_HMAC,
                     pst_hmac_vap->st_vap_base_info.ul_core_id);
}


oal_uint32 hmac_ftm_rx_gas_initial_response_frame(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    mac_rx_ctl_stru *pst_rx_info;
    oal_uint8 *puc_data;
    gas_init_resp_stru st_gas_resp = { 0 };
    oal_uint16 us_index = 0;

    if (oal_unlikely((pst_hmac_vap == OAL_PTR_NULL) || (pst_netbuf == OAL_PTR_NULL))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (!pst_hmac_vap->st_gas_mgmt.uc_is_gas_request_sent) {
        oam_warning_log0(0, OAM_SF_RX, "{hmac_ftm_rx_gas_initial_response_frame::it isn't for hmac, forward to wpa.}");
        return OAL_FAIL;
    }
    pst_rx_info = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    puc_data = (oal_uint8 *)(pst_rx_info->pul_mac_hdr_start_addr) + pst_rx_info->uc_mac_header_len;

    us_index += 2;  /* 2表示跳过category、public action字段 */
    st_gas_resp.uc_dialog_token = *(oal_uint8 *)(&puc_data[us_index]);
    if (st_gas_resp.uc_dialog_token != pst_hmac_vap->st_gas_mgmt.uc_gas_dialog_token) {
        oam_warning_log2(0, OAM_SF_RX, "{gas resp dialog token [%d] != hmac gas dialog token [%d].}",
                         st_gas_resp.uc_dialog_token, pst_hmac_vap->st_gas_mgmt.uc_gas_dialog_token);
        return OAL_FAIL;
    }

    us_index += 1;

    st_gas_resp.status_code = *(oal_uint16 *)(&puc_data[us_index]);
    us_index += 2; /* 2表示跳过status code */

    st_gas_resp.comeback_delay = *(oal_uint16 *)(&puc_data[us_index]);

    oam_warning_log4(0, OAM_SF_FTM,
                     "{dialog token=%d, status code=%d, fragment_id=%d, comebakc_delay=%d", st_gas_resp.uc_dialog_token,
                     st_gas_resp.status_code, st_gas_resp.fragment_id, st_gas_resp.comeback_delay);

    pst_hmac_vap->st_gas_mgmt.uc_gas_response_dialog_token = st_gas_resp.uc_dialog_token;

    // 启动comeback定时器
    if (st_gas_resp.comeback_delay > 0) {
        hmac_ftm_start_gas_comeback_timer(pst_hmac_vap, &st_gas_resp);
    }

    pst_hmac_vap->st_gas_mgmt.uc_gas_dialog_token++;
    pst_hmac_vap->st_gas_mgmt.uc_is_gas_request_sent = OAL_FALSE;

    return OAL_SUCC;
}


oal_uint32 hmac_config_ftm_dbg(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    mac_ftm_debug_switch_stru *pst_ftm_debug;
    mac_bss_dscr_stru *pst_bss_dscr = OAL_PTR_NULL;
    mac_send_iftmr_stru *pst_send_iftmr = OAL_PTR_NULL;

    /* 入参合法判断 */
    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        return OAL_FAIL;
    }

    pst_ftm_debug = (mac_ftm_debug_switch_stru *)puc_param;

    /* ftm_initiator命令 */
    if (pst_ftm_debug->ul_cmd_bit_map & BIT0) {
        OAM_WARNING_LOG1(0, OAM_SF_FTM, "{set ftm initiator mode[%d].}", pst_ftm_debug->en_ftm_initiator_bit0);
        mac_mib_set_FineTimingMsmtInitActivated(pst_mac_vap, pst_ftm_debug->en_ftm_initiator_bit0);
    }

    /* 发送iftmr命令 */
    pst_send_iftmr = &(pst_ftm_debug->st_send_iftmr_bit1);
    if (pst_ftm_debug->ul_cmd_bit_map & BIT1) {
        pst_bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(pst_mac_vap, pst_send_iftmr->auc_bssid);
        if (pst_bss_dscr != OAL_PTR_NULL) {
            if (pst_send_iftmr->uc_channel_num != pst_bss_dscr->st_channel.uc_chan_number) {
                oam_warning_log4(0, OAM_SF_FTM, "{send iftmr:AP [**:%x:%x]channel %d, ap's operation channel %d.}",
                                 /* auc_bssid第4、5byte为参数输出打印 */
                                 pst_send_iftmr->auc_bssid[4], pst_send_iftmr->auc_bssid[5],
                                 pst_send_iftmr->uc_channel_num,
                                 pst_bss_dscr->st_channel.uc_chan_number);
                pst_send_iftmr->uc_channel_num = pst_bss_dscr->st_channel.uc_chan_number;
            }
        }
    }

    /* ftm_resp命令 */
    if (pst_ftm_debug->ul_cmd_bit_map & BIT5) {
        mac_mib_set_FineTimingMsmtRespActivated(pst_mac_vap, pst_ftm_debug->en_ftm_resp_bit5);
        // dmac 处理
    }

    /* ftm_range命令 */
    if (pst_ftm_debug->ul_cmd_bit_map & BIT8) {
        OAM_WARNING_LOG1(0, OAM_SF_FTM, "{hmac_config_ftm_dbg::set ftm range report capability enabled field [%d].}",
                         pst_ftm_debug->en_ftm_range_bit8);
        mac_mib_set_FineTimingMsmtRangeRepActivated(pst_mac_vap, pst_ftm_debug->en_ftm_range_bit8);
        // dmac 处理
    }

    /* gas init req,需要访问扫描列表，因此在host侧实现，消息不用抛到dmac */
    if (pst_ftm_debug->ul_cmd_bit_map & BIT16) {
        return hmac_ftm_send_gas_init_req(pst_mac_vap, &pst_ftm_debug->st_gas_init_req_bit16);
    }
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_FTM_DBG, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_config_protocol_debug_switch::send_event fail[%d].", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

oal_uint32 hmac_sta_not_up_rx_public_action(mac_public_action_type_enum_uint8 en_public_action_field,
    hmac_vap_stru *pst_hmac_vap_sta, oal_netbuf_stru *pst_netbuf)
{
    oal_uint32 ul_ret;

    switch (en_public_action_field) {
        case MAC_PUB_GAS_INIT_RESP:
            oam_warning_log0(0, OAM_SF_RX, "{hmac_sta_not_up_rx_public_action::receive gas initial response frame.}");
            ul_ret = hmac_ftm_rx_gas_initial_response_frame(pst_hmac_vap_sta, pst_netbuf);
            if (ul_ret == OAL_FAIL) {
                hmac_rx_mgmt_send_to_host(pst_hmac_vap_sta, pst_netbuf);
            }
            break;

        default:
            oam_warning_log0(0, OAM_SF_RX, "{hmac_sta_not_up_rx_public_action::rx other public action frame }");
            hmac_rx_mgmt_send_to_host(pst_hmac_vap_sta, pst_netbuf);
            break;
    }

    return OAL_SUCC;
}

oal_void hmac_vap_init_gas(hmac_vap_stru *pst_hmac_vap)
{
    pst_hmac_vap->st_gas_mgmt.uc_is_gas_request_sent = OAL_FALSE;
    pst_hmac_vap->st_gas_mgmt.uc_gas_dialog_token = HMAC_GAS_DIALOG_TOKEN_INITIAL_VALUE;
    pst_hmac_vap->st_gas_mgmt.uc_gas_response_dialog_token = HMAC_GAS_DIALOG_TOKEN_INITIAL_VALUE;
}

oal_uint8 hmac_ftm_is_in_scan_list(mac_vap_stru *pst_mac_vap, oal_uint8 uc_band, oal_uint8 uc_channel_number)
{
    if (!mac_check_ftm_enable(pst_mac_vap)) {
        return OAL_TRUE;
    }

    /* For location Use,no channel switch when vap is up */
    if ((pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP) &&
        (pst_mac_vap->st_channel.en_band == uc_band) &&
        (pst_mac_vap->st_channel.uc_chan_number != uc_channel_number)) {
        return OAL_FALSE;
    } else {
        return OAL_TRUE;
    }
}
#endif
