/*
 * direct_charger_sc.c
 *
 * direct charger with sc (switch cap) driver
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

#include <huawei_platform/power/direct_charger/direct_charger.h>
#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
#include <huawei_platform/power/usb_short_circuit_protect.h>
#endif

#define HWLOG_TAG direct_charge_sc
HWLOG_REGIST();

static struct direct_charge_device *g_sc_di;
static struct loadswitch_ops *g_sc_ops;
static struct batinfo_ops *g_bi_sc_ops;
static struct direct_charge_volt_para_group g_sc_volt_para[DC_VOLT_GROUP_MAX];

static const char * const g_sc_device_name[] = {
	[SWITCHCAP_TI_BQ25970] = "BQ25970",
	[SWITCHCAP_SCHARGERV600] = "HI6526",
	[SWITCHCAP_LTC7820] = "LTC7820",
	[SWITCHCAP_MULTI_SC] = "MULTI_SC",
	[SWITCHCAP_TOTAL] = "ERROR",
};

struct atomic_notifier_head sc_fault_notifier_list;
ATOMIC_NOTIFIER_HEAD(sc_fault_notifier_list);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
static void sc_get_devices_info(struct direct_charge_device *di)
{
	int ret;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!di || !di->ls_ops || !di->ls_ops->ls_init ||
		!di->ls_ops->get_ls_id || !di->ls_ops->ls_exit)
		return;

	ret = di->ls_ops->ls_init();
	di->ls_id = di->ls_ops->get_ls_id();
	if ((di->ls_id < SWITCHCAP_BEGIN) || (di->ls_id >= SWITCHCAP_TOTAL))
		di->ls_id = SWITCHCAP_TOTAL;
	di->ls_name = g_sc_device_name[di->ls_id];
	hwlog_info("switchcap id=%d, %s\n", di->ls_id, di->ls_name);
	ret |= di->ls_ops->ls_exit();
	if (ret == 0)
		set_hw_dev_flag(DEV_I2C_SWITCHCAP);

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->ls_name;
		power_dev_info->dev_id = di->ls_id;
		power_dev_info->ver_id = 0;
	}
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

static int sc_set_iin_limit(unsigned int val)
{
	struct direct_charge_device *di = g_sc_di;
	int index;
	int cur_low;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	/* the value must be (0, 8000) ma */
	if (val > 8000) {
		hwlog_err("val is too large: %u, tuned as 8000\n", val);
		val = 8000;
	}

	if ((di->stage_size < 1) || (di->stage_size > DC_VOLT_LEVEL)) {
		hwlog_err("invalid stage_size=%d\n", di->stage_size);
		return -1;
	}

	index = di->stage_size - 1;
	cur_low = di->orig_volt_para_p[0].volt_info[index].cur_th_low;

	if (val == 0)
		di->sysfs_iin_thermal = di->iin_thermal_default;
	else if (val < cur_low)
		di->sysfs_iin_thermal = cur_low;
	else
		di->sysfs_iin_thermal = val;

	hwlog_info("set input current is:%u, limit current is %d\n",
		val, di->sysfs_iin_thermal);
	return 0;
}

static int sc_get_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_sc_di;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -1;
	}

	*val = di->sysfs_iin_thermal;
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
	hwlog_info("set ichg_ratio=%d\n", val);
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
	hwlog_info("set vterm_dec=%d\n", val);
	return 0;
}

static int sc_get_rt_test_time(unsigned int *val)
{
	struct direct_charge_device *di = g_sc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->rt_test_time;
	return 0;
}

static int sc_get_rt_test_result(unsigned int *val)
{
	int ibat;
	int iin_thermal_th;
	struct direct_charge_device *di = g_sc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ibat = -hisi_battery_current();
	iin_thermal_th = di->rt_curr_th + 500; /* margin is 500mA */
	if (di->bat_temp_err_flag ||
		di->rt_test_succ ||
		(di->sysfs_enable_charger == 0) ||
		((di->dc_succ_flag == DC_SUCC) &&
		((ibat >= (int)di->rt_curr_th) ||
		(di->sysfs_iin_thermal < iin_thermal_th))))
		*val = 0; /* 0: succ */
	else
		*val = 1; /* 1: fail */

	di->bat_temp_err_flag = false;
	di->rt_test_succ = false;
	return 0;
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

static void sc_set_antifake_result(unsigned int data)
{
	struct direct_charge_device *di = g_sc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	di->dc_antifake_result = data;
	complete(&di->dc_af_completion);

	hwlog_info("antifake_result=%d\n", di->dc_antifake_result);
}

static int sc_get_antifake_data(char *buf, unsigned int len)
{
	struct direct_charge_device *di = g_sc_di;
	int i;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	for (i = 0; i < len; i++) {
		buf[i] = di->dc_af_key[i];
		di->dc_af_key[i] = 0;
	}

	return len;
}

static struct power_if_ops sc_if_ops = {
	.set_enable_charger = sc_set_enable_charger,
	.get_enable_charger = sc_get_enable_charger,
	.set_iin_limit = sc_set_iin_limit,
	.get_iin_limit = sc_get_iin_limit,
	.set_ichg_ratio = sc_set_ichg_ratio,
	.set_vterm_dec = sc_set_vterm_dec,
	.get_rt_test_time = sc_get_rt_test_time,
	.get_rt_test_result = sc_get_rt_test_result,
	.get_hota_iin_limit = sc_get_hota_iin_limit,
	.get_startup_iin_limit = sc_get_startup_iin_limit,
	.type_name = "sc",
};

int sc_ops_register(struct loadswitch_ops *ops)
{
	int ret = 0;

	if (ops)
		g_sc_ops = ops;
	else
		ret = -EINVAL;

	return ret;
}

int sc_batinfo_ops_register(struct batinfo_ops *ops)
{
	int ret = 0;

	if (ops)
		g_bi_sc_ops = ops;
	else
		ret = -EINVAL;

	return ret;
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

void sc_get_fault_notifier(struct atomic_notifier_head **notifier)
{
	if (!notifier) {
		hwlog_err("notifier is null\n");
		return;
	}

	*notifier = &sc_fault_notifier_list;
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

	hwlog_info("set_disable_flag val=%d, type=%d, disable=%d\n",
		val, type, disable);
	return 0;
}

void sc_mode_check(void)
{
	unsigned int local_mode;
	unsigned int stage;
	struct direct_charge_device *di = g_sc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	/* step-1: check local mode */
	local_mode = direct_charge_get_local_mode();
	if (!(local_mode & SC_MODE)) {
		hwlog_err("local mode not support sc\n");
		return;
	}

	/* step-2: check sc error and report dmd */
	if (di->error_cnt >= DC_ERR_CNT_MAX) {
		hwlog_info("error exceed %d times, sc is disabled\n",
			DC_ERR_CNT_MAX);

		di->dc_succ_flag = DC_ERROR_CHARGE_DISABLED;
		direct_charge_send_normal_charging_uevent();

		if ((di->dc_err_report_flag == FALSE) &&
			(di->dc_open_retry_cnt <= DC_OPEN_RETRY_CNT_MAX)) {
			direct_charge_show_eh_buf(di->dsm_buff);

			if (di->cc_cable_detect_ok)
				direct_charge_report_eh_buf(di->dsm_buff,
					DSM_DIRECT_CHARGE_ERR_WITH_STD_CABLE);
			else
				direct_charge_report_eh_buf(di->dsm_buff,
					DSM_DIRECT_CHARGE_ERR_WITH_NONSTD_CABLE);

			direct_charge_clean_eh_buf(di->dsm_buff,
				sizeof(di->dsm_buff));
			di->dc_err_report_flag = TRUE;
		}

		return;
	}

	/* step-3: check sc enable status */
	if (di->sysfs_enable_charger == 0) {
		di->dc_succ_flag = DC_ERROR_CHARGE_DISABLED;
		hwlog_info("sc is disabled\n");
		return;
	}

	/* step-4: check scp protocol status */
	stage = direct_charge_get_stage_status();
	if (stage == DC_STAGE_DEFAULT) {
		if (direct_charge_get_protocol_register_state())
			return;
	}

#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
	/* step-5: check uscp status */
	if (is_in_uscp_mode()) {
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		di->dc_succ_flag = DC_ERROR_CHARGE_DISABLED;
		hwlog_err("sc is disabled by uscp\n");
		return;
	}
#endif /* CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT */

	/* step-6: detect cable */
	stage = direct_charge_get_stage_status();
	if (stage == DC_STAGE_ADAPTER_DETECT) {
		di->dc_succ_flag = DC_ERROR_ADAPTER_DETECT;
		di->adapter_detect_flag = DC_ADAPTER_DETECT;
		direct_charge_detect_cable();
	}

	/* step-7: update adapter info */
	direct_charge_update_adapter_info();

	/* step-8: check battery temp */
	if (direct_charge_check_battery_temp()) {
		di->dc_succ_flag = DC_ERROR_BAT_TEMP;
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		di->bat_temp_err_flag = true;
		hwlog_err("temp out of range, try next loop\n");
		return;
	}

	/* step-9: check battery voltage */
	if (direct_charge_check_battery_voltage()) {
		di->dc_succ_flag = DC_ERROR_BAT_VOL;
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		hwlog_err("volt out of range, try next loop\n");
		return;
	}

	/* step-10: check adapter antifake */
	if (direct_charge_check_adapter_antifake()) {
		di->dc_succ_flag = DC_ERROR_ADAPTER_ANTI_FAKE;
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		di->error_cnt += 1;
		hwlog_err("adapter antifake check failed\n");
		return;
	}

	/* step-11: switch buck charging path to sc charging path */
	stage = direct_charge_get_stage_status();
	if (stage == DC_STAGE_SWITCH_DETECT) {
		di->scp_stop_charging_complete_flag = 0;
		di->dc_succ_flag = DC_ERROR_SWITCH;
		if (direct_charge_switch_charging_path(PATH_SC) == 0) {
			direct_charge_set_stage_status(DC_STAGE_CHARGE_INIT);
		} else {
			hwlog_err("direct_charge path switch failed\n");
			direct_charge_set_stop_charging_flag(1);
			direct_charge_stop_charging();
		}
	}

	/* step-12: init scp adapter and device */
	stage = direct_charge_get_stage_status();
	if (stage == DC_STAGE_CHARGE_INIT) {
		di->dc_succ_flag = DC_ERROR_INIT;
		if (direct_charge_init_adapter_and_device() == 0) {
			direct_charge_set_stage_status(DC_STAGE_SECURITY_CHECK);
		} else {
			hwlog_err("direct_charge init failed\n");
			direct_charge_set_stop_charging_flag(1);
			direct_charge_stop_charging();
		}
	}

	/* step-13: direct charging security check */
	stage = direct_charge_get_stage_status();
	if (stage == DC_STAGE_SECURITY_CHECK) {
		if (direct_charge_security_check() == 0) {
			direct_charge_set_stage_status(DC_STAGE_SUCCESS);
		} else {
			hwlog_err("direct_charge security check failed\n");
			direct_charge_set_stop_charging_flag(1);
			direct_charge_stop_charging();
		}
	}

	/* step-14: start direct charging */
	stage = direct_charge_get_stage_status();
	if (stage == DC_STAGE_SUCCESS) {
		di->dc_succ_flag = DC_SUCC;
		direct_charge_start_charging();
	}

	/* step-15: end */
	stage = direct_charge_get_stage_status();
	if (stage == DC_STAGE_CHARGING)
		hwlog_info("in direct_charge process\n");

	hwlog_info("direct_charge stage=%d,%s\n",
		stage, direct_charge_get_stage_status_string(stage));
}

static void sc_fault_work(struct work_struct *work)
{
	char buf[POWER_DSM_BUF_SIZE_0256] = { 0 };
	char reg_info[POWER_DSM_BUF_SIZE_0128] = { 0 };
	struct direct_charge_device *di = NULL;
	struct nty_data *data = NULL;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct direct_charge_device, fault_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

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

	direct_charge_fill_eh_buf(di->dsm_buff, sizeof(di->dsm_buff),
		DC_EH_HAPPEN_SC_FAULT, reg_info);

	switch (di->charge_fault) {
	case DC_FAULT_VBUS_OVP:
		hwlog_err("vbus ovp happened\n");

		snprintf(buf, sizeof(buf), "vbus ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_VBUS_OVP, buf);
		break;
	case DC_FAULT_TSBAT_OTP:
		hwlog_err("tsbat otp happened\n");

		snprintf(buf, sizeof(buf), "tsbat otp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_TSBAT_OTP, buf);
		break;
	case DC_FAULT_TSBUS_OTP:
		hwlog_err("tsbus otp happened\n");

		snprintf(buf, sizeof(buf), "tsbus otp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_TSBUS_OTP, buf);
		break;
	case DC_FAULT_TDIE_OTP:
		hwlog_err("tdie otp happened\n");

		snprintf(buf, sizeof(buf), "tdie otp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_TDIE_OTP, buf);
		break;
	case DC_FAULT_VDROP_OVP:
		hwlog_err("vdrop ovp happened\n");

		snprintf(buf, sizeof(buf), "vdrop ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		break;
	case DC_FAULT_AC_OVP:
		hwlog_err("AC ovp happened\n");

		snprintf(buf, sizeof(buf), "ac ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_AC_OVP, buf);
		break;
	case DC_FAULT_VBAT_OVP:
		hwlog_err("vbat ovp happened\n");

		snprintf(buf, sizeof(buf), "vbat ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_VBAT_OVP, buf);
		break;
	case DC_FAULT_IBAT_OCP:
		hwlog_err("ibat ocp happened\n");

		snprintf(buf, sizeof(buf), "ibat ocp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_IBAT_OCP, buf);
		break;
	case DC_FAULT_IBUS_OCP:
		hwlog_err("ibus ocp happened\n");

		snprintf(buf, sizeof(buf), "ibus ocp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_IBUS_OCP, buf);
		break;
	case DC_FAULT_CONV_OCP:
		hwlog_err("conv ocp happened\n");

		snprintf(buf, sizeof(buf), "conv ocp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_CONV_OCP, buf);

		chg_set_adaptor_test_result(TYPE_SC, 3);
		di->sc_conv_ocp_count++;
		break;
	case DC_FAULT_LTC7820:
		hwlog_err("ltc7820 chip error happened\n");

		snprintf(buf, sizeof(buf), "ltc7820 chip error happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		break;
	case DC_FAULT_INA231:
		hwlog_err("ina231 interrupt happened\n");

		if (di->ls_ops && di->ls_ops->ls_enable)
			di->ls_ops->ls_enable(0);
		break;
	case DC_FAULT_CC_SHORT:
		hwlog_err("typec cc vbus short happened\n");
		break;
	default:
		hwlog_err("unknown fault: %d happened\n", di->charge_fault);
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
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_ADAPTER_DETECT, adaptor_detect),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_LOADSWITCH_ID, loadswitch_id),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_LOADSWITCH_NAME, loadswitch_name),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_VBAT, vbat),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_IBAT, ibat),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_VADAPT, vadapt),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_IADAPT, iadapt),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_LS_VBUS, ls_vbus),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_LS_IBUS, ls_ibus),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_FULL_PATH_RESISTANCE, full_path_resistance),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_DIRECT_CHARGE_SUCC, direct_charge_succ),
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_SET_RESISTANCE_THRESHOLD, set_resistance_threshold),
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_SET_CHARGETYPE_PRIORITY, set_chargetype_priority),
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_AF, af),
};

#define SC_SYSFS_ATTRS_SIZE  ARRAY_SIZE(sc_sysfs_field_tbl)

static struct attribute *sc_sysfs_attrs[SC_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group sc_sysfs_attr_group = {
	.attrs = sc_sysfs_attrs,
};

static void sc_sysfs_init_attrs(void)
{
	int i;

	for (i = 0; i < SC_SYSFS_ATTRS_SIZE; i++)
		sc_sysfs_attrs[i] = &sc_sysfs_field_tbl[i].attr.attr;

	sc_sysfs_attrs[i] = NULL;
}

static struct power_sysfs_attr_info *sc_sysfs_field_lookup(const char *name)
{
	int i;

	for (i = 0; i < SC_SYSFS_ATTRS_SIZE; i++) {
		if (!strncmp(name, sc_sysfs_field_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}

	if (i >= SC_SYSFS_ATTRS_SIZE)
		return NULL;

	return &sc_sysfs_field_tbl[i];
}

static ssize_t sc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct direct_charge_device *di = dev_get_drvdata(dev);
	enum huawei_usb_charger_type type = charge_get_charger_type();
	int chg_state = direct_charge_in_charging_stage();
	int ret;
	int len;

	info = sc_sysfs_field_lookup(attr->attr.name);
	if (!info || !di) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case DC_SYSFS_IIN_THERMAL:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->sysfs_iin_thermal);
		break;
	case DC_SYSFS_ADAPTER_DETECT:
		ret = ADAPTER_DETECT_FAIL;
		if (adapter_get_protocol_register_state(ADAPTER_PROTOCOL_SCP)) {
			len = snprintf(buf, PAGE_SIZE, "%d\n", ret);
			break;
		}

		if ((type == CHARGER_TYPE_STANDARD) ||
			(type == CHARGER_REMOVED &&
			chg_state == DC_IN_CHARGING_STAGE)) {
			if (di->adapter_detect_flag == DC_ADAPTER_DETECT)
				ret = 0;
			else
				ret = ADAPTER_DETECT_FAIL;
		}

		len = snprintf(buf, PAGE_SIZE, "%d\n", ret);
		break;
	case DC_SYSFS_LOADSWITCH_ID:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->ls_id);
		break;
	case DC_SYSFS_LOADSWITCH_NAME:
		len = snprintf(buf, PAGE_SIZE, "%s\n",
			!di->ls_name ? "ERROR" : di->ls_name);
		break;
	case DC_SYSFS_VBAT:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->vbat);
		break;
	case DC_SYSFS_IBAT:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->ibat);
		break;
	case DC_SYSFS_VADAPT:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->vadapt);
		break;
	case DC_SYSFS_IADAPT:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->iadapt);
		break;
	case DC_SYSFS_LS_VBUS:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->ls_vbus);
		break;
	case DC_SYSFS_LS_IBUS:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->ls_ibus);
		break;
	case DC_SYSFS_FULL_PATH_RESISTANCE:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			di->full_path_resistance);
		break;
	case DC_SYSFS_DIRECT_CHARGE_SUCC:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->dc_succ_flag);
		break;
	case DC_SYSFS_AF:
		len = sc_get_antifake_data(buf, DC_AF_KEY_LEN);
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

	info = sc_sysfs_field_lookup(attr->attr.name);
	if (!info || !di) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case DC_SYSFS_IIN_THERMAL:
		/* 10: decimal base */
		if (kstrtol(buf, 10, &val) < 0)
			return -EINVAL;

		sc_set_iin_limit((unsigned int)val);
		break;
	case DC_SYSFS_SET_RESISTANCE_THRESHOLD:
		/* 10: decimal base */
		if ((kstrtol(buf, 10, &val) < 0) ||
			(val < 0) || (val > DC_MAX_RESISTANCE))
			return -EINVAL;

		hwlog_info("set resistance_threshold=%ld\n", val);

		di->std_cable_full_path_res_max = val;
		di->nonstd_cable_full_path_res_max = val;
		di->ctc_cable_full_path_res_max = val;
		break;
	case DC_SYSFS_SET_CHARGETYPE_PRIORITY:
		/* 10: decimal base */
		if ((kstrtol(buf, 10, &val) < 0) ||
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
	case DC_SYSFS_AF:
		/* 10: decimal base, must be 0 or 1 */
		if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 1))
			return -EINVAL;

		sc_set_antifake_result(val);
		break;
	default:
		break;
	}

	return count;
}

static void sc_sysfs_create_group(struct device *dev)
{
	sc_sysfs_init_attrs();
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

static int sc_probe(struct platform_device *pdev)
{
	int ret;
	struct direct_charge_device *di = NULL;
	struct device_node *np = NULL;
	struct direct_charge_pps_ops *l_pps_ops = NULL;
	struct direct_charge_cd_ops *l_cd_ops = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;
	di->orig_volt_para_p = g_sc_volt_para;

	ret = direct_charge_parse_dts(np, di);
	if (ret)
		goto fail_free_mem;

	direct_charge_get_g_pps_ops(&l_pps_ops);
	direct_charge_get_g_cd_ops(&l_cd_ops);

	di->ls_ops = g_sc_ops;
	di->bi_ops = g_bi_sc_ops;
	di->pps_ops = l_pps_ops;
	di->cd_ops = l_cd_ops;
	di->sysfs_enable_charger = 1;
	di->dc_stage = DC_STAGE_DEFAULT;
	di->sysfs_iin_thermal = di->iin_thermal_default;
	di->max_adapter_iset = di->iin_thermal_default;
	di->dc_succ_flag = DC_ERROR_ADAPTER_DETECT;
	di->scp_stop_charging_complete_flag = 1;
	di->dc_err_report_flag = FALSE;
	di->last_basp_level = BASP_PARA_LEVEL;
	di->bat_temp_err_flag = false;
	di->rt_test_succ = false;
	di->sc_conv_ocp_count = 0;
	direct_charge_set_local_mode(OR_SET, SC_MODE);

	platform_set_drvdata(pdev, di);

	di->charging_wq = create_singlethread_workqueue("sc_charging_wq");
	di->kick_wtd_wq = create_singlethread_workqueue("sc_wtd_wq");

	wakeup_source_init(&di->charging_lock, "sc_wakelock");
	INIT_WORK(&di->calc_thld_work, direct_charge_calc_thld_work);
	INIT_WORK(&di->control_work, direct_charge_control_work);
	INIT_WORK(&di->fault_work, sc_fault_work);
	INIT_WORK(&di->kick_wtd_work, direct_charge_kick_wtd_work);

	hrtimer_init(&di->calc_thld_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->calc_thld_timer.function = direct_charge_calc_thld_timer_func;
	hrtimer_init(&di->control_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->control_timer.function = direct_charge_control_timer_func;
	hrtimer_init(&di->kick_wtd_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->kick_wtd_timer.function = direct_charge_kick_wtd_timer_func;

	init_completion(&di->dc_af_completion);

	sc_sysfs_create_group(di->dev);
	g_sc_di = di;
	direct_charge_set_di(di);

	di->fault_nb.notifier_call = direct_charge_fault_notifier_call;
	ret = atomic_notifier_chain_register(&sc_fault_notifier_list,
		&di->fault_nb);
	if (ret < 0) {
		hwlog_err("register notifier failed\n");
		goto fail_create_link;
	}

	sc_get_devices_info(di);
	power_if_ops_register(&sc_if_ops);

	return 0;

fail_create_link:
	sc_sysfs_remove_group(di->dev);
	wakeup_source_trash(&di->charging_lock);
fail_free_mem:
	devm_kfree(&pdev->dev, di);
	di = NULL;
	g_sc_di = NULL;

	return ret;
}

static int sc_remove(struct platform_device *pdev)
{
	struct direct_charge_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

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
