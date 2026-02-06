

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "wal_linux_ioctl.h"
#include "wal_main.h"
#include "wal_config.h"
#include "wal_linux_scan.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_EVENT_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_int32 wal_cfg80211_start_req(oal_net_device_stru *pst_net_dev,
                                 OAL_CONST void *ps_param,
                                 oal_uint16 us_len,
                                 wlan_cfgid_enum_uint16 en_wid,
                                 oal_bool_enum_uint8 en_need_rsp)
{
    wal_msg_write_stru st_write_msg;
    wal_msg_stru *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32 ul_err_code;
    oal_int32 l_ret;

    if (ps_param == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{wal_cfg80211_start_req::param is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写 msg 消息头 */
    st_write_msg.en_wid = en_wid;
    st_write_msg.us_len = us_len;

    /* 填写 msg 消息体 */
    if (us_len > WAL_MSG_WRITE_MAX_LEN) {
        oam_error_log2(0, OAM_SF_SCAN, "{wal_cfg80211_start_req::us_len %d > WAL_MSG_WRITE_MAX_LEN %d err!}", us_len,
                       WAL_MSG_WRITE_MAX_LEN);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value), ps_param, us_len) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "wal_cfg80211_start_req::memcpy fail!");
        return -OAL_EFAIL;
    }

    /***************************************************************************
           抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               en_need_rsp,
                               en_need_rsp ? &pst_rsp_msg : OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{wal_cfg80211_start_req::wal_send_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    if (en_need_rsp && (pst_rsp_msg != OAL_PTR_NULL)) {
        /* 读取返回的错误码 */
        ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
        if (ul_err_code != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{wal_cfg80211_start_req::wal_send_cfg_event return err code:[%u]}",
                             ul_err_code);
            return -OAL_EFAIL;
        }
    }

    return OAL_SUCC;
}


oal_uint32 wal_cfg80211_start_scan(oal_net_device_stru *pst_net_dev, mac_cfg80211_scan_param_stru *pst_scan_param)
{
    if (pst_scan_param == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{wal_cfg80211_start_scan::scan failed, null ptr, pst_scan_param = null.}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return (oal_uint32)wal_cfg80211_start_req(pst_net_dev, pst_scan_param,
                                              OAL_SIZEOF(mac_cfg80211_scan_param_stru),
                                              WLAN_CFGID_CFG80211_START_SCAN, OAL_FALSE);
}


oal_uint32 wal_cfg80211_start_sched_scan(oal_net_device_stru *pst_net_dev, mac_pno_scan_stru *pst_pno_scan_info)
{
    mac_pno_scan_stru *pst_pno_scan_params;
    wal_msg_write_stru st_write_msg;
    wal_msg_stru *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32 ul_err_code;
    oal_int32 l_ret = 0;

    /* 申请pno调度扫描参数，此处申请hmac层释放 */
    pst_pno_scan_params = (mac_pno_scan_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(mac_pno_scan_stru),
                                                               OAL_FALSE);
    if (pst_pno_scan_params == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_SCAN, "{wal_cfg80211_start_sched_scan::alloc pno scan param memory failed!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memcpy_s(pst_pno_scan_params, OAL_SIZEOF(mac_pno_scan_stru), pst_pno_scan_info, OAL_SIZEOF(mac_pno_scan_stru));

    memset_s(&st_write_msg, OAL_SIZEOF(st_write_msg), 0, OAL_SIZEOF(st_write_msg));
    /* 填写 msg 消息头 */
    st_write_msg.en_wid = WLAN_CFGID_CFG80211_START_SCHED_SCAN;
    st_write_msg.us_len = OAL_SIZEOF(mac_pno_scan_stru *);

    /* 填写 msg 消息体 */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(mac_pno_scan_stru *),
                 &pst_pno_scan_params, OAL_SIZEOF(mac_pno_scan_stru *)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "wal_cfg80211_start_sched_scan::memcpy fail!");
        oal_mem_free_m(pst_pno_scan_params, OAL_TRUE);
        return OAL_FAIL;
    }

    /***************************************************************************
           抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_pno_scan_stru *),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);
    if ((l_ret != OAL_SUCC) && (-OAL_ETIMEDOUT != l_ret)) {
        oal_mem_free_m(pst_pno_scan_params, OAL_TRUE);
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{wal_cfg80211_start_sched_scan::wal_send_cfg_event return err code %d!}\r\n",
                         l_ret);
        return OAL_FAIL;
    }

    if (pst_rsp_msg != OAL_PTR_NULL) {
        /* 读取返回的错误码 */
        ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
        if (ul_err_code != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN,
                             "{wal_cfg80211_start_sched_scan::wal_check_and_release_msg_resp return err code:[%u]}",
                             ul_err_code);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}


oal_int32 wal_cfg80211_start_connect(oal_net_device_stru *pst_net_dev,
                                     mac_cfg80211_connect_param_stru *pst_mac_cfg80211_connect_param)
{
    return wal_cfg80211_start_req(pst_net_dev,
                                  pst_mac_cfg80211_connect_param,
                                  OAL_SIZEOF(mac_cfg80211_connect_param_stru),
                                  WLAN_CFGID_CFG80211_START_CONNECT,
                                  OAL_TRUE);
}


oal_int32 wal_cfg80211_start_disconnect(oal_net_device_stru *pst_net_dev,
                                        mac_cfg_kick_user_param_stru *pst_disconnect_param)
{
    mac_vap_stru *pst_mac_vap;
    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{wal_cfg80211_start_disconnect::pst_mac_vap is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    // 终止可能存在的扫描
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        wal_force_scan_complete_for_disconnect_scene(pst_net_dev);
    }

    /* 注意 由于消息未真正处理就直接返回，导致WPA_SUPPLICANT继续下发消息，在驱动侧等到处理时被异常唤醒，导致后续下发的消息误以为操作失败，
       目前将去关联事件修改为等待消息处理结束后再上报，最后一个入参由OAL_FALSE改为OAL_TRUE */
    return wal_cfg80211_start_req(pst_net_dev, pst_disconnect_param, OAL_SIZEOF(mac_cfg_kick_user_param_stru),
                                  WLAN_CFGID_KICK_USER, OAL_TRUE);
}

#ifdef _PRE_WLAN_FEATURE_SAE
/*
 * 函 数 名  : wal_cfg80211_do_external_auth
 * 功能描述  : 执行内核下发 ext_auth 命令到 hmac
 */
oal_uint32 wal_cfg80211_do_external_auth(oal_net_device_stru *pst_netdev,
                                         hmac_external_auth_req_stru *pst_ext_auth)
{
    return (oal_uint32)wal_cfg80211_start_req(pst_netdev, pst_ext_auth, OAL_SIZEOF(*pst_ext_auth),
                                              WLAN_CFGID_CFG80211_EXTERNAL_AUTH, OAL_TRUE);
}
#endif /* _PRE_WLAN_FEATURE_SAE */

