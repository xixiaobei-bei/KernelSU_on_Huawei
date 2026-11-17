

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define HI11XX_LOG_MODULE_NAME "[PCIE_H]"
#define HISI_LOG_TAG           "[PCIE]"
#include "pcie_host.h"
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
#include "oal_thread.h"
#include "oam_ext_if.h"
#include "pcie_linux.h"
#include "pcie_reg.h"
#include "oal_hcc_host_if.h"
#include "oal_kernel_file.h"
#include "plat_firmware.h"
#include "plat_pm_wlan.h"
#include "board.h"
#include "securec.h"
#include "plat_pm.h"

#ifdef _PRE_WLAN_PKT_TIME_STAT
#include <hwnet/ipv4/wifi_delayst.h>
#endif

OAL_STATIC int32_t oal_pcie_d2h_ringbuf_rd_update(oal_pcie_res *pst_pci_res);
OAL_STATIC int32_t oal_pcie_d2h_ringbuf_wr_update(oal_pcie_res *pst_pci_res);
OAL_STATIC int32_t oal_pcie_d2h_ringbuf_write(oal_pcie_res *pst_pci_res,
                                              pci_addr_map *pst_map,
                                              pcie_write_ringbuf_item *pst_item);


/* edma read 对应device->host, ringbuf_write 指更新写指针 */
OAL_STATIC int32_t oal_pcie_d2h_ringbuf_write(oal_pcie_res *pst_pci_res,
                                              pci_addr_map *pst_map,
                                              pcie_write_ringbuf_item *pst_item)
{
    /* 不判断写指针，此函数只执行写操作 */
    oal_pci_dev_stru *pst_pci_dev;
    uint32_t real_wr;

    pcie_ringbuf *pst_ringbuf = &pst_pci_res->st_ringbuf.st_d2h_buf;
    pst_pci_dev = pcie_res_to_dev(pst_pci_res);

    /* Debug */
    if (oal_unlikely(pst_ringbuf->item_len != sizeof(pcie_write_ringbuf_item))) {
        pci_print_log(PCI_LOG_ERR, "[%s]invalid item_len [%u!=%lu]\n",
                      __FUNCTION__, pst_ringbuf->item_len, (unsigned long)sizeof(pcie_write_ringbuf_item));
        return 0;
    }

    if (oal_warn_on(pst_ringbuf->wr - pst_ringbuf->rd >= pst_ringbuf->size)) {
        /* never touch here */
        pci_print_log(PCI_LOG_ERR, "ringbuf full [wr:%u] [rd:%u] [size:%u]\n",
                      pst_ringbuf->wr, pst_ringbuf->rd, pst_ringbuf->size);
        return 0;
    }

    real_wr = pst_ringbuf->wr & (pst_ringbuf->size - 1);
    oal_pcie_io_trans(pst_map->va + real_wr, (uintptr_t)pst_item, pst_ringbuf->item_len);
    pst_ringbuf->wr += pst_ringbuf->item_len;

    return 1;
}

OAL_STATIC int32_t oal_pcie_d2h_ringbuf_wr_update(oal_pcie_res *pst_pci_res)
{
    /* d2h方向，同步host的ringbuf管理结构体的写指针到DEVICE侧, 需要刷cache */
    pci_addr_map st_map;

    oal_pcie_mips_start(PCIE_MIPS_RX_RINGBUF_WR_UPDATE);
    st_map.va = pst_pci_res->st_rx_res.ringbuf_ctrl_dma_addr.va + OAL_OFFSET_OF(pcie_ringbuf, wr);
    st_map.pa = pst_pci_res->st_rx_res.ringbuf_ctrl_dma_addr.pa + OAL_OFFSET_OF(pcie_ringbuf, wr);
    pci_print_log(PCI_LOG_DBG, "d2h ringbuf wr upate wr:%u", pst_pci_res->st_ringbuf.st_d2h_buf.wr);
    oal_pcie_write_mem32(st_map.va, pst_pci_res->st_ringbuf.st_d2h_buf.wr);
    oal_pcie_mips_end(PCIE_MIPS_RX_RINGBUF_WR_UPDATE);

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_d2h_ringbuf_rd_update(oal_pcie_res *pst_pci_res)
{
    /* d2h方向，同步device的读指针到HOST ringbuf管理结构体 */
    /* 需要刷cache */
    uint32_t rd;
    pci_addr_map st_map;

    oal_pcie_mips_start(PCIE_MIPS_RX_RINGBUF_RD_UPDATE);
    st_map.va = pst_pci_res->st_rx_res.ringbuf_ctrl_dma_addr.va + OAL_OFFSET_OF(pcie_ringbuf, rd);
    st_map.pa = pst_pci_res->st_rx_res.ringbuf_ctrl_dma_addr.pa + OAL_OFFSET_OF(pcie_ringbuf, rd);

    rd = oal_pcie_read_mem32(st_map.va);
    if (oal_unlikely(rd == 0xFFFFFFFF)) {
        if (oal_pcie_check_link_state(pst_pci_res) == OAL_FALSE) {
            pci_print_log(PCI_LOG_ERR, "d2h ringbuf rd update: link down[va:0x%lx, pa:0x%lx]", st_map.va, st_map.pa);
            return -OAL_ENODEV;
        }
    }
    pci_print_log(PCI_LOG_DBG, "d2h ringbuf rd update:[0x%lx:rd:%u]", st_map.va, rd);
    if (oal_unlikely(rd < pst_pci_res->st_ringbuf.st_d2h_buf.rd)) {
        /* 判断rd 翻转 */
        pci_print_log(PCI_LOG_INFO, "d2h new rd %u over old rd %u, wr:%u",
                      rd, pst_pci_res->st_ringbuf.st_d2h_buf.rd,
                      pst_pci_res->st_ringbuf.st_d2h_buf.wr);
        declare_dft_trace_key_info("d2h_ringbuf_overrun", OAL_DFT_TRACE_SUCC);
    }
    pst_pci_res->st_ringbuf.st_d2h_buf.rd = rd;
    oal_pcie_mips_end(PCIE_MIPS_RX_RINGBUF_RD_UPDATE);

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_h2d_ringbuf_write(oal_pcie_res *pst_pci_res,
                                              pci_addr_map *pst_map,
                                              pcie_h2d_ringbuf_qtype qtype,
                                              pcie_read_ringbuf_item *pst_item)
{
    /* 不判断写指针，此函数只执行写操作 */
    oal_pci_dev_stru *pst_pci_dev;
    uint32_t real_wr;

    pcie_ringbuf *pst_ringbuf = &pst_pci_res->st_ringbuf.st_h2d_buf[qtype];

    pst_pci_dev = pcie_res_to_dev(pst_pci_res);

    /* Debug */
    if (oal_unlikely(pst_pci_res->link_state < PCI_WLAN_LINK_WORK_UP)) {
        pci_print_log(PCI_LOG_WARN, "oal_pcie_h2d_ringbuf_write invaild, link_state:%s",
                      oal_pcie_get_link_state_str(pst_pci_res->link_state));
        return -OAL_ENODEV;
    }

    /* Debug */
    if (oal_unlikely(pst_ringbuf->item_len != sizeof(pcie_read_ringbuf_item))) {
        pci_print_log(PCI_LOG_ERR, "[%s]invalid item_len [%u!=%lu]\n",
                      __FUNCTION__, pst_ringbuf->item_len, (unsigned long)sizeof(pcie_read_ringbuf_item));
        return 0;
    }

    if (oal_warn_on(pst_ringbuf->wr - pst_ringbuf->rd >= pst_ringbuf->size)) {
        /* never touch here */
        pci_print_log(PCI_LOG_ERR, "ringbuf full [wr:%u] [rd:%u] [size:%u]\n",
                      pst_ringbuf->wr, pst_ringbuf->rd, pst_ringbuf->size);
        return 0;
    }

    real_wr = pst_ringbuf->wr & (pst_ringbuf->size - 1);
    oal_pcie_io_trans(pst_map->va + real_wr, (uintptr_t)pst_item, pst_ringbuf->item_len);
    if (pci_dbg_condtion()) {
        int32_t ret;
        uint64_t cpuaddr;
        ret = oal_pcie_get_ca_by_pa(pst_pci_res, pst_map->pa, &cpuaddr);
        if (ret == OAL_SUCC) {
            pci_print_log(PCI_LOG_DBG, "h2d ringbuf write ringbuf data cpu address:0x%llx", cpuaddr);
        } else {
            pci_print_log(PCI_LOG_DBG, "h2d ringbuf rd pa:0x%lx invaild", pst_map->pa);
        }
        oal_print_hex_dump((uint8_t *)pst_item, pst_ringbuf->item_len, pst_ringbuf->item_len, "ringbuf write: ");
    }

    pst_ringbuf->wr += pst_ringbuf->item_len;

    return 1;
}

OAL_STATIC int32_t oal_pcie_h2d_ringbuf_wr_update(oal_pcie_res *pst_pci_res, pcie_h2d_ringbuf_qtype qtype)
{
    /* h2d方向，同步host的ringbuf管理结构体的写指针到DEVICE侧, 需要刷cache */
    pci_addr_map st_map;

    st_map.va = pst_pci_res->st_tx_res[qtype].ringbuf_ctrl_dma_addr.va + OAL_OFFSET_OF(pcie_ringbuf, wr);
    st_map.pa = pst_pci_res->st_tx_res[qtype].ringbuf_ctrl_dma_addr.pa + OAL_OFFSET_OF(pcie_ringbuf, wr);
    pci_print_log(PCI_LOG_DBG, "h2d ringbuf wr upate wr:%u", pst_pci_res->st_ringbuf.st_h2d_buf[qtype].wr);

    if (oal_unlikely(pst_pci_res->link_state <= PCI_WLAN_LINK_DOWN)) {
        pci_print_log(PCI_LOG_WARN, "pcie is linkdown");
        return -OAL_ENODEV;
    }

    oal_pcie_write_mem32(st_map.va, pst_pci_res->st_ringbuf.st_h2d_buf[qtype].wr);
    if (pci_dbg_condtion()) {
        int32_t ret;
        uint64_t cpuaddr;
        ret = oal_pcie_get_ca_by_pa(pst_pci_res, st_map.pa, &cpuaddr);
        if (ret == OAL_SUCC) {
            pci_print_log(PCI_LOG_DBG, "h2d ringbuf wr cpu address:0x%llx", cpuaddr);
        } else {
            pci_print_log(PCI_LOG_DBG, "h2d ringbuf wr pa:0x%lx invaild", st_map.pa);
        }
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_h2d_ringbuf_rd_update(oal_pcie_res *pst_pci_res, pcie_h2d_ringbuf_qtype qtype)
{
    /* h2d方向，同步device的读指针到HOST ringbuf管理结构体 */
    /* 需要刷cache */
    uint32_t value;
    pci_addr_map st_map;

    st_map.va = pst_pci_res->st_tx_res[qtype].ringbuf_ctrl_dma_addr.va + OAL_OFFSET_OF(pcie_ringbuf, rd);
    st_map.pa = pst_pci_res->st_tx_res[qtype].ringbuf_ctrl_dma_addr.pa + OAL_OFFSET_OF(pcie_ringbuf, rd);

    if (pci_dbg_condtion()) {
        int32_t ret;
        uint64_t cpuaddr;
        ret = oal_pcie_get_ca_by_pa(pst_pci_res, st_map.pa, &cpuaddr);
        if (ret == OAL_SUCC) {
            pci_print_log(PCI_LOG_DBG, "h2d ringbuf rd cpu address:0x%llx", cpuaddr);
        } else {
            pci_print_log(PCI_LOG_DBG, "h2d ringbuf rd pa:0x%lx invaild", st_map.pa);
        }
    }

    if (oal_unlikely(pst_pci_res->link_state <= PCI_WLAN_LINK_DOWN)) {
        pci_print_log(PCI_LOG_WARN, "pcie is linkdown");
        return -OAL_ENODEV;
    }

    value = oal_pcie_read_mem32(st_map.va);
    if (oal_unlikely(value == 0xFFFFFFFF)) {
        if (oal_pcie_check_link_state(pst_pci_res) == OAL_FALSE) {
            pci_print_log(PCI_LOG_ERR, "h2d ringbuf rd update: link down[va:0x%lx, pa:0x%lx]", st_map.va, st_map.pa);
            return -OAL_ENODEV;
        }
    }
    pci_print_log(PCI_LOG_DBG, "h2d ringbuf rd upate rd:%u, curr wr:%u",
                  value, pst_pci_res->st_ringbuf.st_h2d_buf[qtype].wr);
    if (oal_unlikely(value < pst_pci_res->st_ringbuf.st_h2d_buf[qtype].rd)) {
        /* 判断rd 翻转 */
        pci_print_log(PCI_LOG_INFO, "h2d qtype %d new rd %u over old rd %u, wr:%u", qtype,
                      value, pst_pci_res->st_ringbuf.st_h2d_buf[qtype].rd,
                      pst_pci_res->st_ringbuf.st_h2d_buf[qtype].wr);
        declare_dft_trace_key_info("h2d_ringbuf_overrun", OAL_DFT_TRACE_SUCC);
    }
    pst_pci_res->st_ringbuf.st_h2d_buf[qtype].rd = value;

    return OAL_SUCC;
}

/* 获取ringbuf剩余空间大小，is_sync为TRUE时 先从DEVICE同步读指针再判断 */
uint32_t oal_pcie_h2d_ringbuf_freecount(oal_pcie_res *pst_pci_res, pcie_h2d_ringbuf_qtype qtype,
                                        int32_t is_sync)
{
    pcie_ringbuf *pst_ringbuf = &pst_pci_res->st_ringbuf.st_h2d_buf[qtype];

    if (is_sync == OAL_TRUE) {
        /* 同步Host2Dev的读指针 */
        oal_pcie_h2d_ringbuf_rd_update(pst_pci_res, qtype);
    }

    oal_pcie_print_ringbuf_info(pst_ringbuf, PCI_LOG_DBG);
    return oal_pcie_ringbuf_freecount(pst_ringbuf);
}

OAL_STATIC uint32_t oal_pcie_ringbuf_is_empty(pcie_ringbuf *pst_ringbuf)
{
    if (pst_ringbuf->wr == pst_ringbuf->rd) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

uint32_t oal_pcie_h2d_ringbuf_is_empty(oal_pcie_res *pst_pci_res, pcie_h2d_ringbuf_qtype qtype, int32_t is_sync)
{
    pcie_ringbuf *pst_ringbuf = &pst_pci_res->st_ringbuf.st_h2d_buf[qtype];

    if (is_sync == OAL_TRUE) {
        /* 同步Host2Dev的读指针 */
        oal_pcie_h2d_ringbuf_rd_update(pst_pci_res, qtype);
    }

    oal_pcie_print_ringbuf_info(pst_ringbuf, PCI_LOG_DBG);
    return oal_pcie_ringbuf_is_empty(pst_ringbuf);
}

int32_t oal_pcie_edma_tx_is_idle(oal_pcie_res *pst_pci_res, pcie_h2d_ringbuf_qtype qtype)
{
    if (oal_warn_on(pst_pci_res == NULL)) {
        pci_print_log(PCI_LOG_WARN, "pci res is null");
        return OAL_FALSE;
    }

    /* pcie is link */
    if (oal_unlikely(pst_pci_res->link_state <= PCI_WLAN_LINK_DOWN)) {
        return OAL_FALSE;
    }

    return (oal_pcie_h2d_ringbuf_freecount(pst_pci_res, qtype, OAL_FALSE) ? OAL_TRUE : OAL_FALSE);
}


uint32_t oal_pcie_d2h_ringbuf_freecount(oal_pcie_res *pst_pci_res, int32_t is_sync)
{
    pcie_ringbuf *pst_ringbuf = &pst_pci_res->st_ringbuf.st_d2h_buf;

    if (is_sync == OAL_TRUE) {
        /* 同步Dev2Host的读指针 */
        oal_pcie_d2h_ringbuf_rd_update(pst_pci_res);
    }

    return oal_pcie_ringbuf_freecount(pst_ringbuf);
}

OAL_STATIC int32_t oal_pcie_rx_ringbuf_supply(oal_pcie_res *pst_pci_res,
                                              int32_t is_sync,
                                              int32_t is_doorbell,
                                              uint32_t request_cnt,
                                              int32_t gflag,
                                              int32_t *ret)
{
    uint32_t i;
    uint32_t cnt = 0;
    unsigned long flags;
    oal_netbuf_stru *pst_netbuf = NULL;
    pcie_cb_dma_res *pst_cb_res = NULL;
    dma_addr_t pci_dma_addr;
    pcie_write_ringbuf_item st_write_item;
    oal_pci_dev_stru *pst_pci_dev = pcie_res_to_dev(pst_pci_res);
    *ret = OAL_SUCC;
    oal_pcie_mips_start(PCIE_MIPS_RX_NETBUF_SUPPLY);
    if (is_sync == OAL_TRUE) {
        /* 同步Dev2Host的读指针 */
        oal_pcie_d2h_ringbuf_rd_update(pst_pci_res);
    }

    for (i = 0; i < request_cnt; i++) {
        if (oal_pcie_d2h_ringbuf_freecount(pst_pci_res, OAL_FALSE) == 0) {
            break;
        }
        /* ringbuf 有空间 */
        /* 预申请netbuf都按照大包来申请 */
        oal_pcie_mips_start(PCIE_MIPS_RX_MEM_ALLOC);
        pst_netbuf = oal_pcie_rx_netbuf_alloc(HCC_HDR_TOTAL_LEN + PCIE_EDMA_TRANS_MAX_FRAME_LEN, gflag);
        if (pst_netbuf == NULL) {
            pst_pci_res->st_rx_res.stat.alloc_netbuf_failed++;
            *ret = -OAL_ENOMEM;
            oal_pcie_mips_end(PCIE_MIPS_RX_MEM_ALLOC);
            break;
        }
        oal_netbuf_put(pst_netbuf, (HCC_HDR_TOTAL_LEN + PCIE_EDMA_TRANS_MAX_FRAME_LEN));
        oal_pcie_mips_end(PCIE_MIPS_RX_MEM_ALLOC);

        if (g_hipci_sync_flush_cache_enable == 0) {
            // sync dma未使能，map时会clean & inv操作
            oal_pcie_rx_netbuf_hdr_init(pst_pci_dev, pst_netbuf);
        }

        pci_dma_addr = dma_map_single(&pst_pci_dev->dev, oal_netbuf_data(pst_netbuf),
                                      oal_netbuf_len(pst_netbuf), PCI_DMA_FROMDEVICE);
        if (dma_mapping_error(&pst_pci_dev->dev, pci_dma_addr)) {
            pst_pci_res->st_rx_res.stat.map_netbuf_failed++;
            pci_print_log(PCI_LOG_INFO, "rx dma map netbuf failed, len=%u,cnt:%u",
                          oal_netbuf_len(pst_netbuf),
                          pst_pci_res->st_rx_res.stat.map_netbuf_failed);
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

        st_write_item.buff_paddr.addr = pci_dma_addr;

        /* 入队 */
        oal_spin_lock_irq_save(&pst_pci_res->st_rx_res.lock, &flags);
        oal_netbuf_list_tail_nolock(&pst_pci_res->st_rx_res.rxq, pst_netbuf);
        oal_spin_unlock_irq_restore(&pst_pci_res->st_rx_res.lock, &flags);

        pci_print_log(PCI_LOG_DBG, "d2h ringbuf write [netbuf:0x%p, data:[va:0x%lx,pa:0x%llx]",
                      pst_netbuf, (uintptr_t)oal_netbuf_data(pst_netbuf), st_write_item.buff_paddr.addr);
        oal_pcie_mips_start(PCIE_MIPS_RX_RINGBUF_WRITE);
        if (oal_unlikely(oal_pcie_d2h_ringbuf_write(pst_pci_res,
            &pst_pci_res->st_rx_res.ringbuf_data_dma_addr,
            &st_write_item) == 0)) {
            oal_pcie_mips_end(PCIE_MIPS_RX_RINGBUF_WRITE);
            break;
        }
        oal_pcie_mips_end(PCIE_MIPS_RX_RINGBUF_WRITE);

        cnt++;
    }

    /* 这里需要考虑HOST/DEVICE的初始化顺序 */
    if (cnt && (is_doorbell == OAL_TRUE)) {
        oal_pcie_d2h_ringbuf_wr_update(pst_pci_res);
#ifdef _PRE_COMMENT_CODE_
        /* 暂时不需要敲铃，D2H Device是大循环 */
        oal_pcie_d2h_doorbell(pst_pci_res);
#endif
    }

    oal_pcie_mips_end(PCIE_MIPS_RX_NETBUF_SUPPLY);

    return cnt;
}


OAL_STATIC int32_t oal_pcie_rx_thread_condtion(oal_atomic *pst_ato)
{
    int32_t ret = oal_atomic_read(pst_ato);
    if (oal_likely(ret == 1)) {
        oal_atomic_set(pst_ato, 0);
    }

    return ret;
}

/* 预先分配rx的接收buf */
int32_t oal_pcie_rx_ringbuf_build(oal_pcie_res *pst_pci_res)
{
    /* 走到这里要确保DEVICE ZI区已经初始化完成， 中断已经注册和使能 */
    int32_t ret;
    int32_t supply_num;
    supply_num = oal_pcie_rx_ringbuf_supply(pst_pci_res, OAL_TRUE, OAL_TRUE,
                                            PCIE_RX_RINGBUF_SUPPLY_ALL, GFP_KERNEL, &ret);
    if (supply_num == 0) {
        pci_print_log(PCI_LOG_WARN, "oal_pcie_rx_ringbuf_build can't get any netbufs!, rxq len:%u",
                      oal_netbuf_list_len(&pst_pci_res->st_rx_res.rxq));
        oal_pcie_print_ringbuf_info(&pst_pci_res->st_ringbuf.st_d2h_buf, PCI_LOG_WARN);
        return -OAL_ENOMEM;
    } else {
        pci_print_log(PCI_LOG_INFO, "oal_pcie_rx_ringbuf_build got %u netbufs!", supply_num);
    }
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_rx_hi_thread(void *data)
{
    int32_t ret;
    int32_t supply_num;
    oal_pcie_res *pst_pcie_res = (oal_pcie_res *)data;

    if (oal_warn_on(pst_pcie_res == NULL)) {
        pci_print_log(PCI_LOG_ERR, "%s error: pst_pcie_res is null", __FUNCTION__);
        return -EFAIL;
    };

    allow_signal(SIGTERM);

    forever_loop() {
        if (oal_unlikely(kthread_should_stop())) {
            break;
        }

        ret = oal_wait_event_interruptible_m(pst_pcie_res->st_rx_hi_wq,
                                             oal_pcie_rx_thread_condtion(&pst_pcie_res->rx_hi_cond));
        if (oal_unlikely(ret == -ERESTARTSYS)) {
            pci_print_log(PCI_LOG_INFO, "task %s was interrupted by a signal\n", oal_get_current_task_name());
            break;
        }

        mutex_lock(&pst_pcie_res->st_rx_mem_lock);
        if (oal_unlikely(pst_pcie_res->link_state < PCI_WLAN_LINK_WORK_UP || !pst_pcie_res->regions.inited)) {
            pci_print_log(PCI_LOG_WARN, "hi thread link invaild, stop supply mem, link_state:%s, region:%d",
                          oal_pcie_get_link_state_str(pst_pcie_res->link_state),
                          pst_pcie_res->regions.inited);
        } else {
            supply_num = oal_pcie_rx_ringbuf_supply(pst_pcie_res, OAL_TRUE, OAL_TRUE,
                                                    PCIE_RX_RINGBUF_SUPPLY_ALL,
                                                    GFP_ATOMIC | __GFP_NOWARN, &ret);
            if (ret != OAL_SUCC) {
                /* 补充内存失败，成功则忽略，有可能当前不需要补充内存也视为成功 */
                oal_pcie_shced_rx_normal_thread(pst_pcie_res);
            }
        }
        mutex_unlock(&pst_pcie_res->st_rx_mem_lock);
    }

    return 0;
}

OAL_STATIC int32_t oal_pcie_rx_normal_thread(void *data)
{
    int32_t resched;
    int32_t ret;
    int32_t supply_num;
    oal_pcie_res *pst_pcie_res = (oal_pcie_res *)data;

    if (oal_warn_on(pst_pcie_res == NULL)) {
        pci_print_log(PCI_LOG_ERR, "%s error: pst_pcie_res is null", __FUNCTION__);
        return -EFAIL;
    };

    allow_signal(SIGTERM);

    forever_loop() {
        resched = 0;
        if (oal_unlikely(kthread_should_stop())) {
            break;
        }

        ret = oal_wait_event_interruptible_m(pst_pcie_res->st_rx_normal_wq,
                                             oal_pcie_rx_thread_condtion(&pst_pcie_res->rx_normal_cond));
        if (oal_unlikely(ret == -ERESTARTSYS)) {
            pci_print_log(PCI_LOG_INFO, "task %s was interrupted by a signal\n", oal_get_current_task_name());
            break;
        }

        mutex_lock(&pst_pcie_res->st_rx_mem_lock);
        if (oal_unlikely(pst_pcie_res->link_state < PCI_WLAN_LINK_WORK_UP || !pst_pcie_res->regions.inited)) {
            pci_print_log(PCI_LOG_WARN, "hi thread link invaild, stop supply mem, link_state:%s, region:%d",
                          oal_pcie_get_link_state_str(pst_pcie_res->link_state),
                          pst_pcie_res->regions.inited);
        } else {
            supply_num = oal_pcie_rx_ringbuf_supply(pst_pcie_res, OAL_TRUE, OAL_TRUE,
                                                    PCIE_RX_RINGBUF_SUPPLY_ALL, GFP_KERNEL, &ret);
            if (ret != OAL_SUCC) {
                resched = 1;
            }
        }

        mutex_unlock(&pst_pcie_res->st_rx_mem_lock);

        if (resched) {
            /*
             * 补充内存失败，成功则忽略，有可能当前不需要补充内存也视为成功,
             * 如果GFP_KERNEL 方式补充失败，则启动轮询,循环申请
             */
            oal_schedule();
            oal_pcie_shced_rx_normal_thread(pst_pcie_res);
        }
    }

    return 0;
}


/* edma functions */
OAL_STATIC int32_t oal_pcie_edma_get_read_done_fifo(oal_pcie_res *pst_pci_res, edma_paddr_t *addr,
                                                    uint32_t *count)
{
    /* 后续考虑优化， PCIE读内存比较耗时 */
    /* 先读FIFO0，分三次读走64bit 数据，读空 */
    uint32_t addr_low, addr_high;
    uint32_t trans_count;

    /* 处理fifo0 */
    trans_count = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_READ_FIFO0_DATA_OFF);
    addr_low = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_READ_FIFO0_DATA_OFF + 4); // 4B offset
    addr_high = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_READ_FIFO0_DATA_OFF + 8); // 8B offset
    if (pci_dbg_condtion()) {
        addr_low = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_READ_FIFO0_DATA_OFF + 4); // 4B offset
        addr_high = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_READ_FIFO0_DATA_OFF + 8); // 8B offset
        pci_print_log(PCI_LOG_DBG, "read done fifo0 addr_low:0x%8x, addr_high:0x%8x, trans_count:0x%8x\n",
                      addr_low, addr_high, trans_count);
    }

    trans_count = trans_count >> 1; /* 一个数据包对应2个描述符 */
    addr->bits.low_addr = addr_low;
    addr->bits.high_addr = addr_high;
    *count = trans_count;

    /* 处理fifo1 */
    addr++;
    count++;

    trans_count = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_READ_FIFO1_DATA_OFF);
    addr_low = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_READ_FIFO1_DATA_OFF + 4); // 4B offset
    addr_high = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_READ_FIFO1_DATA_OFF + 8); // 8B offset

    if (pci_dbg_condtion()) {
        addr_low = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_READ_FIFO1_DATA_OFF + 4); // 4B offset
        addr_high = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_READ_FIFO1_DATA_OFF + 8); // 8B offset
        pci_print_log(PCI_LOG_DBG, "read done fifo1 addr_low:0x%8x, addr_high:0x%8x, trans_count:0x%8x\n",
                      addr_low, addr_high, trans_count);
    }

    trans_count = trans_count >> 1; /* 一个数据包对应2个描述符 */
    addr->bits.low_addr = addr_low;
    addr->bits.high_addr = addr_high;
    *count = trans_count;

    return OAL_TRUE;
}

OAL_STATIC int32_t oal_pcie_edma_get_write_done_fifo(oal_pcie_res *pst_pci_res, edma_paddr_t *addr,
                                                     uint32_t *count)
{
    /* 后续考虑优化， PCIE读内存比较耗时 */
    /* 先读FIFO0，分三次读走64bit 数据，读空 */
    uint32_t addr_low, addr_high;
    uint32_t trans_count;

    /* 处理fifo0 */
    trans_count = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_WRITE_FIFO0_DATA_OFF);
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE_FIFO_ADDRESS
    addr_low = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_WRITE_FIFO0_DATA_OFF + 4); // 4B offset
    addr_high = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_WRITE_FIFO0_DATA_OFF + 8); // 8B offset
#endif

    if (pci_dbg_condtion()) {
#ifndef _PRE_PLAT_FEATURE_HI110X_PCIE_FIFO_ADDRESS
        addr_low = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_WRITE_FIFO0_DATA_OFF + 4); // 4B offset
        addr_high = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_WRITE_FIFO0_DATA_OFF + 8); // 8B offset
#endif
        pci_print_log(PCI_LOG_DBG, "write done fifo0 addr_low:0x%8x, addr_high:0x%8x, trans_count:0x%8x\n",
                      addr_low, addr_high, trans_count);
    }

    trans_count = trans_count >> 1; /* 一个数据包对应2个描述符 */
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE_FIFO_ADDRESS
    addr->bits.low_addr = addr_low;
    addr->bits.high_addr = addr_high;
#endif
    *count = trans_count;

    /* 处理fifo1 */
    addr++;
    count++;

    trans_count = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_WRITE_FIFO1_DATA_OFF);
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE_FIFO_ADDRESS
    addr_low = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_WRITE_FIFO1_DATA_OFF + 4); // 4B offset
    addr_high = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_WRITE_FIFO1_DATA_OFF + 8); // 8B offset
#endif

    if (pci_dbg_condtion()) {
#ifndef _PRE_PLAT_FEATURE_HI110X_PCIE_FIFO_ADDRESS
        addr_low = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_WRITE_FIFO1_DATA_OFF + 4); // 4B offset
        addr_high = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_FIFO_REMOTE_WRITE_FIFO1_DATA_OFF + 8); // 8B offset
#endif
        pci_print_log(PCI_LOG_DBG, "write done fifo1 addr_low:0x%8x, addr_high:0x%8x, trans_count:0x%8x\n",
                      addr_low, addr_high, trans_count);
    }

    trans_count = trans_count >> 1; /* 一个数据包对应2个描述符 */
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE_FIFO_ADDRESS
    addr->bits.low_addr = addr_low;
    addr->bits.high_addr = addr_high;
#endif
    *count = trans_count;

    return OAL_TRUE;
}


OAL_STATIC OAL_INLINE void oal_pcie_h2d_transfer_done_finish(oal_pcie_res *pst_pci_res, int32_t finish, int32_t i)
{
    if (!finish) {
        /* 维测,未找到FIFO中的地址，地址有错，或者count有错,或者丢中断 这里应该触发DFR */
        declare_dft_trace_key_info("pcie tx done addr error", OAL_DFT_TRACE_EXCEP);
        pci_print_log(PCI_LOG_ERR, "pcie tx done addr error");
    } else {
        oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pcie_res_to_dev(pst_pci_res));
        /* 发送完成,唤醒发送线程 */
        if (oal_likely(pst_pci_lres != NULL)) {
            pci_print_log(PCI_LOG_DBG, "pcie sched hcc thread, qid:%d", i);
            oal_atomic_set(&pst_pci_res->st_tx_res[i].tx_ringbuf_sync_cond, 1); /* 下半部刷新，保持一致性 */
            hcc_sched_transfer(hbus_to_hcc(pst_pci_lres->pst_bus));
        }
    }
}

/* chan_cnt 是已经完成传输完成的netbuf个数，如果队列中不足表示下溢 */
OAL_STATIC OAL_INLINE int32_t oal_pcie_h2d_transfer_free_netbuf(oal_pcie_res *pst_pci_res,
                                                                oal_netbuf_head_stru *pst_txq,
                                                                int32_t i,
                                                                int32_t chan_cnt)
{
    unsigned long flags;
    int32_t cnt = chan_cnt;
    oal_netbuf_stru *pst_netbuf = NULL;
    do {
        /* 这里的锁可以优化 */
        oal_spin_lock_irq_save(&pst_pci_res->st_tx_res[i].lock, &flags);
        /* 头部出队 */
        pst_netbuf = oal_netbuf_delist_nolock(pst_txq);
        oal_spin_unlock_irq_restore(&pst_pci_res->st_tx_res[i].lock, &flags);
        if (pst_netbuf == NULL) {
            /* 不应该为空，count有可能有问题 */
            pci_print_log(PCI_LOG_ERR, "tx netbuf queue underflow[left_cnt:%d]", cnt);
            declare_dft_trace_key_info("pcie tx done count error2", OAL_DFT_TRACE_EXCEP);
            return -OAL_EIO;
        }
#ifdef _PRE_WLAN_PKT_TIME_STAT
        if (DELAY_STATISTIC_SWITCH_ON) {
            delay_record_snd_combine(pst_netbuf);
        }
#endif
        /* unmap dma addr & free netbuf */
        pst_pci_res->st_tx_res[i].stat.tx_done_count++;
        pci_print_log(PCI_LOG_DBG, "h2d send netbuf ok, va:0x%p, cnt:%u",
                      pst_netbuf, pst_pci_res->st_tx_res[i].stat.tx_done_count);
        oal_pcie_tx_netbuf_free(pst_pci_res, pst_netbuf);
    } while (--cnt);

    if (oal_likely(!cnt)) {
        /* 一个通道的地址处理完成 */
        return OAL_SUCC;
    } else {
        return -OAL_EFAIL;
    }
}

OAL_STATIC OAL_INLINE void oal_pcie_h2d_transfer_info_stat(oal_pcie_res *pst_pci_res,
                                                           int32_t i, // qtype
                                                           int32_t total_cnt)
{
    if (oal_unlikely(total_cnt > PCIE_EDMA_READ_BUSRT_COUNT)) {
        pst_pci_res->st_tx_res[i].stat.tx_burst_cnt[0]++;
    } else {
        if (total_cnt) {
            pst_pci_res->st_tx_res[i].stat.tx_burst_cnt[total_cnt]++;
        }
    }

    pst_pci_res->st_tx_res[i].stat.tx_count += total_cnt;
}

int32_t oal_pcie_host_pending_signal_check(oal_pcie_res *pst_pci_res)
{
    int32_t i = 0;
    for (i = 0; i < PCIE_H2D_QTYPE_BUTT; i++) {
        if (oal_atomic_read(&pst_pci_res->st_tx_res[i].tx_ringbuf_sync_cond)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

int32_t oal_pcie_host_pending_signal_process(oal_pcie_res *pst_pci_res)
{
    int32_t i = 0;
    int32_t total_cnt = 0;
    oal_pcie_linux_res *pst_pci_lres;
    oal_pci_dev_stru *pst_pci_dev;
    pst_pci_dev = pcie_res_to_dev(pst_pci_res);
    pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pci_dev);
    if (oal_unlikely(OAL_SUCC != hcc_bus_pm_wakeup_device(pst_pci_lres->pst_bus))) {
        oal_msleep(100); /* wait for a while retry */
        for (i = 0; i < PCIE_H2D_QTYPE_BUTT; i++) {
            oal_atomic_set(&pst_pci_res->st_tx_res[i].tx_ringbuf_sync_cond, 0);
        }
        return total_cnt;
    }

    for (i = 0; i < PCIE_H2D_QTYPE_BUTT; i++) {
        if (oal_atomic_read(&pst_pci_res->st_tx_res[i].tx_ringbuf_sync_cond)) {
            oal_atomic_set(&pst_pci_res->st_tx_res[i].tx_ringbuf_sync_cond, 0);
            if (oal_pcie_h2d_ringbuf_rd_update(pst_pci_res, (pcie_h2d_ringbuf_qtype)i) == OAL_SUCC) {
                total_cnt++;
            }
        }
    }

    return total_cnt;
}

/* isr functions */
OAL_STATIC int32_t oal_pcie_tx_dma_addr_match(oal_netbuf_stru *pst_netbuf, edma_paddr_t dma_addr)
{
    /* dma_addr 存放在CB字段里 */
    pcie_cb_dma_res st_cb_dma;
    int32_t ret;

    /* 不是从CB的首地址开始，必须拷贝，对齐问题。 */
    ret = memcpy_s(&st_cb_dma, sizeof(pcie_cb_dma_res),
                   (uint8_t *)oal_netbuf_cb(pst_netbuf) + sizeof(struct hcc_tx_cb_stru),
                   sizeof(pcie_cb_dma_res));
    if (ret != EOK) {
        pci_print_log(PCI_LOG_ERR, "get tx dma addr failed");
        return OAL_FALSE;
    }

    pci_print_log(PCI_LOG_DBG, "tx dma addr match, cb's addr 0x%llx , dma_addr 0x%llx",
                  st_cb_dma.paddr.addr, dma_addr.addr);

    if (st_cb_dma.paddr.addr == dma_addr.addr) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

/* 发送完成中断 */
OAL_STATIC void oal_pcie_h2d_transfer_done(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    int32_t i, j, flag, cnt, total_cnt, netbuf_cnt, curr_cnt;
    oal_netbuf_stru *pst_netbuf = NULL;
    oal_netbuf_head_stru *pst_txq = NULL;
    /* tx fifo中获取 发送完成的首地址,双通道，双地址，双count */
    edma_paddr_t addr[PCIE_EDMA_MAX_CHANNELS];
    uint32_t count[PCIE_EDMA_MAX_CHANNELS];

    pst_pci_res->stat.intx_tx_count++;

    if (oal_pcie_edma_get_read_done_fifo(pst_pci_res, addr, count) != OAL_TRUE) {
        /* 待增加维测计数 */
        return;
    }

    pci_print_log(PCI_LOG_DBG, "oal_pcie_h2d_transfer_done, cnt:%u", pst_pci_res->stat.intx_tx_count);

    /*
     * 获取到发送完成的DMA地址，遍历发送队列,
     * 先遍历第一个元素，正常应该队头就是发送完成的元素，
     * 如果不在队头说明丢中断了(有FIFO正常不会丢),需要释放元素之前的netbuf
     */
    flag = 0;
    netbuf_cnt = 0;

    for (i = 0; i < PCIE_H2D_QTYPE_BUTT; i++) {
        total_cnt = 0;

        pst_txq = &pst_pci_res->st_tx_res[i].txq;

        if (oal_netbuf_list_empty(pst_txq)) {
            /* next txq */
            continue;
        }

        /* DMA双通道 */
        for (j = 0; j < PCIE_EDMA_MAX_CHANNELS; j++) {
            if (oal_netbuf_list_empty(pst_txq)) {
                /* 队列为空 */
                break;
            }

            pci_print_log(PCI_LOG_DBG, "[q:%d]tx chan:%d pa 0x%llx, cnt:%d", i, j, addr[j].addr, count[j]);

            cnt = count[j]; /* 无效描述符时,count为0 */
            if (!cnt) {
#ifdef _PRE_COMMENT_CODE_
                if (addr[j].addr) {
                    /* cnt 为0 时，addr 是上一次的值 */
                    pci_print_log(PCI_LOG_DBG, "tx chan:%d get invalid dma pa 0x%llx", j, addr[j].addr);
                }
#endif
                continue;
            }

            /* 保证一个地方入队，这里出队 */
            pst_netbuf = (oal_netbuf_stru *)oal_netbuf_next(pst_txq);
            if (oal_pcie_tx_dma_addr_match(pst_netbuf, addr[j]) != OAL_TRUE) {
                /* 地址不匹配，遍历下一个队列 */
                pci_print_log(PCI_LOG_DBG, "[q:%d]tx chan:%d match failed, search next txq", i, j);
                break;
            }

            /* match succ
             * 找到地址，出队,先入先出，所以先检查通道0，再检查通道1,
             * 2个通道的地址 应该在同一个队列中
             */
            curr_cnt = oal_netbuf_list_len(pst_txq);
            if (oal_unlikely(cnt > curr_cnt)) {
                /* count 出错? */
                pci_print_log(PCI_LOG_ERR, "[q:%d]tx chan:%d tx done invalid count cnt %d ,list len %u", i, j,
                              cnt, curr_cnt);
                declare_dft_trace_key_info("pcie tx done count error", OAL_DFT_TRACE_EXCEP);
                flag = 0;
                goto done;
            }

            total_cnt += cnt;
            pci_print_log(PCI_LOG_DBG, "[q:%d]tx chan:%d, process netbuf list, cnt:%u",
                          i, j, pst_pci_res->st_tx_res[i].stat.tx_done_count);
            ret = oal_pcie_h2d_transfer_free_netbuf(pst_pci_res, pst_txq, i, cnt); // i:qtype
            if (oal_likely(ret == OAL_SUCC)) {
                flag = 1;
                pci_print_log(PCI_LOG_DBG, "[q:%d]tx chan:%d all bus process done!", i, j);
            } else {
                flag = 0; // failed
                pci_print_log(PCI_LOG_ERR, "[q:%d]tx chan:%d tx netbuf queue underflow[cnt:%d:%d, qlen:%d]", i, j,
                              cnt, count[j], curr_cnt);
                goto done;
            }
        }

        netbuf_cnt += total_cnt;

        oal_pcie_h2d_transfer_info_stat(pst_pci_res, i, total_cnt);

        if (flag) {
            break;
        }

        /* 未匹配，遍历下一条队列 */
    }

    if (oal_unlikely(netbuf_cnt != (count[0] + count[1]))) {
        pci_print_log(PCI_LOG_WARN, "count error total cnt:%u != dev reg count[0]%u  count[1] %u",
                      netbuf_cnt, count[0], count[1]);
        oal_disable_pcie_irq((oal_pcie_linux_res *)oal_pci_get_drvdata(pcie_res_to_dev(pst_pci_res)));
        flag = 0;
        goto done;
    }

done:
    oal_pcie_h2d_transfer_done_finish(pst_pci_res, flag, i);
}

static uint32_t g_rx_addr_count_err_cnt = 0;
/* dev to host transfer done lable function */
static void oal_pcie_d2h_transfer_done_finish(oal_pcie_res *pst_pci_res, int32_t finish)
{
    if (!finish) {
        /* 维测,未找到FIFO中的地址，地址有错，或者count有错,或者丢中断 这里应该触发DFR */
        oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pcie_res_to_dev(pst_pci_res));
        pci_print_log(PCI_LOG_ERR, "pcie rx done addr error");
        oal_disable_pcie_irq(pst_pci_lres);

        /* DFR trigger */
        oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_DOWN);
        hcc_bus_exception_submit(pst_pci_lres->pst_bus, WIFI_TRANS_FAIL);

        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_PCIE, CHR_PLAT_DRV_ERROR_D2H_TRANSFER_PCIE_LINK_DOWN);
    } else {
        oal_pcie_linux_res *pst_pci_lres = NULL;
        pci_print_log(PCI_LOG_DBG, "d2h trigger hcc_sched_transfer, dev:%p, lres:%p",
                      pcie_res_to_dev(pst_pci_res), oal_pci_get_drvdata(pcie_res_to_dev(pst_pci_res)));

        pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pcie_res_to_dev(pst_pci_res));
        /* 发送完成,唤醒发送线程 */
        if (oal_likely(pst_pci_lres != NULL)) {
            if (oal_likely(pst_pci_lres->pst_bus)) {
                hcc_sched_transfer(hbus_to_hcc(pst_pci_lres->pst_bus));
            } else {
                pci_print_log(PCI_LOG_ERR, "lres's bus is null! %p", pst_pci_lres);
            }

            /* 通知线程，补充RX内存 */
            oal_pcie_shced_rx_hi_thread(pst_pci_res);
        }
    }
}

OAL_STATIC OAL_INLINE void oal_pcie_d2h_transfer_info_stat(oal_pcie_res *pst_pci_res,
                                                           int32_t total_cnt)
{
    if (oal_unlikely(total_cnt > PCIE_EDMA_WRITE_BUSRT_COUNT)) {
        pst_pci_res->st_rx_res.stat.rx_burst_cnt[0]++;
    } else {
        pst_pci_res->st_rx_res.stat.rx_burst_cnt[total_cnt]++;
    }

    pst_pci_res->st_rx_res.stat.rx_count += total_cnt;
}

OAL_STATIC OAL_INLINE int32_t oal_pcie_d2h_transfer_submit_netbuf(oal_pcie_res *pst_pci_res,
                                                                  oal_netbuf_head_stru *pst_rxq,
                                                                  int32_t chan_cnt)
{
    unsigned long flags;
    int32_t cnt = chan_cnt;
    const uint32_t ul_max_dump_bytes = 128;
    oal_netbuf_stru *pst_netbuf = NULL;

    oal_pcie_mips_start(PCIE_MIPS_RX_QUEUE_POP);
    do {
        /* 这里的锁可以优化 */
        oal_spin_lock_irq_save(&pst_pci_res->st_rx_res.lock, &flags);

        /* 头部出队 */
        pst_netbuf = oal_netbuf_delist_nolock(pst_rxq);
        oal_spin_unlock_irq_restore(&pst_pci_res->st_rx_res.lock, &flags);
        if (pst_netbuf == NULL) {
            oal_pcie_mips_end(PCIE_MIPS_RX_QUEUE_POP);

            /* 不应该为空，count有可能有问题 */
            pci_print_log(PCI_LOG_ERR, "rx netbuf queue underflow[left cnt:%d, qlen:%d]",
                          cnt, oal_netbuf_list_len(pst_rxq));
            declare_dft_trace_key_info("pcie rx done count error2", OAL_DFT_TRACE_EXCEP);
            return -OAL_EIO;
        }
        pst_pci_res->st_rx_res.stat.rx_done_count++;
        pci_print_log(PCI_LOG_DBG, "rx netbuf done, va:0x%p, cnt:%u",
                      pst_netbuf, pst_pci_res->st_rx_res.stat.rx_done_count);
        if (pci_dbg_condtion()) {
            oal_print_hex_dump(oal_netbuf_data(pst_netbuf),
                               oal_netbuf_len(pst_netbuf) <
                               ul_max_dump_bytes
                               ? oal_netbuf_len(pst_netbuf)
                               : ul_max_dump_bytes,
                               HEX_DUMP_GROUP_SIZE, "d2h payload: ");
        }
        /* unmap dma addr & free netbuf */
        oal_pcie_rx_netbuf_submit(pst_pci_res, pst_netbuf);
    } while (--cnt);
    oal_pcie_mips_end(PCIE_MIPS_RX_QUEUE_POP);

    if (oal_likely(!cnt)) {
        /* 一个通道的地址处理完成 */
        pci_print_log(PCI_LOG_DBG, "rx all bus process done!");
        return OAL_SUCC;
    } else {
        return -OAL_EIO;
    }
}


OAL_STATIC int32_t oal_pcie_rx_dma_addr_match(oal_netbuf_stru *pst_netbuf, edma_paddr_t dma_addr)
{
    pcie_cb_dma_res *pst_cb_res;
    /* DMA地址填到CB中, CB首地址8字节对齐可以直接强转 */
    pst_cb_res = (pcie_cb_dma_res *)oal_netbuf_cb(pst_netbuf);
    if (pst_cb_res->paddr.addr == dma_addr.addr) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}


/* 接收完成中断 */
OAL_STATIC void oal_pcie_d2h_transfer_done(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    int32_t i, flag, cnt, total_cnt;
    oal_netbuf_stru *pst_netbuf = NULL;
    oal_netbuf_head_stru *pst_rxq = NULL;
    /* rx fifo中获取 发送完成的首地址,双通道，双地址，双count */
    edma_paddr_t addr[PCIE_EDMA_MAX_CHANNELS];
    uint32_t count[PCIE_EDMA_MAX_CHANNELS];

    pst_pci_res->stat.intx_rx_count++;

    oal_pcie_mips_start(PCIE_MIPS_RX_MSG_FIFO);
    if (oal_pcie_edma_get_write_done_fifo(pst_pci_res, addr, count) != OAL_TRUE) {
        /* 待增加维测计数 */
        return;
    }
    oal_pcie_mips_end(PCIE_MIPS_RX_MSG_FIFO);

    pci_print_log(PCI_LOG_DBG, "oal_pcie_d2h_transfer_done, cnt:%u", pst_pci_res->stat.intx_rx_count);

    /*
     * 获取到发送完成的DMA地址，遍历发送队列,
     * 先遍历第一个元素，正常应该队头就是发送完成的元素，
     * 如果不在队头说明丢中断了(有FIFO正常不会丢),需要释放元素之前的netbuf
     */
    flag = 0;
    total_cnt = 0;

    pst_rxq = &pst_pci_res->st_rx_res.rxq;

    if (oal_netbuf_list_empty(pst_rxq)) {
        /* next txq */
        declare_dft_trace_key_info("pcie rx done fifo error", OAL_DFT_TRACE_EXCEP);
        return;
    }

    /* DMA双通道 */
    for (i = 0; i < PCIE_EDMA_MAX_CHANNELS; i++) {
        if (oal_netbuf_list_empty(pst_rxq)) {
            /* 队列为空 */
            break;
        }

        pci_print_log(PCI_LOG_DBG, "rx chan:%d pa 0x%llx, cnt:%d", i, addr[i].addr, count[i]);
        cnt = count[i]; /* 无效描述符时,count为0 */
        if (!cnt) {
#ifdef _PRE_COMMENT_CODE_
            if (addr[i].addr) {
                /* cnt 和 addr 应该同时为 0 */
                pci_print_log(PCI_LOG_DBG, "rx chan:%d get invalid dma pa 0x%llx", i, addr[i].addr);
            }
#endif
            continue;
        }

        /* 保证一个地方入队，这里出队 */
        pst_netbuf = (oal_netbuf_stru *)oal_netbuf_next(pst_rxq);
        if (oal_unlikely(pst_netbuf == NULL)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: pst_netbuf is null", __FUNCTION__);
            continue;
        };

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE_FIFO_ADDRESS
        if (oal_pcie_rx_dma_addr_match(pst_netbuf, addr[i]) != OAL_TRUE) {
            g_rx_addr_count_err_cnt++;
            if (g_rx_addr_count_err_cnt >= 2) { // 2, err retry cnt
                /* 地址不匹配 重试一次 */
                pci_print_log(PCI_LOG_ERR, "rx chan:%d match failed, rx error, count[i]:%u, errcnt:%d",
                              i, count[i], g_rx_addr_count_err_cnt);
                pci_print_log(PCI_LOG_ERR, "count0:%u, count1:%u", count[0], count[1]);
                declare_dft_trace_key_info("pcie rx addr fatal error", OAL_DFT_TRACE_EXCEP);
                flag = 0;
                goto done;
            } else {
                /* 地址不匹配，出错 */
                pci_print_log(PCI_LOG_ERR, "rx chan:%d match failed, rx error, count[i]:%u, errcnt:%d",
                              i, count[i], g_rx_addr_count_err_cnt);
                pci_print_log(PCI_LOG_ERR, "count0:%u, count1:%u", count[0], count[1]);
                declare_dft_trace_key_info("pcie rx addr error,retry", OAL_DFT_TRACE_FAIL);
                flag = 1;
                goto done;
            }
        } else {
            g_rx_addr_count_err_cnt = 0;
        }
#endif

        /*
         * 找到地址，出队,先入先出，所以先检查通道0，再检查通道1,
         * 2个通道的地址 应该在同一个队列中
         */
        cnt = count[i];
        if (oal_unlikely(cnt > oal_netbuf_list_len(pst_rxq))) {
            pci_print_log(PCI_LOG_ERR, "rx chan:%d rx done invalid count cnt %d ,list len %u", i,
                          cnt, oal_netbuf_list_len(pst_rxq));
            declare_dft_trace_key_info("pcie rx done count error", OAL_DFT_TRACE_EXCEP);
            flag = 0;
            goto done;
        }
        total_cnt += cnt;

        ret = oal_pcie_d2h_transfer_submit_netbuf(pst_pci_res, pst_rxq, cnt);
        if (oal_likely(ret == OAL_SUCC)) {
            flag = 1;
            pci_print_log(PCI_LOG_DBG, "rx chan:%d ok", i);
        } else {
            flag = 0;
            pci_print_log(PCI_LOG_ERR, "rx chan:%d cnt=%d netbuf queue failed, ret=%d", i, cnt, ret);
            goto done;
        }
    }

    oal_pcie_d2h_transfer_info_stat(pst_pci_res, total_cnt);

done:
    oal_pcie_d2h_transfer_done_finish(pst_pci_res, flag);
}

OAL_STATIC OAL_INLINE int32_t oal_pcie_transfer_done_input_check(oal_pcie_res *pst_pci_res)
{
    if (oal_unlikely(pst_pci_res == NULL)) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_res is null!");
        return -OAL_ENODEV;
    }

    pst_pci_res->stat.intx_total_count++;
    pci_print_log(PCI_LOG_DBG, "intx int count:%u", pst_pci_res->stat.intx_total_count);

    /* Host收到intx中断,遍历TX/RX FIFO寄存器 */
    if (oal_unlikely(pst_pci_res->pst_pci_dma_ctrl_base == NULL)) {
        pci_print_log(PCI_LOG_ERR, "fifo base addr is null!");
        pst_pci_res->stat.done_err_cnt++;
        return -OAL_ENODEV;
    }

    if (oal_unlikely(pst_pci_res->link_state < PCI_WLAN_LINK_MEM_UP)) {
        pci_print_log(PCI_LOG_ERR, "d2h int link invaild, link_state:%s",
                      oal_pcie_get_link_state_str((pst_pci_res->link_state)));
        pst_pci_res->stat.done_err_cnt++;
        /* linkdown, can't clear intx, disable intx */
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_transfer_check_linkdown(oal_pcie_res *pst_pci_res,
                                                    uint32_t as_dword)
{
    /* Link down check */
    if (oal_unlikely(as_dword == 0xFFFFFFFF)) {
        if (oal_pcie_check_link_state(pst_pci_res) == OAL_FALSE) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}

int32_t oal_pcie_transfer_done(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    int32_t trans_cnt, old_cnt;
    msg_fifo_stat msg_fifo;

    /* 这里的mask 只是mask 状态位，并不是mask中断， 这里的mask只用来标记是否处理这个中断 */
    host_intr_status stat, mask;

    ret = oal_pcie_transfer_done_input_check(pst_pci_res);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }

    mask.as_dword = 0xc;

    trans_cnt = 0;
    old_cnt = trans_cnt;

    do {
        if (oal_unlikely(pst_pci_res->link_state < PCI_WLAN_LINK_MEM_UP)) {
            pci_print_log(PCI_LOG_INFO, "link state is disabled:%s, intx can't process!",
                          oal_pcie_get_link_state_str(pst_pci_res->link_state));
            declare_dft_trace_key_info("pcie mem is not init", OAL_DFT_TRACE_OTHER);
        }

        oal_pcie_mips_start(PCIE_MIPS_RX_INTR_PROCESS);
        stat.as_dword = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_HOST_INTR_STATUS_OFF);
        pci_print_log(PCI_LOG_DBG, "intr status host:0x%8x", stat.as_dword);

        /* check pcie linkdown */
        if (oal_unlikely(oal_pcie_transfer_check_linkdown(pst_pci_res, stat.as_dword) == OAL_TRUE)) {
            pci_print_log(PCI_LOG_ERR, "read transfer done int failed, linkdown..");
            return -OAL_EBUSY;
        }

        stat.as_dword &= mask.as_dword; /* Get mask int */

        if (stat.as_dword == 0) {
            oal_pcie_mips_end(PCIE_MIPS_RX_INTR_PROCESS);
            break;
        }

        pci_print_log(PCI_LOG_DBG, "clear host:0x%8x", stat.as_dword);
        oal_writel(stat.as_dword, (pst_pci_res->pst_pci_ctrl_base + PCIE_HOST_INTR_CLR));
        if (g_pcie_ringbuf_bugfix_enable) {
            /* force read, util the clear intr effect */
            oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_HOST_DEVICE_REG0);
        }
        oal_pcie_mips_end(PCIE_MIPS_RX_INTR_PROCESS);

        /* 读空Hardware FIFO */
        forever_loop() {
            old_cnt = trans_cnt;
            oal_pcie_mips_start(PCIE_MIPS_RX_FIFO_STATUS);
            msg_fifo.as_dword = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_MSG_FIFO_STATUS);
            oal_pcie_mips_end(PCIE_MIPS_RX_FIFO_STATUS);
            pci_print_log(PCI_LOG_DBG, "msg_fifo host:0x%8x", msg_fifo.as_dword);

            if (oal_unlikely(oal_pcie_transfer_check_linkdown(pst_pci_res, msg_fifo.as_dword) == OAL_TRUE)) {
                pci_print_log(PCI_LOG_ERR, "read message fifo stat failed, linkdown..");
                return -OAL_EBUSY;
            }

            if (msg_fifo.bits.rx_msg_fifo0_empty == 0 && msg_fifo.bits.rx_msg_fifo1_empty == 0) {
                oal_pcie_d2h_transfer_done(pst_pci_res);
                trans_cnt++;
            }

            if (msg_fifo.bits.tx_msg_fifo0_empty == 0 && msg_fifo.bits.tx_msg_fifo1_empty == 0) {
                oal_pcie_h2d_transfer_done(pst_pci_res);
                trans_cnt++;
            }

            if (old_cnt == trans_cnt) {
                break;
            }
        }
    } while (1);

    pci_print_log(PCI_LOG_DBG, "trans done process %u cnt data", trans_cnt);

    /* 相等说明已经读空 */
    return !(old_cnt == trans_cnt);
}

int32_t oal_pcie_edma_sleep_request_host_check(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    uint32_t len = 0;
    uint32_t total_len = 0;

    /* 此时allow sleep 应该tx也被释放 */
    for (i = 0; i < PCIE_H2D_QTYPE_BUTT; i++) {
        len = oal_netbuf_list_len(&pst_pci_res->st_tx_res[i].txq);
        if (len) {
            pci_print_log(PCI_LOG_INFO, "txq:%d still had skb len:%u", i, len);
        }
        total_len += len;
    }

    return (total_len != 0) ? -OAL_EBUSY : OAL_SUCC;
}

static int32_t pcie_send_check_param(oal_pcie_res *pst_pci_res, oal_netbuf_head_stru *pst_head)
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

OAL_STATIC OAL_INLINE void oal_pcie_build_netbuf_ringbuf(oal_netbuf_stru *netbuf,
                                                         pcie_read_ringbuf_item *item,
                                                         dma_addr_t pci_dma_addr)
{
    int32_t ret;
    pcie_cb_dma_res st_cb_dma;
    /* 64bits 传输, 不考虑大小端 */
    item->buff_paddr.addr = (uint64_t)pci_dma_addr;

    /* 这里的长度包含64B的头 */
    if (oal_likely(oal_netbuf_len(netbuf) >= HCC_HDR_TOTAL_LEN)) {
        /* tx ringbuf中的长度不包含头,就算包含也只是多传输一个头的长度 */
        /* 计算4字节对齐后的长度，默认进位 */
        item->buf_len = padding_m((oal_netbuf_len(netbuf) - HCC_HDR_TOTAL_LEN), 4);
    } else {
        pci_print_log(PCI_LOG_WARN, "tx netbuf too short %u < %u\n",
                      oal_netbuf_len(netbuf), HCC_HDR_TOTAL_LEN);
        declare_dft_trace_key_info("tx netbuf too short", OAL_DFT_TRACE_FAIL);
    }

    item->reserved0 = 0x4321;

    st_cb_dma.paddr.addr = (uint64_t)pci_dma_addr;
    st_cb_dma.len = oal_netbuf_len(netbuf);

    /* dma地址和长度存在CB字段中，发送完成后释放DMA地址 */
    ret = memcpy_s((uint8_t *)oal_netbuf_cb(netbuf) + sizeof(struct hcc_tx_cb_stru),
                   oal_netbuf_cb_size() - sizeof(struct hcc_tx_cb_stru), &st_cb_dma, sizeof(st_cb_dma));
    if (oal_unlikely(ret != EOK)) {
        pci_print_log(PCI_LOG_ERR, "dma addr copy failed");
    }
}

int32_t oal_pcie_send_netbuf_list(oal_pcie_res *pci_res, oal_netbuf_head_stru *head,
                                  pcie_h2d_ringbuf_qtype qtype)
{
    unsigned long flags;
    oal_pcie_linux_res *pst_pci_lres = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;
    oal_pci_dev_stru *pst_pci_dev = NULL;
    dma_addr_t pci_dma_addr;
    pcie_read_ringbuf_item st_item;
    int32_t send_cnt, queue_cnt, total_cnt;
    const uint32_t ul_max_dump_bytes = 128;
    int32_t ret;

    ret = pcie_send_check_param(pci_res, head);
    if (ret <= 0) {
        return ret;
    }

    pst_pci_dev = pcie_res_to_dev(pci_res);
    pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pci_dev);
    if (oal_unlikely(OAL_SUCC != hcc_bus_pm_wakeup_device(pst_pci_lres->pst_bus))) {
        oal_msleep(100); /* wait for a while retry */
        return -OAL_EIO;
    }

    if (oal_unlikely(pci_res->link_state < PCI_WLAN_LINK_WORK_UP)) {
        pci_print_log(PCI_LOG_WARN, "send netbuf link invaild, link_state:%s",
                      oal_pcie_get_link_state_str(pci_res->link_state));
        return -OAL_ENODEV;
    }

    queue_cnt = oal_netbuf_list_len(head);
    send_cnt = oal_pcie_h2d_ringbuf_freecount(pci_res, qtype, OAL_FALSE);
    if (queue_cnt > send_cnt) {
        /* ringbuf 空间不够, 刷新rd指针，重新判断 */
        send_cnt = oal_pcie_h2d_ringbuf_freecount(pci_res, qtype, OAL_TRUE);
    }

    if (send_cnt == 0) {
        /* ringbuf 为满 */
        return 0;
    }

    pci_print_log(PCI_LOG_DBG, "[q:%d]h2d_ringbuf freecount:%u, qlen:%u", (int32_t)qtype, send_cnt, queue_cnt);

    total_cnt = 0;

    forever_loop() {
        /* 填充ringbuf */
        if (oal_pcie_h2d_ringbuf_freecount(pci_res, qtype, OAL_FALSE) == 0) {
            break;
        }

        /* 取netbuf */
        pst_netbuf = oal_netbuf_delist(head);
        if (pst_netbuf == NULL) {
            break;
        }

        /* Debug */
        if (oal_unlikely(pci_res->link_state < PCI_WLAN_LINK_WORK_UP)) {
            pci_print_log(PCI_LOG_WARN, "oal_pcie_send_netbuf_list loop invaild, link_state:%s",
                          oal_pcie_get_link_state_str(pci_res->link_state));
            hcc_tx_netbuf_free(pst_netbuf);
            return -OAL_ENODEV;
        }

        pci_dma_addr = dma_map_single(&pst_pci_dev->dev, oal_netbuf_data(pst_netbuf),
                                      oal_netbuf_len(pst_netbuf), PCI_DMA_TODEVICE);
        if (dma_mapping_error(&pst_pci_dev->dev, pci_dma_addr)) {
            /* 映射失败先简单处理丢掉netbuf, dma mask 范围内 这里只是刷Cache */
            declare_dft_trace_key_info("pcie tx map failed", OAL_DFT_TRACE_OTHER);
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "edma tx map failed, va=%p, len=%d", oal_netbuf_data(pst_netbuf),
                                 oal_netbuf_len(pst_netbuf));
            hcc_tx_netbuf_free(pst_netbuf);
            continue;
        }

        pci_print_log(PCI_LOG_DBG, "send netbuf 0x%p, dma pa:0x%llx", pst_netbuf, (uint64_t)pci_dma_addr);
        if (pci_dbg_condtion()) {
            oal_print_hex_dump(oal_netbuf_data(pst_netbuf),
                               oal_netbuf_len(pst_netbuf) <
                               ul_max_dump_bytes
                               ? oal_netbuf_len(pst_netbuf)
                               : ul_max_dump_bytes,
                               HEX_DUMP_GROUP_SIZE, "netbuf: ");
        }

        oal_pcie_build_netbuf_ringbuf(pst_netbuf, &st_item, pci_dma_addr);

        /* netbuf入队 */
        oal_spin_lock_irq_save(&pci_res->st_tx_res[qtype].lock, &flags);
        oal_netbuf_list_tail_nolock(&pci_res->st_tx_res[qtype].txq, pst_netbuf);
        oal_spin_unlock_irq_restore(&pci_res->st_tx_res[qtype].lock, &flags);

        pci_print_log(PCI_LOG_DBG, "h2d ringbuf write 0x%llx, len:%u", st_item.buff_paddr.addr, st_item.buf_len);

        /* 这里直接写，上面已经判断过ringbuf有空间 */
        total_cnt += oal_pcie_h2d_ringbuf_write(pci_res,
                                                &pci_res->st_tx_res[qtype].ringbuf_data_dma_addr,
                                                qtype, &st_item);
    }

    if (oal_likely(total_cnt)) {
        /* 更新device侧wr指针,刷ringbuf cache */
        oal_pcie_h2d_ringbuf_wr_update(pci_res, qtype);

        /* tx doorbell */
        oal_pcie_h2d_doorbell(pci_res);
    } else {
        declare_dft_trace_key_info("pcie send toal cnt error", OAL_DFT_TRACE_FAIL);
    }

    return total_cnt;
}

/* 原生dma rx完成MSI中断 */
OAL_STATIC irqreturn_t oal_pcie_edma_rx_intr_status_handler(int irq, void *dev_id)
{
    /* log for msi bbit, del later */
    oal_io_print("pcie_edma_rx_intr_status come, irq:%d\n", irq);
    return IRQ_HANDLED;
}

OAL_STATIC irqreturn_t oal_pcie_edma_tx_intr_status_handler(int irq, void *dev_id)
{
    oal_io_print("pcie_edma_tx_intr_status come, irq:%d\n", irq);
    return IRQ_HANDLED;
}

OAL_STATIC void oal_pcie_hw_edma_intr_status_handler(oal_pcie_res *pst_pci_res)
{
    int32_t trans_cnt = 0;
    int32_t old_cnt;
    msg_fifo_stat msg_fifo;

    forever_loop() {
        if (oal_unlikely(pst_pci_res->link_state < PCI_WLAN_LINK_MEM_UP)) {
            pci_print_log(PCI_LOG_INFO, "link state is disabled:%d, msi can't process!", pst_pci_res->link_state);
            declare_dft_trace_key_info("pcie mem is not init", OAL_DFT_TRACE_OTHER);
        }

        old_cnt = trans_cnt;
        oal_pcie_mips_start(PCIE_MIPS_RX_FIFO_STATUS);
        msg_fifo.as_dword = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_MSG_FIFO_STATUS);
        oal_pcie_mips_end(PCIE_MIPS_RX_FIFO_STATUS);
        pci_print_log(PCI_LOG_DBG, "msg_fifo host:0x%8x", msg_fifo.as_dword);

        if (msg_fifo.bits.rx_msg_fifo0_empty == 0 && msg_fifo.bits.rx_msg_fifo1_empty == 0) {
            oal_pcie_d2h_transfer_done(pst_pci_res);
            trans_cnt++;
        }

        if (msg_fifo.bits.tx_msg_fifo0_empty == 0 && msg_fifo.bits.tx_msg_fifo1_empty == 0) {
            oal_pcie_h2d_transfer_done(pst_pci_res);
            trans_cnt++;
        }

        if (old_cnt == trans_cnt) {
            break;
        }
    }
}

OAL_STATIC irqreturn_t oal_pcie_hw_edma_rx_intr_status_handler(int irq, void *dev_id)
{
    oal_pcie_res *pst_pci_res = (oal_pcie_res *)dev_id;
    pci_print_log(PCI_LOG_DBG, "oal_pcie_hw_edma_rx_intr_status_handler come, irq:%d\n", irq);
    oal_pcie_hw_edma_intr_status_handler(pst_pci_res);
    return IRQ_HANDLED;
}

OAL_STATIC irqreturn_t oal_pcie_hw_edma_tx_intr_status_handler(int irq, void *dev_id)
{
    oal_pcie_res *pst_pci_res = (oal_pcie_res *)dev_id;
    pci_print_log(PCI_LOG_DBG, "oal_pcie_hw_edma_tx_intr_status_handler come, irq:%d\n", irq);
    oal_pcie_hw_edma_intr_status_handler(pst_pci_res);
    return IRQ_HANDLED;
}

irqreturn_t oal_device2host_init_handler(int irq, void *dev_id)
{
    oal_io_print("oal_device2host_init_handler come, irq:%d\n", irq);
    return IRQ_HANDLED;
}

OAL_STATIC irqreturn_t oal_pcie_msg_irq_handler(int irq, void *dev_id)
{
    oal_io_print("oal_pcie_msg_irq_handler come, irq:%d\n", irq);
    return IRQ_HANDLED;
}

/* msi int callback, should move to oal_pcie_host.c */
OAL_STATIC oal_irq_handler_t g_msi_110x_callback[] = {
    oal_pcie_edma_rx_intr_status_handler,
    oal_pcie_edma_tx_intr_status_handler,
    oal_pcie_hw_edma_rx_intr_status_handler,
    oal_pcie_hw_edma_tx_intr_status_handler,
    oal_device2host_init_handler,
    oal_pcie_msg_irq_handler,
};

void oal_pcie_edma_task_exit(oal_pcie_res *pst_pcie_res)
{
    if (pst_pcie_res->chip_info.edma_support != OAL_TRUE) {
        return;
    }

    if (pst_pcie_res->pst_rx_normal_task != NULL) {
        oal_thread_stop(pst_pcie_res->pst_rx_normal_task, NULL);
    }

    if (pst_pcie_res->pst_rx_hi_task != NULL) {
        oal_thread_stop(pst_pcie_res->pst_rx_hi_task, NULL);
    }
}

int32_t oal_pcie_edma_task_init(oal_pcie_res *pst_pcie_res, oal_pcie_msi_stru *msi)
{
    if (pst_pcie_res->chip_info.edma_support != OAL_TRUE) {
        return OAL_SUCC;
    }
    oal_wait_queue_init_head(&pst_pcie_res->st_rx_hi_wq);
    oal_wait_queue_init_head(&pst_pcie_res->st_rx_normal_wq);

    oal_atomic_set(&pst_pcie_res->rx_hi_cond, 0);
    oal_atomic_set(&pst_pcie_res->rx_normal_cond, 0);

    /* 高优先级内存用于补充内存 低耗时 */
    pst_pcie_res->pst_rx_hi_task = oal_thread_create(oal_pcie_rx_hi_thread,
                                                     (void *)pst_pcie_res,
                                                     NULL,
                                                     "pci_rx_hi_task",
                                                     SCHED_RR,
                                                     OAL_BUS_TEST_INIT_PRIORITY,
                                                     -1);
    if (pst_pcie_res->pst_rx_hi_task == NULL) {
        pci_print_log(PCI_LOG_ERR, "pcie rx hi task create failed!");
        return -OAL_EFAIL;
    }

    /* 低优先级线程用于补充内存  高耗时，申请不到就轮询 */
    pst_pcie_res->pst_rx_normal_task = oal_thread_create(oal_pcie_rx_normal_thread,
                                                         (void *)pst_pcie_res,
                                                         NULL,
                                                         "pci_rx_normal_task",
                                                         SCHED_NORMAL,
                                                         MIN_NICE,
                                                         -1);
    if (pst_pcie_res->pst_rx_normal_task == NULL) {
        pci_print_log(PCI_LOG_ERR, "pcie rx normal task create failed!");
        oal_pcie_edma_task_exit(pst_pcie_res);
        return -OAL_EFAIL;
    }

    msi->func = g_msi_110x_callback;
    msi->msi_num = (int32_t)(sizeof(g_msi_110x_callback) / sizeof(oal_irq_handler_t));

    return OAL_SUCC;
}

/* 释放RX通路的资源 */
OAL_STATIC void oal_pcie_rx_res_clean(oal_pcie_res *pst_pci_res)
{
    unsigned long flags;
    oal_netbuf_stru *pst_netbuf = NULL;

    /* 释放RX补充队列 */
    pci_print_log(PCI_LOG_INFO, "prepare free rxq len=%d", oal_netbuf_list_len(&pst_pci_res->st_rx_res.rxq));
    forever_loop() {
        oal_spin_lock_irq_save(&pst_pci_res->st_rx_res.lock, &flags);
        pst_netbuf = oal_netbuf_delist_nolock(&pst_pci_res->st_rx_res.rxq);
        oal_spin_unlock_irq_restore(&pst_pci_res->st_rx_res.lock, &flags);

        if (pst_netbuf == NULL) {
            break;
        }

        oal_pcie_release_rx_netbuf(pst_pci_res, pst_netbuf);
    }
}

/* 释放TX通路的资源 */
OAL_STATIC void oal_pcie_tx_res_clean(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    unsigned long flags;
    oal_netbuf_stru *pst_netbuf = NULL;

    /* 释放待TX发送队列, Ringbuf 在DEV侧 直接下电 */
    for (i = 0; i < PCIE_H2D_QTYPE_BUTT; i++) {
        pci_print_log(PCI_LOG_INFO, "prepare free txq[%d] len=%d",
                      i, oal_netbuf_list_len(&pst_pci_res->st_tx_res[i].txq));
        forever_loop() {
            oal_spin_lock_irq_save(&pst_pci_res->st_tx_res[i].lock, &flags);
            pst_netbuf = oal_netbuf_delist_nolock(&pst_pci_res->st_tx_res[i].txq);
            oal_spin_unlock_irq_restore(&pst_pci_res->st_tx_res[i].lock, &flags);

            if (pst_netbuf == NULL) {
                break;
            }

            oal_pcie_tx_netbuf_free(pst_pci_res, pst_netbuf);
        }
        oal_atomic_set(&pst_pci_res->st_tx_res[i].tx_ringbuf_sync_cond, 0);
    }
}

int32_t oal_pcie_transfer_edma_res_init(oal_pcie_res *pst_pci_res)
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

    oal_pcie_rx_res_clean(pst_pci_res);
    oal_pcie_tx_res_clean(pst_pci_res);

    ret = oal_pcie_rx_ringbuf_build(pst_pci_res);
    if (ret != OAL_SUCC) {
        return ret;
    }

    mutex_lock(&pst_pci_res->st_rx_mem_lock);
    oal_pcie_change_link_state(pst_pci_res, PCI_WLAN_LINK_RES_UP);
    mutex_unlock(&pst_pci_res->st_rx_mem_lock);

    return ret;
}


void oal_pcie_transfer_edma_res_exit(oal_pcie_res *pst_pci_res)
{
    oal_pcie_change_link_state(pst_pci_res, PCI_WLAN_LINK_DOWN);

    oal_pcie_rx_res_clean(pst_pci_res);
    oal_pcie_tx_res_clean(pst_pci_res);

    oal_pcie_ringbuf_res_unmap(pst_pci_res);

    oal_pcie_share_mem_res_unmap(pst_pci_res);
}

#endif

