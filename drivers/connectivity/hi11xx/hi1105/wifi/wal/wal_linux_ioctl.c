

#include "oal_ext_if.h"
#include "oal_kernel_file.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
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
#include "wal_config_alg_cfg.h"
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
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
#include "hisi_customize_wifi.h"
#endif
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#include "plat_firmware.h"
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif

#ifdef _PRE_WLAN_FEATURE_DFR
#include "hmac_dfx.h"
#endif  /* _PRE_WLAN_FEATURE_DFR */

#include "wal_linux_ioctl.h"
#ifdef _PRE_WLAN_FEATURE_11AX
#include "hmac_wifi6_self_cure.h"
#endif

#ifdef _PRE_PRODUCT_HI3751V811
/* 熄屏状态下，仅响应永久组内成员的probe request帧，该功能需要包含注册屏幕状态相关钩子函数的头文件 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "drv_pmoc_ext.h"
#include "hi_module.h"
#endif
#endif

#include "wlan_mib.h"
#ifdef _PRE_WLAN_FEATURE_HIEX
#include "mac_hiex.h"
#include "hmac_hiex.h"
#endif
#include "hmac_tx_data.h"

#ifdef _PRE_WLAN_FEATURE_FTM
#include "mac_ftm.h"
#endif
#include "wal_cfg_ioctl.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "board.h"
#endif
#if defined(WIN32)
#include "hisi_customize_wifi.h"
#endif
#include "securec.h"
#include "securectype.h"

#ifdef _PRE_WLAN_RR_PERFORMANCE
#include "plat_firmware.h"
#include "hmac_auto_adjust_freq.h"
#include "wlan_mib.h"
#include "hmac_config.h"
#include "oal_hcc_bus.h"
#endif /* _PRE_WLAN_RR_PERFORMANCE */
#include "wal_linux_vendor.h"
#include "wal_linux_netdev_ops.h"
#include "wal_linux_customize.h"
#include "hmac_hid2d.h"
#if defined(CONFIG_HISI_CMDLINE_PARSE) || defined(CONFIG_CMDLINE_PARSE)
unsigned int get_boot_into_recovery_flag(void);
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID      OAM_FILE_ID_WAL_LINUX_IOCTL_C

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
typedef struct {
    int8_t *puc_string;
    mac_tcp_ack_buf_cfg_cmd_enum_uint8 en_tcp_ack_buf_cfg_id;
    uint8_t auc_resv[NUM_3_BYTES];
} mac_tcp_ack_buf_cfg_table_stru;

static mac_tcp_ack_buf_cfg_table_stru g_ast_hmac_tcp_ack_buf_cfg_table[] = {
    { "enable",  MAC_TCP_ACK_BUF_ENABLE },
    { "timeout", MAC_TCP_ACK_BUF_TIMEOUT },
    { "count",   MAC_TCP_ACK_BUF_MAX },
};
#endif

OAL_STATIC oal_proc_dir_entry_stru *g_pst_proc_entry = NULL;

OAL_STATIC wal_msg_queue g_wal_wid_msg_queue;

wal_ap_config_stru g_st_ap_config_info = { 0 }; /* AP配置信息,需要在vap 创建后下发的 */

/* hi1102-cb add sys for 51/02 */
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
struct kobject *g_gp_sys_kobject;
#endif

oal_wait_queue_head_stru g_wlan_cali_complete_wq;
uint8_t g_go_cac = OAL_TRUE;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
int32_t wal_hipriv_inetaddr_notifier_call(struct notifier_block *this, unsigned long event, void *ptr);

OAL_STATIC struct notifier_block g_wal_hipriv_notifier = {
    .notifier_call = wal_hipriv_inetaddr_notifier_call
};

int32_t wal_hipriv_inet6addr_notifier_call(struct notifier_block *this, unsigned long event, void *ptr);

OAL_STATIC struct notifier_block g_wal_hipriv_notifier_ipv6 = {
    .notifier_call = wal_hipriv_inet6addr_notifier_call
};
#endif

#ifdef _PRE_WLAN_CFGID_DEBUG
extern const wal_hipriv_cmd_entry_stru g_ast_hipriv_cmd_debug[];
#endif
/* 静态函数声明 */
OAL_STATIC uint32_t wal_hipriv_vap_log_level(oal_net_device_stru *net_dev, int8_t *pc_param);
OAL_STATIC uint32_t wal_hipriv_getcountry(oal_net_device_stru *net_dev, int8_t *pc_param);
OAL_STATIC uint32_t wal_hipriv_set_bw(oal_net_device_stru *net_dev, int8_t *pc_param);
OAL_STATIC uint32_t wal_hipriv_always_tx(oal_net_device_stru *net_dev, int8_t *pc_param);
OAL_STATIC uint32_t wal_hipriv_set_ru_index(oal_net_device_stru *net_dev, int8_t *param);
OAL_STATIC uint32_t wal_hipriv_always_rx(oal_net_device_stru *net_dev, int8_t *pc_param);
#ifdef _PRE_WLAN_FEATURE_MONITOR
OAL_STATIC uint32_t wal_hipriv_set_sniffer(oal_net_device_stru *net_dev, int8_t *pc_param);
OAL_STATIC uint32_t wal_hipriv_enable_monitor_mode(oal_net_device_stru *net_dev, int8_t *pc_param);
#endif
OAL_STATIC uint32_t wal_hipriv_user_info(oal_net_device_stru *net_dev, int8_t *pc_param);
OAL_STATIC uint32_t wal_hipriv_add_vap(oal_net_device_stru *cfg_net_dev, int8_t *pc_param);
OAL_STATIC uint32_t wal_hipriv_add_user(oal_net_device_stru *net_dev, int8_t *pc_param);
OAL_STATIC uint32_t wal_hipriv_del_user(oal_net_device_stru *net_dev, int8_t *pc_param);
#ifdef PLATFORM_DEBUG_ENABLE
OAL_STATIC uint32_t wal_hipriv_reg_info(oal_net_device_stru *net_dev, int8_t *pc_param);
#endif
OAL_STATIC uint32_t wal_hipriv_sdio_flowctrl(oal_net_device_stru *net_dev, int8_t *pc_param);
#ifdef _PRE_WLAN_DELAY_STATISTIC
OAL_STATIC uint32_t wal_hipriv_pkt_time_switch(oal_net_device_stru *net_dev, int8_t *pc_param);
#endif

OAL_STATIC uint32_t wal_hipriv_rx_fcs_info(oal_net_device_stru *net_dev, int8_t *pc_param);
OAL_STATIC uint32_t wal_hipriv_set_mode(oal_net_device_stru *net_dev, int8_t *pc_param);
OAL_STATIC uint32_t wal_hipriv_set_freq(oal_net_device_stru *net_dev, int8_t *pc_param);

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
OAL_STATIC uint32_t wal_hipriv_tcp_ack_buf_cfg(oal_net_device_stru *net_dev, int8_t *pc_param);
#endif

int32_t wal_ioctl_get_blkwhtlst(oal_net_device_stru *net_dev, oal_net_dev_ioctl_data_stru *pst_ioctl_data);
OAL_STATIC int wal_ioctl_get_essid(oal_net_device_stru *net_dev, oal_iw_request_info_stru *pst_info,
                                   oal_iwreq_data_union *pst_wrqu,
                                   char *pc_param);
OAL_STATIC int wal_ioctl_get_apaddr(oal_net_device_stru *net_dev,
                                    oal_iw_request_info_stru *pst_info,
                                    oal_iwreq_data_union *pst_wrqu,
                                    char *pc_extra);
OAL_STATIC int wal_ioctl_get_iwname(oal_net_device_stru *net_dev,
                                    oal_iw_request_info_stru *pst_info,
                                    oal_iwreq_data_union *pst_wrqu,
                                    char *pc_extra);
#ifdef PLATFORM_DEBUG_ENABLE
OAL_STATIC uint32_t wal_hipriv_reg_write(oal_net_device_stru *net_dev, int8_t *pc_param);
#endif
OAL_STATIC uint32_t wal_hipriv_tpc_log(oal_net_device_stru *net_dev, int8_t *pc_param);

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
/* hi1102-cb add sys for 51/02 */
OAL_STATIC oal_ssize_t wal_hipriv_sys_write(struct kobject *dev, struct kobj_attribute *attr,
    const char *buf, oal_size_t count);
OAL_STATIC oal_ssize_t wal_hipriv_sys_read(struct kobject *dev, struct kobj_attribute *attr, char *buf);
OAL_STATIC struct kobj_attribute g_dev_attr_hipriv =
    __ATTR(hipriv, (OAL_S_IRUGO | OAL_S_IWUSR), wal_hipriv_sys_read, wal_hipriv_sys_write);
OAL_STATIC struct attribute *g_hipriv_sysfs_entries[] = {
    &g_dev_attr_hipriv.attr,
    NULL
};
OAL_STATIC struct attribute_group g_hipriv_attribute_group = {
    .attrs = g_hipriv_sysfs_entries,
};
/* hi1102-cb add sys for 51/02 */
#endif

#ifdef _PRE_WLAN_FEATURE_M2S_MSS
OAL_STATIC uint32_t wal_ioctl_set_m2s_blacklist(oal_net_device_stru *net_dev,
    uint8_t *puc_buf, uint32_t buf_len, uint8_t uc_m2s_blacklist_cnt);
uint32_t wal_ioctl_set_m2s_mss(oal_net_device_stru *net_dev, uint8_t uc_m2s_mode);
#endif

#ifdef _PRE_WLAN_FEATURE_HS20
OAL_STATIC int32_t wal_ioctl_set_qos_map(oal_net_device_stru *net_dev,
    hmac_cfg_qos_map_param_stru *pst_qos_map_param);
#endif /* #ifdef _PRE_WLAN_FEATURE_HS20 */

OAL_STATIC int32_t wal_set_ap_max_user(oal_net_device_stru *net_dev, uint32_t ap_max_user);
OAL_STATIC void wal_config_mac_filter(oal_net_device_stru *net_dev, int8_t *pc_command);
OAL_STATIC int32_t wal_kick_sta(oal_net_device_stru *net_dev,
    uint8_t *auc_mac_addr, uint8_t uc_mac_addr_len, uint16_t us_reason_code);
OAL_STATIC int wal_ioctl_set_ap_config(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra);
OAL_STATIC int wal_ioctl_set_mac_filters(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra);
OAL_STATIC int wal_ioctl_get_assoc_list(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra);
OAL_STATIC int wal_ioctl_set_ap_sta_disassoc(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra);
OAL_STATIC uint32_t wal_get_parameter_from_cmd(int8_t *pc_cmd,
    int8_t *pc_arg, const int8_t *puc_token, uint32_t *pul_cmd_offset, uint32_t param_max_len);
OAL_STATIC uint32_t wal_hipriv_set_band(oal_net_device_stru *net_dev, int8_t *pc_param);

int32_t wal_init_wlan_vap(oal_net_device_stru *net_dev);
int32_t wal_deinit_wlan_vap(oal_net_device_stru *net_dev);
int32_t wal_start_vap(oal_net_device_stru *net_dev);
int32_t wal_stop_vap(oal_net_device_stru *net_dev);
OAL_STATIC int32_t wal_set_mac_addr(oal_net_device_stru *net_dev);
int32_t wal_init_wlan_netdev(oal_wiphy_stru *pst_wiphy, const char *dev_name);
int32_t wal_setup_ap(oal_net_device_stru *net_dev);

#ifdef _PRE_WLAN_FEATURE_11D
int32_t wal_regdomain_update_for_dfs(oal_net_device_stru *net_dev, int8_t *pc_country);
int32_t wal_regdomain_update(oal_net_device_stru *net_dev, int8_t *pc_country);
#endif


#ifdef _PRE_WLAN_FEATURE_11K
OAL_STATIC uint32_t wal_hipriv_send_neighbor_req(oal_net_device_stru *net_dev, int8_t *pc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_APF
OAL_STATIC uint32_t wal_hipriv_apf_filter_list(oal_net_device_stru *net_dev, int8_t *pc_param);
#endif

#ifdef _PRE_WLAN_RR_PERFORMANCE
OAL_STATIC int32_t wal_ext_priv_cmd_dev_freq_check_para(oal_net_device_stru *net_dev);
OAL_STATIC int32_t wal_priv_cmd_dev_freq_para(mac_vap_stru *mac_vap, mac_cfg_set_tlv_stru *config_para,
    uint8_t uc_cmd_type, uint8_t value);
OAL_STATIC int32_t wal_ext_priv_cmd_set_dev_freq(oal_net_device_stru *net_dev, uint8_t uc_flag);
OAL_STATIC int32_t wal_ext_priv_cmd_pcie_aspm(int32_t aspm);
#endif /* _PRE_WLAN_RR_PERFORMANCE */
OAL_STATIC uint32_t wal_hipriv_cali_debug(oal_net_device_stru *net_dev, int8_t *pc_param);
OAL_STATIC int32_t wal_wlan_cfg_module_process_entry(wlan_cfg_param_stru *wlan_cfg_param);

const wal_hipriv_cmd_entry_stru  g_ast_hipriv_cmd[] = {
    /* 商用对外发布的私有命令 */
    /* 打印vap的所有参数信息: hipriv "vap0 info" */
    { "info",                 wal_hipriv_vap_info },
    /* 设置国家码命令 hipriv "Hisilicon0 setcountry param "param取值为大写的国家码字，例如 CN US */
    { "setcountry",           wal_hipriv_setcountry },
    /* 查询国家码命令 hipriv "Hisilicon0 getcountry" */
    { "getcountry",           wal_hipriv_getcountry },
    /* ip filter(功能调试接口)hipriv "wlan0 ip_filter cmd param0 param1 ...."
       举例:启动功能 "wlan0 ip_filter set_rx_filter_enable 1/0"
       清空黑名单 "wlan0 ip_filter clear_rx_filters"
       设置黑名单 "wlan0 ip_filter add_rx_filter_items 条目个数(0/1/2...)
       名单内容(protocol0 port0 protocol1 port1...)",目前该调试接口仅支持20对条目
     */
    { "ip_filter",            wal_hipriv_set_ip_filter },
    /* 打印指定mac地址user的所有参数信息: hipriv "vap0 userinfo XX XX XX XX XX XX(16进制oal_strtohex)" */
    { "userinfo",             wal_hipriv_user_info },
#ifdef PLATFORM_DEBUG_ENABLE
    /* 打印寄存器信息: hipriv "Hisilicon0 reginfo 16|32 regtype(soc/mac/phy) startaddr endaddr" */
    { "reginfo",              wal_hipriv_reg_info },
    /* 写寄存器: hipriv "Hisilicon0 regwrite 32/16(51没有16位写寄存器功能) regtype(soc/mac/phy) addr val"
     * addr val必须都是16进制0x开头
     */
    { "regwrite",             wal_hipriv_reg_write },
#endif
    { "create",               wal_hipriv_add_vap }, /* 创建vap私有命令为: hipriv "Hisilicon0 create vap0 ap|sta" */
    /* 设置常发模式:hipriv "vap0 al_tx <value: 0/1/2>  <len>" 由于mac限制，
       11a,b,g下只支持4095以下数据发送,可以使用set_mcast_data对速率进行设置 */
    { "al_tx",                wal_hipriv_always_tx },
    { "al_tx_1102",           wal_hipriv_always_tx },        /* al_tx_02命令先保留 */
    { "al_ru_index",          wal_hipriv_set_ru_index }, /* 设置ru index: hipriv "vap0 al_ru_index 106tone 0 1" */
    { "al_rx",                wal_hipriv_always_rx },  /* 设置常收模式: hipriv "vap0 al_rx <value: 0/1/2>" */
#ifdef _PRE_WLAN_FEATURE_MONITOR
    { "sniffer",              wal_hipriv_set_sniffer }, /* 设置抓包模式: sh hipriv.sh "wlan0 sniffer <value: 0/1>" */
    { "monitor",              wal_hipriv_enable_monitor_mode }, /* sh hipriv.sh "wlan0 monitor <value: 0/1>" */
#endif
    { "rate",  wal_hipriv_set_rate },    /* 设置non-HT模式下的速率: hipriv "vap0 rate  <value>" */
    { "mcs",   wal_hipriv_set_mcs },     /* 设置HT模式下的速率: hipriv "vap0 mcs   <value>" */
    { "mcsac", wal_hipriv_set_mcsac },   /* 设置VHT模式下的速率: hipriv "vap0 mcsac <value>" */
#ifdef _PRE_WLAN_FEATURE_11AX
    { "mcsax", wal_hipriv_set_mcsax },       /* 设置HE模式下的速率:  hipriv "vap0 mcsax <value>" */
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
    { "mcsax_er", _wal_hipriv_set_mcsax_er }, /* 设置HE ER模式下的速率:      hipriv "vap0 mcsax_er <value>" */
#endif
#endif
    { "freq",              wal_hipriv_set_freq },   /* 设置AP 信道 */
    { "mode",              wal_hipriv_set_mode },   /* 设置AP 协议模式 */
    { "band",              wal_hipriv_set_band },   /* 设置VAP 工作频段 */
    { "bw",                wal_hipriv_set_bw },     /* 设置带宽: hipriv "vap0 bw    <value>" */
    /* 打印描述符信息: hipriv "vap0 set_mcast_data <param name> <value>" */
    { "set_mcast_data",    wal_hipriv_set_mcast_data_dscr_param },
    /* 打印接收帧的FCS正确与错误信息:hipriv "vap0 rx_fcs_info 0/1 1/2/3/4" 0/1  0代表不清除，1代表清除 */
    { "rx_fcs_info",       wal_hipriv_rx_fcs_info },
    /* 设置添加用户的配置命令: hipriv "vap0 add_user xx xx xx xx xx xx(mac地址) 0|1(HT能力位) " 该命令针对某一个VAP */
    { "add_user",          wal_hipriv_add_user },
    /* 设置删除用户的配置命令: hipriv "vap0 del_user xx xx xx xx xx xx(mac地址)" 该命令针对某一个VAP */
    { "del_user",        wal_hipriv_del_user },
    { "alg",     wal_hipriv_alg_cfg_entry }, /* 算法参数配置:sh hipriv.sh "wlan0 alg txmode_mode_sw <value>" */
    { "alg_cfg",         wal_hipriv_alg_cfg }, /* 算法参数配置: hipriv "vap0 alg_cfg sch_vi_limit 10" */
    { "alg_tpc_log", wal_hipriv_tpc_log }, /* tpc算法日志参数配置: */
    { "sdio_flowctrl", wal_hipriv_sdio_flowctrl },
#ifdef _PRE_WLAN_DELAY_STATISTIC
    /* 报文时间戳调试开关: hipriv "Hisilicon0 pkt_time_switch on |off */
    { "pkt_time_switch", wal_hipriv_pkt_time_switch },
#endif
#ifdef _PRE_WLAN_FEATURE_11K
    { "send_neighbor_req", wal_hipriv_send_neighbor_req },  /* sh hipriv.sh "wlan0 send_neighbor_req WiFi1" */
#endif
    /* VAP级别日志级别 hipriv "VAPX log_level {1|2}"  Warning与Error级别日志以VAP为维度 */
    { "log_level", wal_hipriv_vap_log_level },

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    { "tcp_ack_buf", wal_hipriv_tcp_ack_buf_cfg }, /* 快速老化 */
#endif

#ifdef _PRE_WLAN_FEATURE_APF
    { "apf_filter_list", wal_hipriv_apf_filter_list },
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
    { "set_hid2d_acs_mode", wal_ioctl_set_hid2d_acs_mode },
#ifdef _PRE_WLAN_FEATURE_HID2D_PRESENTATION
    { "switch_chan", wal_hipriv_hid2d_switch_channel },
#endif
#endif
    { "cali",   wal_hipriv_cali_debug },
#ifdef _PRE_HOST_PERFORMANCE
    { "host_time",   wal_hipriv_show_host_time },
#endif
};

/* net_device上挂接的net_device_ops函数 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_net_device_ops_stru g_st_wal_net_dev_ops = {
    .ndo_get_stats = wal_netdev_get_stats,
    .ndo_open = wal_netdev_open_ext,
    .ndo_stop = wal_netdev_stop,
    .ndo_start_xmit = wal_bridge_vap_xmit,

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    /* Temp blank */
#else
    .ndo_set_multicast_list = NULL,
#endif

    .ndo_do_ioctl = wal_net_device_ioctl,
    .ndo_change_mtu = oal_net_device_change_mtu,
    .ndo_init = oal_net_device_init,

    .ndo_select_queue = wal_netdev_select_queue,

    .ndo_set_mac_address = wal_netdev_set_mac_addr
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
/* 不支持ethtool_ops 相关函数操作,需要显示定义空的结构体，否则将采用内核默认的ethtool_ops会导致空指针异常 */
oal_ethtool_ops_stru g_st_wal_ethtool_ops = { 0 };
#endif

#ifdef _PRE_WLAN_FEATURE_NAN
/* xmit回调必须注册，NAN注册一个空函数。其他接口不需要注册。 */
OAL_STATIC oal_net_dev_tx_enum wal_nan_xmit(oal_netbuf_stru *buf, oal_net_device_stru *dev)
{
    oal_netbuf_free(buf);
    return NETDEV_TX_OK;
}

OAL_STATIC oal_net_device_ops_stru g_nan_net_dev_ops = {
    .ndo_start_xmit = wal_nan_xmit,
};
#endif
#endif

/* 标准ioctl命令函数表 */
OAL_STATIC const oal_iw_handler g_ast_iw_handlers[] = {
    NULL,                         /* SIOCSIWCOMMIT, */
    (oal_iw_handler)wal_ioctl_get_iwname, /* SIOCGIWNAME, */
    NULL,                         /* SIOCSIWNWID, */
    NULL,                         /* SIOCGIWNWID, */
    NULL,                         /* SIOCSIWFREQ, 设置频点信道 */
    NULL,                         /* SIOCGIWFREQ, 获取频点信道 */
    NULL,                         /* SIOCSIWMODE, 设置bss type */
    NULL,                         /* SIOCGIWMODE, 获取bss type */
    NULL,                         /* SIOCSIWSENS, */
    NULL,                         /* SIOCGIWSENS, */
    NULL, /* SIOCSIWRANGE, */     /* not used */
    NULL,                         /* SIOCGIWRANGE, */
    NULL, /* SIOCSIWPRIV, */      /* not used */
    NULL, /* SIOCGIWPRIV, */      /* kernel code */
    NULL, /* SIOCSIWSTATS, */     /* not used */
    NULL,                         /* SIOCGIWSTATS, */
    NULL,                         /* SIOCSIWSPY, */
    NULL,                         /* SIOCGIWSPY, */
    NULL,                         /* -- hole -- */
    NULL,                         /* -- hole -- */
    NULL,                         /* SIOCSIWAP, */
    (oal_iw_handler)wal_ioctl_get_apaddr, /* SIOCGIWAP, */
    NULL,                         /* SIOCSIWMLME, */
    NULL,                         /* SIOCGIWAPLIST, */
    NULL,                         /* SIOCSIWSCAN, */
    NULL,                         /* SIOCGIWSCAN, */
    NULL,                         /* SIOCSIWESSID, 设置ssid */
    (oal_iw_handler)wal_ioctl_get_essid,  /* SIOCGIWESSID, 获取ssid */
    NULL,                         /* SIOCSIWNICKN */
    NULL,                         /* SIOCGIWNICKN */
    NULL,                         /* -- hole -- */
    NULL,                         /* -- hole -- */
    NULL,                         /* SIOCSIWRATE */
    NULL,                         /* SIOCGIWRATE  get_iwrate */
    NULL,                         /* SIOCSIWRTS */
    NULL,                         /* SIOCGIWRTS  get_rtsthres */
    NULL,                         /* SIOCSIWFRAG */
    NULL,                         /* SIOCGIWFRAG  get_fragthres */
    NULL,                         /* SIOCSIWTXPOW, 设置传输功率限制 */
    NULL,                         /* SIOCGIWTXPOW, 设置传输功率限制 */
    NULL,                         /* SIOCSIWRETRY */
    NULL,                         /* SIOCGIWRETRY */
    NULL,                         /* SIOCSIWENCODE */
    NULL,                         /* SIOCGIWENCODE  get_iwencode */
    NULL,                         /* SIOCSIWPOWER */
    NULL,                         /* SIOCGIWPOWER */
    NULL,                         /* -- hole -- */
    NULL,                         /* -- hole -- */
    NULL,                         /* SIOCSIWGENIE */
    NULL,                         /* SIOCGIWGENIE */
    NULL,                         /* SIOCSIWAUTH */
    NULL,                         /* SIOCGIWAUTH */
    NULL,                         /* SIOCSIWENCODEEXT */
    NULL                          /* SIOCGIWENCODEEXT */
};

/* 私有ioctl命令参数定义 */
OAL_STATIC const oal_iw_priv_args_stru g_ast_iw_priv_args[] = {
    { WAL_IOCTL_PRIV_SET_AP_CFG,  OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_TYPE256, 0, "AP_SET_CFG" },
    { WAL_IOCTL_PRIV_AP_MAC_FLTR, OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_TYPE256,
      OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_SIZE_FIXED | 0,         "AP_SET_MAC_FLTR" },
    { WAL_IOCTL_PRIV_AP_GET_STA_LIST, 0, OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_TYPE1024, "AP_GET_STA_LIST" },
    { WAL_IOCTL_PRIV_AP_STA_DISASSOC, OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_TYPE256,
      OAL_IW_PRIV_TYPE_CHAR | 0, "AP_STA_DISASSOC" },

    { WLAN_CFGID_ADD_BLACK_LIST, OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_TYPE40,        0,    "blkwhtlst_add" },
    { WLAN_CFGID_DEL_BLACK_LIST, OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_TYPE40,        0,    "blkwhtlst_del" },
    { WLAN_CFGID_CLR_BLACK_LIST, OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_TYPE40,        0,    "blkwhtlst_clr" },
    /* 配置黑名单模式 */
    { WLAN_CFGID_BLACKLIST_MODE, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0,    "blkwhtlst_mode" },
    /* 黑名单模式打印 */
    { WLAN_CFGID_BLACKLIST_MODE, 0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,    "get_blkwhtmode" },

    { WLAN_CFGID_GET_2040BSS_SW, 0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,    "get_obss_sw" },
    { WLAN_CFGID_2040BSS_ENABLE, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0,    "set_obss_sw" },
};

/* 私有ioctl命令函数表 */
OAL_STATIC const oal_iw_handler g_ast_iw_priv_handlers[] = {
    NULL, /* SIOCWFIRSTPRIV+0 */                                   /* sub-ioctl set 入口 */
    NULL, /* SIOCWFIRSTPRIV+1 */                                   /* sub-ioctl get 入口 */
    NULL, /* SIOCWFIRSTPRIV+2 */                                   /* setkey */
    NULL, /* SIOCWFIRSTPRIV+3 */                                   /* setwmmparams */
    NULL, /* SIOCWFIRSTPRIV+4 */                                   /* delkey */
    NULL, /* SIOCWFIRSTPRIV+5 */                                   /* getwmmparams */
    NULL, /* SIOCWFIRSTPRIV+6 */                                   /* setmlme */
    NULL, /* SIOCWFIRSTPRIV+7 */                                   /* getchaninfo */
    NULL, /* SIOCWFIRSTPRIV+8 */                                   /* setcountry */
    NULL, /* SIOCWFIRSTPRIV+9 */                                   /* getcountry */
    NULL, /* SIOCWFIRSTPRIV+10 */                                  /* addmac */
    NULL, /* SIOCWFIRSTPRIV+11 */                                  /* getscanresults */
    NULL, /* SIOCWFIRSTPRIV+12 */                                  /* delmac */
    NULL, /* SIOCWFIRSTPRIV+13 */                                  /* getchanlist */
    NULL, /* SIOCWFIRSTPRIV+14 */                                  /* setchanlist */
    NULL, /* SIOCWFIRSTPRIV+15 */                                  /* setmac */
    NULL, /* SIOCWFIRSTPRIV+16 */                                  /* chanswitch */
    NULL, /* SIOCWFIRSTPRIV+17 */                                  /* 获取模式, 例: iwpriv vapN get_mode */
    NULL, /* SIOCWFIRSTPRIV+18 */                                  /* 设置模式, 例: iwpriv vapN mode 11g */
    NULL, /* SIOCWFIRSTPRIV+19 */                                  /* getappiebuf */
    NULL, /* SIOCWFIRSTPRIV+20 */                                  /* null */
    (oal_iw_handler)wal_ioctl_get_assoc_list, /* SIOCWFIRSTPRIV+21 */      /* APUT取得关联STA列表 */
    (oal_iw_handler)wal_ioctl_set_mac_filters, /* SIOCWFIRSTPRIV+22 */     /* APUT设置STA过滤 */
    (oal_iw_handler)wal_ioctl_set_ap_config, /* SIOCWFIRSTPRIV+23 */       /* 设置APUT参数 */
    (oal_iw_handler)wal_ioctl_set_ap_sta_disassoc, /* SIOCWFIRSTPRIV+24 */ /* APUT去关联STA */
    NULL, /* SIOCWFIRSTPRIV+25 */                                  /* getStatistics */
    NULL, /* SIOCWFIRSTPRIV+26 */                                  /* sendmgmt */
    NULL, /* SIOCWFIRSTPRIV+27 */                                  /* null */
    NULL, /* SIOCWFIRSTPRIV+28 */                                  /* null */
    NULL, /* SIOCWFIRSTPRIV+29 */                                  /* null */
    NULL, /* SIOCWFIRSTPRIV+30 */                                  /* sethbrparams */

    NULL,
    /* SIOCWFIRSTPRIV+31 */ /* setrxtimeout */
};

/* 无线配置iw_handler_def定义 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_iw_handler_def_stru g_st_iw_handler_def = {
    .standard = g_ast_iw_handlers,
    .num_standard = oal_array_size(g_ast_iw_handlers),
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 59))
#ifdef CONFIG_WEXT_PRIV
    .private = g_ast_iw_priv_handlers,
    .num_private = oal_array_size(g_ast_iw_priv_handlers),
    .private_args = g_ast_iw_priv_args,
    .num_private_args = oal_array_size(g_ast_iw_priv_args),
#endif
#else
    .private = g_ast_iw_priv_handlers,
    .num_private = oal_array_size(g_ast_iw_priv_handlers),
    .private_args = g_ast_iw_priv_args,
    .num_private_args = oal_array_size(g_ast_iw_priv_args),
#endif
    .get_wireless_stats = NULL
};

#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
oal_iw_handler_def_stru g_st_iw_handler_def = {
    NULL, /* 标准ioctl handler */
    0,
    oal_array_size(g_ast_iw_priv_handlers),
    { 0, 0 }, /* 字节对齐 */
    oal_array_size(g_ast_iw_priv_args),
    g_ast_iw_priv_handlers, /* 私有ioctl handler */
    g_ast_iw_priv_args,
    NULL
};
#endif

/* 协议模式字符串定义 */
const wal_ioctl_mode_map_stru g_ast_mode_map[] = {
    /* legacy */
    { "11a",  WLAN_LEGACY_11A_MODE,    WLAN_BAND_5G, WLAN_BAND_WIDTH_20M },
    { "11b",  WLAN_LEGACY_11B_MODE,    WLAN_BAND_2G, WLAN_BAND_WIDTH_20M },
    { "11bg", WLAN_MIXED_ONE_11G_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_20M },
    { "11g",  WLAN_MIXED_TWO_11G_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_20M },

    /* 11n */
    { "11na20",      WLAN_HT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_20M },
    { "11ng20",      WLAN_HT_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_20M },
    { "11na40plus",  WLAN_HT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_40PLUS },
    { "11na40minus", WLAN_HT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_40MINUS },
    { "11ng40plus",  WLAN_HT_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_40PLUS },
    { "11ng40minus", WLAN_HT_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_40MINUS },

    /* 11ac */
    { "11ac20",           WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_20M },
    { "11ac40plus",       WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_40PLUS },
    { "11ac40minus",      WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_40MINUS },
    { "11ac80plusplus",   WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_80PLUSPLUS },
    { "11ac80plusminus",  WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_80PLUSMINUS },
    { "11ac80minusplus",  WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_80MINUSPLUS },
    { "11ac80minusminus", WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_80MINUSMINUS },
#ifdef _PRE_WLAN_FEATURE_160M
    { "11ac160plusplusplus",    WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160PLUSPLUSPLUS },
    { "11ac160plusplusminus",   WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160PLUSPLUSMINUS },
    { "11ac160plusminusplus",   WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160PLUSMINUSPLUS },
    { "11ac160plusminusminus",  WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160PLUSMINUSMINUS },
    { "11ac160minusplusplus",   WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160MINUSPLUSPLUS },
    { "11ac160minusplusminus",  WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160MINUSPLUSMINUS },
    { "11ac160minusminusplus",  WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160MINUSMINUSPLUS },
    { "11ac160minusminusminus", WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160MINUSMINUSMINUS },
#endif

    { "11ac2g20",      WLAN_VHT_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_20M },
    { "11ac2g40plus",  WLAN_VHT_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_40PLUS },
    { "11ac2g40minus", WLAN_VHT_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_40MINUS },
    /* 11n only and 11ac only, 都是20M带宽 */
    { "11nonly2g", WLAN_HT_ONLY_MODE,  WLAN_BAND_2G, WLAN_BAND_WIDTH_20M },
    { "11nonly5g", WLAN_HT_ONLY_MODE,  WLAN_BAND_5G, WLAN_BAND_WIDTH_20M },
    { "11aconly",  WLAN_VHT_ONLY_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_20M },

#ifdef _PRE_WLAN_FEATURE_11AX
    /* 11ax */
    { "11ax2g20",           WLAN_HE_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_20M },
    { "11ax2g40plus",       WLAN_HE_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_40PLUS },
    { "11ax2g40minus",      WLAN_HE_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_40MINUS },
    { "11ax5g20",           WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_20M },
    { "11ax5g40plus",       WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_40PLUS },
    { "11ax5g40minus",      WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_40MINUS },
    { "11ax5g80plusplus",   WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_80PLUSPLUS },
    { "11ax5g80plusminus",  WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_80PLUSMINUS },
    { "11ax5g80minusplus",  WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_80MINUSPLUS },
    { "11ax5g80minusminus", WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_80MINUSMINUS },

#ifdef _PRE_WLAN_FEATURE_160M
    { "11ax5g160plusplusplus",    WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160PLUSPLUSPLUS },
    { "11ax5g160plusplusminus",   WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160PLUSPLUSMINUS },
    { "11ax5g160plusminusplus",   WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160PLUSMINUSPLUS },
    { "11ax5g160plusminusminus",  WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160PLUSMINUSMINUS },
    { "11ax5g160minusplusplus",   WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160MINUSPLUSPLUS },
    { "11ax5g160minusplusminus",  WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160MINUSPLUSMINUS },
    { "11ax5g160minusminusplus",  WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160MINUSMINUSPLUS },
    { "11ax5g160minusminusminus", WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160MINUSMINUSMINUS },
#endif
#endif

    { NULL }
};

/* 注意! 这里的参数定义需要与 g_dmac_config_set_dscr_param中的函数顺序严格一致! */
const int8_t *g_pauc_tx_dscr_param_name[WAL_DSCR_PARAM_BUTT] = {
    "pgl",
    "mtpgl",
    "ta",
    "ra",
    "cc",
    "power",
    "shortgi",
    "preamble",
    "rtscts",
    "lsigtxop",
    "smooth",
    "snding",
    "txbf",
    "stbc",
    "rd_ess",
    "dyn_bw",
    "dyn_bw_exist",
    "ch_bw_exist",
    "rate",
    "mcs",
    "mcsac",
    "mcsax",
    "mcsax_er",
    "nss",
    "bw",
    "ltf",
    "gi",
    "txchain",
    "dcm",
    "protocol_mode"
};

const int8_t *g_pauc_tx_pow_param_name[WAL_TX_POW_PARAM_BUTT] = {
    "rf_reg_ctl",
    "fix_level",
    "mag_level",
    "ctl_level",
    "amend",
    "no_margin",
    "show_log",
    "sar_level",
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    "tas_pwr_ctrl",
    "tas_rssi_measure",
    "tas_ant_switch",
#endif
    "show_tpc_tbl_gain",
    "pow_save",
#ifdef _PRE_WLAN_FEATURE_FULL_QUAN_PROD_CAL
    "pdinfo",
#endif
    "tpc_idx",
};

const int8_t *g_ru_size_tbl[WLAN_HE_RU_SIZE_BUTT] = {
    "26tone",
    "52tone",
    "106tone",
    "242tone",
    "484tone",
    "996tone",
    "1992tone"
};

const int8_t *g_band_tbl[] = {
    "2g",
    "5g",
    "6g",
    NULL,
};

OAL_STATIC const int8_t *g_pauc_tx_dscr_nss_tbl[WLAN_NSS_LIMIT] = {
    "1",
    "2",
    "3",
    "4"
};

const int8_t *g_pauc_bw_tbl[WLAN_BANDWITH_CAP_BUTT] = {
    "20",
    "40",
    "d40",
    "80",
    "d80",
    "160",
    "d160",
    "80_80",
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
    "242tone",
    "106tone",
#endif
};

OAL_STATIC const int8_t *g_pauc_non_ht_rate_tbl[WLAN_LEGACY_RATE_VALUE_BUTT] = {
    "1",
    "2",
    "5.5",
    "11",
    "rsv0",
    "rsv1",
    "rsv2",
    "rsv3",
    "48",
    "24",
    "12",
    "6",
    "54",
    "36",
    "18",
    "9"
};

const wal_ioctl_dyn_cali_stru g_ast_dyn_cali_cfg_map[] = {
    { "realtime_cali_adjust", MAC_DYN_CALI_CFG_SET_EN_REALTIME_CALI_ADJUST },
    { "2g_dscr_interval",     MAC_DYN_CALI_CFG_SET_2G_DSCR_INT },
    { "5g_dscr_interval",     MAC_DYN_CALI_CFG_SET_5G_DSCR_INT },
    { "chain_interval",       MAC_DYN_CALI_CFG_SET_CHAIN_INT },
    { "pdet_min_th",          MAC_DYN_CALI_CFG_SET_PDET_MIN_TH },
    { "pdet_max_th",          MAC_DYN_CALI_CFG_SET_PDET_MAX_TH },
    { NULL }
};

const wal_ioctl_alg_cfg_stru g_ast_alg_cfg_map[] = {
    { "sch_method",         MAC_ALG_CFG_SCHEDULE_SCH_METHOD },
    { "sch_fix_mode",       MAC_ALG_CFG_SCHEDULE_FIX_SCH_MODE },

    { "txbf_txmode_enb",  MAC_ALG_CFG_TXBF_TXMODE_ENABLE },
    { "txbf_2g_bfer",     MAC_ALG_CFG_TXBF_2G_BFER_ENABLE },
    { "txbf_2nss_bfer",   MAC_ALG_CFG_TXBF_2NSS_BFER_ENABLE },
    { "txbf_fix_mode",    MAC_ALG_CFG_TXBF_FIX_MODE },
    { "txbf_fix_sound",   MAC_ALG_CFG_TXBF_FIX_SOUNDING },
    { "txbf_probe_int",   MAC_ALG_CFG_TXBF_PROBE_INT },
    { "txbf_rm_worst",    MAC_ALG_CFG_TXBF_REMOVE_WORST },
    { "txbf_stbl_num",    MAC_ALG_CFG_TXBF_STABLE_NUM },
    { "txbf_probe_cnt",   MAC_ALG_CFG_TXBF_PROBE_COUNT },

    /* 开启或关闭速率自适应算法: sh hipriv.sh "vap0 alg_cfg ar_enable [1|0]" */
    { "ar_enable",            MAC_ALG_CFG_AUTORATE_ENABLE },
    /* 设置rts模式:sh hipriv.sh "vap0 alg_cfg ar_rts_mode
       [0(都不开)|1(都开)|2(rate[0]动态RTS, rate[1..3]都开RTS)|3(rate[0]不开RTS, rate[1..3]都开RTS)]" */
    { "ar_rts_mode",          MAC_ALG_CFG_AUTORATE_RTS_MODE },
    /* 开始速率统计日志:sh hipriv.sh "vap0 alg_cfg ar_stat_log_do [mac地址] [业务类别] [包数目]" */
    /* 打印速率集合:sh hipriv.sh "vap0 alg_cfg ar_disp_rateset 06:31:04:E3:81:02" */
    { "ar_disp_rateset",      MAC_ALG_CFG_AUTORATE_DISPLAY_RATE_SET },
    /* 开启或关闭速率自适应日志: sh hipriv.sh "vap0 alg_cfg ar_log_enable [1|0]" */
    { "ar_log_enable",        MAC_ALG_CFG_AUTORATE_LOG_ENABLE },
    /* 设置最大的VO速率: sh hipriv.sh "vap0 alg_cfg ar_max_vo_rate [速率值]" */
    { "ar_max_vo_rate",       MAC_ALG_CFG_AUTORATE_VO_RATE_LIMIT },
    /* 设置深衰弱的per门限值: sh hipriv.sh "vap0 alg_cfg ar_fading_per_th [per门限值(千分数)]" */
    { "ar_fading_per_th",     MAC_ALG_CFG_AUTORATE_JUDGE_FADING_PER_TH },
    /* 设置聚合自适应开关: sh hipriv.sh "vap0 alg_cfg ar_aggr_opt [1|0]" */
    { "ar_aggr_opt",          MAC_ALG_CFG_AUTORATE_AGGR_OPT },
    /* 设置最大聚合数目: sh hipriv.sh "vap0 alg_cfg ar_max_aggr_mode [聚合数目]" */
    { "ar_aggr_mode",         MAC_ALG_CFG_AUTORATE_AGGR_MODE },
    /* 设置最大聚合数目: sh hipriv.sh "vap0 alg_cfg ar_max_aggr_time_idx [聚合数目]" */
    { "max_aggr_time_idx",    MAC_ALG_CFG_AUTORATE_AGGR_TIME_IDX },
    /* 设置聚合自适应探测间隔: sh hipriv.sh "vap0 alg_cfg ar_aggr_pb_intvl [探测间隔]" */
    { "ar_aggr_pb_intvl",     MAC_ALG_CFG_AUTORATE_AGGR_PROBE_INTVL_NUM },
    /* 设置聚合自适应统计移位值: sh hipriv.sh "vap0 alg_cfg ar_aggr_st_shift [统计移位值]" */
    { "ar_aggr_st_shift",     MAC_ALG_CFG_AUTORATE_AGGR_STAT_SHIFT },
    /* 设置DBAC模式下的最大聚合时间: sh hipriv.sh "vap0 alg_cfg ar_dbac_aggrtime [最大聚合时间(us)]" */
    { "ar_dbac_aggrtime",     MAC_ALG_CFG_AUTORATE_DBAC_AGGR_TIME },
    /* 设置调试用的VI状态: sh hipriv.sh "vap0 alg_cfg ar_dbg_vi_status [0/1/2]" */
    { "ar_dbg_vi_status",     MAC_ALG_CFG_AUTORATE_DBG_VI_STATUS },
    /* 聚合自适应log开关: sh hipriv.sh "vap0 alg_cfg ar_dbg_aggr_log [0/1]" */
    { "ar_dbg_aggr_log",      MAC_ALG_CFG_AUTORATE_DBG_AGGR_LOG },
    /* 最优速率变化时不进行聚合探测的报文数: sh hipriv.sh "vap0 alg_cfg ar_aggr_pck_num [报文数]" */
    { "ar_aggr_pck_num",      MAC_ALG_CFG_AUTORATE_AGGR_NON_PROBE_PCK_NUM },
    /* 最小聚合时间索引: sh hipriv.sh "vap0 alg_cfg ar_aggr_min_idx [索引值]" */
    { "ar_min_aggr_idx",      MAC_ALG_CFG_AUTORATE_AGGR_MIN_AGGR_TIME_IDX },
    /* 设置聚合250us向上的deltaPER门限: sh hipriv.sh "vap0 alg_cfg ar_250us_dper_th [门限值]" */
    { "ar_250us_dper_th",     MAC_ALG_CFG_AUTORATE_AGGR_250US_DELTA_PER_TH },
    /* 设置最大聚合数目: sh hipriv.sh "vap0 alg_cfg ar_max_aggr_num [聚合数目]" */
    { "ar_max_aggr_num",      MAC_ALG_CFG_AUTORATE_MAX_AGGR_NUM },
    /* 设置最低阶MCS限制聚合为1的PER门限: sh hipriv.sh "vap0 alg_cfg ar_1mpdu_per_th [per门限值(千分数)]" */
    { "ar_1mpdu_per_th",      MAC_ALG_CFG_AUTORATE_LIMIT_1MPDU_PER_TH },
    /* 开启或关闭共存探测机制: sh hipriv.sh "vap0 alg_cfg ar_btcoxe_probe [1|0]" */
    { "ar_btcoxe_probe", MAC_ALG_CFG_AUTORATE_BTCOEX_PROBE_ENABLE },
    /* 开启或关闭共存聚合机制: sh hipriv.sh "vap0 alg_cfg ar_btcoxe_aggr [1|0]" */
    { "ar_btcoxe_aggr",  MAC_ALG_CFG_AUTORATE_BTCOEX_AGGR_ENABLE },
    /* 设置共存统计时间间隔参数: sh hipriv.sh "vap0 alg_cfg ar_coxe_intvl [统计周期ms]" */
    { "ar_coxe_intvl",   MAC_ALG_CFG_AUTORATE_COEX_STAT_INTVL },
    /* 设置共存abort低比例门限参数: sh hipriv.sh "vap0 alg_cfg ar_coxe_low_th [千分数]" */
    { "ar_coxe_low_th",  MAC_ALG_CFG_AUTORATE_COEX_LOW_ABORT_TH },
    /* 设置共存abort高比例门限参数: sh hipriv.sh "vap0 alg_cfg ar_coxe_high_th [千分数]" */
    { "ar_coxe_high_th", MAC_ALG_CFG_AUTORATE_COEX_HIGH_ABORT_TH },
    /* 设置共存聚合数目为1的门限参数: sh hipriv.sh "vap0 alg_cfg ar_coxe_agrr_th [千分数]" */
    { "ar_coxe_agrr_th", MAC_ALG_CFG_AUTORATE_COEX_AGRR_NUM_ONE_TH },
    /* 动态带宽特性使能开关: sh hipriv.sh "vap0 alg_cfg ar_dyn_bw_en [0/1]" */
    { "ar_dyn_bw_en",          MAC_ALG_CFG_AUTORATE_DYNAMIC_BW_ENABLE },
    /* 吞吐量波动优化开关: sh hipriv.sh "vap0 alg_cfg ar_thpt_wave_opt [0/1]" */
    { "ar_thpt_wave_opt",      MAC_ALG_CFG_AUTORATE_THRPT_WAVE_OPT },
    /* 设置判断吞吐量波动的goodput差异比例门限(千分数):
       sh hipriv.sh "vap0 alg_cfg ar_gdpt_diff_th [goodput相差比例门限(千分数)]" */
    { "ar_gdpt_diff_th",       MAC_ALG_CFG_AUTORATE_GOODPUT_DIFF_TH },
    /* 设置判断吞吐量波动的PER变差的门限(千分数): sh hipriv.sh "vap0 alg_cfg ar_per_worse_th [PER变差门限(千分数)]" */
    { "ar_per_worse_th",       MAC_ALG_CFG_AUTORATE_PER_WORSE_TH },
    /* 设置发RTS收到CTS但发DATA都不回BA的发送完成中断次数门限: sh hipriv.sh "vap0 alg_cfg ar_cts_no_ba_num [次数]" */
    { "ar_cts_no_ack_num",     MAC_ALG_CFG_AUTORATE_RX_CTS_NO_BA_NUM },
    /* 设置是否支持voice业务聚合: sh hipriv.sh "vap0 alg_cfg ar_vo_aggr [0/1]" */
    { "ar_vo_aggr",            MAL_ALG_CFG_AUTORATE_VOICE_AGGR },
    /* 设置快速平滑统计的平滑因子偏移量: sh hipriv.sh "vap0 alg_cfg ar_fast_smth_shft [偏移量]"(255表示取消快速平滑) */
    { "ar_fast_smth_shft",     MAC_ALG_CFG_AUTORATE_FAST_SMOOTH_SHIFT },
    /* 设置快速平滑统计的最小聚合数目门限: sh hipriv.sh "vap0 alg_cfg ar_fast_smth_aggr_num [最小聚合数目]" */
    { "ar_fast_smth_aggr_num", MAC_ALG_CFG_AUTORATE_FAST_SMOOTH_AGGR_NUM },
    /* 设置short GI惩罚的PER门限值(千分数): sh hipriv.sh "vap0 alg_cfg ar_sgi_punish_per [PER门限值(千分数)]" */
    { "ar_sgi_punish_per",     MAC_ALG_CFG_AUTORATE_SGI_PUNISH_PER },
    /* 设置short GI惩罚的等待探测数目: sh hipriv.sh "vap0 alg_cfg ar_sgi_punish_num [等待探测数目]" */
    { "ar_sgi_punish_num",     MAC_ALG_CFG_AUTORATE_SGI_PUNISH_NUM },
    /* 设置接收通道AGC优化的弱信号RSSI门限: sh hipriv.sh "vap0 alg_cfg ar_weak_rssi_th [RSSI绝对值]"
       (例如: RSSI为-90dBm, 则参数值为90) */
    { "ar_weak_rssi_th",     MAC_ALG_CFG_AUTORATE_WEAK_RSSI_TH },
    /* 设置每个速率等级的最大传输次数: sh hipriv.sh "vap0 alg_cfg ar_max_tx_cnt [传输次数]" */
    { "ar_max_tx_cnt",       MAC_ALG_CFG_AUTORATE_MAX_TX_COUNT },
    { "ar_collision_det",    MAC_ALG_CFG_AUTORATE_COLLISION_DET_EN },
    { "ar_switch_11b",       MAC_ALG_CFG_AUTORATE_SWITCH_11B },
    { "ar_rom_nss_auth",     MAC_ALG_CFG_AUTORATE_ROM_NSS_AUTH },

    /* 弱干扰免疫中non-direct使能: sh hipriv.sh "vap0 alg_cfg anti_inf_imm_en 0|1" */
    { "anti_inf_imm_en",       MAC_ALG_CFG_ANTI_INTF_IMM_ENABLE },
    /* 弱干扰免疫中dynamic unlock使能: sh hipriv.sh "vap0 alg_cfg anti_inf_unlock_en 0|1" */
    { "anti_inf_unlock_en",    MAC_ALG_CFG_ANTI_INTF_UNLOCK_ENABLE },
    /* 弱干扰免疫中rssi统计周期: sh hipriv.sh "vap0 anti_inf_stat_time [time]" */
    { "anti_inf_stat_time",    MAC_ALG_CFG_ANTI_INTF_RSSI_STAT_CYCLE },
    /* 弱干扰免疫中unlock关闭周期: sh hipriv.sh "vap0 anti_inf_off_time [time]" */
    { "anti_inf_off_time",     MAC_ALG_CFG_ANTI_INTF_UNLOCK_CYCLE },
    /* 弱干扰免疫中unlock关闭持续时间: sh hipriv.sh "vap0 anti_inf_off_dur [time]" */
    { "anti_inf_off_dur",      MAC_ALG_CFG_ANTI_INTF_UNLOCK_DUR_TIME },
    /* 抗干扰nav免疫使能: sh hipriv.sh "vap0 alg_cfg anti_inf_nav_en 0|1" */
    { "anti_inf_nav_en",       MAC_ALG_CFG_ANTI_INTF_NAV_IMM_ENABLE },
    /* 弱干扰免疫goodput下降门限: sh hipriv.sh "vap0 alg_cfg anti_inf_gd_th [num]" */
    { "anti_inf_gd_th",        MAC_ALG_CFG_ANTI_INTF_GOODPUT_FALL_TH },
    /* 弱干扰免疫探测保持最大周期数: sh hipriv.sh "vap0 alg_cfg anti_inf_keep_max [num]" */
    { "anti_inf_keep_max",     MAC_ALG_CFG_ANTI_INTF_KEEP_CYC_MAX_NUM },
    /* 弱干扰免疫探测保持最大周期数: sh hipriv.sh "vap0 alg_cfg anti_inf_keep_min [num]" */
    { "anti_inf_keep_min",     MAC_ALG_CFG_ANTI_INTF_KEEP_CYC_MIN_NUM },
    /* 弱干扰免疫是否使能tx time探测: sh hipriv.sh "vap0 anti_inf_tx_pro_en 0|1" */
    { "anti_inf_per_pro_en",   MAC_ALG_CFG_ANTI_INTF_PER_PROBE_EN },
    /* tx time下降门限: sh hipriv.sh "vap0 alg_cfg anti_inf_txtime_th [val]" */
    { "anti_inf_txtime_th",    MAC_ALG_CFG_ANTI_INTF_TX_TIME_FALL_TH },
    /* per下降门限: sh hipriv.sh "vap0 alg_cfg anti_inf_per_th [val]" */
    { "anti_inf_per_th",       MAC_ALG_CFG_ANTI_INTF_PER_FALL_TH },
    /* goodput抖动门限: sh hipriv.sh "vap0 alg_cfg anti_inf_gd_jitter_th [val]" */
    { "anti_inf_gd_jitter_th", MAC_ALG_CFG_ANTI_INTF_GOODPUT_JITTER_TH },
    /* 弱干扰免疫debug的打印信息: sh hipriv.sh "vap0 alg_cfg anti_inf_debug_mode 0|1|2" */
    { "anti_inf_debug_mode",   MAC_ALG_CFG_ANTI_INTF_DEBUG_MODE },
    /* 设置干扰检测周期(ms):sh hipriv.sh "vap0 alg_cfg intf_det_cycle [val]" */
    { "intf_det_cycle",          MAC_ALG_CFG_INTF_DET_CYCLE },
    /* 设置干扰检测模式(同频开/邻频叠频开/都开):sh hipriv.sh "vap0 alg_cfg intf_det_mode 0|1" */
    { "intf_det_mode",           MAC_ALG_CFG_INTF_DET_MODE },
    /* 设置干扰检测debug模式(每个bit表示一类):sh hipriv.sh "vap0 alg_cfg intf_det_debug 0|1" */
    { "intf_det_debug",          MAC_ALG_CFG_INTF_DET_DEBUG },
    /* 设置干扰检测sta阈值(千分之x):sh hipriv.sh "vap0 alg_cfg intf_det_cothr_sta [val]" */
    { "intf_det_cothr_sta",      MAC_ALG_CFG_INTF_DET_COCH_THR_STA },
    /* 设置ACI干扰检测高门限:sh hipriv.sh "vap0 alg_cfg aci_high_th [val]" */
    { "aci_high_th",      MAC_ALG_CFG_INTF_DET_ACI_HIGH_TH },
    /* 设置ACI干扰检测高门限:sh hipriv.sh "vap0 alg_cfg aci_low_th [val]" */
    { "aci_low_th",      MAC_ALG_CFG_INTF_DET_ACI_LOW_TH },
    { "aci_sync_ratio_th",      MAC_ALG_CFG_INTF_DET_ACI_SYNC_TH },
    /* 设置干扰检测sta无干扰阈值(千分之x):sh hipriv.sh "vap0 alg_cfg intf_det_nointf_thr_sta [val]" */
    { "intf_det_nointf_thr_sta", MAC_ALG_CFG_INTF_DET_COCH_NOINTF_STA },
    /* 设置干扰检测ap udp阈值(千分之x):sh hipriv.sh "vap0 alg_cfg intf_det_cothr_udp [val]" */
    { "intf_det_cothr_udp",      MAC_ALG_CFG_INTF_DET_COCH_THR_UDP },
    /* 设置干扰检测ap tcp阈值(千分之x):sh hipriv.sh "vap0 alg_cfg intf_det_cothr_tcp [val]" */
    { "intf_det_cothr_tcp",      MAC_ALG_CFG_INTF_DET_COCH_THR_TCP },
    /* 设置干扰检测邻频干扰扫描周期:sh hipriv.sh "vap0 alg_cfg intf_det_adjscan_cyc [val]" */
    { "intf_det_adjscan_cyc",    MAC_ALG_CFG_INTF_DET_ADJCH_SCAN_CYC },
    /* 设置干扰检测邻频叠频干扰繁忙度阈值(千分之x):sh hipriv.sh "vap0 alg_cfg intf_det_adjratio_thr [val]" */
    { "intf_det_adjratio_thr",   MAC_ALG_CFG_INTF_DET_ADJRATIO_THR },
    /* 设置干扰检测邻频叠频干扰sync error阈值(千分之x):sh hipriv.sh "vap0 alg_cfg intf_det_sync_th [val]" */
    { "intf_det_sync_th",        MAC_ALG_CFG_INTF_DET_SYNC_THR },
    /* 设置干扰检测零频叠频干扰平均rssi阈值(千分之x):sh hipriv.sh "vap0 alg_cfg intf_det_ave_rssi [val]" */
    { "intf_det_ave_rssi",       MAC_ALG_CFG_INTF_DET_AVE_RSSI },
    /* 设置干扰检测非邻/叠频干扰繁忙度阈值(千分之x):sh hipriv.sh "vap0 alg_cfg intf_det_no_adjratio_th [val]" */
    { "intf_det_no_adjratio_th", MAC_ALG_CFG_INTF_DET_NO_ADJRATIO_TH },
    /* 设置干扰检测非邻/叠频干扰计数阈值:sh hipriv.sh "vap0 alg_cfg intf_det_no_adjcyc_th [val]" */
    { "intf_det_no_adjcyc_th",   MAC_ALG_CFG_INTF_DET_NO_ADJCYC_TH },
    /* 设置干扰检测非邻/叠频干扰计数阈值:sh hipriv.sh "vap0 alg_cfg intf_det_collision_th [val] */
    { "intf_det_collision_th",   MAC_ALG_CFG_INTF_DET_COLLISION_TH },
    /* 设置检测到负增益后不探测阈值:sh hipriv.sh "vap0 alg_cfg neg_det_noprobe_th [val]" */
    { "neg_det_noprobe_th",      MAC_ALG_CFG_NEG_DET_NONPROBE_TH },
    /* 打印统计日志:sh hipriv.sh "vap0 alg_intf_det_log intf_det_stat_log_out" */
    { "intf_det_stat_log_out",   MAC_ALG_CFG_INTF_DET_STAT_LOG_WRITE },
    /* ap模式下edca优化使能模式: sh hipriv.sh "vap0 alg_cfg edca_opt_en_ap 0|1|2" */
    { "edca_opt_en_ap",          MAC_ALG_CFG_EDCA_OPT_AP_EN_MODE },
    /* sta模式下edca优化使能模式: sh hipriv.sh "vap0 alg_cfg edca_opt_en_sta 0|1" */
    { "edca_opt_en_sta",         MAC_ALG_CFG_EDCA_OPT_STA_EN },
    /* sta模式下edca txop limit优化使能模式: sh hipriv.sh "vap0 alg_cfg txop_limit_en_sta 0|1" */
    { "txop_limit_en_sta",       MAC_ALG_CFG_TXOP_LIMIT_STA_EN },
    /* sta模式下edca优化的weighting系数: sh hipriv.sh "vap0 alg_cfg edca_opt_sta_weight 0~3" */
    /* CCA优化功能使能: sh hipriv.sh "vap0 alg_cfg cca_opt_alg_en_mode 0|1" */
    { "cca_opt_alg_en_mode",      MAC_ALG_CFG_CCA_OPT_ALG_EN_MODE },
    /* CCA优化DEBUG模式启动: sh hipriv.sh "vap0 alg_cfg cca_opt_debug_mode 0|1" */
    { "cca_opt_debug_mode",       MAC_ALG_CFG_CCA_OPT_DEBUG_MODE },
    /* CCA优化信道扫描的时间(ms):sh hipriv.sh "vap0 alg_cfg cca_opt_set_sync_err_th [time]" */
    { "cca_opt_set_cca_th_debug", MAC_ALG_CFG_CCA_OPT_SET_CCA_TH_DEBUG },
    /* CCA log开关 sh hipriv.sh "vap0 alg_cfg cca_opt_log 0|1" */
    { "cca_opt_log",            MAC_ALG_CFG_CCA_OPT_LOG },
    /* tpc相关命令sh hipriv.sh "wlan0 alg_cfg tpc_level 0|1|2|3|4[TPC档位] */
    { "tpc_mode",               MAC_ALG_CFG_TPC_MODE },              /* 设置TPC工作模式 */
    { "tpc_dbg",                MAC_ALG_CFG_TPC_DEBUG },             /* 设置TPC的debug开关 */
    { "tpc_level",              MAC_ALG_CFG_TPC_LEVEL },             /* 设置TPC的level档位 */
    /* 设置TPC的log开关:sh hipriv.sh "vap0 alg_cfg tpc_log 1 */
    { "tpc_log",                MAC_ALG_CFG_TPC_LOG },
    /* 获取特殊帧功率:sh hipriv.sh "vap0 alg_tpc_log tpc_get_frame_pow beacon_pow" */
    { "tpc_get_frame_pow",      MAC_ALG_CFG_TPC_GET_FRAME_POW },
    { "tpc_over_temp_th",       MAC_ALG_CFG_TPC_OVER_TMP_TH },       /* TPC过温门限 */
    { "tpc_dpd_enable_rate",    MAC_ALG_CFG_TPC_DPD_ENABLE_RATE },   /* 配置DPD生效的速率INDEX */
    { "tpc_target_rate_11b",    MAC_ALG_CFG_TPC_TARGET_RATE_11B },   /* 11b目标速率设置 */
    { "tpc_target_rate_11ag",   MAC_ALG_CFG_TPC_TARGET_RATE_11AG },  /* 11ag目标速率设置 */
    { "tpc_target_rate_11n20",  MAC_ALG_CFG_TPC_TARGET_RATE_HT40 },  /* 11n20目标速率设置 */
    { "tpc_target_rate_11n40",  MAC_ALG_CFG_TPC_TARGET_RATE_HT40 },  /* 11n40目标速率设置 */
    { "tpc_target_rate_11ac20", MAC_ALG_CFG_TPC_TARGET_RATE_VHT20 }, /* 11ac20目标速率设置 */
    { "tpc_target_rate_11ac40", MAC_ALG_CFG_TPC_TARGET_RATE_VHT40 }, /* 11ac40目标速率设置 */
    { "tpc_target_rate_11ac80", MAC_ALG_CFG_TPC_TARGET_RATE_VHT80 }, /* 11ac80目标速率设置 */

#ifdef _PRE_WLAN_FEATURE_MU_TRAFFIC_CTL
    { "traffic_ctl_enable",       MAC_ALG_CFG_TRAFFIC_CTL_ENABLE },
    { "traffic_ctl_timeout",      MAC_ALG_CFG_TRAFFIC_CTL_TIMEOUT },
    { "traffic_ctl_min_thres",    MAC_ALG_CFG_TRAFFIC_CTL_MIN_THRESHOLD },
    { "traffic_ctl_log_debug",    MAC_ALG_CFG_TRAFFIC_CTL_LOG_DEBUG },
    { "traffic_ctl_buf_thres",    MAC_ALG_CFG_TRAFFIC_CTL_BUF_THRESHOLD },
    { "traffic_ctl_buf_adj_enb",  MAC_ALG_CFG_TRAFFIC_CTL_BUF_ADJ_ENABLE },
    { "traffic_ctl_buf_adj_num",  MAC_ALG_CFG_TRAFFIC_CTL_BUF_ADJ_NUM },
    { "traffic_ctl_buf_adj_cyc",  MAC_ALG_CFG_TRAFFIC_CTL_BUF_ADJ_CYCLE },
    { "traffic_ctl_rx_rst_enb",   MAC_ALG_CFG_TRAFFIC_CTL_RX_RESTORE_ENABLE },
    { "traffic_ctl_rx_rst_num",   MAC_ALG_CFG_TRAFFIC_RX_RESTORE_NUM },
    { "traffic_ctl_rx_rst_thres", MAC_ALG_CFG_TRAFFIC_RX_RESTORE_THRESHOLD },

    { "rx_dscr_ctl_enable", MAC_ALG_CFG_RX_DSCR_CTL_ENABLE },
    { "rx_dscr_ctl_log_debug", MAC_ALG_CFG_RX_DSCR_CTL_LOG_DEBUG },
#endif

#ifdef _PRE_WLAN_FEATURE_HID2D
    {"hid2d_debug_log_enable",        MAC_ALG_CFG_HID2D_DEBUG_ENABLE},  /* Debug 使能开关 */
    {"hid2d_always_rts",              MAC_ALG_CFG_HID2D_RTS_ENABLE},  /* 投屏模式强制开启RTS */
    {"hid2d_disable_1024qam",         MAC_ALG_CFG_HID2D_HIGH_BW_MCS_DISABLE},  /* 投屏模式关闭高性能特性 */
    {"hid2d_set_high_txpower",        MAC_ALG_CFG_HID2D_HIGH_TXPOWER_ENABLE},  /* 投屏模式保持高传输功率 */
#ifdef _PRE_WLAN_FEATURE_HID2D_PRESENTATION
    {"hid2d_set_cca",                 MAC_ALG_CFG_HID2D_SET_APK_CCA_TH}, /* 发布会场景下设置CCA门限 */
#endif
#endif
    {"sr_nonsrg_en",     MAC_ALG_CFG_NON_SRG_ENABLE},
    {"sr_srg_en",        MAC_ALG_CFG_SRG_ENABLE},
    {"sr_log",           MAC_ALG_CFG_SR_LOG_ENABLE},
    {"sr_duty_th",       MAC_ALG_CFG_SR_DUTY_TH},
    { NULL }
};

const wal_ioctl_tlv_stru g_ast_set_tlv_table[] = {
    /* cmd: wlan0 set_tlv xx xx */
    { "tx_pkts_stat", WLAN_CFGID_SET_DEVICE_PKT_STAT },
    { "auto_freq", WLAN_CFGID_SET_DEVICE_FREQ },

    { "set_adc_dac_freq", WLAN_CFGID_SET_ADC_DAC_FREQ }, /* 设置ADC DAC频率 */
    { "set_mac_freq", WLAN_CFGID_SET_MAC_FREQ },         /* 设MAC频率 */

    { "rx_ampdu_num", WLAN_CFGID_SET_ADDBA_RSP_BUFFER },
    { "data_collect", WLAN_CFGID_DATA_COLLECT },    /* 第一个参数为模式，第二个参数为点数 */
    /* cmd: wlan0 set_val xx */
    { "tx_ampdu_type",  WLAN_CFGID_SET_TX_AMPDU_TYPE },  /* 设置聚合类型的开关 */
    { "tx_ampdu_amsdu", WLAN_CFGID_AMSDU_AMPDU_SWITCH }, /* 设置tx amsdu ampdu联合聚合功能的开关 */
    { "rx_ampdu_amsdu", WLAN_CFGID_SET_RX_AMPDU_AMSDU }, /* 设置rx ampdu amsdu 联合聚合功能的开关 */

    { "sk_pacing_shift", WLAN_CFGID_SET_SK_PACING_SHIFT },

    { "trx_stat_log_en", WLAN_CFGID_SET_TRX_STAT_LOG }, /* 设置tcp ack缓存时吞吐量统计维测开关，其他模块可参考 */
    { "mimo_blacklist", WLAN_CFGID_MIMO_BLACKLIST },    /* 设置探测MIMO黑名单机制开关 */
    /* cmd: wlan0 set_val dfs_debug 0|1 */
    { "dfs_debug", WLAN_CFGID_SET_DFS_MODE },           /* 设置dfs是否为检测率模式的开关 */
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
    { "hid2d_debug_info", WLAN_CFGID_HID2D_DEBUG_MODE },           /* 设置dfs是否为检测率模式的开关 */
#endif
    { "warning_mode", WLAN_CFGID_SET_WARNING_MODE },    /* 设置部分WARNING是否为测试模式的开关 */

    { "chr_mode", WLAN_CFGID_SET_CHR_MODE },            /* 设置chr模式的开关 */
    {"linkloss_csa_dis", WLAN_CFGID_SET_LINKLOSS_DISABLE_CSA},    /* 配置linkloss是否禁止csa */
    {"rx_filter_frag", WLAN_CFGID_RX_FILTER_FRAG},    /* 配置rx过滤功能 */
#if defined(_PRE_WLAN_FEATURE_HID2D) && defined(_PRE_WLAN_FEATURE_HID2D_PRESENTATION)
    { "hid2d_presentation", WLAN_CFGID_HID2D_PRESENTATION_MODE }, /* 配置为发布会场景 */
#endif
    { NULL }
};

#ifdef _PRE_WLAN_FEATURE_11AX
const wal_ioctl_tlv_stru g_ast_11ax_debug_table[] = {
    /* cmd: wlan0 xxx  2  xxx 1 xxx 0 */
    { "print_log",        MAC_VAP_11AX_DEBUG_PRINT_LOG },
    { "print_rx_log",     MAC_VAP_11AX_DEBUG_PRINT_RX_LOG },
    { "tid",              MAC_VAP_11AX_DEBUG_HE_TB_PPDU_TID_NUM },
    { "htc_order",        MAC_VAP_11AX_DEBUG_HE_TB_PPDU_HTC_ORGER },
    { "htc_val",          MAC_VAP_11AX_DEBUG_HE_TB_PPDU_HTC_VALUE },
    { "fix_power",        MAC_VAP_11AX_DEBUG_HE_TB_PPDU_FIX_POWER },
    { "power_val",        MAC_VAP_11AX_DEBUG_HE_TB_PPDU_POWER_VALUE },
    { "disable_ba_check", MAC_VAP_11AX_DEBUG_HE_TB_PPDU_DISABLE_BA_CHECK },
    { "disable_mu_edca",  MAC_VAP_11AX_DEBUG_DISABLE_MU_EDCA },
    { "manual_cfo",       MAC_VAP_11AX_DEBUG_MANUAL_CFO },
    { "bsrp",             MAC_VAP_11AX_DEBUG_BSRP_CFG },
    { "bsrp_tid",         MAC_VAP_11AX_DEBUG_BSRP_TID },
    { "bsrp_size",        MAC_VAP_11AX_DEBUG_BSRP_QUEUE_SIZE },
    { "mpad_dur",         MAC_VAP_11AX_DEBUG_MAC_PADDING },
    { "tb_pwr_test",      MAC_VAP_11AX_DEBUG_POW_TEST },
    { "tb_ppdu_len",      MAC_VAP_11AX_DEBUG_TB_PPDU_LEN },
    { "tb_ppdu_ac",       MAC_VAP_11AX_DEBUG_TB_PPDU_AC },
    { "print_rx_trig",    MAC_VAP_11AX_DEBUG_PRINT_RX_TRIG_FRAME },
    { "om_auth_flag",     MAC_VAP_11AX_DEBUG_OM_AUTH_FLAG },
    { "tom_bw",           MAC_VAP_11AX_DEBUG_TOM_BW_FLAG },
    { "tom_nss",          MAC_VAP_11AX_DEBUG_TOM_NSS_FLAG },
    { "tom_ul_mu_disable", MAC_VAP_11AX_DEBUG_TOM_UL_MU_DISABLE_FLAG },
    { "uora_ocw",         MAC_VAP_11AX_DEBUG_UORA_OCW_UPDATE},
    { "uora_obo_bypass",  MAC_VAP_11AX_DEBUG_UORA_OBO_BYPASS},
    { "bss_color",        MAC_VAP_11AX_DEBUG_BSS_COLOR },
    { "tx_q_status",      MAC_VAP_11AX_DEBUG_TX_Q_STATUS},

    { NULL }
};
#endif

/* MAC TX RX common info report命令表格 */
const wal_ioctl_tlv_stru g_ast_mac_tx_report_debug_table[] = {
    /* cmd: sh hipriv.sh "wlan0 set_str mac_report_tx XXX(命令参数个数) index XXX status XXX bw XXX protocol XXX
     *                    fr_type XXX sub_type XXX ampdu XXX psdu XXX hw_retry XXX"
     * MAC TX common命令举例:
     * 1.统计TB QoS NULL: sh hipriv.sh "wlan0 set_str mac_report_tx 4 index 0 protocol 11 fr_type 2 sub_type 12"
     * 2.清除统计寄存器配置信息:sh hipriv.sh "wlan0 set_str mac_report_tx 2 index 0 clear 1"
     * 注意:(1)tx只有index0支持status的配置，其他index不支持 (2)清除统计后重新统计需要写0之后才能正常计数
     *      (3)在一条命令中配置多个选项可能导致命令长度超过限制，可以分次进行配置
     * MAC寄存器CFG_TX_COMMON_CNT_CTRL各bit位配置详见寄存器表单
     */
    { "index",  MAC_TX_COMMON_REPORT_INDEX },            /* tx rx分别有8个 index:0-7 */
    { "status", MAC_TX_COMMON_REPORT_STATUS },           /* 0：发送成功 1：响应帧错误 2：响应帧超时 3：发送异常结束 */
    { "bw",     MAC_TX_COMMON_REPORT_BW_MODE },          /* bandwidth 0:20M 1:40M 2:80M 3:160M */
    /* 0000: The data rate is 11b type                    0001: The data rate is legacy OFDM type
     * 0010: The data rate is HT Mixed mode Frame type    0011: The data rate is HT Green Field Frame type
     * 0100: The data rate is VHT type                    0101~0111：reserved
     * 1000：The data rate is HE  SU Format type          1001：The data rate is HE  MU Format type
     * 1010：The data rate is HE  EXT SU Format type      1011：The data rate is HE  TRIG Format type
     * 1100~1111:reserved
     */
    { "protocol", MAC_TX_COMMON_REPORT_PROTOCOL_MODE },  /* 协议模式 */
    { "fr_type",  MAC_TX_COMMON_REPORT_FRAME_TYPE },     /* 帧类型 */
    { "sub_type", MAC_TX_COMMON_REPORT_SUB_TYPE },       /* 子类型 */
    { "ampdu",    MAC_TX_COMMON_REPORT_APMDU },          /* 0：非ampdu时统计 1：ampdu时统计 */
    { "psdu",     MAC_TX_COMMON_REPORT_PSDU },           /* 按psdu统计还是按mpdu统计：0：按mpdu统计 1：按psdu统计 */
    { "hw_retry", MAC_TX_COMMON_REPORT_HW_RETRY },       /* 0：非hw retry帧时统计 1：hw retry帧时统计 */
    { "clear",    MAC_TX_COMMON_REPORT_CTRL_REG_CLEAR }, /* 清除对应index的寄存器配置 */

    { NULL }
};

const wal_ioctl_tlv_stru g_ast_mac_rx_report_debug_table[] = {
    /* cmd: sh hipriv.sh "wlan0 set_str mac_report_rx 11(命令参数个数) index 5 status 1 bw 2 protocol 1 fr_type 2
     *                     sub_type 4 ampdu 1 psdu 1 vap_mode 3 bss 1 direct 1"
     *  clear命令: sh hipriv.sh "wlan0 set_str mac_report_rx index 5 clear 1"
     *  注意：清除统计后重新统计需要写0之后才能正常计数
     */
    /* 0：Invalid  1：RX successful
     * 2：Duplicate detected 3：FCS check failed
     * 4：Key search failed 5：MIC check failed
     * 6：ICV failed        others：Reserved
     */
    { "index",  MAC_RX_COMMON_REPORT_INDEX },            /* tx rx分别有8个 index:0-7 */
    { "status", MAC_RX_COMMON_REPORT_STATUS },           /* 接收状态 */
    { "bw", MAC_RX_COMMON_REPORT_BW_MODE },              /* bandwidth 0:20M 1:40M 2:80M 3:160M */
    /* 0000: The data rate is 11b type                    0001: The data rate is legacy OFDM type
       0010: The data rate is HT Mixed mode Frame type    0011: The data rate is HT Green Field Frame type
       0100: The data rate is VHT type                    0101~0111：reserved
       1000：The data rate is HE  SU Format type          1001：The data rate is HE  MU Format type
       1010：The data rate is HE  EXT SU Format type      1011：The data rate is HE  TRIG Format type
       1100~1111:reserved */
    { "protocol", MAC_RX_COMMON_REPORT_PROTOCOL_MODE },  /* 协议模式 */
    { "fr_type",  MAC_RX_COMMON_REPORT_FRAME_TYPE },     /* 帧类型 */
    { "sub_type", MAC_RX_COMMON_REPORT_SUB_TYPE },       /* 子类型 */
    { "ampdu",    MAC_RX_COMMON_REPORT_APMDU },          /* 0：非ampdu时统计 1：ampdu时统计 */
    { "psdu",     MAC_RX_COMMON_REPORT_PSDU },           /* 按psdu统计还是按mpdu统计：0：按mpdu统计 1：按psdu统计 */
    { "vap_mode", MAC_RX_COMMON_REPORT_VAP_CHK },        /* 是否按vap统计:0~4：接收到vap0~4计数 5~6:resv 7:不区分vap */
    { "bss",      MAC_RX_COMMON_REPORT_BSS_CHK },        /* 0：非本bss时统计 1：本bss时统计 */
    { "direct",   MAC_RX_COMMON_REPORT_DIRECT_CHK },     /* 0：非direct帧时统计 1：direct帧时统计 */
    { "clear",    MAC_RX_COMMON_REPORT_CTRL_REG_CLEAR }, /* 清除对应index的寄存器配置 */

    { NULL }
};

const wal_ioctl_tlv_stru g_ast_common_debug_table[] = {
    { "pg_switch", PG_EFFICIENCY_STATISTICS_ENABLE },
    { "pg_info", PG_EFFICIENCY_INFO },

    { "hw_txq", MAC_VAP_COMMON_SET_TXQ },
    { "tx_info", MAC_VAP_COMMON_TX_INFO },

#ifdef _PRE_WLAN_FEATURE_MBO
    { "mbo_switch",                  MBO_SWITCH },        /* 测试用例1:MBO特性开关， out-of-the-box test */
    { "mbo_cell_capa",               MBO_CELL_CAP },      /* 测试用例2:MBO Capability Indication test  */
    { "mbo_assoc_disallowed_switch", MBO_ASSOC_DISALLOWED_SWITCH }, /* 测试用例6 */
#endif

#ifdef _PRE_WLAN_FEATURE_HIEX
    { "set_dev_hiex",                 HIEX_DEV_CAP },     /* mac device hiex cap cmd */
    { "hiex_debug_switch",            HIEX_DEBUG}, /* hiex debug */
#endif

    { "rifs_en",                      RIFS_ENABLE },
    { "greenfield_en",                GREENFIELD_ENABLE },
    { "protect_info",                 PROTECTION_LOG_SWITCH},

#ifdef _PRE_WLAN_FEATURE_11AX
    {"rx_frame_cnt",                 DMAC_RX_FRAME_STATISTICS}, /* 接收帧统计 */
    {"tx_frame_cnt",                 DMAC_TX_FRAME_STATISTICS}, /* 发送帧统计 */
    {"su_1xltf_0.8us_gi",            SU_PPDU_1XLTF_08US_GI_SWITCH},
#endif
    {"auth_rsp_timeout",             AUTH_RSP_TIMEOUT},
    {"forbid_open_auth",             FORBIT_OPEN_AUTH},

#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
    {"hal_ps_debug_switch",          HAL_PS_DEBUG_SWITCH},
#endif
    {"ht_self_cure",                 HT_SELF_CURE_DEBUG},
    {"set_bindcpu",                  USERCTL_BINDCPU},
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    {"set_mcast_ampdu_retry",        MCAST_AMPDU_RETRY},
#endif
    { NULL }
};

OAL_STATIC const wal_ioctl_tlv_stru g_ast_debug_log_table[] = {
    { "encap_ampdu", MAC_LOG_ENCAP_AMPDU },
    { "ota_switch", MAC_LOG_SET_ALG_OTA },  /* bit[7:4]:0 close/1 open; bit[3:0]:0 tx/1 rx */
    { "temp_protect_log", MAC_LOG_SET_TEMP_PROTECT_LOG },
    { "himit_probe_enable", MAC_LOG_SET_HIMIT_PROBE_ENABLE },
    { "himit_set_htc", MAC_LOG_SET_HIMIT_HTC },
#ifdef _PRE_WLAN_FEATURE_MONITOR
    { "monitor_ota", MAC_LOG_MONITOR_OTA_RPT },
#endif
    { NULL }
};

/* cmd: wlan0 set_str xxx  2  xxx 1 xxx 0 */
const wal_ioctl_str_stru g_ast_set_str_table[] = {
#ifdef _PRE_WLAN_FEATURE_11AX
    { "11ax_debug", WLAN_CFGID_11AX_DEBUG, (wal_ioctl_tlv_stru *)&g_ast_11ax_debug_table },
#endif

    { "mac_report_tx", WLAN_CFGID_MAC_TX_COMMON_REPORT, (wal_ioctl_tlv_stru *)&g_ast_mac_tx_report_debug_table },
    { "mac_report_rx", WLAN_CFGID_MAC_RX_COMMON_REPORT, (wal_ioctl_tlv_stru *)&g_ast_mac_rx_report_debug_table },
    { "common_debug",  WLAN_CFGID_COMMON_DEBUG,         (wal_ioctl_tlv_stru *)&g_ast_common_debug_table },
    { "debug_log",     WLAN_CFGID_LOG_DEBUG,         (wal_ioctl_tlv_stru *)&g_ast_debug_log_table },

    { NULL }
};

const wal_dfs_domain_entry_stru g_ast_dfs_domain_table[] = {
    { "AE", MAC_DFS_DOMAIN_ETSI },
    { "AL", MAC_DFS_DOMAIN_NULL },
    { "AM", MAC_DFS_DOMAIN_ETSI },
    { "AN", MAC_DFS_DOMAIN_ETSI },
    { "AR", MAC_DFS_DOMAIN_FCC },
    { "AT", MAC_DFS_DOMAIN_ETSI },
    { "AU", MAC_DFS_DOMAIN_FCC },
    { "AZ", MAC_DFS_DOMAIN_ETSI },
    { "BA", MAC_DFS_DOMAIN_ETSI },
    { "BE", MAC_DFS_DOMAIN_ETSI },
    { "BG", MAC_DFS_DOMAIN_ETSI },
    { "BH", MAC_DFS_DOMAIN_ETSI },
    { "BL", MAC_DFS_DOMAIN_NULL },
    { "BN", MAC_DFS_DOMAIN_ETSI },
    { "BO", MAC_DFS_DOMAIN_ETSI },
    { "BR", MAC_DFS_DOMAIN_FCC },
    { "BY", MAC_DFS_DOMAIN_ETSI },
    { "BZ", MAC_DFS_DOMAIN_ETSI },
    { "CA", MAC_DFS_DOMAIN_FCC },
    { "CH", MAC_DFS_DOMAIN_ETSI },
    { "CL", MAC_DFS_DOMAIN_NULL },
    { "CN", MAC_DFS_DOMAIN_CN },
    { "CO", MAC_DFS_DOMAIN_FCC },
    { "CR", MAC_DFS_DOMAIN_FCC },
    { "CS", MAC_DFS_DOMAIN_ETSI },
    { "CY", MAC_DFS_DOMAIN_ETSI },
    { "CZ", MAC_DFS_DOMAIN_ETSI },
    { "DE", MAC_DFS_DOMAIN_ETSI },
    { "DK", MAC_DFS_DOMAIN_ETSI },
    { "DO", MAC_DFS_DOMAIN_FCC },
    { "DZ", MAC_DFS_DOMAIN_NULL },
    { "EC", MAC_DFS_DOMAIN_FCC },
    { "EE", MAC_DFS_DOMAIN_ETSI },
    { "EG", MAC_DFS_DOMAIN_ETSI },
    { "ES", MAC_DFS_DOMAIN_ETSI },
    { "FI", MAC_DFS_DOMAIN_ETSI },
    { "FR", MAC_DFS_DOMAIN_ETSI },
    { "GB", MAC_DFS_DOMAIN_ETSI },
    { "GE", MAC_DFS_DOMAIN_ETSI },
    { "GR", MAC_DFS_DOMAIN_ETSI },
    { "GT", MAC_DFS_DOMAIN_FCC },
    { "HK", MAC_DFS_DOMAIN_FCC },
    { "HN", MAC_DFS_DOMAIN_FCC },
    { "HR", MAC_DFS_DOMAIN_ETSI },
    { "HU", MAC_DFS_DOMAIN_ETSI },
    { "ID", MAC_DFS_DOMAIN_NULL },
    { "IE", MAC_DFS_DOMAIN_ETSI },
    { "IL", MAC_DFS_DOMAIN_ETSI },
    { "IN", MAC_DFS_DOMAIN_NULL },
    { "IQ", MAC_DFS_DOMAIN_NULL },
    { "IR", MAC_DFS_DOMAIN_NULL },
    { "IS", MAC_DFS_DOMAIN_ETSI },
    { "IT", MAC_DFS_DOMAIN_ETSI },
    { "JM", MAC_DFS_DOMAIN_FCC },
    { "JO", MAC_DFS_DOMAIN_ETSI },
    { "JP", MAC_DFS_DOMAIN_MKK },
    { "KP", MAC_DFS_DOMAIN_NULL },
    { "KR", MAC_DFS_DOMAIN_KOREA },
    { "KW", MAC_DFS_DOMAIN_ETSI },
    { "KZ", MAC_DFS_DOMAIN_NULL },
    { "LB", MAC_DFS_DOMAIN_NULL },
    { "LI", MAC_DFS_DOMAIN_ETSI },
    { "LK", MAC_DFS_DOMAIN_FCC },
    { "LT", MAC_DFS_DOMAIN_ETSI },
    { "LU", MAC_DFS_DOMAIN_ETSI },
    { "LV", MAC_DFS_DOMAIN_ETSI },
    { "MA", MAC_DFS_DOMAIN_NULL },
    { "MC", MAC_DFS_DOMAIN_ETSI },
    { "MK", MAC_DFS_DOMAIN_ETSI },
    { "MO", MAC_DFS_DOMAIN_FCC },
    { "MT", MAC_DFS_DOMAIN_ETSI },
    { "MX", MAC_DFS_DOMAIN_FCC },
    { "MY", MAC_DFS_DOMAIN_FCC },
    { "NG", MAC_DFS_DOMAIN_NULL },
    { "NL", MAC_DFS_DOMAIN_ETSI },
    { "NO", MAC_DFS_DOMAIN_ETSI },
    { "NP", MAC_DFS_DOMAIN_NULL },
    { "NZ", MAC_DFS_DOMAIN_FCC },
    { "OM", MAC_DFS_DOMAIN_FCC },
    { "PA", MAC_DFS_DOMAIN_FCC },
    { "PE", MAC_DFS_DOMAIN_FCC },
    { "PG", MAC_DFS_DOMAIN_FCC },
    { "PH", MAC_DFS_DOMAIN_FCC },
    { "PK", MAC_DFS_DOMAIN_NULL },
    { "PL", MAC_DFS_DOMAIN_ETSI },
    { "PR", MAC_DFS_DOMAIN_FCC },
    { "PT", MAC_DFS_DOMAIN_ETSI },
    { "QA", MAC_DFS_DOMAIN_NULL },
    { "RO", MAC_DFS_DOMAIN_ETSI },
    { "RU", MAC_DFS_DOMAIN_FCC },
    { "SA", MAC_DFS_DOMAIN_FCC },
    { "SE", MAC_DFS_DOMAIN_ETSI },
    { "SG", MAC_DFS_DOMAIN_NULL },
    { "SI", MAC_DFS_DOMAIN_ETSI },
    { "SK", MAC_DFS_DOMAIN_ETSI },
    { "SV", MAC_DFS_DOMAIN_FCC },
    { "SY", MAC_DFS_DOMAIN_NULL },
    { "TH", MAC_DFS_DOMAIN_FCC },
    { "TN", MAC_DFS_DOMAIN_ETSI },
    { "TR", MAC_DFS_DOMAIN_ETSI },
    { "TT", MAC_DFS_DOMAIN_FCC },
    { "TW", MAC_DFS_DOMAIN_NULL },
    { "UA", MAC_DFS_DOMAIN_NULL },
    { "US", MAC_DFS_DOMAIN_FCC },
    { "UY", MAC_DFS_DOMAIN_FCC },
    { "UZ", MAC_DFS_DOMAIN_FCC },
    { "VE", MAC_DFS_DOMAIN_FCC },
    { "VN", MAC_DFS_DOMAIN_ETSI },
    { "YE", MAC_DFS_DOMAIN_NULL },
    { "ZA", MAC_DFS_DOMAIN_FCC },
    { "ZW", MAC_DFS_DOMAIN_NULL },
};
/* 解圈复杂度需要，wal_hipriv_process_rate_params建立的2个数组 */
OAL_STATIC int32_t g_al_mcs_min_table[WAL_HIPRIV_MCS_TYPE_NUM] = {
    WAL_HIPRIV_HT_MCS_MIN,
    WAL_HIPRIV_VHT_MCS_MIN,
    WAL_HIPRIV_HE_MCS_MIN,
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
    WAL_HIPRIV_HE_ER_MCS_MIN
#endif
};

OAL_STATIC int32_t g_al_mcs_max_table[WAL_HIPRIV_MCS_TYPE_NUM] = {
    WAL_HIPRIV_HT_MCS_MAX,
    WAL_HIPRIV_VHT_MCS_MAX,
    WAL_HIPRIV_HE_MCS_MAX,
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
    WAL_HIPRIV_HE_ER_MCS_MAX
#endif
};
wal_wlan_cfg_process_info_table_stru g_wlan_cfg_process_info_table[] = {
    /* cfg_id need_host_process   need_w4_host_return   need_dev_process  need_w4_dev_return  process_entry_func */
    WLAN_CFG_INFO_TBL(WLAN_CFGID_ALG_ENTRY, OAL_TRUE, OAL_TRUE, OAL_TRUE, OAL_TRUE, wal_wlan_cfg_alg_process_entry),

    /* OTHER wlan config process_info to register */
};


uint32_t wal_get_cmd_one_arg(int8_t *pc_cmd, int8_t *pc_arg,
    uint32_t arg_len, uint32_t *pul_cmd_offset)
{
    int8_t *pc_cmd_copy = NULL;
    uint32_t pos = 0;

    if (oal_unlikely(oal_any_null_ptr3(pc_cmd, pc_arg, pul_cmd_offset))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_get_cmd_one_arg::pc_cmd/pc_arg/pul_cmd_offset is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pc_cmd_copy = pc_cmd;

    /* 去掉字符串开始的空格 */
    while (*pc_cmd_copy == ' ') {
        ++pc_cmd_copy;
    }

    while ((*pc_cmd_copy != ' ') && (*pc_cmd_copy != '\0')) {
        pc_arg[pos] = *pc_cmd_copy;
        ++pos;
        ++pc_cmd_copy;

        if (oal_unlikely(pos >= arg_len)) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_get_cmd_one_arg::pos>=HIPRIV_CMD_NAME_MAX_LEN, pos:%d}", pos);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }

    pc_arg[pos] = '\0';

    /* 字符串到结尾，返回错误码(命令码之后无) */
    if (pos == 0) {
        oam_info_log0(0, OAM_SF_ANY, "{wal_get_cmd_one_arg::return param pc_arg is null!}");
        return OAL_ERR_CODE_CONFIG_ARGS_OVER;
    }

    *pul_cmd_offset = (uint32_t)(pc_cmd_copy - pc_cmd);

    return OAL_SUCC;
}


uint32_t wal_hipriv_set_fix_rate_pre_config(oal_net_device_stru *net_dev,
    oal_bool_enum_uint8 en_fix_rate_enable, mac_cfg_set_dscr_param_stru *pc_stu)
{
    mac_vap_stru *mac_vap = NULL;
    uint32_t ret;
    uint8_t uc_ampdu_cfg_idx;
    int8_t ac_sw_ampdu_cmd[WAL_AMPDU_CFG_BUTT][WAL_HIPRIV_CMD_NAME_MAX_LEN] = { { "0" }, { "1" } };

    mac_vap = oal_net_dev_priv(net_dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_fix_rate_pre_config::(net_dev)null ptr.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 配置无效固定速率时，开启autorate，恢复聚合开关值 */
    if (en_fix_rate_enable == OAL_FALSE) {
        /* 开启autorate算法 */
        ret = wal_hipriv_alg_cfg(net_dev, "ar_enable 1");
        if (ret != OAL_SUCC) {
            oam_error_log1(mac_vap->uc_vap_id, 0, "{wal_hipriv_set_fix_rate_pre_config:ar_enable fail:d%}", ret);
            return OAL_FAIL;
        }

        if (mac_vap->st_fix_rate_pre_para.en_rate_cfg_tag == OAL_TRUE) {
            /*  恢复ampdu聚合  */
            uc_ampdu_cfg_idx = mac_vap->st_fix_rate_pre_para.en_tx_ampdu_last;
            ret = wal_hipriv_ampdu_tx_on(net_dev, ac_sw_ampdu_cmd[uc_ampdu_cfg_idx]);
            if (ret != OAL_SUCC) {
                oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                    "{wal_hipriv_set_fix_rate_pre_config::ampdu cmd cfg fail[d%]}", ret);
                return OAL_FAIL;
            }

            /* 记录固定速率配置标记为未配置状态 */
            mac_vap->st_fix_rate_pre_para.en_rate_cfg_tag = OAL_FALSE;
        }

        return OAL_SUCC;
    }

    /*  关闭autorate算法  */
    ret = wal_hipriv_alg_cfg(net_dev, "ar_enable 0");
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, 0, "{wal_hipriv_set_fix_rate_pre_config::ar disable fail[d%]}", ret);
        return OAL_FAIL;
    }

    /* 11abg模式下配置固定速率前关闭ampdu聚合 */
    if (pc_stu->uc_function_index == WAL_DSCR_PARAM_RATE) {
        if (mac_vap->st_fix_rate_pre_para.en_rate_cfg_tag != OAL_TRUE) {
            /* 记录ampdu开关状态 */
            mac_vap->st_fix_rate_pre_para.en_tx_ampdu_last = mac_mib_get_CfgAmpduTxAtive(mac_vap);
            /* 记录固定速率配置标记为已配置状态 */
            mac_vap->st_fix_rate_pre_para.en_rate_cfg_tag = OAL_TRUE;
        }

        /*  关闭ampdu聚合  */
        ret = wal_hipriv_ampdu_tx_on(net_dev, ac_sw_ampdu_cmd[WAL_AMPDU_DISABLE]);
        if (ret != OAL_SUCC) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{wal_hipriv_set_fix_rate_pre_config::disable ampdu fail:%d}", ret);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_set_non_ht_rate(mac_cfg_non_ht_rate_stru *pst_set_non_ht_rate_param,
    wal_dscr_param_enum_uint8 en_param_index)
{
    if (en_param_index <= WLAN_SHORT_11B_11M_BPS) {
        pst_set_non_ht_rate_param->en_protocol_mode = WLAN_11B_PHY_PROTOCOL_MODE;
    } else if (en_param_index >= WLAN_LEGACY_OFDM_48M_BPS && en_param_index <= WLAN_LEGACY_OFDM_9M_BPS) {
        pst_set_non_ht_rate_param->en_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_non_ht_rate::invalid rate!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_process_rate_params_rate(oal_net_device_stru *net_dev,
    int8_t *ac_arg, mac_cfg_set_dscr_param_stru *pc_stu)
{
    wal_dscr_param_enum_uint8 en_param_index;
    mac_cfg_non_ht_rate_stru *pst_set_non_ht_rate_param = NULL;

    pst_set_non_ht_rate_param = (mac_cfg_non_ht_rate_stru *)(&(pc_stu->l_value));
    /* 解析参数 */
    for (en_param_index = 0; en_param_index < WLAN_LEGACY_RATE_VALUE_BUTT; en_param_index++) {
        if (!oal_strcmp(g_pauc_non_ht_rate_tbl[en_param_index], ac_arg)) {
            pst_set_non_ht_rate_param->en_rate = en_param_index;
            break;
        }
    }

    /* 根据速率配置TX描述符中的协议模式 ; 对于05或者03 ram 会在dmac 根据速率刷新协议 */
    if (OAL_SUCC != wal_hipriv_set_non_ht_rate(pst_set_non_ht_rate_param, en_param_index)) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (OAL_SUCC != wal_hipriv_set_fix_rate_pre_config(net_dev, OAL_TRUE, pc_stu)) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_process_rate_params_mcs(oal_net_device_stru *net_dev, int8_t *ac_arg,
    mac_cfg_set_dscr_param_stru *pc_stu)
{
    int32_t l_val;
    uint8_t uc_mcs_index;
    uint32_t ret;

    l_val = oal_strtol(ac_arg, NULL, 0);
    uc_mcs_index = pc_stu->uc_function_index - WAL_DSCR_PARAM_MCS;
    if (l_val < g_al_mcs_min_table[uc_mcs_index] || l_val > g_al_mcs_max_table[uc_mcs_index]) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::input mcs out of range[%d]}", l_val);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    pc_stu->l_value = l_val;
    ret = wal_hipriv_set_fix_rate_pre_config(net_dev, OAL_TRUE, pc_stu);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params:cfg fixed rate hdl fail[%d]}", ret);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_process_rate_params_nss(int8_t *ac_arg,
    mac_cfg_set_dscr_param_stru *pc_stu)
{
    wal_dscr_param_enum_uint8 en_param_index;

    for (en_param_index = 0; en_param_index < WLAN_NSS_LIMIT; en_param_index++) {
        if (!oal_strcmp(g_pauc_tx_dscr_nss_tbl[en_param_index], ac_arg)) {
            pc_stu->l_value = en_param_index;
            break;
        }
    }
    if (en_param_index == WAL_DSCR_PARAM_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::invalid param for nss!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_process_rate_params_bw(int8_t *ac_arg,
    mac_cfg_set_dscr_param_stru *pc_stu)
{
    wal_dscr_param_enum_uint8 en_param_index;

    for (en_param_index = 0; en_param_index < WLAN_BANDWITH_CAP_BUTT; en_param_index++) {
        if (!oal_strcmp(g_pauc_bw_tbl[en_param_index], ac_arg)) {
            pc_stu->l_value = en_param_index;
            break;
        }
    }
    if (en_param_index >= WLAN_BANDWITH_CAP_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::invalid param for bandwidth!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_process_rate_params_by_dscr_func(oal_net_device_stru *net_dev,
    int8_t *ac_arg, mac_cfg_set_dscr_param_stru *pc_stu, uint8_t ac_arg_len)
{
    uint32_t ret = OAL_FAIL;

    if (g_wlan_spec_cfg->feature_11ax_er_su_dcm_is_open
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
        && (pc_stu->uc_function_index == WAL_DSCR_PARAM_MCSAX_ER)
#endif
        ) {
        ret = wal_hipriv_process_rate_params_mcs(net_dev, ac_arg, pc_stu);
    } else {
        switch (pc_stu->uc_function_index) {
            case WAL_DSCR_PARAM_RATE:
                ret = wal_hipriv_process_rate_params_rate(net_dev, ac_arg, pc_stu);
                break;
            case WAL_DSCR_PARAM_MCS:
            case WAL_DSCR_PARAM_MCSAC:
            case WAL_DSCR_PARAM_MCSAX:
                ret = wal_hipriv_process_rate_params_mcs(net_dev, ac_arg, pc_stu);
                break;

            case WAL_DSCR_PARAM_NSS:
                ret = wal_hipriv_process_rate_params_nss(ac_arg, pc_stu);
                break;

            case WAL_DSCR_PARAM_BW:
                ret = wal_hipriv_process_rate_params_bw(ac_arg, pc_stu);
                break;

            default:
                oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::invalid cmd!}");
                return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}


uint32_t wal_hipriv_process_rate_params(oal_net_device_stru *net_dev,
    int8_t *pc_cmd, mac_cfg_set_dscr_param_stru *pc_stu)
{
    uint32_t ret;
    uint32_t off_set = 0;
    int32_t l_val;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    ret = wal_get_cmd_one_arg(pc_cmd, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /*  配置无效速率值255时恢复自动速率 */
    l_val = oal_strtol(ac_arg, NULL, 0);
    if (wal_ioctl_is_invalid_fixed_rate(l_val, pc_stu)) {
        ret = wal_hipriv_set_fix_rate_pre_config(net_dev, OAL_FALSE, pc_stu);
        if (ret != OAL_SUCC) {
            return OAL_FAIL;
        }
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    ret = wal_hipriv_process_rate_params_by_dscr_func(net_dev, ac_arg, pc_stu, WAL_HIPRIV_CMD_NAME_MAX_LEN);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}


void wal_msg_queue_init(void)
{
    memset_s((void *)&g_wal_wid_msg_queue, sizeof(g_wal_wid_msg_queue), 0, sizeof(g_wal_wid_msg_queue));
    oal_dlist_init_head(&g_wal_wid_msg_queue.st_head);
    g_wal_wid_msg_queue.count = 0;
    oal_spin_lock_init(&g_wal_wid_msg_queue.st_lock);
    oal_wait_queue_init_head(&g_wal_wid_msg_queue.st_wait_queue);
}


uint32_t wal_get_request_msg_count(void)
{
    return g_wal_wid_msg_queue.count;
}

uint32_t wal_check_and_release_msg_resp(wal_msg_stru *rsp_msg)
{
    wal_msg_write_rsp_stru *pst_write_rsp_msg = NULL;
    if (rsp_msg != NULL) {
        uint32_t err_code;
        wlan_cfgid_enum_uint16 en_wid;
        pst_write_rsp_msg = (wal_msg_write_rsp_stru *)(rsp_msg->auc_msg_data);
        err_code = pst_write_rsp_msg->err_code;
        en_wid = pst_write_rsp_msg->en_wid;
        oal_free(rsp_msg);

        if (err_code != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_SCAN,
                "{wal_check_and_release_msg_resp::detect err code:[%u],wid:[%u]}", err_code, en_wid);
            return err_code;
        }
    }

    return OAL_SUCC;
}


void wal_msg_request_add_queue(wal_msg_request_stru *msg)
{
    oal_spin_lock_bh(&g_wal_wid_msg_queue.st_lock);
    oal_dlist_add_tail(&msg->pst_entry, &g_wal_wid_msg_queue.st_head);
    g_wal_wid_msg_queue.count++;
    oal_spin_unlock_bh(&g_wal_wid_msg_queue.st_lock);
}


void wal_msg_request_remove_queue(wal_msg_request_stru *msg)
{
    oal_spin_lock_bh(&g_wal_wid_msg_queue.st_lock);
    g_wal_wid_msg_queue.count--;
    oal_dlist_delete_entry(&msg->pst_entry);
    oal_spin_unlock_bh(&g_wal_wid_msg_queue.st_lock);
}


int32_t wal_set_msg_response_by_addr(uintptr_t addr, void *resp_mem, uint32_t resp_ret, uint32_t rsp_len)
{
    int32_t ret = -OAL_EINVAL;
    oal_dlist_head_stru *pos = NULL;
    oal_dlist_head_stru *entry_temp = NULL;
    wal_msg_request_stru *request = NULL;

    oal_spin_lock_bh(&g_wal_wid_msg_queue.st_lock);
    oal_dlist_search_for_each_safe(pos, entry_temp, (&g_wal_wid_msg_queue.st_head))
    {
        request = (wal_msg_request_stru *)oal_dlist_get_entry(pos, wal_msg_request_stru, pst_entry);
        if (request->request_address == addr) {
            /* address match */
            if (oal_unlikely(request->pst_resp_mem != NULL)) {
                oam_error_log0(0, OAM_SF_ANY,
                    "{wal_set_msg_response_by_addr::wal_set_msg_response_by_addr rsp had set!");
            }
            request->pst_resp_mem = resp_mem;
            request->ret = resp_ret;
            request->resp_len = rsp_len;
            ret = OAL_SUCC;
            break;
        }
    }

    oal_spin_unlock_bh(&g_wal_wid_msg_queue.st_lock);

    return ret;
}


uint32_t wal_alloc_cfg_event(oal_net_device_stru *net_dev, frw_event_mem_stru **event_mem, void *resp_addr,
    wal_msg_stru **cfg_msg, uint16_t len)
{
    mac_vap_stru       *vap     = NULL;
    frw_event_mem_stru *evt_mem = NULL;
    frw_event_stru     *event   = NULL;
    uint16_t           resp_len = 0;
    wal_msg_rep_hdr    *rep_hdr = NULL;

    vap = oal_net_dev_priv(net_dev);
    if (oal_unlikely(vap == NULL)) {
        /* 规避wifi关闭状态下，下发hipriv命令显示error日志 */
        oam_warning_log0(0, OAM_SF_ANY,
            "{wal_alloc_cfg_event::OAL_NET_DEV_PRIV(net_dev) null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    resp_len += sizeof(wal_msg_rep_hdr);
    len += resp_len;

    evt_mem = frw_event_alloc_m(len);
    if (oal_unlikely(evt_mem == NULL)) {
        oam_error_log2(vap->uc_vap_id, OAM_SF_ANY,
            "{wal_alloc_cfg_event::evt_mem null ptr error,request size:len:%d,resp_len:%d}", len, resp_len);
        return OAL_ERR_CODE_PTR_NULL;
    }

    *event_mem = evt_mem; /* 出参赋值 */
    event = frw_get_event_stru(evt_mem);
    /* 填写事件头 */
    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_HOST_CRX,
                       WAL_HOST_CRX_SUBTYPE_CFG, len, FRW_EVENT_PIPELINE_STAGE_0,
                       vap->uc_chip_id, vap->uc_device_id, vap->uc_vap_id);
    /* fill the resp hdr */
    rep_hdr = (wal_msg_rep_hdr *)event->auc_event_data;
    if (resp_addr == NULL) {
        /* no response */
        rep_hdr->request_address = (uintptr_t)0;
    } else {
        /* need response */
        rep_hdr->request_address = (uintptr_t)resp_addr;
    }

    *cfg_msg = (wal_msg_stru *)((uint8_t *)event->auc_event_data + resp_len);

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE int32_t wal_request_wait_event_condition(wal_msg_request_stru *msg_stru)
{
    int32_t ret = OAL_FALSE;
    oal_spin_lock_bh(&g_wal_wid_msg_queue.st_lock);
    if ((msg_stru->pst_resp_mem != NULL) || (msg_stru->ret != OAL_SUCC)) {
        ret = OAL_TRUE;
    }
    oal_spin_unlock_bh(&g_wal_wid_msg_queue.st_lock);
    return ret;
}

void wal_cfg_msg_task_sched(void)
{
    oal_wait_queue_wake_up(&g_wal_wid_msg_queue.st_wait_queue);
}


int32_t wal_send_cfg_event(oal_net_device_stru *pst_net_dev,
    uint8_t msg_type, uint16_t us_len,
    uint8_t *puc_param, oal_bool_enum_uint8 en_need_rsp, wal_msg_stru **response)
{
    wal_msg_stru       *pst_cfg_msg = NULL;
    frw_event_mem_stru *pst_event_mem = NULL;
    wal_msg_stru       *rsp_msg = NULL;
    uint32_t          ret;
    int32_t           l_ret;

    DECLARE_WAL_MSG_REQ_STRU(st_msg_request);
    WAL_MSG_REQ_STRU_INIT(st_msg_request);

    if (response != NULL) {
        *response = NULL;
    }

    if (oal_warn_on((en_need_rsp == OAL_TRUE) && (response == NULL))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_send_cfg_event::response == NULL!}");
        return -OAL_EINVAL;
    }

    /* 申请事件 */
    ret = wal_alloc_cfg_event(pst_net_dev, &pst_event_mem, ((en_need_rsp == OAL_TRUE) ? &st_msg_request : NULL),
                              &pst_cfg_msg, WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_send_cfg_event::wal_alloc_cfg_event err %d!}", ret);
        return -OAL_ENOMEM;
    }

    /* 填写配置消息 */
    WAL_CFG_MSG_HDR_INIT(&(pst_cfg_msg->st_msg_hdr), msg_type, us_len, WAL_GET_MSG_SN());

    /* 填写WID消息 */
    if (EOK != memcpy_s(pst_cfg_msg->auc_msg_data, us_len, puc_param, us_len)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_send_cfg_event::memcpy fail!");
        frw_event_free_m(pst_event_mem);
        return -OAL_EINVAL;
    }

    if (en_need_rsp == OAL_TRUE) {
        /* add queue before post event! */
        wal_msg_request_add_queue(&st_msg_request);
    }

    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);

    /* win32 UT模式，触发一次事件调度 */
#if (_PRE_TEST_MODE == _PRE_TEST_MODE_UT || _PRE_TEST_MODE == _PRE_TEST_MODE_ST)
    frw_event_process_all_event(0);
#endif

    if (en_need_rsp == OAL_FALSE) {
        return OAL_SUCC;
    }

    /* context can't in interrupt */
    if (oal_warn_on(oal_in_interrupt())) {
        declare_dft_trace_key_info("wal_cfg_in_interrupt", OAL_DFT_TRACE_EXCEP);
    }

    if (oal_warn_on(oal_in_atomic())) {
        declare_dft_trace_key_info("wal_cfg_in_atomic", OAL_DFT_TRACE_EXCEP);
    }

    /* 等待事件返回 */
    wal_wake_lock();

    /* lint -e730 */ /* lint -e666 */ /* info, boolean argument to function */
    l_ret = oal_wait_event_timeout_m((g_wal_wid_msg_queue.st_wait_queue),
        (oal_bool_enum_uint8)(OAL_TRUE == wal_request_wait_event_condition(&st_msg_request)),
        (HMAC_WAIT_EVENT_RSP_TIME * OAL_TIME_HZ));
    /* lint +e730 */ /* lint +e666 */
    /* response had been set, remove it from the list */
    if (en_need_rsp == OAL_TRUE) {
        wal_msg_request_remove_queue(&st_msg_request);
    }

    if (oal_warn_on(l_ret == 0)) {
        /* 超时 */
        oam_error_log1(0, OAM_SF_ANY, "{wal_send_cfg_event:: wait queue timeout,%ds!}", HMAC_WAIT_EVENT_RSP_TIME);
        oal_io_print("[E]timeout,request info:%p,ret=%u,addr:0x%lx\n", st_msg_request.pst_resp_mem,
                     st_msg_request.ret, st_msg_request.request_address);
        WAL_MSG_REQ_RESP_MEM_FREE(st_msg_request);
        wal_wake_unlock();
        declare_dft_trace_key_info("wal_send_cfg_timeout", OAL_DFT_TRACE_FAIL);
        /* 打印CFG EVENT内存，方便定位 */
        oal_print_hex_dump((uint8_t *)pst_cfg_msg, (WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len),
            HEX_DUMP_GROUP_SIZE, "cfg event: ");
        /* 上行小包100%跑流，cpu 100%会触发打印，串口打印太多，取消打印 */
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
#ifndef CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT
        if (g_st_event_task[0].pst_event_kthread) {
            sched_show_task(g_st_event_task[0].pst_event_kthread);
        }
#endif
#endif
        return -OAL_ETIMEDOUT;
    }
    /*lint +e774*/
    rsp_msg = (wal_msg_stru *)(st_msg_request.pst_resp_mem);
    if (rsp_msg == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_send_cfg_event:: msg mem null!}");
        /*lint -e613*/
        // tscancode-suppress *
        *response = NULL;
        /*lint +e613*/
        wal_wake_unlock();
        return -OAL_EFAUL;
    }

    if (rsp_msg->st_msg_hdr.us_msg_len == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_send_cfg_event:: no msg resp!}");
        /*lint -e613*/
        *response = NULL;
        /*lint +e613*/
        oal_free(rsp_msg);
        wal_wake_unlock();
        return -OAL_EFAUL;
    }
    /* 发送配置事件返回的状态信息 */
    /*lint -e613*/
    *response = rsp_msg;
    /*lint +e613*/
    wal_wake_unlock();
    return OAL_SUCC;
}



wlan_p2p_mode_enum_uint8 wal_wireless_iftype_to_mac_p2p_mode(enum nl80211_iftype if_type)
{
    wlan_p2p_mode_enum_uint8 en_p2p_mode = WLAN_LEGACY_VAP_MODE;

    switch (if_type) {
        case NL80211_IFTYPE_P2P_CLIENT:
            en_p2p_mode = WLAN_P2P_CL_MODE;
            break;
        case NL80211_IFTYPE_P2P_GO:
            en_p2p_mode = WLAN_P2P_GO_MODE;
            break;
        case NL80211_IFTYPE_P2P_DEVICE:
            en_p2p_mode = WLAN_P2P_DEV_MODE;
            break;
        case NL80211_IFTYPE_AP:
        case NL80211_IFTYPE_STATION:
            en_p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        default:
            en_p2p_mode = WLAN_P2P_BUTT;
    }
    return en_p2p_mode;
}

int32_t wal_cfg_vap_h2d_event(oal_net_device_stru *net_dev)
{
    oal_wireless_dev_stru *wdev = NULL;
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    hmac_vap_stru *pst_cfg_hmac_vap = NULL;
    mac_device_stru *pst_mac_device = NULL;
    oal_net_device_stru *cfg_net_dev = NULL;

    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;
    uint32_t err_code;
    wal_msg_write_stru write_msg = {0};

    wdev = oal_netdevice_wdev(net_dev);
    if (wdev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::wdev is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wiphy_priv = (mac_wiphy_priv_stru *)oal_wiphy_priv(wdev->wiphy);
    if (wiphy_priv == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::wiphy_priv is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_device = wiphy_priv->pst_mac_device;
    if (pst_mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::mac_device is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->uc_cfg_vap_id);
    if (pst_cfg_hmac_vap == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::mac_res_get_hmac_vap fail.vap_id[%u]}",
            pst_mac_device->uc_cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    cfg_net_dev = pst_cfg_hmac_vap->pst_net_device;
    if (cfg_net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::cfg_net_dev is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_CFG_VAP_H2D, sizeof(mac_cfg_vap_h2d_stru));
    ((mac_cfg_vap_h2d_stru *)write_msg.auc_value)->pst_net_dev = cfg_net_dev;

    /* 发送消息 */
    ret = wal_send_cfg_event(cfg_net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_vap_h2d_stru),
                             (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::wal_alloc_cfg_event err %d!}", ret);
        return ret;
    }

    /* 处理返回消息 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event::hmac cfg vap h2d fail,err code[%u]", err_code);
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t wal_host_dev_config(oal_net_device_stru *net_dev, wlan_cfgid_enum_uint16 en_wid)
{
    oal_wireless_dev_stru *pst_wdev = NULL;
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    hmac_vap_stru *pst_cfg_hmac_vap = NULL;
    mac_device_stru *pst_mac_device = NULL;
    oal_net_device_stru *cfg_net_dev = NULL;

    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;
    uint32_t err_code;
    wal_msg_write_stru write_msg = {0};

    pst_wdev = oal_netdevice_wdev(net_dev);
    if (pst_wdev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_host_dev_config::pst_wdev is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wiphy_priv = (mac_wiphy_priv_stru *)oal_wiphy_priv(pst_wdev->wiphy);
    if (wiphy_priv == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_host_dev_config::pst_mac_device is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_device = wiphy_priv->pst_mac_device;
    if (pst_mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_host_dev_config::pst_mac_device is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->uc_cfg_vap_id);
    if (pst_cfg_hmac_vap == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_host_dev_config::pst_cfg_hmac_vap is null vap_id:%d!}",
            pst_mac_device->uc_cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    cfg_net_dev = pst_cfg_hmac_vap->pst_net_device;
    if (cfg_net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_host_dev_config::cfg_net_dev is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛事件到wal层处理 */
    /* 填写消息 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, en_wid, 0);

    /* 发送消息 */
    ret = wal_send_cfg_event(cfg_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH,
                             (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_host_dev_config::wal_alloc_cfg_event err %d!}", ret);
        return ret;
    }

    /* 处理返回消息 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_host_dev_config::hmac cfg vap h2d fail,err code[%u]", err_code);
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}


int32_t wal_host_dev_init(oal_net_device_stru *net_dev)
{
    return wal_host_dev_config(net_dev, WLAN_CFGID_HOST_DEV_INIT);
}


int32_t wal_host_dev_exit(oal_net_device_stru *net_dev)
{
    return wal_host_dev_config(net_dev, WLAN_CFGID_HOST_DEV_EXIT);
}


int32_t wal_set_power_on(oal_net_device_stru *net_dev, int32_t power_flag)
{
    int32_t ret;

    // ap上下电，配置VAP
    if (power_flag == 0) { // 下电
        /* 下电host device_stru去初始化 */
        wal_host_dev_exit(net_dev);
        wal_wake_lock();
#ifndef _PRE_EMU /* EMU平台上下电速度慢，再ap场景下，emu平台不下电 */
        wlan_pm_close();
#endif
        wal_wake_unlock();
        g_st_ap_config_info.l_ap_power_flag = OAL_FALSE;
    } else if (power_flag == 1) {  // 上电
        g_st_ap_config_info.l_ap_power_flag = OAL_TRUE;
        wal_wake_lock();
        ret = wlan_pm_open();
        wal_wake_unlock();
        if (ret == OAL_FAIL) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_set_power_on::wlan_pm_open Fail!}");
            return -OAL_EFAIL;
        } else if (ret != OAL_ERR_CODE_ALREADY_OPEN) {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
            /* 重新上电时置为FALSE */
            hwifi_config_init_force();
#endif
            // 重新上电场景，下发配置VAP
            ret = wal_cfg_vap_h2d_event(net_dev);
            if (ret != OAL_SUCC) {
                return -OAL_EFAIL;
            }
        }

        /* 上电host device_stru初始化 */
        ret = wal_host_dev_init(net_dev);
        if (ret != OAL_SUCC) {
            oal_io_print("wal_set_power_on FAIL %d \r\n", ret);
            return -OAL_EFAIL;
        }
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_set_power_on::pupower_flag:%d error.}", power_flag);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


void wal_set_power_mgmt_on(uint8_t power_mgmt_flag)
{
    struct wlan_pm_s *wlan_pm = NULL;
    wlan_pm = wlan_pm_get_drv();
    if (wlan_pm != NULL) {
        /* ap模式下，是否允许下电操作,1:允许,0:不允许 */
        wlan_pm->apmode_allow_pm_flag = power_mgmt_flag;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_set_power_mgmt_on::wlan_pm_get_drv return null.");
    }
}

#ifdef _PRE_WLAN_RR_PERFORMANCE

OAL_STATIC int32_t wal_ext_priv_cmd_dev_freq_check_para(oal_net_device_stru *net_dev)
{
    if (oal_any_null_ptr1(net_dev)) {
        oam_error_log0(0, OAM_SF_CFG, "wal_ext_priv_cmd_dev_freq_check_para::null_ptr.");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_priv_cmd_dev_freq_para(mac_vap_stru *mac_vap, mac_cfg_set_tlv_stru *config_para,
                                              uint8_t cmd_type, uint8_t value)
{
    int32_t     ret;
    config_para->uc_cmd_type = cmd_type;
    config_para->value    = value;
    config_para->uc_len      = sizeof(mac_cfg_set_tlv_stru);
    config_para->us_cfg_id   = WLAN_CFGID_SET_DEVICE_FREQ;

    ret = hmac_config_set_tlv_cmd(mac_vap, 0, (uint8_t *)config_para);
    oam_error_log3(0, OAM_SF_CFG, "hmac_config_set_tlv_cmd[%d,%d] ret:[%d]!\n", cmd_type, value, ret);
    return ret;
}


OAL_STATIC int32_t wal_ext_priv_cmd_set_dev_freq(oal_net_device_stru *net_dev, uint8_t flag)
{
    mac_cfg_set_tlv_stru *config_para = NULL;
    mac_vap_stru *mac_vap = NULL;
    int32_t result;

    result = wal_ext_priv_cmd_dev_freq_check_para(net_dev);
    if (result != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_CFG, "wal_ext_priv_cmd_set_dev_freq::wal_ext_priv_cmd_dev_freq_check_para fail!\n");
        return result;
    }
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_ext_priv_cmd_set_dev_freq::vap is null.}");
        return -OAL_EINVAL;
    }

    /* 申请内存 */
    config_para = (mac_cfg_set_tlv_stru *)oal_memalloc(sizeof(mac_cfg_set_tlv_stru));
    if (oal_unlikely(config_para == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "wal_ext_priv_cmd_set_dev_freq::config_para malloc fail!\n");
        return -OAL_EFAIL;
    }
    memset_s((void *)config_para, sizeof(mac_cfg_set_tlv_stru), 0, sizeof(mac_cfg_set_tlv_stru));

    if (flag == FREQ_AUTO_FLAG) {
        result = wal_priv_cmd_dev_freq_para(mac_vap, config_para, CMD_SET_AUTO_FREQ_ENDABLE, FREQ_LOCK_ENABLE);
        if (result != OAL_SUCC) {
            oal_free(config_para);
            config_para = NULL;
            oam_error_log0(0, OAM_SF_CFG, "wal_ext_priv_cmd_set_dev_freq::wal_ext_priv_cmd_fill_para fail!\n");
            return result;
        }
    } else if (flag == FREQ_MAX_FLAG) {
        result = wal_priv_cmd_dev_freq_para(mac_vap, config_para, CMD_SET_AUTO_FREQ_ENDABLE, FREQ_LOCK_DISABLE);
        if (result != OAL_SUCC) {
            oal_free(config_para);
            config_para = NULL;
            oam_error_log0(0, OAM_SF_CFG, "wal_ext_priv_cmd_set_dev_freq::wal_ext_priv_cmd_fill_para fail!\n");
            return result;
        }

        result = wal_priv_cmd_dev_freq_para(mac_vap, config_para, CMD_SET_DEVICE_FREQ_VALUE, FREQ_MAX_VALUE);
        if (result != OAL_SUCC) {
            oal_free(config_para);
            config_para = NULL;
            oam_error_log0(0, OAM_SF_CFG, "wal_ext_priv_cmd_set_dev_freq::wal_ext_priv_cmd_fill_para fail!\n");
            return result;
        }
    }

    oal_free(config_para);
    config_para = NULL;
    oam_warning_log0(0, OAM_SF_CFG, "wal_ext_priv_cmd_set_dev_freq::success!\n");
    return OAL_SUCC;
}


OAL_STATIC int32_t wal_ext_priv_cmd_pcie_aspm(int32_t aspm)
{
    return hi110x_hcc_ip_pm_ctrl(aspm);
}


int32_t wal_ext_priv_cmd(oal_net_device_stru *net_dev, oal_ifreq_stru *pst_ifr)
{
    wal_wifi_pc_cmd_stru  priv_cmd = {0};
    int32_t               ret = OAL_SUCC;

    if (oal_any_null_ptr2(pst_ifr->ifr_data, net_dev)) {
        return -OAL_EINVAL;
    }
    /* 将用户态数据拷贝到内核态 */
    if (oal_copy_from_user(&priv_cmd, pst_ifr->ifr_data, sizeof(wal_wifi_pc_cmd_stru))) {
        return -OAL_EINVAL;
    }

    switch (priv_cmd.cmd) {
        case WAL_EXT_PRI_CMD_SET_DEVICE_FREQ_MAX:
            ret = wal_ext_priv_cmd_set_dev_freq(net_dev, FREQ_MAX_FLAG);
            break;
        case WAL_EXT_PRI_CMD_SET_DEVICE_FREQ_AUTO:
            ret = wal_ext_priv_cmd_set_dev_freq(net_dev, FREQ_AUTO_FLAG);
            break;
        case WAL_EXT_PRI_CMD_ENBLE_PCIE_ASPM:
            ret = wal_ext_priv_cmd_pcie_aspm(ASPM_ENABLE);
            oam_warning_log2(0, OAM_SF_CFG, "wal_ext_priv_cmd_pcie_aspm::cmd = %d, ret = %d\n", ASPM_ENABLE, ret);
            break;
        case WAL_EXT_PRI_CMD_DISABLE_PCIE_ASPM:
            ret = wal_ext_priv_cmd_pcie_aspm(ASPM_DISABLE);
            oam_warning_log2(0, OAM_SF_CFG, "wal_ext_priv_cmd_pcie_aspm::cmd = %d, ret = %d\n", ASPM_DISABLE, ret);
            break;
        default:
            break;
    }
    chr_exception_p(CHR_WIFI_IWARE_RR_EVENTID, (uint8_t *)&priv_cmd.cmd, sizeof(uint32_t));
    return ret;
}

#endif /* _PRE_WLAN_RR_PERFORMANCE */
static oal_bool_enum_uint8 wal_sniffer_up_vap_is_p2p_mode(mac_device_stru *mac_device)
{
    mac_vap_stru *mac_vap = NULL;
    uint8_t vap_idx;

    for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
        mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
        if (mac_vap == NULL) {
            oam_error_log1(0, OAM_SF_ANY, "vap is null, vap id is %d",
                           mac_device->auc_vap_id[vap_idx]);
            continue;
        }

        if (mac_vap->en_vap_state == MAC_VAP_STATE_UP && ((mac_vap->en_p2p_mode == WLAN_P2P_GO_MODE) ||
            (mac_vap->en_p2p_mode == WLAN_P2P_CL_MODE))) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_sniffer_up_vap_is_p2p_mode::has up p2p vap[%d].}", mac_vap->en_p2p_mode);
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


static oal_bool_enum_uint8 wal_sniffer_is_single_sta_mode(oal_net_device_stru *net_dev)
{
    oal_wireless_dev_stru *wdev = NULL;
    mac_wiphy_priv_stru *priv = NULL;
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *mac_vap = NULL;
    uint8_t up_vap_num;

    wdev = oal_netdevice_wdev(net_dev);
    if (wdev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_sniffer_is_single_sta_mode::pst_wdev is null!}");
        return OAL_FALSE;
    }

    priv = (mac_wiphy_priv_stru *)oal_wiphy_priv(wdev->wiphy);
    if (priv == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_sniffer_is_single_sta_mode::pst_mac_device is null!}");
        return OAL_FALSE;
    }
    mac_device = priv->pst_mac_device;
    if (mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_sniffer_is_single_sta_mode::pst_mac_device is null!}");
        return OAL_FALSE;
    }
    // 计算当前mac_device下处在up状态的vap个数
    // sniffer场景:打开wifi，关联状态:vap 处于up状态；不关联:fake up，此时p2p_device也处于fake up
    up_vap_num = mac_device_get_up_vap_num(mac_device);
    if (up_vap_num > 1) { // 有多个处于up状态的vap返回；等于0说明没有处于up状态的vap，wlan开启但没有关联属于正常场景
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_sniffer_is_single_sta_mode::up_vap_num[%d] invalid, can not start sniffer!}", up_vap_num);
        return OAL_FALSE;
    } else if (up_vap_num == 1) {
        if (wal_sniffer_up_vap_is_p2p_mode(mac_device)) { // 遍历mac_device下所有vap，如果单vap up为p2p，返回
            oam_warning_log0(0, OAM_SF_ANY, "{wal_sniffer_is_single_sta_mode::single up vap is p2p, stop sniffer!}");
            return OAL_FALSE;
        }
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_sniffer_is_single_sta_mode::mac vap null, can not start sniffer!}");
        return OAL_FALSE;
    }
    if (!IS_LEGACY_STA(mac_vap)) { // 单vap up但不是sta或者为p2p up，返回
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_sniffer_is_single_sta_mode::vap mode[%d]not sta, can not start sniffer!}", mac_vap->en_vap_mode);
        return OAL_FALSE;
    }

    return OAL_TRUE;
}


OAL_STATIC uint32_t wal_hipriv_set_mode_config(oal_net_device_stru *net_dev, uint8_t uc_prot_idx)
{
    mac_cfg_mode_param_stru *pst_mode_param = NULL;
    wal_msg_stru *rsp_msg = NULL;
    wal_msg_write_stru write_msg;
    int32_t ret;

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_MODE, sizeof(mac_cfg_mode_param_stru));

    pst_mode_param = (mac_cfg_mode_param_stru *)(write_msg.auc_value);
    pst_mode_param->en_protocol = g_ast_mode_map[uc_prot_idx].en_mode;
    pst_mode_param->en_band = g_ast_mode_map[uc_prot_idx].en_band;
    pst_mode_param->en_bandwidth = g_ast_mode_map[uc_prot_idx].en_bandwidth;

    oam_warning_log3(0, OAM_SF_CFG, "{wal_hipriv_set_mode_config::protocol[%d],band[%d],bandwidth[%d]!}",
                     pst_mode_param->en_protocol, pst_mode_param->en_band, pst_mode_param->en_bandwidth);

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_mode_param_stru),
                             (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mode_config::wal_alloc_cfg_event err %d!}", ret);
        return (uint32_t)ret;
    }

    /* 读取返回的错误码 */
    ret = wal_check_and_release_msg_resp(rsp_msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_mode_config fail, err code[%u]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_set_mode(oal_net_device_stru *net_dev, int8_t *param)
{
    return wal_ioctl_set_mode(net_dev, param);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))

OAL_STATIC uint32_t wal_hipriv_set_essid_config(oal_net_device_stru *net_dev, mac_vap_stru *mac_vap, int8_t *ac_ssid)
{
    mac_cfg_ssid_param_stru *param = NULL;
    wal_msg_write_stru write_msg;
    uint8_t ssid_len;
    int32_t ret;
    int8_t *ssid = NULL;

    ssid = ac_ssid;
    ssid = oal_strim(ac_ssid); /* 去掉字符串开始结尾的空格 */
    ssid_len = (uint8_t)OAL_STRLEN(ssid);

    oam_info_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_set_essid_config:: ssid length %d!}", ssid_len);

    if (ssid_len > WLAN_SSID_MAX_LEN - 1) { /* -1为\0预留空间 */
        ssid_len = WLAN_SSID_MAX_LEN - 1;
    }

    oam_info_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_set_essid_config:: ssid length is %d!}", ssid_len);
    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SSID, sizeof(mac_cfg_ssid_param_stru));

    /* 填写WID对应的参数 */
    param = (mac_cfg_ssid_param_stru *)(write_msg.auc_value);
    param->uc_ssid_len = ssid_len;
    if (EOK != memcpy_s(param->ac_ssid, sizeof(param->ac_ssid), ssid, ssid_len)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_set_essid_config::memcpy fail!");
        return OAL_FAIL;
    }

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_ssid_param_stru),
                             (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_essid_config::alloc_cfg_event err:%d}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}


uint32_t wal_hipriv_set_essid(oal_net_device_stru *net_dev, int8_t *param)
{
    int32_t ret;
    mac_vap_stru *mac_vap = NULL;
    uint32_t off_set = 0;
    int8_t ac_ssid[WLAN_SSID_MAX_LEN] = { 0 };
    uint32_t result;

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_essid::mac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 设备在up状态且是AP时，不允许配置，必须先down */
        if (0 != (OAL_IFF_RUNNING & oal_netdevice_flags(net_dev))) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                "{wal_hipriv_set_essid::dev  busy, please down it first:%d}", oal_netdevice_flags(net_dev));
            return -OAL_EBUSY;
        }
    }

    /* param指向传入模式参数, 将其取出存放到mode_str中 */
    result = wal_get_cmd_one_arg(param, ac_ssid, WLAN_SSID_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_essid::wal_get_cmd_one_arg vap name err %d}", result);
        return result;
    }

    /* 设置ssid并发送消息到wal层处理 */
    ret = wal_hipriv_set_essid_config(net_dev, mac_vap, ac_ssid);
    if (ret != OAL_SUCC) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_set_essid::send cfg event fail}");
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}
#endif
/*
 * 函 数 名  : wal_octl_get_essid
 * 功能描述  : 获取ssid
 */
OAL_STATIC int wal_ioctl_get_essid(oal_net_device_stru *net_dev, oal_iw_request_info_stru *pst_info,
                                   oal_iwreq_data_union *pst_data, char *pc_ssid)
{
    int32_t ret;
    wal_msg_query_stru st_query_msg = {0};
    mac_cfg_ssid_param_stru *pst_ssid = NULL;
    wal_msg_stru *rsp_msg = NULL;
    wal_msg_rsp_stru *pst_query_rsp_msg = NULL;
    oal_iw_point_stru *pst_essid = (oal_iw_point_stru *)pst_data;

    /* 抛事件到wal层处理 */
    st_query_msg.en_wid = WLAN_CFGID_SSID;

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_QUERY, WAL_MSG_WID_LENGTH,
                             (uint8_t *)&st_query_msg, OAL_TRUE, &rsp_msg);
    if (ret != OAL_SUCC || rsp_msg == NULL) {
        if (rsp_msg != NULL) {
            oal_free(rsp_msg);
        }
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_get_essid:: wal_send_cfg_event err %d!}", ret);
        return -OAL_EFAIL;
    }

    /* 处理返回消息 */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(rsp_msg->auc_msg_data);

    /* 业务处理 */
    pst_ssid = (mac_cfg_ssid_param_stru *)(pst_query_rsp_msg->auc_value);
    pst_essid->flags = 1; /* 设置出参标志为有效 */
    pst_essid->length = oal_min(pst_ssid->uc_ssid_len, OAL_IEEE80211_MAX_SSID_LEN);
    if (EOK != memcpy_s(pc_ssid, pst_essid->length, pst_ssid->ac_ssid, pst_essid->length)) {
        oam_warning_log1(0, OAM_SF_ANY, "wal_ioctl_get_essid::memcpy fail! pst_essid->length[%d]", pst_essid->length);
        oal_free(rsp_msg);
        return -OAL_EINVAL;
    }

    oal_free(rsp_msg);
    return OAL_SUCC;
}


OAL_STATIC int wal_ioctl_get_apaddr(oal_net_device_stru *net_dev,
                                    oal_iw_request_info_stru *pst_info,
                                    oal_iwreq_data_union *pst_wrqu,
                                    char *pc_extra)
{
    mac_vap_stru *mac_vap = NULL;
    oal_sockaddr_stru *pst_addr = (oal_sockaddr_stru *)pst_wrqu;
    uint8_t auc_zero_addr[WLAN_MAC_ADDR_LEN] = { 0 };

    if ((net_dev == NULL) || (pst_addr == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_get_apaddr::param null.}");
        return -OAL_EINVAL;
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_get_apaddr::mac_vap is null!}");
        return -OAL_EFAUL;
    }

    if (mac_vap->en_vap_state == MAC_VAP_STATE_UP) {
        oal_set_mac_addr((uint8_t *)pst_addr->sa_data, mac_vap->auc_bssid);
    } else {
        oal_set_mac_addr((uint8_t *)pst_addr->sa_data, auc_zero_addr);
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_set_freq(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    return wal_ioctl_set_freq(net_dev, pc_param);
}

#ifdef _PRE_WLAN_FEATURE_11D

OAL_STATIC oal_bool_enum_uint8 wal_is_alpha_upper(int8_t c_letter)
{
    if (c_letter >= 'A' && c_letter <= 'Z') {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


uint8_t wal_regdomain_get_band(uint32_t start_freq, uint32_t end_freq)
{
    if (start_freq > 2400 && end_freq < 2500) { // 2400~2500代表2G频率范围
        return MAC_RC_START_FREQ_2;
    } else if (start_freq > 5000 && end_freq < 5870) { // 5000~5870代表5G频率范围
        return MAC_RC_START_FREQ_5;
    } else if (start_freq > 4900 && end_freq < 4999) { // 4900~4999代表5G频率范围
        return MAC_RC_START_FREQ_5;
    } else {
        return MAC_RC_START_FREQ_BUTT;
    }
}


#define WAL_CHAN_BW_80M 80
#define WAL_CHAN_BW_40M 40
#define WAL_CHAN_BW_20M 20
uint8_t wal_regdomain_get_bw(uint8_t uc_bw)
{
    uint8_t uc_bw_map;

    switch (uc_bw) {
        case WAL_CHAN_BW_80M:
            uc_bw_map = MAC_CH_SPACING_80MHZ;
            break;
        case WAL_CHAN_BW_40M:
            uc_bw_map = MAC_CH_SPACING_40MHZ;
            break;
        case WAL_CHAN_BW_20M:
            uc_bw_map = MAC_CH_SPACING_20MHZ;
            break;
        default:
            uc_bw_map = MAC_CH_SPACING_BUTT;
            break;
    };

    return uc_bw_map;
}


uint32_t wal_regdomain_get_channel_2g(uint32_t start_freq, uint32_t end_freq)
{
    uint32_t freq;
    uint32_t i;
    uint32_t ch_bmap = 0;

    for (freq = start_freq + 10; freq <= (end_freq - 10); freq++) { // 10代表找到中心频点需要的增减量
        for (i = 0; i < MAC_CHANNEL_FREQ_2_BUTT; i++) {
            if (freq == g_ast_freq_map_2g[i].us_freq) {
                ch_bmap |= (1 << i);
            }
        }
    }

    return ch_bmap;
}


uint32_t wal_regdomain_get_channel_5g(uint32_t start_freq, uint32_t end_freq)
{
    uint32_t freq;
    uint32_t i;
    uint32_t ch_bmap = 0;
    // 5代表频率增加到下一频率的增量
    for (freq = start_freq + 10; freq <= (end_freq - 10); freq += 5) { // 10代表找到中心频点需要的增减量
        for (i = 0; i < MAC_CHANNEL_FREQ_5_BUTT; i++) {
            if (freq == g_ast_freq_map_5g[i].us_freq) {
                ch_bmap |= (1 << i);
            }
        }
    }

    return ch_bmap;
}


uint32_t wal_regdomain_get_channel(uint8_t uc_band,
    uint32_t start_freq, uint32_t end_freq)
{
    uint32_t ch_bmap = 0;

    switch (uc_band) {
        case MAC_RC_START_FREQ_2:
            ch_bmap = wal_regdomain_get_channel_2g(start_freq, end_freq);
            break;

        case MAC_RC_START_FREQ_5:
            ch_bmap = wal_regdomain_get_channel_5g(start_freq, end_freq);
            break;

        default:
            break;
    }

    return ch_bmap;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0))
uint32_t wal_linux_update_wiphy_channel_list_num(oal_net_device_stru *net_dev,
    oal_wiphy_stru *pst_wiphy)
{
    uint16_t len;
    uint32_t ret;
    mac_vendor_cmd_channel_list_stru st_channel_list = { 0 };
    mac_vap_stru *mac_vap;

    if (oal_any_null_ptr2(pst_wiphy, net_dev)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num::input params null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num::NET_DEV_PRIV is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_config_vendor_cmd_get_channel_list(mac_vap, &len, (uint8_t *)(&st_channel_list));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num::get chan list fail:%d}", ret);
        return ret;
    }

    /* 只更新2G信道个数，5G信道由于存在DFS 区域，且带宽计算并无问题,不需要修改 */
    g_st_supported_band_2ghz_info.n_channels = st_channel_list.uc_channel_num_2g;

    oam_warning_log2(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num::2g_chan_num:%d, 5g_chan_num:%d}",
                     st_channel_list.uc_channel_num_2g,
                     st_channel_list.uc_channel_num_5g);
    return OAL_SUCC;
}
#endif

/*
 * 函 数 名  : wal_get_dfs_domain
 * 功能描述  : 根据国家码，获取对应的雷达检测标准
 */
OAL_STATIC mac_dfs_domain_enum wal_get_dfs_domain(const int8_t *pc_country)
{
    uint32_t u_idx;
    uint32_t size = oal_array_size(g_ast_dfs_domain_table);

    for (u_idx = 0; u_idx < size; u_idx++) {
        if (oal_strcmp(g_ast_dfs_domain_table[u_idx].pc_country, pc_country) == 0) {
            return g_ast_dfs_domain_table[u_idx].en_dfs_domain;
        }
    }

    return MAC_DFS_DOMAIN_NULL;
}

OAL_STATIC void wal_regdomain_fill_info(const oal_ieee80211_regdomain_stru *pst_regdom,
    mac_regdomain_info_stru *pst_mac_regdom)
{
    uint32_t i;
    uint32_t start;
    uint32_t end;
    uint8_t uc_band;
    uint8_t uc_bw;

    /* 复制国家字符串 */
    pst_mac_regdom->ac_country[0] = pst_regdom->alpha2[0];
    pst_mac_regdom->ac_country[1] = pst_regdom->alpha2[1];
    pst_mac_regdom->ac_country[2] = 0;

    /* 获取DFS认证标准类型 */
    pst_mac_regdom->en_dfs_domain = wal_get_dfs_domain(pst_regdom->alpha2);

    /* 填充管制类个数 */
    pst_mac_regdom->uc_regclass_num = (uint8_t)pst_regdom->n_reg_rules;

    /* 填充管制类信息 */
    for (i = 0; i < pst_regdom->n_reg_rules; i++) {
        /* 填写管制类的频段(2.4G或5G) */
        start = pst_regdom->reg_rules[i].freq_range.start_freq_khz / 1000;
        end = pst_regdom->reg_rules[i].freq_range.end_freq_khz / 1000;
        uc_band = wal_regdomain_get_band(start, end);
        pst_mac_regdom->ast_regclass[i].en_start_freq = uc_band;

        /* 填写管制类允许的最大带宽 */
        uc_bw = (uint8_t)(pst_regdom->reg_rules[i].freq_range.max_bandwidth_khz / 1000);
        pst_mac_regdom->ast_regclass[i].en_ch_spacing = wal_regdomain_get_bw(uc_bw);

        /* 填写管制类信道位图 */
        pst_mac_regdom->ast_regclass[i].channel_bmap = wal_regdomain_get_channel(uc_band, start, end);

        /* 标记管制类行为 */
        pst_mac_regdom->ast_regclass[i].uc_behaviour_bmap = 0;

        if (pst_regdom->reg_rules[i].flags & NL80211_RRF_DFS) {
            pst_mac_regdom->ast_regclass[i].uc_behaviour_bmap |= MAC_RC_DFS;
        }

        if (pst_regdom->reg_rules[i].flags & NL80211_RRF_NO_INDOOR) {
            pst_mac_regdom->ast_regclass[i].uc_behaviour_bmap |= MAC_RC_NO_INDOOR;
        }

        if (pst_regdom->reg_rules[i].flags & NL80211_RRF_NO_OUTDOOR) {
            pst_mac_regdom->ast_regclass[i].uc_behaviour_bmap |= MAC_RC_NO_OUTDOOR;
        }
        /* 填充覆盖类和最大发送功率 */
        pst_mac_regdom->ast_regclass[i].uc_coverage_class = 0;
        pst_mac_regdom->ast_regclass[i].uc_max_reg_tx_pwr =
            (uint8_t)(pst_regdom->reg_rules[i].power_rule.max_eirp / 100);
        pst_mac_regdom->ast_regclass[i].us_max_tx_pwr =
            (uint16_t)(pst_regdom->reg_rules[i].power_rule.max_eirp / 10);
    }
}


int32_t wal_regdomain_update(oal_net_device_stru *net_dev, int8_t *pc_country)
{
    uint8_t uc_dev_id;
    mac_device_stru *pst_device = NULL;

    const oal_ieee80211_regdomain_stru *pst_regdom = NULL;
    uint16_t us_size;
    mac_regdomain_info_stru *pst_mac_regdom = NULL;
    wal_msg_write_stru write_msg = {0};
    mac_cfg_country_stru *pst_param = NULL;
    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    int8_t old_pc_country[COUNTRY_CODE_LEN] = { '9', '9' };
#endif

    mac_vap_stru *mac_vap = NULL;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if (EOK != memcpy_s(old_pc_country, COUNTRY_CODE_LEN, hwifi_get_country_code(), COUNTRY_CODE_LEN)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_regdomain_update::memcpy fail!");
        return -OAL_EINVAL;
    }
    hwifi_set_country_code(pc_country, COUNTRY_CODE_LEN);
    /* 如果新的国家码和旧国家处于一个regdomain，不刷新RF参数，只更新国家码 */
    if (OAL_TRUE == hwifi_is_regdomain_changed((uint8_t *)old_pc_country, (uint8_t *)pc_country)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update::regdomain changed, refresh rf param!}");

        /* 刷新参数失败，为了保证国家码和功率参数对应 */
        /* 将国家码设回原来的国家码，本次更新失败 */
        if (hwifi_force_refresh_rf_params(net_dev) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY,
                "{wal_regdomain_update::refresh rf(max_txpwr & dbb scale) params failed. Set country back.!}");
            hwifi_set_country_code(old_pc_country, COUNTRY_CODE_LEN);
        }
    }
#endif

    if (!wal_is_alpha_upper(pc_country[0]) || !wal_is_alpha_upper(pc_country[1])) {
        if ((pc_country[0] == '9') && (pc_country[1] == '9')) {
            oam_info_log0(0, OAM_SF_ANY, "{wal_regdomain_update::set regdomain to 99!}");
        } else {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update::country str invalid!}");
            return -OAL_EINVAL;
        }
    }

    pst_regdom = wal_regdb_find_db(pc_country);
    if (pst_regdom == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update::no regdomain db was found!}");
        return -OAL_EINVAL;
    }

    us_size = (uint16_t)(sizeof(mac_regclass_info_stru) * pst_regdom->n_reg_rules + MAC_RD_INFO_LEN);

    /* 申请内存存放管制域信息，将内存指针作为事件payload抛下去 */
    /* 此处申请的内存在事件处理函数释放(hmac_config_set_country) */
    pst_mac_regdom = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, us_size, OAL_TRUE);
    if (pst_mac_regdom == NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_regdomain_update::alloc regdom mem fail(%d),null ptr!}", us_size);
        return -OAL_ENOMEM;
    }

    wal_regdomain_fill_info(pst_regdom, pst_mac_regdom);

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_COUNTRY, sizeof(mac_cfg_country_stru));

    /* 填写WID对应的参数 */
    pst_mac_regdom->en_regdomain = hwifi_get_regdomain_from_country_code(pc_country);
    pst_param = (mac_cfg_country_stru *)(write_msg.auc_value);
    pst_param->p_mac_regdom = pst_mac_regdom;

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_country_stru),
                             (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_regdomain_update::err %d!}", ret);
        if (oal_value_ne_all2(ret, -OAL_ETIMEDOUT, -OAL_EFAUL)) {
            oal_mem_free_m(pst_mac_regdom, OAL_TRUE);
            pst_mac_regdom = NULL;
        }
        if (rsp_msg != NULL) {
            oal_free(rsp_msg);
        }
        return ret;
    }
    oal_free(rsp_msg);

    /* 驱动不支持ACS时，更新hostapd管制域信息; 如果驱动支持ACS，则不需要更新，否则hostapd无法配置DFS信道 */
    mac_vap = (mac_vap_stru *)oal_net_dev_priv(net_dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update::OAL_NET_DEV_PRIV(net_dev) null.}");
        return -OAL_FAIL;
    }

    uc_dev_id = mac_vap->uc_device_id;
    pst_device = mac_res_get_dev(uc_dev_id);
    if ((pst_device != NULL) && (pst_device->pst_wiphy != NULL)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0))
        
        wal_linux_update_wiphy_channel_list_num(net_dev, pst_device->pst_wiphy);
#endif

        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update::update regdom to kernel.}");

        wal_cfg80211_reset_bands(uc_dev_id);
        oal_wiphy_apply_custom_regulatory(pst_device->pst_wiphy, pst_regdom);
        
        wal_cfg80211_save_bands(uc_dev_id);
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

OAL_STATIC void wal_regdomain_update_rf_param(oal_net_device_stru *net_dev, int8_t *pc_country,
    int8_t *old_pc_country)
{
    if (EOK != memcpy_s(old_pc_country, COUNTRY_CODE_LEN, hwifi_get_country_code(), COUNTRY_CODE_LEN)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_regdomain_update_rf_param::memcpy fail!");
        return;
    }

    hwifi_set_country_code(pc_country, COUNTRY_CODE_LEN);
    /* 如果新的国家码和旧国家处于一个regdomain，不刷新RF参数，只更新国家码 */
    if (OAL_TRUE == hwifi_is_regdomain_changed((uint8_t *)old_pc_country, (uint8_t *)pc_country)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update_rf_param::regdomain changed, refresh rf param!}");

        /* 刷新参数失败，为了保证国家码和功率参数对应 */
        /* 将国家码设回原来的国家码，本次更新失败 */
        if (hwifi_force_refresh_rf_params(net_dev) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY,
                "{wal_regdomain_update_rf_param::refresh rf params failed. Set country back.!}");
            hwifi_set_country_code(old_pc_country, COUNTRY_CODE_LEN);
        }
    }
}
#endif

OAL_STATIC int32_t wal_regdomain_confirm(int8_t *pc_country, mac_regdomain_info_stru *pst_mac_regdom)
{
    if (!wal_is_alpha_upper(pc_country[0]) || !wal_is_alpha_upper(pc_country[1])) {
        if ((pc_country[0] == '9') && (pc_country[1] == '9')) {
            oam_info_log0(0, OAM_SF_ANY, "{wal_regdomain_confirm::set regdomain to 99!}");
        } else {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_confirm::country str invalid!}");
            return -OAL_EINVAL;
        }
    }
    return OAL_SUCC;
}


OAL_STATIC void wal_regdomain_update_hostapd_param(const oal_ieee80211_regdomain_stru *pst_regdom,
                                                           oal_net_device_stru *net_dev)
{
    mac_vap_stru *mac_vap = NULL;
    uint8_t uc_dev_id;
    mac_device_stru *pst_device = NULL;

    mac_vap = (mac_vap_stru *)oal_net_dev_priv(net_dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update_hostapd_param::OAL_NET_DEV_PRIV null.}");
        return;
    }

    uc_dev_id = mac_vap->uc_device_id;
    pst_device = mac_res_get_dev(uc_dev_id);
    if ((pst_device != NULL) && (pst_device->pst_wiphy != NULL)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0))
        
        wal_linux_update_wiphy_channel_list_num(net_dev, pst_device->pst_wiphy);
#endif

        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update_hostapd_param::update regdom to kernel.}");

        wal_cfg80211_reset_bands(uc_dev_id);
        oal_wiphy_apply_custom_regulatory(pst_device->pst_wiphy, pst_regdom);
        
        wal_cfg80211_save_bands(uc_dev_id);
    }
}


int32_t wal_selfstdy_regdomain_update(oal_net_device_stru *net_dev, int8_t *pc_country)
{
    const oal_ieee80211_regdomain_stru *pst_regdom;
    uint16_t us_size;
    mac_regdomain_info_stru *pst_mac_regdom = NULL;
    wal_msg_write_stru write_msg = {0};
    mac_cfg_country_stru *pst_param = NULL;
    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    int8_t old_pc_country[COUNTRY_CODE_LEN] = { '9', '9' };

    wal_regdomain_update_rf_param(net_dev, pc_country, old_pc_country);
#endif

    ret = wal_regdomain_confirm(pc_country, pst_mac_regdom);

    pst_regdom = wal_regdb_find_db(pc_country);
    if (pst_regdom == NULL) {
        return -OAL_EINVAL;
    }
    us_size = (uint16_t)(sizeof(mac_regclass_info_stru) * pst_regdom->n_reg_rules + MAC_RD_INFO_LEN);

    /* 申请内存存放管制域信息，将内存指针作为事件payload抛下去 */
    /* 此处申请的内存在事件处理函数释放(hmac_config_set_country) */
    pst_mac_regdom = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, us_size, OAL_TRUE);
    if (pst_mac_regdom == NULL) {
        return -OAL_ENOMEM;
    }

    wal_regdomain_fill_info(pst_regdom, pst_mac_regdom);

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_COUNTRY, sizeof(mac_cfg_country_stru));

    /* 填写WID对应的参数 */
    pst_mac_regdom->en_regdomain = hwifi_get_regdomain_from_country_code(pc_country);
    pst_param = (mac_cfg_country_stru *)(write_msg.auc_value);
    pst_param->p_mac_regdom = pst_mac_regdom;

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_country_stru),
                             (uint8_t *)&write_msg, OAL_FALSE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        if (oal_value_ne_all2(ret, -OAL_ETIMEDOUT, -OAL_EFAUL)) {
            oal_mem_free_m(pst_mac_regdom, OAL_TRUE);
            pst_mac_regdom = NULL;
        }
        if (rsp_msg != NULL) {
            oal_free(rsp_msg);
        }
        return ret;
    }
    oal_free(rsp_msg);

    /* 驱动不支持ACS时，更新hostapd管制域信息; 如果驱动支持ACS，则不需要更新，否则hostapd无法配置DFS信道 */
    wal_regdomain_update_hostapd_param(pst_regdom, net_dev);
    return OAL_SUCC;
}
#endif

int32_t wal_regdomain_update_for_dfs(oal_net_device_stru *net_dev, int8_t *pc_country)
{
    wal_msg_write_stru write_msg = {0};
    mac_dfs_domain_enum_uint8 *pst_param = NULL;
    int32_t ret;

    if (!wal_is_alpha_upper(pc_country[0]) || !wal_is_alpha_upper(pc_country[1])) {
        if ((pc_country[0] == '9') && (pc_country[1] == '9')) {
            oam_info_log0(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs::set regdomain to 99!}");
        } else {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs::country str is invalid!}");
            return -OAL_EINVAL;
        }
    }

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_COUNTRY_FOR_DFS, sizeof(mac_dfs_domain_enum_uint8));

    /* 填写WID对应的参数 */
    pst_param = (mac_dfs_domain_enum_uint8 *)(write_msg.auc_value);
    *pst_param = wal_get_dfs_domain(pc_country);

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_dfs_domain_enum_uint8),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs::err %d!}", ret);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t wal_regdomain_update_sta(uint8_t uc_vap_id)
{
    int8_t *pc_desired_country = NULL;

    oal_net_device_stru *net_dev = NULL;
    int32_t ret;
    oal_bool_enum_uint8 us_updata_rd_by_ie_switch = OAL_FALSE;

    hmac_vap_get_updata_rd_by_ie_switch(uc_vap_id, &us_updata_rd_by_ie_switch);

    if (us_updata_rd_by_ie_switch == OAL_TRUE) {
        pc_desired_country = hmac_vap_get_desired_country(uc_vap_id);
        if (oal_unlikely(pc_desired_country == NULL)) {
            oam_error_log0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::pc_desired_country null!}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 期望的国家码全为0，表示对端AP的国家码不存在，采用sta当前默认的国家码 */
        if ((pc_desired_country[0] == 0) && (pc_desired_country[1] == 0)) {
            oam_info_log0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::ap has no country ie,use default}");
            return OAL_SUCC;
        }

        net_dev = hmac_vap_get_net_device(uc_vap_id);
        if (oal_unlikely(net_dev == NULL)) {
            oam_error_log0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::net_dev null!}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        ret = wal_regdomain_update_for_dfs(net_dev, pc_desired_country);
        if (ret != OAL_SUCC) {
            oam_warning_log1(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::regdomain_update err:%d!}", ret);
            return OAL_FAIL;
        }

        ret = wal_regdomain_update(net_dev, pc_desired_country);
        if (ret != OAL_SUCC) {
            oam_warning_log1(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::regdomain_update err:%d}", ret);
            return OAL_FAIL;
        }
    } else {
        oam_info_log0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::us_updata_rd_by_ie_switch is fail!}");
    }
    return OAL_SUCC;
}


int32_t wal_regdomain_update_country_code(oal_net_device_stru *net_dev, int8_t *pc_country)
{
    int32_t ret;

    if (net_dev == NULL || pc_country == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_regdomain_update_country_code::null ptr!}");
        return -OAL_EFAIL;
    }

    /* 设置国家码到wifi 驱动 */
    ret = wal_regdomain_update_for_dfs(net_dev, pc_country);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_regdomain_update_country_code::update_for_dfs err [%d]!}", ret);
        return -OAL_EFAIL;
    }

    ret = wal_regdomain_update(net_dev, pc_country);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_regdomain_update_country_code::update err [%d]!}", ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY

int32_t wal_regdomain_update_selfstudy_country_code(oal_net_device_stru *net_dev, int8_t *pc_country)
{
    int32_t   ret;

    if (net_dev == NULL || pc_country == NULL) {
        oam_error_log0(0, OAM_SF_ANY,
            "{wal_regdomain_update_selfstudy_country_code::null ptr.}");
        return -OAL_EFAIL;
    }

    /* 设置国家码到wifi 驱动 */
    ret = wal_regdomain_update_for_dfs(net_dev, pc_country);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_regdomain_update_selfstudy_country_code::update_for_dfs return err code [%d]!}\r\n", ret);
        return -OAL_EFAIL;
    }

    ret = wal_selfstdy_regdomain_update(net_dev, pc_country);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_regdomain_update_selfstudy_country_code::update return err code [%d]!}\r\n", ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC void wal_regdomain_update_by_ap(uint8_t uc_vap_id)
{
    int8_t *pc_desired_country = NULL;
    oal_net_device_stru *net_dev = NULL;
    int32_t ret;
    oal_bool_enum_uint8 us_updata_rd_by_ie_switch = OAL_FALSE;

    /* 关联成功后，停止国家码自学习 */
    g_country_code_self_study_flag = OAL_FALSE;

    hmac_vap_get_updata_rd_by_ie_switch(uc_vap_id, &us_updata_rd_by_ie_switch);
    pc_desired_country = hmac_vap_get_desired_country(uc_vap_id);
    /* 若国家码为99，则以关联AP的国家码为准 */
    if (g_country_code_result[0] == '9' && g_country_code_result[1] == '9') {
        if (oal_unlikely(pc_desired_country == NULL)) {
            oam_error_log0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::pc_desired_country null!}");
            return;
        }

        /* 期望的国家码全为0，表示对端AP的国家码不存在，采用sta当前默认的国家码 */
        if ((pc_desired_country[0] == 0) && (pc_desired_country[1] == 0)) {
            oam_info_log0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::ap has no country ie,use default}");
        }

        net_dev = hmac_vap_get_net_device(uc_vap_id);
        if (oal_unlikely(net_dev == NULL)) {
            oam_error_log0(uc_vap_id, OAM_SF_ANY, "{wal_regdomain_update_sta::net_dev null!}");
        }

        ret = wal_regdomain_update_selfstudy_country_code(net_dev, pc_desired_country);
    }
}

void wal_selfstudy_regdomain_update_by_ap(frw_event_stru *event)
{
    if (g_cust_country_code_ignore_flag.en_country_code_study_ingore_hipriv_flag == OAL_FALSE) {
        wal_regdomain_update_by_ap(event->st_event_hdr.uc_vap_id);
    }
}
#endif
#endif

int32_t wal_set_random_mac_to_mib(oal_net_device_stru *net_dev)
{
    uint32_t result;
    frw_event_mem_stru *evt_mem = NULL;
    wal_msg_stru *cfg_msg = NULL;
    wal_msg_write_stru *write_msg = NULL;
    mac_cfg_staion_id_param_stru *pst_param = NULL;
    mac_vap_stru *mac_vap;
    uint8_t *puc_mac_addr = NULL;
    oal_wireless_dev_stru *pst_wdev = NULL; /* 对于P2P 场景，p2p0 和 p2p-p2p0 MAC 地址从wlan0 获取 */
    wlan_p2p_mode_enum_uint8 en_p2p_mode = WLAN_LEGACY_VAP_MODE;
    uint8_t *auc_wlan_addr = NULL;
    uint8_t *auc_p2p0_addr = NULL;
    int32_t ret = EOK;
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_set_random_mac_to_mib::mac_vap NULL}");
        return OAL_FAIL;
    }

    if (mac_vap->pst_mib_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_random_mac_to_mib::vap->mib_info is NULL !}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    auc_wlan_addr = mac_mib_get_StationID(mac_vap);
    auc_p2p0_addr = mac_mib_get_p2p0_dot11StationID(mac_vap);

    pst_wdev = net_dev->ieee80211_ptr;
    switch (pst_wdev->iftype) {
        case NL80211_IFTYPE_P2P_DEVICE:
            en_p2p_mode = WLAN_P2P_DEV_MODE;
            /* 产生P2P device MAC 地址，将本地mac 地址bit 设置为1 */
            ret += memcpy_s(auc_p2p0_addr, WLAN_MAC_ADDR_LEN, oal_netdevice_mac_addr(net_dev), WLAN_MAC_ADDR_LEN);

            oam_warning_log3(0, OAM_SF_ANY, "wal_set_random_mac_to_mib:p2p0 mac is:%02X:XX:XX:XX:%02X:%02X\n",
                auc_p2p0_addr[0], auc_p2p0_addr[4], auc_p2p0_addr[5]); /* 4 5 macaddr */
            break;
        case NL80211_IFTYPE_P2P_CLIENT:
        case NL80211_IFTYPE_P2P_GO:
            en_p2p_mode = (pst_wdev->iftype == NL80211_IFTYPE_P2P_GO) ? WLAN_P2P_GO_MODE : WLAN_P2P_CL_MODE;
            /* 根据上层下发值，产生P2P interface MAC 地址 */
            /* 上层不下发，跟随主mac地址,在wal_cfg80211_add_p2p_interface_init初始化 */
            ret += memcpy_s(auc_wlan_addr, WLAN_MAC_ADDR_LEN, oal_netdevice_mac_addr(net_dev), WLAN_MAC_ADDR_LEN);
            break;
        default:
            if (0 == (oal_strcmp("p2p0", net_dev->name))) {
                en_p2p_mode = WLAN_P2P_DEV_MODE;
                /* 产生P2P device MAC 地址，将本地mac 地址bit 设置为1 */
                ret += memcpy_s(auc_p2p0_addr, WLAN_MAC_ADDR_LEN, oal_netdevice_mac_addr(net_dev), WLAN_MAC_ADDR_LEN);
                break;
            }

            en_p2p_mode = WLAN_LEGACY_VAP_MODE;
            ret += memcpy_s(auc_wlan_addr, WLAN_MAC_ADDR_LEN,
                            oal_netdevice_mac_addr(net_dev), WLAN_MAC_ADDR_LEN);
            break;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{wal_set_random_mac_to_mib::p2p mode [%d]}", en_p2p_mode);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_set_random_mac_to_mib::memcpy fail!");
        return OAL_FAIL;
    }

    /* send the random mac to dmac */
    /* 抛事件到wal层处理   copy from wal_netdev_set_mac_addr()
       gong TBD : 改为调用通用的config接口
    */
    result = wal_alloc_cfg_event(net_dev, &evt_mem, NULL, &cfg_msg,
        (WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_staion_id_param_stru))); /* 申请事件 */
    if (oal_unlikely(result != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_set_random_mac_to_mib() fail; return %d!}", result);
        return -OAL_ENOMEM;
    }

    /* 填写配置消息 */
    WAL_CFG_MSG_HDR_INIT(&(cfg_msg->st_msg_hdr),
                         WAL_MSG_TYPE_WRITE,
                         WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_staion_id_param_stru),
                         WAL_GET_MSG_SN());

    /* 填写WID消息 */
    write_msg = (wal_msg_write_stru *)cfg_msg->auc_msg_data;
    WAL_WRITE_MSG_HDR_INIT(write_msg, WLAN_CFGID_STATION_ID, sizeof(mac_cfg_staion_id_param_stru));

    pst_param = (mac_cfg_staion_id_param_stru *)write_msg->auc_value; /* 填写WID对应的参数 */
    /* 如果使能P2P，需要将netdevice 对应的P2P 模式在配置参数中配置到hmac 和dmac */
    /* 以便底层识别配到p2p0 或p2p-p2p0 cl */
    pst_param->en_p2p_mode = en_p2p_mode;
    if (en_p2p_mode == WLAN_P2P_DEV_MODE) {
        puc_mac_addr = mac_mib_get_p2p0_dot11StationID(mac_vap);
    } else {
        puc_mac_addr = mac_mib_get_StationID(mac_vap);
    }
    oal_set_mac_addr(pst_param->auc_station_id, puc_mac_addr);

    frw_event_dispatch_event(evt_mem); /* 分发事件 */
    frw_event_free_m(evt_mem);

    return OAL_SUCC;
}


uint32_t wal_hipriv_setcountry(oal_net_device_stru *net_dev, int8_t *pc_param)
{
#ifdef _PRE_WLAN_FEATURE_11D
    int32_t ret;
    uint32_t result;
    uint32_t off_set = 0;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    int8_t *puc_para = NULL;

    /* 设备在up状态不允许配置，必须先down */
    if (0 != (OAL_IFF_RUNNING & oal_netdevice_flags(net_dev))) {
        oam_info_log1(0, OAM_SF_ANY, "{wal_hipriv_setcountry::net_dev flags: %d!}", oal_netdevice_flags(net_dev));
        return OAL_EBUSY;
    }

    /* 获取国家码字符串 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_setcountry::get cmd one arg err[%d]!}", result);
        return result;
    }
    puc_para = &ac_arg[0];

    ret = wal_regdomain_update_for_dfs(net_dev, puc_para);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_setcountry::regdomain_update err %d!}", ret);
        return (uint32_t)ret;
    }
    ret = wal_regdomain_update(net_dev, puc_para);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_setcountry::regdomain_update err %d!}", ret);
        return (uint32_t)ret;
    }
#else
    oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_setcountry::_PRE_WLAN_FEATURE_11D is not define!}");
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    g_cust_country_code_ignore_flag.en_country_code_ingore_hipriv_flag = OAL_TRUE;
#endif

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_getcountry(oal_net_device_stru *net_dev, int8_t *pc_param)
{
#ifdef _PRE_WLAN_FEATURE_11D
    wal_msg_stru *rsp_msg = NULL;
    wal_msg_rsp_stru *pst_query_rsp_msg = NULL;
    wal_msg_query_stru st_query_msg;
    int8_t ac_tmp_buff[OAM_PRINT_FORMAT_LENGTH];
    int32_t ret;

    /* 抛事件到wal层处理 */
    st_query_msg.en_wid = WLAN_CFGID_COUNTRY;

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_QUERY, WAL_MSG_WID_LENGTH,
                             (uint8_t *)&st_query_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        if (rsp_msg != NULL) {
            oal_free(rsp_msg);
        }

        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_getcountry::wal_alloc_cfg_event err %d!}", ret);
        return (uint32_t)ret;
    }

    /* 处理返回消息 */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(rsp_msg->auc_msg_data);

    ret = snprintf_s(ac_tmp_buff, sizeof(ac_tmp_buff), sizeof(ac_tmp_buff) - 1,
                     "getcountry code is : %c%c.\n",
                     pst_query_rsp_msg->auc_value[0],
                     pst_query_rsp_msg->auc_value[1]);
    if (ret < 0) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_list_sta::snprintf_s error!");
        oal_free(rsp_msg);
        oam_print(ac_tmp_buff);
        return OAL_FAIL;
    }

    oam_warning_log3(0, OAM_SF_CFG, "{wal_hipriv_getcountry:: %c, %c, len %d}",
                     pst_query_rsp_msg->auc_value[0],
                     pst_query_rsp_msg->auc_value[1],
                     pst_query_rsp_msg->us_len);

    oal_free(rsp_msg);
    oam_print(ac_tmp_buff);

#else
    oam_info_log0(0, OAM_SF_ANY, "{wal_hipriv_getcountry::_PRE_WLAN_FEATURE_11D is not define!}");
#endif

    return OAL_SUCC;
}

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
void wal_vap_get_wlan_mode_para(wal_msg_write_stru *write_msg)
{
    mac_cfg_add_vap_param_stru *vap_param = (mac_cfg_add_vap_param_stru *)write_msg->auc_value;

    vap_param->bit_11ac2g_enable = wlan_chip_get_11ac2g_enable();

    vap_param->bit_disable_capab_2ght40 = g_wlan_cust.uc_disable_capab_2ght40;
}
#endif

OAL_STATIC uint32_t wal_hipriv_register_netdev(oal_net_device_stru *net_dev, mac_device_stru *mac_device,
    wlan_p2p_mode_enum_uint8 p2p_mode)
{
    uint32_t result;
    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;
    mac_vap_stru *mac_vap = NULL;
    wal_msg_write_stru write_msg = {0};

    if (wal_set_random_mac_to_mib(net_dev) != OAL_SUCC) {
        return OAL_FAIL;
    } /* set random mac to mib ; for hi1102-cb */

    /* 设置netdevice的MAC地址，MAC地址在HMAC层被初始化到MIB中 */
    mac_vap = oal_net_dev_priv(net_dev);
    if (WLAN_P2P_CL_MODE != p2p_mode) {
        mac_vap->en_vap_state = MAC_VAP_STATE_INIT;
    }

    if (p2p_mode == WLAN_P2P_DEV_MODE) {
        oal_set_mac_addr((uint8_t *)oal_netdevice_mac_addr(net_dev), mac_mib_get_p2p0_dot11StationID(mac_vap));
        mac_device->st_p2p_info.uc_p2p0_vap_idx = mac_vap->uc_vap_id;
    } else {
        oal_set_mac_addr((uint8_t *)oal_netdevice_mac_addr(net_dev), mac_mib_get_StationID(mac_vap));
    }

    /* 注册net_device */
    result = (uint32_t)hmac_net_register_netdev(net_dev);
    if (oal_unlikely(result != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_hipriv_add_vap::hmac_net_register_netdev return error code %d!}", result);

        /* 抛删除vap事件释放刚申请的vap */
        WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_DEL_VAP, sizeof(mac_cfg_del_vap_param_stru));

        ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
            WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_del_vap_param_stru),
            (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
        if (OAL_SUCC != wal_check_and_release_msg_resp(rsp_msg)) {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_add_vap::wal_check_and_release_msg_resp fail.}");
        }
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_add_vap::wal_send_cfg_event fail,err code %d!}", ret);
        }
        return result;
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_add_vap_event_process(oal_net_device_stru *cfg_net_dev, oal_net_device_stru *net_dev,
    mac_vap_stru *cfg_mac_vap, wlan_p2p_mode_enum_uint8 p2p_mode, wlan_vap_mode_enum_uint8 vap_mode)
{
    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;
    uint32_t err_code;
    wal_msg_write_stru write_msg = {0};

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_ADD_VAP, sizeof(mac_cfg_add_vap_param_stru));
    ((mac_cfg_add_vap_param_stru *)&write_msg.auc_value)->pst_net_dev = net_dev;
    ((mac_cfg_add_vap_param_stru *)&write_msg.auc_value)->en_vap_mode = vap_mode;
    ((mac_cfg_add_vap_param_stru *)&write_msg.auc_value)->uc_cfg_vap_indx = cfg_mac_vap->uc_vap_id;
    ((mac_cfg_add_vap_param_stru *)&write_msg.auc_value)->en_p2p_mode = p2p_mode;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wal_vap_get_wlan_mode_para(&write_msg);
#endif
    /* 发送消息 */
    ret = wal_send_cfg_event(cfg_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_add_vap_param_stru),
        (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_add_vap::err %d!}", ret);
        return (uint32_t)ret;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(cfg_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_hipriv_add_vap::hmac add vap fail[%u]!}", err_code);
        return err_code;
    }
    return OAL_SUCC;
}

OAL_STATIC oal_wireless_dev_stru *wal_hipriv_alloc_wdev(oal_net_device_stru *net_dev, mac_device_stru *mac_device,
    wlan_p2p_mode_enum_uint8 p2p_mode, wlan_vap_mode_enum_uint8 vap_mode)
{
    oal_wireless_dev_stru *wdev = NULL;
    wdev = (oal_wireless_dev_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL,
        sizeof(oal_wireless_dev_stru), OAL_FALSE);
    if (oal_unlikely(wdev == NULL)) {
        return NULL;
    }

    memset_s(wdev, sizeof(oal_wireless_dev_stru), 0, sizeof(oal_wireless_dev_stru));
    wdev->netdev = net_dev;
    if (vap_mode == WLAN_VAP_MODE_BSS_AP) {
        wdev->iftype = NL80211_IFTYPE_AP;
    } else if (vap_mode == WLAN_VAP_MODE_BSS_STA) {
        wdev->iftype = NL80211_IFTYPE_STATION;
    }
    if (p2p_mode == WLAN_P2P_DEV_MODE) {
        wdev->iftype = NL80211_IFTYPE_P2P_DEVICE;
    } else if (p2p_mode == WLAN_P2P_CL_MODE) {
        wdev->iftype = NL80211_IFTYPE_P2P_CLIENT;
    } else if (p2p_mode == WLAN_P2P_GO_MODE) {
        wdev->iftype = NL80211_IFTYPE_P2P_GO;
    }

    wdev->wiphy = mac_device->pst_wiphy;
    return wdev;
}


OAL_STATIC uint32_t wal_hipriv_add_vap(oal_net_device_stru *cfg_net_dev, int8_t *pc_param)
{
    oal_net_device_stru *net_dev = NULL;
    uint32_t result;
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    int8_t ac_mode[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wlan_vap_mode_enum_uint8 vap_mode;

    oal_wireless_dev_stru *wdev = NULL;
    mac_vap_stru *cfg_mac_vap = NULL;
    mac_device_stru *mac_device = NULL;
    wlan_p2p_mode_enum_uint8 p2p_mode = WLAN_LEGACY_VAP_MODE;

    /* 获取mac device */
    cfg_mac_vap = oal_net_dev_priv(cfg_net_dev);
    mac_device = mac_res_get_dev(cfg_mac_vap->uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* pc_param指向新创建的net_device的name, 将其取出存放到ac_name中 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_add_vap::wal_get_cmd_one_arg err_code %d!}", result);
        return result;
    }

    /* ac_name length不应超过OAL_IF_NAME_SIZE */
    if (OAL_IF_NAME_SIZE <= OAL_STRLEN(ac_name)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_add_vap:: vap name overlength is %d!}", OAL_STRLEN(ac_name));
        /* 输出错误的vap name信息 */
        oal_print_hex_dump((uint8_t *)ac_name, OAL_IF_NAME_SIZE, HEX_DUMP_GROUP_SIZE, "vap name lengh is overlong:");
        return OAL_FAIL;
    }

    pc_param += off_set;

    /* pc_param 指向'ap|sta', 将其取出放到ac_mode中 */
    result = wal_get_cmd_one_arg(pc_param, ac_mode, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        return (uint32_t)result;
    }

    /* 解析ac_mode字符串对应的模式 */
    if (0 == (oal_strcmp("ap", ac_mode))) {
        vap_mode = WLAN_VAP_MODE_BSS_AP;
    } else if (0 == (oal_strcmp("sta", ac_mode))) {
        vap_mode = WLAN_VAP_MODE_BSS_STA;
    } else if (0 == (oal_strcmp("p2p_device", ac_mode))) { /* 创建P2P 相关VAP */
        vap_mode = WLAN_VAP_MODE_BSS_STA;
        p2p_mode = WLAN_P2P_DEV_MODE;
    } else if (0 == (oal_strcmp("p2p_cl", ac_mode))) {
        vap_mode = WLAN_VAP_MODE_BSS_STA;
        p2p_mode = WLAN_P2P_CL_MODE;
    } else if (0 == (oal_strcmp("p2p_go", ac_mode))) {
        vap_mode = WLAN_VAP_MODE_BSS_AP;
        p2p_mode = WLAN_P2P_GO_MODE;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_add_vap::the mode param is invalid!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 如果创建的net device已经存在，直接返回 */
    /* 根据dev_name找到dev */
    net_dev = wal_config_get_netdev(ac_name, OAL_STRLEN(ac_name));
    if (net_dev != NULL) {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(net_dev);
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_add_vap::the net_device is already exist!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    net_dev = oal_net_alloc_netdev_mqs(sizeof(oal_netdev_priv_stru), ac_name,
        oal_ether_setup, WLAN_NET_QUEUE_BUTT, 1); /* 此函数第一个入参代表私有长度，此处不涉及为0 */
    if (oal_unlikely(net_dev == NULL)) {
        oam_warning_log0(cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_add_vap::net_dev null ptr error!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wdev = wal_hipriv_alloc_wdev(net_dev, mac_device, p2p_mode, vap_mode);
    if (wdev == NULL) {
        oal_net_free_netdev(net_dev);
        oam_warning_log0(cfg_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_add_vap::wdev null ptr error!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 对netdevice进行赋值 */
#ifdef CONFIG_WIRELESS_EXT
    net_dev->wireless_handlers = &g_st_iw_handler_def;
#endif /* CONFIG_WIRELESS_EXT */
    net_dev->netdev_ops = &g_st_wal_net_dev_ops;

    oal_netdevice_destructor(net_dev) = oal_net_free_netdev;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 44))
    oal_netdevice_master(net_dev) = NULL;
#endif

    oal_netdevice_ifalias(net_dev) = NULL;
    oal_netdevice_watchdog_timeo(net_dev) = 5;
    oal_netdevice_wdev(net_dev) = wdev;
    oal_netdevice_qdisc(net_dev, NULL);

    oal_netdevice_flags(net_dev) &= ~OAL_IFF_RUNNING; /* 将net device的flag设为down */

    result = wal_hipriv_add_vap_event_process(cfg_net_dev, net_dev, cfg_mac_vap, p2p_mode, vap_mode);
    if (result != OAL_SUCC) {
        /* 异常处理，释放内存 */
        oal_mem_free_m(wdev, OAL_FALSE);
        oal_net_free_netdev(net_dev);
        return result;
    }
    if ((p2p_mode == WLAN_LEGACY_VAP_MODE) && (mac_device->st_p2p_info.pst_primary_net_device == NULL)) {
        /* 如果创建wlan0， 则保存wlan0 为主net_device,p2p0 和p2p-p2p0 MAC 地址从主netdevice 获取 */
        mac_device->st_p2p_info.pst_primary_net_device = net_dev;
    }

    result = wal_hipriv_register_netdev(net_dev, mac_device, p2p_mode);
    if (result != OAL_SUCC) {
        /* 异常处理，释放内存 */
        oal_mem_free_m(wdev, OAL_FALSE);
        oal_net_free_netdev(net_dev);
        return result;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_del_vap_config(oal_net_device_stru *net_dev)
{
    wal_msg_write_stru write_msg = {0};
    wlan_p2p_mode_enum_uint8 p2p_mode;
    wal_msg_stru *rsp_msg = NULL;
    oal_wireless_dev_stru *wdev = NULL;
    int32_t ret;

    memset_s(&write_msg, sizeof(wal_msg_write_stru), 0, sizeof(wal_msg_write_stru));

    ((mac_cfg_del_vap_param_stru *)write_msg.auc_value)->pst_net_dev = net_dev;
    wdev = net_dev->ieee80211_ptr;
    p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(wdev->iftype);
    if (p2p_mode == WLAN_P2P_BUTT) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_del_vap_config::wal_wireless_iftype_to_mac_p2p_mode return BUFF}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    ((mac_cfg_del_vap_param_stru *)write_msg.auc_value)->en_p2p_mode = p2p_mode;

    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_DEL_VAP, sizeof(mac_cfg_del_vap_param_stru));

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_del_vap_param_stru),
        (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (wal_check_and_release_msg_resp(rsp_msg) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_del_vap_config::wal_check_and_release_msg_resp fail}");
    }

    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_del_vap_config::err %d}", ret);
        /* 去注册 */
        oal_net_unregister_netdev(net_dev);
        oal_mem_free_m(wdev, OAL_TRUE);
        return (uint32_t)ret;
    }

    /* 去注册 */
    oal_net_unregister_netdev(net_dev);
    oal_mem_free_m(wdev, OAL_TRUE);

    return OAL_SUCC;
}


uint32_t wal_hipriv_del_vap(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    uint32_t ret;
    mac_vap_stru *mac_vap = NULL;
    if (oal_unlikely(oal_any_null_ptr2(net_dev, pc_param))) {
        // 访问网络接口的模块可能不止一个,需要上层保证可靠删除
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_del_vap::net_dev or pc_param is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设备在up状态不允许删除，必须先down */
    if (oal_unlikely(0 != (OAL_IFF_RUNNING & oal_netdevice_flags(net_dev)))) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_del_vap::dev busy, down it first:%d}",
            oal_netdevice_flags(net_dev));
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_hipriv_del_vap::can't get mac vap from netdevice priv data!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_hipriv_del_vap::invalid para,cfg vap!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 设备在up状态不允许删除，必须先down */
    if ((mac_vap->en_vap_state != MAC_VAP_STATE_INIT) && (mac_vap->en_p2p_mode != WLAN_P2P_CL_MODE)) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_del_vap::device busy, down it first %d!}", mac_vap->en_vap_state);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    /* 发消息到wal层处理 */
    ret = wal_hipriv_del_vap_config(net_dev);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_del_vap::send msg fail!}");
        return ret;
    }

    return OAL_SUCC;
}


uint32_t wal_hipriv_vap_info(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;

    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_VAP_INFO, sizeof(int32_t));

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_vap_info::err %d!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}

#define MAX_HIPRIV_IP_FILTER_BTABLE_SIZE 129

OAL_STATIC uint32_t wal_ipriv_ip_filter_items(wal_hw_wifi_filter_item *pst_items,
    uint32_t item_size, int32_t l_count)
{
    if (item_size > sizeof(wal_hw_wifi_filter_item) * MAX_HIPRIV_IP_FILTER_BTABLE_SIZE) {
        return OAL_FAIL;
    }

    return (uint32_t)wal_add_ip_filter_items(pst_items, l_count);
}


uint32_t wal_hipriv_set_ip_filter(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    int32_t l_items_cnt;
    int32_t l_items_idx;
    int32_t l_enable;
    uint32_t ret;
    uint32_t off_set = 0;

    int8_t ac_cmd[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    int8_t ac_cmd_param[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    wal_hw_wifi_filter_item ast_items[MAX_HIPRIV_IP_FILTER_BTABLE_SIZE];

    l_enable = 0;
    l_items_cnt = 0;
    memset_s((uint8_t *)ast_items, sizeof(wal_hw_wifi_filter_item) * MAX_HIPRIV_IP_FILTER_BTABLE_SIZE,
             0, sizeof(wal_hw_wifi_filter_item) * MAX_HIPRIV_IP_FILTER_BTABLE_SIZE);

    /* 其取出子命令 */
    ret = wal_get_cmd_one_arg(pc_param, ac_cmd, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter::wal_get_cmd_one_arg err_code:%d}", ret);
        return ret;
    }

    if (0 == oal_strncmp(ac_cmd, CMD_CLEAR_RX_FILTERS, OAL_STRLEN(CMD_CLEAR_RX_FILTERS))) {
        /* 清理表单 */
        ret = (uint32_t)wal_clear_ip_filter();
        return ret;
    }

    pc_param += off_set;
    ret = wal_get_cmd_one_arg(pc_param, ac_cmd_param, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter::get cmd_param err %d!}", ret);
        return ret;
    }

    if (0 == oal_strncmp(ac_cmd, CMD_SET_RX_FILTER_ENABLE, OAL_STRLEN(CMD_SET_RX_FILTER_ENABLE))) {
        /* 使能/关闭功能 */
        l_enable = oal_atoi(ac_cmd_param);
        ret = (uint32_t)wal_set_ip_filter_enable(l_enable);
        return ret;
    } else if (0 == oal_strncmp(ac_cmd, CMD_ADD_RX_FILTER_ITEMS, OAL_STRLEN(CMD_ADD_RX_FILTER_ITEMS))) {
        /* 更新黑名单 */
        /* 获取名单条目数 */
        l_items_cnt = oal_atoi(ac_cmd_param);
        l_items_cnt = oal_min(MAX_HIPRIV_IP_FILTER_BTABLE_SIZE, l_items_cnt);
        /* 获取名单条目 */
        for (l_items_idx = 0; l_items_idx < l_items_cnt; l_items_idx++) {
            /* 获取protocol X */
            pc_param += off_set;
            ret = wal_get_cmd_one_arg(pc_param, ac_cmd_param, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter::get item_params err:%d!}", ret);
                return ret;
            }
            ast_items[l_items_idx].protocol = (uint8_t)oal_atoi(ac_cmd_param);

            /* 获取portX */
            pc_param += off_set;
            ret = wal_get_cmd_one_arg(pc_param, ac_cmd_param, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter::get item_params  err: %d!}", ret);
                return ret;
            }
            ast_items[l_items_idx].port = (uint16_t)oal_atoi(ac_cmd_param);
        }

        ret = wal_ipriv_ip_filter_items(ast_items, sizeof(ast_items), l_items_cnt);
        return ret;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter::cmd_one_arg no support!}");
        return OAL_FAIL;
    }
}


OAL_STATIC uint32_t wal_hipriv_rx_fcs_info(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t ret;
    uint32_t result;
    mac_cfg_rx_fcs_info_stru *pst_rx_fcs_info = NULL;
    mac_cfg_rx_fcs_info_stru st_rx_fcs_info; /* 临时保存获取的use的信息 */

    /* 打印接收帧的FCS正确与错误信息:sh hipriv.sh "vap0 rx_fcs_info 0/1 1-4" 0/1  0代表不清除，1代表清除 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info::wal_get_cmd_one_arg  err_code %d!}", result);
        return result;
    }

    st_rx_fcs_info.data_op = (uint32_t)oal_atoi(ac_name);

    if (st_rx_fcs_info.data_op > 1) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info:: data_op cmd is error!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info::get cmd one arg err%d!}", result);
        return result;
    }

    st_rx_fcs_info.print_info = (uint32_t)oal_atoi(ac_name);

    if (st_rx_fcs_info.print_info > 4) { // 4代表参数数值上限
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info:: print_info cmd is error!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_RX_FCS_INFO, sizeof(mac_cfg_rx_fcs_info_stru));

    /* 设置配置命令参数 */
    pst_rx_fcs_info = (mac_cfg_rx_fcs_info_stru *)(write_msg.auc_value);
    pst_rx_fcs_info->data_op = st_rx_fcs_info.data_op;
    pst_rx_fcs_info->print_info = st_rx_fcs_info.print_info;

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_rx_fcs_info_stru),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info::err %d!}", ret);
        return (uint32_t)ret;
    }
    return OAL_SUCC;
}



OAL_STATIC uint32_t wal_hipriv_vap_log_level(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    mac_vap_stru *mac_vap = NULL;
    oam_log_level_enum_uint8 en_level_val;
    uint32_t off_set = 0;
    int8_t ac_param[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;

    wal_msg_write_stru write_msg = {0};

    /* OAM log模块的开关的命令: hipriv "Hisilicon0[vapx] log_level {1/2}"
       1-2(error与warning)级别日志以vap级别为维度；
    */
    mac_vap = oal_net_dev_priv(net_dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_vap_log_level::null pointer.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取日志级别 */
    ret = wal_get_cmd_one_arg(pc_param, ac_param, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }

    en_level_val = (oam_log_level_enum_uint8)oal_atoi(ac_param);
    if ((en_level_val < OAM_LOG_LEVEL_ERROR) || (en_level_val > OAM_LOG_LEVEL_INFO)) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_feature_log_level::invalid switch value[%d].}", en_level_val);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* hipriv "Hisilicon0 log_level 1|2|3" 设置所有vip id的log */
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if (0 == oal_strcmp("Hisilicon0", net_dev->name)) {
        hmac_config_set_all_log_level(mac_vap, sizeof(oam_log_level_enum_uint8),
            (uint8_t *)&en_level_val);
        return OAL_SUCC;
    }
#endif

    oam_log_set_vap_level(mac_vap->uc_vap_id, en_level_val);

    // 目前支持02 device 设置log 级别， 遗留后续的合并问题
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_LOG_LEVEL, sizeof(int32_t));
    *((int32_t *)(write_msg.auc_value)) = en_level_val;
    ret |= (uint32_t)wal_send_cfg_event(net_dev,
                                        WAL_MSG_TYPE_WRITE,
                                        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t),
                                        (uint8_t *)&write_msg,
                                        OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_vap_log_level::err[%d]!}", ret);
        return ret;
    }

    return ret;
}


OAL_STATIC uint32_t wal_hipriv_add_user(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t result;
    int32_t ret;
    mac_cfg_add_user_param_stru *pst_add_user_param = NULL;
    mac_cfg_add_user_param_stru st_add_user_param; /* 临时保存获取的use的信息 */
    uint32_t get_addr_idx;

    /*
        设置添加用户的配置命令: hipriv "vap0 add_user xx xx xx xx xx xx(mac地址) 0 | 1(HT能力位) "
        该命令针对某一个VAP
    */
    /* 获取mac地址 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_add_user::get cmd one arg err[%d]!}", result);
        return result;
    }

    memset_s((uint8_t *)&st_add_user_param, sizeof(st_add_user_param), 0, sizeof(st_add_user_param));
    oal_strtoaddr(ac_name, sizeof(ac_name), st_add_user_param.auc_mac_addr, WLAN_MAC_ADDR_LEN);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    /* 获取用户的HT标识 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_add_user::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 针对解析出的不同命令，对user的HT字段进行不同的设置 */
    if (0 == (oal_strcmp("0", ac_name))) {
        st_add_user_param.en_ht_cap = 0;
    } else if (0 == (oal_strcmp("1", ac_name))) {
        st_add_user_param.en_ht_cap = 1;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_add_user::the mod switch cmd error!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_ADD_USER, sizeof(mac_cfg_add_user_param_stru));

    /* 设置配置命令参数 */
    pst_add_user_param = (mac_cfg_add_user_param_stru *)(write_msg.auc_value);
    for (get_addr_idx = 0; get_addr_idx < WLAN_MAC_ADDR_LEN; get_addr_idx++) {
        pst_add_user_param->auc_mac_addr[get_addr_idx] = st_add_user_param.auc_mac_addr[get_addr_idx];
    }
    pst_add_user_param->en_ht_cap = st_add_user_param.en_ht_cap;

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_add_user_param_stru),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_add_user::err[%d]!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_del_user(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t result;
    int32_t ret;
    mac_cfg_del_user_param_stru *pst_del_user_param = NULL;
    mac_cfg_del_user_param_stru st_del_user_param; /* 临时保存获取的use的信息 */
    uint32_t get_addr_idx;

    /*
        设置删除用户的配置命令: hipriv "vap0 del_user xx xx xx xx xx xx(mac地址)"
        该命令针对某一个VAP
    */
    /* 获取mac地址 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_del_user::get cmd one arg err[%d]!}", result);
        return result;
    }

    memset_s((uint8_t *)&st_del_user_param, sizeof(st_del_user_param), 0, sizeof(st_del_user_param));
    oal_strtoaddr(ac_name, sizeof(ac_name), st_del_user_param.auc_mac_addr, WLAN_MAC_ADDR_LEN);

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_DEL_USER, sizeof(mac_cfg_add_user_param_stru));

    /* 设置配置命令参数 */
    pst_del_user_param = (mac_cfg_add_user_param_stru *)(write_msg.auc_value);
    for (get_addr_idx = 0; get_addr_idx < WLAN_MAC_ADDR_LEN; get_addr_idx++) {
        pst_del_user_param->auc_mac_addr[get_addr_idx] = st_del_user_param.auc_mac_addr[get_addr_idx];
    }

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_add_user_param_stru),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_del_user::err[%d]!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_user_info(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    mac_vap_stru *mac_vap = NULL;
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    mac_cfg_user_info_param_stru *pst_user_info_param = NULL;
    uint8_t auc_mac_addr[NUM_6_BITS] = { 0 }; /* 临时保存获取的use的mac地址信息 */
    uint8_t uc_char_index;
    uint16_t us_user_idx;

    /* 去除字符串的空格 */
    pc_param++;

    /* 获取mac地址,16进制转换 */
    for (uc_char_index = 0; uc_char_index < 12; uc_char_index++) { // 12代表mac地址字符串长度
        if (*pc_param == ':') {
            pc_param++;
            if (uc_char_index != 0) {
                uc_char_index--;
            }

            continue;
        }

        auc_mac_addr[uc_char_index >> BIT_OFFSET_1] = (uint8_t)(((auc_mac_addr[uc_char_index >> BIT_OFFSET_1]
            << BIT_OFFSET_4) * (uc_char_index % MODULO_DIG_2)) + oal_strtohex(pc_param));
        pc_param++;
    }

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_USER_INFO, sizeof(mac_cfg_user_info_param_stru));

    /* 根据mac地址找用户 */
    mac_vap = oal_net_dev_priv(net_dev);

    ret = (int32_t)mac_vap_find_user_by_macaddr(mac_vap, auc_mac_addr, &us_user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_user_info::no such user!}");
        return OAL_FAIL;
    }

    /* 设置配置命令参数 */
    pst_user_info_param = (mac_cfg_user_info_param_stru *)(write_msg.auc_value);
    pst_user_info_param->us_user_idx = us_user_idx;

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_user_info_param_stru),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_user_info::err [%d]!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}


uint32_t wal_hipriv_set_mcast_data_dscr_param(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;
    mac_cfg_set_dscr_param_stru *pst_set_dscr_param = NULL;
    wal_dscr_param_enum_uint8 en_param_index;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_DSCR, sizeof(mac_cfg_set_dscr_param_stru));

    /* 解析并设置配置命令参数 */
    pst_set_dscr_param = (mac_cfg_set_dscr_param_stru *)(write_msg.auc_value);

    /* 获取描述符字段设置命令字符串 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcast_data_dscr_param::get cmd one arg err[%d]!}", result);
        return result;
    }
    pc_param += off_set;

    /* 解析是设置哪一个字段 */
    for (en_param_index = 0; en_param_index < WAL_DSCR_PARAM_BUTT; en_param_index++) {
        if (!oal_strcmp(g_pauc_tx_dscr_param_name[en_param_index], ac_arg)) {
            break;
        }
    }

    /* 检查命令是否打错 */
    if (en_param_index == WAL_DSCR_PARAM_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_mcast_data_dscr_param::no such param for tx dscr!}");
        return OAL_FAIL;
    }

    pst_set_dscr_param->uc_function_index = en_param_index;

    /* 配置速率、空间流数、带宽 */
    if (en_param_index >= WAL_DSCR_PARAM_RATE && en_param_index <= WAL_DSCR_PARAM_BW) {
        result = wal_hipriv_process_rate_params(net_dev, pc_param, pst_set_dscr_param);
        if (result != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcast_data_dscr_param::ucast_params err[%d]}", result);
            return result;
        }
    } else {
        /* 解析要设置为多大的值 */
        result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (result != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcast_data_dscr_param::get cmd one arg err[%d]}", result);
            return result;
        }
        pc_param += off_set;
        pst_set_dscr_param->l_value = oal_strtol(ac_arg, NULL, 0);
    }

    /* 组播数据帧描述符设置 tpye = MAC_VAP_CONFIG_MCAST_DATA */
    pst_set_dscr_param->en_type = MAC_VAP_CONFIG_MCAST_DATA;

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_set_dscr_param_stru),
                             (uint8_t *)&write_msg,
                             OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcast_data_dscr_param::err [%d]!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}


uint32_t wal_hipriv_set_rate(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;
    mac_cfg_non_ht_rate_stru *pst_set_rate_param = NULL;
    wlan_legacy_rate_value_enum_uint8 en_rate_index;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wal_msg_stru *rsp_msg = NULL;

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_RATE, sizeof(mac_cfg_non_ht_rate_stru));

    /* 解析并设置配置命令参数 */
    pst_set_rate_param = (mac_cfg_non_ht_rate_stru *)(write_msg.auc_value);

    /* 获取速率值字符串 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_rate::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 解析是设置为哪一级速率 */
    for (en_rate_index = 0; en_rate_index < WLAN_LEGACY_RATE_VALUE_BUTT; en_rate_index++) {
        if (!oal_strcmp(g_pauc_non_ht_rate_tbl[en_rate_index], ac_arg)) {
            break;
        }
    }

    /* 解析要设置为多大的值 */
    pst_set_rate_param->en_rate = en_rate_index;

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_non_ht_rate_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_rate::err [%d]!}", ret);
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_rate fail, err code[%u]!}", result);
        return result;
    }
    return OAL_SUCC;
}


uint32_t wal_hipriv_set_mcs(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;
    mac_cfg_tx_comp_stru *pst_set_mcs_param = NULL;
    int32_t l_mcs;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t l_idx = 0;
    wal_msg_stru *rsp_msg = NULL;

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_MCS, sizeof(mac_cfg_tx_comp_stru));

    /* 解析并设置配置命令参数 */
    pst_set_mcs_param = (mac_cfg_tx_comp_stru *)(write_msg.auc_value);

    /* 获取速率值字符串 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcs::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 输入命令合法性检测 */
    while (ac_arg[l_idx] != '\0') {
        if (isdigit(ac_arg[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_mcs::input illegal!}");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* 解析要设置为多大的值 */
    l_mcs = oal_atoi(ac_arg);
    if (l_mcs < WAL_HIPRIV_HT_MCS_MIN || l_mcs > WAL_HIPRIV_HT_MCS_MAX) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcs::input val out of range [%d]!}", l_mcs);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_set_mcs_param->uc_param = (uint8_t)l_mcs;

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tx_comp_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcs::err [%d]!}", ret);
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_mcs fail, err code[%u]!}", result);
        return result;
    }
    return OAL_SUCC;
}


uint32_t wal_hipriv_set_mcsac(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;
    mac_cfg_tx_comp_stru *pst_set_mcs_param = NULL;
    int32_t l_mcs;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t l_idx = 0;
    wal_msg_stru *rsp_msg = NULL;

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_MCSAC, sizeof(mac_cfg_tx_comp_stru));

    /* 解析并设置配置命令参数 */
    pst_set_mcs_param = (mac_cfg_tx_comp_stru *)(write_msg.auc_value);

    /* 获取速率值字符串 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcsac::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 输入命令合法性检测 */
    while (ac_arg[l_idx] != '\0') {
        if (isdigit(ac_arg[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_mcsac::input illegal!}");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* 解析要设置为多大的值 */
    l_mcs = oal_atoi(ac_arg);
    if (l_mcs < WAL_HIPRIV_VHT_MCS_MIN || l_mcs > WAL_HIPRIV_VHT_MCS_MAX) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcs::input val out of range [%d]!}", l_mcs);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    pst_set_mcs_param->uc_param = (uint8_t)l_mcs;

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tx_comp_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcsac::err [%d]!}", ret);
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_mcsac fail, err code[%u]!}", result);
        return result;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11AX

uint32_t wal_hipriv_set_mcsax(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;
    mac_cfg_tx_comp_stru *pst_set_mcs_param;
    int32_t l_mcs;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t l_idx = 0;
    wal_msg_stru *rsp_msg = NULL;

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_MCSAX, sizeof(mac_cfg_tx_comp_stru));

    /* 解析并设置配置命令参数 */
    pst_set_mcs_param = (mac_cfg_tx_comp_stru *)(write_msg.auc_value);

    /* 获取速率值字符串 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcsax::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 输入命令合法性检测 */
    while (ac_arg[l_idx] != '\0') {
        if (isdigit(ac_arg[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_mcsax::input illegal!}");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* 解析要设置为多大的值 */
    l_mcs = oal_atoi(ac_arg);
    if (l_mcs < WAL_HIPRIV_HE_MCS_MIN || l_mcs > WAL_HIPRIV_HE_MCS_MAX) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcsax::input val out of range [%d]!}", l_mcs);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_set_mcs_param->uc_param = (uint8_t)l_mcs;

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tx_comp_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcsax::err [%d]!}", ret);
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_mcsax fail, err code[%u]!}", result);
        return result;
    }

    return OAL_SUCC;
}
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM

uint32_t wal_hipriv_set_mcsax_er(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;
    mac_cfg_tx_comp_stru *pst_set_mcs_param;
    int32_t l_mcs;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t l_idx = 0;
    wal_msg_stru *rsp_msg = NULL;

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_MCSAX_ER, sizeof(mac_cfg_tx_comp_stru));

    /* 解析并设置配置命令参数 */
    pst_set_mcs_param = (mac_cfg_tx_comp_stru *)(write_msg.auc_value);

    /* 获取速率值字符串 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcsax_er::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 输入命令合法性检测 */
    while (ac_arg[l_idx] != '\0') {
        if (isdigit(ac_arg[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_mcsax_er::input illegal!}");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* 解析要设置为多大的值 */
    l_mcs = oal_atoi(ac_arg);
    if (l_mcs < WAL_HIPRIV_HE_ER_MCS_MIN || l_mcs > WAL_HIPRIV_HE_ER_MCS_MAX) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcsax_er::input val out of range [%d]!}", l_mcs);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_set_mcs_param->uc_param = (uint8_t)l_mcs;

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tx_comp_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcsax_er::err [%d]!}", ret);
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_mcsax_er fail, err code[%u]!}", result);
        return result;
    }

    return OAL_SUCC;
}

uint32_t _wal_hipriv_set_mcsax_er(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    if (g_wlan_spec_cfg->feature_11ax_er_su_dcm_is_open) {
        return wal_hipriv_set_mcsax_er(net_dev, pc_param);
    }

    return OAL_SUCC;
}
#endif
#endif


OAL_STATIC uint32_t wal_hipriv_set_band(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    mac_cfg_tx_comp_stru *set_band_param = NULL;
    int8_t arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wal_msg_stru *rsp_msg = NULL;
    uint8_t map_index = 0;
    int32_t ret;

    /* 解析并设置配置命令参数 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_BAND, sizeof(mac_cfg_tx_comp_stru));
    set_band_param = (mac_cfg_tx_comp_stru *)(write_msg.auc_value);
    ret = wal_get_cmd_one_arg(pc_param, arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_band::get cmd one arg err[%d]!}", ret);
        return ret;
    }

    /* 寻找匹配的命令 */
    while (g_band_tbl[map_index] != NULL) {
        if (oal_strcmp(g_band_tbl[map_index], arg) == 0) {
            break;
        }
        ++map_index;
    }

    /* 没有找到对应的命令，则报错 */
    if (g_band_tbl[map_index] == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_band::invalid alg_cfg command!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    set_band_param->uc_param = map_index;
    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tx_comp_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_bw::err [%d]!}", ret);
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    ret = wal_check_and_release_msg_resp(rsp_msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_bw fail, err code[%u]!}", ret);
        return ret;
    }
    return OAL_SUCC;
}



OAL_STATIC uint32_t wal_hipriv_set_bw(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;
    mac_cfg_tx_comp_stru *pst_set_bw_param = NULL;
    wlan_bandwith_cap_enum_uint8 en_bw_index;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wal_msg_stru *rsp_msg = NULL;

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_BW, sizeof(mac_cfg_tx_comp_stru));

    /* 解析并设置配置命令参数 */
    pst_set_bw_param = (mac_cfg_tx_comp_stru *)(write_msg.auc_value);

    /* 获取带宽值字符串 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_bw::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 解析要设置为多大的值 */
    for (en_bw_index = 0; en_bw_index < WLAN_BANDWITH_CAP_BUTT; en_bw_index++) {
        if (!oal_strcmp(g_pauc_bw_tbl[en_bw_index], ac_arg)) {
            break;
        }
    }

    /* 检查命令是否打错 */
    if (en_bw_index >= WLAN_BANDWITH_CAP_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_bw::not support this bandwidth!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_set_bw_param->uc_param = (uint8_t)(en_bw_index);

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tx_comp_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_bw::err [%d]!}", ret);
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_bw fail, err code[%u]!}", result);
        return result;
    }
    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_always_tx(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;
    mac_cfg_tx_comp_stru *pst_set_bcast_param;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint8_t en_tx_flag;
    mac_rf_payload_enum_uint8 en_payload_flag = RF_PAYLOAD_RAND;
    uint32_t len = 2000;
    wal_msg_stru *rsp_msg = NULL;

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_ALWAYS_TX, sizeof(mac_cfg_tx_comp_stru));

    /* 解析并设置配置命令参数 */
    pst_set_bcast_param = (mac_cfg_tx_comp_stru *)(write_msg.auc_value);

    /* 获取常发模式开关标志 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::get cmd one arg err[%d]!}", result);
        return result;
    }
    en_tx_flag = (uint8_t)oal_atoi(ac_name);
    if (en_tx_flag > HAL_ALWAYS_TX_MPDU) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_always_tx::input should be 0 or 1.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* 关闭的情况下不需要解析后面的参数 */
    if (en_tx_flag != HAL_ALWAYS_TX_DISABLE) {
        /* ack_policy参数后续扩充 */
        /* 获取payload_flag参数 */
        pc_param = pc_param + off_set;
        result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        /* 若后面payload_flag和len参数没有设置，采用默认RF_PAYLOAD_RAND 2000 */
        if (result == OAL_SUCC) {
            en_payload_flag = (uint8_t)oal_atoi(ac_name);
            if (en_payload_flag >= RF_PAYLOAD_BUTT) {
                oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::payload flag err[%d]!}", en_payload_flag);
                return OAL_ERR_CODE_INVALID_CONFIG;
            }

            /* 获取len参数 */
            pc_param = pc_param + off_set;
            result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
            if (result != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::get cmd one arg err[%d]!}", result);
                return result;
            }
            len = (uint16_t)oal_atoi(ac_name);
            if (len > 65535) { // 65535代表uint16_t类型大小上限
                oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::len [%u] overflow!}", len);
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
        }
    }

    pst_set_bcast_param->en_payload_flag = en_payload_flag;
    pst_set_bcast_param->payload_len = len;
    pst_set_bcast_param->uc_param = en_tx_flag;
    pst_set_bcast_param->write_index = 0;
    oam_warning_log2(0, OAM_SF_ANY, "{wal_hipriv_always_tx::tx_flag[%d],len[%d]!}", en_tx_flag, len);
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tx_comp_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::err [%d]!}", ret);
        return (uint32_t)ret;
    }

    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_always_tx fail, err code[%u]!}", result);
        return result;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_set_ru_index(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t ret;
    mac_cfg_tx_comp_stru *set_param;
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    wal_msg_stru *rsp_msg = NULL;

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_RU_INDEX, sizeof(mac_cfg_tx_comp_stru));
    /* 解析并设置配置命令参数 */
    set_param = (mac_cfg_tx_comp_stru *)(write_msg.auc_value);

    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ru_index::get cmd one arg err[%d]!}", ret);
        return ret;
    }

    for (set_param->uc_param = 0; set_param->uc_param < WLAN_HE_RU_SIZE_BUTT; set_param->uc_param++) {
        if (!oal_strcmp(g_ru_size_tbl[set_param->uc_param], name)) {
            break;
        }
    }
    /* 检查命令是否打错 */
    if (set_param->uc_param >= WLAN_HE_RU_SIZE_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_ru_index::not support this ru size!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    param = param + off_set;
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ru_index::get cmd one arg err[%d]!}", ret);
        return ret;
    }
    set_param->en_payload_flag = (uint8_t)oal_atoi(name);

    param = param + off_set;
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    set_param->en_protocol_mode = (uint8_t)oal_atoi(name);

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH +
        sizeof(mac_cfg_tx_comp_stru), (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ru_index::err [%d]!}", ret);
        return (uint32_t)ret;
    }

    /* 读取返回的错误码 */
    ret = wal_check_and_release_msg_resp(rsp_msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_ru_index fail, err code[%u]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}



OAL_STATIC uint32_t wal_hipriv_always_rx(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    return wal_ioctl_set_al_rx(net_dev, pc_param);
}
#ifdef _PRE_WLAN_FEATURE_MONITOR

OAL_STATIC uint32_t wal_hipriv_set_sniffer(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    return wal_ioctl_set_sniffer(net_dev, pc_param);
}


OAL_STATIC uint32_t wal_hipriv_enable_monitor_mode(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    return wal_ioctl_set_monitor_mode(net_dev, pc_param);
}
#endif


OAL_STATIC int wal_ioctl_get_iwname(oal_net_device_stru *net_dev,
                                    oal_iw_request_info_stru *pst_info,
                                    oal_iwreq_data_union *pst_wrqu,
                                    char *pc_extra)
{
    int8_t ac_iwname[] = "IEEE 802.11";

    if ((net_dev == NULL) || (pst_wrqu == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_get_iwname::param null.}");
        return -OAL_EINVAL;
    }

    if (EOK != memcpy_s(pst_wrqu->name, OAL_IF_NAME_SIZE, ac_iwname, sizeof(ac_iwname))) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_get_iwname::memcpy fail!");
        return -OAL_EINVAL;
    }
    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_find_cmd(int8_t *pc_cmd_name,
    uint8_t uc_cmd_name_len, wal_hipriv_cmd_entry_stru **pst_cmd_id)
{
    uint32_t en_cmd_idx;
    int ret;
    *pst_cmd_id = NULL;

    if (oal_unlikely(oal_any_null_ptr2(pc_cmd_name, pst_cmd_id))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_find_cmd::pc_cmd_name/puc_cmd_id is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (uc_cmd_name_len > WAL_HIPRIV_CMD_NAME_MAX_LEN) {
        return OAL_FAIL;
    }

    for (en_cmd_idx = 0; en_cmd_idx < oal_array_size(g_ast_hipriv_cmd); en_cmd_idx++) {
        ret = oal_strcmp(g_ast_hipriv_cmd[en_cmd_idx].pc_cmd_name, pc_cmd_name);
        if (ret == 0) {
            *pst_cmd_id = (wal_hipriv_cmd_entry_stru *)&g_ast_hipriv_cmd[en_cmd_idx];
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_find_cmd::wal_hipriv_find_cmd en_cmd_idx = %d.}", en_cmd_idx);
            return OAL_SUCC;
        }
    }

#ifdef _PRE_WLAN_CFGID_DEBUG
    for (en_cmd_idx = 0; en_cmd_idx < wal_hipriv_get_debug_cmd_size(); en_cmd_idx++) {
        ret = oal_strcmp(g_ast_hipriv_cmd_debug[en_cmd_idx].pc_cmd_name, pc_cmd_name);
        if (ret == 0) {
            *pst_cmd_id = (wal_hipriv_cmd_entry_stru *)&g_ast_hipriv_cmd_debug[en_cmd_idx];
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_find_cmd::wal_hipriv_find_cmd en_cmd_idx:%d.}", en_cmd_idx);

            return OAL_SUCC;
        }
    }
#endif
    oam_io_printk("cmd name[%s] is not exist. \r\n", pc_cmd_name);
    return OAL_FAIL;
}


OAL_STATIC uint32_t wal_hipriv_get_cmd_net_dev(int8_t *pc_cmd, oal_net_device_stru **net_device, uint32_t *off_set)
{
    oal_net_device_stru *net_dev = NULL;
    int8_t ac_dev_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    uint32_t result;

    if (oal_any_null_ptr3(pc_cmd, net_device, off_set)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_net_dev::cmd/net_device/off_set is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    result = wal_get_cmd_one_arg(pc_cmd, ac_dev_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_net_dev::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 根据dev_name找到dev */
    net_dev = wal_config_get_netdev(ac_dev_name, OAL_STRLEN(ac_dev_name));
    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_net_dev::wal_config_get_netdev return null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(net_dev);

    *net_device = net_dev;

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_get_cmd_id(int8_t *pc_cmd,
    wal_hipriv_cmd_entry_stru **pst_cmd_id, uint32_t *off_set)
{
    int8_t ac_cmd_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    uint32_t result;

    if (oal_unlikely(oal_any_null_ptr3(pc_cmd, pst_cmd_id, off_set))) {
        oam_error_log0(0, OAM_SF_ANY,
            "{wal_hipriv_get_cmd_id::pc_cmd/puc_cmd_id/off_set is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    result = wal_get_cmd_one_arg(pc_cmd, ac_cmd_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_id::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 根据命令名找到命令枚举 */
    result = wal_hipriv_find_cmd(ac_cmd_name, sizeof(ac_cmd_name), pst_cmd_id);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_id::wal_hipriv_find_cmd return error cod [%d]!}", result);
        return result;
    }

    return OAL_SUCC;
}


uint32_t wal_hipriv_parse_cmd(int8_t *pc_cmd)
{
    oal_net_device_stru *net_dev = NULL;
    wal_hipriv_cmd_entry_stru *pst_hipriv_cmd_entry = NULL;
    uint32_t off_set = 0;
    uint32_t result;
    if (oal_unlikely(pc_cmd == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd::pc_cmd null ptr error!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (wal_process_cmd(pc_cmd) == OAL_SUCC) {
        return OAL_SUCC;
    }
    /*
        cmd格式约束
        网络设备名 命令      参数   Hisilicon0 create vap0
        1~15Byte   1~15Byte
    */
    result = wal_hipriv_get_cmd_net_dev(pc_cmd, &net_dev, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd::wal_hipriv_get_cmd_net_dev error[%d]!}", result);
        return result;
    }

    pc_cmd += off_set;
    result = wal_hipriv_get_cmd_id(pc_cmd, &pst_hipriv_cmd_entry, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd::wal_hipriv_get_cmd_id  error[%d]!}", result);
        return result;
    }

    pc_cmd += off_set;
    /* 调用命令对应的函数 */
    result = pst_hipriv_cmd_entry->p_func(net_dev, pc_cmd);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd::g_ast_hipriv_cmd  error[%d]!}", result);
        return result;
    }

    return OAL_SUCC;
}

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT

OAL_STATIC oal_ssize_t wal_hipriv_sys_write(struct kobject *dev, struct kobj_attribute *attr,
    const char *pc_buffer, oal_size_t count)
{
    int8_t *pc_cmd;
    uint32_t result;
    uint32_t len = (uint32_t)count;

    if (len > WAL_HIPRIV_CMD_MAX_LEN) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_sys_write::ul_len>WAL_HIPRIV_CMD_MAX_LEN, len [%d]!}", len);
        return -OAL_EINVAL;
    }

    pc_cmd = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, WAL_HIPRIV_CMD_MAX_LEN, OAL_TRUE);
    if (oal_unlikely(pc_cmd == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_proc_write::alloc mem return null ptr!}");
        return -OAL_ENOMEM;
    }

    memset_s(pc_cmd, WAL_HIPRIV_CMD_MAX_LEN, 0, WAL_HIPRIV_CMD_MAX_LEN);

    if (EOK != memcpy_s(pc_cmd, WAL_HIPRIV_CMD_MAX_LEN, pc_buffer, len)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_sys_write::memcpy fail!");
        oal_mem_free_m(pc_cmd, OAL_TRUE);
        return -OAL_EINVAL;
    }

    pc_cmd[len - 1] = '\0';

    result = wal_hipriv_parse_cmd(pc_cmd);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_proc_write::parse cmd return err code[%d]!}", result);
    }

    oal_mem_free_m(pc_cmd, OAL_TRUE);

    return (int32_t)len;
}


#define SYS_READ_MAX_STRING_LEN (4096 - 40) /* 当前命令字符长度20字节内，预留40保证不会超出 */
OAL_STATIC oal_ssize_t wal_hipriv_sys_read(struct kobject *dev, struct kobj_attribute *attr, char *pc_buffer)
{
    uint32_t cmd_idx;
    uint32_t buff_index = 0;

    for (cmd_idx = 0; cmd_idx < oal_array_size(g_ast_hipriv_cmd); cmd_idx++) {
        buff_index += snprintf_s(pc_buffer + buff_index, (SYS_READ_MAX_STRING_LEN - buff_index),
                                 (SYS_READ_MAX_STRING_LEN - buff_index) - 1,
                                 "\t%s\n", g_ast_hipriv_cmd[cmd_idx].pc_cmd_name);
        if (buff_index > SYS_READ_MAX_STRING_LEN) {
            oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_sys_read::snprintf_s error!");
            return buff_index;
        }
    }
#ifdef _PRE_WLAN_CFGID_DEBUG
    for (cmd_idx = 0; cmd_idx < wal_hipriv_get_debug_cmd_size(); cmd_idx++) {
        buff_index += snprintf_s(pc_buffer + buff_index, (SYS_READ_MAX_STRING_LEN - buff_index),
                                 (SYS_READ_MAX_STRING_LEN - buff_index) - 1,
                                 "\t%s\n", g_ast_hipriv_cmd_debug[cmd_idx].pc_cmd_name);
        if (buff_index > SYS_READ_MAX_STRING_LEN) {
            oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_sys_read::snprintf_s error!");
            break;
        }
    }
#endif

    return buff_index;
}

#endif /* _PRE_OS_VERSION_LINUX */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
// use sys filesystem instead
#else

OAL_STATIC int32_t wal_hipriv_proc_write(oal_file_stru *pst_file,
    const int8_t *pc_buffer, uint32_t len, void *p_data)
{
    int8_t *pc_cmd = NULL;
    uint32_t result;

    if (len > WAL_HIPRIV_CMD_MAX_LEN) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_proc_write::ul_len>WAL_HIPRIV_CMD_MAX_LEN, len [%d]!}", len);
        return -OAL_EINVAL;
    }

    pc_cmd = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, WAL_HIPRIV_CMD_MAX_LEN, OAL_TRUE);
    if (oal_unlikely(pc_cmd == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_proc_write::alloc mem return null ptr!}");
        return -OAL_ENOMEM;
    }

    memset_s(pc_cmd, WAL_HIPRIV_CMD_MAX_LEN, 0, WAL_HIPRIV_CMD_MAX_LEN);

    result = oal_copy_from_user((void *)pc_cmd, pc_buffer, len);
    /* copy_from_user函数的目的是从用户空间拷贝数据到内核空间，失败返回没有被拷贝的字节数，成功返回0 */
    if (result > 0) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_proc_write::oal_copy_from_user return result[%d]!}", result);
        oal_mem_free_m(pc_cmd, OAL_TRUE);

        return -OAL_EFAUL;
    }

    pc_cmd[len - 1] = '\0';

    result = wal_hipriv_parse_cmd(pc_cmd);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_proc_write::parse cmd return err code[%d]!}", result);
    }

    oal_mem_free_m(pc_cmd, OAL_TRUE);

    return (int32_t)len;
}
#endif

uint32_t wal_hipriv_create_proc(void *p_proc_arg)
{
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    uint32_t result;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    g_pst_proc_entry = NULL;
#else

    /* S_IRUSR文件所有者具可读取权限, S_IWUSR文件所有者具可写入权限, S_IRGRP用户组具可读取权限,
       S_IROTH其他用户具可读取权限 */
    /* 420代表十进制对应八进制是0644 linux模式定义 S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH); */
    g_pst_proc_entry = oal_create_proc_entry(WAL_HIPRIV_PROC_ENTRY_NAME, 420, NULL);
    if (g_pst_proc_entry == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_create_proc::oal_create_proc_entry return null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_pst_proc_entry->data = p_proc_arg;
    g_pst_proc_entry->nlink = 1; /* linux创建proc默认值 */
    g_pst_proc_entry->read_proc = NULL;

    g_pst_proc_entry->write_proc = (write_proc_t *)wal_hipriv_proc_write;
#endif

    /* hi1102-cb add sys for 51/02 */
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    g_gp_sys_kobject = oal_get_sysfs_root_object();
    if (g_gp_sys_kobject == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_create_proc::get sysfs root object failed!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    result = (uint32_t)oal_debug_sysfs_create_group(g_gp_sys_kobject, &g_hipriv_attribute_group);
    if (result) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_create_proc::hipriv_attribute_group create failed!}");
        result = OAL_ERR_CODE_PTR_NULL;
        return result;
    }
#endif

    return OAL_SUCC;
}


uint32_t wal_hipriv_remove_proc(void)
{
    /* 卸载时删除sysfs */
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    if (g_gp_sys_kobject != NULL) {
        oal_debug_sysfs_remove_group(g_gp_sys_kobject, &g_hipriv_attribute_group);
        kobject_del(g_gp_sys_kobject);
        g_gp_sys_kobject = NULL;
    }
    oal_conn_sysfs_root_obj_exit();
    oal_conn_sysfs_root_boot_obj_exit();
#endif

    if (g_pst_proc_entry) {
        oal_remove_proc_entry(WAL_HIPRIV_PROC_ENTRY_NAME, NULL);
        g_pst_proc_entry = NULL;
    }

    return OAL_SUCC;
}
#ifdef PLATFORM_DEBUG_ENABLE

OAL_STATIC uint32_t wal_hipriv_reg_info(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    uint16_t len;

    /* 抛事件到wal层处理 */
    ret = memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value), pc_param, OAL_STRLEN(pc_param));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_reg_info::memcpy fail!");
        return OAL_FAIL;
    }

    write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    len = (uint16_t)(OAL_STRLEN(pc_param) + 1);

    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_REG_INFO, len);

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + len,
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_reg_info::return err code [%d]!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}
#endif

OAL_STATIC uint32_t wal_hipriv_sdio_flowctrl(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    uint16_t len;

    if (oal_unlikely(WAL_MSG_WRITE_MAX_LEN <= OAL_STRLEN(pc_param))) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_sdio_flowctrl:: pc_param overlength:%d}", OAL_STRLEN(pc_param));
        return OAL_FAIL;
    }

    /* 抛事件到wal层处理 */
    ret = memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value), pc_param, OAL_STRLEN(pc_param));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_sdio_flowctrl::memcpy fail!");
        return OAL_FAIL;
    }

    write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    len = (uint16_t)(OAL_STRLEN(pc_param) + 1);

    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SDIO_FLOWCTRL, len);

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + len,
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_sdio_flowctrl::return err code [%d]!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}
#ifdef PLATFORM_DEBUG_ENABLE

OAL_STATIC uint32_t wal_hipriv_reg_write(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    uint16_t len;

    /* 抛事件到wal层处理 */
    ret = memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value), pc_param, OAL_STRLEN(pc_param));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_reg_write::memcpy fail!");
        return OAL_FAIL;
    }

    write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    len = (uint16_t)(OAL_STRLEN(pc_param) + 1);

    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_REG_WRITE, len);

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + len,
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_reg_write::return err code [%d]!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}
#endif

uint32_t wal_hipriv_alg_cfg(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t result;
    mac_ioctl_alg_param_stru *pst_alg_param = NULL;
    wal_ioctl_alg_cfg_stru st_alg_cfg;
    uint8_t uc_map_index = 0;
    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;

    pst_alg_param = (mac_ioctl_alg_param_stru *)(write_msg.auc_value);

    /* 获取配置参数名称 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 寻找匹配的命令 */
    st_alg_cfg = g_ast_alg_cfg_map[0];
    while (st_alg_cfg.pc_name != NULL) {
        if (0 == oal_strcmp(st_alg_cfg.pc_name, ac_name)) {
            break;
        }
        st_alg_cfg = g_ast_alg_cfg_map[++uc_map_index];
    }

    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg:: alg_cfg cmd[%d]!}", st_alg_cfg.en_alg_cfg);

    /* 没有找到对应的命令，则报错 */
    if (st_alg_cfg.pc_name == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg::invalid alg_cfg cmd!}");
        return OAL_FAIL;
    }

    /* 记录命令对应的枚举值 */
    pst_alg_param->en_alg_cfg = g_ast_alg_cfg_map[uc_map_index].en_alg_cfg;

    /* 获取参数配置值 */
    result = wal_get_cmd_one_arg(pc_param + off_set, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 记录参数配置值 */
    pst_alg_param->value = (uint32_t)oal_atoi(ac_name);

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_ALG_PARAM, sizeof(mac_ioctl_alg_param_stru));

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_ioctl_alg_param_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_alg_cfg fail, err code[%u]!}", result);
        return result;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_tpc_log(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    uint32_t result;
    int32_t ret;
    wal_ioctl_alg_tpc_log_param_stru *pst_alg_tpc_log_param = NULL;
    wal_ioctl_alg_cfg_stru st_alg_cfg;
    uint8_t uc_map_index = 0;
    oal_bool_enum_uint8 en_stop_flag = OAL_FALSE;

    pst_alg_tpc_log_param = (wal_ioctl_alg_tpc_log_param_stru *)(write_msg.auc_value);

    /* 获取配置参数名称 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::get cmd one arg err[%d]!}", result);
        return result;
    }
    pc_param = pc_param + off_set;

    /* 寻找匹配的命令 */
    st_alg_cfg = g_ast_alg_cfg_map[0];
    while (st_alg_cfg.pc_name != NULL) {
        if (0 == oal_strcmp(st_alg_cfg.pc_name, ac_name)) {
            break;
        }
        st_alg_cfg = g_ast_alg_cfg_map[++uc_map_index];
    }

    /* 没有找到对应的命令，则报错 */
    if (st_alg_cfg.pc_name == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::invalid alg_cfg cmd!}");
        return OAL_FAIL;
    }

    /* 记录命令对应的枚举值 */
    pst_alg_tpc_log_param->en_alg_cfg = g_ast_alg_cfg_map[uc_map_index].en_alg_cfg;

    /* 区分获取特定帧功率和统计日志命令处理:获取功率只需获取帧名字 */
    if (pst_alg_tpc_log_param->en_alg_cfg == MAC_ALG_CFG_TPC_GET_FRAME_POW) {
        /* 获取配置参数名称 */
        result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (result != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::get cmd one arg err[%d]!}", result);
            return result;
        }
        /* 记录命令对应的帧名字 */
        pst_alg_tpc_log_param->pc_frame_name = ac_name;
    } else {
        result = wal_hipriv_get_mac_addr(pc_param, pst_alg_tpc_log_param->auc_mac_addr, &off_set);
        if (result != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::wal_hipriv_get_mac_addr failed!}");
            return result;
        }
        pc_param += off_set;

        while ((*pc_param == ' ') || (*pc_param == '\0')) {
            if (*pc_param == '\0') {
                en_stop_flag = OAL_TRUE;
                break;
            }
            ++pc_param;
        }

        /* 获取业务类型值 */
        if (en_stop_flag != OAL_TRUE) {
            result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
            if (result != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::get cmd one arg err[%d]!}", result);
                return result;
            }

            pst_alg_tpc_log_param->uc_ac_no = (uint8_t)oal_atoi(ac_name);
            pc_param = pc_param + off_set;

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
                result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
                if (result != OAL_SUCC) {
                    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_tpc_log::get cmd one arg err[%d]!}", result);
                    return result;
                }

                /* 记录参数配置值 */
                pst_alg_tpc_log_param->us_value = (uint16_t)oal_atoi(ac_name);
            }
        }
    }

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_ALG_PARAM, sizeof(wal_ioctl_alg_tpc_log_param_stru));

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(wal_ioctl_alg_tpc_log_param_stru),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_HS20

OAL_STATIC int32_t wal_ioctl_set_qos_map(oal_net_device_stru *net_dev,
    hmac_cfg_qos_map_param_stru *pst_qos_map_param)
{
    wal_msg_write_stru write_msg = {0};
    wal_msg_stru *rsp_msg = NULL;
    uint32_t err_code;
    int32_t ret;

    ret = memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value),
                   pst_qos_map_param, sizeof(hmac_cfg_qos_map_param_stru));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_HS20, "wal_ioctl_set_qos_map::memcpy fail!");
        return -OAL_EFAIL;
    }

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_QOS_MAP, sizeof(hmac_cfg_qos_map_param_stru));

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(hmac_cfg_qos_map_param_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if ((ret != OAL_SUCC) || (rsp_msg == NULL)) {
        oam_error_log1(0, OAM_SF_HS20, "{wal_ioctl_set_qos_map:: wal_alloc_cfg_event return err code %d!}", ret);
        return ret;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_HS20, "{wal_ioctl_set_qos_map::wal_check_and_release_msg_resp fail[%x!}",
                         err_code);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_HS20


OAL_STATIC uint32_t wal_get_parameter_from_cmd(int8_t *pc_cmd, int8_t *pc_arg,
    const int8_t *puc_token, uint32_t *pul_cmd_offset, uint32_t param_max_len)
{
    int8_t *pc_cmd_copy = NULL;
    int8_t ac_cmd_copy[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN];
    uint32_t pos = 0;
    uint32_t arg_len;
    int8_t *pc_cmd_tmp = NULL;

    if (oal_unlikely(oal_any_null_ptr3(pc_cmd, pc_arg, pul_cmd_offset))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_get_parameter_from_cmd::pc_cmd/pc_arg/pul_cmd_offset is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pc_cmd_copy = pc_cmd;

    /* 去掉字符串开始的逗号 */
    while (*pc_cmd_copy == ',') {
        ++pc_cmd_copy;
    }
    /* 取得逗号前的字符串 */
    while ((*pc_cmd_copy != ',') && (*pc_cmd_copy != '\0')) {
        ac_cmd_copy[pos] = *pc_cmd_copy;
        ++pos;
        ++pc_cmd_copy;

        if (oal_unlikely(pos >= param_max_len)) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_get_parameter_from_cmd::pos>=CMD_NAME_MAX_LEN, pos %d!}", pos);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }
    ac_cmd_copy[pos] = '\0';
    /* 字符串到结尾，返回错误码 */
    if (pos == 0) {
        oam_info_log0(0, OAM_SF_ANY, "{wal_get_parameter_from_cmd::return param pc_arg is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    *pul_cmd_offset = (uint32_t)(pc_cmd_copy - pc_cmd);

    /* 检查字符串是否包含期望的前置命令字符 */
    pc_cmd_tmp = &ac_cmd_copy[0];
    if (oal_memcmp(pc_cmd_tmp, puc_token, OAL_STRLEN(puc_token)) != 0) {
        return OAL_FAIL;
    } else {
        /* 扣除前置命令字符，回传参数 */
        arg_len = OAL_STRLEN(ac_cmd_copy) - OAL_STRLEN(puc_token);
        if (EOK != memcpy_s(pc_arg, param_max_len, ac_cmd_copy + OAL_STRLEN(puc_token), arg_len)) {
            oam_warning_log0(0, OAM_SF_ANY, "wal_get_parameter_from_cmd::memcpy fail!");
        }
        pc_arg[arg_len] = '\0';
    }
    return OAL_SUCC;
}


OAL_STATIC int32_t wal_set_ap_max_user(oal_net_device_stru *net_dev, uint32_t ap_max_user)
{
    wal_msg_write_stru write_msg = {0};
    wal_msg_stru *rsp_msg = NULL;
    uint32_t err_code;
    int32_t ret;

    oam_warning_log1(0, OAM_SF_ANY, "{wal_set_ap_max_user::set AP max user:%u.}", ap_max_user);

    if (ap_max_user == 0) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_set_ap_max_user::invalid max user(%u),ignore it}", ap_max_user);
        return OAL_SUCC;
    }

    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_MAX_USER, sizeof(ap_max_user));
    *((uint32_t *)write_msg.auc_value) = ap_max_user;
    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(ap_max_user),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if ((ret != OAL_SUCC) || (rsp_msg == NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_set_ap_max_user:: wal_send_cfg_event return err code %d!}", ret);
        return ret;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_set_ap_max_user::wal_check_and_release_msg_resp fail[%d]!}",
                         err_code);
        return -OAL_EFAIL;
    }
    /* 每次设置最大用户数完成后，都清空为非法值0 **/
    /* g_st_ap_config_info.ap_max_user 0 */
    return ret;
}


OAL_STATIC void wal_config_mac_filter(oal_net_device_stru *net_dev, int8_t *pc_command)
{
    int8_t ac_parsed_command[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN] = {0};
    int8_t *pc_parse_command = NULL;
    uint32_t mac_mode;
    uint32_t mac_cnt;
    uint32_t i;
    wal_msg_write_stru write_msg = {0};
    uint16_t len;
    wal_msg_stru *rsp_msg = NULL;
    uint32_t err_code;
    int32_t ret = 0;
    mac_blacklist_stru *pst_blklst = NULL;
    uint32_t result;
    uint32_t off_set = 0;

    if (pc_command == NULL) {
        return;
    }
    pc_parse_command = pc_command;

    /* 解析MAC_MODE */
    result = wal_get_parameter_from_cmd(pc_parse_command, ac_parsed_command, "MAC_MODE=",
        &off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_config_mac_filter::wal_get_parameter_from_cmd err %u.}", result);
        return;
    }
    /* 检查参数是否合法 0,1,2 */
    mac_mode = (uint32_t)oal_atoi(ac_parsed_command);
    if (mac_mode > 2) { // 2代表合法参数值上限大小
        oam_warning_log4(0, OAM_SF_ANY, "{wal_config_mac_filter::invalid MAC_MODE[%c%c%c%c]!}",
            (uint8_t)ac_parsed_command[0], (uint8_t)ac_parsed_command[1],
            (uint8_t)ac_parsed_command[2], (uint8_t)ac_parsed_command[3]); /* 2 3 cmd para */
        return;
    }

    /* 设置过滤模式 */
    result = wal_hipriv_send_cfg_uint32_data(net_dev, ac_parsed_command, WLAN_CFGID_BLACKLIST_MODE);
    if (result != OAL_SUCC) {
        return;
    }
    /* 解析MAC_CNT */
    pc_parse_command += off_set;
    result = wal_get_parameter_from_cmd(pc_parse_command, ac_parsed_command, "MAC_CNT=",
        &off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_config_mac_filter::wal_get_parameter_from_cmd err [%u]!}", result);
        return;
    }
    mac_cnt = (uint32_t)oal_atoi(ac_parsed_command);

    for (i = 0; i < mac_cnt; i++) {
        pc_parse_command += off_set;
        result = wal_get_parameter_from_cmd(pc_parse_command, ac_parsed_command, "MAC=",
            &off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
        if (result != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_config_mac_filter::wal_get_parameter_from_cmd err [%u]!}", result);
            return;
        }
        /* 5.1  检查参数是否符合MAC长度 */
        if (WLAN_MAC_ADDR_LEN << BIT_OFFSET_1 != OAL_STRLEN(ac_parsed_command)) {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_config_mac_filter::invalid MAC format}");
            return;
        }
        /* 6. 添加过滤设备 */
        /* 抛事件到wal层处理 */
        memset_s((uint8_t *)&write_msg, sizeof(write_msg), 0, sizeof(write_msg));
        pst_blklst = (mac_blacklist_stru *)(write_msg.auc_value);
        /* 将字符 ac_name 转换成数组 mac_add[6] */
        oal_strtoaddr(ac_parsed_command, sizeof(ac_parsed_command), pst_blklst->auc_mac_addr, WLAN_MAC_ADDR_LEN);

        len = sizeof(mac_blacklist_stru);
        if (i == (mac_cnt - 1)) {
            /* 等所有的mac地址都添加完成后，才进行关联用户确认，是否需要删除 */
            WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_ADD_BLACK_LIST, len);
        } else {
            WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_ADD_BLACK_LIST_ONLY, len);
        }

        /* 6.1  发送消息 */
        ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
            WAL_MSG_WRITE_MSG_HDR_LENGTH + len, (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
        if ((ret != OAL_SUCC) || (rsp_msg == NULL)) {
            oam_error_log1(0, OAM_SF_ANY, "{wal_config_mac_filter:: wal_send_cfg_event err %d!}", ret);
            return;
        }

        /* 6.2  读取返回的错误码 */
        err_code = wal_check_and_release_msg_resp(rsp_msg);
        if (err_code != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_config_mac_filter::wal_check_and_release_msg_resp fail[%x]!}",
                             err_code);
            return;
        }
    }
    /* 每次设置完成mac地址过滤后，清空此中间变量 */
    return;
}


OAL_STATIC int32_t wal_kick_sta(oal_net_device_stru *net_dev,
    uint8_t *auc_mac_addr, uint8_t uc_mac_addr_len, uint16_t us_reason_code)
{
    wal_msg_write_stru write_msg = {0};
    wal_msg_stru *rsp_msg = NULL;
    uint32_t err_code;
    mac_cfg_kick_user_param_stru *pst_kick_user_param = NULL;
    int32_t ret;

    if ((auc_mac_addr == NULL) || (uc_mac_addr_len != WLAN_MAC_ADDR_LEN)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_kick_sta::check para! .\n");
        return -OAL_EFAIL;
    }

    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_KICK_USER, sizeof(mac_cfg_kick_user_param_stru));

    pst_kick_user_param = (mac_cfg_kick_user_param_stru *)(write_msg.auc_value);
    oal_set_mac_addr(pst_kick_user_param->auc_mac_addr, auc_mac_addr);

    pst_kick_user_param->us_reason_code = us_reason_code;

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_kick_user_param_stru),
        (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if ((ret != OAL_SUCC) || (rsp_msg == NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_kick_sta:: wal_send_cfg_event return err code %d!}", ret);
        return ret;
    }

    /* 4.4  读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_kick_sta::wal_check_and_release_msg_resp return err code: [%x]!}",
                         err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC int wal_ioctl_set_ap_config(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra)
{
    int8_t *pc_command = NULL;
    int8_t *pc_parse_cmd = NULL;
    int32_t ret = OAL_SUCC;
    uint32_t result;
    uint32_t off_set = 0;
    int8_t ac_cmd[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN] = {0};

    if (oal_unlikely(oal_any_null_ptr2(net_dev, pst_wrqu))) {
        oam_warning_log0(0, 0, "{wal_ioctl_set_ap_config:input params null.}");
        return -OAL_EFAIL;
    }

    /* 1. 申请内存保存netd 下发的命令和数据 */
    pc_command = oal_memalloc((int32_t)(pst_wrqu->data.length + 1));
    if (pc_command == NULL) {
        return -OAL_ENOMEM;
    }
    /* 2. 拷贝netd 命令到内核态中 */
    memset_s(pc_command, (uint32_t)(pst_wrqu->data.length + 1), 0, (uint32_t)(pst_wrqu->data.length + 1));
    result = oal_copy_from_user(pc_command, pst_wrqu->data.pointer, (uint32_t)(pst_wrqu->data.length));
    if (result != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_set_ap_config::oal_copy_from_user: -OAL_EFAIL }");
        oal_free(pc_command);
        return -OAL_EFAIL;
    }
    pc_command[pst_wrqu->data.length] = '\0';

    oal_io_print("wal_ioctl_set_ap_config,data len:%u \n", (uint32_t)pst_wrqu->data.length);

    pc_parse_cmd = pc_command;
    /* 3.   解析参数 */
    /* 3.1  解析ASCII_CMD */
    result = wal_get_parameter_from_cmd(pc_parse_cmd, ac_cmd, "ASCII_CMD=", &off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (result != OAL_SUCC) {
        oal_free(pc_command);
        return -OAL_EINVAL;
    }

    if ((0 != oal_strcmp("AP_CFG", ac_cmd))) {
        oal_free(pc_command);
        return -OAL_EINVAL;
    }

    /* 3.2  解析CHANNEL，目前不处理netd下发的channel信息 */
    pc_parse_cmd += off_set;
    result = wal_get_parameter_from_cmd(pc_parse_cmd, ac_cmd, "CHANNEL=", &off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (result != OAL_SUCC) {
        oal_free(pc_command);
        return -OAL_EINVAL;
    }

    /* 3.3  解析MAX_SCB */
    pc_parse_cmd += off_set;
    result = wal_get_parameter_from_cmd(pc_parse_cmd, ac_cmd, "MAX_SCB=", &off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (result != OAL_SUCC) {
        oal_free(pc_command);
        return -OAL_EINVAL;
    }

    g_st_ap_config_info.ap_max_user = (uint32_t)oal_atoi(ac_cmd);
    if (oal_net_dev_priv(net_dev) != NULL) {
        ret = wal_set_ap_max_user(net_dev, (uint32_t)oal_atoi(ac_cmd));
    }

    /* 5. 结束释放内存 */
    oal_free(pc_command);
    return ret;
}


OAL_STATIC int wal_ioctl_get_assoc_list(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra)
{
    int32_t ret;
    int32_t l_mret = EOK;
    wal_msg_query_stru st_query_msg;
    wal_msg_stru *rsp_msg = NULL;
    wal_msg_rsp_stru *pst_query_rsp_msg = NULL;
    int8_t *pc_sta_list = NULL;
    oal_netbuf_stru *pst_rsp_netbuf = NULL;

    if (oal_unlikely(oal_any_null_ptr4(net_dev, pst_info, pst_wrqu, pc_extra))) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_get_assoc_list::para null}");
        return -OAL_EFAIL;
    }

    /* 上层在任何时候都可能下发此命令，需要先判断当前netdev的状态并及时返回 */
    if (oal_unlikely(oal_net_dev_priv(net_dev) == NULL)) {
        return -OAL_EFAIL;
    }

    /* 抛事件到wal层处理 */
    st_query_msg.en_wid = WLAN_CFGID_GET_STA_LIST;

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_QUERY,
        WAL_MSG_WID_LENGTH, (uint8_t *)&st_query_msg, OAL_TRUE, &rsp_msg);
    if ((ret != OAL_SUCC) || (rsp_msg == NULL)) {
        return ret;
    }

    /* 处理返回消息 */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(rsp_msg->auc_msg_data);
    /* 业务处理 */
    if (pst_query_rsp_msg->us_len >= sizeof(oal_netbuf_stru *)) {
        /* 获取hmac保存的netbuf指针 */
        l_mret = memcpy_s(&pst_rsp_netbuf, sizeof(oal_netbuf_stru *),
            pst_query_rsp_msg->auc_value, sizeof(oal_netbuf_stru *));
        if (pst_rsp_netbuf != NULL) {
            /* 保存ap保存的sta地址信息 */
            pc_sta_list = (int8_t *)oal_netbuf_data(pst_rsp_netbuf);
            pst_wrqu->data.length = (uint16_t)(oal_netbuf_len(pst_rsp_netbuf) + 1);
            l_mret += memcpy_s(pc_extra, pst_wrqu->data.length, pc_sta_list, pst_wrqu->data.length);
            pc_extra[oal_netbuf_len(pst_rsp_netbuf)] = '\0';
            oal_netbuf_free(pst_rsp_netbuf);
        } else {
            ret = -OAL_ENOMEM;
        }
    } else {
        oal_print_hex_dump((uint8_t *)rsp_msg->auc_msg_data, pst_query_rsp_msg->us_len,
            HEX_DUMP_GROUP_SIZE, "query msg: ");
        ret = -OAL_EINVAL;
    }

    if ((ret != OAL_SUCC) || (l_mret != EOK)) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_ioctl_get_assoc_list::process failed,ret:%d, mret:%d}", ret, l_mret);
        ret = -OAL_EINVAL;
    }

    oal_free(rsp_msg);
    return ret;
}


OAL_STATIC int wal_ioctl_set_mac_filters(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra)
{
    mac_vap_stru *vap = NULL;
    int8_t  *pc_command = NULL;
    int32_t  ret = 0;
    uint32_t result;
    int8_t   ac_parsed_command[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN] = {0};
    int8_t  *pc_parse_command = NULL;
    uint32_t mac_mode, mac_cnt;
    uint32_t off_set = 0;
    uint8_t  auc_mac_addr[WLAN_MAC_ADDR_LEN];

    if (oal_unlikely(oal_any_null_ptr4(net_dev, pst_info, pst_wrqu, pc_extra))) {
        oam_warning_log0(0, 0, "{wal_ioctl_set_mac_filters::param null}");
        return -OAL_EFAIL;
    }

    /* 1. 申请内存保存netd 下发的命令和数据 */
    pc_command = oal_memalloc((int32_t)(pst_wrqu->data.length + 1));
    if (pc_command == NULL) {
        return -OAL_ENOMEM;
    }

    /* 2. 拷贝netd 命令到内核态中 */
    memset_s(pc_command, (uint32_t)(pst_wrqu->data.length + 1), 0, (uint32_t)(pst_wrqu->data.length + 1));
    result = oal_copy_from_user(pc_command, pst_wrqu->data.pointer, (uint32_t)(pst_wrqu->data.length));
    if (result != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::oal_copy_from_user: -OAL_EFAIL }");
        oal_free(pc_command);
        return -OAL_EFAIL;
    }
    pc_command[pst_wrqu->data.length] = '\0';

    oal_io_print("wal_ioctl_set_mac_filters,data len:%d \n", pst_wrqu->data.length);

    pc_parse_command = pc_command;

    memset_s(g_st_ap_config_info.ac_ap_mac_filter_mode, sizeof(g_st_ap_config_info.ac_ap_mac_filter_mode),
        0, sizeof(g_st_ap_config_info.ac_ap_mac_filter_mode));
    strncpy_s(g_st_ap_config_info.ac_ap_mac_filter_mode, sizeof(g_st_ap_config_info.ac_ap_mac_filter_mode),
              pc_command, sizeof(g_st_ap_config_info.ac_ap_mac_filter_mode) - 1);

    vap = oal_net_dev_priv(net_dev);
    if (vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::netdevice vap is null,just save it.}");
        oal_free(pc_command);
        return OAL_SUCC;
    }

    /* 3  解析MAC_MODE */
    result = wal_get_parameter_from_cmd(pc_parse_command, ac_parsed_command, "MAC_MODE=",
        &off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::wal_get_parameter_from_cmd err [%u]!}", result);
        oal_free(pc_command);
        return -OAL_EINVAL;
    }
    /* 3.1 检查参数是否合法 0,1,2 */
    mac_mode = (uint32_t)oal_atoi(ac_parsed_command);
    if (mac_mode > 2) { // 2代表参数数值上限
        oam_warning_log4(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::invalid MAC_MODE[%c%c%c%c]!}",
            (uint8_t)ac_parsed_command[BYTE_OFFSET_0], (uint8_t)ac_parsed_command[BYTE_OFFSET_1],
            (uint8_t)ac_parsed_command[BYTE_OFFSET_2], (uint8_t)ac_parsed_command[BYTE_OFFSET_3]); /* 2 3 cmd para */
        oal_free(pc_command);
        return -OAL_EINVAL;
    }

    /* 5 解析MAC_CNT */
    pc_parse_command += off_set;
    result = wal_get_parameter_from_cmd(pc_parse_command, ac_parsed_command, "MAC_CNT=",
        &off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::wal_get_parameter_from_cmd err [%u]!}", result);
        oal_free(pc_command);
        return -OAL_EINVAL;
    }
    mac_cnt = (uint32_t)oal_atoi(ac_parsed_command);

    wal_config_mac_filter(net_dev, pc_command);

    /* 如果是白名单模式，且下发允许MAC地址为空，即不允许任何设备关联，需要去关联所有已经关联的STA */
    if ((mac_cnt == 0) && (mac_mode == 2)) { // 2代表白名单模式
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_mac_filters::delete all user!}");

        memset_s(auc_mac_addr, OAL_ETH_ALEN, 0xff, OAL_ETH_ALEN);
        ret = wal_kick_sta(net_dev, auc_mac_addr, sizeof(auc_mac_addr), MAC_AUTH_NOT_VALID);
    }

    oal_free(pc_command);
    return ret;
}


OAL_STATIC int wal_ioctl_set_ap_sta_disassoc(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra)
{
    int8_t  *pc_command = NULL;
    int32_t  ret;
    uint32_t result;
    int8_t   ac_parsed_command[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN] = { 0 };
    uint8_t  auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0 };
    uint32_t off_set = 0;

    if (oal_unlikely(oal_any_null_ptr2(net_dev, pst_wrqu))) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_ap_sta_disassoc:: param is null}");
        return -OAL_EFAIL;
    }

    /* 1. 申请内存保存netd 下发的命令和数据 */
    pc_command = oal_memalloc((int32_t)(pst_wrqu->data.length + 1));
    if (pc_command == NULL) {
        return -OAL_ENOMEM;
    }

    /* 2. 拷贝netd 命令到内核态中 */
    memset_s(pc_command, (uint32_t)(pst_wrqu->data.length + 1), 0, (uint32_t)(pst_wrqu->data.length + 1));
    result = oal_copy_from_user(pc_command, pst_wrqu->data.pointer, (uint32_t)(pst_wrqu->data.length));
    if (result != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_set_ap_sta_disassoc::oal_copy_from_user: -OAL_EFAIL }");
        oal_free(pc_command);
        return -OAL_EFAIL;
    }
    pc_command[pst_wrqu->data.length] = '\0';

    /* 3. 解析命令获取MAC */
    result = wal_get_parameter_from_cmd(pc_command, ac_parsed_command, "MAC=",
        &off_set, WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_set_ap_sta_disassoc::get_parameter_from_cmd MAC err[%u]}", result);
        oal_free(pc_command);
        return -OAL_EINVAL;
    }
    /* 3.1  检查参数是否符合MAC长度 */
    if ((WLAN_MAC_ADDR_LEN << BIT_OFFSET_1) != OAL_STRLEN(ac_parsed_command)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_ap_sta_disassoc::invalid MAC format}");
        oal_free(pc_command);
        return -OAL_EINVAL;
    }

    /* 将字符 ac_name 转换成数组 mac_add[6] */
    oal_strtoaddr(ac_parsed_command, sizeof(ac_parsed_command), auc_mac_addr, WLAN_MAC_ADDR_LEN);

    oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_ap_sta_disassoc::Geting CMD from APP to DISASSOC!!}");
    ret = wal_kick_sta(net_dev, auc_mac_addr, sizeof(auc_mac_addr), MAC_AUTH_NOT_VALID);

    /* 5. 结束释放内存 */
    oal_free(pc_command);
    return ret;
}

uint32_t wal_hipriv_get_mac_addr(int8_t *pc_param,
    uint8_t auc_mac_addr[], uint32_t *pul_total_offset)
{
    uint32_t off_set = 0;
    uint32_t result ;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    /* 获取mac地址 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_get_mac_addr::get cmd one arg err[%d]!}", result);
        return result;
    }
    oal_strtoaddr(ac_name, sizeof(ac_name), auc_mac_addr, WLAN_MAC_ADDR_LEN);

    *pul_total_offset = off_set;

    return OAL_SUCC;
}


uint32_t wal_hipriv_send_cfg_uint32_data(oal_net_device_stru *net_dev,
    int8_t *pc_param, wlan_cfgid_enum_uint16 cfgid)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    uint16_t len;
    uint32_t result;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t off_set = 0;
    uint32_t set_value;

    /* 抛事件到wal层处理 */
    memset_s(&write_msg, sizeof(wal_msg_write_stru), 0, sizeof(wal_msg_write_stru));
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_send_cfg_uint32_data:wal_get_cmd_one_arg fail!}");
        return result;
    }

    pc_param += off_set;
    set_value = (uint32_t)oal_atoi((const int8_t *)ac_name);

    len = sizeof(uint32_t); /* sizeof(uint32_t) */
    *(uint32_t *)(write_msg.auc_value) = set_value;

    WAL_WRITE_MSG_HDR_INIT(&write_msg, cfgid, len);

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + len,
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_send_cfg_uint32_data:send_cfg_event return[%d]}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}


uint32_t wal_ioctl_set_sta_ps_mode(oal_net_device_stru *cfg_net_dev,
    int8_t uc_ps_mode)
{
    uint32_t cmd_id;
    uint32_t params[CMD_PARAMS_MAX_CNT] = {0};
    uint32_t offset = 0;
    uint32_t ret;

    ret = wal_get_cmd_id("set_ps_mode", &cmd_id, &offset);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_set_sta_ps_mode:find set_ps_mode cmd is fail");
        return ret;
    }
    params[0] = uc_ps_mode;
    return wal_process_cmd_params(cfg_net_dev, cmd_id, params);
}


uint32_t wal_ioctl_set_fast_sleep_para(oal_net_device_stru *cfg_net_dev,
    int8_t *pc_param)
{
    uint32_t cmd_id;
    uint32_t offset = 0;
    uint32_t params[CMD_PARAMS_MAX_CNT] = {0};
    uint32_t ret;
    uint8_t idx;

    ret = wal_get_cmd_id("set_fast_sleep_para", &cmd_id, &offset);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_set_fast_sleep_para:find set_fast_sleep_para cmd is fail");
        return ret;
    }
     /* 获取携带的4个参数<min listen时间><max listen时间><亮屏收发包门限><暗屏收发包门限> */
    for (idx = 0; idx < 4; idx++) {
        params[idx] = pc_param[idx];
    }
    return wal_process_cmd_params(cfg_net_dev, cmd_id, params);
}


int32_t wal_start_vap(oal_net_device_stru *net_dev)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;
    uint32_t err_code;
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    oal_wireless_dev_stru *pst_wdev = NULL;

    if (oal_unlikely(net_dev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_start_vap::net_dev null!}");
        return -OAL_EFAUL;
    }

    oal_io_print("wal_start_vap,dev_name is:%s\n", net_dev->name);

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_START_VAP, sizeof(mac_cfg_start_vap_param_stru));
    ((mac_cfg_start_vap_param_stru *)write_msg.auc_value)->pst_net_dev = net_dev;
    pst_wdev = net_dev->ieee80211_ptr;
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(pst_wdev->iftype);
    if (WLAN_P2P_BUTT == en_p2p_mode) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_start_vap::wal_wireless_iftype_to_mac_p2p_mode return BUFF}");
        return -OAL_EINVAL;
    }
    ((mac_cfg_start_vap_param_stru *)write_msg.auc_value)->en_p2p_mode = en_p2p_mode;

    oam_warning_log1(0, OAM_SF_ANY, "{wal_start_vap::en_p2p_mode:%d}", en_p2p_mode);
    ((mac_cfg_start_vap_param_stru *)write_msg.auc_value)->en_mgmt_rate_init_flag = OAL_TRUE;

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_start_vap_param_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_start_vap::wal_alloc_cfg_event return err code %d!}", ret);
        return ret;
    }
    /* 处理返回消息 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_start_vap::hmac start vap fail, err code[%d]!}", err_code);
        return -OAL_EINVAL;
    }
    if ((oal_netdevice_flags(net_dev) & OAL_IFF_RUNNING) == 0) {
        oal_netdevice_flags(net_dev) |= OAL_IFF_RUNNING;
    }
    /* AP模式,启动VAP后,启动发送队列 */
    oal_net_tx_wake_all_queues(net_dev); /* 启动发送队列 */

    return OAL_SUCC;
}


int32_t wal_stop_vap(oal_net_device_stru *net_dev)
{
    wal_msg_write_stru write_msg = {0};
    wal_msg_stru *rsp_msg = NULL;
    int32_t ret;
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    oal_wireless_dev_stru *pst_wdev = NULL;

    if (oal_unlikely(net_dev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_stop_vap::net_dev null!}");
        return -OAL_EFAUL;
    }

    if ((oal_netdevice_flags(net_dev) & OAL_IFF_RUNNING) == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_stop_vap::vap is already down,continue to reset hmac vap state.}");
    }

    oal_io_print("wal_stop_vap,dev_name is:%s\n", net_dev->name);

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_DOWN_VAP, sizeof(mac_cfg_down_vap_param_stru));
    ((mac_cfg_down_vap_param_stru *)write_msg.auc_value)->pst_net_dev = net_dev;
    pst_wdev = net_dev->ieee80211_ptr;
    en_p2p_mode = wal_wireless_iftype_to_mac_p2p_mode(pst_wdev->iftype);
    if (WLAN_P2P_BUTT == en_p2p_mode) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_stop_vap::wal_wireless_iftype_to_mac_p2p_mode return BUFF}");
        return -OAL_EINVAL;
    }
    ((mac_cfg_start_vap_param_stru *)write_msg.auc_value)->en_p2p_mode = en_p2p_mode;

    oam_warning_log1(0, OAM_SF_ANY, "{wal_stop_vap::en_p2p_mode:%d}", en_p2p_mode);

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_down_vap_param_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_stop_vap::wal_alloc_cfg_event return err code %d!}", ret);
        return ret;
    }

    if (OAL_SUCC != wal_check_and_release_msg_resp(rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_stop_vap::wal_check_and_release_msg_resp fail");
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
int32_t wal_init_WiTAS_state(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t wifi_tas_state = 0;
    int32_t ret;
    struct device_node *np = NULL;

    if ((g_tas_switch_en[WLAN_RF_CHANNEL_ZERO] == OAL_FALSE) &&
        (g_tas_switch_en[WLAN_RF_CHANNEL_ONE] == OAL_FALSE)) {
        oam_warning_log0(0, OAM_SF_ANY, "{not support WiTAS}");
        return BOARD_SUCC;
    }
    ret = get_board_dts_node(&np, DTS_NODE_HI110X_WIFI);
    if (ret != BOARD_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{DTS read node hisi_wifi fail!}");
        return BOARD_FAIL;
    }
    ret = of_property_read_u32(np, DTS_PROP_HI110X_WIFI_TAS_STATE, &wifi_tas_state);
    if (ret) {
        oam_warning_log0(0, OAM_SF_ANY, "{read prop gpio_wifi_tas_state fail!}");
        return BOARD_SUCC;
    }
    if (board_get_wifi_tas_gpio_state() != wifi_tas_state) {
        return board_wifi_tas_set(wifi_tas_state);
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{current WiTAS state is right, no need to set again!}");
        return BOARD_SUCC;
    }
#else
    return BOARD_SUCC;
#endif
}
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

#define WAL_PROBE_RESP_LEN 4
OAL_STATIC void wal_init_probe_resp_status(mac_cfg_add_vap_param_stru *write_msg)
{
    int32_t ini_probe_resp_mode;
    oal_bool_enum_uint8 probe_resp_enable;
    mac_probe_resp_mode_enum probe_resp_status;

    if (hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
        ini_probe_resp_mode = wlan_chip_get_probe_resp_mode();

        probe_resp_enable  = !!(oal_mask(WAL_PROBE_RESP_LEN, BYTE_OFFSET_4) & (uint32_t)ini_probe_resp_mode);
        probe_resp_status  = (oal_mask(WAL_PROBE_RESP_LEN, BYTE_OFFSET_0) & (uint32_t)ini_probe_resp_mode);

        write_msg->probe_resp_enable = probe_resp_enable;
        if (probe_resp_status >= MAC_PROBE_RESP_MODE_BUTT) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_init_probe_resp_status::read ini->probe_resp_status error[%d]}",
                probe_resp_status);
            probe_resp_status = MAC_PROBE_RESP_MODE_ACTIVE;
        }
        write_msg->probe_resp_status = probe_resp_status;
    }
}
#endif

static int32_t wal_wlan_vap_init_get_base_dev_mac_stru(oal_net_device_stru *net_dev, mac_vap_stru **mac_vap,
                                                       oal_wireless_dev_stru **pst_wdev,
                                                       mac_wiphy_priv_stru **wiphy_priv,
                                                       mac_device_stru **mac_device)
{
    *mac_vap = oal_net_dev_priv(net_dev);
    if (*mac_vap != NULL) {
        if ((*mac_vap)->en_vap_state != MAC_VAP_STATE_BUTT) {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_wlan_vap_init_get_base_dev_mac_stru::mac_vap is already exist}");
            return OAL_SUCC;
        }
        /* netdev下的vap已经被删除，需要重新创建和挂载 */
        oam_warning_log0((*mac_vap)->uc_vap_id, OAM_SF_ANY,
                         "{wal_wlan_vap_init_get_base_dev_mac_stru::mac_vap is already free!}");
        oal_net_dev_priv(net_dev) = NULL;
    }
    *pst_wdev = oal_netdevice_wdev(net_dev);
    if (*pst_wdev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_wlan_vap_init_get_base_dev_mac_stru::pst_wdev is null!}");
        return -OAL_EFAUL;
    }

    *wiphy_priv = (mac_wiphy_priv_stru *)oal_wiphy_priv((*pst_wdev)->wiphy);
    *mac_device = (*wiphy_priv)->pst_mac_device;
    if (*mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_wlan_vap_init_get_base_dev_mac_stru::pst_mac_device is null!}");
        return -OAL_EFAUL;
    }
    return OAL_SUCC_GO_ON;
}

static int32_t wal_wlan_vap_init_get_cfg_stru(mac_device_stru *mac_device, mac_vap_stru **cfg_mac_vap,
                                              hmac_vap_stru **pst_cfg_hmac_vap, oal_net_device_stru **cfg_net_dev,
                                              wal_msg_write_stru *write_msg)
{
    *cfg_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(mac_device->uc_cfg_vap_id);
    *pst_cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_device->uc_cfg_vap_id);
    if (oal_any_null_ptr2(*cfg_mac_vap, *pst_cfg_hmac_vap)) {
        oam_warning_log2(0, OAM_SF_ANY,
            "{wal_wlan_vap_init_get_cfg_stru::cfg_mac_vap or pst_cfg_hmac_vap is null! vap_id:%d,deviceid[%d]}",
            mac_device->uc_cfg_vap_id, mac_device->uc_device_id);
        return -OAL_EFAUL;
    }

    *cfg_net_dev = (*pst_cfg_hmac_vap)->pst_net_device;
    if (*cfg_net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_wlan_vap_init_get_cfg_stru::cfg_net_dev is null!}");
        return -OAL_EFAUL;
    }
    WAL_WRITE_MSG_HDR_INIT(write_msg, WLAN_CFGID_ADD_VAP, sizeof(mac_cfg_add_vap_param_stru));
    return OAL_SUCC_GO_ON;
}

static int32_t wal_wlan_mac_cfg_add_vap_param_set(mac_device_stru *mac_device,
                                                  mac_cfg_add_vap_param_stru *add_vap_param,
                                                  oal_wireless_dev_stru *pst_wdev, oal_net_device_stru *net_dev)
{
    int32_t ret;
    int8_t ac_wlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    int8_t ac_p2p_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    char *wlan_netdev_name = ac_wlan_netdev_name;
    char *p2p_netdev_name = ac_p2p_netdev_name;
    /* 通过device id获取wlanX和 p2pX的netdev名 */
    snprintf_s(wlan_netdev_name, NET_DEV_NAME_LEN, NET_DEV_NAME_LEN - 1, "wlan%d", mac_device->uc_device_id);
    snprintf_s(p2p_netdev_name, NET_DEV_NAME_LEN, NET_DEV_NAME_LEN - 1, "p2p%d", mac_device->uc_device_id);
    add_vap_param->en_p2p_mode = WLAN_LEGACY_VAP_MODE;
    /* 仅用于WIFI和AP打开时创建VAP */
    if ((NL80211_IFTYPE_STATION == pst_wdev->iftype) || (NL80211_IFTYPE_P2P_DEVICE == pst_wdev->iftype)) {
        if (oal_any_zero_value2(oal_strcmp(wlan_netdev_name, net_dev->name),
            oal_strcmp(WLAN1_NETDEV_NAME, net_dev->name))) {
            add_vap_param->en_vap_mode = WLAN_VAP_MODE_BSS_STA;
        } else if (0 == (oal_strcmp(p2p_netdev_name, net_dev->name))) {
            add_vap_param->en_vap_mode = WLAN_VAP_MODE_BSS_STA;
            add_vap_param->en_p2p_mode = WLAN_P2P_DEV_MODE;
        } else {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_wlan_mac_cfg_add_vap_param_set::net_dev is not wlan or p2p[%d]!}",
                mac_device->uc_device_id);
            return OAL_SUCC;
        }
    } else if (NL80211_IFTYPE_AP == pst_wdev->iftype) {
        add_vap_param->en_vap_mode = WLAN_VAP_MODE_BSS_AP;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_wlan_mac_cfg_add_vap_param_set::net_dev is not wlan0 or p2p0!}");
        return OAL_SUCC;
    }

    oam_warning_log2(0, OAM_SF_ANY, "{wal_wlan_mac_cfg_add_vap_param_set::vap_mode:%d, p2p_mode:%d}",
        add_vap_param->en_vap_mode, add_vap_param->en_p2p_mode);

    if (oal_strcmp(wlan_netdev_name, net_dev->name) == 0) {
        add_vap_param->is_primary_vap = 1;
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
        ret = wal_init_WiTAS_state();
        if (ret != BOARD_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_init_WiTAS_state ini WiTAS state fail!}");
        }
#endif
    } else {
        add_vap_param->is_primary_vap = 0;
    }
    return OAL_SUCC_GO_ON;
}

static void wal_wlan_vap_init_post_event_msg(oal_net_device_stru *net_dev,
    mac_cfg_add_vap_param_stru *add_vap_param, mac_vap_stru *cfg_mac_vap, wal_msg_write_stru *write_msg)
{
    /* 抛事件到wal层处理 */
    add_vap_param->pst_net_dev = net_dev;
    add_vap_param->uc_cfg_vap_indx = cfg_mac_vap->uc_vap_id;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wal_vap_get_wlan_mode_para(write_msg);
    wal_init_probe_resp_status((mac_cfg_add_vap_param_stru *)write_msg->auc_value);
#endif
}

static int32_t wal_init_wlan_vap_post_event_deal(oal_net_device_stru *net_dev, oal_net_device_stru *cfg_net_dev,
                                                 mac_vap_stru *cfg_mac_vap, wal_msg_write_stru *write_msg)
{
    uint32_t err_code;
    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;
    /* 发送消息 */
    ret = wal_send_cfg_event(cfg_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_add_vap_param_stru),
        (uint8_t *)write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(cfg_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_init_wlan_vap_post_event_deal::return err:%d!}", ret);
        return -OAL_EFAIL;
    }
    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(cfg_mac_vap->uc_vap_id, 0,
                         "{wal_init_wlan_vap_post_event_deal::hmac add vap fail[%u]}", err_code);
        return -OAL_EFAIL;
    }

    if (OAL_SUCC != wal_set_random_mac_to_mib(net_dev)) {
        oam_warning_log0(cfg_mac_vap->uc_vap_id, 0, "{wal_init_wlan_vap_post_event_deal::set_random_mac_to_mib fail}");
        return -OAL_EFAUL;
    }
    return OAL_SUCC_GO_ON;
}

static void wal_init_wlan_ap_mode_set_max_user_count_and_mac_filter(oal_net_device_stru *net_dev,
                                                                    oal_wireless_dev_stru *pst_wdev)
{
    if (NL80211_IFTYPE_AP == pst_wdev->iftype) {
        /* AP模式初始化，初始化配置最大用户数和mac地址过滤模式 */
        if (g_st_ap_config_info.ap_max_user > 0) {
            wal_set_ap_max_user(net_dev, g_st_ap_config_info.ap_max_user);
        }

        if (OAL_STRLEN(g_st_ap_config_info.ac_ap_mac_filter_mode) > 0) {
            wal_config_mac_filter(net_dev, (int8_t *)g_st_ap_config_info.ac_ap_mac_filter_mode);
        }
    }
}
#ifdef _PRE_WLAN_CHBA_MGMT

static int32_t wal_start_chba_cfg(oal_net_device_stru *net_dev, uint8_t vap_id)
{
    uint32_t ret;
    /* 1、设置ssid */
    ret = wal_hipriv_set_essid(net_dev, "0000chba");
    if (ret != OAL_SUCC) {
        oam_warning_log1(vap_id, OAM_SF_ANY, "{wal_start_chba_cfg::fail send ssid cfg msg, err[%d]}", ret);
        return -OAL_EFAIL;
    }
    /* 2、设置信道带宽 */
    ret = wal_chba_set_channel(net_dev);
    if (ret != OAL_SUCC) {
        oam_warning_log1(vap_id, OAM_SF_ANY, "{wal_start_chba_cfg::fail set channel, err[%d]}", ret);
        return -OAL_EFAIL;
    }
    /* 3、设置mode */
    ret = wal_ioctl_set_mode(net_dev, "11ac80plusplus");
    if (ret != OAL_SUCC) {
        oam_warning_log1(vap_id, OAM_SF_ANY, "{wal_start_chba_cfg::fail set mode, err[%d]}", ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

int32_t wal_setup_chba(oal_net_device_stru *net_dev)
{
    int32_t ret;
    mac_vap_stru *mac_vap = NULL;
    oal_netdev_priv_stru *net_dev_priv = NULL;
    uint8_t vap_id;
    if (oal_netdevice_flags(net_dev) & OAL_IFF_RUNNING) {
        /* 切换到AP前如果网络设备处于UP状态，需要先down wlan0网络设备 */
        oal_io_print("wal_setup_chba:stop netdevice:%.16s", net_dev->name);
        oam_warning_log0(0, OAM_SF_CFG, "{wal_setup_chba:: oal_iff_running! now, stop netdevice}");
        wal_netdev_stop(net_dev);
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap) {
        if (mac_vap->en_vap_state != MAC_VAP_STATE_INIT) {
            /* 切换到AP前如果网络设备处于UP状态，需要先down wlan0网络设备 */
            oal_io_print("wal_setup_chba:stop netdevice:%s", net_dev->name);
            wal_netdev_stop(net_dev);
        }
    }
    net_dev->ieee80211_ptr->iftype = NL80211_IFTYPE_AP;
    /* add vap */
    ret = wal_init_wlan_vap(net_dev);
    if (ret == OAL_SUCC) {
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        /* chba关闭低功耗 */
        wlan_pm_disable();
#endif
    } else {
        return ret;
    }
    /* 获取vap */
    /* 获取vap id */
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_setup_chba::pst_mac_vap null}");
        return -OAL_EINVAL;
    }
    vap_id = mac_vap->uc_vap_id;
    /* 配置ap参数 */
    ret = wal_start_chba_cfg(net_dev, vap_id);
    /* start vap */
    ret = wal_start_vap(net_dev);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(vap_id, OAM_SF_ANY, "{wal_setup_chba::failed to start ap, error[%d]}", ret);
        return -OAL_EFAIL;
    }
    net_dev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(net_dev);
    if ((net_dev_priv->uc_napi_enable) && (!net_dev_priv->uc_state)) {
        oal_napi_enable(&net_dev_priv->st_napi);
        net_dev_priv->uc_state = 1;
    }
    oam_warning_log0(vap_id, OAM_SF_ANY, "{wal_setup_chba:: start ap}");
    return OAL_SUCC;
}
#endif

int32_t wal_init_wlan_vap(oal_net_device_stru *net_dev)
{
    oal_net_device_stru *cfg_net_dev = NULL;
    mac_vap_stru *mac_vap = NULL;
    oal_wireless_dev_stru *pst_wdev = NULL;
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    mac_vap_stru *cfg_mac_vap = NULL;
    hmac_vap_stru *pst_cfg_hmac_vap = NULL;
    mac_device_stru *pst_mac_device = NULL;
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    mac_cfg_add_vap_param_stru *add_vap_param = NULL;
    ret = wal_wlan_vap_init_get_base_dev_mac_stru(net_dev, &mac_vap, &pst_wdev, &wiphy_priv, &pst_mac_device);
    if (ret != OAL_SUCC_GO_ON) {
        return ret;
    }

    ret = wal_wlan_vap_init_get_cfg_stru(pst_mac_device, &cfg_mac_vap, &pst_cfg_hmac_vap, &cfg_net_dev, &write_msg);
    if (ret != OAL_SUCC_GO_ON) {
        return ret;
    }

    add_vap_param = (mac_cfg_add_vap_param_stru *)write_msg.auc_value;

    ret = wal_wlan_mac_cfg_add_vap_param_set(pst_mac_device, add_vap_param, pst_wdev, net_dev);
    if (ret != OAL_SUCC_GO_ON) {
        return ret;
    }

    wal_wlan_vap_init_post_event_msg(net_dev, add_vap_param, cfg_mac_vap, &write_msg);

    ret = wal_init_wlan_vap_post_event_deal(net_dev, cfg_net_dev, cfg_mac_vap, &write_msg);
    if (ret != OAL_SUCC_GO_ON) {
        return ret;
    }

    /* 设置netdevice的MAC地址，MAC地址在HMAC层被初始化到MIB中 */
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_init_wlan_vap::OAL_NET_DEV_PRIV(net_dev) null.}");
        return -OAL_EINVAL;
    }

    if (add_vap_param->en_p2p_mode == WLAN_P2P_DEV_MODE) {
        pst_mac_device->st_p2p_info.uc_p2p0_vap_idx = mac_vap->uc_vap_id;
    }
    wal_init_wlan_ap_mode_set_max_user_count_and_mac_filter(net_dev, pst_wdev);
    return OAL_SUCC;
}


int32_t wal_deinit_wlan_vap(oal_net_device_stru *net_dev)
{
    wal_msg_write_stru write_msg = {0};
    wal_msg_stru *rsp_msg = NULL;
    mac_vap_stru *mac_vap = NULL;
    int32_t ret;
    int32_t l_del_vap_flag = OAL_TRUE;
    int8_t ac_wlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    int8_t ac_p2p_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    mac_device_stru *pst_mac_dev = NULL;
    wlan_p2p_mode_enum_uint8 en_p2p_mode = WLAN_LEGACY_VAP_MODE;

    if (oal_unlikely(net_dev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_deinit_wlan_vap::pst_del_vap_param null ptr !}");
        return -OAL_EINVAL;
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_deinit_wlan_vap::mac_vap is already null}");
        return OAL_SUCC;
    }

    pst_mac_dev = mac_res_get_dev(mac_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "wal_deinit_wlan_vap::get mac dev id[%d] FAIL", mac_vap->uc_device_id);
        return -OAL_EFAIL;
    }
    /* 通过device id获取netdev名字 */
    snprintf_s(ac_wlan_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, MAC_NET_DEVICE_NAME_LENGTH - 1,
               "wlan%d", pst_mac_dev->uc_device_id);
    snprintf_s(ac_p2p_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, MAC_NET_DEVICE_NAME_LENGTH - 1,
               "p2p%d", pst_mac_dev->uc_device_id);

    /* 仅用于WIFI和AP关闭时删除VAP */
    if ((0 != (oal_strcmp(ac_wlan_netdev_name, net_dev->name)))
        && (0 != (oal_strcmp(ac_p2p_netdev_name, net_dev->name)))
        && (0 != (oal_strcmp(WLAN1_NETDEV_NAME, net_dev->name)))
        && ((oal_strcmp("chba0", net_dev->name)) != 0)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_deinit_wlan_vap::net_dev is not wlan or p2p!}");
        return OAL_SUCC;
    }

    if (0 == oal_strcmp(ac_p2p_netdev_name, net_dev->name)) {
        en_p2p_mode = WLAN_P2P_DEV_MODE;
    }

    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_deinit_wlan_vap::en_p2p_mode:%d}", en_p2p_mode);

    /* 抛事件到wal层处理 */
    memset_s(&write_msg, sizeof(wal_msg_write_stru), 0, sizeof(wal_msg_write_stru));

    ((mac_cfg_del_vap_param_stru *)write_msg.auc_value)->pst_net_dev = net_dev;
    ((mac_cfg_del_vap_param_stru *)write_msg.auc_value)->en_p2p_mode = en_p2p_mode;

    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_DEL_VAP, sizeof(mac_cfg_del_vap_param_stru));
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_del_vap_param_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (OAL_SUCC != wal_check_and_release_msg_resp(rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_deinit_wlan_vap::wal_check_and_release_msg_resp fail.");
        /* can't set net dev's vap ptr to null when
          del vap wid process failed! */
        l_del_vap_flag = OAL_FALSE;
    }

    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_deinit_wlan_vap::return error code %d}", ret);
        if (ret == -OAL_ENOMEM || ret == -OAL_EFAIL) {
            /* wid had't processed */
            l_del_vap_flag = OAL_FALSE;
        }
    }

    if (l_del_vap_flag == OAL_TRUE) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_deinit_wlan_vap::clear netdev priv.}");
        oal_net_dev_priv(net_dev) = NULL;
    }

    return ret;
}


OAL_STATIC int32_t wal_set_mac_addr(oal_net_device_stru *net_dev)
{
    uint8_t auc_primary_mac_addr[WLAN_MAC_ADDR_LEN] = { 0 }; /* MAC地址 */
    oal_wireless_dev_stru *pst_wdev;
    mac_wiphy_priv_stru *wiphy_priv;
    mac_device_stru *pst_mac_device;

    pst_wdev = oal_netdevice_wdev(net_dev);
    wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(pst_wdev->wiphy));
    pst_mac_device = wiphy_priv->pst_mac_device;

    if (oal_unlikely(pst_mac_device->st_p2p_info.pst_primary_net_device == NULL)) {
        /* random mac will be used. hi1102-cb (#include <linux/etherdevice.h>)    */
        oal_random_ether_addr(auc_primary_mac_addr);
        auc_primary_mac_addr[BYTE_OFFSET_0] &= (~0x02);
        auc_primary_mac_addr[BYTE_OFFSET_1] = 0x11;
        auc_primary_mac_addr[BYTE_OFFSET_2] = 0x02;
    } else {
#ifndef _PRE_PC_LINT
        if (oal_likely(NULL != oal_netdevice_mac_addr(pst_mac_device->st_p2p_info.pst_primary_net_device))) {
            if (EOK != memcpy_s(auc_primary_mac_addr, WLAN_MAC_ADDR_LEN,
                oal_netdevice_mac_addr(pst_mac_device->st_p2p_info.pst_primary_net_device), WLAN_MAC_ADDR_LEN)) {
                oam_error_log0(0, OAM_SF_ANY, "wal_set_mac_addr::memcpy fail!");
                return OAL_FAIL;
            }
        } else {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_get_mac_addr() pst_primary_net_device; addr is null}");
            return OAL_FAIL;
        }
#endif
    }

    switch (pst_wdev->iftype) {
        case NL80211_IFTYPE_P2P_DEVICE: {
            /* 产生P2P device MAC 地址，将本地mac 地址bit 设置为1 */
            auc_primary_mac_addr[0] |= 0x02;
            oal_set_mac_addr((uint8_t *)oal_netdevice_mac_addr(net_dev), auc_primary_mac_addr);
            break;
        }
        default: {
            if (oal_strcmp(WLAN1_NETDEV_NAME, net_dev->name) == 0) {
                auc_primary_mac_addr[3] ^= 0x80; // 0x80代表mac地址第四个数值
            } else {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
                hwifi_get_mac_addr(auc_primary_mac_addr);
                auc_primary_mac_addr[0] &= (~0x02);
#else
                oal_random_ether_addr(auc_primary_mac_addr);
                auc_primary_mac_addr[0] &= (~0x02);
                auc_primary_mac_addr[1] = 0x11; // 0x11代表mac地址第二个数值
                auc_primary_mac_addr[2] = 0x02; // 0x02代表mac地址第三个数值
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
            }
            oal_set_mac_addr((uint8_t *)oal_netdevice_mac_addr(net_dev), auc_primary_mac_addr);
            break;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC void wal_set_netdev_priv(oal_netdev_priv_stru *pst_netdev_priv)
{
#ifdef _PRE_CONFIG_HISI_110X_NAPI_DISABLE
    pst_netdev_priv->uc_napi_enable     = OAL_FALSE;
    pst_netdev_priv->uc_gro_enable      = OAL_FALSE;
#else
    pst_netdev_priv->uc_napi_enable = OAL_TRUE;
    pst_netdev_priv->uc_gro_enable = OAL_TRUE;
#endif
    pst_netdev_priv->uc_napi_weight = NAPI_POLL_WEIGHT_LEV1;
    pst_netdev_priv->queue_len_max = NAPI_NETDEV_PRIV_QUEUE_LEN_MAX;
    pst_netdev_priv->uc_state = 0;
    pst_netdev_priv->period_pkts = 0;
    pst_netdev_priv->period_start = 0;
}


OAL_STATIC void wal_init_wlan_netdev_set_netdev(oal_net_device_stru *net_dev, oal_wireless_dev_stru *pst_wdev)
{
#ifdef _PRE_WLAN_FEATURE_GSO
    oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::[GSO] add NETIF_F_SG}");
    net_dev->features |= NETIF_F_SG;
    net_dev->hw_features |= NETIF_F_SG;
#endif
 /* 对netdevice进行赋值 */
#ifdef CONFIG_WIRELESS_EXT
    net_dev->wireless_handlers = &g_st_iw_handler_def;
#endif
    net_dev->netdev_ops = &g_st_wal_net_dev_ops;
    net_dev->ethtool_ops = &g_st_wal_ethtool_ops;
    oal_netdevice_destructor(net_dev) = oal_net_free_netdev;

    /* host发送报文使能csum硬化功能 */
    if (g_trx_switch.tx_switch == HOST_TX) {
        net_dev->features    |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
        net_dev->hw_features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
    }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 44))
    oal_netdevice_master(net_dev) = NULL;
#endif

    oal_netdevice_ifalias(net_dev) = NULL;
    oal_netdevice_watchdog_timeo(net_dev) = 5;
    oal_netdevice_wdev(net_dev) = pst_wdev;
    oal_netdevice_qdisc(net_dev, NULL);
}
#ifdef _PRE_WLAN_CHBA_MGMT

static void wal_set_chba_mac_addr(oal_net_device_stru *net_dev)
{
    uint8_t primary_mac_addr[WLAN_MAC_ADDR_LEN] = { 0 }; /* MAC地址 */
    oal_random_ether_addr(primary_mac_addr);
    primary_mac_addr[BYTE_OFFSET_0] &= (~0x02);
    primary_mac_addr[BYTE_OFFSET_1] = 0x11;
    primary_mac_addr[BYTE_OFFSET_2] = 0x05;
    oal_set_mac_addr((uint8_t *)oal_netdevice_mac_addr(net_dev), primary_mac_addr);
}

static void wal_chba_netdev_set(oal_net_device_stru *net_dev, oal_wireless_dev_stru *wdev, oal_wiphy_stru *wiphy)
{
    oal_netdev_priv_stru *net_dev_priv = NULL;
    memset_s(wdev, sizeof(oal_wireless_dev_stru), 0, sizeof(oal_wireless_dev_stru));
    wal_init_wlan_netdev_set_netdev(net_dev, wdev);
    wdev->netdev = net_dev;
    wdev->iftype = NL80211_IFTYPE_AP;
    wdev->wiphy = wiphy;
    oal_netdevice_flags(net_dev) &= ~OAL_IFF_RUNNING; /* 将net device的flag设为down */
    wal_set_chba_mac_addr(net_dev);
    net_dev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(net_dev);
    wal_set_netdev_priv(net_dev_priv);

    oal_netbuf_list_head_init(&net_dev_priv->st_rx_netbuf_queue);
    oal_netif_napi_add(net_dev, &net_dev_priv->st_napi, hmac_rxdata_polling, NAPI_POLL_WEIGHT_LEV1);
}


int32_t wal_init_chba_netdev(oal_wiphy_stru *wiphy, const char *dev_name)
{
    mac_device_stru *mac_device = NULL;
    oal_net_device_stru *net_dev = NULL;
    oal_wireless_dev_stru *wdev = NULL;
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    int32_t ret;
    if ((g_optimized_feature_switch_bitmap & BIT(CUSTOM_OPTIMIZE_CHBA)) == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_chba_netdev::CHBA unsupported, Not create chba netdev!!}");
        return OAL_SUCC;
    }

    wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(wiphy));
    if (wiphy_priv == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_chba_netdev::wiphy_priv is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_device = wiphy_priv->pst_mac_device;
    if (mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_chba_netdev::mac_device is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 创建netdev */
    /* 如果创建的net device已经存在，直接返回 */
    /* 根据dev_name找到dev */
    net_dev = wal_config_get_netdev(dev_name, OAL_STRLEN(dev_name));
    if (net_dev != NULL) {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(net_dev);
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_chba_netdev::the net_device is already exist!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 此函数第一个入参代表私有长度，此处不涉及为0 */
    net_dev = oal_net_alloc_netdev_mqs(sizeof(oal_netdev_priv_stru),
        dev_name, oal_ether_setup, WLAN_NET_QUEUE_BUTT, 1);
    if (oal_unlikely(net_dev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_init_chba_netdev::oal_net_alloc_netdev return null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wdev = (oal_wireless_dev_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL,
        sizeof(oal_wireless_dev_stru), OAL_FALSE);
    if (oal_unlikely(wdev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_init_chba_netdev::alloc mem, wdev null!}");
        oal_net_free_netdev(net_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }
    wal_chba_netdev_set(net_dev, wdev, wiphy);
    /* 注册net_device */
    ret = hmac_net_register_netdev(net_dev);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_init_chba_netdev::hmac_net_register_netdev err:%d!}", ret);
        oal_mem_free_m(wdev, OAL_FALSE);
        oal_net_free_netdev(net_dev);
        return ret;
    }
    oam_warning_log0(0, OAM_SF_ANY, "{wal_init_chba_netdev::succ!}");
    return OAL_SUCC;
}
#endif

int32_t wal_init_wlan_netdev(oal_wiphy_stru *pst_wiphy, const char *dev_name)
{
    mac_device_stru *pst_mac_device = NULL;
    int8_t ac_wlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    int8_t ac_p2p_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    oal_net_device_stru *net_dev = NULL;
    oal_wireless_dev_stru *pst_wdev = NULL;
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    enum nl80211_iftype en_type;
    int32_t ret;
    oal_netdev_priv_stru *pst_netdev_priv = NULL;

    if (oal_any_null_ptr2(pst_wiphy, dev_name)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::pst_wiphy or dev_name is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(pst_wiphy));
    if (wiphy_priv == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::wiphy_priv is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = wiphy_priv->pst_mac_device;
    if (pst_mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::wiphy_priv is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 通过device id获取netdev名字 */
    snprintf_s(ac_wlan_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, MAC_NET_DEVICE_NAME_LENGTH - 1,
               "wlan%d", pst_mac_device->uc_device_id);
    snprintf_s(ac_p2p_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, MAC_NET_DEVICE_NAME_LENGTH - 1,
               "p2p%d", pst_mac_device->uc_device_id);

    /* 信道跟随--添加hwlan name判断 */
    if (0 == (oal_strcmp(ac_wlan_netdev_name, dev_name)) || 0 == (oal_strcmp(WLAN1_NETDEV_NAME, dev_name))) {
        en_type = NL80211_IFTYPE_STATION;
    } else if (0 == (oal_strcmp(ac_p2p_netdev_name, dev_name))) {
        en_type = NL80211_IFTYPE_P2P_DEVICE;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::dev name is not wlan or p2p}");
        return OAL_SUCC;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{wal_init_wlan_netdev::en_type is %d,2:sta 10:p2p 3:ap}", en_type);

    /* 如果创建的net device已经存在，直接返回 */
    /* 根据dev_name找到dev */
    net_dev = wal_config_get_netdev(dev_name, OAL_STRLEN(dev_name));
    if (net_dev != NULL) {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(net_dev);

        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::the net_device is already exist!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 此函数第一个入参代表私有长度，此处不涉及为0 */
    net_dev = oal_net_alloc_netdev_mqs(sizeof(oal_netdev_priv_stru),
        dev_name, oal_ether_setup, WLAN_NET_QUEUE_BUTT, 1);
    if (oal_unlikely(net_dev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::oal_net_alloc_netdev return null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_wdev = (oal_wireless_dev_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL,
        sizeof(oal_wireless_dev_stru), OAL_FALSE);
    if (oal_unlikely(pst_wdev == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::alloc mem, pst_wdev null!}");
        oal_net_free_netdev(net_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(pst_wdev, sizeof(oal_wireless_dev_stru), 0, sizeof(oal_wireless_dev_stru));
    wal_init_wlan_netdev_set_netdev(net_dev, pst_wdev);
    pst_wdev->netdev = net_dev;
    pst_wdev->iftype = en_type;
    pst_wdev->wiphy = pst_wiphy;
    wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(pst_wiphy));

    /* 信道跟随--add wlan name 判断 */
    if ((NL80211_IFTYPE_STATION == en_type) && (0 == (oal_strcmp(ac_wlan_netdev_name, dev_name)))) {
        /* 如果创建wlan0， 则保存wlan0 为主net_device,p2p0 和p2p-p2p0 MAC 地址从主netdevice 获取 */
        wiphy_priv->pst_mac_device->st_p2p_info.pst_primary_net_device = net_dev;
    } else if ((NL80211_IFTYPE_STATION == en_type) && (0 == (oal_strcmp(WLAN1_NETDEV_NAME, dev_name)))) {
        /* netdevice 指针暂时挂载，未使用 */
        wiphy_priv->pst_mac_device->st_p2p_info.pst_second_net_device = net_dev;
    } else if (NL80211_IFTYPE_P2P_DEVICE == en_type) {
        wiphy_priv->pst_mac_device->st_p2p_info.pst_p2p_net_device = net_dev;
    }
    oal_netdevice_flags(net_dev) &= ~OAL_IFF_RUNNING; /* 将net device的flag设为down */

    if (OAL_SUCC != wal_set_mac_addr(net_dev)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::set mac addr fail}");
    }

    pst_netdev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(net_dev);
    wal_set_netdev_priv(pst_netdev_priv);

    oal_netbuf_list_head_init(&pst_netdev_priv->st_rx_netbuf_queue);
    oal_netif_napi_add(net_dev, &pst_netdev_priv->st_napi, hmac_rxdata_polling, NAPI_POLL_WEIGHT_LEV1);
    /* 注册net_device */
    ret = hmac_net_register_netdev(net_dev);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_init_wlan_netdev::hmac_net_register_netdev err:%d!}", ret);

        oal_mem_free_m(pst_wdev, OAL_FALSE);
        oal_net_free_netdev(net_dev);

        return ret;
    }

    return OAL_SUCC;
}


int32_t wal_setup_ap(oal_net_device_stru *net_dev)
{
    int32_t ret;
    mac_vap_stru *mac_vap = NULL;
    wal_set_power_mgmt_on(OAL_FALSE);
    ret = wal_set_power_on(net_dev, OAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_setup_ap::wal_set_power_on fail [%d]!}", ret);
        return ret;
    }
    if (oal_netdevice_flags(net_dev) & OAL_IFF_RUNNING) {
        /* 切换到AP前如果网络设备处于UP状态，需要先down wlan0网络设备 */
        oal_io_print("wal_setup_ap:stop netdevice:%.16s", net_dev->name);
        oam_warning_log0(0, OAM_SF_CFG, "{wal_setup_ap:: oal_iff_running! now, stop netdevice}");
        wal_netdev_stop(net_dev);
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap != NULL) {
        if (mac_vap->en_vap_state != MAC_VAP_STATE_INIT) {
            /* 切换到AP前如果网络设备处于UP状态，需要先down wlan0网络设备 */
            oal_io_print("wal_setup_ap:stop netdevice:%s", net_dev->name);
            wal_netdev_stop(net_dev);
        }
    }

    net_dev->ieee80211_ptr->iftype = NL80211_IFTYPE_AP;

    ret = wal_init_wlan_vap(net_dev);
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if (ret == OAL_SUCC) {
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        /* 低功耗定制化开关 */
        (g_wlan_host_pm_switch == OAL_TRUE) ? wlan_pm_enable() : wlan_pm_disable();
#endif
    }
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
    return ret;
}
#ifdef _PRE_WLAN_FEATURE_NAN
OAL_STATIC void wal_nan_netdev_setup(oal_net_device_stru *net_dev, oal_wireless_dev_stru *wdev,
    oal_wiphy_stru *wiphy)
{
    memset_s(wdev, sizeof(oal_wireless_dev_stru), 0, sizeof(oal_wireless_dev_stru));
    wdev->netdev = net_dev;
    wdev->iftype = NL80211_IFTYPE_STATION;
    wdev->wiphy = wiphy;

    net_dev->netdev_ops = &g_nan_net_dev_ops;
    net_dev->ethtool_ops = &g_st_wal_ethtool_ops;
    oal_netdevice_destructor(net_dev) = oal_net_free_netdev;
    oal_netdevice_ifalias(net_dev) = NULL;
    oal_netdevice_watchdog_timeo(net_dev) = 5;
    oal_netdevice_wdev(net_dev) = wdev;
    oal_netdevice_qdisc(net_dev, NULL);
    /* 将net device的flag设为down */
    oal_netdevice_flags(net_dev) &= ~OAL_IFF_RUNNING;
}


void wal_init_nan_netdev(oal_wiphy_stru *wiphy)
{
    oal_net_device_stru *net_dev = NULL;
    oal_wireless_dev_stru *wdev = NULL;
    int32_t ret;
    const char *dev_name = "nan0";

    if ((g_optimized_feature_switch_bitmap & BIT(CUSTOM_OPTIMIZE_FEATURE_NAN)) == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_nan_netdev::NAN unsupported, Not create nan netdev!}");
        return;
    }
#if defined(CONFIG_HISI_CMDLINE_PARSE) || defined(CONFIG_CMDLINE_PARSE)
    if (get_boot_into_recovery_flag()) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_nan_netdev::recovery mode, Not create nan netdev!}");
        return;
    }
#endif
    /* 如果创建的net device已经存在，直接返回 */
    net_dev = wal_config_get_netdev(dev_name, OAL_STRLEN(dev_name));
    if (net_dev != NULL) {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(net_dev);
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_nan_netdev::nan net_device is already exist!}");
        return;
    }

    /* 此函数第一个入参代表私有长度，此处不涉及为0 */
    net_dev = oal_net_alloc_netdev(0, dev_name, oal_ether_setup);
    if (oal_unlikely(net_dev == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_nan_netdev::oal_net_alloc_netdev return null!}");
        return;
    }

    wdev = (oal_wireless_dev_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL,
        sizeof(oal_wireless_dev_stru), OAL_FALSE);
    if (oal_unlikely(wdev == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_nan_netdev::alloc wdev fail!}");
        oal_net_free_netdev(net_dev);
        return;
    }

    wal_nan_netdev_setup(net_dev, wdev, wiphy);

    /* 注册net_device */
    ret = hmac_net_register_netdev(net_dev);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_init_nan_netdev::hmac_net_register_netdev err:%d!}", ret);
        oal_mem_free_m(wdev, OAL_FALSE);
        oal_net_free_netdev(net_dev);
        return;
    }
    oam_warning_log0(0, OAM_SF_ANY, "{wal_init_nan_netdev::add nan netdev succ!}");
}
#endif


uint32_t wal_hipriv_register_inetaddr_notifier(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == register_inetaddr_notifier(&g_wal_hipriv_notifier)) {
        return OAL_SUCC;
    }

    oam_error_log0(0, OAM_SF_PWR, "{wal_hipriv_register_inetaddr_notifier::register inetaddr notifier failed.}");
    return OAL_FAIL;

#else
    return OAL_SUCC;
#endif
}


uint32_t wal_hipriv_unregister_inetaddr_notifier(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == unregister_inetaddr_notifier(&g_wal_hipriv_notifier)) {
        return OAL_SUCC;
    }

    oam_error_log0(0, 0, "{wal_hipriv_unregister_inetaddr_notifier::hmac_unregister inetaddr notifier fail}");
    return OAL_FAIL;

#else
    return OAL_SUCC;
#endif
}


uint32_t wal_hipriv_register_inet6addr_notifier(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == register_inet6addr_notifier(&g_wal_hipriv_notifier_ipv6)) {
        return OAL_SUCC;
    }

    oam_error_log0(0, OAM_SF_PWR, "{wal_hipriv_register_inet6addr_notifier::register inetaddr6 notifier failed.}");
    return OAL_FAIL;

#else
    return OAL_SUCC;
#endif
}


uint32_t wal_hipriv_unregister_inet6addr_notifier(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == unregister_inet6addr_notifier(&g_wal_hipriv_notifier_ipv6)) {
        return OAL_SUCC;
    }

    oam_error_log0(0, OAM_SF_PWR, "{wal_hipriv_unregister_inet6addr_notifier::unreg inetaddr6 notifier fail}");
    return OAL_FAIL;

#else
    return OAL_SUCC;
#endif
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

int32_t wal_hipriv_inetaddr_notifier_call(struct notifier_block *this, unsigned long event, void *ptr)
{
    /*
     * Notification mechanism from kernel to our driver. This function is called by the Linux kernel
     * whenever there is an event related to an IP address.
     * ptr : kernel provided pointer to IP address that has changed
     */
    struct in_ifaddr *pst_ifa = (struct in_ifaddr *)ptr;
    mac_vap_stru *mac_vap = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    if (oal_unlikely(pst_ifa == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call::pst_ifa is NULL.}");
        return NOTIFY_DONE;
    }
    if (oal_unlikely(pst_ifa->ifa_dev->dev == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call::pst_ifa->idev->dev is NULL.}");
        return NOTIFY_DONE;
    }

    /* Filter notifications meant for non Hislicon devices */
    if (pst_ifa->ifa_dev->dev->netdev_ops != &g_st_wal_net_dev_ops) {
        return NOTIFY_DONE;
    }

    mac_vap = (mac_vap_stru *)oal_net_dev_priv(pst_ifa->ifa_dev->dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_warning_log1(0, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call::Get mac vap failed, \
            when %d(UP:1 DOWN:2 UNKNOWN:others) ipv4 address.}", event);
        return NOTIFY_DONE;
    }

    if (ipv4_is_linklocal_169(pst_ifa->ifa_address)) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call:: \
            Invalid IPv4[%d.X.X.%d], MASK[0x%08X].}", ((uint8_t *)&(pst_ifa->ifa_address))[0],
            ((uint8_t *)&(pst_ifa->ifa_address))[3], pst_ifa->ifa_mask); /* 3 netaddr */
        return NOTIFY_DONE;
    }

    wal_wake_lock();

    switch (event) {
        case NETDEV_UP: {
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call:: \
                Up IPv4[%d.X.X.%d], MASK[0x%08X].}", ((uint8_t *)&(pst_ifa->ifa_address))[0],
                ((uint8_t *)&(pst_ifa->ifa_address))[3], pst_ifa->ifa_mask); /* 3 netaddr */
            hmac_arp_offload_set_ip_addr(mac_vap, DMAC_CONFIG_IPV4,
                DMAC_IP_ADDR_ADD, &(pst_ifa->ifa_address), &(pst_ifa->ifa_mask));

            if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
                pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
                if (pst_hmac_vap == NULL) {
                    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                        "{wal_hipriv_inetaddr_notifier_call::hmac_vap null.vap[%d]}", mac_vap->uc_vap_id);
                    break;
                }
                /* 获取到IP地址的时候开启低功耗 */
                if ((pst_hmac_vap->uc_ps_mode == MAX_FAST_PS) || (pst_hmac_vap->uc_ps_mode == AUTO_FAST_PS)) {
                    wlan_pm_set_timeout((g_wlan_min_fast_ps_idle > 1) ?
                        (g_wlan_min_fast_ps_idle - 1) : g_wlan_min_fast_ps_idle);
                } else {
                    wlan_pm_set_timeout(WLAN_SLEEP_DEFAULT_CHECK_CNT);
                }
                pst_hmac_vap->ipaddr_obtained = OAL_TRUE;
                /* 获取到IP地址的时候通知漫游计时 */
                hmac_roam_wpas_connect_state_notify(pst_hmac_vap, WPAS_CONNECT_STATE_IPADDR_OBTAINED);
            }
            break;
        }

        case NETDEV_DOWN: {
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call::\
                Down IPv4[%d.X.X.%d], MASK[0x%08X]..}", ((uint8_t *)&(pst_ifa->ifa_address))[0],
                ((uint8_t *)&(pst_ifa->ifa_address))[3], pst_ifa->ifa_mask); /* 3 netaddr */
            hmac_arp_offload_set_ip_addr(mac_vap, DMAC_CONFIG_IPV4, DMAC_IP_ADDR_DEL,
                &(pst_ifa->ifa_address), &(pst_ifa->ifa_mask));

            if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
                /* 获取到IP地址的时候通知漫游计时 */
                pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
                if (pst_hmac_vap == NULL) {
                    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                        "{wal_hipriv_inetaddr_notifier_call::hmac_vap null.[%d]}", mac_vap->uc_vap_id);
                    break;
                }
                pst_hmac_vap->ipaddr_obtained = OAL_FALSE;
                hmac_roam_wpas_connect_state_notify(pst_hmac_vap, WPAS_CONNECT_STATE_IPADDR_REMOVED);
            }
            break;
        }

        default: {
            oam_error_log1(mac_vap->uc_vap_id, 0, "{wal_hipriv_inetaddr_notifier_call::Unknown event[%d]}", event);
            break;
        }
    }
    wal_wake_unlock();

    return NOTIFY_DONE;
}


int32_t wal_hipriv_inet6addr_notifier_call(struct notifier_block *this, unsigned long event, void *ptr)
{
    /*
     * Notification mechanism from kernel to our driver. This function is called by the Linux kernel
     * whenever there is an event related to an IP address.
     * ptr : kernel provided pointer to IP address that has changed
     */
    struct inet6_ifaddr *pst_ifa = (struct inet6_ifaddr *)ptr;
    mac_vap_stru *mac_vap = NULL;

    if (oal_unlikely(pst_ifa == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call::pst_ifa is NULL.}");
        return NOTIFY_DONE;
    }
    if (oal_unlikely(pst_ifa->idev->dev == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call::pst_ifa->idev->dev is NULL.}");
        return NOTIFY_DONE;
    }

    /* Filter notifications meant for non Hislicon devices */
    if (pst_ifa->idev->dev->netdev_ops != &g_st_wal_net_dev_ops) {
        return NOTIFY_DONE;
    }

    mac_vap = (mac_vap_stru *)oal_net_dev_priv(pst_ifa->idev->dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_warning_log1(0, OAM_SF_PWR,
            "{wal_hipriv_inet6addr_notifier_call::vap null,when %d(UP:1 DOWN:2 UNKNOWN:others) ipv6 addr}", event);
        return NOTIFY_DONE;
    }

    switch (event) {
        case NETDEV_UP: {
            oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_PWR,
                "{wal_hipriv_inet6addr_notifier_call::UP IPv6[%04x:%04x:XXXX:XXXX:XXXX:XXXX:%04x:%04x].}",
                oal_net2host_short((pst_ifa->addr.s6_addr16)[0]), oal_net2host_short((pst_ifa->addr.s6_addr16)[1]),
                oal_net2host_short((pst_ifa->addr.s6_addr16)[6]),  /* 6 net6addr */
                oal_net2host_short((pst_ifa->addr.s6_addr16)[7])); /* 7 net6addr */
            hmac_arp_offload_set_ip_addr(mac_vap, DMAC_CONFIG_IPV6, DMAC_IP_ADDR_ADD,
                &(pst_ifa->addr), &(pst_ifa->addr));
            break;
        }

        case NETDEV_DOWN: {
            oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_PWR,
                "{wal_hipriv_inet6addr_notifier_call::DOWN IPv6[%04x:%04x:XXXX:XXXX:XXXX:XXXX:%04x:%04x].}",
                oal_net2host_short((pst_ifa->addr.s6_addr16)[0]), oal_net2host_short((pst_ifa->addr.s6_addr16)[1]),
                oal_net2host_short((pst_ifa->addr.s6_addr16)[6]),  /* 6 net6addr */
                oal_net2host_short((pst_ifa->addr.s6_addr16)[7])); /* 7 net6addr */
            hmac_arp_offload_set_ip_addr(mac_vap, DMAC_CONFIG_IPV6, DMAC_IP_ADDR_DEL,
                &(pst_ifa->addr), &(pst_ifa->addr));
            break;
        }

        default: {
            oam_error_log1(mac_vap->uc_vap_id, 0, "{wal_hipriv_inet6addr_notifier_call::event:%d}", event);
            break;
        }
    }

    return NOTIFY_DONE;
}
#endif /* #if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) */
#ifdef _PRE_WLAN_FEATURE_11K

OAL_STATIC uint32_t wal_hipriv_send_neighbor_req(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t offset = 0;
    uint32_t result;
    int32_t ret;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    mac_cfg_ssid_param_stru *pst_ssid = NULL;
    uint8_t uc_str_len;

    uc_str_len = os_str_len(pc_param);
    uc_str_len = (uc_str_len > 1) ? uc_str_len - 1 : uc_str_len;

    /* 获取SSID字符串 */
    if (uc_str_len != 0) {
        result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
        if (result != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_neighbor_req::get cmd one arg err[%d]!}", result);
            return result;
        }
    }

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SEND_NEIGHBOR_REQ, sizeof(mac_cfg_ssid_param_stru));
    pst_ssid = (mac_cfg_ssid_param_stru *)write_msg.auc_value;
    pst_ssid->uc_ssid_len = uc_str_len;
    ret = memcpy_s(pst_ssid->ac_ssid, WLAN_SSID_MAX_LEN, ac_arg, pst_ssid->uc_ssid_len);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_send_neighbor_req::memcpy fail!");
        return OAL_FAIL;
    }

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_ssid_param_stru),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_neighbor_req::return err code [%d]!}", ret);
        return (uint32_t)ret;
    }
    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_11K


int32_t wal_set_assigned_filter_enable(int32_t filter_id, int32_t on)
{
    int32_t ret;
    uint32_t cmd_id;
    uint32_t params[CMD_PARAMS_MAX_CNT] = { 0 };
    uint32_t offset;
    mac_vap_stru *mac_vap = NULL;
    oal_net_device_stru *net_dev = NULL;

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_set_assigned_filter_enable:: dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

    if (on < 0) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_set_assigned_filter_enable::Invalid input, on/off %d!}", on);
        return -OAL_EINVAL;
    }

    net_dev = wal_config_get_netdev("wlan0", OAL_STRLEN("wlan0"));
    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_set_assigned_filter_enable::wlan0 not exist!}");
        return -OAL_EINVAL;
    }
    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(net_dev);

    /* vap未创建时，不处理下发的命令 */
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_set_assigned_filter_enable::vap not created yet, ignore the cmd!}");
        return -OAL_EINVAL;
    }

    if (mac_vap->st_cap_flag.bit_icmp_filter != OAL_TRUE) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_assigned_filter_enable::Func not enable!}");
        return -OAL_EINVAL;
    }

    ret = wal_get_cmd_id("icmp_filter", &cmd_id, &offset);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_set_assigned_filter_enable:find icmp_filter cmd is fail");
        return ret;
    }
    params[0] = filter_id;
    params[1] = (on > 0) ? OAL_TRUE : OAL_FALSE;
    return wal_process_cmd_params(net_dev, cmd_id, params);
}


int32_t wal_set_ip_filter_enable(int32_t on)
{
    uint16_t len;
    int32_t ret;
    uint32_t netbuf_len;
    oal_netbuf_stru *pst_netbuf = NULL;
    mac_vap_stru *mac_vap = NULL;
    oal_net_device_stru *net_dev = NULL;
    wal_msg_write_stru write_msg = {0};
    mac_ip_filter_cmd_stru st_ip_filter_cmd;
    mac_ip_filter_cmd_stru *pst_cmd_info = NULL;

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_set_ip_filter_enable:: dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

    if (on < 0) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_set_ip_filter_enable::Invalid input parameter, on/off %d!}", on);
        return -OAL_EINVAL;
    }

    net_dev = wal_config_get_netdev("wlan0", OAL_STRLEN("wlan0"));
    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_set_ip_filter_enable::wlan0 not exist!}");
        return -OAL_EINVAL;
    }
    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(net_dev);

    /* vap未创建时，不处理下发的命令 */
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_set_ip_filter_enable::vap not created yet, ignore the cmd!}");
        return -OAL_EINVAL;
    }

    if (mac_vap->st_cap_flag.bit_ip_filter != OAL_TRUE) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_ip_filter_enable::Func not enable,ignore!}");
        return -OAL_EINVAL;
    }

    /* 准备配置命令 */
    netbuf_len = sizeof(st_ip_filter_cmd);
    memset_s((uint8_t *)&st_ip_filter_cmd, netbuf_len, 0, netbuf_len);
    st_ip_filter_cmd.en_cmd = MAC_IP_FILTER_ENABLE;
    st_ip_filter_cmd.en_enable = (on > 0) ? OAL_TRUE : OAL_FALSE;

    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_ip_filter_enable::IP_filter on/off(%d).}",
                     st_ip_filter_cmd.en_enable);

    /* 申请空间 缓存过滤规则 */
    pst_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, netbuf_len, OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_set_ip_filter_enable::netbuf alloc null,size %d.}", netbuf_len);
        return -OAL_EINVAL;
    }
    memset_s(((uint8_t *)oal_netbuf_data(pst_netbuf)), netbuf_len, 0, netbuf_len);
    pst_cmd_info = (mac_ip_filter_cmd_stru *)oal_netbuf_data(pst_netbuf);

    /* 记录过滤规则 */
    ret = memcpy_s((uint8_t *)pst_cmd_info, netbuf_len, (uint8_t *)(&st_ip_filter_cmd), netbuf_len);
    oal_netbuf_put(pst_netbuf, netbuf_len);

    /* 抛事件到wal层处理 */
    len = sizeof(oal_netbuf_stru *);

    /* 填写 msg 消息头 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_IP_FILTER, len);

    /* 将申请的netbuf首地址填写到msg消息体内 */
    ret += memcpy_s(write_msg.auc_value, len, (uint8_t *)&pst_netbuf, len);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_set_ip_filter_enable::memcpy fail!");
        oal_netbuf_free(pst_netbuf);
        return -OAL_EINVAL;
    }

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + len, (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(mac_vap->uc_vap_id, 0, "{wal_set_ip_filter_enable::send_cfg_event fail:%d}", ret);
        oal_netbuf_free(pst_netbuf);
        return ret;
    }

    return OAL_SUCC;
}

int32_t wal_add_ip_filter_items(wal_hw_wifi_filter_item *pst_items, int32_t l_count)
{
    uint16_t len;
    int32_t ret;
    uint32_t netbuf_len;
    uint32_t items_idx;
    oal_netbuf_stru *pst_netbuf = NULL;
    mac_vap_stru *mac_vap = NULL;
    oal_net_device_stru *net_dev = NULL;
    wal_msg_write_stru write_msg = {0};
    mac_ip_filter_cmd_stru st_ip_filter_cmd;
    mac_ip_filter_cmd_stru *pst_cmd_info = NULL;

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_add_ip_filter_items:: dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

    if ((pst_items == NULL) || (l_count <= 0)) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_add_ip_filter_items::Invalid input para, l_count %d!}",
            l_count);
        return -OAL_EINVAL;
    }

    net_dev = wal_config_get_netdev("wlan0", OAL_STRLEN("wlan0"));
    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_add_ip_filter_items::wlan0 not exist!}");
        return -OAL_EINVAL;
    }
    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(net_dev);

    /* vap未创建时，不处理下发的命令 */
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_add_ip_filter_items::vap not created yet, ignore the cmd!.}");
        return -OAL_EINVAL;
    }

    if (mac_vap->st_cap_flag.bit_ip_filter != OAL_TRUE) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_add_ip_filter_items::Func disable, ignore cmd}");
        return -OAL_EINVAL;
    }

    /* 准备配置事件 */
    memset_s((uint8_t *)&st_ip_filter_cmd, sizeof(st_ip_filter_cmd), 0, sizeof(st_ip_filter_cmd));
    st_ip_filter_cmd.en_cmd = MAC_IP_FILTER_UPDATE_BTABLE;

    /* 由于本地名单大小限制，取能收纳的规则条目数最小值 */
    st_ip_filter_cmd.uc_item_count =
        oal_min((MAC_MAX_IP_FILTER_BTABLE_SIZE / sizeof(mac_ip_filter_item_stru)), l_count);
    if (st_ip_filter_cmd.uc_item_count < l_count) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_add_ip_filter_items::Btable(%d) is too small to store %d items!}",
            st_ip_filter_cmd.uc_item_count, l_count);
    }

    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
        "{wal_add_ip_filter_items::Start updating btable, items_cnt(%d).}",
        st_ip_filter_cmd.uc_item_count);

    /* 选择申请事件空间的大小 */
    netbuf_len = (st_ip_filter_cmd.uc_item_count * sizeof(mac_ip_filter_item_stru)) +
        sizeof(st_ip_filter_cmd);

    /* 申请空间 缓存过滤规则 */
    pst_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, netbuf_len, OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_add_ip_filter_items::netbuf alloc null,size %d.}", netbuf_len);
        return -OAL_EINVAL;
    }
    memset_s(((uint8_t *)oal_netbuf_data(pst_netbuf)), netbuf_len, 0, netbuf_len);
    pst_cmd_info = (mac_ip_filter_cmd_stru *)oal_netbuf_data(pst_netbuf);

    /* 记录过滤规则 */
    ret = memcpy_s((uint8_t *)pst_cmd_info, netbuf_len,
                   (uint8_t *)(&st_ip_filter_cmd), sizeof(st_ip_filter_cmd));
    oal_netbuf_put(pst_netbuf, netbuf_len);

    for (items_idx = 0; items_idx < st_ip_filter_cmd.uc_item_count; items_idx++) {
        pst_cmd_info->ast_filter_items[items_idx].uc_protocol = (uint8_t)pst_items[items_idx].protocol;
        pst_cmd_info->ast_filter_items[items_idx].us_port = (uint16_t)pst_items[items_idx].port;
    }

    /* 抛事件到wal层处理 */
    len = sizeof(oal_netbuf_stru *);

    /* 填写 msg 消息头 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_IP_FILTER, len);

    /* 将申请的netbuf首地址填写到msg消息体内 */
    ret += memcpy_s(write_msg.auc_value, len, (uint8_t *)&pst_netbuf, len);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hwifi_config_init_fcc_ce_txpwr_nvram::memcpy fail!");
        oal_netbuf_free(pst_netbuf);
        return -OAL_EINVAL;
    }

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + len, (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_add_ip_filter_items::wal_send_cfg_event fail[%d]!}", ret);
        oal_netbuf_free(pst_netbuf);
        return ret;
    }

    return OAL_SUCC;
}


int32_t wal_clear_ip_filter_config(oal_net_device_stru *net_dev, mac_vap_stru *mac_vap)
{
    mac_ip_filter_cmd_stru ip_filter_cmd;
    mac_ip_filter_cmd_stru *cmd_info = NULL;
    oal_netbuf_stru *netbuf = NULL;
    wal_msg_write_stru write_msg = {0};
    uint32_t netbuf_len;
    int32_t ret;
    uint16_t len;

    /* 清理黑名单 */
    memset_s((uint8_t *)&ip_filter_cmd, sizeof(ip_filter_cmd), 0, sizeof(ip_filter_cmd));
    ip_filter_cmd.en_cmd = MAC_IP_FILTER_CLEAR;

    oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_clear_ip_filter_config::Now start clearing the list.}");

    /* 选择申请事件空间的大小 */
    netbuf_len = sizeof(ip_filter_cmd);

    /* 申请空间 缓存过滤规则 */
    netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, netbuf_len, OAL_NETBUF_PRIORITY_MID);
    if (netbuf == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, 0, "{wal_clear_ip_filter_config::netbuf[%d] alloc fail}", netbuf_len);
        return -OAL_EINVAL;
    }
    memset_s(((uint8_t *)oal_netbuf_data(netbuf)), netbuf_len, 0, netbuf_len);
    cmd_info = (mac_ip_filter_cmd_stru *)oal_netbuf_data(netbuf);

    /* 记录过滤规则 */
    ret = memcpy_s((uint8_t *)cmd_info, netbuf_len, (uint8_t *)(&ip_filter_cmd), netbuf_len);
    oal_netbuf_put(netbuf, netbuf_len);

    /* 抛事件到wal层处理 */
    len = sizeof(oal_netbuf_stru *);

    /* 填写 msg 消息头 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_IP_FILTER, len);

    /* 将申请的netbuf首地址填写到msg消息体内 */
    ret += memcpy_s(write_msg.auc_value, len, (uint8_t *)&netbuf, len);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_clear_ip_filter_config::memcpy fail!");
        oal_netbuf_free(netbuf);
        return -OAL_EINVAL;
    }

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + len,
                             (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_clear_ip_filter_config::send cfg event fail:%d}", ret);
        oal_netbuf_free(netbuf);
        return ret;
    }

    return OAL_SUCC;
}


int32_t wal_clear_ip_filter()
{
    int32_t ret;
    mac_vap_stru *mac_vap = NULL;
    oal_net_device_stru *net_dev;

#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_clear_ip_filter:: dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return -OAL_EFAIL;
    }
#endif

    net_dev = wal_config_get_netdev("wlan0", OAL_STRLEN("wlan0"));
    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_clear_ip_filter::wlan0 not exist!}");
        return -OAL_EINVAL;
    }

    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(net_dev);

    /* vap未创建时，不处理下发的命令 */
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_clear_ip_filter::vap not created yet, ignore the cmd!.}");
        return -OAL_EINVAL;
    }

    if (mac_vap->st_cap_flag.bit_ip_filter != OAL_TRUE) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_clear_ip_filter::Func disable,ignore the cmd}");
        return -OAL_EINVAL;
    }

    /* 清除ip过滤的黑名单并抛事件到wal层处理 */
    ret = wal_clear_ip_filter_config(net_dev, mac_vap);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_clear_ip_filter::send cfg event fail!");
        return ret;
    }

    return OAL_SUCC;
}
int32_t wal_register_ip_filter(wal_hw_wlan_filter_ops *pst_ip_filter_ops)
{
#ifdef CONFIG_DOZE_FILTER
    if (pst_ip_filter_ops == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_register_ip_filter::pg_st_ip_filter_ops is null !}");
        return -OAL_EINVAL;
    }
    hw_register_wlan_filter(pst_ip_filter_ops);
#else
    oam_warning_log0(0, OAM_SF_ANY, "{wal_register_ip_filter:: Not support CONFIG_DOZE_FILTER!}");
#endif
    return OAL_SUCC;
}

int32_t wal_unregister_ip_filter()
{
#ifdef CONFIG_DOZE_FILTER
    hw_unregister_wlan_filter();
#else
    oam_warning_log0(0, OAM_SF_ANY, "{wal_unregister_ip_filter:: Not support CONFIG_DOZE_FILTER!}");
#endif
    return OAL_SUCC;
}


OAL_STATIC int32_t wal_ioctl_set_probe_resp_mode(oal_bool_enum_uint8 en_light_suspend)
{
    uint16_t len;
    int32_t ret;
    mac_vap_stru *mac_vap = NULL;
    oal_net_device_stru *net_dev;
    wal_msg_write_stru write_msg = {0};
    mac_probe_resp_mode_enum en_probe_resp_status;

    en_probe_resp_status = (en_light_suspend == OAL_TRUE) ? MAC_PROBE_RESP_MODE_PASSIVE : MAC_PROBE_RESP_MODE_ACTIVE;

    /* 查找p2p dev */
    net_dev = wal_config_get_netdev("p2p0", OAL_STRLEN("p2p0"));
    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_probe_resp_mode::p2p0 not exist!}");
        return -OAL_EINVAL;
    }
    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(net_dev);

    /* vap未创建时，不处理下发的命令 */
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_set_probe_resp_mode::vap not created yet, ignore the cmd!}");
        return -OAL_EINVAL;
    }

    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_set_probe_resp_mode::set probe_resp_status[%d]!}",
        en_probe_resp_status);

    /* 抛事件到wal层处理 */
    len = sizeof(en_probe_resp_status);

    /* 填写 msg 消息头 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_PROBE_PESP_MODE, len);

    /* 将申请的netbuf首地址填写到msg消息体内 */
    if (EOK != memcpy_s(write_msg.auc_value, len, (uint8_t *)&en_probe_resp_status, len)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_set_probe_resp_mode::memcpy_s failed}");
        return -OAL_EINVAL;
    }

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + len,
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_ioctl_set_probe_resp_mode::send cfg event fail[%d]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

int32_t wlan_enter_light_suspend()
{
    /* 进入熄屏，待处理事件: 1.设置P2P dev Listen状态回复probe pesp模式为PASSIVE */
    if (OAL_SUCC != wal_ioctl_set_probe_resp_mode(OAL_TRUE)) {
        oam_error_log0(0, OAM_SF_ANY, "{wlan_enter_light_suspend::light_suspend set_probe_resp_mode fail}");
    }

    oam_warning_log0(0, OAM_SF_ANY, "{wlan_enter_lights_suspend::light_suspend enter}");
    return OAL_SUCC;
}

int32_t wlan_exit_light_suspend()
{
    /* 结束熄屏，待处理事件: 1.设置P2P dev Listen状态回复probe pesp模式为ACTIVE */
    if (OAL_SUCC != wal_ioctl_set_probe_resp_mode(OAL_FALSE)) {
        oam_error_log0(0, OAM_SF_ANY, "{wlan_exit_light_suspend::light_suspend set_probe_resp_mode fail}");
    }

    oam_error_log0(0, OAM_SF_ANY, "{wlan_exit_light_suspend::light_suspend exit}");
    return OAL_SUCC;
}

#ifdef _PRE_PRODUCT_HI3751_PLATO
#define PMOC_LIGHT_SUSPEND_CB_FUNC_S pmoc_light_suspend_cb_func
#define u32ModuleID module_id
#define sModuleName s_module_name
#define EnterLightSuspendFunc enter_light_suspend_func
#define ExitLightSuspendFunc exit_light_suspend_func
#define PMOC_DRV_LightSuspendCallbackRegister pmoc_drv_light_suspend_callback_register
#define PMOC_DRV_LightSuspendCallbackUnregister pmoc_drv_light_suspend_callback_unregister
#endif

void wal_register_lights_suspend()
{
#ifdef _PRE_PRODUCT_HI3751V811
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    PMOC_LIGHT_SUSPEND_CB_FUNC_S stLightSuspendCb;

    stLightSuspendCb.u32ModuleID = (uint32_t)HI_ID_USER;
    stLightSuspendCb.sModuleName = "HI_WIFI_1103";
    stLightSuspendCb.EnterLightSuspendFunc = wlan_enter_light_suspend;
    stLightSuspendCb.ExitLightSuspendFunc  = wlan_exit_light_suspend;

    /* 大屏熄屏亮屏的钩子注册操作 */
    if (PMOC_DRV_LightSuspendCallbackRegister(&stLightSuspendCb) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_register_lights_suspend::PMOC_DRV_LightSuspendCallbackRegister failed!}");
        PMOC_DRV_LightSuspendCallbackUnregister((HI_U32)HI_ID_USER);
    }
    oam_warning_log0(0, OAM_SF_ANY, "{wal_register_lights_suspend::PMOC_DRV_LightSuspendCallbackRegister  success!}");
#endif // #if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
#else
    oam_warning_log0(0, OAM_SF_ANY, "{wal_register_lights_suspend::no register LightSuspendCallback hook}");
#endif
    return;
}
void wal_unregister_lights_suspend()
{
#ifdef _PRE_PRODUCT_HI3751V811
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    PMOC_DRV_LightSuspendCallbackUnregister((uint32_t)HI_ID_USER);
#endif // #if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
    oam_warning_log0(0, OAM_SF_ANY, "{wal_register_lights_suspend::PMOC_DRV_LightSuspendCallbackUnregister done!}");
#else
    oam_warning_log0(0, OAM_SF_ANY, "{wal_unregister_lights_suspend::no reg LightSuspendCallbackUnregister hook}");
#endif
    return;
}

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
#define WAL_TCP_ACK_BUF_TIMEOUT_MS_INVALID_START 0
#define WAL_TCP_ACK_BUF_TIMEOUT_MS_INVALID_END (10 * 1000)
OAL_STATIC uint32_t wal_hipriv_tcp_ack_buf_cfg(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t result;
    int32_t ret;
    int32_t l_tmp;
    uint8_t uc_idx;
    mac_cfg_tcp_ack_buf_stru *pst_tcp_ack_param;

    pst_tcp_ack_param = (mac_cfg_tcp_ack_buf_stru *)(write_msg.auc_value);
    memset_s((void *)pst_tcp_ack_param, sizeof(mac_cfg_tcp_ack_buf_stru),
             0, sizeof(mac_cfg_tcp_ack_buf_stru));

    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::get cmd one arg err[%d]!}", result);
        return result;
    }
    pc_param = pc_param + off_set;

    for (uc_idx = 0; uc_idx < MAC_TCP_ACK_BUF_TYPE_BUTT; uc_idx++) {
        if (0 == oal_strcmp(ac_name, g_ast_hmac_tcp_ack_buf_cfg_table[uc_idx].puc_string)) {
            break;
        }
    }
    if (uc_idx == MAC_TCP_ACK_BUF_TYPE_BUTT) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg:: parameter error !");
        return uc_idx;
    }
    pst_tcp_ack_param->en_cmd = g_ast_hmac_tcp_ack_buf_cfg_table[uc_idx].en_tcp_ack_buf_cfg_id;

    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::get cmd one arg err[%d]!}", result);
        return result;
    }

    l_tmp = (int32_t)oal_atoi(ac_name);
    if ((l_tmp < WAL_TCP_ACK_BUF_TIMEOUT_MS_INVALID_START) || (l_tmp > WAL_TCP_ACK_BUF_TIMEOUT_MS_INVALID_END)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::  car param[%d] invalid! }", oal_atoi(ac_name));
        return OAL_FAIL;
    }

    if (pst_tcp_ack_param->en_cmd == MAC_TCP_ACK_BUF_ENABLE) {
        if (((uint8_t)l_tmp == OAL_FALSE) || ((uint8_t)l_tmp == OAL_TRUE)) {
            pst_tcp_ack_param->en_enable = (uint8_t)l_tmp;
        } else {
            oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::fast_aging_flag[%d]:0/1}", (uint8_t)l_tmp);
            return OAL_FAIL;
        }
    }
    if (pst_tcp_ack_param->en_cmd == MAC_TCP_ACK_BUF_TIMEOUT) {
        if ((uint8_t)l_tmp == 0) {
            oam_warning_log0(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg:: timer_ms shoule not be 0.}");
            return OAL_FAIL;
        }

        pst_tcp_ack_param->uc_timeout_ms = (uint8_t)l_tmp;
    }
    if (pst_tcp_ack_param->en_cmd == MAC_TCP_ACK_BUF_MAX) {
        if ((uint8_t)l_tmp == 0) {
            oam_warning_log0(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg:: uc_count_limit shoule not be 0.}");
            return OAL_FAIL;
        }

        pst_tcp_ack_param->uc_count_limit = (uint8_t)l_tmp;
    }

    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_TCP_ACK_BUF, sizeof(mac_cfg_tcp_ack_buf_stru));

    oam_warning_log4(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::cmd[%d],enable[%d],timeout_ms[%d] count_limit[%d]!}",
        pst_tcp_ack_param->en_cmd, pst_tcp_ack_param->en_enable,
        pst_tcp_ack_param->uc_timeout_ms, pst_tcp_ack_param->uc_count_limit);

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tcp_ack_buf_stru),
                             (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::return err code [%d]!}", result);
        return (uint32_t)result;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_M2S_MSS

OAL_STATIC uint32_t wal_ioctl_set_m2s_blacklist(oal_net_device_stru *net_dev,
    uint8_t *puc_buf, uint32_t buf_len, uint8_t uc_m2s_blacklist_cnt)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    mac_m2s_ie_stru *pst_m2s_ie;

    /* write_msg作清零操作 */
    memset_s(&write_msg, sizeof(wal_msg_write_stru), 0, sizeof(wal_msg_write_stru));

    if (uc_m2s_blacklist_cnt > WLAN_M2S_BLACKLIST_MAX_NUM) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_ioctl_set_m2s_blacklist::cnt[%d] beyond scope.}", uc_m2s_blacklist_cnt);
        return OAL_FAIL;
    }

    if (buf_len < uc_m2s_blacklist_cnt * sizeof(wlan_m2s_mgr_vap_stru)) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_ioctl_set_m2s_blacklist::src buff len[%d] not enough.}", buf_len);
        return OAL_FAIL;
    }

    /* 抛事件到wal层处理 */
    pst_m2s_ie = (mac_m2s_ie_stru *)write_msg.auc_value;

    pst_m2s_ie->uc_blacklist_cnt = uc_m2s_blacklist_cnt;

    ret = memcpy_s(pst_m2s_ie->ast_m2s_blacklist, WLAN_M2S_BLACKLIST_MAX_NUM * sizeof(wlan_m2s_mgr_vap_stru),
                   puc_buf, uc_m2s_blacklist_cnt * sizeof(wlan_m2s_mgr_vap_stru));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_M2S, "wal_ioctl_set_m2s_blacklist::memcpy fail!");
        return OAL_FAIL;
    }

    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_M2S_BLACKLIST, sizeof(mac_m2s_ie_stru));

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_m2s_ie_stru),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_ioctl_set_m2s_blacklist::wal_send_cfg_event err[%d]}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}


uint32_t wal_ioctl_set_m2s_mss(oal_net_device_stru *net_dev, uint8_t uc_m2s_mode)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;

    /* write_msg作清零操作 */
    memset_s(&write_msg, sizeof(wal_msg_write_stru), 0, sizeof(wal_msg_write_stru));

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_M2S_MSS, sizeof(int32_t));
    *((int32_t *)(write_msg.auc_value)) = uc_m2s_mode; /* 设置配置命令参数 */

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_ioctl_set_m2s_mss::wal_send_cfg_event err:[%d]}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}

#endif

#ifdef _PRE_WLAN_FEATURE_APF

OAL_STATIC uint32_t wal_hipriv_apf_filter_list(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    mac_apf_filter_cmd_stru st_apf_filter_cmd;

    memset_s(&st_apf_filter_cmd, sizeof(mac_apf_filter_cmd_stru), 0, sizeof(mac_apf_filter_cmd_stru));
    st_apf_filter_cmd.en_cmd_type = APF_GET_FILTER_CMD;

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_APF_FILTER, sizeof(st_apf_filter_cmd));
    ret = memcpy_s(write_msg.auc_value, sizeof(st_apf_filter_cmd),
                   &st_apf_filter_cmd, sizeof(st_apf_filter_cmd));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_apf_filter_list::memcpy fail!");
        return OAL_FAIL;
    }

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_apf_filter_cmd),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_80m_rts_debug::return err code [%d]!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_DELAY_STATISTIC

OAL_STATIC uint32_t wal_hipriv_pkt_time_switch(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    uint32_t off_set = 0;
    uint32_t result;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    wal_msg_write_stru write_msg = {0};
    user_delay_switch_stru st_switch_cmd;
    int32_t ret;

    /* 获取命令字符串 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_pkt_time_switch::wal_get_cmd_one_arg err[%d]!}", result);
        return result;
    }

    memset_s(&st_switch_cmd, sizeof(user_delay_switch_stru), 0, sizeof(user_delay_switch_stru));
    if (0 == (oal_strcmp("on", ac_arg))) {
        st_switch_cmd.dmac_stat_enable = 1;

        /* 第二个参数，统计数据帧的数量 */
        pc_param += off_set;
        result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (result != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_pkt_time_switch::wal_get_cmd_one_arg 2nd err:%d}", result);
            return result;
        }
        st_switch_cmd.dmac_packet_count_num = (uint32_t)oal_atoi(ac_arg);

        /* 第三个参数，上报间隔 */
        pc_param += off_set;
        result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (result != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_pkt_time_switch::wal_get_cmd_one_arg 2nd err:%d}", result);
            return result;
        }
        st_switch_cmd.dmac_report_interval = (uint32_t)oal_atoi(ac_arg);
    } else if (0 == (oal_strcmp("off", ac_arg))) {
        st_switch_cmd.dmac_stat_enable = 0;
        st_switch_cmd.dmac_packet_count_num = 0;
        st_switch_cmd.dmac_report_interval = 0;
    } else {
        oam_error_log0(0, OAM_SF_CFG, "{wal_hipriv_pkt_time_switch::invalid parameter!}");
        return OAL_FAIL;
    }
    ret = memcpy_s(write_msg.auc_value, sizeof(st_switch_cmd), &st_switch_cmd, sizeof(st_switch_cmd));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "wal_hipriv_pkt_time_switch::memcpy fail!");
        return OAL_FAIL;
    }

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_PKT_TIME_SWITCH, sizeof(st_switch_cmd));
    result = (uint32_t)wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                                          WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_switch_cmd),
                                          (uint8_t *)&write_msg,
                                          OAL_FALSE, NULL);
    if (oal_unlikely(result != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_pkt_time_switch::return err code %d!}", result);
    }
    return result;
}
#endif


uint32_t wal_ioctl_set_ap_mode(oal_net_device_stru *net_dev, uint8_t uc_param)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SOFTAP_MIMO_MODE, sizeof(uint8_t));
    write_msg.auc_value[0] = uc_param;

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_ioctl_set_ap_mode::return err code [%d]!}", ret);
        return (uint32_t)ret;
    }

    oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_set_ap_mode::uc_param[%d].}", uc_param);
    return OAL_SUCC;
}

OAL_STATIC int32_t wal_psm_query_wait_complete(hmac_psm_flt_stat_query_stru *pst_hmac_psm_query,
    mac_psm_query_type_enum_uint8 en_query_type)
{
    pst_hmac_psm_query->auc_complete_flag[en_query_type] = OAL_FALSE;
    /*lint -e730 -e740 -e774*/
    return oal_wait_event_interruptible_timeout_m(pst_hmac_psm_query->st_wait_queue,
        (pst_hmac_psm_query->auc_complete_flag[en_query_type] == OAL_TRUE),
        OAL_TIME_HZ);
    /*lint +e730 +e740 +e774*/
}

uint32_t wal_ioctl_get_psm_stat(oal_net_device_stru *net_dev,
    mac_psm_query_type_enum_uint8 en_query_type, oal_ifreq_stru *pst_ifr)
{
    mac_vap_stru *mac_vap = NULL;
    hmac_device_stru *pst_hmac_device = NULL;
    wal_msg_write_stru write_msg = {0};
    hmac_psm_flt_stat_query_stru *pst_hmac_psm_query = NULL;
    mac_psm_query_stat_stru  *pst_psm_stat = NULL;
    int32_t ret;

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_ioctl_get_psm_stat::mac_vap get from netdev is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (en_query_type >= MAC_PSM_QUERY_TYPE_BUTT) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_ioctl_get_psm_stat::query type is not supported}");
        return OAL_FAIL;
    }

    /* 抛事件到wal层处理 */
    memset_s(&write_msg, sizeof(write_msg), 0, sizeof(write_msg));
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_QUERY_PSM_STAT, sizeof(mac_psm_query_type_enum_uint8));
    /* 下发查询命令 */
    *(mac_psm_query_type_enum_uint8*)(write_msg.auc_value) = en_query_type;
    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_psm_query_type_enum_uint8),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_ioctl_get_psm_stat::send event fail[%d]}", ret);
        return OAL_FAIL;
    }

    /* 等待查询返回 */
    pst_hmac_psm_query = &pst_hmac_device->st_psm_flt_stat_query;
    ret = wal_psm_query_wait_complete(pst_hmac_psm_query, en_query_type);
    /* 超时或异常 */
    if (ret <= 0) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_ioctl_get_psm_stat::fail! ret:%d}", ret);
        return OAL_FAIL;
    }

    pst_psm_stat = &pst_hmac_psm_query->ast_psm_stat[en_query_type];
    if (pst_psm_stat->query_item > MAC_PSM_QUERY_MSG_MAX_STAT_ITEM) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_ioctl_get_psm_stat::query_item invalid[%d]}", pst_psm_stat->query_item);
    }

    /* 调式命令时该参数为null */
    if (pst_ifr == NULL) {
        return OAL_SUCC;
    }
    if (oal_copy_to_user(pst_ifr->ifr_data + 8, pst_psm_stat->aul_val, /* 8为hdr */
        pst_psm_stat->query_item * sizeof(uint32_t))) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_ioctl_get_psm_stat::Failed to copy ioctl_data to user !}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_HID2D

uint32_t wal_ioctl_set_hid2d_acs_mode(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru              write_msg;
    uint32_t                        off_set;
    int8_t                          ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t                        result;
    int32_t                         ret;
    uint32_t                        value;

    /* 获取参数 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        return result;
    }
    pc_param += off_set;
    value = (oal_bool_enum_uint8)oal_atoi(ac_name);

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_HID2D_ACS_MODE, sizeof(uint32_t));

    /* 设置配置命令参数 */
    *((uint32_t *)(write_msg.auc_value)) = value;

    ret = wal_send_cfg_event(net_dev,
        WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint32_t),
        (uint8_t *)&write_msg,
        OAL_FALSE,
        NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{HiD2D::return err code [%d]!}\r\n", ret);
        return (uint32_t)ret;
    }
    return OAL_SUCC;
}


uint32_t wal_ioctl_update_acs_state(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru              write_msg;
    uint32_t                        off_set;
    int8_t                          name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    uint32_t                        ret;
    int32_t                         rslt;
    uint32_t                        value;

    /* 获取参数 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    param += off_set;
    value = (oal_bool_enum_uint8)oal_atoi(name);

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_HID2D_ACS_STATE, sizeof(uint32_t));

    /* 设置配置命令参数 */
    *((uint32_t *)(write_msg.auc_value)) = value;

    rslt = wal_send_cfg_event(net_dev,
        WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint32_t),
        (uint8_t *)&write_msg,
        OAL_FALSE,
        NULL);
    if (oal_unlikely(rslt != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{HiD2D::return err code [%d]!}\r\n", rslt);
        return (uint32_t)rslt;
    }
    return OAL_SUCC;
}

uint32_t wal_ioctl_hid2d_link_meas(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru              write_msg;
    mac_hid2d_link_meas_stru        hid2d_meas_cmd;
    uint32_t                        off_set;
    int8_t                          name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    uint32_t                        ret;
    int32_t                         rslt;

    /* 获取meas type */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    param += off_set;
    hid2d_meas_cmd.link_meas_cmd_type = (uint8_t)oal_atoi(name);

    /* 获取scan chan */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    param += off_set;
    hid2d_meas_cmd.scan_chan = (uint8_t)oal_atoi(name);

    /* 获取scan interval */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    param += off_set;
    hid2d_meas_cmd.scan_interval = (uint16_t)oal_atoi(name);

    /* 获取scan time */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    param += off_set;
    hid2d_meas_cmd.scan_time = (uint16_t)oal_atoi(name);

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_HID2D_LINK_MEAS, sizeof(mac_hid2d_link_meas_stru));

    /* 设置配置命令参数 */
    if (memcpy_s(write_msg.auc_value, sizeof(mac_hid2d_link_meas_stru),
        (const void *)&hid2d_meas_cmd, sizeof(mac_hid2d_link_meas_stru)) != EOK) {
        return OAL_FAIL;
    }

    rslt = wal_send_cfg_event(net_dev,
        WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_hid2d_link_meas_stru),
        (uint8_t *)&write_msg,
        OAL_FALSE,
        NULL);
    if (oal_unlikely(rslt != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{HiD2D::return err code [%d]!}\r\n", rslt);
        return (uint32_t)rslt;
    }
    return OAL_SUCC;
}

#endif
#ifdef _PRE_WLAN_FEATURE_CSI
uint32_t wal_ioctl_set_csi_switch_config(oal_net_device_stru *pst_cfg_net_dev,
    mac_cfg_csi_param_stru *cfg_csi_param_in)
{
    int32_t l_ret;
    wal_msg_write_stru st_write_msg;
    mac_cfg_csi_param_stru *cfg_csi_param = NULL;

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_CSI, sizeof(mac_cfg_csi_param_stru));

    /* 设置配置命令参数 */
    cfg_csi_param = (mac_cfg_csi_param_stru *)(st_write_msg.auc_value);
    memcpy_s(cfg_csi_param, sizeof(mac_cfg_csi_param_stru), cfg_csi_param_in, sizeof(mac_cfg_csi_param_stru));
    l_ret = wal_send_cfg_event(pst_cfg_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_csi_param_stru),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_csi::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }
    return OAL_SUCC;
}

uint32_t wal_ioctl_set_csi_switch(oal_net_device_stru *pst_cfg_net_dev, int8_t *pc_param)
{
    uint32_t off_set, ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    mac_cfg_csi_param_stru st_cfg_csi_param = {{0}};

    /* PSD设置开关的命令: hipriv "vap0 set_csi ta ta_check csi_en"
       TA:被测量CSI的mac地址，为0表示不使能
       TA_check: 为1时，TA有效，表示每次采集CSI信息时需比对ta。
       csi_en:   为1时，表示使能CSI采集
    */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    oal_strtoaddr(ac_name, sizeof(ac_name), st_cfg_csi_param.auc_mac_addr, WLAN_MAC_ADDR_LEN);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    st_cfg_csi_param.en_cfg_csi_on = (uint8_t)oal_atoi(ac_name);
    // csi也做下限制，防止上层在sniffer模式下采集不到csi误认为是摄像头的问题切换另一通道采集csi
    if (st_cfg_csi_param.en_cfg_csi_on && !wal_sniffer_is_single_sta_mode(pst_cfg_net_dev)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_csi_switch::can not start csi!}");
        return OAL_FAIL;
    }

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    st_cfg_csi_param.uc_csi_bw = (uint8_t)oal_atoi(ac_name);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    st_cfg_csi_param.uc_csi_frame_type = (uint8_t)oal_atoi(ac_name);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    st_cfg_csi_param.uc_csi_sample_period  = (uint8_t)oal_atoi(ac_name);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    st_cfg_csi_param.uc_csi_phy_report_mode = (uint8_t)oal_atoi(ac_name);
    ret = wal_ioctl_set_csi_switch_config(pst_cfg_net_dev, &st_cfg_csi_param);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}

#endif


OAL_STATIC uint32_t wal_hipriv_cali_debug(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru       write_msg;
    uint32_t                 offset = 0;
    int8_t                   ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t                 result;
    mac_cfg_cali_param_stru *cali_cfg_para = NULL;

    uint8_t                  arg_idx = 0;
    uint8_t                  src_str_len;
    uint16_t                 para_len;
    uint16_t                 curr_pos = 0;

    /* 设置配置命令参数 */
    cali_cfg_para = (mac_cfg_cali_param_stru *)(write_msg.auc_value);

    para_len = (uint16_t)OAL_STRLEN(pc_param) + 1;
    while (curr_pos < para_len) {
        /* 获取参数 */
        result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
        if (result == OAL_ERR_CODE_CONFIG_ARGS_OVER) {
            break;
        }

        if (result != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_CFG, "wal_hipriv_cali_debug::wal_get_cmd_one_arg get fail");
            return result;
        }

        src_str_len = OAL_STRLEN(ac_name) + 1;
        if (src_str_len > WLAN_CALI_CFG_CMD_MAX_LEN) {
            oam_error_log1(0, OAM_SF_CFG, "wal_hipriv_cali_debug::args len [%d] overflow", src_str_len);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }

        if (memcpy_s((int8_t*)(cali_cfg_para->input_args[arg_idx]), WLAN_CALI_CFG_CMD_MAX_LEN, ac_name, src_str_len)
            != EOK) {
            return OAL_FAIL;
        }
        pc_param += offset;
        curr_pos += offset;
        arg_idx++;
    }
    cali_cfg_para->input_args_num = arg_idx;

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_CALI_CFG, sizeof(mac_cfg_cali_param_stru));
    result = (uint32_t)wal_send_cfg_event(net_dev,
                                          WAL_MSG_TYPE_WRITE,
                                          WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_cali_param_stru),
                                          (uint8_t *)&write_msg,
                                          OAL_FALSE,
                                          NULL);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_cali_debug::return err code %d!}\r\n", result);
    }

    return result;
}

uint32_t wal_ioctl_set_mode(oal_net_device_stru *net_dev, int8_t *param)
{
    int8_t mode_str[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 }; /* 预留协议模式字符串空间 */
    uint8_t prot_idx;
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(net_dev, param))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_mode::net_dev/p_param is null!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* param指向传入模式参数, 将其取出存放到mode_str中 */
    result = wal_get_cmd_one_arg(param, mode_str, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mode::wal_get_cmd_one_arg vap name err %d!}", result);
        return result;
    }

    mode_str[sizeof(mode_str) - 1] = '\0'; /* 确保以null结尾 */

    for (prot_idx = 0; g_ast_mode_map[prot_idx].pc_name != NULL; prot_idx++) {
        ret = oal_strcmp(g_ast_mode_map[prot_idx].pc_name, mode_str);
        if (ret == 0) {
            break;
        }
    }

    if (g_ast_mode_map[prot_idx].pc_name == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_mode::unrecognized protocol string!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 发送消息到wal层处理 */
    ret = wal_hipriv_set_mode_config(net_dev, prot_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_mode::send cfg event fail!}");
        return ret;
    }
    return OAL_SUCC;
}

uint32_t wal_ioctl_set_freq(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    int32_t l_freq;
    uint32_t off_set = 0;
    int8_t ac_freq[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t result;
    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;

    /* pc_param指向新创建的net_device的name, 将其取出存放到ac_name中 */
    result = wal_get_cmd_one_arg(pc_param, ac_freq, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_freq::wal_get_cmd_one_arg vap name err %d!}", result);
        return result;
    }

    l_freq = oal_atoi(ac_freq);
    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_freq::l_freq = %d!}", l_freq);

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_CURRENT_CHANEL, sizeof(int32_t));
    *((int32_t *)(write_msg.auc_value)) = l_freq;

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_freq::err %d!}", ret);
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_freq fail, err code[%u]!}", result);
        return result;
    }

    return OAL_SUCC;
}


#ifdef _PRE_WLAN_FEATURE_MONITOR
uint32_t wal_ioctl_set_sniffer_config(oal_net_device_stru *net_dev, mac_cfg_sniffer_param_stru *cfg_sniffer_param)
{
    mac_cfg_sniffer_param_stru *pst_cfg_sniffer_param = NULL;
    wal_msg_write_stru write_msg = {0};
    uint32_t mac_address_index, result;
    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;

    oam_warning_log4(0, OAM_SF_ANY, "{wal_ioctl_set_sniffer_config::uc_sniffer_mode [%d],mac[3-4]=[%x:%x:%x].}",
                     cfg_sniffer_param->uc_sniffer_mode, cfg_sniffer_param->auc_mac_addr[MAC_ADDR_3],
                     cfg_sniffer_param->auc_mac_addr[MAC_ADDR_4], cfg_sniffer_param->auc_mac_addr[MAC_ADDR_5]);

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SNIFFER, sizeof(mac_cfg_sniffer_param_stru));

    /* 设置配置命令参数 */
    pst_cfg_sniffer_param = (mac_cfg_sniffer_param_stru *)(write_msg.auc_value);
    for (mac_address_index = 0; mac_address_index < WLAN_MAC_ADDR_LEN; mac_address_index++) {
        pst_cfg_sniffer_param->auc_mac_addr[mac_address_index] =
            cfg_sniffer_param->auc_mac_addr[mac_address_index];
    }
    pst_cfg_sniffer_param->uc_sniffer_mode = cfg_sniffer_param->uc_sniffer_mode;

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_sniffer_param_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_sniffer::err [%d]!}", ret);
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_sniffer fail, err code[%u]!}", result);
        return result;
    }

    return OAL_SUCC;
}

void wal_ioctl_sniffer_switch_handle(oal_net_device_stru *net_dev, uint8_t sniffer_mode)
{
    if (oal_value_eq_any2(sniffer_mode, WLAN_SNIFFER_TRAVEL_CAP_ON, WLAN_SINFFER_ON)) {
        wal_clear_continuous_disable_enable_wifi_cnt(); // 开启sniffer后将计数清零
    }
}
static void wal_ioctl_set_sniffer_hmac_device_flag(oal_net_device_stru *net_dev, uint8_t sniffer_mode)
{
    oal_wireless_dev_stru *wdev = NULL;
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    mac_device_stru *mac_device = NULL;
    hmac_device_stru *hmac_device = NULL;

    wdev = oal_netdevice_wdev(net_dev);
    if (wdev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_sniffer_hmac_device_flag::wdev is null!}");
        return;
    }

    wiphy_priv = (mac_wiphy_priv_stru *)oal_wiphy_priv(wdev->wiphy);
    if (wiphy_priv == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_sniffer_hmac_device_flag::wiphy_priv is null!}");
        return;
    }
    mac_device = wiphy_priv->pst_mac_device;
    if (mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_sniffer_hmac_device_flag::mac_device is null!}");
        return;
    }
    hmac_device = hmac_res_get_mac_dev(mac_device->uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_sniffer_hmac_device_flag::hmac_device is null!}");
        return;
    }
    hmac_device->sniffer_mode = sniffer_mode;
}

uint32_t wal_ioctl_set_sniffer(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    uint32_t off_set = 0;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint8_t uc_sniffer_mode;
    int32_t l_idx = 0;
    mac_cfg_sniffer_param_stru st_cfg_sniffer_param; /* 临时保存sniffer配置信息 */
    uint32_t result;

    memset_s((uint8_t *)&st_cfg_sniffer_param, sizeof(st_cfg_sniffer_param), 0, sizeof(st_cfg_sniffer_param));
    /* 获取sniffer开关标志 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_sniffer::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 输入命令合法性检测 */
    while (ac_arg[l_idx] != '\0') {
        if (isdigit(ac_arg[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_sniffer::input illegal!}");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }
    /* 将命令参数值字符串转化为整数 */
    uc_sniffer_mode = (uint8_t)oal_atoi(ac_arg);
    if (uc_sniffer_mode > WLAN_SNIFFER_STATE_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_sniffer::sniffer mode invalid.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    wal_ioctl_set_sniffer_hmac_device_flag(net_dev, uc_sniffer_mode);
    /*
     * 如果sniffer使能并且当前vap不处于单sta模式返回；伴随sniffer关闭下发，则不判断，防止sniffer使能期间vap模式发生变化
     * 而导致之前host置位无法被清理
     */
    if (uc_sniffer_mode && !wal_sniffer_is_single_sta_mode(net_dev)) {
        return OAL_FAIL;
    }

    st_cfg_sniffer_param.uc_sniffer_mode = uc_sniffer_mode;
    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;
    /* 获取mac地址 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_sniffer::get cmd one arg err[%d]!}", result);
        return result;
    }

    oal_strtoaddr(ac_name, sizeof(ac_name), st_cfg_sniffer_param.auc_mac_addr, WLAN_MAC_ADDR_LEN);

    result = wal_ioctl_set_sniffer_config(net_dev, &st_cfg_sniffer_param);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_sniffer::wal_ioctl_set_sniffer_config err[%d]!}", result);
        return result;
    }
    // 开启sniffer将异常计数清零；关闭sniffer进行wlan0 down/up
    wal_ioctl_sniffer_switch_handle(net_dev, uc_sniffer_mode);

    return OAL_SUCC;
}

uint32_t wal_ioctl_set_monitor_mode(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint8_t uc_monitor_mode;
    int32_t l_idx = 0;
    wal_msg_stru *rsp_msg = NULL;

    /* 获取monitor mode开关标志 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_enable_monitor_mode::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 输入命令合法性检测 */
    while (ac_arg[l_idx] != '\0') {
        if (isdigit(ac_arg[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_enable_monitor_mode::input illegal!}");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* 将命令参数值字符串转化为整数 */
    uc_monitor_mode = (uint8_t)oal_atoi(ac_arg);
    if (uc_monitor_mode > WLAN_MONITOR_STATE_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_enable_monitor_mode::input should be 0 or 1.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    if (uc_monitor_mode && !wal_sniffer_is_single_sta_mode(net_dev)) {
        return OAL_FAIL;
    }

    *(uint8_t *)(write_msg.auc_value) = uc_monitor_mode;

    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_enable_monitor_mode::uc_sniffer_mode [%d].}", uc_monitor_mode);

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_MONITOR_MODE, sizeof(uint8_t));

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
                             (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_enable_monitor_mode::err [%d]!}", ret);
        return (uint32_t)ret;
    }

    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_enable_monitor_mode fail, err code[%u]!}", result);
        return result;
    }

    return OAL_SUCC;
}


#endif
uint32_t wal_ioctl_set_al_rx(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint8_t uc_rx_flag;
    int32_t l_idx = 0;
    wal_msg_stru *rsp_msg = NULL;

    /* 获取常收模式开关标志 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_always_rx::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 输入命令合法性检测 */
    while (ac_arg[l_idx] != '\0') {
        if (isdigit(ac_arg[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_always_rx::input illegal!}");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* 将命令参数值字符串转化为整数 */
    uc_rx_flag = (uint8_t)oal_atoi(ac_arg);
    if (uc_rx_flag > HAL_ALWAYS_RX_RESERVED) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_always_rx::input should be 0 or 1.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (uc_rx_flag && !wal_sniffer_is_single_sta_mode(net_dev)) {
        return OAL_FAIL;
    }
    *(uint8_t *)(write_msg.auc_value) = uc_rx_flag;

    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_SET_ALWAYS_RX, sizeof(uint8_t));
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
                             (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_always_rx::err [%d]!}", ret);
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_always_rx fail, err code[%u]!}", result);
        return result;
    }

    return OAL_SUCC;
}

uint32_t wal_ioctl_set_pm_switch(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint8_t switch_pm = (uint8_t)params[0];

    oam_warning_log1(0, OAM_SF_PWR, "{wal_ioctl_set_pm_switch:[%d]}\r\n", switch_pm);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if ((switch_pm == 1) || (switch_pm == 4)) { /* 1, 4 关闭pm */
        g_wlan_host_pm_switch = OAL_TRUE;
        wlan_pm_enable();
    } else {
        wlan_pm_disable();
        g_wlan_host_pm_switch = OAL_FALSE;
    }
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    g_wlan_device_pm_switch = switch_pm;
#endif
    return OAL_SUCC;
}


uint32_t wal_split_cmd_line(char *args, uint16_t len, wlan_cfg_param_stru *wlan_cfg_param,
    char (*cfg_args)[WLAN_CFG_MAX_LEN_EACH_ARG])
{
    uint32_t offset = 0;
    char  *copy_args = NULL;
    uint32_t ret;
    uint8_t index;

    if (args == NULL || wlan_cfg_param == NULL || cfg_args == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    args[len - 1] = '\0';

    copy_args = args;

    for (index = 0; index < WLAN_CFG_MAX_ARGS_NUM; index++) {
        copy_args += offset;

        ret = alg_cfg_get_cmd_one_arg(copy_args, cfg_args[index], WLAN_CFG_MAX_LEN_EACH_ARG, &offset);
        if (ret == OAL_SUCC) {
            wlan_cfg_param->apc_args[index] = cfg_args[index];
            wlan_cfg_param->uc_args_num++;
        } else if (ret == OAL_ERR_CODE_CONFIG_ARGS_OVER) {
            break;
        } else {
            return ret;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC wal_wlan_cfg_process_info_table_stru *wal_get_wlan_cfg_info(
    wal_wlan_cfg_process_info_table_stru *process_info_tbl, uint32_t tbl_size,
    wlan_cfgid_enum_uint16 cfg_id)
{
    uint32_t index;
    wal_wlan_cfg_process_info_table_stru *current_info = NULL;

    for (index = 0; index < tbl_size; index++) {
        current_info = process_info_tbl + index;
        if (current_info->en_wlan_cfg_id == cfg_id) {
            return current_info;
        }
    }

    return NULL;
}

OAL_STATIC uint32_t wal_hipriv_alg(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;  // st_write_msg can only carry bytes less than 48
    int32_t l_ret;
    uint32_t off_set;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    int8_t *pc_tmp = (int8_t *)pc_param;
    uint16_t us_config_len;
    uint16_t us_param_len;
    uint32_t size = sizeof(mac_ioctl_alg_config_stru);
    mac_ioctl_alg_config_stru st_alg_config;

    st_alg_config.uc_argc = 0;
    while (wal_get_cmd_one_arg(pc_tmp, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set) == OAL_SUCC) {
        st_alg_config.auc_argv_offset[st_alg_config.uc_argc] = (uint8_t)((uint8_t)(pc_tmp - pc_param) +
            (uint8_t)off_set - (uint8_t)OAL_STRLEN(ac_arg));
        pc_tmp += off_set;
        st_alg_config.uc_argc++;
        if (st_alg_config.uc_argc > DMAC_ALG_CONFIG_MAX_ARG) {
            oam_error_log1(0, OAM_SF_ANY,
                "{wal_hipriv_alg::wal_hipriv_alg error, argc too big [%d]!}\r\n", st_alg_config.uc_argc);
            return OAL_FAIL;
        }
    }

    if (st_alg_config.uc_argc == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_alg::argc=0!}\r\n");
        return OAL_FAIL;
    }

    us_param_len = (uint16_t)OAL_STRLEN(pc_param);
    if (us_param_len > WAL_MSG_WRITE_MAX_LEN - 1 - size) {
        return OAL_FAIL;
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    us_config_len = size + us_param_len + 1;
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ALG, us_config_len);
    l_ret = memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value), &st_alg_config, size);
    l_ret += memcpy_s(st_write_msg.auc_value + size,
                      sizeof(st_write_msg.auc_value) - size, pc_param, us_param_len + 1);
    if (l_ret != EOK) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_alg::memcpy fail! [%d]}", l_ret);
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_config_len,
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_alg::wal_send_cfg_event return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC int32_t wal_wlan_cfg_module_process_entry(wlan_cfg_param_stru *wlan_cfg_param)
{
    wal_wlan_cfg_process_info_table_stru *wlan_cfg_process_info = NULL;
    uint16_t mem_block_len = 0;
    wal_msg_write_stru write_msg;
    uint8_t msg_type = WAL_MSG_TYPE_BUTT;
    int32_t ret;

    wlan_cfg_process_info = wal_get_wlan_cfg_info(g_wlan_cfg_process_info_table,
        oal_array_size(g_wlan_cfg_process_info_table), wlan_cfg_param->en_wlan_cfg_id);
    if (oal_any_null_ptr2(wlan_cfg_process_info, wlan_cfg_process_info->p_func)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_wlan_cfg_module_process_entry:wal_get_wlan_cfg_info NULL PARAM!.}\r\n");
        return OAL_ERR_CODE_CFG_PROCESS_ENTRY_NOT_FOUND;
    }

    /* process entry function of wlan_cfg_id
       function: 1. analysis args and output;
                 2. if needed process directly, do the process;
                 3. if needed further more process(need host/dev process),
                    fill the msg_type and mem_block needed to transfer to wal
                 4. update wlan_cfg_process_info if the cfg_id has sub_cfg_id
    */
    /* output: real_mem_block(wal_msg_write_stru.auc_value) to transfer */
    memset_s(&write_msg, sizeof(write_msg), 0, sizeof(write_msg));

    ret = wlan_cfg_process_info->p_func(wlan_cfg_param, &msg_type, write_msg.auc_value, &mem_block_len,
        &wlan_cfg_process_info->st_wlan_cfg_process_info);
    if (ret != OAL_SUCC) {
        return ret;
    }
    /* 抛事件到wal，进行host侧处理 */
    WAL_WRITE_MSG_HDR_INIT(&write_msg, wlan_cfg_param->en_wlan_cfg_id, mem_block_len);

    ret = wal_send_cfg_event(wlan_cfg_param->pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + mem_block_len, (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return (uint32_t)ret;
    }

    return ret;
}


uint32_t wal_algcmd_char_extra_adapt(oal_net_device_stru *net_dev, const char *in_param, signed long in_len,
    char *out_param, signed long out_len)
{
    uint16_t input_length; /* including '\0' */
    uint32_t ret = OAL_SUCC;
    wlan_cfg_param_stru wlan_cfg_param = { 0 };
    char (*cfg_args)[WLAN_CFG_MAX_LEN_EACH_ARG] = NULL;
    char *output = NULL;
    char args_str[IOCTL_IWPRIV_WLAN_CFG_CMD_MAX_LEN] = { 0 };
    errno_t result = EOK;

    if (oal_unlikely(in_param == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    input_length = (uint16_t)OAL_STRLEN(in_param) + 1;

    result = memcpy_s(args_str, IOCTL_IWPRIV_WLAN_CFG_CMD_MAX_LEN, in_param, input_length);
    if (result != EOK) {
        oam_error_log2(0, OAM_SF_CALIBRATE, "copy fail len[%d] ret[%]", input_length, ret);
        return OAL_FAIL;
    }

    cfg_args = (char(*)[WLAN_CFG_MAX_LEN_EACH_ARG])oal_memalloc(WLAN_CFG_MAX_ARGS_NUM * WLAN_CFG_MAX_LEN_EACH_ARG);
    if (cfg_args == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "wal_iwpriv_char_extra_adapt:alloc error: pointer null!");
        return OAL_FAIL;
    }

    output = (char *)oal_memalloc(WLAN_CFG_MAX_RSP_LEN);
    if (output == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "wal_iwpriv_char_extra_adapt:alloc error: pointer null!");
        oal_mem_free_m(cfg_args, OAL_TRUE);
        return OAL_FAIL;
    }

    memset_s(cfg_args, WLAN_CFG_MAX_ARGS_NUM * WLAN_CFG_MAX_LEN_EACH_ARG, 0,
             WLAN_CFG_MAX_ARGS_NUM * WLAN_CFG_MAX_LEN_EACH_ARG);
    memset_s(output, WLAN_CFG_MAX_RSP_LEN, 0, WLAN_CFG_MAX_RSP_LEN);

    wlan_cfg_param.pst_net_dev = net_dev;
    wlan_cfg_param.en_wlan_cfg_id = WLAN_CFGID_ALG_ENTRY;
    wlan_cfg_param.pc_output = output;
    wlan_cfg_param.us_output_len = 0;

    ret = wal_split_cmd_line((char *)args_str, input_length, &wlan_cfg_param, cfg_args);
    if (ret == OAL_SUCC) {
        /* main process: call wlan_cfg module */
        ret = (uint32_t)wal_wlan_cfg_module_process_entry(&wlan_cfg_param);
    } else {
        oam_error_log1(0, OAM_SF_CFG, "{wal_algcmd_char_extra_adapt:wal_split_cmd_line fail[%d].}\r\n", ret);
    }

    /* format output according to specific requirement */
    oal_mem_free_m(cfg_args, OAL_TRUE);
    oal_mem_free_m(output, OAL_TRUE);

    return ret;
}


uint32_t wal_hipriv_alg_cfg_entry(oal_net_device_stru *net_dev, int8_t *param)
{
    uint32_t result, off_set;
    char args_str[IOCTL_IWPRIV_WLAN_CFG_CMD_MAX_LEN] = { 0 };
    int8_t cmd_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    /* 获取配置参数名称 */
    result = wal_get_cmd_one_arg(param, cmd_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg_entry::get_cmd_one_arg failed! [%d]!}\r\n", result);
        return result;
    }

    if (oal_strcmp(cmd_name, "dbac") == 0) {
        return wal_hipriv_alg(net_dev, param);
    }

    result = sprintf_s(args_str, IOCTL_IWPRIV_WLAN_CFG_CMD_MAX_LEN, "%s", param);
    if (result < 0) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg_entry ::snprintf_s return ERR!!}");
        return OAL_FAIL;
    }

    result = wal_algcmd_char_extra_adapt(net_dev, (char *)args_str, IOCTL_IWPRIV_WLAN_CFG_CMD_MAX_LEN,
        NULL, 0);
    if (result != OAL_SUCC) {
        oal_io_print("wal_hipriv_alg_cfg_entry:: return err_code [%d]", result);
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg_entry::algcmd fail.}");
    }

    return result;
}

