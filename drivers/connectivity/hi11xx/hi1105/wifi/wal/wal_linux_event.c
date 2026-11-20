

#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "wal_linux_ioctl.h"
#include "wal_main.h"
#include "wal_config.h"
#include "wal_linux_scan.h"
#include "wal_linux_event.h"
#include "hmac_resource.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_EVENT_C


int32_t wal_cfg80211_start_req(oal_net_device_stru *pst_net_dev,
                                     const void *ps_param,
                                     uint16_t us_len,
                                     wlan_cfgid_enum_uint16 en_wid,
                                     oal_bool_enum_uint8 en_need_rsp)
{
    wal_msg_write_stru st_write_msg;
    wal_msg_stru *pst_rsp_msg = NULL;
    uint32_t err_code;
    int32_t l_ret;

    if (ps_param == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{wal_cfg80211_start_req::param is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写 msg 消息头 */
    st_write_msg.en_wid = en_wid;
    st_write_msg.us_len = us_len;

    /* 填写 msg 消息体 */
    if (us_len > WAL_MSG_WRITE_MAX_LEN) {
        oam_error_log2(0, OAM_SF_SCAN,
                       "{wal_cfg80211_start_req::us_len %d > WAL_MSG_WRITE_MAX_LEN %d err!}\r\n",
                       us_len, WAL_MSG_WRITE_MAX_LEN);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    if (EOK != memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value), ps_param, us_len)) {
        oam_error_log0(0, OAM_SF_SCAN, "wal_cfg80211_start_req::memcpy fail!");
        return -OAL_EFAIL;
    }

    /***************************************************************************
           抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_send_cfg_event(pst_net_dev,
        WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
        (uint8_t *)&st_write_msg,
        en_need_rsp,
        en_need_rsp ? &pst_rsp_msg : NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_SCAN,
                         "{wal_cfg80211_start_req::wal_send_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    if (en_need_rsp && (pst_rsp_msg != NULL)) {
        /* 读取返回的错误码 */
        err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
        if (err_code != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_SCAN,
                             "{wal_cfg80211_start_req::wal_send_cfg_event return err code:[%u]}",
                             err_code);
            return -OAL_EFAIL;
        }
    }

    return OAL_SUCC;
}


uint32_t wal_cfg80211_start_scan(oal_net_device_stru *pst_net_dev, mac_cfg80211_scan_param_stru *pst_scan_param)
{
    if (pst_scan_param == NULL) {
        oam_error_log0(0, OAM_SF_SCAN,
                       "{wal_cfg80211_start_scan::scan failed, null ptr, pst_scan_param = null.}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return (uint32_t)wal_cfg80211_start_req(pst_net_dev, pst_scan_param,
                                            sizeof(mac_cfg80211_scan_param_stru),
                                            WLAN_CFGID_CFG80211_START_SCAN, OAL_FALSE);
}


uint32_t wal_cfg80211_start_sched_scan(oal_net_device_stru *pst_net_dev, mac_pno_scan_stru *pst_pno_scan_info)
{
    mac_pno_scan_stru *pst_pno_scan_params;
    uint32_t ret;
    /* 申请pno调度扫描参数，此处申请hmac层释放 */
    pst_pno_scan_params = (mac_pno_scan_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL,
                                                               sizeof(mac_pno_scan_stru),
                                                               OAL_FALSE);
    if (pst_pno_scan_params == NULL) {
        oam_warning_log0(0, OAM_SF_SCAN,
                         "{wal_cfg80211_start_sched_scan::alloc pno scan param memory failed!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memcpy_s(pst_pno_scan_params, sizeof(mac_pno_scan_stru), pst_pno_scan_info, sizeof(mac_pno_scan_stru));

    ret = (uint32_t)wal_cfg80211_start_req(pst_net_dev, &pst_pno_scan_params,
                                           sizeof(mac_pno_scan_stru *),
                                           WLAN_CFGID_CFG80211_START_SCHED_SCAN, OAL_TRUE);

    /* 无论wal_cfg80211_start_req内部执行成功与否，统一在外部释放 */
    oal_mem_free_m(pst_pno_scan_params, OAL_FALSE);

    return ret;
}


int32_t wal_cfg80211_start_connect(oal_net_device_stru *pst_net_dev, hmac_conn_param_stru *pst_mac_conn_param)
{
    return wal_cfg80211_start_req(pst_net_dev,
                                  pst_mac_conn_param,
                                  sizeof(hmac_conn_param_stru),
                                  WLAN_CFGID_CFG80211_START_CONNECT,
                                  OAL_TRUE);
}


int32_t wal_cfg80211_start_disconnect(oal_net_device_stru *pst_net_dev,
                                      mac_cfg_kick_user_param_stru *pst_disconnect_param)
{
    mac_vap_stru *pst_mac_vap;
    hmac_device_stru *hmac_device = NULL;
    hmac_scan_record_stru *scan_record = NULL;

    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    // 终止可能存在的扫描
    if (pst_mac_vap != NULL && (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA)) {
        hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
        if (hmac_device == NULL) {
            oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                             "{wal_cfg80211_start_disconnect::hmac_device[%d] is null!}",
                             pst_mac_vap->uc_device_id);
            return -OAL_EINVAL;
        }
        /* abort漫游扫描，需要先把扫描回调函数置成null。否则可能先执行漫游入网，后删user，此时找不到user */
        scan_record = &hmac_device->st_scan_mgmt.st_scan_record_mgmt;
        if (scan_record->en_scan_mode == WLAN_SCAN_MODE_ROAM_SCAN) {
            scan_record->p_fn_cb = NULL;
        }
        if (hmac_device->st_scan_mgmt.pst_request == NULL) {
            /* 判断是否存在内部扫描，如果存在，也需要停止 */
            if ((hmac_device->st_scan_mgmt.en_is_scanning == OAL_TRUE) &&
                (pst_mac_vap->uc_vap_id == scan_record->uc_vap_id) &&
                (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN)) {
                scan_record->p_fn_cb = NULL;
            }
        }
        wal_force_scan_abort_then_scan_comp(pst_net_dev);
    }

    /* 注意 由于消息未真正处理就直接返回，导致WPA_SUPPLICANT继续下发消息，在驱动侧等到处理时被异常唤醒，导致后续下发的消息误以为操作失败，
       目前将去关联事件修改为等待消息处理结束后再上报，最后一个入参由OAL_FALSE改为OAL_TRUE */
    return wal_cfg80211_start_req(pst_net_dev, pst_disconnect_param,
                                  sizeof(mac_cfg_kick_user_param_stru), WLAN_CFGID_KICK_USER, OAL_TRUE);
}

#ifdef _PRE_WLAN_FEATURE_SAE

uint32_t wal_cfg80211_do_external_auth(oal_net_device_stru *pst_netdev, hmac_external_auth_req_stru *pst_ext_auth)
{
    return (uint32_t)wal_cfg80211_start_req(pst_netdev, pst_ext_auth,
                                            sizeof(*pst_ext_auth),
                                            WLAN_CFGID_CFG80211_EXTERNAL_AUTH, OAL_TRUE);
}
#endif /* _PRE_WLAN_FEATURE_SAE */
