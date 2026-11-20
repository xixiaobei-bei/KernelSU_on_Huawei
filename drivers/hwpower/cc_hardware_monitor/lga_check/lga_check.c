/*
 * lga_check.c
 *
 * lga board abnormal monitor driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "lga_check.h"
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG lga_check
HWLOG_REGIST();

static struct lga_ck_dev *g_lga_ck_dev;

static void lga_dmd_report(struct lga_ck_dev *l_dev)
{
	int i;
	char dsm_buff[POWER_DSM_BUF_SIZE_0128] = { 0 };

	for (i = 0; i < l_dev->data.total_type; i++) {
		if (l_dev->data.para[i].dmd_switch == 0)
			continue;

		if (l_dev->data.para[i].status == LGA_CK_FRACTURE_FREE)
			continue;

		switch (l_dev->data.para[i].type) {
		case LGA_CK_MODE_ADC:
			snprintf(dsm_buff, POWER_DSM_BUF_SIZE_0128 - 1,
				"lga abnormal: adc_channel=%d, adc_vol=%d[mV]\n",
				l_dev->data.para[i].adc_no,
				l_dev->data.para[i].adc_vol);
			break;
		case LGA_CK_MODE_GPIO:
			snprintf(dsm_buff, POWER_DSM_BUF_SIZE_0128 - 1,
				"lga abnormal: gpio_no=%d, gpio_val=%d\n",
				l_dev->data.para[i].gpio_no,
				l_dev->data.para[i].gpio_val);
			break;
		case LGA_CK_MODE_IRQ:
			snprintf(dsm_buff, POWER_DSM_BUF_SIZE_0128 - 1,
				"lga abnormal: irq_gpio_no=%d, irq_val=%d\n",
				l_dev->data.para[i].irq_gpio_no,
				l_dev->data.para[i].irq_val);
			break;
		default:
			snprintf(dsm_buff, POWER_DSM_BUF_SIZE_0128 - 1,
				"wrong lga check type\n");
			break;
		}

		power_dsm_report_dmd(POWER_DSM_PMU_OCP,
			l_dev->data.para[i].dmd_no, dsm_buff);
		msleep(LGA_CK_DMD_DELAY_TIME);
	}
}

static int lga_get_adc_vol(u32 adc_channel)
{
	int i;
	int adc_vol;

	for (i = 0; i < LGA_CK_ADC_MAX_RETRYS; i++) {
		adc_vol = power_platform_get_adc_voltage(adc_channel);
		if (adc_vol < 0)
			hwlog_err("adc read channel %d fail, time=%d\n",
				adc_channel, i + 1);
		else
			break;
	}

	hwlog_info("adc_channel=%d, adc_vol=%d\n", adc_channel, adc_vol);
	return adc_vol;
}

static int lga_status_check_adc_vol(struct lga_ck_para_info *info)
{
	info->status = LGA_CK_FRACTURE_FREE;
	info->adc_vol = lga_get_adc_vol(info->adc_no);

	if (info->adc_vol > info->threshold) {
		hwlog_info("adc_vol is over threshold %d\n", info->threshold);
		info->status = LGA_CK_FRACTURE_FOUND;
	}

	return info->status;
}

static int lga_get_gpio_val(int gpio_no)
{
	int gpio_val = gpio_get_value(gpio_no);

	hwlog_info("gpio_no=%d, gpio_val=%d\n", gpio_no, gpio_val);
	return gpio_val;
}

static int lga_status_check_gpio_val(struct lga_ck_para_info *info)
{
	info->status = LGA_CK_FRACTURE_FREE;
	info->gpio_val = lga_get_gpio_val(info->gpio_no);

	if (info->gpio_val == info->threshold) {
		hwlog_info("gpio_val is equal threshold %d\n", info->threshold);
		info->status = LGA_CK_FRACTURE_FOUND;
	}

	return info->status;
}

static int lga_status_check_irq_val(struct lga_ck_para_info *info)
{
	info->status = LGA_CK_FRACTURE_FREE;
	info->irq_val = lga_get_gpio_val(info->irq_gpio_no);

	if (info->irq_val == info->threshold) {
		hwlog_info("irq_val is equal threshold %d\n", info->threshold);
		info->status = LGA_CK_FRACTURE_FOUND;
	}

	return info->status;
}

static int lga_status_check(struct lga_ck_dev *l_dev)
{
	int i;
	int ret = LGA_CK_FRACTURE_FREE;

	for (i = 0; i < l_dev->data.total_type; i++) {
		switch (l_dev->data.para[i].type) {
		case LGA_CK_MODE_ADC:
			ret += lga_status_check_adc_vol(&l_dev->data.para[i]);
			break;
		case LGA_CK_MODE_GPIO:
			ret += lga_status_check_gpio_val(&l_dev->data.para[i]);
			break;
		case LGA_CK_MODE_IRQ:
			ret += lga_status_check_irq_val(&l_dev->data.para[i]);
			break;
		default:
			hwlog_err("wrong lga check type\n");
			break;
		}
	}

	return ret;
}

static void lga_ck_detect_work(struct work_struct *work)
{
	struct lga_ck_dev *l_dev = g_lga_ck_dev;

	if (!l_dev) {
		hwlog_err("l_dev is null\n");
		return;
	}

	if (l_dev->abnormal_time >= LGA_CK_MAX_DMD_REPORT_TIME) {
		hwlog_err("abnormal over %d time\n", l_dev->abnormal_time);
		return;
	}

	hwlog_info("start check\n");

	if (lga_status_check(l_dev)) {
		lga_dmd_report(l_dev);
		l_dev->abnormal_time++;
	}
}

static int lga_ck_event_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct lga_ck_dev *l_dev = g_lga_ck_dev;

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_STOP_CHARGING:
		cancel_delayed_work(&l_dev->detect_work);
		break;
	case POWER_NE_START_CHARGING:
		cancel_delayed_work(&l_dev->detect_work);
		schedule_delayed_work(&l_dev->detect_work,
			msecs_to_jiffies(LGA_CK_WORK_DELAY_TIME));
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static irqreturn_t lga_ck_interrupt(int irq, void *data)
{
	struct lga_ck_dev *l_dev = g_lga_ck_dev;

	if (!l_dev) {
		hwlog_err("l_dev is null\n");
		return IRQ_HANDLED;
	}

	cancel_delayed_work(&l_dev->detect_work);
	schedule_delayed_work(&l_dev->detect_work,
		msecs_to_jiffies(LGA_CK_WORK_DELAY_TIME));
	return IRQ_HANDLED;
}

#ifdef CONFIG_SYSFS
static ssize_t lga_ck_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct power_sysfs_attr_info lga_ck_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(lga_ck, 0440, LGA_CK_SYSFS_STATUS, status),
};

#define LGA_CK_SYSFS_ATTRS_SIZE  ARRAY_SIZE(lga_ck_sysfs_field_tbl)

static struct attribute *lga_ck_sysfs_attrs[LGA_CK_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group lga_ck_sysfs_attr_group = {
	.attrs = lga_ck_sysfs_attrs,
};

static ssize_t lga_ck_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct lga_ck_dev *l_dev = g_lga_ck_dev;

	info = power_sysfs_lookup_attr(attr->attr.name,
		lga_ck_sysfs_field_tbl, LGA_CK_SYSFS_ATTRS_SIZE);
	if (!info || !l_dev)
		return -EINVAL;

	switch (info->name) {
	case LGA_CK_SYSFS_STATUS:
		return scnprintf(buf, PAGE_SIZE, "%d\n",
			lga_status_check(l_dev));
	default:
		return 0;
	}
}

static struct device *lga_ck_sysfs_create_group(void)
{
	power_sysfs_init_attrs(lga_ck_sysfs_attrs,
		lga_ck_sysfs_field_tbl, LGA_CK_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "lga_ck",
		&lga_ck_sysfs_attr_group);
}

static void lga_ck_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &lga_ck_sysfs_attr_group);
}
#else
static inline struct device *lga_ck_sysfs_create_group(void)
{
	return NULL;
}

static inline void lga_ck_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int lga_ck_parse_dts_name_para(struct lga_ck_para_info *info,
	struct device_node *np, const char *string)
{
	if (!info || !np || !string)
		return -EINVAL;

	switch (info->type) {
	case LGA_CK_MODE_ADC:
		return power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
			string, &info->adc_no, 0);
	case LGA_CK_MODE_GPIO:
		return power_gpio_config_input(np,
			string, string, &info->gpio_no);
	case LGA_CK_MODE_IRQ:
		if (power_gpio_config_interrupt(np,
			string, string, &info->irq_gpio_no, &info->irq_int))
			return -EINVAL;

		if (request_irq(info->irq_int, lga_ck_interrupt,
			IRQF_TRIGGER_FALLING, string, info)) {
			hwlog_err("could not request lga_ck_irq\n");
			info->irq_int = -EINVAL;
			gpio_free(info->irq_gpio_no);
			return -EINVAL;
		}
		return 0;
	default:
		return -EINVAL;
	}
}

static int lga_ck_parse_dts_check_para(struct lga_ck_dev *l_dev,
	struct device_node *np, int index, const char *string)
{
	int row = index / LGA_CK_PARA_TOTAL;
	int col = index % LGA_CK_PARA_TOTAL;
	int ret;

	switch (col) {
	case LGA_CK_TYPE:
		ret = kstrtoint(string, 0, &l_dev->data.para[row].type);
		if (ret)
			return -EINVAL;
		break;
	case LGA_CK_NAME:
		ret = lga_ck_parse_dts_name_para(&l_dev->data.para[row], np,
			string);
		if (ret)
			return -EINVAL;
		strncpy(l_dev->data.para[row].name,
			string, LGA_CK_NAME_SIZE - 1);
		break;
	case LGA_CK_ABNORMAL_THLD:
		ret = kstrtoint(string, 0, &l_dev->data.para[row].threshold);
		if (ret)
			return -EINVAL;
		break;
	case LGA_CK_DMD_NO:
		ret = kstrtoint(string, 0, &l_dev->data.para[row].dmd_no);
		if (ret)
			return -EINVAL;
		break;
	case LGA_CK_DMD_SWITCH:
		ret = kstrtoint(string, 0, &l_dev->data.para[row].dmd_switch);
		if (ret)
			return -EINVAL;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int lga_ck_parse_dts(struct device_node *np)
{
	int i;
	int array_len;
	const char *tmp_string = NULL;
	struct lga_ck_dev *l_dev = g_lga_ck_dev;

	if (!l_dev) {
		hwlog_err("l_dev is null\n");
		return -EINVAL;
	}

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"check_para", LGA_CK_PARA_LEVEL, LGA_CK_PARA_TOTAL);
	if (array_len < 0) {
		l_dev->data.total_type = 0;
		return -EINVAL;
	}

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
			"check_para", i, &tmp_string))
			return -EINVAL;

		if (lga_ck_parse_dts_check_para(l_dev, np, i, tmp_string))
			return -EINVAL;
	}

	l_dev->data.total_type = array_len / LGA_CK_PARA_TOTAL;
	return 0;
}

static void lga_ck_free_res(struct lga_ck_dev *l_dev)
{
	int i;

	for (i = 0; i < l_dev->data.total_type; i++) {
		switch (l_dev->data.para[i].type) {
		case LGA_CK_MODE_GPIO:
			if (l_dev->data.para[i].gpio_no > 0)
				gpio_free(l_dev->data.para[i].gpio_no);
			break;
		case LGA_CK_MODE_IRQ:
			if (l_dev->data.para[i].irq_int > 0)
				free_irq(l_dev->data.para[i].irq_int, &l_dev->data.para[i]);
			if (l_dev->data.para[i].irq_gpio_no > 0)
				gpio_free(l_dev->data.para[i].irq_gpio_no);
			break;
		default:
			break;
		}
	}
}

static int lga_ck_probe(struct platform_device *pdev)
{
	int ret;
	struct lga_ck_dev *l_dev = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_lga_ck_dev = l_dev;
	np = pdev->dev.of_node;

	ret = lga_ck_parse_dts(np);
	if (ret)
		goto fail_free_mem;

	INIT_DELAYED_WORK(&l_dev->detect_work, lga_ck_detect_work);
	schedule_delayed_work(&l_dev->detect_work,
		msecs_to_jiffies(LGA_CK_WORK_DELAY_TIME));
	l_dev->nb.notifier_call = lga_ck_event_call;
	ret = power_event_bnc_register(POWER_BNT_CHARGING, &l_dev->nb);
	if (ret)
		goto fail_free_res;

	l_dev->dev = lga_ck_sysfs_create_group();
	platform_set_drvdata(pdev, l_dev);
	return 0;

fail_free_res:
	lga_ck_free_res(l_dev);
fail_free_mem:
	kfree(l_dev);
	g_lga_ck_dev = NULL;
	return ret;
}

static int lga_ck_remove(struct platform_device *pdev)
{
	struct lga_ck_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -ENODEV;

	cancel_delayed_work(&l_dev->detect_work);
	power_event_bnc_unregister(POWER_BNT_CHARGING, &l_dev->nb);
	lga_ck_sysfs_remove_group(l_dev->dev);
	platform_set_drvdata(pdev, NULL);
	lga_ck_free_res(l_dev);
	kfree(l_dev);
	g_lga_ck_dev = NULL;

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
