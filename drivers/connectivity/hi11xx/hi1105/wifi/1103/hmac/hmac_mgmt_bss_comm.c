

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 头文件包含 */
#include "oal_kernel_file.h"
#include "wlan_spec.h"
#include "mac_vap.h"
#include "mac_ie.h"
#include "mac_frame.h"
#include "hmac_mgmt_bss_comm.h"
#include "mac_resource.h"
#include "hmac_device.h"
#include "hmac_resource.h"
#include "hmac_fsm.h"
#include "hmac_encap_frame.h"
#include "hmac_tx_amsdu.h"
#include "hmac_mgmt_ap.h"
#include "hmac_mgmt_sta.h"
#include "hmac_blockack.h"
#include "hmac_p2p.h"
#ifdef _PRE_WLAN_1103_CHR
#include "hmac_dfx.h"
#endif

#ifdef _PRE_WLAN_FEATURE_TWT
#include "hmac_twt.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MGMT_BSS_COMM_C

#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include <hwnet/ipv4/sysctl_sniffer.h>
#endif
#include "securec.h"
#include "securectype.h"

/* 2 全局变量定义 */
/*
 * 行:代表VAP 的协议能力
 * 例:代表USER 的协议能力
 */
#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
oal_uint8 g_auc_avail_protocol_mode_etc[WLAN_PROTOCOL_BUTT][WLAN_PROTOCOL_BUTT] = {
    /**
    --------11A--------------------------11B------------------11G---------------ONE_11G----------------TWO_11G--------------------HT------------------------VHT------------------HT_ONLY----------------VHT_ONLY----------HT_11G----------------HE
    **/
    { WLAN_LEGACY_11A_MODE, WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11A_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11A_MODE },                     // 11A
    { WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE,
      WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11B_MODE },                     // 11B
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE,
      WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE },                     // 11G
    { WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT },                       // ONE_11G
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_TWO_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT },                       // TWO_11G
    { WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_ONE_11G_MODE, WLAN_HT_MODE,      WLAN_HT_MODE,         WLAN_HT_ONLY_MODE,
      WLAN_PROTOCOL_BUTT,   WLAN_HT_11G_MODE,     WLAN_HT_MODE },                             // HT
    { WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_ONE_11G_MODE, WLAN_HT_MODE,      WLAN_VHT_MODE,        WLAN_HT_ONLY_MODE,
      WLAN_HT_ONLY_MODE,    WLAN_PROTOCOL_BUTT,   WLAN_VHT_MODE },                            // VHT
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_HT_ONLY_MODE,    WLAN_HT_ONLY_MODE,    WLAN_HT_ONLY_MODE,
      WLAN_HT_ONLY_MODE,    WLAN_HT_ONLY_MODE,    WLAN_HT_ONLY_MODE },                        // HT_ONLY
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_VHT_ONLY_MODE,   WLAN_PROTOCOL_BUTT,
      WLAN_VHT_ONLY_MODE,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT },                       // VHT_ONLY
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE,
      WLAN_LEGACY_11G_MODE, WLAN_HT_11G_MODE,     WLAN_PROTOCOL_BUTT,   WLAN_HT_ONLY_MODE,
      WLAN_PROTOCOL_BUTT,   WLAN_HT_11G_MODE,     WLAN_PROTOCOL_BUTT },                       // HT_11G
    { WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_ONE_11G_MODE, WLAN_HT_MODE,      WLAN_VHT_MODE,        WLAN_HT_ONLY_MODE,
      WLAN_VHT_ONLY_MODE,   WLAN_PROTOCOL_BUTT,   WLAN_HE_MODE },                             /* he */
};
#else
oal_uint8 g_auc_avail_protocol_mode_etc[WLAN_PROTOCOL_BUTT][WLAN_PROTOCOL_BUTT] = {
    { WLAN_LEGACY_11A_MODE, WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11A_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT },
    { WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE,
      WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11B_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT },
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE,
      WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE },
    { WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE },
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_TWO_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_MIXED_ONE_11G_MODE, WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE },
    { WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_ONE_11G_MODE, WLAN_HT_MODE,      WLAN_HT_MODE,         WLAN_HT_ONLY_MODE,
      WLAN_PROTOCOL_BUTT,   WLAN_HT_11G_MODE },
    { WLAN_LEGACY_11A_MODE, WLAN_LEGACY_11B_MODE, WLAN_LEGACY_11G_MODE, WLAN_MIXED_ONE_11G_MODE,
      WLAN_MIXED_ONE_11G_MODE, WLAN_HT_MODE,      WLAN_VHT_MODE,        WLAN_HT_ONLY_MODE,
      WLAN_VHT_ONLY_MODE,   WLAN_PROTOCOL_BUTT },
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_HT_ONLY_MODE,    WLAN_HT_ONLY_MODE,    WLAN_HT_ONLY_MODE,
      WLAN_HT_ONLY_MODE,    WLAN_HT_ONLY_MODE },
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,
      WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_VHT_ONLY_MODE,   WLAN_PROTOCOL_BUTT,
      WLAN_VHT_ONLY_MODE,   WLAN_PROTOCOL_BUTT },
    { WLAN_PROTOCOL_BUTT,   WLAN_PROTOCOL_BUTT,   WLAN_LEGACY_11G_MODE, WLAN_LEGACY_11G_MODE,
      WLAN_LEGACY_11G_MODE, WLAN_HT_11G_MODE,     WLAN_PROTOCOL_BUTT,   WLAN_HT_ONLY_MODE,
      WLAN_PROTOCOL_BUTT,   WLAN_HT_11G_MODE },
};
#endif

oal_uint32 hmac_mgmt_tx_addba_timeout_etc(oal_void *p_arg);

/* 3 函数实现 */

oal_void hmac_rx_ba_session_decr_etc(hmac_vap_stru *pst_hmac_vap, oal_uint8 uc_tidno)
{
    if (0 == mac_mib_get_RxBASessionNumber(&pst_hmac_vap->st_vap_base_info)) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_rx_ba_session_decr_etc::tid[%d] rx_session already zero.}", uc_tidno);
        return;
    }

    mac_mib_decr_RxBASessionNumber(&pst_hmac_vap->st_vap_base_info);

    OAM_WARNING_LOG2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                     "{hmac_rx_ba_session_decr_etc::tid[%d] tx_session[%d] remove.}",
                     uc_tidno, mac_mib_get_RxBASessionNumber(&pst_hmac_vap->st_vap_base_info));
}


oal_void hmac_tx_ba_session_decr_etc(hmac_vap_stru *pst_hmac_vap, oal_uint8 uc_tidno)
{
    if (0 == mac_mib_get_TxBASessionNumber(&pst_hmac_vap->st_vap_base_info)) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_tx_ba_session_decr_etc::tid[%d] tx_session already zero.}", uc_tidno);
        return;
    }

    mac_mib_decr_TxBASessionNumber(&pst_hmac_vap->st_vap_base_info);

    OAM_WARNING_LOG2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                     "{hmac_tx_ba_session_decr_etc::tid[%d] tx_session[%d] remove.}",
                     uc_tidno, mac_mib_get_TxBASessionNumber(&pst_hmac_vap->st_vap_base_info));
}

#ifdef _PRE_WLAN_FEATURE_TWT

oal_uint16 hmac_mgmt_encap_twt_setup_req_etc(hmac_vap_stru *pst_vap,
                                             hmac_user_stru *pst_hmac_user,
                                             oal_uint8 *puc_data,
                                             mac_twt_action_mgmt_args_stru *pst_twt_action_args)
{
    oal_uint16 us_index;
    mac_twt_element_ie_individual_stru pst_twt_element;
    if (OAL_ANY_NULL_PTR3(pst_vap, puc_data, pst_twt_action_args)) {
        OAM_ERROR_LOG3(0, OAM_SF_11AX,
                       "{hmac_mgmt_encap_twt_setup_req_etc::null param.vap:%x data:%x twt:%x}",
                       (uintptr_t)pst_vap, (uintptr_t)puc_data, (uintptr_t)pst_twt_action_args);
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s((oal_uint8 *)&pst_twt_element, OAL_SIZEOF(mac_twt_element_ie_individual_stru),
             0, OAL_SIZEOF(mac_twt_element_ie_individual_stru));
    OAM_WARNING_LOG3(0, OAM_SF_11AX, "{hmac_mgmt_encap_twt_setup_req_etc:: mac_addr[%02x XX XX XX %02x %02x]!.}",
                     pst_hmac_user->st_user_base_info.auc_user_mac_addr[0],
                     pst_hmac_user->st_user_base_info.auc_user_mac_addr[4],
                     pst_hmac_user->st_user_base_info.auc_user_mac_addr[5]);
    /******************************************************************************************************************/
    /*                   TWT element                                                                                  */
    /* -------------------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |Control |TWT Parameter Information|                                                       */
    /* -------------------------------------------------------------------------------------------------------------- */
    /* |1          |1      |1       |variable                 |                                                       */
    /* -------------------------------------------------------------------------------------------------------------- */
    /*                                                                                                                */
    /******************************************************************************************************************/
    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* Frame Control Field 中只需要设置Type/Subtype值，其他设置为0 */
    mac_hdr_set_frame_control(puc_data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* DA is address of STA requesting association */
    oal_set_mac_addr(puc_data + 4, pst_hmac_user->st_user_base_info.auc_user_mac_addr);

    /* SA的值为dot11MACAddress的值 */
    oal_set_mac_addr(puc_data + 10, mac_mib_get_StationID(&pst_vap->st_vap_base_info));

    oal_set_mac_addr(puc_data + 16, pst_vap->st_vap_base_info.auc_bssid);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /* 将索引指向frame body起始位置 */
    us_index = MAC_80211_FRAME_LEN;

    /******************************************************************************************************************/
    /*                   TWT argument                                                                                 */
    /* -------------------------------------------------------------------------------------------------------------- */
    /* |setup_cmd |flow_type |flow_ID |Reserve |                                                                      */
    /* -------------------------------------------------------------------------------------------------------------- */
    /* |1               |1             |1          |1           |                                                     */
    /* -------------------------------------------------------------------------------------------------------------- */
    /*                                                                                                                */
    /******************************************************************************************************************/
    pst_twt_element.uc_category = MAC_ACTION_CATEGORY_S1G;
    pst_twt_element.uc_action = MAC_S1G_ACTION_TWT_SETUP;
    pst_twt_element.uc_dialog_token = pst_vap->st_twt_cfg.uc_twt_dialog_token;
    pst_twt_element.uc_element_id = MAC_EID_TWT;
    pst_twt_element.uc_len = sizeof(mac_twt_element_ie_individual_stru) - 5;
    pst_twt_element.st_control.bit_ndp_paging_indicator = 0;
    pst_twt_element.st_control.bit_responder_pm_mode = 0;
    pst_twt_element.st_control.bit_negotiation = 0;
    pst_twt_element.st_control.bit_resv = 0;
    pst_twt_element.st_request_type.bit_request = 1; /* TODO:如果支持twt responder，回复setup帧时把该位清除 */
    pst_twt_element.st_request_type.bit_setup_command = pst_twt_action_args->uc_twt_setup_cmd; /* 由user决定 */
    /* 默认trigger-enabled TWT，与认证用例对齐，该位最终由AP决定 */
    pst_twt_element.st_request_type.bit_trigger = 1;
    pst_twt_element.st_request_type.bit_implicit = 1;
    /* 由user决定, annouced or unannounced */
    pst_twt_element.st_request_type.bit_flow_type = pst_twt_action_args->uc_twt_flow_type;
    pst_twt_element.st_request_type.bit_flow_id = pst_twt_action_args->uc_twt_flow_id;   /* 由user决定 */
    pst_twt_element.st_request_type.bit_exponent = pst_twt_action_args->ul_twt_exponent; /* unit: microsecond(us) */
    pst_twt_element.st_request_type.bit_protection = 0;
    /* To-Do,  user decide, unit:   0 or positive number(according to TSF, 由DMAC 填写 */
    pst_twt_element.ull_twt = pst_twt_action_args->ul_twt_start_time_offset;
    pst_twt_element.uc_min_duration = pst_twt_action_args->ul_twt_duration; /* user decide, unit: 256us */
    pst_twt_element.us_mantissa = pst_twt_action_args->ul_intrval_mantissa; /* user decide, unit: microsecond(us) */
    pst_twt_element.uc_channel = 0;
    OAM_WARNING_LOG4(0, OAM_SF_11AX,
        "{hmac_mgmt_encap_twt_setup_req_etc::entry bit_setup_command:%d bit_flow_type:%d bit_flow_id:%d trigger:%d}",
        pst_twt_element.st_request_type.bit_setup_command,
        pst_twt_element.st_request_type.bit_flow_type,
        pst_twt_element.st_request_type.bit_flow_id,
        pst_twt_element.st_request_type.bit_trigger);
    OAM_WARNING_LOG3(0, OAM_SF_11AX,
                     "{hmac_mgmt_encap_twt_setup_req_etc::entry ull_twt:%d uc_min_duration:%d us_mantissa:%d}",
                     pst_twt_element.ull_twt,
                     pst_twt_element.uc_min_duration,
                     pst_twt_element.us_mantissa);

    if (EOK != memcpy_s((oal_uint8 *)(puc_data + MAC_80211_FRAME_LEN), OAL_SIZEOF(mac_twt_element_ie_individual_stru),
                        (oal_uint8 *)&pst_twt_element, OAL_SIZEOF(mac_twt_element_ie_individual_stru))) {
        OAM_ERROR_LOG0(0, OAM_SF_11AX, "hmac_mgmt_encap_twt_setup_req_etc::memcpy fail!");
        return us_index;
    }
    us_index = MAC_80211_FRAME_LEN + OAL_SIZEOF(mac_twt_element_ie_individual_stru);
    /* 返回的帧长度中不包括FCS */
    return us_index;
}


oal_uint16 hmac_mgmt_encap_twt_teardown_req_etc(hmac_vap_stru *pst_vap,
                                                hmac_user_stru *pst_hmac_user,
                                                oal_uint8 *puc_data,
                                                mac_twt_action_mgmt_args_stru *pst_twt_action_args)
{
    oal_uint16 us_index;
    mac_twt_teardown_stru pst_twt_teardown;
    if (OAL_ANY_NULL_PTR3(pst_vap, puc_data, pst_twt_action_args)) {
        OAM_ERROR_LOG3(0, OAM_SF_11AX, "{hmac_mgmt_encap_twt_teardown_req_etc::null param.vap:%x data:%x twt:%x}",
                       (uintptr_t)pst_vap, (uintptr_t)puc_data, (uintptr_t)pst_twt_action_args);
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s((oal_uint8 *)&pst_twt_teardown, OAL_SIZEOF(mac_twt_teardown_stru), 0, OAL_SIZEOF(mac_twt_teardown_stru));
    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* Frame Control Field 中只需要设置Type/Subtype值，其他设置为0 */
    mac_hdr_set_frame_control(puc_data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* DA is address of STA requesting association */
    oal_set_mac_addr(puc_data + 4, pst_hmac_user->st_user_base_info.auc_user_mac_addr);

    /* SA的值为dot11MACAddress的值 */
    oal_set_mac_addr(puc_data + 10, mac_mib_get_StationID(&pst_vap->st_vap_base_info));

    oal_set_mac_addr(puc_data + 16, pst_vap->st_vap_base_info.auc_bssid);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /* 将索引指向frame body起始位置 */
    us_index = MAC_80211_FRAME_LEN;

    pst_twt_teardown.uc_category = MAC_ACTION_CATEGORY_S1G;
    pst_twt_teardown.uc_action = MAC_S1G_ACTION_TWT_TEARDOWN;
    pst_twt_teardown.bit_twt_flow_id = pst_twt_action_args->uc_twt_flow_id; /* 由user决定 */
    pst_twt_teardown.bit_nego_type = 0;

    if (EOK != memcpy_s((oal_uint8 *)(puc_data + MAC_80211_FRAME_LEN), OAL_SIZEOF(mac_twt_teardown_stru),
                        (oal_uint8 *)&pst_twt_teardown, OAL_SIZEOF(mac_twt_teardown_stru))) {
        OAM_ERROR_LOG0(0, OAM_SF_11AX, "hmac_mgmt_encap_twt_teardown_req_etc::memcpy fail!");
        return us_index;
    }
    us_index = MAC_80211_FRAME_LEN + OAL_SIZEOF(mac_twt_teardown_stru);
    /* 返回的帧长度中不包括FCS */
    return us_index;
}


oal_uint32 hmac_mgmt_tx_twt_setup_etc(hmac_vap_stru *pst_hmac_vap,
                                      hmac_user_stru *pst_hmac_user,
                                      mac_twt_action_mgmt_args_stru *pst_twt_action_args)
{
    mac_device_stru *pst_device;
    mac_vap_stru *pst_mac_vap;
    frw_event_mem_stru *pst_event_mem; /* 申请事件返回的内存指针 */
    oal_netbuf_stru *pst_twt_setup_req;
    oal_uint16 us_frame_len;
    frw_event_stru *pst_hmac_to_dmac_ctx_event;
    dmac_tx_event_stru *pst_tx_event;
    dmac_ctx_action_event_stru st_wlan_ctx_action;
    oal_uint32 ul_ret;
    mac_tx_ctl_stru *pst_tx_ctl;

    if (OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_hmac_user, pst_twt_action_args)) {
        OAM_ERROR_LOG3(0, OAM_SF_11AX, "{hmac_mgmt_tx_twt_setup_etc::null param, %x %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_hmac_user, (uintptr_t)pst_twt_action_args);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    /* 获取device结构 */
    pst_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (pst_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_11AX, "{hmac_mgmt_tx_twt_setup_etc::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请TWT SETUP管理帧内存 */
    pst_twt_setup_req = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_twt_setup_req == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_11AX, "{hmac_mgmt_tx_twt_setup_etc::pst_twt_setup_req null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEM_NETBUF_TRACE(pst_twt_setup_req, OAL_TRUE);

    OAL_NETBUF_PREV(pst_twt_setup_req) = OAL_PTR_NULL;
    OAL_NETBUF_NEXT(pst_twt_setup_req) = OAL_PTR_NULL;

    pst_hmac_vap->st_twt_cfg.uc_twt_dialog_token++;
    OAM_WARNING_LOG1(0, OAM_SF_11AX, "{hmac_mgmt_tx_twt_setup_etc::token, %d}",
                     pst_hmac_vap->st_twt_cfg.uc_twt_dialog_token);

    /* 调用封装管理帧接口 */
    us_frame_len = hmac_mgmt_encap_twt_setup_req_etc(pst_hmac_vap, pst_hmac_user,
                                                     oal_netbuf_data(pst_twt_setup_req),
                                                     pst_twt_action_args);

    memset_s((oal_uint8 *)&st_wlan_ctx_action, OAL_SIZEOF(st_wlan_ctx_action), 0, OAL_SIZEOF(st_wlan_ctx_action));

    /* 赋值要传入Dmac的信息 */
    st_wlan_ctx_action.us_frame_len = us_frame_len;
    st_wlan_ctx_action.uc_hdr_len = MAC_80211_FRAME_LEN;
    st_wlan_ctx_action.en_action_category = MAC_ACTION_CATEGORY_S1G;
    st_wlan_ctx_action.uc_action = MAC_S1G_ACTION_TWT_SETUP;
    st_wlan_ctx_action.us_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;

    if (EOK != memcpy_s((oal_uint8 *)(oal_netbuf_data(pst_twt_setup_req) + us_frame_len),
                        OAL_SIZEOF(dmac_ctx_action_event_stru),
                        (oal_uint8 *)&st_wlan_ctx_action,
                        OAL_SIZEOF(dmac_ctx_action_event_stru))) {
        OAM_ERROR_LOG0(0, OAM_SF_11AX, "hmac_mgmt_tx_twt_setup_etc::memcpy fail!");
        oal_netbuf_free(pst_twt_setup_req);
        return OAL_FAIL;
    }
    oal_netbuf_put(pst_twt_setup_req, (us_frame_len + OAL_SIZEOF(dmac_ctx_action_event_stru)));
    /* 初始化CB */
    memset_s(oal_netbuf_cb(pst_twt_setup_req), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_twt_setup_req);
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_frame_len + OAL_SIZEOF(dmac_ctx_action_event_stru);
    MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_ACTION;
    MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = WLAN_ACTION_TWT_SETUP_REQ; /* ??? */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_tx_event_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_11AX, "{hmac_mgmt_tx_twt_setup_etc::pst_event_mem null.}");
        oal_netbuf_free(pst_twt_setup_req);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_to_dmac_ctx_event = frw_get_event_stru(pst_event_mem);
    FRW_EVENT_HDR_INIT(&(pst_hmac_to_dmac_ctx_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT,
                       OAL_SIZEOF(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    pst_tx_event = (dmac_tx_event_stru *)(pst_hmac_to_dmac_ctx_event->auc_event_data);
    pst_tx_event->pst_netbuf = pst_twt_setup_req;
    pst_tx_event->us_frame_len = us_frame_len;

    ul_ret = frw_event_dispatch_event_etc(pst_event_mem);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
                       "{hmac_mgmt_tx_twt_setup_etc::frw_event_dispatch_event_etc failed[%d].}", ul_ret);
        oal_netbuf_free(pst_twt_setup_req);
        FRW_EVENT_FREE(pst_event_mem);
        return ul_ret;
    }

    FRW_EVENT_FREE(pst_event_mem);
    return OAL_SUCC;
}


oal_uint32 hmac_mgmt_tx_twt_teardown_etc(hmac_vap_stru *pst_hmac_vap,
                                         hmac_user_stru *pst_hmac_user,
                                         mac_twt_action_mgmt_args_stru *pst_twt_action_args)
{
    mac_device_stru *pst_device;
    mac_vap_stru *pst_mac_vap;
    frw_event_mem_stru *pst_event_mem; /* 申请事件返回的内存指针 */
    oal_netbuf_stru *pst_twt_setup_req;
    oal_uint16 us_frame_len;
    frw_event_stru *pst_hmac_to_dmac_ctx_event;
    dmac_tx_event_stru *pst_tx_event;
    dmac_ctx_action_event_stru st_wlan_ctx_action;
    oal_uint32 ul_ret;
    mac_tx_ctl_stru *pst_tx_ctl;

    if (OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_hmac_user, pst_twt_action_args)) {
        OAM_ERROR_LOG3(0, OAM_SF_11AX, "{hmac_mgmt_tx_twt_teardown_etc::null param, %x %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_hmac_user, (uintptr_t)pst_twt_action_args);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);

    /* 获取device结构 */
    pst_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (pst_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_11AX, "{hmac_mgmt_tx_twt_teardown_etc::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请TWT SETUP管理帧内存 */
    pst_twt_setup_req = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_twt_setup_req == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
                       "{hmac_mgmt_tx_twt_teardown_etc::pst_twt_teardown_req null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEM_NETBUF_TRACE(pst_twt_setup_req, OAL_TRUE);

    OAL_NETBUF_PREV(pst_twt_setup_req) = OAL_PTR_NULL;
    OAL_NETBUF_NEXT(pst_twt_setup_req) = OAL_PTR_NULL;

    /* 调用封装管理帧接口 */
    us_frame_len = hmac_mgmt_encap_twt_teardown_req_etc(pst_hmac_vap, pst_hmac_user,
                                                        oal_netbuf_data(pst_twt_setup_req),
                                                        pst_twt_action_args);
    memset_s((oal_uint8 *)&st_wlan_ctx_action, OAL_SIZEOF(st_wlan_ctx_action), 0, OAL_SIZEOF(st_wlan_ctx_action));
    /* 赋值要传入Dmac的信息 */
    st_wlan_ctx_action.us_frame_len = us_frame_len;
    st_wlan_ctx_action.uc_hdr_len = MAC_80211_FRAME_LEN;
    st_wlan_ctx_action.en_action_category = MAC_ACTION_CATEGORY_S1G;
    st_wlan_ctx_action.uc_action = MAC_S1G_ACTION_TWT_TEARDOWN;
    st_wlan_ctx_action.us_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;

    if (EOK != memcpy_s((oal_uint8 *)(oal_netbuf_data(pst_twt_setup_req) + us_frame_len),
                        OAL_SIZEOF(dmac_ctx_action_event_stru),
                        (oal_uint8 *)&st_wlan_ctx_action,
                        OAL_SIZEOF(dmac_ctx_action_event_stru))) {
        OAM_ERROR_LOG0(0, OAM_SF_11AX, "hmac_mgmt_tx_twt_teardown_etc::memcpy fail!");
        oal_netbuf_free(pst_twt_setup_req);
        return OAL_FAIL;
    }
    oal_netbuf_put(pst_twt_setup_req, (us_frame_len + OAL_SIZEOF(dmac_ctx_action_event_stru)));

    /* 初始化CB */
    memset_s(oal_netbuf_cb(pst_twt_setup_req), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_twt_setup_req);
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_frame_len + OAL_SIZEOF(dmac_ctx_action_event_stru);
    ;
    MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_ACTION;
    /* 在dmac_tx_action_process  skip, 直接send to air */
    MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = WLAN_ACTION_TWT_TEARDOWN_REQ;

    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_tx_event_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_11AX, "{hmac_mgmt_tx_twt_teardown_etc::pst_event_mem null.}");
        oal_netbuf_free(pst_twt_setup_req);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_to_dmac_ctx_event = frw_get_event_stru(pst_event_mem);
    FRW_EVENT_HDR_INIT(&(pst_hmac_to_dmac_ctx_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT,
                       OAL_SIZEOF(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    pst_tx_event = (dmac_tx_event_stru *)(pst_hmac_to_dmac_ctx_event->auc_event_data);
    pst_tx_event->pst_netbuf = pst_twt_setup_req;
    pst_tx_event->us_frame_len = us_frame_len;

    ul_ret = frw_event_dispatch_event_etc(pst_event_mem);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
                       "{hmac_mgmt_tx_twt_teardown_etc::frw_event_dispatch_event_etc failed[%d].}", ul_ret);
        oal_netbuf_free(pst_twt_setup_req);
        FRW_EVENT_FREE(pst_event_mem);
        return ul_ret;
    }

    pst_hmac_vap->st_twt_cfg.uc_twt_session_enable = 0;

    FRW_EVENT_FREE(pst_event_mem);
    return OAL_SUCC;
}



OAL_STATIC oal_uint32 hmac_sta_twt_save_parameters(hmac_vap_stru *pst_hmac_vap,
    mac_twt_element_ie_individual_stru *pst_twt_element)
{
    if (OAL_ANY_NULL_PTR2(pst_hmac_vap, pst_twt_element)) {
        OAM_ERROR_LOG2(0, OAM_SF_11AX, "{hmac_sta_twt_save_parameters::null param, 0x%x 0x%x 0x%x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_twt_element);
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
    pst_hmac_vap->st_twt_cfg.ull_twt_start_time = pst_twt_element->ull_twt;
    pst_hmac_vap->st_twt_cfg.ul_twt_duration =
        ((oal_uint32)pst_twt_element->uc_min_duration) << 8; /* Nominal Minimum TWT Wake Duration, 单位是256us */
    pst_hmac_vap->st_twt_cfg.ull_twt_interval =
        (oal_uint64)pst_twt_element->us_mantissa << pst_twt_element->st_request_type.bit_exponent;
    pst_hmac_vap->st_twt_cfg.uc_twt_flow_id = pst_twt_element->st_request_type.bit_flow_id;
    pst_hmac_vap->st_twt_cfg.uc_twt_announce_bit = pst_twt_element->st_request_type.bit_flow_type;
    pst_hmac_vap->st_twt_cfg.uc_twt_is_trigger_enabled = pst_twt_element->st_request_type.bit_trigger;
    pst_hmac_vap->st_twt_cfg.uc_twt_session_enable = 1;

    return OAL_SUCC;
}



OAL_STATIC oal_uint32 hmac_sta_twt_config_accept_action(hmac_vap_stru *pst_hmac_vap,
    mac_twt_action_mgmt_args_stru *pst_twt_action_args, mac_twt_element_ie_individual_stru *pst_twt_element)
{
    if (OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_twt_action_args, pst_twt_element)) {
        OAM_ERROR_LOG3(0, OAM_SF_11AX, "{hmac_sta_twt_config_accept_action::null param, 0x%x 0x%x 0x%x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_twt_action_args, (uintptr_t)pst_twt_element);
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
    pst_twt_action_args->uc_category = MAC_ACTION_CATEGORY_S1G;
    pst_twt_action_args->uc_action = MAC_S1G_ACTION_TWT_SETUP;
    pst_twt_action_args->uc_twt_setup_cmd = MAC_TWT_COMMAND_ACCEPT;
    pst_twt_action_args->uc_twt_flow_type = pst_twt_element->st_request_type.bit_flow_type;
    pst_twt_action_args->uc_twt_flow_id = pst_twt_element->st_request_type.bit_flow_id;
    pst_twt_action_args->ul_twt_start_time_offset = (oal_uint32)(pst_twt_element->ull_twt & 0xffffffff);
    pst_twt_action_args->ul_twt_exponent = pst_twt_element->st_request_type.bit_exponent;
    pst_twt_action_args->ul_twt_duration = pst_twt_element->uc_min_duration;
    pst_twt_action_args->ul_intrval_mantissa = pst_twt_element->us_mantissa;

    return OAL_SUCC;
}



OAL_STATIC oal_uint32 hmac_sta_twt_update_parameters_to_device(hmac_vap_stru *pst_hmac_vap,
    hmac_user_stru *pst_hmac_user)
{
    mac_vap_stru *pst_mac_vap;
    frw_event_mem_stru *pst_event_mem; /* 申请事件返回的内存指针 */
    frw_event_stru *pst_hmac_to_dmac_crx_sync;
    dmac_ctx_update_twt_stru *pst_update_twt_cfg_event;

    if (OAL_ANY_NULL_PTR2(pst_hmac_vap, pst_hmac_user)) {
        OAM_ERROR_LOG2(0, OAM_SF_11AX, "{hmac_sta_twt_update_parameters_to_device:: null param, 0x%x 0x%x 0x%x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_hmac_user);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);

    /* 抛事件到DMAC处理 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_ctx_update_twt_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
                       "{hmac_sta_twt_update_parameters_to_device::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_hmac_to_dmac_crx_sync = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_hmac_to_dmac_crx_sync->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_UPDATE_TWT,
                       OAL_SIZEOF(dmac_ctx_action_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    /* 获取帧体信息，由于DMAC的同步，填写事件payload */
    pst_update_twt_cfg_event = (dmac_ctx_update_twt_stru *)(pst_hmac_to_dmac_crx_sync->auc_event_data);
    pst_update_twt_cfg_event->en_action_category = MAC_ACTION_CATEGORY_S1G;
    pst_update_twt_cfg_event->uc_action = MAC_S1G_ACTION_TWT_SETUP;
    pst_update_twt_cfg_event->us_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;
    pst_update_twt_cfg_event->st_twt_cfg.ull_twt_start_time = pst_hmac_vap->st_twt_cfg.ull_twt_start_time;
    pst_update_twt_cfg_event->st_twt_cfg.ull_twt_interval = pst_hmac_vap->st_twt_cfg.ull_twt_interval;
    pst_update_twt_cfg_event->st_twt_cfg.ul_twt_duration = pst_hmac_vap->st_twt_cfg.ul_twt_duration;
    pst_update_twt_cfg_event->st_twt_cfg.uc_twt_flow_id = pst_hmac_vap->st_twt_cfg.uc_twt_flow_id;
    pst_update_twt_cfg_event->st_twt_cfg.uc_twt_announce_bit = pst_hmac_vap->st_twt_cfg.uc_twt_announce_bit;
    pst_update_twt_cfg_event->st_twt_cfg.uc_twt_is_trigger_enabled = pst_hmac_vap->st_twt_cfg.uc_twt_is_trigger_enabled;
    pst_update_twt_cfg_event->st_twt_cfg.uc_twt_session_enable = pst_hmac_vap->st_twt_cfg.uc_twt_session_enable;
    pst_update_twt_cfg_event->st_twt_cfg.uc_next_twt_size = 0; /* 只有twt information帧才有 */

    /* 分发 */
    frw_event_dispatch_event_etc(pst_event_mem);

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);
    return OAL_SUCC;
}


oal_uint32 hmac_sta_rx_twt_setup_frame_etc(hmac_vap_stru *pst_hmac_vap,
                                           hmac_user_stru *pst_hmac_user,
                                           oal_uint8 *puc_payload)
{
    mac_twt_element_ie_individual_stru *pst_twt_element;
    mac_twt_command_enum_uint8 en_command;
    oal_uint64 ull_twt_interval;
    oal_uint32 ul_twt_wake_duration;
    mac_twt_action_mgmt_args_stru st_twt_action_args;

    if (g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_twt_responder_support == OAL_FALSE) {
        OAM_WARNING_LOG0(0, OAM_SF_11AX,
            "{hmac_sta_rx_twt_setup_frame_etc::twt responder support is off in the ini file!.}");
        return OAL_FAIL;
    }

    if (OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_hmac_user, puc_payload)) {
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
    pst_twt_element = (mac_twt_element_ie_individual_stru *)(puc_payload);

    OAM_WARNING_LOG3(0, OAM_SF_11AX,
                     "{hmac_sta_rx_twt_setup_frame_etc:: flow_id:%d, bit_flow_type:%d, bit_setup_command:%d}",
                     pst_twt_element->st_request_type.bit_flow_id,
                     pst_twt_element->st_request_type.bit_flow_type,
                     pst_twt_element->st_request_type.bit_setup_command);

    /* 非TWT element */
    if (pst_twt_element->uc_element_id != MAC_EID_TWT) {
        OAM_WARNING_LOG1(0, OAM_SF_11AX,
                         "{hmac_sta_rx_twt_setup_frame_etc:: It's not a twt element id:%d}",
                         pst_twt_element->uc_element_id);
        return OAL_FAIL;
    }

    en_command = (mac_twt_command_enum_uint8)pst_twt_element->st_request_type.bit_setup_command;

    /* 大于2的twt commad都是ap发送过来的，需要检查dialog token */
    if ((en_command > MAC_TWT_COMMAND_DEMAND) &&
        (pst_twt_element->uc_dialog_token != pst_hmac_vap->st_twt_cfg.uc_twt_dialog_token)) {
        OAM_WARNING_LOG3(0, OAM_SF_11AX,
            "{hmac_sta_rx_twt_setup_frame_etc:: twt dialog token is not same: rx %d, tx:%d, twt setup command:%d}",
            pst_twt_element->uc_dialog_token,
            pst_hmac_vap->st_twt_cfg.uc_twt_dialog_token,
            en_command);
        return OAL_FAIL;
    }

    /* 检查twt参数，非法参数不允许建立twt会话 */
    ull_twt_interval = (oal_uint64)pst_twt_element->us_mantissa << pst_twt_element->st_request_type.bit_exponent;
    ul_twt_wake_duration = pst_twt_element->uc_min_duration * 256;
    if (ull_twt_interval < ul_twt_wake_duration) {
        OAM_WARNING_LOG2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_11AX,
                         "{hmac_sta_rx_twt_setup_frame_etc::error twt interval [%d] < twt wake duration!.}",
                         ull_twt_interval, ul_twt_wake_duration);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    switch (en_command) {
        case MAC_TWT_COMMAND_ACCEPT:
            hmac_sta_twt_save_parameters(pst_hmac_vap, pst_twt_element);
            hmac_sta_twt_update_parameters_to_device(pst_hmac_vap, pst_hmac_user);
            break;

        case MAC_TWT_COMMAND_REQUEST:
        case MAC_TWT_COMMAND_SUGGEST:
        case MAC_TWT_COMMAND_DEMAND:
            hmac_sta_twt_save_parameters(pst_hmac_vap, pst_twt_element);
            hmac_sta_twt_config_accept_action(pst_hmac_vap, &st_twt_action_args, pst_twt_element);
            /* 在dmac发送完成中断收到ack后再设置twt参数 */
            hmac_mgmt_tx_twt_setup_etc(pst_hmac_vap, pst_hmac_user, &st_twt_action_args);
            break;

        default:
            break;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_sta_rx_twt_teardown_frame_etc(hmac_vap_stru *pst_hmac_vap,
                                              hmac_user_stru *pst_hmac_user,
                                              oal_uint8 *puc_payload)
{
    mac_device_stru *pst_device;
    mac_vap_stru *pst_mac_vap;
    mac_twt_teardown_stru *pst_twt_teardown;
    frw_event_mem_stru *pst_event_mem; /* 申请事件返回的内存指针 */
    frw_event_stru *pst_hmac_to_dmac_crx_sync;
    dmac_ctx_update_twt_stru *pst_update_twt_cfg_event;

    if (OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_hmac_user, puc_payload)) {
        OAM_ERROR_LOG3(0, OAM_SF_11AX,
                       "{hmac_sta_rx_twt_teardown_frame_etc::twt teardown receive failed, null param, 0x%x 0x%x 0x%x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_hmac_user, (uintptr_t)puc_payload);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);

    /* 获取device结构 */
    pst_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);

    if (pst_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
                       "{hmac_sta_rx_twt_teardown_frame_etc::twt teardown receive failed, pst_device null.}");
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

    pst_twt_teardown = (mac_twt_teardown_stru *)(puc_payload);

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
                     "{hmac_sta_rx_twt_teardown_frame_etc:: flow_id:%d, bit_nego_type:%d}",
                     pst_twt_teardown->bit_twt_flow_id, pst_twt_teardown->bit_nego_type);

    /* 只支持删除单播TWT会话 */
    if (pst_twt_teardown->bit_nego_type != 0) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
                         "{hmac_sta_rx_twt_teardown_frame_etc:: not support twt negotiation type:%d}",
                         pst_twt_teardown->bit_nego_type);
        return OAL_FAIL;
    }

    /* 检查TWT是否在运行 */
    if (pst_hmac_vap->st_twt_cfg.uc_twt_session_enable == 0) {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
                         "{hmac_sta_rx_twt_teardown_frame_etc:: twt session is already disabled!}");
        return OAL_FAIL;
    }

    /* 检查flow id */
    if (pst_hmac_vap->st_twt_cfg.uc_twt_flow_id != pst_twt_teardown->bit_twt_flow_id) {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
                         "{hmac_sta_rx_twt_teardown_frame_etc:: the flow id [%d] is invalid,our flow id is [%d]}",
                         pst_twt_teardown->bit_twt_flow_id, pst_hmac_vap->st_twt_cfg.uc_twt_flow_id);
        return OAL_FAIL;
    }

    /*  To-Do, bit number不一样 */
    pst_hmac_vap->st_twt_cfg.ull_twt_start_time = 0;
    pst_hmac_vap->st_twt_cfg.ul_twt_duration = 0;
    pst_hmac_vap->st_twt_cfg.ull_twt_interval = 0;
    pst_hmac_vap->st_twt_cfg.uc_twt_flow_id = pst_twt_teardown->bit_twt_flow_id;
    pst_hmac_vap->st_twt_cfg.uc_twt_announce_bit = 0;
    pst_hmac_vap->st_twt_cfg.uc_twt_is_trigger_enabled = 0;
    pst_hmac_vap->st_twt_cfg.uc_twt_session_enable = 0;

    /* 抛事件到DMAC处理 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_ctx_update_twt_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
                       "{hmac_sta_rx_twt_teardown_frame_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_hmac_to_dmac_crx_sync = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_hmac_to_dmac_crx_sync->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_UPDATE_TWT,
                       OAL_SIZEOF(dmac_ctx_action_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    /* 获取帧体信息，由于DMAC的同步，填写事件payload */
    pst_update_twt_cfg_event = (dmac_ctx_update_twt_stru *)(pst_hmac_to_dmac_crx_sync->auc_event_data);
    pst_update_twt_cfg_event->en_action_category = MAC_ACTION_CATEGORY_S1G;
    pst_update_twt_cfg_event->uc_action = MAC_S1G_ACTION_TWT_TEARDOWN;
    pst_update_twt_cfg_event->us_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;
    pst_update_twt_cfg_event->st_twt_cfg.ull_twt_start_time = pst_hmac_vap->st_twt_cfg.ull_twt_start_time;
    pst_update_twt_cfg_event->st_twt_cfg.ull_twt_interval = pst_hmac_vap->st_twt_cfg.ull_twt_interval;
    pst_update_twt_cfg_event->st_twt_cfg.ul_twt_duration = pst_hmac_vap->st_twt_cfg.ul_twt_duration;
    pst_update_twt_cfg_event->st_twt_cfg.uc_twt_flow_id = pst_hmac_vap->st_twt_cfg.uc_twt_flow_id;
    pst_update_twt_cfg_event->st_twt_cfg.uc_twt_announce_bit = pst_hmac_vap->st_twt_cfg.uc_twt_announce_bit;
    pst_update_twt_cfg_event->st_twt_cfg.uc_twt_is_trigger_enabled = pst_hmac_vap->st_twt_cfg.uc_twt_is_trigger_enabled;
    pst_update_twt_cfg_event->st_twt_cfg.uc_twt_session_enable = pst_hmac_vap->st_twt_cfg.uc_twt_session_enable;
    pst_update_twt_cfg_event->st_twt_cfg.uc_next_twt_size = 0; /* 只有twt information帧才有 */

    /* 分发 */
    frw_event_dispatch_event_etc(pst_event_mem);

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);
    return OAL_SUCC;
}


OAL_STATIC oal_uint64 hmac_get_next_twt(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_payload,
    oal_uint8 uc_next_twt_subfield_size)
{
    oal_uint32 ul_next_twt_low = 0;
    oal_uint64 ul_next_twt_high = 0;
    oal_uint64 ull_next_twt = 0;

    switch (uc_next_twt_subfield_size) {
        case HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_0:
            ull_next_twt = 0;
            break;

        case HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_32: /* 32 bits */
            ul_next_twt_low = puc_payload[0];
            ul_next_twt_low |= (puc_payload[1] << 8);
            ul_next_twt_low |= (puc_payload[2] << 16);
            ul_next_twt_low |= (puc_payload[3] << 24);
            ull_next_twt = ul_next_twt_low;
            break;

        case HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_48: /* 48 bits */
            ul_next_twt_low = puc_payload[0];
            ul_next_twt_low |= (puc_payload[1] << 8);
            ul_next_twt_low |= (puc_payload[2] << 16);
            ul_next_twt_low |= (puc_payload[3] << 24);
            ul_next_twt_high = puc_payload[4];
            ul_next_twt_high |= (puc_payload[5] << 8);
            ull_next_twt = ul_next_twt_low;
            ull_next_twt |= (ul_next_twt_high << 32);
            break;

        case HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_64: /* 64 bits */
            ul_next_twt_low = puc_payload[0];
            ul_next_twt_low |= (puc_payload[1] << 8);
            ul_next_twt_low |= (puc_payload[2] << 16);
            ul_next_twt_low |= (puc_payload[3] << 24);
            ul_next_twt_high = puc_payload[4];
            ul_next_twt_high |= (puc_payload[5] << 8);
            ul_next_twt_high |= (puc_payload[6] << 16);
            ul_next_twt_high |= ((oal_uint32)puc_payload[7] << 24);
            ull_next_twt = ul_next_twt_low;
            ull_next_twt |= (ul_next_twt_high << 32);
            break;

        default:
            ull_next_twt = 0;
            OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_11AX,
                             "{hmac_get_next_twt:: invalid next twt size [%d]!}",
                             uc_next_twt_subfield_size);
            break;
    }

    return ull_next_twt;
}


oal_uint32 hmac_sta_rx_twt_information_frame_etc(hmac_vap_stru *pst_hmac_vap,
                                                 hmac_user_stru *pst_hmac_user,
                                                 oal_uint8 *puc_payload)
{
    mac_device_stru *pst_device;
    mac_vap_stru *pst_mac_vap;
    mac_twt_information_frame_stru *pst_twt_information_frame_body;

    frw_event_mem_stru *pst_event_mem; /* 申请事件返回的内存指针 */
    frw_event_stru *pst_hmac_to_dmac_crx_sync;
    dmac_ctx_update_twt_stru *pst_update_twt_cfg_event;

    oal_uint64 ull_next_twt;
    oal_uint8 uc_next_twt_subfield_size;

    if (OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_hmac_user, puc_payload)) {
        OAM_ERROR_LOG3(0, OAM_SF_11AX,
                       "{hmac_sta_rx_twt_information_frame_etc::null ptr:0x%x 0x%x 0x%x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_hmac_user, (uintptr_t)puc_payload);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);

    /* 获取device结构 */
    pst_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);

    if (pst_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
                       "{hmac_sta_rx_twt_information_frame_etc::twt information receive failed, pst_device null.}");
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

    pst_twt_information_frame_body = (mac_twt_information_frame_stru *)(puc_payload);

    OAM_WARNING_LOG2(0, OAM_SF_11AX, "{hmac_sta_rx_twt_information_frame_etc:: flow id:%d, next twt subfield size:%d}",
                     pst_twt_information_frame_body->st_twt_information_filed.bit_twt_flow_id,
                     pst_twt_information_frame_body->st_twt_information_filed.bit_next_twt_subfield_size);

    /* 检查TWT是否在运行 */
    if (pst_hmac_vap->st_twt_cfg.uc_twt_session_enable == 0) {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
                         "{hmac_sta_rx_twt_information_frame_etc:: twt session is disabled!}");
        return OAL_FAIL;
    }

    /* 检查flow id */
    if (pst_hmac_vap->st_twt_cfg.uc_twt_flow_id !=
        pst_twt_information_frame_body->st_twt_information_filed.bit_twt_flow_id) {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
                         "{hmac_sta_rx_twt_teardown_frame_etc:: the flow id [%d] is invalid,our flow id is [%d]}",
                         pst_twt_information_frame_body->st_twt_information_filed.bit_twt_flow_id,
                         pst_hmac_vap->st_twt_cfg.uc_twt_flow_id);
        return OAL_FAIL;
    }

    uc_next_twt_subfield_size = pst_twt_information_frame_body->st_twt_information_filed.bit_next_twt_subfield_size;
    puc_payload += OAL_SIZEOF(mac_twt_information_frame_stru);

    ull_next_twt = hmac_get_next_twt(pst_hmac_vap, puc_payload, uc_next_twt_subfield_size);

    /*  这里只更新twt start time和next twt size */
    pst_hmac_vap->st_twt_cfg.ull_twt_start_time = ull_next_twt;
    pst_hmac_vap->st_twt_cfg.uc_next_twt_size = uc_next_twt_subfield_size;
    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
                     "{hmac_sta_rx_twt_information_frame_etc:: ull_next_twt=[%ld], uc_next_twt_subfield_size=[%d]}",
                     ull_next_twt, uc_next_twt_subfield_size);

    /* 抛事件到DMAC处理 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_ctx_update_twt_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_11AX,
                       "{hmac_sta_rx_twt_information_frame_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_hmac_to_dmac_crx_sync = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_hmac_to_dmac_crx_sync->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_UPDATE_TWT,
                       OAL_SIZEOF(dmac_ctx_action_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    /* 获取帧体信息，由于DMAC的同步，填写事件payload */
    pst_update_twt_cfg_event = (dmac_ctx_update_twt_stru *)(pst_hmac_to_dmac_crx_sync->auc_event_data);
    pst_update_twt_cfg_event->en_action_category = MAC_ACTION_CATEGORY_S1G;
    pst_update_twt_cfg_event->uc_action = MAC_S1G_ACTION_TWT_INFORMATION;
    pst_update_twt_cfg_event->us_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;
    pst_update_twt_cfg_event->st_twt_cfg.ull_twt_start_time = pst_hmac_vap->st_twt_cfg.ull_twt_start_time;
    pst_update_twt_cfg_event->st_twt_cfg.ull_twt_interval = pst_hmac_vap->st_twt_cfg.ull_twt_interval;
    pst_update_twt_cfg_event->st_twt_cfg.ul_twt_duration = pst_hmac_vap->st_twt_cfg.ul_twt_duration;
    pst_update_twt_cfg_event->st_twt_cfg.uc_twt_flow_id = pst_hmac_vap->st_twt_cfg.uc_twt_flow_id;
    pst_update_twt_cfg_event->st_twt_cfg.uc_twt_announce_bit = pst_hmac_vap->st_twt_cfg.uc_twt_announce_bit;
    pst_update_twt_cfg_event->st_twt_cfg.uc_twt_is_trigger_enabled = pst_hmac_vap->st_twt_cfg.uc_twt_is_trigger_enabled;
    pst_update_twt_cfg_event->st_twt_cfg.uc_twt_session_enable = pst_hmac_vap->st_twt_cfg.uc_twt_session_enable;
    pst_update_twt_cfg_event->st_twt_cfg.uc_next_twt_size = pst_hmac_vap->st_twt_cfg.uc_next_twt_size;

    /* 分发 */
    frw_event_dispatch_event_etc(pst_event_mem);

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);
    return OAL_SUCC;
}
#endif


oal_uint16 hmac_mgmt_encap_addba_req_etc(hmac_vap_stru *pst_vap,
                                         oal_uint8 *puc_data,
                                         dmac_ba_tx_stru *pst_tx_ba,
                                         oal_uint8 uc_tid)
{
    oal_uint16 us_index;
    oal_uint16 us_ba_param;
    if (OAL_ANY_NULL_PTR3(pst_vap, puc_data, pst_tx_ba)) {
        OAM_ERROR_LOG3(0, OAM_SF_BA, "{hmac_mgmt_encap_addba_req_etc::null param.vap:%x data:%x ba:%x}",
                       (uintptr_t)pst_vap, (uintptr_t)puc_data, (uintptr_t)pst_tx_ba);
        return OAL_ERR_CODE_PTR_NULL;
    }

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
    /* Frame Control Field 中只需要设置Type/Subtype值，其他设置为0 */
    mac_hdr_set_frame_control(puc_data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* DA is address of STA requesting association */
    oal_set_mac_addr(puc_data + 4, pst_tx_ba->puc_dst_addr);

    /* SA的值为dot11MACAddress的值 */
    oal_set_mac_addr(puc_data + 10, mac_mib_get_StationID(&pst_vap->st_vap_base_info));

    oal_set_mac_addr(puc_data + 16, pst_vap->st_vap_base_info.auc_bssid);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /* 将索引指向frame body起始位置 */
    us_index = MAC_80211_FRAME_LEN;

    /* 设置Category */
    puc_data[us_index++] = MAC_ACTION_CATEGORY_BA;

    /* 设置Action */
    puc_data[us_index++] = MAC_BA_ACTION_ADDBA_REQ;

    /* 设置Dialog Token */
    puc_data[us_index++] = pst_tx_ba->uc_dialog_token;

    /*
        设置Block Ack Parameter set field
        bit0 - AMSDU Allowed
        bit1 - Immediate or Delayed block ack
        bit2-bit5 - TID
        bit6-bit15 -  Buffer size
    */
    us_ba_param = pst_tx_ba->en_amsdu_supp;        /* bit0 */
    us_ba_param |= (pst_tx_ba->uc_ba_policy << 1); /* bit1 */
    us_ba_param |= (uc_tid << 2);                  /* bit2 */

    us_ba_param |= (oal_uint16)(pst_tx_ba->us_baw_size << 6); /* bit6 */

    puc_data[us_index++] = (oal_uint8)(us_ba_param & 0xFF);
    puc_data[us_index++] = (oal_uint8)((us_ba_param >> 8) & 0xFF);

    /* 设置BlockAck timeout */
    puc_data[us_index++] = (oal_uint8)(pst_tx_ba->us_ba_timeout & 0xFF);
    puc_data[us_index++] = (oal_uint8)((pst_tx_ba->us_ba_timeout >> 8) & 0xFF);

    /*
        Block ack starting sequence number字段由硬件设置
        bit0-bit3 fragmentnumber
        bit4-bit15: sequence number
    */
    /* us_buf_seq此处暂不填写，在dmac侧会补充填写 */
    *(oal_uint16 *)&puc_data[us_index++] = 0;
    us_index++;
    // puc_data[us_index++] = 0;
    // puc_data[us_index++] = 0;
    /* 返回的帧长度中不包括FCS */
    return us_index;
}


oal_uint16 hmac_mgmt_encap_addba_rsp_etc(hmac_vap_stru *pst_vap,
                                         oal_uint8 *puc_data,
                                         hmac_ba_rx_stru *pst_addba_rsp,
                                         oal_uint8 uc_tid,
                                         oal_uint8 uc_status)
{
    oal_uint16 us_index;
    oal_uint16 us_ba_param;
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    hmac_user_stru *pst_hmac_user;
    hmac_user_btcoex_stru *pst_hmac_user_btcoex;
#endif
    if (OAL_ANY_NULL_PTR3(pst_vap, puc_data, pst_addba_rsp)) {
        OAM_ERROR_LOG3(0, OAM_SF_BA, "{hmac_mgmt_encap_addba_req_etc::null param.vap:%x data:%x rsp:%x}",
                       (uintptr_t)pst_vap, (uintptr_t)puc_data, (uintptr_t)pst_addba_rsp);
        return OAL_ERR_CODE_PTR_NULL;
    }

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
    mac_hdr_set_frame_control(puc_data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* DA is address of STA requesting association */
    oal_set_mac_addr(puc_data + 4, pst_addba_rsp->puc_transmit_addr);

    /* SA is the dot11MACAddress */
    oal_set_mac_addr(puc_data + 10, mac_mib_get_StationID(&pst_vap->st_vap_base_info));

    oal_set_mac_addr(puc_data + 16, pst_vap->st_vap_base_info.auc_bssid);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*             ADDBA Response Frame - Frame Body                         */
    /*    ---------------------------------------------------------------    */
    /*    | Category | Action | Dialog | Status  | Parameters | Timeout |    */
    /*    ---------------------------------------------------------------    */
    /*    | 1        | 1      | 1      | 2       | 2          | 2       |    */
    /*    ---------------------------------------------------------------    */
    /*                                                                       */
    /*************************************************************************/
    /* Initialize index and the frame data pointer */
    us_index = MAC_80211_FRAME_LEN;

    /* Action Category设置 */
    puc_data[us_index++] = MAC_ACTION_CATEGORY_BA;

    /* 特定Action种类下的action的帧类型 */
    puc_data[us_index++] = MAC_BA_ACTION_ADDBA_RSP;

    /* Dialog Token域设置，需要从req中copy过来 */
    puc_data[us_index++] = pst_addba_rsp->uc_dialog_token;

    /* 状态域设置 */
    puc_data[us_index++] = uc_status;
    puc_data[us_index++] = 0;

    /* Block Ack Parameter设置 */
    /* B0 - AMSDU Support, B1- Immediate or Delayed block ack */
    /* B2-B5 : TID, B6-B15: Buffer size */
    us_ba_param = pst_addba_rsp->en_amsdu_supp;        /* BIT0 */
    us_ba_param |= (pst_addba_rsp->uc_ba_policy << 1); /* BIT1 */
    us_ba_param |= (uc_tid << 2);                      /* BIT2 */

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* 手动设置聚合个数，屏蔽删建BA时不采用64聚合 */
    pst_hmac_user = mac_vap_get_hmac_user_by_addr_etc(&(pst_vap->st_vap_base_info), pst_addba_rsp->puc_transmit_addr);

    if (pst_hmac_user != OAL_PTR_NULL) {
        if (OAL_TRUE == MAC_BTCOEX_CHECK_VALID_STA(&(pst_vap->st_vap_base_info))) {
            pst_hmac_user_btcoex = &(pst_hmac_user->st_hmac_user_btcoex);

            /* 1.黑名单用户 */
            if (pst_hmac_user_btcoex->st_hmac_btcoex_addba_req.en_ba_handle_allow == OAL_FALSE) {
                if (BTCOEX_BLACKLIST_TPYE_FIX_BASIZE == HMAC_BTCOEX_GET_BLACKLIST_TYPE(pst_hmac_user)) {
                    us_ba_param |= (oal_uint16)(BTCOEX_BLACKLIST_BA_SIZE_LIMIT << 6);
                } else {
                    /* 黑名单时，btcoex聚合业务处于结束状态，按照默认聚合个数恢复wifi性能 */
                    us_ba_param |= (oal_uint16)(pst_addba_rsp->us_baw_size << 6); /* BIT6 */
                }
            }
            /* 2.共存触发的删除，按照共存配置来 */
            else if ((pst_hmac_user_btcoex->en_delba_btcoex_trigger == OAL_TRUE) &&
                     (pst_hmac_user_btcoex->us_ba_size != 0)) {
                us_ba_param |= (oal_uint16)(pst_hmac_user_btcoex->us_ba_size << 6); /* BIT6 */
            } else {
#ifdef _PRE_WLAN_FEATURE_11AX
                /* 标志vap工作在11ax */
                if ((OAL_TRUE == mac_mib_get_HEOptionImplemented(&pst_vap->st_vap_base_info)) &&
                    (MAC_USER_IS_HE_USER(&pst_hmac_user->st_user_base_info))) {
                    us_ba_param |= (oal_uint16)(pst_hmac_user->aus_tid_baw_size[uc_tid] << 6);
                } else
#endif
                {
                    us_ba_param |= (oal_uint16)(pst_addba_rsp->us_baw_size << 6); /* BIT6 */
                }
            }
        } else {
#ifdef _PRE_WLAN_FEATURE_11AX
            /* 标志vap工作在11ax */
            if ((OAL_TRUE == mac_mib_get_HEOptionImplemented(&pst_vap->st_vap_base_info)) &&
                (MAC_USER_IS_HE_USER(&pst_hmac_user->st_user_base_info))) {
                us_ba_param |= (oal_uint16)(pst_hmac_user->aus_tid_baw_size[uc_tid] << 6);
            } else
#endif
            {
                us_ba_param |= (oal_uint16)(pst_addba_rsp->us_baw_size << 6); /* BIT6 */
            }
        }
    } else
#endif
    {
        us_ba_param |= (oal_uint16)(pst_addba_rsp->us_baw_size << 6); /* BIT6 */
    }

    puc_data[us_index++] = (oal_uint8)(us_ba_param & 0xFF);
    puc_data[us_index++] = (oal_uint8)((us_ba_param >> 8) & 0xFF);

    puc_data[us_index++] = 0x00;
    puc_data[us_index++] = 0x00;

    /* 返回的帧长度中不包括FCS */
    return us_index;
}


oal_uint16 hmac_mgmt_encap_delba_etc(hmac_vap_stru *pst_vap,
                                     oal_uint8 *puc_data,
                                     oal_uint8 *puc_addr,
                                     oal_uint8 uc_tid,
                                     mac_delba_initiator_enum_uint8 en_initiator,
                                     oal_uint8 reason)
{
    oal_uint16 us_index;

    if (OAL_ANY_NULL_PTR3(pst_vap, puc_data, puc_addr)) {
        OAM_ERROR_LOG3(0, OAM_SF_BA, "{hmac_mgmt_encap_delba_etc::null param, %x %x %x.}",
                       (uintptr_t)pst_vap, (uintptr_t)puc_data, (uintptr_t)puc_addr);
        return OAL_ERR_CODE_PTR_NULL;
    }

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
    mac_hdr_set_frame_control(puc_data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* duration */
    puc_data[2] = 0;
    puc_data[3] = 0;

    /* DA is address of STA requesting association */
    oal_set_mac_addr(puc_data + 4, puc_addr);

    /* SA is the dot11MACAddress */
    oal_set_mac_addr(puc_data + 10, mac_mib_get_StationID(&pst_vap->st_vap_base_info));

    oal_set_mac_addr(puc_data + 16, pst_vap->st_vap_base_info.auc_bssid);

    /* seq control */
    puc_data[22] = 0;
    puc_data[23] = 0;

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*             DELBA Response Frame - Frame Body                         */
    /*        -------------------------------------------------              */
    /*        | Category | Action |  Parameters | Reason code |              */
    /*        -------------------------------------------------              */
    /*        | 1        | 1      |       2     | 2           |              */
    /*        -------------------------------------------------              */
    /*                          Parameters                                   */
    /*                  -------------------------------                      */
    /*                  | Reserved | Initiator |  TID  |                     */
    /*                  -------------------------------                      */
    /*             bit  |    11    |    1      |  4    |                     */
    /*                  --------------------------------                     */
    /*************************************************************************/
    /* Initialize index and the frame data pointer */
    us_index = MAC_80211_FRAME_LEN;

    /* Category */
    puc_data[us_index++] = MAC_ACTION_CATEGORY_BA;

    /* Action */
    puc_data[us_index++] = MAC_BA_ACTION_DELBA;

    /* DELBA parameter set */
    /* B0 - B10 -reserved */
    /* B11 - initiator */
    /* B12-B15 - TID */
    puc_data[us_index++] = 0;
    puc_data[us_index] = (oal_uint8)(en_initiator << 3);
    puc_data[us_index++] |= (oal_uint8)(uc_tid << 4);

    /* Reason field */
    /* Reason can be either of END_BA, QSTA_LEAVING, UNKNOWN_BA */
    puc_data[us_index++] = reason;
    puc_data[us_index++] = 0;

    /* 返回的帧长度中不包括FCS */
    return us_index;
}


oal_uint32 hmac_mgmt_tx_addba_req_etc(hmac_vap_stru *pst_hmac_vap,
                                      hmac_user_stru *pst_hmac_user,
                                      mac_action_mgmt_args_stru *pst_action_args)
{
    mac_device_stru *pst_device = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL; /* 申请事件返回的内存指针 */
    oal_netbuf_stru *pst_addba_req = OAL_PTR_NULL;
    dmac_ba_tx_stru st_tx_ba;
    oal_uint8 uc_tidno;
    oal_uint16 us_frame_len;
    frw_event_stru *pst_hmac_to_dmac_ctx_event = OAL_PTR_NULL;
    dmac_tx_event_stru *pst_tx_event = OAL_PTR_NULL;
    dmac_ctx_action_event_stru st_wlan_ctx_action;
    oal_uint32 ul_ret;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;

    if (OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_hmac_user, pst_action_args)) {
        OAM_ERROR_LOG3(0, OAM_SF_BA, "{hmac_mgmt_tx_addba_req_etc::null param, %x %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_hmac_user, (uintptr_t)pst_action_args);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_tx_addba_req_etc:: vap has been down/del, vap_state[%d].}",
                         pst_mac_vap->en_vap_state);
        return OAL_FAIL;
    }
    /* 获取device结构 */
    pst_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (pst_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_tx_addba_req_etc::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请ADDBA_REQ管理帧内存 */
    pst_addba_req = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_addba_req == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_tx_addba_req_etc::pst_addba_req null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEM_NETBUF_TRACE(pst_addba_req, OAL_TRUE);

    OAL_NETBUF_PREV(pst_addba_req) = OAL_PTR_NULL;
    OAL_NETBUF_NEXT(pst_addba_req) = OAL_PTR_NULL;

    uc_tidno = (oal_uint8)(pst_action_args->ul_arg1);
    /* 对tid对应的txBA会话状态加锁 */
    oal_spin_lock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));

    pst_hmac_vap->uc_ba_dialog_token++;
    st_tx_ba.uc_dialog_token = pst_hmac_vap->uc_ba_dialog_token; /* 保证ba会话创建能够区分 */
    st_tx_ba.us_baw_size = (oal_uint8)(pst_action_args->ul_arg2);
    st_tx_ba.uc_ba_policy = (oal_uint8)(pst_action_args->ul_arg3);
    st_tx_ba.us_ba_timeout = (oal_uint16)(pst_action_args->ul_arg4);
    st_tx_ba.puc_dst_addr = pst_hmac_user->st_user_base_info.auc_user_mac_addr;

    /* 发端对AMPDU+AMSDU的支持 */
    st_tx_ba.en_amsdu_supp = (oal_bool_enum_uint8)mac_mib_get_AmsduPlusAmpduActive(pst_mac_vap);

    /*lint -e502*/
    if (st_tx_ba.en_amsdu_supp == OAL_FALSE) {
        HMAC_USER_SET_AMSDU_NOT_SUPPORT(pst_hmac_user, uc_tidno);
    }
    /*lint +e502*/
    else {
        HMAC_USER_SET_AMSDU_SUPPORT(pst_hmac_user, uc_tidno);
    }

    /* 调用封装管理帧接口 */
    us_frame_len = hmac_mgmt_encap_addba_req_etc(pst_hmac_vap, oal_netbuf_data(pst_addba_req), &st_tx_ba, uc_tidno);
    memset_s((oal_uint8 *)&st_wlan_ctx_action, OAL_SIZEOF(st_wlan_ctx_action), 0, OAL_SIZEOF(st_wlan_ctx_action));
    /* 赋值要传入Dmac的信息 */
    st_wlan_ctx_action.us_frame_len = us_frame_len;
    st_wlan_ctx_action.uc_hdr_len = MAC_80211_FRAME_LEN;
    st_wlan_ctx_action.en_action_category = MAC_ACTION_CATEGORY_BA;
    st_wlan_ctx_action.uc_action = MAC_BA_ACTION_ADDBA_REQ;
    st_wlan_ctx_action.us_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;
    st_wlan_ctx_action.uc_tidno = uc_tidno;
    st_wlan_ctx_action.uc_dialog_token = st_tx_ba.uc_dialog_token;
    st_wlan_ctx_action.uc_ba_policy = st_tx_ba.uc_ba_policy;
    st_wlan_ctx_action.us_baw_size = st_tx_ba.us_baw_size;
    st_wlan_ctx_action.us_ba_timeout = st_tx_ba.us_ba_timeout;
    st_wlan_ctx_action.en_amsdu_supp = st_tx_ba.en_amsdu_supp;

    if (EOK != memcpy_s((oal_uint8 *)(oal_netbuf_data(pst_addba_req) + us_frame_len),
                        OAL_SIZEOF(dmac_ctx_action_event_stru),
                        (oal_uint8 *)&st_wlan_ctx_action,
                        OAL_SIZEOF(dmac_ctx_action_event_stru))) {
        OAM_ERROR_LOG0(0, OAM_SF_BA, "hmac_mgmt_tx_addba_req_etc::memcpy fail!");
        oal_netbuf_free(pst_addba_req);
        oal_spin_unlock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));
        return OAL_FAIL;
    }
    oal_netbuf_put(pst_addba_req, (us_frame_len + OAL_SIZEOF(dmac_ctx_action_event_stru)));

    /* 初始化CB */
    memset_s(oal_netbuf_cb(pst_addba_req), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_addba_req);
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_frame_len + OAL_SIZEOF(dmac_ctx_action_event_stru);
    MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_ACTION;
    MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = WLAN_ACTION_BA_ADDBA_REQ;

    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_tx_event_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_tx_addba_req_etc::pst_event_mem null.}");
        oal_netbuf_free(pst_addba_req);
        oal_spin_unlock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_to_dmac_ctx_event = frw_get_event_stru(pst_event_mem);
    FRW_EVENT_HDR_INIT(&(pst_hmac_to_dmac_ctx_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT,
                       OAL_SIZEOF(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    pst_tx_event = (dmac_tx_event_stru *)(pst_hmac_to_dmac_ctx_event->auc_event_data);
    pst_tx_event->pst_netbuf = pst_addba_req;
    pst_tx_event->us_frame_len = us_frame_len + OAL_SIZEOF(dmac_ctx_action_event_stru);

    ul_ret = frw_event_dispatch_event_etc(pst_event_mem);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                       "{hmac_mgmt_tx_addba_req_etc::frw_event_dispatch_event_etc failed[%d].}", ul_ret);
        oal_netbuf_free(pst_addba_req);
        FRW_EVENT_FREE(pst_event_mem);
        oal_spin_unlock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));
        return ul_ret;
    }

    FRW_EVENT_FREE(pst_event_mem);

    /* 更新对应的TID信息 */
    pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.en_ba_status = DMAC_BA_INPROGRESS;
    pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.uc_dialog_token = st_tx_ba.uc_dialog_token;
    pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.uc_ba_policy = st_tx_ba.uc_ba_policy;

    mac_mib_incr_TxBASessionNumber(pst_mac_vap);
    /* 启动ADDBA超时计时器 */
    FRW_TIMER_CREATE_TIMER(&pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_addba_timer,
                           hmac_mgmt_tx_addba_timeout_etc,
                           WLAN_ADDBA_TIMEOUT,
                           &pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_alarm_data,
                           OAL_FALSE,
                           OAM_MODULE_ID_HMAC,
                           pst_device->ul_core_id);

    /* 对tid对应的tx BA会话状态解锁 */
    oal_spin_unlock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));

    return OAL_SUCC;
}


oal_uint32 hmac_mgmt_tx_addba_rsp_etc(hmac_vap_stru *pst_hmac_vap,
                                      hmac_user_stru *pst_hmac_user,
                                      hmac_ba_rx_stru *pst_ba_rx_info,
                                      oal_uint8 uc_tid,
                                      oal_uint8 uc_status)
{
    mac_device_stru *pst_device = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL; /* 申请事件返回的内存指针 */
    frw_event_stru *pst_hmac_to_dmac_ctx_event = OAL_PTR_NULL;
    dmac_tx_event_stru *pst_tx_event = OAL_PTR_NULL;
    dmac_ctx_action_event_stru st_wlan_ctx_action;
    oal_netbuf_stru *pst_addba_rsp = OAL_PTR_NULL;
    oal_uint16 us_frame_len;
    oal_uint32 ul_ret;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;

    if (OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_hmac_user, pst_ba_rx_info)) {
        OAM_ERROR_LOG3(0, OAM_SF_BA, "{hmac_mgmt_tx_addba_rsp_etc::send addba rsp failed, null param, %x %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_hmac_user, (uintptr_t)pst_ba_rx_info);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_tx_addba_rsp_etc:: vap has been down/del, vap_state[%d].}",
                         pst_mac_vap->en_vap_state);
        return OAL_FAIL;
    }
    /* 获取device结构 */
    pst_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);

    if (pst_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                       "{hmac_mgmt_tx_addba_rsp_etc::send addba rsp failed, pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请ADDBA_RSP管理帧内存 */
    pst_addba_rsp = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_addba_rsp == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                       "{hmac_mgmt_tx_addba_rsp_etc::send addba rsp failed, pst_addba_rsp mem alloc failed.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEM_NETBUF_TRACE(pst_addba_rsp, OAL_TRUE);

    OAL_NETBUF_PREV(pst_addba_rsp) = OAL_PTR_NULL;
    OAL_NETBUF_NEXT(pst_addba_rsp) = OAL_PTR_NULL;

    us_frame_len = hmac_mgmt_encap_addba_rsp_etc(pst_hmac_vap, oal_netbuf_data(pst_addba_rsp),
                                                 pst_ba_rx_info, uc_tid, uc_status);
    memset_s((oal_uint8 *)&st_wlan_ctx_action, OAL_SIZEOF(st_wlan_ctx_action), 0, OAL_SIZEOF(st_wlan_ctx_action));
    st_wlan_ctx_action.en_action_category = MAC_ACTION_CATEGORY_BA;
    st_wlan_ctx_action.uc_action = MAC_BA_ACTION_ADDBA_RSP;
    st_wlan_ctx_action.uc_hdr_len = MAC_80211_FRAME_LEN;
    st_wlan_ctx_action.us_baw_size = pst_hmac_user->aus_tid_baw_size[uc_tid];
    st_wlan_ctx_action.us_frame_len = us_frame_len;
    st_wlan_ctx_action.us_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;
    st_wlan_ctx_action.uc_tidno = uc_tid;
    st_wlan_ctx_action.uc_status = uc_status;
    st_wlan_ctx_action.us_ba_timeout = pst_ba_rx_info->us_ba_timeout;
    st_wlan_ctx_action.en_back_var = pst_ba_rx_info->en_back_var;
    st_wlan_ctx_action.uc_lut_index = pst_ba_rx_info->uc_lut_index;
    st_wlan_ctx_action.us_baw_start = pst_ba_rx_info->us_baw_start;
    st_wlan_ctx_action.uc_ba_policy = pst_ba_rx_info->uc_ba_policy;

    if (EOK != memcpy_s((oal_uint8 *)(oal_netbuf_data(pst_addba_rsp) + us_frame_len),
                        OAL_SIZEOF(dmac_ctx_action_event_stru),
                        (oal_uint8 *)&st_wlan_ctx_action,
                        OAL_SIZEOF(dmac_ctx_action_event_stru))) {
        OAM_ERROR_LOG0(0, OAM_SF_BA, "hmac_mgmt_tx_addba_rsp_etc::memcpy fail!");
        oal_netbuf_free(pst_addba_rsp);
        return OAL_FAIL;
    }
    oal_netbuf_put(pst_addba_rsp, (us_frame_len + OAL_SIZEOF(dmac_ctx_action_event_stru)));

    /* 填写netbuf的cb字段，共发送管理帧和发送完成接口使用 */
    memset_s(oal_netbuf_cb(pst_addba_rsp), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_addba_rsp);
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = pst_hmac_user->st_user_base_info.us_assoc_id;
    MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl) = uc_tid;
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_frame_len + OAL_SIZEOF(dmac_ctx_action_event_stru);
    MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_ACTION;
    MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = WLAN_ACTION_BA_ADDBA_RSP;

    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_tx_event_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                       "{hmac_mgmt_tx_addba_rsp_etc::send addba rsp failed, pst_event_mem mem alloc failed.}");
        oal_netbuf_free(pst_addba_rsp);

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_to_dmac_ctx_event = frw_get_event_stru(pst_event_mem);
    FRW_EVENT_HDR_INIT(&(pst_hmac_to_dmac_ctx_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT,
                       OAL_SIZEOF(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    /* 填写事件payload */
    pst_tx_event = (dmac_tx_event_stru *)(pst_hmac_to_dmac_ctx_event->auc_event_data);
    pst_tx_event->pst_netbuf = pst_addba_rsp;
    pst_tx_event->us_frame_len = us_frame_len + OAL_SIZEOF(dmac_ctx_action_event_stru);

    ul_ret = frw_event_dispatch_event_etc(pst_event_mem);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_mgmt_tx_addba_rsp_etc::send addba rsp failed, frw_event_dispatch_event_etc failed[%d].}", ul_ret);
        oal_netbuf_free(pst_addba_rsp);
    } else {
        pst_ba_rx_info->en_ba_status = DMAC_BA_COMPLETE;
    }

    FRW_EVENT_FREE(pst_event_mem);
    return ul_ret;
}

oal_uint32 hmac_mgmt_tx_delba_etc(hmac_vap_stru *pst_hmac_vap,
                                  hmac_user_stru *pst_hmac_user,
                                  mac_action_mgmt_args_stru *pst_action_args)
{
    mac_device_stru *pst_device = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL; /* 申请事件返回的内存指针 */
    oal_netbuf_stru *pst_delba = OAL_PTR_NULL;
    oal_uint16 us_frame_len;
    frw_event_stru *pst_hmac_to_dmac_ctx_event = OAL_PTR_NULL;
    dmac_tx_event_stru *pst_tx_event = OAL_PTR_NULL;
    dmac_ctx_action_event_stru st_wlan_ctx_action;
    mac_delba_initiator_enum_uint8 en_initiator;
    oal_uint32 ul_ret;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;
    oal_uint8 uc_tidno;

    if (OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_hmac_user, pst_action_args)) {
        OAM_ERROR_LOG3(0, OAM_SF_BA, "{hmac_mgmt_tx_delba_etc::null param, %x %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_hmac_user, (uintptr_t)pst_action_args);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_tx_delba_etc:: vap has been down/del, vap_state[%d].}",
                         pst_mac_vap->en_vap_state);
        return OAL_FAIL;
    }
    /* 获取device结构 */
    pst_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);

    if (pst_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_tx_delba_etc::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    en_initiator = (mac_delba_initiator_enum_uint8)(pst_action_args->ul_arg2);
    uc_tidno = (oal_uint8)(pst_action_args->ul_arg1);

    /* 对tid对应的tx BA会话状态加锁 */
    oal_spin_lock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));

    
    if (en_initiator == MAC_ORIGINATOR_DELBA) {
        if (pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.en_ba_status == DMAC_BA_INIT) {
            oal_spin_unlock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));
            return OAL_SUCC;
        }
    }

    /* 申请DEL_BA管理帧内存 */
    pst_delba = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_delba == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_tx_delba_etc::pst_delba null.}");
        oal_spin_unlock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEM_NETBUF_TRACE(pst_delba, OAL_TRUE);

    OAL_NETBUF_PREV(pst_delba) = OAL_PTR_NULL;
    OAL_NETBUF_NEXT(pst_delba) = OAL_PTR_NULL;

    /* 调用封装管理帧接口 */
    us_frame_len = hmac_mgmt_encap_delba_etc(pst_hmac_vap,
                                             (oal_uint8 *)OAL_NETBUF_HEADER(pst_delba),
                                             pst_action_args->puc_arg5, uc_tidno,
                                             en_initiator, (oal_uint8)pst_action_args->ul_arg3);
    memset_s((oal_uint8 *)&st_wlan_ctx_action, OAL_SIZEOF(st_wlan_ctx_action), 0, OAL_SIZEOF(st_wlan_ctx_action));
    st_wlan_ctx_action.us_frame_len = us_frame_len;
    st_wlan_ctx_action.uc_hdr_len = MAC_80211_FRAME_LEN;
    st_wlan_ctx_action.en_action_category = MAC_ACTION_CATEGORY_BA;
    st_wlan_ctx_action.uc_action = MAC_BA_ACTION_DELBA;
    st_wlan_ctx_action.us_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;
    st_wlan_ctx_action.uc_tidno = uc_tidno;
    st_wlan_ctx_action.uc_initiator = en_initiator;

    if (EOK != memcpy_s((oal_uint8 *)(oal_netbuf_data(pst_delba) + us_frame_len),
                        OAL_SIZEOF(dmac_ctx_action_event_stru),
                        (oal_uint8 *)&st_wlan_ctx_action,
                        OAL_SIZEOF(dmac_ctx_action_event_stru))) {
        OAM_ERROR_LOG0(0, OAM_SF_BA, "hmac_mgmt_tx_delba_etc::memcpy fail!");
        /* 释放管理帧内存到netbuf内存池 */
        oal_netbuf_free(pst_delba);
        /* 对tid对应的tx BA会话状态解锁 */
        oal_spin_unlock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));
        return OAL_FAIL;
    }
    oal_netbuf_put(pst_delba, (us_frame_len + OAL_SIZEOF(dmac_ctx_action_event_stru)));

#ifdef _PRE_WLAN_1103_CHR
    hmac_chr_set_del_ba_info(uc_tidno, (oal_uint16)pst_action_args->ul_arg3);
#endif
    /* 初始化CB */
    memset_s(oal_netbuf_cb(pst_delba), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_delba);
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_frame_len + OAL_SIZEOF(dmac_ctx_action_event_stru);
    MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_ACTION;
    MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = WLAN_ACTION_BA_DELBA;

    /* 抛事件，到DMAC模块发送 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_tx_event_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_tx_delba_etc::pst_event_mem null.}");
        /* 释放管理帧内存到netbuf内存池 */
        oal_netbuf_free(pst_delba);
        /* 对tid对应的tx BA会话状态解锁 */
        oal_spin_unlock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_hmac_to_dmac_ctx_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_hmac_to_dmac_ctx_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT,
                       OAL_SIZEOF(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    /* 填写事件payload */
    pst_tx_event = (dmac_tx_event_stru *)(pst_hmac_to_dmac_ctx_event->auc_event_data);
    pst_tx_event->pst_netbuf = pst_delba;
    pst_tx_event->us_frame_len = us_frame_len + OAL_SIZEOF(dmac_ctx_action_event_stru);

    /* 分发 */
    ul_ret = frw_event_dispatch_event_etc(pst_event_mem);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                       "{hmac_mgmt_tx_delba_etc::frw_event_dispatch_event_etc failed[%d].}", ul_ret);
        oal_netbuf_free(pst_delba);
        FRW_EVENT_FREE(pst_event_mem);
        oal_spin_unlock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));
        return ul_ret;
    }

    FRW_EVENT_FREE(pst_event_mem);

    if (en_initiator == MAC_RECIPIENT_DELBA) {
        /* 更新对应的TID信息 */
        hmac_ba_reset_rx_handle_etc(pst_device, pst_hmac_user, uc_tidno, OAL_FALSE);
    } else {
        /* 更新对应的TID信息 */
        pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.en_ba_status = DMAC_BA_INIT;
        pst_hmac_user->auc_ba_flag[uc_tidno] = 0;

        hmac_tx_ba_session_decr_etc(pst_hmac_vap, pst_hmac_user->ast_tid_info[uc_tidno].uc_tid_no);
        /* 还原设置AMPDU下AMSDU的支持情况 */
        HMAC_USER_SET_AMSDU_SUPPORT(pst_hmac_user, uc_tidno);
    }

    /* 对tid对应的tx BA会话状态解锁 */
    oal_spin_unlock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));

    return OAL_SUCC;
}


oal_uint32 hmac_mgmt_rx_addba_req_etc(hmac_vap_stru *pst_hmac_vap,
                                      hmac_user_stru *pst_hmac_user,
                                      oal_uint8 *puc_payload)
{
    mac_device_stru *pst_device = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    oal_uint8 uc_tid;
    oal_uint8 uc_status;
    hmac_ba_rx_stru *pst_ba_rx_stru = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    if (OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_hmac_user, puc_payload)) {
        OAM_ERROR_LOG3(0, OAM_SF_BA,
                       "{hmac_mgmt_rx_addba_req_etc::addba req receive failed, null param, 0x%x 0x%x 0x%x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_hmac_user, (uintptr_t)puc_payload);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);

    /* 11n以上能力才可接收ampdu */
    if ((!(pst_mac_vap->en_protocol >= WLAN_HT_MODE)) ||
        (!(pst_hmac_user->st_user_base_info.en_protocol_mode >= WLAN_HT_MODE))) {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_mgmt_rx_addba_req_etc::ampdu not supprot or not open,vap protocol mode:%d,user protocol mode:%d.}",
            pst_mac_vap->en_protocol, pst_hmac_user->st_user_base_info.en_protocol_mode);
        return OAL_SUCC;
    }

    /* 获取device结构 */
    pst_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);

    if (pst_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                       "{hmac_mgmt_rx_addba_req_etc::addba req receive failed, pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /******************************************************************/
    /*       ADDBA Request Frame - Frame Body                         */
    /* --------------------------------------------------------------- */
    /* | Category | Action | Dialog | Parameters | Timeout | SSN     | */
    /* --------------------------------------------------------------- */
    /* | 1        | 1      | 1      | 2          | 2       | 2       | */
    /* --------------------------------------------------------------- */
    /*                                                                */
    /******************************************************************/
    uc_tid = (puc_payload[3] & 0x3C) >> 2;
    if (uc_tid >= WLAN_TID_MAX_NUM) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_rx_addba_req_etc::addba req receive failed, tid %d overflow.}", uc_tid);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    if (pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info != OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_rx_addba_req_etc::addba req received, but tid [%d] already set up.}", uc_tid);
        hmac_ba_reset_rx_handle_etc(pst_device, pst_hmac_user, uc_tid, OAL_FALSE);
    }

    /* ADDBA REQ baw size来区分重排序队列是64还是256 */
    pst_ba_rx_stru = (hmac_ba_rx_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,
                                                      (oal_uint16)OAL_SIZEOF(hmac_ba_rx_stru),
                                                      OAL_TRUE);
    if (pst_ba_rx_stru == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_rx_addba_req_etc:: mem alloc failed.tid[%d],baw[%d]}", uc_tid);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info = pst_ba_rx_stru;

    /* Ba会话参数初始化 */
    pst_ba_rx_stru->en_ba_status = DMAC_BA_INIT;
    pst_ba_rx_stru->uc_dialog_token = puc_payload[2];
    pst_ba_rx_stru->uc_ba_policy = (puc_payload[3] & 0x02) >> 1;
    pst_ba_rx_stru->us_ba_timeout = puc_payload[5] | (puc_payload[6] << 8);
    /* 初始化接收窗口 */
    pst_ba_rx_stru->us_baw_start = (puc_payload[7] >> 4) | (puc_payload[8] << 4);
    pst_ba_rx_stru->us_baw_size = (puc_payload[3] & 0xC0) >> 6;
    pst_ba_rx_stru->us_baw_size |= (puc_payload[4] << 2);

    hmac_ba_rx_hdl_init_etc(pst_hmac_vap, pst_hmac_user, uc_tid);

#ifndef _PRE_PROFILING_MODE
    /* profiling测试中，接收端不删除ba */
    FRW_TIMER_CREATE_TIMER(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer), hmac_ba_timeout_fn_etc,
                           pst_hmac_vap->us_rx_timeout[WLAN_WME_TID_TO_AC(uc_tid)],
                           &(pst_ba_rx_stru->st_alarm_data),
                           OAL_FALSE,
                           OAM_MODULE_ID_HMAC,
                           pst_device->ul_core_id);
#endif
    mac_mib_incr_RxBASessionNumber(pst_mac_vap);

    /* 判断建立能否成功 */
    uc_status = hmac_mgmt_check_set_rx_ba_ok_etc(pst_hmac_vap, pst_hmac_user,
                                                 pst_ba_rx_stru, pst_device,
                                                 &(pst_hmac_user->ast_tid_info[uc_tid]));
    if (uc_status == MAC_SUCCESSFUL_STATUSCODE) {
        pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info->en_ba_status = DMAC_BA_INPROGRESS;
    }

    ul_ret = hmac_mgmt_tx_addba_rsp_etc(pst_hmac_vap, pst_hmac_user, pst_ba_rx_stru, uc_tid, uc_status);

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_BA,
        "{hmac_mgmt_rx_addba_req_etc::process addba req receive and send addba rsp, uc_tid[%d], \
        uc_status[%d], baw_size[%d], RXAMSDU[%d]}\r\n",
        uc_tid, uc_status, pst_ba_rx_stru->us_baw_size, pst_ba_rx_stru->en_amsdu_supp);

    if ((uc_status != MAC_SUCCESSFUL_STATUSCODE) || (ul_ret != 0)) {
        /* pst_hmac_user->ast_tid_info[uc_tid].pst_ba_rx_info修改为在函数中置空，与其他
                调用一致 */
        hmac_ba_reset_rx_handle_etc(pst_device, pst_hmac_user, uc_tid, OAL_FALSE);
    }

    return OAL_SUCC;
}


oal_uint32 hmac_mgmt_rx_addba_rsp_etc(hmac_vap_stru *pst_hmac_vap,
                                      hmac_user_stru *pst_hmac_user,
                                      oal_uint8 *puc_payload)
{
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL; /* 申请事件返回的内存指针 */
    frw_event_stru *pst_hmac_to_dmac_crx_sync = OAL_PTR_NULL;
    dmac_ctx_action_event_stru *pst_rx_addba_rsp_event = OAL_PTR_NULL;
    oal_uint8 uc_tidno;
    hmac_tid_stru *pst_tid = OAL_PTR_NULL;
    oal_uint8 uc_dialog_token;
    oal_uint8 uc_ba_policy;
    oal_uint16 us_baw_size;
    oal_uint8 uc_ba_status;
    oal_bool_enum_uint8 en_amsdu_supp;

    if (OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_hmac_user, puc_payload)) {
        OAM_ERROR_LOG3(0, OAM_SF_BA, "{hmac_mgmt_rx_addba_rsp_etc::null param, %x %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_hmac_user, (uintptr_t)puc_payload);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);

    /* 获取device结构 */
    pst_mac_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_rx_addba_rsp_etc::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /******************************************************************/
    /*       ADDBA Response Frame - Frame Body                        */
    /* --------------------------------------------------------------- */
    /* | Category | Action | Dialog | Status  | Parameters | Timeout | */
    /* --------------------------------------------------------------- */
    /* | 1        | 1      | 1      | 2       | 2          | 2       | */
    /* --------------------------------------------------------------- */
    /*                                                                */
    /******************************************************************/

    uc_tidno = (puc_payload[5] & 0x3C) >> 2;
    /* 协议支持tid为0~15,02只支持tid0~7 */
    if (uc_tidno >= WLAN_TID_MAX_NUM) {
        /* 对于tid > 7的resp直接忽略 */
        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_rx_addba_rsp_etc::addba rsp tid[%d]} token[%d] state[%d]",
                         uc_tidno, puc_payload[2], puc_payload[3]);
        return OAL_SUCC;
    }

    uc_dialog_token = puc_payload[2];
    uc_ba_status = puc_payload[3];
    uc_ba_policy = ((puc_payload[5] & 0x02) >> 1);
    en_amsdu_supp = puc_payload[5] & BIT0;

    pst_tid = &(pst_hmac_user->ast_tid_info[uc_tidno]);

    /* 对tid对应的tx BA会话状态加锁 */
    oal_spin_lock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));

    /* BA状态成功，但token、policy不匹配，需要删除聚合 */
    
    if ((pst_tid->st_ba_tx_info.en_ba_status == DMAC_BA_INPROGRESS) && (uc_ba_status == MAC_SUCCESSFUL_STATUSCODE)) {
        if ((uc_dialog_token != pst_tid->st_ba_tx_info.uc_dialog_token) ||
            (uc_ba_policy != pst_tid->st_ba_tx_info.uc_ba_policy)) {
            /* 对tid对应的tx BA会话状态解锁 */
            oal_spin_unlock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                "{hmac_mgmt_rx_addba_rsp_etc::addba rsp tid[%d],status SUCC,but token/policy wr}", uc_tidno);
            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_BA,
                "{hmac_mgmt_rx_addba_rsp_etc::rsp policy[%d],req policy[%d], rsp dialog[%d], req dialog[%d]}",
                uc_ba_policy, pst_tid->st_ba_tx_info.uc_ba_policy,
                uc_dialog_token, pst_tid->st_ba_tx_info.uc_dialog_token);
            return OAL_SUCC;
        }
    }

    /* 停止计时器 */
    if (pst_tid->st_ba_tx_info.st_addba_timer.en_is_registerd == OAL_TRUE) {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&pst_tid->st_ba_tx_info.st_addba_timer);
    }

    if (pst_tid->st_ba_tx_info.en_ba_status == DMAC_BA_INIT) {
        /* 对tid对应的tx BA会话状态解锁 */
        oal_spin_unlock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));
        hmac_tx_ba_del(pst_hmac_vap, pst_hmac_user, uc_tidno);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_mgmt_rx_addba_rsp_etc::addba rsp is received when ba status is DMAC_BA_INIT.tid[%d]}", uc_tidno);
        return OAL_SUCC;
    }

    if (pst_tid->st_ba_tx_info.en_ba_status == DMAC_BA_COMPLETE) {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_mgmt_rx_addba_rsp_etc::addba rsp is received when ba status is \
            DMAC_BA_COMPLETE or uc_dialog_token wrong. tid[%d], status[%d]}",
            uc_tidno, pst_tid->st_ba_tx_info.en_ba_status);
        /* 对tid对应的tx BA会话状态解锁 */
        oal_spin_unlock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));
        return OAL_SUCC;
    }

    /* 抛事件到DMAC处理 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_ctx_action_event_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BA, "{hmac_mgmt_rx_addba_rsp_etc::pst_event_mem null.}");
        oal_spin_unlock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_hmac_to_dmac_crx_sync = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_hmac_to_dmac_crx_sync->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_BA_SYNC,
                       OAL_SIZEOF(dmac_ctx_action_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    /* 获取帧体信息，由于DMAC的同步，填写事件payload */
    pst_rx_addba_rsp_event = (dmac_ctx_action_event_stru *)(pst_hmac_to_dmac_crx_sync->auc_event_data);
    pst_rx_addba_rsp_event->en_action_category = MAC_ACTION_CATEGORY_BA;
    pst_rx_addba_rsp_event->uc_action = MAC_BA_ACTION_ADDBA_RSP;
    pst_rx_addba_rsp_event->us_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;
    pst_rx_addba_rsp_event->uc_status = uc_ba_status;
    pst_rx_addba_rsp_event->uc_tidno = uc_tidno;
    pst_rx_addba_rsp_event->uc_dialog_token = uc_dialog_token;
    pst_rx_addba_rsp_event->uc_ba_policy = uc_ba_policy;
    pst_rx_addba_rsp_event->us_ba_timeout = puc_payload[7] | (puc_payload[8] << 8);
    pst_rx_addba_rsp_event->en_amsdu_supp = en_amsdu_supp;

    us_baw_size = (oal_uint16)(((puc_payload[5] & 0xC0) >> 6) | (puc_payload[6] << 2));
    if ((us_baw_size == 0) || (us_baw_size > WLAN_AMPDU_TX_MAX_BUF_SIZE)) {
        us_baw_size = WLAN_AMPDU_TX_MAX_BUF_SIZE;
    }

    pst_rx_addba_rsp_event->us_baw_size = us_baw_size;

    /* 分发 */
    frw_event_dispatch_event_etc(pst_event_mem);

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);

    /* 先抛事件，再处理host BA句柄，防止异步发送ADDBA REQ */
    if (uc_ba_status == MAC_SUCCESSFUL_STATUSCODE) {
        /* 设置hmac模块对应的BA句柄的信息 */
        pst_tid->st_ba_tx_info.en_ba_status = DMAC_BA_COMPLETE;
        pst_tid->st_ba_tx_info.uc_addba_attemps = 0;

        /*lint -e502*/
        if (en_amsdu_supp && mac_mib_get_AmsduPlusAmpduActive(pst_mac_vap)) {
            HMAC_USER_SET_AMSDU_SUPPORT(pst_hmac_user, uc_tidno);
        } else {
            HMAC_USER_SET_AMSDU_NOT_SUPPORT(pst_hmac_user, uc_tidno);
        }
        /*lint +e502*/
    } else { /* BA被拒绝 */
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_BA,
            "{hmac_mgmt_rx_addba_rsp_etc::addba rsp  status err[%d].tid[%d],DEL BA.}", uc_ba_status, uc_tidno);

        hmac_tx_ba_session_decr_etc(pst_hmac_vap, uc_tidno);

        /* 先抛事件删除dmac旧BA句柄后，再重置HMAC模块信息，确保删除dmac ba事件在下一次ADDBA REQ事件之前到达dmac */
        pst_tid->st_ba_tx_info.en_ba_status = DMAC_BA_INIT;
    }

    /* 对tid对应的tx BA会话状态解锁 */
    oal_spin_unlock_bh(&(pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.st_ba_status_lock));

    return OAL_SUCC;
}


oal_uint32 hmac_mgmt_rx_delba_etc(hmac_vap_stru *pst_hmac_vap,
                                  hmac_user_stru *pst_hmac_user,
                                  oal_uint8 *puc_payload)
{
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL; /* 申请事件返回的内存指针 */
    frw_event_stru *pst_hmac_to_dmac_crx_sync = OAL_PTR_NULL;
    dmac_ctx_action_event_stru *pst_wlan_crx_action = OAL_PTR_NULL;
    mac_device_stru *pst_device = OAL_PTR_NULL;
    hmac_tid_stru *pst_tid = OAL_PTR_NULL;
    oal_uint8 uc_tid;
    oal_uint8 uc_initiator;
    oal_uint16 us_reason;

    if (OAL_UNLIKELY(OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_hmac_user, puc_payload))) {
        OAM_ERROR_LOG3(0, OAM_SF_BA, "{hmac_mgmt_rx_delba_etc::null param, %x %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_hmac_user, (uintptr_t)puc_payload);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取device结构 */
    pst_device = mac_res_get_dev_etc(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(pst_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                       "{hmac_mgmt_rx_delba_etc::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /************************************************/
    /*       DELBA Response Frame - Frame Body      */
    /* -------------------------------------------- */
    /* | Category | Action | Parameters | Reason  | */
    /* -------------------------------------------- */
    /* | 1        | 1      | 2          | 2       | */
    /* -------------------------------------------- */
    /*                                              */
    /************************************************/
    uc_tid = (puc_payload[3] & 0xF0) >> 4;
    uc_initiator = (puc_payload[3] & 0x08) >> 3;
    us_reason = (puc_payload[4] & 0xFF) | ((puc_payload[5] << 8) & 0xFF00);

    /* tid保护，避免数组越界 */
    if (uc_tid >= WLAN_TID_MAX_NUM) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                         "{hmac_mgmt_rx_delba_etc::delba receive failed, tid %d overflow.}", uc_tid);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }
    pst_tid = &(pst_hmac_user->ast_tid_info[uc_tid]);

    OAM_WARNING_LOG3(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                     "{hmac_mgmt_rx_delba_etc::receive delba from peer sta, tid[%d], uc_initiator[%d], reason[%d].}",
                     uc_tid, uc_initiator, us_reason);

    /* 对tid对应的tx BA会话状态加锁 */
    oal_spin_lock_bh(&(pst_tid->st_ba_tx_info.st_ba_status_lock));

    /* 重置BA发送会话 */
    if (uc_initiator == MAC_RECIPIENT_DELBA) {
        if (pst_tid->st_ba_tx_info.en_ba_status == DMAC_BA_INIT) {
            oal_spin_unlock_bh(&(pst_tid->st_ba_tx_info.st_ba_status_lock));
            return OAL_SUCC;
        }

        pst_hmac_user->auc_ba_flag[uc_tid] = 0;

        /* 还原设置AMPDU下AMSDU的支持情况 */
        HMAC_USER_SET_AMSDU_SUPPORT(pst_hmac_user, uc_tid);

        hmac_tx_ba_session_decr_etc(pst_hmac_vap, pst_hmac_user->ast_tid_info[uc_tid].uc_tid_no);
    } else { /* 重置BA接收会话 */
        hmac_ba_reset_rx_handle_etc(pst_device, pst_hmac_user, uc_tid, OAL_FALSE);
        OAM_INFO_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                      "{hmac_mgmt_rx_delba_etc::rcv rx dir del ba.}\r\n");
    }

    /* 抛事件到DMAC处理 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_ctx_action_event_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                       "{hmac_mgmt_rx_delba_etc::pst_event_mem null.}");
        oal_spin_unlock_bh(&(pst_tid->st_ba_tx_info.st_ba_status_lock));
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_hmac_to_dmac_crx_sync = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_hmac_to_dmac_crx_sync->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_BA_SYNC,
                       OAL_SIZEOF(dmac_ctx_action_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_hmac_vap->st_vap_base_info.uc_chip_id,
                       pst_hmac_vap->st_vap_base_info.uc_device_id,
                       pst_hmac_vap->st_vap_base_info.uc_vap_id);

    /* 填写事件payload */
    pst_wlan_crx_action = (dmac_ctx_action_event_stru *)(pst_hmac_to_dmac_crx_sync->auc_event_data);
    pst_wlan_crx_action->en_action_category = MAC_ACTION_CATEGORY_BA;
    pst_wlan_crx_action->uc_action = MAC_BA_ACTION_DELBA;
    pst_wlan_crx_action->us_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;

    pst_wlan_crx_action->uc_tidno = uc_tid;
    pst_wlan_crx_action->uc_initiator = uc_initiator;

    /* 分发 */
    frw_event_dispatch_event_etc(pst_event_mem);

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);

    /* DELBA事件先处理再改状态,防止addba req先处理 */
    if (uc_initiator == MAC_RECIPIENT_DELBA) {
        pst_tid->st_ba_tx_info.en_ba_status = DMAC_BA_INIT;
    }

    /* 对tid对应的tx BA会话状态解锁 */
    oal_spin_unlock_bh(&(pst_tid->st_ba_tx_info.st_ba_status_lock));

    return OAL_SUCC;
}


oal_uint32 hmac_mgmt_tx_addba_timeout_etc(oal_void *p_arg)
{
    hmac_vap_stru *pst_vap = OAL_PTR_NULL; /* vap指针 */
    oal_uint8 *puc_da = OAL_PTR_NULL;      /* 保存用户目的地址的指针 */
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    mac_action_mgmt_args_stru st_action_args;
    dmac_ba_alarm_stru *pst_alarm_data = OAL_PTR_NULL;

    if (p_arg == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_BA, "{hmac_mgmt_tx_addba_timeout_etc::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_alarm_data = (dmac_ba_alarm_stru *)p_arg;

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(pst_alarm_data->us_mac_user_idx);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_BA, "{hmac_mgmt_tx_addba_timeout_etc::pst_hmac_user[%d] null.}",
                         pst_alarm_data->us_mac_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    puc_da = pst_hmac_user->st_user_base_info.auc_user_mac_addr;

    pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_alarm_data->uc_vap_id);
    if (OAL_UNLIKELY(pst_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_BA,
                       "{hmac_mgmt_tx_addba_timeout_etc::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 生成DELBA帧 */
    st_action_args.uc_category = MAC_ACTION_CATEGORY_BA;
    st_action_args.uc_action = MAC_BA_ACTION_DELBA;
    st_action_args.ul_arg1 = pst_alarm_data->uc_tid; /* 该数据帧对应的TID号 */
    st_action_args.ul_arg2 = MAC_ORIGINATOR_DELBA;   /* DELBA中，触发删除BA会话的发起端 */
    st_action_args.ul_arg3 = MAC_QSTA_TIMEOUT;       /* DELBA中代表删除reason */
    st_action_args.puc_arg5 = puc_da;                /* DELBA中代表目的地址 */

    OAM_WARNING_LOG1(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_BA,
        "{hmac_mgmt_tx_addba_timeout_etc::TID[%d] add timeout.}", pst_alarm_data->uc_tid);
    hmac_mgmt_tx_delba_etc(pst_vap, pst_hmac_user, &st_action_args);

    return OAL_SUCC;
}


oal_uint32 hmac_mgmt_tx_ampdu_start_etc(hmac_vap_stru *pst_hmac_vap,
                                        hmac_user_stru *pst_hmac_user,
                                        mac_priv_req_args_stru *pst_priv_req)
{
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL; /* 申请事件返回的内存指针 */
    frw_event_stru *pst_crx_priv_req_event = OAL_PTR_NULL;
    mac_priv_req_args_stru *pst_rx_ampdu_start_event = OAL_PTR_NULL;
    oal_uint8 uc_tidno;
    hmac_tid_stru *pst_tid = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    if (OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_hmac_user, pst_priv_req)) {
        OAM_ERROR_LOG3(0, OAM_SF_AMPDU, "{hmac_mgmt_tx_ampdu_start_etc::param null, %x %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_hmac_user, (uintptr_t)pst_priv_req);
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_tidno = pst_priv_req->uc_arg1;
    pst_tid = &(pst_hmac_user->ast_tid_info[uc_tidno]);

    /* AMPDU为NORMAL ACK时，对应的BA会话没有建立，则返回 */
    if (pst_priv_req->uc_arg3 == WLAN_TX_NORMAL_ACK) {
        if (pst_tid->st_ba_tx_info.en_ba_status == DMAC_BA_INIT) {
            OAM_INFO_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMPDU,
                          "{hmac_mgmt_tx_ampdu_start_etc::normal ack but ba session is not build.}");
            return OAL_SUCC;
        }
    }

    /* 抛事件到DMAC处理 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(mac_priv_req_args_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMPDU,
                       "{hmac_mgmt_tx_ampdu_start_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_crx_priv_req_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_crx_priv_req_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_PRIV_REQ,
                       OAL_SIZEOF(mac_priv_req_args_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_hmac_vap->st_vap_base_info.uc_chip_id,
                       pst_hmac_vap->st_vap_base_info.uc_device_id,
                       pst_hmac_vap->st_vap_base_info.uc_vap_id);

    /* 获取设置AMPDU的参数，到dmac进行设置 */
    pst_rx_ampdu_start_event = (mac_priv_req_args_stru *)(pst_crx_priv_req_event->auc_event_data);
    pst_rx_ampdu_start_event->uc_type = MAC_A_MPDU_START;
    pst_rx_ampdu_start_event->uc_arg1 = pst_priv_req->uc_arg1;
    pst_rx_ampdu_start_event->uc_arg2 = pst_priv_req->uc_arg2;
    pst_rx_ampdu_start_event->uc_arg3 = pst_priv_req->uc_arg3;
    pst_rx_ampdu_start_event->us_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id; /* 保存的是资源池的索引 */

    /* 分发 */
    ul_ret = frw_event_dispatch_event_etc(pst_event_mem);

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);

    return ul_ret;
}


oal_uint32 hmac_mgmt_tx_ampdu_end_etc(hmac_vap_stru *pst_hmac_vap,
                                      hmac_user_stru *pst_hmac_user,
                                      mac_priv_req_args_stru *pst_priv_req)
{
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL; /* 申请事件返回的内存指针 */
    frw_event_stru *pst_crx_priv_req_event = OAL_PTR_NULL;
    mac_priv_req_args_stru *pst_rx_ampdu_end_event = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    if (OAL_ANY_NULL_PTR3(pst_hmac_vap, pst_hmac_user, pst_priv_req)) {
        OAM_ERROR_LOG3(0, OAM_SF_AMPDU, "{hmac_mgmt_tx_ampdu_end_etc::param null, %d %d %d.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_hmac_user, (uintptr_t)pst_priv_req);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛事件到DMAC处理 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(mac_priv_req_args_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMPDU,
                       "{hmac_mgmt_tx_ampdu_end_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_crx_priv_req_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_crx_priv_req_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_PRIV_REQ,
                       OAL_SIZEOF(mac_priv_req_args_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_hmac_vap->st_vap_base_info.uc_chip_id,
                       pst_hmac_vap->st_vap_base_info.uc_device_id,
                       pst_hmac_vap->st_vap_base_info.uc_vap_id);

    /* 获取设置AMPDU的参数，到dmac进行设置 */
    pst_rx_ampdu_end_event = (mac_priv_req_args_stru *)(pst_crx_priv_req_event->auc_event_data);
    pst_rx_ampdu_end_event->uc_type = MAC_A_MPDU_END;                                   /* 类型 */
    pst_rx_ampdu_end_event->uc_arg1 = pst_priv_req->uc_arg1;                            /* tid no */
    pst_rx_ampdu_end_event->us_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id; /* 保存的是资源池的索引 */

    /* 分发 */
    ul_ret = frw_event_dispatch_event_etc(pst_event_mem);

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);

    return ul_ret;
}


oal_uint32 hmac_tx_mgmt_send_event_etc(mac_vap_stru *pst_vap, oal_netbuf_stru *pst_mgmt_frame, oal_uint16 us_frame_len)
{
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    oal_uint32 ul_return;
    dmac_tx_event_stru *pst_ctx_stru = OAL_PTR_NULL;

    if (pst_vap == OAL_PTR_NULL || pst_mgmt_frame == OAL_PTR_NULL) {
        OAM_ERROR_LOG2(0, OAM_SF_SCAN, "{hmac_tx_mgmt_send_event_etc::param null, %x %x.}",
                       (uintptr_t)pst_vap, (uintptr_t)pst_mgmt_frame);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛事件给DMAC,让DMAC完成配置VAP创建 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_tx_event_stru));
    if (OAL_UNLIKELY(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_tx_mgmt_send_event_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT,
                       OAL_SIZEOF(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_vap->uc_chip_id,
                       pst_vap->uc_device_id,
                       pst_vap->uc_vap_id);

    pst_ctx_stru = (dmac_tx_event_stru *)pst_event->auc_event_data;
    pst_ctx_stru->pst_netbuf = pst_mgmt_frame;
    pst_ctx_stru->us_frame_len = us_frame_len;

#ifdef _PRE_WLAN_FEATURE_SNIFFER
    proc_sniffer_write_file (NULL, 0, (oal_uint8 *)(OAL_NETBUF_HEADER(pst_mgmt_frame)), us_frame_len, 1);
#endif

    ul_return = frw_event_dispatch_event_etc(pst_event_mem);
    if (ul_return != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_tx_mgmt_send_event_etc::frw_event_dispatch_event_etc failed[%d].}",
                       ul_return);
        FRW_EVENT_FREE(pst_event_mem);
        return ul_return;
    }

    /* 释放事件 */
    FRW_EVENT_FREE(pst_event_mem);

    return OAL_SUCC;
}


oal_uint32 hmac_mgmt_reset_psm_etc(mac_vap_stru *pst_vap, oal_uint16 us_user_id)
{
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    oal_uint16 *pus_user_id = OAL_PTR_NULL;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;

    if (pst_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{hmac_mgmt_reset_psm_etc::pst_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 在这里直接做重置的一些操作，不需要再次同步 */
    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(us_user_id);
    if (OAL_UNLIKELY(pst_hmac_user == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_PWR,
                       "{hmac_mgmt_reset_psm_etc::pst_hmac_user[%d] null.}",
                       us_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(oal_uint16));
    if (OAL_UNLIKELY(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_PWR, "{hmac_mgmt_reset_psm_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_RESET_PSM,
                       OAL_SIZEOF(oal_uint16),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_vap->uc_chip_id,
                       pst_vap->uc_device_id,
                       pst_vap->uc_vap_id);

    pus_user_id = (oal_uint16 *)pst_event->auc_event_data;

    *pus_user_id = us_user_id;

    frw_event_dispatch_event_etc(pst_event_mem);

    FRW_EVENT_FREE(pst_event_mem);

    return OAL_SUCC;
}

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
OAL_STATIC oal_uint32 hmac_sa_query_del_user(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user)
{
    wlan_vap_mode_enum_uint8 en_vap_mode;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    mac_sa_query_stru *pst_sa_query_info;

    if (OAL_ANY_NULL_PTR2(pst_mac_vap, pst_hmac_user)) {
        OAM_ERROR_LOG2(0, OAM_SF_PMF, "{hmac_sa_query_del_user::param null, %x %x.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)pst_hmac_user);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_device_id, OAM_SF_PMF, "{hmac_sa_query_del_user::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* pending SA Query requests 计数器清零 & sa query流程开始时间清零 */
    pst_sa_query_info = &pst_hmac_user->st_sa_query_info;
    pst_sa_query_info->ul_sa_query_count = 0;
    pst_sa_query_info->ul_sa_query_start_time = 0;

    /* 修改 state & 删除 user */
    en_vap_mode = pst_mac_vap->en_vap_mode;
    switch (en_vap_mode) {
        case WLAN_VAP_MODE_BSS_AP: {
            /* 抛事件上报内核，已经去关联某个STA */
            hmac_handle_disconnect_rsp_ap_etc(pst_hmac_vap, pst_hmac_user);
        }
        break;

        case WLAN_VAP_MODE_BSS_STA: {
            /* 上报内核sta已经和某个ap去关联 */
            hmac_sta_handle_disassoc_rsp_etc(pst_hmac_vap, MAC_DEAUTH_LV_SS);
        }
        break;
        default:
            break;
    }

    /* 删除user */
    ul_ret = hmac_user_del_etc(&pst_hmac_vap->st_vap_base_info, pst_hmac_user);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_device_id, OAM_SF_PMF,
                       "{hmac_sa_query_del_user::hmac_user_del_etc failed[%d].}", ul_ret);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_send_sa_query_req(mac_vap_stru *pst_mac_vap,
                                             hmac_user_stru *pst_hmac_user,
                                             oal_bool_enum_uint8 en_is_protected,
                                             oal_uint16 us_init_trans_id)
{
    oal_uint16 us_sa_query_len;
    oal_netbuf_stru *pst_sa_query = OAL_PTR_NULL;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    if (OAL_ANY_NULL_PTR2(pst_mac_vap, pst_hmac_user)) {
        OAM_ERROR_LOG2(0, OAM_SF_PMF, "{hmac_send_sa_query_req::param null, %x %x.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)pst_hmac_user);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请SA Query 帧空间 */
    pst_sa_query = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,
                                                           WLAN_MEM_NETBUF_SIZE2,
                                                           OAL_NETBUF_PRIORITY_MID);
    if (pst_sa_query == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_PMF, "{hmac_send_sa_query_req::pst_sa_query null.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 封装SA Query request帧 */
    memset_s(oal_netbuf_cb(pst_sa_query), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    us_sa_query_len = hmac_encap_sa_query_req_etc(pst_mac_vap,
                                                  (oal_uint8 *)OAL_NETBUF_HEADER(pst_sa_query),
                                                  pst_hmac_user->st_user_base_info.auc_user_mac_addr,
                                                  pst_hmac_user->st_sa_query_info.us_sa_query_trans_id);

    /* 单播管理帧加密 */
    if (en_is_protected == OAL_TRUE) {
        mac_set_protectedframe((oal_uint8 *)OAL_NETBUF_HEADER(pst_sa_query));
    }

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_sa_query); /* 获取cb结构体 */
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_sa_query_len;           /* dmac发送需要的mpdu长度 */
    /* 发送完成需要获取user结构体 */
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = pst_hmac_user->st_user_base_info.us_assoc_id;

    oal_netbuf_put(pst_sa_query, us_sa_query_len);

    /* Buffer this frame in the Memory Queue for transmission */
    ul_ret = hmac_tx_mgmt_send_event_etc(pst_mac_vap, pst_sa_query, us_sa_query_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(pst_sa_query);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PMF,
                         "{hmac_send_sa_query_req::hmac_tx_mgmt_send_event_etc failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

oal_uint32 hmac_pmf_check_err_code_etc(mac_user_stru *pst_user_base_info,
                                       oal_bool_enum_uint8 en_is_protected,
                                       oal_uint8 *puc_mac_hdr)
{
    oal_bool_enum_uint8 en_aim_err_code;
    oal_uint16 us_err_code;

    us_err_code = OAL_MAKE_WORD16(puc_mac_hdr[MAC_80211_FRAME_LEN], puc_mac_hdr[MAC_80211_FRAME_LEN + 1]);
    en_aim_err_code = ((us_err_code == MAC_NOT_AUTHED) || (us_err_code == MAC_NOT_ASSOCED)) ? OAL_TRUE : OAL_FALSE;

    if ((pst_user_base_info->st_cap_info.bit_pmf_active == OAL_TRUE) &&
        (en_aim_err_code == OAL_TRUE || en_is_protected == OAL_FALSE)) {
        return OAL_SUCC;
    }

    return OAL_FAIL;
}


oal_uint32 hmac_sa_query_interval_timeout_etc(oal_void *p_arg)
{
    hmac_user_stru *pst_hmac_user;
    hmac_vap_stru *pst_hmac_vap;
    oal_uint32 ul_relt;
    oal_uint32 ul_now;
    oal_uint32 ul_passed;
    oal_uint32 ul_retry_timeout;

    pst_hmac_user = (hmac_user_stru *)p_arg;
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_sa_query_interval_timeout_etc::invalid param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_hmac_user->st_user_base_info.uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_hmac_user->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_ASSOC) {
        pst_hmac_user->st_sa_query_info.ul_sa_query_count = 0;
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                         "{hmac_sa_query_interval_timeout_etc::USER UNEXPECTED STATE[%d].}",
                         pst_hmac_user->st_user_base_info.en_user_asoc_state);
        return OAL_SUCC;
    }

    ul_now = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    ul_passed = (oal_uint32)OAL_TIME_GET_RUNTIME(pst_hmac_user->st_sa_query_info.ul_sa_query_start_time, ul_now);

    if (ul_passed >= mac_mib_get_dot11AssociationSAQueryMaximumTimeout(&(pst_hmac_vap->st_vap_base_info))) {
        /* change state & ul_sa_query_count=0 */
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                         "{hmac_sa_query_interval_timeout_etc::Deleting user.}");
        ul_relt = hmac_sa_query_del_user(&(pst_hmac_vap->st_vap_base_info), pst_hmac_user);
        if (ul_relt != OAL_SUCC) {
            OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                           "{hmac_sa_query_interval_timeout_etc::hmac_sa_query_del_user failed[%d].}", ul_relt);
            return OAL_ERR_CODE_PMF_SA_QUERY_DEL_USER_FAIL;
        }
        return OAL_SUCC;
    }

    /* 再发送一帧sa query request */
    pst_hmac_user->st_sa_query_info.us_sa_query_trans_id += 1;
    pst_hmac_user->st_sa_query_info.ul_sa_query_count += 1;
    OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                     "{hmac_sa_query_interval_timeout_etc::SA query, trans_id %d.}",
                     pst_hmac_user->st_sa_query_info.us_sa_query_trans_id);

    ul_retry_timeout = mac_mib_get_dot11AssociationSAQueryRetryTimeout(&(pst_hmac_vap->st_vap_base_info));

    /* 设置间隔定时器 */
    FRW_TIMER_CREATE_TIMER(&(pst_hmac_user->st_sa_query_info.st_sa_query_interval_timer),
                           hmac_sa_query_interval_timeout_etc,
                           ul_retry_timeout,
                           (oal_void *)pst_hmac_user,
                           OAL_FALSE,
                           OAM_MODULE_ID_HMAC,
                           pst_hmac_vap->st_vap_base_info.ul_core_id);

    ul_relt = hmac_send_sa_query_req(&(pst_hmac_vap->st_vap_base_info),
                                     pst_hmac_user,
                                     pst_hmac_user->st_sa_query_info.en_is_protected,
                                     pst_hmac_user->st_sa_query_info.us_sa_query_trans_id);
    if (ul_relt != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                       "{hmac_sa_query_interval_timeout_etc::hmac_send_sa_query_req failed[%d].}", ul_relt);
        return OAL_ERR_CODE_PMF_SA_QUERY_REQ_SEND_FAIL;
    }
    return OAL_SUCC;
}


oal_uint32 hmac_start_sa_query_etc(mac_vap_stru *pst_mac_vap,
                                   hmac_user_stru *pst_hmac_user,
                                   oal_bool_enum_uint8 en_is_protected)
{
    oal_uint32 ul_retry_timeout;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_uint16 us_init_trans_id = 0;

    /* 入参判断 */
    if (OAL_ANY_NULL_PTR2(pst_mac_vap, pst_hmac_user)) {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{hmac_start_sa_query_etc::param null, %x %x.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)pst_hmac_user);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 判断vap有无pmf能力 */
    if (pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active != OAL_TRUE) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_start_sa_query_etc::bit_pmf_active is down.}");
        return OAL_ERR_CODE_PMF_DISABLED;
    }

    /* 避免重复启动SA Query流程 */
    if (pst_hmac_user->st_sa_query_info.ul_sa_query_count != 0) {
        OAM_INFO_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_start_sa_query_etc::SA Query is already in process.}");
        return OAL_SUCC;
    }

    /* 获得hmac vap 结构指针 */
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_start_sa_query_etc::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_retry_timeout = mac_mib_get_dot11AssociationSAQueryRetryTimeout(pst_mac_vap);

    /* 记录sa query流程开始时间,单位ms */
    pst_hmac_user->st_sa_query_info.ul_sa_query_start_time = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    /* 获得初始trans_id */
    pst_hmac_user->st_sa_query_info.us_sa_query_trans_id = (oal_uint16)OAL_TIME_GET_STAMP_MS();

    pst_hmac_user->st_sa_query_info.ul_sa_query_count = 1;
    pst_hmac_user->st_sa_query_info.en_is_protected = en_is_protected;

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                     "{hmac_start_sa_query_etc::SA query, trans_id %d.}",
                     pst_hmac_user->st_sa_query_info.us_sa_query_trans_id);

    /* 设置间隔定时器 */
    FRW_TIMER_CREATE_TIMER(&(pst_hmac_user->st_sa_query_info.st_sa_query_interval_timer),
                           hmac_sa_query_interval_timeout_etc,
                           ul_retry_timeout,
                           (oal_void *)pst_hmac_user,
                           OAL_FALSE,
                           OAM_MODULE_ID_HMAC,
                           pst_mac_vap->ul_core_id);

    /* 发送SA Query request，开始查询流程 */
    ul_ret = hmac_send_sa_query_req(pst_mac_vap, pst_hmac_user, en_is_protected, us_init_trans_id);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_start_sa_query_etc::hmac_send_sa_query_req failed[%d].}", ul_ret);
        return OAL_ERR_CODE_PMF_SA_QUERY_REQ_SEND_FAIL;
    }

    return OAL_SUCC;
}


oal_void hmac_send_sa_query_rsp_etc(mac_vap_stru *pst_mac_vap, oal_uint8 *pst_hdr, oal_bool_enum_uint8 en_is_protected)
{
    oal_uint16 us_sa_query_len;
    oal_netbuf_stru *pst_sa_query = 0;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;
    oal_uint16 us_user_idx = MAC_INVALID_USER_ID;
    oal_uint32 ul_ret;

    if (OAL_ANY_NULL_PTR2(pst_mac_vap, pst_hdr)) {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{hmac_send_sa_query_rsp_etc::param null, %d %d.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)pst_hdr);
        return;
    }
    OAM_INFO_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_send_sa_query_rsp_etc::AP ready to TX a sa query rsp.}");

    pst_sa_query = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,
                                                           WLAN_MEM_NETBUF_SIZE2,
                                                           OAL_NETBUF_PRIORITY_MID);
    if (pst_sa_query == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_send_sa_query_rsp_etc::pst_sa_query null.}");
        return;
    }

    memset_s(oal_netbuf_cb(pst_sa_query), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    us_sa_query_len = hmac_encap_sa_query_rsp_etc(pst_mac_vap, pst_hdr, (oal_uint8 *)OAL_NETBUF_HEADER(pst_sa_query));

    /* 单播管理帧加密 */
    if (en_is_protected == OAL_TRUE) {
        mac_set_protectedframe((oal_uint8 *)OAL_NETBUF_HEADER(pst_sa_query));
    }

    ul_ret = mac_vap_find_user_by_macaddr_etc(pst_mac_vap,
                                              ((mac_ieee80211_frame_stru *)pst_hdr)->auc_address2,
                                              &us_user_idx);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_send_sa_query_rsp_etc::mac_vap_find_user_by_macaddr_etc fail,error_code=%d.}", ul_ret);
        oal_netbuf_free(pst_sa_query);
        return;
    }

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_sa_query); /* 获取cb结构体 */
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_sa_query_len;           /* dmac发送需要的mpdu长度 */
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = us_user_idx;            /* 发送完成需要获取user结构体 */

    oal_netbuf_put(pst_sa_query, us_sa_query_len);

    /* Buffer this frame in the Memory Queue for transmission */
    ul_ret = hmac_tx_mgmt_send_event_etc(pst_mac_vap, pst_sa_query, us_sa_query_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(pst_sa_query);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_send_sa_query_rsp_etc::hmac_tx_mgmt_send_event_etc failed[%d].}", ul_ret);
    }

    return;
}
#endif

oal_void hmac_mgmt_send_deauth_frame_etc(mac_vap_stru *pst_mac_vap,
                                         const unsigned char *puc_da,
                                         oal_uint16 us_err_code,
                                         oal_bool_enum_uint8 en_is_protected)
{
    oal_uint16 us_deauth_len;
    oal_netbuf_stru *pst_deauth = 0;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    if (OAL_ANY_NULL_PTR2(pst_mac_vap, puc_da)) {
        OAM_ERROR_LOG2(0, OAM_SF_AUTH, "{hmac_mgmt_send_deauth_frame_etc::param null, %d %d.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)pst_mac_vap);
        return;
    }

    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_AUTH,
                         "{hmac_mgmt_send_deauth_frame_etc:: vap has been down/del, vap_state[%d].}",
                         pst_mac_vap->en_vap_state);
        return;
    }
    pst_deauth = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,
                                                         WLAN_MEM_NETBUF_SIZE2,
                                                         OAL_NETBUF_PRIORITY_MID);

    OAL_MEM_NETBUF_TRACE(pst_deauth, OAL_TRUE);

    if (pst_deauth == OAL_PTR_NULL) {
        /* Reserved Memory pool tried for high priority deauth frames */
        pst_deauth = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,
                                                             WLAN_MEM_NETBUF_SIZE2,
                                                             OAL_NETBUF_PRIORITY_MID);
        if (pst_deauth == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_AUTH, "{hmac_mgmt_send_deauth_frame_etc::pst_deauth null.}");
            return;
        }
    }

    memset_s(oal_netbuf_cb(pst_deauth), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    us_deauth_len = hmac_mgmt_encap_deauth_etc(pst_mac_vap, (oal_uint8 *)OAL_NETBUF_HEADER(pst_deauth),
                                               puc_da, us_err_code);
    if (us_deauth_len == 0) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_AUTH, "{hmac_mgmt_send_deauth_frame_etc:: us_deauth_len = 0.}");
        oal_netbuf_free(pst_deauth);
        return;
    }
    oal_netbuf_put(pst_deauth, us_deauth_len);

    /* MIB variables related to deauthentication are updated */
    /* 增加发送去认证帧时的维测信息 */
    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CONN,
                     "{hmac_mgmt_send_deauth_frame_etc:: DEAUTH TX :  to %2x:XX:XX:XX:%2x:%2x, status code[%d]}",
                     puc_da[0], puc_da[4], puc_da[5], us_err_code);

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_deauth); /* 获取cb结构体 */
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_deauth_len;           /* dmac发送需要的mpdu长度 */
    /* 发送完成需要获取user结构体 */
    ul_ret = mac_vap_set_cb_tx_user_idx(pst_mac_vap, pst_tx_ctl, puc_da);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_AUTH,
                         "(hmac_mgmt_send_deauth_frame_etc::fail to find user by xx:xx:xx:%2x:%2x:%2x.}",
                         puc_da[3],
                         puc_da[4],
                         puc_da[5]);
    }

    /* Buffer this frame in the Memory Queue for transmission */
    ul_ret = hmac_tx_mgmt_send_event_etc(pst_mac_vap, pst_deauth, us_deauth_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(pst_deauth);

        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_AUTH,
                         "{hmac_mgmt_send_deauth_frame_etc::hmac_tx_mgmt_send_event_etc failed[%d].}", ul_ret);
    }
}

oal_uint32 hmac_config_send_deauth_etc(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_da)
{
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;

    if (OAL_UNLIKELY(OAL_ANY_NULL_PTR2(pst_mac_vap, puc_da))) {
        OAM_ERROR_LOG2(0, OAM_SF_AUTH, "{hmac_config_send_deauth_etc::param null, %d %d.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)puc_da);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_user = mac_vap_get_hmac_user_by_addr_etc(pst_mac_vap, puc_da);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_AUTH, "{hmac_config_send_deauth_etc::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_hmac_user->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_ASSOC) {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_AUTH,
                         "{hmac_config_send_deauth_etc::the user is unassociated.}");
        return OAL_FAIL;
    }

    /* 发去认证帧 */
    hmac_mgmt_send_deauth_frame_etc(pst_mac_vap, puc_da, MAC_AUTH_NOT_VALID, OAL_FALSE);

    /* 删除用户 */
    hmac_user_del_etc(pst_mac_vap, pst_hmac_user);

    return OAL_SUCC;
}


oal_void hmac_mgmt_send_disassoc_frame_etc(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_da,
                                           oal_uint16 us_err_code,
                                           oal_bool_enum_uint8 en_is_protected)
{
    oal_uint16 us_disassoc_len;
    oal_netbuf_stru *pst_disassoc = 0;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;
    oal_uint32 ul_ret;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint32 ul_pedding_data = 0;
#endif

    if (OAL_ANY_NULL_PTR2(pst_mac_vap, puc_da)) {
        OAM_ERROR_LOG2(0, OAM_SF_ASSOC, "{hmac_mgmt_send_disassoc_frame_etc::param null, %x %x.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)pst_mac_vap);
        return;
    }

    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_mgmt_send_disassoc_frame_etc:: vap has been down/del, vap_state[%d].}",
                         pst_mac_vap->en_vap_state);
        return;
    }
    pst_disassoc = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,
                                                           WLAN_MEM_NETBUF_SIZE2,
                                                           OAL_NETBUF_PRIORITY_MID);

    if (pst_disassoc == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_mgmt_send_disassoc_frame_etc::pst_disassoc null.}");
        return;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    hmac_config_scan_abort_etc(pst_mac_vap, OAL_SIZEOF(oal_uint32), (oal_uint8 *)&ul_pedding_data);
#endif

    OAL_MEM_NETBUF_TRACE(pst_disassoc, OAL_TRUE);

    memset_s(oal_netbuf_cb(pst_disassoc), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    us_disassoc_len = hmac_mgmt_encap_disassoc_etc(pst_mac_vap, (oal_uint8 *)OAL_NETBUF_HEADER(pst_disassoc),
                                                   puc_da, us_err_code);
    if (us_disassoc_len == 0) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_mgmt_send_disassoc_frame_etc:: us_disassoc_len = 0.}");
        oal_netbuf_free(pst_disassoc);
        return;
    }

    /* 增加发送去关联帧时的维测信息 */
    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CONN,
        "{hmac_mgmt_send_disassoc_frame_etc:: DISASSOC tx, Because of err_code[%d], \
        send disassoc frame to dest addr, da[%2X:XX:XX:XX:%2X:%2X].}",
        us_err_code, puc_da[0], puc_da[4], puc_da[5]);

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_disassoc);
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_disassoc_len;
    /* 填写非法值,发送完成之后获取用户为NULL,直接释放去认证帧  */
    ul_ret = mac_vap_set_cb_tx_user_idx(pst_mac_vap, pst_tx_ctl, puc_da);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "(hmac_mgmt_send_disassoc_frame_etc::fail to find user by xx:xx:xx:0x:0x:0x.}",
                         puc_da[3], puc_da[4],
                         puc_da[5]);
    }

    oal_netbuf_put(pst_disassoc, us_disassoc_len);

    /* 加入发送队列 */
    ul_ret = hmac_tx_mgmt_send_event_etc(pst_mac_vap, pst_disassoc, us_disassoc_len);

    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(pst_disassoc);

        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_mgmt_send_disassoc_frame_etc::hmac_tx_mgmt_send_event_etc failed[%d].}", ul_ret);
    }
}


oal_void hmac_mgmt_update_assoc_user_qos_table_etc(oal_uint8 *puc_payload,
                                                   oal_uint16 us_msg_len,
                                                   hmac_user_stru *pst_hmac_user)
{
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    dmac_ctx_asoc_set_reg_stru st_asoc_set_reg_param = { 0 };
    oal_uint8 *puc_ie = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;

    /* 如果关联用户之前就是wmm使能的，什么都不用做，直接返回  */
    if (pst_hmac_user->st_user_base_info.st_cap_info.bit_qos == OAL_TRUE) {
        OAM_INFO_LOG0(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                      "{hmac_mgmt_update_assoc_user_qos_table_etc::assoc user is wmm cap already.}");
        return;
    }

    mac_user_set_qos_etc(&pst_hmac_user->st_user_base_info, OAL_FALSE);

    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_hmac_user->st_user_base_info.uc_vap_id);
    if (OAL_UNLIKELY(pst_mac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_mgmt_update_assoc_user_qos_table_etc::pst_mac_vap null.}");
        return;
    }

    puc_ie = mac_get_wmm_ie_etc(puc_payload, us_msg_len);

    if (puc_ie != OAL_PTR_NULL) {
        mac_user_set_qos_etc(&pst_hmac_user->st_user_base_info,
                             pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QosOptionImplemented);
    } else {
        if (IS_STA(pst_mac_vap)) {
            /* 如果关联用户之前就是没有携带wmm ie, 再查找HT CAP能力 */
            puc_ie = mac_find_ie_etc(MAC_EID_HT_CAP, puc_payload, us_msg_len);
            if (puc_ie != OAL_PTR_NULL) {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
                /* 再查找HT CAP能力第2字节BIT 5 short GI for 20M 能力位 */
                if ((puc_ie[1] >= 2) && (puc_ie[2] & BIT5))
#endif
                {
                    mac_user_set_qos_etc(&pst_hmac_user->st_user_base_info, OAL_TRUE);
                }
            }
        }
    }

    /* 如果关联用户到现在仍然不是wmm使能的，什么也不做，直接返回 */
    if (pst_hmac_user->st_user_base_info.st_cap_info.bit_qos == OAL_FALSE) {
        OAM_INFO_LOG0(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                      "{hmac_mgmt_update_assoc_user_qos_table_etc::assoc user is not wmm cap.}");
        return;
    }

    /* 当关联用户从不支持wmm到支持wmm转换时，抛事件到DMAC 写寄存器 */
    /* 申请事件内存 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_ctx_asoc_set_reg_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_mgmt_update_assoc_user_qos_table_etc::event_mem alloc null, size[%d].}",
                       OAL_SIZEOF(dmac_ctx_asoc_set_reg_stru));
        return;
    }

    st_asoc_set_reg_param.uc_user_index = pst_hmac_user->st_user_base_info.us_assoc_id;

    /* 填写事件 */
    pst_event = frw_get_event_stru(pst_event_mem);

    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_ASOC_WRITE_REG,
                       OAL_SIZEOF(dmac_ctx_asoc_set_reg_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_hmac_user->st_user_base_info.uc_chip_id,
                       pst_hmac_user->st_user_base_info.uc_device_id,
                       pst_hmac_user->st_user_base_info.uc_vap_id);

    /* 拷贝参数 */
    if (EOK != memcpy_s(pst_event->auc_event_data, OAL_SIZEOF(dmac_ctx_asoc_set_reg_stru),
                        (oal_void *)&st_asoc_set_reg_param, OAL_SIZEOF(dmac_ctx_asoc_set_reg_stru))) {
        OAM_ERROR_LOG0(0, OAM_SF_ASSOC, "hmac_mgmt_update_assoc_user_qos_table_etc::memcpy fail!");
        FRW_EVENT_FREE(pst_event_mem);
        return;
    }

    /* 分发事件 */
    frw_event_dispatch_event_etc(pst_event_mem);
    FRW_EVENT_FREE(pst_event_mem);
}


#ifdef _PRE_WLAN_FEATURE_TXBF
oal_void hmac_mgmt_update_11ntxbf_cap_etc(oal_uint8 *puc_payload, hmac_user_stru *pst_hmac_user)
{
    mac_11ntxbf_vendor_ie_stru *pst_vendor_ie;
    if (puc_payload == OAL_PTR_NULL) {
        return;
    }

    /* 检测到vendor ie */
    pst_vendor_ie = (mac_11ntxbf_vendor_ie_stru *)puc_payload;

    if (pst_vendor_ie->uc_len < (OAL_SIZEOF(mac_11ntxbf_vendor_ie_stru) - MAC_IE_HDR_LEN)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "hmac_mgmt_update_11ntxbf_cap_etc:invalid vendor ie len[%d]",
                         pst_vendor_ie->uc_len);
        return;
    }

    pst_hmac_user->st_user_base_info.st_cap_info.bit_11ntxbf = pst_vendor_ie->st_11ntxbf.bit_11ntxbf;

    return;
}
#endif /* #ifdef _PRE_WLAN_FEATURE_TXBF */


oal_uint32 hmac_check_bss_cap_info_etc(oal_uint16 us_cap_info, mac_vap_stru *pst_mac_vap)
{
    hmac_vap_stru *pst_hmac_vap;
    oal_uint32 ul_ret;
    wlan_mib_desired_bsstype_enum_uint8 en_bss_type;

    /* 获取CAP INFO里BSS TYPE */
    en_bss_type = mac_get_bss_type_etc(us_cap_info);

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_check_bss_cap_info_etc::pst_hmac_vap null.}");
        return OAL_FAIL;
    }

    /* 比较BSS TYPE是否一致 不一致，如果是STA仍然发起入网，增加兼容性，其它模式则返回不支持 */
    if (en_bss_type != mac_mib_get_DesiredBSSType(pst_mac_vap)) {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_check_bss_cap_info_etc::vap_bss_type[%d] is different from asoc_user_bss_type[%d].}",
                         mac_mib_get_DesiredBSSType(pst_mac_vap), en_bss_type);
    }

    if (OAL_TRUE == mac_mib_get_WPSActive(pst_mac_vap)) {
        return OAL_TRUE;
    }

    /* 比较CAP INFO中privacy位，检查是否加密，加密不一致，返回失败 */
    ul_ret = mac_check_mac_privacy_etc(us_cap_info, (oal_uint8 *)pst_mac_vap);
    if (ul_ret != OAL_TRUE) {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_check_bss_cap_info_etc::mac privacy capabilities is different.}");
    }

    return OAL_TRUE;
}
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_user_protocol_mode_vap_protocol_check(
    mac_user_stru *pst_mac_user, mac_vap_stru *pst_mac_vap)
{
    /* 兼容性问题：思科AP 2.4G（11b）和5G(11a)共存时发送的assoc rsp帧携带的速率分别是11g和11b，导致STA创建用户时通知算法失败，
    Autorate失效，DBAC情况下，DBAC无法启动已工作的VAP状态无法恢复的问题 临时方案，建议针对对端速率异常的情况统一分析优化 */
    return (((pst_mac_user->en_protocol_mode == WLAN_LEGACY_11B_MODE) &&
             (pst_mac_vap->en_protocol == WLAN_LEGACY_11A_MODE)) ||
            ((pst_mac_user->en_protocol_mode == WLAN_LEGACY_11G_MODE) &&
             (pst_mac_vap->en_protocol == WLAN_LEGACY_11B_MODE)));
}


oal_void hmac_set_user_protocol_mode_etc(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user)
{
    mac_user_ht_hdl_stru *pst_mac_ht_hdl;
    mac_vht_hdl_stru *pst_mac_vht_hdl;
    mac_user_stru *pst_mac_user;

    /* 获取HT和VHT结构体指针 */
    pst_mac_user = &pst_hmac_user->st_user_base_info;
    pst_mac_vht_hdl = &(pst_mac_user->st_vht_hdl);
    pst_mac_ht_hdl = &(pst_mac_user->st_ht_hdl);

#ifdef _PRE_WLAN_FEATURE_11AX
    if (MAC_USER_IS_HE_USER(&pst_hmac_user->st_user_base_info)) {
        mac_user_set_protocol_mode_etc(pst_mac_user, WLAN_HE_MODE);
    } else if (pst_mac_vht_hdl->en_vht_capable == OAL_TRUE)
#else
    if (pst_mac_vht_hdl->en_vht_capable == OAL_TRUE)
#endif
    {
        mac_user_set_protocol_mode_etc(pst_mac_user, WLAN_VHT_MODE);
    } else if (pst_mac_ht_hdl->en_ht_capable == OAL_TRUE) {
        mac_user_set_protocol_mode_etc(pst_mac_user, WLAN_HT_MODE);
    } else {
        if (pst_mac_vap->st_channel.en_band == WLAN_BAND_5G) { /* 判断是否是5G */
            mac_user_set_protocol_mode_etc(pst_mac_user, WLAN_LEGACY_11A_MODE);
        } else {
            if (OAL_TRUE == hmac_is_support_11grate_etc(pst_hmac_user->st_op_rates.auc_rs_rates,
                                                        pst_hmac_user->st_op_rates.uc_rs_nrates)) {
                mac_user_set_protocol_mode_etc(pst_mac_user, WLAN_LEGACY_11G_MODE);
                if (OAL_TRUE == hmac_is_support_11brate_etc(pst_hmac_user->st_op_rates.auc_rs_rates,
                                                            pst_hmac_user->st_op_rates.uc_rs_nrates)) {
                    mac_user_set_protocol_mode_etc(pst_mac_user, WLAN_MIXED_ONE_11G_MODE);
                }
            } else if (OAL_TRUE == hmac_is_support_11brate_etc(pst_hmac_user->st_op_rates.auc_rs_rates,
                                                               pst_hmac_user->st_op_rates.uc_rs_nrates)) {
                mac_user_set_protocol_mode_etc(pst_mac_user, WLAN_LEGACY_11B_MODE);
            } else {
                OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                                 "{hmac_set_user_protocol_mode_etc::set user protocol failed.}");
            }
        }
    }

    if (hmac_user_protocol_mode_vap_protocol_check(pst_mac_user, pst_mac_vap)) {
        mac_user_set_protocol_mode_etc(pst_mac_user, pst_mac_vap->en_protocol);
        if (EOK != memcpy_s((oal_void *)&(pst_hmac_user->st_op_rates), OAL_SIZEOF(pst_hmac_user->st_op_rates),
                            (oal_void *)&(pst_mac_vap->st_curr_sup_rates.st_rate), OAL_SIZEOF(mac_rate_stru))) {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_set_user_protocol_mode_etc::memcpy fail!");
            return;
        }
    }

    /*
     * 兼容性问题：ws880配置11a时beacon和probe resp帧中协议vht能力，
     * vap的protocol能力要根据关联响应帧的实际能力刷新成实际11a能力
     */
    if ((pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (pst_mac_vap->en_protocol == WLAN_VHT_MODE) && (pst_mac_user->en_protocol_mode == WLAN_LEGACY_11A_MODE)) {
        mac_vap_init_by_protocol_etc(pst_mac_vap, pst_mac_user->en_protocol_mode);
    }

    mac_vap_tx_data_set_user_htc_cap(pst_mac_vap, &pst_hmac_user->st_user_base_info);
}

oal_void hmac_user_init_rates_etc(hmac_user_stru *pst_hmac_user)
{
    memset_s((oal_uint8 *)(&pst_hmac_user->st_op_rates), OAL_SIZEOF(pst_hmac_user->st_op_rates),
             0, OAL_SIZEOF(pst_hmac_user->st_op_rates));
}


oal_uint8 hmac_add_user_rates_etc(hmac_user_stru *pst_hmac_user, oal_uint8 uc_rates_cnt, oal_uint8 *puc_rates)
{
    if (pst_hmac_user->st_op_rates.uc_rs_nrates + uc_rates_cnt > WLAN_USER_MAX_SUPP_RATES) {
        uc_rates_cnt = WLAN_USER_MAX_SUPP_RATES - pst_hmac_user->st_op_rates.uc_rs_nrates;
    }

    if (EOK != memcpy_s(&(pst_hmac_user->st_op_rates.auc_rs_rates[pst_hmac_user->st_op_rates.uc_rs_nrates]),
                        WLAN_USER_MAX_SUPP_RATES - pst_hmac_user->st_op_rates.uc_rs_nrates, puc_rates, uc_rates_cnt)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_add_user_rates_etc::memcpy fail!");
        return uc_rates_cnt;
    }
    pst_hmac_user->st_op_rates.uc_rs_nrates += uc_rates_cnt;

    return uc_rates_cnt;
}

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)


oal_void hmac_rx_sa_query_req_etc(hmac_vap_stru *pst_hmac_vap,
                                  oal_netbuf_stru *pst_netbuf,
                                  oal_bool_enum_uint8 en_is_protected)
{
    oal_uint8 *puc_sa;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_uint8 *puc_mac_hdr = OAL_PTR_NULL;

    if (OAL_ANY_NULL_PTR2(pst_hmac_vap, pst_netbuf)) {
        OAM_ERROR_LOG2(0, OAM_SF_RX, "{hmac_rx_sa_query_req_etc::null param %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_netbuf);
        return;
    }

    puc_mac_hdr = OAL_NETBUF_HEADER(pst_netbuf);

    mac_rx_get_sa((mac_ieee80211_frame_stru *)puc_mac_hdr, &puc_sa);
    pst_hmac_user = mac_vap_get_hmac_user_by_addr_etc(&pst_hmac_vap->st_vap_base_info, puc_sa);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                       "{hmac_rx_sa_query_req_etc::pst_hmac_user null.}");
        return;
    }

    /* 如果该用户的管理帧加密属性不一致，丢弃该报文 */
    if (en_is_protected != pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active) {
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                         "{hmac_rx_sa_query_req_etc::PMF check failed.}");
        return;
    }

    /* sa Query rsp发送 */
    hmac_send_sa_query_rsp_etc(&pst_hmac_vap->st_vap_base_info, puc_mac_hdr, en_is_protected);

    return;
}

oal_void hmac_rx_sa_query_rsp_etc(hmac_vap_stru *pst_hmac_vap,
                                  oal_netbuf_stru *pst_netbuf,
                                  oal_bool_enum_uint8 en_is_protected)
{
    oal_uint8 *puc_mac_hdr = OAL_PTR_NULL;
    oal_uint8 *puc_sa;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_uint16 *pus_trans_id;
    mac_sa_query_stru *pst_sa_query_info;

    if (OAL_ANY_NULL_PTR2(pst_hmac_vap, pst_netbuf)) {
        OAM_ERROR_LOG2(0, OAM_SF_AMPDU, "{hmac_rx_sa_query_rsp_etc::param null,%x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_netbuf);
        return;
    }

    puc_mac_hdr = OAL_NETBUF_HEADER(pst_netbuf);

    mac_rx_get_sa((mac_ieee80211_frame_stru *)puc_mac_hdr, &puc_sa);
    pst_hmac_user = mac_vap_get_hmac_user_by_addr_etc(&pst_hmac_vap->st_vap_base_info, puc_sa);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMPDU,
                       "{hmac_rx_sa_query_rsp_etc::pst_hmac_user null.}");
        return;
    }

    /* 如果该用户的管理帧加密属性不一致，丢弃该报文 */
    if (en_is_protected != pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMPDU,
                       "{hmac_rx_sa_query_rsp_etc::PMF check failed.}");
        return;
    }

    /* 对比trans_id */
    pus_trans_id = (oal_uint16 *)(puc_mac_hdr + MAC_80211_FRAME_LEN + 2);
    pst_sa_query_info = &pst_hmac_user->st_sa_query_info;

    /* 收到有效的SA query reqponse，保留这条有效的SA */
    if (0 == oal_memcmp(pus_trans_id, &(pst_sa_query_info->us_sa_query_trans_id), 2)) {
        /* pending SA Query requests 计数器清零 & sa query流程开始时间清零 */
        pst_sa_query_info->ul_sa_query_count = 0;
        pst_sa_query_info->ul_sa_query_start_time = 0;

        /* 删除timer */
        if (pst_sa_query_info->st_sa_query_interval_timer.en_is_registerd != OAL_FALSE) {
            FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_sa_query_info->st_sa_query_interval_timer));
        }
    }

    return;
}
#endif


oal_void hmac_send_mgmt_to_host_etc(hmac_vap_stru *pst_hmac_vap,
                                    oal_netbuf_stru *puc_buf,
                                    oal_uint16 us_len,
                                    oal_int l_freq)
{
    mac_device_stru *pst_mac_device;
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    hmac_rx_mgmt_event_stru *pst_mgmt_frame = OAL_PTR_NULL;
    mac_rx_ctl_stru *pst_rx_info = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_P2P
    mac_ieee80211_frame_stru *pst_frame_hdr;
    mac_vap_stru *pst_mac_vap;
    oal_uint8 uc_hal_vap_id;
#endif
    oal_uint8 *pst_mgmt_data = OAL_PTR_NULL;
    oal_int32 l_ret;

    pst_mac_device = mac_res_get_dev_etc(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_send_mgmt_to_host_etc::pst_mac_device null.}");
        return;
    }

    /* 抛关联一个新的sta完成事件到WAL */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(hmac_rx_mgmt_event_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{hmac_send_mgmt_to_host_etc::malloc fail}");
        return;
    }

    pst_rx_info = (mac_rx_ctl_stru *)oal_netbuf_cb(puc_buf);

    pst_mgmt_data = (oal_uint8 *)oal_memalloc(us_len);
    if (pst_mgmt_data == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{hmac_send_mgmt_to_host_etc::malloc fail.}");
        FRW_EVENT_FREE(pst_event_mem);
        return;
    }
    l_ret = memcpy_s(pst_mgmt_data, us_len, (oal_uint8 *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_info), us_len);

    /* 填写事件 */
    pst_event = frw_get_event_stru(pst_event_mem);

    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr), FRW_EVENT_TYPE_HOST_CTX, HMAC_HOST_CTX_EVENT_SUB_TYPE_RX_MGMT,
        OAL_SIZEOF(hmac_rx_mgmt_event_stru), FRW_EVENT_PIPELINE_STAGE_0, pst_hmac_vap->st_vap_base_info.uc_chip_id,
        pst_hmac_vap->st_vap_base_info.uc_device_id, pst_hmac_vap->st_vap_base_info.uc_vap_id);

    /* 填写上报管理帧数据 */
    pst_mgmt_frame = (hmac_rx_mgmt_event_stru *)(pst_event->auc_event_data);
    pst_mgmt_frame->puc_buf = (oal_uint8 *)pst_mgmt_data;
    pst_mgmt_frame->us_len = us_len;
    pst_mgmt_frame->l_freq = l_freq;
    OAL_NETBUF_LEN(puc_buf) = us_len;

    l_ret += memcpy_s(pst_mgmt_frame->ac_name, OAL_IF_NAME_SIZE, pst_hmac_vap->pst_net_device->name, OAL_IF_NAME_SIZE);

#ifdef _PRE_WLAN_FEATURE_P2P
    pst_frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(pst_rx_info);
    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    if (!IS_LEGACY_VAP(pst_mac_vap)) {
        /*
         * 仅针对P2P设备做处理。P2P vap 存在一个vap 对应多个hal_vap 情况，
         * 非P2P vap 不存在一个vap 对应多个hal_vap 情况
         */
        /* 对比接收到的管理帧vap_id 是否和vap 中hal_vap_id 相同 */
        /* 从管理帧cb字段中的hal vap id 的相应信息查找对应的net dev 指针 */
        uc_hal_vap_id = MAC_GET_RX_CB_HAL_VAP_IDX((mac_rx_ctl_stru *)oal_netbuf_cb(puc_buf));
        if (0 == oal_compare_mac_addr(pst_frame_hdr->auc_address1, mac_mib_get_p2p0_dot11StationID(pst_mac_vap))) {
            /* 第二个net dev槽 */
            l_ret += memcpy_s(pst_mgmt_frame->ac_name, OAL_IF_NAME_SIZE,
                              pst_hmac_vap->pst_p2p0_net_device->name, OAL_IF_NAME_SIZE);
        } else if (0 == oal_compare_mac_addr(pst_frame_hdr->auc_address1, mac_mib_get_StationID(pst_mac_vap))) {
            l_ret += memcpy_s(pst_mgmt_frame->ac_name, OAL_IF_NAME_SIZE,
                              pst_hmac_vap->pst_net_device->name, OAL_IF_NAME_SIZE);
        } else if ((uc_hal_vap_id == 14) && (IS_P2P_CL(pst_mac_vap) || IS_P2P_DEV(pst_mac_vap)) &&
                   (pst_frame_hdr->st_frame_control.bit_sub_type == WLAN_PROBE_REQ)) {
            if (!hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
                FRW_EVENT_FREE(pst_event_mem);
                oal_free(pst_mgmt_data);
                return;
            }
            l_ret += memcpy_s(pst_mgmt_frame->ac_name, OAL_IF_NAME_SIZE,
                              pst_hmac_vap->pst_p2p0_net_device->name, OAL_IF_NAME_SIZE);
        } else {
            FRW_EVENT_FREE(pst_event_mem);
            oal_free(pst_mgmt_data);
            return;
        }
    }
#endif
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "hmac_send_mgmt_to_host_etc::memcpy fail!");
        FRW_EVENT_FREE(pst_event_mem);
        oal_free(pst_mgmt_data);
        return;
    }

    /* 分发事件 */
    frw_event_dispatch_event_etc(pst_event_mem);
    FRW_EVENT_FREE(pst_event_mem);
}


oal_uint32 hmac_wpas_mgmt_tx_etc(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_netbuf_stru *pst_netbuf_mgmt_tx = OAL_PTR_NULL;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    mac_mgmt_frame_stru *pst_mgmt_tx = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    oal_uint8 ua_dest_mac_addr[WLAN_MAC_ADDR_LEN];
    oal_uint16 us_user_idx;

    if (OAL_ANY_NULL_PTR2(pst_mac_vap, puc_param)) {
        OAM_ERROR_LOG2(0, OAM_SF_P2P, "{hmac_wpas_mgmt_tx_etc::param null, %d %d.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)pst_mac_vap);
        return OAL_FAIL;
    }

    pst_mgmt_tx = (mac_mgmt_frame_stru *)puc_param;

    pst_mac_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_wpas_mgmt_tx_etc::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                     "{hmac_wpas_mgmt_tx_etc::mgmt frame id=[%d]}",
                     pst_mgmt_tx->mgmt_frame_id);

    /*  申请netbuf 空间 */
    pst_netbuf_mgmt_tx = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,
                                                                 pst_mgmt_tx->us_len,
                                                                 OAL_NETBUF_PRIORITY_MID);

    if (pst_netbuf_mgmt_tx == OAL_PTR_NULL) {
        /* Reserved Memory pool tried for high priority deauth frames */
        pst_netbuf_mgmt_tx = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,
                                                                     WLAN_MEM_NETBUF_SIZE2,
                                                                     OAL_NETBUF_PRIORITY_MID);
        if (pst_netbuf_mgmt_tx == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_wpas_mgmt_tx_etc::pst_mgmt_tx null.}");
            return OAL_FAIL;
        }
    }
    OAL_MEM_NETBUF_TRACE(pst_netbuf_mgmt_tx, OAL_TRUE);

    memset_s(oal_netbuf_cb(pst_netbuf_mgmt_tx), OAL_SIZEOF(mac_tx_ctl_stru), 0, OAL_SIZEOF(mac_tx_ctl_stru));

    /* 填充netbuf */
    if (EOK != memcpy_s((oal_uint8 *)OAL_NETBUF_HEADER(pst_netbuf_mgmt_tx), pst_mgmt_tx->us_len,
                        pst_mgmt_tx->puc_frame, pst_mgmt_tx->us_len)) {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "hmac_wpas_mgmt_tx_etc::memcpy fail!");
        oal_netbuf_free(pst_netbuf_mgmt_tx);
        return OAL_FAIL;
    }
    oal_netbuf_put(pst_netbuf_mgmt_tx, pst_mgmt_tx->us_len);
    
    mac_get_addr1((oal_uint8 *)pst_mgmt_tx->puc_frame, ua_dest_mac_addr, WLAN_MAC_ADDR_LEN);
    us_user_idx = MAC_INVALID_USER_ID;
    /* 管理帧可能发给已关联的用户，也可能发给未关联的用户。已关联的用户可以找到，未关联的用户找不到。不用判断返回值 */
    ul_ret = mac_vap_find_user_by_macaddr_etc(pst_mac_vap, ua_dest_mac_addr, &us_user_idx);

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf_mgmt_tx); /* 获取cb结构体 */
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = pst_mgmt_tx->us_len;             /* dmac发送需要的mpdu长度 */
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = us_user_idx;                  /* 发送完成需要获取user结构体 */
    MAC_GET_CB_IS_NEED_RESP(pst_tx_ctl) = OAL_TRUE;
    MAC_GET_CB_IS_NEEDRETRY(pst_tx_ctl) = OAL_TRUE;
    MAC_GET_CB_MGMT_FRAME_ID(pst_tx_ctl) = pst_mgmt_tx->mgmt_frame_id;
    /* 仅用subtype做识别, 置action会误入dmac_tx_action_process */
    MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_MGMT;
    MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = WLAN_ACTION_P2PGO_FRAME_SUBTYPE;

    /* Buffer this frame in the Memory Queue for transmission */
    ul_ret = hmac_tx_mgmt_send_event_etc(pst_mac_vap, pst_netbuf_mgmt_tx, pst_mgmt_tx->us_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf_mgmt_tx);

        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                         "{hmac_wpas_mgmt_tx_etc::hmac_tx_mgmt_send_event_etc failed[%d].}", ul_ret);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


oal_void hmac_rx_mgmt_send_to_host_etc(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    mac_rx_ctl_stru *pst_rx_info;
    oal_int l_freq;

    pst_rx_info = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
    l_freq = oal_ieee80211_channel_to_frequency(pst_rx_info->uc_channel_number,
        (pst_rx_info->uc_channel_number > 14) ? NL80211_BAND_5GHZ : NL80211_BAND_2GHZ);
#else
    l_freq = oal_ieee80211_channel_to_frequency(pst_rx_info->uc_channel_number,
        (pst_rx_info->uc_channel_number > 14) ? IEEE80211_BAND_5GHZ : IEEE80211_BAND_2GHZ);
#endif
    hmac_send_mgmt_to_host_etc(pst_hmac_vap, pst_netbuf, pst_rx_info->us_frame_len, l_freq);
}

#ifdef _PRE_WLAN_FEATURE_LOCATION

oal_uint8 g_auc_send_csi_buf[HMAC_CSI_SEND_BUF_LEN] = { 0 };
oal_uint8 g_auc_send_ftm_buf[HMAC_FTM_SEND_BUF_LEN] = { 0 };


OAL_STATIC oal_uint32 hmac_netlink_location_send(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    hmac_location_event_stru *pst_location_event;
    struct timeval st_tv;
    oal_time_stru st_local_time;
    oal_uint16 us_action_len;
    oal_uint8 *puc_payload;
    mac_rx_ctl_stru *pst_rx_ctrl;
    oal_uint32 ul_index = 0;
    oal_uint32 *pul_len = OAL_PTR_NULL;
    oal_uint8 *puc_send_csi_buf = g_auc_send_csi_buf;
    oal_uint8 *puc_send_ftm_buf = g_auc_send_ftm_buf;
    oal_int32 l_ret = EOK;

    pst_location_event = (hmac_location_event_stru *)mac_netbuf_get_payload(pst_netbuf);
    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    us_action_len = pst_rx_ctrl->us_frame_len;

    if (us_action_len < MAC_CSI_LOCATION_INFO_LEN) {
        OAM_ERROR_LOG1(0, OAM_SF_FTM, "{hmac_netlink_location_send: unexpected len %d}", us_action_len);
        return OAL_ERR_CODE_MSG_LENGTH_ERR;
    }
    do_gettimeofday(&st_tv);
    OAL_GET_REAL_TIME(&st_local_time);

    switch (pst_location_event->uc_location_type) {
        case MAC_HISI_LOCATION_RSSI_IE:

            break;
        case MAC_HISI_LOCATION_CSI_IE:
            puc_payload = (oal_uint8 *)(pst_location_event->auc_payload);

            /* 第一片 */
            if ((puc_payload[0] == 0) &&
                ((puc_payload[1] == 0) || (puc_payload[1] == 1))) {
                memset_s(puc_send_csi_buf, HMAC_CSI_SEND_BUF_LEN, 0, HMAC_CSI_SEND_BUF_LEN);

                /* Type 4Bytes */
                *(oal_uint32 *)&puc_send_csi_buf[ul_index] = (oal_uint32)2;
                ul_index += 4;

                /* len 4Bytes */
                pul_len = (oal_uint32 *)&puc_send_csi_buf[ul_index];
                ul_index += 4;

                /* mac1 6Bytes */
                l_ret += memcpy_s(&puc_send_csi_buf[ul_index], WLAN_MAC_ADDR_LEN,
                                  pst_location_event->auc_mac_server, WLAN_MAC_ADDR_LEN);
                ul_index += WLAN_MAC_ADDR_LEN;

                /* mac2 6Bytes */
                l_ret += memcpy_s(&puc_send_csi_buf[ul_index], WLAN_MAC_ADDR_LEN,
                                  pst_location_event->auc_mac_client, WLAN_MAC_ADDR_LEN);
                ul_index += WLAN_MAC_ADDR_LEN;

                /* timestamp23Bytes */
                ul_index += snprintf_s(puc_send_csi_buf + ul_index, HMAC_FTM_SEND_BUF_LEN, HMAC_FTM_SEND_BUF_LEN - 1,
                                       "%04d-%02d-%02d %02d:%02d:%02d.%03ld", st_local_time.tm_year + 1900,
                                       st_local_time.tm_mon + 1, st_local_time.tm_mday,
                                       st_local_time.tm_hour, st_local_time.tm_min,
                                       st_local_time.tm_sec, st_tv.tv_usec / 1000);
                if (ul_index < 0) {
                    OAM_ERROR_LOG0(0, OAM_SF_FTM, "hmac_netlink_location_send::snprintf_s error!");
                    return OAL_FAIL;
                }

                /* rssi snr */
                l_ret += memcpy_s(&puc_send_csi_buf[ul_index], MAC_REPORT_RSSIINFO_SNR_LEN,
                                  puc_payload + 3, MAC_REPORT_RSSIINFO_SNR_LEN);

                *pul_len = ul_index + MAC_REPORT_RSSIINFO_SNR_LEN;
                OAM_WARNING_LOG1(0, OAM_SF_FTM, "{hmac_netlink_location_send::send len[%d].}", *pul_len);
            }

            pul_len = (oal_uint32 *)&puc_send_csi_buf[4];

            OAM_WARNING_LOG2(0, OAM_SF_FTM, "{hmac_netlink_location_send::len[%d], copy_len[%d].}",
                             *pul_len, us_action_len - MAC_CSI_LOCATION_INFO_LEN);
            OAM_WARNING_LOG3(0, OAM_SF_FTM, "{hmac_netlink_location_send::frag [%d], [%d] ,[%d].}",
                             puc_payload[0], puc_payload[1], puc_payload[2]);

            if (*pul_len + us_action_len - MAC_CSI_LOCATION_INFO_LEN > HMAC_CSI_SEND_BUF_LEN) {
                OAM_ERROR_LOG0(0, OAM_SF_FTM, "{hmac_netlink_location_send::puc_send_buf not enough.}");
                return OAL_FAIL;
            }

            l_ret += memcpy_s(&puc_send_csi_buf[*pul_len], us_action_len - MAC_CSI_LOCATION_INFO_LEN,
                              puc_payload + 3 + MAC_REPORT_RSSIINFO_SNR_LEN,
                              us_action_len - MAC_CSI_LOCATION_INFO_LEN);
            *pul_len += us_action_len - MAC_CSI_LOCATION_INFO_LEN;

            /* 最后一片 */
            if (pst_location_event->auc_payload[2] == 1) {
                drv_netlink_location_send((oal_void *)puc_send_csi_buf, *pul_len);
                OAM_WARNING_LOG1(0, OAM_SF_FTM, "{hmac_netlink_location_send::send len[%d].}", *pul_len);
            }
            break;
        case MAC_HISI_LOCATION_FTM_IE:
            memset_s(puc_send_ftm_buf, HMAC_FTM_SEND_BUF_LEN, 0, HMAC_FTM_SEND_BUF_LEN);
            *(oal_uint32 *)&puc_send_ftm_buf[ul_index] = (oal_uint32)3;
            ul_index += 4;
            *(oal_uint32 *)&puc_send_ftm_buf[ul_index] = 99;
            ul_index += 4;
            l_ret += memcpy_s(&puc_send_ftm_buf[ul_index], WLAN_MAC_ADDR_LEN,
                              pst_location_event->auc_mac_server, WLAN_MAC_ADDR_LEN);
            ul_index += WLAN_MAC_ADDR_LEN;
            l_ret += memcpy_s(&puc_send_ftm_buf[ul_index], WLAN_MAC_ADDR_LEN,
                              pst_location_event->auc_mac_client, WLAN_MAC_ADDR_LEN);
            ul_index += WLAN_MAC_ADDR_LEN;
            ul_index += snprintf_s(puc_send_ftm_buf + ul_index, HMAC_FTM_SEND_BUF_LEN,
                                   HMAC_FTM_SEND_BUF_LEN - 1,
                                   "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
                                   st_local_time.tm_year + 1900, st_local_time.tm_mon + 1,
                                   st_local_time.tm_mday,
                                   st_local_time.tm_hour, st_local_time.tm_min,
                                   st_local_time.tm_sec, st_tv.tv_usec / 1000);

            puc_payload = (oal_uint8 *)(pst_location_event->auc_payload);

            l_ret += memcpy_s(&puc_send_ftm_buf[ul_index], 56, puc_payload, 56);
            drv_netlink_location_send((oal_void *)puc_send_ftm_buf, 99);
            break;
        default:
            return OAL_SUCC;
    }
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_FTM, "hmac_netlink_location_send::memcpy fail!");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}
#endif
#if defined(_PRE_WLAN_FEATURE_LOCATION) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)

OAL_STATIC oal_uint32 hmac_proc_location_action(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    hmac_location_event_stru *pst_location_event;
    struct timeval st_tv;
    oal_time_stru st_local_time;
    oal_uint8 auc_filename[128];
    oal_uint16 us_action_len;
    oal_uint16 us_delta_len;
    // oal_uint16                     us_info_idx;
    oal_int32 l_str_len;
    oal_file *f_file;
    oal_mm_segment_t old_fs;
    oal_uint8 *puc_payload;
    mac_rx_ctl_stru *pst_rx_ctrl;
    oal_int8 *pc_psd_payload;

    /* Vendor Public Action Header| EID |Length |OUI | type | mac_s | mac_c | rssi */
    /* 获取本地时间精确到us 2017-11-03-23-50-12-xxxxxxxx */
    pst_location_event = (hmac_location_event_stru *)mac_netbuf_get_payload(pst_netbuf);
    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    us_action_len = pst_rx_ctrl->us_frame_len;

    if (us_action_len < MAC_CSI_LOCATION_INFO_LEN) {
        OAM_ERROR_LOG1(0, OAM_SF_FTM, "{hmac_proc_location_action: unexpected len %d}", us_action_len);
        return OAL_ERR_CODE_MSG_LENGTH_ERR;
    }
    do_gettimeofday(&st_tv);
    OAL_GET_REAL_TIME(&st_local_time);

    /* 获取文件路径\data\log\location\wlan0\ */
    l_str_len = snprintf_s(auc_filename, OAL_SIZEOF(auc_filename), OAL_SIZEOF(auc_filename) - 1,
                           "/data/log/location/%s/", pst_hmac_vap->auc_name);
    if (l_str_len < 0) {
        OAM_ERROR_LOG0(0, OAM_SF_FTM, "hmac_proc_location_action::snprintf_s error!");
        return OAL_FAIL;
    }

    OAM_WARNING_LOG1(0, OAM_SF_FTM, "{hmac_proc_location_action::location type[%d]}",
                     pst_location_event->uc_location_type);

    switch (pst_location_event->uc_location_type) {
        case MAC_HISI_LOCATION_RSSI_IE:
            /* 获取文件名 MAC_ADDR_S_MAC_ADDR_C_RSSI */
            l_str_len += snprintf_s(auc_filename + l_str_len, OAL_SIZEOF(auc_filename) - l_str_len,
                                    OAL_SIZEOF(auc_filename) - l_str_len - 1, "RSSI.TXT");

            break;
        case MAC_HISI_LOCATION_CSI_IE:
            /* 获取文件名 MAC_ADDR_S_MAC_ADDR_C_CSI */
            l_str_len += snprintf_s(auc_filename + l_str_len, OAL_SIZEOF(auc_filename) - l_str_len,
                                    OAL_SIZEOF(auc_filename) - l_str_len - 1, "CSI.TXT");

            break;
        case MAC_HISI_LOCATION_FTM_IE:
            /* 获取文件名 MAC_ADDR_S_MAC_ADDR_C_FTM */
            l_str_len += snprintf_s(auc_filename + l_str_len, OAL_SIZEOF(auc_filename) - l_str_len,
                                    OAL_SIZEOF(auc_filename) - l_str_len - 1, "FTM.TXT");

            break;
        case MAC_HISI_PSD_SAVE_FILE_IE:
            /* 获取文件名 MAC_ADDR_S_MAC_ADDR_C_FTM */
            l_str_len += snprintf_s(auc_filename + l_str_len, OAL_SIZEOF(auc_filename) - l_str_len,
                                    OAL_SIZEOF(auc_filename) - l_str_len - 1, "PSD.TXT");

            break;
        default:
            return OAL_SUCC;
    }

    f_file = oal_kernel_file_open_etc(auc_filename, OAL_O_RDWR | OAL_O_CREAT | OAL_O_APPEND);
    if (IS_ERR_OR_NULL(f_file)) {
        OAM_ERROR_LOG1(0, OAM_SF_FTM, "{hmac_proc_location_action: ****************save file failed %d }", l_str_len);

        return OAL_ERR_CODE_OPEN_FILE_FAIL;
    }
    old_fs = oal_get_fs();

    /* 对于CSI来说，payload[0]表示当前的分片序列号；payload[1]表示内存块分段序列号，0表示不分段，1表示第一个分段
       payload[2]表示当前分片是不是最后一片  */
    if (pst_location_event->uc_location_type == MAC_HISI_LOCATION_CSI_IE) {
        if ((pst_location_event->auc_payload[0] == 0) &&
            ((pst_location_event->auc_payload[1] == 0) || (pst_location_event->auc_payload[1] == 1))) {
            oal_kernel_file_print_etc(f_file, "%04d-%02d-%02d-", st_local_time.tm_year + 1900,
                                      st_local_time.tm_mon + 1, st_local_time.tm_mday);
            oal_kernel_file_print_etc(f_file, "%02d-%02d-%02d-%08d : ", st_local_time.tm_hour,
                                      st_local_time.tm_min, st_local_time.tm_sec, st_tv.tv_usec);
            /* rssi snr */
            puc_payload = (oal_uint8 *)(pst_location_event->auc_payload);
            l_str_len = 0;
            while (l_str_len < MAC_REPORT_RSSIINFO_SNR_LEN) {
                oal_kernel_file_print_etc(f_file, "%02X ", *(puc_payload++));
                l_str_len++;
            }
        }
    } else {
        oal_kernel_file_print_etc(f_file, "%04d-%02d-%02d-", st_local_time.tm_year + 1900,
                                  st_local_time.tm_mon + 1, st_local_time.tm_mday);
        oal_kernel_file_print_etc(f_file, "%02d-%02d-%02d-%08d : ", st_local_time.tm_hour,
                                  st_local_time.tm_min, st_local_time.tm_sec, st_tv.tv_usec);
    }

    l_str_len = 0;
    puc_payload = (oal_uint8 *)(pst_location_event->auc_payload);

    if (pst_location_event->uc_location_type == MAC_HISI_LOCATION_CSI_IE) {
        us_delta_len = us_action_len - MAC_CSI_LOCATION_INFO_LEN;
        puc_payload += MAC_REPORT_RSSIINFO_SNR_LEN + 3;
    } else {
        us_delta_len = us_action_len - MAC_FTM_LOCATION_INFO_LEN + MAC_REPORT_RSSIINFO_LEN;
    }

    if (pst_location_event->uc_location_type != MAC_HISI_PSD_SAVE_FILE_IE) {
        while (l_str_len < us_delta_len) {
            oal_kernel_file_print_etc(f_file, "%02X ", *(puc_payload++));
            l_str_len++;
        }
    } else {
        pc_psd_payload = (oal_int8 *)(pst_location_event->auc_payload);
        while (l_str_len < us_delta_len) {
            oal_kernel_file_print_etc(f_file, "%d ", *(pc_psd_payload++));
            l_str_len++;
        }
    }

#if 1
    /* 未考虑80M场景 TBD */
    if (OAL_VALUE_EQ_ANY3(pst_location_event->uc_location_type,
                          MAC_HISI_LOCATION_RSSI_IE, MAC_HISI_LOCATION_FTM_IE, MAC_HISI_PSD_SAVE_FILE_IE) ||
        ((pst_location_event->uc_location_type == MAC_HISI_LOCATION_CSI_IE) &&
         ((pst_location_event->auc_payload[2] == 1) &&
          OAL_VALUE_EQ_ANY2(pst_location_event->auc_payload[1], 0, 2)))) {
        l_str_len += oal_kernel_file_print_etc(f_file, "\n");
    }
#endif
    oal_kernel_file_close(f_file);
    oal_set_fs(old_fs);

    return OAL_SUCC;
}

oal_uint32 hmac_huawei_action_process(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf, oal_uint8 uc_type)
{
    switch (uc_type) {
        case MAC_HISI_LOCATION_RSSI_IE:
        case MAC_HISI_LOCATION_CSI_IE:
        case MAC_HISI_LOCATION_FTM_IE:
        case MAC_HISI_PSD_SAVE_FILE_IE:
            /* 将其他设备上报的私有信息去掉ie头抛事件到hmac进行保存 */
            /* type | mac_s | mac_c   | csi or ftm or rssi   */
            /* csi 信息注意长度 */
            hmac_proc_location_action(pst_hmac_vap, pst_netbuf);
#ifdef _PRE_WLAN_FEATURE_LOCATION
            hmac_netlink_location_send(pst_hmac_vap, pst_netbuf);
#endif
            break;
        default:
            break;
    }

    return OAL_SUCC;
}
#endif

oal_uint32 hmac_mgmt_tx_event_status_etc(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_crx_mgmt_tx_status_stru *pst_mgmt_tx_status_param = OAL_PTR_NULL;
    dmac_crx_mgmt_tx_status_stru *pst_mgmt_tx_status_param_2wal = OAL_PTR_NULL;
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    if (puc_param == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{hmac_mgmt_tx_event_status_etc::puc_param is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mgmt_tx_status_param = (dmac_crx_mgmt_tx_status_stru *)puc_param;

    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
        "{hmac_mgmt_tx_event_status_etc::dmac tx mgmt status report.userindx[%d], tx mgmt status[%d], frame_id[%d]}",
        pst_mgmt_tx_status_param->us_user_idx,
        pst_mgmt_tx_status_param->uc_dscr_status,
        pst_mgmt_tx_status_param->mgmt_frame_id);

    /* 抛管理帧发送完成事件到WAL */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_crx_mgmt_tx_status_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{hmac_mgmt_tx_event_status_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    pst_event = frw_get_event_stru(pst_event_mem);

    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_MGMT_TX_STATUS,
                       OAL_SIZEOF(dmac_crx_mgmt_tx_status_stru),
                       FRW_EVENT_PIPELINE_STAGE_0,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    pst_mgmt_tx_status_param_2wal = (dmac_crx_mgmt_tx_status_stru *)(pst_event->auc_event_data);
    pst_mgmt_tx_status_param_2wal->uc_dscr_status = pst_mgmt_tx_status_param->uc_dscr_status;
    pst_mgmt_tx_status_param_2wal->mgmt_frame_id = pst_mgmt_tx_status_param->mgmt_frame_id;

    /* 分发事件 */
    ul_ret = frw_event_dispatch_event_etc(pst_event_mem);
    FRW_EVENT_FREE(pst_event_mem);

    return ul_ret;
}


oal_void hmac_vap_set_user_avail_rates_etc(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user)
{
    mac_user_stru *pst_mac_user = OAL_PTR_NULL;
    mac_curr_rateset_stru *pst_mac_vap_rate = OAL_PTR_NULL;
    hmac_rate_stru *pst_hmac_user_rate;
    mac_rate_stru st_avail_op_rates;
    oal_uint8 uc_mac_vap_rate_num;
    oal_uint8 uc_hmac_user_rate_num;
    oal_uint8 uc_vap_rate_idx;
    oal_uint8 uc_user_rate_idx;
    oal_uint8 uc_user_avail_rate_idx = 0;

    /* 获取VAP和USER速率的结构体指针 */
    pst_mac_user = &(pst_hmac_user->st_user_base_info);
    pst_mac_vap_rate = &(pst_mac_vap->st_curr_sup_rates);
    pst_hmac_user_rate = &(pst_hmac_user->st_op_rates);
    memset_s ((oal_uint8 *)(&st_avail_op_rates), OAL_SIZEOF(mac_rate_stru), 0, OAL_SIZEOF(mac_rate_stru));

    uc_mac_vap_rate_num = pst_mac_vap_rate->st_rate.uc_rs_nrates;
    uc_hmac_user_rate_num = pst_hmac_user_rate->uc_rs_nrates;

    for (uc_vap_rate_idx = 0; uc_vap_rate_idx < uc_mac_vap_rate_num; uc_vap_rate_idx++) {
        for (uc_user_rate_idx = 0; uc_user_rate_idx < uc_hmac_user_rate_num; uc_user_rate_idx++) {
            if ((pst_mac_vap_rate->st_rate.ast_rs_rates[uc_vap_rate_idx].uc_mac_rate > 0) &&
                (IS_EQUAL_RATES(pst_mac_vap_rate->st_rate.ast_rs_rates[uc_vap_rate_idx].uc_mac_rate,
                                pst_hmac_user_rate->auc_rs_rates[uc_user_rate_idx]))) {
                st_avail_op_rates.auc_rs_rates[uc_user_avail_rate_idx] =
                    (pst_hmac_user_rate->auc_rs_rates[uc_user_rate_idx] & 0x7F);
                uc_user_avail_rate_idx++;
                st_avail_op_rates.uc_rs_nrates++;
            }
        }
    }

    mac_user_set_avail_op_rates_etc(pst_mac_user, st_avail_op_rates.uc_rs_nrates, st_avail_op_rates.auc_rs_rates);
    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                     "{hmac_vap_set_user_avail_rates_etc::uc_avail_op_rates_num=[%d].}",
                     st_avail_op_rates.uc_rs_nrates);
}

oal_uint32 hmac_proc_ht_cap_ie_etc(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, oal_uint8 *puc_ht_cap_ie)
{
    oal_uint8 uc_mcs_bmp_index;
    mac_user_ht_hdl_stru *pst_ht_hdl = OAL_PTR_NULL;
    oal_uint16 us_tmp_info_elem;
    oal_uint16 us_tmp_txbf_low;
    oal_uint16 us_ht_cap_info;
    oal_uint32 ul_tmp_txbf_elem;
    oal_uint16 us_offset = 0;

    if (OAL_ANY_NULL_PTR3(pst_mac_vap, pst_mac_user, puc_ht_cap_ie)) {
        OAM_WARNING_LOG3(0, OAM_SF_ROAM,
                         "{hmac_proc_ht_cap_ie_etc::PARAM NULL! vap=[0x%X],user=[0x%X],cap_ie=[0x%X].}",
                         (uintptr_t)pst_mac_vap, (uintptr_t)pst_mac_user, (uintptr_t)puc_ht_cap_ie);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 至少支持11n才进行后续的处理 */
    if (OAL_FALSE == mac_mib_get_HighThroughputOptionImplemented(pst_mac_vap)) {
        return OAL_SUCC;
    }

    mac_user_set_ht_capable_etc(pst_mac_user, OAL_TRUE);

    pst_ht_hdl = &pst_mac_user->st_ht_hdl;

    /* 带有 HT Capability Element 的 AP，标示它具有HT capable. */
    pst_ht_hdl->en_ht_capable = OAL_TRUE;

    us_offset += MAC_IE_HDR_LEN;

    /********************************************/
    /*     解析 HT Capabilities Info Field      */
    /********************************************/
    us_ht_cap_info = OAL_MAKE_WORD16(puc_ht_cap_ie[us_offset], puc_ht_cap_ie[us_offset + 1]);

    /* 检查STA所支持的LDPC编码能力 B0，0:不支持，1:支持 */
    pst_ht_hdl->bit_ldpc_coding_cap = (us_ht_cap_info & BIT0);

    /* 提取AP所支持的带宽能力  */
    pst_ht_hdl->bit_supported_channel_width = ((us_ht_cap_info & BIT1) >> 1);

    /* 检查空间复用节能模式 B2~B3 */
    mac_ie_proc_sm_power_save_field_etc (pst_mac_user, (oal_uint8)((us_ht_cap_info & (BIT3 | BIT2)) >> 2));

    /* 提取AP支持Greenfield情况 */
    pst_ht_hdl->bit_ht_green_field = ((us_ht_cap_info & BIT4) >> 4);

    /* 提取AP支持20MHz Short-GI情况 */
    pst_ht_hdl->bit_short_gi_20mhz = ((us_ht_cap_info & BIT5) >> 5);

    /* 提取AP支持40MHz Short-GI情况 */
    pst_ht_hdl->bit_short_gi_40mhz = ((us_ht_cap_info & BIT6) >> 6);

    /* 提取AP支持STBC PPDU情况 */
    pst_ht_hdl->bit_rx_stbc = (oal_uint8)((us_ht_cap_info & (BIT9 | BIT8)) >> 8);

    /* 提取AP支持最大A-MSDU长度情况 */
    // *pus_amsdu_maxsize = (0 == (us_ht_cap_info & BIT11)) ?
    // WLAN_MIB_MAX_AMSDU_LENGTH_SHORT : WLAN_MIB_MAX_AMSDU_LENGTH_LONG;
    /* 提取AP 40M上DSSS/CCK的支持情况 */
    pst_ht_hdl->bit_dsss_cck_mode_40mhz = ((us_ht_cap_info & BIT12) >> 12);

    /* 提取AP L-SIG TXOP 保护的支持情况 */
    pst_ht_hdl->bit_lsig_txop_protection = ((us_ht_cap_info & BIT15) >> 15);

    us_offset += MAC_HT_CAPINFO_LEN;

    /********************************************/
    /*     解析 A-MPDU Parameters Field         */
    /********************************************/
    /* 提取 Maximum Rx A-MPDU factor (B1 - B0) */
    pst_ht_hdl->uc_max_rx_ampdu_factor = (puc_ht_cap_ie[us_offset] & 0x03);

    /* 提取 Minmum Rx A-MPDU factor (B3 - B2) */
    pst_ht_hdl->uc_min_mpdu_start_spacing = (puc_ht_cap_ie[us_offset] >> 2) & 0x07;

    us_offset += MAC_HT_AMPDU_PARAMS_LEN;

    /********************************************/
    /*     解析 Supported MCS Set Field         */
    /********************************************/
    for (uc_mcs_bmp_index = 0; uc_mcs_bmp_index < WLAN_HT_MCS_BITMASK_LEN; uc_mcs_bmp_index++) {
        pst_ht_hdl->uc_rx_mcs_bitmask[uc_mcs_bmp_index] =
            (mac_mib_get_SupportedMCSTxValue(pst_mac_vap, uc_mcs_bmp_index)) &
           (*(oal_uint8 *)(puc_ht_cap_ie + us_offset + uc_mcs_bmp_index));
    }

    pst_ht_hdl->uc_rx_mcs_bitmask[WLAN_HT_MCS_BITMASK_LEN - 1] &= 0x1F;

    us_offset += MAC_HT_SUP_MCS_SET_LEN;

    /********************************************/
    /* 解析 HT Extended Capabilities Info Field */
    /********************************************/
    us_ht_cap_info = OAL_MAKE_WORD16(puc_ht_cap_ie[us_offset], puc_ht_cap_ie[us_offset + 1]);

    /* 提取 HTC support Information */
    pst_ht_hdl->uc_htc_support = ((us_ht_cap_info & BIT10) >> 10);

    us_offset += MAC_HT_EXT_CAP_LEN;

    /********************************************/
    /*  解析 Tx Beamforming Field               */
    /********************************************/
    us_tmp_info_elem = OAL_MAKE_WORD16(puc_ht_cap_ie[us_offset], puc_ht_cap_ie[us_offset + 1]);
    us_tmp_txbf_low = OAL_MAKE_WORD16(puc_ht_cap_ie[us_offset + 2], puc_ht_cap_ie[us_offset + 3]);
    ul_tmp_txbf_elem = OAL_MAKE_WORD32(us_tmp_info_elem, us_tmp_txbf_low);
    pst_ht_hdl->bit_imbf_receive_cap = (ul_tmp_txbf_elem & BIT0);
    pst_ht_hdl->bit_receive_staggered_sounding_cap = ((ul_tmp_txbf_elem & BIT1) >> 1);
    pst_ht_hdl->bit_transmit_staggered_sounding_cap = ((ul_tmp_txbf_elem & BIT2) >> 2);
    pst_ht_hdl->bit_receive_ndp_cap = ((ul_tmp_txbf_elem & BIT3) >> 3);
    pst_ht_hdl->bit_transmit_ndp_cap = ((ul_tmp_txbf_elem & BIT4) >> 4);
    pst_ht_hdl->bit_imbf_cap = ((ul_tmp_txbf_elem & BIT5) >> 5);
    pst_ht_hdl->bit_calibration = ((ul_tmp_txbf_elem & 0x000000C0) >> 6);
    pst_ht_hdl->bit_exp_csi_txbf_cap = ((ul_tmp_txbf_elem & BIT8) >> 8);
    pst_ht_hdl->bit_exp_noncomp_txbf_cap = ((ul_tmp_txbf_elem & BIT9) >> 9);
    pst_ht_hdl->bit_exp_comp_txbf_cap = ((ul_tmp_txbf_elem & BIT10) >> 10);
    pst_ht_hdl->bit_exp_csi_feedback = ((ul_tmp_txbf_elem & 0x00001800) >> 11);
    pst_ht_hdl->bit_exp_noncomp_feedback = ((ul_tmp_txbf_elem & 0x00006000) >> 13);
    pst_ht_hdl->bit_exp_comp_feedback = ((ul_tmp_txbf_elem & 0x0001C000) >> 15);
    pst_ht_hdl->bit_min_grouping = ((ul_tmp_txbf_elem & 0x00060000) >> 17);
    pst_ht_hdl->bit_csi_bfer_ant_number = ((ul_tmp_txbf_elem & 0x001C0000) >> 19);
    pst_ht_hdl->bit_noncomp_bfer_ant_number = ((ul_tmp_txbf_elem & 0x00600000) >> 21);
    pst_ht_hdl->bit_comp_bfer_ant_number = ((ul_tmp_txbf_elem & 0x01C00000) >> 23);
    pst_ht_hdl->bit_csi_bfee_max_rows = ((ul_tmp_txbf_elem & 0x06000000) >> 25);
    pst_ht_hdl->bit_channel_est_cap = ((ul_tmp_txbf_elem & 0x18000000) >> 27);

    return OAL_SUCC;
}


oal_uint32 hmac_proc_vht_cap_ie_etc(mac_vap_stru *pst_mac_vap,
                                    hmac_user_stru *pst_hmac_user,
                                    oal_uint8 *puc_vht_cap_ie)
{
    mac_vht_hdl_stru *pst_mac_vht_hdl = OAL_PTR_NULL;
    mac_vht_hdl_stru st_mac_vht_hdl;
    mac_user_stru *pst_mac_user = OAL_PTR_NULL;
    oal_uint16 us_vht_cap_filed_low;
    oal_uint16 us_vht_cap_filed_high;
    oal_uint32 ul_vht_cap_field;
    oal_uint16 us_rx_mcs_map;
    oal_uint16 us_tx_mcs_map;
    oal_uint16 us_rx_highest_supp_logGi_data;
    oal_uint16 us_tx_highest_supp_logGi_data;
    oal_uint16 us_msg_idx = 0;
    oal_int32 l_ret;

    /* 解析vht cap IE */
    if (OAL_ANY_NULL_PTR3(pst_mac_vap, pst_hmac_user, puc_vht_cap_ie)) {
        OAM_ERROR_LOG3(0, OAM_SF_ANY,
                       "{hmac_proc_vht_cap_ie_etc::param null,mac_vap[0x%x], hmac_user[0x%x], vht_cap_ie[0x%x].}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)pst_hmac_user, (uintptr_t)puc_vht_cap_ie);

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (puc_vht_cap_ie[1] < MAC_VHT_CAP_IE_LEN) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_proc_vht_cap_ie_etc::invalid vht cap ie len[%d].}", puc_vht_cap_ie[1]);
        return OAL_FAIL;
    }

    pst_mac_user = &pst_hmac_user->st_user_base_info;

    /* 支持11ac，才进行后续的处理 */
    if (OAL_FALSE == mac_mib_get_VHTOptionImplemented(pst_mac_vap)) {
        return OAL_SUCC;
    }

    pst_mac_vht_hdl = &st_mac_vht_hdl;
    mac_user_get_vht_hdl_etc(pst_mac_user, pst_mac_vht_hdl);

    /* 进入此函数代表user支持11ac */
    pst_mac_vht_hdl->en_vht_capable = OAL_TRUE;

#ifdef _PRE_WLAN_FEATURE_11AC2G
    /* 定制化实现如果不支持11ac2g模式，则关掉vht cap */
    if ((pst_mac_vap->st_cap_flag.bit_11ac2g == OAL_FALSE)
        && (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band)) {
        pst_mac_vht_hdl->en_vht_capable = OAL_FALSE;
    }
#endif

    us_msg_idx += MAC_IE_HDR_LEN;

    /* 解析VHT capablities info field */
    us_vht_cap_filed_low = OAL_MAKE_WORD16(puc_vht_cap_ie[us_msg_idx], puc_vht_cap_ie[us_msg_idx + 1]);
    us_vht_cap_filed_high = OAL_MAKE_WORD16(puc_vht_cap_ie[us_msg_idx + 2], puc_vht_cap_ie[us_msg_idx + 3]);
    ul_vht_cap_field = OAL_MAKE_WORD32(us_vht_cap_filed_low, us_vht_cap_filed_high);

    /* 解析max_mpdu_length 参见11ac协议 Table 8-183u */
    pst_mac_vht_hdl->bit_max_mpdu_length = (ul_vht_cap_field & (BIT1 | BIT0));
    pst_mac_vht_hdl->us_max_mpdu_length =
            mac_ie_get_max_mpdu_len_by_vht_cap(pst_mac_vht_hdl->bit_max_mpdu_length);

    hmac_user_set_amsdu_level_by_max_mpdu_len(pst_hmac_user,
        pst_mac_vht_hdl->us_max_mpdu_length);

    /* 解析supported_channel_width */
    pst_mac_vht_hdl->bit_supported_channel_width = ((ul_vht_cap_field & (BIT3 | BIT2)) >> 2);

    /* 解析rx_ldpc */
    pst_mac_vht_hdl->bit_rx_ldpc = ((ul_vht_cap_field & BIT4) >> 4);

    /* 解析short_gi_80mhz和short_gi_160mhz支持情况 */
    pst_mac_vht_hdl->bit_short_gi_80mhz = ((ul_vht_cap_field & BIT5) >> 5);
    pst_mac_vht_hdl->bit_short_gi_80mhz &= mac_mib_get_VHTShortGIOptionIn80Implemented(pst_mac_vap);

    pst_mac_vht_hdl->bit_short_gi_160mhz = ((ul_vht_cap_field & BIT6) >> 6);
    pst_mac_vht_hdl->bit_short_gi_160mhz &= mac_mib_get_VHTShortGIOptionIn160and80p80Implemented(pst_mac_vap);

    /* 解析tx_stbc 和rx_stbc */
    pst_mac_vht_hdl->bit_tx_stbc = ((ul_vht_cap_field & BIT7) >> 7);
    pst_mac_vht_hdl->bit_rx_stbc = ((ul_vht_cap_field & (BIT10 | BIT9 | BIT8)) >> 8);

    /* 解析su_beamformer_cap和su_beamformee_cap */
    pst_mac_vht_hdl->bit_su_beamformer_cap = ((ul_vht_cap_field & BIT11) >> 11);
    pst_mac_vht_hdl->bit_su_beamformee_cap = ((ul_vht_cap_field & BIT12) >> 12);

    /* 解析num_bf_ant_supported */
    pst_mac_vht_hdl->bit_num_bf_ant_supported = ((ul_vht_cap_field & (BIT15 | BIT14 | BIT13)) >> 13);

    /* 以对端天线数初始化可用最大空间流 */
    pst_mac_user->en_avail_bf_num_spatial_stream = pst_mac_vht_hdl->bit_num_bf_ant_supported;

    /* 解析num_sounding_dim */
    pst_mac_vht_hdl->bit_num_sounding_dim = ((ul_vht_cap_field & (BIT18 | BIT17 | BIT16)) >> 16);

    /* 解析mu_beamformer_cap和mu_beamformee_cap */
    pst_mac_vht_hdl->bit_mu_beamformer_cap = ((ul_vht_cap_field & BIT19) >> 19);
    pst_mac_vht_hdl->bit_mu_beamformee_cap = ((ul_vht_cap_field & BIT20) >> 20);

    /* 解析vht_txop_ps */
    pst_mac_vht_hdl->bit_vht_txop_ps = ((ul_vht_cap_field & BIT21) >> 21);
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    if (pst_mac_vht_hdl->bit_vht_txop_ps == OAL_TRUE && OAL_TRUE == mac_mib_get_txopps(pst_mac_vap)) {
        mac_vap_set_txopps(pst_mac_vap, OAL_TRUE);
    }
#endif
    /* 解析htc_vht_capable */
    pst_mac_vht_hdl->bit_htc_vht_capable = ((ul_vht_cap_field & BIT22) >> 22);

    /* 解析max_ampdu_len_exp */
    pst_mac_vht_hdl->bit_max_ampdu_len_exp = ((ul_vht_cap_field & (BIT25 | BIT24 | BIT23)) >> 23);

    /* 解析vht_link_adaptation */
    pst_mac_vht_hdl->bit_vht_link_adaptation = ((ul_vht_cap_field & (BIT27 | BIT26)) >> 26);

    /* 解析rx_ant_pattern */
    pst_mac_vht_hdl->bit_rx_ant_pattern = ((ul_vht_cap_field & BIT28) >> 28);

    /* 解析tx_ant_pattern */
    pst_mac_vht_hdl->bit_tx_ant_pattern = ((ul_vht_cap_field & BIT29) >> 29);

    /* 解析extend_nss_bw_supp */
    pst_mac_vht_hdl->bit_extend_nss_bw_supp = ((ul_vht_cap_field & (BIT31 | BIT30)) >> 30);

    us_msg_idx += MAC_VHT_CAP_INFO_FIELD_LEN;

    /* 解析VHT Supported MCS Set field */
    us_rx_mcs_map = OAL_MAKE_WORD16(puc_vht_cap_ie[us_msg_idx], puc_vht_cap_ie[us_msg_idx + 1]);

    l_ret = memcpy_s(&(pst_mac_vht_hdl->st_rx_max_mcs_map), OAL_SIZEOF(mac_rx_max_mcs_map_stru),
                     &us_rx_mcs_map, OAL_SIZEOF(mac_rx_max_mcs_map_stru));

    us_msg_idx += MAC_VHT_CAP_RX_MCS_MAP_FIELD_LEN;

    /* 解析rx_highest_supp_logGi_data */
    us_rx_highest_supp_logGi_data = OAL_MAKE_WORD16(puc_vht_cap_ie[us_msg_idx], puc_vht_cap_ie[us_msg_idx + 1]);
    pst_mac_vht_hdl->bit_rx_highest_rate = us_rx_highest_supp_logGi_data & (0x1FFF);

    us_msg_idx += MAC_VHT_CAP_RX_HIGHEST_DATA_FIELD_LEN;

    /* 解析tx_mcs_map */
    us_tx_mcs_map = OAL_MAKE_WORD16(puc_vht_cap_ie[us_msg_idx], puc_vht_cap_ie[us_msg_idx + 1]);
    l_ret += memcpy_s(&(pst_mac_vht_hdl->st_tx_max_mcs_map), OAL_SIZEOF(mac_tx_max_mcs_map_stru),
                      &us_tx_mcs_map, OAL_SIZEOF(mac_tx_max_mcs_map_stru));
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_proc_vht_cap_ie_etc::memcpy fail!");
        return OAL_FAIL;
    }

    us_msg_idx += MAC_VHT_CAP_TX_MCS_MAP_FIELD_LEN;

    /* 解析tx_highest_supp_logGi_data */
    us_tx_highest_supp_logGi_data = OAL_MAKE_WORD16(puc_vht_cap_ie[us_msg_idx], puc_vht_cap_ie[us_msg_idx + 1]);
    pst_mac_vht_hdl->bit_tx_highest_rate = us_tx_highest_supp_logGi_data & (0x1FFF);

    mac_user_set_vht_hdl_etc(pst_mac_user, pst_mac_vht_hdl);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11AX

oal_uint32 hmac_proc_he_cap_ie(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user, oal_uint8 *puc_he_cap_ie)
{
    mac_he_hdl_stru *pst_mac_he_hdl;
    mac_he_hdl_stru st_mac_he_hdl;
    mac_user_stru *pst_mac_user;
    oal_uint32 ul_ret;

    /* 解析he cap IE */
    if (OAL_ANY_NULL_PTR2(pst_mac_vap, pst_hmac_user)) {
        OAM_ERROR_LOG2(0, OAM_SF_11AX, "{hmac_proc_he_cap_ie::param null,mac_vap[0x%x], hmac_user[0x%x].}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)pst_hmac_user);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (puc_he_cap_ie == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_11AX, "{hmac_proc_he_cap_ie::param null, he_cap_ie[0x%x].}",
                         (uintptr_t)puc_he_cap_ie);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (puc_he_cap_ie[1] < MAC_HE_CAP_MIN_LEN) {
        OAM_WARNING_LOG1(0, OAM_SF_11AX, "{hmac_proc_he_cap_ie::invalid he cap ie len[%d].}", puc_he_cap_ie[1]);
        return OAL_FAIL;
    }

    pst_mac_user = &pst_hmac_user->st_user_base_info;

    /* 支持11ax，才进行后续的处理 */
    if (OAL_FALSE == mac_mib_get_HEOptionImplemented(pst_mac_vap)) {
        return OAL_SUCC;
    }

    pst_mac_he_hdl = &st_mac_he_hdl;
    mac_user_get_he_hdl(pst_mac_user, pst_mac_he_hdl);

    /* 解析 HE MAC Capabilities Info */
    ul_ret = mac_ie_parse_he_cap(puc_he_cap_ie, &pst_mac_he_hdl->st_he_cap_ie);

    if (ul_ret != OAL_SUCC) {
        return OAL_FAIL;
    }

    pst_mac_he_hdl->en_he_capable = OAL_TRUE;

    mac_user_set_he_hdl(pst_mac_user, pst_mac_he_hdl);

    /* 适配user支持的Beamforming空间流个数 */
    pst_mac_user->en_avail_bf_num_spatial_stream =
        (pst_mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS) ?
        pst_mac_he_hdl->st_he_cap_ie.st_he_phy_cap.bit_beamformee_sts_below_80mhz :
        pst_mac_he_hdl->st_he_cap_ie.st_he_phy_cap.bit_beamformee_sts_over_80mhz;

    return OAL_SUCC;
}


oal_uint32 hmac_proc_he_bss_color_change_announcement_ie(mac_vap_stru *pst_mac_vap,
                                                         hmac_user_stru *pst_hmac_user,
                                                         oal_uint8 *puc_bss_color_ie)
{
    mac_frame_bss_color_change_annoncement_ie_stru st_bss_color_info;
    mac_he_hdl_stru *pst_mac_he_hdl;

    if (OAL_FALSE == mac_mib_get_HEOptionImplemented(pst_mac_vap)) {
        return OAL_SUCC;
    }

    memset_s(&st_bss_color_info, OAL_SIZEOF(mac_frame_bss_color_change_annoncement_ie_stru),
             0, OAL_SIZEOF(mac_frame_bss_color_change_annoncement_ie_stru));
    if (OAL_SUCC != mac_ie_parse_he_bss_color_change_announcement_ie(puc_bss_color_ie, &st_bss_color_info)) {
        return OAL_SUCC;
    }

    pst_mac_he_hdl = MAC_USER_HE_HDL_STRU(&pst_hmac_user->st_user_base_info);
    pst_mac_he_hdl->bit_change_announce_bss_color = st_bss_color_info.bit_new_bss_color;
    pst_mac_he_hdl->bit_change_announce_bss_color_exist = 1;

    OAM_WARNING_LOG2(0, 0, "hmac_proc_he_bss_color_change_announcement_ie,bss_color=%d, bss_color_exist=[%d]",
        pst_mac_he_hdl->bit_change_announce_bss_color, pst_mac_he_hdl->bit_change_announce_bss_color_exist);

    return OAL_SUCC;
}
#endif


oal_void hmac_add_and_clear_repeat_op_rates(oal_uint8 *puc_ie_rates,
                                            oal_uint8 uc_ie_num_rates,
                                            hmac_rate_stru *pst_op_rates)
{
    oal_uint8 uc_ie_rates_idx;
    oal_uint8 uc_user_rates_idx;

    for (uc_ie_rates_idx = 0; uc_ie_rates_idx < uc_ie_num_rates; uc_ie_rates_idx++) {
        /* 判断该速率是否已经记录在op中 */
        for (uc_user_rates_idx = 0; uc_user_rates_idx < pst_op_rates->uc_rs_nrates; uc_user_rates_idx++) {
            if (IS_EQUAL_RATES(puc_ie_rates[uc_ie_rates_idx], pst_op_rates->auc_rs_rates[uc_user_rates_idx])) {
                break;
            }
        }

        /* 相等时，说明ie中的速率与op中的速率都不相同，可以加入op的速率集中 */
        if (uc_user_rates_idx == pst_op_rates->uc_rs_nrates) {
            /* 当长度超出限制时告警，不加入op rates中 */
            if (pst_op_rates->uc_rs_nrates == WLAN_USER_MAX_SUPP_RATES) {
                OAM_WARNING_LOG0(0, OAM_SF_ANY,
                    "{hmac_add_and_clear_repeat_op_rates::user option rates more then WLAN_USER_MAX_SUPP_RATES.}");
                break;
            }
            pst_op_rates->auc_rs_rates[pst_op_rates->uc_rs_nrates++] = puc_ie_rates[uc_ie_rates_idx];
        }
    }
}


oal_uint32 hmac_ie_proc_assoc_user_legacy_rate(oal_uint8 *puc_payload,
                                               oal_uint32 us_rx_len,
                                               hmac_user_stru *pst_hmac_user)
{
    oal_uint8 *puc_ie = OAL_PTR_NULL;
    mac_user_stru *pst_mac_user;
    mac_vap_stru *pst_mac_vap;
    oal_uint8 uc_num_rates = 0;
    oal_uint8 uc_num_ex_rates = 0;

    pst_mac_user = &(pst_hmac_user->st_user_base_info);

    pst_mac_vap = mac_res_get_mac_vap(pst_mac_user->uc_vap_id);
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_ie_proc_assoc_user_legacy_rate::pst_mac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* user的可选速率集此时应该为空 */
    if (pst_hmac_user->st_op_rates.uc_rs_nrates != 0) {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_ie_proc_assoc_user_legacy_rate::the number of user option rates is not 0.}");
    }

    /* 1.处理基础速率集 */
    puc_ie = mac_find_ie_etc(MAC_EID_RATES, puc_payload, (oal_int32)us_rx_len);

    if (puc_ie != OAL_PTR_NULL) {
        uc_num_rates = puc_ie[1];
        /*
         * 判断supported rates长度，当长度为0时告警，当长度超出限制时，在hmac_add_and_clear_repeat_op_rates里告警，
         * extended supported rates同样处理
         */
        if (uc_num_rates < MAC_MIN_RATES_LEN) {
            OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                "{hmac_ie_proc_assoc_user_legacy_rate::supported rates length less then \
                MAC_MIN_RATES_LEN vap mode[%d] num_rates[%d]}",
                pst_mac_vap->en_vap_mode, uc_num_rates);
        } else {
            hmac_add_and_clear_repeat_op_rates(puc_ie + MAC_IE_HDR_LEN, uc_num_rates, &(pst_hmac_user->st_op_rates));
        }
    } else {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{hmac_ie_proc_assoc_user_legacy_rate::unsupport basic rates vap mode[%d]}", pst_mac_vap->en_vap_mode);
    }

    /* 2.处理扩展速率集 */
    puc_ie = mac_find_ie_etc(MAC_EID_XRATES, puc_payload, (oal_int32)us_rx_len);

    if (puc_ie != OAL_PTR_NULL) {
        uc_num_ex_rates = puc_ie[1];
        /* AP模式下，协议大于11a且小等11G时，不允许对端携带拓展速率集，STA模式下不做限制 */
        if ((pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
            ((pst_mac_vap->en_protocol <= WLAN_LEGACY_11G_MODE) && (pst_mac_vap->en_protocol > WLAN_LEGACY_11A_MODE))) {
            OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                "{hmac_ie_proc_assoc_user_legacy_rate::invaild xrates rate vap protocol[%d] num_ex_rates[%d]}",
                pst_mac_vap->en_protocol, uc_num_ex_rates);
        } else {
            /* 长度告警处理同上 */
            if (uc_num_ex_rates < MAC_MIN_XRATE_LEN) {
                OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                    "{hmac_ie_proc_assoc_user_legacy_rate::extended supported rates length less then \
                    MAC_MIN_RATES_LEN vap mode[%d] num_rates[%d]}",
                    pst_mac_vap->en_vap_mode, uc_num_ex_rates);
            } else {
                hmac_add_and_clear_repeat_op_rates(puc_ie + MAC_IE_HDR_LEN,
                                                   uc_num_ex_rates,
                                                   &(pst_hmac_user->st_op_rates));
            }
        }
    } else {
        if (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                "{hmac_ie_proc_assoc_user_legacy_rate::unsupport xrates vap mode[%d]}", pst_mac_vap->en_vap_mode);
        }
    }

    if (pst_hmac_user->st_op_rates.uc_rs_nrates == 0) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_ie_proc_assoc_user_legacy_rate::rate is 0.}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_HS20

oal_uint32 hmac_interworking_check(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_param)
{
    oal_uint8 *puc_extend_cap_ie;
    mac_bss_dscr_stru *pst_bss_dscr;

    if (OAL_UNLIKELY(OAL_ANY_NULL_PTR2(pst_hmac_vap, puc_param))) {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_interworking_check:: check failed, null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_bss_dscr = (mac_bss_dscr_stru *)puc_param;
    if (pst_bss_dscr->ul_mgmt_len < (MAC_80211_FRAME_LEN + MAC_SSID_OFFSET)) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                         "{hmac_interworking_check:: mgmt_len(%d) < (80211_FRAME_LEN+SSID_OFFSET).}",
                         pst_bss_dscr->ul_mgmt_len);
        return OAL_FAIL;
    }

    /* 查找interworking ie */
    /*lint -e416*/
    puc_extend_cap_ie = mac_find_ie_etc(MAC_EID_EXT_CAPS,
                                        pst_bss_dscr->auc_mgmt_buff + MAC_80211_FRAME_LEN + MAC_SSID_OFFSET,
                                        (oal_int32)(pst_bss_dscr->ul_mgmt_len - MAC_80211_FRAME_LEN - MAC_SSID_OFFSET));
    /*lint +e416*/
    if (puc_extend_cap_ie == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                         "{hmac_interworking_check:: puc_extend_cap_ie is NULL, the ul_mgmt_len is %d.}",
                         pst_bss_dscr->ul_mgmt_len);
        return OAL_FAIL;
    }

    /*  未检测到interworking能力位，返回fail */
    if (puc_extend_cap_ie[1] < 4 || !(puc_extend_cap_ie[5] & 0x80)) {
        pst_hmac_vap->st_vap_base_info.st_cap_flag.bit_is_interworking = OAL_FALSE;
        return OAL_FAIL;
    }

    pst_hmac_vap->st_vap_base_info.st_cap_flag.bit_is_interworking = OAL_TRUE;
    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_HS20

/*lint -e19*/
oal_module_symbol(hmac_config_send_deauth_etc);
oal_module_symbol(hmac_wpas_mgmt_tx_etc);
/*lint +e19*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

