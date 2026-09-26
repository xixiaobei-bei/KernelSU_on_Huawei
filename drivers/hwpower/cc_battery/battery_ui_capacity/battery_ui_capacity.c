/*
 * battery_ui_capacity.c
 *
 * huawei battery ui capacity
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

#include "battery_ui_capacity.h"
#include <linux/init.h>
#include <linux/module.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/battery/battery_capacity_public.h>
#include "../battery_model/battery_model.h"
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>

#define HWLOG_TAG bat_ui_capacity
HWLOG_REGIST();

static struct bat_ui_capacity_device *g_bat_ui_capacity_dev;
static const struct bat_ui_vbat_para g_bat_ui_fake_vbat_level[] = {
	{ 3500,  2 },
	{ 3550,  10 },
	{ 3600,  20 },
	{ 3700,  30 },
	{ 3800,  40 },
	{ 3850,  50 },
	{ 3900,  60 },
	{ 4000,  70 },
	{ 4250,  85 },
};

int bat_ui_capacity(void)
{
	struct bat_ui_capacity_device *di = g_bat_ui_capacity_dev;

	if (!di)
		return coul_interface_get_battery_capacity(COUL_TYPE_MAIN);

	return di->ui_capacity;
}

int bat_ui_raw_capacity(void)
{
	return coul_interface_get_battery_capacity(COUL_TYPE_MAIN);
}

static int bat_ui_capacity_calc_from_voltage(struct bat_ui_capacity_device *di)
{
	int i;
	int vbat_size = ARRAY_SIZE(g_bat_ui_fake_vbat_level);
	int bat_volt = di->bat_volt;

	if (bat_volt <= BUC_VBAT_MIN)
		return BUC_CAPACITY_EMPTY;

	bat_volt -= di->bat_cur * BUC_CONVERT_VOLTAGE / BUC_CONVERT_CURRENT;

	if (bat_volt >= g_bat_ui_fake_vbat_level[vbat_size - 1].volt)
		return BUC_CAPACITY_FULL;

	for (i = 0; i < vbat_size; i++) {
		if (bat_volt < g_bat_ui_fake_vbat_level[i].volt)
			return g_bat_ui_fake_vbat_level[i].soc;
	}

	return BUC_DEFAULT_CAPACITY;
}

static int bat_ui_capacity_pulling_filter(struct bat_ui_capacity_device *di,
	int curr_capacity)
{
	int index;

	if (!di->bat_exist) {
		curr_capacity = bat_ui_capacity_calc_from_voltage(di);
		hwlog_info("cal curr_capacity=%d from voltage\n", curr_capacity);
		return curr_capacity;
	}

	index = di->capacity_filter_count % BUC_WINDOW_LEN;

	di->capacity_sum -= di->capacity_filter[index];
	di->capacity_filter[index] = curr_capacity;
	di->capacity_sum += di->capacity_filter[index];

	if (++di->capacity_filter_count >= BUC_WINDOW_LEN)
		di->capacity_filter_count = 0;

	return di->capacity_sum / BUC_WINDOW_LEN;
}

int bat_ui_capacity_get_filter_sum(int base)
{
	struct bat_ui_capacity_device *di = g_bat_ui_capacity_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	hwlog_info("get filter sum=%d\n", di->capacity_sum);

	return (di->capacity_sum * base) / BUC_WINDOW_LEN;
}

static void bat_ui_capacity_reset_capacity_fifo(struct bat_ui_capacity_device *di,
	int curr_capacity)
{
	unsigned int i;

	if (curr_capacity < 0)
		return;

	di->capacity_sum = 0;

	for (i = 0; i < BUC_WINDOW_LEN; i++) {
		di->capacity_filter[i] = curr_capacity;
		di->capacity_sum += curr_capacity;
	}
}

void bat_ui_capacity_sync_filter(int rep_soc, int round_soc, int base)
{
	int new_soc;
	struct bat_ui_capacity_device *di = g_bat_ui_capacity_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (!base) {
		hwlog_err("base is 0\n");
		return;
	}

	/* step1: reset capacity fifo */
	bat_ui_capacity_reset_capacity_fifo(di, round_soc);
	new_soc = (rep_soc * BUC_WINDOW_LEN / base) - round_soc * (BUC_WINDOW_LEN - 1);
	bat_ui_capacity_pulling_filter(di, new_soc);

	hwlog_info("sync filter prev_soc=%d,new_soc=%d,round_soc=%d\n",
		di->prev_soc, new_soc, round_soc);

	/* step2: capacity changed (example: 86% to 87%) */
	if (di->prev_soc != round_soc) {
		di->prev_soc = round_soc;
		di->ui_capacity = round_soc;
		di->ui_prev_capacity = round_soc;
		power_supply_sync_changed("Battery");
		power_supply_sync_changed("battery");
	}
}

void bat_ui_capacity_cancle_work(void)
{
	struct bat_ui_capacity_device *di = g_bat_ui_capacity_dev;

	if (!di)
		return;

	cancel_delayed_work_sync(&di->update_work);
}

void bat_ui_capacity_restart_work(void)
{
	struct bat_ui_capacity_device *di = g_bat_ui_capacity_dev;

	if (!di)
		return;

	mod_delayed_work(system_power_efficient_wq, &di->update_work,
		msecs_to_jiffies(BUC_WORK_INTERVAL_NORMAL));
}

static void bat_ui_capacity_force_full_timer(struct bat_ui_capacity_device *di,
	int *curr_capacity)
{
	if (*curr_capacity > di->chg_force_full_soc_thld) {
		di->chg_force_full_count++;
		if (di->chg_force_full_count >= di->chg_force_full_wait_time) {
			hwlog_info("wait out full time curr_capacity=%d\n", *curr_capacity);
			di->chg_force_full_count = di->chg_force_full_wait_time;
			*curr_capacity = BUC_CAPACITY_FULL;
		}
	} else {
		di->chg_force_full_count = 0;
	}
}

static int bat_ui_capacity_correct(struct bat_ui_capacity_device *di,
	int curr_cap)
{
	int i;

	for (i = 0; i < BUC_SOC_CALIBRATION_PARA_LEVEL; i++) {
		if ((curr_cap < di->vth_soc_calibration_data[i].soc) &&
			(di->bat_volt >= di->vth_soc_calibration_data[i].volt)) {
			hwlog_info("correct capacity: bat_vol=%d,cap=%d,lock_cap=%d\n",
				di->bat_volt, curr_cap, di->vth_soc_calibration_data[i].soc);
			return di->vth_soc_calibration_data[i].soc;
		}
	}
	return curr_cap;
}

static int bat_ui_capacity_vth_correct_soc(struct bat_ui_capacity_device *di,
	int curr_capacity)
{
	if (!di->vth_correct_en)
		return curr_capacity;

	if ((di->charge_status == POWER_SUPPLY_STATUS_DISCHARGING) ||
		(di->charge_status == POWER_SUPPLY_STATUS_NOT_CHARGING))
		curr_capacity = bat_ui_capacity_correct(di, curr_capacity);

	return curr_capacity;
}

void bat_ui_capacity_set_work_interval(struct bat_ui_capacity_device *di)
{
	if (!di->monitoring_interval ||
		(di->charge_status == POWER_SUPPLY_STATUS_DISCHARGING) ||
		(di->charge_status == POWER_SUPPLY_STATUS_NOT_CHARGING))
		di->monitoring_interval = BUC_WORK_INTERVAL_NORMAL;

	if ((di->bat_temp < BUC_LOWTEMP_THRESHOLD) &&
		(di->monitoring_interval > BUC_WORK_INTERVAL_LOWTEMP))
		di->monitoring_interval = BUC_WORK_INTERVAL_LOWTEMP;

	if (di->charge_status == POWER_SUPPLY_STATUS_CHARGING)
		di->monitoring_interval = BUC_WORK_INTERVAL_CHARGING;
}

static int bat_ui_capacity_handle(struct bat_ui_capacity_device *di,
	int *curr_capacity)
{
	int cap;

	if (!di->bat_exist) {
		*curr_capacity = bat_ui_capacity_calc_from_voltage(di);
		hwlog_info("use demon capacity=%d from voltage\n", *curr_capacity);
	} else {
		cap = coul_interface_get_battery_capacity(COUL_TYPE_MAIN);
		/* uisoc = soc/soc_at_term - ui_cap_zero_offset */
		if (di->soc_at_term)
			*curr_capacity = DIV_ROUND_CLOSEST((cap * BUC_CAPACITY_AMPLIFY),
				di->soc_at_term) - di->ui_cap_zero_offset;
		else
			*curr_capacity = cap;

		if (*curr_capacity > BUC_CAPACITY_FULL)
			*curr_capacity = BUC_CAPACITY_FULL;
		if (*curr_capacity < 0)
			*curr_capacity = 0;
		hwlog_info("capacity=%d from coul %d, zoom:%d\n",
			*curr_capacity, cap, di->soc_at_term);
	}

	if (!di->bat_exist && power_cmdline_is_factory_mode() &&
		(*curr_capacity <= BUC_LOW_CAPACITY_THRESHOLD)) {
		di->ui_capacity = BUC_LOW_CAPACITY;
		di->ui_prev_capacity = BUC_LOW_CAPACITY;
		hwlog_info("battery not exist in factory mode, force low capacity to %d\n",
			di->ui_capacity);
		return -1;
	}

	if (di->ui_capacity == -1) {
		di->ui_capacity = *curr_capacity;
		di->ui_prev_capacity = *curr_capacity;
		hwlog_info("first init ui_capacity=%d\n", di->ui_capacity);
		return -1;
	}

	if (abs(di->ui_prev_capacity - *curr_capacity) >= BUC_SOC_JUMP_THRESHOLD)
		hwlog_info("prev_capacity=%d, curr_capacity=%d, bat_volt=%d\n",
			di->ui_prev_capacity, *curr_capacity, di->bat_volt);

	return 0;
}

static void bat_ui_capacity_update_charge_status(struct bat_ui_capacity_device *di,
	int status)
{
	int cur_status = status;

	if ((status == POWER_SUPPLY_STATUS_CHARGING) &&
		(di->ui_capacity == BUC_CAPACITY_FULL))
		cur_status = POWER_SUPPLY_STATUS_FULL;

	di->charge_status = cur_status;
}

static int bat_ui_capacity_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	int status;
	struct bat_ui_capacity_device *di = g_bat_ui_capacity_dev;

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_START_CHARGING:
		status = POWER_SUPPLY_STATUS_CHARGING;
		break;
	case POWER_NE_STOP_CHARGING:
		status = POWER_SUPPLY_STATUS_DISCHARGING;
		break;
	case POWER_NE_SUSPEND_CHARGING:
		status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		break;
	default:
		return NOTIFY_OK;
	}

	hwlog_info("receive event=%lu,status=%d\n", event, status);
	bat_ui_capacity_update_charge_status(di, status);

	return NOTIFY_OK;
}

static int bat_ui_capacity_charge_status_check(struct bat_ui_capacity_device *di,
	int *curr_capacity)
{
	switch (di->charge_status) {
	case POWER_SUPPLY_STATUS_CHARGING:
		bat_ui_capacity_force_full_timer(di, curr_capacity);
		break;
	case POWER_SUPPLY_STATUS_FULL:
		if ((coul_interface_get_battery_avg_current(COUL_TYPE_MAIN) >= 0) &&
			(di->bat_volt >= (di->bat_max_volt - BUC_RECHG_PROTECT_VOLT_THRESHOLD))) {
			*curr_capacity = BUC_CAPACITY_FULL;
			hwlog_info("force curr_capacity=100\n");
		}
		di->chg_force_full_count = 0;
		break;
	case POWER_SUPPLY_STATUS_DISCHARGING:
	case POWER_SUPPLY_STATUS_NOT_CHARGING:
		if (di->ui_prev_capacity <= *curr_capacity) {
			hwlog_err("not charging pre=%d < curr_capacity=%d\n",
				di->ui_prev_capacity, *curr_capacity);
			return -1;
		}
		di->chg_force_full_count = 0;
		break;
	default:
		hwlog_err("other charge status %d\n", di->charge_status);
		break;
	}

	return 0;
}

static int bat_ui_capacity_exception_check(struct bat_ui_capacity_device *di,
	int curr_capacity)
{
	if (di->ui_prev_capacity == curr_capacity)
		return -1;

	if ((di->charge_status == POWER_SUPPLY_STATUS_DISCHARGING) ||
		(di->charge_status == POWER_SUPPLY_STATUS_NOT_CHARGING)) {
		if (di->ui_prev_capacity < curr_capacity)
			return -1;
	}

	if ((di->charge_status == POWER_SUPPLY_STATUS_CHARGING) &&
		((di->bat_cur >= BUC_CURRENT_THRESHOLD) ||
		power_platform_in_dc_charging_stage())) {
		if (di->ui_prev_capacity > curr_capacity)
			return -1;
	}

	return 0;
}

static bool bat_ui_capacity_is_changed(struct bat_ui_capacity_device *di)
{
	int curr_capacity = 0;
	int low_temp_capacity_record;
	int ret = bat_ui_capacity_handle(di, &curr_capacity);

	if (ret)
		return true;

	if ((curr_capacity < BUC_LOW_CAPACITY_THRESHOLD) &&
		!di->disable_pre_vol_check && (di->bat_volt >= BUC_VBAT_MIN)) {
		hwlog_info("low capacity: battery_vol=%d,curr_capacity=%d\n",
			di->bat_volt, curr_capacity);
		return false;
	}

	ret = bat_ui_capacity_charge_status_check(di, &curr_capacity);
	if (ret)
		return false;

	curr_capacity = bat_ui_capacity_vth_correct_soc(di, curr_capacity);
	low_temp_capacity_record = curr_capacity;
	curr_capacity = bat_ui_capacity_pulling_filter(di, curr_capacity);
	if ((di->bat_temp < BUC_LOWTEMP_THRESHOLD) &&
		((curr_capacity - low_temp_capacity_record) > 1)) {
		hwlog_info("curr_capacity=%d, low_temp_capacity_record=%d\n",
			curr_capacity, low_temp_capacity_record);
		curr_capacity -= 1;
	}

	if (bat_ui_capacity_exception_check(di, curr_capacity)) {
		hwlog_err("battery ui capacity exception\n");
		return false;
	}

	di->ui_capacity = curr_capacity;
	di->ui_prev_capacity = curr_capacity;
	return true;
}

static void bat_ui_capacity_update_info(struct bat_ui_capacity_device *di)
{
	di->bat_exist = coul_interface_is_battery_exist(COUL_TYPE_MAIN);
	di->bat_volt = coul_interface_get_battery_voltage(COUL_TYPE_MAIN);
	di->bat_cur = coul_interface_get_battery_current(COUL_TYPE_MAIN);
	di->bat_temp = coul_interface_get_battery_temperature(COUL_TYPE_MAIN);
}

static void bat_ui_capacity_work(struct work_struct *work)
{
	struct bat_ui_capacity_device *di = container_of(work,
		struct bat_ui_capacity_device, update_work.work);

	bat_ui_capacity_update_info(di);
	hwlog_info("update: e=%d,v=%d,t=%d,c=%d,s=%d,cap=%d,pcap=%d,cnt=%d\n",
		di->bat_exist, di->bat_volt, di->bat_temp,
		di->bat_cur, di->charge_status, di->ui_capacity,
		di->ui_prev_capacity, di->chg_force_full_count);

	if (bat_ui_capacity_is_changed(di)) {
		power_supply_sync_changed("Battery");
		power_supply_sync_changed("battery");
		hwlog_info("ui capacity change to %d\n", di->ui_capacity);
	}

	bat_ui_capacity_set_work_interval(di);
	queue_delayed_work(system_power_efficient_wq, &di->update_work,
		msecs_to_jiffies(di->monitoring_interval));
}

static void bat_ui_capacity_data_init(struct bat_ui_capacity_device *di)
{
	int capacity;

	di->bat_max_volt = bat_model_get_vbat_max();
	di->monitoring_interval = BUC_WORK_INTERVAL_NORMAL;
	di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;
	di->prev_soc = 0;
	di->capacity_filter_count = 0;
	di->disable_pre_vol_check = 0;
	di->chg_force_full_count = 0;
	di->chg_force_full_soc_thld = BUC_CHG_FORCE_FULL_SOC_THRESHOLD;
	di->chg_force_full_wait_time = BUC_CHG_FORCE_FULL_TIME;

	capacity = coul_interface_get_battery_last_capacity(COUL_TYPE_MAIN);
	di->ui_capacity = capacity;
	di->ui_prev_capacity = capacity;
	bat_ui_capacity_reset_capacity_fifo(di, capacity);
	hwlog_info("capacity_filter = %d\n", capacity);
}

static void bat_ui_vth_correct_dts(struct bat_ui_capacity_device *di)
{
	struct device_node *np = di->dev->of_node;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vth_correct_en", &di->vth_correct_en, 0);

	if (!di->vth_correct_en)
		return;
	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"vth_correct_para",
		(u32 *)&di->vth_soc_calibration_data[0],
		sizeof(di->vth_soc_calibration_data) / sizeof(int)))
		di->vth_correct_en = 0;
}

static void bat_ui_capacity_dts_init(struct bat_ui_capacity_device *di)
{
	struct device_node *np = di->dev->of_node;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"soc_at_term", &di->soc_at_term, BUC_CAPACITY_DIVISOR);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ui_cap_zero_offset", &di->ui_cap_zero_offset, 0);
	bat_ui_vth_correct_dts(di);
}

static int bat_ui_capacity_probe(struct platform_device *pdev)
{
	int ret;
	struct bat_ui_capacity_device *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;
	g_bat_ui_capacity_dev = di;

	di->dev = &pdev->dev;
	bat_ui_capacity_dts_init(di);
	bat_ui_capacity_data_init(di);
	di->event_nb.notifier_call = bat_ui_capacity_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_CHARGING, &di->event_nb);
	if (ret)
		goto fail_free_mem;

	INIT_DELAYED_WORK(&di->update_work, bat_ui_capacity_work);
	queue_delayed_work(system_power_efficient_wq, &di->update_work, 0);
	platform_set_drvdata(pdev, di);

	return 0;

fail_free_mem:
	kfree(di);
	g_bat_ui_capacity_dev = NULL;
	return ret;
}

static int bat_ui_capacity_remove(struct platform_device *pdev)
{
	struct bat_ui_capacity_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	platform_set_drvdata(pdev, NULL);
	cancel_delayed_work(&di->update_work);
	power_event_bnc_unregister(POWER_BNT_CHARGING, &di->event_nb);
	kfree(di);
	g_bat_ui_capacity_dev = NULL;
	return 0;
}

#ifdef CONFIG_PM
static int bat_ui_capacity_suspend(struct platform_device *pdev,
	pm_message_t state)
{
	struct bat_ui_capacity_device *di = platform_get_drvdata(pdev);

	if (!di)
		return 0;

	cancel_delayed_work(&di->update_work);
	return 0;
}

static int bat_ui_capacity_resume(struct platform_device *pdev)
{
	struct bat_ui_capacity_device *di = platform_get_drvdata(pdev);

	if (!di)
		return 0;

	queue_delayed_work(system_power_efficient_wq, &di->update_work, 0);
	return 0;
}
#endif /* CONFIG_PM */

static void bat_ui_capacity_shutdown(struct platform_device *pdev)
{
	struct bat_ui_capacity_device *di = platform_get_drvdata(pdev);
	int val;

	if (!di)
		return;

	val = bat_ui_capacity();
	coul_interface_set_battery_last_capacity(COUL_TYPE_MAIN, val);
}

static const struct of_device_id bat_ui_capacity_match_table[] = {
	{
		.compatible = "huawei,battery_ui_capacity",
		.data = NULL,
	},
	{},
};

static struct platform_driver bat_ui_capacity_driver = {
	.probe = bat_ui_capacity_probe,
	.remove = bat_ui_capacity_remove,
#ifdef CONFIG_PM
	.suspend = bat_ui_capacity_suspend,
	.resume = bat_ui_capacity_resume,
#endif /* CONFIG_PM */
	.shutdown = bat_ui_capacity_shutdown,
	.driver = {
		.name = "huawei,battery_ui_capacity",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bat_ui_capacity_match_table),
	},
};

static int __init bat_ui_capacity_init(void)
{
	return platform_driver_register(&bat_ui_capacity_driver);
}

static void __exit bat_ui_capacity_exit(void)
{
	platform_driver_unregister(&bat_ui_capacity_driver);
}

device_initcall_sync(bat_ui_capacity_init);
module_exit(bat_ui_capacity_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery ui capacity driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
