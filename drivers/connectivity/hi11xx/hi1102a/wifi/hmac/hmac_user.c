
#include "oam_ext_if.h"
#include "dmac_ext_if.h"
#include "hmac_user.h"
#include "hmac_fsm.h"
#include "hmac_main.h"
#include "hmac_tx_amsdu.h"
#include "hmac_protection.h"
#include "hmac_ext_if.h"
#include "hmac_config.h"
#include "hmac_mgmt_ap.h"
#include "hmac_chan_mgmt.h"
#include "hmac_scan.h"
#include "hmac_204080_coexist.h"
#include "hmac_support_pmf.h"
#include "securec.h"
#ifdef _PRE_WLAN_FEATURE_WAPI
#include "hmac_wapi.h"
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
#include "hmac_roam_main.h"
#endif  // _PRE_WLAN_FEATURE_ROAM

#include "hmac_blockack.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_USER_C
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
hmac_user_stru g_ast_hmac_user[MAC_RES_MAX_USER_NUM];
#endif


hmac_user_stru *mac_res_get_hmac_user_alloc(oal_uint16 us_idx)
{
    hmac_user_stru *pst_hmac_user;

    pst_hmac_user = (hmac_user_stru *)_mac_res_get_hmac_user(us_idx);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_UM, "{mac_res_get_hmac_user_init::pst_hmac_user null,user_idx=%d.}", us_idx);
        return OAL_PTR_NULL;
    }

    /* 重复申请异常,避免影响业务，暂时打印error但正常申请 */
    if (pst_hmac_user->st_user_base_info.uc_is_user_alloced == MAC_USER_ALLOCED) {
        OAM_ERROR_LOG1(0, OAM_SF_UM, "{mac_res_get_hmac_user_init::[E]user has been alloced,user_idx=%d.}", us_idx);
    }

    return pst_hmac_user;
}


hmac_user_stru *mac_res_get_hmac_user(oal_uint16 us_idx)
{
    hmac_user_stru *pst_hmac_user;

    pst_hmac_user = (hmac_user_stru *)_mac_res_get_hmac_user(us_idx);
    if (pst_hmac_user == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    /* 异常: 用户资源已被释放, user idx0 为组播user */
    if ((pst_hmac_user->st_user_base_info.uc_is_user_alloced != MAC_USER_ALLOCED) && (us_idx != 0)) {
        OAM_WARNING_LOG1(0, OAM_SF_UM, "{mac_res_get_hmac_user::[E]user has been freed,user_idx=%d.}", us_idx);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        oal_mem_print_funcname(OAL_RET_ADDR);
#endif
        /* host侧获取用户时用户已经释放属于正常，返回空指针，后续调用者查找用户失败，请打印WARNING并直接释放buf，走其他分支等等 */
        return OAL_PTR_NULL;
    }

    return pst_hmac_user;
}


oal_void hmac_user_destroy_timer(hmac_user_stru *pst_hmac_user)
{
    oal_uint8 uc_tid;
    oal_uint8 uc_ac_idx;

    for (uc_tid = 0; uc_tid < WLAN_WME_MAX_TID_NUM; uc_tid++) {
        if (pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer.en_is_registerd == OAL_TRUE) {
            frw_immediate_destroy_timer(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_timer));
            OAM_ERROR_LOG1(0, OAM_SF_UM, "{hmac_user_destroy_timer::tid[%d] st_ba_timer not destroy.}", uc_tid);
        }
        if (pst_hmac_user->ast_tid_info[uc_tid].st_ba_tx_info.st_addba_timer.en_is_registerd == OAL_TRUE) {
            frw_immediate_destroy_timer(&(pst_hmac_user->ast_tid_info[uc_tid].st_ba_tx_info.st_addba_timer));
            OAM_WARNING_LOG1(0, OAM_SF_UM, "{hmac_user_destroy_timer::tid[%d] st_addba_timer not destroy.}", uc_tid);
        }
        if (pst_hmac_user->ast_hmac_amsdu[uc_tid].st_amsdu_timer.en_is_registerd == OAL_TRUE) {
            frw_immediate_destroy_timer(&(pst_hmac_user->ast_hmac_amsdu[uc_tid].st_amsdu_timer));
            OAM_ERROR_LOG1(0, OAM_SF_UM, "{hmac_user_destroy_timer::tid[%d] st_amsdu_timer not destroy.}", uc_tid);
        }
    }
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    if (pst_hmac_user->st_hmac_user_btcoex.st_hmac_btcoex_arp_req_process.st_delba_opt_timer.en_is_registerd ==
        OAL_TRUE) {
        frw_immediate_destroy_timer(&(pst_hmac_user->st_hmac_user_btcoex.st_hmac_btcoex_arp_req_process.
                                            st_delba_opt_timer));
        OAM_ERROR_LOG0(0, OAM_SF_UM, "{hmac_user_destroy_timer::st_delba_opt_timer not destroy.}");
    }
#endif
    if (pst_hmac_user->st_mgmt_timer.en_is_registerd == OAL_TRUE) {
        frw_immediate_destroy_timer(&(pst_hmac_user->st_mgmt_timer));
        OAM_ERROR_LOG0(0, OAM_SF_UM, "{hmac_user_destroy_timer::st_mgmt_timer not destroy.}");
    }

    if (pst_hmac_user->st_defrag_timer.en_is_registerd == OAL_TRUE) {
        frw_immediate_destroy_timer(&(pst_hmac_user->st_defrag_timer));
        OAM_ERROR_LOG0(0, OAM_SF_UM, "{hmac_user_destroy_timer::st_defrag_timer not destroy.}");
    }

    if (pst_hmac_user->st_sa_query_info.st_sa_query_interval_timer.en_is_registerd == OAL_TRUE) {
        frw_immediate_destroy_timer(&(pst_hmac_user->st_sa_query_info.st_sa_query_interval_timer));
        OAM_ERROR_LOG0(0, OAM_SF_UM, "{hmac_user_destroy_timer::st_sa_query_interval_timer not destroy.}");
    }
#ifdef _PRE_WLAN_FEATURE_WMMAC
    for (uc_ac_idx = 0; uc_ac_idx < WLAN_WME_AC_BUTT; uc_ac_idx++) {
        if (pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_idx].st_addts_timer.en_is_registerd == OAL_TRUE) {
            frw_immediate_destroy_timer(&(pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_idx].st_addts_timer));
            OAM_ERROR_LOG1(0, OAM_SF_UM,
                           "{hmac_user_destroy_timer::uc_ac_index[%d] st_addts_timer not destroy.}", uc_ac_idx);
        }
    }
#endif
}


oal_uint32 hmac_user_alloc_multi_user(oal_uint16 *pus_user_idx)
{
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_uint32 ul_rslt;
    oal_uint16 us_user_idx_temp;

    if (oal_unlikely(pus_user_idx == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_UM, "{hmac_user_alloc::pus_user_idx null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请hmac user内存 */
    /*lint -e413*/
    ul_rslt = mac_res_alloc_hmac_user(&us_user_idx_temp, OAL_OFFSET_OF(hmac_user_stru, st_user_base_info));
    if (ul_rslt != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_UM, "{hmac_user_alloc::mac_res_alloc_hmac_user failed[%d].}", ul_rslt);
        return ul_rslt;
    }
    /*lint +e413*/
    pst_hmac_user = mac_res_get_hmac_user_alloc(us_user_idx_temp);
    if (pst_hmac_user == OAL_PTR_NULL) {
        mac_res_free_mac_user(us_user_idx_temp);
        OAM_ERROR_LOG1(0, OAM_SF_UM, "{hmac_user_alloc::pst_hmac_user null,user_idx=%d.}", us_user_idx_temp);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user_destroy_timer(pst_hmac_user);

    /* 标记user资源已被alloc，这里申请multiuser为了不memzero user下面的timer,这里不memset user。挪到外面了 */
    pst_hmac_user->st_user_base_info.uc_is_user_alloced = MAC_USER_ALLOCED;

    *pus_user_idx = us_user_idx_temp;

    return OAL_SUCC;
}


oal_uint32 hmac_user_alloc(oal_uint16 *pus_user_idx)
{
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_uint32 ul_rslt;
    oal_uint16 us_user_idx_temp;

    if (oal_unlikely(pus_user_idx == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_UM, "{hmac_user_alloc::pus_user_idx null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请hmac user内存 */
    /*lint -e413*/
    ul_rslt = mac_res_alloc_hmac_user(&us_user_idx_temp, OAL_OFFSET_OF(hmac_user_stru, st_user_base_info));
    if (ul_rslt != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_UM, "{hmac_user_alloc::mac_res_alloc_hmac_user failed[%d].}", ul_rslt);
        return ul_rslt;
    }
    /*lint +e413*/
    pst_hmac_user = mac_res_get_hmac_user_alloc(us_user_idx_temp);
    if (pst_hmac_user == OAL_PTR_NULL) {
        mac_res_free_mac_user(us_user_idx_temp);
        OAM_ERROR_LOG1(0, OAM_SF_UM, "{hmac_user_alloc::pst_hmac_user null,user_idx=%d.}", us_user_idx_temp);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user_destroy_timer(pst_hmac_user);
    /* 初始清0 */
    memset_s(pst_hmac_user, OAL_SIZEOF(hmac_user_stru), 0, OAL_SIZEOF(hmac_user_stru));
    /* 标记user资源已被alloc */
    pst_hmac_user->st_user_base_info.uc_is_user_alloced = MAC_USER_ALLOCED;

    *pus_user_idx = us_user_idx_temp;

    return OAL_SUCC;
}


oal_uint32 hmac_user_free(oal_uint16 us_idx)
{
    hmac_user_stru *pst_hmac_user;
    oal_uint32 ul_ret;

    pst_hmac_user = mac_res_get_hmac_user(us_idx);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_UM, "{hmac_user_free::pst_hmac_user null,user_idx=%d.}", us_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = mac_res_free_mac_user(us_idx);
    if (ul_ret == OAL_SUCC) {
        /* 清除alloc标志 */
        pst_hmac_user->st_user_base_info.uc_is_user_alloced = MAC_USER_FREED;
    }

    return ul_ret;
}


oal_void hmac_user_init(hmac_user_stru *pst_hmac_user)
{
    oal_uint8 uc_tid_loop;
    hmac_ba_tx_stru *pst_tx_ba = OAL_PTR_NULL;

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    oal_uint8 uc_ac_idx;
    oal_uint8 uc_data_idx;
#endif

    /* 初始化tid信息 */
    for (uc_tid_loop = 0; uc_tid_loop < WLAN_TID_MAX_NUM; uc_tid_loop++) {
        pst_hmac_user->ast_tid_info[uc_tid_loop].uc_tid_no = (oal_uint8)uc_tid_loop;

        // pst_hmac_user->ast_tid_info[uc_tid_loop].pst_hmac_user  = (oal_void *)pst_hmac_user;
        pst_hmac_user->ast_tid_info[uc_tid_loop].us_hmac_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;

        /* 初始化ba rx操作句柄 */
        pst_hmac_user->ast_tid_info[uc_tid_loop].pst_ba_rx_info = OAL_PTR_NULL;

        /* 初始化ba tx操作句柄 */
        pst_hmac_user->ast_tid_info[uc_tid_loop].st_ba_tx_info.en_ba_status = DMAC_BA_INIT;
        pst_hmac_user->ast_tid_info[uc_tid_loop].st_ba_tx_info.uc_addba_attemps = 0;
        pst_hmac_user->ast_tid_info[uc_tid_loop].st_ba_tx_info.uc_dialog_token = 0;
        pst_hmac_user->ast_tid_info[uc_tid_loop].st_ba_tx_info.uc_ba_policy = 0;
        pst_hmac_user->ast_tid_info[uc_tid_loop].st_ba_tx_info.en_ba_switch = OAL_TRUE;
        pst_hmac_user->auc_ba_flag[uc_tid_loop] = 0;

        /* addba req超时处理函数入参填写 */
        pst_tx_ba = &pst_hmac_user->ast_tid_info[uc_tid_loop].st_ba_tx_info;
        pst_tx_ba->st_alarm_data.pst_ba = (oal_void *)pst_tx_ba;
        pst_tx_ba->st_alarm_data.uc_tid = uc_tid_loop;
        pst_tx_ba->st_alarm_data.us_mac_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;
        pst_tx_ba->st_alarm_data.uc_vap_id = pst_hmac_user->st_user_base_info.uc_vap_id;

        /* 初始化用户关联请求帧参数 */
        pst_hmac_user->puc_assoc_req_ie_buff = OAL_PTR_NULL;
        pst_hmac_user->ul_assoc_req_ie_len = 0;

        /* 初始化ba状态锁，防止删建BA时tid相关信息被重入 */
        oal_spin_lock_init(&(pst_hmac_user->ast_tid_info[uc_tid_loop].st_ba_tx_info.st_ba_status_lock));
    }

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    for (uc_ac_idx = 0; uc_ac_idx < WLAN_WME_AC_BUTT; uc_ac_idx++) {
        for (uc_data_idx = 0; uc_data_idx < WLAN_TXRX_DATA_BUTT; uc_data_idx++) {
            pst_hmac_user->aaul_txrx_data_stat[uc_ac_idx][uc_data_idx] = 0;
        }
    }
#endif

    pst_hmac_user->pst_defrag_netbuf = OAL_PTR_NULL;
    pst_hmac_user->en_user_bw_limit = OAL_FALSE;
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    pst_hmac_user->st_sa_query_info.ul_sa_query_count = 0;
    pst_hmac_user->st_sa_query_info.ul_sa_query_start_time = 0;
#endif
    memset_s(&pst_hmac_user->st_defrag_timer, OAL_SIZEOF(frw_timeout_stru), 0, OAL_SIZEOF(frw_timeout_stru));
    pst_hmac_user->ul_rx_pkt_drop = 0;

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    /* 清除usr统计信息 */
    oam_stats_clear_user_stat_info(pst_hmac_user->st_user_base_info.us_assoc_id);
#endif

    pst_hmac_user->ul_first_add_time = (oal_uint32)oal_time_get_stamp_ms();
    pst_hmac_user->us_clear_judge_count = 0;
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    pst_hmac_user->st_hmac_user_btcoex.st_hmac_btcoex_addba_req.en_ba_handle_allow = OAL_TRUE;
    pst_hmac_user->st_hmac_user_btcoex.en_delba_btcoex_trigger = OAL_FALSE;
    pst_hmac_user->st_hmac_user_btcoex.en_ba_type = BTCOEX_BA_TYPE_NORMAL;
#endif
    return;
}

oal_uint32 hmac_user_set_avail_num_space_stream(mac_user_stru *pst_mac_user, wlan_nss_enum_uint8 en_vap_nss)
{
    mac_user_ht_hdl_stru *pst_mac_ht_hdl;
    mac_vht_hdl_stru *pst_mac_vht_hdl;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    oal_uint8 uc_avail_num_spatial_stream = 0;
    oal_uint32 ul_ret = OAL_SUCC;
    mac_user_nss_stru st_user_nss;

    /* AP(STA)为legacy设备，只支持1根天线，不需要再判断天线个数 */
    /* 获取HT和VHT结构体指针 */
    pst_mac_ht_hdl = &(pst_mac_user->st_ht_hdl);
    pst_mac_vht_hdl = &(pst_mac_user->st_vht_hdl);

    if (pst_mac_vht_hdl->en_vht_capable == OAL_TRUE) {
        if (pst_mac_vht_hdl->st_rx_max_mcs_map.us_max_mcs_4ss != 3) { /* 3代表不支持 */
            uc_avail_num_spatial_stream = WLAN_FOUR_NSS;
        } else if (pst_mac_vht_hdl->st_rx_max_mcs_map.us_max_mcs_3ss != 3) { /* 3代表不支持 */
            uc_avail_num_spatial_stream = WLAN_TRIPLE_NSS;
        } else if (pst_mac_vht_hdl->st_rx_max_mcs_map.us_max_mcs_2ss != 3) { /* 3代表不支持 */
            uc_avail_num_spatial_stream = WLAN_DOUBLE_NSS;
        } else if (pst_mac_vht_hdl->st_rx_max_mcs_map.us_max_mcs_1ss != 3) { /* 3代表不支持 */
            uc_avail_num_spatial_stream = WLAN_SINGLE_NSS;
        } else {
            MAC_WARNING_LOG(0, "mac_user_set_avail_num_space_stream: get vht nss error");
            oam_warning_log0(pst_mac_user->uc_vap_id, OAM_SF_ANY,
                             "{mac_user_set_avail_num_space_stream::invalid vht nss.}");

            ul_ret = OAL_FAIL;
        }
    } else if (pst_mac_ht_hdl->en_ht_capable == OAL_TRUE) {
        if (pst_mac_ht_hdl->uc_rx_mcs_bitmask[3] > 0) { /* 接收mcs比特掩码第3字节<=0表示不支持 */
            uc_avail_num_spatial_stream = WLAN_FOUR_NSS;
        } else if (pst_mac_ht_hdl->uc_rx_mcs_bitmask[2] > 0) { /* 接收mcs比特掩码第2字节<=0表示不支持 */
            uc_avail_num_spatial_stream = WLAN_TRIPLE_NSS;
        } else if (pst_mac_ht_hdl->uc_rx_mcs_bitmask[1] > 0) {
            uc_avail_num_spatial_stream = WLAN_DOUBLE_NSS;
        } else if (pst_mac_ht_hdl->uc_rx_mcs_bitmask[0] > 0) {
            uc_avail_num_spatial_stream = WLAN_SINGLE_NSS;
        } else {
            MAC_WARNING_LOG(0, "mac_user_set_avail_num_space_stream: get ht nss error");
            oam_warning_log0(pst_mac_user->uc_vap_id, OAM_SF_ANY,
                             "{mac_user_set_avail_num_space_stream::invalid ht nss.}");

            ul_ret = OAL_FAIL;
        }
    } else {
        uc_avail_num_spatial_stream = WLAN_SINGLE_NSS;
    }

    /* 赋值给用户结构体变量 */
    mac_user_set_num_spatial_stream(pst_mac_user, uc_avail_num_spatial_stream);
    mac_user_set_avail_num_spatial_stream(pst_mac_user, oal_min(uc_avail_num_spatial_stream, en_vap_nss));

    /* 空间流信息同步dmac */
    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_user->uc_vap_id);
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_mac_user->uc_vap_id, OAM_SF_CFG,
                       "hmac_user_set_avail_num_space_stream::mac vap(idx=%d) is null!", pst_mac_user->uc_vap_id);
        return ul_ret;
    }
    st_user_nss.uc_avail_num_spatial_stream = pst_mac_user->uc_avail_num_spatial_stream;
    st_user_nss.uc_num_spatial_stream = pst_mac_user->uc_num_spatial_stream;
    st_user_nss.us_user_idx = pst_mac_user->us_assoc_id;

    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_NSS,
                                    OAL_SIZEOF(mac_user_nss_stru),
                                    (oal_uint8 *)(&st_user_nss));
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_user->uc_vap_id, OAM_SF_CFG,
                         "{hmac_user_set_avail_num_space_stream::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}

#ifdef _PRE_WLAN_FEATURE_WAPI
hmac_wapi_stru *hmac_user_get_wapi_ptr(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_pairwise,
                                       oal_uint16 us_pairwise_idx)
{
    hmac_user_stru *pst_hmac_user;
    // oal_uint32                       ul_ret;
    oal_uint16 us_user_index;

    if (en_pairwise == OAL_TRUE) {
        us_user_index = us_pairwise_idx;
    } else {
        us_user_index = pst_mac_vap->us_multi_user_idx;
    }

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(us_user_index);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_user_get_wapi_ptr::pst_hmac_user[%d] null.}",
                         us_user_index);
        return OAL_PTR_NULL;
    }

    return &pst_hmac_user->st_wapi;
}
#endif

OAL_STATIC oal_void hmac_txbf_ability_set(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_txbf_enable)
{
    pst_mac_vap->pst_mib_info->st_wlan_mib_txbf_config.en_dot11ReceiveStaggerSoundingOptionImplemented =
        en_txbf_enable;
    pst_mac_vap->pst_mib_info->st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformeeOptionImplemented =
        en_txbf_enable;
    pst_mac_vap->pst_mib_info->st_wlan_mib_vht_txbf_config.en_dot11VHTMUBeamformeeOptionImplemented =
        en_txbf_enable;
    pst_mac_vap->pst_mib_info->st_wlan_mib_vht_txbf_config.ul_dot11VHTBeamformeeNTxSupport =
        en_txbf_enable;

    pst_mac_vap->en_txbf_enable = en_txbf_enable;
}


OAL_STATIC oal_uint32 hmac_txbf_ability_sync(mac_vap_stru *pst_mac_vap, oal_uint16 us_user_idx)
{
    mac_h2d_txbf_ability_stru st_txbf_ability;
    oal_uint32 ul_ret;

    st_txbf_ability.us_user_idx = us_user_idx;
    st_txbf_ability.en_txbf_enable = pst_mac_vap->en_txbf_enable;

    /***************************************************************************
        抛事件到DMAC层, 同步txbf能力到device侧
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_TXBF_ABILITY, OAL_SIZEOF(mac_h2d_txbf_ability_stru),
                                    (oal_uint8 *)&st_txbf_ability);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_send_dscr_th_update_event::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


OAL_STATIC oal_void hmac_txbf_ability_enable(mac_vap_stru *pst_mac_vap, mac_bss_dscr_stru *pst_bss_dscr)
{
    oal_bool_enum_uint8 en_txbf_enable = (pst_mac_vap->st_channel.en_band == WLAN_BAND_5G) &&
                                         ((pst_bss_dscr->en_support_max_nss < WLAN_TRIPLE_NSS) ||
                                          (pst_bss_dscr->uc_num_sounding_dim < WLAN_TRIPLE_NSS));

    if (pst_mac_vap->en_host_txbf_mode != COMPABILITY_TXBF) {
        en_txbf_enable = (pst_mac_vap->en_host_txbf_mode) > 0 ? ENABLE_TXBF : NO_TXBF;
    }

    hmac_txbf_ability_set(pst_mac_vap, en_txbf_enable);

    oam_warning_log4(0, OAM_SF_AUTH,
                     "hmac_txbf_ability_enable:enable txbfee[%d], band[%d], max_nss[%d], max_sounding_dim[%d]",
                     en_txbf_enable, pst_mac_vap->st_channel.en_band, pst_bss_dscr->en_support_max_nss,
                     pst_bss_dscr->uc_num_sounding_dim);
}

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU

oal_void hmac_amsdu_ampdu_compability_enable(mac_vap_stru *pst_mac_vap, mac_bss_dscr_stru *pst_bss_dscr)
{
    mac_tx_large_amsdu_ampdu_stru *tx_large_amsdu = mac_get_tx_large_amsdu_addr();
    if (IS_LEGACY_VAP(pst_mac_vap)) {
        /* 识别联合聚合有兼容性问题的AP */
        tx_large_amsdu->uc_compability_en = OAL_FALSE;
        if (MAC_IS_TPLINK_847N(pst_bss_dscr)) {
            tx_large_amsdu->uc_compability_en = OAL_TRUE;
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                             "hmac_compability_ap_type_identify: tx amsdu+ampdu compability !");
        }
    }
}
#endif

mac_ap_type_enum_uint8 hmac_ddc_compability_enable(mac_vap_stru *pst_mac_vap, mac_bss_dscr_stru *pst_bss_dscr)
{
    if (IS_LEGACY_VAP(pst_mac_vap)) {
        /* 识别需要加入ddc白名单的有兼容性问题的AP */
        if ((((MAC_IS_TPLINK_890N(pst_bss_dscr) || MAC_IS_TPLINK_880N(pst_bss_dscr) ||
               MAC_IS_TPLINK_2041N(pst_bss_dscr)) &&
              (pst_bss_dscr->en_support_max_nss == WLAN_TRIPLE_NSS)) ||
             (MAC_IS_TPLINK_H28R(pst_bss_dscr) && (pst_bss_dscr->en_support_max_nss == WLAN_DOUBLE_NSS))) &&
            (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                             "hmac_compability_ap_type_identify: ddc white list compability !");
            return MAC_AP_TYPE_DDC_WHITELIST;
        }
    }
    return 0;
}

oal_void hmac_judge_enable_fft_window_offset(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_device,
                                             oal_bool_enum_uint8 en_set_compability_offset)
{
    if (en_set_compability_offset != pst_mac_device->en_fft_window_offset_enable) {
        pst_mac_device->en_fft_window_offset_enable = en_set_compability_offset;
        hmac_config_fft_window_offset(pst_mac_vap, en_set_compability_offset);
    }
}

oal_void static hmac_fft_window_compability_enable(mac_vap_stru *pst_mac_vap,
                                                   mac_ap_type_enum_uint8 en_ap_type)
{
    oal_bool_enum_uint8 en_set_compability_offset = OAL_FALSE;
    mac_device_stru *pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "hmac_fft_window_compability_enable: get mac device fail!");
        return;
    }

    /* R7800在2G 40M有兼容性问题，需要配置fft window offset 为3 */
    if (IS_LEGACY_VAP(pst_mac_vap)
        && (en_ap_type & MAC_AP_TYPE_R7800)
        && ((pst_mac_vap->st_channel).en_band == WLAN_BAND_2G)
        && ((((pst_mac_vap->st_channel).en_bandwidth) == WLAN_BAND_WIDTH_40PLUS) ||
                  (((pst_mac_vap->st_channel).en_bandwidth) == WLAN_BAND_WIDTH_40MINUS))
        && (mac_device_calc_up_vap_num(pst_mac_device) == 0)) {
        en_set_compability_offset = OAL_TRUE;
    }

    hmac_judge_enable_fft_window_offset(pst_mac_vap, pst_mac_device, en_set_compability_offset);
}
mac_ap_type_enum_uint8 hmac_bt_only_on_compability_enable(mac_vap_stru *pst_mac_vap, mac_bss_dscr_stru *pst_bss_dscr)
{
    if (IS_LEGACY_VAP(pst_mac_vap)) {
        /* 识别需要加入bt仅打开黑名单的有兼容性问题的AP */
        if (pst_bss_dscr->en_is_realtek_chip_oui == OAL_TRUE) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                             "hmac_bt_only_on_compability_enable: bt only on black list compability !");
            return MAC_AP_TYPE_BT_ONLY_ON;
        }
    }
    return 0;
}


mac_ap_type_enum_uint8 hmac_compability_ap_tpye_identify(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_mac_addr)
{
    mac_bss_dscr_stru *pst_bss_dscr = OAL_PTR_NULL;
    mac_ap_type_enum_uint8 en_ap_type = 0;

    if (MAC_IS_GOLDEN_AP(puc_mac_addr)) {
        en_ap_type |= MAC_AP_TYPE_GOLDENAP;
    }

    pst_bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(pst_mac_vap, puc_mac_addr);
    if (pst_bss_dscr != OAL_PTR_NULL) {
#ifdef _PRE_WLAN_FEATURE_BTCOEX
        hmac_btcoex_ap_tpye_identify(pst_mac_vap, puc_mac_addr, pst_bss_dscr, &en_ap_type);
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
        /* 关闭时候识别斐讯k3，不能进行漫游 */
        if (MAC_IS_FEIXUN_K3(puc_mac_addr)) {
            if (pst_bss_dscr->en_roam_blacklist_chip_oui == OAL_TRUE) {
                oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ROAM,
                                 "hmac_compability_ap_tpye_identify:roam blacklist!");
                en_ap_type |= MAC_AP_TYPE_ROAM;
            }
        }
#endif

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        /* 联合聚合兼容性AP识别 */
        hmac_amsdu_ampdu_compability_enable(pst_mac_vap, pst_bss_dscr);
#endif
        /* 兼容性FFT窗口配置 */
        if (MAC_IS_NETGEAR_R7800(pst_bss_dscr)) {
            en_ap_type |= MAC_AP_TYPE_R7800;
        }
        /* 规避txbf的负收益方案 */
        hmac_txbf_ability_enable(pst_mac_vap, pst_bss_dscr);
        en_ap_type |= hmac_ddc_compability_enable(pst_mac_vap, pst_bss_dscr);
        en_ap_type |= hmac_bt_only_on_compability_enable(pst_mac_vap, pst_bss_dscr);
    }

    return en_ap_type;
}


oal_uint32 hmac_user_del(mac_vap_stru *pst_mac_vap, hmac_user_stru *pst_hmac_user)
{
    oal_uint16 us_user_index;
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    dmac_ctx_del_user_stru *pst_del_user_payload = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    mac_user_stru *pst_mac_user = OAL_PTR_NULL;
    oal_uint32 ul_ret;
#ifdef _PRE_WLAN_FEATURE_WMMAC
    oal_uint8 uc_ac_index = 0;
#endif
#ifdef _PRE_WLAN_FEATURE_WAPI
    hmac_user_stru *pst_hmac_user_multi;
#endif
    mac_ap_type_enum_uint8 en_ap_type = 0;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    oal_int8 pc_param[18] = { 0 };
    oal_int8 pc_tmp[8] = { 0 };
    oal_uint16 us_len;
#endif

    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (pst_hmac_user == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_UM, "{hmac_user_del::null,%x %x.}", (uintptr_t)pst_mac_vap, (uintptr_t)pst_hmac_user);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_user = (mac_user_stru *)(&pst_hmac_user->st_user_base_info);

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_del::del user[%d] start,is multi user[%d], \
        user mac:XX:XX:XX:XX:%02X:%02X}", pst_mac_user->us_assoc_id, pst_mac_user->en_is_multi_user,
        pst_mac_user->auc_user_mac_addr[4], pst_mac_user->auc_user_mac_addr[5]); /* auc_user_mac_addr第4、5字节输出打印 */
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* 清理arp探测timer */
    if (pst_hmac_user->st_hmac_user_btcoex.st_hmac_btcoex_arp_req_process.st_delba_opt_timer.en_is_registerd ==
        OAL_TRUE) {
        frw_immediate_destroy_timer(&(pst_hmac_user->st_hmac_user_btcoex.st_hmac_btcoex_arp_req_process.
                                            st_delba_opt_timer));
    }
#endif

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    hmac_chan_update_40m_intol_user(pst_mac_vap, &(pst_hmac_user->st_user_base_info), OAL_FALSE);
#endif
    /* 删除user时候，需要更新保护机制 */
    ul_ret = hmac_protection_del_user(pst_mac_vap, &(pst_hmac_user->st_user_base_info));
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_UM, "{hmac_user_del::hmac_protection_del_user[%d]}", ul_ret);
    }

    /* 获取用户对应的索引 */
    us_user_index = pst_hmac_user->st_user_base_info.us_assoc_id;

    /* 删除hmac user 的关联请求帧空间 */
    if (pst_hmac_user->puc_assoc_req_ie_buff != OAL_PTR_NULL) {
        oal_mem_free_m(pst_hmac_user->puc_assoc_req_ie_buff, OAL_TRUE);
        pst_hmac_user->puc_assoc_req_ie_buff = OAL_PTR_NULL;
        pst_hmac_user->ul_assoc_req_ie_len = 0;
    }
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    hmac_stop_sa_query_timer(pst_hmac_user);
#endif
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_del::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_hmac_vap->en_web_fail_roam = OAL_FALSE;
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_del::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_VOWIFI
    if (pst_hmac_vap->st_vap_base_info.pst_vowifi_cfg_param != OAL_PTR_NULL) {
        if (pst_hmac_vap->st_vap_base_info.pst_vowifi_cfg_param->en_vowifi_mode == VOWIFI_LOW_THRES_REPORT) {
            /* 针对漫游和去关联场景,切换vowifi语音状态 */
            hmac_config_vowifi_report((&pst_hmac_vap->st_vap_base_info), 0, OAL_PTR_NULL);
        }
    }
#endif /* _PRE_WLAN_FEATURE_VOWIFI */

#ifdef _PRE_WLAN_FEATURE_WAPI
    hmac_wapi_deinit(&pst_hmac_user->st_wapi);

    /* STA模式下，清组播wapi加密端口 */
    pst_hmac_user_multi = (hmac_user_stru *)mac_res_get_hmac_user(pst_hmac_vap->st_vap_base_info.us_multi_user_idx);
    if (pst_hmac_user_multi == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{hmac_user_del:get_user fail, id[%u]}",
            pst_hmac_vap->st_vap_base_info.us_multi_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_wapi_reset_port(&pst_hmac_user_multi->st_wapi);
    pst_mac_device->uc_wapi = OAL_FALSE;
#endif
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_device_set_random_mac_for_scan(pst_mac_device, pst_mac_vap);

#ifdef _PRE_WLAN_FEATURE_STA_PM
        mac_vap_set_aid(pst_mac_vap, 0);
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
        /* 避免用户先删除，后接收到删除IP地址，device ip_addr_obtained标识未被清除，一直允许漫游 */
        hmac_roam_wpas_connect_state_notify(pst_hmac_vap, WPAS_CONNECT_STATE_IPADDR_REMOVED);
        hmac_roam_exit(pst_hmac_vap);
        hmac_roam_ping_pong_clear(pst_hmac_vap);
#endif  // _PRE_WLAN_FEATURE_ROAM

        en_ap_type = hmac_compability_ap_tpye_identify(pst_mac_vap, pst_mac_user->auc_user_mac_addr);
        mac_vap_disable_amsdu_ampdu(pst_mac_vap);
    }

    /* delete的时候再做一次fft window配置 */
    hmac_fft_window_compability_enable(pst_mac_vap, en_ap_type);
    /***************************************************************************
        抛事件到DMAC层, 删除dmac用户
    ***************************************************************************/
    pst_event_mem = frw_event_alloc_m(OAL_SIZEOF(dmac_ctx_del_user_stru));
    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_del::pst_event_mem null.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;
    pst_del_user_payload = (dmac_ctx_del_user_stru *)pst_event->auc_event_data;
    pst_del_user_payload->us_user_idx = us_user_index;
    pst_del_user_payload->en_ap_type = en_ap_type;
#if (_PRE_OS_VERSION_WIN32 != _PRE_OS_VERSION)
    /* 用户 mac地址和idx 需至少一份有效，供dmac侧查找待删除的用户 */
    memcpy_s(pst_del_user_payload->auc_user_mac_addr, WLAN_MAC_ADDR_LEN,
        pst_mac_user->auc_user_mac_addr, WLAN_MAC_ADDR_LEN);
#endif

    /* 填充事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_DEL_USER,
        OAL_SIZEOF(dmac_ctx_del_user_stru), FRW_EVENT_PIPELINE_STAGE_1,
        pst_mac_vap->uc_chip_id, pst_mac_vap->uc_device_id, pst_mac_vap->uc_vap_id);

    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        /* 做维测，如果删除用户失败，前面清hmac资源的操作本身已经异常，需要定位 */
        frw_event_free_m(pst_event_mem);
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_del::dispatch_event failed[%d].}", ul_ret);
        return ul_ret;
    }
    frw_event_free_m(pst_event_mem);
#ifdef _PRE_WLAN_FEATURE_WMMAC
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    /* 删除user时删除发送addts req超时定时器 */
    for (uc_ac_index = 0; uc_ac_index < WLAN_WME_AC_BUTT; uc_ac_index++) {
        if (pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_index].st_addts_timer.en_is_registerd == OAL_TRUE) {
            frw_immediate_destroy_timer(&(pst_mac_user->st_ts_info[uc_ac_index].st_addts_timer));
        }
        memset_s(&(pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_index]), OAL_SIZEOF(mac_ts_stru), 0,
                 OAL_SIZEOF(mac_ts_stru));
        pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_index].uc_tsid = 0xFF;
    }
#endif
#endif

    hmac_tid_clear(pst_mac_vap, pst_hmac_user);

    if (pst_hmac_user->st_mgmt_timer.en_is_registerd == OAL_TRUE) {
        frw_immediate_destroy_timer(&pst_hmac_user->st_mgmt_timer);
    }

    hmac_user_clear_defrag_res(pst_hmac_user);

    /* 从vap中删除用户 */
    mac_vap_del_user(pst_mac_vap, us_user_index);
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if (pst_mac_vap->us_user_nums == 5) { /* AP用户达到5时，调整流控参数为配置文件原有值 */
        /* 整形转字符串函数中字符串长度为8 */
        oal_itoa(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_USED_MEM_FOR_START), pc_param, 8);
        /* 整形转字符串函数中字符串长度为8 */
        oal_itoa(hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_USED_MEM_FOR_STOP), pc_tmp, 8);
        pc_param[OAL_STRLEN(pc_param)] = ' ';
        if (memcpy_s(pc_param + OAL_STRLEN(pc_param), OAL_SIZEOF(pc_param) - OAL_STRLEN(pc_param),
            pc_tmp, OAL_STRLEN(pc_tmp)) != EOK) {
            OAM_ERROR_LOG0(0, OAM_SF_UM, "hmac_user_del_etc::memcpy fail!");
        }

        us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);
        hmac_config_sdio_flowctrl(pst_mac_vap, us_len, pc_param);
    }
#endif
    /* 释放用户内存 */
    ul_ret = hmac_user_free(us_user_index);
    if (ul_ret == OAL_SUCC) {
        /* device下已关联user个数-- */
        pst_mac_device->uc_asoc_user_cnt--;
    } else {
        OAM_ERROR_LOG1(0, OAM_SF_UM, "{hmac_user_del::mac_res_free_mac_user fail[%d].}", ul_ret);
    }

    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
    }

#ifdef _PRE_WLAN_FEATURE_ROAM
    pst_hmac_vap->en_roam_prohibit_on = OAL_FALSE;
#endif

    return OAL_SUCC;
}


oal_void hmac_set_roam_prohibit_on(hmac_vap_stru *pst_hmac_vap, mac_ap_type_enum_uint8 en_ap_type)
{
#ifdef _PRE_WLAN_FEATURE_ROAM
    if (en_ap_type & MAC_AP_TYPE_ROAM) {
        pst_hmac_vap->en_roam_prohibit_on = OAL_TRUE;
    } else {
        pst_hmac_vap->en_roam_prohibit_on = OAL_FALSE;
    }
#endif
}


oal_uint32 hmac_sta_user_num_check(mac_vap_stru *pst_mac_vap)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    if (IS_P2P_CL(pst_mac_vap)) {
        if (pst_mac_vap->us_user_nums >= 2) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_UM,
                             "{hmac_sta_user_num_check::a STA can only associated with 2 ap.}");
            return OAL_FAIL;
        }
    } else
#endif
    {
        if (pst_mac_vap->us_user_nums >= 1) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_UM,
                             "{hmac_sta_user_num_check::a STA can only associated with one ap.}");
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

oal_void hmac_adjust_sdio_flowctrl(mac_vap_stru *pst_mac_vap)
{
    oal_uint16 us_len;
    oal_int8 pc_param[] = "30 25";

    if (pst_mac_vap->us_user_nums == 6) { /* AP用户达到6时，调整流控参数为Stop为25，Start为30 */
        us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);
        hmac_config_sdio_flowctrl(pst_mac_vap, us_len, pc_param);
    }
}
#endif


oal_uint32 hmac_user_add(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_mac_addr, oal_uint16 *pus_user_index)
{
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    dmac_ctx_add_user_stru *pst_add_user_payload = OAL_PTR_NULL;
    oal_uint16 us_user_idx;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    mac_cfg_80211_ucast_switch_stru st_80211_ucast_switch;
    mac_ap_type_enum_uint8 en_ap_type = 0;

    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_mac_addr == OAL_PTR_NULL) ||
                     (pus_user_index == OAL_PTR_NULL))) {
        oam_error_log3(0, OAM_SF_UM, "{hmac_user_add::param null, %x %x %x.}", (uintptr_t)pst_mac_vap,
                       (uintptr_t)puc_mac_addr, (uintptr_t)pus_user_index);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_UM, "{hmac_user_add::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_vap_clean_rx_rate(pst_hmac_vap);
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 在HMAC处作最大活跃用户数判断，已达32则返回错误 */
    if (pst_mac_device->uc_active_user_cnt >= WLAN_ACTIVE_USER_MAX_NUM) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add::invalid uc_active_user_cnt[%d].}",
                         pst_mac_device->uc_active_user_cnt);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    if (pst_hmac_vap->st_vap_base_info.us_user_nums >= pst_hmac_vap->us_user_nums_max) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_UM,
                         "{hmac_user_add::invalid us_user_nums[%d], us_user_nums_max[%d].}",
                         pst_hmac_vap->st_vap_base_info.us_user_nums, pst_hmac_vap->us_user_nums_max);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    /* 如果此用户已经创建，则返回失败 */
    ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap, puc_mac_addr, &us_user_idx);
    if (ul_ret == OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add::mac_vap_find_user_by_macaddr failed[%d].}",
                         ul_ret);
        return OAL_FAIL;
    }

    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        ul_ret = hmac_sta_user_num_check(pst_mac_vap);
        if (ul_ret != OAL_SUCC) {
            return ul_ret;
        }
        if (pst_mac_vap->en_p2p_mode == WLAN_LEGACY_VAP_MODE) {
            en_ap_type = hmac_compability_ap_tpye_identify(pst_mac_vap, puc_mac_addr);
        }
#ifdef _PRE_WLAN_FEATURE_ROAM
        hmac_roam_init(pst_hmac_vap);
#endif  // _PRE_WLAN_FEATURE_ROAM
    }

    /* 申请hmac用户内存，并初始清0 */
    ul_ret = hmac_user_alloc(&us_user_idx);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add::hmac_user_alloc failed[%d].}", ul_ret);
        return ul_ret;
    }

    /* 单播用户不能使用userid为0，需重新申请一个。将userid作为aid分配给对端，处理psm时会出错 */
    if (us_user_idx == 0) {
        hmac_user_free(us_user_idx);
        ul_ret = hmac_user_alloc(&us_user_idx);
        if ((ul_ret != OAL_SUCC) || (us_user_idx == 0)) {
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_UM,
                             "{hmac_user_add::hmac_user_alloc failed ret[%d] us_user_idx[%d].}", ul_ret, us_user_idx);
            return ul_ret;
        }
    }

    *pus_user_index = us_user_idx; /* 出参赋值 */

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(us_user_idx);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add::pst_hmac_user[%d] null.}", us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 初始化mac_user_stru */
    mac_user_init(&(pst_hmac_user->st_user_base_info), us_user_idx, puc_mac_addr, pst_mac_vap->uc_chip_id,
                  pst_mac_vap->uc_device_id, pst_mac_vap->uc_vap_id);

#ifdef _PRE_WLAN_FEATURE_WAPI
    /* 初始化单播wapi对象 */
    hmac_wapi_init(&pst_hmac_user->st_wapi, OAL_TRUE);
    pst_mac_device->uc_wapi = OAL_FALSE;
#endif
    /* 设置amsdu域 */
    hmac_amsdu_init_user(pst_hmac_user);

    /***************************************************************************
        抛事件到DMAC层, 创建dmac用户
    ***************************************************************************/
    pst_event_mem = frw_event_alloc_m(OAL_SIZEOF(dmac_ctx_add_user_stru));
    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        /* 异常处理，释放内存，device下关联用户数还没有++，这里不需要判断返回值做--操作 */
        hmac_user_free(us_user_idx);

        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add::pst_event_mem null.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;
    pst_add_user_payload = (dmac_ctx_add_user_stru *)pst_event->auc_event_data;
    pst_add_user_payload->us_user_idx = us_user_idx;
    pst_add_user_payload->en_ap_type = en_ap_type;
    oal_set_mac_addr(pst_add_user_payload->auc_user_mac_addr, puc_mac_addr);

    /* 填充事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_ADD_USER,
                       OAL_SIZEOF(dmac_ctx_add_user_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id, pst_mac_vap->uc_device_id, pst_mac_vap->uc_vap_id);

    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        /* 异常处理，释放内存，device下关联用户数还没有++，这里不需要判断返回值做--操作 */
        hmac_user_free(us_user_idx);
        frw_event_free_m(pst_event_mem);
        /* 不应该出现用户添加失败，失败需要定位具体原因 */
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add::frw_event_dispatch_event fail[%d]}", ul_ret);
        return ul_ret;
    }

    frw_event_free_m(pst_event_mem);

    /* 添加用户到MAC VAP */
    ul_ret = mac_vap_add_assoc_user(pst_mac_vap, us_user_idx);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add::mac_vap_add_assoc_user fail[%d]}", ul_ret);

        /* 异常处理，释放内存，device下关联用户数还没有++，这里不需要判断返回值做--操作 */
        hmac_user_free(us_user_idx);
        return OAL_FAIL;
    }

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hmac_adjust_sdio_flowctrl(pst_mac_vap);
#endif

    /* 初始话hmac user部分信息 */
    hmac_user_init(pst_hmac_user);
    pst_mac_device->uc_asoc_user_cnt++;

    /* 打开80211单播管理帧开关，同03修改,关联成功也保存打开,方便问题定位,如BAR相关问题 */
    st_80211_ucast_switch.en_frame_direction = OAM_OTA_FRAME_DIRECTION_TYPE_TX;
    st_80211_ucast_switch.en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
    st_80211_ucast_switch.en_frame_switch = OAL_SWITCH_ON;
    st_80211_ucast_switch.en_cb_switch = OAL_SWITCH_ON;
    st_80211_ucast_switch.en_dscr_switch = OAL_SWITCH_ON;
    if (memcpy_s(st_80211_ucast_switch.auc_user_macaddr, OAL_SIZEOF(st_80211_ucast_switch.auc_user_macaddr),
        (const oal_void *)puc_mac_addr, OAL_SIZEOF(st_80211_ucast_switch.auc_user_macaddr)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_UM, "hmac_user_add::memcpy fail!");
        hmac_user_free(us_user_idx);
        return OAL_FAIL;
    }
    hmac_config_80211_ucast_switch(pst_mac_vap, OAL_SIZEOF(st_80211_ucast_switch), (oal_uint8 *)&st_80211_ucast_switch);

    st_80211_ucast_switch.en_frame_direction = OAM_OTA_FRAME_DIRECTION_TYPE_RX;
    st_80211_ucast_switch.en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
    st_80211_ucast_switch.en_frame_switch = OAL_SWITCH_ON;
    st_80211_ucast_switch.en_cb_switch = OAL_SWITCH_ON;
    st_80211_ucast_switch.en_dscr_switch = OAL_SWITCH_ON;
    hmac_config_80211_ucast_switch(pst_mac_vap, OAL_SIZEOF(st_80211_ucast_switch), (oal_uint8 *)&st_80211_ucast_switch);

    hmac_set_roam_prohibit_on(pst_hmac_vap, en_ap_type);

    /* 兼容性FFT窗口配置 */
    hmac_fft_window_compability_enable(pst_mac_vap, en_ap_type);
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    hmac_btcoex_process_exception_ap(pst_mac_vap, &(pst_hmac_user->st_user_base_info), en_ap_type);
#endif
    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add::user[%d] mac:%02X:XX:XX:XX:%02X:%02X}",
                     us_user_idx, puc_mac_addr[0], puc_mac_addr[4], puc_mac_addr[5]); /* 0/4/5为对应的索引 */

    return OAL_SUCC;
}


oal_uint32 hmac_config_add_user(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_add_user_param_stru *pst_add_user;
    oal_uint16 us_user_index;
    hmac_vap_stru *pst_hmac_vap;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    mac_user_ht_hdl_stru st_ht_hdl;
    oal_uint32 ul_rslt;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;

    pst_add_user = (mac_cfg_add_user_param_stru *)puc_param;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_UM, "{hmac_config_add_user::pst_hmac_vap null.}");
        return OAL_FAIL;
    }

    ul_ret = hmac_user_add(pst_mac_vap, pst_add_user->auc_mac_addr, &us_user_index);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_add_user::hmac_user_add failed.}", ul_ret);
        return ul_ret;
    }

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(us_user_index);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_add_user::pst_hmac_user[%d] null.}",
                         us_user_index);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置qos域，后续如有需要可以通过配置命令参数配置 */
    mac_user_set_qos(&pst_hmac_user->st_user_base_info, OAL_TRUE);

    /* 设置HT域 */
    mac_user_get_ht_hdl(&pst_hmac_user->st_user_base_info, &st_ht_hdl);
    st_ht_hdl.en_ht_capable = pst_add_user->en_ht_cap;

    if (pst_add_user->en_ht_cap == OAL_TRUE) {
        pst_hmac_user->st_user_base_info.en_cur_protocol_mode = WLAN_HT_MODE;
        pst_hmac_user->st_user_base_info.en_avail_protocol_mode = WLAN_HT_MODE;
    }

    /* 设置HT相关的信息:应该在关联的时候赋值 这个值配置的合理性有待考究 2012->page:786 */
    st_ht_hdl.uc_min_mpdu_start_spacing = 6;
    st_ht_hdl.uc_max_rx_ampdu_factor = 3; /* 最大接收聚合因子是3 */
    mac_user_set_ht_hdl(&pst_hmac_user->st_user_base_info, &st_ht_hdl);

    mac_user_set_asoc_state(&pst_hmac_user->st_user_base_info, MAC_USER_STATE_ASSOC);

    /* 设置amsdu域 */
    hmac_amsdu_init_user(pst_hmac_user);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    /* 重新设置DMAC需要的参数 */
    pst_add_user->us_user_idx = us_user_index;

    ul_ret = hmac_config_send_event(&pst_hmac_vap->st_vap_base_info,
                                    WLAN_CFGID_ADD_USER,
                                    us_len,
                                    puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        /* 异常处理，释放内存 */
        ul_rslt = hmac_user_free(us_user_index);
        if (ul_rslt == OAL_SUCC) {
            pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
            if (pst_mac_device == OAL_PTR_NULL) {
                oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_add_user::pst_mac_device null.}");
                return OAL_ERR_CODE_PTR_NULL;
            }

            /* hmac_add_user成功时device下关联用户数已经++, 这里的device下已关联user个数要-- */
            pst_mac_device->uc_asoc_user_cnt--;
        }

        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_add_user::hmac_config_send_event failed[%d]}",
                         ul_ret);
        return ul_ret;
    }

    /* 打桩添加用户信息不全，不需要通知算法 */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (IS_LEGACY_VAP(pst_mac_vap)) {
        mac_vap_state_change(pst_mac_vap, MAC_VAP_STATE_UP);
    }
#endif

    return OAL_SUCC;
}


oal_uint32 hmac_config_del_user(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_del_user_param_stru *pst_del_user;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap;
    oal_uint16 us_user_index;
    oal_uint32 ul_ret;

    pst_del_user = (mac_cfg_add_user_param_stru *)puc_param;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_del_user::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取用户对应的索引 */
    ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap, pst_del_user->auc_mac_addr, &us_user_index);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM,
                         "{hmac_config_del_user::mac_vap_find_user_by_macaddr failed[%d].}", ul_ret);
        return ul_ret;
    }

    /* 获取hmac用户 */
    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(us_user_index);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_del_user::pst_hmac_user[%d] null.}",
                         us_user_index);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_user_del(pst_mac_vap, pst_hmac_user);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_del_user::hmac_user_del failed[%d]}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_void hmac_set_multi_user_allocated(oal_uint16 us_user_index)
{
    hmac_user_stru *pst_hmac_user;
    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(us_user_index);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_set_multi_user_allocated::get hmac_user[%d] fail.}", us_user_index);
        return;
        ;
    }
    /* 初始清0 */
    memset_s(pst_hmac_user, OAL_SIZEOF(hmac_user_stru), 0, OAL_SIZEOF(hmac_user_stru));
    /* 标记user资源已被alloc */
    pst_hmac_user->st_user_base_info.uc_is_user_alloced = MAC_USER_ALLOCED;
}

oal_uint32 hmac_user_add_multi_user(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_user_index)
{
    oal_uint32 ul_ret;
    oal_uint16 us_user_index = 0;
    mac_user_stru *pst_mac_user;
#ifdef _PRE_WLAN_FEATURE_WAPI
    hmac_user_stru *pst_hmac_user;
#endif
    oal_uint8 uc_loop;

    /* 首先判断index=0是否有被使用,如果没有则优先使用index=0 */
    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(0);
    if (pst_mac_user == OAL_PTR_NULL) {
        for (uc_loop = 0; uc_loop < MAC_RES_MAX_USER_NUM; uc_loop++) {
            ul_ret = hmac_user_alloc_multi_user(&us_user_index);
            if (ul_ret != OAL_SUCC) {
                OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_multi_user::user_alloc failed[%d].}",
                               ul_ret);
                return ul_ret;
            }
            if (us_user_index != 0) {
                hmac_user_free(us_user_index);
            } else {
                /* 由于没有在hmac_user_alloc_multi_user memset user，在这里memset user. */
                hmac_set_multi_user_allocated(us_user_index);
                break;
            }
        }
        if (uc_loop == MAC_RES_MAX_USER_NUM) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "hmac_user_add_multi_user:cannot alloc index=0");
            return OAL_ERR_CODE_PTR_NULL;
        }
    } else {
        ul_ret = hmac_user_alloc(&us_user_index);
        if (ul_ret != OAL_SUCC) {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_multi_user::hmac_user_alloc failed[%d].}",
                           ul_ret);
            return ul_ret;
        }
    }
    /* 初始化组播用户基本信息 */
    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(us_user_index);
    if (pst_mac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_user_add_multi_user::pst_mac_user[%d] null.}",
                         us_user_index);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_user_init(pst_mac_user, us_user_index, OAL_PTR_NULL, pst_mac_vap->uc_chip_id, pst_mac_vap->uc_device_id,
                  pst_mac_vap->uc_vap_id);

    *pus_user_index = us_user_index;

#ifdef _PRE_WLAN_FEATURE_WAPI
    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(us_user_index);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_user_add_multi_user::get hmac_user[%d] fail.}",
                       us_user_index);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 初始化wapi对象 */
    hmac_wapi_init(&pst_hmac_user->st_wapi, OAL_FALSE);
#endif

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_user_add_multi_user, user index[%d].}", us_user_index);

    return OAL_SUCC;
}


oal_uint32 hmac_user_del_multi_user(mac_vap_stru *pst_mac_vap)
{
    oal_uint32 ul_ret;
#ifdef _PRE_WLAN_FEATURE_WAPI
    hmac_user_stru *pst_hmac_user;
#endif

#ifdef _PRE_WLAN_FEATURE_WAPI
    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(pst_mac_vap->us_multi_user_idx);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_user_add_multi_user::get hmac_user fail.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_wapi_deinit(&pst_hmac_user->st_wapi);
#endif

    ul_ret = hmac_user_free(pst_mac_vap->us_multi_user_idx);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_UM, "{hmac_user_del_multi_user::hmac_user_free fail.}");
    }

    return OAL_SUCC;
}


#ifdef _PRE_WLAN_FEATURE_WAPI
oal_uint8 hmac_user_is_wapi_connected(oal_uint8 uc_device_id)
{
    oal_uint8 uc_vap_idx;
    hmac_user_stru *pst_hmac_user_multi;
    mac_device_stru *pst_mac_device;
    mac_vap_stru *pst_mac_vap;

    pst_mac_device = mac_res_get_dev(uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_UM, "{hmac_user_is_wapi_connected::pst_mac_device null.id %u}", uc_device_id);
        return OAL_FALSE;
    }

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_mac_vap == OAL_PTR_NULL)) {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "vap is null! vap id is %d", pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if (!IS_STA(pst_mac_vap)) {
            continue;
        }

        pst_hmac_user_multi = (hmac_user_stru *)mac_res_get_hmac_user(pst_mac_vap->us_multi_user_idx);
        if ((pst_hmac_user_multi != OAL_PTR_NULL)
            && (pst_hmac_user_multi->st_wapi.uc_port_valid == OAL_TRUE)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}
#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */


oal_uint32 hmac_user_add_notify_alg(mac_vap_stru *pst_mac_vap, oal_uint16 us_user_idx)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    dmac_ctx_add_user_stru *pst_add_user_payload = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;

    /* 抛事件给Dmac，在dmac层挂用户算法钩子 */
    pst_event_mem = frw_event_alloc_m(OAL_SIZEOF(dmac_ctx_add_user_stru));
    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_user_add_notify_alg::pst_event_mem null.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;
    pst_add_user_payload = (dmac_ctx_add_user_stru *)pst_event->auc_event_data;

    pst_add_user_payload->us_user_idx = us_user_idx;
    pst_add_user_payload->us_sta_aid = pst_mac_vap->us_sta_aid;
    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(us_user_idx);
    if (oal_unlikely(pst_hmac_user == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{hmac_user_add_notify_alg::null param,pst_hmac_user[%d].}", us_user_idx);
        frw_event_free_m(pst_event_mem);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_user_get_vht_hdl(&pst_hmac_user->st_user_base_info, &pst_add_user_payload->st_vht_hdl);
    mac_user_get_ht_hdl(&pst_hmac_user->st_user_base_info, &pst_add_user_payload->st_ht_hdl);

    /* 填充事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_NOTIFY_ALG_ADD_USER,
                       OAL_SIZEOF(dmac_ctx_add_user_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_user_add_notify_alg::frw_event_dispatch_event failed[%d].}", ul_ret);
    }

    frw_event_free_m(pst_event_mem);

    hmac_txbf_ability_sync(pst_mac_vap, us_user_idx);

    return OAL_SUCC;
}


hmac_user_stru *mac_vap_get_hmac_user_by_addr(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_mac_addr)
{
    oal_uint32 ul_ret;
    oal_uint16 us_user_idx = 0xffff;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;

    /* 根据mac addr找sta索引 */
    ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap, puc_mac_addr, &us_user_idx);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{mac_vap_get_hmac_user_by_addr::find_user_by_macaddr failed[%d].}", ul_ret);
        if (puc_mac_addr != OAL_PTR_NULL) {
            oam_warning_log3(0, OAM_SF_ANY, "{mac_vap_get_hmac_user_by_addr:: mac_addr[%02x XX XX XX %02x %02x]!.}",
                             puc_mac_addr[0], puc_mac_addr[4], puc_mac_addr[5]); /* puc_mac_addr第0、4、5byte输出打印 */
        }
        return OAL_PTR_NULL;
    }

    /* 根据sta索引找到user内存区域 */
    pst_hmac_user = mac_res_get_hmac_user(us_user_idx);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_vap_get_hmac_user_by_addr::user ptr[%d] null.}", us_user_idx);
    }
    return pst_hmac_user;
}


oal_void *mac_res_get_mac_user(oal_uint16 us_idx)
{
    mac_user_stru *pst_mac_user = OAL_PTR_NULL;

    pst_mac_user = (mac_user_stru *)_mac_res_get_mac_user(us_idx);
    if (pst_mac_user == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    /* 异常: 用户资源已被释放 */
    /* user id=0为特殊user,单播不为0 */
#ifdef WIN32
    /* 此分支是为了保证ut能过加的 */
    if ((pst_mac_user->uc_is_user_alloced != MAC_USER_ALLOCED) && (us_idx != 0))
#else
    if (pst_mac_user->uc_is_user_alloced != MAC_USER_ALLOCED)
#endif
    {
#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)
        /*lint -e718*//*lint -e746*/
        oam_warning_log2(0, OAM_SF_UM, "{mac_res_get_mac_user::[E]user has been freed,user_idx=%d, func[%x].}", us_idx,
                         (oal_uint32)__return_address());
        /*lint +e718*//*lint +e746*/
#else
        OAM_WARNING_LOG1(0, OAM_SF_UM, "{mac_res_get_mac_user::[E]user has been freed,user_idx=%d.}", us_idx);
#endif
        /* device侧获取用户时用户已经释放属于正常，返回空指针，后续调用者查找用户失败，请打印WARNING并直接释放buf，走其他分支等等 */
        return OAL_PTR_NULL;
    }

    return (void *)pst_mac_user;
}

// add_key流程中清除user下的分片缓存，防止重关联或者rekey流程报文重组攻击
void hmac_user_clear_defrag_res(hmac_user_stru *hmac_user)
{
    if (hmac_user == NULL) {
        return;
    }

    oam_warning_log2(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
        "{hmac_user_clear_defrag_res :: timer[%d] netbuf NULL[%d] .}",
        hmac_user->st_defrag_timer.en_is_registerd, (hmac_user->pst_defrag_netbuf == NULL));

    if (hmac_user->st_defrag_timer.en_is_registerd == OAL_TRUE) {
        frw_immediate_destroy_timer(&hmac_user->st_defrag_timer);
    }

    if (hmac_user->pst_defrag_netbuf != NULL) {
        oal_netbuf_free(hmac_user->pst_defrag_netbuf);
        hmac_user->pst_defrag_netbuf = NULL;
    }
}

oal_module_symbol(hmac_user_alloc);
oal_module_symbol(hmac_user_init);
oal_module_symbol(hmac_config_kick_user);
oal_module_symbol(mac_vap_get_hmac_user_by_addr);
oal_module_symbol(mac_res_get_hmac_user);
