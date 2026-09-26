/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub flp dt driver.
 * Create: 2016-03-15
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/pm.h>
#include <linux/platform_device.h>
#include "../common.h"
#include <securec.h>
#include "smart_flp.h"

static int generic_flp_probe(struct platform_device *pdev);
static int generic_flp_remove(struct platform_device *pdev);

unsigned int g_flp_debug_level = 0;
static struct mutex g_flp_dt_lock;

/*lint -e715*/
/*
 * Display the priority of DEBUG
 */
static ssize_t show_debug_level(struct device_driver *d, char *buf)
{
	ssize_t ret;
	if (buf == NULL) {
		pr_err("[%s] buf is err.\n", __func__);
		return -EINVAL;
	}
	mutex_lock(&g_flp_dt_lock);
	ret = snprintf_s(buf, (size_t)PAGE_SIZE, (size_t)PAGE_SIZE, "0x%08X\n", g_flp_debug_level);
	mutex_unlock(&g_flp_dt_lock);

	return ret;
}

/*
 * Save DEBUG priority
 */
static ssize_t store_debug_level(struct device_driver *d, const char *buf, size_t count)
{
	char *p = NULL ;
	unsigned int val;
	const int strtoul_16 = 16;
	const int strtoul_10 = 10;

	if (buf == NULL) {
		pr_err("[%s] buf is err.\n", __func__);
		return -EINVAL;
	}

	p = (char *)buf;

	if (p[1] == 'x' || p[1] == 'X' || p[0] == 'x' || p[0] == 'X') {
		p++;
		if (p[0] == 'x' || p[0] == 'X')
			p++;
		val = (unsigned int)simple_strtoul(p, &p, strtoul_16);
	} else {
		val = (unsigned int)simple_strtoul(p, &p, strtoul_10);
	}
	if (p == buf)
		pr_info(": %s is not in hex or decimal form.\n", buf);

	if ((val == 0) || (val == 1)) {
		mutex_lock(&g_flp_dt_lock);
		g_flp_debug_level = val;
		mutex_unlock(&g_flp_dt_lock);
	}
	pr_info("flp:%s debug_level:%d", buf, val);
	return (ssize_t)strnlen(buf, count);
}
/*lint +e715*/

/*lint -e846 -e514 -e778 -e866 -e84 */
static struct driver_attribute driver_attr_debug_level =
			__ATTR(debug_level, 0660, show_debug_level, store_debug_level);
/*lint +e846 +e514 +e778 +e866 +e84 */

static int flp_pm_suspend(struct device *dev)
{
	dev_info(dev, "%s %d: \n", __func__, __LINE__);
	return 0;
}

/*
 * Drive resume function
 */
static int flp_pm_resume(struct device *dev)
{
	dev_info(dev, "%s %d: \n", __func__, __LINE__);
	flp_port_resume();

	return 0;
}

/*lint -e785*/
struct dev_pm_ops flp_pm_ops = {
#ifdef CONFIG_PM_SLEEP
	.suspend = flp_pm_suspend,
	.resume  = flp_pm_resume,
#endif
};

static const struct of_device_id generic_flp[] = {
	{ .compatible = "hisilicon,flp-common" },
	{},
};
MODULE_DEVICE_TABLE(of, generic_flp);

/*lint -e64*/
static struct platform_driver generic_flp_platdrv = {
	.driver = {
		.name	= "smart-flp",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(generic_flp),
		.pm = &flp_pm_ops,
	},
	.probe = generic_flp_probe,
	.remove = generic_flp_remove,
};
/*lint +e64*/
/*lint +e785*/

/*
 * Drive probe function to get the dts status and determine whether to create a driver
 */
static int generic_flp_probe(struct platform_device *pdev)
{
	int ret;

	ret = get_contexthub_dts_status();
	if (ret != 0) {
		return ret;
	}
	dev_info(&pdev->dev, "generic_flp_probe \n");
#ifdef CONFIG_HISI_DEBUG_FS
	ret = driver_create_file(&generic_flp_platdrv.driver,
				 &driver_attr_debug_level);
	if (ret != 0)
		dev_info(&pdev->dev, "%s %d create file error: \n", __func__, __LINE__);
#endif
	mutex_init(&g_flp_dt_lock);
	ret = flp_register();
	return ret;
}

/*
 * Drive the remove function
 */
static int generic_flp_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "%s %d: \n", __func__, __LINE__);
	driver_remove_file(&generic_flp_platdrv.driver,
			   &driver_attr_debug_level);
	flp_unregister();
	return 0;
}

/*lint -e64*/
static int __init flp_init(void)
{
	return platform_driver_register(&generic_flp_platdrv);
}
/*lint +e64*/

static void __exit flp_exit(void)
{
	platform_driver_unregister(&generic_flp_platdrv);
}

/*lint -e528 -esym(528,*) */
late_initcall_sync(flp_init);
module_exit(flp_exit);
/*lint +e528 -esym(528,*) */
