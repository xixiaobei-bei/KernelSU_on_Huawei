
/* 1 头文件包含 */
#include "mac_ie.h"
#include "mac_regdomain.h"
#include "mac_device.h"
#include "wlan_types.h"
#include "hmac_mgmt_sta.h"
#include "hmac_sme_sta.h"
#include "hmac_fsm.h"
#include "hmac_dfs.h"
#include "hmac_chan_mgmt.h"
#include "mac_device.h"
#include "hmac_scan.h"
#include "frw_ext_if.h"
#include "hmac_resource.h"
#include "securec.h"
#include "mac_mib.h"
#include "wlan_chip_i.h"
#ifdef _PRE_WLAN_FEATURE_HID2D
#include "hmac_hid2d.h"
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CHAN_MGMT_C

/* 2 全局变量定义 */
#define HMAC_CENTER_FREQ_2G_40M_OFFSET 2 /* 中心频点相对于主信道idx的偏移量 */
#define HMAC_AFFECTED_CH_IDX_OFFSET    5 /* 2.4GHz下，40MHz带宽所影响的信道半径，中心频点 +/- 5个信道 */

/* 3 函数声明 */
/* 4 函数实现 */
void hmac_dump_chan(mac_vap_stru *pst_mac_vap, uint8_t *puc_param)
{
    dmac_set_chan_stru *pst_chan = NULL;

    if (oal_any_null_ptr2(pst_mac_vap, puc_param)) {
        return;
    }

    pst_chan = (dmac_set_chan_stru *)puc_param;
    oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_2040, "channel mgmt info\r\n");
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "uc_chan_number=%d\r\n", pst_chan->st_channel.uc_chan_number);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_band=%d\r\n", pst_chan->st_channel.en_band);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_bandwidth=%d\r\n", pst_chan->st_channel.en_bandwidth);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "uc_idx=%d\r\n", pst_chan->st_channel.uc_chan_idx);

    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "c_announced_channel=%d\r\n",
                  pst_chan->st_ch_switch_info.uc_announced_channel);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_announced_bandwidth=%d\r\n",
                  pst_chan->st_ch_switch_info.en_announced_bandwidth);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "uc_ch_switch_cnt=%d\r\n",
                  pst_chan->st_ch_switch_info.uc_ch_switch_cnt);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_ch_switch_status=%d\r\n",
                  pst_chan->st_ch_switch_info.en_ch_switch_status);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_bw_switch_status=%d\r\n",
                  pst_chan->st_ch_switch_info.en_bw_switch_status);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_csa_present_in_bcn=%d\r\n",
                  pst_chan->st_ch_switch_info.en_csa_present_in_bcn);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "uc_start_chan_idx=%d\r\n",
                  pst_chan->st_ch_switch_info.uc_start_chan_idx);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "uc_end_chan_idx=%d\r\n",
                  pst_chan->st_ch_switch_info.uc_end_chan_idx);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_user_pref_bandwidth=%d\r\n",
                  pst_chan->st_ch_switch_info.en_user_pref_bandwidth);

    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "uc_new_channel=%d\r\n",
                  pst_chan->st_ch_switch_info.uc_new_channel);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_new_bandwidth=%d\r\n",
                  pst_chan->st_ch_switch_info.en_new_bandwidth);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "en_te_b=%d\r\n", pst_chan->st_ch_switch_info.en_te_b);
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "ul_chan_report_for_te_a=%d\r\n",
                  pst_chan->st_ch_switch_info.chan_report_for_te_a);
}


void hmac_chan_initiate_switch_to_new_channel(mac_vap_stru *pst_mac_vap, uint8_t uc_channel,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    frw_event_mem_stru *pst_event_mem = NULL;
    frw_event_stru *pst_event = NULL;
    uint32_t ret;
    dmac_set_ch_switch_info_stru *pst_ch_switch_info = NULL;
    mac_device_stru *pst_mac_device = NULL;

    /* AP准备切换信道 */
    pst_mac_vap->st_ch_switch_info.en_ch_switch_status = WLAN_CH_SWITCH_STATUS_1;
    pst_mac_vap->st_ch_switch_info.uc_announced_channel = uc_channel;
    pst_mac_vap->st_ch_switch_info.en_announced_bandwidth = en_bandwidth;

    /* 在Beacon帧中添加Channel Switch Announcement IE */
    pst_mac_vap->st_ch_switch_info.en_csa_present_in_bcn = OAL_TRUE;

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_2040,
        "{hmac_chan_initiate_switch_to_new_channel::uc_announced_channel=%d,en_announced_bandwidth=%d csa_cnt=%d}",
        uc_channel, en_bandwidth, pst_mac_vap->st_ch_switch_info.uc_ch_switch_cnt);

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_chan_initiate_switch_to_new_channel::pst_mac_device null.}");
        return;
    }
    /* 申请事件内存 */
    pst_event_mem = frw_event_alloc_m(sizeof(dmac_set_ch_switch_info_stru));
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_chan_initiate_switch_to_new_channel::pst_event_mem null.}");
        return;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_SWITCH_TO_NEW_CHAN,
                       sizeof(dmac_set_ch_switch_info_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    /* 填写事件payload */
    pst_ch_switch_info = (dmac_set_ch_switch_info_stru *)pst_event->auc_event_data;
    pst_ch_switch_info->en_ch_switch_status = WLAN_CH_SWITCH_STATUS_1;
    pst_ch_switch_info->uc_announced_channel = uc_channel;
    pst_ch_switch_info->en_announced_bandwidth = en_bandwidth;
    pst_ch_switch_info->uc_ch_switch_cnt = pst_mac_vap->st_ch_switch_info.uc_ch_switch_cnt;
    pst_ch_switch_info->en_csa_present_in_bcn = OAL_TRUE;
    pst_ch_switch_info->uc_csa_vap_cnt = pst_mac_device->uc_csa_vap_cnt;

    pst_ch_switch_info->en_csa_mode = pst_mac_vap->st_ch_switch_info.en_csa_mode;

    /* 分发事件 */
    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_chan_initiate_switch_to_new_channel::frw_event_dispatch_event failed[%d].}", ret);
        frw_event_free_m(pst_event_mem);
        return;
    }

    /* 释放事件 */
    frw_event_free_m(pst_event_mem);
}


uint32_t hmac_check_ap_channel_follow_sta(mac_vap_stru *pst_check_mac_vap,
    const mac_channel_stru *pst_set_mac_channel, uint8_t *puc_ap_follow_channel)
{
    mac_device_stru *pst_mac_device = NULL;
    uint8_t uc_vap_idx;
    mac_vap_stru *pst_index_mac_vap = NULL;

    if (oal_any_null_ptr3(pst_set_mac_channel, puc_ap_follow_channel, pst_check_mac_vap)) {
        oam_error_log0(0, OAM_SF_2040, "{hmac_check_ap_channel_follow_sta:: input param is null,return}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (!IS_LEGACY_VAP(pst_check_mac_vap)) {
        oam_warning_log1(pst_check_mac_vap->uc_vap_id, OAM_SF_2040,
            "{hmac_check_ap_channel_follow_sta:: vap_p2p_mode=%d, not neet to check}", pst_check_mac_vap->en_p2p_mode);
        return OAL_FAIL;
    }

    if (!IS_STA(pst_check_mac_vap) && !IS_AP(pst_check_mac_vap)) {
        oam_error_log1(pst_check_mac_vap->uc_vap_id, OAM_SF_2040,
                       "{hmac_check_ap_channel_follow_sta:: pst_mac_vap_current->en_vap_mode=%d,not neet to check}\n",
                       pst_check_mac_vap->en_vap_mode);
        return OAL_FAIL;
    }

    pst_mac_device = mac_res_get_dev(pst_check_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log1(pst_check_mac_vap->uc_vap_id, OAM_SF_2040,
            "{hmac_check_ap_channel_follow_sta::get mac_device(%d) is null. Return}", pst_check_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log3(pst_check_mac_vap->uc_vap_id, OAM_SF_2040,
        "{hmac_check_ap_channel_follow_sta:: check_vap_state=%d, check_vap_band=%d  check_vap_channel=%d.}",
        pst_check_mac_vap->en_vap_state, pst_set_mac_channel->en_band, pst_set_mac_channel->uc_chan_number);

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_index_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if ((pst_index_mac_vap == NULL) ||
            (pst_check_mac_vap->uc_vap_id == pst_index_mac_vap->uc_vap_id) ||
            (pst_index_mac_vap->en_p2p_mode != WLAN_LEGACY_VAP_MODE)) {
            continue;
        }
        if ((pst_index_mac_vap->en_vap_state != MAC_VAP_STATE_UP) &&
            (pst_index_mac_vap->en_vap_state != MAC_VAP_STATE_PAUSE)) {
            continue;
        }

        oam_warning_log4(pst_index_mac_vap->uc_vap_id, OAM_SF_2040,
            "{hmac_check_ap_channel_follow_sta::index_vap_state=%d,index_vap_mode=%d,idx_vap_band=%d,idx_vap_chan=%d.}",
            pst_index_mac_vap->en_vap_state, pst_check_mac_vap->en_vap_mode,
            pst_index_mac_vap->st_channel.en_band, pst_index_mac_vap->st_channel.uc_chan_number);

        if (IS_STA(pst_check_mac_vap) && IS_AP(pst_index_mac_vap)) { /* AP先启动;STA后启动 */
            if ((pst_set_mac_channel->en_band == pst_index_mac_vap->st_channel.en_band) &&
                (pst_set_mac_channel->uc_chan_number != pst_index_mac_vap->st_channel.uc_chan_number)) { /* CSA */
                oam_warning_log2(pst_index_mac_vap->uc_vap_id, OAM_SF_2040,
                    "{hmac_check_ap_channel_follow_sta::<vap_current_mode=STA vap_index_mode=Ap> \
                    SoftAp CSA Operate, Channel from [%d] To [%d]}.\n",
                    pst_index_mac_vap->st_channel.uc_chan_number, pst_set_mac_channel->uc_chan_number);
                pst_mac_device->uc_csa_vap_cnt++;
                pst_index_mac_vap->st_ch_switch_info.uc_ch_switch_cnt = HMAC_CHANNEL_SWITCH_COUNT; /* CSA cnt 设置为5 */
                pst_index_mac_vap->st_ch_switch_info.en_csa_mode = WLAN_CSA_MODE_TX_DISABLE;
                hmac_chan_initiate_switch_to_new_channel(pst_index_mac_vap, pst_set_mac_channel->uc_chan_number,
                                                         pst_index_mac_vap->st_channel.en_bandwidth);
                *puc_ap_follow_channel = pst_set_mac_channel->uc_chan_number;
                return OAL_SUCC;
            }
        } else if (IS_AP(pst_check_mac_vap) && IS_STA(pst_index_mac_vap)) { /* STA先启动;AP后启动 */
            if ((pst_set_mac_channel->en_band == pst_index_mac_vap->st_channel.en_band) &&
                (pst_set_mac_channel->uc_chan_number != pst_index_mac_vap->st_channel.uc_chan_number)) {
                /* 替换信道值 */
                *puc_ap_follow_channel = pst_index_mac_vap->st_channel.uc_chan_number;
                oam_warning_log2(pst_index_mac_vap->uc_vap_id, OAM_SF_2040,
                    "{hmac_check_ap_channel_follow_sta::<vap_current_mode=Ap vap_index_mode=Sta> \
                    SoftAp change Channel from [%d] To [%d]}.\n",
                    pst_set_mac_channel->uc_chan_number, *puc_ap_follow_channel);
                return OAL_SUCC;
            }
        }
    }

    return OAL_FAIL;
}

void hmac_chan_sync_init(mac_vap_stru *pst_mac_vap, dmac_set_chan_stru *pst_set_chan)
{
    int32_t l_ret;

    memset_s(pst_set_chan, sizeof(dmac_set_chan_stru), 0, sizeof(dmac_set_chan_stru));
    l_ret = memcpy_s(&pst_set_chan->st_channel, sizeof(mac_channel_stru),
                     &pst_mac_vap->st_channel, sizeof(mac_channel_stru));
    l_ret += memcpy_s(&pst_set_chan->st_ch_switch_info, sizeof(mac_ch_switch_info_stru),
                      &pst_mac_vap->st_ch_switch_info, sizeof(mac_ch_switch_info_stru));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_chan_sync_init::memcpy fail!");
        return;
    }
}


void hmac_chan_do_sync(mac_vap_stru *pst_mac_vap, dmac_set_chan_stru *pst_set_chan)
{
    frw_event_mem_stru *pst_event_mem = NULL;
    frw_event_stru *pst_event = NULL;
    uint32_t ret;
    uint8_t uc_idx;

    hmac_dump_chan(pst_mac_vap, (uint8_t *)pst_set_chan);
    /* 更新VAP下的主20MHz信道号、带宽模式、信道索引 */
    ret = mac_get_channel_idx_from_num(pst_mac_vap->st_channel.en_band, pst_set_chan->st_channel.uc_chan_number,
                                       pst_set_chan->st_channel.ext6g_band, &uc_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_chan_sync::mac_get_channel_idx_from_num failed[%d].}", ret);

        return;
    }

    pst_mac_vap->st_channel.uc_chan_number = pst_set_chan->st_channel.uc_chan_number;
    pst_mac_vap->st_channel.en_bandwidth = pst_set_chan->st_channel.en_bandwidth;
    pst_mac_vap->st_channel.uc_chan_idx = uc_idx;

    /* 申请事件内存 */
    pst_event_mem = frw_event_alloc_m(sizeof(dmac_set_chan_stru));
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_chan_sync::pst_event_mem null.}");
        return;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WALN_CTX_EVENT_SUB_TYPR_SELECT_CHAN,
                       sizeof(dmac_set_chan_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    if (EOK != memcpy_s(frw_get_event_payload(pst_event_mem), sizeof(dmac_set_chan_stru),
                        (uint8_t *)pst_set_chan, sizeof(dmac_set_chan_stru))) {
        oam_error_log0(0, OAM_SF_SCAN, "hmac_chan_do_sync::memcpy fail!");
        frw_event_free_m(pst_event_mem);
        return;
    }

    /* 分发事件 */
    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_chan_sync::frw_event_dispatch_event failed[%d].}", ret);
        frw_event_free_m(pst_event_mem);
        return;
    }

    /* 释放事件 */
    frw_event_free_m(pst_event_mem);
}


void hmac_chan_sync(mac_vap_stru *pst_mac_vap,
                    uint8_t uc_channel, wlan_channel_bandwidth_enum_uint8 en_bandwidth,
                    oal_bool_enum_uint8 en_switch_immediately)
{
    dmac_set_chan_stru st_set_chan;

    hmac_chan_sync_init(pst_mac_vap, &st_set_chan);
    st_set_chan.st_channel.uc_chan_number = uc_channel;
    st_set_chan.st_channel.en_bandwidth = en_bandwidth;
    st_set_chan.en_switch_immediately = en_switch_immediately;
    st_set_chan.en_dot11FortyMHzIntolerant = mac_mib_get_FortyMHzIntolerant(pst_mac_vap);
    wlan_chip_update_cur_chn_cali_data(pst_mac_vap, WLAN_NORMAL_CHN);
    hmac_chan_do_sync(pst_mac_vap, &st_set_chan);
}


void hmac_chan_multi_select_channel_mac(mac_vap_stru *pst_mac_vap, uint8_t uc_channel,
                                        wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    uint8_t uc_vap_idx;
    mac_device_stru *pst_device = NULL;
    mac_vap_stru *pst_vap = NULL;

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                     "{hmac_chan_multi_select_channel_mac::uc_channel=%d,en_bandwidth=%d}",
                     uc_channel, en_bandwidth);

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_device == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                       "{hmac_chan_multi_select_channel_mac::pst_device null,device_id=%d.}",
                       pst_mac_vap->uc_device_id);
        return;
    }

    if (pst_device->uc_vap_num == 0) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{hmac_chan_multi_select_channel_mac::none vap.}");
        return;
    }

    if (mac_is_dbac_running(pst_device)) {
        hmac_chan_sync(pst_mac_vap, uc_channel, en_bandwidth, OAL_TRUE);
        return;
    }

    /* 遍历device下所有vap， */
    for (uc_vap_idx = 0; uc_vap_idx < pst_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == NULL) {
            oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                           "{hmac_chan_multi_select_channel_mac::pst_vap null,vap_id=%d.}",
                           pst_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        hmac_chan_sync(pst_vap, uc_channel, en_bandwidth, OAL_TRUE);
    }
}


void hmac_chan_update_40m_intol_user(mac_vap_stru *pst_mac_vap)
{
    oal_dlist_head_stru *pst_entry = NULL;
    mac_user_stru *pst_mac_user = NULL;

    oal_dlist_search_for_each(pst_entry, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_mac_user = oal_dlist_get_entry(pst_entry, mac_user_stru, st_user_dlist);
        if (oal_unlikely(pst_mac_user == NULL)) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                             "{hmac_chan_update_40m_intol_user::pst_user null pointer.}");
            continue;
        } else {
            if (pst_mac_user->st_ht_hdl.bit_forty_mhz_intolerant) {
                pst_mac_vap->en_40M_intol_user = OAL_TRUE;
                return;
            }
        }
    }

    pst_mac_vap->en_40M_intol_user = OAL_FALSE;
}


OAL_STATIC OAL_INLINE uint8_t hmac_chan_get_user_pref_primary_ch(mac_device_stru *pst_mac_device)
{
    return pst_mac_device->uc_max_channel;
}


OAL_STATIC OAL_INLINE wlan_channel_bandwidth_enum_uint8 hmac_chan_get_user_pref_bandwidth(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->st_ch_switch_info.en_user_pref_bandwidth;
}


void hmac_chan_reval_bandwidth_sta(mac_vap_stru *pst_mac_vap, uint32_t change)
{
    if (oal_unlikely(pst_mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_chan_reval_bandwidth_sta::pst_mac_vap null.}");
        return;
    }

    /* 需要进行带宽切换 */
    if (MAC_BW_CHANGE & change) {
        hmac_chan_multi_select_channel_mac(pst_mac_vap, pst_mac_vap->st_channel.uc_chan_number,
                                           pst_mac_vap->st_channel.en_bandwidth);
    }
}


uint32_t hmac_start_bss_in_available_channel(hmac_vap_stru *pst_hmac_vap)
{
    hmac_ap_start_rsp_stru st_ap_start_rsp;
    uint32_t ret;

    mac_vap_init_rates(&(pst_hmac_vap->st_vap_base_info));

    /* 设置AP侧状态机为 UP */
    hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_UP);

    /* 调用hmac_config_start_vap_event，启动BSS */
    ret = hmac_config_start_vap_event(&(pst_hmac_vap->st_vap_base_info), OAL_TRUE);
    if (oal_unlikely(ret != OAL_SUCC)) {
        hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_INIT);
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_start_bss_in_available_channel::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    /* 设置bssid */
    mac_vap_set_bssid(&pst_hmac_vap->st_vap_base_info, mac_mib_get_StationID(&pst_hmac_vap->st_vap_base_info));

    /* 入网优化，不同频段下的能力不一样 */
    if (WLAN_BAND_2G == pst_hmac_vap->st_vap_base_info.st_channel.en_band) {
        mac_mib_set_SpectrumManagementRequired(&(pst_hmac_vap->st_vap_base_info), OAL_FALSE);
    } else {
        mac_mib_set_SpectrumManagementRequired(&(pst_hmac_vap->st_vap_base_info), OAL_TRUE);
    }

    /* 将结果上报至sme */
    st_ap_start_rsp.en_result_code = HMAC_MGMT_SUCCESS;

    return OAL_SUCC;
}


uint16_t mac_get_center_freq1_from_freq_and_bandwidth(uint16_t freq,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    uint16_t       us_center_freq1;

    /******************************************************
     * channel number to center channel idx map
    BAND WIDTH                            CENTER CHAN INDEX:

    WLAN_BAND_WIDTH_20M                   chan_num
    WLAN_BAND_WIDTH_40PLUS                chan_num + 2
    WLAN_BAND_WIDTH_40MINUS               chan_num - 2
    WLAN_BAND_WIDTH_80PLUSPLUS            chan_num + 6
    WLAN_BAND_WIDTH_80PLUSMINUS           chan_num - 2
    WLAN_BAND_WIDTH_80MINUSPLUS           chan_num + 2
    WLAN_BAND_WIDTH_80MINUSMINUS          chan_num - 6
    WLAN_BAND_WIDTH_160PLUSPLUSPLUS       chan_num + 14
    WLAN_BAND_WIDTH_160MINUSPLUSPLUS      chan_num + 10
    WLAN_BAND_WIDTH_160PLUSMINUSPLUS      chan_num + 6
    WLAN_BAND_WIDTH_160MINUSMINUSPLUS     chan_num + 2
    WLAN_BAND_WIDTH_160PLUSPLUSMINUS      chan_num - 2
    WLAN_BAND_WIDTH_160MINUSPLUSMINUS     chan_num - 6
    WLAN_BAND_WIDTH_160PLUSMINUSMINUS     chan_num - 10
    WLAN_BAND_WIDTH_160MINUSMINUSMINUS    chan_num - 14
    ********************************************************/
    int8_t ac_center_chan_offset[WLAN_BAND_WIDTH_BUTT - 2] = {  /* 减2可以得到中心信道的最大偏移量 */
#ifdef _PRE_WLAN_FEATURE_160M
        0, 2, -2, 6, -2, 2, -6, 14, -2, 6, -10, 10, -6, 2, -14
#else
        0, 2, -2, 6, -2, 2, -6
#endif
    };
    /* 2412为2.4G频点，5825为5G频点 */
    if (freq < 2412 || freq > 5825) {
        return OAL_FAIL;
    }
    /* 减2可以得到支持的最大带宽 */
    if (en_bandwidth >= WLAN_BAND_WIDTH_BUTT - 2) {
        return OAL_FAIL;
    }
    /* 偏移5M */
    us_center_freq1 = freq + ac_center_chan_offset[en_bandwidth] * 5;

    return us_center_freq1;
}


OAL_STATIC uint32_t hmac_report_channel_switch(hmac_vap_stru *pst_hmac_vap, mac_channel_stru *pst_channel)
{
    frw_event_mem_stru *pst_event_mem = NULL;
    frw_event_stru *pst_event = NULL;
    hmac_channel_switch_stru st_channel_switch = {0};
    /* 避免漫游状态csa上报导致supplicant和驱动状态不一致 */
    if (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_ROAMING) {
        oam_warning_log0(0, OAM_SF_CHAN, "{hmac_report_channel_switch::vap in roam state, No csa report!}");
        return OAL_SUCC;
    }

    if (mac_is_channel_num_valid(pst_channel->en_band, pst_channel->uc_chan_number,
        pst_channel->ext6g_band) != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_CHAN, "{hmac_report_channel_switch::invalid channel, en_band=%d, ch_num=%d",
            pst_channel->en_band, pst_channel->uc_chan_number);
        return OAL_FAIL;
    }

    st_channel_switch.center_freq = (pst_channel->en_band == WLAN_BAND_2G) ?
            (g_ast_freq_map_2g[pst_channel->uc_chan_idx].us_freq) :
            (g_ast_freq_map_5g[pst_channel->uc_chan_idx].us_freq);
    st_channel_switch.width = (oal_nl80211_chan_width)WLAN_BANDWIDTH_TO_IEEE_CHAN_WIDTH(pst_channel->en_bandwidth,
        mac_mib_get_HighThroughputOptionImplemented(&pst_hmac_vap->st_vap_base_info));

    st_channel_switch.center_freq1 = mac_get_center_freq1_from_freq_and_bandwidth(st_channel_switch.center_freq,
                                                                                  pst_channel->en_bandwidth);
    /* 5MHz, 10MHz, 20MHz, 40MHz, 80MHz center_freq2 is zero */
    /* 80+80MHz need set center_freq2 */
    oam_warning_log4(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CHAN,
        "hmac_report_channel_switch: center_freq=%d width=%d, center_freq1=%d center_freq2=%d",
        st_channel_switch.center_freq, st_channel_switch.width,
        st_channel_switch.center_freq1, st_channel_switch.center_freq2);

    pst_event_mem = frw_event_alloc_m(sizeof(st_channel_switch));
    if (pst_event_mem == NULL) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CHAN,
            "{hmac_report_channel_switch::frw_event_alloc_m fail! size[%d]}", sizeof(st_channel_switch));
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    pst_event = frw_get_event_stru(pst_event_mem);

    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_CH_SWITCH_NOTIFY,
                       sizeof(st_channel_switch), FRW_EVENT_PIPELINE_STAGE_0,
                       pst_hmac_vap->st_vap_base_info.uc_chip_id,
                       pst_hmac_vap->st_vap_base_info.uc_device_id,
                       pst_hmac_vap->st_vap_base_info.uc_vap_id);

    if (EOK != memcpy_s((uint8_t *)frw_get_event_payload(pst_event_mem), sizeof(st_channel_switch),
                        (uint8_t *)&st_channel_switch, sizeof(st_channel_switch))) {
        oam_error_log0(0, OAM_SF_CHAN, "hmac_report_channel_switch::memcpy fail!");
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);

    return OAL_SUCC;
}


void hmac_chan_restart_network_after_switch(mac_vap_stru *pst_mac_vap)
{
    frw_event_mem_stru *pst_event_mem = NULL;
    frw_event_stru *pst_event = NULL;
    uint32_t ret;

    oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{hmac_chan_restart_network_after_switch}");

    /* 申请事件内存 */
    pst_event_mem = frw_event_alloc_m(0);
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_chan_restart_network_after_switch::pst_event_mem null.}");

        return;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPR_RESTART_NETWORK,
                       0,
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    /* 分发事件 */
    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
            "{hmac_chan_restart_network_after_switch::frw_event_dispatch_event failed[%d].}", ret);
        frw_event_free_m(pst_event_mem);

        return;
    }
    frw_event_free_m(pst_event_mem);
}


uint32_t hmac_chan_switch_to_new_chan_complete(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    mac_device_stru *pst_mac_device = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    dmac_set_chan_stru *pst_set_chan = NULL;
    uint32_t ret;
    uint8_t uc_idx;
    uint8_t uc_ap_follow_channel = 0;
    uint8_t band;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_2040, "{hmac_switch_to_new_chan_complete::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_set_chan = (dmac_set_chan_stru *)pst_event->auc_event_data;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (oal_unlikely(pst_hmac_vap == NULL)) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_2040,
                       "{hmac_switch_to_new_chan_complete::pst_hmac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap = &pst_hmac_vap->st_vap_base_info;

    oam_warning_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_2040, "hmac_chan_switch_to_new_chan_complete");
    hmac_dump_chan(pst_mac_vap, (uint8_t *)pst_set_chan);

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == NULL)) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_2040,
                       "{hmac_switch_to_new_chan_complete::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
    band = mac_get_band_by_channel_num(pst_set_chan->st_channel.uc_chan_number);
    ret = mac_get_channel_idx_from_num(band, pst_set_chan->st_channel.uc_chan_number,
        pst_set_chan->st_channel.ext6g_band, &uc_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                         "{hmac_switch_to_new_chan_complete::mac_get_channel_idx_from_num failed[%d].}", ret);

        return OAL_FAIL;
    }

    pst_mac_vap->st_channel.uc_chan_number = pst_set_chan->st_channel.uc_chan_number;
    pst_mac_vap->st_channel.en_band = pst_set_chan->st_channel.en_band;
    pst_mac_vap->st_channel.en_bandwidth = pst_set_chan->st_channel.en_bandwidth;
    pst_mac_vap->st_channel.uc_chan_idx = uc_idx;

    pst_mac_vap->st_ch_switch_info.en_waiting_to_shift_channel =
        pst_set_chan->st_ch_switch_info.en_waiting_to_shift_channel;

    pst_mac_vap->st_ch_switch_info.en_ch_switch_status = pst_set_chan->st_ch_switch_info.en_ch_switch_status;
    pst_mac_vap->st_ch_switch_info.en_bw_switch_status = pst_set_chan->st_ch_switch_info.en_bw_switch_status;

    /* aput切完信道同步切信道的标志位,防止再有用户关联,把此变量又同步下去 */
    pst_mac_vap->st_ch_switch_info.uc_ch_switch_cnt = pst_set_chan->st_ch_switch_info.uc_ch_switch_cnt;
    pst_mac_vap->st_ch_switch_info.en_csa_present_in_bcn = pst_set_chan->st_ch_switch_info.en_csa_present_in_bcn;

    /* 同步device信息 */
    pst_mac_device->uc_max_channel = pst_mac_vap->st_channel.uc_chan_number;
    pst_mac_device->en_max_band = pst_mac_vap->st_channel.en_band;
    pst_mac_device->en_max_bandwidth = pst_mac_vap->st_channel.en_bandwidth;

#ifdef _PRE_WLAN_FEATURE_HID2D
    /* 通知HiD2D ACS切换成功 */
    hmac_hid2d_acs_switch_completed(pst_mac_vap);
#endif

    /* 信道跟随检查 */
    if (IS_STA(pst_mac_vap)) {
        ret = hmac_check_ap_channel_follow_sta(pst_mac_vap, &pst_mac_vap->st_channel, &uc_ap_follow_channel);
        if (ret == OAL_SUCC) {
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                "{hmac_chan_switch_to_new_chan_complete::ap channel change from %d to %d}",
                pst_mac_vap->st_channel.uc_chan_number, uc_ap_follow_channel);
        }
    }
    ret = hmac_report_channel_switch(pst_hmac_vap, &(pst_mac_vap->st_channel));
    if (ret != OAL_SUCC) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
            "hmac_chan_switch_to_new_chan_complete::hmac_report_channel_switch return fail");
    }

    if (pst_set_chan->en_check_cac == OAL_FALSE) {
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_FEATURE_DFS
    if (OAL_TRUE == hmac_dfs_need_for_cac(pst_mac_device, &pst_hmac_vap->st_vap_base_info)) {
        hmac_dfs_cac_start(pst_mac_device, pst_hmac_vap);
        oam_info_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_2040,
                      "{hmac_chan_switch_to_new_chan_complete::[DFS]CAC START!}");

        return OAL_SUCC;
    }

    hmac_chan_restart_network_after_switch(&(pst_hmac_vap->st_vap_base_info));
#endif

    return OAL_SUCC;
}

void hmac_40m_intol_sync_data(mac_vap_stru *mac_vap,
                              wlan_channel_bandwidth_enum_uint8 bandwidth_40m,
                              oal_bool_enum_uint8 intol_user_40m)
{
    mac_bandwidth_stru band_prot;

    memset_s(&band_prot, sizeof(mac_bandwidth_stru), 0, sizeof(mac_bandwidth_stru));

    band_prot.en_40M_bandwidth = bandwidth_40m;
    band_prot.en_40M_intol_user = intol_user_40m;
    hmac_40M_intol_sync_event(mac_vap, sizeof(band_prot), (uint8_t *)&band_prot);
}
