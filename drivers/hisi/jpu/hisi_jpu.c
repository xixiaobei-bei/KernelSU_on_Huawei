/*
 * This file is jpu device, driver registered and achieve.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "hisi_jpu.h"
#include <linux/slab.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/device.h>
#include <securec.h>
#include "hisi_jpu_def.h"

#define JPU_DSS_LEN             8
#define DEV_NAME_JPU            "dev_jpu"
#define DTS_COMP_JPU_NAME       "hisilicon,hisijpu"

uint32_t g_hisi_jpu_msg_level = 7;
int g_debug_jpu_dec;
int g_debug_jpu_dec_job_timediff;

module_param_named(debug_msg_level, g_hisi_jpu_msg_level, int, 0644);
MODULE_PARM_DESC(debug_msg_level, "hisi jpu msg level");

#ifdef CONFIG_FB_DEBUG_USED
module_param_named(debug_jpu_dec, g_debug_jpu_dec, int, 0644);
MODULE_PARM_DESC(debug_jpu_dec, "hisi jpu decode debug");
#endif

#ifdef CONFIG_FB_DEBUG_USED
module_param_named(debug_jpu_decode_job_timediff, g_debug_jpu_dec_job_timediff,
	int, 0644);
MODULE_PARM_DESC(debug_jpu_decode_job_timediff,
	"hisi jpu decode job timediff debug");
#endif

static struct hisi_jpu_data_type *g_hisijd = NULL;

static int hisi_jpu_open(struct inode *inode, struct file *filp)
{
	struct hisi_jpu_data_type *hisijd = NULL;
	hisi_jpu_info("+\n");
	jpu_check_null_return(filp, -EINVAL);

	if (filp->private_data == NULL)
		filp->private_data = g_hisijd;

	hisijd = filp->private_data;
	jpu_check_null_return(hisijd, -EINVAL);

	hisijd->ref_cnt++;
	hisi_jpu_info("-\n");
	return 0;
}

static int hisi_jpu_release(struct inode *inode, struct file *filp)
{
	struct hisi_jpu_data_type *hisijd = NULL;

	hisi_jpu_info("+\n");
	jpu_check_null_return(filp, -EINVAL);
	jpu_check_null_return(filp->private_data, -EINVAL);

	hisijd = filp->private_data;
	if (hisijd->ref_cnt <= 0) {
		hisi_jpu_info("try to close unopened jpu exit\n");
		return -EINVAL;
	}

	hisijd->ref_cnt--;
	hisi_jpu_info("-\n");
	return 0;
}

#ifdef CONFIG_COMPAT
static long hisi_jpu_ioctl(struct file *filp, u_int cmd, u_long arg)
{
	int ret;
	struct hisi_jpu_data_type *hisijd = NULL;
	void __user *argp = (void __user *)(uintptr_t)arg;

	hisi_jpu_debug("+\n");
	jpu_check_null_return(filp, -EINVAL);
	jpu_check_null_return(filp->private_data, -EINVAL);

	hisijd = filp->private_data;

	if (cmd == HISIJPU_JOB_EXEC)
		ret = hisijpu_job_exec(hisijd, argp);
	else
		ret = -ENOSYS;

	if (ret != 0)
		hisi_jpu_err("unsupported ioctl %x, ret = %d\n", cmd, ret);

	hisi_jpu_debug("-\n");
	return ret;
}
#endif

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

static int hisi_jpu_set_platform(struct hisi_jpu_data_type *hisijd,
	struct device_node *np)
{
	int ret = of_property_read_string_index(np, "platform-names", 0,
		&(hisijd->jpg_platform_name));
	if ((ret != 0) || (hisijd->jpg_platform_name == NULL)) {
		hisi_jpu_err("failed to get jpg resource ret = %d\n", ret);
		return -EINVAL;
	}

	if (strncmp(hisijd->jpg_platform_name, "dss_v400", JPU_DSS_LEN) == 0) {
		hisijd->jpu_support_platform = HISI_DSS_V400;
	} else if (strncmp(hisijd->jpg_platform_name, "dss_v500", JPU_DSS_LEN) == 0) {
		hisijd->jpu_support_platform = HISI_DSS_V500;
	} else if (strncmp(hisijd->jpg_platform_name, "dss_v501", JPU_DSS_LEN) == 0) {
		hisijd->jpu_support_platform = HISI_DSS_V501;
	} else if (strncmp(hisijd->jpg_platform_name, "dss_v510", JPU_DSS_LEN) == 0) {
		hisijd->jpu_support_platform = HISI_DSS_V510;
	} else if (strncmp(hisijd->jpg_platform_name, "dss_v511", JPU_DSS_LEN) == 0) {
		hisijd->jpu_support_platform = HISI_DSS_V510_CS;
	} else if (strncmp(hisijd->jpg_platform_name, "dss_v600", JPU_DSS_LEN) == 0) {
		hisijd->jpu_support_platform = HISI_DSS_V600;
	} else {
		hisijd->jpu_support_platform = UNSUPPORT_PLATFORM;
		hisi_jpu_err("hisijd jpg platform is not support\n");
		return -EINVAL;
	}
	hisi_jpu_info("hisijd jpg platform is %d\n", hisijd->jpu_support_platform);
	return ret;
}

static int hisi_jpu_chrdev_setup(struct hisi_jpu_data_type *hisijd)
{
	const unsigned int minor = 0;

	/* get the major number of the character device */
	int ret = register_chrdev(hisijd->jpu_major, DEV_NAME_JPU, &hisi_jpu_fops);
	if (ret < 0) {
		hisi_jpu_err("fail to register driver\n");
		return -ENXIO;
	}

	hisijd->jpu_major = (uint32_t)ret;
	hisijd->jpu_class = class_create(THIS_MODULE, DEV_NAME_JPU);
	if (hisijd->jpu_class == NULL) {
		hisi_jpu_err("fail to create jpu class\n");
		ret = -ENOMEM;
		goto err_class_create;
	}

	hisijd->jpu_dev = device_create(hisijd->jpu_class, 0,
		MKDEV(hisijd->jpu_major, minor), NULL, DEV_NAME_JPU);
	if (hisijd->jpu_dev == NULL) {
		hisi_jpu_err("fail to create jpu device\n");
		ret = -ENOMEM;
		goto err_device_create;
	}

	return 0;

err_device_create:
	class_destroy(hisijd->jpu_class);
	hisijd->jpu_class = NULL;
err_class_create:
	unregister_chrdev(hisijd->jpu_major, DEV_NAME_JPU);
	hisijd->jpu_major = 0;
	return ret;
}

static void hisi_jpu_chrdev_remove(struct hisi_jpu_data_type *hisijd)
{
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
}

static int hisi_jpu_get_reg_base(struct hisi_jpu_data_type *hisijd,
	struct device_node *np)
{
	hisijd->jpu_dec_base = of_iomap(np, JPEG_DECODER_REG);
	jpu_check_null_return(hisijd->jpu_dec_base, -ENXIO);

	hisijd->jpu_top_base = of_iomap(np, JPEG_TOP_REG);
	jpu_check_null_return(hisijd->jpu_top_base, -ENXIO);

	hisijd->jpu_cvdr_base = of_iomap(np, JPEG_CVDR_REG);
	jpu_check_null_return(hisijd->jpu_cvdr_base, -ENXIO);

	hisijd->jpu_smmu_base = of_iomap(np, JPEG_SMMU_REG);
	jpu_check_null_return(hisijd->jpu_smmu_base, -ENXIO);

	hisijd->media1_crg_base = of_iomap(np, JPEG_MEDIA1_REG);
	jpu_check_null_return(hisijd->media1_crg_base, -ENXIO);

	hisijd->peri_crg_base = of_iomap(np, JPEG_PERI_REG);
	jpu_check_null_return(hisijd->peri_crg_base, -ENXIO);

	hisijd->pmctrl_base = of_iomap(np, JPEG_PMCTRL_REG);
	jpu_check_null_return(hisijd->pmctrl_base, -ENXIO);

	hisijd->sctrl_base = of_iomap(np, JPEG_SCTRL_REG);
	jpu_check_null_return(hisijd->sctrl_base, -ENXIO);


	return 0;
}

static int hisi_jpu_get_irqs(struct hisi_jpu_data_type *hisijd,
	struct device_node *np)
{
	hisijd->jpu_err_irq = irq_of_parse_and_map(np, JPEG_ERR_IRQ);
	if (hisijd->jpu_err_irq == 0) {
		hisi_jpu_err("failed to get jpu_err_irq resource\n");
		return -ENXIO;
	}

	hisijd->jpu_done_irq = irq_of_parse_and_map(np, JPEG_DONE_IRQ);
	if (hisijd->jpu_done_irq == 0) {
		hisi_jpu_err("failed to get jpu_done_irq resource\n");
		return -ENXIO;
	}

	hisijd->jpu_other_irq = irq_of_parse_and_map(np, JPEG_OTHER_IRQ);
	if (hisijd->jpu_other_irq == 0) {
		hisi_jpu_err("failed to get jpu_other_irq resource\n");
		return -ENXIO;
	}
	hisi_jpu_info("jpu_err_irq:%u, jpu_done:%u, jpu_othe:%u\n",
		hisijd->jpu_err_irq, hisijd->jpu_done_irq,
		hisijd->jpu_other_irq);

	return 0;
}


static int hisi_jpu_get_regulators(struct hisi_jpu_data_type *hisijd)
{
	/* get jpu regulator */
	hisijd->jpu_regulator = devm_regulator_get(&(hisijd->pdev->dev),
		"jpu-regulator");
	if (IS_ERR(hisijd->jpu_regulator)) {
		hisi_jpu_err("failed to get jpu_regulator\n");
		return -ENXIO;
	}

	/* get media1 regulator */
	hisijd->media1_regulator = devm_regulator_get(&(hisijd->pdev->dev),
		"media1-regulator");
	if (IS_ERR(hisijd->media1_regulator)) {
		hisi_jpu_err("failed to get media1_regulator\n");
		return -ENXIO;
	}

	return 0;
}

static void hisi_jpu_put_regulators(struct hisi_jpu_data_type *hisijd)
{
	if (hisijd->jpu_regulator != NULL)
		devm_regulator_put(hisijd->jpu_regulator);

	if (hisijd->media1_regulator != NULL)
		devm_regulator_put(hisijd->media1_regulator);
}

static int hisi_jpu_remove(struct platform_device *pdev)
{
	struct hisi_jpu_data_type *hisijd = NULL;

	hisi_jpu_info("+\n");
	jpu_check_null_return(pdev, -ENXIO);

	hisijd = platform_get_drvdata(pdev);
	jpu_check_null_return(hisijd, -EINVAL);

	if (hisi_jpu_unregister(hisijd) != 0) {
		hisi_jpu_err("hisi_jpu_unregister failed\n");
		return -EINVAL;
	}

	hisi_jpu_chrdev_remove(hisijd);

	/* clk handle */
	if ((hisijd->jpg_func_clk != NULL) && (hisijd->pdev != NULL))
		devm_clk_put(&(hisijd->pdev->dev), hisijd->jpg_func_clk);

	hisi_jpu_put_regulators(hisijd);

	if (hisijd != NULL) {
		kfree(hisijd);
		hisijd = NULL;
		platform_set_drvdata(pdev, hisijd);
	}
	g_hisijd = NULL;
	hisi_jpu_info("-\n");
	return 0;
}

static int hisi_jpu_get_reg_name_resource(struct device_node *np,
	struct hisi_jpu_data_type *hisijd, struct device *dev)
{
	int ret = 0;

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
		dev_err(dev, "failed to get jpg_func_clk_name resource ret = %d\n",
			ret);
		return -ENXIO;
	}

	/* get jpg platfrom name resource */
	if (hisi_jpu_set_platform(hisijd, np) != 0) {
		dev_err(dev, "failed to set platform info\n");
		return -ENXIO;
	}
	return ret;
}

static int hisi_jpu_get_dts_resource(struct device_node *np,
	struct hisi_jpu_data_type *hisijd, struct device *dev)
{
	if (of_property_read_u32(np, "fpga_flag", &(hisijd->fpga_flag)) != 0) {
		dev_err(dev, "failed to get fpga_flag resource\n");
		return -ENXIO;
	}
	hisi_jpu_info("fpga_flag = %u\n", hisijd->fpga_flag);

	if (hisi_jpu_get_reg_name_resource(np, hisijd, dev) != 0)
		return -ENXIO;

	/* get irq no */
	if (hisi_jpu_get_irqs(hisijd, np)) {
		dev_err(dev, "failed to get jpu irq\n");
		return -ENXIO;
	}


	hisijd->jpg_func_clk = devm_clk_get(&(hisijd->pdev->dev),
		hisijd->jpg_func_clk_name);
	if (IS_ERR(hisijd->jpg_func_clk)) {
		dev_err(dev, "jpg_func_clk devm_clk_get error");
		return -ENXIO;
	}

	if (hisi_jpu_chrdev_setup(hisijd) != 0) {
		dev_err(dev, "fail to hisi_jpu_chrdev_setup\n");
		return -ENXIO;
	}
	return 0;
}

static int hisi_jpu_dev_alloc(struct device *dev,
	struct device_node **np, struct hisi_jpu_data_type **hisijd)
{
	*np = of_find_compatible_node(NULL, NULL, DTS_COMP_JPU_NAME);
	if (*np == NULL) {
		dev_err(dev, "NOT FOUND device node %s\n", DTS_COMP_JPU_NAME);
		return -ENXIO;
	}

	*hisijd = (struct hisi_jpu_data_type *)kzalloc(
		sizeof(struct hisi_jpu_data_type), GFP_KERNEL);
	if (*hisijd == NULL) {
		dev_err(dev, "failed to alloc hisijd\n");
		return -ENXIO;
	}
	return 0;
}

static int hisi_jpu_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = NULL;
	struct device_node *np = NULL;
	struct hisi_jpu_data_type *hisijd = NULL;

	hisi_jpu_info("+\n");
	jpu_check_null_return(pdev, -ENXIO);

	dev = &pdev->dev;
	if (hisi_jpu_dev_alloc(dev, &np, &hisijd) != 0)
		return -ENXIO;

	/* cppcheck-suppress */
	(void)memset_s(hisijd, sizeof(struct hisi_jpu_data_type),
		0, sizeof(struct hisi_jpu_data_type));
	hisijd->pdev = pdev;

	/* get jpu dts config */
	ret = hisi_jpu_get_dts_resource(np, hisijd, dev);
	if (ret != 0) {
		dev_err(dev, "failed to get dts resource\n");
		goto err_device_put;
	}

	platform_set_drvdata(pdev, hisijd);
	g_hisijd = platform_get_drvdata(pdev);
	if (g_hisijd == NULL) {
		dev_err(dev, "hisijd load and reload failed\n");
		ret = -ENXIO;
		goto err_device_put;
	}

	/* jpu register */
	ret = hisi_jpu_register(hisijd);
	if (ret != 0) {
		dev_err(dev, "fail to hisi_jpu_register\n");
		goto err_device_put;
	}
	hisi_jpu_info("-\n");
	return 0;
err_device_put:
	if (hisi_jpu_remove(pdev) != 0)
		dev_err(dev, "hisi_jpu_remove failed\n");

	hisijd = platform_get_drvdata(pdev);
	if (hisijd != NULL) {
		kfree(hisijd);
		hisijd = NULL;
		platform_set_drvdata(pdev, hisijd);
	}
	g_hisijd = NULL;
	return ret;
}

#if defined(CONFIG_PM_SLEEP)
static int hisi_jpu_suspend(struct device *dev)
{
	struct hisi_jpu_data_type *hisijd = NULL;

	hisi_jpu_info("+\n");
	jpu_check_null_return(dev, -EINVAL);

	hisijd = dev_get_drvdata(dev);
	jpu_check_null_return(hisijd, -EINVAL);

	hisi_jpu_info("-\n");
	return 0;
}
#else
#define hisi_jpu_suspend NULL
#endif

static void hisi_jpu_shutdown(struct platform_device *pdev)
{
	struct hisi_jpu_data_type *hisijd = NULL;
	hisi_jpu_info("+\n");
	if (pdev == NULL) {
		hisi_jpu_err("pdev is NULL\n");
		return;
	}

	hisijd = platform_get_drvdata(pdev);
	if (hisijd == NULL)
		hisi_jpu_err("hisijd is NULL\n");

	hisi_jpu_info("-\n");
}

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

static int __init hisi_jpu_init(void)
{
	int ret = platform_driver_register(&hisi_jpu_driver);
	if (ret != 0) {
		hisi_jpu_err("driver register failed, error=%d\n", ret);
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
