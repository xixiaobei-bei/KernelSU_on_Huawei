

#ifdef _PRE_WLAN_CFGID_DEBUG

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oal_cfg80211.h"
#include "oal_util.h"

#include "oam_ext_if.h"
#include "frw_ext_if.h"

#include "wlan_types.h"

#include "mac_vap.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#include "mac_ie.h"
#include "mac_frame.h"


#include "hmac_ext_if.h"
#include "hmac_chan_mgmt.h"

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
#include "hmac_rx_filter.h"
#include "hmac_scan.h"

#ifdef _PRE_WLAN_FEATURE_P2P
#include "wal_linux_cfg80211.h"
#endif

#include "wal_dfx.h"

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#include "oal_hcc_host_if.h"
#include "plat_cali.h"
#endif

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/notifier.h>
#include <linux/inetdevice.h>
#include <net/addrconf.h>
#endif
#include "hmac_arp_offload.h"
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
#include "hmac_roam_main.h"
#endif  // _PRE_WLAN_FEATURE_ROAM
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif

#include "mac_device.h"
#include "oal_main.h"
#include "securec.h"
#include "hmac_rx_data.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID      OAM_FILE_ID_WAL_LINUX_IOCTL_DEBUG_C
#define MAX_PRIV_CMD_SIZE 4096
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#define HIPRIV_NBFH "nbfh"

#define WLAN_PACKET_CHECK_THR     10         /* 包检查超时后判定收包数量成功的默认门限,百分比 */
#define WLAN_PACKET_CHECK_PER     100        /* 接收广播帧校验比例参数 */
#define WLAN_PACKET_CHECK_TIMEOUT 1000       /* 定时器1000ms定时 */
#define WLAN_NO_FILTER_EQ_LOCAL   0xFFFFFFFD /* 接收到发送地址跟本地MAC地址一样的帧不过滤 */
#define WLAN_RX_FILTER_CNT        20         /* 驱动适配的rx指定过滤的报文种类数 */
/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_IP_FILTER

OAL_STATIC oal_uint32 wal_hipriv_set_assigned_filter_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint16 us_len;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wal_msg_write_stru st_write_msg;
    mac_assigned_filter_cmd_stru st_assigned_filter_cmd;

    /* 准备配置命令 */
    us_len = OAL_SIZEOF(mac_assigned_filter_cmd_stru);
    memset_s((oal_uint8 *)&st_assigned_filter_cmd, us_len, 0, us_len);

    /* 获取filter id */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_assigned_filter_switch::1th parm err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    if (oal_atoi(ac_name) < 0 || oal_atoi(ac_name) > WLAN_RX_FILTER_CNT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_assigned_filter_switch::icmp filter id not match.}");
        return OAL_FAIL;
    }

    st_assigned_filter_cmd.uc_filter_id = (oal_uint8)oal_atoi(ac_name);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + ul_off_set;

    /* 获取开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_assigned_filter_switch::2th para err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    st_assigned_filter_cmd.en_enable = ((oal_uint8)oal_atoi(ac_name) > 0) ? OAL_TRUE : OAL_FALSE;

    oam_warning_log2(0, OAM_SF_ANY, "{wal_hipriv_set_assigned_filter_switch::filter id [%d] on/off[%d].}",
                     st_assigned_filter_cmd.uc_filter_id, st_assigned_filter_cmd.en_enable);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ASSIGNED_FILTER, us_len);
    /* 将申请的netbuf首地址填写到msg消息体内 */
    if (EOK != memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                        (oal_uint8 *)&st_assigned_filter_cmd, us_len)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_assigned_filter_switch::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_assigned_filter_switch::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif

OAL_STATIC oal_uint32 wal_hipriv_global_log_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_int32 l_switch_val;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;

    /* 获取开关状态值 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_global_log_switch::error code[%d]}\r\n", ul_ret);
        return ul_ret;
    }

    if ((0 != strcmp("0", ac_name)) && (0 != strcmp("1", ac_name))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_global_log_switch::invalid switch value}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    l_switch_val = oal_atoi(ac_name);

    return oam_log_set_global_switch((oal_switch_enum_uint8)l_switch_val);
}


OAL_STATIC oal_uint32 wal_hipriv_vap_log_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    mac_vap_stru *pst_mac_vap;
    oal_int32 l_switch_val;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;

    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (oal_unlikely(pst_mac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_vap_log_switch::null pointer.}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取开关状态值 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_vap_log_switch::error code[%d]}\r\n", ul_ret);
        return ul_ret;
    }

    if ((0 != strcmp("0", ac_name)) && (0 != strcmp("1", ac_name))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_vap_log_switch::invalid switch value}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    l_switch_val = oal_atoi(ac_name);

    return oam_log_set_vap_switch(pst_mac_vap->uc_vap_id, (oal_switch_enum_uint8)l_switch_val);
}


OAL_STATIC oal_uint32 wal_hipriv_feature_log_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    mac_vap_stru *pst_mac_vap;
    oam_feature_enum_uint8 en_feature_id;
    oal_uint8 uc_switch_vl;
    oal_uint32 ul_off_set;
    oal_int8 ac_param[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oam_log_level_enum_uint8 en_log_lvl;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    wal_msg_write_stru st_write_msg;
#endif

    /* OAM log模块的开关的命令: hipriv "Hisilicon0[vapx] feature_log_switch {feature_name} {0/1}"
       1-2(error与warning)级别日志以vap级别为维度；
 */
    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (oal_unlikely(pst_mac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_feature_log_switch::null pointer.}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取特性名称 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_param, OAL_SIZEOF(ac_param), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }
    pc_param += ul_off_set;

    /* 提供特性名的帮助信息 */
    if ('?' == ac_param[0]) {
        OAL_IO_PRINT("please input abbr feature name. \r\n");
        oam_show_feature_list();
        return OAL_SUCC;
    }

    /* 获取特性ID */
    ul_ret = oam_get_feature_id((oal_uint8 *)ac_param, &en_feature_id);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_feature_log_switch::invalid feature name}\r\n");
        return ul_ret;
    }

    /* 获取开关值 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_param, OAL_SIZEOF(ac_param), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }
    pc_param += ul_off_set;

    /* 获取INFO级别开关状态 */
    if ((0 != strcmp("0", ac_param)) && (0 != strcmp("1", ac_param))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_feature_log_switch::invalid switch value}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    uc_switch_vl = (oal_uint8)oal_atoi(ac_param);

    /* 关闭INFO日志级别时，恢复成默认的日志级别 */
    en_log_lvl = (uc_switch_vl == OAL_SWITCH_ON) ? OAM_LOG_LEVEL_INFO : OAM_LOG_DEFAULT_LEVEL;
    ul_ret = oam_log_set_feature_level(pst_mac_vap->uc_vap_id, en_feature_id, en_log_lvl);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_FEATURE_LOG, OAL_SIZEOF(oal_int32));
    /* feature_id高8位与log_lvl拼接成st_write_msg.auc_value */
    *((oal_uint16 *)(st_write_msg.auc_value)) = ((en_feature_id << 8) | en_log_lvl);
    ul_ret |= (oal_uint32)wal_send_cfg_event(pst_net_dev,
                                             WAL_MSG_TYPE_WRITE,
                                             WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                                             (oal_uint8 *)&st_write_msg,
                                             OAL_FALSE,
                                             OAL_PTR_NULL);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_feature_log_switch::return err code[%d]!}\r\n", ul_ret);
        return ul_ret;
    }
#endif

    return ul_ret;
}


OAL_STATIC oal_uint32 wal_hipriv_log_ratelimit(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oam_ratelimit_stru st_ratelimit;
    oam_ratelimit_type_enum_uint8 en_ratelimit_type;
    oal_uint32 ul_off_set;
    oal_int8 ac_param[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;

    /* OAM流控配置命令: hipriv "Hisilicon0[vapx] {log_ratelimit} {printk(0)/sdt(1)}{switch(0/1)} {interval} {burst}" */
    st_ratelimit.en_ratelimit_switch = OAL_SWITCH_OFF;
    st_ratelimit.ul_interval = OAM_RATELIMIT_DEFAULT_INTERVAL;
    st_ratelimit.ul_burst = OAM_RATELIMIT_DEFAULT_BURST;

    /* 获取限速类型 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_param, OAL_SIZEOF(ac_param), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }
    pc_param += ul_off_set;

    en_ratelimit_type = (oam_ratelimit_type_enum_uint8)oal_atoi(ac_param);

    /* 获取开关状态 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_param, OAL_SIZEOF(ac_param), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }
    pc_param += ul_off_set;

    st_ratelimit.en_ratelimit_switch = (oal_switch_enum_uint8)oal_atoi(ac_param);

    if (st_ratelimit.en_ratelimit_switch == OAL_SWITCH_ON) {
        /* 获取interval值 */
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_param, OAL_SIZEOF(ac_param), &ul_off_set);
        if (ul_ret != OAL_SUCC) {
            return ul_ret;
        }
        pc_param += ul_off_set;

        st_ratelimit.ul_interval = (oal_uint32)oal_atoi(ac_param);

        /* 获取burst值 */
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_param, OAL_SIZEOF(ac_param), &ul_off_set);
        if (ul_ret != OAL_SUCC) {
            return ul_ret;
        }
        pc_param += ul_off_set;

        st_ratelimit.ul_burst = (oal_uint32)oal_atoi(ac_param);
    }

    return oam_log_set_ratelimit_param(en_ratelimit_type, &st_ratelimit);
}

OAL_STATIC oal_uint32 wal_hipriv_set_2040_coext_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_param;
    oal_uint32 ul_off_set = 0;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    oal_int32 l_ret;
    oal_uint32 ul_ret;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_2040_coext_switch::wal_get_cmd_one_arg fails!}\r\n");
        return ul_ret;
    }
    l_param = oal_atoi((const oal_int8 *)ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_VAP_CAP_FLAG_SWITCH, OAL_SIZEOF(oal_uint32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_param;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_2040_coext_switch::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

OAL_STATIC oal_uint32 wal_hipriv_log_lowpower(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_tmp;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32 l_ret;
    oal_uint32 ul_ret;

    /* OAM event模块的开关的命令: hipriv "Hisilicon0 log_pm 0 | 1"
        此处将解析出"1"或"0"存入ac_name
 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_log_lowpower::wal_get_cmd_one_arg return err_code[%d]}\r\n",
                         ul_ret);
        return ul_ret;
    }

    /* 针对解析出的不同命令，对event模块进行不同的设置 */
    if (0 == (strcmp("0", ac_name))) {
        l_tmp = 0;
    } else if (0 == (strcmp("1", ac_name))) {
        l_tmp = 1;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_log_lowpower::the log switch command is error [%x]!}\r\n",
                         (uintptr_t)ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_LOG_PM, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_tmp; /* 设置配置命令参数 */

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_event_switch::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_pm_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_tmp;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32 l_ret;
    oal_uint32 ul_ret;

    /* OAM event模块的开关的命令: hipriv "Hisilicon0 wal_hipriv_pm_switch 0 | 1"
        此处将解析出"1"或"0"存入ac_name
 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_pm_switch::wal_get_cmd_one_arg return err_code[%d]}\r\n", ul_ret);
        return ul_ret;
    }

    /* 针对解析出的不同命令，对event模块进行不同的设置 */
    if (0 == (strcmp("0", ac_name))) {
        l_tmp = 0;
    } else if (0 == (strcmp("1", ac_name))) {
        l_tmp = 1;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_pm_switch::the log switch command is error [%x]!}\r\n",
                         (uintptr_t)ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_PM_SWITCH, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_tmp; /* 设置配置命令参数 */

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_pm_switch::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif


oal_uint32 wal_hipriv_set_ucast_data_dscr_param(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_set_dscr_param_stru *pst_set_dscr_param;
    wal_dscr_param_enum_uint8 en_param_index;
    oal_int8 ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_DSCR, OAL_SIZEOF(mac_cfg_set_dscr_param_stru));

    /* 解析并设置配置命令参数 */
    pst_set_dscr_param = (mac_cfg_set_dscr_param_stru *)(st_write_msg.auc_value);

    /* 获取描述符字段设置命令字符串 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ucast_data_dscr_param::getonearg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    /* 解析是设置哪一个字段 */
    for (en_param_index = 0; en_param_index < WAL_DSCR_PARAM_BUTT; en_param_index++) {
        if (!strcmp(g_pauc_tx_dscr_param_name[en_param_index], ac_arg)) {
            break;
        }
    }

    /* 检查命令是否打错 */
    if (en_param_index == WAL_DSCR_PARAM_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_ucast_data_dscr_param::no such param for tx dscr!}\r\n");
        return OAL_FAIL;
    }

    pst_set_dscr_param->uc_function_index = en_param_index;

    /* 解析要设置为多大的值 */
    pst_set_dscr_param->l_value = oal_strtol(pc_param, OAL_PTR_NULL, 0);

    /* 单播数据帧描述符设置 tpye = MAC_VAP_CONFIG_UCAST_DATA */
    pst_set_dscr_param->en_type = MAC_VAP_CONFIG_UCAST_DATA;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_set_dscr_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ucast_data_dscr_param::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_bcast_data_dscr_param(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_set_dscr_param_stru *pst_set_dscr_param;
    wal_dscr_param_enum_uint8 en_param_index;
    oal_int8 ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_DSCR, OAL_SIZEOF(mac_cfg_set_dscr_param_stru));

    /* 解析并设置配置命令参数 */
    pst_set_dscr_param = (mac_cfg_set_dscr_param_stru *)(st_write_msg.auc_value);

    /* 获取描述符字段设置命令字符串 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_bcast_data_dscr_param::getonearg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    /* 解析是设置哪一个字段 */
    for (en_param_index = 0; en_param_index < WAL_DSCR_PARAM_BUTT; en_param_index++) {
        if (!strcmp(g_pauc_tx_dscr_param_name[en_param_index], ac_arg)) {
            break;
        }
    }

    /* 检查命令是否打错 */
    if (en_param_index == WAL_DSCR_PARAM_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_bcast_data_dscr_param::no such param for tx dscr!}\r\n");
        return OAL_FAIL;
    }

    pst_set_dscr_param->uc_function_index = en_param_index;

    /* 解析要设置为多大的值 */
    pst_set_dscr_param->l_value = oal_strtol(pc_param, OAL_PTR_NULL, 0);

    /* 广播数据帧描述符设置 tpye = MAC_VAP_CONFIG_BCAST_DATA */
    pst_set_dscr_param->en_type = MAC_VAP_CONFIG_BCAST_DATA;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_set_dscr_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_bcast_data_dscr_param::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_ucast_mgmt_dscr_param(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_set_dscr_param_stru *pst_set_dscr_param;
    wal_dscr_param_enum_uint8 en_param_index;
    oal_int8 ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint8 uc_band;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_DSCR, OAL_SIZEOF(mac_cfg_set_dscr_param_stru));

    /* 解析并设置配置命令参数 */
    pst_set_dscr_param = (mac_cfg_set_dscr_param_stru *)(st_write_msg.auc_value);

    /***************************************************************************
             sh hipriv.sh "vap0 set_ucast_mgmt data0 2 8389137"
    ***************************************************************************/
    /* 解析data0 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ucast_mgmt_dscr_param::getonearg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    /* 解析是设置哪一个字段 */
    for (en_param_index = 0; en_param_index < WAL_DSCR_PARAM_BUTT; en_param_index++) {
        if (!strcmp(g_pauc_tx_dscr_param_name[en_param_index], ac_arg)) {
            break;
        }
    }

    /* 检查命令是否打错 */
    if (en_param_index == WAL_DSCR_PARAM_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_ucast_mgmt_dscr_param::no such param for tx dscr!}\r\n");
        return OAL_FAIL;
    }

    pst_set_dscr_param->uc_function_index = en_param_index;

    /* 解析要设置为哪个频段的单播管理帧 2G or 5G */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ucast_mgmt_dscr_param::getonearg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    uc_band = (oal_uint8)oal_atoi(ac_arg);
    /* 单播管理帧描述符设置 tpye = MAC_VAP_CONFIG_UCAST_MGMT 2为2G,否则为5G */
    if (WLAN_BAND_2G == uc_band) {
        pst_set_dscr_param->en_type = MAC_VAP_CONFIG_UCAST_MGMT_2G;
    } else {
        pst_set_dscr_param->en_type = MAC_VAP_CONFIG_UCAST_MGMT_5G;
    }

    /* 解析要设置为多大的速率 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ucast_mgmt_dscr_param::getonearg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    pst_set_dscr_param->l_value = oal_strtol(ac_arg, OAL_PTR_NULL, 0);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_set_dscr_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ucast_mgmt_dscr_param::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_mbcast_mgmt_dscr_param(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_set_dscr_param_stru *pst_set_dscr_param;
    wal_dscr_param_enum_uint8 en_param_index;
    oal_int8 ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint8 uc_band;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_DSCR, OAL_SIZEOF(mac_cfg_set_dscr_param_stru));

    /* 解析并设置配置命令参数 */
    pst_set_dscr_param = (mac_cfg_set_dscr_param_stru *)(st_write_msg.auc_value);

    /***************************************************************************
             sh hipriv.sh "vap0 set_mcast_mgmt data0 5 8389137"
    ***************************************************************************/
    /* 解析data0 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mbcast_mgmt_dscr_param::getonearg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    /* 解析是设置哪一个字段 */
    for (en_param_index = 0; en_param_index < WAL_DSCR_PARAM_BUTT; en_param_index++) {
        if (!strcmp(g_pauc_tx_dscr_param_name[en_param_index], ac_arg)) {
            break;
        }
    }

    /* 检查命令是否打错 */
    if (en_param_index == WAL_DSCR_PARAM_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_mbcast_mgmt_dscr_param::no such param for tx dscr!}\r\n");
        return OAL_FAIL;
    }

    pst_set_dscr_param->uc_function_index = en_param_index;

    /* 解析要设置为哪个频段的单播管理帧 2G or 5G */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mbcast_mgmt_dscr_param::getonearg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    uc_band = (oal_uint8)oal_atoi(ac_arg);
    /* 单播管理帧描述符设置 tpye = MAC_VAP_CONFIG_UCAST_MGMT 2为2G,否则为5G */
    if (WLAN_BAND_2G == uc_band) {
        pst_set_dscr_param->en_type = MAC_VAP_CONFIG_MBCAST_MGMT_2G;
    } else {
        pst_set_dscr_param->en_type = MAC_VAP_CONFIG_MBCAST_MGMT_5G;
    }

    /* 解析要设置为多大的速率 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mbcast_mgmt_dscr_param::getonearg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    pst_set_dscr_param->l_value = oal_strtol(ac_arg, OAL_PTR_NULL, 0);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_set_dscr_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_mbcast_mgmt_dscr_param::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11D

oal_uint32 wal_hipriv_set_rd_by_ie_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_switch_enum_uint8 *pst_set_rd_by_ie_switch;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_switch_enum_uint8 en_rd_by_ie_switch;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_RD_IE_SWITCH, OAL_SIZEOF(oal_switch_enum_uint8));

    /* 解析并设置配置命令参数 */
    pst_set_rd_by_ie_switch = (oal_switch_enum_uint8 *)(st_write_msg.auc_value);

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_rd_by_ie_switch::walgetcmdone_arg return err_code [%d]!}",
                         ul_ret);
        return ul_ret;
    }
    en_rd_by_ie_switch = (oal_uint8)oal_atoi(ac_name);
    *pst_set_rd_by_ie_switch = en_rd_by_ie_switch;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_switch_enum_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_rd_by_ie_switch::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32 wal_hipriv_event_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_tmp;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32 l_ret;
    oal_uint32 ul_ret;

    /* OAM event模块的开关的命令: hipriv "Hisilicon0 event_switch 0 | 1"
        此处将解析出"1"或"0"存入ac_name
 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_event_switch::wal_get_cmd_one_arg return err_code[%d]}\r\n",
                         ul_ret);
        return ul_ret;
    }

    /* 针对解析出的不同命令，对event模块进行不同的设置 */
    if (0 == (strcmp("0", ac_name))) {
        l_tmp = 0;
    } else if (0 == (strcmp("1", ac_name))) {
        l_tmp = 1;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_event_switch::the log switch command is error [%x]!}\r\n",
                         (uintptr_t)ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_EVENT_SWITCH, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_tmp; /* 设置配置命令参数 */

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_event_switch::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_ota_beacon_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_param;
    oal_uint32 ul_off_set = 0;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    oal_int32 l_ret;
    oal_uint32 ul_ret;

    /* OAM ota模块的开关的命令: hipriv "Hisilicon0 ota_beacon_switch 0 | 1"
 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ota_beacon_switch::wal_get_cmd_one_arg fails!}\r\n");
        return ul_ret;
    }
    l_param = oal_atoi((const oal_int8 *)ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_OTA_BEACON_SWITCH, OAL_SIZEOF(oal_uint32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_param;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ota_beacon_switch::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_ota_rx_dscr_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_param;
    oal_uint32 ul_off_set = 0;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    oal_int32 l_ret;
    oal_uint32 ul_ret;

    /* OAM ota模块的开关的命令: hipriv "Hisilicon0 ota_rx_dscr_switch 0 | 1"
 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ota_rx_dscr_switch::wal_get_cmd_one_arg fails!}\r\n");
        return ul_ret;
    }

    l_param = oal_atoi((const oal_int8 *)ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_OTA_RX_DSCR_SWITCH, OAL_SIZEOF(oal_uint32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_param;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ota_rx_dscr_switch::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_ether_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32 l_ret;
    oal_uint32 ul_ret;
    mac_cfg_eth_switch_param_stru st_eth_switch_param;

    /* "vap0 ether_switch user_macaddr oam_ota_frame_direction_type_enum(帧方向) 0|1(开关)" */
    memset_s(&st_eth_switch_param, OAL_SIZEOF(mac_cfg_eth_switch_param_stru), 0,
             OAL_SIZEOF(mac_cfg_eth_switch_param_stru));

    ul_ret = wal_hipriv_get_mac_addr(pc_param, st_eth_switch_param.auc_user_macaddr, &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ether_switch::walhipriv_get_mac_addr return err_code[%d]}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    /* 获取以太网帧方向 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ether_switch::wal_get_cmd_one_arg return err_code[%d]}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_eth_switch_param.en_frame_direction = (oal_uint8)oal_atoi(ac_name);

    /* 获取开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ether_switch::wal_get_cmd_one_arg return err_code[%d]}\r\n",
                         ul_ret);
        return ul_ret;
    }
    st_eth_switch_param.en_switch = (oal_uint8)oal_atoi(ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ETH_SWITCH, OAL_SIZEOF(st_eth_switch_param));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
                 (const oal_void *)&st_eth_switch_param,
                 OAL_SIZEOF(st_eth_switch_param)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_ether_switch::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_eth_switch_param),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ether_switch::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_80211_ucast_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32 l_ret;
    oal_uint32 ul_ret;
    mac_cfg_80211_ucast_switch_stru st_80211_ucast_switch;

    /* sh hipriv.sh "vap0 80211_uc_switch user_macaddr 0|1(帧方向tx|rx) 0|1(帧类型:管理帧|数据帧)
                                                       0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关)"
 */
    memset_s(&st_80211_ucast_switch, OAL_SIZEOF(mac_cfg_80211_ucast_switch_stru), 0,
             OAL_SIZEOF(mac_cfg_80211_ucast_switch_stru));

    /* 获取mac地址 */
    ul_ret = wal_hipriv_get_mac_addr(pc_param, st_80211_ucast_switch.auc_user_macaddr, &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_get_mac_addr return err_code[%d]}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    /* 获取80211帧方向 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{get 80211 ucast frame direction return err_code[%d]}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_80211_ucast_switch.en_frame_direction = (oal_uint8)oal_atoi(ac_name);

    /* 获取帧类型 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{get ucast frame type return err_code[%d]}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_80211_ucast_switch.en_frame_type = (oal_uint8)oal_atoi(ac_name);

    /* 获取帧内容打印开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{get ucast frame content switch  return err_code[%d]}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_80211_ucast_switch.en_frame_switch = (oal_uint8)oal_atoi(ac_name);

    /* 获取帧CB字段打印开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{get ucast frame cb switch return err_code[%d]}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_80211_ucast_switch.en_cb_switch = (oal_uint8)oal_atoi(ac_name);

    /* 获取描述符打印开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{get ucast frame dscr switch return err_code[%d]}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_80211_ucast_switch.en_dscr_switch = (oal_uint8)oal_atoi(ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_80211_UCAST_SWITCH, OAL_SIZEOF(st_80211_ucast_switch));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
                 (const oal_void *)&st_80211_ucast_switch,
                 OAL_SIZEOF(st_80211_ucast_switch)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_80211_ucast_switch::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_80211_ucast_switch),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_80211_ucast_switch::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_TXOPPS

OAL_STATIC oal_uint32 wal_hipriv_set_txop_ps_machw(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32 l_ret;
    oal_uint32 ul_ret;
    mac_txopps_machw_param_stru st_txopps_machw_param = { 0 };

    /* sh hipriv.sh "stavap_name txopps_hw_en 0|1(txop_ps_en) 0|1(condition1) 0|1(condition2)" */
    /* 获取txop ps使能开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_TXOP, "{get machw txop_ps en return err_code[%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_txopps_machw_param.en_machw_txopps_en = (oal_switch_enum_uint8)oal_atoi(ac_name);

    /* 获取txop ps condition1使能开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_TXOP, "{get machw txop_ps condition1 return err_code[%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_txopps_machw_param.en_machw_txopps_condition1 = (oal_switch_enum_uint8)oal_atoi(ac_name);

    /* 获取txop ps condition2使能开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_TXOP, "{get machw txop_ps condition2 return err_code[%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_txopps_machw_param.en_machw_txopps_condition2 = (oal_switch_enum_uint8)oal_atoi(ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_TXOP_PS_MACHW, OAL_SIZEOF(st_txopps_machw_param));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
                 (const oal_void *)&st_txopps_machw_param,
                 OAL_SIZEOF(st_txopps_machw_param)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_txop_ps_machw::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_txopps_machw_param),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_TXOP, "{wal_hipriv_set_txop_ps_machw::return err code[%d]!}\r\n", ul_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32 wal_hipriv_set_80211_mcast_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32 l_ret;
    oal_uint32 ul_ret;
    mac_cfg_80211_mcast_switch_stru st_80211_mcast_switch = { 0 };

    memset_s((oal_uint8 *)&st_write_msg, OAL_SIZEOF(st_write_msg), 0, OAL_SIZEOF(st_write_msg));
    /* sh hipriv.sh "Hisilicon0 80211_mc_switch 0|1(帧方向tx|rx) 0|1(帧类型:管理帧|数据帧)
                                                0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关)"
 */
    /* 获取80211帧方向 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{get 80211 mcast frame direction return err_code[%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_80211_mcast_switch.en_frame_direction = (oal_uint8)oal_atoi(ac_name);

    /* 获取帧类型 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{get mcast frame type return err_code[%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_80211_mcast_switch.en_frame_type = (oal_uint8)oal_atoi(ac_name);

    /* 获取帧内容打印开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{get mcast frame content switch return err_code[%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_80211_mcast_switch.en_frame_switch = (oal_uint8)oal_atoi(ac_name);

    /* 获取帧CB字段打印开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{get mcast frame cb switch return err_code[%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_80211_mcast_switch.en_cb_switch = (oal_uint8)oal_atoi(ac_name);

    /* 获取描述符打印开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{get mcast frame dscr switch return err_code[%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_80211_mcast_switch.en_dscr_switch = (oal_uint8)oal_atoi(ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_80211_MCAST_SWITCH, OAL_SIZEOF(st_80211_mcast_switch));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
                 (const oal_void *)&st_80211_mcast_switch,
                 OAL_SIZEOF(st_80211_mcast_switch)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_80211_mcast_switch::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_80211_mcast_switch),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_80211_mcast_switch::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_all_80211_ucast(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    mac_cfg_80211_ucast_switch_stru st_80211_ucast_switch;
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;

    /* sh hipriv.sh "Hisilicon0 80211_uc_all 0|1(帧方向tx|rx) 0|1(帧类型:管理帧|数据帧)
                                             0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关)"
 */
    memset_s(&st_80211_ucast_switch, OAL_SIZEOF(mac_cfg_80211_ucast_switch_stru), 0,
             OAL_SIZEOF(mac_cfg_80211_ucast_switch_stru));

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{get 80211 ucast frame direction return err_code[%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_80211_ucast_switch.en_frame_direction = (oal_uint8)oal_atoi(ac_name);

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{get ucast frame type return err_code[%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_80211_ucast_switch.en_frame_type = (oal_uint8)oal_atoi(ac_name);

    /* 获取帧内容打印开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{get ucast frame content switch return err_code[%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_80211_ucast_switch.en_frame_switch = (oal_uint8)oal_atoi(ac_name);

    /* 获取帧CB字段打印开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{get ucast frame cb switch return err_code[%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_80211_ucast_switch.en_cb_switch = (oal_uint8)oal_atoi(ac_name);

    /* 获取描述符打印开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_all_80211_ucast::ucastframedscrswitch return errcode[%d]}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_80211_ucast_switch.en_dscr_switch = (oal_uint8)oal_atoi(ac_name);

    /* 设置广播mac地址 */
    memcpy_s(st_80211_ucast_switch.auc_user_macaddr, WLAN_MAC_ADDR_LEN, BROADCAST_MACADDR, WLAN_MAC_ADDR_LEN);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_80211_UCAST_SWITCH, OAL_SIZEOF(st_80211_ucast_switch));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 (const oal_void *)&st_80211_ucast_switch,
                 OAL_SIZEOF(st_80211_ucast_switch)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_all_80211_ucast::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_80211_ucast_switch),
                               (oal_uint8 *)&st_write_msg, OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_80211_all_ucast_switch::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_all_ether_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_uint8 uc_user_num;
    oal_uint8 uc_frame_direction;
    oal_uint8 uc_switch;

    /* sh hipriv.sh "Hisilicon0 ether_all 0|1(帧方向tx|rx) 0|1(开关)" */
    /* 获取以太网帧方向 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_all_ether_switch::ethframedirection return err_code[%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    uc_frame_direction = (oal_uint8)oal_atoi(ac_name);

    /* 获取帧开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_all_ether_switch::get eth type return err_code[%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    uc_switch = (oal_uint8)oal_atoi(ac_name);

    /* 设置开关 */
    for (uc_user_num = 0; uc_user_num < WLAN_ACTIVE_USER_MAX_NUM + WLAN_MAX_MULTI_USER_NUM_SPEC; uc_user_num++) {
        oam_report_eth_frame_set_switch(uc_user_num, uc_switch, uc_frame_direction);
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_dhcp_arp_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint8 uc_switch;

    /* sh hipriv.sh "Hisilicon0 dhcp_arp_switch 0|1(开关)" */
    /* 获取帧方向 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_dhcp_arp_switch::get switch return err_code[%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    uc_switch = (oal_uint8)oal_atoi(ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_DHCP_ARP, OAL_SIZEOF(oal_uint32));
    *((oal_int32 *)(st_write_msg.auc_value)) = (oal_uint32)uc_switch;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_dhcp_arp_switch::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_phy_debug_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int8 ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN];
    mac_phy_debug_switch_stru st_phy_debug_switch;
    oal_int32 l_ret;
    oal_uint32 ul_ret;
    oal_uint8 uc_value;

    /* sh hipriv.sh "wlan0 phy_debug snr 0|1(关闭|打开) rssi 0|1(关闭|打开) trlr 1234a count N(每个N个报文打印一次)" */
    memset_s(&st_phy_debug_switch, OAL_SIZEOF(st_phy_debug_switch), 0, OAL_SIZEOF(st_phy_debug_switch));
    st_phy_debug_switch.ul_rx_comp_isr_interval = 10;  // 如果没有设置，则默认10个包打印一次，命令码可以更新
    st_phy_debug_switch.uc_edca_param_switch = 0x0;    // EDCA参数设置开关
#ifdef _PRE_WLAN_FEATURE_DYN_BYPASS_EXTLNA
    st_phy_debug_switch.uc_extlna_chg_bypass_switch = 0xff;
#endif

    do {
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
        if (oal_all_true_value2((ul_ret != OAL_SUCC), (ul_off_set != 0))) {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_set_phy_debug_switch::cmd format err, ret:%d;!!}\r\n", ul_ret);
            return ul_ret;
        }

        if (ul_off_set == 0) {
            break;
        }
        pc_param += ul_off_set;

        ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
        if ((ul_ret != OAL_SUCC) || ((!isdigit(ac_value[0])) && (0 != strcmp("help", ac_value)))) {
            OAM_ERROR_LOG0(0, OAM_SF_CFG,
                           "{CMD format::sh hipriv.sh 'wlan0 phy_debug [rssi 0|1] [snr 0|1] \
                           [trlr xxxxx] [vect yyyyy] [count N] [edca 0-3(tid_no)] [aifsn N] [cwmin N] [cwmax N] \
                           [txoplimit N]'!!}\r\n");
            return ul_ret;
        }
        pc_param += ul_off_set;
        ul_off_set = 0;

        /* 提取ac_value参数 */
        uc_value = (oal_uint8)oal_atoi(ac_value);

        if (strcmp("rssi", ac_name) == 0) {
            st_phy_debug_switch.en_rssi_debug_switch = uc_value & OAL_TRUE;
        } else if (strcmp("count", ac_name) == 0) {
            st_phy_debug_switch.ul_rx_comp_isr_interval = (oal_uint32)oal_atoi(ac_value);
        } else if (strcmp("tsensor", ac_name) == 0) {
            st_phy_debug_switch.en_tsensor_debug_switch = uc_value & OAL_TRUE;
        }
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
        else if (strcmp("pdet", ac_name) == 0) {
            st_phy_debug_switch.en_pdet_debug_switch = uc_value & OAL_TRUE;
        }
#endif
        /* 解析命令并进行EDCA参数配置 */
        else if (strcmp("edca", ac_name) == 0) {
            /* edca_param_switch与uc_value低4位拼接成EDCA参数设置开关 */
            st_phy_debug_switch.uc_edca_param_switch |= uc_value << 4;

            /* 固定EDCA参数，让寄存器不再刷新 */
            wal_hipriv_alg_cfg(pst_net_dev, "edca_opt_fix_param 1");
        } else if (strcmp("11h_test", ac_name) == 0) {
            mac_set_11h_test((oal_uint8)oal_atoi(ac_value));
        } else if (strcmp("aifsn", ac_name) == 0) {
            st_phy_debug_switch.uc_edca_param_switch |= (oal_uint8)BIT3;
            st_phy_debug_switch.uc_edca_aifsn = uc_value;
        } else if (strcmp("cwmin", ac_name) == 0) {
            st_phy_debug_switch.uc_edca_param_switch |= (oal_uint8)BIT2;
            st_phy_debug_switch.uc_edca_cwmin = uc_value;
        } else if (strcmp("cwmax", ac_name) == 0) {
            st_phy_debug_switch.uc_edca_param_switch |= (oal_uint8)BIT1;
            st_phy_debug_switch.uc_edca_cwmax = uc_value;
        } else if (strcmp("txoplimit", ac_name) == 0) {
            st_phy_debug_switch.uc_edca_param_switch |= (oal_uint8)BIT0;
            st_phy_debug_switch.us_edca_txoplimit = (oal_uint16)oal_atoi(ac_value);
        }
#ifdef _PRE_WLAN_FEATURE_DYN_BYPASS_EXTLNA
        else if (strcmp("extlna_bypass", ac_name) == 0) {
            st_phy_debug_switch.uc_extlna_chg_bypass_switch = (oal_uint8)oal_atoi(ac_value);
        }
#endif
        else {
            oam_warning_log0(0, OAM_SF_CFG,
                             "{CMD format::sh hipriv.sh 'wlan0 phy_debug [rssi 0|1] [snr 0|1] \
                             [trlr xxxx] [vect yyyy] [count N] [edca 0-3(tid_no)] [aifsn N] [cwmin N] [cwmax N] \
                             [txoplimit N] [11h_test 0|1]!!}\r\n");
            return OAL_FAIL;
        }
    } while (*pc_param != '\0');

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_PHY_DEBUG_SWITCH, OAL_SIZEOF(st_phy_debug_switch));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value), (const oal_void *)&st_phy_debug_switch,
        OAL_SIZEOF(st_phy_debug_switch)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_phy_debug_switch::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_phy_debug_switch),
                               (oal_uint8 *)&st_write_msg, OAL_FALSE, OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_rssi_switch::return err code[%d]!}", ul_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32 wal_hipriv_set_opmode_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int8 ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN];
    oal_int8 *pc_value = ac_value;
    mac_opmode_switch_stru st_opmode_switch = {0};
    oal_int32 l_ret;
    oal_uint32 ul_ret;
    oal_uint8 uc_value;

    do {
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
        if (oal_all_true_value2((ul_ret != OAL_SUCC), (ul_off_set != 0))) {
            return ul_ret;
        }

        if (ul_off_set == 0) {
            break;
        }
        pc_param += ul_off_set;

        ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
        if ((ul_ret != OAL_SUCC) || ((!isdigit(ac_value[0])) && (0 != strcmp("help", ac_value)))) {
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "{sh hipriv.sh 'wlan0 opmode_switch [opmode 0|1]\r\n");
            return ul_ret;
        }
        pc_param += ul_off_set;
        ul_off_set = 0;

        /* 提取ac_value参数 */
        uc_value = (oal_uint8)oal_atoi(pc_value);

        if (strcmp("opmode", ac_name) == 0) {
            st_opmode_switch.uc_opmode_switch = uc_value & OAL_TRUE;
        } else {
            oam_warning_log0(0, OAM_SF_CFG, "{sh hipriv.sh 'wlan0 wlan0 opmode_switch [opmode 0|1]!!}\r\n");
            return OAL_FAIL;
        }
    } while (*pc_param != '\0');

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_OPMODE_SWITCH, OAL_SIZEOF(st_opmode_switch));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value), (const oal_void *)&st_opmode_switch,
        OAL_SIZEOF(st_opmode_switch)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_opmode_switch::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_opmode_switch),
                               (oal_uint8 *)&st_write_msg, OAL_FALSE, OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_opmode_switch::return err code[%d]!}", ul_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_probe_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32 l_ret;
    oal_uint32 ul_ret;
    mac_cfg_probe_switch_stru st_probe_switch;

    /* sh hipriv.sh "Hisilicon0 probe_switch 0|1(帧方向tx|rx) 0|1(帧内容开关)
                                             0|1(CB开关) 0|1(描述符开关)"
 */
    /* 获取帧方向 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_probe_switch::get probe direction return err_code[%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_probe_switch.en_frame_direction = (oal_uint8)oal_atoi(ac_name);

    /* 获取帧内容打印开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{probe frame content switch return errcode[%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_probe_switch.en_frame_switch = (oal_uint8)oal_atoi(ac_name);

    /* 获取帧CB字段打印开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_probe_switch::probe frame cb switch return err_code[%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_probe_switch.en_cb_switch = (oal_uint8)oal_atoi(ac_name);

    /* 获取描述符打印开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_probe_switch::probeframe dscr switch return errcode[%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    st_probe_switch.en_dscr_switch = (oal_uint8)oal_atoi(ac_name);

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_PROBE_SWITCH, OAL_SIZEOF(st_probe_switch));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
                 (const oal_void *)&st_probe_switch, OAL_SIZEOF(st_probe_switch)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_probe_switch::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_probe_switch),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_80211_ucast_switch::return err code[%d]!}\r\n", ul_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_all_ota(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32 l_param;
    wal_msg_write_stru st_write_msg;

    /* 获取开关 sh hipriv.sh "Hisilicon0 set_all_ota 0|1" */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    l_param = oal_atoi((const oal_int8 *)ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_ALL_OTA, OAL_SIZEOF(oal_uint32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_param;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_all_ota::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_oam_output(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_tmp;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;

    /* OAM log模块的开关的命令: hipriv "Hisilicon0 log_level 0~3"
        此处将解析出"1"或"0"存入ac_name
 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_oam_output::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    /* 针对解析出的不同命令，对log模块进行不同的设置 取值:oam_output_type_enum_uint8 */
    l_tmp = oal_atoi(ac_name);
    if (l_tmp >= OAM_OUTPUT_TYPE_BUTT) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_oam_output::output type invalid [%d]!}\r\n", l_tmp);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_OAM_OUTPUT_TYPE, OAL_SIZEOF(oal_int32));
    /* 设置配置命令参数 */
    *((oal_int32 *)(st_write_msg.auc_value)) = l_tmp;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_oam_output::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_ampdu_start(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_ampdu_start_param_stru *pst_ampdu_start_param = OAL_PTR_NULL;
    mac_cfg_ampdu_start_param_stru st_ampdu_start_param; /* 临时保存获取的use的信息 */
    oal_uint32 ul_get_addr_idx;

    /*
        设置AMPDU开启的配置命令: hipriv "Hisilicon0  ampdu_start xx xx xx xx xx xx(mac地址) tidno ack_policy"
 */
    /* 获取mac地址 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ampdu_start::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    memset_s((oal_uint8 *)&st_ampdu_start_param, OAL_SIZEOF(st_ampdu_start_param), 0,
             OAL_SIZEOF(st_ampdu_start_param));
    oal_strtoaddr(ac_name, st_ampdu_start_param.auc_mac_addr);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + ul_off_set;

    /* 获取tid */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ampdu_start::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    if (OAL_STRLEN(ac_name) > 2) { /* 命令参数长度不能小于2字节 */
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ampdu_start::the ampdu start command is erro [%x]!}\r\n",
                         (uintptr_t)ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    st_ampdu_start_param.uc_tidno = (oal_uint8)oal_atoi(ac_name);
    if (st_ampdu_start_param.uc_tidno >= WLAN_TID_MAX_NUM) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ampdu_start::ampdu start cmd is error! uc_tidno is  [%d]!}\r\n",
                         st_ampdu_start_param.uc_tidno);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_AMPDU_START, OAL_SIZEOF(mac_cfg_ampdu_start_param_stru));

    /* 设置配置命令参数 */
    pst_ampdu_start_param = (mac_cfg_ampdu_start_param_stru *)(st_write_msg.auc_value);
    for (ul_get_addr_idx = 0; ul_get_addr_idx < WLAN_MAC_ADDR_LEN; ul_get_addr_idx++) {
        pst_ampdu_start_param->auc_mac_addr[ul_get_addr_idx] = st_ampdu_start_param.auc_mac_addr[ul_get_addr_idx];
    }

    pst_ampdu_start_param->uc_tidno = st_ampdu_start_param.uc_tidno;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ampdu_start_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ampdu_start::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_auto_ba_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_tmp;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32 l_ret;
    oal_uint32 ul_ret;

    /* 设置自动开始BA会话的开关:hipriv "vap0  auto_ba 0 | 1" 该命令针对某一个VAP */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_auto_ba_switch::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    /* 针对解析出的不同命令，对AUTO BA进行不同的设置 */
    if (0 == (strcmp("0", ac_name))) {
        l_tmp = 0;
    } else if (0 == (strcmp("1", ac_name))) {
        l_tmp = 1;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_auto_ba_switch::the auto ba switch command is error[%x]!}\r\n",
                         (uintptr_t)ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_AUTO_BA_SWITCH, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_tmp; /* 设置配置命令参数 */

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_auto_ba_switch::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_addba_req(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_addba_req_param_stru *pst_addba_req_param = OAL_PTR_NULL;
    mac_cfg_addba_req_param_stru st_addba_req_param; /* 临时保存获取的addba req的信息 */
    oal_uint32 ul_get_addr_idx;

    /*
        设置AMPDU关闭的配置命令:
        hipriv "Hisilicon0 addba_req xx xx xx xx xx xx(mac地址) tidno ba_policy buffsize timeout"
 */
    /* 获取mac地址 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    memset_s((oal_uint8 *)&st_addba_req_param, OAL_SIZEOF(st_addba_req_param), 0, OAL_SIZEOF(st_addba_req_param));
    oal_strtoaddr(ac_name, st_addba_req_param.auc_mac_addr);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + ul_off_set;

    /* 获取tid */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    if (OAL_STRLEN(ac_name) > 2) { /* 命令参数长度不能小于2字节 */
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::the addba req command is error[%x]!}\r\n",
                         (uintptr_t)ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    st_addba_req_param.uc_tidno = (oal_uint8)oal_atoi(ac_name);
    if (st_addba_req_param.uc_tidno >= WLAN_TID_MAX_NUM) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::the addba req command is error!uc_tidno is [%d]!}\r\n",
                         st_addba_req_param.uc_tidno);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pc_param = pc_param + ul_off_set;

    /* 获取ba_policy */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    st_addba_req_param.en_ba_policy = (oal_uint8)oal_atoi(ac_name);
    if (st_addba_req_param.en_ba_policy != MAC_BA_POLICY_IMMEDIATE) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::the ba policy is not correct! ba_policy is[%d]!}\r\n",
                         st_addba_req_param.en_ba_policy);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pc_param = pc_param + ul_off_set;

    /* 获取buffsize */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    st_addba_req_param.us_buff_size = (oal_uint16)oal_atoi(ac_name);

    pc_param = pc_param + ul_off_set;

    /* 获取timeout时间 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    st_addba_req_param.us_timeout = (oal_uint16)oal_atoi(ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ADDBA_REQ, OAL_SIZEOF(mac_cfg_addba_req_param_stru));

    /* 设置配置命令参数 */
    pst_addba_req_param = (mac_cfg_addba_req_param_stru *)(st_write_msg.auc_value);
    for (ul_get_addr_idx = 0; ul_get_addr_idx < WLAN_MAC_ADDR_LEN; ul_get_addr_idx++) {
        pst_addba_req_param->auc_mac_addr[ul_get_addr_idx] = st_addba_req_param.auc_mac_addr[ul_get_addr_idx];
    }

    pst_addba_req_param->uc_tidno = st_addba_req_param.uc_tidno;
    pst_addba_req_param->en_ba_policy = st_addba_req_param.en_ba_policy;
    pst_addba_req_param->us_buff_size = st_addba_req_param.us_buff_size;
    pst_addba_req_param->us_timeout = st_addba_req_param.us_timeout;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_addba_req_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_delba_req(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_delba_req_param_stru *pst_delba_req_param = OAL_PTR_NULL;
    mac_cfg_delba_req_param_stru st_delba_req_param; /* 临时保存获取的addba req的信息 */
    oal_uint32 ul_get_addr_idx;

    /*
        设置AMPDU关闭的配置命令:
        hipriv "Hisilicon0 delba_req xx xx xx xx xx xx(mac地址) tidno direction reason_code"
 */
    /* 获取mac地址 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_delba_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    memset_s((oal_uint8 *)&st_delba_req_param, OAL_SIZEOF(st_delba_req_param), 0, OAL_SIZEOF(st_delba_req_param));
    oal_strtoaddr(ac_name, st_delba_req_param.auc_mac_addr);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + ul_off_set;

    /* 获取tid */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_delba_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    if (OAL_STRLEN(ac_name) > 2) { /* 命令参数长度不能小于2字节 */
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_delba_req::the delba_req req command is error[%x]!}\r\n",
                         (uintptr_t)ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    st_delba_req_param.uc_tidno = (oal_uint8)oal_atoi(ac_name);
    if (st_delba_req_param.uc_tidno >= WLAN_TID_MAX_NUM) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_delba_req::delba_req req command is error! uc_tidno is[%d]!}\r\n",
                         st_delba_req_param.uc_tidno);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pc_param = pc_param + ul_off_set;

    /* 获取direction */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_delba_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    st_delba_req_param.en_direction = (oal_uint8)oal_atoi(ac_name);
    if (st_delba_req_param.en_direction >= MAC_BUTT_DELBA) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_delba_req::the direction is not correct! direction is[%d]!}\r\n",
                         st_delba_req_param.en_direction);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DELBA_REQ, OAL_SIZEOF(mac_cfg_delba_req_param_stru));

    /* 设置配置命令参数 */
    pst_delba_req_param = (mac_cfg_delba_req_param_stru *)(st_write_msg.auc_value);
    for (ul_get_addr_idx = 0; ul_get_addr_idx < WLAN_MAC_ADDR_LEN; ul_get_addr_idx++) {
        pst_delba_req_param->auc_mac_addr[ul_get_addr_idx] = st_delba_req_param.auc_mac_addr[ul_get_addr_idx];
    }

    pst_delba_req_param->uc_tidno = st_delba_req_param.uc_tidno;
    pst_delba_req_param->en_direction = st_delba_req_param.en_direction;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_delba_req_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_delba_req::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_WMMAC

OAL_STATIC oal_uint32 wal_hipriv_addts_req(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_wmm_tspec_stru_param_stru *pst_addts_req_param;
    mac_cfg_wmm_tspec_stru_param_stru st_addts_req_param; /* 临时保存获取的addts req的信息 */

    /*
    设置发送ADDTS REQ配置命令:
    hipriv "vap0 addts_req tid direction psb up nominal_msdu_size maximum_data_rate
            minimum_data_rate mean_data_rate peak_data_rate minimum_phy_rate surplus_bandwidth_allowance"
 */
    /***********************************************************************************************
 TSPEC字段:
          --------------------------------------------------------------------------------------
          |TS Info|Nominal MSDU Size|Max MSDU Size|Min Serv Itvl|Max Serv Itvl|
          ---------------------------------------------------------------------------------------
 Octets:  | 3     |  2              |   2         |4            |4            |
          ---------------------------------------------------------------------------------------
          | Inactivity Itvl | Suspension Itvl | Serv Start Time |Min Data Rate | Mean Data Rate |
          ---------------------------------------------------------------------------------------
 Octets:  |4                | 4               | 4               |4             |  4             |
          ---------------------------------------------------------------------------------------
          |Peak Data Rate|Burst Size|Delay Bound|Min PHY Rate|Surplus BW Allowance  |Medium Time|
          ---------------------------------------------------------------------------------------
 Octets:  |4             |4         | 4         | 4          |  2                   |2          |
          ---------------------------------------------------------------------------------------

 TS info字段:
          ---------------------------------------------------------------------------------------
          |Reserved |TSID |Direction |1 |0 |Reserved |PSB |UP |Reserved |Reserved |Reserved |
          ---------------------------------------------------------------------------------------
   Bits:  |1        |4    |2         |  2  |1        |1   |3  |2        |1        |7        |
          ----------------------------------------------------------------------------------------

 ***********************************************************************************************/
    /* 获取tid，取值范围0~7 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addts_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    if (OAL_STRLEN(ac_name) > 2) { /* 命令参数长度不能小于2字节 */
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addts_req::the addba req command is error[%x]!}\r\n",
                         (uintptr_t)ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    st_addts_req_param.ts_info.bit_tsid = (oal_uint16)oal_atoi(ac_name);
    if (st_addts_req_param.ts_info.bit_tsid >= WLAN_TID_MAX_NUM) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addts_req::the addts req command is error!uc_tidno is [%d]!}\r\n",
                         st_addts_req_param.ts_info.bit_tsid);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pc_param = pc_param + ul_off_set;

    /* 获取direction 00:uplink 01:downlink 10:reserved 11:Bi-directional */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addts_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    st_addts_req_param.ts_info.bit_direction = (oal_uint16)oal_atoi(ac_name);
    if (st_addts_req_param.ts_info.bit_direction == MAC_WMMAC_DIRECTION_RESERVED) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addts_req::the direction is not correct! direction is[%d]!}\r\n",
                         st_addts_req_param.ts_info.bit_direction);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pc_param = pc_param + ul_off_set;

    /* 获取PSB，1表示U-APSD，0表示legacy */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addts_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    st_addts_req_param.ts_info.bit_apsd = (oal_uint16)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /* 获取UP */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addts_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    st_addts_req_param.ts_info.bit_user_prio = (oal_uint16)oal_atoi(ac_name);

    pc_param = pc_param + ul_off_set;

    /* 获取Nominal MSDU Size ,第一位为1 */
    /*
        ------------
        |fixed|size|
        ------------
 bits:  |1    |15  |
        ------------
 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addts_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    st_addts_req_param.us_norminal_msdu_size = (oal_uint16)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /* 获取maximum MSDU size */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addts_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    st_addts_req_param.us_max_msdu_size = (oal_uint16)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /* 获取minimum data rate */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addts_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    st_addts_req_param.ul_min_data_rate = (oal_uint32)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /* 获取mean data rate */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addts_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    st_addts_req_param.ul_mean_data_rate = (oal_uint32)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /* 获取peak data rate */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addts_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    st_addts_req_param.ul_peak_data_rate = (oal_uint32)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /* 获取minimum PHY Rate */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addts_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    st_addts_req_param.ul_min_phy_rate = (oal_uint32)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /* 获取surplus bandwidth allowance */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addts_req::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    st_addts_req_param.us_surplus_bw = (oal_uint16)oal_atoi(ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ADDTS_REQ, OAL_SIZEOF(mac_cfg_wmm_tspec_stru_param_stru));

    /* 设置配置命令参数 */
    pst_addts_req_param = (mac_cfg_wmm_tspec_stru_param_stru *)(st_write_msg.auc_value);

    pst_addts_req_param->ts_info.bit_tsid = st_addts_req_param.ts_info.bit_tsid;
    pst_addts_req_param->ts_info.bit_direction = st_addts_req_param.ts_info.bit_direction;
    pst_addts_req_param->ts_info.bit_apsd = st_addts_req_param.ts_info.bit_apsd;
    pst_addts_req_param->ts_info.bit_user_prio = st_addts_req_param.ts_info.bit_user_prio;
    pst_addts_req_param->us_norminal_msdu_size = st_addts_req_param.us_norminal_msdu_size;
    pst_addts_req_param->us_max_msdu_size = st_addts_req_param.us_max_msdu_size;
    pst_addts_req_param->ul_min_data_rate = st_addts_req_param.ul_min_data_rate;
    pst_addts_req_param->ul_mean_data_rate = st_addts_req_param.ul_mean_data_rate;
    pst_addts_req_param->ul_peak_data_rate = st_addts_req_param.ul_peak_data_rate;
    pst_addts_req_param->ul_min_phy_rate = st_addts_req_param.ul_min_phy_rate;
    pst_addts_req_param->us_surplus_bw = st_addts_req_param.us_surplus_bw;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_wmm_tspec_stru_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_addts_req::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_delts(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_wmm_tspec_stru_param_stru *pst_delts_param;
    mac_cfg_wmm_tspec_stru_param_stru st_delts_param;

    /* 设置删除TS的配置命令: hipriv "Hisilicon0 delts tidno" */
    /* 获取tsid */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_delts::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    st_delts_param.ts_info.bit_tsid = (oal_uint8)oal_atoi(ac_name);
    if (st_delts_param.ts_info.bit_tsid >= WLAN_TID_MAX_NUM) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_delts::the delts command is error! tsid is[%d]!}\r\n",
                         st_delts_param.ts_info.bit_tsid);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DELTS, OAL_SIZEOF(mac_cfg_wmm_tspec_stru_param_stru));

    /* 设置配置命令参数 */
    pst_delts_param = (mac_cfg_wmm_tspec_stru_param_stru *)(st_write_msg.auc_value);
    memset_s(pst_delts_param, OAL_SIZEOF(mac_cfg_wmm_tspec_stru_param_stru), 0,
             OAL_SIZEOF(mac_cfg_wmm_tspec_stru_param_stru));

    pst_delts_param->ts_info.bit_tsid = st_delts_param.ts_info.bit_tsid;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_wmm_tspec_stru_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_delts::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_wmmac_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint8 uc_wmmac_switch;
    mac_cfg_wmm_ac_param_stru st_wmm_ac_param;

    /* 设置删除TS的配置命令: hipriv "vap0 wmmac_switch 1/0(使能) 0|1(WMM_AC认证使能) AC xxx(limit_medium_time)" */
    memset_s(&st_wmm_ac_param, OAL_SIZEOF(mac_cfg_wmm_ac_param_stru), 0, OAL_SIZEOF(mac_cfg_wmm_ac_param_stru));
    /* 获取mac地址 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_wmmac_switch::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    uc_wmmac_switch = (oal_uint8)oal_atoi(ac_name);
    if (uc_wmmac_switch != OAL_FALSE) {
        uc_wmmac_switch = OAL_TRUE;
    }
    st_wmm_ac_param.en_wmm_ac_switch = uc_wmmac_switch;

    /* 获取auth flag */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_wmmac_switch::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    st_wmm_ac_param.en_auth_flag = (oal_uint8)oal_atoi(ac_name);
    pc_param += ul_off_set;

    /* 获取ac */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_wmmac_switch::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    st_wmm_ac_param.en_ac_type = (oal_uint8)oal_atoi(ac_name);
    pc_param += ul_off_set;
    if (st_wmm_ac_param.en_ac_type > WLAN_WME_AC_BUTT) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_wmmac_switch::AC Type is invalid ac=[%d]!}\r\n",
                       st_wmm_ac_param.en_ac_type);
        return OAL_FAIL;
    }

    /* 获取limit time */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_wmmac_switch::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    st_wmm_ac_param.ul_limit_time = (oal_uint32)oal_atoi(ac_name);
    pc_param += ul_off_set;

    /* 设置配置命令参数 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_WMMAC_SWITCH, OAL_SIZEOF(st_wmm_ac_param));
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
                 (const oal_void *)&st_wmm_ac_param,
                 OAL_SIZEOF(st_wmm_ac_param)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_wmmac_switch::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_wmm_ac_param),
                               (oal_uint8 *)&st_write_msg, OAL_FALSE, OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_delts::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_reassoc_req(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_REASSOC_REQ, OAL_SIZEOF(oal_int32));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_reassoc_req::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif

OAL_STATIC oal_uint32 wal_hipriv_mem_info(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    oal_int8 auc_token[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_mem_pool_id_enum_uint8 en_pool_id;
    oal_uint32 ul_off_set;
    oal_uint32 ul_ret;

    /* 入参检查 */
    if (oal_unlikely(pst_cfg_net_dev == OAL_PTR_NULL) || oal_unlikely(pc_param == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_hipriv_mem_info::pst_net_dev or pc_param null ptr error [%x] [%x]!}\r\n",
                       (uintptr_t)pst_cfg_net_dev, (uintptr_t)pc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取内存池ID */
    ul_ret = wal_get_cmd_one_arg(pc_param, auc_token, OAL_SIZEOF(auc_token), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_mem_info::wal_get_cmd_one_arg return error code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    en_pool_id = (oal_mem_pool_id_enum_uint8)oal_atoi(auc_token);

    /* 打印内存池信息 */
    oal_mem_info(en_pool_id);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_mem_leak(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    oal_int8 auc_token[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_mem_pool_id_enum_uint8 en_pool_id;
    oal_uint32 ul_off_set;
    oal_uint32 ul_ret;

    /* 入参检查 */
    if (oal_unlikely(pst_cfg_net_dev == OAL_PTR_NULL) || oal_unlikely(pc_param == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_hipriv_mem_leak::pst_net_dev or pc_param null ptr error [%x] [%x]!}\r\n",
                       (uintptr_t)pst_cfg_net_dev, (uintptr_t)pc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取内存池ID */
    ul_ret = wal_get_cmd_one_arg(pc_param, auc_token, OAL_SIZEOF(auc_token), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_mem_leak::wal_get_cmd_one_arg return error code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    en_pool_id = (oal_mem_pool_id_enum_uint8)oal_atoi(auc_token);
    if (en_pool_id > OAL_MEM_POOL_ID_SDT_NETBUF) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_mem_leak::mem pool id exceeds,en_pool_id[%d]!}\r\n", en_pool_id);
        return OAL_SUCC;
    }

    /* 检查内存池泄漏内存块 */
    oal_mem_leak(en_pool_id);

    return OAL_SUCC;
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)


OAL_STATIC oal_uint32 wal_hipriv_memory_info(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
    oal_uint8 uc_pool_id;
    mac_device_pool_id_stru *pst_pool_id_param = OAL_PTR_NULL;
    oal_uint8 uc_meminfo_type = MAC_MEMINFO_BUTT;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_memory_info::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    if (0 == (strcmp("host", ac_name))) {
        oal_mem_print_pool_info();
        return OAL_SUCC;
    } else if (0 == (strcmp("device", ac_name))) {
        hcc_print_device_mem_info();
        return OAL_SUCC;
    } else if (0 == (strcmp("pool", ac_name))) {
        uc_meminfo_type = MAC_MEMINFO_POOL_INFO;
    } else if (0 == (strcmp("sample_alloc", ac_name))) {
        uc_meminfo_type = MAC_MEMINFO_SAMPLE_ALLOC;
    } else if (0 == (strcmp("sample_free", ac_name))) {
        uc_meminfo_type = MAC_MEMINFO_SAMPLE_FREE;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_memory_info::getcmdonearg::second arg:: please check input!}\r\n");
        return OAL_FAIL;
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DEVICE_MEM_INFO, OAL_SIZEOF(mac_device_pool_id_stru));

    /* 设置配置命令参数 */
    pst_pool_id_param = (mac_device_pool_id_stru *)(st_write_msg.auc_value);
    pst_pool_id_param->uc_meminfo_type = uc_meminfo_type;
    pst_pool_id_param->uc_pool_id = 0xff;
    pc_param = pc_param + ul_off_set;
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    /* 没有后续参数不退出 */
    if (ul_ret == OAL_SUCC) {
        uc_pool_id = (oal_uint8)oal_atoi(ac_name);
        pst_pool_id_param->uc_pool_id = uc_pool_id;
    }

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_device_pool_id_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_PWR, "{wal_hipriv_memory_info::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#endif


OAL_STATIC oal_int8 wal_get_dbb_scaling_index(oal_uint8 uc_band,
                                              oal_uint8 uc_bw,
                                              oal_int8 *pc_mcs_type,
                                              oal_int8 *pc_mcs_value)
{
    oal_int8 *pc_end = OAL_PTR_NULL;
    oal_bool_enum_uint8 en_mcs_found = OAL_FALSE;
    oal_int8 *rate_table[MAC_DBB_SCALING_2G_RATE_NUM] = {
        "1", "2", "5.5", "11", "6", "9", "12", "18", "24", "36", "48", "54"
    };
    oal_uint8 uc_scaling_offset = 0;
    oal_uint8 uc_mcs_index = 0;

    if (uc_band == 0) {                                  // 2g band
        if (strcmp(pc_mcs_type, "rate") == 0) {  // rate xxx
            uc_scaling_offset = MAC_DBB_SCALING_2G_RATE_OFFSET;
            for (uc_mcs_index = 0; uc_mcs_index < MAC_DBB_SCALING_2G_RATE_NUM; uc_mcs_index++) {
                if (strcmp(pc_mcs_value, rate_table[uc_mcs_index]) == 0) {
                    en_mcs_found = OAL_TRUE;
                    break;
                }
            }
            if (en_mcs_found) {
                uc_scaling_offset += uc_mcs_index;
            }
        } else if (strcmp(pc_mcs_type, "mcs") == 0) {  // mcs xxx
            if (uc_bw == 20) {                                  // HT20
                uc_scaling_offset = MAC_DBB_SCALING_2G_HT20_MCS_OFFSET;
            } else if (uc_bw == 40) {  // HT40
                uc_scaling_offset = MAC_DBB_SCALING_2G_HT40_MCS_OFFSET;
            } else {
                oam_warning_log0(0, OAM_SF_CFG, "{dmac_config_dbb_scaling_amend:: 2G uc_bw out of range.");
                return -1;
            }
            uc_mcs_index = (oal_uint8)oal_strtol(pc_mcs_value, &pc_end, 10); /* 10代表十进制 */
            if (uc_mcs_index <= 7) { /* mcs0~7 */
                en_mcs_found = OAL_TRUE;
                uc_scaling_offset += uc_mcs_index;
            }
            if ((uc_bw == 40) && (uc_mcs_index == 32)) { /* HT40, 2.4G mcs32 */
                en_mcs_found = OAL_TRUE;
                uc_scaling_offset = MAC_DBB_SCALING_2G_HT40_MCS32_OFFSET;
            }
        }
    } else {                                             // 5g band
        if (strcmp(pc_mcs_type, "rate") == 0) {  // rate xxx
            uc_scaling_offset = MAC_DBB_SCALING_5G_RATE_OFFSET;
            for (uc_mcs_index = 0; uc_mcs_index < MAC_DBB_SCALING_5G_RATE_NUM; uc_mcs_index++) {
                /* rate_table偏移uc_mcs_index+4个字节与pc_mcs_value进行字符串比较 */
                if (strcmp(pc_mcs_value, rate_table[uc_mcs_index + 4]) == 0) {
                    en_mcs_found = OAL_TRUE;
                    break;
                }
            }
            if (en_mcs_found) {
                uc_scaling_offset += uc_mcs_index;
            }
        } else if (strcmp(pc_mcs_type, "mcs") == 0) {  // mcs
            uc_mcs_index = (oal_uint8)oal_strtol(pc_mcs_value, &pc_end, 10); /* 10代表十进制 */
            if (uc_bw == 20) {  // HT20
                uc_scaling_offset = MAC_DBB_SCALING_5G_HT20_MCS_OFFSET;
                if (uc_mcs_index <= 7) {  // HT20 mcs0~7
                    en_mcs_found = OAL_TRUE;
                    uc_scaling_offset += uc_mcs_index;
                } else if (uc_mcs_index == 8) {  // HT20 mcs8
                    en_mcs_found = OAL_TRUE;
                    uc_scaling_offset = MAC_DBB_SCALING_5G_HT20_MCS8_OFFSET;
                }
            } else if (uc_bw == 40) {  // HT40
                uc_scaling_offset = MAC_DBB_SCALING_5G_HT40_MCS_OFFSET;
                if (uc_mcs_index <= 9) {  // HT20 mcs0~9
                    en_mcs_found = OAL_TRUE;
                    uc_scaling_offset += uc_mcs_index;
                } else if (uc_mcs_index == 32) {  // 5G mcs32
                    en_mcs_found = OAL_TRUE;
                    uc_scaling_offset = MAC_DBB_SCALING_5G_HT40_MCS32_OFFSET;
                }
            } else {
                oam_warning_log0(0, OAM_SF_CFG, "{dmac_config_dbb_scaling_amend:: 5G mcs uc_bw out of range.");
                return -1;
            }
        } else if (strcmp(pc_mcs_type, "mcsac") == 0) {  // mcsac
            uc_mcs_index = (oal_uint8)oal_strtol(pc_mcs_value, &pc_end, 10); /* 10代表十进制 */
            if (uc_bw == 80) { /* HT80 */
                uc_scaling_offset = MAC_DBB_SCALING_5G_HT80_MCS_OFFSET;
                if (uc_mcs_index <= 9) {  // HT20 mcs0~9
                    en_mcs_found = OAL_TRUE;
                    uc_scaling_offset += uc_mcs_index;
                }
                if ((uc_mcs_index == 0) || (uc_mcs_index == 1)) {
                    uc_scaling_offset -= MAC_DBB_SCALING_5G_HT80_MCS0_DELTA_OFFSET;
                }
            } else {
                oam_warning_log0(0, OAM_SF_CFG, "{dmac_config_dbb_scaling_amend:: 5G mcsac uc_bw out of range.");
                return -1;
            }
        }
    }
    if (en_mcs_found) {
        return (oal_int8)uc_scaling_offset;
    }
    return -1;
}


OAL_STATIC oal_uint32 wal_dbb_scaling_amend_calc(oal_int8 *pc_param, mac_cfg_dbb_scaling_stru *pst_dbb_scaling)
{
    oal_int8 *pc_token = OAL_PTR_NULL;
    oal_int8 *pc_end = OAL_PTR_NULL;
    oal_int8 *pc_ctx = OAL_PTR_NULL;
    oal_int8 *pc_sep = " ";
    oal_int8 *pc_mcs_type = OAL_PTR_NULL;
    oal_int8 *pc_mcs_value = OAL_PTR_NULL;
    oal_uint8 uc_band;
    oal_uint8 uc_bw;
    oal_int8 uc_scaling_offset;
    oal_uint16 us_delta_gain;
    oal_uint8 uc_offset_addr_a;
    oal_uint8 uc_offset_addr_b;

    if (pc_param == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_dbb_scaling_amend_calc:: param1 NULL!.");
        return OAL_FAIL;
    }

    /* 获取band字符串 */
    pc_token = oal_strtok(pc_param, pc_sep, &pc_ctx);
    if (pc_token == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_dbb_scaling_amend_calc:: param2 NULL!.");
        return OAL_FAIL;
    }
    if (0 != strcmp(pc_token, "band")) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_dbb_scaling_amend_calc:: band not match.");
        return OAL_FAIL;
    }

    /* 获取band值 */
    pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
    if (pc_token == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_dbb_scaling_amend_calc:: param3 NULL!.");
        return OAL_FAIL;
    }
    uc_band = (oal_uint8)oal_strtol(pc_token, &pc_end, 10); /* 10代表十进制 */
    if ((uc_band != 0) && (uc_band != 1)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_dbb_scaling_amend_calc:: band = %d, value out of range.", uc_band);
        return OAL_FAIL;
    }

    /* 获取bw字符串 */
    pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
    if (pc_token == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_dbb_scaling_amend_calc:: param4 NULL!.");
        return OAL_FAIL;
    }

    if (0 != strcmp(pc_token, "bw")) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_dbb_scaling_amend_calc:: bw not match.");
        return OAL_FAIL;
    }

    /* 获取bw值 */
    pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
    if (pc_token == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_dbb_scaling_amend_calc:: param5 NULL!.");
        return OAL_FAIL;
    }
    uc_bw = (oal_uint8)oal_strtol(pc_token, &pc_end, 10); /* 10代表十进制 */
    if ((uc_bw != 20) && (uc_bw != 40) && (uc_bw != 80)) { /* 20、40、80分别代表HT20 HT40 HT80 */
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_dbb_scaling_amend_calc:: bw = %d, value out of range.", uc_bw);
        return OAL_FAIL;
    }

    /* 获取rate/mcs/mcsac字符串 */
    pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
    if (pc_token == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_dbb_scaling_amend_calc:: param6 NULL!.");
        return OAL_FAIL;
    }
    pc_mcs_type = pc_token;
    if ((strcmp(pc_token, "rate") != 0)
        && (strcmp(pc_token, "mcs") != 0)
        && (strcmp(pc_token, "mcsac") != 0)) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_dbb_scaling_amend_calc:: rate/mcs/mcsac not match.");
        return OAL_FAIL;
    }

    /* 获取rate/mcs/mcsac值 */
    pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
    if (pc_token == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_dbb_scaling_amend_calc:: param7 NULL!.");
        return OAL_FAIL;
    }
    pc_mcs_value = pc_token;

    /* 解析速率值 */
    uc_scaling_offset = wal_get_dbb_scaling_index(uc_band, uc_bw, pc_mcs_type, pc_mcs_value);
    /*lint -e571*/
    OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_dbb_scaling_amend_calc::  uc_scaling_offset = %d.", uc_scaling_offset);
    /*lint +e571*/
    if (uc_scaling_offset == -1) {
        return OAL_FAIL;
    }

    /* 获取delta_gain值 */
    pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
    if (pc_token == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_dbb_scaling_amend_calc:: param8 NULL!.");
        return OAL_FAIL;
    }
    us_delta_gain = (oal_uint16)oal_strtol(pc_token, &pc_end, 10); /* 10代表十进制 */

    uc_offset_addr_a = (oal_uint8)uc_scaling_offset;
    uc_offset_addr_a = (uc_offset_addr_a >> 2); /* uc_offset_addr_a右移2位 */
    uc_offset_addr_b =
       /* 判断dbb scaling对应的索引值是否小于4倍uc_offset_addr_a（uc_offset_addr_a左移2位） */
       (uc_scaling_offset < (uc_offset_addr_a << 2)) ? 0 : ((oal_uint8)(uc_scaling_offset - (uc_offset_addr_a << 2)));
    pst_dbb_scaling->uc_offset_addr_a = uc_offset_addr_a;
    pst_dbb_scaling->uc_offset_addr_b = uc_offset_addr_b;
    pst_dbb_scaling->us_delta_gain = us_delta_gain;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_dbb_scaling_amend(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
    oal_uint16 us_len;
    mac_cfg_dbb_scaling_stru st_dbb_scaling;

    if (OAL_SUCC != wal_dbb_scaling_amend_calc(pc_param, &st_dbb_scaling)) {
        return OAL_FAIL;
    }
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    us_len = OAL_SIZEOF(mac_cfg_dbb_scaling_stru);

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DBB_SCALING_AMEND, us_len);

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
        (const oal_void *)&st_dbb_scaling, us_len) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_dbb_scaling_amend::memcpy fail!}");
        return OAL_FAIL;
    }
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_reg_info::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_2040_channel_switch_prohibited(oal_net_device_stru *pst_net_dev,
                                                                oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint8 uc_csp;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_2040_channel_switch_prohibite::getonearg return errcode{%d}}\r\n",
                         ul_ret);
        return ul_ret;
    }

    if (0 == (strcmp("0", ac_name))) {
        uc_csp = 0;
    } else if (0 == (strcmp("1", ac_name))) {
        uc_csp = 1;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{the channel_switch_prohibited switch command is error %x!}\r\n",
                         (uintptr_t)ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_2040_CHASWI_PROHI, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = uc_csp; /* 设置配置命令参数 */

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_2040_channel_switch_prohibited::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_fortymhzintolerant(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint8 uc_csp;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_fortymhzintolerant::get_cmd_one_arg return err_code %d!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    if (0 == (strcmp("0", ac_name))) {
        uc_csp = 0;
    } else if (0 == (strcmp("1", ac_name))) {
        uc_csp = 1;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_fortymhzintolerant::the 2040_intolerant cmd is error %x!}\r\n",
                         (uintptr_t)ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_2040_INTOLERANT, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = uc_csp; /* 设置配置命令参数 */

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_fortymhzintolerant::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

OAL_STATIC oal_uint32 wal_hipriv_get_lauch_cap(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_query_stru st_query_msg;
    oal_int32 l_ret;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    st_query_msg.en_wid = WLAN_CFGID_LAUCH_CAP;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_get_lauch_cap::wal_alloc_cfg_event return err code %d!}\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_dev_customize_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SHOW_DEV_CUSTOMIZE_INFOS, OAL_SIZEOF(oal_int32));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_dev_customize_info::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32 wal_hipriv_set_txpower(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
    oal_int32 l_pwer;
    oal_uint32 ul_off_set;
    oal_int8 ac_val[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_idx = 0;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_val, OAL_SIZEOF(ac_val), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_txpower::get_cmd_one_arg vap name return err_code %d!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    /* 输入命令合法性检测 */
    while ('\0' != ac_val[l_idx]) {
        if (isdigit(ac_val[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_txpower::input illegal!}\r\n");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    l_pwer = oal_atoi(ac_val);
    if (l_pwer > 0xFF || l_pwer < 0) { /* 参数异常 */
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_txpower::invalid argument,l_pwer=%d!}", l_pwer);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_TX_POWER, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_pwer;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_txpower::return err code %d!}", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))

OAL_STATIC oal_uint32 wal_ioctl_set_beacon_interval(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_beacon_interval;
    oal_uint32 ul_off_set;
    oal_int8 ac_beacon_interval[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;

    /* 设备在up状态不允许配置，必须先down */
    if (0 != (OAL_IFF_RUNNING & oal_netdevice_flags(pst_net_dev))) {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{wal_ioctl_set_beacon_interval::device is busy, please down it firs %d!}\r\n",
                       oal_netdevice_flags(pst_net_dev));
        return -OAL_EBUSY;
    }

    /* pc_param指向新创建的net_device的name, 将其取出存放到ac_name中 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_beacon_interval, OAL_SIZEOF(ac_beacon_interval), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_ioctl_set_beacon_interval::getcmdonearg vap name return errcode %d!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    l_beacon_interval = oal_atoi(ac_beacon_interval);
    oam_info_log1(0, OAM_SF_ANY, "{wal_ioctl_set_beacon_interval::l_beacon_interval = %d!}\r\n", l_beacon_interval);

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 填写消息 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_BEACON_INTERVAL, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_beacon_interval;

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_ioctl_set_beacon_interval::return err code %d!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_start_vap(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    OAM_ERROR_LOG0(0, OAM_SF_CFG, "DEBUG:: priv start enter.");
    wal_netdev_open(pst_net_dev);
    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32 wal_hipriv_amsdu_start(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_amsdu_start_param_stru *pst_amsdu_start_param;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    /* 填写消息 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_AMSDU_START, OAL_SIZEOF(mac_cfg_amsdu_start_param_stru));

    /* 解析并设置配置命令参数 */
    pst_amsdu_start_param = (mac_cfg_amsdu_start_param_stru *)(st_write_msg.auc_value);

    /* 获取mac地址 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_amsdu_start::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    oal_strtoaddr(ac_name, pst_amsdu_start_param->auc_mac_addr);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + ul_off_set;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_amsdu_start::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    pst_amsdu_start_param->uc_amsdu_max_num = (oal_uint8)oal_atoi(ac_name);

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_amsdu_start::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    pst_amsdu_start_param->us_amsdu_max_size = (oal_uint16)oal_atoi(ac_name);

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_amsdu_start_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_amsdu_start::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_list_ap(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_LIST_AP, OAL_SIZEOF(oal_int32));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_list_ap::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_list_sta(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_LIST_STA, OAL_SIZEOF(oal_int32));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_list_sta::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_list_channel(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_LIST_CHAN, OAL_SIZEOF(oal_int32));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_list_channel::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_regdomain_pwr_priv(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_pwr;
    wal_msg_write_stru st_write_msg;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "wal_hipriv_set_regdomain_pwr, get arg return err %d", ul_ret);
        return ul_ret;
    }

    ul_pwr = (oal_uint32)oal_atoi(ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_REGDOMAIN_PWR, OAL_SIZEOF(oal_int32));

    ((mac_cfg_regdomain_max_pwr_stru *)st_write_msg.auc_value)->uc_pwr = (oal_uint8)ul_pwr;
    ((mac_cfg_regdomain_max_pwr_stru *)st_write_msg.auc_value)->en_exceed_reg = OAL_TRUE;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_set_regdomain_pwr::wal_send_cfg_event fail.return err code %d}",
                         l_ret);
    }

    return (oal_uint32)l_ret;
}


OAL_STATIC oal_uint32 wal_hipriv_start_scan(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
#ifdef _PRE_WLAN_FEATURE_P2P
    oal_uint8 uc_is_p2p0_scan;
#endif /* _PRE_WLAN_FEATURE_P2P */

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_START_SCAN, OAL_SIZEOF(oal_int32));

#ifdef _PRE_WLAN_FEATURE_P2P
    uc_is_p2p0_scan = (oal_memcmp(pst_net_dev->name, "p2p0", OAL_STRLEN("p2p0")) == 0) ? 1 : 0;
    // uc_is_p2p0_scan = (pst_net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_DEVICE)?1:0;
    st_write_msg.auc_value[0] = uc_is_p2p0_scan;
#endif /* _PRE_WLAN_FEATURE_P2P */

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_start_scan::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_start_join(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
    oal_uint32 ul_ret;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_START_JOIN, OAL_SIZEOF(oal_int32));

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_start_join::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    /* 将要关联AP的编号复制到事件msg中，AP编号是数字的ASSCI码，不超过4个字节 */
    if (memcpy_s((oal_int8 *)st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
                 (oal_int8 *)ac_name, OAL_SIZEOF(oal_int32)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_start_join::memcpy fail}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_start_join::return err codereturn err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_start_deauth(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_START_DEAUTH, OAL_SIZEOF(oal_int32));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_start_deauth::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32 wal_hipriv_kick_user(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_kick_user_param_stru *pst_kick_user_param = OAL_PTR_NULL;
    oal_uint8 auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };

    /* 去关联1个用户的命令 hipriv "vap0 kick_user xx:xx:xx:xx:xx:xx" */
    /* 获取mac地址 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_kick_user::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    oal_strtoaddr(ac_name, auc_mac_addr);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + ul_off_set;

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_KICK_USER, OAL_SIZEOF(mac_cfg_kick_user_param_stru));

    /* 设置配置命令参数 */
    pst_kick_user_param = (mac_cfg_kick_user_param_stru *)(st_write_msg.auc_value);
    oal_set_mac_addr(pst_kick_user_param->auc_mac_addr, auc_mac_addr);

    /* 填写去关联reason code */
    pst_kick_user_param->us_reason_code = MAC_UNSPEC_REASON;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_kick_user_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_kick_user::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_pause_tid(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_pause_tid_param_stru *pst_pause_tid_param = OAL_PTR_NULL;
    oal_uint8 auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };
    oal_uint8 uc_tid;
    /* 去关联1个用户的命令 hipriv "vap0 kick_user xx xx xx xx xx xx" */
    /* 获取mac地址 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_pause_tid::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    oal_strtoaddr(ac_name, auc_mac_addr);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + ul_off_set;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_pause_tid::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    uc_tid = (oal_uint8)oal_atoi(ac_name);

    pc_param = pc_param + ul_off_set;

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_PAUSE_TID, OAL_SIZEOF(mac_cfg_pause_tid_param_stru));

    /* 设置配置命令参数 */
    pst_pause_tid_param = (mac_cfg_pause_tid_param_stru *)(st_write_msg.auc_value);
    oal_set_mac_addr(pst_pause_tid_param->auc_mac_addr, auc_mac_addr);
    pst_pause_tid_param->uc_tid = uc_tid;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_pause_tid::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    pst_pause_tid_param->uc_is_paused = (oal_uint8)oal_atoi(ac_name);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_pause_tid_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_pause_tid::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_get_hipkt_stat(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;

    // sh hipriv.sh "wlan0 get_hipkt_stat"
    /* 申请事件内存 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_GET_HIPKT_STAT, OAL_SIZEOF(oal_uint8));

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_get_hipkt_stat:: return err_code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_flowctl_param(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_ret;
    oal_uint32 ul_off_set = 0;
    oal_int32 l_ret;
    oal_int8 ac_param[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    mac_cfg_flowctl_param_stru st_flowctl_param;
    mac_cfg_flowctl_param_stru *pst_param = OAL_PTR_NULL;

    // sh hipriv.sh "Hisilicon0 set_flowctl_param 0/1/2/3 20 20 40"
    // 0/1/2/3 分别代表be,bk,vi,vo
    /* 获取队列类型参数 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_param, OAL_SIZEOF(ac_param), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_flowctl_param::getcmdonearg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    st_flowctl_param.uc_queue_type = (oal_uint8)oal_atoi(ac_param);

    /* 设置队列对应的每次调度报文个数 */
    pc_param += ul_off_set;
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_param, OAL_SIZEOF(ac_param), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_flowctl_param::getonearg burst_limit return err_code %d!}\r\n",
                         ul_ret);
        return (oal_uint32)ul_ret;
    }
    st_flowctl_param.us_burst_limit = (oal_uint16)oal_atoi(ac_param);

    /* 设置队列对应的流控low_waterline */
    pc_param += ul_off_set;
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_param, OAL_SIZEOF(ac_param), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_flowctl_param::getarg lowwaterline return errcode[%d]}\r\n",
                         ul_ret);
        return (oal_uint32)ul_ret;
    }
    st_flowctl_param.us_low_waterline = (oal_uint16)oal_atoi(ac_param);

    /* 设置队列对应的流控high_waterline */
    pc_param += ul_off_set;
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_param, OAL_SIZEOF(ac_param), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_flowctl::get_cmd_onearg highwaterline return errcode{%d}}\r\n",
                         ul_ret);
        return (oal_uint32)ul_ret;
    }
    st_flowctl_param.us_high_waterline = (oal_uint16)oal_atoi(ac_param);

    /* 申请事件内存 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_FLOWCTL_PARAM, OAL_SIZEOF(mac_cfg_flowctl_param_stru));
    pst_param = (mac_cfg_flowctl_param_stru *)(st_write_msg.auc_value);

    pst_param->uc_queue_type = st_flowctl_param.uc_queue_type;
    pst_param->us_burst_limit = st_flowctl_param.us_burst_limit;
    pst_param->us_low_waterline = st_flowctl_param.us_low_waterline;
    pst_param->us_high_waterline = st_flowctl_param.us_high_waterline;

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_flowctl_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_flowctl_param:: return err_code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_get_flowctl_stat(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;

    // sh hipriv.sh "Hisilicon0 get_flowctl_stat"
    /* 申请事件内存 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_GET_FLOWCTL_STAT, OAL_SIZEOF(oal_uint8));

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_get_flowctl_stat:: return err_code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_ampdu_amsdu_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_tmp;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32 l_ret;
    oal_uint32 ul_ret;

    /* 设置自动开始BA会话的开关:hipriv "vap0  auto_ba 0 | 1" 该命令针对某一个VAP */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ampdu_amsdu_switch::get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    /* 针对解析出的不同命令，对AUTO BA进行不同的设置 */
    if (0 == (strcmp("0", ac_name))) {
        l_tmp = 0;
    } else if (0 == (strcmp("1", ac_name))) {
        l_tmp = 1;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ampdu_amsdu_switch::the auto ba switch command is error[%x]!}\r\n",
                         (uintptr_t)ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_AMSDU_AMPDU_SWITCH, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_tmp; /* 设置配置命令参数 */

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ampdu_amsdu_switch::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_event_queue_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return frw_event_queue_info();
}

OAL_STATIC oal_uint32 wal_hipriv_amsdu_tx_on(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_ampdu_tx_on_param_stru *pst_aggr_tx_on_param = OAL_PTR_NULL;
    oal_uint8 uc_aggr_tx_on;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_amsdu_tx_on::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    uc_aggr_tx_on = (oal_uint8)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_AMSDU_TX_ON, OAL_SIZEOF(mac_cfg_ampdu_tx_on_param_stru));

    /* 设置配置命令参数 */
    pst_aggr_tx_on_param = (mac_cfg_ampdu_tx_on_param_stru *)(st_write_msg.auc_value);
    pst_aggr_tx_on_param->uc_aggr_tx_on = uc_aggr_tx_on;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ampdu_tx_on_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_amsdu_tx_on::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_frag_threshold(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_ret;
    oal_int32 l_cfg_rst;
    oal_uint16 us_len;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_off_set = 0;
    mac_cfg_frag_threshold_stru *pst_threshold = OAL_PTR_NULL;
    oal_uint32 ul_threshold;

    /* 获取分片门限 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_frag_threshold::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    ul_threshold = (oal_uint16)oal_atoi(ac_name);
    pc_param += ul_off_set;

    if ((ul_threshold < WLAN_FRAG_THRESHOLD_MIN) ||
        (ul_threshold > WLAN_FRAG_THRESHOLD_MAX)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_frag_threshold::ul_threshold value error [%d]!}\r\n",
                         ul_threshold);
        return OAL_FAIL;
    }

    pst_threshold = (mac_cfg_frag_threshold_stru *)(st_write_msg.auc_value);
    pst_threshold->ul_frag_threshold = ul_threshold;

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    us_len = OAL_SIZEOF(mac_cfg_frag_threshold_stru);
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_FRAG_THRESHOLD_REG, us_len);

    l_cfg_rst = wal_send_cfg_event(pst_net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
    if (oal_unlikely(l_cfg_rst != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_frag_threshold::return err code [%d]!}\r\n", l_cfg_rst);
        return (oal_uint32)l_cfg_rst;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_wmm_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_ret;
    oal_int32 l_cfg_rst;
    oal_uint16 us_len;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_off_set = 0;
    oal_uint8 uc_open_wmm;

    /* 获取设定的值 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_wmm_switch::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    uc_open_wmm = (oal_uint8)oal_atoi(ac_name);
    pc_param += ul_off_set;

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    us_len = OAL_SIZEOF(oal_uint8);
    *(oal_uint8 *)(st_write_msg.auc_value) = uc_open_wmm;
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_WMM_SWITCH, us_len);

    l_cfg_rst = wal_send_cfg_event(pst_net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
    if (oal_unlikely(l_cfg_rst != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_wmm_switch::return err code [%d]!}\r\n", l_cfg_rst);
        return (oal_uint32)l_cfg_rst;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_ampdu_tx_on(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_ampdu_tx_on_param_stru *pst_aggr_tx_on_param = OAL_PTR_NULL;
    oal_uint8 uc_aggr_tx_on;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ampdu_tx_on::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    uc_aggr_tx_on = (oal_uint8)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_AMPDU_TX_ON, OAL_SIZEOF(mac_cfg_ampdu_tx_on_param_stru));

    /* 设置配置命令参数 */
    pst_aggr_tx_on_param = (mac_cfg_ampdu_tx_on_param_stru *)(st_write_msg.auc_value);
    pst_aggr_tx_on_param->uc_aggr_tx_on = uc_aggr_tx_on;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ampdu_tx_on_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ampdu_tx_on::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_txbf_switch(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_ampdu_tx_on_param_stru *pst_aggr_tx_on_param = OAL_PTR_NULL;
    oal_uint8 uc_aggr_tx_on;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_txbf_switch::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    uc_aggr_tx_on = (oal_uint8)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_TXBF_SWITCH, OAL_SIZEOF(mac_cfg_ampdu_tx_on_param_stru));

    /* 设置配置命令参数 */
    pst_aggr_tx_on_param = (mac_cfg_ampdu_tx_on_param_stru *)(st_write_msg.auc_value);
    pst_aggr_tx_on_param->uc_aggr_tx_on = uc_aggr_tx_on;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ampdu_tx_on_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_txbf_switch::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_reset_operate(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
    oal_uint16 us_len;
    if (oal_unlikely(WAL_MSG_WRITE_MAX_LEN <= OAL_STRLEN(pc_param))) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_reset_operate:: pc_param overlength is %d}\n",
                         OAL_STRLEN(pc_param));
        return OAL_FAIL;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value), pc_param, OAL_STRLEN(pc_param)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_reset_operate::memcpy fail!}");
        return OAL_FAIL;
    }

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_RESET_HW_OPERATE, us_len);

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_reset_operate::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_UAPSD

OAL_STATIC oal_uint32 wal_hipriv_uapsd_debug(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
    oal_uint16 us_len;

    if (oal_unlikely(WAL_MSG_WRITE_MAX_LEN <= OAL_STRLEN(pc_param))) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_uapsd_debug:: pc_param overlength is %d}\n", OAL_STRLEN(pc_param));
        return OAL_FAIL;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value), pc_param, OAL_STRLEN(pc_param)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_uapsd_debug::memcpy fail!}");
        return OAL_FAIL;
    }

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_UAPSD_DEBUG, us_len);

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_uapsd_debug::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_DFT_STAT

OAL_STATIC oal_uint32 wal_hipriv_usr_queue_stat(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_usr_queue_param_stru st_usr_queue_param;

    /* sh hipriv.sh "vap_name usr_queue_stat XX:XX:XX:XX:XX:XX 0|1" */
    memset_s((oal_uint8 *)&st_write_msg, OAL_SIZEOF(st_write_msg), 0, OAL_SIZEOF(st_write_msg));
    memset_s((oal_uint8 *)&st_usr_queue_param, OAL_SIZEOF(st_usr_queue_param), 0, OAL_SIZEOF(st_usr_queue_param));

    /* 获取用户mac地址 */
    ul_ret = wal_hipriv_get_mac_addr(pc_param, st_usr_queue_param.auc_user_macaddr, &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_usr_queue_stat::wal_hipriv_get_mac_addr return [%d].}", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_usr_queue_stat::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    st_usr_queue_param.uc_param = (oal_uint8)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_USR_QUEUE_STAT, OAL_SIZEOF(st_usr_queue_param));

    /* 填写消息体，参数 */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
                 &st_usr_queue_param, OAL_SIZEOF(st_usr_queue_param)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_usr_queue_stat::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_usr_queue_param),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_usr_queue_stat::wal_send_cfg_event return err code [%d]!}\r\n",
                         l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32 wal_hipriv_set_ampdu_aggr_num(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    mac_cfg_aggr_num_stru st_aggr_num_ctl = { 0 };
    oal_uint32 ul_ret;
    oal_int32 l_ret;

    memset_s((oal_uint8 *)&st_write_msg, OAL_SIZEOF(st_write_msg), 0, OAL_SIZEOF(st_write_msg));
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ampdu_aggr_num::get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    st_aggr_num_ctl.uc_aggr_num_switch = (oal_uint8)oal_atoi(ac_name);
    if (st_aggr_num_ctl.uc_aggr_num_switch == 0) {
        st_aggr_num_ctl.uc_aggr_num = 0; /* 不指定聚合个数时，聚合个数恢复为0 */
    } else {
        /* 获取聚合个数 */
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hipriv_set_ampdu_aggr_num::getonearg return errcode [%d]!}\r\n", ul_ret);
            return ul_ret;
        }

        st_aggr_num_ctl.uc_aggr_num = (oal_uint8)oal_atoi(ac_name);

        /* 超过聚合最大限制判断 */
        if (st_aggr_num_ctl.uc_aggr_num > WLAN_AMPDU_TX_MAX_NUM) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ampdu_aggr_num::exceed max aggr num [%d]!}\r\n",
                             st_aggr_num_ctl.uc_aggr_num);
            return ul_ret;
        }
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_AGGR_NUM, OAL_SIZEOF(st_aggr_num_ctl));

    /* 填写消息体，参数 */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
                 &st_aggr_num_ctl, OAL_SIZEOF(st_aggr_num_ctl)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_ampdu_aggr_num::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_aggr_num_ctl),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ampdu_aggr_num::wal_send_cfg_event return err code [%d]!}\r\n",
                         l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_stbc_cap(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_value;

    if (oal_unlikely((pst_cfg_net_dev == OAL_PTR_NULL) || (pc_param == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_hipriv_set_stbc_cap::cfg_net_dev or pc_param null ptr error %x, %x!}\r\n",
                       (uintptr_t)pst_cfg_net_dev, (uintptr_t)pc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* STBC设置开关的命令: hipriv "vap0 set_stbc_cap 0 | 1"
            此处将解析出"1"或"0"存入ac_name
 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_stbc_cap::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    /* 针对解析出的不同命令，设置TDLS禁用开关 */
    if (0 == (strcmp("0", ac_name))) {
        ul_value = 0;
    } else if (0 == (strcmp("1", ac_name))) {
        ul_value = 1;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_stbc_cap::the set stbc command is error %x!}\r\n",
                         (uintptr_t)ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_STBC_CAP, OAL_SIZEOF(oal_uint32));

    /* 设置配置命令参数 */
    *((oal_uint32 *)(st_write_msg.auc_value)) = ul_value;

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_stbc_cap::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_ldpc_cap(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_value;

    if (oal_unlikely((pst_cfg_net_dev == OAL_PTR_NULL) || (pc_param == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_ANY,
                       "{wal_hipriv_set_ldpc_cap::pst_cfg_net_dev or pc_param null ptr error %x, %x!}\r\n",
                       (uintptr_t)pst_cfg_net_dev, (uintptr_t)pc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* LDPC设置开关的命令: hipriv "vap0 set_ldpc_cap 0 | 1"
            此处将解析出"1"或"0"存入ac_name
 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ldpc_cap::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    /* 针对解析出的不同命令，设置TDLS禁用开关 */
    if (0 == (strcmp("0", ac_name))) {
        ul_value = 0;
    } else if (0 == (strcmp("1", ac_name))) {
        ul_value = 1;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ldpc_cap::the set ldpc command is error %x!}\r\n",
                         (uintptr_t)ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_LDPC_CAP, OAL_SIZEOF(oal_uint32));

    /* 设置配置命令参数 */
    *((oal_uint32 *)(st_write_msg.auc_value)) = ul_value;

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_ldpc_cap::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_NARROW_BAND

OAL_STATIC oal_uint32 wal_hipriv_narrow_bw(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
    mac_cfg_narrow_bw_stru *pst_nrw_bw;
    oal_uint32 ul_ret;
    oal_uint32 ul_off_set;
    oal_int8 ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    /* 入参合法判断 */
    if (oal_unlikely((pst_net_dev == OAL_PTR_NULL) || (pc_param == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_narrow_bw: input pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_NARROW_BW, OAL_SIZEOF(mac_cfg_narrow_bw_stru));

    /* 解析并设置配置命令参数 */
    pst_nrw_bw = (mac_cfg_narrow_bw_stru *)(st_write_msg.auc_value);

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_narrow_bw::get switch  [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param = pc_param + ul_off_set;

    pst_nrw_bw->en_open = (oal_uint8)oal_atoi(ac_arg);

    /* 窄带打开模式下强制关闭ampdu amsdu聚合和若干扰免疫算法 */
    /* 窄带下关闭rts */
    if (pst_nrw_bw->en_open == OAL_TRUE) {
        wal_hipriv_alg_cfg(pst_net_dev, "anti_inf_unlock_en 0");
        wal_hipriv_alg_cfg(pst_net_dev, "ar_rts_mode 0");
        wal_hipriv_ampdu_tx_on(pst_net_dev, "0");
        wal_hipriv_amsdu_tx_on(pst_net_dev, "0");
    } else {
        wal_hipriv_alg_cfg(pst_net_dev, "anti_inf_unlock_en 1");
        wal_hipriv_alg_cfg(pst_net_dev, "ar_rts_mode 1");
        wal_hipriv_ampdu_tx_on(pst_net_dev, "1");
        wal_hipriv_amsdu_tx_on(pst_net_dev, "1");
    }
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_narrow_bw::get switch  [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param = pc_param + ul_off_set;

    if (0 == (strcmp("1m", ac_arg))) {
        pst_nrw_bw->en_bw = NARROW_BW_1M;
    } else if (0 == (strcmp("5m", ac_arg))) {
        pst_nrw_bw->en_bw = NARROW_BW_5M;
    } else if (0 == (strcmp("10m", ac_arg))) {
        pst_nrw_bw->en_bw = NARROW_BW_10M;
    } else {
        pst_nrw_bw->en_bw = NARROW_BW_BUTT;
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_narrow_bw::bw should be 1/5/10 m");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_nrw_bw->uc_chn_extend = 0xff;
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg), &ul_off_set);
    if (ul_ret == OAL_SUCC) {
        pst_nrw_bw->uc_chn_extend = (oal_uint8)oal_atoi(ac_arg);
        if (pst_nrw_bw->uc_chn_extend >= WLAN_2G_HITALK_EXT_CHANNEL_NUM) {
            pst_nrw_bw->uc_chn_extend = 0xff;
        }
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_narrow_bw_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_hitalk_set(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_ret;
    oal_int8 ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_off_set;

    /* 入参合法判断 */
    if (oal_unlikely((pst_net_dev == OAL_PTR_NULL) || (pc_param == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_hitalk_set: input pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_hitalk_status = NARROW_BAND_ON_MASK;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_hitalk_set::no other arg [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    pc_param = pc_param + ul_off_set;

    if (0 == oal_strncmp(ac_arg, HIPRIV_NBFH, OAL_STRLEN(HIPRIV_NBFH))) {
        g_hitalk_status |= NBFH_ON_MASK;
    }

    return ul_ret;
}


OAL_STATIC oal_uint32 wal_hipriv_hitalk_bg_listen(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    mac_cfg_hitalk_listen_stru *pst_hitalk_listen;
    oal_uint32 ul_ret;
    oal_int8 ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_off_set;

    /* 入参合法判断 */
    if (oal_unlikely((pst_net_dev == OAL_PTR_NULL) || (pc_param == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_hitalk_set: input pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_HITALK_LISTEN, OAL_SIZEOF(mac_cfg_hitalk_listen_stru));

    /* 解析并设置配置命令参数 */
    pst_hitalk_listen = (mac_cfg_hitalk_listen_stru *)(st_write_msg.auc_value);

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_hitalk_bg_listen::get enable[%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param = pc_param + ul_off_set;
    pst_hitalk_listen->en_enable = (oal_uint8)oal_atoi(ac_arg);

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_hitalk_bg_listen::get idle time [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param = pc_param + ul_off_set;
    pst_hitalk_listen->us_work_time_ms = (oal_uint16)oal_atoi(ac_arg);

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_hitalk_bg_listen::get listen time [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param = pc_param + ul_off_set;
    pst_hitalk_listen->us_listen_time_ms = (oal_uint16)oal_atoi(ac_arg);

    oam_warning_log3(0, OAM_SF_ANY,
                     "{wal_hipriv_hitalk_bg_listen: enable=%d work=%d listen=%d", pst_hitalk_listen->en_enable,
                     pst_hitalk_listen->us_work_time_ms,
                     pst_hitalk_listen->us_listen_time_ms);

    ul_ret = wal_send_cfg_event(pst_net_dev,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_hitalk_listen_stru),
                                (oal_uint8 *)&st_write_msg,
                                OAL_FALSE,
                                OAL_PTR_NULL);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        return (oal_uint32)ul_ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32 wal_hipriv_bindcpu(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_int32 l_ret;
    oal_uint32 ul_ret;
    wal_msg_write_stru st_write_msg;
    mac_cfg_set_bindcpu_stru *pst_bindcpu = OAL_PTR_NULL;
    oal_uint8 uc_thread_id;
    oal_uint8 uc_cpumask = 0;

    oal_uint32 ul_off_set;
    oal_uint8 uc_param;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret == OAL_ERR_CODE_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{wal_hipriv_bindcpu::roam_start cfg_cmd error[%d]}", ul_ret);
        return ul_ret;
    } else if (ul_ret == OAL_SUCC) {
        uc_param = (oal_uint8)oal_atoi(ac_name);

        uc_thread_id = uc_param;

        pc_param += ul_off_set;

        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
        if (ul_ret == OAL_ERR_CODE_PTR_NULL) {
            OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{wal_hipriv_bindcpu::bindcpu cfg_cmd error[%d]}", ul_ret);
            return ul_ret;
        } else if (ul_ret == OAL_SUCC) {
            uc_param = (oal_uint8)oal_atoi(ac_name);
            uc_cpumask = uc_param;
        }
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{wal_hipriv_bindcpu::bindcpu cfg_cmd error[%d]}", ul_ret);
        return ul_ret;
    }
    oam_warning_log2(0, OAM_SF_ROAM,
                     "{wal_hipriv_bindcpu::bindcpu thread[%d](0:hisi_hcc;1:hmac_rxdata;2:rx_tsk), cpumask[%d]}",
                     uc_thread_id, uc_cpumask);
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    memset_s(&st_write_msg, OAL_SIZEOF(wal_msg_write_stru), 0, OAL_SIZEOF(wal_msg_write_stru));
    pst_bindcpu = (mac_cfg_set_bindcpu_stru *)(st_write_msg.auc_value);
    pst_bindcpu->uc_thread_id = uc_thread_id;
    pst_bindcpu->uc_thread_mask = uc_cpumask;
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_BINDCPU, OAL_SIZEOF(mac_cfg_set_bindcpu_stru));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_set_bindcpu_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{wal_hipriv_bindcpu::bindcpu cfg_cmd error[%d]}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32 wal_hipriv_napi_weight(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_int32 l_ret;
    oal_uint32 ul_ret;
    wal_msg_write_stru st_write_msg;
    mac_cfg_set_napi_weight_stru *pst_napiweight = OAL_PTR_NULL;
    oal_bool_enum_uint8 en_napi_weight_adjust;
    oal_uint8 uc_napi_weight = 16;

    oal_uint32 ul_off_set;
    oal_uint8 uc_param;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret == OAL_ERR_CODE_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{wal_hipriv_napi_weight::napi_weight cfg_cmd error[%d]}", ul_ret);
        return ul_ret;
    } else if (ul_ret == OAL_SUCC) {
        uc_param = (oal_uint8)oal_atoi(ac_name);

        en_napi_weight_adjust = (uc_param > 0) ? OAL_TRUE : OAL_FALSE;

        pc_param += ul_off_set;

        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
        if (ul_ret == OAL_ERR_CODE_PTR_NULL) {
            OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{wal_hipriv_napi_weight::napi_weight cfg_cmd error[%d]}", ul_ret);
            return ul_ret;
        } else if (ul_ret == OAL_SUCC) {
            uc_param = (oal_uint8)oal_atoi(ac_name);
            uc_napi_weight = uc_param;
        }
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{wal_hipriv_napi_weight::napi_weight cfg_cmd error[%d]}", ul_ret);
        return ul_ret;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    memset_s(&st_write_msg, OAL_SIZEOF(wal_msg_write_stru), 0, OAL_SIZEOF(wal_msg_write_stru));
    pst_napiweight = (mac_cfg_set_napi_weight_stru *)(st_write_msg.auc_value);
    pst_napiweight->en_napi_weight_adjust = en_napi_weight_adjust;
    pst_napiweight->uc_napi_weight = uc_napi_weight;
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_NAPIWEIGHT, OAL_SIZEOF(mac_cfg_set_napi_weight_stru));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_set_napi_weight_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{wal_hipriv_napi_weight::napi_weight cfg_cmd error[%d]}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

oal_uint32 wal_hipriv_dump_rx_dscr(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_value;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_dump_rx_dscr::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    ul_value = (oal_uint8)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DUMP_RX_DSCR, OAL_SIZEOF(oal_uint32));

    /* 设置配置命令参数 */
    *((oal_uint32 *)(st_write_msg.auc_value)) = ul_value;

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_dump_rx_dscr::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_dump_tx_dscr(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_value;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_dump_tx_dscr::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    ul_value = (oal_uint8)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DUMP_TX_DSCR, OAL_SIZEOF(oal_uint32));

    /* 设置配置命令参数 */
    *((oal_uint32 *)(st_write_msg.auc_value)) = ul_value;

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_dump_tx_dscr::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_dump_memory(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_addr[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int8 ac_len[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_len;
    oal_uint32 ul_addr;
    mac_cfg_dump_memory_stru *pst_cfg = OAL_PTR_NULL;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_addr, OAL_SIZEOF(ac_addr), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_dump_memory::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    pc_param += ul_off_set;
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_len, OAL_SIZEOF(ac_len), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_dump_memory::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    /* 地址字符串转成16位地址 */
    ul_addr = (oal_uint32)oal_strtol(ac_addr, 0, 16);
    ul_len = (oal_uint32)oal_atoi(ac_len);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DUMP_MEMORY, OAL_SIZEOF(mac_cfg_dump_memory_stru));

    /* 设置配置命令参数 */
    pst_cfg = (mac_cfg_dump_memory_stru *)(st_write_msg.auc_value);

    pst_cfg->ul_addr = ul_addr;
    pst_cfg->ul_len = ul_len;

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_dump_memory_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_dump_memory::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_void wal_hipriv_packet_check_init(mac_vap_stru *pst_mac_vap, hmac_device_stru *pst_hmac_device,
                                                 hmac_scan_state_enum_uint8 *pen_bgscan_flag)
{
    oal_uint32 ul_rx_filter_val;

    /* 关闭扫描,打开帧过滤 */
    *pen_bgscan_flag = hmac_scan_get_en_bgscan_enable_flag();
    hmac_scan_set_en_bgscan_enable_flag(HMAC_SCAN_DISABLE);

    ul_rx_filter_val = g_rx_filter_val & WLAN_NO_FILTER_EQ_LOCAL;

    hmac_set_rx_filter_send_event(pst_mac_vap, ul_rx_filter_val);

    pst_hmac_device->st_packet_check.en_pkt_check_completed = OAL_FALSE;
    pst_hmac_device->st_packet_check.en_pkt_check_result = OAL_FALSE;
}

OAL_STATIC oal_void wal_hipriv_packet_check_deinit(
    mac_vap_stru *pst_mac_vap, hmac_device_stru *pst_hmac_device, hmac_scan_state_enum_uint8 en_bgscan_flag)
{
    pst_hmac_device->st_packet_check.en_pkt_check_on = OAL_FALSE;

    /* 恢复扫描,关闭帧过滤 */
    hmac_scan_set_en_bgscan_enable_flag(en_bgscan_flag);
    hmac_set_rx_filter_send_event(pst_mac_vap, g_rx_filter_val);
}


oal_uint32 wal_hipriv_packet_check_send_event(
    oal_net_device_stru *pst_net_dev, mac_vap_stru *pst_mac_vap, hmac_device_stru *pst_hmac_device,
    oal_uint8 uc_packet_num, oal_uint8 uc_packet_thr, oal_uint8 uc_packet_timeout)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
    oal_int32 i_leftime;
    oal_uint32 ul_start_time;
    hmac_scan_state_enum_uint8 en_bgscan_flag;
    oal_uint16 us_pkt_check_num;
    mac_cfg_packet_check_param_stru *pst_pkt_check_tx_param = OAL_PTR_NULL;
    hmac_packet_check_rx_info_stru *pst_pkt_check_rx_info = hmac_get_pkt_check_rx_info_addr();

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_hipriv_packet_check_init(pst_mac_vap, pst_hmac_device, &en_bgscan_flag);

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CHECK_PACKET, OAL_SIZEOF(mac_cfg_packet_check_param_stru));

    /* 设置配置命令参数 */
    pst_pkt_check_tx_param = (mac_cfg_packet_check_param_stru *)(st_write_msg.auc_value);
    pst_pkt_check_tx_param->uc_packet_num = uc_packet_num;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_packet_check_param_stru),
                               (oal_uint8 *)&st_write_msg, OAL_FALSE, OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        wal_hipriv_packet_check_deinit(pst_mac_vap, pst_hmac_device, en_bgscan_flag);
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_packet_check::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }
    ul_start_time = (oal_uint32)oal_time_get_stamp_ms();

    /* 阻塞等待dmac上报 */
    i_leftime = oal_wait_event_interruptible_timeout(pst_hmac_device->st_packet_check.st_check_wait_q,
        (oal_uint32)(pst_hmac_device->st_packet_check.en_pkt_check_completed == OAL_TRUE),
        (oal_uint32)oal_msecs_to_jiffies(WLAN_PACKET_CHECK_TIMEOUT * uc_packet_timeout));

    wal_hipriv_packet_check_deinit(pst_mac_vap, pst_hmac_device, en_bgscan_flag);

    if (i_leftime == 0) {
        /* 定时器超时,超时后判定成功条件:收到包数量是发包数量的10% */
        us_pkt_check_num = (pst_pkt_check_rx_info->uc_pkt_check_num0 + pst_pkt_check_rx_info->uc_pkt_check_num1 +
             pst_pkt_check_rx_info->uc_pkt_check_num2 + pst_pkt_check_rx_info->uc_pkt_check_num3) *
            WLAN_PACKET_CHECK_PER;
        if (us_pkt_check_num >=
            (pst_pkt_check_rx_info->us_pkt_check_send_num * WLAN_PACKET_CHECK_TYPE_NUM * uc_packet_thr)) {
            pst_hmac_device->st_packet_check.en_pkt_check_result = OAL_TRUE;
        } else {
            pst_hmac_device->st_packet_check.en_pkt_check_result = OAL_FALSE;
        }
        oam_warning_log2(0, OAM_SF_ANY, "{wal_hipriv_packet_check::check timeout! result = %d,runtime[%d]}",
                         pst_hmac_device->st_packet_check.en_pkt_check_result,
                         oal_time_get_runtime(ul_start_time, (oal_uint32)oal_time_get_stamp_ms()));
        return OAL_SUCC;
    } else if (i_leftime < 0) {
        /* 定时器内部错误 */
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_packet_check::check error! result = %d}\r\n",
                         pst_hmac_device->st_packet_check.en_pkt_check_result);
        return -OAL_EINVAL;
    } else {
        /* 正常结束 */
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{checkdone result[%d]}", pst_hmac_device->st_packet_check.en_pkt_check_result);
        return OAL_SUCC;
    }
    return OAL_SUCC;
}

OAL_STATIC oal_uint32 wal_hipriv_packet_check_get_param(oal_int8 *pc_param, oal_uint8 *puc_packet_num,
                                                        oal_uint8 *puc_packet_thr, oal_uint8 *puc_packet_timeout)
{
    oal_uint32 ul_off_set = 0;
    oal_uint32 ul_ret;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    oal_int8 ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = { 0 };
    oal_bool_enum_uint8 en_cmd_updata = OAL_FALSE;
    oal_int8 *pc_value = OAL_PTR_NULL;
    oal_int8 *pc_name = OAL_PTR_NULL;

    do {
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
        if ((ul_ret != OAL_SUCC) && (ul_off_set != 0)) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{packet_check_get_param::cmd format err, ret:%d;!!}\r\n", ul_ret);
            return ul_ret;
        }
        pc_param += ul_off_set;

        if (en_cmd_updata == OAL_FALSE) {
            en_cmd_updata = OAL_TRUE;
        } else if (ul_off_set == 0) {
            break;
        }

        ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
        if (ul_ret != OAL_SUCC) {
            OAL_IO_PRINT("CMD format::sh hipriv.sh 'wlan0 packet_check [num 0-100] [thr 0-100] [time 0-10]'");
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hipriv.sh 'wlan0 packet_check [num 0-100] [thr 0-100] [time 0-10]'!!}\r\n");
            return ul_ret;
        }
        pc_param += ul_off_set;
        ul_off_set = 0;

        /* 查询各个命令 */
        pc_name = ac_name;
        pc_value = ac_value;
        if (oal_strcmp("num", pc_name) == 0) {
            *puc_packet_num = (oal_uint8)oal_atoi(pc_value);
        } else if (oal_strcmp("thr", pc_name) == 0) {
            *puc_packet_thr = (oal_uint8)oal_atoi(pc_value);
            if (*puc_packet_thr > WLAN_PACKET_CHECK_PER) {
                *puc_packet_thr = WLAN_PACKET_CHECK_THR;
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{packet_check_get_param::thr error, set uc_packet_thr = [%d]}\r\n",
                                 *puc_packet_thr);
            }
        } else if (oal_strcmp("time", pc_name) == 0) {
            *puc_packet_timeout = (oal_uint8)oal_atoi(pc_value);
        } else {
            OAL_IO_PRINT("CMD format::sh hipriv.sh 'wlan0 packet_check [num 0-100] [thr 0-100] [time 0-10]'");
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hipriv.sh 'wlan0 packet_check [num 0-100] [thr 0-100] [time 0-10]'!!}\r\n");
            return OAL_FAIL;
        }
    } while (*pc_param != '\0');
    return OAL_SUCC;
}

OAL_STATIC oal_uint32 wal_hipriv_packet_check(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_ret;
    oal_uint8 uc_packet_num = 10;                    /* 默认发包数量为10 */
    oal_uint8 uc_packet_thr = WLAN_PACKET_CHECK_THR; /* 默认门限 */
    oal_uint8 uc_packet_timeout = 1;                 /* 默认超时时间是1s */
    mac_vap_stru *pst_mac_vap;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;

    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_packet_check::oal_net_dev_priv, return null!}");
        return -OAL_EINVAL;
    }
    if (pst_mac_vap->en_vap_state != MAC_VAP_STATE_UP) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_packet_check::vap is not up!}");
        return -OAL_EINVAL;
    }
    if (!IS_LEGACY_STA(pst_mac_vap)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_packet_check::not STA!}");
        return -OAL_EINVAL;
    }
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_packet_check::hmac_res_get_mac_dev null. device_id:%u}",
                       pst_mac_vap->uc_device_id);
        return -OAL_EINVAL;
    }
    ul_ret = wal_hipriv_packet_check_get_param(pc_param, &uc_packet_num, &uc_packet_thr, &uc_packet_timeout);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    oam_warning_log3(0, OAM_SF_ANY,
                     "{wal_hipriv_packet_check::packet_num = [%d], packet_thr = [%d], packet_timeout = [%d]}\r\n",
                     uc_packet_num, uc_packet_thr, uc_packet_timeout);

    return wal_hipriv_packet_check_send_event(pst_net_dev, pst_mac_vap, pst_hmac_device, uc_packet_num, uc_packet_thr,
                                              uc_packet_timeout);
}

OAL_STATIC oal_uint32 wal_hipriv_alg(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
    oal_uint32 ul_off_set;
    oal_int8 ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int8 *pc_tmp = (oal_int8 *)pc_param;
    oal_uint16 us_config_len;
    oal_uint16 us_param_len;
    oal_int32 l_memcpy_ret;

    mac_ioctl_alg_config_stru st_alg_config;

    st_alg_config.uc_argc = 0;
    while (OAL_SUCC == wal_get_cmd_one_arg(pc_tmp, ac_arg, OAL_SIZEOF(ac_arg), &ul_off_set)) {
        st_alg_config.auc_argv_offset[st_alg_config.uc_argc] =
           (oal_uint8)((oal_uint8)(pc_tmp - pc_param) + (oal_uint8)ul_off_set - (oal_uint8)OAL_STRLEN(ac_arg));
        pc_tmp += ul_off_set;
        st_alg_config.uc_argc++;

        if (st_alg_config.uc_argc > DMAC_ALG_CONFIG_MAX_ARG) {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_alg::wal_hipriv_alg error, argc too big [%d]!}\r\n",
                           st_alg_config.uc_argc);
            return OAL_FAIL;
        }
    }

    if (st_alg_config.uc_argc == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_alg::argc=0!}\r\n");
        return OAL_FAIL;
    }

    us_param_len = (oal_uint16)OAL_STRLEN(pc_param);
    if (us_param_len > WAL_MSG_WRITE_MAX_LEN - 1 - OAL_SIZEOF(mac_ioctl_alg_config_stru)) {
        return OAL_FAIL;
    }

    us_config_len = OAL_SIZEOF(mac_ioctl_alg_config_stru) + us_param_len + 1;
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ALG, us_config_len);
    l_memcpy_ret = memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value), &st_alg_config,
                            OAL_SIZEOF(mac_ioctl_alg_config_stru));
    l_memcpy_ret += memcpy_s(st_write_msg.auc_value + OAL_SIZEOF(mac_ioctl_alg_config_stru),
                             OAL_SIZEOF(st_write_msg.auc_value) - OAL_SIZEOF(mac_ioctl_alg_config_stru),
                             pc_param, us_param_len + 1);
    if (l_memcpy_ret != EOK) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_alg::memcpy fail! [%d]}", l_memcpy_ret);
        return OAL_FAIL;
    }
    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_config_len,
                               (oal_uint8 *)&st_write_msg, OAL_FALSE, OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_alg::wal_send_cfg_event return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_show_stat_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
#ifdef _PRE_PRODUCT_ID_HI110X_HOST
    oam_stats_report_info_to_sdt(OAM_OTA_TYPE_DEV_STAT_INFO);
    oam_stats_report_info_to_sdt(OAM_OTA_TYPE_VAP_STAT_INFO);
#endif
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_show_vap_pkt_stat(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;

    /***************************************************************************
                                 抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_VAP_PKT_STAT, OAL_SIZEOF(oal_uint32));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_show_vap_pkt_stat::wal_send_cfg_event return err code [%d]!}\r\n",
                         l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_cca_opt_log(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    mac_ioctl_alg_cca_opt_log_param_stru *pst_alg_cca_opt_log_param = OAL_PTR_NULL;
    wal_ioctl_alg_cfg_stru st_alg_cfg;
    oal_uint8 uc_map_index = 0;
    oal_int32 l_ret;
    pst_alg_cca_opt_log_param = (mac_ioctl_alg_cca_opt_log_param_stru *)(st_write_msg.auc_value);

    /* 获取配置参数名称 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_cca_opt_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param = pc_param + ul_off_set;

    /* 寻找匹配的命令 */
    st_alg_cfg = g_ast_alg_cfg_map[0];
    while (st_alg_cfg.pc_name != OAL_PTR_NULL) {
        if (0 == strcmp(st_alg_cfg.pc_name, ac_name)) {
            break;
        }
        st_alg_cfg = g_ast_alg_cfg_map[++uc_map_index];
    }

    /* 没有找到对应的命令，则报错 */
    if (st_alg_cfg.pc_name == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_cca_opt_log::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* 记录命令对应的枚举值 */
    pst_alg_cca_opt_log_param->en_alg_cfg = g_ast_alg_cfg_map[uc_map_index].en_alg_cfg;

    /* 区分获取特定帧功率和统计日志命令处理:获取功率只需获取帧名字 */
    if (pst_alg_cca_opt_log_param->en_alg_cfg == MAC_ALG_CFG_CCA_OPT_STAT_LOG_START) {
        /* 获取配置参数名称 */
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_cca_opt_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                             ul_ret);
            return ul_ret;
        }

        /* 记录参数 */
        pst_alg_cca_opt_log_param->us_value = (oal_uint16)oal_atoi(ac_name);
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ALG_PARAM, OAL_SIZEOF(mac_ioctl_alg_cca_opt_log_param_stru));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_ioctl_alg_cca_opt_log_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_clear_stat_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
#ifdef _PRE_PRODUCT_ID_HI110X_HOST
    oam_stats_clear_stat_info();
#endif
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_user_stat_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
#ifdef _PRE_PRODUCT_ID_HI110X_HOST

    oal_int32 l_tmp;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;

    /* sh hipriv.sh "Hisilicon0 usr_stat_info usr_id" */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_user_stat_info::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    l_tmp = oal_atoi(ac_name);
    if ((l_tmp < 0) || (l_tmp >= WLAN_ACTIVE_USER_MAX_NUM)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_user_stat_info::user id invalid [%d]!}\r\n", l_tmp);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    oam_stats_report_usr_info((oal_uint16)l_tmp);
#endif
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_timer_start(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint8 uc_timer_switch;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_timer_start::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    uc_timer_switch = (oal_uint8)oal_atoi(ac_name);
    if (uc_timer_switch >= 2) { /* 整型值uc_timer_switch大于2是非法值 */
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_timer_start::invalid choicee [%d]!}\r\n", uc_timer_switch);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_TIMER_START, OAL_SIZEOF(oal_uint8));

    /* 设置配置命令参数 */
    *((oal_uint8 *)(st_write_msg.auc_value)) = uc_timer_switch;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_timer_start::wal_send_cfg_event return err code [%d]!}\r\n",
                         l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_show_profiling(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_DFT_STAT

OAL_STATIC oal_uint32 wal_hipriv_clear_vap_stat_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint8 uc_vap_id;

    if (OAL_PTR_NULL == oal_net_dev_priv(pst_net_dev)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_clear_vap_stat_info::oal_net_dev_priv(pst_net_dev) is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_vap_id = ((mac_vap_stru *)oal_net_dev_priv(pst_net_dev))->uc_vap_id;
    oam_stats_clear_vap_stat_info(uc_vap_id);

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_DFR

OAL_STATIC oal_uint32 wal_hipriv_test_dfr_start(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_wireless_dev_stru *pst_wdev;
    mac_wiphy_priv_stru *pst_wiphy_priv;
    mac_device_stru *pst_mac_device;

    pst_wdev = oal_netdevice_wdev(pst_net_dev);
    if (pst_wdev == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_test_dfr_start::pst_wdev is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_wiphy_priv = (mac_wiphy_priv_stru *)oal_wiphy_priv(pst_wdev->wiphy);
    if (pst_wiphy_priv == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_test_dfr_start::pst_wiphy_priv is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_device = pst_wiphy_priv->pst_mac_device;
    if (pst_mac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_test_dfr_start::pst_mac_device is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    g_st_dfr_info.bit_device_reset_enable = OAL_TRUE;
    g_st_dfr_info.bit_device_reset_process_flag = OAL_FALSE;

    wal_dfr_excp_rx(pst_mac_device->uc_device_id, 0);
    return OAL_SUCC;
}

#endif  // _PRE_WLAN_FEATURE_DFR


OAL_STATIC oal_uint32 wal_hipriv_ar_log(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    mac_ioctl_alg_ar_log_param_stru *pst_alg_ar_log_param = OAL_PTR_NULL;
    wal_ioctl_alg_cfg_stru st_alg_cfg;
    oal_uint8 uc_map_index = 0;
    oal_int32 l_ret;
    oal_bool_enum_uint8 en_stop_flag = OAL_FALSE;

    pst_alg_ar_log_param = (mac_ioctl_alg_ar_log_param_stru *)(st_write_msg.auc_value);

    /* 获取配置参数名称 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ar_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param = pc_param + ul_off_set;

    /* 寻找匹配的命令 */
    st_alg_cfg = g_ast_alg_cfg_map[0];
    while (st_alg_cfg.pc_name != OAL_PTR_NULL) {
        if (0 == strcmp(st_alg_cfg.pc_name, ac_name)) {
            break;
        }
        st_alg_cfg = g_ast_alg_cfg_map[++uc_map_index];
    }

    /* 没有找到对应的命令，则报错 */
    if (st_alg_cfg.pc_name == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ar_log::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* 记录命令对应的枚举值 */
    pst_alg_ar_log_param->en_alg_cfg = g_ast_alg_cfg_map[uc_map_index].en_alg_cfg;

    ul_ret = wal_hipriv_get_mac_addr(pc_param, pst_alg_ar_log_param->auc_mac_addr, &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ar_log::wal_hipriv_get_mac_addr failed!}\r\n");
        return ul_ret;
    }
    pc_param += ul_off_set;

    while ((*pc_param == ' ') || (*pc_param == '\0')) {
        if (*pc_param == '\0') {
            en_stop_flag = OAL_TRUE;
            break;
        }
        ++pc_param;
    }

    /* 获取业务类型值 */
    if (en_stop_flag != OAL_TRUE) {
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ar_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                             ul_ret);
            return ul_ret;
        }

        pst_alg_ar_log_param->uc_ac_no = (oal_uint8)oal_atoi(ac_name);
        pc_param = pc_param + ul_off_set;

        en_stop_flag = OAL_FALSE;
        while ((*pc_param == ' ') || (*pc_param == '\0')) {
            if (*pc_param == '\0') {
                en_stop_flag = OAL_TRUE;
                break;
            }
            ++pc_param;
        }

        if (en_stop_flag != OAL_TRUE) {
            /* 获取参数配置值 */
            ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
            if (ul_ret != OAL_SUCC) {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ar_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                                 ul_ret);
                return ul_ret;
            }

            /* 记录参数配置值 */
            pst_alg_ar_log_param->us_value = (oal_uint16)oal_atoi(ac_name);
        }
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ALG_PARAM, OAL_SIZEOF(mac_ioctl_alg_ar_log_param_stru));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_ioctl_alg_ar_log_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32 wal_hipriv_txbf_log(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    mac_ioctl_alg_txbf_log_param_stru *pst_alg_txbf_log_param = OAL_PTR_NULL;
    wal_ioctl_alg_cfg_stru st_alg_cfg;
    oal_uint8 uc_map_index = 0;
    oal_int32 l_ret;
    oal_bool_enum_uint8 en_stop_flag = OAL_FALSE;
    pst_alg_txbf_log_param = (mac_ioctl_alg_txbf_log_param_stru *)(st_write_msg.auc_value);
    /* 获取配置参数名称 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_txbf_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param = pc_param + ul_off_set;
    /* 寻找匹配的命令 */
    st_alg_cfg = g_ast_alg_cfg_map[0];
    while (st_alg_cfg.pc_name != OAL_PTR_NULL) {
        if (0 == strcmp(st_alg_cfg.pc_name, ac_name)) {
            break;
        }
        st_alg_cfg = g_ast_alg_cfg_map[++uc_map_index];
    }
    /* 没有找到对应的命令，则报错 */
    if (st_alg_cfg.pc_name == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ar_log::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }
    /* 记录命令对应的枚举值 */
    pst_alg_txbf_log_param->en_alg_cfg = g_ast_alg_cfg_map[uc_map_index].en_alg_cfg;
    ul_ret = wal_hipriv_get_mac_addr(pc_param, pst_alg_txbf_log_param->auc_mac_addr, &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_txbf_log::wal_hipriv_get_mac_addr failed!}\r\n");
        return ul_ret;
    }
    pc_param += ul_off_set;
    while (*pc_param == ' ') {
        ++pc_param;
        if (*pc_param == '\0') {
            en_stop_flag = OAL_TRUE;
            break;
        }
    }
    /* 获取参数配置值 */
    if (en_stop_flag != OAL_TRUE) {
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_txbf_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                             ul_ret);
            return ul_ret;
        }

        pst_alg_txbf_log_param->uc_ac_no = (oal_uint8)oal_atoi(ac_name);
        pc_param = pc_param + ul_off_set;

        en_stop_flag = OAL_FALSE;
        while ((*pc_param == ' ') || (*pc_param == '\0')) {
            if (*pc_param == '\0') {
                en_stop_flag = OAL_TRUE;
                break;
            }
            ++pc_param;
        }

        if (en_stop_flag != OAL_TRUE) {
            /* 获取参数配置值 */
            ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
            if (ul_ret != OAL_SUCC) {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_txbf_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                                 ul_ret);
                return ul_ret;
            }

            /* 记录参数配置值 */
            pst_alg_txbf_log_param->us_value = (oal_uint16)oal_atoi(ac_name);
        }
    }
    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ALG_PARAM, OAL_SIZEOF(mac_ioctl_alg_txbf_log_param_stru));
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_ioctl_alg_txbf_log_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        return (oal_uint32)l_ret;
    }
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_ar_test(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_offset;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    mac_ioctl_alg_ar_test_param_stru *pst_alg_ar_test_param = OAL_PTR_NULL;
    wal_ioctl_alg_cfg_stru st_alg_cfg;
    oal_uint8 uc_map_index = 0;
    oal_int32 l_ret;

    pst_alg_ar_test_param = (mac_ioctl_alg_ar_test_param_stru *)(st_write_msg.auc_value);

    /* 获取配置参数名称 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_offset);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ar_test::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param = pc_param + ul_offset;

    /* 寻找匹配的命令 */
    st_alg_cfg = g_ast_alg_cfg_map[0];
    while (st_alg_cfg.pc_name != OAL_PTR_NULL) {
        if (0 == strcmp(st_alg_cfg.pc_name, ac_name)) {
            break;
        }
        st_alg_cfg = g_ast_alg_cfg_map[++uc_map_index];
    }

    /* 没有找到对应的命令，则报错 */
    if (st_alg_cfg.pc_name == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ar_test::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* 记录命令对应的枚举值 */
    pst_alg_ar_test_param->en_alg_cfg = g_ast_alg_cfg_map[uc_map_index].en_alg_cfg;

    ul_ret = wal_hipriv_get_mac_addr(pc_param, pst_alg_ar_test_param->auc_mac_addr, &ul_offset);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ar_test::wal_hipriv_get_mac_addr failed!}\r\n");
        return ul_ret;
    }
    pc_param += ul_offset;

    /* 获取参数配置值 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_offset);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ar_test::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    /* 记录参数配置值 */
    pst_alg_ar_test_param->us_value = (oal_uint16)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ALG_PARAM, OAL_SIZEOF(mac_ioctl_alg_ar_test_param_stru));
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_ioctl_alg_ar_test_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP

OAL_STATIC oal_uint32 wal_hipriv_set_edca_opt_weight_sta(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint8 uc_weight;
    oal_uint8 *puc_value = 0;
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_off_set = 0;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    // sh hipriv.sh "vap0 set_edca_weight_sta 1"
    if (OAL_PTR_NULL == oal_net_dev_priv(pst_net_dev)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_edca_opt_weight_sta::oal_net_dev_priv(pst_net_dev) is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取mac_vap */
    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
        oam_warning_log0(0, OAM_SF_EDCA, "{wal_hipriv_set_edca_opt_cycle_ap:: only AP_MODE support}");
        return OAL_FAIL;
    }

    /* 获取参数值 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_EDCA,
                         "{wal_hipriv_set_edca_opt_cycle_ap::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    /*lint -e734*/
    uc_weight = (oal_uint32)oal_atoi(ac_name);
    /*lint +e734*/
    /* 最大权重为3 */
    if (uc_weight > 3) {
        oam_warning_log0(0, OAM_SF_EDCA, "wal_hipriv_set_edca_opt_weight_sta: valid value is between 0 and 3");
        return OAL_FAIL;
    }

    /* 申请事件内存 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_EDCA_OPT_WEIGHT_STA, OAL_SIZEOF(oal_uint8));
    puc_value = (oal_uint8 *)(st_write_msg.auc_value);
    *puc_value = uc_weight;

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_EDCA, "{wal_hipriv_set_edca_opt_weight_sta:: return err_code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_edca_opt_switch_ap(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint8 uc_flag;
    oal_uint8 *puc_value = 0;
    oal_uint32 ul_ret;
    oal_uint32 ul_off_set = 0;
    oal_int32 l_ret;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    // sh hipriv.sh "vap0 set_edca_switch_ap 1/0"
    if (OAL_PTR_NULL == oal_net_dev_priv(pst_net_dev)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_edca_opt_switch_ap::oal_net_dev_priv(pst_net_dev) is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取mac_vap */
    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log0(0, OAM_SF_EDCA, "{wal_hipriv_set_edca_opt_cycle_ap:: only AP_MODE support}");
        return OAL_FAIL;
    }

    /* 获取配置参数 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_EDCA,
                         "{wal_hipriv_set_edca_opt_cycle_ap::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    uc_flag = (oal_uint8)oal_atoi(ac_name);
    /* 非法配置参数 */
    if (uc_flag > 1) {
        oam_warning_log0(0, OAM_SF_EDCA, "wal_hipriv_set_edca_opt_cycle_ap, invalid config, should be 0 or 1");
        return OAL_SUCC;
    }

    /* 申请事件内存 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_EDCA_OPT_SWITCH_AP, OAL_SIZEOF(oal_uint8));
    puc_value = (oal_uint8 *)(st_write_msg.auc_value);
    *puc_value = uc_flag;

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_EDCA, "{wal_hipriv_set_edca_opt_switch_ap:: return err_code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_edca_opt_cycle_ap(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_cycle_ms;
    oal_uint32 *pul_value = 0;
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_off_set = 0;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    // sh hipriv.sh "vap0 set_edca_cycle_ap 200"
    if (OAL_PTR_NULL == oal_net_dev_priv(pst_net_dev)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_edca_opt_cycle_ap::oal_net_dev_priv(pst_net_dev) is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取mac_vap */
    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log0(0, OAM_SF_EDCA, "{wal_hipriv_set_edca_opt_cycle_ap:: only AP_MODE support}");
        return OAL_FAIL;
    }

    /* 获取参数值 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_EDCA,
                         "{wal_hipriv_set_edca_opt_cycle_ap::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    ul_cycle_ms = (oal_uint32)oal_atoi(ac_name);

    /* 申请事件内存 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_EDCA_OPT_CYCLE_AP, OAL_SIZEOF(oal_uint32));
    pul_value = (oal_uint32 *)(st_write_msg.auc_value);
    *pul_value = ul_cycle_ms;

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_EDCA, "{wal_hipriv_set_edca_opt_cycle_ap:: return err_code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32 wal_hipriv_set_default_key(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    mac_setdefaultkey_param_stru st_payload_params = { 0 };
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32 l_ret;
    oal_uint32 ul_ret;
    wal_msg_write_stru st_write_msg;

    /* 1.1 入参检查 */
    if ((pst_net_dev == OAL_PTR_NULL) || (pc_param == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY,
                       "{wal_hipriv_set_default_key::Param Check ERROR,pst_netdev, pst_params %x, %x!}\r\n",
                       (uintptr_t)pst_net_dev, (uintptr_t)pc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取key_index */
    ul_ret = wal_get_cmd_one_arg((oal_int8 *)pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ul_ret);
        return ul_ret;
    }
    st_payload_params.uc_key_index = (oal_uint8)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /* 获取en_unicast */
    ul_ret = wal_get_cmd_one_arg((oal_int8 *)pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ul_ret);
        return ul_ret;
    }
    st_payload_params.en_unicast = (oal_uint8)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /* 获取multicast */
    ul_ret = wal_get_cmd_one_arg((oal_int8 *)pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ul_ret);
        return ul_ret;
    }
    st_payload_params.en_multicast = (oal_uint8)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /* 3.2 填写 msg 消息体 */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
                 &st_payload_params, OAL_SIZEOF(mac_setdefaultkey_param_stru)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::memcpy fail!}");
        return OAL_FAIL;
    }
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DEFAULT_KEY, OAL_SIZEOF(mac_setdefaultkey_param_stru));
    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_setdefaultkey_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE, OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_default_key::wal_send_cfg_event return err_code [%d]!}\r\n",
                         l_ret);
        return (oal_uint32)l_ret;
    }
    return OAL_SUCC;
}

oal_int32 wal_hipriv_add_key_send_event(oal_net_device_stru *pst_net_dev, wal_msg_write_stru *pst_write_msg,
    wal_msg_stru *pst_rsp_msg, oal_uint16 us_len)
{
    oal_int32 l_ret;

    wal_write_msg_hdr_init(pst_write_msg, WLAN_CFGID_ADD_KEY, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
        (oal_uint8 *)pst_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_enable_pmf::wal_send_cfg_event return err_code [%d]!}", l_ret);
        return l_ret;
    }

    if (OAL_SUCC != wal_check_and_release_msg_resp(pst_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_hipriv_test_add_key::wal_check_and_release_msg_resp fail");
    }
    return OAL_SUCC;
}

OAL_STATIC oal_uint32 wal_hipriv_test_add_key(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    mac_addkey_param_stru st_payload_params;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int32 l_ret;
    oal_uint32 ul_ret;
    oal_uint32 ul_off_set;
    oal_uint8 auc_key[OAL_WPA_KEY_LEN] = { 0 };
    oal_int8 *pc_key = OAL_PTR_NULL;
    oal_uint32 ul_char_index;
    oal_uint16 us_len;
    wal_msg_stru *pst_rsp_msg = OAL_PTR_NULL;

    /* 1.1 入参检查 */
    if ((pst_net_dev == OAL_PTR_NULL) || (pc_param == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::Param Check ERROR,pst_netdev, pst_params %x, %x!}\r\n",
                       (uintptr_t)pst_net_dev, (uintptr_t)pc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* xxx(cipher) xx(en_pairwise) xx(key_len) xxx(key_index) xxxx:xx:xx:xx:xx:xx...(key 小于32字节)
     * xx:xx:xx:xx:xx:xx(目的地址)
 */
    memset_s(&st_payload_params, OAL_SIZEOF(st_payload_params), 0, OAL_SIZEOF(st_payload_params));
    memset_s(&st_payload_params.st_key, OAL_SIZEOF(mac_key_params_stru), 0, OAL_SIZEOF(mac_key_params_stru));
    st_payload_params.st_key.seq_len = 6; /* wpa seq长度设置为6 */
    memset_s(st_payload_params.auc_mac_addr, WLAN_MAC_ADDR_LEN, 0, WLAN_MAC_ADDR_LEN);

    /* 获取cipher */
    ul_ret = wal_get_cmd_one_arg((oal_int8 *)pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ul_ret);
        return ul_ret;
    }
    st_payload_params.st_key.cipher = (oal_uint32)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /* 获取en_pairwise */
    ul_ret = wal_get_cmd_one_arg((oal_int8 *)pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ul_ret);
        return ul_ret;
    }
    st_payload_params.en_pairwise = (oal_uint8)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /* 获取key_len */
    ul_ret = wal_get_cmd_one_arg((oal_int8 *)pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ul_ret);
        return ul_ret;
    }
    pc_param = pc_param + ul_off_set;
    st_payload_params.st_key.key_len = (oal_uint8)oal_atoi(ac_name);
    if ((st_payload_params.st_key.key_len > OAL_WPA_KEY_LEN) || (st_payload_params.st_key.key_len < 0)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::Param Check ERROR! key_len[%x]  }\r\n",
                       (oal_int32)st_payload_params.st_key.key_len);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 获取key_index */
    ul_ret = wal_get_cmd_one_arg((oal_int8 *)pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ul_ret);
        return ul_ret;
    }
    st_payload_params.uc_key_index = (oal_uint8)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /* 获取key */
    ul_ret = wal_get_cmd_one_arg((oal_int8 *)pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ul_ret);
        return ul_ret;
    }
    pc_param = pc_param + ul_off_set;
    pc_key = ac_name;
    /* 16进制转换 */
    for (ul_char_index = 0; ul_char_index < ul_off_set; ul_char_index++) {
        if (*pc_key == '-') {
            pc_key++;
            if (ul_char_index != 0) {
                ul_char_index--;
            }

            continue;
        }

        auc_key[ul_char_index / 2] = /* ul_char_index除以2 */
            /* 2、16作为16进制转换的运算数 */
           (oal_uint8)(auc_key[ul_char_index / 2] * 16 * (ul_char_index % 2) + oal_strtohex(pc_key));
        pc_key++;
    }
    l_ret = memcpy_s(st_payload_params.st_key.auc_key, OAL_WPA_KEY_LEN,
        auc_key, (oal_uint32)st_payload_params.st_key.key_len);

    /* 获取目的地址 */
    memset_s(ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, 0, WAL_HIPRIV_CMD_NAME_MAX_LEN);
    ul_ret = wal_get_cmd_one_arg((oal_int8 *)pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ul_ret);
        return ul_ret;
    }
    oal_strtoaddr(ac_name, st_payload_params.auc_mac_addr);
    pc_param = pc_param + ul_off_set;

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    /* 3.2 填写 msg 消息体 */
    us_len = (oal_uint32)OAL_SIZEOF(mac_addkey_param_stru);
    l_ret += memcpy_s((oal_int8 *)st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
        (oal_int8 *)&st_payload_params, (oal_uint32)us_len);
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_hipriv_test_add_key::memcpy fail");
        return OAL_FAIL;
    }
    l_ret = wal_hipriv_add_key_send_event(pst_net_dev, &st_write_msg, pst_rsp_msg, us_len);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        return (oal_uint32)l_ret;
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
OAL_STATIC oal_uint32 wal_hipriv_set_auto_freq(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_ret;
    oal_int32 l_cfg_rst;
    oal_uint16 us_len;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_off_set = 0;
    oal_uint8 uc_cmd_type;
    oal_uint8 uc_value;
    mac_cfg_set_auto_freq_stru *pst_set_auto_freq;

    /* 获取设定mib名称 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_hipriv_set_thruput_bypass::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    uc_cmd_type = (oal_uint8)oal_atoi(ac_name);

    /* 获取设定置 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_hipriv_set_thruput_bypass::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return (oal_uint32)ul_ret;
    }
    pc_param += ul_off_set;
    uc_value = (oal_uint8)oal_atoi(ac_name);

    pst_set_auto_freq = (mac_cfg_set_auto_freq_stru *)(st_write_msg.auc_value);
    pst_set_auto_freq->uc_cmd_type = uc_cmd_type;
    pst_set_auto_freq->uc_value = uc_value;
    us_len = OAL_SIZEOF(mac_cfg_set_thruput_bypass_stru);
    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_DEVICE_FREQ_ENABLE, us_len);

    l_cfg_rst = wal_send_cfg_event(pst_net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
    if (oal_unlikely(l_cfg_rst != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_thruput_bypass::wal_send_cfg_event return err_code [%d]!}\r\n",
                         l_cfg_rst);
        return (oal_uint32)l_cfg_rst;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32 wal_hipriv_set_auto_protection(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_ret;
    oal_int32 l_cfg_rst;
    oal_uint16 us_len;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_off_set = 0;
    oal_uint32 ul_auto_protection_flag;

    /* 获取mib名称 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_hipriv_set_auto_protection::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    ul_auto_protection_flag = (oal_uint32)oal_atoi(ac_name);

    us_len = OAL_SIZEOF(oal_uint32);
    *(oal_uint32 *)(st_write_msg.auc_value) = ul_auto_protection_flag;
    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_AUTO_PROTECTION, us_len);

    l_cfg_rst = wal_send_cfg_event(pst_net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);
    if (oal_unlikely(l_cfg_rst != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_auto_protection::wal_send_cfg_event return err_code [%d]!}",
                         l_cfg_rst);
        return (oal_uint32)l_cfg_rst;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_WAPI

#ifdef _PRE_WAPI_DEBUG
OAL_STATIC oal_uint32 wal_hipriv_show_wapi_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    mac_vap_stru *pst_mac_vap;
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
    mac_cfg_user_info_param_stru *pst_user_info_param;
    oal_uint8 auc_mac_addr[6] = { 0 }; /* 临时保存获取的use的mac地址信息 */
    oal_uint8 uc_char_index;
    oal_uint16 us_user_idx;

    /* 去除字符串的空格 */
    pc_param++;

    /* 获取mac地址,16进制转换 */
    for (uc_char_index = 0; uc_char_index < 12; uc_char_index++) { /* uc_char_index小于12 */
        if (*pc_param == ':') {
            pc_param++;
            if (uc_char_index != 0) {
                uc_char_index--;
            }

            continue;
        }

        auc_mac_addr[uc_char_index / 2] = /* ul_char_index除以2 */
            (oal_uint8)(auc_mac_addr[uc_char_index / 2] * 16 * (uc_char_index % 2) + /* 2、16作为16进制转换的运算数 */
                        oal_strtohex(pc_param));
        pc_param++;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_WAPI_INFO, OAL_SIZEOF(mac_cfg_user_info_param_stru));

    /* 根据mac地址找用户 */
    pst_mac_vap = oal_net_dev_priv(pst_net_dev);

    l_ret = (oal_int32)mac_vap_find_user_by_macaddr(pst_mac_vap, auc_mac_addr, &us_user_idx);
    if (l_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_user_info::no such user!}\r\n");
        return OAL_FAIL;
    }

    /* 设置配置命令参数 */
    pst_user_info_param = (mac_cfg_user_info_param_stru *)(st_write_msg.auc_value);
    pst_user_info_param->us_user_idx = us_user_idx;

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_hipriv_show_wapi_info::us_user_idx %u", us_user_idx);
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_user_info_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_user_info::return err code [%d]!}\r\n",
                         l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif /* #ifdef WAPI_DEBUG_MODE */

#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */

#ifdef _PRE_WLAN_FEATURE_FTM
OAL_STATIC wal_ftm_cmd_entry_stru g_ast_ftm_common_cmd[] = {
    { "enable_ftm_initiator", OAL_TRUE, BIT0 },
    { "enable",         OAL_TRUE, BIT2 },
    { "cali",         OAL_TRUE, BIT3 },
    { "enable_ftm_responder", OAL_TRUE, BIT5 },
    { "enable_ftm_range",     OAL_TRUE, BIT8 },
};
OAL_STATIC wal_ftm_cmd_entry_stru g_ast_ftm_send_req_cmd[] = {
    { "send_initial_ftm_request", OAL_TRUE, BIT1 },
    { "send_range_req",    OAL_TRUE, BIT7 },
    { "send_nbr_rpt_req",         OAL_TRUE, BIT15 },
    { "send_gas_init_req",        OAL_TRUE, BIT16 },
};

OAL_STATIC OAL_INLINE oal_void ftm_debug_cmd_format_info(void)
{
    oam_warning_log0(0, OAM_SF_FTM,
                     "{CMD format::hipriv.sh wlan0 ftm_debug \
    enable_ftm_initiator[0|1] \
    enable[0|1] \
    cali[0|1] \
    enable_ftm_responder[0|1] \
    set_ftm_time t1[] t2[] t3[] t4[] \
    set_ftm_timeout ul_bursts_timeout ul_ftms_timeout \
    enable_ftm_responder[0|1] \
    enable_ftm_range[0|1] \
    get_cali \
    !!}\r\n");
    oam_warning_log0(0, OAM_SF_FTM,
                     "{CMD format::hipriv.sh wlan0 ftm_debug \
    send_initial_ftm_request channel[] ftms_per_burst[n] burst_num[n] en_lci_civic_request[0|1] \
    asap[0|1] bssid[xx:xx:xx:xx:xx:xx] format_bw[9~13]\
    send_ftm bssid[xx:xx:xx:xx:xx:xx] \
    send_range_req mac[] num_rpt[0-65535] delay[] ap_cnt[] bssid[] channel[] bssid[] channel[] ...\
    set_location type[] mac[] mac[] mac[] \
    !!}\r\n");
}


OAL_STATIC oal_uint32 ftm_debug_parase_iftmr_cmd(oal_int8 *pc_param, mac_ftm_debug_switch_stru *pst_debug_info,
                                                 oal_uint32 *pul_offset)
{
    oal_uint32 ul_ret;
    oal_uint32 ul_off_set = 0;
    oal_uint8 auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };
    oal_int8 ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = { 0 };

    *pul_offset = 0;

    /* 命令格式:send_initial_ftm_request channel[] ftms_per_burst[n] burst_num[n]---代表(2^n)个burst
                        en_lci_civic_request[0|1] asap[0|1] bssid[xx:xx:xx:xx:xx:xx]
                        format_bw[9~13] */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get iftmr mode error,return.}");
        return ul_ret;
    }

    *pul_offset += ul_off_set;
    pst_debug_info->st_send_iftmr_bit1.uc_channel_num = (oal_uint8)oal_atoi(ac_value);
    pc_param += ul_off_set;
    ul_off_set = 0;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get uc_ftms_per_burst error,return.}");
        return ul_ret;
    }
    *pul_offset += ul_off_set;
    pst_debug_info->st_send_iftmr_bit1.uc_ftms_per_burst = (oal_uint8)oal_atoi(ac_value);
    pc_param += ul_off_set;
    ul_off_set = 0;

    /* 解析burst */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get burst error,return.}");
        return ul_ret;
    }
    *pul_offset += ul_off_set;
    pst_debug_info->st_send_iftmr_bit1.uc_burst_num = (oal_uint8)oal_atoi(ac_value);
    pc_param += ul_off_set;
    ul_off_set = 0;

    /* 解析LCI and Location Civic Measurement request使能标志 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get measure_req error,return.}");
        return ul_ret;
    }
    *pul_offset += ul_off_set;
    pst_debug_info->st_send_iftmr_bit1.measure_req = (oal_uint8)oal_atoi(ac_value);
    pc_param += ul_off_set;
    ul_off_set = 0;

    /* 解析asap,as soon as possible使能标志 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get measure_req error,return.}");
        return ul_ret;
    }
    *pul_offset += ul_off_set;
    pst_debug_info->st_send_iftmr_bit1.en_asap = (oal_uint8)oal_atoi(ac_value);
    pc_param += ul_off_set;
    ul_off_set = 0;

    /* 获取bssid */
    ul_ret = wal_hipriv_get_mac_addr(pc_param, auc_mac_addr, &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::No bssid,set the associated bssid.}");
        *pul_offset += ul_off_set;
        memset_s(pst_debug_info->st_send_iftmr_bit1.auc_bssid, OAL_SIZEOF(pst_debug_info->st_send_iftmr_bit1.auc_bssid),
                 0, OAL_SIZEOF(pst_debug_info->st_send_iftmr_bit1.auc_bssid));
        pc_param += ul_off_set;
        ul_off_set = 0;

        return OAL_SUCC;
    }
    *pul_offset += ul_off_set;
    oal_set_mac_addr(pst_debug_info->st_send_iftmr_bit1.auc_bssid, auc_mac_addr);
    pc_param += ul_off_set;
    ul_off_set = 0;

    /* 解析format bw */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get format bw error,return.}");
        return OAL_SUCC;
    }
    *pul_offset += ul_off_set;
    pst_debug_info->st_send_iftmr_bit1.uc_format_bw = (oal_uint8)oal_atoi(ac_value);
    pc_param += ul_off_set;
    ul_off_set = 0;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 ftm_debug_parase_ftm_cmd(oal_int8 *pc_param, mac_ftm_debug_switch_stru *pst_debug_info,
                                               oal_uint32 *pul_offset)
{
    oal_uint32 ul_ret;
    oal_int8 ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN];
    oal_uint32 ul_off_set = 0;

    /* 入参合法判断 */
    if (oal_unlikely((pc_param == OAL_PTR_NULL) || (pst_debug_info == OAL_PTR_NULL) || (pul_offset == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{ftm_debug_parase_ftm_cmd: input pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *pul_offset = 0;
    /* 解析mac */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_ftm_cmd::ger mac error.}");
        return OAL_FAIL;
    }
    *pul_offset += ul_off_set;
    oal_strtoaddr(ac_value, pst_debug_info->st_send_ftm_bit4.auc_mac);
    pc_param += ul_off_set;
    ul_off_set = 0;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 ftm_debug_parase_ftm_timeout_cmd(oal_int8 *pc_param,
                                                       mac_ftm_debug_switch_stru *pst_debug_info,
                                                       oal_uint32 *pul_offset)
{
    oal_uint32 ul_ret;
    oal_int8 ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN];
    oal_uint32 ul_off_set = 0;

    /* 入参合法判断 */
    if (oal_unlikely((pc_param == OAL_PTR_NULL) || (pst_debug_info == OAL_PTR_NULL) || (pul_offset == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{ftm_debug_parase_ftm_time_cmd: input pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *pul_offset = 0;
    /* 解析tm bursts timeout */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_ftm_time_cmd::get correct time1 error,return.}");
        return ul_ret;
    }

    *pul_offset += ul_off_set;
    pst_debug_info->st_ftm_timeout_bit14.ul_bursts_timeout = (oal_uint32)oal_atoi(ac_value);
    pc_param += ul_off_set;
    ul_off_set = 0;

    /* 解析ftms timeout */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_ftm_time_cmd::get correct time2 error,return.}");
        return ul_ret;
    }
    *pul_offset += ul_off_set;
    pst_debug_info->st_ftm_timeout_bit14.ul_ftms_timeout = (oal_uint32)oal_atoi(ac_value);
    pc_param += ul_off_set;
    ul_off_set = 0;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 ftm_debug_parase_range_req_cmd(oal_int8 *pc_param, mac_ftm_debug_switch_stru *pst_debug_info,
                                                     oal_uint32 *pul_offset)
{
    oal_uint32 ul_ret;
    oal_int8 ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN];
    oal_uint32 ul_off_set = 0;
    oal_uint8 uc_ap_cnt;

    /* 入参合法判断 */
    if (oal_unlikely((pc_param == OAL_PTR_NULL) || (pst_debug_info == OAL_PTR_NULL) || (pul_offset == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{ftm_debug_parase_range_req_cmd: input pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *pul_offset = 0;

    /* 解析mac */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_range_req_cmd::mac error!.}");
        return ul_ret;
    }
    *pul_offset += ul_off_set;
    oal_strtoaddr(ac_value, pst_debug_info->st_send_range_req_bit7.auc_mac);
    pc_param += ul_off_set;
    ul_off_set = 0;

    /* 解析num_rpt */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_range_req_cmd::get num_rpt error,return.}");
        return ul_ret;
    }
    *pul_offset += ul_off_set;
    pst_debug_info->st_send_range_req_bit7.us_num_rpt = (oal_uint16)oal_atoi(ac_value);
    pc_param += ul_off_set;
    ul_off_set = 0;

    /* 解析ap_cnt */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_range_req_cmd::get ap_cnt error,return.}");
        return ul_ret;
    }
    *pul_offset += ul_off_set;
    pst_debug_info->st_send_range_req_bit7.uc_minimum_ap_count = (oal_uint8)oal_atoi(ac_value);
    pc_param += ul_off_set;
    ul_off_set = 0;

    for (uc_ap_cnt = 0; uc_ap_cnt < pst_debug_info->st_send_range_req_bit7.uc_minimum_ap_count; uc_ap_cnt++) {
        /* 解析bssid */
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_range_req_cmd::bssid error!.}");
            return ul_ret;
        }
        *pul_offset += ul_off_set;
        oal_strtoaddr(ac_value, pst_debug_info->st_send_range_req_bit7.aauc_bssid[uc_ap_cnt]);
        pc_param += ul_off_set;
        ul_off_set = 0;

        /* 解析channel */
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_range_req_cmd::get channel error,return.}");
            return ul_ret;
        }

        *pul_offset += ul_off_set;
        pst_debug_info->st_send_range_req_bit7.auc_channel[uc_ap_cnt] = (oal_uint8)oal_atoi(ac_value);
        pc_param += ul_off_set;
        ul_off_set = 0;
    }
    return OAL_SUCC;
}

OAL_STATIC oal_uint8 wal_hipriv_ftm_is_common_cmd(oal_int8 ac_name[], oal_uint8 *puc_cmd_index)
{
    oal_uint8 uc_cmd_index;

    for (uc_cmd_index = 0; uc_cmd_index < OAL_SIZEOF(g_ast_ftm_common_cmd) /
         OAL_SIZEOF(wal_ftm_cmd_entry_stru);
         uc_cmd_index++) {
        if (strcmp(g_ast_ftm_common_cmd[uc_cmd_index].pc_cmd_name, ac_name) == 0) {
            break;
        }
    }

    *puc_cmd_index = uc_cmd_index;
    if (uc_cmd_index == OAL_SIZEOF(g_ast_ftm_common_cmd) / OAL_SIZEOF(wal_ftm_cmd_entry_stru)) {
        return OAL_FALSE;
    } else {
        return OAL_TRUE;
    }
}


OAL_STATIC oal_uint32 wal_hipriv_ftm_set_common_cmd(oal_int8 *pc_param, oal_uint8 uc_cmd_index,
                                                    mac_ftm_debug_switch_stru *pst_ftm_debug, oal_uint32 *pul_offset)
{
    oal_uint32 ul_ret;
    oal_uint32 ul_off_set = 0;
    oal_int8 ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN];
    *pul_offset = 0;

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ftm_set_common_cmd:: ftm common cmd [%d].}", uc_cmd_index);

    /* 取命令配置值 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        ftm_debug_cmd_format_info();
        return ul_ret;
    }

    /* 检查命令的配置值 */
    if ((g_ast_ftm_common_cmd[uc_cmd_index].uc_is_check_para) &&
        oal_value_not_in_valid_range(ac_value[0], '0', '9')) {
        ftm_debug_cmd_format_info();
        return OAL_FAIL;
    }

    *pul_offset += ul_off_set;
    ul_off_set = 0;

    /* 填写命令结构体 */
    switch (g_ast_ftm_common_cmd[uc_cmd_index].ul_bit) {
        case BIT0:
            pst_ftm_debug->en_ftm_initiator_bit0 = ((oal_uint8)oal_atoi(ac_value));
            pst_ftm_debug->ul_cmd_bit_map |= BIT0;
            break;
        case BIT2:
            pst_ftm_debug->en_enable_bit2 = ((oal_uint8)oal_atoi(ac_value));
            pst_ftm_debug->ul_cmd_bit_map |= BIT2;
            break;
        case BIT3:
            pst_ftm_debug->en_cali_bit3 = ((oal_uint8)oal_atoi(ac_value));
            pst_ftm_debug->ul_cmd_bit_map |= BIT3;
            break;
        case BIT5:
            pst_ftm_debug->en_ftm_resp_bit5 = ((oal_uint8)oal_atoi(ac_value));
            pst_ftm_debug->ul_cmd_bit_map |= BIT5;
            break;
        case BIT8:
            pst_ftm_debug->en_ftm_range_bit8 = ((oal_uint8)oal_atoi(ac_value));
            pst_ftm_debug->ul_cmd_bit_map |= BIT8;
            break;
        default:
            OAM_WARNING_LOG1(0, OAM_SF_FTM, "{wal_hipriv_ftm_set_common_cmd::invalid cmd bit[0x%x]!}",
                             g_ast_ftm_common_cmd[uc_cmd_index].ul_bit);
            break;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 ftm_debug_parase_neighbor_report_req_cmd(
    oal_int8 *pc_param, mac_ftm_debug_switch_stru *pst_debug_info, oal_uint32 *pul_offset)
{
    oal_uint32 ul_ret;
    oal_uint32 ul_off_set = 0;
    oal_uint8 auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };

    *pul_offset = 0;

    /* 解析bssid */
    ul_ret = wal_hipriv_get_mac_addr(pc_param, auc_mac_addr, &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_neighbor_report_req_cmd::No bssid,set the assoc bssid.}");
        *pul_offset += ul_off_set;
        memset_s(pst_debug_info->st_neighbor_report_req_bit15.auc_bssid,
                 OAL_SIZEOF(pst_debug_info->st_neighbor_report_req_bit15.auc_bssid),
                 0, OAL_SIZEOF(pst_debug_info->st_neighbor_report_req_bit15.auc_bssid));
        return ul_ret;
    }
    *pul_offset += ul_off_set;
    oal_set_mac_addr(pst_debug_info->st_neighbor_report_req_bit15.auc_bssid, auc_mac_addr);
    pc_param += ul_off_set;

    oam_warning_log3(0, OAM_SF_ANY,
                     "{ftm_debug_parase_neighbor_report_req_cmd::send neighbor request frame to ap[*:*:*:%x:%x:%x]}",
                     auc_mac_addr[3], auc_mac_addr[4], auc_mac_addr[5]); /* auc_mac_addr第3、4、5byte */

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 ftm_debug_parase_gas_init_req_cmd(oal_int8 *pc_param,
                                                        mac_ftm_debug_switch_stru *pst_debug_info,
                                                        oal_uint32 *pul_offset)
{
    oal_uint32 ul_ret;
    oal_int8 ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN];
    oal_uint32 ul_off_set = 0;
    oal_uint8 auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };

    /* 命令格式:send_gas_init_req lci_enable[0|1] bssid[xx:xx:xx:xx:xx:xx] */
    *pul_offset = 0;

    /* 解析lci_enable */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM,
                         "{ftm_debug_parase_gas_init_req_cmd::get lci_enable error, use default value [true].}");
        ac_value[0] = OAL_TRUE;
        return ul_ret;
    }
    *pul_offset += ul_off_set;
    pst_debug_info->st_gas_init_req_bit16.en_lci_enable = (oal_uint8)oal_atoi(ac_value);
    pc_param += ul_off_set;
    ul_off_set = 0;

    /* 解析bssid */
    ul_ret = wal_hipriv_get_mac_addr(pc_param, auc_mac_addr, &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_gas_init_req_cmd::No bssid,set the associated bssid.}");
        *pul_offset += ul_off_set;
        memset_s(pst_debug_info->st_gas_init_req_bit16.auc_bssid,
                 OAL_SIZEOF(pst_debug_info->st_gas_init_req_bit16.auc_bssid),
                 0, OAL_SIZEOF(pst_debug_info->st_gas_init_req_bit16.auc_bssid));
        pc_param += ul_off_set;
        ul_off_set = 0;
        return ul_ret;
    }
    *pul_offset += ul_off_set;
    oal_set_mac_addr(pst_debug_info->st_gas_init_req_bit16.auc_bssid, auc_mac_addr);
    pc_param += ul_off_set;
    ul_off_set = 0;

    oam_warning_log3(0, OAM_SF_ANY,
                     "{ftm_debug_parase_gas_init_req_cmd::send gas initial request frame to ap[*:*:*:%x:%x:%x]}",
                     auc_mac_addr[3], auc_mac_addr[4], auc_mac_addr[5]); /* auc_mac_addr第3、4、5byte */

    return OAL_SUCC;
}


OAL_STATIC oal_uint8 wal_hipriv_ftm_is_send_req_cmd(oal_int8 ac_name[], oal_uint8 *puc_cmd_index)
{
    oal_uint8 uc_cmd_index;

    for (uc_cmd_index = 0; uc_cmd_index < OAL_SIZEOF(g_ast_ftm_send_req_cmd) /
         OAL_SIZEOF(wal_ftm_cmd_entry_stru);
         uc_cmd_index++) {
        if (strcmp(g_ast_ftm_send_req_cmd[uc_cmd_index].pc_cmd_name, ac_name) == 0) {
            break;
        }
    }

    *puc_cmd_index = uc_cmd_index;
    if (uc_cmd_index == OAL_SIZEOF(g_ast_ftm_send_req_cmd) / OAL_SIZEOF(wal_ftm_cmd_entry_stru)) {
        return OAL_FALSE;
    } else {
        return OAL_TRUE;
    }
}


OAL_STATIC oal_uint32 wal_hipriv_ftm_set_send_req_cmd(oal_int8 *pc_param, oal_uint8 uc_cmd_index,
                                                      mac_ftm_debug_switch_stru *pst_ftm_debug, oal_uint32 *pul_offset)
{
    oal_uint32 ul_ret;
    oal_uint32 ul_off_set = 0;
    *pul_offset = 0;

    /* 填写命令结构体 */
    switch (g_ast_ftm_send_req_cmd[uc_cmd_index].ul_bit) {
        case BIT1:
            ul_ret = ftm_debug_parase_iftmr_cmd(pc_param, pst_ftm_debug, &ul_off_set);
            if (ul_ret != OAL_SUCC) {
                ftm_debug_cmd_format_info();
                return ul_ret;
            }
            *pul_offset += ul_off_set;
            ul_off_set = 0;
            pst_ftm_debug->ul_cmd_bit_map |= BIT1;
            break;
        case BIT7:
            ul_ret = ftm_debug_parase_range_req_cmd(pc_param, pst_ftm_debug, &ul_off_set);
            if (ul_ret != OAL_SUCC) {
                ftm_debug_cmd_format_info();
                return ul_ret;
            }
            *pul_offset += ul_off_set;
            ul_off_set = 0;
            pst_ftm_debug->ul_cmd_bit_map |= BIT7;
            break;
        case BIT15:
            ul_ret = ftm_debug_parase_neighbor_report_req_cmd(pc_param, pst_ftm_debug, &ul_off_set);
            if (ul_ret != OAL_SUCC) {
                ftm_debug_cmd_format_info();
                return ul_ret;
            }
            *pul_offset += ul_off_set;
            ul_off_set = 0;
            pst_ftm_debug->ul_cmd_bit_map |= BIT15;
            break;
        case BIT16:
            ul_ret = ftm_debug_parase_gas_init_req_cmd(pc_param, pst_ftm_debug, &ul_off_set);
            if (ul_ret != OAL_SUCC) {
                ftm_debug_cmd_format_info();
                return ul_ret;
            }
            *pul_offset += ul_off_set;
            ul_off_set = 0;
            pst_ftm_debug->ul_cmd_bit_map |= BIT16;
            break;
        default:
            break;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_ftm(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set = 0;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    mac_ftm_debug_switch_stru st_ftm_debug;
    oal_uint32 ul_ret = 0;
    oal_int32 l_ret;
    oal_uint8 uc_cmd_index = 0;

    /* 入参合法判断 */
    if (oal_unlikely(pst_net_dev == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_ftm: input pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(&st_ftm_debug, OAL_SIZEOF(st_ftm_debug), 0, OAL_SIZEOF(st_ftm_debug));

    do {
        /* 获取命令关键字 */
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
        if ((ul_ret != OAL_SUCC) && (ul_off_set != 0)) {
            ftm_debug_cmd_format_info();
            return ul_ret;
        }
        pc_param += ul_off_set;

        if (ul_off_set == 0) {
            break;
        }

        /* 命令分类 */
        if (wal_hipriv_ftm_is_common_cmd(ac_name, &uc_cmd_index)) {
            ul_ret = wal_hipriv_ftm_set_common_cmd(pc_param, uc_cmd_index, &st_ftm_debug, &ul_off_set);
            pc_param += ul_off_set;
            ul_off_set = 0;
            if (ul_ret != OAL_SUCC) {
                return ul_ret;
            }
        } else if (wal_hipriv_ftm_is_send_req_cmd(ac_name, &uc_cmd_index)) {
            ul_ret = wal_hipriv_ftm_set_send_req_cmd(pc_param, uc_cmd_index, &st_ftm_debug, &ul_off_set);
            pc_param += ul_off_set;
            ul_off_set = 0;
            if (ul_ret != OAL_SUCC) {
                oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ftm::send_req_cmd fail.}");
                return ul_ret;
            }
        } else if (strcmp("send_ftm", ac_name) == 0) {
            ul_ret = ftm_debug_parase_ftm_cmd(pc_param, &st_ftm_debug, &ul_off_set);
            if (ul_ret != OAL_SUCC) {
                ftm_debug_cmd_format_info();
                return ul_ret;
            }
            pc_param += ul_off_set;
            ul_off_set = 0;
            st_ftm_debug.ul_cmd_bit_map |= BIT4;
        } else if (strcmp("set_ftm_timeout", ac_name) == 0) {
            ul_ret = ftm_debug_parase_ftm_timeout_cmd(pc_param, &st_ftm_debug, &ul_off_set);
            if (ul_ret != OAL_SUCC) {
                ftm_debug_cmd_format_info();
                return ul_ret;
            }
            pc_param += ul_off_set;
            ul_off_set = 0;
            st_ftm_debug.ul_cmd_bit_map |= BIT14;
        } else if (strcmp("get_cali", ac_name) == 0) {
            st_ftm_debug.ul_cmd_bit_map |= BIT9;
        } else {
            ftm_debug_cmd_format_info();
            return OAL_FAIL;
        }
    } while (*pc_param != '\0');

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_ftm::ul_cmd_bit_map: 0x%08x.}", st_ftm_debug.ul_cmd_bit_map);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_FTM_DBG, OAL_SIZEOF(st_ftm_debug));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
        (const oal_void *)&st_ftm_debug, OAL_SIZEOF(st_ftm_debug)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_ftm::memcpy fail.}");
    }

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_ftm_debug),
                               (oal_uint8 *)&st_write_msg, OAL_FALSE, OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_FTM, "{wal_hipriv_ftm::return err code[%d]!}", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#endif

#ifdef _PRE_WLAN_FEATURE_CSI_RAM

OAL_STATIC oal_uint32 wal_hipriv_set_csi(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_csi_param_stru *pst_cfg_csi_param;
    oal_uint8 auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };
    oal_uint8 uc_ta_check;
    oal_uint8 uc_ftm_check;
    oal_uint8 uc_csi_en;

    if (oal_unlikely((pst_cfg_net_dev == OAL_PTR_NULL) || (pc_param == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_hipriv_set_csi::pst_cfg_net_dev or pc_param null ptr error %x, %x!}\r\n",
                       (uintptr_t)pst_cfg_net_dev, (uintptr_t)pc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* PSD设置开关的命令: hipriv "vap0 set_csi ta ta_check csi_en"
       TA:被测量CSI的mac地址，为0表示不使能
       FTM check:为1时，只FTM交互帧被采集
       TA_check: 为1时，TA有效，表示每次采集CSI信息时需比对ta。
       csi_en:   为1时，表示使能CSI采集
 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_csi::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    uc_csi_en = (oal_uint8)oal_atoi(ac_name);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + ul_off_set;
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_csi::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    uc_ta_check = (oal_uint8)oal_atoi(ac_name);

    pc_param = pc_param + ul_off_set;
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_csi::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    uc_ftm_check = (oal_uint8)oal_atoi(ac_name);

    pc_param = pc_param + ul_off_set;
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    /* 地址获取失败，则默认地址为0 */
    if (ul_ret == OAL_SUCC) {
        oal_strtoaddr(ac_name, auc_mac_addr);
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_CSI, OAL_SIZEOF(mac_cfg_csi_param_stru));

    /* 设置配置命令参数 */
    pst_cfg_csi_param = (mac_cfg_csi_param_stru *)(st_write_msg.auc_value);
    oal_set_mac_addr(pst_cfg_csi_param->auc_mac_addr, auc_mac_addr);
    pst_cfg_csi_param->en_ta_check = uc_ta_check;
    pst_cfg_csi_param->en_csi = uc_csi_en;
    pst_cfg_csi_param->en_ftm_check = uc_ftm_check;

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_csi_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_csi::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32 wal_hipriv_send_2040_coext(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
    oal_uint32 ul_ret;
    oal_uint32 ul_off_set = 0;
    mac_cfg_set_2040_coexist_stru *pst_2040_coexist;

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    pst_2040_coexist = (mac_cfg_set_2040_coexist_stru *)st_write_msg.auc_value;
    /* 获取mib名称 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_send_2040_coext::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    pst_2040_coexist->ul_coext_info = (oal_uint32)oal_atoi(ac_name);

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_send_2040_coext::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;
    pst_2040_coexist->ul_channel_report = (oal_uint32)oal_atoi(ac_name);

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SEND_2040_COEXT,
                           OAL_SIZEOF(mac_cfg_set_2040_coexist_stru));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_set_2040_coexist_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_send_2040_coext::wal_send_cfg_event return err_code [%d]!}\r\n",
                         l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_2040_coext_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
    oal_uint16 us_len;

    if (oal_unlikely(WAL_MSG_WRITE_MAX_LEN <= OAL_STRLEN(pc_param))) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_2040_coext_info:: pc_param overlength is %d}\n",
                         OAL_STRLEN(pc_param));
        /* dump为维测函数传参groupsize大小为32 */
        oal_print_hex_dump((oal_uint8 *)pc_param, WAL_MSG_WRITE_MAX_LEN, 32,
                           "wal_hipriv_2040_coext_info: param is overlong:");
        return OAL_FAIL;
    }

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value), pc_param, OAL_STRLEN(pc_param)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_2040_coext_info::memcpy fail!}");
        return OAL_FAIL;
    }
    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_2040_COEXT_INFO, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_2040_coext_info::wal_send_cfg_event return err_code [%d]!}\r\n",
                         l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_get_version(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
    oal_uint16 us_len;

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value), pc_param, OAL_STRLEN(pc_param)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_get_version::memcpy fail!}");
        return OAL_FAIL;
    }

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (oal_uint16)(OAL_STRLEN(pc_param) + 1);

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_GET_VERSION, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_get_version::wal_send_cfg_event return err_code [%d]!}", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY

OAL_STATIC oal_uint32 wal_hipriv_set_opmode_notify(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
    oal_uint16 us_len;
    oal_uint32 ul_ret;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    oal_uint32 ul_off_set = 0;
    oal_uint8 uc_value;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_opmode_notify::wal_get_cmd_one_arg fails!}\r\n");
        return ul_ret;
    }

    pc_param += ul_off_set;
    uc_value = (oal_uint8)oal_atoi((const oal_int8 *)ac_name);

    us_len = OAL_SIZEOF(oal_uint8);
    *(oal_uint8 *)(st_write_msg.auc_value) = uc_value;

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_OPMODE_NOTIFY, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG,
                         "{wal_hipriv_set_opmode_notify::wal_hipriv_reset_device return err code = [%d].}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_get_user_nssbw(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_add_user_param_stru *pst_add_user_param;
    mac_cfg_add_user_param_stru st_add_user_param; /* 临时保存获取的use的信息 */
    oal_uint32 ul_get_addr_idx;

    /* 获取用户带宽和空间流信息: hipriv "vap0 add_user xx xx xx xx xx xx(mac地址)" */
    memset_s((oal_void *)&st_add_user_param, OAL_SIZEOF(mac_cfg_add_user_param_stru), 0,
             OAL_SIZEOF(mac_cfg_add_user_param_stru));
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_add_user::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }
    oal_strtoaddr(ac_name, st_add_user_param.auc_mac_addr);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + ul_off_set;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    /* 设置配置命令参数 */
    pst_add_user_param = (mac_cfg_add_user_param_stru *)(st_write_msg.auc_value);
    for (ul_get_addr_idx = 0; ul_get_addr_idx < WLAN_MAC_ADDR_LEN; ul_get_addr_idx++) {
        pst_add_user_param->auc_mac_addr[ul_get_addr_idx] = st_add_user_param.auc_mac_addr[ul_get_addr_idx];
    }

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_GET_USER_RSSBW, OAL_SIZEOF(mac_cfg_add_user_param_stru));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_add_user_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_add_user::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#endif
#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY


OAL_STATIC oal_uint32 wal_hipriv_blacklist_add(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
    oal_uint16 us_len;
    oal_uint32 ul_ret;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    oal_uint32 ul_off_set = 0;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_blacklist_add:wal_get_cmd_one_arg fail!}\r\n");
        return ul_ret;
    }
    memset_s((oal_uint8 *)&st_write_msg, OAL_SIZEOF(st_write_msg), 0, OAL_SIZEOF(st_write_msg));

    oal_strtoaddr(ac_name, st_write_msg.auc_value); /* 将字符 ac_name 转换成数组 mac_add[6] */

    us_len = OAL_MAC_ADDR_LEN; /* OAL_SIZEOF(oal_uint8); */

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ADD_BLACK_LIST, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_blacklist_add:wal_send_cfg_event return[%d].}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_blacklist_del(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;
    oal_uint16 us_len;
    oal_uint32 ul_ret;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    oal_uint32 ul_off_set = 0;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_blacklist_add:wal_get_cmd_one_arg fail!}\r\n");
        return ul_ret;
    }
    memset_s((oal_uint8 *)&st_write_msg, OAL_SIZEOF(st_write_msg), 0, OAL_SIZEOF(st_write_msg));

    oal_strtoaddr(ac_name, st_write_msg.auc_value); /* 将字符 ac_name 转换成数组 mac_add[6] */

    us_len = OAL_MAC_ADDR_LEN; /* OAL_SIZEOF(oal_uint8); */

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DEL_BLACK_LIST, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_blacklist_add:wal_send_cfg_event return[%d].}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_blacklist_mode(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return wal_hipriv_send_cfg_uint32_data(pst_net_dev, pc_param, WLAN_CFGID_BLACKLIST_MODE);
}

OAL_STATIC oal_uint32 wal_hipriv_blacklist_show(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return wal_hipriv_send_cfg_uint32_data(pst_net_dev, pc_param, WLAN_CFGID_BLACKLIST_SHOW);
}

OAL_STATIC oal_uint32 wal_hipriv_set_abl_on(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return wal_hipriv_send_cfg_uint32_data(pst_net_dev, pc_param, WLAN_CFGID_AUTOBLACKLIST_ON);
}

OAL_STATIC oal_uint32 wal_hipriv_set_abl_aging(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return wal_hipriv_send_cfg_uint32_data(pst_net_dev, pc_param, WLAN_CFGID_AUTOBLACKLIST_AGING);
}

OAL_STATIC oal_uint32 wal_hipriv_set_abl_threshold(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return wal_hipriv_send_cfg_uint32_data(pst_net_dev, pc_param, WLAN_CFGID_AUTOBLACKLIST_THRESHOLD);
}

OAL_STATIC oal_uint32 wal_hipriv_set_abl_reset(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return wal_hipriv_send_cfg_uint32_data(pst_net_dev, pc_param, WLAN_CFGID_AUTOBLACKLIST_RESET);
}

OAL_STATIC oal_uint32 wal_hipriv_set_isolation_mode(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return wal_hipriv_send_cfg_uint32_data(pst_net_dev, pc_param, WLAN_CFGID_ISOLATION_MODE);
}

OAL_STATIC oal_uint32 wal_hipriv_set_isolation_type(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return wal_hipriv_send_cfg_uint32_data(pst_net_dev, pc_param, WLAN_CFGID_ISOLATION_TYPE);
}

OAL_STATIC oal_uint32 wal_hipriv_set_isolation_fwd(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return wal_hipriv_send_cfg_uint32_data(pst_net_dev, pc_param, WLAN_CFGID_ISOLATION_FORWARD);
}

OAL_STATIC oal_uint32 wal_hipriv_set_isolation_clear(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return wal_hipriv_send_cfg_uint32_data(pst_net_dev, pc_param, WLAN_CFGID_ISOLATION_CLEAR);
}

OAL_STATIC oal_uint32 wal_hipriv_set_isolation_show(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    return wal_hipriv_send_cfg_uint32_data(pst_net_dev, pc_param, WLAN_CFGID_ISOLATION_SHOW);
}
#endif /* _PRE_WLAN_FEATURE_CUSTOM_SECURITY */


OAL_STATIC oal_uint32 wal_hipriv_vap_classify_en(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_val = 0xff;
    wal_msg_write_stru st_write_msg;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret == OAL_SUCC) {
        ul_val = (oal_uint32)oal_atoi(ac_name);
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_VAP_CLASSIFY_EN, OAL_SIZEOF(oal_uint32));

    /* 设置配置命令参数 */
    *((oal_uint32 *)(st_write_msg.auc_value)) = ul_val;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_packet_xmit::wal_send_cfg_event fail.return err code [%d]!}\r\n",
                         l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_vap_classify_tid(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_val = 0xff;
    wal_msg_write_stru st_write_msg;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret == OAL_SUCC) {
        ul_val = (oal_uint32)oal_atoi(ac_name);
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_VAP_CLASSIFY_TID, OAL_SIZEOF(oal_uint32));

    /* 设置配置命令参数 */
    *((oal_uint32 *)(st_write_msg.auc_value)) = ul_val;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_packet_xmit::wal_send_cfg_event fail.return err code [%d]!}\r\n",
                         l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_STA_PM

OAL_STATIC oal_uint32 wal_hipriv_sta_psm_param(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint16 us_beacon_timeout;
    oal_uint16 us_tbtt_offset;
    oal_uint16 us_ext_tbtt_offset;
    oal_uint16 us_dtim3_on;
    mac_cfg_ps_param_stru *pst_ps_para;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_sta_psm_param::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    us_beacon_timeout = (oal_uint16)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_sta_psm_param::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    us_tbtt_offset = (oal_uint16)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_sta_psm_param::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    us_ext_tbtt_offset = (oal_uint16)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_sta_psm_param::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    us_dtim3_on = (oal_uint16)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_PSM_PARAM, OAL_SIZEOF(mac_cfg_ps_param_stru));

    /* 设置配置命令参数 */
    pst_ps_para = (mac_cfg_ps_param_stru *)(st_write_msg.auc_value);
    pst_ps_para->us_beacon_timeout = us_beacon_timeout;
    pst_ps_para->us_tbtt_offset = us_tbtt_offset;
    pst_ps_para->us_ext_tbtt_offset = us_ext_tbtt_offset;
    pst_ps_para->us_dtim3_on = us_dtim3_on;

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ps_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_PWR, "{wal_hipriv_sta_psm_param::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


oal_uint32 wal_hipriv_sta_pm_on(oal_net_device_stru *pst_cfg_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint8 uc_sta_pm_open;
    mac_cfg_ps_open_stru *pst_sta_pm_open;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_sta_pm_open::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    uc_sta_pm_open = (oal_uint8)oal_atoi(ac_name);
    pc_param = pc_param + ul_off_set;

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_STA_PM_ON, OAL_SIZEOF(mac_cfg_ps_open_stru));

    /* 设置配置命令参数 */
    pst_sta_pm_open = (mac_cfg_ps_open_stru *)(st_write_msg.auc_value);
    /* MAC_STA_PM_SWITCH_ON / MAC_STA_PM_SWITCH_OFF */
    pst_sta_pm_open->uc_pm_enable = uc_sta_pm_open;
    pst_sta_pm_open->uc_pm_ctrl_type = MAC_STA_PM_CTRL_TYPE_CMD;

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_ps_open_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_PWR, "{wal_hipriv_sta_pm_open::return err code [%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_TCP_OPT

OAL_STATIC oal_uint32 wal_hipriv_get_tcp_ack_stream_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_GET_TCP_ACK_STREAM_INFO, OAL_SIZEOF(oal_uint32));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_hipriv_show_arpoffload_info::wal_send_cfg_event fail.return err code [%d]!}\r\n",
                         l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_tcp_tx_ack_opt_enable(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_val;
    wal_msg_write_stru st_write_msg;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_hipriv_tcp_tx_ack_opt_enable::wal_get_cmd_one_arg vap name return err_code %d!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    pc_param += ul_off_set;

    ul_val = (oal_uint32)oal_atoi((const oal_int8 *)ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_TX_TCP_ACK_OPT_ENALBE, OAL_SIZEOF(oal_uint32));

    /* 设置配置命令参数 */
    *((oal_uint32 *)(st_write_msg.auc_value)) = ul_val;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_hipriv_show_arpoffload_info::wal_send_cfg_event fail.return err code [%ud]!}\r\n",
                         l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_tcp_rx_ack_opt_enable(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_val;
    wal_msg_write_stru st_write_msg;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_hipriv_tcp_rx_ack_opt_enable::wal_get_cmd_one_arg vap name return err_code %d!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    ul_val = (oal_uint32)oal_atoi((const oal_int8 *)ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_RX_TCP_ACK_OPT_ENALBE, OAL_SIZEOF(oal_uint32));

    /* 设置配置命令参数 */
    *((oal_uint32 *)(st_write_msg.auc_value)) = ul_val;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_hipriv_show_arpoffload_info::wal_send_cfg_event fail.return err code [%d]!}\r\n",
                         l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32 wal_hipriv_tcp_tx_ack_limit(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_val;
    wal_msg_write_stru st_write_msg;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_hipriv_tcp_tx_ack_limit::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    ul_val = (oal_uint32)oal_atoi((const oal_int8 *)ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_TX_TCP_ACK_OPT_LIMIT, OAL_SIZEOF(oal_uint32));

    /* 设置配置命令参数 */
    *((oal_uint32 *)(st_write_msg.auc_value)) = ul_val;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_hipriv_show_arpoffload_info::wal_send_cfg_event fail.return err code [%d]!}\r\n",
                         l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32 wal_hipriv_tcp_rx_ack_limit(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_val;
    wal_msg_write_stru st_write_msg;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_hipriv_tcp_tx_ack_limit::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    ul_val = (oal_uint32)oal_atoi((const oal_int8 *)ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_RX_TCP_ACK_OPT_LIMIT, OAL_SIZEOF(oal_uint32));

    /* 设置配置命令参数 */
    *((oal_uint32 *)(st_write_msg.auc_value)) = ul_val;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_hipriv_show_arpoffload_info::wal_send_cfg_event fail.return err code [%d]!}\r\n",
                         l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST

OAL_STATIC oal_uint32 wal_hipriv_enable_2040bss(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_uint8 uc_2040bss_switch;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_enable_2040bss::wal_get_cmd_one_arg return err_code %d!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    if ((0 != strcmp("0", ac_name)) && (0 != strcmp("1", ac_name))) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{wal_hipriv_enable_2040bss::invalid parameter.}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    uc_2040bss_switch = (oal_uint8)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_2040BSS_ENABLE, OAL_SIZEOF(oal_uint8));
    *((oal_uint8 *)(st_write_msg.auc_value)) = uc_2040bss_switch; /* 设置配置命令参数 */

    ul_ret = (oal_uint32)wal_send_cfg_event(pst_net_dev,
                                            WAL_MSG_TYPE_WRITE,
                                            WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                                            (oal_uint8 *)&st_write_msg,
                                            OAL_FALSE,
                                            OAL_PTR_NULL);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_hipriv_enable_2040bss::return err code %d!}\r\n", ul_ret);
    }

    return ul_ret;
}
#endif /* _PRE_WLAN_FEATURE_20_40_80_COEXIST */

#ifdef _PRE_DEBUG_MODE_USER_TRACK

OAL_STATIC oal_uint32 wal_hipriv_report_thrput_stat(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    mac_cfg_usr_thrput_stru st_usr_thrput;

    /* sh hipriv.sh "vap_name thrput_stat  XX:XX:XX:XX:XX;XX 0|1" */
    /* 获取用户mac地址 */
    ul_ret = wal_hipriv_get_mac_addr(pc_param, st_usr_thrput.auc_user_macaddr, &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_report_thrput_stat::wal_hipriv_get_mac_addr return [%d].}",
                         ul_ret);
        return ul_ret;
    }
    pc_param += ul_off_set;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_report_thrput_stat::get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }

    st_usr_thrput.uc_param = (oal_uint8)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_USR_THRPUT_STAT, OAL_SIZEOF(st_usr_thrput));

    /* 填写消息体，参数 */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
                 &st_usr_thrput, OAL_SIZEOF(st_usr_thrput)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_report_thrput_stat::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_usr_thrput),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_report_thrput_stat::wal_send_cfg_event return err code [%d]!}\r\n",
                         l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

#endif
#ifdef _PRE_WLAN_FEATURE_ROAM

OAL_STATIC oal_uint32 wal_hipriv_roam_enable(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_off_set;
    oal_bool_enum_uint8 en_enable;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wal_msg_write_stru st_write_msg;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "roam enable type return err_code [%d]", ul_ret);
        return ul_ret;
    }
    en_enable = (oal_bool_enum_uint8)oal_atoi(ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ROAM_ENABLE, OAL_SIZEOF(oal_uint32));
    *((oal_bool_enum_uint8 *)(st_write_msg.auc_value)) = en_enable;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_bool_enum_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_roam_enable::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32 wal_hipriv_roam_org(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_off_set;
    oal_uint8 uc_org;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wal_msg_write_stru st_write_msg;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "roam org type return err_code[%d]", ul_ret);
        return ul_ret;
    }
    uc_org = (oal_uint8)oal_atoi(ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ROAM_ORG, OAL_SIZEOF(oal_uint32));
    *((oal_uint8 *)(st_write_msg.auc_value)) = uc_org;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_bool_enum_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{wal_hipriv_roam_org::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32 wal_hipriv_roam_band(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_uint32 ul_off_set;
    oal_uint8 uc_band;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wal_msg_write_stru st_write_msg;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "roam band type return err_code[%d]", ul_ret);
        return ul_ret;
    }
    uc_band = (oal_uint8)oal_atoi(ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ROAM_BAND, OAL_SIZEOF(oal_uint32));
    *((oal_uint8 *)(st_write_msg.auc_value)) = uc_band;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_bool_enum_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{wal_hipriv_roam_band::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32 wal_hipriv_roam_start(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_int32 l_ret;
    oal_uint32 ul_ret;
    roam_channel_org_enum_uint8 en_scan_type = ROAM_SCAN_CHANNEL_ORG_0;
    oal_bool_enum_uint8 en_current_bss_ignore = OAL_TRUE;
    oal_uint8 auc_bssid[OAL_MAC_ADDR_LEN] = { 0 };
    wal_msg_write_stru st_write_msg;
    mac_cfg_set_roam_start_stru *pst_roam_start;

    oal_uint32 ul_off_set;
    oal_uint8 uc_param;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret == OAL_ERR_CODE_PTR_NULL) {
        /* 默认扫描+漫游 */
        en_scan_type = ROAM_SCAN_CHANNEL_ORG_0;
        en_current_bss_ignore = OAL_TRUE;
        memset_s(auc_bssid, OAL_SIZEOF(auc_bssid), 0, OAL_SIZEOF(auc_bssid));
    } else if (ul_ret == OAL_SUCC) {
        en_scan_type = (oal_uint8)oal_atoi(ac_name); /* 指定漫游时刻是否搭配扫描操作 */
        pc_param += ul_off_set;
        /* 获取是否可以关联到自己的参数 */
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
        if (ul_ret == OAL_ERR_CODE_PTR_NULL) {
            /* 默认不能关联到自己 */
            en_current_bss_ignore = OAL_TRUE;
            memset_s(auc_bssid, OAL_SIZEOF(auc_bssid), 0, OAL_SIZEOF(auc_bssid));
        } else if (ul_ret == OAL_SUCC) {
            uc_param = (oal_uint8)oal_atoi(ac_name);
            /* 0/TRUE或者参数缺失表示漫游到其它AP, 1/FALSE表示漫游到自己 */
            en_current_bss_ignore = (uc_param == 0) ? OAL_TRUE : OAL_FALSE;
            /* 获取BSSID */
            pc_param = pc_param + ul_off_set;
            ul_ret = wal_hipriv_get_mac_addr(pc_param, auc_bssid, &ul_off_set);
            if (ul_ret == OAL_ERR_CODE_PTR_NULL) {
                memset_s(auc_bssid, OAL_SIZEOF(auc_bssid), 0, OAL_SIZEOF(auc_bssid));
            } else if (ul_ret != OAL_SUCC) {
                oam_warning_log0(0, OAM_SF_ROAM, "{wal_hipriv_roam_start::bssid failed!}\r\n");
                return ul_ret;
            }
        } else {
            OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{wal_hipriv_roam_start::roam_start return err_code [%d]!}\r\n", ul_ret);
            return (oal_uint32)ul_ret;
        }
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{wal_hipriv_roam_start::roam_start cfg_cmd error[%d]}", ul_ret);
        return ul_ret;
    }
    oam_warning_log4(0, OAM_SF_ROAM,
                     "{wal_hipriv_roam_start::roam_start scantype[%d], encurbssignore[%d],targetbssid[XXXX:%02X:%02X]}",
                     en_scan_type, en_current_bss_ignore, auc_bssid[4], auc_bssid[5]); /* auc_bssid第4、5byte */
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    memset_s(&st_write_msg, OAL_SIZEOF(wal_msg_write_stru), 0, OAL_SIZEOF(wal_msg_write_stru));
    pst_roam_start = (mac_cfg_set_roam_start_stru *)(st_write_msg.auc_value);
    pst_roam_start->en_scan_type = en_scan_type;
    pst_roam_start->en_current_bss_ignore = en_current_bss_ignore;
    if (memcpy_s(pst_roam_start->auc_bssid,
                 OAL_SIZEOF(pst_roam_start->auc_bssid), auc_bssid, OAL_SIZEOF(auc_bssid)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_roam_start::memcpy fail!}");
        return OAL_FAIL;
    }

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ROAM_START, OAL_SIZEOF(mac_cfg_set_roam_start_stru));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_set_roam_start_stru),
                               (oal_uint8 *)&st_write_msg, OAL_FALSE, OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{wal_hipriv_roam_enable::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_roam_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_int32 l_ret;
    oal_bool_enum_uint8 en_enable;
    wal_msg_write_stru st_write_msg;

    en_enable = 1;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ROAM_INFO, OAL_SIZEOF(oal_uint32));
    *((oal_bool_enum_uint8 *)(st_write_msg.auc_value)) = en_enable;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_bool_enum_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{wal_hipriv_roam_enable::return err code[%d]!}\r\n", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_ROAM
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI

oal_uint32 wal_hipriv_dyn_cali_cfg(oal_net_device_stru *pst_net_dev, oal_int8 *puc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    mac_ioctl_dyn_cali_param_stru *pst_dyn_cali_param = OAL_PTR_NULL;
    wal_ioctl_dyn_cali_stru st_cyn_cali_cfg;
    oal_uint32 ul_ret;
    oal_uint8 uc_map_index = 0;
    oal_int32 l_send_event_ret;

    pst_dyn_cali_param = (mac_ioctl_dyn_cali_param_stru *)(st_write_msg.auc_value);
    memset_s(pst_dyn_cali_param, OAL_SIZEOF(mac_ioctl_dyn_cali_param_stru),
             0, OAL_SIZEOF(mac_ioctl_dyn_cali_param_stru));

    if (oal_unlikely((pst_net_dev == OAL_PTR_NULL) || (puc_param == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_dyn_cali_cfg::pst_cfg_net_dev or puc_param null ptr error }\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = wal_get_cmd_one_arg(puc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_hipriv_dyn_cali_cfg::wal_get_cmd_one_arg return err_code [%d]!}\r\n",
                         ul_ret);
        return ul_ret;
    }
    /* 寻找匹配的命令 */
    st_cyn_cali_cfg = g_ast_dyn_cali_cfg_map[0];
    while (st_cyn_cali_cfg.pc_name != OAL_PTR_NULL) {
        if (0 == strcmp(st_cyn_cali_cfg.pc_name, ac_name)) {
            break;
        }
        st_cyn_cali_cfg = g_ast_dyn_cali_cfg_map[++uc_map_index];
    }

    /* 没有找到对应的命令，则报错 */
    if (st_cyn_cali_cfg.pc_name == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_dyn_cali_cfg::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* 记录命令对应的枚举值 */
    pst_dyn_cali_param->en_dyn_cali_cfg = g_ast_dyn_cali_cfg_map[uc_map_index].en_dyn_cali_cfg;
    ul_ret = wal_get_cmd_one_arg(puc_param + ul_off_set, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret == OAL_SUCC) {
        /* 记录参数配置值 */
        pst_dyn_cali_param->us_value = (oal_uint16)oal_atoi(ac_name);
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_hipriv_dyn_cali_cfg::wal_get_cmd_one_arg [%d]!}\r\n",
                         pst_dyn_cali_param->us_value);
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DYN_CALI_CFG, OAL_SIZEOF(oal_uint32));

    l_send_event_ret = wal_send_cfg_event(pst_net_dev,
                                          WAL_MSG_TYPE_WRITE,
                                          WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                                          (oal_uint8 *)&st_write_msg,
                                          OAL_FALSE, OAL_PTR_NULL);
    if (oal_unlikely(l_send_event_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_hipriv_cyn_cali_set_dscr_interval::wal_send_cfg_event return err code [%d]!}\r\n",
                         l_send_event_ret);
        return (oal_uint32)l_send_event_ret;
    }
    return OAL_SUCC;
}
#endif

OAL_STATIC OAL_INLINE oal_void protocol_debug_cmd_format_info(void)
{
    oam_warning_log0(0, OAM_SF_ANY,
                     "{CMD format::sh hipriv.sh 'wlan0 protocol_debug\
    [band_force_switch 0|1|2(20M|40M+|40M-)]\
    [2040_ch_swt_prohi 0|1]\
    [40_intol 0|1]'!!}\r\n");
    oam_warning_log0(0, OAM_SF_ANY,
                     "{[csa 0(csa mode) 1(csa channel) 10(csa cnt) 1(debug  flag,0:normal channel channel,1: \
    only include csa ie 2:cannel debug)] \
    [2040_user_switch 0|1]'!!}\r\n");
    oam_warning_log0(0, OAM_SF_ANY,
                     "[lsig 0|1]\
    '!!}\r\n");
}


OAL_STATIC oal_uint32 wal_protocol_debug_parse_csa_cmd(oal_int8 *pc_param,
                                                       mac_protocol_debug_switch_stru *pst_debug_info,
                                                       oal_uint32 *pul_offset)
{
    oal_uint32 ul_ret;
    oal_int8 ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN];
    oal_uint32 ul_off_set = 0;
    oal_uint8 uc_value;

    *pul_offset = 0;
    /* 解析csa mode */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_protocol_debug_parse_csa_cmd::get csa mode error,return.}");
        return ul_ret;
    }
    uc_value = (oal_uint8)oal_atoi(ac_value);
    if (uc_value > 1) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_protocol_debug_parse_csa_cmd::csa mode=[%d] invalid,return.}", uc_value);
        return OAL_FAIL;
    }
    *pul_offset += ul_off_set;
    pst_debug_info->st_csa_debug.en_mode = uc_value;
    pc_param += ul_off_set;
    ul_off_set = 0;

    /* 解析csa channel */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_protocol_debug_parse_csa_cmd::get csa channel error,return.}");
        return ul_ret;
    }
    *pul_offset += ul_off_set;
    pst_debug_info->st_csa_debug.uc_channel = (oal_uint8)oal_atoi(ac_value);
    pc_param += ul_off_set;
    ul_off_set = 0;

    /* 解析bandwidth */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_protocol_debug_parse_csa_cmd::get bandwidth error,return.}");
        return ul_ret;
    }
    uc_value = (oal_uint8)oal_atoi(ac_value);
    if (uc_value >= WLAN_BAND_WIDTH_BUTT) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{wal_protocol_debug_parse_csa_cmd::invalid bandwidth = %d,return.}", uc_value);
        return OAL_FAIL;
    }
    *pul_offset += ul_off_set;
    pst_debug_info->st_csa_debug.en_bandwidth = uc_value;
    pc_param += ul_off_set;
    ul_off_set = 0;

    /* 解析csa cnt */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_protocol_debug_parse_csa_cmd::get csa cnt error,return.}");
        return ul_ret;
    }
    uc_value = (oal_uint8)oal_atoi(ac_value);
    if (uc_value >= 255) { /* csa cnt最大值是255 */
        uc_value = 255; /* csa cnt最大值是255 */
    }
    *pul_offset += ul_off_set;
    pst_debug_info->st_csa_debug.uc_cnt = uc_value;
    pc_param += ul_off_set;
    ul_off_set = 0;

    /* 解析debug flag */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_protocol_debug_parse_csa_cmd::get debug flag error,return.}");
        return ul_ret;
    }
    *pul_offset += ul_off_set;
    uc_value = (oal_uint8)oal_atoi(ac_value);
    if (uc_value >= MAC_CSA_FLAG_BUTT) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "wal_protocol_debug_parse_csa_cmd:invalid debug flag = %d,return", uc_value);
        return OAL_FAIL;
    }
    pst_debug_info->st_csa_debug.en_debug_flag = (mac_csa_flag_enum_uint8)oal_atoi(ac_value);
    pc_param += ul_off_set;
    ul_off_set = 0;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_set_protocol_debug_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_off_set = 0;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_int8 ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN];
    mac_protocol_debug_switch_stru st_protocol_debug;
    oal_uint32 ul_ret = 0;
    oal_int32 l_ret;
    oal_bool_enum_uint8 en_cmd_updata = OAL_FALSE;

    /* sh hipriv.sh "wlan0 protocol_debug band_force_switch 0|1|2(20|40+|40-) csa [mode 1:立即停止 0:不停止]0
     * [channel]1 [bandwidth]0 [cnt]2 [type]0=切换 1=发送 2=停止"
     */
    memset_s(&st_protocol_debug, OAL_SIZEOF(st_protocol_debug), 0, OAL_SIZEOF(st_protocol_debug));

    do {
        /* 获取命令关键字 */
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
        if ((ul_ret != OAL_SUCC) && (ul_off_set != 0)) {
            protocol_debug_cmd_format_info();
            return ul_ret;
        }
        pc_param += ul_off_set;
        if (en_cmd_updata == OAL_FALSE) {
            en_cmd_updata = OAL_TRUE;
        } else if (ul_off_set == 0) {
            break;
        }

        /* 命令分类 */
        if (0 == strcmp("band_force_switch", ac_name)) {
            /* 取命令配置值 */
            ul_ret = wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &ul_off_set);
            if ((ul_ret != OAL_SUCC) || (ac_value[0]) < '0' || (ac_value[0] > '9')) {
                protocol_debug_cmd_format_info();
                return ul_ret;
            }
            pc_param += ul_off_set;
            ul_off_set = 0;
            /* 填写结构体 */
            st_protocol_debug.en_band_force_switch = ((oal_uint8)oal_atoi(ac_value));
            st_protocol_debug.ul_cmd_bit_map |= BIT0;
        } else if (0 == strcmp("csa", ac_name)) {
            ul_ret = wal_protocol_debug_parse_csa_cmd(pc_param, &st_protocol_debug, &ul_off_set);
            if (ul_ret != OAL_SUCC) {
                protocol_debug_cmd_format_info();
                return ul_ret;
            }
            pc_param += ul_off_set;
            ul_off_set = 0;
            st_protocol_debug.ul_cmd_bit_map |= BIT1;
        } else {
            protocol_debug_cmd_format_info();
            return OAL_FAIL;
        }
    } while (*pc_param != '\0');

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_protocol_debug_info::ul_cmd_bit_map: 0x%08x.}",
                     st_protocol_debug.ul_cmd_bit_map);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_PROTOCOL_DBG, OAL_SIZEOF(st_protocol_debug));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
                 (const oal_void *)&st_protocol_debug,
                 OAL_SIZEOF(st_protocol_debug)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_hipriv_show_protocol_debug_info::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_protocol_debug),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_protocol_debug_info::return err code[%d]!}", l_ret);
        return (oal_uint32)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_hipriv_sk_pacing_shift(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_uint32 ul_ret;
    oal_uint32 ul_off_set;
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &ul_off_set);
    if (ul_ret == OAL_SUCC) {
        g_sk_pacing_shift = (oal_uint8)oal_atoi(ac_name);
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{wal_hipriv_sk_pacing_shift::input parameter error[%d]}", ul_ret);
        return ul_ret;
    }

    OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{wal_hipriv_sk_pacing_shift::set sk pacing shift [%d]}", g_sk_pacing_shift);
    return OAL_SUCC;
}

OAL_STATIC oal_uint32 wal_hipriv_psm_flt_info(oal_net_device_stru *pst_net_dev, oal_int8 *pc_param)
{
    oal_int8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    oal_uint32 ul_off_set;
    oal_uint32 ul_ret;
    oal_uint8 uc_query_type;
    /* 获取query type */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &ul_off_set);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_psm_flt_info::first para return err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    uc_query_type = (oal_uint8)oal_atoi((const oal_int8 *)ac_name);
    wal_ioctl_get_psm_info(pst_net_dev, uc_query_type, OAL_PTR_NULL);
    return OAL_SUCC;
}

OAL_CONST wal_hipriv_cmd_entry_stru  g_ast_hipriv_cmd_debug[] = {
    /***********************调试命令***********************/
    {"destroy",                 wal_hipriv_del_vap},                /* 删除vap私有命令为: hipriv "vap0 destroy" */
    /* 设置mac_vap_cap_flag: hipriv "vap0 2040_coext_switch 0|1" */
    {"2040_coext_switch",       wal_hipriv_set_2040_coext_switch},
    /* 全局日志开关:  hipriv "Hisilicon0 global_log_switch 0 | 1 */
    {"global_log_switch",       wal_hipriv_global_log_switch},
    /* VAP级别的日志开关: hipriv "Hisilicon0{VAPx} log_switch 0 | 1"，该命令针对所有的VAP */
    {"log_switch",              wal_hipriv_vap_log_switch},
    /* 特性的INFO级别日志开关 hipriv "VAPX feature_name {0/1}"   */
    {"feature_log_switch",      wal_hipriv_feature_log_switch},
    /* 特性的INFO级别日志开关 hipriv "Hisilicon0 log_ratelimit {type} {switch} {interval} {burst}" */
    {"log_ratelimit",           wal_hipriv_log_ratelimit},
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* log低功耗模式: hipriv "Hisilicon0 log_pm 0 | 1"，log pm模式开关 */
    {"log_pm",                  wal_hipriv_log_lowpower},
    /* log低功耗模式: hipriv "Hisilicon0 pm_switch 0 | 1"，log pm模式开关 */
    {"pm_switch",               wal_hipriv_pm_switch},
#endif
    /* OAM event模块的开关的命令: hipriv "Hisilicon0 event_switch 0 | 1"，该命令针对所有的VAP */
    {"event_switch",            wal_hipriv_event_switch},
    /* 设置是否上报beacon帧开关: hipriv "Hisilicon0 ota_beacon_switch 0 | 1"，该命令针对所有的VAP */
    {"ota_beacon_on",           wal_hipriv_ota_beacon_switch},
    /* 设置是否上报接收描述符帧开关: hipriv "Hisilicon0 ota_rx_dscr_switch 0 | 1"，该命令针对所有的VAP */
    {"ota_switch",              wal_hipriv_ota_rx_dscr_switch},
    /* 设置oam模块的信息打印位置命令:
     * hipriv "Hisilicon0 oam_output 0~4 (oam_output_type_enum_uint8)"，该命令针对所有的VAP
     */
    {"oam_output",              wal_hipriv_oam_output},
    /* 设置AMPDU开启的配置命令: hipriv "vap0  ampdu_start xx xx xx xx xx xx(mac地址) tidno" 该命令针对某一个VAP */
    {"ampdu_start",             wal_hipriv_ampdu_start},
    /* 设置自动开始BA会话的开关:hipriv "vap0  auto_ba 0 | 1" 该命令针对某一个VAP */
    {"auto_ba",                 wal_hipriv_auto_ba_switch},
    /* 设置建立BA会话的配置命令:
     * hipriv "vap0 addba_req xx xx xx xx xx xx(mac地址) tidno ba_policy buffsize timeout" 该命令针对某一个VAP
     */
    {"addba_req",               wal_hipriv_addba_req},
    /* 设置删除BA会话的配置命令:
     * hipriv "vap0 delba_req xx xx xx xx xx xx(mac地址) tidno direction" 该命令针对某一个VAP
     */
    {"delba_req",               wal_hipriv_delba_req},
#ifdef _PRE_WLAN_FEATURE_WMMAC
    /* 设置建立TS，即发送ADDTS REQ的配置命令:hipriv "vap0 addts_req tid direction apsd up nominal_msdu_size
     * max_msdu_size minimum_data_rate mean_data_rate peak_data_rate minimum_phy_rate surplus_bandwidth_allowance"
     * 该命令针对某一个VAP
     */
    {"addts_req",               wal_hipriv_addts_req},
    /* 设置删除TS，即发送DELTS的配置命令: hipriv "vap0 tidno" 该命令针对某一个VAP */
    {"delts",                   wal_hipriv_delts},
    /* 发送重关联请求帧: hipriv "vap0 reassoc_req" */
    {"reassoc_req",             wal_hipriv_reassoc_req},
     /* 设置删除TS的配置命令: hipriv "vap0 wmmac_switch 1/0(使能) 0|1(WMM_AC认证使能) AC xxx(limit_medium_time)" */
    {"wmmac_switch",            wal_hipriv_wmmac_switch},
#endif
    /* 打印内存池信息: hipriv "Hisilicon0 meminfo poolid" */
    {"meminfo",                 wal_hipriv_mem_info},
    /* 打印内存池信息: hipriv "Hisilicon0 memleak poolid" */
    {"memleak",                 wal_hipriv_mem_leak},
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 打印内存池信息: hipriv "Hisilicon0 memoryinfo host/device" */
    {"memoryinfo",              wal_hipriv_memory_info},
#endif
    /* 调整dbb scaling值: hipriv "wlan0 dbb_scaling_amend <param name> <value>" */
    {"dbb_scaling_amend",       wal_hipriv_dbb_scaling_amend},
    /* 设置20/40共存是否禁止信道切换: hipriv "vap0 2040_ch_swt_prohi 0|1" 0表示不禁止，1表示禁用 */
    {"2040_ch_swt_prohi",       wal_hipriv_2040_channel_switch_prohibited},
    /* 设置40MHz不允许位: hipriv "vap0 2040_intolerant 0|1" 0表示不允许运行40MHz，1表示允许运行40MHz */
    {"2040_intolerant",         wal_hipriv_set_fortymhzintolerant},
    /* 打印描述符信息: hipriv "vap0 set_ucast_data <param name> <value>" */
    {"set_ucast_data", wal_hipriv_set_ucast_data_dscr_param},
    /* 打印描述符信息: hipriv "vap0 set_bcast_data <param name> <value>" */
    {"set_bcast_data", wal_hipriv_set_bcast_data_dscr_param},
    /* 打印描述符信息: hipriv "vap0 set_ucast_mgmt <param name> <value>" */
    {"set_ucast_mgmt", wal_hipriv_set_ucast_mgmt_dscr_param},
    /* 打印描述符信息: hipriv "vap0 set_mbcast_mgmt <param name> <value>" */
    {"set_mbcast_mgmt", wal_hipriv_set_mbcast_mgmt_dscr_param},
    /* 打印寄存器信息: hipriv "vap0 amsdu_start xx xx xx xx xx xx(mac地址10进制oal_atoi) <max num> <max size> " */
    {"amsdu_start",             wal_hipriv_amsdu_start},
    /* 打印STA扫描到的AP列表: hipriv "sta0 list_ap" */
    {"list_ap",                 wal_hipriv_list_ap},
    /* 打印AP关联的STA列表: hipriv "sta0 list_sta" */
    {"list_sta",                wal_hipriv_list_sta},
    /* 触发sta扫描: hipriv "sta0 start_scan" */
    {"start_scan",              wal_hipriv_start_scan},
    /* 触发sta加入并认证关联: hipriv "sta0 start_join 1" 1表示扫描到的AP在device写数组下标号 */
    {"start_join",              wal_hipriv_start_join},
    /* 触发sta去认证: hipriv "vap0 start_deauth" */
    {"start_deauth",            wal_hipriv_start_deauth},
    /* 删除1个用户 hipriv "vap0 kick_user xx xx xx xx xx xx(mac地址)" */
    {"kick_user",               wal_hipriv_kick_user},
    /* 暂停指定用户的指定tid hipriv "vap0 pause_tid xx xx xx xx xx xx(mac地址) tid_num 0\1" */
    {"pause_tid",               wal_hipriv_pause_tid},
    /* 开启或关闭ampdu发送功能 hipriv "vap0 ampdu_tx_on 0\1" */
    {"ampdu_tx_on",             wal_hipriv_ampdu_tx_on},
    /* 开启或关闭ampdu发送功能 hipriv "vap0 amsdu_tx_on 0\1" */
    {"amsdu_tx_on",             wal_hipriv_amsdu_tx_on},
    /* 获取所有维测统计信息: hipriv "Hisilicon0 wifi_stat_info" */
    {"wifi_stat_info",          wal_hipriv_show_stat_info},
    /* 获取某一个vap下的收发包统计信息: sh hipriv.sh "vap_name vap_pkt_stat" */
    {"vap_pkt_stat",            wal_hipriv_show_vap_pkt_stat},
    /* 清零所有维测统计信息: hipriv "Hisilicon0 clear_stat_info" */
    {"clear_stat_info",         wal_hipriv_clear_stat_info},
    /* 上报某个user下的维测统计信息: sh hipriv.sh "Hisilicon0 usr_stat_info usr_id" */
    {"usr_stat_info",           wal_hipriv_user_stat_info},
    /* 开启5115硬件定时器: hipriv "Hisilicon0 timer_start 0/1" */
    {"timer_start",             wal_hipriv_timer_start},
    /* 开启5115硬件定时器: hipriv "Hisilicon0 show_profiling 0/1/2 (0是rx 1是tx 2是chipstart)" */
    {"show_profiling",          wal_hipriv_show_profiling},
    /* 设置amsdu ampdu联合聚合功能的开关:hipriv "vap0  ampdu_amsdu 0 | 1" 该命令针对某一个VAP */
    {"ampdu_amsdu",             wal_hipriv_ampdu_amsdu_switch},
    /* 复位硬件phy&mac: hipriv "Hisilicon0 reset_hw 0|1|2|3(all|phy|mac|debug) 0|1(reset phy reg) 0|1(reset mac reg) */
    {"reset_operate",           wal_hipriv_reset_operate},
    /* hipriv "wlan0 thread_bindcpu 0|1|2(hisi_hcc,hmax_rxdata,rx_tsk) 0(cpumask)" */
    {"thread_bindcpu",          wal_hipriv_bindcpu},
    /* hipriv "wlan0 napi_weight 0|1(动态调) 16(weight)" */
    {"napi_weight",             wal_hipriv_napi_weight},
    /* dump出来接收描述符队列，hipriv "Hisilicon0 dump_rx_dscr 0|1", 0:高优先级队列 1:普通优先级队列 */
    {"dump_rx_dscr",            wal_hipriv_dump_rx_dscr},
    /* dump出来发送描述符队列，hipriv "Hisilicon0 dump_tx_dscr value", value取值0~3代表AC发送队列，4代表管理帧 */
    {"dump_tx_dscr",            wal_hipriv_dump_tx_dscr},
    /* dump内存， hipriv "Hisilicon0 dump_memory 0xabcd len" */
    {"dump_memory",             wal_hipriv_dump_memory},
    /* 支持信道列表， hipriv "Hisilicon0 list_channel" */
    {"list_channel",            wal_hipriv_list_channel},
    /* 设置管制域最大发送功率(可以突破管制域的限制)，hipriv "Hisilicon0 set_regdomain_pwr_priv 20",单位dBm */
    {"set_regdomain_pwr_p",     wal_hipriv_set_regdomain_pwr_priv},
    /* 打印事件队列信息，将打印出每一个非空事件队列中事件的个数，以及每一个事件头信息,
     * hipriv "Hisilicon0 event_queue"
     */
    {"event_queue",             wal_hipriv_event_queue_info},
    /* 设置分片门限的配置命令: hipriv "vap0 frag_threshold (len)" 该命令针对某一个VAP */
    {"frag_threshold",          wal_hipriv_frag_threshold},
    /* 动态开启或者关闭wmm hipriv "vap0 wmm_switch 0|1"(0不使能，1使能) */
    {"wmm_switch",              wal_hipriv_wmm_switch},
    /* 设置以太网帧上报的开关，
     * sh hipriv.sh "vap0 ether_switch user_macaddr oam_ota_frame_direction_type_enum(帧方向) 0|1(开关)"
     */
    {"ether_switch",            wal_hipriv_set_ether_switch},
    /* 设置80211单播帧上报的开关，sh hipriv.sh "vap0 80211_uc_switch user_macaddr 0|1(帧方向tx|rx)
     * 0|1(帧类型:管理帧|数据帧) 0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关)"
     */
    {"80211_uc_switch",         wal_hipriv_set_80211_ucast_switch},
    /* 设置80211组播\广播帧上报的开关，sh hipriv.sh "Hisilicon0 80211_mc_switch 0|1(帧方向tx|rx)
     * 0|1(帧类型:管理帧|数据帧) 0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关)"
     */
    {"80211_mc_switch",         wal_hipriv_set_80211_mcast_switch},
    /* 设置probe req与rsp上报的开关，sh hipriv.sh "Hisilicon0 probe_switch 0|1(帧方向tx|rx)
     * 0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关)"
     */
    {"probe_switch",            wal_hipriv_set_probe_switch},
    /* 设置打印接收报文rssi信息的开关，sh hipriv.sh "Hisilicon0 rssi_switch 0|1(打开|关闭) N(间隔N个帧打印)" */
    {"phy_debug",             wal_hipriv_set_phy_debug_switch},
    /* 设置所有ota上报，如果为1，则所有类型帧的cb描述符都报，如果为0，什么都不报，sh hipriv.sh "Hisilicon0 set_all_ota 0|1" */
    {"set_all_ota",             wal_hipriv_set_all_ota},
    /* 设置所有用户的单播开关，sh hipriv.sh "Hisilicon0 80211_uc_all 0|1(帧方向tx|rx) 0|1(帧类型:管理帧|数据帧)
     * 0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关)"
     */
    {"80211_uc_all",            wal_hipriv_set_all_80211_ucast},
    /* 设置所有用户的以太网开关，sh hipriv.sh "Hisilicon0 ether_all 0|1(帧方向tx|rx) 0|1(开关)" */
    {"ether_all",               wal_hipriv_set_all_ether_switch},
    /* 设置发送广播arp和dhcp开关，sh hipriv.sh "Hisilicon0 dhcp_arp_switch 0|1(开关)" */
    {"dhcp_arp_switch",         wal_hipriv_set_dhcp_arp_switch},
#ifdef _PRE_DEBUG_MODE_USER_TRACK
    /* 上报或者停止上报反应user实时吞吐统计信息: sh hipriv.sh "vap_name thrput_stat  XX:XX:XX:XX:XX:XX 0|1" */
    {"thrput_stat",             wal_hipriv_report_thrput_stat},
#endif
#ifdef _PRE_WLAN_DFT_STAT
    /* 清零指定VAP的统计信息: hipriv "vap_name clear_vap_stat_info" */
    {"clear_vap_stat_info",     wal_hipriv_clear_vap_stat_info},
#endif
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    /* 设置mac txop ps使能寄存器，sh hipriv.sh "stavap_name txopps_hw_en 0|1(txop_ps_en)
     * 0|1(condition1) 0|1(condition2)"
     */
    {"txopps_hw_en",            wal_hipriv_set_txop_ps_machw},
#endif
#ifdef _PRE_WLAN_FEATURE_UAPSD
    /* uapsd维测信息，sh hipriv "vap0 uapsd_debug 0|1|2(单用户|all user|清空统计计数器) xx:xx:xx:xx:xx:xx(mac地址)" */
    {"uapsd_debug",             wal_hipriv_uapsd_debug},
#endif
#ifdef _PRE_WLAN_DFT_STAT
    /* 上报或者清零用户队列统计信息: sh hipriv.sh "vap_name usr_queue_stat XX:XX:XX:XX:XX:XX 0|1" */
    {"usr_queue_stat",          wal_hipriv_usr_queue_stat},
#endif
    /* 设置AMPDU聚合个数: sh hipriv.sh "Hisilicon0 ampdu_aggr_num aggr_num_switch aggr_num"
     * ,aggr_num_switch非0时，aggr_num有效
     */
    {"ampdu_aggr_num",          wal_hipriv_set_ampdu_aggr_num},
    {"set_stbc_cap",            wal_hipriv_set_stbc_cap},           /* 设置STBC能力 */
    {"set_ldpc_cap",            wal_hipriv_set_ldpc_cap},           /* 设置LDPC能力 */
#ifdef _PRE_WLAN_FEATURE_STA_PM
    {"set_psm_para",            wal_hipriv_sta_psm_param},          /* sh hipriv.sh 'wlan0 set_psm_para 100 40 */
    {"set_sta_pm_on",           wal_hipriv_sta_pm_on},              /* sh hipriv.sh 'wlan0 set_sta_pm_on xx xx xx xx */
#endif

    /* 设置chip test中强制使能pmf能力 (用于关联之后)??
     * sh hipriv.sh "vap0 set_default_key x(key_index) 0|1(en_unicast) 0|1(multicast)"
     */
    {"set_default_key",         wal_hipriv_set_default_key},
    /* chip test配置add key操作的私有配置命令接口??
     * sh hipriv.sh "xxx(cipher) xx(en_pairwise) xx(key_len) xxx(key_index) xxxx:xx:xx:xx:xx:xx...(key 小于32字节)
     * xx:xx:xx:xx:xx:xx(目的地址)
     */
    {"add_key",                 wal_hipriv_test_add_key},
#ifdef _PRE_WLAN_FEATURE_DFR
    /* dfr功能打桩触发接口?箂h hipriv.sh "vap0 dfr_start 0(dfr子功能:0-device异常复位 )" */
    {"dfr_start",              wal_hipriv_test_dfr_start},
#endif //_PRE_WLAN_FEATURE_DFR
    /* 算法相关的命令 */
    {"alg_ar_log",              wal_hipriv_ar_log},                 /* autorate算法日志参数配置: */
    {"alg_ar_test",             wal_hipriv_ar_test},                /* autorate算法系统测试命令 */
    {"alg",                     wal_hipriv_alg},                    /* alg */
    /* 开启或关闭tx beamforming能力 hipriv "vap0 alg_txbf_switch 0|1" */
    {"alg_txbf_switch",         wal_hipriv_txbf_switch},
    {"alg_txbf_log",            wal_hipriv_txbf_log},               /* autorate算法日志参数配置: */
    {"alg_cca_opt_log",         wal_hipriv_cca_opt_log},            /* cca算法日志参数配置: */

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    {"set_edca_weight_sta",        wal_hipriv_set_edca_opt_weight_sta},       /* STA edca参数调整权重 */
    {"set_edca_switch_ap",         wal_hipriv_set_edca_opt_switch_ap},        /* 是否开启edca优化机制 */
    {"set_edca_cycle_ap",          wal_hipriv_set_edca_opt_cycle_ap},         /* 设置edca参数调整的周期 */
#endif

    {"get_hipkt_stat",             wal_hipriv_get_hipkt_stat},                /* 获取高优先级报文的统计情况 */
    {"set_flowctl_param",          wal_hipriv_set_flowctl_param},             /* 设置流控相关参数 */
    {"get_flowctl_stat",           wal_hipriv_get_flowctl_stat},              /* 获取流控相关状态信息 */

    {"auto_protection",         wal_hipriv_set_auto_protection},       /* 设置自动保护开关 */

    /* 共存维测相关 */
    /* 发送20/40共存管理帧: hipriv "Hisilicon0 send_2040_coext coext_info chan_report" */
    {"send_2040_coext",         wal_hipriv_send_2040_coext},
    /* 打印vap的所有20/40共存参数信息: hipriv "vap0 2040_coext_info" */
    {"2040_coext_info",         wal_hipriv_2040_coext_info},
    /* 获取软件版本: hipriv "vap0 get_version" */
    {"get_version",             wal_hipriv_get_version},

#ifdef _PRE_WLAN_FEATURE_FTM
    /* hipriv.sh "wlan0 ftm_debug ------------------------------------------------------------------------------------*/
    /* -------------------------- enable_ftm_initiator [0|1] */
    /* -------------------------- send_initial_ftm_request channel[] ftms_per_burst[n] burst_num[n]
                                 en_lci_civic_request[0|1] asap[0|1] bssid[xx:xx:xx:xx:xx:xx] format_bw[9~13] */
    /* -------------------------- enable [0|1] */
    /* -------------------------- cali [0|1] */
    /* -------------------------- send_ftm bssid[xx:xx:xx:xx:xx:xx] */
    /* -------------------------- set_ftm_time t1[] t2[] t3[] t4[] */
    /* -------------------------- enable_ftm_responder [0|1] */
    /* -------------------------- send_range_req mac[] num_rpt[] delay[] ap_cnt[] bssid[]
                                 channel[] bssid[] channel[] ... */
    /* -------------------------- enable_ftm_range [0|1] */
    /* -------------------------- get_cali */
    /* -------------------------- set_location type[] mac[] mac[] mac[] */
    /* -------------------------- set_ftm_m2s en_is_mimo[] uc_tx_chain_selection[] */
    {"ftm_debug",               wal_hipriv_ftm},
#endif

#ifdef _PRE_WLAN_FEATURE_CSI_RAM
    /* 使能CSI上报: hipriv "Hisilicon0 set_csi xx xx xx xx xx xx(mac地址) ta_check csi_en" */
    {"set_csi",                 wal_hipriv_set_csi},
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    /* 设置VAP工作模式通知: hipriv "vap0 set_opmode_notify 0/1"  0-不支持; 1-支持 */
    {"set_opmode_notify",       wal_hipriv_set_opmode_notify},
    /* 设置添加用户的配置命令: hipriv "vap0 get_user_nssbw xx xx xx xx xx xx(mac地址) "  该命令针对某一个VAP */
    {"get_user_nssbw",          wal_hipriv_get_user_nssbw},
#endif

#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY

    {"blacklist_add",           wal_hipriv_blacklist_add},          /* 1 */
    {"blacklist_del",           wal_hipriv_blacklist_del},          /* 2 */
    {"blacklist_mode",          wal_hipriv_set_blacklist_mode},     /* 3 */
    {"blacklist_show",          wal_hipriv_blacklist_show},         /* 4 wal_config_blacklist_show */
    {"abl_on",                  wal_hipriv_set_abl_on},             /* 5 */
    {"abl_aging",               wal_hipriv_set_abl_aging},          /* 6 */
    {"abl_threshold",           wal_hipriv_set_abl_threshold},      /* 7 */
    {"abl_reset",               wal_hipriv_set_abl_reset},          /* 8 wal_config_set_autoblacklist_reset_time */
    {"isolation_mode",          wal_hipriv_set_isolation_mode},     /* 9 */
    {"isolation_type",          wal_hipriv_set_isolation_type},     /* 10 */
    {"isolation_fwd",           wal_hipriv_set_isolation_fwd},      /* 11 */
    {"isolation_clear",         wal_hipriv_set_isolation_clear},    /* 12 wal_config_set_isolation_clear */
    {"isolation_show",          wal_hipriv_set_isolation_show},     /* 13 wal_config_isolation_show */

#endif
    /* device级别配置命令 设置基于vap的业务分类是否使能 hipriv "Hisilicon0 vap_classify_en 0/1" */
    {"vap_classify_en",         wal_hipriv_vap_classify_en},
    /* 设置vap的流等级 hipriv "vap0 classify_tid 0~7" */
    {"vap_classify_tid",        wal_hipriv_vap_classify_tid},

    {"txpower",         wal_hipriv_set_txpower},                 /* 设置最大发送功率 */
#if  (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
    {"essid",           wal_hipriv_set_essid},                   /* 设置AP ssid */
    {"bintval",         wal_ioctl_set_beacon_interval},         /* 设置AP beacon 周期 */
    {"up",              wal_hipriv_start_vap},
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44)) */

#ifdef _PRE_WLAN_FEATURE_11D
    /* 设置是否根据关联ap更新国家码信息 hipriv "Hisilicon0 set_rd_by_ie_switch 0/1" */
    {"set_rd_by_ie_switch",      wal_hipriv_set_rd_by_ie_switch},
#endif
#ifdef  _PRE_WLAN_FEATURE_P2P
    /* sh hipriv.sh "vap0 p2p_ps ops 0/1(0不使能，1使能) [0~255] 设置OPS 节能下ct_window 参数 */
    /* sh hipriv.sh "vap0 p2p_ps noa start_time duration interval count 设置NOA 节能参数 */
    /* sh hipriv.sh "vap0 p2p_ps statistics 0/1(0 清空统计，1查看统计) P2P 中断统计 */
#endif  /* _PRE_WLAN_FEATURE_P2P */
#ifdef _PRE_WLAN_NARROW_BAND
    {"narrow_bw",                wal_hipriv_narrow_bw},                /* 设置窄带参数 */
    {"hitalk_set",               wal_hipriv_hitalk_set},               /* 加载独立固件命令 */
    {"hitalk_bg_listen",         wal_hipriv_hitalk_bg_listen},         /* 设置hitalk后台监听 */
#endif
#ifdef _PRE_WLAN_TCP_OPT
    /* 显示TCP ACK 过滤统计值 sh hipriv.sh "vap0 get_tx_ack_stream_info */
    {"get_tcp_ack_stream_info",               wal_hipriv_get_tcp_ack_stream_info},
    /* 设置发送TCP ACK优化使能  sh hipriv.sh "vap0 tcp_tx_ack_opt_enable 0 | 1 */
    {"tcp_tx_ack_opt_enable",                 wal_hipriv_tcp_tx_ack_opt_enable},
    /* 设置接收TCP ACK优化使能 sh hipriv.sh "vap0 tcp_rx_ack_opt_enable 0 | 1 */
    {"tcp_rx_ack_opt_enable",                 wal_hipriv_tcp_rx_ack_opt_enable},
    /* 设置发送TCP ACK LIMIT sh hipriv.sh "vap0 tcp_tx_ack_opt_limit X */
    {"tcp_tx_ack_opt_limit",                  wal_hipriv_tcp_tx_ack_limit},
    /* 设置接收TCP ACKLIMIT  sh hipriv.sh "vap0 tcp_tx_ack_opt_limit X */
    {"tcp_rx_ack_opt_limit",                  wal_hipriv_tcp_rx_ack_limit},
#endif

#ifdef _PRE_WLAN_FEATURE_WAPI
#ifdef _PRE_WAPI_DEBUG
    /* wapi hipriv "vap0 wal_hipriv_show_wapi_info " */
    {"wapi_info",                             wal_hipriv_show_wapi_info},
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */
#ifdef _PRE_WLAN_FEATURE_ROAM
    {"roam_enable",      wal_hipriv_roam_enable},   /* 设置漫游开关 */
    {"roam_org",         wal_hipriv_roam_org},      /* 设置漫游正交 */
    {"roam_band",        wal_hipriv_roam_band},     /* 设置漫游频段 */
    /* 漫游测试命令  hipriv "vap0 roam_start 0|1" 0或者参数缺失表示扫描+漫游, 1表示漫游前不扫描 */
    {"roam_start",       wal_hipriv_roam_start},
    {"roam_info",        wal_hipriv_roam_info},     /* 漫游信息打印 */
#endif  //_PRE_WLAN_FEATURE_ROAM
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    /* 设置20/40 bss使能: hipriv "Hisilicon0 2040bss_enable 0|1" 0表示20/40 bss判断关闭，1表示使能 */
    {"2040bss_enable",   wal_hipriv_enable_2040bss},
#endif
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    /* 设置自动调频使能: hipriv "wlan0 auto_freq 0 0" 第二个参数0表示关闭，1表示使能 */
    {"auto_freq",   wal_hipriv_set_auto_freq},
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    {"customize_info",   wal_hipriv_dev_customize_info},            /* 打印device侧定制化信息 */
    /* 查询发射能力命令 sh hipriv.sh "Hisilicon0 get_lauch_cap" */
    {"get_lauch_cap",    wal_hipriv_get_lauch_cap},
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    /*  动态校准参数配置 sh hipriv "wlan0 dyn_cali   " */
    {"dyn_cali",                       wal_hipriv_dyn_cali_cfg},
#endif
    /* 设置打印phy维测的相关信息，sh hipriv.sh "wlan0 protocol_debug [band_force_switch 0|1|2]
     * [csa 0|1 [channel] [bandwidth] [cnt] 0|1|2]"
     */
    {"protocol_debug",                 wal_hipriv_set_protocol_debug_info},
    /* 设置sk_pacing_shift  hipriv "wlan0 sk_pacing_shift <value>" */
    {"sk_pacing_shift",                wal_hipriv_sk_pacing_shift},
    /* 向目标STA发送校验数据帧: hipriv "vap0 packet_check  [num 0-100](单个type报文个数)
     * [thr 0-100](超时门限百分比) [time 0-10](超时时间单位s)"
     */
    {"packet_check",                   wal_hipriv_packet_check},

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    {"assigned_filter",                wal_hipriv_set_assigned_filter_switch}, /* 过滤指定rx报文:[1~20] 0|1 */
#endif
    /* 打印低功耗帧过滤统计 sh hipriv.sh "wlan0 psm_flt_stat [0|1|2|...]" 目前只支持了0 */
    { "psm_flt_info",                  wal_hipriv_psm_flt_info},
    /* 设置打印接收报文rssi信息的开关，sh hipriv.sh "wlan0 opmode_switch [opmode 0|1](打开|关闭)" */
    {"opmode_switch",                  wal_hipriv_set_opmode_switch},
};

oal_uint32 wal_hipriv_get_debug_cmd_size(oal_void)
{
    return oal_array_size(g_ast_hipriv_cmd_debug);
}

#endif


