

#ifdef _PRE_WLAN_FEATURE_P2P

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_util.h"
#include "hmac_ext_if.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_vap.h"
#include "hmac_rx_filter.h"
#include "hmac_fsm.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_mgmt_ap.h"
#include "mac_frame.h"
#include "hmac_p2p.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_P2P_C

/*****************************************************************************
  2 静态函数声明
*****************************************************************************/
/*****************************************************************************
  3 全局变量定义
*****************************************************************************/
/*****************************************************************************
  4 函数实现
*****************************************************************************/
typedef struct mac_vap_state_priority {
    oal_uint8 uc_priority;
} mac_vap_state_priority_stru;

typedef struct hmac_input_req_priority {
    oal_uint8 uc_priority;
} hmac_input_req_priority_stru;

mac_vap_state_priority_stru g_mac_vap_state_priority_table[MAC_VAP_STATE_BUTT] = {
    { 0 },  // {MAC_VAP_STATE_INIT
    { 2 },  // {MAC_VAP_STATE_UP                    /* VAP UP */
    { 0 },  // {MAC_VAP_STATE_PAUSE                 /* pause , for ap &sta */
    /* ap 独有状态 */
    { 0 },  // {MAC_VAP_STATE_AP_PAUSE
    /* sta独有状态 */
    { 0 },   // {MAC_VAP_STATE_STA_FAKE_UP
    { 10 },  // {MAC_VAP_STATE_STA_WAIT_SCAN
    { 0 },   // {MAC_VAP_STATE_STA_SCAN_COMP
    { 10 },  // {MAC_VAP_STATE_STA_JOIN_COMP
    { 10 },  // {MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2
    { 10 },  // {MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4
    { 10 },  // {MAC_VAP_STATE_STA_AUTH_COMP
    { 10 },  // {MAC_VAP_STATE_STA_WAIT_ASOC
    { 10 },  // {MAC_VAP_STATE_STA_OBSS_SCAN
    { 10 },  // {MAC_VAP_STATE_STA_BG_SCAN
    { 0 },   // MAC_VAP_STATE_STA_LISTEN/* p2p0 监听 */
};

hmac_input_req_priority_stru g_mac_fsm_input_type_priority_table[HMAC_FSM_INPUT_TYPE_BUTT] = {
    { 0 }, /* HMAC_FSM_INPUT_RX_MGMT */
    { 0 }, /* HMAC_FSM_INPUT_RX_DATA */
    { 0 }, /* HMAC_FSM_INPUT_TX_DATA */
    { 0 }, /* HMAC_FSM_INPUT_TIMER0_OUT */
    { 0 }, /* HMAC_FSM_INPUT_MISC */

    { 0 }, /* HMAC_FSM_INPUT_START_REQ */

    { 5 }, /* HMAC_FSM_INPUT_SCAN_REQ */
    { 5 }, /* HMAC_FSM_INPUT_JOIN_REQ */
    { 5 }, /* HMAC_FSM_INPUT_AUTH_REQ */
    { 5 }, /* HMAC_FSM_INPUT_ASOC_REQ */

    { 5 }, /* HMAC_FSM_INPUT_LISTEN_REQ */ /* P2P 监听 */
    { 0 } /* HMAC_FSM_INPUT_LISTEN_TIMEOUT */  /* P2P 监听超时 */
};


oal_uint32 hmac_p2p_check_can_enter_state(mac_vap_stru *pst_mac_vap,
                                          hmac_fsm_input_type_enum_uint8 en_input_req)
{
    mac_device_stru *pst_mac_device;
    mac_vap_stru *pst_other_vap;
    oal_uint8 uc_vap_num;
    oal_uint8 uc_vap_idx;

    /* 2.1 检查其他vap 状态，判断输入事件优先级是否比vap 状态优先级高
    *  如果输入事件优先级高，则可以执行输入事件
 */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_p2p_check_can_enter_state::mac_res_get_dev fail.device_id = %u}",
                         pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    uc_vap_num = pst_mac_device->uc_vap_num;

    for (uc_vap_idx = 0; uc_vap_idx < uc_vap_num; uc_vap_idx++) {
        pst_other_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_other_vap == OAL_PTR_NULL) {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_p2p_check_can_enter_state::hmac_res_get_mac_vap fail.vap_idx = %u}",
                             pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if (pst_other_vap->uc_vap_id == pst_mac_vap->uc_vap_id) {
            /* 如果检测到是自己，则继续检查其他VAP 状态 */
            continue;
        }
        if ((g_mac_vap_state_priority_table[pst_other_vap->en_vap_state].uc_priority
             > g_mac_fsm_input_type_priority_table[en_input_req].uc_priority)
            || hmac_go_is_auth(pst_other_vap)) {
            return OAL_ERR_CODE_CONFIG_BUSY;
        }
    }
    return OAL_SUCC;
}


oal_uint32 hmac_p2p_get_home_channel(mac_vap_stru *pst_mac_vap,
                                     oal_uint32 *pul_home_channel,
                                     wlan_channel_bandwidth_enum_uint8 *pen_home_channel_bandwidth)
{
    mac_device_stru *pst_mac_device;
    oal_uint8 uc_vap_idx;

    oal_uint32 ul_home_channel = 0;
    oal_uint32 ul_last_home_channel = 0;
    wlan_channel_bandwidth_enum_uint8 en_home_channel_bandwidth = WLAN_BAND_WIDTH_20M;
    wlan_channel_bandwidth_enum_uint8 en_last_home_channel_bandwidth = WLAN_BAND_WIDTH_20M;

    if (pst_mac_vap == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_p2p_get_home_channel::pst_mac_vap is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 初始化 */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_p2p_get_home_channel::mac_res_get_dev fail.device_id = %u}",
                         pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    *pul_home_channel = 0;
    *pen_home_channel_bandwidth = WLAN_BAND_WIDTH_20M;

    /* 获取home 信道 */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_mac_vap == OAL_PTR_NULL) {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_p2p_get_home_channel:get_mac_vap fail.vap_idx = %u}", uc_vap_idx);
            continue;
        }

        if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP) {
            ul_home_channel = pst_mac_vap->st_channel.uc_chan_number;
            en_home_channel_bandwidth = pst_mac_vap->st_channel.en_bandwidth;

            if (ul_last_home_channel == 0) {
                ul_last_home_channel = ul_home_channel;
                en_last_home_channel_bandwidth = en_home_channel_bandwidth;
            } else if ((ul_last_home_channel != ul_home_channel)
                       || (en_last_home_channel_bandwidth != en_home_channel_bandwidth)) {
                oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                                 "{hmac_p2p_get_home_channel::failed.home_channel[%d], last_home_channel[%d], \
                                 home bw[%d], last bw[%d].}", ul_home_channel, ul_last_home_channel,
                                 en_home_channel_bandwidth, en_last_home_channel_bandwidth);
                return OAL_FAIL;
            }
        }
    }

    *pul_home_channel = ul_home_channel;
    *pen_home_channel_bandwidth = en_home_channel_bandwidth;

    return OAL_SUCC;
}


oal_uint32 hmac_check_p2p_vap_num(mac_device_stru *pst_mac_device, wlan_p2p_mode_enum_uint8 en_p2p_mode)
{
    if (en_p2p_mode == WLAN_P2P_DEV_MODE) {
        /* 判断已创建的P2P_DEVICE 个数是否达到最大值 */
        /* P2P0只能创建1个、且不能与AP共存 */
        if (pst_mac_device->st_p2p_info.uc_p2p_device_num >= WLAN_MAX_SERVICE_P2P_DEV_NUM) {
            /* 已创建的P2P_DEV个数达到最大值 */
            OAM_WARNING_LOG1(0, OAM_SF_CFG,
                             "hmac_config_check_vap_num:have created p2p_device.can not create p2p_device any more[%d]",
                             pst_mac_device->st_p2p_info.uc_p2p_device_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }

        /* 剩余资源是否足够申请一份P2P_DEV */
        if (pst_mac_device->uc_sta_num >= WLAN_SERVICE_STA_MAX_NUM_PER_DEVICE) {
            /* 已有AP创建 */
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "hmac_check_p2p_vap_num:can not create p2p_device any more[uc_sta_num:%d]",
                             pst_mac_device->uc_sta_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }
    } else if (en_p2p_mode == WLAN_P2P_CL_MODE) {
        /* 判断已创建的P2P_CLIENT 个数是否达到最大值 */
        if (pst_mac_device->st_p2p_info.uc_p2p_goclient_num >= WLAN_MAX_SERVICE_P2P_GOCLIENT_NUM) {
            /* 已创建的P2P_DEV个数达到最大值 */
            OAM_WARNING_LOG1(0, OAM_SF_CFG,
                             "{hmac_check_p2p_vap_num::have created 1 p2p_GO/Client.can not createany more[%d].}",
                             pst_mac_device->st_p2p_info.uc_p2p_goclient_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }

        /* P2P0是否已经创建 */
        if (pst_mac_device->st_p2p_info.uc_p2p_device_num != WLAN_MAX_SERVICE_P2P_DEV_NUM) {
            /* 已有AP创建 */
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_check_p2p_vap_num::there ia no p2p0 %d, so can not create p2p_CL.}",
                             pst_mac_device->st_p2p_info.uc_p2p_device_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }
    } else if (en_p2p_mode == WLAN_P2P_GO_MODE) {
        /* 判断已创建的P2P_CL/P2P_GO 个数是否达到最大值 */
        if (pst_mac_device->st_p2p_info.uc_p2p_goclient_num >= WLAN_MAX_SERVICE_P2P_GOCLIENT_NUM) {
            /* 已创建的P2P_DEV个数达到最大值 */
            OAM_WARNING_LOG1(0, OAM_SF_CFG,
                             "{hmac_check_p2p_vap_num::have created 1 p2p_GO/Client.can not createany more[%d].}",
                             pst_mac_device->st_p2p_info.uc_p2p_goclient_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }
        /* 是否有足够的AP vap模式资源供创建一个P2P_GO */
        if ((pst_mac_device->uc_vap_num - pst_mac_device->uc_sta_num) >= WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE) {
            /* 已创建的AP个数达到最大值4 */
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_check_p2p_vap_num::ap num exceeds the supported spec[%d].}",
                             (pst_mac_device->uc_vap_num - pst_mac_device->uc_sta_num));
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }
    } else {
        return OAL_ERR_CODE_INVALID_CONFIG; /* 入参错误，无效配置 */
    }
    return OAL_SUCC;
}


oal_uint32 hmac_add_p2p_cl_vap(mac_vap_stru *pst_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_cfg_add_vap_param_stru *pst_param;
    hmac_vap_stru *pst_hmac_vap;
    oal_uint32 ul_ret;
    mac_device_stru *pst_dev;
    oal_uint8 uc_vap_id;
    wlan_p2p_mode_enum_uint8 en_p2p_mode;

    pst_param = (mac_cfg_add_vap_param_stru *)puc_param;
    pst_dev = mac_res_get_dev(pst_vap->uc_device_id);
    if (oal_unlikely(pst_dev == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_add_p2p_cl_vap::pst_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* VAP个数判断 */
    en_p2p_mode = pst_param->en_p2p_mode;
    ul_ret = hmac_check_p2p_vap_num(pst_dev, en_p2p_mode);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_add_p2p_cl_vap::hmac_config_check_vap_num failed[%d].}",
                         ul_ret);
        return ul_ret;
    }

    /* P2P CL 和P2P0 共用一个VAP 结构，创建P2P CL 时不需要申请VAP 资源，需要返回p2p0 的vap 结构 */
    uc_vap_id = pst_dev->st_p2p_info.uc_p2p0_vap_idx;
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
    if (oal_unlikely(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "hmac_add_p2p_cl_vap::mac_res_get_hmac_vap failed.vap_id:[%d]",
                       uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_param->uc_vap_id = uc_vap_id;
    pst_hmac_vap->pst_net_device = pst_param->pst_net_dev;

    /* 包括'\0' */
    if (memcpy_s(pst_hmac_vap->auc_name, OAL_IF_NAME_SIZE, pst_param->pst_net_dev->name, OAL_IF_NAME_SIZE) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "hmac_add_p2p_cl_vap::memcpy fail!");
    }

    /* 将申请到的mac_vap空间挂到net_device priv指针上去 */
    oal_net_dev_priv(pst_param->pst_net_dev) = &pst_hmac_vap->st_vap_base_info;

    pst_param->us_muti_user_id = pst_hmac_vap->st_vap_base_info.us_multi_user_idx;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(&pst_hmac_vap->st_vap_base_info, WLAN_CFGID_ADD_VAP, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        /* 异常处理，释放内存 */
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_add_p2p_cl_vap::hmac_config_alloc_event failed[%d].}",
                       ul_ret);
        return ul_ret;
    }

    if (pst_param->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* 初始化uc_assoc_vap_id为最大值代表ap未关联 */
        mac_vap_set_assoc_id(&pst_hmac_vap->st_vap_base_info, 0xff);
    }
    mac_vap_set_p2p_mode(&pst_hmac_vap->st_vap_base_info, pst_param->en_p2p_mode);
    mac_inc_p2p_num(&pst_hmac_vap->st_vap_base_info);
    /* 设置帧过滤 */
    hmac_set_rx_filter_value(&pst_hmac_vap->st_vap_base_info);
    return OAL_SUCC;
}


oal_uint32 hmac_del_p2p_cl_vap(mac_vap_stru *pst_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    oal_uint32 ul_ret;
    mac_device_stru *pst_device;
    oal_uint8 uc_vap_id;
    mac_cfg_del_vap_param_stru *pst_del_vap_param;

    if (oal_unlikely((pst_vap == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_del_vap::param null,pst_vap=%x puc_param=%x.}", (uintptr_t)pst_vap,
                       (uintptr_t)puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_del_vap_param = (mac_cfg_del_vap_param_stru *)puc_param;

    pst_device = mac_res_get_dev(pst_vap->uc_device_id);
    if (oal_unlikely(pst_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_vap::pst_device null.devid[%d]}",
                       pst_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 如果是删除P2P CL ，则不需要释放VAP 资源 */
    uc_vap_id = pst_device->st_p2p_info.uc_p2p0_vap_idx;
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
    if (oal_unlikely(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_vap::mac_res_get_hmac_vap fail.vap_id[%d]}",
                       uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_net_dev_priv(pst_hmac_vap->pst_net_device) = OAL_PTR_NULL;
    pst_hmac_vap->pst_net_device = pst_hmac_vap->pst_p2p0_net_device;

    mac_dec_p2p_num(&pst_hmac_vap->st_vap_base_info);
    mac_vap_set_p2p_mode(&pst_hmac_vap->st_vap_base_info, WLAN_P2P_DEV_MODE);

    if (memcpy_s(pst_hmac_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID,
                 WLAN_MAC_ADDR_LEN,
                 pst_hmac_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID,
                 WLAN_MAC_ADDR_LEN) != EOK) {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_CFG, "hmac_del_p2p_cl_vap::memcpy fail!");
    }

    if (pst_hmac_vap->puc_asoc_req_ie_buff != OAL_PTR_NULL) {
        oal_mem_free_m(pst_hmac_vap->puc_asoc_req_ie_buff, OAL_TRUE);
        pst_hmac_vap->puc_asoc_req_ie_buff = OAL_PTR_NULL;
        pst_hmac_vap->ul_asoc_req_ie_len = 0;
    }

    /***************************************************************************
                          抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_vap, WLAN_CFGID_DEL_VAP, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_del_p2p_cl_vap::hmac_config_send_event failed[%d].}",
                         ul_ret);
        return ul_ret;
    }
    return OAL_SUCC;
}


oal_uint32 hmac_p2p_send_listen_expired_to_host(hmac_vap_stru *pst_hmac_vap)
{
    mac_device_stru *pst_mac_device;
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event;
    oal_wireless_dev_stru *pst_wdev;
    mac_p2p_info_stru *pst_p2p_info;
    hmac_p2p_listen_expired_stru *pst_p2p_listen_expired;

    pst_mac_device = mac_res_get_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                         "{hmac_send_mgmt_to_host::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_p2p_info = &pst_mac_device->st_p2p_info;

    /* 填写上报监听超时, 上报的网络设备应该采用p2p0 */
    if (pst_hmac_vap->pst_p2p0_net_device && pst_hmac_vap->pst_p2p0_net_device->ieee80211_ptr) {
        pst_wdev = pst_hmac_vap->pst_p2p0_net_device->ieee80211_ptr;
    } else if (pst_hmac_vap->pst_net_device && pst_hmac_vap->pst_net_device->ieee80211_ptr) {
        pst_wdev = pst_hmac_vap->pst_net_device->ieee80211_ptr;
    } else {
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                         "{hmac_send_mgmt_to_host::vap has deleted.}");
        return OAL_FAIL;
    }

    /* 组装事件到WAL ，上报监听结束 */
    pst_event_mem = frw_event_alloc_m(OAL_SIZEOF(hmac_p2p_listen_expired_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                       "{hmac_send_mgmt_to_host::pst_event_mem null.}");
        return OAL_FAIL;
    }

    /* 填写事件 */
    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_LISTEN_EXPIRED,
                       OAL_SIZEOF(hmac_p2p_listen_expired_stru),
                       FRW_EVENT_PIPELINE_STAGE_0,
                       pst_hmac_vap->st_vap_base_info.uc_chip_id,
                       pst_hmac_vap->st_vap_base_info.uc_device_id,
                       pst_hmac_vap->st_vap_base_info.uc_vap_id);

    pst_p2p_listen_expired = (hmac_p2p_listen_expired_stru *)(pst_event->auc_event_data);
    pst_p2p_listen_expired->st_listen_channel = pst_p2p_info->st_listen_channel;
    pst_p2p_listen_expired->pst_wdev = pst_wdev;

    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);

    return OAL_SUCC;
}


oal_uint32 hmac_p2p_send_listen_expired_to_device(hmac_vap_stru *pst_hmac_vap)
{
    mac_device_stru *pst_mac_device;
    mac_vap_stru *pst_mac_vap;
    mac_p2p_info_stru *pst_p2p_info;
    oal_uint32 ul_ret;
    hmac_device_stru *pst_hmac_device;

    pst_mac_device = mac_res_get_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                       "{hmac_p2p_send_listen_expired_to_device::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_info_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                  "{hmac_p2p_send_listen_expired_to_device:: listen timeout!.}");

    /***************************************************************************
     抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    pst_p2p_info = &pst_mac_device->st_p2p_info;
    pst_mac_vap = &pst_hmac_vap->st_vap_base_info;
    ul_ret = hmac_config_send_event(pst_mac_vap,
                                    WLAN_CFGID_CFG80211_CANCEL_REMAIN_ON_CHANNEL,
                                    OAL_SIZEOF(mac_p2p_info_stru),
                                    (oal_uint8 *)pst_p2p_info);

    /* 强制stop listen */
    pst_hmac_device = hmac_res_get_mac_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(pst_hmac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_p2p_send_listen_expired_to_device::pst_hmac_device[%d] null!}",
                       pst_hmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (pst_hmac_device->st_scan_mgmt.en_is_scanning == OAL_TRUE) {
        pst_hmac_device->st_scan_mgmt.en_is_scanning = OAL_FALSE;
    }

    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                         "{hmac_p2p_send_listen_expired_to_device::hmac_config_send_event failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_void hmac_disable_p2p_pm(hmac_vap_stru *pst_hmac_vap)
{
    mac_vap_stru *pst_mac_vap;
    mac_cfg_p2p_ops_param_stru st_p2p_ops;
    mac_cfg_p2p_noa_param_stru st_p2p_noa;
    oal_uint32 ul_ret;

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);

    memset_s(&st_p2p_noa, OAL_SIZEOF(st_p2p_noa), 0, OAL_SIZEOF(st_p2p_noa));
    ul_ret = hmac_config_set_p2p_ps_noa(pst_mac_vap, OAL_SIZEOF(mac_cfg_p2p_noa_param_stru), (oal_uint8 *)&st_p2p_noa);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                       "{hmac_disable_p2p_pm::hmac_config_set_p2p_ps_noa disable p2p NoA fail.}");
    }
    memset_s(&st_p2p_ops, OAL_SIZEOF(st_p2p_ops), 0, OAL_SIZEOF(st_p2p_ops));
    ul_ret = hmac_config_set_p2p_ps_ops(pst_mac_vap, OAL_SIZEOF(mac_cfg_p2p_ops_param_stru), (oal_uint8 *)&st_p2p_ops);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                       "{hmac_disable_p2p_pm::hmac_config_set_p2p_ps_ops disable p2p OppPS fail.}");
    }
}


oal_bool_enum_uint8 hmac_is_p2p_go_neg_req_frame(OAL_CONST oal_uint8 *puc_data)
{
    if ((puc_data[MAC_ACTION_OFFSET_CATEGORY + MAC_80211_FRAME_LEN] == MAC_ACTION_CATEGORY_PUBLIC) &&
        (puc_data[P2P_PUB_ACT_OUI_OFF1 + MAC_80211_FRAME_LEN] == WFA_OUI_BYTE1) &&
        (puc_data[P2P_PUB_ACT_OUI_OFF2 + MAC_80211_FRAME_LEN] == WFA_OUI_BYTE2) &&
        (puc_data[P2P_PUB_ACT_OUI_OFF3 + MAC_80211_FRAME_LEN] == WFA_OUI_BYTE3) &&
        (WFA_P2P_v1_0 == puc_data[P2P_PUB_ACT_OUI_TYPE_OFF + MAC_80211_FRAME_LEN]) &&
        (puc_data[P2P_PUB_ACT_OUI_SUBTYPE_OFF + MAC_80211_FRAME_LEN] == P2P_PAF_GON_REQ)) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}


oal_uint8 hmac_get_p2p_status(oal_uint32 ul_p2p_status, hmac_cfgp2p_status_enum_uint32 en_status)
{
    if (ul_p2p_status & BIT(en_status)) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}


oal_void hmac_set_p2p_status(oal_uint32 *pul_p2p_status, hmac_cfgp2p_status_enum_uint32 en_status)
{
    *pul_p2p_status |= ((oal_uint32)BIT(en_status));
}


oal_void hmac_clr_p2p_status(oal_uint32 *pul_p2p_status, hmac_cfgp2p_status_enum_uint32 en_status)
{
    *pul_p2p_status &= ~((oal_uint32)BIT(en_status));
}


oal_void hmac_p2p_listen_comp_cb(void *p_arg)
{
    hmac_vap_stru *pst_hmac_vap;
    mac_device_stru *pst_mac_device;
    hmac_scan_record_stru *pst_scan_record;

    pst_scan_record = (hmac_scan_record_stru *)p_arg;

    /* 判断listen完成时的状态 */
    if (pst_scan_record->en_scan_rsp_status != MAC_SCAN_SUCCESS) {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{hmac_p2p_listen_comp_cb::listen failed, listen rsp status: %d.}",
                         pst_scan_record->en_scan_rsp_status);
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_scan_record->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{hmac_p2p_listen_comp_cb::pst_hmac_vap is null:vap_id %d.}",
                       pst_scan_record->uc_vap_id);
        return;
    }

    pst_mac_device = mac_res_get_dev(pst_scan_record->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{hmac_p2p_listen_comp_cb::pst_mac_device is null:vap_id %d.}",
                       pst_scan_record->uc_device_id);
        return;
    }

    
    if (pst_scan_record->ull_cookie == pst_mac_device->st_p2p_info.ull_last_roc_id) {
        /* 状态机调用: hmac_p2p_listen_timeout */
        if (hmac_fsm_call_func_sta(pst_hmac_vap, HMAC_FSM_INPUT_LISTEN_TIMEOUT,
                                   &(pst_hmac_vap->st_vap_base_info)) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_P2P, "{hmac_p2p_listen_comp_cb::hmac_fsm_call_func_sta fail.}");
        }
    } else {
        oam_warning_log3(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                         "{ignore listen complete.scanreportcookie[%x],current_listen_cookie[%x],ull_last_roc_id[%x].}",
                         pst_scan_record->ull_cookie, pst_mac_device->st_scan_params.ull_cookie,
                         pst_mac_device->st_p2p_info.ull_last_roc_id);
    }

    return;
}


oal_void hmac_mgmt_tx_roc_comp_cb(void *p_arg)
{
    hmac_vap_stru *pst_hmac_vap;
    mac_device_stru *pst_mac_device;
    hmac_scan_record_stru *pst_scan_record;

    pst_scan_record = (hmac_scan_record_stru *)p_arg;

    /* 判断listen完成时的状态 */
    if (pst_scan_record->en_scan_rsp_status != MAC_SCAN_SUCCESS) {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{hmac_mgmt_tx_roc_comp_cb::listen failed, listen rsp status: %d.}",
                         pst_scan_record->en_scan_rsp_status);
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_scan_record->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{hmac_mgmt_tx_roc_comp_cb::pst_hmac_vap is null:vap_id %d.}",
                       pst_scan_record->uc_vap_id);
        return;
    }

    pst_mac_device = mac_res_get_dev(pst_scan_record->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{hmac_mgmt_tx_roc_comp_cb::pst_mac_device is null:vap_id %d.}",
                       pst_scan_record->uc_device_id);
        return;
    }

    /* 由于P2P0 和P2P_CL 共用vap 结构体，监听超时，返回监听前保存的状态 */
    if ((pst_hmac_vap->st_vap_base_info.en_vap_state >= MAC_VAP_STATE_STA_JOIN_COMP) &&
        (pst_hmac_vap->st_vap_base_info.en_vap_state <= MAC_VAP_STATE_STA_WAIT_ASOC)) {
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                         "{hmac_mgmt_tx_roc_comp_cb::vap is connecting,can not change vap state.}");
    } else {
        mac_vap_state_change(&pst_hmac_vap->st_vap_base_info, pst_mac_device->st_p2p_info.en_last_vap_state);
    }

    oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P, "{hmac_mgmt_tx_roc_comp_cb}");
}


OAL_STATIC oal_void hmac_cfg80211_prepare_listen_req_param(mac_scan_req_stru *pst_scan_params,
                                                           oal_int8 *puc_param)
{
    mac_remain_on_channel_param_stru *pst_remain_on_channel;
    mac_channel_stru *pst_channel_tmp;

    pst_remain_on_channel = (mac_remain_on_channel_param_stru *)puc_param;

    memset_s(pst_scan_params, OAL_SIZEOF(mac_scan_req_stru), 0, OAL_SIZEOF(mac_scan_req_stru));

    /* 设置监听信道信息到扫描参数中 */
    pst_scan_params->ast_channel_list[0].en_band = pst_remain_on_channel->en_band;
    pst_scan_params->ast_channel_list[0].en_bandwidth = pst_remain_on_channel->en_listen_channel_type;
    pst_scan_params->ast_channel_list[0].uc_chan_number = pst_remain_on_channel->uc_listen_channel;
    pst_scan_params->ast_channel_list[0].uc_idx = 0;
    pst_channel_tmp = &(pst_scan_params->ast_channel_list[0]);
    if (mac_get_channel_idx_from_num(pst_channel_tmp->en_band, pst_channel_tmp->uc_chan_number,
                                     &(pst_channel_tmp->uc_idx)) != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_P2P,
                         "{mac_get_channel_idx_from_num fail.band[%u]  channel[%u]}",
                         pst_channel_tmp->en_band, pst_channel_tmp->uc_chan_number);
    }

    /* 设置其它监听参数 */
    pst_scan_params->uc_max_scan_count_per_channel = 1;
    pst_scan_params->uc_channel_nums = 1;
    pst_scan_params->uc_scan_func = MAC_SCAN_FUNC_P2P_LISTEN;
    pst_scan_params->us_scan_time = (oal_uint16)pst_remain_on_channel->ul_listen_duration;
    if (pst_remain_on_channel->en_roc_type == IEEE80211_ROC_TYPE_MGMT_TX) {
        pst_scan_params->p_fn_cb = hmac_mgmt_tx_roc_comp_cb;
    } else {
        pst_scan_params->p_fn_cb = hmac_p2p_listen_comp_cb;
        pst_scan_params->uc_p2p0_listen_channel = pst_remain_on_channel->uc_listen_channel;
    }
    pst_scan_params->ull_cookie = pst_remain_on_channel->ull_cookie;
    pst_scan_params->bit_is_p2p0_scan = OAL_TRUE;

    return;
}


oal_uint32 hmac_p2p_listen_timeout(hmac_vap_stru *pst_hmac_vap_sta, oal_void *p_param)
{
    mac_device_stru *pst_mac_device;
    hmac_vap_stru *pst_hmac_vap;
    mac_vap_stru *pst_mac_vap;
    hmac_device_stru *pst_hmac_device;
    hmac_scan_record_stru *pst_scan_record;

    pst_mac_vap = (mac_vap_stru *)p_param;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{hmac_p2p_listen_timeout::mac_res_get_hmac_vap fail.vap_id[%u]!}",
                       pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_P2P, "{hmac_p2p_listen_timeout::mac_res_get_dev fail.device_id[%u]!}",
                         pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取hmac device */
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (oal_unlikely(pst_hmac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{hmac_p2p_listen_timeout::pst_hmac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_info_log2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                  "{hmac_p2p_listen_timeout::current pst_mac_vap channel is [%d] state[%d]}",
                  pst_mac_vap->st_channel.uc_chan_number,
                  pst_hmac_vap->st_vap_base_info.en_vap_state);

    oam_info_log2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                  "{hmac_p2p_listen_timeout::next pst_mac_vap channel is [%d] state[%d]}",
                  pst_mac_vap->st_channel.uc_chan_number,
                  pst_mac_device->st_p2p_info.en_last_vap_state);

    /* 由于P2P0 和P2P_CL 共用vap 结构体，监听超时，返回监听前保存的状态 */
    mac_vap_state_change(&pst_hmac_vap->st_vap_base_info, pst_mac_device->st_p2p_info.en_last_vap_state);
    hmac_set_rx_filter_value(&pst_hmac_vap->st_vap_base_info);

    pst_scan_record = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt);
    if (pst_scan_record->ull_cookie == pst_mac_device->st_p2p_info.ull_last_roc_id) {
        /* 3.1 抛事件到WAL ，上报监听结束 */
        hmac_p2p_send_listen_expired_to_host(pst_hmac_vap);
    }

    /* 3.2 抛事件到DMAC ，返回监听信道 */
    if (hmac_p2p_send_listen_expired_to_device(pst_hmac_vap) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_P2P, "{hmac_p2p_listen_timeout::hmac_p2p_send_listen_expired_to_device fail.}");
    }

    return OAL_SUCC;
}


oal_uint32 hmac_p2p_remain_on_channel(hmac_vap_stru *pst_hmac_vap_sta, oal_void *p_param)
{
    mac_device_stru *pst_mac_device;
    mac_vap_stru *pst_mac_vap;
    mac_remain_on_channel_param_stru *pst_remain_on_channel;
    mac_scan_req_stru st_scan_params;
    oal_uint32 ul_ret;

    pst_remain_on_channel = (mac_remain_on_channel_param_stru *)p_param;

    pst_mac_vap = mac_res_get_mac_vap(pst_hmac_vap_sta->st_vap_base_info.uc_vap_id);
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{hmac_p2p_remain_on_channel::mac_res_get_mac_vap fail.vap_id[%u]!}",
                       pst_hmac_vap_sta->st_vap_base_info.uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_p2p_listen_timeout::pst_mac_device[%d](%p) null!}",
                       pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    
    if (pst_hmac_vap_sta->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
        hmac_p2p_send_listen_expired_to_host(pst_hmac_vap_sta);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                         "{listen nested, send remain on channel expired to host!curr_state[%d]\r\n}",
                         pst_hmac_vap_sta->st_vap_base_info.en_vap_state);
    }

    /* 修改P2P_DEVICE 状态为监听状态 */
    mac_vap_state_change((mac_vap_stru *)&pst_hmac_vap_sta->st_vap_base_info, MAC_VAP_STATE_STA_LISTEN);
    hmac_set_rx_filter_value((mac_vap_stru *)&pst_hmac_vap_sta->st_vap_base_info);

    oam_info_log4(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                  "{get in listen state!last_state %d, channel %d, duration %d, curr_state %d}\r\n",
                  pst_mac_device->st_p2p_info.en_last_vap_state,
                  pst_remain_on_channel->uc_listen_channel,
                  pst_remain_on_channel->ul_listen_duration,
                  pst_hmac_vap_sta->st_vap_base_info.en_vap_state);

    /* 准备监听参数 */
    hmac_cfg80211_prepare_listen_req_param(&st_scan_params, (oal_int8 *)pst_remain_on_channel);

    /* 调用扫描入口，准备进行监听动作，不管监听动作执行成功或失败，都返回监听成功 */
    /* 状态机调用: hmac_scan_proc_scan_req_event */
    ul_ret = hmac_fsm_call_func_sta(pst_hmac_vap_sta, HMAC_FSM_INPUT_SCAN_REQ, (oal_void *)(&st_scan_params));
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{hmac_p2p_remain_on_channel::hmac_fsm_call_func_sta fail[%d].}", ul_ret);
    }

    return OAL_SUCC;
}


oal_uint32 hmac_config_set_p2p_miracast_status(mac_vap_stru *pst_mac_vap,
                                               oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    mac_vap_rom_stru *pst_mac_vap_rom = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap_wfd = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;

    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_config_set_p2p_miracast_status::pst_mac_vap is null");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* GO模式下,上层会使用p2p0下发命令,此时需要重新查找p2p vap */
    if (pst_mac_vap->en_p2p_mode == WLAN_P2P_DEV_MODE) {
        mac_device_find_up_p2p_go(pst_mac_device, &pst_mac_vap_wfd);
    } else if (pst_mac_vap->en_p2p_mode == WLAN_P2P_CL_MODE) {
        pst_mac_vap_wfd = pst_mac_vap;
    } else {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{dmac_config_set_p2p_miracast_status:: error! p2p_mode[%d].}", pst_mac_vap->en_p2p_mode);
    }

    if (pst_mac_vap_wfd == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_p2p_miracast_status::pst_mac_vap_wfd is null,p2p mode %d.}",
                         pst_mac_vap->en_p2p_mode);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap_rom = (mac_vap_rom_stru *)(pst_mac_vap_wfd->_rom);
    if (pst_mac_vap_rom == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap_wfd->uc_vap_id, OAM_SF_ANY,
                       "hmac_config_set_p2p_miracast_status::pst_mac_vap_rom is null");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap_rom->en_wfd_status = *((oal_bool_enum_uint8 *)puc_param);
    pst_mac_vap_rom->en_aggr_limit_on = *((oal_bool_enum_uint8 *)puc_param);
    pst_mac_vap_rom->us_tx_pkts = 0;

    ul_ret = hmac_config_wfd_aggr_limit_syn(pst_mac_vap_wfd,
                                            pst_mac_vap_rom->en_wfd_status, pst_mac_vap_rom->en_aggr_limit_on);
    return ul_ret;
}


oal_uint32 hmac_config_set_p2p_ps_ops(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    mac_cfg_p2p_ops_param_stru *pst_p2p_ops;
    pst_p2p_ops = (mac_cfg_p2p_ops_param_stru *)puc_param;
    oam_info_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_p2p_ps_ops:ctrl:%d, ct_window:%d\r\n}",
                  pst_p2p_ops->en_ops_ctrl,
                  pst_p2p_ops->uc_ct_window);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_P2P_PS_OPS, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_p2p_ps_ops::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_config_set_p2p_ps_noa(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint32 ul_ret;
    mac_cfg_p2p_noa_param_stru *pst_p2p_noa;
    pst_p2p_noa = (mac_cfg_p2p_noa_param_stru *)puc_param;
    oam_info_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                  "{hmac_config_set_p2p_ps_noa:start_time:%d, duration:%d, interval:%d, count:%d\r\n}",
                  pst_p2p_noa->ul_start_time,
                  pst_p2p_noa->ul_duration,
                  pst_p2p_noa->ul_interval,
                  pst_p2p_noa->uc_count);
    pst_p2p_noa->ul_start_time *= 1000; /* 1ms to 1000us */
    pst_p2p_noa->ul_duration *= 1000; /* 1ms to 1000us */
    pst_p2p_noa->ul_interval *= 1000; /* 1ms to 1000us */

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_P2P_PS_NOA, us_len, puc_param);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_p2p_ps_noa::hmac_config_send_event failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_uint32 hmac_find_p2p_listen_channel(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    oal_uint8 *puc_p2p_ie = OAL_PTR_NULL;
    oal_uint8 *puc_listen_channel_ie = OAL_PTR_NULL;

    /* 查找P2P IE信息 */
    puc_p2p_ie = mac_find_vendor_ie(MAC_WLAN_OUI_WFA, MAC_WLAN_OUI_TYPE_WFA_P2P, puc_param, (oal_int32)us_len);
    if (puc_p2p_ie == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_find_p2p_listen_channel::p2p ie is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 长度校验 */
    if (puc_p2p_ie[1] < MAC_P2P_MIN_IE_LEN) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_find_p2p_listen_channel::invalid p2p ie len[%d].}",
                         puc_p2p_ie[1]);
        return OAL_FAIL;
    }

    /* 查找P2P Listen channel信息 */
    /* puc_p2p_ie跳过6byte（MAC_WLAN_OUI_WFA(3byte)、MAC_EID_VENDOR(1byte)、lenth(1byte)、id(1byte)），p2p ie第1byte减4 */
    puc_listen_channel_ie = mac_find_p2p_attribute(MAC_P2P_ATTRIBUTE_LISTEN_CHAN, puc_p2p_ie + 6, (puc_p2p_ie[1] - 4));
    if (puc_listen_channel_ie == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_find_p2p_listen_channel:listen channel ie null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* listen channel长度校验，大端（判断listen channel长度非等于puc_listen_channel_ie第2byte左移8位与其第1byte之和） */
    if (MAC_P2P_LISTEN_CHN_ATTR_LEN != (oal_int32)((puc_listen_channel_ie[2] << 8) + puc_listen_channel_ie[1])) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_find_p2p_listen_channel::invalid p2p listen channel ie len[%d].}",
                         /* puc_listen_channel_ie[2]的高8位与puc_listen_channel_ie[1]作为参数输出打印 */
                         (oal_int32)((puc_listen_channel_ie[2] << 8) + puc_listen_channel_ie[1]));
        return OAL_FAIL;
    }

    /* 获取P2P Listen channel信息(puc_listen_channel_ie第7byte) */
    pst_mac_vap->uc_p2p_listen_channel = puc_listen_channel_ie[7];
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_find_p2p_listen_channel::END CHANNEL[%d].}",
                  pst_mac_vap->uc_p2p_listen_channel);

    return OAL_SUCC;
}

oal_module_symbol(hmac_config_set_p2p_miracast_status);
oal_module_symbol(hmac_config_set_p2p_ps_ops);
oal_module_symbol(hmac_config_set_p2p_ps_noa);
/*lint -e578*//*lint -e19*/
oal_module_symbol(hmac_get_p2p_status);
oal_module_symbol(hmac_set_p2p_status);
oal_module_symbol(hmac_clr_p2p_status);
/*lint +e578*//*lint +e19*/
#endif /* _PRE_WLAN_FEATURE_P2P */


