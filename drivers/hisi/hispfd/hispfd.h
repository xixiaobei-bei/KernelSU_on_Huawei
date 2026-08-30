

#ifndef __HISPFD_H__
#define __HISPFD_H__

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/iommu.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <linux/hisi-iommu.h>
#include <linux/time.h>
#include <linux/semaphore.h>
#include <linux/jiffies.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/statfs.h>
#include <linux/hisi/hipp.h>
#include <linux/list.h>

#include <asm/uaccess.h>
#include <linux/ion.h>
#include <linux/hisi/hisi_ion.h>


//#define HIFD_TIME_TEST
//#define HIFD_REG_DEBUG
#ifdef HIFD_V130
#define HIFD_IPP_SMMU_API
#endif
// IPP TOP
#ifdef HIFD_V130
#define FD_REG_JPG_TOP_AXI_CFG0     (0x0)
#define FD_REG_JPG_DMA_CRG_CFG1     (0x4)
#define FD_REG_JPG_HIFD_CRG_CFG0    (0x0400)
#define FD_REG_JPG_HIFD_CRG_CFG1    (0x0404)
#else
#define FD_REG_JPG_TOP_AXI_CFG0     (0x0040)
#define FD_REG_JPG_DMA_CRG_CFG1     (0x0054)
#define FD_REG_JPG_TOP_CFG0         (0x0070)
#define FD_REG_JPG_HIFD_CRG_CFG0    (0x00a0)
#define FD_REG_JPG_HIFD_CRG_CFG1    (0x00a4)
#endif
// SMMU COMMON
#define FD_REG_SMMU_SCACHEI_ALL     (0x0214)

// SMMU Master
#define FD_REG_SMMU_MSTR_SMRX_START_0   (0x0028)
#define FD_REG_SMMU_MSTR_END_ACK_0      (0x001c)

// HIFD Reg
#define FD_REG_MEM_DEEP_SLP_OFF     (0x0004)
#define FD_REG_START_OFFSET         (0x0010)
#define FD_REG_HIFD_CLK_SEL_OFFSET  (0x0014)
#define FD_REG_HIFD_CLK_EN_OFFSET   (0x0018)
#define FD_REG_STATUS_OFFSET        (0x0020)
#define FD_REG_BASE_ADDR_OFFSET     (0x0028)
#define FD_REG_INTS_OFFSET          (0x0040)
#define FD_REG_INT_MASK_OFFSET      (0x0044)
#define FD_REG_CLEAR_EN_OFFSET      (0x0050)
#define FD_REG_CAU_ID_PC_OFFSET     (0x0104)
#define FD_REG_DEBUG_MODE_OFFSET    (0x0C10)
#define FD_REG_INST_ADDR_OFFSET     (0x1000)

#define ADDR_FDAI_PROP_CFG1         1024    // (0x0400)
#define ADDR_FDAI_PROP_CFG2         1028    // (0x0404)
#define ADDR_FDAI_PROP_CFG3         1032    // (0x0408)
#define ADDR_FDAI_PROP_CFG4         1036    // (0x040C)
#define ADDR_FDAI_PROP_CFG5         1040    // (0x0410)
#define ADDR_FDAI_PROP_CFG6         1044    // (0x0414)
#define ADDR_FDAI_PROP_CFG7         1048    // (0x0418)
#define ADDR_FDAI_PROP_CFG8         1052    // (0x041C)
#define ADDR_FDAI_PROP_CFG9         1056    // (0x0420)
#define ADDR_FDAI_PROP_CFG10        1060    // (0x0424)
#define ADDR_FDAI_PROP_CFG11        1064    // (0x0428)
#define ADDR_FDAI_PROP_CFG12        1068    // (0x042C)
#define ADDR_FDAI_PROP_CFG13        1072    // (0x0430)
#define ADDR_FDAI_PROP_CFG14        1076    // (0x0434)
#define ADDR_FDAI_PROP_CFG15        1080    // (0x0438)
#define ADDR_FDAI_PROP_CFG16        1084    // (0x043C)
#define ADDR_FDAI_PROP_CFG17        1088    // (0x0440)
#define ADDR_FDAI_PROP_CFG18        1092    // (0x0444)
#define ADDR_FDAI_PROP_CFG19        1096    // (0x0448)
#define ADDR_FDAI_PROP_CFG20        1100    // (0x044C)
#define ADDR_FDAI_PROP_CFG21        1104    // (0x0450)
#define ADDR_FDAI_PROP_CFG22        1108    // (0x0454)

#define ADDR_FDAI_MODULE_CLK_SEL    (0x0500)
#define ADDR_FDAI_MODULE_CLK_EN     (0x0504)
#define ADDR_FDAI_CH_CLK_SEL        (0x0508)
#define ADDR_FDAI_CH_CLK_EN         (0x050c)

#define ADDR_FDAI_PRE_SAMP_Y_OFFSET 2048    // (0x0800)
#define ADDR_FDAI_PRE_SAMP_WIDTH    2052    // (0x0804)
#define ADDR_FDAI_PRE_DFC_CLIP_CTL_HRZ 2080 // (0x0820)
#define ADDR_FDAI_PRE_DFC_MODULE_EN 2084    // (0x0824)
#define ADDR_FDAI_PRE_DFC_CLIP_CTL_VRT 2088 // (0x0828)
#define ADDR_FDAI_PRE_CSC_IDC0      2112    // (0x0840)
#define ADDR_FDAI_PRE_CSC_IDC2      2116    // (0x0844)
#define ADDR_FDAI_PRE_CSC_ODC0      2120    // (0x0848)
#define ADDR_FDAI_PRE_CSC_ODC2      2124    // (0x084c)
#define ADDR_FDAI_PRE_CSC_P0        2128    // (0x0850)
#define ADDR_FDAI_PRE_CSC_P1        2132    // (0x0854)
#define ADDR_FDAI_PRE_CSC_P2        2136    // (0x0858)
#define ADDR_FDAI_PRE_CSC_P3        2140    // (0x085c)
#define ADDR_FDAI_PRE_CSC_P4        2144    // (0x0860)
#define ADDR_FDAI_PRE_CSC_ICG_MODULE 2148   // (0x0864)
#define ADDR_FDAI_PRE_CSC_MPREC     2152    // (0x0868)
#define ADDR_FDAI_PRE_SCF_INPUT_WIDTH 2176  // (0x0880)
#define ADDR_FDAI_PRE_SCF_OUTPUT_WIDTH 2180 // (0x0884)
#define ADDR_FDAI_PRE_SCF_EN_HSCL_STR 2184  // (0x0888)
#define ADDR_FDAI_PRE_SCF_EN_VSCL_STR 2188  // (0x088C)
#define ADDR_FDAI_PRE_SCF_H_V_ORDER 2192    // (0x0890)
#define ADDR_FDAI_PRE_SCF_CH_CORE_GT 2196   // (0x0894)
#define ADDR_FDAI_PRE_SCF_EN_HSCL 2200      // (0x0898)
#define ADDR_FDAI_PRE_SCF_EN_VSCL 2204      // (0x089C)
#define ADDR_FDAI_PRE_SCF_INC_HSCL 2208     // (0x08A0)
#define ADDR_FDAI_PRE_SCF_INC_VSCL 2212     // (0x08A4)
#define ADDR_FDAI_PRE_POST_CLIP_CTL_HRZ 2240// (0x08C0)
#define ADDR_FDAI_PRE_REA_WIDTH 2272        // (0x08E0)
#define ADDR_FDAI_PRE_REA_PAD_HRZ 2276      // (0x08E4)
#define ADDR_FDAI_PRE_REA_PAD_SHIFT 2280    // (0x08E8)
#define ADDR_FDAI_RDMA_RAW_EN 2304          // (0x0900)
#define ADDR_FDAI_RDMA_BLOCK_ROT_EN 2308    // (0x0904)
#define ADDR_FDAI_RDMA_OUTSTANGING 2368     // (0x0940)
#define ADDR_FDAI_RDMA_ARCACHE 2372         // (0x0944)
#define ADDR_FDAI_RDMA_ARPROT 2376          // (0x0948)
#define ADDR_FDAI_RDMA_ARQOS 2380           // (0x094C)
#define ADDR_FDAI_RDMA_RESERVE0 2432        // (0x0980)
#define ADDR_FDAI_RDMA_RESERVE1 2436        // (0x0984)
#define ADDR_FDAI_RDMA_RESERVE2 2440        // (0x098C)
#define ADDR_FDAI_RDMA_RESERVE3 2444        // (0x0990)
#define ADDR_FDAI_WDMA_OUTSTANDING 2560     // (0x0A00)
#define ADDR_FDAI_WDMA_RAW_EN 2564          // (0x0A04)
#define ADDR_FDAI_WDMA_BLOCK_ROT_EN 2568    // (0x0A08)
#define ADDR_FDAI_WDMA_HFBCE_AUTO_CLK_GT_EN 2572    // (0x0A0C)
#define ADDR_FDAI_WDMA_THRESHOLD 2576       // (0x0A10)
#define ADDR_FDAI_WDMA_DEBUG0 2580          // (0x0A14)
#define ADDR_FDAI_WDMA_DEBUG1 2584          // (0x0A18)
#define ADDR_FDAI_WDMA_MONITOR0 2588        // (0x0A1C)
#define ADDR_FDAI_WDMA_MONITOR1 2592        // (0x0A20)
#define ADDR_FDAI_WDMA_MONITOR2 2596        // (0x0A24)
#define ADDR_FDAI_WDMA_MONITOR3 2600        // (0x0A2C)
#define ADDR_FDAI_WDMA_AWCACHE 2604         // (0x0A30)
#define ADDR_FDAI_WDMA_AWPROT 2608          // (0x0A34)
#define ADDR_FDAI_WDMA_AWQOS 2612           // (0x0A38)
#define ADDR_FDAI_BASE_ADDR_1                   (0x0A40)
#define ADDR_FDAI_BASE_ADDR_2                   (0x0A44)
#define ADDR_FDAI_BASE_ADDR_3                   (0x0A48)
#define ADDR_FDAI_BLOCK_ALIGN_EN                (0x0A50) 
#define ADDR_FDAI_X2X_ENABLE_DATA_PACKING_N 2816    // (0x0A3C)

#define BIT_FDAI_INTS_FRAME_OVER            (0)
#define BIT_FDAI_INTS_NO_FACE               (1)
#define BIT_FDAI_INTS_WDMA_ERROR            (2)
#define BIT_FDAI_INTS_RDMA_ERROR            (3)
#define BIT_FDAI_INTS_CLEAR_OVER            (4)
#define BIT_FDAI_INTS_PC_OVERFLOW           (5)
#define BIT_FDAI_INTS_STEP_OVER             (6)
#define BIT_FDAI_INTS_RESERVE               (7)

#define FD_COMM_REG_MAX                     64
#define FD_INTR_MASK                        (0xd0)

#define FD_OM_NORESET                       (1 << 0)
#define FD_OM_TIME                          (1 << 1)

#define FD_BURST_NUM                        (4)


//     ATL: 0.65V/0.70V/0.80V;     PHX:0.60V/0.65V/0.70V/0.80V
//           480M/ 554M/ 640M;          384M/ 480M/ 554M/ 640M
#define HIFD_CLK_LEVEL_LOW                  (0)
#define HIFD_CLK_MAX_NUM                    (4)


typedef enum FDAI_INTS_TYPE {
    FDAI_INTS_FRAME_OVER    = 1 << BIT_FDAI_INTS_FRAME_OVER,
    FDAI_INTS_NO_FACE       = 1 << BIT_FDAI_INTS_NO_FACE,
    FDAI_INTS_WDMA_ERROR    = 1 << BIT_FDAI_INTS_WDMA_ERROR,
    FDAI_INTS_RDMA_ERROR    = 1 << BIT_FDAI_INTS_RDMA_ERROR,
    FDAI_INTS_CLEAR_OVER    = 1 << BIT_FDAI_INTS_CLEAR_OVER,
    FDAI_INTS_PC_OVERFLOW   = 1 << BIT_FDAI_INTS_PC_OVERFLOW,
    FDAI_INTS_STEP_OVER     = 1 << BIT_FDAI_INTS_STEP_OVER,
    FDAI_INTS_RESERVE       = 1 << BIT_FDAI_INTS_RESERVE
}fdai_ints_type_en;


typedef enum HISPFD_PIC_TYPE {
    TYPE_FD_HP = 0,         // HP
    TYPE_FD_LP,             // LP
    
    TYPE_PIC_BIG_MAX = 100,

    TYPE_SMALL  = 200,

    TYPE_186_LANDMARK = 300
}hispfd_pic_type_en;

#define DEBUG_BIT   (1 << 2)
#define INFO_BIT    (1 << 1)
#define ERROR_BIT   (1 << 0)

#define D(fmt, args...) \
    do { \
        if (kmsgcat_mask & DEBUG_BIT) \
            printk("[hispfd][%s][%d] " fmt, __func__, __LINE__, ##args); \
    } while (0)
    
#define I(fmt, args...) \
    do { \
        if (kmsgcat_mask & INFO_BIT) \
            printk("[hispfd][%s][%d] " fmt, __func__, __LINE__, ##args); \
    } while (0)
    
#define E(fmt, args...) \
    do { \
        if (kmsgcat_mask & ERROR_BIT) \
            printk("[hispfd][%s][%d] " fmt, __func__, __LINE__, ##args); \
    } while (0)

enum HISPFD_CLK_TYPE {
    HFD_CLK        = 0,
    HFD_CLK1       = 1,
    HFD_CLK2       = 2,
    HFD_CLK3       = 3,
    MAX_HISPFD_CLK
};

enum HISPFD_IRQ_TYPE {
    HFD_IRQ        = 0,
    SMMU_IRQ       = 1,
    MAX_HISPFD_IRQ
};

enum HISPFD_REG_TYPE {
    HFD_REG        = 0,
    SMMU_REG       = 1,
    SMMU_MST_REG   = 2,
    JPG_TOP_REG    = 3,
    MAX_HISPFD_REG
};

enum HISPFD_BOOT_TYPE {
    BOOT_TYPE_FD,
    BOOT_TYPE_ESP,
    BOOT_TYPE_FFD,
    BOOT_TYPE_MAX
};


#define HISPFD_BOOT_SIZE    (100 * 8)

struct hispfd_reg_s {
    unsigned int index;
    unsigned int offset;
    unsigned int value;
};

struct hispfd_fd_reg_s {
    unsigned int offset;
    unsigned int value;
};

#define HIFD_INVALID    0
#define HIFD_VALID      1

struct hispfd_start_st {
    unsigned int            flag;      // 0 : invalid(no boot code) ; 1 - valid(no boot code)
    unsigned int            step;
    unsigned int            pdata_size;
    unsigned int            reg_num;
    unsigned char          *pdata;
    struct hispfd_fd_reg_s *reg;
};

struct hispfd_start_burst{
    // If the following nn type is the same as the first one, only the first start_tbl->ctrl->flag set to 1;
    struct hispfd_start_st *start_tbl;
    unsigned int num;
};

#ifdef HIFD_TIME_TEST
struct hisp_time_s{
    u64 time_max;
    u64 time_min;
    u64 avrg;
    u32 num;
    u32 index;
    u64 time[100];
};
#endif

struct hispfd_common_reg_st {
    unsigned int reg_num;
    struct hispfd_fd_reg_s *preg;
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
struct hifd_memory_node{
    struct dma_buf *dmabuf;
    int shared_fd;
    struct list_head nd;
};
struct hifd_memory_block_st {
    int shared_fd;
    int size;
    unsigned long prot;
    unsigned int da;
    int usage;
    void *viraddr;
};
#endif

struct hispfd_s {
    struct miscdevice miscdev;
    struct platform_device *pdev;
    wait_queue_head_t wait;
    int hfd_ready;
    int initialized;
    int open_refs;
    struct regulator *hfd_supply;
    struct regulator *hfd_media1_supply;
    unsigned int irq_num;
    unsigned int reg_num;
    int irq[MAX_HISPFD_IRQ];
    struct resource *r[MAX_HISPFD_REG];
    void __iomem *reg[MAX_HISPFD_REG];
    struct clk  *hfdclk;
    struct clk  *jpgclk;
    
    unsigned int poweroff_clk_rate;
    unsigned int clk_rate[HIFD_CLK_MAX_NUM];
    unsigned int clk_num;
    
    struct iommu_domain *domain;
    unsigned long long pteaddr;
    
    struct hispfd_start_st start_tbl[FD_BURST_NUM];
    unsigned int start_num;
    unsigned int start_idx;
    
    struct hispfd_fd_reg_s pre_reg[FD_BURST_NUM][FD_COMM_REG_MAX];

    unsigned int om_flag;

    int pwrupflag;
    
    struct semaphore hifd_sem;
    struct semaphore open_sem;

    
    struct semaphore fd_switch_sem;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    struct list_head dma_buf_list;
#endif
#ifdef HIFD_IPP_SMMU_API
    struct hipp_common_s *smmu_nsec_drv;
#endif
};

typedef int (*fn_hifd_ioctl)(struct hispfd_s *dev, unsigned long args);

struct hisp_ioctl_s{
    u32             ioctl_id;
    fn_hifd_ioctl   hifd_ioctl;
};


#define HISPFD_WR_REG           _IOW( 'F', 0x2004, struct hispfd_reg_s)
#define HISPFD_RD_REG           _IOWR('F', 0x2005, struct hispfd_reg_s)
#define HISPFD_GET_TIME         _IOR( 'F', 0x2008, struct hisp_time_s)
#define HISPFD_GET_IRQ_TIME     _IOR( 'F', 0x2009, struct hisp_time_s)
#define HISPFD_SEM_P            _IO(  'F', 0x200A)
#define HISPFD_SEM_V            _IO(  'F', 0x200B)
#define HISPFD_CLK_SETRATE      _IOW( 'F', 0x200E, unsigned int)
#define HISPFD_START_BURST      _IOW( 'F', 0x2010, struct hispfd_start_burst)
#define HISPFD_COMMON_REG       _IOW( 'F', 0x2011, struct hispfd_common_reg_st)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#define HISPFD_IOMMU_MAP        _IOWR( 'F', 0x2012, struct hifd_memory_block_st)
#define HISPFD_IOMMU_UNMAP      _IOWR( 'F', 0x2013, struct hifd_memory_block_st)
#endif


#define DTS_NAME_HISPFD "hisilicon,hisp-hfd"
#define HISP_MIN(a, b) (((a) < (b)) ? (a) : (b))

#endif /* __HISPFD_H__ */
