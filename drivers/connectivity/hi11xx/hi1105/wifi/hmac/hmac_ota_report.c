

#include "hmac_ota_report.h"
#include "hmac_vap.h"
#include "hmac_tx_data.h"
#include "hmac_config.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_OTA_REPORT_C


/* 4 函数实现 */

void mac_report_beacon(mac_rx_ctl_stru *pst_rx_cb, oal_netbuf_stru *pst_netbuf)
{
    uint32_t ret;

    ret = oam_report_beacon((uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_cb),
                            pst_rx_cb->uc_mac_header_len,
                            (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_cb) + pst_rx_cb->uc_mac_header_len,
                            pst_rx_cb->us_frame_len,
                            OAM_OTA_FRAME_DIRECTION_TYPE_RX);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_WIFI_BEACON,
                         "{mac_report_beacon::oam_report_beacon return err: 0x%x.}\r\n", ret);
    }
}


uint32_t mac_report_80211_get_switch(mac_vap_stru *pst_mac_vap,
                                     mac_rx_ctl_stru *pst_rx_cb,
                                     oam_80211_frame_ctx_union *oam_frame_report_un)
{
    mac_ieee80211_frame_stru    *pst_frame_hdr;
    uint8_t                    uc_frame_type = 0;
    uint16_t                   us_user_idx = 0xffff;
    uint8_t                   *puc_da = NULL;
    uint32_t                   ret;

    pst_frame_hdr = (mac_ieee80211_frame_stru *)(mac_get_rx_cb_mac_hdr(pst_rx_cb));
    if (pst_frame_hdr == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_report_80211_get_switch::pst_frame_hdr null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((pst_frame_hdr->st_frame_control.bit_type == WLAN_CONTROL) ||
        (pst_frame_hdr->st_frame_control.bit_type == WLAN_MANAGEMENT)) {
        uc_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
    }

    if (pst_frame_hdr->st_frame_control.bit_type == WLAN_DATA_BASICTYPE) {
        uc_frame_type = OAM_USER_TRACK_FRAME_TYPE_DATA;
    }

    /* probe request 和 probe response太多，单独过滤一次 */
    if (pst_frame_hdr->st_frame_control.bit_type == WLAN_MANAGEMENT) {
        if (pst_frame_hdr->st_frame_control.bit_sub_type == WLAN_PROBE_REQ ||
            pst_frame_hdr->st_frame_control.bit_sub_type == WLAN_PROBE_RSP) {
            ret = oam_report_80211_probe_get_switch(OAM_OTA_FRAME_DIRECTION_TYPE_RX, oam_frame_report_un);
            if (ret != OAL_SUCC) {
                oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                                 "{mac_report_80211_get_switch::oam_report_80211_probe_get_switch failed.}");

                return ret;
            }

            return OAL_SUCC;
        } else if (pst_frame_hdr->st_frame_control.bit_sub_type == WLAN_DEAUTH ||
                   pst_frame_hdr->st_frame_control.bit_sub_type == WLAN_DISASOC) {
            oam_frame_report_un->frame_ctx.bit_cb = OAL_TRUE;
            oam_frame_report_un->frame_ctx.bit_dscr = OAL_TRUE;
            oam_frame_report_un->frame_ctx.bit_content = OAL_TRUE;
            return OAL_SUCC;
        } else if (pst_frame_hdr->st_frame_control.bit_sub_type == WLAN_ACTION ||
                   pst_frame_hdr->st_frame_control.bit_sub_type == WLAN_ACTION_NO_ACK) {
            oam_frame_report_un->frame_ctx.bit_cb = OAL_TRUE;
            oam_frame_report_un->frame_ctx.bit_dscr = OAL_TRUE;
            oam_frame_report_un->frame_ctx.bit_content = OAL_TRUE;
            return OAL_SUCC;
        }
    }

    mac_rx_get_da(pst_frame_hdr, &puc_da);

    if (ether_is_multicast(puc_da)) {
        ret = oam_report_80211_mcast_get_switch(OAM_OTA_FRAME_DIRECTION_TYPE_RX, uc_frame_type, oam_frame_report_un);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_RX,
                             "{mac_report_80211_get_switch::80211_mcast_get_switch failed! ret[%d], frame_type[%d]}",
                             ret, uc_frame_type);
            return ret;
        }
    } else {
        ret = mac_vap_find_user_by_macaddr(pst_mac_vap, pst_frame_hdr->auc_address2, &us_user_idx);
        if (ret == OAL_ERR_CODE_PTR_NULL) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                             "{mac_report_80211_get_switch::mac_vap_find_user_by_macaddr failed.}");
            return ret;
        }
        if (ret == OAL_FAIL) {
            oam_frame_report_un->value = 0;
            return OAL_FAIL;
        }
        ret = oam_report_80211_ucast_get_switch(OAM_OTA_FRAME_DIRECTION_TYPE_RX, uc_frame_type, oam_frame_report_un,
                                                us_user_idx);
        if (ret != OAL_SUCC) {
            oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_RX,
                "{mac_report_80211_get_switch::ucast_get_switch failed! ret[%d], type[%d], user_idx[%d]}",
                ret, uc_frame_type, us_user_idx);
            oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_RX,
                "{oam_report_80211_ucast_get_switch::frame_switch[%x], cb_switch[%x], dscr_switch[%x]",
                oam_frame_report_un->frame_ctx.bit_content, oam_frame_report_un->frame_ctx.bit_cb,
                oam_frame_report_un->frame_ctx.bit_dscr);

            return ret;
        }
    }

    return OAL_SUCC;
}


uint32_t mac_report_80211_get_user_macaddr(mac_rx_ctl_stru *pst_rx_cb, uint8_t auc_user_macaddr[])
{
    mac_ieee80211_frame_stru *pst_frame_hdr;
    uint8_t                *puc_da = NULL;

    pst_frame_hdr = (mac_ieee80211_frame_stru *)(mac_get_rx_cb_mac_hdr(pst_rx_cb));
    if (pst_frame_hdr == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_report_80211_get_user_macaddr::pst_frame_hdr null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    mac_rx_get_da(pst_frame_hdr, &puc_da);

    if (ether_is_multicast(puc_da)) {
        oal_set_mac_addr(&auc_user_macaddr[0], BROADCAST_MACADDR);
    } else {
        oal_set_mac_addr(&auc_user_macaddr[0], pst_frame_hdr->auc_address2);
    }

    return OAL_SUCC;
}


uint32_t mac_report_80211_frame(uint8_t *puc_mac_vap,
                                uint8_t *puc_rx_cb,
                                oal_netbuf_stru *pst_netbuf,
                                uint8_t *puc_des_addr,
                                oam_ota_type_enum_uint8 en_ota_type)
{
    oal_switch_enum_uint8    en_frame_switch;
    oal_switch_enum_uint8    en_cb_switch;
    oal_switch_enum_uint8    en_dscr_switch;
    uint32_t               ret;
    uint8_t                auc_user_macaddr[WLAN_MAC_ADDR_LEN] = {0};
    mac_vap_stru            *pst_mac_vap = (mac_vap_stru *)puc_mac_vap;
    mac_rx_ctl_stru         *pst_rx_cb = (mac_rx_ctl_stru *)puc_rx_cb;
    oam_80211_frame_ctx_union oam_frame_report_un = {0};

    /* 获取打印开关 */
    ret = mac_report_80211_get_switch(pst_mac_vap, pst_rx_cb, &oam_frame_report_un);
    if (ret == OAL_ERR_CODE_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_report_80211_frame::report 80211 get switch failed");
        return ret;
    }

    if (ret != OAL_SUCC) {
        return ret;
    }

    en_frame_switch = oam_frame_report_un.frame_ctx.bit_content;
    en_cb_switch = oam_frame_report_un.frame_ctx.bit_cb;
    en_dscr_switch = oam_frame_report_un.frame_ctx.bit_dscr;

    /* 获取发送端用户地址，用户SDT过滤,如果是组播\广播帧，则地址填为全F */
    ret = mac_report_80211_get_user_macaddr(pst_rx_cb, &auc_user_macaddr[0]);
    if (ret != OAL_SUCC) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_report_80211_frame::get user mac_addr failed}");
        return ret;
    }

    if ((en_ota_type == OAM_OTA_TYPE_RX_DSCR) || (en_ota_type == OAM_OTA_TYPE_RX_DSCR_PILOT)) {
        /* 上报接收帧的接收描述符 */
        if (en_dscr_switch == OAL_SWITCH_ON) {
            ret = oam_report_dscr(&auc_user_macaddr[0], puc_des_addr, (uint16_t)WLAN_RX_DSCR_SIZE, en_ota_type);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_RX, "{mac_report_80211_frame::oam_report_dscr return err:0x%x}\r\n", ret);
            }
        }
    } else {
        /* 上报接收到的帧 */
        if (en_frame_switch == OAL_SWITCH_ON) {
            ret = oam_report_80211_frame(&auc_user_macaddr[0], (uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_cb),
                pst_rx_cb->uc_mac_header_len,(uint8_t *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_cb) + \
                pst_rx_cb->uc_mac_header_len, pst_rx_cb->us_frame_len, OAM_OTA_FRAME_DIRECTION_TYPE_RX);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_RX, "{mac_report_80211_frame::oam_report_80211_frame err:0x%x}\r\n", ret);
            }
        }

        /* 上报接收帧的CB字段 */
        if (en_cb_switch == OAL_SWITCH_ON) {
            ret = oam_report_netbuf_cb(auc_user_macaddr, (uint8_t *)pst_rx_cb, sizeof(*pst_rx_cb), en_ota_type);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_RX, "{mac_report_80211_frame::oam_report_netbuf_cb error:0x%x}\r\n", ret);
            }
        }
    }

    return OAL_SUCC;
}


uint32_t mac_rx_report_80211_frame(uint8_t *pst_vap, uint8_t *pst_rx_cb, oal_netbuf_stru *pst_netbuf,
                                   oam_ota_type_enum_uint8 en_ota_type)
{
    uint8_t        uc_sub_type;
    mac_vap_stru    *pst_mac_vap = NULL;
    mac_rx_ctl_stru *pst_mac_rx_cb = NULL;

    pst_mac_vap = (mac_vap_stru *)pst_vap;
    pst_mac_rx_cb = (mac_rx_ctl_stru *)pst_rx_cb;

    uc_sub_type = mac_get_frame_type_and_subtype((uint8_t *)mac_get_rx_cb_mac_hdr(pst_mac_rx_cb));
    if (uc_sub_type == (WLAN_FC0_SUBTYPE_BEACON | WLAN_FC0_TYPE_MGT)) {
        mac_report_beacon(pst_mac_rx_cb, pst_netbuf);
    } else {
        mac_report_80211_frame((uint8_t *)pst_mac_vap,
                               (uint8_t *)pst_mac_rx_cb,
                               pst_netbuf, NULL,
                               en_ota_type);
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_DFT_STAT

uint32_t hmac_config_report_all_stat(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_ALL_STAT, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_report_all_stat::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif

