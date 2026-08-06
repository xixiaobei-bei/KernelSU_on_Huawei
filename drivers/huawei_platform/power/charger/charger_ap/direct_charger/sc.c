/*
 * sc.c
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

#include <huawei_platform/power/direct_charger.h>
#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
#include <huawei_platform/power/usb_short_circuit_protect.h>
#endif

#define HWLOG_TAG direct_charge_sc
HWLOG_REGIST();

static struct direct_charge_device *g_sc_di;
static struct loadswitch_ops *g_sc_ops;
static struct batinfo_ops *g_bi_sc_ops;
static struct direct_charge_volt_para_group g_sc_volt_para[DC_VOLT_GROUP_MAX];

static const char * const sc_device_name[] = {
	[SWITCHCAP_TI_BQ25970] = "BQ25970",
	[SWITCHCAP_SCHARGERV600] = "HI6526",
	[SWITCHCAP_LTC7820] = "LTC7820",
	[SWITCHCAP_TOTAL] = "ERROR",
};

struct atomic_notifier_head sc_fault_notifier_list;
ATOMIC_NOTIFIER_HEAD(sc_fault_notifier_list);

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

	hwlog_info("set enable_charger=%d\n", di->sysfs_enable_charger);

	ret = set_direct_charger_disable_flags((val ?
		DIRECT_CHARGER_CLEAR_DISABLE_FLAGS :
		DIRECT_CHARGER_SET_DISABLE_FLAGS),
		DIRECT_CHARGER_SYS_NODE);
	if (ret)
		hwlog_err("set direct charge disable flags failed\n");

	return 0;
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

/* define public power interface */
static struct power_if_ops sc_power_if_ops = {
	.set_enable_charger = sc_set_enable_charger,
	.get_enable_charger = sc_get_enable_charger,
	.type_name = "sc",
};

int sc_ops_register(struct loadswitch_ops *ops)
{
	int ret = 0;

	if (ops)
		g_sc_ops = ops;
	else
		ret = -EPERM;

	return ret;
}

int sc_batinfo_ops_register(struct batinfo_ops *ops)
{
	int ret = 0;

	if (ops)
		g_bi_sc_ops = ops;
	else
		ret = -EPERM;

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

void sc_mode_check(void)
{
	int local_mode;
	struct direct_charge_device *di = g_sc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	/* first: check local mode */
	local_mode = direct_charge_get_local_mode();
	if (!(local_mode & SC_MODE)) {
		hwlog_err("local mode not support sc\n");
		return;
	}

	/* second: check ops */
	if (direct_charge_ops_is_valid(di) == INVALID)
		return;

	/* third: check sc error and report dmd */
	if (di->error_cnt >= DC_ERR_CNT_MAX) {
		hwlog_info("error exceed %d times, sc is disabled\n",
			DC_ERR_CNT_MAX);

		di->direct_charge_succ_flag = DC_ERROR_CHARGE_DISABLED;
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

	/* fourth: check sc enable status */
	if (di->sysfs_enable_charger == 0) {
		di->direct_charge_succ_flag = DC_ERROR_CHARGE_DISABLED;
		direct_charge_send_normal_charging_uevent();

		hwlog_info("sc is disabled\n");
		return;
	}

	/* fifth: check scp protocol status */
	if (direct_charge_get_stage_status() == SCP_STAGE_DEFAULT) {
		if (is_support_scp()) {
			hwlog_err("not support scp protocol\n");
			return;
		}
	}

#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
	/* sixth: check uscp status */
	if (is_in_uscp_mode()) {
		direct_charge_set_stage_status(SCP_STAGE_DEFAULT);
		di->direct_charge_succ_flag = DC_ERROR_CHARGE_DISABLED;

		hwlog_err("sc is disabled by uscp\n");
		return;
	}
#endif /* CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT */

	/* seventh: detect cable */
	if (direct_charge_get_stage_status() == SCP_STAGE_ADAPTER_DETECT) {
		di->direct_charge_succ_flag = DC_ERROR_ADAPTOR_DETECT;
		di->scp_adaptor_detect_flag = SCP_ADAPTOR_DETECT;
		scp_cable_detect();
	}

	/* eighth: check battery temp */
	if (!direct_charge_pre_check_battery_temp()) {
		di->direct_charge_succ_flag = DC_ERROR_BAT_TEMP;
		direct_charge_set_stage_status(SCP_STAGE_DEFAULT);

		hwlog_err("temp out of range, try next loop\n");
		return;
	}

	/* ninth: check battery voltage */
	if (!direct_charge_pre_check_battery_voltage()) {
		di->direct_charge_succ_flag = DC_ERROR_BAT_VOL;
		direct_charge_set_stage_status(SCP_STAGE_DEFAULT);

		hwlog_err("volt out of range, try next loop\n");
		return;
	}

	/* tenth: check adapter antifake */
	if (do_adpator_antifake_check()) {
		di->direct_charge_succ_flag = DC_ERROR_ANTI_FAKE_ADAPTOR;
		direct_charge_set_stage_status(SCP_STAGE_DEFAULT);
		di->error_cnt += 1;

		hwlog_err("adapter antifake check failed\n");
		return;
	}

	/* eleventh: switch buck charging path to sc charging path */
	if (direct_charge_get_stage_status() == SCP_STAGE_SWITCH_DETECT) {
		di->scp_stop_charging_complete_flag = 0;
		di->direct_charge_succ_flag = DC_ERROR_SWITCH;
		if (cutoff_normal_charge() == SUCC) {
			direct_charge_set_stage_status(SCP_STAGE_CHARGE_INIT);
		} else {
			hwlog_err("direct_charge path switch failed\n");
			direct_charge_set_stop_charging_flag(1);
			scp_stop_charging();
		}
	}

	/* twelfth: init scp adapter and device */
	if (direct_charge_get_stage_status() == SCP_STAGE_CHARGE_INIT) {
		di->direct_charge_succ_flag = DC_ERROR_INIT;
		if (direct_charge_init_adapter_and_device() == SUCC) {
			direct_charge_set_stage_status(SCP_STAGE_SECURITY_CHECK);
		} else {
			hwlog_err("direct_charge init failed\n");
			direct_charge_set_stop_charging_flag(1);
			scp_stop_charging();
		}
	}

	/* thirteenth: direct charging security check */
	if (direct_charge_get_stage_status() == SCP_STAGE_SECURITY_CHECK) {
		if (direct_charge_security_check() == 0) {
			direct_charge_set_stage_status(SCP_STAGE_SUCCESS);
		} else {
			hwlog_err("direct_charge security check failed\n");
			direct_charge_set_stop_charging_flag(1);
			scp_stop_charging();
		}
	}

	/* fourteenth: start direct charging */
	if (direct_charge_get_stage_status() == SCP_STAGE_SUCCESS) {
		di->direct_charge_succ_flag = DC_SUCC;
		scp_start_charging();
	}

	/* fifteenth: end */
	if (direct_charge_get_stage_status() == SCP_STAGE_CHARGING)
		hwlog_info("in direct_charge process\n");

	hwlog_info("direct_charge stage(%d,%s)\n",
		direct_charge_get_stage_status(),
		scp_check_stage[direct_charge_get_stage_status()]);
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
		/*
		 * power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
		 * DSM_DIRECT_CHARGE_VDROP_OVP, buf);
		 */
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
		/*
		 * power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
		 * DSM_DIRECT_CHARGE_SC_FAULT_IBUS_OCP, buf);
		 */
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
#define SC_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, sc_sysfs_show, store), \
	.name = DC_SYSFS_##n, \
}

#define SC_SYSFS_FIELD_RW(_name, n) \
	SC_SYSFS_FIELD(_name, n, 0644, sc_sysfs_store)

#define SC_SYSFS_FIELD_RO(_name, n) \
	SC_SYSFS_FIELD(_name, n, 0444, NULL)

static ssize_t sc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t sc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

struct sc_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static struct sc_sysfs_field_info sc_sysfs_field_tbl[] = {
	SC_SYSFS_FIELD_RW(enable_charger, ENABLE_CHARGER),
	SC_SYSFS_FIELD_RW(iin_thermal, IIN_THERMAL),
	SC_SYSFS_FIELD_RO(adaptor_detect, ADAPTOR_DETECT),
	SC_SYSFS_FIELD_RO(loadswitch_id, LOADSWITCH_ID),
	SC_SYSFS_FIELD_RO(loadswitch_name, LOADSWITCH_NAME),
	SC_SYSFS_FIELD_RO(vbat, VBAT),
	SC_SYSFS_FIELD_RO(ibat, IBAT),
	SC_SYSFS_FIELD_RO(vadapt, VADAPT),
	SC_SYSFS_FIELD_RO(iadapt, IADAPT),
	SC_SYSFS_FIELD_RO(ls_vbus, LS_VBUS),
	SC_SYSFS_FIELD_RO(ls_ibus, LS_IBUS),
	SC_SYSFS_FIELD_RO(full_path_resistance, FULL_PATH_RESISTANCE),
	SC_SYSFS_FIELD_RO(direct_charge_succ, DIRECT_CHARGE_SUCC),
	SC_SYSFS_FIELD_RW(set_resistance_threshold, SET_RESISTANCE_THRESHOLD),
	SC_SYSFS_FIELD_RW(set_chargetype_priority, SET_CHARGETYPE_PRIORITY),
	SC_SYSFS_FIELD_RO(direct_charge_type, DIRECT_CHARGE_TYPE),
};

static struct attribute *sc_sysfs_attrs[ARRAY_SIZE(sc_sysfs_field_tbl) + 1];

static const struct attribute_group sc_sysfs_attr_group = {
	.attrs = sc_sysfs_attrs,
};

static void sc_sysfs_init_attrs(void)
{
	int i;
	int limit = ARRAY_SIZE(sc_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		sc_sysfs_attrs[i] = &sc_sysfs_field_tbl[i].attr.attr;

	sc_sysfs_attrs[limit] = NULL;
}

static struct sc_sysfs_field_info *sc_sysfs_field_lookup(const char *name)
{
	int i;
	int limit = ARRAY_SIZE(sc_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name, sc_sysfs_field_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}

	if (i >= limit)
		return NULL;

	return &sc_sysfs_field_tbl[i];
}

static ssize_t sc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sc_sysfs_field_info *info = NULL;
	struct direct_charge_device *di = dev_get_drvdata(dev);
	enum huawei_usb_charger_type type = charge_get_charger_type();
	int ret;
	int len;

	info = sc_sysfs_field_lookup(attr->attr.name);
	if (!info || !di) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case DC_SYSFS_ENABLE_CHARGER:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			di->sysfs_enable_charger);
		break;

	case DC_SYSFS_IIN_THERMAL:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->sysfs_iin_thermal);
		break;

	case DC_SYSFS_ADAPTOR_DETECT:
		ret = SCP_ADAPTOR_DETECT_FAIL;
		if (direct_charge_ops_is_valid(di) == INVALID) {
			len = snprintf(buf, PAGE_SIZE, "%d\n", ret);
			break;
		}

		if (adapter_get_protocol_register_state()) {
			len = snprintf(buf, PAGE_SIZE, "%d\n", ret);
			break;
		}

		if ((type == CHARGER_TYPE_STANDARD) ||
			(type == CHARGER_REMOVED &&
			is_in_scp_charging_stage() == IN_SCP_CHARGING_STAGE)) {
			if (di->scp_adaptor_detect_flag == SCP_ADAPTOR_DETECT)
				ret = 0;
			else
				ret = ADAPTOR_SCP_DETECT_FAIL;
		}

		hwlog_info("adaptor_detect=%d\n", ret);
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
		hwlog_info("sc_succ_flag=%d\n", di->direct_charge_succ_flag);
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			di->direct_charge_succ_flag);
		break;
	case DC_SYSFS_DIRECT_CHARGE_TYPE:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
		direct_charge_get_adapter_type());
		break;

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		len = 0;
		break;
	}

	return len;
}

static ssize_t sc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct sc_sysfs_field_info *info = NULL;
	struct direct_charge_device *di = dev_get_drvdata(dev);
	long val = 0;
	int ret;
	int index;
	int cur_low;

	info = sc_sysfs_field_lookup(attr->attr.name);
	if (!info || !di) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case DC_SYSFS_ENABLE_CHARGER:
		/* 10: decimal base, the value only 0 or 1 */
		if ((kstrtol(buf, 10, &val) < 0) ||
			(val < 0) || (val > 1)) {
			hwlog_err("get kstrtoint failed\n");
			return -EINVAL;
		}

		ret = set_direct_charger_disable_flags((val ?
			DIRECT_CHARGER_CLEAR_DISABLE_FLAGS :
			DIRECT_CHARGER_SET_DISABLE_FLAGS),
			DIRECT_CHARGER_SYS_NODE);
		if (ret)
			hwlog_err("set direct charge disable flags failed");

		hwlog_info("set enable_charger=%d\n", di->sysfs_enable_charger);
		break;

	case DC_SYSFS_IIN_THERMAL:
		/* 10: decimal base, the value must be (0, 8000) ma */
		if ((kstrtol(buf, 10, &val) < 0) ||
			(val < 0) || (val > 8000)) {
			hwlog_err("get kstrtoint failed\n");
			return -EINVAL;
		}

		if (di->stage_size > DC_VOLT_LEVEL) {
			hwlog_err("invalid stage_size=%d\n", di->stage_size);
			return -EINVAL;
		}

		index = di->stage_size - 1;
		cur_low = di->orig_volt_para_p[0].volt_info[index].cur_th_low;

		hwlog_info("set iin_thermal=%ld, cur_low=%d\n", val, cur_low);

		if (val == 0)
			di->sysfs_iin_thermal = di->iin_thermal_default;
		else if (val < cur_low)
			di->sysfs_iin_thermal = cur_low;
		else
			di->sysfs_iin_thermal = val;

		break;

	case DC_SYSFS_SET_RESISTANCE_THRESHOLD:
		/* 10: decimal base */
		if ((kstrtol(buf, 10, &val) < 0) ||
			(val < 0) || (val > MAX_RESISTANCE)) {
			hwlog_err("get kstrtoint failed\n");
			return -EINVAL;
		}

		hwlog_info("set resistance_threshold=%ld\n", val);

		di->standard_cable_full_path_res_max = val;
		di->full_path_res_max = val;
		di->ctc_cable_full_path_res_max = val;
		break;

	case DC_SYSFS_SET_CHARGETYPE_PRIORITY:
		/* 10: decimal base */
		if ((kstrtol(buf, 10, &val) < 0) ||
			(val < 0) || (val > MAX_RESISTANCE)) {
			hwlog_err("get kstrtoint failed\n");
			return -EINVAL;
		}

		hwlog_info("set chargertype_priority=%ld\n", val);

		if (val == SCP_CHARGE_TYPE_SC)
			direct_charge_set_local_mode(OR_SET, SC_MODE);
		else if (val == SCP_CHARGE_TYPE_LVC)
			direct_charge_set_local_mode(AND_SET, LVC_MODE);
		else
			hwlog_info("invalid chargertype priority\n");
		break;

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		break;
	}

	return count;
}

static inline int sc_sysfs_create_group(struct direct_charge_device *di)
{
	sc_sysfs_init_attrs();

	return sysfs_create_group(&di->dev->kobj, &sc_sysfs_attr_group);
}

static inline void sc_sysfs_remove_group(struct direct_charge_device *di)
{
	sysfs_remove_group(&di->dev->kobj, &sc_sysfs_attr_group);
}

#else

static inline int sc_sysfs_create_group(struct direct_charge_device *di)
{
	return 0;
}

static inline void sc_sysfs_remove_group(struct direct_charge_device *di)
{
}
#endif /* CONFIG_SYSFS */

static int sc_probe(struct platform_device *pdev)
{
	int ret;
	struct direct_charge_device *di = NULL;
	struct class *power_class = NULL;
	struct device_node *np = NULL;
	struct direct_charge_pps_ops *l_pps_ops = NULL;
	struct direct_charge_cd_ops *l_cd_ops = NULL;

	hwlog_info("probe begin\n");

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
	di->scp_stage = SCP_STAGE_DEFAULT;
	di->sysfs_iin_thermal = di->iin_thermal_default;
	di->max_adaptor_iset = di->iin_thermal_default;
	di->direct_charge_succ_flag = DC_ERROR_ADAPTOR_DETECT;
	di->scp_stop_charging_complete_flag = 1;
	di->dc_err_report_flag = FALSE;
	di->last_basp_level = BASP_PARA_LEVEL;

	if (direct_charge_ops_is_valid(di) == INVALID) {
		ret = -EINVAL;
		goto fail_free_mem;
	}

	di->sc_conv_ocp_count = 0;
	direct_charge_set_local_mode(OR_SET, SC_MODE);

	platform_set_drvdata(pdev, di);

	di->charging_wq = create_singlethread_workqueue("sc_charging_wq");
	di->kick_wtd_wq = create_singlethread_workqueue("sc_wtd_wq");

	wake_lock_init(&di->charging_lock, WAKE_LOCK_SUSPEND, "sc_wakelock");
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

	ret = sc_sysfs_create_group(di);
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
			"direct_charger_sc");
		if (ret) {
			hwlog_err("sysfs link create failed\n");
			goto fail_create_link;
		}
	}

	g_sc_di = di;
	direct_charge_set_di(di);
	direct_charge_gen_nl_init(pdev);

	di->fault_nb.notifier_call = direct_charge_fault_notifier_call;
	ret = atomic_notifier_chain_register(&sc_fault_notifier_list,
		&di->fault_nb);
	if (ret < 0) {
		hwlog_err("register notifier failed\n");
		goto fail_create_link;
	}

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	ret = di->ls_ops->ls_init();
	di->ls_id = di->ls_ops->get_ls_id();
	if (di->ls_id < SWITCHCAP_BEGIN || di->ls_id >= SWITCHCAP_TOTAL) {
		hwlog_err("get device id info failed\n");
		di->ls_id = SWITCHCAP_TOTAL;
	}

	di->ls_name = sc_device_name[di->ls_id];
	hwlog_info("switchcap id=%d(%s)\n", di->ls_id, di->ls_name);
	ret |= di->ls_ops->ls_exit();
	if (ret == 0)
		set_hw_dev_flag(DEV_I2C_SWITCHCAP);
#endif /* CONFIG_HUAWEI_HW_DEV_DCT */

	if (power_if_ops_register(&sc_power_if_ops))
		hwlog_err("register power_if_ops failed\n");

	hwlog_info("probe end\n");
	return 0;

fail_create_link:
	sc_sysfs_remove_group(di);
	wake_lock_destroy(&di->charging_lock);
fail_free_mem:
	devm_kfree(&pdev->dev, di);
	di = NULL;
	g_sc_di = NULL;

	return ret;
}

static int sc_remove(struct platform_device *pdev)
{
	struct direct_charge_device *di = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	wake_lock_destroy(&di->charging_lock);
	devm_kfree(&pdev->dev, di);
	di = NULL;
	g_sc_di = NULL;

	hwlog_info("remove end\n");
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
