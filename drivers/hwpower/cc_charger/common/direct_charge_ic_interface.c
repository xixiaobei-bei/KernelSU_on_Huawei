/*
 * direct_charge_ic_interface.c
 *
 * common interface, variables, definition etc for direct charge module
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/common_module/power_calibration.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_interface.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic.h>

#define HWLOG_TAG direct_charge_ic_interface
HWLOG_REGIST();

int dc_init_ic(int mode, unsigned int index)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	hwlog_info("init ic index is %u\n", index);

	if (!temp_ops || !temp_ops->ic_init)
		return -1;
	return temp_ops->ic_init(temp_ops->dev_data);
}

int dc_exit_ic(int mode, unsigned int index)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	hwlog_info("exit ic index is %u\n", index);

	if (!temp_ops || !temp_ops->ic_exit)
		return -1;
	return temp_ops->ic_exit(temp_ops->dev_data);
}

int dc_enable_ic(int mode, unsigned int index, int enable)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	hwlog_info("enable ic index is %u, enable %d\n", index, enable);

	if (!temp_ops || !temp_ops->ic_enable)
		return -1;
	return temp_ops->ic_enable(enable, temp_ops->dev_data);
}

int dc_enable_ic_adc(int mode, unsigned int index, int enable)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	if (!temp_ops || !temp_ops->ic_adc_enable)
		return -1;
	return temp_ops->ic_adc_enable(enable, temp_ops->dev_data);
}

int dc_discharge_ic(int mode, unsigned int index, int enable)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	if (!temp_ops || !temp_ops->ic_discharge)
		return -1;
	return temp_ops->ic_discharge(enable, temp_ops->dev_data);
}

int dc_is_ic_close(int mode, unsigned int index)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	if (!temp_ops || !temp_ops->is_ic_close)
		return -1;
	return temp_ops->is_ic_close(temp_ops->dev_data);
}

bool dc_is_ic_support_prepare(int mode, unsigned int index)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	if (temp_ops && temp_ops->ic_enable_prepare)
		return true;
	return false;
}

int dc_prepare_enable_ic(int mode, unsigned int index)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	if (!temp_ops || !temp_ops->ic_enable_prepare)
		return -1;
	return temp_ops->ic_enable_prepare(temp_ops->dev_data);
}

int dc_config_ic_watchdog(int mode, unsigned int index, int time)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	if (!temp_ops || !temp_ops->config_ic_watchdog)
		return -1;
	return temp_ops->config_ic_watchdog(time, temp_ops->dev_data);
}

int dc_kick_ic_watchdog(int mode, unsigned int index)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	if (!temp_ops || !temp_ops->kick_ic_watchdog)
		return -1;
	return temp_ops->kick_ic_watchdog(temp_ops->dev_data);
}

int dc_get_ic_id(int mode, unsigned int index)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	if (temp_ops && temp_ops->get_ic_id)
		return temp_ops->get_ic_id(temp_ops->dev_data);
	return -1;
}

int dc_get_ic_status(int mode, unsigned int index)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	if (!temp_ops)
		return -1;
	if (!temp_ops->get_ic_status)
		return 0;
	return temp_ops->get_ic_status(temp_ops->dev_data);
}

int dc_set_ic_buck_enable(int mode, unsigned int index, int enable)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	if (!temp_ops || !temp_ops->set_ic_buck_enable)
		return -1;
	return temp_ops->set_ic_buck_enable(enable, temp_ops->dev_data);
}

int dc_reset_and_init_ic_reg(int mode, unsigned int index)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	if (!temp_ops || !temp_ops->ic_reg_reset_and_init)
		return -1;
	return temp_ops->ic_reg_reset_and_init(temp_ops->dev_data);
}

int dc_get_ic_freq(int mode, unsigned int index)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	if (!temp_ops || !temp_ops->get_ic_freq)
		return -1;
	return temp_ops->get_ic_freq(temp_ops->dev_data);
}

int dc_set_ic_freq(int mode, unsigned int index, int freq)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	if (!temp_ops || !temp_ops->set_ic_freq)
		return 0;
	return temp_ops->set_ic_freq(freq, temp_ops->dev_data);
}

const char *dc_get_ic_name(int mode, unsigned int index)
{
	struct dc_ic_ops *temp_ops = dc_ic_get_ic_ops(mode, index);

	if (!temp_ops || !temp_ops->dev_name)
		return "invalid ic";
	return temp_ops->dev_name;
}

int dc_init_batinfo(int mode, unsigned int index)
{
	struct dc_batinfo_ops *temp_ops = dc_ic_get_battinfo_ops(mode, index);

	if (!temp_ops || !temp_ops->init)
		return -1;
	return temp_ops->init(temp_ops->dev_data);
}

int dc_exit_batinfo(int mode, unsigned int index)
{
	struct dc_batinfo_ops *temp_ops = dc_ic_get_battinfo_ops(mode, index);

	if (!temp_ops || !temp_ops->exit)
		return -1;
	return temp_ops->exit(temp_ops->dev_data);
}

int dc_get_ic_vbtb(int mode, unsigned int index)
{
	struct dc_batinfo_ops *temp_ops = dc_ic_get_battinfo_ops(mode, index);

	if (!temp_ops || !temp_ops->get_bat_btb_voltage)
		return -1;
	return temp_ops->get_bat_btb_voltage(temp_ops->dev_data);
}

int dc_get_ic_vpack(int mode, unsigned int index)
{
	struct dc_batinfo_ops *temp_ops = dc_ic_get_battinfo_ops(mode, index);

	if (!temp_ops || !temp_ops->get_bat_package_voltage)
		return -1;
	return temp_ops->get_bat_package_voltage(temp_ops->dev_data);
}

int dc_get_ic_vbus(int mode, unsigned int index, int *vbus)
{
	struct dc_batinfo_ops *temp_ops = dc_ic_get_battinfo_ops(mode, index);

	if (!vbus)
		return -1;

	if (!temp_ops || !temp_ops->get_vbus_voltage)
		return -1;
	return temp_ops->get_vbus_voltage(vbus, temp_ops->dev_data);
}

int dc_get_ic_ibat(int mode, unsigned int index, int *ibat)
{
	s64 temp;
	int c_offset_a = 0;
	int c_offset_b = 0;
	struct dc_batinfo_ops *temp_ops = dc_ic_get_battinfo_ops(mode, index);

	if (!ibat)
		return -1;

	if (!temp_ops || !temp_ops->get_bat_current ||
		temp_ops->get_bat_current(ibat, temp_ops->dev_data))
		return -1;

	hwlog_info("ic[%u] cali_b: bat_cur=%d\n", index, *ibat);
	switch (index) {
	case IC_ONE:
		/* 0:CUR_A_OFFSET 1:CUR_B_OFFSET */
		power_cali_common_get_data("dc", 0, &c_offset_a);
		power_cali_common_get_data("dc", 1, &c_offset_b);
		break;
	case IC_TWO:
		/* 2:AUX_CUR_A_OFFSET 3:AUX_CUR_B_OFFSET */
		power_cali_common_get_data("dc", 2, &c_offset_a);
		power_cali_common_get_data("dc", 3, &c_offset_b);
		break;
	default:
		break;
	}

	if (c_offset_a) {
		temp = (*ibat * (s64)c_offset_a) + c_offset_b;
		*ibat = (int)div_s64(temp, 1000000); /* base 1000000 */
		hwlog_info("cali_a: bat_cur=%d,c_offset_a=%d,c_offset_b=%d\n",
			*ibat, c_offset_a, c_offset_b);
	}
	return 0;
}

int dc_get_ic_ibus(int mode, unsigned int index, int *ibus)
{
	struct dc_batinfo_ops *temp_ops = dc_ic_get_battinfo_ops(mode, index);

	if (!ibus)
		return -1;

	if (!temp_ops || !temp_ops->get_ic_ibus)
		return -1;
	return temp_ops->get_ic_ibus(ibus, temp_ops->dev_data);
}

int dc_get_ic_temp(int mode, unsigned int index, int *temp)
{
	struct dc_batinfo_ops *temp_ops = dc_ic_get_battinfo_ops(mode, index);

	if (!temp)
		return -1;

	if (!temp_ops || !temp_ops->get_ic_temp)
		return 0;
	return temp_ops->get_ic_temp(temp, temp_ops->dev_data);
}

int dc_get_ic_vusb(int mode, unsigned int index, int *vusb)
{
	struct dc_batinfo_ops *temp_ops = dc_ic_get_battinfo_ops(mode, index);

	if (!vusb)
		return -1;

	if (!temp_ops || !temp_ops->get_ic_vusb)
		return -1;
	return temp_ops->get_ic_vusb(vusb, temp_ops->dev_data);
}

int dc_get_ic_vout(int mode, unsigned int index, int *vout)
{
	struct dc_batinfo_ops *temp_ops = dc_ic_get_battinfo_ops(mode, index);

	if (!vout)
		return -1;

	if (!temp_ops || !temp_ops->get_ic_vout)
		return -1;
	return temp_ops->get_ic_vout(vout, temp_ops->dev_data);
}
