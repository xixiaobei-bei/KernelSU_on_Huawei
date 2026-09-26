

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP

/* 1 头文件包含 */
#include "hmac_edca_opt.h"
#include "hmac_vap.h"
#include "oam_wdk.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_EDCA_OPT_C
/* 2 结构体定义 */
/* 3 宏定义 */
#define HMAC_EDCA_OPT_ADJ_STEP 2

/* (3-a)/3*X + a/3*Y */
#define wlan_edca_opt_mod(X, Y, a) (((X) * (WLAN_EDCA_OPT_MAX_WEIGHT_STA - a) + (Y) * (a)) / WLAN_EDCA_OPT_MAX_WEIGHT_STA);

/* 4 全局变量定义 */
/* 5 内部静态函数声明 */
OAL_STATIC oal_bool_enum_uint8 hmac_edca_opt_check_is_tcp_data(mac_ip_header_stru *pst_ip);
OAL_STATIC oal_void hmac_edca_opt_stat_traffic_num(
    hmac_vap_stru *pst_hmac_vap, oal_uint8 (*ppuc_traffic_num)[WLAN_TXRX_DATA_BUTT]);

/* 6 函数实现 */

OAL_STATIC oal_bool_enum_uint8 hmac_edca_opt_check_is_tcp_data(mac_ip_header_stru *pst_ip)
{
    oal_uint8 *puc_ip = (oal_uint8 *)pst_ip;
    oal_uint16 us_ip_len;
    oal_uint8 uc_ip_header_len = ((*puc_ip) & 0x0F) << 2; /* 先保留*puc_ip低四位，再左移2位，这个值就是header len */
    oal_uint8 uc_tcp_header_len;

    /* 获取ip报文长度 */
    us_ip_len = (*(puc_ip + 2 /* length in ip header(偏移2byte) */)) << 8; /* puc_ip偏移2byte再左移8位 */
    us_ip_len |= *(puc_ip + 2 /* length in ip header(偏移2byte) */ + 1);

    /* 获取tcp header长度 */
    uc_tcp_header_len = *(puc_ip + uc_ip_header_len + 12 /* length in tcp header(偏移12byte) */);
    uc_tcp_header_len = (uc_tcp_header_len >> 4) << 2; /* cuc_tcp_header_len先右移4位，再左移2位 */

    if ((us_ip_len - uc_ip_header_len) == uc_tcp_header_len) {
        return OAL_FALSE;
    } else {
        return OAL_TRUE;
    }
}


OAL_STATIC oal_void hmac_edca_opt_stat_traffic_num(hmac_vap_stru *pst_hmac_vap,
                                                   oal_uint8 (*ppuc_traffic_num)[WLAN_TXRX_DATA_BUTT])
{
    mac_user_stru *pst_user;
    hmac_user_stru *pst_hmac_user;
    oal_uint8 uc_ac_idx;
    oal_uint8 uc_data_idx;
    mac_vap_stru *pst_vap = &(pst_hmac_vap->st_vap_base_info);
    oal_dlist_head_stru *pst_list_pos = OAL_PTR_NULL;

    pst_list_pos = pst_vap->st_mac_user_list_head.pst_next;

    for (; pst_list_pos != &(pst_vap->st_mac_user_list_head); pst_list_pos = pst_list_pos->pst_next) {
        pst_user = oal_dlist_get_entry(pst_list_pos, mac_user_stru, st_user_dlist);
        pst_hmac_user = mac_res_get_hmac_user(pst_user->us_assoc_id);
        if (pst_hmac_user == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG,
                           "hmac_edca_opt_stat_traffic_num: pst_hmac_user[%d] is null ptr!", pst_user->us_assoc_id);
            continue;
        }

        for (uc_ac_idx = 0; uc_ac_idx < WLAN_WME_AC_BUTT; uc_ac_idx++) {
            for (uc_data_idx = 0; uc_data_idx < WLAN_TXRX_DATA_BUTT; uc_data_idx++) {
                if (pst_hmac_user->aaul_txrx_data_stat[uc_ac_idx][uc_data_idx] > HMAC_EDCA_OPT_PKT_NUM) {
                    ppuc_traffic_num[uc_ac_idx][uc_data_idx]++;
                }

                /* 统计完毕置0 */
                pst_hmac_user->aaul_txrx_data_stat[uc_ac_idx][uc_data_idx] = 0;
            }
        }
    }

    return;
}


oal_uint32 hmac_edca_opt_timeout_fn(oal_void *p_arg)
{
    oal_uint8 aast_uc_traffic_num[WLAN_WME_AC_BUTT][WLAN_TXRX_DATA_BUTT];
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;

    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event;

    if (oal_unlikely(p_arg == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_edca_opt_timeout_fn::p_arg is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)p_arg;

    /* 计数初始化 */
    memset_s(aast_uc_traffic_num, OAL_SIZEOF(aast_uc_traffic_num), 0, OAL_SIZEOF(aast_uc_traffic_num));

    /* 统计device下所有用户上/下行 TPC/UDP条数目 */
    hmac_edca_opt_stat_traffic_num(pst_hmac_vap, aast_uc_traffic_num);

    /***************************************************************************
        抛事件到dmac模块,将统计信息报给dmac
    ***************************************************************************/
    pst_event_mem = frw_event_alloc_m(OAL_SIZEOF(aast_uc_traffic_num));
    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANTI_INTF,
                       "{hmac_edca_opt_timeout_fn::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPR_EDCA_OPT,
                       OAL_SIZEOF(aast_uc_traffic_num), FRW_EVENT_PIPELINE_STAGE_1,
                       pst_hmac_vap->st_vap_base_info.uc_chip_id,
                       pst_hmac_vap->st_vap_base_info.uc_device_id,
                       pst_hmac_vap->st_vap_base_info.uc_vap_id);

    /* 拷贝参数 */
    if (memcpy_s(frw_get_event_payload(pst_event_mem), OAL_SIZEOF(aast_uc_traffic_num),
                 (oal_uint8 *)aast_uc_traffic_num, OAL_SIZEOF(aast_uc_traffic_num)) != EOK) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANTI_INTF,
                       "{hmac_edca_opt_timeout_fn::memcpy fail!}");
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);

    return OAL_SUCC;
}


oal_void hmac_edca_opt_rx_pkts_stat(oal_uint16 us_assoc_id, oal_uint8 uc_tidno, mac_ip_header_stru *pst_ip)
{
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(us_assoc_id);
    if (oal_unlikely(pst_hmac_user == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_RX, "{hmac_edca_opt_rx_pkts_stat::null param,pst_hmac_user[%d].}", us_assoc_id);
        return;
    }
    oam_info_log0(0, OAM_SF_RX, "{hmac_edca_opt_rx_pkts_stat}");

    /* 过滤IP_LEN 小于 HMAC_EDCA_OPT_MIN_PKT_LEN的报文 */
    if (OAL_NET2HOST_SHORT(pst_ip->us_tot_len) < HMAC_EDCA_OPT_MIN_PKT_LEN) {
        return;
    }

    if (pst_ip->uc_protocol == MAC_UDP_PROTOCAL) {
        pst_hmac_user->aaul_txrx_data_stat[WLAN_WME_TID_TO_AC(uc_tidno)][WLAN_RX_UDP_DATA]++;
    } else if (pst_ip->uc_protocol == MAC_TCP_PROTOCAL) {
        if (hmac_edca_opt_check_is_tcp_data(pst_ip)) {
            pst_hmac_user->aaul_txrx_data_stat[WLAN_WME_TID_TO_AC(uc_tidno)][WLAN_RX_TCP_DATA]++;
        }
    } else {
        oam_info_log0(0, OAM_SF_RX, "{hmac_tx_pkts_stat_for_edca_opt: neither UDP nor TCP ");
    }
}


oal_void hmac_edca_opt_tx_pkts_stat(mac_tx_ctl_stru *pst_tx_ctl, oal_uint8 uc_tidno, mac_ip_header_stru *pst_ip)
{
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(MAC_GET_CB_TX_USER_IDX(pst_tx_ctl));
    if (oal_unlikely(pst_hmac_user == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{hmac_edca_opt_rx_pkts_stat::null param,pst_hmac_user[%d].}",
                       MAC_GET_CB_TX_USER_IDX(pst_tx_ctl));
        return;
    }
    oam_info_log0(0, OAM_SF_RX, "{hmac_edca_opt_tx_pkts_stat}");

    /* 过滤IP_LEN 小于 HMAC_EDCA_OPT_MIN_PKT_LEN的报文 */
    if (OAL_NET2HOST_SHORT(pst_ip->us_tot_len) < HMAC_EDCA_OPT_MIN_PKT_LEN) {
        return;
    }

    if (pst_ip->uc_protocol == MAC_UDP_PROTOCAL) {
        pst_hmac_user->aaul_txrx_data_stat[WLAN_WME_TID_TO_AC(uc_tidno)][WLAN_TX_UDP_DATA]++;
    } else if (pst_ip->uc_protocol == MAC_TCP_PROTOCAL) {
        if (hmac_edca_opt_check_is_tcp_data(pst_ip)) {
            pst_hmac_user->aaul_txrx_data_stat[WLAN_WME_TID_TO_AC(uc_tidno)][WLAN_TX_TCP_DATA]++;
        }
    } else {
        oam_info_log0(0, OAM_SF_TX, "{hmac_edca_opt_tx_pkts_stat: neither UDP nor TCP");
    }
}

#endif /* end of _PRE_WLAN_FEATURE_EDCA_OPT_AP */

