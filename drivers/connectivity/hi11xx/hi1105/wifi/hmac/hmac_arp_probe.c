

#ifndef __HMAC_ARP_PROBE_C__
#define __HMAC_ARP_PROBE_C__

#include "hmac_ext_if.h"
#include "mac_data.h"
#include "hmac_resource.h"
#include "hmac_arp_probe.h"
#include "hmac_vap.h"
#include "hmac_user.h"
#include "hmac_fsm.h"
#include "hmac_roam_main.h"
#ifdef _PRE_WLAN_FEATURE_11AX
#include "hmac_wifi6_self_cure.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ARP_PROBE_C

/*
 * 功能描述 : htc arp probe,设置arp_probe 开关，配置arp_probe类型
 * 打开htc arp_probe 条件: IS_LEGACY_STA && MAC_USER_TX_DATA_INCLUDE_HTC
 * 打开htc arp_probe 条件: 收到一个trigger帧
 */
OAL_STATIC uint32_t  hmac_arp_probe_htctype_switch_get(hmac_vap_stru *pst_hmac_vap, oal_bool_enum_uint8 *pen_switch)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        hmac_user_stru *pst_hmac_user = NULL;
        if (IS_LEGACY_STA(&pst_hmac_vap->st_vap_base_info)) {
            pst_hmac_user = mac_res_get_hmac_user(pst_hmac_vap->st_vap_base_info.us_assoc_vap_id);
            if (pst_hmac_user == NULL) {
                oam_error_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2S,
                               "hmac_arp_probe_htctype_switch_get: pst_hmac_user is null ptr. user id:%d",
                               pst_hmac_vap->st_vap_base_info.us_assoc_vap_id);
                return OAL_FAIL;
            }

            if (hmac_wifi6_self_cure_get_handle_type() == HMAC_WIFI6_SELF_CURE_HANDLE_TYPE_APP) {
                return OAL_SUCC;
            }

            if (MAC_USER_TX_DATA_INCLUDE_HTC(&pst_hmac_user->st_user_base_info)) {
                *pen_switch = OAL_TRUE;
            }
            return OAL_SUCC;
        }
    }
#endif
    return OAL_FAIL;
}

void hmac_arp_probe_type_set(void *p_arg, oal_bool_enum_uint8 en_arp_detect_on, uint8_t en_probe_type)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)p_arg;
    hmac_vap_arp_probe_stru *pst_hmac_arp_probe = NULL;
    hmac_vap_stru *pst_hmac_vap;
    oal_bool_enum_uint8 en_arp_probe_switch = OAL_FALSE;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{hmac_arp_probe_type_set::pst_hmac_vap null.}");
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
    } else {
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
    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{hmac_arp_probe_type_set::en_arp_probe_switch=[%d], \
        en_arp_probe_type=[%d]}",
        pst_hmac_arp_probe->en_arp_probe_on, pst_hmac_arp_probe->uc_arp_probe_type);
}

uint32_t hmac_ps_rx_amsdu_arp_probe_process(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    dmac_to_hmac_ps_arp_probe_event_stru *pst_dmac_to_hmac_ps_arp_probe = NULL;
    uint8_t uc_probe_type = HMAC_VAP_ARP_PROBE_TYPE_BUTT;
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
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
            "{hmac_ps_rx_amsdu_arp_probe_process::en_arp_detect_on[%d],en_probe_type[%d].}",
            en_arp_detect_on, uc_probe_type);
    }

    return OAL_SUCC;
}
/*
 * 功能描述 : htc 类型arp 探测失败处理函数，设置bit_arp_probe_close_htc，触发重关联
 */
OAL_STATIC oal_bool_enum_uint8 hmac_arp_probe_fail_htc_handle(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
#if defined(_PRE_WLAN_FEATURE_11AX)
    if ((g_wlan_spec_cfg->feature_11ax_is_open == OAL_FALSE) ||
        (!MAC_USER_TX_DATA_INCLUDE_HTC(&hmac_user->st_user_base_info))) {
        return OAL_FALSE;
    }
    MAC_USER_ARP_PROBE_CLOSE_HTC(&hmac_user->st_user_base_info) = OAL_TRUE;
    oam_warning_log0(0, OAM_SF_M2S, "{hmac_arp_probe_fail_htc_handle::Set MAC_USER_ARP_PROBE_CLOSE_HTC .}");
    hmac_wifi6_close_htc_user_info(hmac_user);
    return OAL_TRUE;
#endif
}

OAL_STATIC oal_bool_enum_uint8  hmac_arp_probe_fail_process(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user)
{
    hmac_vap_arp_probe_stru *pst_hmac_arp_probe;
    uint8_t uc_arp_probe_type;
    oal_bool_enum_uint8 need_reassoc = OAL_FALSE;

    pst_hmac_arp_probe = &(pst_hmac_vap->st_hmac_arp_probe);
    uc_arp_probe_type = pst_hmac_arp_probe->uc_arp_probe_type;

    /* 停止arp探测 */
    pst_hmac_arp_probe->en_arp_probe_on    = OAL_FALSE;
    pst_hmac_arp_probe->uc_rx_no_pkt_count = 0;
    pst_hmac_arp_probe->uc_arp_probe_type  = 0;

    if (uc_arp_probe_type & HMAC_VAP_ARP_PROBE_TYPE_HTC) {
        need_reassoc = hmac_arp_probe_fail_htc_handle(pst_hmac_vap, pst_hmac_user);
    }
    if (uc_arp_probe_type & HMAC_VAP_ARP_PROBE_TYPE_M2S) {
        need_reassoc = OAL_TRUE;
    }
    if ((uc_arp_probe_type & HMAC_VAP_ARP_PROBE_TYPE_DBAC) ||
        (uc_arp_probe_type & HMAC_VAP_ARP_PROBE_TYPE_BTCOEX)) {
        pst_hmac_vap->en_ps_rx_amsdu = OAL_FALSE;
        need_reassoc = OAL_TRUE;
    }
    if (uc_arp_probe_type & HMAC_VAP_ARP_PROBE_TYPE_BTCOEX_DELBA) {
        pst_hmac_user->st_hmac_user_btcoex.us_ba_size = 0;
        pst_hmac_arp_probe->uc_arp_probe_type &= (~HMAC_VAP_ARP_PROBE_TYPE_BTCOEX_DELBA);
        need_reassoc = OAL_TRUE;
    }
    oam_warning_log3(0, OAM_SF_ANY, "hmac_arp_probe_fail_process:cur_rssi=%d, arp_probe_type=0x%x, need_reassoc=%d",
        pst_hmac_user->c_rssi, uc_arp_probe_type, need_reassoc);
    if ((need_reassoc == OAL_TRUE) && (pst_hmac_user->c_rssi > WLAN_FAR_DISTANCE_RSSI)) {
        /* 发起reassoc req */
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
            "{hmac_arp_probe_fail_process::to reassoc.}");
        hmac_roam_start(pst_hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_FALSE,
            NULL, ROAM_TRIGGER_ARP_PROBE_FAIL_REASSOC);
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

OAL_STATIC void hmac_arp_probe_succ_process(hmac_vap_stru *pst_hmac_vap)
{
    hmac_vap_arp_probe_stru *pst_hmac_arp_probe;

    pst_hmac_arp_probe = &(pst_hmac_vap->st_hmac_arp_probe);

    /* 仅打开m2s探测时，停止arp探测 */
    if (oal_bit_get_bit_one_byte(pst_hmac_arp_probe->uc_arp_probe_type, HMAC_VAP_ARP_PROBE_TYPE_M2S)) {
        /* 停止arp探测 */
        pst_hmac_arp_probe->en_arp_probe_on = OAL_FALSE;
        oam_warning_log1(0, OAM_SF_M2S, "{hmac_arp_probe_succ_process::only m2s_type detect succ,\
            en_arp_probe_on[%d].}", pst_hmac_arp_probe->en_arp_probe_on);
    }

    /* 清除m2s type */
    oal_bit_clear_bit_one_byte(&pst_hmac_arp_probe->uc_arp_probe_type, HMAC_VAP_ARP_PROBE_TYPE_M2S);
    pst_hmac_arp_probe->uc_rx_no_pkt_count = 0;
}

OAL_STATIC uint32_t hmac_arp_probe_timeout(void *p_arg)
{
    hmac_vap_arp_probe_stru *pst_hmac_arp_probe = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    uint32_t ui_val;
    oal_bool_enum_uint8 en_reassoc_codeid;

    pst_hmac_vap = (hmac_vap_stru *)p_arg;
    if (pst_hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_arp_probe = &(pst_hmac_vap->st_hmac_arp_probe);
    ui_val = oal_atomic_read(&(pst_hmac_arp_probe->rx_unicast_pkt_to_lan));
    if (ui_val == 0) {
        pst_hmac_arp_probe->uc_rx_no_pkt_count++;
        oam_warning_log2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2S,
                         "{hmac_arp_probe_timeout::rx_arp_pkt fail cnt[%d], probe_type=[%d]!}",
                         pst_hmac_arp_probe->uc_rx_no_pkt_count, pst_hmac_arp_probe->uc_arp_probe_type);
        if (pst_hmac_arp_probe->uc_rx_no_pkt_count > ARP_PROBE_FAIL_REASSOC_NUM) {
            pst_hmac_user = mac_res_get_hmac_user(pst_hmac_vap->st_vap_base_info.us_assoc_vap_id);
            if (pst_hmac_user == NULL) {
                oam_error_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2S,
                               "hmac_arp_probe_timeout: pst_hmac_user is null ptr. user id:%d",
                               pst_hmac_vap->st_vap_base_info.us_assoc_vap_id);
                return OAL_ERR_CODE_PTR_NULL;
            }
            en_reassoc_codeid = hmac_arp_probe_fail_process(pst_hmac_vap, pst_hmac_user);

            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
                                 CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_MIMO_TO_SISO_FAIL,
                                 en_reassoc_codeid);
        }
    } else {
        hmac_arp_probe_succ_process(pst_hmac_vap);
    }

    oal_atomic_set(&pst_hmac_arp_probe->rx_unicast_pkt_to_lan, 0);

    return OAL_SUCC;
}

/*
 * 功能描述  : 发送方向arp帧成功率统计(发送arp req之后进行rx方向单播数据帧统计，一直收不到，触发重关联逻辑)
 */
void hma_arp_probe_timer_start(oal_netbuf_stru *pst_netbuf, void *p_arg)
{
    hmac_vap_stru *pst_hmac_vap = NULL;
    hmac_vap_arp_probe_stru *pst_hmac_arp_probe = NULL;
    mac_ether_header_stru *pst_mac_ether_hdr = NULL;
    uint8_t uc_data_type;

    pst_hmac_vap = (hmac_vap_stru *)p_arg;

    pst_hmac_arp_probe = &(pst_hmac_vap->st_hmac_arp_probe);

    /* 只要统计功能打开，就需要做一次探测 */
    if (pst_hmac_arp_probe->en_init_succ && pst_hmac_arp_probe->en_arp_probe_on == OAL_TRUE &&
        pst_hmac_arp_probe->st_arp_probe_timer.en_is_registerd == OAL_FALSE) {
        pst_mac_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf);

        /* 参数外面已经做检查，里面没必要再做检查了 */
        uc_data_type = mac_get_data_type_from_8023((uint8_t *)pst_mac_ether_hdr, MAC_NETBUFF_PAYLOAD_ETH);
        /* 发送方向创建定时器，多次创建定时器 */
        if (uc_data_type == MAC_DATA_ARP_REQ) {
            /* 每次重启定时器之前清零,保证统计的时间 */
            oal_atomic_set(&(pst_hmac_arp_probe->rx_unicast_pkt_to_lan), 0);
            frw_timer_create_timer_m(&(pst_hmac_arp_probe->st_arp_probe_timer), hmac_arp_probe_timeout,
                ARP_PROBE_TIMEOUT, pst_hmac_vap, OAL_FALSE, OAM_MODULE_ID_HMAC,
                pst_hmac_vap->st_vap_base_info.core_id);
        }
    }
}

void hmac_arp_probe_destory(void *p_hmac_vap, void *p_hmac_user)
{
    hmac_vap_stru *pst_hmac_vap = NULL;
#ifdef _PRE_WLAN_FEATURE_11AX
    hmac_user_stru *pst_hmac_user = NULL;
#endif
    hmac_vap_arp_probe_stru *pst_hmac_arp_probe = NULL;

    if (p_hmac_vap == NULL) {
        return;
    }

    pst_hmac_vap = (hmac_vap_stru *)p_hmac_vap;
    if (!IS_LEGACY_STA(&pst_hmac_vap->st_vap_base_info)) {
        return;
    }

    pst_hmac_arp_probe = &pst_hmac_vap->st_hmac_arp_probe;
    oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2S, "hmac_arp_probe_destory::to destory");

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        pst_hmac_user = (hmac_user_stru *)p_hmac_user;
        if (p_hmac_user == NULL) {
            return;
        }
        MAC_USER_ARP_PROBE_CLOSE_HTC(&pst_hmac_user->st_user_base_info) = OAL_FALSE;
    }
#endif

    pst_hmac_arp_probe->en_arp_probe_on          = OAL_FALSE;
    pst_hmac_arp_probe->uc_arp_probe_type        = 0;
    oal_atomic_set(&(pst_hmac_arp_probe->rx_unicast_pkt_to_lan), 0);
    pst_hmac_arp_probe->uc_rx_no_pkt_count       = 0;
    pst_hmac_arp_probe->en_init_succ             = OAL_FALSE;
}

void hmac_arp_probe_init(void *p_hmac_vap, void *p_hmac_user)
{
    hmac_vap_stru *pst_hmac_vap = NULL;
    hmac_vap_arp_probe_stru *pst_hmac_arp_probe = NULL;

    if (p_hmac_vap == NULL || p_hmac_user == NULL) {
        return;
    }

    pst_hmac_vap = (hmac_vap_stru *)p_hmac_vap;
    if (!IS_LEGACY_STA(&pst_hmac_vap->st_vap_base_info)) {
        return;
    }

    pst_hmac_arp_probe = &pst_hmac_vap->st_hmac_arp_probe;
    oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2S, "hmac_arp_probe_init::to init");
    hmac_arp_probe_destory(pst_hmac_vap, p_hmac_user);
    pst_hmac_arp_probe->en_init_succ = OAL_TRUE;
}

#endif /* end of __HMAC_M2S_C__ */