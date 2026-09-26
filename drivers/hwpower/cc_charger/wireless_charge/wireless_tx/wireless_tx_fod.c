/*
 * wireless_tx_fod.c
 *
 * tx fod for wireless reverse charging
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

#include <linux/workqueue.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/protocol/wireless_protocol.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_fod.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>

#define HWLOG_TAG wireless_tx_fod
HWLOG_REGIST();

static struct wltx_fod_dev_info *g_tx_fod_di;

static int wltx_get_fod_alarm_id(struct wltx_fod_dev_info *di)
{
	int ret;
	int i;
	s32 ploss = 0;
	u8 ploss_id = 0;

	ret = wltx_ic_get_ploss(WLTRX_IC_MAIN, &ploss);
	ret += wltx_ic_get_ploss_id(WLTRX_IC_MAIN, &ploss_id);
	if (ret)
		return di->cfg.fod_alarm_level;

	for (i = 0; i < di->cfg.fod_alarm_level; i++) {
		if (ploss_id != di->cfg.fod_alarm[i].ploss_id)
			continue;
		if ((ploss <= di->cfg.fod_alarm[i].ploss_lth) ||
			(ploss >= di->cfg.fod_alarm[i].ploss_hth))
			continue;
		break;
	}

	return i;
}

static void wltx_monitor_fod_alarm(struct wltx_fod_dev_info *di)
{
	int cur_id;
	static unsigned long time_out;

	if (di->cfg.fod_alarm_level <= 0)
		return;

	cur_id = wltx_get_fod_alarm_id(di);
	if ((cur_id < 0) || (cur_id >= di->cfg.fod_alarm_level))
		return;

	if (di->alarm_id != cur_id)
		time_out = jiffies + msecs_to_jiffies(WLTX_FOD_MON_DEBOUNCE);
	di->alarm_id = cur_id;
	if (!time_after(jiffies, time_out))
		return;

	wltx_update_alarm_data(&di->cfg.fod_alarm[cur_id].alarm, TX_ALARM_SRC_FOD);
	di->mon_delay = di->cfg.fod_alarm[cur_id].delay;
}

static void wltx_fod_monitor(struct wltx_fod_dev_info *di)
{
	wltx_monitor_fod_alarm(di);
}

static void wltx_fod_monitor_work(struct work_struct *work)
{
	struct wltx_fod_dev_info *di = container_of(work,
		struct wltx_fod_dev_info, mon_work.work);

	if (!di || !di->need_monitor)
		return;

	wltx_fod_monitor(di);

	if (di->mon_delay > 0) {
		schedule_delayed_work(&di->mon_work,
			msecs_to_jiffies(di->mon_delay));
		di->mon_delay = 0;
		return;
	}

	schedule_delayed_work(&di->mon_work,
		msecs_to_jiffies(WLTX_FOD_MON_INTERVAL));
}

void wltx_start_monitor_fod(struct wltx_fod_cfg *cfg, const unsigned int delay)
{
	struct wltx_fod_dev_info *di = g_tx_fod_di;

	if (!di || !cfg)
		return;

	hwlog_info("start monitor fod_alarm\n");
	if (delayed_work_pending(&di->mon_work))
		cancel_delayed_work_sync(&di->mon_work);
	di->need_monitor = true;
	di->mon_delay = 0;
	di->alarm_id = -1;
	memcpy(&di->cfg, cfg, sizeof(di->cfg));
	schedule_delayed_work(&di->mon_work, msecs_to_jiffies(delay));
}

void wltx_stop_monitor_fod(void)
{
	struct wltx_fod_dev_info *di = g_tx_fod_di;

	if (!di)
		return;

	hwlog_info("stop monitor fod\n");
	di->need_monitor = false;
}

static int __init wltx_fod_init(void)
{
	struct wltx_fod_dev_info *di = NULL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_tx_fod_di = di;

	INIT_DELAYED_WORK(&di->mon_work, wltx_fod_monitor_work);
	return 0;
}

static void __exit wltx_fod_exit(void)
{
	struct wltx_fod_dev_info *di = g_tx_fod_di;

	kfree(di);
	g_tx_fod_di = NULL;
}

device_initcall(wltx_fod_init);
module_exit(wltx_fod_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless tx fod driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
