
#ifdef _PRE_WLAN_FEATURE_NRCOEX
/* 1 头文件包含 */
#include "hmac_nrcoex.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_NRCOEX_C
/*
 * 函 数 名  : wal_ioctl_nrcoex_priority_set
 * 功能描述  : 设置NR共存WiFi优先级
 */
OAL_STATIC uint32_t wal_ioctl_nrcoex_priority_set(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    uint32_t cmd_id;
    uint32_t params[CMD_PARAMS_MAX_CNT] = {0};
    uint32_t offset;
    uint32_t ret;

    ret = wal_get_cmd_id("nrcoex_set_prior", &cmd_id, &offset);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_nrcoex_priority_set:find nrcoex_set_prior cmd is fail");
        return ret;
    }
    params[0] = pc_param[0];
    return wal_process_cmd_params(pst_net_dev, cmd_id, params);
}
/*
 * 函 数 名  : wal_vendor_set_nrcoex_priority
 * 功能描述  : NRCOEX上层下发设置优先级参数
 */
int32_t wal_vendor_set_nrcoex_priority(oal_net_device_stru *pst_net_dev,
    oal_ifreq_stru *pst_ifr, int8_t *pc_command)
{
    int32_t l_ret;
    int8_t nrcoex_priority;

    /* 格式:SET_CELLCOEX_PRIOR 0/1/2/3 */
    if (OAL_STRLEN(pc_command) < (OAL_STRLEN((int8_t *)CMD_SET_NRCOEX_PRIOR) + 2)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_vendor_set_nrcoex_priority::CMD_SET_NRCOEX_PRIOR len err[%d].}", OAL_STRLEN(pc_command));

        return -OAL_EFAIL;
    }
    nrcoex_priority = oal_atoi(pc_command + OAL_STRLEN((int8_t *)CMD_SET_NRCOEX_PRIOR) + 1);

    oam_warning_log1(0, OAM_SF_ANY,
        "{wal_vendor_set_nrcoex_priority::CMD_SET_NRCOEX_PRIOR cmd,nrcoex priority:%d}", nrcoex_priority);

    l_ret = (int32_t)wal_ioctl_nrcoex_priority_set(pst_net_dev, (int8_t *)&nrcoex_priority);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_vendor_set_nrcoex_priority::err [%d]!}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_ioctl_get_nrcoex_stat(oal_net_device_stru *pst_net_dev, int8_t *puc_stat)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    mac_vap_stru *pst_mac_vap = NULL;
    hmac_device_stru *pst_hmac_device = NULL;

    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "wal_ioctl_get_nrcoex_stat: pst_mac_vap get from netdev is null.");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "wal_ioctl_get_nrcoex_stat:hmac_device null. dev id:%d",
            pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(&st_write_msg, sizeof(st_write_msg), 0, sizeof(st_write_msg));
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_QUERY_NRCOEX_STAT, 0);

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH,
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "wal_ioctl_get_nrcoex_stat:send event fail:%d", l_ret);
    }

    pst_hmac_device->st_nrcoex_stat_query.en_query_completed_flag = OAL_FALSE;
    /*lint -e730 -e740 -e774*/
#ifndef _PRE_WINDOWS_SUPPORT
    l_ret = oal_wait_event_interruptible_timeout_m(pst_hmac_device->st_nrcoex_stat_query.st_wait_queue,
        (pst_hmac_device->st_nrcoex_stat_query.en_query_completed_flag == OAL_TRUE), 5 * OAL_TIME_HZ); /* 5 delay HZ */
#else
    oal_wait_event_interruptible_timeout_m(&pst_hmac_device->st_nrcoex_stat_query.st_wait_queue,
        (pst_hmac_device->st_nrcoex_stat_query.en_query_completed_flag == OAL_TRUE),
        5 * OAL_TIME_HZ, l_ret); /* 5 delay HZ */
#endif
    /*lint +e730 +e740 +e774*/
    if (l_ret <= 0) { /* 等待超时或异常 */
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "wal_ioctl_get_nrcoex_stat: timeout or err:%d", l_ret);
        return OAL_FAIL;
    }

    if (puc_stat != NULL) {
        if (EOK != memcpy_s(puc_stat, sizeof(mac_nrcoex_stat_stru),
                            (uint8_t *)&pst_hmac_device->st_nrcoex_stat_query.st_nrcoex_stat,
                            sizeof(mac_nrcoex_stat_stru))) {
            oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_get_nrcoex_stat::memcpy fail!");
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

int32_t wal_vendor_get_nrcoex_info(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr, int8_t *pc_cmd)
{
    int32_t l_ret;

    l_ret = wal_ioctl_get_nrcoex_stat(pst_net_dev, pc_cmd);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "wal_vendor_priv_cmd:CMD_GET_NRCOEX_INFO Failed to get nrcoex stat ret[%d] !", l_ret);
        return -OAL_EFAIL;
    }
    /* 2倍表示8个字节 */
    if (oal_copy_to_user(pst_ifr->ifr_data + BYTE_OFFSET_8, pc_cmd, 2 * sizeof(uint32_t))) {
        oam_error_log0(0, OAM_SF_ANY,
            "wal_vendor_priv_cmd:CMD_GET_NRCOEX_INFO Failed to copy ioctl_data to user !");
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
#endif
