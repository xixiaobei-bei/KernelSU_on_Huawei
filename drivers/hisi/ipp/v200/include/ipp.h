

#ifndef _IPP_H_
#define _IPP_H_

#include <linux/types.h>
#include <linux/stddef.h>

#define CPE_FW_OK    (0)
#define CPE_FW_ERR  (-1)
#define CVDR_ALIGN_BYTES            16  //bytes
#define MAX_STRIPE_NUM              16
#define CPE_CLK    (0)

enum HISP_CPE_REG_TYPE {
    CPE_TOP        = 0,
    CMDLIST_REG    = 1,       
    CVDR_REG       = 2,
    MCF_REG        = 3,
    MFNR_REG       = 4,
    SLAM_REG       = 5,
    SMMU_REG       = 6,
    MAX_HISP_CPE_REG
};

#define CPE_IRQ0_REG0_OFFSET 0x260
#define CPE_IRQ0_REG4_OFFSET 0x270
#define CPE_IRQ1_REG0_OFFSET 0x274
#define CPE_IRQ1_REG4_OFFSET 0x284
#define CPE_COMP_IRQ_REG_OFFSET 0x288

#define CROP_DEFAULT_VALUE 0x800

void hispcpe_reg_set(unsigned int mode, unsigned int offset, unsigned int value);
unsigned int hispcpe_reg_get(unsigned int mode, unsigned int offset);

#define DEBUG_BIT   (1 << 2)
#define INFO_BIT    (1 << 1)
#define ERROR_BIT   (1 << 0)

extern int kmsgcat_mask;
/*lint +e21 +e846 +e514 +e778 +e866 +e84*/
#define D(fmt, args...) \
    do { \
        if (kmsgcat_mask & DEBUG_BIT) \
            printk("[ispcpe][%s] " fmt, __func__, ##args); \
    } while (0)
#define I(fmt, args...) \
    do { \
        if (kmsgcat_mask & INFO_BIT) \
            printk("[ispcpe][%s] " fmt, __func__, ##args); \
    } while (0)
#define E(fmt, args...) \
    do { \
        if (kmsgcat_mask & ERROR_BIT) \
            printk("[ispcpe][%s] " fmt, __func__, ##args); \
    } while (0)

#ifndef ALIGN_DOWN
#define ALIGN_DOWN(val, al)  ((unsigned int)(val) & ~((al) - 1))
#endif
#ifndef ALIGN_UP
#define ALIGN_UP(val, al)    (((unsigned int)(val) + ((al) - 1)) & ~((al) - 1))
#endif
#ifndef MAX
#define MAX(x,y)             ((x) > (y) ? (x) : (y))
#endif

#define loge_if(x) \
    if(x)  \
    {\
        pr_err("'%s' failed", #x); \
    }

#define loge_if_ret(x) \
   {\
      if (x) \
      {\
          pr_err("'%s' failed", #x); \
          return 1; \
      } \
   }

typedef enum
{
    PIXEL_FMT_CPE_WARP_UVC = 0,
    PIXEL_FMT_CPE_Y8 = 1,
    PIXEL_FMT_CPE_PACKED_UV = 2,
    PIXEL_FMT_CPE_C8 = 3,
    PIXEL_FMT_CPE_A_B = 4,
    PIXEL_FMT_CPE_LF_HF = 5,
    PIXEL_FMT_CPE_RM_PACKED_YUV = 6,
    PIXEL_FMT_CPE_RM_PACKED_Y = 7,
    PIXEL_FMT_CPE_G8 = 8,
    PIXEL_FMT_CPE_PACKED_YUV = 9,
    PIXEL_FMT_CPE_S8 = 10,
    PIXEL_FMT_CPE_SLAM_IN = 11,
    PIXEL_FMT_CPE_SLAM_OUT = 12,
    PIXEL_FMT_CPE_SLAM_STAT = 13,
    PIXEL_FMT_CPE_SLAM_SCORE = 14,
    PIXEL_FMT_CPE_MAX,
} pix_format_e;

typedef struct _cpe_size_t
{
    unsigned int width;
    unsigned int height;
    unsigned int stride;
    unsigned int buf;
    pix_format_e format;
} cpe_stream_t;

#define JPG_SUBSYS_BASE_ADDR                    (0xE8300000)

#define JPG_TOP_OFFSET (0x00004000)
#define JPG_CMDLST_OFFSET (0x00005000)
#define JPG_CVDR_OFFSET (0x00006000)
#define JPG_MCF_OFFSET (0x0000A000)
#define JPG_MFNR_OFFSET (0x0000B000)
#define JPG_SLAM_OFFSET (0x0000C000)

#define JPG_TOP_ADDR                    (JPG_SUBSYS_BASE_ADDR + JPG_TOP_OFFSET)
#define JPG_CMDLST_ADDR                    (JPG_SUBSYS_BASE_ADDR + JPG_CMDLST_OFFSET)
#define JPG_CVDR_ADDR                    (JPG_SUBSYS_BASE_ADDR + JPG_CVDR_OFFSET)
#define JPG_MCF_ADDR                    (JPG_SUBSYS_BASE_ADDR + JPG_MCF_OFFSET)
#define JPG_MFNR_ADDR                    (JPG_SUBSYS_BASE_ADDR + JPG_MFNR_OFFSET)
#define JPG_SLAM_ADDR                    (JPG_SUBSYS_BASE_ADDR + JPG_SLAM_OFFSET)

void *hisi_fstcma_alloc(dma_addr_t *dma_handle, size_t size, gfp_t flag);
void hisi_fstcma_free(void *va, dma_addr_t dma_handle, size_t size);

enum mem_attr_type_e {
    MMU_READ    = 0,
    MMU_WRITE   = 1,
    MMU_EXEC    = 2,
    MMU_SEC     = 3,
    MMU_CACHE   = 4,
    MMU_DEVICE  = 5,
    MMU_INV     = 6,
    MAX_MMUATTR
};

#define MEM_ATTR_READ   (0x1 << MMU_READ)
#define MEM_ATTR_WRITE  (0x1 << MMU_WRITE)
#define MEM_ATTR_EXEC   (0x1 << MMU_EXEC)
#define MEM_ATTR_SEC    (0x1 << MMU_SEC)
#define MEM_ATTR_CACHE  (0x1 << MMU_CACHE)
#define MEM_ATTR_DEVICE (0x1 << MMU_DEVICE)
#define MEM_ATTR_INV    (0x1 << MMU_INV)

#define HIPP_BL31_OFFSET    (0x00000000)
#define HIPP_BL31_SIZE      (0x00004000)
#define HIPP_SGPOOL_OFFSET  (HIPP_BL31_OFFSET + HIPP_BL31_SIZE)
#define HIPP_SGPOOL_SIZE    (0x00004000)
#define HIPP_SHAREDMEM_SIZE (HIPP_BL31_SIZE + HIPP_SGPOOL_SIZE)
#define HIPP_SECDDR_MAX    (0x00100000)
struct hipp_mapping_s {
    unsigned long daddr;
    unsigned int size;
    unsigned int prot;
    unsigned int offset;
    unsigned int reserve;
    unsigned long sglpaddr;
    unsigned int sglnum;
    void *kva;
};

enum hipp_mapping_type_e {
    HIPP_DYNAMIC_MAPPING = 0,
    MAX_HIPP_MAPPING
};

struct hipp_smem_s {
    unsigned long pte_nontrus;
    unsigned long pte_trusted;
    struct hipp_mapping_s mapping[MAX_HIPP_MAPPING];
};

int hipp_smmu_nontrus_init(void __iomem *smmuaddr, unsigned long long pteaddr);
int atfhipp_smmu_enable(unsigned int mode);
int atfhipp_smmu_disable(void);
int atfhipp_smmu_smrx(unsigned int sid, unsigned int mode);
int atfhipp_orb_init(void);
int atfhipp_orb_deinit(void);
int atfhipp_orb_request(void);
int atfhipp_trusted_map(void);
int atfhipp_trusted_unmap(void);
int atfhipp_trusted_smem(u64 addr);
int atfhipp_ptable_init(void);
int atfhipp_ptable_exit(void);
int hipp_ptable_secmem_init(void);
int hipp_ptable_secmem_deinit(void);


#endif /* _IPP_H_ */

/*************************************** END *******************************************/

