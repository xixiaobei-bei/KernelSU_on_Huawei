/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * pmic_dcxo_ap.c
 *
 * driver for pmic ap dcxo calibration
 *
 * This software is licensed under the terms of the GNU General Public
 * either version 2 of that License or (at your option) any later version.
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/mfd/hisi_pmic_dcxo.h>
#include <hisi_pmic_dcxo_ap.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/mtd/hisi_nve_number.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/platform_device.h>
#include <linux/thermal.h>
#include <linux/of.h>
#include <securec.h>

#define DCXO_DEV_NAME        "pmic_dcxo"

struct pmic_dcxo_device *dcxodev_table[DCXO_MAX_DEVICES];
static DEFINE_MUTEX(minors_lock);

/*
 * Function: hkadc_get_xoadc_temp
 * Description: read xo temp from hkadc interface
 * return value: temperature (1000*degree)
 */
static int hkadc_get_xoadc_temp(void)
{
	int ret;
	struct thermal_zone_device *tz = NULL;
	int temp = 0;

	tz = thermal_zone_get_zone_by_name(HKADC_XOADC);
	ret = thermal_zone_get_temp(tz, &temp);
	if (ret)
		temp = DEFAULT_TEMP;
	pr_err("the xoadc's temp is %d\n", temp);
	return temp;
}

static int pmu_dcxo_get_set(
	uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix, bool get)
{
	int ret;
	struct pmic_nve_cali nve_cali;
	errno_t err;

	err = strncpy_s(nve_cali.nve_name, sizeof(nve_cali.nve_name),
		NVE_CALC1C2_AP_NAME, sizeof(NVE_CALC1C2_AP_NAME) - 1);
	if (err != EOK) {
		pr_err("[%s]nve.nv_name strncpy_s failed\n", __func__);
		ret = -1;
		return ret;
	}
	nve_cali.nve_num = NVE_CALC1C2_AP_NUM;
	ret = pmu_dcxo_get_set_nv(dcxo_ctrim, dcxo_c2_fix, get, &nve_cali);
	if (ret)
		pr_err("[%s]%d pmu_dcxo_get_set_nv error\n", __func__, get);

	return ret;
}

static int pmu_dcxo_reg_get_set(
	uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix, bool get)
{
	int ret;
	struct pmic_cali_c1c2 cali_parameter;

	cali_parameter.c1_reg = PMIC_DCXO_CFIX1;
	cali_parameter.c2_reg = PMIC_DCXO_CFIX2;
	cali_parameter.c2_mask = PMIC_DCXO_CFIX2_MASK;
	cali_parameter.modem_cali_in_mmw_pmu = 0;

	ret = pmu_dcxo_reg_get_set_c1c2(AP_CALI, &cali_parameter,
		dcxo_ctrim, dcxo_c2_fix, get);
	if (ret)
		pr_err("[%s]error, %d\n", __func__, get);

	return ret;
}

static int pmic_dcxo_ioct_set(struct pmu_dcxo_cali_msg msg, struct pmic_dcxo_device *di)
{
	int ret = 0;
	enum pmu_dcxo_msg_type msg_type = msg.msg_type;
	uint16_t dcxo_ctrim;
	uint16_t dcxo_c2_fix;

	switch (msg_type) {
	case PMU_DCXO_CLK_EN:
		pmic_reg_write_mask(AP_CALI, 0, di->dcxo_clk.clk_en,
			1 << di->dcxo_clk.clk_shift, di->dcxo_clk.clk_mask);
		break;
	case PMU_DCXO_CLK_DIS:
		pmic_reg_write_mask(AP_CALI, 0, di->dcxo_clk.clk_en,
			0 << di->dcxo_clk.clk_shift, di->dcxo_clk.clk_mask);
		break;
	case PMU_DCXO_CALI_REG_SET:
		dcxo_ctrim = msg.msg_data.dcxo_ctrim;
		dcxo_c2_fix = msg.msg_data.dcxo_c2_fix;
		ret = pmu_dcxo_reg_get_set(&dcxo_ctrim, &dcxo_c2_fix, 0);
		break;
	case PMU_DCXO_CALI_NV_SET:
		dcxo_ctrim = msg.msg_data.dcxo_ctrim;
		dcxo_c2_fix = msg.msg_data.dcxo_c2_fix;
		ret = pmu_dcxo_get_set(&dcxo_ctrim, &dcxo_c2_fix, 0);
		break;
	case PMU_DCXO_CALI_RC_EN:
		pmic_reg_write_mask(AP_CALI, 0, PMIC_DCXO_CALI_RC_EN,
			1 << PMIC_DCXO_CALI_RC_EN_SHIFT,
			PMIC_DCXO_CALI_RC_EN_MASK);
		break;
	case PMU_DCXO_CALI_RC_DIS:
		pmic_reg_write_mask(AP_CALI, 0, PMIC_DCXO_CALI_RC_EN,
			0 << PMIC_DCXO_CALI_RC_EN_SHIFT,
			PMIC_DCXO_CALI_RC_EN_MASK);
		break;
	default:
		pr_err("[%s] msg_type %u error\n", __func__, msg_type);
		ret = -EINVAL;
		break;
	};

	return ret;
}
static int pmic_dcxo_ioct_get(struct pmu_dcxo_cali_msg *msg, struct pmic_dcxo_device *di)
{
	enum pmu_dcxo_msg_type msg_type = msg->msg_type;
	uint16_t *dcxo_ctrim = &(msg->msg_data.dcxo_ctrim);
	uint16_t *dcxo_c2_fix = &(msg->msg_data.dcxo_c2_fix);
	unsigned int val;
	int ret = 0;

	switch (msg_type) {
	case PMU_DCXO_CLK_STATUS_GET:
		val = pmic_reg_read(AP_CALI, 0, di->dcxo_clk.clk_en);
		msg->get_val = val & PMIC_DCXO_CLK_EN_MASK;
		break;
	case PMU_DCXO_TEMP_GET:
		msg->get_val = hkadc_get_xoadc_temp();
		break;
	case PMU_DCXO_CALI_REG_GET:
		ret = pmu_dcxo_reg_get_set(dcxo_ctrim, dcxo_c2_fix, 1);
		break;
	case PMU_DCXO_CALI_NV_GET:
		ret = pmu_dcxo_get_set(dcxo_ctrim, dcxo_c2_fix, 1);
		break;
	default:
		pr_err("[%s] msg_type %u error\n", __func__, msg_type);
		ret = -EFAULT;
		break;
	};

	return ret;
}

static long pmic_dcxo_ioctl(struct file *filp, unsigned int cmd,
	unsigned long args)
{
	int ret = 0;
	void __user *user_args = NULL;
	struct pmu_dcxo_cali_msg msg;
	struct pmic_dcxo_device *di = NULL;

	mutex_lock(&minors_lock);
	if (filp->private_data == NULL) {
		pr_err("[%s] filp private_data is NULL\n", __func__);
		mutex_unlock(&minors_lock);
		return -EFAULT;
	}

	di = filp->private_data;

#ifndef CONFIG_PRODUCT_ARMPC
	if (strstr(saved_command_line, "androidboot.swtype=factory")) {
#endif
		user_args = (void __user *)(uintptr_t)args;
		if (user_args == NULL) {
			pr_err("[%s] user_args %pK\n", __func__, user_args);
			mutex_unlock(&minors_lock);
			return -EINVAL;
		}

		ret = copy_from_user(&msg, user_args,
				sizeof(msg));
		if (ret != 0) {
			pr_err("[%s] copy_from_user %d\n", __func__, ret);
			mutex_unlock(&minors_lock);
			return -EFAULT;
		}
		if (msg.msg_data.dcxo_ctrim > 0xff) {
			mutex_unlock(&minors_lock);
			return -EINVAL;
		}
		if (msg.msg_data.dcxo_c2_fix > 0xff) {
			mutex_unlock(&minors_lock);
			return -EINVAL;
		}
		switch (cmd) {
		case PMU_DCXO_SET_CMD:
			ret = pmic_dcxo_ioct_set(msg, di);
			if (ret != 0) {
				pr_err("[%s] pmic_dcxo_ioct_set %d\n", __func__,
					ret);
				mutex_unlock(&minors_lock);
				return -EFAULT;
			}
			break;
		case PMU_DCXO_GET_CMD:
			ret = pmic_dcxo_ioct_get(&msg, di);
			if (ret != 0) {
				pr_err("[%s] pmic_dcxo_ioct_get %d\n", __func__,
					ret);
				mutex_unlock(&minors_lock);
				return -EFAULT;
			}
			ret = copy_to_user(user_args, &msg, sizeof(msg));
			if (ret != 0) {
				pr_err("[%s] copy_to_user %d\n", __func__, ret);
				mutex_unlock(&minors_lock);
				return -EFAULT;
			}
			break;
		default:
			mutex_unlock(&minors_lock);
			return -EINVAL;
		}
#ifndef CONFIG_PRODUCT_ARMPC
	}
#endif
	mutex_unlock(&minors_lock);
	return ret;
}

#ifdef CONFIG_HISI_PMIC_DEBUG
int pmu_dcxo_ap_test(int get, uint16_t v1, uint16_t v2)
{
	int ret;
	uint16_t dcxo_ap_ctrim = v1;
	uint16_t dcxo_ap_c2_fix = v2;

	pr_err("[%s]test get ap trim value, ctrim = 0x%x, dcxo_ap_c2_fix = 0x%x\n",
		__func__, dcxo_ap_ctrim, dcxo_ap_c2_fix);

	ret = pmu_dcxo_get_set(&dcxo_ap_ctrim, &dcxo_ap_c2_fix, get);

	pr_err("[%s]test get ap trim value, ctrim = 0x%x, dcxo_ap_c2_fix = 0x%x\n",
		__func__, dcxo_ap_ctrim, dcxo_ap_c2_fix);

	return ret;
}
#endif

static int pmic_dcxo_open(struct inode *inode, struct file *file)
{
	unsigned int minor = iminor(inode);

	mutex_lock(&minors_lock);
	if (dcxodev_table[minor] == NULL) {
		mutex_unlock(&minors_lock);
		return -ENODEV;
	}

	file->private_data = dcxodev_table[minor];
	mutex_unlock(&minors_lock);
	return 0;
}

const static struct file_operations pmic_dcxo_fops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = pmic_dcxo_ioctl,
	.open = pmic_dcxo_open,
};

static void pmu_dcxo_get_clk_property(struct pmic_dcxo_device *di)
{
	const char *ref_clk = NULL;
	int ret;

	di->dcxo_clk.clk_en = PMIC_DCXO_CLK_EN;
	di->dcxo_clk.clk_mask = PMIC_DCXO_CLK_EN_MASK;
	di->dcxo_clk.clk_shift = PMIC_DCXO_CLK_EN_SHIFT;

	ret = of_property_read_string(di->dev->of_node, "ref_clk", &ref_clk);
	if (ret) {
		pr_err("pmu dcxo  sys clk property get\n");
		return;
	}

	if (strstr(ref_clk, "rf0_clk")) {
		di->dcxo_clk.clk_en = PMIC_DCXO_RF0_CLK_EN;
		di->dcxo_clk.clk_mask = PMIC_DCXO_RF0_CLK_EN_MASK;
		di->dcxo_clk.clk_shift = PMIC_DCXO_RF0_CLK_EN_SHIFT;
		pr_err("pmu dcxo  rf0 clk property get\n");
		return;
	}

	if (strstr(ref_clk, "rf1_clk")) {
		di->dcxo_clk.clk_en = PMIC_DCXO_RF1_CLK_EN;
		di->dcxo_clk.clk_mask = PMIC_DCXO_RF1_CLK_EN_MASK;
		di->dcxo_clk.clk_shift = PMIC_DCXO_RF1_CLK_EN_SHIFT;
		pr_err("pmu dcxo  rf1 clk property get\n");
		return;
	}
}


static int pmic_dcxo_ap_probe(struct platform_device *pdev)
{
	struct pmic_dcxo_device *di = NULL;
	struct device_node *np = NULL;
	struct device *pdevice = NULL;
	int minor;

	di = (struct pmic_dcxo_device *)devm_kzalloc(&pdev->dev,
		sizeof(*di), GFP_KERNEL);
	if (!di) {
		pr_err("%s failed to alloc di struct\n", __func__);
		return -ENOMEM;
	}

	di->dev = &pdev->dev;
	np = di->dev->of_node;

	platform_set_drvdata(pdev, di);

	di->dcxoid_major = register_chrdev(0, DCXO_DEV_NAME, &pmic_dcxo_fops);
	if (di->dcxoid_major < 0) {
		pr_err("pmic_dcxo: unable to get dcxoid_major for memory devs\n");
		return -EFAULT;
	}

	mutex_lock(&minors_lock);
	di->dcxoid_class = class_create(THIS_MODULE, DCXO_DEV_NAME);
	if (IS_ERR(di->dcxoid_class)) {
		pr_err("pmic_dcxo: class_create error\n");
		goto error1;
	}

	for (minor = 0; minor < DCXO_MAX_DEVICES; minor++) {
		if (dcxodev_table[minor])
			continue;
		dcxodev_table[minor] = di;
		break;
	}
	if (minor >= DCXO_MAX_DEVICES) {
		pr_err("pmic_dcxo: dcxo device table is full\n");\
		goto error2;
	}

	pdevice = device_create(di->dcxoid_class, NULL, MKDEV(di->dcxoid_major, minor), NULL, DCXO_DEV_NAME);
	if (IS_ERR(pdevice)) {
		pr_err("pmic_dcxo: device_create error\n");
		dcxodev_table[minor] = NULL;
		goto error2;
	}

	pmu_dcxo_get_clk_property(di);
	mutex_unlock(&minors_lock);
	pr_info("[%s]ok!\n", __func__);

	return 0;

error2:
	class_destroy(di->dcxoid_class);
	di->dcxoid_class = NULL;
error1:
	unregister_chrdev(di->dcxoid_major, DCXO_DEV_NAME);
	di->dcxoid_major = 0;
	mutex_unlock(&minors_lock);
	return -ENODEV;
}

static int pmic_dcxo_remove(struct platform_device *pdev)
{
	struct pmic_dcxo_device *di = platform_get_drvdata(pdev);

	if (!di) {
		pr_err("[%s]di is NULL!\n", __func__);
		return -ENODEV;
	}
	device_destroy(di->dcxoid_class, MKDEV(di->dcxoid_major, 0));
	class_destroy(di->dcxoid_class);
	di->dcxoid_class = NULL;
	unregister_chrdev(di->dcxoid_major, DCXO_DEV_NAME);
	di->dcxoid_major = 0;
	return 0;
}

const static struct of_device_id pmic_dcxo_ap_match_table[] = {
	{
		.compatible = "hisilicon,pmic_dcxo_ap",
	},
	{ },   /* end */
};

static struct platform_driver pmic_dcxo_ap_driver = {
	.probe      = pmic_dcxo_ap_probe,
	.remove     = pmic_dcxo_remove,
	.driver     = {
		.name           = "pmic_dcxo_ap",
		.owner          = THIS_MODULE,
		.of_match_table = pmic_dcxo_ap_match_table,
	},
};

int __init pmic_dcxo_ap_init(void)
{
	return platform_driver_register(&pmic_dcxo_ap_driver);
}

void __exit pmic_dcxo_ap_exit(void)
{
	platform_driver_unregister(&pmic_dcxo_ap_driver);
}

late_initcall(pmic_dcxo_ap_init);
module_exit(pmic_dcxo_ap_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("pmic dcxo ap module");
