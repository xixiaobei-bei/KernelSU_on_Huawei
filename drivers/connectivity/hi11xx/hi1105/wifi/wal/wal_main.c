

#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "hmac_ext_if.h"
#include "wal_main.h"
#ifdef CONFIG_DOZE_FILTER
#include <huawei_platform/power/wifi_filter/wifi_filter.h>
#endif
#include "wal_config.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_cfg80211.h"
#include "oal_net.h"
#include "wal_linux_flowctl.h"
#include "wal_config_base.h"

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
#include "oal_kernel_file.h"
#endif

#include "wal_dfx.h"

#include "hmac_vap.h"
#include "wal_linux_atcmdsrv.h"
#include "securec.h"
#include "securectype.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_MAIN_C

/* HOST CRX子表 */
OAL_STATIC frw_event_sub_table_item_stru g_ast_wal_host_crx_table[WAL_HOST_CRX_SUBTYPE_BUTT];

/* HOST CTX字表 */
OAL_STATIC frw_event_sub_table_item_stru g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_BUTT];

/* HOST DRX子表 */
/* wal对外钩子函数 */
oam_wal_func_hook_stru g_st_wal_drv_func_hook;

oal_wakelock_stru g_st_wal_wakelock;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
wal_hw_wlan_filter_ops g_st_ip_filter_ops = {
    .set_filter_enable = wal_set_ip_filter_enable,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    .set_filter_enable_ex = wal_set_assigned_filter_enable,
#endif
    .add_filter_items = wal_add_ip_filter_items,
    .clear_filters = wal_clear_ip_filter,
    .get_filter_pkg_stat = NULL,
};
#else
wal_hw_wlan_filter_ops g_st_ip_filter_ops;
#endif


void wal_event_fsm_init_2(void)
{
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_TAS_NOTIFY_RSSI].p_func = wal_cfg80211_tas_rssi_access_report;
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_HID2D_SEQNUM].p_func = wal_cfg80211_hid2d_seqnum_report;
#endif
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_EXT_AUTH_REQ].p_func = wal_report_external_auth_req;

    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_CH_SWITCH_NOTIFY].p_func = wal_report_channel_switch;
#ifdef _PRE_WLAN_FEATURE_NAN
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_NAN_RSP].p_func = wal_nan_response_event_process;
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_RTT_RESULT_RPT].p_func = wal_report_rtt_result;
#endif
}

void wal_event_fsm_init(void)
{
    g_ast_wal_host_crx_table[WAL_HOST_CRX_SUBTYPE_CFG].p_func = wal_config_process_pkt;
    frw_event_table_register(FRW_EVENT_TYPE_HOST_CRX, FRW_EVENT_PIPELINE_STAGE_0, g_ast_wal_host_crx_table);

    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_SCAN_COMP_STA].p_func = wal_scan_comp_proc_sta;
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_ASOC_COMP_STA].p_func = wal_asoc_comp_proc_sta;
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_DISASOC_COMP_STA].p_func = wal_disasoc_comp_proc_sta;
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_STA_CONNECT_AP].p_func = wal_connect_new_sta_proc_ap;
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_STA_DISCONNECT_AP].p_func = wal_disconnect_sta_proc_ap;
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_MIC_FAILURE].p_func = wal_mic_failure_proc;
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_RX_MGMT].p_func = wal_send_mgmt_to_host;
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_LISTEN_EXPIRED].p_func = wal_p2p_listen_timeout;

    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_INIT].p_func = wal_cfg80211_init_evt_handle;
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_MGMT_TX_STATUS].p_func = wal_cfg80211_mgmt_tx_status;

    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_ROAM_COMP_STA].p_func = wal_roam_comp_proc_sta;
#ifdef _PRE_WLAN_FEATURE_11R
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_FT_EVENT_STA].p_func = wal_ft_event_proc_sta;
#endif  // _PRE_WLAN_FEATURE_11R
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_VOWIFI_REPORT].p_func = wal_cfg80211_vowifi_report;

#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_SAMPLE_REPORT].p_func = wal_sample_report2sdt;
#endif
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_DPD].p_func = wal_dpd_report2sdt;
#ifdef _PRE_WLAN_FEATURE_DFS
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_CAC_REPORT].p_func = wal_cfg80211_cac_report;
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
    g_ast_wal_host_ctx_table[HMAC_HOST_CTX_EVENT_SUB_TYPE_M2S_STATUS].p_func = wal_cfg80211_m2s_status_report;
#endif
    wal_event_fsm_init_2();

    frw_event_table_register(FRW_EVENT_TYPE_HOST_CTX, FRW_EVENT_PIPELINE_STAGE_0, g_ast_wal_host_ctx_table);

    return;
}


void wal_event_fsm_exit(void)
{
    memset_s(g_ast_wal_host_crx_table, sizeof(g_ast_wal_host_crx_table),
             0, sizeof(g_ast_wal_host_crx_table));

    memset_s(g_ast_wal_host_ctx_table, sizeof(g_ast_wal_host_ctx_table),
             0, sizeof(g_ast_wal_host_ctx_table));

    return;
}

#if defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT)

/* debug sysfs */
OAL_STATIC oal_kobject *g_conn_syfs_wal_object = NULL;

int32_t wal_wakelock_info_print(char *buf, int32_t buf_len)
{
    int32_t ret = 0;

#ifdef CONFIG_PRINTK
    if (g_st_wal_wakelock.locked_addr) {
        ret += snprintf_s(buf + ret, buf_len - ret, (buf_len - ret) - 1, "wakelocked by:%pf\n",
                          (void *)g_st_wal_wakelock.locked_addr);
    }
#endif

    ret += snprintf_s(buf + ret, buf_len - ret, (buf_len - ret) - 1,
                      "hold %lu locks\n", g_st_wal_wakelock.lock_count);

    return ret;
}

OAL_STATIC ssize_t wal_get_wakelock_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret += wal_wakelock_info_print(buf, PAGE_SIZE - ret);

    return ret;
}

OAL_STATIC ssize_t wal_get_packet_statistics_wlan0_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    ssize_t ret = 0;
    oal_net_device_stru *pst_net_dev = NULL;
    mac_vap_stru *pst_vap = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    int32_t l_rx_pckg_succ_num, l_ret;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    pst_net_dev = wal_config_get_netdev("wlan0", OAL_STRLEN("wlan0"));
    if (pst_net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY,
                         "{wal_packet_statistics_wlan0_info_print::wal_config_get_netdev return null ptr!}\r\n");
        return ret;
    }
    oal_dev_put(pst_net_dev); /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    /* 获取VAP结构体 */
    pst_vap = (mac_vap_stru *)oal_net_dev_priv(pst_net_dev);
    /* 如果VAP结构体不存在，返回0 */
    if (oal_unlikely(pst_vap == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_packet_statistics_wlan0_info_print::pst_vap = NULL!}\r\n");
        return ret;
    }
    /* 非STA直接返回 */
    if (pst_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{wal_packet_statistics_wlan0_info_print::vap_mode:%d.}\r\n", pst_vap->en_vap_mode);
        return ret;
    }
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_vap->uc_vap_id, OAM_SF_CFG, "{wal_packet_statistics_wlan0_info_print::pst_hmac_vap null.}");
        return ret;
    }

    l_ret = wal_atcmsrv_ioctl_get_rx_pckg_old(pst_net_dev, &l_rx_pckg_succ_num);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_packet_statistics_wlan0_info_print::wal_atcmsrv_ioctl_get_rx_pckg fail, failed err:%d!}", l_ret);
        return ret;
    }

    ret += snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "rx_packet=%d\r\n", l_rx_pckg_succ_num);

    return ret;
}

#ifndef _PRE_WINDOWS_SUPPORT
OAL_STATIC struct kobj_attribute g_dev_attr_wakelock =
    __ATTR(wakelock, S_IRUGO, wal_get_wakelock_info, NULL);
OAL_STATIC struct kobj_attribute g_dev_attr_packet_statistics_wlan0 =
    __ATTR(packet_statistics_wlan0, S_IRUGO, wal_get_packet_statistics_wlan0_info, NULL);
#endif

int32_t wal_msg_queue_info_print(char *buf, int32_t buf_len)
{
    int32_t ret = 0;

    ret += snprintf_s(buf + ret, buf_len - ret, (buf_len - ret) - 1,
                      "message count:%u\n", wal_get_request_msg_count());

    return ret;
}

OAL_STATIC ssize_t wal_get_msg_queue_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret += wal_msg_queue_info_print(buf, PAGE_SIZE - ret);

    return ret;
}
OAL_STATIC ssize_t wal_get_dev_wifi_info_print(char *buf, int32_t buf_len)
{
    ssize_t ret = 0;
    oal_net_device_stru *pst_net_dev = NULL;
    mac_vap_stru *pst_vap = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    pst_net_dev = wal_config_get_netdev("wlan0", OAL_STRLEN("wlan0"));
    if (pst_net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_get_dev_wifi_info_print::wal_config_get_netdev return null ptr!}\r\n");
        return ret;
    }
    oal_dev_put(pst_net_dev); /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    /* 获取VAP结构体 */
    pst_vap = (mac_vap_stru *)oal_net_dev_priv(pst_net_dev);
    /* 如果VAP结构体不存在，返回0 */
    if (oal_unlikely(pst_vap == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_get_dev_wifi_info_print::pst_vap = NULL!}\r\n");
        return ret;
    }
    /* 非STA直接返回 */
    if (pst_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_get_dev_wifi_info_print::vap_mode:%d.}\r\n", pst_vap->en_vap_mode);
        return ret;
    }
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_vap->uc_vap_id, OAM_SF_CFG, "{wal_get_dev_wifi_info_print::pst_hmac_vap null.}");
        return ret;
    }

    ret += snprintf_s(buf + ret, buf_len - ret, (buf_len - ret) - 1,
                      "tx_frame_amount:%d\n", pst_hmac_vap->station_info.tx_packets);
    ret += snprintf_s(buf + ret, buf_len - ret, (buf_len - ret) - 1,
                      "tx_byte_amount:%d\n", (uint32_t)pst_hmac_vap->station_info.tx_bytes);
    ret += snprintf_s(buf + ret, buf_len - ret, (buf_len - ret) - 1,
                      "tx_data_frame_error_amount:%d\n", pst_hmac_vap->station_info.tx_failed);
    ret += snprintf_s(buf + ret, buf_len - ret, (buf_len - ret) - 1,
                      "tx_retrans_amount:%d\n", pst_hmac_vap->station_info.tx_retries);
    ret += snprintf_s(buf + ret, buf_len - ret, (buf_len - ret) - 1,
                      "rx_frame_amount:%d\n", pst_hmac_vap->station_info.rx_packets);
    ret += snprintf_s(buf + ret, buf_len - ret, (buf_len - ret) - 1,
                      "rx_byte_amount:%d\n", (uint32_t)pst_hmac_vap->station_info.rx_bytes);
    ret += snprintf_s(buf + ret, buf_len - ret, (buf_len - ret) - 1,
                      "rx_beacon_from_assoc_ap:%d\n", pst_hmac_vap->st_station_info_extend.bcn_cnt);
    ret += snprintf_s(buf + ret, buf_len - ret, (buf_len - ret) - 1,
                      "ap_distance:%d\n", pst_hmac_vap->st_station_info_extend.uc_distance);
    ret += snprintf_s(buf + ret, buf_len - ret, (buf_len - ret) - 1,
                      "disturbing_degree:%d\n", pst_hmac_vap->st_station_info_extend.uc_cca_intr);
    ret += snprintf_s(buf + ret, buf_len - ret, (buf_len - ret) - 1,
                      "lost_beacon_amount:%d\n", pst_hmac_vap->st_station_info_extend.bcn_tout_cnt);

    return ret;
}
OAL_STATIC ssize_t wal_get_dev_wifi_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret += wal_get_dev_wifi_info_print(buf, PAGE_SIZE - ret);

    return ret;
}
#ifdef _PRE_WLAN_FEATURE_PC_KK_DEMAND
#define NETCARD_INFO 2
#define NETCARD_TYPE 5
#define HI1103_11AC_MCS9_160M_2STREAMS_RATE "1.73Gbps"
#define HI1105_11AX_MCS9_160M_2STREAMS_RATE "1.92Gbps"
#define HI1106_11AX_MCS9_160M_4STREAMS_RATE "3.84Gbps"

OAL_STATIC ssize_t wal_get_wifi_devices_info_print(char *buf, int32_t buf_len)
{
    ssize_t ret;
    int32_t chip_type = get_hi110x_subchip_type();
    char *netcard_info[][NETCARD_INFO] = {
        { "Hi1102", "NULL" }, { "Hi1103", HI1103_11AC_MCS9_160M_2STREAMS_RATE },
        { "Hi1102a", "NULL" }, { "Hi1105", HI1105_11AX_MCS9_160M_2STREAMS_RATE },
        { "Hi1106", HI1106_11AX_MCS9_160M_4STREAMS_RATE }
    };

    if (chip_type < 0) {
        ret = snprintf_s(buf, buf_len, buf_len - 1, "This Chip is not supported\n");
        return ret;
    }

    ret = snprintf_s(buf, buf_len, buf_len - 1,
                     "Chip Type: HUAWEI %s\n" \
                     "NIC  Type: WLAN\n" \
                     "NIC  Rate(theory): %s\n", netcard_info[chip_type][0], netcard_info[chip_type][1]);

    return ret;
}

OAL_STATIC ssize_t wal_get_wifi_devices_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret = wal_get_wifi_devices_info_print(buf, PAGE_SIZE);

    return ret;
}

OAL_STATIC ssize_t wal_get_bt_wifi_device_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    ssize_t ret;
    ssize_t buf_used_len;
    int32_t buf_len = PAGE_SIZE;
    int32_t chip_type = get_hi110x_subchip_type();
    const char *netcard_info[NETCARD_TYPE] = {
        "Hi1102", "Hi1103", "Hi1102a", "Hi1105", "Hi1106"
    };

    if ((buf == NULL) || (attr == NULL) || (dev == NULL)) {
        oal_io_print("param error r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (chip_type < 0) {
        ret = snprintf_s(buf, buf_len, buf_len - 1, "This Chip is not supported\n");
        return ret;
    }

    ret = snprintf_s(buf, buf_len, buf_len - 1,
                     "WiFi:\n" \
                     "Name: %s\n" \
                     "Type: WLAN Device\n" \
                     "Manufacturer: HISILICON\n", netcard_info[chip_type]);
    if (ret < 0) {
        ret = snprintf_s(buf, buf_len, buf_len - 1, "Wi-Fi info prints fail\n");
        return ret;
    }
    buf_used_len = ret;
    ret = snprintf_s(buf + buf_used_len, buf_len - buf_used_len, (buf_len - buf_used_len) - 1,
                     "BT:\n" \
                     "Name: %s\n" \
                     "Type: Bluetooth Device\n" \
                     "Manufacturer: HISILICON\n", netcard_info[chip_type]);
    if (ret < 0) {
        ret = snprintf_s(buf + buf_used_len, buf_len - buf_used_len, (buf_len - buf_used_len) - 1,
                         "BT info prints fail\n");
        return buf_used_len;
    }
    buf_used_len += ret;
    return buf_used_len;
}
#endif

OAL_STATIC ssize_t wal_show_country_code(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    char *country_code = NULL;
    int ret;
    char ibuf[COUNTRY_CODE_LEN] = {0};

    if (buf == NULL) {
        oal_io_print("buf is NULL\n");
        return -OAL_EFAIL;
    }

    country_code = hwifi_get_country_code();
    if (strncmp(country_code, "99", strlen("99")) == 0) {
        ret = get_cust_conf_string(INI_MODU_WIFI, STR_COUNTRY_CODE, ibuf, sizeof(ibuf));
        if (ret == INI_SUCC) {
            ret = strncpy_s(buf, PAGE_SIZE, ibuf, COUNTRY_CODE_LEN);
            if (ret != EOK) {
                oal_io_print("strncpy_s error, please check!\n");
                return -OAL_EFAIL;
            }
            buf[COUNTRY_CODE_LEN - 1] = '\0';
            return strlen(buf);
        } else {
            oal_io_print("get dts country_code error\n");
            return 0;
        }
    } else {
        return snprintf_s(buf, PAGE_SIZE, COUNTRY_CODE_LEN, "%s", country_code);
    }
}

#ifndef _PRE_WINDOWS_SUPPORT
OAL_STATIC struct kobj_attribute g_dev_attr_dev_wifi_info =
    __ATTR(dev_wifi_info, S_IRUGO, wal_get_dev_wifi_info, NULL);

OAL_STATIC struct kobj_attribute g_dev_attr_msg_queue =
    __ATTR(msg_queue, S_IRUGO, wal_get_msg_queue_info, NULL);

OAL_STATIC struct kobj_attribute g_country_code =
    __ATTR(country_code, S_IRUGO, wal_show_country_code, NULL);
#endif

#ifdef _PRE_WLAN_FEATURE_PC_KK_DEMAND
#ifndef _PRE_WINDOWS_SUPPORT
OAL_STATIC struct kobj_attribute g_dev_attr_wifi_devices_info =
    __ATTR(wifi_devices_info, S_IRUGO, wal_get_wifi_devices_info, NULL);
OAL_STATIC struct kobj_attribute g_dev_attr_bt_wifi_device_info =
    __ATTR(bt_wifi_device_info, S_IRUGO, wal_get_bt_wifi_device_info, NULL);
#endif
#endif

OAL_STATIC struct attribute *g_wal_sysfs_entries[] = {
    &g_dev_attr_wakelock.attr,
    &g_dev_attr_msg_queue.attr,
    &g_dev_attr_packet_statistics_wlan0.attr,
    &g_dev_attr_dev_wifi_info.attr,
#ifdef _PRE_WLAN_FEATURE_PC_KK_DEMAND
    &g_dev_attr_wifi_devices_info.attr,
    &g_dev_attr_bt_wifi_device_info.attr,
#endif
    &g_country_code.attr,
    NULL
};

OAL_STATIC struct attribute_group g_wal_attribute_group = {
    // .name = "vap",
    .attrs = g_wal_sysfs_entries,
};

OAL_STATIC void wal_sysfs_entry_init(void)
{
    int32_t ret;
    oal_kobject *pst_root_object = NULL;
    pst_root_object = oal_get_sysfs_root_object();
    if (pst_root_object == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_sysfs_entry_init::get sysfs root object failed!}");
        return;
    }

    g_conn_syfs_wal_object = kobject_create_and_add("wal", pst_root_object);
    if (g_conn_syfs_wal_object == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_sysfs_entry_init::create wal object failed!}");
        return;
    }

    ret = sysfs_create_group(g_conn_syfs_wal_object, &g_wal_attribute_group);
    if (ret) {
        kobject_put(g_conn_syfs_wal_object);
        oam_error_log0(0, OAM_SF_ANY, "{wal_sysfs_entry_init::sysfs create group failed!}");
        return;
    }
    return;
}

OAL_STATIC void wal_sysfs_entry_exit(void)
{
    if (g_conn_syfs_wal_object) {
        sysfs_remove_group(g_conn_syfs_wal_object, &g_wal_attribute_group);
        kobject_put(g_conn_syfs_wal_object);
    }
    return;
}
#endif


int32_t wal_main_init(void)
{
    uint32_t ret;
    uint16_t en_init_state;

    oal_wake_lock_init(&g_st_wal_wakelock, "wlan_wal_lock");
    wal_msg_queue_init();

    en_init_state = frw_get_init_state();
    /* WAL模块初始化开始时，说明HMAC肯定已经初始化成功 */
    if ((en_init_state == FRW_INIT_STATE_BUTT) || (en_init_state < FRW_INIT_STATE_HMAC_CONFIG_VAP_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_main_init::en_init_state has a invalid value [%d]!}", en_init_state);

        frw_timer_delete_all_timer();
        return -OAL_EFAIL;
    }

    wal_event_fsm_init();

    /* 创建proc */
    ret = wal_hipriv_create_proc(NULL);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, 0, "{wal_main_init::wal_hipriv_create_proc has a wrong return value[%d]!}", ret);

        frw_timer_delete_all_timer();
        return -OAL_EFAIL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    wal_dfx_init();
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wal_set_custom_process_func();
#endif

    /* 初始化每个device硬件设备对应的wiphy */
    ret = wal_cfg80211_init();
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, 0, "{wal_main_init::wal_cfg80211_init has a wrong return value [%d]!}", ret);

        frw_timer_delete_all_timer();
        return -OAL_EFAIL;
    }

    /* 在host侧如果WAL初始化成功，即为全部初始化成功 */
    frw_set_init_state(FRW_INIT_STATE_ALL_SUCC);

    /* wal钩子函数初始化 */
    wal_drv_cfg_func_hook_init();

    /* wal层对外钩子函数注册至oam模块 */
    oam_wal_func_fook_register(&g_st_wal_drv_func_hook);

#if defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT)
    /* debug sysfs */
    wal_sysfs_entry_init();
#endif

    wal_register_ip_filter(&g_st_ip_filter_ops);

    if (hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
        wal_register_lights_suspend();
    }
    return OAL_SUCC;
}


void wal_destroy_all_vap(void)
{
    uint8_t uc_vap_id = 0;
    oal_net_device_stru *pst_net_dev = NULL;
    int8_t ac_param[NUM_10_BYTES] = { 0 };
    oal_io_print("wal_destroy_all_vap start");

    for (uc_vap_id = oal_board_get_service_vap_start_id(); uc_vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_vap_id++) {
        pst_net_dev = hmac_vap_get_net_device(uc_vap_id);
        if (pst_net_dev != NULL) {
            oal_net_close_dev(pst_net_dev);

            wal_hipriv_del_vap(pst_net_dev, ac_param);
            frw_event_process_all_event(0);
        }
    }
    return;
}


void wal_main_exit(void)
{
#if defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT)
    /* debug sysfs */
    wal_sysfs_entry_exit();
#endif
    /* down掉所有的vap */
    wal_destroy_all_vap();
    wal_event_fsm_exit();

    /* 删除proc */
    wal_hipriv_remove_proc();

    /* 卸载成功时，将初始化状态置为HMAC初始化成功 */
    frw_set_init_state(FRW_INIT_STATE_HMAC_CONFIG_VAP_SUCC);

    /* 去注册钩子函数 */
    oam_wal_func_fook_unregister(); /* DTSxxxxxx 删除cfg80211 删除网络设备工作队列 */
#ifdef _PRE_WLAN_FEATURE_DFR
    wal_dfx_exit();
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

    oal_wake_lock_exit(&g_st_wal_wakelock);
    frw_timer_clean_timer(OAM_MODULE_ID_WAL);

    wal_unregister_ip_filter();

    if (hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
        wal_unregister_lights_suspend();
    }
}

