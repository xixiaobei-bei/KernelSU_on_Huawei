

#ifndef __OAL_HCC_HOST_IF_H
#define __OAL_HCC_HOST_IF_H

#include "oal_ext_if.h"
#include "oal_util.h"
#include "oal_net.h"

#include "oal_sdio_host_if.h"
#include "oal_hcc_comm.h"
#include "oal_workqueue.h"
#include "pcie_host.h"
#include "oal_hcc_bus.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/kernel.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/sched/debug.h>
#endif
#include "board.h"
#endif

#ifdef _PRE_WLAN_TCP_OPT
#define TCP_ACK_WAIT_HCC_SCHDULE_COUNT 1
#endif
#define CONFIG_CREDIT_MSG_FLOW_WATER_LINE 60

#define HCC_FLOW_HIGH_PRI_BUFF_CNT 5 /* device侧预留的高优先级专用buffer个数，要与OAL_NETBUF_HIGH_PRIORITY_COUNT相同 */

#define HCC_FLUSH_ALL (~0UL)

/* hcc tx transfer flow control */
#define HCC_FC_NONE 0x0 /* 对调用者不进行流控，netbuf一直缓冲在hcc队列中,这种类型的数据包不宜过多 */
#define HCC_FC_WAIT 0x1 /* 阻塞等待，如果是在中断上下文调用，该标记被自动清除,非中断上下文生效 */
#define HCC_FC_NET  0x2 /* 对于网络层的流控 */
#define HCC_FC_DROP 0x4 /* 流控采用丢包方式,流控时返回成功 */
#define HCC_FC_ALL  (HCC_FC_WAIT | HCC_FC_NET | HCC_FC_DROP)

struct hcc_transfer_param {
    uint32_t main_type;
    uint32_t sub_type;
    uint32_t extend_len;
    uint32_t fc_flag;  /* 流控标记 */
    uint32_t queue_id; /* 期望进入的队列号, */
};
#define OAL_SAVE_MODE_BUFF_SIZE 128

#define OAL_PHY_LOOPBACK_IS_MAC         0
#define OAL_PHY_LOOPBACK_IS_INTERNALPHY 1
#define OAL_PHY_LOOPBACK_IS_REMOTEPHY   2

#define HCC_TX_LOW_WATERLINE  512
#define HCC_TX_HIGH_WATERLINE 1024

#define HCC_RX_LOW_WATERLINE  128
#define HCC_RX_HIGH_WATERLINE 512

#define CTRL_BURST_LIMIT    256
#define CTRL_LOW_WATERLINE  128
#define CTRL_HIGH_WATERLINE 256

#define DATA_HI_BURST_LIMIT    256
#ifdef _PRE_WINDOWS_SUPPORT
#define DATA_HI_LOW_WATERLINE  256
#define DATA_HI_HIGH_WATERLINE 512
#else
#define DATA_HI_LOW_WATERLINE  128
#define DATA_HI_HIGH_WATERLINE 256
#endif

#define DATA_LO_BURST_LIMIT    256
#define DATA_LO_LOW_WATERLINE  128
#define DATA_LO_HIGH_WATERLINE 256

#define TCP_DATA_BURST_LIMIT    256
#define TCP_DATA_LOW_WATERLINE  128
#define TCP_DATA_HIGH_WATERLINE 256

#define TCP_ACK_BURST_LIMIT    256
#define TCP_ACK_LOW_WATERLINE  128
#define TCP_ACK_HIGH_WATERLINE 256

#define UDP_BK_BURST_LIMIT    10
#define UDP_BK_LOW_WATERLINE  10
#define UDP_BK_HIGH_WATERLINE 20

#define UDP_BE_BURST_LIMIT    20
#define UDP_BE_LOW_WATERLINE  20
#define UDP_BE_HIGH_WATERLINE 40

#define UDP_VI_BURST_LIMIT    40
#define UDP_VI_LOW_WATERLINE  40
#define UDP_VI_HIGH_WATERLINE 80

#define UDP_VO_BURST_LIMIT    60
#define UDP_VO_LOW_WATERLINE  60
#define UDP_VO_HIGH_WATERLINE 120

typedef enum _hcc_queue_type_ {
    CTRL_QUEUE = 0,
    DATA_HI_QUEUE,
    DATA_HI_QUEUE2, /* high queue2 for wifi tae */
    DATA_LO_QUEUE,

    DATA_TCP_DATA_QUEUE,
    DATA_TCP_ACK_QUEUE,

    DATA_UDP_BK_QUEUE,
    DATA_UDP_BE_QUEUE,
    DATA_UDP_VI_QUEUE,
    DATA_UDP_VO_QUEUE,

    HCC_QUEUE_COUNT
} hcc_queue_type;

#define declare_hcc_tx_param_initializer(name, mtype, stype, ext_len, arg_flag, queueid) \
    struct hcc_transfer_param name;                                                      \
    hcc_hdr_param_init(&(name), mtype, stype, ext_len, arg_flag, queueid);

/* the macro to set hcc hdr */
OAL_STATIC OAL_INLINE void hcc_hdr_param_init(struct hcc_transfer_param *param,
                                              uint32_t main_type,
                                              uint32_t sub_type,
                                              uint32_t extend_len,
                                              uint32_t fc_flag,
                                              uint32_t queue_id)
{
    if (oal_unlikely(param == NULL)) {
        oal_warn_on(1);
        return;
    }
    param->main_type = main_type;
    param->sub_type = sub_type;
    param->extend_len = extend_len;
    param->fc_flag = fc_flag;
    param->queue_id = queue_id;
}

/* 低功耗wlan侧收发包统计:统计数据不准确(未加锁,调度数量不确定),仅用于判断是否可以进低功耗 */
typedef struct {
    uint32_t total_trx_pkt_cnt; /* trx报文总数统计 */
    uint32_t ring_tx_pkt; /* ring发包统计 */
    uint32_t h2d_tx_pkt; /* 非ring发包,包括tid7/mgmt帧/vip帧/非单播数据帧等 */
    uint32_t d2h_rx_pkt;  /* 非host ring rx，hcc接收报文上报 */
    uint32_t host_rx_pkt; /* host ring接收报文统计 */
} pm_wlan_pkt_statis_stru;
extern pm_wlan_pkt_statis_stru g_pm_wlan_pkt_statis;

extern uint32_t g_hcc_assemble_count;

#define hcc_get_chan_string(type) (((type) == HCC_TX) ? "tx" : "rx")

typedef enum _hcc_send_mode_ {
    HCC_SINGLE_SEND = 0,
    HCC_ASSEM_SEND,
    HCC_MODE_COUNT
} hcc_send_mode;

typedef enum _hcc_flowctrl_type_ {
    HCC_FLOWCTRL_SDIO,
    HCC_FLOWCTRL_CREDIT,
    HCC_FLOWCTRL_BUTT
} hcc_flowctrl_type;

typedef struct _hcc_flow_ctrl_ {
    uint8_t flowctrl_flag;
    uint8_t enable;
    uint16_t flow_type;
    uint16_t is_stopped;
    uint16_t low_waterline;
    uint16_t high_waterline;
} hcc_flow_ctrl;

typedef struct _hcc_trans_queue_ {
    /* transfer pkts limit every loop */
    uint32_t netbuf_pool_type;
    uint32_t burst_limit;
    hcc_flow_ctrl flow_ctrl;
    hcc_send_mode send_mode;
    uint32_t total_pkts;
    uint32_t loss_pkts;
    oal_netbuf_head_stru data_queue;
    oal_spin_lock_stru data_queue_lock;
    wlan_net_queue_type wlan_queue_id;
} hcc_trans_queue;

#define HCC_TX_ASSEM_INFO_MAX_NUM (HISDIO_HOST2DEV_SCATT_MAX + 1)
#define HCC_RX_ASSEM_INFO_MAX_NUM (HISDIO_DEV2HOST_SCATT_MAX + 1)
typedef struct _hcc_tx_assem_info_ {
    /* The +1 for the single pkt */
    uint32_t info[HCC_TX_ASSEM_INFO_MAX_NUM];

    uint64_t pkt_cnt; /* pps count for switch */

    /* The max support assemble pkts */
    uint32_t assemble_max_count;

    /* netx assem pkts list */
    oal_netbuf_head_stru assembled_head;
    /* the queue is assembling */
    hcc_queue_type assembled_queue_type;
} hcc_tx_assem_info;

typedef struct _hcc_rx_assem_info_ {
    uint64_t pkt_cnt; /* pps count for switch */
    uint32_t info[HCC_RX_ASSEM_INFO_MAX_NUM];
} hcc_rx_assem_info;

typedef struct _hcc_netbuf_stru_ {
    oal_netbuf_stru *pst_netbuf;
    int32_t len; /* for hcc transfer */
} hcc_netbuf_stru;

typedef int32_t (*hcc_rx_pre_do)(struct hcc_handler *hcc, uint8_t stype,
    hcc_netbuf_stru *pst_netbuf, uint8_t **pre_do_context);
typedef int32_t (*hcc_rx_post_do)(struct hcc_handler *hcc, uint8_t stype,
    hcc_netbuf_stru *pst_netbuf, uint8_t *pre_do_context);

struct hcc_handler;
typedef struct _hcc_rx_action_ {
    uint32_t pkts_count;
    hcc_rx_pre_do pre_do;
    hcc_rx_post_do post_do;
    void *context; /* the callback argument */
    struct hcc_handler *hcc;
} hcc_rx_action;

typedef struct _hcc_rx_action_info_ {
    hcc_rx_action action[HCC_ACTION_TYPE_BUTT];
} hcc_rx_action_info;

typedef struct _hcc_trans_queues_ {
    hcc_trans_queue queues[HCC_QUEUE_COUNT];
} hcc_trans_queues;

typedef oal_bool_enum_uint8 (*hcc_flowctl_get_mode)(void);
void hcc_flowctl_get_device_mode_register(hcc_flowctl_get_mode get_mode);

typedef void (*hcc_flowctl_start_subq)(uint16_t us_queue_idx);
typedef void (*hcc_flowctl_stop_subq)(uint16_t us_queue_idx);
void hcc_flowctl_operate_subq_register(hcc_flowctl_start_subq start_subq, hcc_flowctl_stop_subq stop_subq);
void hcc_host_set_flowctl_param(uint8_t uc_queue_type, uint16_t us_burst_limit,
                                uint16_t us_low_waterline, uint16_t us_high_waterline);
void hcc_host_get_flowctl_stat(void);
int32_t hcc_print_current_trans_info(uint32_t print_device_info);

typedef void (*flowctrl_cb)(void);

typedef struct _hcc_tx_flow_ctrl_info_ {
    uint32_t flowctrl_flag;
    uint32_t flowctrl_on_count;
    uint32_t flowctrl_off_count;
    uint32_t flowctrl_reset_count;
    uint32_t flowctrl_hipri_update_count;
    uint8_t uc_hipriority_cnt;
    uint8_t auc_resv[3];
    oal_spin_lock_stru st_hipri_lock; /* 读写uc_hipriority_cnt时要加锁 */
    oal_wait_queue_head_stru wait_queue;
    flowctrl_cb net_stopall;
    flowctrl_cb net_startall;
    hcc_flowctl_get_mode get_mode;
    hcc_flowctl_stop_subq stop_subq;
    hcc_flowctl_start_subq start_subq;
    oal_timer_list_stru flow_timer;
    unsigned long timeout;
    oal_delayed_work worker;
    oal_spin_lock_stru lock;
} hcc_tx_flow_ctrl_info;

typedef struct _hcc_thread_stat_ {
    uint64_t wait_event_block_count;
    uint64_t wait_event_run_count;
    uint64_t loop_have_data_count;
    uint64_t loop_no_data_count; /* 空转 */
} hcc_thread_stat;

struct hcc_transfer_handler {
    struct task_struct *hcc_transfer_thread;
    oal_wait_queue_head_stru hcc_transfer_wq;
#ifdef _PRE_CONFIG_WLAN_THRANS_THREAD_DEBUG
    hcc_thread_stat thread_stat;
#endif
    hcc_trans_queues hcc_queues[HCC_DIR_COUNT];
    hcc_tx_assem_info tx_assem_info;
    hcc_rx_assem_info rx_assem_info;
    hcc_tx_flow_ctrl_info tx_flow_ctrl;
    hcc_rx_action_info rx_action_info;
};

typedef int32_t (*hcc_rx_cb)(oal_netbuf_stru *netbuf,
                             struct hcc_header *hdr,
                             void *data);

struct hcc_tx_assem_descr {
    uint8_t descr_num;
    oal_netbuf_head_stru tx_assem_descr_hdr;
};
typedef void (*hcc_sched_transfer_func)(void);
typedef int32_t (*hmac_tcp_ack_process_func)(void);
typedef oal_bool_enum_uint8 (*hmac_tcp_ack_need_schedule_func)(void);

#define HCC_OFF       0
#define HCC_ON        1
#define HCC_EXCEPTION 2

struct hcc_handler {
    oal_atomic state; /* hcc's state */
    oal_atomic tx_seq;
    hcc_bus_dev *bus_dev;
    oal_uint hdr_rever_max_len;

    oal_mutex_stru tx_transfer_lock;
    oal_wakelock_stru tx_wake_lock;

    struct hcc_transfer_handler hcc_transer_info;

    /* the tx descr info, first descr */
    struct hcc_tx_assem_descr tx_descr_info;
    hmac_tcp_ack_process_func p_hmac_tcp_ack_process_func;
    hmac_tcp_ack_need_schedule_func p_hmac_tcp_ack_need_schedule_func;
};

typedef void (*hcc_tx_cb_callback)(struct hcc_handler *hcc);

#define HCC_TX_WAKELOCK_MAGIC 0xdead5ead
struct hcc_tx_cb_stru {
    hcc_tx_cb_callback destory;
    uint32_t qtype;
    uint32_t magic;
};

#define HCC_TC_TX_PKT_FLAG 0x5A
#define HCC_TC_RX_PKT_FLAG 0xA5
#define HCC_TC_TX_TCM_FLAG 0xEF
#define HCC_TC_RX_TCM_FLAG 0xFE

typedef hcc_bus_msg_rx hcc_msg_rx;

int32_t hcc_tx(struct hcc_handler *hcc, oal_netbuf_stru *netbuf, struct hcc_transfer_param *param);
#ifdef _PRE_WLAN_TCP_OPT
int32_t hcc_transfer_thread(void *data);
#endif

struct hcc_handler *hcc_module_init(hcc_bus_dev *pst_bus_dev);

void hcc_tx_assem_info_reset(struct hcc_handler *hcc);
void oal_sdio_rx_assem_info_reset(struct hcc_handler *hcc);

void hcc_module_exit(struct hcc_handler *);

int32_t hcc_message_register(struct hcc_handler *hcc, uint8_t msg, hcc_msg_rx cb, void *data);
void hcc_message_unregister(struct hcc_handler *hcc, uint8_t msg);
void hcc_clear_all_queues(struct hcc_handler *hcc, int32_t is_need_lock);
void hcc_enable(struct hcc_handler *hcc, int32_t is_need_lock);
void hcc_disable(struct hcc_handler *hcc, int32_t is_need_lock);

/* 获取默认的HCC通道句柄 */
extern struct hcc_handler *hcc_get_110x_handler(void);
extern void hcc_dev_flowctrl_on(struct hcc_handler *hcc, uint8_t need_notify_dev);
extern void hcc_dev_flowctrl_off(struct hcc_handler *hcc);
extern int32_t hcc_set_all_loglevel(int32_t loglevel);
extern uint32_t hcc_get_max_buf_len(void);
extern uint32_t hcc_get_max_trans_size(struct hcc_handler *hcc);
extern int32_t hcc_transfer_rx_register(struct hcc_handler *hcc, void *data, hcc_bus_data_rx rx);
extern void hcc_trans_flow_ctrl_info_reset(struct hcc_handler *hcc);

typedef uint32_t (*custom_cali_func)(void);
struct custom_process_func_handler {
    custom_cali_func p_custom_cali_func;
};
extern struct custom_process_func_handler *oal_get_custom_process_func(void);


OAL_STATIC OAL_INLINE void hcc_tx_transfer_lock(struct hcc_handler *hcc)
{
    if (oal_unlikely(hcc == NULL)) {
        oal_io_print("%s,hcc is null\n", __FUNCTION__);
        return;
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mutex_lock(&hcc->tx_transfer_lock);
#endif
}

OAL_STATIC OAL_INLINE void hcc_tx_transfer_unlock(struct hcc_handler *hcc)
{
    if (oal_unlikely(hcc == NULL)) {
        oal_io_print("%s,hcc is null\n", __FUNCTION__);
        return;
    }
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mutex_unlock(&hcc->tx_transfer_lock);
#endif
}

OAL_STATIC OAL_INLINE void hcc_rx_transfer_lock(struct hcc_handler *hcc)
{
    if (oal_unlikely(hcc == NULL)) {
        oal_io_print("%s,hcc is null\n", __FUNCTION__);
        return;
    }
    hcc_bus_rx_transfer_lock(hcc_to_bus(hcc));
}

OAL_STATIC OAL_INLINE void hcc_rx_transfer_unlock(struct hcc_handler *hcc)
{
    if (oal_unlikely(hcc == NULL)) {
        oal_io_print("%s,hcc is null\n", __FUNCTION__);
        return;
    }
    hcc_bus_rx_transfer_unlock(hcc_to_bus(hcc));
}

OAL_STATIC OAL_INLINE void hcc_transfer_lock(struct hcc_handler *hcc)
{
    if (oal_unlikely(hcc == NULL)) {
        oal_io_print("%s,hcc is null\n", __FUNCTION__);
        return;
    }
    hcc_rx_transfer_lock(hcc);
    hcc_tx_transfer_lock(hcc);
}

OAL_STATIC OAL_INLINE void hcc_transfer_unlock(struct hcc_handler *hcc)
{
    if (oal_unlikely(hcc == NULL)) {
        oal_io_print("%s,hcc is null\n", __FUNCTION__);
        return;
    }
    hcc_tx_transfer_unlock(hcc);
    hcc_rx_transfer_unlock(hcc);
}

#if (_PRE_TEST_MODE != _PRE_TEST_MODE_ST)
OAL_STATIC OAL_INLINE void hcc_sched_transfer(struct hcc_handler *hcc)
{
    if (oal_warn_on(hcc == NULL)) {
        oal_io_print("%s,hcc is null\n", __FUNCTION__);
        return;
    }
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_wait_queue_wake_up_interrupt(&hcc->hcc_transer_info.hcc_transfer_wq);
#endif
}
#endif

/* 统计一次传输的最大报文个数，每次传输报文个数越多，调度消耗占比越低
 * info[0]表示聚合个数超过了最大统计大小 */
OAL_STATIC OAL_INLINE void hcc_tx_assem_count(struct hcc_handler *hcc, uint32_t assem_count)
{
    if (oal_unlikely(hcc == NULL)) {
        oal_io_print("%s,hcc is null\n", __FUNCTION__);
        return;
    }
    if (oal_unlikely(assem_count == 0)) {
        return;
    }
    if (oal_likely(assem_count < oal_array_size(hcc->hcc_transer_info.tx_assem_info.info))) {
        hcc->hcc_transer_info.tx_assem_info.info[assem_count]++;
    } else {
        hcc->hcc_transer_info.tx_assem_info.info[0]++;
    }
}

OAL_STATIC OAL_INLINE void hcc_rx_assem_count(struct hcc_handler *hcc, uint32_t assem_count)
{
    if (oal_unlikely(hcc == NULL)) {
        oal_io_print("%s,hcc is null\n", __FUNCTION__);
        return;
    }
    if (oal_unlikely(assem_count == 0)) {
        return;
    }
    if (oal_likely(assem_count < oal_array_size(hcc->hcc_transer_info.rx_assem_info.info))) {
        hcc->hcc_transer_info.rx_assem_info.info[assem_count]++;
    } else {
        hcc->hcc_transer_info.rx_assem_info.info[0]++;
    }
}

extern void hcc_set_tcpack_cnt(uint32_t ul_val);
extern int32_t hcc_rx_register(struct hcc_handler *hcc, uint8_t mtype,
                               hcc_rx_post_do post_do, hcc_rx_pre_do pre_do);
extern int32_t hcc_rx_unregister(struct hcc_handler *hcc, uint8_t mtype);
extern void hcc_tx_wlan_queue_map_set(struct hcc_handler *hcc, hcc_queue_type hcc_queue_id,
                                      wlan_net_queue_type wlan_queue_id);

extern void hcc_rx_submit(struct hcc_handler *hcc, oal_netbuf_stru *pst_netbuf);
extern void hcc_restore_assemble_netbuf_list(struct hcc_handler *hcc);
extern void hcc_restore_tx_netbuf(struct hcc_handler *hcc, oal_netbuf_stru *pst_netbuf);
extern void hcc_change_state_exception(void);

OAL_STATIC OAL_INLINE void hi_wlan_power_set(int32_t on)
{
    /*
     * this should be done in mpw1
     * it depends on the gpio used to power up and down 1101 chip
     *
     */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (on) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "hcc probe:pull up power on gpio");
        board_host_wakeup_dev_set(0);
        board_power_on(WLAN_POWER);
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "hcc probe:pull down power on gpio");
        board_power_off(WLAN_POWER);
        board_host_wakeup_dev_set(0);
    }
#endif
}

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
extern int32_t hcc_test_init_module(struct hcc_handler *hcc);
extern void hcc_test_exit_module(struct hcc_handler *hcc);
#else
/* function stub */
OAL_STATIC OAL_INLINE int32_t hcc_test_init_module(struct hcc_handler *hcc)
{
    oal_reference(hcc);
    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE void hcc_test_exit_module(struct hcc_handler *hcc)
{
    oal_reference(hcc);
    return;
}
#endif

extern void hcc_print_device_mem_info(void);
extern void hcc_trigger_device_panic(void);

extern struct custom_process_func_handler g_custom_process_func;

#define HCC_NETBUF_RESERVED_ROOM_SIZE (HCC_HDR_TOTAL_LEN + HISDIO_H2D_SCATT_BUFFLEN_ALIGN)
/*
 * 函 数 名  : hcc_netbuf_alloc
 * 功能描述  : reserved the fixed headroom and tailroom for hcc transfer!
 */
OAL_STATIC OAL_INLINE oal_netbuf_stru *hcc_netbuf_alloc(uint32_t ul_size)
{
    oal_netbuf_stru *pst_netbuf = NULL;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    gfp_t flags;
    if (in_interrupt() || irqs_disabled() || in_atomic() || rcu_preempt_depth()) {
        flags = GFP_ATOMIC;
    } else {
        flags = GFP_KERNEL;
    }

    pst_netbuf = __netdev_alloc_skb(NULL, ul_size + HCC_NETBUF_RESERVED_ROOM_SIZE, flags);
    if (oal_unlikely(pst_netbuf != NULL)) {
        skb_reserve(pst_netbuf, HCC_NETBUF_RESERVED_ROOM_SIZE);
    }
#else
    pst_netbuf = oal_netbuf_alloc(ul_size + HCC_NETBUF_RESERVED_ROOM_SIZE, HCC_NETBUF_RESERVED_ROOM_SIZE, 0);
#endif
    return pst_netbuf;
}

OAL_STATIC OAL_INLINE void hcc_tx_netbuf_free(oal_netbuf_stru *pst_netbuf)
{
    struct hcc_tx_cb_stru *pst_cb_stru = NULL;

    if (oal_warn_on(pst_netbuf == NULL)) {
        oal_io_print("[Error] pst_netbuf is null r failed!%s\n", __FUNCTION__);
        return;
    }

    pst_cb_stru = (struct hcc_tx_cb_stru *)oal_netbuf_cb(pst_netbuf);
    if (oal_unlikely(pst_cb_stru->magic != HCC_TX_WAKELOCK_MAGIC)) {
#ifdef CONFIG_PRINTK
        printk(KERN_EMERG "BUG: tx netbuf:%p on CPU#%d,magic:%08x should be %08x\n", pst_cb_stru,
               raw_smp_processor_id(), pst_cb_stru->magic, HCC_TX_WAKELOCK_MAGIC);
        print_hex_dump(KERN_ERR, "tx_netbuf_magic", DUMP_PREFIX_ADDRESS, 16, 1, /* rowsize is 16 */
                       (uint8_t *)pst_netbuf, sizeof(oal_netbuf_stru), true); /* 内核函数固定的传参 */
        printk(KERN_ERR "\n");
#endif
        oal_warn_on(1);
        declare_dft_trace_key_info("tx_wakelock_crash", OAL_DFT_TRACE_EXCEP);
        return;
    }

    if (oal_likely(pst_cb_stru->destory)) {
        pst_cb_stru->destory(hcc_get_110x_handler());
    }

    pst_cb_stru->magic = 0x0;

    oal_netbuf_free(pst_netbuf);
}

OAL_STATIC OAL_INLINE void hcc_tx_netbuf_list_free(oal_netbuf_head_stru *pst_netbuf_hdr)
{
    oal_netbuf_stru *pst_netbuf = NULL;
    if (oal_unlikely(pst_netbuf_hdr == NULL)) {
        oal_warn_on(1);
        return;
    }
    for (;;) {
        pst_netbuf = oal_netbuf_delist(pst_netbuf_hdr);
        if (pst_netbuf == NULL) {
            break;
        }
        hcc_tx_netbuf_free(pst_netbuf);
    }
}

#endif /* end of oal_hcc_if.h */
