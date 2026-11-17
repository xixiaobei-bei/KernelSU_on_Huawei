

#ifndef __HMAC_HID2D_H__
#define __HMAC_HID2D_H__
/* 1 其他头文件包含 */
#include "hmac_vap.h"
#include "hmac_device.h"
#include "wal_linux_ioctl.h"
#include "wal_cfg_ioctl.h"
#include "wal_linux_vendor.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_HID2D_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_HID2D

/* 2 宏定义 */
#define HMAC_HID2D_CHANNEL_NUM_2G                  32     /* 2G上全部的信道 */
#define HMAC_HID2D_CHANNEL_NUM_5G                  73     /* 5G上全部的信道 */
#define HMAC_HID2D_MAX_CHANNELS                    4      /* 最多支持80M，由4个信道组成 */
#define HMAC_HID2D_MAX_SCAN_CHAN_NUM               25     /* 最多的扫描信道数目 */
#define HMAC_HID2D_SCAN_CHAN_NUM_FOR_APK           6      /* 发布会扫描信道数 */
#define HMAC_HID2D_SCAN_TIMES_PER_CHAN_FOR_APK     10     /* 发布会每个信道扫描次数 */
#define HMAC_HID2D_SCAN_TIME_PER_CHAN_APK          150    /* 发布会扫描每个信道的测量时间为150ms */
#define HMAC_HID2D_SCAN_TIME_PER_CHAN_ACS          15     /* ACS模式下每个信道的测量时间为15ms */
#define HMAC_HID2D_SCAN_TIMER_CYCLE_MS             1000   /* 扫描间隔的基本单位 */
#define HMAC_HID2D_REPORT_TIMEOUT_MS               120 * 1000 /* dmac 120s内不上报认为出现异常，关闭acs方案 */
#define HMAC_HID2D_DECAY                           90     /* 滑动平均中的老化系数 */
#define HMAC_HID2D_CHAN_LOAD_SHIFT                 10     /* 计算信道繁忙度的扩大因子 */
#define HMAC_HID2D_CHAN_LOAD_DIFF                  50
#define HMAC_HID2D_NOISE_DIFF                      5

/* 3 枚举定义 */
/* HiD2D设备的组网模式类型 */
typedef enum {
    HMAC_HID2D_P2P_ONLY                      = 0,       /* 没有关联AP，仅P2P连接 */
    HMAC_HID2D_DBDC                          = 1,       /* AP与P2P DBDC模式 */
    HMAC_HID2D_SAME_BAND                     = 2,       /* AP与P2P同频模式 */

    HMAC_HID2D_NET_MODE_BUTT,
} hmac_hid2d_net_mode_enum;
/* 自动切换算法的状态类型 */
typedef enum {
    HMAC_HID2D_ACS_DISABLE                   = 0,       /* 方案被禁用 */
    HMAC_HID2D_ACS_WAIT                      = 1,       /* 该状态下不会主动触发扫描，不会进行信道切换 */
    HMAC_HID2D_ACS_WORK                      = 2,       /* 该状态下会主动触发扫描，会进行信道切换决策 */

    HMAC_HID2D_ACS_STATE_BUTT,
} hmac_hid2d_acs_state_enum;
/* HiD2D ACS异常退出原因 */
typedef enum {
    HMAC_HID2D_NON_GO                        = 1,       /* 非GO模式 */
    HMAC_HID2D_DMAC_REPORT_TIMEOUT           = 2,       /* DMAC长时间不上报信道信息 */
    HMAC_HID2D_P2P_LOST                      = 3,       /* P2P断链 */
    HMAC_HID2D_FWK_DISABLE                   = 4,       /* 上层调用接口关闭 */

    HMAC_HID2D_DISABLE_REASON_BUTT,
} hmac_hid2d_acs_disable_reason_enum;
/* HiD2D ACS上报内核的消息类型 */
typedef enum {
    HMAC_HID2D_INIT_REPORT                    = 0,       /* 初始化信息上报 */
    HMAC_HID2D_LINK_INFO_REPORT               = 1,       /* 当前链路信息反馈 */
    HMAC_HID2D_CHAN_STAT_REPORT               = 2,       /* 探测信道信息反馈 */
    HMAC_HID2D_SWITCH_SUCC_REPORT             = 3,       /* 切换成功反馈 */
    HMAC_HID2D_EXTI_REPORT                    = 4,       /* 退出原因上报 */

    HMAC_HID2D_REPORT_TYPE_BUTT,
} hmac_hid2d_acs_report_type_enum;
typedef enum {
    HMAC_HID2D_LINK_MEAS_START_BY_CHAN_LIST   = 0,       /* 根据默认的信道列表开始扫描 */
    HMAC_HID2D_LINK_MEAS_UPDATE_CHAN_LIST     = 1,       /* 更新扫描列表 */
    HMAC_HID2D_LINK_MEAS_UPDATE_SCAN_INTERVAL = 2,       /* 更新扫描间隔 */
    HMAC_HID2D_LINK_MEAS_UPDATE_SCAN_TIME     = 3,       /* 更新扫描时间 */
    HMAC_HID2D_LINK_MEAS_START_BY_CHAN        = 4,       /* 下发单个信道扫描的命令 */
} hmac_hid2d_link_meas_type_enum;

/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
/* 全局待选信道中每个信道的表示 */
typedef struct {
    uint8_t idx;                            /* 序号 */
    uint8_t uc_chan_idx;                    /* 主20MHz信道索引号 */
    uint8_t uc_chan_number;                 /* 主20MHz信道号 */
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽模式 */
}hmac_hid2d_chan_stru;

/* 当前信道的信息结构体 */
typedef struct {
    uint8_t is_chan_load_valid;             /* 如果alg_intf_det算法没使能，该值为OAL_FALSE */
    int8_t data_rssi;                       /* 当前的信号强度 */
    int16_t chan_load;                      /* 该信道的占空比: (total_time - rx_nondir_time) / total_time */
    uint16_t link_speed;                    /* 每1s内的平均速率 */
    uint32_t rx_succ_pkts;               /* 每1s内接收成功的帧数 */
    uint32_t rx_dropped_pkts;            /* 每1s内接收失败的帧数 */
    uint32_t tx_succ_pkts;               /* 每1s内发送的帧数 */
    uint32_t tx_fail_pkts;               /* 每1s内重传的次数 */
    uint32_t tx_dropped_pkts;            /* 每1s内丢弃的数据包 */
} hmac_hid2d_chan_stat_stru;

/* 待选信道的信息结构体 */
typedef struct {
    uint8_t good_cnt;                       /* 该信道信道质量最好的周期数 */
    int16_t update_num;                     /* 该信道信道信息更新次数，用于滑动平均 */
    int16_t chan_noise_rssi;                /* 该信道的底噪 */
    int16_t chan_load;                      /* 该信道的占空比: free_time / total_time */
    int16_t estimated_snr;                  /* 该信道预估的SNR */
    int32_t estimated_throughput;           /* 该信道预估的吞吐量 */
}hmac_hid2d_chan_info_stru;

	/* 初始化后上报内核的信息 */
typedef struct {
    hmac_hid2d_acs_report_type_enum acs_info_type;     /* 内核用来区分消息类型 */
    hmac_hid2d_acs_disable_reason_enum acs_exit_type;  /* 驱动可能判断出当前不能支持acs，如果不支持则反馈不能支持的原因 */
    uint8_t cur_band;                                  /* HiD2D建立的P2P所在频段 */
    uint8_t chan_number;                               /* 当前的信道号 */
    uint8_t bandwidth;                                 /* 当前的带宽 */
    hmac_hid2d_net_mode_enum go_mode;                  /* GO的组网模式 */
    uint8_t supp_chan_list[HMAC_HID2D_CHANNEL_NUM_5G];
    uint8_t supp_chan_num_max;
    uint8_t scan_chan_list[HMAC_HID2D_CHANNEL_NUM_5G];
    uint8_t scan_chan_num_max;
} hmac_hid2d_acs_init_report_stru;

/* 退出时上报信息 */
typedef struct {
    hmac_hid2d_acs_report_type_enum acs_info_type;     /* 内核用来区分消息类型 */
    hmac_hid2d_acs_disable_reason_enum acs_exit_type;    /* 退出的原因 */
} hmac_hid2d_acs_exit_report_stru;

/* 当前链路信息上报内核 */
typedef struct {
    hmac_hid2d_acs_report_type_enum acs_info_type;    /* 内核用来区分消息类型 */
    uint8_t chan_number;                              /* 当前的信道号 */
    uint8_t bandwidth;                                /* 当前的带宽 */
    hmac_hid2d_net_mode_enum go_mode;                 /* GO的组网模式 */
    hmac_hid2d_chan_stat_stru link_stat;              /* 上报的当前链路信息 */
} hmac_hid2d_link_stat_report_stru;

/* 扫描信息上报内核 */
typedef struct {
    hmac_hid2d_acs_report_type_enum acs_info_type;    /* 内核用来区分消息类型 */
    uint8_t scan_chan_sbs;                            /* 当前扫描信道在扫描list中的下标 */
    uint8_t scan_chan_idx;                            /* 当前扫描信道的信道号 */
    uint32_t total_stats_time_us;
    uint32_t total_free_time_20m_us;
    uint32_t total_free_time_40m_us;
    uint32_t total_free_time_80m_us;
    uint32_t total_send_time_us;
    uint32_t total_recv_time_us;
    uint8_t free_power_cnt;                           /* 信道空闲功率测量次数 */
    int16_t free_power_stats_20m;
    int16_t free_power_stats_40m;
    int16_t free_power_stats_80m;
    int16_t free_power_stats_160m;
} hmac_hid2d_chan_stat_report_stru;

/* 切换成功后信息上报内核 */
typedef struct {
    hmac_hid2d_acs_report_type_enum acs_info_type;    /* 内核用来区分消息类型 */
    uint8_t chan_number;                              /* 当前的信道号 */
    uint8_t bandwidth;                                /* 当前的带宽 */
} hmac_hid2d_switch_report_stru;

typedef struct {
    uint8_t uc_vap_id;                          /* P2P对应的mac vap id */
    uint8_t uc_apk_scan;                        /* 区别是apk触发的5G全信道扫描，还是一般业务模式下的扫描上报 */
    uint8_t uc_acs_mode;                        /* 自动信道切换算法使能 */
    hmac_hid2d_acs_state_enum acs_state;       /* 自动信道切换算法的状态 */

    wlan_channel_band_enum_uint8 uc_cur_band;     /* HiD2D建立的P2P所在频段 */
    uint8_t uc_cur_chan_idx;                    /* 当前信道在全局待选信道列表中所对应的下标 */
    uint8_t target_chan_idx;                      /* 记录切换的目的信道号 */
    uint8_t uc_cur_scan_chan_idx;               /* 当前扫描信道在扫描list中的下标 */
    uint8_t scan_chan_idx;                        /* 当前扫描信道在全局待选信道中对应的下标 */
    uint16_t scan_interval;                       /* 扫描间隔类型，根据当前信道质量变化 */
    int16_t update_num;
    int16_t max_update_num;                     /* 用于滑动平均 */
    uint16_t us_scan_times_for_apk_scan;        /* APK扫描模式的扫描次数 */
    uint32_t scan_time;                         /* 在每个信道上停留的时间 (ms) */

    frw_timeout_stru st_scan_chan_timer;          /* 信道扫描周期定时器 */
    frw_timeout_stru st_chan_stat_report_timer;   /* dmac层上报信息超时定时器 */
    hmac_hid2d_chan_stru *candidate_list;     /* 指向当前band对应的全局待选信道数组 */
    hmac_hid2d_chan_info_stru chan_info_5g[HMAC_HID2D_CHANNEL_NUM_5G]; /* 5G待选信道信息 */
} hmac_hid2d_auto_channel_switch_stru;

/* HiD2D APK ALG命令配置结构体 */
typedef struct {
    uint8_t alg_cfg_type;  /* 配置命令类型 hmac_hid2d_cmd_enum */
    int8_t *name;                           /* 配置命令对应的字符串 */
    uint8_t resv[3];                        /* 3实现字节对齐 */
} hmac_ioctl_hid2d_apk_cfg_stru;
/* HiD2D 命令配置结构体 */
typedef struct {
    uint8_t hid2d_cfg_type;  /* 配置命令类型 hmac_hid2d_cmd_enum */
    wal_hid2d_vendor_cmd_func func;           /* 命令对应处理函数 */
} hmac_hid2d_cmd_entry_stru;

extern hmac_hid2d_chan_stru g_aus_channel_candidate_list_5g[];
extern uint8_t g_scan_chan_list[];
extern uint8_t g_scan_chan_num_max;
extern hmac_hid2d_auto_channel_switch_stru g_st_hmac_hid2d_acs_info;
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
uint32_t hmac_hid2d_set_acs_mode(mac_vap_stru *mac_vap, uint8_t acs_mode);
uint32_t hmac_hid2d_update_acs_state(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_hid2d_link_meas(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_hid2d_scan_chan_start(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_device);
uint32_t hmac_hid2d_scan_complete_handler(hmac_scan_record_stru *pst_scan_record,
    uint8_t uc_scan_idx);
uint32_t hmac_hid2d_cur_chan_stat_handler(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
void hmac_hid2d_acs_switch_completed(mac_vap_stru *mac_vap);
uint32_t wal_hipriv_hid2d_scan_channel(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_hid2d_switch_channel(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
int32_t wal_hid2d_sleep_mode(oal_net_device_stru *net_dev, uint8_t ctrl,
    wal_wifi_priv_cmd_stru *priv_cmd);
int32_t wal_hid2d_napi_mode(oal_net_device_stru *pst_net_dev, uint8_t uc_ctrl);
int32_t wal_hid2d_freq_boost(oal_net_device_stru *pst_net_dev, uint8_t uc_ctrl);
#ifdef _PRE_WLAN_FEATURE_HID2D_PRESENTATION
uint32_t wal_hipriv_hid2d_get_best_chan(oal_net_device_stru *net_dev, oal_ifreq_stru *ifr);
uint32_t wal_hipriv_hid2d_presentation_init(oal_net_device_stru *net_dev, int8_t *cmd);
uint32_t wal_hipriv_hid2d_opt_cfg(oal_net_device_stru *net_dev, int8_t *cmd, uint8_t cmd_type);
uint32_t wal_hipriv_hid2d_swchan(oal_net_device_stru *net_dev, int8_t *cmd);
uint32_t wal_hipriv_hid2d_edca_cfg(oal_net_device_stru *net_dev, int8_t *cmd);
uint32_t wal_vendor_priv_cmd_hid2d_apk(oal_net_device_stru *net_dev, oal_ifreq_stru *ifr,
    uint8_t *cmd);
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
int32_t wal_ioctl_set_hid2d_state(oal_net_device_stru *pst_net_dev, uint8_t uc_param,
    wal_wifi_priv_cmd_stru *priv_cmd);
#endif
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


#endif /* end of hmac_hid2d.h */
