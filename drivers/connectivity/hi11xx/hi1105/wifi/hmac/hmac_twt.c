

#ifdef _PRE_WLAN_FEATURE_TWT

#include "oal_util.h"
#include "hmac_resource.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_hcc_adapt.h"
#include "hmac_twt.h"
#include "securec.h"
#include "mac_mib.h"
#include "mac_frame_inl.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TWT_C

OAL_STATIC void hmac_mgmt_tx_twt_action(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_twt_action_mgmt_args_stru *twt_action_args);

void hmac_individual_twt_init(hmac_vap_stru *hmac_vap, mac_itwt_ie_stru *twt_element,
    mac_twt_action_mgmt_args_stru *twt_action_args)
{
    /*
     * TWT argument
     * -----------------------------------------
     * |setup_cmd |flow_type |flow_ID |Reserve |
     * -----------------------------------------
     * |1         |1         |1       |1       |
     * -----------------------------------------
     */
    twt_element->category = MAC_ACTION_CATEGORY_S1G;
    twt_element->action = MAC_S1G_ACTION_TWT_SETUP;
    twt_element->dialog_token = hmac_vap->st_twt_cfg[0].uc_twt_dialog_token;
    twt_element->element_id = MAC_EID_TWT;
    twt_element->len = sizeof(mac_itwt_ie_stru) - 5; /* 5表示twt头部长度 */
    twt_element->st_control.bit_ndp_paging_indicator = 0;
    twt_element->st_control.bit_responder_pm_mode = 0;
    twt_element->st_control.bit_negotiation = 0;
    twt_element->st_request_type.bit_request = 1; /* 如果支持twt responder，回复setup帧时把该位清除 */
    twt_element->st_request_type.bit_setup_command = twt_action_args->twt_setup_cmd; /* 由user决定 */
    /* 默认trigger-enabled TWT，与认证用例对齐，该位最终由AP决定 */
    twt_element->st_request_type.bit_trigger = 1;
    twt_element->st_request_type.bit_implicit = 1;
    /* 由user决定, annouced or unannounced */
    twt_element->st_request_type.bit_flow_type = twt_action_args->twt_flow_type;
    twt_element->st_request_type.bit_flow_id = twt_action_args->twt_flow_id; /* 由user决定 */
    twt_element->st_request_type.bit_exponent = twt_action_args->twt_exponent; /* unit: microsecond(us) */
    twt_element->st_request_type.bit_protection = 0;
    twt_element->twt = twt_action_args->twt;
    twt_element->min_duration = twt_action_args->twt_duration; /* user decide, unit: 256us */
    twt_element->mantissa = twt_action_args->intrval_mantissa; /* user decide, unit: microsecond(us) */
    twt_element->channel = 0;
}

static uint16_t hmac_mgmt_sta_itwt_setup_encap(hmac_vap_stru *hmac_vap, mac_twt_action_mgmt_args_stru *twt_action_args,
    uint8_t *data)
{
    mac_itwt_ie_stru individual_twt_element;

    memset_s((uint8_t *)&individual_twt_element, sizeof(mac_itwt_ie_stru), 0, sizeof(mac_itwt_ie_stru));

    hmac_individual_twt_init(hmac_vap, &individual_twt_element, twt_action_args);

    oam_warning_log4(0, OAM_SF_11AX, "{hmac_mgmt_encap_twt_setup_req::entry bit_setup_command:%d bit_flow_type:\
        %d bit_flow_id:%d trigger:%d}", individual_twt_element.st_request_type.bit_setup_command,
        individual_twt_element.st_request_type.bit_flow_type, individual_twt_element.st_request_type.bit_flow_id,
        individual_twt_element.st_request_type.bit_trigger);
    oam_warning_log4(0, OAM_SF_11AX,
        "{hmac_mgmt_encap_twt_setup_req::entry ull_twt:%d min_duration:%d us_mantissa:%d, implicit[%d]}",
        individual_twt_element.twt, individual_twt_element.min_duration, individual_twt_element.mantissa,
        individual_twt_element.st_request_type.bit_implicit);

    if (memcpy_s((uint8_t *)(data + MAC_80211_FRAME_LEN), WLAN_MEM_NETBUF_SIZE2 - MAC_80211_FRAME_LEN,
        (uint8_t *)&individual_twt_element, sizeof(mac_itwt_ie_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_11AX, "hmac_mgmt_encap_twt_setup_req::memcpy fail!");
        return MAC_80211_FRAME_LEN;
    }
    return (MAC_80211_FRAME_LEN + sizeof(mac_itwt_ie_stru));
}


uint16_t hmac_mgmt_encap_twt_setup_req(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    uint8_t *data, mac_twt_action_mgmt_args_stru *twt_action_args)
{
    uint16_t index;
    if (oal_any_null_ptr3(hmac_vap, data, twt_action_args)) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_encap_twt_setup_req::hmac_vap or data or twt_action_args is null.}");
        return 0;
    }

    oam_warning_log3(0, OAM_SF_11AX, "{hmac_mgmt_encap_twt_setup_req:: mac_addr[%02x XX XX XX %02x %02x]!.}",
                     hmac_user->st_user_base_info.auc_user_mac_addr[MAC_ADDR_0],
                     hmac_user->st_user_base_info.auc_user_mac_addr[MAC_ADDR_4],
                     hmac_user->st_user_base_info.auc_user_mac_addr[MAC_ADDR_5]);

    /* Set the fields in the frame header */
    /* Frame Control Field 中只需要设置Type/Subtype值，其他设置为0 */
    mac_hdr_set_frame_control(data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* DA is address of STA requesting association */
    oal_set_mac_addr(data + HMAC_80211_FC_AND_DID_LEN, hmac_user->st_user_base_info.auc_user_mac_addr);

    /* SA的值为dot11MACAddress的值 */
    oal_set_mac_addr(data + HMAC_80211_FC_AND_DID_LEN + WLAN_MAC_ADDR_LEN,
        mac_mib_get_StationID(&hmac_vap->st_vap_base_info));

    oal_set_mac_addr(data + HMAC_80211_FC_AND_DID_LEN + WLAN_MAC_ADDR_LEN + WLAN_MAC_ADDR_LEN,
        hmac_vap->st_vap_base_info.auc_bssid);

    /* Set the contents of the frame body */
    /* 将索引指向frame body起始位置 */
    index = MAC_80211_FRAME_LEN;

    /*
     * TWT element
     * --------------------------------------------------------
     * |Element ID |Length |Control |TWT Parameter Information|
     * --------------------------------------------------------
     * |1          |1      |1       |variable                 |
     * --------------------------------------------------------
     */
    if (hmac_vap->st_twt_cfg[0].twt_type == 0) { // 支持单播twt
        return hmac_mgmt_sta_itwt_setup_encap(hmac_vap, twt_action_args, data);
    } else { // 支持广播twt会话条件
        if (memcpy_s((uint8_t *)(data + MAC_80211_FRAME_LEN), WLAN_MEM_NETBUF_SIZE2 - MAC_80211_FRAME_LEN,
            (uint8_t *)twt_action_args, sizeof(mac_btwt_action_stru)) != EOK) {
            oam_error_log0(0, OAM_SF_11AX, "hmac_mgmt_encap_twt_setup_req::memcpy fail!");
            return index;
        }
        return index + sizeof(mac_btwt_action_stru);
    }
}


uint16_t hmac_mgmt_encap_twt_teardown_req(hmac_vap_stru *vap,
                                          hmac_user_stru *hmac_user,
                                          uint8_t *puc_data,
                                          mac_twt_action_mgmt_args_stru *twt_action_args)
{
    uint16_t us_index;
    mac_twt_teardown_stru twt_teardown;
    if (oal_any_null_ptr3(vap, puc_data, twt_action_args)) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_encap_twt_teardown_req::vap or data or twt_action_args is null.}");
        return 0;
    }
    memset_s((uint8_t *)&twt_teardown, sizeof(mac_twt_teardown_stru), 0, sizeof(mac_twt_teardown_stru));
    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* Frame Control Field 中只需要设置Type/Subtype值，其他设置为0 */
    mac_hdr_set_frame_control(puc_data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* DA is address of STA requesting association */
    oal_set_mac_addr(puc_data + 4, hmac_user->st_user_base_info.auc_user_mac_addr); // 前4字节为DA前的字段

    /* SA的值为dot11MACAddress的值 */
    oal_set_mac_addr(puc_data + 10, mac_mib_get_StationID(&vap->st_vap_base_info)); // 前10字节为SA前的字段

    oal_set_mac_addr(puc_data + 16, vap->st_vap_base_info.auc_bssid);              // 前16字节为bssid前的字段

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /* 将索引指向frame body起始位置 */
    us_index = MAC_80211_FRAME_LEN;

    twt_teardown.uc_category = MAC_ACTION_CATEGORY_S1G;
    twt_teardown.uc_action = MAC_S1G_ACTION_TWT_TEARDOWN;
    twt_teardown.bit_twt_flow_id = twt_action_args->twt_flow_id; /* 由user决定 */
    twt_teardown.bit_nego_type = twt_action_args->twt_flow_type;

    if (memcpy_s((uint8_t *)(puc_data + MAC_80211_FRAME_LEN), WLAN_MEM_NETBUF_SIZE2 - MAC_80211_FRAME_LEN,
                 (uint8_t *)&twt_teardown, sizeof(mac_twt_teardown_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_mgmt_encap_twt_teardown_req:memcopy fail!");
        return 0;
    }
    us_index = MAC_80211_FRAME_LEN + sizeof(mac_twt_teardown_stru);
    /* 返回的帧长度中不包括FCS */
    return us_index;
}


uint32_t hmac_mgmt_tx_twt_setup_precheck(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_twt_action_mgmt_args_stru *twt_action_args, mac_vap_stru **mac_vap)
{
    mac_device_stru *mac_device = NULL;

    if (oal_any_null_ptr3(hmac_vap, hmac_user, twt_action_args)) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_tx_twt_setup::hmac_vap or hmac_user or twt_action_args is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *mac_vap = &(hmac_vap->st_vap_base_info);
    mac_device = mac_res_get_dev((*mac_vap)->uc_device_id);
    if (mac_device == NULL) {
        oam_error_log0((*mac_vap)->uc_vap_id, OAM_SF_11AX, "{hmac_mgmt_tx_twt_setup::device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_netbuf_stru* hmac_mgmt_tx_twt_steup_alloc_and_encap(hmac_user_stru *hmac_user,
    mac_twt_action_mgmt_args_stru *twt_action_args, hmac_vap_stru *hmac_vap, uint16_t *frame_len)
{
    dmac_ctx_action_event_stru wlan_ctx_action;
    mac_tx_ctl_stru *tx_ctl = NULL;

    /* 申请TWT SETUP管理帧内存 */
    oal_netbuf_stru *twt_setup_req = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2,
        OAL_NETBUF_PRIORITY_MID);

    if (twt_setup_req == NULL) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_tx_twt_setup::twt_setup_req null.}");
        return NULL;
    }

    oal_mem_netbuf_trace(twt_setup_req, OAL_TRUE);
    oal_netbuf_prev(twt_setup_req) = NULL;
    oal_netbuf_next(twt_setup_req) = NULL;

    hmac_vap->st_twt_cfg[0].uc_twt_dialog_token++;
    oam_warning_log1(0, OAM_SF_11AX, "{hmac_mgmt_tx_twt_setup::token,%d}", hmac_vap->st_twt_cfg[0].uc_twt_dialog_token);

    /* 调用封装管理帧接口 */
    *frame_len = hmac_mgmt_encap_twt_setup_req(hmac_vap, hmac_user, oal_netbuf_data(twt_setup_req), twt_action_args);

    memset_s((uint8_t *)&wlan_ctx_action, sizeof(wlan_ctx_action), 0, sizeof(wlan_ctx_action));

    /* 赋值要传入Dmac的信息 */
    wlan_ctx_action.us_frame_len = *frame_len;
    wlan_ctx_action.uc_hdr_len = MAC_80211_FRAME_LEN;
    wlan_ctx_action.en_action_category = MAC_ACTION_CATEGORY_S1G;
    wlan_ctx_action.uc_action = MAC_S1G_ACTION_TWT_SETUP;
    wlan_ctx_action.us_user_idx = hmac_user->st_user_base_info.us_assoc_id;

    if (memcpy_s((uint8_t *)(oal_netbuf_data(twt_setup_req) + (*frame_len)), WLAN_MEM_NETBUF_SIZE2 - (*frame_len),
        (uint8_t *)&wlan_ctx_action, sizeof(dmac_ctx_action_event_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_11AX, "hmac_mgmt_tx_twt_setup::memcpy fail, free netbuf!");
        oal_netbuf_free(twt_setup_req);
        return NULL;
    }
    oal_netbuf_put(twt_setup_req, (*frame_len + sizeof(dmac_ctx_action_event_stru)));
    /* 初始化CB */
    memset_s(oal_netbuf_cb(twt_setup_req), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(twt_setup_req);
    MAC_GET_CB_MPDU_LEN(tx_ctl) = *frame_len + sizeof(dmac_ctx_action_event_stru);
    MAC_GET_CB_FRAME_TYPE(tx_ctl) = WLAN_CB_FRAME_TYPE_ACTION;
    MAC_GET_CB_FRAME_SUBTYPE(tx_ctl) = WLAN_ACTION_TWT_SETUP_REQ;

    return twt_setup_req;
}


uint32_t hmac_mgmt_tx_twt_setup(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_twt_action_mgmt_args_stru *twt_action_args)
{
    mac_vap_stru *mac_vap = NULL;
    frw_event_mem_stru *event_mem = NULL;
    oal_netbuf_stru *twt_setup_req = NULL;
    uint16_t frame_len = 0;
    frw_event_stru *hmac_to_dmac_ctx_event = NULL;
    dmac_tx_event_stru *tx_event = NULL;
    uint32_t ret;

    ret = hmac_mgmt_tx_twt_setup_precheck(hmac_vap, hmac_user, twt_action_args, &mac_vap);
    if (ret != OAL_SUCC) {
        return ret;
    }

    twt_setup_req = hmac_mgmt_tx_twt_steup_alloc_and_encap(hmac_user, twt_action_args, hmac_vap, &frame_len);
    if (twt_setup_req == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (event_mem == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_11AX, "{hmac_mgmt_tx_twt_setup::event_mem null.}");
        oal_netbuf_free(twt_setup_req);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_to_dmac_ctx_event = frw_get_event_stru(event_mem);
    frw_event_hdr_init(&(hmac_to_dmac_ctx_event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX,
        DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT, sizeof(dmac_tx_event_stru), FRW_EVENT_PIPELINE_STAGE_1,
        mac_vap->uc_chip_id, mac_vap->uc_device_id, mac_vap->uc_vap_id);

    tx_event = (dmac_tx_event_stru *)(hmac_to_dmac_ctx_event->auc_event_data);
    tx_event->pst_netbuf = twt_setup_req;
    tx_event->us_frame_len = frame_len;

    ret = frw_event_dispatch_event(event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_11AX,
                       "{hmac_mgmt_tx_twt_setup::frw_event_dispatch_event failed[%d].}", ret);
        oal_netbuf_free(twt_setup_req);
        frw_event_free_m(event_mem);
        return ret;
    }

    frw_event_free_m(event_mem);
    return OAL_SUCC;
}


uint32_t hmac_mgmt_tx_twt_teardown(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, mac_twt_action_mgmt_args_stru *twt_action_args)
{
    mac_device_stru *device = NULL;
    mac_vap_stru *mac_vap = NULL;
    frw_event_mem_stru *event_mem = NULL; /* 申请事件返回的内存指针 */
    oal_netbuf_stru *twt_setup_req = NULL;
    uint16_t us_frame_len;
    frw_event_stru *hmac_to_dmac_ctx_event = NULL;
    dmac_tx_event_stru *tx_event = NULL;
    dmac_ctx_action_event_stru wlan_ctx_action;
    uint32_t ret;
    mac_tx_ctl_stru *tx_ctl = NULL;

    if (oal_any_null_ptr3(hmac_vap, hmac_user, twt_action_args)) {
        oam_error_log0(0, OAM_SF_11AX,
            "{hmac_mgmt_tx_twt_teardown::hmac_vap or hmac_user or twt_action_args is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap = &(hmac_vap->st_vap_base_info);

    /* 获取device结构 */
    device = mac_res_get_dev(mac_vap->uc_device_id);
    if (device == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_11AX, "{hmac_mgmt_tx_twt_teardown::device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请TWT SETUP管理帧内存 */
    twt_setup_req = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (twt_setup_req == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_11AX,
                       "{hmac_mgmt_tx_twt_teardown::twt_teardown_req null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_mem_netbuf_trace(twt_setup_req, OAL_TRUE);

    oal_netbuf_prev(twt_setup_req) = NULL;
    oal_netbuf_next(twt_setup_req) = NULL;

    /* 调用封装管理帧接口 */
    us_frame_len = hmac_mgmt_encap_twt_teardown_req(hmac_vap, hmac_user,
                                                    oal_netbuf_data(twt_setup_req),
                                                    twt_action_args);
    memset_s((uint8_t *)&wlan_ctx_action, sizeof(wlan_ctx_action), 0, sizeof(wlan_ctx_action));
    /* 赋值要传入Dmac的信息 */
    wlan_ctx_action.us_frame_len = us_frame_len;
    wlan_ctx_action.uc_hdr_len = MAC_80211_FRAME_LEN;
    wlan_ctx_action.en_action_category = MAC_ACTION_CATEGORY_S1G;
    wlan_ctx_action.uc_action = MAC_S1G_ACTION_TWT_TEARDOWN;
    wlan_ctx_action.us_user_idx = hmac_user->st_user_base_info.us_assoc_id;

    if (EOK != memcpy_s((uint8_t *)(oal_netbuf_data(twt_setup_req) + us_frame_len),
        (WLAN_MEM_NETBUF_SIZE2 - us_frame_len), (uint8_t *)&wlan_ctx_action,
        sizeof(dmac_ctx_action_event_stru))) {
        oam_error_log0(0, OAM_SF_11AX, "hmac_mgmt_tx_twt_teardown::memcpy fail!");
        oal_netbuf_free(twt_setup_req);
        return OAL_FAIL;
    }
    oal_netbuf_put(twt_setup_req, (us_frame_len + sizeof(dmac_ctx_action_event_stru)));

    /* 初始化CB */
    memset_s(oal_netbuf_cb(twt_setup_req), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(twt_setup_req);
    MAC_GET_CB_MPDU_LEN(tx_ctl) = us_frame_len + sizeof(dmac_ctx_action_event_stru);
    MAC_GET_CB_FRAME_TYPE(tx_ctl) = WLAN_CB_FRAME_TYPE_ACTION;
    /* 在dmac_tx_action_process  skip, 直接send to air */
    MAC_GET_CB_FRAME_SUBTYPE(tx_ctl) = WLAN_ACTION_TWT_TEARDOWN_REQ;

    event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (event_mem == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_11AX, "{hmac_mgmt_tx_twt_teardown::event_mem null.}");
        oal_netbuf_free(twt_setup_req);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_to_dmac_ctx_event = frw_get_event_stru(event_mem);
    frw_event_hdr_init(&(hmac_to_dmac_ctx_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT,
                       sizeof(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       mac_vap->uc_chip_id,
                       mac_vap->uc_device_id,
                       mac_vap->uc_vap_id);

    tx_event = (dmac_tx_event_stru *)(hmac_to_dmac_ctx_event->auc_event_data);
    tx_event->pst_netbuf = twt_setup_req;
    tx_event->us_frame_len = us_frame_len;

    ret = frw_event_dispatch_event(event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_11AX,
                       "{hmac_mgmt_tx_twt_teardown::frw_event_dispatch_event failed[%d].}", ret);
        oal_netbuf_free(twt_setup_req);
        frw_event_free_m(event_mem);
        return ret;
    }

    hmac_vap->st_twt_cfg[0].uc_twt_session_enable = 0;

    frw_event_free_m(event_mem);
    return OAL_SUCC;
}


OAL_STATIC void hmac_sta_twt_save_parameters(hmac_vap_stru *hmac_vap,
    mac_itwt_ie_stru *twt_element, const uint8_t idx)
{
    /*************************************************************************/
    /*                  TWT Element Format                                   */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Control | TWT parameter Information            */
    /* --------------------------------------------------------------------- */
    /* | 1         | 1      | 1      |      Variable             |           */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    /* 单播twt按单播结构体取值 */
    if (twt_element->st_control.bit_negotiation < MAC_TWT_NEGO_BROADCAST) {
        hmac_vap->st_twt_cfg[idx].ull_twt_start_time = twt_element->twt;
        /* Nominal Minimum TWT Wake Duration, 单位是256us */
        hmac_vap->st_twt_cfg[idx].twt_duration =
            ((uint32_t)twt_element->min_duration) << BIT_OFFSET_8;
        hmac_vap->st_twt_cfg[idx].ull_twt_interval =
            (uint64_t)twt_element->mantissa << twt_element->st_request_type.bit_exponent;
        hmac_vap->st_twt_cfg[idx].uc_twt_flow_id = twt_element->st_request_type.bit_flow_id;
        hmac_vap->st_twt_cfg[idx].uc_twt_announce_bit = twt_element->st_request_type.bit_flow_type;
        hmac_vap->st_twt_cfg[idx].uc_twt_is_trigger_enabled = twt_element->st_request_type.bit_trigger;
    } else {
        mac_btwt_ie_stru *btwt_element = &((mac_btwt_action_stru *)twt_element)->btwt_ie;
        hmac_vap->st_twt_cfg[idx].ull_twt_start_time = btwt_element->twt;
        /* Nominal Minimum TWT Wake Duration, 单位是256us */
        hmac_vap->st_twt_cfg[idx].twt_duration =
            ((uint32_t)btwt_element->min_duration) << BIT_OFFSET_8;
        hmac_vap->st_twt_cfg[idx].ull_twt_interval =
            (uint64_t)btwt_element->mantissa << btwt_element->request_type.exponent;
        hmac_vap->st_twt_cfg[idx].uc_twt_flow_id = btwt_element->request_type.flow_id;
        hmac_vap->st_twt_cfg[idx].uc_twt_announce_bit = btwt_element->request_type.flow_type;
        hmac_vap->st_twt_cfg[idx].uc_twt_is_trigger_enabled = btwt_element->request_type.trigger;
    }
    hmac_vap->st_twt_cfg[idx].uc_twt_session_enable = 1;
}


OAL_STATIC uint32_t hmac_sta_twt_config_accept_action(hmac_vap_stru *hmac_vap,
    mac_twt_action_mgmt_args_stru *twt_action_args, mac_itwt_ie_stru *twt_element)
{
    if (oal_any_null_ptr3(hmac_vap, twt_action_args, twt_element)) {
        oam_error_log0(0, OAM_SF_11AX,
            "{hmac_sta_twt_config_accept_action::hmac_vap or twt_action_args or twt_element is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*************************************************************************/
    /*                  TWT Element Format                                   */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Control | TWT parameter Information            */
    /* --------------------------------------------------------------------- */
    /* | 1         | 1      | 1      |      Variable             |           */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    twt_action_args->category = MAC_ACTION_CATEGORY_S1G;
    twt_action_args->action = MAC_S1G_ACTION_TWT_SETUP;
    twt_action_args->twt_setup_cmd = MAC_TWT_COMMAND_ACCEPT;
    twt_action_args->twt_flow_type = twt_element->st_request_type.bit_flow_type;
    twt_action_args->twt_flow_id = twt_element->st_request_type.bit_flow_id;
    twt_action_args->twt = (uint32_t)(twt_element->twt & 0xffffffff);
    twt_action_args->twt_exponent = twt_element->st_request_type.bit_exponent;
    twt_action_args->twt_duration = twt_element->min_duration;
    twt_action_args->intrval_mantissa = twt_element->mantissa;

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_sta_twt_update_parameters_to_device(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, uint8_t idx)
{
    mac_vap_stru *mac_vap = NULL;
    frw_event_mem_stru *event_mem = NULL; /* 申请事件返回的内存指针 */
    frw_event_stru *hmac_to_dmac_crx_sync = NULL;
    dmac_ctx_update_twt_stru *update_twt_cfg_event = NULL;

    if (oal_any_null_ptr2(hmac_vap, hmac_user)) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_sta_twt_update_parameters_to_device::hmac_vap or hmac_user is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap = &(hmac_vap->st_vap_base_info);

    /* 抛事件到DMAC处理 */
    event_mem = frw_event_alloc_m(sizeof(dmac_ctx_update_twt_stru));
    if (event_mem == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_11AX,
                       "{hmac_sta_twt_update_parameters_to_device::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    hmac_to_dmac_crx_sync = frw_get_event_stru(event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(hmac_to_dmac_crx_sync->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_UPDATE_TWT,
                       sizeof(dmac_ctx_update_twt_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       mac_vap->uc_chip_id,
                       mac_vap->uc_device_id,
                       mac_vap->uc_vap_id);

    /* 获取帧体信息，由于DMAC的同步，填写事件payload */
    update_twt_cfg_event = (dmac_ctx_update_twt_stru *)(hmac_to_dmac_crx_sync->auc_event_data);
    update_twt_cfg_event->en_action_category = MAC_ACTION_CATEGORY_S1G;
    update_twt_cfg_event->uc_action = MAC_S1G_ACTION_TWT_SETUP;
    update_twt_cfg_event->us_user_idx = hmac_user->st_user_base_info.us_assoc_id;
    update_twt_cfg_event->st_twt_cfg.ull_twt_start_time = hmac_vap->st_twt_cfg[idx].ull_twt_start_time;
    update_twt_cfg_event->st_twt_cfg.ull_twt_interval = hmac_vap->st_twt_cfg[idx].ull_twt_interval;
    update_twt_cfg_event->st_twt_cfg.twt_duration = hmac_vap->st_twt_cfg[idx].twt_duration;
    update_twt_cfg_event->st_twt_cfg.uc_twt_flow_id = hmac_vap->st_twt_cfg[idx].uc_twt_flow_id;
    update_twt_cfg_event->st_twt_cfg.uc_twt_announce_bit = hmac_vap->st_twt_cfg[idx].uc_twt_announce_bit;
    update_twt_cfg_event->st_twt_cfg.uc_twt_is_trigger_enabled = hmac_vap->st_twt_cfg[idx].uc_twt_is_trigger_enabled;
    update_twt_cfg_event->st_twt_cfg.uc_twt_session_enable = hmac_vap->st_twt_cfg[idx].uc_twt_session_enable;
    update_twt_cfg_event->st_twt_cfg.uc_next_twt_size = 0; /* 只有twt information帧才有 */
    update_twt_cfg_event->st_twt_cfg.twt_type = hmac_vap->st_twt_cfg[idx].twt_type;
    update_twt_cfg_event->twt_idx = idx;

    /* 分发 */
    frw_event_dispatch_event(event_mem);

    /* 释放事件内存 */
    frw_event_free_m(event_mem);
    return OAL_SUCC;
}


uint32_t hmac_sta_rx_twt_setup_check(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    uint8_t *puc_payload)
{
    mac_itwt_ie_stru *itwt_element = NULL;
    mac_twt_command_enum_uint8 twt_command;
    uint64_t twt_interval;
    uint32_t twt_wake_duration;

    if (g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_twt_requester_support == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_11AX,
            "{hmac_sta_rx_twt_setup_frame::twt requester support is off in the ini file!.}");
        return OAL_FAIL;
    }

    if (oal_any_null_ptr3(hmac_vap, hmac_user, puc_payload)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /******************************************************************/
    /*       TWT setup Frame - Frame Body                         */
    /* --------------------------------------------------------------- */
    /* | Category | Action | Dialog | TWT element     |               */
    /* --------------------------------------------------------------- */
    /* | 1        | 1      | 1      |  Variable  |                    */
    /* --------------------------------------------------------------- */
    /*                                                                */
    /******************************************************************/
    /*************************************************************************/
    /*                  TWT Element Format              */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Control | TWT parameter Information            */
    /* --------------------------------------------------------------------- */
    /* | 1         | 1      | 1      |      Variable             |           */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    itwt_element = (mac_itwt_ie_stru *)(puc_payload);
    /* 非TWT element */
    if (itwt_element->element_id != MAC_EID_TWT) {
        oam_warning_log1(0, OAM_SF_11AX,
            "{hmac_sta_rx_twt_setup_frame:: It's not a twt element id:%d}",itwt_element->element_id);
        return OAL_FAIL;
    }
    oam_warning_log3(0, OAM_SF_11AX,
        "{hmac_sta_rx_twt_setup_frame:: flow_id:%d, bit_flow_type:%d, bit_setup_command:%d}",
        itwt_element->st_request_type.bit_flow_id, itwt_element->st_request_type.bit_flow_type,
        itwt_element->st_request_type.bit_setup_command);

    twt_command = (mac_twt_command_enum_uint8)itwt_element->st_request_type.bit_setup_command;

    /* 大于2的twt commad都是ap发送过来的，需要检查dialog token */
    if ((twt_command > MAC_TWT_COMMAND_DEMAND) &&
        (itwt_element->dialog_token != hmac_vap->st_twt_cfg[0].uc_twt_dialog_token)) {
        oam_warning_log3(0, OAM_SF_11AX,
            "{hmac_sta_rx_twt_setup_frame:: twt dialog token is not same: rx %d, tx:%d, twt setup command:%d}",
            itwt_element->dialog_token, hmac_vap->st_twt_cfg[0].uc_twt_dialog_token, twt_command);
        return OAL_FAIL;
    }

    /* 检查twt参数，非法参数不允许建立twt会话 */
    if (itwt_element->st_control.bit_negotiation < MAC_TWT_NEGO_BROADCAST) {
        twt_interval = (uint64_t)itwt_element->mantissa << itwt_element->st_request_type.bit_exponent;
        twt_wake_duration = itwt_element->min_duration * 256; /* Nominal Minimum TWT Wake Duration, 单位是256us */
    } else {
        mac_btwt_ie_stru *btwt_ie = &((mac_btwt_action_stru *)puc_payload)->btwt_ie;
        twt_interval = (uint64_t)btwt_ie->mantissa << btwt_ie->request_type.exponent;
        twt_wake_duration = btwt_ie->min_duration * 256; /* Nominal Minimum TWT Wake Duration, 单位是256us */
    }

    if (twt_interval < twt_wake_duration) {
        oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_11AX,
            "{hmac_sta_rx_twt_setup_frame::twt interval [%d] < twt wake duration!}",
            twt_interval, twt_wake_duration);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}

uint32_t hmac_sta_rx_twt_setup_frame(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    uint8_t *puc_payload)
{
    mac_twt_action_mgmt_args_stru twt_action_args = { 0 };
    mac_btwt_action_stru *btwt_action_args = (mac_btwt_action_stru *)(puc_payload);
    mac_itwt_ie_stru *itwt_element = (mac_itwt_ie_stru *)(puc_payload);
    mac_twt_command_enum_uint8 twt_command =
        (mac_twt_command_enum_uint8)itwt_element->st_request_type.bit_setup_command;

    if (hmac_sta_rx_twt_setup_check(hmac_vap, hmac_user, puc_payload) != OAL_SUCC) {
        return OAL_FAIL;
    }
    if (twt_command == MAC_TWT_COMMAND_REJECT || twt_command == MAC_TWT_COMMAND_GROUPING) {
        return OAL_SUCC;
    }

    /* 单播处理 */
    if (itwt_element->st_control.bit_negotiation < MAC_TWT_NEGO_BROADCAST) {
        hmac_sta_twt_save_parameters(hmac_vap, itwt_element, 0);
        if (twt_command == MAC_TWT_COMMAND_ACCEPT) {
            hmac_sta_twt_update_parameters_to_device(hmac_vap, hmac_user, 0);
        } else {
            hmac_sta_twt_config_accept_action(hmac_vap, &twt_action_args, itwt_element);
            /* 在dmac发送完成中断收到ack后再设置twt参数 */
            hmac_mgmt_tx_twt_setup(hmac_vap, hmac_user, &twt_action_args);
        }
        return OAL_SUCC;
    }

    /* 广播处理 */
    hmac_vap->st_twt_cfg[0].twt_type = 1;
    hmac_sta_twt_save_parameters(hmac_vap, itwt_element, 0);
    if (twt_command == MAC_TWT_COMMAND_ACCEPT) {
        hmac_sta_twt_update_parameters_to_device(hmac_vap, hmac_user, 0);
    } else {
        btwt_action_args->btwt_ie.request_type.setup_command = MAC_TWT_COMMAND_ACCEPT;
        /* 在dmac发送完成中断收到ack后再设置twt参数 */
        hmac_mgmt_tx_twt_setup(hmac_vap, hmac_user, (mac_twt_action_mgmt_args_stru *)puc_payload);
    }
    return OAL_SUCC;
}


uint32_t hmac_sta_rx_twt_teardown_frame(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, uint8_t *puc_payload)
{
    mac_device_stru *device = NULL;
    mac_vap_stru *mac_vap = NULL;
    mac_twt_teardown_stru *twt_teardown = NULL;
    frw_event_mem_stru *event_mem = NULL; /* 申请事件返回的内存指针 */
    frw_event_stru *hmac_to_dmac_crx_sync = NULL;
    dmac_ctx_update_twt_stru *update_twt_cfg_event = NULL;
    uint8_t idx = 0;

    if (oal_any_null_ptr3(hmac_vap, hmac_user, puc_payload)) {
        oam_error_log0(0, OAM_SF_11AX,
                       "{hmac_sta_rx_twt_teardown_frame::hmac_vap or hmac_user or payload is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap = &(hmac_vap->st_vap_base_info);

    /* 获取device结构 */
    device = mac_res_get_dev(mac_vap->uc_device_id);
    if (device == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_11AX,
                       "{hmac_sta_rx_twt_teardown_frame::twt teardown receive failed, device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /******************************************************************/
    /*       TWT teardown Frame - Frame Body                         */
    /* --------------------------------------------------------------- */
    /* | Category | Action | TWT Flow   |                             */
    /* --------------------------------------------------------------- */
    /* | 1        | 1      |    1       |                             */
    /* --------------------------------------------------------------- */
    /*                                                                */
    /******************************************************************/
    twt_teardown = (mac_twt_teardown_stru *)(puc_payload);

    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_11AX,
                     "{hmac_sta_rx_twt_teardown_frame:: flow_id:%d, bit_nego_type:%d}",
                     twt_teardown->bit_twt_flow_id, twt_teardown->bit_nego_type);

    /* 只支持删除单播TWT会话 */
    if (twt_teardown->bit_nego_type != 0) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_11AX,
                         "{hmac_sta_rx_twt_teardown_frame:: not support twt negotiation type:%d}",
                         twt_teardown->bit_nego_type);
        return OAL_FAIL;
    }

    /* 检查TWT是否在运行 */
    if (hmac_vap->st_twt_cfg[idx].uc_twt_session_enable == 0) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_11AX,
                         "{hmac_sta_rx_twt_teardown_frame:: twt session is already disabled!}");
        return OAL_FAIL;
    }

    /* 检查flow id */
    if (hmac_vap->st_twt_cfg[idx].uc_twt_flow_id != twt_teardown->bit_twt_flow_id) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_11AX,
                         "{hmac_sta_rx_twt_teardown_frame:: the flow id [%d] is invalid,our flow id is [%d]}",
                         twt_teardown->bit_twt_flow_id, hmac_vap->st_twt_cfg[idx].uc_twt_flow_id);
        return OAL_FAIL;
    }

    /*  To-Do, bit number不一样 */
    hmac_vap->st_twt_cfg[idx].ull_twt_start_time = 0;
    hmac_vap->st_twt_cfg[idx].twt_duration = 0;
    hmac_vap->st_twt_cfg[idx].ull_twt_interval = 0;
    hmac_vap->st_twt_cfg[idx].uc_twt_flow_id = twt_teardown->bit_twt_flow_id;
    hmac_vap->st_twt_cfg[idx].uc_twt_announce_bit = 0;
    hmac_vap->st_twt_cfg[idx].uc_twt_is_trigger_enabled = 0;
    hmac_vap->st_twt_cfg[idx].uc_twt_session_enable = 0;
    hmac_vap->st_twt_cfg[idx].twt_idx = 0;

    /* 抛事件到DMAC处理 */
    event_mem = frw_event_alloc_m(sizeof(dmac_ctx_update_twt_stru));
    if (event_mem == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_11AX,
                       "{hmac_sta_rx_twt_teardown_frame::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    hmac_to_dmac_crx_sync = frw_get_event_stru(event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(hmac_to_dmac_crx_sync->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_UPDATE_TWT,
                       sizeof(dmac_ctx_update_twt_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       mac_vap->uc_chip_id,
                       mac_vap->uc_device_id,
                       mac_vap->uc_vap_id);

    /* 获取帧体信息，由于DMAC的同步，填写事件payload */
    update_twt_cfg_event = (dmac_ctx_update_twt_stru *)(hmac_to_dmac_crx_sync->auc_event_data);
    update_twt_cfg_event->en_action_category = MAC_ACTION_CATEGORY_S1G;
    update_twt_cfg_event->uc_action = MAC_S1G_ACTION_TWT_TEARDOWN;
    update_twt_cfg_event->us_user_idx = hmac_user->st_user_base_info.us_assoc_id;
    update_twt_cfg_event->st_twt_cfg.ull_twt_start_time = hmac_vap->st_twt_cfg[idx].ull_twt_start_time;
    update_twt_cfg_event->st_twt_cfg.ull_twt_interval = hmac_vap->st_twt_cfg[idx].ull_twt_interval;
    update_twt_cfg_event->st_twt_cfg.twt_duration = hmac_vap->st_twt_cfg[idx].twt_duration;
    update_twt_cfg_event->st_twt_cfg.uc_twt_flow_id = hmac_vap->st_twt_cfg[idx].uc_twt_flow_id;
    update_twt_cfg_event->st_twt_cfg.uc_twt_announce_bit = hmac_vap->st_twt_cfg[idx].uc_twt_announce_bit;
    update_twt_cfg_event->st_twt_cfg.uc_twt_is_trigger_enabled = hmac_vap->st_twt_cfg[idx].uc_twt_is_trigger_enabled;
    update_twt_cfg_event->st_twt_cfg.uc_twt_session_enable = hmac_vap->st_twt_cfg[idx].uc_twt_session_enable;
    update_twt_cfg_event->st_twt_cfg.uc_next_twt_size = 0; /* 只有twt information帧才有 */
    update_twt_cfg_event->st_twt_cfg.twt_idx = idx;

    /* 分发 */
    frw_event_dispatch_event(event_mem);

    /* 释放事件内存 */
    frw_event_free_m(event_mem);
    return OAL_SUCC;
}


OAL_STATIC uint64_t hmac_get_next_twt(hmac_vap_stru *hmac_vap, uint8_t *puc_payload,
    uint8_t next_twt_subfield_size)
{
    uint32_t next_twt_low;
    uint64_t next_twt_high;
    uint64_t ull_next_twt;

    switch (next_twt_subfield_size) {
        case HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_0:
            ull_next_twt = 0;
            break;

        case HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_32: /* 32 bits */
            next_twt_low = puc_payload[BYTE_OFFSET_0];
            next_twt_low |= (puc_payload[BYTE_OFFSET_1] << BIT_OFFSET_8);
            next_twt_low |= (puc_payload[BYTE_OFFSET_2] << BIT_OFFSET_16);
            next_twt_low |= (puc_payload[BYTE_OFFSET_3] << BIT_OFFSET_24);
            ull_next_twt = next_twt_low;
            break;

        case HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_48: /* 48 bits */
            next_twt_low = puc_payload[BYTE_OFFSET_0];
            next_twt_low |= (puc_payload[BYTE_OFFSET_1] << BIT_OFFSET_8);
            next_twt_low |= (puc_payload[BYTE_OFFSET_2] << BIT_OFFSET_16);
            next_twt_low |= (puc_payload[BYTE_OFFSET_3] << BIT_OFFSET_24);
            next_twt_high = puc_payload[BYTE_OFFSET_4];
            next_twt_high |= (puc_payload[BYTE_OFFSET_5] << BIT_OFFSET_8);
            ull_next_twt = next_twt_low;
            ull_next_twt |= (next_twt_high << 32); /* 左移32位 */
            break;

        case HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_64: /* 64 bits */
            next_twt_low = puc_payload[BYTE_OFFSET_0];
            next_twt_low |= (puc_payload[BYTE_OFFSET_1] << BIT_OFFSET_8);
            next_twt_low |= (puc_payload[BYTE_OFFSET_2] << BIT_OFFSET_16);
            next_twt_low |= (puc_payload[BYTE_OFFSET_3] << BIT_OFFSET_24);
            next_twt_high = puc_payload[BYTE_OFFSET_4];
            next_twt_high |= (puc_payload[BYTE_OFFSET_5] << BIT_OFFSET_8);
            next_twt_high |= (puc_payload[BYTE_OFFSET_6] << BIT_OFFSET_16);
            next_twt_high |= ((uint32_t)puc_payload[BYTE_OFFSET_7] << BIT_OFFSET_24);
            ull_next_twt = next_twt_low;
            ull_next_twt |= (next_twt_high << 32); /* 左移32位 */
            break;

        default:
            ull_next_twt = 0;
            oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_11AX,
                             "{hmac_get_next_twt:: invalid next twt size [%d]!}",
                             next_twt_subfield_size);
            break;
    }

    return ull_next_twt;
}


uint32_t hmac_sta_rx_twt_information_frame(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, uint8_t *puc_payload)
{
    mac_device_stru *device = NULL;
    mac_vap_stru *mac_vap = NULL;
    mac_twt_information_frame_stru *twt_information_frame_body = NULL;

    frw_event_mem_stru *event_mem = NULL; /* 申请事件返回的内存指针 */
    frw_event_stru *hmac_to_dmac_crx_sync = NULL;
    dmac_ctx_update_twt_stru *update_twt_cfg_event = NULL;

    uint64_t ull_next_twt;
    uint8_t next_twt_subfield_size;

    if (oal_any_null_ptr3(hmac_vap, hmac_user, puc_payload)) {
        oam_error_log0(0, OAM_SF_11AX,
                       "{hmac_sta_rx_twt_information_frame::hmac_vap or hmac_user or payload is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap = &(hmac_vap->st_vap_base_info);

    /* 获取device结构 */
    device = mac_res_get_dev(mac_vap->uc_device_id);
    if (device == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_11AX,
            "{hmac_sta_rx_twt_information_frame::twt information receive failed, device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /******************************************************************/
    /*       TWT information Frame - Frame Body                       */
    /* --------------------------------------------------------------- */
    /* | Category | Action | TWT element | Next TWT                   */
    /* --------------------------------------------------------------- */
    /* | 1        | 1      | 1           |  Variable                   */
    /* --------------------------------------------------------------- */
    /*                                                                */
    /******************************************************************/
    twt_information_frame_body = (mac_twt_information_frame_stru *)(puc_payload);

    oam_warning_log2(0, OAM_SF_11AX, "{hmac_sta_rx_twt_information_frame:: flow id:%d, next twt subfield size:%d}",
        twt_information_frame_body->st_twt_information_filed.bit_twt_flow_id,
        twt_information_frame_body->st_twt_information_filed.bit_next_twt_subfield_size);

    /* 检查TWT是否在运行 */
    if (hmac_vap->st_twt_cfg[0].uc_twt_session_enable == 0) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_11AX,
            "{hmac_sta_rx_twt_information_frame:: twt session is disabled!}");
        return OAL_FAIL;
    }

    /* 检查flow id */
    if (hmac_vap->st_twt_cfg[0].uc_twt_flow_id !=
        twt_information_frame_body->st_twt_information_filed.bit_twt_flow_id) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_11AX,
            "{hmac_sta_rx_twt_information_frame:: the flow id [%d] is invalid,our flow id is [%d]}",
            twt_information_frame_body->st_twt_information_filed.bit_twt_flow_id,
            hmac_vap->st_twt_cfg[0].uc_twt_flow_id);
        return OAL_FAIL;
    }

    next_twt_subfield_size = twt_information_frame_body->st_twt_information_filed.bit_next_twt_subfield_size;
    puc_payload += sizeof(mac_twt_information_frame_stru);

    ull_next_twt = hmac_get_next_twt(hmac_vap, puc_payload, next_twt_subfield_size);
    /* 不支持flex twt且all_twt=0不处理        */
    if (ull_next_twt != 0 && twt_information_frame_body->st_twt_information_filed.bit_all_twt == 0 &&
        g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_twt_requester_support == OAL_TRUE) {
        return OAL_SUCC;
    }

    /*  这里只更新twt start time和next twt size */
    hmac_vap->st_twt_cfg[0].ull_twt_start_time = ull_next_twt;
    hmac_vap->st_twt_cfg[0].uc_next_twt_size = next_twt_subfield_size;
    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_11AX,
        "{hmac_sta_rx_twt_information_frame:: ull_next_twt=[%ld], next_twt_subfield_size=[%d]}",
        ull_next_twt, next_twt_subfield_size);

    /* 抛事件到DMAC处理 */
    event_mem = frw_event_alloc_m(sizeof(dmac_ctx_update_twt_stru));
    if (event_mem == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_11AX, "{hmac_sta_rx_twt_information_frame::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    hmac_to_dmac_crx_sync = frw_get_event_stru(event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(hmac_to_dmac_crx_sync->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_UPDATE_TWT,
                       sizeof(dmac_ctx_update_twt_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       mac_vap->uc_chip_id, mac_vap->uc_device_id, mac_vap->uc_vap_id);

    /* 获取帧体信息，由于DMAC的同步，填写事件payload */
    update_twt_cfg_event = (dmac_ctx_update_twt_stru *)(hmac_to_dmac_crx_sync->auc_event_data);
    update_twt_cfg_event->en_action_category = MAC_ACTION_CATEGORY_S1G;
    update_twt_cfg_event->uc_action = MAC_S1G_ACTION_TWT_INFORMATION;
    update_twt_cfg_event->us_user_idx = hmac_user->st_user_base_info.us_assoc_id;
    update_twt_cfg_event->st_twt_cfg.ull_twt_start_time = hmac_vap->st_twt_cfg[0].ull_twt_start_time;
    update_twt_cfg_event->st_twt_cfg.ull_twt_interval = hmac_vap->st_twt_cfg[0].ull_twt_interval;
    update_twt_cfg_event->st_twt_cfg.twt_duration = hmac_vap->st_twt_cfg[0].twt_duration;
    update_twt_cfg_event->st_twt_cfg.uc_twt_flow_id = hmac_vap->st_twt_cfg[0].uc_twt_flow_id;
    update_twt_cfg_event->st_twt_cfg.uc_twt_announce_bit = hmac_vap->st_twt_cfg[0].uc_twt_announce_bit;
    update_twt_cfg_event->st_twt_cfg.uc_twt_is_trigger_enabled = hmac_vap->st_twt_cfg[0].uc_twt_is_trigger_enabled;
    update_twt_cfg_event->st_twt_cfg.uc_twt_session_enable = hmac_vap->st_twt_cfg[0].uc_twt_session_enable;
    update_twt_cfg_event->st_twt_cfg.uc_next_twt_size = hmac_vap->st_twt_cfg[0].uc_next_twt_size;

    /* 分发 */
    frw_event_dispatch_event(event_mem);

    /* 释放事件内存 */
    frw_event_free_m(event_mem);
    return OAL_SUCC;
}


uint32_t hmac_dbac_teardown_twt_session(mac_device_stru *dev)
{
    mac_chip_stru *chip = NULL;
    mac_vap_stru *mac_vap_temp = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    uint8_t vap_num;
    uint32_t ret;

    chip = hmac_res_get_mac_chip(dev->uc_chip_id);
    if (chip == NULL) {
        oam_error_log1(0, OAM_SF_ANY,
            "hmac_dbac_teardown_twt_session::hmac_res_get_mac_chip id[%d] NULL",
            dev->uc_chip_id);
        return OAL_FAIL;
    }

    vap_num = hmac_get_chip_vap_num(chip);

    oam_warning_log1(0, OAM_SF_ANY, "{hmac_dbac_teardown_twt_session::check vap num for twt,vap num=[%d]}", vap_num);

    /* 非DBDC场景，如果启动了3个及以上的vap，说明是wlan0/p2p共存，
       此时如果已经建立twt会话，则需要删除twt会话 */
    if ((dev->en_dbdc_running == OAL_FALSE) && (vap_num >= HMAC_TWT_P2P_RUNNING_VAP_NUM)) {
        /* 先找到wlan vap */
        ret = mac_device_find_up_sta_wlan(dev, &mac_vap_temp);
        if ((ret == OAL_SUCC) && (mac_vap_temp != NULL)) {
            hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_temp->uc_vap_id);
            if (hmac_vap == NULL) {
                oam_error_log0(0, OAM_SF_CFG, "{hmac_dbac_teardown_twt_session::mac_res_get_hmac_vap failed.}");
                return OAL_FAIL;
            }

            oam_warning_log1(0, OAM_SF_ANY,
                "{hmac_dbac_teardown_twt_session::twt_session_enable=[%d]}",
                hmac_vap->st_twt_cfg[0].uc_twt_session_enable);

            /* 如果该vap的twt会话已经开启，则删除twt会话 */
            if (hmac_vap->st_twt_cfg[0].uc_twt_session_enable == 1) {
                hmac_config_twt_teardown_req_auto(&(hmac_vap->st_vap_base_info));
            }
        }
    }

    return OAL_SUCC;
}


uint32_t hmac_twt_auto_setup_session(mac_device_stru *device)
{
    mac_chip_stru *chip = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = NULL;
    mac_vap_stru *mac_vap_temp = NULL;
    uint8_t vap_num;
    uint32_t ret;
    mac_he_hdl_stru he_hdl;

    chip = hmac_res_get_mac_chip(device->uc_chip_id);
    if (chip == NULL) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_twt_auto_setup_session::hmac_res_get_mac_chip id[%d] NULL",
                       device->uc_chip_id);
        return OAL_FAIL;
    }
    memset_s(&he_hdl, sizeof(mac_he_hdl_stru), 0, sizeof(mac_he_hdl_stru));
    vap_num = hmac_get_chip_vap_num(chip);

    oam_warning_log1(0, OAM_SF_ANY, "{hmac_twt_auto_setup_session::check vap num for twt,vap num=[%d]}", vap_num);

    /* 只有1个vap的时候，如果ap支持twt，sta没有建立会话，则建立twt会话 */
    if ((device->en_dbdc_running == OAL_FALSE) && (vap_num == 2)) { /* 2表示VAP数 */
        ret = mac_device_find_up_sta_wlan(device, &mac_vap_temp);
        if ((ret == OAL_SUCC) && (mac_vap_temp != NULL)) {
            /* 重新建立twt会话 */
            hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_temp->uc_vap_id);
            if (hmac_vap == NULL) {
                oam_error_log0(0, OAM_SF_CFG, "{hmac_twt_auto_setup_session::mac_res_get_hmac_vap failed.}");
                return OAL_FAIL;
            }

            /* 获用户 */
            hmac_user = mac_res_get_hmac_user(hmac_vap->st_vap_base_info.us_assoc_vap_id);
            if (hmac_user == NULL) {
                /* 和ap侧一样，上层已经删除了的话，属于正常 */
                oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                                 "{hmac_twt_auto_setup_session::hmac_user[%d] is null.}",
                                 hmac_vap->st_vap_base_info.us_assoc_vap_id);
                return OAL_ERR_CODE_PTR_NULL;
            }

            /* 判断该AP是否支持twt，如果是fast ps模式，先检查ap是否支持twt，
             如果支持twt，建立twt会话，然后直接返回，否则进入fast ps模式 */
            mac_user_get_he_hdl(&(hmac_user->st_user_base_info), &he_hdl);
            if (he_hdl.st_he_cap_ie.st_he_mac_cap.bit_twt_responder_support == OAL_TRUE &&
                hmac_vap->st_twt_cfg[0].uc_twt_session_enable == 0) {
                hmac_config_twt_setup_req_auto(&(hmac_vap->st_vap_base_info));
            }
        }
    }

    return OAL_SUCC;
}

uint32_t _hmac_twt_auto_setup_session(mac_device_stru *device)
{
    if (g_wlan_spec_cfg->feature_twt_is_open) {
        return hmac_twt_auto_setup_session(device);
    }

    return OAL_SUCC;
}

OAL_STATIC void hmac_config_twt_action_args_init(mac_twt_action_mgmt_args_stru *twt_action_args,
    mac_cfg_twt_setup_req_param_stru *twt_setup_req)
{
    twt_action_args->category = MAC_ACTION_CATEGORY_S1G;
    twt_action_args->action = MAC_S1G_ACTION_TWT_SETUP;
    twt_action_args->twt_setup_cmd = twt_setup_req->uc_twt_setup_cmd;
    twt_action_args->twt_flow_type = twt_setup_req->uc_twt_flow_type;
    twt_action_args->twt_flow_id = twt_setup_req->uc_twt_flow_id;
    twt_action_args->twt = twt_setup_req->twt_start_time_offset;
    twt_action_args->twt_exponent = twt_setup_req->twt_exponent;
    twt_action_args->twt_duration = twt_setup_req->twt_duration;
    twt_action_args->intrval_mantissa = twt_setup_req->intrval_mantissa;
    return;
}


uint32_t hmac_config_twt_setup_req(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_twt_setup_req_param_stru *twt_setup_req = (mac_cfg_twt_setup_req_param_stru *)puc_param;
    hmac_user_stru *hmac_user = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_twt_action_mgmt_args_stru twt_action_args; /* 用于填写ACTION帧的参数 */
    mac_he_hdl_stru he_hdl;
    uint64_t ull_twt_interval;
    uint32_t twt_wake_duration;
    mac_device_stru *dev = NULL;
    mac_chip_stru *chip = NULL;
    uint8_t vap_num;

    if (g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_twt_requester_support == OAL_FALSE) {
        return OAL_FAIL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 如果是已经启动多vap的情况下，不能建立TWT会话 */
    dev = mac_res_get_dev(mac_vap->uc_device_id);
    if (oal_unlikely(dev == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    chip = hmac_res_get_mac_chip(dev->uc_chip_id);
    if (chip == NULL) {
        return OAL_FAIL;
    }
    vap_num = hmac_get_chip_vap_num(chip);
    /* 非DBDC场景，如果启动了3个及以上的vap，说明是wlan0/p2p共存，
    此时如果已经建立twt会话，则需要删除twt会话 */
    if ((dev->en_dbdc_running == OAL_FALSE) && (vap_num >= HMAC_TWT_P2P_RUNNING_VAP_NUM)) {
        return OAL_FAIL;
    }

    /* 若广播使能，后续不处理 */
    if (twt_setup_req->twt_nego_type != 0) {
        g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_btwt_requester_support = twt_setup_req->twt_nego_type;
        return OAL_SUCC;
    }

    /* 获取用户对应的索引 */
    hmac_user = mac_vap_get_hmac_user_by_addr(mac_vap, twt_setup_req->auc_mac_addr);
    if (hmac_user == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 判断该AP是否支持twt */
    mac_user_get_he_hdl(&(hmac_user->st_user_base_info), &he_hdl);
    if (he_hdl.st_he_cap_ie.st_he_mac_cap.bit_twt_responder_support == OAL_FALSE) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_11AX, "{hmac_config_twt_setup_req::ap not support twt responder!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* TWT Wake Interval Exponent: 5 bits， 取值范围[0,31] */
    if (twt_setup_req->twt_exponent > 31) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* Nominal Minimum TWT Wake Duration: 1byte, 取值范围[0, 255] */
    if (twt_setup_req->twt_duration > 255) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    ull_twt_interval = (uint64_t)twt_setup_req->intrval_mantissa << twt_setup_req->twt_exponent;
    /* Nominal Minimum TWT Wake Duration, 单位是256us */
    twt_wake_duration = twt_setup_req->twt_duration * 256;
    if (ull_twt_interval < twt_wake_duration) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    hmac_config_twt_action_args_init(&twt_action_args, twt_setup_req);
    hmac_mgmt_tx_twt_action(hmac_vap, hmac_user, &twt_action_args);

    return OAL_SUCC;
}


uint32_t hmac_config_twt_setup_req_auto(mac_vap_stru *mac_vap)
{
    hmac_user_stru *hmac_user = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_twt_action_mgmt_args_stru twt_action_args; /* 用于填写ACTION帧的参数 */
    mac_he_hdl_stru he_hdl;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_11AX, "{hmac_config_twt_setup_req_auto::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取用户对应的索引 */
    hmac_user = mac_res_get_hmac_user(hmac_vap->st_vap_base_info.us_assoc_vap_id);
    if (hmac_user == NULL) {
        /* 和ap侧一样，上层已经删除了的话，属于正常 */
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_config_twt_setup_req_auto::hmac_user[%d] is null.}",
                         hmac_vap->st_vap_base_info.us_assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 判断该AP是否支持twt */
    mac_user_get_he_hdl(&(hmac_user->st_user_base_info), &he_hdl);
    if (he_hdl.st_he_cap_ie.st_he_mac_cap.bit_twt_responder_support == OAL_FALSE) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_11AX,
                         "{hmac_config_twt_setup_req_auto::the ap doesn't support twt responder!.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    twt_action_args.category = MAC_ACTION_CATEGORY_S1G;
    twt_action_args.action = MAC_S1G_ACTION_TWT_SETUP;
    twt_action_args.twt_setup_cmd = 1;
    twt_action_args.twt_flow_type = 0;
    twt_action_args.twt_flow_id = 0;
    twt_action_args.twt = 0;
    twt_action_args.twt_exponent = 10; /* 10表示TWT清醒间隔指数部分 */
    twt_action_args.twt_duration = 255; /* 255表示STA希望的TWT SP最小持续时间 */
    twt_action_args.intrval_mantissa = 512; /* 512表示TWT清醒间隔基数部分 */

    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_11AX, "{hmac_config_twt_setup_req_auto:: %d, %d, %d, %d}",
                     twt_action_args.twt_setup_cmd, twt_action_args.twt_flow_type,
                     twt_action_args.twt_flow_id, twt_action_args.twt);

    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_11AX, "{hmac_config_twt_setup_req_auto:: %d, %d, %d}",
                     twt_action_args.twt_exponent, twt_action_args.twt_duration,
                     twt_action_args.intrval_mantissa);

    hmac_mgmt_tx_twt_action(hmac_vap, hmac_user, &twt_action_args);

    return OAL_SUCC;
}


uint32_t hmac_config_twt_teardown_req(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_twt_teardown_req_param_stru *twt_teardown_req = NULL;
    hmac_user_stru *hmac_user = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_twt_action_mgmt_args_stru twt_action_args = {0}; /* 用于填写ACTION帧的参数 */

    twt_teardown_req = (mac_cfg_twt_teardown_req_param_stru *)puc_param;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_11AX, "{hmac_config_twt_teardown_req::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获取用户对应的索引 */
    hmac_user = mac_vap_get_hmac_user_by_addr(mac_vap, twt_teardown_req->auc_mac_addr);
    if (hmac_user == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_11AX, "{hmac_config_twt_teardown_req::hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap->st_twt_cfg[0].uc_twt_session_enable == 0) {
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_11AX,
            "{hmac_config_twt_teardown_req::the twt session of the ap[*:*:%x:%x:%x:%x] hasn't seted up yet!.}",
            twt_teardown_req->auc_mac_addr[MAC_ADDR_2],
            twt_teardown_req->auc_mac_addr[MAC_ADDR_3],
            twt_teardown_req->auc_mac_addr[MAC_ADDR_4],
            twt_teardown_req->auc_mac_addr[MAC_ADDR_5]);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (twt_teardown_req->uc_twt_flow_id != hmac_vap->st_twt_cfg[0].uc_twt_flow_id) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_11AX,
            "{hmac_config_twt_teardown_req:: twt flow id isn't same:seted up id[%d],request teardown id[%d]!.}",
            hmac_vap->st_twt_cfg[0].uc_twt_flow_id, twt_teardown_req->uc_twt_flow_id);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    twt_action_args.category = MAC_ACTION_CATEGORY_S1G;
    twt_action_args.action = MAC_S1G_ACTION_TWT_TEARDOWN;
    twt_action_args.twt_flow_id = twt_teardown_req->uc_twt_flow_id;
    twt_action_args.twt_nego_type = twt_teardown_req->twt_nego_type;

    hmac_mgmt_tx_twt_action(hmac_vap, hmac_user, &twt_action_args);

    return OAL_SUCC;
}


uint32_t hmac_config_twt_teardown_req_auto(mac_vap_stru *mac_vap)
{
    hmac_user_stru *hmac_user = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_twt_action_mgmt_args_stru twt_action_args = {0}; /* 用于填写ACTION帧的参数 */

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_11AX, "{hmac_config_twt_teardown_req::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取用户对应的索引 */
    hmac_user = mac_res_get_hmac_user(hmac_vap->st_vap_base_info.us_assoc_vap_id);
    if (oal_unlikely(hmac_user == NULL)) {
        /* 和ap侧一样，上层已经删除了的话，属于正常 */
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_config_twt_setup_req_auto::hmac_user[%d] is null.}",
                         hmac_vap->st_vap_base_info.us_assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    twt_action_args.category = MAC_ACTION_CATEGORY_S1G;
    twt_action_args.action = MAC_S1G_ACTION_TWT_TEARDOWN;
    twt_action_args.twt_flow_id = hmac_vap->st_twt_cfg[0].uc_twt_flow_id;

    hmac_mgmt_tx_twt_action(hmac_vap, hmac_user, &twt_action_args);

    return OAL_SUCC;
}


uint32_t hmac_proc_rx_process_twt_sync_event_tx_adapt(frw_event_mem_stru *event_mem)
{
    return hmac_hcc_tx_event_payload_to_netbuf(event_mem, sizeof(dmac_ctx_update_twt_stru));
}


OAL_STATIC void hmac_mgmt_tx_twt_action(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_twt_action_mgmt_args_stru *twt_action_args)
{
    if (twt_action_args->category == (uint8_t)MAC_ACTION_CATEGORY_S1G) {
        switch (twt_action_args->action) {
            case MAC_S1G_ACTION_TWT_SETUP:
                oam_info_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_11AX,
                              "{hmac_mgmt_tx_twt_action::MAC_TWT_SETUP.}");
                hmac_mgmt_tx_twt_setup(hmac_vap, hmac_user, twt_action_args);
                break;
            case MAC_S1G_ACTION_TWT_TEARDOWN:
                oam_info_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_11AX,
                              "{hmac_mgmt_tx_twt_action::MAC_TWT_TEARDOWN.}");
                hmac_mgmt_tx_twt_teardown(hmac_vap, hmac_user, twt_action_args);
                break;
        }
    } else {
        oam_info_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_11AX,
                      "{hmac_mgmt_tx_twt_action::invalid action type[%d].}", twt_action_args->category);
    }
}


uint32_t mac_device_find_up_sta_wlan(mac_device_stru *mac_device, mac_vap_stru **ppst_mac_vap)
{
    uint8_t vap_idx;
    mac_vap_stru *mac_vap = NULL;

    for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
        mac_vap = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
        if (oal_unlikely(mac_vap == NULL)) {
            oam_warning_log1(0, OAM_SF_SCAN,
                             "mac_device_find_up_sta_wlan::vap is null! vap id is %d",
                             mac_device->auc_vap_id[vap_idx]);
            continue;
        }

        if ((mac_vap->en_vap_state == MAC_VAP_STATE_UP || mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE) &&
            (mac_vap->en_p2p_mode == WLAN_LEGACY_VAP_MODE)) {
            *ppst_mac_vap = mac_vap;

            return OAL_SUCC;
        }
    }

    *ppst_mac_vap = NULL;

    return OAL_FAIL;
}

void hmac_sta_rx_btwt(uint8_t *payload, uint16_t msg_len, hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    uint8_t *btwt_element = NULL;
    uint64_t twt_interval;
    uint32_t twt_wake_duration;
    mac_btwt_action_stru btwt_action_args;

    /* 不支持广播twt，直接返回 */
    if (g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_btwt_requester_support == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_11AX,
            "{hmac_sta_rx_twt_setup_frame::twt responder support is off in the ini file!.}");
        return;
    }

    /* 可用会话已满，返回 */
    if (hmac_vap->st_twt_cfg[0].uc_twt_session_enable == OAL_TRUE) {
        return;
    }

    /*************************************************************************/
    /*                         TWT Element Format                            */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Control | TWT parameter Information            */
    /* --------------------------------------------------------------------- */
    /* | 1         | 1      | 1       |      Variable             |          */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    /*************************************************************************/
    /*                 Broadcast TWT parameter Information                   */
    /* --------------------------------------------------------------------- */
    /* |Request Type | TWT | Min TWT duration | TWT Wake Interval | TWT Info */
    /* --------------------------------------------------------------------- */
    /* | 2           | 2   | 1                | 2                 |  2       */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    btwt_element = mac_find_ie(MAC_EID_TWT, payload, msg_len);
    if (btwt_element == NULL) {
        return;
    }

    if (memcpy_s((uint8_t *)&(btwt_action_args.btwt_ie), sizeof(mac_btwt_ie_stru), (uint8_t *)btwt_element,
        btwt_element[1] + MAC_IE_HDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_sta_rx_btwt::memcpy fail!");
        return;
    }

    oam_warning_log3(0, OAM_SF_11AX,
        "{hmac_sta_rx_btwt:: flow_id:%d, bit_flow_type:%d, bit_setup_command:%d}",
        btwt_action_args.btwt_ie.request_type.flow_id, btwt_action_args.btwt_ie.request_type.flow_type,
        btwt_action_args.btwt_ie.request_type.setup_command);

    /* 检查twt参数，非法参数不允许建立twt会话 */
    twt_interval =
        (uint64_t)btwt_action_args.btwt_ie.mantissa << btwt_action_args.btwt_ie.request_type.exponent;
    /* Nominal Minimum TWT Wake Duration, 单位是256us */
    twt_wake_duration = btwt_action_args.btwt_ie.min_duration * 256;
    if (twt_interval < twt_wake_duration) {
        oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_11AX,
            "{hmac_sta_rx_btwt::error twt interval [%d] < twt wake duration!.}", twt_interval, twt_wake_duration);
        return;
    }

    btwt_action_args.category = MAC_ACTION_CATEGORY_S1G;
    btwt_action_args.action = MAC_S1G_ACTION_TWT_SETUP;
    btwt_action_args.btwt_ie.ie_len = sizeof(mac_btwt_ie_stru) - MAC_IE_HDR_LEN;
    btwt_action_args.btwt_ie.request_type.setup_command = MAC_TWT_COMMAND_REQUEST;
    hmac_vap->st_twt_cfg[0].twt_type = 1;
    hmac_sta_twt_save_parameters(hmac_vap, (mac_itwt_ie_stru *)&(btwt_action_args.btwt_ie), 0);

    /* 在dmac发送完成中断收到ack后再设置twt参数 */
    if (hmac_mgmt_tx_twt_setup(hmac_vap, hmac_user, (mac_twt_action_mgmt_args_stru *)&btwt_action_args) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_sta_rx_btwt::hmac_mgmt_tx_twt_setup fail!");
    }
    return;
}

#endif /* _PRE_WLAN_FEATURE_TWT */
