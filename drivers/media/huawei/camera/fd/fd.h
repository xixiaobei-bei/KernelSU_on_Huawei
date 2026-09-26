#ifndef _FD_BASE_H_
#define _FD_BASE_H_
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h>
#include <asm/io.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/hisi/hisi_ion.h>

struct fd_smmu_dev{
    struct iommu_domain *domain;
    struct device  *dev;
    void __iomem   *reg_base;
    unsigned long   reg_size;
    unsigned long   pgd_base;
};
#define FD_HW_READ_32(addr)             readl((void __iomem *)(addr))
#define FD_HW_WRITE_32(value, addr)     writel((value), (void __iomem *)(addr))
#define FD_REG_GET_FIELD(reg, name) \
        (((reg) >> (name##_OFFSET)) & MASK0(name))

#define FD_REG_SET_FIELD(reg, name, val) \
        (reg = ((reg) & ~MASK1(name)) | (((val) & MASK0(name)) << (name##_OFFSET)))

//#define AHFD_IPU_VER_OFFSET     0x0418
#define AHFD_IPU_VER_OFFSET     fd_offset_list[0]
//#define AHFD_INT_MASK           0x040C
//#define AHFD_INT_CLEAR          0x0410
//#define AHFD_INT_STATUS         0x0414
//#define AHFD_ERR_STATUS         0x0464
#define AHFD_INT_MASK           fd_offset_list[1]
#define AHFD_INT_CLEAR          fd_offset_list[2]
#define AHFD_INT_STATUS         fd_offset_list[3]
#define AHFD_ERR_STATUS         fd_offset_list[4]

#define AHFD_MAX_OFFSET         0x1FFFC
//#define AHFD_PRE_ERR_MASK       0x00FD
//#define AHFD_TME_ERR_MASK       0x007A
//#define AHFD_SWRST_ERR_MASK     0x009F
#define AHFD_PRE_ERR_MASK       fd_offset_list[5]
#define AHFD_TME_ERR_MASK       fd_offset_list[6]
#define AHFD_SWRST_ERR_MASK     fd_offset_list[7]
//#define AHFD_PRE_EN             0x0058
//#define AHFD_TME_EN             0x031C
//#define AHFD_IDLE_STATUS                 0x0470
#define AHFD_PRE_EN              fd_offset_list[8]
#define AHFD_TME_EN              fd_offset_list[9]
#define AHFD_IDLE_STATUS    fd_offset_list[10]

#define RESULT_TYPE_COUNT       2 //PRE and TME
#define IDX_WAIT_PRE            0
#define IDX_WAIT_TME            1

enum
{
    IRQ_PRE_WR          = (1 << 6),
    IRQ_ERR             = (1 << 7),
    IRQ_TME_WR          = (1 << 8)
};
#define WAIT_RESULT_PENDING  0
#define WAIT_RESULT_READY  1
#define WAIT_RESULT_ERROR  2

struct fd_iomem_res {
    char __iomem *viraddr;
    phys_addr_t  phyaddr;
    unsigned int mem_size;
};

struct mem_base {
    ion_phys_addr_t img_phys_addr;
    size_t          img_size;
    ion_phys_addr_t ipu_phys_addr;
    size_t          ipu_size;
};
//#define POWER_CTRL_HW_INTF_FD      (0)
//#define POWER_CTRL_CFG_REGS_FD     (1)
//#define POWER_CTRL_INTERFACE_FD    POWER_CTRL_HW_INTF_FD
enum RC_NUM {
    FD_RC_REGULATOR = 1,
    FD_RC_CLK,
    FD_RC_MAX
};
struct fd_device {
    struct miscdevice device;
    unsigned int fd_irq;  /*FD interrupt*/
    atomic_t accessible;
    uint32_t registerBufferWrite[FD_REG_MAX_ARRAY];
    uint32_t registerBufferRead[FD_REG_MAX_ARRAY];
    wait_queue_head_t wq[RESULT_TYPE_COUNT];
    atomic_t waitFlag[RESULT_TYPE_COUNT];
    atomic_t tme_mode;                 /*TME work mode */
    struct mutex lock;
    unsigned int smmu_flag;            /*1: enable smmu;0 :smmu stream bypass*/
    unsigned int ram_config;           /*1: need config ram; 0: use asic register default value*/
    struct ion_client*                          ion_client;
    int                                         img_share_fd;
    int                                         ipu_share_fd;
    struct mem_base                             mem_base;
    struct regulator *                          fd_supply;
    struct regulator *                          fd_media1_supply;
    struct clk *                                clk;
    struct clk *                                jpg_clk;
    unsigned int clk_rate;
    unsigned int                                fd_low_clk_rate;
    unsigned int                                fd_power_ref;       /*1:need poweron;0:no need poweron*/
    struct iommu_domain *                       domain;
    unsigned int                                phy_pgd_base;      /*smmu pgd base addr*/
    unsigned int                                phy_pgd_fama_ptw_msb;  /*for FAMA smmu*/
    unsigned int                                fama_bps_msb_ns;  /* for FAMA smmubypass  */
    void __iomem* fd_viraddr;                          /* FD virtual addr */
    void __iomem* smmu_vir_base;                /* smmu common virtual base */
    void __iomem* mstr_vir_base;                   /* smmu master virtual base */
    phys_addr_t  top_phy_base;
    u32         top_mem_size;
    int rc_seq[FD_RC_MAX+1];                          /* fd power sequece */
    int smmu_img_fd;
    int smmu_ipu_fd;
};

#define MAX_FD_OFFSET_LIST_NUM 20

uint32_t smmu_offset_list[MAX_FD_OFFSET_LIST_NUM] = { 0 };
uint32_t mstr_offset_list[MAX_FD_OFFSET_LIST_NUM] = { 0 };
uint32_t fd_offset_list[MAX_FD_OFFSET_LIST_NUM] = { 0 };



//#define POWER_CTRL_HW_INTF      (0)
//#define POWER_CTRL_CFG_REGS     (1)
//#define POWER_CTRL_INTERFACE    POWER_CTRL_HW_INTF
#define MASK0(name)  (((unsigned int)1<<(name##_LEN))-1)
#define MASK1(name)  ((((unsigned int)1<<(name##_LEN))-1) << (name##_OFFSET))


// SMMU_FAMA_CTRL0_NS                      0x0220
#define FAMA_CTRL0_NS_fama_bps_msb_ns_LEN       6
#define FAMA_CTRL0_NS_fama_bps_msb_ns_OFFSET    8
#define FAMA_CTRL0_NS_fama_chn_sel_ns_LEN       1
#define FAMA_CTRL0_NS_fama_chn_sel_ns_OFFSET    7
#define FAMA_CTRL0_NS_fama_sdes_msb_ns_LEN      7
#define FAMA_CTRL0_NS_fama_sdes_msb_ns_OFFSET   0

// SMMU_FAMA_CTRL1_NS                      0x0224
#define FAMA_CTRL1_NS_fama_ptw_msb_ns_LEN       7
#define FAMA_CTRL1_NS_fama_ptw_msb_ns_OFFSET    0
//#define REG_BASE_SMMU_MASTER ((unsigned int)0xE8403000)  //TC
//#define REG_BASE_SMMU_MASTER ((unsigned int)0xE8303000) //CS

//#define SMRX_START_1 0x2C
//#define SMTR_END_ACK_0  0x1C

#define SMMU_GLOBAL_BYPASS                   0x0
//#define REG_BASE_SMMU   ((unsigned int)0xE8420000)  //TC
#define SMMU_MSTR_GLB_BYPASS  0x0
//#define FD_INPUT_BUFFER ((unsigned int)0x2E000000)
#define MIN_FD_SMMU_STREAM_ID           8
#define MAX_FD_SMMU_STREAM_ID           19
#define MIN_FD_MSTR_STREAM_ID           0
#define MAX_FD_MSTR_STREAM_ID           11
#define FD_MSTR_STREAM_ID_2               9

#if 1
#define SMMU_INTMASK_NS                     0x10
#define SMMU_INTCLR_NS                        0x1C
#define SMMU_SMRX_NS_STREAM          0x20             //stream ID 8~18 *0x4
#define SMMU_CB_TTBR0                         0x204
#define SMMU_CB_TTBCR                         0x20C
#define SMMU_FAMA_CTRL0_NS          0x220
#define SMMU_FAMA_CTRL1_NS          0x224



//master


#define SMRX_START_0 0x28
#define SMMU_MSTR_INPT_SEL 0x34
#define SMMU_MSTR_SMRX_0_STREAM 0x100  //stream ID 0~10*0x4
//master_va_start
#define SMMU_MSTR_SMRX_1_STREAM 0x260   //stream ID 0~10*0x4
//master_va_end
#define SMMU_MSTR_SMRX_2_STREAM 0x3C0   //stream ID 0~10*0x4
#endif
#define SMMU_STREAM_OFFSET 0x4

#define REG_BASE_SMMU   ((unsigned int)0xE8320000)   //CS
#define REG_BASE_SMMU_MASTER ((unsigned int)0xE8303000) //CS

#define SMMU_SMRX_NS_STREAM_ID_8    (0x20+8*0x4)
#define SMMU_SMRX_NS_STREAM_ID_9    (0x20+9*0x4)
#define SMMU_SMRX_NS_STREAM_ID_10  (0x20+10*0x4)
#define SMMU_SMRX_NS_STREAM_ID_11  (0x20+11*0x4)
#define SMMU_SMRX_NS_STREAM_ID_12  (0x20+12*0x4)
#define SMMU_SMRX_NS_STREAM_ID_13  (0x20+13*0x4)
#define SMMU_SMRX_NS_STREAM_ID_14  (0x20+14*0x4)
#define SMMU_SMRX_NS_STREAM_ID_15  (0x20+15*0x4)
#define SMMU_SMRX_NS_STREAM_ID_16  (0x20+16*0x4)
#define SMMU_SMRX_NS_STREAM_ID_17  (0x20+17*0x4)
#define SMMU_SMRX_NS_STREAM_ID_18  (0x20+18*0x4)

#define SMMU_MSTR_SMRX_0_STREAM_ID_8    (0x100+0*0x4)
#define SMMU_MSTR_SMRX_0_STREAM_ID_9    (0x100+1*0x4)
#define SMMU_MSTR_SMRX_0_STREAM_ID_10  (0x100+2*0x4)
#define SMMU_MSTR_SMRX_0_STREAM_ID_11  (0x100+3*0x4)
#define SMMU_MSTR_SMRX_0_STREAM_ID_12  (0x100+4*0x4)
#define SMMU_MSTR_SMRX_0_STREAM_ID_13  (0x100+5*0x4)
#define SMMU_MSTR_SMRX_0_STREAM_ID_14  (0x100+6*0x4)
#define SMMU_MSTR_SMRX_0_STREAM_ID_15  (0x100+7*0x4)
#define SMMU_MSTR_SMRX_0_STREAM_ID_16  (0x100+8*0x4)
#define SMMU_MSTR_SMRX_0_STREAM_ID_17  (0x100+9*0x4)
#define SMMU_MSTR_SMRX_0_STREAM_ID_18  (0x100+10*0x4)


#endif
