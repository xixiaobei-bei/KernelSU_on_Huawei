/*
 *  ISP driver, isp.c
 *
 * Copyright (c) 2018 ISP Technologies CO., Ltd.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/iommu.h>
#include <linux/hisi-iommu.h>
#include <linux/dma-mapping.h>
#include "isp_ddr_map.h"
#include "hisp_internel.h"
#include <linux/platform_data/remoteproc_hisp.h>

/* Regs Base */
#define ISP_BASE_ADDR_SSMMU         0x6000
#define ISP_BASE_ADDR_CVDR_SRT      0x2E000
#define ISP_BASE_ADDR_CVDR_RT       0x22000
#define ISP_SUB_CTRL_BASE_ADDR      0x183000
#define ISP_020010_MODULE_CGR_TOP   0x020010

/* regs offset*/
#define CVDR_SRT_AXI_CFG_VP_WR_25           0xCE4
#define CVDR_SRT_AXI_CFG_NR_RD_1            0xC44
#define MID_FOR_JPGEN                       0xF

/*cvdr*/
#define CVDR_RT_CVDR_CFG_REG            0x0
#define CVDR_RT_CVDR_WR_QOS_CFG_REG     0xC
#define CVDR_RT_CVDR_RD_QOS_CFG_REG     0x10
#define CVDR_SRT_CVDR_CFG_REG           0x0
#define CVDR_SRT_CVDR_WR_QOS_CFG_REG    0xC
#define CVDR_SRT_CVDR_RD_QOS_CFG_REG    0x10
#define CVDR_SRT_VP_WR_IF_CFG_10_REG    0xC8
#define CVDR_SRT_VP_WR_IF_CFG_11_REG    0xD8
#define CVDR_SRT_VP_WR_IF_CFG_25_REG    0x1B8
#define CVDR_SRT_NR_RD_CFG_1_REG        0xA10
#define CVDR_SRT_LIMITER_NR_RD_1_REG    0xA18
#define CVDR_SRT_NR_RD_CFG_3_REG        0xA30

#define SUB_CTRL_ISP_FLUX_CTRL2_0_REG   0x190
#define SUB_CTRL_ISP_FLUX_CTRL2_1_REG   0x194
#define SUB_CTRL_ISP_FLUX_CTRL3_0_REG   0x198
#define SUB_CTRL_ISP_FLUX_CTRL3_1_REG   0x19C

#define VP_WR_REG_OFFSET (0x10)
#define VP_RD_REG_OFFSET (0x20)
#define CVDR_VP_WR_NBR    (38)
#define CVDR_VP_RD_NBR    (22)
#define CVDR_CLK_ENABLE   (0x40020)

struct hisp_cvdr_device {
	int ispsmmu_init_byap;
	u64 pgd_base;
} hisp_cvdr_dev;

//lint -save -e529 -e438
int ispmmu_init(void)
{

	return 0;
}
//lint -restore
int ispmmu_exit(void)
{
	pr_info("[%s] +\n", __func__);

	return 0;
}

int hisp_cvdr_getdts(struct device_node *np)
{
	int ret;
	struct hisp_cvdr_device *dev = &hisp_cvdr_dev;

	if (np == NULL) {
		pr_err("[%s] Failed : np.%pK\n", __func__, np);
		return -ENODEV;
	}

	pr_info("[%s] +\n", __func__);
	ret = of_property_read_u32(np, "ispsmmu-init-byap",
			(unsigned int *)(&dev->ispsmmu_init_byap));
	if (ret < 0) {
		pr_err("[%s] Failed: ispsmmu-init-byap of_property_read_u32.%d\n",
				__func__, ret);
		return -EINVAL;
	}
	pr_info("[%s] isp-smmu-flag.0x%x\n", __func__, dev->ispsmmu_init_byap);
	pr_info("[%s] -\n", __func__);

	return 0;
}

static struct hisp_mdc_device hisp_mdc_dev;

u64 get_mdc_addr_pa(void)
{
	struct hisp_mdc_device *dev = &hisp_mdc_dev;

	if (dev == NULL) {
		pr_err("[%s], NONE mdc_mem_info!\n", __func__);
		return 0;
	}

	pr_debug("[%s], FASTCMA_MDC_DEBUG!\n", __func__);
	if (dev->mdc_dma_addr == 0x0) {
		pr_err("[%s], NONE isp_mdc_fstcma mdc_dma_addr!\n", __func__);
		return 0;
	}

	return dev->mdc_dma_addr;
}

int hisp_mdcmem_pa_set(u64 mdc_phymem_addr,
		unsigned int mdc_mem_size, int shared_fd)
{
	struct hisp_mdc_device *dev = &hisp_mdc_dev;
	int ret = 0;

	pr_info("[%s] + : addr.0x%llx, size.0x%x", __func__,
			mdc_phymem_addr, mdc_mem_size);
	if (dev->isp_mdc_count > 0) {
		pr_err("[%s] Failed : isp_mdc_count.%u\n",
				__func__, dev->isp_mdc_count);
		return -EINVAL;
	}

	if (mdc_phymem_addr == 0) {
		pr_err("[%s] Failed : mdc_phymem_addr.0x%llx\n",
				__func__, mdc_phymem_addr);
		return -EINVAL;
	}

	if (mdc_mem_size < dev->size) {
		pr_err("[%s] Failed : mdc_mem_size err.%u\n",
				__func__, mdc_mem_size);
		return -EINVAL;
	}

	ret = hispmdc_map_fw(dev->iova, mdc_phymem_addr, dev->size,
				IOMMU_READ | IOMMU_WRITE | IOMMU_CACHE);
	if (ret) {
		pr_err("[%s] Failed :hispmdc_map_fw.%d\n", __func__, ret);
		return -EINVAL;
	}

	dev->mdc_dma_addr = (dma_addr_t)mdc_phymem_addr;
	dev->isp_mdc_count++;
	pr_info("[%s] - secmem_count.%u\n", __func__, dev->isp_mdc_count);

	set_shared_mdc_pa_addr(dev->mdc_dma_addr);
	return 0;
}
EXPORT_SYMBOL(hisp_mdcmem_pa_set);

int hisp_mdcmem_pa_release(void)
{
	struct hisp_mdc_device *dev = &hisp_mdc_dev;
	int ret = 0;

	if (dev->isp_mdc_count <= 0) {
		pr_err("[%s] Failed : isp_mdc_count.%u\n",
				__func__, dev->isp_mdc_count);
		return -EINVAL;
	}

	ret = hispmdc_unmap_fw(dev->iova, dev->size);
	if (ret < 0) {
		pr_err("[%s] Failed : hispmdc_unmap_fw err.%d\n",
				__func__, ret);
		return -EINVAL;
	}

	dev->mdc_dma_addr  = 0;
	dev->isp_mdc_count--;
	pr_info("[%s] isp_mdc_count.%u\n", __func__, dev->isp_mdc_count);
	return 0;
}
EXPORT_SYMBOL(hisp_mdcmem_pa_release);

void hisp_mdc_dev_init(void)
{
	struct hisp_mdc_device *dev = &hisp_mdc_dev;

	dev->isp_mdc_count = 0;
	dev->mdc_dma_addr  = 0;
	dev->iova = MEM_MDC_DA;
	dev->size = MEM_MDC_SIZE;
}

void hisp_mdc_dev_deinit(void)
{
	struct hisp_mdc_device *dev = &hisp_mdc_dev;

	dev->isp_mdc_count = 0;
	dev->mdc_dma_addr  = 0;
}

int hisp_mdc_getdts(struct device_node *np)
{
	int ret;
	struct hisp_mdc_device *dev = &hisp_mdc_dev;

	if ((!np) || (!dev)) {
		pr_err("[%s] Failed : np.%pK, dev.%pK\n", __func__, np, dev);
		return -ENODEV;
	}

	pr_info("[%s] +\n", __func__);
	ret = of_property_read_u32(np, "isp-mdc-flag",
			(unsigned int *)(&dev->isp_mdc_flag));
	if (ret < 0) {
		pr_err("[%s] Failed: isp-mdc-flag of_property_read_u32.%d\n",
				__func__, ret);
		return -EINVAL;
	}
	pr_info("[%s] isp_mdc_flag.0x%x\n", __func__, dev->isp_mdc_flag);
	pr_info("[%s] -\n", __func__);

	return 0;
}

//lint -save -e529 -e438
int get_isp_mdc_flag(void)
{
	struct hisp_mdc_device *dev = &hisp_mdc_dev;

	return dev->isp_mdc_flag;
}
//lint -restore

int ispcvdr_init(void)
{
	void __iomem *isp_regs_base;
	void __iomem *isprt_base;
	void __iomem *ispsrt_base;
	void __iomem *ispsctrl_base;
	unsigned int value;

	pr_info("[%s] +\n", __func__);

	isp_regs_base = get_regaddr_by_pa(ISPCORE);
	if (unlikely(!isp_regs_base)) {
		pr_err("[%s] isp_regs_base failed!\n", __func__);
		return -ENOMEM;
	}

	isprt_base = isp_regs_base + ISP_BASE_ADDR_CVDR_RT;
	ispsrt_base = isp_regs_base + ISP_BASE_ADDR_CVDR_SRT;
	ispsctrl_base = isp_regs_base + ISP_SUB_CTRL_BASE_ADDR;

	value = readl(ISP_020010_MODULE_CGR_TOP + isp_regs_base);
	value |= CVDR_CLK_ENABLE;
	writel(value, ISP_020010_MODULE_CGR_TOP + isp_regs_base);

	/* CVDR RT*/
	writel(0x0B0B4001, isprt_base + CVDR_RT_CVDR_CFG_REG);
	/* CVDR SRT*/
	writel(0x0B132201, ispsrt_base + CVDR_SRT_CVDR_CFG_REG);
	//CVDR_RT QOS
	writel(0xF8765432, isprt_base + CVDR_RT_CVDR_WR_QOS_CFG_REG);
	writel(0xF8122334, isprt_base + CVDR_RT_CVDR_RD_QOS_CFG_REG);

	//CVDR_SRT QOS
	writel(0xD0765432, ispsrt_base + CVDR_SRT_CVDR_WR_QOS_CFG_REG);
	writel(0xD0122334, ispsrt_base + CVDR_SRT_CVDR_RD_QOS_CFG_REG);

	//CVDR SRT PORT STOP
	writel(0x00020000, ispsrt_base + CVDR_SRT_NR_RD_CFG_3_REG);
	writel(0x00020000, ispsrt_base + CVDR_SRT_VP_WR_IF_CFG_10_REG);
	writel(0x00020000, ispsrt_base + CVDR_SRT_VP_WR_IF_CFG_11_REG);

	//JPGENC limiter DU=8
	writel(0x00000000, ispsrt_base + CVDR_SRT_VP_WR_IF_CFG_25_REG);
	writel(0x80060100, ispsrt_base + CVDR_SRT_NR_RD_CFG_1_REG);
	writel(0x0F001111, ispsrt_base + CVDR_SRT_LIMITER_NR_RD_1_REG);

	//SRT READ
	writel(0x00026E10, ispsctrl_base + SUB_CTRL_ISP_FLUX_CTRL2_0_REG);
	writel(0x0000021F, ispsctrl_base + SUB_CTRL_ISP_FLUX_CTRL2_1_REG);
	//SRT WRITE
	writel(0x00027210, ispsctrl_base + SUB_CTRL_ISP_FLUX_CTRL3_0_REG);
	writel(0x0000024E, ispsctrl_base + SUB_CTRL_ISP_FLUX_CTRL3_1_REG);

	pr_info("[%s] -\n", __func__);

	return 0;
}
