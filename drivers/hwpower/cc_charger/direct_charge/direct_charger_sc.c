/*
 * direct_charger_sc.c
 *
 * direct charger with sc (switch cap) driver
 *
 * Copyright (c) 2012-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_debug.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_work.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <huawei_platform/usb/hw_pd_dev.h>

#define HWLOG_TAG direct_charge_sc
HWLOG_REGIST();

static struct direct_charge_device *g_sc_di;

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
static void sc_get_devices_info_by_type(struct direct_charge_device *di,
	int type, int dev_check_type)
{
	int ret;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!di)
		return;

	ret = dcm_init_ic(SC_MODE, type);
	di->ls_id = dcm_get_ic_id(SC_MODE, type);
	ret += dcm_exit_ic(SC_MODE, type);
	if ((di->ls_id < DC_DEVICE_ID_BEGIN) || (di->ls_id >= DC_DEVICE_ID_END))
		di->ls_id = DC_DEVICE_ID_END;
	di->ls_name = dc_get_device_name(di->ls_id);
	hwlog_info("switchcap id=%d, %s\n", di->ls_id, di->ls_name);
	if (ret == 0)
		set_hw_dev_flag(dev_check_type);
	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->ls_name;
		power_dev_info->dev_id = di->ls_id;
		power_dev_info->ver_id = 0;
	}
}

static void sc_get_devices_info(struct direct_charge_device *di)
{
	sc_get_devices_info_by_type(di, CHARGE_IC_MAIN, DEV_I2C_SWITCHCAP);
	if (di->multi_ic_mode_para.support_multi_ic)
		sc_get_devices_info_by_type(di, CHARGE_IC_AUX, DEV_I2C_SC_AUX);
}
#else
static void sc_get_devices_info(struct direct_charge_device *di)
{
}
#endif /* CONFIG_HUAWEI_HW_DEV_DCT */

static int sc_set_enable_charger(unsigned int val)
{
	struct direct_charge_device *di = g_sc_di;
	int ret;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	/* must be 0 or 1, 0: disable, 1: enable */
	if ((val < 0) || (val > 1))
		return -1;

	ret = sc_set_disable_flags((val ?
		DC_CLEAR_DISABLE_FLAGS : DC_SET_DISABLE_FLAGS),
		DC_DISABLE_SYS_NODE);
	hwlog_info("set enable_charger=%d\n", di->sysfs_enable_charger);
	return ret;
}

static int sc_get_enable_charger(unsigned int *val)
{
	struct direct_charge_device *di = g_sc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->sysfs_enable_charger;
	return 0;
}

static int mainsc_set_enable_charger(unsigned int val)
{
	struct direct_charge_device *di = g_sc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	/* must be 0 or 1, 0: disable, 1: enable */
	if ((val < 0) || (val > 1))
		return -1;

	di->sysfs_mainsc_enable_charger = val;
	hwlog_info("set mainsc enable_charger=%d\n",
		di->sysfs_mainsc_enable_charger);
	return 0;
}

static int mainsc_get_enable_charger(unsigned int *val)
{
	struct direct_charge_device *di = g_sc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->sysfs_mainsc_enable_charger;
	return 0;
}

static int auxsc_set_enable_charger(unsigned int val)
{
	struct direct_charge_device *di = g_sc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	/* must be 0 or 1, 0: disable, 1: enable */
	if ((val < 0) || (val > 1))
		return -1;

	di->sysfs_auxsc_enable_charger = val;
	hwlog_info("set auxsc enable_charger=%d\n",
		di->sysfs_auxsc_enable_charger);
	return 0;
}

static int auxsc_get_enable_charger(unsigned int *val)
{
	struct direct_charge_device *di = g_sc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->sysfs_auxsc_enable_charger;
	return 0;
}

static int sc_set_iin_limit(unsigned int val)
{
	struct direct_charge_device *di = g_sc_di;
	int index;
	int cur_low;
	unsigned int idx;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (val > di->iin_thermal_default) {
		hwlog_err("val is too large: %u, tuned as default\n", val);
		val = di->iin_thermal_default;
	}

	if ((di->stage_size < 1) || (di->stage_size > DC_VOLT_LEVEL)) {
		hwlog_err("invalid stage_size=%d\n", di->stage_size);
		return -1;
	}

	index = di->stage_size - 1;
	cur_low = di->orig_volt_para_p[0].volt_info[index].cur_th_low;
	idx = (di->cur_mode == CHARGE_MULTI_IC) ? DC_DUAL_CHANNEL : DC_SINGLE_CHANNEL;
	if (val == 0)
		di->sysfs_iin_thermal_array[idx] = di->iin_thermal_default;
	else if (val < cur_low)
		di->sysfs_iin_thermal_array[idx] = cur_low;
	else
		di->sysfs_iin_thermal_array[idx] = val;

	hwlog_info("set input current: %u, limit current: %d, current channel type: %u\n",
		val, di->sysfs_iin_thermal_array[idx], idx);
	return 0;
}

static int sc_set_iin_limit_array(unsigned int idx, unsigned int val)
{
	struct direct_charge_device *di = g_sc_di;
	int index;
	int cur_low;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (val > di->iin_thermal_default) {
		hwlog_err("val is too large: %u, tuned as default\n", val);
		val = di->iin_thermal_default;
	}

	if ((di->stage_size < 1) || (di->stage_size > DC_VOLT_LEVEL)) {
		hwlog_err("invalid stage_size=%d\n", di->stage_size);
		return -1;
	}

	index = di->stage_size - 1;
	cur_low = di->orig_volt_para_p[0].volt_info[index].cur_th_low;

	if (val == 0)
		di->sysfs_iin_thermal_array[idx] = di->iin_thermal_default;
	else if (val < cur_low)
		di->sysfs_iin_thermal_array[idx] = cur_low;
	else
		di->sysfs_iin_thermal_array[idx] = val;
	di->sysfs_iin_thermal = di->sysfs_iin_thermal_array[idx];

	hwlog_info("set input current: %u, limit current: %d, channel type: %u\n",
		val, di->sysfs_iin_thermal_array[idx], idx);
	return 0;
}

static int sc_set_iin_limit_ichg_control(unsigned int val)
{
	struct direct_charge_device *di = g_sc_di;
	int index;
	int cur_low;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (val > di->iin_thermal_default) {
		hwlog_err("val is too large: %u, tuned as default\n", val);
		return -1;
	}

	if ((di->stage_size < 1) || (di->stage_size > DC_VOLT_LEVEL)) {
		hwlog_err("invalid stage_size=%d\n", di->stage_size);
		return -1;
	}

	index = di->stage_size - 1;
	cur_low = di->orig_volt_para_p[0].volt_info[index].cur_th_low;

	if (val == 0)
		di->sysfs_iin_thermal_ichg_control = di->iin_thermal_default;
	else if (val < cur_low)
		di->sysfs_iin_thermal_ichg_control = cur_low;
	else
		di->sysfs_iin_thermal_ichg_control = val;

	hwlog_info("ichg_control set input current: %u, limit current: %d\n",
		val, di->sysfs_iin_thermal_ichg_control);
	return 0;
}

static int sc_get_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_sc_di;
	int idx;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -1;
	}

	idx = (di->cur_mode == CHARGE_MULTI_IC) ? DC_DUAL_CHANNEL : DC_SINGLE_CHANNEL;
	*val = di->sysfs_iin_thermal_array[idx];
	return 0;
}

static int sc_set_iin_thermal(unsigned int index, unsigned int iin_thermal_value)
{
	if (index >= DC_CHANNEL_TYPE_END) {
		hwlog_err("error index: %u, out of boundary\n", index);
		return -1;
	}
	return sc_set_iin_limit_array(index, iin_thermal_value);
}

static int sc_set_iin_thermal_all(unsigned int value)
{
	int i;

	for (i = DC_CHANNEL_TYPE_BEGIN; i < DC_CHANNEL_TYPE_END; i++) {
		if (sc_set_iin_limit_array(i, value))
			return -1;
	}
	return 0;
}

static int sc_set_ichg_ratio(unsigned int val)
{
	struct direct_charge_device *di = g_sc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	di->ichg_ratio = val;
	hwlog_info("set ichg_ratio=%u\n", val);
	return 0;
}

static int sc_set_vterm_dec(unsigned int val)
{
	struct direct_charge_device *di = g_sc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	di->vterm_dec = val;
	hwlog_info("set vterm_dec=%u\n", val);
	return 0;
}

static int sc_get_rt_test_time_by_mode(unsigned int *val, int mode)
{
	struct direct_charge_device *di = g_sc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->rt_test_para[mode].rt_test_time;
	return 0;
}

static int sc_get_rt_test_time(unsigned int *val)
{
	return sc_get_rt_test_time_by_mode(val, DC_NORMAL_MODE);
}

static int mainsc_get_rt_test_time(unsigned int *val)
{
	return sc_get_rt_test_time_by_mode(val, DC_CHAN1_MODE);
}

static int auxsc_get_rt_test_time(unsigned int *val)
{
	return sc_get_rt_test_time_by_mode(val, DC_CHAN2_MODE);
}

static int sc_get_rt_test_result_by_mode(unsigned int *val, int mode)
{
	int iin_thermal_th;
	struct direct_charge_device *di = g_sc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	iin_thermal_th = di->rt_test_para[mode].rt_curr_th + 500; /* margin is 500mA */
	if (pd_dpm_get_cc_moisture_status() || di->bat_temp_err_flag ||
		di->rt_test_para[mode].rt_test_result ||
		(di->sysfs_enable_charger == 0) ||
		((direct_charge_get_stage_status() == DC_STAGE_CHARGING) &&
		(di->sysfs_iin_thermal < iin_thermal_th)) ||
		direct_charge_check_charge_done())
		*val = 0; /* 0: succ */
	else
		*val = 1; /* 1: fail */

	return 0;
}

static int sc_get_rt_test_result(unsigned int *val)
{
	return sc_get_rt_test_result_by_mode(val, DC_NORMAL_MODE);
}

static int mainsc_get_rt_test_result(unsigned int *val)
{
	return sc_get_rt_test_result_by_mode(val, DC_CHAN1_MODE);
}

static int auxsc_get_rt_test_result(unsigned int *val)
{
	return sc_get_rt_test_result_by_mode(val, DC_CHAN2_MODE);
}

static int sc_get_hota_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_sc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->hota_iin_limit;
	return 0;
}

static int sc_get_startup_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_sc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->startup_iin_limit;
	return 0;
}

static int sc_get_multi_cur(char *buf, unsigned int len)
{
	int ret;
	int i = 0;
	char temp_buf[DC_SC_CUR_LEN] = {0};
	struct direct_charge_device *di = g_sc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di || (di->multi_ic_mode_para.support_multi_ic == 0)) {
		hwlog_err("di is null or not support multi sc\n");
		return 0;
	}

	ret = snprintf(buf, len, "%d,%d", di->curr_info.channel_num,
		di->curr_info.ibat_max);
	while ((ret > 0) && (i < di->curr_info.channel_num)) {
		memset(temp_buf, 0, sizeof(temp_buf));
		ret += snprintf(temp_buf, DC_SC_CUR_LEN,
			"\r\n^MULTICHARGER:%s,%d,%d,%d,%d",
			di->curr_info.ic_name[i], di->curr_info.ibus[i],
			di->curr_info.vout[i], di->curr_info.vbat[i],
			di->curr_info.tbat[i]);
		strncat(buf, temp_buf, strlen(temp_buf));
		i++;
	}

	return ret;
}

static int sc_get_multi_ibat(char *buf, unsigned int len)
{
	int ret;
	int i = 0;
	char temp_buf[DC_SC_CUR_LEN] = {0};
	struct direct_charge_device *di = g_sc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di || (di->multi_ic_mode_para.support_multi_ic == 0)) {
		hwlog_err("di is null or not support multi sc\n");
		return 0;
	}

	ret = snprintf(buf, len, "%d,%d\r\n^GETCHARGEINFO:PMU,%d",
		di->curr_info.channel_num, di->curr_info.pmu_ibat, di->curr_info.pmu_ibat);

	while ((ret > 0) && (i < di->curr_info.channel_num)) {
		memset(temp_buf, 0, sizeof(temp_buf));
		ret += snprintf(temp_buf, DC_SC_CUR_LEN, "\r\n^GETCHARGEINFO:%s,%d",
			di->curr_info.ic_name[i], di->curr_info.ibat[i]);
		if ((len - strlen(buf)) < strlen(temp_buf))
			return 0;
		strncat(buf, temp_buf, strlen(temp_buf));
		i++;
	}

	return ret;
}

static int sc_get_ibus(int *ibus)
{
	int ret;

	if (!ibus)
		return -1;

	ret = dcm_get_ic_ibus(SC_MODE, CHARGE_MULTI_IC, ibus);
	if (ret || ibus < 0)
		return -1;
	return 0;
}

static int sc_get_vbus(int *vbus)
{
	struct direct_charge_device *di = g_sc_di;

	if (!di || di->ls_vbus < 0 || !vbus)
		return -1;

	*vbus = di->ls_vbus;
	return 0;
}

static struct power_if_ops sc_if_ops = {
	.set_enable_charger = sc_set_enable_charger,
	.get_enable_charger = sc_get_enable_charger,
	.set_iin_limit = sc_set_iin_limit,
	.get_iin_limit = sc_get_iin_limit,
	.set_iin_thermal = sc_set_iin_thermal,
	.set_iin_thermal_all = sc_set_iin_thermal_all,
	.set_ichg_ratio = sc_set_ichg_ratio,
	.set_vterm_dec = sc_set_vterm_dec,
	.get_rt_test_time = sc_get_rt_test_time,
	.get_rt_test_result = sc_get_rt_test_result,
	.get_hota_iin_limit = sc_get_hota_iin_limit,
	.get_startup_iin_limit = sc_get_startup_iin_limit,
	.get_ibus = sc_get_ibus,
	.get_vbus = sc_get_vbus,
	.type_name = "sc",
};

static struct power_if_ops mainsc_if_ops = {
	.set_enable_charger = mainsc_set_enable_charger,
	.get_enable_charger = mainsc_get_enable_charger,
	.get_rt_test_time = mainsc_get_rt_test_time,
	.get_rt_test_result = mainsc_get_rt_test_result,
	.type_name = "mainsc",
};

static struct power_if_ops auxsc_if_ops = {
	.set_enable_charger = auxsc_set_enable_charger,
	.get_enable_charger = auxsc_get_enable_charger,
	.get_rt_test_time = auxsc_get_rt_test_time,
	.get_rt_test_result = auxsc_get_rt_test_result,
	.type_name = "auxsc",
};

static void sc_power_if_ops_register(struct direct_charge_device *di)
{
	power_if_ops_register(&sc_if_ops);
	if (di->multi_ic_mode_para.support_multi_ic) {
		power_if_ops_register(&mainsc_if_ops);
		power_if_ops_register(&auxsc_if_ops);
	}
}

int sc_get_di(struct direct_charge_device **di)
{
	if (!g_sc_di || !di) {
		hwlog_err("g_sc_di or di is null\n");
		return -1;
	}

	*di = g_sc_di;

	return 0;
}

int sc_set_disable_flags(int val, int type)
{
	int i;
	unsigned int disable;
	struct direct_charge_device *di = g_sc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (type < DC_DISABLE_BEGIN || type >= DC_DISABLE_END) {
		hwlog_err("invalid disable_type=%d\n", type);
		return -1;
	}

	disable = 0;
	di->sysfs_disable_charger[type] = val;
	for (i = 0; i < DC_DISABLE_END; i++)
		disable |= di->sysfs_disable_charger[i];
	di->sysfs_enable_charger = !disable;

	hwlog_info("set_disable_flag val=%d, type=%d, disable=%u\n",
		val, type, disable);
	return 0;
}

static void sc_fault_work(struct work_struct *work)
{
	char buf[POWER_DSM_BUF_SIZE_0256] = { 0 };
	char reg_info[POWER_DSM_BUF_SIZE_0128] = { 0 };
	struct direct_charge_device *di = NULL;
	struct nty_data *data = NULL;

	if (!work)
		return;

	di = container_of(work, struct direct_charge_device, fault_work);
	if (!di)
		return;

	data = di->fault_data;
	direct_charge_set_stop_charging_flag(1);

	if (data)
		snprintf(reg_info, sizeof(reg_info),
			"sc charge_fault=%d, addr=0x%x, event1=0x%x, event2=0x%x\n",
			di->charge_fault, data->addr,
			data->event1, data->event2);
	else
		snprintf(reg_info, sizeof(reg_info),
			"sc charge_fault=%d, addr=0x0, event1=0x0, event2=0x0\n",
			di->charge_fault);

	dc_fill_eh_buf(di->dsm_buff, sizeof(di->dsm_buff),
		DC_EH_HAPPEN_SC_FAULT, reg_info);

	switch (di->charge_fault) {
	case POWER_NE_DC_FAULT_VBUS_OVP:
		hwlog_err("vbus ovp happened\n");
		snprintf(buf, sizeof(buf), "vbus ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_VBUS_OVP, buf);
		break;
	case POWER_NE_DC_FAULT_TSBAT_OTP:
		hwlog_err("tsbat otp happened\n");
		snprintf(buf, sizeof(buf), "tsbat otp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_TSBAT_OTP, buf);
		break;
	case POWER_NE_DC_FAULT_TSBUS_OTP:
		hwlog_err("tsbus otp happened\n");
		snprintf(buf, sizeof(buf), "tsbus otp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_TSBUS_OTP, buf);
		break;
	case POWER_NE_DC_FAULT_TDIE_OTP:
		hwlog_err("tdie otp happened\n");
		snprintf(buf, sizeof(buf), "tdie otp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_TDIE_OTP, buf);
		break;
	case POWER_NE_DC_FAULT_VDROP_OVP:
		hwlog_err("vdrop ovp happened\n");
		snprintf(buf, sizeof(buf), "vdrop ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		break;
	case POWER_NE_DC_FAULT_AC_OVP:
		hwlog_err("AC ovp happened\n");
		snprintf(buf, sizeof(buf), "ac ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_AC_OVP, buf);
		break;
	case POWER_NE_DC_FAULT_VBAT_OVP:
		hwlog_err("vbat ovp happened\n");
		snprintf(buf, sizeof(buf), "vbat ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_VBAT_OVP, buf);
		break;
	case POWER_NE_DC_FAULT_IBAT_OCP:
		hwlog_err("ibat ocp happened\n");
		snprintf(buf, sizeof(buf), "ibat ocp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_IBAT_OCP, buf);
		break;
	case POWER_NE_DC_FAULT_IBUS_OCP:
		hwlog_err("ibus ocp happened\n");
		snprintf(buf, sizeof(buf), "ibus ocp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_IBUS_OCP, buf);
		break;
	case POWER_NE_DC_FAULT_CONV_OCP:
		hwlog_err("conv ocp happened\n");
		snprintf(buf, sizeof(buf), "conv ocp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_CONV_OCP, buf);
		adapter_test_set_result(AT_TYPE_SC, AT_DETECT_OTHER);
		di->sc_conv_ocp_count++;
		break;
	case POWER_NE_DC_FAULT_LTC7820:
		hwlog_err("ltc7820 chip error happened\n");
		snprintf(buf, sizeof(buf), "ltc7820 chip error happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		break;
	case POWER_NE_DC_FAULT_INA231:
		hwlog_err("ina231 interrupt happened\n");
		dcm_enable_ic(SC_MODE, di->cur_mode, DC_IC_DISABLE);
		dc_close_aux_wired_channel();
		break;
	case POWER_NE_DC_FAULT_CC_SHORT:
		hwlog_err("typec cc vbus short happened\n");
		break;
	default:
		hwlog_err("unknown fault: %u happened\n", di->charge_fault);
		break;
	}
}

#ifdef CONFIG_SYSFS
static ssize_t sc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t sc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info sc_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_IIN_THERMAL, iin_thermal),
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_IIN_THERMAL_ICHG_CONTROL, iin_thermal_ichg_control),
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_ICHG_CONTROL_ENABLE, ichg_control_enable),
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_THERMAL_REASON, thermal_reason),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_ADAPTER_DETECT, adaptor_detect),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_IADAPT, iadapt),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_FULL_PATH_RESISTANCE, full_path_resistance),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_DIRECT_CHARGE_SUCC, direct_charge_succ),
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_SET_RESISTANCE_THRESHOLD, set_resistance_threshold),
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_SET_CHARGETYPE_PRIORITY, set_chargetype_priority),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_MULTI_SC_CUR, multi_sc_cur),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_MULTI_IBAT, multi_ibat),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_SC_STATE, sc_state),
	power_sysfs_attr_rw(sc, 0664, DC_SYSFS_SC_FREQ, sc_freq),
};

#define SC_SYSFS_ATTRS_SIZE  ARRAY_SIZE(sc_sysfs_field_tbl)

static struct attribute *sc_sysfs_attrs[SC_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group sc_sysfs_attr_group = {
	.attrs = sc_sysfs_attrs,
};

static ssize_t sc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct direct_charge_device *di = dev_get_drvdata(dev);
	unsigned int type = power_platform_get_charger_type();
	int chg_state = direct_charge_get_stage_status();
	int work_mode = direct_charge_get_working_mode();
	int ret;
	int len;
	int idx;

	info = power_sysfs_lookup_attr(attr->attr.name,
		sc_sysfs_field_tbl, SC_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case DC_SYSFS_IIN_THERMAL:
		idx = (di->cur_mode == CHARGE_MULTI_IC) ? DC_DUAL_CHANNEL : DC_SINGLE_CHANNEL;
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->sysfs_iin_thermal_array[idx]);
		break;
	case DC_SYSFS_IIN_THERMAL_ICHG_CONTROL:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->sysfs_iin_thermal_ichg_control);
		break;
	case DC_SYSFS_ICHG_CONTROL_ENABLE:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->ichg_control_enable);
		break;
	case DC_SYSFS_THERMAL_REASON:
		len = snprintf(buf, PAGE_SIZE, "%s\n", di->thermal_reason);
		break;
	case DC_SYSFS_ADAPTER_DETECT:
		ret = ADAPTER_DETECT_FAIL;
		if (adapter_get_protocol_register_state(ADAPTER_PROTOCOL_SCP) ||
			(di->working_mode != work_mode)) {
			len = snprintf(buf, PAGE_SIZE, "%d\n", ret);
			break;
		}

		if (((type == CHARGER_TYPE_STANDARD) &&
			(chg_state >= DC_STAGE_ADAPTER_DETECT)) ||
			((type == CHARGER_REMOVED) &&
			(chg_state == DC_STAGE_CHARGING)))
			ret = ADAPTER_DETECT_SUCC;

		len = snprintf(buf, PAGE_SIZE, "%d\n", ret);
		break;
	case DC_SYSFS_IADAPT:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->iadapt);
		break;
	case DC_SYSFS_FULL_PATH_RESISTANCE:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			di->full_path_resistance);
		break;
	case DC_SYSFS_DIRECT_CHARGE_SUCC:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->dc_succ_flag);
		break;
	case DC_SYSFS_MULTI_SC_CUR:
		len = sc_get_multi_cur(buf, PAGE_SIZE);
		break;
	case DC_SYSFS_MULTI_IBAT:
		len = sc_get_multi_ibat(buf, PAGE_SIZE);
		break;
	case DC_SYSFS_SC_STATE:
		idx = (di->cur_mode == CHARGE_MULTI_IC) ? DC_DUAL_CHANNEL : DC_SINGLE_CHANNEL;
		len = snprintf(buf, PAGE_SIZE, "%d\n", idx);
		break;
	case DC_SYSFS_SC_FREQ:
		len = snprintf(buf, PAGE_SIZE, "%d\n", dcm_get_ic_freq(SC_MODE, CHARGE_IC_MAIN));
		break;
	default:
		len = 0;
		break;
	}

	return len;
}

static ssize_t sc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct direct_charge_device *di = dev_get_drvdata(dev);
	long val = 0;

	info = power_sysfs_lookup_attr(attr->attr.name,
		sc_sysfs_field_tbl, SC_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case DC_SYSFS_IIN_THERMAL:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
			return -EINVAL;

		sc_set_iin_limit((unsigned int)val);
		break;
	case DC_SYSFS_IIN_THERMAL_ICHG_CONTROL:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
			return -EINVAL;
		sc_set_iin_limit_ichg_control((unsigned int)val);
		break;
	case DC_SYSFS_ICHG_CONTROL_ENABLE:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
			return -EINVAL;
		di->ichg_control_enable = val;
		break;
	case DC_SYSFS_THERMAL_REASON:
		if (strlen(buf) >= DC_THERMAL_REASON_SIZE)
			return -EINVAL;
		snprintf(di->thermal_reason, strlen(buf), "%s", buf);
		power_event_notify_sysfs(&di->dev->kobj, NULL, "thermal_reason");
		hwlog_info("THERMAL set reason = %s, buf = %s\n", di->thermal_reason, buf);
		break;
	case DC_SYSFS_SET_RESISTANCE_THRESHOLD:
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
			(val < 0) || (val > DC_MAX_RESISTANCE))
			return -EINVAL;

		hwlog_info("set resistance_threshold=%ld\n", val);

		di->std_cable_full_path_res_max = val;
		di->nonstd_cable_full_path_res_max = val;
		di->ctc_cable_full_path_res_max = val;
		di->ignore_full_path_res = true;
		break;
	case DC_SYSFS_SET_CHARGETYPE_PRIORITY:
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
			(val < 0) || (val > DC_MAX_RESISTANCE))
			return -EINVAL;

		hwlog_info("set chargertype_priority=%ld\n", val);

		if (val == DC_CHARGE_TYPE_SC)
			direct_charge_set_local_mode(OR_SET, SC_MODE);
		else if (val == DC_CHARGE_TYPE_LVC)
			direct_charge_set_local_mode(AND_SET, LVC_MODE);
		else
			hwlog_info("invalid chargertype priority\n");
		break;
	case DC_SYSFS_SC_FREQ:
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
			(val < 0) || (val > 1))
			return -EINVAL;
		hwlog_info("set freq=%ld\n", val);
		(void)dcm_set_ic_freq(SC_MODE, CHARGE_IC_MAIN, val);
		break;
	default:
		break;
	}

	return count;
}

static void sc_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(sc_sysfs_attrs,
		sc_sysfs_field_tbl, SC_SYSFS_ATTRS_SIZE);
	power_sysfs_create_link_group("hw_power", "charger", "direct_charger_sc",
		dev, &sc_sysfs_attr_group);
}

static void sc_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "direct_charger_sc",
		dev, &sc_sysfs_attr_group);
}
#else
static inline void sc_sysfs_create_group(struct device *dev)
{
}

static inline void sc_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void sc_init_parameters(struct direct_charge_device *di)
{
	int i;

	di->sysfs_enable_charger = 1;
	di->sysfs_mainsc_enable_charger = 1;
	di->sysfs_auxsc_enable_charger = 1;
	di->dc_stage = DC_STAGE_DEFAULT;
	di->sysfs_iin_thermal = di->iin_thermal_default;
	di->sysfs_iin_thermal_array[DC_SINGLE_CHANNEL] = di->iin_thermal_default;
	di->sysfs_iin_thermal_array[DC_DUAL_CHANNEL] = di->iin_thermal_default;
	di->max_adapter_iset = di->iin_thermal_default;
	di->sysfs_iin_thermal_ichg_control = di->iin_thermal_default;
	di->ichg_control_enable = 0;
	di->dc_succ_flag = DC_ERROR;
	di->scp_stop_charging_complete_flag = 1;
	di->dc_err_report_flag = FALSE;
	di->bat_temp_err_flag = false;
	di->sc_conv_ocp_count = 0;
	di->ignore_full_path_res = false;
	di->cur_mode = CHARGE_IC_MAIN;
	di->tbat_id = BAT_TEMP_MIXED;
	di->local_mode = SC_MODE;
	di->force_single_path_flag = false;
	for (i = 0; i < DC_MODE_TOTAL; i++)
		di->rt_test_para[i].rt_test_result = false;
	di->cc_safe = true;
}

static void sc_init_mulit_ic_check_info(struct direct_charge_device *di)
{
	di->multi_ic_check_info.limit_current = di->iin_thermal_default;
	di->multi_ic_check_info.ibat_th = MULTI_IC_INFO_IBAT_TH_DEFAULT;
}

static int sc_probe(struct platform_device *pdev)
{
	int ret;
	struct direct_charge_device *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;

	ret = dc_parse_dts(np, di);
	if (ret)
		goto fail_free_mem;

	sc_init_parameters(di);
	sc_init_mulit_ic_check_info(di);
	direct_charge_set_local_mode(OR_SET, SC_MODE);

	di->charging_wq = create_singlethread_workqueue("sc_charging_wq");
	di->kick_wtd_wq = create_singlethread_workqueue("sc_wtd_wq");

	wakeup_source_init(&di->charging_lock, "sc_wakelock");

	INIT_WORK(&di->calc_thld_work, dc_calc_thld_work);
	INIT_WORK(&di->control_work, dc_control_work);
	INIT_WORK(&di->fault_work, sc_fault_work);
	INIT_WORK(&di->kick_wtd_work, dc_kick_wtd_work);

	hrtimer_init(&di->calc_thld_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->calc_thld_timer.function = dc_calc_thld_timer_func;
	hrtimer_init(&di->control_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->control_timer.function = dc_control_timer_func;
	hrtimer_init(&di->kick_wtd_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->kick_wtd_timer.function = dc_kick_wtd_timer_func;

	sc_sysfs_create_group(di->dev);
	g_sc_di = di;
	direct_charge_set_di(di);

	di->fault_nb.notifier_call = direct_charge_fault_notifier_call;
	ret = power_event_anc_register(POWER_ANT_SC_FAULT, &di->fault_nb);
	if (ret < 0)
		goto fail_create_link;

	sc_get_devices_info(di);
	sc_power_if_ops_register(di);
	sc_dbg_register(di);

	platform_set_drvdata(pdev, di);

	return 0;

fail_create_link:
	sc_sysfs_remove_group(di->dev);
	wakeup_source_trash(&di->charging_lock);
fail_free_mem:
	devm_kfree(&pdev->dev, di);
	g_sc_di = NULL;

	return ret;
}

static int sc_remove(struct platform_device *pdev)
{
	struct direct_charge_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	power_event_anc_unregister(POWER_ANT_SC_FAULT, &di->fault_nb);
	sc_sysfs_remove_group(di->dev);
	wakeup_source_trash(&di->charging_lock);
	devm_kfree(&pdev->dev, di);
	g_sc_di = NULL;

	return 0;
}

static const struct of_device_id sc_match_table[] = {
	{
		.compatible = "direct_charger_sc",
		.data = NULL,
	},
	{},
};

static struct platform_driver sc_driver = {
	.probe = sc_probe,
	.remove = sc_remove,
	.driver = {
		.name = "direct_charger_sc",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(sc_match_table),
	},
};

static int __init sc_init(void)
{
	return platform_driver_register(&sc_driver);
}

static void __exit sc_exit(void)
{
	platform_driver_unregister(&sc_driver);
}

late_initcall(sc_init);
module_exit(sc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("direct charger with switch cap module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
