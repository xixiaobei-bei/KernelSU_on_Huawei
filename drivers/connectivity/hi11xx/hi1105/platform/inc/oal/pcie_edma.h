

#ifndef __PCIE_EDMA_H__
#define __PCIE_EDMA_H__

#include "oal_util.h"

/* MACRO DEFINITIONS */
#define PCIE_CTRL_IP_HI1103_5_00A 0x002
#define PCIE_CTRL_IP_VERSION PCIE_CTRL_IP_HI1103_5_00A
#define PCIE_CONFIG_BASE_ADDRESS 0x40102000 /* pcie config base address,4KB, 4.70a */

/* Hi1103 5.00a pcie dma address */
#define PCIE_LOGIC_DMA_ADDRESS 0x40106000

#define pcie_get_config_addr(offset) (PCIE_CONFIG_BASE_ADDRESS + (offset))
#define pcie_get_edma_addr(offset)   (PCIE_LOGIC_DMA_ADDRESS + (offset))

/* eDMA Features, 2 channels for fifo transfer */
#define PCIE_EDMA_MAX_CHANNELS 2

/* Physcial address. */
#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

/* 0xa6c  DMA_VIEWPORT_SEL_OFF Call-back Debug */
#define PCIE_EDMA_VIEWPORT_REG_CALL_BACK 1

/* 无效的element首地址，此地址压入FIFO，该通道不启动 */
#define PCIE_EDMA_INVAILD_CHANN_ADDR 0xFFFFFFFFUL

/* 新增硬化逻辑FIFO操作寄存器,WCPU看到的地址 */
#define PCIE_DMA_CTRL_BASE_ADDR 0x40008000 /* 对应PCIE_DMA_CTRL页 */
#define PCIE_CTRL_BASE_ADDR     0x40007000 /* 对应PCIE_CTRL页 */

#define PCIE_REVISION_4_70A 0x1
#define PCIE_REVISION_5_00A 0x2

/* PCIE_CTRL_BASE_ADDR */
/* MSG FIFO */
#define PCIE_MSG_FIFO_STATUS      0x478
#define PCIE_MSG_FIFO_STATUS_IDLE 0xaaaa

#define PCIE_AUX_CLK_FREQ_OFF 0xb40

#define PCIE_MSG_MODE                       0x204
#define PCIE_LOW_POWER_CFG_OFF              0x220
#define PCIE_PCIE_STATUS0                   0x224
#define PCIE_MSG_INTR_OFF                   0x2A8
#define PCIE_MSG_INTR_STATUS_OFF            0x2AC
#define PCIE_MSG_INTR_CLR_OFF               0x2B0
#define PCIE_MSG_INTR_MASK_OFF              0x2B4
#define PCIE_CFG_OFF                        0x2D0
#define PCIE_HOST_DEVICE_REG0               0x2E0
#define PCIE_HOST_DEVICE_REG1               0x2E4
#define PCIE_HOST_INTR_MASK_OFF             0x2E8
#define PCIE_HOST_INTR_STATUS_OFF           0x2EC
#define PCIE_HOST_INTR_CLR                  0x2F0
#define PCIE_PHY_CFG_ADDR_OFF               0x41C
#define PCIE_PHY_WR_DATA_OFF                0x420
#define PCIE_PHY_RD_DATA_OFF                0x424
#define PCIE_PHY_CFG_OFF                    0x428
#define PCIE_STATUS2_OFF                    0x43C
#define PCIE_TX_ABORT_STATUS_OFF            0x47C
#define PCIE_RX_ABORT_STATUS_OFF            0x480
#define PCIE_INTR_RAW_STATUS_OFF            0x484
#define PCIE_STAT_CNT_LTSSM_ENTER_RCVRY_OFF 0x498
#define PCIE_STAT_CNT_L1_ENTER_RCVRY_OFF    0x49C

/* Port Logic Reg */
#define PCIE_ACK_F_ASPM_CTRL_OFF 0x70c

#define PCIE_H2D_DOORBELL_OFF            0x2D4
#define PCIE_D2H_DOORBELL_OFF            0x2D4
#define PCIE_H2D_TRIGGER_VALUE          (1 << 4)
#define PCIE_H2D_DOORBELL_TRIGGER_VALUE (1 << 5)
#define PCIE_D2H_DOORBELL_TRIGGER_VALUE (1 << 6)

/* Count必须读，address为可选 */
/* remote */
#define PCIE_FIFO_REMOTE_READ_FIFO0_DATA_OFF  0x818 /* tx */
#define PCIE_FIFO_REMOTE_READ_FIFO1_DATA_OFF  0x824 /* tx */
#define PCIE_FIFO_REMOTE_WRITE_FIFO0_DATA_OFF 0x800 /* rx */
#define PCIE_FIFO_REMOTE_WRITE_FIFO1_DATA_OFF 0x80c /* rx */

/* local */
#define PCIE_FIFO_LOCAL_READ_FIFO0_DATA_OFF  0x840 /* tx */
#define PCIE_FIFO_LOCAL_READ_FIFO1_DATA_OFF  0x848 /* tx */
#define PCIE_FIFO_LOCAL_WRITE_FIFO0_DATA_OFF 0x830 /* rx */
#define PCIE_FIFO_LOCAL_WRITE_FIFO1_DATA_OFF 0x838 /* rx */

/* PCIE_DMA_CTRL_BASE_ADDR */
#define PCIE_SYS_CTL_ID_OFF              0x000
#define PCIE_DMA_RX_LL_CHANLE0_HIGH_ADDR 0x004
#define PCIE_DMA_RX_LL_CHANLE1_HIGH_ADDR 0x008
#define PCIE_DMA_TX_LL_CHANLE0_HIGH_ADDR 0x00C
#define PCIE_DMA_TX_LL_CHANLE1_HIGH_ADDR 0x010
#define PCIE_DMA_RX_LL_CHANEL0_CTL_DATA  0x014
#define PCIE_DMA_RX_LL_CHANEL1_CTL_DATA  0x018
#define PCIE_DMA_TX_LL_CHANEL0_CTL_DATA  0x01C
#define PCIE_DMA_TX_LL_CHANEL1_CTL_DATA  0x020
#define PCIE_CFG_BASE_ADDR               0x024

/* DMA LLT FIFO, 启动DMA */
#define PCIE_CFG_BASE_ADD_OFF        0x024
#define PCIE_LLT_FIFO_STATUS         0x028
#define PCIE_DMA_WRITE_FIFO0_LLT_OFF 0x400
#define PCIE_DMA_WRITE_FIFO1_LLT_OFF 0x404
#define PCIE_DMA_READ_FIFO0_LLT_OFF  0x408
#define PCIE_DMA_READ_FIFO1_LLT_OFF  0x40C
typedef union {
    struct {
        uint32_t low_part;
        uint32_t high_part;
    } bits;
    uint64_t quad_part;
} phy_addr_info;

/* The direction of DMA transfer. */
typedef enum {
    DMA_WRITE = 0, /* < Write /ep->rc */
    DMA_READ = 1,  /* < Read rc->ep */
    DMA_BUTT = 2
} dma_dirc;

/* DMA transfer mode. */
typedef enum {
    DMA_SINGLE_BLOCK = 0, /* < Single block mode */
    DMA_MULTI_BLOCK       /* < Multi block mode */
} dma_xfer_mode;

/* DMA interrupt type. */
typedef enum {
    DMA_INT_DONE = 0, /* < DONE interrupt */
    DMA_INT_ABORT     /* < ABORT interrupt */
} dma_int_type;

/* DMA Channel status. */
typedef enum {
    CHAN_UNKNOWN = 0, /* < Unknown */
    CHAN_RUNNING,     /* < Channel is running */
    CHAN_HALTED,      /* < Channel is halted */
    CHAN_STOPPED,     /* < Channel is stopped */
    CHAN_QUEUING      /* < Queuing. Not a real HW status */
} dma_chan_status;

/* DMA hardware error types. */
typedef enum {
    DMA_ERR_NONE,     /* < No DMA error found */
    DMA_ERR_WR,       /*
                       * < The DMA Write Channel has received an error
                       *   response from the AHB/AXI bus (or RTRGT1 interface
                       *   when the AHB/AXI Bridge is not used) while reading
                       *   data from it. It's fatal error.
                       */
    DMA_ERR_RD,       /*
                       * < The DMA Read Channel has received an error response
                       *   from the AHB/AXI bus (or RTRGT1 interface when the
                       *   AHB/AXI Bridge is not used) while writing data to
                       *   it. It's fatal error.
                       */
    DMA_ERR_FETCH_LL, /*
                       * < The DMA Write/Read Channel has received an error
                       *   response from the AHB/AXI bus (or RTRGT1 interface
                       *   when the AHB/AXI Bridge is not used) while reading
                       *   a Linked List Element from local memory. It's fatal
                       *   error.
                       */
    DMA_ERR_UNSUPPORTED_RQST,
    /*
     * < The DMA Read Channel has received a PCIe
     *   Unsupported Request CPL status from the remote
     *   device in response to the MRd Request.
     */
    DMA_ERR_COMPLETER_ABORT,
    /*
     * < The DMA Read Channel has received a PCIe Completer
     *  Abort CPL status from the remote device in response
     *  to the MRd Request. Non-fatal error.
     */
    DMA_ERR_CPL_TIME_OUT,
    /*
     * < The DMA Read Channel has timed-out while waiting
     * for the remote device to respond to the MRd Request,
     * or a malformed CplD has been received. Non-fatal
     * error.
     */
    DMA_ERR_DATA_POISONING,
    /*
     * < The DMA Read Channel has detected data poisoning
     * in the CPL from the remote device in response to the
     * MRd Request. Non-fatal error.
     */
    DMA_ERR_PCIE_BUTT
} dma_err;

/* DATA STRUCTURE DECLARATIONS */
/* The Channel Control Register for read and write. */
typedef union {
    struct {
        // LSB
        uint32_t cb : 1;     // 0
        uint32_t tcb : 1;    // 1
        uint32_t llp : 1;    // 2
        uint32_t lie : 1;    // 3
        uint32_t rie : 1;    // 4
        uint32_t cs : 2;     // 5:6
        uint32_t rsvd1 : 1;  // 7
        uint32_t ccs : 1;    // 8
        uint32_t llen : 1;   // 9
        uint32_t b_64s : 1;  // 10
        uint32_t b_64d : 1;  // 11
        uint32_t pf : 5;     // 12:16
        uint32_t rsvd2 : 7;  // 17:23
        uint32_t sn : 1;     // 24
        uint32_t ro : 1;     // 25
        uint32_t td : 1;     // 26
        uint32_t tc : 3;     // 27:29
        uint32_t at : 2;     // 30:31
        // MSB
    } bits;
    uint32_t as_dword;
} chan_ctrl_lo;

/* PCIe msg */
typedef union {
    struct {
        uint32_t soc_owrd_wake_err_status : 1;       /* 0 */
        uint32_t soc_owrd_wake_cpu_act_status : 1;   /* 1 */
        uint32_t soc_owrd_wake_obff_status : 1;      /* 2 */
        uint32_t soc_owrd_wake_idle_status : 1;      /* 3 */
        uint32_t soc_pcie_send_f_err_status : 1;     /* 4 */
        uint32_t soc_pcie_send_nf_err_status : 1;    /* 5 */
        uint32_t soc_pcie_send_cor_err_status : 1;   /* 6 */
        uint32_t soc_radm_msg_cpu_active_status : 1; /* 7 */
        uint32_t soc_radm_msg_idle_status : 1;       /* 8 */
        uint32_t soc_radm_msg_obff_status : 1;       /* 9 */
        uint32_t soc_radm_msg_turnoff_status : 1;    /* 10 */
        uint32_t soc_radm_msg_unlock_status : 1;     /* 11 */
        uint32_t soc_radm_msg_ltr_status : 1;        /* 12 */
        uint32_t soc_radm_vendor_msg_status : 1;     /* 13 */
        uint32_t link_down_irq : 1;                  /* 14 */
        uint32_t reserved : 17;                      /* 15:31 */
    } bits;
    uint32_t as_dword;
} pcie_msg_intr_status;

typedef union {
    struct {
        uint32_t soc_owrd_wake_err_mask : 1;       /* 0 */
        uint32_t soc_owrd_wake_cpu_act_mask : 1;   /* 1 */
        uint32_t soc_owrd_wake_obff_mask : 1;      /* 2 */
        uint32_t soc_owrd_wake_idle_mask : 1;      /* 3 */
        uint32_t soc_pcie_send_f_err_mask : 1;     /* 4 */
        uint32_t soc_pcie_send_nf_err_mask : 1;    /* 5 */
        uint32_t soc_pcie_send_cor_err_mask : 1;   /* 6 */
        uint32_t soc_radm_msg_cpu_active_mask : 1; /* 7 */
        uint32_t soc_radm_msg_idle_mask : 1;       /* 8 */
        uint32_t soc_radm_msg_obff_mask : 1;       /* 9 */
        uint32_t soc_radm_msg_turnoff_mask : 1;    /* 10 */
        uint32_t soc_radm_msg_unlock_mask : 1;     /* 11 */
        uint32_t soc_radm_msg_ltr_mask : 1;        /* 12 */
        uint32_t soc_radm_vendor_msg_mask : 1;     /* 13 */
        uint32_t reserved : 18;                    /* 14:31 */
    } bits;
    uint32_t as_dword;
} pcie_msg_intr_mask;

/* soc dma int mask */
typedef union {
    struct {
        uint32_t pcie_edma_tx_intr_mask : 1;            /* 0 */
        uint32_t pcie_edma_rx_intr_mask : 1;            /* 1 */
        uint32_t pcie_hw_edma_tx_intr_mask : 1;         /* 2 */
        uint32_t pcie_hw_edma_rx_intr_mask : 1;         /* 3 */
        uint32_t device2host_intr_mask : 1;             /* 4 */
        uint32_t link_down_intr_mask : 1;               /* 5 */
        uint32_t pcie_msg_irq_mask : 1;                 /* 6 */
        uint32_t host2device_intr_mask : 1;             /* 7 */
        uint32_t host2device_tx_doorbell_intr_mask : 1; /* 8 */
        uint32_t host2device_rx_doorbell_intr_mask : 1; /* 9 */
        uint32_t pcie_device_hw_edma_tx_intr_mask : 1;  /* 10 */
        uint32_t pcie_device_hw_edma_rx_intr_mask : 1;  /* 11 */
        uint32_t pcie_device_hw_edma_tx_abort_mask : 1; /* 12 */
        uint32_t pcie_device_hw_edma_rx_abort_mask : 1; /* 13 */
        uint32_t pcie_edma_rxch01_intr_mask : 1;        /* 14 */
        uint32_t pcie_edma_txch01_intr_mask : 1;        /* 15 */
        uint32_t reserved : 16;                         /* 16:31 */
    } bits;
    uint32_t as_dword;
} host_edma_intr_mask;

/* 状态位和中断清除位必须一一对应 */
typedef union {
    struct {
        uint32_t pcie_edma_tx_intr_status : 1;            /* 0 */
        uint32_t pcie_edma_rx_intr_status : 1;            /* 1 */
        uint32_t pcie_hw_edma_tx_intr_status : 1;         /* 2 */
        uint32_t pcie_hw_edma_rx_intr_status : 1;         /* 3 */
        uint32_t host2device_intr_status : 1;             /* 4 */
        uint32_t host2device_tx_doorbell_intr_status : 1; /* 5 */
        uint32_t host2device_rx_doorbell_intr_status : 1; /* 6 */
        uint32_t pcie_device_hw_edma_tx_intr_status : 1;  /* 7 */
        uint32_t pcie_device_hw_edma_rx_intr_status : 1;  /* 8 */
        uint32_t pcie_device_hw_edma_tx_abort_status : 1; /* 9 */
        uint32_t pcie_device_hw_edma_rx_abort_status : 1; /* 10 */
        uint32_t reserved1 : 21;                          /* 11:31 */
    } bits;
    uint32_t as_dword;
} host_intr_status;

typedef union {
    struct {
        uint32_t tx_app_wr_err : 2;    /* 0~1 */
        uint32_t reserved1 : 2;        /* 2~3 */
        uint32_t tx_ll_fetch_err : 2;  /* 4~5 */
        uint32_t reserved2 : 2;        /* 6~7 */
        uint32_t tx_unsupport_err : 2; /* 8~9 */
        uint32_t reserved3 : 2;        /* 10~11 */
        uint32_t tx_cpl_abort : 2;     /* 12~13 */
        uint32_t reserved4 : 2;        /* 14~15 */
        uint32_t tx_cpl_timeout : 2;   /* 16~17 */
        uint32_t reserved5 : 2;        /* 18~19 */
        uint32_t tx_data_poision : 2;  /* 20~21 */
        uint32_t reserved6 : 2;        /* 22~23 */
        uint32_t reserved7 : 8;        /* 24~31 */
    } bits;
    uint32_t as_dword;
} pcie_tx_abort_status;

typedef union {
    struct {
        uint32_t rx_app_wr_err : 2;   /* 0~1 */
        uint32_t reserved1 : 2;       /* 2~3 */
        uint32_t rx_ll_fetch_err : 2; /* 4~5 */
        uint32_t reserved2 : 2;       /* 6~7 */
        uint32_t reserved3 : 24;      /* 8~31 */
    } bits;
    uint32_t as_dword;
} pcie_rx_abort_status;

typedef union {
    struct {
        uint32_t rx_fifo0_full : 1;  /* 0 */
        uint32_t rx_fifo0_empty : 1; /* 1 */
        uint32_t rx_fifo1_full : 1;  /* 2 */
        uint32_t rx_fifo1_empty : 1; /* 3 */
        uint32_t tx_fifo0_full : 1;  /* 4 */
        uint32_t tx_fifo0_empty : 1; /* 5 */
        uint32_t tx_fifo1_full : 1;  /* 6 */
        uint32_t tx_fifo1_empty : 1; /* 7 */
        uint32_t reserved : 24;      /* 8~31 */
    } bits;
    uint32_t as_dword;
} edma_fifo_stat; /* LLT edma_fifo_stat */

typedef union {
    struct {
        uint32_t rx_msg_fifo0_full : 1;      /* 0 */
        uint32_t rx_msg_fifo0_empty : 1;     /* 1 */
        uint32_t rx_msg_fifo1_full : 1;      /* 2 */
        uint32_t rx_msg_fifo1_empty : 1;     /* 3 */
        uint32_t tx_msg_fifo0_full : 1;      /* 4 */
        uint32_t tx_msg_fifo0_empty : 1;     /* 5 */
        uint32_t tx_msg_fifo1_full : 1;      /* 6 */
        uint32_t tx_msg_fifo1_empty : 1;     /* 7 */
        uint32_t rx_cpu_msg_fifo0_full : 1;  /* 8 */
        uint32_t rx_cpu_msg_fifo0_empty : 1; /* 9 */
        uint32_t rx_cpu_msg_fifo1_full : 1;  /* 10 */
        uint32_t rx_cpu_msg_fifo1_empty : 1; /* 11 */
        uint32_t tx_cpu_msg_fifo0_full : 1;  /* 12 */
        uint32_t tx_cpu_msg_fifo0_empty : 1; /* 13 */
        uint32_t tx_cpu_msg_fifo1_full : 1;  /* 14 */
        uint32_t tx_cpu_msg_fifo1_empty : 1; /* 15 */
        uint32_t reserved : 16;              /* 16:31 */
    } bits;
    uint32_t as_dword;
} msg_fifo_stat;

typedef union {
    struct {
        uint32_t soc_outband_pwrup_cmd : 1;   /* 0 */
        uint32_t soc_apps_pm_xmt_pme : 1;   /* 1 */
        uint32_t soc_app_req_entr_l1 : 1;   /* 2 */
        uint32_t soc_app_ready_entr_l23 : 1;   /* 3 */
        uint32_t soc_app_req_exit_l1 : 1;   /* 4 */
        uint32_t reserved1 : 1;   /* 5 */
        uint32_t soc_sys_aux_pwr_det : 1;   /* 6 */
        uint32_t soc_app_xfer_pending : 1;   /* 7 */
        uint32_t reserved2 : 24;   /* 8:31 */
    } bits;
    uint32_t as_dword;
} low_power_cfg;

typedef union {
    struct {
        uint32_t phy_cr_addr : 16;   /* 0:15 */
        uint32_t reserved : 16;      /* 16:31 */
    } bits;
    uint32_t as_dword;
} pcie_phy_cfg_addr;

typedef union {
    struct {
        uint32_t phy_cr_wr_data : 16;   /* 0:15 */
        uint32_t reserved : 16;      /* 16:31 */
    } bits;
    uint32_t as_dword;
} pcie_phy_wr_data;

typedef union {
    struct {
        uint32_t phy_cr_rd_data : 16;   /* 0:15 */
        uint32_t reserved : 16;      /* 16:31 */
    } bits;
    uint32_t as_dword;
} pcie_phy_rd_data;

typedef union {
    struct {
        uint32_t phy_config_en : 1;   /* 0 */
        uint32_t reserved1 : 1; /* 1 */
        uint32_t phy_write : 1; /* 2 */
        uint32_t reserevd2 : 12; /* 3:14 */
        uint32_t phy_config_rdy : 1; /* 15 */
        uint32_t reserved : 16;      /* 16:31 */
    } bits;
    uint32_t as_dword;
} pcie_phy_cfg;

typedef struct {
    volatile chan_ctrl_lo cctrlo;    /* 0~3 bytes */
    volatile uint32_t xfer_size;    /* 4~7 bytes */
    volatile uint32_t src_low_part;  /* 8~11 bytes */
    volatile uint32_t src_high_part; /* 12~15 bytes */
    volatile uint32_t dst_low_part;  /* 16~19 bytes */
    volatile uint32_t dst_high_part; /* 20~23 bytes */
} pcie_dma_data_element;

typedef struct {
    volatile chan_ctrl_lo cctrlo;    /* 0~3 bytes */
    volatile uint32_t rsvd0;       /* 4~7 bytes */
    volatile uint32_t nxt_low_part;  /* 8~11 bytes */
    volatile uint32_t nxt_high_part; /* 12~15 bytes */
    volatile uint32_t rsvd1;       /* 16~19 bytes */
    volatile uint32_t rsvd2;       /* 20~23 bytes */
} pcie_dma_link_element;

typedef union {
    volatile pcie_dma_data_element data;
    volatile pcie_dma_link_element link;
} pcie_dma_element;

/* EXTERN FUNCTION DECLARATIONS */
void pcie_edma_set_op(dma_dirc dirc, int enb);
void pcie_edma_set_wei(dma_dirc dirc, uint8_t chan, uint8_t wei);
void pcie_edma_set_door_bell(dma_dirc dirc, uint8_t chan, uint8_t stop);
#ifdef PCIE_EDMA_SINGLE_BLOCK_TRANSFER
void pcie_edma_set_xfer_size(dma_dirc dirc, uint8_t chan, uint32_t size);
void pcie_edma_set_src(dma_dirc dirc, uint8_t chan, phy_addr_info addr);
void pcie_edma_set_dst(dma_dirc dirc, uint8_t chan, phy_addr_info addr);
#endif
void pcie_edma_set_ctrl(dma_dirc dirc, uint8_t chan, dma_xfer_mode mode);
void pcie_edma_set_msi_addr(dma_dirc dirc, dma_int_type type, phy_addr_info addr);
void pcie_edma_set_msi_data(dma_dirc dirc, uint8_t chan, uint16_t data);
void pcie_edma_set_ll_int_err(dma_dirc dirc, uint8_t chan, uint8_t loc, uint8_t enb);
void pcie_edma_set_ll(dma_dirc dirc, uint8_t chan, phy_addr_info addr);
void pcie_edma_get_data_ele(uint32_t lladdr, uint8_t idx, uint8_t *pcs, uint8_t *intr,
                            uint32_t *xfer_size, phy_addr_info *src, phy_addr_info *dst);
void pcie_edma_get_data_ele_dst(uint32_t lladdr, uint8_t idx, phy_addr_info *dst);
void pcie_edma_get_data_ele_src(uint32_t lladdr, uint8_t idx, phy_addr_info *src);
void pcie_edma_set_data_ele(uint32_t lladdr, uint8_t idx, uint8_t pcs, uint8_t intr,
                            uint32_t xfer_size, phy_addr_info src, phy_addr_info dst);
void pcie_edma_set_link_ele_int(uint32_t lladdr, uint32_t idx, uint8_t intr);
void pcie_edma_set_link_ele(uint32_t lladdr, uint8_t idx, uint8_t pcs, phy_addr_info nxt,
                            uint8_t recyc);
void pcie_edma_get_link_ele(uint32_t lladdr, uint8_t idx, uint8_t *pcs, phy_addr_info *nxt);
void pcie_edma_get_chan_num(uint8_t *wrch, uint8_t *rdch);
void pcie_edma_get_chan_status(dma_dirc dirc, uint8_t chan,
                               dma_chan_status *status);
void pcie_edma_get_xfer_size(dma_dirc dirc, uint8_t chan, uint32_t *size);
void pcie_edma_get_element_ptr(dma_dirc dirc, uint8_t chan,
                               phy_addr_info *addr);
void pcie_edma_clr_int(dma_dirc dirc, uint8_t chan, dma_int_type type);
void pcie_edma_clr_int_chans(dma_dirc dirc, uint8_t chanMask, dma_int_type type);
uint8_t pcie_edma_query_int_src(dma_dirc *dirc, uint8_t *chan,
                                dma_int_type *type);
uint8_t pcie_edma_query_err_src(dma_dirc dirc, uint8_t chan, dma_err *type);
void pcie_edma_dump_reg(void);
void pcie_edma_llt_push_fifo(uint32_t addr, dma_dirc dirc, uint8_t chan);
uint32_t pcie_edma_en_fifo_is_full(dma_dirc dirc);
int32_t pcie_edma_get_write_done_fifo(uint32_t *addr, uint32_t *count);
int32_t pcie_edma_get_read_done_fifo(uint32_t *addr, uint32_t *count);
void pcie_edma_init(void);
#ifdef _PRE_PLAT_FEATURE_PCIE_DEBUG
void pcie_edma_dump_eles(uint32_t lladdr, uint8_t nums);
#endif

#endif
