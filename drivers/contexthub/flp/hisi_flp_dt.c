/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub flp dt driver.
 * Author: Huawei
 * Create: 2016-03-15
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/pm.h>
#include <linux/platform_device.h>
#include "../common.h"
#include <libhwsecurec/securec.h>
#include "hisi_flp.h"

static int generic_flp_probe(struct platform_device *pdev);
static int generic_flp_remove(struct platform_device *pdev);

unsigned int g_flp_debug_level = 0;

/*lint -e715*/
/*
 * 显示DEBUG的优先级
 */
static ssize_t show_debug_level(struct device_driver *d, char *buf)
{
	if (buf == NULL) {
		pr_err("[%s] buf is err.\n", __func__);
		return -EINVAL;
	}

	return snprintf_s(buf, (size_t)PAGE_SIZE, (size_t)PAGE_SIZE, "0x%08X\n", g_flp_debug_level);
}

/*
 * 保存DEBUG的优先级
 */
static ssize_t store_debug_level(struct device_driver *d, const char *buf, size_t count)
{
	char *p = NULL ;
	unsigned int val;

	if (buf == NULL) {
		pr_err("[%s] buf is err.\n", __func__);
		return -EINVAL;
	}

	p = (char *)buf;

	if (p[1] == 'x' || p[1] == 'X' || p[0] == 'x' || p[0] == 'X') {
		p++;
		if (p[0] == 'x' || p[0] == 'X') {
			p++;
		}
		val = (unsigned int)simple_strtoul(p, &p, 16);
	} else {
		val = (unsigned int)simple_strtoul(p, &p, 10);
	}

	if (p == buf) {
		pr_info(": %s is not in hex or decimal form.\n", buf);
	}

	if ((0 == val) || (1 == val)) {
		g_flp_debug_level = val;
	}
	pr_info("flp:%s debug_level:%d", buf, val);
	return (ssize_t)strnlen(buf, count);
}
/*lint +e715*/

/*lint -e846 -e514 -e778 -e866 -e84 */
static struct driver_attribute driver_attr_debug_level =
			__ATTR(debug_level, 0660, show_debug_level, store_debug_level);
/*lint +e846 +e514 +e778 +e866 +e84 */

static int hisi_flp_pm_suspend(struct device *dev)
{
	dev_info(dev, "%s %d: \n", __func__, __LINE__);
	return 0;
}

/*
 * 驱动resume函数
 */
static int hisi_flp_pm_resume(struct device *dev)
{
	dev_info(dev, "%s %d: \n", __func__, __LINE__);
	flp_port_resume();
	return 0;
}

/*lint -e785*/
struct dev_pm_ops hisi_flp_pm_ops = {
#ifdef CONFIG_PM_SLEEP
	.suspend = hisi_flp_pm_suspend,
	.resume  = hisi_flp_pm_resume,
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
		.name	= "hisi-flp",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(generic_flp),
		.pm = &hisi_flp_pm_ops,
	},
	.probe = generic_flp_probe,
	.remove = generic_flp_remove,
};
/*lint +e64*/
/*lint +e785*/

/*
 * 驱动probe函数获取dts状态，判断是否创建驱动
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
	if (ret != 0) {
		dev_info(&pdev->dev, "%s %d create file error: \n", __func__, __LINE__);
	}
#endif
	ret = hisi_flp_register();
	return ret;
}

/*
 * 驱动remove函数
 */
static int generic_flp_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "%s %d: \n", __func__, __LINE__);
	driver_remove_file(&generic_flp_platdrv.driver,
			   &driver_attr_debug_level);
	hisi_flp_unregister();
	return 0;
}

/*lint -e64*/
static int __init hisi_flp_init(void)
{
	return platform_driver_register(&generic_flp_platdrv);
}
/*lint +e64*/

static void __exit hisi_flp_exit(void)
{
	platform_driver_unregister(&generic_flp_platdrv);
}

/*lint -e528 -esym(528,*) */
late_initcall_sync(hisi_flp_init);
module_exit(hisi_flp_exit);
/*lint +e528 -esym(528,*) */
