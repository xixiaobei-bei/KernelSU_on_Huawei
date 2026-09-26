/*
 * sc200x_tcpc.c
 *
 * sc200x tcpc driver
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

#include "sc200x_tcpc.h"
#include "sc200x_i2c.h"
#include <huawei_platform/hwpower/common_module/power_platform.h>

#define HWLOG_TAG sc200x
HWLOG_REGIST();

static int sc200x_notify_tcpc_vbus(unsigned long event, int mv, int ma)
{
	struct pd_dpm_vbus_state vbus_state = { 0 };

	if (mv < 0)
		mv = SC200X_TYPEC_VBUS_DEFAULT;
	if (ma < 0)
		 ma = SC200X_TYPEC_IBUS_DEFAULT;

	vbus_state.ma = ma;
	vbus_state.mv = mv;
	vbus_state.vbus_type = TCP_VBUS_CTRL_TYPEC;
	if (pd_dpm_handle_pe_event(event, (void *)&vbus_state))
		return SC200X_FAILED;

	return SC200X_SUCCESS;
}

static inline int sc200x_notify_tcpc_source_vbus(int mv, int ma)
{
	return sc200x_notify_tcpc_vbus(PD_DPM_PE_EVT_SOURCE_VBUS, mv, ma);
}

static inline int sc200x_notify_tcpc_sink_vbus(int mv, int ma)
{
	return sc200x_notify_tcpc_vbus(PD_DPM_PE_EVT_SINK_VBUS, mv, ma);
}

static int sc200x_notify_tcpc_state(bool polarity, int new_state)
{
	struct pd_dpm_typec_state typec_state = { 0 };

	typec_state.polarity = polarity;
	typec_state.new_state = new_state;
	pd_dpm_handle_pe_event(PD_DPM_PE_EVT_TYPEC_STATE, &typec_state);

	return SC200X_SUCCESS;
}

static void sc200x_device_detached(struct sc200x_device_info *di)
{
	sc200x_notify_tcpc_state(true, PD_DPM_TYPEC_UNATTACHED);
	if (di->power_role == SC200X_TCPC_PR_SINK)
		sc200x_notify_tcpc_sink_vbus(0, 0); /* 0 mV, 0 mA */
	else if (di->power_role == SC200X_TCPC_PR_SOURCE)
		sc200x_notify_tcpc_source_vbus(0, 0); /* 0 mV, 0 mA */
	di->power_role = SC200X_TCPC_PR_UNKNOWN;
}

static void sc200x_source_attached(struct sc200x_device_info *di,
	int mv, int ma)
{
	di->power_role = SC200X_TCPC_PR_SOURCE;
	sc200x_notify_tcpc_state(true, PD_DPM_TYPEC_ATTACHED_SRC);
	sc200x_notify_tcpc_source_vbus(mv, ma);
	hwlog_info("source_atteched\n");
}

static void sc200x_sink_attached(struct sc200x_device_info *di,
	int mv, int ma)
{
	di->power_role = SC200X_TCPC_PR_SINK;
	sc200x_notify_tcpc_state(true, PD_DPM_TYPEC_ATTACHED_SNK);
	sc200x_notify_tcpc_sink_vbus(mv, ma);
	hwlog_info("sink_atteched\n");
}

static int sc200x_get_typec_current(int val)
{
	int cur;

	switch (val) {
	case SC200X_RP_VAL_DEFAULT:
		cur = SC200X_TYPEC_IBUS_DEFAULT;
		break;
	case SC200X_RP_VAL_MIDDLE:
		cur = SC200X_TYPEC_IBUS_MIDDLE;
		break;
	case SC200X_RP_VAL_HIGH:
		cur = SC200X_TYPEC_IBUS_HIGH;
		break;
	default:
		cur = 0;
		break;
	}

	return cur;
}

static int sc200x_get_tcpc_current(int cc1, int cc2)
{
	if (cc1)
		return sc200x_get_typec_current(cc1);
	else
		return sc200x_get_typec_current(cc2);
}

int sc200x_cc_state_alert_handler(struct sc200x_device_info *di,
	unsigned int alert)
{
	int val;
	int role;
	int attach;
	int cc1;
	int cc2;
	int cur;

	if (!di)
		return SC200X_FAILED;

	val = sc200x_read_reg(di, SC200X_REG_CC_STATUS);
	if (val < 0)
		return SC200X_FAILED;
	hwlog_info("cc_status=%x\n", val);

	attach = (val & SC200X_ATTACH_FLAG_MASK) >> SC200X_ATTACH_FLAG_SHIFT;
	if (!attach) {
		sc200x_device_detached(di);
		return SC200X_SUCCESS;
	}

	cc1 = (val & SC200X_CC1_STATUS_MASK) >> SC200X_CC1_STATUS_SHIFT;
	cc2 = (val & SC200X_CC2_STATUS_MASK) >> SC200X_CC2_STATUS_SHIFT;
	role = (val & SC200X_CC_ROLE_MASK) >> SC200X_CC_ROLE_SHIFT;
	if (!role) { /* tcpc as sink */
		cur = sc200x_get_tcpc_current(cc1, cc2);
		sc200x_sink_attached(di, SC200X_TYPEC_VBUS_DEFAULT, cur);
	} else { /* tcpc as source */
		cur = sc200x_get_tcpc_current(di->rp_value, di->rp_value);
		sc200x_source_attached(di, SC200X_TYPEC_VBUS_DEFAULT, cur);
	}

	return SC200X_SUCCESS;
}

int sc200x_tcpc_init(struct sc200x_device_info *di)
{
	int ret;
	unsigned int val;

	if (!di)
		 return -ENODEV;

	di->power_role = SC200X_TCPC_PR_UNKNOWN;
	if (di->src_enable) {
		val = SC200X_SRC_EN_MASK;
		val |= di->rp_value & SC200X_RP_VALUE_MASK;
		ret = sc200x_write_reg(di, SC200X_REG_ROLE_CTRL, (int)val);
		if (ret < 0)
			return SC200X_FAILED;
	}

	sc200x_cc_state_alert_handler(di, SC200X_ALERT_CC_STATUS_MASK);

	return SC200X_SUCCESS;
}
