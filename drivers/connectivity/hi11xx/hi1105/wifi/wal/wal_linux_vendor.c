

#include "wal_linux_vendor.h"
#include "oal_ext_if.h"

#include "oam_ext_if.h"
#include "frw_ext_if.h"

#include "hmac_tx_data.h"
#include "mac_ftm.h"
#include "hmac_dfs.h"
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
#ifndef CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT
#include "frw_task.h"
#endif
#endif

#include "wlan_spec.h"
#include "wlan_types.h"
#include "wlan_chip_i.h"
#include "mac_vap.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#include "mac_ie.h"
#include "mac_device.h"

#include "hmac_ext_if.h"
#include "hmac_chan_mgmt.h"
#include "hmac_tx_data.h"
#include "hmac_config.h"
#include "wal_main.h"
#include "wal_ext_if.h"
#include "wal_config.h"
#include "wal_regdb.h"
#include "wal_linux_scan.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_bridge.h"
#include "wal_linux_flowctl.h"
#include "wal_linux_atcmdsrv.h"
#include "wal_linux_event.h"
#include "hmac_resource.h"
#include "hmac_p2p.h"

#include "wal_linux_cfg80211.h"
#include "wal_linux_cfgvendor.h"
#include "wal_dfx.h"
#include "wal_cfg_ioctl.h"
#include "oal_hcc_host_if.h"
#include "plat_cali.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/notifier.h>
#include <linux/inetdevice.h>
#include <net/addrconf.h>
#endif
#include "hmac_arp_offload.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 59)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "../fs/proc/internal.h"
#endif

#include "hmac_auto_adjust_freq.h"
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
#include "hmac_tx_amsdu.h"
#endif

#include "hmac_roam_main.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "oal_net.h"
#include "hisi_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#include "plat_firmware.h"
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif

#ifdef _PRE_WLAN_FEATURE_11AX
#include "hmac_wifi6_self_cure.h"
#endif
#ifdef _PRE_WLAN_FEATURE_HIEX
#include "hmac_hiex.h"
#endif

#include "hmac_dfx.h"
#include "hmac_hid2d.h"
#include "hmac_monitor.h"
#include "hmac_nrcoex.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_VENDOR_C

#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
OAL_STATIC uint32_t wal_ioctl_set_rx_listen_ps_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_command);
#endif
OAL_STATIC int32_t wal_ioctl_set_mlme_ie(oal_net_device_stru *pst_net_dev, oal_mlme_ie_stru *pst_mlme_ie);
OAL_STATIC int32_t wal_ioctl_set_dc_status(oal_net_device_stru *pst_net_dev, int32_t dc_param);

OAL_STATIC int32_t wal_ioctl_set_p2p_miracast_status(oal_net_device_stru *pst_net_dev, uint8_t uc_param);
OAL_STATIC int32_t wal_ioctl_set_p2p_noa(oal_net_device_stru *net_dev, mac_cfg_p2p_noa_param_stru *p2p_noa_param);
OAL_STATIC int32_t wal_ioctl_set_p2p_ops(oal_net_device_stru *net_dev, mac_cfg_p2p_ops_param_stru *p2p_ops_param);
OAL_STATIC int32_t wal_ioctl_set_vowifi_param(oal_net_device_stru *pst_net_dev,
    int8_t *puc_command, wal_wifi_priv_cmd_stru *pst_priv_cmd);
OAL_STATIC int32_t wal_ioctl_get_vowifi_param(oal_net_device_stru *net_dev, int8_t *command, int32_t *value);
OAL_STATIC int32_t wal_ioctl_priv_cmd_set_ap_wps_p2p_ie(oal_net_device_stru *pst_net_dev,
    int8_t *pc_command, wal_wifi_priv_cmd_stru st_priv_cmd);
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
OAL_STATIC int32_t wal_ioctl_tas_pow_ctrl(oal_net_device_stru *pst_net_dev, uint8_t uc_coreindex,
                                          oal_bool_enum_uint8 en_need_improve);
OAL_STATIC int32_t wal_ioctl_tas_rssi_access(oal_net_device_stru *pst_net_dev, uint8_t uc_coreindex);
#endif
#ifndef CONFIG_HAS_EARLYSUSPEND
OAL_STATIC int32_t wal_ioctl_set_suspend_mode(oal_net_device_stru *pst_net_dev, uint8_t uc_suspend);
#endif
OAL_STATIC uint32_t wal_ioctl_set_fastsleep_switch(oal_net_device_stru *pst_net_dev, int8_t *puc_para_str);
OAL_STATIC uint32_t wal_ioctl_tcp_ack_buf_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_command);
OAL_STATIC int32_t wal_ioctl_get_wifi_priv_feature_cap_param(oal_net_device_stru *pst_net_dev,
    int8_t *puc_command, int32_t *pl_value);
OAL_STATIC uint32_t wal_ioctl_force_stop_filter(oal_net_device_stru *pst_net_dev, uint8_t uc_param);
OAL_STATIC int32_t wal_ioctl_reduce_sar(oal_net_device_stru *pst_net_dev, uint16_t tx_power);
int32_t wal_vendor_priv_cmd_ext(oal_net_device_stru *pst_net_dev,
    oal_ifreq_stru *pst_ifr, wal_wifi_priv_cmd_stru *p_priv_cmd, uint8_t *pc_cmd);

OAL_STATIC const wal_ioctl_priv_cmd_stru g_ast_vowifi_cmd_table[VOWIFI_CMD_BUTT] = {
    {CMD_VOWIFI_SET_MODE,                   VOWIFI_SET_MODE},
    {CMD_VOWIFI_GET_MODE,                   VOWIFI_GET_MODE},
    {CMD_VOWIFI_SET_PERIOD,                 VOWIFI_SET_PERIOD},
    {CMD_VOWIFI_GET_PERIOD,                 VOWIFI_GET_PERIOD},
    {CMD_VOWIFI_SET_LOW_THRESHOLD,          VOWIFI_SET_LOW_THRESHOLD},
    {CMD_VOWIFI_GET_LOW_THRESHOLD,          VOWIFI_GET_LOW_THRESHOLD},
    {CMD_VOWIFI_SET_HIGH_THRESHOLD,         VOWIFI_SET_HIGH_THRESHOLD},
    {CMD_VOWIFI_GET_HIGH_THRESHOLD,         VOWIFI_GET_HIGH_THRESHOLD},
    {CMD_VOWIFI_SET_TRIGGER_COUNT,          VOWIFI_SET_TRIGGER_COUNT},
    {CMD_VOWIFI_GET_TRIGGER_COUNT,          VOWIFI_GET_TRIGGER_COUNT},
    {CMD_VOWIFI_IS_SUPPORT,                 VOWIFI_SET_IS_SUPPORT},
};

OAL_STATIC uint32_t wal_vendor_set_p2p_scenes(oal_net_device_stru *net_dev, uint8_t *p_cmd, uint32_t cmd_len)
{
    int32_t ret;
    uint8_t *p_cmd_data;
    uint32_t cmd_id;
    uint32_t params[CMD_PARAMS_MAX_CNT] = {0};
    uint32_t offset = 0;

    p_cmd_data = p_cmd + CMD_SET_P2P_SCENE_LEN + 1;
    params[0]  = oal_atoi(p_cmd_data);

    ret = wal_get_cmd_id("p2p_scenes", &cmd_id, &offset);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_vendor_set_p2p_scenes:find p2p_scenes cmd is fail");
        return ret;
    }
    return wal_process_cmd_params(net_dev, cmd_id, params);
}

OAL_STATIC uint32_t wal_vendor_set_change_go_channel(oal_net_device_stru *net_dev,
    oal_ifreq_stru *ifr, uint8_t *cmd, uint32_t cmd_len)
{
    uint8_t *cmd_data;
    uint32_t offset;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    wal_msg_write_stru writeMsg;
    wal_msg_stru *rsp_msg = NULL;
    uint8_t val = 0;
    int32_t ret;

    WAL_WRITE_MSG_HDR_INIT(&writeMsg, WALN_CFGID_SET_CHANGE_GO_CHANNEL, 2 * sizeof(uint8_t)); // 参数长度2个字节

    cmd_data = cmd + CMD_SET_CHANGE_GO_CHANNEL_LEN + 1;
    ret = wal_get_cmd_one_arg(cmd_data, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_vendor_set_change_go_channel::WALN_CFGID_SET_CHANGE_GO_CHANNEL error!}");
        return OAL_FAIL;
    }

    writeMsg.auc_value[0] = (uint8_t)oal_atoi(ac_name); // 第一个参数：新信道号
    cmd_data += offset;
    ret = wal_get_cmd_one_arg(cmd_data, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret == OAL_SUCC) {
        writeMsg.auc_value[1] = (uint8_t)oal_atoi(ac_name); // 第二个参数：新带宽宽度
    } else if (ret == OAL_ERR_CODE_CONFIG_ARGS_OVER) {
        writeMsg.auc_value[1] = WLAN_BW_CAP_BUTT; // 没填带宽默认WLAN_BW_CAP_BUTT
    } else {
        return OAL_FAIL;
    }

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + 2 * sizeof(uint8_t), (uint8_t *)&writeMsg, OAL_TRUE, &rsp_msg); // 参数长度2个字节
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_vendor_set_change_go_channel::returnerr code = [%d].}", ret);
        return (uint32_t)ret;
    }

    if (wal_check_and_release_msg_resp(rsp_msg) != OAL_SUCC) {
        val = 1;
    }

    if (oal_copy_to_user(ifr->ifr_data + WAL_IFR_DATA_OFFSET, &val, sizeof(uint8_t))) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_vendor_set_change_go_channel::Failed to copy ioctl_data to user !}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


int32_t wal_vendor_get_radar_result(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr, int8_t *pc_cmd)
{
#ifdef _PRE_WLAN_FEATURE_DFS
    hmac_dfs_radar_result_stru *pst_radar_result;

    pst_radar_result = hmac_dfs_result_info_get();
    if (pst_radar_result == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "wal_vendor_get_radar_result::pst_radar_result is NULL!");
        return -OAL_FAIL;
    }

    oal_io_print("wal_vendor_get_radar_result,dev_name is:%s, en_dfs_status=%d\n",
        pst_radar_result->auc_name, pst_radar_result->en_dfs_status);
    oam_warning_log3(0, OAM_SF_CFG, "wal_vendor_get_radar_result::dfs_status=[%d], freq=[%d], bw=%d",
        pst_radar_result->en_dfs_status, pst_radar_result->us_freq, pst_radar_result->uc_bw);

    /* 8 为偏移头字节 */
    if (oal_copy_to_user(pst_ifr->ifr_data + 8, (void *)pst_radar_result, sizeof(hmac_dfs_radar_result_stru))) {
        oam_error_log0(0, OAM_SF_ANY, "wal_vendor_get_radar_result: Failed to copy ioctl_data to user !");
        return -OAL_EFAIL;
    }
#endif

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING

OAL_STATIC uint32_t wal_ioctl_set_rx_listen_ps_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_command)
{
    int8_t *puc_para_str = NULL;
    uint8_t uc_rx_listen_ps_switch;
    uint8_t *puc_value;
    int32_t l_ret;
    wal_msg_write_stru st_write_msg;

    if (OAL_STRLEN(pc_command) < CMD_SET_RX_LISTEN_PS_SWITCH_LEN + 1) {
        oam_warning_log1(0, OAM_SF_CFG, "wal_ioctl_set_rx_listen_ps_switch:cmd fail!", OAL_STRLEN(pc_command));
        return OAL_FAIL;
    }
    puc_para_str = pc_command + CMD_SET_RX_LISTEN_PS_SWITCH_LEN;
    uc_rx_listen_ps_switch = (uint8_t)oal_atoi(puc_para_str);

    /* 0表示处于游戏场景/其他需要退出ps的场景，软件需要拋事件退出ps work态，1表示非游戏场景 */
    if (!oal_strncasecmp(puc_para_str, "0", 1)) {
        oam_warning_log0(0, OAM_SF_CFG, "wal_ioctl_set_rx_listen_ps_switch:game on, if ps work, need to exit!");
    } else if (!oal_strncasecmp(puc_para_str, "1", 1)) {
        oam_warning_log0(0, OAM_SF_CFG, "wal_ioctl_set_rx_listen_ps_switch:game off!");
    } else {
        oam_warning_log0(0, OAM_SF_CFG, "wal_ioctl_set_rx_listen_ps_switch:invalid cmd str!");
        return OAL_FAIL;
    }

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_RX_LISTEN_PS_SWITCH, sizeof(uint8_t));
    /* 设置配置命令参数 */
    puc_value = (uint8_t *)st_write_msg.auc_value;
    *puc_value = uc_rx_listen_ps_switch;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_PWR, "{wal_ioctl_set_rx_listen_ps_switch::return err code [%d]!}", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}
#endif


int32_t wal_vendor_priv_cmd_power_saving(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr, uint8_t *pc_cmd)
{
    if (oal_strncasecmp(pc_cmd, CMD_GET_FAST_SLEEP_CNT, CMD_GET_FAST_SLEEP_CNT_LEN) == 0) {
        return wal_ioctl_get_psm_stat(pst_net_dev, MAC_PSM_QUERY_FASTSLEEP_STAT, pst_ifr);
    }
#ifdef _PRE_WLAN_FEATURE_PSM_DFX_EXT
    if (g_wlan_spec_cfg->feature_psm_dfx_ext_is_open) {
        if (oal_strncasecmp(pc_cmd, CMD_GET_BEACON_CNT, CMD_GET_BEACON_CNT_LEN) == 0) {
            return wal_ioctl_get_psm_stat(pst_net_dev, MAC_PSM_QUERY_BEACON_CNT, pst_ifr);
        }
    }
#endif

#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
    if (g_wlan_spec_cfg->rx_listen_ps_is_open) {
        /* (1)终端获取rx listen power saving状态，以及当前状态与进入该状态的时间间隔 (2)命令配置rx listen ps开关 */
        if (oal_strncasecmp(pc_cmd, CMD_GET_RX_LISTEN_STATE, OAL_STRLEN(CMD_GET_RX_LISTEN_STATE)) == 0) {
            return wal_ioctl_get_psm_stat(pst_net_dev, MAC_PSM_QUERY_RX_LISTEN_STATE, pst_ifr);
        }
        if (oal_strncasecmp(pc_cmd, CMD_SET_RX_LISTEN_PS_SWITCH, OAL_STRLEN(CMD_SET_RX_LISTEN_PS_SWITCH)) == 0) {
            return wal_ioctl_set_rx_listen_ps_switch(pst_net_dev, pc_cmd);
        }
    }
#endif

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11AX
uint32_t wal_vendor_11ax_close_htc_handle(oal_net_device_stru *p_net_dev)
{
    uint8_t tmpSwitch = 0;
    int32_t ret;
    wal_msg_write_stru writeMsg;

    WAL_WRITE_MSG_HDR_INIT(&writeMsg, WLAN_CFGID_SET_11AX_CLOSE_HTC, sizeof(tmpSwitch));
    writeMsg.auc_value[0] = (uint8_t)(tmpSwitch ? OAL_TRUE : OAL_FALSE);

    ret = wal_send_cfg_event(p_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(tmpSwitch), (uint8_t *)&writeMsg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_11AX, "{wal_vendor_11ax_close_htc_handle::return err code = [%d].}\r\n", ret);
    }

    return (uint32_t)ret;
}

uint32_t wal_vendor_11ax_wifi6_balcklist_handle(oal_net_device_stru *p_net_dev, uint8_t *pc_cmd, uint32_t cmd_len)
{
    int32_t  ret;
    uint8_t  blacklistCnt;
    wal_msg_write_stru writeMsg;
    hmac_wifi6_self_cure_info_stru *p_wifi6_self_cure_info = NULL;
    uint8_t *p_cmd_data = NULL;

    p_cmd_data      = pc_cmd + CMD_SET_AX_BLACKLIST_LEN + 1;
    blacklistCnt = (uint8_t)*p_cmd_data;
    p_cmd_data += 1;

    if (blacklistCnt > HMAC_WIFI6_SELF_CURE_BLACK_LIST_MAX_NUM) {
        oam_warning_log2(0, OAM_SF_M2S, "{wal_vendor_11ax_wifi6_balcklist_handle::uc_blacklist_cnt=[%d],\
            max_num=[%d].}\r\n", blacklistCnt, HMAC_WIFI6_SELF_CURE_BLACK_LIST_MAX_NUM);
        return OAL_FAIL;
    }

    WAL_WRITE_MSG_HDR_INIT(&writeMsg, WLAN_CFGID_SET_11AX_WIFI6_BLACKLIST,
        sizeof(hmac_wifi6_self_cure_info_stru));
    p_wifi6_self_cure_info = (hmac_wifi6_self_cure_info_stru *)writeMsg.auc_value;
    p_wifi6_self_cure_info->blaclist_num = blacklistCnt;
    memmove_s (p_wifi6_self_cure_info->wifi6_blacklist, blacklistCnt * sizeof(hmac_wifi6_self_cure_blacklist_stru),
        p_cmd_data, blacklistCnt * sizeof(hmac_wifi6_self_cure_blacklist_stru));

    ret = wal_send_cfg_event(p_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(hmac_wifi6_self_cure_info_stru),
        (uint8_t *)&writeMsg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_11AX, "{wal_vendor_11ax_wifi6_balcklist_handle::returnerr code = [%d].}\r\n", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}
#endif

uint32_t wal_vendor_clear_11n_blacklist_handle(oal_net_device_stru *p_net_dev)
{
    int32_t ret;
    wal_msg_write_stru writeMsg;

    WAL_WRITE_MSG_HDR_INIT(&writeMsg, WLAN_CFGID_SET_CLEAR_11N_BLACKLIST, sizeof(uint8_t));

    ret = wal_send_cfg_event(p_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t), (uint8_t *)&writeMsg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_vendor_clear_11n_blacklist_handle::return err code = [%d].}\r\n", ret);
    }

    return (uint32_t)ret;
}

#ifdef _PRE_WLAN_FEATURE_CSI
/*
 * 命令格式:
 * CMD_QUERY_CSI TA MAC(6b) CSI enable(1b) sample_bw(1b) frame_type(1b) sample_period(1b) phy_report_mode(1b)
 */
OAL_STATIC uint32_t wal_vendor_csi_handle(oal_net_device_stru *p_net_dev, uint8_t *pc_cmd, uint32_t cmd_len)
{
    uint8_t *csi_cmd_str = pc_cmd + CMD_QUERY_CSI_LEN + 1;
    return wal_ioctl_set_csi_switch(p_net_dev, csi_cmd_str);
}
#endif


int32_t wal_vendor_get_ap_bandwidth(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr)
{
    mac_vap_stru *pst_mac_vap;
    uint8_t uc_current_bandwidth;

    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if ((pst_mac_vap == NULL) || (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP)) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_vendor_get_ap_bandwidth::vap not find or is not ap mode");
        return -OAL_EFAIL;
    }

    if (WLAN_BAND_WIDTH_20M == pst_mac_vap->st_channel.en_bandwidth) {
        uc_current_bandwidth = WLAN_AP_BANDWIDTH_20M;
    } else if (WLAN_BAND_WIDTH_40MINUS >= pst_mac_vap->st_channel.en_bandwidth) {
        uc_current_bandwidth = WLAN_AP_BANDWIDTH_40M;
    } else if (WLAN_BAND_WIDTH_80MINUSMINUS >= pst_mac_vap->st_channel.en_bandwidth) {
        uc_current_bandwidth = WLAN_AP_BANDWIDTH_80M;
    } else {
#ifdef _PRE_WLAN_FEATURE_160M
        if (WLAN_BAND_WIDTH_160MINUSMINUSMINUS >= pst_mac_vap->st_channel.en_bandwidth) {
            uc_current_bandwidth = WLAN_AP_BANDWIDTH_160M;
        } else {
            return -OAL_EFAIL;
        }
#else
        return -OAL_EFAIL;
#endif
    }

    oal_copy_to_user(pst_ifr->ifr_data + BYTE_OFFSET_8, &uc_current_bandwidth, sizeof(uint8_t));

    return OAL_SUCC;
}


int32_t wal_vendor_get_wifi6_supported(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr)
{
    int32_t l_priv_value;
    uint8_t uc_val;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    l_priv_value = wlan_chip_get_11ax_switch_mask();
#else
    l_priv_value = 0;
#endif
    /* 判断STA 11ax定制化是否打开 */
    if ((uint32_t)l_priv_value & BIT0) {
        uc_val = PROTOCOL_WIFI6_SUPPORT;
    } else {
        uc_val = PROTOCOL_DEFAULT;
    }

    if (oal_copy_to_user(pst_ifr->ifr_data + WAL_IFR_DATA_OFFSET, &uc_val, sizeof(uint8_t))) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_vendor_get_wifi6_supported::Failed to copy ioctl_data to user !}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


int32_t wal_vendor_get_160m_supported(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr)
{
    oal_bool_enum_uint8 aput_support_160m = OAL_FALSE;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    aput_support_160m = wlan_chip_is_aput_support_160m();
#endif
    oal_copy_to_user(pst_ifr->ifr_data + BYTE_OFFSET_8, &aput_support_160m, sizeof(oal_bool_enum_uint8));

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_vendor_fem_lowpower(oal_net_device_stru *pst_net_dev, int8_t *pc_fem_cmd, uint8_t uc_cmd_len)
{
    if (uc_cmd_len > WAL_HIPRIV_CMD_NAME_MAX_LEN) {
        return OAL_FAIL;
    }

    return wal_hipriv_fem_lowpower(pst_net_dev, pc_fem_cmd);
}

OAL_STATIC int32_t wal_ioctl_judge_input_param_length(wal_wifi_priv_cmd_stru st_priv_cmd,
    uint32_t cmd_length, uint16_t us_adjust_val)
{
    /* 其中+1为 字符串命令与后续参数中间的空格字符 */
    if (st_priv_cmd.total_len < cmd_length + 1 + us_adjust_val) {
        /* 入参长度不满足要求, 申请的内存pc_command统一在主函数wal_vendor_priv_cmd中释放 */
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}


OAL_STATIC int32_t wal_ioctl_priv_cmd_country(oal_net_device_stru *pst_net_dev,
    int8_t *pc_command, wal_wifi_priv_cmd_stru st_priv_cmd)
{
#ifdef _PRE_WLAN_FEATURE_11D
    const int8_t *country_code = NULL;
    int8_t auc_country_code[NUM_3_BYTES] = { 0 };
    int32_t l_ret;

    /* 格式:COUNTRY CN */
    l_ret = wal_ioctl_judge_input_param_length(st_priv_cmd, CMD_COUNTRY_LEN, 2); /* 2表示后续参数长度 */
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, 0, "{wal_ioctl_priv_cmd_country::len gt[%d]}", CMD_COUNTRY_LEN + 3); /* 3 cmdpara */
        return -OAL_EFAIL;
    }

    country_code = pc_command + CMD_COUNTRY_LEN + 1;
    if (EOK != memcpy_s(auc_country_code, sizeof(auc_country_code), country_code, NUM_2_BYTES)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_priv_cmd_country::memcpy fail!");
        return -OAL_EFAIL;
    }

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if (g_cust_country_code_ignore_flag.en_country_code_ingore_ini_flag == OAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_priv_cmd_country::wlan_pm_set_country is ignore, ini[%d]",
                         g_cust_country_code_ignore_flag.en_country_code_ingore_ini_flag);

        return OAL_SUCC;
    }
#endif
    l_ret = wal_regdomain_update_for_dfs(pst_net_dev, auc_country_code);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_priv_cmd_country::err [%d]!}", l_ret);

        return -OAL_EFAIL;
    }

    l_ret = wal_regdomain_update(pst_net_dev, auc_country_code);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_priv_cmd_country::err [%d]!}", l_ret);

        return -OAL_EFAIL;
    }
#else
    oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_priv_cmd_country::_PRE_WLAN_FEATURE_11D is not define!}");
#endif

    return OAL_SUCC;
}

OAL_STATIC int32_t wal_netdev_set_power_on(oal_net_device_stru *pst_net_dev, int32_t power_flag)
{
    int32_t l_ret = 0;

    oam_warning_log1(0, OAM_SF_ANY, "{wal_netdev_set_power_on::CMD_SET_POWER_ON cmd,power flag:%d}", power_flag);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    // ap上下电，配置VAP
    if (power_flag == 0) { // 下电
        /* 下电host device_stru去初始化 */
        wal_host_dev_exit(pst_net_dev);

        wal_wake_lock();
        wlan_pm_close();
        wal_wake_unlock();

        g_st_ap_config_info.l_ap_power_flag = OAL_FALSE;
    } else if (power_flag == 1) {  // 上电
        g_st_ap_config_info.l_ap_power_flag = OAL_TRUE;

        wal_wake_lock();
        l_ret = wlan_pm_open();
        wal_wake_unlock();
        if (l_ret == OAL_FAIL) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_netdev_set_power_on::wlan_pm_open Fail!}");
            return -OAL_EFAIL;
        } else if (l_ret != OAL_ERR_CODE_ALREADY_OPEN) {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
            /* 重新上电时置为FALSE */
            hwifi_config_init_force();
#endif
            // 重新上电场景，下发配置VAP
            l_ret = wal_cfg_vap_h2d_event(pst_net_dev);
            if (l_ret != OAL_SUCC) {
                return -OAL_EFAIL;
            }

            /* 上电host device_stru初始化 */
            l_ret = wal_host_dev_init(pst_net_dev);
            if (l_ret != OAL_SUCC) {
                oal_io_print("wal_host_dev_init FAIL %d ", l_ret);
                return -OAL_EFAIL;
            }
        }
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_netdev_set_power_on::pupower_flag:%d error.}", power_flag);
        return -OAL_EFAIL;
    }
#endif

    return OAL_SUCC;
}

OAL_STATIC int32_t wal_ioctl_set_sta_pm_on(oal_net_device_stru *pst_net_dev,
    uint8_t *pc_command, wal_wifi_priv_cmd_stru st_priv_cmd)
{
    wal_msg_write_stru st_write_msg;
    mac_cfg_ps_open_stru *pst_sta_pm_open = NULL;
    int32_t l_ret;
    mac_vap_stru *mac_vap = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    l_ret = wal_ioctl_judge_input_param_length(st_priv_cmd, CMD_SET_STA_PM_ON_LEN, 1);
    if (l_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_sta_pm_on::CMD_SET_STA_PM_ON puc_command len error.}");
        return -OAL_EFAIL;
    }

    mac_vap = oal_net_dev_priv(pst_net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_sta_pm_on::get mac vap failed!}");
        return -OAL_EFAIL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_ioctl_set_sta_pm_on::get hmac vap[%d] failed.}", mac_vap->uc_vap_id);
        return -OAL_EFAIL;
    }
    /* 未获取到ip地址，不允许通过私有命令配置低功耗开关 */
    if (!hmac_vap->ipaddr_obtained) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_sta_pm_on::ip addr not obtained, cannot set pm switch.}");
        return OAL_SUCC;
    }

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_STA_PM_ON, sizeof(mac_cfg_ps_open_stru));

    /* 设置配置命令参数 */
    pst_sta_pm_open = (mac_cfg_ps_open_stru *)(st_write_msg.auc_value);
    /* MAC_STA_PM_SWITCH_ON / MAC_STA_PM_SWITCH_OFF */
    pst_sta_pm_open->uc_pm_enable = *(pc_command + CMD_SET_STA_PM_ON_LEN + 1) - '0';
    pst_sta_pm_open->uc_pm_ctrl_type = MAC_STA_PM_CTRL_TYPE_HOST;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,WAL_MSG_WRITE_MSG_HDR_LENGTH + \
        sizeof(mac_cfg_ps_open_stru), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_set_sta_pm_on::CMD_SET_STA_PM_ON err [%d]!}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t wal_ioctl_set_dynamic_dbac_mode(oal_net_device_stru *p_net_dev, uint8_t *p_command)
{
    wal_msg_write_stru write_msg;
    int32_t ret;
    uint32_t offset;
    int8_t arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    uint16_t config_len;
    uint16_t param_len;
    mac_ioctl_alg_config_stru alg_config;

    uint8_t p_command_new[15] = "dbac percent ";  // 15为拼接后的字符串所占空间
    uint8_t *p_tmp = (uint8_t *)p_command_new;
    uint8_t *p_data = (p_command + CMD_SET_DYNAMIC_DBAC_MODE_LEN + 1);
    uint8_t value = oal_atoi(p_data);
    if (value > 3) {   // mode最大为3
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_set_dynamic_dbac_mode::command param error!}");
        return OAL_FAIL;
    }
    p_command_new[13] = *p_data; // 13是倒数第二位为数字[0|1|2|3]
    p_command_new[14] = '\0';   // 第14位是字符串的结束符

    alg_config.uc_argc = 0;
    while (OAL_SUCC == wal_get_cmd_one_arg(p_tmp, arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset)) {
        alg_config.auc_argv_offset[alg_config.uc_argc] = (uint8_t)((uint8_t)(p_tmp - p_command_new) +
            (uint8_t)offset - (uint8_t)OAL_STRLEN(arg));
        p_tmp += offset;
        alg_config.uc_argc++;
    }

    param_len = (uint8_t)OAL_STRLEN(p_command_new);
    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    config_len = sizeof(mac_ioctl_alg_config_stru) + param_len + 1;
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_ALG, config_len);
    ret = memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value), &alg_config, sizeof(mac_ioctl_alg_config_stru));
    ret += memcpy_s(write_msg.auc_value + sizeof(mac_ioctl_alg_config_stru),
        sizeof(write_msg.auc_value) - sizeof(mac_ioctl_alg_config_stru), p_command_new, param_len + 1);
    if (ret != EOK) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_ioctl_set_dynamic_dbac_mode::memcpy fail! [%d]}", ret);
        return OAL_FAIL;
    }

    ret = wal_send_cfg_event(p_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + config_len,
                             (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_ioctl_set_dynamic_dbac_mode::wal_send_cfg_event return err code [%d]!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_set_dfs_mode_scenes(oal_net_device_stru *net_dev, uint8_t *cmd, uint32_t cmd_len)
{
    int32_t ret;
    uint8_t *temp = NULL;
    uint8_t *cmd_data = NULL;
    uint8_t acname[16] = "dfs_debug ";      /* 16: size of array to be sufficient for string storage */
    cmd_data = cmd + CMD_SET_GO_DETECT_RADAR_LEN + 1;
    if ((uint8_t)*cmd_data != '0' && (uint8_t)*cmd_data != '1') {
        oam_warning_log0(0, OAM_SF_M2S, "{wal_hipriv_set_dfs_mode_scenes::input parameter is not 0 or 1.}");
        return  OAL_FAIL;
    }
    acname[strlen(acname)] = (uint8_t)*cmd_data;
    temp = acname;
    ret = wal_hipriv_set_val_cmd(net_dev, temp);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_hipriv_set_dfs_mode_scenes::return err code = [%d].}", ret);
        return ret;
    }
    return OAL_SUCC;
}


int32_t wal_hid2d_sleep_mode(oal_net_device_stru *net_dev, uint8_t ctrl, wal_wifi_priv_cmd_stru *priv_cmd)
{
    char *cmd = CMD_SET_STA_PM_ON;
    char cmd_buf[BUFF_SIZE] = { 0 };

    oam_warning_log1(0, OAM_SF_ANY, "wal_hid2d_sleep_mode::ctrl = %d", ctrl);
    if (snprintf_s(cmd_buf, BUFF_SIZE, BUFF_SIZE - 1, "%s %d", cmd, ctrl) > 0) {
        return wal_ioctl_set_sta_pm_on(net_dev, cmd_buf, *priv_cmd);
    }
    return -OAL_EFAUL;
}


int32_t wal_hid2d_napi_mode(oal_net_device_stru *pst_net_dev, uint8_t uc_ctrl)
{
    oal_netdev_priv_stru *pst_netdev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(pst_net_dev);

    if (pst_netdev_priv == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hid2d_napi_mode::pst_netdev_priv is null!}");
        return -OAL_EFAUL;
    }

    oam_warning_log1(0, OAM_SF_ANY, "wal_hid2d_napi_mode::ctrl = %d", uc_ctrl);
    pst_netdev_priv->uc_napi_enable = uc_ctrl;
    pst_netdev_priv->uc_gro_enable = uc_ctrl;
    return OAL_SUCC;
}


int32_t wal_hid2d_gso_mode(oal_net_device_stru *pst_net_dev, uint8_t uc_ctrl)
{
#ifdef _PRE_WLAN_FEATURE_GSO
    oam_warning_log1(0, OAM_SF_ANY, "wal_hid2d_gso_mode::uc_ctrl = %d", uc_ctrl);
    if (uc_ctrl == DISABLE) {
        pst_net_dev->features &= ~NETIF_F_SG;
        pst_net_dev->hw_features &= ~NETIF_F_SG;
    } else if (uc_ctrl == ENABLE) {
        pst_net_dev->features |= NETIF_F_SG;
        pst_net_dev->hw_features |= NETIF_F_SG;
    }
#endif
    return OAL_SUCC;
}


int32_t wal_hid2d_freq_boost(oal_net_device_stru *pst_net_dev, uint8_t uc_ctrl)
{
    uint32_t l_ret = 0;
    char freq_cmd1[] = {"auto_freq 0 0"};
    char freq_cmd2[] = {"auto_freq 1 30"};
    char freq_cmd3[] = {"auto_freq 0 1"};

    oam_warning_log1(0, OAM_SF_ANY, "wal_hid2d_freq_boost::uc_ctrl = %d", uc_ctrl);
    if (uc_ctrl == ENABLE) {
        l_ret = wal_hipriv_set_tlv_cmd(pst_net_dev, (uint8_t*) freq_cmd1);
        l_ret = wal_hipriv_set_tlv_cmd(pst_net_dev, (uint8_t*) freq_cmd2);
    } else if (uc_ctrl == DISABLE) {
        l_ret = wal_hipriv_set_tlv_cmd(pst_net_dev, (uint8_t*) freq_cmd3);
    }
    return l_ret;
}

#ifdef _PRE_WLAN_FEATURE_HIEX
OAL_STATIC int32_t wal_get_tb_frame_gain(oal_net_device_stru *net_dev, oal_ifreq_stru *ifr)
{
    mac_vap_stru *mac_vap;
    mac_channel_stru *channel;
    uint8_t narrowband_gain_info[2]; // 上报2个字节，工作信道的信道号和此信道的增益
    int32_t ret;

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "wal_get_tb_frame_gain: pst_mac_vap get from netdev is null.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    channel = &mac_vap->st_channel;
    narrowband_gain_info[0] = channel->uc_chan_number;
    narrowband_gain_info[1] = hmac_get_tb_frame_gain(channel->en_band, channel->uc_chan_number);

    ret = oal_copy_to_user(ifr->ifr_data + 8, narrowband_gain_info, sizeof(narrowband_gain_info)); /* 8为hdr len */
    if (ret) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_get_narrowband_gain::Failed to copy data to user error %d!}", ret);
    }
    return ret;
}
#endif

OAL_STATIC uint32_t wal_vendor_priv_cmd_hid2d(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *p_ifr,
    uint8_t *pc_cmd, wal_wifi_priv_cmd_stru *p_priv_cmd)
{
    if (oal_strncasecmp(pc_cmd, CMD_PCIE_ASPM_STATE, CMD_PCIE_ASPM_STATE_LEN) == 0) {
        if (OAL_STRLEN(pc_cmd) < (CMD_PCIE_ASPM_STATE_LEN + CMD_LENGTH)) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_vendor_priv_cmd_hid2d::aspm status, cmd invalid");
            return -OAL_EFAIL;
        }
        return hi110x_hcc_ip_pm_ctrl(oal_atoi(pc_cmd + CMD_PCIE_ASPM_STATE_LEN + 1));
    } else if  (oal_strncasecmp(pc_cmd, CMD_WLAN_FREQ, CMD_WLAN_FREQ_LEN) == 0) {
        if (OAL_STRLEN(pc_cmd) < (CMD_WLAN_FREQ_LEN + CMD_LENGTH)) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_vendor_priv_cmd_hid2d::freq boost ctrl, cmd invalid}");
            return -OAL_EFAIL;
        }
        return wal_hid2d_freq_boost(pst_net_dev, oal_atoi(pc_cmd + CMD_WLAN_FREQ_LEN + 1));
    } else if  (oal_strncasecmp(pc_cmd, CMD_NAPI_STATE, CMD_NAPI_STATE_LEN) == 0) {
        if (OAL_STRLEN(pc_cmd) < (CMD_NAPI_STATE_LEN + CMD_LENGTH)) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_vendor_priv_cmd_hid2d::napi ctrl, cmd invalid}");
            return -OAL_EFAIL;
        }
        return wal_hid2d_napi_mode(pst_net_dev, oal_atoi(pc_cmd + CMD_NAPI_STATE_LEN + 1));
    } else if  (oal_strncasecmp(pc_cmd, CMD_PM_STATE, CMD_PM_STATE_LEN) == 0) {
        if (OAL_STRLEN(pc_cmd) < (CMD_PM_STATE_LEN + CMD_LENGTH)) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_vendor_priv_cmd_hid2d::sleep mode ctrl, cmd invalid}");
            return -OAL_EFAIL;
        }
        return wal_hid2d_sleep_mode(pst_net_dev, oal_atoi(pc_cmd + CMD_PM_STATE_LEN + 1), p_priv_cmd);
    } else if  (oal_strncasecmp(pc_cmd, CMD_GSO_STATE, CMD_GSO_STATE_LEN) == 0) {
        if (OAL_STRLEN(pc_cmd) < (CMD_GSO_STATE_LEN + CMD_LENGTH)) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_vendor_priv_cmd_hid2d::gso ctrl, cmd invalid");
            return -OAL_EFAIL;
        }
        return wal_hid2d_gso_mode(pst_net_dev, oal_atoi(pc_cmd + CMD_GSO_STATE_LEN + 1));
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
    } else if (oal_strncasecmp(pc_cmd, CMD_LOW_LATENCY_ON, CMD_LOW_LATENCY_ON_LEN) == 0) {
        wal_ioctl_set_hid2d_state(pst_net_dev, ENABLE, p_priv_cmd);
    } else if (oal_strncasecmp(pc_cmd, CMD_LOW_LATENCY_OFF, CMD_LOW_LATENCY_OFF_LEN) == 0) {
        wal_ioctl_set_hid2d_state(pst_net_dev, DISABLE, p_priv_cmd);
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D_PRESENTATION
    } else {
        return wal_vendor_priv_cmd_hid2d_apk(pst_net_dev, p_ifr, pc_cmd);
#endif
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_vendor_priv_cmd_ext3(oal_net_device_stru *p_net_dev,
    oal_ifreq_stru *p_ifr, wal_wifi_priv_cmd_stru *p_priv_cmd, uint8_t *p_cmd)
{
    if (oal_strncasecmp(p_cmd, CMD_GET_WIFI6_SUPPORT, CMD_GET_WIFI6_SUPPORT_LEN) == 0) {
        return wal_vendor_get_wifi6_supported(p_net_dev, p_ifr);
    }
#ifdef _PRE_WLAN_FEATURE_11AX
    if (oal_strncasecmp(p_cmd, CMD_SET_AX_CLOSE_HTC, CMD_SET_AX_CLOSE_HTC_LEN) == 0) {
        return wal_vendor_11ax_close_htc_handle(p_net_dev);
    } else if (oal_strncasecmp(p_cmd, CMD_SET_AX_BLACKLIST, CMD_SET_AX_BLACKLIST_LEN) == 0) {
        return wal_vendor_11ax_wifi6_balcklist_handle(p_net_dev, p_cmd, p_priv_cmd->total_len);
    }
#endif
    if (oal_strncasecmp(p_cmd, CMD_SET_CLREAR_11N_BLACKLIST, CMD_SET_CLREAR_11N_BLACKLIST_LEN) == 0) {
        return wal_vendor_clear_11n_blacklist_handle(p_net_dev);
    }
#ifdef _PRE_WLAN_FEATURE_MONITOR
    if (oal_strncasecmp(p_cmd, CMD_QUERY_SNIFFER, CMD_QUERY_SNIFFER_LEN) == 0) {
        return wal_vendor_sniffer_handle(p_net_dev, p_cmd, p_priv_cmd->total_len);
    }
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
    if (oal_strncasecmp(p_cmd, CMD_QUERY_CSI, CMD_QUERY_CSI_LEN) == 0) {
        return wal_vendor_csi_handle(p_net_dev, p_cmd, p_priv_cmd->total_len);
    }
#endif

    return wal_vendor_priv_cmd_hid2d(p_net_dev, p_ifr, p_cmd, p_priv_cmd);
}


OAL_STATIC int32_t wal_vendor_priv_cmd_ext2(oal_net_device_stru *pst_net_dev,
    oal_ifreq_stru *pst_ifr, wal_wifi_priv_cmd_stru *p_priv_cmd, uint8_t *pc_cmd)
{
    if (oal_strncasecmp(pc_cmd, CMD_SET_CLOSE_GO_CAC, CMD_SET_CLOSE_GO_CAC_LEN) == 0) {
        g_go_cac = OAL_FALSE;
        return OAL_SUCC;
    }

    if (OAL_SUCC != wal_vendor_priv_cmd_power_saving(pst_net_dev, pst_ifr, pc_cmd)) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_vendor_priv_cmd:one of power saving cmds faild!");
        return -OAL_EFAIL;
    }

    if (0 == oal_strncasecmp(pc_cmd, CMD_GET_RADAR_RESULT, CMD_GET_RADAR_RESULT_LEN)) {
        return wal_vendor_get_radar_result(pst_net_dev, pst_ifr, pc_cmd);
    }
#ifdef _PRE_WLAN_FEATURE_HIEX
    if (0 == oal_strncasecmp(pc_cmd, CMD_GET_TB_FRAME_GAIN, CMD_GET_TB_FRAME_GAIN_LEN)) {
        return wal_get_tb_frame_gain(pst_net_dev, pst_ifr);
    }
#endif
    if (0 == oal_strncasecmp(pc_cmd, CMD_SET_P2P_SCENE, CMD_SET_P2P_SCENE_LEN)) {
        wal_vendor_set_p2p_scenes(pst_net_dev, pc_cmd, p_priv_cmd->total_len);
    }
    if (oal_strncasecmp(pc_cmd, CMD_SET_GO_DETECT_RADAR, CMD_SET_GO_DETECT_RADAR_LEN) == 0) {
        wal_hipriv_set_dfs_mode_scenes(pst_net_dev, pc_cmd, p_priv_cmd->total_len);
    }
    if (0 == oal_strncasecmp(pc_cmd, CMD_SET_CHANGE_GO_CHANNEL, CMD_SET_CHANGE_GO_CHANNEL_LEN)) {
        return wal_vendor_set_change_go_channel(pst_net_dev, pst_ifr, pc_cmd, p_priv_cmd->total_len);
    }
    if (0 == oal_strncasecmp(pc_cmd, CMD_SET_DYNAMIC_DBAC_MODE, CMD_SET_DYNAMIC_DBAC_MODE_LEN)) {
        return wal_ioctl_set_dynamic_dbac_mode(pst_net_dev, pc_cmd);
    }

    return wal_vendor_priv_cmd_ext3(pst_net_dev, pst_ifr, p_priv_cmd, pc_cmd);
}


int32_t wal_vendor_set_cali_fem_mode(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr, int8_t *pc_cmd)
{
#ifndef WIN32
    /* 设置校准时是否关fem的标志 */
    uint8_t uc_cali_fem_on = (uint8_t)oal_atoi(pc_cmd + OAL_STRLEN((int8_t *)CMD_SET_CALI_FEM_MODE) + 1);
    if (uc_cali_fem_on) {
        hmac_set_close_fem_cali_status(OAL_FALSE);
    } else {
        hmac_set_close_fem_cali_status(OAL_TRUE);
    }
#endif

    return OAL_SUCC;
}


int32_t wal_ioctl_set_mlme_ie(oal_net_device_stru *pst_net_dev, oal_mlme_ie_stru *pst_mlme_ie)
{
    wal_msg_write_stru st_write_msg;
    oal_mlme_ie_stru st_mlme_ie;
    oal_w2h_mlme_ie_stru *pst_w2h_mlme_ie = NULL;
    wal_msg_stru *pst_rsp_msg = NULL;
    uint32_t err_code;
    int32_t l_ret;

    memset_s(&st_mlme_ie, sizeof(oal_mlme_ie_stru), 0, sizeof(oal_mlme_ie_stru));

    if (pst_mlme_ie->us_ie_len > WLAN_WPS_IE_MAX_SIZE) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_ioctl_set_mlme_ie::en_mlme_type[%d], uc_ie_len[%u] beyond range !}",
                       st_mlme_ie.en_mlme_type, st_mlme_ie.us_ie_len);
        return -OAL_EFAIL;
    }
    l_ret = memcpy_s(&st_mlme_ie, sizeof(oal_mlme_ie_stru), pst_mlme_ie, sizeof(oal_mlme_ie_stru));

    oam_warning_log3(0, OAM_SF_ANY, "{wal_ioctl_set_mlme_ie::en_mlme_type[%d], uc_status=%u, uc_ie_len[%u]!}",
                     st_mlme_ie.en_mlme_type, st_mlme_ie.us_status, st_mlme_ie.us_ie_len);

    pst_w2h_mlme_ie = (oal_w2h_mlme_ie_stru *)(st_write_msg.auc_value);
    l_ret += memcpy_s(pst_w2h_mlme_ie, sizeof(st_write_msg.auc_value),
                      &st_mlme_ie, (sizeof(oal_mlme_ie_stru) - WLAN_WPS_IE_MAX_SIZE));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_set_mlme_ie::memcpy fail!");
        return -OAL_EFAIL;
    }

    pst_w2h_mlme_ie->puc_data_ie = st_mlme_ie.auc_ie;

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_MLME, sizeof(oal_w2h_mlme_ie_stru));

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(oal_w2h_mlme_ie_stru),
        (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if ((l_ret != OAL_SUCC) || (pst_rsp_msg == NULL)) {
        oam_error_log1(0, OAM_SF_P2P, "{wal_ioctl_set_mlme:: wal_alloc_cfg_event return err code %d!}", l_ret);
        return l_ret;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_P2P, "{wal_ioctl_set_mlme::wal_send_cfg_event return err code:[%x]!}", err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t wal_ioctl_set_dc_status(oal_net_device_stru *pst_net_dev, int32_t dc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;

    if (dc_param < 0) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_ioctl_set_dc_status::check input[%d]!}", dc_param);
        return -OAL_EFAIL;
    }

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DC_STATUS, sizeof(uint8_t));
    st_write_msg.auc_value[0] = (uint8_t)(dc_param ? OAL_TRUE : OAL_FALSE);

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_ioctl_set_dc_status::return err code [%d]!}", l_ret);
        return (uint32_t)l_ret;
    }

    oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_set_dc_status::dc_param[%d].}", dc_param);
    return OAL_SUCC;
}
OAL_STATIC int32_t wal_ioctl_set_p2p_miracast_status(oal_net_device_stru *pst_net_dev, uint8_t uc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_P2P_MIRACAST_STATUS, sizeof(uint8_t));

    st_write_msg.auc_value[0] = uc_param;

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_ioctl_set_miracast_status::return err code [%d]!}", l_ret);
        return (uint32_t)l_ret;
    }

    oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_set_miracast_status::uc_param[%d].}", uc_param);
    return OAL_SUCC;
}


OAL_STATIC int32_t wal_ioctl_set_p2p_noa(oal_net_device_stru *net_dev, mac_cfg_p2p_noa_param_stru *p2p_noa_param)
{
    wal_msg_write_stru st_write_msg;
    wal_msg_stru *pst_rsp_msg = NULL;
    uint32_t err_code;
    int32_t l_ret;

    l_ret = memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                     p2p_noa_param, sizeof(mac_cfg_p2p_noa_param_stru));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_P2P, "wal_ioctl_set_p2p_noa::memcpy fail!");
        return -OAL_EFAIL;
    }

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_P2P_PS_NOA, sizeof(mac_cfg_p2p_noa_param_stru));

    /* 发送消息 */
    l_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_p2p_noa_param_stru),
        (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if ((l_ret != OAL_SUCC) || (pst_rsp_msg == NULL)) {
        oam_error_log1(0, OAM_SF_P2P, "{wal_ioctl_set_p2p_noa:: wal_alloc_cfg_event return err code %d!}", l_ret);
        return l_ret;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_P2P, "{wal_ioctl_set_p2p_noa::wal_check_and_release_msg_resp fail[%d]!}", err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_ioctl_set_p2p_ops(oal_net_device_stru *net_dev, mac_cfg_p2p_ops_param_stru *p2p_ops_param)
{
    wal_msg_write_stru st_write_msg;
    wal_msg_stru *pst_rsp_msg = NULL;
    uint32_t err_code;
    int32_t l_ret;

    l_ret = memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                     p2p_ops_param, sizeof(mac_cfg_p2p_ops_param_stru));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_P2P, "wal_ioctl_set_p2p_ops::memcpy fail!");
        return -OAL_EFAIL;
    }

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_P2P_PS_OPS, sizeof(mac_cfg_p2p_ops_param_stru));

    /* 发送消息 */
    l_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_p2p_ops_param_stru),
        (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if ((l_ret != OAL_SUCC) || (pst_rsp_msg == NULL)) {
        oam_error_log1(0, OAM_SF_P2P, "{wal_ioctl_set_p2p_ops:: wal_alloc_cfg_event return err code %d!}", l_ret);
        return l_ret;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_P2P, "{wal_ioctl_set_p2p_ops::wal_check_and_release_msg_resp fail[%d]!}", err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_ioctl_set_vowifi_param(oal_net_device_stru *pst_net_dev, int8_t *puc_command,
    wal_wifi_priv_cmd_stru *pst_priv_cmd)
{
    int32_t l_ret;
    uint16_t us_len;
    wal_msg_write_stru st_write_msg;
    mac_cfg_vowifi_stru *pst_cfg_vowifi = NULL;
    mac_vowifi_cmd_enum_uint8 en_vowifi_cmd_id;
    uint8_t uc_param;
    uint8_t uc_cfg_id;

    /* vap未创建时，不处理supplicant命令 */
    if (oal_net_dev_priv(pst_net_dev) == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "wal_ioctl_set_vowifi_param::vap not created yet, ignore the cmd!");
        return -OAL_EINVAL;
    }

    for (uc_cfg_id = VOWIFI_SET_MODE; uc_cfg_id < VOWIFI_CMD_BUTT; uc_cfg_id++) {
        if ((0 == oal_strncmp(puc_command, g_ast_vowifi_cmd_table[uc_cfg_id].pc_priv_cmd,
            OAL_STRLEN(g_ast_vowifi_cmd_table[uc_cfg_id].pc_priv_cmd))) &&
            (pst_priv_cmd->total_len > (OAL_STRLEN(g_ast_vowifi_cmd_table[uc_cfg_id].pc_priv_cmd) + 1))) {
            uc_param = (uint8_t)oal_atoi((int8_t*)puc_command +
                OAL_STRLEN((uint8_t *)g_ast_vowifi_cmd_table[uc_cfg_id].pc_priv_cmd) + 1);
            en_vowifi_cmd_id = (uint8_t)g_ast_vowifi_cmd_table[uc_cfg_id].case_entry;
            break;
        }
    }

    if (uc_cfg_id >= VOWIFI_CMD_BUTT) {
        oam_warning_log0(0, OAM_SF_CFG, "wal_ioctl_set_vowifi_param::invalid cmd!");
        return -OAL_EINVAL;
    }

    oam_warning_log2(0, OAM_SF_ANY,
        "{wal_ioctl_set_vowifi_param::set VoWiFi_param cmd(%d), value[%d]}", en_vowifi_cmd_id, uc_param);

    /* 抛事件到wal层处理 */
    us_len = sizeof(mac_cfg_vowifi_stru);
    pst_cfg_vowifi = (mac_cfg_vowifi_stru *)(st_write_msg.auc_value);
    pst_cfg_vowifi->en_vowifi_cfg_cmd = en_vowifi_cmd_id;
    pst_cfg_vowifi->uc_value = uc_param;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_VOWIFI_INFO, us_len);
    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len, (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "wal_ioctl_set_vowifi_param::send cfg event fail[%d]!", l_ret);
        return l_ret;
    }
    return OAL_SUCC;
}


OAL_STATIC int32_t wal_ioctl_get_vowifi_param(oal_net_device_stru *net_dev, int8_t *command, int32_t *value)
{
    mac_vap_stru *pst_mac_vap = NULL;

    /* vap未创建时，不处理supplicant命令 */
    if (oal_net_dev_priv(net_dev) == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "wal_ioctl_get_vowifi_param::vap not created yet, ignore the cmd!");
        return -OAL_EINVAL;
    }

    /* 获取mac_vap */
    pst_mac_vap = oal_net_dev_priv(net_dev);
    if (pst_mac_vap->pst_vowifi_cfg_param == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_ioctl_get_vowifi_param::pst_vowifi_cfg_param is null.}");
        return OAL_SUCC;
    }

    if (oal_strncmp(command, CMD_VOWIFI_GET_MODE, OAL_STRLEN(CMD_VOWIFI_GET_MODE)) == 0) {
        *value = (int32_t)pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_mode;
    } else if (oal_strncmp(command, CMD_VOWIFI_GET_PERIOD, OAL_STRLEN(CMD_VOWIFI_GET_PERIOD)) == 0) {
        *value = (int32_t)pst_mac_vap->pst_vowifi_cfg_param->us_rssi_period_ms / HMAC_S_TO_MS;
    } else if (oal_strncmp(command, CMD_VOWIFI_GET_LOW_THRESHOLD, OAL_STRLEN(CMD_VOWIFI_GET_LOW_THRESHOLD)) == 0) {
        *value = (int32_t)pst_mac_vap->pst_vowifi_cfg_param->c_rssi_low_thres;
    } else if (oal_strncmp(command, CMD_VOWIFI_GET_HIGH_THRESHOLD,
        OAL_STRLEN(CMD_VOWIFI_GET_HIGH_THRESHOLD)) == 0) {
        *value = (int32_t)pst_mac_vap->pst_vowifi_cfg_param->c_rssi_high_thres;
    } else if (oal_strncmp(command, CMD_VOWIFI_GET_TRIGGER_COUNT, OAL_STRLEN(CMD_VOWIFI_GET_TRIGGER_COUNT)) == 0) {
        *value = (int32_t)pst_mac_vap->pst_vowifi_cfg_param->uc_trigger_count_thres;
    } else {
        oam_warning_log0(0, OAM_SF_CFG, "wal_ioctl_get_vowifi_param::invalid cmd!");
        *value = 0xffffffff;
        return -OAL_EINVAL;
    }

    oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_get_vowifi_param::supplicant get VoWiFi_param value[%d] }", *value);

    return OAL_SUCC;
}


static uint32_t wal_ioctl_parse_wps_p2p_ie(oal_app_ie_stru *pst_app_ie, uint8_t *puc_src, uint32_t src_len)
{
    uint8_t *puc_ie = NULL;
    uint32_t ie_len;
    uint8_t *puc_buf_remain = NULL;
    uint32_t len_remain;

    if (oal_any_null_ptr2(pst_app_ie, puc_src)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_ioctl_parse_wps_p2p_ie::param NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (src_len == 0 || src_len > WLAN_WPS_IE_MAX_SIZE) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_ioctl_parse_wps_p2p_ie::ul_src_len=[%d] is invailid!}", src_len);
        return OAL_FAIL;
    }

    pst_app_ie->ie_len = 0;
    puc_buf_remain = puc_src;
    len_remain = src_len;

    while (len_remain > MAC_IE_HDR_LEN) {
        /* MAC_EID_WPS,MAC_EID_P2P ID均为221 */
        puc_ie = mac_find_ie(MAC_EID_P2P, puc_buf_remain, (int32_t)len_remain);
        if (puc_ie != NULL) {
            ie_len = (uint8_t)puc_ie[1] + MAC_IE_HDR_LEN;
            if ((ie_len > (WLAN_WPS_IE_MAX_SIZE - pst_app_ie->ie_len)) ||
                (src_len < ((uint16_t)(puc_ie - puc_src) + ie_len))) {
                oam_warning_log3(0, OAM_SF_CFG,
                    "{wal_ioctl_parse_wps_p2p_ie::ie_len[%d],left buffer size[%d], src_end_len[%d],param invalid!}",
                    ie_len, WLAN_WPS_IE_MAX_SIZE - pst_app_ie->ie_len, puc_ie - puc_src + ie_len);
                return OAL_FAIL;
            }
            if (EOK != memcpy_s(&(pst_app_ie->auc_ie[pst_app_ie->ie_len]), (WLAN_WPS_IE_MAX_SIZE - pst_app_ie->ie_len), puc_ie, ie_len)) {
                oam_error_log0(0, OAM_SF_CFG, "wal_ioctl_parse_wps_p2p_ie::memcpy fail!");
                return OAL_FAIL;
            }
            pst_app_ie->ie_len += ie_len;
            puc_buf_remain = puc_ie + ie_len;
            len_remain = src_len - (uint32_t)(puc_buf_remain - puc_src);
        } else {
            break;
        }
    }

    if (pst_app_ie->ie_len > 0) {
        return OAL_SUCC;
    }

    return OAL_FAIL;
}

OAL_STATIC int32_t wal_ioctl_send_wps_p2p_ie_cfg_event(oal_net_device_stru *net_dev,
    wal_msg_write_stru *write_msg, oal_app_ie_stru *app_ie)
{
    int32_t ret;
    uint32_t err_code;
    oal_w2h_app_ie_stru *w2h_wps_p2p_ie = NULL;
    wal_msg_stru *rsp_msg = NULL;
    /* 抛事件到wal层处理 */
    w2h_wps_p2p_ie = (oal_w2h_app_ie_stru *)write_msg->auc_value;
    w2h_wps_p2p_ie->en_app_ie_type = app_ie->en_app_ie_type;
    w2h_wps_p2p_ie->ie_len = app_ie->ie_len;
    w2h_wps_p2p_ie->puc_data_ie = app_ie->auc_ie;
    w2h_wps_p2p_ie->delivery_time = oal_time_get_stamp_ms();

    WAL_WRITE_MSG_HDR_INIT(write_msg, WLAN_CFGID_SET_WPS_P2P_IE, sizeof(oal_w2h_app_ie_stru));

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(oal_w2h_app_ie_stru),
                             (uint8_t *)write_msg, OAL_TRUE, &rsp_msg);
    if ((ret != OAL_SUCC) || (rsp_msg == NULL)) {
        oam_error_log1(0, OAM_SF_P2P, "{wal_ioctl_set_wps_p2p_ie:: wal_alloc_cfg_event err %d!}", ret);
        return ret;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_P2P, "{wal_ioctl_set_wps_p2p_ie::check_and_release_msg_resp fail[%d]!}", err_code);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}


OAL_STATIC int32_t wal_ioctl_set_wps_p2p_ie(oal_net_device_stru *net_dev,
    uint8_t *puc_buf, uint32_t len, en_app_ie_type_uint8 en_type)
{
    wal_msg_write_stru st_write_msg;
    oal_app_ie_stru st_app_ie;

    mac_vap_stru *pst_mac_vap = NULL;

    if (len > WLAN_WPS_IE_MAX_SIZE) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_ioctl_set_wps_p2p_ie:: wrong len:[%u]!}", len);
        return -OAL_EFAIL;
    }

    pst_mac_vap = oal_net_dev_priv(net_dev);
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_ioctl_set_wps_p2p_ie::pst_mac_vap null}");
        return -OAL_EINVAL;
    }

    memset_s(&st_app_ie, sizeof(oal_app_ie_stru), 0, sizeof(oal_app_ie_stru));
    switch (en_type) {
        case OAL_APP_BEACON_IE:
        case OAL_APP_PROBE_RSP_IE:
        case OAL_APP_ASSOC_RSP_IE:
            st_app_ie.en_app_ie_type = en_type;
            break;
        default:
            oam_error_log1(0, OAM_SF_CFG, "{wal_ioctl_set_wps_p2p_ie:: wrong type: [%x]!}", en_type);
            return -OAL_EFAIL;
    }

    if (OAL_FALSE == IS_LEGACY_VAP(pst_mac_vap)) {
        if (OAL_SUCC != wal_ioctl_parse_wps_p2p_ie(&st_app_ie, puc_buf, len)) {
            oam_warning_log1(0, OAM_SF_CFG, "{wal_ioctl_set_wps_p2p_ie::Type=[%d], parse p2p ie fail!}", en_type);
            return -OAL_EFAIL;
        }
    } else {
        if (EOK != memcpy_s(st_app_ie.auc_ie, WLAN_WPS_IE_MAX_SIZE, puc_buf, len)) {
            oam_error_log0(0, OAM_SF_CFG, "wal_ioctl_set_wps_p2p_ie::memcpy fail!");
            return -OAL_EFAIL;
        }
        st_app_ie.ie_len = len;
    }
    oam_warning_log3(0, OAM_SF_CFG, "{wal_ioctl_set_wps_p2p_ie::p2p_ie_type[%d],len[%d],st_app_ie.ie_len[%d]}",
                     en_type, len, st_app_ie.ie_len);

    return wal_ioctl_send_wps_p2p_ie_cfg_event(net_dev, &st_write_msg, &st_app_ie);
}


OAL_STATIC int32_t wal_ioctl_priv_cmd_set_ap_wps_p2p_ie(oal_net_device_stru *pst_net_dev,
    int8_t *pc_command, wal_wifi_priv_cmd_stru st_priv_cmd)
{
    uint32_t skip = CMD_SET_AP_WPS_P2P_IE_LEN + 1;
    oal_app_ie_stru *pst_wps_p2p_ie = NULL;
    int32_t l_ret;

    /* 外部输入参数判断，外部输入数据长度必须要满足oal_app_ie_stru结构体头部大小 */
    l_ret = wal_ioctl_judge_input_param_length(st_priv_cmd, CMD_SET_AP_WPS_P2P_IE_LEN,
        (sizeof(oal_app_ie_stru) - (sizeof(uint8_t) * WLAN_WPS_IE_MAX_SIZE)));
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_priv_cmd_set_ap_wps_p2p_ie::len too short! at least need [%d]!}",
                         (skip + sizeof(oal_app_ie_stru) - (sizeof(uint8_t) * WLAN_WPS_IE_MAX_SIZE)));
        return -OAL_EFAIL;
    }

    pst_wps_p2p_ie = (oal_app_ie_stru *)(pc_command + skip);

    /*lint -e413*/
    if ((skip + pst_wps_p2p_ie->ie_len + OAL_OFFSET_OF(oal_app_ie_stru, auc_ie)) >
        (uint32_t)st_priv_cmd.total_len) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_ioctl_priv_cmd_set_ap_wps_p2p_ie::SET_AP_WPS_P2P_IE param len:%d err}",
            (skip + pst_wps_p2p_ie->ie_len));
        return -OAL_EFAIL;
    }
    /*lint +e413*/
    l_ret = wal_ioctl_set_wps_p2p_ie(pst_net_dev, pst_wps_p2p_ie->auc_ie,
        pst_wps_p2p_ie->ie_len, pst_wps_p2p_ie->en_app_ie_type);

    return l_ret;
}

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH

OAL_STATIC int32_t wal_ioctl_tas_pow_ctrl(oal_net_device_stru *pst_net_dev, uint8_t uc_coreindex,
                                          oal_bool_enum_uint8 en_need_improve)
{
#ifdef _PRE_WLAN_FEATURE_TPC_OPT
    int32_t l_ret;
    wal_msg_write_stru st_write_msg;
    mac_cfg_tas_pwr_ctrl_stru st_tas_pow_ctrl_params;
    mac_device_stru *pst_mac_device;

    if (g_tas_switch_en[uc_coreindex] == OAL_FALSE) {
        /* 当前天线不支持TAS切换方案 */
        oam_error_log1(0, OAM_SF_ANY, "wal_ioctl_tas_pow_ctrl::core[%d] is not supported!", uc_coreindex);
        return OAL_SUCC;
    }

    memset_s(&st_tas_pow_ctrl_params, sizeof(mac_cfg_tas_pwr_ctrl_stru), 0, sizeof(mac_cfg_tas_pwr_ctrl_stru));
    st_tas_pow_ctrl_params.en_need_improved = en_need_improve;
    st_tas_pow_ctrl_params.uc_core_idx = uc_coreindex;

    /* vap未创建时，不处理supplicant命令 */
    if (oal_net_dev_priv(pst_net_dev) == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "wal_ioctl_tas_pow_ctrl::vap not created yet, ignore the cmd!");
        return -OAL_EINVAL;
    }

    pst_mac_device = mac_res_get_dev(0);
    /* 如果非单VAP,则不处理 */
    if (1 != mac_device_calc_up_vap_num(pst_mac_device)) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_ioctl_tas_pow_ctrl::abort for more than 1 vap");
        return OAL_SUCC;
    }

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_TAS_PWR_CTRL, sizeof(mac_cfg_tas_pwr_ctrl_stru));
    if (EOK != memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                        &st_tas_pow_ctrl_params, sizeof(mac_cfg_tas_pwr_ctrl_stru))) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_tas_pow_ctrl::memcpy fail!");
        return OAL_FAIL;
    }
    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tas_pwr_ctrl_stru),
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "wal_ioctl_tas_pow_ctrl::send cfg event fail[%d]!", l_ret);
        return l_ret;
    }
#endif
    return OAL_SUCC;
}


OAL_STATIC int32_t wal_ioctl_tas_rssi_access(oal_net_device_stru *pst_net_dev, uint8_t uc_coreindex)
{
    int32_t l_ret;
    wal_msg_write_stru st_write_msg;

    /* vap未创建时，不处理supplicant命令 */
    if (oal_net_dev_priv(pst_net_dev) == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "wal_ioctl_tas_rssi_access::vap not created yet, ignore the cmd!");
        return -OAL_EINVAL;
    }

    if (g_tas_switch_en[uc_coreindex] == OAL_FALSE) {
        /* 当前天线不支持TAS切换方案 */
        oam_error_log1(0, OAM_SF_ANY, "wal_ioctl_tas_rssi_access::core[%d] is not supported!", uc_coreindex);
        return OAL_SUCC;
    }

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_TAS_RSSI_ACCESS, sizeof(uint8_t));
    st_write_msg.auc_value[0] = uc_coreindex;
    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "wal_ioctl_tas_rssi_access::send cfg event fail[%d]!", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}
#endif

#ifndef CONFIG_HAS_EARLYSUSPEND

OAL_STATIC int32_t wal_ioctl_set_suspend_mode(oal_net_device_stru *pst_net_dev, uint8_t uc_suspend)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;

    if (oal_unlikely((pst_net_dev == NULL))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_set_suspend_mode::pst_net_dev null ptr error!}");
        return -OAL_EFAUL;
    }

    st_write_msg.auc_value[0] = uc_suspend;

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_SUSPEND_MODE, sizeof(uc_suspend));

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uc_suspend), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    return l_ret;
}
#endif


OAL_STATIC uint32_t wal_ioctl_set_fastsleep_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_command)
{
    uint32_t ret;
    uint8_t  auc_para_val[4]; /* 4 cmdpara */
    uint8_t  auc_str_tmp[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t pul_cmd_offset;
    uint32_t para_cnt;
    int8_t  *puc_para_str = NULL;

    if (OAL_STRLEN(pc_command) < CMD_SET_FASTSLEEP_SWITCH_LEN + 1) {
        oam_warning_log1(0, OAM_SF_CFG, "wal_ioctl_set_fastsleep_switch:cmd fail!", OAL_STRLEN(pc_command));
        return OAL_FAIL;
    }

    puc_para_str = pc_command + CMD_SET_FASTSLEEP_SWITCH_LEN;

    if (!oal_strncasecmp(puc_para_str, "0", 1)) {
        /* 关闭fastsleep */
        wal_ioctl_set_sta_ps_mode(pst_net_dev, MIN_FAST_PS);
        oam_warning_log0(0, OAM_SF_CFG, "wal_ioctl_set_fastsleep_switch:disable fast sleep!");
    } else if (!oal_strncasecmp(puc_para_str, "1", 1)) {
        puc_para_str += 1;
        /* 获取携带的4个参数<min listen时间><max listen时间><亮屏收发包门限><暗屏收发包门限> */
        for (para_cnt = 0; para_cnt < 4; para_cnt++) {
            ret = wal_get_cmd_one_arg(puc_para_str, auc_str_tmp, WAL_HIPRIV_CMD_NAME_MAX_LEN, &pul_cmd_offset);
            if (ret != OAL_SUCC) {
                oam_warning_log2(0, OAM_SF_CFG, "{wal_ioctl_set_fastsleep_switch::get para[%d] fail, err_code[%d]}",
                    para_cnt, ret);
                return ret;
            }
            auc_para_val[para_cnt] = (uint8_t)oal_atoi(auc_str_tmp);
            puc_para_str += pul_cmd_offset;
        }

#ifndef WIN32
        auc_para_val[0] /= WLAN_SLEEP_TIMER_PERIOD;
        auc_para_val[1] /= WLAN_SLEEP_TIMER_PERIOD;
        /* 赋值给host全局变量 */
        g_wlan_min_fast_ps_idle = auc_para_val[0];
        g_wlan_max_fast_ps_idle = auc_para_val[1];
        g_wlan_auto_ps_screen_on = auc_para_val[BYTE_OFFSET_2];
        g_wlan_auto_ps_screen_off = auc_para_val[BYTE_OFFSET_3];

        oam_warning_log4(0, OAM_SF_CFG, "{wal_ioctl_set_fastsleep_switch::g_wlan_min_fast_ps_idle[%d], \
            max_fast_ps_idle[%d], auto_ps_thresh_screen_on[%d], auto_ps_thresh_screen_off!}", g_wlan_min_fast_ps_idle,
            g_wlan_max_fast_ps_idle, g_wlan_auto_ps_screen_on, g_wlan_auto_ps_screen_off);
#endif
        /* 下发参数 */
        wal_ioctl_set_fast_sleep_para(pst_net_dev, auc_para_val);
        /* 下发ps mode */
        wal_ioctl_set_sta_ps_mode(pst_net_dev, AUTO_FAST_PS);
    } else {
        oam_warning_log0(0, OAM_SF_CFG, "wal_ioctl_set_fastsleep_switch:invalid cmd str!");
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_ioctl_tcp_ack_buf_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_command)
{
    int8_t *puc_para_str = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    mac_vap_stru *pst_mac_vap = oal_net_dev_priv(pst_net_dev);

    oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_tcp_ack_buf_switch::user control tcp_ack_buf start!}");
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_ioctl_tcp_ack_buf_switch:pst_mac_vap == NULL!");
        return OAL_FAIL;
    }

    if (!IS_LEGACY_STA(pst_mac_vap)) {
        /* 非STA模式不支持上层指定TCP ack缓存 */
        oam_warning_log0(0, OAM_SF_ANY, "wal_ioctl_tcp_ack_buf_switch:not STA!");
        return OAL_FAIL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_ioctl_tcp_ack_buf_switch:hmac_vap null!");
        return OAL_FAIL;
    }

    /* STA模式支持上层指定TCP ack缓存 */
    /* 合法的pc_command为命令说明加1个命令码 */
    if (OAL_STRLEN(pc_command) != CMD_SET_TCP_ACK_CTL_LEN + 1) {
        oam_warning_log1(0, OAM_SF_CFG, "wal_ioctl_tcp_ack_buf_switch:cmd fail!cmd_len[%d]", OAL_STRLEN(pc_command));
        return OAL_FAIL;
    }

    /* 获取命令码 */
    puc_para_str = pc_command + CMD_SET_TCP_ACK_CTL_LEN;

    if (!oal_strncasecmp(puc_para_str, "0", 1)) {
        /* 关闭上层控制的TCP ack缓存功能开关 */
        oam_warning_log0(0, OAM_SF_ANY, "wal_ioctl_tcp_ack_buf_switch:user_tcp_ack_buf_switch is turn off!");
        pst_hmac_vap->tcp_ack_buf_use_ctl_switch = OAL_SWITCH_OFF;
    } else if (!oal_strncasecmp(puc_para_str, "1", 1)) {
        /* 打开上层控制的TCP ack缓存功能开关 */
        oam_warning_log0(0, OAM_SF_ANY, "wal_ioctl_tcp_ack_buf_switch:user_tcp_ack_buf_switch is turn on!");
        pst_hmac_vap->tcp_ack_buf_use_ctl_switch = OAL_SWITCH_ON;
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "wal_ioctl_tcp_ack_buf_switch:cmd code error!cmd[%d]", *puc_para_str);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


OAL_STATIC int32_t wal_ioctl_get_wifi_priv_feature_cap_param(oal_net_device_stru *pst_net_dev,
    int8_t *puc_command, int32_t *pl_value)
{
    mac_vap_stru *pst_mac_vap = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
#ifdef _PRE_WLAN_FEATURE_HIEX
    mac_device_stru *pst_mac_dev = NULL;
#endif
    uint32_t value = 0;

    /* vap未创建时，不处理supplicant命令 */
    if (oal_net_dev_priv(pst_net_dev) == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "wal_ioctl_get_wifi_priv_feature_cap_param::vap not created yet!");
        return -OAL_EINVAL;
    }

    /* 获取mac_vap */
    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_ioctl_get_wifi_priv_feature_cap_param::netdevice->mac_vap is null.}");
        return OAL_SUCC;
    }

    *pl_value = 0;
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_ioctl_get_wifi_priv_feature_cap_param::pst_hmac_vap is null.}");
        return OAL_SUCC;
    }

#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11R)
    /* 11k能力 */
    if (pst_hmac_vap->bit_11k_enable == OAL_TRUE) {
        value |= BIT(WAL_WIFI_FEATURE_SUPPORT_11K);
    }

    /* 11v能力 */
    if (pst_hmac_vap->bit_11v_enable == OAL_TRUE) {
        value |= BIT(WAL_WIFI_FEATURE_SUPPORT_11V);
    }

    if (pst_hmac_vap->bit_11r_enable == OAL_TRUE) {
        value |= BIT(WAL_WIFI_FEATURE_SUPPORT_11R);
    }
#endif
    value |= BIT(WAL_WIFI_FEATURE_SUPPORT_VOWIFI_NAT_KEEP_ALIVE);

#ifdef _PRE_WLAN_FEATURE_HIEX
    pst_mac_dev = mac_res_get_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_mac_dev != NULL && pst_mac_dev->st_hiex_cap.bit_hiex_enable) {
        value |= BIT(WAL_WIFI_FEATURE_SUPPORT_NARROWBAND);
    }
#endif
#ifdef _PRE_WLAN_FEATURE_160M
    value |= BIT(WAL_WIFI_FEATURE_SUPPORT_160M_STA);
    value |= BIT(WAL_WIFI_FEATURE_SUPPORT_160M_AP);
#endif
    *pl_value = value;

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_ioctl_force_stop_filter(oal_net_device_stru *pst_net_dev, uint8_t uc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_FORCE_STOP_FILTER, sizeof(uint8_t));
    st_write_msg.auc_value[0] = uc_param;

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_ioctl_force_stop_filter::return err code [%d]!}", l_ret);
        return (uint32_t)l_ret;
    }

    oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_force_stop_filter::uc_param[%d].}", uc_param);
    return OAL_SUCC;
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

OAL_STATIC int32_t wal_ioctl_reduce_sar(oal_net_device_stru *pst_net_dev, uint16_t tx_power)
{
#ifdef _PRE_WLAN_FEATURE_TPC_OPT
    uint8_t uc_lvl_idx = 0;
    int32_t l_ret;
    wal_msg_write_stru st_write_msg;
    uint16_t data_len = 0;

    oam_warning_log1(0, OAM_SF_TPC, "wal_ioctl_reduce_sar::set tx_power[%d] for reduce SAR purpose.", tx_power);

    /* vap未创建时，不处理supplicant命令 */
    if (oal_net_dev_priv(pst_net_dev) == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "wal_ioctl_reduce_sar::vap not created yet, ignore the cmd!");
        return -OAL_EINVAL;
    }

    /*
        参数10XX代表上层下发的降SAR档位，
        当前档位需求按照"两个WiFi天线接SAR sensor，
        并且区分单WiFi工作，WiFi和Modem一起工作"来预留，
        共需要1001~1020档。
        场景        档位          条件0        条件1    条件2（RPC）   条件3(Ant是否SAR sensor触发)
                              是否和主频同传                           Ant1 Ant3
        Head SAR    档位1001        N           CE卡    receiver on     NA  NA
                    档位1002        Y           CE卡    receiver on     NA  NA
                    档位1003        N           FCC卡   receiver on     NA  NA
                    档位1004        Y           FCC卡   receiver on     NA  NA
        -------------------------------------------------------------------------
        Body SAR    档位1005        N           CE卡    receiver off    0   0
                    档位1006        N           CE卡    receiver off    0   1
                    档位1007        N           CE卡    receiver off    1   0
                    档位1008        N           CE卡    receiver off    1   1
                    档位1009        Y           CE卡    receiver off    0   0
                    档位1010        Y           CE卡    receiver off    0   1
                    档位1011        Y           CE卡    receiver off    1   0
                    档位1012        Y           CE卡    receiver off    1   1
                    档位1013        N           FCC卡   receiver off    0   0
                    档位1014        N           FCC卡   receiver off    0   1
                    档位1015        N           FCC卡   receiver off    1   0
                    档位1016        N           FCC卡   receiver off    1   1
                    档位1017        Y           FCC卡   receiver off    0   0
                    档位1018        Y           FCC卡   receiver off    0   1
                    档位1019        Y           FCC卡   receiver off    1   0
                    档位1020        Y           FCC卡   receiver off    1   1
    */
    if ((tx_power >= 1001) && (tx_power <= 1020)) { /* 1001~1020档 */
        uc_lvl_idx = tx_power - 1000; /* 档位从1000开始计算 */
    }
    if (wlan_chip_get_sar_ctrl_params(uc_lvl_idx, st_write_msg.auc_value, &data_len,
        sizeof(st_write_msg.auc_value)) != OAL_SUCC) {
        return -OAL_EINVAL;
    }

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_REDUCE_SAR, data_len);
    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + data_len,
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "wal_ioctl_reduce_sar::send cfg event fail[%d]!", l_ret);
        return l_ret;
    }
#endif
    return OAL_SUCC;
}

#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

OAL_STATIC int32_t wal_ioctl_priv_cmd_tx_power(oal_net_device_stru *pst_net_dev,
    int8_t *pc_command, wal_wifi_priv_cmd_stru st_priv_cmd)
{
    uint16_t us_txpwr;
    int32_t l_ret;

    l_ret = wal_ioctl_judge_input_param_length(st_priv_cmd, CMD_TX_POWER_LEN, 1);
    if (l_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_priv_cmd_tx_power::len at least CMD_TX_POWER_LEN + 2}");
        return -OAL_EFAIL;
    }

    us_txpwr = (uint16_t)oal_atoi(pc_command + CMD_TX_POWER_LEN + 1);
    l_ret = wal_ioctl_reduce_sar(pst_net_dev, us_txpwr);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_priv_cmd_tx_power::err [%d]!}", l_ret);
        /* 驱动打印错误码，返回成功，防止supplicant 累计4次 ioctl失败导致wifi异常重启 */
        return OAL_SUCC;
    }
    return l_ret;
}

#endif

OAL_STATIC int32_t wal_ioctl_priv_cmd_set_mlme_ie(oal_net_device_stru *pst_net_dev,
    int8_t *pc_command, wal_wifi_priv_cmd_stru st_priv_cmd)
{
    uint32_t skip = CMD_SET_MLME_IE_LEN + 1;
        /* 结构体类型 */
    oal_mlme_ie_stru *pst_mlme_ie;
    pst_mlme_ie = (oal_mlme_ie_stru *)(pc_command + skip);

    /*lint -e413*/
    if ((skip + pst_mlme_ie->us_ie_len + OAL_OFFSET_OF(oal_mlme_ie_stru, auc_ie)) > (uint32_t)st_priv_cmd.total_len) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::SET_ASSOC_RSP_IE param len lt:%d}",
            (skip + pst_mlme_ie->us_ie_len));
        return -OAL_EFAIL;
    }
    /*lint +e413*/
    return wal_ioctl_set_mlme_ie(pst_net_dev, pst_mlme_ie);
}
OAL_STATIC int32_t wal_ioctl_priv_cmd_p2p_set_noa(oal_net_device_stru *pst_net_dev,
    int8_t *pc_command, wal_wifi_priv_cmd_stru st_priv_cmd, int32_t *l_memcpy_ret)
{
    int32_t l_ret;
    uint32_t skip = CMD_P2P_SET_NOA_LEN + 1;

        mac_cfg_p2p_noa_param_stru st_p2p_noa_param;
        l_ret = wal_ioctl_judge_input_param_length(st_priv_cmd, CMD_P2P_SET_NOA_LEN, sizeof(st_p2p_noa_param));
        if (l_ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::CMD_P2P_SET_NOA param len too short. need %d.}",
                skip + sizeof(st_p2p_noa_param));
            return -OAL_EFAIL;
        }
        *l_memcpy_ret += memcpy_s(&st_p2p_noa_param, sizeof(mac_cfg_p2p_noa_param_stru),
            pc_command + skip, sizeof(mac_cfg_p2p_noa_param_stru));
        if (*l_memcpy_ret != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_priv_cmd_p2p_set_noa::memcpy fail!");
        }

    return wal_ioctl_set_p2p_noa(pst_net_dev, &st_p2p_noa_param);
}
OAL_STATIC int32_t wal_ioctl_priv_cmd_p2p_set_ps(oal_net_device_stru *pst_net_dev,
    int8_t *pc_command, wal_wifi_priv_cmd_stru st_priv_cmd, int32_t *l_memcpy_ret)
{
    int32_t l_ret;
    uint32_t skip = CMD_P2P_SET_PS_LEN + 1;

        mac_cfg_p2p_ops_param_stru st_p2p_ops_param;
        l_ret = wal_ioctl_judge_input_param_length(st_priv_cmd, CMD_P2P_SET_PS_LEN, sizeof(st_p2p_ops_param));
        if (l_ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::CMD_P2P_SET_PS param len is too short.need %d.}",
                skip + sizeof(st_p2p_ops_param));
            return -OAL_EFAIL;
        }
        *l_memcpy_ret += memcpy_s(&st_p2p_ops_param, sizeof(mac_cfg_p2p_ops_param_stru),
            pc_command + skip, sizeof(mac_cfg_p2p_ops_param_stru));
        if (*l_memcpy_ret != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_priv_cmd_p2p_set_ps::memcpy fail!");
        }

    return wal_ioctl_set_p2p_ops(pst_net_dev, &st_p2p_ops_param);
}
OAL_STATIC int32_t wal_ioctl_priv_cmd_set_power_on(oal_net_device_stru *pst_net_dev, int8_t *pc_command)
{
    int32_t power_flag;

        uint32_t command_len = OAL_STRLEN(pc_command);
        /* 格式:SET_POWER_ON 1 or SET_POWER_ON 0, +2 表示空格和数字的长度 */
        if (command_len < (CMD_SET_POWER_ON_LEN + 2)) {
            oam_error_log1(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::CMD_SET_POWER_ON cmd len[%d] err}", command_len);

            return -OAL_EFAIL;
        }

        power_flag = oal_atoi(pc_command + CMD_SET_POWER_ON_LEN + 1);

    return wal_netdev_set_power_on(pst_net_dev, power_flag);
}
OAL_STATIC int32_t wal_ioctl_priv_cmd_set_power_mgmt_on(int8_t *pc_command)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    struct wlan_pm_s *pst_wlan_pm = NULL;
#endif
    unsigned long power_mgmt_flag; /* AP模式,默认电源管理是开启的 */
    uint32_t command_len = OAL_STRLEN(pc_command);
    /* 格式:CMD_SET_POWER_MGMT_ON 1 or CMD_SET_POWER_MGMT_ON 0， +2 表示空格和数字的长度 */
    if (command_len < (CMD_SET_POWER_MGMT_ON_LEN + 2)) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::CMD_SET_POWER_MGMT_ON len err:%d}", command_len);

        return -OAL_EFAIL;
    }

    power_mgmt_flag = (unsigned long)oal_atoi(pc_command + CMD_SET_POWER_MGMT_ON_LEN + 1);
    oam_warning_log1(0, 0, "{wal_vendor_priv_cmd::CMD_SET_POWER_MGMT_ON cmd,flag:%u}", power_mgmt_flag);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    pst_wlan_pm = wlan_pm_get_drv();
    if (pst_wlan_pm != NULL) {
        pst_wlan_pm->apmode_allow_pm_flag = power_mgmt_flag;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::wlan_pm_get_drv return null.");
    }
#endif

    return OAL_SUCC;
}
OAL_STATIC int32_t wal_ioctl_priv_cmd_get_capa_dbdc(int8_t *pc_command,
    wal_wifi_priv_cmd_stru st_priv_cmd, int32_t *l_memcpy_ret, oal_ifreq_stru *pst_ifr)
{
    int32_t l_ret;
    int32_t cmd_len = CMD_CAPA_DBDC_SUPP_LEN;
    int32_t ret_len;
    int32_t l_memset_ret;

    if ((uint32_t)st_priv_cmd.total_len < CMD_CAPA_DBDC_SUPP_LEN) {
        oam_error_log1(0, 0, "{wal_vendor_priv_cmd::CMD_GET_CAPA_DBDC len[%d] err}", st_priv_cmd.total_len);
        return -OAL_EFAIL;
    }

    /* 将buf清零 */
    ret_len = oal_max(st_priv_cmd.total_len, cmd_len);
    l_memset_ret = memset_s(pc_command, (uint32_t)(ret_len + 1), 0, (uint32_t)(ret_len + 1));
    if (l_memset_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_priv_cmd_get_capa_dbdc::memset fail!");
    }
    pc_command[ret_len] = '\0';

    /* hi1103 support DBDC */
    *l_memcpy_ret += memcpy_s(pc_command,
        (uint32_t)(ret_len + 1), CMD_CAPA_DBDC_SUPP, CMD_CAPA_DBDC_SUPP_LEN);
    if (*l_memcpy_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_priv_cmd_get_capa_dbdc::memcpy fail!");
    }

    l_ret = oal_copy_to_user(pst_ifr->ifr_data + BYTE_OFFSET_8, pc_command, ret_len);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_vendor_priv_cmd:CMD_GET_CAPA_DBDC Fail to copy ioctl_data to user!");
        return -OAL_EFAIL;
    }
    oam_warning_log2(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::CMD_GET_CAPA_DBDC reply len:%d, ret:%d}",
        OAL_STRLEN(pc_command), l_ret);

    return l_ret;
}
OAL_STATIC int32_t wal_ioctl_priv_cmd_set_memo_change(int8_t *pc_command, wal_wifi_priv_cmd_stru st_priv_cmd)
{
    int32_t l_ret = OAL_SUCC;
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    int32_t l_param_1 = 0;
#endif

    if (oal_any_true_value2(g_tas_switch_en[WLAN_RF_CHANNEL_ZERO], g_tas_switch_en[WLAN_RF_CHANNEL_ONE])) {
        l_ret = wal_ioctl_judge_input_param_length(st_priv_cmd, CMD_SET_MEMO_CHANGE_LEN, 1);
        if (l_ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::CMD_SET_MEMO_CHANGE cmd len err.}");
            oal_free(pc_command);
            return -OAL_EFAIL;
        }

        /* 0:默认态 1:tas态 */
        l_param_1 = oal_atoi(pc_command + CMD_SET_MEMO_CHANGE_LEN + 1);
        oam_warning_log1(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::CMD_SET_MEMO_CHANGE antIndex[%d].}", l_param_1);
        l_ret = board_wifi_tas_set(l_param_1);
    }
    oal_free(pc_command);
    return l_ret;
}
OAL_STATIC int32_t wal_ioctl_priv_cmd_measure_tas_rssi(oal_net_device_stru *pst_net_dev,
    int8_t *pc_command, wal_wifi_priv_cmd_stru st_priv_cmd)
{
    int32_t l_ret;
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    int32_t l_param_1;
#endif

    l_ret = wal_ioctl_judge_input_param_length(st_priv_cmd, CMD_MEASURE_TAS_RSSI_LEN, 1);
    if (l_ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::CMD_MEASURE_TAS_RSSI cmd len error}");
        return -OAL_EFAIL;
    }

    l_param_1 = !!oal_atoi(pc_command + CMD_MEASURE_TAS_RSSI_LEN + 1);
    oam_warning_log1(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::CMD_MEASURE_TAS_RSSI coreIndex[%d].}", l_param_1);
    /* 测量天线 */
    return wal_ioctl_tas_rssi_access(pst_net_dev, l_param_1);
}
OAL_STATIC int32_t wal_ioctl_priv_cmd_set_tas_txpower(oal_net_device_stru *pst_net_dev, int8_t *pc_command)
{
    int32_t l_ret = OAL_SUCC;
    int32_t l_value;
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    int32_t l_param_1;
    int32_t l_param_2 = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 }; /* 预留协议模式字符串空间 */
    uint32_t off_set;
    int8_t *pc_cmd_copy;
#endif

    /* tas抬功率 */
    pc_cmd_copy = pc_command;
    pc_cmd_copy += CMD_SET_TAS_TXPOWER_LEN;
    l_value = wal_get_cmd_one_arg(pc_cmd_copy, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (l_value != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_TPC, "{wal_vendor_priv_cmd::CMD_SET_TAS_TXPOWER err [%d]!}", l_ret);
        return OAL_SUCC;
    }
    l_param_1 = !!oal_atoi(ac_name);
    /* 获取needImprove参数 */
    pc_cmd_copy += off_set;
    l_ret = wal_get_cmd_one_arg(pc_cmd_copy, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (l_ret == OAL_SUCC) {
        l_param_2 = !!oal_atoi(ac_name);
        oam_warning_log2(0, OAM_SF_TPC, "{SET_TAS_TXPOWER coreidx[%d] needImprove[%d]}", l_param_1, l_param_2);
        /* TAS控制抬功率 */
        return wal_ioctl_tas_pow_ctrl(pst_net_dev, l_param_1, l_param_2);
    }

    return OAL_SUCC;
}
#if _PRE_PLAT_FEATURE_CUSTOMIZE
OAL_STATIC int32_t wal_ioctl_priv_cmd_wpas_get_gust(oal_ifreq_stru *pst_ifr,
    int8_t *pc_command, wal_wifi_priv_cmd_stru st_priv_cmd)
{
    int32_t l_memset_ret;
    /* 将buf清零 */
    l_memset_ret = memset_s(pc_command, st_priv_cmd.total_len + 1, 0, st_priv_cmd.total_len + 1);
    if (l_memset_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_priv_cmd_wpas_get_gust::memset fail!");
    }
    pc_command[st_priv_cmd.total_len] = '\0';

    /* 读取全部定制化配置，不单独读取disable_capab_ht40 */
    hwifi_config_host_global_ini_param();

    /* 赋值ht40禁止位 */
    *pc_command = g_wlan_cust.uc_disable_capab_2ght40;

    if (oal_copy_to_user(pst_ifr->ifr_data + BYTE_OFFSET_8, pc_command, (uint32_t)(st_priv_cmd.total_len))) {
        oam_error_log0(0, OAM_SF_ANY, "wal_vendor_priv_cmd: Failed to copy ioctl_data to user !");
        /* 返回错误，通知supplicant拷贝失败，supplicant侧做参数保护处理 */
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif
OAL_STATIC int32_t wal_ioctl_priv_cmd_vowifi_get_param(oal_net_device_stru *pst_net_dev,
    int8_t *pc_command, wal_wifi_priv_cmd_stru st_priv_cmd, oal_ifreq_stru *pst_ifr)
{
    int32_t l_ret;
    int32_t l_value = 0;
    int32_t *pl_value;
    int32_t l_memset_ret;

    l_ret = wal_ioctl_get_vowifi_param(pst_net_dev, pc_command, &l_value);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::CMD_VOWIFI_GET_MODE(%d) err [%d]!}", l_ret);
        return -OAL_EFAIL;
    }

    /* 将buf清零 */
    l_memset_ret = memset_s(pc_command, (uint32_t)(st_priv_cmd.total_len + 1), 0,
        (uint32_t)(st_priv_cmd.total_len + 1));
    if (l_memset_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_priv_cmd_vowifi_get_param::memset fail!");
    }
    pc_command[st_priv_cmd.total_len] = '\0';
    pl_value = (int32_t *)pc_command;
    *pl_value = l_value;

    if (oal_copy_to_user(pst_ifr->ifr_data + BYTE_OFFSET_8, pc_command, (uint32_t)(st_priv_cmd.total_len))) {
        oam_error_log0(0, OAM_SF_ANY, "wal_vendor_priv_cmd:CMD_VOWIFi_GET_MODE copy ioctl_data to user fail!");
        return -OAL_EFAIL;
    }

    return l_ret;
}
OAL_STATIC int32_t wal_ioctl_priv_cmd_get_wifi_priv_feature_capability(oal_net_device_stru *pst_net_dev,
    int8_t *pc_command, wal_wifi_priv_cmd_stru st_priv_cmd, oal_ifreq_stru *pst_ifr)
{
    int32_t l_ret;
    int32_t l_value = 0;
    int32_t *pl_value;
    int32_t l_memset_ret;

    l_ret = wal_ioctl_get_wifi_priv_feature_cap_param(pst_net_dev, pc_command, &l_value);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, 0, "{wal_vendor_priv_cmd::GET_WIFI_PRVI_FEATURE_CAPABILITY err:%d}", l_ret);
        return -OAL_EFAIL;
    }

    /* 将buf清零 */
    l_memset_ret = memset_s(pc_command, (uint32_t)(st_priv_cmd.total_len + 1), 0,
        (uint32_t)(st_priv_cmd.total_len + 1));
    if (l_memset_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_priv_cmd_get_wifi_priv_feature_capability::memset fail!");
    }
    pc_command[st_priv_cmd.total_len] = '\0';
    pl_value = (int32_t *)pc_command;
    *pl_value = l_value;

    oam_warning_log1(0, OAM_SF_ANY, "wal_vendor_priv_cmd::GET_WIFI_PRVI_FEATURE_CAPABILITY[%x]!", *pl_value);

    if (oal_copy_to_user(pst_ifr->ifr_data + BYTE_OFFSET_8, pc_command, (uint32_t)(st_priv_cmd.total_len))) {
        oam_error_log0(0, OAM_SF_ANY, "wal_vendor_priv_cmd:GET_WIFI_PRVI_FEATURE_CAPABILITY copy to user fail");
        return -OAL_EFAIL;
    }

    return l_ret;
}
OAL_STATIC int32_t wal_ioctl_priv_cmd_vowifi_is_support(int8_t *pc_command,
    wal_wifi_priv_cmd_stru st_priv_cmd, oal_ifreq_stru *pst_ifr, int32_t *l_memcpy_ret)
{
    int32_t cmd_len = CMD_VOWIFI_IS_SUPPORT_REPLY_LEN;
    int32_t ret_len;
    int32_t l_memset_ret;

    if ((uint32_t)st_priv_cmd.total_len < CMD_VOWIFI_IS_SUPPORT_REPLY_LEN) {
        oam_error_log1(0, 0, "{wal_vendor_priv_cmd::VOWIFI SUPPORT len[%d]}", st_priv_cmd.total_len);
        return -OAL_EFAIL;
    }
    /* 将buf清零 */
    ret_len = oal_max(st_priv_cmd.total_len, cmd_len);
    l_memset_ret = memset_s(pc_command, (uint32_t)(ret_len + 1), 0, (uint32_t)(ret_len + 1));
    if (l_memset_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_priv_cmd_vowifi_is_support::memset fail!");
    }
    pc_command[ret_len] = '\0';
    *l_memcpy_ret += memcpy_s(pc_command,
        (uint32_t)(ret_len + 1), CMD_VOWIFI_IS_SUPPORT_REPLY, CMD_VOWIFI_IS_SUPPORT_REPLY_LEN);
    if (*l_memcpy_ret != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_priv_cmd_vowifi_is_support::memcpy fail!");
    }
    if (oal_copy_to_user(pst_ifr->ifr_data + BYTE_OFFSET_8, pc_command, ret_len)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_vendor_priv_cmd:CMD_VOWIFI_IS_SUPPORT Failed to copy to user !");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
OAL_STATIC int32_t wal_ioctl_priv_cmd_filter_switch(int8_t *pc_command, wal_wifi_priv_cmd_stru st_priv_cmd)
{
    int32_t l_ret = OAL_SUCC;
#ifdef CONFIG_DOZE_FILTER
    int32_t l_on;
    uint32_t command_len = OAL_STRLEN(pc_command);

    /* 格式:FILTER 1 or FILTER 0 */
    l_ret = wal_ioctl_judge_input_param_length(st_priv_cmd, CMD_FILTER_SWITCH_LEN, 1);
    if (l_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::CMD_FILTER_SWITCH cmd len[%d] < 8}", command_len);

        return -OAL_EFAIL;
    }

    l_on = oal_atoi(pc_command + CMD_FILTER_SWITCH_LEN + 1);

    oam_warning_log1(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::CMD_FILTER_SWITCH %d.}", l_on);

    /* 调用内核接口调用 gWlanFilterOps.set_filter_enable */
    l_ret = hw_set_net_filter_enable(l_on);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::CMD_FILTER_SWITCH err [%d]!}", l_ret);
        return -OAL_EFAIL;
    }
#else
    oam_warning_log0(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::Not support CMD_FILTER_SWITCH.}");
#endif

    return l_ret;
}
#ifndef CONFIG_HAS_EARLYSUSPEND
OAL_STATIC int32_t wal_ioctl_priv_cmd_set_suspend_mode(oal_net_device_stru *pst_net_dev,
    int8_t *pc_command, wal_wifi_priv_cmd_stru st_priv_cmd)
{
    int32_t l_ret;

    l_ret = wal_ioctl_judge_input_param_length(st_priv_cmd, CMD_SETSUSPENDMODE_LEN, 1);
    if (l_ret != OAL_SUCC) {
        /* 2 cmdpara */
        oam_warning_log1(0, OAM_SF_ANY, "{CMD_SETSUSPENDMODE len err[%d]}", (CMD_SETSUSPENDMODE_LEN + 2));
        return -OAL_EFAIL;
    }
    l_ret = wal_ioctl_set_suspend_mode(pst_net_dev, *(pc_command + CMD_SETSUSPENDMODE_LEN + 1) - '0');

    return l_ret;
}
#endif

static int32_t wal_vendor_priv_cmd_expansion_sub_interface(oal_net_device_stru *pst_net_dev, int8_t *pc_command,
    wal_wifi_priv_cmd_stru *st_priv_cmd, oal_ifreq_stru *pst_ifr, int32_t *l_memcpy_ret)
{
    int32_t l_ret = OAL_SUCC;

    if (oal_strncasecmp(pc_command, CMD_VOWIFI_GET_PARAM, CMD_VOWIFI_GET_PARAM_LEN) == 0) {
        l_ret = wal_ioctl_priv_cmd_vowifi_get_param(pst_net_dev, pc_command, *st_priv_cmd, pst_ifr);
    } else if (oal_strncasecmp(pc_command, CMD_GET_WIFI_PRIV_FEATURE_CAPABILITY,
        CMD_GET_WIFI_PRIV_FEATURE_CAPABILITY_LEN) == 0) {
        l_ret = wal_ioctl_priv_cmd_get_wifi_priv_feature_capability(pst_net_dev, pc_command, *st_priv_cmd, pst_ifr);
    } else if (oal_strncasecmp(pc_command, CMD_VOWIFI_IS_SUPPORT, CMD_VOWIFI_IS_SUPPORT_LEN) == 0) {
        l_ret = wal_ioctl_priv_cmd_vowifi_is_support(pc_command, *st_priv_cmd, pst_ifr, l_memcpy_ret);
    } else if (oal_strncasecmp(pc_command, CMD_FILTER_SWITCH, CMD_FILTER_SWITCH_LEN) == 0) {
        l_ret = wal_ioctl_priv_cmd_filter_switch(pc_command, *st_priv_cmd);
#ifndef CONFIG_HAS_EARLYSUSPEND
    } else if (0 == oal_strncasecmp(pc_command, CMD_SETSUSPENDMODE, CMD_SETSUSPENDMODE_LEN)) {
        l_ret = wal_ioctl_priv_cmd_set_suspend_mode(pst_net_dev, pc_command, *st_priv_cmd);
#endif
    } else if (0 == oal_strncasecmp(pc_command, CMD_SET_STA_PM_ON, CMD_SET_STA_PM_ON_LEN)) {
        l_ret = wal_ioctl_set_sta_pm_on(pst_net_dev, pc_command, *st_priv_cmd);
    } else if (OAL_TRUE == wal_vendor_cmd_gather(pc_command)) {
        l_ret = wal_vendor_cmd_gather_handler(pst_net_dev, pc_command);
    } else if (0 == oal_strncasecmp(pc_command, CMD_RXFILTER_START, CMD_RXFILTER_START_LEN)) {
        wal_ioctl_force_stop_filter(pst_net_dev, OAL_FALSE);
    } else if (0 == oal_strncasecmp(pc_command, CMD_RXFILTER_STOP, CMD_RXFILTER_STOP_LEN)) {
        wal_ioctl_force_stop_filter(pst_net_dev, OAL_TRUE);
    } else if (0 == oal_strncasecmp(pc_command, CMD_GET_APF_PKTS_CNT, CMD_GET_APF_PKTS_CNT_LEN)) {
        l_ret = wal_ioctl_get_psm_stat(pst_net_dev, MAC_PSM_QUERY_FLT_STAT, pst_ifr);
        if (l_ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "wal_vendor_priv_cmd:CMD_GET_APF_PKTS_CNT Fail get psm stat:%d", l_ret);
            return -OAL_EFAIL;
        }
    } else if (0 == oal_strncasecmp(pc_command, CMD_SET_FASTSLEEP_SWITCH, CMD_SET_FASTSLEEP_SWITCH_LEN)) {
        l_ret = wal_ioctl_set_fastsleep_switch(pst_net_dev, pc_command);
    } else if (0 == oal_strncasecmp(pc_command, CMD_SET_TCP_ACK_CTL, CMD_SET_TCP_ACK_CTL_LEN)) {
        l_ret = wal_ioctl_tcp_ack_buf_switch(pst_net_dev, pc_command);
    } else {
        /* 圈复杂度限制，新增拓展私有命令接口 */
        l_ret = wal_vendor_priv_cmd_ext(pst_net_dev, pst_ifr, st_priv_cmd, pc_command);
    }

    return l_ret;
}

static int32_t wal_vendor_priv_cmd_expansion_interface(oal_net_device_stru *pst_net_dev, int8_t *pc_command,
    wal_wifi_priv_cmd_stru *st_priv_cmd, oal_ifreq_stru *pst_ifr, int32_t *l_memcpy_ret)
{
    int32_t l_ret;

    if (0 == oal_strncasecmp(pc_command, CMD_GET_CAPA_DBDC, CMD_GET_CAPA_DBDC_LEN)) {
        l_ret = wal_ioctl_priv_cmd_get_capa_dbdc(pc_command, *st_priv_cmd, l_memcpy_ret, pst_ifr);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    } else if (oal_strncasecmp(pc_command, CMD_TX_POWER, CMD_TX_POWER_LEN) == 0) {
        l_ret = wal_ioctl_priv_cmd_tx_power(pst_net_dev, pc_command, *st_priv_cmd);
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    } else if (oal_strncasecmp(pc_command, CMD_MEASURE_TAS_RSSI, CMD_MEASURE_TAS_RSSI_LEN) == 0) {
        l_ret = wal_ioctl_priv_cmd_measure_tas_rssi(pst_net_dev, pc_command, *st_priv_cmd);
    } else if (oal_strncasecmp(pc_command, CMD_SET_TAS_TXPOWER, CMD_SET_TAS_TXPOWER_LEN) == 0) {
        l_ret = wal_ioctl_priv_cmd_set_tas_txpower(pst_net_dev, pc_command);
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    } else if (oal_strncasecmp(pc_command, CMD_WPAS_GET_CUST, CMD_WPAS_GET_CUST_LEN) == 0) {
        l_ret = wal_ioctl_priv_cmd_wpas_get_gust(pst_ifr, pc_command, *st_priv_cmd);
#endif
#endif
    } else if (oal_strncasecmp(pc_command, CMD_VOWIFI_SET_PARAM, CMD_VOWIFI_SET_PARAM_LEN) == 0) {
        l_ret = wal_ioctl_set_vowifi_param(pst_net_dev, pc_command, st_priv_cmd);
        if (oal_unlikely(l_ret != OAL_SUCC)) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::VOWIFI_SET_PARAM err [%d]!}", l_ret);
            return -OAL_EFAIL;
        }
    } else {
        l_ret = wal_vendor_priv_cmd_expansion_sub_interface(pst_net_dev, pc_command,
            st_priv_cmd, pst_ifr, l_memcpy_ret);
    }

    return l_ret;
}

static int32_t wal_vendor_priv_cmd_start_traverse(oal_net_device_stru *net_dev, int8_t *command,
    wal_wifi_priv_cmd_stru *priv_cmd, oal_ifreq_stru *ifr, int32_t *memcpy_ret)
{
    int32_t ret;

    if (oal_strncasecmp(command, CMD_SET_AP_WPS_P2P_IE, CMD_SET_AP_WPS_P2P_IE_LEN) == 0) {
        ret = wal_ioctl_priv_cmd_set_ap_wps_p2p_ie(net_dev, command, *priv_cmd);
    } else if (oal_strncasecmp(command, CMD_SET_MLME_IE, CMD_SET_MLME_IE_LEN) == 0) {
        ret = wal_ioctl_priv_cmd_set_mlme_ie(net_dev, command, *priv_cmd);
    } else if (0 == oal_strncasecmp(command, CMD_MIRACAST_START, CMD_MIRACAST_START_LEN)) {
        oam_warning_log0(0, OAM_SF_M2S, "{wal_vendor_priv_cmd::Miracast start.}");
        ret = wal_ioctl_set_p2p_miracast_status(net_dev, OAL_TRUE);
    } else if (0 == oal_strncasecmp(command, CMD_MIRACAST_STOP, CMD_MIRACAST_STOP_LEN)) {
        oam_warning_log0(0, OAM_SF_M2S, "{wal_vendor_priv_cmd::Miracast stop.}");
        ret = wal_ioctl_set_p2p_miracast_status(net_dev, OAL_FALSE);
    } else if (oal_strncasecmp(command, CMD_P2P_SET_NOA, CMD_P2P_SET_NOA_LEN) == 0) {
        ret = wal_ioctl_priv_cmd_p2p_set_noa(net_dev, command, *priv_cmd, memcpy_ret);
    } else if (oal_strncasecmp(command, CMD_P2P_SET_PS, CMD_P2P_SET_PS_LEN) == 0) {
        ret = wal_ioctl_priv_cmd_p2p_set_ps(net_dev, command, *priv_cmd, memcpy_ret);
    } else if (0 == oal_strncasecmp(command, CMD_SET_POWER_ON, CMD_SET_POWER_ON_LEN)) {
        ret = wal_ioctl_priv_cmd_set_power_on(net_dev, command);
    } else if (0 == oal_strncasecmp(command, CMD_SET_POWER_MGMT_ON, CMD_SET_POWER_MGMT_ON_LEN)) {
        ret = wal_ioctl_priv_cmd_set_power_mgmt_on(command);
    } else if (0 == oal_strncasecmp(command, CMD_COUNTRY, CMD_COUNTRY_LEN)) {
        ret = wal_ioctl_priv_cmd_country(net_dev, command, *priv_cmd);
    } else {
        ret = wal_vendor_priv_cmd_expansion_interface(net_dev, command, priv_cmd, ifr, memcpy_ret);
    }

    return ret;
}


int32_t wal_vendor_priv_cmd(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr, int32_t cmd)
{
    wal_wifi_priv_cmd_stru priv_cmd;
    int8_t *command = NULL;
    int32_t l_ret;
    int32_t l_memcpy_ret = EOK;
    oal_bool_enum_uint8 temp_flag;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (!capable(CAP_NET_ADMIN)) {
        return -EPERM;
    }
#endif

    if (pst_ifr->ifr_data == NULL) {
        return -OAL_EINVAL;
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        oam_warning_log1(0, OAM_SF_ANY, "{dfr_process_status[%d]!}", g_st_dfr_info.bit_device_reset_process_flag);
        return OAL_SUCC;
    }
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR
    if (oal_copy_from_user((uint8_t *)&priv_cmd, pst_ifr->ifr_data, sizeof(wal_wifi_priv_cmd_stru))) {
        return -OAL_EINVAL;
    }

    temp_flag = (priv_cmd.total_len > MAX_PRIV_CMD_SIZE || priv_cmd.total_len < 0);
    if (temp_flag) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::private cmd len err:%d}", priv_cmd.total_len);
        return -OAL_EINVAL;
    }

    /* 申请内存保存wpa_supplicant 下发的命令和数据，5为申请的数据空间 */
    command = oal_memalloc((uint32_t)(priv_cmd.total_len + 5)); /* total len 为priv cmd 后面buffer 长度 */
    if (command == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::mem alloc failed.}");
        return -OAL_ENOMEM;
    }

    /* 拷贝wpa_supplicant 命令到内核态中，5为申请的数据空间 */
    memset_s(command, (uint32_t)(priv_cmd.total_len + 5), 0, (uint32_t)(priv_cmd.total_len + 5));

    l_ret = (int32_t)oal_copy_from_user(command, pst_ifr->ifr_data + BYTE_OFFSET_8, (uint32_t)(priv_cmd.total_len));
    if (l_ret != 0) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::oal_copy_from_user: -OAL_EFAIL }");
        l_ret = -OAL_EFAIL;
        oal_free(command);
        return l_ret;
    }
    command[priv_cmd.total_len] = '\0';

    if (!hmac_get_feature_switch(HMAC_MIRACAST_REDUCE_LOG_SWITCH)) {
        oam_warning_log2(0, OAM_SF_ANY, "{wal_vendor_priv_cmd::vendor private cmd total_len:%d, used_len:%d.}",
            priv_cmd.total_len, priv_cmd.used_len);
    }
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    if (oal_strncasecmp(command, CMD_SET_MEMO_CHANGE, CMD_SET_MEMO_CHANGE_LEN) == 0) {
        return wal_ioctl_priv_cmd_set_memo_change(command, priv_cmd);
    } else if (oal_strncasecmp(command, CMD_TAS_GET_ANT, CMD_TAS_GET_ANT_LEN) == 0) {
        oal_free(command);
        return board_get_wifi_tas_gpio_state();
    }
#endif

    l_ret = wal_vendor_priv_cmd_start_traverse(pst_net_dev, command, &priv_cmd, pst_ifr, &l_memcpy_ret);
    oal_free(command);
    if (((oal_unlikely(l_ret != OAL_SUCC)) || (l_memcpy_ret != EOK)) && (l_ret != OAL_FAIL)) {
        return -OAL_EFAIL;
    }

    return l_ret;
}


int32_t wal_vendor_priv_cmd_ext(oal_net_device_stru *pst_net_dev,
    oal_ifreq_stru *pst_ifr, wal_wifi_priv_cmd_stru *p_priv_cmd, uint8_t *pc_cmd)
{
    char fem_lower_cmd[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { "1" };
    int32_t l_ret = 0;

    if (0 == oal_strncasecmp(pc_cmd, CMD_SET_VHT160_FEM_LOWER, CMD_SET_VHT160_FEM_LOWER_LEN)) {
        /* 开启fem低功耗 */
        return wal_vendor_fem_lowpower(pst_net_dev, fem_lower_cmd, sizeof(fem_lower_cmd));
    } else if (0 == oal_strncasecmp(pc_cmd, CMD_GET_VHT160_SUPPORTED, CMD_GET_VHT160_SUPPORTED_LEN)) {
        return wal_vendor_get_160m_supported(pst_net_dev, pst_ifr);
    } else if (0 == oal_strncasecmp(pc_cmd, CMD_GET_AP_BANDWIDTH, CMD_GET_AP_BANDWIDTH_LEN)) {
        return wal_vendor_get_ap_bandwidth(pst_net_dev, pst_ifr);
    } else if (0 == oal_strncasecmp(pc_cmd, CMD_SET_DC_STATE, CMD_SET_DC_STATE_LEN)) {
        int32_t l_dc_status = -1;
        if (OAL_STRLEN(pc_cmd) < (CMD_SET_DC_STATE_LEN + 2)) { /* 2为空格和设置值的长度 */
            oam_error_log1(0, OAM_SF_ANY, "{wal_vendor_priv_cmd_ext::dc status,cmd len[%d]}", OAL_STRLEN(pc_cmd));
            return -OAL_EFAIL;
        }

        l_dc_status = oal_atoi(pc_cmd + CMD_SET_DC_STATE_LEN + 1);
        l_ret = wal_ioctl_set_dc_status(pst_net_dev, l_dc_status);
        if (l_ret != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "wal_vendor_priv_cmd_ext:dc status syn fail!");
            return -OAL_EFAIL;
        }
    } else if (0 == oal_strncasecmp(pc_cmd, CMD_SET_CALI_FEM_MODE, OAL_STRLEN(CMD_SET_CALI_FEM_MODE))) {
        return wal_vendor_set_cali_fem_mode(pst_net_dev, pst_ifr, pc_cmd);
#ifdef _PRE_WLAN_FEATURE_NRCOEX
    } else if (0 == oal_strncasecmp(pc_cmd, CMD_SET_NRCOEX_PRIOR, OAL_STRLEN(CMD_SET_NRCOEX_PRIOR))) {
        return wal_vendor_set_nrcoex_priority(pst_net_dev, pst_ifr, pc_cmd);
    } else if (0 == oal_strncasecmp(pc_cmd, CMD_GET_NRCOEX_INFO, OAL_STRLEN(CMD_GET_NRCOEX_INFO))) {
        return wal_vendor_get_nrcoex_info(pst_net_dev, pst_ifr, pc_cmd);
#endif
#ifdef _PRE_WLAN_FEATURE_PSM_ABN_PKTS_STAT
    } else if (0 == oal_strncasecmp(pc_cmd, CMD_GET_ABNORMAL_PKTS_CNT, CMD_GET_ABNORMAL_PKTS_CNT_LEN)) {
        return wal_ioctl_get_psm_stat(pst_net_dev, MAC_PSM_QUERY_ABNORMAL_PKTS_CNT, pst_ifr);
#endif
    } else {
        return wal_vendor_priv_cmd_ext2(pst_net_dev, pst_ifr, p_priv_cmd, pc_cmd);
    }

    /* 驱动对于不支持的命令，返回成功，否则上层wpa_supplicant认为ioctl失败，导致异常重启wifi */
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_M2S
#ifdef _PRE_WLAN_FEATURE_M2S_MODEM

uint32_t wal_vendor_cmd_modem_handle(oal_net_device_stru *pst_net_dev, int8_t *pc_command)
{
    wal_msg_write_stru  st_write_msg;
    uint8_t           uc_modem_mode;
    int32_t           l_ret;

    if (OAL_STRLEN(pc_command) < (OAL_STRLEN((int8_t *)CMD_SET_M2S_MODEM) + 2)) {  /* 2为空格和设置值的长度 */
        oam_warning_log0(0, OAM_SF_M2S, "{wal_vendor_cmd_modem_handle::CMD_SET_M2S_MODEM command len error}");
        return -OAL_EFAIL;
    }

    uc_modem_mode = (uint8_t)oal_atoi(pc_command + OAL_STRLEN((int8_t *)CMD_SET_M2S_MODEM) + 1);

    /* st_write_msg作清零操作 */
    memset_s(&st_write_msg, sizeof(wal_msg_write_stru), 0, sizeof(wal_msg_write_stru));

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_M2S_MODEM, sizeof(int32_t));
    *((int32_t *)(st_write_msg.auc_value)) = uc_modem_mode; /* 设置配置命令参数 */

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_vendor_cmd_modem_handle::wal_send_cfg_event return err = [%d].}", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}
#endif
#endif


uint32_t wal_vendor_cmd_gather_handler(oal_net_device_stru *pst_net_dev, int8_t *pc_command)
{
    int32_t l_ret = 0;

    if (0 == oal_strncasecmp(pc_command, CMD_SET_SOFTAP_MIMOMODE, OAL_STRLEN(CMD_SET_SOFTAP_MIMOMODE))) {
        if (OAL_STRLEN(pc_command) < OAL_STRLEN((int8_t *)CMD_SET_SOFTAP_MIMOMODE)) {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_vendor_cmd_gather_handler::CMD_SET_SOFTAP_MIMOMODE cmd len err}");
            return -OAL_EFAIL;
        }

        oam_warning_log1(0, OAM_SF_ANY, "{wal_vendor_cmd_gather_handler::CMD_SET_SOFTAP_MIMOMODE %d.}", OAL_TRUE);

        l_ret = wal_ioctl_set_ap_mode(pst_net_dev, OAL_TRUE);
        if (oal_unlikely(l_ret != OAL_SUCC)) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_vendor_cmd_gather_handler::CMD_SET_SOFTAP_MIMOMODE err[%d]}", l_ret);
            return -OAL_EFAIL;
        }
#ifdef _PRE_WLAN_FEATURE_M2S
    } else if (0 == oal_strncasecmp(pc_command, CMD_SET_M2S_SWITCH, OAL_STRLEN(CMD_SET_M2S_SWITCH))) {
        uint8_t uc_m2s_switch_on;

        if (OAL_STRLEN(pc_command) < (OAL_STRLEN((int8_t *)CMD_SET_M2S_SWITCH) + 2)) { /* 2为空格和设置值的长度 */
            oam_warning_log0(0, OAM_SF_M2S, "{wal_vendor_cmd_gather_handler::CMD_SET_M2S_SWITCH cmd len error.}");
            return -OAL_EFAIL;
        }

        uc_m2s_switch_on = (uint8_t)oal_atoi(pc_command + OAL_STRLEN((int8_t *)CMD_SET_M2S_SWITCH) + 1);

        oam_warning_log1(0, OAM_SF_M2S, "{wal_vendor_cmd_gather_handler::CMD_SET_M2S_SWITCH %d.}", uc_m2s_switch_on);

#ifdef _PRE_WLAN_FEATURE_M2S_MSS
        l_ret = wal_ioctl_set_m2s_mss(pst_net_dev, uc_m2s_switch_on);
        if (oal_unlikely(l_ret != OAL_SUCC)) {
            oam_warning_log1(0, OAM_SF_M2S, "{wal_vendor_cmd_gather_handler::CMD_SET_M2S_SWITCH err[%d]!}", l_ret);
            return -OAL_EFAIL;
        }
#endif
#ifdef _PRE_WLAN_FEATURE_M2S_MODEM
    } else if (g_wlan_spec_cfg->feature_m2s_modem_is_open &&
               0 == oal_strncasecmp(pc_command, CMD_SET_M2S_MODEM, OAL_STRLEN(CMD_SET_M2S_MODEM))) {
        l_ret = wal_vendor_cmd_modem_handle(pst_net_dev, pc_command);
        if (oal_unlikely(l_ret != OAL_SUCC)) {
            return -OAL_EFAIL;
        }
#endif
#endif
    }
    return l_ret;
}


uint8_t wal_vendor_cmd_gather(int8_t *pc_command)
{
    oal_bool_enum_uint8 en_cmd_is_invalid;

    if (g_wlan_spec_cfg->feature_m2s_modem_is_open) {
        en_cmd_is_invalid =
            ((0 == oal_strncasecmp(pc_command, CMD_SET_SOFTAP_MIMOMODE, OAL_STRLEN(CMD_SET_SOFTAP_MIMOMODE)))
#ifdef _PRE_WLAN_FEATURE_M2S
            || (0 == oal_strncasecmp(pc_command, CMD_SET_M2S_SWITCH, OAL_STRLEN(CMD_SET_M2S_SWITCH)))
#ifdef _PRE_WLAN_FEATURE_M2S_MODEM
            || (0 == oal_strncasecmp(pc_command, CMD_SET_M2S_MODEM, OAL_STRLEN(CMD_SET_M2S_MODEM)))
#endif
#endif
        );
    } else {
        en_cmd_is_invalid =
            ((0 == oal_strncasecmp(pc_command, CMD_SET_SOFTAP_MIMOMODE, OAL_STRLEN(CMD_SET_SOFTAP_MIMOMODE)))
#ifdef _PRE_WLAN_FEATURE_M2S
            || (0 == oal_strncasecmp(pc_command, CMD_SET_M2S_SWITCH, OAL_STRLEN(CMD_SET_M2S_SWITCH)))
#endif
        );
    }

    return en_cmd_is_invalid;
}

