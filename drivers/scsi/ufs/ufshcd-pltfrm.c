/*
 * Universal Flash Storage Host controller Platform bus based glue driver
 *
 * This code is based on drivers/scsi/ufs/ufshcd-pltfrm.c
 * Copyright (C) 2011-2019 Samsung India Software Operations
 *
 * Authors:
 *	Santosh Yaraganavi <santosh.sy@samsung.com>
 *	Vinayak Holikatti <h.vinayak@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * See the COPYING file in the top-level directory or visit
 * <http://www.gnu.org/licenses/gpl-2.0.html>
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * This program is provided "AS IS" and "WITH ALL FAULTS" and
 * without warranty of any kind. You are solely responsible for
 * determining the appropriateness of using and distributing
 * the program and assume all risks associated with your exercise
 * of rights with respect to the program, including but not limited
 * to infringement of third party rights, the risks and costs of
 * program errors, damage to or loss of data, programs or equipment,
 * and unavailability or interruption of operations. Under no
 * circumstances will the contributor of this Program be liable for
 * any damages of any kind arising from your use or distribution of
 * this program.
 */

#define pr_fmt(fmt) "ufshcd :" fmt

#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/of.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/bootdevice.h>
#include <linux/hisi/rdr_hisi_platform.h>

#include "ufshcd.h"
#include "ufs-hpb.h"
#include "ufs-kirin.h"
#include "ufs-hisi.h"

static const struct of_device_id ufs_of_match[] = {
	{.compatible = "jedec,ufs-1.1"},
	{
		.compatible = "hisilicon,kirin-ufs",
		.data = &ufs_hba_hufs_vops,
	},
	{}, /*lint !e785*/
};

static struct ufs_hba_variant_ops *get_variant_ops(struct device *dev)
{
	if (dev->of_node) {
		const struct of_device_id *match;

		match = of_match_node(ufs_of_match, dev->of_node);
		if (match)
			return (struct ufs_hba_variant_ops *)match->data;
	}

	return NULL;
}

#ifdef CONFIG_PM
/*
 * ufshcd_pltfrm_suspend - suspend power management function
 * @dev: pointer to device handle
 *
 * Returns 0 if successful
 * Returns non-zero otherwise
 */
static int ufshcd_pltfrm_suspend(struct device *dev)
{
	int ret;
	struct ufs_hba *hba = (struct ufs_hba *)dev_get_drvdata(dev);
	struct Scsi_Host *host = hba->host;

	dev_info(dev, "%s:%d ++\n", __func__, __LINE__);
#ifdef CONFIG_MAS_BLK
	if (host->queue_quirk_flag &
		SHOST_QUIRK(SHOST_QUIRK_SCSI_QUIESCE_IN_LLD)) {

		blk_generic_freeze(
			hba->host->use_blk_mq
				? &(hba->host->tag_set.lld_func)
				: &(hba->host->bqt->lld_func),
			BLK_LLD, true);
#endif
		__set_quiesce_for_each_device(hba->host);
#ifdef CONFIG_MAS_BLK
	}
#endif
	ret = ufshcd_system_suspend(dev_get_drvdata(dev));
	if (ret) {
#ifdef CONFIG_MAS_BLK
		if (host->queue_quirk_flag &
			SHOST_QUIRK(SHOST_QUIRK_SCSI_QUIESCE_IN_LLD)) {
#endif
			__clr_quiesce_for_each_device(hba->host);
#ifdef CONFIG_MAS_BLK
			blk_generic_freeze(
				hba->host->use_blk_mq
					? &(hba->host->tag_set.lld_func)
					: &(hba->host->bqt->lld_func),
				BLK_LLD, false);
		}
#endif
	}
	dev_info(dev, "%s:%d ret:%d--\n", __func__, __LINE__, ret);
	return ret;
}

/*
 * ufshcd_pltfrm_resume - resume power management function
 * @dev: pointer to device handle
 *
 * Returns 0 if successful
 * Returns non-zero otherwise
 */
static int ufshcd_pltfrm_resume(struct device *dev)
{
	int ret;
	struct ufs_hba *hba = (struct ufs_hba *)dev_get_drvdata(dev);
	struct Scsi_Host *host = hba->host;

	dev_info(dev, "%s:%d ++\n", __func__, __LINE__);
	ret = ufshcd_system_resume(dev_get_drvdata(dev));
#ifdef CONFIG_MAS_BLK
	if (host->queue_quirk_flag &
		SHOST_QUIRK(SHOST_QUIRK_SCSI_QUIESCE_IN_LLD)) {
#endif
		__clr_quiesce_for_each_device(hba->host);
#ifdef CONFIG_MAS_BLK
		blk_generic_freeze(
			hba->host->use_blk_mq
				? &(hba->host->tag_set.lld_func)
				: &(hba->host->bqt->lld_func),
			BLK_LLD, false);
	}
#endif
	hba->in_shutdown = false;
	dev_info(dev, "%s:%d ret:%d--\n", __func__, __LINE__, ret);
	return ret;
}

static int ufshcd_pltfrm_restore(struct device *dev)
{
	struct ufs_hba *hba = (struct ufs_hba *)dev_get_drvdata(dev);
	dev_info(dev, "%s:%d ++\n", __func__, __LINE__);

	hba->wlun_dev_clr_ua = true;

	dev_info(dev, "%s:%d --\n", __func__, __LINE__);
	return 0;
}

static int ufshcd_pltfrm_runtime_suspend(struct device *dev)
{
	return ufshcd_runtime_suspend((struct ufs_hba *)dev_get_drvdata(dev));
}

static int ufshcd_pltfrm_runtime_resume(struct device *dev)
{
	return ufshcd_runtime_resume((struct ufs_hba *)dev_get_drvdata(dev));
}

static int ufshcd_pltfrm_runtime_idle(struct device *dev)
{
	return ufshcd_runtime_idle((struct ufs_hba *)dev_get_drvdata(dev));
}
#else /* !CONFIG_PM */
#define ufshcd_pltfrm_suspend	NULL
#define ufshcd_pltfrm_resume	NULL
#define ufshcd_pltfrm_restore	NULL
#define ufshcd_pltfrm_runtime_suspend	NULL
#define ufshcd_pltfrm_runtime_resume	NULL
#define ufshcd_pltfrm_runtime_idle	NULL
#endif /* CONFIG_PM */

#define SHUTDOWN_TIMEOUT (32 * 1000)
static void ufshcd_pltfrm_shutdown(struct platform_device *pdev)
{
	struct ufs_hba *hba = (struct ufs_hba *)platform_get_drvdata(pdev);
	struct Scsi_Host *host = hba->host;
	unsigned long timeout = SHUTDOWN_TIMEOUT;

	dev_err(&pdev->dev, "%s ++ lrb_in_use 0x%llx\n", __func__, hba->lrb_in_use);
	hba->in_shutdown = true;
	suspend_wait_hpb(hba);

#ifdef CONFIG_MAS_BLK
	if (host->queue_quirk_flag &
		SHOST_QUIRK(SHOST_QUIRK_SCSI_QUIESCE_IN_LLD)) {

		blk_generic_freeze(
			hba->host->use_blk_mq
				? &(hba->host->tag_set.lld_func)
				: &(hba->host->bqt->lld_func),
			BLK_LLD, true);
#endif
		/*
		 * set all scsi device state to quiet to
		 * forbid io form blk level
		 */
		__set_quiesce_for_each_device(hba->host);
#ifdef CONFIG_MAS_BLK
	}
#endif

	while (hba->lrb_in_use) {
		if (timeout == 0)
			dev_err(&pdev->dev,
				"%s: wait cmdq complete reqs timeout!\n",
				__func__);
		timeout--;
		mdelay(1);
	}

	ufshcd_shutdown(hba);

	dev_err(&pdev->dev, "%s --\n", __func__);
}

static int hufs_ufshcd_use_hc_value(struct device *dev)
{
	int use_hufs_hc = 0;
	struct device_node *np = dev->of_node;

	if (of_property_read_u32(np, "ufs-kirin-use-hisi-hc", &use_hufs_hc))
		dev_info(dev, "find ufs-kirin-use-hisi-hc value, is %d\n",
		    use_hufs_hc);

	return use_hufs_hc;
}

static int hufs_ufshcd_unipro_base(struct platform_device *pdev,
				   void __iomem **ufs_unipro_base,
				   struct resource **mem_res)
{
	int use_hufs_hc;
	struct device *dev = &pdev->dev;
	int err;

	use_hufs_hc = hufs_ufshcd_use_hc_value(dev);
	if (use_hufs_hc) {
		dev_info(dev, "use hufs host controller\n");
		*mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
		*ufs_unipro_base = devm_ioremap_resource(dev, *mem_res);
		if (IS_ERR(*ufs_unipro_base)) {
			err = PTR_ERR(*ufs_unipro_base);
			return err;
		}
	} else {
		*ufs_unipro_base = 0;
	}

	return 0;
}

#ifdef CONFIG_HISI_UFS_HC_CORE_UTR
static int hufs_ufshcd_get_core_irqs(struct platform_device *pdev, struct ufs_hba *hba)
{
	unsigned int i;
	unsigned int core0_interrupt_index;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

	if (of_property_read_u32(np, "core0-interrupt-index",
				 &core0_interrupt_index)) {
		dev_err(dev, "ufshc core interrupt is not available!\n");
		return -ENODEV;
	}
	for (i = 0; i < UTR_CORE_NUM; i++) {
		hba->core_irq[i] =
			platform_get_irq(pdev, core0_interrupt_index + i);
		if (hba->core_irq[i] < 0) {
			dev_err(dev, "core %d irq resource not available!\n", i);
			return -ENODEV;
		}
#ifdef CONFIG_HISI_DEBUG_FS
		if ((hba->core_irq[i] - hba->core_irq[0]) != i)
			rdr_syserr_process_for_ap(
				(u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
#endif
		snprintf(hba->core_irq_name[i], CORE_IRQ_NAME_LEN, "ufshcd_hufs_core%d_irq", i);
	}

	return 0;
}
#endif

static int hufs_ufshcd_pltfrm_irq(struct platform_device *pdev,
				  struct ufs_hba *hba,
				  void __iomem *ufs_unipro_base)
{
	struct device *dev = &pdev->dev;
	int err = -ENODEV;

	hba->use_hufs_hc = hufs_ufshcd_use_hc_value(dev);
	if (!hba->use_hufs_hc)
		return 0;

#ifdef CONFIG_SCSI_UFS_INTR_HUB
	hba->ufs_intr_hub_irq = platform_get_irq(pdev, 0);
	if (hba->ufs_intr_hub_irq < 0) {
		dev_err(dev, "IRQ resource not available\n");
		return err;
	}
#else
	hba->unipro_irq = platform_get_irq(pdev, 1);
	if (hba->unipro_irq < 0) {
		dev_err(dev, "IRQ resource not available\n");
		return err;
	}

	hba->fatal_err_irq = platform_get_irq(pdev, 2);
	if (hba->fatal_err_irq < 0) {
		dev_err(dev, "IRQ resource not available\n");
		return err;
	}
#endif

#ifdef CONFIG_HISI_UFS_HC_CORE_UTR
	if (hufs_ufshcd_get_core_irqs(pdev, hba))
		return err;
#endif
	hba->ufs_unipro_base = ufs_unipro_base;

	return 0;
}

/*
 * ufshcd_pltfrm_probe - probe routine of the driver
 * @pdev: pointer to Platform device handle
 *
 * Returns 0 on success, non-zero value on failure
 */
int ufshcd_pltfrm_probe(struct platform_device *pdev)
{
	struct ufs_hba *hba = NULL;
	void __iomem *mmio_base = NULL;
	struct resource *mem_res = NULL;
	int irq;
	int timer_irq = -1;
	int err;
	void __iomem *ufs_unipro_base = NULL;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	unsigned int timer_intr_index;

	if (get_bootdevice_type() != BOOT_DEVICE_UFS) {
		dev_err(dev, "system is't booted from UFS on ARIES FPGA board\n");
		return -ENODEV;
	}

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	mmio_base = devm_ioremap_resource(dev, mem_res);
	if (IS_ERR(mmio_base)) {
		err = PTR_ERR(mmio_base);
		goto out;
	}

	err = hufs_ufshcd_unipro_base(pdev, &ufs_unipro_base, &mem_res);
	if (err)
		goto out;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(dev, "IRQ resource not available\n");
		err = -ENODEV;
		goto out;
	}

	err = ufshcd_alloc_host(dev, &hba);
	if (err) {
		dev_err(&pdev->dev, "Allocation failed\n");
		goto out;
	}

	hba->vops = get_variant_ops(&pdev->dev);

	if (!of_property_read_u32(
		    np, "timer-interrupt-index", &timer_intr_index)) {
		timer_irq = platform_get_irq(pdev, timer_intr_index);
		if (timer_irq < 0)
			dev_err(dev, "UFS timer interrupt is not available!\n");
	}

	pm_runtime_set_active(&pdev->dev);
	pm_runtime_irq_safe(&pdev->dev);
	pm_suspend_ignore_children(&pdev->dev, true);
	/* pm runtime delay 5 us time */
	pm_runtime_set_autosuspend_delay(&pdev->dev, 5);
	pm_runtime_use_autosuspend(&pdev->dev);
	if (of_find_property(np, "ufs-kirin-disable-pm-runtime", NULL))
		hba->caps |= DISABLE_UFS_PMRUNTIME;

	/* auto hibern8 can not exist with pm runtime */
	if ((hba->caps & DISABLE_UFS_PMRUNTIME) ||
		of_find_property(np, "ufs-kirin-use-auto-H8", NULL))
		pm_runtime_forbid(hba->dev);

	parse_hpb_dts(hba, np);

	err = hufs_ufshcd_pltfrm_irq(pdev, hba, ufs_unipro_base);
	if (err)
		goto out;

	err = ufshcd_init(hba, mmio_base, irq, timer_irq);
	if (err) {
		dev_err(dev, "Initialization failed\n");
		goto out_disable_rpm;
	}

#ifndef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V2
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
	/*
	 * to improve writing key efficiency,
	 * remap key regs with writecombine
	 */
	err = ufshcd_keyregs_remap_wc(hba, mem_res->start);
	if (err) {
		dev_err(dev, "ufshcd_keyregs_remap_wc err\n");
		goto out_disable_rpm;
	}

#endif
#endif
	platform_set_drvdata(pdev, hba);

	return 0;

out_disable_rpm:
	pm_runtime_disable(&pdev->dev);
	pm_runtime_set_suspended(&pdev->dev);
out:
	return err;
}

/*
 * ufshcd_pltfrm_remove - remove platform driver routine
 * @pdev: pointer to platform device handle
 *
 * Returns 0 on success, non-zero value on failure
 */
static int ufshcd_pltfrm_remove(struct platform_device *pdev)
{
	struct ufs_hba *hba =  (struct ufs_hba *)platform_get_drvdata(pdev);

	pm_runtime_get_sync(&(pdev)->dev);
	ufshcd_remove(hba);
	return 0;
}

static const struct dev_pm_ops ufshcd_dev_pm_ops = {
	.suspend	= ufshcd_pltfrm_suspend,
	.resume		= ufshcd_pltfrm_resume,
	.restore	= ufshcd_pltfrm_restore,
	.runtime_suspend = ufshcd_pltfrm_runtime_suspend,
	.runtime_resume  = ufshcd_pltfrm_runtime_resume,
	.runtime_idle    = ufshcd_pltfrm_runtime_idle,
};

static struct platform_driver ufshcd_pltfrm_driver = {
	.probe	= ufshcd_pltfrm_probe,
	.remove	= ufshcd_pltfrm_remove,
	.shutdown = ufshcd_pltfrm_shutdown,
	.driver	= {
		.name	= "ufshcd",
		.pm	= &ufshcd_dev_pm_ops,
		.of_match_table = ufs_of_match,
	},
};

module_platform_driver(ufshcd_pltfrm_driver);

MODULE_AUTHOR("Santosh Yaragnavi <santosh.sy@samsung.com>");
MODULE_AUTHOR("Vinayak Holikatti <h.vinayak@samsung.com>");
MODULE_DESCRIPTION("UFS host controller Platform bus based glue driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(UFSHCD_DRIVER_VERSION);
