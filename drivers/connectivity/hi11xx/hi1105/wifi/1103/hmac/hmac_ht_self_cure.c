
#ifndef __HMAC_HT_SELF_CURE_C_
#define __HMAC_HT_SELF_CURE_C_

/* 1 头文件包含 */
#include "hmac_ext_if.h"
#include "mac_data.h"
#include "hmac_resource.h"
#include "hmac_ht_self_cure.h"
#include "hmac_vap.h"
#include "hmac_user.h"
#include "hmac_fsm.h"
#include "mac_user.h"
#include "mac_frame.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_HT_SELF_CURE_C

#define HT_SELF_CURE_CLOSE_TX_DHCP_MAX_TIMES 2

OAL_STATIC hmac_ht_self_cure_stru g_st_ht_self_cure_info;

/* 2 函数声明 */
/* 3 函数实现 */
OAL_STATIC oal_void hmac_ht_self_cure_checkinfo_init(oal_void)
{
    hmac_ht_self_cure_stru *p_ht_self_cure = OAL_PTR_NULL;

    p_ht_self_cure = &g_st_ht_self_cure_info;
    memset_s(&p_ht_self_cure->htCheckInfo, sizeof(hmac_ht_blacklist_checkinfo_stru), 0,
        sizeof(hmac_ht_blacklist_checkinfo_stru));
}

oal_bool_enum_uint8 hmac_ht_self_cure_is_htassoc_user(uint8_t *p_UserMac)
{
    hmac_ht_self_cure_stru *p_ht_self_cure = OAL_PTR_NULL;

    p_ht_self_cure = &g_st_ht_self_cure_info;

    if (!OAL_MEMCMP(p_ht_self_cure->htCheckInfo.htAssocUser, p_UserMac, WLAN_MAC_ADDR_LEN)) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

OAL_STATIC oal_void hmac_ht_self_cure_state_change(uint8_t newChange)
{
    hmac_ht_self_cure_stru *p_ht_self_cure = OAL_PTR_NULL;
    p_ht_self_cure = &g_st_ht_self_cure_info;

    OAM_WARNING_LOG2(0, OAM_SF_SCAN, "hmac_ht_self_cure_state_change::state change from %d to %d",
        p_ht_self_cure->htCheckInfo.htSelfCureCheckState, newChange);
    p_ht_self_cure->htCheckInfo.htSelfCureCheckState = newChange;
}

OAL_STATIC oal_void hmac_ht_self_cure_add_blacklist(uint8_t *userMac)
{
    uint8_t loop;
    hmac_ht_self_cure_stru *p_ht_self_cure = &g_st_ht_self_cure_info;
    hmac_ht_blacklist_stru *p_blacklist = OAL_PTR_NULL;

    if (p_ht_self_cure->blackListNum == HMAC_HT_BLACKLIST_MAX_NUM) {
        memset_s(p_ht_self_cure->htBlackList, sizeof(hmac_ht_blacklist_stru) * HMAC_HT_BLACKLIST_MAX_NUM, 0,
            sizeof(hmac_ht_blacklist_stru) * HMAC_HT_BLACKLIST_MAX_NUM);
        p_ht_self_cure->blackListNum = 0;
    }

    for (loop = 0; loop < p_ht_self_cure->blackListNum; loop++) {
        p_blacklist = &p_ht_self_cure->htBlackList[loop];
        if (!OAL_MEMCMP(p_blacklist->userMacAddr, userMac, WLAN_MAC_ADDR_LEN)) {
            return;
        }
    }

    memcpy_s(p_ht_self_cure->htBlackList[p_ht_self_cure->blackListNum].userMacAddr, WLAN_MAC_ADDR_LEN,
        userMac, WLAN_MAC_ADDR_LEN);
    p_ht_self_cure->blackListNum++;
#ifdef _PRE_WLAN_1103_CHR
    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
        CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_HT_SELF_CURE_OCCUR, CHR_WIFI_HT_SELF_CURE_ASSOC);
#endif

    OAM_WARNING_LOG4(0, OAM_SF_SCAN, "hmac_ht_self_cure_add_blacklist::user_mac_%d:%02X:XX:XX:XX:%02X:%02X",
        p_ht_self_cure->blackListNum, userMac[0], userMac[4], userMac[5]);
}

OAL_STATIC oal_void hmac_ht_self_cure_state_init_assoc_event_handle(mac_vap_stru *p_mac_vap,
    uint8_t *userMac, uint8_t eventId)
{
    hmac_ht_self_cure_stru *p_ht_self_cure = &g_st_ht_self_cure_info;

    if (p_mac_vap->en_protocol == WLAN_HT_MODE) {
        memcpy_s(p_ht_self_cure->htCheckInfo.htAssocUser, WLAN_MAC_ADDR_LEN, userMac, WLAN_MAC_ADDR_LEN);
        hmac_ht_self_cure_state_change(HMAC_HT_SELF_CURE_STATE_ASSOC);
        p_ht_self_cure->htCheckInfo.txDhcpTimes = 0;
    } else {
        hmac_ht_self_cure_checkinfo_init();
    }
}

OAL_STATIC oal_void hmac_ht_self_cure_state_init_handle(mac_vap_stru *p_mac_vap,
    uint8_t *userMac, uint8_t eventId)
{
  switch (eventId) {
      case HMAC_HT_SELF_CURE_EVENT_ASSOC:
          hmac_ht_self_cure_state_init_assoc_event_handle(p_mac_vap, userMac, eventId);
          break;
      default:
          break;
  }
}

OAL_STATIC oal_void hmac_ht_self_cure_state_assoc_handle(mac_vap_stru *p_mac_vap,
    uint8_t *userMac, uint8_t eventId)
{
    hmac_ht_self_cure_stru *p_ht_self_cure = &g_st_ht_self_cure_info;

    switch (eventId) {
        case HMAC_HT_SELF_CURE_EVENT_TX_DHCP_FRAME:
            p_ht_self_cure->htCheckInfo.txDhcpTimes++;
            hmac_ht_self_cure_state_change(HMAC_HT_SELF_CURE_STATE_HT_TX_DHCP);
            break;
        default:
            break;
    }
}

OAL_STATIC oal_void hmac_ht_self_cure_state_ht_txdhcp_handle(mac_vap_stru *p_mac_vap,
    uint8_t *userMac, uint8_t eventId)
{
    hmac_ht_self_cure_stru *p_ht_self_cure = &g_st_ht_self_cure_info;

    switch (eventId) {
        case HMAC_HT_SELF_CURE_EVENT_TX_DHCP_FRAME:
            p_ht_self_cure->htCheckInfo.txDhcpTimes++;
            if (p_ht_self_cure->htCheckInfo.txDhcpTimes > HT_SELF_CURE_CLOSE_TX_DHCP_MAX_TIMES) {
                hmac_ht_self_cure_state_change(HMAC_HT_SELF_CURE_STATE_CLOSE_HT);
                memcpy_s(p_ht_self_cure->htCheckInfo.checkHtUser, WLAN_MAC_ADDR_LEN, userMac, WLAN_MAC_ADDR_LEN);
            }
            break;
        case HMAC_HT_SELF_CURE_EVENT_RX_DHCP_FRAME:
            hmac_ht_self_cure_checkinfo_init();
            break;
        default:
            break;
    }
}

OAL_STATIC oal_void hmac_ht_self_cure_state_closeht_handle(mac_vap_stru *p_mac_vap,
    uint8_t *userMac, uint8_t eventId)
{
    hmac_ht_self_cure_stru *p_ht_self_cure = &g_st_ht_self_cure_info;

    switch (eventId) {
        case HMAC_HT_SELF_CURE_EVENT_ASSOC:
            if (p_mac_vap->en_protocol < WLAN_HT_MODE &&
                !OAL_MEMCMP(p_ht_self_cure->htCheckInfo.checkHtUser, userMac, WLAN_MAC_ADDR_LEN)) {
                hmac_ht_self_cure_state_change(HMAC_HT_SELF_CURE_STATE_11BG_ASSOC);
                p_ht_self_cure->htCheckInfo.txDhcpTimes = 0;
            } else {
                hmac_ht_self_cure_checkinfo_init();
            }
            break;
        case HMAC_HT_SELF_CURE_EVENT_RX_DHCP_FRAME:
            hmac_ht_self_cure_checkinfo_init();
            break;
        default:
            break;
    }
}

OAL_STATIC oal_void hmac_ht_self_cure_state_11bg_assoc_handle(mac_vap_stru *p_mac_vap,
    uint8_t *userMac, uint8_t eventId)
{
    hmac_ht_self_cure_stru *p_ht_self_cure = &g_st_ht_self_cure_info;

    switch (eventId) {
        case HMAC_HT_SELF_CURE_EVENT_TX_DHCP_FRAME:
            hmac_ht_self_cure_state_change(HMAC_HT_SELF_CURE_STATE_11BG_TX_DHCP);
            p_ht_self_cure->htCheckInfo.txDhcpTimes++;
            break;
        default:
            break;
    }
}

OAL_STATIC oal_void hmac_ht_self_cure_state_11bg_tx_dhcp_handle(mac_vap_stru *p_mac_vap,
    uint8_t *userMac, uint8_t eventId)
{
    hmac_ht_self_cure_stru *p_ht_self_cure = &g_st_ht_self_cure_info;

    switch (eventId) {
        case HMAC_HT_SELF_CURE_EVENT_TX_DHCP_FRAME:
            p_ht_self_cure->htCheckInfo.txDhcpTimes++;
            if (p_ht_self_cure->htCheckInfo.txDhcpTimes > HT_SELF_CURE_CLOSE_TX_DHCP_MAX_TIMES) {
                hmac_ht_self_cure_checkinfo_init();
                OAM_WARNING_LOG0(0, OAM_SF_SCAN, "hmac_ht_self_cure_state_11bg_tx_dhcp_handle:: fail to initcheckinfo");
            }
            break;
         case HMAC_HT_SELF_CURE_EVENT_RX_DHCP_FRAME:
            if (!OAL_MEMCMP(p_ht_self_cure->htCheckInfo.checkHtUser, userMac, WLAN_MAC_ADDR_LEN)) {
                hmac_ht_self_cure_add_blacklist(userMac);
                hmac_ht_self_cure_checkinfo_init();
            }
            break;
        default:
            break;
    }
}

OAL_STATIC oal_bool_enum_uint8 hmac_ht_self_cure_is_need_check(mac_vap_stru *p_mac_vap, uint8_t *userMac)
{
        /* 非legacy sta直接退出 */
    if (!IS_LEGACY_STA(p_mac_vap)) {
        return OAL_FALSE;
    }

    if (g_st_ht_self_cure_info.needCheckFlag == OAL_TRUE) {
        return OAL_TRUE;
    }

    if (p_mac_vap->st_channel.en_band == WLAN_BAND_2G && MAC_IS_H3C_AP(userMac)) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

oal_void  hmac_ht_self_cure_event_set(mac_vap_stru *p_mac_vap, uint8_t *userMac, uint8_t eventId)
{
    hmac_ht_self_cure_stru *p_ht_self_cure = OAL_PTR_NULL;
    hmac_ht_self_cure_state_enum_uint8 nowState;

    p_ht_self_cure = &g_st_ht_self_cure_info;
    if (hmac_ht_self_cure_is_need_check(p_mac_vap, userMac) == OAL_FALSE) {
        return;
    }

    if (!hmac_ht_self_cure_is_htassoc_user(userMac)) {
        hmac_ht_self_cure_checkinfo_init();
    }

    nowState = p_ht_self_cure->htCheckInfo.htSelfCureCheckState;
    OAM_WARNING_LOG4(p_mac_vap->uc_vap_id, OAM_SF_SCAN, "hmac_ht_self_cure_event_set:Begin:nowState=%d eventId=%d \
        user:%02X:%02X", nowState, eventId, userMac[4], userMac[5]);

    switch (nowState) {
        case HMAC_HT_SELF_CURE_STATE_INIT:
            hmac_ht_self_cure_state_init_handle(p_mac_vap, userMac, eventId);
            break;
        case HMAC_HT_SELF_CURE_STATE_ASSOC:
            hmac_ht_self_cure_state_assoc_handle(p_mac_vap, userMac, eventId);
            break;
        case HMAC_HT_SELF_CURE_STATE_HT_TX_DHCP:
            hmac_ht_self_cure_state_ht_txdhcp_handle(p_mac_vap, userMac, eventId);
            break;
        case HMAC_HT_SELF_CURE_STATE_CLOSE_HT:
            hmac_ht_self_cure_state_closeht_handle(p_mac_vap, userMac, eventId);
            break;
        case HMAC_HT_SELF_CURE_STATE_11BG_ASSOC:
            hmac_ht_self_cure_state_11bg_assoc_handle(p_mac_vap, userMac, eventId);
            break;
        case HMAC_HT_SELF_CURE_STATE_11BG_TX_DHCP:
            hmac_ht_self_cure_state_11bg_tx_dhcp_handle(p_mac_vap, userMac, eventId);
            break;
        default:
            break;
    }

    OAM_WARNING_LOG4(p_mac_vap->uc_vap_id, OAM_SF_SCAN, "hmac_ht_self_cure_event_set:After:pro=%d afterState=%d \
        txDhcpTimes=%d blackListNum=%d", p_mac_vap->en_protocol, p_ht_self_cure->htCheckInfo.htSelfCureCheckState,
        p_ht_self_cure->htCheckInfo.txDhcpTimes, g_st_ht_self_cure_info.blackListNum);
}

oal_bool_enum_uint8 hmac_ht_self_cure_in_blacklist(uint8_t *userMac)
{
    uint8_t loop;
    hmac_ht_self_cure_stru *p_ht_self_cure = OAL_PTR_NULL;
    hmac_ht_blacklist_stru *p_blacklist = OAL_PTR_NULL;

    p_ht_self_cure = &g_st_ht_self_cure_info;

    if (p_ht_self_cure->htCheckInfo.htSelfCureCheckState == HMAC_HT_SELF_CURE_STATE_CLOSE_HT &&
        !OAL_MEMCMP(p_ht_self_cure->htCheckInfo.checkHtUser, userMac, WLAN_MAC_ADDR_LEN) ) {
        return OAL_TRUE;
    }

    for (loop = 0; loop < p_ht_self_cure->blackListNum; loop++) {
        p_blacklist = &p_ht_self_cure->htBlackList[loop];
        if (!OAL_MEMCMP(p_blacklist->userMacAddr, userMac, WLAN_MAC_ADDR_LEN)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

oal_void hmac_ht_self_cure_need_check_flag_set(uint8_t flag)
{
    g_st_ht_self_cure_info.needCheckFlag = flag;

    OAM_WARNING_LOG1(0, OAM_SF_SCAN, "hmac_ht_self_cure_need_check_flag_set::needCheckFlag = %d",
        g_st_ht_self_cure_info.needCheckFlag);
}
#endif

