

#ifndef __PCIE_HOST_H__
#define __PCIE_HOST_H__

#include "oal_util.h"
#include "oal_hardware.h"
#include "pcie_comm.h"
#include "oal_pci_if.h"
#include "oal_hcc_bus.h"
#include "securec.h"
#include "ete_host.h"

#define PCIE_INVALID_VALUE (0xFFFFFFFFUL)

#define PCIE_RX_TRANS_FLAG 0xbeafbeaf

#define PCIE_TRANS_US_OFFSET_BITS 17 /* 推算出的，计算需要 */

#define PCIE_RX_RINGBUF_SUPPLY_ALL 0xFFFFFFFFUL

/* 64bit bar, bar1 reference to  bar 2 */
#define PCIE_IATU_BAR_INDEX OAL_PCI_BAR_2

#define PCIE_MEM_MSG_SIZE 2 /* 存放每段mem的开始和结束地址信息 */

#define PCIE_DEBUG_MSG_LEN 100
typedef enum _pci_log_type_ {
    PCI_LOG_ERR,
    PCI_LOG_WARN,
    PCI_LOG_INFO,
    PCI_LOG_DBG,
    PCI_LOG_BUTT
} pci_log_type;

/*
 * when deepsleep not S/R, pcie is PCI_WLAN_LINK_UP,
 * when deepsleep under S/R, pcie is PCI_WLAN_LINK_DOWN,
 * when down firmware , pcie is PCI_WLAN_LINK_MEM_UP,
 * after wcpu main func up device ready, pcie is PCI_WLAN_LINK_DMA_UP,
 * we can't access pcie ep's AXI interface when it's  power down,
 * cased host bus error
 */
typedef enum _pci_wlan_link_state_ {
    PCI_WLAN_LINK_DOWN = 0,  /* default state, PCIe not ready */
    PCI_WLAN_LINK_DEEPSLEEP, /* pcie linkdown, but soc sleep mode */
    PCI_WLAN_LINK_UP,        /* 物理链路已使能 */
    PCI_WLAN_LINK_MEM_UP,    /* IATU已经配置OK，可以访问AXI */
    PCI_WLAN_LINK_RES_UP,    /* RINGBUF OK */
    PCI_WLAN_LINK_WORK_UP,   /* 业务层可以访问PCIE */
    PCI_WLAN_LINK_BUTT
} pci_wlan_link_state;

typedef struct _pcie_callback_ {
    void (*pf_func)(void *para);
    void *para;
} pcie_callback_stru;

#define PCIE_GEN1 0x0
#define PCIE_GEN2 0x1

extern char *g_pcie_link_state_str[PCI_WLAN_LINK_BUTT + 1];
extern int32_t g_hipcie_loglevel;
extern char *g_pci_loglevel_format[];
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
extern int32_t g_pcie_memcopy_type;
#endif
extern int32_t g_pcie_ringbuf_bugfix_enable;
extern int32_t g_pcie_dma_data_check_enable;
extern int32_t g_pcie_dma_data_rx_hdr_init;

#define pci_dbg_condtion() (oal_unlikely(g_hipcie_loglevel >= PCI_LOG_DBG))
OAL_STATIC OAL_INLINE void oal_pcie_log_record(pci_log_type type)
{
    if (oal_unlikely(type <= PCI_LOG_WARN)) {
        if (type == PCI_LOG_ERR) {
            declare_dft_trace_key_info("pcie error happend", OAL_DFT_TRACE_OTHER);
        }
        if (type == PCI_LOG_WARN) {
            declare_dft_trace_key_info("pcie warn happend", OAL_DFT_TRACE_OTHER);
        }
    }
}
#ifdef CONFIG_PRINTK
#define pci_print_log(loglevel, fmt, arg...)                                                                    \
    do {                                                                                                        \
        if (oal_unlikely(g_hipcie_loglevel >= loglevel)) {                                                        \
            printk("%s" fmt "[%s:%d]\n", g_pci_loglevel_format[loglevel] ? : "", ##arg, __FUNCTION__, __LINE__); \
            oal_pcie_log_record(loglevel);                                                                      \
        }                                                                                                       \
    } while (0)
#else
#define pci_print_log
#endif

OAL_STATIC OAL_INLINE void oal_pcie_print_config_reg(oal_pci_dev_stru *dev, int32_t reg_name, char *name)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    int32_t ret_t;
    uint32_t reg_t = 0;
    ret_t = oal_pci_read_config_dword(dev, reg_name, &reg_t);
    if (!ret_t) {
        oal_io_print(" [0x%8x:0x%8x]\n", reg_name, reg_t);
    } else {
        oal_io_print("read %s register failed, ret=%d\n", name, ret_t);
    }
#endif
}

#define print_pcie_config_reg(dev, reg_name)                 \
    do {                                                     \
        oal_pcie_print_config_reg(dev, reg_name, #reg_name); \
    } while (0)

typedef uint32_t pcie_dev_ptr; /* Device CPU 指针大小，目前都是32bits */

typedef struct _oal_pcie_res__ oal_pcie_res;

typedef struct _oal_reg_bits_stru_ {
    uint32_t flag;
    uint32_t value;
    char *name;
} oal_reg_bits_stru;

typedef struct _oal_pcie_msi_stru_ {
    int32_t is_msi_support;
    oal_irq_handler_t *func; /* msi interrupt map */
    int32_t msi_num;       /* msi number */
} oal_pcie_msi_stru;

typedef struct _oal_pcie_h2d_stat_ {
    uint32_t tx_count;
    uint32_t tx_done_count;
    uint32_t tx_burst_cnt[PCIE_EDMA_READ_BUSRT_COUNT + 1];
} oal_pcie_h2d_stat;

typedef struct _oal_pcie_d2h_stat_ {
    uint32_t rx_count;
    uint32_t rx_done_count;
    uint32_t rx_burst_cnt[PCIE_EDMA_WRITE_BUSRT_COUNT + 1];
    uint32_t alloc_netbuf_failed;
    uint32_t map_netbuf_failed;
} oal_pcie_d2h_stat;

typedef struct _oal_pcie_trans_stat_ {
    uint32_t intx_total_count;
    uint32_t intx_tx_count;
    uint32_t intx_rx_count;
    uint32_t done_err_cnt;
    uint32_t h2d_doorbell_cnt;
    uint32_t d2h_doorbell_cnt;
} oal_pcie_trans_stat;

typedef struct _pci_addr_map__ {
    /* unsigned long 指针长度和CPU位宽等长 */
    uintptr_t va; /* 虚拟地址 */
    uintptr_t pa; /* 物理地址 */
} pci_addr_map;

typedef struct _pcie_cb_dma_res_ {
    edma_paddr_t paddr;
    uint32_t len;
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
    uint8_t bar_idx;
    uint64_t start; /* PCIe在Host分配到的总物理地址大小 */
    uint64_t end;

    /* PCIe 发出的总线地址空间， 和start 有可能一样，
      有可能不一样，这个值是配置到BAR 和iatu 的 SRC 地址 */
    uint64_t bus_start;

    uint32_t size;
} oal_pcie_bar_info;

#define oal_pcie_to_name(name) #name
typedef struct _oal_pcie_region_ {
    void *vaddr;  /* virtual address after remap */
    uint64_t paddr; /* PCIe在Host侧分配到的物理地址 */

    uint64_t bus_addr; /* PCIe RC 发出的总线地址 */

    /* pci为PCI看到的地址和CPU看到的地址 每个SOC 大小和地址可能有差异 */
    /* device pci address */
    uint64_t pci_start;
    uint64_t pci_end;
    /* Device侧CPU看到的地址 */
    uint64_t cpu_start;
    uint64_t cpu_end;
    uint32_t size;

    uint32_t flag; /* I/O type,是否需要刷Cache */

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
    int32_t region_nums; /* region nums */

    oal_pcie_bar_info *pst_bars;
    int32_t bar_nums;

    int32_t inited; /* 非0表示初始化过 */
} oal_pcie_regions;

typedef uint32_t  dev_caddr; /* device cpu address */

typedef struct _oal_pcie_address_info_ {
    dev_caddr edma_ctrl; /* edma ctrl base address */
    dev_caddr pcie_ctrl; /* pcie ctrl base address */
    dev_caddr dbi; /* pcie config base address */
    dev_caddr ete_ctrl; /* pcie host ctrl address */
    dev_caddr glb_ctrl;
    dev_caddr pmu_ctrl;
    dev_caddr pmu2_ctrl;
    dev_caddr sharemem_addr; /* device reg1 */
    dev_caddr boot_version; /* bootloader  */
} oal_pcie_address_info;

typedef int32_t (*pcie_chip_func)(oal_pcie_res *, int32_t);
typedef struct _pcie_chip_id_stru_ {
    int32_t device_id;
    pcie_chip_func func;
} pcie_chip_id_stru;

typedef struct _pcie_chip_pm_cb_ {
    int32_t (*pcie_device_auxclk_init)(oal_pcie_res *pst_pci_res);
    int32_t (*pcie_device_aspm_init)(oal_pcie_res *pst_pci_res);
    void    (*pcie_device_aspm_ctrl)(oal_pcie_res *pst_pci_res, oal_bool_enum clear);
    int32_t (*pcie_device_phy_config)(oal_pcie_res *pst_pci_res);
} pcie_chip_pm_cb;

typedef struct _pcie_chip_cb_ {
    uintptr_t (*get_test_ram_address)(void);
    int32_t (*pcie_voltage_bias_init)(oal_pcie_res *);
    int32_t (*pcie_get_bar_region_info)(oal_pcie_res *, oal_pcie_region **, uint32_t *);
    int32_t (*pcie_set_outbound_membar)(oal_pcie_res *, oal_pcie_iatu_bar *);
    int32_t (*pcie_host_slave_address_switch)(oal_pcie_res *, uint64_t, uint64_t*, int32_t);
    int32_t (*pcie_ip_factory_test)(hcc_bus *pst_bus, int32_t test_count);
    pcie_chip_pm_cb pm_cb;
} pcie_chip_cb;

typedef struct _pcie_chip_info_ {
    uint32_t dual_pci_support; /* 2 pcie_eps */
    uint32_t ete_support;
    uint32_t edma_support;
    uint32_t membar_support; /* iatu set by membar or config */

    oal_pcie_address_info addr_info;
    pcie_chip_cb  cb; // callback functions
} pcie_chip_info;

struct _oal_pcie_res__ {
    void *data;      /* callback pointer */
    uint32_t revision; /* ip version */

    pci_wlan_link_state link_state;

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

#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
    /* dual bar1 info */
    oal_pcie_iatu_bar st_iatu_dual_bar;
#endif

    /* PCIe Device 寄存器基地址,Host Virtual Address */
    void *pst_pci_dma_ctrl_base;
    void *pst_pci_ctrl_base;
    void *pst_pci_ctrl2_base; /* pcie1 ctrl */
    void *pst_pci_dbi_base;
    void *pst_ete_base;

    /* Rx 补充内存线程 2级线程 高优先级线程实时补充+低优先级补充 */
    struct task_struct *pst_rx_hi_task;
    struct task_struct *pst_rx_normal_task;

    oal_mutex_stru st_rx_mem_lock;
    oal_wait_queue_head_stru st_rx_hi_wq;
    oal_wait_queue_head_stru st_rx_normal_wq;

    oal_atomic rx_hi_cond;
    oal_atomic rx_normal_cond;

    pcie_chip_info chip_info;
    oal_ete_res        ete_info;
};

#define pcie_res_to_dev(pst_pci_res) (oal_pci_dev_stru *)((pst_pci_res)->data)

typedef enum _pcie_mips_type_ {
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
} pcie_mips_type;

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE_PERFORMANCE
void oal_pcie_mips_start(pcie_mips_type type);
void oal_pcie_mips_end(pcie_mips_type type);
void oal_pcie_mips_clear(void);
void oal_pcie_mips_show(void);
#else
OAL_STATIC OAL_INLINE void oal_pcie_mips_start(pcie_mips_type type)
{
    oal_reference(type);
}

OAL_STATIC OAL_INLINE void oal_pcie_mips_end(pcie_mips_type type)
{
    oal_reference(type);
}

OAL_STATIC OAL_INLINE void oal_pcie_mips_clear(void)
{
}

OAL_STATIC OAL_INLINE void oal_pcie_mips_show(void)
{
}
#endif

/* function declare */
void oal_pcie_tx_netbuf_free(oal_pcie_res *pci_res, oal_netbuf_stru *netbuf);
int32_t oal_pcie_send_netbuf_list(oal_pcie_res *pci_res, oal_netbuf_head_stru *head,
                                  pcie_h2d_ringbuf_qtype qtype);
oal_pcie_res *oal_pcie_host_init(void *data, oal_pcie_msi_stru *msi, uint32_t revision);
void oal_pcie_host_exit(oal_pcie_res *pci_res);
int32_t oal_pcie_dev_init(oal_pcie_res *pci_res);
void oal_pcie_dev_deinit(oal_pcie_res *pci_res);
oal_pcie_res *oal_get_default_pcie_handler(void);
void oal_pcie_set_dma_data_rx_check_flag(int32_t flag);

int32_t oal_pcie_set_outbound_iatu_by_membar(void* pst_bar1_vaddr, uint32_t index, uint64_t src_addr,
                                             uint64_t dst_addr, uint64_t limit_size);
int32_t oal_pcie_set_outbound_by_membar(oal_pcie_res *pst_pci_res);
void oal_pcie_rx_netbuf_submit(oal_pcie_res *pci_res, oal_netbuf_stru *netbuf);
void oal_pcie_release_rx_netbuf(oal_pcie_res *pci_res, oal_netbuf_stru *netbuf);
int32_t oal_pcie_vaddr_isvalid(oal_pcie_res *pst_pci_res, const void *vaddr);
int32_t oal_pcie_inbound_ca_to_va(oal_pcie_res *pst_pci_res, uint64_t dev_cpuaddr,
                                  pci_addr_map *addr_map);
int32_t oal_pcie_inbound_va_to_ca(oal_pcie_res *pst_pci_res, uint64_t host_va, uint64_t *dev_cpuaddr);
int32_t oal_pcie_transfer_done(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_edma_tx_is_idle(oal_pcie_res *pst_pci_res, pcie_h2d_ringbuf_qtype qtype);
int32_t oal_pcie_read_d2h_message(oal_pcie_res *pst_pci_res, uint32_t *message);
int32_t oal_pcie_send_message_to_dev(oal_pcie_res *pst_pci_res, uint32_t message);
int32_t oal_pcie_get_host_trans_count(oal_pcie_res *pst_pci_res, uint64_t *tx, uint64_t *rx);
int32_t oal_pcie_edma_sleep_request_host_check(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_disable_regions(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_enable_regions(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_transfer_res_init(oal_pcie_res *pst_pci_res);
void oal_pcie_transfer_res_exit(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_iatu_init(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_check_link_state(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_set_l1pm_ctrl(oal_pcie_res *pst_pci_res, int32_t enable);
int32_t oal_pcie_set_device_soft_fifo_enable(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_set_device_dma_check_enable(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_set_device_ringbuf_bugfix_enable(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_device_aspm_init(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_device_auxclk_init(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_copy_from_device_by_dword(oal_pcie_res *pst_pci_res,
                                           void *ddr_address,
                                           uintptr_t start,
                                           uint32_t data_size);
int32_t oal_pcie_copy_to_device_by_dword(oal_pcie_res *pst_pci_res,
                                         void *ddr_address,
                                         uintptr_t start,
                                         uint32_t data_size);
uintptr_t oal_pcie_slt_get_deivce_ram_cpuaddr(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_device_changeto_high_cpufreq(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_device_mem_scanall(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_get_gen_mode(oal_pcie_res *pst_pci_res);
void oal_pcie_set_voltage_bias_param(uint32_t phy_0v9_bias, uint32_t phy_1v8_bias);
int32_t oal_pcie_get_vol_reg_1v8_value(int32_t request_vol, uint32_t *pst_value);
int32_t oal_pcie_get_vol_reg_0v9_value(int32_t request_vol, uint32_t *pst_value);
int32_t oal_pcie_voltage_bias_init(oal_pcie_res *pst_pci_res);
void oal_pcie_print_transfer_info(oal_pcie_res *pst_pci_res, uint64_t print_flag);
void oal_pcie_reset_transfer_info(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_ringbuf_h2d_refresh(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_host_pending_signal_check(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_host_pending_signal_process(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_device_phy_config(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_set_device_aspm_dync_disable(oal_pcie_res *pst_pci_res, uint32_t disable);
void oal_pcie_device_xfer_pending_sig_clr(oal_pcie_res *pst_pci_res, oal_bool_enum clear);
int32_t oal_pcie_share_mem_res_map(oal_pcie_res *pst_pci_res);
void oal_pcie_share_mem_res_unmap(oal_pcie_res *pst_pci_res);
void oal_pcie_ringbuf_res_unmap(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_ringbuf_res_map(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_host_slave_address_switch(oal_pcie_res *pst_pci_res, uint64_t src_addr,
                                           uint64_t* dst_addr, int32_t is_host_iova);
int32_t oal_pcie_sysfs_group_create(oal_kobject *root_obj);
void oal_pcie_sysfs_group_remove(oal_kobject *root_obj);
void oal_pcie_voltage_bias_param_init(void);

int32_t oal_pcie_h2d_doorbell(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_d2h_doorbell(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_get_ca_by_pa(oal_pcie_res *pst_pci_res, uintptr_t paddr, uint64_t *cpuaddr);

void oal_pcie_edma_task_exit(oal_pcie_res *pst_pcie_res);
int32_t oal_pcie_edma_task_init(oal_pcie_res *pst_pcie_res, oal_pcie_msi_stru *msi);
uint32_t oal_pcie_ringbuf_freecount(pcie_ringbuf *pst_ringbuf);

int32_t oal_pcie_transfer_edma_res_init(oal_pcie_res *pst_pci_res);
void oal_pcie_transfer_edma_res_exit(oal_pcie_res *pst_pci_res);

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/* PCIE EP Slave 口看到的地址转换 --片内外设通过PCIE访问主芯片DDR空间
 * PCIE EP Slave 口访问, 片内IP 访问HOST DDR
 * 地址转换主芯片的DDR设备地址转换成device的SLAVE口地址
 *  注意这里的DDR设备地址(iova)不是直接的物理地址，而是对应Kernel的DMA地址
 *  设备地址不能通过phys_to_virt/virt_to_phys直接转换
 */
int32_t pcie_if_hostca_to_devva(uint32_t ul_chip_id, uint64_t host_iova, uint64_t* addr);
int32_t pcie_if_devva_to_hostca(uint32_t ul_chip_id, uint64_t devva, uint64_t* addr);
oal_bool_enum_uint8 oal_pcie_link_state_up(void);
/* PCIE EP Master口看到的地址转换 --主芯片外设(CPU)通过PCIE访问片内IP(RAM+寄存器) */
int32_t oal_pcie_devca_to_hostva(uint32_t ul_chip_id, uint64_t dev_cpuaddr, uint64_t *host_va);
int32_t oal_pcie_get_dev_ca(uint32_t ul_chip_id, void *host_va, uint64_t* dev_cpuaddr);
#else
OAL_STATIC OAL_INLINE int32_t pcie_if_hostca_to_devva(uint32_t ul_chip_id, uint64_t host_iova, uint64_t* addr)
{
    *addr = host_iova;

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE int32_t pcie_if_devva_to_hostca(uint32_t ul_chip_id, uint64_t devva, uint64_t* addr)
{
    *addr = devva;

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 oal_pcie_link_state_up(void)
{
    return OAL_TRUE;
}

OAL_STATIC OAL_INLINE int32_t oal_pcie_devca_to_hostva(uint32_t ul_chip_id, uint64_t dev_cpuaddr, uint64_t *host_va)
{
    *host_va = dev_cpuaddr;

    return OAL_SUCC;
}
#endif
#else
OAL_STATIC OAL_INLINE int32_t pcie_if_hostca_to_devva(uint32_t ul_chip_id, uint64_t host_iova, uint64_t* addr)
{
    return -OAL_ENODEV;
}

OAL_STATIC OAL_INLINE int32_t pcie_if_devva_to_hostca(uint32_t ul_chip_id, uint64_t devva, uint64_t* addr)
{
    return -OAL_ENODEV;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 oal_pcie_link_state_up(void)
{
    return OAL_FALSE;
}

OAL_STATIC OAL_INLINE int32_t oal_pcie_devca_to_hostva(uint32_t ul_chip_id, uint64_t dev_cpuaddr, uint64_t *host_va)
{
    return -OAL_ENODEV;
}

OAL_STATIC OAL_INLINE int32_t oal_pcie_get_dev_ca(uint32_t ul_chip_id, void *host_va, uint64_t* dev_cpuaddr)
{
    return -OAL_ENODEV;
}

#endif

int32_t oal_ete_host_init(oal_pcie_res *pst_pci_res, oal_pcie_msi_stru *msi);
void  oal_ete_host_exit(oal_pcie_res *pst_pci_res);
void oal_ete_print_transfer_info(oal_pcie_res *pst_pci_res, uint64_t print_flag);
void oal_ete_reset_transfer_info(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_transfer_ete_done(oal_pcie_res *pst_pci_res);
int32_t oal_ete_send_netbuf_list(oal_pcie_res *pst_pci_res, oal_netbuf_head_stru *pst_head,
                                 hcc_netbuf_queue_type qtype);
int32_t oal_ete_ip_init(oal_pcie_res *pst_pci_res);
void  oal_ete_ip_exit(oal_pcie_res *pst_pci_res);
int32_t oal_ete_transfer_res_init(oal_pcie_res *pst_pci_res);
void oal_ete_transfer_res_exit(oal_pcie_res *pst_pci_res);
void oal_ete_transfer_deepsleep_clean(oal_pcie_res *pst_pci_res);
int32_t oal_ete_transfer_deepwkup_restore(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_ete_tx_is_idle(oal_pcie_res *pst_pci_res, hcc_netbuf_queue_type qtype);
void oal_ete_sched_rx_threads(oal_pcie_res *pst_pci_res);
int32_t oal_ete_sleep_request_host_check(oal_pcie_res *pst_pci_res);
int32_t oal_ete_host_pending_signal_check(oal_pcie_res *pst_pci_res);
int32_t oal_ete_host_pending_signal_process(oal_pcie_res *pst_pci_res);
void oal_ete_rx_res_clean(oal_pcie_res *pst_pci_res);
void oal_ete_tx_res_clean(oal_pcie_res *pst_pci_res, uint32_t force_clean);
int32_t oal_ete_rx_ringbuf_build(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_h2d_int(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_d2h_int(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_set_outbound_membar(oal_pcie_res *pst_pcie_res, oal_pcie_iatu_bar* pst_iatu_bar);
int32_t oal_pcie_bar1_init(oal_pcie_iatu_bar* pst_iatu_bar);
void oal_pcie_bar1_exit(oal_pcie_iatu_bar *pst_iatu_bar);
int32_t oal_pcie_print_device_mem(oal_pcie_res *pst_pcie_res, uint32_t cpu_address, uint32_t length);
int32_t oal_pcie_device_check_alive(oal_pcie_res *pst_pci_res);
void oal_pcie_regions_info_dump(oal_pcie_res *pci_res);
int32_t oal_pcie_write_dsm32(oal_pcie_res *pst_pci_res, pcie_shared_device_addr_type type, uint32_t val);
int32_t oal_pcie_read_dsm32(oal_pcie_res *pst_pci_res, pcie_shared_device_addr_type type, uint32_t *val);
uint32_t oal_pcie_h2d_ringbuf_freecount(oal_pcie_res *pst_pci_res, pcie_h2d_ringbuf_qtype qtype,
                                        int32_t is_sync);
uint32_t oal_pcie_d2h_ringbuf_freecount(oal_pcie_res *pst_pci_res, int32_t is_sync);
int32_t oal_pcie_print_device_transfer_info(oal_pcie_res *pst_pcie_res);
int32_t oal_pcie_get_bar_region_info(oal_pcie_res *pst_pci_res,
                                     oal_pcie_region **region_base, uint32_t *region_num);
void oal_pcie_print_debug_usages(void);
void oal_pcie_print_ringbuf_info(pcie_ringbuf *pst_ringbuf, pci_log_type level);
#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
int32_t oal_pcie_dual_bar_init(oal_pcie_res *pst_pci_res, oal_pci_dev_stru *pst_pci_dev);
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/* Inline functions */
OAL_STATIC OAL_INLINE void oal_pci_cache_flush(oal_pci_dev_stru *hwdev, dma_addr_t pa, int32_t size)
{
    if (oal_likely(size > 0)) {
        dma_sync_single_for_device(&hwdev->dev, pa, (size_t)size, PCI_DMA_TODEVICE);
    } else {
        oal_warn_on(1);
    }
}

/*
 * 函 数 名  : oal_pci_cache_flush
 * 功能描述  : 无效化cache
 */
OAL_STATIC OAL_INLINE void oal_pci_cache_inv(oal_pci_dev_stru *hwdev, dma_addr_t pa, int32_t size)
{
    if (oal_likely(size > 0)) {
        dma_sync_single_for_cpu(&hwdev->dev, pa, (size_t)size, PCI_DMA_FROMDEVICE);
    } else {
        oal_warn_on(1);
    }
}
#endif

/* 'offset' is a backplane address */
OAL_STATIC OAL_INLINE void oal_pcie_write_mem8(uintptr_t va, uint8_t data)
{
    *(volatile uint8_t *)(va) = (uint8_t)data;
}

OAL_STATIC OAL_INLINE uint8_t oal_pcie_read_mem8(uintptr_t va)
{
    volatile uint8_t data;

    data = *(volatile uint8_t *)(va);

    return data;
}

OAL_STATIC OAL_INLINE void oal_pcie_write_mem32(uintptr_t va, uint32_t data)
{
    *(volatile uint32_t *)(va) = (uint32_t)data;
}

OAL_STATIC OAL_INLINE void oal_pcie_write_mem16(uintptr_t va, uint16_t data)
{
    *(volatile uint16_t *)(va) = (uint16_t)data;
}

OAL_STATIC OAL_INLINE void oal_pcie_write_mem64(uintptr_t va, uint64_t data)
{
    *(volatile uint64_t *)(va) = (uint64_t)data;
}

OAL_STATIC OAL_INLINE uint16_t oal_pcie_read_mem16(uintptr_t va)
{
    volatile uint16_t data;

    data = *(volatile uint16_t *)(va);

    return data;
}

OAL_STATIC OAL_INLINE uint32_t oal_pcie_read_mem32(uintptr_t va)
{
    volatile uint32_t data;

    data = *(volatile uint32_t *)(va);

    return data;
}

OAL_STATIC OAL_INLINE uint64_t oal_pcie_read_mem64(uintptr_t va)
{
    volatile uint64_t data;

    data = *(volatile uint64_t *)(va);

    return data;
}

extern void oal_pcie_io_trans64(void *dst, void *src, int32_t size);
extern void oal_pcie_io_trans32(uint32_t *dst, uint32_t *src, int32_t size);
extern int32_t g_pcie_memcopy_type;
/* dst/src 有一端地址在PCIE EP侧，PCIE按burst方式传输 */
OAL_STATIC OAL_INLINE void oal_pcie_io_trans(uintptr_t dst, uintptr_t src, uint32_t size)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /* 网络安全函数整改 */
    if ((g_pcie_memcopy_type == 0) || (g_pcie_memcopy_type == 1)) {
        if (WARN((dst & 0x3), "invalid dst address 0x%lx", dst) ||
            WARN((src & 0x3), "invalid src address 0x%lx", dst) ||
            WARN((size & 0x3), "invalid size address 0x%lx", dst)) {
            return;
        }
#ifdef CONFIG_64BIT
        oal_pcie_io_trans64((void *)dst, (void *)src, (int32_t)size);
#else
        oal_pcie_io_trans32((uint32_t *)dst, (uint32_t *)src, (int32_t)size);
#endif
    } else if (g_pcie_memcopy_type == 2) { /* 类型2, 4字节读写 */
        uint32_t i;
        uint32_t value;
        /* 最长4字节对齐访问, Test Code 暂时不考虑 单字节，双字节 */
        if (WARN((dst & 0x3), "invalid dst address 0x%lx", dst) ||
            WARN((src & 0x3), "invalid src address 0x%lx", dst) ||
            WARN((size & 0x3), "invalid size address 0x%lx", dst)) {
            return;
        }

        for (i = 0; i < size; i += sizeof(uint32_t)) { /* 每次偏移4字节 */
            value = oal_readl((void *)(src + i));
            oal_writel(value, (void *)(dst + i));
        }
    } else if (g_pcie_memcopy_type == 3) { /* 类型3，内存块批量读写 */
        if (WARN((dst & 0x3), "invalid dst address 0x%lx", dst) ||
            WARN((src & 0x3), "invalid src address 0x%lx", dst) ||
            WARN((size & 0x3), "invalid size address 0x%lx", dst)) {
            return;
        }

        oal_pcie_io_trans32((uint32_t *)dst, (uint32_t *)src, (int32_t)size);
    }
#endif
}

OAL_STATIC OAL_INLINE uint32_t pcie_ringbuf_len(pcie_ringbuf *pst_ringbuf)
{
    /* 无符号，已经考虑了翻转 */
    uint32_t len = (pst_ringbuf->wr - pst_ringbuf->rd);
    if (len == 0) {
        return 0;
    }
#ifdef _PRE_PLAT_FEATURE_PCIE_DEBUG
    if (len % pst_ringbuf->item_len) {
        oal_io_print("pcie_ringbuf_len, size:%u, wr:%u, rd:%u" NEWLINE,
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
OAL_STATIC OAL_INLINE void oal_pcie_print_bits(void *data, uint32_t size)
{
#ifdef CONFIG_PRINTK
    int32_t ret = 0;
    uint32_t value;
    char buf[32 * 3 + 1]; /* 按bit打印，最多打印32bit，每个bit3个字符 */
    int32_t i;
    int32_t count = 0;

    if (size == 1) { /* 1表示要打印长度为1字节 */
        value = (uint32_t) * (uint8_t *)data;
        oal_io_print("value= 0x%2x, =%u (dec) \n", value, value);
        oal_io_print("07 06 05 04 03 02 01 00\n");
    } else if (size == 2) { /* 2表示要打印长度为2字节 */
        value = (uint32_t) * (uint16_t *)data;
        oal_io_print("value= 0x%4x, =%u (dec) \n", value, value);
        oal_io_print("15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00\n");
    } else if (size == 4) { /* 4表示要打印长度为4字节 */
        value = (uint32_t) * (uint32_t *)data;
        oal_io_print("value= 0x%8x, =%u (dec) \n", value, value);
        oal_io_print("31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00\n");
    } else {
        /* 传入了错误的参数，不处理返回 */
        return;
    }

    /* 按比特打印 */
    for (i = size * 8 - 1; i >= 0; i--) { // 8 bits, 1 bit each time
        ret = snprintf_s(buf + count, sizeof(buf) - count, sizeof(buf) - count - 1, "%s",
                         (1u << (uint32_t)i) & value ? " 1 " : " 0 ");
        if (ret < 0) {
            pci_print_log(PCI_LOG_ERR, "log str format err line[%d]\n", __LINE__);
            return;
        }
        count += ret;
    }
    oal_io_print("%s\n", buf);
#else
    oal_reference(data);
    oal_reference(size);
#endif
}

OAL_STATIC OAL_INLINE oal_netbuf_stru *oal_pcie_rx_netbuf_alloc(uint32_t ul_size, int32_t gflag)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    return __netdev_alloc_skb(NULL, ul_size, gflag);
#else
    oal_reference(gflag);
    return oal_netbuf_alloc(ul_size, 0, 0);
#endif
}

OAL_STATIC OAL_INLINE void oal_pcie_shced_rx_hi_thread(oal_pcie_res *pst_pci_res)
{
    oal_atomic_set(&pst_pci_res->rx_hi_cond, 1);
    oal_wait_queue_wake_up_interrupt(&pst_pci_res->st_rx_hi_wq);
}

OAL_STATIC OAL_INLINE void oal_pcie_shced_rx_normal_thread(oal_pcie_res *pst_pci_res)
{
    oal_atomic_set(&pst_pci_res->rx_normal_cond, 1);
    oal_wait_queue_wake_up_interrupt(&pst_pci_res->st_rx_normal_wq);
}

OAL_STATIC OAL_INLINE void oal_pcie_print_config_reg_bar(oal_pcie_res *pst_pci_res, uint32_t offset, char *name)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    uint32_t reg;
    reg = oal_readl(pst_pci_res->st_iatu_bar.st_region.vaddr + offset);
    oal_io_print("%-50s [0x%8x:0x%8x]\n", name, offset, reg);
#else
    oal_reference(pst_pci_res);
    oal_reference(offset);
    oal_reference(name);
#endif
}
#ifndef _PRE_LINUX_TEST
OAL_STATIC OAL_INLINE char *oal_pcie_get_link_state_str(pci_wlan_link_state link_state)
{
    if (oal_warn_on(link_state > PCI_WLAN_LINK_BUTT)) {
        pci_print_log(PCI_LOG_WARN, "invalid link_state:%d", link_state);
        return "overrun";
    }

    if (g_pcie_link_state_str[link_state] == NULL) {
        return "unkown";
    }

    return g_pcie_link_state_str[link_state];
}

OAL_STATIC OAL_INLINE void oal_pcie_change_link_state(oal_pcie_res *pst_pci_res, pci_wlan_link_state link_state_new)
{
    if (pst_pci_res->link_state != link_state_new) {
        pci_print_log(PCI_LOG_INFO, "link_state change from %s to %s",
                      oal_pcie_get_link_state_str(pst_pci_res->link_state),
                      oal_pcie_get_link_state_str(link_state_new));
    } else {
        pci_print_log(PCI_LOG_INFO, "link_state still %s", oal_pcie_get_link_state_str(link_state_new));
    }

    pst_pci_res->link_state = link_state_new;
}
#endif
#endif
