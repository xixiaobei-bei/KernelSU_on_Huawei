

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define HI11XX_LOG_MODULE_NAME "[PCIE_ETE]"
#define HISI_LOG_TAG           "[PCIE]"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/syscalls.h>

#include "pcie_linux.h"
#include "oal_hcc_host_if.h"

#include "plat_firmware.h"
#include "securec.h"
#include "hi1106/pcie_ctrl_rb_regs.h"

/* netbuf头体是否分离,需要和device同开关 */
uint32_t g_ete_trans_buf_split = 1;

int32_t oal_ete_task_init(oal_pcie_res *pst_pcie_res);
void oal_ete_task_exit(oal_pcie_res *pst_pcie_res);

OAL_STATIC pcie_callback_stru g_pcie_ete_intx_callback[PCIE_HOST_CTL_INTR_BUTT];

/* ete tx config info */
OAL_STATIC oal_ete_chan_h2d_res g_pcie_h2d_res_cfg[ETE_TX_CHANN_MAX_NUMS] = {
    /* high queue */
    [ETE_TX_CHANN_0] = {
        .ring_max_num = PCIE_ETE_TX_CHAN0_ENTRY_NUM,
        .enabled = 1,
    },
    /* tae message */
    [ETE_TX_CHANN_1] = {
        .ring_max_num = PCIE_ETE_TX_CHAN1_ENTRY_NUM,
        .enabled = 0,
    },
    /* low queue */
    [ETE_TX_CHANN_2] = {
        .ring_max_num = PCIE_ETE_TX_CHAN2_ENTRY_NUM,
        .enabled = 1,
    },
    /* reserved */
    [ETE_TX_CHANN_3] = {
        .ring_max_num = 0,
        .enabled = 0,
    },
};

#define ETE_D2H_MAX_BUFF_LEN (HCC_HDR_TOTAL_LEN + PCIE_EDMA_TRANS_MAX_FRAME_LEN)
/* ete rx config info */
OAL_STATIC oal_ete_chan_d2h_res g_pcie_d2h_res_config[ETE_RX_CHANN_MAX_NUMS] = {
    /* normal queue */
    [ETE_RX_CHANN_0] = {
        .ring_max_num = PCIE_ETE_RX_CHAN0_ENTRY_NUM,
        .max_len = ETE_D2H_MAX_BUFF_LEN,
        .enabled = 1,
    },
    /* tae message */
    [ETE_RX_CHANN_1] = {
        .ring_max_num = PCIE_ETE_RX_CHAN1_ENTRY_NUM,
        .max_len = ETE_D2H_MAX_BUFF_LEN,
        .enabled = 0,
    },
    /* reserved */
    [ETE_RX_CHANN_2] = {
        .ring_max_num = 0,
        .max_len = ETE_D2H_MAX_BUFF_LEN,
        .enabled = 0,
    },
    /* reserved */
    [ETE_RX_CHANN_3] = {
        .ring_max_num = 0,
        .max_len = ETE_D2H_MAX_BUFF_LEN,
        .enabled = 0,
    },
    /* reserved */
    [ETE_RX_CHANN_4] = {
        .ring_max_num = 0,
        .max_len = ETE_D2H_MAX_BUFF_LEN,
        .enabled = 0,
    },
};

OAL_STATIC OAL_INLINE uint32_t oal_ete_get_netbuf_step(void)
{
    /* 2 is split buff, 1 is not */
    return g_ete_trans_buf_split ? 2 : 1;
}

OAL_STATIC OAL_INLINE int32_t oal_ete_rx_thread_condtion(oal_atomic *pst_ato)
{
    int32_t ret = oal_atomic_read(pst_ato);
    if (oal_likely(ret == 1)) {
        oal_atomic_set(pst_ato, 0);
    }

    return ret;
}

/* 调度接收线程，补充rx dr描述符 */
OAL_STATIC OAL_INLINE void oal_ete_shced_rx_thread(oal_ete_chan_d2h_res *pst_d2h_res)
{
    oal_atomic_set(&pst_d2h_res->rx_cond, 1);
    oal_wait_queue_wake_up_interrupt(&pst_d2h_res->rx_wq);
}

void oal_ete_sched_rx_threads(oal_pcie_res *pst_pci_res)
{
    uint32_t chan_idx;
    oal_ete_chan_d2h_res *pst_d2h_res = NULL;
    for (chan_idx = 0; chan_idx < ETE_RX_CHANN_MAX_NUMS; chan_idx++) {
        pst_d2h_res = &pst_pci_res->ete_info.d2h_res[chan_idx];
        if (pst_d2h_res->enabled == 0) {
            continue;
        }
        oal_ete_shced_rx_thread(pst_d2h_res);
    }
}

void oal_ete_sched_hcc_thread(oal_pcie_res *pst_pci_res)
{
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pcie_res_to_dev(pst_pci_res));
    /* 发送完成,唤醒发送线程 */
    if (oal_likely(pst_pci_lres != NULL)) {
        if (oal_likely(pst_pci_lres->pst_bus)) {
            hcc_sched_transfer(hbus_to_hcc(pst_pci_lres->pst_bus));
        } else {
            pci_print_log(PCI_LOG_ERR, "lres's bus is null! %p", pst_pci_lres);
        }
    }
}

OAL_STATIC OAL_INLINE int32_t oal_pcie_process_ete_host_intr(oal_pcie_res *pst_pci_res, unsigned int status)
{
    unsigned int bit;
    if (status == 0) {
        return OAL_SUCC;
    }

    /* 从bit0遍历 */
    for (bit = 0; bit < sizeof(status) * 8; bit++) { /* 8 is bits of bytes */
        unsigned int mask = 1 << bit;
        if (status & mask) {
            if (oal_likely((bit < PCIE_HOST_CTL_INTR_BUTT) && (g_pcie_ete_intx_callback[bit].pf_func != NULL))) {
                g_pcie_ete_intx_callback[bit].pf_func(g_pcie_ete_intx_callback[bit].para);
            } else {
                oal_print_hi11xx_log(HI11XX_LOG_WARN, "unregister host intr:%u", bit);
            }
            status &= ~mask;
        }

        if (!status) { /* all done */
            break;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE int32_t oal_pcie_process_ete_trans_tx(oal_pcie_res *pst_pci_res,
                                                            unsigned int done, unsigned int err)
{
    unsigned int chan_idx;
    if (!(done | err)) {
        return OAL_SUCC;
    }

    for (chan_idx = 0; chan_idx < ETE_TX_CHANN_MAX_NUMS; chan_idx++) {
        /* process chan tx id */
        unsigned int mask = 1 << chan_idx;
        if (done & mask) {
            oal_print_hi11xx_log(HI11XX_LOG_DBG,
                                 "tx chan %u done, trans count:0x%x sr_headptr:0x%x, sr_tailptr:0x%x",
                                 chan_idx,
                                 pcie_ete_get_tx_done_total_cnt((uintptr_t)pst_pci_res->pst_ete_base, chan_idx),
                                 pcie_ete_get_tx_sr_head_ptr((uintptr_t)pst_pci_res->pst_ete_base, chan_idx),
                                 pcie_ete_get_tx_sr_tail_ptr((uintptr_t)pst_pci_res->pst_ete_base, chan_idx));
            done &= ~mask;

            /* sched tx queue 调发送线程,复用hcc */
            oal_atomic_set(&pst_pci_res->ete_info.h2d_res[chan_idx].tx_sync_cond, 1);
            oal_ete_sched_hcc_thread(pst_pci_res);
        }
        if (err & mask) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "tx chan %u err", chan_idx);
            declare_dft_trace_key_info("tx_chan_error", OAL_DFT_TRACE_EXCEP);
            err &= ~mask;
        }
        if (!(done | err)) {
            break;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE int32_t oal_pcie_process_ete_trans_rx(oal_pcie_res *pst_pci_res,
                                                            unsigned int done, unsigned int err)
{
    unsigned int chan_idx;
    if (!(done | err)) {
        return OAL_SUCC;
    }

    for (chan_idx = 0; chan_idx < ETE_RX_CHANN_MAX_NUMS; chan_idx++) {
        /* process chan tx id */
        unsigned int mask = 1 << chan_idx;
        if (done & mask) {
            oal_print_hi11xx_log(HI11XX_LOG_DBG,
                                 "rx chan %u done, trans count:0x%x dr_headptr:0x%x, dr_tailptr:0x%x",
                                 chan_idx,
                                 pcie_ete_get_rx_done_total_cnt((uintptr_t)pst_pci_res->pst_ete_base, chan_idx),
                                 pcie_ete_get_rx_dr_head_ptr((uintptr_t)pst_pci_res->pst_ete_base, chan_idx),
                                 pcie_ete_get_rx_dr_tail_ptr((uintptr_t)pst_pci_res->pst_ete_base, chan_idx));
            oal_ete_shced_rx_thread(&pst_pci_res->ete_info.d2h_res[chan_idx]);
            done &= ~mask;
        }
        if (err & mask) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "rx chan %u err", chan_idx);
            declare_dft_trace_key_info("rx_chan_error", OAL_DFT_TRACE_EXCEP);
            err &= ~mask;
        }
        if (!(done | err)) {
            break;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE int32_t oal_pcie_process_ete_trans_intr(oal_pcie_res *pst_pci_res, unsigned int status)
{
    hreg_pcie_ctl_ete_intr_sts pcie_ctl_ete_intr_sts;
    pcie_ctl_ete_intr_sts.as_dword = status;

    oal_pcie_process_ete_trans_tx(pst_pci_res, pcie_ctl_ete_intr_sts.bits.host_tx_done_intr_status,
                                  pcie_ctl_ete_intr_sts.bits.host_tx_err_intr_status);

    oal_pcie_process_ete_trans_rx(pst_pci_res, pcie_ctl_ete_intr_sts.bits.host_rx_done_intr_status,
                                  pcie_ctl_ete_intr_sts.bits.host_rx_err_intr_status);

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE int32_t oal_pcie_process_ete_dre_tx(oal_pcie_res *pst_pci_res,
                                                          unsigned int tx_dr_empty_sts)
{
    unsigned int chan_idx;
    if (!tx_dr_empty_sts) {
        return OAL_SUCC;
    }

    for (chan_idx = 0; chan_idx < ETE_TX_CHANN_MAX_NUMS; chan_idx++) {
        /* process chan tx id */
        unsigned int mask = 1 << chan_idx;
        if (tx_dr_empty_sts & mask) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "tx dr empty sts %u done", chan_idx);
            /* tx dr empty should done in wcpu */
            tx_dr_empty_sts &= ~mask;
        }

        if (!tx_dr_empty_sts) {
            break;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE int32_t oal_pcie_process_ete_dre_rx(oal_pcie_res *pst_pci_res,
                                                          unsigned int rx_dr_empty_sts)
{
    unsigned int chan_idx;

    if (!rx_dr_empty_sts) {
        return OAL_SUCC;
    }

    for (chan_idx = 0; chan_idx < ETE_RX_CHANN_MAX_NUMS; chan_idx++) {
        /* process chan tx id */
        unsigned int mask = 1 << chan_idx;
        if (rx_dr_empty_sts & mask) {
            oal_print_hi11xx_log(HI11XX_LOG_DBG, "rx dr empty sts %u done", chan_idx);
            /* 补充内存 */
            oal_ete_shced_rx_thread(&pst_pci_res->ete_info.d2h_res[chan_idx]);
            rx_dr_empty_sts &= ~mask;
        }

        if (!rx_dr_empty_sts) {
            break;
        }
    }

    return OAL_SUCC;
}

/* ete dr empty intr */
OAL_STATIC OAL_INLINE int32_t oal_pcie_process_ete_dre_intr(oal_pcie_res *pst_pci_res, unsigned int status)
{
    hreg_pcie_ctrl_ete_ch_dr_empty_intr_sts dr_empty_sts;
    dr_empty_sts.as_dword = status;

    oal_pcie_process_ete_dre_tx(pst_pci_res, dr_empty_sts.bits.host_tx_ch_dr_empty_intr_status);
    oal_pcie_process_ete_dre_rx(pst_pci_res, dr_empty_sts.bits.host_rx_ch_dr_empty_intr_status);
    return OAL_SUCC;
}

int32_t oal_pcie_transfer_ete_done(oal_pcie_res *pst_pci_res)
{
    int32_t ret;

    hreg_host_intr_status intr_status;
    hreg_pcie_ctl_ete_intr_sts pcie_ctl_ete_intr_sts;
    hreg_pcie_ctrl_ete_ch_dr_empty_intr_sts dr_empty_sts;
    void *pst_host_int_status = NULL;
    void *pst_host_int_clr = NULL;
    void *pst_ete_intr_sts = NULL;
    void *pst_ete_intr_clr = NULL;
    void *pst_ete_dr_empty_sts = NULL;
    void *pst_ete_dr_empty_clr = NULL;

    if (oal_unlikely(pst_pci_res->link_state < PCI_WLAN_LINK_MEM_UP)) {
        pci_print_log(PCI_LOG_ERR, "ete int link invaild, link_state:%s",
                      oal_pcie_get_link_state_str((pst_pci_res->link_state)));
        pst_pci_res->stat.done_err_cnt++;
        return -OAL_EBUSY;
    }

    if (oal_unlikely(pst_pci_res->pst_pci_ctrl_base == NULL)) {
        return -OAL_ENODEV;
    }

    pst_host_int_status = pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_STATUS_OFF;
    pst_host_int_clr = pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_CLR_OFF;
    pst_ete_intr_sts = pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_PCIE_CTL_ETE_INTR_STS_OFF;
    pst_ete_intr_clr = pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_PCIE_CTL_ETE_INTR_CLR_OFF;
    pst_ete_dr_empty_sts = pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_STS_OFF;
    pst_ete_dr_empty_clr = pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_CLR_OFF;

    hreg_get_val(intr_status, pst_host_int_status);
    hreg_get_val(pcie_ctl_ete_intr_sts, pst_ete_intr_sts);
    hreg_get_val(dr_empty_sts, pst_ete_dr_empty_sts);

    /* 先清中断 */
    if (intr_status.as_dword != 0) {
        hreg_set_val(intr_status, pst_host_int_clr);
    }
    if (pcie_ctl_ete_intr_sts.as_dword != 0) {
        hreg_set_val(pcie_ctl_ete_intr_sts, pst_ete_intr_clr);
    }
    if (dr_empty_sts.as_dword != 0) {
        hreg_set_val(dr_empty_sts, pst_ete_dr_empty_clr);
    }

    oal_print_hi11xx_log(HI11XX_LOG_DBG, "intr_status=0x%x, ete_intr_sts=0x%x dr_empty_sts=0x%x",
                         intr_status.as_dword, pcie_ctl_ete_intr_sts.as_dword, dr_empty_sts.as_dword);

    if (oal_unlikely((intr_status.as_dword | pcie_ctl_ete_intr_sts.as_dword | dr_empty_sts.as_dword)
        == 0)) {
        return 0;
    }

    if (oal_unlikely(dr_empty_sts.as_dword == 0xFFFFFFFF)) { // 判断最后一个PCIE MEM读操作
        if (oal_pcie_check_link_state(pst_pci_res) == OAL_FALSE) {
            return -OAL_ENODEV;
        }
    }

    ret = oal_pcie_process_ete_host_intr(pst_pci_res, intr_status.as_dword);
    if (oal_unlikely(ret)) {
        return ret;
    }

    ret = oal_pcie_process_ete_trans_intr(pst_pci_res, pcie_ctl_ete_intr_sts.as_dword);
    if (oal_unlikely(ret)) {
        return ret;
    }

    ret = oal_pcie_process_ete_dre_intr(pst_pci_res, dr_empty_sts.as_dword);
    if (oal_unlikely(ret)) {
        return ret;
    }

    return 0;
}

/* refer to g_pcie_h2d_res_cfg */
OAL_STATIC ete_tx_chann_type g_pcie_hcc2ete_qmap[HCC_NETBUF_QUEUE_BUTT] = {
    [HCC_NETBUF_NORMAL_QUEUE] = ETE_TX_CHANN_2,
    [HCC_NETBUF_HIGH_QUEUE] = ETE_TX_CHANN_0,
    [HCC_NETBUF_HIGH2_QUEUE] = ETE_TX_CHANN_1,
};
OAL_STATIC OAL_INLINE ete_tx_chann_type oal_pcie_hcc_qtype_to_ete_qtype(hcc_netbuf_queue_type qtype)
{
    return g_pcie_hcc2ete_qmap[qtype];
}

OAL_STATIC OAL_INLINE int32_t ete_send_check_param(oal_pcie_res *pst_pci_res, oal_netbuf_head_stru *pst_head)
{
    if (oal_unlikely(pst_pci_res == NULL || pst_head == NULL)) {
        pci_print_log(PCI_LOG_ERR, "invalid input pst_pci_res is %pK, pst_head %pK", pst_pci_res, pst_head);
        return -OAL_EINVAL;
    }

    /* pcie is link */
    if (oal_unlikely(pst_pci_res->link_state <= PCI_WLAN_LINK_DOWN)) {
        pci_print_log(PCI_LOG_WARN, "linkdown hold the pkt, link_state:%s",
                      oal_pcie_get_link_state_str(pst_pci_res->link_state));
        return 0;
    }

    if (oal_unlikely(oal_netbuf_list_empty(pst_head))) {
        return 0;
    }

    return 1;
}

int32_t oal_ete_host_pending_signal_check(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    oal_ete_chan_h2d_res *pst_h2d_res = NULL;
    /* ETE use bit ops */
    for (i = 0; i < ETE_TX_CHANN_MAX_NUMS; i++) {
        pst_h2d_res = &pst_pci_res->ete_info.h2d_res[i];
        if (pst_h2d_res->enabled == 0) {
            continue;
        }

        if (oal_atomic_read(&pst_h2d_res->tx_sync_cond)) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}

/* 释放RX通路的资源 */
void oal_ete_rx_res_clean(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    unsigned long flags;
    oal_ete_chan_d2h_res *pst_d2h_res = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;
    for (i = 0; i < ETE_RX_CHANN_MAX_NUMS; i++) {
        pst_d2h_res = &pst_pci_res->ete_info.d2h_res[i];
        if (pst_d2h_res->enabled == 0) {
            continue;
        }

        forever_loop() {
            /* 释放RX补充队列 */
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "prepare free rxq[%d] len=%d",
                                 i,
                                 oal_netbuf_list_len(&pst_d2h_res->rxq));
            oal_spin_lock_irq_save(&pst_d2h_res->lock, &flags);
            pst_netbuf = oal_netbuf_delist_nolock(&pst_d2h_res->rxq);
            oal_spin_unlock_irq_restore(&pst_d2h_res->lock, &flags);
            if (pst_netbuf == NULL) {
                break;
            }

            oal_pcie_release_rx_netbuf(pst_pci_res, pst_netbuf);
        }

        /* reset ringbuf */
        (void)pcie_ete_ringbuf_reset(&pst_d2h_res->ring);
    }
}

void oal_ete_tx_res_clean(oal_pcie_res *pst_pci_res, uint32_t force_clean)
{
    int32_t i;
    unsigned long flags;
    oal_ete_chan_h2d_res *pst_h2d_res = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;
    for (i = 0; i < ETE_TX_CHANN_MAX_NUMS; i++) {
        pst_h2d_res = &pst_pci_res->ete_info.h2d_res[i];
        if (pst_h2d_res->enabled == 0) {
            continue;
        }
        /* 释放TX补充队列 */
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "prepare free txq[%d] len=%d",
                             i,
                             oal_netbuf_list_len(&pst_h2d_res->txq));
        if (force_clean == OAL_FALSE) {
            /* If force clean false, txq had pkts, it's fatal err.
             * If txq not empty, shouldn't allow sleep! */
            if (oal_netbuf_list_len(&pst_h2d_res->txq) != 0) {
                declare_dft_trace_key_info("txq_not_empty_under_lowerpower", OAL_DFT_TRACE_FAIL);
            }
        }
        forever_loop() {
            oal_spin_lock_irq_save(&pst_h2d_res->lock, &flags);
            pst_netbuf = oal_netbuf_delist_nolock(&pst_h2d_res->txq);
            oal_spin_unlock_irq_restore(&pst_h2d_res->lock, &flags);
            if (pst_netbuf == NULL) {
                break;
            }

            oal_pcie_tx_netbuf_free(pst_pci_res, pst_netbuf);
        }

        oal_atomic_set(&pst_h2d_res->tx_sync_cond, 0);
        (void)pcie_ete_ringbuf_reset(&pst_h2d_res->ring);
    }
}

int32_t oal_ete_tx_transfer_process(oal_pcie_res *pst_pci_res, uint32_t chan_idx)
{
    uint32_t trans_cnt, trans_cnt_t, netbuf_cnt;
    unsigned long flags;
    uint32_t netbuf_trans_cnt = 0;
    struct hcc_handler *hcc = NULL;
    oal_pcie_linux_res *pst_pci_lres = NULL;
    oal_pci_dev_stru *pst_pci_dev = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;
    oal_ete_chan_h2d_res *pst_h2d_res = NULL;
    uint32_t step = oal_ete_get_netbuf_step();
    pst_h2d_res = &pst_pci_res->ete_info.h2d_res[chan_idx];

    trans_cnt = pcie_ete_update_ring_tx_donecnt(&pst_h2d_res->ring,
                                                (uintptr_t)pst_pci_res->pst_ete_base, chan_idx);
    if (oal_likely(trans_cnt != PCIE_ETE_INVALID_RD_VALUE)) {
        trans_cnt_t = trans_cnt;
        netbuf_cnt = oal_netbuf_list_len(&pst_h2d_res->txq);
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "txq trans_cnt=%u, txq len=%u done_cnt=%u",
                             trans_cnt, netbuf_cnt, pst_h2d_res->ring.done_cnt);
        if (trans_cnt) {
            pst_h2d_res->ring.done_cnt += trans_cnt;
        }
        /* release trans_cnt netbufs */
        forever_loop() {
            if (pst_h2d_res->ring.done_cnt < step) {
                break;
            }

            oal_spin_lock_irq_save(&pst_h2d_res->lock, &flags);
            pst_netbuf = oal_netbuf_delist_nolock(&pst_h2d_res->txq);
            oal_spin_unlock_irq_restore(&pst_h2d_res->lock, &flags);
            if (oal_unlikely(pst_netbuf == NULL)) {
                declare_dft_trace_key_info("ete_tx_netbuf_underflow", OAL_DFT_TRACE_FAIL);
                oal_print_hi11xx_log(HI11XX_LOG_ERR,
                                     "ete_tx_netbuf_underflow[%u] trans_cnt=%u, trans_cnt_t=%u, netbuf_cnt=%u",
                                     chan_idx, trans_cnt + 1, trans_cnt_t, netbuf_cnt);
                return -OAL_EFAIL; /* DFR Here */
            }
            oal_pcie_tx_netbuf_free(pst_pci_res, pst_netbuf);
            netbuf_trans_cnt++;
            pst_h2d_res->ring.done_cnt -= step;
        }
        if (netbuf_trans_cnt != 0) {
            pst_pci_dev = pcie_res_to_dev(pst_pci_res);
            pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pci_dev);
            hcc = hbus_to_hcc(pst_pci_lres->pst_bus);
            hcc_tx_assem_count(hcc, netbuf_trans_cnt);
        }
    }

    return OAL_SUCC;
}

int32_t oal_ete_host_pending_signal_process(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    int32_t total_cnt = 0;
    oal_ete_chan_h2d_res *pst_h2d_res = NULL;
    oal_pcie_linux_res *pst_pci_lres;
    oal_pci_dev_stru *pst_pci_dev;
    pst_pci_dev = pcie_res_to_dev(pst_pci_res);
    pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pci_dev);
    if (oal_unlikely(OAL_SUCC != hcc_bus_pm_wakeup_device(pst_pci_lres->pst_bus))) {
        oal_msleep(100); /* wait for a while retry(100ms) */
        for (i = 0; i < ETE_TX_CHANN_MAX_NUMS; i++) {
            pst_h2d_res = &pst_pci_res->ete_info.h2d_res[i];
            oal_atomic_set(&pst_h2d_res->tx_sync_cond, 0);
        }
        return total_cnt;
    }

    for (i = 0; i < ETE_TX_CHANN_MAX_NUMS; i++) {
        pst_h2d_res = &pst_pci_res->ete_info.h2d_res[i];
        if (oal_atomic_read(&pst_h2d_res->tx_sync_cond)) {
            oal_atomic_set(&pst_h2d_res->tx_sync_cond, 0);
            if (oal_ete_tx_transfer_process(pst_pci_res, i) == OAL_SUCC) {
                total_cnt++;
            }
        }
    }

    return total_cnt;
}

OAL_STATIC OAL_INLINE void oal_ete_init_h2d_sr_descr(h2d_sr_descr *tx_sr, uint64_t dst_addr, uint32_t len)
{
    tx_sr->reserved = 0x4321; /* 0x4321 is flag */
    tx_sr->ctrl.dword = 0;
    tx_sr->addr = dst_addr; /* devva */
    tx_sr->ctrl.bits.nbytes = len;
}

OAL_STATIC OAL_INLINE int32_t oal_ete_build_send_netbuf_cb(oal_netbuf_stru *pst_netbuf, oal_pci_dev_stru *pst_pci_dev,
                                                           pcie_cb_dma_res *cb_dma,
                                                           uint64_t pci_dma_addr, uint32_t len)
{
    int32_t ret;

    /* dma地址和长度存在CB字段中，发送完成后释放DMA地址 */
    ret = memcpy_s((uint8_t *)oal_netbuf_cb(pst_netbuf) + sizeof(struct hcc_tx_cb_stru),
                   oal_netbuf_cb_size() - sizeof(struct hcc_tx_cb_stru), cb_dma, sizeof(pcie_cb_dma_res));
    if (oal_unlikely(ret != EOK)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "dma addr copy failed ret=%d", ret);
        declare_dft_trace_key_info("ete tx cb cp failed", OAL_DFT_TRACE_FAIL);
        dma_unmap_single(&pst_pci_dev->dev, pci_dma_addr, len, PCI_DMA_TODEVICE);
        hcc_tx_netbuf_free(pst_netbuf);
    }

    return ret;
}

OAL_STATIC OAL_INLINE void oal_ete_dump_send_netbuf(oal_netbuf_stru *pst_netbuf)
{
    if (pci_dbg_condtion()) {
        uint32_t len = oal_netbuf_len(pst_netbuf);
        const uint32_t ul_max_dump_bytes = 128;
        oal_print_hex_dump(oal_netbuf_data(pst_netbuf),
                           len <
                           ul_max_dump_bytes
                           ? len
                           : ul_max_dump_bytes,
                           HEX_DUMP_GROUP_SIZE, "netbuf: ");
    }
}

// host smmu/iommu addr to ep ip slave access address(+offset 0x80000000)
OAL_STATIC OAL_INLINE int32_t oal_ete_get_send_ep_slave_addr(oal_pcie_res *pst_pci_res, oal_pci_dev_stru *pst_pci_dev,
                                                             oal_netbuf_stru *pst_netbuf,
                                                             uint64_t *dst_addr, uint64_t pci_dma_addr,
                                                             uint32_t len)
{
    int32_t ret = oal_pcie_host_slave_address_switch(pst_pci_res, (uint64_t)pci_dma_addr, dst_addr, OAL_TRUE);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "slave address switch failed ret=%d iova=%llx",
                             ret, (uint64_t)pci_dma_addr);
        dma_unmap_single(&pst_pci_dev->dev, pci_dma_addr, len, PCI_DMA_TODEVICE);
        hcc_tx_netbuf_free(pst_netbuf);
    }
    return ret;
}

OAL_STATIC OAL_INLINE dma_addr_t oal_ete_get_send_dma_addr(oal_pci_dev_stru *pst_pci_dev,
                                                           oal_netbuf_stru *pst_netbuf, uint32_t len)
{
    dma_addr_t pci_dma_addr = dma_map_single(&pst_pci_dev->dev, oal_netbuf_data(pst_netbuf),
                                             len, PCI_DMA_TODEVICE);
    if (oal_unlikely(dma_mapping_error(&pst_pci_dev->dev, pci_dma_addr))) {
        declare_dft_trace_key_info("ete tx map failed", OAL_DFT_TRACE_FAIL);
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "ete tx map failed, va=%p, len=%d", oal_netbuf_data(pst_netbuf),
                             len);
        hcc_tx_netbuf_free(pst_netbuf);
    }

    return pci_dma_addr;
}

OAL_STATIC OAL_INLINE void oal_ete_send_netbuf_h2d_enqueue(oal_ete_chan_h2d_res *pst_h2d_res,
                                                           oal_netbuf_stru *pst_netbuf)
{
    unsigned long flags;
    /* netbuf入队 */
    oal_spin_lock_irq_save(&pst_h2d_res->lock, &flags);
    oal_netbuf_list_tail_nolock(&pst_h2d_res->txq, pst_netbuf);
    oal_spin_unlock_irq_restore(&pst_h2d_res->lock, &flags);
}

OAL_STATIC OAL_INLINE uint32_t oal_ete_write_ringbuf_by_sendbuf(oal_ete_chan_h2d_res *pst_h2d_res, h2d_sr_descr *tx_sr,
                                                                oal_pci_dev_stru *pst_pci_dev,
                                                                oal_netbuf_stru *pst_netbuf,
                                                                dma_addr_t pci_dma_addr,
                                                                uint32_t len)
{
    uint32_t wr_len;
    /* 硬件需要更新head_ptr才启动传输,可以先写ringbuf */
    wr_len = pcie_ete_ringbuf_write(&pst_h2d_res->ring, (uint8_t *)tx_sr);
    if (oal_unlikely(wr_len == 0)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "ete tx ring write failed");
        declare_dft_trace_key_info("ete tx ring write failed", OAL_DFT_TRACE_FAIL);
        dma_unmap_single(&pst_pci_dev->dev, pci_dma_addr, len, PCI_DMA_TODEVICE);
        hcc_tx_netbuf_free(pst_netbuf);
    }
    return wr_len;
}

/* build tx netbuf, dma map & tx descr build */
OAL_STATIC OAL_INLINE int32_t oal_ete_send_netbuf_list_build(oal_pcie_res *pst_pci_res,
                                                             oal_netbuf_head_stru *pst_head,
                                                             oal_ete_chan_h2d_res *pst_h2d_res,
                                                             oal_pci_dev_stru *pst_pci_dev)
{
    int32_t ret;
    int32_t i;
    uint32_t len;
    int32_t free_cnt, send_cnt;
    uint64_t dst_addr;
    pcie_cb_dma_res st_cb_dma;
    int32_t total_cnt = 0;
    dma_addr_t pci_dma_addr;
    h2d_sr_descr st_tx_sr_item;
    oal_netbuf_stru *pst_netbuf = NULL;

    uint32_t step = oal_ete_get_netbuf_step();

    free_cnt = pcie_ete_ringbuf_freecount(&pst_h2d_res->ring);

    send_cnt = oal_netbuf_list_len(pst_head);
    send_cnt = oal_min((free_cnt / step), send_cnt);

    /* freecount & netbuf_cnt 只增不减 */
    for (i = 0; i < send_cnt; i++) {
        free_cnt = pcie_ete_ringbuf_freecount(&pst_h2d_res->ring);
        if (free_cnt < step) {
            declare_dft_trace_key_info("unexpect ete tx ringbuf empty", OAL_DFT_TRACE_FAIL);
            break;
        }

        /* 取netbuf */
        pst_netbuf = oal_netbuf_delist(pst_head);
        if (oal_unlikely(pst_netbuf == NULL)) {
            declare_dft_trace_key_info("unexpect ete txq empty", OAL_DFT_TRACE_FAIL);
            break;
        }

        /* Debug */
        if (oal_unlikely(pst_pci_res->link_state < PCI_WLAN_LINK_WORK_UP)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "oal_pcie_send_netbuf_list loop invaild, link_state:%s",
                                 oal_pcie_get_link_state_str(pst_pci_res->link_state));
            hcc_tx_netbuf_free(pst_netbuf);
            return total_cnt;
        }

        len = oal_netbuf_len(pst_netbuf);

        pci_dma_addr = oal_ete_get_send_dma_addr(pst_pci_dev, pst_netbuf, len);
        if (oal_unlikely(dma_mapping_error(&pst_pci_dev->dev, pci_dma_addr))) {
            continue;
        }

        oal_print_hi11xx_log(HI11XX_LOG_DBG, "ete tx[%d] netbuf 0x%p, dma pa:0x%llx, len=%d", pst_h2d_res->chan_idx,
                             pst_netbuf, (uint64_t)pci_dma_addr, len);
        oal_ete_dump_send_netbuf(pst_netbuf);

        st_cb_dma.paddr.addr = (uint64_t)pci_dma_addr;
        st_cb_dma.len = len;

        ret = oal_ete_build_send_netbuf_cb(pst_netbuf, pst_pci_dev, &st_cb_dma, pci_dma_addr, len);
        if (oal_unlikely(ret != EOK)) {
            // release netbuf before continue;
            continue;
        }

        ret = oal_ete_get_send_ep_slave_addr(pst_pci_res, pst_pci_dev, pst_netbuf,
                                             &dst_addr, (uint64_t)pci_dma_addr, len);
        if (oal_unlikely(ret != OAL_SUCC)) {
            return total_cnt;
        }

        if (g_ete_trans_buf_split) {
            oal_ete_init_h2d_sr_descr(&st_tx_sr_item, dst_addr, HCC_HDR_TOTAL_LEN);
            if (oal_unlikely(oal_ete_write_ringbuf_by_sendbuf(
                pst_h2d_res, &st_tx_sr_item, pst_pci_dev, pst_netbuf, pci_dma_addr, len) == 0)) {
                continue;
            }
            dst_addr += HCC_HDR_TOTAL_LEN;
            len -= HCC_HDR_TOTAL_LEN;
        }

        if (oal_unlikely(len == 0)) {
            len = 4; /* ete 传输0长度的描述符会导致挂死,最少4个字节! */
            declare_dft_trace_key_info("ete tx buff len is zero", OAL_DFT_TRACE_OTHER);
        }

        oal_ete_init_h2d_sr_descr(&st_tx_sr_item, dst_addr, len);
#ifdef _PRE_COMMENT_CODE_
        /* 聚合中断，减少中断产生的次数，简单处理
         * 此处未能聚合中断，需要继续分析empty中断流程 */
        if (i + 1 == send_cnt) { // 需要配合 dr empty使用
#endif
            st_tx_sr_item.ctrl.bits.src_intr = 1;
            st_tx_sr_item.ctrl.bits.dst_intr = 1;
#ifdef _PRE_COMMENT_CODE_
        }
#endif

        /* 硬件需要更新head_ptr才启动传输,可以先写ringbuf */
        if (oal_unlikely(oal_ete_write_ringbuf_by_sendbuf(
            pst_h2d_res, &st_tx_sr_item, pst_pci_dev, pst_netbuf, pci_dma_addr, st_cb_dma.len) == 0)) {
            continue;
        }

        oal_print_hi11xx_log(HI11XX_LOG_DBG, "ete[%d] h2d ring write 0x%llx , len=%d, ctrl=0x%x",
                             pst_h2d_res->chan_idx,
                             dst_addr, st_cb_dma.len,
                             st_tx_sr_item.ctrl.dword);

        oal_ete_send_netbuf_h2d_enqueue(pst_h2d_res, pst_netbuf);

        total_cnt++;
    }

    if (total_cnt) {
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "h2d tx assem count=%d", total_cnt);
        /* 更新最后一个描述符 */
        pcie_ete_ring_update_tx_sr_hptr(&pst_h2d_res->ring,
                                        (uintptr_t)pst_pci_res->pst_ete_base, pst_h2d_res->chan_idx);
    }

    return total_cnt;
}

int32_t oal_ete_send_netbuf_list(oal_pcie_res *pst_pci_res, oal_netbuf_head_stru *pst_head,
                                 hcc_netbuf_queue_type qtype)
{
    int32_t ret;
    int32_t free_cnt, queue_cnt;
    ete_tx_chann_type tx_chan_num;

    uint32_t step = oal_ete_get_netbuf_step();

    oal_pcie_linux_res *pst_pci_lres = NULL;
    oal_pci_dev_stru *pst_pci_dev = NULL;
    oal_ete_chan_h2d_res *pst_h2d_res = NULL;

    ret = ete_send_check_param(pst_pci_res, pst_head);
    if (ret <= 0) {
        return ret;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pci_res);
    pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pci_dev);
    /* 发送唤醒 */
    if (oal_unlikely(OAL_SUCC != hcc_bus_pm_wakeup_device(pst_pci_lres->pst_bus))) {
        oal_msleep(100); /* wakeup failed, wait 100ms block the send thread */
        return -OAL_EIO;
    }

    tx_chan_num = oal_pcie_hcc_qtype_to_ete_qtype(qtype);
    pst_h2d_res = &pst_pci_res->ete_info.h2d_res[tx_chan_num];

    queue_cnt = oal_netbuf_list_len(pst_head);
    free_cnt = pcie_ete_ringbuf_freecount(&pst_h2d_res->ring);
    if (queue_cnt > (free_cnt / step)) {
        
        ret = oal_ete_tx_transfer_process(pst_pci_res, tx_chan_num);
        if (oal_likely(ret == OAL_SUCC)) {
            free_cnt = pcie_ete_ringbuf_freecount(&pst_h2d_res->ring);
        }
    }

    if (free_cnt < step) {
        /* ringbuf 为满，挂起发送线程 */
        return 0;
    }

    oal_print_hi11xx_log(HI11XX_LOG_DBG, "[txq:%d]h2d_ringbuf freecount:%u, qlen:%u", (int32_t)tx_chan_num, free_cnt,
                         queue_cnt);

    return oal_ete_send_netbuf_list_build(pst_pci_res, pst_head, pst_h2d_res, pst_pci_dev);
}

int32_t oal_pcie_ete_tx_is_idle(oal_pcie_res *pst_pci_res, hcc_netbuf_queue_type qtype)
{
    uint32_t free_cnt;
    uint32_t step = oal_ete_get_netbuf_step();

    if (oal_warn_on(pst_pci_res == NULL)) {
        pci_print_log(PCI_LOG_WARN, "pci res is null");
        return OAL_FALSE;
    }

    /* pcie is link */
    if (oal_unlikely(pst_pci_res->link_state <= PCI_WLAN_LINK_DOWN)) {
        return OAL_FALSE;
    }

    free_cnt = pcie_ete_ringbuf_freecount(&pst_pci_res->ete_info.h2d_res[g_pcie_hcc2ete_qmap[qtype]].ring);
    oal_print_hi11xx_log(HI11XX_LOG_DBG, "tx is idle free_cnt=%u", free_cnt);
    return ((free_cnt >= step) ? OAL_TRUE : OAL_FALSE);
}

/* check ete tx queue */
int32_t oal_ete_sleep_request_host_check(oal_pcie_res *pst_pci_res)
{
    uint32_t chan_idx;
    uint32_t total_cnt = 0;
    oal_ete_chan_h2d_res *pst_h2d_res = NULL;
    for (chan_idx = 0; chan_idx < ETE_TX_CHANN_MAX_NUMS; chan_idx++) {
        pst_h2d_res = &pst_pci_res->ete_info.h2d_res[chan_idx];
        if (pst_h2d_res->enabled == 0) {
            continue;
        }
        total_cnt += oal_netbuf_list_len(&pst_h2d_res->txq);
    }

    return (total_cnt != 0) ? -OAL_EBUSY : OAL_SUCC;
}

int32_t oal_pcie_h2d_int(oal_pcie_res *pst_pci_res)
{
    void *pst_pci_ctrl_h2d_int_base = NULL;
    hreg_host2device_intr_set hreg;

    if (oal_unlikely(pst_pci_res->link_state <= PCI_WLAN_LINK_DOWN)) {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "pcie is linkdown");
        return -OAL_ENODEV;
    }
    hreg.as_dword = 0x0;
    pst_pci_ctrl_h2d_int_base = pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST2DEVICE_INTR_SET_OFF;

    hreg.bits.host2device_tx_intr_set = 1; /* 1 for int */

    hreg_set_val(hreg, pst_pci_ctrl_h2d_int_base);
    return OAL_SUCC;
}

int32_t oal_pcie_d2h_int(oal_pcie_res *pst_pci_res)
{
    void *pst_pci_ctrl_h2d_int_base = NULL;
    hreg_host2device_intr_set hreg;

    if (oal_unlikely(pst_pci_res->link_state <= PCI_WLAN_LINK_DOWN)) {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "pcie is linkdown");
        return -OAL_ENODEV;
    }

    hreg.as_dword = 0x0;
    pst_pci_ctrl_h2d_int_base = pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST2DEVICE_INTR_SET_OFF;

    hreg.bits.device2host_rx_intr_set = 1; /* 1 for int */

    hreg_set_val(hreg, pst_pci_ctrl_h2d_int_base);
    return OAL_SUCC;
}

/* device intx/msi init */
int32_t oal_ete_intr_init(oal_pcie_res *pst_pci_res)
{
    hreg_host_intr_mask intr_mask;
#ifdef _PRE_COMMENT_CODE_
    hreg_pcie_ctrl_ete_ch_dr_empty_intr_mask host_dre_mask;
#endif

    /* enable interrupts */
    if (pst_pci_res->pst_pci_ctrl_base == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pst_pci_ctrl_base is null");
        return -OAL_ENODEV;
    }

    hreg_get_val(intr_mask, pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_MASK_OFF);

    intr_mask.bits.device2host_tx_intr_mask = 0;
    intr_mask.bits.device2host_rx_intr_mask = 0;
    intr_mask.bits.device2host_intr_mask = 0;

    /* WiFi中断需要在WiFi回调注册后Unmask */
    intr_mask.bits.mac_n1_intr_mask = 0;
    intr_mask.bits.mac_n2_intr_mask = 0;
    intr_mask.bits.phy_n1_intr_mask = 0;
    intr_mask.bits.phy_n2_intr_mask = 0;

    hreg_set_val(intr_mask, pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_MASK_OFF);
    hreg_get_val(intr_mask, pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_MASK_OFF);
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie intr mask=0x%x", intr_mask.as_dword);

    /* mask:1 for mask, 0 for unmask */
#ifdef _PRE_COMMENT_CODE_
    hreg_get_val(host_dre_mask, pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_MASK_OFF);
    host_dre_mask.bits.host_rx_ch_dr_empty_intr_mask = 0; /* unmask rx dre int */
    hreg_set_val(host_dre_mask,  pst_pci_res->pst_pci_ctrl_base +
                 PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_MASK_OFF);
#endif

    /* unmask all ete host int */
    oal_writel(0x0,
               pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_PCIE_CTL_ETE_INTR_MASK_OFF);

    return OAL_SUCC;
}

/* ete ip 重新初始化接口 */
int32_t oal_ete_chan_tx_ring_init(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    oal_ete_chan_h2d_res *pst_h2d_res = NULL;

    for (i = 0; i < ETE_TX_CHANN_MAX_NUMS; i++) {
        pst_h2d_res = &pst_pci_res->ete_info.h2d_res[i];
        if (pst_h2d_res->enabled == 0) {
            continue;
        }

        /* 设置TX SR地址 */
        if (oal_unlikely(pst_pci_res->pst_ete_base != NULL)) {
            pcie_ete_tx_sr_hardinit((uintptr_t)pst_pci_res->pst_ete_base, i, &pst_h2d_res->ring);
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_ete_base is null");
        }

        oal_print_hi11xx_log(HI11XX_LOG_DBG, "ete h2d chan%d hardinit succ, io_addr=0x%llx, items=%u",
                             i, pst_h2d_res->ring.io_addr, pst_h2d_res->ring.items);
    }

    return OAL_SUCC;
}

int32_t oal_ete_chan_rx_ring_init(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    oal_ete_chan_d2h_res *pst_d2h_res = NULL;

    for (i = 0; i < ETE_TX_CHANN_MAX_NUMS; i++) {
        pst_d2h_res = &pst_pci_res->ete_info.d2h_res[i];
        if (pst_d2h_res->enabled == 0) {
            continue;
        }

        /* 设置TX SR地址 */
        if (oal_unlikely(pst_pci_res->pst_ete_base != NULL)) {
            pcie_ete_rx_dr_hardinit((uintptr_t)pst_pci_res->pst_ete_base, i, &pst_d2h_res->ring);
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_ete_base is null");
        }

        oal_ete_print_ringbuf_info(&pst_d2h_res->ring);
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "ete d2h chan%d hardinit succ, io_addr=0x%llx, items=%u",
                             i, pst_d2h_res->ring.io_addr, pst_d2h_res->ring.items);
    }

    return OAL_SUCC;
}

int32_t oal_ete_chan_ring_init(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    ret = oal_ete_chan_tx_ring_init(pst_pci_res);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = oal_ete_chan_rx_ring_init(pst_pci_res);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return ret;
}

int32_t oal_ete_ip_init(oal_pcie_res *pst_pci_res)
{
    int32_t ret = oal_ete_intr_init(pst_pci_res);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = oal_ete_chan_ring_init(pst_pci_res);
    if (ret != OAL_SUCC) {
        return ret;
    }

    pcie_dual_enable((uintptr_t)pst_pci_res->pst_ete_base,
#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
                    g_dual_pci_support
#else
                    OAL_FALSE
#endif
                    );

    /* enable ete, 必须要在head ptr配置前使能! */
    ret = pcie_ete_clk_en((uintptr_t)pst_pci_res->pst_ete_base,
                          1); /* 0 means disable */
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "ete clk en enable failed!");
    }

    return OAL_SUCC;
}

void oal_ete_ip_exit(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    /* enable interrupts */
    if (pst_pci_res->pst_pci_ctrl_base == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pst_pci_ctrl_base is null");
        return;
    }

    if (oal_unlikely(pst_pci_res->link_state < PCI_WLAN_LINK_MEM_UP)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "ete int link invaild, link_state:%s",
                             oal_pcie_get_link_state_str((pst_pci_res->link_state)));
        return;
    }

    oal_writel(0xffffffff, pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_MASK_OFF);

    /* stop ete */
    ret = pcie_ete_clk_en((uintptr_t)pst_pci_res->pst_ete_base, 0); /* 0 means disable */
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "ete clk en disable failed!");
    }
}

int32_t oal_ete_rx_transfer_process(oal_pcie_res *pst_pci_res, uint32_t chan_idx)
{
    uint32_t trans_cnt, trans_cnt_t, netbuf_cnt;
    unsigned long flags;
    struct hcc_handler *hcc = NULL;
    oal_pcie_linux_res *pst_pci_lres = NULL;
    oal_pci_dev_stru *pst_pci_dev = NULL;
    uint32_t netbuf_trans_cnt = 0;
    uint32_t step = oal_ete_get_netbuf_step();
    oal_netbuf_stru *pst_netbuf = NULL;
    oal_ete_chan_d2h_res *pst_d2h_res = NULL;
    pst_d2h_res = &pst_pci_res->ete_info.d2h_res[chan_idx];

    trans_cnt = pcie_ete_update_ring_rx_donecnt(&pst_d2h_res->ring,
                                                (uintptr_t)pst_pci_res->pst_ete_base, chan_idx);
    if (oal_likely(trans_cnt != PCIE_ETE_INVALID_RD_VALUE)) {
        trans_cnt_t = trans_cnt;
        netbuf_cnt = oal_netbuf_list_len(&pst_d2h_res->rxq);
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "rxq trans_cnt=%u, rxq len=%u, done_cnt=%u",
                             trans_cnt, netbuf_cnt, pst_d2h_res->ring.done_cnt);

        if (trans_cnt) {
            pst_d2h_res->ring.done_cnt += trans_cnt;
        }

        /* release trans_cnt netbufs */
        forever_loop() {
            if (pst_d2h_res->ring.done_cnt < step) {
                break;
            }

            oal_spin_lock_irq_save(&pst_d2h_res->lock, &flags);
            pst_netbuf = oal_netbuf_delist_nolock(&pst_d2h_res->rxq);
            oal_spin_unlock_irq_restore(&pst_d2h_res->lock, &flags);
            if (oal_unlikely(pst_netbuf == NULL)) {
                declare_dft_trace_key_info("ete_rx_netbuf_underflow", OAL_DFT_TRACE_FAIL);
                oal_print_hi11xx_log(HI11XX_LOG_ERR,
                                     "ete_rx_netbuf_underflow[%u] trans_cnt=%u, trans_cnt_t=%u, netbuf_cnt=%u",
                                     chan_idx, trans_cnt + 1, trans_cnt_t, netbuf_cnt);
                return -OAL_EFAIL; /* DFR Here */
            }
            oal_print_hi11xx_log(HI11XX_LOG_DBG, "ete rx netbuf remain=%u", pst_d2h_res->ring.done_cnt);
            oal_pcie_rx_netbuf_submit(pst_pci_res, pst_netbuf);
            netbuf_trans_cnt++;
            pst_d2h_res->ring.done_cnt -= step;
        }
        if (netbuf_trans_cnt != 0) {
            pst_pci_dev = pcie_res_to_dev(pst_pci_res);
            pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pci_dev);
            hcc = hbus_to_hcc(pst_pci_lres->pst_bus);
            hcc_rx_assem_count(hcc, netbuf_trans_cnt);
        }
    }

    if (netbuf_trans_cnt != 0) {
        oal_ete_sched_hcc_thread(pst_pci_res);
        /* 通知线程，补充RX内存 */
#ifdef _PRE_COMMENT_CODE_
        oal_ete_shced_rx_thread(pst_pci_res);
#endif
    }

    return OAL_SUCC;
}

int32_t oal_ete_rx_ringbuf_supply(oal_pcie_res *pst_pci_res,
                                  uint32_t chan_idx,
                                  int32_t is_sync,
                                  int32_t is_up_headptr,
                                  uint32_t request_cnt,
                                  int32_t gflag,
                                  int32_t *ret)
{
    uint32_t i;
    int32_t max_len;
    uint32_t cnt = 0;
    uint64_t dst_addr;
    unsigned long flags;
    oal_netbuf_stru *pst_netbuf = NULL;
    pcie_cb_dma_res *pst_cb_res = NULL;
    dma_addr_t pci_dma_addr;
    d2h_dr_descr st_rx_dr_item;
    oal_ete_chan_d2h_res *pst_d2h_res = NULL;
    uint32_t step = oal_ete_get_netbuf_step();

    oal_pci_dev_stru *pst_pci_dev = pcie_res_to_dev(pst_pci_res);

    *ret = OAL_SUCC;
    max_len = pst_pci_res->ete_info.d2h_res[chan_idx].max_len;
    pst_d2h_res = &pst_pci_res->ete_info.d2h_res[chan_idx];
    if (pst_d2h_res->enabled == 0) { /* 0 is disable */
        return cnt;
    }
    if (oal_unlikely(max_len <= 0)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "invalid max_len=%d from ete rx chan %d", max_len, chan_idx);
        return -OAL_EINVAL;
    }
    if (is_sync == OAL_TRUE) {
        /* 更新rd指针,处理已经接受完成的报文 */
        oal_ete_rx_transfer_process(pst_pci_res, chan_idx);
    }

    for (i = 0; i < request_cnt; i++) {
        if (pcie_ete_ringbuf_freecount(&pst_d2h_res->ring) < step) {
            /* rx ring full */
            oal_print_hi11xx_log(HI11XX_LOG_DBG, "rx ring chan %d is full", chan_idx);
            break;
        }

        /* ringbuf 有空间 */
        /* 预申请netbuf都按照大包来申请 */
        pst_netbuf = oal_pcie_rx_netbuf_alloc(max_len, gflag);
        if (pst_netbuf == NULL) {
#ifdef _PRE_COMMENT_CODE_
            pst_pci_res->st_rx_res.stat.alloc_netbuf_failed++;
#endif
            *ret = -OAL_ENOMEM;
            break;
        }

        oal_netbuf_put(pst_netbuf, max_len);

        if (g_hipci_sync_flush_cache_enable == 0) {
            // sync dma未使能，map时会clean & inv操作
            oal_pcie_rx_netbuf_hdr_init(pst_pci_dev, pst_netbuf);
        }

        pci_dma_addr = dma_map_single(&pst_pci_dev->dev, oal_netbuf_data(pst_netbuf),
                                      oal_netbuf_len(pst_netbuf), PCI_DMA_FROMDEVICE);
        if (dma_mapping_error(&pst_pci_dev->dev, pci_dma_addr)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "rx dma map netbuf failed, len=%u",
                                 oal_netbuf_len(pst_netbuf));
            oal_netbuf_free(pst_netbuf);
            break;
        }

        /* DMA地址填到CB中, CB首地址8字节对齐可以直接强转 */
        pst_cb_res = (pcie_cb_dma_res *)oal_netbuf_cb(pst_netbuf);
        pst_cb_res->paddr.addr = pci_dma_addr;
        pst_cb_res->len = oal_netbuf_len(pst_netbuf);

        if (g_hipci_sync_flush_cache_enable != 0) {
            oal_pcie_rx_netbuf_hdr_init(pst_pci_dev, pst_netbuf);
            oal_pci_cache_flush(pst_pci_dev, pci_dma_addr, sizeof(uint32_t));
        }

        *ret = oal_pcie_host_slave_address_switch(pst_pci_res, (uint64_t)pci_dma_addr, &dst_addr, OAL_TRUE);
        if (*ret != OAL_SUCC) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "slave address switch failed ret=%d iova=%llx",
                                 *ret, (uint64_t)pci_dma_addr);
            dma_unmap_single(&pst_pci_dev->dev, pci_dma_addr, pst_cb_res->len, PCI_DMA_FROMDEVICE);
            oal_netbuf_free(pst_netbuf);
            break;
        }

        /* 头体分离 */
        if (g_ete_trans_buf_split != 0) {
            st_rx_dr_item.addr = dst_addr;
            if (oal_unlikely(pcie_ete_ringbuf_write(&pst_d2h_res->ring,
                (uint8_t *)&st_rx_dr_item) == 0)) {
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "pcie_ete_ringbuf_write failed, fatal error");
                dma_unmap_single(&pst_pci_dev->dev, pci_dma_addr, pst_cb_res->len, PCI_DMA_FROMDEVICE);
                oal_netbuf_free(pst_netbuf);
                declare_dft_trace_key_info("pcie_ete_ringbuf_write_head_failed", OAL_DFT_TRACE_FAIL);
                // never should happened
                break;
            }
            dst_addr += HCC_HDR_TOTAL_LEN;
        }

        /* dma_addr 转换成pcie slave地址 */
        st_rx_dr_item.addr = dst_addr;
        if (oal_unlikely(pcie_ete_ringbuf_write(&pst_d2h_res->ring,
            (uint8_t *)&st_rx_dr_item) == 0)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "pcie_ete_ringbuf_write failed, fatal error");
            dma_unmap_single(&pst_pci_dev->dev, pci_dma_addr, pst_cb_res->len, PCI_DMA_FROMDEVICE);
            oal_netbuf_free(pst_netbuf);
            declare_dft_trace_key_info("pcie_ete_ringbuf_write_failed", OAL_DFT_TRACE_FAIL);
            // never should happened
            break;
        }

        /* 入队 */
        oal_spin_lock_irq_save(&pst_d2h_res->lock, &flags);
        oal_netbuf_list_tail_nolock(&pst_d2h_res->rxq, pst_netbuf);
        oal_spin_unlock_irq_restore(&pst_d2h_res->lock, &flags);

        oal_print_hi11xx_log(HI11XX_LOG_DBG, "d2h ring write [netbuf:0x%p, data:[va:0x%lx,pa:0x%llx,deva:0x%llx]",
                             pst_netbuf, (uintptr_t)oal_netbuf_data(pst_netbuf),
                             (uint64_t)pci_dma_addr, st_rx_dr_item.addr);

        cnt++;
    }

    /* 这里需要考虑HOST/DEVICE的初始化顺序 */
    if (cnt && (is_up_headptr == OAL_TRUE)) {
        pcie_ete_ring_update_rx_dr_hptr(&pst_d2h_res->ring, (uintptr_t)pst_pci_res->pst_ete_base,
                                        chan_idx);
    }

    return cnt;
}

/* 预先分配rx的接收buf */
int32_t oal_ete_rx_ringbuf_build(oal_pcie_res *pst_pci_res)
{
    /* 走到这里要确保DEVICE ZI区已经初始化完成， 中断已经注册和使能 */
    int32_t ret;
    int32_t i;
    int32_t supply_num;

    for (i = 0; i < ETE_RX_CHANN_MAX_NUMS; i++) {
        if (pst_pci_res->ete_info.d2h_res[i].enabled != OAL_TRUE) {
            continue;
        }
        supply_num = oal_ete_rx_ringbuf_supply(pst_pci_res, i, OAL_TRUE, OAL_TRUE,
                                               PCIE_RX_RINGBUF_SUPPLY_ALL, GFP_KERNEL, &ret);
        if (supply_num == 0) {
            oal_print_hi11xx_log(HI11XX_LOG_WARN, "oal_pcie_rx_ringbuf_build chan %d can't get any netbufs!", i);
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "oal_pcie_rx_ringbuf_build chan %d got %u netbufs!", i, supply_num);
        }
    }

    return OAL_SUCC;
}

/* 芯片上电初始化 */
int32_t oal_ete_transfer_res_init(oal_pcie_res *pst_pci_res)
{
    int32_t ret;

    /* 下载完PATCH才需要执行下面的操作, 片验证阶段通过SSI下载代码 */
    ret = oal_pcie_share_mem_res_map(pst_pci_res);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = oal_pcie_ringbuf_res_map(pst_pci_res);
    if (ret != OAL_SUCC) {
        return ret;
    }

    oal_ete_rx_res_clean(pst_pci_res);
    oal_ete_tx_res_clean(pst_pci_res, OAL_TRUE);

    mutex_lock(&pst_pci_res->st_rx_mem_lock);
    oal_pcie_change_link_state(pst_pci_res, PCI_WLAN_LINK_RES_UP);
    mutex_unlock(&pst_pci_res->st_rx_mem_lock);

    ret = oal_ete_task_init(pst_pci_res);
    if (ret != OAL_SUCC) {
        oal_ete_transfer_res_exit(pst_pci_res);
    }

    return ret;
}

void oal_ete_transfer_res_exit(oal_pcie_res *pst_pci_res)
{
    /* stop task */
    oal_ete_task_exit(pst_pci_res);

    oal_pcie_change_link_state(pst_pci_res, PCI_WLAN_LINK_DOWN);

    oal_ete_rx_res_clean(pst_pci_res);
    oal_ete_tx_res_clean(pst_pci_res, OAL_TRUE);

    oal_pcie_ringbuf_res_unmap(pst_pci_res);

    /* clean ring */
    oal_pcie_share_mem_res_unmap(pst_pci_res);
}

/* 主芯片suspend时断开pcie 需要释放和ETE相关的资源，唤醒后重新初始化IP */
void oal_ete_transfer_deepsleep_clean(oal_pcie_res *pst_pci_res)
{
    /* stop task */
    oal_ete_task_exit(pst_pci_res);
    oal_ete_rx_res_clean(pst_pci_res);
    oal_ete_tx_res_clean(pst_pci_res, OAL_FALSE);
}

int32_t oal_ete_transfer_deepwkup_restore(oal_pcie_res *pst_pci_res)
{
    return oal_ete_task_init(pst_pci_res);
}

int32_t pcie_wlan_func_register(pcie_wlan_callback_group_stru *pst_func)
{
    if (pst_func == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "ptr NULL");
        return -OAL_ENODEV;
    }

    g_pcie_ete_intx_callback[MAC_5G_INTR_MASK] = pst_func->pcie_mac_5g_isr_callback;
    g_pcie_ete_intx_callback[MAC_2G_INTR_MASK] = pst_func->pcie_mac_2g_isr_callback;
    g_pcie_ete_intx_callback[PHY_5G_INTR_MASK] = pst_func->pcie_phy_5g_isr_callback;
    g_pcie_ete_intx_callback[PHY_2G_INTR_MASK] = pst_func->pcie_phy_2g_isr_callback;

    return OAL_SUCC;
}
oal_module_symbol(pcie_wlan_func_register);

OAL_STATIC void oal_ete_int_mask_init(oal_pcie_res *pst_pci_res)
{
    hreg_host_intr_mask host_intr_mask;
    hreg_pcie_ctl_ete_intr_mask ete_intr_mask;
    hreg_pcie_ctrl_ete_ch_dr_empty_intr_mask ete_ch_dr_empty_intr_mask;

    host_intr_mask.as_dword = 0x0;
    ete_intr_mask.as_dword = 0x0;

    /* host int mask */
    host_intr_mask.bits.device2host_tx_intr_mask = 1;
    host_intr_mask.bits.device2host_rx_intr_mask = 1;
    host_intr_mask.bits.device2host_intr_mask = 1;
    host_intr_mask.bits.mac_n2_intr_mask = 1;
    host_intr_mask.bits.mac_n1_intr_mask = 1;
    host_intr_mask.bits.phy_n2_intr_mask = 1;
    host_intr_mask.bits.phy_n1_intr_mask = 1;

    pst_pci_res->ete_info.host_intr_mask = host_intr_mask.as_dword;
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "host_intr_mask:0x%x", pst_pci_res->ete_info.host_intr_mask);

    ete_intr_mask.bits.host_tx_err_intr_mask = ((1 << ETE_TX_CHANN_MAX_NUMS) - 1);
    ete_intr_mask.bits.host_tx_done_intr_mask = ((1 << ETE_TX_CHANN_MAX_NUMS) - 1);
    ete_intr_mask.bits.host_rx_err_intr_mask = ((1 << ETE_RX_CHANN_MAX_NUMS) - 1);
    ete_intr_mask.bits.host_rx_done_intr_mask = ((1 << ETE_RX_CHANN_MAX_NUMS) - 1);
    pst_pci_res->ete_info.pcie_ctl_ete_intr_mask = ete_intr_mask.as_dword;
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_ctl_ete_intr_mask:0x%x", pst_pci_res->ete_info.pcie_ctl_ete_intr_mask);

    ete_ch_dr_empty_intr_mask.bits.host_tx_ch_dr_empty_intr_mask = ((1 << ETE_TX_CHANN_MAX_NUMS) - 1);
    ete_ch_dr_empty_intr_mask.bits.host_rx_ch_dr_empty_intr_mask = ((1 << ETE_RX_CHANN_MAX_NUMS) - 1);
    pst_pci_res->ete_info.ete_ch_dr_empty_intr_mask = ete_ch_dr_empty_intr_mask.as_dword;
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_ctl_ete_intr_mask:0x%x",
                         pst_pci_res->ete_info.ete_ch_dr_empty_intr_mask);
}

void oal_ete_chan_tx_exit(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    int32_t ret;
    uint64_t iova;
    oal_ete_chan_h2d_res *pst_h2d_res = NULL;
    oal_pci_dev_stru *pst_pci_dev = pcie_res_to_dev(pst_pci_res);

    for (i = 0; i < ETE_TX_CHANN_MAX_NUMS; i++) {
        pst_h2d_res = &pst_pci_res->ete_info.h2d_res[i];
        if (pst_h2d_res->ring.cpu_addr != 0) {
            ret = oal_pcie_host_slave_address_switch(pst_pci_res,
                                                     (uint64_t)pst_h2d_res->ring.io_addr, &iova, OAL_FALSE);
            if (oal_unlikely(ret != OAL_SUCC)) {
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "slave address switch failed ret=%d devca=%llx", ret,
                                     (uint64_t)pst_h2d_res->ring.io_addr);
            } else {
                dma_free_coherent(&pst_pci_dev->dev, pst_h2d_res->ring.size,
                                  (void *)(uintptr_t)pst_h2d_res->ring.cpu_addr,
                                  (dma_addr_t)iova);
                pst_h2d_res->ring.cpu_addr = 0;
                pst_h2d_res->ring.io_addr = 0;
            }
        }
    }
}

void oal_ete_chan_rx_exit(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    int32_t ret;
    uint64_t iova;
    oal_ete_chan_d2h_res *pst_d2h_res = NULL;
    oal_pci_dev_stru *pst_pci_dev = pcie_res_to_dev(pst_pci_res);

    for (i = 0; i < ETE_RX_CHANN_MAX_NUMS; i++) {
        pst_d2h_res = &pst_pci_res->ete_info.d2h_res[i];
        if (pst_d2h_res->ring.cpu_addr != 0) {
            ret = oal_pcie_host_slave_address_switch(pst_pci_res,
                                                     (uint64_t)pst_d2h_res->ring.io_addr, &iova, OAL_FALSE);
            if (oal_unlikely(ret != OAL_SUCC)) {
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "slave address switch failed ret=%d devca=%llx", ret,
                                     (uint64_t)pst_d2h_res->ring.io_addr);
            } else {
                dma_free_coherent(&pst_pci_dev->dev, pst_d2h_res->ring.size,
                                  (void *)(uintptr_t)pst_d2h_res->ring.cpu_addr,
                                  (dma_addr_t)iova);
                pst_d2h_res->ring.cpu_addr = 0;
                pst_d2h_res->ring.io_addr = 0;
            }
        }
    }
}

void oal_ete_task_exit(oal_pcie_res *pst_pcie_res)
{
    int i;
    oal_ete_chan_d2h_res *pst_d2h_res = NULL;
    for (i = 0; i < ETE_RX_CHANN_MAX_NUMS; i++) {
        pst_d2h_res = &pst_pcie_res->ete_info.d2h_res[i];
        if (pst_d2h_res->enabled == 0) {
            continue;
        }
        if (pst_d2h_res->pst_rx_chan_task != NULL) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "stop rx chan%d's task", i);
            oal_thread_stop(pst_d2h_res->pst_rx_chan_task, NULL);
            pst_d2h_res->pst_rx_chan_task = NULL;
        }
        if (pst_d2h_res->task_name != NULL) {
            oal_free(pst_d2h_res->task_name);
            pst_d2h_res->task_name = NULL;
        }
        mutex_destroy(&pst_d2h_res->rx_mem_lock);
    }
}

void oal_ete_chan_exit(oal_pcie_res *pst_pci_res)
{
    oal_ete_chan_tx_exit(pst_pci_res);
    oal_ete_chan_rx_exit(pst_pci_res);
}

OAL_STATIC int32_t oal_ete_rx_hi_thread(void *data)
{
    int32_t ret;
    int32_t supply_num;
    oal_ete_chan_d2h_res *pst_d2h_res = (oal_ete_chan_d2h_res *)data;
    oal_pcie_res *pst_pcie_res = (oal_pcie_res *)pst_d2h_res->task_data;

    if (oal_warn_on(pst_d2h_res == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "error: pst_d2h_res is null");
        return -EFAIL;
    }

    if (oal_warn_on(pst_pcie_res == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "error: pst_pcie_res is null");
        return -EFAIL;
    }

    allow_signal(SIGTERM);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "%s thread start", oal_get_current_task_name());

    forever_loop() {
        if (oal_unlikely(kthread_should_stop())) {
            break;
        }

        ret = oal_wait_event_interruptible_m(pst_d2h_res->rx_wq,
                                             oal_ete_rx_thread_condtion(&pst_d2h_res->rx_cond));
        if (oal_unlikely(ret == -ERESTARTSYS)) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "task %s was interrupted by a signal\n",
                                 oal_get_current_task_name());
            break;
        }

        if (oal_unlikely(pst_pcie_res->link_state < PCI_WLAN_LINK_WORK_UP || !pst_pcie_res->regions.inited)) {
            oal_print_hi11xx_log(HI11XX_LOG_WARN,
                                 "hi thread link invaild, stop supply mem, link_state:%s, region:%d",
                                 oal_pcie_get_link_state_str(pst_pcie_res->link_state),
                                 pst_pcie_res->regions.inited);
        } else {
            ret = OAL_SUCC;
            supply_num = oal_ete_rx_ringbuf_supply(pst_pcie_res, pst_d2h_res->chan_idx, OAL_TRUE, OAL_TRUE,
                                                   PCIE_RX_RINGBUF_SUPPLY_ALL,
                                                   GFP_ATOMIC | __GFP_NOWARN, &ret);
            if (ret != OAL_SUCC) {
                /* 补充内存失败，成功则忽略，有可能当前不需要补充内存也视为成功 */
                oal_ete_shced_rx_thread(pst_d2h_res);
                declare_dft_trace_key_info("ete_rx_ringbuf_supply_retry", OAL_DFT_TRACE_OTHER);
                oal_msleep(10); /* 10 supply failed, may be memleak, wait & retry */
            }
        }
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "%s thread stop", oal_get_current_task_name());

    return 0;
}

int32_t oal_ete_task_init(oal_pcie_res *pst_pcie_res)
{
    int i;
    char *name = NULL;
    oal_ete_chan_d2h_res *pst_d2h_res = NULL;
    for (i = 0; i < ETE_RX_CHANN_MAX_NUMS; i++) {
        pst_d2h_res = &pst_pcie_res->ete_info.d2h_res[i];
        if (pst_d2h_res->enabled == 0) {
            continue;
        }

        if (pst_d2h_res->pst_rx_chan_task != NULL) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "ete rx task already exist, chan %d", i);
            continue;
        }

        oal_wait_queue_init_head(&pst_d2h_res->rx_wq);
        oal_atomic_set(&pst_d2h_res->rx_cond, 0);
        mutex_init(&pst_d2h_res->rx_mem_lock);

        name = oal_memalloc(TASK_COMM_LEN);
        if (name == NULL) {
            name = "pci_ete_rx_unkown";
        } else {
            pst_d2h_res->task_name = name;
            snprintf_s(name, TASK_COMM_LEN, TASK_COMM_LEN - 1, "pci_ete_rx%d", i);
        }

        pst_d2h_res->task_data = (void *)pst_pcie_res;
        pst_d2h_res->pst_rx_chan_task = oal_thread_create(oal_ete_rx_hi_thread,
                                                          (void *)pst_d2h_res,
                                                          NULL,
                                                          name,
                                                          SCHED_RR,
                                                          OAL_BUS_TEST_INIT_PRIORITY,
                                                          -1);
        if (pst_d2h_res->pst_rx_chan_task == NULL) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "create thread %s failed", name);
            oal_ete_task_exit(pst_pcie_res);
            return -OAL_ENODEV;
        }
    }
    return OAL_SUCC;
}

int32_t oal_ete_chan_tx_init(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    int32_t ret;
    size_t buff_size;
    dma_addr_t iova = 0;
    uint64_t dst_addr;
    void *cpu_addr = NULL;
    oal_ete_chan_h2d_res *pst_h2d_res = NULL;
    oal_pci_dev_stru *pst_pci_dev = pcie_res_to_dev(pst_pci_res);

    for (i = 0; i < ETE_TX_CHANN_MAX_NUMS; i++) {
        pst_h2d_res = &pst_pci_res->ete_info.h2d_res[i];
        pst_h2d_res->chan_idx = i;
        if (pst_h2d_res->enabled == 0) {
            continue;
        }
        if (pst_h2d_res->ring_max_num == 0) {
            pst_h2d_res->enabled = 0;
            continue;
        }

        oal_spin_lock_init(&pst_h2d_res->lock);
        oal_netbuf_list_head_init(&pst_h2d_res->txq);

        oal_atomic_set(&pst_h2d_res->tx_sync_cond, 0);

        buff_size = sizeof(h2d_sr_descr) * pst_h2d_res->ring_max_num;

        /* (h2d_sr_descr*) */
        cpu_addr = dma_alloc_coherent(&pst_pci_dev->dev, buff_size,
                                      &iova,
                                      GFP_KERNEL);
        if (cpu_addr == NULL) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "ete h2d chan%d init failed, ring_max_num=%d, buff_size=%u",
                                 i, pst_h2d_res->ring_max_num, (uint32_t)buff_size);
            return -OAL_ENOMEM;
        }

        ret = oal_pcie_host_slave_address_switch(pst_pci_res, (uint64_t)iova, &dst_addr, OAL_TRUE);
        if (ret != OAL_SUCC) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "slave address switch failed ret=%d iova=%llx", ret, (uint64_t)iova);
            return ret;
        }
        ret = pcie_ete_ringbuf_init(&pst_h2d_res->ring, (uint64_t)(uintptr_t)cpu_addr,
                                    (uint64_t)dst_addr, 0, buff_size, sizeof(h2d_sr_descr));
        if (ret != OAL_SUCC) {
            return ret;
        }

        pst_h2d_res->ring.debug = 0; /* 0 no debug info */

        oal_print_hi11xx_log(HI11XX_LOG_DBG, "ete h2d chan%d iova=%llx  devva=%llx, io_addr=%llx",
                             i, (uint64_t)iova, dst_addr, pst_h2d_res->ring.io_addr);

        oal_print_hi11xx_log(HI11XX_LOG_INFO, "ete h2d chan%d init succ, ring_max_num=%d, buff_size=%u",
                             i, pst_h2d_res->ring_max_num, (uint32_t)buff_size);
        oal_ete_print_ringbuf_info(&pst_h2d_res->ring);
    }

    return OAL_SUCC;
}

int32_t oal_ete_chan_rx_init(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    int32_t ret;
    size_t buff_size;
    dma_addr_t iova = 0;
    uint64_t dst_addr;
    void *cpu_addr = NULL;
    oal_ete_chan_d2h_res *pst_d2h_res = NULL;
    oal_pci_dev_stru *pst_pci_dev = pcie_res_to_dev(pst_pci_res);

    for (i = 0; i < ETE_RX_CHANN_MAX_NUMS; i++) {
        pst_d2h_res = &pst_pci_res->ete_info.d2h_res[i];
        pst_d2h_res->chan_idx = i;
        if (pst_d2h_res->enabled == 0) {
            continue;
        }
        if (pst_d2h_res->ring_max_num == 0) {
            pst_d2h_res->enabled = 0;
            continue;
        }

        oal_spin_lock_init(&pst_d2h_res->lock);
        oal_netbuf_list_head_init(&pst_d2h_res->rxq);

        buff_size = sizeof(d2h_dr_descr) * pst_d2h_res->ring_max_num;

        /* (d2h_sr_descr*) */
        cpu_addr = dma_alloc_coherent(&pst_pci_dev->dev, buff_size,
                                      &iova,
                                      GFP_KERNEL);
        if (cpu_addr == NULL) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "ete d2h chan%d init failed, ring_max_num=%d, buff_size=%u",
                                 i, pst_d2h_res->ring_max_num, (uint32_t)buff_size);
            return -OAL_ENOMEM;
        }
        ret = oal_pcie_host_slave_address_switch(pst_pci_res, (uint64_t)iova, &dst_addr, OAL_TRUE);
        if (ret != OAL_SUCC) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "slave address switch failed ret=%d iova=%llx", ret, (uint64_t)iova);
            return ret;
        }
        ret = pcie_ete_ringbuf_init(&pst_d2h_res->ring, (uint64_t)(uintptr_t)cpu_addr, (uint64_t)dst_addr,
                                    0, buff_size, sizeof(d2h_dr_descr));
        if (ret != OAL_SUCC) {
            return ret;
        }

        oal_print_hi11xx_log(HI11XX_LOG_DBG, "ete d2h chan%d iova=%llx  devva=%llx",
                             i, (uint64_t)iova, dst_addr);

        oal_print_hi11xx_log(HI11XX_LOG_DBG, "ete d2h chan%d init succ, ring_max_num=%d, buff_size=%u",
                             i, pst_d2h_res->ring_max_num, (uint32_t)buff_size);

        oal_ete_print_ringbuf_info(&pst_d2h_res->ring);
    }

    return OAL_SUCC;
}

#define ETE_ENABLE
int32_t oal_ete_chan_init(oal_pcie_res *pst_pci_res)
{
#ifdef ETE_ENABLE
    int32_t ret;
    errno_t err1, err2;

    err1 = memcpy_s((void *)pst_pci_res->ete_info.h2d_res,
                    sizeof(pst_pci_res->ete_info.h2d_res),
                    (void *)g_pcie_h2d_res_cfg, sizeof(pst_pci_res->ete_info.h2d_res));
    err2 = memcpy_s((void *)pst_pci_res->ete_info.d2h_res,
                    sizeof(pst_pci_res->ete_info.d2h_res),
                    (void *)g_pcie_d2h_res_config, sizeof(pst_pci_res->ete_info.d2h_res));
    if (err1 != EOK || err2 != EOK) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "err1=%d, err2=%d", err1, err2);
    }

    (void)memset_s((void *)g_pcie_ete_intx_callback, sizeof(g_pcie_ete_intx_callback),
                   0, sizeof(g_pcie_ete_intx_callback));
    oal_ete_int_mask_init(pst_pci_res);

    ret = oal_ete_chan_tx_init(pst_pci_res);
    if (ret != OAL_SUCC) {
        goto fail_tx;
    }

    ret = oal_ete_chan_rx_init(pst_pci_res);
    if (ret != OAL_SUCC) {
        goto fail_rx;
    }
    return OAL_SUCC;
fail_rx:
    oal_ete_chan_tx_exit(pst_pci_res);
fail_tx:
    return ret;
#else
    (void)memset_s((void *)g_pcie_ete_intx_callback, sizeof(g_pcie_ete_intx_callback),
                   0, sizeof(g_pcie_ete_intx_callback));
    oal_ete_int_mask_init(pst_pci_res);
    return OAL_SUCC;
#endif
}

void pcie_ete_print_trans_tx_ringbuf(oal_ete_chan_h2d_res *pst_h2d_res)
{
    /* dump ring memory */
    oal_ete_print_ringbuf_info(&pst_h2d_res->ring);
    if (pst_h2d_res->ring.item_len == sizeof(h2d_sr_descr)) {
        int32_t i;
        h2d_sr_descr *pst_tx_sr = (h2d_sr_descr *)(uintptr_t)pst_h2d_res->ring.cpu_addr;
        for (i = 0; i < pst_h2d_res->ring.items; i++, pst_tx_sr++) {
            oal_io_print("item[%d][0x%lx] iova:0x%llx ctrl:0x%x nbytes:%u" NEWLINE, i,
                         ((uintptr_t)pst_tx_sr - (uintptr_t)pst_h2d_res->ring.cpu_addr)
                         + (uintptr_t) pst_h2d_res->ring.io_addr,
                         pst_tx_sr->addr, pst_tx_sr->ctrl.dword, pst_tx_sr->ctrl.bits.nbytes);
        }
    } else {
        oal_io_print("unexpect ringbuf, item_len=%u" NEWLINE, pst_h2d_res->ring.item_len);
    }

    oal_io_print("" NEWLINE);
}

void pcie_ete_print_trans_rx_ringbuf(oal_ete_chan_d2h_res *pst_d2h_res)
{
    oal_ete_print_ringbuf_info(&pst_d2h_res->ring);
    /* dump ring memory */
    if (pst_d2h_res->ring.item_len == sizeof(d2h_sr_descr)) {
        int32_t i;
        d2h_dr_descr *pst_rx_dr = (d2h_dr_descr *)(uintptr_t)pst_d2h_res->ring.cpu_addr;
        for (i = 0; i < pst_d2h_res->ring.items; i++, pst_rx_dr++) {
            oal_io_print("item[%d][0x%lx] iova:0x%llx" NEWLINE,
                         i, (uintptr_t)pst_rx_dr, pst_rx_dr->addr);
        }
    } else {
        oal_io_print("unexpect ringbuf, item_len=%u" NEWLINE, pst_d2h_res->ring.item_len);
    }

    oal_io_print("" NEWLINE);
}

void oal_ete_print_trans_tx_regs(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    uintptr_t base = (uintptr_t)pst_pci_res->pst_ete_base;
    oal_ete_chan_h2d_res *pst_h2d_res = NULL;
    for (i = 0; i < ETE_TX_CHANN_MAX_NUMS; i++) {
        pst_h2d_res = &pst_pci_res->ete_info.h2d_res[i];
        if (pst_h2d_res->enabled == 0) {
            continue;
        }
        pcie_ete_print_trans_tx_chan_regs(base, i);
        pcie_ete_print_trans_tx_ringbuf(pst_h2d_res);
    }
}

void oal_ete_print_trans_rx_regs(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    uintptr_t base = (uintptr_t)pst_pci_res->pst_ete_base;
    oal_ete_chan_d2h_res *pst_d2h_res = NULL;
    for (i = 0; i < ETE_RX_CHANN_MAX_NUMS; i++) {
        pst_d2h_res = &pst_pci_res->ete_info.d2h_res[i];
        if (pst_d2h_res->enabled == 0) {
            continue;
        }

        pcie_ete_print_trans_rx_chan_regs(base, i);
        pcie_ete_print_trans_rx_ringbuf(pst_d2h_res);
    }
}

int32_t oal_ete_print_trans_regs(void)
{
    oal_pcie_res *pst_pci_res = oal_get_default_pcie_handler();
    if (oal_unlikely(pst_pci_res->link_state < PCI_WLAN_LINK_MEM_UP)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "link failed, link_state:%s",
                             oal_pcie_get_link_state_str(pst_pci_res->link_state));
        return -OAL_EBUSY;
    }

    oal_ete_print_trans_comm_regs((uintptr_t)pst_pci_res->pst_ete_base);
    oal_ete_print_trans_tx_regs(pst_pci_res);
    oal_ete_print_trans_rx_regs(pst_pci_res);
    return OAL_SUCC;
}

void oal_ete_print_transfer_info(oal_pcie_res *pst_pci_res, uint64_t print_flag)
{
    if (pst_pci_res == NULL) {
        return;
    }
    /* dump pcie hardware info */
    if (oal_unlikely(pst_pci_res->link_state >= PCI_WLAN_LINK_RES_UP)) {
        if (board_get_host_wakeup_dev_stat() == 1) {
            /* gpio is high axi is alive */
            if (print_flag & (HCC_PRINT_TRANS_FLAG_DEVICE_REGS | HCC_PRINT_TRANS_FLAG_DEVICE_STAT)) {
                (void)oal_ete_print_trans_regs();
                oal_pcie_send_message_to_dev(pst_pci_res, 19); /* 19 is ete print message */
            }
        }
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie is %s", g_pcie_link_state_str[pst_pci_res->link_state]);
    }
}

void oal_ete_reset_transfer_info(oal_pcie_res *pst_pci_res)
{
    if (pst_pci_res == NULL) {
        return;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "reset transfer info");
}

int32_t oal_ete_host_init(oal_pcie_res *pst_pci_res, oal_pcie_msi_stru *msi)
{
    if (pst_pci_res->chip_info.ete_support != OAL_TRUE) {
        return OAL_SUCC;
    }
    /* init ringbuf and alloc descr mem */
    return oal_ete_chan_init(pst_pci_res);
}

void oal_ete_host_exit(oal_pcie_res *pst_pci_res)
{
    if (pst_pci_res->chip_info.ete_support != OAL_TRUE) {
        return;
    }
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "oal_ete_host_exit");
    oal_ete_chan_exit(pst_pci_res);
}

#endif
