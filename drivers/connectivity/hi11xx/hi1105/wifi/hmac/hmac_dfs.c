
#ifdef _PRE_WLAN_FEATURE_DFS

/* 1 头文件包含 */
#include "hmac_fsm.h"
#include "hmac_chan_mgmt.h"
#include "hmac_dfs.h"
#include "hmac_resource.h"
#include "securec.h"
#include "wal_linux_ioctl.h"
#include "hmac_rx_filter.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DFS_C

hmac_dfs_radar_result_stru g_st_dfs_result = {
    .en_dfs_status = HMAC_DFS_STUS_BUTT,
};

#define HMAC_DFS_IS_CHAN_WEATHER_RADAR(_us_freq) \
    ((5600 <= (_us_freq)) && ((_us_freq) <= 5650))
#define channel_status_is_not_use(ch_status) \
    (((ch_status) != MAC_CHAN_NOT_SUPPORT) && ((ch_status) != MAC_CHAN_BLOCK_DUE_TO_RADAR))

OAL_STATIC uint32_t hmac_dfs_nol_addchan(mac_device_stru *mac_device, uint8_t chan_idx);
OAL_STATIC uint32_t hmac_dfs_nol_delchan(mac_device_stru *mac_device, mac_dfs_nol_node_stru *nol_node);
#ifdef _PRE_WLAN_FEATURE_OFFCHAN_CAC
OAL_STATIC void hmac_scan_switch_channel_back(mac_vap_stru *mac_vap);
OAL_STATIC void hmac_scan_switch_channel_off(mac_vap_stru *mac_vap);
#endif
OAL_STATIC uint32_t hmac_dfs_off_chan_cac_off_ch_dwell_timeout(void *p_arg);
OAL_STATIC uint32_t hmac_dfs_off_chan_cac_opern_ch_dwell_timeout(void *p_arg);
void hmac_dfs_status_set(uint32_t dfs_status);
OAL_STATIC void hmac_dfs_go_cac_forbit_scan_flag_set(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 flag);


void hmac_dfs_init(mac_device_stru *mac_device)
{
    mac_dfs_info_stru *dfs_info = NULL;

    /* 初始化Non-Occupancy List链表 */
    oal_dlist_init_head(&(mac_device->st_dfs.st_dfs_nol));

    /* CAC检测默认不使能 */
    mac_dfs_set_cac_enable(mac_device, OAL_TRUE);
    /* OFFCHAN-CAC检测默认不使能 */
    mac_dfs_set_offchan_cac_enable(mac_device, OAL_FALSE);

    /* 设置CAC, Off-Channel CAC, etc... 超时时间 */
    dfs_info = &(mac_device->st_dfs.st_dfs_info);

    dfs_info->dfs_cac_outof_5600_to_5650_time_ms = HMAC_DFS_CAC_OUTOF_5600_TO_5650_MHZ_TIME_MS;
    dfs_info->dfs_cac_in_5600_to_5650_time_ms = HMAC_DFS_CAC_IN_5600_TO_5650_MHZ_TIME_MS;
    dfs_info->off_chan_cac_outof_5600_to_5650_time_ms = HMAC_DFS_OFF_CH_CAC_OUTOF_5600_TO_5650_MHZ_TIME_MS;
    dfs_info->off_chan_cac_in_5600_to_5650_time_ms = HMAC_DFS_OFF_CH_CAC_IN_5600_TO_5650_MHZ_TIME_MS;
    dfs_info->us_dfs_off_chan_cac_opern_chan_dwell_time = HMAC_DFS_OFF_CHAN_CAC_PERIOD_TIME_MS;
    dfs_info->us_dfs_off_chan_cac_off_chan_dwell_time = HMAC_DFS_OFF_CHAN_CAC_DWELL_TIME_MS;
    dfs_info->dfs_non_occupancy_period_time_ms = HMAC_DFS_NON_OCCUPANCY_PERIOD_TIME_MS;
    dfs_info->en_dfs_init = OAL_FALSE;

    /* 默认当前信道为home channel */
    dfs_info->uc_offchan_flag = 0;
}


void hmac_dfs_channel_list_init(mac_device_stru *mac_device)
{
    mac_chan_status_enum_uint8 ch_status;
    uint8_t idx;
    uint32_t ret;

    for (idx = 0; idx < MAC_MAX_SUPP_CHANNEL; idx++) {
        ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, idx, OAL_FALSE);
        if (ret == OAL_SUCC) {
            if (OAL_TRUE == mac_is_ch_in_radar_band(MAC_RC_START_FREQ_5, idx)) {
                /* DFS信道 */
                ch_status = MAC_CHAN_DFS_REQUIRED;
            } else {
                ch_status = MAC_CHAN_AVAILABLE_ALWAYS;
            }
        } else {
            /* 管制域不支持 */
            ch_status = MAC_CHAN_NOT_SUPPORT;
        }

        mac_device->st_ap_channel_list[idx].en_ch_status = ch_status;
    }
}


OAL_STATIC uint8_t hmac_dfs_find_lowest_available_channel(mac_device_stru *mac_device)
{
    uint8_t channel_num = 36;
    uint8_t chan_idx;
    uint8_t num_supp_chan = mac_get_num_supp_channel(mac_device->en_max_band);
    uint32_t ret;

    for (chan_idx = 0; chan_idx < num_supp_chan; chan_idx++) {
        ret = mac_is_channel_idx_valid(mac_device->en_max_band, chan_idx, OAL_FALSE);
        if (ret != OAL_SUCC) {
            continue;
        }

        if ((mac_device->st_ap_channel_list[chan_idx].en_ch_status != MAC_CHAN_NOT_SUPPORT) &&
            (mac_device->st_ap_channel_list[chan_idx].en_ch_status != MAC_CHAN_BLOCK_DUE_TO_RADAR)) {
            mac_get_channel_num_from_idx(mac_device->en_max_band, chan_idx, OAL_FALSE, &channel_num);

            return channel_num;
        }
    }

    /* should not be here */
    return channel_num;
}


uint32_t hmac_dfs_recalculate_channel(mac_device_stru *mac_device,
    uint8_t *freq, wlan_channel_bandwidth_enum_uint8 *bandwidth)
{
    mac_channel_list_stru st_chan_info;
    uint8_t chan_idx;
    uint8_t loop;
    uint32_t ret;
    oal_bool_enum_uint8 recalc = OAL_FALSE;

    ret = mac_get_channel_idx_from_num(mac_device->en_max_band, *freq, OAL_FALSE, &chan_idx);
    if (ret != OAL_SUCC) {
        oam_error_log2(mac_device->uc_device_id, OAM_SF_DFS,
            "{hmac_dfs_recalculate_channel::mac_get_channel_idx_from_num fail.max_band:%d  freq:%d}",
            mac_device->en_max_band, *freq);
        return ret;
    }

    mac_get_ext_chan_info(chan_idx, *bandwidth, &st_chan_info);

    for (loop = 0; loop < st_chan_info.channels; loop++) {
        chan_idx = st_chan_info.ast_channels[loop].uc_idx;

        if ((mac_device->st_ap_channel_list[chan_idx].en_ch_status == MAC_CHAN_NOT_SUPPORT) ||
            (mac_device->st_ap_channel_list[chan_idx].en_ch_status == MAC_CHAN_BLOCK_DUE_TO_RADAR)) {
            recalc = OAL_TRUE;
            break;
        }
    }

    if (recalc == OAL_FALSE) {
        return OAL_FALSE;
    }

    /* 如果由于雷达干扰导致当前信道不可用，则需要从管制域支持的信道中，选择最低可用信道，带宽20MHz */
    *freq = hmac_dfs_find_lowest_available_channel(mac_device);

    *bandwidth = WLAN_BAND_WIDTH_20M;

    return OAL_TRUE;
}

OAL_STATIC OAL_INLINE uint32_t hmac_dfs_cac_event_report(mac_vap_stru *mac_vap,
    hmac_cac_event_stru *cac_event)
{
    frw_event_mem_stru *event_mem;
    frw_event_stru *event = NULL;

    event_mem = frw_event_alloc_m(sizeof(hmac_cac_event_stru));
    if (event_mem == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_DFS, "{[DFS]hmac_dfs_cac_event_report::pst_event_mem null.}");
        return OAL_FAIL;
    }

    /* 填写事件 */
    event = (frw_event_stru *)event_mem->puc_data;

    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_HOST_CTX, HMAC_HOST_CTX_EVENT_SUB_TYPE_CAC_REPORT,
        sizeof(hmac_cac_event_stru), FRW_EVENT_PIPELINE_STAGE_0, mac_vap->uc_chip_id,
        mac_vap->uc_device_id, mac_vap->uc_vap_id);

    /* cac事件 */
    if (EOK != memcpy_s(frw_get_event_payload(event_mem), sizeof(hmac_cac_event_stru),
        (const void *)cac_event, sizeof(hmac_cac_event_stru))) {
        oam_error_log0(0, OAM_SF_ANY, "hwifi_config_init_fcc_ce_txpwr_nvram::memcpy fail!");
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);
    return OAL_SUCC;
}


uint32_t hmac_dfs_radar_detect_event(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_misc_input_stru st_misc_input;
    uint8_t fem_enable = 0;

    if (oal_unlikely(event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_radar_detect_event::pst_event_mem is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    event = frw_get_event_stru(event_mem);

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(event->st_event_hdr.uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_radar_detect_event::pst_hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 检测到雷达，关闭fem低功耗 */
    hmac_config_fem_lp_flag(&hmac_vap->st_vap_base_info, sizeof(uint8_t), &fem_enable);

    st_misc_input.en_type = HMAC_MISC_RADAR;

    hmac_fsm_call_func_ap(hmac_vap, HMAC_FSM_INPUT_MISC, &st_misc_input);

    return OAL_SUCC;
}

uint32_t hmac_dfs_radar_detect_event_test(uint8_t vap_id)
{
    hmac_vap_stru *hmac_vap = NULL;
    hmac_misc_input_stru st_misc_input;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_radar_detect_event_test::pst_hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_misc_input.en_type = HMAC_MISC_RADAR;

    hmac_fsm_call_func_ap(hmac_vap, HMAC_FSM_INPUT_MISC, &st_misc_input);

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_dfs_update_available_channel_list(mac_device_stru *mac_device,
    uint8_t chan_idx, oal_bool_enum_uint8 radar_detected)
{
    uint32_t ret;

    ret = mac_is_channel_idx_valid(mac_device->en_max_band, chan_idx, OAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_DFS, "{hmac_dfs_update_available_channel_list::chan_idx(%d) invalid!}", chan_idx);
        oal_io_print("[DFS]hmac_dfs_update_available_channel_list::ch is not available.\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (mac_device->st_ap_channel_list[chan_idx].en_ch_status == MAC_CHAN_AVAILABLE_ALWAYS) {
        oam_warning_log1(0, OAM_SF_DFS,
            "{hmac_dfs_update_available_channel_list::Radar detected in Non-Radar Channel(%d)!}", chan_idx);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (radar_detected == OAL_TRUE) {
        mac_device->st_ap_channel_list[chan_idx].en_ch_status = MAC_CHAN_BLOCK_DUE_TO_RADAR;
        oal_io_print("[DFS]ch status is changed to due to radar.\n");
    } else {
        mac_device->st_ap_channel_list[chan_idx].en_ch_status = MAC_CHAN_AVAILABLE_TO_OPERATE;
    }

    hmac_config_ch_status_sync(mac_device);

    return OAL_SUCC;
}

OAL_STATIC void hmac_dfs_cac_timeout_fn_start_cac(mac_device_stru *mac_device, hmac_vap_stru *hmac_vap)
{
    uint8_t idx;
    uint32_t ret;
    hmac_cac_event_stru cac_event;

    /* off-channel cac start */
    if (mac_dfs_get_offchan_cac_enable(mac_device) == OAL_TRUE) {
        hmac_dfs_off_chan_cac_start(mac_device, hmac_vap);
    }

    // 调用内核接口通知应用层CAC启动
    idx = hmac_vap->st_vap_base_info.st_channel.uc_chan_idx;
    ret = mac_get_channel_idx_from_num(MAC_RC_START_FREQ_5, mac_device->uc_max_channel, OAL_FALSE, &idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFS, "{[DFS]hmac_dfs_cac_timeout_fn::mac_get_channel_idx_from_num failed=%d}", ret);
    }
    hmac_dfs_status_set(HMAC_INS_START);
    cac_event.en_type = HMAC_CAC_FINISHED;
    cac_event.freq = g_ast_freq_map_5g[idx].us_freq;
    cac_event.en_bw_mode = hmac_vap->st_vap_base_info.st_channel.en_bandwidth;
    ret = hmac_dfs_cac_event_report(&hmac_vap->st_vap_base_info, &cac_event);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFS, "{[DFS]hmac_dfs_cac_timeout_fn::hmac_dfs_cac_event_report failed=%d}", ret);
    }
}

OAL_STATIC void hmac_set_prot_param(mac_cfg_mode_param_stru *prot_param, mac_channel_stru *channel_dst)
{
    prot_param->en_channel_idx = channel_dst->uc_chan_number;
    prot_param->en_bandwidth = channel_dst->en_bandwidth;
    prot_param->en_band = channel_dst->en_band;
    prot_param->ext6g_band = channel_dst->ext6g_band;
}


uint32_t hmac_sta_sync_vap(hmac_vap_stru *pst_hmac_vap, mac_channel_stru *pst_channel,
    wlan_protocol_enum_uint8 en_protocol)
{
    mac_device_stru *pst_mac_device = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    uint32_t ret;
    mac_cfg_mode_param_stru st_prot_param;
    mac_channel_stru *pst_channel_dst = NULL;

    if (pst_hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_sta_sync_vap::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);

    /* 获取mac device指针 */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == NULL)) {
        hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_INIT);
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_sta_sync_vap::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_channel_dst = pst_channel ? pst_channel : &pst_mac_vap->st_channel;
    hmac_set_prot_param(&st_prot_param, pst_channel_dst);

    // use input protocol if exists
    st_prot_param.en_protocol = (en_protocol >= WLAN_PROTOCOL_BUTT) ? pst_mac_vap->en_protocol : en_protocol;

    ret = mac_is_channel_num_valid(st_prot_param.en_band, st_prot_param.en_channel_idx, st_prot_param.ext6g_band);
    if (ret != OAL_SUCC) {
        hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_INIT);
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_sta_sync_vap::mac_is_channel_num_valid failed[%d].}", ret);
        return ret;
    }

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                     "{hmac_sta_sync_vap::AP: Starting network in Channel: %d, bandwidth: %d protocol: %d.}",
                     st_prot_param.en_channel_idx, st_prot_param.en_bandwidth, st_prot_param.en_protocol);

    /* 同时更改多个VAP的信道，此时需要强制清除记录 */
    /* 若启动了DBAC，则按照原始流程进行 */
    if (!mac_is_dbac_enabled(pst_mac_device)) {
        pst_mac_device->uc_max_channel = 0;
        pst_mac_device->en_max_bandwidth = WLAN_BAND_WIDTH_BUTT;
    }

    // force channel setting is required
    pst_mac_vap->st_channel = *pst_channel_dst;

    ret = hmac_config_set_freq(pst_mac_vap, sizeof(uint8_t), &st_prot_param.en_channel_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_sta_sync_vap::hmac_config_set_freq failed[%d].}", ret);
        return ret;
    }

    ret = hmac_config_set_mode(pst_mac_vap, (uint16_t)sizeof(st_prot_param), (uint8_t *)&st_prot_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_sta_sync_vap::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t hmac_chan_start_bss(hmac_vap_stru *pst_hmac_vap, mac_channel_stru *pst_channel,
                             wlan_protocol_enum_uint8 en_protocol)
{
    uint32_t ret;

    // 同步信道和模式
    ret = hmac_sta_sync_vap(pst_hmac_vap, pst_channel, en_protocol);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_chan_start_bss::hmac_sta_sync_vap failed[%d].}", ret);
        return ret;
    }

    // 启动vap
    return hmac_start_bss_in_available_channel(pst_hmac_vap);
}


static uint32_t hmac_dfs_start_all_bss_of_device(hmac_device_stru *hmac_dev)
{
    uint8_t uc_idx;
    hmac_vap_stru *hmac_vap = NULL;
    mac_device_stru *dev = hmac_dev->pst_device_base_info;

    oam_warning_log1(0, OAM_SF_ACS, "{hmac_dfs_start_all_bss_of_device:device id=%d}",
                     hmac_dev->pst_device_base_info->uc_device_id);
    if (oal_unlikely(dev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_dfs_start_all_bss_of_device::device_base_info null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (uc_idx = 0; uc_idx < dev->uc_vap_num; uc_idx++) {
        hmac_vap = mac_res_get_hmac_vap(dev->auc_vap_id[uc_idx]);
        if (hmac_vap == NULL) {
            oam_warning_log2(0, OAM_SF_ACS, "hmac_dfs_start_all_bss_of_device:null ap, idx=%d id=%d",
                             uc_idx, dev->auc_vap_id[uc_idx]);
            continue;
        }

        if ((hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
            (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_AP_WAIT_START ||
            (mac_is_dbac_enabled(dev) && hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_PAUSE) ||
            hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_UP)) {
            hmac_chan_start_bss(hmac_vap, NULL, WLAN_PROTOCOL_BUTT);
        } else {
            oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ACS, "vap %d not started\n",
                             dev->auc_vap_id[uc_idx]);
            continue;
        }
    }

    if (mac_is_dbac_enabled(dev)) {
        return hmac_scan_start_dbac(dev);
    }

    return OAL_SUCC;
}


uint32_t hmac_dfs_cac_timeout_fn(void *p_arg)
{
    mac_device_stru *mac_device = NULL;
    hmac_device_stru *hmac_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_channel_list_stru st_chan_info;
    uint8_t idx;
    mac_dfs_info_stru *dfs_info = NULL;
    uint32_t ret;

    if (oal_unlikely(p_arg == NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_cac_timeout_fn::p_arg is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)p_arg;
    hmac_dfs_go_cac_forbit_scan_flag_set(&hmac_vap->st_vap_base_info, OAL_FALSE);

    oal_io_print("[DFS]hmac_dfs_cac_timeout_fn, CAC expired, channel number:%d.\n",
        hmac_vap->st_vap_base_info.st_channel.uc_chan_number);

    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_any_null_ptr2(hmac_device, hmac_device->pst_device_base_info)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_cac_timeout_fn::pst_mac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_device = hmac_device->pst_device_base_info;

    dfs_info = &mac_device->st_dfs.st_dfs_info;

    dfs_info->uc_timer_cnt++;

    if (dfs_info->uc_timer_cnt != dfs_info->uc_timer_end_cnt) {
        /* 启动 CAC 定时器 */
        frw_timer_create_timer_m(&mac_device->st_dfs.st_dfs_cac_timer, hmac_dfs_cac_timeout_fn,
            HMAC_DFS_SIXTY_TWO_SEC_IN_MS, hmac_vap, OAL_FALSE, OAM_MODULE_ID_HMAC,
            hmac_vap->st_vap_base_info.core_id);
        return OAL_SUCC;
    }

    mac_get_ext_chan_info(hmac_vap->st_vap_base_info.st_channel.uc_chan_idx,
        hmac_vap->st_vap_base_info.st_channel.en_bandwidth, &st_chan_info);

    /* 将当前信道设置为工作信道 */
    for (idx = 0; idx < st_chan_info.channels; idx++) {
        ret = hmac_dfs_update_available_channel_list(mac_device, st_chan_info.ast_channels[idx].uc_idx, OAL_FALSE);
        if (ret != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_cac_timeout_fn::hmac_dfs_update_available_channel_list fail.}");
        }
    }

    if (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_AP_WAIT_START) {
        hmac_device->en_init_scan = OAL_FALSE;
        hmac_device->en_in_init_scan = OAL_FALSE;
        /* 在该信道启动 BSS */
        hmac_dfs_start_all_bss_of_device(hmac_device);
    } else if ((hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_PAUSE) ||
        (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_UP)) {
        /* cac超时后把vap状态置为up,恢复发送队列 */
        hmac_cac_chan_ctrl_machw_tx(&(hmac_vap->st_vap_base_info), OAL_TRUE);
    }

    hmac_dfs_cac_timeout_fn_start_cac(mac_device, hmac_vap);

    return OAL_SUCC;
}


uint32_t hmac_dfs_start_bss(hmac_vap_stru *hmac_vap)
{
    mac_device_stru *mac_device = NULL;

    if (oal_unlikely(hmac_vap == NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_start_bss::pst_hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_cac_timeout_fn::pst_mac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 在该信道启动 BSS */
    return hmac_start_bss_in_available_channel(hmac_vap);
}


OAL_STATIC uint32_t hmac_chan_get_cac_time(mac_device_stru *mac_device, mac_vap_stru *mac_vap)
{
    mac_regdomain_info_stru *rd_info = NULL;
    mac_channel_list_stru st_chan_info;
    uint8_t idx;
    uint32_t ret;

    mac_get_regdomain_info(&rd_info);

    if (rd_info->en_dfs_domain == MAC_DFS_DOMAIN_ETSI) {
        ret = mac_get_channel_idx_from_num(mac_vap->st_channel.en_band, mac_vap->st_channel.uc_chan_number,
            mac_vap->st_channel.ext6g_band, &idx);
        if (ret != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_DFS,
                "{hmac_chan_get_cac_time::mac_get_channel_idx_from_num fail. _band:%d, offchan_num:%u}",
                mac_vap->st_channel.en_band, mac_vap->st_channel.uc_chan_number);
            return ret;
        }
        mac_get_ext_chan_info(idx, mac_vap->st_channel.en_bandwidth, &st_chan_info);

        for (idx = 0; idx < st_chan_info.channels; idx++) {
            if (HMAC_DFS_IS_CHAN_WEATHER_RADAR(st_chan_info.ast_channels[idx].us_freq)) {
                return mac_device->st_dfs.st_dfs_info.dfs_cac_in_5600_to_5650_time_ms;
            }
        }
    }

    return mac_device->st_dfs.st_dfs_info.dfs_cac_outof_5600_to_5650_time_ms;
}


OAL_STATIC uint32_t hmac_chan_get_off_chan_cac_time(mac_device_stru *mac_device,
    mac_vap_stru *mac_vap)
{
    mac_channel_list_stru st_chan_info;
    uint8_t idx = 0;

    mac_get_channel_idx_from_num(mac_vap->st_channel.en_band,
        mac_device->st_dfs.st_dfs_info.uc_offchan_num, OAL_FALSE, &idx);

    mac_get_ext_chan_info(idx, mac_vap->st_channel.en_bandwidth, &st_chan_info);

    for (idx = 0; idx < st_chan_info.channels; idx++) {
        if (HMAC_DFS_IS_CHAN_WEATHER_RADAR(st_chan_info.ast_channels[idx].us_freq)) {
            return mac_device->st_dfs.st_dfs_info.off_chan_cac_in_5600_to_5650_time_ms;
        }
    }

    return mac_device->st_dfs.st_dfs_info.off_chan_cac_outof_5600_to_5650_time_ms;
}


void hmac_dfs_cac_stop(mac_device_stru *mac_device, mac_vap_stru *mac_vap)
{
    uint32_t ret;
    uint8_t idx;
    hmac_cac_event_stru st_cac_event;
    uint32_t dfs_status; // 枚举定义hmac_dfs_status_enum

    if (g_st_dfs_result.en_dfs_status == HMAC_CAC_START ||
        g_st_dfs_result.en_dfs_status == HMAC_INS_START) {
        dfs_status = (g_st_dfs_result.en_dfs_status == HMAC_CAC_START) ? HMAC_CAC_STOP : HMAC_INS_STOP;
        hmac_dfs_status_set(dfs_status);
    }
    hmac_dfs_go_cac_forbit_scan_flag_set(mac_vap, OAL_FALSE);

    if (mac_device->st_dfs.st_dfs_cac_timer.en_is_enabled == OAL_TRUE) {
        /* 关闭CAC检测时长定时器 */
        frw_timer_immediate_destroy_timer_m(&(mac_device->st_dfs.st_dfs_cac_timer));
        mac_device->st_dfs.st_dfs_cac_timer.en_is_enabled = OAL_FALSE;

        // 调用内核接口通知应用层CAC停止
        idx = mac_vap->st_channel.uc_chan_idx;
        st_cac_event.en_type = HMAC_CAC_ABORTED;
        st_cac_event.freq = g_ast_freq_map_5g[idx].us_freq;
        st_cac_event.en_bw_mode = mac_vap->st_channel.en_bandwidth;
        ret = hmac_dfs_cac_event_report(mac_vap, &st_cac_event);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_DFS,
                "{[DFS]hmac_dfs_cac_stop::hmac_dfs_cac_event_report failed=%d}", ret);
        }

        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_cac_stop::[DFS]CAC timer stopped.}");
    }
}

void hmac_dfs_off_cac_stop(mac_device_stru *mac_device, mac_vap_stru *mac_vap)
{
    if (mac_device->st_dfs.st_dfs_off_chan_cac_timer.en_is_enabled == OAL_TRUE) {
        /* 关闭OFF CAC检测时长定时器 */
        frw_timer_immediate_destroy_timer_m(&(mac_device->st_dfs.st_dfs_off_chan_cac_timer));
        oam_info_log0(0, OAM_SF_DFS, "{hmac_dfs_cac_stop::[DFS]CAC big timer stopped.}");
    }
    if (mac_device->st_dfs.st_dfs_chan_dwell_timer.en_is_enabled == OAL_TRUE) {
        /* 关闭OFFCAC信道驻留定时器 */
        frw_timer_immediate_destroy_timer_m(&(mac_device->st_dfs.st_dfs_chan_dwell_timer));
        oam_info_log0(0, OAM_SF_DFS, "{hmac_dfs_cac_stop::[DFS]CAC timer stopped.}");
    }
#ifdef _PRE_WLAN_FEATURE_OFFCHAN_CAC
    /* 若VAP在Off-Channel信道上则切回工作信道 */
    if (mac_device->st_dfs.st_dfs_info.uc_offchan_flag & BIT0) {
        oam_info_log0(0, OAM_SF_DFS, "{hmac_dfs_cac_stop::[DFS]switch back to home channel.}");
        hmac_scan_switch_channel_back(mac_vap);
    }
#endif
    mac_device->st_dfs.st_dfs_info.uc_offchan_flag = 0;
}


void hmac_dfs_status_set(uint32_t dfs_status)
{
    oam_warning_log2(0, OAM_SF_CFG, "hmac_dfs_status_set::from [%d] to [%d] !",
        g_st_dfs_result.en_dfs_status, dfs_status);
    g_st_dfs_result.en_dfs_status = dfs_status;
#ifndef _PRE_LINUX_TEST
    oal_do_gettimeofday(&(g_st_dfs_result.st_time));
#endif
}


OAL_STATIC void hmac_dfs_go_cac_forbit_scan_flag_set(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 flag)
{
    g_st_dfs_result.bit_go_cac_forbit_scan = OAL_FALSE;

    if (IS_P2P_GO(mac_vap)) {
        g_st_dfs_result.bit_go_cac_forbit_scan = flag;
    }
}


OAL_STATIC void hmac_dfs_cac_start_save_dfs_result(hmac_vap_stru *hmac_vap)
{
    mac_vap_stru *mac_vap;

    mac_vap = &hmac_vap->st_vap_base_info;
    g_st_dfs_result.us_freq = g_ast_freq_map_5g[mac_vap->st_channel.uc_chan_idx].us_freq;
    g_st_dfs_result.uc_bw = mac_vap_bw_mode_to_bw(mac_vap->st_channel.en_bandwidth);

    if (EOK != memcpy_s(g_st_dfs_result.auc_name, OAL_IF_NAME_SIZE, \
        hmac_vap->auc_name, OAL_IF_NAME_SIZE)) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_dfs_cac_start_save_dfs_result::memcpy fail!");
    }
}


uint32_t hmac_dfs_go_cac_check(mac_vap_stru *mac_vap)
{
    hmac_vap_stru *hmac_vap;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_M2S,
            "{hmac_dfs_go_cac_check::pst_hmac_vap null.}");
        return OAL_FAIL;
    }

    hmac_dfs_status_set(HMAC_INS_START);
    hmac_dfs_cac_start_save_dfs_result(hmac_vap);

    if (!IS_LEGACY_VAP(mac_vap) && g_go_cac == OAL_FALSE) {
        g_go_cac = OAL_TRUE;
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


void hmac_dfs_cac_start(mac_device_stru *mac_device, hmac_vap_stru *hmac_vap)
{
    uint32_t scan_time;
    uint32_t ret;
    uint8_t idx;
    hmac_cac_event_stru st_cac_event;

    /* 如果已经启动 CAC 定时器，则直接返回 */
    if (mac_device->st_dfs.st_dfs_cac_timer.en_is_enabled == OAL_TRUE) {
        return;
    }

    /* 设置 CAC 检测时间 */
    scan_time = hmac_chan_get_cac_time(mac_device, &(hmac_vap->st_vap_base_info));
    oam_warning_log2(0, OAM_SF_DFS, "start cac time=%d ms ch=%d\n",
        scan_time, hmac_vap->st_vap_base_info.st_channel.uc_chan_number);

    if (scan_time < HMAC_DFS_SIXTY_TWO_SEC_IN_MS) {
        mac_device->st_dfs.st_dfs_info.uc_timer_cnt = 0;
        mac_device->st_dfs.st_dfs_info.uc_timer_end_cnt = 1;

        /* 启动 CAC 定时器 */
        frw_timer_create_timer_m(&mac_device->st_dfs.st_dfs_cac_timer, hmac_dfs_cac_timeout_fn,
            scan_time, hmac_vap, OAL_FALSE, OAM_MODULE_ID_HMAC, hmac_vap->st_vap_base_info.core_id);
    } else {
        mac_device->st_dfs.st_dfs_info.uc_timer_cnt = 0;
        mac_device->st_dfs.st_dfs_info.uc_timer_end_cnt = (uint8_t)(scan_time / HMAC_DFS_SIXTY_TWO_SEC_IN_MS);

        /* 启动 CAC 定时器 */
        frw_timer_create_timer_m(&mac_device->st_dfs.st_dfs_cac_timer, hmac_dfs_cac_timeout_fn,
            HMAC_DFS_SIXTY_TWO_SEC_IN_MS, hmac_vap, OAL_FALSE, OAM_MODULE_ID_HMAC,
            hmac_vap->st_vap_base_info.core_id);
    }

    /* start 时保存信息 */
    hmac_dfs_status_set(HMAC_CAC_START);
    hmac_dfs_cac_start_save_dfs_result(hmac_vap);
    hmac_dfs_go_cac_forbit_scan_flag_set(&hmac_vap->st_vap_base_info, OAL_TRUE);

    // 通知wal CAC启动
    idx = hmac_vap->st_vap_base_info.st_channel.uc_chan_idx;
    st_cac_event.en_type = HMAC_CAC_STARTED;
    st_cac_event.freq = g_ast_freq_map_5g[idx].us_freq;
    st_cac_event.en_bw_mode = hmac_vap->st_vap_base_info.st_channel.en_bandwidth;
    ret = hmac_dfs_cac_event_report(&hmac_vap->st_vap_base_info, &st_cac_event);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFS, "{[DFS]hmac_dfs_cac_start::hmac_dfs_cac_event_report failed=%d}", ret);
    }

    oam_warning_log2(0, OAM_SF_DFS,
        "{[DFS]hmac_dfs_cac_start, CAC start, channel number:%d, timer cnt:%d.}",
        hmac_vap->st_vap_base_info.st_channel.uc_chan_number,
        mac_device->st_dfs.st_dfs_info.uc_timer_end_cnt);
}


void hmac_dfs_radar_wait(mac_device_stru *mac_device, hmac_vap_stru *hmac_vap)
{
    mac_vap_stru *mac_vap = NULL;
    uint8_t vap_idx;
    uint8_t vap_cnt_5g = 0;

    if ((hmac_vap == NULL) || (hmac_vap->st_vap_base_info.st_channel.en_band != WLAN_BAND_5G)) {
        return;
    }

    /* 如果雷达使能没有开启直接返回 */
    if (OAL_FALSE == mac_dfs_get_dfs_enable(mac_device)) {
        return;
    }

    /* 遍历device下的所有vap，当所有5G vap都down后，将当前信道状态置为MAC_CHAN_DFS_REQUIRED */
    for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
        mac_vap = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
        if (oal_unlikely(mac_vap == NULL)) {
            oam_warning_log1(0, OAM_SF_DFS,
                "{hmac_dfs_radar_wait::pst_mac_vap null, vap id=%d.", mac_device->auc_vap_id[vap_idx]);

            continue;
        }

        if ((mac_vap->en_vap_state == MAC_VAP_STATE_UP || mac_vap->en_vap_state == MAC_VAP_STATE_AP_WAIT_START) &&
            (mac_vap->st_channel.en_band == WLAN_BAND_5G)) {
            vap_cnt_5g++;
        }
    }

    if (vap_cnt_5g == 0) {
        oam_warning_log1(0, OAM_SF_DFS,
            "{hmac_dfs_radar_wait::reset dfsstatus for device[%d] after all 5g vap down", mac_device->uc_device_id);

        hmac_dfs_channel_list_init(mac_device);
        mac_device->st_dfs.st_dfs_info.en_dfs_init = OAL_TRUE;
        hmac_config_ch_status_sync(mac_device);
    }
}

OAL_STATIC uint32_t hmac_dfs_nol_timeout_fn(void *p_arg)
{
    mac_device_stru *mac_device = NULL;
    mac_dfs_nol_node_stru *nol_node = NULL;
    uint8_t chan_num = 0;

    if (oal_unlikely(p_arg == NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_nol_timeout_fn::p_arg is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    nol_node = (mac_dfs_nol_node_stru *)p_arg;

    mac_device = mac_res_get_dev(nol_node->uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_nol_timeout_fn::pst_mac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_get_channel_num_from_idx(mac_device->en_max_band, nol_node->uc_chan_idx, OAL_FALSE, &chan_num);

    oam_warning_log1(0, OAM_SF_DFS,
        "{[DFS]hmac_dfs_nol_timeout_fn, Non-Occupancy Period expired, remove channel %d from NOL.}", chan_num);

    return hmac_dfs_nol_delchan(mac_device, nol_node);
}


OAL_STATIC uint32_t hmac_dfs_nol_addchan(mac_device_stru *mac_device, uint8_t chan_idx)
{
    mac_dfs_nol_node_stru *nol_node = NULL;
    uint8_t chan_num = 0;
    uint32_t ret;

    /* 如果不可占用周期为0，则不添加新的nol信道 */
    if (mac_device->st_dfs.st_dfs_info.dfs_non_occupancy_period_time_ms == 0) {
        return OAL_SUCC;
    }

    /* 更新可用信道列列表 */
    ret = hmac_dfs_update_available_channel_list(mac_device, chan_idx, OAL_TRUE);
    if (ret == OAL_SUCC) {
        nol_node = (mac_dfs_nol_node_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL,
            sizeof(mac_dfs_nol_node_stru), OAL_TRUE);
        if (oal_unlikely(nol_node == NULL)) {
            oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_nol_addchan::memory not enough.}");
            mac_device->st_ap_channel_list[chan_idx].en_ch_status = MAC_CHAN_DFS_REQUIRED;
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        memset_s(nol_node, sizeof(mac_dfs_nol_node_stru), 0, sizeof(mac_dfs_nol_node_stru));
        nol_node->uc_chan_idx = chan_idx;
        nol_node->uc_device_id = mac_device->uc_device_id;
        oal_dlist_add_tail(&(nol_node->st_entry), &(mac_device->st_dfs.st_dfs_nol));
        mac_get_channel_num_from_idx(mac_device->en_max_band, chan_idx, OAL_FALSE, &chan_num);

        oam_warning_log1(0, OAM_SF_DFS, "{[DFS]hmac_dfs_nol_addchan, add channel %d to NOL.}", chan_num);

        /* 启动Non-Occupancy Peroid定时器 */
        frw_timer_create_timer_m(&nol_node->st_dfs_nol_timer, hmac_dfs_nol_timeout_fn,
            mac_device->st_dfs.st_dfs_info.dfs_non_occupancy_period_time_ms, nol_node,
            OAL_FALSE, OAM_MODULE_ID_HMAC, mac_device->core_id);
    }

    return ret;
}


OAL_STATIC uint32_t hmac_dfs_nol_delchan(mac_device_stru *mac_device,
    mac_dfs_nol_node_stru *nol_node)
{
    /* 更新可用信道列列表 */
    mac_device->st_ap_channel_list[nol_node->uc_chan_idx].en_ch_status = MAC_CHAN_DFS_REQUIRED;
    oal_dlist_delete_entry(&nol_node->st_entry);
    oal_mem_free_m(nol_node, OAL_FALSE);

    hmac_config_ch_status_sync(mac_device);

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_dfs_nol_clear(oal_dlist_head_stru *dlist_head)
{
    mac_device_stru *mac_device = NULL;
    mac_dfs_nol_node_stru *nol_node = NULL;
    oal_dlist_head_stru *dlist_pos = NULL;
    oal_dlist_head_stru *dlist_temp = NULL;
    uint32_t   ret;

    if (oal_unlikely(dlist_head == NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_nol_clear::pst_dlist_head is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_dlist_search_for_each_safe(dlist_pos, dlist_temp, dlist_head)
    {
        nol_node = oal_dlist_get_entry(dlist_pos, mac_dfs_nol_node_stru, st_entry);
        mac_device = mac_res_get_dev(nol_node->uc_device_id);
        if (oal_unlikely(mac_device == NULL)) {
            oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_nol_clear::pst_mac_device is null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 删除相应的Non-Occupancy Peroid定时器 */
        frw_timer_destroy_timer(&nol_node->st_dfs_nol_timer);

        ret = hmac_dfs_nol_delchan(mac_device, nol_node);
        if (ret != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_nol_clear::hmac_dfs_nol_delchan fail.}");
        }
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_dfs_channel_mark_radar(mac_device_stru *mac_device,
    mac_vap_stru *mac_vap)
{
    mac_channel_list_stru st_chan_info;
    uint8_t idx;
    uint32_t cnt = 0;
    uint32_t ret;

    if (mac_device->st_dfs.st_dfs_info.uc_offchan_flag == (BIT0 | BIT1)) {
        ret = mac_get_channel_idx_from_num(mac_vap->st_channel.en_band,
            mac_device->st_dfs.st_dfs_info.uc_offchan_num, OAL_FALSE, &idx);
        if (ret != OAL_SUCC) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_DFS,
                "{hmac_dfs_channel_mark_radar::mac_get_channel_idx_from_num failed = %d.}", ret);
            return OAL_FAIL;
        }

        mac_get_ext_chan_info(idx, mac_vap->st_channel.en_bandwidth, &st_chan_info);
    } else {
        mac_get_ext_chan_info(mac_vap->st_channel.uc_chan_idx, mac_vap->st_channel.en_bandwidth, &st_chan_info);
    }

    for (idx = 0; idx < st_chan_info.channels; idx++) {
        // 只要有一条信道标记成功，就需要切换信道
        if (OAL_SUCC == hmac_dfs_nol_addchan(mac_device, st_chan_info.ast_channels[idx].uc_idx)) {
            cnt++;
        }
    }

    return (uint32_t)((cnt > 0) ? OAL_SUCC : OAL_FAIL);
}
#define HMAC_40MHZ_BITMAP_MASK  0x1EFFFFFF
#define HMAC_80MHZ_BITMAP_MASK  0x00FFFFFF
#define HMAC_160MHZ_BITMAP_MASK 0x00FFFF

OAL_STATIC uint32_t hmac_dfs_is_channel_support_160mbw(wlan_channel_bandwidth_enum_uint8 bandwidth,
    uint32_t chan_bitmap, uint8_t target_idx)
{
#ifdef _PRE_WLAN_FEATURE_160M
    uint32_t window;
    uint8_t step;
    int8_t offset;
    uint8_t idx;

    window = 0xff;
    step = 8; /* 8 开始信道索引 */
    chan_bitmap &= HMAC_160MHZ_BITMAP_MASK;
    switch (bandwidth) {
        case WLAN_BAND_WIDTH_160PLUSPLUSMINUS:
            offset = BYTE_OFFSET_4;
            break;
        case WLAN_BAND_WIDTH_160PLUSMINUSPLUS:
            offset = BYTE_OFFSET_2;
            break;
        case WLAN_BAND_WIDTH_160PLUSMINUSMINUS:
            offset = BYTE_OFFSET_6;
            break;
        case WLAN_BAND_WIDTH_160MINUSPLUSPLUS:
            offset = 1;
            break;
        case WLAN_BAND_WIDTH_160MINUSPLUSMINUS:
            offset = BYTE_OFFSET_5;
            break;
        case WLAN_BAND_WIDTH_160MINUSMINUSPLUS:
            offset = BYTE_OFFSET_3;
            break;
        case WLAN_BAND_WIDTH_160MINUSMINUSMINUS:
            offset = BYTE_OFFSET_7;
            break;
        default:
            offset = 0;
    }

    /* channel 36 ~ 161 */
    for (idx = 0; idx < MAC_CHANNEL165; idx += step) {
        if ((window << idx) == (chan_bitmap & (window << idx))) {
            if ((idx + offset) == target_idx) {
                return OAL_SUCC;
            }
        }
    }
#endif
    return OAL_FAIL;
}

OAL_STATIC uint32_t hmac_dfs_is_channel_support_80mbw(wlan_channel_bandwidth_enum_uint8 bandwidth,
    uint32_t chan_bitmap, uint8_t target_idx)
{
    uint32_t window;
    uint8_t step;
    int8_t offset;
    uint8_t idx;

    window = 0xF;
    step = 4; /* 4 开始信道索引 */
    chan_bitmap &= HMAC_80MHZ_BITMAP_MASK;
    switch (bandwidth) {
        case WLAN_BAND_WIDTH_80PLUSMINUS:
            offset = BYTE_OFFSET_2;
            break;
        case WLAN_BAND_WIDTH_80MINUSPLUS:
            offset = 1;
            break;
        case WLAN_BAND_WIDTH_80MINUSMINUS:
            offset = BYTE_OFFSET_3;
            break;
        default:
            offset = 0;
    }

    /* channel 36 ~ 161 */
    for (idx = 0; idx < MAC_CHANNEL165; idx += step) { /* JP channel 184, 188, 192, 196, */
        /* 4条信道组不了80MHz, 参考802.11 ac Annex E */
        if ((window << idx) == (chan_bitmap & (window << idx))) {
            if ((idx + offset) == target_idx) {
                return OAL_SUCC;
            }
        }
    }
    return OAL_FAIL;
}

OAL_STATIC uint32_t hmac_dfs_is_channel_support_40mbw(wlan_channel_bandwidth_enum_uint8 bandwidth,
    uint32_t chan_bitmap, uint8_t target_idx, uint8_t num_supp_chan)
{
    uint32_t window;
    uint8_t step;
    int8_t offset;
    uint8_t idx;

    window = 0x3; /* 11b */
    step = 2; /* 2 开始信道索引 */
    chan_bitmap &= HMAC_40MHZ_BITMAP_MASK;
    offset = (bandwidth == WLAN_BAND_WIDTH_40PLUS) ? 0 : 1;

    /* channel 36 ~ 144 */
    for (idx = 0; idx < MAC_CHANNEL149; idx += step) { /* skip channel 140 */
        if ((window << idx) == (chan_bitmap & (window << idx))) {
            if ((idx + offset) == target_idx) {
                return OAL_SUCC;
            }
        }
    }

    /* channel 149 ~ 161 */
    for (idx = MAC_CHANNEL149; idx < MAC_CHANNEL165; idx += step) { /* skip channel 165 */
        if ((window << idx) == (chan_bitmap & (window << idx))) {
            if ((idx + offset) == target_idx) {
                return OAL_SUCC;
            }
        }
    }

    /* channel 184 ~ 196 */
    for (idx = MAC_CHANNEL184; idx < num_supp_chan; idx += step) {
        if ((window << idx) == (chan_bitmap & (window << idx))) {
            if ((idx + offset) == target_idx) {
                return OAL_SUCC;
            }
        }
    }

    return OAL_FAIL;
}

OAL_STATIC uint32_t hmac_dfs_is_channel_support_20mbw(wlan_channel_bandwidth_enum_uint8 bandwidth,
    uint32_t chan_bitmap, uint8_t target_idx, uint8_t num_supp_chan)
{
    uint32_t window;
    uint8_t step;
    uint8_t idx;

    window = 0x1;
    step = 1;
    for (idx = 0; idx < num_supp_chan; idx += step) {
        if ((window << idx) == (chan_bitmap & (window << idx))) {
            if (idx == target_idx) {
                    return OAL_SUCC;
            }
        }
    }

    return OAL_FAIL;
}

OAL_STATIC uint32_t hmac_dfs_is_channel_support_bw_check(wlan_channel_bandwidth_enum_uint8 bandwidth,
    uint32_t chan_bitmap, uint8_t target_idx, uint8_t num_supp_chan)
{
    /* 20MHz */
    if (WLAN_BAND_WIDTH_20M == bandwidth) {
        return hmac_dfs_is_channel_support_20mbw(bandwidth, chan_bitmap, target_idx, num_supp_chan);
    } else if (bandwidth < WLAN_BAND_WIDTH_80PLUSPLUS) {
        /* 40MHz */
        return hmac_dfs_is_channel_support_40mbw(bandwidth, chan_bitmap, target_idx, num_supp_chan);
    } else if (bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS) {
        /* 80MHz */
        return hmac_dfs_is_channel_support_80mbw(bandwidth, chan_bitmap, target_idx);
    } else {
        /* 160MHz */
        return hmac_dfs_is_channel_support_160mbw(bandwidth, chan_bitmap, target_idx);
    }
}

OAL_STATIC void hmac_dfs_set_chan_bitmap(mac_device_stru *mac_device, uint8_t idx, uint32_t *chan_bitmap)
{
    if (channel_status_is_not_use(mac_device->st_ap_channel_list[idx].en_ch_status)) {
        if (mac_device->st_dfs.st_dfs_info.custom_chanlist_bitmap) {
            if (mac_device->st_dfs.st_dfs_info.custom_chanlist_bitmap & (0x01 << idx)) {
                *chan_bitmap |= (0x1 << idx);
            }
        } else {
            *chan_bitmap |= (0x1 << idx);
        }
    }
}


OAL_STATIC uint32_t hmac_dfs_is_channel_support_bw(mac_vap_stru *mac_vap,
    uint8_t channel, wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    mac_device_stru *mac_device;
    uint8_t num_supp_chan = mac_get_num_supp_channel(mac_vap->st_channel.en_band);
    uint32_t chan_bitmap = 0;
    uint8_t idx;
    uint8_t target_idx;
    uint32_t ret;

    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_DFS,
            "{hmac_dfs_is_channel_support_bw::pst_mac_device null, device_id = %d.}", mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = mac_get_channel_idx_from_num(mac_vap->st_channel.en_band, channel, OAL_FALSE, &target_idx);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_DFS,
            "{hmac_dfs_is_channel_support_bw::get channel idx failed(%d).}", mac_vap->uc_device_id);
        return OAL_FAIL;
    }

hmac_dfs_is_channel_support_bw_update:

    /* 获取可用信道位图 */
    for (idx = 0; idx < num_supp_chan; idx++) {
        hmac_dfs_set_chan_bitmap(mac_device, idx, &chan_bitmap);
    }

    if (!chan_bitmap) {
        // 此策略经产品讨论，当因为雷达检测导致已经无可用信道可跳时，将
        // 原来被标记为雷达的信道全部清掉，重新开始选择，然后CAC
        if (hmac_dfs_nol_clear(&mac_device->st_dfs.st_dfs_nol) != OAL_SUCC) {
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_DFS,
                "{hmac_dfs_is_channel_support_bw::hmac_dfs_nol_clear() Fail!}");

            return OAL_FAIL;
        }

        /*lint -e801*/
        goto hmac_dfs_is_channel_support_bw_update;
        /*lint +e801*/
    }

    return hmac_dfs_is_channel_support_bw_check(bandwidth, chan_bitmap, target_idx, num_supp_chan);
}


OAL_STATIC void hmac_dfs_160m_select_new_chan(mac_vap_stru *mac_vap, uint8_t *new_channel,
    wlan_channel_bandwidth_enum_uint8 *new_bandwidth)
{
    if (mac_vap->st_channel.uc_chan_idx < MAC_CHANNEL52) {
        *new_channel    = mac_vap->st_channel.uc_chan_number;
        *new_bandwidth  = mac_vap_get_bandwith(WLAN_BW_CAP_80M, mac_vap->st_channel.en_bandwidth);
    } else {
        *new_channel    = 36; /* 36信道 */
        *new_bandwidth  = WLAN_BAND_WIDTH_80PLUSPLUS;

        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CHAN,
            "{hmac_dfs_160m_select_new_chan::chan idx error [%d]}", mac_vap->st_channel.uc_chan_idx);
    }

    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CHAN,
        "{hmac_dfs_160m_select_new_chan::[DFS]select_channel::bandwidth = %d, channelnum = %d!}",
        *new_bandwidth, *new_channel);
}

OAL_STATIC uint8_t hmac_dfs_available_chan_idx_set(wlan_channel_bandwidth_enum_uint8 try_bandwidth,
    uint8_t idx, uint8_t *available_chan_idx, uint32_t chan_bitmap, uint8_t num_supp_chan)
{
    uint32_t window;
    uint8_t step;
    uint8_t available_chan_cnt = 0;

    /* 20MHz */
    if (try_bandwidth == WLAN_BAND_WIDTH_20M) {
        window = 0x1; /* 1b */
        step = 1;

        for (idx = 0; idx < num_supp_chan; idx += step) {
            if ((window << idx) == (chan_bitmap & (window << idx))) {
                available_chan_idx[available_chan_cnt++] = idx;
            }
        }
    } else if (try_bandwidth < WLAN_BAND_WIDTH_80PLUSPLUS) {
        /* 40MHz */
        window = 0x3; /* 11b */
        step = 2; /* 2 信道开始索引 */
        chan_bitmap &= HMAC_40MHZ_BITMAP_MASK;

        /* channel 36 ~ 144 */
        for (idx = 0; idx < MAC_CHANNEL149; idx += step) { /* skip channel 140 */
            if ((window << idx) == (chan_bitmap & (window << idx))) {
                available_chan_idx[available_chan_cnt++] = idx;
            }
        }

        /* channel 149 ~ 161 */
        for (idx = MAC_CHANNEL149; idx < MAC_CHANNEL165; idx += step) { /* skip channel 165 */
            if ((window << idx) == (chan_bitmap & (window << idx))) {
                available_chan_idx[available_chan_cnt++] = idx;
            }
        }

        /* channel 184 ~ 196 */
        for (idx = MAC_CHANNEL184; idx < num_supp_chan; idx += step) {
            if ((window << idx) == (chan_bitmap & (window << idx))) {
                available_chan_idx[available_chan_cnt++] = idx;
            }
        }
    } else if (try_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS) {
        /* 80MHz上检测到有问题,固定选择36-64的信道,带宽80M */
        available_chan_idx[available_chan_cnt++] = MAC_CHANNEL36;
    }

    return available_chan_cnt;
}

OAL_STATIC uint8_t hmac_dfs_avoid_adjacent_channel(mac_vap_stru *mac_vap, uint8_t *new_channel,
    wlan_channel_bandwidth_enum_uint8 try_bandwidth, uint8_t *available_chan_idx, uint8_t available_chan_cnt)
{
    uint8_t sub = 0;
    uint32_t window;
    uint8_t cur_sub;
    uint8_t cur_idx;
    uint8_t idx = 0;

    for (window = 0; window < available_chan_cnt; window++) {
        cur_idx = available_chan_idx[window];
        switch (try_bandwidth) {
            case WLAN_BAND_WIDTH_40MINUS:
            case WLAN_BAND_WIDTH_80MINUSPLUS:
                cur_idx += 1;
                break;
            case WLAN_BAND_WIDTH_80PLUSMINUS:
                cur_idx += BYTE_OFFSET_2;
                break;
            case WLAN_BAND_WIDTH_80MINUSMINUS:
                cur_idx += BYTE_OFFSET_3;
                break;
            default:
                break;
        }
        mac_get_channel_num_from_idx(mac_vap->st_channel.en_band, cur_idx, OAL_FALSE, new_channel);
        cur_sub = oal_absolute_sub(mac_vap->st_channel.uc_chan_number, *new_channel);
        if (cur_sub > sub) {
            sub = cur_sub;
            idx = cur_idx;
        }
    }

    return idx;
}

OAL_STATIC uint32_t hmac_dfs_select_random_below_160m_channel(mac_vap_stru *mac_vap,
    wlan_channel_bandwidth_enum_uint8 try_bandwidth, uint8_t *new_channel,
    wlan_channel_bandwidth_enum_uint8 *new_bandwidth)
{
    mac_device_stru *mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    uint8_t num_supp_chan = mac_get_num_supp_channel(mac_vap->st_channel.en_band);
    uint32_t chan_bitmap = 0;
    uint8_t available_chan_idx[MAC_CHANNEL_FREQ_5_BUTT];
    uint8_t available_chan_cnt;
    uint8_t idx;
    uint8_t idx_tmp;

    /* 获取可用信道位图 */
    for (idx = 0; idx < num_supp_chan; idx++) {
        hmac_dfs_set_chan_bitmap(mac_device, idx, &chan_bitmap);
    }

    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_DFS,
        "{hmac_dfs_select_random_channel::custom_chanlist_bitmap=0x%x,chan_bitmap=0x%x!}",
        mac_device->st_dfs.st_dfs_info.custom_chanlist_bitmap, chan_bitmap);
    available_chan_cnt = hmac_dfs_available_chan_idx_set(try_bandwidth, idx,
        available_chan_idx, chan_bitmap, num_supp_chan);
    /* 如果找不到可用信道，从管制域支持的信道中，选择最低可用信道，带宽20MHz */
    if (available_chan_cnt == 0) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_DFS,
            "{hmac_dfs_select_random_channel::No availabe channel,NOL will be cleared!}");
        // 此策略经产品讨论，当因为雷达检测导致已经无可用信道可跳时，将
        // 原来被标记为雷达的信道全部清掉，重新开始选择，然后CAC
        if (hmac_dfs_nol_clear(&mac_device->st_dfs.st_dfs_nol) != OAL_SUCC) {
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_DFS,
                "{hmac_dfs_select_random_channel::hmac_dfs_nol_clear() Fail!}");

            return OAL_FAIL;
        }
        if (try_bandwidth == WLAN_BAND_WIDTH_20M) {
            return OAL_FAIL;
        }
        try_bandwidth = WLAN_BAND_WIDTH_20M;
        return OAL_SUCC_GO_ON;  // 需要重新获取信道位图
    }

    idx_tmp = hmac_dfs_avoid_adjacent_channel(mac_vap, new_channel,
        try_bandwidth, available_chan_idx, available_chan_cnt);
    if (idx_tmp != 0) {
        idx = idx_tmp;
    }

    mac_get_channel_num_from_idx(mac_vap->st_channel.en_band, idx, OAL_FALSE, new_channel);

    *new_bandwidth = try_bandwidth;

    oal_io_print("[DFS]select_channel::bandwidth = %d, channelnum = %d.\n", *new_bandwidth, *new_channel);
    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CHAN,
        "{hmac_dfs_select_random_channel::[DFS]select_channel::bandwidth = %d, channelnum = %d!}",
        *new_bandwidth, *new_channel);
    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_dfs_select_random_channel(mac_vap_stru *mac_vap,
    uint8_t *new_channel, wlan_channel_bandwidth_enum_uint8 *new_bandwidth)
{
    mac_device_stru *mac_device = NULL;
    wlan_channel_bandwidth_enum_uint8 try_bandwidth;
    uint32_t ret;

    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_DFS,
            "{hmac_dfs_select_random_channel::pst_mac_device null, device_id = %d.}", mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (mac_device->st_dfs.st_dfs_info.custom_chanlist_bitmap) {
        try_bandwidth = WLAN_BAND_WIDTH_20M;  // 强制20M带宽
    } else {
        try_bandwidth = mac_vap->st_channel.en_bandwidth;
    }
    if (try_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS) {
        ret = hmac_dfs_select_random_below_160m_channel(mac_vap, try_bandwidth, new_channel, new_bandwidth);
        if (ret == OAL_SUCC_GO_ON) {
            try_bandwidth = WLAN_BAND_WIDTH_20M;
            if (hmac_dfs_select_random_below_160m_channel(mac_vap, try_bandwidth, new_channel, new_bandwidth) ==
                OAL_FAIL) {
                return OAL_FAIL;
            }
        } else if (ret == OAL_FAIL) {
            return ret;
        }
    } else {
        hmac_dfs_160m_select_new_chan(mac_vap, new_channel, new_bandwidth);
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_dfs_select_new_channel(mac_device_stru *mac_device, mac_vap_stru *mac_vap)
{
    uint32_t ret;
    uint8_t new_channel = 0;
    wlan_channel_bandwidth_enum_uint8 new_bandwidth = WLAN_BAND_WIDTH_BUTT;

    if (mac_device->st_dfs.st_dfs_info.uc_custom_next_chnum) {
        new_channel = mac_device->st_dfs.st_dfs_info.uc_custom_next_chnum;
        new_bandwidth = mac_vap->st_channel.en_bandwidth;
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_DFS, "{hmac_dfs_ap_wait_start_radar_handler::\
            next ch(%d) and bw(%d) set by user.}", new_channel, new_bandwidth);
        if (hmac_dfs_is_channel_support_bw(mac_vap, new_channel, new_bandwidth) != OAL_SUCC) {
            oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_DFS, "{hmac_dfs_switch_channel_for_radar::new chan(%d) \
                not support curr bw mode(%d).}", new_channel, new_bandwidth);
            // 如果应用设定的下一跳信道不支持当前的带宽模式，那么按照驱动的随机策略选择
            ret = hmac_dfs_select_random_channel(mac_vap, &new_channel, &new_bandwidth);
            if (ret != OAL_SUCC) {
                oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_DFS,
                    "{hmac_dfs_ap_wait_start_radar_handler::hmac_dfs_select_random_channel failed.}");
                return ret;
            }
        }
        mac_device->st_dfs.st_dfs_info.uc_custom_next_chnum = 0;
        mac_device->st_dfs.st_dfs_info.en_next_ch_width_type = WLAN_BAND_WIDTH_BUTT;
    } else {
        ret = hmac_dfs_select_random_channel(mac_vap, &new_channel, &new_bandwidth);
        if (ret != OAL_SUCC) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_DFS,
                "{hmac_dfs_ap_wait_start_radar_handler::hmac_dfs_select_random_channel failed.}");
            return ret;
        }
    }

    ret = mac_is_channel_num_valid(mac_vap->st_channel.en_band, new_channel, mac_vap->st_channel.ext6g_band);
    if ((ret != OAL_SUCC) || (new_bandwidth >= WLAN_BAND_WIDTH_BUTT)) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_DFS, "{hmac_dfs_ap_wait_start_radar_handler::cann't start network\
            using the selected chan[%d] or bw[%d]}", new_channel, new_bandwidth);
        return ret;
    }

    /* 只需要设置硬件寄存器一次，然后同步一下软件vap的配置即可 */
    hmac_chan_multi_select_channel_mac(mac_vap, new_channel, new_bandwidth);

    return OAL_SUCC;
}


uint32_t hmac_dfs_ap_wait_start_radar_handler(hmac_vap_stru *hmac_vap)
{
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *mac_vap = NULL;

    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_ap_wait_start_radar_handler::pst_hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap = &(hmac_vap->st_vap_base_info);
    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_DFS, "{hmac_dfs_ap_wait_start_radar_handler::mac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 关闭 CAC 检测 */
    hmac_dfs_status_set(HMAC_CAC_DETECT);
    hmac_dfs_cac_stop(mac_device, mac_vap);

    /* 标记主、次信道检测到雷达 */
    if (hmac_dfs_channel_mark_radar(mac_device, mac_vap) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 重新选择一条信道 */
    if (hmac_dfs_select_new_channel(mac_device, mac_vap) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 判断是否需要进行 */
    if (OAL_TRUE == hmac_dfs_need_for_cac(mac_device, &hmac_vap->st_vap_base_info)) {
        hmac_dfs_cac_start(mac_device, hmac_vap);
        return OAL_SUCC;
    }
    /* 切换到非雷达信道,把vap状态置为up,恢复发送队列 */
    hmac_cac_chan_ctrl_machw_tx(mac_vap, OAL_TRUE);
    /* 否则，直接启动BSS */
    return hmac_start_bss_in_available_channel(hmac_vap);
}

/* 信道切换计数 */
#define WLAN_CHAN_SWITCH_DETECT_RADAR_CNT 1

uint32_t hmac_dfs_switch_channel_for_radar(mac_device_stru *mac_device,
    mac_vap_stru *mac_vap)
{
    uint8_t new_channel = 0;
    wlan_channel_bandwidth_enum_uint8 new_bandwidth = WLAN_BAND_WIDTH_BUTT;
    uint32_t ret;

    /* 关闭 CAC 检测 */
    hmac_dfs_status_set(HMAC_INS_DETECT);
    hmac_dfs_cac_stop(mac_device, mac_vap);

    /* 标记主、次信道检测到雷达 */
    if (hmac_dfs_channel_mark_radar(mac_device, mac_vap) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 如果AP已经准备进行信道切换，则直接返回，不做任何处理 */
    if (WLAN_CH_SWITCH_STATUS_1 == mac_vap->st_ch_switch_info.en_ch_switch_status) {
        return OAL_SUCC;
    }

    if (mac_device->st_dfs.st_dfs_info.uc_custom_next_chnum) {
        new_channel = mac_device->st_dfs.st_dfs_info.uc_custom_next_chnum;
        if (mac_device->st_dfs.st_dfs_info.en_next_ch_width_type != WLAN_BAND_WIDTH_BUTT) {
            new_bandwidth = mac_device->st_dfs.st_dfs_info.en_next_ch_width_type;
        } else {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_DFS,
                "{hmac_dfs_switch_channel_for_radar::user not set next ch bw.}");
            new_bandwidth = mac_vap->st_channel.en_bandwidth;
        }

        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CHAN, "{hmac_dfs_switch_channel_for_radar::\
            next ch(%d) and bw(%d) set by user.}", new_channel, new_bandwidth);

        if (hmac_dfs_is_channel_support_bw(mac_vap, new_channel, new_bandwidth) != OAL_SUCC) {
            oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_DFS,
                "{hmac_dfs_switch_channel_for_radar::new chan(%d) not support current bw mode(%d).}",
                new_channel, new_bandwidth);
            // 如果应用设定的下一跳信道不支持当前的带宽模式，那么按照驱动的随机策略选择
            ret = hmac_dfs_select_random_channel(mac_vap, &new_channel, &new_bandwidth);
            if (ret != OAL_SUCC) {
                oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_DFS,
                    "{hmac_dfs_switch_channel_for_radar::hmac_dfs_select_random_channel failed. }");
                return ret;
            }
        }
        mac_device->st_dfs.st_dfs_info.uc_custom_next_chnum = 0;
        mac_device->st_dfs.st_dfs_info.en_next_ch_width_type = WLAN_BAND_WIDTH_BUTT;
    } else {
        ret = hmac_dfs_select_random_channel(mac_vap, &new_channel, &new_bandwidth);
        if (ret != OAL_SUCC) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_DFS,
                "{hmac_dfs_switch_channel_for_radar::hmac_dfs_select_random_channel failed.}");
            return ret;
        }
    }

    mac_vap->st_ch_switch_info.uc_ch_switch_cnt = WLAN_CHAN_SWITCH_DETECT_RADAR_CNT;
    hmac_chan_multi_switch_to_new_channel(mac_vap, new_channel, new_bandwidth);

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_dfs_off_chan_cac_timeout_fn(void *p_arg)
{
    mac_device_stru *mac_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_dfs_info_stru *dfs_info = NULL;

    if (oal_unlikely(p_arg == NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_timeout_fn::p_arg is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)p_arg;

    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_timeout_fn::pst_mac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    dfs_info = &mac_device->st_dfs.st_dfs_info;

    dfs_info->uc_timer_cnt++;

    if (dfs_info->uc_timer_cnt != dfs_info->uc_timer_end_cnt) {
        /* 启动OFF CHAN CAC 定时器 */
        frw_timer_create_timer_m(&mac_device->st_dfs.st_dfs_off_chan_cac_timer, hmac_dfs_off_chan_cac_timeout_fn,
            HMAC_DFS_SIXTY_TWO_SEC_IN_MS, hmac_vap, OAL_FALSE,
            OAM_MODULE_ID_HMAC, hmac_vap->st_vap_base_info.core_id);
        return OAL_SUCC;
    }

    oal_io_print("[DFS]hmac_dfs_off_chan_cac_timeout_fn::off-channel cac end.\n");

    /* 关闭Off-channel CAC 信道驻留定时器 */
    frw_timer_immediate_destroy_timer_m(&(mac_device->st_dfs.st_dfs_chan_dwell_timer));
#ifdef _PRE_WLAN_FEATURE_OFFCHAN_CAC
    /* 若VAP在Off-Channel信道上则切回工作信道 */
    if (mac_device->st_dfs.st_dfs_info.uc_offchan_flag & BIT0) {
        hmac_scan_switch_channel_back(&(hmac_vap->st_vap_base_info));
    }
#endif
    mac_device->st_dfs.st_dfs_info.uc_offchan_flag = 0;

    return OAL_SUCC;
}


uint32_t hmac_dfs_ap_up_radar_handler(hmac_vap_stru *hmac_vap)
{
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *mac_vap = NULL;
    uint8_t offchan_flag;

    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_ap_wait_start_radar_handler::pst_hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap = &(hmac_vap->st_vap_base_info);

    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_DFS,
            "{hmac_dfs_ap_up_radar_handler::pst_mac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    offchan_flag = mac_device->st_dfs.st_dfs_info.uc_offchan_flag;
    /* off-chan cac 期间 */
    if (offchan_flag & BIT1) {
        /* home channel检测到radar */
        if (!(offchan_flag & BIT0)) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_DFS, "{[DFS]radar detected in channel %d.}",
                mac_vap->st_channel.uc_chan_number);

            /* 关闭Off-channel CAC 定时器 */
            frw_timer_immediate_destroy_timer_m(&(mac_device->st_dfs.st_dfs_off_chan_cac_timer));

            hmac_dfs_off_chan_cac_timeout_fn(hmac_vap);

            return hmac_dfs_switch_channel_for_radar(mac_device, mac_vap);
        } else {
            /* off channel检测到radar */
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_DFS, "{[DFS]radar detected in channel %d.}",
                mac_device->st_dfs.st_dfs_info.uc_offchan_num);
            /* 标记主、次信道检测到雷达 */
            hmac_dfs_channel_mark_radar(mac_device, mac_vap);
            return OAL_SUCC;
        }
    } else {
        /* in service monitor期间 */
        return hmac_dfs_switch_channel_for_radar(mac_device, mac_vap);
    }
}
#ifdef _PRE_WLAN_FEATURE_OFFCHAN_CAC

OAL_STATIC void hmac_scan_switch_channel_off(mac_vap_stru *mac_vap)
{
    frw_event_mem_stru *event_mem;
    frw_event_stru *event;
    uint32_t ret;

    /* 申请事件内存 */
    event_mem = frw_event_alloc_m(0);
    if (oal_unlikely(event_mem == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_DFS, "{hmac_scan_switch_channel_off::pst_event_mem null.}");

        return;
    }

    event = frw_get_event_stru(event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX,
        DMAC_WLAN_CTX_EVENT_SUB_TYPR_SWITCH_TO_OFF_CHAN, 0, FRW_EVENT_PIPELINE_STAGE_1,
        mac_vap->uc_chip_id, mac_vap->uc_device_id, mac_vap->uc_vap_id);

    /* 分发事件 */
    ret = frw_event_dispatch_event(event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_DFS,
            "{hmac_scan_switch_channel_off::frw_event_dispatch_event failed[%d].}", ret);
        frw_event_free_m(event_mem);
        return;
    }

    /* 释放事件 */
    frw_event_free_m(event_mem);

    return;
}


OAL_STATIC void hmac_scan_switch_channel_back(mac_vap_stru *mac_vap)
{
    frw_event_mem_stru *event_mem;
    frw_event_stru *event;
    uint32_t ret;

    /* 申请事件内存 */
    event_mem = frw_event_alloc_m(0);
    if (oal_unlikely(event_mem == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_DFS, "{hmac_scan_switch_channel_back::pst_event_mem null.}");
        return;
    }

    event = frw_get_event_stru(event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX,
        DMAC_WLAN_CTX_EVENT_SUB_TYPR_SWITCH_TO_HOME_CHAN, 0,
        FRW_EVENT_PIPELINE_STAGE_1, mac_vap->uc_chip_id, mac_vap->uc_device_id, mac_vap->uc_vap_id);

    /* 分发事件 */
    ret = frw_event_dispatch_event(event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_DFS,
            "{hmac_scan_switch_channel_back::frw_event_dispatch_event failed[%d].}", ret);
        frw_event_free_m(event_mem);
        return;
    }

    /* 释放事件 */
    frw_event_free_m(event_mem);

    return;
}
#endif


OAL_STATIC uint32_t hmac_dfs_off_chan_cac_opern_ch_dwell_timeout(void *p_arg)
{
    mac_device_stru *mac_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    if (oal_unlikely(p_arg == NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_opern_ch_dwell_timeout::p_arg is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)p_arg;

    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_opern_ch_dwell_timeout::pst_mac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 当前信道为offchan 标志 */
    mac_device->st_dfs.st_dfs_info.uc_offchan_flag = (BIT0 | BIT1);
#ifdef _PRE_WLAN_FEATURE_OFFCHAN_CAC
    /* 切换到Off-channel CAC检测信道 */
    hmac_scan_switch_channel_off(&hmac_vap->st_vap_base_info);
#endif
    /* 将当前off-channel cac信道信息赋值给VAP */
    /* 启动Off-channel信道上检测时长定时器 */
    frw_timer_create_timer_m(&mac_device->st_dfs.st_dfs_chan_dwell_timer,
        hmac_dfs_off_chan_cac_off_ch_dwell_timeout,
        mac_device->st_dfs.st_dfs_info.us_dfs_off_chan_cac_off_chan_dwell_time,
        hmac_vap, OAL_FALSE, OAM_MODULE_ID_HMAC, hmac_vap->st_vap_base_info.core_id);

    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_dfs_off_chan_cac_off_ch_dwell_timeout(void *p_arg)
{
    mac_device_stru *mac_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    if (oal_unlikely(p_arg == NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_off_ch_dwell_timeout::p_arg is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)p_arg;

    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_off_ch_dwell_timeout::pst_mac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 当前信道为homechan标志 */
    mac_device->st_dfs.st_dfs_info.uc_offchan_flag = BIT1;
#ifdef _PRE_WLAN_FEATURE_OFFCHAN_CAC
    /* 切换回工作信道 */
    hmac_scan_switch_channel_back(&(hmac_vap->st_vap_base_info));
#endif
    /* 启动在工作信道上的工作时长定时器 */
    frw_timer_create_timer_m(&mac_device->st_dfs.st_dfs_chan_dwell_timer,
        hmac_dfs_off_chan_cac_opern_ch_dwell_timeout,
        mac_device->st_dfs.st_dfs_info.us_dfs_off_chan_cac_opern_chan_dwell_time,
        hmac_vap, OAL_FALSE, OAM_MODULE_ID_HMAC, hmac_vap->st_vap_base_info.core_id);

    return OAL_SUCC;
}


void hmac_dfs_off_chan_cac_start(mac_device_stru *mac_device, hmac_vap_stru *hmac_vap)
{
    uint32_t scan_time;

    /* 如果已经启动OFF-CHAN-CAC 定时器, 直接返回 */
    if (mac_device->st_dfs.st_dfs_off_chan_cac_timer.en_is_enabled == OAL_TRUE) {
        oam_info_log0(0, OAM_SF_DFS,
            "{hmac_dfs_off_chan_cac_start::off-chan-cac is already started by another vap.}");
        return;
    }

    /* 设置 Off-Channel CAC 检测时间 */
    scan_time = hmac_chan_get_off_chan_cac_time(mac_device, &hmac_vap->st_vap_base_info);

    mac_device->st_dfs.st_dfs_info.uc_timer_cnt = 0;
    mac_device->st_dfs.st_dfs_info.uc_timer_end_cnt = (uint8_t)(scan_time / HMAC_DFS_SIXTY_TWO_SEC_IN_MS);

    /* 启动 CAC 定时器 */
    frw_timer_create_timer_m(&mac_device->st_dfs.st_dfs_off_chan_cac_timer,
        hmac_dfs_off_chan_cac_timeout_fn, HMAC_DFS_SIXTY_TWO_SEC_IN_MS,
        hmac_vap, OAL_FALSE, OAM_MODULE_ID_HMAC, hmac_vap->st_vap_base_info.core_id);

    oam_info_log2(0, OAM_SF_DFS,
        "{[DFS]hmac_dfs_off_chan_cac_start, OFF-CHAN-CAC start, channel number:%d, scan time = %d.}",
        mac_device->st_dfs.st_dfs_info.uc_offchan_num, scan_time);

    if (hmac_dfs_off_chan_cac_opern_ch_dwell_timeout(hmac_vap) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_start::off_chan_cac_opern_ch_dwell_timeout failed.}");
    }
}

oal_bool_enum_uint8 hmac_dfs_try_cac(hmac_device_stru *hmac_device, mac_vap_stru *mac_vap)
{
    hmac_vap_stru *hmac_vap = NULL;
    mac_device_stru *mac_device = NULL;
    mac_channel_stru *channel = NULL;

    if (oal_unlikely(oal_any_null_ptr2(hmac_device, mac_vap))) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_dfs_try_cac::pst_device_base_info null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_device = hmac_device->pst_device_base_info;

    /* 如果已经启动 CAC 定时器，则直接返回 */
    if (mac_device->st_dfs.st_dfs_cac_timer.en_is_enabled == OAL_TRUE) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ACS, "cac started\n");
        return OAL_TRUE;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{hmac_dfs_try_cac::mac_res_get_hmac_vap fail.vap_id[%u]}", mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    // 至此，vap的信道结构为最终的信道,但是可能并未设置到硬件
    channel = &hmac_vap->st_vap_base_info.st_channel;

    /* 如果需要进行 CAC 检测，这里启动定时器，直到CAC结束后才启动BSS(VAP UP) */
    if (OAL_TRUE == hmac_dfs_need_for_cac(mac_device, &hmac_vap->st_vap_base_info)) {
        // 此时设置硬件信道
        hmac_chan_sync(&hmac_vap->st_vap_base_info, channel->uc_chan_number, channel->en_bandwidth, OAL_TRUE);
        /* 把vap状态置为pause, 抛事件给dmac关发送队列, 待cac超时后把vap状态置为up,恢复发送队列 */
        hmac_cac_chan_ctrl_machw_tx(mac_vap, OAL_FALSE);
        hmac_dfs_cac_start(mac_device, hmac_vap);
        hmac_device->en_init_scan = OAL_FALSE;  // 进入CAC，认为初始扫描完成。
        hmac_device->en_in_init_scan = OAL_FALSE;
        mac_device->st_dfs.st_dfs_info.en_dfs_switch &= ~BIT1;

        return OAL_TRUE;
    }

    return OAL_FALSE;
}

uint32_t hmac_dfs_init_scan_hook(hmac_scan_record_stru *scan_record,
    hmac_device_stru *hmac_device)
{
    uint8_t vap_id;
    mac_vap_stru *vap = NULL;
    mac_device_stru *mac_device = NULL;

    oam_warning_log0(0, OAM_SF_ACS, "dfs init scan hook run\n");

    mac_device = hmac_device->pst_device_base_info;

    for (vap_id = 0; vap_id < mac_device->uc_vap_num; vap_id++) {
        vap = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_id]);
        if (vap != NULL && vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
            /* 只要当前device上已经开始进行CAC过程，那么同device的其他VAP不再进行 */
            /* CAC完成之后会统一设置信道并启动VAP */
            /* DBAC场景下依然可以满足 */
            if (OAL_TRUE == hmac_dfs_try_cac(hmac_device, vap)) {
                oam_warning_log0(0, OAM_SF_ACS, "cac started\n");
                return OAL_SUCC;
            }
        }
    }

    oam_warning_log0(0, OAM_SF_ACS, "cac not need\n");
    return OAL_FAIL;
}


#ifdef _PRE_WLAN_FEATURE_DFS
void hmac_chan_multi_switch_to_new_channel(mac_vap_stru *pst_mac_vap,
    uint8_t uc_channel, wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    uint8_t uc_vap_idx;
    mac_device_stru *pst_mac_device;
    mac_vap_stru *pst_vap;
    uint8_t uc_chan_idx;
    uint32_t ret;

    oam_info_log2(pst_mac_vap->uc_vap_id, OAM_SF_2040,
        "{hmac_chan_multi_switch_to_new_channel::uc_channel=%d,en_bandwidth=%d}", uc_channel, en_bandwidth);

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
            "{hmac_chan_multi_switch_to_new_channel::pst_device(%d) null.}", pst_mac_vap->uc_device_id);
        return;
    }

    if (pst_mac_device->uc_vap_num == 0) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{hmac_chan_multi_switch_to_new_channel::none vap.}");
        return;
    }

    ret = mac_get_channel_idx_from_num(pst_mac_vap->st_channel.en_band, uc_channel, OAL_FALSE, &uc_chan_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
            "{hmac_chan_multi_switch_to_new_channel::mac_get_channel_idx_from_num failed[%d].}", ret);

        return;
    }

    pst_mac_device->uc_csa_vap_cnt = 0;

    /* 遍历device下所有ap，设置ap信道参数，发送CSA帧，准备切换至新信道运行 */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == NULL) {
            oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                "{hmac_chan_multi_switch_to_new_channel::pst_ap(%d) null.}", pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        /* 只有running AP需要发送CSA帧 */
        if ((pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
            (pst_vap->en_vap_state == MAC_VAP_STATE_UP)) {
            pst_mac_device->uc_csa_vap_cnt++;
            pst_vap->st_ch_switch_info.en_csa_mode = WLAN_CSA_MODE_TX_DISABLE;
            hmac_chan_initiate_switch_to_new_channel(pst_vap, uc_channel, en_bandwidth);
        } else { /* 其它站点只需要更新信道信息 */
            /* 更新VAP下的主20MHz信道号、带宽模式、信道索引 */
            pst_vap->st_channel.uc_chan_number = uc_channel;
            pst_vap->st_channel.uc_chan_idx = uc_chan_idx;
            pst_vap->st_channel.en_bandwidth = en_bandwidth;
        }
    }
}

#else

void hmac_chan_multi_switch_to_new_channel(mac_vap_stru *pst_mac_vap,
    uint8_t uc_channel, wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    uint8_t uc_vap_idx;
    mac_device_stru *pst_device = NULL;
    mac_vap_stru *pst_ap = NULL;

    oam_info_log2(pst_mac_vap->uc_vap_id, OAM_SF_2040,
        "{hmac_chan_multi_switch_to_new_channel::uc_channel=%d,en_bandwidth=%d}", uc_channel, en_bandwidth);

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_device == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
            "{hmac_chan_multi_switch_to_new_channel::pst_device null.}");
        return;
    }

    if (pst_device->uc_vap_num == 0) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_chan_multi_switch_to_new_channel::none vap.}");
        return;
    }

    /* 遍历device下所有ap，设置ap信道参数，准备切换至新信道运行 */
    for (uc_vap_idx = 0; uc_vap_idx < pst_device->uc_vap_num; uc_vap_idx++) {
        pst_ap = (mac_vap_stru *)mac_res_get_mac_vap(pst_device->auc_vap_id[uc_vap_idx]);
        if (pst_ap == NULL) {
            oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                           "{hmac_chan_multi_switch_to_new_channel::pst_ap null.}",
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
#endif /* end of _PRE_WLAN_FEATURE_DFS */


OAL_STATIC void hmac_cac_chan_ctrl_machw_tx_event(mac_vap_stru *pst_mac_vap, uint8_t uc_cac_machw_en)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event;
    uint32_t ret;
    dmac_set_cac_machw_info_stru *pst_cac_machw_info;

    /* 申请事件内存 */
    pst_event_mem = frw_event_alloc_m(sizeof(dmac_set_cac_machw_info_stru));
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_cac_chan_ctrl_machw_tx::pst_event_mem null.}");
        return;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX,
        DMAC_WALN_CTX_EVENT_SUB_TYPR_DFS_CAC_CTRL_TX, sizeof(dmac_set_cac_machw_info_stru),
        FRW_EVENT_PIPELINE_STAGE_1, pst_mac_vap->uc_chip_id,
        pst_mac_vap->uc_device_id, pst_mac_vap->uc_vap_id);

    /* 填写事件payload */
    pst_cac_machw_info = (dmac_set_cac_machw_info_stru *)pst_event->auc_event_data;
    pst_cac_machw_info->uc_cac_machw_en = uc_cac_machw_en;

    /* 分发事件 */
    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_cac_chan_ctrl_machw_tx::frw_event_dispatch_event failed[%d].}", ret);
        frw_event_free_m(pst_event_mem);
        return;
    }

    /* 释放事件 */
    frw_event_free_m(pst_event_mem);
}


void hmac_cac_chan_ctrl_machw_tx(mac_vap_stru *pst_mac_vap, uint8_t uc_cac_machw_en)
{
    if (uc_cac_machw_en == OAL_FALSE) {
        mac_vap_state_change(pst_mac_vap, MAC_VAP_STATE_PAUSE);
    } else if (uc_cac_machw_en == OAL_TRUE) {
        mac_vap_state_change(pst_mac_vap, MAC_VAP_STATE_UP);
    } else {
        return;
    }

    hmac_cac_chan_ctrl_machw_tx_event(pst_mac_vap, uc_cac_machw_en);
}


uint32_t hmac_cac_abort_scan_check(hmac_device_stru *pst_hmac_device)
{
    hmac_dfs_radar_result_stru *pst_dfs_result;
    pst_dfs_result = hmac_dfs_result_info_get();
    if (pst_dfs_result->bit_go_cac_forbit_scan == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_SCAN,
            "{hmac_cac_abort_scan_check:: go cac forbit scan.}");
        return OAL_SUCC;
    }
    return OAL_FAIL;
}


void mac_dfs_init(mac_device_stru *pst_mac_device)
{
    /* 软件雷达检测默认关闭 */
    mac_dfs_set_dfs_enable(pst_mac_device, OAL_TRUE);
    /* debug默认关闭 */
    mac_dfs_set_debug_level(pst_mac_device, 0);
}


void mac_dfs_set_cac_enable(mac_device_stru *pst_mac_device, oal_bool_enum_uint8 en_val)
{
    pst_mac_device->st_dfs.st_dfs_info.en_cac_switch = en_val;
}


void mac_dfs_set_dfs_enable(mac_device_stru *pst_mac_device, oal_bool_enum_uint8 en_val)
{
    pst_mac_device->st_dfs.st_dfs_info.en_dfs_switch = en_val;

    /* 如果 软件雷达检测使能 关闭，则关闭CAC检测 */
    if (en_val == OAL_FALSE) {
        pst_mac_device->st_dfs.st_dfs_info.en_cac_switch = OAL_FALSE;
    }
}


oal_bool_enum_uint8 mac_dfs_get_dfs_enable(mac_device_stru *pst_mac_device)
{
    if (pst_mac_device->en_max_band == WLAN_BAND_5G) {
        return pst_mac_device->st_dfs.st_dfs_info.en_dfs_switch;
    }

    return OAL_FALSE;
}


void mac_dfs_set_debug_level(mac_device_stru *pst_mac_device, uint8_t uc_debug_lev)
{
    pst_mac_device->st_dfs.st_dfs_info.uc_debug_level = uc_debug_lev;
}


void mac_dfs_set_offchan_cac_enable(mac_device_stru *pst_mac_device, oal_bool_enum_uint8 en_val)
{
    pst_mac_device->st_dfs.st_dfs_info.en_offchan_cac_switch = en_val;
}


oal_bool_enum_uint8 mac_dfs_get_offchan_cac_enable(mac_device_stru *pst_mac_device)
{
    mac_regdomain_info_stru *pst_rd_info = NULL;

    mac_get_regdomain_info(&pst_rd_info);
    if (pst_rd_info->en_dfs_domain == MAC_DFS_DOMAIN_ETSI) {
        return pst_mac_device->st_dfs.st_dfs_info.en_offchan_cac_switch;
    }

    return OAL_FALSE;
}


oal_bool_enum_uint8 mac_dfs_get_cac_enable(mac_device_stru *pst_mac_device)
{
    return pst_mac_device->st_dfs.st_dfs_info.en_cac_switch;
}


oal_bool_enum_uint8 mac_vap_get_dfs_enable(mac_vap_stru *pst_mac_vap)
{
    mac_device_stru *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_get_dfs_enable::mac_res_get_dev[%d] Error.}",
            pst_mac_vap->uc_device_id);
        return OAL_FALSE;
    }

    /*lint -save -e731 */
    /* 先判断vap是否支持，再判断mac_device的dfs开关是否打开 */
    if (OAL_TRUE == MAC_VAP_IS_SUPPORT_DFS(pst_mac_vap)) {
        return mac_dfs_get_dfs_enable(pst_mac_device);
    }
    /*lint -restore */
    return OAL_FALSE;
}


uint32_t hmac_dfs_zero_wait_report_event(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    mac_cfg_channel_param_stru *channel_info = (mac_cfg_channel_param_stru *)param;

    if ((mac_vap == NULL) || (channel_info == NULL)) {
        oam_error_log0(0, 0, "hmac_zero_wait_dfs_report_event:param is NULL!");
        return OAL_ERR_CODE_PTR_NULL;
    }
    mac_vap->st_channel.en_bandwidth = channel_info->en_bandwidth;
    return OAL_SUCC;
}


void hmac_config_start_zero_wait_dfs_handle(mac_vap_stru *mac_vap, mac_cfg_channel_param_stru *channel_info)
{
    zero_wait_dfs_param_stru zero_wait_dfs_param;
    wlan_channel_bandwidth_enum_uint8 new_80bw;
    mac_cfg_mib_by_bw_param_stru mib_cfg = {0};
    uint8_t channel_index = 0;
    uint32_t ret;
    mac_vap_stru *mac_vap_temp = NULL;
    mac_device_stru *mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        return;
    }

    /* zero wait dfs工作场景: 单AP或p2p go，且带宽为160m，且支持雷达检测功能 */
    if ((hmac_find_up_vap(mac_device, &mac_vap_temp) == OAL_SUCC) ||
        (!IS_AP(mac_vap)) ||
        (mac_vap_bw_mode_to_bw(channel_info->en_bandwidth) != WLAN_BW_CAP_160M) ||
        (mac_vap_get_dfs_enable(mac_vap) == OAL_FALSE)) {
        return;
    }
    if (memset_s(&zero_wait_dfs_param,sizeof(zero_wait_dfs_param_stru),0,sizeof(zero_wait_dfs_param_stru)) != EOK) {
        return;
    }
    if (mac_get_channel_idx_from_num(channel_info->en_band, channel_info->uc_channel,
        channel_info->ext6g_band, &channel_index) != OAL_SUCC) {
        return;
    }
    zero_wait_dfs_param.channel_param.en_band = channel_info->en_band;
    zero_wait_dfs_param.channel_param.en_bandwidth = channel_info->en_bandwidth;
    zero_wait_dfs_param.channel_param.uc_chan_number = channel_info->uc_channel;
    zero_wait_dfs_param.channel_param.uc_chan_idx = channel_index;
    zero_wait_dfs_param.channel_param.ext6g_band = channel_info->ext6g_band;
    /* 将带宽160m修改为80m，设置mib能力 */
    new_80bw = mac_regdomain_get_support_bw_mode(WLAN_BW_CAP_80M, channel_info->uc_channel);
    if (mac_vap_bw_mode_to_bw(new_80bw) != WLAN_BW_CAP_80M) {
        return;
    }
    mib_cfg.en_band = mac_vap->st_channel.en_band;
    mib_cfg.en_bandwidth = new_80bw;
    hmac_config_set_mib_by_bw(mac_vap, sizeof(mac_cfg_mib_by_bw_param_stru), (uint8_t *)&mib_cfg);
    /* **************************************************************************
     抛事件到DMAC层, 同步DMAC数据
    ************************************************************************** */
    zero_wait_dfs_param.radar_detect_time = 60000; // 60000ms
    oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CHAN,
        "hmac_config_zero_wait_dfs_not_start_160m_handle::send event to dmac, set register");
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_ZERO_WAIT_DFS, sizeof(zero_wait_dfs_param),
        (uint8_t *)&zero_wait_dfs_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CHAN,
            "{hmac_config_zero_wait_dfs_not_start_160m_handle::send event failed[%d].}", ret);
    }
    /* 将信道带宽由原来160m设置为80m */
    channel_info->en_bandwidth = new_80bw;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

