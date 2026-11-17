

#ifndef __WLAN_CHIP_H__
#define __WLAN_CHIP_H__

#include "wlan_types.h"
#include "wlan_cali.h"
#include "mac_vap.h"
#include "hmac_vap.h"
#include "hmac_user.h"

/* host使用的最大天线规格 */
#define HAL_HOST_MAX_RF_NUM          4
#define HAL_HOST_MAX_SLAVE_RF_NUM    2

struct wlan_flow_ctrl_params {
    uint16_t start;
    uint16_t stop;
};

typedef struct {
    uint8_t auc_sar_ctrl_params_c0;
    uint8_t auc_sar_ctrl_params_c1;
} wlan_cust_sar_ctrl_stru;

struct wlan_chip_ops {
    // 定制化相关
    /* 配置host全局变量值 */
    void (*host_global_init_param)(void);

    /* 开机首次次上电标志位刷新 */
    oal_bool_enum_uint8 (*first_power_on_mark)(void);
    /* 开机首次上电校准完成 */
    void (*first_powon_cali_completed)(void);

    /* 获取硬件是否支持160M APUT */
    oal_bool_enum_uint8 (*is_aput_support_160M)(void);

    /* 获取定制化流控参数 */
    void (*get_flow_ctrl_used_mem)(struct wlan_flow_ctrl_params *flow_ctrl);

    /* hmac命令下发到dmac过滤判断 */
    /* 返回值：TRUE:过滤该命令，不下发dmac；FALSE:命令下发dmac */
    oal_bool_enum_uint8 (*h2d_cmd_need_filter)(uint32_t cmd_id);

    /* 读取ini文件，更新定制化成员数值 */
    uint32_t (*force_update_custom_params)(void);

    /* 下发host定制化nv参数到device */
    uint32_t (*init_nvram_main)(oal_net_device_stru *cfg_net_dev);

    /* 根据定制化更新CPU调频参数 */
    void (*cpu_freq_ini_param_init)(void);

    /* 读取全部定制化配置 */
    void (*host_global_ini_param_init)(void);

    /* 读取定制化国家码自学习标志 */
    uint8_t (*get_selfstudy_country_flag)(void);

    /* host侧上电校准事件和校准参数下发接口 */
    uint32_t (*custom_cali)(void);
    /* host侧校准数据内存申请接口 */
    void (*custom_cali_data_host_addr_init)(void);

    /* 首次读取定制化配置文件总入口 */
    uint32_t (*custom_host_read_cfg_init)(void);

    /* 协议栈初始化前定制化配置入口 */
    uint32_t (*hcc_customize_h2d_data_cfg)(void);

    /* host打印定制化信息 */
    void (*show_customize_info)(void);

    /* 获取降sar参数 */
    uint32_t (*get_sar_ctrl_params)(uint8_t lvl_num, uint8_t *data_addr, uint16_t *data_len, uint16_t dest_len);

    /* 获取11ax特性配置参数 */
    uint32_t (*get_11ax_switch_mask)(void);

    /* 获取2G 11AC 特性是否使能开关 */
    /* 返回值：TRUE:2G  11AC功能使能；FALSE:2G 11AC功能关闭 */
    oal_bool_enum_uint8 (*get_11ac2g_enable)(void);

    /* 获取定制化文件probe_resp模式标识 */
    uint32_t (*get_probe_resp_mode)(void);
    /* 获取定制化文件trx switch标识 */
    uint32_t (*get_trx_switch)(void);
    /* 更新调用wal_cfg80211_mgmt_tx 监听时间 */
    uint32_t (*update_cfg80211_mgmt_tx_wait_time)(uint32_t wait_time);

    // 收发和聚合相关
    /* 判断是否需要建立BA会话 */
    /* 返回值：TRUE:允许建立BA会话；FALSE:不允许建立BA 会话 */
    oal_bool_enum_uint8 (*check_need_setup_ba_session)(void);

    /* 根据当前吞吐决定amsdu聚合个数 */
    uint8_t (*tx_update_amsdu_num)(mac_vap_stru *pst_mac_vap, hmac_performance_stat_stru *performance_stat_params,
        oal_bool_enum_uint8 en_mu_vap_flag);

    /* 初始化ba相关参数 */
    void (*ba_rx_hdl_init)(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user, uint8_t uc_tid);

    /* 检查软件是否需要处理BAR帧 */
    /* 返回值：TRUE:需要处理BAR帧；FALSE:不需要处理BAR帧 */
    oal_bool_enum_uint8 (*check_need_process_bar)(void);

    /* 上报重排序队列中超时的报文 */
    uint32_t (*ba_send_reorder_timeout)(hmac_ba_rx_stru *rx_ba, hmac_vap_stru *hmac_vap,
        hmac_ba_alarm_stru *alarm_data, uint32_t *pus_timeout);

    /* 软件接收到ADDBA REQ,是否检查BA LUT index */
    /* 返回值：TRUE:需要检查BA LUT index;FALSE:不需要检查BA LUT index */
    oal_bool_enum_uint8 (*ba_need_check_lut_idx)(void);

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    /* tcp ack缓存命令，06直接赋值，0305参数传递到device */
    /* 返回值：OAL_SUCC:传参成功;OAL_FAIL:传参失败 */
    uint32_t (*tcp_ack_buff_config)(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
#endif

#ifdef _PRE_WLAN_FEATURE_CSI
    /* set_csi设置命令，06直接host设置，0305参数传递到device */
    /* 返回值：OAL_SUCC:传参成功;OAL_FAIL:传参失败 */
    uint32_t (*csi_config)(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    uint32_t (*ftm_config)(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
    oal_bool_enum_uint8 (*rrm_proc_rm_request)(hmac_vap_stru *hmac_vap_sta, oal_netbuf_stru *netbuf);
    uint32_t (*config_wifi_rtt_config)(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
    void (*ftm_vap_init)(hmac_vap_stru *hmac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_WMMAC
    oal_bool_enum_uint8 (*wmmac_need_degrade_for_ts)(uint8_t tid, uint8_t need_degrade, uint8_t wmmac_auth_flag);
#endif
    oal_bool_enum_uint8 (*get_6g_flag)(dmac_rx_ctl_stru *rx_ctrl);
    // 校准相关
    /* 下发校准用到的矩阵信息 */
    uint32_t (*send_cali_matrix_data)(mac_vap_stru *mac_vap);

    /* 校准数据下发 */
    void (*send_cali_data)(oal_net_device_stru *cfg_net_dev);
    /* 扫描全信道20M校准数据下发 */
    void (*send_20m_all_chn_cali_data)(oal_net_device_stru *cfg_net_dev);
    /* 保存device上报的校准参数 */
    uint32_t (*save_cali_event)(frw_event_mem_stru *event_mem);
    /* 更新当前信道校准数据 */
    void (*update_cur_chn_cali_data)(mac_vap_stru *mac_vap, wlan_h2d_cali_data_type_enum_uint8 data_type);

    /* 11ax mib值初始化 */
    void (*mac_vap_init_mib_11ax)(mac_vap_stru *mac_vap, uint32_t nss_num);

    /* 更新auth超时时间 */
    void (*mac_mib_set_auth_rsp_time_out)(mac_vap_stru *mac_vap);

    /* 当前vap模式是否设置user htc cap */
    oal_bool_enum_uint8 (*mac_vap_need_set_user_htc_cap)(mac_vap_stru *mac_vap);

    /* 大包AMPDU+AMSDU入口03 05生效 */
    void (*tx_encap_large_skb_amsdu)(hmac_vap_stru *hmac_vap, hmac_user_stru *user,
        oal_netbuf_stru *buf, mac_tx_ctl_stru *tx_ctl);
    /* hcc headroom长度检查 */
    uint32_t (*check_headroom_len)(mac_tx_ctl_stru *pst_tx_ctl,
        uint8_t en_nest_type, uint8_t uc_nest_sub_type, uint8_t uc_cb_length);
    /* 调整hcc头格式 */
    void (*adjust_netbuf_data)(oal_netbuf_stru *netbuf, mac_tx_ctl_stru *tx_ctrl,
        uint8_t en_nest_type, uint8_t uc_nest_sub_type);
    /* 更新get_station查询接口数据包统计信息 */
    void (*proc_query_station_packets)(hmac_vap_stru *hmac_vap, dmac_query_station_info_response_event *reponse_event);
    /* 申请并填充扫描下发的netbuf */
    uint32_t (*scan_req_alloc_and_fill_netbuf)(frw_event_mem_stru *event_mem, hmac_vap_stru *hmac_vap,
                                               oal_netbuf_stru **netbuf_scan_req, void *params);
    uint32_t (*set_sniffer_config)(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);

#ifdef _PRE_WLAN_FEATURE_DFS
    /* zero wait dfs流程，目前，1106支持，1103/05不支持 */
    void (*start_zero_wait_dfs)(mac_vap_stru *mac_vap, mac_cfg_channel_param_stru *channel_info);
#endif
};

extern const struct wlan_chip_ops *g_wlan_chip_ops;
extern const struct wlan_chip_ops g_wlan_chip_dummy_ops;
extern const struct wlan_chip_ops g_wlan_chip_ops_1103;
extern const struct wlan_chip_ops g_wlan_chip_ops_1105;
extern const struct wlan_chip_ops g_wlan_chip_ops_1106;
extern const struct wlan_chip_ops g_wlan_chip_ops_bisheng;

void wlan_chip_ops_register(struct wlan_chip_ops *ops);

#endif /* end of wlan_chip.h */


