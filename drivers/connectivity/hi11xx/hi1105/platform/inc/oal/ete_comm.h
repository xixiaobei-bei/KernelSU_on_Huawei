

#ifndef __OAL_ETE_COMM_H__
#define __OAL_ETE_COMM_H__

#include "oal_types.h"
#include "ete_addr.h"
#undef ETE_COMM_DEBUG

#ifndef NEWLINE
#ifdef CONFIG_PRINTK
#define NEWLINE "\n"
#else
#define NEWLINE "\r\n"
#endif
#endif

/* EP slave口访问主芯片DDR按照IATU Outbound映射固定偏移 */
#define HI_PCIE_EP_SLAVE_OFFSET (0x80000000)

#define PCIE_ETE_INVALID_RD_VALUE (0xFFFFFFFF)

#define PCIE_ETE_TRANS_OVERFLOW_TAG            1
#define pcie_ete_trans_overflow_bitmask(value) (((value) >> 10) & 0x1)
#define pcie_ete_trans_value(value)            ((value) & ((1 << 10) - 1))
#define pcie_ete_trans_get_ptr(value, flag)    (((flag) << 10) | pcie_ete_trans_value(value))

/* ETE IP芯片约束, TX SR/DR entry num必须相同(防止丢中断),
 * 不同的chan之间的entry num必须相同(芯片bug) */
#define PCIE_ETE_TX_CHAN0_ENTRY_NUM 8
#define PCIE_ETE_TX_CHAN1_ENTRY_NUM 8
#define PCIE_ETE_TX_CHAN2_ENTRY_NUM 8

#define PCIE_ETE_RX_CHAN0_ENTRY_NUM 8
#define PCIE_ETE_RX_CHAN1_ENTRY_NUM 8

typedef enum _PCIE_HOST_CTL_INTR_ {
    HOST2DEVICE_TX_INTR_MASK,
    HOST2DEVICE_RX_INTR_MASK,
    DEVICE2HOST_TX_INTR_MASK,
    DEVICE2HOST_RX_INTR_MASK,
    DEVICE2HOST_INTR_MASK,
    PCIE_MSG_IRQ_MASK,
    MAC_5G_INTR_MASK,
    MAC_2G_INTR_MASK,
    PHY_5G_INTR_MASK,
    PHY_2G_INTR_MASK,
    PCIE_HOST_CTL_INTR_BUTT /* max support msg count */
} pcie_host_ctl_intr;

typedef union {
    struct {
        uint32_t reserve : 13; /* 0~12 */
        uint32_t dst_intr : 1; /* 13 , send int to device when set 1 */
        uint32_t src_intr : 1; /* 14 , send int to host when set 1 */
        uint32_t gather : 1;   /* 15 */
        uint32_t nbytes : 16;  /* 16~31 */
    } bits;
    uint32_t dword;
} ete_h2d_sr_ctrl;

typedef union {
    struct {
        uint32_t reserve : 13; /* 0~12 */
        uint32_t dst_intr : 1; /* 13 , send int to host when set 1 */
        uint32_t src_intr : 1; /* 14 , send int to device when set 1 */
        uint32_t gather : 1;   /* 15 */
        uint32_t nbytes : 16;  /* 16~31 */
    } bits;
    uint32_t dword;
} ete_d2h_sr_ctrl;

typedef struct _h2d_sr_descr_ {
    uint64_t addr; /* rc address */
    ete_h2d_sr_ctrl ctrl;
    uint32_t reserved;
} h2d_sr_descr; /* set by rc cpu */

typedef struct _h2d_dr_descr_ {
    uint64_t addr; /* ep address */
} h2d_dr_descr;    /* set by ep cpu */

typedef struct _d2h_sr_descr_ {
    uint32_t addr; /* ep address */
    ete_d2h_sr_ctrl ctrl;
} d2h_sr_descr; /* set by ep cpu */

typedef struct _d2h_dr_descr_ {
    uint64_t addr; /* rc address */
} d2h_dr_descr;    /* set by rc cpu */

/* Ring-Buff */
typedef struct _pcie_ete_ringbuf_ {
    /* 32字节，CPU cache line(32位系统为32字节，64位系统为64字节，尽量要能整除) */
    /* 无锁操作，只能一个核写一个核读 */
    uint32_t idx;      /* ring id, used for future */
    uint32_t done_cnt; /* reserv0 */
    uint32_t rd;       /* 消费者, 读指针 */
    uint32_t rflag;    /* rd翻转标记 */
    uint32_t wr;       /* 生产者,写指针 */
    uint32_t wflag;    /* wr翻转标记 */
    uint32_t size;     /* maximum size of items in ring */
    uint32_t items;    /* maximum number of items in ring */
    uint16_t item_len; /* length of each item in the ring */
    uint16_t debug;    /* debug flag */

    /* 兼容64 bit地址，device 用低32bit */
    uint64_t cpu_addr; /* CPU访问地址 virtual address */
    uint64_t io_addr;  /* 外设访问地址 device address(iova) */
} pcie_ete_ringbuf;

int32_t pcie_ete_ringbuf_init(pcie_ete_ringbuf *pst_ringbuf, uint64_t cpu_addr, uint64_t io_addr,
                              uint32_t idx, uint32_t size, uint16_t item_len);
/* producer, move the write ptr, return the write bytes count */
uint32_t pcie_ete_ringbuf_write(pcie_ete_ringbuf *pst_ringbuf, uint8_t *buf);

/* consumer, move the read ptr, return the read bytes count */
uint32_t pcie_ete_ringbuf_read(pcie_ete_ringbuf *pst_ringbuf, uint8_t *buf, uint32_t buf_len);
int32_t pcie_ete_ringbuf_reset(pcie_ete_ringbuf *pst_ringbuf);

uint32_t pcie_ete_get_rx_status(uintptr_t base_address, uint32_t chan_idx);
uint64_t pcie_ete_get_rx_dr_base_addr(uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_get_tx_status(uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_get_rx_sr_base_addr(uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_get_tx_dr_base_addr(uintptr_t base_address, uint32_t chan_idx);
uint64_t pcie_ete_get_tx_sr_base_addr(uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_get_tx_sr_head_ptr(uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_get_tx_dr_head_ptr(uintptr_t base_address, uint32_t chan_idx);
unsigned int pcie_ete_get_tx_done_total_cnt(uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_get_tx_dr_tail_ptr(uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_get_tx_sr_tail_ptr(uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_get_rx_dr_head_ptr(uintptr_t base_address, uint32_t chan_idx);
unsigned int pcie_ete_get_rx_done_total_cnt(uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_get_rx_dr_tail_ptr(uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_get_rx_sr_head_ptr(uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_get_rx_sr_tail_ptr(uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_get_ete_intr_sts(uintptr_t base_address);
uint32_t pcie_ete_get_ete_intr_raw_sts(uintptr_t base_address);
uint32_t pcie_ete_get_ete_ch_dr_empty_intr_sts(uintptr_t base_address);
uint32_t pcie_ete_get_ete_ch_dr_empty_intr_raw_sts(uintptr_t base_address);
uint32_t pcie_ete_get_rx_err_addr(uintptr_t base_address);
uint32_t pcie_ete_get_tx_err_addr(uintptr_t base_address);
uint32_t pcie_ete_get_ete_intr_mask(uintptr_t base_address);
uint32_t pcie_ete_get_ete_ch_dr_empty_intr_mask(uintptr_t base_address);
uint32_t pcie_ete_get_rx_sr_entry_num(uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_get_rx_dr_entry_num(uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_get_tx_sr_entry_num(uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_get_tx_dr_entry_num(uintptr_t base_address, uint32_t chan_idx);

int pcie_ete_tx_sr_hardinit(uintptr_t base_address, uint32_t chan_idx, pcie_ete_ringbuf *pst_ring);
int pcie_ete_rx_dr_hardinit(uintptr_t base_address, uint32_t chan_idx, pcie_ete_ringbuf *pst_ring);
int pcie_ete_tx_dr_hardinit(uintptr_t base_address, uint32_t chan_idx, pcie_ete_ringbuf *pst_ring);
int pcie_ete_rx_sr_hardinit(uintptr_t base_address, uint32_t chan_idx, pcie_ete_ringbuf *pst_ring);
void pcie_ete_ring_update_rx_dr_hptr(pcie_ete_ringbuf *pst_ringbuf, uintptr_t base_address, uint32_t chan_idx);
void pcie_ete_ring_update_tx_sr_hptr(pcie_ete_ringbuf *pst_ringbuf, uintptr_t base_address, uint32_t chan_idx);
void pcie_ete_ring_update_rx_sr_hptr(pcie_ete_ringbuf *pst_ringbuf, uintptr_t base_address, uint32_t chan_idx);
void pcie_ete_ring_update_tx_dr_hptr(pcie_ete_ringbuf *pst_ringbuf, uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_update_ring_rx_donecnt(pcie_ete_ringbuf *pst_ringbuf, uintptr_t base_address, uint32_t chan_idx);
uint32_t pcie_ete_update_ring_tx_donecnt(pcie_ete_ringbuf *pst_ringbuf, uintptr_t base_address, uint32_t chan_idx);
void pcie_ete_rx_chan_clken_enable(uintptr_t base_address, uint32_t chan_idx);
void pcie_ete_tx_chan_clken_enable(uintptr_t base_address, uint32_t chan_idx);
int32_t pcie_ete_clk_en(uintptr_t base_address, uint32_t enable);
void pcie_dual_enable(uintptr_t base_address, uint32_t enable);
void pcie_ete_intr_winit(uintptr_t base_address);

void pcie_ete_print_trans_tx_chan_regs(uintptr_t base, uint32_t i);
void pcie_ete_print_trans_rx_chan_regs(uintptr_t base, uint32_t i);
void oal_ete_print_trans_comm_regs(uintptr_t base);

OAL_STATIC OAL_INLINE int32_t pcie_ete_ringbuf_is_overflow(pcie_ete_ringbuf *pst_ringbuf)
{
    uint32_t flag = pst_ringbuf->rflag ^ pst_ringbuf->wflag;
    return flag & 0x1; /* 0x1 bit0 */
}

OAL_STATIC OAL_INLINE uint32_t pcie_ete_ringbuf_len(pcie_ete_ringbuf *pst_ringbuf)
{
    uint32_t data_cnt, rd, wr;
    uint32_t flag = pcie_ete_ringbuf_is_overflow(pst_ringbuf);

    rd = pst_ringbuf->rd;
    wr = pst_ringbuf->wr;

    if (flag == PCIE_ETE_TRANS_OVERFLOW_TAG) { /* 0 means overflow */
        data_cnt = wr + pst_ringbuf->items - rd;
    } else {
        data_cnt = wr - rd;
    }

    return data_cnt;
}

OAL_STATIC OAL_INLINE uint32_t pcie_ete_ringbuf_is_full(pcie_ete_ringbuf *pst_ringbuf)
{
    uint32_t data_cnt = pcie_ete_ringbuf_len(pst_ringbuf);
    if (data_cnt == pst_ringbuf->items) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

OAL_STATIC OAL_INLINE uint32_t pcie_ete_ringbuf_is_empty(pcie_ete_ringbuf *pst_ringbuf)
{
    uint32_t data_cnt = pcie_ete_ringbuf_len(pst_ringbuf);
    if (data_cnt == 0) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

OAL_STATIC OAL_INLINE uint32_t pcie_ete_ringbuf_freecount(pcie_ete_ringbuf *pst_ringbuf)
{
    uint32_t freecount;
    uint32_t data_cnt = pcie_ete_ringbuf_len(pst_ringbuf);
    if (oal_likely(pst_ringbuf->items >= data_cnt)) {
        freecount = pst_ringbuf->items - data_cnt;
    } else {
        return 0;
    }

    return freecount;
}

#endif

