

#ifndef __OAL_HCC_BUS_H
#define __OAL_HCC_BUS_H

/* 其他头文件包含 */
#include "oal_sdio_comm.h"
#include "oal_net.h"
#include "oal_schedule.h"
#include "oal_hcc_comm.h"
#include "oal_workqueue.h"

#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
#include <linux/sched/signal.h>
#endif

/* 宏定义 */
#define OAL_BUS_RX_THREAD_POLICY        SCHED_NORMAL
#define OAL_BUS_RXDATA_THREAD_PRIORITY  0
#define OAL_BUS_DISPOSE_THREAD_PRIORITY 10

#define OAL_BUS_TEST_WORK_NICE                (-10)
#define OAL_BUS_TEST_WORK_PRIORITY            97
#define OAL_BUS_TEST_INIT_PRIORITY            98
#define OAL_BUS_SWITCH_THREAD_PRIORITY        97
#define OAL_BUS_SWITCH_THREAD_CREATE_PRIORITY 99

/* 高性能CPU，默认值 */
#define OAL_BUS_HPCPU_NUM    4
#define OAL_BUS_HPCPU_NUM_1  5
#define OAL_BUS_MAXCPU_NUM   8

#define OAL_BUS_STATE_TX  (1 << 0)
#define OAL_BUS_STATE_RX  (1 << 1)
#define OAL_BUS_STATE_ALL (OAL_BUS_STATE_TX | OAL_BUS_STATE_RX)

#define OAL_BUS_HCC_NAME_LEN 50
#define OAL_BUS_INI_STR_LEN  100
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define OAL_BUS_BINDCPU_LIMIT (200 * 1024 * 128 / 1500)
#define OAL_BUS_SWITCH_LIMIT  (350 * 1024 * 128 / 1500)
#else
#define OAL_BUS_SDIO_BINDCPU_LIMIT (300 * 1024 * 128 / 1500)
#define OAL_BUS_PCIE_BINDCPU_LIMIT (450 * 1024 * 128 / 1500)
#define OAL_BUS_SWITCH_LIMIT  (350 * 1024 * 128 / 1500)

#define OAL_BUS_DEV_RES_SIZE 10
#endif

typedef struct pm_callback {
    oal_uint (*pm_wakeup_dev)(void);      // ??????PM????,???????,????
    oal_uint (*pm_state_get)(void);       // ????PM??
    oal_uint (*pm_wakeup_host)(void);     // device??host????
    void (*pm_feed_wdg)(void);        // PM Sleep watch dog????
    void (*pm_wakeup_dev_ack)(void);  // ??device?ACK ????
    uint32_t (*pm_disable)(int32_t);
} pm_callback_stru;

typedef int32_t (*hcc_bus_msg_rx)(void *data);

typedef int32_t (*hcc_bus_data_rx)(void *data);
typedef struct _hcc_bus_ops_ {
    hcc_bus_data_rx rx;
} hcc_bus_ops;

struct bus_msg_stru {
    hcc_bus_msg_rx msg_rx;
    void *data;
    uint32_t count;
    uint64_t cpu_time; /* the time of the last come! */
};

/* 全局变量声明 */
extern oal_atomic g_wakeup_dev_wait_ack;

/* 一种类型对应一种IP驱动，PCIE驱动不同SOC架构驱动差异较大 */
#define HCC_BUS_SDIO  0
#define HCC_BUS_PCIE  1 /* 110x PCIE */
#define HCC_BUS_PCIE2 2 /* reserved for 118x's PCIE */
#define HCC_BUS_USB   3
#define HCC_BUS_BUTT  (HCC_BUS_USB + 1)

#define HCC_BUS_SDIO_CAP  (1 << HCC_BUS_SDIO)
#define HCC_BUS_PCIE_CAP  (1 << HCC_BUS_PCIE)
#define HCC_BUS_PCIE2_CAP (1 << HCC_BUS_PCIE2)
#define HCC_BUS_USB_CAP   (1 << HCC_BUS_USB)

/* dev define */
#define HCC_CHIP_110X_DEV 0
#define HCC_CHIP_118X_DEV 1 /* Reserved */
#ifndef _PRE_EMU
#define HOST_WAIT_BOTTOM_INIT_TIMEOUT 20000
#else
#define HOST_WAIT_BOTTOM_INIT_TIMEOUT 200000
#endif
#define HOST_WAIT_INIT_RETRY (HOST_WAIT_BOTTOM_INIT_TIMEOUT / 4)

#define HCC_BUS_PPS_COUNT_TIMEOUT 100 /* 100ms */

/* 队列优先级 */
typedef enum _hcc_bus_q_prio_ {
    HCC_BUS_Q_HIGH,
    HCC_BUS_Q_NORMAL,
    HCC_BUS_Q_LOW,
    HCC_BUS_Q_BUTT
} hcc_bus_q_prio;

/* Power Action */
typedef enum _hcc_bus_power_action_type_ {
    HCC_BUS_POWER_DOWN,               /* The action when wlan power down */
    HCC_BUS_POWER_UP,                 /* The action when wlan power up */
    HCC_BUS_POWER_PATCH_LOAD_PREPARE, /* The action before patch downlaod */
    HCC_BUS_POWER_PATCH_LAUCH,        /* The action after patch download before channel enabled */
    HCC_BUS_SW_POWER_DOWN,            /* switch power down */
    HCC_BUS_SW_POWER_UP,              /* switch power down */
    HCC_BUS_SW_POWER_PATCH_LAUCH,     /* swtich power patch lauch */
    HCC_BUS_SW_POWER_PATCH_LOAD_PREPARE,
    HCC_BUS_POWER_BUTT
} hcc_bus_power_action_type;

typedef enum _hcc_bus_power_ctrl_type_ {
    HCC_BUS_CTRL_POWER_DOWN, /* the func callbed by bus */
    HCC_BUS_CTRL_POWER_UP,   /* the func callbed by bus */
    HCC_BUS_CTRL_POWER_BUTT
} hcc_bus_power_ctrl_type;

#define hcc_to_bus(hcc)   (((struct hcc_handler *)(hcc))->bus_dev->cur_bus)
#define hcc_to_dev(hcc)   (((struct hcc_handler *)(hcc))->bus_dev)
#define hbus_to_dev(bus)  (((hcc_bus *)(bus))->bus_dev)
#define hbus_to_hcc(bus)  (((hcc_bus *)(bus))->bus_dev->hcc)
#define hdev_to_hbus(dev) ((dev)->cur_bus)
typedef struct _hcc_bus_ hcc_bus;
typedef struct _hcc_bus_dev_ hcc_bus_dev;

typedef struct _hcc_switch_action_ {
    oal_dlist_head_stru list;
    int32_t (*switch_notify)(uint32_t dev_id, hcc_bus *old_bus, hcc_bus *new_bus, void *data);
    void *data;
    char *name;
} hcc_switch_action;

typedef struct _hcc_bus_opt_ops {
    int32_t (*get_bus_state)(hcc_bus *pst_bus, uint32_t mask);
    void (*enable_bus_state)(hcc_bus *pst_bus, uint32_t mask);
    void (*disable_bus_state)(hcc_bus *pst_bus, uint32_t mask);

    int32_t (*rx_netbuf_list)(hcc_bus *pst_bus, oal_netbuf_head_stru *pst_head);
    int32_t (*tx_netbuf_list)(hcc_bus *pst_bus, oal_netbuf_head_stru *pst_head, hcc_netbuf_queue_type qtype);

    int32_t (*tx_condition)(hcc_bus *pst_bus, hcc_netbuf_queue_type qtype); /* ip idle ?, 1 is idle, 0 is busy */

    int32_t (*send_msg)(hcc_bus *pst_bus, uint32_t val);

    int32_t (*lock)(hcc_bus *pst_bus);
    int32_t (*unlock)(hcc_bus *pst_bus);

    int32_t (*sleep_request)(hcc_bus *pst_bus);      /* 硬件行为 通知DEV睡眠 */
    int32_t (*sleep_request_host)(hcc_bus *pst_bus); /* 检查Host是否满足睡眠条件 */
    int32_t (*wakeup_request)(hcc_bus *pst_bus);     /* 硬件行为 唤醒DEV */
    int32_t (*get_sleep_state)(hcc_bus *pst_bus);
    int32_t (*wakeup_complete)(hcc_bus *pst_bus);

    int32_t (*switch_suspend_tx)(hcc_bus *pst_bus);
    int32_t (*switch_resume_tx)(hcc_bus *pst_bus);
    int32_t (*get_trans_count)(hcc_bus *pst_bus, uint64_t *tx, uint64_t *rx); /* the pkt nums had transfer done */

    int32_t (*rx_int_mask)(hcc_bus *pst_bus, int32_t is_mask);

    int32_t (*reinit)(hcc_bus *pst_bus); /* ip reinit */
    int32_t (*deinit)(hcc_bus *pst_bus); /* ip deinit */

    /* do something before power on/off */
    int32_t (*power_action)(hcc_bus *pst_bus, hcc_bus_power_action_type action);

    int32_t (*power_ctrl)(hcc_bus *hi_bus, hcc_bus_power_ctrl_type ctrl, int (*func)(void *data), void *data);

    int32_t (*wlan_gpio_handler)(hcc_bus *pst_bus, int32_t irq); /* wlan gpio handler func */
    int32_t (*wlan_gpio_rxdata_proc)(hcc_bus *pst_bus);            /* wlan rx gpio bottom half */

    int32_t (*flowctrl_gpio_handler)(hcc_bus *pst_bus, int32_t irq); /* flowctrl gpio handler func */

    int32_t (*patch_read)(hcc_bus *pst_bus, uint8_t *buff, int32_t len, uint32_t timeout);
    int32_t (*patch_write)(hcc_bus *pst_bus, uint8_t *buff, int32_t len);

    int32_t (*bindcpu)(hcc_bus *pst_bus, uint32_t chan, int32_t is_bind); /* 绑定相关任务，提高处理能力 */

    int32_t (*voltage_bias_init)(hcc_bus *pst_bus);

    void (*chip_info)(hcc_bus *pst_bus, uint32_t is_need_wakeup, uint32_t is_full_log);

    void (*print_trans_info)(hcc_bus *pst_bus, uint64_t print_flag);
    void (*reset_trans_info)(hcc_bus *pst_bus);
    int32_t (*pending_signal_check)(hcc_bus *pst_bus);   /* 调度中检查process条件是否符合, 0:不符合   非0:符合 */
    int32_t (*pending_signal_process)(hcc_bus *pst_bus); /* process中要清掉调度标记，否者会死循环 */

    /* device cpu address<->device pcie address
     * <->host io address<-> host cpu virtual address
     * is_host_va ,
     *   if ture ,src_addr means  host cpu virtual address, return device cpu address
     *   if false ,src_addr means device cpu address, return host virtual address
     */
    int32_t (*master_address_switch)(hcc_bus *pst_bus, uint64_t src_addr, uint64_t* dst_addr, int32_t is_host_va);

    /* host ddr iommu/smmu address <-> pcie slave address
     * is_host_iova ,
     *   if ture ,src_addr means smmu io virtual address or kernel dma address
     *   if false ,src_addr means pcie ep slave address(start with 0x80000000)
     */
    int32_t (*slave_address_switch)(hcc_bus *pst_bus, uint64_t src_addr, uint64_t* dst_addr, int32_t is_host_iova);
#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
    void (*wlan_pm_vote)(hcc_bus *hi_bus, uint8_t uc_allow);
#endif
} hcc_bus_opt_ops;

typedef struct _hcc_bus_cap_ {
    /* 接口是否为全双工,全双工软件收发不能相互阻塞 */
    int32_t is_full_duplex; /* 预留 */

    /* TX/RX 长度对齐要求，业务使用 */
    uint32_t align_size[HCC_DIR_COUNT]; /* bus align request */

    uint32_t max_trans_size; /* IP层一次可以传输的最大长度 */
} hcc_bus_cap;

struct _hcc_bus_ {
    oal_dlist_head_stru list;
    uint32_t bus_type; /* current bus type, sdio/usb/pcie */
    uint32_t bus_id;
    uint32_t dev_id;

    hcc_bus_cap cap;

    hcc_bus_dev *bus_dev;

    char name[OAL_BUS_HCC_NAME_LEN];

    /* IP 私有结构体索引 */
    void *data; /* dev driver strut reference, sdio~usb */

    void *hcc; /* reference to hcc */

    /* pm part */
    struct semaphore sr_wake_sema;     /* suspend and resume sem */
    pm_callback_stru *pst_pm_callback; /* ??????? */

    oal_wakelock_stru st_bus_wakelock;

    struct task_struct *pst_rx_tsk;  /* ???? */
    struct semaphore rx_sema;        /* ????? */
    oal_mutex_stru rx_transfer_lock; /* ????? */

    void *bus_ops_data;
    hcc_bus_ops bus_ops;

    hcc_bus_opt_ops *opt_ops;

    oal_completion st_device_ready;

    struct bus_msg_stru msg[D2H_MSG_COUNT];
    uint32_t last_msg;

    oal_work_stru bus_excp_worker;
    int32_t bus_excp_type;
    oal_spin_lock_stru bus_excp_lock;
    oal_wakelock_stru bus_excp_wlock; /* kernel wakelock */

    uint64_t gpio_int_count;
    uint64_t data_int_count;
    uint64_t wakeup_int_count;

    uint32_t is_bind;

    uint32_t mem_size;
    struct st_wifi_dump_mem_info *mem_info;
};

#define HCC_BUS_SWITCH_STATE_START 0
#define HCC_BUS_SWITCH_STATE_END   1
#define HCC_BUS_SWITCH_STATE_ABORT 2

struct _hcc_bus_dev_ {
    uint32_t dev_id;        /* bus id, reference to the chip */
    uint32_t init_bus_type; /* init select bus type */
    uint32_t bus_cap;       /* support bus type */

    uint32_t pcie_bugfix_enable;        /* 1 for enable */
    oal_wakelock_stru st_switch_wakelock; /* wake lock for switch */

    uint32_t bus_auto_bindcpu; /* 动态绑核 */

    unsigned long bus_pps_start_time;
    oal_timer_list_stru bus_pps_timer;

    int32_t wlan_irq; /* gpio for wlan transfer */
    int32_t irq_stat; /* 0:enable, 1:disable */
    int32_t is_wakeup_gpio_support;
    oal_spin_lock_stru st_irq_lock;

    struct hcc_handler *hcc;

    hcc_bus *cur_bus;

    uint32_t bus_num; /* for each cap bit */
    char *name;

    int32_t                   l_flowctrl_irq;             /* gpio irq for hcc non-high priority flowctrl */
    oal_bool_enum_uint8         en_flowctrl_irq_stat;       /* irq status. 0: disabled, 1: enabled */
    oal_bool_enum_uint8         en_flowctrl_gpio_registered; /* is gpio flowctrl registered */
};

typedef struct _hcc_bus_res_ {
    uint32_t cur_type;

    oal_spin_lock_stru lock;
    oal_dlist_head_stru head;
} hcc_bus_res;

#define HCC_BUS_SWITCH_POWERUP   1
#define HCC_BUS_SWITCH_POWERDOWN 2

#define HCC_BUS_TRANSFER_SWITCH_ACTION 0

typedef struct _hcc_switch_info_ {
    uint8_t action_type; /* action type */
    uint8_t bus_id;      /* ip index, the next work ip */
    uint8_t is_need_resp;
} hcc_switch_info;

typedef struct _hcc_switch_response_ {
    uint8_t action_type; /* action type */
    uint8_t bus_id;      /* ip index */
    uint8_t is_succ;
} hcc_switch_response;

/* 函数声明 */
extern hcc_bus *hcc_get_current_110x_bus(void);
extern hcc_bus_dev *hcc_get_bus_dev(uint32_t dev_id);
extern int32_t hcc_bus_bindcpu(hcc_bus *hi_bus, uint32_t chan, int32_t is_bind);
extern int32_t hcc_dev_bindcpu(uint32_t dev_id, int32_t is_bind);
extern int32_t hi110x_hcc_dev_bindcpu(int32_t is_bind, oal_bool_enum_uint8 is_userctl,
                                      uint8_t irq_cpu, uint8_t thread_cmd);

#ifdef _PRE_PRODUCT_HI1620S_KUNPENG

#else

#if defined(CONFIG_ARCH_HISI)
#if defined(CONFIG_ARCH_PLATFORM)
extern void get_slow_cpus(struct cpumask *cpumask);
extern void get_fast_cpus(struct cpumask *cpumask);
#else
extern void hisi_get_slow_cpus(struct cpumask *cpumask);
extern void hisi_get_fast_cpus(struct cpumask *cpumask);
#endif
#endif
#endif
extern int32_t g_pcie_ringbuf_bugfix_enable;
extern oal_atomic g_bus_powerup_dev_wait_ack;
extern uint32_t hcc_bus_flowctrl_init(uint8_t uc_hcc_flowctrl_type);

/*
 * 函 数 名  : hcc_bus_wake_lock
 * 功能描述  : 获取wifi wakelock锁
 */
#define hcc_bus_wake_lock(pst_hi_bus) oal_wake_lock(&(pst_hi_bus)->st_bus_wakelock)

/*
 * 函 数 名  : hcc_bus_wake_unlock
 * 功能描述  : 释放wifi wakelock锁
 */
#define hcc_bus_wake_unlock(pst_hi_bus) oal_wake_unlock(&(pst_hi_bus)->st_bus_wakelock)

/*
 * 函 数 名  : hcc_bus_wakelock_active
 * 功能描述  : 判断 wifi wakelock锁是否active
 */
#define hcc_bus_wakelock_active(pst_hi_bus) oal_wakelock_active(&(pst_hi_bus)->st_bus_wakelock)
extern struct task_struct *oal_thread_create(int (*threadfn)(void *data),
                                             void *data,
                                             struct semaphore *sema_sync,
                                             const char *namefmt,
                                             uint32_t policy,
                                             int32_t prio,
                                             int32_t cpuid);
extern void oal_thread_stop(struct task_struct *tsk,
                            struct semaphore *sema_sync);
extern void oal_set_thread_property(struct task_struct *p, int policy,
#ifdef _PRE_LINUX_TEST
                                    const struct oal_sched_param_stru *param,
#else
                                    const struct sched_param *param,
#endif
                                    long nice);


extern int32_t hcc_add_bus(hcc_bus *pst_bus, const char *name);
extern int32_t hcc_remove_bus(hcc_bus *pst_bus);
extern hcc_bus *hcc_alloc_bus(void);
extern void hcc_free_bus(hcc_bus *pst_bus);
extern int32_t hcc_dev_init(void);
extern int32_t oal_wifi_platform_load_dev(void);
extern void oal_wifi_platform_unload_dev(void);
extern int32_t oal_trigger_bus_exception(hcc_bus *hi_bus, int32_t is_sync);
extern int32_t hcc_bus_exception_is_busy(hcc_bus *hi_bus);
extern void hcc_bus_exception_submit(hcc_bus *hi_bus, int32_t excep_type);
extern uint32_t hcc_bus_dump_mem_check(hcc_bus *hi_bus);
extern void hcc_bus_try_to_dump_device_mem(hcc_bus *hi_bus, int32_t is_sync);
extern void hcc_bus_wakelocks_release_detect(hcc_bus *pst_bus);
extern void oal_wlan_gpio_intr_enable(hcc_bus_dev *pst_bus_dev, uint32_t ul_en);

extern int32_t hcc_bus_message_register(hcc_bus *hi_bus, uint8_t msg, hcc_bus_msg_rx cb, void *data);
extern void hcc_bus_message_unregister(hcc_bus *hi_bus, uint8_t msg);
extern int32_t hcc_bus_transfer_rx_register(hcc_bus *hi_bus, void *data, hcc_bus_data_rx rx);
extern void hcc_bus_transfer_rx_unregister(hcc_bus *hi_bus);
extern int32_t hcc_bus_cap_init(uint32_t dev_id, const char *bus_select);
extern uint64_t oal_get_gpio_int_count_para(void);
extern int32_t hcc_switch_action_register(hcc_switch_action *action, void *data);
extern void hcc_switch_action_unregister(hcc_switch_action *action);
extern int32_t hcc_switch_bus(uint32_t dev_id, uint32_t bus_type);
extern int32_t hcc_send_message(struct hcc_handler *hcc, uint32_t val);
extern int32_t hcc_bus_performance_core_schedule(uint32_t dev_id);
extern int32_t hcc_bus_pm_wakeup_device(hcc_bus *hi_bus);
extern void oal_unregister_gpio_flowctrl_intr(hcc_bus_dev *pst_bus_dev);
extern void hcc_bus_sched_flowctrl_gpio_task(hcc_bus* pst_bus, int32_t l_irq);
extern int32_t hi110x_hcc_ip_pm_ctrl(uint32_t enable);
extern hcc_bus *hcc_find_bus_by_devid(uint32_t dev_id, uint32_t bus_type);
extern int32_t hcc_bus_isvalid(hcc_bus *pst_bus);
#ifdef _PRE_CONFIG_ARCH_KIRIN_S4_FEATURE
extern void oal_free_irq_in_s4(void);
extern void oal_request_irq_in_s4(void);
extern irqreturn_t hcc_bus_wlan_gpio_irq(int32_t irq, void *dev_id);
#endif

OAL_STATIC OAL_INLINE void hcc_bus_rx_transfer_lock(hcc_bus *hi_bus)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (oal_warn_on(hi_bus == NULL)) {
        return;
    }
    mutex_lock(&hi_bus->rx_transfer_lock);
#endif
}

OAL_STATIC OAL_INLINE void hcc_bus_rx_transfer_unlock(hcc_bus *hi_bus)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (oal_warn_on(hi_bus == NULL)) {
        return;
    }
    mutex_unlock(&hi_bus->rx_transfer_lock);
#endif
}

OAL_STATIC OAL_INLINE uint32_t hcc_bus_get_max_trans_size(hcc_bus *hi_bus)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return 0;
    }

    return hi_bus->cap.max_trans_size;
}

/*
 * Prototype    : hcc_bus_get_state
 * Description  : get the sdio bus state
 */
OAL_STATIC OAL_INLINE int32_t hcc_bus_get_state(hcc_bus *hi_bus, uint32_t mask)
{
    if (oal_warn_on(hi_bus == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hcc_bus_get_state: hibus is null");
        return 0;
    }

    if (oal_warn_on(hi_bus->opt_ops == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hcc_bus_get_state: hi_bus->opt_ops is null");
        return 0;
    }

    if (hi_bus->opt_ops->get_bus_state == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hcc_bus_get_state: hi_bus->opt_ops->get_bus_state is null");
        return 0;
    }

    return hi_bus->opt_ops->get_bus_state(hi_bus, mask);
}

/*
 * Prototype    : hcc_bus_enable_state
 * Description  : set the sdio bus state
 */
OAL_STATIC OAL_INLINE void hcc_bus_enable_state(hcc_bus *hi_bus, uint32_t mask)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return;
    }

    if (oal_warn_on(hi_bus->opt_ops == NULL)) {
        return;
    }

    if (hi_bus->opt_ops->enable_bus_state == NULL) {
        return;
    }

    hi_bus->opt_ops->enable_bus_state(hi_bus, mask);
}

OAL_STATIC OAL_INLINE void hcc_bus_disable_state(hcc_bus *hi_bus, uint32_t mask)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return;
    }

    if (oal_warn_on(hi_bus->opt_ops == NULL)) {
        return;
    }

    if (hi_bus->opt_ops->disable_bus_state == NULL) {
        return;
    }

    hi_bus->opt_ops->disable_bus_state(hi_bus, mask);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_lock(hcc_bus *hi_bus)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return -OAL_ENODEV;
    }

    if (oal_warn_on(hi_bus->opt_ops == NULL)) {
        return -OAL_ENODEV;
    }

    if (hi_bus->opt_ops->lock == NULL) {
        return -OAL_EIO;
    }

    return hi_bus->opt_ops->lock(hi_bus);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_unlock(hcc_bus *hi_bus)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return -OAL_ENODEV;
    }

    if (oal_warn_on(hi_bus->opt_ops == NULL)) {
        return -OAL_ENODEV;
    }

    if (hi_bus->opt_ops->unlock == NULL) {
        return -OAL_EIO;
    }

    return hi_bus->opt_ops->unlock(hi_bus);
}

OAL_STATIC OAL_INLINE void hcc_bus_and_wake_lock(hcc_bus *hi_bus)
{
    hcc_bus_wake_lock(hi_bus);
    hcc_bus_lock(hi_bus);
}

OAL_STATIC OAL_INLINE void hcc_bus_and_wake_unlock(hcc_bus *hi_bus)
{
    hcc_bus_unlock(hi_bus);
    hcc_bus_wake_unlock(hi_bus);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_sleep_request(hcc_bus *hi_bus)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return -OAL_ENODEV;
    }

    if (oal_warn_on(hi_bus->opt_ops == NULL)) {
        return -OAL_ENODEV;
    }

    if (hi_bus->opt_ops->sleep_request == NULL) {
        return -OAL_EIO;
    }

    return hi_bus->opt_ops->sleep_request(hi_bus);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_sleep_request_host(hcc_bus *hi_bus)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return -OAL_ENODEV;
    }

    if (oal_warn_on(hi_bus->opt_ops == NULL)) {
        return -OAL_ENODEV;
    }

    if (hi_bus->opt_ops->sleep_request_host == NULL) {
        return -OAL_EIO;
    }

    return hi_bus->opt_ops->sleep_request_host(hi_bus);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_get_sleep_state(hcc_bus *hi_bus)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return -OAL_ENODEV;
    }

    if (oal_warn_on(hi_bus->opt_ops == NULL)) {
        return -OAL_ENODEV;
    }

    if (hi_bus->opt_ops->get_sleep_state == NULL) {
        return -OAL_EIO;
    }

    return hi_bus->opt_ops->get_sleep_state(hi_bus);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_rx_int_mask(hcc_bus *hi_bus)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return -OAL_ENODEV;
    }

    if (oal_warn_on(hi_bus->opt_ops == NULL)) {
        return -OAL_ENODEV;
    }

    if (hi_bus->opt_ops->rx_int_mask == NULL) {
        return -OAL_EIO;
    }

    return hi_bus->opt_ops->rx_int_mask(hi_bus, 1);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_rx_int_unmask(hcc_bus *hi_bus)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return -OAL_ENODEV;
    }

    if (oal_warn_on(hi_bus->opt_ops == NULL)) {
        return -OAL_ENODEV;
    }

    if (hi_bus->opt_ops->rx_int_mask == NULL) {
        return -OAL_EIO;
    }

    return hi_bus->opt_ops->rx_int_mask(hi_bus, 0);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_get_trans_count(hcc_bus *hi_bus, uint64_t *tx, uint64_t *rx)
{
    if (oal_likely(tx)) {
        *tx = 0;
    }

    if (oal_likely(rx)) {
        *rx = 0;
    }

    if (oal_warn_on(hi_bus == NULL)) {
        return -OAL_ENODEV;
    }

    if (hi_bus->opt_ops->get_trans_count == NULL) {
        return -OAL_EIO;
    }

    return hi_bus->opt_ops->get_trans_count(hi_bus, tx, rx);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_wakeup_request(hcc_bus *hi_bus)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return -OAL_ENODEV;
    }

    if (oal_warn_on(hi_bus->opt_ops == NULL)) {
        return -OAL_ENODEV;
    }

    if (hi_bus->opt_ops->wakeup_request == NULL) {
        return -OAL_EIO;
    }

    return hi_bus->opt_ops->wakeup_request(hi_bus);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_power_action(hcc_bus *hi_bus, hcc_bus_power_action_type action)
{
    if (oal_unlikely(hi_bus == NULL)) {
        return -OAL_ENODEV;
    }

    if (oal_warn_on(hi_bus->opt_ops == NULL)) {
        return -OAL_ENODEV;
    }

    if (hi_bus->opt_ops->power_action == NULL) {
        return -OAL_EIO;
    }

    return hi_bus->opt_ops->power_action(hi_bus, action);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_power_ctrl_register(hcc_bus *hi_bus, hcc_bus_power_ctrl_type ctrl,
                                                          int (*func)(void *data), void *data)
{
    if (oal_unlikely(hi_bus == NULL)) {
        if (func != NULL) {
            /* ip driver no callback, call the function directly */
            return func(data);
        }
        return -OAL_ENODEV;
    }

    if (oal_warn_on(hi_bus->opt_ops == NULL)) {
        return -OAL_ENODEV;
    }

    if (hi_bus->opt_ops->power_ctrl == NULL) {
        if (func != NULL) {
            /* ip driver no callback, call the function directly */
            return func(data);
        }
        return -OAL_EIO;
    }

    return hi_bus->opt_ops->power_ctrl(hi_bus, ctrl, func, data);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_send_message(hcc_bus *hi_bus, uint32_t val)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return -OAL_EINVAL;
    }

    if (oal_likely(hi_bus->opt_ops->send_msg)) {
        return hi_bus->opt_ops->send_msg(hi_bus, val);
    } else {
        return -OAL_ENODEV;
    }
}

extern uint32_t g_hcc_tx_err_cnt;
OAL_STATIC OAL_INLINE int32_t hcc_bus_tx_netbuf_list(hcc_bus *hi_bus, oal_netbuf_head_stru *pst_head,
                                                     hcc_netbuf_queue_type qtype)
{
    int32_t ret;
    const uint32_t ul_long_delay_tx_err_times = 2;

    if (oal_unlikely(!hi_bus)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hcc_bus_tx_netbuf_list: bus is null");
        return -OAL_EINVAL;
    }

    if (oal_likely(hi_bus->opt_ops->tx_netbuf_list)) {
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "tx netbuf list:%s, skb head:%p, len:%d, qtype:%d",
                             hi_bus->bus_type == HCC_BUS_SDIO ? "sdio" : "pcie",
                             oal_netbuf_head_next(pst_head),
                             oal_netbuf_list_len(pst_head),
                             qtype);

        ret = hi_bus->opt_ops->tx_netbuf_list(hi_bus, pst_head, qtype);
        if (oal_unlikely(ret < 0)) {
            g_hcc_tx_err_cnt++;
            if (g_hcc_tx_err_cnt >= ul_long_delay_tx_err_times) {
                oal_msleep(1000);
            } else {
                oal_msleep(100);
            }
            return 0;
        } else {
            if (oal_unlikely(g_hcc_tx_err_cnt)) {
                g_hcc_tx_err_cnt = 0;
            }
            return ret;
        }
    } else {
        return 0;
    }
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_rx_netbuf_list(hcc_bus *hi_bus, oal_netbuf_head_stru *pst_head)
{
    if (oal_unlikely(!hi_bus)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hcc_bus_rx_netbuf_list: bus is null");
        return -OAL_EINVAL;
    }

    if (oal_likely(hi_bus->opt_ops->rx_netbuf_list)) {
        return hi_bus->opt_ops->rx_netbuf_list(hi_bus, pst_head);
    } else {
        return -OAL_ENODEV;
    }
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_reinit(hcc_bus *hi_bus)
{
    if (oal_unlikely(hi_bus == NULL)) {
        return -OAL_EINVAL;
    }

    if (oal_unlikely(hi_bus->opt_ops->reinit == NULL)) {
        return -OAL_ENODEV;
    }

    return hi_bus->opt_ops->reinit(hi_bus);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_voltage_bias_init(hcc_bus *hi_bus)
{
    if (oal_unlikely(hi_bus == NULL)) {
        return -OAL_EINVAL;
    }

    if (oal_unlikely(hi_bus->opt_ops->voltage_bias_init == NULL)) {
        return -OAL_ENODEV;
    }

    return hi_bus->opt_ops->voltage_bias_init(hi_bus);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_check_tx_condition(hcc_bus *hi_bus, hcc_netbuf_queue_type qtype)
{
    if (oal_unlikely(hi_bus == NULL)) {
        return OAL_TRUE;
    }

    if (oal_unlikely(hi_bus->opt_ops->tx_condition == NULL)) {
        /* 没有回调说明 发送不受限制 直接返回1 */
        return OAL_TRUE;
    }

    return hi_bus->opt_ops->tx_condition(hi_bus, qtype);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_patch_read(hcc_bus *hi_bus, uint8_t *buff, int32_t len, uint32_t timeout)
{
    if (oal_warn_on((hi_bus == NULL) || (buff == NULL))) {
        return -OAL_EINVAL;
    }

    if (oal_unlikely(hi_bus->opt_ops->patch_read == NULL)) {
        return -OAL_EIO;
    }

    return hi_bus->opt_ops->patch_read(hi_bus, buff, len, timeout);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_patch_write(hcc_bus *hi_bus, uint8_t *buff, int32_t len)
{
    if (oal_warn_on((hi_bus == NULL) || (buff == NULL))) {
        return -OAL_EINVAL;
    }

    if (oal_unlikely(hi_bus->opt_ops->patch_write == NULL)) {
        return -OAL_EIO;
    }

    return hi_bus->opt_ops->patch_write(hi_bus, buff, len);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_wakeup_complete(hcc_bus *hi_bus)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return -OAL_EINVAL;
    }

    if (oal_unlikely(hi_bus->opt_ops->wakeup_complete == NULL)) {
        return -OAL_EIO;
    }

    return hi_bus->opt_ops->wakeup_complete(hi_bus);
}

OAL_STATIC OAL_INLINE void hcc_bus_chip_info(hcc_bus *hi_bus, uint32_t is_need_wakeup, uint32_t is_full_log)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return;
    }

    if (oal_unlikely(hi_bus->opt_ops->chip_info == NULL)) {
        return;
    }

    hi_bus->opt_ops->chip_info(hi_bus, is_need_wakeup, is_full_log);
}

#define HCC_PRINT_TRANS_FLAG_DEVICE_STAT (1 << 0)
#define HCC_PRINT_TRANS_FLAG_DEVICE_REGS (1 << 1)
OAL_STATIC OAL_INLINE void hcc_bus_print_trans_info(hcc_bus *hi_bus, uint64_t print_flag)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return;
    }

    if (oal_unlikely(hi_bus->opt_ops->print_trans_info == NULL)) {
        return;
    }

    /* 打印device信息要保证打印过程中 不会进入深睡 */
    hi_bus->opt_ops->print_trans_info(hi_bus, print_flag);
}

OAL_STATIC OAL_INLINE void hcc_bus_reset_trans_info(hcc_bus *hi_bus)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return;
    }

    if (oal_unlikely(hi_bus->opt_ops->reset_trans_info == NULL)) {
        return;
    }

    hi_bus->opt_ops->reset_trans_info(hi_bus);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_pending_signal_check(hcc_bus *hi_bus)
{
    if (oal_unlikely(hi_bus == NULL)) {
        return OAL_FALSE;
    }

    if (oal_unlikely(hi_bus->opt_ops->pending_signal_check == NULL)) {
        return OAL_FALSE;
    }

    return hi_bus->opt_ops->pending_signal_check(hi_bus);
}

OAL_STATIC OAL_INLINE int32_t hcc_bus_pending_signal_process(hcc_bus *hi_bus)
{
    if (oal_unlikely(hi_bus == NULL)) {
        return 0;
    }

    if (oal_unlikely(hi_bus->opt_ops->pending_signal_process == NULL)) {
        return 0;
    }

    return hi_bus->opt_ops->pending_signal_process(hi_bus);
}

#define MASTER_VA_TO_WCPUADDR   OAL_TRUE
#define MASTER_WCPUADDR_TO_VA   OAL_FALSE
OAL_STATIC OAL_INLINE int32_t hcc_bus_master_address_switch(hcc_bus *hi_bus, uint64_t src_addr,
                                                            uint64_t *dst_addr, int32_t is_host_va)
{
    if (oal_unlikely(hi_bus == NULL)) {
        return -OAL_ENODEV;
    }

    if (oal_unlikely(hi_bus->opt_ops->master_address_switch == NULL)) {
        return -OAL_ENODEV;
    }

    return hi_bus->opt_ops->master_address_switch(hi_bus, src_addr, dst_addr, is_host_va);
}

#define SLAVE_IOVA_TO_PCI_SLV   OAL_TRUE
#define SLAVE_PCI_SLV_TO_IOVA   OAL_FALSE
OAL_STATIC OAL_INLINE int32_t hcc_bus_slave_address_switch(hcc_bus *hi_bus, uint64_t src_addr,
                                                           uint64_t *dst_addr, int32_t is_host_iova)
{
    if (oal_unlikely(hi_bus == NULL)) {
        return -OAL_ENODEV;
    }

    if (oal_unlikely(hi_bus->opt_ops->slave_address_switch == NULL)) {
        return -OAL_ENODEV;
    }

    return hi_bus->opt_ops->slave_address_switch(hi_bus, src_addr, dst_addr, is_host_iova);
}

#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
OAL_STATIC OAL_INLINE void hcc_bus_wlan_pm_vote(hcc_bus *hi_bus, uint8_t uc_allow)
{
    if (oal_unlikely(hi_bus == NULL)) {
        return;
    }

    if (oal_unlikely(hi_bus->opt_ops->wlan_pm_vote == NULL)) {
        return;
    }

    hi_bus->opt_ops->wlan_pm_vote(hi_bus, uc_allow);
}
#endif

#endif /* end of Hcc.h */
