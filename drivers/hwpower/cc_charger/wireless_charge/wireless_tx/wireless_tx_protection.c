/*
 * wireless_tx_protection.c
 *
 * tx protection for wireless reverse charging
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

#include <linux/slab.h>
#include <linux/workqueue.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/protocol/wireless_protocol.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_protection.h>

#define HWLOG_TAG wireless_tx_protect
HWLOG_REGIST();

static struct wltx_protect_dev_info *g_tx_protect_di;

static void wltx_monitor_tbatt_alarm(struct wltx_protect_dev_info *di)
{
	int i;
	int tbatt = 0;
	static int last_i;
	int cur_i = last_i;

	if (di->cfg.tbatt_alarm_level <= 0)
		return;

	bat_temp_get_temperature(BAT_TEMP_MIXED, &tbatt);
	for (i = 0; di->cfg.tbatt_alarm_level; i++) {
		if ((tbatt < di->cfg.tbatt_alarm[i].tbatt_lth) ||
			(tbatt >= di->cfg.tbatt_alarm[i].tbatt_hth))
			continue;
		if (di->first_monitor || (last_i < i) ||
			((di->cfg.tbatt_alarm[i].tbatt_hth - tbatt) >=
			di->cfg.tbatt_alarm[i].tbatt_back))
			cur_i = i;
		else
			cur_i = last_i;
		break;
	}
	if (i >= di->cfg.tbatt_alarm_level)
		return;

	last_i = cur_i;
	wltx_update_alarm_data(&di->cfg.tbatt_alarm[i].alarm, TX_ALARM_SRC_TEMP);
}

static void wltx_protection_monitor(struct wltx_protect_dev_info *di)
{
	wltx_monitor_tbatt_alarm(di);
}

static void wltx_protection_monitor_work(struct work_struct *work)
{
	struct wltx_protect_dev_info *di = container_of(work,
		struct wltx_protect_dev_info, mon_work.work);

	if (!di || !di->need_monitor)
		return;

	wltx_protection_monitor(di);
	di->first_monitor = false;

	schedule_delayed_work(&di->mon_work,
		msecs_to_jiffies(WLTX_PROTECT_MON_INTERVAL));
}

void wltx_start_monitor_protection(struct wltx_protect_cfg *cfg,
	const unsigned int delay)
{
	struct wltx_protect_dev_info *di = g_tx_protect_di;

	if (!di || !cfg)
		return;

	hwlog_info("start monitor tx_protection\n");

	if (delayed_work_pending(&di->mon_work))
		cancel_delayed_work_sync(&di->mon_work);
	di->need_monitor = true;
	di->first_monitor = true;
	memcpy(&di->cfg, cfg, sizeof(di->cfg));
	schedule_delayed_work(&di->mon_work, msecs_to_jiffies(delay));
}

void wltx_stop_monitor_protection(void)
{
	struct wltx_protect_dev_info *di = g_tx_protect_di;

	if (!di)
		return;

	hwlog_info("stop monitor tx_protection\n");
	di->need_monitor = false;
}

static int __init wltx_protection_init(void)
{
	struct wltx_protect_dev_info *di = NULL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_tx_protect_di = di;

	INIT_DELAYED_WORK(&di->mon_work, wltx_protection_monitor_work);
	return 0;
}

static void __exit wltx_protection_exit(void)
{
	struct wltx_protect_dev_info *di = g_tx_protect_di;

	kfree(di);
	g_tx_protect_di = NULL;
}

device_initcall(wltx_protection_init);
module_exit(wltx_protection_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless tx protection driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
