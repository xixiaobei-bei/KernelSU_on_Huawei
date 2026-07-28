

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
#include <linux/hisi/hisi-iommu.h>
#include <linux/time.h>
#include <linux/semaphore.h>
#include <linux/jiffies.h>

#include <asm/uaccess.h>
#include <linux/ion.h>
#include <linux/hisi/hisi_ion.h>


//#define HIFD_TIME_TEST
//#define HIFD_REG_DEBUG


// IPP TOP
#define FD_REG_JPG_TOP_AXI_CFG0     (0x0040)
#define FD_REG_JPG_DMA_CRG_CFG1     (0x0054)
#define FD_REG_JPG_TOP_CFG0         (0x0070)
#define FD_REG_JPG_HIFD_CRG_CFG0    (0x00a0)
#define FD_REG_JPG_HIFD_CRG_CFG1    (0x00a4)

// SMMU COMMON
#define FD_REG_SMMU_SCACHEI_ALL     (0x0214)

// SMMU Master
#define FD_REG_SMMU_MSTR_SMRX_START_0   (0x0028)
#define FD_REG_SMMU_MSTR_END_ACK_0      (0x001c)

#define FD_REG_MEM_DEEP_SLP_OFF     (0x0004)
#define FD_REG_START_OFFSET         (0x0010)
#define FD_REG_HIFD_CLK_SEL_OFFSET  (0x0014)
#define FD_REG_HIFD_CLK_EN_OFFSET   (0x0018)
#define FD_REG_STATUS_OFFSET        (0x0020)
#define FD_REG_BASE_ADDR_OFFSET     (0x0028)
#define FD_REG_INTS_OFFSET          (0x0040)
#define FD_REG_INT_MASK_OFFSET      (0x0044)
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
    TYPE_768X576_640X480 = 0,
    TYPE_576X576_480X480,
    TYPE_768X432_640X360,
    TYPE_768X576_320X240,
    TYPE_576X576_240X240,
    TYPE_768X432_320X180,
    TYPE_768X384_480X240,
    TYPE_768X384_640X320,
    TYPE_768X768_240X240,
    TYPE_768X768_480X480, 
    TYPE_PIC_BIG_MAX = 100,

    TYPE_SMALL  = 200,

    TYPE_186_LANDMARK = 300
}hispfd_pic_type_en;

typedef enum DATA_TYPE {
    SINGED_16 = 0,
    UNSIGNED_8 ,
    DTYPE_MAX
}data_type_enum;

typedef enum OUTPUT_SIZE_SCALE { 
    FDAI_PROP_HEIGHT = 0,
    FDAI_PROP_WIDTH ,
    FDAI_PROP_SCALE ,
    FDAI_OUTPUT_SIZE_SCALE_MAX 
}output_info;


#define NUM_PRE_CFG     64

struct hispfd_pic_type_s {
    unsigned int value;     // register value 
    unsigned int reg;       // register addr
};


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
    IRQ2           = 2,
    IRQ3           = 3,
    MAX_HISPFD_IRQ
};

enum HISPFD_REG_TYPE {
    HFD_REG        = 0,
    SMMU_REG       = 1,
    SMMU_MST_REG   = 2,
    JPG_TOP_REG    = 3,
    MAX_HISPFD_REG
};

#define FD_PRE_MAX_NUM  32

struct hisp_4baseaddr{
    u32 base_addr0_yuv;         // yuv pic addr
    u32 base_addr1_instr_para;  // instruction and para bin
    u32 base_addr2_middle;      // the middle feature map of NN layers 
    u32 base_addr3_result;      // the result of NN
};


enum HISPFD_CLK_LEVEL {
    HIFD_CLK_LEVEL_LOW          = 0,
    HIFD_CLK_LEVEL_MEDIUM       = 1,
    HIFD_CLK_LEVEL_HIGH         = 2,
    HIFD_CLK_NUM
};

struct hispfd_s {
    struct miscdevice miscdev;
    struct platform_device *pdev;
    wait_queue_head_t wait;
    int hfd_ready;
    int initialized;
    atomic_t open_refs;
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
    unsigned int clk_rate[HIFD_CLK_NUM];
    struct iommu_domain *domain;
    unsigned long long pteaddr;
    
    unsigned int pre_reg[FD_PRE_MAX_NUM];
    unsigned int pre_val[FD_PRE_MAX_NUM];
    int pwrupflag;
    unsigned int clk_usrsetrate;
    unsigned int switchflag;
    
    struct semaphore hifd_sem;
};


#define HISPFD_BOOT_SIZE    (100 * 8)

struct hispfd_control_s {
    unsigned int flag;      // 0 : invalid ; 1 - valid
    unsigned int step;     // FD(1), ESP(2), ROLL(3), FFD5(4), FFD106_STEP1(5), FFD106_STEP2(5), FFD106_STEP3(5); 
    unsigned int size;
    unsigned char data[800];
};

struct hispfd_reg_s {
    unsigned int index;
    unsigned int offset;
    unsigned int value;
};

struct hispfd_pre_cfg_s{
    unsigned int pic_type;  // enum HISPFD_PIC_TYPE
    data_type_enum dtype;
    
    unsigned int W;         // 640
    unsigned int H;         // 480
    unsigned int w_in;      // 130
    unsigned int h_in;      // 186
    unsigned int w_out;     // 60
    unsigned int h_out;     // 60
    unsigned int l_pad;     // 0
    unsigned int r_pad;     // 0
    unsigned int rgb_pad;   // 0
    unsigned int X0;        // 0
    unsigned int Y0;
    unsigned int prop_h;
    unsigned int prop_w;
    unsigned int scale_factor;
};

struct hisp_time_s{
    u64 time_max;
    u64 time_min;
    u64 avrg;
    u32 num;
    u32 index;
    u64 time[100];
};

struct hispfd_start_st {
    unsigned int reg[4];   // baseaddr0/baseaddr1/reserve1/reserve2
    struct hispfd_pre_cfg_s stpre;
    struct hispfd_control_s ctrl;
};


#define HISPFD_SMMUINIT         _IO(  'F', 0x2001)
#define HISPFD_REQUEST          _IOWR('F', 0x2002, struct hispfd_control_s)
#define HISPFD_BASEADDR         _IOW( 'F', 0x2003, unsigned int)
#ifdef HIFD_REG_DEBUG
#define HISPFD_WR_REG           _IOW( 'F', 0x2004, struct hispfd_reg_s)
#define HISPFD_RD_REG           _IOWR('F', 0x2005, struct hispfd_reg_s)
#endif
#define HISPFD_SWITCH           _IO(  'F', 0x2006)
#define HISPFD_PRE_CFG          _IOW( 'F', 0x2007, struct hispfd_pre_cfg_s)
#define HISPFD_GET_TIME         _IOR( 'F', 0x2008, struct hisp_time_s)
#ifdef HIFD_REG_DEBUG
#define HISPFD_SEM_P            _IO(  'F', 0x200A)
#define HISPFD_SEM_V            _IO(  'F', 0x200B)
#endif
#define HISPFD_CS_4ADDR         _IOW( 'F', 0x200C, struct hisp_4baseaddr)
#define HISPFD_CLK_SETRATE      _IOW( 'F', 0x200E, unsigned int)
#define HISPFD_START            _IOW( 'F', 0x200F, struct hispfd_start_st)

#define DTS_NAME_HISPFD "hisilicon,hisp-hfd"
#define HISP_MIN(a, b) (((a) < (b)) ? (a) : (b))


extern int hispfd_atfd_smmuenable(u64 pteaddr);
extern int hispfd_atfd_hfdpwrup(void);
extern int hispfd_atfd_hfdpwrdn(void);

extern int hispfp_smmu_enable(void __iomem *smmuaddr, void __iomem *mmumasteraddr, unsigned long long pteaddr);


#endif /* __HISPFD_H__ */
