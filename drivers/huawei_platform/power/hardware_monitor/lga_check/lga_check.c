/*
 * lga_check.c
 *
 * lga baoard abnormal monitor driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/types.h>

#include <linux/power/hisi/hisi_bci_battery.h>
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/power_dsm.h>
#include <huawei_platform/power/huawei_charger.h>

#include "lga_check.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG lga_check
HWLOG_REGIST();

static struct lga_ck_dev *g_lga_check_dev;

static void lga_dsm_dump(struct lga_ck_dev *l_dev, char *buff)
{
	switch (l_dev->lga_ck_mode) {
	case LGA_CK_MODE_ADC:
		snprintf(buff, POWER_DSM_BUF_SIZE_0128 - 1,
			"lga abnormal: adc_channel=%d, adc_vol=%d[mV]\n",
			l_dev->adc_channel_no, l_dev->adc_vol);
		break;

	default:
		snprintf(buff, POWER_DSM_BUF_SIZE_0128 - 1,
			"lga mode undefined\n");
		break;
	}
}

static int lga_ck_get_adc_vol(u32 adc_channel)
{
	int i;
	int adc_vol;

	for (i = 0; i < LGA_CK_ADC_MAX_RETRYS; i++) {
		adc_vol = hisi_adc_get_value(adc_channel);
		if (adc_vol < 0)
			hwlog_err("adc read channel %d fail, time=%d\n",
				adc_channel, i + 1);
		else
			break;
	}

	hwlog_info("adc_vol=%d\n", adc_vol);

	return adc_vol;
}

static int lga_status_check_adc_vol(struct lga_ck_dev *l_dev)
{
	l_dev->adc_vol = lga_ck_get_adc_vol(l_dev->adc_channel_no);

	if (l_dev->adc_vol > l_dev->adc_abnormal_thld) {
		hwlog_err("voltage is over threshold %d\n",
			l_dev->adc_abnormal_thld);
		return LGA_CK_FRACTURE_FOUND;
	}

	return LGA_CK_FRACTURE_FREE;
}

static int lga_status_check(struct lga_ck_dev *l_dev)
{
	int lga_status;

	switch (l_dev->lga_ck_mode) {
	case LGA_CK_MODE_ADC:
		lga_status = lga_status_check_adc_vol(l_dev);
		break;

	default:
		lga_status = LGA_CK_STATUS_DEFAULT;
		hwlog_err("wrong lga check mode index\n");
		break;
	}

	return lga_status;
}

static void lga_fracture_detection_work(struct work_struct *work)
{
	char dsm_buff[POWER_DSM_BUF_SIZE_0128] = { 0 };
	struct lga_ck_dev *l_dev = g_lga_check_dev;

	if (!l_dev) {
		hwlog_err("l_dev is null\n");
		return;
	}

	if (l_dev->lga_ck_abnormal_time >= LGA_CK_MAX_DMD_REPORT_TIME) {
		hwlog_err("lga abnormal over %d time\n",
			l_dev->lga_ck_abnormal_time);
		return;
	}

	hwlog_info("start lga check\n");

	if (lga_status_check(l_dev) == LGA_CK_FRACTURE_FOUND) {
		lga_dsm_dump(l_dev, dsm_buff);
		power_dsm_dmd_report(POWER_DSM_PMU_OCP, DSM_LGA_CHECK_ERROR_NO,
			dsm_buff);

		l_dev->lga_ck_abnormal_time++;
	}
}

static int lga_fracture_event_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct lga_ck_dev *l_dev = g_lga_check_dev;

	if (!l_dev) {
		hwlog_err("l_dev is null\n");
		return NOTIFY_OK;
	}

	if (event == CHARGER_START_CHARGING_EVENT) {
		cancel_delayed_work(&l_dev->lga_ck_ws);
		schedule_delayed_work(&l_dev->lga_ck_ws,
			msecs_to_jiffies(LGA_CK_DELAY_TIME));
	}

	return NOTIFY_OK;
}

#ifdef CONFIG_SYSFS
#define LGA_CK_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, lga_ck_sysfs_show, store), \
	.name = LGA_CK_SYSFS_##n, \
}

#define LGA_CK_SYSFS_FIELD_RO(_name, n) \
	LGA_CK_SYSFS_FIELD(_name, n, 0444, NULL)

struct lga_ck_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static ssize_t lga_ck_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct lga_ck_sysfs_field_info lga_ck_sysfs_field_tbl[] = {
	LGA_CK_SYSFS_FIELD_RO(adc_vol, ADC_VOL),
};

#define LGA_CK_SYSFS_ATTRS_SIZE (ARRAY_SIZE(lga_ck_sysfs_field_tbl) + 1)

static struct attribute *lga_ck_sysfs_attrs[LGA_CK_SYSFS_ATTRS_SIZE];

static const struct attribute_group lga_ck_sysfs_attr_group = {
	.attrs = lga_ck_sysfs_attrs,
};

static void lga_ck_sysfs_init_attrs(void)
{
	int s;
	int e = ARRAY_SIZE(lga_ck_sysfs_field_tbl);

	for (s = 0; s < e; s++)
		lga_ck_sysfs_attrs[s] = &lga_ck_sysfs_field_tbl[s].attr.attr;

	lga_ck_sysfs_attrs[e] = NULL;
}

static struct lga_ck_sysfs_field_info *lga_ck_sysfs_field_lookup(
	const char *name)
{
	int s;
	int e = ARRAY_SIZE(lga_ck_sysfs_field_tbl);

	for (s = 0; s < e; s++) {
		if (!strncmp(name, lga_ck_sysfs_field_tbl[s].attr.attr.name,
			strlen(name)))
			break;
	}

	if (s >= e)
		return NULL;

	return &lga_ck_sysfs_field_tbl[s];
}

static ssize_t lga_ck_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lga_ck_sysfs_field_info *info = NULL;
	struct lga_ck_dev *l_dev = g_lga_check_dev;
	int len;

	info = lga_ck_sysfs_field_lookup(attr->attr.name);
	if (!info || !l_dev) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case LGA_CK_SYSFS_ADC_VOL:
		len = scnprintf(buf, PAGE_SIZE, "%d\n",
			lga_ck_get_adc_vol(l_dev->adc_channel_no));
		break;

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		len = 0;
		break;
	}

	return len;
}

static int lga_ck_sysfs_create_group(struct lga_ck_dev *l_dev)
{
	lga_ck_sysfs_init_attrs();
	return sysfs_create_group(&l_dev->dev->kobj, &lga_ck_sysfs_attr_group);
}

static void lga_ck_sysfs_remove_group(struct lga_ck_dev *l_dev)
{
	sysfs_remove_group(&l_dev->dev->kobj, &lga_ck_sysfs_attr_group);
}

#else

static inline int lga_ck_sysfs_create_group(struct lga_ck_dev *l_dev)
{
	return 0;
}

static inline void lga_ck_sysfs_remove_group(struct lga_ck_dev *l_dev)
{
}
#endif /* CONFIG_SYSFS */

static int lga_ck_parse_dts(struct device_node *np)
{
	int ret;
	struct lga_ck_dev *l_dev = g_lga_check_dev;

	if (!l_dev) {
		hwlog_err("l_dev is null\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "check_mode", &l_dev->lga_ck_mode);
	if (ret) {
		hwlog_err("check_mode dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("check_mode=%d\n", l_dev->lga_ck_mode);

	switch (l_dev->lga_ck_mode) {
	case LGA_CK_MODE_ADC:
		ret = of_property_read_u32(np, "adc_channel_no",
			&l_dev->adc_channel_no);
		if (ret) {
			hwlog_err("adc_channel_no dts read failed\n");
			return -EINVAL;
		}
		hwlog_info("adc_channel_no=%d\n", l_dev->adc_channel_no);

		ret = of_property_read_u32(np, "adc_abnormal_threshold",
			&l_dev->adc_abnormal_thld);
		if (ret) {
			hwlog_err("adc_abnormal_thld dts read failed\n");
			return -EINVAL;
		}
		hwlog_info("adc_abnormal_thld=%d\n", l_dev->adc_abnormal_thld);

		break;

	default:
		hwlog_err("wrong lga check mode index\n");
		return -EINVAL;
	}

	return ret;
}

static int lga_ck_probe(struct platform_device *pdev)
{
	int ret;
	struct lga_ck_dev *l_dev = NULL;
	struct device_node *np = NULL;

	hwlog_info("probe begin\n");

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_lga_check_dev = l_dev;
	l_dev->dev = &pdev->dev;
	np = pdev->dev.of_node;

	ret = lga_ck_parse_dts(np);
	if (ret)
		goto fail_free_mem;

	INIT_DELAYED_WORK(&l_dev->lga_ck_ws, lga_fracture_detection_work);
	schedule_delayed_work(&l_dev->lga_ck_ws,
		msecs_to_jiffies(LGA_CK_DELAY_TIME));

	l_dev->lga_ck_nb.notifier_call = lga_fracture_event_call;
	ret = blocking_notifier_chain_register(&charger_event_notify_head,
		&l_dev->lga_ck_nb);
	if (ret) {
		hwlog_err("register charger_event notifier failed\n");
		goto fail_free_mem;
	}

#ifdef CONFIG_SYSFS
	ret = lga_ck_sysfs_create_group(l_dev);
	if (ret)
		hwlog_err("sysfs group create failed\n");
#endif /* CONFIG_SYSFS */

	platform_set_drvdata(pdev, l_dev);

	hwlog_info("probe end\n");
	return 0;

fail_free_mem:
	kfree(l_dev);
	g_lga_check_dev = NULL;

	return ret;
}

static int lga_ck_remove(struct platform_device *pdev)
{
	struct lga_ck_dev *l_dev = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	if (!l_dev)
		return -ENODEV;

	cancel_delayed_work(&l_dev->lga_ck_ws);
	blocking_notifier_chain_unregister(&charger_event_notify_head,
		&l_dev->lga_ck_nb);
#ifdef CONFIG_SYSFS
	lga_ck_sysfs_remove_group(l_dev);
#endif /* CONFIG_SYSFS */

	kfree(l_dev);
	g_lga_check_dev = NULL;

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id lga_ck_match_table[] = {
	{
		.compatible = "huawei,lga_check",
		.data = NULL,
	},
	{},
};

static struct platform_driver lga_ck_driver = {
	.probe = lga_ck_probe,
	.remove = lga_ck_remove,
	.driver = {
		.name = "huawei,lga_check",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(lga_ck_match_table),
	},
};

static int __init lga_ck_init(void)
{
	return platform_driver_register(&lga_ck_driver);
}

static void __exit lga_ck_exit(void)
{
	platform_driver_unregister(&lga_ck_driver);
}

fs_initcall_sync(lga_ck_init);
module_exit(lga_ck_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("lga board abnormal monitor driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
