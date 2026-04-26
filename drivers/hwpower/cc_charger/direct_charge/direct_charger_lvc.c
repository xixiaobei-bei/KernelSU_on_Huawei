/*
 * direct_charger_lvc.c
 *
 * direct charger with lvc (load switch) driver
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

#define HWLOG_TAG direct_charge_lvc
HWLOG_REGIST();

static struct direct_charge_device *g_lvc_di;

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
static void lvc_get_devices_info(struct direct_charge_device *di)
{
	int ret;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!di)
		return;

	ret = dcm_init_ic(LVC_MODE, CHARGE_IC_MAIN);
	di->ls_id = dcm_get_ic_id(LVC_MODE, di->cur_mode);
	ret += dcm_exit_ic(LVC_MODE, CHARGE_IC_MAIN);
	if ((di->ls_id < DC_DEVICE_ID_BEGIN) || (di->ls_id >= DC_DEVICE_ID_END))
		di->ls_id = DC_DEVICE_ID_END;
	di->ls_name = dc_get_device_name(di->ls_id);
	hwlog_info("loadswitch id=%d, %s\n", di->ls_id, di->ls_name);
	if (ret == 0)
		set_hw_dev_flag(DEV_I2C_LOADSWITCH);

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->ls_name;
		power_dev_info->dev_id = di->ls_id;
		power_dev_info->ver_id = 0;
	}
}
#else
static void lvc_get_devices_info(struct direct_charge_device *di)
{
}
#endif /* CONFIG_HUAWEI_HW_DEV_DCT */

static int lvc_set_enable_charger(unsigned int val)
{
	struct direct_charge_device *di = g_lvc_di;
	int ret;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	/* must be 0 or 1, 0: disable, 1: enable */
	if ((val < 0) || (val > 1))
		return -1;

	ret = lvc_set_disable_flags((val ?
		DC_CLEAR_DISABLE_FLAGS : DC_SET_DISABLE_FLAGS),
		DC_DISABLE_SYS_NODE);
	hwlog_info("set enable_charger=%d\n", di->sysfs_enable_charger);
	return ret;
}

static int lvc_get_enable_charger(unsigned int *val)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->sysfs_enable_charger;
	return 0;
}

static int lvc_set_iin_limit(unsigned int val)
{
	struct direct_charge_device *di = g_lvc_di;
	int index;
	int cur_low;
	unsigned int idx = DC_SINGLE_CHANNEL;

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

	hwlog_info("set input current: %u, limit current: %d\n",
		val, di->sysfs_iin_thermal_array[idx]);
	return 0;
}

static int lvc_set_iin_limit_ichg_control(unsigned int val)
{
	struct direct_charge_device *di = g_lvc_di;
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
		di->sysfs_iin_thermal_ichg_control = di->iin_thermal_default;
	else if (val < cur_low)
		di->sysfs_iin_thermal_ichg_control = cur_low;
	else
		di->sysfs_iin_thermal_ichg_control = val;

	hwlog_info("ichg_control set input current: %u, limit current: %d\n",
		val, di->sysfs_iin_thermal_ichg_control);
	return 0;
}

static int lvc_get_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -1;
	}

	*val = di->sysfs_iin_thermal_array[DC_SINGLE_CHANNEL];
	return 0;
}

static int lvc_set_iin_thermal(unsigned int index, unsigned int iin_thermal_value)
{
	if (index != DC_SINGLE_CHANNEL) {
		hwlog_err("error index: %u, out of boundary\n", index);
		return -1;
	}
	return lvc_set_iin_limit(iin_thermal_value);
}

static int lvc_set_ichg_ratio(unsigned int val)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	di->ichg_ratio = val;
	hwlog_info("set ichg_ratio=%u\n", val);
	return 0;
}

static int lvc_set_vterm_dec(unsigned int val)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	di->vterm_dec = val;
	hwlog_info("set vterm_dec=%u\n", val);
	return 0;
}

static int lvc_get_rt_test_time(unsigned int *val)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->rt_test_para[DC_NORMAL_MODE].rt_test_time;
	return 0;
}

static int lvc_get_rt_test_result(unsigned int *val)
{
	int iin_thermal_th;
	struct direct_charge_device *di = g_lvc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	iin_thermal_th = di->rt_test_para[DC_NORMAL_MODE].rt_curr_th + 500; /* margin is 500mA */
	if (pd_dpm_get_cc_moisture_status() || di->bat_temp_err_flag ||
		di->rt_test_para[DC_NORMAL_MODE].rt_test_result ||
		(di->sysfs_enable_charger == 0) ||
		((direct_charge_get_stage_status() == DC_STAGE_CHARGING) &&
		(di->sysfs_iin_thermal < iin_thermal_th)) ||
		direct_charge_check_charge_done())
		*val = 0; /* 0: succ */
	else
		*val = 1; /* 1: fail */

	return 0;
}

static int lvc_get_hota_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	*val = di->hota_iin_limit;
	return 0;
}

static int lvc_get_startup_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	*val = di->startup_iin_limit;
	return 0;
}

static int lvc_get_ibus(int *ibus)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di || di->ls_ibus < 0 || !ibus) {
		hwlog_err("di or ibus is null, or ibus value is err\n");
		return -1;
	}

	*ibus = di->ls_ibus;
	return 0;
}

static int lvc_get_vbus(int *vbus)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di || di->ls_vbus < 0 || !vbus)
		return -1;

	*vbus = di->ls_vbus;
	return 0;
}

static struct power_if_ops lvc_if_ops = {
	.set_enable_charger = lvc_set_enable_charger,
	.get_enable_charger = lvc_get_enable_charger,
	.set_iin_limit = lvc_set_iin_limit,
	.get_iin_limit = lvc_get_iin_limit,
	.set_iin_thermal = lvc_set_iin_thermal,
	.set_iin_thermal_all = lvc_set_iin_limit,
	.set_ichg_ratio = lvc_set_ichg_ratio,
	.set_vterm_dec = lvc_set_vterm_dec,
	.get_rt_test_time = lvc_get_rt_test_time,
	.get_rt_test_result = lvc_get_rt_test_result,
	.get_hota_iin_limit = lvc_get_hota_iin_limit,
	.get_startup_iin_limit = lvc_get_startup_iin_limit,
	.get_ibus = lvc_get_ibus,
	.get_vbus = lvc_get_vbus,
	.type_name = "lvc",
};

int lvc_get_di(struct direct_charge_device **di)
{
	if (!g_lvc_di || !di) {
		hwlog_err("g_lvc_di or di is null\n");
		return -1;
	}

	*di = g_lvc_di;

	return 0;
}

int lvc_set_disable_flags(int val, int type)
{
	int i;
	unsigned int disable;
	struct direct_charge_device *di = g_lvc_di;

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

static void lvc_fault_work(struct work_struct *work)
{
	char buf[POWER_DSM_BUF_SIZE_0256] = { 0 };
	char reg_info[POWER_DSM_BUF_SIZE_0128] = { 0 };
	struct direct_charge_device *di = NULL;
	struct nty_data *data = NULL;
	int bat_capacity;

	if (!work)
		return;

	di = container_of(work, struct direct_charge_device, fault_work);
	if (!di)
		return;

	data = di->fault_data;
	direct_charge_set_stop_charging_flag(1);

	if (data)
		snprintf(reg_info, sizeof(reg_info),
			"lvc charge_fault=%d, addr=0x%x, event1=0x%x, event2=0x%x\n",
			di->charge_fault, data->addr,
			data->event1, data->event2);
	else
		snprintf(reg_info, sizeof(reg_info),
			"lvc charge_fault=%d, addr=0x0, event1=0x0, event2=0x0\n",
			di->charge_fault);

	dc_fill_eh_buf(di->dsm_buff, sizeof(di->dsm_buff),
		DC_EH_HAPPEN_LVC_FAULT, reg_info);

	switch (di->charge_fault) {
	case POWER_NE_DC_FAULT_VBUS_OVP:
		hwlog_err("vbus ovp happened\n");
		snprintf(buf, sizeof(buf), "vbus ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_BATTERY,
			DSM_DIRECT_CHARGE_VBUS_OVP, buf);
		break;
	case POWER_NE_DC_FAULT_REVERSE_OCP:
		bat_capacity = power_platform_get_battery_ui_capacity();
		if (bat_capacity < BATTERY_CAPACITY_HIGH_TH)
			di->reverse_ocp_cnt++;

		hwlog_err("reverse ocp happened, capacity=%d, ocp_cnt=%d\n",
			bat_capacity, di->reverse_ocp_cnt);

		if (di->reverse_ocp_cnt >= REVERSE_OCP_CNT) {
			di->reverse_ocp_cnt = REVERSE_OCP_CNT;
			snprintf(buf, sizeof(buf), "reverse ocp happened\n");
			strncat(buf, reg_info, strlen(reg_info));
			power_dsm_report_dmd(POWER_DSM_BATTERY,
				DSM_DIRECT_CHARGE_REVERSE_OCP, buf);
		}
		break;
	case POWER_NE_DC_FAULT_OTP:
		hwlog_err("otp happened\n");
		di->otp_cnt++;
		if (di->otp_cnt >= OTP_CNT) {
			di->otp_cnt = OTP_CNT;
			snprintf(buf, sizeof(buf), "otp happened\n");
			strncat(buf, reg_info, strlen(reg_info));
			power_dsm_report_dmd(POWER_DSM_BATTERY,
				DSM_DIRECT_CHARGE_OTP, buf);
		}
		break;
	case POWER_NE_DC_FAULT_INPUT_OCP:
		hwlog_err("input ocp happened\n");
		snprintf(buf, sizeof(buf), "input ocp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_report_dmd(POWER_DSM_BATTERY,
			DSM_DIRECT_CHARGE_INPUT_OCP, buf);
		break;
	case POWER_NE_DC_FAULT_VDROP_OVP:
		hwlog_err("vdrop ovp happened\n");
		snprintf(buf, sizeof(buf), "vdrop ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		break;
	case POWER_NE_DC_FAULT_INA231:
		hwlog_err("ina231 interrupt happened\n");
		dcm_enable_ic(LVC_MODE, di->cur_mode, DC_IC_DISABLE);
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
static ssize_t lvc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t lvc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info lvc_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(lvc, 0644, DC_SYSFS_IIN_THERMAL, iin_thermal),
	power_sysfs_attr_rw(lvc, 0644, DC_SYSFS_IIN_THERMAL_ICHG_CONTROL, iin_thermal_ichg_control),
	power_sysfs_attr_rw(lvc, 0644, DC_SYSFS_ICHG_CONTROL_ENABLE, ichg_control_enable),
	power_sysfs_attr_rw(lvc, 0644, DC_SYSFS_THERMAL_REASON, thermal_reason),
	power_sysfs_attr_ro(lvc, 0444, DC_SYSFS_ADAPTER_DETECT, adaptor_detect),
	power_sysfs_attr_ro(lvc, 0444, DC_SYSFS_IADAPT, iadapt),
	power_sysfs_attr_ro(lvc, 0444, DC_SYSFS_FULL_PATH_RESISTANCE, full_path_resistance),
	power_sysfs_attr_ro(lvc, 0444, DC_SYSFS_DIRECT_CHARGE_SUCC, direct_charge_succ),
	power_sysfs_attr_rw(lvc, 0644, DC_SYSFS_SET_RESISTANCE_THRESHOLD, set_resistance_threshold),
};

#define LVC_SYSFS_ATTRS_SIZE  ARRAY_SIZE(lvc_sysfs_field_tbl)

static struct attribute *lvc_sysfs_attrs[LVC_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group lvc_sysfs_attr_group = {
	.attrs = lvc_sysfs_attrs,
};

static ssize_t lvc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct direct_charge_device *di = dev_get_drvdata(dev);
	unsigned int type = power_platform_get_charger_type();
	int chg_state = direct_charge_get_stage_status();
	int work_mode = direct_charge_get_working_mode();
	int ret;
	int len;

	info = power_sysfs_lookup_attr(attr->attr.name,
		lvc_sysfs_field_tbl, LVC_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case DC_SYSFS_IIN_THERMAL:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			di->sysfs_iin_thermal_array[DC_SINGLE_CHANNEL]);
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
	default:
		len = 0;
		break;
	}

	return len;
}

static ssize_t lvc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct direct_charge_device *di = dev_get_drvdata(dev);
	long val = 0;

	info = power_sysfs_lookup_attr(attr->attr.name,
		lvc_sysfs_field_tbl, LVC_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case DC_SYSFS_IIN_THERMAL:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
			return -EINVAL;

		lvc_set_iin_limit((unsigned int)val);
		break;
	case DC_SYSFS_IIN_THERMAL_ICHG_CONTROL:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
			return -EINVAL;
		lvc_set_iin_limit_ichg_control((unsigned int)val);
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
		break;
	default:
		break;
	}

	return count;
}

static void lvc_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(lvc_sysfs_attrs,
		lvc_sysfs_field_tbl, LVC_SYSFS_ATTRS_SIZE);
	power_sysfs_create_link_group("hw_power", "charger", "direct_charger",
		dev, &lvc_sysfs_attr_group);
}

static void lvc_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "direct_charger",
		dev, &lvc_sysfs_attr_group);
}
#else
static inline void lvc_sysfs_create_group(struct device *dev)
{
}

static inline void lvc_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void lvc_init_parameters(struct direct_charge_device *di)
{
	int i;

	di->sysfs_enable_charger = 1;
	di->dc_stage = DC_STAGE_DEFAULT;
	di->sysfs_iin_thermal = di->iin_thermal_default;
	di->sysfs_iin_thermal_array[DC_SINGLE_CHANNEL] = di->iin_thermal_default;
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
	di->local_mode = LVC_MODE;
	di->force_single_path_flag = false;
	for (i = 0; i < DC_MODE_TOTAL; i++)
		di->rt_test_para[i].rt_test_result = false;
}

static int lvc_probe(struct platform_device *pdev)
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

	lvc_init_parameters(di);
	direct_charge_set_local_mode(OR_SET, LVC_MODE);

	di->charging_wq = create_singlethread_workqueue("lvc_charging_wq");
	di->kick_wtd_wq = create_singlethread_workqueue("lvc_wtd_wq");

	wakeup_source_init(&di->charging_lock, "lvc_wakelock");

	INIT_WORK(&di->calc_thld_work, dc_calc_thld_work);
	INIT_WORK(&di->control_work, dc_control_work);
	INIT_WORK(&di->fault_work, lvc_fault_work);
	INIT_WORK(&di->kick_wtd_work, dc_kick_wtd_work);

	hrtimer_init(&di->calc_thld_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->calc_thld_timer.function = dc_calc_thld_timer_func;
	hrtimer_init(&di->control_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->control_timer.function = dc_control_timer_func;
	hrtimer_init(&di->kick_wtd_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->kick_wtd_timer.function = dc_kick_wtd_timer_func;

	lvc_sysfs_create_group(di->dev);
	g_lvc_di = di;
	direct_charge_set_di(di);

	di->fault_nb.notifier_call = direct_charge_fault_notifier_call;
	ret = power_event_anc_register(POWER_ANT_LVC_FAULT, &di->fault_nb);
	if (ret < 0)
		goto fail_create_link;

	lvc_get_devices_info(di);
	power_if_ops_register(&lvc_if_ops);
	lvc_dbg_register(di);

	platform_set_drvdata(pdev, di);

	return 0;

fail_create_link:
	lvc_sysfs_remove_group(di->dev);
	wakeup_source_trash(&di->charging_lock);
fail_free_mem:
	devm_kfree(&pdev->dev, di);
	g_lvc_di = NULL;

	return ret;
}

static int lvc_remove(struct platform_device *pdev)
{
	struct direct_charge_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	power_event_anc_unregister(POWER_ANT_LVC_FAULT, &di->fault_nb);
	lvc_sysfs_remove_group(di->dev);
	wakeup_source_trash(&di->charging_lock);
	devm_kfree(&pdev->dev, di);
	g_lvc_di = NULL;

	return 0;
}

static const struct of_device_id lvc_match_table[] = {
	{
		.compatible = "direct_charger",
		.data = NULL,
	},
	{},
};

static struct platform_driver lvc_driver = {
	.probe = lvc_probe,
	.remove = lvc_remove,
	.driver = {
		.name = "direct_charger",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(lvc_match_table),
	},
};

static int __init lvc_init(void)
{
	return platform_driver_register(&lvc_driver);
}

static void __exit lvc_exit(void)
{
	platform_driver_unregister(&lvc_driver);
}

late_initcall(lvc_init);
module_exit(lvc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("direct charger with loadswitch module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
