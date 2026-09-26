/*
 * btb_check.c
 *
 * btb abnormal monitor driver
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

#include <chipset_common/hwpower/hardware_monitor/btb_check.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_manager.h>

#define HWLOG_TAG btb_check
HWLOG_REGIST();

static struct btb_ck_dev *g_btb_ck_dev;

static struct btb_ck_dev *btb_ck_get_dev(void)
{
	if (!g_btb_ck_dev) {
		hwlog_err("g_btb_ck_dev is null\n");
		return NULL;
	}

	return g_btb_ck_dev;
}

unsigned int btb_ck_get_result(int type)
{
	struct btb_ck_dev *l_dev = btb_ck_get_dev();

	if (!l_dev)
		return 0;

	switch (type) {
	case BTB_VOLT_CHECK:
		return l_dev->volt_ck_result;
	case BTB_TEMP_CHECK:
		return l_dev->temp_ck_result;
	default:
		return 0;
	}
}

static void btb_ck_bat_volt(struct btb_ck_para *data, unsigned int *result)
{
	int main_btb;
	int aux_btb;
	int btb_diff;

	if (data->enable == 0)
		return;

	main_btb = dcm_get_ic_vbtb(SC_MODE, CHARGE_IC_MAIN);
	if ((main_btb < 0) || (main_btb > data->max_th) ||
		(main_btb < data->min_th))
		*result |= MAIN_BAT_BTB_ERR;
	hwlog_info("main btb voltage=%dmV\n", main_btb);
	if (data->is_multi_btb) {
		aux_btb = dcm_get_ic_vbtb(SC_MODE, CHARGE_IC_AUX);
		btb_diff = main_btb - aux_btb;
		btb_diff = btb_diff > 0 ? btb_diff : -btb_diff;
		if ((aux_btb < 0) || (aux_btb > data->max_th) ||
			(aux_btb < data->min_th))
			*result |= AUX_BAT_BTB_ERR;
		if (btb_diff > data->diff_th)
			*result |= BTB_BAT_DIFF_ERR;
		hwlog_info("aux btb voltage=%dmV, btb_diff=%dmV\n",
			aux_btb, btb_diff);
	}
}

static void btb_ck_bat_volt_by_multi_times(struct btb_ck_para *data,
	unsigned int *result)
{
	int i;

	for (i = 0; i < data->times; i++) {
		btb_ck_bat_volt(data, result);
		if (*result)
			return;
		/* delay 30ms for adc sampling interval */
		usleep_range(30000, 31000);
	}
}

static void btb_ck_bat_temp(struct btb_ck_para *data, unsigned int *result)
{
	int main_btb = 0;
	int aux_btb = 0;
	int btb_diff;
	int i;
	int ret;

	for (i = 0; i < data->times; i++) {
		ret = bat_temp_get_rt_temperature(BTB_TEMP_0, &main_btb);
		if ((ret < 0) || (main_btb > data->max_th) ||
			(main_btb < data->min_th))
			*result |= MAIN_BAT_BTB_ERR;
		hwlog_info("main btb temp=%d\n", main_btb);
		if (data->is_multi_btb) {
			ret = bat_temp_get_rt_temperature(BTB_TEMP_1, &aux_btb);
			if ((ret < 0) || (aux_btb > data->max_th) ||
				(aux_btb < data->min_th))
				*result |= AUX_BAT_BTB_ERR;
			btb_diff = main_btb - aux_btb;
			btb_diff = btb_diff > 0 ? btb_diff : -btb_diff;
			if (btb_diff > data->diff_th)
				*result |= BTB_BAT_DIFF_ERR;
			hwlog_info("aux btb temp=%d, btb_diff=%d\n",
				aux_btb, btb_diff);
		}
		if (*result)
			return;
		/* delay 30ms for adc sampling interval */
		usleep_range(30000, 31000);
	}
}

void btb_ck_get_result_now(int type, unsigned int *result)
{
	struct btb_ck_dev *l_dev = btb_ck_get_dev();

	if (!l_dev || !result)
		return;

	switch (type) {
	case BTB_VOLT_CHECK:
		btb_ck_bat_volt(&l_dev->volt_ck_para, result);
		break;
	case BTB_TEMP_CHECK:
		btb_ck_bat_temp(&l_dev->temp_ck_para, result);
		break;
	default:
		break;
	}
}

static void btb_ck_battery(int type, struct btb_ck_para *data,
	unsigned int *result)
{
	if (!data || !result || (data->enable == 0))
		return;

	*result = 0;
	switch (type) {
	case BTB_VOLT_CHECK:
		btb_ck_bat_volt_by_multi_times(data, result);
		break;
	case BTB_TEMP_CHECK:
		btb_ck_bat_temp(data, result);
		break;
	default:
		break;
	}

	hwlog_info("btb check type is %d, result=%u\n", type, *result);
}

static void btb_ck_volt_work(struct work_struct *work)
{
	struct btb_ck_dev *l_dev = btb_ck_get_dev();

	if (!l_dev)
		return;

	btb_ck_battery(BTB_VOLT_CHECK, &l_dev->volt_ck_para,
		&l_dev->volt_ck_result);
}

static void btb_ck_temp_work(struct work_struct *work)
{
	struct btb_ck_dev *l_dev = btb_ck_get_dev();

	if (!l_dev)
		return;

	btb_ck_battery(BTB_TEMP_CHECK, &l_dev->temp_ck_para,
		&l_dev->temp_ck_result);
}

static void btb_ck_start(struct btb_ck_dev *l_dev)
{
	cancel_delayed_work(&l_dev->volt_ck_work);
	schedule_delayed_work(&l_dev->volt_ck_work,
		msecs_to_jiffies(BTB_CK_WORK_DELAY_TIME));
	cancel_delayed_work(&l_dev->temp_ck_work);
	schedule_delayed_work(&l_dev->temp_ck_work,
		msecs_to_jiffies(BTB_CK_WORK_DELAY_TIME));
}

static void btb_ck_stop(struct btb_ck_dev *l_dev)
{
	cancel_delayed_work(&l_dev->volt_ck_work);
	cancel_delayed_work(&l_dev->temp_ck_work);
}

static int btb_ck_event_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct btb_ck_dev *l_dev = btb_ck_get_dev();

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_STOP_CHARGING:
		btb_ck_stop(l_dev);
		break;
	case POWER_NE_START_CHARGING:
		btb_ck_start(l_dev);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

#ifdef CONFIG_SYSFS
static ssize_t btb_ck_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct power_sysfs_attr_info btb_ck_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(btb_ck, 0444, BTB_CK_SYSFS_VOLT_RESULT, volt_result),
	power_sysfs_attr_ro(btb_ck, 0444, BTB_CK_SYSFS_TEMP_RESULT, temp_result),
};

#define BTB_CK_SYSFS_ATTRS_SIZE  ARRAY_SIZE(btb_ck_sysfs_field_tbl)

static struct attribute *btb_ck_sysfs_attrs[BTB_CK_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group btb_ck_sysfs_attr_group = {
	.attrs = btb_ck_sysfs_attrs,
};

static ssize_t btb_ck_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct btb_ck_dev *l_dev = btb_ck_get_dev();
	struct power_sysfs_attr_info *info = NULL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		btb_ck_sysfs_field_tbl, BTB_CK_SYSFS_ATTRS_SIZE);
	if (!info || !l_dev)
		return -EINVAL;

	switch (info->name) {
	case BTB_CK_SYSFS_VOLT_RESULT:
		return scnprintf(buf, PAGE_SIZE, "%d\n",
			btb_ck_get_result(BTB_VOLT_CHECK));
	case BTB_CK_SYSFS_TEMP_RESULT:
		return scnprintf(buf, PAGE_SIZE, "%d\n",
			btb_ck_get_result(BTB_TEMP_CHECK));
	default:
		return 0;
	}
}

static struct device *btb_ck_sysfs_create_group(void)
{
	power_sysfs_init_attrs(btb_ck_sysfs_attrs,
		btb_ck_sysfs_field_tbl, BTB_CK_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "btb_ck",
		&btb_ck_sysfs_attr_group);
}

static void btb_ck_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &btb_ck_sysfs_attr_group);
}
#else
static inline struct device *btb_ck_sysfs_create_group(void)
{
	return NULL;
}

static inline void btb_ck_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void btb_ck_parse_check_para(struct device_node *np,
	struct btb_ck_para *data, const char *name)
{
	int len;
	int idata[BTB_CK_PARA_TOTAL] = { 0 };

	/* 1:only one line parameters */
	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		name, idata, 1, BTB_CK_PARA_TOTAL);
	if (len < 0)
		return;

	data->enable = idata[BTB_CK_ENABLE];
	data->is_multi_btb = idata[BTB_CK_IS_MULTI_BTB];
	data->min_th = idata[BTB_CK_MIN_TH];
	data->max_th = idata[BTB_CK_MAX_TH];
	data->diff_th = idata[BTB_CK_DIFF_TH];
	data->times = idata[BTB_CK_TIMES];
	data->dmd_no = idata[BTB_CK_DMD_NO];
	hwlog_info("[%s]enable=%d is_multi_btb=%d min_th=%d max_th=%d diff_th=%d times=%d dmd_no=%d\n",
		name, data->enable, data->is_multi_btb, data->min_th,
		data->max_th, data->diff_th, data->times, data->dmd_no);
}

static void btb_ck_parse_dts(struct device_node *np, struct btb_ck_dev *l_dev)
{
	btb_ck_parse_check_para(np, &l_dev->volt_ck_para, "vol_check_para");
	btb_ck_parse_check_para(np, &l_dev->temp_ck_para, "temp_check_para");
}

static int btb_ck_probe(struct platform_device *pdev)
{
	struct btb_ck_dev *l_dev = NULL;
	struct device_node *np = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_btb_ck_dev = l_dev;
	np = pdev->dev.of_node;

	btb_ck_parse_dts(np, l_dev);
	INIT_DELAYED_WORK(&l_dev->volt_ck_work, btb_ck_volt_work);
	INIT_DELAYED_WORK(&l_dev->temp_ck_work, btb_ck_temp_work);
	l_dev->nb.notifier_call = btb_ck_event_call;
	ret = power_event_bnc_register(POWER_BNT_CHARGING, &l_dev->nb);
	if (ret)
		goto fail_free_mem;

	l_dev->dev = btb_ck_sysfs_create_group();
	platform_set_drvdata(pdev, l_dev);
	return 0;

fail_free_mem:
	kfree(l_dev);
	g_btb_ck_dev = NULL;
	return ret;
}

static int btb_ck_remove(struct platform_device *pdev)
{
	struct btb_ck_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -ENODEV;

	cancel_delayed_work(&l_dev->volt_ck_work);
	cancel_delayed_work(&l_dev->temp_ck_work);
	power_event_bnc_unregister(POWER_BNT_CHARGING, &l_dev->nb);
	btb_ck_sysfs_remove_group(l_dev->dev);
	platform_set_drvdata(pdev, NULL);
	kfree(l_dev);
	g_btb_ck_dev = NULL;

	return 0;
}

static const struct of_device_id btb_ck_match_table[] = {
	{
		.compatible = "huawei,btb_check",
		.data = NULL,
	},
	{},
};

static struct platform_driver btb_ck_driver = {
	.probe = btb_ck_probe,
	.remove = btb_ck_remove,
	.driver = {
		.name = "huawei,btb_check",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(btb_ck_match_table),
	},
};

static int __init btb_ck_init(void)
{
	return platform_driver_register(&btb_ck_driver);
}

static void __exit btb_ck_exit(void)
{
	platform_driver_unregister(&btb_ck_driver);
}

device_initcall_sync(btb_ck_init);
module_exit(btb_ck_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("btb abnormal monitor driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
