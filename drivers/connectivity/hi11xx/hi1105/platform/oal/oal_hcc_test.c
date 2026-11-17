

#define HISI_LOG_TAG "[HCC_TEST]"
#include "oal_hcc_host_if.h"
#include "oal_ext_if.h"
#include "plat_pm_wlan.h"
#include "plat_pm.h"
#include "securec.h"
#include "hisi_ini.h"
#include "plat_cali.h"
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#include "pcie_host.h"
#include "pcie_linux.h"
#endif
#if defined(_PRE_CONFIG_GPIO_TO_SSI_DEBUG)
#include "board.h"
#endif
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
#include "oal_kernel_file.h"
#include "oal_dft.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/host.h>

typedef struct _hcc_test_data_ {
    int32_t mode_idx;
    int32_t pkt_rcvd;      /* packet received */
    int32_t pkt_sent;      /* packet sent */
    int32_t pkt_gen;       /* packet generate */
    int32_t pkt_len;       /* packet  length */
    unsigned long trans_timeout; /* msec */
    uint64_t total_rcvd_bytes;
    uint64_t total_sent_bytes;
    uint64_t trans_time;
    uint64_t trans_time_us;
    uint64_t throughput;
    hsdio_trans_test_info trans_info;
} hcc_test_data;

struct hcc_test_event {
    oal_mutex_stru mutex_lock; /* sdio test task lock */
    int32_t errorno;

    oal_workqueue_stru *test_workqueue;
    oal_work_stru test_work;
    hcc_test_data test_data;

    /* hcc perf started,for data transfer */
    int32_t started;

    int32_t rx_stop;

    /* hcc perf statistic */
    ktime_t start_time;
    /* The last update time */
    ktime_t last_time;

    /* To hcc test sync */
    oal_completion test_done;
    oal_completion test_trans_done;

    hcc_queue_type hcc_test_queue;

    uint8_t pad_payload;

    uint8_t test_value;
    uint8_t verified;

    unsigned long start_tick;
    unsigned long tick_timeout;

    /* sdio test thread and seam */
};

struct hcc_test_stru {
    const char *mode;
    uint16_t start_cmd;
    const char *mode_desc;
};

/* 全局变量定义 */
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
OAL_STATIC oal_completion g_pcie_test_trans_done;
OAL_STATIC hcc_pcie_test_request_stru g_pcie_test_request_ack;
#endif
OAL_STATIC int32_t g_test_force_stop = 0;
OAL_STATIC struct hcc_test_event *g_hcc_test_event = NULL;

OAL_STATIC struct hcc_test_stru g_hcc_test_stru[HCC_TEST_CASE_COUNT] = {
    [HCC_TEST_CASE_TX] = { "tx",   HCC_TEST_CMD_START_TX,   "HCC_TX_MODE" },
    [HCC_TEST_CASE_RX] = { "rx",   HCC_TEST_CMD_START_RX,   "HCC_RX_MODE" },
    [HCC_TEST_CASE_LOOP] = { "loop", HCC_TEST_CMD_START_LOOP, "HCC_LOOP_MODE" },
};

#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
OAL_STATIC uint32_t g_wifi_panic_debug = 0;
oal_debug_module_param(g_wifi_panic_debug, int, S_IRUGO | S_IWUSR);
OAL_STATIC LIST_HEAD(g_wifi_panic_log_head);
#endif

OAL_STATIC int32_t g_ft_pcie_test_wifi_runtime = 5000; /* 5 seconds */
OAL_STATIC int32_t g_ft_sdio_test_wifi_runtime = 5000; /* 5 seconds */
OAL_STATIC int32_t g_ft_pcie_test_min_throught = 1613; /* 1600Mbps */
OAL_STATIC int32_t g_ft_pcie_test_retry_cnt = 3;       /* retry 3 times */
OAL_STATIC int32_t g_ft_ip_test_cpu_max_freq = 1;      /* retry 3 times */
OAL_STATIC int32_t g_hcc_test_performance_mode = 0;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_debug_module_param(g_ft_pcie_test_wifi_runtime, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(g_ft_sdio_test_wifi_runtime, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(g_ft_pcie_test_min_throught, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(g_ft_pcie_test_retry_cnt, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(g_ft_ip_test_cpu_max_freq, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(g_hcc_test_performance_mode, int, S_IRUGO | S_IWUSR);
#endif
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
OAL_STATIC int32_t g_ft_pcie_loopback_rw_test_count = 500;
OAL_STATIC int32_t g_ft_pcie_loopback_linkup_timeout = 600000; /* us */
OAL_STATIC int32_t g_ft_pcie_loopback_bias_cldo3 = 900;        /* 0.9v */
OAL_STATIC int32_t g_ft_pcie_loopback_bias_vptx = 900;         /* 0.9v */
OAL_STATIC int32_t g_ft_pcie_loopback_bias_vph = 1800;         /* 1.8v */
module_param(g_ft_pcie_loopback_rw_test_count, int, S_IRUGO | S_IWUSR);
module_param(g_ft_pcie_loopback_linkup_timeout, int, S_IRUGO | S_IWUSR);
module_param(g_ft_pcie_loopback_bias_cldo3, int, S_IRUGO | S_IWUSR);
module_param(g_ft_pcie_loopback_bias_vptx, int, S_IRUGO | S_IWUSR);
module_param(g_ft_pcie_loopback_bias_vph, int, S_IRUGO | S_IWUSR);
#endif

int32_t conn_test_hcc_chann_switch(const char *new_dev);
#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
OAL_STATIC int32_t hcc_assem_info_print(void *data, char *buf, int32_t buf_len);
OAL_STATIC int32_t hcc_flowctrl_info_print(void *data, char *buf, int32_t buf_len);
OAL_STATIC int32_t hcc_queues_len_info_print(void *data, char *buf, int32_t buf_len);
OAL_STATIC int32_t hcc_queues_pkts_info_print(void *data, char *buf, int32_t buf_len);
OAL_STATIC int32_t hsdio_sysfs_info_print(void *data, char *buf, int32_t buf_len);
OAL_STATIC declare_wifi_panic_stru(hcc_panic_assem_info, hcc_assem_info_print);
OAL_STATIC declare_wifi_panic_stru(hcc_panic_flowctrl, hcc_flowctrl_info_print);
OAL_STATIC declare_wifi_panic_stru(hcc_panic_queues_len, hcc_queues_len_info_print);
OAL_STATIC declare_wifi_panic_stru(hcc_panic_queues_pkts, hcc_queues_pkts_info_print);
OAL_STATIC declare_wifi_panic_stru(sdio_panic, hsdio_sysfs_info_print);
#endif

OAL_STATIC oal_kobject *g_conn_syfs_hcc_object = NULL;
OAL_STATIC int32_t hcc_test_normal_start(uint16_t test_type);
OAL_STATIC int32_t hcc_send_test_cmd(uint8_t *cmd, int32_t hdr_len, int32_t cmd_len);
OAL_STATIC int32_t hcc_test_start(uint16_t start_cmd);
OAL_STATIC int hcc_test_set_case(hcc_test_data *data);

/* 函数声明 */
#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
void hwifi_panic_log_register(hwifi_panic_log *pst_log, void *data)
{
    if (oal_unlikely(!pst_log)) {
        oal_io_print("%s error: pst_log is null", __FUNCTION__);
        return;
    };

    pst_log->data = data;
    list_add_tail(&pst_log->list, &g_wifi_panic_log_head);
    oal_io_print("Log module %s added![%pF]\n", pst_log->name, (void *)_RET_IP_);
}
oal_module_symbol(hwifi_panic_log_register);

/* should't use dynamic mem when panic */
OAL_STATIC char g_panic_mem[PAGE_SIZE];
void hwifi_panic_log_dump(char *print_level)
{
    uint32_t buf_size = PAGE_SIZE;
    char *pst_buf = NULL;
    hwifi_panic_log *pst_log = NULL;
    struct list_head *head = &g_wifi_panic_log_head;

    if (print_level == NULL) {
        print_level = "";
    }

    pst_buf = g_panic_mem;

    printk("%sdump wifi info when panic\n", print_level);
    list_for_each_entry(pst_log, head, list)
    {
        if (pst_log == NULL) {
            printk(KERN_ERR "hwifi_panic_log_dump:pst_log is null\n");
            return;
        }
        pst_buf[0] = '\0';
        printk("%s[%s]:\n", print_level, pst_log->name);
        pst_log->cb(pst_log->data, pst_buf, buf_size);
        printk("%s%s\n", print_level, pst_buf);
        printk("%s\n", print_level);
    }
    printk("%s\n", print_level);
}
oal_module_symbol(hwifi_panic_log_dump);
#endif

OAL_STATIC int32_t hcc_flowctrl_info_print_ext(char *buf, int32_t buf_len)
{
    struct oal_sdio *hi_sdio = NULL;
    int32_t count = 0;
#ifdef CONFIG_MMC
    int32_t ret = 0;
#endif

#ifdef CONFIG_MMC
    hi_sdio = oal_get_sdio_default_handler();
    if ((hi_sdio != NULL) && (hi_sdio->func->card->host->claimer != NULL)) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0))
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "claim host name:%s\n",
                         hi_sdio->func->card->host->claimer->comm);
#else
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "claim host name:%s\n",
                         hi_sdio->func->card->host->claimer->task->comm);
#endif
    }
    if (ret < 0) {
        return ret;
    }

    count += ret;
#else
    oal_reference(hi_sdio);
#endif

    return count;
}

OAL_STATIC int32_t hcc_flowctrl_info_print(void *data, char *buf, int32_t buf_len)
{
    int32_t ret;
    int32_t count = 0;
    struct hcc_handler *hcc = (struct hcc_handler *)data;
    if (hcc == NULL) {
        return 0;
    }

    ret = hcc_flowctrl_info_print_ext(buf, buf_len);
    if (ret < 0) {
        return count;
    } else {
        count = ret;
    }

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "hcc state:%s[%d]\n",
                     (oal_atomic_read(&hcc->state) == HCC_OFF) ? "off" : "on",
                     oal_atomic_read(&hcc->state));
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "flowctrl flag:%s\n",
                     (D2H_MSG_FLOWCTRL_OFF == hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_flag) ?
                     "off" : "on");
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "flowctrl on:%d\n",
                     hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_on_count);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "flowctrl off:%d\n",
                     hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_off_count);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "flowctrl reset count:%d\n",
                     hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_reset_count);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "flowctrl hi update count:%d,hi credit value:%u\n",
                     hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_hipri_update_count,
                     hcc->hcc_transer_info.tx_flow_ctrl.uc_hipriority_cnt);
    if (ret < 0) {
        return count;
    }
    count += ret;
    return count;
}

OAL_STATIC int32_t hcc_assem_info_print(void *data, char *buf, int32_t buf_len)
{
    int32_t ret;
    int32_t i;
    int32_t count = 0;
    struct hcc_handler *hcc = (struct hcc_handler *)data;
    if (hcc == NULL) {
        return 0;
    }
    for (i = 0; i < HCC_TX_ASSEM_INFO_MAX_NUM; i++) {
        if (hcc->hcc_transer_info.tx_assem_info.info[i]) {
            ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "[tx][%2d]:%-20u pkts\n",
                             i, hcc->hcc_transer_info.tx_assem_info.info[i]);
            if (ret < 0) {
                return count;
            }
            count += ret;
        }
    }
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "hcc tx total:%llu\n",
                     hcc->hcc_transer_info.tx_assem_info.pkt_cnt);
    if (ret < 0) {
        return count;
    }
    count += ret;

    for (i = 0; i < HCC_RX_ASSEM_INFO_MAX_NUM; i++) {
        if (hcc->hcc_transer_info.rx_assem_info.info[i]) {
            ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "[rx][%2d]:%-20u pkts\n",
                             i, hcc->hcc_transer_info.rx_assem_info.info[i]);
            if (ret < 0) {
                return count;
            }
            count += ret;
        }
    }

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "hcc rx total:%llu\n",
                     hcc->hcc_transer_info.rx_assem_info.pkt_cnt);
    if (ret < 0) {
        return count;
    }
    count += ret;
    return count;
}

static int32_t hcc_queues_info_print_total_pkts(struct hcc_handler *hcc, char *buf, int32_t buf_len)
{
    int i, j;
    int32_t ret;
    uint64_t total;
    int32_t count = 0;
    hcc_trans_queue *pst_hcc_queue = NULL;
    for (i = 0; i < HCC_DIR_COUNT; i++) {
        total = 0;
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                         "transfer dir:%s\n", hcc_get_chan_string(i));
        if (ret < 0) {
            return count;
        }
        count += ret;

        for (j = 0; j < HCC_QUEUE_COUNT; j++) {
            pst_hcc_queue = &hcc->hcc_transer_info.hcc_queues[i].queues[j];
            if (pst_hcc_queue->total_pkts || pst_hcc_queue->loss_pkts) {
                ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                                 "queue:%4d,pkts num:%10u,loss num:%10u\n", j,
                                 pst_hcc_queue->total_pkts,
                                 pst_hcc_queue->loss_pkts);
                if (ret < 0) {
                    return count;
                }
                count += ret;
            }
            total += pst_hcc_queue->total_pkts;
        }
        if (total) {
            ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "total:%llu\n", total);
            if (ret < 0) {
                return count;
            }
            count += ret;
        }
    }
    return count;
}

static int32_t hcc_queues_info_print_flowctrl(struct hcc_handler *hcc, char *buf, int32_t buf_len)
{
    int32_t count = 0;
    int j;
    int32_t ret;
    hcc_trans_queue *pst_hcc_queue = NULL;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "flow ctrl info show\n");
    if (ret < 0) {
        return count;
    }
    count += ret;

    for (j = 0; j < HCC_QUEUE_COUNT; j++) {
        pst_hcc_queue = &hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[j];
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "tx queue:%4d,%s,low wl:%u, high wl:%u\n",
                         j,
                         (pst_hcc_queue->flow_ctrl.is_stopped == OAL_FALSE) ? "run " : "stop",
                         pst_hcc_queue->flow_ctrl.low_waterline,
                         pst_hcc_queue->flow_ctrl.high_waterline);
        if (ret < 0) {
            return count;
        }
        count += ret;
    }

    pst_hcc_queue = &hcc->hcc_transer_info.hcc_queues[HCC_RX].queues[DATA_LO_QUEUE];
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "rx queue:%4d,low wl:%u,high wl:%u, enabled:%d\n",
                     DATA_LO_QUEUE,
                     pst_hcc_queue->flow_ctrl.low_waterline,
                     pst_hcc_queue->flow_ctrl.high_waterline, pst_hcc_queue->flow_ctrl.enable);
    if (ret < 0) {
        return count;
    }
    count += ret;

    return count;
}

static int32_t hcc_queues_info_print_thread_info(struct hcc_handler *hcc, char *buf, int32_t buf_len)
{
    int32_t count = 0;
#ifdef _PRE_CONFIG_WLAN_THRANS_THREAD_DEBUG
    int32_t ret;
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "hcc thread stat info:\n");
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "condition true:%llu,false:%llu\n",
                     hcc->hcc_transer_info.thread_stat.wait_event_run_count,
                     hcc->hcc_transer_info.thread_stat.wait_event_block_count);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "thread loop do:%llu,empty:%llu\n",
                     hcc->hcc_transer_info.thread_stat.loop_have_data_count,
                     hcc->hcc_transer_info.thread_stat.loop_no_data_count);
    if (ret < 0) {
        return count;
    }
    count += ret;
#endif
    return count;
}


OAL_STATIC int32_t hcc_queues_pkts_info_print(void *data, char *buf, int32_t buf_len)
{
    int32_t ret;
    int32_t count = 0;
    struct hcc_handler *hcc = (struct hcc_handler *)data;
    if (hcc == NULL) {
        return 0;
    }
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "queues_pkts_info_show [tx_seq:%d]\n",
                     atomic_read(&hcc->tx_seq));
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = hcc_queues_info_print_total_pkts(hcc, buf + count, buf_len - count);
    count += ret;
    if (buf_len <= count) {
        return count;
    }

    ret = hcc_queues_info_print_flowctrl(hcc, buf + count, buf_len - count);
    count += ret;
    if (buf_len <= count) {
        return count;
    }

    ret = hcc_queues_info_print_thread_info(hcc, buf + count, buf_len - count);
    count += ret;
    if (buf_len <= count) {
        return count;
    }

    return count;
}

OAL_STATIC int32_t hcc_queues_len_info_print(void *data, char *buf, int32_t buf_len)
{
    int32_t ret;
    int32_t count = 0;
    int32_t i, j;
    struct hcc_handler *hcc = (struct hcc_handler *)data;
    if (hcc == NULL) {
        return 0;
    }
    for (i = 0; i < HCC_DIR_COUNT; i++) {
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "dir:%s\n", hcc_get_chan_string(i));
        if (ret < 0) {
            return count;
        }
        count += ret;

        for (j = 0; j < HCC_QUEUE_COUNT; j++) {
            if (oal_netbuf_list_len(&hcc->hcc_transer_info.hcc_queues[i].queues[j].data_queue)) {
                ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "queue:%d, len:%d\n", j,
                                 oal_netbuf_list_len(&hcc->hcc_transer_info.hcc_queues[i].queues[j].data_queue));
                if (ret < 0) {
                    return count;
                }
                count += ret;
            }
        }
    }
    return count;
}

int32_t hcc_print_current_trans_info(uint32_t print_device_info)
{
    int ret;
    char *buf = NULL;
    uint32_t buf_size = PAGE_SIZE;
    hcc_bus *pst_bus = NULL;
    struct hcc_handler *hcc;
    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        oal_io_print("get hcc handler failed!%s\n", __FUNCTION__);
        return -OAL_EFAIL;
    }

    pst_bus = hcc_get_current_110x_bus();

    buf = oal_memalloc(buf_size);
    if (buf != NULL) {
        ret = 0;
        memset_s(buf, buf_size, 0, buf_size);

        if ((pst_bus != NULL) && (pst_bus->bus_type == HCC_BUS_SDIO)) {
            ret += hcc_flowctrl_info_print(hcc, buf, buf_size - ret);
        }

        ret += hcc_queues_len_info_print(hcc, buf, buf_size - ret);

        ret += hcc_queues_pkts_info_print(hcc, buf + ret, buf_size - ret);

        oal_io_print("%s\n", buf);

        oal_free(buf);
    } else {
        oal_io_print("alloc buf size %u failed\n", buf_size);
    }

    if (pst_bus != NULL) {
        /* 打印device信息要保证打印过程中 不会进入深睡 */
        hcc_bus_print_trans_info(pst_bus, print_device_info ?
                                 (HCC_PRINT_TRANS_FLAG_DEVICE_STAT | HCC_PRINT_TRANS_FLAG_DEVICE_REGS) : 0x0);
    }

    return OAL_SUCC;
}

OAL_STATIC ssize_t hcc_get_assem_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    struct hcc_handler *hcc = NULL;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        oal_io_print("get hcc handler failed!%s\n", __FUNCTION__);
        return ret;
    }

    ret += hcc_assem_info_print(hcc, buf, PAGE_SIZE - ret);

    return ret;
}

OAL_STATIC ssize_t hcc_set_assem_info(struct kobject *dev, struct kobj_attribute *attr, const char *buf, size_t count)
{
    struct hcc_handler *hcc = NULL;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        oal_io_print("get hcc handler failed!%s\n", __FUNCTION__);
        return count;
    }
    hcc_transfer_lock(hcc);
    hcc_tx_assem_info_reset(hcc);
    oal_sdio_rx_assem_info_reset(hcc);
    hcc_transfer_unlock(hcc);

    oal_io_print("reset done!\n");

    return count;
}

OAL_STATIC ssize_t hcc_get_queues_pkts_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    struct hcc_handler *hcc = NULL;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        oal_io_print("get hcc handler failed!%s\n", __FUNCTION__);
        return ret;
    }

    ret += hcc_queues_pkts_info_print(hcc, buf, PAGE_SIZE - ret);

    return ret;
}

OAL_STATIC ssize_t hcc_get_queues_len_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    struct hcc_handler *hcc = NULL;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return ret;
    }
    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        oal_io_print("get hcc handler failed!%s\n", __FUNCTION__);
        return ret;
    }

    ret += hcc_queues_len_info_print(hcc, buf, PAGE_SIZE - ret);

    return ret;
}

OAL_STATIC ssize_t hcc_get_flowctrl_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    struct hcc_handler *hcc = NULL;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        oal_io_print("get hcc handler failed!%s\n", __FUNCTION__);
        return ret;
    }

    ret += hcc_flowctrl_info_print(hcc, buf, PAGE_SIZE - ret);

    return ret;
}

OAL_STATIC int32_t hcc_wakelock_info_print(struct hcc_handler *hcc, char *buf, int32_t buf_len)
{
    int32_t ret;
    int32_t count = 0;

    if (hcc == NULL) {
        return 0;
    }
#ifdef CONFIG_PRINTK
    if (hcc->tx_wake_lock.locked_addr) {
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "wakelocked by:%pf\n",
                         (void *)hcc->tx_wake_lock.locked_addr);
        if (ret < 0) {
            return count;
        }
        count += ret;
    }
#endif

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "hold %lu locks\n",
                     hcc->tx_wake_lock.lock_count);
    if (ret < 0) {
        return count;
    }
    count += ret;

    return count;
}

OAL_STATIC ssize_t hcc_get_wakelock_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    struct hcc_handler *hcc = NULL;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        oal_io_print("get hcc handler failed!%s\n", __FUNCTION__);
        return ret;
    }

    ret += hcc_wakelock_info_print(hcc, buf, PAGE_SIZE - ret);

    return ret;
}

OAL_STATIC ssize_t hcc_get_allwakelock_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    ret += oal_print_all_wakelock_buff(buf, PAGE_SIZE);

    return ret;
}

OAL_STATIC ssize_t hcc_set_allwakelock_info(struct kobject *dev, struct kobj_attribute *attr,
                                            const char *buf, size_t count)
{
    uint32_t level;
    char lockname[200]; /* 200字节足够存放锁名 */

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((count == 0) || (count >= sizeof(lockname)) || (buf[count] != '\0')) {
        oal_io_print("input illegal!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    memset_s(lockname, sizeof(lockname), 0, sizeof(lockname));

    if ((sscanf_s(buf, "%s %u", lockname, sizeof(lockname), &level) < 2)) { // 2 args
        oal_io_print("error input,must input more than 2 arguments!\n");
        return count;
    }

    oal_set_wakelock_debuglevel(lockname, !!level);

    return count;
}

OAL_STATIC struct kobj_attribute g_dev_attr_flowctrl =
    __ATTR(flowctrl, S_IRUGO, hcc_get_flowctrl_info, NULL);
OAL_STATIC struct kobj_attribute g_dev_attr_assem_info =
    __ATTR(assem_info, S_IRUGO | S_IWUSR, hcc_get_assem_info, hcc_set_assem_info);
OAL_STATIC struct kobj_attribute g_dev_attr_queues_pkts =
    __ATTR(queues_pkts, S_IRUGO, hcc_get_queues_pkts_info, NULL);
OAL_STATIC struct kobj_attribute g_dev_attr_queues_len =
    __ATTR(queues_len, S_IRUGO, hcc_get_queues_len_info, NULL);
OAL_STATIC struct kobj_attribute g_dev_attr_wakelock =
    __ATTR(wakelock, S_IRUGO, hcc_get_wakelock_info, NULL);
OAL_STATIC struct kobj_attribute g_dev_attr_wakelockall =
    __ATTR(wakelockall, S_IRUGO | S_IWUSR, hcc_get_allwakelock_info, hcc_set_allwakelock_info);

/*
 * Prototype    : hcc_test_get_trans_time
 * Description  : get hcc test trans time
 * Return Value : succ or fail
 */
OAL_STATIC uint64_t hcc_test_get_trans_time(ktime_t start_time, ktime_t stop_time)
{
    ktime_t trans_time;
    uint64_t trans_us;

    trans_time = ktime_sub(stop_time, start_time);
#ifdef _PRE_COMMENT_CODE_
    oal_io_print("start time:%llu, stop time:%llu, trans_time:%llu\n",
                 ktime_to_us(start_time), ktime_to_us(stop_time), ktime_to_us(trans_time));
#endif
    trans_us = (uint64_t)ktime_to_us(trans_time);
    if (trans_us == 0) {
        trans_us = 1;
    }

    return trans_us;
}

OAL_STATIC void hcc_test_throughput_cac(uint64_t trans_bytes, ktime_t start_time, ktime_t stop_time)
{
    uint64_t trans_us;
    uint64_t temp;
    uint64_t us_to_s; /* converted  usecond to second */

    trans_us = hcc_test_get_trans_time(start_time, stop_time);

    temp = (trans_bytes);

    temp = temp * 1000u;

    temp = temp * 1000u;

    /* 17是推算出的，无实际意义 */
    temp = (temp >> 17);

    temp = div_u64(temp, trans_us);

    us_to_s = trans_us;
    g_hcc_test_event->test_data.trans_time_us = trans_us;
    do_div(us_to_s, 1000000u);
    g_hcc_test_event->test_data.throughput = temp;
    g_hcc_test_event->test_data.trans_time = us_to_s;
}

OAL_STATIC void hcc_test_throughput_gen(void)
{
    if (g_hcc_test_event->test_data.mode_idx == HCC_TEST_CASE_TX) {
        hcc_test_throughput_cac(g_hcc_test_event->test_data.total_sent_bytes,
                                g_hcc_test_event->start_time,
                                g_hcc_test_event->last_time);
    } else if (g_hcc_test_event->test_data.mode_idx == HCC_TEST_CASE_RX) {
        hcc_test_throughput_cac(g_hcc_test_event->test_data.total_rcvd_bytes,
                                g_hcc_test_event->start_time,
                                g_hcc_test_event->last_time);
    } else if (g_hcc_test_event->test_data.mode_idx == HCC_TEST_CASE_LOOP) {
        hcc_test_throughput_cac(g_hcc_test_event->test_data.total_rcvd_bytes +
                                g_hcc_test_event->test_data.total_sent_bytes,
                                g_hcc_test_event->start_time,
                                g_hcc_test_event->last_time);
    }
}

OAL_STATIC uint64_t hcc_test_utilization_ratio_gen(uint64_t payload_size, uint64_t transfer_size)
{
    uint64_t ret;
    payload_size = payload_size * 1000; /* 1000 计算占有率 */
    if (transfer_size) {
        ret = div_u64(payload_size, transfer_size);
    } else {
        ret = 0;
    }
    return ret;
}

/* 统计发送方向的丢包率，接收方向默认不丢包 */
OAL_STATIC uint32_t hcc_test_tx_pkt_loss_gen(uint32_t tx_pkts, uint32_t actual_tx_pkts)
{
    uint32_t ul_loss;

    if (tx_pkts == actual_tx_pkts || !tx_pkts) {
        return 0;
    }
    if (tx_pkts < actual_tx_pkts) {
        return 0;
    }

    ul_loss = tx_pkts - actual_tx_pkts;
    return ul_loss * 1000 / tx_pkts; /* 1000 计算占有率 */
}

OAL_STATIC void hcc_test_rcvd_data(oal_netbuf_stru *pst_netbuf)
{
    int32_t ret;
    int32_t filter_flag = 0;

    /* 计算总共数据包长度 */
    if (oal_unlikely(g_hcc_test_event->test_data.pkt_len != oal_netbuf_len(pst_netbuf))) {
        if (printk_ratelimit()) {
            oal_io_print("[E]recvd netbuf pkt len:%d,but request len:%d\n",
                         oal_netbuf_len(pst_netbuf),
                         g_hcc_test_event->test_data.pkt_len);
        }
        filter_flag = 1;
    }

    if (g_hcc_test_event->verified) {
        int32_t i;
        int32_t flag = 0;
        uint8_t *data = oal_netbuf_data(pst_netbuf);
        for (i = 0; i < oal_netbuf_len(pst_netbuf); i++) {
            if (*(data + i) != g_hcc_test_event->test_value) {
                flag = 1;
                oal_io_print("[E]data wrong, [i:%d] value:%x should be %x\n",
                             i, *(data + i), g_hcc_test_event->test_value);
                break;
            }
        }

        if (flag) {
            oal_print_hex_dump(data, oal_netbuf_len(pst_netbuf), HEX_DUMP_GROUP_SIZE, "hcc rx verified ");
            filter_flag = 1;
        }
    }

    if (!filter_flag) {
        /* filter_flag=1 时接收的数据包不符合要求，则过滤掉 */
        g_hcc_test_event->test_data.pkt_rcvd++;
        g_hcc_test_event->test_data.total_rcvd_bytes += oal_netbuf_len(pst_netbuf);
        g_hcc_test_event->last_time = ktime_get();
    }

    if (g_hcc_test_event->test_data.mode_idx == HCC_TEST_CASE_RX) {
        if (time_after(jiffies, (g_hcc_test_event->start_tick + g_hcc_test_event->tick_timeout))) {
            if (!g_hcc_test_event->rx_stop) {
                oal_io_print("RxTestTimeIsUp\n");
                ret = hcc_send_message(hcc_get_110x_handler(), H2D_MSG_STOP_SDIO_TEST);
                if (ret) {
                    oal_io_print("send message failed, ret=%d", ret);
                }
                g_hcc_test_event->rx_stop = 1;
            }
        }
    }
}

OAL_STATIC void hcc_test_rcvd_cmd(oal_netbuf_stru *pst_netbuf)
{
    int32_t ret;
    hcc_test_cmd_stru cmd;
    ret = memcpy_s(&cmd, sizeof(hcc_test_cmd_stru), oal_netbuf_data(pst_netbuf), sizeof(hcc_test_cmd_stru));
    if (ret != EOK) {
        oal_io_print("memcpy_s err!\n");
    }
    if (cmd.cmd_type == HCC_TEST_CMD_STOP_TEST) {
        ret = memcpy_s(&g_hcc_test_event->test_data.trans_info,
                       sizeof(hsdio_trans_test_info),
                       hcc_get_test_cmd_data(oal_netbuf_data(pst_netbuf)),
                       sizeof(hsdio_trans_test_info));
        if (ret != EOK) {
            oal_io_print("memcpy_s err!\n");
        }
        g_hcc_test_event->last_time = ktime_get();
        oal_io_print("hcc_test_rcvd:cmd %d recvd!\n", cmd.cmd_type);
        oal_complete(&g_hcc_test_event->test_trans_done);
    }
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    else if ((cmd.cmd_type == HCC_TEST_CMD_PCIE_MAC_LOOPBACK_TST) ||
             (cmd.cmd_type == HCC_TEST_CMD_PCIE_PHY_LOOPBACK_TST) ||
             (cmd.cmd_type == HCC_TEST_CMD_PCIE_REMOTE_PHY_LOOPBACK_TST)) {
        ret = memcpy_s(&g_pcie_test_request_ack, sizeof(g_pcie_test_request_ack),
                       (void *)hcc_get_test_cmd_data(oal_netbuf_data(pst_netbuf)),
                       sizeof(g_pcie_test_request_ack));
        if (ret != EOK) {
            oal_io_print("memcpy_s err!\n");
        }
        oal_complete(&g_pcie_test_trans_done);
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie test done type=%s",
                             (cmd.cmd_type == HCC_TEST_CMD_PCIE_MAC_LOOPBACK_TST) ? "mac" : "phy");
    }
#endif
}

/*
 * Prototype    : hcc_test_rcvd
 * Description  : test pkt rcvd
 * Input        : main_type   uint8_t
 *                test_type   uint8_t
 *                skb         sk_buff handle
 *                context     context handle
 * Return Value : succ or fail
 */
OAL_STATIC int32_t hcc_test_rcvd(struct hcc_handler *hcc, uint8_t stype,
                                 hcc_netbuf_stru *pst_hcc_netbuf,
                                 uint8_t *pst_context)
{
    oal_netbuf_stru *pst_netbuf = pst_hcc_netbuf->pst_netbuf;
    oal_reference(pst_context);
    oal_reference(hcc);

    if (oal_likely(stype == HCC_TEST_SUBTYPE_DATA)) {
        hcc_test_rcvd_data(pst_netbuf);
    } else if (stype == HCC_TEST_SUBTYPE_CMD) {
        hcc_test_rcvd_cmd(pst_netbuf);
    } else {
        /* unkown subtype */
        oal_io_print("receive unkown stype:%d\n", stype);
    }

    oal_netbuf_free(pst_netbuf);

    return OAL_SUCC;
}

/*
 * Prototype    : hcc_test_sent
 * Description  : test pkt sent
 * Input        : uint8_t  test_type, uint8_t gen_type
 * Return Value : succ or fail
 */
OAL_STATIC int32_t hcc_test_sent(struct hcc_handler *hcc, struct hcc_transfer_param *param, uint16_t start_cmd)
{
    int32_t ret;
    uint8_t pad_payload = g_hcc_test_event->pad_payload;
    oal_netbuf_stru *pst_netbuf;
    /*
    * 1) alloc memory for skb,
    * 2) skb free when send after dequeue from tx queue
    * */
    pst_netbuf = hcc_netbuf_alloc(g_hcc_test_event->test_data.pkt_len + pad_payload);
    if (pst_netbuf == NULL) {
        oal_io_print("hwifi alloc skb fail.\n");
        return -OAL_EFAIL;
    }

    if (pad_payload) {
        oal_netbuf_reserve(pst_netbuf, pad_payload);
    }

    if (g_hcc_test_event->test_value) {
        memset_s(oal_netbuf_put(pst_netbuf, g_hcc_test_event->test_data.pkt_len),
                 g_hcc_test_event->test_data.pkt_len,
                 g_hcc_test_event->test_value, g_hcc_test_event->test_data.pkt_len);
    } else {
        oal_netbuf_put(pst_netbuf, g_hcc_test_event->test_data.pkt_len);
    }

    if (start_cmd == HCC_TEST_SUBTYPE_DATA) {
        g_hcc_test_event->test_data.total_sent_bytes += oal_netbuf_len(pst_netbuf);
    }

    ret = hcc_tx(hcc, pst_netbuf, param);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);
    }

    return ret;
}

OAL_STATIC int32_t hcc_send_test_cmd(uint8_t *cmd, int32_t hdr_len, int32_t cmd_len)
{
    int32_t ret;
    oal_netbuf_stru *pst_netbuf = NULL;
    struct hcc_transfer_param st_hcc_transfer_param = {0};
    struct hcc_handler *hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        return -OAL_EFAIL;
    }

    pst_netbuf = hcc_netbuf_alloc(cmd_len);
    if (pst_netbuf == NULL) {
        oal_io_print("hwifi alloc skb fail.\n");
        return -OAL_EFAIL;
    }

    if (hdr_len > cmd_len) {
        hdr_len = cmd_len;
    }

    if (memcpy_s(oal_netbuf_put(pst_netbuf, cmd_len), cmd_len, cmd, hdr_len) != EOK) {
        oal_netbuf_free(pst_netbuf);
        oal_io_print("memcpy_s error, destlen=%d, srclen=%d\n ", cmd_len, hdr_len);
        return -OAL_EFAIL;
    }

    hcc_hdr_param_init(&st_hcc_transfer_param,
                       HCC_ACTION_TYPE_TEST,
                       HCC_TEST_SUBTYPE_CMD,
                       0,
                       HCC_FC_WAIT,
                       g_hcc_test_event->hcc_test_queue);
    ret =  hcc_tx(hcc, pst_netbuf, &st_hcc_transfer_param);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);
    }

    return ret;
}

OAL_STATIC int32_t hcc_test_rx_start(uint16_t start_cmd)
{
    uint32_t cmd_len;
    int32_t ret;

    hcc_test_cmd_stru *pst_cmd = NULL;
    hcc_trans_test_rx_info *pst_rx_info = NULL;

    struct hcc_handler *hcc;
    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        return -OAL_EFAIL;
    }
    cmd_len = sizeof(hcc_test_cmd_stru) + sizeof(hcc_trans_test_rx_info);
    pst_cmd = (hcc_test_cmd_stru *)oal_memalloc(cmd_len);
    if (pst_cmd == NULL) {
        return -OAL_EFAIL;
    }

    oal_init_completion(&g_hcc_test_event->test_trans_done);
    g_hcc_test_event->test_data.pkt_rcvd = 0;
    g_hcc_test_event->test_data.pkt_sent = 0;
    g_hcc_test_event->test_data.total_rcvd_bytes = 0;
    g_hcc_test_event->test_data.total_sent_bytes = 0;
    g_hcc_test_event->test_data.throughput = 0;
    g_hcc_test_event->test_data.trans_time = 0;
    g_hcc_test_event->start_tick = jiffies;
    g_hcc_test_event->last_time = g_hcc_test_event->start_time = ktime_get();

    memset_s((void *)pst_cmd, cmd_len, 0, cmd_len);
    pst_cmd->cmd_type = start_cmd;
    pst_cmd->cmd_len = cmd_len;

    pst_rx_info = (hcc_trans_test_rx_info *)hcc_get_test_cmd_data(pst_cmd);

    pst_rx_info->total_trans_pkts = g_hcc_test_event->test_data.pkt_gen;
    pst_rx_info->pkt_len = g_hcc_test_event->test_data.pkt_len;
    pst_rx_info->pkt_value = g_hcc_test_event->test_value;

    if (hcc_send_test_cmd((uint8_t *)pst_cmd, pst_cmd->cmd_len, pst_cmd->cmd_len) != OAL_SUCC) {
        oal_free(pst_cmd);
        return -OAL_EFAIL;
    }

    oal_free(pst_cmd);

    g_hcc_test_event->last_time = ktime_get();

    /* 等待回来的CMD命令 */
    ret = wait_for_completion_interruptible(&g_hcc_test_event->test_trans_done);
    if (ret < 0) {
        oal_io_print("Test Event  terminated ret=%d\n", ret);
        ret = -OAL_EFAIL;
        oal_io_print("H2D_MSG_STOP_SDIO_TEST send\n");
        hcc_send_message(hcc, H2D_MSG_STOP_SDIO_TEST);
    }

    if (g_test_force_stop) {
        hcc_send_message(hcc, H2D_MSG_STOP_SDIO_TEST);
        g_test_force_stop = 0;
        oal_msleep(100); // wait 100ms for stop message
    }

    oal_complete(&g_hcc_test_event->test_done);
    return ret;
}

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
static uint16_t map_cmd_type(int32_t is_phy_loopback)
{
    uint16_t cmd_type;

    if (is_phy_loopback == OAL_PHY_LOOPBACK_IS_MAC) {
        cmd_type = HCC_TEST_CMD_PCIE_MAC_LOOPBACK_TST;
    } else if (is_phy_loopback == OAL_PHY_LOOPBACK_IS_INTERNALPHY) {
        cmd_type = HCC_TEST_CMD_PCIE_PHY_LOOPBACK_TST;
    } else if (is_phy_loopback == OAL_PHY_LOOPBACK_IS_REMOTEPHY) {
        cmd_type = HCC_TEST_CMD_PCIE_REMOTE_PHY_LOOPBACK_TST;
    } else {
        cmd_type = 0;
    }
    return cmd_type;
}

static int32_t hcc_test_pcie_loopback_resp(hcc_pcie_test_request_stru *pcie_test_rq,
                                           int32_t is_phy_loopback)
{
    int32_t ret = OAL_SUCC;
    if (pcie_test_rq->request_flag != g_pcie_test_request_ack.request_flag) {
        ret = -OAL_EFAIL;
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "pcie test request mismatch request 0x%llx , return 0x%llx",
                             pcie_test_rq->request_flag,
                             g_pcie_test_request_ack.request_flag);
    } else {
        if (g_pcie_test_request_ack.response_ret == OAL_SUCC) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_test_ok linkup cost %u us , rw_test cost %u us , total %u us",
                                 g_pcie_test_request_ack.link_up_cost, g_pcie_test_request_ack.rw_test_cost,
                                 g_pcie_test_request_ack.total_test_cost);
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_ERR,
                                 "pcie_test_failed linkup cost %u us , rw_test cost %u us , total %u us, ret=%d",
                                 g_pcie_test_request_ack.link_up_cost, g_pcie_test_request_ack.rw_test_cost,
                                 g_pcie_test_request_ack.total_test_cost, g_pcie_test_request_ack.response_ret);
        }
        ret = g_pcie_test_request_ack.response_ret;
    }
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie %s test %s", is_phy_loopback ? "phy" : "mac",
                         (ret == OAL_SUCC) ? "succ" : "failed");
    return ret;
}

static int32_t hcc_test_pcie_loopback_vol_init(hcc_pcie_test_request_stru *pcie_test_rq)
{
    int32_t ret;
    uint32_t vol = 0;
    ret = oal_pcie_get_vol_reg_1v8_value(g_ft_pcie_loopback_bias_vph, &vol);
    if (ret != OAL_SUCC) {
        return -OAL_EFAUL;
    }
    pcie_test_rq->bias_vph = vol;

    ret = oal_pcie_get_vol_reg_0v9_value(g_ft_pcie_loopback_bias_cldo3, &vol);
    if (ret != OAL_SUCC) {
        return -OAL_EFAUL;
    }
    pcie_test_rq->bias_cldo3 = vol;

    ret = oal_pcie_get_vol_reg_0v9_value(g_ft_pcie_loopback_bias_vptx, &vol);
    if (ret != OAL_SUCC) {
        return -OAL_EFAUL;
    }
    pcie_test_rq->bias_vptx = vol;

    return OAL_SUCC;
}

OAL_STATIC int32_t hcc_test_pcie_loopback(int32_t is_phy_loopback)
{
    int32_t ret;
    oal_netbuf_stru *pst_netbuf = NULL;
    hcc_test_cmd_stru cmd = {0};
    hcc_pcie_test_request_stru pcie_test_rq = {0};

    struct hcc_handler *hcc = NULL;

    struct hcc_transfer_param st_hcc_transfer_param = {0};

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        return -OAL_EFAIL;
    }

    oal_init_completion(&g_pcie_test_trans_done);
    memset_s(&g_pcie_test_request_ack, sizeof(g_pcie_test_request_ack), 0, sizeof(g_pcie_test_request_ack));
    g_pcie_test_request_ack.response_ret = HCC_PCIE_RESPONSE_INITIAL_VALUE;

    cmd.cmd_type = map_cmd_type(is_phy_loopback);
    cmd.cmd_len = sizeof(hcc_test_cmd_stru);

    pst_netbuf = hcc_netbuf_alloc(cmd.cmd_len + sizeof(pcie_test_rq));
    if (pst_netbuf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc %u mem failed!", cmd.cmd_len);
        return -OAL_ENOMEM;
    }

    oal_netbuf_put(pst_netbuf, cmd.cmd_len + sizeof(pcie_test_rq));

    ret = memcpy_s((void *)oal_netbuf_data(pst_netbuf), cmd.cmd_len + sizeof(pcie_test_rq),
                   (void *)&cmd, cmd.cmd_len);
    if (ret != EOK) {
        oal_netbuf_free(pst_netbuf);
        oal_io_print("memcpy_s error, destlen=%u, srclen=%u\n ",
                     (uint32_t)(cmd.cmd_len + sizeof(pcie_test_rq)), (uint32_t)cmd.cmd_len);
        return -OAL_EFAIL;
    }

    pcie_test_rq.request_flag = (uintptr_t)&pcie_test_rq;
    pcie_test_rq.linkup_timeout = (uint32_t)g_ft_pcie_loopback_linkup_timeout;
    pcie_test_rq.rw_test_count = (uint32_t)g_ft_pcie_loopback_rw_test_count;
    ret = hcc_test_pcie_loopback_vol_init(&pcie_test_rq);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);
        return ret;
    }
    ret = memcpy_s((void *)hcc_get_test_cmd_data(oal_netbuf_data(pst_netbuf)),
                   sizeof(pcie_test_rq), (void *)&pcie_test_rq, sizeof(pcie_test_rq));
    if (ret != EOK) {
        oal_netbuf_free(pst_netbuf);
        oal_io_print("memcpy_s error!");
        return -OAL_EFAIL;
    }

    hcc_hdr_param_init(&st_hcc_transfer_param,
                       HCC_ACTION_TYPE_TEST,
                       HCC_TEST_SUBTYPE_CMD,
                       0,
                       HCC_FC_WAIT,
                       DATA_HI_QUEUE);

    ret = hcc_tx(hcc, pst_netbuf, &st_hcc_transfer_param);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pcie test send failed ret=%d", ret);
        oal_netbuf_free(pst_netbuf);
        return ret;
    }
    ret = wait_for_completion_interruptible_timeout(&g_pcie_test_trans_done, oal_msecs_to_jiffies(15000)); // 15000 ms
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie test Event  terminated ret=%d\n", ret);
        oal_dft_print_all_key_info();
        return ret;
    } else if (ret == 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pcie test timeout");
        oal_dft_print_all_key_info();
        return -OAL_ETIMEDOUT;
    }
    ret = hcc_test_pcie_loopback_resp(&pcie_test_rq, is_phy_loopback);
    return ret;
}
OAL_STATIC ssize_t hcc_test_pcie_loopback_get_para(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int32_t ret;
    int32_t count = 0;
    uint32_t buf_len = PAGE_SIZE - 1;
    if (dev == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "dev is null");
        return 0;
    }
    if (attr == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "attr is null");
        return 0;
    }
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "TestResult: %s\n",
                     (g_pcie_test_request_ack.response_ret == OAL_SUCC) ? "PASS" : "FAIL");
    if (ret < 0) {
        return count;
    }
    count += ret;
    if (g_pcie_test_request_ack.response_ret != OAL_SUCC) {
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "ReturnFailValue: %d\n",
                         g_pcie_test_request_ack.response_ret);
        if (ret < 0) {
            return count;
        }
        count += ret;
    }
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "Bias: CLDO3 %u mv VPTX %u mv VPH %u mv\n",
                     g_ft_pcie_loopback_bias_cldo3, g_ft_pcie_loopback_bias_vptx, g_ft_pcie_loopback_bias_vph);
    if (ret < 0) {
        return count;
    }
    count += ret;
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "TimeCost linkup %u us  %u ms\n",
                     g_pcie_test_request_ack.link_up_cost, g_pcie_test_request_ack.link_up_cost / 1000);
    if (ret < 0) {
        return count;
    }
    count += ret;
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "TimeCost rw_test_cost %u us  %u ms\n",
                     g_pcie_test_request_ack.rw_test_cost, g_pcie_test_request_ack.rw_test_cost / 1000);
    if (ret < 0) {
        return count;
    }
    count += ret;
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "TimeCost total_test_cost %u us  %u ms\n",
                     g_pcie_test_request_ack.total_test_cost, g_pcie_test_request_ack.total_test_cost / 1000);
    if (ret < 0) {
        return count;
    }
    count += ret;
    return count;
}

static int32_t hcc_test_pcie_loopback_set_para_input_check(struct kobject *dev, struct kobj_attribute *attr,
                                                           const char *buf, size_t count,
                                                           size_t max_count)
{
    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return -OAL_EFAUL;
    }
    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return -OAL_EFAUL;
    }
    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return -OAL_EFAUL;
    }

    if ((count == 0) || (count >= max_count) || (buf[count] != '\0')) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "input illegal!\n");
        return -OAL_EFAUL;
    }

    return OAL_SUCC;
}

static int32_t hcc_test_pcie_loopback_set_param_input_check(uint32_t vph, uint32_t cldo3,
                                                            uint32_t vptx, uint32_t *vol)
{
    int32_t ret = oal_pcie_get_vol_reg_1v8_value(vph, vol);
    if (ret != OAL_SUCC) {
        return ret;
    }
    ret = oal_pcie_get_vol_reg_0v9_value(cldo3, vol);
    if (ret != OAL_SUCC) {
        return ret;
    }
    ret = oal_pcie_get_vol_reg_0v9_value(vptx, vol);
    if (ret != OAL_SUCC) {
        return ret;
    }
    return OAL_SUCC;
}

static int32_t hcc_test_pcie_loopback_get_phy_loop_type(char *test_case, int32_t *is_phy_loopback)
{
    if (!oal_strcmp("mac", test_case)) {
        *is_phy_loopback = OAL_PHY_LOOPBACK_IS_MAC;
    } else if (!oal_strcmp("internalPhy", test_case)) {
        *is_phy_loopback = OAL_PHY_LOOPBACK_IS_INTERNALPHY;
    } else if (!oal_strcmp("remotePhy", test_case)) {
        *is_phy_loopback = OAL_PHY_LOOPBACK_IS_REMOTEPHY;
    } else {
        return -OAL_EFAUL;
    }
    return OAL_SUCC;
}

OAL_STATIC ssize_t hcc_test_pcie_loopback_set_para(struct kobject *dev, struct kobj_attribute *attr,
                                                   const char *buf, size_t count)
{
    int32_t ret;
    int32_t test_rw_count = 0;
    uint32_t cldo3 = 0;
    uint32_t vptx = 0;
    uint32_t vph = 0;
    int32_t is_phy_loopback = 0;
    int32_t test_linkup_timeout = 0;
    char test_case[128]; /* sys文件允许的最大输入字符长度128 */

    ret = hcc_test_pcie_loopback_set_para_input_check(dev, attr, buf, count, sizeof(test_case));
    if (ret != OAL_SUCC) {
        return 0;
    }

    if (!oal_strncmp("setparam", buf, OAL_STRLEN("setparam"))) {
        uint32_t vol;
        buf += OAL_STRLEN("setparam");
        for (; *buf == ' '; buf++);
        if ((sscanf_s(buf, "%u %u %u", &cldo3, &vptx, &vph) != 3)) { // 3 args
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "error input:%s", buf);
            return count;
        }
        ret = hcc_test_pcie_loopback_set_param_input_check(vph, cldo3, vptx, &vol);
        if (ret != OAL_SUCC) {
            return count;
        }
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "setbias vph:%umv cldo3:%umv vptx:%umv", vph, cldo3, vptx);
        g_ft_pcie_loopback_bias_vph = vph;
        g_ft_pcie_loopback_bias_cldo3 = cldo3;
        g_ft_pcie_loopback_bias_vptx = vptx;
    } else if (!oal_strncmp("runloopback", buf, OAL_STRLEN("runloopback"))) {
        buf += OAL_STRLEN("runloopback");
        for (; *buf == ' '; buf++);
        /* 3 args */
        if ((sscanf_s(buf, "%s %d %d", test_case, sizeof(test_case), &test_linkup_timeout, &test_rw_count) != 3)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "error input:%s", buf);
            return count;
        }
        g_ft_pcie_loopback_rw_test_count = test_rw_count;
        g_ft_pcie_loopback_linkup_timeout = test_linkup_timeout;
        ret = hcc_test_pcie_loopback_get_phy_loop_type(test_case, &is_phy_loopback);
        if (ret != OAL_SUCC) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "error input:%s", buf);
            return count;
        }
        g_pcie_test_request_ack.response_ret = HCC_PCIE_RESPONSE_INITIAL_VALUE;
        hcc_test_pcie_loopback(is_phy_loopback);
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "unkown:%s\n", buf);
    }
    return count;
}
OAL_STATIC struct kobj_attribute g_dev_attr_pcie_loopback =
    __ATTR(pcie_loopback, S_IRUGO | S_IWUSR, hcc_test_pcie_loopback_get_para,
           hcc_test_pcie_loopback_set_para);
#endif

OAL_STATIC int32_t hcc_test_fix_wcpu_freq(void)
{
    oal_netbuf_stru *pst_netbuf = NULL;
    hcc_test_cmd_stru cmd = {0};
    int32_t ret;

    struct hcc_handler *hcc = NULL;

    struct hcc_transfer_param st_hcc_transfer_param = {0};

    if (g_ft_ip_test_cpu_max_freq == 0) {
        return OAL_SUCC;
    }

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        return -OAL_EFAIL;
    }

    cmd.cmd_type = HCC_TEST_CMD_CFG_FIX_FREQ;
    cmd.cmd_len = sizeof(hcc_test_cmd_stru);

    pst_netbuf = hcc_netbuf_alloc(cmd.cmd_len);
    if (pst_netbuf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc %u mem failed!", cmd.cmd_len);
        return -OAL_ENOMEM;
    }

    ret = memcpy_s(oal_netbuf_put(pst_netbuf, cmd.cmd_len), cmd.cmd_len, &cmd, cmd.cmd_len);
    if (ret != EOK) {
        oal_netbuf_free(pst_netbuf);
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "copy cmd to netbuf failed!");
        return -OAL_EFAIL;
    }

    hcc_hdr_param_init(&st_hcc_transfer_param,
                       HCC_ACTION_TYPE_TEST,
                       HCC_TEST_SUBTYPE_CMD,
                       0,
                       HCC_FC_WAIT,
                       DATA_HI_QUEUE);

    ret = hcc_tx(hcc, pst_netbuf, &st_hcc_transfer_param);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);
    }

    return ret;
}

OAL_STATIC int32_t hcc_test_normal_start(uint16_t start_cmd)
{
    int32_t ret;
    int32_t retry_count = 0;
    int32_t i;
    hcc_test_cmd_stru cmd = {0};

    struct hcc_transfer_param st_hcc_transfer_param = {0};
    struct hcc_handler *hcc = NULL;
    hcc_hdr_param_init(&st_hcc_transfer_param, HCC_ACTION_TYPE_TEST, HCC_TEST_SUBTYPE_DATA, 0, HCC_FC_WAIT,
                       g_hcc_test_event->hcc_test_queue);
    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "wifi is closed!");
        return -OAL_EFAIL;
    }

    oal_init_completion(&g_hcc_test_event->test_trans_done);
    g_hcc_test_event->test_data.pkt_rcvd = 0;
    g_hcc_test_event->test_data.pkt_sent = 0;
    g_hcc_test_event->test_data.total_rcvd_bytes = 0;
    g_hcc_test_event->test_data.total_sent_bytes = 0;
    g_hcc_test_event->test_data.throughput = 0;
    g_hcc_test_event->test_data.trans_time = 0;

    cmd.cmd_type = start_cmd;
    cmd.cmd_len = sizeof(hcc_test_cmd_stru) + sizeof(hsdio_trans_test_info);
    if (hcc_send_test_cmd((uint8_t *)&cmd, sizeof(cmd), cmd.cmd_len) != OAL_SUCC) {
        return -OAL_EFAIL;
    }
    /* wait the device recv the cmd,change the test mode! */
    oal_msleep(50);

    g_hcc_test_event->last_time = g_hcc_test_event->start_time = ktime_get();
    g_hcc_test_event->start_tick = jiffies;

    for (i = 0; i < g_hcc_test_event->test_data.pkt_gen; i++) {
        ret = hcc_test_sent(hcc, &st_hcc_transfer_param, HCC_TEST_SUBTYPE_DATA);
        if (ret < 0) {
            oal_io_print("hcc test gen pkt send fail.\n");
            break;
        }

        g_hcc_test_event->test_data.pkt_sent++;
        g_hcc_test_event->last_time = ktime_get();

        if (time_after(jiffies, (g_hcc_test_event->start_tick + g_hcc_test_event->tick_timeout))) {
            oal_io_print("TestTimeIsUp\n");
            break;
        }

        if (oal_unlikely(g_hcc_test_event->started == OAL_FALSE)) {
            ret = -OAL_EFAIL;
            break;
        }
    }

    cmd.cmd_type = HCC_TEST_CMD_STOP_TEST;
    hcc_send_test_cmd((uint8_t *)&cmd, sizeof(cmd), cmd.cmd_len);
    g_hcc_test_event->last_time = ktime_get();

retry:
    /* 等待回来的CMD命令 */
    ret = wait_for_completion_interruptible_timeout(&g_hcc_test_event->test_trans_done, oal_msecs_to_jiffies(5000));
    if (ret < 0) {
        oal_io_print("Test Event  terminated ret=%d\n", ret);
        ret = -OAL_EFAIL;
        hcc_print_current_trans_info(0);
        hcc_send_test_cmd((uint8_t *)&cmd, sizeof(cmd), cmd.cmd_len);
    } else if (ret == 0) {
        /* cmd response timeout */
        if (retry_count++ < 1) {
            oal_msleep(100);
            hcc_send_test_cmd((uint8_t *)&cmd, sizeof(cmd), cmd.cmd_len);
            g_hcc_test_event->last_time = ktime_get();
            oal_io_print("resend the stop cmd!retry count:%d\n", retry_count);
            goto retry;
        } else {
            oal_io_print("resend the stop cmd timeout!retry count:%d\n", retry_count);
            ret = -OAL_EFAIL;
        }
    } else {
        if (g_test_force_stop) {
            hcc_send_test_cmd((uint8_t *)&cmd, sizeof(cmd), cmd.cmd_len);
            g_hcc_test_event->last_time = ktime_get();
            g_test_force_stop = 0;
            oal_io_print("normal start force stop\n");
            oal_msleep(100);
        }
        ret = OAL_SUCC;
    }

    oal_complete(&g_hcc_test_event->test_done);
    return ret;
}

OAL_STATIC int32_t hcc_test_start(uint16_t start_cmd)
{
    oal_io_print("%s Test start.\n", g_hcc_test_stru[g_hcc_test_event->test_data.mode_idx].mode);
    if (g_hcc_test_event->test_data.mode_idx == HCC_TEST_CASE_RX) {
        return hcc_test_rx_start(start_cmd);
    } else {
        return hcc_test_normal_start(start_cmd);
    }
}

/*
 * Prototype    :hcc_test_work
 * Description  :sdio test work_struct function
 * Input        :main_type char
 * Return Value : succ or fail
 */
OAL_STATIC void hcc_test_work(oal_work_stru *work)
{
    uint16_t start_cmd;
    int32_t ret;

#ifdef _PRE_HISI_BINDCPU
    struct cpumask fast_cpus;
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    struct sched_param param = {0};
#endif

    start_cmd = g_hcc_test_stru[g_hcc_test_event->test_data.mode_idx].start_cmd;

    if (g_hcc_test_performance_mode) {
#ifdef _PRE_HISI_BINDCPU
#ifdef CONFIG_ARCH_PLATFORM
        get_fast_cpus(&fast_cpus);
#else
        hisi_get_fast_cpus(&fast_cpus);
#endif
        oal_io_print("hcc test bind to fast cpus\n");
        set_cpus_allowed_ptr(current, &fast_cpus);
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        param.sched_priority = OAL_BUS_TEST_WORK_PRIORITY;
        oal_set_thread_property(current,
                                SCHED_RR,
                                &param,
                                OAL_BUS_TEST_WORK_NICE);
        oal_msleep(oal_jiffies_to_msecs(1));
#endif
    }

    /* hcc test start */
    ret = hcc_test_start(start_cmd);
    if (ret == -OAL_EFAIL) {
        g_hcc_test_event->errorno = ret;
        oal_complete(&g_hcc_test_event->test_done);
        oal_io_print("hcc test work start test pkt send fail. ret = %d\n", ret);
        return;
    }
}

OAL_STATIC ssize_t hcc_test_print_thoughput(char *buf, uint32_t buf_len)
{
    int ret;
    int32_t count = 0;
    const char *mode_str = NULL;
    int32_t tmp_mode_idx;

    if (buf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "buf is null");
        return 0;
    }

    tmp_mode_idx = g_hcc_test_event->test_data.mode_idx;

    mode_str = "unknown";

    if ((tmp_mode_idx >= 0) && (tmp_mode_idx < oal_array_size(g_hcc_test_stru))) {
        mode_str = g_hcc_test_stru[tmp_mode_idx].mode;
    }

    hcc_test_throughput_gen();

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "Test_Mode: %s\n", mode_str);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "Actual sent %d pkts, request %llu bytes\n",
                     g_hcc_test_event->test_data.pkt_sent,
                     ((uint64_t)g_hcc_test_event->test_data.pkt_sent) *
                     (uint64_t)g_hcc_test_event->test_data.pkt_len);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "Actual rcvd %d pkts, request %llu bytes\n",
                     g_hcc_test_event->test_data.pkt_rcvd,
                     ((uint64_t)g_hcc_test_event->test_data.pkt_rcvd) *
                     (uint64_t)g_hcc_test_event->test_data.pkt_len);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "PayloadSend %llu bytes, ActualSend  %llu bytes\n",
                     g_hcc_test_event->test_data.total_sent_bytes,
                     g_hcc_test_event->test_data.trans_info.total_h2d_trans_bytes);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "PayloadRcvd %llu bytes, ActualRecv  %llu bytes\n",
                     g_hcc_test_event->test_data.total_rcvd_bytes,
                     g_hcc_test_event->test_data.trans_info.total_d2h_trans_bytes);
    if (ret < 0) {
        return count;
    }
    count += ret;

    /* HCC通道利用率 */
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "Hcc Utilization Ratio %llu\n",
                     hcc_test_utilization_ratio_gen(g_hcc_test_event->test_data.total_sent_bytes +
                                                    g_hcc_test_event->test_data.total_rcvd_bytes,
                                                    g_hcc_test_event->test_data.trans_info.total_h2d_trans_bytes +
                                                    g_hcc_test_event->test_data.trans_info.total_d2h_trans_bytes));
    if (ret < 0) {
        return count;
    }
    count += ret;

    /* 发送方向的丢包率 */
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "TxPackageLoss %u, pkt_sent: %d actual_tx_pkts: %u\n",
                     hcc_test_tx_pkt_loss_gen(g_hcc_test_event->test_data.pkt_sent,
                                              g_hcc_test_event->test_data.trans_info.actual_tx_pkts),
                     g_hcc_test_event->test_data.pkt_sent,
                     g_hcc_test_event->test_data.trans_info.actual_tx_pkts);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "Requet Generate %d pkts\n",
                     g_hcc_test_event->test_data.pkt_gen);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "Per-package Length %d\n",
                     g_hcc_test_event->test_data.pkt_len);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "TranserTimeCost %llu Seconds, %llu microsecond\n",
                     g_hcc_test_event->test_data.trans_time, g_hcc_test_event->test_data.trans_time_us);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "Throughput %u Mbps\n",
                     (int32_t)g_hcc_test_event->test_data.throughput);
    if (ret < 0) {
        return count;
    }
    count += ret;

    return count;
}

/*
 * Prototype    : hcc_test_get_para
 * Description  : get test param
 * Input        : dev   device_handle
 *                attr  device_attribute handle
 *                buf   data buf handle
 * Return Value : get test param
 */
OAL_STATIC ssize_t hcc_test_get_para(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    if (dev == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "dev is null");
        return 0;
    }

    if (attr == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "attr is null");
        return 0;
    }

    return hcc_test_print_thoughput(buf, PAGE_SIZE - 1);
}

/*
 * Prototype    : hcc_test_set_para
 * Description  : set test param
 */
OAL_STATIC ssize_t hcc_test_set_para(struct kobject *dev, struct kobj_attribute *attr, const char *buf, size_t count)
{
    hcc_test_data data = {0};
    int32_t tmp_pkt_len;
    int32_t tmp_pkt_gen;
    char mode[OAL_SAVE_MODE_BUFF_SIZE] = {0};
    int32_t i;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((count == 0) || (count >= sizeof(mode)) || (buf[count] != '\0')) {
        oal_io_print("input illegal!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    if ((sscanf_s(buf, "%15s %d %d", mode, sizeof(mode), &tmp_pkt_len, &tmp_pkt_gen) < 1)) {
        oal_io_print("error input,must input more than 1 arguments!\n");
        return -OAL_EINVAL;
    }

    for (i = 0; i < oal_array_size(g_hcc_test_stru); i++) {
        /* find mode if match */
        if (sysfs_streq(g_hcc_test_stru[i].mode, mode)) {
            break;
        }
    }

    if (i == oal_array_size(g_hcc_test_stru)) {
        oal_io_print("unknown test mode.%s\n", mode);

        return -OAL_EINVAL;
    }

    data.pkt_len = tmp_pkt_len;
    data.pkt_gen = tmp_pkt_gen;
    data.mode_idx = i;
    data.trans_timeout = ~0UL;

    if (hcc_test_set_case(&data)) {
        return count;
    }

    return count;
}

/*
 * Prototype    : hcc_test_set_rt_para
 * Description  : set test param
 */
OAL_STATIC ssize_t hcc_test_set_rt_para(struct kobject *dev, struct kobj_attribute *attr,
                                        const char *buf, size_t count)
{
    hcc_test_data data = {0};
    int32_t tmp_pkt_len;
    unsigned long tmp_runtime;
    char mode[OAL_SAVE_MODE_BUFF_SIZE] = {0};
    int32_t i;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((count == 0) || (count >= sizeof(mode)) || (buf[count] != '\0')) {
        oal_io_print("input illegal!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    if ((sscanf_s(buf, "%15s %d %lu", mode, sizeof(mode), &tmp_pkt_len, &tmp_runtime) < 1)) {
        oal_io_print("error input,must input more than 1 arguments!\n");
        return -OAL_EINVAL;
    }

    for (i = 0; i < oal_array_size(g_hcc_test_stru); i++) {
        /* find mode if match */
        if (sysfs_streq(g_hcc_test_stru[i].mode, mode)) {
            break;
        }
    }

    if (i == oal_array_size(g_hcc_test_stru)) {
        oal_io_print("unknown test mode.%s\n", mode);

        return -OAL_EINVAL;
    }

    data.pkt_len = tmp_pkt_len;
    data.pkt_gen = 0x7fffffff;
    data.mode_idx = i;
    data.trans_timeout = tmp_runtime;

    if (hcc_test_set_case(&data)) {
        return count;
    }

    return count;
}

OAL_STATIC struct kobj_attribute g_dev_attr_test =
    __ATTR(test, S_IRUGO | S_IWUSR, hcc_test_get_para, hcc_test_set_para);
OAL_STATIC struct kobj_attribute g_dev_attr_test_rt =
    __ATTR(test_rt, S_IRUGO | S_IWUSR, hcc_test_get_para, hcc_test_set_rt_para);

OAL_STATIC ssize_t hcc_dev_panic_set_para(struct kobject *dev, struct kobj_attribute *attr,
                                          const char *buf, size_t count)
{
    char mode[OAL_SAVE_MODE_BUFF_SIZE] = {0};

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((count == 0) || (count >= sizeof(mode)) || (buf[count] != '\0')) {
        oal_io_print("input illegal!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    if ((sscanf_s(buf, "%s", mode, sizeof(mode)) < 1)) {
        oal_io_print("error input,must input more than 1 arguments!\n");
        return count;
    }

    if (oal_strncmp(mode, "panic", OAL_STRLEN("panic"))) {
        oal_io_print("invalid input:%s\n", mode);
        return count;
    }

    wlan_pm_disable();
    hcc_bus_send_message(hcc_get_current_110x_bus(), H2D_MSG_TEST);
    wlan_pm_enable();
    return count;
}
OAL_STATIC struct kobj_attribute g_dev_attr_dev_panic =
    __ATTR(dev_panic, S_IWUSR, NULL, hcc_dev_panic_set_para);

OAL_STATIC ssize_t hcc_test_set_abort_test(struct kobject *dev, struct kobj_attribute *attr,
                                           const char *buf, size_t count)
{
    int32_t ret;
    struct hcc_handler *hcc;
    hcc = hcc_get_110x_handler();
    ret = hcc_send_message(hcc, H2D_MSG_STOP_SDIO_TEST);
    oal_io_print("hcc_test_set_abort_test msg send ret=%d\n", ret);
    oal_msleep(500);
    return count;
}

OAL_STATIC struct kobj_attribute g_dev_attr_abort_test =
    __ATTR(abort_test, S_IWUSR, NULL, hcc_test_set_abort_test);

OAL_STATIC ssize_t hcc_test_set_value(struct kobject *dev, struct kobj_attribute *attr, const char *buf, size_t count)
{
    uint32_t value;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((sscanf_s(buf, "0x%2x", &value) != 1)) {
        oal_io_print("set value one char!\n");
        return -OAL_EINVAL;
    }

    g_hcc_test_event->test_value = value;

    return count;
}

OAL_STATIC ssize_t hcc_test_get_value(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret += snprintf_s(buf + ret, PAGE_SIZE, PAGE_SIZE - 1, "0x%2x\n", g_hcc_test_event->test_value);
    return ret;
}

OAL_STATIC ssize_t hcc_test_set_queue_id(struct kobject *dev, struct kobj_attribute *attr,
                                         const char *buf, size_t count)
{
    uint32_t queue_id;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((sscanf_s(buf, "%u", &queue_id) != 1)) {
        oal_io_print("set value one char!\n");
        return -OAL_EINVAL;
    }
    if (queue_id >= HCC_QUEUE_COUNT) {
        oal_io_print("wrong queue id:%u\n", queue_id);
        return count;
    }
    g_hcc_test_event->hcc_test_queue = (hcc_queue_type)queue_id;

    return count;
}

OAL_STATIC ssize_t hcc_test_get_queue_id(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret += snprintf_s(buf + ret, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", g_hcc_test_event->hcc_test_queue);
    return ret;
}

OAL_STATIC ssize_t hcc_test_set_pad_payload(struct kobject *dev, struct kobj_attribute *attr,
                                            const char *buf, size_t count)
{
    uint32_t pad_payload;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((sscanf_s(buf, "%u", &pad_payload) != 1)) {
        oal_io_print("set value one char!\n");
        return -OAL_EINVAL;
    }

    g_hcc_test_event->pad_payload = (uint8_t)pad_payload;

    return count;
}

OAL_STATIC ssize_t hcc_test_get_pad_payload(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }
    ret += snprintf_s(buf + ret, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", g_hcc_test_event->pad_payload);
    return ret;
}

OAL_STATIC ssize_t hcc_test_set_verified(struct kobject *dev, struct kobj_attribute *attr,
                                         const char *buf, size_t count)
{
    uint32_t verified;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((sscanf_s(buf, "%u", &verified) != 1)) {
        oal_io_print("set value one char!\n");
        return -OAL_EINVAL;
    }
    verified = !!verified;
    g_hcc_test_event->verified = (uint8_t)verified;

    return count;
}

OAL_STATIC ssize_t hcc_test_get_verified(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret += snprintf_s(buf + ret, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", g_hcc_test_event->verified);
    return ret;
}

OAL_STATIC ssize_t hcc_test_set_switch(struct kobject *dev, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int32_t ret;
    uint8_t input[200]; /* 200字节足够存放参数 */

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((count == 0) || (count >= sizeof(input)) || (buf[count] != '\0')) {
        oal_io_print("input illegal!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    memset_s((void *)input, sizeof(input), 0, sizeof(input));

    if ((sscanf_s(buf, "%s", input, sizeof(input)) != 1)) {
        oal_io_print("set value one char!\n");
        return -OAL_EINVAL;
    }

    ret = conn_test_hcc_chann_switch(input);
    if (ret == OAL_SUCC) {
        oal_io_print("swtich to %s succ\n", input);
    } else {
        oal_io_print("swtich to %s failed\n", input);
    }

    return count;
}

OAL_STATIC ssize_t hcc_test_get_switch(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    char *current_bus = NULL;
    hcc_bus *pst_bus = NULL;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    pst_bus = hcc_get_current_110x_bus();
    if (pst_bus == NULL) {
        current_bus = "none";
    } else {
        if (pst_bus->bus_type == HCC_BUS_SDIO) {
            current_bus = "sdio";
        } else if (pst_bus->bus_type == HCC_BUS_PCIE) {
            current_bus = "pcie";
        } else if (pst_bus->bus_type == HCC_BUS_USB) {
            current_bus = "usb";
        } else {
            current_bus = "unkown type";
        }
    }

    ret += snprintf_s(buf + ret, PAGE_SIZE, PAGE_SIZE - 1, "%s\n", current_bus);
    return ret;
}

OAL_STATIC struct kobj_attribute g_dev_attr_value =
    __ATTR(value, S_IRUGO | S_IWUSR, hcc_test_get_value, hcc_test_set_value);
OAL_STATIC struct kobj_attribute g_dev_attr_queue_id =
    __ATTR(queue_id, S_IRUGO | S_IWUSR, hcc_test_get_queue_id, hcc_test_set_queue_id);
OAL_STATIC struct kobj_attribute g_dev_attr_pad_payload =
    __ATTR(pad_payload, S_IRUGO | S_IWUSR, hcc_test_get_pad_payload, hcc_test_set_pad_payload);
OAL_STATIC struct kobj_attribute g_dev_attr_verified =
    __ATTR(verified, S_IRUGO | S_IWUSR, hcc_test_get_verified, hcc_test_set_verified);
OAL_STATIC struct kobj_attribute g_dev_attr_switch_bus =
    __ATTR(switch_bus, S_IRUGO | S_IWUSR, hcc_test_get_switch, hcc_test_set_switch);

OAL_STATIC struct attribute *g_hcc_test_sysfs_entries[] = {
    &g_dev_attr_test.attr,
    &g_dev_attr_test_rt.attr,
    &g_dev_attr_abort_test.attr,
    &g_dev_attr_value.attr,
    &g_dev_attr_verified.attr,
    &g_dev_attr_queue_id.attr,
    &g_dev_attr_pad_payload.attr,
    &g_dev_attr_switch_bus.attr,
    &g_dev_attr_dev_panic.attr,
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    &g_dev_attr_pcie_loopback.attr,
#endif
    NULL
};

OAL_STATIC struct attribute_group g_hcc_test_attribute_group = {
    .name = "test",
    .attrs = g_hcc_test_sysfs_entries,
};

OAL_STATIC struct attribute *g_hcc_sysfs_entries[] = {
    &g_dev_attr_flowctrl.attr,
    &g_dev_attr_assem_info.attr,
    &g_dev_attr_queues_pkts.attr,
    &g_dev_attr_queues_len.attr,
    &g_dev_attr_wakelock.attr,
    &g_dev_attr_wakelockall.attr,
    NULL
};
OAL_STATIC struct attribute_group g_hcc_attribute_group = {
    .attrs = g_hcc_sysfs_entries,
};

OAL_STATIC int32_t hsdio_sysfs_info_print(void *data, char *buf, int32_t buf_len)
{
    int32_t ret;
    int32_t bit;
    int32_t count = 0;
    struct oal_sdio *hi_sdio = (struct oal_sdio *)data;
    if (hi_sdio == NULL) {
        return 0;
    }
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "sdio info, state:0x%4x\n gpio_int_count:%llu \n wakeup_int_count:%llu \n",
                     hi_sdio->state, hi_sdio->pst_bus->gpio_int_count,
                     hi_sdio->pst_bus->wakeup_int_count);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "func1_err_reg_info:%u \n func1_err_int_count:%u \n func1_ack_int_acount:%u \n",
                     hi_sdio->func1_stat.func1_err_reg_info,
                     hi_sdio->func1_stat.func1_err_int_count,
                     hi_sdio->func1_stat.func1_ack_int_acount);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "func1_msg_int_count:%u \n func1_data_int_count:%u \n func1_no_int_count:%u \n",
                     hi_sdio->func1_stat.func1_msg_int_count,
                     hi_sdio->func1_stat.func1_data_int_count,
                     hi_sdio->func1_stat.func1_no_int_count);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "\nsdio error stat:\n rx_scatt_info_not_match:%d\n",
                     hi_sdio->error_stat.rx_scatt_info_not_match);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "msg count info:\n tx scatt buf len:%u\n rx scatt buf len:%u\n",
                     hi_sdio->tx_scatt_buff.len, hi_sdio->rx_scatt_buff.len);
    if (ret < 0) {
        return count;
    }
    count += ret;

    for (bit = 0; bit < D2H_MSG_COUNT; bit++) {
        if (hi_sdio->pst_bus->msg[bit].count) {
            ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                             "msg [%d] count:%u:,last update time:%llu\n",
                             bit, hi_sdio->pst_bus->msg[bit].count,
                             hi_sdio->pst_bus->msg[bit].cpu_time);
            if (ret < 0) {
                return count;
            }
            count += ret;
        }
    }
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "last msg:%u\n", hi_sdio->pst_bus->last_msg);
    if (ret < 0) {
        return count;
    }
    count += ret;
    return count;
}

OAL_STATIC ssize_t hsdio_get_sdio_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    struct hcc_handler *hcc = NULL;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        oal_io_print("get hcc handler failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (oal_get_sdio_default_handler()) {
        ret += hsdio_sysfs_info_print(oal_get_sdio_default_handler(), buf, PAGE_SIZE - ret);
    }

    return ret;
}

int32_t hsdio_wakelock_info_print(struct oal_sdio *hi_sdio, char *buf, int32_t buf_len)
{
    int32_t ret;
    int32_t count = 0;

#ifdef CONFIG_PRINTK
    if (hi_sdio->pst_bus->st_bus_wakelock.locked_addr) {
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "wakelocked by:%pf\n",
                         (void *)hi_sdio->pst_bus->st_bus_wakelock.locked_addr);
        if (ret < 0) {
            return count;
        }
        count += ret;
    }
#endif

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "hold %lu locks\n",
                     hi_sdio->pst_bus->st_bus_wakelock.lock_count);
    if (ret < 0) {
        return count;
    }
    count += ret;

    return count;
}

OAL_STATIC ssize_t hsdio_get_wakelock_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    struct hcc_handler *hcc = NULL;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        oal_io_print("get hcc handler failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (oal_get_sdio_default_handler()) {
        ret += hsdio_wakelock_info_print(oal_get_sdio_default_handler(), buf, PAGE_SIZE - ret);
    }

    return ret;
}

OAL_STATIC struct kobj_attribute g_dev_attr_sdio_info = __ATTR(sdio_info, S_IRUGO, hsdio_get_sdio_info, NULL);
OAL_STATIC struct kobj_attribute g_dev_attr_sdio_wakelock = __ATTR(wakelock, S_IRUGO, hsdio_get_wakelock_info, NULL);
OAL_STATIC struct attribute *g_hsdio_sysfs_entries[] = {
    &g_dev_attr_sdio_info.attr,
    &g_dev_attr_sdio_wakelock.attr,
    NULL
};

OAL_STATIC struct attribute_group g_hsdio_attribute_group = {
    .name = "sdio",
    .attrs = g_hsdio_sysfs_entries,
};

int hcc_test_set_case(hcc_test_data *data)
{
    int ret;
    int errorno;
    if (oal_warn_on(data == NULL)) {
        return -OAL_EINVAL;
    }

    if (oal_unlikely(g_hcc_test_event->test_workqueue == NULL)) {
        oal_io_print("wifi probe failed, please retry.\n");
        return -OAL_EBUSY;
    }

    mutex_lock(&g_hcc_test_event->mutex_lock);
    if (g_hcc_test_event->started == OAL_TRUE) {
        oal_io_print("sdio test task is processing, wait for end and reinput.\n");
        mutex_unlock(&g_hcc_test_event->mutex_lock);
        return -OAL_EINVAL;
    }

    oal_io_print("%s Test Start,test pkts:%d,pkt len:%d\n",
                 g_hcc_test_stru[data->mode_idx].mode, data->pkt_gen, data->pkt_len);

    g_hcc_test_event->started = OAL_TRUE;
    g_hcc_test_event->rx_stop = 0;
    g_hcc_test_event->errorno = OAL_SUCC;
    g_hcc_test_event->tick_timeout = oal_msecs_to_jiffies(data->trans_timeout);

    memcpy_s(&g_hcc_test_event->test_data, sizeof(hcc_test_data), data, sizeof(hcc_test_data));

    g_test_force_stop = 0;
    oal_reinit_completion(g_hcc_test_event->test_done);

    queue_work(g_hcc_test_event->test_workqueue, &g_hcc_test_event->test_work);
    ret = wait_for_completion_interruptible(&g_hcc_test_event->test_done);
    if (ret < 0) {
        oal_io_print("Test Event  terminated ret=%d\n", ret);
        g_hcc_test_event->started = OAL_FALSE;
        oal_complete(&g_hcc_test_event->test_trans_done);
        g_test_force_stop = 1;
        cancel_work_sync(&g_hcc_test_event->test_work);
    }

    oal_io_print("Test Done.ret=%d\n", g_hcc_test_event->errorno);

    hcc_test_throughput_gen();

    memcpy_s(data, sizeof(hcc_test_data), &g_hcc_test_event->test_data, sizeof(hcc_test_data));
    g_hcc_test_event->started = OAL_FALSE;
    errorno = g_hcc_test_event->errorno;
    mutex_unlock(&g_hcc_test_event->mutex_lock);
    return errorno;
}

OAL_STATIC int32_t hcc_test_current_bus_chan(int32_t pkt_len,
                                             int32_t mode_idx,
                                             unsigned long trans_timeout,
                                             uint32_t min_throught,
                                             int32_t retry_cnt)
{
    int32_t ret;
    int32_t retry_times = 0;

    void *pst_buff = NULL;

    hcc_test_data data = {0};

    data.pkt_len = pkt_len;
    data.pkt_gen = 0x7fffffff;
    data.trans_timeout = trans_timeout;
    data.mode_idx = mode_idx;

    g_netdev_is_open = OAL_TRUE;
    ret = wlan_pm_open();
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 下发固定频率的命令 */
    ret = hcc_test_fix_wcpu_freq();
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "set  wcpu fix freq failed! ret=%d", ret);
        wlan_pm_close();
        return ret;
    }

    /* 电压拉偏 */
    hcc_bus_voltage_bias_init(hcc_get_current_110x_bus());

retry:

    ret = hcc_test_set_case(&data);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "conn_test_wifi_chan_loop test failed!");
        wlan_pm_close();
        return ret;
    }

    pst_buff = oal_memalloc(PAGE_SIZE);
    if (pst_buff != NULL) {
        (void)memset_s(pst_buff, PAGE_SIZE, 0, 1);
        hcc_test_print_thoughput(pst_buff, PAGE_SIZE - 1);
        oal_io_print("%s", (char *)pst_buff);
        oal_free(pst_buff);
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc mem failed!");
    }

    if (min_throught > 0) {
        /* 吞吐率门限，当小于最小值时，认为性能失败 */
        if (g_hcc_test_event->test_data.throughput < (uint64_t)min_throught) {
            retry_times++;
            if (retry_times <= retry_cnt) {
                /* retry */
                oal_print_hi11xx_log(HI11XX_LOG_WARN,
                    "loop test warning, throught too low, just %llu Mbps, min_limit is %u Mbps, retry_cnt:%d",
                    g_hcc_test_event->test_data.throughput,
                    min_throught,
                    retry_cnt);
                goto retry;
            } else {
                /* touch retry limit, failed */
                oal_print_hi11xx_log(HI11XX_LOG_ERR,
                    "loop test failed, throught too low, just %llu Mbps, min_limit is %u Mbps, retry_cnt:%d",
                    g_hcc_test_event->test_data.throughput,
                    min_throught,
                    retry_cnt);
                wlan_pm_close();
                return -OAL_EFAIL;
            }
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "throught %llu Mbps over limit %u Mbps, retry times %d, test pass!",
                                 g_hcc_test_event->test_data.throughput,
                                 min_throught,
                                 retry_times);
        }
    }

    hcc_bus_chip_info(hcc_get_current_110x_bus(), OAL_TRUE, OAL_TRUE);

    ret = wlan_pm_close();
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "conn_test_wifi_chan_loop test failed! ret=%d", ret);
    }

    return OAL_SUCC;
}

static int32_t conn_test_wifi_chan_pcie(int32_t pkt_len)
{
    int32_t ret;
    int32_t pcie_min_throught = 0;
    hcc_bus *pst_bus = hcc_get_current_110x_bus();
    if (pst_bus == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_bus is null");
        return -OAL_EFAIL;
    }

    if (pst_bus->bus_type != HCC_BUS_PCIE) {
        /* 尝试切换到PCIE */
        ret = hcc_switch_bus(HCC_CHIP_110X_DEV, HCC_BUS_PCIE);
        if (ret) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "switch to PCIe failed, ret=%d", ret);
            return -OAL_ENODEV;
        }
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "switch to PCIe ok.");
    }

    /* Test Start */
    /* check pcie loop first */
    ret = hcc_test_current_bus_chan(pkt_len, HCC_TEST_CMD_START_LOOP,
                                    g_ft_pcie_test_wifi_runtime, 0, 0);
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "test PCIe failed, ret=%d", ret);
        return ret;
    }

    if (get_cust_conf_int32(INI_MODU_PLAT, "pcie_min_throught", &pcie_min_throught) == INI_SUCC) {
        g_ft_pcie_test_min_throught = pcie_min_throught;
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie min throught %d Mbps from ini", pcie_min_throught);
    }

    /* check the pcie throught */
    if (g_ft_pcie_test_min_throught) {
        ret = hcc_test_current_bus_chan(pkt_len, HCC_TEST_CMD_START_RX,
                                        g_ft_pcie_test_wifi_runtime,
                                        (uint32_t)g_ft_pcie_test_min_throught,
                                        g_ft_pcie_test_retry_cnt);
        if (ret != OAL_SUCC) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "test PCIe failed, ret=%d", ret);
            return ret;
        }
    }

    return OAL_SUCC;
}

#ifdef CONFIG_MMC
static int32_t conn_test_wifi_chan_sdio(int32_t pkt_len)
{
    int32_t ret;
    hcc_bus *pst_bus = hcc_get_current_110x_bus();
    if (pst_bus == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_bus is null");
        return -OAL_EFAIL;
    }

    if (pst_bus->bus_type != HCC_BUS_SDIO) {
        /* 尝试切换到SDIO */
        ret = hcc_switch_bus(HCC_CHIP_110X_DEV, HCC_BUS_SDIO);
        if (ret) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "switch to PCIe failed, ret=%d", ret);
            return -OAL_ENODEV;
        }
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "switch to SDIO ok.");
    }

    /* Test Start */
    ret = hcc_test_current_bus_chan(pkt_len,
                                    HCC_TEST_CMD_START_RX,
                                    g_ft_sdio_test_wifi_runtime,
                                    0, 0);
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "test PCIe failed, ret=%d", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif

static int32_t conn_test_wifi_chan_restore(hcc_bus *old_bus)
{
    int32_t ret;
    hcc_bus *pst_bus = hcc_get_current_110x_bus();
    if (pst_bus == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_bus is null");
        return -OAL_EFAIL;
    }

    if (old_bus->bus_type != pst_bus->bus_type) {
        ret = hcc_switch_bus(HCC_CHIP_110X_DEV, old_bus->bus_type);
        if (ret) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "restore to %u failed, ret=%d", old_bus->bus_type, ret);
            return -OAL_ENODEV;
        }
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "restore to %d ok.", old_bus->bus_type);
    }

    return OAL_SUCC;
}

/* 测试WIFI通道是否连通 */
int32_t conn_test_wifi_chan_loop(const char *param)
{
    int32_t ret;
    int32_t run_flag = 0;
    hcc_bus *old_bus = NULL;
#if defined(_PRE_PLAT_FEATURE_HI110X_PCIE) || defined(CONFIG_MMC)
    declare_time_cost_stru(cost);
    int32_t pkt_len = 1500;
#endif
    oal_reference(param);

    if (!wlan_is_shutdown()) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "wlan is open, test abort!");
        return -OAL_EBUSY;
    }

    old_bus = hcc_get_current_110x_bus();

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    if (oal_pcie_110x_working_check() == OAL_TRUE) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "PCIe Chan Test!");
        oal_get_time_cost_start(cost);
        ret = conn_test_wifi_chan_pcie(pkt_len);
        if (ret != OAL_SUCC) {
            (void)conn_test_wifi_chan_restore(old_bus);
            return ret;
        }
        oal_get_time_cost_end(cost);
        oal_calc_time_cost_sub(cost);
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie loop cost %llu us", time_cost_var_sub(cost));
        run_flag++;
    }
#endif

#ifdef CONFIG_MMC
    if (oal_sdio_110x_working_check() == OAL_TRUE) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "SDIO Chan Test!");
        oal_get_time_cost_start(cost);
        ret = conn_test_wifi_chan_sdio(pkt_len);
        if (ret != OAL_SUCC) {
            (void)conn_test_wifi_chan_restore(old_bus);
            return ret;
        }
        oal_get_time_cost_end(cost);
        oal_calc_time_cost_sub(cost);
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "sdio loop cost %llu us", time_cost_var_sub(cost));
        run_flag++;
    }
#endif

    ret = conn_test_wifi_chan_restore(old_bus);
    if (ret != OAL_SUCC) {
        return ret;
    }

    if (run_flag) {
        return 0;
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "no wifi chan is runing...");
        return -1;
    }
}
EXPORT_SYMBOL(conn_test_wifi_chan_loop);

/* Input, "sdio" or "pcie" */
int32_t conn_test_hcc_chann_switch(const char *new_dev)
{
    int32_t ret;
    if (oal_warn_on(new_dev == NULL)) {
        return -OAL_EINVAL;
    }
    if (!oal_strcmp("sdio", new_dev)) {
        ret = hcc_switch_bus(HCC_CHIP_110X_DEV, HCC_BUS_SDIO);
    } else if (!oal_strcmp("pcie", new_dev)) {
        ret = hcc_switch_bus(HCC_CHIP_110X_DEV, HCC_BUS_PCIE);
    } else {
        ret = -OAL_ENODEV;
    }

    return ret;
}
EXPORT_SYMBOL(conn_test_hcc_chann_switch);

void hcc_test_get_case(hcc_test_data *data)
{
    if (oal_warn_on(data == NULL)) {
        return;
    }
    hcc_test_throughput_gen();
    memcpy_s((void *)data, sizeof(hcc_test_data),
             (void *)&g_hcc_test_event->test_data, sizeof(hcc_test_data));
}

#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
OAL_STATIC int32_t hwifi_panic_handler(struct notifier_block *this, unsigned long event, void *unused)
{
    if (g_wifi_panic_debug) {
        hwifi_panic_log_dump(KERN_ERR);
    } else {
        printk(KERN_WARNING "wifi panic debug off\n");
    }

    return NOTIFY_OK;
}

OAL_STATIC struct notifier_block g_hwifi_panic_notifier = {
    .notifier_call = hwifi_panic_handler,
};
#endif

void hcc_test_kirin_noc_handler(void)
{
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    hcc_bus *pst_bus = NULL;

    g_hi11xx_kernel_crash = 1;

    pst_bus = hcc_get_current_110x_bus();
    if (pst_bus != NULL) {
        if (pst_bus->bus_type == HCC_BUS_PCIE) {
#if defined(_PRE_CONFIG_GPIO_TO_SSI_DEBUG)
            oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
            printk(KERN_ERR "hisi wifi noc dump\n");
            if ((pst_pci_lres != NULL) && (pst_pci_lres->pst_pci_res != NULL)) {
                printk(KERN_ERR "pcie link state:%s\n",
                       oal_pcie_get_link_state_str(pst_pci_lres->pst_pci_res->link_state));
            }

            printk(KERN_ERR "host wakeup dev gpio:%d\n", board_get_host_wakeup_dev_stat());
            (void)ssi_dump_err_regs(SSI_ERR_PCIE_KIRIN_NOC);
#else
            printk(KERN_ERR "ssi not support\n");
#endif
        }
    }
#endif
}

#if (defined(CONFIG_KIRIN_PCIE_NOC_NOTIFY))
typedef void (*func_pcie_noc_notify)(void);
extern func_pcie_noc_notify g_pcie_noc_notify;
#elif (defined(CONFIG_KIRIN_PCIE_NOC_DBG))
typedef void (*WIFI_DUMP_FUNC)(void);
void register_wifi_dump_func(WIFI_DUMP_FUNC func);
#else
#endif

int32_t hcc_test_init_module(struct hcc_handler *hcc)
{
    int32_t ret = OAL_SUCC;
    oal_kobject *pst_root_object = NULL;

    if (oal_unlikely(hcc == NULL)) {
        oal_warn_on(1);
        return OAL_FAIL;
    }

#if (defined(CONFIG_KIRIN_PCIE_NOC_NOTIFY))
    g_pcie_noc_notify = hcc_test_kirin_noc_handler;
#elif (defined(CONFIG_KIRIN_PCIE_NOC_DBG))
    register_wifi_dump_func(hcc_test_kirin_noc_handler);
#else
#endif

    pst_root_object = oal_get_sysfs_root_object();
    if (pst_root_object == NULL) {
        oal_io_print("[E]get root sysfs object failed!\n");
        return -OAL_EFAIL;
    }

    g_conn_syfs_hcc_object = kobject_create_and_add("hcc", pst_root_object);
    if (g_conn_syfs_hcc_object == NULL) {
        goto fail_g_conn_syfs_hcc_object;
    }

    ret = oal_debug_sysfs_create_group(g_conn_syfs_hcc_object, &g_hsdio_attribute_group);
    if (ret) {
        ret = -OAL_ENOMEM;
        oal_io_print("sysfs create sdio_sysfs_entries group fail.ret=%d\n", ret);
        goto fail_create_sdio_group;
    }

    /* create the files associated with this kobject */
    ret = oal_debug_sysfs_create_group(g_conn_syfs_hcc_object, &g_hcc_test_attribute_group);
    if (ret) {
        ret = -OAL_ENOMEM;
        oal_io_print("sysfs create test group fail.ret=%d\n", ret);
        goto fail_create_hcc_test_group;
    }

    ret = oal_debug_sysfs_create_group(g_conn_syfs_hcc_object, &g_hcc_attribute_group);
    if (ret) {
        oal_io_print("sysfs create hcc group fail.ret=%d\n", ret);
        ret = -OAL_ENOMEM;
        goto fail_create_hcc_group;
    }

    /* alloc memory for perf_action pointer */
    g_hcc_test_event = kzalloc(sizeof(*g_hcc_test_event), GFP_KERNEL);
    if (g_hcc_test_event == NULL) {
        oal_io_print("error kzalloc g_hcc_test_event mem.\n");
        ret = -OAL_ENOMEM;
        goto fail_g_hcc_test_event;
    }

    g_hcc_test_event->hcc_test_queue = DATA_LO_QUEUE;
    g_hcc_test_event->test_value = 0x5a;

    /* register callback for rx */
    ret = hcc_rx_register(hcc, HCC_ACTION_TYPE_TEST, hcc_test_rcvd, NULL);
    if (ret != OAL_SUCC) {
        oal_io_print("error %d register callback for rx.\n", ret);
        ret = -OAL_EFAIL;
        goto fail_rx_cb_register;
    }

    /* mutex lock init */
    mutex_init(&g_hcc_test_event->mutex_lock);

    oal_init_completion(&g_hcc_test_event->test_done);
    oal_init_completion(&g_hcc_test_event->test_trans_done);

    /* init hcc_test param */
    g_hcc_test_event->test_data.mode_idx = -1;
    g_hcc_test_event->test_data.pkt_len = 0;
    g_hcc_test_event->test_data.pkt_sent = 0;
    g_hcc_test_event->test_data.pkt_gen = 0;
    g_hcc_test_event->started = OAL_FALSE;

    /* create workqueue */
    g_hcc_test_event->test_workqueue = create_singlethread_workqueue("sdio_test");
    if (g_hcc_test_event == NULL) {
        oal_io_print("work queue create fail.\n");
        ret = -OAL_ENOMEM;
        goto fail_sdio_test_workqueue;
    }
    INIT_WORK(&g_hcc_test_event->test_work, hcc_test_work);

#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
    atomic_notifier_chain_register(&panic_notifier_list,
                                   &g_hwifi_panic_notifier);
#endif

#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
    if (oal_get_sdio_default_handler()) {
        hwifi_panic_log_register(&sdio_panic, oal_get_sdio_default_handler());
    }
    hwifi_panic_log_register(&hcc_panic_assem_info, hcc);
    hwifi_panic_log_register(&hcc_panic_flowctrl, hcc);

    hwifi_panic_log_register(&hcc_panic_queues_len, hcc);
    hwifi_panic_log_register(&hcc_panic_queues_pkts, hcc);
#endif
    return ret;
fail_sdio_test_workqueue:
    hcc_rx_unregister(hcc, HCC_ACTION_TYPE_TEST);
fail_rx_cb_register:
    kfree(g_hcc_test_event);
    g_hcc_test_event = NULL;
fail_g_hcc_test_event:
    oal_debug_sysfs_remove_group(g_conn_syfs_hcc_object, &g_hcc_attribute_group);
fail_create_hcc_group:
    oal_debug_sysfs_remove_group(g_conn_syfs_hcc_object, &g_hcc_test_attribute_group);
fail_create_hcc_test_group:
    oal_debug_sysfs_remove_group(g_conn_syfs_hcc_object, &g_hsdio_attribute_group);
fail_create_sdio_group:
    kobject_put(g_conn_syfs_hcc_object);
fail_g_conn_syfs_hcc_object:
    return ret;
}

void hcc_test_exit_module(struct hcc_handler *hcc)
{
#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
    atomic_notifier_chain_unregister(&panic_notifier_list,
                                     &g_hwifi_panic_notifier);
#endif

    if (g_hcc_test_event->test_workqueue != NULL) {
        destroy_workqueue(g_hcc_test_event->test_workqueue);
        g_hcc_test_event->test_workqueue = NULL;
    }
    oal_debug_sysfs_remove_group(g_conn_syfs_hcc_object, &g_hcc_attribute_group);
    oal_debug_sysfs_remove_group(g_conn_syfs_hcc_object, &g_hcc_test_attribute_group);
    oal_debug_sysfs_remove_group(g_conn_syfs_hcc_object, &g_hsdio_attribute_group);
    kobject_put(g_conn_syfs_hcc_object);
    hcc_rx_unregister(hcc, HCC_ACTION_TYPE_TEST);

#if (defined(CONFIG_KIRIN_PCIE_NOC_NOTIFY))
    g_pcie_noc_notify = NULL;
#elif (defined(CONFIG_KIRIN_PCIE_NOC_DBG))
    register_wifi_dump_func(NULL);
#else
#endif

    kfree(g_hcc_test_event);
    g_hcc_test_event = NULL;
}
#endif
