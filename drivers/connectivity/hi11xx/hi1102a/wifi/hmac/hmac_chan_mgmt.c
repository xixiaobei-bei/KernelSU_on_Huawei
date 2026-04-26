

/* 1 头文件包含 */
#include "mac_ie.h"
#include "mac_regdomain.h"
#include "mac_device.h"
#include "hmac_mgmt_sta.h"
#include "hmac_sme_sta.h"
#include "hmac_fsm.h"
#include "hmac_chan_mgmt.h"
#include "mac_device.h"
#include "hmac_scan.h"
#include "frw_ext_if.h"
#include "hmac_resource.h"
#include "hmac_encap_frame.h"
#include "hmac_204080_coexist.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CHAN_MGMT_C

/* 2 全局变量定义 */
#define HMAC_CENTER_FREQ_2G_40M_OFFSET 2 /* 中心频点相对于主信道idx的偏移量 */
#define HMAC_AFFECTED_CH_IDX_OFFSET    5 /* 2.4GHz下，40MHz带宽所影响的信道半径，中心频点 +/- 5个信道 */
/* 5G频段 信道与中心频率映射 */
OAL_CONST mac_freq_channel_map_stru g_ast_freq_map_5g[MAC_CHANNEL_FREQ_5_BUTT] = {
    { 5180, 36,  0 },
    { 5200, 40,  1 },
    { 5220, 44,  2 },
    { 5240, 48,  3 },
    { 5260, 52,  4 },
    { 5280, 56,  5 },
    { 5300, 60,  6 },
    { 5320, 64,  7 },
    { 5500, 100, 8 },
    { 5520, 104, 9 },
    { 5540, 108, 10 },
    { 5560, 112, 11 },
    { 5580, 116, 12 },
    { 5600, 120, 13 },
    { 5620, 124, 14 },
    { 5640, 128, 15 },
    { 5660, 132, 16 },
    { 5680, 136, 17 },
    { 5700, 140, 18 },
    { 5720, 144, 19 },
    { 5745, 149, 20 },
    { 5765, 153, 21 },
    { 5785, 157, 22 },
    { 5805, 161, 23 },
    { 5825, 165, 24 },
    /* for JP 4.9G */
    { 4920, 184, 25 },
    { 4940, 188, 26 },
    { 4960, 192, 27 },
    { 4980, 196, 28 },
};

/* 2.4G频段 信道与中心频率映射 */
OAL_CONST mac_freq_channel_map_stru g_ast_freq_map_2g[MAC_CHANNEL_FREQ_2_BUTT] = {
    { 2412, 1,  0 },
    { 2417, 2,  1 },
    { 2422, 3,  2 },
    { 2427, 4,  3 },
    { 2432, 5,  4 },
    { 2437, 6,  5 },
    { 2442, 7,  6 },
    { 2447, 8,  7 },
    { 2452, 9,  8 },
    { 2457, 10, 9 },
    { 2462, 11, 10 },
    { 2467, 12, 11 },
    { 2472, 13, 12 },
    { 2484, 14, 13 },
};
/* 4 函数实现 */
oal_void hmac_dump_chan(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_param)
{
    dmac_set_chan_stru *pst_chan = OAL_PTR_NULL;

    if ((pst_mac_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL)) {
        return;
    }

    pst_chan = (dmac_set_chan_stru *)puc_param;
    return;
}


oal_void hmac_send_ht_notify_chan_width(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_data)
{
    oal_uint16                    us_frame_len;
    oal_netbuf_stru              *pst_netbuf = OAL_PTR_NULL;
    mac_tx_ctl_stru              *pst_tx_ctl = OAL_PTR_NULL;
    oal_uint32                    ul_ret;

    if ((pst_mac_vap == OAL_PTR_NULL) || (puc_data == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_2040, "{chan_width::param null, %p %p.}", (uintptr_t)pst_mac_vap, (uintptr_t)puc_data);
        return;
    }

    /* 申请ht_notify_chan_width 帧空间 */
    pst_netbuf = (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF,
                                                         MAC_80211_FRAME_LEN + MAC_HT_NOTIFY_CHANNEL_WIDTH_LEN,
                                                         OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                         "{hmac_send_ht_notify_chan_width::Alloc netbuf(size %d) null in normal_netbuf.}",
                         MAC_80211_FRAME_LEN + MAC_HT_NOTIFY_CHANNEL_WIDTH_LEN);
        return;
    }

    /* 封装SA Query request帧 */
    memset_s(oal_netbuf_cb(pst_netbuf), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    us_frame_len = hmac_encap_notify_chan_width(pst_mac_vap, (oal_uint8 *)oal_netbuf_header(pst_netbuf), puc_data);

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf); /* 获取cb结构体 */
    pst_tx_ctl->us_mpdu_len = us_frame_len;                    /* dmac发送需要的mpdu长度 */
    pst_tx_ctl->us_tx_user_idx = MAC_INVALID_USER_ID;          /* 发送完成需要获取user结构体 */
    oal_netbuf_put(pst_netbuf, us_frame_len);

    /* Buffer this frame in the Memory Queue for transmission */
    ul_ret = hmac_tx_mgmt_send_event(pst_mac_vap, pst_netbuf, us_frame_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{ht_notify_chan_width::send_event failed[%d].}", ul_ret);
        return;
    }

    return;
}


oal_void hmac_chan_multi_switch_to_20MHz_ap(hmac_vap_stru *pst_hmac_vap)
{
    oal_uint8          uc_vap_idx;
    mac_device_stru   *pst_device = OAL_PTR_NULL;
    mac_vap_stru      *pst_mac_vap = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    hmac_vap_stru     *pst_hmac_vap_iter = OAL_PTR_NULL;
#endif

    oam_info_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_2040,
                  "{hmac_chan_multi_switch_to_20MHz_ap::bit_2040_channel_switch_prohibited=%d}",
                  pst_hmac_vap->en_2040_switch_prohibited);

    /* 如果不允许切换带宽，则直接返回 */
    if (pst_hmac_vap->en_2040_switch_prohibited == OAL_TRUE) {
        return;
    }

    pst_device = mac_res_get_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_2040,
                       "{hmac_chan_multi_switch_to_20MHz_ap::pst_device null,uc_device_id=%d.}",
                       pst_hmac_vap->st_vap_base_info.uc_device_id);
        return;
    }

    if (pst_device->uc_vap_num == 0) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_2040,
                       "{hmac_chan_multi_switch_to_20MHz_ap::pst_device null,none vap.}");
        return;
    }

    if (mac_is_dbac_running(pst_device)) {
        pst_mac_vap = &pst_hmac_vap->st_vap_base_info;
        pst_hmac_vap->en_40m_bandwidth = pst_mac_vap->st_channel.en_bandwidth;
        hmac_chan_initiate_switch_to_20mhz_ap(&pst_hmac_vap->st_vap_base_info);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        hmac_chan_sync(pst_mac_vap, pst_mac_vap->st_channel.uc_chan_number,
                       pst_mac_vap->st_channel.en_bandwidth, OAL_FALSE);
#endif
        hmac_send_ht_notify_chan_width(pst_mac_vap, BROADCAST_MACADDR);
        return;
    }

    /* 遍历device下所有vap，设置VAP信道参数，准备切换至20MHz运行 */
    for (uc_vap_idx = 0; uc_vap_idx < pst_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_device->auc_vap_id[uc_vap_idx]);
        if (pst_mac_vap == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_2040,
                           "{hmac_chan_multi_switch_to_20MHz_ap::pst_mac_vap null,vap_idx=%d.}",
                           pst_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
        pst_hmac_vap_iter = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
        if (pst_hmac_vap_iter == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_2040,
                           "{hmac_chan_multi_switch_to_20MHz_ap::pst_hmac_vap_iter null,vap_idx=%d.}",
                           pst_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        /* 40M切20M前记录40M带宽，40M恢复定时器将会使用这个信道进行恢复 */
        pst_hmac_vap_iter->en_40m_bandwidth = pst_mac_vap->st_channel.en_bandwidth;
#endif

        hmac_chan_initiate_switch_to_20mhz_ap(pst_mac_vap);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        hmac_chan_sync(pst_mac_vap, pst_mac_vap->st_channel.uc_chan_number,
                       pst_mac_vap->st_channel.en_bandwidth, OAL_FALSE);
#endif

        hmac_send_ht_notify_chan_width(pst_mac_vap, BROADCAST_MACADDR);
    }
}


oal_void hmac_chan_initiate_switch_to_new_channel(mac_vap_stru *pst_mac_vap, oal_uint8 uc_channel,
                                                  wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    frw_event_mem_stru            *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru                *pst_event = OAL_PTR_NULL;
    oal_uint32                     ul_ret;
    dmac_set_ch_switch_info_stru  *pst_ch_switch_info = OAL_PTR_NULL;
    mac_device_stru               *pst_mac_device = OAL_PTR_NULL;

    /* AP准备切换信道 */
    pst_mac_vap->st_ch_switch_info.en_ch_switch_status = WLAN_CH_SWITCH_STATUS_1;
    pst_mac_vap->st_ch_switch_info.uc_announced_channel = uc_channel;
    pst_mac_vap->st_ch_switch_info.en_announced_bandwidth = en_bandwidth;

    /* 在Beacon帧中添加Channel Switch Announcement IE */
    pst_mac_vap->st_ch_switch_info.en_csa_present_in_bcn = OAL_TRUE;

    oam_info_log2(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                  "{hmac_chan_initiate_switch_to_new_channel::uc_announced_channel=%d,en_announced_bandwidth=%d}",
                  uc_channel, en_bandwidth);

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_chan_initiate_switch_to_new_channel::pst_mac_device null.}");
        return;
    }
    /* 申请事件内存 */
    pst_event_mem = frw_event_alloc_m(OAL_SIZEOF(dmac_set_ch_switch_info_stru));
    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_chan_initiate_switch_to_new_channel::pst_event_mem null.}");
        return;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_SWITCH_TO_NEW_CHAN,
                       OAL_SIZEOF(dmac_set_ch_switch_info_stru),
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
    pst_ch_switch_info->en_csa_mode = pst_mac_vap->st_ch_switch_info.en_csa_mode;
    pst_ch_switch_info->uc_csa_vap_cnt = pst_mac_device->uc_csa_vap_cnt;

    /* 分发事件 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_chan_initiate_switch_to_new_channel::frw_event_dispatch_event failed[%d].}", ul_ret);
        frw_event_free_m(pst_event_mem);
        return;
    }

    /* 释放事件 */
    frw_event_free_m(pst_event_mem);
}


oal_void hmac_chan_multi_switch_to_new_channel(mac_vap_stru *pst_mac_vap, oal_uint8 uc_channel,
                                               wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    oal_uint8          uc_vap_idx;
    mac_device_stru   *pst_device = OAL_PTR_NULL;
    mac_vap_stru      *pst_ap = OAL_PTR_NULL;

    oam_info_log2(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                  "{hmac_chan_multi_switch_to_new_channel::uc_channel=%d,en_bandwidth=%d}",
                  uc_channel, en_bandwidth);

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_chan_multi_switch_to_new_channel::pstdevice null.}");
        return;
    }

    if (pst_device->uc_vap_num == 0) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_chan_multi_switch_to_new_channel::none vap.}");
        return;
    }

    /* 遍历device下所有ap，设置ap信道参数，准备切换至新信道运行 */
    for (uc_vap_idx = 0; uc_vap_idx < pst_device->uc_vap_num; uc_vap_idx++) {
        pst_ap = (mac_vap_stru *)mac_res_get_mac_vap(pst_device->auc_vap_id[uc_vap_idx]);
        if (pst_ap == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_chan_multi_switch_to_new_channel::pst_ap null.}",
                           pst_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        /* 只更新AP侧的信道切换信息 */
        if (pst_ap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
            continue;
        }

        pst_ap->st_ch_switch_info.en_csa_mode = WLAN_CSA_MODE_TX_DISABLE;
        hmac_chan_initiate_switch_to_new_channel(pst_ap, uc_channel, en_bandwidth);
    }
}

oal_void hmac_chan_sync_init(mac_vap_stru *pst_mac_vap, dmac_set_chan_stru *pst_set_chan)
{
    oal_int32 l_ret;

    memset_s(pst_set_chan, OAL_SIZEOF(dmac_set_chan_stru), 0, OAL_SIZEOF(dmac_set_chan_stru));
    l_ret = memcpy_s(&pst_set_chan->st_channel, OAL_SIZEOF(mac_channel_stru),
                     &pst_mac_vap->st_channel, OAL_SIZEOF(mac_channel_stru));
    l_ret += memcpy_s(&pst_set_chan->st_ch_switch_info, OAL_SIZEOF(mac_ch_switch_info_stru),
                      &pst_mac_vap->st_ch_switch_info, OAL_SIZEOF(mac_ch_switch_info_stru));
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_chan_sync_init::memcpy fail!");
        return;
    }
}


oal_void hmac_chan_do_sync(mac_vap_stru *pst_mac_vap, dmac_set_chan_stru *pst_set_chan)
{
    frw_event_mem_stru       *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru           *pst_event = OAL_PTR_NULL;
    oal_uint32                ul_ret;
    oal_uint8                 uc_idx;

    hmac_dump_chan(pst_mac_vap, (oal_uint8 *)pst_set_chan);
    /* 更新VAP下的主20MHz信道号、带宽模式、信道索引 */
    ul_ret = mac_get_channel_idx_from_num(pst_mac_vap->st_channel.en_band,
                                          pst_set_chan->st_channel.uc_chan_number, &uc_idx);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_chan_sync::get_channel_idx failed[%d].}", ul_ret);

        return;
    }

    pst_mac_vap->st_channel.uc_chan_number = pst_set_chan->st_channel.uc_chan_number;
    pst_mac_vap->st_channel.en_bandwidth = pst_set_chan->st_channel.en_bandwidth;
    pst_mac_vap->st_channel.uc_idx = uc_idx;

    /* 申请事件内存 */
    pst_event_mem = frw_event_alloc_m(OAL_SIZEOF(dmac_set_chan_stru));
    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_chan_sync::pst_event_mem null.}");
        return;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WALN_CTX_EVENT_SUB_TYPR_SELECT_CHAN,
                       OAL_SIZEOF(dmac_set_chan_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    if (memcpy_s(frw_get_event_payload(pst_event_mem), OAL_SIZEOF(dmac_set_chan_stru),
                 (oal_uint8 *)pst_set_chan, OAL_SIZEOF(dmac_set_chan_stru)) != EOK) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "hmac_chan_sync::memcpy fail!");
        frw_event_free_m(pst_event_mem);
        return;
    }
    /* 分发事件 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_chan_sync::frw_event_dispatch_event failed[%d].}", ul_ret);
        frw_event_free_m(pst_event_mem);
        return;
    }

    /* 释放事件 */
    frw_event_free_m(pst_event_mem);
}


oal_void hmac_chan_sync(mac_vap_stru *pst_mac_vap,
                        oal_uint8 uc_channel, wlan_channel_bandwidth_enum_uint8 en_bandwidth,
                        oal_bool_enum_uint8 en_switch_immediately)
{
    dmac_set_chan_stru st_set_chan;

    hmac_chan_sync_init(pst_mac_vap, &st_set_chan);
    st_set_chan.st_channel.uc_chan_number = uc_channel;
    st_set_chan.st_channel.en_bandwidth = en_bandwidth;
    st_set_chan.en_switch_immediately = en_switch_immediately;
    hmac_chan_do_sync(pst_mac_vap, &st_set_chan);
}


oal_void hmac_chan_multi_select_channel_mac(mac_vap_stru *pst_mac_vap, oal_uint8 uc_channel,
                                            wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    oal_uint8 uc_vap_idx;
    mac_device_stru *pst_device = OAL_PTR_NULL;
    mac_vap_stru *pst_vap = OAL_PTR_NULL;

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                     "{hmac_chan_multi_select_channel_mac::uc_channel=%d,en_bandwidth=%d}", uc_channel, en_bandwidth);

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                       "{hmac_chan_multi_select_channel_mac::pstdevice null,device_id=%d.}", pst_mac_vap->uc_device_id);
        return;
    }

    if (pst_device->uc_vap_num == 0) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{hmac_chan_multi_select_channel_mac::none vap.}");
        return;
    }

    if (mac_is_dbac_running(pst_device)) {
        hmac_chan_sync(pst_mac_vap, uc_channel, en_bandwidth, OAL_TRUE);
        return;
    }

    /* 遍历device下所有vap， */
    for (uc_vap_idx = 0; uc_vap_idx < pst_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                           "{hmac_chan_multi_select_channel_mac::pst_vap null,vap_id=%d.}",
                           pst_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        hmac_chan_sync(pst_vap, uc_channel, en_bandwidth, OAL_TRUE);
    }
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_chan_get_40mhz_possibility(
    mac_vap_stru *pst_mac_vap,
    hmac_eval_scan_report_stru *pst_chan_scan_report)
{
    oal_bool_enum_uint8 en_fortymhz_poss = OAL_FALSE;

    oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{hmac_chan_get_40mhz_possibility}");
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    if (mac_mib_get_FortyMHzOperationImplemented(pst_mac_vap) == OAL_TRUE) {
        en_fortymhz_poss = hmac_chan_get_2040_op_chan_list(pst_mac_vap, pst_chan_scan_report);
    }
#endif

    return en_fortymhz_poss;
}


OAL_STATIC OAL_INLINE oal_void hmac_chan_init_chan_scan_report(mac_device_stru *pst_mac_device,
                                                               hmac_eval_scan_report_stru *pst_chan_scan_report,
                                                               oal_uint8 uc_num_supp_chan)
{
    oal_uint8 uc_idx;

    memset_s(pst_chan_scan_report, uc_num_supp_chan * OAL_SIZEOF(*pst_chan_scan_report),
        0, uc_num_supp_chan * OAL_SIZEOF(*pst_chan_scan_report));

    for (uc_idx = 0; uc_idx < uc_num_supp_chan; uc_idx++) {
        pst_chan_scan_report[uc_idx].en_chan_op |= HMAC_OP_ALLOWED;
    }
}


OAL_STATIC oal_bool_enum_uint8 hmac_chan_is_40mhz_sca_allowed(mac_vap_stru *pst_mac_vap,
                                                              hmac_eval_scan_report_stru *pst_chan_scan_report,
                                                              oal_uint8 uc_pri_chan_idx,
                                                              mac_sec_ch_off_enum_uint8 en_user_chan_offset)
{
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    wlan_channel_band_enum_uint8    en_band = pst_mac_vap->st_channel.en_band;
    oal_uint8                       uc_num_supp_chan = mac_get_num_supp_channel(en_band);
    oal_uint8                       uc_sec_chan_idx;
    oal_uint32                      ul_ret;

    oam_info_log2(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                  "{hmac_chan_is_40mhz_sca_allowed::uc_pri_chan_idx=%d, en_user_chan_offset=%d}",
                  uc_pri_chan_idx, en_user_chan_offset);

    if (en_user_chan_offset == MAC_SCB) {
        return OAL_FALSE;
    }

    if ((mac_mib_get_2040BSSCoexistenceManagementSupport(pst_mac_vap) == OAL_TRUE) &&
        (en_band == WLAN_BAND_2G)) {
        if (!(pst_chan_scan_report[uc_pri_chan_idx].en_chan_op & HMAC_SCA_ALLOWED)) {
            return OAL_FALSE;
        }
    }

    uc_sec_chan_idx = uc_pri_chan_idx + mac_get_sec_ch_idx_offset(en_band);
    if (uc_sec_chan_idx >= uc_num_supp_chan) {
        return OAL_FALSE;
    }

    ul_ret = mac_is_channel_idx_valid(en_band, uc_sec_chan_idx);
    if (ul_ret != OAL_SUCC) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
#else
    return OAL_FALSE;
#endif
}


OAL_STATIC oal_bool_enum_uint8 hmac_chan_is_40mhz_scb_allowed(mac_vap_stru *pst_mac_vap,
                                                              hmac_eval_scan_report_stru *pst_chan_scan_report,
                                                              oal_uint8 uc_pri_chan_idx,
                                                              mac_sec_ch_off_enum_uint8 en_user_chan_offset)
{
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    wlan_channel_band_enum_uint8    en_band = pst_mac_vap->st_channel.en_band;
    oal_uint8                       uc_sec_ch_idx_offset = mac_get_sec_ch_idx_offset(en_band);
    oal_uint8                       uc_sec_chan_idx = 0;
    oal_uint32                      ul_ret;

    oam_info_log2(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                  "{hmac_chan_is_40mhz_sca_allowed::uc_pri_chan_idx=%d, en_user_chan_offset=%d}",
                  uc_pri_chan_idx, en_user_chan_offset);

    if (en_user_chan_offset == MAC_SCA) {
        return OAL_FALSE;
    }

    if ((mac_mib_get_2040BSSCoexistenceManagementSupport(pst_mac_vap) == OAL_TRUE) &&
        (en_band == WLAN_BAND_2G)) {
        if (!(pst_chan_scan_report[uc_pri_chan_idx].en_chan_op & HMAC_SCB_ALLOWED)) {
            return OAL_FALSE;
        }
    }

    if (uc_pri_chan_idx >= uc_sec_ch_idx_offset) {
        uc_sec_chan_idx = uc_pri_chan_idx - uc_sec_ch_idx_offset;
    } else {
        return OAL_FALSE;
    }

    ul_ret = mac_is_channel_idx_valid(en_band, uc_sec_chan_idx);
    if (ul_ret != OAL_SUCC) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
#else
    return OAL_FALSE;
#endif
}


OAL_STATIC oal_uint16 hmac_chan_get_cumulative_networks(mac_device_stru *pst_mac_device,
                                                        wlan_channel_bandwidth_enum_uint8 en_band,
                                                        oal_uint8 uc_pri_chan_idx)
{
    oal_uint16   us_cumulative_networks = 0;
    oal_uint8    uc_num_supp_chan = mac_get_num_supp_channel(en_band);
    oal_uint8    uc_affected_ch_idx_offset = mac_get_affected_ch_idx_offset(en_band);
    oal_uint8    uc_affected_chan_lo, uc_affected_chan_hi;
    oal_uint8    uc_chan_idx;
    oal_uint32   ul_ret;

    uc_affected_chan_lo =
    (uc_pri_chan_idx >= uc_affected_ch_idx_offset) ? (uc_pri_chan_idx - uc_affected_ch_idx_offset) : 0;

    uc_affected_chan_hi =
    (uc_num_supp_chan > uc_pri_chan_idx + uc_affected_ch_idx_offset) ?
    (uc_pri_chan_idx + uc_affected_ch_idx_offset) : (uc_num_supp_chan - 1);

    for (uc_chan_idx = uc_affected_chan_lo; uc_chan_idx <= uc_affected_chan_hi; uc_chan_idx++) {
        ul_ret = mac_is_channel_idx_valid(en_band, uc_chan_idx);
        if (ul_ret == OAL_SUCC) {
            us_cumulative_networks += pst_mac_device->st_ap_channel_list[uc_pri_chan_idx].us_num_networks;
        }
    }

    return us_cumulative_networks;
}


OAL_STATIC OAL_INLINE oal_uint8 hmac_chan_get_user_pref_primary_ch(mac_device_stru *pst_mac_device)
{
    return pst_mac_device->uc_max_channel;
}


OAL_STATIC OAL_INLINE wlan_channel_bandwidth_enum_uint8 hmac_chan_get_user_pref_bandwidth(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->st_ch_switch_info.en_user_pref_bandwidth;
}
static oal_uint32 hmac_get_channel_idx_from_num(mac_vap_stru *mac_vap,
                                                mac_device_stru *mac_device, oal_uint8 *user_chan_idx)
{
    oal_uint32 ret;
    if (mac_is_dbac_enabled(mac_device)) {
        ret = mac_get_channel_idx_from_num(mac_vap->st_channel.en_band,
                                           mac_vap->st_channel.uc_chan_number, user_chan_idx);
    } else {
        ret = mac_get_channel_idx_from_num(mac_vap->st_channel.en_band,
                                           hmac_chan_get_user_pref_primary_ch(mac_device),
                                           user_chan_idx);
    }
    return ret;
}

static void hmac_get_channel_idx_fail(mac_vap_stru *mac_vap, oal_uint32 ret)
{
    OAM_WARNING_LOG1(mac_vap->uc_vap_id, OAM_SF_SCAN,
                     "{chan_select_channel_for_operation::mac_get_channel_idx_from_num failed[%d].}", ret);
}

static void hmac_select_channel_print(mac_vap_stru *mac_vap,
                                      mac_sec_ch_off_enum_uint8 user_chan_offset, oal_uint8 user_chan_idx)
{
    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_SCAN,
                     "{hmac_chan_select_channel_for_operation::User Preferred Channel id=%d sco=%d.}",
                     user_chan_idx, user_chan_offset);
}

oal_uint32 hmac_chan_select_channel_for_operation(mac_vap_stru *pst_mac_vap,
                                                  oal_uint8 *puc_new_channel,
                                                  wlan_channel_bandwidth_enum_uint8 *pen_new_bandwidth)
{
    mac_device_stru                     *pst_mac_device = OAL_PTR_NULL;
    hmac_eval_scan_report_stru          *pst_chan_scan_report = OAL_PTR_NULL;
    oal_uint8                            uc_least_busy_chan_idx = 0xFF;
    oal_uint16                           us_least_networks = 0xFFFF;
    oal_uint16                           us_cumulative_networks = 0;
    oal_bool_enum_uint8                  en_fortymhz_poss;
    oal_bool_enum_uint8                  en_rslt = OAL_FALSE;
    mac_sec_ch_off_enum_uint8            en_user_chan_offset = MAC_SEC_CH_BUTT;
    mac_sec_ch_off_enum_uint8            en_chan_offset = MAC_SCN;
    oal_uint8                            uc_user_chan_idx = 0xFF;
    oal_uint8                            uc_chan_idx = 0xFF;
    oal_uint8                            uc_num_supp_chan = mac_get_num_supp_channel(pst_mac_vap->st_channel.en_band);
    oal_uint8                            uc_max_supp_channle = MAC_MAX_SUPP_CHANNEL;
    oal_uint32                           ul_ret = OAL_FAIL;

    // cppcheck-suppress * ignore Uninitialized variable: pst_chan_scan_report
    pst_chan_scan_report = (hmac_eval_scan_report_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, uc_max_supp_channle *
                                                                         OAL_SIZEOF(hmac_eval_scan_report_stru),
                                                                         OAL_TRUE);
    if (oal_unlikely(pst_chan_scan_report == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{memalloc failed, size[%d].}", (uc_max_supp_channle * OAL_SIZEOF(hmac_eval_scan_report_stru)));
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{mac_device[%d] null}", pst_mac_vap->uc_device_id);
        oal_mem_free_m((oal_void *)pst_chan_scan_report, OAL_TRUE);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_chan_init_chan_scan_report(pst_mac_device, pst_chan_scan_report, uc_num_supp_chan);

    /* 如自动信道选择没有开启，则获取用户选择的主信道号，以及带宽模式 */
    if (mac_device_is_auto_chan_sel_enabled(pst_mac_device) == OAL_FALSE) {
        ul_ret = hmac_get_channel_idx_from_num(pst_mac_vap, pst_mac_device, &uc_user_chan_idx);
        if (ul_ret != OAL_SUCC) {
            hmac_get_channel_idx_fail(pst_mac_vap, ul_ret);
            oal_mem_free_m((oal_void *)pst_chan_scan_report, OAL_TRUE);

            return ul_ret;
        }

        en_user_chan_offset = mac_get_sco_from_bandwidth(hmac_chan_get_user_pref_bandwidth(pst_mac_vap));
        hmac_select_channel_print(pst_mac_vap, en_user_chan_offset, uc_user_chan_idx);
    }

    /* 判断在当前环境下是否能够建立40MHz BSS */
    en_fortymhz_poss = hmac_chan_get_40mhz_possibility(pst_mac_vap, pst_chan_scan_report);

    /* 用户选择了主信道 */
    if (uc_user_chan_idx != 0xFF) {
        /* 如果能够建立40MHz BSS，并且用户也希望建立40MHz */
        if ((en_fortymhz_poss == OAL_TRUE) && (en_user_chan_offset != MAC_SCN)) {
            if (en_user_chan_offset == MAC_SCA) {
                en_rslt = hmac_chan_is_40mhz_sca_allowed(pst_mac_vap, pst_chan_scan_report, uc_user_chan_idx,
                                                         en_user_chan_offset);
            } else if (en_user_chan_offset == MAC_SCB) {
                en_rslt = hmac_chan_is_40mhz_scb_allowed(pst_mac_vap, pst_chan_scan_report, uc_user_chan_idx,
                                                         en_user_chan_offset);
            }
        }

        if (en_rslt == OAL_TRUE) {
            en_chan_offset = en_user_chan_offset;
        }

        uc_least_busy_chan_idx = uc_user_chan_idx;
    } else { /* 用户没有选择信道，自动选择一条最不繁忙的信道(对) */
        for (uc_chan_idx = 0; uc_chan_idx < uc_num_supp_chan; uc_chan_idx++) {
            if (!(pst_chan_scan_report[uc_chan_idx].en_chan_op & HMAC_OP_ALLOWED)) {
                continue;
            }

            /* 判断主信道索引号是否有效 */
            ul_ret = mac_is_channel_idx_valid(pst_mac_vap->st_channel.en_band, uc_chan_idx);
            if (ul_ret != OAL_SUCC) {
                continue;
            }

            /* 能够建立40MHz BSS */
            if (en_fortymhz_poss == OAL_TRUE) {
                /* 判断主信道上(右)面的信道是否可作为次信道 */
                en_rslt = hmac_chan_is_40mhz_sca_allowed(pst_mac_vap, pst_chan_scan_report, uc_chan_idx,
                                                         en_user_chan_offset);
                if (en_rslt == OAL_TRUE) {
                    /* 如果这条信道最不繁忙，则选择这条信道作为"当前最不繁忙信道" */
                    if (pst_chan_scan_report[uc_chan_idx].aus_num_networks[HMAC_NETWORK_SCA] < us_least_networks) {
                        us_least_networks = pst_chan_scan_report[uc_chan_idx].aus_num_networks[HMAC_NETWORK_SCA];
                        uc_least_busy_chan_idx = uc_chan_idx;
                        en_chan_offset = MAC_SCA;
                    }
                }

                /* 判断主信道下(左)面的信道是否可作为次信道 */
                en_rslt = hmac_chan_is_40mhz_scb_allowed(pst_mac_vap, pst_chan_scan_report, uc_chan_idx,
                                                         en_user_chan_offset);
                if (en_rslt == OAL_TRUE) {
                    /* 如果这条信道最不繁忙，则选择这条信道作为"当前最不繁忙信道" */
                    if (pst_chan_scan_report[uc_chan_idx].aus_num_networks[HMAC_NETWORK_SCB] < us_least_networks) {
                        us_least_networks = pst_chan_scan_report[uc_chan_idx].aus_num_networks[HMAC_NETWORK_SCB];
                        uc_least_busy_chan_idx = uc_chan_idx;
                        en_chan_offset = MAC_SCB;
                    }
                }
            } else { /* 不能够建立40MHz BSS */
                /* 获取当前信道邻近的BSS个数 */
                us_cumulative_networks = hmac_chan_get_cumulative_networks(pst_mac_device,
                                                                           pst_mac_vap->st_channel.en_band,
                                                                           uc_chan_idx);
                /* 选择邻近BSS最少的一条信道作为"当前最不繁忙信道" */
                if (us_cumulative_networks < us_least_networks) {
                    us_least_networks = us_cumulative_networks;
                    uc_least_busy_chan_idx = uc_chan_idx;
                }
            }
        }
    }

    mac_get_channel_num_from_idx(pst_mac_vap->st_channel.en_band, uc_least_busy_chan_idx, puc_new_channel);

    *pen_new_bandwidth = mac_get_bandwidth_from_sco(en_chan_offset);
    oal_mem_free_m((oal_void *)pst_chan_scan_report, OAL_TRUE);

    return OAL_SUCC;
}


oal_void hmac_chan_reval_status(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap)
{
    oal_uint8                            uc_new_channel   = 0;
    wlan_channel_bandwidth_enum_uint8    en_new_bandwidth = WLAN_BAND_WIDTH_BUTT;
    oal_uint32                           ul_ret;

    oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{hmac_chan_reval_status}");

    if (oal_unlikely((pst_mac_device == OAL_PTR_NULL) || (pst_mac_vap == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_SCAN, "{hmac_chan_reval_status::pst_mac_device[%x] or pst_mac_vap[%x] null.}",
                       (uintptr_t)pst_mac_device, (uintptr_t)pst_mac_vap);
        return;
    }

    /* 如果AP已经准备进行信道切换，则直接返回，不做任何处理 */
    if (pst_mac_vap->st_ch_switch_info.en_ch_switch_status == WLAN_CH_SWITCH_STATUS_1) {
        return;
    }

    ul_ret = hmac_chan_select_channel_for_operation(pst_mac_vap, &uc_new_channel, &en_new_bandwidth);
    if (ul_ret != OAL_SUCC) {
        return;
    }

    ul_ret = mac_is_channel_num_valid(pst_mac_vap->st_channel.en_band, uc_new_channel);
    if ((ul_ret != OAL_SUCC) || (en_new_bandwidth >= WLAN_BAND_WIDTH_BUTT)) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{Could not start network using the selected channel[%d] or bandwidth[%d].}",
                         uc_new_channel, en_new_bandwidth);
        return;
    }

    /* 主20MHz信道改变(按照目前的算法，主信道号应该不会改变，可能会改变的只是带宽模式) */
    if (uc_new_channel != pst_mac_vap->st_channel.uc_chan_number) {
        pst_mac_vap->st_ch_switch_info.uc_ch_switch_cnt = WLAN_CHAN_SWITCH_DEFAULT_CNT;
        hmac_chan_multi_switch_to_new_channel(pst_mac_vap, uc_new_channel, en_new_bandwidth);
    } else if (en_new_bandwidth != pst_mac_vap->st_channel.en_bandwidth) {
        hmac_chan_multi_select_channel_mac(pst_mac_vap, pst_mac_vap->st_channel.uc_chan_number, en_new_bandwidth);
        hmac_send_ht_notify_chan_width(pst_mac_vap, BROADCAST_MACADDR);
    } else {
        oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{No Channel change after re-evaluation.}");
    }
}


oal_void hmac_chan_reval_bandwidth_sta(mac_vap_stru *pst_mac_vap, oal_uint32 ul_change)
{
    if (oal_unlikely(pst_mac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{hmac_chan_reval_bandwidth_sta::pst_mac_vap null.}");
        return;
    }

    /* 需要进行带宽切换 */
    if (MAC_BW_CHANGE & ul_change) {
        hmac_chan_multi_select_channel_mac(pst_mac_vap, pst_mac_vap->st_channel.uc_chan_number,
                                           pst_mac_vap->st_channel.en_bandwidth);
    }
}


OAL_STATIC oal_void hmac_chan_ctrl_machw_tx(mac_vap_stru *pst_mac_vap, oal_uint8 uc_sub_type)
{
    frw_event_mem_stru       *pst_event_mem;
    frw_event_stru           *pst_event = OAL_PTR_NULL;
    oal_uint32                ul_ret;

    /* 申请事件内存 */
    pst_event_mem = frw_event_alloc_m(0);
    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_chan_ctrl_machw_tx::pst_event_mem null.}");
        return;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       uc_sub_type,
                       0,
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    /* 分发事件 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{ctrl_machw_tx::dispatch_event failed[%d].}", ul_ret);
        frw_event_free_m(pst_event_mem);
        return;
    }

    /* 释放事件 */
    frw_event_free_m(pst_event_mem);
}


oal_void hmac_chan_disable_machw_tx(mac_vap_stru *pst_mac_vap)
{
    hmac_chan_ctrl_machw_tx(pst_mac_vap, DMAC_WALN_CTX_EVENT_SUB_TYPR_DISABLE_TX);
}


oal_void hmac_chan_enable_machw_tx(mac_vap_stru *pst_mac_vap)
{
    hmac_chan_ctrl_machw_tx(pst_mac_vap, DMAC_WALN_CTX_EVENT_SUB_TYPR_ENABLE_TX);
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_chan_check_channnel_avaible(wlan_channel_band_enum_uint8 en_band,
                                                                           oal_uint8 *puc_start_ch_idx,
                                                                           oal_uint8 *puc_end_ch_idx)
{
    oal_int32    l_ch_idx;
    oal_uint8    uc_num_supp_chan = mac_get_num_supp_channel(en_band);
    oal_uint32   ul_ret;

    /* 取低有效信道 */
    for (l_ch_idx = *puc_start_ch_idx; l_ch_idx < uc_num_supp_chan; l_ch_idx++) {
        ul_ret = mac_is_channel_idx_valid(en_band, (oal_uint8)l_ch_idx);
        if (ul_ret == OAL_SUCC) {
            *puc_start_ch_idx = (oal_uint8)l_ch_idx;
            break;
        }
    }

    if (l_ch_idx == uc_num_supp_chan) {
        return OAL_FALSE;
    }

    /* 取高有效信道 */
    for (l_ch_idx = *puc_end_ch_idx; l_ch_idx >= 0; l_ch_idx--) {
        ul_ret = mac_is_channel_idx_valid(en_band, (oal_uint8)l_ch_idx);
        if (ul_ret == OAL_SUCC) {
            *puc_end_ch_idx = (oal_uint8)l_ch_idx;
            break;
        }
    }

    if (l_ch_idx < 0) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}


OAL_STATIC OAL_INLINE oal_uint32 hmac_chan_get_affected_channels(
    wlan_channel_band_enum_uint8 en_band,
    oal_uint8 uc_user_pri_ch_idx,
    wlan_channel_bandwidth_enum_uint8 en_user_pref_bandwidth,
    oal_uint8 *puc_start_ch_idx,
    oal_uint8 *puc_end_ch_idx)
{
    oal_uint8 uc_num_supp_chan = mac_get_num_supp_channel(en_band);
    oal_uint8 uc_pll_div_idx = 0; /* 中心频点 */

    /* 计算中心频点 */
    if (en_user_pref_bandwidth == WLAN_BAND_WIDTH_40PLUS) {
        uc_pll_div_idx = uc_user_pri_ch_idx + HMAC_CENTER_FREQ_2G_40M_OFFSET;
    } else if (en_user_pref_bandwidth == WLAN_BAND_WIDTH_40MINUS) {
        uc_pll_div_idx = uc_user_pri_ch_idx - HMAC_CENTER_FREQ_2G_40M_OFFSET;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{get_affected_channels::invalid bandwidth[%d].}", en_user_pref_bandwidth);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 计算需要扫描的信道索引下限 */
    *puc_start_ch_idx =
    (uc_pll_div_idx >= HMAC_AFFECTED_CH_IDX_OFFSET) ? (uc_pll_div_idx - HMAC_AFFECTED_CH_IDX_OFFSET) : 0;

    /* 计算需要扫描的信道索引上限 */
    *puc_end_ch_idx =
    (uc_num_supp_chan > uc_pll_div_idx + HMAC_AFFECTED_CH_IDX_OFFSET) ?
    (uc_pll_div_idx + HMAC_AFFECTED_CH_IDX_OFFSET) : (uc_num_supp_chan - 1);

    return OAL_SUCC;
}


OAL_STATIC oal_bool_enum_uint8 hmac_chan_is_scan_needed_for_multi_vap(mac_device_stru *pst_mac_device,
                                                                      mac_vap_stru *pst_mac_vap)
{
    mac_vap_stru   *pst_vap = OAL_PTR_NULL;
    oal_uint8       uc_vap_idx;

    /* 第一个VAP(AP)启动前，可能需要进行信道扫描，直接返回 */
    if (pst_mac_device->uc_vap_num == 1) {
        return OAL_TRUE;
    }

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(0, OAM_SF_SCAN,
                           "{hmac_chan_is_scan_needed_for_multi_vap::pst_mac_vap null,vap_idx=%d.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        /* 跳过本VAP */
        if (pst_vap->uc_vap_id == pst_mac_vap->uc_vap_id) {
            continue;
        }

        if (pst_vap->en_vap_state != MAC_VAP_STATE_INIT) {
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}


OAL_STATIC oal_bool_enum_uint8 hmac_chan_is_chan_scan_needed(mac_device_stru *pst_mac_device,
                                                             mac_vap_stru *pst_mac_vap,
                                                             oal_uint8 *puc_start_ch_idx,
                                                             oal_uint8 *puc_end_ch_idx)
{
    wlan_channel_band_enum_uint8    en_band = pst_mac_vap->st_channel.en_band;
    oal_uint8                       uc_num_supp_chan = mac_get_num_supp_channel(en_band);
    oal_bool_enum_uint8             en_rslt;
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    oal_uint8                       uc_user_ch_idx;
    oal_uint32                      ul_ret;
#endif

    /* 5g频段没有20/40共存,如果开启，BSS启动时直接配置信道，不需要进行扫描，避免出错 */
    if ((mac_mib_get_2040BSSCoexistenceManagementSupport(pst_mac_vap) == OAL_TRUE) &&
        (en_band == WLAN_BAND_5G)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{20/40 MHz BSS only in 2.4G.}");
        return OAL_FALSE;
    }

    /* 如果开启自动信道选择，则BSS启动时需要进行扫描 */
    if (mac_device_is_auto_chan_sel_enabled(pst_mac_device) == OAL_TRUE) {
        oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{Auto Channel Selection in progress.}");
        *puc_start_ch_idx = 0;
        *puc_end_ch_idx = uc_num_supp_chan - 1;

        if (hmac_chan_check_channnel_avaible(en_band, puc_start_ch_idx, puc_end_ch_idx) == OAL_FALSE) {
            return OAL_FALSE;
        }

        return OAL_TRUE;
    }

    en_rslt = hmac_chan_is_scan_needed_for_multi_vap(pst_mac_device, pst_mac_vap);
    if (en_rslt != OAL_TRUE) {
#ifdef _PRE_WLAN_FEATURE_DBAC
        if (mac_is_dbac_enabled(pst_mac_device) == OAL_FALSE) {
            pst_mac_vap->st_channel.uc_chan_number = pst_mac_device->uc_max_channel;
        }
#else
        pst_mac_vap->st_channel.uc_chan_number = pst_mac_device->uc_max_channel;
#endif

        return OAL_FALSE;
    }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    /* 进行信道扫描，当且仅当: */
    /* (1) 20/40MHz共存使能 */
    /* (2) 准备建立40MHz BSS */
    /* (3) 2.4GHz */
    if ((mac_mib_get_2040BSSCoexistenceManagementSupport(pst_mac_vap) == OAL_TRUE) &&
        (mac_mib_get_FortyMHzOperationImplemented(pst_mac_vap) == OAL_TRUE) &&
        (en_band == WLAN_BAND_2G)) {
        ul_ret = mac_get_channel_idx_from_num(pst_mac_vap->st_channel.en_band,
                                              hmac_chan_get_user_pref_primary_ch(pst_mac_device), &uc_user_ch_idx);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{mac_get_channel_idx_from_num failed[%d].}", ul_ret);
            return OAL_FALSE;
        }

        /* 获取需要扫描的信道范围 */
        ul_ret = hmac_chan_get_affected_channels(en_band, uc_user_ch_idx,
                                                 hmac_chan_get_user_pref_bandwidth(pst_mac_vap),
                                                 puc_start_ch_idx, puc_end_ch_idx);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{chan_get_affected_channels failed[%d].}", ul_ret);
            return OAL_FALSE;
        }

        if (hmac_chan_check_channnel_avaible(en_band, puc_start_ch_idx, puc_end_ch_idx) == OAL_FALSE) {
            return OAL_FALSE;
        }

        return OAL_TRUE;
    }
#endif

    return OAL_FALSE;
}


oal_uint32 hmac_start_bss_in_available_channel(hmac_vap_stru *pst_hmac_vap)
{
    hmac_ap_start_rsp_stru          st_ap_start_rsp;
    oal_uint32                      ul_ret;

    /* 调用hmac_config_start_vap_event，启动BSS */
    ul_ret = hmac_config_start_vap_event(&(pst_hmac_vap->st_vap_base_info), OAL_TRUE);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_INIT);
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_start_bss_in_available_channel::hmac_config_send_event failed[%d].}", ul_ret);
        return ul_ret;
    }

    /* 设置bssid */
    mac_vap_set_bssid(&pst_hmac_vap->st_vap_base_info,
                      pst_hmac_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);

    /* 入网优化，不同频段下的能力不一样 */
    if (pst_hmac_vap->st_vap_base_info.st_channel.en_band == WLAN_BAND_2G) {
        mac_mib_set_ShortPreambleOptionImplemented(&(pst_hmac_vap->st_vap_base_info),
                                                   WLAN_LEGACY_11B_MIB_SHORT_PREAMBLE);
        mac_mib_set_SpectrumManagementRequired(&(pst_hmac_vap->st_vap_base_info), OAL_FALSE);
    } else {
        mac_mib_set_ShortPreambleOptionImplemented(&(pst_hmac_vap->st_vap_base_info),
                                                   WLAN_LEGACY_11B_MIB_LONG_PREAMBLE);
        mac_mib_set_SpectrumManagementRequired(&(pst_hmac_vap->st_vap_base_info), OAL_TRUE);
    }

    /* 设置AP侧状态机为 UP */
    hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_UP);

    /* 将结果上报至sme */
    st_ap_start_rsp.en_result_code = HMAC_MGMT_SUCCESS;
    hmac_send_rsp_to_sme_ap(pst_hmac_vap, HMAC_AP_SME_START_RSP, (oal_uint8 *)&st_ap_start_rsp);

    return OAL_SUCC;
}


oal_uint32 hmac_chan_start_bss(hmac_vap_stru *pst_hmac_vap, mac_channel_stru *pst_channel,
                               wlan_protocol_enum_uint8 en_protocol)
{
    mac_device_stru                     *pst_mac_device = OAL_PTR_NULL;
    mac_vap_stru                        *pst_mac_vap = OAL_PTR_NULL;
    oal_uint32                           ul_ret;
    mac_cfg_mode_param_stru              st_prot_param;
    mac_channel_stru                    *pst_channel_dst = OAL_PTR_NULL;

    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{hmac_chan_start_bss::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);

    /* 获取mac device指针 */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_INIT);
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_chan_start_bss::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_channel != OAL_PTR_NULL) {
        pst_channel_dst = pst_channel;
    }
    if (pst_channel_dst == OAL_PTR_NULL) {
        pst_channel_dst = &pst_mac_vap->st_channel;
    }

    st_prot_param.en_channel_idx = pst_channel_dst->uc_chan_number;
    st_prot_param.en_bandwidth = pst_channel_dst->en_bandwidth;
    st_prot_param.en_band = pst_channel_dst->en_band;

    // use input protocol if exists
    st_prot_param.en_protocol = (en_protocol >= WLAN_PROTOCOL_BUTT) ? pst_mac_vap->en_protocol : en_protocol;

    ul_ret = mac_is_channel_num_valid(st_prot_param.en_band, st_prot_param.en_channel_idx);
    if (ul_ret != OAL_SUCC) {
        hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_INIT);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_chan_start_bss::mac_is_channel_num_valid failed[%d].}", ul_ret);
        return ul_ret;
    }

#ifdef _PRE_WLAN_FEATURE_DBAC
    /* 同时更改多个VAP的信道，此时需要强制清除记录 */
    /* 若启动了DBAC，则按照原始流程进行 */
    if (!mac_is_dbac_enabled(pst_mac_device))
#endif
    {
        pst_mac_device->uc_max_channel = 0;
        pst_mac_device->en_max_bandwidth = WLAN_BAND_WIDTH_BUTT;
    }

    // force channel setting is required
    memcpy_s(&pst_mac_vap->st_channel, OAL_SIZEOF(mac_channel_stru), pst_channel_dst, OAL_SIZEOF(mac_channel_stru));

    ul_ret = hmac_config_set_freq(pst_mac_vap, OAL_SIZEOF(oal_uint8), &st_prot_param.en_channel_idx);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_chan_start_bss::hmac_config_set_freq failed[%d].}", ul_ret);
        return ul_ret;
    }

    ul_ret = hmac_config_set_mode(pst_mac_vap, (oal_uint16)OAL_SIZEOF(st_prot_param), (oal_uint8 *)&st_prot_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_chan_start_bss::set_mode failed[%d].}", ul_ret);
        return ul_ret;
    }

    return hmac_start_bss_in_available_channel(pst_hmac_vap);
}

oal_uint32 hmac_chan_ch_av_chk_timeout_fn(void *p_arg)
{
    mac_device_stru              *pst_mac_device;
    hmac_vap_stru                *pst_hmac_vap = OAL_PTR_NULL;
    mac_vap_stru                 *pst_mac_vap;
    mac_chnl_av_chk_enum_uint8    en_status;

    pst_mac_vap = (mac_vap_stru *)p_arg;
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_chan_ch_av_chk_timeout_fn::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (oal_unlikely(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_chan_ch_av_chk_timeout_fn::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_vap->st_ch_switch_info.en_ch_switch_status == WLAN_CH_SWITCH_STATUS_1) {
        return OAL_SUCC;
    }

    /* 继续进行信道扫描 */
    en_status = hmac_chan_do_channel_availability_check(pst_mac_device, pst_mac_vap, OAL_FALSE);
    /* 如果信道扫描已完成，则选择一条信道(对)并启动BSS */
    if (en_status == MAC_CHNL_AV_CHK_COMPLETE) {
        hmac_chan_start_bss(pst_hmac_vap, OAL_PTR_NULL, WLAN_PROTOCOL_BUTT);
    }

    return OAL_SUCC;
}


mac_chnl_av_chk_enum_uint8 hmac_chan_do_channel_availability_check(mac_device_stru *pst_mac_device,
                                                                   mac_vap_stru *pst_mac_vap,
                                                                   oal_bool_enum_uint8 en_first_time)
{
    wlan_channel_band_enum_uint8    en_band = pst_mac_vap->st_channel.en_band;
    oal_uint8                       uc_num_supp_chan = mac_get_num_supp_channel(en_band);
    mac_ch_switch_info_stru        *pst_csi = &(pst_mac_vap->st_ch_switch_info);
    oal_uint8                       uc_channel = 0;
    oal_bool_enum_uint8             en_need_for_scan = OAL_FALSE;
    oal_bool_enum_uint8             en_scan_itr_over = OAL_FALSE;

    for (;;) {
        if (en_first_time) {
            /* 判断是否需要进行信道扫描 */
            en_need_for_scan = hmac_chan_is_chan_scan_needed(pst_mac_device, pst_mac_vap, &(pst_csi->uc_start_chan_idx),
                                                             &(pst_csi->uc_end_chan_idx));
            if (en_need_for_scan == OAL_FALSE) {
                return MAC_CHNL_AV_CHK_NOT_REQ;
            } else {
                oam_info_log2(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                              "{hmac_chan_do_channel_availability_check::Channel scanning from %d to %d.}",
                              pst_csi->uc_start_chan_idx, pst_csi->uc_end_chan_idx);
            }

            /* 初始化信道扫描索引，从第一条需要扫描的信道开始 */
            pst_mac_device->uc_ap_chan_idx = pst_csi->uc_start_chan_idx;

            /* 清信道扫描结果列表 */
            memset_s(pst_mac_device->st_ap_channel_list, OAL_SIZEOF(pst_mac_device->st_ap_channel_list),
                0, OAL_SIZEOF(pst_mac_device->st_ap_channel_list));

            /* 清'40MHz不允许位' */
            pst_mac_device->en_40MHz_intol_bit_recd = OAL_FALSE;

            /* 当前信道下扫描到的BSS个数清零 */
            pst_mac_device->st_bss_id_list.us_num_networks = 0;
        } else {
            /* 保存当前信道扫描到的BSS个数 */
            pst_mac_device->st_ap_channel_list[pst_mac_device->uc_ap_chan_idx].us_num_networks =
                pst_mac_device->st_bss_id_list.us_num_networks;

            /* 清零，为扫描下一条信道做准备 */
            pst_mac_device->st_bss_id_list.us_num_networks = 0;

            /* 扫到最后一条信道，扫描结束 */
            if (pst_mac_device->uc_ap_chan_idx == pst_csi->uc_end_chan_idx) {
                en_scan_itr_over = OAL_TRUE;
            }

            /* 本次扫描完成，要扫描的信道索引+1 */
            pst_mac_device->uc_ap_chan_idx++;

            if (pst_mac_device->uc_ap_chan_idx >= uc_num_supp_chan) {
                pst_mac_device->uc_ap_chan_idx = 0;
            }

            if (pst_mac_device->uc_ap_chan_idx == pst_csi->uc_start_chan_idx) {
                en_scan_itr_over = OAL_TRUE;
            }

            if (en_scan_itr_over == OAL_TRUE) {
                break;
            }
        }

        mac_get_channel_num_from_idx(en_band, pst_mac_device->uc_ap_chan_idx, &uc_channel);

        if (OAL_TRUE == hmac_chan_scan_availability(pst_mac_device,
                                                    &pst_mac_device->
                                                    st_ap_channel_list[pst_mac_device->uc_ap_chan_idx])) {
            /* 切换至当前信道 */
            hmac_chan_sync(pst_mac_vap, uc_channel, WLAN_BAND_WIDTH_20M, OAL_TRUE);

            /* 设置信道扫描超时时间 */
            pst_mac_device->st_scan_params.us_scan_time = WLAN_DEFAULT_PASSIVE_SCAN_TIME;

            /* 启动定时器 */
            frw_create_timer(&pst_mac_device->st_scan_timer,
                             hmac_chan_ch_av_chk_timeout_fn,
                             pst_mac_device->st_scan_params.us_scan_time,
                             pst_mac_vap,
                             OAL_FALSE,
                             OAM_MODULE_ID_HMAC,
                             pst_mac_device->ul_core_id);
            return MAC_CHNL_AV_CHK_IN_PROG;
        }

        en_first_time = OAL_FALSE;
    }

    return MAC_CHNL_AV_CHK_COMPLETE;
}

oal_void hmac_chan_get_ext_chan_info(oal_uint8 uc_pri20_channel_idx,
                                     wlan_channel_bandwidth_enum_uint8 en_bandwidth,
                                     hmac_channel_list_stru *pst_chan_info)
{
    switch (en_bandwidth) {
        case WLAN_BAND_WIDTH_20M:
            pst_chan_info->ul_channels = 1;
            pst_chan_info->ast_channels[0] = g_ast_freq_map_5g[uc_pri20_channel_idx];
            break;

        case WLAN_BAND_WIDTH_40PLUS:
            pst_chan_info->ul_channels = 2; /* 2信道 */
            pst_chan_info->ast_channels[0] = g_ast_freq_map_5g[uc_pri20_channel_idx];
            pst_chan_info->ast_channels[1] = g_ast_freq_map_5g[uc_pri20_channel_idx + 1];
            break;

        case WLAN_BAND_WIDTH_40MINUS:
            pst_chan_info->ul_channels = 2; /* 2信道 */
            pst_chan_info->ast_channels[0] = g_ast_freq_map_5g[uc_pri20_channel_idx - 1];
            pst_chan_info->ast_channels[1] = g_ast_freq_map_5g[uc_pri20_channel_idx];
            break;

        case WLAN_BAND_WIDTH_80PLUSPLUS:
            pst_chan_info->ul_channels = 4; /* 4信道 */
            pst_chan_info->ast_channels[0] = g_ast_freq_map_5g[uc_pri20_channel_idx]; /* 主20中心频点 */
            pst_chan_info->ast_channels[1] = g_ast_freq_map_5g[uc_pri20_channel_idx + 1]; /* 从20信道+1 */
            /* ast_channels 2、3byte表示从40信道+1 */
            pst_chan_info->ast_channels[2] = g_ast_freq_map_5g[uc_pri20_channel_idx + 2];
            /* ast_channels 2、3byte表示从40信道+1 */
            pst_chan_info->ast_channels[3] = g_ast_freq_map_5g[uc_pri20_channel_idx + 3];
            break;

        case WLAN_BAND_WIDTH_80PLUSMINUS:
            pst_chan_info->ul_channels = 4; /* 4信道 */
            /* ast_channels 0、1byte表示从40信道-1（g_ast_freq_map_5g[uc_pri20_channel_idx]偏移2字节） */
            pst_chan_info->ast_channels[0] = g_ast_freq_map_5g[uc_pri20_channel_idx - 2];
            pst_chan_info->ast_channels[1] = g_ast_freq_map_5g[uc_pri20_channel_idx - 1];
            pst_chan_info->ast_channels[2] = g_ast_freq_map_5g[uc_pri20_channel_idx]; /* ast_channels[2]是主20中心频点 */
            pst_chan_info->ast_channels[3] = g_ast_freq_map_5g[uc_pri20_channel_idx + 1]; /* ast_channels[3]从20信道+1 */
            break;

        case WLAN_BAND_WIDTH_80MINUSPLUS:
            pst_chan_info->ul_channels = 4; /* 4信道 */
            pst_chan_info->ast_channels[0] = g_ast_freq_map_5g[uc_pri20_channel_idx - 1]; /* 从20信道-1 */
            pst_chan_info->ast_channels[1] = g_ast_freq_map_5g[uc_pri20_channel_idx]; /* 主20中心频点 */
            /* ast_channels 2、3byte表示从40信道+1 */
            pst_chan_info->ast_channels[2] = g_ast_freq_map_5g[uc_pri20_channel_idx + 1];
            /* ast_channels 2、3byte表示从40信道+1 */
            pst_chan_info->ast_channels[3] = g_ast_freq_map_5g[uc_pri20_channel_idx + 2];
            break;

        case WLAN_BAND_WIDTH_80MINUSMINUS:
            pst_chan_info->ul_channels = 4; /* 4信道 */
            /* ast_channels 0、1byte表示从40信道-1（g_ast_freq_map_5g[uc_pri20_channel_idx]偏移3字节） */
            pst_chan_info->ast_channels[0] = g_ast_freq_map_5g[uc_pri20_channel_idx - 3];
            /* ast_channels 0、1byte表示从40信道-1（g_ast_freq_map_5g[uc_pri20_channel_idx]偏移2字节） */
            pst_chan_info->ast_channels[1] = g_ast_freq_map_5g[uc_pri20_channel_idx - 2];
            pst_chan_info->ast_channels[2] = g_ast_freq_map_5g[uc_pri20_channel_idx - 1]; /* ast_channels[2]从20信道-1 */
            pst_chan_info->ast_channels[3] = g_ast_freq_map_5g[uc_pri20_channel_idx]; /* ast_channels[3]是主20中心频点 */
            break;

        default:
            pst_chan_info->ul_channels = 0;
            OAM_ERROR_LOG1(0, OAM_SF_DFS, "{hmac_chan_get_ext_chan_info::Invalid bandwidth %d.}", en_bandwidth);
            break;
    }
}


oal_void hmac_chan_restart_network_after_switch(mac_vap_stru *pst_mac_vap)
{
    frw_event_mem_stru   *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru       *pst_event = OAL_PTR_NULL;
    oal_uint32            ul_ret;

    oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{hmac_chan_restart_network_after_switch}");

    /* 申请事件内存 */
    pst_event_mem = frw_event_alloc_m(0);
    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_chan_restart_network_after_switch::pst_event_mem null.}");

        return;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

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
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_chan_restart_network_after_switch::frw_event_dispatch_event failed[%d].}", ul_ret);
        frw_event_free_m(pst_event_mem);

        return;
    }
    frw_event_free_m(pst_event_mem);

    return;
}
/*lint -e578*/ /*lint -e19*/
oal_module_symbol(g_ast_freq_map_5g);
oal_module_symbol(g_ast_freq_map_2g);
/*lint +e578*/ /*lint +e19*/


oal_uint32 hmac_chan_switch_to_new_chan_complete(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru     *pst_event = OAL_PTR_NULL;
    mac_device_stru    *pst_mac_device = OAL_PTR_NULL;
    hmac_vap_stru      *pst_hmac_vap = OAL_PTR_NULL;
    mac_vap_stru       *pst_mac_vap = OAL_PTR_NULL;
    dmac_set_chan_stru *pst_set_chan = OAL_PTR_NULL;
    oal_uint32          ul_ret;
    oal_uint8           uc_idx;

    oam_info_log0(0, OAM_SF_2040, "{hmac_chan_restart_network_after_switch}");

    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_2040, "{hmac_switch_to_new_chan_complete::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;
    pst_set_chan = (dmac_set_chan_stru *)pst_event->auc_event_data;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (oal_unlikely(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_2040,
                       "{hmac_switch_to_new_chan_complete::pst_hmac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap = &pst_hmac_vap->st_vap_base_info;

    oam_info_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_2040, "hmac_chan_switch_to_new_chan_complete");
    hmac_dump_chan(pst_mac_vap, (oal_uint8 *)pst_set_chan);

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_2040,
                       "{hmac_switch_to_new_chan_complete::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = mac_get_channel_idx_from_num(pst_mac_vap->st_channel.en_band,
                                          pst_set_chan->st_channel.uc_chan_number, &uc_idx);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                         "{hmac_switch_to_new_chan_complete::mac_get_channel_idx_from_num failed[%d].}", ul_ret);

        return OAL_FAIL;
    }

    pst_mac_vap->st_channel.uc_chan_number = pst_set_chan->st_channel.uc_chan_number;
    pst_mac_vap->st_channel.en_bandwidth = pst_set_chan->st_channel.en_bandwidth;
    pst_mac_vap->st_channel.uc_idx = uc_idx;

    pst_mac_vap->st_ch_switch_info.en_waiting_to_shift_channel =
    pst_set_chan->st_ch_switch_info.en_waiting_to_shift_channel;

    pst_mac_vap->st_ch_switch_info.en_ch_switch_status = pst_set_chan->st_ch_switch_info.en_ch_switch_status;
    pst_mac_vap->st_ch_switch_info.en_bw_switch_status = pst_set_chan->st_ch_switch_info.en_bw_switch_status;

    /* aput切完信道同步切信道的标志位,防止再有用户关联,把此变量又同步下去 */
    pst_mac_vap->st_ch_switch_info.uc_ch_switch_cnt = pst_set_chan->st_ch_switch_info.uc_ch_switch_cnt;
    pst_mac_vap->st_ch_switch_info.en_csa_present_in_bcn = pst_set_chan->st_ch_switch_info.en_csa_present_in_bcn;

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    /* 带宽已成功切换到40M, 停止40M恢复定时器 */
    if ((pst_mac_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_20M) &&
        (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP)) {
        hmac_chan_stop_40m_recovery_timer(&pst_hmac_vap->st_vap_base_info);
    }
#endif

    if (pst_set_chan->en_check_cac == OAL_FALSE) {
        return OAL_SUCC;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_dbac_status_notify(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru     *pst_event = OAL_PTR_NULL;
    mac_device_stru    *pst_mac_device = OAL_PTR_NULL;
    oal_bool_enum_uint8 *pen_dbac_enable = OAL_PTR_NULL;

    if (oal_unlikely(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_2040, "{hmac_dbac_status_notify::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;
    pen_dbac_enable = (oal_bool_enum_uint8 *)pst_event->auc_event_data;
    oam_info_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_2040,
                  "hmac_dbac_status_notify::dbac switch to enable=%d", *pen_dbac_enable);

    pst_mac_device = mac_res_get_dev(pst_event->st_event_hdr.uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_2040,
                       "{hmac_dbac_status_notify::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device->en_dbac_running = *pen_dbac_enable;

    return OAL_SUCC;
}


