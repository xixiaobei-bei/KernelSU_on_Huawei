/*
 * wlc_mmi.c
 *
 * wireless charge MMI test
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
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/power_dts.h>
#include <huawei_platform/power/power_test.h>
#include <huawei_platform/power/power_sysfs.h>
#include <huawei_platform/power/wireless_charger.h>

#include "wlc_mmi.h"

#define HWLOG_TAG wireless_charge_mmi
HWLOG_REGIST();

static struct wlc_mmi_data *g_wlc_mmi_di;

static int wlc_mmi_parse_dts(struct device_node *np, struct wlc_mmi_data *di)
{
	u32 tmp_para[WLC_MMI_PARA_MAX] = { 0 };

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"wlc_mmi_para", tmp_para, WLC_MMI_PARA_MAX)) {
		tmp_para[WLC_MMI_PARA_TIMEOUT] = WLC_MMI_DFLT_EX_TIMEOUT;
		tmp_para[WLC_MMI_PARA_EXPT_PORT] = WLC_MMI_DFLT_EX_PROT;
		tmp_para[WLC_MMI_PARA_EXPT_ICON] = WLC_MMI_DFLT_EX_ICON;
	}
	di->dts_para.timeout = (int)tmp_para[WLC_MMI_PARA_TIMEOUT];
	di->dts_para.expt_prot = tmp_para[WLC_MMI_PARA_EXPT_PORT];
	di->dts_para.expt_icon = (int)tmp_para[WLC_MMI_PARA_EXPT_ICON];
	di->dts_para.iout_lth = (int)tmp_para[WLC_MMI_PARA_IOUT_LTH];
	di->dts_para.vmax_lth = (int)tmp_para[WLC_MMI_PARA_VMAX_LTH];
	hwlog_info("[wlc_mmi_para] timeout:%ds, expt_prot:0x%x, expt_icon:%d\t"
		"iout_lth:%dmA vmax_lth:%dmV\n",
		di->dts_para.timeout, di->dts_para.expt_prot,
		di->dts_para.expt_icon, di->dts_para.iout_lth,
		di->dts_para.vmax_lth);

	return 0;
}

static void wlc_mmi_state_init(struct wlc_mmi_data *di)
{
	di->prot_state = 0;
	di->icon = 0;
	di->vmax = 0;
}

static void wlc_mmi_tx_vset_handler(struct wlc_mmi_data *di, void *data)
{
	int vmax;

	if (!data || (*(int *)data <= 0))
		return;

	vmax = *(int *)data;
	hwlog_info("tx_vmax = %dmV\n", vmax);
	if (vmax > di->vmax)
		di->vmax = vmax;
}

static void wlc_mmi_icon_handler(struct wlc_mmi_data *di, void *data)
{
	if (!data)
		return;

	di->icon = *(int *)data;
}

static int wlc_mmi_notifier_call(struct notifier_block *wlc_mmi_nb,
	unsigned long evt, void *data)
{
	struct wlc_mmi_data *di = g_wlc_mmi_di;

	if (!di) {
		hwlog_err("notifier_call: di null\n");
		return NOTIFY_OK;
	}
	switch (evt) {
	case WLC_NOTIFY_READY:
		wlc_mmi_state_init(di);
		break;
	case WLC_NOTIFY_HS_SUCC:
		di->prot_state |= WLC_MMI_PROT_HANDSHAKE;
		break;
	case WLC_NOTIFY_TX_CAP_SUCC:
		di->prot_state |= WLC_MMI_PROT_TX_CAP;
		break;
	case WLC_NOTIFY_CERT_SUCC:
		di->prot_state |= WLC_MMI_PROT_CERT;
		break;
	case WLC_NOTIFY_TX_VSET:
		wlc_mmi_tx_vset_handler(di, data);
		break;
	case WLC_NOTIFY_ICON_TYPE:
		wlc_mmi_icon_handler(di, data);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static void wlc_mmi_timeout_work(struct work_struct *work)
{
	wlc_set_high_pwr_test_flag(false);
}

static int wlc_mmi_iout_test(struct wlc_mmi_data *di)
{
	int iout;
	static unsigned long timeout;

	iout = wlc_get_cp_avg_iout();
	hwlog_info("cp_iout = %dmA\n", iout);
	if (!di->iout_first_match && (iout >= di->dts_para.iout_lth)) {
		di->iout_first_match = true;
		timeout = jiffies + msecs_to_jiffies(
			WLC_MMI_IOUT_HOLD_TIME * MSEC_PER_SEC);
	}
	if (iout < di->dts_para.iout_lth) {
		di->iout_first_match = false;
		return -1;
	}
	if (!time_after(jiffies, timeout))
		return -1;

	return 0;
}

static int wlc_mmi_result(struct wlc_mmi_data *di)
{
	hwlog_info("prot = 0x%x, icon = %d, vmax = %d\n",
		di->prot_state, di->icon, di->vmax);
	if (wlc_get_charge_stage() < WIRELESS_STAGE_CHARGING)
		return -1;
	if (di->prot_state != di->dts_para.expt_prot)
		return -1;
	if (di->icon < di->dts_para.expt_icon)
		return -1;
	if ((di->dts_para.vmax_lth > 0) && (di->vmax < di->dts_para.vmax_lth))
		return -1;
	if ((di->dts_para.iout_lth > 0) && wlc_mmi_iout_test(di))
		return -1;

	return 0;
}

static void wlc_mmi_start(struct wlc_mmi_data *di)
{
	unsigned long timeout;

	hwlog_info("start MMI test\n");
	di->iout_first_match = false;
	wlc_set_high_pwr_test_flag(true);
	timeout = di->dts_para.timeout + WLC_MMI_TO_BUFF;
	if (delayed_work_pending(&di->timeout_work))
		cancel_delayed_work_sync(&di->timeout_work);
	schedule_delayed_work(&di->timeout_work,
		msecs_to_jiffies(timeout * MSEC_PER_SEC));
}

#ifdef CONFIG_SYSFS
static ssize_t wlc_mmi_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static ssize_t wlc_mmi_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info wlc_mmi_sysfs_field_tbl[] = {
	power_sysfs_attr_wo(wlc_mmi, 0640, WLC_MMI_SYSFS_START, start),
	power_sysfs_attr_ro(wlc_mmi, 0440, WLC_MMI_SYSFS_RESULT, result),
	power_sysfs_attr_ro(wlc_mmi, 0440, WLC_MMI_SYSFS_TIMEOUT, timeout),
};

#define WLC_MMI_SYSFS_ATTRS_SIZE  ARRAY_SIZE(wlc_mmi_sysfs_field_tbl)

static struct attribute *wlc_mmi_sysfs_attrs[WLC_MMI_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group wlc_mmi_sysfs_attr_group = {
	.name = "wlc_mmi",
	.attrs = wlc_mmi_sysfs_attrs,
};

static void wlc_mmi_sysfs_init_attrs(void)
{
	int s;

	for (s = 0; s < WLC_MMI_SYSFS_ATTRS_SIZE; s++)
		wlc_mmi_sysfs_attrs[s] = &wlc_mmi_sysfs_field_tbl[s].attr.attr;

	wlc_mmi_sysfs_attrs[s] = NULL;
}

static struct power_sysfs_attr_info *wlc_mmi_sysfs_field_lkup(const char *name)
{
	int s;

	for (s = 0; s < WLC_MMI_SYSFS_ATTRS_SIZE; s++) {
		if (!strncmp(name, wlc_mmi_sysfs_field_tbl[s].attr.attr.name,
			strlen(name)))
			break;
	}

	if (s >= WLC_MMI_SYSFS_ATTRS_SIZE)
		return NULL;

	return &wlc_mmi_sysfs_field_tbl[s];
}

static ssize_t wlc_mmi_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct wlc_mmi_data *di = g_wlc_mmi_di;
	struct power_sysfs_attr_info *info = NULL;

	info = wlc_mmi_sysfs_field_lkup(attr->attr.name);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case WLC_MMI_SYSFS_RESULT:
		return snprintf(buf, PAGE_SIZE, "%d\n", wlc_mmi_result(di));
	case WLC_MMI_SYSFS_TIMEOUT:
		return snprintf(buf, PAGE_SIZE, "%d\n", di->dts_para.timeout);
	default:
		break;
	}

	return 0;
}

static ssize_t wlc_mmi_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct wlc_mmi_data *di = g_wlc_mmi_di;
	struct power_sysfs_attr_info *info = NULL;

	info = wlc_mmi_sysfs_field_lkup(attr->attr.name);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case WLC_MMI_SYSFS_START:
		wlc_mmi_start(di);
		break;
	default:
		break;
	}

	return count;
}

static int wlc_mmi_sysfs_create_group(struct device *dev)
{
	wlc_mmi_sysfs_init_attrs();
	return sysfs_create_group(&dev->kobj, &wlc_mmi_sysfs_attr_group);
}

static void wlc_mmi_sysfs_remove_group(struct device *dev)
{
	sysfs_remove_group(&dev->kobj, &wlc_mmi_sysfs_attr_group);
}
#else
static int wlc_mmi_sysfs_create_group(struct device *dev)
{
	return 0;
}

static void wlc_mmi_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void wlc_mmi_init(struct device *dev)
{
	int ret;
	struct wlc_mmi_data *di = NULL;

	if (!dev || !dev->of_node) {
		hwlog_err("init: para error\n");
		return;
	}

	di = devm_kzalloc(dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return;

	g_wlc_mmi_di = di;
	ret = wlc_mmi_parse_dts(dev->of_node, di);
	if (ret) {
		hwlog_err("init: parse dts failed\n");
		goto free_mem;
	}

	INIT_DELAYED_WORK(&di->timeout_work, wlc_mmi_timeout_work);
	di->wlc_mmi_nb.notifier_call = wlc_mmi_notifier_call;
	ret = register_wireless_charge_notifier(&di->wlc_mmi_nb);
	if (ret) {
		hwlog_err("init: register notifier failed\n");
		goto free_mem;
	}

	ret = wlc_mmi_sysfs_create_group(dev);
	if (ret) {
		hwlog_err("init: create group failed\n");
		goto unregister_notifier;
	}

	hwlog_info("init succ\n");
	return;

unregister_notifier:
	unregister_wireless_charge_notifier(&di->wlc_mmi_nb);
free_mem:
	devm_kfree(dev, di);
	g_wlc_mmi_di = NULL;
}

static void wlc_mmi_exit(struct device *dev)
{
	if (!g_wlc_mmi_di || !dev)
		return;

	wlc_mmi_sysfs_remove_group(dev);
	cancel_delayed_work(&g_wlc_mmi_di->timeout_work);
	(void)unregister_wireless_charge_notifier(&g_wlc_mmi_di->wlc_mmi_nb);
	devm_kfree(dev, g_wlc_mmi_di);
	g_wlc_mmi_di = NULL;
}

static struct power_test_ops g_wlc_mmi_ops = {
	.name   = "wlc_mmi",
	.init   = wlc_mmi_init,
	.exit   = wlc_mmi_exit,
};

static int __init wlc_mmi_module_init(void)
{
	return power_test_ops_register(&g_wlc_mmi_ops);
}

module_init(wlc_mmi_module_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wlc_mmi module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
