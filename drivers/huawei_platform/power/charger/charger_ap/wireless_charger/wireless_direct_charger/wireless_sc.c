/*
 * wireless_sc.c
 *
 * wireless sc driver
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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
#include <huawei_platform/log/hw_log.h>
#include <linux/power/hisi/hisi_bci_battery.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/power_interface.h>
#include <huawei_platform/power/wireless_power_supply.h>
#include <huawei_platform/power/wireless_charger.h>
#include <huawei_platform/power/wireless_dc_error_handle.h>
#include <huawei_platform/power/wireless_direct_charger.h>
#include <huawei_platform/power/wired_channel_switch.h>
#include <huawei_platform/power/boost_5v.h>

#define HWLOG_TAG wireless_sc
HWLOG_REGIST();

static struct wldc_dev_info *g_wlsc_di;
static struct loadswitch_ops *g_wireless_ls_ops;
static struct batinfo_ops *g_wireless_bi_ops;
static struct wldc_volt_para_group g_wlsc_volt_para[WLDC_VOLT_GROUP_MAX];

static int wlsc_set_enable_charger(unsigned int val)
{
	struct wldc_dev_info *di = g_wlsc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -WLC_ERR_PARA_NULL;
	}

	/* must be 0 or 1, 0: disable, 1: enable */
	if ((val < 0) || (val > 1))
		return -WLC_ERR_PARA_WRONG;

	di->sysfs_data.enable_charger = val;
	hwlog_info("set enable_charger = %d\n", di->sysfs_data.enable_charger);
	return 0;
}

static int wlsc_get_enable_charger(unsigned int *val)
{
	struct wldc_dev_info *di = g_wlsc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -WLC_ERR_PARA_NULL;
	}

	*val = di->sysfs_data.enable_charger;
	return 0;
}

static int wlsc_set_ichg_ratio(unsigned int val)
{
	struct wldc_dev_info *di = g_wlsc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	di->ichg_ratio = val;
	hwlog_info("set ichg_ratio=%d\n", val);
	return 0;
}

static int wlsc_set_vterm_dec(unsigned int val)
{
	struct wldc_dev_info *di = g_wlsc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	di->vterm_dec = val;
	hwlog_info("set vterm_dec=%d\n", val);
	return 0;
}

static int wlsc_set_pwr_limit(unsigned int val)
{
	struct wldc_dev_info *di = g_wlsc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -WLC_ERR_PARA_NULL;
	}

	di->sysfs_data.pwr_limit = val;
	return 0;
}

static int wlsc_get_pwr_limit(unsigned int *val)
{
	struct wldc_dev_info *di = g_wlsc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -WLC_ERR_PARA_NULL;
	}

	*val = di->sysfs_data.pwr_limit;
	return 0;
}

static struct power_if_ops wlsc_if_ops = {
	.set_enable_charger = wlsc_set_enable_charger,
	.get_enable_charger = wlsc_get_enable_charger,
	.set_ichg_ratio = wlsc_set_ichg_ratio,
	.set_vterm_dec = wlsc_set_vterm_dec,
	.set_pwr_limit = wlsc_set_pwr_limit,
	.get_pwr_limit = wlsc_get_pwr_limit,
	.type_name = "wl_sc",
};

int wireless_sc_ops_register(struct loadswitch_ops *ops)
{
	if (ops) {
		g_wireless_ls_ops = ops;
		return 0;
	}

	hwlog_err("%s: fail\n", __func__);
	return -EPERM;
}

int wireless_sc_batinfo_ops_register(struct batinfo_ops *ops)
{
	if (ops) {
		g_wireless_bi_ops = ops;
		return 0;
	}

	hwlog_err("%s: fail\n", __func__);
	return -EPERM;
}

void wireless_sc_get_di(struct wldc_dev_info **di)
{
	if (!g_wlsc_di || !di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	*di = g_wlsc_di;
}

int wireless_sc_charge_check(void)
{
	int ret;
	struct wldc_dev_info *di = g_wlsc_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = wldc_set_rx_init_vout(di);
	if (ret) {
		di->stop_flag_error = 1;
		hwlog_err("%s: set initial vout fail\n", __func__);
		wldc_stop_charging(di);
		return ret;
	}
	ret = wldc_chip_init(di);
	if (ret) {
		di->stop_flag_error = 1;
		hwlog_err("%s: sc_chip_init fail\n", __func__);
		wldc_stop_charging(di);
		return ret;
	}
	wldc_extra_power_supply(WLPS_CTRL_ON);
	wireless_charge_chip_init(WILREESS_SC_CHIP_INIT);
	ret = wldc_cut_off_normal_charge_path();
	if (ret) {
		di->stop_flag_error = 1;
		hwlog_err("%s: cutt_off_normal_charge fail\n", __func__);
		wldc_fill_eh_buf(di->wldc_err_dsm_buff,
			sizeof(di->wldc_err_dsm_buff),
			WLDC_CUT_OFF_NORMAL_PATH, NULL);
		wldc_stop_charging(di);
		return ret;
	}
	wlc_ignore_vbus_only_event(true);
	ret = wldc_turn_on_direct_charge_channel();
	if (ret) {
		di->stop_flag_error = 1;
		hwlog_err("%s: turn on dc path fail\n", __func__);
		wldc_fill_eh_buf(di->wldc_err_dsm_buff,
			sizeof(di->wldc_err_dsm_buff),
			WLDC_TURN_ON_DC_PATH, NULL);
		wldc_stop_charging(di);
		return ret;
	}
	ret = wldc_security_check(di);
	if (ret) {
		di->stop_flag_error = 1;
		hwlog_err("%s: security_check fail\n", __func__);
		wldc_stop_charging(di);
		return ret;
	}
	return 0;
}

/*
 * There are a numerous options that are configurable on the wireless receiver
 * that go well beyond what the power_supply properties provide access to.
 * Provide sysfs access to them so they can be examined and possibly modified
 * on the fly.
 */
#ifdef CONFIG_SYSFS
static ssize_t wlsc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t wlsc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info wlsc_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(wlsc, 0640, WLDC_SYSFS_IIN_THERMAL, iin_thermal),
};

static struct attribute *wlsc_sysfs_attrs[ARRAY_SIZE(wlsc_sysfs_field_tbl) + 1];
static const struct attribute_group wlsc_sysfs_attr_group = {
	.attrs = wlsc_sysfs_attrs,
};

static void wlsc_sysfs_init_attrs(void)
{
	int i;
	int limit = ARRAY_SIZE(wlsc_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		wlsc_sysfs_attrs[i] = &wlsc_sysfs_field_tbl[i].attr.attr;

	wlsc_sysfs_attrs[limit] = NULL;
}

static struct power_sysfs_attr_info *wlsc_sysfs_field_lookup(const char *name)
{
	int i;
	int limit = ARRAY_SIZE(wlsc_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name,
			wlsc_sysfs_field_tbl[i].attr.attr.name, strlen(name)))
			break;
	}

	if (i >= limit)
		return NULL;

	return &wlsc_sysfs_field_tbl[i];
}

static void wlsc_sysfs_create_group(struct device *dev)
{
	wlsc_sysfs_init_attrs();
	power_sysfs_create_link_group("hw_power", "charger", "wireless_sc",
		dev, &wlsc_sysfs_attr_group);
}
#else
static inline void wlsc_sysfs_create_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static ssize_t wlsc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct wldc_dev_info *di = g_wlsc_di;

	info = wlsc_sysfs_field_lookup(attr->attr.name);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case WLDC_SYSFS_IIN_THERMAL:
		return snprintf(buf, PAGE_SIZE, "%d\n",
			di->sysfs_data.cp_iin_thermal);
	default:
		break;
	}
	return 0;
}

static ssize_t wlsc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct wldc_dev_info *di = g_wlsc_di;
	long val = 0;

	info = wlsc_sysfs_field_lookup(attr->attr.name);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case WLDC_SYSFS_IIN_THERMAL:
		if ((kstrtol(buf, WLC_DECIMAL, &val) < 0) ||
			(val < 0) || (val > WLDC_DFT_MAX_CP_IOUT))
			return -EINVAL;
		hwlog_info("[%s] set iin_thermal = %ld\n", __func__, val);
		if (!val)
			di->sysfs_data.cp_iin_thermal = WLDC_DFT_MAX_CP_IOUT;
		else
			di->sysfs_data.cp_iin_thermal = val;
		break;
	default:
		break;
	}
	return count;
}

static int wlsc_probe(struct platform_device *pdev)
{
	struct wldc_dev_info *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = pdev->dev.of_node;
	g_wlsc_di = di;
	di->ls_ops = g_wireless_ls_ops;
	di->bi_ops = g_wireless_bi_ops;
	di->orig_volt_para_p = g_wlsc_volt_para;
	platform_set_drvdata(pdev, di);

	wldc_parse_dts(np, di);

	di->sysfs_data.pwr_limit = WLDC_IMPOSSIBLE_PWR;
	di->sysfs_data.cp_iin_thermal = WLDC_DFT_MAX_CP_IOUT;
	di->sysfs_data.enable_charger = 1;

	INIT_DELAYED_WORK(&di->wldc_ctrl_work, wldc_control_work);
	INIT_DELAYED_WORK(&di->wldc_calc_work, wldc_calc_work);
	wlsc_sysfs_create_group(di->dev);

	power_if_ops_register(&wlsc_if_ops);

	hwlog_info("wireless_sc probe ok\n");
	return 0;
}

static const struct of_device_id wlsc_match_table[] = {
	{
		.compatible = "huawei,wireless_sc",
		.data = NULL,
	},
	{},
};

static struct platform_driver wlsc_driver = {
	.probe = wlsc_probe,
	.driver = {
		.name = "huawei,wireless_sc",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(wlsc_match_table),
	},
};

static int __init wlsc_init(void)
{
	return platform_driver_register(&wlsc_driver);
}

static void __exit wlsc_exit(void)
{
	platform_driver_unregister(&wlsc_driver);
}

device_initcall_sync(wlsc_init);
module_exit(wlsc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless sc module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
