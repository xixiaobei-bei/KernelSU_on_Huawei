

#ifndef __MAC_STATISTIC_DATA_FLOW_H__
#define __MAC_STATISTIC_DATA_FLOW_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_main.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define WLAN_STATIS_DATA_TIMER_PERIOD    (100)          /*定时器100ms定时*/
#define WLAN_THROUGHPUT_STA_PERIOD        (20)
#define WLAN_THROUGHPUT_LOAD_LOW          (10)           /* 低负载10M */
/* WIFI测吞吐量较大时将收发中断绑定在大核 */
#define WLAN_IRQ_AFFINITY_IDLE_CPU   0
#define WLAN_IRQ_AFFINITY_BUSY_CPU   4
#define PM_QOS_THOUGHTPUT_VALUE      60

/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct
{
    oal_uint32          ul_tx_pkts;
    oal_uint32          ul_rx_pkts;
    oal_uint32          ul_tx_bytes;            /* WIFI 业务发送帧统计 */
    oal_uint32          ul_rx_bytes;            /* WIFI 业务接收帧统计 */
    oal_uint32          ul_tx_large_pkt;
    oal_uint32          ul_rx_large_pkt;
    oal_uint32          ul_tx_small_pkt;
    oal_uint32          ul_rx_small_pkt;
    oal_uint32          ul_total_sdio_pps;      /* 吞吐速率 */
    oal_uint32          ul_tx_pps;              /* 每秒pkt数 */
    oal_uint32          ul_pre_time;            /* 用于计算吞吐率 */
    oal_bool_enum_uint8 en_wifi_rx_busy;
    oal_uint8           uc_timer_cycles;        /* 定时器周期数 */
    oal_uint8           auc_resv[2];
    frw_timeout_stru    st_statis_data_timer;   /* 此成员一定要是结构体最后一个成员 */
} wifi_txrx_pkt_statis_stru;

typedef struct
{
    oal_uint32 ul_rx_throughput_mbps;
    oal_uint32 ul_tx_throughput_mbps;
    oal_uint32 ul_tx_pps;
    oal_uint32 ul_tx_large_pps;
    oal_uint32 ul_rx_large_pps;
    oal_uint32 ul_tx_small_pps;
    oal_uint32 ul_rx_small_pps;
} wifi_txrx_calculated_stat_stru;

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ

enum
{
    /* frequency lock disable mode */
    H2D_FREQ_MODE_DISABLE                 = 0,
    /* frequency lock enable mode */
    H2D_FREQ_MODE_ENABLE                  = 1,
};
typedef oal_uint8 oal_freq_mode_enum_uint8;

typedef enum
{
    CMD_SET_DEVICE_FREQ_ENDABLE, //cmd type 0
    CMD_SET_DEVICE_FREQ_VALUE,
    CMD_SET_AUTO_BYPASS_DEVICE_AUTO_FREQ,
    CMD_GET_DEVICE_AUTO_FREQ,
    CMD_AUTO_FREQ_BUTT,
} oal_h2d_freq_cmd_enum;
typedef oal_uint8 oal_h2d_freq_cmd_enum_uint8;

typedef struct
{
    oal_uint8  uc_device_type;   /*device主频类型*/
    oal_uint8  uc_reserve[3];   /*保留字段*/
} device_speed_freq_level_stru;

typedef struct
{
    oal_uint32  ul_speed_level;    /*吞吐量门限*/
    oal_uint32  ul_min_cpu_freq;  /*CPU主频下限*/
    oal_uint32  ul_min_ddr_freq;   /*DDR主频下限*/
} host_speed_freq_level_stru;

typedef struct
{
    oal_uint16           us_throughput_pps_irq_high;
    oal_uint16           us_throughput_pps_irq_low;
    oal_uint16           us_btcoex_throughput_pps_irq_high;
    oal_uint16           us_btcoex_throughput_pps_irq_low;
    oal_bool_enum_uint8  en_irq_cpu_state;            /* 绑核状态 */
    oal_bool_enum_uint8  en_irq_bindcpu;            /* 绑核总开关 */
    oal_uint8            uc_cpumask;
    oal_bool_enum_uint8  en_btcoex_flag;  /* 标示BT绑核门限参数是否生效 */
} thread_bindcpu_stru;

extern host_speed_freq_level_stru g_host_speed_freq_level[];
extern device_speed_freq_level_stru g_device_speed_freq_level[];
extern thread_bindcpu_stru g_st_thread_bindcpu;

#endif
/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_void hmac_rx_statistics_update_hmac_vap(hmac_vap_stru *pst_hmac_vap);
extern oal_void hmac_wifi_statistic_rx_tcp_ack(oal_uint32 ul_small_pkt_cnt, oal_uint32 ul_large_pkt_cnt);
extern oal_void hmac_wifi_statistic_rx_packets(oal_uint32 ul_pkt_bytes);
extern oal_void hmac_wifi_statistic_tx_packets(oal_uint32 ul_pkt_bytes);
extern oal_bool_enum_uint8 hmac_wifi_rx_is_busy(oal_void);
extern oal_void hmac_wifi_calculate_throughput(oal_void);
extern oal_void hmac_wifi_statis_data_timeout(oal_void);
extern oal_void hmac_wifi_statis_data_timer_init(oal_void);
extern oal_void hmac_wifi_statis_data_timer_deinit(oal_void);
extern oal_void hmac_wifi_pm_state_notify(oal_bool_enum_uint8 en_wake_up);
extern oal_void hmac_wifi_state_notify(oal_bool_enum_uint8 en_wifi_on);


#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif




