/*
 * wireless_test_mmi.c
 *
 * wireless charge mmi test
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
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_test.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <chipset_common/hwpower/wireless_charge/wireless_test_mmi.h>
#include <huawei_platform/power/wireless/wireless_charger.h>

#define HWLOG_TAG wireless_test_mmi
HWLOG_REGIST();

static struct wlrx_mmi_data *g_wlrx_mmi_di;

static int wlrx_mmi_parse_dts(struct device_node *np, struct wlrx_mmi_data *di)
{
	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"wlc_mmi_para", (u32 *)&di->dts_para, WLRX_MMI_PARA_END)) {
		di->dts_para.timeout = WLRX_MMI_DFLT_EX_TIMEOUT;
		di->dts_para.expt_prot = WLRX_MMI_DFLT_EX_PROT;
		di->dts_para.expt_icon = WLRX_MMI_DFLT_EX_ICON;
	}
	hwlog_info("timeout:%ds, expt_prot:0x%x, expt_icon:%d, iout_lth:%dmA, vmax_lth:%dmV\n",
		di->dts_para.timeout, di->dts_para.expt_prot,
		di->dts_para.expt_icon, di->dts_para.iout_lth,
		di->dts_para.vmax_lth);

	return 0;
}

static void wlrx_mmi_state_init(struct wlrx_mmi_data *di)
{
	di->prot_state = 0;
	di->icon = 0;
	di->vmax = 0;
}

static void wlrx_mmi_tx_vset_handler(struct wlrx_mmi_data *di, const void *data)
{
	int vmax;

	if (!data || (*(int *)data <= 0))
		return;

	vmax = *(int *)data;
	hwlog_info("tx_vmax = %dmV\n", vmax);
	if (vmax > di->vmax)
		di->vmax = vmax;
}

static void wlrx_mmi_icon_handler(struct wlrx_mmi_data *di, const void *data)
{
	if (!data)
		return;

	di->icon = *(int *)data;
}

static int wlrx_mmi_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct wlrx_mmi_data *di = g_wlrx_mmi_di;

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_WLC_READY:
		wlrx_mmi_state_init(di);
		break;
	case POWER_NE_WLC_HS_SUCC:
		di->prot_state |= WLRX_MMI_PROT_HANDSHAKE;
		break;
	case POWER_NE_WLC_TX_CAP_SUCC:
		di->prot_state |= WLRX_MMI_PROT_TX_CAP;
		break;
	case POWER_NE_WLC_CERT_SUCC:
		di->prot_state |= WLRX_MMI_PROT_CERT;
		break;
	case POWER_NE_WLC_TX_VSET:
		wlrx_mmi_tx_vset_handler(di, data);
		break;
	case POWER_NE_WLC_ICON_TYPE:
		wlrx_mmi_icon_handler(di, data);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static void wlrx_mmi_timeout_work(struct work_struct *work)
{
	wlc_set_high_pwr_test_flag(false);
}

#ifdef CONFIG_SYSFS
static int wlrx_mmi_iout_test(struct wlrx_mmi_data *di)
{
	int iout;
	static unsigned long timeout;

	iout = wlc_get_cp_avg_iout();
	hwlog_info("cp_iout = %dmA\n", iout);
	if (!di->iout_first_match && (iout >= di->dts_para.iout_lth)) {
		di->iout_first_match = true;
		timeout = jiffies + msecs_to_jiffies(
			WLRX_MMI_IOUT_HOLD_TIME * MSEC_PER_SEC);
	}
	if (iout < di->dts_para.iout_lth) {
		di->iout_first_match = false;
		return -1;
	}
	if (!time_after(jiffies, timeout))
		return -1;

	return 0;
}

static int wlrx_mmi_result(struct wlrx_mmi_data *di)
{
	hwlog_info("prot = 0x%x, icon = %d, vmax = %d\n",
		di->prot_state, di->icon, di->vmax);
	if (wlrx_get_charge_stage() < WLRX_STAGE_CHARGING)
		return -1;
	if (di->prot_state != di->dts_para.expt_prot)
		return -1;
	if (di->icon < di->dts_para.expt_icon)
		return -1;
	if ((di->dts_para.vmax_lth > 0) && (di->vmax < di->dts_para.vmax_lth))
		return -1;
	if ((di->dts_para.iout_lth > 0) && wlrx_mmi_iout_test(di))
		return -1;

	return 0;
}

static void wlrx_mmi_start(struct wlrx_mmi_data *di)
{
	unsigned long timeout;

	hwlog_info("start mmi test\n");
	di->iout_first_match = false;
	wlc_set_high_pwr_test_flag(true);
	timeout = di->dts_para.timeout + WLRX_MMI_TO_BUFF;
	if (delayed_work_pending(&di->timeout_work))
		cancel_delayed_work_sync(&di->timeout_work);
	schedule_delayed_work(&di->timeout_work,
		msecs_to_jiffies(timeout * MSEC_PER_SEC));
}

static ssize_t wlrx_mmi_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static ssize_t wlrx_mmi_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info wlrx_mmi_sysfs_field_tbl[] = {
	power_sysfs_attr_wo(wlrx_mmi, 0200, WLRX_MMI_SYSFS_START, start),
	power_sysfs_attr_ro(wlrx_mmi, 0440, WLRX_MMI_SYSFS_RESULT, result),
	power_sysfs_attr_ro(wlrx_mmi, 0440, WLRX_MMI_SYSFS_TIMEOUT, timeout),
};

#define WLRX_MMI_SYSFS_ATTRS_SIZE  ARRAY_SIZE(wlrx_mmi_sysfs_field_tbl)

static struct attribute *wlrx_mmi_sysfs_attrs[WLRX_MMI_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group wlrx_mmi_sysfs_attr_group = {
	.name = "wlc_mmi",
	.attrs = wlrx_mmi_sysfs_attrs,
};

static ssize_t wlrx_mmi_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct wlrx_mmi_data *di = g_wlrx_mmi_di;
	struct power_sysfs_attr_info *info = NULL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		wlrx_mmi_sysfs_field_tbl, WLRX_MMI_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case WLRX_MMI_SYSFS_RESULT:
		return snprintf(buf, PAGE_SIZE, "%d\n", wlrx_mmi_result(di));
	case WLRX_MMI_SYSFS_TIMEOUT:
		return snprintf(buf, PAGE_SIZE, "%d\n", di->dts_para.timeout);
	default:
		break;
	}

	return 0;
}

static ssize_t wlrx_mmi_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct wlrx_mmi_data *di = g_wlrx_mmi_di;
	struct power_sysfs_attr_info *info = NULL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		wlrx_mmi_sysfs_field_tbl, WLRX_MMI_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case WLRX_MMI_SYSFS_START:
		wlrx_mmi_start(di);
		break;
	default:
		break;
	}

	return count;
}

static int wlrx_mmi_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(wlrx_mmi_sysfs_attrs,
		wlrx_mmi_sysfs_field_tbl, WLRX_MMI_SYSFS_ATTRS_SIZE);
	return sysfs_create_group(&dev->kobj, &wlrx_mmi_sysfs_attr_group);
}

static void wlrx_mmi_sysfs_remove_group(struct device *dev)
{
	sysfs_remove_group(&dev->kobj, &wlrx_mmi_sysfs_attr_group);
}
#else
static int wlrx_mmi_sysfs_create_group(struct device *dev)
{
	return 0;
}

static void wlrx_mmi_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void wlrx_mmi_init(struct device *dev)
{
	int ret;
	struct wlrx_mmi_data *di = NULL;

	if (!dev || !dev->of_node)
		return;

	di = devm_kzalloc(dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return;

	g_wlrx_mmi_di = di;
	ret = wlrx_mmi_parse_dts(dev->of_node, di);
	if (ret)
		goto free_mem;

	INIT_DELAYED_WORK(&di->timeout_work, wlrx_mmi_timeout_work);
	di->wlrx_mmi_nb.notifier_call = wlrx_mmi_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_WLC, &di->wlrx_mmi_nb);
	if (ret)
		goto free_mem;

	ret = wlrx_mmi_sysfs_create_group(dev);
	if (ret)
		goto unregister_notifier;

	hwlog_info("init succ\n");
	return;

unregister_notifier:
	(void)power_event_bnc_unregister(POWER_BNT_WLC, &di->wlrx_mmi_nb);
free_mem:
	devm_kfree(dev, di);
	g_wlrx_mmi_di = NULL;
}

static void wlrx_mmi_exit(struct device *dev)
{
	if (!g_wlrx_mmi_di || !dev)
		return;

	wlrx_mmi_sysfs_remove_group(dev);
	cancel_delayed_work(&g_wlrx_mmi_di->timeout_work);
	(void)power_event_bnc_unregister(POWER_BNT_WLC, &g_wlrx_mmi_di->wlrx_mmi_nb);
	devm_kfree(dev, g_wlrx_mmi_di);
	g_wlrx_mmi_di = NULL;
}

static struct power_test_ops g_wlrx_mmi_ops = {
	.name = "wlc_mmi",
	.init = wlrx_mmi_init,
	.exit = wlrx_mmi_exit,
};

static int __init wlrx_mmi_module_init(void)
{
	return power_test_ops_register(&g_wlrx_mmi_ops);
}

module_init(wlrx_mmi_module_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wlrx_mmi module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
