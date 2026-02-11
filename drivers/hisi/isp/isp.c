/*
 * hisilicon ISP driver, isp.c
 *
 * Copyright (c) 2018 Hisilicon Technologies CO., Ltd.
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
#include <linux/platform_data/remoteproc-hisi.h>

/* Regs Base */

#define ISP_020010_MODULE_CGR_TOP   0x020010

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

int hisi_isp_cvdr_getdts(struct device_node *np)
{
	int ret;
	struct hisp_cvdr_device *dev = &hisp_cvdr_dev;

	if (np == NULL) {
		pr_err("[%s] Failed : np.%pK\n", __func__, np);
		return -ENODEV;
	}

	pr_info("[%s] +\n", __func__);
	if ((ret = of_property_read_u32(np, "ispsmmu-init-byap", (unsigned int *)(&dev->ispsmmu_init_byap))) < 0 ) {
		pr_err("[%s] Failed: ispsmmu-init-byap of_property_read_u32.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("[%s] isp-smmu-flag.0x%x\n", __func__, dev->ispsmmu_init_byap);
	pr_info("[%s] -\n", __func__);

	return 0;
}

int hisi_isp_set_pgd(void)
{
	int ret;

	if ((ret = hisp_get_pgd_base(&hisp_cvdr_dev.pgd_base)) < 0) {
		pr_err("[%s] Failed : hisp_get_pgd_base.%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static struct hisp_mdc_device hisp_mdc_dev;

u64 get_mdc_addr_pa(void)
{
	struct hisp_mdc_device *dev = &hisp_mdc_dev;

	if(dev == NULL)
	{
		pr_err("[%s], NONE mdc_mem_info!\n", __func__);
		return 0;
	}
	pr_debug("get_mdc_addr_pa FASTCMA_MDC_DEBUG!\n");
	if(dev->mdc_dma_addr == 0x0)
	{
		pr_err("[%s], NONE hisi_mdc_fstcma mdc_dma_addr!\n", __func__);
		return 0;
	}
	return dev->mdc_dma_addr;
}

int hisp_mdcmem_pa_set(u64 mdc_phymem_addr, unsigned int mdc_mem_size, int shared_fd)
{
	struct hisp_mdc_device *dev = &hisp_mdc_dev;
	int ret = 0;

	pr_info("[%s] + : addr.0x%llx, size.0x%x", __func__, mdc_phymem_addr, mdc_mem_size);
	if (dev->isp_mdc_count > 0){
		pr_err("[%s] Failed : isp_mdc_count.%u\n", __func__, dev->isp_mdc_count);
		return -EINVAL;
	}
	if (mdc_phymem_addr == 0) {
		pr_err("[%s] Failed : mdc_phymem_addr.0x%llx\n", __func__, mdc_phymem_addr);
		return -EINVAL;
	}
	if (mdc_mem_size < dev->size) {
		pr_err("[%s] Failed : mdc_mem_size err.%u\n", __func__, mdc_mem_size);
		return -EINVAL;
	}

	ret = hispmdc_map_fw(dev->iova, mdc_phymem_addr, dev->size, IOMMU_READ | IOMMU_WRITE | IOMMU_CACHE);
	if (ret) {
		pr_err("[%s] Failed :hispmdc_map_fw.%d\n", __func__, ret);
		return -EINVAL;
	}

	dev->mdc_dma_addr = (dma_addr_t)mdc_phymem_addr;
	dev->isp_mdc_count ++;
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
		pr_err("[%s] Failed : isp_mdc_count.%u\n", __func__, dev->isp_mdc_count);
		return -EINVAL;
	}

	if((ret = hispmdc_unmap_fw(dev->iova, dev->size)) < 0){
		pr_err("[%s] Failed : hispmdc_unmap_fw err.%d\n", __func__, ret);
		return -EINVAL;
	}
	dev->mdc_dma_addr  = 0;
	dev->isp_mdc_count --;
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

int hisi_isp_mdc_getdts(struct device_node *np)
{
	int ret;
	struct hisp_mdc_device *dev = &hisp_mdc_dev;

	if ((!np) || (!dev)) {
		pr_err("[%s] Failed : np.%pK, dev.%pK\n", __func__, np, dev);
		return -ENODEV;
	}

	pr_info("[%s] +\n", __func__);
	if ((ret = of_property_read_u32(np, "isp-mdc-flag", (unsigned int *)(&dev->isp_mdc_flag))) < 0 ) {
		pr_err("[%s] Failed: isp-mdc-flag of_property_read_u32.%d\n", __func__, ret);
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

