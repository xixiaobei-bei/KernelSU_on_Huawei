

#ifndef __MAC_AUTO_ADJUST_FREQ_H__
#define __MAC_AUTO_ADJUST_FREQ_H__

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#include <linux/pm_qos.h>
#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
#include <linux/cpufreq.h>
#ifdef CONFIG_ARCH_PLATFORM
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#include <linux/platform_drivers/lpcpu_cpufreq_req.h>
#include <linux/platform_drivers/hisi_core_ctl.h>
#else
#include <linux/hisi/lpcpu_cpufreq_req.h>
#include <linux/core_ctl.h>
#endif
#else
#include <linux/hisi/hisi_cpufreq_req.h>
#include <linux/hisi/hisi_core_ctl.h>
#endif /* end for CONFIG_ARCH_PLATFORM */
#endif
#endif

/*
 * set cpu freq A53-0
 * this function used on K3V3+ platform
 */
#define CPU_MAX_FREQ "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"
#define CPU_MIN_FREQ "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq"

/*
 * set ddr freq
 * this function used on K3V3+ platform
 */
#define DDR_MAX_FREQ "/sys/class/devfreq/ddrfreq/max_freq"
#define DDR_MIN_FREQ "/sys/class/devfreq/ddrfreq/min_freq"

#define MAX_DEGRADE_FREQ_COUNT_THRESHOLD_SUCCESSIVE_3  (3)   /* 连续3个周期都需要降频才降频 */
#define MAX_DEGRADE_FREQ_COUNT_THRESHOLD_SUCCESSIVE_10 (100) /* 有包时连续100个周期都需要降频才降频 */
/* WIFI测吞吐量较大时将收发中断绑定在大核 */
#define WLAN_IRQ_AFFINITY_IDLE_CPU 0
#define WLAN_IRQ_AFFINITY_BUSY_CPU 4

#define WLAN_CPU_FREQ_IDLE  0
#define WLAN_CPU_FREQ_SUPER 1

#define WLAN_IRQ_THROUGHPUT_THRESHOLD_HIGH 200 /* mbps */
#define WLAN_IRQ_THROUGHPUT_THRESHOLD_LOW  150 /* mbps */

#define WLAN_IRQ_PPS_THRESHOLD_HIGH 25000 /* pps */
#define WLAN_IRQ_PPS_THRESHOLD_LOW  5000  /* pps */

#define WLAN_CPU_FREQ_THROUGHPUT_THRESHOLD_HIGH 800 /* mbps */
#define WLAN_CPU_FREQ_THROUGHPUT_THRESHOLD_LOW  500 /* mbps */

#define WLAN_FREQ_TIMER_PERIOD     100 /* 定时器100ms定时 */
#define WLAN_THROUGHPUT_STA_PERIOD 20

#define WLAN_THROUGHPUT_LOAD_LOW   10 /* 低负载10M */
#define WLAN_THROUGHPUT_LOAD_HIGH 200
#define PM_QOS_THOUGHTPUT_VALUE 60
#define HMAC_FREQ_S_TRANS_TO_MS 1000

#define irq_cmd_is_onecpu(_cmd) ((_cmd) & ((_cmd) - 1)) /* 判断_cmd中是否只有一位bit为1 */

enum {
    /* indicate to set cpu/ddr max frequencies */
    SCALING_MAX_FREQ = 0,
    /* indicate to set cpu/ddr min frequencies */
    SCALING_MIN_FREQ = 1,
};

typedef enum {
    /* frequency lock disable mode */
    FREQ_LOCK_DISABLE = 0,
    /* frequency lock enable mode */
    FREQ_LOCK_ENABLE = 1,
} oal_freq_lock_enum;

typedef enum {
    CMD_SET_AUTO_FREQ_ENDABLE,
    CMD_SET_DEVICE_FREQ_VALUE,
    CMD_SET_CPU_FREQ_VALUE,
    CMD_SET_DDR_FREQ_VALUE,
    CMD_SET_AUTO_BYPASS_DEVICE_AUTO_FREQ,
    CMD_GET_DEVICE_AUTO_FREQ,
    CMD_SET_DEVICE_FREQ_TC,
    CMD_AUTO_FREQ_BUTT,
} oal_auto_freq_cmd_enum;

typedef enum {
    HMAC_FREQ_LEVEL_0,
    HMAC_FREQ_LEVEL_1,
    HMAC_FREQ_LEVEL_2,
    HMAC_FREQ_LEVEL_3,

    HMAC_FREQ_LEVEL_BUTT
} hmac_freq_level_enum;

typedef enum {
    HOST_SPEED_FREQ_LEVEL_0,
    HOST_SPEED_FREQ_LEVEL_1,
    HOST_SPEED_FREQ_LEVEL_2,
    HOST_SPEED_FREQ_LEVEL_3,

    HOST_SPEED_FREQ_LEVEL_BUTT
} host_speed_freq_level_enum;

typedef struct {
    uint8_t uc_trx_stat_log_en;
    uint8_t auc_resv[3]; /* 3代表4字节对齐，保留字段 */
    oal_atomic tx_pkts; /* WIFI 业务发送帧统计 */
    oal_atomic rx_pkts; /* WIFI 业务接收帧统计 */
    oal_atomic tx_bytes; /* WIFI 业务发送帧统计 */
    oal_atomic rx_bytes; /* WIFI 业务接收帧统计 */
} wifi_txrx_pkt_stat;
extern wifi_txrx_pkt_stat g_st_wifi_rxtx_total;

typedef struct {
    uint8_t uc_device_type; /* device主频类型 */
    uint8_t uc_reserve[3];  /* 3代表4字节对齐，保留字段 */
} device_speed_freq_level_stru;

typedef struct {
    uint32_t speed_level;  /* 吞吐量门限 */
    uint32_t min_cpu_freq; /* CPU主频下限 */
    uint32_t min_ddr_freq; /* DDR主频下限 */
} host_speed_freq_level_stru;

extern host_speed_freq_level_stru g_host_speed_freq_level[HOST_SPEED_FREQ_LEVEL_BUTT];
extern device_speed_freq_level_stru g_device_speed_freq_level[DEV_WORK_FREQ_LVL_NUM]; /* device主频类型数 */

#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
extern struct cpufreq_req g_ast_cpufreq[OAL_BUS_MAXCPU_NUM];
extern hisi_max_cpu_freq g_aul_cpumaxfreq[OAL_BUS_MAXCPU_NUM];
extern struct pm_qos_request g_st_pmqos_requset;
#endif

typedef struct {
    oal_bool_enum_uint8 en_is_inited;
    uint8_t uc_lock_mod;        /* 使能开关 */
    uint8_t uc_curr_lock_level; /* 当前主频等级 */
    uint8_t uc_req_lock_level;  /* 目标主频等级 */
    uint32_t pre_jiffies;
    uint32_t adjust_count;
    oal_mutex_stru st_lock_freq_mtx;
    oal_bool_enum_uint8 en_irq_affinity; /* 中断开关 */
    uint8_t uc_timer_cycles;           /* 定时器周期数 */
    uint8_t uc_cur_irq_cpu;            /* 当前中断所在CPU */
    uint8_t uc_req_irq_cpu;            /* 期望中断所在CPU */
    uint32_t pre_time;              /* 上一次统计时间 */
    uint32_t total_sdio_pps;        /* 吞吐速率 */
    uint32_t tx_pps;                /* 发送方向pps */
    uint32_t rx_pps;                /* 接收方向pps */
    uint16_t us_throughput_irq_high;
    uint16_t us_throughput_irq_low;
    uint32_t irq_pps_high;
    uint32_t irq_pps_low;
    oal_bool_enum_uint8 en_userctl_bindcpu;
    uint8_t uc_userctl_irqbind;
    uint8_t uc_userctl_threadbind;
    uint8_t uc_trx_stat_log_en;
#ifdef RND_HUAWEI_LOW_LATENCY_SWITCHING
    oal_bool_enum_uint8 en_low_latency_bindcpu;
    uint8_t uc_low_latency_irqbind;
    uint8_t uc_low_latency_threadbind;
    uint8_t resv;
#endif
    /* CPU锁频 */
    oal_bool_enum_uint8 uc_lock_max_cpu_freq;
    oal_bool_enum_uint8 uc_lock_dma_latency;
    uint8_t uc_req_super_cpu_freq;            /* 期望CPU超频 */
    uint8_t uc_cur_cpu_freq_state;
    uint16_t us_lock_cpu_th_high;
    uint16_t us_lock_cpu_th_low;
    uint16_t dma_latency_value;

#ifdef CONFIG_HI110X_SOFT_AP_LIMIT_CPU_FREQ
    oal_bool_enum_uint8 limit_cpu_freq;
#endif

    frw_timeout_stru hmac_freq_timer; /* auto freq timer */
} freq_lock_control_stru;
extern freq_lock_control_stru g_freq_lock_control;

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
extern oal_bool_enum_uint8 g_en_pk_mode_swtich;
extern uint32_t g_st_pk_mode_high_th_table[WLAN_PROTOCOL_CAP_BUTT][WLAN_BW_CAP_BUTT];
extern uint32_t g_st_pk_mode_low_th_table[WLAN_PROTOCOL_CAP_BUTT][WLAN_BW_CAP_BUTT];
#endif

/* 当前wifi业务负载 */
typedef struct {
    oal_bool_enum_uint8 en_wifi_rx_busy;
    oal_bool_enum_uint8 en_wifi_tx_busy;
    uint8_t uc_res[2]; // 2代表4字节对齐，保留字段
} freq_wifi_load_stru;
extern freq_wifi_load_stru g_st_wifi_load;


oal_bool_enum_uint8 hmac_set_auto_freq_mod(oal_freq_lock_enum uc_freq_enable);
void hmac_auto_freq_wifi_rx_stat(uint32_t pkt_count);
void hmac_auto_freq_wifi_tx_stat(uint32_t pkt_count);
void hmac_adjust_freq(void);
void hmac_adjust_set_freq(void);
void hmac_wifi_auto_freq_ctrl_init(void);
void hmac_wifi_auto_freq_ctrl_deinit(void);

void hmac_freq_timer_init(void);
void hmac_freq_timer_deinit(void);
void hmac_auto_freq_wifi_rx_bytes_stat(uint32_t pkt_bytes);
void hmac_auto_freq_wifi_tx_bytes_stat(uint32_t pkt_bytes);
void hmac_wifi_pm_state_notify(oal_bool_enum_uint8 en_wake_up);
void hmac_wifi_state_notify(oal_bool_enum_uint8 en_wifi_on);
void hmac_userctl_bindcpu_get_cmd(uint32_t cmd);
oal_bool_enum_uint8 hmac_wifi_rx_is_busy(void);
void hmac_set_cpu_freq(uint8_t uc_req_freq_state);
#ifdef CONFIG_HI110X_SOFT_AP_LIMIT_CPU_FREQ
void hmac_cpu_freq_upper_limit_switch(oal_bool_enum_uint8 limit);
#endif
#ifdef RND_HUAWEI_LOW_LATENCY_SWITCHING
void hmac_low_latency_bindcpu_fast(void);
void hmac_low_latency_bindcpu_default(void);
#endif
#endif

