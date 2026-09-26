

#ifndef __HMAC_BTCOEX_C__
#define __HMAC_BTCOEX_C__

/* 1 头文件包含 */
#include "hmac_ext_if.h"
#include "mac_data.h"
#include "hmac_resource.h"
#include "hmac_btcoex.h"
#include "hmac_vap.h"
#include "hmac_user.h"
#include "hmac_fsm.h"
#include "hmac_mgmt_sta.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_roam_main.h"
#include "securec.h"
#include "hmac_mgmt_sta_up.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_BTCOEX_C

#define BTCOEX_BLACKLIST_BA_SIZE_LIMIT 0x0002


OAL_STATIC uint32_t hmac_btcoex_delba_foreach_tid(mac_vap_stru *mac_vap,
                                                  mac_user_stru *mac_user,
                                                  mac_cfg_delba_req_param_stru *mac_cfg_delba_param)
{
    uint32_t ret;

    oal_set_mac_addr(mac_cfg_delba_param->auc_mac_addr, mac_user->auc_user_mac_addr);
    mac_cfg_delba_param->en_trigger = MAC_DELBA_TRIGGER_BTCOEX;

    for (mac_cfg_delba_param->uc_tidno = 0;
         mac_cfg_delba_param->uc_tidno < WLAN_TID_MAX_NUM; mac_cfg_delba_param->uc_tidno++) {
        ret = hmac_config_delba_req(mac_vap, 0, (uint8_t *)mac_cfg_delba_param);
        if (ret != OAL_SUCC) {
            oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_COEX,
                             "{hmac_btcoex_delba_foreach_tid::ul_ret: %d, tid: %d}",
                             ret, mac_cfg_delba_param->uc_tidno);
            return ret;
        }
    }
    return ret;
}


OAL_STATIC uint32_t hmac_btcoex_delba_from_user(mac_vap_stru *mac_vap, hmac_user_stru *hmac_user)
{
    mac_cfg_delba_req_param_stru mac_cfg_delba_param;
    hmac_vap_stru *hmac_vap = NULL;

    memset_s((uint8_t *)&mac_cfg_delba_param, sizeof(mac_cfg_delba_param),
             0, sizeof(mac_cfg_delba_param));
    mac_cfg_delba_param.en_direction = MAC_RECIPIENT_DELBA;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_COEX, "{hmac_btcoex_delba_from_user::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 属于黑名单AP，并且已经处于电话业务时，不进行删BA逻辑 */
    if (hmac_user->st_hmac_user_btcoex.st_hmac_btcoex_addba_req.en_ba_handle_allow == OAL_FALSE) {
        if (BTCOEX_BLACKLIST_TPYE_FIX_BASIZE == HMAC_BTCOEX_GET_BLACKLIST_TYPE(hmac_user)) {
            oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_delba_from_user::DO NOT DELBA.}");
        } else {
            oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_delba_from_user::need to reassoc to READDBA.}");
            /* 发起reassoc req */
            hmac_roam_start(hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_FALSE, NULL, ROAM_TRIGGER_COEX);
            /* 重关联之后，刷新为允许建立聚合 */
            hmac_user->st_hmac_user_btcoex.st_hmac_btcoex_addba_req.en_ba_handle_allow = OAL_TRUE;
            /* 保证wifi恢复聚合64 */
            hmac_user->st_hmac_user_btcoex.us_ba_size = 0;
        }
        return OAL_FAIL;
    }
    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_arp_probe_type_set(mac_vap, OAL_TRUE, HMAC_VAP_ARP_PROBE_TYPE_BTCOEX_DELBA);
    }
    return hmac_btcoex_delba_foreach_tid(mac_vap, &(hmac_user->st_user_base_info), &mac_cfg_delba_param);
}


uint32_t hmac_btcoex_rx_delba_trigger(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    dmac_to_hmac_btcoex_rx_delba_trigger_event_stru *dmac_to_hmac_btcoex_rx_delba = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = NULL;
    uint32_t ret;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_COEX,
                       "{hmac_btcoex_rx_delba_trigger::pst_hmac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    dmac_to_hmac_btcoex_rx_delba = (dmac_to_hmac_btcoex_rx_delba_trigger_event_stru *)param;
    hmac_user = mac_res_get_hmac_user(dmac_to_hmac_btcoex_rx_delba->us_user_id);
    if (oal_unlikely(hmac_user == NULL)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_COEX,
                         "{hmac_btcoex_rx_delba_trigger::pst_hmac_user is null! user_id is %d.}",
                         dmac_to_hmac_btcoex_rx_delba->us_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_user->st_hmac_user_btcoex.us_ba_size = (uint16_t)dmac_to_hmac_btcoex_rx_delba->uc_ba_size;
    if (dmac_to_hmac_btcoex_rx_delba->en_need_delba == OAL_TRUE) {
        /* 刷新共存触发删建BA标记 */
        hmac_user->st_hmac_user_btcoex.en_delba_btcoex_trigger = OAL_TRUE;
    }
    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_COEX,
                     "{hmac_btcoex_rx_delba_trigger:delba size:%d, need_delba:%d, delba_trigger:%d.}",
                     dmac_to_hmac_btcoex_rx_delba->uc_ba_size,
                     dmac_to_hmac_btcoex_rx_delba->en_need_delba,
                     hmac_user->st_hmac_user_btcoex.en_delba_btcoex_trigger);
    if (dmac_to_hmac_btcoex_rx_delba->en_need_delba) {
        ret = hmac_btcoex_delba_from_user(mac_vap, hmac_user);
        if (ret != OAL_SUCC) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_COEX,
                             "{hmac_btcoex_rx_delba_trigger:delba send failed:ul_ret: %d.}", ret);
            return ret;
        }
    }
    return OAL_SUCC;
}

void hmac_btcoex_blacklist_handle_init(void *arg)
{
    hmac_user_stru *hmac_user = (hmac_user_stru *)arg;

    hmac_user->st_hmac_user_btcoex.st_hmac_btcoex_addba_req.en_blacklist_tpye = BTCOEX_BLACKLIST_TPYE_NOT_AGGR;
    oam_warning_log1(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_COEX,
                     "{hmac_btcoex_blacklist_handle_init::en_blacklist_tpye[%d]!}",
                     hmac_user->st_hmac_user_btcoex.st_hmac_btcoex_addba_req.en_blacklist_tpye);
}


uint32_t hmac_btcoex_check_exception_in_list(void *arg, uint8_t *mac_addr)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)arg;
    hmac_btcoex_delba_exception_stru *btcoex_exception = NULL;
    hmac_device_stru *hmac_device;
    uint8_t index;

    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_COEX,
                       "{hmac_btcoex_check_exception_in_list::pst_hmac_device null}");
        return OAL_FALSE;
    }
    for (index = 0; index < BTCOEX_BSS_NUM_IN_BLACKLIST; index++) {
        btcoex_exception = &(hmac_device->st_hmac_device_btcoex.btcoex_delba_exception[index]);
        if ((btcoex_exception->uc_used != 0) &&
            (oal_compare_mac_addr(btcoex_exception->auc_user_mac_addr, mac_addr) == 0)) {
            oam_warning_log4(0, OAM_SF_COEX,
                "{hmac_btcoex_check_exception_in_list::Find in blacklist, addr->%02x:%02x:XX:XX:%02x:%02x.}",
                mac_addr[0], mac_addr[1], mac_addr[4], mac_addr[5]); /* 0/1/4/5是打印mac地址的部分信息 */
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}


OAL_STATIC void hmac_btcoex_add_exception_to_list(hmac_vap_stru *hmac_vap, uint8_t *mac_addr)
{
    hmac_btcoex_delba_exception_stru *btcoex_exception = NULL;
    hmac_device_btcoex_stru *hmac_device_btcoex = NULL;
    hmac_device_stru *hmac_device;

    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_COEX,
                       "{hmac_btcoex_check_exception_in_list::pst_hmac_device null}");
        return;
    }
    hmac_device_btcoex = &(hmac_device->st_hmac_device_btcoex);
    if (hmac_device_btcoex->uc_exception_bss_index >= BTCOEX_BSS_NUM_IN_BLACKLIST) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_COEX,
                         "{hmac_btcoex_add_exception_to_list::already reach max num:%d.}", BTCOEX_BSS_NUM_IN_BLACKLIST);
        hmac_device_btcoex->uc_exception_bss_index = 0;
    }
    btcoex_exception = &(hmac_device_btcoex->btcoex_delba_exception[hmac_device_btcoex->uc_exception_bss_index]);
    oal_set_mac_addr(btcoex_exception->auc_user_mac_addr, mac_addr);
    btcoex_exception->uc_type = 0;
    btcoex_exception->uc_used = 1;
    hmac_device_btcoex->uc_exception_bss_index++;
}


void hmac_btcoex_check_rx_same_baw_start_from_addba_req(void *arg1, void *arg2, mac_ieee80211_frame_stru *frame_hdr,
    uint8_t *action, uint32_t frame_body_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)arg1;
    hmac_user_stru *hmac_user = (hmac_user_stru *)arg2;
    hmac_btcoex_addba_req_stru *hmac_btcoex_addba_req = NULL;
    hmac_user_btcoex_stru *hmac_user_btcoex = NULL;
    mac_ba_parameterset_stru *ba_param = NULL;
    mac_ba_seqctrl_stru *ba_seqctrl = NULL;
    hmac_disconnect disconnect;
    if (frame_body_len < MAC_ADDBA_REQ_FRAME_BODY_LEN) {
        oam_warning_log1(0, OAM_SF_COEX, "{frame_body_len[%d] < MAC_ADDBA_REQ_FRAME_BODY_LEN.}", frame_body_len);
        return;
    }
    hmac_user_btcoex = &(hmac_user->st_hmac_user_btcoex);
    hmac_btcoex_addba_req = &(hmac_user_btcoex->st_hmac_btcoex_addba_req);
    /* 两次收到addba req的start num一样且不是重传帧，认为对端移窗卡死  */
    if (frame_hdr->st_frame_control.bit_retry == OAL_TRUE &&
        frame_hdr->bit_seq_num == hmac_btcoex_addba_req->us_last_seq_num) {
        oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_check_rx_same_baw_start_from_addba_req::retry addba req.}");
        return;
    }
    /******************************************************************/
    /*       ADDBA Request Frame - Frame Body                         */
    /* --------------------------------------------------------------- */
    /* | Category | Action | Dialog | Parameters | Timeout | SSN     | */
    /* --------------------------------------------------------------- */
    /* | 1        | 1      | 1      | 2          | 2       | 2       | */
    /* --------------------------------------------------------------- */
    /*                                                                */
    /******************************************************************/
    ba_param = (mac_ba_parameterset_stru *)(action + MAC_ADDBA_REQ_BA_PARAM_OFFSET);
    if (ba_param->bit_tid != 0) {
        oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_check_rx_same_baw_start_from_addba_req::uc_tid != 0.}");
        return;
    }
    ba_seqctrl = (mac_ba_seqctrl_stru *)(action + MAC_ADDBA_REQ_BA_SEQCTRL_OFFSET);
    if ((ba_seqctrl->bit_startseqnum != 0) &&
        (ba_seqctrl->bit_startseqnum == hmac_btcoex_addba_req->us_last_baw_start) &&
        (hmac_vap->st_hmac_arp_probe.uc_rx_no_pkt_count > BTCOEX_ARP_FAIL_DELBA_NUM)) {
        oam_warning_log1(0, OAM_SF_COEX, "{hmac_btcoex_check_rx_same_baw_start_from_addba_req::baw_start:%d,\
            delba will forbidden.}", ba_seqctrl->bit_startseqnum);
        /* 黑名单ba处理标记 */
        hmac_btcoex_addba_req->en_ba_handle_allow = OAL_FALSE;
        if (OAL_FALSE == hmac_btcoex_check_exception_in_list(hmac_vap, frame_hdr->auc_address2)) {
            oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_check_rx_same_baw_start_from_addba_req:add exception}");
            hmac_btcoex_add_exception_to_list(hmac_vap, frame_hdr->auc_address2);
        }
        if (BTCOEX_BLACKLIST_TPYE_FIX_BASIZE == HMAC_BTCOEX_GET_BLACKLIST_TYPE(hmac_user)) {
            /* 发送去认证帧到AP */
            hmac_mgmt_send_disassoc_frame(&hmac_vap->st_vap_base_info, frame_hdr->auc_address2, MAC_UNSPEC_REASON,
                                          hmac_user->st_user_base_info.st_cap_info.bit_pmf_active);
            /* 删除对应用户 */
            hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);
            disconnect.reason_code = MAC_AUTH_NOT_VALID;
            disconnect.type = WLAN_FC0_SUBTYPE_DISASSOC;
            disconnect.is_roam = OAL_FALSE;
            hmac_sta_handle_disassoc_rsp(hmac_vap, &disconnect);
        }
    }
    hmac_btcoex_addba_req->us_last_baw_start = ba_seqctrl->bit_startseqnum;
    hmac_btcoex_addba_req->us_last_seq_num = frame_hdr->bit_seq_num;
}


uint32_t hmac_config_btcoex_set_perf_param(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_BTCOEX_SET_PERF_PARAM, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_COEX,
                         "{hmac_config_btcoex_set_perf_param::send event return err code [%d].}", ret);
    }
    return ret;
}


uint32_t hmac_btcoex_check_by_ba_size(hmac_user_stru *hmac_user)
{
    hmac_user_btcoex_stru *hmac_user_btcoex;

    hmac_user_btcoex = &(hmac_user->st_hmac_user_btcoex);
    if ((hmac_user_btcoex->us_ba_size > 0) && (hmac_user_btcoex->us_ba_size < WLAN_AMPDU_RX_BA_LUT_WSIZE)) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

#define MAC_BTCOEX_CHECK_VALID_STA(_mac_vap) \
    ((IS_STA(_mac_vap) && (((_mac_vap)->st_cap_flag.bit_bt20dbm) || \
    (WLAN_BAND_2G == (_mac_vap)->st_channel.en_band))) ? OAL_TRUE : OAL_FALSE)
uint8_t hmac_btcoex_check_addba_req(void *vap, void *user)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    hmac_user_stru *hmac_user = (hmac_user_stru *)user;
    /* 共存黑名单用户，不建立聚合，直到对应业务将标记清除 */
    if (OAL_TRUE == MAC_BTCOEX_CHECK_VALID_STA(&(hmac_vap->st_vap_base_info))) {
        if (hmac_user->st_hmac_user_btcoex.st_hmac_btcoex_addba_req.en_ba_handle_allow == OAL_FALSE) {
            oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                             "{hmac_btcoex_check_addba_req::btcoex blacklist user, not addba!");
            return MAC_REQ_DECLINED;
        }
    }
    return MAC_SUCCESSFUL_STATUSCODE;
}

void hmac_btcoex_set_addba_rsp_ba_param(
    void *vap, void *user, void *ba_rx_info, uint16_t *ba_param)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    hmac_user_stru *hmac_user = (hmac_user_stru *)user;
    hmac_ba_rx_stru *hmac_ba_rx_info = (hmac_ba_rx_stru *)ba_rx_info;
    hmac_user_btcoex_stru *hmac_user_btcoex = NULL;
    if (MAC_BTCOEX_CHECK_VALID_STA(&(hmac_vap->st_vap_base_info)) == OAL_TRUE) {
        hmac_user_btcoex = &(hmac_user->st_hmac_user_btcoex);
        /* 1.黑名单用户 */
        if (hmac_user_btcoex->st_hmac_btcoex_addba_req.en_ba_handle_allow == OAL_FALSE) {
            /* BIT6~BIT15赋值前需要清理内容 */
            *ba_param &= (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5);
            if (BTCOEX_BLACKLIST_TPYE_FIX_BASIZE == HMAC_BTCOEX_GET_BLACKLIST_TYPE(hmac_user)) {
                *ba_param |= (uint16_t)(BTCOEX_BLACKLIST_BA_SIZE_LIMIT << 6); /* BA SIZE在BA param set的BIT6~BIT15 */
            } else {
                /* 黑名单时，btcoex聚合业务处于结束状态，按照默认聚合个数恢复wifi性能 */
                *ba_param |= (uint16_t)(hmac_ba_rx_info->us_baw_size << 6); /* BA SIZE在BA param set的BIT6~BIT15 */
            }
        } else if ((hmac_user_btcoex->en_delba_btcoex_trigger == OAL_TRUE) &&
                   (hmac_user_btcoex->us_ba_size != 0)) { /* 2.共存触发的删除，按照共存配置来 */
            /* BIT6~BIT15赋值前需要清理内容 */
            *ba_param &= (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5);
            *ba_param |= (uint16_t)(hmac_user_btcoex->us_ba_size << 6); /* 最大聚合个数是BA param set的BIT6~BIT15 */
        }
    }
}

/*
* 功能描述  : BT状态同步到HMAC
*/
uint32_t hmac_btcoex_d2h_bt_state_syn(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    return OAL_SUCC;
}
#endif
