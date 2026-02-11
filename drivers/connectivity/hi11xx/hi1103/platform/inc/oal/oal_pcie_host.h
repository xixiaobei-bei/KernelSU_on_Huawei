

#ifndef __OAL_PCIE_HOST_H__
#define __OAL_PCIE_HOST_H__

#include "oal_util.h"
#include "oal_hardware.h"
#include "oal_pcie_comm.h"
#include "oal_pci_if.h"
#include "oal_hcc_bus.h"
#include "securec.h"

#undef CONFIG_PCIE_MEM_WR_CACHE_ENABLE

#define PCIE_INVALID_VALUE (~0x0)

#define PCIE_DEV_SHARE_MEM_CPU_ADDRESS ((PCIE_CTRL_BASE_ADDR + PCIE_HOST_DEVICE_REG1))

#define PCIE_RX_RINGBUF_SUPPLY_ALL 0xFFFFFFFFUL

/* 64bit bar, bar1 reference to  bar 2 */
#define PCIE_IATU_BAR_INDEX OAL_PCI_BAR_2

#define PCIE_MEM_MSG_SIZE 2 /* 存放每段mem的开始和结束地址信息 */

#define PCIE_DEBUG_MSG_LEN 100
typedef enum _PCI_LOG_TYPE_ {
    PCI_LOG_ERR,
    PCI_LOG_WARN,
    PCI_LOG_INFO,
    PCI_LOG_DBG,
    PCI_LOG_BUTT
} PCI_LOG_TYPE;

/*
 * when deepsleep not S/R, pcie is PCI_WLAN_LINK_UP,
 * when deepsleep under S/R, pcie is PCI_WLAN_LINK_DOWN,
 * when down firmware , pcie is PCI_WLAN_LINK_MEM_UP,
 * after wcpu main func up device ready, pcie is PCI_WLAN_LINK_DMA_UP,
 * we can't access pcie ep's AXI interface when it's  power down,
 * cased host bus error
 */
typedef enum _PCI_WLAN_LINK_STATE_ {
    PCI_WLAN_LINK_DOWN = 0,  /* default state, PCIe not ready */
    PCI_WLAN_LINK_DEEPSLEEP, /* pcie linkdown, but soc sleep mode */
    PCI_WLAN_LINK_UP,        /* 物理链路已使能 */
    PCI_WLAN_LINK_MEM_UP,    /* IATU已经配置OK，可以访问AXI */
    PCI_WLAN_LINK_RES_UP,    /* RINGBUF OK */
    PCI_WLAN_LINK_WORK_UP,   /* 业务层可以访问PCIE */
    PCI_WLAN_LINK_BUTT
} PCI_WLAN_LINK_STATE;

#define PCIE_GEN1 0x0
#define PCIE_GEN2 0x1

extern char *pcie_link_state_str[PCI_WLAN_LINK_BUTT + 1];
extern oal_int32 hipcie_loglevel;
extern char *pci_loglevel_format[];
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
extern oal_int32 pcie_memcopy_type;
#endif

#define PCI_DBG_CONDTION() (OAL_UNLIKELY(hipcie_loglevel >= PCI_LOG_DBG))
OAL_STATIC OAL_INLINE oal_void oal_pcie_log_record(PCI_LOG_TYPE type)
{
    if (OAL_UNLIKELY(type <= PCI_LOG_WARN)) {
        if (type == PCI_LOG_ERR) {
            DECLARE_DFT_TRACE_KEY_INFO("pcie error happend", OAL_DFT_TRACE_OTHER);
        }
        if (type == PCI_LOG_WARN) {
            DECLARE_DFT_TRACE_KEY_INFO("pcie warn happend", OAL_DFT_TRACE_OTHER);
        }
    }
}
#ifdef CONFIG_PRINTK
#define PCI_PRINT_LOG(loglevel, fmt, arg...)                                                                    \
    do {                                                                                                        \
        if (OAL_UNLIKELY(hipcie_loglevel >= loglevel)) {                                                        \
            printk("%s" fmt "[%s:%d]\n", pci_loglevel_format[loglevel] ? : "", ##arg, __FUNCTION__, __LINE__); \
            oal_pcie_log_record(loglevel);                                                                      \
        }                                                                                                       \
    } while (0)
#else
#define PCI_PRINT_LOG
#endif

OAL_STATIC OAL_INLINE oal_void oal_pcie_print_config_reg(oal_pci_dev_stru *dev, oal_int32 reg_name, char *name)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_int32 ret_t;
    oal_uint32 reg_t = 0;
    ret_t = oal_pci_read_config_dword(dev, reg_name, &reg_t);
    if (!ret_t) {
        OAL_IO_PRINT(" [0x%8x:0x%8x]\n", reg_name, reg_t);
    } else {
        OAL_IO_PRINT("read %s register failed, ret=%d\n", name, ret_t);
    }
#endif
}

#define PRINT_PCIE_CONFIG_REG(dev, reg_name)                 \
    do {                                                     \
        oal_pcie_print_config_reg(dev, reg_name, #reg_name); \
    } while (0)

typedef oal_uint32 pcie_dev_ptr; /* Device CPU 指针大小，目前都是32bits */

typedef struct _oal_reg_bits_stru_ {
    oal_uint32 flag;
    oal_uint32 value;
    char *name;
} oal_reg_bits_stru;

typedef struct _oal_pcie_msi_stru_ {
    oal_int32 is_msi_support;
    oal_irq_handler_t *func; /* msi interrupt map */
    oal_int32 msi_num;       /* msi number */
} oal_pcie_msi_stru;

typedef struct _oal_pcie_h2d_stat_ {
    oal_uint32 tx_count;
    oal_uint32 tx_done_count;
    oal_uint32 tx_burst_cnt[PCIE_EDMA_READ_BUSRT_COUNT + 1];
} oal_pcie_h2d_stat;

typedef struct _oal_pcie_d2h_stat_ {
    oal_uint32 rx_count;
    oal_uint32 rx_done_count;
    oal_uint32 rx_burst_cnt[PCIE_EDMA_WRITE_BUSRT_COUNT + 1];
    oal_uint32 alloc_netbuf_failed;
    oal_uint32 map_netbuf_failed;
} oal_pcie_d2h_stat;

typedef struct _oal_pcie_trans_stat_ {
    oal_uint32 intx_total_count;
    oal_uint32 intx_tx_count;
    oal_uint32 intx_rx_count;
    oal_uint32 done_err_cnt;
    oal_uint32 h2d_doorbell_cnt;
    oal_uint32 d2h_doorbell_cnt;
} oal_pcie_trans_stat;

typedef struct _pci_addr_map__ {
    /* unsigned long 指针长度和CPU位宽等长 */
    uintptr_t va; /* 虚拟地址 */
    uintptr_t pa; /* 物理地址 */
} pci_addr_map;

typedef struct _pcie_cb_dma_res_ {
    edma_paddr_t paddr;
    oal_uint32 len;
} pcie_cb_dma_res;

typedef struct _pcie_h2d_res_ {
    /* device ringbuf 虚拟地址(数据) */
    pci_addr_map ringbuf_data_dma_addr; /* ringbuf buf地址 */
    pci_addr_map ringbuf_ctrl_dma_addr; /* ringbuf 控制结构体地址 */
    oal_netbuf_head_stru txq;           /* 正在发送中的netbuf队列 */
    oal_atomic tx_ringbuf_sync_cond;
    oal_spin_lock_stru lock;
    oal_pcie_h2d_stat stat;
} pcie_h2d_res;

typedef struct _pcie_d2h_res_ {
    /* device ringbuf 虚拟地址(数据) */
    pci_addr_map ringbuf_data_dma_addr; /* ringbuf buf地址 */
    pci_addr_map ringbuf_ctrl_dma_addr; /* ringbuf 控制结构体地址 */
    oal_netbuf_head_stru rxq;           /* 正在接收中的netbuf队列 */
    oal_spin_lock_stru lock;
    oal_pcie_d2h_stat stat;
} pcie_d2h_res;

typedef struct _pcie_h2d_message_res_ {
    pci_addr_map ringbuf_data_dma_addr; /* ringbuf buf地址 */
    pci_addr_map ringbuf_ctrl_dma_addr; /* ringbuf 控制结构体地址 */
    oal_spin_lock_stru lock;
} pcie_h2d_message_res;

typedef struct _pcie_d2h_message_res_ {
    pci_addr_map ringbuf_data_dma_addr; /* ringbuf buf地址 */
    pci_addr_map ringbuf_ctrl_dma_addr; /* ringbuf 控制结构体地址 */
    oal_spin_lock_stru lock;
} pcie_d2h_message_res;

typedef struct _pcie_message_res_ {
    pcie_h2d_message_res h2d_res;
    pcie_d2h_message_res d2h_res;
} pcie_message_res;

typedef struct _pcie_comm_rb_ctrl_res_ {
    pci_addr_map data_daddr; /* ringbuf buf地址 */
    pci_addr_map ctrl_daddr; /* ringbuf 控制结构体地址 */
    oal_spin_lock_stru lock;
} pcie_comm_rb_ctrl_res;

typedef struct _pcie_comm_ringbuf_res_ {
    pcie_comm_rb_ctrl_res comm_rb_res[PCIE_COMM_RINGBUF_BUTT];
} pcie_comm_ringbuf_res;

typedef struct _oal_pcie_bar_info_ {
    oal_uint8 bar_idx;
    oal_uint64 start; /* PCIe在Host分配到的总物理地址大小 */
    oal_uint64 end;

    /* PCIe 发出的总线地址空间， 和start 有可能一样，
      有可能不一样，这个值是配置到BAR 和iatu 的 SRC 地址 */
    oal_uint64 bus_start;

    oal_uint32 size;
} oal_pcie_bar_info;

#define OAL_PCIE_TO_NAME(name) #name
typedef struct _oal_pcie_region_ {
    oal_void *vaddr;  /* virtual address after remap */
    oal_uint64 paddr; /* PCIe在Host侧分配到的物理地址 */

    oal_uint64 bus_addr; /* PCIe RC 发出的总线地址 */

    /* pci为PCI看到的地址和CPU看到的地址 每个SOC 大小和地址可能有差异 */
    /* device pci address */
    oal_uint64 pci_start;
    oal_uint64 pci_end;
    /* Device侧CPU看到的地址 */
    oal_uint64 cpu_start;
    oal_uint64 cpu_end;
    oal_uint32 size;

    oal_uint32 flag; /* I/O type,是否需要刷Cache */

    oal_resource *res;
    char *name; /* resource name */

    oal_pcie_bar_info *bar_info; /* iatu 对应的bar信息 */
} oal_pcie_region;

/* IATU BAR by PCIe mem package */
typedef struct _oal_pcie_iatu_bar_ {
    oal_pcie_region st_region;
    oal_pcie_bar_info st_bar_info;
} oal_pcie_iatu_bar;

typedef struct _oal_pcie_regions_ {
    oal_pcie_region *pst_regions;
    oal_int32 region_nums; /* region nums */

    oal_pcie_bar_info *pst_bars;
    oal_int32 bar_nums;

    oal_int32 inited; /* 非0表示初始化过 */
} oal_pcie_regions;

typedef struct _oal_pcie_res__ {
    oal_void *data;      /* callback pointer */
    oal_uint32 revision; /* ip version */

    PCI_WLAN_LINK_STATE link_state;

    pci_addr_map dev_share_mem; /* Device share mem 管理结构体地址 */

    /* ringbuf 管理结构体,Host存放一份是因为PCIE访问效率没有DDR直接访问高 */
    pcie_ringbuf_res st_ringbuf;
    pci_addr_map st_ringbuf_map; /* device ringbuf在host侧的地址映射 */

    pci_addr_map st_device_stat_map;
    pcie_stats st_device_stat;

    pci_addr_map st_device_shared_addr_map[PCIE_SHARED_ADDR_BUTT];

    /* RINGBUFF在Host侧对应的资源 */
    pcie_h2d_res st_tx_res[PCIE_H2D_QTYPE_BUTT];
    pcie_d2h_res st_rx_res;
    pcie_message_res st_message_res; /* Message Ringbuf */
    pcie_comm_ringbuf_res st_ringbuf_res;

    oal_pcie_trans_stat stat;

    /* 根据Soc设计信息表刷新，不同的产品划分不一样, iATU必须对每个region分别映射 */
    oal_pcie_regions regions; /* Device地址划分 */

    /* Bar1 for iatu by mem package */
    oal_pcie_iatu_bar st_iatu_bar;

    /* PCIe Device 寄存器基地址,Host Virtual Address */
    oal_void *pst_pci_dma_ctrl_base;
    oal_void *pst_pci_ctrl_base;
    oal_void *pst_pci_dbi_base;

    /* Rx 补充内存线程 2级线程 高优先级线程实时补充+低优先级补充 */
    struct task_struct *pst_rx_hi_task;
    struct task_struct *pst_rx_normal_task;

    oal_mutex_stru st_rx_mem_lock;
    oal_wait_queue_head_stru st_rx_hi_wq;
    oal_wait_queue_head_stru st_rx_normal_wq;

    oal_atomic rx_hi_cond;
    oal_atomic rx_normal_cond;
} oal_pcie_res;

#define PCIE_RES_TO_DEV(pst_pci_res) (oal_pci_dev_stru *)((pst_pci_res)->data)

typedef enum _PCIE_MIPS_TYPE_ {
    PCIE_MIPS_HCC_RX_TOTAL,
    PCIE_MIPS_RX_FIFO_STATUS,
    PCIE_MIPS_RX_INTR_PROCESS,
    PCIE_MIPS_RX_NETBUF_SUPPLY,
    PCIE_MIPS_RX_NETBUF_SUPPLY_TEST,
    PCIE_MIPS_RX_RINGBUF_RD_UPDATE,
    PCIE_MIPS_RX_RINGBUF_WR_UPDATE,
    PCIE_MIPS_RX_RINGBUF_WRITE,
    PCIE_MIPS_RX_RINGBUF_ENQUEUE,
    PCIE_MIPS_RX_MSG_FIFO,
    PCIE_MIPS_RX_MEM_ALLOC,
    PCIE_MIPS_RX_NETBUF_MAP,
    PCIE_MIPS_RX_QUEUE_POP,
    PCIE_MIPS_RX_MEM_FREE,
    PCIE_MIPS_BUTT
} PCIE_MIPS_TYPE;

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE_PERFORMANCE
oal_void oal_pcie_mips_start(PCIE_MIPS_TYPE type);
oal_void oal_pcie_mips_end(PCIE_MIPS_TYPE type);
oal_void oal_pcie_mips_clear(oal_void);
oal_void oal_pcie_mips_show(oal_void);
#else
OAL_STATIC OAL_INLINE oal_void oal_pcie_mips_start(PCIE_MIPS_TYPE type)
{
    OAL_REFERENCE(type);
}

OAL_STATIC OAL_INLINE oal_void oal_pcie_mips_end(PCIE_MIPS_TYPE type)
{
    OAL_REFERENCE(type);
}

OAL_STATIC OAL_INLINE oal_void oal_pcie_mips_clear(oal_void)
{
}

OAL_STATIC OAL_INLINE oal_void oal_pcie_mips_show(oal_void)
{
}
#endif

oal_int32 oal_pcie_send_netbuf_list(oal_pcie_res *pst_pci_res, oal_netbuf_head_stru *head,
                                    PCIE_H2D_RINGBUF_QTYPE qtype);
oal_pcie_res *oal_pcie_host_init(oal_void *data, oal_pcie_msi_stru *pst_msi, oal_uint32 revision);
oal_void oal_pcie_host_exit(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_dev_init(oal_pcie_res *pst_pci_res);
oal_void oal_pcie_dev_deinit(oal_pcie_res *pst_pci_res);
oal_pcie_res *oal_get_default_pcie_handler(oal_void);

oal_int32 oal_pcie_vaddr_isvalid(oal_pcie_res *pst_pci_res, oal_void *vaddr);
oal_int32 oal_pcie_inbound_ca_to_va(oal_pcie_res *pst_pci_res, oal_uint64 dev_cpuaddr,
                                    pci_addr_map *addr_map);
oal_int32 oal_pcie_get_ca_by_pa(oal_pcie_res *pst_pci_res, oal_ulong paddr, oal_uint64 *cpuaddr);
oal_int32 oal_pcie_transfer_done(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_tx_is_idle(oal_pcie_res *pst_pci_res, PCIE_H2D_RINGBUF_QTYPE qtype);
oal_int32 oal_pcie_read_d2h_message(oal_pcie_res *pst_pci_res, oal_uint32 *message);
oal_int32 oal_pcie_send_message_to_dev(oal_pcie_res *pst_pci_res, oal_uint32 message);
oal_int32 oal_pcie_get_host_trans_count(oal_pcie_res *pst_pci_res, oal_uint64 *tx, oal_uint64 *rx);
oal_int32 oal_pcie_sleep_request_host_check(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_disable_regions(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_enable_regions(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_transfer_res_init(oal_pcie_res *pst_pci_res);
oal_void oal_pcie_transfer_res_exit(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_iatu_init(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_check_link_state(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_set_l1pm_ctrl(oal_pcie_res *pst_pci_res, oal_int32 enable);
oal_int32 oal_pcie_read_dsm32(oal_pcie_res *pst_pci_res, PCIE_SHARED_DEVICE_ADDR_TYPE type, oal_uint32 *val);
oal_int32 oal_pcie_set_device_soft_fifo_enable(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_set_device_dma_check_enable(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_set_device_ringbuf_bugfix_enable(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_device_aspm_init(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_device_auxclk_init(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_copy_from_device_by_dword(oal_pcie_res *pst_pci_res,
                                             oal_void *ddr_address,
                                             oal_ulong start,
                                             oal_uint32 data_size);
oal_int32 oal_pcie_copy_to_device_by_dword(oal_pcie_res *pst_pci_res,
                                           oal_void *ddr_address,
                                           oal_ulong start,
                                           oal_uint32 data_size);
oal_ulong oal_pcie_get_deivce_dtcm_cpuaddr(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_device_changeto_high_cpufreq(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_device_mem_scanall(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_get_gen_mode(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_unmask_device_link_erros(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_check_device_link_errors(oal_pcie_res *pst_pci_res);
oal_void oal_pcie_print_ringbuf_info(pcie_ringbuf *pst_ringbuf, PCI_LOG_TYPE level);
oal_void oal_pcie_set_voltage_bias_param(oal_uint32 phy_0v9_bias, oal_uint32 phy_1v8_bias);
oal_int32 oal_pcie_get_vol_reg_1v8_value(oal_int32 request_vol, oal_uint32 *pst_value);
oal_int32 oal_pcie_get_vol_reg_0v9_value(oal_int32 request_vol, oal_uint32 *pst_value);
oal_int32 oal_pcie_voltage_bias_init(oal_pcie_res *pst_pci_res);
oal_void oal_pcie_print_transfer_info(oal_pcie_res *pst_pci_res, oal_uint64 print_flag);
oal_void oal_pcie_reset_transfer_info(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_ringbuf_h2d_refresh(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_host_pending_signal_check(oal_pcie_res *pst_pci_res);
oal_int32 oal_pcie_host_pending_signal_process(oal_pcie_res *pst_pci_res);

/* Inline functions */
OAL_STATIC OAL_INLINE oal_void oal_pci_cache_flush(oal_pci_dev_stru *hwdev, oal_void *pa, oal_int32 size)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (OAL_LIKELY(size > 0)) {
        dma_sync_single_for_device(&hwdev->dev, (dma_addr_t)pa, (size_t)size, PCI_DMA_TODEVICE);
    }
    {
        OAL_WARN_ON(1);
    }
#endif
}

/*
 * 函 数 名  : oal_pci_cache_flush
 * 功能描述  : 无效化cache
 */
OAL_STATIC OAL_INLINE oal_void oal_pci_cache_inv(oal_pci_dev_stru *hwdev, oal_void *pa, oal_int32 size)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (OAL_LIKELY(size > 0)) {
        dma_sync_single_for_cpu(&hwdev->dev, (dma_addr_t)pa, (size_t)size, PCI_DMA_FROMDEVICE);
    } else {
        OAL_WARN_ON(1);
    }
#endif
}

/* 'offset' is a backplane address */
OAL_STATIC OAL_INLINE oal_void oal_pcie_write_mem8(oal_ulong va, oal_uint8 data)
{
    *(volatile oal_uint8 *)(va) = (oal_uint8)data;
}

OAL_STATIC OAL_INLINE oal_uint8 oal_pcie_read_mem8(oal_ulong va)
{
    volatile oal_uint8 data;

    data = *(volatile oal_uint8 *)(va);

    return data;
}

OAL_STATIC OAL_INLINE oal_void oal_pcie_write_mem32(uintptr_t va, oal_uint32 data)
{
    *(volatile oal_uint32 *)(va) = (oal_uint32)data;
}

OAL_STATIC OAL_INLINE oal_void oal_pcie_write_mem16(uintptr_t va, oal_uint16 data)
{
    *(volatile oal_uint16 *)(va) = (oal_uint16)data;
}

OAL_STATIC OAL_INLINE oal_void oal_pcie_write_mem64(uintptr_t va, oal_uint64 data)
{
    *(volatile oal_uint64 *)(va) = (oal_uint64)data;
}

OAL_STATIC OAL_INLINE oal_uint16 oal_pcie_read_mem16(uintptr_t va)
{
    volatile oal_uint16 data;

    data = *(volatile oal_uint16 *)(va);

    return data;
}

OAL_STATIC OAL_INLINE oal_uint32 oal_pcie_read_mem32(uintptr_t va)
{
    volatile oal_uint32 data;

    data = *(volatile oal_uint32 *)(va);

    return data;
}

OAL_STATIC OAL_INLINE oal_uint64 oal_pcie_read_mem64(uintptr_t va)
{
    volatile oal_uint64 data;

    data = *(volatile oal_uint64 *)(va);

    return data;
}

extern oal_void oal_pcie_io_trans64(oal_void *dst, oal_void *src, oal_int32 size);
extern oal_void oal_pcie_io_trans32(oal_uint32 *dst, oal_uint32 *src, oal_int32 size);
extern oal_int32 pcie_memcopy_type;
/* dst/src 有一端地址在PCIE EP侧，PCIE按burst方式传输 */
OAL_STATIC OAL_INLINE oal_void oal_pcie_io_trans(uintptr_t dst, uintptr_t src, oal_uint32 size)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /* 网络安全函数整改 */
    if ((pcie_memcopy_type == 0) || (pcie_memcopy_type == 1)) {
        if (WARN((dst & 0x3), "invalid dst address 0x%lx", dst) ||
            WARN((src & 0x3), "invalid src address 0x%lx", dst) ||
            WARN((size & 0x3), "invalid size address 0x%lx", dst)) {
            return;
        }
#ifdef CONFIG_64BIT
        oal_pcie_io_trans64((oal_void *)dst, (oal_void *)src, (oal_int32)size);
#else
        oal_pcie_io_trans32((oal_uint32 *)dst, (oal_uint32 *)src, (oal_int32)size);
#endif
    } else if (pcie_memcopy_type == 2) { /* 可以根据用户输入执行不同操作 */
        oal_uint32 i;
        oal_uint32 value;
        /* 最长4字节对齐访问, Test Code 暂时不考虑 单字节 ，双字节 */
        if (WARN((dst & 0x3), "invalid dst address 0x%lx", dst) ||
            WARN((src & 0x3), "invalid src address 0x%lx", dst) ||
            WARN((size & 0x3), "invalid size address 0x%lx", dst)) {
            return;
        }

        for (i = 0; i < size; i += sizeof(oal_uint32)) { /* 每次偏移4字节 */
            value = oal_readl((void *)(src + i));
            oal_writel (value, (void *)(dst + i));
        }
    } else if (pcie_memcopy_type == 3) { /* 可以根据用户输入执行不同操作 */
        if (WARN((dst & 0x3), "invalid dst address 0x%lx", dst) ||
            WARN((src & 0x3), "invalid src address 0x%lx", dst) ||
            WARN((size & 0x3), "invalid size address 0x%lx", dst)) {
            return;
        }

        oal_pcie_io_trans32((oal_uint32 *)dst, (oal_uint32 *)src, (oal_int32)size);
    }
#endif
}

OAL_STATIC OAL_INLINE oal_uint32 pcie_ringbuf_len(pcie_ringbuf *pst_ringbuf)
{
    /* 无符号，已经考虑了翻转 */
    oal_uint32 len = (pst_ringbuf->wr - pst_ringbuf->rd);
    if (len == 0) {
        return 0;
    }
#ifdef _PRE_PLAT_FEATURE_PCIE_DEBUG
    if (len % pst_ringbuf->item_len) {
        OAL_IO_PRINT("pcie_ringbuf_len, size:%u, wr:%u, rd:%u" NEWLINE,
                     pst_ringbuf->size,
                     pst_ringbuf->wr,
                     pst_ringbuf->rd);
    }
#endif
    if (pst_ringbuf->item_mask) {
        /* item len 如果是2的N次幂，则移位 */
        len = len >> pst_ringbuf->item_mask;
    } else {
        len /= pst_ringbuf->item_len;
    }
    return len;
}

/* 打印 */
OAL_STATIC OAL_INLINE oal_void oal_pcie_print_bits(oal_void *data, oal_uint32 size)
{
#ifdef CONFIG_PRINTK
    oal_int32 ret = 0;
    const oal_uint32 buf_len = 32 * 3 + 1; /* 按bit打印，最多打印32bit，每个bit3个字符 */
    oal_uint32 value;
    char buf[buf_len];
    oal_int32 i;
    oal_int32 count = 0;

    if (size == 1) { /* 1表示要打印长度为1字节 */
        value = (oal_uint32) * (oal_uint8 *)data;
        OAL_IO_PRINT("value= 0x%2x, =%u (dec) \n", value, value);
        OAL_IO_PRINT("07 06 05 04 03 02 01 00\n");
    } else if (size == 2) { /* 2表示要打印长度为2字节 */
        value = (oal_uint32) * (oal_uint16 *)data;
        OAL_IO_PRINT("value= 0x%4x, =%u (dec) \n", value, value);
        OAL_IO_PRINT("15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00\n");
    } else if (size == 4) { /* 4表示要打印长度为4字节 */
        value = (oal_uint32) * (oal_uint32 *)data;
        OAL_IO_PRINT("value= 0x%8x, =%u (dec) \n", value, value);
        OAL_IO_PRINT("31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00\n");
    } else {
        /* 传入了错误的参数，不处理返回 */
        return;
    }

    /* 按比特打印 */
    for (i = size * 8 - 1; i >= 0; i--) {
        ret = snprintf_s(buf + count, sizeof(buf) - count, sizeof(buf) - count - 1, "%s",
                         (1u << (oal_uint32)i) & value ? " 1 " : " 0 ");
        if (ret < 0) {
            PCI_PRINT_LOG(PCI_LOG_ERR, "log str format err line[%d]\n", __LINE__);
            return;
        }
        count += ret;
    }
    OAL_IO_PRINT("%s\n", buf);
#else
    OAL_REFERENCE(data);
    OAL_REFERENCE(size);
#endif
}

OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_pcie_rx_netbuf_alloc(oal_uint32 ul_size, oal_int32 gflag)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    return __netdev_alloc_skb(NULL, ul_size, gflag);
#else
    OAL_REFERENCE(gflag);
    return oal_netbuf_alloc(ul_size, 0, 0);
#endif
}

OAL_STATIC OAL_INLINE oal_void oal_pcie_shced_rx_hi_thread(oal_pcie_res *pst_pci_res)
{
    oal_atomic_set(&pst_pci_res->rx_hi_cond, 1);
    OAL_WAIT_QUEUE_WAKE_UP_INTERRUPT(&pst_pci_res->st_rx_hi_wq);
}

OAL_STATIC OAL_INLINE oal_void oal_pcie_shced_rx_normal_thread(oal_pcie_res *pst_pci_res)
{
    oal_atomic_set(&pst_pci_res->rx_normal_cond, 1);
    OAL_WAIT_QUEUE_WAKE_UP_INTERRUPT(&pst_pci_res->st_rx_normal_wq);
}

OAL_STATIC OAL_INLINE oal_void oal_pcie_print_config_reg_bar(oal_pcie_res *pst_pci_res, oal_uint32 offset, char *name)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_uint32 reg;
    reg = oal_readl(pst_pci_res->st_iatu_bar.st_region.vaddr + offset);
    OAL_IO_PRINT("%-50s [0x%8x:0x%8x]\n", name, offset, reg);
#else
    OAL_REFERENCE(pst_pci_res);
    OAL_REFERENCE(offset);
    OAL_REFERENCE(name);
#endif
}

OAL_STATIC OAL_INLINE char *oal_pcie_get_link_state_str(PCI_WLAN_LINK_STATE link_state)
{
    if (OAL_WARN_ON(link_state > PCI_WLAN_LINK_BUTT)) {
        PCI_PRINT_LOG(PCI_LOG_WARN, "invalid link_state:%d", link_state);
        return "overrun";
    }

    if (pcie_link_state_str[link_state] == NULL) {
        return "unkown";
    }

    return pcie_link_state_str[link_state];
}

OAL_STATIC OAL_INLINE oal_void oal_pcie_change_link_state(oal_pcie_res *pst_pci_res, PCI_WLAN_LINK_STATE link_state_new)
{
    if (pst_pci_res->link_state != link_state_new) {
        PCI_PRINT_LOG(PCI_LOG_INFO, "link_state change from %s to %s",
                      oal_pcie_get_link_state_str(pst_pci_res->link_state),
                      oal_pcie_get_link_state_str(link_state_new));
    } else {
        PCI_PRINT_LOG(PCI_LOG_INFO, "link_state still %s", oal_pcie_get_link_state_str(link_state_new));
    }

    pst_pci_res->link_state = link_state_new;
}

#endif
