

#ifndef __PLAT_PM_H__
#define __PLAT_PM_H__

/* 其他头文件包含 */
#include <linux/version.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/kernel.h>
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
#include <linux/pm_wakeup.h>
#endif
#ifdef _PRE_CONFIG_HISI_110X_BLUEZ
#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>
#endif
#include "plat_pm_wlan.h"
#include "hw_bfg_ps.h"
#include "board.h"
#ifdef CONFIG_HUAWEI_DSM
#if defined(_PRE_PRODUCT_HI1620S_KUNPENG) || defined(_PRE_WINDOWS_SUPPORT)
#include <linux/huawei/dsm/dsm_pub.h>
#else
#include <dsm/dsm_pub.h>
#endif
#endif
#include "hi_drv_gpio.h"

/* 宏定义 */
#define BFG_LOCK_NAME               "bfg_wake_lock"
#define BT_LOCK_NAME                "bt_wake_lock"
#define GNSS_LOCK_NAME              "gnss_wake_lock"

#define DEFAULT_WAKELOCK_TIMEOUT 2000 /* msec */

#define FIRMWARE_CFG_INIT_OK 0x01

#define SUCCESS 0
#define FAILURE 1

#define GNSS_AGREE_SLEEP     1
#define GNSS_NOT_AGREE_SLEEP 0

#define BFGX_SLEEP  0
#define BFGX_ACTIVE 1

#define HOST_DISALLOW_TO_SLEEP 0
#define HOST_ALLOW_TO_SLEEP    1

#define BFGX_PM_ENABLE  1
#define BFGX_PM_DISABLE 0

#define WAIT_DEVACK_MSEC              10
#define WAIT_DEVACK_CNT               10
#define WAIT_DEVACK_TIMEOUT_MSEC      200
#define WAIT_WKUP_DEVACK_TIMEOUT_MSEC 1000

#define SLEEP_10_MSEC              10
#define SLEEP_100_MSEC             100


#define DMD_EVENT_BUFF_SIZE 1024

/* 1103 bootloader DTCM 地址区间是0x20019000~0x2001a620，共0xa20(2592)字节 */
#define HI1103_BOOTLOAD_DTCM_BASE_ADDR 0x20019000
#define HI1103_BOOTLOAD_DTCM_SIZE      2592

/* iomcu power state:0,1->ST_POWERON,8->ST_SLEEP,9->ST_WAKEUP */
#define ST_POWERON       0
#define ST_POWERON_OTHER 1
#define ST_SLEEP         8
#define ST_WAKEUP        9

/* 超时时间要大于wkup dev work中的最长执行时间，否则超时以后进入DFR和work中会同时操作tty，导致冲突 */
#define WAIT_WKUPDEV_MSEC 10000

#define RAM_TEST_RUN_VOLTAGE_BIAS_HIGH 0x0 /* 拉偏高压 */
#define RAM_TEST_RUN_VOLTAGE_BIAS_LOW  0x1 /* 拉偏低压 */

#define RAM_TEST_RUN_VOLTAGE_REG_ADDR     0x50002010
#define RAM_TEST_RUN_PROCESS_SEL_REG_ADDR 0x50002014

#define RAM_TEST_RUN_VOLTAGE_REG_ADDR06     0x40002010
#define RAM_TEST_RUN_PROCESS_SEL_REG_ADDR06 0x40002014

/* device soc test专用宏 */
#define SOC_TEST_RUN_FINISH     0
#define SOC_TEST_RUN_BEGIN      1

#define SOC_TEST_STATUS_REG     0x50000014
#define SOC_TEST_MODE_REG       0x50000018
#define SOC_TEST_RST_FLAG       "0x5000001c,4"

/* device soc test mode */
#define TIMER_TEST_FLAG         BIT0
#define REG_MEM_CHECK_TEST_FLAG BIT1
#define INTERRUPT_TEST_FLAG     BIT2
#define MEM_MONITOR_TEST_FLAG   BIT3
#define DMA_TEST_FLAG           BIT4
#define CPU_TRACE_TEST_FLAG     BIT5
#define PATCH_TEST_FLAG         BIT6
#define SSI_MASTER_TEST_FLAG    BIT7
#define EFUSE_TEST_FLAG         BIT8
#define WDT_TEST_FLAG           BIT9
#define MPU_TEST_FLAG           BIT10
#define IPC_TEST_FLAG           BIT11
#define MBIST_TEST_FLAG         BIT12
#define SCHEDULE_FPU_TEST_FLAG  BIT13
#define I3C_SLAVE_TEST_FLAG     BIT14

#define BT_SLEEP_TIME       1500
#define PLATFORM_SLEEP_TIME 50

/* 检查是否bfg timer工作正常,5s内无有mod timer, 异常处理,防止始终不睡眠 */
#define PL_CHECK_TIMER_WORK 5000

/* 检查是否gnss异常不投票睡眠的次数:5s */
#define PL_CHECK_GNSS_VOTE_CNT (5000 / PLATFORM_SLEEP_TIME)

enum UART_STATE_ENUM {
    UART_NOT_READY = 0,
    UART_READY = 1,
};

/* BFGX系统上电加载异常类型 */
enum BFGX_POWER_ON_EXCEPTION_ENUM {
    BFGX_POWER_FAILED = -1,
    BFGX_POWER_SUCCESS = 0,

    BFGX_POWER_PULL_POWER_GPIO_FAIL = 1,
    BFGX_POWER_TTY_OPEN_FAIL = 2,
    BFGX_POWER_TTY_FLOW_ENABLE_FAIL = 3,

    BFGX_POWER_WIFI_DERESET_BCPU_FAIL = 4,
    BFGX_POWER_WIFI_ON_BOOT_UP_FAIL = 5,

    BFGX_POWER_WIFI_OFF_BOOT_UP_FAIL = 6,
    BFGX_POWER_DOWNLOAD_FIRMWARE_FAIL = 7,

    BFGX_POWER_WAKEUP_FAIL = 8,
    BFGX_POWER_OPEN_CMD_FAIL = 9,

    BFGX_POWER_DOWNLOAD_FIRMWARE_INTERRUPT = 10,

    BFGX_POWER_ENUM_BUTT,
};

/* wifi系统上电加载异常类型 */
enum WIFI_POWER_ON_EXCEPTION_ENUM {
    WIFI_POWER_FAIL = -1,
    WIFI_POWER_SUCCESS = 0,
    WIFI_POWER_PULL_POWER_GPIO_FAIL = 1,

    WIFI_POWER_BFGX_OFF_BOOT_UP_FAIL = 2,
    WIFI_POWER_BFGX_OFF_FIRMWARE_DOWNLOAD_FAIL = 3,

    WIFI_POWER_BFGX_ON_BOOT_UP_FAIL = 4,
    WIFI_POWER_BFGX_DERESET_WCPU_FAIL = 5,
    WIFI_POWER_BFGX_ON_FIRMWARE_DOWNLOAD_FAIL = 6,
    WIFI_POWER_ON_FIRMWARE_DOWNLOAD_INTERRUPT = 7,

    WIFI_POWER_BFGX_OFF_PULL_WLEN_FAIL = 8,
    WIFI_POWER_BFGX_ON_PULL_WLEN_FAIL = 9,
    WIFI_POWER_ON_FIRMWARE_FILE_OPEN_FAIL = 10,

    WIFI_POWER_ON_CALI_FAIL = 11,

    WIFI_POWER_ENUM_BUTT,
};

#ifdef CONFIG_HI110X_GPS_SYNC
enum gnss_rat_mode_enum {
    GNSS_RAT_MODE_NO_SERVICE = 0,
    GNSS_RAT_MODE_GSM = 1,
    GNSS_RAT_MODE_CDMA = 2,
    GNSS_RAT_MODE_WCDMA = 3,
    GNSS_RAT_MODE_LTE = 6,
    GNSS_RAT_MODE_NR = 11,
    GNSS_RAT_MODE_BUTT
};

enum gnss_sync_mode_enum {
    GNSS_SYNC_MODE_UNKNOWN = -1,
    GNSS_SYNC_MODE_LTE = 0,
    GNSS_SYNC_MODE_LTE2 = 1,
    GNSS_SYNC_MODE_CDMA = 2,
    GNSS_SYNC_MODE_G1 = 3,
    GNSS_SYNC_MODE_G2 = 4,
    GNSS_SYNC_MODE_G3 = 5,
    GNSS_SYNC_MODE_PW = 6,
    GNSS_SYNC_MODE_SW = 7,
    GNSS_SYNC_MODE_NSTU = 8,
    GNSS_SYNC_MODE_BUTT
};

enum gnss_sync_version_enum {
    GNSS_SYNC_VERSION_OFF = 0, // feature switch off
    GNSS_SYNC_VERSION_4G = 1, // support 2/3/4g
    GNSS_SYNC_VERSION_5G = 2, // based 1 and add 5g
    GNSS_SYNC_VERSION_BUTT
};
#endif

#ifdef _PRE_CONFIG_HISI_110X_BLUEZ
struct hisi_bt_dev {
    struct hci_dev *hdev;
    struct task_struct *bt_recv_task;
};
#endif

struct pm_top {
    /* mutex for sync */
    oal_mutex_stru host_mutex;
        /* flag for firmware cfg file init */
    unsigned long firmware_cfg_init_flag;
    struct wlan_pm_s *wlan_pm_info;
    struct pm_drv_data *bfg_pm_info;
    struct pm_drv_data *gnss_pm_info;
};

/* private data for pm driver */
struct pm_drv_data {
    struct ps_core_s *ps_core_data;
    int index;
    /* wake lock for bfg,be used to prevent host form suspend */
    oal_wakelock_stru bus_wake_lock;
    oal_wakelock_stru bt_wake_lock;
    oal_wakelock_stru gnss_wake_lock;

    /* wakelock protect spinlock while wkup isr VS allow sleep ack and devack_timer */
    oal_spin_lock_stru wakelock_protect_spinlock;

    /* bfgx VS. bfg timer spinlock */
    oal_spin_lock_stru uart_state_spinlock;

    /* uart could be used or not */
    uint8_t uart_state;

    /* mark receiving data after set dev as sleep state but before get ack of device */
    uint8_t rcvdata_bef_devack_flag;

    uint8_t uc_dev_ack_wait_cnt;

    /* bfgx sleep state */
    uint8_t bfgx_dev_state;

    /* hal layer ioctl set flag not really close bt */
    uint32_t bt_fake_close_flag;

    oal_spin_lock_stru bfg_irq_spinlock;

    /* bfg irq num */
    uint32_t bfg_irq;

    /* wakeup gpio */
    int32_t wakeup_host_gpio;

    uint32_t ul_irq_stat;

    /* bfg wakeup host count */
    uint32_t bfg_wakeup_host;

    uint32_t wakeup_src_debug;

    /* gnss lowpower state */
    atomic_t gnss_sleep_flag;

    /* flag to mark whether enable lowpower or not */
    uint32_t bfgx_pm_ctrl_enable;
    uint8_t bfgx_lowpower_enable;
    uint8_t bfgx_bt_lowpower_enable;
    uint8_t bfgx_gnss_lowpower_enable;
    uint8_t bfgx_nfc_lowpower_enable;

    /* workqueue for wkup device */
    oal_workqueue_stru *wkup_dev_workqueue;
    oal_work_stru wkup_dev_work;
    oal_work_stru send_disallow_msg_work;
    oal_work_stru send_allow_sleep_work;
    uint32_t uart_baud_switch_to;

    /* wait device ack timer */
    struct timer_list bfg_timer;
    uint32_t bfg_timer_mod_cnt;
    uint32_t bfg_timer_mod_cnt_pre;
    unsigned long bfg_timer_check_time;
    unsigned long rx_pkt_gnss_pre;
    uint32_t gnss_votesleep_check_cnt;
    struct timer_list dev_ack_timer;

    /* the completion for waiting for host wake up device ok */
    struct completion host_wkup_dev_comp;
    /* the completion for waiting for device sleep ack ok */
    struct completion dev_slp_comp;
    /* the completion for waiting for dev boot ok */
    struct completion dev_bootok_ack_comp;
#ifdef _PRE_H2D_GPIO_WKUP
    /* the completion for waiting for cfg wakeup dev ok */
    struct completion gpio_wkup_dev_ack_comp;
    atomic_t cfg_wkup_dev_flag;
#endif
#ifdef _PRE_CONFIG_HISI_110X_BLUEZ
    struct hisi_bt_dev st_bt_dev;
#endif

    int32_t (*change_baud_rate)(struct ps_core_s *, long, uint8_t);
    int32_t (*bfg_power_set)(struct pm_drv_data *, uint8_t, uint8_t);
    void (*bfg_wake_lock)(struct pm_drv_data *);
    void (*bfg_wake_unlock)(struct pm_drv_data *);
    int32_t (*bfgx_dev_state_get)(struct pm_drv_data *);
    void (*bfgx_dev_state_set)(struct pm_drv_data *, uint8_t);
    int32_t (*operate_beat_timer)(uint8_t);
    void (*bfgx_uart_state_set)(struct pm_drv_data *, uint8_t);
    char (*bfgx_uart_state_get)(struct pm_drv_data *);

#ifdef BFGX_UART_DOWNLOAD_SUPPORT
    int32_t (*download_patch)(void);
    int32_t (*recv_patch)(const uint8_t *, int32_t count);
    int32_t (*write_patch)(uint8_t *, int32_t count);
#endif
};

#define  index2name(index) ((index == 0) ? "BUART" : "GUART")

#ifdef CONFIG_HI110X_GPS_SYNC
struct gnss_sync_data {
    void __iomem *addr_base_virt;
    uint32_t addr_offset;
    uint32_t version;
};
#endif

#define BUCK_MODE_OFFSET                        14
#define BUCK_MODE_MASK                          0xC000   // bit[15:14]
#define EXT_BUCK_OFF                             0        // 无外置buck,全内置
#define EXT_BUCK_I2C_CTL                         1        // I2C控制独立外置buck
#define EXT_BUCK_GPIO_CTL                        2        // GPIO控制独立外置buck
#define EXT_BUCK_HOST_CTL                        3        // host控制共享外置buck

#define BUCK_NUMBER_OFFSET                      0
#define BUCK_NUMBER_MASK                        0xFF     // bit[7:0]
#define BUCK_NUMBER_AE                          0x0      // ANNA ELSA上用的外置buck
#define BUCK_NUMBER_ONNT                        0x2      // ONNT 上用的外置buck

/* EXTERN VARIABLE */
extern int g_ram_test_detail_tsensor_dump;
extern int g_ram_test_run_voltage_bias_sel;

#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE
typedef uint32_t (*work_cb)(void);
void pm_host_walcb_register(work_cb suspend_cb, work_cb resume_cb);
#endif
#ifdef _PRE_CONFIG_ARCH_KIRIN_S4_FEATURE
irqreturn_t bfg_wake_host_isr(int irq, void *dev_id);
#endif

/* EXTERN FUNCTION */
void buck_power_ctrl(uint8_t enable, uint8_t subsys);
int32_t wait_gpio_level(int32_t gpio_num, int32_t gpio_level, unsigned long timeout);
void bfgx_gpio_intr_enable(struct pm_drv_data *pm_data, uint32_t ul_en);
bool bfgx_is_boot_up(struct pm_drv_data *pm_data);
void gnss_lowpower_vote(struct pm_drv_data *pm_data, uint32_t vote);
int32_t gnss_me_open(void);
int32_t gnss_me_close(void);
struct pm_drv_data *pm_get_drvdata(uint32_t index);
int32_t host_wkup_dev(struct pm_drv_data *pm_data);
int32_t bfgx_other_subsys_all_shutdown(struct pm_drv_data *pm_data, uint8_t subsys);
void bfgx_print_subsys_state(void);
bool wlan_is_shutdown(void);
bool bfgx_is_shutdown(void);
int32_t wlan_power_on(void);
int32_t wlan_power_off(void);
int32_t wifi_power_fail_process(int32_t error);
int32_t bfgx_pm_feature_set(struct pm_drv_data *pm_data);
int firmware_download_function(uint32_t which_cfg);
typedef int32_t (*firmware_downlaod_privfunc)(void);
int firmware_download_function_priv(uint32_t which_cfg, firmware_downlaod_privfunc);
int32_t hi110x_get_wifi_power_stat(void);
int32_t device_mem_check(unsigned long long *time);
int32_t memcheck_is_working(void);
void bfg_check_timer_work(struct pm_drv_data *pm_data);
#ifdef CONFIG_HI110X_GPS_SYNC
struct gnss_sync_data *gnss_get_sync_data(void);
int gnss_sync_init(void);
void gnss_sync_exit(void);
#endif
#ifdef CONFIG_HUAWEI_DSM
void hw_110x_dsm_client_notify(int sub_sys, int dsm_id, const char *fmt, ...);
#endif
int32_t ps_pm_register(struct ps_core_s *ps_core_d, int index);
int32_t ps_pm_unregister(struct ps_core_s *ps_core_d, int index);
int low_power_init(void);
void low_power_exit(void);
struct pm_top* pm_get_top(void);
#ifdef _PRE_SUSPORT_OEMINFO
void resume_wlan_wakeup_host_gpio(void);
#endif
#ifdef _PRE_H2D_GPIO_WKUP
int32_t host_send_cfg_gpio_wkup_dev_msg(struct pm_drv_data *pm_data);
#endif
#endif
