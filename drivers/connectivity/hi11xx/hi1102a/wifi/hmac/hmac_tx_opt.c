
/* 1 头文件包含 */
#include "hmac_tx_opt.h"
#include "hmac_vap.h"
#include "mac_device.h"
#include "hmac_config.h"
#include "hmac_statistic_data_flow.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TX_OPT_C

/* 2 全局变量定义 */
#define HMAC_TX_OPT_BITMAP_STA (HMAC_TX_OPT_TPC_DISABLE | HMAC_TX_OPT_EDCA_OPT | \
                                HMAC_TX_OPT_TXOP_OPT | HMAC_TX_OPT_PPDU_SCH)
#define HMAC_TX_OPT_BITMAP_AP  (HMAC_TX_OPT_TPC_DISABLE | HMAC_TX_OPT_EDCA_OPT | \
                                HMAC_TX_OPT_TXOP_OPT | HMAC_TX_OPT_PPDU_SCH)
#define HMAC_HOTSPOT_IP_ADDR0  192
#define HMAC_HOTSPOT_IP_ADDR1  168
#define HMAC_HOTSPOT_IP_ADDR2  43

oal_bool_enum_uint8 g_en_tx_opt = OAL_FALSE;
oal_bool_enum_uint8 g_en_tx_opt_tcp_ack = OAL_FALSE;
oal_uint8 g_uc_opt_switch_cnt = 0;
oal_uint8 g_uc_opt_switch_cnt_tcp_ack = 0;
oal_uint8 g_en_hotspot = OAL_FALSE;

/* 3 函数实现 */

oal_void hmac_set_tx_opt_switch_cnt(oal_uint8 uc_opt_switch_cnt)
{
    g_uc_opt_switch_cnt = uc_opt_switch_cnt;
}

oal_void hmac_set_tx_opt_switch_cnt_tcp_ack(oal_uint8 uc_opt_switch_cnt)
{
    g_uc_opt_switch_cnt_tcp_ack = uc_opt_switch_cnt;
}

oal_bool_enum_uint8 hmac_tx_opt_is_hotspot(oal_uint8 *puc_ip_addr)
{
    return puc_ip_addr[0] == HMAC_HOTSPOT_IP_ADDR0 && puc_ip_addr[1] == HMAC_HOTSPOT_IP_ADDR1 &&
           puc_ip_addr[2] == HMAC_HOTSPOT_IP_ADDR2;
}

oal_void hmac_tx_opt_set_ip_addr(mac_vap_stru *pst_mac_vap, oal_void *pst_ip_addr)
{
    oal_uint8 *puc_ip_addr = (oal_uint8 *)pst_ip_addr;

    if (!IS_LEGACY_STA(pst_mac_vap)) {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{hmac_tx_opt_set_ip_addr::Only legacy sta needs opt}");
        return;
    }

    g_en_hotspot = hmac_tx_opt_is_hotspot(puc_ip_addr);

    OAM_WARNING_LOG3(0, OAM_SF_CFG, "{hmac_tx_opt_set_ip_addr:Up IPv4[%d.X.X.%d]}, g_en_hotspot[%d]",
        puc_ip_addr[0], puc_ip_addr[3], g_en_hotspot);
}

oal_void hmac_tx_opt_del_ip_addr(mac_vap_stru *pst_mac_vap, oal_void *pst_ip_addr)
{
    oal_uint8 *puc_ip_addr = (oal_uint8 *)pst_ip_addr;

    if (!IS_LEGACY_STA(pst_mac_vap)) {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{hmac_tx_opt_del_ip_addr::Only legacy sta needs opt}");
        return;
    }

    if (hmac_tx_opt_is_hotspot(puc_ip_addr)) {
        g_en_hotspot = OAL_FALSE;
    }

    OAM_WARNING_LOG3(0, OAM_SF_CFG, "{hmac_tx_opt_del_ip_addr:Down IPv4[%d.X.X.%d], g_en_hotspot}",
        puc_ip_addr[0], puc_ip_addr[3], g_en_hotspot);
}

oal_bool_enum_uint8 hmac_get_tx_opt_tcp_ack(oal_void)
{
    return g_en_tx_opt_tcp_ack;
}

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ

OAL_STATIC oal_void hmac_set_tx_opt_param(mac_vap_stru *pst_mac_vap, mac_cfg_tx_opt *pst_tx_opt)
{
    oal_uint8 uc_tx_opt_switch;

    if (IS_LEGACY_STA(pst_mac_vap)) {
        uc_tx_opt_switch = g_st_wifi_tx_opt.uc_tx_opt_switch_sta & HMAC_TX_OPT_BITMAP_STA;
    } else if (IS_LEGACY_AP(pst_mac_vap)) {
        uc_tx_opt_switch = g_st_wifi_tx_opt.uc_tx_opt_switch_ap & HMAC_TX_OPT_BITMAP_AP;
    } else {
        return;
    }

    pst_tx_opt->en_tpc_disable = (uc_tx_opt_switch & HMAC_TX_OPT_TPC_DISABLE) != 0;;
    pst_tx_opt->en_edca_opt = (uc_tx_opt_switch & HMAC_TX_OPT_EDCA_OPT) != 0;
    pst_tx_opt->en_txop_opt = (uc_tx_opt_switch & HMAC_TX_OPT_TXOP_OPT) != 0;
    pst_tx_opt->en_ppdu_sch = (uc_tx_opt_switch & HMAC_TX_OPT_PPDU_SCH) != 0;
    pst_tx_opt->uc_cwmin = g_st_wifi_tx_opt.uc_cwmin;
    pst_tx_opt->uc_cwmax = g_st_wifi_tx_opt.uc_cwmax;
    pst_tx_opt->us_txoplimit = g_st_wifi_tx_opt.us_txoplimit;
    pst_tx_opt->us_dyn_txoplimit = g_st_wifi_tx_opt.us_dyn_txoplimit;
}


OAL_STATIC oal_void hmac_set_tx_opt(mac_vap_stru *pst_mac_vap, mac_cfg_tx_opt *pst_tx_opt)
{
    if (g_en_tx_opt) {
        hmac_set_tx_opt_param(pst_mac_vap, pst_tx_opt);
    }
}


OAL_STATIC oal_uint32 hmac_sync_tx_opt_switch(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_tx_opt)
{
    oal_uint32 ul_ret;
    mac_cfg_tx_opt st_tx_opt = {0};

    g_en_tx_opt = en_tx_opt;
    g_uc_opt_switch_cnt = 0;

    hmac_set_tx_opt(pst_mac_vap, &st_tx_opt);

    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TX_OPT_SYN, OAL_SIZEOF(mac_cfg_tx_opt),
                                    (oal_uint8 *)&st_tx_opt);
    if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_sync_tx_opt_switch::hmac_config_send_event failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32 hmac_sync_tx_opt_switch_tcp_ack(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_tx_opt)
{
    oal_uint32 ul_ret;

    g_en_tx_opt_tcp_ack = en_tx_opt;
    g_uc_opt_switch_cnt_tcp_ack = 0;

    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TX_OPT_SYN_TCP_ACK, OAL_SIZEOF(oal_bool_enum_uint8),
                                    (oal_uint8 *)&en_tx_opt);
    if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_sync_tx_opt_switch_tcp_ack::hmac_config_send_event failed[%d]}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_bool_enum_uint8 hmac_get_tx_opt_status(oal_uint32 ul_large_pps, oal_bool_enum_uint8 en_tx_opt)
{
    if (ul_large_pps > g_st_wifi_tx_opt.us_txopt_th_pps_high) {
        return OAL_TRUE;
    } else if (ul_large_pps < g_st_wifi_tx_opt.us_txopt_th_pps_low) {
        return OAL_FALSE;
    } else {
        return en_tx_opt;
    }
}


OAL_STATIC oal_bool_enum_uint8 hmac_tx_opt_change_state(oal_bool_enum_uint8 en_tx_opt)
{
    if (g_en_tx_opt == en_tx_opt) {
        g_uc_opt_switch_cnt = 0;
        return OAL_FALSE;
    }

    g_uc_opt_switch_cnt++;

    /* 尝试开启时立刻关闭; 尝试关闭时连续3次与原状态不一致才关闭 */
    return en_tx_opt || (g_uc_opt_switch_cnt >= MAX_TX_OPT_SWITCH_CNT);
}

OAL_STATIC oal_bool_enum_uint8 hmac_tx_opt_change_state_tcp_ack(oal_bool_enum_uint8 en_tx_opt)
{
    if (g_en_tx_opt_tcp_ack == en_tx_opt) {
        g_uc_opt_switch_cnt_tcp_ack = 0;
        return OAL_FALSE;
    }

    g_uc_opt_switch_cnt_tcp_ack++;

    /* 尝试开启时立刻开启; 尝试关闭时连续3次与原状态不一致才关闭 */
    return en_tx_opt || (g_uc_opt_switch_cnt_tcp_ack >= MAX_TX_OPT_SWITCH_CNT);
}


OAL_STATIC oal_bool_enum_uint8 hmac_tx_opt_allowed_ap(mac_vap_stru *pst_mac_vap)
{
    return (g_st_wifi_tx_opt.uc_tx_opt_switch_ap & HMAC_TX_OPT_BITMAP_AP) != 0 && pst_mac_vap->us_user_nums == 1;
}


OAL_STATIC oal_bool_enum_uint8 hmac_tx_opt_allowed_sta(oal_void)
{
    return (g_st_wifi_tx_opt.uc_tx_opt_switch_sta & HMAC_TX_OPT_BITMAP_STA) != 0;
}


OAL_STATIC oal_bool_enum_uint8 hmac_tx_opt_allowed(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    if (mac_device_calc_up_vap_num(pst_mac_device) != 1 ||
        mac_device_find_up_vap(pst_mac_device, ppst_mac_vap) != OAL_SUCC) {
        return OAL_FALSE;
    }

    if (IS_LEGACY_STA(*ppst_mac_vap)) {
        return hmac_tx_opt_allowed_sta();
    } else if (IS_LEGACY_AP(*ppst_mac_vap)) {
        return hmac_tx_opt_allowed_ap(*ppst_mac_vap);
    } else {
        return OAL_FALSE;
    }
}

OAL_STATIC oal_bool_enum_uint8 hmac_tx_opt_allowed_tcp_ack(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    if (mac_device_calc_up_vap_num(pst_mac_device) != 1 ||
        mac_device_find_up_vap(pst_mac_device, ppst_mac_vap) != OAL_SUCC) {
        return OAL_FALSE;
    }

    if (IS_LEGACY_STA(*ppst_mac_vap)) {
        return (g_st_wifi_tx_opt.uc_tx_opt_switch_sta & HMAC_TX_OPT_STAUT_TCP_ACK_OPT) != 0 && g_en_hotspot;
    }

    return OAL_FALSE;
}


oal_void hmac_tx_opt_switch(oal_uint32 ul_tx_large_pps)
{
    oal_bool_enum_uint8 en_tx_opt;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device = mac_res_get_dev(0);

    if (!hmac_tx_opt_allowed(pst_mac_device, &pst_mac_vap)) {
        return;
    }

    en_tx_opt = hmac_get_tx_opt_status(ul_tx_large_pps, g_en_tx_opt);
    if (hmac_tx_opt_change_state(en_tx_opt)) {
        hmac_sync_tx_opt_switch(pst_mac_vap, en_tx_opt);
    }
}

oal_void hmac_tx_opt_switch_tcp_ack(oal_uint32 ul_rx_large_pps)
{
    oal_bool_enum_uint8 en_tx_opt;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device = mac_res_get_dev(0);

    if (!hmac_tx_opt_allowed_tcp_ack(pst_mac_device, &pst_mac_vap)) {
        return;
    }

    en_tx_opt = hmac_get_tx_opt_status(ul_rx_large_pps, g_en_tx_opt_tcp_ack);
    if (hmac_tx_opt_change_state_tcp_ack(en_tx_opt)) {
        hmac_sync_tx_opt_switch_tcp_ack(pst_mac_vap, en_tx_opt);
    }
}
#endif

