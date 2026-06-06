/*
 * charging_core_debug.c
 *
 * debug for charging core module
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/raid/pq.h>
#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif
#include <charging_core.h>
#ifdef CONFIG_TCPC_CLASS
#include <huawei_platform/usb/hw_pd_dev.h>
#endif
#include <huawei_platform/power/battery_voltage.h>
#include <huawei_platform/power/huawei_battery_temp.h>
#include <huawei_platform/power/power_debug.h>

#define HWLOG_TAG charging_core_debug
HWLOG_REGIST();

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static ssize_t charging_core_dbg_iterm_store(void *dev_data,
	const char *buf, size_t size)
{
	int iterm = 0;
	int ret;
	struct charge_core_info *di = NULL;

	di = (struct charge_core_info *)dev_data;
	if (!di)
		return -EINVAL;

	ret = kstrtoint(buf, 0, &iterm);
	if (ret) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	di->data.iterm = iterm;

	return size;
}

static ssize_t charging_core_dbg_iterm_show(void *dev_data,
	char *buf, size_t size)
{
	struct charge_core_info *di = NULL;

	di = (struct charge_core_info *)dev_data;
	if (!di)
		return scnprintf(buf, size, "not support\n");

	return scnprintf(buf, size, "iterm is %d\n", di->data.iterm);
}

static ssize_t charging_core_dbg_ichg_ac_store(void *dev_data,
	const char *buf, size_t size)
{
	int ichg = 0;
	int ret;
	struct charge_core_info *di = NULL;

	di = (struct charge_core_info *)dev_data;
	if (!di)
		return -EINVAL;

	ret = kstrtoint(buf, 0, &ichg);
	if (ret) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	di->data.ichg_ac = ichg;

	return size;
}

static ssize_t charging_core_dbg_ichg_ac_show(void *dev_data,
	char *buf, size_t size)
{
	struct charge_core_info *di = NULL;

	di = (struct charge_core_info *)dev_data;
	if (!di)
		return scnprintf(buf, size, "not support\n");

	return scnprintf(buf, size, "ichg_ac is %d\n", di->data.ichg_ac);
}

static ssize_t charging_core_dbg_iin_ac_store(void *dev_data,
	const char *buf, size_t size)
{
	int iin = 0;
	int ret;
	struct charge_core_info *di = NULL;

	di = (struct charge_core_info *)dev_data;
	if (!di)
		return -EINVAL;

	ret = kstrtoint(buf, 0, &iin);
	if (ret) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	di->data.iin_ac = iin;

	return size;
}

static ssize_t charging_core_dbg_iin_ac_show(void *dev_data,
	char *buf, size_t size)
{
	struct charge_core_info *di = NULL;

	di = (struct charge_core_info *)dev_data;
	if (!di)
		return scnprintf(buf, size, "not support\n");

	return scnprintf(buf, size, "iin_ac is %d\n", di->data.iin_ac);
}

static ssize_t charging_core_dbg_ichg_fcp_store(void *dev_data,
	const char *buf, size_t size)
{
	int ichg_fcp = 0;
	int ret;
	struct charge_core_info *di = NULL;

	di = (struct charge_core_info *)dev_data;
	if (!di)
		return -EINVAL;

	ret = kstrtoint(buf, 0, &ichg_fcp);
	if (ret) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	di->data.ichg_fcp = ichg_fcp;

	return size;
}

static ssize_t charging_core_dbg_ichg_fcp_show(void *dev_data,
	char *buf, size_t size)
{
	struct charge_core_info *di = NULL;

	di = (struct charge_core_info *)dev_data;
	if (!di)
		return scnprintf(buf, size, "not support\n");

	return scnprintf(buf, size, "ichg_fcp is %d\n", di->data.ichg_fcp);
}

static ssize_t charging_core_dbg_iin_fcp_store(void *dev_data,
	const char *buf, size_t size)
{
	int iin_fcp = 0;
	int ret;
	struct charge_core_info *di = NULL;

	di = (struct charge_core_info *)dev_data;
	if (!di)
		return -EINVAL;

	ret = kstrtoint(buf, 0, &iin_fcp);
	if (ret) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	di->data.iin_fcp = iin_fcp;

	return size;
}

static ssize_t charging_core_dbg_iin_fcp_show(void *dev_data,
	char *buf, size_t size)
{
	struct charge_core_info *di = NULL;

	di = (struct charge_core_info *)dev_data;
	if (!di)
		return scnprintf(buf, size, "not support\n");

	return scnprintf(buf, size, "iin_fcp is %d\n", di->data.iin_fcp);
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

void charging_core_dbg_register(struct charge_core_info *di)
{
#ifdef CONFIG_HUAWEI_POWER_DEBUG
	power_dbg_ops_register("iterm", (void *)di,
		(power_dbg_show)charging_core_dbg_iterm_show,
		(power_dbg_store)charging_core_dbg_iterm_store);
	power_dbg_ops_register("ichg_ac", (void *)di,
		(power_dbg_show)charging_core_dbg_ichg_ac_show,
		(power_dbg_store)charging_core_dbg_ichg_ac_store);
	power_dbg_ops_register("iin_ac", (void *)di,
		(power_dbg_show)charging_core_dbg_iin_ac_show,
		(power_dbg_store)charging_core_dbg_iin_ac_store);
	power_dbg_ops_register("ichg_fcp", (void *)di,
		(power_dbg_show)charging_core_dbg_ichg_fcp_show,
		(power_dbg_store)charging_core_dbg_ichg_fcp_store);
	power_dbg_ops_register("iin_fcp", (void *)di,
		(power_dbg_show)charging_core_dbg_iin_fcp_show,
		(power_dbg_store)charging_core_dbg_iin_fcp_store);
#else
	return;
#endif /* CONFIG_HUAWEI_POWER_DEBUG */
}
