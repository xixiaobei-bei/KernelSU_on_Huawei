/*
 * 版权所有 (c) 华为技术有限公司 2013-2020
 * 功能说明 : STA侧管理面处理
 * 作    者 : zhangheng
 * 创建日期 : 2013年6月18日
 */
#include "wlan_spec.h"
#include "wlan_mib.h"

#include "mac_frame.h"
#include "mac_ie.h"
#include "mac_regdomain.h"
#include "mac_user.h"
#include "mac_vap.h"

#include "mac_device.h"
#include "hmac_device.h"
#include "hmac_user.h"
#include "hmac_mgmt_sta_up.h"
#include "hmac_fsm.h"
#include "hmac_rx_data.h"
#include "hmac_chan_mgmt.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_sme_sta.h"
#include "hmac_scan.h"
#include "hmac_degradation.h"

#include "hmac_tx_amsdu.h"

#include "hmac_11i.h"

#include "hmac_protection.h"

#include "hmac_config.h"
#include "hmac_ext_if.h"
#include "hmac_p2p.h"

#include "hmac_mgmt_ap.h"

#include "hmac_blockack.h"
#include "frw_ext_if.h"

#include "hmac_roam_main.h"

#ifdef _PRE_WLAN_FEATURE_WAPI
#include "hmac_wapi.h"
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS
#include "hmac_smps.h"
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#include "hmac_opmode.h"
#endif

#ifdef _PRE_WLAN_FEATURE_WMMAC
#include "hmac_wmmac.h"
#endif

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
#include "hmac_11v.h"
#endif

#include "hmac_btcoex.h"

#ifdef _PRE_WLAN_FEATURE_SNIFFER
#ifdef CONFIG_HW_SNIFFER
#include <hwnet/ipv4/sysctl_sniffer.h>
#endif
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif

#ifdef _PRE_WLAN_FEATURE_HIEX
#include "hmac_hiex.h"
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
#include "hmac_wifi6_self_cure.h"
#include "hmac_11ax.h"
#endif
#include "hmac_ht_self_cure.h"
#include "securec.h"
#ifdef _PRE_WLAN_FEATURE_TWT
#include "hmac_twt.h"
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
#include "hmac_ftm.h"
#endif
#include "hmac_sae.h"
#include "mac_mib.h"
#include "hmac_11r.h"
#include "hmac_ota_report.h"
#include "hmac_11w.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID       OAM_FILE_ID_HMAC_MGMT_STA_UP_C

/* 获取帧体长度 */
uint32_t hmac_get_frame_body_len(oal_netbuf_stru *net_buf)
{
    dmac_rx_ctl_stru *rx_ctrl = NULL;
    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(net_buf);
    if (rx_ctrl == NULL) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_get_frame_body_len::get null.}");
        return 0;
    }
    if (rx_ctrl->st_rx_info.us_frame_len < rx_ctrl->st_rx_info.uc_mac_header_len) {
        oam_error_log1(0, OAM_SF_ASSOC, "{hmac_get_frame_body_len::us_frame_len%d.}", rx_ctrl->st_rx_info.us_frame_len);
        return 0;
    }

    return rx_ctrl->st_rx_info.us_frame_len - rx_ctrl->st_rx_info.uc_mac_header_len;
}



uint32_t hmac_sta_up_update_edca_params_machw(hmac_vap_stru *pst_hmac_sta, mac_wmm_set_param_type_enum_uint8 en_type)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event = NULL;
    dmac_ctx_sta_asoc_set_edca_reg_stru st_asoc_set_edca_reg_param = { 0 };

    /* 抛事件到dmac写寄存器 */
    /* 申请事件内存 */
    pst_event_mem = frw_event_alloc_m(sizeof(dmac_ctx_sta_asoc_set_edca_reg_stru));
    if (pst_event_mem == NULL) {
        oam_error_log1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_update_vht_opern_ie_sta::event_mem alloc null, size[%d].}",
                       sizeof(dmac_ctx_sta_asoc_set_edca_reg_stru));
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_asoc_set_edca_reg_param.uc_vap_id = pst_hmac_sta->st_vap_base_info.uc_vap_id;
    st_asoc_set_edca_reg_param.en_set_param_type = en_type;

    /* 填写事件 */
    pst_event = frw_get_event_stru(pst_event_mem);
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_EDCA_REG,
                       sizeof(dmac_ctx_sta_asoc_set_edca_reg_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_hmac_sta->st_vap_base_info.uc_chip_id,
                       pst_hmac_sta->st_vap_base_info.uc_device_id,
                       pst_hmac_sta->st_vap_base_info.uc_vap_id);

    if (en_type != MAC_WMM_SET_PARAM_TYPE_DEFAULT) {
        memcpy_s((uint8_t *)&st_asoc_set_edca_reg_param.ast_wlan_mib_qap_edac,
                 (sizeof(wlan_mib_Dot11QAPEDCAEntry_stru) * WLAN_WME_AC_BUTT),
                 (uint8_t *)&pst_hmac_sta->st_vap_base_info.pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac,
                 (sizeof(wlan_mib_Dot11QAPEDCAEntry_stru) * WLAN_WME_AC_BUTT));
    }

    /* 拷贝参数 */
    if (memcpy_s(frw_get_event_payload(pst_event_mem), sizeof(dmac_ctx_sta_asoc_set_edca_reg_stru),
        (uint8_t *)&st_asoc_set_edca_reg_param, sizeof(dmac_ctx_sta_asoc_set_edca_reg_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_sta_up_update_edca_params_machw::memcpy fail!");
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);

    return OAL_SUCC;
}


OAL_STATIC void hmac_sta_up_update_edca_params_mib(hmac_vap_stru *pst_hmac_sta, uint8_t *puc_payload)
{
    uint8_t uc_aifsn;
    uint8_t uc_aci;
    uint8_t uc_ecwmin;
    uint8_t uc_ecwmax;
    uint16_t us_txop_limit;
    oal_bool_enum_uint8 en_acm;
    /*         AC Parameters Record Format        */
    /* ------------------------------------------ */
    /* |     1     |       1       |      2     | */
    /* ------------------------------------------ */
    /* | ACI/AIFSN | ECWmin/ECWmax | TXOP Limit | */
    /* ------------------------------------------ */
    /************* ACI/AIFSN Field ***************/
    /*     ---------------------------------- */
    /* bit |   4   |  1  |  2  |    1     |   */
    /*     ---------------------------------- */
    /*     | AIFSN | ACM | ACI | Reserved |   */
    /*     ---------------------------------- */
    uc_aifsn = puc_payload[0] & MAC_WMM_QOS_PARAM_AIFSN_MASK;
    en_acm = (puc_payload[0] & BIT4) ? OAL_TRUE : OAL_FALSE;
    uc_aci = (puc_payload[0] >> MAC_WMM_QOS_PARAM_ACI_BIT_OFFSET) & MAC_WMM_QOS_PARAM_ACI_MASK;

    /* ECWmin/ECWmax Field */
    /*     ------------------- */
    /* bit |   4    |   4    | */
    /*     ------------------- */
    /*     | ECWmin | ECWmax | */
    /*     ------------------- */
    uc_ecwmin = (puc_payload[1] & MAC_WMM_QOS_PARAM_ECWMIN_MASK);
    uc_ecwmax = ((puc_payload[1] & MAC_WMM_QOS_PARAM_ECWMAX_MASK) >> MAC_WMM_QOS_PARAM_ECWMAX_BIT_OFFSET);

    /* 在mib库中和寄存器里保存的TXOP值都是以us为单位的，但是传输的时候是以32us为
       单位进行传输的，因此在解析的时候需要将解析到的值乘以32
    */
    us_txop_limit = puc_payload[BYTE_OFFSET_2] |
        ((puc_payload[BYTE_OFFSET_3] & MAC_WMM_QOS_PARAM_TXOPLIMIT_MASK) << MAC_WMM_QOS_PARAM_BIT_NUMS_OF_ONE_BYTE);
    us_txop_limit = (uint16_t)(us_txop_limit << MAC_WMM_QOS_PARAM_TXOPLIMIT_SAVE_TO_TRANS_TIMES);

    /* 更新相应的MIB库信息 */
    if (uc_aci < WLAN_WME_AC_BUTT) {
        /* 注: 协议规定取值1 2 3 4 */
        mac_mib_set_QAPEDCATableIndex(&pst_hmac_sta->st_vap_base_info, uc_aci, uc_aci + 1);
        mac_mib_set_QAPEDCATableCWmin(&pst_hmac_sta->st_vap_base_info, uc_aci, uc_ecwmin);
        mac_mib_set_QAPEDCATableCWmax(&pst_hmac_sta->st_vap_base_info, uc_aci, uc_ecwmax);
        mac_mib_set_QAPEDCATableAIFSN(&pst_hmac_sta->st_vap_base_info, uc_aci, uc_aifsn);
        mac_mib_set_QAPEDCATableTXOPLimit(&pst_hmac_sta->st_vap_base_info, uc_aci, us_txop_limit);
        mac_mib_set_QAPEDCATableMandatory(&pst_hmac_sta->st_vap_base_info, uc_aci, en_acm);
    }
}

#ifdef _PRE_WLAN_FEATURE_WMMAC
/* 保存ts相关信息 */
static uint32_t hmac_set_ts_info(hmac_user_stru *hmac_user, hmac_vap_stru *hmac_sta,
    oal_bool_enum_uint8 *wmmac_auth_flag, uint8_t frame_sub_type, uint8_t ac_num_loop)
{
    if (OAL_TRUE == mac_mib_get_QAPEDCATableMandatory(&(hmac_sta->st_vap_base_info), ac_num_loop)) {
        *wmmac_auth_flag = (hmac_sta->en_wmmac_auth_flag && frame_sub_type == WLAN_FC0_SUBTYPE_REASSOC_RSP &&
            hmac_user->st_user_base_info.st_ts_info[ac_num_loop].en_ts_status == MAC_TS_SUCCESS);
        if (*wmmac_auth_flag) {
            return OAL_FAIL;
        }
        hmac_user->st_user_base_info.st_ts_info[ac_num_loop].en_ts_status = MAC_TS_INIT;
        hmac_user->st_user_base_info.st_ts_info[ac_num_loop].uc_tsid = 0xFF;
    } else {
        hmac_user->st_user_base_info.st_ts_info[ac_num_loop].en_ts_status = MAC_TS_NONE;
        hmac_user->st_user_base_info.st_ts_info[ac_num_loop].uc_tsid = 0xFF;
    }
    return OAL_SUCC;
}
#endif

static void hmac_sta_up_find_edca_ie_params(uint8_t *puc_payload, hmac_vap_stru *hmac_sta,
    mac_device_stru *mac_device, uint16_t msg_len, uint8_t frame_sub_type)
{
    uint32_t ret;
    uint8_t *puc_ie = mac_find_ie(MAC_EID_HT_CAP, puc_payload, msg_len);
    if (puc_ie != NULL) {
        /* 再查找HT CAP能力第2字节BIT 5 short GI for 20M 能力位 */
        if ((puc_ie[1] >= 2) && (puc_ie[2] & BIT5)) {
            mac_vap_init_wme_param(&hmac_sta->st_vap_base_info);
            mac_device->en_wmm = OAL_TRUE;
            hcc_host_update_vi_flowctl_param(mac_mib_get_QAPEDCATableCWmin(&hmac_sta->st_vap_base_info, WLAN_WME_AC_BE),
                mac_mib_get_QAPEDCATableCWmin(&hmac_sta->st_vap_base_info, WLAN_WME_AC_VI));
            /* 更新EDCA相关的MAC寄存器 */
            hmac_sta_up_update_edca_params_machw(hmac_sta, MAC_WMM_SET_PARAM_TYPE_UPDATE_EDCA);

            return;
        }
    }

    if (frame_sub_type == WLAN_FC0_SUBTYPE_ASSOC_RSP) {
        /* 当与STA关联的AP不是QoS的，STA会去使能EDCA寄存器，并默认利用VO级别发送数据 */
        ret = hmac_sta_up_update_edca_params_machw(hmac_sta, MAC_WMM_SET_PARAM_TYPE_DEFAULT);
        if (ret != OAL_SUCC) {
            oam_warning_log1(hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                "{hmac_sta_up_find_edca_ie_params::hmac_sta_up_update_edca_params_machw failed[%d].}", ret);
        }

        mac_device->en_wmm = OAL_FALSE;
    }
}

static uint32_t hmac_sta_up_get_edca_params(uint8_t *ie, uint8_t *edca_param_set, uint8_t *param_set_cnt,
    hmac_vap_stru *hmac_sta, uint8_t frame_sub_type)
{
    /* ie[1]的长度是不包括自己和之前字节的长度总长 */
    if (ie[1] < HMAC_WMM_QOS_PARAMS_HDR_LEN) {
        oam_error_log1(0, OAM_SF_ASSOC, "{hmac_sta_up_get_edca_params::ie len error[%d]!}", ie[1]);
        return OAL_FAIL;
    }
    /* 解析wmm ie是否携带EDCA参数 */
    *edca_param_set = ie[MAC_OUISUBTYPE_WMM_PARAM_OFFSET];
    *param_set_cnt = ie[HMAC_WMM_QOS_PARAMS_HDR_LEN] & 0x0F;

    /* 如果收到的是beacon帧，并且param_set_count没有改变，说明AP的WMM参数没有变
       则STA也不用做任何改变，直接返回即可 */
    if ((WLAN_FC0_SUBTYPE_BEACON == frame_sub_type)
        && (*param_set_cnt == hmac_sta->st_vap_base_info.uc_wmm_params_update_count)) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


void hmac_sta_up_update_edca_params(uint8_t *payload, uint16_t msg_len, hmac_vap_stru *hmac_sta,
    uint8_t frame_sub_type, hmac_user_stru *hmac_user)
{
    uint16_t msg_offset;
    uint8_t param_set_cnt, num_loop, edca_param_set;
    mac_device_stru *mac_device;
    uint8_t *ie = NULL;
#ifdef _PRE_WLAN_FEATURE_WMMAC
    oal_bool_enum_uint8 wmmac_auth_flag;
#endif

    mac_device = (mac_device_stru *)mac_res_get_dev(hmac_sta->st_vap_base_info.uc_device_id);
    if (mac_device == NULL) {
        oam_warning_log1(0, OAM_SF_ASSOC, "{sta up:fail.device_id[%u]!}", hmac_sta->st_vap_base_info.uc_device_id);
        return;
    }

    /************************ WMM Parameter Element ***************************/
    /* ------------------------------------------------------------------------------ */
    /* | EID | LEN | OUI |OUI Type |OUI Subtype |Version |QoS Info |Resd |AC Params | */
    /* ------------------------------------------------------------------------------ */
    /* |  1  |  1  |  3  |    1    |     1      |    1   |    1    |  1  |    16    | */
    /* ------------------------------------------------------------------------------ */
    /******************* QoS Info field when sent from WMM AP *****************/
    /*        --------------------------------------------                    */
    /*          | Parameter Set Count | Reserved | U-APSD |                   */
    /*          --------------------------------------------                  */
    /*   bit    |        0~3          |   4~6    |   7    |                   */
    /*          --------------------------------------------                  */
    /**************************************************************************/
    ie = mac_get_wmm_ie(payload, msg_len);
    if (ie != NULL) {
        if (hmac_sta_up_get_edca_params(ie, &edca_param_set, &param_set_cnt, hmac_sta, frame_sub_type) != OAL_SUCC) {
            return;
        }

        mac_device->en_wmm = OAL_TRUE;

        if (WLAN_FC0_SUBTYPE_BEACON == frame_sub_type) {
            /* 保存QoS Info */
            mac_vap_set_wmm_params_update_count(&hmac_sta->st_vap_base_info, param_set_cnt);
        }

        if (ie[HMAC_WMM_QOS_PARAMS_HDR_LEN] & BIT7) {
            mac_user_set_apsd(&(hmac_user->st_user_base_info), OAL_TRUE);
        } else {
            mac_user_set_apsd(&(hmac_user->st_user_base_info), OAL_FALSE);
        }

        msg_offset = (HMAC_WMM_QOSINFO_AND_RESV_LEN + HMAC_WMM_QOS_PARAMS_HDR_LEN);

        /* wmm ie中不携带edca参数 直接返回 */
        if (edca_param_set != MAC_OUISUBTYPE_WMM_PARAM ||
            ie[1] < (HMAC_WMM_QOS_PARAMS_HDR_LEN + WLAN_WME_AC_BUTT * HMAC_WMM_AC_PARAMS_RECORD_LEN)) {
            return;
        }

        /* 针对每一个AC，更新EDCA参数 */
        for (num_loop = 0; num_loop < WLAN_WME_AC_BUTT; num_loop++) {
            hmac_sta_up_update_edca_params_mib(hmac_sta, &ie[msg_offset]);
#ifdef _PRE_WLAN_FEATURE_WMMAC
            if (hmac_set_ts_info(hmac_user, hmac_sta, &wmmac_auth_flag, frame_sub_type, num_loop) != OAL_SUCC) {
                return;
            }
#endif  // _PRE_WLAN_FEATURE_WMMAC
            msg_offset += HMAC_WMM_AC_PARAMS_RECORD_LEN;
        }

        hcc_host_update_vi_flowctl_param(mac_mib_get_QAPEDCATableCWmin(&hmac_sta->st_vap_base_info, WLAN_WME_AC_BE),
            mac_mib_get_QAPEDCATableCWmin(&hmac_sta->st_vap_base_info, WLAN_WME_AC_VI));

        /* 更新EDCA相关的MAC寄存器 */
        hmac_sta_up_update_edca_params_machw(hmac_sta, MAC_WMM_SET_PARAM_TYPE_UPDATE_EDCA);

        return;
    }
    hmac_sta_up_find_edca_ie_params(payload, hmac_sta, mac_device, msg_len, frame_sub_type);
}


void hmac_sta_handle_disassoc_rsp(hmac_vap_stru *hmac_vap, hmac_disconnect *disconnect)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    hmac_disconnect *h2w_disconnect = NULL;

    /* 通告vap sta去关联 */
    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        oal_notice_sta_join_result(hmac_vap->st_vap_base_info.uc_chip_id, OAL_FALSE);
    }

    /* 抛加入完成事件到WAL */
    event_mem = frw_event_alloc_m(sizeof(hmac_disconnect));
    if (event_mem == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{hmac_sta_handle_disassoc_rsp::event_mem null.}");
        return;
    }

    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_DISASOC_COMP_STA,
                       sizeof(hmac_disconnect),
                       FRW_EVENT_PIPELINE_STAGE_0,
                       hmac_vap->st_vap_base_info.uc_chip_id,
                       hmac_vap->st_vap_base_info.uc_device_id,
                       hmac_vap->st_vap_base_info.uc_vap_id);

    h2w_disconnect = (hmac_disconnect *)(event->auc_event_data);
    h2w_disconnect->reason_code = disconnect->reason_code;
    h2w_disconnect->type = disconnect->type;
    h2w_disconnect->is_roam = disconnect->is_roam;

    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);

    return;
}

static void hmac_sta_rx_deauth_req_debug(uint8_t vap_id, uint8_t *mac_hdr)
{
    uint8_t *sa = NULL;

    mac_rx_get_sa((mac_ieee80211_frame_stru *)mac_hdr, &sa);
    oam_warning_log4(vap_id, OAM_SF_AUTH, "{hmac_sta_rx_deauth_req::\
        Because of err_code[%d], received deauth or disassoc frame frome source addr, sa xx:xx:xx:%2x:%2x:%2x.}",
        *((uint16_t *)(mac_hdr + MAC_80211_FRAME_LEN)), sa[BYTE_OFFSET_3], sa[BYTE_OFFSET_4], sa[BYTE_OFFSET_5]);
    return;
}

static void hmac_sta_handle_disassoc_rsp_help(hmac_vap_stru *hmac_vap, uint8_t *mac_hdr)
{
    hmac_disconnect disconnect;

    disconnect.reason_code = *((uint16_t *)(mac_hdr + MAC_80211_FRAME_LEN));
    disconnect.type = mac_get_frame_sub_type(mac_hdr);
    disconnect.is_roam = OAL_FALSE;
    /* sta上报和ap去关联 */
    hmac_sta_handle_disassoc_rsp(hmac_vap, &disconnect);
    return;
}

static void hmac_sta_rx_deauth_req_usr_null_proc(hmac_vap_stru *hmac_vap, uint8_t *mac_hdr, uint16_t usr_idx)
{
    uint8_t bssid[WLAN_MAC_ADDR_LEN] = { 0 };

    mac_get_address2(mac_hdr, bssid, WLAN_MAC_ADDR_LEN);
    oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
        "{hmac_sta_rx_deauth_req::pst_hmac_user_vap[%d] null.}", usr_idx);
    /* 没有查到对应的USER,发送去认证消息 */
    hmac_mgmt_send_deauth_frame(&(hmac_vap->st_vap_base_info), bssid, MAC_NOT_AUTHED, OAL_FALSE);
    hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
    hmac_sta_handle_disassoc_rsp_help(hmac_vap, mac_hdr);
    return;
}


static uint32_t hmac_sta_rx_deauth_req(hmac_vap_stru *hmac_vap, uint8_t *mac_hdr,
    oal_bool_enum_uint8 is_protected)
{
    uint8_t bssid[WLAN_MAC_ADDR_LEN] = { 0 };
    hmac_user_stru *hmac_user = NULL;
    uint16_t usr_idx = 0xffff;
    uint32_t ret, ret_del_user;
    uint8_t *da = NULL;
    uint8_t vap_id;

    if (oal_any_null_ptr2(hmac_vap, mac_hdr)) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_sta_rx_deauth_req:input params null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    vap_id = hmac_vap->st_vap_base_info.uc_vap_id;

    /* 接收到去认证帧或者去关联帧时的维测信息 */
    hmac_sta_rx_deauth_req_debug(vap_id, mac_hdr);

    mac_get_address2(mac_hdr, bssid, WLAN_MAC_ADDR_LEN);
    ret = mac_vap_find_user_by_macaddr(&hmac_vap->st_vap_base_info, bssid, &usr_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(vap_id, OAM_SF_AUTH, "{hmac_sta_rx_deauth_req::find user failed[%d],other bss deauth}", ret);
        return ret;
    }
    hmac_user = mac_res_get_hmac_user(usr_idx);
    if (hmac_user == NULL) {
        hmac_sta_rx_deauth_req_usr_null_proc(hmac_vap, mac_hdr, usr_idx);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_PMF
    /* 检查是否需要发送SA query request */
    if ((hmac_user->st_user_base_info.en_user_asoc_state == MAC_USER_STATE_ASSOC) &&
        (hmac_pmf_check_err_code(&hmac_user->st_user_base_info, is_protected, mac_hdr) == OAL_SUCC)) {
        /* 在关联状态下收到未加密的ReasonCode 6/7需要启动SA Query流程 */
        ret = hmac_start_sa_query(&hmac_vap->st_vap_base_info, hmac_user,
            hmac_user->st_user_base_info.st_cap_info.bit_pmf_active);
        if (ret != OAL_SUCC) {
            return OAL_ERR_CODE_PMF_SA_QUERY_START_FAIL;
        }
        return OAL_SUCC;
    }
#endif

    /* 如果该用户的管理帧加密属性不一致，丢弃该报文 */
    mac_rx_get_da((mac_ieee80211_frame_stru *)mac_hdr, &da);
    if ((ether_is_multicast(da) != OAL_TRUE) &&
        (is_protected != hmac_user->st_user_base_info.st_cap_info.bit_pmf_active)) {
        oam_error_log0(vap_id, OAM_SF_AUTH, "{hmac_sta_rx_deauth_req::PMF check failed.}");
        return OAL_FAIL;
    }
    /* 删除user */
    ret_del_user = hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);
    if (ret_del_user != OAL_SUCC) {
        oam_error_log0(vap_id, OAM_SF_AUTH, "{hmac_sta_rx_deauth_req::hmac_user_del failed.}");
        hmac_sta_handle_disassoc_rsp_help(hmac_vap, mac_hdr);
        return OAL_FAIL;
    }

    hmac_sta_handle_disassoc_rsp_help(hmac_vap, mac_hdr);
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_sta_up_rx_beacon
 * 功能描述  : sta up状态接收beacon帧处理
 * 修改历史  :
 * 1.日    期  : 2013年8月27日
 *   作    者  : zhangheng
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_sta_up_rx_beacon(hmac_vap_stru *pst_hmac_vap_sta, oal_netbuf_stru *pst_netbuf)
{
    dmac_rx_ctl_stru *pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    mac_rx_ctl_stru *pst_rx_info = (mac_rx_ctl_stru *)(&(pst_rx_ctrl->st_rx_info));
    mac_ieee80211_frame_stru *pst_mac_hdr = NULL;
    uint32_t ret;
    uint16_t us_frame_len, us_frame_offset, us_user_idx;
    uint8_t *puc_frame_body = NULL;
    uint8_t uc_frame_sub_type;
    hmac_user_stru *pst_hmac_user = NULL;
    uint8_t auc_addr_sa[WLAN_MAC_ADDR_LEN];

    pst_mac_hdr = (mac_ieee80211_frame_stru *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_info);
    puc_frame_body = (uint8_t *)pst_mac_hdr + pst_rx_info->uc_mac_header_len;
    us_frame_len = pst_rx_info->us_frame_len - pst_rx_info->uc_mac_header_len; /* 帧体长度 */
    us_frame_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    if (us_frame_len <= us_frame_offset) {
        oam_warning_log1(0, OAM_SF_RX, "{hmac_sta_up_rx_beacon::frame too short, len[%d].}", us_frame_len);
        return OAL_FAIL;
    }

    uc_frame_sub_type = mac_get_frame_sub_type((uint8_t *)pst_mac_hdr);

    /* 来自其它bss的Beacon不做处理 */
    if (oal_compare_mac_addr(pst_hmac_vap_sta->st_vap_base_info.auc_bssid, pst_mac_hdr->auc_address3) != 0) {
        return OAL_SUCC;
    }

    /* 获取管理帧的源地址SA */
    mac_get_address2((uint8_t *)pst_mac_hdr, auc_addr_sa, WLAN_MAC_ADDR_LEN);

    /* 根据SA 地地找到对应AP USER结构 */
    ret = mac_vap_find_user_by_macaddr(&(pst_hmac_vap_sta->st_vap_base_info), auc_addr_sa, &us_user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_hmac_vap_sta->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                         "{hmac_sta_up_rx_beacon::mac_vap_find_user_by_macaddr failed[%d].}", ret);
        return ret;
    }
    pst_hmac_user = mac_res_get_hmac_user(us_user_idx);
    if (pst_hmac_user == NULL) {
        oam_error_log1(pst_hmac_vap_sta->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                       "{hmac_sta_up_rx_beacon::pst_hmac_user[%d] null.}", us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 更新edca参数 */
    hmac_sta_up_update_edca_params(puc_frame_body + us_frame_offset, us_frame_len - us_frame_offset,
        pst_hmac_vap_sta, uc_frame_sub_type, pst_hmac_user);

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        hmac_sta_up_update_he_edca_params(puc_frame_body + us_frame_offset, us_frame_len - us_frame_offset,
                                          pst_hmac_vap_sta, uc_frame_sub_type, pst_hmac_user);
        hmac_sta_up_update_he_nfrp_params(puc_frame_body + us_frame_offset,
            us_frame_len - us_frame_offset, pst_hmac_vap_sta, uc_frame_sub_type, pst_hmac_user);
    }
#endif
#ifdef _PRE_WLAN_FEATURE_TWT
    hmac_sta_rx_btwt(puc_frame_body + us_frame_offset, us_frame_len - us_frame_offset, pst_hmac_vap_sta, pst_hmac_user);
#endif
    return OAL_SUCC;
}

static void hmac_sta_up_rx_action_nonuser_vendor_specific(uint8_t *puc_data, hmac_vap_stru *pst_hmac_vap,
    oal_netbuf_stru *pst_netbuf)
{
    /* Action */
    switch (puc_data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_PUB_VENDOR_SPECIFIC: {
#if defined(_PRE_WLAN_FEATURE_LOCATION) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
            if (oal_memcmp(puc_data + MAC_ACTION_CATEGORY_AND_CODE_LEN, g_auc_huawei_oui, MAC_OUI_LEN) == 0) {
            oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                "{hmac_sta_up_rx_action_nonuser::hmac location get.}");
            hmac_huawei_action_process(pst_hmac_vap, pst_netbuf,
                puc_data[MAC_ACTION_CATEGORY_AND_CODE_LEN + MAC_OUI_LEN]);
            }
#endif
            break;
        }
        default:
            break;
    }
}

/*
 * 函 数 名  : hmac_sta_up_rx_action_nonuser
 * 功能描述  : STA在UP状态下的接收ACTION帧处理
 * 修改历史  :
 * 1.日    期  : 2014年2月21日
 *   作    者  : mayuan
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_sta_up_rx_action_nonuser(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    dmac_rx_ctl_stru *pst_rx_ctrl;
    uint8_t *puc_data;
    mac_ieee80211_frame_stru *pst_frame_hdr; /* 保存mac帧的指针 */

    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    /* 获取帧头信息 */
    pst_frame_hdr = (mac_ieee80211_frame_stru *)MAC_GET_RX_CB_MAC_HEADER_ADDR(&pst_rx_ctrl->st_rx_info);

    /* 获取帧体指针 */
    puc_data = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(&pst_rx_ctrl->st_rx_info) +
        pst_rx_ctrl->st_rx_info.uc_mac_header_len;

    /* Category */
    switch (puc_data[MAC_ACTION_OFFSET_CATEGORY]) {
        case MAC_ACTION_CATEGORY_PUBLIC: {
            hmac_sta_up_rx_action_nonuser_vendor_specific(puc_data, pst_hmac_vap, pst_netbuf);
        }
        break;

        default:
            break;
    }
    return;
}


void hmac_sta_rx_radio_measurment(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user,
    oal_netbuf_stru *pst_netbuf, uint8_t *puc_data)
{
    if (pst_hmac_vap->bit_11k_enable == OAL_FALSE) {
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
            "{hmac_sta_rx_radio_measurment::11k is not enable,do not process this action frame.}");
        return;
    }

    switch (puc_data[MAC_ACTION_OFFSET_ACTION]) {
#ifdef _PRE_WLAN_FEATURE_11K
        case MAC_RM_ACTION_NEIGHBOR_REPORT_RESPONSE:
            hmac_roam_rx_neighbor_response_action(pst_hmac_vap, pst_netbuf);
            break;
#endif  // _PRE_WLAN_FEATURE_11K
        case MAC_RM_ACTION_RADIO_MEASUREMENT_REQUEST:
                wlan_chip_rrm_proc_rm_request(pst_hmac_vap, pst_netbuf);
                oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                                 "{hmac_sta_rx_radio_measurment::action code[%d] invalid.}",
                                 puc_data[MAC_ACTION_OFFSET_ACTION]);
            break;
        default:
            oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                             "{hmac_sta_rx_radio_measurment::action code[%d] invalid.}",
                             puc_data[MAC_ACTION_OFFSET_ACTION]);
            break;
    }

    return;
}
#ifdef _PRE_WLAN_FEATURE_PSM_ABN_PKTS_STAT
OAL_STATIC void hmac_sta_up_rx_addba_req_stat(hmac_vap_stru *pst_hmac_vap)
{
    mac_vap_stru *pst_mac_vap;
    hmac_device_stru *pst_hmac_device;

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    pst_hmac_device = (hmac_device_stru *)hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        return;
    }
    // 只统计wlan0并处于暗屏状态的数量
    if ((pst_hmac_device->pst_device_base_info->uc_in_suspend) && (mac_is_primary_legacy_sta(pst_mac_vap))) {
        pst_hmac_device->addba_req_cnt++;
    }
}
#endif


OAL_STATIC void hmac_sta_up_rx_action_ba(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    uint8_t *data, mac_ieee80211_frame_stru *frame_hdr, uint32_t frame_body_len)
{
    switch (data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_BA_ACTION_ADDBA_REQ:
            hmac_btcoex_check_rx_same_baw_start_from_addba_req(hmac_vap, hmac_user,
                frame_hdr, data, frame_body_len);
#ifdef _PRE_WLAN_FEATURE_PSM_ABN_PKTS_STAT
            hmac_sta_up_rx_addba_req_stat(hmac_vap);
#endif
            hmac_mgmt_rx_addba_req(hmac_vap, hmac_user, data, frame_body_len);
            break;
        case MAC_BA_ACTION_ADDBA_RSP:
            hmac_mgmt_rx_addba_rsp(hmac_vap, hmac_user, data, frame_body_len);
            break;
        case MAC_BA_ACTION_DELBA:
            hmac_mgmt_rx_delba(hmac_vap, hmac_user, data, frame_body_len);
            break;

        default:
            break;
    }
}

static uint32_t hmac_sta_up_rx_action_get_para(hmac_vap_stru *hmac_vap, oal_netbuf_stru *net_buf,
    mac_ieee80211_frame_stru **frame_hdr, hmac_user_stru **hmac_user, uint8_t **data)
{
    dmac_rx_ctl_stru *rx_ctrl = NULL;
    uint8_t *p2p0_mac_addr = NULL;

    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(net_buf);
    if (rx_ctrl->st_rx_info.us_frame_len - rx_ctrl->st_rx_info.uc_mac_header_len < MAC_ACTION_CATEGORY_AND_CODE_LEN) {
        oam_warning_log1(0, OAM_SF_RX, "{hmac_sta_up_rx_action::frame len too short[%d].}",
            rx_ctrl->st_rx_info.us_frame_len);
        return OAL_FAIL;
    }
    /* 获取帧头信息 */
    *frame_hdr = (mac_ieee80211_frame_stru *)MAC_GET_RX_CB_MAC_HEADER_ADDR(&rx_ctrl->st_rx_info);
    /* P2P0设备所接受的action全部上报 */
    p2p0_mac_addr = mac_mib_get_p2p0_dot11StationID(&hmac_vap->st_vap_base_info);
    if (oal_compare_mac_addr((*frame_hdr)->auc_address1, p2p0_mac_addr) == 0) {
        hmac_rx_mgmt_send_to_host(hmac_vap, net_buf);
    }

    /* 获取发送端的用户指针 */
    *hmac_user = mac_vap_get_hmac_user_by_addr(&hmac_vap->st_vap_base_info, (*frame_hdr)->auc_address2);
    if (*hmac_user == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX, "{hmac_sta_up_rx_action::hmac_user NULL.}");
        hmac_sta_up_rx_action_nonuser(hmac_vap, net_buf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取帧体指针 */
    *data = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(&rx_ctrl->st_rx_info) + rx_ctrl->st_rx_info.uc_mac_header_len;

    return OAL_SUCC;
}

static void hmac_sta_up_rx_action_wnm(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *net_buf,
    uint8_t *data)
{
    oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
        "{hmac_sta_up_rx_action::MAC_ACTION_CATEGORY_WNM action=%d.}", data[MAC_ACTION_OFFSET_ACTION]);
    switch (data[MAC_ACTION_OFFSET_ACTION]) {
#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
        /* bss transition request 帧处理入口 */
        case MAC_WNM_ACTION_BSS_TRANSITION_MGMT_REQUEST:
            hmac_rx_bsst_req_action(hmac_vap, hmac_user, net_buf);
            break;
#endif
        default:
#ifdef _PRE_WLAN_FEATURE_HS20
            /* 上报WNM Notification Request Action帧 */
            hmac_rx_mgmt_send_to_host(hmac_vap, net_buf);
#endif
            break;
    }

    return;
}

static void hmac_sta_up_rx_action_public(hmac_vap_stru *hmac_vap, oal_netbuf_stru *net_buf, uint8_t *data,
    uint32_t frame_body_len)
{
    switch (data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_PUB_VENDOR_SPECIFIC:
            if (frame_body_len <= MAC_ACTION_CATEGORY_AND_CODE_LEN + MAC_OUI_LEN) {
                oam_warning_log1(0, OAM_SF_RX, "{hmac_action_category_public::frame_body_len %d.}", frame_body_len);
                return;
            }
            /* 查找OUI-OUI type值为 50 6F 9A - 09 (WFA P2P v1.0)  */
            /* 并用hmac_rx_mgmt_send_to_host接口上报 */
            if (mac_ie_check_p2p_action(data + MAC_ACTION_CATEGORY_AND_CODE_LEN) == OAL_TRUE) {
                hmac_rx_mgmt_send_to_host(hmac_vap, net_buf);
            }
#if defined(_PRE_WLAN_FEATURE_LOCATION) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
            if (oal_memcmp(data + MAC_ACTION_CATEGORY_AND_CODE_LEN, g_auc_huawei_oui, MAC_OUI_LEN) == 0) {
                oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                    "{hmac_sta_up_rx_action::hmac location get.}");
                hmac_huawei_action_process(hmac_vap, net_buf,
                    data[MAC_ACTION_CATEGORY_AND_CODE_LEN + MAC_OUI_LEN]);
            }
#endif
            break;

        case MAC_PUB_GAS_INIT_RESP:
        case MAC_PUB_GAS_COMBAK_RESP:
#ifdef _PRE_WLAN_FEATURE_HS20
            /* 上报GAS查询的ACTION帧 */
            hmac_rx_mgmt_send_to_host(hmac_vap, net_buf);
#endif
            break;

        default:
            break;
    }

    return;
}

static void hmac_sta_up_rx_action_sa_query(hmac_vap_stru *hmac_vap, oal_netbuf_stru *net_buf, uint8_t *data,
    oal_bool_enum_uint8 is_protected)
{
    switch (data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_SA_QUERY_ACTION_REQUEST:
            hmac_rx_sa_query_req(hmac_vap, net_buf, is_protected);
            break;
        case MAC_SA_QUERY_ACTION_RESPONSE:
            hmac_rx_sa_query_rsp(hmac_vap, net_buf, is_protected);
            break;
        default:
            break;
    }

    return;
}

static void hmac_sta_up_rx_action_vht(hmac_vap_stru *hmac_vap, oal_netbuf_stru *net_buf, hmac_user_stru *hmac_user,
    uint8_t *data)
{
    switch (data[MAC_ACTION_OFFSET_ACTION]) {
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
        case MAC_VHT_ACTION_OPREATING_MODE_NOTIFICATION:
            hmac_mgmt_rx_opmode_notify_frame(hmac_vap, hmac_user, net_buf);
            break;
#endif
        default:
            break;
    }

    return;
}

static void hmac_sta_up_rx_action_s1g(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, uint8_t *data)
{
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        switch (data[MAC_ACTION_OFFSET_ACTION]) {
#ifdef _PRE_WLAN_FEATURE_TWT
            case MAC_S1G_ACTION_TWT_SETUP:
                hmac_sta_rx_twt_setup_frame(hmac_vap, hmac_user, data);
                break;

            case MAC_S1G_ACTION_TWT_TEARDOWN:
                hmac_sta_rx_twt_teardown_frame(hmac_vap, hmac_user, data);
                break;

            case MAC_S1G_ACTION_TWT_INFORMATION:
                hmac_sta_rx_twt_information_frame(hmac_vap, hmac_user, data);
                break;
#endif
            default:
                break;
        }
    }

    return;
}

static void hmac_sta_up_rx_action_extend(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *net_buf,
    uint8_t *data)
{
    switch (data[MAC_ACTION_OFFSET_CATEGORY]) {
        case MAC_ACTION_CATEGORY_VENDOR_SPECIFIC_PROTECTED:
            if (data[MAC_ACTION_OFFSET_ACTION] == MAC_HISHELL_ACTION_PASSWORD_CHANGE_REQUEST) {
                hmac_rx_mgmt_send_to_host(hmac_vap, net_buf);
            }
            break;

        case MAC_ACTION_CATEGORY_VENDOR:
            /* 查找OUI-OUI type值为 50 6F 9A - 09 (WFA P2P v1.0) */
            if (mac_ie_check_p2p_action(data + MAC_ACTION_CATEGORY_AND_CODE_LEN) == OAL_TRUE) {
                hmac_rx_mgmt_send_to_host(hmac_vap, net_buf);
            }
            break;

#ifdef _PRE_WLAN_FEATURE_11R
        case MAC_ACTION_CATEGORY_FAST_BSS_TRANSITION:
            if (hmac_vap->bit_11r_enable != OAL_TRUE) {
                break;
            }
            hmac_roam_rx_ft_action(hmac_vap, net_buf);
            break;
#endif  // _PRE_WLAN_FEATURE_11R

        case MAC_ACTION_CATEGORY_VHT:
            hmac_sta_up_rx_action_vht(hmac_vap, net_buf, hmac_user, data);
            break;

        case MAC_ACTION_CATEGORY_RADIO_MEASURMENT:
            hmac_sta_rx_radio_measurment(hmac_vap, hmac_user, net_buf, data);
            break;

#ifdef _PRE_WLAN_FEATURE_11AX
        case MAC_ACTION_CATEGORY_S1G:
            hmac_sta_up_rx_action_s1g(hmac_vap, hmac_user, data);
            break;
#endif

        default:
            break;
    }

    return;
}
/*
 * 函 数 名  : hmac_sta_up_rx_action
 * 功能描述  : STA在UP状态下的接收ACTION帧处理
 * 修改历史  :
 * 1.日    期  : 2014年3月12日
 *   作    者  : mayuan
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_sta_up_rx_action(hmac_vap_stru *hmac_vap, oal_netbuf_stru *net_buf,
    oal_bool_enum_uint8 is_protected)
{
    uint8_t *data = NULL;
    mac_ieee80211_frame_stru *frame_hdr = NULL; /* 保存mac帧的指针 */
    hmac_user_stru *hmac_user = NULL;
    uint32_t frame_body_len;

    if (hmac_sta_up_rx_action_get_para(hmac_vap, net_buf, &frame_hdr, &hmac_user, &data) != OAL_SUCC) {
        return;
    }
    frame_body_len = hmac_get_frame_body_len(net_buf);
    /* Category */
    switch (data[MAC_ACTION_OFFSET_CATEGORY]) {
        case MAC_ACTION_CATEGORY_BA:
            hmac_sta_up_rx_action_ba(hmac_vap, hmac_user, data, frame_hdr, frame_body_len);
            break;

        case MAC_ACTION_CATEGORY_WNM:
            hmac_sta_up_rx_action_wnm(hmac_vap, hmac_user, net_buf, data);
            break;

        case MAC_ACTION_CATEGORY_PUBLIC:
            hmac_sta_up_rx_action_public(hmac_vap, net_buf, data, frame_body_len);
            break;

#ifdef _PRE_WLAN_FEATURE_WMMAC
        case MAC_ACTION_CATEGORY_WMMAC_QOS:
            hmac_sta_up_rx_action_wmmac_qos_proc(hmac_vap, hmac_user, data, frame_body_len);
            break;
#endif  // _PRE_WLAN_FEATURE_WMMAC

#ifdef _PRE_WLAN_FEATURE_PMF
        case MAC_ACTION_CATEGORY_SA_QUERY:
            hmac_sta_up_rx_action_sa_query(hmac_vap, net_buf, data, is_protected);
            break;
#endif
        default:
            break;
    }

    hmac_sta_up_rx_action_extend(hmac_vap, hmac_user, net_buf, data);
}

OAL_STATIC void hmac_sta_up_rx_deauth_disassoc(hmac_vap_stru *pst_hmac_vap_sta,
    mac_rx_ctl_stru *pst_rx_info, uint8_t *puc_mac_hdr, oal_bool_enum_uint8 en_is_protected)
{
    if (pst_rx_info->us_frame_len < pst_rx_info->uc_mac_header_len + MAC_80211_REASON_CODE_LEN) {
        oam_warning_log1(pst_hmac_vap_sta->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                         "hmac_sta_up_rx_mgmt:: invalid deauth_req length[%d]}", pst_rx_info->us_frame_len);
    } else {
        hmac_sta_rx_deauth_req(pst_hmac_vap_sta, puc_mac_hdr, en_is_protected);
    }
}

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
static void hmac_sta_up_rx_mgmt_wakeup(hmac_vap_stru *hmac_vap, dmac_rx_ctl_stru *rx_ctrl, uint8_t *mac_hdr)
{
    uint8_t *data;

    if (wlan_pm_wkup_src_debug_get() == OAL_TRUE) {
        wlan_pm_wkup_src_debug_set(OAL_FALSE);
        oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                         "{wifi_wake_src:hmac_sta_up_rx_mgmt_wakeup::wakeup frame type[0x%x] sub type[0x%x]}",
                         mac_get_frame_type(mac_hdr), mac_get_frame_sub_type(mac_hdr));
        /* action帧唤醒时打印action帧类型 */
        if (mac_get_frame_type_and_subtype(mac_hdr) == (WLAN_FC0_SUBTYPE_ACTION | WLAN_FC0_TYPE_MGT)) {
            /* 获取帧体指针 */
            data = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(&rx_ctrl->st_rx_info) +
                rx_ctrl->st_rx_info.uc_mac_header_len;
            oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                "{wifi_wake_src:hmac_sta_up_rx_mgmt_wakeup::wakeup action category[0x%x], action details[0x%x]}",
                data[MAC_ACTION_OFFSET_CATEGORY], data[MAC_ACTION_OFFSET_ACTION]);
        }
    }

    return;
}
#endif

static void hmac_sta_up_rx_mgmt_core_proc(hmac_vap_stru *hmac_vap, mac_rx_ctl_stru *rx_info, uint8_t *mac_hdr,
    oal_bool_enum_uint8 is_protected, dmac_wlan_crx_event_stru *mgmt_rx_event)
{
    switch (mac_get_frame_sub_type(mac_hdr)) {
        case WLAN_FC0_SUBTYPE_DEAUTH:
        case WLAN_FC0_SUBTYPE_DISASSOC:
            hmac_sta_up_rx_deauth_disassoc(hmac_vap, rx_info, mac_hdr, is_protected);
            break;

        case WLAN_FC0_SUBTYPE_BEACON:
            hmac_sta_up_rx_beacon(hmac_vap, mgmt_rx_event->pst_netbuf);
            break;

        case WLAN_FC0_SUBTYPE_ACTION:
            hmac_sta_up_rx_action(hmac_vap, mgmt_rx_event->pst_netbuf, is_protected);
            break;
        default:
            break;
    }
    return;
}


uint32_t hmac_sta_up_rx_mgmt(hmac_vap_stru *hmac_vap, void *param)
{
    dmac_wlan_crx_event_stru *mgmt_rx_event = NULL;
    dmac_rx_ctl_stru *rx_ctrl = NULL;
    mac_rx_ctl_stru *rx_info = NULL;
    uint8_t *mac_hdr = NULL;
    oal_bool_enum_uint8 is_protected;

    if (oal_any_null_ptr2(hmac_vap, param)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_sta_up_rx_mgmt::hmac_vap or param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mgmt_rx_event = (dmac_wlan_crx_event_stru *)param;
    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(mgmt_rx_event->pst_netbuf);
    rx_info = (mac_rx_ctl_stru *)(&(rx_ctrl->st_rx_info));
    mac_hdr = (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(rx_info);
    is_protected = mac_is_protectedframe(mac_hdr);
    if (is_protected >= OAL_BUTT) {
        oam_warning_log1(0, OAM_SF_RX, "{hmac_sta_up_rx_mgmt::is_protected is %d.}", is_protected);
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    hmac_sta_up_rx_mgmt_wakeup(hmac_vap, rx_ctrl, mac_hdr);
#endif

    /* Bar frame proc here */
    if (mac_get_frame_type(mac_hdr) == WLAN_FC0_TYPE_CTL) {
        if (mac_get_frame_sub_type(mac_hdr) == WLAN_FC0_SUBTYPE_BAR) {
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#ifdef CONFIG_HW_SNIFFER
            proc_sniffer_write_file(NULL, 0, mac_hdr, rx_info->us_frame_len, 0);
#endif
#endif
            hmac_up_rx_bar(hmac_vap, rx_ctrl, mgmt_rx_event->pst_netbuf);
        }
    } else if (mac_get_frame_type(mac_hdr) == WLAN_FC0_TYPE_MGT) {
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#ifdef CONFIG_HW_SNIFFER
        proc_sniffer_write_file(NULL, 0, mac_hdr, rx_info->us_frame_len, 0);
#endif
#endif
        hmac_sta_up_rx_mgmt_core_proc(hmac_vap, rx_info, mac_hdr, is_protected, mgmt_rx_event);
    }

    return OAL_SUCC;
}
