/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2025. All rights reserved.
 * Description: jpeg jpu
 * Author: Huawei Hisilicon
 * Create: 2013
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include "hisi_jpu.h"
#include <linux/device.h>

uint32_t hisi_jpu_msg_level = 7;

/*lint -save -e21 -e64 -e514 -e84 -e528 -e708 -e753 -e778 -e846 -e866*/
module_param_named(debug_msg_level, hisi_jpu_msg_level, int, 0644);
MODULE_PARM_DESC(debug_msg_level, "hisi jpu msg level");

int g_debug_jpu_dec;
#ifdef CONFIG_FB_DEBUG_USED
module_param_named(debug_jpu_dec, g_debug_jpu_dec, int, 0644);
MODULE_PARM_DESC(debug_jpu_dec, "hisi jpu decode debug");
#endif

int g_debug_jpu_dec_job_timediff;
#ifdef CONFIG_FB_DEBUG_USED
module_param_named(debug_jpu_decode_job_timediff, g_debug_jpu_dec_job_timediff,
	int, 0644);
MODULE_PARM_DESC(debug_jpu_decode_job_timediff,
	"hisi jpu decode job timediff debug");
#endif

/* lint -restore */
static struct hisi_jpu_data_type *g_hisijd;

/*lint -save -e715*/
static int hisi_jpu_open(struct inode *inode, struct file *filp)
{
	struct hisi_jpu_data_type *hisijd = NULL;
	int ret = 0;

	HISI_JPU_INFO("+\n");

	if (filp == NULL) {
		HISI_JPU_ERR("filp is NULL!\n");
		return -EINVAL;
	}

	if (filp->private_data == NULL)
		filp->private_data = g_hisijd;

	hisijd = filp->private_data;
	if (hisijd == NULL) {
		HISI_JPU_ERR("hisijd is NULL!");
		return -EINVAL;
	}

	hisijd->ref_cnt++;

	HISI_JPU_INFO("-\n");
	return ret;
}

static int hisi_jpu_check_flip(struct file *filp)
{
	if (filp == NULL) {
		HISI_JPU_ERR("filp is NULL!\n");
		return -EINVAL;
	}

	if (filp->private_data == NULL) {
		HISI_JPU_ERR("hisijd is NULL!");
		return -EINVAL;
	}
	return 0;
}

int hisi_jpu_release(struct inode *inode, struct file *filp)
{
	struct hisi_jpu_data_type *hisijd = NULL;
	int ret;

	HISI_JPU_INFO("+\n");

	ret = hisi_jpu_check_flip(filp);
	if (ret != 0)
		return -EINVAL;

	hisijd = filp->private_data;
	if (hisijd->ref_cnt <= 0) {
		HISI_JPU_INFO("try to close unopened jpu! exit --\n");
		return -EINVAL;
	}

	hisijd->ref_cnt--;

	HISI_JPU_INFO("-\n");
	return ret;
}

/* lint -restore */

#ifdef CONFIG_COMPAT
static long hisi_jpu_ioctl(struct file *filp, u_int cmd, u_long arg)
{
	int ret;
	struct hisi_jpu_data_type *hisijd = NULL;
	void __user *argp = (void __user *)(uintptr_t) arg;

	HISI_JPU_DEBUG("+\n");

	ret = hisi_jpu_check_flip(filp);
	if (ret != 0)
		return -EINVAL;

	hisijd = filp->private_data;

	if (cmd == HISIJPU_JOB_EXEC)
		ret = hisijpu_job_exec(hisijd, argp);
	/*lint -restore*/
	if (ret != 0)
		HISI_JPU_ERR("unsupported ioctl %x\n", cmd);


	HISI_JPU_DEBUG("-\n");
	return ret;
}
#endif

/* lint -save -e785 */
static const struct file_operations hisi_jpu_fops = {
	.owner = THIS_MODULE,
	.open = hisi_jpu_open,
	.release = hisi_jpu_release,
	.unlocked_ioctl = hisi_jpu_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = hisi_jpu_ioctl,
#endif
	.mmap = NULL,
};

/*lint -restore*/

static int hisi_jpu_set_platform(struct hisi_jpu_data_type *hisijd,
				struct device_node *np)
{
	int ret;

	if (hisijd == NULL) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	ret = of_property_read_string_index(np, "platform-names", 0,
			&(hisijd->jpg_platform_name));
	if ((ret != 0) || (hisijd->jpg_platform_name == NULL)) {
		HISI_JPU_ERR("failed to get jpg resource! ret=%d\n", ret);
		ret = -EINVAL;
		return ret;
	}

	/* 9 is number of string for compare */
	if (strncmp(hisijd->jpg_platform_name, "kirin_970", 9) == 0) {
		hisijd->jpu_support_platform = HISI_KIRIN_970;
		HISI_JPU_INFO("hisijd jpg platform is %d\n",
			hisijd->jpu_support_platform);
		return ret;
	} else if (strncmp(hisijd->jpg_platform_name, "dss_v500", 8) == 0) {
		hisijd->jpu_support_platform = HISI_DSS_V500;
		HISI_JPU_INFO("hisijd jpg platform is %d\n",
			hisijd->jpu_support_platform);
		return ret;
	} else if (strncmp(hisijd->jpg_platform_name, "dss_v501", 8) == 0) {
		hisijd->jpu_support_platform = HISI_DSS_V501;
		HISI_JPU_INFO("hisijd jpg platform is %d\n",
			hisijd->jpu_support_platform);
		return ret;
	} else if (strncmp(hisijd->jpg_platform_name, "dss_v510", 8) == 0) {
		hisijd->jpu_support_platform = HISI_DSS_V510;
		HISI_JPU_INFO("hisijd jpg platform is %d\n",
			hisijd->jpu_support_platform);
		return ret;
	} else if (strncmp(hisijd->jpg_platform_name, "dss_v511", 8) == 0) {
		hisijd->jpu_support_platform = HISI_DSS_V510_CS;
		HISI_JPU_INFO("hisijd jpg platform is %d\n",
			hisijd->jpu_support_platform);
		return ret;
	}

	hisijd->jpu_support_platform = UNSUPPORT_PLATFORM;

	HISI_JPU_ERR("hisijd jpg platform is not support!\n");
	return -EINVAL;
}

static int hisi_jpu_chrdev_setup(struct hisi_jpu_data_type *hisijd)
{
	int ret;
	unsigned int minor = 0;

	if (hisijd == NULL) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	/* get the major number of the character device */
	ret = register_chrdev(hisijd->jpu_major, DEV_NAME_JPU, &hisi_jpu_fops);
	if (ret < 0) {
		HISI_JPU_ERR("fail to register driver!\n");
		return -ENXIO;
	}

	hisijd->jpu_major = (uint32_t)ret;
	hisijd->jpu_class = class_create(THIS_MODULE, DEV_NAME_JPU);
	if (hisijd->jpu_class == NULL) {
		HISI_JPU_ERR("fail to create jpu class!\n");
		ret = -ENOMEM;
		goto err_class_create;
	}

	/*lint -save -e845 */
	hisijd->jpu_dev = device_create(hisijd->jpu_class, 0,
		MKDEV(hisijd->jpu_major, minor), NULL, DEV_NAME_JPU);
	if (hisijd->jpu_dev == NULL) {
		HISI_JPU_ERR("fail to create jpu device!\n");
		ret = -ENOMEM;
		goto err_device_create;
	}

	return 0;

err_device_create:
	if (hisijd->jpu_class != NULL) {
		class_destroy(hisijd->jpu_class);
		hisijd->jpu_dev = NULL;
	}

err_class_create:
	if (hisijd->jpu_major > 0) {
		unregister_chrdev(hisijd->jpu_major, DEV_NAME_JPU);
		hisijd->jpu_major = 0;
	}

	return ret;
}

static int hisi_jpu_chrdev_remove(struct hisi_jpu_data_type *hisijd)
{
	if (hisijd == NULL) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (hisijd->jpu_class != NULL) {
		if (hisijd->jpu_dev != NULL) {
			device_destroy(hisijd->jpu_class, MKDEV(hisijd->jpu_major, 0));
			hisijd->jpu_dev = NULL;
		}
		class_destroy(hisijd->jpu_class);
		hisijd->jpu_class = NULL;
	}

	if (hisijd->jpu_major > 0) {
		unregister_chrdev(hisijd->jpu_major, DEV_NAME_JPU);
		hisijd->jpu_major = 0;
	}

	return 0;
}

static int hisi_jpu_get_reg_base(struct hisi_jpu_data_type *hisijd,
				struct device_node *np)
{
	bool cond = ((np == NULL) || (hisijd == NULL));

	if (cond) {
		HISI_JPU_ERR("np or hisijd is NULL!\n");
		return -ENXIO;
	}

	/* get jpu decoder reg base */
	hisijd->jpu_dec_base = of_iomap(np, 0);
	if (hisijd->jpu_dec_base == NULL) {
		HISI_JPU_ERR("failed to get jpu_dec_base resource\n");
		return -ENXIO;
	}

	/* get top reg base */
	hisijd->jpu_top_base = of_iomap(np, 1);
	if (hisijd->jpu_top_base == NULL) {
		HISI_JPU_ERR("failed to get jpu_top_base resource\n");
		return -ENXIO;
	}

	/* get cvdr reg base */
	hisijd->jpu_cvdr_base = of_iomap(np, 2);
	if (hisijd->jpu_cvdr_base == NULL) {
		HISI_JPU_ERR("failed to get jpu_cvdr_base resource\n");
		return -ENXIO;
	}

	/* get smmu reg base */
	hisijd->jpu_smmu_base = of_iomap(np, 3);
	if (hisijd->jpu_smmu_base == NULL) {
		HISI_JPU_ERR("failed to get jpu_smmu_base resource\n");
		return -ENXIO;
	}

	hisijd->media1_crg_base = of_iomap(np, 4);
	if (hisijd->media1_crg_base == NULL) {
		HISI_JPU_ERR("failed to get media1_crg_base resource\n");
		return -ENXIO;
	}

	hisijd->peri_crg_base = of_iomap(np, 5);
	if (hisijd->peri_crg_base == NULL) {
		HISI_JPU_ERR("failed to get peri_crg_base resource\n");
		return -ENXIO;
	}

	hisijd->pmctrl_base = of_iomap(np, 6);
	if (hisijd->pmctrl_base == NULL) {
		HISI_JPU_ERR("failed to get pmctrl_base resource\n");
		return -ENXIO;
	}

	hisijd->sctrl_base = of_iomap(np, 7);
	if (hisijd->sctrl_base == NULL) {
		HISI_JPU_ERR("failed to get sctrl_base resource\n");
		return -ENXIO;
	}

	return 0;
}

static int hisi_jpu_get_irqs(struct hisi_jpu_data_type *hisijd,
			 struct device_node *np)
{
	bool cond = ((np == NULL) || (hisijd == NULL));

	if (cond) {
		HISI_JPU_ERR("np or hisijd NULL!\n");
		return -ENXIO;
	}

	if (hisijd->is_irq_merge) {
		hisijd->jpu_merged_irq = irq_of_parse_and_map(np, 0);
		if (!hisijd->jpu_merged_irq) {
			HISI_JPU_ERR("failed to get jpu resource\n");
			return -ENXIO;
		}
		HISI_JPU_INFO("jpu_merged_irq:%d\n", hisijd->jpu_merged_irq);
	} else {
		/* get jpu err irq no */
		hisijd->jpu_err_irq = irq_of_parse_and_map(np, 0);
		if (hisijd->jpu_err_irq == 0) {
			HISI_JPU_ERR("failed to get jpu_err_irq resource\n");
			return -ENXIO;
		}

		/* get jpu done irq no */
		hisijd->jpu_done_irq = irq_of_parse_and_map(np, 1);
		if (hisijd->jpu_done_irq == 0) {
			HISI_JPU_ERR("failed to get jpu_done_irq resource\n");
			return -ENXIO;
		}

		/* get jpu other irq no */
		hisijd->jpu_other_irq = irq_of_parse_and_map(np, 2);
		if (hisijd->jpu_other_irq == 0) {
			HISI_JPU_ERR("failed to get jpu_other_irq resource\n");
			return -ENXIO;
		}
		HISI_JPU_INFO("jpu_err_irq:%d, jpu_done:%d, jpu_othe:%d\n",
			hisijd->jpu_err_irq, hisijd->jpu_done_irq,
			hisijd->jpu_other_irq);
	}

	return 0;
}

static int hisi_jpu_get_regulators(struct hisi_jpu_data_type *hisijd)
{
	if (hisijd == NULL) {
		HISI_JPU_ERR("np or hisijd is NULL!\n");
		return -ENXIO;
	}

	/* get jpu regulator */
	hisijd->jpu_regulator = devm_regulator_get(&(hisijd->pdev->dev),
						"jpu-regulator");
	if (IS_ERR(hisijd->jpu_regulator)) {
		HISI_JPU_ERR("failed to get jpu_regulator\n");
		return -ENXIO;
	}

	/* get media1 regulator */
	hisijd->media1_regulator = devm_regulator_get(&(hisijd->pdev->dev),
						"media1-regulator");
	if (IS_ERR(hisijd->media1_regulator)) {
		HISI_JPU_ERR("failed to get media1_regulator\n");
		return -ENXIO;
	}

	return 0;
}

static int hisi_jpu_put_regulators(struct hisi_jpu_data_type *hisijd)
{
	if (hisijd == NULL) {
		HISI_JPU_ERR("np or hisijd is NULL!\n");
		return -ENXIO;
	}

	if (hisijd->jpu_regulator != NULL)
		devm_regulator_put(hisijd->jpu_regulator);

	if (hisijd->media1_regulator != NULL)
		devm_regulator_put(hisijd->media1_regulator);

	return 0;
}

/*lint -save -e438 -e774*/
static int hisi_jpu_remove(struct platform_device *pdev)
{
	struct hisi_jpu_data_type *hisijd = NULL;
	int ret;

	HISI_JPU_INFO("+\n");

	if (pdev == NULL) {
		HISI_JPU_ERR("pdev is NULL!\n");
		return -EINVAL;
	}

	hisijd = platform_get_drvdata(pdev);
	if (hisijd == NULL) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	ret = hisi_jpu_unregister(hisijd);
	if (ret != 0) {
		HISI_JPU_ERR("hisi_jpu_unregister failed!\n");
		return -EINVAL;
	}

	ret = hisi_jpu_chrdev_remove(hisijd);
	if (ret != 0) {
		HISI_JPU_ERR("hisi_jpu_chrdev_remove failed!\n");
		return -EINVAL;
	}

	/* clk handle */
	if ((hisijd->jpg_func_clk != NULL) && (hisijd->pdev != NULL))
		devm_clk_put(&(hisijd->pdev->dev), hisijd->jpg_func_clk);

	ret = hisi_jpu_put_regulators(hisijd);
	if (ret != 0) {
		HISI_JPU_ERR("hisi_jpu_put_regulators failed!\n");
		return -EINVAL;
	}

	if (hisijd != NULL) {
		kfree(hisijd);
		hisijd = NULL;
		g_hisijd = NULL;
		platform_set_drvdata(pdev, hisijd);
	}
	HISI_JPU_INFO("-\n");
	return 0;
}

static int hisi_jpu_get_dts_resource(struct device_node *np,
		struct hisi_jpu_data_type *hisijd, struct device *dev)
{
	int ret;

	ret = of_property_read_u32(np, "fpga_flag", &(hisijd->fpga_flag));
	if (ret != 0) {
		dev_err(dev, "failed to get fpga_flag resource\n");
		return -ENXIO;
	}
	HISI_JPU_INFO("fpga_flag=%d\n", hisijd->fpga_flag);

	if (hisi_jpu_get_reg_base(hisijd, np)) {
		dev_err(dev, "failed to get reg base resource\n");
		return -ENXIO;
	}
	if (hisi_jpu_get_regulators(hisijd)) {
		dev_err(dev, "failed to get jpu regulator\n");
		return -ENXIO;
	}

	/* get jpg_func_clk_name resource */
	ret = of_property_read_string_index(np, "clock-names", 0,
			&(hisijd->jpg_func_clk_name));
	if (ret != 0) {
		dev_err(dev,
			"failed to get jpg_func_clk_name resource! ret=%d\n",
			ret);
		return -ENXIO;
	}

	/* get jpg platfrom name resource */
	if (hisi_jpu_set_platform(hisijd, np) != 0) {
		dev_err(dev, "failed to set platform info\n");
		return -ENXIO;
	}

	hisijd->is_irq_merge = false;
	if (hisijd->jpu_support_platform == HISI_DSS_V510_CS)
		hisijd->is_irq_merge = true;

	/* get irq no */
	if (hisi_jpu_get_irqs(hisijd, np)) {
		dev_err(dev, "failed to get jpu irq\n");
		return -ENXIO;
	}

	hisijd->jpg_func_clk = devm_clk_get(&(hisijd->pdev->dev),
			hisijd->jpg_func_clk_name);
	if (IS_ERR(hisijd->jpg_func_clk)) {
		dev_err(dev, "jpg_func_clk devm_clk_get error!");
		return -ENXIO;
	}

	ret = hisi_jpu_chrdev_setup(hisijd);
	if (ret != 0) {
		dev_err(dev, "fail to hisi_jpu_chrdev_setup!\n");
		return -ENXIO;
	}
	return 0;
}

/*lint -restore*/
/*lint -save -e438 -e593*/
static int hisi_jpu_probe(struct platform_device *pdev)
{
	int ret;
	struct hisi_jpu_data_type *hisijd = NULL;
	struct device_node *np = NULL;
	struct device *dev = NULL;

	HISI_JPU_INFO("+\n");

	if (pdev == NULL) {
		HISI_JPU_ERR("platform_device is null\n");
		return -ENXIO;
	}

	dev = &pdev->dev;
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_JPU_NAME);
	if (np == NULL) {
		dev_err(dev, "NOT FOUND device node %s!\n", DTS_COMP_JPU_NAME);
		return -ENXIO;
	}

	hisijd = (struct hisi_jpu_data_type *)kzalloc(
			sizeof(struct hisi_jpu_data_type),
			GFP_KERNEL);
	if (hisijd == NULL) {
		dev_err(dev, "failed to alloc hisijd\n");
		return -ENXIO;
	}

	// cppcheck-suppress *
	(void)memset_s(hisijd, sizeof(struct hisi_jpu_data_type),
		0, sizeof(struct hisi_jpu_data_type));
	hisijd->pdev = pdev;

	/* get jpu dts config */
	ret = hisi_jpu_get_dts_resource(np, hisijd, dev);
	if (ret != 0) {
		dev_err(dev, "failed to get dts resource\n");
		ret = -ENXIO;
		goto err_device_put;
	}

	platform_set_drvdata(pdev, hisijd);
	g_hisijd = platform_get_drvdata(pdev);
	if (g_hisijd == NULL) {
		dev_err(dev, "hisijd load and reload failed!\n");
		goto err_device_put;
	}

	/* jpu register */
	ret = hisi_jpu_register(hisijd);
	if (ret != 0) {
		dev_err(dev, "fail to hisi_jpu_register!\n");
		goto err_device_put;
	}

	HISI_JPU_INFO("-\n");

	return 0;
err_device_put:
	if (hisi_jpu_remove(pdev) != 0)
		dev_err(dev, "hisi_jpu_remove failed!\n");

	hisijd = platform_get_drvdata(pdev);
	if (hisijd != NULL) {
		kfree(hisijd);
		hisijd = NULL;
		g_hisijd = NULL;
	}
	return ret;
}

/*lint -restore*/

#if defined(CONFIG_PM_SLEEP)
static int hisi_jpu_suspend(struct device *dev)
{
	struct hisi_jpu_data_type *hisijd = NULL;

	HISI_JPU_INFO("+\n");

	if (dev == NULL) {
		HISI_JPU_ERR("dev is NULL!\n");
		return -EINVAL;
	}

	hisijd = dev_get_drvdata(dev);
	if (hisijd == NULL) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	HISI_JPU_INFO("-\n");
	return 0;
}
#else
#define hisi_jpu_suspend NULL
#endif

static void hisi_jpu_shutdown(struct platform_device *pdev)
{
	struct hisi_jpu_data_type *hisijd = NULL;

	HISI_JPU_INFO("+\n");

	if (pdev == NULL) {
		HISI_JPU_ERR("pdev is NULL!\n");
		return;
	}

	hisijd = platform_get_drvdata(pdev);
	if (hisijd == NULL) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return;
	}

	HISI_JPU_INFO("-\n");
}

/* lint -save -e785 */
static const struct of_device_id hisi_jpu_match_table[] = {
	{
	 .compatible = DTS_COMP_JPU_NAME,
	 .data = NULL,
	 },
	{},
};

MODULE_DEVICE_TABLE(of, hisi_jpu_match_table);

static const struct dev_pm_ops hisi_jpu_dev_pm_ops = {
#ifdef CONFIG_PM_SLEEP
	.suspend = hisi_jpu_suspend,
	.resume = NULL,
#endif
};

static struct platform_driver hisi_jpu_driver = {
	.probe = hisi_jpu_probe,
	.remove = hisi_jpu_remove,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = hisi_jpu_shutdown,
	.driver = {
		.name = DEV_NAME_JPU,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_jpu_match_table),
		.pm = &hisi_jpu_dev_pm_ops,
	},
};


/*lint -save -e602 -e603*/
static int __init hisi_jpu_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&hisi_jpu_driver);
	if (ret != 0) {
		HISI_JPU_ERR("driver register failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

static void __exit hisi_jpu_exit(void)
{
	platform_driver_unregister(&hisi_jpu_driver);
}

module_init(hisi_jpu_init);
module_exit(hisi_jpu_exit);

MODULE_DESCRIPTION("Hisilicon JPU Driver");
MODULE_LICENSE("GPL v2");
