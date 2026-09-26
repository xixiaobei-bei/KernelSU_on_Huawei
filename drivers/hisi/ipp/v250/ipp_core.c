/*
 * Hisi ISP CPE
 *
 * Copyright (c) 2017 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*lint -e598 -e648
-esym(598,*)  -esym(648,*)*/

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <linux/of_irq.h>
#include <linux/iommu.h>
#include <linux/pm_wakeup.h>
#include <linux/ion.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/genalloc.h>
#include <linux/hisi-iommu.h>
#include <linux/version.h>
#include <linux/hisi/hipp.h>
#include "mcf.h"
#include "vbk.h"
#include "slam.h"
#include "reorder.h"
#include "compare.h"
#include "ipp.h"
#include "sub_ctrl_drv_priv.h"
#include "sub_ctrl_reg_offset.h"
#include "memory.h"
#include <media/camera/jpeg/jpeg_base.h>
#include <linux/delay.h>
#include "soc_acpu_baseaddr_interface.h"

#define SLAM_BI_SID     (30)
#define SLAM_STAT_SID   (32)
#define SLAM_RATE_INDEX_MAX (3)
#define CPE_RATE_INDEX_MAX (3)

#define SMMU_COMN_SMMU_CB_TTBR0_REG          0x204   /* SMMU Translation Table Base Register for Non-Secure Context Bank0Can be accessed in Non-Secure mode */
#define SMMU_COMN_SMMU_FAMA_CTRL1_NS_REG     0x224   /* SMMU Control Register for FAMA for TCU of Non-Secure Context Bank */
#define SMMU_COMN_SMMU_ADDR_MSB_REG          0x300   /* Register for MSB of all 33-bits address configuration */
#define SMMU_COMN_SMMU_ERR_RDADDR_REG        0x304   /* SMMU Error Address of TLB miss for Read transaction */
#define SMMU_COMN_SMMU_ERR_WRADDR_REG        0x308   /* SMMU Error Address of TLB miss for Write transaction */
#define SMMU_FAMA_MASK                       0x7F
#define SMMU_REG_BIT                         32
#define SMMU_MSB_ERR_MASK                    0x3FFF
#define SMMU_MSB_ERRWR_OFFSET                7

unsigned int kmsgcat_mask = (INFO_BIT | ERROR_BIT);
/*lint -e529 -e578 -e514 -e778 -e866 -e84 -e429 -e613 -e668*/

unsigned int  clk_rate_map[4] = {
	[0] = CLK_TURBO_RATE,
	[1] = CLK_NORMAL_RATE,
	[2] = CLK_SVS_RATE,
	[3] = CLK_LOW_SVS_RATE,
};

enum hipp_refs_type_e {
	REFS_TYPE_MCF = 0,
	REFS_TYP_SLAM = 1,
	REFS_TYP_VBK = 2,
	REFS_TYP_MATCHER = 3,
	REFS_TYP_MAX
};

struct hispcpe_s {
	struct miscdevice miscdev;
	struct platform_device *pdev;
	wait_queue_head_t cpe_wait;
	wait_queue_head_t slam_wait;
	wait_queue_head_t reorder_wait;
	wait_queue_head_t compare_wait;
	int cpe_ready;
	int slam_ready;
	int reorder_ready;
	int compare_ready;
	int initialized;
	unsigned int  sec_path;
	unsigned int  sec_req;
	unsigned int open_refs;
	struct regulator *cpe_supply;
	atomic_t cpe_supply_refs;
	unsigned int  hipp_refs[REFS_TYP_MAX];
	atomic_t mapbuf_ready;
	struct mutex ipp_work_lock;

	struct mutex lock;
	unsigned int irq_num;
	unsigned int reg_num;
	unsigned int curr_slam_rate_value;
	unsigned int curr_cpe_rate_value;
	unsigned int ipp_set_rate_value;
	int irq[MAX_HISP_CPE_IRQ];
	struct resource *r[MAX_HISP_CPE_REG];
	void __iomem *reg[MAX_HISP_CPE_REG];
	struct iommu_domain *domain;
	struct hipp_common_s *ippdrv;
	struct hipp_common_s *sec_slamdrv;
	struct hipp_common_s *rdrdrv;
	struct hipp_common_s *cmpdrv;
	struct dma_buf *devbuf;
	struct dma_buf *modulebuf;

	unsigned int daddr;
	void *virt_addr;
	unsigned int jpg_clk_rate;
	unsigned int jpgclk_low_frequency;
	unsigned int jpgclk_off_frequency;
	struct clk *jpg_clk;
	struct wakeup_source ipp_wakelock;
	struct mutex ipp_wakelock_mutex;
	unsigned int version;
	struct mutex dev_lock;
};

struct hispcpe_s *hispcpe_dev = NULL;

struct pw_memory_s {
	int shared_fd;
	int size;
	unsigned long prot;
};
struct memory_block_s {
	int shared_fd;
	int size;
	unsigned long prot;
	unsigned int da;
	int usage;
	void *viraddr;
};

struct power_para_s {
	unsigned int pw_flag;
	struct pw_memory_s mem;
};

struct hispcpe_powerup_param_t {
	unsigned long source_rate;
	unsigned long dest_rate0;
	unsigned long transition_rate0;
	unsigned long dest_rate1;
	unsigned long transition_rate1;
	unsigned long dest_rate0_denve;
	unsigned long dest_rate1_denve;
	unsigned long dest_rate2_denve;
	unsigned long dest_rate3_denve;
	unsigned long trans_rate_denve;
};

#define HISP_CPE_PWRUP          _IOWR('C', 0x1001, int)
#define HISP_CPE_PWRDN          _IOWR('C', 0x1002, int)
#define HISP_CPE_MCF_REQ        _IOWR('C', 0x2001, int)
#define HISP_CPE_MFNR_REQ       _IOWR('C', 0x2002, int)
#define HISP_CPE_MAP_BUF        _IOWR('C', 0x2003, int)
#define HISP_CPE_UNMAP_BUF      _IOWR('C', 0x2004, int)
#define HISP_SLAM_REQ           _IOWR('C', 0x2005, int)
#define HISP_CPE_VBK_REQ        _IOWR('C', 0x2009, int)
#define HISP_MATCHER_REQ        _IOWR('C', 0x200C, int)

#if FLAG_LOG_DEBUG
#define HISP_REORDER_REQ        _IOWR('C', 0x2007, int)
#define HISP_COMPARE_REQ        _IOWR('C', 0x2008, int)
#define HISP_CPE_MAP_IOMMU      _IOWR('C', 0x200A, int)
#define HISP_CPE_UNMAP_IOMMU    _IOWR('C', 0x200B, int)
#endif

#define DTS_NAME_HISI_IPP "hisilicon,ipp"
#define HISP_CPE_TIMEOUT_MS (2000)
#define hipp_min(a, b) (((a) < (b)) ? (a) : (b))

unsigned int g_share_fd_size[MAX_HIPP_TYPE_ORB] = { 0 };

static struct hispcpe_s *get_cpedev_bymisc(struct miscdevice *mdev)
{
	struct hispcpe_s *dev = NULL;

	if (mdev == NULL) {
		pr_err("[%s] Failed : mdev.%pK\n", __func__, mdev);
		return NULL;
	}

	dev = container_of(mdev, struct hispcpe_s, miscdev);
	return dev;
}

void hispcpe_reg_set(unsigned int mode, unsigned int offset, unsigned int value)
{
	struct hispcpe_s *dev = hispcpe_dev;
	void __iomem *reg_base = dev->reg[mode];
	writel(value, reg_base + offset);
}

unsigned int hispcpe_reg_get(unsigned int mode, unsigned int offset)
{
	struct hispcpe_s *dev = hispcpe_dev;
	unsigned int value = 0;
	void __iomem *reg_base = dev->reg[mode];

	if (mode == SMMU_REG) {
		e("Failed : mode.(0x%x = 0x%x)\n", mode, SMMU_REG);
		return 0;
	}

	value = readl(reg_base + offset);//lint !e529
	return value;
}

static irqreturn_t hispcpe_irq_handler(int irq, void *devid)
{
	struct hispcpe_s *dev = NULL;
	unsigned int value = 0;
	unsigned int irq_value = 0;
	d("+\n");

	if ((dev = (struct hispcpe_s *)devid) == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return IRQ_NONE;
	}

	if (irq == dev->irq[CPE_IRQ_0]) {
		value = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CMDLST_ACPU_IRQ_REG2_REG));//lint !e529
		pr_info("IPP_CMDLST_ACPU_IRQ VALUE = 0x%08x\n", value);

		if ((value == 0xdeadbeef) || (value == 0x0))
			value = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CMDLST_ACPU_IRQ_REG2_REG));//lint !e529

		writel((value >> 16), (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CMDLST_ACPU_IRQ_REG0_REG));

		if (value & (0x1 << CMP_CHANNEL)) {
			irq_value = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CMP_IRQ_REG2_REG));//lint !e529
			pr_info("CMP_IRQ VALUE = 0x%08x\n", irq_value);
			writel(0x1F, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CMP_IRQ_REG0_REG));
			dev->compare_ready = 1;
			wake_up_interruptible(&dev->compare_wait);
		}

		if (value & (0x1 << RDR_CHANNEL)) {
			irq_value = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_RDR_IRQ_REG2_REG));//lint !e529
			pr_info("RDR_IRQ VALUE = 0x%08x\n", irq_value);
			writel(0x1F, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_RDR_IRQ_REG0_REG));
			dev->reorder_ready = 1;
			wake_up_interruptible(&dev->reorder_wait);
		}

		if (value & (0x1 << SLAM_CHANNEL)) {
			irq_value = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_SLAM_IRQ_REG2_REG));//lint !e529
			pr_info("ORB_IRQ VALUE = 0x%08x\n", irq_value);
			writel(0x3FFF, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_SLAM_IRQ_REG0_REG));
			dev->slam_ready = 1;
			wake_up_interruptible(&dev->slam_wait);
		}

		if (value & (0x1 << CPE_CHANNEL)) {
			irq_value = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CPE_IRQ_REG4_REG));//lint !e529
			pr_info("CPE_IRQ RAW VALUE = 0x%08x\n", irq_value);
			writel(0x03FFFFFF, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CPE_IRQ_REG0_REG));
			dev->cpe_ready = 1;
			wake_up_interruptible(&dev->cpe_wait);
		}
	}

	d("-\n");
	return IRQ_HANDLED;
}

static void set_ipp_top_clk_on(struct hispcpe_s *dev)
{
	writel(0x00020001, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_DMA_CRG_CFG0_REG));
	return;
}

static int hispcpe_irq_config(struct hispcpe_s *dev)
{
	union u_cmdlst_acpu_irq_reg1 cfg_acpu;
	unsigned int cmdlst_ctrl_value = 0;
	d("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	//ACPU mask.
	cfg_acpu.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CMDLST_ACPU_IRQ_REG1_REG));//lint !e529
	cfg_acpu.bits.cmdlst_acpu_irq_mask = 0x00;
	writel(cfg_acpu.u32, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CMDLST_ACPU_IRQ_REG1_REG));
	cmdlst_ctrl_value = ((0x3 << 0) | (0x3 << 3) | (0x2 << 6) | (0x2 << 9) | (0x1 << 12) | (0x1 << 15) |
						 (0x0 << 18) | (0x0 << 21));
	writel(cmdlst_ctrl_value, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CMDLST_CTRL_REG));
	d("-\n");
	return 0;
}

static int ipp_setclk_enable(struct hispcpe_s *dev)
{
	int ret;

	if ((ret = jpeg_ipp_set_rate(dev->jpg_clk, dev->jpgclk_low_frequency)) != 0)
		pr_err("[%s] Failed: jpeg_ipp_set_rate( %d).%d\n", __func__, dev->jpgclk_low_frequency, ret);

	if ((ret = jpeg_ipp_clk_prepare_enable(dev->jpg_clk)) != 0) {
		pr_err("[%s] Failed: jpeg_ipp_clk_prepare_enable.%d\n", __func__, ret);
		return ret;
	}

	return ret;
}

static int ipp_setclk_disable(struct hispcpe_s *dev)
{
	int ret = 0;

	if ((ret = jpeg_ipp_set_rate(dev->jpg_clk, dev->jpgclk_off_frequency)) != 0) {
		pr_err("[%s] Failed: jpeg_ipp_set_rate.%d\n", __func__, ret);
		return ret;
	}

	jpeg_ipp_clk_disable_unprepare(dev->jpg_clk);
	return 0;
}

int ipp_cfg_qos_reg(struct hispcpe_s *dev)
{
	if (dev->version == IPP_PHOENIX) {
		writel(0x00000002, (volatile void __iomem *)(dev->reg[NOC_ISP] + 0x30C));
		writel(0x00000002, (volatile void __iomem *)(dev->reg[NOC_ISP] + 0x38C));
	}

	writel(0x00000000, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_JPG_FLUX_CTRL0_0_REG));
	writel(0x08000105, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_JPG_FLUX_CTRL0_1_REG));
	writel(0x00000000, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_JPG_FLUX_CTRL1_0_REG));
	writel(0x08000085, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_JPG_FLUX_CTRL1_1_REG));
	return 0;
}

int hispcpe_smmu_nontrus_init(void)
{
	unsigned int offset = 0;
	unsigned int value = 0;
	struct device *device = NULL;
	unsigned long pgd_base = 0;
	struct hispcpe_s *dev = hispcpe_dev;
	u64 smmu_err_addr = 0;
	i("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	if (dev->pdev == NULL) {
		e("Failed : pdev.%pK\n", dev->pdev);
		return -EINVAL;
	}

	if (dev->reg[SMMU_REG] == NULL) {
		e("Failed : SMMU_REG NULL\n");
		return -EINVAL;
	}

	device = &dev->pdev->dev;
	pgd_base = hisi_domain_get_ttbr(device);

	if (pgd_base == 0) {
		e("Failed : hisi_domain_get_ttbr\n");
		return -EINVAL;
	}

	/* SMMU_CB_TTBR0 */
	offset = SMMU_COMN_SMMU_CB_TTBR0_REG;
	value = (unsigned int)pgd_base;
	writel(value, (volatile void __iomem *)(dev->reg[SMMU_REG] + offset));
	/* SMMU_FAMA_CTRL1_NS */
	offset = SMMU_COMN_SMMU_FAMA_CTRL1_NS_REG;
	value = readl((volatile void __iomem *)(dev->reg[SMMU_REG] + offset));//lint !e529
	value &= ~SMMU_FAMA_MASK;
	value |= (pgd_base >> SMMU_REG_BIT) & SMMU_FAMA_MASK;
	writel(value, (volatile void __iomem *)(dev->reg[SMMU_REG] + offset));

	/* SMMU_ERR_ADDR_NS */
	smmu_err_addr = hipp_smmu_err_addr();
	offset = SMMU_COMN_SMMU_ERR_RDADDR_REG;
	writel((unsigned int)smmu_err_addr, (volatile void __iomem *)(dev->reg[SMMU_REG] + offset));
	offset = SMMU_COMN_SMMU_ERR_WRADDR_REG;
	writel((unsigned int)smmu_err_addr, (volatile void __iomem *)(dev->reg[SMMU_REG] + offset));
	offset = SMMU_COMN_SMMU_ADDR_MSB_REG;
	value = readl((volatile void __iomem *)(dev->reg[SMMU_REG] + offset));
	value &= ~SMMU_MSB_ERR_MASK;
	value |= (smmu_err_addr >> SMMU_REG_BIT) |
		((smmu_err_addr >> SMMU_REG_BIT) << SMMU_MSB_ERRWR_OFFSET);
	writel(value, (volatile void __iomem *)(dev->reg[SMMU_REG] + offset));

	return 0;
}

static int hispcpe_set_transition_rate(struct hispcpe_s *dev, struct hispcpe_powerup_param_t *param, int *ret)
{
	if (dev->version == IPP_PHOENIX) {
		if ((*ret = jpeg_ipp_set_transition_rate(param->source_rate, param->dest_rate0,
					param->transition_rate0)) != true) {
			pr_err("[%s] Failed: jpeg_ipp_set_transition_rate0.%d\n", __func__, *ret);
			return -1;
		}

		if ((*ret = jpeg_ipp_set_transition_rate(param->source_rate, param->dest_rate1,
					param->transition_rate1)) != true) {
			pr_err("[%s] Failed: jpeg_ipp_set_transition_rate1.%d\n", __func__, *ret);
			return -1;
		}
	} else if (dev->version == IPP_DENVER) {
		if ((*ret = jpeg_ipp_set_transition_rate(param->source_rate, param->dest_rate0_denve,
					param->trans_rate_denve)) != true) {
			pr_err("[%s] Failed: rate0.%d\n", __func__, *ret);
			return -1;
		}

		if ((*ret = jpeg_ipp_set_transition_rate(param->source_rate, param->dest_rate1_denve,
					param->trans_rate_denve)) != true) {
			pr_err("[%s] Failed: rate1.%d\n", __func__, *ret);
			return -1;
		}

		if ((*ret = jpeg_ipp_set_transition_rate(param->source_rate, param->dest_rate2_denve,
					param->trans_rate_denve)) != true) {
			pr_err("[%s] Failed: rate2.%d\n", __func__, *ret);
			return -1;
		}

		if ((*ret = jpeg_ipp_set_transition_rate(param->source_rate, param->dest_rate3_denve,
					param->trans_rate_denve)) != true) {
			pr_err("[%s] Failed: rate3.%d\n", __func__, *ret);
			return -1;
		}
	}

	return 0;
}

static int hispcpe_powerup_config(struct hispcpe_s *dev, int *ret)
{
	if ((*ret = hispcpe_irq_config(dev)) != 0) {
		pr_err("[%s] Failed: hispcpe_irq_config.%d\n", __func__, *ret);
		return -1;
	}

	set_ipp_top_clk_on(dev);

	if (dev->ippdrv->enable_smmu == NULL) {
		pr_err("[%s] Failed : ippdrv->enable_smmu.%pK\n", __func__, dev->ippdrv->enable_smmu);
		*ret = -ENOMEM;
		return -1;
	}

	if ((*ret = dev->ippdrv->enable_smmu(dev->ippdrv)) != 0) {
		pr_err("[%s] Failed : dev->ippdrv->enable_smmu.%d\n", __func__, *ret);
		return -1;
	}

	if ((*ret = ipp_cfg_qos_reg(dev)) != 0) {
		pr_err("[%s] Failed : ipp_cfg_qos_reg.\n", __func__);
		return -1;
	}

	return 0;
}

int hispcpe_powerup(struct hispcpe_s *dev)
{
	int ret = 0, err = 0;
	struct hispcpe_powerup_param_t param;
	param.source_rate      = 600000000;
	param.dest_rate0       = 238000000;
	param.transition_rate0 = 240000000;
	param.dest_rate1       = 415000000;
	param.transition_rate1 = 400000000;
	param.dest_rate0_denve = 406000000;
	param.dest_rate1_denve = 325000000;
	param.dest_rate2_denve = 232000000;
	param.dest_rate3_denve = 102000000;
	param.trans_rate_denve = 172000000;
	d("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	if (atomic_read(&dev->cpe_supply_refs) > 0) {
		pr_err("[%s] Failed: cpe_supply Opened.0x%x\n", __func__, atomic_read(&dev->cpe_supply_refs));
		return -EBUSY;
	}

	if ((ret = regulator_enable(dev->cpe_supply)) != 0) {
		pr_err("[%s] Failed: cpe regulator_enable.%d\n", __func__, ret);
		return ret;
	}

	if (hispcpe_set_transition_rate(dev, &param, &ret) != 0)
		goto fail_enjgpclk;

	if ((ret = ipp_setclk_enable(dev)) != 0) {
		pr_err("[%s] Failed: ipp_setclk_enable.%d\n", __func__, ret);
		goto fail_enjgpclk;
	}

	dev->curr_slam_rate_value = dev->jpgclk_low_frequency;
	dev->curr_cpe_rate_value = dev->jpgclk_low_frequency;
	dev->ipp_set_rate_value = dev->jpgclk_low_frequency;

	if (hispcpe_powerup_config(dev, &ret) != 0)
		goto fail_irq;

	atomic_set(&dev->cpe_supply_refs, 1);
	d("-\n");
	return ret;
fail_irq:

	if ((err = ipp_setclk_disable(dev)) != 0)
		pr_err("[%s] Failed : ipp_setclk_disable.%d\n", __func__, err);

fail_enjgpclk:

	if ((err = regulator_disable(dev->cpe_supply)) != 0)
		pr_err("[%s] Failed: cpe regulator_disable.%d\n", __func__, err);

	return ret;
}

int hispcpe_powerdn(struct hispcpe_s *dev)
{
	int ret = 0;
	d("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	mutex_lock(&dev->ipp_work_lock);
	if (dev->hipp_refs[REFS_TYPE_MCF] > 0) {
		pr_err("[%s] : mcf refs Opened .\n", __func__);
		mutex_unlock(&dev->ipp_work_lock);
		return -ENOMEM;
	}
	if (dev->hipp_refs[REFS_TYP_SLAM] > 0) {
		pr_err("[%s] : slam refs Opened .\n", __func__);
		mutex_unlock(&dev->ipp_work_lock);
		return -ENOMEM;
	}
	if (dev->hipp_refs[REFS_TYP_VBK] > 0) {
		pr_err("[%s] : vbk refs Opened .\n", __func__);
		mutex_unlock(&dev->ipp_work_lock);
		return -ENOMEM;
	}
	if (dev->hipp_refs[REFS_TYP_MATCHER] > 0) {
		pr_err("[%s] : matcher refs Opened .\n", __func__);
		mutex_unlock(&dev->ipp_work_lock);
		return -ENOMEM;
	}
	mutex_unlock(&dev->ipp_work_lock);

	if (atomic_read(&dev->cpe_supply_refs) <= 0) {
		pr_err("[%s] Failed: cpe_supply not Opened.0x%x\n", __func__, atomic_read(&dev->cpe_supply_refs));
		return -EINVAL;
	}

	atomic_set(&dev->cpe_supply_refs, 0);

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	if (dev->ippdrv == NULL) {
		pr_err("[%s] Failed : ippdrv.%pK\n", __func__, dev->ippdrv);
		return -ENOMEM;
	}

	if (dev->ippdrv->disable_smmu == NULL) {
		pr_err("[%s] Failed : ippdrv->disable_smmu.%pK\n", __func__, dev->ippdrv->disable_smmu);
		return -ENOMEM;
	}

	if ((ret = dev->ippdrv->disable_smmu(dev->ippdrv)) != 0) {
		pr_err("[%s] Failed : dev->ippdrv->disable_smmu.%d\n", __func__, ret);
		return ret;
	}

	if ((ret = ipp_setclk_disable(dev)) != 0) {
		pr_err("[%s] Failed: ipp_setclk_disable.%d\n", __func__, ret);
		return ret;
	}

	if ((ret = regulator_disable(dev->cpe_supply)) != 0)
		pr_err("[%s] Failed: cpe regulator_disable.%d\n", __func__, ret);

	jpeg_ipp_clear_transition_rate();
	d("-\n");
	return 0;
}

static int hispcpe_cpe_clk_enable(struct hispcpe_s *dev)
{
	union u_cpe_crg_cfg0 cfg;
	d("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CPE_CRG_CFG0_REG));//lint !e529
	cfg.bits.mcf_clken  = 1;
	cfg.bits.vbk_clken  = 1;
	writel(cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CPE_CRG_CFG0_REG));

	d("-\n");
	return 0;
}

static int hispcpe_cpe_clk_disable(struct hispcpe_s *dev)
{
	union u_cpe_crg_cfg0 cfg;
	d("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CPE_CRG_CFG0_REG));//lint !e529
	cfg.bits.mcf_clken  = 0;
	cfg.bits.vbk_clken  = 0;
	writel(cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CPE_CRG_CFG0_REG));

	d("-\n");
	return 0;
}

static int hispcpe_slam_clk_check(struct hispcpe_s *dev)
{
	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	if (0 != dev->sec_path)
		pr_err("[%s] Failed: check sec_path\n", __func__);

	return 0;
}


static int hispcpe_slam_clk_enable(struct hispcpe_s *dev)
{
	union u_slam_crg_cfg0 cfg;
	d("+\n");

	if (hispcpe_slam_clk_check(dev)) {
		pr_err("[%s] Failed : hispcpe_slam_clk_check.%pK\n", __func__, dev);
		return -EINVAL;
	}

	if (0 != dev->sec_path)
		pr_err("[%s] Failed: check sec_path\n", __func__);

	cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_SLAM_CRG_CFG0_REG));//lint !e529
	cfg.bits.slam_clken = 1;
	writel(cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_SLAM_CRG_CFG0_REG));

	d("-\n");
	return 0;
}

static int hispcpe_slam_clk_disable(struct hispcpe_s *dev)
{
	union u_slam_crg_cfg0 cfg;
	d("+\n");

	if (hispcpe_slam_clk_check(dev)) {
		pr_err("[%s] Failed : hispcpe_slam_clk_check.%pK\n", __func__, dev);
		return -EINVAL;
	}

	if (0 != dev->sec_path)
		pr_err("[%s] Failed: check sec_path\n", __func__);

	cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_SLAM_CRG_CFG0_REG));//lint !e529
	cfg.bits.slam_clken = 0;
	writel(cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_SLAM_CRG_CFG0_REG));

	d("-\n");
	return 0;
}

#if FLAG_LOG_DEBUG
static int hispcpe_reorder_clk_enable(struct hispcpe_s *dev)
{
	union u_rdr_crg_cfg0 cfg;
	d("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_RDR_CRG_CFG0_REG));//lint !e529
	cfg.bits.rdr_clken  = 1;
	writel(cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_RDR_CRG_CFG0_REG));

	d("-\n");
	return 0;
}

static int hispcpe_reorder_clk_disable(struct hispcpe_s *dev)
{
	union u_rdr_crg_cfg0 cfg;
	d("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_RDR_CRG_CFG0_REG));//lint !e529
	cfg.bits.rdr_clken  = 0;
	writel(cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_RDR_CRG_CFG0_REG));

	d("-\n");
	return 0;
}

static int hispcpe_compare_clk_enable(struct hispcpe_s *dev)
{
	union u_cmp_crg_cfg0 cfg;
	d("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CMP_CRG_CFG0_REG));//lint !e529
	cfg.bits.cmp_clken  = 1;
	writel(cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CMP_CRG_CFG0_REG));

	d("-\n");
	return 0;
}

static int hispcpe_compare_clk_disable(struct hispcpe_s *dev)
{
	union u_cmp_crg_cfg0 cfg;
	d("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CMP_CRG_CFG0_REG));//lint !e529
	cfg.bits.cmp_clken  = 0;
	writel(cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CMP_CRG_CFG0_REG));

	d("-\n");
	return 0;
}
#endif

static int hispcpe_matcher_clk_enable(struct hispcpe_s *dev)
{
	union u_rdr_crg_cfg0 rdr_cfg;
	union u_cmp_crg_cfg0 cmp_cfg;
	d("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	rdr_cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_RDR_CRG_CFG0_REG));//lint !e529
	rdr_cfg.bits.rdr_clken  = 1;
	writel(rdr_cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_RDR_CRG_CFG0_REG));
	cmp_cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CMP_CRG_CFG0_REG));//lint !e529
	cmp_cfg.bits.cmp_clken  = 1;
	writel(cmp_cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CMP_CRG_CFG0_REG));

	d("-\n");
	return 0;
}

static int hispcpe_matcher_clk_disable(struct hispcpe_s *dev)
{
	union u_rdr_crg_cfg0 rdr_cfg;
	union u_cmp_crg_cfg0 cmp_cfg;
	d("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	rdr_cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_RDR_CRG_CFG0_REG));//lint !e529
	rdr_cfg.bits.rdr_clken  = 0;
	writel(rdr_cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_RDR_CRG_CFG0_REG));
	cmp_cfg.u32 = readl((volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CMP_CRG_CFG0_REG));//lint !e529
	cmp_cfg.bits.cmp_clken  = 0;
	writel(cmp_cfg.u32, (volatile void __iomem *)(dev->reg[CPE_TOP] + SUB_CTRL_CMP_CRG_CFG0_REG));

	d("-\n");
	return 0;
}


static int hispcpe_clean_wait_flag(struct hispcpe_s *dev, unsigned int wait_mode)
{
	d("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	if (wait_mode == 0)
		dev->cpe_ready = 0;
	else if (wait_mode == 1)
		dev->slam_ready = 0;
	else if (wait_mode == 2)
		dev->reorder_ready = 0;
	else if (wait_mode == 3)
		dev->compare_ready = 0;

	d("-\n");
	return 0;
}

static int hispcpe_do_wait_mode_0(struct hispcpe_s *dev, unsigned int wait_mode, int *ret, int *times)
{
	while (1) {
		*ret = wait_event_interruptible_timeout(dev->cpe_wait, dev->cpe_ready,
											   msecs_to_jiffies(HISP_CPE_TIMEOUT_MS));/*lint !e578 !e666*/

		if ((*ret == -ERESTARTSYS) && ((*times)++ < 200))
			msleep(5);
		else
			break;
	}

	return 0;
}

static int hispcpe_do_wait_mode(struct hispcpe_s *dev, unsigned int wait_mode, int *ret, int *times)
{
	if (wait_mode == 0) {
		hispcpe_do_wait_mode_0(dev, wait_mode, ret, times);
	} else if (wait_mode == 1) {
		while (1) {
			*ret = wait_event_interruptible_timeout(dev->slam_wait, dev->slam_ready,
												   msecs_to_jiffies(HISP_CPE_TIMEOUT_MS));/*lint !e578  !e666*/

			if ((*ret == -ERESTARTSYS) && ((*times)++ < 200))
				msleep(5);
			else
				break;
		}
	} else if (wait_mode == 2) {
		while (1) {
			*ret = wait_event_interruptible_timeout(dev->reorder_wait, dev->reorder_ready,
												   msecs_to_jiffies(HISP_CPE_TIMEOUT_MS));/*lint !e578  !e666*/

			if ((*ret == -ERESTARTSYS) && ((*times)++ < 200))
				msleep(5);
			else
				break;
		}
	} else if (wait_mode == 3) {
		while (1) {
			*ret = wait_event_interruptible_timeout(dev->compare_wait, dev->compare_ready,
												   msecs_to_jiffies(HISP_CPE_TIMEOUT_MS));/*lint !e578  !e666*/

			if ((*ret == -ERESTARTSYS) && ((*times)++ < 200))
				msleep(1);
			else
				break;
		}
	}


	return 0;
}

static int hispcpe_wait(struct hispcpe_s *dev, unsigned int wait_mode)
{
	int ret = 0;
	int times = 0;
	d("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	hispcpe_do_wait_mode(dev, wait_mode, &ret, &times);

	if (ret <= 0) {
		pr_err("[%s] Failed :wait_timeout, ret = %d, times = %d\n", __func__, ret, times);
		return CPE_FW_ERR;
	}

	d("-\n");
	return CPE_FW_OK;
}

int hispcpe_mcf_request(struct hispcpe_s *dev, msg_req_mcf_request_t *ctrl)
{
	int ret = 0;
	d("+\n");

	if (dev == NULL || ctrl == NULL || (ctrl->streams[BI_MONO].width > CPE_SIZE_MAX)) {
		pr_err("[%s] Failed : dev.%pK, ctrl.%pK, width.%d\n", __func__, dev, ctrl, ctrl->streams[BI_MONO].width);
		return -EINVAL;
	}

	if ((ret = hispcpe_clean_wait_flag(dev, 0)) < 0) {
		pr_err("[%s] Failed : hispcpe_clean_wait_flag.%d\n", __func__, ret);
		return ret;
	}

	mcf_request_handler(ctrl);

	if ((ret = hispcpe_wait(dev, 0)) < 0)
		pr_err("[%s] Failed : hispcpe_wait.%d\n", __func__, ret);

	ipp_eop_handler(CMD_EOF_CPE_MODE);
	d("-\n");
	return ret;
}

int hispcpe_vbk_request(struct hispcpe_s *dev, msg_req_vbk_request_t *ctrl)
{
	int ret = 0;
	d("+\n");

	if (dev == NULL || ctrl == NULL) {
		pr_err("[%s] Failed : dev.%pK, ctrl.%pK\n", __func__, dev, ctrl);
		return -EINVAL;
	}

	if ((ret = hispcpe_clean_wait_flag(dev, 0)) < 0) {
		pr_err("[%s] Failed : hispcpe_clean_wait_flag.%d\n", __func__, ret);
		return ret;
	}

	vbk_request_handler(ctrl);

	if ((ret = hispcpe_wait(dev, 0)) < 0)
		pr_err("[%s] Failed : hispcpe_wait.%d\n", __func__, ret);

	ipp_eop_handler(CMD_EOF_CPE_MODE);
	d("-\n");
	return ret;
}

int hispcpe_slam_request(struct hispcpe_s *dev, msg_req_slam_request_t *ctrl)
{
	int ret = 0;
	d("+\n");

	if (dev == NULL || ctrl == NULL) {
		pr_err("[%s] Failed : dev.%pK, ctrl.%pK\n", __func__, dev, ctrl);
		return -EINVAL;
	}

	if ((ret = hispcpe_clean_wait_flag(dev, 1)) < 0) {
		pr_err("[%s] Failed : hispcpe_clean_wait_flag.%d\n", __func__, ret);
		return ret;
	}

	slam_request_handler(ctrl);

	if ((ret = hispcpe_wait(dev, 1)) < 0)
		pr_err("[%s] Failed : hispcpe_wait.%d\n", __func__, ret);

	ipp_eop_handler(CMD_EOF_SLAM_MODE);
	d("-\n");
	return ret;
}

int hispcpe_reorder_request(struct hispcpe_s *dev, msg_req_reorder_request_t *ctrl)
{
	int ret = 0;
	d("+\n");

	if (dev == NULL || ctrl == NULL) {
		pr_err("[%s] Failed : dev.%pK, ctrl.%pK\n", __func__, dev, ctrl);
		return -EINVAL;
	}

	if ((ret = hispcpe_clean_wait_flag(dev, 2)) < 0) {
		pr_err("[%s] Failed : hispcpe_clean_wait_flag.%d\n", __func__, ret);
		return ret;
	}

	reorder_request_handler(ctrl);

	if ((ret = hispcpe_wait(dev, 2)) < 0)
		pr_err("[%s] Failed : hispcpe_wait.%d\n", __func__, ret);

	ipp_eop_handler(CMD_EOF_REORDER_MODE);
	d("-\n");
	return ret;
}

int hispcpe_compare_request(struct hispcpe_s *dev, msg_req_compare_request_t *ctrl)
{
	int ret = 0;
	unsigned int match_kpt = 0;
	d("+\n");

	if (dev == NULL || ctrl == NULL) {
		pr_err("[%s] Failed : dev.%pK, ctrl.%pK\n", __func__, dev, ctrl);
		return -EINVAL;
	}

	if ((ret = hispcpe_clean_wait_flag(dev, 3)) < 0) {
		pr_err("[%s] Failed : hispcpe_clean_wait_flag.%d\n", __func__, ret);
		return ret;
	}

	compare_request_handler(ctrl);

	if ((ret = hispcpe_wait(dev, 3)) < 0)
		pr_err("[%s] Failed : hispcpe_wait.%d\n", __func__, ret);

	match_kpt = (unsigned int)(readl((volatile void __iomem *)(dev->reg[COMPARE_REG] + 0x0))) >> 8;//lint !e529
	d("@@@@@@@@match_kpt =%d", match_kpt);
	compare_eof_handler(ctrl, match_kpt);
	d("-\n");
	return ret;
}

int hispcpe_matcher_request(struct hispcpe_s *dev, msg_req_matcher_request_t *ctrl)
{
	int ret = 0;
	d("+\n");

	if (dev == NULL || ctrl == NULL) {
		pr_err("[%s] Failed : dev.%pK, ctrl.%pK\n", __func__, dev, ctrl);
		return -EINVAL;
	}

	if ((ret = hispcpe_clean_wait_flag(dev, 2)) < 0) {
		pr_err("[%s] Failed : hispcpe_clean_wait_flag--reorder.%d\n", __func__, ret);
		return ret;
	}

	if ((ret = hispcpe_clean_wait_flag(dev, 3)) < 0) {
		pr_err("[%s] Failed : hispcpe_clean_wait_flag--compare.%d\n", __func__, ret);
		return ret;
	}

	ret = matcher_request_handler(ctrl);

	if (ret < 0) {
		pr_err("[%s] Failed : matcher_request_handler check.%d\n", __func__, ret);
		goto matcher_release;
	} else if (ret > 0) {
		return CPE_FW_OK;
	}

	if (ctrl->rdr_pyramid_layer > 0) {
		if ((ret = hispcpe_wait(dev, 2)) < 0) {
			pr_err("[%s] Failed : hispcpe_wait reorder.%d\n", __func__, ret);
			goto matcher_release;
		}
	}

	if (ctrl->cmp_pyramid_layer > 0) {
		if ((ret = hispcpe_wait(dev, 3)) < 0) {
			pr_err("[%s] Failed : hispcpe_wait compare.%d\n", __func__, ret);
			goto matcher_release;
		}
	}

matcher_release:
	matcher_eof_handler(ctrl);
	d("-\n");
	return ret;
}

static int hispcpe_check_args_mapkernel(void __user *args_mapkernel)
{
	if (args_mapkernel == NULL) {
		pr_err("[%s] args_mapkernel.%pK\n", __func__, args_mapkernel);
		return -EINVAL;
	}

	return CPE_FW_OK;
}


static int hispcpe_map_kernel_check_param(struct hispcpe_s *dev, struct memory_block_s *frame_buf)
{
	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	if (dev->pdev == NULL) {
		pr_err("[%s] Failed : pdev.NULL\n", __func__);
		return -EINVAL;
	}

	if (frame_buf->shared_fd < 0) {
		pr_err("[%s] Failed : hare_fd.%d\n", __func__, frame_buf->shared_fd);
		return -EINVAL;
	}

	dev->devbuf = dma_buf_get(frame_buf->shared_fd);

	if (IS_ERR_OR_NULL(dev->devbuf)) {
		pr_err("[%s] Failed : dma_buf_get, buf.%pK\n", __func__, dev->devbuf);
		return -EINVAL;
	}

	return CPE_FW_OK;
}

static int hispcpe_map_kernel(struct hispcpe_s *dev, unsigned long args, unsigned int *ipp_buff_size)
{
	void *virt_addr = NULL;
	struct memory_block_s frame_buf = { 0 };
	unsigned int iova = 0;
	unsigned long iova_size = 0;
	int ret = 0;
	void __user *args_mapkernel = (void __user *)(uintptr_t)args;

	if (hispcpe_check_args_mapkernel(args_mapkernel) != CPE_FW_OK)
		return -EINVAL;

	i("+\n");

	ret = atomic_read(&dev->mapbuf_ready);
	if (ret == 1) {
		pr_info("[%s] Failed : map_kernel already done.%d\n",
			__func__, ret);
		return -EINVAL;
	}

	if ((ret = copy_from_user(&frame_buf, args_mapkernel, sizeof(struct memory_block_s))) != 0) {
		pr_err("[%s] copy_from_user.%d\n", __func__, ret);
		return -EFAULT;
	}

	if (hispcpe_map_kernel_check_param(dev, &frame_buf) != CPE_FW_OK)
		return -EINVAL;

	iova = hisi_iommu_map_dmabuf(&dev->pdev->dev, dev->devbuf, frame_buf.prot, &iova_size);

	if (iova == 0) {
		pr_err("[%s] Failed : hisi_iommu_map_sg\n", __func__);
		goto err_dma_buf_get;
	}

	if (frame_buf.size != iova_size) {
		pr_err("[%s] hisi_iommu_map_range failed:  %d len 0x%lx\n", __func__, frame_buf.size, iova_size);
		goto err_hisi_iommu_check;
	}

	if ((ret = dma_buf_begin_cpu_access(dev->devbuf, DMA_FROM_DEVICE)) < 0) {
		pr_err("[%s] hisi_iommu_map_range failed: ret.%d\n", __func__, ret);
		goto err_hisi_iommu_check;
	}

	virt_addr = dma_buf_kmap(dev->devbuf, 0);

	if (NULL == virt_addr) {
		pr_err("[%s] Failed : dma_buf_kmap.%pK\n", __func__, virt_addr);
		goto err_cpu_access;
	}

	dev->virt_addr = virt_addr;
	dev->daddr = iova;
	*ipp_buff_size = frame_buf.size;

	atomic_set(&dev->mapbuf_ready, 1);

	i("-\n");
	return ret;
err_cpu_access:

	if ((ret = dma_buf_end_cpu_access(dev->devbuf, DMA_FROM_DEVICE)) < 0)
		pr_err("[%s] dma_buf_end_cpu_access failed: ret.%d\n", __func__, ret);

err_hisi_iommu_check:

	if ((ret = hisi_iommu_unmap_dmabuf(&dev->pdev->dev, dev->devbuf, iova)) < 0)
		pr_err("[%s] hisi_iommu_unmap_dmabuf failed: ret.%d\n", __func__, ret);

err_dma_buf_get:
	dma_buf_put(dev->devbuf);
	dev->devbuf = NULL;
	return -ENOMEM;
}

static int hispcpe_unmap_kernel(struct hispcpe_s *dev)
{
	int ret = 0;
	i("+\n");

	ret = atomic_read(&dev->mapbuf_ready);
	if (ret == 0) {
		pr_info("[%s] Failed : no map_kernel ops before.%d\n",
			__func__, ret);
		return -EINVAL;
	}
	atomic_set(&dev->mapbuf_ready, 0);

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	if (dev->pdev == NULL) {
		pr_err("[%s] Failed : pdev.NULL\n", __func__);
		return -EINVAL;
	}

	if ((dev->devbuf == NULL) || (dev->virt_addr == NULL)) {
		pr_err("[%s] Failed : devbuf.%pK, virt_addr.%pK\n", __func__, dev->devbuf, dev->virt_addr);
		return -EINVAL;
	}

	mutex_lock(&dev->ipp_work_lock);
	if (dev->hipp_refs[REFS_TYPE_MCF] > 0) {
		pr_err("[%s] : mcf refs Opened .\n", __func__);
		mutex_unlock(&dev->ipp_work_lock);
		return -ENOMEM;
	}
	if (dev->hipp_refs[REFS_TYP_SLAM] > 0) {
		pr_err("[%s] : slam refs Opened .\n", __func__);
		mutex_unlock(&dev->ipp_work_lock);
		return -ENOMEM;
	}
	if (dev->hipp_refs[REFS_TYP_VBK] > 0) {
		pr_err("[%s] : vbk refs Opened .\n", __func__);
		mutex_unlock(&dev->ipp_work_lock);
		return -ENOMEM;
	}
	if (dev->hipp_refs[REFS_TYP_MATCHER] > 0) {
		pr_err("[%s] : matcher refs Opened .\n", __func__);
		mutex_unlock(&dev->ipp_work_lock);
		return -ENOMEM;
	}
	mutex_unlock(&dev->ipp_work_lock);

	dma_buf_kunmap(dev->devbuf, 0, dev->virt_addr);
	ret = dma_buf_end_cpu_access(dev->devbuf, DMA_FROM_DEVICE);

	if (ret < 0)
		pr_err("[%s] dma_buf_end_cpu_access failed: ret.%d\n", __func__, ret);

	ret = hisi_iommu_unmap_dmabuf(&dev->pdev->dev, dev->devbuf, dev->daddr);

	if (ret < 0)
		pr_err("[%s] hisi_iommu_unmap_dmabuf failed: ret.%d\n", __func__, ret);

	dma_buf_put(dev->devbuf);
	dev->devbuf = NULL;
	dev->virt_addr = NULL;
	i("-\n");
	return 0;
}

static int hispcpe_init_map_memory(struct hispcpe_s *dev, unsigned long args)
{
	int ret = 0;
	unsigned int ipp_buff_size = 0;
	i("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	if ((ret = hispcpe_map_kernel(dev, args, &ipp_buff_size)) != 0) {
		pr_err("[%s] Failed : hispcpe_map_kernel.%d\n", __func__, ret);
		return ret;
	}

	if ((ret = cpe_init_memory(ipp_buff_size)) != 0) {
		pr_err("[%s] Failed : cpe_init_memory.%d\n", __func__, ret);

		if ((ret = hispcpe_unmap_kernel(dev)) != 0) {
			pr_err("[%s] Failed : hispcpe_umap_kernel.%d\n", __func__, ret);
			return -EINVAL;
		}

		return ret;
	}

	return 0;
}

#if FLAG_LOG_DEBUG
static int hispcpe_map_iommu(struct hispcpe_s *dev, struct memory_block_s *buf)
{
	unsigned int iova;
	unsigned long size;
	i("+\n");

	if (buf->shared_fd < 0) {
		pr_err("[%s] Failed : hare_fd.%d\n", __func__, buf->shared_fd);
		return -EINVAL;
	}

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	if (dev->pdev == NULL) {
		pr_err("[%s] Failed : pdev.NULL\n", __func__);
		return -EINVAL;
	}

	dev->modulebuf = dma_buf_get(buf->shared_fd);

	if (IS_ERR_OR_NULL(dev->modulebuf)) {
		pr_err("[%s] Failed : dma_buf_get, buf.%pK\n", __func__, dev->modulebuf);
		return -EINVAL;
	}

	iova = hisi_iommu_map_dmabuf(&dev->pdev->dev, dev->modulebuf, buf->prot, &size);

	if (iova == 0) {
		pr_err("[%s] Failed : hisi_iommu_map_dmabuf\n", __func__);
		goto err_dma_buf_get;
	}

	if (buf->size != size) {
		pr_err("[%s] hisi_iommu_map_range failed:  %d len 0x%lx\n", __func__, buf->size, size);
		goto err_dma_buf_get;
	}

	buf->da = iova;
	pr_info("[after map iommu]da.(0x%x)", buf->da);/*lint !e626 */
	i("-\n");
	return 0;
err_dma_buf_get:
	dma_buf_put(dev->modulebuf);
	dev->modulebuf = NULL;
	i("-\n");
	return -ENOMEM;
}

void hispcpe_unmap_iommu(struct hispcpe_s *dev, struct memory_block_s *buf)
{
	int ret = 0;
	i("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.NULL\n", __func__);
		return;
	}

	if (dev->pdev == NULL) {
		pr_err("[%s] Failed : pdev.NULL\n", __func__);
		return;
	}

	if (IS_ERR_OR_NULL(dev->modulebuf)) {
		pr_err("[%s] Failed : modulebuf.%pK\n", __func__, dev->modulebuf);
		return;
	}

	if (buf->da == 0) {
		pr_err("[%s] Failed : buf->da.0\n", __func__);
		return;
	}

	ret = hisi_iommu_unmap_dmabuf(&dev->pdev->dev, dev->modulebuf, buf->da);

	if (ret < 0)
		pr_err("[%s] Failed : hisi_iommu_unmap_dmabuf\n", __func__);

	dma_buf_put(dev->modulebuf);
	dev->modulebuf = NULL;
	buf->da = 0;
	i("-\n");
}
#endif

int ipp_set_rate(struct hispcpe_s *dev)
{
	unsigned int ret = 0;
	unsigned int curr_max_rate = dev->curr_cpe_rate_value;

	if (dev->curr_slam_rate_value > dev->curr_cpe_rate_value)
		curr_max_rate = dev->curr_slam_rate_value;

	if (curr_max_rate !=  dev->ipp_set_rate_value) {
		pr_info("curr_max_rate  = %d\n", curr_max_rate);

		if ((ret = jpeg_ipp_set_rate(dev->jpg_clk, curr_max_rate)) != 0) {
			pr_err("[%s] Failed to set ipp  rate mode: %d.\n", __func__, curr_max_rate);

			if ((ret = jpeg_ipp_set_rate(dev->jpg_clk, dev->jpgclk_low_frequency)) != 0) {
				pr_err("[%s] Failed to set ipp  rate mode: %d.\n", __func__, dev->jpgclk_low_frequency);
				return -EINVAL;
			}
		}

		dev->ipp_set_rate_value = curr_max_rate;
	}

	return 0;
}

static int mcf_process_internal(struct hispcpe_s *dev, msg_req_mcf_request_t *ctrl_mcf)
{
	unsigned int ret = 0;

	if ((ret = ipp_set_rate(dev)) != 0) {
		pr_err("[%s] Failed : ipp_set_rate.%d\n", __func__, ret);
		return ret;
	}

	if ((ret = hispcpe_cpe_clk_enable(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_cpe_clk_enable.%d\n", __func__, ret);
		return ret;
	}

	if ((ret = hispcpe_mcf_request(dev, ctrl_mcf)) != 0) {
		pr_err("[%s] Failed : hispcpe_mcf_request.%d\n", __func__, ret);

		if (hispcpe_cpe_clk_disable(dev) != 0)
			pr_err("[%s] Failed : hispcpe_cpe_clk_disable\n", __func__);

		return ret;
	}

	if ((ret = hispcpe_cpe_clk_disable(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_cpe_clk_disable.%d\n", __func__, ret);
		return ret;
	}

	dev->curr_cpe_rate_value = dev->jpgclk_low_frequency;

	if ((ret = ipp_set_rate(dev)) != 0) {
		pr_err("[%s] Failed : ipp_set_rate.%d\n", __func__, ret);
		return ret;
	}

	return ret;
}

static int ipp_module_lockusecase(struct hispcpe_s *dev, void __user *args)
{
	unsigned int ret = 0;

	if (dev->ippdrv == NULL) {
		pr_err("[%s] Failed : ippdrv.%pK\n", __func__, dev->ippdrv);
		return -EINVAL;
	}

	if (dev->ippdrv->lock_usecase == NULL) {
		pr_err("[%s] Failed : ippdrv->lock_usecase.%pK\n", __func__, dev->ippdrv->lock_usecase);
		return -EINVAL;
	}

	if ((ret = dev->ippdrv->lock_usecase(dev->ippdrv)) != 0) {
		pr_err("[%s] Failed : dev->ippdrv->lock_usecase.%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int mcf_process(struct hispcpe_s *dev, unsigned long args)
{
	unsigned int ret = 0;
	unsigned int ret1 = 0;
	msg_req_mcf_request_t *ctrl_mcf = NULL;
	unsigned int mcf_rate_index = 0;
	void __user *args_mcf = (void __user *)(uintptr_t)args;

	if (args_mcf == NULL) {
		pr_err("[%s] args_mcf NULL.%pK\n", __func__, args_mcf);
		return -EINVAL;
	}

	if (atomic_read(&dev->mapbuf_ready) <= 0) {
		pr_err("[%s] no map_kernel ops before.0x%x\n",
		       __func__, atomic_read(&dev->mapbuf_ready));
		return -EINVAL;
	}

	if (atomic_read(&dev->cpe_supply_refs) <= 0) {
		pr_err("[%s] Failed: cpe_supply not Opened.0x%x\n", __func__, atomic_read(&dev->cpe_supply_refs));
		return -EINVAL;
	}

	if (ipp_module_lockusecase(dev, args_mcf)) {
		pr_err("[%s] Failed : ipp_module_lockusecase.%pK\n", __func__, dev);
		return -EINVAL;
	}

	ctrl_mcf = (msg_req_mcf_request_t *)vmalloc(sizeof(msg_req_mcf_request_t));

	if (NULL == ctrl_mcf) {
		pr_err(" fail to vmalloc ctrl_mcf\n");
		goto free_mcf_kmalloc_memory0;
	}

	if (memset_s(ctrl_mcf, sizeof(msg_req_mcf_request_t), 0, sizeof(msg_req_mcf_request_t))) {
		pr_err(" fail to memset_s ctrl_mcf\n");
		goto free_mcf_kmalloc_memory;
	}

	if ((ret = copy_from_user(ctrl_mcf, args_mcf, sizeof(msg_req_mcf_request_t))) != 0) {
		pr_err("[%s] copy_from_user.%d\n", __func__, ret);
		goto free_mcf_kmalloc_memory;
	}

	mcf_rate_index = ctrl_mcf->mcf_rate_value;
	mcf_rate_index = mcf_rate_index > CPE_RATE_INDEX_MAX ? 0 : mcf_rate_index ;
	dev->curr_cpe_rate_value = clk_rate_map[mcf_rate_index];

	if ((ret = mcf_process_internal(dev, ctrl_mcf)) != 0) {
		pr_err("[%s] Failed : ipp_set_rate.%d\n", __func__, ret);
		goto free_mcf_kmalloc_memory;
	}

free_mcf_kmalloc_memory:
	vfree(ctrl_mcf);
	ctrl_mcf = NULL;
free_mcf_kmalloc_memory0:
	dev->curr_cpe_rate_value = dev->jpgclk_low_frequency;

	if ((ret1 = dev->ippdrv->unlock_usecase(dev->ippdrv)) != 0) {
		pr_err("[%s] Failed : dev->ippdrv->unlock_usecase.%d\n", __func__, ret1);
		return ret1;
	}

	return ret;
}

static int vbk_process_check_param(struct hispcpe_s *dev, void __user *args_vbk)
{
	if (args_vbk == NULL) {
		pr_err("[%s] args_vbk NULL.%pK\n", __func__, args_vbk);
		return -EINVAL;
	}

	if (atomic_read(&dev->mapbuf_ready) <= 0) {
		pr_err("[%s] no map_kernel ops before.0x%x\n",
		       __func__, atomic_read(&dev->mapbuf_ready));
		return -EINVAL;
	}

	if (atomic_read(&dev->cpe_supply_refs) <= 0) {
		pr_err("[%s] Failed: cpe_supply not Opened.0x%x\n", __func__, atomic_read(&dev->cpe_supply_refs));
		return -EINVAL;
	}

	if (ipp_module_lockusecase(dev, args_vbk)) {
		pr_err("[%s] Failed : ipp_module_lockusecase.%pK\n", __func__, dev);
		return -EINVAL;
	}

	return CPE_FW_OK;
}

static int vbk_processv_vbk_req(struct hispcpe_s *dev, unsigned int *ret, msg_req_vbk_request_t *ctrl_vbk)
{
	if ((*ret = ipp_set_rate(dev)) != 0) {
		pr_err("[%s] Failed : ipp_set_rate.%d\n", __func__, *ret);
		return -1;
	}

	if ((*ret = hispcpe_cpe_clk_enable(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_cpe_clk_enable.%d\n", __func__, *ret);
		return -1;
	}

	if ((*ret = hispcpe_vbk_request(dev, ctrl_vbk)) != 0) {
		pr_err("[%s] Failed : hispcpe_vbk_request.%d\n", __func__, *ret);

		if (hispcpe_cpe_clk_disable(dev) != 0)
			pr_err("[%s] Failed : hispcpe_cpe_clk_disable\n", __func__);

		return -1;
	}

	if ((*ret = hispcpe_cpe_clk_disable(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_cpe_clk_disable.%d\n", __func__, *ret);
		return -1;
	}

	dev->curr_cpe_rate_value = dev->jpgclk_low_frequency;

	if ((*ret = ipp_set_rate(dev)) != 0) {
		pr_err("[%s] Failed : ipp_set_rate.%d\n", __func__, *ret);
		return -1;
	}

	return CPE_FW_OK;
}

static int vbk_process(struct hispcpe_s *dev, unsigned long args)
{
	unsigned int ret = 0;
	unsigned int ret1 = 0;
	msg_req_vbk_request_t *ctrl_vbk = NULL;
	void __user *args_vbk = (void __user *)(uintptr_t)args;
#if VBK_RATE_SET_BY_HAL
	unsigned int vbk_rate_index = 0;
#endif

	if (vbk_process_check_param(dev, args_vbk) != CPE_FW_OK)
		return -EINVAL;

	ctrl_vbk = (msg_req_vbk_request_t *)vmalloc(sizeof(msg_req_vbk_request_t));

	if (NULL == ctrl_vbk) {
		pr_err(" fail to vmalloc ctrl_vbk\n");
		goto free_vbk_kmalloc_memory0;
	}

	if (memset_s(ctrl_vbk, sizeof(msg_req_vbk_request_t), 0, sizeof(msg_req_vbk_request_t))) {
		pr_err(" fail to memset_s ctrl_vbk\n");
		goto free_vbk_kmalloc_memory;
	}

	if ((ret = copy_from_user(ctrl_vbk, args_vbk, sizeof(msg_req_vbk_request_t))) != 0) {
		pr_err("[%s] copy_from_user.%d\n", __func__, ret);
		goto free_vbk_kmalloc_memory;
	}

#if VBK_RATE_SET_BY_HAL
	vbk_rate_index = ctrl_vbk->vbk_rate_value;
	vbk_rate_index = vbk_rate_index > CPE_RATE_INDEX_MAX ? 0 : vbk_rate_index ;
	dev->curr_cpe_rate_value = clk_rate_map[vbk_rate_index];
#else
	dev->curr_cpe_rate_value = clk_rate_map[3];
#endif

	if (vbk_processv_vbk_req(dev, &ret, ctrl_vbk) != CPE_FW_OK)
		goto free_vbk_kmalloc_memory;

free_vbk_kmalloc_memory:
	vfree(ctrl_vbk);
	ctrl_vbk = NULL;
free_vbk_kmalloc_memory0:
	dev->curr_cpe_rate_value = dev->jpgclk_low_frequency;

	if ((ret1 = dev->ippdrv->unlock_usecase(dev->ippdrv)) != 0) {
		pr_err("[%s] Failed : dev->ippdrv->unlock_usecase.%d\n", __func__, ret1);
		return ret1;
	}

	return ret ;
}

#if FLAG_LOG_DEBUG
static int reorder_process_internal(struct hispcpe_s *dev, msg_req_reorder_request_t *ctrl_reorder)
{
	unsigned int ret = 0;

	if ((ret = ipp_set_rate(dev)) != 0) {
		pr_err("[%s] Failed : ipp_set_rate.%d\n", __func__, ret);
		return ret;
	}

	if ((ret = hispcpe_reorder_clk_enable(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_reorder_clk_enable.%d\n", __func__, ret);
		return ret;
	}

	if ((ret = hispcpe_reorder_request(dev, ctrl_reorder)) != 0) {
		pr_err("[%s] Failed : hispcpe_reorder_request.%d\n", __func__, ret);

		if (hispcpe_reorder_clk_disable(dev) != 0)
			pr_err("[%s] Failed : hispcpe_cpe_clk_disable\n", __func__);

		return ret;
	}

	if ((ret = hispcpe_reorder_clk_disable(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_cpe_clk_disable.%d\n", __func__, ret);
		return ret;
	}

	dev->curr_cpe_rate_value = dev->jpgclk_low_frequency;

	if ((ret = ipp_set_rate(dev)) != 0) {
		pr_err("[%s] Failed : ipp_set_rate.%d\n", __func__, ret);
		return ret;
	}

	return ret;
}


static int reorder_process(struct hispcpe_s *dev, unsigned long args)
{
	unsigned int ret = 0;
	unsigned int ret1 = 0;
	unsigned int rdr_rate_index = 0;
	msg_req_reorder_request_t *ctrl_reorder = NULL;
	void __user *args_rdr = (void __user *)(uintptr_t)args;

	if (args_rdr == NULL) {
		pr_err("[%s] args_rdr NULL.%pK\n", __func__, args_rdr);
		return -EINVAL;
	}

	if (atomic_read(&dev->cpe_supply_refs) <= 0) {
		pr_err("[%s] Failed: cpe_supply not Opened.0x%x\n", __func__, atomic_read(&dev->cpe_supply_refs));
		return -EINVAL;
	}

	if (ipp_module_lockusecase(dev, args_rdr)) {
		pr_err("[%s] Failed : ipp_module_lockusecase.%pK\n", __func__, dev);
		return -EINVAL;
	}

	ctrl_reorder = (msg_req_reorder_request_t *)vmalloc(sizeof(msg_req_reorder_request_t));

	if (NULL == ctrl_reorder) {
		pr_err(" fail to vmalloc ctrl_reorder\n");
		goto free_reorder_kmalloc_memory0;
	}

	if (memset_s(ctrl_reorder, sizeof(msg_req_reorder_request_t), 0, sizeof(msg_req_reorder_request_t))) {
		pr_err(" fail to memset_s ctrl_reorder\n");
		goto free_reorder_kmalloc_memory;
	}

	if ((ret = copy_from_user(ctrl_reorder, args_rdr, sizeof(msg_req_reorder_request_t))) != 0) {
		pr_err("[%s] copy_from_user.%d\n", __func__, ret);
		goto free_reorder_kmalloc_memory;
	}

	rdr_rate_index = ctrl_reorder->rdr_rate_value;
	rdr_rate_index = rdr_rate_index > SLAM_RATE_INDEX_MAX ? 0 : rdr_rate_index ;
	dev->curr_slam_rate_value = clk_rate_map[rdr_rate_index];

	if ((ret = reorder_process_internal(dev, ctrl_reorder)) != 0) {
		pr_err("[%s] Failed : reorder_process_internal.%d\n", __func__, ret);
		goto free_reorder_kmalloc_memory;
	}

free_reorder_kmalloc_memory:
	vfree(ctrl_reorder);
	ctrl_reorder = NULL;
free_reorder_kmalloc_memory0:
	dev->curr_cpe_rate_value = dev->jpgclk_low_frequency;

	if ((ret1 = dev->ippdrv->unlock_usecase(dev->ippdrv)) != 0) {
		pr_err("[%s] Failed : dev->ippdrv->unlock_usecase.%d\n", __func__, ret1);
		return ret1;
	}

	return ret ;
}

static int compare_process_internal(struct hispcpe_s *dev, msg_req_compare_request_t *ctrl_compare)
{
	unsigned int ret = 0;

	if ((ret = ipp_set_rate(dev)) != 0) {
		pr_err("[%s] Failed : ipp_set_rate.%d\n", __func__, ret);
		return ret;
	}

	if ((ret = hispcpe_compare_clk_enable(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_cpe_clk_enable.%d\n", __func__, ret);
		return ret;
	}

	if ((ret = hispcpe_compare_request(dev, ctrl_compare)) != 0) {
		pr_err("[%s] Failed : hispcpe_compare_request.%d\n", __func__, ret);

		if (hispcpe_compare_clk_disable(dev) != 0)
			pr_err("[%s] Failed : hispcpe_cpe_clk_disable\n", __func__);

		return ret;
	}

	if ((ret = hispcpe_compare_clk_disable(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_cpe_clk_disable.%d\n", __func__, ret);
		return ret;
	}

	dev->curr_cpe_rate_value = dev->jpgclk_low_frequency;

	if ((ret = ipp_set_rate(dev)) != 0) {
		pr_err("[%s] Failed : ipp_set_rate.%d\n", __func__, ret);
		return ret;
	}

	return ret ;
}


static int compare_process(struct hispcpe_s *dev, unsigned long args)
{
	unsigned int ret = 0;
	unsigned int ret1 = 0;
	unsigned int cmp_rate_index = 0;
	msg_req_compare_request_t *ctrl_compare = NULL;
	void __user *args_cmp = (void __user *)(uintptr_t)args;

	if (args_cmp == NULL) {
		pr_err("[%s] args_cmp NULL.%pK\n", __func__, args_cmp);
		return -EINVAL;
	}

	if (atomic_read(&dev->cpe_supply_refs) <= 0) {
		pr_err("[%s] Failed: cpe_supply not Opened.0x%x\n", __func__, atomic_read(&dev->cpe_supply_refs));
		return -EINVAL;
	}

	if (ipp_module_lockusecase(dev, args_cmp)) {
		pr_err("[%s] Failed : ipp_module_lockusecase.%pK\n", __func__, dev);
		return -EINVAL;
	}

	ctrl_compare = (msg_req_compare_request_t *)vmalloc(sizeof(msg_req_compare_request_t));

	if (NULL == ctrl_compare) {
		pr_err(" fail to vmalloc ctrl_compare\n");
		goto free_compare_kmalloc_memory0;
	}

	if (memset_s(ctrl_compare, sizeof(msg_req_compare_request_t), 0, sizeof(msg_req_compare_request_t))) {
		pr_err(" fail to memset_s ctrl_compare\n");
		goto free_compare_kmalloc_memory;
	}

	if ((ret = copy_from_user(ctrl_compare, args_cmp, sizeof(msg_req_compare_request_t))) != 0) {
		pr_err("[%s] copy_from_user.%d\n", __func__, ret);
		goto free_compare_kmalloc_memory;
	}

	cmp_rate_index = ctrl_compare->cmp_rate_value;
	cmp_rate_index = cmp_rate_index > SLAM_RATE_INDEX_MAX ? 0 : cmp_rate_index ;
	dev->curr_slam_rate_value = clk_rate_map[cmp_rate_index];

	if ((ret = compare_process_internal(dev, ctrl_compare)) != 0) {
		pr_err("[%s] Failed : compare_process_internal.%d\n", __func__, ret);
		goto free_compare_kmalloc_memory;
	}

free_compare_kmalloc_memory:
	vfree(ctrl_compare);
	ctrl_compare = NULL;
free_compare_kmalloc_memory0:
	dev->curr_cpe_rate_value = dev->jpgclk_low_frequency;

	if ((ret1 = dev->ippdrv->unlock_usecase(dev->ippdrv)) != 0) {
		pr_err("[%s] Failed : dev->ippdrv->unlock_usecase.%d\n", __func__, ret1);
		return ret1;
	}

	return ret ;
}
#endif

static int slam_process_slam_req(struct hispcpe_s *dev, msg_req_slam_request_t *ctrl_slam,
						void __user *args_slam, unsigned int *ret)
{
	unsigned int slam_rate_index = 0;

	if (memset_s(ctrl_slam, sizeof(msg_req_slam_request_t), 0, sizeof(msg_req_slam_request_t))) {
		pr_err(" fail to memset_s ctrl_slam\n");
		return -1;
	}

	if ((*ret = copy_from_user(ctrl_slam, args_slam, sizeof(msg_req_slam_request_t))) != 0) {
		pr_err("[%s] copy_from_user.%d\n", __func__, *ret);
		return -1;
	}

	slam_rate_index = ctrl_slam->req_orb.slam_rate_value;

	if (slam_rate_index > SLAM_RATE_INDEX_MAX) {
		pr_err("[%s] check slam rate index.%d\n", __func__, slam_rate_index);
		return -1;
	} else
		dev->curr_slam_rate_value = clk_rate_map[slam_rate_index];

	if ((*ret = ipp_set_rate(dev)) != 0) {
		pr_err("[%s] Failed : slam_set_rate.%d\n", __func__, *ret);
		return -1;
	}

	if (ctrl_slam->slam_pyramid_layer > SLAM_LAYER_MAX) {
		pr_err("[%s] Failed : Invalid slam_pyramid_layer = %d\n", __func__, ctrl_slam->slam_pyramid_layer);
		return -1;
	}

	if ((*ret = hispcpe_slam_clk_enable(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_slam_clk_enable.%d\n", __func__, *ret);
		return -1;
	}

	if ((*ret = hispcpe_slam_request(dev, ctrl_slam)) != 0) {
		pr_err("[%s] Failed : hispcpe_slam_request.%d\n", __func__, *ret);

		if (hispcpe_slam_clk_disable(dev) != 0)
			pr_err("[%s] Failed : hispcpe_slam_clk_disable\n", __func__);

		return -1;
	}

	if ((*ret = hispcpe_slam_clk_disable(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_slam_clk_disable.%d\n", __func__, *ret);
		return -1;
	}

	return CPE_FW_OK;
}

static int slam_process(struct hispcpe_s *dev, unsigned long args)
{
	unsigned int ret = 0;
	unsigned int ret1 = 0;
	msg_req_slam_request_t *ctrl_slam = NULL;
	void __user *args_slam = (void __user *)(uintptr_t)args;

	if (args_slam == NULL) {
		pr_err("[%s] args_slam NULL.%pK\n", __func__, args_slam);
		return -EINVAL;
	}

	if (atomic_read(&dev->mapbuf_ready) <= 0) {
		pr_err("[%s] no map_kernel ops before.0x%x\n",
		       __func__, atomic_read(&dev->mapbuf_ready));
		return -EINVAL;
	}

	if (atomic_read(&dev->cpe_supply_refs) <= 0) {
		pr_err("[%s] Failed: cpe_supply not Opened.0x%x\n", __func__, atomic_read(&dev->cpe_supply_refs));
		return -EINVAL;
	}

	if (ipp_module_lockusecase(dev, args_slam)) {
		pr_err("[%s] Failed : ipp_module_lockusecase.%pK\n", __func__, dev);
		return -EINVAL;
	}

	ctrl_slam = (msg_req_slam_request_t *)vmalloc(sizeof(msg_req_slam_request_t));

	if (NULL == ctrl_slam) {
		pr_err(" fail to vmalloc ctrl_slam\n");
		goto free_slam_kmalloc_memory0;
	}

	if (slam_process_slam_req(dev, ctrl_slam, args_slam, &ret) != CPE_FW_OK)
		goto free_slam_kmalloc_memory;

free_slam_kmalloc_memory:
	vfree(ctrl_slam);
	ctrl_slam = NULL;
free_slam_kmalloc_memory0:

	if ((ret1 = dev->ippdrv->unlock_usecase(dev->ippdrv)) != 0) {
		pr_err("[%s] Failed : dev->ippdrv->unlock_usecase.%d\n", __func__, ret1);
		return ret1;
	}

	return ret ;
}

static int matcher_process_matcher_req(struct hispcpe_s *dev, msg_req_matcher_request_t *ctrl_matcher, unsigned int *ret)
{
	if (ctrl_matcher->rdr_pyramid_layer > SLAM_LAYER_MAX) {
		pr_err("[%s] Failed : Invalid rdr_pyramid_layer = %d\n", __func__, ctrl_matcher->rdr_pyramid_layer);
		return -1;
	}

	if (ctrl_matcher->cmp_pyramid_layer > SLAM_LAYER_MAX) {
		pr_err("[%s] Failed : Invalid cmp_pyramid_layer = %d\n", __func__, ctrl_matcher->cmp_pyramid_layer);
		return -1;
	}

	if ((*ret = ipp_set_rate(dev)) != 0) {
		pr_err("[%s] Failed : ipp_set_rate.%d\n", __func__, *ret);
		return -1;
	}

	if ((*ret = hispcpe_matcher_clk_enable(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_reorder_clk_enable.%d\n", __func__, *ret);
		return -1;
	}

	if ((*ret = hispcpe_matcher_request(dev, ctrl_matcher)) != 0) {
		pr_err("[%s] Failed : hispcpe_matcher_request.%d\n", __func__, *ret);

		if (hispcpe_matcher_clk_disable(dev) != 0)
			pr_err("[%s] Failed : hispcpe_cpe_clk_disable\n", __func__);

		return -1;
	}

	if ((*ret = hispcpe_matcher_clk_disable(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_cpe_clk_disable.%d\n", __func__, *ret);
		return -1;
	}

	dev->curr_cpe_rate_value = dev->jpgclk_low_frequency;

	if ((*ret = ipp_set_rate(dev)) != 0) {
		pr_err("[%s] Failed : ipp_set_rate.%d\n", __func__, *ret);
		return -1;
	}

	return CPE_FW_OK;
}

static int matcher_process(struct hispcpe_s *dev, unsigned long args)
{
	unsigned int ret = 0;
	unsigned int ret1 = 0;
	unsigned int matcher_rate_index = 0;
	msg_req_matcher_request_t *ctrl_matcher = NULL;
	void __user *args_mathcer = (void __user *)(uintptr_t)args;

	if (args_mathcer == NULL) {
		pr_err("[%s] args_mathcer NULL.%pK\n", __func__, args_mathcer);
		return -EINVAL;
	}

	if (atomic_read(&dev->mapbuf_ready) <= 0) {
		pr_err("[%s] no map_kernel ops before.0x%x\n",
		       __func__, atomic_read(&dev->mapbuf_ready));
		return -EINVAL;
	}

	if (atomic_read(&dev->cpe_supply_refs) <= 0) {
		pr_err("[%s] Failed: cpe_supply not Opened.0x%x\n", __func__, atomic_read(&dev->cpe_supply_refs));
		return -EINVAL;
	}

	if (ipp_module_lockusecase(dev, args_mathcer)) {
		pr_err("[%s] Failed : ipp_module_lockusecase.%pK\n", __func__, dev);
		return -EINVAL;
	}

	ctrl_matcher = (msg_req_matcher_request_t *)vmalloc(sizeof(msg_req_matcher_request_t));

	if (NULL == ctrl_matcher) {
		pr_err(" fail to vmalloc ctrl_matcher\n");
		goto free_matcher_kmalloc_memory0;
	}

	if (memset_s(ctrl_matcher, sizeof(msg_req_matcher_request_t), 0, sizeof(msg_req_matcher_request_t))) {
		pr_err(" fail to memset_s ctrl_matcher\n");
		goto free_matcher_kmalloc_memory;
	}

	if ((ret = copy_from_user(ctrl_matcher, args_mathcer, sizeof(msg_req_matcher_request_t))) != 0) {
		pr_err("[%s] copy_from_user.%d\n", __func__, ret);
		goto free_matcher_kmalloc_memory;
	}

	matcher_rate_index = ctrl_matcher->matcher_rate_value;
	matcher_rate_index = matcher_rate_index > SLAM_RATE_INDEX_MAX ? 0 : matcher_rate_index ;
	dev->curr_slam_rate_value = clk_rate_map[matcher_rate_index];

	if(matcher_process_matcher_req(dev, ctrl_matcher, &ret) != CPE_FW_OK)
		goto free_matcher_kmalloc_memory;

free_matcher_kmalloc_memory:
	vfree(ctrl_matcher);
	ctrl_matcher = NULL;
free_matcher_kmalloc_memory0:
	dev->curr_cpe_rate_value = dev->jpgclk_low_frequency;

	if ((ret1 = dev->ippdrv->unlock_usecase(dev->ippdrv)) != 0) {
		pr_err("[%s] Failed : dev->ippdrv->unlock_usecase.%d\n", __func__, ret1);
		return ret1;
	}

	return ret ;
}

static int hispcpe_ioctl_relax_ipp_wakelock(struct hispcpe_s *dev)
{
	mutex_lock(&dev->ipp_wakelock_mutex);

	if (dev->ipp_wakelock.active) {
		__pm_relax(&dev->ipp_wakelock);
		pr_info("ipp power up wake unlock.\n");
	}

	mutex_unlock(&dev->ipp_wakelock_mutex);

	return CPE_FW_OK;
}

#if FLAG_LOG_DEBUG
static int hispcpe_ioctl_cmd_unmap_iommu(struct hispcpe_s *dev, unsigned int cmd,
						unsigned long args, struct memory_block_s *buf)
{
	int ret = 0;
	void __user *args_unmap_iommu = NULL;

	switch (cmd) {
	case HISP_CPE_UNMAP_IOMMU:
		args_unmap_iommu = (void __user *)(uintptr_t)args;

		if (args_unmap_iommu == NULL) {
			pr_err("[%s] args_unmap_iommu.%pK\n", __func__, args_unmap_iommu);
			return -EINVAL;
		}

		if ((ret = copy_from_user(buf, args_unmap_iommu, sizeof(struct memory_block_s))) != 0) {
			pr_err("[%s] copy_from_user.%d\n", __func__, ret);
			return -EFAULT;
		}

		hispcpe_unmap_iommu(dev, buf);
		break;

	default :
		pr_err("[%s] Failed : Invalid cmd.0x%x\n", __func__, cmd);
		break;
	}

	return CPE_FW_OK;
}
#endif

static int hispcpe_ioctl_cmd_unmap_buf_map_iommu(struct hispcpe_s *dev, unsigned int cmd,
						unsigned long args, struct memory_block_s *buf)
{
	int ret = 0;

	switch (cmd) {
	case HISP_CPE_UNMAP_BUF:
		if (args == 0) {
			pr_err("[%s] args is 0\n", __func__);
			return -EINVAL;
		}

		if ((ret = hispcpe_unmap_kernel(dev)) != 0) {
			pr_err("[%s] Failed : hispcpe_umap_kernel.%d\n", __func__, ret);
			return -EINVAL;
		}

		break;

#if FLAG_LOG_DEBUG
	case HISP_CPE_MAP_IOMMU:
		void __user *args_map_iommu = (void __user *)(uintptr_t)args;

		if (args_map_iommu == NULL) {
			pr_err("[%s] args_map_iommu.%pK\n", __func__, args_map_iommu);
			return -EINVAL;
		}

		if ((ret = copy_from_user(buf, args_map_iommu, sizeof(struct memory_block_s))) != 0) {
			pr_err("[%s] copy_from_user.%d\n", __func__, ret);
			return -EFAULT;
		}

		if ((ret = hispcpe_map_iommu(dev, buf)) != 0) {
			pr_err("[%s] Failed : hispcpe_map_iommu.%d\n", __func__, ret);
			return ret;
		}

		if ((ret = copy_to_user(args_map_iommu, buf, sizeof(struct memory_block_s))) != 0) {
			pr_err("[%s] copy_to_user.%d\n", __func__, ret);
			return -EFAULT;
		}

		break;
#endif
	default:
#if FLAG_LOG_DEBUG
		if ((ret = hispcpe_ioctl_cmd_unmap_iommu(dev, cmd, args, buf)) != 0)
			return ret;
#endif
		break;
	}

	return CPE_FW_OK;
}

static int hispcpe_ioctl_cmd_slam_reorder_aux(struct hispcpe_s *dev, unsigned int cmd,
						unsigned long args, struct memory_block_s *buf)
{
	int ret = 0;

	switch (cmd) {
	case HISP_SLAM_REQ:
		if (0 == dev->sec_path) {
			mutex_lock(&dev->ipp_work_lock);
			if (dev->hipp_refs[REFS_TYP_SLAM] > 0) {
				pr_err("[%s] : slam refs Opened .\n", __func__);
				mutex_unlock(&dev->ipp_work_lock);
				return -EINVAL;
			}
			dev->hipp_refs[REFS_TYP_SLAM] = 1;
			mutex_unlock(&dev->ipp_work_lock);
			if ((ret = slam_process(dev, args)) != 0) {
				pr_err("[%s] Failed : slam_process.%d\n", __func__, ret);

				mutex_lock(&dev->ipp_work_lock);
				dev->hipp_refs[REFS_TYP_SLAM] = 0;
				mutex_unlock(&dev->ipp_work_lock);
				return ret;
			}
			mutex_lock(&dev->ipp_work_lock);
			dev->hipp_refs[REFS_TYP_SLAM] = 0;
			mutex_unlock(&dev->ipp_work_lock);

		} else {
			pr_err(" Failed : check sec_path \n");
			return -EINVAL;
		}

		break;

#if FLAG_LOG_DEBUG
	case HISP_REORDER_REQ:
		if ((ret = reorder_process(dev, args)) != 0) {
			pr_err("[%s] Failed : hispcpe_reorder_request.%d\n", __func__, ret);
			return ret;
		}

		break;

	case HISP_COMPARE_REQ:
		if ((ret = compare_process(dev, args)) != 0) {
			pr_err("[%s] Failed : hispcpe_compare_request.%d\n", __func__, ret);
			return ret;
		}

		break;
#endif
	case HISP_MATCHER_REQ:
		mutex_lock(&dev->ipp_work_lock);
		if (dev->hipp_refs[REFS_TYP_MATCHER] > 0) {
			pr_err("[%s] : matcher refs Opened .\n", __func__);
			mutex_unlock(&dev->ipp_work_lock);
			return -EINVAL;
		}
		dev->hipp_refs[REFS_TYP_MATCHER] = 1;
		mutex_unlock(&dev->ipp_work_lock);

		if ((ret = matcher_process(dev, args)) != 0) {
			pr_err("[%s] Failed : hispcpe_mather_request.%d\n", __func__, ret);

			mutex_lock(&dev->ipp_work_lock);
			dev->hipp_refs[REFS_TYP_MATCHER] = 0;
			mutex_unlock(&dev->ipp_work_lock);
			return ret;
		}
		mutex_lock(&dev->ipp_work_lock);
		dev->hipp_refs[REFS_TYP_MATCHER] = 0;
		mutex_unlock(&dev->ipp_work_lock);
		break;

	case HISP_CPE_MAP_BUF:
		pr_info("[%s] cmd.HISP_CPE_MAP_BUF\n", __func__);

		if ((ret = hispcpe_init_map_memory(dev, args)) != 0) {
			pr_err("[%s] Failed : hispcpe_init_map_memory.%d\n", __func__, ret);
			return ret;
		}

		break;

	default:
		if ((ret = hispcpe_ioctl_cmd_unmap_buf_map_iommu(dev, cmd, args, buf)) != 0)
			return ret;
		break;
	}

	return CPE_FW_OK;
}

static int hispcpe_ioctl_cmd_cpe(struct hispcpe_s *dev, unsigned int cmd,
						unsigned long args, struct memory_block_s *buf)
{
	int ret = 0;

	switch (cmd) {
	case HISP_CPE_PWRDN:
		if (0 == dev->sec_path) {
			if ((ret = hispcpe_powerdn(dev)) != 0) {
				pr_err("[%s] Failed : hispcpe_powerdn.%d\n", __func__, ret);
				goto relax_ipp_wakelock;
			}
		} else {
			pr_err(" Failed :check sec_path.%d\n", dev->sec_path);
			goto relax_ipp_wakelock;
		}

		mutex_lock(&dev->ipp_wakelock_mutex);

		if (dev->ipp_wakelock.active) {
			__pm_relax(&dev->ipp_wakelock);
			pr_info("ipp power up wake unlock.\n");
		}

		mutex_unlock(&dev->ipp_wakelock_mutex);
		break;

	case HISP_CPE_MCF_REQ:
		mutex_lock(&dev->ipp_work_lock);
		if (dev->hipp_refs[REFS_TYPE_MCF] > 0) {
			pr_err("[%s] : mcf refs Opened .\n", __func__);
			mutex_unlock(&dev->ipp_work_lock);
			return -EINVAL;
		}
		dev->hipp_refs[REFS_TYPE_MCF] = 1;
		mutex_unlock(&dev->ipp_work_lock);

		if ((ret = mcf_process(dev, args)) != 0) {
			pr_err("[%s] Failed : hispcpe_mcf_request.%d\n", __func__, ret);

			mutex_lock(&dev->ipp_work_lock);
			dev->hipp_refs[REFS_TYPE_MCF] = 0;
			mutex_unlock(&dev->ipp_work_lock);
			return ret;
		}

		mutex_lock(&dev->ipp_work_lock);
		dev->hipp_refs[REFS_TYPE_MCF] = 0;
		mutex_unlock(&dev->ipp_work_lock);
		break;

	case HISP_CPE_VBK_REQ:
		mutex_lock(&dev->ipp_work_lock);
		if (dev->hipp_refs[REFS_TYP_VBK] > 0) {
			pr_err("[%s] : vbk refs Opened .\n", __func__);
			mutex_unlock(&dev->ipp_work_lock);
			return -EINVAL;
		}
		dev->hipp_refs[REFS_TYP_VBK] = 1;
		mutex_unlock(&dev->ipp_work_lock);

		if ((ret = vbk_process(dev, args)) != 0) {
			pr_err("[%s] Failed : hispcpe_vbk_request.%d\n", __func__, ret);
			mutex_lock(&dev->ipp_work_lock);
			dev->hipp_refs[REFS_TYP_VBK] = 0;
			mutex_unlock(&dev->ipp_work_lock);
			return ret;
		}
		mutex_lock(&dev->ipp_work_lock);
		dev->hipp_refs[REFS_TYP_VBK] = 0;
		mutex_unlock(&dev->ipp_work_lock);
		break;

	default:
		if ((ret = hispcpe_ioctl_cmd_slam_reorder_aux(dev, cmd, args, buf)) != 0)
			return ret;
		break;
	}

	return 0;

relax_ipp_wakelock:
	hispcpe_ioctl_relax_ipp_wakelock(dev);
	return -EINVAL;
}

static int hispcpe_ioctl_process_cmd(struct hispcpe_s *dev, unsigned int cmd,
						unsigned long args, struct memory_block_s *buf)
{
	int ret = 0;
	struct power_para_s para = { 0 };
	void __user *args_pwup = NULL;

	switch (cmd) {
	case HISP_CPE_PWRUP:
		args_pwup = (void __user *)(uintptr_t)args;

		if (args_pwup == NULL) {
			pr_err("[%s] args_pwup.%pK\n", __func__, args_pwup);
			return -EINVAL;
		}

		if ((ret = copy_from_user(&para, args_pwup, sizeof(struct power_para_s))) != 0) {
			pr_err("[%s] copy_from_user.%d\n", __func__, ret);
			return -EFAULT;
		}

		i(" info : HISP_CPE_PWRUP.pw_flag.%d\n", para.pw_flag);
		mutex_lock(&dev->ipp_wakelock_mutex);

		if (!dev->ipp_wakelock.active) {
			__pm_stay_awake(&dev->ipp_wakelock);
			pr_info("ipp power up wake lock.\n");
		}

		mutex_unlock(&dev->ipp_wakelock_mutex);

		if (0 == para.pw_flag) {
			if ((ret = hispcpe_powerup(dev)) != 0) {
				pr_err("[%s] Failed : hispcpe_powerup.%d\n", __func__, ret);
				goto relax_ipp_wakelock;
			}
		} else {
			pr_err(" Failed : check up layer pw_flag.%d\n", para.pw_flag);
			goto relax_ipp_wakelock;
		}

		break;

	default:
		if ((ret = hispcpe_ioctl_cmd_cpe(dev, cmd, args, buf)) != 0)
			return ret;
		break;
	}

	return 0;

relax_ipp_wakelock:
	hispcpe_ioctl_relax_ipp_wakelock(dev);
	return -EINVAL;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wframe-larger-than="
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
static long hispcpe_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
	if (filp == NULL) {
		pr_err("[%s] filp is null\n", __func__);
		return -EINVAL;
	}
	struct hispcpe_s *dev = NULL;
	int r_ret = 0;
	struct memory_block_s buf = { 0 };

	d("+\n");

	if ((dev = get_cpedev_bymisc((struct miscdevice *)filp->private_data)) == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	if (!dev->initialized) {
		pr_err("[%s] Failed : CPE Device Not Exist.%d\n", __func__, dev->initialized);
		return -ENXIO;
	}

	pr_info("[%s] cmd.0x%x\n", __func__, cmd);

	if ((r_ret = hispcpe_ioctl_process_cmd(dev, cmd, args, &buf)) != 0)
		return r_ret;

	d("-\n");
	return 0;
}
#pragma GCC diagnostic pop

static int hispcpe_open(struct inode *inode, struct file *filp)
{
	struct hispcpe_s *dev = NULL;
	d("+\n");

	if ((dev = get_cpedev_bymisc((struct miscdevice *)filp->private_data)) == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	if (!dev->initialized) {
		pr_err("[%s] Failed : CPE Device Not Exist.%d\n", __func__, dev->initialized);
		return -ENXIO;
	}

	mutex_lock(&dev->dev_lock);

	if (dev->open_refs != 0) {
		pr_err("[%s] Failed: Opened, open_refs.0x%x\n", __func__, dev->open_refs);
		mutex_unlock(&dev->dev_lock);
		return -EBUSY;
	}

	dev->open_refs++;
	mutex_unlock(&dev->dev_lock);
	d("-\n");
	return 0;
}

static int hispcpe_release(struct inode *inode, struct file *filp)
{
	struct hispcpe_s *dev = NULL;
	unsigned int ret = 0;
	d("+\n");

	if ((dev = get_cpedev_bymisc((struct miscdevice *)filp->private_data)) == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EINVAL;
	}

	if (!dev->initialized) {
		pr_err("[%s] Failed : CPE Device Not Exist.%d\n", __func__, dev->initialized);
		return -ENXIO;
	}

	mutex_lock(&dev->dev_lock);

	if (dev->open_refs <= 0) {
		pr_err("[%s] Failed: Closed, open_refs.0x%x\n", __func__, dev->open_refs);
		mutex_unlock(&dev->dev_lock);
		return -EBUSY;
	}

	dev->open_refs = 0;

	if (atomic_read(&dev->cpe_supply_refs) > 0) {
		pr_err("[%s] Failed: cpe_supply Opened.0x%x\n", __func__, atomic_read(&dev->cpe_supply_refs));

		if ((ret = hispcpe_powerdn(dev)) != 0)
			pr_err("[%s] Failed : hispcpe_powerdn.%d\n", __func__, ret);
	}

	if (dev->devbuf != NULL) {
		ret = hispcpe_unmap_kernel(dev);

		if (ret < 0)
			pr_err("[%s] Failed : hispcpe_umap_kernel.%d\n", __func__, ret);
	}

	mutex_unlock(&dev->dev_lock);
	d("-\n");
	return 0;
}

static ssize_t hispcpe_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
	char *s = buf;
	d("+\n");
	d("-\n");
	return (ssize_t)(s - buf);
}

static ssize_t hispcpe_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t count)
{
	d("+\n");
	d("-\n");
	return (ssize_t)count;
}

static DEVICE_ATTR(hispcpe, (S_IRUGO | S_IWUSR | S_IWGRP), hispcpe_show, hispcpe_store);

static struct file_operations hispcpe_fops = {
	.owner          = THIS_MODULE,
	.open           = hispcpe_open,
	.release        = hispcpe_release,
	.unlocked_ioctl = hispcpe_ioctl,
};

static struct miscdevice hispcpe_miscdev = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = KBUILD_MODNAME,
	.fops   = &hispcpe_fops,
};

static int hispcpe_register_irq(struct hispcpe_s *dev)
{
	int i = 0, ret = 0;
	d("+\n");

	for (i = 0; i < hipp_min(MAX_HISP_CPE_IRQ, dev->irq_num); i ++) {/*lint !e574 */
		if ((ret = request_irq(dev->irq[i], hispcpe_irq_handler, 0, "HISP_CPE_IRQ", (void *)dev)) != 0) {
			pr_err("[%s] Failed : %d.request_irq.%d\n", __func__, i, ret);
			return ret;
		}

		pr_info("[%s] Hisp CPE %d.IRQ.%d, handler.%pK\n", __func__, i, dev->irq[i], hispcpe_irq_handler);
	}

	d("-\n");
	return 0;
}

static int hispcpe_ioremap_reg(struct hispcpe_s *dev)
{
	struct device *device = NULL;
	int i = 0;
	d("+\n");
	device = &dev->pdev->dev;

	for (i = 0; i < hipp_min(MAX_HISP_CPE_REG, dev->reg_num); i ++) {/*lint !e574 */
		dev->reg[i] = devm_ioremap_resource(device, dev->r[i]);

		if (dev->reg[i] == NULL) {
			pr_err("[%s] Failed : %d.devm_ioremap_resource.%pK\n", __func__, i, dev->reg[i]);
			return -ENOMEM;
		}

		pr_debug("[%s] Hisp CPE %d.Resource.%pK.(n.%s, s.0x%llx, e.0x%llx, f.0x%lx) > %pK\n", __func__, i, dev->r[i],
				 dev->r[i]->name, (unsigned long long)dev->r[i]->start, (unsigned long long)dev->r[i]->end, dev->r[i]->flags,
				 dev->reg[i]);
	}

	d("-\n");
	return 0;
}

static int hispcpe_resource_init(struct hispcpe_s *dev)
{
	int ret = 0;

	if (dev == NULL) {
		e("Failed : hipp dev.NULL\n");
		return -ENODEV;
	}

	if ((ret = hispcpe_register_irq(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_register_irq.%d\n", __func__, ret);
		return ret;
	}

	if ((ret = hispcpe_ioremap_reg(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_ioremap_reg.%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int hipp_enable_iommu(struct hispcpe_s *dev)
{
	struct iommu_domain *domain = NULL;
	struct device *device = NULL;
	i("+\n");

	if (dev == NULL) {
		e("Failed : hipp smmu dev.%pK\n", dev);
		return -ENODEV;
	}

	if (dev->pdev == NULL) {
		e("Failed : pdev.%pK\n", dev->pdev);
		return -ENODEV;
	}

	device = &dev->pdev->dev;
	dma_set_mask_and_coherent(device, DMA_BIT_MASK(64));/*lint !e598 !e648*/
	domain = iommu_get_domain_for_dev(device);

	if (domain == NULL) {
		e("Failed : iommu_get_domain_for_dev fail\n");
		return -EINVAL;
	}

	dev->domain = domain;
	return 0; /*lint !e715 !e438*/
}

static void hipp_disable_iommu(struct hispcpe_s *dev)
{
	if (dev->domain != NULL)
		dev->domain = NULL;
}

static int hispcpe_getdts_pwrclk_cp_supply(struct hispcpe_s *dev, struct device *device)
{
	if (IS_ERR(dev->cpe_supply)) {
		pr_err("[%s] Failed : CPE devm_regulator_get.%pK\n", __func__, dev->cpe_supply);
		return -EINVAL;
	}

	pr_info("[%s] Hisp CPE cpe_supply.%pK\n", __func__, dev->cpe_supply);
	dev->jpg_clk = devm_clk_get(device, "clk_jpg_func");

	if (IS_ERR(dev->jpg_clk)) {
		pr_err("get jpg_clk failed");
		return -EINVAL;
	}

	return 0;
}

static int hispcpe_set_denver_clk_rate_map(struct hispcpe_s *dev)
{
	if (dev->version == IPP_DENVER) {
		clk_rate_map[0] = DENV_CLK_TURBO_RATE;
		clk_rate_map[1] = DENV_CLK_NORMAL_RATE;
		clk_rate_map[2] = DENV_CLK_SVS_RATE;
		clk_rate_map[3] = DENV_CLK_LOW_SVS_RATE;
		pr_info("ipp jpgclk 1.%d 2.%d 3.%d 4.%d\n",
				clk_rate_map[0], clk_rate_map[1], clk_rate_map[2], clk_rate_map[3]);
	}

	return 0;
}

static int hispcpe_getdts_pwrclk(struct hispcpe_s *dev)
{
	struct device *device = NULL;
	struct device_node *np = NULL;
	unsigned int ret = 0;
	d("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -ENXIO;
	}

	if (dev->pdev == NULL) {
		pr_err("[%s] Failed : pdev.%pK\n", __func__, dev->pdev);
		return -ENXIO;
	}

	device = &dev->pdev->dev;
	dev->cpe_supply = devm_regulator_get(device, "ipp-cpe");

	if (hispcpe_getdts_pwrclk_cp_supply(dev, device) != 0)
		return -EINVAL;

	np = device->of_node;

	if (NULL == np) {
		pr_err("%s: of node NULL", __func__);
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "platform-version", &dev->version);

	if (ret) {
		pr_err("Failed : ipp get version, ret:%d", ret);
		dev->version = IPP_PHOENIX;
	}

	hispcpe_set_denver_clk_rate_map(dev);

	ret = of_property_read_u32(np, "clock-value", &dev->jpg_clk_rate);

	if (ret) {
		pr_err("ipp get rate failed, ret:%d", ret);
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "clock-low-frequency", &dev->jpgclk_low_frequency);

	if (ret) {
		pr_err("ipp get jpgclk_low_frequency failed, ret:%d", ret);
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "power-off-frequency", &dev->jpgclk_off_frequency);

	if (ret) {
		pr_err("ipp get jpgclk_off_frequency failed, ret:%d", ret);
		return -EINVAL;
	}

	pr_err("ipp get jpgclk frequency 1:%d 2:%d 3:%d", dev->jpg_clk_rate, dev->jpgclk_low_frequency,
		   dev->jpgclk_off_frequency);
	i("-\n");
	return 0;
}

int hispcpe_get_irq(int index)
{
	struct device_node *np = NULL;
	char *name = DTS_NAME_HISI_IPP;
	int irq = 0;
	np = of_find_compatible_node(NULL, NULL, name);

	if (np == NULL) {
		pr_err("[%s] Failed : %s.of_find_compatible_node.%pK\n", __func__, name, np);
		return -ENXIO;
	}

	irq = irq_of_parse_and_map(np, index);

	if (!irq) {
		pr_err("[%s] Failed : irq_of_parse_and_map.%d\n", __func__, irq);
		return -ENXIO;
	}

	pr_info("%s: comp.%s, cpe irq.%d.\n", __func__, name, irq);
	return irq;
}

static int hispcpe_getdts_irq(struct hispcpe_s *dev)
{
	struct device *device = NULL;
	int i = 0, irq = 0, ret = 0;
	d("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -ENXIO;
	}

	if (dev->pdev == NULL) {
		pr_err("[%s] Failed : pdev.%pK\n", __func__, dev->pdev);
		return -ENXIO;
	}

	device = &dev->pdev->dev;

	if ((ret = of_property_read_u32(device->of_node, "irq-num", (unsigned int *)(&dev->irq_num))) < 0) {
		pr_err("[%s] Failed: irq-num of_property_read_u32.%d\n", __func__, ret);
		return -EINVAL;
	}

	pr_info("[%s] Hisp CPE irq_num.%d\n", __func__, dev->irq_num);

	for (i = 0; i < hipp_min(MAX_HISP_CPE_IRQ, dev->irq_num); i ++) {/*lint !e574 */
		if ((irq = hispcpe_get_irq(i)) <= 0) {
			pr_err("[%s] Failed : platform_get_irq.%d\n", __func__, irq);
			return -EINVAL;
		}

		dev->irq[i] = irq;
		pr_info("[%s] Hisp CPE %d.IRQ.%d\n", __func__, i, dev->irq[i]);
	}

	i("-\n");
	return 0;
}

static int hispcpe_getdts_reg(struct hispcpe_s *dev)
{
	struct device *device = NULL;
	int i = 0, ret = 0;
	d("+\n");

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -ENXIO;
	}

	if (dev->pdev == NULL) {
		pr_err("[%s] Failed : pdev.%pK\n", __func__, dev->pdev);
		return -ENXIO;
	}

	device = &dev->pdev->dev;

	if ((ret = of_property_read_u32(device->of_node, "reg-num", (unsigned int *)(&dev->reg_num))) < 0) {
		pr_err("[%s] Failed: reg-num of_property_read_u32.%d\n", __func__, ret);
		return -EINVAL;
	}

	pr_info("[%s] Hisp CPE reg_num.%d\n", __func__, dev->reg_num);

	for (i = 0; i < hipp_min(MAX_HISP_CPE_REG, dev->reg_num); i ++) {/*lint !e574 */
		if ((dev->r[i] = platform_get_resource(dev->pdev, IORESOURCE_MEM, i)) == NULL) {
			pr_err("[%s] Failed : platform_get_resource.%pK\n", __func__, dev->r[i]);
			return -ENXIO;
		}

		pr_debug("[%s] Hisp CPE %d.Resource.%pK.(n.%s, s.0x%llx, e.0x%llx, f.0x%lx)\n", __func__, i, dev->r[i],
				 dev->r[i]->name, (unsigned long long)dev->r[i]->start, (unsigned long long)dev->r[i]->end, dev->r[i]->flags);
	}

	i("-\n");
	return 0;
}

static int hispcpe_getdts(struct hispcpe_s *dev)
{
	int ret = 0;

	if ((ret = hispcpe_getdts_pwrclk(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_getdts_pwrclk.%d\n", __func__, ret);
		return ret;
	}

	if ((ret = hispcpe_getdts_irq(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_getdts_irq.%d\n", __func__, ret);
		return ret;
	}

	if ((ret = hispcpe_getdts_reg(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_getdts_reg.%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int hispcpe_attach_misc(struct hispcpe_s *dev, struct miscdevice *mdev)
{
	i("+\n");

	if (dev == NULL || mdev == NULL) {
		pr_err("[%s] Failed : dev.%pK, mdev.%pK\n", __func__, dev, mdev);
		return -EINVAL;
	}

	dev->miscdev = hispcpe_miscdev;
	i("-\n");
	return 0;
}

unsigned int get_cpe_addr_da(void)
{
	if (hispcpe_dev == NULL) {
		pr_err("[%s], NONE cpe_mem_info!\n", __func__);
		return 0;
	}

	return hispcpe_dev->daddr;
}

void *get_cpe_addr_va(void)
{
	if (hispcpe_dev == NULL) {
		pr_err("[%s], NONE cpe_mem_info!\n", __func__);
		return 0;
	}

	return hispcpe_dev->virt_addr;
}
//lint -save -e454 -e455
int ippdev_lock(void)
{
	d("+\n");

	if (hispcpe_dev == NULL) {
		pr_err("[%s], NONE cpe_mem_info!\n", __func__);
		return -1;
	}

	mutex_lock(&hispcpe_dev->lock);
	d("-\n");
	return 0;
}

int ippdev_unlock(void)
{
	d("+\n");

	if (hispcpe_dev == NULL) {
		pr_err("[%s], NONE cpe_mem_info!\n", __func__);
		return -1;
	}

	mutex_unlock(&hispcpe_dev->lock);
	d("-\n");
	return 0;
}
//lint -restore

static int hispcpe_probe_resource_init(struct hispcpe_s *dev, int *ret)
{
	if ((*ret = hispcpe_getdts(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_getdts.%d\n", __func__, *ret);
		return -1;
	}

	if ((*ret = hispcpe_resource_init(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_resource_init.%d\n", __func__, *ret);
		return -1;
	}

	if ((*ret = hipp_enable_iommu(dev)) != 0) {
		pr_err("[%s] Failed : hispcpe_resource_init.%d\n", __func__, *ret);
		return -1;
	}

	return 0;
}

static int hispcpe_probe_hipp_misc_register(struct hispcpe_s *dev, int *ret)
{
	if ((dev->ippdrv = hipp_register(CPE_UNIT, NONTRUS)) == NULL) {
		pr_err("[%s] Failed : hipp_register.%pK\n", __func__, dev->ippdrv);
		return -1;
	}

	if ((dev->sec_slamdrv = hipp_register(ORB_UNIT, NONTRUS)) == NULL) {
		pr_err("[%s] Failed : slam_register.%pK\n", __func__, dev->sec_slamdrv);
		return -1;
	}

	if ((dev->rdrdrv = hipp_register(REORDER_UNIT, NONTRUS)) == NULL) {
		pr_err("[%s] Failed : reorder_register.%pK\n", __func__, dev->rdrdrv);
		return -1;
	}

	if ((dev->cmpdrv = hipp_register(COMPARE_UNIT, NONTRUS)) == NULL) {
		pr_err("[%s] Failed : compare_register.%pK\n", __func__, dev->cmpdrv);
		return -1;
	}

	if ((*ret = misc_register((struct miscdevice *)&dev->miscdev)) != 0) {
		pr_err("[%s] Failed : misc_register.%d\n", __func__, *ret);
		return -1;
	}

	return 0;
}

static int hispcpe_probe_cmdlst_prepare(struct hispcpe_s *dev, int *ret)
{
	if ((*ret = device_create_file(dev->miscdev.this_device, &dev_attr_hispcpe)) != 0) {
		pr_err("[%s] Faield : hispcpe device_create_file.%d\n", __func__, *ret);
		return -1;
	}

	if ((*ret = cmdlst_priv_prepare()) != 0) {
		pr_err("[%s] Faield : hispcpe cmdlst_priv_prepare.%d\n", __func__, *ret);
		return -1;
	}

	return 0;
}

static int hispcpe_probe(struct platform_device *pdev)
{
	struct hispcpe_s *dev = NULL;
	int ret = 0;
	unsigned int i = 0;

	i("+\n");

	if ((dev = (struct hispcpe_s *)kzalloc(sizeof(struct hispcpe_s), GFP_KERNEL)) == NULL) {
		pr_err("[%s] Failed : kzalloc\n", __func__);
		return -ENOMEM;
	}

	dev->pdev = pdev;
	platform_set_drvdata(pdev, dev);

	if (hispcpe_probe_resource_init(dev, &ret) != 0)
		goto free_dev;

	dev->initialized = 0;
	init_waitqueue_head(&dev->cpe_wait);
	init_waitqueue_head(&dev->slam_wait);
	init_waitqueue_head(&dev->reorder_wait);
	init_waitqueue_head(&dev->compare_wait);
	hispcpe_attach_misc(dev, &hispcpe_miscdev);

	if (hispcpe_probe_hipp_misc_register(dev, &ret) != 0)
		goto free_iommu;

	if (hispcpe_probe_cmdlst_prepare(dev, &ret) != 0)
		goto clean_misc;

	dev->open_refs = 0;
	atomic_set(&dev->cpe_supply_refs, 0);

	atomic_set(&dev->mapbuf_ready, 0);
	mutex_init(&dev->ipp_work_lock);
	for (i = 0; i < REFS_TYP_MAX; i++)
		dev->hipp_refs[i] = 0;

	mutex_init(&dev->lock);
	wakeup_source_init(&dev->ipp_wakelock, "ipp_wakelock");
	mutex_init(&dev->ipp_wakelock_mutex);
	mutex_init(&dev->dev_lock);
	dev->initialized = 1;
	hispcpe_dev = dev;
	dev->sec_path = 0;
	dev->sec_req = 0;
	i("-\n");
	return 0;
clean_misc:
	misc_deregister(&dev->miscdev);
free_iommu:
	hipp_disable_iommu(dev);
free_dev:
	kfree(dev);
	dev = NULL;
	return ret;
}

static int hispcpe_remove(struct platform_device *pdev)
{
	struct hispcpe_s *dev = NULL;
	unsigned int i = 0;

	i("+\n");

	if ((dev = (struct hispcpe_s *)platform_get_drvdata(pdev)) == NULL) {
		pr_err("[%s] Failed : platform_get_drvdata, dev.%pK.pdev.%pK\n", __func__, dev, pdev);
		return -ENODEV;
	}

	misc_deregister(&dev->miscdev);
	dev->open_refs = 0;
	atomic_set(&dev->cpe_supply_refs, 0);

	atomic_set(&dev->mapbuf_ready, 0);
	mutex_destroy(&dev->ipp_work_lock);
	for (i = 0; i < REFS_TYP_MAX; i++)
		dev->hipp_refs[i] = 0;

	wakeup_source_trash(&dev->ipp_wakelock);
	mutex_destroy(&dev->ipp_wakelock_mutex);
	mutex_destroy(&dev->dev_lock);

	if (hipp_unregister(CPE_UNIT))
		pr_err("[%s] Failed : hipp_unregister\n", __func__);

	if (hipp_unregister(ORB_UNIT))
		pr_err("[%s] Failed : slam_unregister\n", __func__);

	if (hipp_unregister(REORDER_UNIT))
		pr_err("[%s] Failed : rdr_unregister\n", __func__);

	if (hipp_unregister(COMPARE_UNIT))
		pr_err("[%s] Failed : cmp_unregister\n", __func__);

	if (dev->ippdrv != NULL)
		dev->ippdrv = NULL;

	if (dev->sec_slamdrv != NULL)
		dev->sec_slamdrv = NULL;

	if (dev->rdrdrv != NULL)
		dev->rdrdrv = NULL;

	if (dev->cmpdrv != NULL)
		dev->cmpdrv = NULL;

	dev->initialized = 0;
	kfree(dev);
	dev = NULL;
	i("-\n");
	return 0;
}

#ifdef CONFIG_OF
static struct of_device_id hisiipp_of_id[] = {
	{.compatible = DTS_NAME_HISI_IPP},
	{}
};
#endif

static struct platform_driver hispcpe_pdrvr = {
	.probe          = hispcpe_probe,
	.remove         = hispcpe_remove,
	.driver         = {
		.name           = "hisiipp",
		.owner          = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(hisiipp_of_id),
#endif
	},
};

module_platform_driver(hispcpe_pdrvr);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hisilicon ISP CPE Driver");
MODULE_AUTHOR("isp");

/*lint +e21 +e846 +e514 +e778 +e866 +e84 +e429 +e613 +e668*/
