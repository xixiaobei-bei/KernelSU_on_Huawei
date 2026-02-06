

/* 1 头文件包含 */
#include "hmac_mgmt_ap.h"
#include "hmac_mgmt_sta.h"
#include "hmac_support_pmf.h"
#include "hmac_encap_frame.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_device.h"
#include "hmac_resource.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SUPPORT_PMF_C

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)

oal_uint32 hmac_sa_query_del_user(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user)
{
    wlan_vap_mode_enum_uint8 en_vap_mode;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    mac_sa_query_stru *pst_sa_query_info;

    if ((pst_mac_vap == OAL_PTR_NULL) || (pst_hmac_user == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_PMF,
                       "{hmac_sa_query_del_user::param null, %x %x.}",
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
            hmac_handle_disconnect_rsp_ap(pst_hmac_vap, pst_hmac_user);
        }
        break;
        case WLAN_VAP_MODE_BSS_STA: {
            /* 上报内核sta已经和某个ap去关联 */
            hmac_sta_handle_disassoc_rsp(pst_hmac_vap, MAC_DEAUTH_LV_SS);
        }
        break;
        default:
            break;
    }

    /* 删除user */
    ul_ret = hmac_user_del(&pst_hmac_vap->st_vap_base_info, pst_hmac_user);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_device_id, OAM_SF_PMF, "{hmac_sa_query_del_user::hmac_user_del failed[%d].}",
                       ul_ret);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


oal_uint32 hmac_send_sa_query_req(mac_vap_stru *pst_mac_vap,
                                  hmac_user_stru *pst_hmac_user,
                                  oal_bool_enum_uint8 en_is_protected,
                                  oal_uint16 us_init_trans_id)
{
    oal_uint16 us_sa_query_len;
    oal_netbuf_stru *pst_sa_query;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    if ((pst_mac_vap == OAL_PTR_NULL) || (pst_hmac_user == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_PMF,
                       "{hmac_send_sa_query_req::param null, %x %x.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)pst_hmac_user);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请SA Query 帧空间 */
    pst_sa_query = (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2,
                                                           OAL_NETBUF_PRIORITY_MID);
    if (pst_sa_query == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_PMF, "{hmac_send_sa_query_req::pst_sa_query null.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 封装SA Query request帧 */
    memset_s(oal_netbuf_cb(pst_sa_query), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    us_sa_query_len = hmac_encap_sa_query_req(pst_mac_vap,
                                              (oal_uint8 *)oal_netbuf_header(pst_sa_query),
                                              pst_hmac_user->st_user_base_info.auc_user_mac_addr,
                                              pst_hmac_user->st_sa_query_info.us_sa_query_trans_id);

    /* 单播管理帧加密 */
    if (en_is_protected == OAL_TRUE) {
        mac_set_protectedframe((oal_uint8 *)oal_netbuf_header(pst_sa_query));
    }

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_sa_query);                /* 获取cb结构体 */
    pst_tx_ctl->us_mpdu_len = us_sa_query_len;                                  /* dmac发送需要的mpdu长度 */
    pst_tx_ctl->us_tx_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;  /* 发送完成需要获取user结构体 */

    oal_netbuf_put(pst_sa_query, us_sa_query_len);

    /* Buffer this frame in the Memory Queue for transmission */
    ul_ret = hmac_tx_mgmt_send_event(pst_mac_vap, pst_sa_query, us_sa_query_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(pst_sa_query);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PMF,
                         "{hmac_send_sa_query_req::hmac_tx_mgmt_send_event failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

oal_uint32 hmac_pmf_check_err_code(mac_user_stru *pst_user_base_info, oal_bool_enum_uint8 en_is_protected,
                                   oal_uint8 *puc_mac_hdr)
{
    oal_bool_enum_uint8 en_aim_err_code;
    oal_uint16 us_err_code;

    us_err_code = oal_make_word16(puc_mac_hdr[MAC_80211_FRAME_LEN], puc_mac_hdr[MAC_80211_FRAME_LEN + 1]);
    en_aim_err_code = ((us_err_code == MAC_NOT_AUTHED) || (us_err_code == MAC_NOT_ASSOCED)) ? OAL_TRUE : OAL_FALSE;

    if ((pst_user_base_info->st_cap_info.bit_pmf_active == OAL_TRUE) &&
        ((en_aim_err_code == OAL_TRUE) || (en_is_protected == OAL_FALSE))) {
        return OAL_SUCC;
    }

    return OAL_FAIL;
}


oal_uint32 hmac_sa_query_interval_timeout(oal_void *p_arg)
{
    hmac_user_stru *pst_hmac_user;
    hmac_vap_stru *pst_hmac_vap;
    oal_uint32 ul_relt;
    oal_uint32 ul_now;
    oal_uint32 ul_passed;
    oal_uint32 ul_retry_timeout;

    pst_hmac_user = (hmac_user_stru *)p_arg;
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_sa_query_interval_timeout::invalid param.}");
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

    ul_now = (oal_uint32)oal_time_get_stamp_ms();
    ul_passed = (oal_uint32)oal_time_get_runtime(pst_hmac_user->st_sa_query_info.ul_sa_query_start_time, ul_now);
    /* SA Query总时间超时 */
    if (ul_passed >= mac_mib_get_dot11AssociationSAQueryMaximumTimeout(&pst_hmac_vap->st_vap_base_info)) {
        /* change state & ul_sa_query_count=0 */
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                         "{hmac_sa_query_interval_timeout_etc::Deleting user.}");
        ul_relt = hmac_sa_query_del_user(&(pst_hmac_vap->st_vap_base_info), pst_hmac_user);
        if (ul_relt != OAL_SUCC) {
            OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                           "{hmac_sa_query_interval_timeout_etc::hmac_sa_query_del_user failed[%d].}", ul_relt);
            return OAL_ERR_CODE_PMF_SA_QUERY_DEL_USER_FAIL;
        }

        return OAL_SUCC;
    }

    /* 未超时，则再发送一帧SA Query Request */
    pst_hmac_user->st_sa_query_info.us_sa_query_trans_id += 1;
    pst_hmac_user->st_sa_query_info.ul_sa_query_count += 1;
    OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                     "{hmac_sa_query_interval_timeout_etc::SA query, trans_id %d.}",
                     pst_hmac_user->st_sa_query_info.us_sa_query_trans_id);

    ul_retry_timeout = mac_mib_get_dot11AssociationSAQueryRetryTimeout(&pst_hmac_vap->st_vap_base_info);

    /* 设置间隔定时器 */
    frw_create_timer(&(pst_hmac_user->st_sa_query_info.st_sa_query_interval_timer),
                     hmac_sa_query_interval_timeout,
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


oal_uint32 hmac_start_sa_query(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user,
                               oal_bool_enum_uint8 en_is_protected)
{
    oal_uint16 us_retry_timeout;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_uint16 us_init_trans_id = 0;

    /* 入参判断 */
    if ((pst_mac_vap == OAL_PTR_NULL) || (pst_hmac_user == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY,
                       "{hmac_start_sa_query::param null, %x %x.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)pst_hmac_user);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 判断vap有无pmf能力 */
    if (pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active != OAL_TRUE) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_start_sa_query::bit_pmf_active is down.}");
        return OAL_ERR_CODE_PMF_DISABLED;
    }

    /* 避免重复启动SA Query流程 */
    if (pst_hmac_user->st_sa_query_info.ul_sa_query_count != 0) {
        oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_start_sa_query::SA Query is already in process.}");
        return OAL_SUCC;
    }

    /* 获得hmac vap 结构指针 */
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_start_sa_query::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    us_retry_timeout = (oal_uint16)mac_mib_get_dot11AssociationSAQueryRetryTimeout(pst_mac_vap);

    /* 记录sa query流程开始时间,单位ms */
    pst_hmac_user->st_sa_query_info.ul_sa_query_start_time = (oal_uint32)oal_time_get_stamp_ms();
    /* 获得初始trans_id */
    pst_hmac_user->st_sa_query_info.us_sa_query_trans_id = (oal_uint16)oal_time_get_stamp_ms();
    pst_hmac_user->st_sa_query_info.ul_sa_query_count = 1;
    pst_hmac_user->st_sa_query_info.en_is_protected = en_is_protected;

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_start_sa_query::SA query, trans_id %d.}",
                     pst_hmac_user->st_sa_query_info.us_sa_query_trans_id);

    /* 设置间隔定时器 */
    frw_create_timer(&(pst_hmac_user->st_sa_query_info.st_sa_query_interval_timer),
                     hmac_sa_query_interval_timeout,
                     us_retry_timeout,
                     (oal_void *)pst_hmac_user,
                     OAL_FALSE,
                     OAM_MODULE_ID_HMAC,
                     pst_mac_vap->ul_core_id);

    /* 发送SA Query request，开始查询流程 */
    ul_ret = hmac_send_sa_query_req(pst_mac_vap, pst_hmac_user, en_is_protected, us_init_trans_id);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_start_sa_query::hmac_send_sa_query_req failed[%d].}", ul_ret);
        return OAL_ERR_CODE_PMF_SA_QUERY_REQ_SEND_FAIL;
    }

    return OAL_SUCC;
}


oal_void hmac_send_sa_query_rsp(mac_vap_stru *pst_mac_vap, oal_uint8 *pst_hdr,
                                oal_bool_enum_uint8 en_is_protected)
{
    oal_uint16 us_sa_query_len;
    oal_netbuf_stru *pst_sa_query;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;
    oal_uint16 us_user_idx = MAC_INVALID_USER_ID;
    oal_uint32 ul_ret;

    if ((pst_mac_vap == OAL_PTR_NULL) || (pst_hdr == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY,
                       "{hmac_send_sa_query_rsp::param null, %d %d.}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)pst_hdr);
        return;
    }
    oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_send_sa_query_rsp::AP ready to TX a sa query rsp.}");

    pst_sa_query = (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2,
                                                           OAL_NETBUF_PRIORITY_MID);
    if (pst_sa_query == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_send_sa_query_rsp::pst_sa_query null.}");
        return;
    }

    memset_s(oal_netbuf_cb(pst_sa_query), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    us_sa_query_len = hmac_encap_sa_query_rsp(pst_mac_vap, pst_hdr, (oal_uint8 *)oal_netbuf_header(pst_sa_query));

    /* 单播管理帧加密 */
    if (en_is_protected == OAL_TRUE) {
        mac_set_protectedframe((oal_uint8 *)oal_netbuf_header(pst_sa_query));
    }

    mac_vap_find_user_by_macaddr(pst_mac_vap, ((mac_ieee80211_frame_stru *)pst_hdr)->auc_address2, &us_user_idx);

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_sa_query); /* 获取cb结构体 */
    pst_tx_ctl->us_mpdu_len = us_sa_query_len;                   /* dmac发送需要的mpdu长度 */
    pst_tx_ctl->us_tx_user_idx = us_user_idx;                    /* 发送完成需要获取user结构体 */

    oal_netbuf_put(pst_sa_query, us_sa_query_len);

    /* Buffer this frame in the Memory Queue for transmission */
    ul_ret = hmac_tx_mgmt_send_event(pst_mac_vap, pst_sa_query, us_sa_query_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(pst_sa_query);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_send_sa_query_rsp::hmac_tx_mgmt_send_event failed[%d].}", ul_ret);
    }

    return;
}


oal_void hmac_rx_sa_query_req(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf,
                              oal_bool_enum_uint8 en_is_protected)
{
    oal_uint8 *puc_sa;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_uint8 *puc_mac_hdr = OAL_PTR_NULL;

    if ((pst_hmac_vap == OAL_PTR_NULL) || (pst_netbuf == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_RX,
                       "{hmac_rx_sa_query_req::null param %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_netbuf);
        return;
    }

    puc_mac_hdr = oal_netbuf_header(pst_netbuf);

    mac_rx_get_sa((mac_ieee80211_frame_stru *)puc_mac_hdr, &puc_sa);
    pst_hmac_user = mac_vap_get_hmac_user_by_addr(&pst_hmac_vap->st_vap_base_info, puc_sa);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                       "{hmac_rx_sa_query_req::pst_hmac_user null.}");
        return;
    }

    /* 如果该用户的管理帧加密属性不一致，丢弃该报文 */
    if (en_is_protected != pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active) {
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                         "{hmac_rx_sa_query_req::PMF check failed.}");
        return;
    }

    /* sa Query rsp发送 */
    hmac_send_sa_query_rsp(&pst_hmac_vap->st_vap_base_info, puc_mac_hdr, en_is_protected);

    return;
}

oal_void hmac_rx_sa_query_rsp(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf,
                              oal_bool_enum_uint8 en_is_protected)
{
    oal_uint8 *puc_mac_hdr = OAL_PTR_NULL;
    oal_uint8 *puc_sa;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_uint16 *pus_trans_id;
    mac_sa_query_stru *pst_sa_query_info;

    if ((pst_hmac_vap == OAL_PTR_NULL) || (pst_netbuf == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_AMPDU,
                       "{hmac_rx_sa_query_rsp::param null,%x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_netbuf);
        return;
    }

    puc_mac_hdr = oal_netbuf_header(pst_netbuf);

    mac_rx_get_sa((mac_ieee80211_frame_stru *)puc_mac_hdr, &puc_sa);
    pst_hmac_user = mac_vap_get_hmac_user_by_addr(&pst_hmac_vap->st_vap_base_info, puc_sa);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMPDU,
                       "{hmac_rx_sa_query_rsp::pst_hmac_user null.}");
        return;
    }

    /* 如果该用户的管理帧加密属性不一致，丢弃该报文 */
    if (en_is_protected != pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMPDU,
                       "{hmac_rx_sa_query_rsp::PMF check failed.}");
        return;
    }

    /* 对比trans_id */
    pus_trans_id = (oal_uint16 *)(puc_mac_hdr + MAC_80211_FRAME_LEN + 2); /* 2表示duration */
    pst_sa_query_info = &pst_hmac_user->st_sa_query_info;

    /* 收到有效的SA query reqponse，保留这条有效的SA */
    if (oal_memcmp(pus_trans_id, &(pst_sa_query_info->us_sa_query_trans_id), 2) == 0) { /* 2表示字符串比较个数 */
        /* pending SA Query requests 计数器清零 & sa query流程开始时间清零 */
        pst_sa_query_info->ul_sa_query_count = 0;
        pst_sa_query_info->ul_sa_query_start_time = 0;

        /* 删除timer */
        if (pst_sa_query_info->st_sa_query_interval_timer.en_is_registerd != OAL_FALSE) {
            frw_immediate_destroy_timer(&(pst_sa_query_info->st_sa_query_interval_timer));
        }
    }

    return;
}

oal_void hmac_sta_roam_update_pmf(mac_vap_stru *pst_mac_vap, mac_bss_dscr_stru *pst_bss_dscr)
{
    oal_bool_enum_uint8 en_pmf_capable;
    oal_bool_enum_uint8 en_pmf_required;
    hmac_device_stru *pst_hmac_device;
    mac_rsn_cap_stru *pst_rsn_cap;

    if (pst_mac_vap->en_vap_state != MAC_VAP_STATE_ROAMING) {
        return;
    }

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "hmac_sta_roam_update_pmf_etc:: hmac device not find.");
        return;
    }

    pst_rsn_cap = (mac_rsn_cap_stru *)pst_bss_dscr->st_bss_sec_info.auc_rsn_cap;

    en_pmf_capable = pst_rsn_cap->bit_mfpc;
    en_pmf_required = pst_rsn_cap->bit_mfpr;

    if (hmac_device_pmf_find_black_list(pst_hmac_device, pst_bss_dscr->auc_mac_addr) && en_pmf_capable &&
        !en_pmf_required) {
        en_pmf_capable = OAL_FALSE;
        en_pmf_required = OAL_FALSE;
        oam_warning_log3(0, OAM_SF_ANY, "hmac_sta_roam_update_pmf::black list mac addr %02X:xx:xx:xx:%02X:%02X",
                         /* auc_mac_addr的0、4、5byte为参数输出打印 */
                         pst_bss_dscr->auc_mac_addr[0], pst_bss_dscr->auc_mac_addr[4], pst_bss_dscr->auc_mac_addr[5]);
    }

    hmac_config_set_pmf_cap(pst_mac_vap, en_pmf_capable, en_pmf_required);
}
oal_void hmac_sta_up_category_sa_query(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf,
    oal_bool_enum_uint8 en_is_protected, oal_uint8 *puc_data)
{
    switch (puc_data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_SA_QUERY_ACTION_REQUEST:
            hmac_rx_sa_query_req(pst_hmac_vap, pst_netbuf, en_is_protected);
            break;
        case MAC_SA_QUERY_ACTION_RESPONSE:
            hmac_rx_sa_query_rsp(pst_hmac_vap, pst_netbuf, en_is_protected);
            break;

        default:
            break;
    }
}

oal_void hmac_stop_sa_query_timer(hmac_user_stru *pst_hmac_user)
{
    frw_timeout_stru *pst_sa_query_interval_timer;

    pst_sa_query_interval_timer = &(pst_hmac_user->st_sa_query_info.st_sa_query_interval_timer);
    if (pst_sa_query_interval_timer->en_is_registerd != OAL_FALSE) {
        frw_immediate_destroy_timer(pst_sa_query_interval_timer);
    }
}

oal_uint32 hmac_check_sa_query_trigger(
    hmac_user_stru *pst_hmac_user, hmac_vap_stru *pst_hmac_vap, mac_status_code_enum_uint16 *pen_status_code)
{
    oal_uint32 ul_rslt;

    if ((pst_hmac_user->st_sa_query_info.st_sa_query_interval_timer.en_is_enabled != OAL_TRUE) &&
        (pst_hmac_user->st_user_base_info.en_user_asoc_state == MAC_USER_STATE_ASSOC) &&
        (pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active == OAL_TRUE)) {
        oam_info_log0(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                      "{hmac_check_sa_query_trigger::AP rx STA assoc req ,and start sa query process.}");
        ul_rslt = hmac_start_sa_query(&pst_hmac_vap->st_vap_base_info, pst_hmac_user,
                                      pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active);
        if (ul_rslt != OAL_SUCC) {
            OAM_ERROR_LOG1(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                           "{hmac_check_sa_query_trigger::hmac_start_sa_query failed[%d].}", ul_rslt);
            return ul_rslt;
        }
        oam_info_log0(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                      "{hmac_check_sa_query_trigger::set en_status_code is MAC_REJECT_TEMP.}");
        *pen_status_code = MAC_REJECT_TEMP;
    }
    return OAL_SUCC;
}
oal_void hmac_ap_up_category_sa_query(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_data,
    oal_netbuf_stru *pst_netbuf, oal_bool_enum_uint8 en_is_protected)
{
    /* Action */
    switch (puc_data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_SA_QUERY_ACTION_REQUEST:
            hmac_rx_sa_query_req(pst_hmac_vap, pst_netbuf, en_is_protected);
            break;
        case MAC_SA_QUERY_ACTION_RESPONSE:
            hmac_rx_sa_query_rsp(pst_hmac_vap, pst_netbuf, en_is_protected);
            break;
        default:
            break;
    }
}

oal_uint32 hmac_get_assoc_comeback_time(mac_vap_stru *pst_mac_vap,
                                        hmac_user_stru *pst_hmac_user)
{
    oal_uint32                     ul_timeout = 0;
    oal_uint32                     ul_now_time;
    oal_uint32                     ul_passed_time;
    oal_uint32                     ul_sa_query_maxtimeout;

    /* 获取现在时间 */
    ul_now_time = (oal_uint32)oal_time_get_stamp_ms();

    /* 设置ASSOCIATION_COMEBACK_TIME，使STA在AP完成SA Query流程之后再发送关联请求 */
    /* 获得sa Query Max timeout值 */
    ul_sa_query_maxtimeout = mac_mib_get_dot11AssociationSAQueryMaximumTimeout(pst_mac_vap);

    /* 是否现在有sa Query流程正在进行 */
    if ((pst_hmac_user->st_sa_query_info.st_sa_query_interval_timer.en_is_enabled == OAL_TRUE) &&
        (ul_now_time >= pst_hmac_user->st_sa_query_info.ul_sa_query_start_time)) {
        /* 待现有SA Query流程结束后才可以接受STA发过来的关联帧 */
        ul_passed_time = ul_now_time - pst_hmac_user->st_sa_query_info.ul_sa_query_start_time;
        ul_timeout = ul_sa_query_maxtimeout - ul_passed_time;
    } else {
        /* 给接下来的SA Query流程预留时间 */
        ul_timeout = ul_sa_query_maxtimeout;
    }

    return ul_timeout;
}

oal_bool_enum_uint8 hmac_device_pmf_find_black_list(hmac_device_stru *pst_hmac_dev, oal_uint8 *puc_mac_addr)
{
    oal_uint8 uc_index;

    for (uc_index = 0; uc_index < pst_hmac_dev->st_pmf_black_list.uc_cnt; ++uc_index) {
        if (oal_memcmp(pst_hmac_dev->st_pmf_black_list.auc_black_list[uc_index],
                       puc_mac_addr, WLAN_MAC_ADDR_LEN) == 0) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


oal_void hmac_device_pmf_add_black_list(hmac_device_stru *pst_hmac_dev, oal_uint8 *puc_mac_addr)
{
    oal_uint8 uc_black_add_pos;

    if (hmac_device_pmf_find_black_list(pst_hmac_dev, puc_mac_addr)) {
        return;
    }

    /* 黑名单列表未满，插入到列表末尾，更新黑名单总数，否则覆盖最早的一个 */
    if (pst_hmac_dev->st_pmf_black_list.uc_cnt < PMF_BLACK_LIST_MAX_CNT) {
        uc_black_add_pos = pst_hmac_dev->st_pmf_black_list.uc_cnt++;
    } else {
        uc_black_add_pos = pst_hmac_dev->st_pmf_black_list.uc_first_idx++;
        pst_hmac_dev->st_pmf_black_list.uc_first_idx %= PMF_BLACK_LIST_MAX_CNT;
    }

    if (memcpy_s(pst_hmac_dev->st_pmf_black_list.auc_black_list[uc_black_add_pos],
                 WLAN_MAC_ADDR_LEN, puc_mac_addr, WLAN_MAC_ADDR_LEN) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_pmf_add_black_list::memcpy fail!");
        return;
    }

    oam_warning_log4(0, OAM_SF_ANY, "hmac_pmf_add_black_list::cnt[%d] mac addr %02X:xx:xx:xx:%02X:%02X",
                     /* puc_mac_addr第0、4、5byte作为参数输出打印 */
                     pst_hmac_dev->st_pmf_black_list.uc_cnt, puc_mac_addr[0], puc_mac_addr[4], puc_mac_addr[5]);
}

oal_void hmac_pmf_update_pmf_black_list(mac_vap_stru *pst_mac_vap, mac_bss_dscr_stru *pst_bss_dscr,
                                        mac_nl80211_mfp_enum_uint8 en_conn_pmf_enable)
{
    hmac_device_stru *pst_hmac_devive;

    /* PMF Required: BIT6, PMF Capable: BIT7 */
    if (((pst_bss_dscr->st_bss_sec_info.auc_rsn_cap[0] & BIT7) != 0) &&
        ((pst_bss_dscr->st_bss_sec_info.auc_rsn_cap[0] & BIT6) == 0) &&
        (en_conn_pmf_enable == MAC_NL80211_MFP_NO)) {
        pst_hmac_devive = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
        if (pst_hmac_devive == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "hmac_pmf_update_pmf_black_list:: pst_hmac_devive null");
            return;
        }

        hmac_device_pmf_add_black_list(pst_hmac_devive, pst_bss_dscr->auc_mac_addr);
    }
}

oal_uint32 hmac_config_set_pmf_cap(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_pmf_capable,
                                   oal_bool_enum_uint8 en_pmf_required)
{
    oal_uint32 ul_ret;
    mac_cfg_pmf_cap_stru st_pmf_cap;

    mac_mib_set_dot11RSNAMFPC(pst_mac_vap, en_pmf_capable);
    mac_mib_set_dot11RSNAMFPR(pst_mac_vap, en_pmf_required);
    pst_mac_vap->en_user_pmf_cap = en_pmf_capable;

    st_pmf_cap.en_pmf_capable = en_pmf_capable;
    st_pmf_cap.en_pmf_required = en_pmf_required;

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_vap_pmf_cap::pmf cap[%d], req[%d]",
                     en_pmf_capable, en_pmf_required);

    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PMF_CAP, OAL_SIZEOF(mac_cfg_pmf_cap_stru),
                                    (oal_uint8 *)&st_pmf_cap);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_vap_pmf_cap::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}

oal_uint32 hmac_enable_pmf(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_param)
{
    oal_bool_enum_uint8 en_pmf_active;
    wlan_pmf_cap_status_uint8 *puc_pmf_cap;
    oal_dlist_head_stru *pst_entry;
    oal_dlist_head_stru *pst_user_list_head;
    mac_user_stru *pst_user_tmp;

    if ((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY, "hmac_enable_pmf:: pointer is null: pst_mac_vap[%x],puc_param[%x]",
                       (uintptr_t)pst_mac_vap, (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    puc_pmf_cap = (wlan_pmf_cap_status_uint8 *)puc_param;

    switch (*puc_pmf_cap) {
        case MAC_PMF_DISABLED: {
            mac_mib_set_dot11RSNAMFPC(pst_mac_vap, OAL_FALSE);
            mac_mib_set_dot11RSNAMFPR(pst_mac_vap, OAL_FALSE);
            mac_mib_set_rsnaactivated(pst_mac_vap, OAL_FALSE);
            en_pmf_active = OAL_FALSE;
        }
        break;
        case MAC_PMF_ENABLED: {
            mac_mib_set_dot11RSNAMFPC(pst_mac_vap, OAL_TRUE);
            mac_mib_set_dot11RSNAMFPR(pst_mac_vap, OAL_FALSE);
            mac_mib_set_rsnaactivated(pst_mac_vap, OAL_TRUE);
            return OAL_SUCC;
        }
        case MAC_PME_REQUIRED: {
            mac_mib_set_dot11RSNAMFPC(pst_mac_vap, OAL_TRUE);
            mac_mib_set_dot11RSNAMFPR(pst_mac_vap, OAL_TRUE);
            mac_mib_set_rsnaactivated(pst_mac_vap, OAL_TRUE);
            en_pmf_active = OAL_TRUE;
        }
        break;
        default:
        {
            OAL_IO_PRINT("hmac_enable_pmf: commend error!");
            return OAL_FALSE;
        }
    }

    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP) {
        pst_user_list_head = &(pst_mac_vap->st_mac_user_list_head);

        for (pst_entry = pst_user_list_head->pst_next; pst_entry != pst_user_list_head;) {
            pst_user_tmp = oal_dlist_get_entry(pst_entry, mac_user_stru, st_user_dlist);

            /* 指向双向链表下一个节点 */
            pst_entry = pst_entry->pst_next;
            if (pst_user_tmp == OAL_PTR_NULL) {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_enable_pmf:: pst_user_tmp is null");
                return OAL_ERR_CODE_PTR_NULL;
            }
            mac_user_set_pmf_active(pst_user_tmp, en_pmf_active);
        }
    }

    return OAL_SUCC;
}
#endif
