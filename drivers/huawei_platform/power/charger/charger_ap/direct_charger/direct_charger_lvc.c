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

#include <huawei_platform/power/direct_charger.h>
#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
#include <huawei_platform/power/usb_short_circuit_protect.h>
#endif

#define HWLOG_TAG direct_charge_lvc
HWLOG_REGIST();

static struct direct_charge_device *g_lvc_di;
static struct loadswitch_ops *g_lvc_ops;
static struct batinfo_ops *g_bi_lvc_ops;
static struct direct_charge_volt_para_group g_lvc_volt_para[DC_VOLT_GROUP_MAX];

static const char * const lvc_device_name[] = {
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
	if ((val < 0) || (val > 8000)) {
		hwlog_err("invalid val=%u\n", val);
		return -1;
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

	hwlog_info("set input currrent is:%u, limit current is %d\n",
		val, di->sysfs_iin_thermal);
	return 0;
}

static int lvc_get_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_lvc_di;

	if (!di) {
		hwlog_err("di is null\n");
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
	struct direct_charge_device *di = g_lvc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
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

/* define public power interface */
static struct power_if_ops lvc_power_if_ops = {
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

	/* first: check local mode */
	local_mode = direct_charge_get_local_mode();
	if (!(local_mode & LVC_MODE)) {
		hwlog_err("local mode not support lvc\n");
		return;
	}

	/* second: check ops */
	if (direct_charge_ops_is_valid(di) == INVALID)
		return;

	/* third: check lvc error and report dmd */
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

	/* fourth: check lvc enable status */
	if (di->sysfs_enable_charger == 0) {
		di->dc_succ_flag = DC_ERROR_CHARGE_DISABLED;
		hwlog_info("lvc is disabled\n");
		return;
	}

	/* fifth: check scp protocol status */
	stage = direct_charge_get_stage_status();
	if (stage == DC_STAGE_DEFAULT) {
		if (direct_charge_get_protocol_register_state())
			return;
	}

#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
	/* sixth: check uscp status */
	if (is_in_uscp_mode()) {
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		di->dc_succ_flag = DC_ERROR_CHARGE_DISABLED;

		hwlog_err("lvc is disabled by uscp\n");
		return;
	}
#endif /* CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT */

	/* seventh: detect cable */
	stage = direct_charge_get_stage_status();
	if (stage == DC_STAGE_ADAPTER_DETECT) {
		di->dc_succ_flag = DC_ERROR_ADAPTER_DETECT;
		di->adapter_detect_flag = DC_ADAPTER_DETECT;
		direct_charge_detect_cable();
	}

	/* eighth: check battery temp */
	if (!direct_charge_pre_check_battery_temp()) {
		di->dc_succ_flag = DC_ERROR_BAT_TEMP;
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		di->bat_temp_err_flag = true;

		hwlog_err("temp out of range, try next loop\n");
		return;
	}

	/* ninth: check battery voltage */
	if (!direct_charge_pre_check_battery_voltage()) {
		di->dc_succ_flag = DC_ERROR_BAT_VOL;
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);

		hwlog_err("volt out of range, try next loop\n");
		return;
	}

	/* tenth: check adapter antifake */
	if (direct_charge_check_adapter_antifake()) {
		di->dc_succ_flag = DC_ERROR_ADAPTER_ANTI_FAKE;
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		di->error_cnt += 1;

		hwlog_err("adapter antifake check failed\n");
		return;
	}

	/* eleventh: switch buck charging path to lvc charging path */
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

	/* twelfth: init scp adapter and device */
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

	/* thirteenth: direct charging security check */
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

	/* fourteenth: start direct charging */
	stage = direct_charge_get_stage_status();
	if (stage == DC_STAGE_SUCCESS) {
		di->dc_succ_flag = DC_SUCC;
		direct_charge_start_charging();
	}

	/* fifteenth: end */
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
#define LVC_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, lvc_sysfs_show, store), \
	.name = DC_SYSFS_##n, \
}

#define LVC_SYSFS_FIELD_RW(_name, n) \
	LVC_SYSFS_FIELD(_name, n, 0644, lvc_sysfs_store)

#define LVC_SYSFS_FIELD_RO(_name, n) \
	LVC_SYSFS_FIELD(_name, n, 0444, NULL)

static ssize_t lvc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t lvc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

struct lvc_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static struct lvc_sysfs_field_info lvc_sysfs_field_tbl[] = {
	LVC_SYSFS_FIELD_RW(iin_thermal, IIN_THERMAL),
	LVC_SYSFS_FIELD_RO(adaptor_detect, ADAPTER_DETECT),
	LVC_SYSFS_FIELD_RO(loadswitch_id, LOADSWITCH_ID),
	LVC_SYSFS_FIELD_RO(loadswitch_name, LOADSWITCH_NAME),
	LVC_SYSFS_FIELD_RO(vbat, VBAT),
	LVC_SYSFS_FIELD_RO(ibat, IBAT),
	LVC_SYSFS_FIELD_RO(vadapt, VADAPT),
	LVC_SYSFS_FIELD_RO(iadapt, IADAPT),
	LVC_SYSFS_FIELD_RO(ls_vbus, LS_VBUS),
	LVC_SYSFS_FIELD_RO(ls_ibus, LS_IBUS),
	LVC_SYSFS_FIELD_RO(full_path_resistance, FULL_PATH_RESISTANCE),
	LVC_SYSFS_FIELD_RO(direct_charge_succ, DIRECT_CHARGE_SUCC),
	LVC_SYSFS_FIELD_RW(set_resistance_threshold, SET_RESISTANCE_THRESHOLD),
};

static struct attribute *lvc_sysfs_attrs[ARRAY_SIZE(lvc_sysfs_field_tbl) + 1];

static const struct attribute_group lvc_sysfs_attr_group = {
	.attrs = lvc_sysfs_attrs,
};

static void lvc_sysfs_init_attrs(void)
{
	int i;
	int limit = ARRAY_SIZE(lvc_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		lvc_sysfs_attrs[i] = &lvc_sysfs_field_tbl[i].attr.attr;

	lvc_sysfs_attrs[limit] = NULL;
}

static struct lvc_sysfs_field_info *lvc_sysfs_field_lookup(const char *name)
{
	int i;
	int limit = ARRAY_SIZE(lvc_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name, lvc_sysfs_field_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}

	if (i >= limit)
		return NULL;

	return &lvc_sysfs_field_tbl[i];
}

static ssize_t lvc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lvc_sysfs_field_info *info = NULL;
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
		if (direct_charge_ops_is_valid(di) == INVALID) {
			len = snprintf(buf, PAGE_SIZE, "%d\n", ret);
			break;
		}

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
		hwlog_err("invalid sysfs_name\n");
		len = 0;
		break;
	}

	return len;
}

static ssize_t lvc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lvc_sysfs_field_info *info = NULL;
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
		hwlog_err("invalid sysfs_name\n");
		break;
	}

	return count;
}

static int lvc_sysfs_create_group(struct direct_charge_device *di)
{
	lvc_sysfs_init_attrs();
	return sysfs_create_group(&di->dev->kobj, &lvc_sysfs_attr_group);
}

static inline void lvc_sysfs_remove_group(struct direct_charge_device *di)
{
	sysfs_remove_group(&di->dev->kobj, &lvc_sysfs_attr_group);
}
#else
static inline int lvc_sysfs_create_group(struct direct_charge_device *di)
{
	return 0;
}

static inline void lvc_sysfs_remove_group(struct direct_charge_device *di)
{
}
#endif /* CONFIG_SYSFS */

static int lvc_probe(struct platform_device *pdev)
{
	int ret;
	struct direct_charge_device *di = NULL;
	struct class *power_class = NULL;
	struct device_node *np = NULL;
	struct direct_charge_pps_ops *l_pps_ops = NULL;
	struct direct_charge_cd_ops *l_cd_ops = NULL;
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	struct power_devices_info_data *power_dev_info = NULL;
#endif /* CONFIG_HUAWEI_HW_DEV_DCT */

	hwlog_info("probe begin\n");

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

	if (direct_charge_ops_is_valid(di) == INVALID) {
		ret = -EINVAL;
		goto fail_free_mem;
	}

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

	ret = lvc_sysfs_create_group(di);
	if (ret) {
		hwlog_err("sysfs group create failed\n");
		goto fail_free_mem;
	}

	power_class = hw_power_get_class();
	if (power_class) {
		if (!charge_dev)
			charge_dev = device_create(power_class, NULL, 0, NULL,
			"charger");
		if (IS_ERR(charge_dev)) {
			hwlog_err("sysfs device create failed\n");
			ret = PTR_ERR(charge_dev);
			goto fail_free_mem;
		}

		ret = sysfs_create_link(&charge_dev->kobj, &di->dev->kobj,
			"direct_charger");
		if (ret) {
			hwlog_err("sysfs link create failed\n");
			goto fail_create_link;
		}
	}

	g_lvc_di = di;
	direct_charge_set_di(di);

	di->fault_nb.notifier_call = direct_charge_fault_notifier_call;
	ret = atomic_notifier_chain_register(&lvc_fault_notifier_list,
		&di->fault_nb);
	if (ret < 0) {
		hwlog_err("register notifier failed\n");
		goto fail_create_link;
	}

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	ret = di->ls_ops->ls_init();
	di->ls_id = di->ls_ops->get_ls_id();
	if (di->ls_id < LOADSWITCH_BEGIN || di->ls_id >= LOADSWITCH_TOTAL) {
		hwlog_err("get device id info failed\n");
		di->ls_id = LOADSWITCH_TOTAL;
	}

	di->ls_name = lvc_device_name[di->ls_id];
	hwlog_info("loadswitch id=%d,%s\n", di->ls_id, di->ls_name);
	ret |= di->ls_ops->ls_exit();
	if (ret == 0)
		set_hw_dev_flag(DEV_I2C_LOADSWITCH);

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->ls_name;
		power_dev_info->dev_id = di->ls_id;
		power_dev_info->ver_id = 0;
	}
#endif /* CONFIG_HUAWEI_HW_DEV_DCT */

	if (power_if_ops_register(&lvc_power_if_ops))
		hwlog_err("register power_if_ops failed\n");

	hwlog_info("probe end\n");
	return 0;

fail_create_link:
	lvc_sysfs_remove_group(di);
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

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	wakeup_source_trash(&di->charging_lock);
	devm_kfree(&pdev->dev, di);
	di = NULL;
	g_lvc_di = NULL;

	hwlog_info("remove end\n");
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
