

#ifndef __HMAC_ARP_PROBE_C__
#define __HMAC_ARP_PROBE_C__

/* 1 头文件包含 */
#include "hmac_ext_if.h"
#include "mac_data.h"
#include "hmac_resource.h"
#include "hmac_arp_probe.h"
#include "hmac_vap.h"
#include "hmac_user.h"
#include "hmac_fsm.h"
#ifdef _PRE_WLAN_FEATURE_ROAM
#include "hmac_roam_main.h"
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
#include "hmac_wifi6_self_cure.h"
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ARP_PROBE_C

/* 2 函数声明 */
/* 3 函数实现 */

OAL_STATIC oal_uint32  hmac_arp_probe_htctype_switch_get(hmac_vap_stru *pst_hmac_vap,
    oal_bool_enum_uint8 *pen_switch)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    hmac_user_stru *pst_hmac_user;

    if (IS_LEGACY_STA(&pst_hmac_vap->st_vap_base_info)) {
        pst_hmac_user = mac_res_get_hmac_user_etc(pst_hmac_vap->st_vap_base_info.us_assoc_vap_id);
        if (pst_hmac_user == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2S,
                           "hmac_arp_probe_htctype_switch_get: pst_hmac_user is null ptr. user id:%d",
                           pst_hmac_vap->st_vap_base_info.us_assoc_vap_id);
            return OAL_FAIL;
        }

        if (HMAC_WIFI6_SELF_CURE_HANDLE_TYPE_APP == hmac_wifi6_self_cure_get_handle_type()) {
            return OAL_SUCC;
        }

        if (MAC_USER_TX_DATA_INCLUDE_HTC(&pst_hmac_user->st_user_base_info)) {
            *pen_switch = OAL_TRUE;
        }

        return OAL_SUCC;
    }
#endif
    return OAL_FAIL;
}


oal_void hmac_arp_probe_type_set(oal_void *p_arg, oal_bool_enum_uint8 en_arp_detect_on,
    oal_uint8 en_probe_type)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)p_arg;
    hmac_vap_arp_probe_stru *pst_hmac_arp_probe = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap;
    oal_bool_enum_uint8 en_arp_probe_switch = OAL_FALSE;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{hmac_arp_probe_type_set::pst_hmac_vap null.}");
        return;
    }

    pst_hmac_arp_probe = &pst_hmac_vap->st_hmac_arp_probe;
    if (en_probe_type == HMAC_VAP_ARP_PROBE_TYPE_HTC && en_arp_detect_on) {
        if (hmac_arp_probe_htctype_switch_get(pst_hmac_vap, &en_arp_probe_switch) != OAL_SUCC) {
            return;
        }
    } else if ((en_arp_detect_on == OAL_TRUE) &&
        ((en_probe_type == HMAC_VAP_ARP_PROBE_TYPE_DBAC) || (en_probe_type == HMAC_VAP_ARP_PROBE_TYPE_BTCOEX))) {
        if (pst_hmac_vap->en_ps_rx_amsdu == OAL_FALSE) {
            return;
        }
        en_arp_probe_switch = en_arp_detect_on;
    } else if (en_probe_type == HMAC_VAP_ARP_PROBE_TYPE_M2S) {
        en_arp_probe_switch = en_arp_detect_on;
    }

    if (en_arp_probe_switch == OAL_FALSE) {
        /* close */
        pst_hmac_arp_probe->uc_arp_probe_type &= ~en_probe_type;
        /* 其他类型探测已开启，则不能关闭 */
        if (pst_hmac_arp_probe->uc_arp_probe_type != 0) {
            en_arp_probe_switch = OAL_TRUE;
        }
    } else {
        pst_hmac_arp_probe->uc_arp_probe_type |= en_probe_type;
    }
    pst_hmac_arp_probe->en_arp_probe_on = en_arp_probe_switch;
    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{hmac_arp_probe_type_set::en_arp_probe_switch=[%d], \
        en_arp_probe_type=[%d]}",
        pst_hmac_arp_probe->en_arp_probe_on, pst_hmac_arp_probe->uc_arp_probe_type);
}

oal_uint32 hmac_ps_rx_amsdu_arp_probe_process(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len,
    oal_uint8 *puc_param)
{
    dmac_to_hmac_ps_arp_probe_event_stru *pst_dmac_to_hmac_ps_arp_probe = OAL_PTR_NULL;
    oal_uint8 uc_probe_type = HMAC_VAP_ARP_PROBE_TYPE_BUTT;
    oal_bool_enum_uint8 en_arp_detect_on;

    pst_dmac_to_hmac_ps_arp_probe = (dmac_to_hmac_ps_arp_probe_event_stru *)puc_param;
    en_arp_detect_on = pst_dmac_to_hmac_ps_arp_probe->en_arp_probe;
    if (pst_dmac_to_hmac_ps_arp_probe->uc_ps_type == MAC_PS_TYPE_DBAC) {
        uc_probe_type = HMAC_VAP_ARP_PROBE_TYPE_DBAC;
    } else if (pst_dmac_to_hmac_ps_arp_probe->uc_ps_type == MAC_PS_TYPE_BTCOEX) {
        uc_probe_type = HMAC_VAP_ARP_PROBE_TYPE_BTCOEX;
    }
    if (uc_probe_type != HMAC_VAP_ARP_PROBE_TYPE_BUTT) {
        hmac_arp_probe_type_set(pst_mac_vap, en_arp_detect_on, uc_probe_type);
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
            "{hmac_ps_rx_amsdu_arp_probe_process::en_arp_detect_on[%d],en_probe_type[%d].}",
            en_arp_detect_on, uc_probe_type);
    }

    return OAL_SUCC;
}


oal_bool_enum_uint8 hmac_arp_probe_fail_reassoc_trigger(oal_void *p_hmac_vap,
    oal_void *p_hmac_user)
{
    hmac_vap_stru *pst_hmac_vap;
    hmac_user_stru *pst_hmac_user;
    oal_bool_enum_uint8 en_reassoc_codeid = OAL_FALSE;

    pst_hmac_vap  = (hmac_vap_stru *)p_hmac_vap;
    pst_hmac_user = (hmac_user_stru *)p_hmac_user;

    OAM_WARNING_LOG2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2S,
        "{hmac_arp_probe_fail_reassoc_trigger::user rssi[%d] threhold[%d].}",
        pst_hmac_user->c_rssi, WLAN_FAR_DISTANCE_RSSI);

    /* 非远场才触发重关联逻辑 */
    if (pst_hmac_user->c_rssi >= WLAN_FAR_DISTANCE_RSSI) {
        /* 发起reassoc req */
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2S,
            "{hmac_arp_probe_fail_reassoc_trigger::to reassoc.}");
        hmac_roam_start_etc(pst_hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_FALSE,
            NULL, ROAM_TRIGGER_ARP_PROBE_FAIL_REASSOC);
        en_reassoc_codeid = OAL_TRUE;
    }

    return en_reassoc_codeid;
}


OAL_STATIC oal_bool_enum_uint8 hmac_arp_probe_fail_htc_handle(hmac_vap_stru *pst_hmac_vap,
    hmac_user_stru *pst_hmac_user)
{
#if defined(_PRE_WLAN_FEATURE_11AX)
    OAM_WARNING_LOG0(0, OAM_SF_M2S, "{hmac_arp_probe_fail_htc_handle::Set MAC_USER_ARP_PROBE_CLOSE_HTC .}");
    return hmac_wifi6_self_cure_close_htc_handle(&pst_hmac_vap->st_vap_base_info);
#else
    return OAL_FALSE;
#endif
}


OAL_STATIC oal_bool_enum_uint8 hmac_arp_probe_fail_rx_amsdu_handle(hmac_vap_stru *pst_hmac_vap,
    hmac_user_stru *pst_hmac_user)
{
    pst_hmac_vap->en_ps_rx_amsdu = OAL_FALSE;
    OAM_WARNING_LOG0(0, OAM_SF_M2S, "{hmac_arp_probe_fail_rx_amsdu_handle::Set MAC_USER_ARP_PROBE_CLOSE_RX_AMSDU .}");
    return hmac_arp_probe_fail_reassoc_trigger(pst_hmac_vap, pst_hmac_user);
}



OAL_STATIC oal_bool_enum_uint8  hmac_arp_probe_fail_process(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user)
{
    hmac_vap_arp_probe_stru *pst_hmac_arp_probe;
    oal_bool_enum_uint8 en_reassoc_codeid = OAL_FALSE;
    oal_uint8 uc_arp_probe_type;

    pst_hmac_arp_probe = &(pst_hmac_vap->st_hmac_arp_probe);
    uc_arp_probe_type = pst_hmac_arp_probe->uc_arp_probe_type;

    /* 停止arp探测 */
    pst_hmac_arp_probe->en_arp_probe_on    = OAL_FALSE;
    pst_hmac_arp_probe->uc_rx_no_pkt_count = 0;
    pst_hmac_arp_probe->uc_arp_probe_type  = 0;

    if (uc_arp_probe_type & HMAC_VAP_ARP_PROBE_TYPE_HTC) {
        en_reassoc_codeid = hmac_arp_probe_fail_htc_handle(pst_hmac_vap, pst_hmac_user);
    } else if (uc_arp_probe_type & HMAC_VAP_ARP_PROBE_TYPE_M2S) {
        en_reassoc_codeid = hmac_arp_probe_fail_reassoc_trigger(pst_hmac_vap, pst_hmac_user);
    } else if ((uc_arp_probe_type & HMAC_VAP_ARP_PROBE_TYPE_DBAC) ||
        (uc_arp_probe_type & HMAC_VAP_ARP_PROBE_TYPE_BTCOEX)) {
        en_reassoc_codeid = hmac_arp_probe_fail_rx_amsdu_handle(pst_hmac_vap, pst_hmac_user);
    }

    return en_reassoc_codeid;
}


OAL_STATIC oal_void hmac_arp_probe_succ_process(hmac_vap_stru *pst_hmac_vap)
{
    hmac_vap_arp_probe_stru *pst_hmac_arp_probe;

    pst_hmac_arp_probe = &(pst_hmac_vap->st_hmac_arp_probe);

    /* 仅打开m2s探测时，停止arp探测 */
    if (oal_bit_get_bit_one_byte(pst_hmac_arp_probe->uc_arp_probe_type, HMAC_VAP_ARP_PROBE_TYPE_M2S)) {
        /* 停止arp探测 */
        pst_hmac_arp_probe->en_arp_probe_on = OAL_FALSE;
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{hmac_arp_probe_succ_process::only m2s_type detect succ,\
            en_arp_probe_on[%d].}", pst_hmac_arp_probe->en_arp_probe_on);
    }

    /* 清除m2s type */
    oal_bit_clear_bit_one_byte(&pst_hmac_arp_probe->uc_arp_probe_type, HMAC_VAP_ARP_PROBE_TYPE_M2S);
    pst_hmac_arp_probe->uc_rx_no_pkt_count = 0;
}


OAL_STATIC oal_uint32 hmac_arp_probe_timeout(oal_void *p_arg)
{
    hmac_vap_arp_probe_stru *pst_hmac_arp_probe = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_uint32 ui_val;
    oal_bool_enum_uint8 en_reassoc_codeid = OAL_FALSE;

    pst_hmac_vap = (hmac_vap_stru *)p_arg;
    if (pst_hmac_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_arp_probe = &(pst_hmac_vap->st_hmac_arp_probe);
    ui_val = oal_atomic_read(&(pst_hmac_arp_probe->ul_rx_unicast_pkt_to_lan));
    if (ui_val == 0) {
        pst_hmac_arp_probe->uc_rx_no_pkt_count++;
        OAM_WARNING_LOG2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2S,
                         "{hmac_arp_probe_timeout::rx_arp_pkt fail cnt[%d], probe_type=[%d]!}",
                         pst_hmac_arp_probe->uc_rx_no_pkt_count, pst_hmac_arp_probe->uc_arp_probe_type);
        if (pst_hmac_arp_probe->uc_rx_no_pkt_count > ARP_PROBE_FAIL_REASSOC_NUM) {
            pst_hmac_user = mac_res_get_hmac_user_etc(pst_hmac_vap->st_vap_base_info.us_assoc_vap_id);
            if (pst_hmac_user == OAL_PTR_NULL) {
                OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2S,
                               "hmac_arp_probe_timeout: pst_hmac_user is null ptr. user id:%d",
                               pst_hmac_vap->st_vap_base_info.us_assoc_vap_id);
                return OAL_ERR_CODE_PTR_NULL;
            }
            en_reassoc_codeid = hmac_arp_probe_fail_process(pst_hmac_vap, pst_hmac_user);

#ifdef _PRE_WLAN_1103_CHR
            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
                                 CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_MIMO_TO_SISO_FAIL,
                                 en_reassoc_codeid);
#endif
        }
    } else {
        hmac_arp_probe_succ_process(pst_hmac_vap);
    }

    oal_atomic_set(&pst_hmac_arp_probe->ul_rx_unicast_pkt_to_lan, 0);

    return OAL_SUCC;
}


oal_void hma_arp_probe_timer_start(oal_netbuf_stru *pst_netbuf, oal_void *p_arg)
{
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    hmac_vap_arp_probe_stru *pst_hmac_arp_probe = OAL_PTR_NULL;
    mac_ether_header_stru *pst_mac_ether_hdr = OAL_PTR_NULL;
    oal_uint8 uc_data_type;

    pst_hmac_vap = (hmac_vap_stru *)p_arg;

    pst_hmac_arp_probe = &(pst_hmac_vap->st_hmac_arp_probe);

    /* 只要统计功能打开，就需要做一次探测 */
    if (pst_hmac_arp_probe->en_init_succ && pst_hmac_arp_probe->en_arp_probe_on == OAL_TRUE &&
        pst_hmac_arp_probe->st_arp_probe_timer.en_is_registerd == OAL_FALSE) {
        pst_mac_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf);

        /* 参数外面已经做检查，里面没必要再做检查了 */
        uc_data_type = mac_get_data_type_from_8023_etc((oal_uint8 *)pst_mac_ether_hdr, MAC_NETBUFF_PAYLOAD_ETH);
        /* 发送方向创建定时器，多次创建定时器 */
        if (uc_data_type == MAC_DATA_ARP_REQ) {
            /* 每次重启定时器之前清零,保证统计的时间 */
            oal_atomic_set(&(pst_hmac_arp_probe->ul_rx_unicast_pkt_to_lan), 0);
            FRW_TIMER_CREATE_TIMER(&(pst_hmac_arp_probe->st_arp_probe_timer),
                                   hmac_arp_probe_timeout,
                                   ARP_PROBE_TIMEOUT,
                                   pst_hmac_vap,
                                   OAL_FALSE,
                                   OAM_MODULE_ID_HMAC,
                                   pst_hmac_vap->st_vap_base_info.ul_core_id);
        }
    }
}


oal_void hmac_arp_probe_destory(oal_void *p_hmac_vap, oal_void *p_hmac_user)
{
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_11AX
    hmac_user_stru *pst_hmac_user;
#endif
    hmac_vap_arp_probe_stru *pst_hmac_arp_probe = OAL_PTR_NULL;

    if (p_hmac_vap == OAL_PTR_NULL) {
        return;
    }

    pst_hmac_vap = (hmac_vap_stru *)p_hmac_vap;
    if (!IS_LEGACY_STA(&pst_hmac_vap->st_vap_base_info)) {
        return;
    }

    pst_hmac_arp_probe = &pst_hmac_vap->st_hmac_arp_probe;
    OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2S, "hmac_arp_probe_destory::to destory");

#ifdef _PRE_WLAN_FEATURE_11AX
    pst_hmac_user = (hmac_user_stru *)p_hmac_user;
    if (p_hmac_user == OAL_PTR_NULL) {
        return;
    }
    MAC_USER_ARP_PROBE_CLOSE_HTC(&pst_hmac_user->st_user_base_info) = OAL_FALSE;
#endif

    pst_hmac_arp_probe->en_arp_probe_on          = OAL_FALSE;
    pst_hmac_arp_probe->uc_arp_probe_type        = 0;
    oal_atomic_set(&(pst_hmac_arp_probe->ul_rx_unicast_pkt_to_lan), 0);
    pst_hmac_arp_probe->uc_rx_no_pkt_count       = 0;
    pst_hmac_arp_probe->en_init_succ             = OAL_FALSE;
}


oal_void hmac_arp_probe_init(oal_void *p_hmac_vap, oal_void *p_hmac_user)
{
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    hmac_vap_arp_probe_stru *pst_hmac_arp_probe = OAL_PTR_NULL;

    if (p_hmac_vap == OAL_PTR_NULL || p_hmac_user == OAL_PTR_NULL) {
        return;
    }

    pst_hmac_vap = (hmac_vap_stru *)p_hmac_vap;
    if (!IS_LEGACY_STA(&pst_hmac_vap->st_vap_base_info)) {
        return;
    }

    pst_hmac_arp_probe = &pst_hmac_vap->st_hmac_arp_probe;
    OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2S, "hmac_arp_probe_init::to init");
    hmac_arp_probe_destory(pst_hmac_vap, p_hmac_user);
    pst_hmac_arp_probe->en_init_succ = OAL_TRUE;
}

#endif /* end of __HMAC_M2S_C__ */

