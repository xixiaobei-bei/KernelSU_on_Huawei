/*
 * lvc.c
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

	hwlog_info("set enable_charger=%d\n", di->sysfs_enable_charger);

	ret = set_direct_charger_disable_flags((val ?
		DIRECT_CHARGER_CLEAR_DISABLE_FLAGS :
		DIRECT_CHARGER_SET_DISABLE_FLAGS),
		DIRECT_CHARGER_SYS_NODE);
	if (ret)
		hwlog_err("set direct charge disable flags failed\n");

	return 0;
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

/* define public power interface */
static struct power_if_ops lvc_power_if_ops = {
	.set_enable_charger = lvc_set_enable_charger,
	.get_enable_charger = lvc_get_enable_charger,
	.type_name = "lvc",
};

int lvc_ops_register(struct loadswitch_ops *ops)
{
	int ret = 0;

	if (ops)
		g_lvc_ops = ops;
	else
		ret = -EPERM;

	return ret;
}

int lvc_batinfo_ops_register(struct batinfo_ops *ops)
{
	int ret = 0;

	if (ops)
		g_bi_lvc_ops = ops;
	else
		ret = -EPERM;

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

void lvc_mode_check(void)
{
	int local_mode;
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

	/* fourth: check lvc enable status */
	if (di->sysfs_enable_charger == 0) {
		di->direct_charge_succ_flag = DC_ERROR_CHARGE_DISABLED;
		direct_charge_send_normal_charging_uevent();

		hwlog_info("lvc is disabled\n");
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

		hwlog_err("lvc is disabled by uscp\n");
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

	/* eleventh: switch buck charging path to lvc charging path */
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
		/*
		 * power_dsm_dmd_report(POWER_DSM_BATTERY,
		 * DSM_DIRECT_CHARGE_VDROP_OVP, buf);
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
	LVC_SYSFS_FIELD_RW(enable_charger, ENABLE_CHARGER),
	LVC_SYSFS_FIELD_RW(iin_thermal, IIN_THERMAL),
	LVC_SYSFS_FIELD_RO(adaptor_detect, ADAPTOR_DETECT),
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
	int ret;
	int len;

	info = lvc_sysfs_field_lookup(attr->attr.name);
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
		hwlog_info("lvc_succ_flag=%d\n", di->direct_charge_succ_flag);
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			di->direct_charge_succ_flag);
		break;

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
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
	int ret;
	int index;
	int cur_low;

	info = lvc_sysfs_field_lookup(attr->attr.name);
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

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
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
	direct_charge_set_local_mode(OR_SET, LVC_MODE);

	platform_set_drvdata(pdev, di);

	di->charging_wq = create_singlethread_workqueue("lvc_charging_wq");
	di->kick_wtd_wq = create_singlethread_workqueue("lvc_wtd_wq");

	wake_lock_init(&di->charging_lock, WAKE_LOCK_SUSPEND, "lvc_wakelock");
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
	hwlog_info("loadswitch id=%d(%s)\n", di->ls_id, di->ls_name);
	ret |= di->ls_ops->ls_exit();
	if (ret == 0)
		set_hw_dev_flag(DEV_I2C_LOADSWITCH);
#endif /* CONFIG_HUAWEI_HW_DEV_DCT */

	if (power_if_ops_register(&lvc_power_if_ops))
		hwlog_err("register power_if_ops failed\n");

	hwlog_info("probe end\n");
	return 0;

fail_create_link:
	lvc_sysfs_remove_group(di);
	wake_lock_destroy(&di->charging_lock);
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

	wake_lock_destroy(&di->charging_lock);
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
