

/* 1 头文件包含 */
#ifdef _PRE_WLAN_FEATURE_WMMAC
#include "hmac_wmmac.h"
#include "hmac_mgmt_bss_comm.h"
#include "securec.h"
#include "mac_mib.h"
#include "mac_data.h"
#include "hmac_tx_data.h"
#include "wlan_chip_i.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_WMMAC_C

/* 2 全局变量定义 */
/* 3 函数实现 */

OAL_STATIC uint16_t hmac_mgmt_encap_addts_req(hmac_vap_stru *pst_vap,
                                              uint8_t *puc_data,
                                              mac_wmm_tspec_stru *pst_addts_args)
{
    uint16_t us_index;

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
    oal_set_mac_addr(puc_data + 4, pst_vap->st_vap_base_info.auc_bssid); /* Frame Control|Duration| 2+2=4 */

    /* SA的值为dot11MACAddress的值 */
    oal_set_mac_addr(puc_data + 10,  /* Frame Control|Duration|DA| 10 */
                     pst_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);

    oal_set_mac_addr(puc_data + 16, pst_vap->st_vap_base_info.auc_bssid); /* Frame Control|Duration|DA|SA 16 */

    /***************************************************************************/
    /*                Set the contents of the frame body                       */
    /* ----------------------------------------------------------------------- */
    /* |MAC HEADER| Category| Action| Dialog token| Status| Elements| FCS|     */
    /* ----------------------------------------------------------------------- */
    /* |24/30     |1        |1      |1            |1      |         |4   |     */
    /* ----------------------------------------------------------------------- */
    /*                                                                         */
    /***************************************************************************/
    /* 将索引指向frame body起始位置 */
    us_index = MAC_80211_FRAME_LEN;

    /* 设置Category */
    puc_data[us_index++] = MAC_ACTION_CATEGORY_WMMAC_QOS;

    /* 设置Action */
    puc_data[us_index++] = MAC_WMMAC_ACTION_ADDTS_REQ;

    /* 设置Dialog Token */
    puc_data[us_index++] = pst_vap->uc_ts_dialog_token;

    /* Status 固定值为0 */
    puc_data[us_index++] = 0;

    /* 设置WMM AC参数 */
    us_index += mac_set_wmmac_ie_sta((void *)pst_vap, puc_data + us_index, pst_addts_args);

    /* 返回的帧长度中不包括FCS */
    return us_index;
}


OAL_STATIC uint16_t hmac_mgmt_encap_delts(hmac_vap_stru *pst_vap,
                                          uint8_t *puc_data,
                                          uint8_t *puc_addr,
                                          uint8_t uc_tsid)
{
    uint16_t us_index;
    uint16_t us_ie_len;
    mac_wmm_tspec_stru st_addts_args;

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
    if (IS_STA(&(pst_vap->st_vap_base_info))) {
        /* DA is address of STA requesting association */
        oal_set_mac_addr(puc_data + 4, pst_vap->st_vap_base_info.auc_bssid); /* Frame Control|Duration| 2+2=4 */

        /* SA的值为dot11MACAddress的值 */
        oal_set_mac_addr(puc_data + 10, /* |Frame Control|Duration|DA| 2+2+6=10 */
            pst_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);
        oal_set_mac_addr(puc_data + 16, pst_vap->st_vap_base_info.auc_bssid); /* |Frame Con|Durat|DA|SA| 2+2+6+6=16 */
    } else if (IS_AP(&(pst_vap->st_vap_base_info))) {
        /* DA is puc_addr, user's address */
        oal_set_mac_addr(puc_data + 4, puc_addr); /* Frame Control|Duration| 2+2=4 */

        /* SA的值为dot11MACAddress的值 */
        oal_set_mac_addr(puc_data + 10, /* |Frame Control|Duration|DA| 2+2+6=10 */
            pst_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);
        oal_set_mac_addr(puc_data + 16, /* |Frame Con|Durat|DA|SA| 2+2+6+6=16 */
            pst_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);
    }

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /* ----------------------------------------------------------------------- */
    /* |MAC HEADER| Category| Action| Dialog token| Status| Elements| FCS|   */
    /* ----------------------------------------------------------------------- */
    /* |24/30     |1        |1      |1            |1      |         |4   |   */
    /* ----------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    /* 将索引指向frame body起始位置 */
    us_index = MAC_80211_FRAME_LEN;

    /* 设置Category */
    puc_data[us_index++] = MAC_ACTION_CATEGORY_WMMAC_QOS;

    /* 设置Action */
    puc_data[us_index++] = MAC_WMMAC_ACTION_DELTS;

    /* 设置Dialog Token */
    puc_data[us_index++] = 0;

    puc_data[us_index++] = 0; /* Status 固定值为0 */

    memset_s(&st_addts_args, sizeof(st_addts_args), 0, sizeof(st_addts_args));
    st_addts_args.ts_info.bit_tsid = uc_tsid;

    /* 设置WMM AC参数，除了TSID，delts其他值均为0 */
    us_ie_len = mac_set_wmmac_ie_sta((void *)pst_vap, puc_data + us_index, &st_addts_args);

    us_index += us_ie_len;

    /* 返回的帧长度中不包括FCS */
    return us_index;
}

static uint8_t hmac_delts_args_user_prio_update(hmac_user_stru *pst_hmac_user, mac_wmm_tspec_stru *pst_delts_args)
{
    uint8_t uc_ac_num;
    for (uc_ac_num = 0; uc_ac_num < WLAN_WME_AC_BUTT; uc_ac_num++) {
        if (pst_delts_args->ts_info.bit_tsid == pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num].uc_tsid) {
            if (pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num].en_ts_status == MAC_TS_NONE) {
                continue;
            }
            /* 如果找到了对应的TSID，则以对应的UP发送,更新对应的TS信息 */
            pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num].en_ts_status = MAC_TS_INIT;
            pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num].uc_tsid = 0xFF;
            pst_delts_args->ts_info.bit_user_prio = pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num].uc_up;
            break;
        }
    }
    return uc_ac_num;
}

static void hmac_set_delts_args_user_prio(hmac_vap_stru *pst_hmac_vap,
                                          hmac_user_stru *pst_hmac_user,
                                          mac_wmm_tspec_stru *pst_delts_args)
{
    uint8_t uc_ac_num;
    /* 遍历所有AC，找到对应TSID的AC和UP */
    if (IS_STA(&pst_hmac_vap->st_vap_base_info)) {
        uc_ac_num = hmac_delts_args_user_prio_update(pst_hmac_user, pst_delts_args);
        /* 如果没找到对应的TSID，delts默认以up7发送 */
        if (uc_ac_num == WLAN_WME_AC_BUTT) {
            pst_delts_args->ts_info.bit_user_prio = 0x7;
        }
    }
}

static uint32_t hmac_set_delts_args_dmac_info(hmac_user_stru *pst_hmac_user, mac_wmm_tspec_stru *pst_delts_args,
                                              oal_netbuf_stru *pst_delts, uint16_t us_frame_len)
{
    mac_tx_ctl_stru *pst_tx_ctl = NULL;
    dmac_ctx_action_event_stru st_wlan_ctx_action;

    memset_s((uint8_t *)&st_wlan_ctx_action, sizeof(st_wlan_ctx_action), 0, sizeof(st_wlan_ctx_action));
    /* 赋值要传入Dmac的信息 */
    st_wlan_ctx_action.us_frame_len = us_frame_len;
    st_wlan_ctx_action.uc_hdr_len = MAC_80211_FRAME_LEN;
    st_wlan_ctx_action.en_action_category = MAC_ACTION_CATEGORY_WMMAC_QOS;
    st_wlan_ctx_action.uc_action = MAC_WMMAC_ACTION_DELTS;
    st_wlan_ctx_action.us_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;
    st_wlan_ctx_action.uc_tidno = pst_delts_args->ts_info.bit_user_prio;
    st_wlan_ctx_action.uc_tsid = pst_delts_args->ts_info.bit_tsid;

    if (EOK != memcpy_s((uint8_t *)(oal_netbuf_data(pst_delts) + us_frame_len),
        WLAN_MEM_NETBUF_SIZE2 - us_frame_len,
        (uint8_t *)&st_wlan_ctx_action,
        sizeof(dmac_ctx_action_event_stru))) {
        oam_error_log0(0, OAM_SF_ANY, "hwifi_config_init_fcc_ce_txpwr_nvram::memcpy fail!");
        return OAL_FAIL;
    }
    oal_netbuf_put(pst_delts, (us_frame_len + sizeof(dmac_ctx_action_event_stru)));

    /* 初始化CB */
    memset_s(oal_netbuf_cb(pst_delts), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_delts);
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_frame_len + sizeof(dmac_ctx_action_event_stru);
    MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_ACTION;
    MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = WLAN_ACTION_BA_WMMAC_DELTS;

    return OAL_SUCC;
}

static uint32_t hmac_post_delts_event(mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_delts,
                                      uint16_t us_frame_len)
{
    uint32_t ret;
    frw_event_stru *pst_hmac_to_dmac_ctx_event = NULL;
    dmac_tx_event_stru *pst_tx_event = NULL;
    frw_event_mem_stru *pst_event_mem = NULL; /* 申请事件返回的内存指针 */

    pst_event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (pst_event_mem == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC, "{hmac_mgmt_tx_delts::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_to_dmac_ctx_event = (frw_event_stru *)pst_event_mem->puc_data;
    frw_event_hdr_init(&(pst_hmac_to_dmac_ctx_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT,
                       sizeof(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    pst_tx_event = (dmac_tx_event_stru *)(pst_hmac_to_dmac_ctx_event->auc_event_data);
    pst_tx_event->pst_netbuf = pst_delts;
    pst_tx_event->us_frame_len = us_frame_len + sizeof(dmac_ctx_action_event_stru);

    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC,
            "{hmac_mgmt_tx_delts::frw_event_dispatch_event failed[%d].}", ret);
        frw_event_free_m(pst_event_mem);
        return ret;
    }

    frw_event_free_m(pst_event_mem);
    return OAL_SUCC;
}


uint32_t hmac_mgmt_tx_delts(hmac_vap_stru *pst_hmac_vap,
                            hmac_user_stru *pst_hmac_user,
                            mac_wmm_tspec_stru *pst_delts_args)
{
    mac_vap_stru *pst_mac_vap;
    oal_netbuf_stru *pst_delts = NULL;
    uint16_t us_frame_len;
    uint32_t ret;

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    /* 确定vap处于工作状态 */
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC,
            "{hmac_mgmt_tx_delts:: vap has been down/del, vap_state[%d].}",
            pst_mac_vap->en_vap_state);
        return OAL_FAIL;
    }

    hmac_set_delts_args_user_prio(pst_hmac_vap, pst_hmac_user, pst_delts_args);

    /* 申请DELTS管理帧内存 */
    pst_delts = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_delts == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC, "{hmac_mgmt_tx_delts::pst_addts_req null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_mem_netbuf_trace(pst_delts, OAL_TRUE);
    oal_netbuf_prev(pst_delts) = NULL;
    oal_netbuf_next(pst_delts) = NULL;

    /* 调用封装管理帧接口 */
    us_frame_len = hmac_mgmt_encap_delts(pst_hmac_vap,
                                         oal_netbuf_data(pst_delts),
                                         pst_hmac_user->st_user_base_info.auc_user_mac_addr,
                                         (uint8_t)pst_delts_args->ts_info.bit_tsid);

    ret = hmac_set_delts_args_dmac_info(pst_hmac_user, pst_delts_args, pst_delts, us_frame_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_delts);
        return ret;
    }
    ret = hmac_post_delts_event(pst_mac_vap, pst_delts, us_frame_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_delts);
        return ret;
    }
    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_mgmt_tx_addts_req_timeout(void *p_arg)
{
    hmac_vap_stru *pst_vap = NULL; /* vap指针 */
    hmac_user_stru *pst_hmac_user = NULL;
    mac_ts_stru *pst_ts_args = NULL;
    mac_wmm_tspec_stru st_addts_args;

    if (p_arg == NULL) {
        oam_error_log0(0, OAM_SF_WMMAC, "{hmac_mgmt_tx_addts_req_timeout::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ts_args = (mac_ts_stru *)p_arg;

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(pst_ts_args->us_mac_user_idx);
    if (pst_hmac_user == NULL) {
        oam_error_log0(0, OAM_SF_WMMAC, "{hmac_mgmt_tx_addts_req_timeout::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_ts_args->uc_vap_id);
    if (oal_unlikely(pst_vap == NULL)) {
        oam_error_log0(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_WMMAC,
            "{hmac_mgmt_tx_addts_req_timeout::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_ts_args->uc_second_flag == OAL_TRUE) {
        return OAL_SUCC;
    }

    /* 生成DELTS帧,只需要改变TSID */
    st_addts_args.ts_info.bit_tsid = pst_ts_args->uc_tsid;
    st_addts_args.ts_info.bit_user_prio = pst_ts_args->uc_up;

    oam_warning_log2(0, OAM_SF_WMMAC, "{hmac_mgmt_tx_addts_req_timeout::AC=[%d],TS_TID=[%d] send delts.}",
                     WLAN_WME_TID_TO_AC(pst_ts_args->uc_up), pst_ts_args->uc_tsid);

    return hmac_mgmt_tx_delts(pst_vap, pst_hmac_user, &st_addts_args);
}

static uint32_t hmac_set_addts_dmac_info(hmac_user_stru *pst_hmac_user, mac_wmm_tspec_stru *pst_addts_args,
                                         hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_addts_req,
                                         uint16_t us_frame_len)
{
    mac_tx_ctl_stru *pst_tx_ctl = NULL;
    dmac_ctx_action_event_stru st_wlan_ctx_action;
    uint8_t uc_ac_num;
    memset_s((uint8_t *)&st_wlan_ctx_action, sizeof(st_wlan_ctx_action), 0, sizeof(st_wlan_ctx_action));
    uc_ac_num = WLAN_WME_TID_TO_AC(pst_addts_args->ts_info.bit_user_prio);

    /* 赋值要传入Dmac的信息 */
    st_wlan_ctx_action.us_frame_len = us_frame_len;
    st_wlan_ctx_action.uc_hdr_len = MAC_80211_FRAME_LEN;
    st_wlan_ctx_action.en_action_category = MAC_ACTION_CATEGORY_WMMAC_QOS;
    st_wlan_ctx_action.uc_action = MAC_WMMAC_ACTION_ADDTS_REQ;
    st_wlan_ctx_action.us_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;
    st_wlan_ctx_action.uc_tidno = pst_addts_args->ts_info.bit_user_prio;
    st_wlan_ctx_action.uc_ts_dialog_token = pst_hmac_vap->uc_ts_dialog_token;
    st_wlan_ctx_action.uc_initiator = pst_addts_args->ts_info.bit_direction;
    st_wlan_ctx_action.uc_tsid = pst_addts_args->ts_info.bit_tsid;
    st_wlan_ctx_action.st_addts_info.uc_ac = uc_ac_num;
    st_wlan_ctx_action.st_addts_info.bit_psb = pst_addts_args->ts_info.bit_apsd;
    st_wlan_ctx_action.st_addts_info.bit_direction = pst_addts_args->ts_info.bit_direction;
    if (EOK != memcpy_s((uint8_t *)(oal_netbuf_data(pst_addts_req) + us_frame_len),
        WLAN_MEM_NETBUF_SIZE2 - us_frame_len,
        (uint8_t *)&st_wlan_ctx_action,
        sizeof(dmac_ctx_action_event_stru))) {
        oam_error_log0(0, OAM_SF_ANY, "hwifi_config_init_fcc_ce_txpwr_nvram::memcpy fail!");
        return OAL_FAIL;
    }
    oal_netbuf_put(pst_addts_req, (us_frame_len + sizeof(dmac_ctx_action_event_stru)));

    /* 初始化CB */
    memset_s(oal_netbuf_cb(pst_addts_req), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_addts_req);
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_frame_len + sizeof(dmac_ctx_action_event_stru);
    MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_ACTION;
    MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = WLAN_ACTION_BA_WMMAC_ADDTS_REQ;
    return OAL_SUCC;
}

static uint32_t hmac_post_addts_req_event(mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_addts_req,
                                          uint16_t us_frame_len)
{
    frw_event_mem_stru *pst_event_mem = NULL; /* 申请事件返回的内存指针 */
    frw_event_stru *pst_hmac_to_dmac_ctx_event = NULL;
    dmac_tx_event_stru *pst_tx_event = NULL;
    uint32_t ret;
    pst_event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (pst_event_mem == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC, "{hmac_mgmt_tx_addts_req::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_to_dmac_ctx_event = (frw_event_stru *)pst_event_mem->puc_data;
    frw_event_hdr_init(&(pst_hmac_to_dmac_ctx_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT,
                       sizeof(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    pst_tx_event = (dmac_tx_event_stru *)(pst_hmac_to_dmac_ctx_event->auc_event_data);
    pst_tx_event->pst_netbuf = pst_addts_req;
    pst_tx_event->us_frame_len = us_frame_len + sizeof(dmac_ctx_action_event_stru);

    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC,
            "{hmac_mgmt_tx_addts_req::frw_event_dispatch_event failed[%d].}", ret);
        frw_event_free_m(pst_event_mem);
        return ret;
    }

    frw_event_free_m(pst_event_mem);
    return OAL_SUCC;
}

static void hmac_addts_req_set_timer(hmac_user_stru *pst_hmac_user, mac_device_stru *pst_device,
                                     hmac_vap_stru *pst_hmac_vap, mac_wmm_tspec_stru *pst_addts_args,
                                     uint8_t uc_ac_num)
{
    mac_ts_stru *pst_ts_use = NULL;
    mac_ts_stru *pst_ts_tmp = NULL;
    mac_ts_stru *pst_ts_timer = NULL;
        /* 更新对应的TS信息 */
    pst_ts_use = &pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num];
    pst_ts_tmp = &pst_hmac_user->st_ts_tmp_info[uc_ac_num];

    if (pst_ts_use->en_ts_status == MAC_TS_SUCCESS) {
        pst_ts_tmp->en_ts_status = MAC_TS_INPROGRESS;
        pst_ts_tmp->uc_ts_dialog_token = pst_hmac_vap->uc_ts_dialog_token;
        pst_ts_tmp->uc_tsid = pst_addts_args->ts_info.bit_tsid;
        pst_ts_timer = pst_ts_tmp;
        pst_ts_timer->uc_second_flag = OAL_TRUE;
    } else {
        pst_ts_use->en_ts_status = MAC_TS_INPROGRESS;
        pst_ts_use->uc_ts_dialog_token = pst_hmac_vap->uc_ts_dialog_token;
        pst_ts_use->uc_tsid = pst_addts_args->ts_info.bit_tsid;
        pst_ts_timer = pst_ts_use;
    }

    /* 启动ADDTS REQ超时计时器 */
    frw_timer_create_timer_m(&pst_ts_timer->st_addts_timer,
                             hmac_mgmt_tx_addts_req_timeout,
                             WLAN_ADDTS_TIMEOUT,
                             pst_ts_timer,
                             OAL_FALSE,
                             OAM_MODULE_ID_HMAC,
                             pst_device->core_id);
}

uint32_t hmac_mgmt_tx_addts_req(hmac_vap_stru *pst_hmac_vap,
                                hmac_user_stru *pst_hmac_user,
                                mac_wmm_tspec_stru *pst_addts_args)
{
    mac_vap_stru *pst_mac_vap;
    mac_device_stru *pst_device = NULL;
    oal_netbuf_stru *pst_addts_req = NULL;
    uint16_t us_frame_len;
    uint8_t uc_ac_num;
    uint32_t ret;
    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    /* 确定vap处于工作状态 */
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC,
                         "{hmac_mgmt_tx_addts_req::vap has been down/del, vap_state[%d].}",
                         pst_mac_vap->en_vap_state);
        return OAL_FAIL;
    }

    /* 获取device结构 */
    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_device == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC, "{hmac_mgmt_tx_addba_req::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请ADDTS_REQ管理帧内存 */
    pst_addts_req = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_addts_req == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC, "{hmac_mgmt_tx_addts_req::pst_addts_req null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_mem_netbuf_trace(pst_addts_req, OAL_TRUE);

    oal_netbuf_prev(pst_addts_req) = NULL;
    oal_netbuf_next(pst_addts_req) = NULL;

    pst_hmac_vap->uc_ts_dialog_token++;
    uc_ac_num = WLAN_WME_TID_TO_AC(pst_addts_args->ts_info.bit_user_prio);

    /* 调用封装管理帧接口 */
    us_frame_len = hmac_mgmt_encap_addts_req(pst_hmac_vap, oal_netbuf_data(pst_addts_req), pst_addts_args);
    ret = hmac_set_addts_dmac_info(pst_hmac_user, pst_addts_args, pst_hmac_vap, pst_addts_req, us_frame_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_addts_req);
        return ret;
    }
    ret = hmac_post_addts_req_event(pst_mac_vap, pst_addts_req, us_frame_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_addts_req);
        return ret;
    }
    hmac_addts_req_set_timer(pst_hmac_user, pst_device, pst_hmac_vap, pst_addts_args, uc_ac_num);
    return OAL_SUCC;
}


OAL_STATIC uint16_t hmac_mgmt_encap_addts_rsp(hmac_vap_stru *pst_vap,
                                              uint8_t *puc_data,
                                              uint8_t *puc_addts_req,
                                              uint16_t us_frame_len)
{
    uint16_t us_index;
    uint8_t *puc_sa = NULL;

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
    mac_rx_get_sa((mac_ieee80211_frame_stru *)puc_addts_req, &puc_sa);

    /* 拷贝整个ADDTS REQ帧 */
    if (EOK != memcpy_s(puc_data, WLAN_MEM_NETBUF_SIZE2, puc_addts_req, us_frame_len)) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_mgmt_encap_addts_rsp::memcpy fail!");
        return 0;
    }

    /* Frame Control Field 中只需要设置Type/Subtype值，其他设置为0 */
    mac_hdr_set_frame_control(puc_data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* DA is ADDTS Request frame's SA */
    oal_set_mac_addr(puc_data + 4, puc_sa);  /* |Frame Control|Duration| 2+2=4 */

    /* SA的值为dot11MACAddress的值 */
    oal_set_mac_addr(puc_data + 10, /* Frame Control|Duration|DA 2+2+6=10 */
                     pst_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);
    oal_set_mac_addr(puc_data + 16, /* Frame Control|Duration|DA|SA 2+2+6+6=16 */
                     pst_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);

    /***************************************************************************/
    /*                Set the contents of the frame body                       */
    /* ----------------------------------------------------------------------- */
    /* |MAC HEADER| Category| Action| Dialog token| Status| Elements| FCS|     */
    /* ----------------------------------------------------------------------- */
    /* |24/30     |1        |1      |1            |1      |         |4   |     */
    /* ----------------------------------------------------------------------- */
    /*                                                                         */
    /***************************************************************************/
    /* 将索引指向frame body起始位置 */
    us_index = MAC_80211_FRAME_LEN;

    /* 设置Category */
    puc_data[us_index++] = MAC_ACTION_CATEGORY_WMMAC_QOS;

    /* 设置Action */
    puc_data[us_index++] = MAC_WMMAC_ACTION_ADDTS_RSP;

    /* 设置Dialog Token */
    us_index++;

    puc_data[us_index++] = 0; /* Status 为Succ */

    /* 返回的帧长度中不包括FCS */
    return us_frame_len;
}

static uint32_t hmac_update_user_st_temp_info(hmac_user_stru *pst_hmac_user, mac_vap_stru *pst_mac_vap,
                                              uint8_t uc_tsid, uint8_t uc_dialog_token, uint8_t uc_ac_num)
{
    mac_ts_stru *pst_ts_tmp = NULL;
    mac_ts_stru *pst_ts = NULL;

    pst_ts = &(pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num]);
    pst_ts_tmp = &pst_hmac_user->st_ts_tmp_info[uc_ac_num];
    if (pst_ts->en_ts_status == MAC_TS_SUCCESS) {
        if (pst_ts_tmp->en_ts_status == MAC_TS_INPROGRESS) {
            if (pst_ts_tmp->st_addts_timer.en_is_registerd == OAL_TRUE) {
                frw_timer_immediate_destroy_timer_m(&pst_ts_tmp->st_addts_timer);
            }
            pst_ts_tmp->en_ts_status = MAC_TS_NONE;
            pst_ts_tmp->uc_second_flag = OAL_FALSE;
        }
        return OAL_SUCC;
    }

    if (pst_ts->uc_tsid != uc_tsid) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC,
            "{hmac_mgmt_rx_addts_rsp::addba rsp tsid[%d],old_tsid[%d]} ",
            uc_tsid, pst_ts->uc_tsid);
        return OAL_SUCC;
    }

    if (uc_dialog_token != pst_ts->uc_ts_dialog_token) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC,
            "{hmac_mgmt_rx_addts_rsp::addts rspuc_dialog_token wrong.rsp dialog[%d], req dialog[%d]}",
            uc_dialog_token, pst_ts->uc_ts_dialog_token);
        return OAL_SUCC;
    }

    frw_timer_immediate_destroy_timer_m(&pst_ts->st_addts_timer);
    return OAL_FAIL;
}

static void hmac_update_user_ts_info(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user,
                                     mac_ts_stru *pst_ts, uint8_t *puc_payload, uint8_t uc_ac_num)
{
    /* 根据ADDTS RSP信息，更新dmac TS状态 */
    if (puc_payload[3] == MAC_SUCCESSFUL_STATUSCODE) { /* | Category 1| Action 1|Dialog token 1| 3 */
        pst_ts->en_ts_status = MAC_TS_SUCCESS;
        pst_hmac_user->st_user_base_info.need_degrade[uc_ac_num] = OAL_FALSE;
    } else {
        memset_s(pst_ts, sizeof(mac_ts_stru), 0, sizeof(mac_ts_stru));
        pst_ts->en_ts_status = MAC_TS_INIT;
    }

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC,
                     "{hmac_mgmt_rx_addts_rsp::medium_time[%d], tsid[%d], uc_up[%d], en_direction[%d].}",
                     pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num].medium_time,
                     pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num].uc_tsid,
                     pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num].uc_up,
                     pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num].en_direction);

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC,
                     "{hmac_mgmt_rx_addts_rsp::ts status[%d], dialog_token[%d], vap id[%d], ac num[%d].}",
                     pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num].en_ts_status,
                     pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num].uc_ts_dialog_token,
                     pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num].uc_vap_id,
                     uc_ac_num);
}

uint32_t hmac_mgmt_rx_addts_rsp(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user,
    uint8_t *puc_payload, uint32_t frame_body_len)
{
    mac_vap_stru *pst_mac_vap = NULL;
    uint8_t uc_user_prio;
    uint8_t uc_ac_num;
    uint8_t uc_tsid;
    uint8_t uc_dialog_token;
    mac_wmm_tspec_stru st_tspec_args;
    mac_wmm_tspec_stru *pst_tspec_info = NULL;
    mac_ts_stru *pst_ts = NULL;

    if (oal_any_null_ptr3(pst_hmac_vap, pst_hmac_user, puc_payload)) {
        oam_error_log0(0, OAM_SF_WMMAC, "{hmac_mgmt_rx_addts_rsp::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (frame_body_len < MAC_ADDTS_RSP_FRAME_BODY_LEN) { /* 10个元素长度总和 12  */
        oam_warning_log1(0, OAM_SF_WMMAC, "{hmac_mgmt_rx_addts_rsp::frame_body_len [%d]}", frame_body_len);
        return OAL_FAIL;
    }
    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
/*
 *   ***************************************************************************
 *   *                ADDTS RSP frame body                                     *
 *   * ----------------------------------------------------------------------- *
 *   * |MAC HEADER| Category| Action| Dialog token| Status| Elements| FCS|     *
 *   * ----------------------------------------------------------------------- *
 *   * |24/30     |1        |1      |1            |1      |         |4   |     *
 *   * ----------------------------------------------------------------------- *
 *   *                                                                         *
 *   ***************************************************************************
 *   * Category 1| Action 1| Dialog token 1| Status1 |ID 1| Length 1| OUI 3|OUI Type 1| OUI subtype 1| Version 1|
 */
    pst_tspec_info = (mac_wmm_tspec_stru *)(puc_payload + 12);  /* 上面10个元素长度总和 12  */
    uc_user_prio = pst_tspec_info->ts_info.bit_user_prio;

    /* 协议支持tid为0~15,02只支持tid0~7 */
    if (uc_user_prio >= WLAN_TID_MAX_NUM) {
        /* 对于tid > 7的resp直接忽略 */
        oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC,
            "{hmac_mgmt_rx_addts_rsp::addts rsp tsid[%d]} token[%d] status[%d]",
            uc_user_prio, puc_payload[2], puc_payload[3]); /* | Category 1 | Action 1 | 2, Dialog token 1| 3 */
        return OAL_SUCC;
    }
    uc_ac_num = WLAN_WME_TID_TO_AC(uc_user_prio);
    uc_tsid = pst_tspec_info->ts_info.bit_tsid;

    /* 获得之前保存的ts信息 */
    pst_ts = &(pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num]);
    uc_dialog_token = puc_payload[2]; /* | Category| Action| 2 */
    if (hmac_update_user_st_temp_info(pst_hmac_user, pst_mac_vap, uc_tsid, uc_dialog_token, uc_ac_num) == OAL_SUCC) {
        return OAL_SUCC;
    }
    /* 如果对应TSID的TS未建立，则发送DELTS */
    if (pst_ts->en_ts_status == MAC_TS_INIT) {
        /* 发送DELTS帧 */
        st_tspec_args.ts_info.bit_tsid = uc_tsid;
        st_tspec_args.ts_info.bit_user_prio = uc_user_prio;
        st_tspec_args.ts_info.bit_acc_policy = 1;
        hmac_mgmt_tx_delts(pst_hmac_vap, pst_hmac_user, &st_tspec_args);
        return OAL_SUCC;
    }

    hmac_update_user_ts_info(pst_mac_vap, pst_hmac_user, pst_ts, puc_payload, uc_ac_num);

    return OAL_SUCC;
}


uint32_t hmac_mgmt_rx_delts(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user,
    uint8_t *puc_payload, uint32_t frame_body_len)
{
    mac_vap_stru *pst_mac_vap = NULL;
    uint8_t uc_ac_num;
    uint8_t uc_tsid;
    mac_wmm_tspec_stru *pst_tspec_info = NULL;
    mac_ts_stru *pst_ts = NULL;
    if (oal_any_null_ptr3(pst_hmac_vap, pst_hmac_user, puc_payload)) {
        oam_error_log0(0, OAM_SF_WMMAC, "{hmac_mgmt_rx_delts::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (frame_body_len < MAC_DELTS_FRAME_BODY_LEN) {
        oam_warning_log1(0, OAM_SF_WMMAC, "{hmac_mgmt_rx_delts::frame_body_len [%d]}", frame_body_len);
        return OAL_FAIL;
    }
    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);

/*
 *   **************************************************************************
 *   *                DELTS frame body                                        *
 *   * ----------------------------------------------------------------------- *
 *   * |MAC HEADER| Category| Action| Dialog token| Status| Elements| FCS|   *
 *   * ----------------------------------------------------------------------- *
 *   * |24/30     |1        |1      |1            |1      |         |4   |   *
 *   * ----------------------------------------------------------------------- *
 *   *                                                                       *
 *   *************************************************************************
 *   * Category 1| Action 1| Dialog token 1| Status1 |ID 1| Length 1| OUI 3|OUI Type 1| OUI subtype 1| Version 1|
 */
    pst_tspec_info = (mac_wmm_tspec_stru *)(puc_payload + 12); /* 12  上面10个元素长度总和 */
    uc_tsid = pst_tspec_info->ts_info.bit_tsid;

    /* 协议支持tid为0~15,02只支持tid0~7 */
    if (uc_tsid >= WLAN_TID_MAX_NUM) {
        /* 对于tid > 7的resp直接忽略 */
        oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC,
            "{hmac_mgmt_rx_delts::tsid[%d]} token[%d] status[%d]",
            uc_tsid, puc_payload[2], puc_payload[3]); /* | Category 1| Action 1| 2, Dialog token 1| 3 */
        return OAL_SUCC;
    }
    /* 遍历所有AC，找到对应的TSID，清空TS信息 */
    for (uc_ac_num = 0; uc_ac_num < WLAN_WME_AC_BUTT; uc_ac_num++) {
        /* 获得之前保存的ts信息 */
        pst_ts = &(pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num]);
        if (pst_ts->uc_tsid != uc_tsid) {
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC,
                "{hmac_mgmt_rx_delts::tsid[%d],old_tsid[%d]} ", uc_tsid, pst_ts->uc_tsid);
            continue;
        }

        /* 清空已保存的TS信息 */
        if (pst_ts->st_addts_timer.en_is_registerd == OAL_TRUE) {
            frw_timer_immediate_destroy_timer_m(&(pst_ts->st_addts_timer));
        }
        memset_s(pst_ts, sizeof(mac_ts_stru), 0, sizeof(mac_ts_stru));
        if (OAL_TRUE == mac_mib_get_QAPEDCATableMandatory(&(pst_hmac_vap->st_vap_base_info), uc_ac_num)) {
            pst_ts->en_ts_status = MAC_TS_INIT;
        } else {
            pst_ts->en_ts_status = MAC_TS_NONE;
        }
        pst_ts->uc_tsid = 0xFF;
    }

    return OAL_SUCC;
}


uint32_t hmac_mgmt_rx_addts_req_frame(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    mac_user_stru *pst_mac_user = NULL;
    mac_rx_ctl_stru *pst_rx_ctrl = NULL;
    uint16_t us_user_idx = 0;
    uint8_t auc_sta_addr[WLAN_MAC_ADDR_LEN];
    uint32_t ret;

    oal_netbuf_stru *pst_addts_rsp = NULL;
    mac_tx_ctl_stru *pst_tx_ctl = NULL;
    uint16_t us_frame_len;

    if (oal_any_null_ptr2(pst_hmac_vap, pst_netbuf)) {
        oam_error_log0(0, OAM_SF_WMMAC,
                       "{hmac_mgmt_rx_addts_req_frame::pst_hmac_vap or pst_netbuf is null!}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    mac_get_address2((uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_ctrl), auc_sta_addr, WLAN_MAC_ADDR_LEN);

    ret = mac_vap_find_user_by_macaddr(&(pst_hmac_vap->st_vap_base_info), auc_sta_addr, &us_user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_WMMAC,
                         "{hmac_mgmt_rx_addts_req_frame::mac_vap_find_user_by_macaddr failed[%d].}", ret);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(us_user_idx);
    if (pst_mac_user == NULL) {
        oam_error_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_WMMAC,
            "{hmac_mgmt_rx_addts_req_frame::pst_mac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请ADDTS_RSP管理帧内存 */
    pst_addts_rsp = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_addts_rsp == NULL) {
        oam_error_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_WMMAC,
            "{hmac_mgmt_rx_addts_req_frame::pst_addts_rsp null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_mem_netbuf_trace(pst_addts_rsp, OAL_TRUE);

    memset_s(oal_netbuf_cb(pst_addts_rsp), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_addts_rsp);

    oal_netbuf_prev(pst_addts_rsp) = NULL;
    oal_netbuf_next(pst_addts_rsp) = NULL;

    /* 调用封装管理帧接口 */
    us_frame_len = hmac_mgmt_encap_addts_rsp(pst_hmac_vap, oal_netbuf_data(pst_addts_rsp),
        oal_netbuf_data(pst_netbuf), pst_rx_ctrl->us_frame_len);
    if (us_frame_len == 0) {
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_WMMAC,
            "{hmac_mgmt_rx_addts_req_frame::encap_addts_rsp, but frame len is zero.}");
        oal_netbuf_free(pst_addts_rsp);
        return OAL_FAIL;
    }

    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = us_user_idx; /* dmac发送需要的mpdu长度 */
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_frame_len;   /* 发送完成需要获取user结构体 */

    oal_netbuf_put(pst_addts_rsp, us_frame_len);

    /* Buffer this frame in the Memory Queue for transmission */
    ret = hmac_tx_mgmt_send_event(&(pst_hmac_vap->st_vap_base_info), pst_addts_rsp, us_frame_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_addts_rsp);
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_WMMAC,
            "{hmac_mgmt_rx_addts_req_frame::hmac_tx_mgmt_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

uint32_t hmac_config_wmmac_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint8_t uc_ac_idx;
    hmac_user_stru *pst_hmac_user = NULL;
    mac_cfg_wmm_ac_param_stru *pst_wmm_ac_param = NULL;
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC, "{hmac_config_wmmac_switch::pst_hmac_vap is  NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_wmm_ac_param = (mac_cfg_wmm_ac_param_stru *)puc_param;
    g_en_wmmac_switch = pst_wmm_ac_param->en_wmm_ac_switch;
    pst_hmac_vap->en_wmmac_auth_flag = pst_wmm_ac_param->en_auth_flag;

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(pst_mac_vap->us_assoc_vap_id);
    if (pst_hmac_user == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC,
            "{hmac_config_wmmac_switch::pst_hmac_user is deleted already.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 关闭wmmac开关时，需要删除user下所有的TS信息 */
    if (!g_en_wmmac_switch) {
        for (uc_ac_idx = 0; uc_ac_idx < WLAN_WME_AC_BUTT; uc_ac_idx++) {
            if (pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_idx].st_addts_timer.en_is_registerd == OAL_TRUE) {
                frw_timer_immediate_destroy_timer_m(
                    &(pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_idx].st_addts_timer));
            }
            memset_s(&(pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_idx]),
                     sizeof(mac_ts_stru), 0, sizeof(mac_ts_stru));
            pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_idx].en_ts_status = MAC_TS_NONE;
            pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_idx].uc_tsid = 0xFF;

            if (pst_hmac_user->st_ts_tmp_info[uc_ac_idx].st_addts_timer.en_is_registerd == OAL_TRUE) {
                frw_timer_immediate_destroy_timer_m(&(pst_hmac_user->st_ts_tmp_info[uc_ac_idx].st_addts_timer));
            }
            memset_s(&(pst_hmac_user->st_ts_tmp_info[uc_ac_idx]),
                     sizeof(mac_ts_stru), 0, sizeof(mac_ts_stru));
            pst_hmac_user->st_ts_tmp_info[uc_ac_idx].en_ts_status = MAC_TS_NONE;
            pst_hmac_user->st_ts_tmp_info[uc_ac_idx].uc_tsid = 0xFF;
        }
    }

    /***************************************************************************
    抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_WMMAC_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC,
            "{hmac_config_wmmac_switch::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

uint32_t hmac_config_d2h_wmmac_update_info(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    hmac_user_stru *hmac_user = NULL;
    dmac_wmmac_info_sync_stru *wmmac_param = (dmac_wmmac_info_sync_stru *)param;

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(wmmac_param->user_id);
    if (hmac_user == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "hmac_config_d2h_wmmac_update_info: hmac_user is null ptr. user id:%d", wmmac_param->user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user->st_user_base_info.need_degrade[wmmac_param->ac_idx] = wmmac_param->need_degrade;
    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
        "hmac_config_d2h_wmmac_update_info::user_id[%d] ac[%d] need_degrade[%d]",
        wmmac_param->user_id, wmmac_param->ac_idx, hmac_user->st_user_base_info.need_degrade[wmmac_param->ac_idx]);
    return OAL_SUCC;
}

oal_bool_enum_uint8 hmac_need_degrade_for_ts_1106(uint8_t tid, uint8_t need_degrade, uint8_t wmmac_auth_flag)
{
    return (need_degrade && (tid > WLAN_TIDNO_BEST_EFFORT) && (wmmac_auth_flag == OAL_FALSE));
}

oal_bool_enum_uint8 hmac_need_degrade_for_ts_1103(uint8_t tid, uint8_t need_degrade, uint8_t wmmac_auth_flag)
{
    return OAL_FALSE;
}

oal_bool_enum_uint8 hmac_need_degrade_for_wmmac(hmac_vap_stru *hmac_vap, mac_tx_ctl_stru *tx_ctl,
    mac_user_stru *user, uint8_t ac_num, uint8_t tid)
{
    if (((mac_mib_get_QAPEDCATableMandatory(&(hmac_vap->st_vap_base_info), ac_num) == OAL_TRUE) &&
        !MAC_GET_CB_IS_VIPFRAME(tx_ctl) && (user->st_ts_info[ac_num].en_ts_status != MAC_TS_SUCCESS)) ||
        ((mac_mib_get_QAPEDCATableMandatory(&(hmac_vap->st_vap_base_info), ac_num) == OAL_TRUE) &&
        (wlan_chip_wmmac_need_degrade_for_ts(tid, user->need_degrade[ac_num], hmac_vap->en_wmmac_auth_flag)))) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}
void hmac_sta_up_rx_action_wmmac_qos_proc(hmac_vap_stru *pst_hmac_vap,
    hmac_user_stru *pst_hmac_user, uint8_t *puc_data, uint32_t frame_body_len)
{
    if (g_en_wmmac_switch == OAL_TRUE) {
        switch (puc_data[MAC_ACTION_OFFSET_ACTION]) {
            case MAC_WMMAC_ACTION_ADDTS_RSP:
                hmac_mgmt_rx_addts_rsp(pst_hmac_vap, pst_hmac_user, puc_data, frame_body_len);
                break;
            case MAC_WMMAC_ACTION_DELTS:
                hmac_mgmt_rx_delts(pst_hmac_vap, pst_hmac_user, puc_data, frame_body_len);
                break;
            default:
                break;
        }
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  // _PRE_WLAN_FEATURE_WMMAC
