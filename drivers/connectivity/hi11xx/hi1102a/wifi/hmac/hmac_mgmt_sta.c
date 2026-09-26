
#include "wlan_mib.h"

#include "mac_frame.h"
#include "mac_ie.h"
#include "mac_regdomain.h"
#include "mac_user.h"
#include "mac_vap.h"

#include "mac_device.h"
#include "hmac_device.h"
#include "hmac_user.h"
#include "hmac_mgmt_sta.h"
#include "hmac_fsm.h"
#include "hmac_rx_data.h"
#include "hmac_chan_mgmt.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_sme_sta.h"
#include "hmac_scan.h"
#include "hmac_204080_coexist.h"
#include "hmac_support_pmf.h"
#include "hmac_tx_amsdu.h"
#include "hmac_location_ram.h"
#include "hmac_11i.h"
#include "hmac_sae.h"
#include "hmac_protection.h"
#include "hmac_ftm.h"
#include "hmac_config.h"
#include "hmac_ext_if.h"
#include "hmac_p2p.h"
#include "hmac_edca_opt.h"
#include "hmac_mgmt_ap.h"
#include "hmac_mgmt_join.h"
#include "securec.h"

#include "hmac_blockack.h"
#include "frw_ext_if.h"

#ifdef _PRE_WLAN_FEATURE_ROAM
#include "hmac_roam_main.h"
#endif  // _PRE_WLAN_FEATURE_ROAM

#ifdef _PRE_WLAN_FEATURE_WAPI
#include "hmac_wapi.h"
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
#include "hmac_wmmac.h"
#endif

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
#include "hmac_11v.h"
#endif
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include <hwnet/ipv4/sysctl_sniffer.h>
#endif
#include "plat_pm_wlan.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID       OAM_FILE_ID_HMAC_MGMT_STA_C

/* _puc_ie是指向ht cap字段的指针，故偏移5,6,7,8字节分别对应MCS四条空间流所支持的速率 */
#define is_invalid_ht_rate_hp(_puc_ie) (((_puc_ie)[5] == 0x02) && ((_puc_ie)[6] == 0x00) && \
    ((_puc_ie)[7] == 0x05) && ((_puc_ie)[8] == 0x00))

/*****************************************************************************
  2 静态函数声明
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_BTCOEX
oal_void hmac_btcoex_check_rx_same_baw_start_from_addba_req(hmac_vap_stru *pst_hmac_vap,
                                                            hmac_user_stru *pst_hmac_user,
                                                            mac_ieee80211_frame_stru *pst_frame_hdr,
                                                            oal_uint8 *puc_action,
                                                            oal_uint32 frame_body_len);
#endif

/*****************************************************************************
  3 全局变量定义
*****************************************************************************/
oal_bool_enum_uint8 g_ht_mcs_set_check = OAL_TRUE;

#if defined(_PRE_WLAN_FEATURE_HS20) || defined(_PRE_WLAN_FEATURE_P2P)

oal_uint32 hmac_sta_not_up_rx_mgmt(hmac_vap_stru *pst_hmac_vap_sta, oal_void *p_param)
{
    dmac_wlan_crx_event_stru *pst_mgmt_rx_event;
    mac_vap_stru *pst_mac_vap;
    mac_rx_ctl_stru *pst_rx_info;
    oal_uint8 *puc_mac_hdr;
#ifdef _PRE_WLAN_FEATURE_LOCATION_RAM
    oal_uint8 *puc_data;
#endif
    oal_uint8 uc_mgmt_frm_type;

    if ((pst_hmac_vap_sta == OAL_PTR_NULL) || (p_param == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_sta_not_up_rx_mgmt::PTR_NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap = &(pst_hmac_vap_sta->st_vap_base_info);
    pst_mgmt_rx_event = (dmac_wlan_crx_event_stru *)p_param;
    pst_rx_info = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_mgmt_rx_event->pst_netbuf);
    puc_mac_hdr = (oal_uint8 *)(pst_rx_info->pul_mac_hdr_start_addr);
    if (puc_mac_hdr == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_rx_info->uc_mac_vap_id, OAM_SF_RX, "{hmac_sta_not_up_rx_mgmt::puc_mac_hdr null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#ifdef _PRE_WLAN_FEATURE_LOCATION_RAM
    /* 获取帧体指针 */
    puc_data = (oal_uint8 *)pst_rx_info->pul_mac_hdr_start_addr + pst_rx_info->uc_mac_header_len;
#endif
    /* STA在NOT UP状态下接收到各种管理帧处理 */
    uc_mgmt_frm_type = mac_get_frame_sub_type(puc_mac_hdr);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    if (wlan_pm_wkup_src_debug_get() == OAL_TRUE) {
        wlan_pm_wkup_src_debug_set(OAL_FALSE);
    }
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_SNIFFER
    proc_sniffer_write_file(NULL, 0, puc_mac_hdr, pst_rx_info->us_frame_len, 0);
#endif
    switch (uc_mgmt_frm_type) {
        /* 判断接收到的管理帧类型 */
        case WLAN_FC0_SUBTYPE_PROBE_REQ:
#ifdef _PRE_WLAN_FEATURE_P2P
            /* 判断为P2P设备,则上报probe req帧到wpa_supplicant */
            if (!IS_LEGACY_VAP(pst_mac_vap)) {
                hmac_rx_mgmt_send_to_host(pst_hmac_vap_sta, pst_mgmt_rx_event->pst_netbuf);
            }
            break;
#endif
        case WLAN_FC0_SUBTYPE_ACTION | WLAN_FC0_TYPE_MGT:
            /* 如果是Action 帧，则直接上报wpa_supplicant */
#ifdef _PRE_WLAN_FEATURE_LOCATION_RAM
            if (oal_memcmp(puc_data + 2, g_auc_huawei_oui, MAC_OUI_LEN) == 0) { /* 2表示duration */
                hmac_huawei_action_process(pst_hmac_vap_sta, pst_mgmt_rx_event->pst_netbuf, puc_data[2 + MAC_OUI_LEN]);
            } else
#endif /* _PRE_WLAN_FEATURE_LOCATION */
                /* 如果是Action 帧，则直接上报wpa_supplicant */
            {
#ifdef _PRE_WLAN_FEATURE_FTM
                switch (puc_data[MAC_ACTION_OFFSET_CATEGORY]) {
                    case MAC_ACTION_CATEGORY_PUBLIC:
                        hmac_sta_not_up_rx_public_action(puc_data[MAC_ACTION_OFFSET_ACTION],
                                                         pst_hmac_vap_sta, pst_mgmt_rx_event->pst_netbuf);
                        break;
                    default:
#endif
                        hmac_rx_mgmt_send_to_host(pst_hmac_vap_sta, pst_mgmt_rx_event->pst_netbuf);
#ifdef _PRE_WLAN_FEATURE_FTM
                        break;
                }
#endif
            }
            break;
        case WLAN_FC0_SUBTYPE_PROBE_RSP:
            /* 如果是probe response帧，则直接上报wpa_supplicant */
            hmac_rx_mgmt_send_to_host(pst_hmac_vap_sta, pst_mgmt_rx_event->pst_netbuf);
            break;
        default:
            break;
    }
    return OAL_SUCC;
}
#endif /* _PRE_WLAN_FEATURE_HS20 and _PRE_WLAN_FEATURE_P2P */


OAL_STATIC oal_uint32 hmac_update_vht_opern_ie_sta(mac_vap_stru *pst_mac_vap,
                                                   hmac_user_stru *pst_hmac_user,
                                                   oal_uint8 *puc_payload,
                                                   oal_uint16 us_msg_idx)
{
    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (pst_hmac_user == OAL_PTR_NULL) ||
                     (puc_payload == OAL_PTR_NULL))) {
        oam_error_log3(0, OAM_SF_ASSOC, "{hmac_update_vht_opern_ie_sta::param null,%x %x %x.}", (uintptr_t)pst_mac_vap,
                       (uintptr_t)pst_hmac_user, (uintptr_t)puc_payload);
        return MAC_NO_CHANGE;
    }

    /* 支持11ac，才进行后续的处理 */
    if (mac_mib_get_VHTOptionImplemented(pst_mac_vap) == OAL_FALSE) {
        return MAC_NO_CHANGE;
    }

    return mac_ie_proc_vht_opern_ie(pst_mac_vap, puc_payload, &(pst_hmac_user->st_user_base_info));
}


oal_uint32 hmac_sta_up_update_edca_params_machw(hmac_vap_stru *pst_hmac_sta,
                                                mac_wmm_set_param_type_enum_uint8 en_type)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    dmac_ctx_sta_asoc_set_edca_reg_stru st_asoc_set_edca_reg_param = { 0 };

    /* 抛事件到dmac写寄存器 */
    /* 申请事件内存 */
    pst_event_mem = frw_event_alloc_m(OAL_SIZEOF(dmac_ctx_sta_asoc_set_edca_reg_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_update_vht_opern_ie_sta::event_mem alloc null, size[%d].}",
                       OAL_SIZEOF(dmac_ctx_sta_asoc_set_edca_reg_stru));
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_asoc_set_edca_reg_param.uc_vap_id = pst_hmac_sta->st_vap_base_info.uc_vap_id;
    st_asoc_set_edca_reg_param.en_set_param_type = en_type;

    /* 填写事件 */
    pst_event = (frw_event_stru *)pst_event_mem->puc_data;
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_EDCA_REG,
                       OAL_SIZEOF(dmac_ctx_sta_asoc_set_edca_reg_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_hmac_sta->st_vap_base_info.uc_chip_id,
                       pst_hmac_sta->st_vap_base_info.uc_device_id,
                       pst_hmac_sta->st_vap_base_info.uc_vap_id);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (en_type != MAC_WMM_SET_PARAM_TYPE_DEFAULT) {
        memcpy_s((oal_uint8 *)&st_asoc_set_edca_reg_param.ast_wlan_mib_qap_edac,
                 (OAL_SIZEOF(wlan_mib_Dot11QAPEDCAEntry_stru) * WLAN_WME_AC_BUTT),
                 (oal_uint8 *)&pst_hmac_sta->st_vap_base_info.pst_mib_info->st_wlan_mib_qap_edac,
                 (OAL_SIZEOF(wlan_mib_Dot11QAPEDCAEntry_stru) * WLAN_WME_AC_BUTT));
    }
#endif

    /* 拷贝参数 */
    if (memcpy_s(frw_get_event_payload(pst_event_mem), OAL_SIZEOF(dmac_ctx_sta_asoc_set_edca_reg_stru),
                 (oal_uint8 *)&st_asoc_set_edca_reg_param, OAL_SIZEOF(dmac_ctx_sta_asoc_set_edca_reg_stru)) != EOK) {
        OAM_ERROR_LOG0(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "hmac_sta_up_update_edca_params_machw::memcpy fail!");
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);

    return OAL_SUCC;
}


OAL_STATIC oal_void hmac_sta_up_update_edca_params_mib(hmac_vap_stru *pst_hmac_sta, oal_uint8 *puc_payload)
{
    oal_uint8 uc_aifsn;
    oal_uint8 uc_aci;
    oal_uint8 uc_ecwmin;
    oal_uint8 uc_ecwmax;
    oal_uint16 us_txop_limit;
    oal_bool_enum_uint8 en_acm;
    /* AC Parameters Record Format */
    /* ------------------------------------------ */
    /* |     1     |       1       |      2     | */
    /* ------------------------------------------ */
    /* | ACI/AIFSN | ECWmin/ECWmax | TXOP Limit | */
    /* ------------------------------------------ */
    /************* ACI/AIFSN Field ***************/
    /* ---------------------------------- */
    /* bit |   4   |  1  |  2  |    1     | */
    /* ---------------------------------- */
    /* | AIFSN | ACM | ACI | Reserved | */
    /* ---------------------------------- */
    uc_aifsn = puc_payload[0] & MAC_WMM_QOS_PARAM_AIFSN_MASK;
    en_acm = (puc_payload[0] & BIT4) ? OAL_TRUE : OAL_FALSE;
    uc_aci = (puc_payload[0] >> MAC_WMM_QOS_PARAM_ACI_BIT_OFFSET) & MAC_WMM_QOS_PARAM_ACI_MASK;

    /* ECWmin/ECWmax Field */
    /* ------------------- */
    /* bit |   4    |   4    | */
    /* ------------------- */
    /* | ECWmin | ECWmax | */
    /* ------------------- */
    uc_ecwmin = (puc_payload[1] & MAC_WMM_QOS_PARAM_ECWMIN_MASK);
    uc_ecwmax = ((puc_payload[1] & MAC_WMM_QOS_PARAM_ECWMAX_MASK) >> MAC_WMM_QOS_PARAM_ECWMAX_BIT_OFFSET);

    /* 在mib库中和寄存器里保存的TXOP值都是以us为单位的，但是传输的时候是以32us为
       单位进行传输的，因此在解析的时候需要将解析到的值乘以32
 */
    us_txop_limit = puc_payload[2] | /* puc_payload 2、3字节为TXOP Limit */
                    ((puc_payload[3] & MAC_WMM_QOS_PARAM_TXOPLIMIT_MASK) << MAC_WMM_QOS_PARAM_BIT_NUMS_OF_ONE_BYTE);
    us_txop_limit = (oal_uint16)(us_txop_limit << MAC_WMM_QOS_PARAM_TXOPLIMIT_SAVE_TO_TRANS_TIMES);

    /* 更新相应的MIB库信息 */
    if (uc_aci < WLAN_WME_AC_BUTT) {
        pst_hmac_sta->st_vap_base_info.pst_mib_info->st_wlan_mib_qap_edac[uc_aci].ul_dot11QAPEDCATableIndex =
            uc_aci + 1;
        pst_hmac_sta->st_vap_base_info.pst_mib_info->st_wlan_mib_qap_edac[uc_aci].ul_dot11QAPEDCATableCWmin =
            uc_ecwmin;
        pst_hmac_sta->st_vap_base_info.pst_mib_info->st_wlan_mib_qap_edac[uc_aci].ul_dot11QAPEDCATableCWmax =
            uc_ecwmax;
        pst_hmac_sta->st_vap_base_info.pst_mib_info->st_wlan_mib_qap_edac[uc_aci].ul_dot11QAPEDCATableAIFSN = uc_aifsn;
        pst_hmac_sta->st_vap_base_info.pst_mib_info->st_wlan_mib_qap_edac[uc_aci].ul_dot11QAPEDCATableTXOPLimit =
            us_txop_limit;
        pst_hmac_sta->st_vap_base_info.pst_mib_info->st_wlan_mib_qap_edac[uc_aci].en_dot11QAPEDCATableMandatory =
            en_acm;
    }
}

oal_void hmac_sta_up_update_edca_params(oal_uint8 *puc_payload,
                                        oal_uint16 us_msg_len,
                                        oal_uint16 us_info_elem_offset,
                                        hmac_vap_stru *pst_hmac_sta,
                                        oal_uint8 uc_frame_sub_type,
                                        hmac_user_stru *pst_hmac_user)
{
    oal_uint16 us_msg_offset = 0;
    oal_uint8 uc_param_set_cnt;
    oal_uint8 uc_ac_num_loop;
    oal_uint32 ul_ret;
    oal_uint8 uc_edca_param_set;
    oal_uint8 *puc_ie = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device = (mac_device_stru *)mac_res_get_dev(pst_hmac_sta->st_vap_base_info.uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ASSOC, "{hmac_sta_up_update_edca_params::mac_res_get_dev fail.device_id[%u]!}",
                         pst_hmac_sta->st_vap_base_info.uc_device_id);
        return;
    }

    /************************ WMM Parameter Element ***************************/
    /* ------------------------------------------------------------------------------ */
    /* | EID | LEN | OUI |OUI Type |OUI Subtype |Version |QoS Info |Resd |AC Params | */
    /* ------------------------------------------------------------------------------ */
    /* |  1  |  1  |  3  |    1    |     1      |    1   |    1    |  1  |    16    | */
    /* ------------------------------------------------------------------------------ */
    /******************* QoS Info field when sent from WMM AP *****************/
    /* -------------------------------------------- */
    /* | Parameter Set Count | Reserved | U-APSD | */
    /* -------------------------------------------- */
    /* bit    |        0~3          |   4~6    |   7    | */
    /* -------------------------------------------- */
    /**************************************************************************/
    if (us_info_elem_offset < us_msg_len) {
        us_msg_len -= us_info_elem_offset;
        puc_payload += us_info_elem_offset;

        puc_ie = mac_get_wmm_ie(puc_payload, us_msg_len);
        if (puc_ie != OAL_PTR_NULL) {
            /* puc_ie[1]的长度是不包括自己和之前字节的长度总长 */
            if (puc_ie[1] < HMAC_WMM_QOS_PARAMS_HDR_LEN) {
                OAM_ERROR_LOG1(0, OAM_SF_ASSOC, "{hmac_sta_up_get_edca_params::ie len error[%d]!}", puc_ie[1]);
                return;
            }

            /* 解析wmm ie是否携带EDCA参数 */
            uc_edca_param_set = puc_ie[MAC_OUISUBTYPE_WMM_PARAM_OFFSET];
            uc_param_set_cnt  = puc_ie[HMAC_WMM_QOS_PARAMS_HDR_LEN] & 0x0F;

            /* 如果收到的是beacon帧，并且param_set_count没有改变，说明AP的WMM参数没有变
               则STA也不用做任何改变，直接返回即可 */
            if ((uc_frame_sub_type == WLAN_FC0_SUBTYPE_BEACON) &&
                (uc_param_set_cnt == pst_hmac_sta->st_vap_base_info.uc_wmm_params_update_count)) {
                return;
            }

            pst_mac_device->en_wmm = OAL_TRUE;

            if (uc_frame_sub_type == WLAN_FC0_SUBTYPE_BEACON) {
                /* 保存QoS Info */
                mac_vap_set_wmm_params_update_count(&pst_hmac_sta->st_vap_base_info, uc_param_set_cnt);
            }

            if (puc_ie[HMAC_WMM_QOS_PARAMS_HDR_LEN] & BIT7) {
                mac_user_set_apsd(&(pst_hmac_user->st_user_base_info), OAL_TRUE);
            } else {
                mac_user_set_apsd(&(pst_hmac_user->st_user_base_info), OAL_FALSE);
            }

            us_msg_offset = (HMAC_WMM_QOSINFO_AND_RESV_LEN + HMAC_WMM_QOS_PARAMS_HDR_LEN);

            /* wmm ie中不携带edca参数 直接返回 */
            if (uc_edca_param_set != MAC_OUISUBTYPE_WMM_PARAM ||
                (puc_ie[1] < (HMAC_WMM_QOS_PARAMS_HDR_LEN + WLAN_WME_AC_BUTT * HMAC_WMM_AC_PARAMS_RECORD_LEN))) {
                return;
            }

            /* 针对每一个AC，更新EDCA参数 */
            for (uc_ac_num_loop = 0; uc_ac_num_loop < WLAN_WME_AC_BUTT; uc_ac_num_loop++) {
                hmac_sta_up_update_edca_params_mib(pst_hmac_sta, &puc_ie[us_msg_offset]);
#ifdef _PRE_WLAN_FEATURE_WMMAC
                if (pst_hmac_sta->st_vap_base_info.pst_mib_info->
                    st_wlan_mib_qap_edac[uc_ac_num_loop].en_dot11QAPEDCATableMandatory == 1) {
                    pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num_loop].en_ts_status = MAC_TS_INIT;
                    pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num_loop].uc_tsid = 0xFF;
                } else {
                    pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num_loop].en_ts_status = MAC_TS_NONE;
                    pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num_loop].uc_tsid = 0xFF;
                }

                oam_info_log2(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                              "{hmac_sta_up_update_edca_params::ac num[%d], ts status[%d].}",
                              uc_ac_num_loop, pst_hmac_user->st_user_base_info.st_ts_info[uc_ac_num_loop].en_ts_status);
#endif  // _PRE_WLAN_FEATURE_WMMAC
                us_msg_offset += HMAC_WMM_AC_PARAMS_RECORD_LEN;
            }

            hcc_host_update_vi_flowctl_param(pst_hmac_sta->st_vap_base_info.pst_mib_info->
                st_wlan_mib_qap_edac[WLAN_WME_AC_BE].ul_dot11QAPEDCATableCWmin,
                                             pst_hmac_sta->st_vap_base_info.pst_mib_info->
                st_wlan_mib_qap_edac[WLAN_WME_AC_VI].ul_dot11QAPEDCATableCWmin);
            /* 更新EDCA相关的MAC寄存器 */
            hmac_sta_up_update_edca_params_machw(pst_hmac_sta, MAC_WMM_SET_PARAM_TYPE_UPDATE_EDCA);

            return;
        }

        puc_ie = mac_find_ie(MAC_EID_HT_CAP, puc_payload, us_msg_len);
        if (puc_ie != OAL_PTR_NULL) {
            mac_vap_init_wme_param(&pst_hmac_sta->st_vap_base_info);
            pst_mac_device->en_wmm = OAL_TRUE;
            hcc_host_update_vi_flowctl_param(pst_hmac_sta->st_vap_base_info.pst_mib_info->
                st_wlan_mib_qap_edac[WLAN_WME_AC_BE].ul_dot11QAPEDCATableCWmin,
                                             pst_hmac_sta->st_vap_base_info.pst_mib_info->
                st_wlan_mib_qap_edac[WLAN_WME_AC_VI].ul_dot11QAPEDCATableCWmin);
            /* 更新EDCA相关的MAC寄存器 */
            hmac_sta_up_update_edca_params_machw(pst_hmac_sta, MAC_WMM_SET_PARAM_TYPE_UPDATE_EDCA);

            return;
        }
    }

    if (uc_frame_sub_type == WLAN_FC0_SUBTYPE_ASSOC_RSP) {
        /* 当与STA关联的AP不是QoS的，STA会去使能EDCA寄存器，并默认利用VO级别发送数据 */
        ul_ret = hmac_sta_up_update_edca_params_machw(pst_hmac_sta, MAC_WMM_SET_PARAM_TYPE_DEFAULT);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                             "{sta_up_update_edca_params::hmac_sta_up_update_edca_params_machw failed[%d].}", ul_ret);
        }

        pst_mac_device->en_wmm = OAL_FALSE;
    }
}

#ifdef _PRE_WLAN_FEATURE_TXOPPS

oal_uint32 hmac_sta_set_txopps_partial_aid(mac_vap_stru *pst_mac_vap)
{
    oal_uint16 us_temp_aid;
    oal_uint8 uc_temp_bssid;
    oal_uint32 ul_ret;
    mac_cfg_txop_sta_stru st_txop_info;

    /* 此处需要注意:按照协议规定(802.11ac-2013.pdf, 9.17a)，ap分配给sta的aid，不可以
       使计算出来的partial aid为0，后续如果ap支持的最大关联用户数目超过512，必须要
       对aid做这个合法性检查!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    */
    if ((pst_mac_vap->en_protocol != WLAN_VHT_MODE) && (pst_mac_vap->en_protocol != WLAN_VHT_ONLY_MODE)) {
        return OAL_SUCC;
    }

    us_temp_aid = pst_mac_vap->us_sta_aid & 0x1FF;
    /* 获取bssid auc_bssid[5]的高4位与低4位异或得到 */
    uc_temp_bssid = (pst_mac_vap->auc_bssid[5] & 0x0F) ^ ((pst_mac_vap->auc_bssid[5] & 0xF0) >> 4);
    /* 取bssid的5-8位加上temp_aid等到partial_aid，((1 << 9) - 1)是 0001 1111 1111 */
    st_txop_info.us_partial_aid = (us_temp_aid + (uc_temp_bssid << 5)) & ((1 << 9) - 1);
    st_txop_info.en_protocol = pst_mac_vap->en_protocol;

    /***************************************************************************
    抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_STA_TXOP_AID, OAL_SIZEOF(mac_cfg_txop_sta_stru),
                                    (oal_uint8 *)&st_txop_info);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TXOP,
                         "{hmac_sta_set_txopps_partial_aid::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return OAL_SUCC;
}
#endif


oal_void hmac_sta_update_mac_user_info(hmac_user_stru *pst_hmac_user_ap, oal_uint16 us_user_idx)
{
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    mac_user_stru *pst_mac_user_ap = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    if (pst_hmac_user_ap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_sta_update_mac_user_info::param null.}");
        return;
    }

    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_hmac_user_ap->st_user_base_info.uc_vap_id);
    if (oal_unlikely(pst_mac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_RX, "{hmac_sta_update_mac_user_info::get mac_vap [vap_id:%d] null.}",
                       pst_hmac_user_ap->st_user_base_info.uc_vap_id);
        return;
    }

    pst_mac_user_ap = &(pst_hmac_user_ap->st_user_base_info);

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_RX,
                     "{hmac_sta_update_mac_user_info::us_user_idx:%d,en_avail_bandwidth:%d,en_cur_bandwidth:%d}",
                     us_user_idx,
                     pst_mac_user_ap->en_avail_bandwidth,
                     pst_mac_user_ap->en_cur_bandwidth);

    ul_ret = hmac_config_user_info_syn(pst_mac_vap, pst_mac_user_ap);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_RX,
                       "{hmac_sta_update_mac_user_info::hmac_config_user_info_syn failed[%d].}", ul_ret);
    }

    ul_ret = hmac_config_user_rate_info_syn(pst_mac_vap, pst_mac_user_ap);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_RX,
                       "{hmac_sta_wait_asoc_rx::hmac_syn_rate_info failed[%d].}", ul_ret);
    }
    return;
}


oal_uint8 *hmac_sta_find_ie_in_probe_rsp(mac_vap_stru *pst_mac_vap, oal_uint8 uc_eid, oal_uint16 *pus_index)
{
    hmac_scanned_bss_info *pst_scanned_bss_info = OAL_PTR_NULL;
    hmac_bss_mgmt_stru *pst_bss_mgmt = OAL_PTR_NULL;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    mac_bss_dscr_stru *pst_bss_dscr = OAL_PTR_NULL;
    oal_uint8 *puc_ie = OAL_PTR_NULL;
    oal_uint8 *puc_payload = OAL_PTR_NULL;
    oal_uint8 us_offset;

    if (pst_mac_vap == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_SCAN, "{find ie fail, pst_mac_vap is null.}");
        return OAL_PTR_NULL;
    }

    /* 获取hmac device 结构 */
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{find ie fail, pst_hmac_device is null, dev id[%d].}",
                         pst_mac_vap->uc_device_id);
        return OAL_PTR_NULL;
    }

    /* 获取管理扫描的bss结果的结构体 */
    pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    pst_scanned_bss_info = hmac_scan_find_scanned_bss_by_bssid(pst_bss_mgmt, pst_mac_vap->auc_bssid);
    if (pst_scanned_bss_info == OAL_PTR_NULL) {
        oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{find the bss failed by bssid:%02X:XX:XX:%02X:%02X:%02X}",
                         pst_mac_vap->auc_bssid[0], pst_mac_vap->auc_bssid[3], /* 打印auc_bssid的第0、3、4、5byte */
                         pst_mac_vap->auc_bssid[4], pst_mac_vap->auc_bssid[5]);

        /* 解锁 */
        oal_spin_unlock(&(pst_bss_mgmt->st_lock));
        return OAL_PTR_NULL;
    }

    pst_bss_dscr = &(pst_scanned_bss_info->st_bss_dscr_info);
    /* 解锁 */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));

    /* 以IE开头的payload，返回供调用者使用 */
    us_offset = MAC_80211_FRAME_LEN + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    /*lint -e416*/
    puc_payload = (oal_uint8 *)(pst_bss_dscr->auc_mgmt_buff + us_offset);
    /*lint +e416*/
    if (pst_bss_dscr->ul_mgmt_len < us_offset) {
        return OAL_PTR_NULL;
    }

    puc_ie = mac_find_ie(uc_eid, puc_payload, (oal_int32)(pst_bss_dscr->ul_mgmt_len - us_offset));
    if (puc_ie == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    /* IE长度初步校验 */
    if (*(puc_ie + 1) == 0) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{IE[%d] len in probe rsp is 0, find ie fail.}", uc_eid);
        return OAL_PTR_NULL;
    }

    *pus_index = (oal_uint16)(puc_ie - puc_payload);

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{found ie[%d] in probe rsp.}", uc_eid);

    return puc_payload;
}


oal_bool_enum_uint8 hmac_is_ht_mcs_set_valid(oal_uint8 *puc_ht_cap_ie, wlan_channel_band_enum_uint8 en_band)
{
    if (g_ht_mcs_set_check == OAL_FALSE) {
        return OAL_TRUE;
    }

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID |Length |HT Capa. Info |A-MPDU Parameters |Supported MCS Set|
    -------------------------------------------------------------------------
    |1   |1      |2             |1                 |16               |
    -------------------------------------------------------------------------
    |HT Extended Cap. |Transmit Beamforming Cap. |ASEL Cap.          |
    -------------------------------------------------------------------------
    |2                |4                         |1                  |
    -------------------------------------------------------------------------
    ***************************************************************************/
    if (puc_ht_cap_ie == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_is_ht_mcs_set_valid:puc_ht_cap_ie is null");
        return OAL_FALSE;
    }

    if (puc_ht_cap_ie[1] < MAC_HT_CAP_LEN) {
        return OAL_FALSE;
    }

    if ((en_band == WLAN_BAND_2G) && is_invalid_ht_rate_hp(puc_ht_cap_ie)) {
        oam_warning_log0(0, OAM_SF_ANY, "hmac_is_ht_mcs_set_valid:it is hp printer");
        return OAL_FALSE;
    }
    /* 如果所有mcs_set（puc_ht_cap_ie的5、6、7、8字节）都为0时才返回OAL_FALSE,因为存在部分AP只支持部分速率的情况 */
    if ((puc_ht_cap_ie[5] == 0) && (puc_ht_cap_ie[6] == 0) && (puc_ht_cap_ie[7] == 0) && (puc_ht_cap_ie[8] == 0)) {
        oam_warning_log0(0, OAM_SF_ANY, "hmac_is_ht_mcs_set_valid:all mcs_set is 0");
        return OAL_FALSE;
    }
    return OAL_TRUE;
}


oal_void hmac_sta_update_ht_cap(mac_vap_stru *pst_mac_sta,
                                oal_uint8 *puc_payload,
                                mac_user_stru *pst_mac_user_ap,
                                oal_uint16 *pus_amsdu_maxsize,
                                oal_uint16 us_payload_len)
{
    oal_uint8 *puc_ie = OAL_PTR_NULL;
    oal_uint8 *puc_payload_for_ht_cap_chk = OAL_PTR_NULL;
    oal_uint16 us_ht_cap_index;
    oal_uint16 us_ht_cap_info = 0;

    if ((pst_mac_sta == OAL_PTR_NULL) || (puc_payload == OAL_PTR_NULL) || (pst_mac_user_ap == OAL_PTR_NULL)) {
        return;
    }

    puc_ie = mac_find_ie(MAC_EID_HT_CAP, puc_payload, us_payload_len);
    if ((puc_ie == OAL_PTR_NULL) || (puc_ie[1] < MAC_HT_CAP_LEN)) {
        puc_payload_for_ht_cap_chk = hmac_sta_find_ie_in_probe_rsp(pst_mac_sta, MAC_EID_HT_CAP, &us_ht_cap_index);
        if (puc_payload_for_ht_cap_chk == OAL_PTR_NULL) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_sta_update_ht_cap::puc_payload_for_ht_cap_chk is null.}");
            return;
        }

        /*lint -e413*/
        if (puc_payload_for_ht_cap_chk[us_ht_cap_index + 1] < MAC_HT_CAP_LEN) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_sta_update_ht_cap::invalid ht cap len[%d].}",
                             puc_payload_for_ht_cap_chk[us_ht_cap_index + 1]);
            return;
        }
        /*lint +e413*/
        puc_ie = puc_payload_for_ht_cap_chk + us_ht_cap_index; /* 赋值HT CAP IE */
    } else {
        if (puc_ie < puc_payload) {
            return;
        }
        us_ht_cap_index = (oal_uint16)(puc_ie - puc_payload);
        puc_payload_for_ht_cap_chk = puc_payload;
    }

    if (hmac_is_ht_mcs_set_valid(puc_ie, pst_mac_sta->st_channel.en_band) == OAL_FALSE) {
        oam_warning_log0(pst_mac_sta->uc_vap_id, OAM_SF_ASSOC, "{hmac_sta_update_ht_cap:: invalid mcs, disable HT.}");
        mac_user_set_ht_capable(pst_mac_user_ap, OAL_FALSE);
        return;
    }

    mac_user_set_ht_capable(pst_mac_user_ap, OAL_TRUE);

    /* 支持HT, 默认初始化 */
    /* 根据协议值设置特性，必须在hmac_amsdu_init_user后面调用 */
    mac_ie_proc_ht_sta(pst_mac_sta, puc_payload_for_ht_cap_chk, &us_ht_cap_index, pst_mac_user_ap, &us_ht_cap_info,
                       pus_amsdu_maxsize);
}


oal_void hmac_sta_update_ext_cap(mac_vap_stru *pst_mac_sta,
                                 mac_user_stru *pst_mac_user_ap,
                                 oal_uint8 *puc_payload,
                                 oal_uint16 us_rx_len)
{
    oal_uint8 *puc_ie;
    oal_uint8 *puc_payload_proc = OAL_PTR_NULL;
    oal_uint16 us_index;

    puc_ie = mac_find_ie(MAC_EID_EXT_CAPS, puc_payload, us_rx_len);
    if ((puc_ie == OAL_PTR_NULL) || (puc_ie[1] < MAC_MIN_XCAPS_LEN)) {
        puc_payload_proc = hmac_sta_find_ie_in_probe_rsp(pst_mac_sta, MAC_EID_EXT_CAPS, &us_index);
        if (puc_payload_proc == OAL_PTR_NULL) {
            return;
        }

        /*lint -e413*/
        if (puc_payload_proc[us_index + 1] < MAC_MIN_XCAPS_LEN) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_sta_update_ext_cap::invalid ext cap len[%d].}",
                             puc_payload_proc[us_index + 1]);
            return;
        }
        /*lint +e413*/
    } else {
        puc_payload_proc = puc_payload;
        if (puc_ie < puc_payload) {
            return;
        }

        us_index = (oal_uint16)(puc_ie - puc_payload);
    }

    /* 处理 Extended Capabilities IE */
    /*lint -e613*/
    mac_ie_proc_ext_cap_ie(pst_mac_user_ap, &puc_payload_proc[us_index]);
    /*lint +e613*/
}


oal_uint32 hmac_sta_update_ht_opern(mac_vap_stru *pst_mac_sta,
                                    mac_user_stru *pst_mac_user_ap,
                                    oal_uint8 *puc_payload,
                                    oal_uint16 us_rx_len)
{
    oal_uint8 *puc_ie;
    oal_uint8 *puc_payload_proc = OAL_PTR_NULL;
    oal_uint16 us_index;
    oal_uint32 ul_change = MAC_NO_CHANGE;

    puc_ie = mac_find_ie(MAC_EID_HT_OPERATION, puc_payload, us_rx_len);
    if ((puc_ie == OAL_PTR_NULL) || (puc_ie[1] < MAC_HT_OPERN_LEN)) {
        puc_payload_proc = hmac_sta_find_ie_in_probe_rsp(pst_mac_sta, MAC_EID_HT_OPERATION, &us_index);
        if (puc_payload_proc == OAL_PTR_NULL) {
            return ul_change;
        }

        /*lint -e413*/
        if (puc_payload_proc[us_index + 1] < MAC_HT_OPERN_LEN) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_sta_update_ht_opern::invalid ht cap len[%d].}",
                             puc_payload_proc[us_index + 1]);
            return ul_change;
        }
        /*lint +e413*/
    } else {
        puc_payload_proc = puc_payload;
        if (puc_ie < puc_payload) {
            return ul_change;
        }

        us_index = (oal_uint16)(puc_ie - puc_payload);
    }

    ul_change |= mac_proc_ht_opern_ie(pst_mac_sta, &puc_payload_proc[us_index], pst_mac_user_ap);
    /* 只针对 HT Operation中的Secondary Channel Offset进行处理 */
    ul_change |= mac_ie_proc_sec_chan_offset_2040(pst_mac_sta, puc_payload[us_index + MAC_IE_HDR_LEN + 1] & 0x3);

    return ul_change;
}


oal_uint32 hmac_update_ht_sta(mac_vap_stru *pst_mac_sta,
                              oal_uint8 *puc_payload,
                              oal_uint16 us_offset,
                              oal_uint16 us_rx_len,
                              mac_user_stru *pst_mac_user_ap,
                              oal_uint16 *pus_amsdu_maxsize)
{
    oal_uint32 ul_change = MAC_NO_CHANGE;
    oal_uint8 *puc_ie_payload_start = OAL_PTR_NULL;
    oal_uint16 us_ie_payload_len;

    if ((pst_mac_sta == OAL_PTR_NULL) || (puc_payload == OAL_PTR_NULL) || (pst_mac_user_ap == OAL_PTR_NULL)) {
        return ul_change;
    }

    /* 初始化HT cap为FALSE，入网时会把本地能力跟随AP能力 */
    mac_user_set_ht_capable(pst_mac_user_ap, OAL_FALSE);

    /* 至少支持11n才进行后续的处理 */
    if (mac_mib_get_HighThroughputOptionImplemented(pst_mac_sta) == OAL_FALSE) {
        return ul_change;
    }

    puc_ie_payload_start = puc_payload + us_offset;
    us_ie_payload_len = us_rx_len - us_offset;

    hmac_sta_update_ht_cap(pst_mac_sta, puc_ie_payload_start, pst_mac_user_ap, pus_amsdu_maxsize, us_ie_payload_len);

    hmac_sta_update_ext_cap(pst_mac_sta, pst_mac_user_ap, puc_ie_payload_start, us_ie_payload_len);

    ul_change = hmac_sta_update_ht_opern(pst_mac_sta, pst_mac_user_ap, puc_ie_payload_start, us_ie_payload_len);

    return ul_change;
}


oal_void hmac_add_and_clear_repeat_op_rates(oal_uint8 *puc_ie_rates, oal_uint8 uc_ie_num_rates,
                                            mac_rate_stru *pst_op_rates)
{
    oal_uint8 uc_ie_rates_idx;
    oal_uint8 uc_user_rates_idx;

    for (uc_ie_rates_idx = 0; uc_ie_rates_idx < uc_ie_num_rates; uc_ie_rates_idx++) {
        /* 判断该速率是否已经记录在op中 */
        for (uc_user_rates_idx = 0; uc_user_rates_idx < pst_op_rates->uc_rs_nrates; uc_user_rates_idx++) {
            if (is_equal_rates(puc_ie_rates[uc_ie_rates_idx], pst_op_rates->auc_rs_rates[uc_user_rates_idx])) {
                break;
            }
        }

        /* 相等时，说明ie中的速率与op中的速率都不相同，可以加入op的速率集中 */
        if (uc_user_rates_idx == pst_op_rates->uc_rs_nrates) {
            /* 当长度超出限制时告警，不加入op rates中 */
            if (pst_op_rates->uc_rs_nrates == WLAN_MAX_SUPP_RATES) {
                oam_warning_log0(0, OAM_SF_ANY,
                                 "{user option rates more then WLAN_USER_MAX_SUPP_RATES.}");
                break;
            }
            pst_op_rates->auc_rs_rates[pst_op_rates->uc_rs_nrates++] = puc_ie_rates[uc_ie_rates_idx];
        }
    }
}


OAL_STATIC oal_void hmac_ie_proc_assoc_user_legacy_rate(
    oal_uint8 *puc_payload, oal_uint16 us_offset, oal_uint16 us_rx_len, hmac_user_stru *pst_hmac_user)
{
    oal_uint8 *puc_ie;
    oal_uint8 uc_num_rates = 0;
    oal_uint8 uc_num_ex_rates = 0;

    puc_ie = mac_find_ie(MAC_EID_RATES, puc_payload + us_offset, us_rx_len - us_offset);
    if (puc_ie != OAL_PTR_NULL) {
        uc_num_rates = puc_ie[1];

        if (uc_num_rates < MAC_MIN_RATES_LEN) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_ie_proc_assoc_user_legacy_rate:: invaild rates:%d}", uc_num_rates);
        } else {
            hmac_add_and_clear_repeat_op_rates(puc_ie + MAC_IE_HDR_LEN, uc_num_rates, &(pst_hmac_user->st_op_rates));
        }
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ie_proc_assoc_user_legacy_rate::unsupport basic rates}");
    }

    puc_ie = mac_find_ie(MAC_EID_XRATES, puc_payload + us_offset, us_rx_len - us_offset);
    if (puc_ie != OAL_PTR_NULL) {
        uc_num_ex_rates = puc_ie[1];

        if (uc_num_ex_rates < MAC_MIN_XRATE_LEN) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_ie_proc_assoc_user_legacy_rate:: invaild xrates:%d}",
                             uc_num_ex_rates);
            return;
        } else {
            hmac_add_and_clear_repeat_op_rates(puc_ie + MAC_IE_HDR_LEN, uc_num_ex_rates,
                                               &(pst_hmac_user->st_op_rates));
        }
    }

    if (pst_hmac_user->st_op_rates.uc_rs_nrates == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ie_proc_assoc_user_legacy_rate::rate is 0.}");
        pst_hmac_user->st_op_rates.uc_rs_nrates = 2;
        pst_hmac_user->st_op_rates.auc_rs_rates[0] = 0x82;
        pst_hmac_user->st_op_rates.auc_rs_rates[1] = 0x0C;
        return;
    }
    return;
}
OAL_STATIC oal_void hmac_assoc_bw_change(hmac_vap_stru *pst_hmac_sta, mac_vap_stru *pst_mac_vap,
    oal_uint32 ul_change)
{
    /* 评估是否需要进行带宽切换 */
    if (MAC_BW_CHANGE & ul_change) {
        oam_warning_log3(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_process_assoc_rsp::change BW. ul_change[0x%x], uc_channel[%d], en_bandwidth[%d].}",
                         ul_change,
                         pst_mac_vap->st_channel.uc_chan_number,
                         pst_mac_vap->st_channel.en_bandwidth);
        hmac_chan_sync(pst_mac_vap,
                       pst_mac_vap->st_channel.uc_chan_number,
                       pst_mac_vap->st_channel.en_bandwidth,
                       OAL_TRUE);
    }
}
OAL_STATIC oal_uint32 hmac_process_vendor_vht_ie(hmac_vap_stru *pst_hmac_sta, hmac_user_stru *pst_hmac_user,
    oal_uint16 us_offset, oal_uint8 *puc_payload, oal_uint16 us_msg_len)
{
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    oal_uint8 *puc_tmp_ie = OAL_PTR_NULL;
    oal_uint32 ul_change = MAC_NO_CHANGE;
    oal_uint8 *puc_vendor_vht_ie = OAL_PTR_NULL;
    oal_uint32 ul_vendor_vht_ie_offset = MAC_WLAN_OUI_VENDOR_VHT_HEADER + MAC_IE_HDR_LEN;

    pst_mac_vap = &(pst_hmac_sta->st_vap_base_info);
#ifdef _PRE_WLAN_FEATURE_TXBF
    /* 更新11n txbf能力 */
    puc_tmp_ie = mac_find_vendor_ie(MAC_HUAWEI_VENDER_IE, MAC_EID_11NTXBF, puc_payload + us_offset,
                                    us_msg_len - us_offset);
    hmac_mgmt_update_11ntxbf_cap(puc_tmp_ie, pst_hmac_user);
#endif

    /* 更新11ac VHT capabilities ie */
    memset_s(&(pst_hmac_user->st_user_base_info.st_vht_hdl), OAL_SIZEOF(mac_vht_hdl_stru),
             0, OAL_SIZEOF(mac_vht_hdl_stru));
    puc_tmp_ie = mac_find_ie(MAC_EID_VHT_CAP, puc_payload + us_offset, us_msg_len - us_offset);
    if (puc_tmp_ie != OAL_PTR_NULL) {
        hmac_proc_vht_cap_ie(pst_mac_vap, pst_hmac_user, puc_tmp_ie);
    }

    /* 更新11ac VHT operation ie */
    puc_tmp_ie = mac_find_ie(MAC_EID_VHT_OPERN, puc_payload + us_offset, us_msg_len - us_offset);
    if (puc_tmp_ie != OAL_PTR_NULL) {
        ul_change |= hmac_update_vht_opern_ie_sta(pst_mac_vap, pst_hmac_user, puc_tmp_ie, us_offset);
    }

    /* 根据BRCM VENDOR OUI 适配2G 11AC */
    if (pst_hmac_user->st_user_base_info.st_vht_hdl.en_vht_capable == OAL_FALSE) {
        puc_vendor_vht_ie = mac_find_vendor_ie(MAC_WLAN_OUI_BROADCOM_EPIGRAM, MAC_WLAN_OUI_VENDOR_VHT_TYPE,
                                               puc_payload + us_offset, us_msg_len - us_offset);
        if ((puc_vendor_vht_ie != OAL_PTR_NULL) && (puc_vendor_vht_ie[1] >= ul_vendor_vht_ie_offset)) {
            oam_warning_log0(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                             "{hmac_process_assoc_rsp::find broadcom/epigram vendor ie, enable hidden bit_11ac2g}");

            /* 进入此函数代表user支持2G 11ac */
            puc_tmp_ie = mac_find_ie(MAC_EID_VHT_CAP, puc_vendor_vht_ie + ul_vendor_vht_ie_offset,
                                     (oal_int32)(puc_vendor_vht_ie[1] - MAC_WLAN_OUI_VENDOR_VHT_HEADER));
            if (puc_tmp_ie != OAL_PTR_NULL) {
                hmac_proc_vht_cap_ie(pst_mac_vap, pst_hmac_user, puc_tmp_ie);
            }

            /* 更新11ac VHT operation ie */
            puc_tmp_ie = mac_find_ie(MAC_EID_VHT_OPERN, puc_vendor_vht_ie + ul_vendor_vht_ie_offset,
                                     (oal_int32)(puc_vendor_vht_ie[1] - MAC_WLAN_OUI_VENDOR_VHT_HEADER));
            if (puc_tmp_ie != OAL_PTR_NULL) {
                ul_change |= hmac_update_vht_opern_ie_sta(pst_mac_vap, pst_hmac_user, puc_tmp_ie, 0);
            }
        }
    }
    return ul_change;
}
OAL_STATIC oal_void hmac_process_vendor_vht(hmac_vap_stru *pst_hmac_sta, hmac_user_stru *pst_hmac_user,
    oal_uint16 us_offset, oal_uint8 *puc_payload, oal_uint16 us_msg_len)
{
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    oal_uint32 ul_change = MAC_NO_CHANGE;

    pst_mac_vap = &(pst_hmac_sta->st_vap_base_info);
    /* 更新 HT 参数 */
    ul_change |= hmac_update_ht_sta(&pst_hmac_sta->st_vap_base_info, puc_payload, us_offset, us_msg_len,
                                    &pst_hmac_user->st_user_base_info, &pst_hmac_user->us_amsdu_maxsize);
    if (hmac_user_ht_support(pst_hmac_user) == OAL_TRUE) {
        ul_change |= hmac_process_vendor_vht_ie(pst_hmac_sta, pst_hmac_user, us_offset, puc_payload, us_msg_len);
        hmac_assoc_bw_change(pst_hmac_sta, pst_mac_vap, ul_change);
    }
}
OAL_STATIC oal_void hmac_config_to_dmac_syn(hmac_vap_stru *pst_hmac_sta, hmac_user_stru *pst_hmac_user)
{
    oal_uint32 ul_ret;
    ul_ret = hmac_config_user_cap_syn(&(pst_hmac_sta->st_vap_base_info), &pst_hmac_user->st_user_base_info);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_process_assoc_rsp::hmac_config_usr_cap_syn failed[%d].}", ul_ret);
    }

    ul_ret = hmac_config_user_info_syn(&(pst_hmac_sta->st_vap_base_info), &pst_hmac_user->st_user_base_info);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_process_assoc_rsp::hmac_syn_vap_state failed[%d].}", ul_ret);
    }

    ul_ret = hmac_config_user_rate_info_syn(&(pst_hmac_sta->st_vap_base_info), &pst_hmac_user->st_user_base_info);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_process_assoc_rsp::hmac_syn_rate_info failed[%d].}", ul_ret);
    }

    /* dmac offload架构下，同步STA USR信息到dmac */
    ul_ret = hmac_config_sta_vap_info_syn(&(pst_hmac_sta->st_vap_base_info));
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_process_assoc_rsp::hmac_syn_vap_state failed[%d].}", ul_ret);
    }
}

oal_uint32 hmac_process_assoc_rsp(hmac_vap_stru *pst_hmac_sta, hmac_user_stru *pst_hmac_user,
                                  oal_uint8 *puc_mac_hdr, oal_uint8 *puc_payload, oal_uint16 us_msg_len)
{
    oal_uint32 ul_rslt;
    oal_uint16 us_offset;
    oal_uint16 us_aid;
    wlan_bw_cap_enum_uint8 en_bandwidth_cap;
    oal_uint32 ul_ret;
    mac_vap_stru *pst_mac_vap;
    wlan_bw_cap_enum_uint8 en_bwcap;

    pst_mac_vap = &(pst_hmac_sta->st_vap_base_info);
    /* 更新关联ID */
    us_aid = mac_get_asoc_id(puc_payload);
    if ((us_aid > 0) && (us_aid <= 2007)) { /* 关联ID 1-2007 */
        mac_vap_set_aid(&pst_hmac_sta->st_vap_base_info, us_aid);
#ifdef _PRE_WLAN_FEATURE_TXOPPS
        /* sta计算自身的partial aid，保存到vap结构中，并写入到mac寄存器 */
        hmac_sta_set_txopps_partial_aid(&pst_hmac_sta->st_vap_base_info);
#endif
    } else {
        OAM_WARNING_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_process_assoc_rsp::invalid us_sta_aid[%d].}", us_aid);
    }
    us_offset = MAC_CAP_INFO_LEN + MAC_STATUS_CODE_LEN + MAC_AID_LEN;

    /* 初始化安全端口过滤参数 */
#if defined(_PRE_WLAN_FEATURE_WPA) || defined(_PRE_WLAN_FEATURE_WPA2)
    ul_rslt = hmac_init_user_security_port(&(pst_hmac_sta->st_vap_base_info), &(pst_hmac_user->st_user_base_info));
    if (ul_rslt != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_process_assoc_rsp::hmac_init_user_security_port failed[%d].}", ul_rslt);
    }

#endif

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    /* STA模式下的pmf能力来源于WPA_supplicant，只有启动pmf和不启动pmf两种类型 */
    mac_user_set_pmf_active(&pst_hmac_user->st_user_base_info, pst_mac_vap->en_user_pmf_cap);
#endif /* #if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT) */

    /* sta更新自身的edca parameters */
    hmac_sta_up_update_edca_params(puc_payload, us_msg_len, us_offset, pst_hmac_sta,
                                   mac_get_frame_sub_type(puc_mac_hdr), pst_hmac_user);

    /* 更新关联用户的 QoS protocol table */
    hmac_mgmt_update_assoc_user_qos_table(puc_payload, us_msg_len, us_offset, pst_hmac_user);

    /* 更新关联用户的legacy速率集合 */
    hmac_user_init_rates(pst_hmac_user);
    hmac_ie_proc_assoc_user_legacy_rate(puc_payload, us_offset, us_msg_len, pst_hmac_user);

    /* 初始化设置对端 HT 能力不使能 */
    mac_user_set_ht_capable(&(pst_hmac_user->st_user_base_info), OAL_FALSE);
    /* 初始化设置对端VHT 能力不使能 */
    mac_user_set_vht_capable(&(pst_hmac_user->st_user_base_info), OAL_FALSE);
    /* 更新 HT 参数 */
    hmac_process_vendor_vht(pst_hmac_sta, pst_hmac_user, us_offset, puc_payload, us_msg_len);
    /* 获取用户的协议模式 */
    hmac_set_user_protocol_mode(pst_mac_vap, pst_hmac_user);

    if (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) {
        pst_hmac_sta->en_rx_ampduplusamsdu_active = OAL_FALSE;
    } else {
        pst_hmac_sta->en_rx_ampduplusamsdu_active = g_uc_host_rx_ampdu_amsdu;
    }

    /* 获取用户与VAP协议模式交集 */
    mac_user_set_avail_protocol_mode(&pst_hmac_user->st_user_base_info,
                                     g_auc_avail_protocol_mode[pst_mac_vap->en_protocol]
                                     [pst_hmac_user->st_user_base_info.en_protocol_mode]);
    mac_user_set_cur_protocol_mode(&pst_hmac_user->st_user_base_info,
                                   pst_hmac_user->st_user_base_info.en_avail_protocol_mode);

    /* 获取用户和VAP 可支持的11a/b/g 速率交集 */
    hmac_vap_set_user_avail_rates(&(pst_hmac_sta->st_vap_base_info), pst_hmac_user);

    /* 获取用户与VAP带宽能力交集 */
    /* 获取用户的带宽能力 */
    mac_user_get_ap_opern_bandwidth(&(pst_hmac_user->st_user_base_info), &en_bandwidth_cap);

    mac_vap_get_bandwidth_cap(pst_mac_vap, &en_bwcap);
    en_bwcap = oal_min(en_bwcap, en_bandwidth_cap);
    mac_user_set_bandwidth_info(&pst_hmac_user->st_user_base_info, en_bwcap, en_bwcap);

    oam_warning_log3(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                     "{hmac_process_assoc_rsp::mac user[%d] en_bandwidth_cap:%d,en_avail_bandwidth:%d}",
                     pst_hmac_user->st_user_base_info.us_assoc_id,
                     en_bandwidth_cap, pst_hmac_user->st_user_base_info.en_avail_bandwidth);

    /* 获取用户与VAP空间流交集 */
    ul_ret = hmac_user_set_avail_num_space_stream(&(pst_hmac_user->st_user_base_info),
                                                  pst_hmac_sta->st_vap_base_info.en_vap_rx_nss);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_process_assoc_rsp::mac_user_set_avail_num_space_stream failed[%d].}", ul_ret);
    }

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY

    /* 处理Operating Mode Notification 信息元素 */
    ul_ret = hmac_check_opmode_notify(pst_hmac_sta, puc_mac_hdr, puc_payload, us_offset, us_msg_len, pst_hmac_user);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_process_assoc_rsp::hmac_check_opmode_notify failed[%d].}", ul_ret);
    }

#endif

    mac_user_set_asoc_state(&pst_hmac_user->st_user_base_info, MAC_USER_STATE_ASSOC);

    /* dmac offload架构下，同步STA USR信息到dmac */
    hmac_config_to_dmac_syn(pst_hmac_sta, pst_hmac_user);
    pst_hmac_sta->ul_assoc_timestamp = oal_time_get_stamp_ms();
    return OAL_SUCC;
}
OAL_STATIC oal_void hmac_btcoex_init(hmac_vap_stru *pst_hmac_sta, oal_uint8 *auc_addr_sa, oal_uint32 ul_addr_sa_len,
    hmac_user_stru *pst_hmac_user_ap)
{
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* 关联上用户之后，初始化黑名单方案 */
    hmac_btcoex_blacklist_handle_init(pst_hmac_user_ap);

    if (hmac_btcoex_check_exception_in_list(pst_hmac_sta, auc_addr_sa) == OAL_TRUE) {
        if (hmac_btcoex_get_blacklist_type(pst_hmac_user_ap) == BTCOEX_BLACKLIST_TPYE_FIX_BASIZE) {
            oam_warning_log0(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_COEX,
                             "{hmac_sta_wait_asoc_rx::mac_addr in blacklist.}");
            pst_hmac_user_ap->st_hmac_user_btcoex.st_hmac_btcoex_addba_req.en_ba_handle_allow = OAL_FALSE;
        } else {
            pst_hmac_user_ap->st_hmac_user_btcoex.st_hmac_btcoex_addba_req.en_ba_handle_allow = OAL_TRUE;
        }
    } else {
        /* 初始允许建立聚合，两个方案保持对齐 */
        pst_hmac_user_ap->st_hmac_user_btcoex.st_hmac_btcoex_addba_req.en_ba_handle_allow = OAL_TRUE;
    }
#endif
}

OAL_STATIC oal_void hmac_set_state_assoc(hmac_vap_stru *pst_hmac_sta, hmac_user_stru *pst_hmac_user_ap)
{
#ifdef _PRE_DEBUG_MODE_USER_TRACK
    mac_user_change_info_event(pst_hmac_user_ap->st_user_base_info.auc_user_mac_addr,
                               pst_hmac_sta->st_vap_base_info.uc_vap_id,
                               pst_hmac_user_ap->st_user_base_info.en_user_asoc_state,
                               MAC_USER_STATE_ASSOC, OAM_MODULE_ID_HMAC,
                               OAM_USER_INFO_CHANGE_TYPE_ASSOC_STATE);
#endif
}
OAL_STATIC oal_void hmac_fill_assoc_resp(hmac_vap_stru *pst_hmac_sta, hmac_asoc_rsp_stru *pst_asoc_rsp,
    oal_uint8 *puc_mac_hdr, oal_uint16 us_msg_len)
{
    /* 准备消息，上报给APP */
    pst_asoc_rsp->en_result_code = HMAC_MGMT_SUCCESS;
    pst_asoc_rsp->en_status_code = MAC_SUCCESSFUL_STATUSCODE;

    /* 记录关联响应帧的部分内容，用于上报给内核 */
    pst_asoc_rsp->ul_asoc_rsp_ie_len = us_msg_len - OAL_ASSOC_RSP_FIXED_OFFSET; /* 除去MAC帧头24字节和FIXED部分6字节 */
    pst_asoc_rsp->puc_asoc_rsp_ie_buff = puc_mac_hdr + OAL_ASSOC_RSP_IE_OFFSET;

    /* 获取AP的mac地址 */
    mac_get_bssid(puc_mac_hdr, pst_asoc_rsp->auc_addr_ap);

    /* 获取关联请求帧信息 */
    pst_asoc_rsp->puc_asoc_req_ie_buff = pst_hmac_sta->puc_asoc_req_ie_buff;
    pst_asoc_rsp->ul_asoc_req_ie_len = pst_hmac_sta->ul_asoc_req_ie_len;

    hmac_send_rsp_to_sme_sta (pst_hmac_sta, HMAC_SME_ASOC_RSP, (oal_uint8 *)(pst_asoc_rsp));
}
OAL_STATIC oal_void hmac_wakeup_src_type_print(hmac_vap_stru *pst_hmac_sta, oal_uint8 uc_frame_sub_type)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    if (wlan_pm_wkup_src_debug_get() == OAL_TRUE) {
        wlan_pm_wkup_src_debug_set(OAL_FALSE);
        OAM_WARNING_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                         "{wifi_wake_src:hmac_sta_wait_asoc_rx_etc::wakeup mgmt type[0x%x]}", uc_frame_sub_type);
    }
#endif
#endif
}

oal_uint32 hmac_sta_wait_asoc_rx(hmac_vap_stru *pst_hmac_sta, oal_void *pst_msg)
{
    mac_status_code_enum_uint16 en_asoc_status;
    oal_uint8 uc_frame_sub_type;
    dmac_wlan_crx_event_stru *pst_mgmt_rx_event = OAL_PTR_NULL;
    dmac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;
    mac_rx_ctl_stru *pst_rx_info = OAL_PTR_NULL;
    oal_uint8 *puc_mac_hdr = OAL_PTR_NULL;
    oal_uint8 *puc_payload = OAL_PTR_NULL;
    oal_uint16 us_msg_len;
    hmac_asoc_rsp_stru st_asoc_rsp;
    oal_uint8 auc_addr_sa[WLAN_MAC_ADDR_LEN];
    oal_uint16 us_user_idx;
    oal_uint32 ul_rslt;
    hmac_user_stru *pst_hmac_user_ap = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    mac_cfg_user_info_param_stru st_hmac_user_info_event;

    if ((pst_hmac_sta == OAL_PTR_NULL) || (pst_msg == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ASSOC, "{hmac_sta_wait_asoc_rx::param null,%x %x.}", (uintptr_t)pst_hmac_sta,
                       (uintptr_t)pst_msg);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &(pst_hmac_sta->st_vap_base_info);

    pst_mgmt_rx_event = (dmac_wlan_crx_event_stru *)pst_msg;
    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_mgmt_rx_event->pst_netbuf);
    pst_rx_info = (mac_rx_ctl_stru *)(&(pst_rx_ctrl->st_rx_info));
    puc_mac_hdr = (oal_uint8 *)(pst_rx_info->pul_mac_hdr_start_addr);
    puc_payload = (oal_uint8 *)(puc_mac_hdr) + pst_rx_info->uc_mac_header_len;
    us_msg_len = pst_rx_info->us_frame_len - pst_rx_info->uc_mac_header_len; /* 消息总长度,不包括FCS */

    uc_frame_sub_type = mac_get_frame_sub_type(puc_mac_hdr);
    en_asoc_status = mac_get_asoc_status(puc_payload);
    hmac_wakeup_src_type_print(pst_hmac_sta, uc_frame_sub_type);

    if ((uc_frame_sub_type != WLAN_FC0_SUBTYPE_ASSOC_RSP) &&
        (uc_frame_sub_type != WLAN_FC0_SUBTYPE_REASSOC_RSP)) {
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_SNIFFER
    proc_sniffer_write_file(NULL, 0, puc_mac_hdr, pst_rx_info->us_frame_len, 0);
#endif
    if (en_asoc_status != MAC_SUCCESSFUL_STATUSCODE) {
        OAM_WARNING_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_sta_wait_asoc_rx:: AP refuse STA assoc reason=%d.}", en_asoc_status);
#ifdef _PRE_WLAN_1102A_CHR
        if (en_asoc_status != MAC_AP_FULL) {
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                                 CHR_WIFI_DRV_EVENT_CONNECT, CHR_WIFI_DRV_ERROR_ASSOC_REJECTED);
        }
#endif

        pst_hmac_sta->st_mgmt_timetout_param.en_status_code = en_asoc_status;

        return OAL_FAIL;
    }

    if (us_msg_len < OAL_ASSOC_RSP_FIXED_OFFSET) {
        OAM_ERROR_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_sta_wait_asoc_rx::asoc_rsp_body is too short(%d) to going on!}",
                       us_msg_len);
        return OAL_FAIL;
    }

    /* 获取SA 地址 */
    mac_get_address2(puc_mac_hdr, auc_addr_sa);

    /* 根据SA 找到对应AP USER结构 */
    ul_rslt = mac_vap_find_user_by_macaddr(&(pst_hmac_sta->st_vap_base_info), auc_addr_sa, &us_user_idx);
    if (ul_rslt != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_sta_wait_asoc_rx:: mac_vap_find_user_by_macaddr failed[%d].}", ul_rslt);

        return ul_rslt;
    }

    /* 获取STA关联的AP的用户指针 */
    pst_hmac_user_ap = mac_res_get_hmac_user(us_user_idx);
    if (pst_hmac_user_ap == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                         "{hmac_sta_wait_asoc_rx::pst_hmac_user_ap[%d] null.}", us_user_idx);
        return OAL_FAIL;
    }

    /* 取消定时器 */
    frw_immediate_destroy_timer(&(pst_hmac_sta->st_mgmt_timer));

    ul_ret = hmac_process_assoc_rsp(pst_hmac_sta, pst_hmac_user_ap, puc_mac_hdr, puc_payload, us_msg_len);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_sta_wait_asoc_rx::hmac_process_assoc_rsp failed[%d].}", ul_ret);
        return ul_ret;
    }
    hmac_btcoex_init(pst_hmac_sta, auc_addr_sa, OAL_SIZEOF(auc_addr_sa), pst_hmac_user_ap);
    /* STA切换到UP状态 */
    hmac_fsm_change_state(pst_hmac_sta, MAC_VAP_STATE_UP);

    /* user已经关联上，抛事件给DMAC，在DMAC层挂用户算法钩子 */
    hmac_user_add_notify_alg(&(pst_hmac_sta->st_vap_base_info), us_user_idx);

#ifdef _PRE_WLAN_FEATURE_ROAM
    hmac_roam_info_init(pst_hmac_sta);
#endif  // _PRE_WLAN_FEATURE_ROAM

    /* 将用户(AP)在本地的状态信息设置为已关联状态 */
    hmac_set_state_assoc(pst_hmac_sta, pst_hmac_user_ap);

    /* 准备消息，上报给APP */
    hmac_fill_assoc_resp(pst_hmac_sta, &st_asoc_rsp, puc_mac_hdr, us_msg_len);

    /* 1102 STA 入网后，上报VAP 信息和用户信息 */
    st_hmac_user_info_event.us_user_idx = us_user_idx;

    hmac_config_vap_info(pst_mac_vap, OAL_SIZEOF(oal_uint32), (oal_uint8 *)&ul_ret);
    hmac_config_user_info(pst_mac_vap, OAL_SIZEOF(mac_cfg_user_info_param_stru),
                          (oal_uint8 *)&st_hmac_user_info_event);

    return OAL_SUCC;
}


oal_uint32 hmac_get_auth_assoc_send_status_report_flag(
    hmac_vap_stru *pst_hmac_vap, mac_cfg_query_mgmt_send_status_stru *pst_sendstat_info)
{
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = &pst_hmac_vap->st_vap_base_info;

    if (!IS_LEGACY_STA(pst_mac_vap)) {
        return OAL_FAIL;
    }
    if (pst_sendstat_info == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_sta_get_mngpkt_sendstat::pst_sendstat_info null.}");
        return OAL_FAIL;
    }
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_sta_get_mngpkt_sendstat::mac_res_get_dev failed.}");
        return OAL_FAIL;
    }
    if (pst_mac_device->en_report_mgmt_req_status == OAL_FALSE) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_sta_get_mngpkt_sendstat::get auth or assoc req status failed.If send success, NO PROBLEM!}");
        return OAL_FAIL;
    } else {
        pst_sendstat_info->uc_auth_req_st = pst_mac_device->uc_auth_req_sendst;
        pst_sendstat_info->uc_asoc_req_st = pst_mac_device->uc_asoc_req_sendst;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_sta_auth_timeout(hmac_vap_stru *pst_hmac_sta, oal_void *p_param)
{
    hmac_auth_rsp_stru st_auth_rsp = {
        {
            0,
        },
    };
    mac_cfg_query_mgmt_send_status_stru st_send_state;

    /* and send it to the host. */
    if (pst_hmac_sta->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2) {
        st_auth_rsp.us_status_code = MAC_AUTH_RSP2_TIMEOUT;
    } else if (pst_hmac_sta->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4) {
        st_auth_rsp.us_status_code = MAC_AUTH_RSP4_TIMEOUT;
    } else {
        st_auth_rsp.us_status_code = HMAC_MGMT_TIMEOUT;
    }

    if (hmac_get_auth_assoc_send_status_report_flag(pst_hmac_sta, &st_send_state) == OAL_SUCC) {
        if (st_send_state.uc_auth_req_st > 0) {
            st_auth_rsp.us_status_code = MAC_AUTH_REQ_SEND_FAIL_BEGIN + st_send_state.uc_auth_req_st;
            hmac_set_auth_assoc_send_status_report_flag(pst_hmac_sta, OAL_FALSE);
        }
    }

    /* Send the response to host now. */
    hmac_send_rsp_to_sme_sta(pst_hmac_sta, HMAC_SME_AUTH_RSP, (oal_uint8 *)&st_auth_rsp);

    return OAL_SUCC;
}


wlan_channel_bandwidth_enum_uint8 hmac_sta_get_band(wlan_bw_cap_enum_uint8 en_dev_cap,
                                                    wlan_channel_bandwidth_enum_uint8 en_bss_cap)
{
    wlan_channel_bandwidth_enum_uint8 en_band;

    en_band = WLAN_BAND_WIDTH_20M;

    if ((en_dev_cap == WLAN_BW_CAP_80M) &&
        (en_bss_cap >= WLAN_BAND_WIDTH_80PLUSPLUS)) {
        /* 如果AP和STAUT都支持80M，则设置为AP一样 */
        en_band = en_bss_cap;
        return en_band;
    }

    switch (en_bss_cap) {
        case WLAN_BAND_WIDTH_40PLUS:
        case WLAN_BAND_WIDTH_80PLUSPLUS:
        case WLAN_BAND_WIDTH_80PLUSMINUS:
            if (en_dev_cap >= WLAN_BW_CAP_40M) {
                en_band = WLAN_BAND_WIDTH_40PLUS;
            }
            break;

        case WLAN_BAND_WIDTH_40MINUS:
        case WLAN_BAND_WIDTH_80MINUSPLUS:
        case WLAN_BAND_WIDTH_80MINUSMINUS:
            if (en_dev_cap >= WLAN_BW_CAP_40M) {
                en_band = WLAN_BAND_WIDTH_40MINUS;
            }
            break;

        default:
            en_band = WLAN_BAND_WIDTH_20M;
            break;
    }

    return en_band;
}


OAL_STATIC oal_void hmac_sta_update_join_bss_info(mac_bss_dscr_stru *pst_bss_dscr)
{
    /* 165 信道只允许20MHz 带宽 */
    if ((pst_bss_dscr->st_channel.uc_chan_number == 165) &&
        (pst_bss_dscr->en_channel_bandwidth != WLAN_BAND_WIDTH_20M)) {
        oam_warning_log4(0, OAM_SF_ASSOC,
                         "{BSS [XX:XX:XX:XX:%02X:%02X] set wrong bandwidth [%d] at channel [%d]}",
                         pst_bss_dscr->auc_bssid[4], /* 打印auc_bssid的4、5byte */
                         pst_bss_dscr->auc_bssid[5],
                         pst_bss_dscr->en_channel_bandwidth,
                         pst_bss_dscr->st_channel.uc_chan_number);

        pst_bss_dscr->st_channel.en_bandwidth = WLAN_BAND_WIDTH_20M;
        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_20M;
        pst_bss_dscr->en_bw_cap = WLAN_BW_CAP_20M;
    }

    return;
}

oal_void hmac_sta_update_capability(mac_vap_stru *pst_mac_vap)
{
    if (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) {
        mac_mib_set_ShortPreambleOptionImplemented(pst_mac_vap, WLAN_LEGACY_11B_MIB_SHORT_PREAMBLE);
        mac_mib_set_SpectrumManagementRequired(pst_mac_vap, OAL_FALSE);
    } else {
        mac_mib_set_ShortPreambleOptionImplemented(pst_mac_vap, WLAN_LEGACY_11B_MIB_LONG_PREAMBLE);
        mac_mib_set_SpectrumManagementRequired(pst_mac_vap, OAL_TRUE);
    }
}

oal_void hmac_sta_set_reg_params(mac_vap_stru *pst_mac_vap, hmac_join_req_stru *pst_join_req,
    dmac_ctx_join_req_set_reg_stru *pst_reg_params)
{
    /* 设置需要写入寄存器的BSSID信息 */
    oal_set_mac_addr(pst_reg_params->auc_bssid, pst_join_req->st_bss_dscr.auc_bssid);

    /* 填写信道相关信息 */
    pst_reg_params->st_current_channel.uc_chan_number = pst_mac_vap->st_channel.uc_chan_number;
    pst_reg_params->st_current_channel.en_band = pst_mac_vap->st_channel.en_band;
    pst_reg_params->st_current_channel.en_bandwidth = pst_mac_vap->st_channel.en_bandwidth;
    pst_reg_params->st_current_channel.uc_idx = pst_mac_vap->st_channel.uc_idx;

    /* 设置beaocn period信息 */
    pst_reg_params->us_beacon_period = (pst_join_req->st_bss_dscr.us_beacon_period);

    /* 同步FortyMHzOperationImplemented */
    pst_reg_params->en_dot11FortyMHzOperationImplemented = mac_mib_get_FortyMHzOperationImplemented(pst_mac_vap);

    /* 设置beacon filter关闭 */
    pst_reg_params->ul_beacon_filter = OAL_FALSE;

    /* 设置no frame filter打开 */
    pst_reg_params->ul_non_frame_filter = OAL_TRUE;
}


OAL_STATIC oal_void hmac_update_sta_country_code(hmac_vap_stru *pst_hmac_vap, hmac_join_req_stru *pst_join_req)
{
#ifdef _PRE_WLAN_FEATURE_11D
    /* 更新sta期望加入的国家字符串 */
    pst_hmac_vap->ac_desired_country[0] = pst_join_req->st_bss_dscr.ac_country[0];
    pst_hmac_vap->ac_desired_country[1] = pst_join_req->st_bss_dscr.ac_country[1];
    pst_hmac_vap->ac_desired_country[2] = pst_join_req->st_bss_dscr.ac_country[2]; /* 将\0赋值给desired_country第2byte */
#endif
}


OAL_STATIC oal_void hmac_set_device_channel_info(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap)
{
    pst_mac_device->uc_max_channel = pst_mac_vap->st_channel.uc_chan_number;
    pst_mac_device->en_max_band = pst_mac_vap->st_channel.en_band;
    pst_mac_device->en_max_bandwidth = pst_mac_vap->st_channel.en_bandwidth;
}


OAL_STATIC oal_void hmac_set_fortymhzoperationimplemented_by_bssid(
    hmac_vap_stru *pst_hmac_vap, hmac_join_req_stru *pst_join_req)
{
    hmac_sta_20m_ap_stru *pst_ap_info;

    pst_ap_info = &pst_hmac_vap->st_20m_ap_info;
    /* 当记录的BSSID与要关联的BSSID相同且当前AP的信号强度小于门限值则按20M带宽进行关联 */
    if ((oal_memcmp(pst_ap_info->auc_bssid, pst_join_req->st_bss_dscr.auc_bssid, WLAN_MAC_ADDR_LEN) == 0) &&
        ((pst_join_req->st_bss_dscr.c_rssi < pst_ap_info->c_rssi) ||
        (pst_hmac_vap->en_web_fail_roam == OAL_TRUE))) { /* 重关联的漫游是没有扫描的，历史记录里面的rssi是不可靠的 */
        mac_mib_set_FortyMHzOperationImplemented(&pst_hmac_vap->st_vap_base_info, OAL_FALSE);
        oam_warning_log0(0, OAM_SF_ANY, "hmac_set_FortyMHzOperationImplemented_by_chr:only 20M");
    }
}
OAL_STATIC oal_void hmac_sta_update_mib_for_assoc_ap(hmac_vap_stru *pst_hmac_vap, hmac_join_req_stru *pst_join_req,
    mac_cfg_mode_param_stru *pst_cfg_mode, mac_vap_stru *pst_mac_vap, wlan_mib_ieee802dot11_stru *pst_mib)
{
    memset_s(pst_cfg_mode, OAL_SIZEOF(mac_cfg_mode_param_stru), 0, OAL_SIZEOF(mac_cfg_mode_param_stru));
    pst_mib->st_wlan_mib_sta_config.en_dot11HighThroughputOptionImplemented =
        pst_join_req->st_bss_dscr.en_ht_capable;
    pst_mib->st_wlan_mib_sta_config.en_dot11VHTOptionImplemented = pst_join_req->st_bss_dscr.en_vht_capable;
    pst_mib->st_phy_ht.en_dot11LDPCCodingOptionActivated =
        (pst_join_req->st_bss_dscr.en_ht_ldpc && pst_mib->st_phy_ht.en_dot11LDPCCodingOptionImplemented);
    pst_mib->st_phy_ht.en_dot11TxSTBCOptionActivated =
        (pst_join_req->st_bss_dscr.en_ht_stbc && pst_mib->st_phy_ht.en_dot11TxSTBCOptionImplemented);

    /* 关联2G AP，且2ght40禁止位为1时，不学习AP的HT 40能力 */
    if (!((pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) && pst_mac_vap->st_cap_flag.bit_disable_2ght40)) {
        if (pst_join_req->st_bss_dscr.en_bw_cap == WLAN_BW_CAP_20M) {
            mac_mib_set_FortyMHzOperationImplemented(pst_mac_vap, OAL_FALSE);
        } else {
            mac_mib_set_FortyMHzOperationImplemented(pst_mac_vap, OAL_TRUE);
        }
    }
    hmac_set_fortymhzoperationimplemented_by_bssid(pst_hmac_vap, pst_join_req);
}

oal_uint32 hmac_sta_update_join_req_params(hmac_vap_stru *pst_hmac_vap, hmac_join_req_stru *pst_join_req)
{
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    mac_cfg_mode_param_stru st_cfg_mode;
    mac_vap_stru *pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    wlan_mib_ieee802dot11_stru *pst_mib = pst_mac_vap->pst_mib_info;
    mac_device_stru *pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    dmac_ctx_join_req_set_reg_stru *pst_reg_params = OAL_PTR_NULL;

    if (pst_mib == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (pst_mac_device == OAL_PTR_NULL) {
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }
    /* 设置BSSID */
    mac_vap_set_bssid(pst_mac_vap, pst_join_req->st_bss_dscr.auc_bssid);

    /* 更新mib库对应的dot11BeaconPeriod值 */
    pst_mib->st_wlan_mib_sta_config.ul_dot11BeaconPeriod = (oal_uint32)(pst_join_req->st_bss_dscr.us_beacon_period);

    /* 更新mib库对应的ul_dot11CurrentChannel值 */
    mac_vap_set_current_channel(pst_mac_vap, pst_join_req->st_bss_dscr.st_channel.en_band,
                                pst_join_req->st_bss_dscr.st_channel.uc_chan_number);
    hmac_update_sta_country_code(pst_hmac_vap, pst_join_req);
    /* 更新mib库对应的ssid */
    if (memcpy_s(pst_mib->st_wlan_mib_sta_config.auc_dot11DesiredSSID, WLAN_SSID_MAX_LEN,
        pst_join_req->st_bss_dscr.ac_ssid, WLAN_SSID_MAX_LEN) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "hmac_sta_update_join_req_params::memcpy fail!");
        return OAL_FAIL;
    }

    pst_mib->st_wlan_mib_sta_config.auc_dot11DesiredSSID[WLAN_SSID_MAX_LEN - 1] = '\0';

    hmac_sta_update_join_bss_info(&(pst_join_req->st_bss_dscr));

    /* 更新频带、主20MHz信道号，与AP通信 DMAC切换信道时直接调用 */
    pst_mac_vap->st_channel.en_bandwidth = hmac_sta_get_band(pst_mac_device->en_bandwidth_cap,
                                                             pst_join_req->st_bss_dscr.en_channel_bandwidth);
    pst_mac_vap->st_channel.uc_chan_number = pst_join_req->st_bss_dscr.st_channel.uc_chan_number;
    pst_mac_vap->st_channel.en_band = pst_join_req->st_bss_dscr.st_channel.en_band;
    /* 在STA未配置协议模式情况下，根据要关联的AP，更新mib库中对应的HT/VHT能力 */
    if (pst_hmac_vap->bit_sta_protocol_cfg == OAL_SWITCH_OFF) {
        hmac_sta_update_mib_for_assoc_ap(pst_hmac_vap, pst_join_req, &st_cfg_mode, pst_mac_vap, pst_mib);

        /* 根据要加入AP的协议模式更新STA侧速率集 */
        ul_ret = hmac_sta_get_user_protocol(&(pst_join_req->st_bss_dscr), &(st_cfg_mode.en_protocol));
        if (ul_ret != OAL_SUCC) {
            OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{hmac_sta_get_user_protocol fail %d.}", ul_ret);
            return ul_ret;
        }

        st_cfg_mode.en_band = pst_join_req->st_bss_dscr.st_channel.en_band;
        st_cfg_mode.en_bandwidth =
            hmac_sta_get_band(pst_mac_device->en_bandwidth_cap, pst_join_req->st_bss_dscr.en_channel_bandwidth);
        st_cfg_mode.en_channel_idx = pst_join_req->st_bss_dscr.st_channel.uc_chan_number;
        ul_ret = hmac_config_sta_update_rates(pst_mac_vap, &st_cfg_mode, (oal_void *)&pst_join_req->st_bss_dscr);
        if (ul_ret != OAL_SUCC) {
            OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{hmac_config_sta_update_rates fail %d.}", ul_ret);
            return ul_ret;
        }
    }

    /* 有些加密协议只能工作在legacy */
    hmac_sta_protocol_down_by_chipher(pst_mac_vap, &pst_join_req->st_bss_dscr);
#if defined(_PRE_WLAN_FEATURE_WPA) || defined(_PRE_WLAN_FEATURE_WPA2)
    st_cfg_mode.en_protocol = pst_mac_vap->en_protocol;
    st_cfg_mode.en_band = pst_mac_vap->st_channel.en_band;
    st_cfg_mode.en_bandwidth = pst_mac_vap->st_channel.en_bandwidth;
    st_cfg_mode.en_channel_idx = pst_join_req->st_bss_dscr.st_channel.uc_chan_number;
    hmac_config_sta_update_rates(pst_mac_vap, &st_cfg_mode, (oal_void *)&pst_join_req->st_bss_dscr);
#endif

    /* STA首先以20MHz运行，如果要切换到40 or 80MHz运行，需要满足一下条件: */
    /* (1) 用户支持40 or 80MHz运行 */
    /* (2) AP支持40 or 80MHz运行(HT Supported Channel Width Set = 1 && VHT Supported Channel Width Set = 0) */
    /* (3) AP在40 or 80MHz运行(SCO = SCA or SCB && VHT Channel Width = 1) */
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    pst_mac_vap->st_channel.en_bandwidth = WLAN_BAND_WIDTH_20M;
#endif
    ul_ret = mac_get_channel_idx_from_num(pst_mac_vap->st_channel.en_band, pst_mac_vap->st_channel.uc_chan_number,
                                          &(pst_mac_vap->st_channel.uc_idx));
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{channelnum are not compatible.band[%d], channel_num[%d]}",
                       pst_mac_vap->st_channel.en_band, pst_mac_vap->st_channel.uc_chan_number);
        return ul_ret;
    }

    /* 更新协议相关信息，包括WMM P2P 11I 20/40M等 */
    hmac_update_join_req_params_prot_sta(pst_hmac_vap, pst_join_req);
    /* 入网优化，不同频段下的能力不一样 */
    hmac_sta_update_capability(pst_mac_vap);

    if (hmac_calc_up_ap_num(pst_mac_device) == 0) {
        hmac_set_device_channel_info(pst_mac_device, pst_mac_vap);
    }

    /* 抛事件到DMAC, 申请事件内存 */
    pst_event_mem = frw_event_alloc_m(OAL_SIZEOF(dmac_ctx_join_req_set_reg_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_sta_update_join_req_params::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 填写事件 */
    pst_event = (frw_event_stru *)pst_event_mem->puc_data;
    frw_event_hdr_init(&(pst_event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_JOIN_SET_REG,
                       OAL_SIZEOF(dmac_ctx_join_req_set_reg_stru),
                       FRW_EVENT_PIPELINE_STAGE_1, pst_hmac_vap->st_vap_base_info.uc_chip_id,
                       pst_hmac_vap->st_vap_base_info.uc_device_id, pst_hmac_vap->st_vap_base_info.uc_vap_id);
    pst_reg_params = (dmac_ctx_join_req_set_reg_stru *)pst_event->auc_event_data;

    hmac_sta_set_reg_params(pst_mac_vap, pst_join_req, pst_reg_params);

    /* 下发ssid */
    if (memcpy_s(pst_reg_params->auc_ssid, WLAN_SSID_MAX_LEN,
        pst_join_req->st_bss_dscr.ac_ssid, WLAN_SSID_MAX_LEN) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "hmac_sta_update_join_req_params::memcpy fail!");
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    pst_reg_params->auc_ssid[WLAN_SSID_MAX_LEN - 1] = '\0';

    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    hmac_sta_roam_update_pmf(pst_mac_vap, &pst_join_req->st_bss_dscr);
#endif

    return OAL_SUCC;
}


oal_uint32 hmac_sta_wait_asoc_timeout(hmac_vap_stru *pst_hmac_sta, oal_void *p_param)
{
    hmac_asoc_rsp_stru st_asoc_rsp = { 0 };
    hmac_mgmt_timeout_param_stru *pst_timeout_param = OAL_PTR_NULL;
    mac_cfg_query_mgmt_send_status_stru st_send_state;

    if ((pst_hmac_sta == OAL_PTR_NULL) || (p_param == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_SCAN, "{hmac_sta_wait_asoc_timeout::param null,%x %x.}", (uintptr_t)pst_hmac_sta,
                       (uintptr_t)p_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_timeout_param = (hmac_mgmt_timeout_param_stru *)p_param;

    /* 填写关联结果 */
    st_asoc_rsp.en_result_code = HMAC_MGMT_TIMEOUT;

    /* 关联超时失败,原因码上报wpa_supplicant */
    st_asoc_rsp.en_status_code = pst_timeout_param->en_status_code;

    if (hmac_get_auth_assoc_send_status_report_flag(pst_hmac_sta, &st_send_state) == OAL_SUCC) {
        if (st_send_state.uc_asoc_req_st > 0 && st_asoc_rsp.en_status_code == MAC_ASOC_RSP_TIMEOUT) {
            st_asoc_rsp.en_status_code = MAC_ASOC_REQ_SEND_FAIL_BEGIN + st_send_state.uc_asoc_req_st;
            hmac_set_auth_assoc_send_status_report_flag(pst_hmac_sta, OAL_FALSE);
        }
    }

    /* 发送关联结果给SME */
    hmac_send_rsp_to_sme_sta(pst_hmac_sta, HMAC_SME_ASOC_RSP, (oal_uint8 *)&st_asoc_rsp);

    return OAL_SUCC;
}


oal_void hmac_sta_handle_disassoc_rsp(hmac_vap_stru *pst_hmac_vap, oal_uint16 us_disasoc_reason_code)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event = OAL_PTR_NULL;

    /* 抛加入完成事件到WAL */
    pst_event_mem = frw_event_alloc_m(OAL_SIZEOF(oal_uint16));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{hmac_sta_handle_disassoc_rsp::pst_event_mem null.}");
        return;
    }

    /* 填写事件 */
    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_DISASOC_COMP_STA,
                       OAL_SIZEOF(oal_uint16),
                       FRW_EVENT_PIPELINE_STAGE_0,
                       pst_hmac_vap->st_vap_base_info.uc_chip_id,
                       pst_hmac_vap->st_vap_base_info.uc_device_id,
                       pst_hmac_vap->st_vap_base_info.uc_vap_id);

    *((oal_uint16 *)pst_event->auc_event_data) = us_disasoc_reason_code; /* 事件payload填写的是错误码 */

    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);

    return;
}


OAL_STATIC oal_uint32 hmac_sta_rx_deauth_req(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_mac_hdr,
                                             oal_bool_enum_uint8 en_is_protected)
{
    oal_uint8 auc_bssid[6] = { 0 };
    hmac_user_stru *pst_hmac_user_vap = OAL_PTR_NULL;
    oal_uint16 us_user_idx = 0xffff;
    oal_uint32 ul_ret;
    oal_uint8 *puc_da = OAL_PTR_NULL;
    oal_uint8 *puc_sa = OAL_PTR_NULL;
    oal_uint32 ul_ret_del_user;

    if ((pst_hmac_vap == OAL_PTR_NULL) || (puc_mac_hdr == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_AUTH, "{hmac_sta_rx_deauth_req::param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 增加接收到去认证帧或者去关联帧时的维测信息 */
    mac_rx_get_sa((mac_ieee80211_frame_stru *)puc_mac_hdr, &puc_sa);
    oam_warning_log4(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                     "{err_code[%d], received deauth or disassoc frame from source addr,sa xx:xx:xx:%2x:%2x:%2x.}",
                     /* puc_sa第3、4、5byte为参数输出打印 */
                     *((oal_uint16 *)(puc_mac_hdr + MAC_80211_FRAME_LEN)), puc_sa[3], puc_sa[4], puc_sa[5]);

    mac_get_address2(puc_mac_hdr, auc_bssid);

    ul_ret = mac_vap_find_user_by_macaddr(&pst_hmac_vap->st_vap_base_info, auc_bssid, &us_user_idx);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                         "{hmac_sta_rx_deauth_req::find user failed[%d],other bss deauth frame!}", ul_ret);
        /* 没有查到对应的USER,发送去认证消息 */
        return ul_ret;
    }
    pst_hmac_user_vap = mac_res_get_hmac_user(us_user_idx);
    if (pst_hmac_user_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                       "{hmac_sta_rx_deauth_req::pst_hmac_user_vap[%d] null.}", us_user_idx);
        /* 没有查到对应的USER,发送去认证消息 */
        hmac_mgmt_send_deauth_frame(&(pst_hmac_vap->st_vap_base_info), auc_bssid, MAC_NOT_AUTHED, OAL_FALSE);

        hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
        /* 上报内核sta已经和某个ap去关联 */
        hmac_sta_handle_disassoc_rsp (pst_hmac_vap, *((oal_uint16 *)(puc_mac_hdr + MAC_80211_FRAME_LEN)));
        return OAL_FAIL;
    }

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    /* 检查是否需要发送SA query request */
    if ((pst_hmac_user_vap->st_user_base_info.en_user_asoc_state == MAC_USER_STATE_ASSOC) &&
        (hmac_pmf_check_err_code(&pst_hmac_user_vap->st_user_base_info, en_is_protected, puc_mac_hdr) == OAL_SUCC)) {
        /* 在关联状态下收到未加密的ReasonCode 6/7需要启动SA Query流程 */
        ul_ret = hmac_start_sa_query(&pst_hmac_vap->st_vap_base_info, pst_hmac_user_vap,
                                     pst_hmac_user_vap->st_user_base_info.st_cap_info.bit_pmf_active);
        if (ul_ret != OAL_SUCC) {
            return OAL_ERR_CODE_PMF_SA_QUERY_START_FAIL;
        }

        return OAL_SUCC;
    }

#endif

    /* 如果该用户的管理帧加密属性不一致，丢弃该报文 */
    mac_rx_get_da((mac_ieee80211_frame_stru *)puc_mac_hdr, &puc_da);
    if ((ether_is_multicast(puc_da) != OAL_TRUE) &&
        (en_is_protected != pst_hmac_user_vap->st_user_base_info.st_cap_info.bit_pmf_active)) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                       "{hmac_sta_rx_deauth_req::PMF check failed.}");

        return OAL_FAIL;
    }

    /* 删除user */
    ul_ret_del_user = hmac_user_del(&pst_hmac_vap->st_vap_base_info, pst_hmac_user_vap);
    if (ul_ret_del_user != OAL_SUCC) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                       "{hmac_sta_rx_deauth_req::hmac_user_del failed.}");

        /* 上报内核sta已经和某个ap去关联 */
        hmac_sta_handle_disassoc_rsp (pst_hmac_vap, *((oal_uint16 *)(puc_mac_hdr + MAC_80211_FRAME_LEN)));
        return OAL_FAIL;
    }

    /* 上报内核sta已经和某个ap去关联 */
    hmac_sta_handle_disassoc_rsp (pst_hmac_vap, *((oal_uint16 *)(puc_mac_hdr + MAC_80211_FRAME_LEN)));

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_sta_up_rx_beacon(hmac_vap_stru *pst_hmac_vap_sta, oal_netbuf_stru *pst_netbuf)
{
    dmac_rx_ctl_stru *pst_rx_ctrl;
    mac_rx_ctl_stru *pst_rx_info;
    mac_ieee80211_frame_stru *pst_mac_hdr;
    oal_uint32 ul_ret;
    oal_uint16 us_frame_len;
    oal_uint16 us_frame_offset;
    oal_uint8 *puc_frame_body;
    oal_uint8 uc_frame_sub_type;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_uint8 auc_addr_sa[WLAN_MAC_ADDR_LEN];
    oal_uint16 us_user_idx;
#ifdef _PRE_WLAN_FEATURE_TXBF
    oal_uint8 *puc_txbf_vendor_ie;
#endif

    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    pst_rx_info = (mac_rx_ctl_stru *)(&(pst_rx_ctrl->st_rx_info));
    pst_mac_hdr = (mac_ieee80211_frame_stru *)(pst_rx_info->pul_mac_hdr_start_addr);
    puc_frame_body = (oal_uint8 *)pst_mac_hdr + pst_rx_info->uc_mac_header_len;
    us_frame_len = pst_rx_info->us_frame_len - pst_rx_info->uc_mac_header_len; /* 帧体长度 */

    us_frame_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    uc_frame_sub_type = mac_get_frame_sub_type((oal_uint8 *)pst_mac_hdr);

    /* 来自其它bss的Beacon不做处理 */
    ul_ret = oal_compare_mac_addr(pst_hmac_vap_sta->st_vap_base_info.auc_bssid, pst_mac_hdr->auc_address3);
    if (ul_ret != 0) {
        return OAL_SUCC;
    }

    /* 获取管理帧的源地址SA */
    mac_get_address2((oal_uint8 *)pst_mac_hdr, auc_addr_sa);

    /* 根据SA 地地找到对应AP USER结构 */
    ul_ret = mac_vap_find_user_by_macaddr(&(pst_hmac_vap_sta->st_vap_base_info), auc_addr_sa, &us_user_idx);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_vap_sta->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                         "{hmac_sta_up_rx_beacon::mac_vap_find_user_by_macaddr failed[%d].}", ul_ret);
        return ul_ret;
    }
    pst_hmac_user = mac_res_get_hmac_user(us_user_idx);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_hmac_vap_sta->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                       "{hmac_sta_up_rx_beacon::pst_hmac_user[%d] null.}", us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }
#ifdef _PRE_WLAN_FEATURE_TXBF
    /* 更新11n txbf能力 */
    puc_txbf_vendor_ie = mac_find_vendor_ie(MAC_HUAWEI_VENDER_IE, MAC_EID_11NTXBF,
                                            (oal_uint8 *)pst_mac_hdr + MAC_TAG_PARAM_OFFSET,
                                            us_frame_len - us_frame_offset);
    hmac_mgmt_update_11ntxbf_cap(puc_txbf_vendor_ie, pst_hmac_user);
#endif

    /* 评估是否需要进行带宽切换 */
    /* 更新edca参数 */
    hmac_sta_up_update_edca_params(puc_frame_body, us_frame_len, us_frame_offset, pst_hmac_vap_sta, uc_frame_sub_type,
                                   pst_hmac_user);

    return OAL_SUCC;
}
OAL_STATIC oal_void hmac_sta_up_category_ba(hmac_vap_stru *pst_hmac_vap, mac_ieee80211_frame_stru *pst_frame_hdr,
    hmac_user_stru *pst_hmac_user, oal_uint8 *puc_data, oal_uint32 frame_body_len)
{
    switch (puc_data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_BA_ACTION_ADDBA_REQ:
#ifdef _PRE_WLAN_FEATURE_BTCOEX
            hmac_btcoex_check_rx_same_baw_start_from_addba_req(pst_hmac_vap, pst_hmac_user, pst_frame_hdr,
                                                               puc_data, frame_body_len);
#endif
            hmac_mgmt_rx_addba_req(pst_hmac_vap, pst_hmac_user, puc_data, frame_body_len);
            break;

        case MAC_BA_ACTION_ADDBA_RSP:
            hmac_mgmt_rx_addba_rsp(pst_hmac_vap, pst_hmac_user, puc_data, frame_body_len);
            break;

        case MAC_BA_ACTION_DELBA:
            hmac_mgmt_rx_delba(pst_hmac_vap, pst_hmac_user, puc_data, frame_body_len);
            break;

        default:
            break;
    }
}

OAL_STATIC oal_void hmac_sta_up_category_wnm(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user,
    oal_netbuf_stru *pst_netbuf, oal_uint8 *puc_data)
{
    OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                     "{hmac_sta_up_rx_action::MAC_ACTION_CATEGORY_WNM action=%d.}",
                     puc_data[MAC_ACTION_OFFSET_ACTION]);
    switch (puc_data[MAC_ACTION_OFFSET_ACTION]) {
#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
        /* bss transition request 帧处理入口 */
        case MAC_WNM_ACTION_BSS_TRANSITION_MGMT_REQUEST:
            hmac_rx_bsst_req_action(pst_hmac_vap, pst_hmac_user, pst_netbuf);
            break;
#endif
        default:
#ifdef _PRE_WLAN_FEATURE_HS20
            /* 上报WNM Notification Request Action帧 */
            hmac_rx_mgmt_send_to_host(pst_hmac_vap, pst_netbuf);
#endif
            break;
    }
}

OAL_STATIC oal_void hmac_sta_up_category_public(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf,
    oal_uint8 *puc_data, oal_uint32 frame_body_len)
{
    /* Action */
    switch (puc_data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_PUB_VENDOR_SPECIFIC:
            if (frame_body_len <= MAC_ACTION_CATEGORY_AND_CODE_LEN + MAC_OUI_LEN) {
                OAM_WARNING_LOG1(0, OAM_SF_RX, "{hmac_action_category_public::frame_body_len %d.}", frame_body_len);
                return;
            }
#ifdef _PRE_WLAN_FEATURE_P2P
            /* 查找OUI-OUI type值为 50 6F 9A - 09 (WFA P2P v1.0) */
            /* 并用hmac_rx_mgmt_send_to_host接口上报 */
            if (mac_ie_check_p2p_action(puc_data + MAC_ACTION_OFFSET_ACTION) == OAL_TRUE) {
                hmac_rx_mgmt_send_to_host(pst_hmac_vap, pst_netbuf);
            }
#endif
#ifdef _PRE_WLAN_FEATURE_LOCATION_RAM
            if (oal_memcmp(puc_data + MAC_ACTION_CATEGORY_AND_CODE_LEN, g_auc_huawei_oui, MAC_OUI_LEN) == 0) {
                oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                                 "{hmac_sta_up_rx_action::hmac location get.}");
                hmac_huawei_action_process(pst_hmac_vap, pst_netbuf,
                                           puc_data[MAC_ACTION_CATEGORY_AND_CODE_LEN + MAC_OUI_LEN]);
            }
#endif /* _PRE_WLAN_FEATURE_LOCATION */
            break;

        case MAC_PUB_GAS_INIT_RESP:
        case MAC_PUB_GAS_COMBAK_RESP:
#ifdef _PRE_WLAN_FEATURE_HS20
            /* 上报GAS查询的ACTION帧 */
            hmac_rx_mgmt_send_to_host(pst_hmac_vap, pst_netbuf);
#endif
            break;

        default:
            break;
    }
}
OAL_STATIC oal_void hmac_sta_up_category_vendor(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf,
    oal_uint8 *puc_data)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    /* 查找OUI-OUI type值为 50 6F 9A - 09 (WFA P2P v1.0) */
    /* 并用hmac_rx_mgmt_send_to_host接口上报 */
    if (mac_ie_check_p2p_action(puc_data + MAC_ACTION_OFFSET_CATEGORY) == OAL_TRUE) {
        hmac_rx_mgmt_send_to_host(pst_hmac_vap, pst_netbuf);
    }
#endif
}
/* 获取帧体长度 */
oal_uint32 hmac_get_frame_body_len(oal_netbuf_stru *net_buf)
{
    dmac_rx_ctl_stru *rx_ctrl = NULL;
    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(net_buf);
    if (rx_ctrl == NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ASSOC, "{hmac_get_frame_body_len::get null.}");
        return 0;
    }
    if (rx_ctrl->st_rx_info.us_frame_len < rx_ctrl->st_rx_info.uc_mac_header_len) {
        OAM_ERROR_LOG1(0, OAM_SF_ASSOC, "{hmac_get_frame_body_len::us_frame_len%d.}", rx_ctrl->st_rx_info.us_frame_len);
        return 0;
    }

    return rx_ctrl->st_rx_info.us_frame_len - rx_ctrl->st_rx_info.uc_mac_header_len;
}


OAL_STATIC oal_void hmac_sta_up_rx_action(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf,
                                          oal_bool_enum_uint8 en_is_protected)
{
    dmac_rx_ctl_stru *pst_rx_ctrl = NULL;
    oal_uint8 *puc_data = OAL_PTR_NULL;
    mac_ieee80211_frame_stru *pst_frame_hdr = NULL; /* 保存mac帧的指针 */
    hmac_user_stru *pst_hmac_user = NULL;
#ifdef _PRE_WLAN_FEATURE_P2P
    oal_uint8 *puc_p2p0_mac_addr;
#endif
    oal_uint32 frame_body_len;

    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    frame_body_len = hmac_get_frame_body_len(pst_netbuf);
    if (frame_body_len < MAC_ACTION_CATEGORY_AND_CODE_LEN) {
        OAM_WARNING_LOG1(0, OAM_SF_RX, "{sta_up::frame len too short[%d].}", pst_rx_ctrl->st_rx_info.us_frame_len);
        return;
    }

    /* 获取帧头信息 */
    pst_frame_hdr = (mac_ieee80211_frame_stru *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr;
#ifdef _PRE_WLAN_FEATURE_P2P
    /* P2P0设备所接受的action全部上报 */
    puc_p2p0_mac_addr = pst_hmac_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID;
    if (oal_compare_mac_addr(pst_frame_hdr->auc_address1, puc_p2p0_mac_addr) == 0) {
        hmac_rx_mgmt_send_to_host(pst_hmac_vap, pst_netbuf);
    }
#endif

    /* 获取发送端的用户指针 */
    pst_hmac_user = mac_vap_get_hmac_user_by_addr(&pst_hmac_vap->st_vap_base_info, pst_frame_hdr->auc_address2);
    if (pst_hmac_user == OAL_PTR_NULL) {
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                         "{hmac_sta_up_rx_action::mac_vap_find_user_by_macaddr failed.}");
#ifdef _PRE_WLAN_FEATURE_LOCATION_RAM
        hmac_sta_up_rx_action_nonuser(pst_hmac_vap, pst_netbuf);
#endif
        return;
    }

    /* 获取帧体指针 */
    puc_data = (oal_uint8 *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr + pst_rx_ctrl->st_rx_info.uc_mac_header_len;

    /* Category */
    switch (puc_data[MAC_ACTION_OFFSET_CATEGORY]) {
        case MAC_ACTION_CATEGORY_BA: {
            hmac_sta_up_category_ba(pst_hmac_vap, pst_frame_hdr, pst_hmac_user, puc_data, frame_body_len);
        }
        break;

        case MAC_ACTION_CATEGORY_WNM: {
            hmac_sta_up_category_wnm(pst_hmac_vap, pst_hmac_user, pst_netbuf, puc_data);
        }
        break;

        case MAC_ACTION_CATEGORY_PUBLIC: {
            hmac_sta_up_category_public(pst_hmac_vap, pst_netbuf, puc_data, frame_body_len);
        }
        break;
#ifdef _PRE_WLAN_FEATURE_WMMAC
        case MAC_ACTION_CATEGORY_WMMAC_QOS: {
            hmac_sta_up_category_wmmac_qos(pst_hmac_vap, pst_hmac_user, puc_data, frame_body_len);
        }
        break;
#endif  // _PRE_WLAN_FEATURE_WMMAC

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
        case MAC_ACTION_CATEGORY_SA_QUERY: {
            hmac_sta_up_category_sa_query(pst_hmac_vap, pst_netbuf, en_is_protected, puc_data);
        }
        break;
#endif
        case MAC_ACTION_CATEGORY_VENDOR: {
            if (frame_body_len <= MAC_ACTION_CATEGORY_AND_CODE_LEN + MAC_OUI_LEN) {
                OAM_WARNING_LOG1(0, OAM_SF_RX, "{hmac_action_category_vendor::frame_body_len %d.}", frame_body_len);
                return;
            }
            hmac_sta_up_category_vendor(pst_hmac_vap, pst_netbuf, puc_data);
        }
        break;
#ifdef _PRE_WLAN_FEATURE_ROAM
#ifdef _PRE_WLAN_FEATURE_11R
        case MAC_ACTION_CATEGORY_FAST_BSS_TRANSITION: {
            if (pst_hmac_vap->bit_11r_enable != OAL_TRUE) {
                break;
            }
            hmac_roam_rx_ft_action(pst_hmac_vap, pst_netbuf);
            break;
        }
#endif  // _PRE_WLAN_FEATURE_11R
#endif  // _PRE_WLAN_FEATURE_ROAM

        default:
            break;
    }
}


oal_uint32 hmac_sta_up_rx_mgmt(hmac_vap_stru *pst_hmac_vap_sta, oal_void *p_param)
{
    dmac_wlan_crx_event_stru *pst_mgmt_rx_event = OAL_PTR_NULL;
    dmac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;
    mac_rx_ctl_stru *pst_rx_info = OAL_PTR_NULL;
    oal_uint8 *puc_mac_hdr = OAL_PTR_NULL;
    oal_uint8 uc_mgmt_frm_type;
    oal_bool_enum_uint8 en_is_protected;
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    oal_uint8 *puc_data = OAL_PTR_NULL;
#endif

    if ((pst_hmac_vap_sta == OAL_PTR_NULL) || (p_param == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_sta_up_rx_mgmt::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mgmt_rx_event = (dmac_wlan_crx_event_stru *)p_param;
    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_mgmt_rx_event->pst_netbuf);
    pst_rx_info = (mac_rx_ctl_stru *)(&(pst_rx_ctrl->st_rx_info));
    puc_mac_hdr = (oal_uint8 *)(pst_rx_info->pul_mac_hdr_start_addr);
    en_is_protected = mac_get_protectedframe(puc_mac_hdr);

    /* STA在UP状态下 接收到的各种管理帧处理 */
    uc_mgmt_frm_type = mac_get_frame_sub_type(puc_mac_hdr);

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    if (wlan_pm_wkup_src_debug_get() == OAL_TRUE) {
        wlan_pm_wkup_src_debug_set(OAL_FALSE);

        oam_warning_log2(pst_hmac_vap_sta->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                         "{wakeup frame type[0x%x] sub type[0x%x]}", mac_get_frame_type(puc_mac_hdr), uc_mgmt_frm_type);
        /* action帧唤醒时打印action帧类型 */
        if (mac_get_frame_type_and_subtype(puc_mac_hdr) == (WLAN_FC0_SUBTYPE_ACTION | WLAN_FC0_TYPE_MGT)) {
            /* 获取帧体指针 */
            puc_data = puc_mac_hdr + pst_rx_ctrl->st_rx_info.uc_mac_header_len;
            oam_warning_log2(pst_hmac_vap_sta->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                             "{wifi_wake_src:hmac_sta_up_rx_mgmt::wakeup action category[0x%x], action details[0x%x]}",
                             puc_data[MAC_ACTION_OFFSET_CATEGORY], puc_data[MAC_ACTION_OFFSET_ACTION]);
        }
    }
#endif

    /* Bar frame proc here */
    if (mac_get_frame_type(puc_mac_hdr) == WLAN_FC0_TYPE_CTL) {
        uc_mgmt_frm_type = mac_get_frame_sub_type(puc_mac_hdr);
        if ((uc_mgmt_frm_type >> 4) == WLAN_BLOCKACK_REQ) { /* 判断uc_mgmt_frm_type高4位是否等于WLAN_BLOCKACK_REQ（8） */
            hmac_up_rx_bar(pst_hmac_vap_sta, pst_rx_ctrl, pst_mgmt_rx_event->pst_netbuf);
        }
    }

#ifdef _PRE_WLAN_FEATURE_SNIFFER
    proc_sniffer_write_file(NULL, 0, puc_mac_hdr, pst_rx_info->us_frame_len, 0);
#endif
    switch (uc_mgmt_frm_type) {
        case WLAN_FC0_SUBTYPE_DEAUTH:
        case WLAN_FC0_SUBTYPE_DISASSOC:
            if (pst_rx_info->us_frame_len < pst_rx_info->uc_mac_header_len + MAC_80211_REASON_CODE_LEN) {
                OAM_WARNING_LOG1(pst_hmac_vap_sta->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                                 "hmac_sta_up_rx_mgmt:: invalid deauth_req length[%d]}", pst_rx_info->us_frame_len);
            } else {
                hmac_sta_rx_deauth_req(pst_hmac_vap_sta, puc_mac_hdr, en_is_protected);
            }
            break;

        case WLAN_FC0_SUBTYPE_BEACON:
            hmac_sta_up_rx_beacon(pst_hmac_vap_sta, pst_mgmt_rx_event->pst_netbuf);
            break;

        case WLAN_FC0_SUBTYPE_ACTION:
            hmac_sta_up_rx_action(pst_hmac_vap_sta, pst_mgmt_rx_event->pst_netbuf, en_is_protected);
            break;
        default:
            break;
    }

    return OAL_SUCC;
}

/*lint -e578*//*lint -e19*/
oal_module_symbol(hmac_check_capability_mac_phy_supplicant);
/*lint +e578*//*lint +e19*/


