/*
 * huawei_charger_common.c
 *
 * common interface for huawei charger driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/power_common_sh.h>
#include <huawei_platform/power/huawei_charger_common.h>

#define HWLOG_TAG huawei_charger_common
HWLOG_REGIST();

static struct charge_extra_ops *g_extra_ops;
struct charge_device_ops *g_ops;

static inline int charge_common_check_iom3_status(void)
{
#ifdef CONFIG_INPUTHUB
	if (iom3_sr_status == ST_SLEEP) {
		hwlog_err("sensorhub is sleep\n");
		return -EINVAL;
	}
#endif /* CONFIG_INPUTHUB */

	return 0;
}

static inline int charge_common_check_iom3_rec_state(void)
{
#ifdef CONFIG_INPUTHUB
	if (atomic_read(&iom3_rec_state) == IOM3_RECOVERY_UNINIT) {
		hwlog_err("sensorhub recovery is uninit\n");
		return -EINVAL;
	}
#endif /* CONFIG_INPUTHUB */

	return 0;
}

int charge_extra_ops_register(struct charge_extra_ops *ops)
{
	int ret = 0;

	if (ops) {
		g_extra_ops = ops;
		hwlog_info("charge extra ops register ok\n");
	} else {
		hwlog_err("charge extra ops register fail\n");
		ret = -EPERM;
	}

	return ret;
}

int charge_ops_register(struct charge_device_ops *ops)
{
	int ret = 0;

	if (ops) {
		g_ops = ops;
		hwlog_info("charge ops register ok\n");
	} else {
		hwlog_err("charge ops register fail\n");
		ret = -EPERM;
	}

	return ret;
}

int get_charger_vbus_vol(void)
{
	unsigned int vbus_vol = 0;

	if (charge_common_check_iom3_status())
		return 0;

	if (charge_common_check_iom3_rec_state())
		return 0;

	if (!g_ops || !g_ops->get_vbus) {
		hwlog_err("g_charge_ops or get_vbus is null\n");
		return 0;
	}

	g_ops->get_vbus(&vbus_vol);
	return vbus_vol;
}

int get_charger_ibus_curr(void)
{
	if (charge_common_check_iom3_status())
		return -1;

	if (charge_common_check_iom3_rec_state())
		return -1;

	if (!g_ops || !g_ops->get_ibus) {
		hwlog_err("g_charge_ops or get_ibus is null\n");
		return -1;
	}

	return g_ops->get_ibus();
}

int charge_check_input_dpm_state(void)
{
	if (charge_common_check_iom3_status())
		return -1;

	if (!g_ops || !g_ops->check_input_dpm_state) {
		hwlog_err("g_charge_ops or check_input_dpm_state is null\n");
		return -1;
	}

	return g_ops->check_input_dpm_state();
}

int charge_check_charger_plugged(void)
{
	if (!g_ops || !g_ops->check_charger_plugged) {
		hwlog_err("g_charge_ops or check_charger_plugged is null\n");
		return -1;
	}

	return g_ops->check_charger_plugged();
}

bool charge_check_charger_ts(void)
{
	if (!g_extra_ops || !g_extra_ops->check_ts) {
		hwlog_err("g_extra_ops or check_ts is null\n");
		return false;
	}

	return g_extra_ops->check_ts();
}

bool charge_check_charger_otg_state(void)
{
	if (!g_extra_ops || !g_extra_ops->check_otg_state) {
		hwlog_err("g_extra_ops or check_otg_state is null\n");
		return false;
	}

	return g_extra_ops->check_otg_state();
}

enum fcp_check_stage_type fcp_get_stage_status(void)
{
	if (!g_extra_ops || !g_extra_ops->get_stage) {
		hwlog_err("g_extra_ops or get_stage is null\n");
		return FCP_STAGE_DEFAUTL;
	}

	return g_extra_ops->get_stage();
}

enum huawei_usb_charger_type charge_get_charger_type(void)
{
	if (!g_extra_ops || !g_extra_ops->get_charger_type) {
		hwlog_err("g_extra_ops or get_charger_type is null\n");
		return CHARGER_REMOVED;
	}

	return g_extra_ops->get_charger_type();
}

int charge_set_charge_state(int state)
{
	if (!g_extra_ops || !g_extra_ops->set_state) {
		hwlog_err("g_extra_ops or set_state is null\n");
		return -1;
	}

	return g_extra_ops->set_state(state);
}

int get_charge_current_max(void)
{
	if (!g_extra_ops || !g_extra_ops->get_charge_current) {
		hwlog_err("g_extra_ops or get_charge_current is null\n");
		return 0;
	}

	return g_extra_ops->get_charge_current();
}
