/*
 * series_batt_speaker_smart_charger.c
 *
 * series_batt_speaker_smart_charger driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/power/speaker_charger/series_batt_speaker_smart_charger.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>

#define HWLOG_TAG series_batt_speaker_smart_charger
HWLOG_REGIST();

static struct smart_charge_device_info *g_smart_charge_dev;

static struct smart_charge_device_info *series_batt_speaker_smart_charge_get_di(void)
{
	if (!g_smart_charge_dev) {
		hwlog_err("di is null\n");
		return NULL;
	}

	return g_smart_charge_dev;
}

static void series_batt_speaker_smart_charge_config_cc_cv(struct charge_device_info *di)
{
	int ret;
	int charge_current, charge_voltage;

	charge_current = coul_drv_get_desired_charging_current();
	charge_voltage = coul_drv_get_desired_charging_voltage();

	charge_current = charge_current < di->charge_current ?
		charge_current : di->charge_current;

	/* set CC charge current */
	ret = di->ops->set_charge_current(charge_current);
	if (ret > 0) {
		hwlog_info("charge current is out of range:%dmA\n", ret);
		di->ops->set_charge_current(ret);
	} else if (ret < 0) {
		hwlog_err("set charge current fail\n");
	}

	/* set CV terminal voltage */
	if (power_cmdline_is_factory_mode() && !coul_drv_is_battery_exist()) {
		charge_voltage = coul_drv_battery_vbat_max();
		hwlog_info("facory_version and battery not exist, vterm is set to %d\n", charge_voltage);
	} else if (charge_voltage <= 0) {
		charge_voltage = di->core_data->vterm < di->sysfs_data.vterm_rt ?
			di->core_data->vterm : di->sysfs_data.vterm_rt;
	}
	hwlog_info("set vterm %d\n", charge_voltage);
	ret = di->ops->set_terminal_voltage(charge_voltage);
	if (ret > 0) {
		hwlog_info("terminal voltage is out of range, set vterm:%dmV\n", ret);
		di->ops->set_terminal_voltage(ret);
	} else if (ret < 0) {
		hwlog_err("set terminal voltage fail\n");
	}

	return;
}

static void series_batt_speaker_smart_charge_work(struct work_struct *work)
{
	struct smart_charge_device_info *sc_di = container_of(work,
		struct smart_charge_device_info, charge_work.work);
	struct charge_device_info *di = sc_di->charge_di;

	if (!di || !di->ops || !di->ops->set_charge_current ||
		!di->ops->set_terminal_voltage) {
		hwlog_err("di or ops is null\n");
		return;
	}

	series_batt_speaker_smart_charge_config_cc_cv(di);
	queue_delayed_work(system_power_efficient_wq, &sc_di->charge_work,
		msecs_to_jiffies(SMART_CHARGING_WORK_TIMEOUT));
}

int series_batt_speaker_smart_charge_start_charging(struct charge_device_info *di)
{
	struct smart_charge_device_info *sc_di = series_batt_speaker_smart_charge_get_di();

	if (!sc_di)
		return -1;

	sc_di->charge_di = di;
	queue_delayed_work(system_power_efficient_wq, &sc_di->charge_work, 0);

	return 0;
}

int series_batt_speaker_smart_charge_stop_charging(struct charge_device_info *di)
{
	struct smart_charge_device_info *sc_di = series_batt_speaker_smart_charge_get_di();

	if (!sc_di)
		return -1;

	cancel_delayed_work(&sc_di->charge_work);

	return 0;
}

static int series_batt_speaker_smart_charge_probe(struct platform_device *pdev)
{
	struct smart_charge_device_info *sc_di = NULL;

	sc_di = kzalloc(sizeof(*sc_di), GFP_KERNEL);
	if (!sc_di) {
		hwlog_err("alloc sc di failed\n");
		return -ENOMEM;
	}

	sc_di->dev = &pdev->dev;
	g_smart_charge_dev = sc_di;
	INIT_DELAYED_WORK(&sc_di->charge_work,
		series_batt_speaker_smart_charge_work);

	hwlog_info("probe ok\n");
	return 0;
}

static int series_batt_speaker_smart_charge_remove(struct platform_device *pdev)
{
	struct smart_charge_device_info *sc_di = platform_get_drvdata(pdev);

	if (!sc_di) {
		hwlog_err("sc di is NULL\n");
		return -ENODEV;
	}

	cancel_delayed_work(&sc_di->charge_work);
	kfree(sc_di);
	g_smart_charge_dev = NULL;
}

static void series_batt_speaker_smart_charge_shutdown(struct platform_device *pdev)
{
	struct smart_charge_device_info *sc_di = platform_get_drvdata(pdev);

	if (!sc_di) {
		hwlog_err("sc di is NULL\n");
		return;
	}

	cancel_delayed_work(&sc_di->charge_work);
	return;
}

#ifdef CONFIG_PM
static int series_batt_speaker_smart_charge_resume(struct platform_device *pdev)
{
	struct smart_charge_device_info *sc_di = platform_get_drvdata(pdev);

	if (!sc_di)
		return 0;

	queue_delayed_work(system_power_efficient_wq, &sc_di->charge_work, 0);
	return 0;
}

static int series_batt_speaker_smart_charge_suspend(struct platform_device *pdev,
	pm_message_t state)
{
	return 0;
}
#endif /* CONFIG_PM */

static const struct of_device_id smart_charge_match_table[] = {
	{
		.compatible = "huawei,series_batt_speaker_smart_charger",
		.data = NULL,
	},
	{},
};

static struct platform_driver smart_charge_driver = {
	.probe = series_batt_speaker_smart_charge_probe,
	.remove = series_batt_speaker_smart_charge_remove,
#ifdef CONFIG_PM
	.suspend = series_batt_speaker_smart_charge_suspend,
	.resume = series_batt_speaker_smart_charge_resume,
#endif /* CONFIG_PM */
	.shutdown = series_batt_speaker_smart_charge_shutdown,
	.driver = {
		.name = "huawei,series_batt_speaker_smart_charger",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(smart_charge_match_table),
	},
};

static int __init series_batt_speaker_smart_charge_init(void)
{
	return platform_driver_register(&smart_charge_driver);
}

static void __exit series_batt_speaker_smart_charge_exit(void)
{
	platform_driver_unregister(&smart_charge_driver);
}

module_init(series_batt_speaker_smart_charge_init);
module_exit(series_batt_speaker_smart_charge_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei series batt speaker smart charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
