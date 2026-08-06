/*
 * huawei_mixed_battery.c
 *
 * power supply interface for merge multi power supply
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

#include <linux/module.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/power_supply.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG mixed_batt
HWLOG_REGIST();

#define BATT_PSY_SUB_PSY_MAX 5

struct mixed_batt {
	struct device *dev;
	struct power_supply *batt;
	struct notifier_block psy_nb;
	int psy_cnt;
	const char *sub_psy[BATT_PSY_SUB_PSY_MAX];
};

static int battery_prop_is_writeable(const char *name,
	enum power_supply_property psp)
{
	struct power_supply *psy = NULL;

	psy = power_supply_get_by_name(name);
	if (!psy)
		return 0;
	if (!psy->desc->property_is_writeable)
		return 0;
	return psy->desc->property_is_writeable(psy, psp);
}

static int mixed_batt_get_property(struct power_supply *psy,
	enum power_supply_property psp,
	union power_supply_propval *val)
{
	int i;
	int ret = -ENODEV;
	struct mixed_batt *di = dev_get_drvdata(psy->dev.parent);

	if (!val)
		return -EINVAL;

	for (i = 0; i < di->psy_cnt; i++) {
		ret = power_supply_get_property_value(di->sub_psy[i],
			psp, val);
		if (!ret)
			return ret;
	}

	hwlog_info("prop %d %s err\n", psp, __func__);
	return ret;
}

static int mixed_batt_set_property(struct power_supply *psy,
	enum power_supply_property psp,
	const union power_supply_propval *val)
{
	int i;
	int ret = -ENODEV;
	struct mixed_batt *di = dev_get_drvdata(psy->dev.parent);

	if (!val)
		return -EINVAL;

	for (i = 0; i < di->psy_cnt; i++) {
		ret = power_supply_set_property_value(di->sub_psy[i],
			psp, val);
		if (!ret) {
			hwlog_info("prop to %s\n", di->sub_psy[i]);
			return ret;
		}
	}

	hwlog_err("prop %d %s err\n", psp, __func__);
	return ret;
}

static int mixed_batt_property_is_writeable(struct power_supply *psy,
	enum power_supply_property psp)
{
	int i, ret;
	struct mixed_batt *di = dev_get_drvdata(psy->dev.parent);

	for (i = 0; i < di->psy_cnt; i++) {
		ret = battery_prop_is_writeable(di->sub_psy[i], psp);
		if (ret)
			return ret;
	}
	return 0;
}

static struct power_supply_desc mixed_batt_desc = {
	.name = "battery",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.get_property = mixed_batt_get_property,
	.set_property = mixed_batt_set_property,
	.property_is_writeable = mixed_batt_property_is_writeable,
};

static int batt_props_add(enum power_supply_property *dst_prop, int dts_len,
	int wp, struct power_supply *psy)
{
	int j, k;

	for (j = 0; j < psy->desc->num_properties; j++) {
		for (k = 0; k < wp; k++) {
			if (dst_prop[k] == psy->desc->properties[j])
				break;
		}
		if (k < wp)
			continue;
		if (wp >= dts_len)
			return -ENOMEM;

		dst_prop[wp++] = psy->desc->properties[j];
	}
	return wp;
}

static int mixed_batt_creat_desc(struct mixed_batt *di)
{
	int i;
	struct power_supply *psy[BATT_PSY_SUB_PSY_MAX] = { 0 };
	enum power_supply_property *prop_ptr = NULL;
	int ret = -ENODEV;
	int prop_cnt = 0;
	int widx = 0;

	for (i = 0; i < di->psy_cnt; i++) {
		psy[i] = power_supply_get_by_name(di->sub_psy[i]);
		if (!psy[i]) {
			hwlog_err("%s not exist\n", di->sub_psy[i]);
			ret = -ENODEV;
			goto out;
		}
		if (!psy[i]->desc->properties ||
			!psy[i]->desc->num_properties) {
			ret = -ENODEV;
			goto out;
		}
		prop_cnt += psy[i]->desc->num_properties;
	}
	prop_ptr = devm_kzalloc(di->dev, sizeof(*prop_ptr) * prop_cnt,
		GFP_KERNEL);
	if (!prop_ptr) {
		ret = -ENOMEM;
		goto out;
	}
	for (i = 0; i < di->psy_cnt; i++) {
		widx = batt_props_add(prop_ptr, prop_cnt, widx, psy[i]);
		if (widx < 0) {
			ret = widx;
			goto out;
		}
	}
	mixed_batt_desc.properties = prop_ptr;
	mixed_batt_desc.num_properties = widx;
	ret = 0;
out:
	for (i = 0; psy[i] != NULL; i++)
		power_supply_put(psy[i]);
	return ret;
}

static int mixed_batt_notifier_call(struct notifier_block *nb,
	unsigned long action, void *data)
{
	int i;
	struct power_supply *psy = data;
	struct mixed_batt *di = container_of(nb, struct mixed_batt, psy_nb);

	for (i = 0; i < di->psy_cnt; i++) {
		if (strcmp(psy->desc->name, di->sub_psy[i]))
			continue;
		hwlog_info("updata from %s\n", di->sub_psy[i]);
		power_supply_changed(di->batt);
		break;
	}

	return NOTIFY_OK;
}

static int mixed_batt_parse_dts(struct mixed_batt *di)
{
	struct device_node *np = di->dev->of_node;
	int len = of_property_count_strings(np, "psy-names");
	int i;
	int ret;

	if (len <= 0 || len > BATT_PSY_SUB_PSY_MAX) {
		ret = -EINVAL;
		goto out;
	}
	for (i = 0; i < len; i++) {
		ret = power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "psy-names", i, &di->sub_psy[i]);
		if (ret) {
			ret = -EINVAL;
			goto out;
		}
	}
	di->psy_cnt = len;
	return 0;
out:
	hwlog_err("psy-names dts err\n");
	return ret;
}

static int mixed_batt_probe(struct platform_device *pdev)
{
	struct mixed_batt *di = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;
	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;
	di->dev = &pdev->dev;
	platform_set_drvdata(pdev, di);
	ret = mixed_batt_parse_dts(di);
	if (ret)
		goto err_out;
	ret = mixed_batt_creat_desc(di);
	if (ret)
		goto err_out;
	di->batt = power_supply_register(&pdev->dev, &mixed_batt_desc, NULL);
	if (IS_ERR(di->batt)) {
		ret = -ENOMEM;
		goto err_out;
	}

	di->psy_nb.notifier_call = mixed_batt_notifier_call;
	ret = power_supply_reg_notifier(&di->psy_nb);
	if (ret)
		goto err_out_1;

	hwlog_info("%s ok\n", __func__);
	return 0;

err_out_1:
	power_supply_unregister(di->batt);
err_out:
	hwlog_err("%s err\n", __func__);
	return ret;
}

static int mixed_batt_remove(struct platform_device *pdev)
{
	struct mixed_batt *di = platform_get_drvdata(pdev);

	power_supply_unreg_notifier(&di->psy_nb);
	power_supply_unregister(di->batt);
	return 0;
}

static const struct of_device_id mixed_batt_match_table[] = {
	{
		.compatible = "huawei,mixed_batt",
		.data = NULL,
	},
	{},
};

static struct platform_driver mixed_batt_driver = {
	.probe = mixed_batt_probe,
	.remove = mixed_batt_remove,
	.driver = {
		.name = "huawei,mixed_batt",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(mixed_batt_match_table),
	},
};

static int __init mixed_batt_init(void)
{
	return platform_driver_register(&mixed_batt_driver);
}

static void __exit mixed_batt_exit(void)
{
	platform_driver_unregister(&mixed_batt_driver);
}

late_initcall_sync(mixed_batt_init);
module_exit(mixed_batt_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery power supply");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
