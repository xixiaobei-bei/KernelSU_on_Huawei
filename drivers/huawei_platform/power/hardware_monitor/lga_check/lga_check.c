/*
 * lga_check.c
 *
 * lga board abnormal monitor driver
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/power_dsm.h>
#include <huawei_platform/power/huawei_charger.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

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
		default:
			snprintf(dsm_buff, POWER_DSM_BUF_SIZE_0128 - 1,
				"wrong lga check type\n");
			break;
		}

		power_dsm_dmd_report(POWER_DSM_PMU_OCP,
			DSM_LGA_CHECK_ERROR_NO + l_dev->data.para[i].dmd_offset,
			dsm_buff);
		msleep(LGA_CK_DMD_DELAY_TIME);
	}
}

static int lga_get_adc_vol(u32 adc_channel)
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
		default:
			hwlog_err("wrong lga check type\n");
			break;
		}
	}

	return ret;
}

static void lga_fracture_detection_work(struct work_struct *work)
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

static int lga_fracture_event_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct lga_ck_dev *l_dev = g_lga_ck_dev;

	if (!l_dev) {
		hwlog_err("l_dev is null\n");
		return NOTIFY_OK;
	}

	if (event == CHARGER_START_CHARGING_EVENT) {
		cancel_delayed_work(&l_dev->lga_ck_ws);
		schedule_delayed_work(&l_dev->lga_ck_ws,
			msecs_to_jiffies(LGA_CK_WORK_DELAY_TIME));
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
	LGA_CK_SYSFS_FIELD(_name, n, 0440, NULL)

struct lga_ck_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static ssize_t lga_ck_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct lga_ck_sysfs_field_info lga_ck_sysfs_field_tbl[] = {
	LGA_CK_SYSFS_FIELD_RO(status, STATUS),
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
	struct lga_ck_dev *l_dev = g_lga_ck_dev;
	int len;

	info = lga_ck_sysfs_field_lookup(attr->attr.name);
	if (!info || !l_dev) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case LGA_CK_SYSFS_STATUS:
		len = scnprintf(buf, PAGE_SIZE, "%d\n",
			lga_status_check(l_dev));
		break;
	default:
		hwlog_err("invalid sysfs_name\n");
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

static int lga_ck_parse_dts_check_para(struct lga_ck_dev *l_dev,
	struct device_node *np, int index, const char *string)
{
	int row = index / LGA_CK_PARA_TOTAL;
	int col = index % LGA_CK_PARA_TOTAL;
	int ret;
	int tmp_type;
	int tmp_gpio;

	switch (col) {
	case LGA_CK_TYPE:
		ret = kstrtoint(string, 0, &l_dev->data.para[row].type);
		if (ret)
			return -EINVAL;
		break;
	case LGA_CK_NAME:
		tmp_type = l_dev->data.para[row].type;
		if (tmp_type == LGA_CK_MODE_ADC) {
			ret = power_dts_read_u32(np, string,
				&l_dev->data.para[row].adc_no, 0);
			if (ret)
				return -EINVAL;
		} else if (tmp_type == LGA_CK_MODE_GPIO) {
			tmp_gpio = of_get_named_gpio(np, string, 0);
			if (!gpio_is_valid(tmp_gpio)) {
				hwlog_err("gpio is not valid\n");
				return -EINVAL;
			}
			if (gpio_request(tmp_gpio, string)) {
				hwlog_err("gpio request fail\n");
				return -EINVAL;
			}
			if (gpio_direction_input(tmp_gpio)) {
				hwlog_err("gpio set input fail\n");
				gpio_free(tmp_gpio);
				return -EINVAL;
			}

			l_dev->data.para[row].gpio_no = tmp_gpio;
		} else {
			hwlog_err("type invalid\n");
			return -EINVAL;
		}

		strncpy(l_dev->data.para[row].name,
			string, LGA_CK_NAME_SIZE - 1);
		break;
	case LGA_CK_ABNORMAL_THLD:
		ret = kstrtoint(string, 0, &l_dev->data.para[row].threshold);
		if (ret)
			return -EINVAL;
		break;
	case LGA_CK_DMD_OFFSET:
		ret = kstrtoint(string, 0, &l_dev->data.para[row].dmd_offset);
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
	int ret;
	int array_len;
	const char *tmp_string = NULL;
	struct lga_ck_dev *l_dev = g_lga_ck_dev;

	if (!l_dev) {
		hwlog_err("l_dev is null\n");
		return -EINVAL;
	}

	array_len = of_property_count_strings(np, "check_para");
	hwlog_info("array_len=%d\n", array_len);

	if ((array_len <= 0) ||
		(array_len % LGA_CK_PARA_TOTAL != 0) ||
		(array_len > LGA_CK_PARA_TOTAL * LGA_CK_PARA_LEVEL)) {
		l_dev->data.total_type = 0;
		hwlog_err("check_para invalid\n");
		return -EINVAL;
	}

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np, "check_para",
			i, &tmp_string);
		if (ret) {
			hwlog_err("check_para dts read failed\n");
			return -EINVAL;
		}

		if (lga_ck_parse_dts_check_para(l_dev, np, i, tmp_string))
			return -EINVAL;
	}

	l_dev->data.total_type = array_len / LGA_CK_PARA_TOTAL;

	for (i = 0; i < l_dev->data.total_type; i++) {
		hwlog_info("para type:%d name:%s,%d,%d %d %d %d\n",
			l_dev->data.para[i].type,
			l_dev->data.para[i].name,
			l_dev->data.para[i].gpio_no,
			l_dev->data.para[i].adc_no,
			l_dev->data.para[i].threshold,
			l_dev->data.para[i].dmd_offset,
			l_dev->data.para[i].dmd_switch);
	}

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

	hwlog_info("probe begin\n");

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_lga_ck_dev = l_dev;
	l_dev->dev = &pdev->dev;
	np = pdev->dev.of_node;

	ret = lga_ck_parse_dts(np);
	if (ret)
		goto fail_free_mem;

	INIT_DELAYED_WORK(&l_dev->lga_ck_ws, lga_fracture_detection_work);
	schedule_delayed_work(&l_dev->lga_ck_ws,
		msecs_to_jiffies(LGA_CK_WORK_DELAY_TIME));

	l_dev->lga_ck_nb.notifier_call = lga_fracture_event_call;
	ret = blocking_notifier_chain_register(&charger_event_notify_head,
		&l_dev->lga_ck_nb);
	if (ret) {
		hwlog_err("register charger_event notifier failed\n");
		goto fail_free_res;
	}

#ifdef CONFIG_SYSFS
	ret = lga_ck_sysfs_create_group(l_dev);
	if (ret)
		hwlog_err("sysfs group create failed\n");
#endif /* CONFIG_SYSFS */

	platform_set_drvdata(pdev, l_dev);

	hwlog_info("probe end\n");
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

	hwlog_info("remove begin\n");

	if (!l_dev)
		return -ENODEV;

	cancel_delayed_work(&l_dev->lga_ck_ws);
	blocking_notifier_chain_unregister(&charger_event_notify_head,
		&l_dev->lga_ck_nb);
#ifdef CONFIG_SYSFS
	lga_ck_sysfs_remove_group(l_dev);
#endif /* CONFIG_SYSFS */

	lga_ck_free_res(l_dev);

	kfree(l_dev);
	g_lga_ck_dev = NULL;

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
