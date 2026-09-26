

#define HI11XX_LOG_MODULE_NAME     "[HCC]"
#define HI11XX_LOG_MODULE_NAME_VAR hcc_loglevel
#define HISI_LOG_TAG               "[HCC]"
#include "oal_util.h"
#include "oal_sdio_host_if.h"
#include "pcie_host.h"
#include "pcie_linux.h"
#include "oal_hcc_host_if.h"
#include "oam_ext_if.h"
#include "securec.h"

#ifdef _PRE_WLAN_PKT_TIME_STAT
#include <hwnet/ipv4/wifi_delayst.h>
#endif

#ifdef CONFIG_MMC
#include "plat_pm_wlan.h"
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/signal.h>
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_HCC_HOST_C

#define HCC_TX_FLOW_HI_LEVEL 512
#define HCC_TX_FLOW_LO_LEVEL 128

#define HCC_TRANS_THREAD_POLICY   SCHED_FIFO
#define HCC_TRANS_THERAD_PRIORITY 10
#define HCC_TRANS_THERAD_NICE     (-10)

#define HCC_QUEUE_FC_HIGH_WATER_DROP_LINE 100

OAL_STATIC struct hcc_handler *g_hcc_tc = NULL;

OAL_STATIC uint32_t g_hcc_tx_max_buf_len = 4096;

uint32_t g_tcp_ack_wait_sch_cnt = 1;

struct custom_process_func_handler g_custom_process_func;
EXPORT_SYMBOL_GPL(g_custom_process_func);  //lint !e132 !e745 !e578

/* use lint -e16 to mask the error19! */
pm_wlan_pkt_statis_stru g_pm_wlan_pkt_statis = { 0 };
/*lint -e19 */
oal_module_symbol(g_pm_wlan_pkt_statis);
/*lint +e19 */
uint32_t g_hcc_assemble_count = 16;
/*lint -e19 */
oal_module_symbol(g_hcc_assemble_count);
/*lint +e19 */
/* 1 means hcc rx data process in hcc thread, 0 means process in sdio thread */
uint32_t g_hcc_rx_thread_enable = 1;
uint32_t g_hcc_credit_bottom_value = 2;
uint32_t g_hcc_flowctrl_detect_panic = 0;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_debug_module_param(g_hcc_tx_max_buf_len, uint, S_IRUGO | S_IWUSR);
oal_debug_module_param(g_hcc_assemble_count, uint, S_IRUGO | S_IWUSR);
oal_debug_module_param(g_hcc_rx_thread_enable, uint, S_IRUGO | S_IWUSR);
oal_debug_module_param(g_hcc_credit_bottom_value, uint, S_IRUGO | S_IWUSR);
oal_debug_module_param(g_hcc_flowctrl_detect_panic, uint, S_IRUGO | S_IWUSR);
#endif
int32_t hcc_send_rx_queue(struct hcc_handler *hcc, hcc_queue_type type);
uint32_t hcc_queues_flow_ctrl_len(struct hcc_handler *hcc, hcc_chan_type dir);
OAL_STATIC void hcc_dev_flowctr_timer_del(struct hcc_handler *hcc);

oal_bool_enum_uint8 hcc_flowctl_get_device_mode(struct hcc_handler *hcc);
OAL_STATIC void hcc_tx_network_start_subq(struct hcc_handler *hcc, uint16_t us_queue_idx);
OAL_STATIC void hcc_tx_network_stop_subq(struct hcc_handler *hcc, uint16_t us_queue_idx);

int32_t hcc_set_hi11xx_loglevel(int32_t loglevel)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    int32_t ret = HI11XX_LOG_MODULE_NAME_VAR;
    HI11XX_LOG_MODULE_NAME_VAR = loglevel;
    return ret;
#else
    return 0;
#endif
}

int32_t hcc_set_all_loglevel(int32_t loglevel)
{
    int32_t ret = hcc_set_hi11xx_loglevel(loglevel);
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    oal_pcie_set_loglevel(loglevel);
    oal_pcie_set_hi11xx_loglevel(loglevel);
#endif
    return ret;
}

uint32_t hcc_get_max_buf_len(void)
{
    return g_hcc_tx_max_buf_len;
}

/* This is not good */
struct hcc_handler *hcc_get_110x_handler(void)
{
    hcc_bus_dev *pst_bus_dev = hcc_get_bus_dev(HCC_CHIP_110X_DEV);
    if (oal_unlikely(pst_bus_dev == NULL)) {
        return NULL;
    }

    return pst_bus_dev->hcc;
}
/*lint -e19 */
oal_module_symbol(hcc_get_110x_handler);
/*lint +e19 */
OAL_STATIC void hcc_tx_netbuf_destory(struct hcc_handler *hcc)
{
    if (oal_warn_on(hcc == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: hcc is null", __FUNCTION__);
        return;
    };

    oal_wake_unlock(&hcc->tx_wake_lock);
}

struct custom_process_func_handler *oal_get_custom_process_func(void)
{
    return &g_custom_process_func;
}
/*lint -e19 */
oal_module_symbol(oal_get_custom_process_func);
/*lint +e19 */

OAL_STATIC oal_netbuf_stru *hcc_tx_assem_descr_get(struct hcc_handler *hcc)
{
    return oal_netbuf_delist(&hcc->tx_descr_info.tx_assem_descr_hdr);
}

OAL_STATIC void hcc_tx_assem_descr_put(struct hcc_handler *hcc, oal_netbuf_stru *netbuf)
{
    oal_netbuf_list_tail(&hcc->tx_descr_info.tx_assem_descr_hdr, netbuf);
}

void hcc_clear_next_assem_descr(struct hcc_handler *hcc,
                                oal_netbuf_stru *descr_netbuf)
{
    oal_reference(hcc);
    /* Just need clear the first bytes */
    *(uint8_t *)oal_netbuf_data(descr_netbuf) = 0;
}

#ifdef CONFIG_MMC
uint32_t hcc_get_large_pkt_free_cnt(struct hcc_handler *hcc)
{
    return oal_sdio_get_large_pkt_free_cnt(oal_get_sdio_default_handler());
}
#endif

/* align_size must be power of 2 */
OAL_STATIC oal_netbuf_stru *hcc_netbuf_len_align(oal_netbuf_stru *netbuf,
                                                 uint32_t align_size)
{
    int32_t ret;
    uint32_t len_algin, tail_room_len;
    uint32_t len = oal_netbuf_len(netbuf);
    if (oal_is_aligned(len, align_size)) {
        return netbuf;
    }
    /* align the netbuf */
    len_algin = OAL_ROUND_UP(len, align_size);
    if (oal_unlikely(len_algin < len)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: len_algin:%d < len:%d", __FUNCTION__, len_algin, len);
        return NULL;
    }

    tail_room_len = len_algin - len;
    if (oal_unlikely(tail_room_len > oal_netbuf_tailroom(netbuf))) {
        /* tailroom not enough */
        ret = oal_netbuf_expand_head(netbuf, 0, (int32_t)tail_room_len, GFP_KERNEL);
        if (oal_warn_on(ret != OAL_SUCC)) {
            oal_print_hi11xx_log(HI11XX_LOG_WARN, "alloc head room failed,expand tail len=%d", tail_room_len);
            declare_dft_trace_key_info("netbuf align expand head fail", OAL_DFT_TRACE_FAIL);
            return NULL;
        }
    }

    oal_netbuf_put(netbuf, tail_room_len);

    if (oal_unlikely(!oal_is_aligned(oal_netbuf_len(netbuf), align_size))) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: netbuf len :%d not align %d",
                             __FUNCTION__, oal_netbuf_len(netbuf), align_size);
        return NULL;
    };

    return netbuf;
}

OAL_STATIC void hcc_build_next_assem_descr(struct hcc_handler *hcc,
                                           hcc_queue_type type,
                                           oal_netbuf_head_stru *head,
                                           oal_netbuf_head_stru *next_assembled_head,
                                           oal_netbuf_stru *descr_netbuf,
                                           uint32_t remain_len)
{
    int32_t i = 0;
    int32_t len;
    uint8_t *buf = NULL;
    oal_netbuf_stru *netbuf = NULL;
    oal_netbuf_stru *netbuf_t = NULL;
    uint32_t assemble_max_count, queue_len, current_trans_len;

    buf = (uint8_t *)oal_netbuf_data(descr_netbuf);
    len = (int32_t)oal_netbuf_len(descr_netbuf);

    if (oal_unlikely(!oal_netbuf_list_empty(next_assembled_head))) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: next_assembled_head is empty", __FUNCTION__);
        return;
    };

    assemble_max_count = oal_max(1, g_hcc_assemble_count);
    queue_len = oal_netbuf_list_len(head);
    current_trans_len = oal_min(queue_len, assemble_max_count);
    current_trans_len = oal_min(current_trans_len, remain_len);

    oal_print_hi11xx_log(HI11XX_LOG_DBG,
                         "build next descr, queue:[remain_len:%u][len:%u][trans_len:%u][max_assem_len:%u]",
                         remain_len, queue_len, current_trans_len, assemble_max_count);

    buf[0] = 0;

    if (current_trans_len == 0) {
        return;
    }

    forever_loop() {
        /* move the netbuf from head queue to prepare-send queue, head->tail */
        netbuf = oal_netbuf_delist(head);
        if (netbuf == NULL) {
            oal_print_hi11xx_log(HI11XX_LOG_WARN, "why? this should never happaned! assem list len:%d",
                                 oal_netbuf_list_len(next_assembled_head));
            declare_dft_trace_key_info("buid assem error", OAL_DFT_TRACE_FAIL);
            break;
        }

        /* align the buff len to 32B */
        netbuf_t = hcc_netbuf_len_align(netbuf, HISDIO_H2D_SCATT_BUFFLEN_ALIGN);
        if (netbuf_t == NULL) {
            /* return to the list */
            oal_netbuf_addlist(head, netbuf);
            break;
        }

        current_trans_len--;

        oal_netbuf_list_tail(next_assembled_head, netbuf_t);
        if (oal_unlikely(i >= len)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "hcc tx scatt num :%d over buff len:%d,assem count:%u",
                                 i, len, g_hcc_assemble_count);
            break;
        }

        buf[i++] = (uint8_t)(oal_netbuf_len(netbuf) >> HISDIO_H2D_SCATT_BUFFLEN_ALIGN_BITS);
        if (current_trans_len == 0) {
            /* send empty */
            if (i != len) {
                buf[i] = 0;
            }
            break;
        }
    }
#ifdef _PRE_COMMENT_CODE_
    oal_print_hex_dump((uint8_t *)buf, HISDIO_HOST2DEV_SCATT_SIZE, HEX_DUMP_GROUP_SIZE, "h2d assem info");
#endif
    if (oal_likely(!oal_netbuf_list_empty(next_assembled_head))) {
        hcc->hcc_transer_info.tx_assem_info.assembled_queue_type = type;
    }
}

/*
 * Prototype    : hcc_tx_param_check
 * Description  : check tx param
 */
OAL_STATIC int32_t hcc_tx_param_check(struct hcc_handler *hcc,
                                      struct hcc_transfer_param *param)
{
    if (oal_unlikely(param->extend_len > hcc->hdr_rever_max_len)) {
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        WARN(1, "invaild reserved len %u ,never should over %zu\n",
             param->extend_len, HCC_HDR_RESERVED_MAX_LEN);
#endif
        /* can't free buf here */
        return -OAL_EINVAL;
    }

    if (oal_warn_on(param->main_type >= HCC_ACTION_TYPE_BUTT)) {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "wrong main type:%d", param->main_type);
        return -OAL_EINVAL;
    }

    if (oal_warn_on(param->fc_flag & (~HCC_FC_ALL))) {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "wrong fc_flag:%d", param->fc_flag);
        return -OAL_EINVAL;
    }

    if (oal_warn_on(param->queue_id >= HCC_QUEUE_COUNT)) {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "wrong queue_id:%d", param->queue_id);
        return -OAL_EINVAL;
    }
    return OAL_SUCC;
}

void hcc_tx_network_stopall_queues(struct hcc_handler *hcc)
{
    if (oal_warn_on(hcc == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: hcc is null", __FUNCTION__);
        return;
    }
    if (oal_likely(hcc->hcc_transer_info.tx_flow_ctrl.net_stopall)) {
        hcc->hcc_transer_info.tx_flow_ctrl.net_stopall();
    }
}

void hcc_tx_network_startall_queues(struct hcc_handler *hcc)
{
    if (oal_warn_on(hcc == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: hcc is null", __FUNCTION__);
        return;
    }

    if (oal_likely(hcc->hcc_transer_info.tx_flow_ctrl.net_startall)) {
        hcc->hcc_transer_info.tx_flow_ctrl.net_startall();
    }
}

void hcc_tx_flow_ctrl_cb_register(flowctrl_cb stopall, flowctrl_cb startall)
{
    if (g_hcc_tc == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[ERROR]hcc_tx_flow_ctrl_cb_register failed! g_hcc_tc is NULL");
        return;
    }
    g_hcc_tc->hcc_transer_info.tx_flow_ctrl.net_stopall = stopall;
    g_hcc_tc->hcc_transer_info.tx_flow_ctrl.net_startall = startall;
}

/*lint -e19 */
oal_module_symbol(hcc_tx_flow_ctrl_cb_register);
/*lint +e19 */
void hcc_tx_wlan_queue_map_set(struct hcc_handler *hcc, hcc_queue_type hcc_queue_id,
                               wlan_net_queue_type wlan_queue_id)
{
    hcc_trans_queue *pst_hcc_queue = NULL;
    if (oal_warn_on(hcc == NULL)) {
        return;
    }

    if (oal_warn_on(hcc_queue_id >= HCC_QUEUE_COUNT || wlan_queue_id >= WLAN_NET_QUEUE_BUTT)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "invaild param,hcc id:%d,wlan id:%d",
                             (int32_t)hcc_queue_id, (int32_t)wlan_queue_id);
        return;
    }
    pst_hcc_queue = &hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[hcc_queue_id];
    pst_hcc_queue->wlan_queue_id = wlan_queue_id;
}

/*lint -e19 */
oal_module_symbol(hcc_tx_wlan_queue_map_set);
/*lint +e19 */
OAL_STATIC uint32_t hcc_check_header_vaild(struct hcc_header *hdr)
{
    if (oal_unlikely(hdr->main_type >= HCC_ACTION_TYPE_BUTT) ||
        (HCC_HDR_LEN + hdr->pad_hdr + hdr->pad_payload > HCC_HDR_TOTAL_LEN)) {
        declare_dft_trace_key_info("hcc_check_header_vaild_fail", OAL_DFT_TRACE_FAIL);
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

static inline int32_t hcc_tx_para_check(struct hcc_handler *hcc, oal_netbuf_stru *netbuf,
                                        struct hcc_transfer_param *param)
{
    if (oal_warn_on(hcc == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "HCC is null!\n");
        return -OAL_EINVAL;
    }

    if (oal_warn_on(netbuf == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "netbuf is null!\n");
        return -OAL_EINVAL;
    }

    if (oal_warn_on(param == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "param is null!\n");
        return -OAL_EINVAL;
    }

    /*lint -e522*/
    oal_warn_on(oal_netbuf_len(netbuf) == 0);
    /*lint +e522*/

    return OAL_SUCC;
}

static int32_t hcc_tx_cond_check(struct hcc_handler *hcc)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (oal_unlikely(oal_atomic_read(&hcc->state) != HCC_ON)) {
        if (oal_atomic_read(&hcc->state) == HCC_OFF) {
            return -OAL_EBUSY;
        } else if (oal_atomic_read(&hcc->state) == HCC_EXCEPTION) {
            return -OAL_EIO;
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "invaild hcc state:%d", oal_atomic_read(&hcc->state));
            return -OAL_EINVAL;
        }
    }
#endif
    return OAL_SUCC;
}

static int32_t hcc_tx_flowctrl_process(struct hcc_handler *hcc, hcc_trans_queue *pst_hcc_queue,
                                       oal_netbuf_stru *netbuf,
                                       struct hcc_transfer_param *param)
{
    int32_t ret;
    /* flowctrl: 1. sched thread fc pause 2.interrupt  3. drop package */
    if (oal_warn_on((param->fc_flag & HCC_FC_WAIT) && oal_in_interrupt())) {
        /* when in interrupt,can't sched! */
        param->fc_flag &= ~HCC_FC_WAIT;
    }

    if (param->fc_flag) {
        /* flow control process */
        /* Block if fc */
        if (HCC_FC_WAIT & param->fc_flag) {
            /*lint -e730*/
            ret = oal_wait_event_interruptible_timeout_m(hcc->hcc_transer_info.tx_flow_ctrl.wait_queue,
                                                         (oal_netbuf_list_len(&pst_hcc_queue->data_queue) <
                                                          pst_hcc_queue->flow_ctrl.low_waterline),
                                                         OAL_SEC_PER_MIN * OAL_TIME_HZ);
            if (ret == 0) {
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "[WARN]hcc flow control wait event timeout! too much time locked");
                declare_dft_trace_key_info("hcc flow control wait timeout", OAL_DFT_TRACE_FAIL);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
                hcc_print_current_trans_info(0);
#endif
            } else if (ret == -ERESTARTSYS) {
                oal_print_hi11xx_log(HI11XX_LOG_WARN, "wifi task was interrupted by a signal");
                return -OAL_EFAIL;
            }
        }

        /* control net layer if fc */
        if (HCC_FC_NET & param->fc_flag) {
            /* net layer */
            oal_spin_lock_bh(&hcc->hcc_transer_info.tx_flow_ctrl.lock);
            if ((oal_netbuf_list_len(&pst_hcc_queue->data_queue) > pst_hcc_queue->flow_ctrl.high_waterline) &&
                (pst_hcc_queue->flow_ctrl.is_stopped == OAL_FALSE)) {
                hcc_tx_network_stop_subq(hcc, (uint16_t)pst_hcc_queue->wlan_queue_id);
                pst_hcc_queue->flow_ctrl.is_stopped = OAL_TRUE;
            }
            oal_spin_unlock_bh(&hcc->hcc_transer_info.tx_flow_ctrl.lock);
        }

        /* control net layer if fc */
        if (HCC_FC_DROP & param->fc_flag) {
            /* 10 netbufs to buff */
            if (oal_netbuf_list_len(&pst_hcc_queue->data_queue) >
                pst_hcc_queue->flow_ctrl.high_waterline + HCC_QUEUE_FC_HIGH_WATER_DROP_LINE) {
                pst_hcc_queue->loss_pkts++;
                /* drop the netbuf */
                return -OAL_EBUSY;
            }
        }
    }

    return OAL_SUCC;
}

#ifdef CONFIG_HCC_DEBUG
static inline void hcc_tx_rebuild_netbuf_print_info(oal_netbuf_stru* netbuf, struct hcc_header *hdr)
{
    oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "hcc tx pkt seq:%d", hdr->seq);
    oal_print_hex_dump(oal_netbuf_data(netbuf), HCC_HDR_TOTAL_LEN, HEX_DUMP_GROUP_SIZE, "hcc hdr ");
    oal_print_hex_dump(oal_netbuf_data(netbuf) + HCC_HDR_TOTAL_LEN + hdr->pad_payload,
                       hdr->pay_len, HEX_DUMP_GROUP_SIZE, "hcc payload ");
}
#endif

static inline int32_t hcc_tx_rebuild_netbuf_expand_head(oal_netbuf_stru *netbuf, int32_t headroom_add,
                                                        uint8_t **old_data_addr)
{
    int32_t ret;
    if (headroom_add) {
        /* relloc the netbuf */
        ret = oal_netbuf_expand_head(netbuf, headroom_add, 0, GFP_ATOMIC);
        if (oal_unlikely(ret != OAL_SUCC)) {
            oal_print_hi11xx_log(HI11XX_LOG_WARN, "alloc head room failed,netbuf len is %d,expand len:%d\n",
                                 oal_netbuf_len(netbuf), headroom_add);
            return -OAL_EFAIL;
        }
        *old_data_addr = oal_netbuf_data(netbuf);
    }

    return OAL_SUCC;
}

#define hcc_tx_rebuild_netbuf_fill_hdr(hcc, hdr, param, pad_payload,  payload_len, pad_hdr)  \
        do {                                                                  \
                (hdr)->pad_payload = (uint16_t)(pad_payload);                     \
                (hdr)->pay_len = (uint16_t)(payload_len);                         \
                (hdr)->seq = ((uint32_t)(oal_atomic_inc_return(&((hcc)->tx_seq)))) & 0xFF; \
                (hdr)->main_type = (uint8_t)(param)->main_type;                   \
                (hdr)->sub_type = (uint8_t)(param)->sub_type;                     \
                (hdr)->pad_hdr = (uint8_t)(pad_hdr);                              \
                (hdr)->more = 0;                                                \
                (hdr)->option = 0;                                              \
        } while (0)

static int32_t hcc_tx_rebuild_netbuf_memmove_payload(struct hcc_handler *hcc, struct hcc_header *hdr,
                                                     oal_netbuf_stru *netbuf, struct hcc_transfer_param *param)
{
    if (oal_warn_on(hdr->pay_len > g_hcc_tx_max_buf_len)) {
        /* pay_len超过DEVICE 最大内存长度 */
        oal_print_hi11xx_log(HI11XX_LOG_ERR,
                             "[ERROR]main:%d, sub:%d,pay len:%d,netbuf len:%d, extend len:%d,pad_payload:%d,max len:%u",
                             hdr->main_type,
                             hdr->sub_type,
                             hdr->pay_len,
                             hdr->pay_len + param->extend_len,
                             param->extend_len,
                             hdr->pad_payload,
                             g_hcc_tx_max_buf_len);
        declare_dft_trace_key_info("hcc_tx_check_param_fail", OAL_DFT_TRACE_FAIL);
        return -OAL_EINVAL;
    } else {
        /* 当长度 在1544 + [1~3] 之内， 牺牲性能,将payload 内存前移 1~3B，节省DEVICE内存 */
        if (hdr->pad_payload + hdr->pay_len > g_hcc_tx_max_buf_len) {
            uint8_t *pst_dst = (uint8_t *)hdr + HCC_HDR_TOTAL_LEN;
            uint8_t *pst_src = pst_dst + hdr->pad_payload;
            if (memmove_s((void *)pst_dst, hdr->pay_len + hdr->pad_payload,
                          (const void *)pst_src, hdr->pay_len) != EOK) {
                oal_print_hi11xx_log(HI11XX_LOG_ERR,
                                     "memmove_s: move the payload forward failed.");
            }
            oal_netbuf_trim(netbuf, hdr->pad_payload);
            hdr->pad_payload = 0; /* after memmove,the pad is 0 */
            declare_dft_trace_key_info("hcc_tx_check_memmove", OAL_DFT_TRACE_SUCC);
        }
    }

    return OAL_SUCC;
}

static int32_t hcc_tx_rebuild_netbuf(struct hcc_handler *hcc,
                                     oal_netbuf_stru *netbuf, struct hcc_transfer_param *param)
{
    int32_t ret;
    oal_uint pad_payload, headroom,
             payload_len, payload_addr,
             pad_hdr;
    uint8_t *old_data_addr = NULL;
    struct hcc_header *hdr = NULL;
    int32_t headroom_add = 0;

    /* head : 64B + 4B , tail : 512 */
    /* only payload data */
    payload_len = oal_netbuf_len(netbuf) - param->extend_len;

    old_data_addr = oal_netbuf_data(netbuf);
    payload_addr = (uintptr_t)oal_netbuf_data(netbuf) + param->extend_len;

    /* if pad not 0, we must copy the extend data */
    pad_payload = payload_addr - OAL_ROUND_DOWN(payload_addr, 4); /* 清除低2bit，保证4字节对齐 */
    /* should be 1 byte */
    pad_hdr = HCC_HDR_RESERVED_MAX_LEN - param->extend_len;

    headroom = HCC_HDR_LEN + pad_hdr + pad_payload;
    if (headroom > oal_netbuf_headroom(netbuf)) {
        headroom_add = (int32_t)(headroom - oal_netbuf_headroom(netbuf));
    }

    ret = hcc_tx_rebuild_netbuf_expand_head(netbuf, headroom_add, &old_data_addr);
    if (ret != OAL_SUCC) {
        return ret;
    }

    hdr = (struct hcc_header *)oal_netbuf_push(netbuf, headroom);
    if (oal_unlikely(!oal_is_aligned((uintptr_t)hdr, sizeof(uint32_t)))) { /* 判断是不是4字节对齐 */
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hdr:%ld not aligned len:%d", (uintptr_t)hdr, (int)sizeof(uint32_t));
        return -OAL_EFAIL;
    }

    if (pad_payload > 0) {
        /* algin the extend data */
        if (memmove_s(old_data_addr - pad_payload, pad_payload + payload_len + param->extend_len,
                      old_data_addr, param->extend_len) != EOK) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "memmove_s: align the extend data failed.");
        }
    }

    hcc_tx_rebuild_netbuf_fill_hdr(hcc, hdr, param, pad_payload, payload_len, pad_hdr);

    if (oal_unlikely(hdr->pad_hdr + HCC_HDR_LEN > HCC_HDR_TOTAL_LEN)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hdr->pad_hdr + HCC_HDR_LEN:%ld > len:%d",
                             hdr->pad_hdr + HCC_HDR_LEN, HCC_HDR_TOTAL_LEN);
        return -OAL_EFAIL;
    }

    ret = hcc_tx_rebuild_netbuf_memmove_payload(hcc, hdr, netbuf, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }

    if (oal_warn_on(OAL_ROUND_UP(oal_netbuf_len(netbuf), HISDIO_H2D_SCATT_BUFFLEN_ALIGN)
                    > g_hcc_tx_max_buf_len + HCC_HDR_TOTAL_LEN)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[E]Fatal error,netbuf's len:%d over the payload:%d",
                             OAL_ROUND_UP(oal_netbuf_len(netbuf), HISDIO_H2D_SCATT_BUFFLEN_ALIGN),
                             g_hcc_tx_max_buf_len + HCC_HDR_TOTAL_LEN);
        declare_dft_trace_key_info("hcc_tx_check_algin_fail", OAL_DFT_TRACE_FAIL);
        return -OAL_EINVAL;
    }

#ifdef CONFIG_HCC_DEBUG
    hcc_tx_rebuild_netbuf_print_info(netbuf, hdr);
#endif

    return OAL_SUCC;
}

/*
 * Prototype    : hcc_tx
 * Description  : hcc tx function, the format like below:
 * Return Value : succ or fail
 */
int32_t hcc_tx(struct hcc_handler *hcc, oal_netbuf_stru *netbuf,
               struct hcc_transfer_param *param)
{
    uint32_t queue_id;
    int32_t ret;
    hcc_trans_queue *pst_hcc_queue = NULL;
    struct hcc_tx_cb_stru *pst_cb_stru = NULL;

    ret = hcc_tx_para_check(hcc, netbuf, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }

    ret = hcc_tx_cond_check(hcc);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }

    if (oal_unlikely(hcc_tx_param_check(hcc, param) != OAL_SUCC)) {
        return -OAL_EINVAL;
    }

    queue_id = param->queue_id;
    pst_hcc_queue = &hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[queue_id];

    ret = hcc_tx_flowctrl_process(hcc, pst_hcc_queue, netbuf, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        if (oal_unlikely(ret == -OAL_EBUSY)) {
            // drop netbuf
            oal_netbuf_free(netbuf);
            return OAL_SUCC;
        } else {
            return ret;
        }
    }

    ret = hcc_tx_rebuild_netbuf(hcc, netbuf, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }

#ifdef CONFIG_HCC_DEBUG
    oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "hcc_tx into queue:%d, main:%d, sub:%d",
                         queue_id, param->main_type, param->sub_type);
#endif

    /* wakelock,one netbuf one lock */
    oal_wake_lock(&hcc->tx_wake_lock);

    pst_cb_stru = (struct hcc_tx_cb_stru *)oal_netbuf_cb(netbuf);
    pst_cb_stru->destory = hcc_tx_netbuf_destory;
    pst_cb_stru->magic = HCC_TX_WAKELOCK_MAGIC;
    pst_cb_stru->qtype = queue_id;

    oal_netbuf_list_tail(&pst_hcc_queue->data_queue, netbuf);

    hcc_sched_transfer(hcc);

    return OAL_SUCC;
}

/*lint -e19 */
oal_module_symbol(hcc_tx); /*lint +e19 */

OAL_STATIC void hcc_transfer_rx_handler(struct hcc_handler *hcc, oal_netbuf_stru *netbuf)
{
    /* get the rx buf and enqueue */
    oal_netbuf_list_tail(&hcc->hcc_transer_info.hcc_queues[HCC_RX].queues[DATA_LO_QUEUE].data_queue,
                         netbuf);
    if (g_hcc_rx_thread_enable == 0) {
        hcc_send_rx_queue(hcc, DATA_LO_QUEUE);
    }
}

OAL_STATIC void hcc_transfer_rx_handler_replace(struct hcc_handler *hcc, oal_netbuf_stru *pst_netbuf_new)
{
    oal_netbuf_stru *pst_netbuf_old;
    hcc_trans_queue *pst_hcc_queue = &hcc->hcc_transer_info.hcc_queues[HCC_RX].queues[DATA_LO_QUEUE];

    /* remove from head */
    pst_netbuf_old = oal_netbuf_delist(&pst_hcc_queue->data_queue);
    if (oal_likely(pst_netbuf_old != NULL)) {
        pst_hcc_queue->loss_pkts++;
        if (oal_print_rate_limit(PRINT_RATE_SECOND)) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "hcc_rx loss pkts :%u", pst_hcc_queue->loss_pkts);
        }
        oal_netbuf_free(pst_netbuf_old);
    }

    /* get the rx buf and enqueue */
    oal_netbuf_list_tail(&pst_hcc_queue->data_queue,
                         pst_netbuf_new);

    if (g_hcc_rx_thread_enable == 0) {
        hcc_send_rx_queue(hcc, DATA_LO_QUEUE);
    }
}

/*
 * Prototype    : oal_hcc_bus_rx_handler
 * Description  : hcc tx function, the format like below:
 * Return Value : succ or fail
 */
int32_t hcc_bus_rx_handler(void *data)
{
    int32_t ret;
    uint32_t scatt_count;
    oal_netbuf_stru *netbuf = NULL;
    oal_netbuf_head_stru head;
    struct hcc_handler *hcc = (struct hcc_handler *)data;
    struct hcc_header *pst_hcc_head = NULL;

    if (oal_warn_on(hcc == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hcc is null:%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    oal_netbuf_head_init(&head);

    /* 调用bus底层的实体处理接口获取netbuf list */
    ret = hcc_bus_rx_netbuf_list(hcc_to_bus(hcc), &head);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }

    scatt_count = oal_netbuf_list_len(&head);
    if (oal_unlikely(scatt_count >= HCC_RX_ASSEM_INFO_MAX_NUM)) {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "WARN:why so much scatt buffs[%u]?", scatt_count);
        scatt_count = 0;
    }

    hcc->hcc_transer_info.rx_assem_info.info[scatt_count]++;

    forever_loop() {
        netbuf = oal_netbuf_delist(&head);
        if (netbuf == NULL) {
            break;
        }

        /* RX 流控，当接收来不及处理时丢掉最旧的数据包,SDIO不去读DEVICE侧会堵住 */
        pst_hcc_head = (struct hcc_header *)oal_netbuf_data(netbuf);
        if (oal_unlikely(hcc_check_header_vaild(pst_hcc_head) != OAL_TRUE)) {
            oal_print_hex_dump((uint8_t *)pst_hcc_head, HCC_HDR_TOTAL_LEN, HEX_DUMP_GROUP_SIZE,
                               "invaild hcc header: ");
            hcc_bus_try_to_dump_device_mem(hcc_get_current_110x_bus(), OAL_FALSE);
        }

        if (hcc->hcc_transer_info.hcc_queues[HCC_RX].queues[DATA_LO_QUEUE].flow_ctrl.enable == OAL_TRUE) {
            if (oal_netbuf_list_len(&hcc->hcc_transer_info.hcc_queues[HCC_RX].queues[DATA_LO_QUEUE].data_queue)
                > hcc->hcc_transer_info.hcc_queues[HCC_RX].queues[DATA_LO_QUEUE].flow_ctrl.high_waterline) {
                hcc_transfer_rx_handler_replace(hcc, netbuf);
            } else {
                hcc_transfer_rx_handler(hcc, netbuf);
            }
        } else {
            hcc_transfer_rx_handler(hcc, netbuf);
        }
    }

    /* sched hcc thread */
    if (g_hcc_rx_thread_enable == 1) {
        hcc_sched_transfer(hcc);
    }

    return OAL_SUCC;
}

void hcc_rx_submit(struct hcc_handler *hcc, oal_netbuf_stru *pst_netbuf)
{
    if (oal_unlikely((hcc == NULL) || (pst_netbuf == NULL))) {
        oal_warn_on(1);
        return;
    }
#ifdef CONFIG_HCC_DEBUG
    {
        struct hcc_header *hdr = (struct hcc_header *)oal_netbuf_data(pst_netbuf);
        oal_io_print("[WIFI]hcc rx pkt seq enqueue:%d\n", hdr->seq);
    }
#endif
    oal_netbuf_list_tail(&hcc->hcc_transer_info.hcc_queues[HCC_RX].queues[DATA_LO_QUEUE].data_queue,
                         pst_netbuf);
}

OAL_STATIC hcc_netbuf_queue_type g_hcc_queue_to_netbuf_qmap[HCC_QUEUE_COUNT] = {
    [CTRL_QUEUE] = HCC_NETBUF_HIGH_QUEUE,
    [DATA_HI_QUEUE] = HCC_NETBUF_HIGH_QUEUE,
    [DATA_HI_QUEUE2] = HCC_NETBUF_HIGH2_QUEUE,
    [DATA_LO_QUEUE] = HCC_NETBUF_NORMAL_QUEUE,
    [DATA_TCP_DATA_QUEUE] = HCC_NETBUF_NORMAL_QUEUE,
    [DATA_TCP_ACK_QUEUE] = HCC_NETBUF_NORMAL_QUEUE,
    [DATA_UDP_BK_QUEUE] = HCC_NETBUF_NORMAL_QUEUE,
    [DATA_UDP_BE_QUEUE] = HCC_NETBUF_NORMAL_QUEUE,
    [DATA_UDP_VI_QUEUE] = HCC_NETBUF_NORMAL_QUEUE,
    [DATA_UDP_VO_QUEUE] = HCC_NETBUF_NORMAL_QUEUE,
};

OAL_STATIC OAL_INLINE hcc_netbuf_queue_type hcc_queue_map_to_netbuf_queue(hcc_queue_type qtype)
{
    return g_hcc_queue_to_netbuf_qmap[qtype];
}

OAL_STATIC int32_t _queues_not_flowctrl_len_check(struct hcc_handler *hcc,
                                                  hcc_chan_type dir)
{
    int32_t i;
    for (i = 0; i < HCC_QUEUE_COUNT; i++) {
        if (hcc->hcc_transer_info.hcc_queues[dir].queues[i].flow_ctrl.enable != OAL_TRUE &&
            hcc->hcc_transer_info.hcc_queues[dir].queues[i].flow_ctrl.flow_type != HCC_FLOWCTRL_CREDIT) {
            if (oal_netbuf_list_len(&hcc->hcc_transer_info.hcc_queues[dir].queues[i].data_queue)) {
                if (oal_unlikely((hcc_to_dev(hcc) == NULL) || (hcc_to_bus(hcc) == NULL))) {
                    oal_print_hi11xx_log(HI11XX_LOG_WARN, "hcc'dev is %p, hcc'dev bus is %p",
                                         hcc_to_dev(hcc), hcc_to_dev(hcc) ? hcc_to_bus(hcc) : NULL);
                } else {
                    if (hcc_bus_check_tx_condition(hcc_to_bus(hcc),
                                                   hcc_queue_map_to_netbuf_queue((hcc_queue_type)i)) == OAL_TRUE) {
                        /* 发送通道畅通 */
                        oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "sdio tx cond true");
                        return OAL_TRUE;
                    }
                }
            }
        }
    }

    return OAL_FALSE;
}

OAL_STATIC int32_t _queues_pcie_len_check(struct hcc_handler *hcc,
                                          hcc_chan_type dir)
{
    int32_t i;
    for (i = 0; i < HCC_QUEUE_COUNT; i++) {
        if (oal_netbuf_list_len(&hcc->hcc_transer_info.hcc_queues[dir].queues[i].data_queue)) {
            if (oal_unlikely((hcc_to_dev(hcc) == NULL) || (hcc_to_bus(hcc) == NULL))) {
                oal_print_hi11xx_log(HI11XX_LOG_WARN, "hcc'dev is %p, hcc'dev bus is %p",
                                     hcc_to_dev(hcc), hcc_to_dev(hcc) ? hcc_to_bus(hcc) : NULL);
            } else {
                if (hcc_bus_check_tx_condition(hcc_to_bus(hcc),
                                               hcc_queue_map_to_netbuf_queue((hcc_queue_type)i)) == OAL_TRUE) {
                    /* 发送通道畅通 */
                    oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "pcie check tx cond true");
                    return OAL_TRUE;
                }
            }
        }
    }

    return OAL_FALSE;
}

/* wether the hcc flow ctrl queues have data. */
OAL_STATIC int32_t _queues_flow_ctrl_len_check(struct hcc_handler *hcc, hcc_chan_type dir)
{
    int32_t i;
    for (i = 0; i < HCC_QUEUE_COUNT; i++) {
        if (hcc->hcc_transer_info.hcc_queues[dir].queues[i].flow_ctrl.enable == OAL_TRUE) {
            if (hcc->hcc_transer_info.hcc_queues[dir].queues[i].flow_ctrl.flow_type == HCC_FLOWCTRL_SDIO) {
                if (oal_netbuf_list_len(&hcc->hcc_transer_info.hcc_queues[dir].queues[i].data_queue)
                    && (hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_flag == D2H_MSG_FLOWCTRL_ON)) {
                    oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "sdio fc true");
                    return 1;
                }
            }
        } else {
            if (hcc->hcc_transer_info.hcc_queues[dir].queues[i].flow_ctrl.flow_type == HCC_FLOWCTRL_CREDIT) {
                /* credit flowctrl */
                if (oal_netbuf_list_len(&hcc->hcc_transer_info.hcc_queues[dir].queues[i].data_queue)
                    && (hcc->hcc_transer_info.tx_flow_ctrl.uc_hipriority_cnt > g_hcc_credit_bottom_value)) {
                    oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "sdio fcr true");
                    return 1;
                }
            }
        }
    }

    return 0;
}

uint32_t hcc_queues_flow_ctrl_len(struct hcc_handler *hcc, hcc_chan_type dir)
{
    int32_t i;
    uint32_t total = 0;
    for (i = 0; i < HCC_QUEUE_COUNT; i++) {
        if (hcc->hcc_transer_info.hcc_queues[dir].queues[i].flow_ctrl.enable == OAL_TRUE) {
            total += oal_netbuf_list_len(&hcc->hcc_transer_info.hcc_queues[dir].queues[i].data_queue);
        }
    }
    return total;
}

/* wether the hcc queue have data. */
OAL_STATIC int32_t _queues_len_check(struct hcc_handler *hcc, hcc_chan_type dir)
{
    int32_t i;
    for (i = 0; i < HCC_QUEUE_COUNT; i++) {
        if (oal_netbuf_list_len(&hcc->hcc_transer_info.hcc_queues[dir].queues[i].data_queue)) {
            oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "d:%d, qid:%d had pkts", dir, i);
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}

/*
 * Prototype    : hcc_thread_wait_event_cond_check
 * Description  : the hcc thread process conditon check, check whether queue empty
 * Input        : struct hcc_handler* hcc
 */
OAL_STATIC int32_t hcc_thread_wait_event_cond_check(struct hcc_handler *hcc)
{
    int32_t ret;
    if (oal_warn_on(hcc == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: hcc is null", __FUNCTION__);
        return OAL_FALSE;
    };
    /* please first check the condition
      which may be ok likely to reduce the cpu mips */
    if (hcc->bus_dev->cur_bus->bus_type == HCC_BUS_PCIE) {
        ret = ((_queues_len_check(hcc, HCC_RX)) ||
               (_queues_pcie_len_check(hcc, HCC_TX)) || (hcc_bus_pending_signal_check(hcc->bus_dev->cur_bus)) ||
               (hcc->p_hmac_tcp_ack_need_schedule_func && hcc->p_hmac_tcp_ack_need_schedule_func() == OAL_TRUE));
    } else if (hcc->bus_dev->cur_bus->bus_type == HCC_BUS_SDIO) {
        ret = (_queues_flow_ctrl_len_check(hcc, HCC_TX) ||
               (_queues_len_check(hcc, HCC_RX)) ||
               (_queues_not_flowctrl_len_check(hcc, HCC_TX)) ||
               (hcc->p_hmac_tcp_ack_need_schedule_func && hcc->p_hmac_tcp_ack_need_schedule_func() == OAL_TRUE));
    } else {
        ret = OAL_FALSE;
    }

#ifdef _PRE_CONFIG_WLAN_THRANS_THREAD_DEBUG
    if (ret == OAL_TRUE) {
        hcc->hcc_transer_info.thread_stat.wait_event_run_count++;
    }
    if (ret == OAL_FALSE) {
        hcc->hcc_transer_info.thread_stat.wait_event_block_count++;
    }
#endif
    oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "wait_cond:%d", ret);
    return ret;
}

OAL_STATIC int32_t hcc_send_single_descr(struct hcc_handler *hcc, oal_netbuf_stru *netbuf)
{
    oal_netbuf_head_stru head_send;
    oal_reference(hcc);

    if (oal_warn_on(netbuf == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "netbuf is null!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    oal_netbuf_list_head_init(&head_send);
    oal_netbuf_list_tail(&head_send, netbuf);

    return hcc_bus_tx_netbuf_list(hcc_to_bus(hcc), &head_send, HCC_NETBUF_NORMAL_QUEUE);
}

int32_t hcc_send_descr_control_data(struct hcc_handler *hcc, hcc_descr_type descr_type,
                                    const void *data, uint32_t ul_size)
{
    int32_t ret;
    oal_netbuf_stru *netbuf = NULL;
    struct hcc_descr_header *dscr_hdr = NULL;
    netbuf = hcc_tx_assem_descr_get(hcc);
    if (oal_warn_on(netbuf == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "No descr mem!");
        return -OAL_ENOMEM;
    }

    dscr_hdr = (struct hcc_descr_header *)oal_netbuf_data(netbuf);
    dscr_hdr->descr_type = descr_type;

    if (ul_size) {
        if (oal_warn_on(data == NULL)) {
            hcc_tx_assem_descr_put(hcc, netbuf);
            return -OAL_EINVAL;
        }
        if (oal_warn_on(ul_size + sizeof(struct hcc_descr_header) > oal_netbuf_len(netbuf))) {
            oal_io_print("invaild request size:%u,max size:%u\r\n",
                         (uint32_t)(ul_size + sizeof(struct hcc_descr_header)),
                         (uint32_t)oal_netbuf_len(netbuf));
            hcc_tx_assem_descr_put(hcc, netbuf);
            return -OAL_EINVAL;
        }

        /*lint -e124*/
        if (memcpy_s((void *)((uint8_t *)oal_netbuf_data(netbuf) + sizeof(struct hcc_descr_header)),
                     oal_netbuf_len(netbuf) - sizeof(struct hcc_descr_header), data, ul_size) != EOK) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "memcpy_s error, destlen=%u, srclen=%u\n ",
                                 (uint32_t)(oal_netbuf_len(netbuf) - sizeof(struct hcc_descr_header)), ul_size);
            hcc_tx_assem_descr_put(hcc, netbuf);
            return -OAL_EFAIL;
        }
    }

    ret = hcc_send_single_descr(hcc, netbuf);

    hcc_tx_assem_descr_put(hcc, netbuf);
    return ret;
}

OAL_STATIC int32_t hcc_tx_netbuf_queue_switch(struct hcc_handler *hcc, hcc_netbuf_queue_type queue_type)
{
    return hcc_send_descr_control_data(hcc, HCC_NETBUF_QUEUE_SWITCH, &queue_type, sizeof(queue_type));
}

OAL_STATIC int32_t hcc_tx_netbuf_test_and_switch_high_pri_queue(struct hcc_handler *hcc,
                                                                hcc_netbuf_queue_type pool_type)
{
    int32_t ret = OAL_EFAIL;
    if (pool_type == HCC_NETBUF_HIGH_QUEUE) {
        ret = hcc_tx_netbuf_queue_switch(hcc, HCC_NETBUF_HIGH_QUEUE);
    }
    return ret;
}

OAL_STATIC int32_t hcc_tx_netbuf_restore_normal_pri_queue(struct hcc_handler *hcc,
                                                          hcc_netbuf_queue_type pool_type)
{
    int32_t ret = OAL_EFAIL;
    if (pool_type == HCC_NETBUF_HIGH_QUEUE) {
        ret = hcc_tx_netbuf_queue_switch(hcc, HCC_NETBUF_NORMAL_QUEUE);
    }
    return ret;
}

/* 归还待发送队列 */
void hcc_restore_tx_netbuf(struct hcc_handler *hcc, oal_netbuf_stru *pst_netbuf)
{
    struct hcc_tx_cb_stru *pst_cb_stru = NULL;
    hcc_trans_queue *pst_hcc_queue = NULL;

    if (oal_unlikely((hcc == NULL) || (pst_netbuf == NULL))) {
        oal_warn_on(1);
        return;
    }

    pst_cb_stru = (struct hcc_tx_cb_stru *)oal_netbuf_cb(pst_netbuf);
    if (oal_unlikely(pst_cb_stru->magic != HCC_TX_WAKELOCK_MAGIC || pst_cb_stru->qtype >= HCC_QUEUE_COUNT)) {
#ifdef CONFIG_PRINTK
        printk(KERN_EMERG "BUG: tx netbuf:%p on CPU#%d,magic:%08x should be %08x, qtype:%u\n", pst_cb_stru,
               raw_smp_processor_id(), pst_cb_stru->magic, HCC_TX_WAKELOCK_MAGIC, pst_cb_stru->qtype);
        print_hex_dump(KERN_ERR, "tx_netbuf_magic", DUMP_PREFIX_ADDRESS, 16, 1, /* 16进制 */
                       (uint8_t *)pst_netbuf, sizeof(oal_netbuf_stru), true); /* 内核函数固定的传参 */
        printk(KERN_ERR "\n");
#endif
        declare_dft_trace_key_info("tx_restore_wakelock_crash", OAL_DFT_TRACE_EXCEP);
        return;
    }

    pst_hcc_queue = &hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[pst_cb_stru->qtype];

    oal_netbuf_addlist(&pst_hcc_queue->data_queue, pst_netbuf);
}

void hcc_restore_assemble_netbuf_list(struct hcc_handler *hcc)
{
    hcc_queue_type type;
    oal_netbuf_head_stru *assembled_head = NULL;

    if (oal_unlikely(hcc == NULL)) {
        oal_warn_on(1);
        return;
    }

    type = hcc->hcc_transer_info.tx_assem_info.assembled_queue_type;
    assembled_head = &hcc->hcc_transer_info.tx_assem_info.assembled_head;

    if (type >= HCC_QUEUE_COUNT) {
        type = DATA_LO_QUEUE;
    }

    if (!oal_netbuf_list_empty(assembled_head))
        oal_netbuf_splice_sync(&hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[type].data_queue,
                               assembled_head);
}

OAL_STATIC int32_t hcc_send_assemble_reset(struct hcc_handler *hcc)
{
    int32_t ret;

    hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_reset_count++;

    /* 当只发送一个聚合描述符包，并且聚合个数为0描述通知Device 重置聚合信息 */
    ret = hcc_send_descr_control_data(hcc, HCC_DESCR_ASSEM_RESET, NULL, 0);

    hcc_restore_assemble_netbuf_list(hcc);

    return ret;
}

static inline int32_t hcc_send_data_credit_fc_cond(struct hcc_handler *hcc, hcc_trans_queue *pst_hcc_queue)
{
    uint8_t uc_credit;
    if (pst_hcc_queue->flow_ctrl.flow_type == HCC_FLOWCTRL_CREDIT) {
        /* credit flowctrl */
        uc_credit = hcc->hcc_transer_info.tx_flow_ctrl.uc_hipriority_cnt;

        /* 高优先级流控: credit值为0时不发送 */
        if (!(uc_credit > g_hcc_credit_bottom_value)) {
            return -OAL_EBUSY;
        }
    }

    return OAL_SUCC;
}

static inline void hcc_send_data_update_credit_info(struct hcc_handler *hcc, hcc_trans_queue *pst_hcc_queue,
                                                    uint32_t total_send)
{
    uint8_t uc_credit;
   /* 高优先级流控: 更新credit值 */
    if (pst_hcc_queue->flow_ctrl.flow_type == HCC_FLOWCTRL_CREDIT) {
        oal_spin_lock(&(hcc->hcc_transer_info.tx_flow_ctrl.st_hipri_lock));

        uc_credit = hcc->hcc_transer_info.tx_flow_ctrl.uc_hipriority_cnt;
        uc_credit = (uc_credit > total_send) ? (uint8_t)(uc_credit - total_send) : 0;
        hcc->hcc_transer_info.tx_flow_ctrl.uc_hipriority_cnt = uc_credit;

        oal_spin_unlock(&(hcc->hcc_transer_info.tx_flow_ctrl.st_hipri_lock));
    }
}

static int32_t hcc_send_prepare_data_packet(struct hcc_handler *hcc, oal_netbuf_head_stru *head,
                                            oal_netbuf_head_stru *head_send,
                                            oal_netbuf_stru *netbuf, oal_netbuf_head_stru *next_assembled_head)
{
    oal_netbuf_stru *netbuf_t = NULL;
    uint32_t *info = hcc->hcc_transer_info.tx_assem_info.info;
    if (oal_netbuf_list_empty(next_assembled_head)) {
        /* single send */
        netbuf = oal_netbuf_delist(head);
        if (netbuf == NULL) {
            oal_io_print("netbuf is NULL [len:%d]\n", oal_netbuf_list_len(head));
            return -OAL_EINVAL;
        }
        netbuf_t = hcc_netbuf_len_align(netbuf, HISDIO_H2D_SCATT_BUFFLEN_ALIGN);
        if (oal_unlikely(netbuf_t == NULL)) {
            /* return to the list */
            oal_netbuf_addlist(head, netbuf);
            return -OAL_ENOMEM;
        }
        oal_netbuf_list_tail(head_send, netbuf);
        info[0]++;
    } else {
        uint32_t assemble_len = oal_netbuf_list_len(next_assembled_head);
        if (oal_unlikely(assemble_len > HISDIO_HOST2DEV_SCATT_SIZE)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: assemble_len:%d > HISDIO_HOST2DEV_SCATT_SIZE:%d",
                                 __FUNCTION__, assemble_len, HISDIO_HOST2DEV_SCATT_SIZE);
        } else {
            info[assemble_len]++;
        }
        /* move the assem list to send queue */
        oal_netbuf_splice_init(next_assembled_head, head_send);
    }
    return OAL_SUCC;
}

OAL_STATIC int32_t hcc_send_data_packet(struct hcc_handler *hcc,
                                        oal_netbuf_head_stru *head,
                                        hcc_queue_type type,
                                        oal_netbuf_head_stru *next_assembled_head,
                                        hcc_send_mode mode,
                                        uint32_t *remain_len)
{
    uint8_t *buf = NULL;
    uint32_t total_send;
    int32_t ret;
    oal_netbuf_head_stru head_send;
    oal_netbuf_stru *netbuf = NULL;
    oal_netbuf_stru *descr_netbuf = NULL;
    hcc_trans_queue *pst_hcc_queue = NULL;

    if (*remain_len == 0) {
        return OAL_SUCC;
    }

    oal_print_hi11xx_log(HI11XX_LOG_DBG, "send queue:%d, mode:%d,next assem len:%d",
                         type, mode,
                         oal_netbuf_list_len(next_assembled_head));

    pst_hcc_queue = &hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[type];

    ret = hcc_send_data_credit_fc_cond(hcc, pst_hcc_queue);
    if (ret != OAL_SUCC) {
        // credit flowctrl, return
        return OAL_SUCC;
    }

    descr_netbuf = hcc_tx_assem_descr_get(hcc);
    if (descr_netbuf == NULL) {
        ret = -OAL_ENOMEM;
        /*lint -e801*/
        goto failed_get_assem_descr;
    }

    oal_netbuf_list_head_init(&head_send);

    ret = hcc_send_prepare_data_packet(hcc, head, &head_send, netbuf, next_assembled_head);
    if (ret != OAL_SUCC) {
        if (ret == -OAL_EINVAL) {
            ret = OAL_SUCC;
        }
        goto failed_align_netbuf;
    }

    total_send = oal_netbuf_list_len(&head_send);
    if (*remain_len >= total_send) {
        *remain_len -= total_send;
    } else {
        *remain_len = 0;
    }

    if (oal_warn_on(!oal_netbuf_list_empty(next_assembled_head))) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: next_assembled_head is empty", __FUNCTION__);
    }

    if (mode == HCC_ASSEM_SEND) {
        hcc_build_next_assem_descr(hcc, type, head, next_assembled_head, descr_netbuf, *remain_len);
    } else {
        buf = oal_netbuf_data(descr_netbuf);
        *((uint32_t *)buf) = 0;
    }

    /* add the assem descr buf */
    oal_netbuf_addlist(&head_send, descr_netbuf);

    ret = hcc_bus_tx_netbuf_list(hcc_to_bus(hcc), &head_send, hcc_queue_map_to_netbuf_queue(type));

    g_pm_wlan_pkt_statis.h2d_tx_pkt += total_send;

    hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[type].total_pkts += total_send;

    hcc_send_data_update_credit_info(hcc, pst_hcc_queue, total_send);

    descr_netbuf = oal_netbuf_delist(&head_send);
    if (descr_netbuf == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hcc_send_data_packet::oal_netbuf_delist fail.descr_netbuf is NULL.");
        ret = OAL_SUCC;
        goto failed_get_sig_buff;
    }

    hcc_tx_assem_descr_put(hcc, descr_netbuf);

    /* free the sent netbuf,release the wakelock */
    hcc_tx_netbuf_list_free(&head_send);

    return ret;
failed_align_netbuf:
failed_get_sig_buff:
    hcc_tx_assem_descr_put(hcc, descr_netbuf);
failed_get_assem_descr:
    return ret;
}

void hcc_clear_all_queues(struct hcc_handler *hcc, int32_t is_need_lock)
{
    int32_t i;
    oal_netbuf_head_stru *pst_head = NULL;

    if (oal_warn_on(hcc == NULL)) {
        oal_io_print("[E]hcc is null\n");
        return;
    }

    if (is_need_lock == OAL_TRUE) {
        hcc_tx_transfer_lock(hcc);
    }

    /* Restore assem queues */
    hcc_restore_assemble_netbuf_list(hcc);

    /* Clear all tx queues */
    for (i = 0; i < HCC_QUEUE_COUNT; i++) {
        uint32_t ul_list_len;
        pst_head = &hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[i].data_queue;
        ul_list_len = oal_netbuf_list_len(pst_head);
        hcc_tx_netbuf_list_free(pst_head);
        if (ul_list_len) {
            oal_io_print("Clear queue:%d,total %u hcc tx pkts!\n", i, ul_list_len);
        }
    }

    if (is_need_lock == OAL_TRUE) {
        hcc_tx_transfer_unlock(hcc);
    }

    /* Clear all rx queues */
    if (is_need_lock == OAL_TRUE) {
        hcc_rx_transfer_lock(hcc);
    }

    for (i = 0; i < HCC_QUEUE_COUNT; i++) {
        uint32_t ul_list_len;
        pst_head = &hcc->hcc_transer_info.hcc_queues[HCC_RX].queues[i].data_queue;
        ul_list_len = oal_netbuf_list_len(pst_head);
        oal_netbuf_queue_purge(pst_head);
        if (ul_list_len) {
            oal_io_print("Clear queue:%d,total %u hcc rx pkts!\n", i, ul_list_len);
        }
    }

    if (is_need_lock == OAL_TRUE) {
        hcc_rx_transfer_unlock(hcc);
    }
}

void hcc_change_state(struct hcc_handler *hcc, int32_t state)
{
    int32_t old_state, new_state;
    if (oal_warn_on(hcc == NULL)) {
        return;
    }

    old_state = oal_atomic_read(&hcc->state);
    oal_atomic_set(&hcc->state, state);
    new_state = oal_atomic_read(&hcc->state);
    if (old_state != new_state) {
        oal_io_print("hcc state %s[%d]=>%s[%d]\n",
                     old_state == HCC_ON ? "on " : "off",
                     old_state,
                     new_state == HCC_ON ? "on " : "off",
                     new_state);
    }
}

void hcc_change_state_exception(void)
{
    hcc_change_state(hcc_get_110x_handler(), HCC_EXCEPTION);
}

void hcc_enable(struct hcc_handler *hcc, int32_t is_need_lock)
{
    int32_t i;
    if (oal_warn_on(hcc == NULL)) {
        return;
    }

    if (is_need_lock == OAL_TRUE) {
        hcc_transfer_lock(hcc);
    }

    hcc_clear_all_queues(hcc, OAL_FALSE);

    for (i = 0; i < HCC_QUEUE_COUNT; i++) {
        oal_spin_lock_bh(&hcc->hcc_transer_info.tx_flow_ctrl.lock);
        hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[i].flow_ctrl.is_stopped = OAL_FALSE;
        oal_spin_unlock_bh(&hcc->hcc_transer_info.tx_flow_ctrl.lock);
    }

    hcc_change_state(hcc, HCC_ON);

    if (is_need_lock == OAL_TRUE) {
        hcc_transfer_unlock(hcc);
    }
}

void hcc_disable(struct hcc_handler *hcc, int32_t is_need_lock)
{
    if (oal_unlikely(hcc == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "hcc is null");
        return;
    }

    hcc_change_state(hcc, HCC_OFF);

    /* disable flow ctrl detect timer */
    hcc_dev_flowctr_timer_del(hcc);
    oal_cancel_delayed_work_sync(&hcc->hcc_transer_info.tx_flow_ctrl.worker);

    if (is_need_lock == OAL_TRUE) {
        hcc_transfer_lock(hcc);
    }

    hcc_clear_all_queues(hcc, OAL_FALSE);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    atomic_set(&hcc->tx_seq, 0);
#endif
    if (is_need_lock == OAL_TRUE) {
        hcc_transfer_unlock(hcc);
    }
    /* must wake up tx thread after clear the hcc queues */
    oal_wait_queue_wake_up_interrupt(&hcc->hcc_transer_info.tx_flow_ctrl.wait_queue);
}

static int32_t hcc_send_tx_queue_by_pcie(struct hcc_handler *hcc, hcc_queue_type type)
{
    int32_t count = 0;
    hcc_netbuf_queue_type qtype;

    hcc_trans_queue *pst_hcc_queue = &hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[type];
    oal_netbuf_head_stru *head = &pst_hcc_queue->data_queue;

    /* 参考 hcc_send_data_packet */
    if (oal_netbuf_list_empty(head)) {
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "queue type %d is empty\n", type);
        return count;
    }

    qtype = hcc_queue_map_to_netbuf_queue(type);
    count += hcc_bus_tx_netbuf_list(hcc_to_bus(hcc), head, qtype);
    hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[type].total_pkts += count;
    hcc->hcc_transer_info.tx_assem_info.pkt_cnt += count;
    g_pm_wlan_pkt_statis.h2d_tx_pkt += (uint32_t)count;

    oal_spin_lock_bh(&hcc->hcc_transer_info.tx_flow_ctrl.lock);

    if ((oal_netbuf_list_len(&pst_hcc_queue->data_queue) < pst_hcc_queue->flow_ctrl.low_waterline)) {
        if (pst_hcc_queue->flow_ctrl.is_stopped == OAL_TRUE) {
            pst_hcc_queue->flow_ctrl.is_stopped = OAL_FALSE;
            hcc_tx_network_start_subq(hcc, (uint16_t)pst_hcc_queue->wlan_queue_id);
        }
        oal_wait_queue_wake_up_interrupt(&hcc->hcc_transer_info.tx_flow_ctrl.wait_queue);
    }
    oal_spin_unlock_bh(&hcc->hcc_transer_info.tx_flow_ctrl.lock);
    return count;
}

#ifdef CONFIG_SDIO_MSG_FLOWCTRL
static inline int32_t hcc_reset_tx_assem_queue(struct hcc_handler *hcc, hcc_trans_queue *pst_hcc_queue)
{
    if (pst_hcc_queue->flow_ctrl.enable != OAL_TRUE) {
        return OAL_SUCC;
    }

    if (pst_hcc_queue->flow_ctrl.flow_type != HCC_FLOWCTRL_SDIO) {
        // If not sdio flowctrl, continue
        return OAL_SUCC;
    }

    if (D2H_MSG_FLOWCTRL_ON == hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_flag) {
        // If can send pkts, continue
        return OAL_SUCC;
    }

    oal_print_hi11xx_log(HI11XX_LOG_DBG, "[WARNING]can't send data, flow off");

    if (oal_netbuf_list_empty(&hcc->hcc_transer_info.tx_assem_info.assembled_head)) {
        // If assem list is empty, need't send assem info, break transfer
        return -OAL_EBUSY;
    }

    /* flow ctrl, break transfer */
    if (hcc_send_assemble_reset(hcc) != OAL_SUCC) {
        /* send one pkt */
        return -OAL_EFAIL;
    }
    return -OAL_EBUSY;
}
#endif

static inline void hcc_tx_restore_credit_queue(struct hcc_handler *hcc, hcc_trans_queue *pst_hcc_queue,
                                               hcc_netbuf_queue_type pool_type,
                                               uint32_t ul_pool_type_flag)
{
    if (pst_hcc_queue->flow_ctrl.flow_type == HCC_FLOWCTRL_CREDIT) {
        if (ul_pool_type_flag == OAL_TRUE) {
            hcc_tx_netbuf_restore_normal_pri_queue(hcc, pool_type);
        }
    }
}

static inline void hcc_flowctrl_wakeup_tx_thread(struct hcc_handler *hcc, hcc_trans_queue *pst_hcc_queue)
{
    oal_spin_lock_bh(&hcc->hcc_transer_info.tx_flow_ctrl.lock);

    if ((oal_netbuf_list_len(&pst_hcc_queue->data_queue) < pst_hcc_queue->flow_ctrl.low_waterline)) {
        if (pst_hcc_queue->flow_ctrl.is_stopped == OAL_TRUE) {
            pst_hcc_queue->flow_ctrl.is_stopped = OAL_FALSE;
            hcc_tx_network_start_subq(hcc, (uint16_t)pst_hcc_queue->wlan_queue_id);
        }
        oal_wait_queue_wake_up_interrupt(&hcc->hcc_transer_info.tx_flow_ctrl.wait_queue);
    }
    oal_spin_unlock_bh(&hcc->hcc_transer_info.tx_flow_ctrl.lock);
}

static inline int32_t hcc_send_tx_credit_process(struct hcc_handler *hcc, hcc_trans_queue *pst_hcc_queue,
                                                 hcc_netbuf_queue_type pool_type,
                                                 uint32_t ul_pool_type_flag)
{
    uint8_t uc_credit;
    if (pst_hcc_queue->flow_ctrl.flow_type == HCC_FLOWCTRL_CREDIT) {
        uc_credit = hcc->hcc_transer_info.tx_flow_ctrl.uc_hipriority_cnt;

        /* 高优先级如果没有内存，直接返回规避死循环问题。 */
        if (!(uc_credit > g_hcc_credit_bottom_value)) {
            if (ul_pool_type_flag == OAL_TRUE) {
                /* 恢复成普通优先级 */
                hcc_tx_netbuf_restore_normal_pri_queue(hcc, pool_type);
            }
            return -OAL_EBUSY;
        }

        if (ul_pool_type_flag == OAL_FALSE) {
            if (hcc_tx_netbuf_test_and_switch_high_pri_queue(hcc, pool_type) == OAL_SUCC) {
                ul_pool_type_flag = OAL_TRUE;
            }
        }
    }

    return OAL_SUCC;
}

static int32_t hcc_send_tx_queue_by_sdio(struct hcc_handler *hcc, hcc_queue_type type)
{
    int32_t count = 0;

    hcc_trans_queue *pst_hcc_queue = &hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[type];
    oal_netbuf_head_stru *head = &pst_hcc_queue->data_queue;

    int32_t ret;
    uint32_t queue_len, remain_len, remain_len_t;
    oal_netbuf_head_stru *next_assembled_head = NULL;
    hcc_send_mode send_mode;
    hcc_netbuf_queue_type pool_type;
    uint32_t ul_pool_type_flag = OAL_FALSE;

    if (oal_netbuf_list_empty(head)) {
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "queue type %d is empty", type);
        return count;
    }

    queue_len = oal_netbuf_list_len(head);
    remain_len = queue_len;

    oal_print_hi11xx_log(HI11XX_LOG_DBG,
                         "before_update: hcc_send_tx_queue: queue_type = %d, burst_limit = %d, remain_len = %d",
                         type, pst_hcc_queue->burst_limit, remain_len);
    remain_len = oal_min(pst_hcc_queue->burst_limit, remain_len);

    remain_len_t = remain_len;
    next_assembled_head = &hcc->hcc_transer_info.tx_assem_info.assembled_head;

    if (oal_unlikely(!oal_netbuf_list_empty(next_assembled_head))) {
        /* First enter the queue, should single send */
        oal_io_print("[WARN]reset assemble package![queue type:%d,len:%d]\n",
                     (int32_t)type, oal_netbuf_list_len(next_assembled_head));
        if (OAL_SUCC != hcc_send_assemble_reset(hcc)) {
            /* send one pkt */
            return 1;
        }
    }

    send_mode = pst_hcc_queue->send_mode;
    pool_type = (hcc_netbuf_queue_type)pst_hcc_queue->netbuf_pool_type;

    forever_loop() {
        if (remain_len == 0) {
            break;
        }

#ifdef CONFIG_SDIO_MSG_FLOWCTRL
        ret = hcc_reset_tx_assem_queue(hcc, pst_hcc_queue);
        if (ret != OAL_SUCC) {
            /* If SUCC, continue */
            if (ret == -OAL_EFAIL) {
                count++;
            }
            return count;
        }
#endif
        ret = hcc_send_tx_credit_process(hcc, pst_hcc_queue, pool_type, ul_pool_type_flag);
        if (ret != OAL_SUCC) {
            // flowctrl, break
            return OAL_SUCC;
        }

        ret = hcc_send_data_packet(hcc, head, type, next_assembled_head, send_mode, &remain_len);
        if (oal_unlikely(remain_len > remain_len_t)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: remain_len:%d > remain_len_t:%d",
                                 __FUNCTION__, remain_len, remain_len_t);
            return OAL_FAIL;
        };

        if (oal_likely(ret == OAL_SUCC)) {
            count += (int32_t)(remain_len_t - remain_len);
            hcc->hcc_transer_info.tx_assem_info.pkt_cnt += (remain_len_t - remain_len);
        }

#ifdef CONFIG_SDIO_MSG_FLOWCTRL
        hcc_flowctrl_wakeup_tx_thread(hcc, pst_hcc_queue);
#endif
    }
    hcc_tx_restore_credit_queue(hcc, pst_hcc_queue, pool_type, ul_pool_type_flag);
    return count;
}

/*
 * Prototype    : hcc_send_tx_queue
 * Description  : send the special
 * Input        : struct hcc_handler *hcc
 */
int hcc_send_tx_queue(struct hcc_handler *hcc, hcc_queue_type type)
{
    int32_t count = 0;
    hcc_trans_queue *pst_hcc_queue = NULL;

    if (oal_unlikely(hcc == NULL)) {
        oal_io_print("%s error: hcc is null", __FUNCTION__);
        return 0;
    };

    if (oal_unlikely(type >= HCC_QUEUE_COUNT)) {
        oal_io_print("unkown hcc queue type %d\n", type);
        return count;
    }

    pst_hcc_queue = &hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[type];

    hcc_tx_transfer_lock(hcc);
#ifdef WIN32
    if (0) {
#else
    if (hcc->bus_dev->cur_bus->bus_type == HCC_BUS_PCIE) {
#endif
        count = hcc_send_tx_queue_by_pcie(hcc, type);
    } else { /* SDIO & USB */
        count = hcc_send_tx_queue_by_sdio(hcc, type);
    }
    hcc_tx_transfer_unlock(hcc);
    return count;
}

/*
 * Prototype    : hcc_rx_register
 * Description  : register cb function
 */
int32_t hcc_rx_register(struct hcc_handler *hcc, uint8_t mtype, hcc_rx_post_do post_do, hcc_rx_pre_do pre_do)
{
    hcc_rx_action *rx_action = NULL;

    if (oal_warn_on(hcc == NULL)) {
        return -OAL_EINVAL;
    }

    rx_action = &(hcc->hcc_transer_info.rx_action_info.action[mtype]);

    if (oal_warn_on(rx_action->post_do != NULL)) {
#ifdef CONFIG_PRINTK
        oal_io_print("repeat regist hcc action :%d,it's had %pF\n", mtype, rx_action->post_do);
#else
        oal_io_print("repeat regist hcc action :%d,it's had %p\n", mtype, rx_action->post_do);
#endif
        return -OAL_EBUSY;
    }
    /* 此处暂时不加互斥锁，由流程保证。 */
    rx_action->post_do = post_do;
    rx_action->pre_do = pre_do;

    return OAL_SUCC;
}
/*lint -e19*/
oal_module_symbol(hcc_rx_register);
/*lint +e19*/
int32_t hcc_rx_unregister(struct hcc_handler *hcc, uint8_t mtype)
{
    hcc_rx_action *rx_action = NULL;

    if (oal_warn_on((hcc == NULL) || (mtype >= HCC_ACTION_TYPE_BUTT))) {
        oal_io_print("Invalid params:hcc = %p, main type = %u\n", hcc, mtype);
        return -OAL_EFAIL;
    }

    rx_action = &hcc->hcc_transer_info.rx_action_info.action[mtype];

    memset_s((void *)rx_action, sizeof(hcc_rx_action), 0, sizeof(hcc_rx_action));

    return OAL_SUCC;
}
/*lint -e19*/
oal_module_symbol(hcc_rx_unregister);
/*lint +e19*/
OAL_STATIC uint32_t hcc_check_rx_netbuf_vaild(struct hcc_header *hdr, int32_t netbuf_len)
{
    if (oal_unlikely(hcc_check_header_vaild(hdr) != OAL_TRUE)) {
        return OAL_FALSE;
    }

    if (oal_unlikely(hdr->pad_payload + HCC_HDR_TOTAL_LEN + hdr->pay_len > netbuf_len)) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

/*
 * Prototype    : hcc_rx
 * Description  : trim the hcc header etc.
 */
OAL_STATIC int32_t hcc_rx(struct hcc_handler *hcc, oal_netbuf_stru *netbuf)
{
    struct hcc_header *hdr = NULL;
    int32_t netbuf_len;
    int32_t extend_len;

    if (oal_unlikely(!hcc)) {
        oal_io_print("hcc is null r failed!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    hdr = (struct hcc_header *)oal_netbuf_data(netbuf);
    netbuf_len = (int32_t)oal_netbuf_len(netbuf);

    if (oal_unlikely(hcc_check_rx_netbuf_vaild(hdr, netbuf_len) != OAL_TRUE)) {
        oal_print_hex_dump((uint8_t *)hdr, (int32_t)oal_netbuf_len(netbuf), HEX_DUMP_GROUP_SIZE,
                           "invaild hcc_rx header: ");
        if (hcc_to_dev(hcc)) {
            hcc_bus_try_to_dump_device_mem(hcc_to_bus(hcc), OAL_FALSE);
        }
        return -OAL_EFAIL;
    }

    extend_len = HCC_HDR_TOTAL_LEN - HCC_HDR_LEN - hdr->pad_hdr;

    if (hdr->pad_payload) {
        /* memmove */
        uint8_t *extend_addr = (uint8_t *)hdr + HCC_HDR_LEN + hdr->pad_hdr;
        if (memmove_s(extend_addr + hdr->pad_payload, (uint32_t)extend_len,
                      extend_addr, (uint32_t)extend_len) != EOK) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "memmove_s failed.\n");
        }
    }

    if (HCC_HDR_LEN + hdr->pad_hdr + hdr->pad_payload > oal_netbuf_len(netbuf)) {
        /* add check detail */
        oal_io_print("[Error]wrong pad_hdr,hcc hdr too long,len:%d!\n", (int32_t)oal_netbuf_len(netbuf));
        oal_print_hex_dump((uint8_t *)oal_netbuf_data(netbuf), (int32_t)oal_netbuf_len(netbuf),
                           HEX_DUMP_GROUP_SIZE, "rx wrong data: ");
        /* print the data buff here! */
        return -OAL_EINVAL;
    }

#ifdef CONFIG_HCC_DEBUG
    oal_io_print("[WIFI]hcc rx pkt seq:%d\n", hdr->seq);
    oal_print_hex_dump((uint8_t *)hdr, HCC_HDR_TOTAL_LEN, HEX_DUMP_GROUP_SIZE, "hcc rx hdr:");
    oal_print_hex_dump(oal_netbuf_data(netbuf) + HCC_HDR_TOTAL_LEN + hdr->pad_payload,
                       hdr->pay_len, HEX_DUMP_GROUP_SIZE, "hcc rx payload:");
#endif

    oal_netbuf_pull(netbuf, HCC_HDR_LEN + hdr->pad_hdr + hdr->pad_payload);

    /* 传出去的netbuf len 包含extend_len长度! */
    oal_netbuf_trim(netbuf, oal_netbuf_len(netbuf) - hdr->pay_len - (uint32_t)extend_len);

    oal_netbuf_next(netbuf) = NULL;
    oal_netbuf_prev(netbuf) = NULL;

    return OAL_SUCC;
}

int32_t hcc_send_rx_queue(struct hcc_handler *hcc, hcc_queue_type type)
{
    int32_t count = 0;
    int32_t pre_ret;
#ifndef WIN32
    oal_uint ul_irq_flag;
#endif
    uint8_t *pst_pre_context = NULL;
    oal_netbuf_head_stru *netbuf_hdr = NULL;
    hcc_rx_action *rx_action = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;
    hcc_netbuf_stru st_hcc_netbuf;
    struct hcc_header *pst_hcc_head = NULL;
    oal_netbuf_head_stru st_netbuf_header;

    if (oal_warn_on(type >= HCC_QUEUE_COUNT)) {
        return -OAL_EINVAL;
    }

    netbuf_hdr = &hcc->hcc_transer_info.hcc_queues[HCC_RX].queues[type].data_queue;

    oal_netbuf_list_head_init(&st_netbuf_header);

#ifndef WIN32
    spin_lock_irqsave(&netbuf_hdr->lock, ul_irq_flag);
    oal_netbuf_splice_init(netbuf_hdr, &st_netbuf_header);
    spin_unlock_irqrestore(&netbuf_hdr->lock, ul_irq_flag);
#else
    oal_netbuf_splice_sync(&st_netbuf_header, netbuf_hdr);
#endif

    /* 依次处理队列中每个netbuf */
    forever_loop() {
        pst_netbuf = oal_netbuf_delist_nolock(&st_netbuf_header);
        if (pst_netbuf == NULL) {
            break;
        }

        pst_hcc_head = (struct hcc_header *)oal_netbuf_data(pst_netbuf);
        if (oal_unlikely(hcc_check_header_vaild(pst_hcc_head) != OAL_TRUE)) {
            oal_print_hex_dump((uint8_t *)pst_hcc_head, HCC_HDR_TOTAL_LEN, HEX_DUMP_GROUP_SIZE,
                               "invaild hcc header: ");
            declare_dft_trace_key_info("hcc_rx_hdr_err", OAL_DFT_TRACE_EXCEP);
            oam_error_log0(0, OAM_SF_ANY, "invaild hcc rx head");
            count++;
            oal_netbuf_free(pst_netbuf);
            return count;
        }

        rx_action = &hcc->hcc_transer_info.rx_action_info.action[pst_hcc_head->main_type];

        pre_ret = OAL_SUCC;

        /* prepare */
        if (rx_action->pre_do) {
            st_hcc_netbuf.pst_netbuf = pst_netbuf;
            st_hcc_netbuf.len = (int32_t)oal_netbuf_len(pst_netbuf);
            pre_ret = rx_action->pre_do(hcc, pst_hcc_head->sub_type, &st_hcc_netbuf, &pst_pre_context);
        }

        if (pre_ret == OAL_SUCC) {
            if (oal_likely(hcc_rx(hcc, pst_netbuf) == OAL_SUCC)) {
                if (oal_likely(rx_action->post_do != NULL)) {
                    st_hcc_netbuf.pst_netbuf = pst_netbuf;
                    st_hcc_netbuf.len = (int32_t)oal_netbuf_len(pst_netbuf);
                    rx_action->post_do(hcc, pst_hcc_head->sub_type, &st_hcc_netbuf, pst_pre_context);
#ifdef _PRE_WLAN_PKT_TIME_STAT
                    if (DELAY_STATISTIC_SWITCH_ON) {
                        skbprobe_record_first(pst_netbuf, TP_SKB_DMAC);
                        skbprobe_record_time(pst_netbuf, TP_SKB_HMAC_RX);
                    }
#endif
                } else {
                    oam_error_log2(0, OAM_SF_ANY, "hcc mtype:%d,stype:%d did't register cb function!",
                                   pst_hcc_head->main_type,
                                   pst_hcc_head->sub_type);
                    oal_print_hex_dump((uint8_t *)pst_hcc_head, HCC_HDR_TOTAL_LEN,
                                       HEX_DUMP_GROUP_SIZE, "hcc invaild header: ");
                    oal_print_hex_dump(oal_netbuf_data(pst_netbuf), (int32_t)oal_netbuf_len(pst_netbuf),
                                       HEX_DUMP_GROUP_SIZE, "hcc invaild header(payload): ");
                    oal_netbuf_free(pst_netbuf);
                }
            } else {
                oal_netbuf_free(pst_netbuf);
            }
            hcc->hcc_transer_info.hcc_queues[HCC_RX].queues[type].total_pkts++;
            count++;
        } else {
#ifdef _PRE_WLAN_PKT_TIME_STAT
            if (DELAY_STATISTIC_SWITCH_ON) {
                skbprobe_record_first(pst_netbuf, TP_SKB_DMAC);
                skbprobe_record_time(pst_netbuf, TP_SKB_HMAC_RX);
            }
#endif
            /*
             * simple process, when pre do failed,
             * keep the netbuf in list,
             * and skip the loop
             */
            oal_netbuf_list_tail_nolock(&st_netbuf_header, pst_netbuf);
            oal_netbuf_splice_sync(netbuf_hdr, &st_netbuf_header);
            break;
        }
    }

    hcc->hcc_transer_info.rx_assem_info.pkt_cnt += count;

    return count;
}

OAL_STATIC void hcc_tx_network_start_subq(struct hcc_handler *hcc, uint16_t us_queue_idx)
{
    if (oal_warn_on(hcc == NULL)) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return;
    }

    if (oal_likely(hcc->hcc_transer_info.tx_flow_ctrl.start_subq)) {
        hcc->hcc_transer_info.tx_flow_ctrl.start_subq(us_queue_idx);
    }
}

OAL_STATIC void hcc_tx_network_stop_subq(struct hcc_handler *hcc, uint16_t us_queue_idx)
{
    if (oal_warn_on(hcc == NULL)) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return;
    }

    if (oal_likely(hcc->hcc_transer_info.tx_flow_ctrl.stop_subq)) {
        hcc->hcc_transer_info.tx_flow_ctrl.stop_subq(us_queue_idx);
    }
}

void hcc_flowctl_operate_subq_register(hcc_flowctl_start_subq start_subq, hcc_flowctl_stop_subq stop_subq)
{
    if (g_hcc_tc == NULL) {
        oal_io_print("[ERROR]hcc_flowctl_operate_subq_register failed! g_hcc_tc is NULL\n");
        return;
    }
    g_hcc_tc->hcc_transer_info.tx_flow_ctrl.start_subq = start_subq;
    g_hcc_tc->hcc_transer_info.tx_flow_ctrl.stop_subq = stop_subq;
    return;
}

void hcc_host_set_flowctl_param(uint8_t uc_queue_type, uint16_t us_burst_limit,
                                uint16_t us_low_waterline, uint16_t us_high_waterline)
{
    if (uc_queue_type >= HCC_QUEUE_COUNT) {
        oal_io_print("CONFIG_ERROR: hcc_host_set_flowctl_param: uc_queue_type = %d\r\n", uc_queue_type);
        return;
    }
    g_hcc_tc->hcc_transer_info.hcc_queues[HCC_TX].queues[uc_queue_type].burst_limit = us_burst_limit;
    g_hcc_tc->hcc_transer_info.hcc_queues[HCC_TX].queues[uc_queue_type].flow_ctrl.low_waterline = us_low_waterline;
    g_hcc_tc->hcc_transer_info.hcc_queues[HCC_TX].queues[uc_queue_type].flow_ctrl.high_waterline = us_high_waterline;

    oal_io_print("hcc_host_set_flowctl_param, queue[%d]: burst limit = %d, low_waterline = %d, high_waterline =%d\r\n",
                 uc_queue_type, us_burst_limit, us_low_waterline, us_high_waterline);
    return;
}

void hcc_host_get_flowctl_param(uint8_t uc_queue_type, uint16_t *pus_burst_limit,
                                uint16_t *pus_low_waterline, uint16_t *pus_high_waterline)
{
    if (uc_queue_type >= HCC_QUEUE_COUNT) {
        oal_io_print("CONFIG_ERROR: hcc_host_get_flowctl_param: uc_queue_type = %d\r\n", uc_queue_type);
        return;
    }

    *pus_burst_limit = g_hcc_tc->hcc_transer_info.hcc_queues[HCC_TX].queues[uc_queue_type].burst_limit;
    *pus_low_waterline = g_hcc_tc->hcc_transer_info.hcc_queues[HCC_TX].queues[uc_queue_type].flow_ctrl.low_waterline;
    *pus_high_waterline = g_hcc_tc->hcc_transer_info.hcc_queues[HCC_TX].queues[uc_queue_type].flow_ctrl.high_waterline;
    return;
}

void hcc_host_get_flowctl_stat(void)
{
    uint16_t us_queue_idx;
    hcc_trans_queue *queue = NULL;

    /* 输出各个队列的状态信息 */
    for (us_queue_idx = 0; us_queue_idx < HCC_QUEUE_COUNT; us_queue_idx++) {
        queue = &(g_hcc_tc->hcc_transer_info.hcc_queues[HCC_TX].queues[us_queue_idx]);
        oal_io_print("Q[%d]:bst_lmt[%d],low_wl[%d],high_wl[%d]\r\n",
                     us_queue_idx,
                     queue->burst_limit,
                     queue->flow_ctrl.low_waterline,
                     queue->flow_ctrl.high_waterline);

        oal_io_print("      wlan_q[%d],stoped?[%d],q_len[%d],total_pkt[%d],loss_pkt[%d]\r\n",
                     queue->wlan_queue_id,
                     queue->flow_ctrl.is_stopped,
                     oal_netbuf_list_len(&(queue->data_queue)),
                     queue->total_pkts,
                     queue->loss_pkts);
    }

    return;
}

oal_bool_enum_uint8 hcc_flowctl_get_device_mode(struct hcc_handler *hcc)
{
    if (oal_warn_on(hcc == NULL)) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return OAL_FALSE;
    }

    if (oal_likely(hcc->hcc_transer_info.tx_flow_ctrl.get_mode)) {
        return hcc->hcc_transer_info.tx_flow_ctrl.get_mode();
    }

    return OAL_FALSE;
}

void hcc_flowctl_get_device_mode_register(hcc_flowctl_get_mode get_mode)
{
    if (g_hcc_tc == NULL) {
        oal_io_print("[ERROR]hcc_flowctl_get_device_mode_register failed! g_hcc_tc is NULL\n");
        return;
    }
    g_hcc_tc->hcc_transer_info.tx_flow_ctrl.get_mode = get_mode;
}

void hcc_host_update_vi_flowctl_param(uint32_t be_cwmin, uint32_t vi_cwmin)
{
    uint16_t us_burst_limit;
    uint16_t us_low_waterline;
    uint16_t us_high_waterline;

    /* 如果vi与be的edca参数设置为一致，则更新VI的拥塞控制参数 */
    if (be_cwmin == vi_cwmin) {
        hcc_host_get_flowctl_param(DATA_UDP_BE_QUEUE, &us_burst_limit, &us_low_waterline, &us_high_waterline);
        hcc_host_set_flowctl_param(DATA_UDP_VI_QUEUE, us_burst_limit, us_low_waterline, us_high_waterline);
    } else {  // 否则设置vi的拥塞控制参数为默认值
        hcc_host_set_flowctl_param(DATA_UDP_VI_QUEUE, UDP_VI_BURST_LIMIT,
                                   UDP_VI_LOW_WATERLINE, UDP_VI_HIGH_WATERLINE);
    }
}

/*lint -e19 */
oal_module_symbol(hcc_flowctl_get_device_mode_register);
oal_module_symbol(hcc_flowctl_operate_subq_register);
oal_module_symbol(hcc_host_update_vi_flowctl_param);

/*
 * Prototype    : hcc_thread_process
 * Description  : hcc main thread
 * Input        : struct hcc_handler *hcc
 */
OAL_STATIC int32_t hcc_thread_process(struct hcc_handler *hcc)
{
    int32_t ret = 0;
    oal_bool_enum_uint8 en_device_is_sta;

    en_device_is_sta = hcc_flowctl_get_device_mode(hcc);
    if (en_device_is_sta == OAL_TRUE) {
        /*
         * Tx Tcp Data queue > Rx Tcp Ack
         * Rx Tcp Data > Tx Tcp Ack
         * Tx Tcp Data queue > Rx Tcp Data queue
         */
        ret += hcc_send_rx_queue(hcc, CTRL_QUEUE);
        ret += hcc_send_tx_queue(hcc, CTRL_QUEUE);

        ret += hcc_send_rx_queue(hcc, DATA_HI_QUEUE);
        ret += hcc_send_tx_queue(hcc, DATA_HI_QUEUE);

        /* 下行TCP优先 */
        ret += hcc_send_rx_queue(hcc, DATA_TCP_DATA_QUEUE);
        ret += hcc_send_tx_queue(hcc, DATA_TCP_ACK_QUEUE);

        ret += hcc_send_tx_queue(hcc, DATA_TCP_DATA_QUEUE);
        ret += hcc_send_rx_queue(hcc, DATA_TCP_ACK_QUEUE);

        /* Tx Lo < Rx Lo */
        ret += hcc_send_rx_queue(hcc, DATA_LO_QUEUE);
        ret += hcc_send_tx_queue(hcc, DATA_LO_QUEUE);

        ret += hcc_send_rx_queue(hcc, DATA_UDP_VO_QUEUE);
        ret += hcc_send_tx_queue(hcc, DATA_UDP_VO_QUEUE);

        ret += hcc_send_rx_queue(hcc, DATA_UDP_VI_QUEUE);
        ret += hcc_send_tx_queue(hcc, DATA_UDP_VI_QUEUE);

        ret += hcc_send_rx_queue(hcc, DATA_UDP_BE_QUEUE);
        ret += hcc_send_tx_queue(hcc, DATA_UDP_BE_QUEUE);

        ret += hcc_send_rx_queue(hcc, DATA_UDP_BK_QUEUE);
        ret += hcc_send_tx_queue(hcc, DATA_UDP_BK_QUEUE);
    } else {
        /*
         * Tx Tcp Data queue > Rx Tcp Ack
         * Rx Tcp Data > Tx Tcp Ack
         * Tx Tcp Data queue < Rx Tcp Data queue
         */
        ret += hcc_send_tx_queue(hcc, CTRL_QUEUE);
        ret += hcc_send_rx_queue(hcc, CTRL_QUEUE);

        ret += hcc_send_tx_queue(hcc, DATA_HI_QUEUE);
        ret += hcc_send_rx_queue(hcc, DATA_HI_QUEUE);

        ret += hcc_send_tx_queue(hcc, DATA_TCP_DATA_QUEUE);
        ret += hcc_send_rx_queue(hcc, DATA_TCP_ACK_QUEUE);

        ret += hcc_send_rx_queue(hcc, DATA_TCP_DATA_QUEUE);
        ret += hcc_send_tx_queue(hcc, DATA_TCP_ACK_QUEUE);

        /* Tx Lo < Rx Lo */
        ret += hcc_send_tx_queue(hcc, DATA_LO_QUEUE);
        ret += hcc_send_rx_queue(hcc, DATA_LO_QUEUE);

        /* udp业务 */
        ret += hcc_send_tx_queue(hcc, DATA_UDP_VO_QUEUE);
        ret += hcc_send_rx_queue(hcc, DATA_UDP_VO_QUEUE);

        ret += hcc_send_tx_queue(hcc, DATA_UDP_VI_QUEUE);
        ret += hcc_send_rx_queue(hcc, DATA_UDP_VI_QUEUE);

        ret += hcc_send_tx_queue(hcc, DATA_UDP_BE_QUEUE);
        ret += hcc_send_rx_queue(hcc, DATA_UDP_BE_QUEUE);

        ret += hcc_send_tx_queue(hcc, DATA_UDP_BK_QUEUE);
        ret += hcc_send_rx_queue(hcc, DATA_UDP_BK_QUEUE);
    }

    return ret;
}

void hcc_set_tcpack_cnt(uint32_t ul_val)
{
    g_tcp_ack_wait_sch_cnt = ul_val;
    return;
}

int32_t hcc_transfer_thread(void *data)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    uint32_t group_ret;
    int32_t ret;
    struct hcc_handler *hcc = NULL;
    struct sched_param param;
#ifdef _PRE_WLAN_TCP_OPT
    OAL_STATIC uint8_t ack_loop_count = 0;
#endif

    hcc = (struct hcc_handler *)data;

    if (oal_warn_on(hcc == NULL)) {
        oal_io_print("%s error: hcc is null", __FUNCTION__);
        return OAL_FAIL;
    };

    oal_io_print("hisi wifi hcc transfer thread enter\n");
    param.sched_priority = 1;
    oal_set_thread_property(current,
                            SCHED_FIFO,
                            &param,
                            HCC_TRANS_THERAD_NICE);

    allow_signal(SIGTERM);

    forever_loop() {
        group_ret = 0;
        if (oal_unlikely(kthread_should_stop())) {
            oal_io_print("hisi wifi hcc transfer thread leave\n");
            break;
        }

        if (oal_unlikely((hcc->bus_dev == NULL) || (hcc->bus_dev->cur_bus == NULL))) {
            oal_io_print("bus_dev or cur_bus is null\n");
            msleep(1000);
            continue;
        }

        ret = oal_wait_event_interruptible_m(hcc->hcc_transer_info.hcc_transfer_wq,
                                             (hcc_thread_wait_event_cond_check(hcc) == OAL_TRUE));
        if (oal_unlikely(ret == -ERESTARTSYS)) {
            oal_io_print("wifi task %s was interrupted by a signal\n", oal_get_current_task_name());
            break;
        }
#ifdef _PRE_WLAN_TCP_OPT
        if (hcc->p_hmac_tcp_ack_process_func != NULL) {
            ack_loop_count++;
            if (ack_loop_count >= g_tcp_ack_wait_sch_cnt) {
                ack_loop_count = 0;
                hcc->p_hmac_tcp_ack_process_func();
            }
        }
#endif
        group_ret |= (uint32_t)hcc_bus_pending_signal_process(hcc->bus_dev->cur_bus);
        group_ret |= (uint32_t)hcc_thread_process(hcc);
#ifdef _PRE_CONFIG_WLAN_THRANS_THREAD_DEBUG
        if (group_ret) {
            hcc->hcc_transer_info.thread_stat.loop_have_data_count++;
        } else {
            hcc->hcc_transer_info.thread_stat.loop_no_data_count++;
        }
#else
        oal_reference(group_ret);
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        if (!group_ret) {
            /* 空转 */
            cpu_relax();
        }
#endif
    }
#else
    oal_io_print("hisi wifi hcc transfer thread done!%p\n", data);
#endif
    oal_io_print("hisi wifi hcc transfer thread done!\n");
    return OAL_SUCC;
}
/*lint -e19*/
oal_module_symbol(hcc_transfer_thread);
oal_module_symbol(g_tcp_ack_wait_sch_cnt);
/*lint +e19*/
OAL_STATIC void hcc_dev_flowctr_timer_del(struct hcc_handler *hcc)
{
    if (oal_in_interrupt()) {
        oal_timer_delete(&hcc->hcc_transer_info.tx_flow_ctrl.flow_timer);
    } else {
        oal_timer_delete_sync(&hcc->hcc_transer_info.tx_flow_ctrl.flow_timer);
    }
}

void hcc_dev_flowctrl_on(struct hcc_handler *hcc, uint8_t need_notify_dev)
{
    if (oal_unlikely(hcc == NULL)) {
        oal_warn_on(1);
        return;
    }
    hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_on_count++;
    oal_print_hi11xx_log(HI11XX_LOG_DBG, "start tranferring to device");

    if (hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_flag == D2H_MSG_FLOWCTRL_OFF) {
        hcc_dev_flowctr_timer_del(hcc);
        hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_flag = D2H_MSG_FLOWCTRL_ON;
        hcc_sched_transfer(hcc);
    }

    if (need_notify_dev) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "Host turn on dev flow ctrl...");
        hcc_bus_send_message(hcc_to_bus(hcc), H2D_MSG_FLOWCTRL_ON);
    }
}

void hcc_dev_flowctrl_off(struct hcc_handler *hcc)
{
    if (oal_unlikely(hcc == NULL)) {
        oal_warn_on(1);
        return;
    }
    if (hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_flag == D2H_MSG_FLOWCTRL_ON) {
        oal_timer_start(&hcc->hcc_transer_info.tx_flow_ctrl.flow_timer,
                        hcc->hcc_transer_info.tx_flow_ctrl.timeout);
    }

    hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_flag = D2H_MSG_FLOWCTRL_OFF;
    hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_off_count++;
    oal_print_hi11xx_log(HI11XX_LOG_DBG, "stop tranferring to device");
}

OAL_STATIC void hcc_transfer_queues_init(struct hcc_handler *hcc)
{
    int32_t i, j;
    for (i = 0; i < HCC_DIR_COUNT; i++) {
        for (j = 0; j < HCC_QUEUE_COUNT; j++) {
            oal_netbuf_head_init(&hcc->hcc_transer_info.hcc_queues[i].queues[j].data_queue);
        }
    }
}

OAL_STATIC int32_t hcc_tx_assem_descr_init(struct hcc_handler *hcc)
{
    int32_t i;
    int32_t ret = OAL_SUCC;
    oal_netbuf_stru *netbuf = NULL;

    oal_netbuf_head_init(&hcc->tx_descr_info.tx_assem_descr_hdr);

    /* assem descr ping-pong buff, 2 should be ok */
    hcc->tx_descr_info.descr_num = 2;

    for (i = 0; i < hcc->tx_descr_info.descr_num; i++) {
        netbuf = oal_netbuf_alloc(HISDIO_HOST2DEV_SCATT_SIZE, 0, 0);
        if (netbuf == NULL) {
            /*lint -e801*/
            goto failed_netbuf_alloc;
        }

        oal_netbuf_put(netbuf, HISDIO_HOST2DEV_SCATT_SIZE);
        memset_s(oal_netbuf_data(netbuf), oal_netbuf_len(netbuf), 0, oal_netbuf_len(netbuf));
        oal_netbuf_list_tail(&hcc->tx_descr_info.tx_assem_descr_hdr, netbuf);
        if (oal_warn_on(!oal_is_aligned(((uintptr_t)oal_netbuf_data(netbuf)), 4))) { /* 判断是不是4字节对齐 */
            oam_warning_log1(0, OAM_SF_ANY, "{(oal_uint)oal_netbuf_data(netbuf):%d not align 4}",
                             (uintptr_t)oal_netbuf_data(netbuf));
        }
    }

    oal_build_bug_on(HISDIO_HOST2DEV_SCATT_SIZE < 4);

    return ret;
failed_netbuf_alloc:
    oal_netbuf_list_purge(&hcc->tx_descr_info.tx_assem_descr_hdr);
    return -OAL_ENOMEM;
}

OAL_STATIC void hcc_tx_assem_descr_exit(struct hcc_handler *hcc)
{
    oal_netbuf_list_purge(&hcc->tx_descr_info.tx_assem_descr_hdr);
}

void hcc_tx_assem_info_reset(struct hcc_handler *hcc)
{
    if (oal_unlikely(hcc == NULL)) {
        oal_warn_on(1);
        return;
    }
    memset_s(hcc->hcc_transer_info.tx_assem_info.info, sizeof(hcc->hcc_transer_info.tx_assem_info.info),
             0, sizeof(hcc->hcc_transer_info.tx_assem_info.info));
    hcc->hcc_transer_info.tx_assem_info.pkt_cnt = 0;
}

void oal_sdio_rx_assem_info_reset(struct hcc_handler *hcc)
{
    if (oal_unlikely(hcc == NULL)) {
        oal_warn_on(1);
        return;
    }
    memset_s(hcc->hcc_transer_info.rx_assem_info.info, sizeof(hcc->hcc_transer_info.rx_assem_info.info),
             0, sizeof(hcc->hcc_transer_info.rx_assem_info.info));
    hcc->hcc_transer_info.rx_assem_info.pkt_cnt = 0;
}

void hcc_assem_info_init(struct hcc_handler *hcc)
{
    hcc->hcc_transer_info.tx_assem_info.assemble_max_count = g_hcc_assemble_count;
    hcc_tx_assem_info_reset(hcc);
    oal_sdio_rx_assem_info_reset(hcc);
    oal_netbuf_list_head_init(&hcc->hcc_transer_info.tx_assem_info.assembled_head);
}

OAL_STATIC void hcc_trans_limit_parm_init(struct hcc_handler *hcc)
{
    int32_t i;
    hcc_trans_queues *pst_hcc_tx_queues = &hcc->hcc_transer_info.hcc_queues[HCC_TX];

    for (i = 0; i < HCC_QUEUE_COUNT; i++) {
        hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[i].burst_limit = (uint32_t)HCC_FLUSH_ALL;
        hcc->hcc_transer_info.hcc_queues[HCC_RX].queues[i].burst_limit = (uint32_t)HCC_FLUSH_ALL;
    }

    pst_hcc_tx_queues->queues[CTRL_QUEUE].burst_limit = CTRL_BURST_LIMIT;
    pst_hcc_tx_queues->queues[DATA_HI_QUEUE].burst_limit = DATA_HI_BURST_LIMIT;
    pst_hcc_tx_queues->queues[DATA_LO_QUEUE].burst_limit = DATA_LO_BURST_LIMIT;
    pst_hcc_tx_queues->queues[DATA_TCP_DATA_QUEUE].burst_limit = TCP_DATA_BURST_LIMIT;
    pst_hcc_tx_queues->queues[DATA_TCP_ACK_QUEUE].burst_limit = TCP_ACK_BURST_LIMIT;
    pst_hcc_tx_queues->queues[DATA_UDP_BK_QUEUE].burst_limit = UDP_BK_BURST_LIMIT;
    pst_hcc_tx_queues->queues[DATA_UDP_BE_QUEUE].burst_limit = UDP_BE_BURST_LIMIT;
    pst_hcc_tx_queues->queues[DATA_UDP_VI_QUEUE].burst_limit = UDP_VI_BURST_LIMIT;
    pst_hcc_tx_queues->queues[DATA_UDP_VO_QUEUE].burst_limit = UDP_VO_BURST_LIMIT;
}

OAL_STATIC void hcc_trans_send_mode_init(struct hcc_handler *hcc)
{
    int32_t i;

    for (i = 0; i < HCC_QUEUE_COUNT; i++) {
        hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[i].send_mode = HCC_ASSEM_SEND;
    }
    hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[DATA_HI_QUEUE].send_mode = HCC_SINGLE_SEND;
}

OAL_STATIC OAL_VOLATILE int32_t g_flowctrl_info_flag;
OAL_STATIC void hcc_dev_flow_ctrl_timeout_isr(uintptr_t arg)
{
    struct hcc_handler *hcc = (struct hcc_handler *)arg;
    if (hcc == NULL) {
        oal_io_print("hcc is null\n");
        return;
    }
    /* flowctrl lock too much time. */
    declare_dft_trace_key_info("hcc_flow_lock_timeout", OAL_DFT_TRACE_EXCEP);
    oam_warning_log1(0, OAM_SF_ANY, "{SDIO flow ctrl had off for %u ms, it's a long time}",
                     (uint32_t)hcc->hcc_transer_info.tx_flow_ctrl.timeout);
    g_flowctrl_info_flag = 0;

    /* If work is idle,queue a new work. */
    if (oal_work_is_busy(&hcc->hcc_transer_info.tx_flow_ctrl.worker.work) == 0) {
        oal_queue_delayed_system_work(&hcc->hcc_transer_info.tx_flow_ctrl.worker, 0);
    }
}

void hcc_flowctrl_deadlock_detect_worker(oal_work_stru *pst_flow_work)
{
    struct hcc_handler *hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        oal_io_print("hcc_flowctrl_deadlock_detect_worker hcc is null\n");
        return;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{hcc_flowctrl_deadlock_detect_worker action,%d}", g_flowctrl_info_flag);
    if (g_flowctrl_info_flag == 0) {
        g_flowctrl_info_flag = 1;
        oal_smp_mb();
        /* queue system_wq delay work,and send other message 20ms later. */
        /* print device mem */
        hcc_print_device_mem_info();
        oal_queue_delayed_system_work(&hcc->hcc_transer_info.tx_flow_ctrl.worker, oal_msecs_to_jiffies(20));
    } else if (g_flowctrl_info_flag == 1) {
        /* print device mem */
        if (g_hcc_flowctrl_detect_panic) {
            hcc_trigger_device_panic();
        } else {
            hcc_print_device_mem_info();
        }
    }

    return;
}

void hcc_trans_flow_ctrl_info_reset(struct hcc_handler *hcc)
{
    if (oal_unlikely(hcc == NULL)) {
        oal_warn_on(1);
        return;
    }
    /* disable flow ctrl detect timer */
    hcc_dev_flowctr_timer_del(hcc);
    oal_cancel_delayed_work_sync(&hcc->hcc_transer_info.tx_flow_ctrl.worker);

    hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_flag = D2H_MSG_FLOWCTRL_OFF;
    hcc->hcc_transer_info.tx_flow_ctrl.uc_hipriority_cnt = 0; /* 默认不允许发送等待wcpu更新credit */
}

OAL_STATIC void hcc_trans_flow_ctrl_info_init(struct hcc_handler *hcc)
{
    int32_t i;
    hcc_trans_queues *hcc_tx_queues = &(hcc->hcc_transer_info.hcc_queues[HCC_TX]);
    hcc_trans_queues *hcc_rx_queues = &(hcc->hcc_transer_info.hcc_queues[HCC_RX]);
    hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_flag = D2H_MSG_FLOWCTRL_ON;
    hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_off_count = 0;
    hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_on_count = 0;
    oal_spin_lock_init(&hcc->hcc_transer_info.tx_flow_ctrl.lock);
    hcc->hcc_transer_info.tx_flow_ctrl.timeout = 20 * 1000;
    oal_init_delayed_work(&hcc->hcc_transer_info.tx_flow_ctrl.worker, hcc_flowctrl_deadlock_detect_worker);
    oal_timer_init(&hcc->hcc_transer_info.tx_flow_ctrl.flow_timer,
                   hcc->hcc_transer_info.tx_flow_ctrl.timeout,
                   hcc_dev_flow_ctrl_timeout_isr,
                   (uintptr_t)hcc);

    hcc->hcc_transer_info.tx_flow_ctrl.uc_hipriority_cnt = HCC_FLOW_HIGH_PRI_BUFF_CNT;
    oal_spin_lock_init(&hcc->hcc_transer_info.tx_flow_ctrl.st_hipri_lock);

    for (i = 0; i < HCC_QUEUE_COUNT; i++) {
        hcc_tx_queues->queues[i].flow_ctrl.enable = OAL_TRUE;
        hcc_tx_queues->queues[i].flow_ctrl.flow_type = HCC_FLOWCTRL_SDIO;
        hcc_tx_queues->queues[i].flow_ctrl.is_stopped = OAL_FALSE;
        hcc_tx_queues->queues[i].flow_ctrl.low_waterline = HCC_TX_LOW_WATERLINE;
        hcc_tx_queues->queues[i].flow_ctrl.high_waterline = HCC_TX_HIGH_WATERLINE;
        hcc_tx_queues->queues[i].netbuf_pool_type = HCC_NETBUF_NORMAL_QUEUE;
    }

    hcc_tx_queues->queues[DATA_HI_QUEUE].flow_ctrl.flow_type = HCC_FLOWCTRL_CREDIT;

    for (i = 0; i < HCC_QUEUE_COUNT; i++) {
        hcc_rx_queues->queues[i].flow_ctrl.enable = OAL_TRUE;
        hcc_tx_queues->queues[i].flow_ctrl.is_stopped = OAL_FALSE;
        hcc_rx_queues->queues[i].flow_ctrl.low_waterline = HCC_RX_LOW_WATERLINE;
        hcc_rx_queues->queues[i].flow_ctrl.high_waterline = HCC_RX_HIGH_WATERLINE;
    }

    /* DEVICE 没有给高优先级预留内存，所有队列都需要流控。 */
    hcc_tx_queues->queues[DATA_HI_QUEUE].flow_ctrl.enable = OAL_FALSE;

    hcc_tx_queues->queues[CTRL_QUEUE].flow_ctrl.low_waterline = CTRL_LOW_WATERLINE;
    hcc_tx_queues->queues[CTRL_QUEUE].flow_ctrl.high_waterline = CTRL_HIGH_WATERLINE;
    hcc_tx_queues->queues[CTRL_QUEUE].netbuf_pool_type = HCC_NETBUF_HIGH_QUEUE;

    hcc_tx_queues->queues[DATA_HI_QUEUE].flow_ctrl.low_waterline = DATA_HI_LOW_WATERLINE;
    hcc_tx_queues->queues[DATA_HI_QUEUE].flow_ctrl.high_waterline = DATA_HI_HIGH_WATERLINE;
    hcc_tx_queues->queues[DATA_HI_QUEUE].netbuf_pool_type = HCC_NETBUF_HIGH_QUEUE;

    hcc_tx_queues->queues[DATA_LO_QUEUE].flow_ctrl.low_waterline = DATA_LO_LOW_WATERLINE;
    hcc_tx_queues->queues[DATA_LO_QUEUE].flow_ctrl.high_waterline = DATA_LO_HIGH_WATERLINE;

    hcc_tx_queues->queues[DATA_TCP_DATA_QUEUE].flow_ctrl.low_waterline = TCP_DATA_LOW_WATERLINE;
    hcc_tx_queues->queues[DATA_TCP_DATA_QUEUE].flow_ctrl.high_waterline = TCP_DATA_HIGH_WATERLINE;

    hcc_tx_queues->queues[DATA_TCP_ACK_QUEUE].flow_ctrl.low_waterline = TCP_ACK_LOW_WATERLINE;
    hcc_tx_queues->queues[DATA_TCP_ACK_QUEUE].flow_ctrl.high_waterline = TCP_ACK_HIGH_WATERLINE;

    hcc_tx_queues->queues[DATA_UDP_BK_QUEUE].flow_ctrl.low_waterline = UDP_BK_LOW_WATERLINE;
    hcc_tx_queues->queues[DATA_UDP_BK_QUEUE].flow_ctrl.high_waterline = UDP_BK_HIGH_WATERLINE;

    hcc_tx_queues->queues[DATA_UDP_BE_QUEUE].flow_ctrl.low_waterline = UDP_BE_LOW_WATERLINE;
    hcc_tx_queues->queues[DATA_UDP_BE_QUEUE].flow_ctrl.high_waterline = UDP_BE_HIGH_WATERLINE;

    hcc_tx_queues->queues[DATA_UDP_VI_QUEUE].flow_ctrl.low_waterline = UDP_VI_LOW_WATERLINE;
    hcc_tx_queues->queues[DATA_UDP_VI_QUEUE].flow_ctrl.high_waterline = UDP_VI_HIGH_WATERLINE;

    hcc_tx_queues->queues[DATA_UDP_VO_QUEUE].flow_ctrl.low_waterline = UDP_VO_LOW_WATERLINE;
    hcc_tx_queues->queues[DATA_UDP_VO_QUEUE].flow_ctrl.high_waterline = UDP_VO_HIGH_WATERLINE;
}
int32_t hcc_flow_on_callback(void *data)
{
    hcc_dev_flowctrl_on((struct hcc_handler *)data, 0);
    return OAL_SUCC;
}

int32_t hcc_flow_off_callback(void *data)
{
    hcc_dev_flowctrl_off((struct hcc_handler *)data);
    return OAL_SUCC;
}

/*
 * 函 数 名  : hcc_credit_update_callback
 * 功能描述  : D2H_MSG_CREDIT_UPDATE msg对应的处理函数
 */
int32_t hcc_credit_update_callback(void *data)
{
    uint8_t uc_large_cnt;
    struct hcc_handler *hcc = (struct hcc_handler *)data;
    struct oal_sdio *hi_sdio = oal_get_sdio_default_handler();

    if (oal_warn_on(hi_sdio == NULL)) {
        oal_io_print("hcc_credit_update_callback set fail: hi_sdio is null!\n");
        return OAL_FAIL;
    }

    uc_large_cnt = hisdio_large_pkt_get(hi_sdio->sdio_extend->credit_info);

    oal_spin_lock(&(hcc->hcc_transer_info.tx_flow_ctrl.st_hipri_lock));

    hcc->hcc_transer_info.tx_flow_ctrl.uc_hipriority_cnt = uc_large_cnt;

    oal_spin_unlock(&(hcc->hcc_transer_info.tx_flow_ctrl.st_hipri_lock));

    hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_hipri_update_count++;

    hcc_sched_transfer(hcc);

    return OAL_SUCC;
}

int32_t hcc_high_pkts_loss_callback(void *data)
{
    oal_reference(data);
    declare_dft_trace_key_info("sdio_high_pkts_loss", OAL_DFT_TRACE_EXCEP);
    return OAL_SUCC;
}

/*
 * Prototype    : hcc_module_init
 * Description  : hcc init function, also probe sdio function.
 * Input        : hcc_msg_rx dev_ready_cb   the callback function called when device ready
 *                void* init_data       the input param for the callback function.
 * Return Value : struct hcc_handler*
 */
struct hcc_handler *hcc_module_init(hcc_bus_dev *pst_bus_dev)
{
    uint32_t ul_tx_max_len;
    struct hcc_handler *hcc = NULL;

    oal_build_bug_on(HCC_HDR_LEN > HCC_HDR_TOTAL_LEN);

    /* main_type:4 只能表示16种类型 */
    oal_build_bug_on(HCC_ACTION_TYPE_BUTT > 15);

    if (oal_warn_on(pst_bus_dev == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: pst_bus_dev is null", __FUNCTION__);
        return NULL;
    }

    /* 1544-the max netbuf len of device */
    /*lint -e778*/
    g_hcc_tx_max_buf_len = OAL_ROUND_DOWN(HSDIO_HOST2DEV_PKTS_MAX_LEN, HISDIO_H2D_SCATT_BUFFLEN_ALIGN);
    /* one byte,2^8=256 */
    ul_tx_max_len = (1 << (8 + HISDIO_H2D_SCATT_BUFFLEN_ALIGN_BITS));
    g_hcc_tx_max_buf_len = oal_min(g_hcc_tx_max_buf_len, ul_tx_max_len);
    /*lint +e778*/
    hcc = (struct hcc_handler *)oal_memalloc(sizeof(struct hcc_handler));
    if (hcc == NULL) {
        oal_io_print("hcc mem alloc  failed!\n");
        return NULL;
    }

    memset_s((void *)hcc, sizeof(struct hcc_handler), 0, sizeof(struct hcc_handler));

    hcc->bus_dev = pst_bus_dev;

    hcc->hdr_rever_max_len = HCC_HDR_RESERVED_MAX_LEN;

    /* disable hcc default */
    oal_atomic_set(&hcc->state, HCC_OFF);

    oal_wait_queue_init_head(&hcc->hcc_transer_info.hcc_transfer_wq); /* queues init */
    hcc_transfer_queues_init(hcc);

    hcc_trans_flow_ctrl_info_init(hcc);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mutex_init(&hcc->tx_transfer_lock);
#endif

    oal_wait_queue_init_head(&hcc->hcc_transer_info.tx_flow_ctrl.wait_queue);

#if !(defined(_PRE_WLAN_TCP_OPT) || defined(WIN32))
    hcc->hcc_transer_info.hcc_transfer_thread = oal_thread_create(hcc_transfer_thread,
                                                                  hcc,
                                                                  NULL,
                                                                  "hisi_hcc",
                                                                  HCC_TRANS_THREAD_POLICY,
                                                                  HCC_TRANS_THERAD_PRIORITY,
                                                                  -1);
    if (!hcc->hcc_transer_info.hcc_transfer_thread) {
        oal_io_print("hcc thread create failed!\n");
        goto failed_create_hcc_thread;
    }
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (hcc_transfer_rx_register(hcc, (void *)hcc, hcc_bus_rx_handler) != OAL_SUCC) {
        oal_io_print("sdio rx transfer callback register failed!\n");
        goto failed_rx_cb_reg;
    }
#endif

    hcc_assem_info_init(hcc);
    hcc_trans_limit_parm_init(hcc);
    hcc_trans_send_mode_init(hcc);

    oal_build_bug_on(!oal_is_aligned(HISDIO_HOST2DEV_SCATT_SIZE, HISDIO_H2D_SCATT_BUFFLEN_ALIGN));

    if (hcc_tx_assem_descr_init(hcc) != OAL_SUCC) {
        oal_io_print("hcc tx assem descrt alloc failed!\n");
        goto failed_tx_assem_descr_alloc;
    }
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (hcc_message_register(hcc, D2H_MSG_FLOWCTRL_ON, hcc_flow_on_callback, hcc) != OAL_SUCC) {
        oal_io_print("register flow ctrl on failed!\n");
        goto failed_reg_flowon_msg;
    }

    if (hcc_message_register(hcc, D2H_MSG_FLOWCTRL_OFF, hcc_flow_off_callback, hcc) != OAL_SUCC) {
        oal_io_print("register flow ctrl off failed!\n");
        goto failed_reg_flowoff_msg;
    }

    hcc_message_register(hcc, D2H_MSG_CREDIT_UPDATE, hcc_credit_update_callback, hcc);

    hcc_message_register(hcc, D2H_MSG_HIGH_PKT_LOSS, hcc_high_pkts_loss_callback, hcc);
#endif
    if (hcc_test_init_module(hcc) != OAL_SUCC) {
        oal_io_print("register flow ctrl off failed!\n");
        goto failed_hcc_test_init;
    }

    oal_wake_lock_init(&hcc->tx_wake_lock, "hcc_tx");

    oal_io_print("hcc_module_init dev id:%d succ\n", pst_bus_dev->dev_id);

    /* g_hcc_tc is used to test! */
    g_hcc_tc = hcc;

    return hcc;
failed_hcc_test_init:
    hcc_message_unregister(hcc, D2H_MSG_FLOWCTRL_OFF);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
failed_reg_flowoff_msg:
    hcc_message_unregister(hcc, D2H_MSG_FLOWCTRL_ON);
failed_reg_flowon_msg:
#endif
    hcc_tx_assem_descr_exit(hcc);
failed_tx_assem_descr_alloc:
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hcc_bus_transfer_rx_unregister(hcc_to_bus(hcc));
failed_rx_cb_reg:
#endif
#if !defined(_PRE_WLAN_TCP_OPT) || defined(WIN32)
    oal_thread_stop(hcc->hcc_transer_info.hcc_transfer_thread, NULL);
    hcc->hcc_transer_info.hcc_transfer_thread = NULL;
#endif
#if !(defined(_PRE_WLAN_TCP_OPT) || defined(WIN32))
failed_create_hcc_thread:
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mutex_destroy(&hcc->tx_transfer_lock);
#endif
#endif
    oal_free(hcc);
    /*lint +e801*/
    return NULL;
}

/*
 * Prototype    : hcc_module_exit
 * Description  : the exit function for hcc.
 * Input        : struct hcc_handler* hcc
 * Return Value : struct hcc_handler*
 */
void hcc_module_exit(struct hcc_handler *hcc)
{
    if (oal_unlikely(hcc == NULL)) {
        oal_warn_on(1);
        return;
    }

    oal_wake_lock_exit(&hcc->tx_wake_lock);
    hcc_test_exit_module(hcc);
    hcc_message_unregister(hcc, D2H_MSG_FLOWCTRL_OFF);
    hcc_message_unregister(hcc, D2H_MSG_FLOWCTRL_ON);
    hcc_tx_assem_descr_exit(hcc);
    hcc_bus_transfer_rx_unregister(hcc_to_bus(hcc));
#if !defined(_PRE_WLAN_TCP_OPT) || defined(WIN32)
    oal_thread_stop(hcc->hcc_transer_info.hcc_transfer_thread, NULL);
    hcc->hcc_transer_info.hcc_transfer_thread = NULL;
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mutex_destroy(&hcc->tx_transfer_lock);
#endif

    oal_timer_delete_sync(&hcc->hcc_transer_info.tx_flow_ctrl.flow_timer);
    oal_cancel_delayed_work_sync(&hcc->hcc_transer_info.tx_flow_ctrl.worker);

    oal_free(hcc);
    g_hcc_tc = NULL;
}

#ifdef CONFIG_PRINTK
void hcc_sched_transfer_test(void)
{
    hcc_sched_transfer(g_hcc_tc);
}

void hcc_dev_flowctrl_on_test(void)
{
    if (g_hcc_tc != NULL) {
        hcc_dev_flowctrl_on(g_hcc_tc, 1);
    }
}
#endif
#ifdef CONFIG_MMC
void hcc_device_info_dump(void)
{
    struct hcc_handler *hcc = hcc_get_110x_handler();
    oal_io_print("hcc_device_info_dump\n");
    hcc_bus_send_message(hcc_to_bus(hcc), H2D_MSG_DEVICE_INFO_DUMP);
}

void hcc_device_mem_dump(void)
{
    struct hcc_handler *hcc = hcc_get_110x_handler();
    oal_io_print("hcc_device_mem_dump\n");
    hcc_bus_send_message(hcc_to_bus(hcc), H2D_MSG_DEVICE_MEM_DUMP);
}
#endif
/*lint -e19 */
void hcc_trigger_device_panic(void)
{
#ifdef CONFIG_MMC
    struct hcc_handler *hcc = hcc_get_110x_handler();
    wlan_pm_disable();
    hcc_bus_send_message(hcc_to_bus(hcc), H2D_MSG_TEST);
    wlan_pm_enable();
#endif
}
oal_module_symbol(hcc_trigger_device_panic);

void hcc_print_device_mem_info(void)
{
#ifdef CONFIG_MMC
    struct hcc_handler *hcc = hcc_get_110x_handler();

    wlan_pm_disable();
    hcc_bus_send_message(hcc_to_bus(hcc), H2D_MSG_DEVICE_MEM_INFO);
    wlan_pm_enable();
#endif
}
oal_module_symbol(hcc_get_max_buf_len);
oal_module_symbol(hcc_print_device_mem_info);

oal_module_symbol(hcc_host_set_flowctl_param);
oal_module_symbol(hcc_host_get_flowctl_param);
oal_module_symbol(hcc_host_get_flowctl_stat);
