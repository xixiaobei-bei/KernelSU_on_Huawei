/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description:  this file implements api for node hisi-ivp to register, device
 * init, remove, etc
 * Author:  chenweiyu
 * Create:  2019-07-30
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/dma-buf.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/pm_wakeup.h>
#include <linux/ion.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/hisi-iommu.h>
#include <linux/syscalls.h>
#include <linux/clk-provider.h>
#include <linux/bitops.h>
#include <linux/firmware.h>
#include "securec.h"
#include "ivp.h"
#include "ivp_log.h"
#include "ivp_sec.h"
#include "ivp_manager.h"
#include "ivp_ioctl.h"
#include "ivp_smmu.h"
#include "ivp_reg.h"
//lint -save -e750 -e753 -specific(-e753) -e750 -e528
//-specific(-e528) -e144 -e82 -e64 -e785 -e715 -e712 -e40
//lint -save -e63 -e732 -e42 -e550 -e438 -e834 -e648 -e747 -e778 -e50
//lint -save -e749 -e84 -e866 -e514 -e846 -e737 -e574


static void ivp_mutex_init(struct ivp_device *pdev)
{
	mutex_init(&pdev->ivp_wake_lock_mutex);
	mutex_init(&pdev->ivp_load_image_mutex);
	mutex_init(&pdev->ivp_power_up_off_mutex);
	mutex_init(&pdev->ivp_open_release_mutex);
}

static void ivp_mutex_deinit(struct ivp_device *pdev)
{
	mutex_destroy(&pdev->ivp_open_release_mutex);
	mutex_destroy(&pdev->ivp_power_up_off_mutex);
	mutex_destroy(&pdev->ivp_load_image_mutex);
	mutex_destroy(&pdev->ivp_wake_lock_mutex);
}

static int ivp_open(struct inode *inode, struct file *fd)
{
	struct ivp_device *pdev = NULL;
	struct miscdevice *pmiscdev =  (struct miscdevice *)fd->private_data;

	if (!pmiscdev) {
		ivp_err("invalid param pmiscdev");
		return -ENODEV;
	}

	pdev = container_of(pmiscdev, struct ivp_device, device);

	if (!pdev) {
		ivp_err("invalid param pdev");
		return -EINVAL;
	}
	mutex_lock(&pdev->ivp_open_release_mutex);

	if (atomic_read(&pdev->accessible) == 0) {
		mutex_unlock(&pdev->ivp_open_release_mutex);
		ivp_err("maybe ivp dev has been opened!");
		return -EBUSY;
	}

	atomic_dec(&pdev->accessible);

	fd->private_data = (void *)pdev;
	mutex_unlock(&pdev->ivp_open_release_mutex);
	ivp_info("open node %s success", pdev->device.name);
	return 0;
}


static void ivp_poweroff(struct ivp_device *pdev)
{
	if (atomic_read(&pdev->poweron_success) != 0) {
		ivp_err("maybe ivp dev not poweron success!");
		return;
	}

	ivp_deinit_resethandler(pdev);

	ivp_hw_runstall(pdev, IVP_RUNSTALL_STALL);
	if (ivp_hw_query_runstall(pdev) != IVP_RUNSTALL_STALL)
		ivp_err("Failed to stall ivp.");
	ivp_hw_clr_wdg_irq(pdev);

	disable_irq(pdev->wdg_irq);
	free_irq(pdev->wdg_irq, pdev);

	if (pdev->ivp_secmode == NOSEC_MODE) {
		disable_irq(pdev->dwaxi_dlock_irq);
		free_irq(pdev->dwaxi_dlock_irq, pdev);
		ivp_dev_smmu_deinit(pdev);
	}

	ivp_dev_poweroff(pdev);

	pdev->ivp_secmode = NOSEC_MODE;
	atomic_inc(&pdev->poweron_access);
	atomic_inc(&pdev->poweron_success);
}

static int ivp_release(struct inode *inode, struct file *fd)
{
	struct ivp_device *pdev = fd->private_data;
	int ret = 0;

	if (!pdev) {
		ivp_err("invalid param pdev");
		return -ENODEV;
	}

	mutex_lock(&pdev->ivp_open_release_mutex);
	if (atomic_read(&pdev->accessible) != 0) {
		mutex_unlock(&pdev->ivp_open_release_mutex);
		ivp_err("maybe ivp dev not opened!");
		return -1;
	}

	mutex_lock(&pdev->ivp_power_up_off_mutex);
	ivp_poweroff(pdev);
	mutex_unlock(&pdev->ivp_power_up_off_mutex);

	ivp_info("ivp device closed.");

	atomic_inc(&pdev->accessible);
	mutex_unlock(&pdev->ivp_open_release_mutex);
	return ret;
}


#ifdef CONFIG_COMPAT
static long ivp_ioctl32(struct file *fd, unsigned int cmd, unsigned long arg)
{
	long ret;
	void *ptr_user = compat_ptr(arg);

	ret = ivp_ioctl(fd, cmd, (uintptr_t)ptr_user);
	return ret;
}
#endif

static int ivp_mmap(struct file *fd, struct vm_area_struct *vma)
{
	int i;
	int ret = 0;
	unsigned long size = 0;
	unsigned long mm_pgoff = (vma->vm_pgoff << IVP_MMAP_SHIFT);
	unsigned long phy_addr = vma->vm_pgoff << (PAGE_SHIFT + IVP_MMAP_SHIFT);
	struct ivp_sect_info *psects = NULL;
	struct ivp_device *pdev = fd->private_data;

	if (!pdev) {
		ivp_err("invalid param pdev");
		return -EINVAL;
	}

	if (atomic_read(&pdev->poweron_success) != 0) {
		ivp_err("Invalid mmap, not powerup!");
		return -EINVAL;
	}
	size = vma->vm_end - vma->vm_start;
	if (size > LISTENTRY_SIZE) {
		ivp_err("Invalid size = 0x%08x.", size);
		return -EINVAL;
	}

	if (pdev->ivp_secmode == SECURE_MODE)
		psects = pdev->sec_sects;
	else
		psects = pdev->sects;
	for (i = 0; i < pdev->sect_count; i++) {
		if (phy_addr >= (psects[i].acpu_addr << IVP_MMAP_SHIFT) &&
			(phy_addr <= ((psects[i].acpu_addr << IVP_MMAP_SHIFT) + psects[i].len)) &&
		    (phy_addr + size) <= ((psects[i].acpu_addr << IVP_MMAP_SHIFT) + psects[i].len)) {
			ivp_info("Valid section %d for target.", i);
			break;
		}
	}

	if (i == pdev->sect_count) {
		ivp_err("Invalid mapping address or size.");
		return -EINVAL;
	}

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	ret = remap_pfn_range(vma, vma->vm_start,
			mm_pgoff, size, vma->vm_page_prot);
	if (ret < 0) {
		ivp_err("Failed to map address space. Error code is %d.", ret);
		return ret;
	}
	ivp_dbg("nocached success, ret:%#x", ret);

	return 0;
}

static const struct file_operations ivp_fops = {
	.owner = THIS_MODULE,
	.open = ivp_open,
	.release = ivp_release,
	.unlocked_ioctl = ivp_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = ivp_ioctl32,
#endif
	.mmap = ivp_mmap,
};

static int ivp_remove(struct platform_device *plat_devp)
{
	struct ivp_device *pdev =
		(struct ivp_device *)platform_get_drvdata(plat_devp);

	if (!pdev || !pdev->sects) {
		ivp_err("invalid param pdev");
		return -ENODEV;
	}

	misc_deregister(&pdev->device);

	ivp_deinit_pri(pdev);

	kfree(pdev->sects);
	kfree(pdev->sec_sects);
	pdev->sects = NULL;
	pdev->sec_sects = NULL;
	pdev->sect_count = 0;
	ivp_release_iores(plat_devp);

	pdev->smmu_dev = NULL;
	wakeup_source_trash(&pdev->ivp_power_wakelock);
	ivp_mutex_deinit(pdev);
	return 0;
}

static int ivp_device_init(struct platform_device *plat_dev,
		struct ivp_device *pdev)
{
	int ret;

	atomic_set(&pdev->accessible, 1);
	atomic_set(&pdev->poweron_access, 1);
	atomic_set(&pdev->poweron_success, 1);
	ivp_mutex_init(pdev);

	/*lint -save -e838*/
	ret = misc_register(&pdev->device);
	if (ret)
		goto err_out_misc;

	ret = ivp_setup_smmu_dev(pdev);
	if (ret)
		goto err_out_smmu;

	ret = ivp_setup_onchipmem_sections(plat_dev, pdev);
	if (ret)
		goto err_out_onchipmem;

	ret = ivp_setup_irq(plat_dev, pdev);
	if (ret)
		goto err_out_irq;

	ret = ivp_init_reg_res(plat_dev, pdev);
	if (ret)
		goto err_out_reg_res;

	ret = ivp_init_pri(plat_dev, pdev);
	if (ret)
		goto err_out_init_pri;


	ivp_info("device:%s init sucess", pdev->device.name);
	return ret;
	/*lint -restore*/

err_out_init_pri:
	ivp_release_iores(plat_dev);
err_out_reg_res:
err_out_irq:
	kfree(pdev->sects);
	kfree(pdev->sec_sects);
	pdev->sects = NULL;
	pdev->sec_sects = NULL;
	pdev->sect_count = 0;
err_out_onchipmem:
	pdev->smmu_dev = NULL;
err_out_smmu:
	misc_deregister(&pdev->device);
err_out_misc:
	ivp_mutex_deinit(pdev);
	wakeup_source_trash(&pdev->ivp_power_wakelock);

	ivp_err("device:%s init error", pdev->device.name);
	return ret;
}

static int ivp_probe(struct platform_device *plat_dev)
{
	struct ivp_device *pdev = NULL;
	int ret = 0;

	pdev = kzalloc(sizeof(struct ivp_device), GFP_KERNEL);
	if (!pdev) {
		ivp_err("ivp device kzalloc failed!");
		return -ENOMEM;
	}

	pdev->ivp_pdev = plat_dev;
	pdev->device.minor = MISC_DYNAMIC_MINOR,
	pdev->device.name = IVP_MODULE_NAME,
	pdev->device.fops = &ivp_fops,

	platform_set_drvdata(plat_dev, pdev);
	wakeup_source_init(&pdev->ivp_power_wakelock, "ivp_power_wakelock");

	ret = ivp_device_init(plat_dev, pdev);
	if (ret) {
		ivp_err("ivp_device_init failed %d", ret);
		return ret;
	}

	ivp_err("ivp_device %s probe sucess", pdev->device.name);
	return ret;
}

/*lint -save -e785 -e64*/
#ifdef CONFIG_OF
static const struct of_device_id ivp_of_id[] = {
	{ .compatible = "hisilicon,hisi-ivp", },
	{}
};
#endif

static struct platform_driver ivp_platform_driver = {
	.probe = ivp_probe,
	.remove = ivp_remove,
	.driver = {
		.name = IVP_MODULE_NAME,
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(ivp_of_id),
#endif
	},
};

module_platform_driver(ivp_platform_driver);
/*lint -restore*/


MODULE_DESCRIPTION("hisilicon_ivp driver");
MODULE_LICENSE("GPL");
//lint -restore
