/*
 * power_wakeup.c
 *
 * wakeup interface for power module
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

#include <chipset_common/hwpower/common_module/power_wakeup.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_wakeup
HWLOG_REGIST();

struct wakeup_source *power_wakeup_source_register(struct device *dev,
	const char *name)
{
	struct wakeup_source *ws = NULL;

	if (!dev || !name) {
		hwlog_err("dev or name is null\n");
		return NULL;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	ws = wakeup_source_register(dev, name);
#else
	ws = wakeup_source_register(name);
#endif
	if (!ws) {
		hwlog_err("wakeup source %s register fail\n", name);
		return NULL;
	}

	hwlog_info("wakeup source %s register ok\n", name);
	return ws;
}

void power_wakeup_source_unregister(struct wakeup_source *ws)
{
	if (!ws || !ws->name) {
		hwlog_err("ws or name is null\n");
		return;
	}

	hwlog_info("wakeup source %s unregister ok\n", ws->name);
	wakeup_source_unregister(ws);
}

void power_wakeup_lock(struct wakeup_source *ws)
{
	if (!ws || !ws->name) {
		hwlog_err("ws or name is null\n");
		return;
	}

	if (!ws->active) {
		hwlog_info("wakeup source %s lock\n", ws->name);
		__pm_stay_awake(ws);
	}
}

void power_wakeup_unlock(struct wakeup_source *ws)
{
	if (!ws || !ws->name) {
		hwlog_err("ws or name is null\n");
		return;
	}

	if (ws->active) {
		hwlog_info("wakeup source %s unlock\n", ws->name);
		__pm_relax(ws);
	}
}
