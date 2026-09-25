/*
 * direct_charger_lvc.c
 *
 * direct charger with lvc (load switch) driver
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

#define HWLOG_TAG direct_charge_lvc
HWLOG_REGIST();

static struct direct_charge_device *g_lvc_di;
static struct loadswitch_ops *g_lvc_ops;
static struct batinfo_ops *g_bi_lvc_ops;
static struct direct_charge_volt_para_group g_lvc_volt_para[DC_VOLT_GROUP_MAX];

static const char * const g_lvc_device_name[] = {
	[LOADSWITCH_RICHTEK] = "RT9748",
	[LOADSWITCH_TI] = "BQ25870",
	[LOADSWITCH_FAIRCHILD] = "FAN54161",
	[LOADSWITCH_NXP] = "PCA9498",
	[LOADSWITCH_SCHARGERV600] = "HI6526",
	[LOADSWITCH_FPF2283] = "FPF2283",
	[LOADSWITCH_TOTAL] = "ERROR",
};

struct atomic_notifier_head lvc_fault_notifier_list;
ATOMIC_NOTIFIER_HEAD(lvc_fault_notifier_list);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
static void lvc_get_devices_info(struct direct_charge_device *di)
{
	int ret;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!di || !di->ls_ops || !di->ls_ops->ls_init ||
		!di->ls_ops->get_ls_id || !di->ls_ops->ls_exit)
		return;

	ret = di->ls_ops->ls_init();
	di->ls_id = di->ls_ops->get_ls_id();
	if ((di->ls_id < LOADSWITCH_BEGIN) || (di->ls_id >= LOADSWITCH_TOTAL))
		di->ls_id = LOADSWITCH_TOTAL;
	di->ls_name = g_lvc_device_name[di->ls_id];
	hwlog_info("loadswitch id=%d, %s\n", di->ls_id, di->ls_name);
	ret |= di->ls_ops->ls_exit();
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

static int lvc_get_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -1;
	}

	*val = di->sysfs_iin_thermal;
	return 0;
}

static int lvc_set_ichg_ratio(unsigned int val)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	di->ichg_ratio = val;
	hwlog_info("set ichg_ratio=%d\n", val);
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
	hwlog_info("set vterm_dec=%d\n", val);
	return 0;
}

static int lvc_get_rt_test_time(unsigned int *val)
{
	int adapter_type = ADAPTER_TYPE_UNKNOWN;
	struct direct_charge_device *di = g_lvc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	/* 10v2a&10v2.25a adapter not support lvc, close lvc test */
	adapter_get_adp_type(ADAPTER_PROTOCOL_SCP, &adapter_type);
	if ((adapter_type == ADAPTER_TYPE_10V2A) ||
		(adapter_type == ADAPTER_TYPE_10V2P25A)) {
		*val = 0;
		hwlog_info("use 10v2a or 10v2.25a adapter, close lvc test\n");
		return 0;
	}

	*val = di->rt_test_time;
	return 0;
}

static int lvc_get_rt_test_result(unsigned int *val)
{
	int ibat;
	int iin_thermal_th;
	struct direct_charge_device *di = g_lvc_di;

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

static struct power_if_ops lvc_if_ops = {
	.set_enable_charger = lvc_set_enable_charger,
	.get_enable_charger = lvc_get_enable_charger,
	.set_iin_limit = lvc_set_iin_limit,
	.get_iin_limit = lvc_get_iin_limit,
	.set_ichg_ratio = lvc_set_ichg_ratio,
	.set_vterm_dec = lvc_set_vterm_dec,
	.get_rt_test_time = lvc_get_rt_test_time,
	.get_rt_test_result = lvc_get_rt_test_result,
	.get_hota_iin_limit = lvc_get_hota_iin_limit,
	.get_startup_iin_limit = lvc_get_startup_iin_limit,
	.type_name = "lvc",
};

int lvc_ops_register(struct loadswitch_ops *ops)
{
	int ret = 0;

	if (ops)
		g_lvc_ops = ops;
	else
		ret = -EINVAL;

	return ret;
}

int lvc_batinfo_ops_register(struct batinfo_ops *ops)
{
	int ret = 0;

	if (ops)
		g_bi_lvc_ops = ops;
	else
		ret = -EINVAL;

	return ret;
}

int lvc_get_di(struct direct_charge_device **di)
{
	if (!g_lvc_di || !di) {
		hwlog_err("g_lvc_di or di is null\n");
		return -1;
	}

	*di = g_lvc_di;

	return 0;
}

void lvc_get_fault_notifier(struct atomic_notifier_head **notifier)
{
	if (!notifier) {
		hwlog_err("notifier is null\n");
		return;
	}

	*notifier = &lvc_fault_notifier_list;
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

	hwlog_info("set_disable_flag val=%d, type=%d, disable=%d\n",
		val, type, disable);
	return 0;
}

void lvc_mode_check(void)
{
	unsigned int local_mode;
	unsigned int stage;
	struct direct_charge_device *di = g_lvc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	/* step-1: check local mode */
	local_mode = direct_charge_get_local_mode();
	if (!(local_mode & LVC_MODE)) {
		hwlog_err("local mode not support lvc\n");
		return;
	}

	/* step-2: check lvc error and report dmd */
	if (di->error_cnt >= DC_ERR_CNT_MAX) {
		hwlog_info("error exceed %d times, lvc is disabled\n",
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

	/* step-3: check lvc enable status */
	if (di->sysfs_enable_charger == 0) {
		di->dc_succ_flag = DC_ERROR_CHARGE_DISABLED;
		hwlog_info("lvc is disabled\n");
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
		hwlog_err("lvc is disabled by uscp\n");
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

	/* step-11: switch buck charging path to lvc charging path */
	stage = direct_charge_get_stage_status();
	if (stage == DC_STAGE_SWITCH_DETECT) {
		di->scp_stop_charging_complete_flag = 0;
		di->dc_succ_flag = DC_ERROR_SWITCH;
		if (direct_charge_switch_charging_path(PATH_LVC) == 0) {
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

static void lvc_fault_work(struct work_struct *work)
{
	char buf[POWER_DSM_BUF_SIZE_0256] = { 0 };
	char reg_info[POWER_DSM_BUF_SIZE_0128] = { 0 };
	struct direct_charge_device *di = NULL;
	struct nty_data *data = NULL;
	int bat_capacity;

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
			"lvc charge_fault=%d, addr=0x%x, event1=0x%x, event2=0x%x\n",
			di->charge_fault, data->addr,
			data->event1, data->event2);
	else
		snprintf(reg_info, sizeof(reg_info),
			"lvc charge_fault=%d, addr=0x0, event1=0x0, event2=0x0\n",
			di->charge_fault);

	direct_charge_fill_eh_buf(di->dsm_buff, sizeof(di->dsm_buff),
		DC_EH_HAPPEN_LVC_FAULT, reg_info);

	switch (di->charge_fault) {
	case DC_FAULT_VBUS_OVP:
		hwlog_err("vbus ovp happened\n");

		snprintf(buf, sizeof(buf), "vbus ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_BATTERY,
			DSM_DIRECT_CHARGE_VBUS_OVP, buf);
		break;
	case DC_FAULT_REVERSE_OCP:
		bat_capacity = hisi_battery_capacity();
		if (bat_capacity < BATTERY_CAPACITY_HIGH_TH)
			di->reverse_ocp_cnt++;

		hwlog_err("reverse ocp happened, capacity=%d, ocp_cnt=%d\n",
			bat_capacity, di->reverse_ocp_cnt);

		if (di->reverse_ocp_cnt >= REVERSE_OCP_CNT) {
			di->reverse_ocp_cnt = REVERSE_OCP_CNT;
			snprintf(buf, sizeof(buf), "reverse ocp happened\n");
			strncat(buf, reg_info, strlen(reg_info));
			power_dsm_dmd_report(POWER_DSM_BATTERY,
				DSM_DIRECT_CHARGE_REVERSE_OCP, buf);
		}
		break;
	case DC_FAULT_OTP:
		hwlog_err("otp happened\n");

		di->otp_cnt++;
		if (di->otp_cnt >= OTP_CNT) {
			di->otp_cnt = OTP_CNT;
			snprintf(buf, sizeof(buf), "otp happened\n");
			strncat(buf, reg_info, strlen(reg_info));
			power_dsm_dmd_report(POWER_DSM_BATTERY,
				DSM_DIRECT_CHARGE_OTP, buf);
		}
		break;
	case DC_FAULT_INPUT_OCP:
		hwlog_err("input ocp happened\n");

		snprintf(buf, sizeof(buf), "input ocp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_BATTERY,
			DSM_DIRECT_CHARGE_INPUT_OCP, buf);
		break;
	case DC_FAULT_VDROP_OVP:
		hwlog_err("vdrop ovp happened\n");

		snprintf(buf, sizeof(buf), "vdrop ovp happened\n");
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
static ssize_t lvc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t lvc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info lvc_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(lvc, 0644, DC_SYSFS_IIN_THERMAL, iin_thermal),
	power_sysfs_attr_ro(lvc, 0444, DC_SYSFS_ADAPTER_DETECT, adaptor_detect),
	power_sysfs_attr_ro(lvc, 0444, DC_SYSFS_LOADSWITCH_ID, loadswitch_id),
	power_sysfs_attr_ro(lvc, 0444, DC_SYSFS_LOADSWITCH_NAME, loadswitch_name),
	power_sysfs_attr_ro(lvc, 0444, DC_SYSFS_VBAT, vbat),
	power_sysfs_attr_ro(lvc, 0444, DC_SYSFS_IBAT, ibat),
	power_sysfs_attr_ro(lvc, 0444, DC_SYSFS_VADAPT, vadapt),
	power_sysfs_attr_ro(lvc, 0444, DC_SYSFS_IADAPT, iadapt),
	power_sysfs_attr_ro(lvc, 0444, DC_SYSFS_LS_VBUS, ls_vbus),
	power_sysfs_attr_ro(lvc, 0444, DC_SYSFS_LS_IBUS, ls_ibus),
	power_sysfs_attr_ro(lvc, 0444, DC_SYSFS_FULL_PATH_RESISTANCE, full_path_resistance),
	power_sysfs_attr_ro(lvc, 0444, DC_SYSFS_DIRECT_CHARGE_SUCC, direct_charge_succ),
	power_sysfs_attr_rw(lvc, 0644, DC_SYSFS_SET_RESISTANCE_THRESHOLD, set_resistance_threshold),
};

#define LVC_SYSFS_ATTRS_SIZE  ARRAY_SIZE(lvc_sysfs_field_tbl)

static struct attribute *lvc_sysfs_attrs[LVC_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group lvc_sysfs_attr_group = {
	.attrs = lvc_sysfs_attrs,
};

static void lvc_sysfs_init_attrs(void)
{
	int i;

	for (i = 0; i < LVC_SYSFS_ATTRS_SIZE; i++)
		lvc_sysfs_attrs[i] = &lvc_sysfs_field_tbl[i].attr.attr;

	lvc_sysfs_attrs[i] = NULL;
}

static struct power_sysfs_attr_info *lvc_sysfs_field_lookup(const char *name)
{
	int i;

	for (i = 0; i < LVC_SYSFS_ATTRS_SIZE; i++) {
		if (!strncmp(name, lvc_sysfs_field_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}

	if (i >= LVC_SYSFS_ATTRS_SIZE)
		return NULL;

	return &lvc_sysfs_field_tbl[i];
}

static ssize_t lvc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct direct_charge_device *di = dev_get_drvdata(dev);
	enum huawei_usb_charger_type type = charge_get_charger_type();
	int chg_state = direct_charge_in_charging_stage();
	int ret;
	int len;

	info = lvc_sysfs_field_lookup(attr->attr.name);
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

	info = lvc_sysfs_field_lookup(attr->attr.name);
	if (!info || !di) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case DC_SYSFS_IIN_THERMAL:
		/* 10: decimal base */
		if (kstrtol(buf, 10, &val) < 0)
			return -EINVAL;

		lvc_set_iin_limit((unsigned int)val);
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
	default:
		break;
	}

	return count;
}

static void lvc_sysfs_create_group(struct device *dev)
{
	lvc_sysfs_init_attrs();
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

static int lvc_probe(struct platform_device *pdev)
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
	di->orig_volt_para_p = g_lvc_volt_para;

	ret = direct_charge_parse_dts(np, di);
	if (ret)
		goto fail_free_mem;

	direct_charge_get_g_pps_ops(&l_pps_ops);
	direct_charge_get_g_cd_ops(&l_cd_ops);

	di->ls_ops = g_lvc_ops;
	di->bi_ops = g_bi_lvc_ops;
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
	direct_charge_set_local_mode(OR_SET, LVC_MODE);

	platform_set_drvdata(pdev, di);

	di->charging_wq = create_singlethread_workqueue("lvc_charging_wq");
	di->kick_wtd_wq = create_singlethread_workqueue("lvc_wtd_wq");

	wakeup_source_init(&di->charging_lock, "lvc_wakelock");
	INIT_WORK(&di->calc_thld_work, direct_charge_calc_thld_work);
	INIT_WORK(&di->control_work, direct_charge_control_work);
	INIT_WORK(&di->fault_work, lvc_fault_work);
	INIT_WORK(&di->kick_wtd_work, direct_charge_kick_wtd_work);

	hrtimer_init(&di->calc_thld_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->calc_thld_timer.function = direct_charge_calc_thld_timer_func;
	hrtimer_init(&di->control_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->control_timer.function = direct_charge_control_timer_func;
	hrtimer_init(&di->kick_wtd_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->kick_wtd_timer.function = direct_charge_kick_wtd_timer_func;

	init_completion(&di->dc_af_completion);

	lvc_sysfs_create_group(di->dev);
	g_lvc_di = di;
	direct_charge_set_di(di);

	di->fault_nb.notifier_call = direct_charge_fault_notifier_call;
	ret = atomic_notifier_chain_register(&lvc_fault_notifier_list,
		&di->fault_nb);
	if (ret < 0) {
		hwlog_err("register notifier failed\n");
		goto fail_create_link;
	}

	lvc_get_devices_info(di);
	power_if_ops_register(&lvc_if_ops);

	return 0;

fail_create_link:
	lvc_sysfs_remove_group(di->dev);
	wakeup_source_trash(&di->charging_lock);
fail_free_mem:
	devm_kfree(&pdev->dev, di);
	di = NULL;
	g_lvc_di = NULL;

	return ret;
}

static int lvc_remove(struct platform_device *pdev)
{
	struct direct_charge_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

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
