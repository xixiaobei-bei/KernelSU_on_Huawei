/*
 *  driver, hisp_rproc.c
 *
 * Copyright (c) 2013 ISP Technologies CO., Ltd.
 *
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/mod_devicetable.h>
#include <linux/amba/bus.h>
#include <linux/dma-mapping.h>
#include <linux/remoteproc.h>
#include <linux/platform_data/remoteproc_hisp.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/delay.h>
#include <linux/kfifo.h>
#include <linux/mutex.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/rproc_share.h>
#include <linux/sched/rt.h>
#include <linux/kthread.h>
#include <global_ddr_map.h>
#include <linux/iommu.h>
#include <linux/hisi-iommu.h>
#include <linux/miscdevice.h>
#include "hisp_internel.h"
#include "isp_ddr_map.h"
#include <linux/hisi/rdr_hisi_platform.h>
#include <securec.h>
#include <linux/random.h>

#define ISP_MEM_SIZE    0x10000
#define CRGPERIPH_PERPWRSTAT_ADDR       (0x158)
#define ISP_BOOT_MEM_ATTR         ((1 << 0) | (1 << 9))

static struct hisp_nsec nsec_rproc_dev;

static int nonsec_isp_find_textmem_va(struct rproc *rproc,
		void **va)
{
	struct rproc_mem_entry *dynmem = NULL;
	struct rproc_mem_entry *tmpmem = NULL;
	int ret;

	list_for_each_entry_safe(dynmem, tmpmem, &rproc->dynamic_mems, node) {
		if (dynmem->priv == NULL)
			continue;

		ret = strncmp(dynmem->priv, "ISP_MEM_TEXT", strlen("ISP_MEM_TEXT"));
		if (ret == 0) {
			*va = dynmem->va;
			return 0;
		}
	}

	return -EINVAL;
}

void dump_hisp_boot(struct rproc *rproc, unsigned int size)
{
}

static void set_isp_nonsec(void)
{
	pr_alert("[%s] +\n", __func__);
	atfisp_set_nonsec();
	pr_alert("[%s] -\n", __func__);
}

int hisp_nsec_jpeg_powerup(void)
{
	struct hisp_nsec *dev = &nsec_rproc_dev;
	int ret = 0;

	pr_info("[%s] +\n", __func__);
	mutex_lock(&dev->pwrlock);
	ret = hisp_subsys_powerup();
	if (ret != 0) {
		pr_err("[%s] Failed : ispup.%d\n", __func__, ret);
		mutex_unlock(&dev->pwrlock);
		return ret;
	}

	ret = ispcpu_qos_cfg();
	if (ret != 0) {
		pr_err("[%s] Failed : ispcpu_qos_cfg.%d\n",
				__func__, ret);
		goto isp_down;
	}
	set_isp_nonsec();

	ret = hisp_pwr_core_nsec_init();
	if (ret != 0) {
		pr_err("[%s] hisp_pwr_core_nsec_init.%d\n", __func__, ret);
		goto isp_down;
	}
	mutex_unlock(&dev->pwrlock);
	pr_info("[%s] -\n", __func__);

	return 0;

isp_down:
	if ((hisp_subsys_powerdn()) != 0)
		pr_err("[%s] Failed : ispdn\n", __func__);

	mutex_unlock(&dev->pwrlock);
	pr_info("[%s] -\n", __func__);

	return ret;
}
EXPORT_SYMBOL(hisp_nsec_jpeg_powerup);

/*lint -save -e631 -e613*/
int hisp_nsec_jpeg_powerdn(void)
{
	struct hisp_nsec *dev = &nsec_rproc_dev;
	int ret;

	pr_info("%s: +\n", __func__);
	mutex_lock(&dev->pwrlock);
	ret = hisp_pwr_core_nsec_exit();
	if (ret != 0)
		pr_err("%s: hisp_pwr_core_nsec_exit failed, ret.%d\n",
			__func__, ret);

	ret = hisp_subsys_powerdn();
	if (ret != 0)
		pr_err("%s: hisp_subsys_powerdn failed, ret.%d\n",
			__func__, ret);

	mutex_unlock(&dev->pwrlock);
	pr_info("%s: -\n", __func__);

	return 0;
}
EXPORT_SYMBOL(hisp_nsec_jpeg_powerdn);
/*lint -restore */

static int nonsec_isp_boot_image_prepare(struct rproc *rproc)
{
	struct hisp_nsec *dev = &nsec_rproc_dev;
	void *va = NULL;
	int ret;

	if (rproc == NULL) {
		pr_err("%s: rproc is NULL\n", __func__);
		return -EINVAL;
	}

	ret = nonsec_isp_find_textmem_va(rproc, &va);
	if (ret != 0) {
		pr_err("%s: find text mem va fail\n", __func__);
		return ret;
	}

	ret = memcpy_s(dev->isp_dma_va, ISP_MEM_SIZE, va, PAGE_SIZE);
	if (ret != 0) {
		pr_err("%s: copy text to boot image fail\n", __func__);
		return ret;
	}

	if (hisp_get_smmuc3_flag() == 1) {
		ret = iommu_map(rproc->domain, 0, dev->isp_dma,
			PAGE_SIZE, ISP_BOOT_MEM_ATTR);
		if (ret != 0) {
			pr_err("%s iommu_map failed: ret %d len\n",
				__func__, ret);
			return ret;
		}
	}

	return 0;
}

static int nonsec_isp_boot_image_release(void)
{
	struct device *subdev = NULL;
	size_t phy_len = 0;

	if (hisp_get_smmuc3_flag() == 0)
		return 0;

	subdev = get_isp_device();
	if (subdev == NULL) {
		pr_err("%s: platform dev is NULL!\n", __func__);
		return 0;
	}

	phy_len = hisi_iommu_unmap_fast(subdev, 0, PAGE_SIZE);
	if (phy_len != PAGE_SIZE) {
		pr_err("%s: iommu_unmap failed: phy_len 0x%lx size 0x%lx\n",
				__func__, phy_len, PAGE_SIZE);
		return -EINVAL;
	}

	return 0;
}

int nonsec_isp_device_enable(struct rproc *rproc)
{
	struct hisp_nsec *dev = &nsec_rproc_dev;
	unsigned int canary = 0;
	int ret;

	mutex_lock(&dev->pwrlock);
	ret = hisp_subsys_powerup();
	if (ret != 0) {
		pr_err("[%s] Failed : hisp_subsys_powerup.%d\n", __func__, ret);
		mutex_unlock(&dev->pwrlock);
		return ret;
	}

	ret = ispcpu_qos_cfg();
	if (ret != 0) {
		pr_err("[%s] Failed : ispcpu_qos_cfg.%d\n", __func__, ret);
		goto isp_down;
	}
	set_isp_nonsec();

	ret = hisp_pwr_core_nsec_init();
	if (ret != 0) {
		pr_err("[%s] Failed : hisp_pwr_core_nsec_init.%d\n",
			__func__, ret);
		goto isp_down;
	}

	ret = nonsec_isp_boot_image_prepare(rproc);
	if (ret != 0) {
		pr_err("[%s] Failed : nonsec_isp_boot_image_prepare.%d\n",
			__func__, ret);
		goto isp_exit;
	}

	get_random_bytes(&canary, sizeof(canary));
	ret = hisp_wait_rpmsg_completion();
	if (ret != 0)
		goto boot_exit;

	ret = hisp_pwr_cpu_nsec_dst(dev->isp_dma, canary);
	if (ret != 0) {
		pr_err("[%s] Failed : hisp_pwr_cpu_nsec_dst.%d\n",
			__func__, ret);
		goto boot_exit;
	}
	mutex_unlock(&dev->pwrlock);

	return 0;

boot_exit:
	if ((nonsec_isp_boot_image_release()) != 0)
		pr_err("[%s] Failed : boot_image_release\n", __func__);
isp_exit:
	if ((hisp_pwr_core_nsec_exit()) != 0)
		pr_err("[%s] Failed : hisp_pwr_core_nsec_exit\n", __func__);
isp_down:
	if ((hisp_subsys_powerdn()) != 0)
		pr_err("[%s] Failed : hisp_subsys_powerdn\n", __func__);

	mutex_unlock(&dev->pwrlock);

	return ret;
}
EXPORT_SYMBOL(nonsec_isp_device_enable);

int nonsec_isp_device_disable(void)
{
	struct hisp_nsec *dev = &nsec_rproc_dev;
	int ret;
	mutex_lock(&dev->pwrlock);
	ret = nonsec_isp_boot_image_release();
	if (ret != 0)
		pr_err("[%s] boot_image_release failed, ret.%d\n",
			__func__, ret);

	ret = hisp_pwr_cpu_nsec_rst();
	if (ret != 0)
		pr_err("[%s] hisp_pwr_cpu_nsec_rst failed, ret.%d\n",
			__func__, ret);

	ret = hisp_pwr_core_nsec_exit();
	if (ret != 0)
		pr_err("[%s] hisp_pwr_core_nsec_exit failed, ret.%d\n",
			__func__, ret);

	ret = hisp_subsys_powerdn();
	if (ret != 0)
		pr_err("[%s] hisp_subsys_powerdn failed, ret.%d\n",
			__func__, ret);
	mutex_unlock(&dev->pwrlock);

	return 0;
}
EXPORT_SYMBOL(nonsec_isp_device_disable);

u64 hisp_get_pgd_base(void)
{
	struct hisp_nsec *dev = &nsec_rproc_dev;

	return dev->pgd_base;
}

int hisp_rproc_pgd_set(struct rproc *rproc)
{
	struct hisp_nsec *dev = &nsec_rproc_dev;
	struct rproc_shared_para *param = NULL;

	hisp_lock_sharedbuf();
	param = rproc_get_share_para();
	if (param == NULL) {
		pr_err("[%s] Failed : param.%pK\n", __func__, param);
		hisp_unlock_sharedbuf();
		return -EINVAL;
	}
	param->dynamic_pgtable_base = dev->pgd_base;
	hisp_unlock_sharedbuf();

	return 0;
}

static int isp_remap_rsc(struct hisp_nsec *dev)
{
	dev->isp_dma_va = dma_alloc_coherent(dev->device, ISP_MEM_SIZE,
					&dev->isp_dma, GFP_KERNEL);
	if (dev->isp_dma_va == NULL) {
		pr_err("[%s] isp_dma_va failed\n", __func__);
		return -ENOMEM;
	}
	pr_info("[%s] isp_dma_va.%pK\n", __func__, dev->isp_dma_va);

	return 0;
}

static void isp_unmap_rsc(struct hisp_nsec *dev)
{
	if (dev->isp_dma_va != NULL)
		dma_free_coherent(dev->device, ISP_MEM_SIZE,
				dev->isp_dma_va, dev->isp_dma);

	dev->isp_dma_va = NULL;
}

int hisp_nsec_probe(struct platform_device *pdev)
{
	struct hisp_nsec *dev = &nsec_rproc_dev;
	int ret;

	pr_alert("[%s] +\n", __func__);

	dev->device = &pdev->dev;
	dev->isp_pdev = pdev;

	ret = isp_remap_rsc(dev);
	if (ret != 0) {
		pr_err("[%s] failed, isp_remap_src.%d\n", __func__, ret);
		return ret;
	}

	ret = hisp_nsec_getdts(pdev, dev);
	if (ret != 0) {
		pr_err("[%s] Failed : hisp_nsec_getdts.%d.\n",
				__func__, ret);
		goto hisp_nsec_getdts_fail;
	}

	dev->pgd_base = hisi_domain_get_ttbr(dev->device);

	mutex_init(&dev->pwrlock);
	pr_alert("[%s] -\n", __func__);

	return 0;
hisp_nsec_getdts_fail:
	isp_unmap_rsc(dev);

	return ret;
}

int hisp_nsec_remove(struct platform_device *pdev)
{
	struct hisp_nsec *dev = &nsec_rproc_dev;

	mutex_destroy(&dev->pwrlock);
	isp_unmap_rsc(dev);
	return 0;
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HiStar V150 rproc driver");

