

#ifndef _IPP_CS_H_
#define _IPP_CS_H_

#include <linux/types.h>
#include <linux/stddef.h>

#define CPE_FW_OK	 (0)
#define CPE_FW_ERR	(-1)
#define CVDR_ALIGN_BYTES			(16)  //bytes
#define MAX_CPE_STRIPE_NUM			(8)
#define CPE_CLK    (0)

#define CPE_SIZE_MAX    (8192)

enum HISP_CPE_REG_TYPE {
	CPE_TOP 	   = 0,
	CMDLIST_REG    = 1,
	CVDR_REG	   = 2,
	VBK_REG 	   = 3,
	MCF_REG 	   = 4,
	MFNR_REG	   = 5,
	SLAM_REG	   = 6,
	REORDER_REG    = 7,
	COMPARE_REG    = 8,
	SMMU_REG	   = 9,
	NOC_ISP 	   = 10,
	MAX_HISP_CPE_REG
};

enum HISP_CPE_CLK_TYPE {
	CLK_TURBO_RATE   = 600000000,//600MHz-0.8V
	CLK_NORMAL_RATE  = 415000000,//415M-0.7V
	CLK_SVS_RATE     = 332000000,//332M-0.65V
	CLK_LOW_SVS_RATE = 238000000,//238M-0.6V
	MAX_CPE_CLK
};

enum DENV_CPE_CLK_TYPE {
	DENV_CLK_TURBO_RATE   = 600000000,//600MHz-0.8V
	DENV_CLK_NORMAL_RATE  = 406000000,//406M-0.7V
	DENV_CLK_SVS_RATE     = 325000000,//325M-0.65V
	DENV_CLK_LOW_SVS_RATE = 232000000,//232M-0.6V
};

enum HISP_CPE_IRQ_TYPE {
	CPE_IRQ_0   = 0, //ACPU
	MAX_HISP_CPE_IRQ
};

enum hipp_fd_type_e {
	BI_SHARE_FD    = 0,
	STAT_SHARE_FD  = 1,
	MAX_HIPP_TYPE_ORB
};

#define CPE_IRQ0_REG0_OFFSET 0x260
#define CPE_IRQ0_REG4_OFFSET 0x270
#define CPE_IRQ1_REG0_OFFSET 0x274
#define CPE_IRQ1_REG4_OFFSET 0x284
#define CPE_COMP_IRQ_REG_OFFSET 0x288

#define CROP_DEFAULT_VALUE 0x800

void hispcpe_reg_set(unsigned int mode, unsigned int offset, unsigned int value);
unsigned int hispcpe_reg_get(unsigned int mode, unsigned int offset);

#define DEBUG_BIT	(1 << 2)
#define INFO_BIT	(1 << 1)
#define ERROR_BIT	(1 << 0)

extern unsigned int kmsgcat_mask;

#define FLAG_LOG_DEBUG  (0)

/*lint +e21 +e846 +e514 +e778 +e866 +e84*/
#define d(fmt, args...) do { \
	if (kmsgcat_mask & DEBUG_BIT) printk("[ispcpe][%s] \n" fmt, __func__, ##args); \
} while (0)
#define i(fmt, args...) do { \
	if (kmsgcat_mask & INFO_BIT) printk("[ispcpe][%s] \n" fmt, __func__, ##args); \
} while (0)
#define e(fmt, args...) do { \
	if (kmsgcat_mask & ERROR_BIT) printk("[ispcpe][%s] \n" fmt, __func__, ##args); \
} while (0)

#ifndef ipp_align_down
#define ipp_align_down(val, al)  ((unsigned int)(val) & ~((al) - 1))
#endif
#ifndef ipp_align_up
#define ipp_align_up(val, al)	 (((unsigned int)(val) + ((al) - 1)) & ~((al) - 1))
#endif

#define loge_if(x) \
	if(x)  \
	{\
		pr_err("'%s' failed", #x); \
	}

#define loge_if_ret(x) \
	if (x) {\
		pr_err("'%s' failed", #x); \
		return 1; \
	}

typedef enum {
	PIXEL_FMT_CPE_Y8   = 0,
	PIXEL_FMT_CPE_1PF8 = 1,
	PIXEL_FMT_CPE_2PF8 = 2,
	PIXEL_FMT_CPE_3PF8 = 3,
	PIXEL_FMT_CPE_D32  = 4,
	PIXEL_FMT_CPE_D48  = 5,
	PIXEL_FMT_CPE_D64  = 6,
	PIXEL_FMT_CPE_D128 = 7,
	PIXEL_FMT_CPE_MAX,
} pix_format_e;

typedef struct _cpe_size_t {
	unsigned int width;
	unsigned int height;
	unsigned int stride;
	unsigned int buf;
	pix_format_e format;
} cpe_stream_t;

enum {
	IPP_PHOENIX = 0,
	IPP_DENVER = 1,
	IPP_VER_MAX
};

#define IPP_PCTRL_PERI_STAT_ADDR    (0x000000BC)
#define IPP_PCTRL_PERI_FLAG         (1 << 4)/*  BIT 4 : ISP   */
#define IPP_PCTRL_PERI_SIZE         (0x1000)/*  BIT 4 : ISP   */

#define JPG_SUBSYS_BASE_ADDR   (0xE8000000)

#define JPG_TOP_OFFSET	  (0x00004000)
#define JPG_CMDLST_OFFSET (0x00005000)
#define JPG_CVDR_OFFSET   (0x00006000)
#define JPG_VBK_OFFSET	  (0x00009000)
#define JPG_MCF_OFFSET	  (0x0000A000)
#define JPG_MFNR_OFFSET   (0x0000B000)
#define JPG_SLAM_OFFSET   (0x0000C000)
#define JPG_REORDER_OFFSET	 (0x0000D000)
#define JPG_COMPARE_OFFSET	 (0x0000E000)

#define JPG_TOP_ADDR	 (JPG_SUBSYS_BASE_ADDR + JPG_TOP_OFFSET)
#define JPG_CMDLST_ADDR  (JPG_SUBSYS_BASE_ADDR + JPG_CMDLST_OFFSET)
#define JPG_CVDR_ADDR	 (JPG_SUBSYS_BASE_ADDR + JPG_CVDR_OFFSET)
#define JPG_VBK_ADDR	 (JPG_SUBSYS_BASE_ADDR + JPG_VBK_OFFSET)
#define JPG_MCF_ADDR	 (JPG_SUBSYS_BASE_ADDR + JPG_MCF_OFFSET)
#define JPG_MFNR_ADDR	 (JPG_SUBSYS_BASE_ADDR + JPG_MFNR_OFFSET)
#define JPG_SLAM_ADDR	 (JPG_SUBSYS_BASE_ADDR + JPG_SLAM_OFFSET)
#define JPG_REORDER_ADDR	(JPG_SUBSYS_BASE_ADDR + JPG_REORDER_OFFSET)
#define JPG_COMPARE_ADDR	(JPG_SUBSYS_BASE_ADDR + JPG_COMPARE_OFFSET)

void *isp_fstcma_alloc(dma_addr_t *dma_handle, size_t size, gfp_t flag);
void isp_fstcma_free(void *va, dma_addr_t dma_handle, size_t size);

#define HIPP_SHAREDMEM_SIZE (0x1000)
#define HIPP_SECDDR_MAX    (0x00100000)

int atfhipp_smmu_enable(unsigned int mode);
int atfhipp_smmu_disable(void);
int atfhipp_smmu_smrx(unsigned int sid, unsigned int mode);
u64 hipp_smmu_err_addr(void);

#endif /* _IPP_CS_H_ */

/*************************************** END *******************************************/

