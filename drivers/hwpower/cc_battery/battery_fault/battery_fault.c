/*
 * battery_fault.c
 *
 * huawei battery fault driver
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

#include "battery_fault.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include "../battery_ui_capacity/battery_ui_capacity.h"
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG battery_fault
HWLOG_REGIST();

static struct bat_fault_device *g_bat_fault_dev;

int bat_fault_register_ops(const struct bat_fault_ops *ops)
{
	struct bat_fault_device *di = g_bat_fault_dev;

	if (!di)
		return -1;

	di->ops = ops;
	return 0;
}

int bat_fault_is_cutoff_vol(void)
{
	struct bat_fault_device *di = g_bat_fault_dev;

	if (!di)
		return 0;

	return di->data.vol_cutoff_sign;
}

static void bat_fault_notify(unsigned int event)
{
	struct bat_fault_device *di = g_bat_fault_dev;

	if (!di || !di->ops || !di->ops->notify)
		return;

	di->ops->notify(event);
}

static void bat_fault_update_cutoff_vol(bool sleep_mode)
{
	int vol;
	int temp;
	struct bat_fault_device *di = g_bat_fault_dev;

	if (!di)
		return;

	if (!sleep_mode)
		vol = di->config.vol_cutoff_normal;
	else
		vol = di->config.vol_cutoff_sleep;

	temp = coul_interface_get_battery_temperature(COUL_TYPE_MAIN);
	if (temp <= BAT_FAULT_LOW_TEMP_THLD)
		vol = (di->config.vol_cutoff_low_temp < vol) ?
			di->config.vol_cutoff_low_temp : vol;

	di->data.vol_cutoff_used = vol;
	coul_interface_set_battery_low_voltage(COUL_TYPE_MAIN, vol);
	hwlog_info("v_cut=%d, v_sleep=%d, v_low_temp=%d, temp=%d, vol=%d\n",
		di->config.vol_cutoff_normal,
		di->config.vol_cutoff_sleep,
		di->config.vol_cutoff_low_temp,
		temp, vol);
}

static void bat_fault_cutoff_vol_event_handle(struct bat_fault_device *di)
{
	int voltage;
	int ui_soc;
	int cutoff_vol = di->data.vol_cutoff_used;
	int count = di->config.vol_cutoff_filter_cnt;
	int bat_exist = coul_interface_is_battery_exist(COUL_TYPE_MAIN);
	char buf[BAT_FAULT_DSM_BUF_SIZE] = { 0 };

	if (power_cmdline_is_factory_mode() || !bat_exist)
		return;

	while (count-- >= 0) {
		voltage = coul_interface_get_battery_voltage(COUL_TYPE_MAIN);
		if ((voltage - BAT_FAULT_CUTOFF_VOL_OFFSET) > cutoff_vol) {
			hwlog_err("filter fail:vol=%d,count=%d,v_cut=%d\n",
				voltage, count, cutoff_vol);
			return;
		}

		if (count > 0)
			msleep(BAT_FAULT_CUTOFF_VOL_PERIOD);
	}

	di->data.vol_cutoff_sign = 1;
	ui_soc = bat_ui_capacity();
	hwlog_info("cutoff:vol=%d,v_cut=%d,cur_soc=%d\n",
		voltage, cutoff_vol, ui_soc);

	bat_fault_notify(POWER_NE_COUL_LOW_VOL);
	if (ui_soc > BAT_FAULT_ABNORMAL_DELTA_SOC) {
		snprintf(buf, BAT_FAULT_DSM_BUF_SIZE,
			"[LOW VOL]cur_vol:%dmV,cut_vol=%dmV,cur_soc=%d",
			voltage, cutoff_vol, ui_soc);
		power_dsm_report_dmd(POWER_DSM_BATTERY_DETECT,
			ERROR_LOW_VOL_INT, buf);
	}
}

static void bat_fault_work(struct work_struct *work)
{
	struct bat_fault_device *di = container_of(work,
		struct bat_fault_device, fault_work.work);

	__pm_stay_awake(&di->wake_lock);
	bat_fault_cutoff_vol_event_handle(di);
	__pm_relax(&di->wake_lock);
}

static int bat_fault_coul_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct bat_fault_device *di = g_bat_fault_dev;

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_COUL_LOW_VOL:
		queue_delayed_work(system_power_efficient_wq, &di->fault_work,
			msecs_to_jiffies(0));
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static ssize_t bat_fault_vol_cutoff_show(void *dev_data,
	char *buf, size_t size)
{
	struct bat_fault_device *di = dev_data;

	if (!di)
		return scnprintf(buf, size, "not support\n");

	return scnprintf(buf, size, "vol_cutoff_used is %d\n",
		di->data.vol_cutoff_used);
}

static ssize_t bat_fault_vol_cutoff_store(void *dev_data,
	const char *buf, size_t size)
{
	int val = 0;
	struct bat_fault_device *di = dev_data;

	if (!di)
		return -EINVAL;

	if (kstrtoint(buf, 0, &val)) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	di->config.vol_cutoff_normal = val;
	bat_fault_update_cutoff_vol(false);
	return size;
}

static void bat_fault_dbg_register(struct bat_fault_device *di)
{
	power_dbg_ops_register("bat_fault_vol_cutoff", (void *)di,
		(power_dbg_show)bat_fault_vol_cutoff_show,
		(power_dbg_store)bat_fault_vol_cutoff_store);
}

static void bat_fault_parse_dts(struct bat_fault_device *di)
{
	struct device_node *np = di->dev->of_node;

	if (!np)
		return;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vol_cutoff_normal", &di->config.vol_cutoff_normal,
		BAT_FAULT_NORMAL_CUTOFF_VOL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vol_cutoff_sleep", &di->config.vol_cutoff_sleep,
		BAT_FAULT_SLEEP_CUTOFF_VOL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vol_cutoff_low_temp", &di->config.vol_cutoff_low_temp,
		BAT_FAULT_NORMAL_CUTOFF_VOL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vol_cutoff_filter_cnt", &di->config.vol_cutoff_filter_cnt,
		BAT_FAULT_CUTOFF_VOL_FILTERS);
}

static int bat_fault_probe(struct platform_device *pdev)
{
	int ret;
	struct bat_fault_device *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	g_bat_fault_dev = di;
	bat_fault_parse_dts(di);
	bat_fault_dbg_register(di);
	bat_fault_update_cutoff_vol(true);

	INIT_DELAYED_WORK(&di->fault_work, bat_fault_work);
	di->coul_event_nb.notifier_call = bat_fault_coul_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_COUL, &di->coul_event_nb);
	if (ret)
		goto fail_free_mem;

	wakeup_source_init(&di->wake_lock, pdev->name);
	platform_set_drvdata(pdev, di);
	return 0;

fail_free_mem:
	kfree(di);
	g_bat_fault_dev = NULL;
	return ret;
}

static int bat_fault_remove(struct platform_device *pdev)
{
	struct bat_fault_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	platform_set_drvdata(pdev, NULL);
	wakeup_source_trash(&di->wake_lock);
	cancel_delayed_work(&di->fault_work);
	power_event_bnc_unregister(POWER_BNT_COUL, &di->coul_event_nb);
	kfree(di);
	g_bat_fault_dev = NULL;
	return 0;
}

#ifdef CONFIG_PM
static int bat_fault_suspend(struct platform_device *pdev, pm_message_t state)
{
	bat_fault_update_cutoff_vol(true);
	return 0;
}

static int bat_fault_resume(struct platform_device *pdev)
{
	bat_fault_update_cutoff_vol(false);
	return 0;
}
#endif /* CONFIG_PM */

static const struct of_device_id bat_fault_match_table[] = {
	{
		.compatible = "huawei,battery_fault",
		.data = NULL,
	},
	{},
};

static struct platform_driver bat_fault_driver = {
	.probe = bat_fault_probe,
	.remove = bat_fault_remove,
#ifdef CONFIG_PM
	.suspend = bat_fault_suspend,
	.resume = bat_fault_resume,
#endif /* CONFIG_PM */
	.driver = {
		.name = "huawei,battery_fault",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bat_fault_match_table),
	},
};

static int __init bat_fault_init(void)
{
	return platform_driver_register(&bat_fault_driver);
}

static void __exit bat_fault_exit(void)
{
	platform_driver_unregister(&bat_fault_driver);
}

device_initcall_sync(bat_fault_init);
module_exit(bat_fault_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery fault driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
