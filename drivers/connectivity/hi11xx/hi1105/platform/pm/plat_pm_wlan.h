

#ifndef __PLAT_PM_WLAN_H__
#define __PLAT_PM_WLAN_H__

/* 其他头文件包含 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/mutex.h>
#include <linux/kernel.h>
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
#include <linux/pm_wakeup.h>
#endif
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio.h>

#include <linux/fb.h>
#endif
#include "oal_hcc_bus.h"

#include "oal_ext_if.h"

#ifdef WIN32
#include "plat_type.h"
#endif

#define WLAN_WAKUP_MSG_WAIT_TIMEOUT    1000
#define WLAN_SLEEP_MSG_WAIT_TIMEOUT    10000
#define WLAN_POWEROFF_ACK_WAIT_TIMEOUT 1000
#define WLAN_OPEN_BCPU_WAIT_TIMEOUT    1000
#define WLAN_HALT_BCPU_TIMEOUT         1000
#define WLAN_SLEEP_TIMER_PERIOD        20  /* 睡眠定时器20ms定时 */
#define WLAN_SLEEP_DEFAULT_CHECK_CNT   5   /* 默认100ms */
#define WLAN_SLEEP_LONG_CHECK_CNT      20  /* 入网阶段,延长至400ms */
#define WLAN_SLEEP_FAST_CHECK_CNT      1   /* fast sleep,20ms */
#define WLAN_WAKELOCK_HOLD_TIME        500 /* hold wakelock 500ms */
#define WLAN_WAKELOCK_NO_TIMEOUT       (-1) /* hold wakelock no timeout */
#define WLAN_BUS_SEMA_TIME             (6 * HZ)   /* 6s 等待信号量 */

// 100us for WL_DEV_WAKEUP hold lowlevel
#define WLAN_WAKEUP_DEV_EVENT_DELAY_US (0) // 150 us, reserve to 0us

#define WLAN_SDIO_MSG_RETRY_NUM      3
#define WLAN_WAKEUP_FAIL_MAX_TIMES   0               /* 连续多少次wakeup失败，可进入DFR流程 */
#define WLAN_PACKET_CHECK_TIME       5000            /* 唤醒后，每5s打印一次报文个数用于持锁是否异常的debug */
#define WLAN_SLEEP_FORBID_CHECK_TIME (2 * 60 * 1000) /* 连续2分钟sleep forbid */

#define WLAN_PM_MODULE "[wlan]"

enum WLAN_PM_CPU_FREQ_ENUM {
    WLCPU_40MHZ = 1,
    WLCPU_80MHZ = 2,
    WLCPU_160MHZ = 3,
    WLCPU_240MHZ = 4,
    WLCPU_320MHZ = 5,
    WLCPU_480MHZ = 6,
};

enum WLAN_PM_SLEEP_STAGE {
    SLEEP_STAGE_INIT = 0,    // 初始
    SLEEP_REQ_SND = 1,       // sleep request发送完成
    SLEEP_ALLOW_RCV = 2,     // 收到allow sleep response
    SLEEP_DISALLOW_RCV = 3,  // 收到allow sleep response
    SLEEP_CMD_SND = 4,       // 允许睡眠reg设置完成
};

#define ALLOW_IDLESLEEP    1
#define DISALLOW_IDLESLEEP 0

#define WIFI_PM_POWERUP_EVENT   3
#define WIFI_PM_POWERDOWN_EVENT 2
#define WIFI_PM_SLEEP_EVENT     1
#define WIFI_PM_WAKEUP_EVENT    0

/* STRUCT 定义 */
typedef oal_bool_enum_uint8 (*wifi_srv_get_pm_pause_func)(void);
typedef void (*wifi_srv_open_notify)(oal_bool_enum_uint8);
typedef void (*wifi_srv_pm_state_notify)(oal_bool_enum_uint8);

typedef void (*wifi_srv_data_wkup_print_en_func)(oal_bool_enum_uint8);
struct wifi_srv_callback_handler {
    wifi_srv_get_pm_pause_func p_wifi_srv_get_pm_pause_func;
    wifi_srv_open_notify p_wifi_srv_open_notify;
    wifi_srv_pm_state_notify p_wifi_srv_pm_state_notify;
};

struct wlan_pm_s {
    hcc_bus *pst_bus;  // 保存oal_bus 的指针

    oal_uint wlan_pm_enable;       // pm使能开关
    oal_uint wlan_power_state;     // wlan power on state
    oal_uint apmode_allow_pm_flag; /* ap模式下，是否允许下电操作,1:允许,0:不允许 */

    volatile oal_uint wlan_dev_state;  // wlan sleep state
    uint8_t wakeup_err_count;         // 连续唤醒失败次数
    uint8_t fail_sleep_count;         // 连续睡眠失败次数

    oal_workqueue_stru *pst_pm_wq;       // pm work quque
    oal_work_stru st_wakeup_work;        // 唤醒work
    oal_work_stru st_sleep_work;         // sleep work
    oal_work_stru st_ram_reg_test_work;  // ram_reg_test work

    oal_timer_list_stru st_watchdog_timer;  // sleep watch dog
    oal_wakelock_stru st_pm_wakelock;
    oal_wakelock_stru st_delaysleep_wakelock;

    uint32_t packet_cnt;        // 睡眠周期内统计的packet个数
    uint32_t packet_total_cnt;  // 从上次唤醒至今的packet个数，定期输出for debug
    unsigned long packet_check_time;
    unsigned long sleep_forbid_check_time;
    uint32_t wdg_timeout_cnt;       // timeout check cnt
    uint32_t wdg_timeout_curr_cnt;  // timeout check current cnt
    volatile oal_uint sleep_stage;    // 睡眠过程阶段标识

    oal_completion st_open_bcpu_done;
    oal_completion st_close_bcpu_done;
    oal_completion st_close_done;
    oal_completion st_wakeup_done;
    oal_completion st_sleep_request_ack;
    oal_completion st_halt_bcpu_done;
    oal_completion st_wifi_powerup_done;

    uint32_t wkup_src_print_en;

    struct wifi_srv_callback_handler st_wifi_srv_handler;

    /* 维测统计 */
    uint32_t open_cnt;
    uint32_t open_bcpu_done_callback;
    uint32_t close_bcpu_done_callback;
    uint32_t close_cnt;
    uint32_t close_done_callback;
    uint32_t wakeup_succ;
    uint32_t wakeup_succ_work_submit;
    uint32_t wakeup_dev_ack;
    uint32_t wakeup_done_callback;
    uint32_t wakeup_fail_wait_sdio;
    uint32_t wakeup_fail_timeout;
    uint32_t wakeup_fail_set_reg;
    uint32_t wakeup_fail_submit_work;

    uint32_t sleep_succ;
    uint32_t sleep_feed_wdg_cnt;
    uint32_t sleep_fail_request;
    uint32_t sleep_fail_wait_timeout;
    uint32_t sleep_fail_set_reg;
    uint32_t sleep_request_host_forbid;
    uint32_t sleep_fail_forbid;
    uint32_t sleep_fail_forbid_cnt; /* forbid 计数，当睡眠成功后清除，维测信息 */
    uint32_t sleep_work_submit;

    uint32_t tid_not_empty_cnt;
    uint32_t is_ddr_rx_cnt;
    uint32_t ring_has_mpdu_cnt;
};
typedef struct wlan_memdump_s {
    int32_t addr;
    int32_t len;
    int32_t en;
} wlan_memdump_t;

/* EXTERN VARIABLE */
extern oal_bool_enum g_wlan_host_pm_switch;
extern uint8_t g_wlan_device_pm_switch;
extern uint8_t g_wlan_min_fast_ps_idle;
extern uint8_t g_wlan_max_fast_ps_idle;
extern uint8_t g_wlan_auto_ps_screen_on;
extern uint8_t g_wlan_auto_ps_screen_off;
extern uint8_t g_wlan_ps_mode;
extern uint16_t g_download_rate_limit_pps;
extern uint8_t g_wlan_fast_ps_dyn_ctl;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
extern uint8_t g_custom_cali_done;
#endif

/* EXTERN FUNCTION */
extern struct wlan_pm_s *wlan_pm_get_drv(void);
extern void wlan_pm_debug_sleep(void);
extern void wlan_pm_debug_wakeup(void);
extern void wlan_pm_dump_host_info(void);
extern void wlan_pm_dump_device_info(void);
extern struct wlan_pm_s *wlan_pm_init(void);
extern oal_uint wlan_pm_exit(void);
extern uint32_t wlan_pm_is_poweron(void);
extern int32_t wlan_pm_open(void);
extern uint32_t wlan_pm_close(void);
extern uint32_t wlan_pm_close_by_shutdown(void);
extern oal_uint wlan_pm_init_dev(void);
extern oal_uint wlan_pm_wakeup_dev(void);
void wlan_pm_wakeup_dev_lock(void);
extern oal_uint wlan_pm_wakeup_host(void);
extern oal_uint wlan_pm_open_bcpu(void);
extern oal_uint wlan_pm_state_get(void);
extern uint32_t wlan_pm_enable(void);
extern uint32_t wlan_pm_disable(void);
extern uint32_t wlan_pm_statesave(void);
extern uint32_t wlan_pm_staterestore(void);
extern uint32_t wlan_pm_disable_check_wakeup(int32_t flag);
struct wifi_srv_callback_handler *wlan_pm_get_wifi_srv_handler(void);
extern void wlan_pm_wakeup_dev_ack(void);
extern void wlan_pm_set_timeout(uint32_t timeout);
extern int32_t wlan_pm_poweroff_cmd(void);
extern int32_t wlan_pm_shutdown_bcpu_cmd(void);
extern void wlan_pm_feed_wdg(void);
extern int32_t wlan_pm_stop_wdg(struct wlan_pm_s *pst_wlan_pm_info);
extern void wlan_pm_info_clean(void);
extern void wlan_pm_wkup_src_debug_set(uint32_t en);
extern uint32_t wlan_pm_wkup_src_debug_get(void);
extern wlan_memdump_t *get_wlan_memdump_cfg(void);
extern int32_t g_wlan_mem_check_mdelay;
extern int32_t g_bfgx_mem_check_mdelay;
extern wlan_memdump_t *get_wlan_memdump_cfg(void);

extern int32_t wlan_device_mem_check(int32_t l_runing_test_mode);
extern int32_t wlan_device_mem_check_result(unsigned long long *time);
extern void wlan_device_mem_check_work(oal_work_stru *pst_worker);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
int32_t wlan_pm_register_notifier(struct notifier_block *nb);
void wlan_pm_unregister_notifier(struct notifier_block *nb);
#endif

#ifdef CONFIG_HUAWEI_DSM
extern void hw_110x_register_dsm_client(void);
extern void hw_110x_unregister_dsm_client(void);
#endif

void wlan_pm_idle_sleep_vote(uint8_t uc_allow);

#endif
