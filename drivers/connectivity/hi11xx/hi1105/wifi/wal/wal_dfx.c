

#include "wlan_types.h"
#include "wal_dfx.h"
#include "oal_net.h"
#include "oal_cfg80211.h"
#include "oal_ext_if.h"
#include "frw_ext_if.h"

#include "hmac_ext_if.h"
#include "hmac_device.h"
#include "hmac_resource.h"
#include "hmac_ext_if.h"
#include "hmac_vap.h"
#include "hmac_p2p.h"
#include "wal_ext_if.h"
#include "wal_linux_cfg80211.h"
#include "wal_linux_scan.h"
#include "wal_linux_event.h"
#include "wal_ext_if.h"
#include "wal_config.h"
#include "wal_regdb.h"
#include "wal_linux_ioctl.h"
#include "mac_board.h"
#include "oam_ext_if.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif
#ifdef  _PRE_CONFIG_HISI_S3S4_POWER_STATE
#include "plat_pm.h"
#endif

#include "securec.h"
#include "wal_linux_netdev_ops.h"
#include "hmac_dfx.h"


#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_DFX_C

#ifdef _PRE_WLAN_FEATURE_DFR

#define DFR_WAIT_PLAT_FINISH_TIME (25000) /* 等待平台完成dfr工作的等待时间 */

int8_t *g_auc_dfr_error_type[] = {
    "AP",
    "STA",
    "P2P0",
    "GO",
    "CLIENT",
    "DFR UNKOWN ERR TYPE!!"
};

/* 此枚举为g_auc_dfr_error_type字符串的indx集合 */
typedef enum {
    DFR_ERR_TYPE_AP = 0,
    DFR_ERR_TYPE_STA,
    DFR_ERR_TYPE_P2P,
    DFR_ERR_TYPE_GO,
    DFR_ERR_TYPE_CLIENT,

    DFR_ERR_TYPE_BUTT
} wal_dfr_error_type;

#ifdef  _PRE_CONFIG_HISI_S3S4_POWER_STATE
wal_info_recovery_stru g_st_recovery_info;
#endif

struct st_wifi_dfr_callback *g_st_wifi_callback = NULL;

void wal_dfr_init_param(void);

#endif  // _PRE_WLAN_FEATURE_DFR

#ifdef _PRE_WLAN_FEATURE_DFR

OAL_STATIC void wal_dfr_kick_all_user(hmac_vap_stru *pst_hmac_vap)
{
    wal_msg_write_stru st_write_msg;
    wal_msg_stru *pst_rsp_msg = NULL;
    uint32_t err_code;
    int32_t l_ret;
    mac_cfg_kick_user_param_stru *pst_kick_user_param = NULL;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_KICK_USER, sizeof(mac_cfg_kick_user_param_stru));

    /* 设置配置命令参数 */
    pst_kick_user_param = (mac_cfg_kick_user_param_stru *)(st_write_msg.auc_value);
    oal_set_mac_addr(pst_kick_user_param->auc_mac_addr, BROADCAST_MACADDR);

    /* 填写去关联reason code */
    pst_kick_user_param->us_reason_code = MAC_UNSPEC_REASON;
    if (pst_hmac_vap->pst_net_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_dfr_kick_all_user::pst_net_device is null!}");
        return;
    }

    l_ret = wal_send_cfg_event(pst_hmac_vap->pst_net_device,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_kick_user_param_stru),
                               (uint8_t *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_dfr_kick_all_user::return err code [%d]!}\r\n", l_ret);
        return;
    }

    /* 处理返回消息 */
    err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_dfr_kick_all_user::hmac start vap fail,err code[%u]!}\r\n", err_code);
        return;
    }

    return;
}


uint32_t wal_process_p2p_excp(hmac_vap_stru *pst_hmac_vap)
{
    mac_vap_stru *pst_mac_vap;
    hmac_device_stru *pst_hmac_dev = NULL;

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_DFR,
        "{hmac_process_sta_excp::Now begin P2P recovery program, user[num:%d] when state is P2P0[%d]/CL[%d]/GO[%d].}",
        pst_mac_vap->us_user_nums,
        IS_P2P_DEV(pst_mac_vap),
        IS_P2P_CL(pst_mac_vap),
        IS_P2P_GO(pst_mac_vap));

    /* 删除用户 */
    wal_dfr_kick_all_user(pst_hmac_vap);

    /* AP模式还是STA模式 */
    if (IS_AP(pst_mac_vap)) {
        /* vap信息初始化 */
    } else if (IS_STA(pst_mac_vap)) {
        pst_hmac_dev = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
        if (pst_hmac_dev == NULL) {
            oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_DFR,
                           "{hmac_process_p2p_excp::pst_hmac_device is null, dev_id[%d].}",
                           pst_mac_vap->uc_device_id);

            return OAL_ERR_CODE_MAC_DEVICE_NULL;
        }
        /* 删除扫描信息列表，停止扫描 */
        if (pst_hmac_dev->st_scan_mgmt.st_scan_record_mgmt.uc_vap_id == pst_mac_vap->uc_vap_id) {
            pst_hmac_dev->st_scan_mgmt.st_scan_record_mgmt.p_fn_cb = NULL;
            pst_hmac_dev->st_scan_mgmt.en_is_scanning = OAL_FALSE;
        }
    }

    return OAL_SUCC;
}

uint32_t wal_process_ap_excp(hmac_vap_stru *pst_hmac_vap)
{
    mac_vap_stru *pst_mac_vap;

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_DFR,
                     "{hmac_process_sta_excp::Now begin AP exception recovery program, when AP have [%d] USERs.}",
                     pst_mac_vap->us_user_nums);

    /* 删除用户 */
    wal_dfr_kick_all_user(pst_hmac_vap);
    return OAL_SUCC;
}

uint32_t wal_process_sta_excp(hmac_vap_stru *pst_hmac_vap)
{
    mac_vap_stru *pst_mac_vap;
    hmac_device_stru *pst_hmac_dev;

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    pst_hmac_dev = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_dev == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_DFR,
                       "{hmac_process_sta_excp::pst_hmac_device is null, dev_id[%d].}",
                       pst_mac_vap->uc_device_id);

        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_DFR,
                     "{hmac_process_sta_excp::Now begin STA exception recovery program, when sta have [%d] users.}",
                     pst_mac_vap->us_user_nums);

    /* 关联状态下上报关联失败，删除用户 */
    wal_dfr_kick_all_user(pst_hmac_vap);

    /* 删除扫描信息列表，停止扫描 */
    if (pst_hmac_dev->st_scan_mgmt.st_scan_record_mgmt.uc_vap_id == pst_mac_vap->uc_vap_id) {
        pst_hmac_dev->st_scan_mgmt.st_scan_record_mgmt.p_fn_cb = NULL;
        pst_hmac_dev->st_scan_mgmt.en_is_scanning = OAL_FALSE;
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t wal_dfr_destroy_vap(oal_net_device_stru *pst_netdev)
{
    wal_msg_write_stru st_write_msg;
    wal_msg_stru *pst_rsp_msg = NULL;
    uint32_t err_code;
    int32_t l_ret;

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DESTROY_VAP, sizeof(int32_t));
    l_ret = wal_send_cfg_event(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t),
                               (uint8_t *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oal_io_print("DFR DESTROY_VAP[name:%s] fail, return[%d]!", pst_netdev->name, l_ret);
        oam_warning_log2(0, OAM_SF_DFR, "{wal_dfr_destroy_vap::DESTROY_VAP return err code [%d], iftype[%d]!}\r\n",
                         l_ret,
                         pst_netdev->ieee80211_ptr->iftype);

        return l_ret;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_dfr_destroy_vap::hmac add vap fail, err code[%u]!}\r\n", err_code);
        return err_code;
    }

    oal_net_dev_priv(pst_netdev) = NULL;
    oam_warning_log0(0, OAM_SF_DFR, "{wal_dfr_destroy_vap::finish!}\r\n");

    return OAL_SUCC;
}

#ifdef  _PRE_CONFIG_HISI_S3S4_POWER_STATE

uint32_t  wal_host_resume_work(void)
{
    uint32_t                    ret;
    int32_t                     l_ret;
    oal_net_device_stru          *pst_netdev = NULL;
    oal_wireless_dev_stru        *pst_wireless_dev = NULL;

    /* 恢复vap, 上报异常给上层 */
    for (; (g_st_recovery_info.netdev_num > 0 &&
        g_st_recovery_info.netdev_num < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT); g_st_recovery_info.netdev_num--) {
        ret = OAL_SUCC;
        pst_netdev = g_st_recovery_info.netdev[g_st_recovery_info.netdev_num - 1];

        if (pst_netdev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
            wal_custom_cali();
            hwifi_config_init_force();
#endif
            l_ret = wal_cfg_vap_h2d_event(pst_netdev);
            if (l_ret != OAL_SUCC) {
                oam_error_log1(0, OAM_SF_DFR, "wal_host_resume_work:Device cfg_vap creat false[%d]!", l_ret);
                return OAL_FAIL;
            }

            /* host device_stru初始化 */
            l_ret = wal_host_dev_init(pst_netdev);
            if (l_ret != OAL_SUCC) {
                oam_error_log1(0, OAM_SF_DFR, "wal_host_resume_work::wal_host_dev_init FAIL %d ", l_ret);
            }

            ret = wal_setup_ap(pst_netdev);
            oal_net_device_open(pst_netdev);
        } else if ((pst_netdev->ieee80211_ptr->iftype) == NL80211_IFTYPE_STATION ||
                (pst_netdev->ieee80211_ptr->iftype) == NL80211_IFTYPE_P2P_DEVICE) {
            l_ret = wal_netdev_open(pst_netdev, OAL_FALSE);
        } else {
            pst_wireless_dev = oal_netdevice_wdev(pst_netdev);

            oal_net_unregister_netdev(pst_netdev);
            oal_mem_free_m(pst_wireless_dev, OAL_TRUE);

            continue;
        }

        if (oal_unlikely(l_ret != OAL_SUCC) || oal_unlikely(l_ret != OAL_SUCC)) {
            oam_warning_log2(0, OAM_SF_ANY,
                             "{wal_host_resume_work:: Boot vap Failure, vap_iftype[%d], error_code[%d]!}\r\n",
                             pst_netdev->ieee80211_ptr->iftype, ((uint8_t)l_ret | (uint8_t)ret));
            continue;
        }
    }
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    wlan_pm_enable();
#endif
    g_st_recovery_info.netdev_num = 0;
    g_st_recovery_info.device_s3s4_process_flag = OAL_FALSE;

    return OAL_SUCC;
}
#endif


OAL_STATIC void wal_dfr_recovery_dev_timer(void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    /* WIFI DFR成功后调用hmac_wifi_state_notify重启device侧pps统计定时器 */
    if (pst_wlan_pm == NULL ||
        pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_open_notify == NULL) {
        oam_warning_log0(0, OAM_SF_DFR, "{wal_dfr_recovery_dev_timer:: Reinit device pps timer failed!}");
        return;
    }

    pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_open_notify(OAL_TRUE);
}


OAL_STATIC int32_t wal_dfr_recovery_all_vap(void)
{
    wal_dfr_error_type err_type = DFR_ERR_TYPE_BUTT;
    oal_wireless_dev_stru *wireless_dev = NULL;
    oal_net_device_stru *netdev = NULL;
    int32_t ret = OAL_SUCC;
    int32_t ret_event = OAL_SUCC;
    uint32_t ret_setup = OAL_SUCC;

    for (; (g_st_dfr_info.netdev_num > 0 && g_st_dfr_info.netdev_num < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT);
        g_st_dfr_info.netdev_num--) {
        netdev = g_st_dfr_info.past_netdev[g_st_dfr_info.netdev_num - 1];

        if (NL80211_IFTYPE_AP == netdev->ieee80211_ptr->iftype) {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
            wal_dfr_custom_cali();
            hwifi_config_init_force();
#endif
            ret_event = wal_cfg_vap_h2d_event(netdev);
            if (ret_event != OAL_SUCC) {
                oam_error_log1(0, OAM_SF_DFR, "wal_dfr_recovery_all_vap:DFR Device cfg_vap creat false %d!", ret_event);
                return OAL_FAIL;
            }

            /* host device_stru初始化 */
            ret = wal_host_dev_init(netdev);
            if (ret != OAL_SUCC) {
                oam_error_log1(0, OAM_SF_DFR, "wal_dfr_recovery_all_vap::DFR wal_host_dev_init FAIL %d ", ret);
            }

            ret_setup = wal_setup_ap(netdev);
            err_type = DFR_ERR_TYPE_AP;
            
            oal_net_device_open(netdev);
        } else if ((netdev->ieee80211_ptr->iftype == NL80211_IFTYPE_STATION) ||
            (netdev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_DEVICE)) {
            ret = wal_netdev_open(netdev, OAL_FALSE);
            err_type = (!OAL_STRCMP(netdev->name, "p2p0")) ? DFR_ERR_TYPE_P2P : DFR_ERR_TYPE_STA;
        } else {
            wireless_dev = oal_netdevice_wdev(netdev);

            /* 去注册netdev */
            oal_net_unregister_netdev(netdev);
            oal_mem_free_m(wireless_dev, OAL_TRUE);

            continue;
        }

        if (oal_unlikely(ret != OAL_SUCC) || oal_unlikely(ret_setup != OAL_SUCC)) {
            oal_io_print("DFR BOOT_VAP name:%sfail!error_code[%d]", netdev->name, ((uint8_t)ret | (uint8_t)ret_setup));
            oam_warning_log2(0, OAM_SF_ANY,
                             "{wal_dfr_recovery_all_vap:: Boot vap Failure, vap_iftype[%d], error_code[%d]!}\r\n",
                             netdev->ieee80211_ptr->iftype, ((uint8_t)ret | (uint8_t)ret_setup));
            continue;
        }
        /* 上报异常 */
        oal_cfg80211_rx_exception(netdev, (uint8_t *)g_auc_dfr_error_type[err_type],
                                  OAL_STRLEN(g_auc_dfr_error_type[err_type]));
    }

    return ret_event;
}


OAL_STATIC uint32_t wal_dfr_recovery_env(void)
{
    uint32_t timeleft;
    int32_t ret;

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
                         CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_RECV_LASTWORD);

    if (g_st_dfr_info.bit_ready_to_recovery_flag != OAL_TRUE) {
        return OAL_SUCC;
    }

    timeleft = oal_wait_for_completion_timeout(&g_st_dfr_info.st_plat_process_comp,
                                               oal_msecs_to_jiffies(DFR_WAIT_PLAT_FINISH_TIME));
    if (!timeleft) {
        oam_error_log1(0, OAM_SF_DFR, "wal_dfr_recovery_env:wait dev reset timeout[%d]", DFR_WAIT_PLAT_FINISH_TIME);
    }

    oam_warning_log2(0, OAM_SF_ANY, "wal_dfr_recovery_env: get plat_process_comp signal after[%u]ms, netdev_num=%d!",
                     (uint32_t)oal_jiffies_to_msecs(oal_msecs_to_jiffies(DFR_WAIT_PLAT_FINISH_TIME) - timeleft),
                     g_st_dfr_info.netdev_num);

    /* 恢复vap, 上报异常给上层 */
    ret = wal_dfr_recovery_all_vap();
    if (ret != OAL_SUCC) {
        return ret;
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    wlan_pm_enable();
    wal_dfr_recovery_dev_timer();
#endif

    g_st_dfr_info.bit_device_reset_process_flag = OAL_FALSE;
    g_st_dfr_info.bit_ready_to_recovery_flag = OAL_FALSE;
    g_st_dfr_info.netdev_num = 0;

    return OAL_SUCC;
}


static uint32_t wal_dfr_excp_process(mac_device_stru *mac_device, uint32_t exception_type)
{
    hmac_vap_stru *pst_hmac_vap = NULL;
    mac_vap_stru *mac_vap = NULL;
    uint8_t uc_vap_idx;
    int32_t l_ret;
    oal_net_device_stru *pst_netdev = NULL;
    oal_net_device_stru *pst_p2p0_netdev = NULL;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        oam_warning_log0(0, OAM_SF_DFR, "{wal_dfr_excp_process::pst_exception_data is NULL!}");
        return OAL_FAIL;
    }
    wlan_pm_disable_check_wakeup(OAL_FALSE);
#endif

    for (uc_vap_idx = mac_device->uc_vap_num; uc_vap_idx > 0; uc_vap_idx--) {
        /* 获取最右边一位为1的位数，此值即为vap的数组下标 */
        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_device->auc_vap_id[uc_vap_idx - 1]);
        if (pst_hmac_vap == NULL) {
            oam_warning_log1(0, OAM_SF_DFR,
                "{wal_dfr_excp_process::mac_res_get_hmac_vap fail!vap_idx = %u}\r",
                mac_device->auc_vap_id[uc_vap_idx - 1]);
            continue;
        }
        mac_vap = &(pst_hmac_vap->st_vap_base_info);
        pst_netdev = pst_hmac_vap->pst_net_device;

        if (IS_P2P_DEV(mac_vap)) {
            pst_netdev = pst_hmac_vap->pst_p2p0_net_device;
        } else if (IS_P2P_CL(mac_vap)) {
            pst_p2p0_netdev = pst_hmac_vap->pst_p2p0_net_device;
            if (pst_p2p0_netdev != NULL) {
                g_st_dfr_info.past_netdev[g_st_dfr_info.netdev_num] = pst_p2p0_netdev;
                g_st_dfr_info.netdev_num++;
            }
        }
        if (pst_netdev == NULL) {
            oam_warning_log1(0, OAM_SF_DFR,
                "{wal_dfr_excp_process::pst_netdev NULL!vap_idx = %u}\r", mac_device->auc_vap_id[uc_vap_idx - 1]);
            continue;
        } else if (pst_netdev->ieee80211_ptr == NULL) {
            oam_warning_log1(0, OAM_SF_DFR,
                "{wal_dfr_excp_process::ieee80211_ptr NULL !vap_idx = %u}\r", mac_device->auc_vap_id[uc_vap_idx - 1]);
            continue;
        }

        g_st_dfr_info.past_netdev[g_st_dfr_info.netdev_num] = pst_netdev;
        g_st_dfr_info.netdev_num++;

        oam_warning_log4(0, OAM_SF_DFR,
                         "wal_dfr_excp_process:vap_iftype [%d], vap_id[%d], vap_idx[%d], netdev_num[%d]",
                         pst_netdev->ieee80211_ptr->iftype, mac_vap->uc_vap_id, uc_vap_idx, g_st_dfr_info.netdev_num);

        wal_force_scan_complete(pst_netdev, OAL_TRUE);
        wal_stop_sched_scan(pst_netdev);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        mutex_lock(&g_st_dfr_info.wifi_excp_mutex);
#endif

        oal_net_device_close(pst_netdev);
        l_ret = wal_dfr_destroy_vap(pst_netdev);
        if (oal_unlikely(l_ret != OAL_SUCC)) {
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
            mutex_unlock(&g_st_dfr_info.wifi_excp_mutex);
#endif
            g_st_dfr_info.netdev_num--;
            continue;
        }

        if (pst_p2p0_netdev != NULL) {
            l_ret = wal_dfr_destroy_vap(pst_p2p0_netdev);
            if (oal_unlikely(l_ret != OAL_SUCC)) {
                oam_warning_log0(0, OAM_SF_DFR, "{wal_dfr_excp_process::DESTROY_P2P0 return err code [%d]!}\r\n");
                oal_net_unregister_netdev(pst_netdev);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
                mutex_unlock(&g_st_dfr_info.wifi_excp_mutex);
#endif
                g_st_dfr_info.netdev_num--;
                continue;
            }
            pst_p2p0_netdev = NULL;
        }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        mutex_unlock(&g_st_dfr_info.wifi_excp_mutex);
#endif
    }

#ifndef _PRE_LINUX_TEST
    /* wifi 下电过程和dfr过程中释放rx ring相关资源 */
    hal_device_reset_rx_res();
#endif

    /* device close& open */
    oal_init_completion(&g_st_dfr_info.st_plat_process_comp);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    plat_exception_handler(SUBSYS_WIFI, THREAD_WIFI, exception_type);
#endif

    // 开始dfr恢复动作: wal_dfr_recovery_env();
    g_st_dfr_info.bit_ready_to_recovery_flag = OAL_TRUE;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_queue_work(pst_exception_data->wifi_exception_workqueue, &pst_exception_data->wifi_excp_recovery_worker);
#endif

    return OAL_SUCC;
}

void wal_dfr_signal_complete(void)
{
    oal_complete(&g_st_dfr_info.st_plat_process_comp);
}


uint32_t wal_dfr_excp_rx(uint8_t uc_device_id, uint32_t exception_type)
{
    uint8_t uc_vap_idx;
    hmac_vap_stru *pst_hmac_vap = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    mac_device_stru *pst_mac_dev;

    pst_mac_dev = mac_res_get_dev(uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_error_log1(0, OAM_SF_DFR, "wal_dfr_excp_rx:ERROR dev_ID[%d] in DFR process!", uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 03暂作异常现场保留，不复位 */
    /*  异常复位开关是否开启 */
    if ((!g_st_dfr_info.bit_device_reset_enable) || g_st_dfr_info.bit_device_reset_process_flag) {
        oam_error_log1(0, OAM_SF_DFR, "wal_dfr_excp_rx:ERROR now in DFR process! bit_device_reset_process_flag=%d",
                       g_st_dfr_info.bit_device_reset_process_flag);
        return OAL_SUCC;
    }

    /* 和s3互斥，要dfr流程结束，才能开始s3;或者s3先来，要置bit_device_reset_process_flag，本次dfr退出 */
#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mutex_lock(&g_st_recovery_info.wifi_recovery_mutex);
    oam_warning_log0(0, OAM_SF_DFR, "wal_dfr_excp_rx:DFR process! get wifi_recovery_mutex!\n");
#endif
#endif

    g_st_dfr_info.bit_device_reset_process_flag = OAL_TRUE;
    g_st_dfr_info.bit_user_disconnect_flag = OAL_TRUE;
    g_st_dfr_info.dfr_num++;

    /* log现在进入异常处理流程 */
    oam_warning_log3(0, OAM_SF_DFR, "{wal_dfr_excp_rx:: Enter the exception processing[%d], type[%d] dev_ID[%d].}",
                     g_st_dfr_info.dfr_num, exception_type, uc_device_id);

    /* 按照每个vap模式进行异常处理 */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_dev->uc_vap_num; uc_vap_idx++) {
        /* 获取最右边一位为1的位数，此值即为vap的数组下标 */
        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_dev->auc_vap_id[uc_vap_idx]);
        if (pst_hmac_vap == NULL) {
            oam_warning_log2(0, OAM_SF_DFR, "{wal_dfr_excp_rx::mac_res_get_hmac_vap fail!vap_idx = %u dev_ID[%d]}\r",
                             pst_mac_dev->auc_vap_id[uc_vap_idx], uc_device_id);
            continue;
        }

        pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
        if (!IS_LEGACY_VAP(pst_mac_vap)) {
            wal_process_p2p_excp(pst_hmac_vap);
        } else if (IS_AP(pst_mac_vap)) {
            wal_process_ap_excp(pst_hmac_vap);
        } else if (IS_STA(pst_mac_vap)) {
            wal_process_sta_excp(pst_hmac_vap);
        } else {
            continue;
        }
    }

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                         CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_WIFI_RECOVERY);

    return wal_dfr_excp_process(pst_mac_dev, exception_type);
}

uint32_t wal_dfr_get_excp_type(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        oam_warning_log0(0, OAM_SF_DFR, "{wal_dfr_excp_process::pst_exception_data is NULL!}");
        return OAL_FAIL;
    }
    return pst_exception_data->wifi_excp_type;
#else
    return 0;
#endif
}

#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE
OAL_STATIC uint32_t  wal_host_suspend_del_vap(
    oal_net_device_stru *pst_netdev, oal_net_device_stru *pst_p2p0_netdev)
{
    int32_t   l_ret;

    wal_force_scan_complete(pst_netdev, OAL_TRUE);
    wal_stop_sched_scan(pst_netdev);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mutex_lock(&g_st_dfr_info.wifi_excp_mutex);
#endif

    oal_net_device_close(pst_netdev);
    l_ret = wal_dfr_destroy_vap(pst_netdev);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        mutex_unlock(&g_st_dfr_info.wifi_excp_mutex);
#endif
        g_st_recovery_info.netdev_num--;
        return OAL_FAIL;
    }

    if (pst_p2p0_netdev != NULL) {
        l_ret = wal_dfr_destroy_vap(pst_p2p0_netdev);
        if (oal_unlikely(l_ret != OAL_SUCC)) {
            oam_warning_log0(0, OAM_SF_DFR,
                "{wal_host_suspend_process::DESTROY_P2P0 return err code [%d]!}\r\n");
            oal_net_unregister_netdev(pst_netdev);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
            mutex_unlock(&g_st_dfr_info.wifi_excp_mutex);
#endif
            g_st_recovery_info.netdev_num--;
            return OAL_FAIL;
        }
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mutex_unlock(&g_st_dfr_info.wifi_excp_mutex);
#endif
    return OAL_SUCC;
}

uint32_t  wal_host_suspend_process(mac_device_stru *pst_mac_device)
{
    hmac_vap_stru               *pst_hmac_vap = NULL;
    mac_vap_stru                *pst_mac_vap = NULL;
    uint8_t                    uc_vap_idx;

    oal_net_device_stru          *pst_netdev = NULL;
    oal_net_device_stru          *pst_p2p0_netdev = NULL;

    for (uc_vap_idx = pst_mac_device->uc_vap_num; uc_vap_idx > 0; uc_vap_idx--) {
        /* 获取最右边一位为1的位数，此值即为vap的数组下标 */
        pst_p2p0_netdev = NULL;

        pst_hmac_vap    = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->auc_vap_id[uc_vap_idx - 1]);
        if (pst_hmac_vap == NULL) {
            oam_warning_log0(pst_mac_device->auc_vap_id[uc_vap_idx - 1], OAM_SF_DFR,
                "{wal_host_suspend_process::mac_res_get_hmac_vap fail!}\r");
            continue;
        }
        pst_mac_vap     = &(pst_hmac_vap->st_vap_base_info);
        pst_netdev      = pst_hmac_vap->pst_net_device;

        if (pst_netdev == NULL || pst_netdev->ieee80211_ptr == NULL) {
            oam_warning_log0(pst_mac_device->auc_vap_id[uc_vap_idx - 1], OAM_SF_DFR,
                "{wal_host_suspend_process::pst_netdev or ieee80211_ptr NULL pointer !vap_idx = %u}\r");
            continue;
        }

        if (IS_P2P_DEV(pst_mac_vap)) {
            pst_netdev = pst_hmac_vap->pst_p2p0_net_device;
        } else if (IS_P2P_CL(pst_mac_vap)) {
            pst_p2p0_netdev = pst_hmac_vap->pst_p2p0_net_device;
            if (pst_p2p0_netdev != NULL) {
                g_st_recovery_info.netdev[g_st_recovery_info.netdev_num]  = pst_p2p0_netdev;
                g_st_recovery_info.netdev_num++;
            }
        }

        g_st_recovery_info.netdev[g_st_recovery_info.netdev_num]  = pst_netdev;
        g_st_recovery_info.netdev_num++;

        oam_warning_log3(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_DFR,
            "wal_host_suspend_process:vap_type[%d], vap_idx[%d], netdev_num[%d]",
            pst_netdev->ieee80211_ptr->iftype, uc_vap_idx, g_st_recovery_info.netdev_num);

        if (wal_host_suspend_del_vap(pst_netdev, pst_p2p0_netdev) != OAL_SUCC) {
            continue;
        }
    }

    return OAL_SUCC;
}

uint32_t wal_host_suspend(uint8_t uc_device_id)
{
    uint8_t                     uc_vap_idx;
    hmac_vap_stru                *pst_hmac_vap = NULL;
    mac_vap_stru                 *pst_mac_vap = NULL;
    mac_device_stru              *pst_mac_dev;
    uint32_t                    ret;

    pst_mac_dev = mac_res_get_dev(uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_error_log1(0, OAM_SF_DFR, "wal_dfr_excp_rx:ERROR dev_ID[%d] in DFR process!", uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 和s3互斥，要dfr流程结束，才能开始s3;或者s3先来，要置bit_device_reset_process_flag，本次dfr退出 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mutex_lock(&g_st_recovery_info.wifi_recovery_mutex);
    oam_warning_log0(0, OAM_SF_DFR, "wal_host_suspend:S3S4 process! get wifi_recovery_mutex!\n");
#endif

    /* 按照每个vap模式进行异常处理 */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_dev->uc_vap_num; uc_vap_idx++) {
        /* 获取最右边一位为1的位数，此值即为vap的数组下标 */
        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_dev->auc_vap_id[uc_vap_idx]);
        if (pst_hmac_vap == NULL) {
            oam_warning_log1(pst_mac_dev->auc_vap_id[uc_vap_idx], OAM_SF_DFR,
                             "{wal_host_suspend::mac_res_get_hmac_vap fail!dev_ID[%d]}\r", uc_device_id);
            continue;
        }

        pst_mac_vap  = &(pst_hmac_vap->st_vap_base_info);
        if (!IS_LEGACY_VAP(pst_mac_vap)) {
            wal_process_p2p_excp(pst_hmac_vap);
        } else if (IS_AP(pst_mac_vap)) {
            wal_process_ap_excp(pst_hmac_vap);
        } else if (IS_STA(pst_mac_vap)) {
            wal_process_sta_excp(pst_hmac_vap);
        } else {
            continue;
        }
    }

    ret = wal_host_suspend_process(pst_mac_dev);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mutex_unlock(&g_st_recovery_info.wifi_recovery_mutex);
    oam_warning_log0(0, OAM_SF_DFR, "wal_host_suspend: S3S4 suspend finish! release wifi_recovery_mutex!\n");
#endif

    return ret;
}

uint32_t  wal_host_suspend_work(void)
{
    uint8_t  uc_device_index;
    int32_t  l_ret;

    g_st_recovery_info.netdev_num = 0;
    memset_s((uint8_t *)(g_st_recovery_info.netdev),
        sizeof(g_st_recovery_info.netdev[0]) * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT,
        0,
        sizeof(g_st_recovery_info.netdev[0]) * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT);

    g_st_recovery_info.device_s3s4_process_flag = OAL_TRUE;

    for (uc_device_index = 0; uc_device_index < MAC_RES_MAX_DEV_NUM; uc_device_index++) {
        l_ret = wal_host_suspend(uc_device_index);
    }
    memset_s(g_country_code_result, COUNTRY_CODE_LEN, 0, COUNTRY_CODE_LEN);
    return l_ret;
}
#endif


void wal_dfr_excp_work(oal_work_stru *work)
{
    uint32_t exception_type;
    uint8_t uc_device_index;

    exception_type = wal_dfr_get_excp_type();

    /* 暂不支持多chip */
    if (1 != WLAN_CHIP_MAX_NUM_PER_BOARD) {
        oam_error_log1(0, OAM_SF_DFR, "DFR Can not support muti_chip[%d].\n", WLAN_CHIP_MAX_NUM_PER_BOARD);
        return;
    }

    
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_dfr_excp_work:: dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return;
    }

    g_st_dfr_info.netdev_num = 0;
    memset_s((uint8_t *)(g_st_dfr_info.past_netdev),
             sizeof(g_st_dfr_info.past_netdev[0]) * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT, 0,
             sizeof(g_st_dfr_info.past_netdev[0]) * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT);

    for (uc_device_index = 0; uc_device_index < MAC_RES_MAX_DEV_NUM; uc_device_index++) {
        if (wal_dfr_excp_rx(uc_device_index, exception_type) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_dfr_excp_work::wal_send_cfg_event return err_code!}");
        }
    }
}
void wal_dfr_bfgx_excp(void)
{
    wal_dfr_excp_work(NULL);
}
void wal_dfr_recovery_work(oal_work_stru *work)
{
    uint32_t ret;

    ret = wal_dfr_recovery_env();
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_DFR, "wal_dfr_excp_rx:wal_dfr_recovery_env fail\n");
    }

#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mutex_unlock(&g_st_recovery_info.wifi_recovery_mutex);
    oam_warning_log0(0, OAM_SF_DFR, "wal_dfr_excp_rx:DFR recovery! release wifi_recovery_mutex!\n");
#endif
#endif
}

void wal_dfr_init_param(void)
{
    memset_s((uint8_t *)&g_st_dfr_info, sizeof(hmac_dfr_info_stru), 0, sizeof(hmac_dfr_info_stru));
#ifdef  _PRE_CONFIG_HISI_S3S4_POWER_STATE
    memset_s((uint8_t *)&g_st_recovery_info, sizeof(wal_info_recovery_stru),
             0, sizeof(wal_info_recovery_stru));
#endif

    g_st_dfr_info.bit_device_reset_enable = OAL_TRUE;
    g_st_dfr_info.bit_hw_reset_enable = OAL_FALSE;
    g_st_dfr_info.bit_soft_watchdog_enable = OAL_FALSE;
    g_st_dfr_info.bit_device_reset_process_flag = OAL_FALSE;
    g_st_dfr_info.bit_ready_to_recovery_flag = OAL_FALSE;
    g_st_dfr_info.bit_user_disconnect_flag = OAL_FALSE;
    g_st_dfr_info.excp_type = 0xffffffff;

    /* use mutex in wal_netdev_open and wal_netdev_stop in case DFR and WiFi on/off conflict,
     * don't use mutex in wal_dfr_excp_work, wal_dfr_recovery_work, and wal_hipriv_test_dfr_start */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mutex_init(&g_st_dfr_info.wifi_excp_mutex);
#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE
    mutex_init(&g_st_recovery_info.wifi_recovery_mutex);
    pm_host_walcb_register(wal_host_suspend_work, wal_host_resume_work);
#endif

#endif
}


void wal_dfr_custom_cali(void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    uint32_t ret;

    if (pst_wlan_pm == NULL) {
        oam_error_log0(0, OAM_SF_DFR, "{wal_dfr_custom_cali::pst_wlan_pm is null}");
        return;
    }
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    ret = wal_custom_cali();
    if (ret != OAL_SUCC) {
        return;
    }
#endif
    oal_init_completion(&pst_wlan_pm->pst_bus->st_device_ready);
    ret = oal_wait_for_completion_timeout(&pst_wlan_pm->pst_bus->st_device_ready,
                                          (uint32_t)oal_msecs_to_jiffies(HOST_WAIT_BOTTOM_INIT_TIMEOUT));
    if (ret == 0) {
        oam_error_log0(0, OAM_SF_DFR, "{wal_dfr_custom_cali::wait cali complete fail}");
    }
}

OAL_STATIC uint32_t wal_dfr_excp_init_handler(void)
{
#ifndef _PRE_LINUX_TEST
    uint8_t uc_device_index;
    hmac_device_stru *pst_hmac_dev = NULL;
    struct st_wifi_dfr_callback *pst_wifi_callback = NULL;
    struct st_exception_info *pst_exception_data = get_exception_info_reference();

    for (uc_device_index = 0; uc_device_index < MAC_RES_MAX_DEV_NUM; uc_device_index++) {
        pst_hmac_dev = hmac_res_get_mac_dev(uc_device_index);
        if (pst_hmac_dev == NULL) {
            oam_error_log1(0, OAM_SF_DFR, "wal_dfr_excp_init_handler:ERROR hmac dev_ID[%d] in DFR process!",
                uc_device_index);
            return OAL_ERR_CODE_PTR_NULL;
        }
    }
    /* 初始化dfr开关 */
    wal_dfr_init_param();

    /* 挂接调用钩子 */
    if (pst_exception_data != NULL) {
        oal_init_work(&pst_exception_data->wifi_excp_worker, wal_dfr_excp_work);
        oal_init_work(&pst_exception_data->wifi_excp_recovery_worker, wal_dfr_recovery_work);
        pst_exception_data->wifi_exception_workqueue = oal_create_singlethread_workqueue_m("wifi_exception_queue");
    }

    pst_wifi_callback = (struct st_wifi_dfr_callback *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL,
        sizeof(struct st_wifi_dfr_callback), OAL_TRUE);
    if (pst_wifi_callback == NULL) {
        oam_error_log1(0, OAM_SF_DFR, "wal_init_dev_excp_handler:can not alloc mem,size[%d]!",
            sizeof(struct st_wifi_dfr_callback));
        g_st_wifi_callback = NULL;
        return OAL_ERR_CODE_PTR_NULL;
    }
    g_st_wifi_callback = pst_wifi_callback;
    pst_wifi_callback->wifi_recovery_complete = wal_dfr_signal_complete;
    pst_wifi_callback->notify_wifi_to_recovery = wal_dfr_bfgx_excp;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    plat_wifi_exception_rst_register(pst_wifi_callback);
#endif

    oam_warning_log0(0, OAM_SF_DFR, "DFR wal_init_dev_excp_handler init ok.\n");
#endif
    return OAL_SUCC;
}


OAL_STATIC void wal_dfr_excp_exit_handler(void)
{
#ifndef _PRE_LINUX_TEST
    struct st_exception_info *pst_exception_data = get_exception_info_reference();

    if (pst_exception_data != NULL) {
        oal_cancel_work_sync(&pst_exception_data->wifi_excp_worker);
        oal_cancel_work_sync(&pst_exception_data->wifi_excp_recovery_worker);
        oal_destroy_workqueue(pst_exception_data->wifi_exception_workqueue);
    }
    oal_mem_free_m(g_st_wifi_callback, OAL_TRUE);

    oam_warning_log0(0, OAM_SF_DFR, "wal_dfr_excp_exit_handler::DFR dev_excp_handler remove ok.");
#endif
}
#else
uint32_t wal_dfr_excp_rx(uint8_t uc_device_id, uint32_t exception_type)
{
    return OAL_SUCC;
}

#endif  // _PRE_WLAN_FEATURE_DFR

uint32_t wal_dfx_init(void)
{
    uint32_t l_ret = OAL_SUCC; // 未初始化构建不过

#ifdef _PRE_WLAN_FEATURE_DFR
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    l_ret = init_dev_excp_handler();
    if (l_ret != OAL_SUCC) {
        return l_ret;
    }
#endif
    l_ret = wal_dfr_excp_init_handler();
#endif  // _PRE_WLAN_FEATURE_DFR

    return l_ret;
}

void wal_dfx_exit(void)
{
#ifdef _PRE_WLAN_FEATURE_DFR
    wal_dfr_excp_exit_handler();

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    deinit_dev_excp_handler();
#endif
#endif  // _PRE_WLAN_FEATURE_DFR
}

